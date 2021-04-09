/*****************************************************************************
      Author
        ©. Michel Condemine, 4CE Industry (2010-2012)
      
      Contributors


	This software is a computer program whose purpose is to 
	        implement behavior describe in the OPC UA specification.
		see wwww.opcfoundation.org for more details about OPC.
	This software is governed by the CeCILL-C license under French law and
	abiding by the rules of distribution of free software.  You can  use, 
	modify and/ or redistribute the software under the terms of the CeCILL-C
	license as circulated by CEA, CNRS and INRIA at the following URL
	"http://www.cecill.info". 

	As a counterpart to the access to the source code and  rights to copy,
	modify and redistribute granted by the license, users are provided only
	with a limited warranty  and the software's author,  the holder of the
	economic rights,  and the successive licensors  have only  limited
	liability. 

	In this respect, the user's attention is drawn to the risks associated
	with loading,  using,  modifying and/or developing or reproducing the
	software by the user in light of its specific status of free software,
	that may mean  that it is complicated to manipulate,  and  that  also
	therefore means  that it is reserved for developers  and  experienced
	professionals having in-depth computer knowledge. Users are therefore
	encouraged to load and test the software's suitability as regards their
	requirements in conditions enabling the security of their systems and/or 
	data to be ensured and,  more generally, to use and operate it in the 
	same conditions as regards security. 

	The fact that you are presently reading this means that you have had
        knowledge of the CeCILL-C license and that you accept its terms.

*****************************************************************************/

/* core */
#include <opcua.h>

#ifdef OPCUA_HAVE_CLIENTAPI

#include <opcua_mutex.h>
#include <opcua_socket.h>
#include <opcua_pkifactory.h>
#include <opcua_cryptofactory.h>
#include <opcua_list.h>
#include <opcua_datetime.h>
#include <opcua_utilities.h>
#include <opcua_guid.h>

/* stackcore */
#include <opcua_types.h>
#include <opcua_builtintypes.h>
#include <opcua_binaryencoder.h>

/* communication */
#include <opcua_connection.h>
#include <opcua_tcpconnection.h>
#include <opcua_secureconnection.h>
#include <opcua_tcplistener.h>

#if OPCUA_HAVE_SOAPHTTP
#include <opcua_httpconnection_securityproxy.h>
#endif /* OPCUA_HAVE_SOAPHTTP */

#if OPCUA_HAVE_HTTPS
#include <opcua_https_connection.h>
#endif /* OPCUA_HAVE_HTTPS */

/* security */
#include <opcua_crypto.h>

/* client api */
#include <opcua_channel.h>
#include <opcua_asynccallstate.h>
#include <opcua_channel_internal.h>

/*============================================================================
 * Prototypes
 *===========================================================================*/
static OpcUa_StatusCode OpcUa_Channel_InternalDisconnectComplete(
    OpcUa_Channel                   hChannel,
    OpcUa_Void*                     pCallbackData,
    OpcUa_Channel_Event             eEvent,
    OpcUa_StatusCode                uStatus,
    OpcUa_Channel_SecurityToken*    pSecurityToken);

static OpcUa_StatusCode OpcUa_Channel_ResponseAvailable(
    OpcUa_Connection*               pConnection,
    OpcUa_Void*                     pCallbackData,
    OpcUa_StatusCode                uOperationStatus,
    OpcUa_InputStream**             ppIstrm);

/*============================================================================
 ** @brief Global table of known types.
 *===========================================================================*/
extern OpcUa_EncodeableTypeTable OpcUa_ProxyStub_g_EncodeableTypes;

/*============================================================================
 ** @brief Global table of supported namespaces.
 *===========================================================================*/
extern OpcUa_StringA             OpcUa_ProxyStub_StandardNamespaceUris[];

/*============================================================================
 * OpcUa_Channel_Create
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Channel_Create(  OpcUa_Channel*                  a_phChannel,
                                        OpcUa_Channel_SerializerType    a_eSerializerType)
{
    OpcUa_InternalChannel* pInternalChannel = OpcUa_Null;

    OpcUa_InitializeStatus(OpcUa_Module_Channel, "OpcUa_Channel_Create");

    OpcUa_ReturnErrorIfArgumentNull(a_phChannel);

    *a_phChannel = OpcUa_Null;

    /* increment counter */
    OpcUa_ProxyStub_RegisterChannel();

    pInternalChannel = (OpcUa_InternalChannel*)OpcUa_Alloc(sizeof(OpcUa_InternalChannel));
    OpcUa_GotoErrorIfAllocFailed(pInternalChannel);
    OpcUa_MemSet(pInternalChannel, 0, sizeof(OpcUa_InternalChannel));

    uStatus = OpcUa_Mutex_Create(&(pInternalChannel->Mutex));
    OpcUa_GotoErrorIfBad(uStatus);

    /* create the encoder and decoder */
    if(a_eSerializerType == OpcUa_Channel_SerializerType_Binary)
    {
        /* create encoder */
        uStatus = OpcUa_BinaryEncoder_Create(&pInternalChannel->Encoder);
        OpcUa_GotoErrorIfBad(uStatus);

        /* create decoder */
        uStatus = OpcUa_BinaryDecoder_Create(&pInternalChannel->Decoder);
        OpcUa_GotoErrorIfBad(uStatus);
    }

    /* the encoding type is not supported */
    else
    {
        OpcUa_GotoErrorWithStatus(OpcUa_BadNotSupported);
    }

    OpcUa_StringTable_Initialize(&pInternalChannel->NamespaceUris);
    uStatus = OpcUa_StringTable_AddStringList(&pInternalChannel->NamespaceUris, OpcUa_ProxyStub_StandardNamespaceUris, OpcUa_False);
    OpcUa_GotoErrorIfBad(uStatus);

    *a_phChannel = (OpcUa_Channel)pInternalChannel;

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    OpcUa_Channel_Delete((OpcUa_Channel*)&pInternalChannel);

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_Channel_Clear
 *===========================================================================*/
OPCUA_EXPORT OpcUa_Void OpcUa_Channel_Clear(OpcUa_Channel a_hChannel)
{
    if(a_hChannel != OpcUa_Null)
    {
        OpcUa_InternalChannel* pChannel = (OpcUa_InternalChannel*)a_hChannel;

        OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Channel_Clear: waiting for channel access\n");

        OpcUa_Mutex_Lock(pChannel->Mutex);

        OpcUa_MemSet(&pChannel->SecurityToken, 0, sizeof(OpcUa_Channel_SecurityToken));

        if(pChannel->TransportConnection != OpcUa_Null)
        {
            /* unlock the channel object during the connection shutdown to allow events to check the channel state */
            OpcUa_Mutex_Unlock(pChannel->Mutex);

            OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Channel_Clear: deleting transport connection\n");

            /* delete the transport connection */
            OpcUa_Connection_Delete(&pChannel->TransportConnection);

            OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Channel_Clear: transport connection deleted\n");

            OpcUa_Mutex_Lock(pChannel->Mutex);
        }

        if(pChannel->SecureConnection != OpcUa_Null)
        {
            /* unlock the channel object during the connection shutdown to allow events to check the channel state */
            OpcUa_Mutex_Unlock(pChannel->Mutex);

            OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Channel_Clear: deleting secure connection\n");

            /* delete the secure connection */
            OpcUa_Connection_Delete(&pChannel->SecureConnection);

            OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Channel_Clear: secure connection deleted\n");

            OpcUa_Mutex_Lock(pChannel->Mutex);
        }

        OpcUa_Encoder_Delete(&pChannel->Encoder);
        OpcUa_Decoder_Delete(&pChannel->Decoder);

        OpcUa_StringTable_Clear(&pChannel->NamespaceUris);

        OpcUa_String_Clear(&(pChannel->Url));
        OpcUa_Mutex_Unlock(pChannel->Mutex);
        OpcUa_Mutex_Delete(&pChannel->Mutex);

        OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Channel_Clear: done\n");
    }
}

/*============================================================================
 * OpcUa_Channel_Delete
 *===========================================================================*/
OPCUA_EXPORT OpcUa_Void OpcUa_Channel_Delete(OpcUa_Channel* a_phChannel)
{
    if(a_phChannel != OpcUa_Null && *a_phChannel != OpcUa_Null)
    {
        OpcUa_InternalChannel* pChannel = (OpcUa_InternalChannel*)*a_phChannel;

        *a_phChannel = OpcUa_Null;

        OpcUa_Channel_Clear(pChannel);
        OpcUa_Free(pChannel);

        /* decrement counter */
        OpcUa_ProxyStub_DeRegisterChannel();
    }
}

/*============================================================================
 * OpcUa_Channel_ReadResponse
 *===========================================================================*/
static OpcUa_StatusCode OpcUa_Channel_ReadResponse(
    OpcUa_Channel          a_pChannel,
    OpcUa_InputStream*     a_pIstrm,
    OpcUa_EncodeableType** a_ppResponseType,
    OpcUa_Void**           a_ppResponse)
{
    OpcUa_MessageContext cContext;
    OpcUa_Decoder* pDecoder = OpcUa_Null;
    OpcUa_InternalChannel* pChannel = OpcUa_Null;
    OpcUa_Handle hDecodeContext = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_Channel, "OpcUa_Channel_ReadResponse");

    OpcUa_ReturnErrorIfArgumentNull(a_pChannel);
    OpcUa_ReturnErrorIfArgumentNull(a_pIstrm);
    OpcUa_ReturnErrorIfArgumentNull(a_ppResponseType);
    OpcUa_ReturnErrorIfArgumentNull(a_ppResponse);

    *a_ppResponseType = OpcUa_Null;
    *a_ppResponse = OpcUa_Null;

    pChannel = (OpcUa_InternalChannel*)a_pChannel;
    pDecoder = pChannel->Decoder;

    /* initialize context */
    OpcUa_MessageContext_Initialize(&cContext);

    cContext.KnownTypes    = &OpcUa_ProxyStub_g_EncodeableTypes;
    cContext.NamespaceUris = &pChannel->NamespaceUris;

    /* create decoder */
    uStatus = pDecoder->Open(pDecoder, a_pIstrm, &cContext, &hDecodeContext);
    if(OpcUa_IsBad(uStatus))
    {
        OpcUa_GotoError;
    }

    /* decode message */
    uStatus = pDecoder->ReadMessage((struct _OpcUa_Decoder*)hDecodeContext, a_ppResponseType, a_ppResponse);
    if(OpcUa_IsBad(uStatus))
    {
        OpcUa_GotoError;
    }

    /* close decoder */
    OpcUa_Decoder_Close(pDecoder, &hDecodeContext);

    OpcUa_MessageContext_Clear(&cContext);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    /* delete decoder and stream */
    OpcUa_Decoder_Close(pDecoder, &hDecodeContext);
    OpcUa_MessageContext_Clear(&cContext);

    OpcUa_EncodeableObject_Delete(*a_ppResponseType, a_ppResponse);

    /* put output parameters in a known state */
    *a_ppResponseType = OpcUa_Null;
    *a_ppResponse = OpcUa_Null;

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_Channel_ResponseAvailable
 *===========================================================================*/
/* This callback gets called by the connection if an event occured. */
static OpcUa_StatusCode OpcUa_Channel_ResponseAvailable(
    OpcUa_Connection*   a_pConnection,
    OpcUa_Void*         a_pCallbackData,
    OpcUa_StatusCode    a_uOperationStatus,
    OpcUa_InputStream** a_ppIstrm)
{
    OpcUa_AsyncCallState*   pAsyncState     = OpcUa_Null;
    OpcUa_Void*             pResponse       = OpcUa_Null;
    OpcUa_EncodeableType*   pResponseType   = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_Channel, "ResponseAvailable");

    OpcUa_ReturnErrorIfArgumentNull(a_pConnection);
    OpcUa_ReturnErrorIfArgumentNull(a_pCallbackData);

    pAsyncState = (OpcUa_AsyncCallState*)a_pCallbackData;

    if(pAsyncState != OpcUa_Null)
    {
        OpcUa_Mutex_Lock(pAsyncState->WaitMutex);

        OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Channel_ResponseAvailable: Operation Status 0x%X (async state 0x%p)\n", a_uOperationStatus, pAsyncState);

        if(OpcUa_IsGood(a_uOperationStatus))
        {
            /* if the caller does not give a stream, just do a notification */
            if(a_ppIstrm != OpcUa_Null && (*a_ppIstrm) != OpcUa_Null)
            {
                uStatus = OpcUa_Channel_ReadResponse(   pAsyncState->Channel,
                                                        (*a_ppIstrm),
                                                        &pResponseType,
                                                        &pResponse);
                if(OpcUa_IsGood(uStatus))
                {
                    pAsyncState->ResponseData = pResponse;
                    pAsyncState->ResponseType = pResponseType;
                    pResponse = OpcUa_Null;

                    if(pResponseType != OpcUa_Null)
                    {
#if !OPCUA_ENCODEABLE_OBJECT_OMIT_TYPE_NAME
                        OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Channel_ResponseAvailable: %s\n", pResponseType->TypeName);
#else /* !OPCUA_ENCODEABLE_OBJECT_OMIT_TYPE_NAME */
                        OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Channel_ResponseAvailable: Type Id %u\n", pResponseType->TypeId);
#endif /* !OPCUA_ENCODEABLE_OBJECT_OMIT_TYPE_NAME */
                    }
                    else
                    {
                        OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_WARNING, "OpcUa_Channel_ResponseAvailable: Empty or unknown response! (0x%08X)\n", a_uOperationStatus);
                    }
                }
                else
                {
                    a_uOperationStatus  = uStatus;
                    OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_WARNING, "OpcUa_Channel_ResponseAvailable: Decoding failed! (0x%08X)\n", a_uOperationStatus);
                }
            }
        }
        else
        {
            OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_WARNING, "OpcUa_Channel_ResponseAvailable: Request failed! (0x%08X)\n", a_uOperationStatus);
        }

        /* signal the request issuer that a response is available */
        if(pAsyncState->Callback == OpcUa_Null)
        {
#if OPCUA_USE_SYNCHRONISATION
            OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Channel_ResponseAvailable: Signalling Response!\n");

            /* this path is executed when the synchronous invoke is called. */
            uStatus = OpcUa_AsyncCallState_SignalCompletion(pAsyncState,
                                                            a_uOperationStatus);

            OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Channel_ResponseAvailable: Signalling Response Done!\n");

            OpcUa_Mutex_Unlock(pAsyncState->WaitMutex);
            /* unlike below, the asyncstate object is destroyed be the waiting thread. */
#else /* OPCUA_USE_SYNCHRONISATION */
            OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Channel_ResponseAvailable: Internal error!\n");
#endif /* OPCUA_USE_SYNCHRONISATION */
        }
        else /* invoke the application supplied callback */
        {
            pAsyncState->Status = a_uOperationStatus;

            OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Channel_ResponseAvailable: Calling Application Callback!\n");

            /* user supplied callback */
            uStatus = pAsyncState->Callback(    pAsyncState->Channel,
                                                pAsyncState->ResponseData,
                                                pAsyncState->ResponseType,
                                                pAsyncState->CallbackData,
                                                pAsyncState->Status);

            /*pAsyncState->ResponseData = OpcUa_Null;*/

            OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Channel_ResponseAvailable: Calling Application Callback Done!\n");

            OpcUa_Mutex_Unlock(pAsyncState->WaitMutex);
            OpcUa_AsyncCallState_Delete(&pAsyncState);
        }
    }

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Channel_ResponseAvailable: Leaving with Error %08X!\n", uStatus);

    if(pAsyncState != OpcUa_Null)
    {
        OpcUa_Mutex_Unlock(pAsyncState->WaitMutex); /* if this function is changed, check if mutex is always locked when getting to this point */
        OpcUa_AsyncCallState_Delete(&pAsyncState);
    }

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_Channel_BeginInvokeService
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Channel_BeginInvokeService(  OpcUa_Channel                     a_hChannel,
                                                    OpcUa_StringA                     a_sName,
                                                    OpcUa_Void*                       a_pRequest,
                                                    OpcUa_EncodeableType*             a_pRequestType,
                                                    OpcUa_Channel_PfnRequestComplete* a_pCallback,
                                                    OpcUa_Void*                       a_pCallbackData)
{
    OpcUa_MessageContext    cContext;
    OpcUa_Encoder*          pEncoder        = OpcUa_Null;
    OpcUa_OutputStream*     pSecureOstrm    = OpcUa_Null;
    OpcUa_InternalChannel*  pChannel        = OpcUa_Null;
    OpcUa_UInt32            uTimeout        = 0;
    OpcUa_AsyncCallState*   pAsyncState     = OpcUa_Null;
    OpcUa_Handle            hEncodeContext  = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_Channel, "OpcUa_Channel_BeginInvokeService");

    OpcUa_ReturnErrorIfArgumentNull(a_hChannel);
    OpcUa_ReturnErrorIfArgumentNull(a_pRequest);
    OpcUa_ReturnErrorIfArgumentNull(a_pRequestType);
    OpcUa_ReturnErrorIfArgumentNull(a_pCallback);
    OpcUa_ReferenceParameter(a_sName);

    pChannel = (OpcUa_InternalChannel*)a_hChannel;
    pEncoder = pChannel->Encoder;

    OpcUa_Mutex_Lock(pChannel->Mutex);

    if(pChannel->SecureConnection == OpcUa_Null)
    {
        OpcUa_Mutex_Unlock(pChannel->Mutex);
        OpcUa_GotoErrorWithStatus(OpcUa_BadServerNotConnected);
    }

    OpcUa_Mutex_Unlock(pChannel->Mutex);

#if !OPCUA_ENCODEABLE_OBJECT_OMIT_TYPE_NAME
    OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Channel_BeginInvokeService: called for %s!\n", a_pRequestType->TypeName);
#else /* !OPCUA_ENCODEABLE_OBJECT_OMIT_TYPE_NAME */
    OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Channel_BeginInvokeService: called for service with type id %us!\n", a_pRequestType->TypeId);
#endif /* !OPCUA_ENCODEABLE_OBJECT_OMIT_TYPE_NAME */


    /* initialize context */
    OpcUa_MessageContext_Initialize(&cContext);

    cContext.KnownTypes         = &OpcUa_ProxyStub_g_EncodeableTypes;
    cContext.NamespaceUris      = &pChannel->NamespaceUris;
    cContext.AlwaysCheckLengths = OPCUA_SERIALIZER_CHECKLENGTHS;

    /* retrieve the service call timeout */
    uTimeout = ((OpcUa_RequestHeader*)a_pRequest)->TimeoutHint;

    /* create output stream */
    uStatus = OpcUa_Connection_BeginSendRequest(pChannel->SecureConnection, &pSecureOstrm);
    OpcUa_GotoErrorIfBad(uStatus);

    /* open encoder */
    uStatus = pEncoder->Open(pEncoder, pSecureOstrm, &cContext, &hEncodeContext);
    OpcUa_GotoErrorIfBad(uStatus);

    /* encode message */
    uStatus = pEncoder->WriteMessage((struct _OpcUa_Encoder*)hEncodeContext, a_pRequest, a_pRequestType);
    OpcUa_GotoErrorIfBad(uStatus);

    /* close encoder and stream */
    OpcUa_Encoder_Close(pEncoder, &hEncodeContext);

    /* allocate the call state object */
    uStatus =  OpcUa_AsyncCallState_Create( pChannel,      /* channel handle */
                                            OpcUa_Null,    /* request */
                                            OpcUa_Null,    /* request type */
                                            &pAsyncState); /* the async state object */
    OpcUa_GotoErrorIfBad(uStatus);

    pAsyncState->Callback       = a_pCallback;
    pAsyncState->CallbackData   = a_pCallbackData;

    /* finish sending the request */
    uStatus = OpcUa_Connection_EndSendRequest(  pChannel->SecureConnection,
                                                &pSecureOstrm,
                                                uTimeout,
                                                OpcUa_Channel_ResponseAvailable,
                                                (OpcUa_Void*)pAsyncState);

    OpcUa_GotoErrorIfBad(uStatus);

    OpcUa_MessageContext_Clear(&cContext);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Channel_BeginInvokeService: failed with 0x%08X!\n", uStatus);

    if(pSecureOstrm != OpcUa_Null)
    {
        /* error occurred during message transmission; clean up resources allocated for request */
        OpcUa_Connection_AbortSendRequest(  pChannel->SecureConnection,
                                            uStatus,
                                            OpcUa_Null,
                                            &pSecureOstrm);
    }

    /* delete encoder and stream */
    OpcUa_Encoder_Close(pEncoder, &hEncodeContext);

    OpcUa_Stream_Delete((OpcUa_Stream**)&pSecureOstrm);

    OpcUa_MessageContext_Clear(&cContext);

    if(pAsyncState != OpcUa_Null)
    {
        OpcUa_AsyncCallState_Delete(&pAsyncState);
    }

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_Channel_InvokeService
 *===========================================================================*/
/* Main service invoke for synchronous behaviour; this function blocks until */
/* the server sends a response for this request.                             */
#if OPCUA_USE_SYNCHRONISATION
OpcUa_StatusCode OpcUa_Channel_InvokeService(   OpcUa_Channel           a_pChannel,
                                                OpcUa_StringA           a_sName,
                                                OpcUa_Void*             a_pRequest,
                                                OpcUa_EncodeableType*   a_pRequestType,
                                                OpcUa_Void**            a_ppResponse,
                                                OpcUa_EncodeableType**  a_ppResponseType)
{
    OpcUa_InternalChannel*  pChannel            = OpcUa_Null;
    OpcUa_OutputStream*     pOstrm              = OpcUa_Null;
    OpcUa_Encoder*          pEncoder            = OpcUa_Null;
    OpcUa_AsyncCallState*   pAsyncState         = OpcUa_Null;
    OpcUa_Handle            hEncodeContext      = OpcUa_Null;
    OpcUa_UInt32            uTimeout            = OPCUA_INFINITE;
    OpcUa_MessageContext    cContext;

OpcUa_InitializeStatus(OpcUa_Module_Channel, "InvokeService");

    OpcUa_ReturnErrorIfArgumentNull(a_pChannel);
    OpcUa_ReturnErrorIfArgumentNull(a_pRequest);
    OpcUa_ReturnErrorIfArgumentNull(a_pRequestType);
    OpcUa_ReturnErrorIfArgumentNull(a_ppResponse);
    OpcUa_ReturnErrorIfArgumentNull(a_ppResponseType);

    OpcUa_ReferenceParameter(a_sName);

    *a_ppResponse = OpcUa_Null;
    *a_ppResponseType = OpcUa_Null;

    OpcUa_MessageContext_Initialize(&cContext);

    pChannel = (OpcUa_InternalChannel*)a_pChannel;
    pEncoder = pChannel->Encoder;

    OpcUa_GotoErrorIfTrue((pChannel->SecureConnection == OpcUa_Null), OpcUa_BadServerNotConnected);

    /* initialize context */
    cContext.KnownTypes         = &OpcUa_ProxyStub_g_EncodeableTypes;
    cContext.NamespaceUris      = &pChannel->NamespaceUris;
    cContext.AlwaysCheckLengths = OPCUA_SERIALIZER_CHECKLENGTHS;

    /* retrieve the service call timeout */
    uTimeout = ((OpcUa_RequestHeader*)a_pRequest)->TimeoutHint;

    /* create output stream through connection */
    uStatus = OpcUa_Connection_BeginSendRequest(pChannel->SecureConnection, &pOstrm);
    OpcUa_GotoErrorIfBad(uStatus);

    /* open encoder */
    uStatus = pEncoder->Open(pEncoder, pOstrm, &cContext, &hEncodeContext);
    OpcUa_GotoErrorIfBad(uStatus);

    /* encode message */
    uStatus = pEncoder->WriteMessage((struct _OpcUa_Encoder*)hEncodeContext, a_pRequest, a_pRequestType);
    OpcUa_GotoErrorIfBad(uStatus);

    /* finish encoding of message */
    OpcUa_Encoder_Close(pEncoder, &hEncodeContext);

    /* allocate the call state object */
    uStatus =  OpcUa_AsyncCallState_Create(a_pChannel, OpcUa_Null, OpcUa_Null, &pAsyncState);
    OpcUa_ReturnErrorIfBad(uStatus);

    /*** send request to the server. ***/
    uStatus = OpcUa_Connection_EndSendRequest(  pChannel->SecureConnection,
                                                &pOstrm,
                                                uTimeout,
                                                OpcUa_Channel_ResponseAvailable,
                                                (OpcUa_Void*)pAsyncState);
    OpcUa_GotoErrorIfBad(uStatus);

    /*** clean up ***/
    OpcUa_MessageContext_Clear(&cContext);

    /* wait for notification on the socket ; timeout is handled in the layer below, hence we wait for "infinit" time */
    uStatus = OpcUa_AsyncCallState_WaitForCompletion(pAsyncState, OPCUA_INFINITE);
    if(OpcUa_IsBad(uStatus))
    {
        OpcUa_AsyncCallState_Delete(&pAsyncState);
    }
    else
    {
        *a_ppResponse = pAsyncState->ResponseData;
        *a_ppResponseType = pAsyncState->ResponseType;

        OpcUa_AsyncCallState_Delete(&pAsyncState);
    }

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    if(hEncodeContext != OpcUa_Null)
    {
        OpcUa_Encoder_Close(pEncoder, &hEncodeContext);
    }

    if(pOstrm != OpcUa_Null)
    {
        /* error occurred during message transmission; clean up resources allocated for request */
        OpcUa_Connection_AbortSendRequest(  pChannel->SecureConnection,
                                            uStatus,
                                            OpcUa_Null,
                                            &pOstrm);
    }

    OpcUa_MessageContext_Clear(&cContext);

    if(pAsyncState != OpcUa_Null)
    {
        OpcUa_AsyncCallState_Delete(&pAsyncState);
    }

OpcUa_FinishErrorHandling;
}
#else
/* function is not supported with synchronisation disabled. */
OpcUa_StatusCode OpcUa_Channel_InvokeService(   OpcUa_Channel           a_pChannel,
                                                OpcUa_StringA           a_sName,
                                                OpcUa_Void*             a_pRequest,
                                                OpcUa_EncodeableType*   a_pRequestType,
                                                OpcUa_Void**            a_ppResponse,
                                                OpcUa_EncodeableType**  a_ppResponseType)
{

    OpcUa_ReferenceParameter(a_pChannel);
    OpcUa_ReferenceParameter(a_sName);
    OpcUa_ReferenceParameter(a_pRequest);
    OpcUa_ReferenceParameter(a_pRequestType);
    OpcUa_ReferenceParameter(a_ppResponse);
    OpcUa_ReferenceParameter(a_ppResponseType);

    return OpcUa_BadNotSupported;
}
#endif /* OPCUA_USE_SYNCHRONISATION */

/*============================================================================
 * OpcUa_Channel_BeginSendEncodedRequest
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Channel_BeginSendEncodedRequest(
    OpcUa_Channel                   a_hChannel,
    OpcUa_ByteString*               a_pRequest,
    OpcUa_UInt32                    a_uTimeout,
    OpcUa_Connection_PfnOnResponse* a_pCallback,
    OpcUa_Void*                     a_pCallbackData)
{
    OpcUa_OutputStream*     pOstrm   = OpcUa_Null;
    OpcUa_InternalChannel*  pChannel = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_Channel, "BeginSendEncodedRequest");

    OpcUa_ReturnErrorIfArgumentNull(a_hChannel);
    OpcUa_ReturnErrorIfArgumentNull(a_pRequest);
    OpcUa_ReturnErrorIfArgumentNull(a_pCallback);

    pChannel = (OpcUa_InternalChannel*)a_hChannel;

    /* lock the session until the request is sent - not release until End or Abort is called. */
    OpcUa_Mutex_Lock(pChannel->Mutex);

    OpcUa_GotoErrorIfTrue((pChannel->SecureConnection == OpcUa_Null), OpcUa_BadServerNotConnected);

    /* create output stream */
    uStatus = OpcUa_Connection_BeginSendRequest(pChannel->SecureConnection, &pOstrm);
    OpcUa_GotoErrorIfBad(uStatus);

    /* write data */
    uStatus = OpcUa_Stream_Write(pOstrm, a_pRequest->Data, a_pRequest->Length);
    OpcUa_GotoErrorIfBad(uStatus);

    /* finish sending the request */
    uStatus = OpcUa_Connection_EndSendRequest(  pChannel->SecureConnection,
                                                &pOstrm,
                                                a_uTimeout,
                                                a_pCallback,
                                                a_pCallbackData);
    OpcUa_GotoErrorIfBad(uStatus);

    /* unlock the mutex (locked by OpcUa_Channel_BeginSendRequest) */
    OpcUa_Mutex_Unlock(pChannel->Mutex);

    /* close stream */
    OpcUa_Stream_Close((OpcUa_Stream*)pOstrm);
    OpcUa_Stream_Delete((OpcUa_Stream**)&pOstrm);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    /* unlock the mutex (locked by OpcUa_Channel_BeginSendRequest) */
    OpcUa_Mutex_Unlock(pChannel->Mutex);

    /* delete stream */
    OpcUa_Stream_Close((OpcUa_Stream*)pOstrm);
    OpcUa_Stream_Delete((OpcUa_Stream**)&pOstrm);

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_Channel_OnNotify
 *===========================================================================*/
/* this function is called asynchronously by the underlying connection object  */
/* if a connection event happens. Only used for connecting and disconnecting. */
static OpcUa_StatusCode OpcUa_Channel_OnNotify( OpcUa_Connection*     a_pConnection,
                                                OpcUa_Void*           a_pCallbackData,
                                                OpcUa_ConnectionEvent a_eEvent,
                                                OpcUa_InputStream**   a_ppInputStream,
                                                OpcUa_ByteString*     a_pCertificate,
                                                OpcUa_StatusCode      a_uOperationStatus)
{
    OpcUa_InternalChannel*                      pInternalChannel    = (OpcUa_InternalChannel*)a_pCallbackData;
    OpcUa_Channel_PfnConnectionStateChanged*    pfCallback          = OpcUa_Null;
    OpcUa_Void*                                 pvCallbackData      = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_Channel, "OnNotify");

    OpcUa_ReturnErrorIfArgumentNull(a_pConnection);
    OpcUa_ReturnErrorIfArgumentNull(pInternalChannel);

    OpcUa_ReferenceParameter(a_ppInputStream); /* a stream is not expected in this callback */

    OpcUa_Mutex_Lock(pInternalChannel->Mutex);

    pfCallback      = pInternalChannel->pfCallback;
    pvCallbackData  = pInternalChannel->pvCallbackData;

    switch(a_eEvent)
    {
    case OpcUa_ConnectionEvent_Connect:
        {
            OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Channel_OnNotify: Underlying connection raised connect event with status 0x%08X!\n", a_uOperationStatus);

            if(pfCallback != OpcUa_Null)
            {
                OpcUa_MemSet(&pInternalChannel->SecurityToken, 0, sizeof(OpcUa_Channel_SecurityToken));

                /* simple way to check for HTTPS - Transport Connection is null */
                if(pInternalChannel->TransportConnection != OpcUa_Null)
                {
                    pInternalChannel->SecurityToken.eTokenType = OpcUa_Channel_SecurityTokenType_OpcSecureConversation;
                    OpcUa_SecureConnection_GetSecurityToken(a_pConnection,
                                                            &pInternalChannel->SecurityToken.SecurityToken.pOpcChannelSecurityToken);
                }

                OpcUa_Mutex_Unlock(pInternalChannel->Mutex);

                OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Channel_OnNotify: Notifying application!\n");

                uStatus = pfCallback(   pInternalChannel,
                                        pvCallbackData,
                                        eOpcUa_Channel_Event_Connected,
                                        a_uOperationStatus,
                                       &pInternalChannel->SecurityToken);

                OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Channel_OnNotify: Notifying application done!\n");
            }
            else
            {
                OpcUa_Mutex_Unlock(pInternalChannel->Mutex);
                OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Channel_OnNotify: Can not notify application!\n");
            }

            break;
        }
    case OpcUa_ConnectionEvent_Reconnecting:
        {
            OpcUa_Mutex_Unlock(pInternalChannel->Mutex);
            OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Channel_OnNotify: Underlying connection is trying to reconnect!\n");

            /* no way to tell the application about the disconnect directly; only through the status codes returned with the requests. */
            break;
        }
    case OpcUa_ConnectionEvent_Renew:
        {
            OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Channel_OnNotify: Security token has been renewed!\n");

            if(     OpcUa_IsGood(uStatus)
                &&  pfCallback != OpcUa_Null)
            {
                OpcUa_Channel_SecurityToken SecurityToken;

                OpcUa_MemSet(&SecurityToken, 0, sizeof(OpcUa_Channel_SecurityToken));

                /* simple way to check for HTTPS - Transport Connection is null */
                if(pInternalChannel->TransportConnection != OpcUa_Null)
                {
                    SecurityToken.eTokenType = OpcUa_Channel_SecurityTokenType_OpcSecureConversation;
                    OpcUa_SecureConnection_GetSecurityToken(a_pConnection,
                                                            &SecurityToken.SecurityToken.pOpcChannelSecurityToken);
                }

                OpcUa_Mutex_Unlock(pInternalChannel->Mutex);

                OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Channel_OnNotify: Notifying application!\n");

                uStatus = pfCallback(   pInternalChannel,
                                        pvCallbackData,
                                        eOpcUa_Channel_Event_Renewed,
                                        a_uOperationStatus,
                                       &SecurityToken);

                OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Channel_OnNotify: Notifying application done!\n");
            }
            else
            {
                OpcUa_Mutex_Unlock(pInternalChannel->Mutex);
                OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Channel_OnNotify: Can not notify application: CB %p!\n", pfCallback);
            }
            break;
        }
    case OpcUa_ConnectionEvent_Disconnect:
        {
            OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Channel_OnNotify: Underlying connection raised disconnect event!\n");

            if(pfCallback != OpcUa_Null)
            {
                pInternalChannel->pfCallback = OpcUa_Null;
                pInternalChannel->pvCallbackData = OpcUa_Null;

                OpcUa_Mutex_Unlock(pInternalChannel->Mutex);

                OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Channel_OnNotify: Notifying application!\n");

                uStatus = pfCallback(   pInternalChannel,
                                        pvCallbackData,
                                        eOpcUa_Channel_Event_Disconnected,
                                        a_uOperationStatus,
                                        OpcUa_Null);

                OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Channel_OnNotify: Notifying application done!\n");
            }
            else
            {
				OpcUa_Mutex_Unlock(pInternalChannel->Mutex);
                OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Channel_OnNotify: Can not notify application: CB %p!\n", pInternalChannel?pInternalChannel->pfCallback:OpcUa_Null);
            }

            break;
        }
    case OpcUa_ConnectionEvent_UnexpectedError:
        {
            OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Channel_OnNotify: Underlying connection raised unexpected error event with status 0x%08X!\n", a_uOperationStatus);

            if(pfCallback != OpcUa_Null)
            {
                pInternalChannel->pfCallback = OpcUa_Null;
                pInternalChannel->pvCallbackData = OpcUa_Null;

                OpcUa_Mutex_Unlock(pInternalChannel->Mutex);

                OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Channel_OnNotify: Notifying application!\n");

                uStatus = pfCallback(   pInternalChannel,
                                        pvCallbackData,
                                        eOpcUa_Channel_Event_Disconnected,
                                        a_uOperationStatus,
                                        OpcUa_Null);

                OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Channel_OnNotify: Notifying application done!\n");
            }
            else
            {
				OpcUa_Mutex_Unlock(pInternalChannel->Mutex);
                OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Channel_OnNotify: Cannot inform client application about error 0x%08X\n", a_uOperationStatus);
            }

            break;
        }
#if OPCUA_HAVE_HTTPS
    case OpcUa_ConnectionEvent_VerifyCertificate:
        {
            OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Channel_OnNotify: Underlying connection raised certificate validation event with status 0x%08X!\n", a_uOperationStatus);

            if(pfCallback != OpcUa_Null)
            {
                OpcUa_Channel_SecurityToken SecurityToken;

                OpcUa_MemSet(&SecurityToken, 0, sizeof(OpcUa_Channel_SecurityToken));

                /* simple way to check for HTTPS - Transport Connection is null */
                if(pInternalChannel->TransportConnection != OpcUa_Null)
                {
                    OpcUa_Mutex_Unlock(pInternalChannel->Mutex);
                    OpcUa_GotoErrorWithStatus(OpcUa_BadUnexpectedError);
                }
                else
                {
                    SecurityToken.eTokenType = OpcUa_Channel_SecurityTokenType_Https;
                    SecurityToken.SecurityToken.HttpsSecurityToken.bsServerCertificate = *a_pCertificate;

                }

                OpcUa_Mutex_Unlock(pInternalChannel->Mutex);

                OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Channel_OnNotify: Notifying application!\n");

                uStatus = pfCallback(   pInternalChannel,
                                        pvCallbackData,
                                        eOpcUa_Channel_Event_VerifyCertificate,
                                        a_uOperationStatus,
                                       &SecurityToken);

                OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Channel_OnNotify: Notifying application done!\n");
            }
            else
            {
                OpcUa_Mutex_Unlock(pInternalChannel->Mutex);
                OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Channel_OnNotify: Can not notify application!\n");
            }

            break;
        }
#else
        OpcUa_ReferenceParameter(a_pCertificate);
#endif /* OPCUA_HAVE_HTTPS */
    default:
        {
            OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Channel_OnNotify: Underlying connection raised unspecified event!\n");
			OpcUa_Mutex_Unlock(pInternalChannel->Mutex);
            break;
        }
    } /* switch on event type */

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_Channel_DisconnectComplete
 *===========================================================================*/
static OpcUa_StatusCode OpcUa_Channel_InternalDisconnectComplete(OpcUa_Channel                  a_hChannel,
                                                                OpcUa_Void*                     a_pCallbackData,
                                                                OpcUa_Channel_Event             a_eEvent,
                                                                OpcUa_StatusCode                a_uStatus,
                                                                OpcUa_Channel_SecurityToken*    a_pSecurityToken)
{
    OpcUa_InternalChannel* pChannel = (OpcUa_InternalChannel *)a_hChannel;

OpcUa_InitializeStatus(OpcUa_Module_Channel, "DisconnectComplete");

    OpcUa_ReferenceParameter(a_eEvent);
    OpcUa_ReferenceParameter(a_pSecurityToken);

    OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Channel_InternalDisconnectComplete called!\n");

    /* Moved deletion to OpcUa_Channel_Connect(), because this function could be called from */
    /* a network thread created through theses connections. They could not delete their    */
    /* threads. */
#if OPCUA_USE_SYNCHRONISATION
    uStatus = OpcUa_AsyncCallState_SignalCompletion((OpcUa_AsyncCallState*)a_pCallbackData, a_uStatus);
#else
    OpcUa_ReferenceParameter(a_pCallbackData);
    OpcUa_ReferenceParameter(a_uStatus);
#endif /* OPCUA_USE_SYNCHRONISATION */

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    OpcUa_Connection_Delete(&pChannel->SecureConnection);

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_Channel_BeginDisconnect
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_Channel_BeginDisconnect(OpcUa_Channel                               a_pChannel,
                                                            OpcUa_Channel_PfnConnectionStateChanged*    a_pfCallback,
                                                            OpcUa_Void*                                 a_pCallbackData)
{
    OpcUa_InternalChannel* pChannel = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_Channel, "BeginDisconnect");

    pChannel = (OpcUa_InternalChannel*)a_pChannel;

    /* lock the session until the request is sent */
    OpcUa_Mutex_Lock(pChannel->Mutex);

    pChannel->pfCallback        = a_pfCallback;
    pChannel->pvCallbackData    = a_pCallbackData;

    OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Channel_BeginDisconnect: Beginning to disconnect!\n");

    OpcUa_Mutex_Unlock(pChannel->Mutex);

    /* disconnect to the server - notify on standard callback */
    uStatus = OpcUa_Connection_Disconnect(  pChannel->SecureConnection,
                                            OpcUa_True); /* request callback if call succeeds */

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    OpcUa_Mutex_Unlock(pChannel->Mutex);

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_Channel_Disconnect
 *===========================================================================*/
/* synchronous disconnect from server - blocks because of securechannel messages delay */
OPCUA_EXPORT OpcUa_StatusCode OpcUa_Channel_Disconnect(OpcUa_Channel a_hChannel)
{
    OpcUa_AsyncCallState*   pAsyncState = OpcUa_Null;
    OpcUa_InternalChannel*  pChannel    = (OpcUa_InternalChannel*)a_hChannel;

OpcUa_InitializeStatus(OpcUa_Module_Channel, "Disconnect");

    OpcUa_ReturnErrorIfArgumentNull(a_hChannel);
    OpcUa_ReferenceParameter(pChannel);

    /* create waithandle */
    uStatus =  OpcUa_AsyncCallState_Create(a_hChannel, OpcUa_Null, OpcUa_Null, &pAsyncState);
    OpcUa_ReturnErrorIfBad(uStatus);

    uStatus = OpcUa_Channel_BeginDisconnect(    a_hChannel,
                                                OpcUa_Channel_InternalDisconnectComplete,
                                                (OpcUa_Void*)pAsyncState);
    OpcUa_GotoErrorIfBad(uStatus);

    /* ************************ wait for completion ************************** */
#if OPCUA_MULTITHREADED
    uStatus = OpcUa_AsyncCallState_WaitForCompletion(   pAsyncState,
                                                        OPCUA_INFINITE /*pChannel->NetworkTimeout*/);

    if(OpcUa_IsNotEqual(OpcUa_BadTimeout))
    {
        /* do not free async state object if timeout occurred (callback will happen eventually). */
        OpcUa_AsyncCallState_Delete(&pAsyncState);
        OpcUa_GotoError;
    }
#else
    OpcUa_ReferenceParameter(pChannel);
    uStatus = OpcUa_GoodCompletesAsynchronously;
#endif

    OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Channel_Disconnect: Woke up with status 0x%08X\n", uStatus);

    /* ********************** end wait for completion ************************ */

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    if(pAsyncState != OpcUa_Null)
    {
        OpcUa_AsyncCallState_Delete(&pAsyncState);
    }

OpcUa_FinishErrorHandling;
}

#if OPCUA_USE_SYNCHRONISATION
/*============================================================================
 * OpcUa_Channel_InternalConnectComplete
 *===========================================================================*/
/* this function is the counterpart of OpcUa_Channel_Connect() and unlocks it */
static OpcUa_StatusCode OpcUa_Channel_InternalConnectComplete(  OpcUa_Channel                   a_hChannel,
                                                                OpcUa_Void*                     a_pCallbackData,
                                                                OpcUa_Channel_Event             a_eEvent,
                                                                OpcUa_StatusCode                a_uStatus,
                                                                OpcUa_Channel_SecurityToken*    a_pSecurityToken)
{
    OpcUa_AsyncCallState* pAsyncState = (OpcUa_AsyncCallState*)a_pCallbackData;

    OpcUa_ReturnErrorIfArgumentNull(a_pCallbackData);

    OpcUa_ReferenceParameter(a_hChannel);
    OpcUa_ReferenceParameter(a_eEvent);

    switch(a_eEvent)
    {
    case eOpcUa_Channel_Event_Connected:
    case eOpcUa_Channel_Event_Disconnected:
        {
            pAsyncState->ResponseData = a_pSecurityToken;
            pAsyncState->ResponseType = &OpcUa_ChannelSecurityToken_EncodeableType;

            return OpcUa_AsyncCallState_SignalCompletion(pAsyncState, a_uStatus);
        }
#if OPCUA_HAVE_HTTPS
    case eOpcUa_Channel_Event_VerifyCertificate:
        {
            /* if the sync wrapper is used, the event has to be forwarded one additional step */
            OpcUa_InternalChannel* pChannel = (OpcUa_InternalChannel*)a_hChannel;

            if(pChannel != OpcUa_Null && pChannel->pfTempCallback != OpcUa_Null)
            {
                return pChannel->pfTempCallback(a_hChannel,
                                                pChannel->pvTempCallbackData,
                                                a_eEvent,
                                                a_uStatus,
                                                a_pSecurityToken);
            }

            break;
        }
#endif /* OPCUA_HAVE_HTTPS */
    default:
        {
            /* unexpected error is default */
            break;
        }
    }

    return OpcUa_BadUnexpectedError;
}
#endif /* OPCUA_USE_SYNCHRONISATION */

/*============================================================================
 * OpcUa_Channel_InternalBeginConnect
 *===========================================================================*/
/* initiates an asynchronous connect process */
OpcUa_StatusCode OpcUa_Channel_BeginConnect(OpcUa_Channel                               a_pChannel,
                                            const OpcUa_CharA*                          a_sUrl,
                                            const OpcUa_CharA*                          a_sTransportProfileUri,
                                            OpcUa_ByteString*                           a_pClientCertificate,
                                            OpcUa_ByteString*                           a_pClientPrivateKey,
                                            OpcUa_ByteString*                           a_pServerCertificate,
                                            OpcUa_Void*                                 a_pPKIConfig,
                                            OpcUa_String*                               a_pRequestedSecurityPolicyUri,
                                            OpcUa_Int32                                 a_nRequestedLifetime,
                                            OpcUa_MessageSecurityMode                   a_messageSecurityMode,
                                            OpcUa_UInt32                                a_nNetworkTimeout,
                                            OpcUa_Channel_PfnConnectionStateChanged*    a_pfCallback,
                                            OpcUa_Void*                                 a_pCallbackData)
{
    OpcUa_InternalChannel*      pChannel            = (OpcUa_InternalChannel*)a_pChannel;
    OpcUa_ClientCredential*     pClientCredentials  = OpcUa_Null;

#if OPCUA_USE_SYNCHRONISATION
    OpcUa_Boolean               bLocked             = OpcUa_False;
#endif /* OPCUA_USE_SYNCHRONISATION */

OpcUa_InitializeStatus(OpcUa_Module_Channel, "BeginConnect");

    OpcUa_ReturnErrorIfArgumentNull(a_pChannel);
    OpcUa_ReturnErrorIfArgumentNull(a_pfCallback);

    if(a_nNetworkTimeout == 0)
    {
        uStatus = OpcUa_BadInvalidArgument;
        OpcUa_ReturnStatusCode;
    }

    /* lock the session until the request is sent */
#if OPCUA_USE_SYNCHRONISATION
    OpcUa_Mutex_Lock(pChannel->Mutex);
    bLocked = OpcUa_True;
#endif

	OpcUa_String* pszTransportProfileUri = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszTransportProfileUri);
	OpcUa_String_AttachCopy(pszTransportProfileUri, a_sTransportProfileUri);
	//
	OpcUa_String* pszTransportUaTcp = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszTransportUaTcp);
	OpcUa_String_AttachCopy(pszTransportUaTcp, OpcUa_TransportProfile_UaTcp);
	//
	OpcUa_String* pszTransportHttpsBinary = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszTransportHttpsBinary);
	OpcUa_String_AttachCopy(pszTransportHttpsBinary, OpcUa_TransportProfile_HttpsBinary);
	//
	OpcUa_String* pszTransportHttpsSoapBinary = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszTransportHttpsSoapBinary);
	OpcUa_String_AttachCopy(pszTransportHttpsSoapBinary, OpcUa_TransportProfile_HttpsSoapBinary);
    /* save url */
    OpcUa_String_Initialize(&pChannel->Url);
	uStatus = OpcUa_String_AttachCopy(&pChannel->Url, a_sUrl);

    OpcUa_GotoErrorIfBad(uStatus);

    /* save session timeout */
    pChannel->NetworkTimeout = a_nNetworkTimeout;

    /* remove old connection objects */
    if(pChannel->TransportConnection != OpcUa_Null)
    {
        OpcUa_Connection_Delete(&pChannel->TransportConnection);
    }

    /* delete the secure connection */
    if(pChannel->SecureConnection != OpcUa_Null)
    {
        OpcUa_Connection_Delete(&pChannel->SecureConnection);
    }

    /* select the protocol type based on the url scheme */
	OpcUa_String* pszEndPointHeader = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszEndPointHeader);
	OpcUa_String_AttachCopy(pszEndPointHeader, "opc.tcp:");
    if(!OpcUa_String_StrnCmp(   &(pChannel->Url),
								pszEndPointHeader,
                                OpcUa_String_StrLen(pszEndPointHeader),
                                OpcUa_False))
    {
		if (!OpcUa_String_StrnCmp(pszTransportProfileUri, pszTransportUaTcp,OPCUA_STRING_LENDONTCARE,OpcUa_False))
        {
            if(     (a_messageSecurityMode != OpcUa_MessageSecurityMode_None)
                &&  ((a_pServerCertificate->Length <= 0) || (a_pClientCertificate->Length <= 0) || (a_pClientPrivateKey->Length <= 0)))
            {
                OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Channel_BeginConnect: Cannot create secure channel without certificates!\n");
                OpcUa_GotoErrorWithStatus(OpcUa_BadInvalidArgument);
            }

            uStatus = OpcUa_TcpConnection_Create(&pChannel->TransportConnection);
            OpcUa_GotoErrorIfBad(uStatus);
        }
        else
        {
            OpcUa_GotoErrorWithStatus(OpcUa_BadNotSupported);
        }
    }
	else
	{
#if OPCUA_HAVE_HTTPS || OPCUA_HAVE_SOAPHTTP
		OpcUa_String_Clear(pszEndPointHeader);
		OpcUa_String_Initialize(pszEndPointHeader);
		OpcUa_String_AttachCopy(pszEndPointHeader, "http:");
		if (!OpcUa_String_StrnCmp(&(pChannel->Url),
			pszEndPointHeader,
			OpcUa_String_StrLen(pszEndPointHeader),
			OpcUa_False))
		{
#if OPCUA_HAVE_HTTPS
			if (!OpcUa_String_StrnCmp(pszTransportProfileUri, pszTransportHttpsBinary, OPCUA_STRING_LENDONTCARE, OpcUa_False))
			{
				uStatus = OpcUa_HttpsConnection_Create(&pChannel->SecureConnection);
				OpcUa_GotoErrorIfBad(uStatus);
			}
			else
				if (!OpcUa_String_StrnCmp(pszTransportProfileUri, pszTransportHttpsSoapBinary, OPCUA_STRING_LENDONTCARE, OpcUa_False))
				{
					OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Channel_BeginConnect: Encoding limited to UA Binary!\n");
					uStatus = OpcUa_HttpsConnection_Create(&pChannel->SecureConnection);
					OpcUa_GotoErrorIfBad(uStatus);
				}
				else
#endif /* OPCUA_HAVE_SOAPHTTP */
				{
					OpcUa_GotoErrorWithStatus(OpcUa_BadNotSupported);
				}
		}
		else
#endif /* OPCUA_HAVE_HTTPS || OPCUA_HAVE_SOAPHTTP */
		{
			OpcUa_GotoErrorWithStatus(OpcUa_BadNotSupported);
		}
	}
	// clear resources
	if (pszEndPointHeader)
	{
		OpcUa_String_Clear(pszEndPointHeader);
		OpcUa_Free(pszEndPointHeader);
		pszEndPointHeader = OpcUa_Null;
	}
	if (pszTransportProfileUri)
	{
		OpcUa_String_Clear(pszTransportProfileUri);
		OpcUa_Free(pszTransportProfileUri);
		pszTransportProfileUri = OpcUa_Null;
	}
	if (pszTransportUaTcp)
	{
		OpcUa_String_Clear(pszTransportUaTcp);
		OpcUa_Free(pszTransportUaTcp);
		pszTransportUaTcp = OpcUa_Null;
	}
	if (pszTransportHttpsBinary)
	{
		OpcUa_String_Clear(pszTransportHttpsBinary);
		OpcUa_Free(pszTransportHttpsBinary);
		pszTransportHttpsBinary = OpcUa_Null;
	}
	if (pszTransportHttpsSoapBinary)
	{
		OpcUa_String_Clear(pszTransportHttpsSoapBinary);
		OpcUa_Free(pszTransportHttpsSoapBinary);
		pszTransportHttpsSoapBinary = OpcUa_Null;
	}

    if(pChannel->SecureConnection == OpcUa_Null)
    {
        uStatus = OpcUa_SecureConnection_Create(pChannel->TransportConnection,
                                                pChannel->Encoder,
                                                pChannel->Decoder,
                                                &pChannel->NamespaceUris,
                                                &OpcUa_ProxyStub_g_EncodeableTypes,
                                                &pChannel->SecureConnection);
    }
    OpcUa_GotoErrorIfBad(uStatus);

    pChannel->pfCallback        = a_pfCallback;
    pChannel->pvCallbackData    = a_pCallbackData;

    /* create transport credential */
    pClientCredentials = (OpcUa_ClientCredential*)OpcUa_Alloc(sizeof(OpcUa_ClientCredential));
    OpcUa_GotoErrorIfAllocFailed(pClientCredentials);
    OpcUa_MemSet(pClientCredentials, 0, sizeof(OpcUa_ClientCredential));

    pClientCredentials->Credential.TheActuallyUsedCredential.messageSecurityMode         = a_messageSecurityMode;
    pClientCredentials->Credential.TheActuallyUsedCredential.nRequestedLifetime          = a_nRequestedLifetime;
    pClientCredentials->Credential.TheActuallyUsedCredential.pClientCertificateChain     = a_pClientCertificate;
    pClientCredentials->Credential.TheActuallyUsedCredential.pClientPrivateKey           = a_pClientPrivateKey;
    pClientCredentials->Credential.TheActuallyUsedCredential.pkiConfig                   = a_pPKIConfig;
    pClientCredentials->Credential.TheActuallyUsedCredential.pRequestedSecurityPolicyUri = a_pRequestedSecurityPolicyUri;
    pClientCredentials->Credential.TheActuallyUsedCredential.pServerCertificate          = a_pServerCertificate;

    /* lock the session until the request is sent */
#if OPCUA_USE_SYNCHRONISATION
    OpcUa_Mutex_Unlock(pChannel->Mutex);
    bLocked = OpcUa_False;
#endif

    /* connect asynchronously to the server - the remaining stuff is done in the callback */
    uStatus = pChannel->SecureConnection->Connect(  pChannel->SecureConnection,
                                                    &(pChannel->Url),
                                                    pClientCredentials,
                                                    pChannel->NetworkTimeout,
                                                    OpcUa_Channel_OnNotify,
                                                    (OpcUa_Void*)pChannel);
    OpcUa_GotoErrorIfBad(uStatus);

    OpcUa_Free(pClientCredentials);
	// 
	OpcUa_String_Clear(&pChannel->Url);
OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
	// clear resources
	if (pszEndPointHeader)
	{
		OpcUa_String_Clear(pszEndPointHeader);
		OpcUa_Free(pszEndPointHeader);
		pszEndPointHeader = OpcUa_Null;
	}
	if (pszTransportProfileUri)
	{
		OpcUa_String_Clear(pszTransportProfileUri);
		OpcUa_Free(pszTransportProfileUri);
		pszTransportProfileUri = OpcUa_Null;
	}
	if (pszTransportUaTcp)
	{
		OpcUa_String_Clear(pszTransportUaTcp);
		OpcUa_Free(pszTransportUaTcp);
		pszTransportUaTcp = OpcUa_Null;
	}
	if (pszTransportHttpsBinary)
	{
		OpcUa_String_Clear(pszTransportHttpsBinary);
		OpcUa_Free(pszTransportHttpsBinary);
		pszTransportHttpsBinary = OpcUa_Null;
	}
	if (pszTransportHttpsSoapBinary)
	{
		OpcUa_String_Clear(pszTransportHttpsSoapBinary);
		OpcUa_Free(pszTransportHttpsSoapBinary);
		pszTransportHttpsSoapBinary = OpcUa_Null;
	}
    if(pClientCredentials != OpcUa_Null)
    {
        OpcUa_Free(pClientCredentials);
    }

    /* lock the session until the request is sent */
#if OPCUA_USE_SYNCHRONISATION
    if(bLocked != OpcUa_False)
    {
        OpcUa_Mutex_Unlock(pChannel->Mutex);
    }
#endif

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_Channel_Connect
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Channel_Connect( OpcUa_Channel                               a_hChannel,
                                        const OpcUa_CharA*                          a_sUrl,
                                        const OpcUa_CharA*                          a_sTransportProfileUri,
                                        OpcUa_Channel_PfnConnectionStateChanged*    a_pfCallback,
                                        OpcUa_Void*                                 a_pvCallbackData,
                                        OpcUa_ByteString*                           a_pClientCertificate,
                                        OpcUa_ByteString*                           a_pClientPrivateKey,
                                        OpcUa_ByteString*                           a_pServerCertificate,
                                        OpcUa_Void*                                 a_pPKIConfig,
                                        OpcUa_String*                               a_pRequestedSecurityPolicyUri,
                                        OpcUa_Int32                                 a_nRequestedLifetime,
                                        OpcUa_MessageSecurityMode                   a_messageSecurityMode,
                                        OpcUa_Channel_SecurityToken**               a_ppSecurityToken,
                                        OpcUa_UInt32                                a_nNetworkTimeout)
{
    OpcUa_AsyncCallState*   pAsyncState      = OpcUa_Null;

#if OPCUA_MULTITHREADED
    OpcUa_InternalChannel*  pInternalChannel = (OpcUa_InternalChannel*)(a_hChannel);
#endif /* OPCUA_MULTITHREADED */

OpcUa_InitializeStatus(OpcUa_Module_Channel, "Connect");

    OpcUa_ReturnErrorIfArgumentNull(a_hChannel);
    OpcUa_ReturnErrorIfArgumentNull(a_sUrl);
    OpcUa_ReturnErrorIfArgumentNull(a_ppSecurityToken);

    /* create waithandle */
    uStatus =  OpcUa_AsyncCallState_Create(a_hChannel, OpcUa_Null, OpcUa_Null, &pAsyncState);
    OpcUa_ReturnErrorIfBad(uStatus);

#if OPCUA_MULTITHREADED && OPCUA_HAVE_HTTPS
    OpcUa_Mutex_Lock(pInternalChannel->Mutex);
    pInternalChannel->pfTempCallback        = a_pfCallback;
    pInternalChannel->pvTempCallbackData    = a_pvCallbackData;
    OpcUa_Mutex_Unlock(pInternalChannel->Mutex);
#endif

    /* call the async connect */
    uStatus = OpcUa_Channel_BeginConnect(   a_hChannel,
                                            a_sUrl,
                                            a_sTransportProfileUri,
                                            a_pClientCertificate,
                                            a_pClientPrivateKey,
                                            a_pServerCertificate,
                                            a_pPKIConfig,
                                            a_pRequestedSecurityPolicyUri,
                                            a_nRequestedLifetime,
                                            a_messageSecurityMode,
                                            a_nNetworkTimeout,
#if OPCUA_USE_SYNCHRONISATION
                                            OpcUa_Channel_InternalConnectComplete,
#else /* OPCUA_USE_SYNCHRONISATION */
                                            OpcUa_Null,
#endif /* OPCUA_USE_SYNCHRONISATION */
                                            (OpcUa_Void*)pAsyncState);
    OpcUa_GotoErrorIfBad(uStatus);

    /* ************************ wait for completion ************************** */
#if OPCUA_MULTITHREADED

    uStatus = OpcUa_AsyncCallState_WaitForCompletion(   pAsyncState,
                                                        a_nNetworkTimeout);

    if(OpcUa_IsEqual(OpcUa_BadTimeout))
    {
        /* this error was created locally. connect may happen and we need to reset the pipe */
        OpcUa_Channel_Disconnect(a_hChannel);
    }

    *a_ppSecurityToken = (OpcUa_Channel_SecurityToken*)pAsyncState->ResponseData;

    OpcUa_AsyncCallState_Delete(&pAsyncState);

    OpcUa_Mutex_Lock(pInternalChannel->Mutex);

#if OPCUA_HAVE_HTTPS
    pInternalChannel->pfTempCallback        = OpcUa_Null;
    pInternalChannel->pvTempCallbackData    = OpcUa_Null;
#endif

    pInternalChannel->pfCallback            = a_pfCallback;
    pInternalChannel->pvCallbackData        = a_pvCallbackData;

    OpcUa_Mutex_Unlock(pInternalChannel->Mutex);

#else

    OpcUa_ReferenceParameter(a_pfCallback);
    OpcUa_ReferenceParameter(a_pvCallbackData);

    uStatus = OpcUa_GoodCompletesAsynchronously;

#endif
    /* ********************** end wait for completion ************************ */

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    /* destroy async handle */
    if(pAsyncState != OpcUa_Null)
    {
        OpcUa_AsyncCallState_Delete(&pAsyncState);
    }

OpcUa_FinishErrorHandling;
}


/*============================================================================
 * OpcUa_Channel_SetUrl
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_Channel_SetUrl( OpcUa_Channel a_pChannel,
                                                    OpcUa_StringA a_sUrl)
{
OpcUa_InitializeStatus(OpcUa_Module_Channel, "SetUrl");

    OpcUa_GotoErrorIfArgumentNull(a_pChannel);
    OpcUa_GotoErrorIfArgumentNull(a_sUrl);

    OpcUa_String_Clear(&(((OpcUa_InternalChannel*)a_pChannel)->Url));

    uStatus = OpcUa_String_AttachToString(  a_sUrl,
                                            OPCUA_STRINGLENZEROTERMINATED,
                                            0,
                                            OpcUa_True,
                                            OpcUa_True,
                                            &(((OpcUa_InternalChannel*)a_pChannel)->Url));
    OpcUa_GotoErrorIfBad(uStatus);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_Channel_SetNamespaceUris
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Channel_SetNamespaceUris(OpcUa_Channel   a_hChannel,
                                                OpcUa_StringA*  a_psNamespaceUris,
                                                OpcUa_Boolean   a_bMakeCopy)
{
    OpcUa_InternalChannel* pChannel = (OpcUa_InternalChannel*)a_hChannel;

OpcUa_InitializeStatus(OpcUa_Module_ProxyStub, "SetNamespaceUris");

    OpcUa_ReturnErrorIfArgumentNull(a_hChannel);

    /* discard existing strings */
    OpcUa_StringTable_Clear(&pChannel->NamespaceUris);

    /* update table */
    uStatus = OpcUa_StringTable_AddStringList(  &pChannel->NamespaceUris,
                                                a_psNamespaceUris,
                                                a_bMakeCopy);
    OpcUa_GotoErrorIfBad(uStatus);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

#endif /* OPCUA_HAVE_CLIENTAPI */
