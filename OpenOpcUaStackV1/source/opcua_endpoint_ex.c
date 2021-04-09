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

/* platform */
#include <opcua.h>

#ifdef OPCUA_HAVE_SERVERAPI

/* core */
#include <opcua_mutex.h>

/* types */
#include <opcua_types.h>

/* serializing */
#include <opcua_binaryencoder.h>

/* communication */
#include <opcua_securelistener.h>
#include <opcua_tcplistener.h>

/* self */
#include <opcua_endpoint_ex.h>
#include <opcua_servicetable.h>
#include <opcua_endpoint_internal.h>

/*============================================================================
 * Extern Callback
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Endpoint_OnNotify(
    OpcUa_Listener*                                     pListener,
    OpcUa_Void*                                         pCallbackData,
    OpcUa_ListenerEvent                                 eEvent,
    OpcUa_Handle                                        hConnection,
    OpcUa_InputStream**                                 ppIstrm,
    OpcUa_StatusCode                                    uOperationStatus);

OpcUa_StatusCode OpcUa_Endpoint_OnSecureChannelEvent(   
    OpcUa_UInt32                                        uSecureChannelId,
    OpcUa_SecureListener_SecureChannelEvent             eSecureChannelEvent,
    OpcUa_StatusCode                                    uStatus,
    OpcUa_ByteString*                                   pbsClientCertificate,
    OpcUa_String*                                       sSecurityPolicy,
    OpcUa_UInt16                                        uMessageSecurityModes,
    OpcUa_UInt32                                        uRequestedLifetime,
    OpcUa_Void*                                         pCallbackData);

/*============================================================================
 ** @brief Global table of known types.
 *===========================================================================*/
extern OpcUa_EncodeableTypeTable OpcUa_ProxyStub_g_EncodeableTypes;

/*============================================================================
 ** @brief Global table of supported namespaces.
 *===========================================================================*/
extern OpcUa_StringTable OpcUa_ProxyStub_g_NamespaceUris;

/*============================================================================
 * OpcUa_Endpoint_Open
 *===========================================================================*/

OpcUa_StatusCode OpcUa_Endpoint_OpenEx( OpcUa_Endpoint                              a_hEndpoint,
                                        OpcUa_StringA                               a_sUrl,
                                        OpcUa_Endpoint_PfnEndpointCallback          a_pfEndpointCallback,
                                        OpcUa_Void*                                 a_pvEndpointCallbackData,
                                        OpcUa_ByteString*                           a_pServerCertificate,
                                        OpcUa_Key*                                  a_pPrivateKey,
                                        OpcUa_Void*                                 a_pPKIConfig,
                                        OpcUa_UInt32                                a_nNoOfSecurityPolicies,
                                        OpcUa_Endpoint_SecurityPolicyConfiguration* a_pSecurityPolicies,
                                        OpcUa_Listener_PfnOnNotify*                 a_pfListenerCallback,
                                        OpcUa_Void*                                 a_pListenerCallbackData)
{
    OpcUa_EndpointInternal* pEndpointInt = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_Endpoint, "Open");

    OpcUa_ReturnErrorIfArgumentNull(a_hEndpoint);

    OpcUa_ReferenceParameter(a_pfEndpointCallback);

    pEndpointInt = (OpcUa_EndpointInternal*)a_hEndpoint;

	OpcUa_String* pszEndPointHeader = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszEndPointHeader);
	OpcUa_String_AttachCopy(pszEndPointHeader, "opc.tcp:");

    /* acquire the lock on the endpoint structure */
    OpcUa_Mutex_Lock(pEndpointInt->Mutex);
        
    /* attach the given url to the endpoint (make copy) */
    OpcUa_String_AttachToString(    a_sUrl,
                                    OPCUA_STRINGLENZEROTERMINATED,
                                    0,
                                    OpcUa_True,
                                    OpcUa_False,
                                    &(pEndpointInt->Url));
    OpcUa_GotoErrorIfBad(uStatus);     

    /* create the encoder and decoder */
    if (pEndpointInt->EncoderType == OpcUa_EncoderType_Binary)
    {
        /* create encoder */
        uStatus = OpcUa_BinaryEncoder_Create(&pEndpointInt->Encoder);
        OpcUa_GotoErrorIfBad(uStatus);

        /* create decoder */
        uStatus = OpcUa_BinaryDecoder_Create(&pEndpointInt->Decoder);
        OpcUa_GotoErrorIfBad(uStatus);
    }
    else /* the encoding type is not supported */
    {
        OpcUa_GotoErrorWithStatus(OpcUa_BadNotSupported);
    }

    /* select the connect type based on the url scheme */
    if(!OpcUa_String_StrnCmp(   &(pEndpointInt->Url), pszEndPointHeader,
                                OpcUa_String_StrLen(pszEndPointHeader), 
                                OpcUa_True))
    {
        uStatus = OpcUa_TcpListener_Create(     a_pServerCertificate,
                                                a_pPrivateKey,
                                                a_pPKIConfig,
                                                &pEndpointInt->TransportListener);
        OpcUa_GotoErrorIfBad(uStatus);

        uStatus = OpcUa_SecureListener_Create(  pEndpointInt->TransportListener, 
                                                pEndpointInt->Decoder,
                                                pEndpointInt->Encoder,
                                                &OpcUa_ProxyStub_g_NamespaceUris,
                                                &OpcUa_ProxyStub_g_EncodeableTypes,
                                                a_pServerCertificate,
                                                a_pPrivateKey,
                                                a_pPKIConfig,
                                                a_nNoOfSecurityPolicies,
                                                (OpcUa_SecureListener_SecurityPolicyConfiguration*)a_pSecurityPolicies,
                                                OpcUa_Endpoint_OnSecureChannelEvent,
                                                (OpcUa_Void*)pEndpointInt,
                                                &pEndpointInt->SecureListener);

        OpcUa_GotoErrorIfBad(uStatus);
    }

    /* the endpoint type is not supported */
    else
    {
        OpcUa_GotoErrorWithStatus(OpcUa_BadNotSupported);
    }

    pEndpointInt->Status = OpcUa_BadWaitingForResponse;

    if (a_pfListenerCallback == OpcUa_Null)
    {
        a_pfListenerCallback    = OpcUa_Endpoint_OnNotify;
        a_pListenerCallbackData = a_hEndpoint;
    }

    /* open the endpoint. */
    uStatus = OpcUa_Listener_Open(  
        pEndpointInt->SecureListener, 
        &(pEndpointInt->Url), 
        a_pfListenerCallback, 
        a_pListenerCallbackData);

    OpcUa_GotoErrorIfBad(uStatus);

    pEndpointInt->pfEndpointCallback     = a_pfEndpointCallback;
    pEndpointInt->pvEndpointCallbackData = a_pvEndpointCallbackData;

    /* release lock */
    OpcUa_Mutex_Unlock(pEndpointInt->Mutex);

	if (pszEndPointHeader)
	{
		OpcUa_String_Clear(pszEndPointHeader);
		OpcUa_Free(pszEndPointHeader);
		pszEndPointHeader = OpcUa_Null;
	}

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    OpcUa_Listener_Delete(&pEndpointInt->TransportListener);
    OpcUa_Listener_Delete(&pEndpointInt->SecureListener);
    OpcUa_Encoder_Delete(&pEndpointInt->Encoder);
    OpcUa_Decoder_Delete(&pEndpointInt->Decoder);
    OpcUa_Mutex_Unlock(pEndpointInt->Mutex);
	if (pszEndPointHeader)
	{
		OpcUa_String_Clear(pszEndPointHeader);
		OpcUa_Free(pszEndPointHeader);
		pszEndPointHeader = OpcUa_Null;
	}
OpcUa_FinishErrorHandling;
}


/*============================================================================
 * OpcUa_Endpoint_BeginSendEncodedResponse
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Endpoint_BeginSendEncodedResponse( 
    OpcUa_Endpoint       a_hEndpoint,
    OpcUa_Handle         a_pConnection,
    OpcUa_InputStream**  a_ppIstrm,
    OpcUa_OutputStream** a_ppOstrm)
{
    OpcUa_EndpointInternal* pEndpointInt = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_Endpoint, "BeginSendEncodedRespons");

    /* check arguments */
    OpcUa_ReturnErrorIfArgumentNull(a_hEndpoint);
    OpcUa_ReturnErrorIfArgumentNull(a_pConnection);
    OpcUa_ReturnErrorIfArgumentNull(a_ppOstrm);

    *a_ppOstrm = OpcUa_Null;

    pEndpointInt = (OpcUa_EndpointInternal*)a_hEndpoint;

    /* acquire the lock on the endpoint structure */
    OpcUa_Mutex_Lock(pEndpointInt->Mutex);

    pEndpointInt = (OpcUa_EndpointInternal*)a_hEndpoint;

    /* Initialize Response Stream */
    uStatus = OpcUa_Listener_BeginSendResponse( 
        pEndpointInt->SecureListener, 
        a_pConnection, 
        a_ppIstrm, 
        a_ppOstrm);

    OpcUa_GotoErrorIfBad(uStatus);

    /* release lock */
    OpcUa_Mutex_Unlock(pEndpointInt->Mutex);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    /* release lock */
    OpcUa_Mutex_Unlock(pEndpointInt->Mutex);

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_Endpoint_EndSendEncodedResponse
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Endpoint_EndSendEncodedResponse( 
    OpcUa_Endpoint       a_hEndpoint,
    OpcUa_StatusCode     a_uStatus,
    OpcUa_OutputStream** a_ppOstrm)
{
    OpcUa_EndpointInternal* pEndpointInt = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_Endpoint, "EndSendEncodedResponse");

    /* check arguments */
    OpcUa_ReturnErrorIfArgumentNull(a_hEndpoint);
    OpcUa_ReturnErrorIfArgumentNull(a_ppOstrm);
    OpcUa_ReturnErrorIfArgumentNull(*a_ppOstrm);

    pEndpointInt = (OpcUa_EndpointInternal*)a_hEndpoint;

    /* acquire the lock on the endpoint structure */
    OpcUa_Mutex_Lock(pEndpointInt->Mutex);

    pEndpointInt = (OpcUa_EndpointInternal*)a_hEndpoint;
        
    uStatus = OpcUa_Listener_EndSendResponse( 
        pEndpointInt->SecureListener, 
        a_uStatus, 
        a_ppOstrm);

    OpcUa_GotoErrorIfBad(uStatus);

    /* release lock */
    OpcUa_Mutex_Unlock(pEndpointInt->Mutex);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
  
    /* release lock */
    OpcUa_Mutex_Unlock(pEndpointInt->Mutex);

OpcUa_FinishErrorHandling;
}

#endif /* OPCUA_HAVE_SERVERAPI */
