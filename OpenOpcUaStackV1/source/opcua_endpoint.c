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

#if OPCUA_HAVE_SOAPHTTP
#include <opcua_httplistener_securitystub.h>
#endif /* OPCUA_HAVE_SOAPHTTP */

#if OPCUA_HAVE_HTTPS
#include <opcua_https_listener.h>
#endif /* OPCUA_HAVE_HTTPS */

/* self */
#include <opcua_endpoint.h>
#include <opcua_servicetable.h>
#include <opcua_endpoint_internal.h>

static OpcUa_UInt32 PublishCounter=0;
static OpcUa_UInt32 PublishRequestCounter = 0;
static OpcUa_UInt32 PublishResponseCounter = 0;
/** @brief If yes and possible, a service fault is created instead of a stack error. */
#define OPCUA_ENDPOINT_USE_SERVICE_FAULT            OPCUA_CONFIG_YES

/**
 * @brief Processes a request recieved on an endpoint.
 *
 * @param hEndpoint   [in] The endpoint which received the request.
 * @param pConnection [in] The connection which is the source of the request.
 * @param pIstrm      [in] The stream used to read the request.
 */
static OpcUa_StatusCode OpcUa_Endpoint_BeginProcessRequest(
	OpcUa_Endpoint              hEndpoint,
	OpcUa_Handle                pConnection,
	struct _OpcUa_InputStream** ppIstrm);

/**
 * @brief Processes a request recieved on an endpoint.
 *
 * @param hEndpoint   [in] The endpoint which received the request.
 * @param pConnection [in] The connection which is the source of the request.
 * @param pIstrm      [in] The stream used to read the request.
 */
static OpcUa_StatusCode OpcUa_Endpoint_BeginProcessRawRequest(
	OpcUa_Endpoint              hEndpoint,
	OpcUa_Handle                pConnection,
	struct _OpcUa_InputStream** ppIstrm);

/**
 * @brief Returns a pointer to the function that implements the service.
 *
 * @param hEndpoint [in]  The endpoint which received the request.
 * @param hContext  [in]  The context to for a request.
 * @param ppInvoke  [out] A pointer to the service function.
 */
OpcUa_StatusCode OpcUa_Endpoint_GetServiceFunction(
	OpcUa_Endpoint           hEndpoint,
	OpcUa_Handle             hContext,
	OpcUa_PfnInvokeService** ppInvoke);

///*============================================================================
// * OpcUa_EndpointContext
// *===========================================================================*/
///**
//  @brief Stores the context for a request received from a client.
//*/
//struct _OpcUa_EndpointContext
//{
//    /** @brief The stream to use to write the response to. */
//    OpcUa_InputStream*      pIstrm;
//
//    /** @brief The stream to use to write the response to. */
//    OpcUa_Handle            hConnection;
//
//    /** @brief The stream to use to write the response to. */
//    OpcUa_OutputStream*     pOstrm;
//
//    /** @brief The service definition associated with the request. */
//    OpcUa_ServiceType       ServiceType;
//
//    /** @brief The id of the corresponding securechannel. */
//    OpcUa_UInt32            uSecureChannelId;
//};
//
//typedef struct _OpcUa_EndpointContext OpcUa_EndpointContext;

/*============================================================================
 * OpcUa_SupportedServiceTypes
 *===========================================================================*/
extern struct _OpcUa_ServiceType* OpcUa_SupportedServiceTypes[];

/*============================================================================
 ** @brief Global table of known types.
 *===========================================================================*/
extern OpcUa_EncodeableTypeTable OpcUa_ProxyStub_g_EncodeableTypes;

/*============================================================================
 ** @brief Global table of supported namespaces.
 *===========================================================================*/
extern OpcUa_StringTable OpcUa_ProxyStub_g_NamespaceUris;

/*============================================================================
 * OpcUa_Endpoint_Create
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Endpoint_Create( OpcUa_Endpoint*                     a_phEndpoint,
										OpcUa_Endpoint_SerializerType       a_eSerializerType,
										OpcUa_ServiceType**                 a_pSupportedServices)
{
	OpcUa_EndpointInternal* pEndpointInt            = OpcUa_Null;
	OpcUa_ServiceType**     pSupportedServiceTypes  = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_Endpoint, "Create");

	OpcUa_ReturnErrorIfArgumentNull(a_phEndpoint);

	if(a_pSupportedServices != OpcUa_Null)
	{
		pSupportedServiceTypes = a_pSupportedServices;
	}
	else
	{
		pSupportedServiceTypes = &OpcUa_SupportedServiceTypes[0];
	}

	*a_phEndpoint = OpcUa_Null;

	/* increment counter */
	OpcUa_ProxyStub_RegisterEndpoint();

	pEndpointInt = (OpcUa_EndpointInternal*)OpcUa_Alloc(sizeof(OpcUa_EndpointInternal));
	OpcUa_GotoErrorIfAllocFailed(pEndpointInt);
	OpcUa_MemSet(pEndpointInt, 0, sizeof(OpcUa_EndpointInternal));

	pEndpointInt->State = eOpcUa_Endpoint_State_Closed;

	/* initialize serializer */
	switch(a_eSerializerType)
	{
	case OpcUa_Endpoint_SerializerType_Binary:
		{
			pEndpointInt->EncoderType = OpcUa_EncoderType_Binary;
			break;
		}
	case OpcUa_Endpoint_SerializerType_Xml:
		{
			uStatus = OpcUa_BadNotImplemented;
			/* pEndpointInt->EncoderType = OpcUa_EncoderType_Xml; */
			break;
		}
	default:
		{
			uStatus = OpcUa_BadInvalidArgument;
		}
	}
	OpcUa_GotoErrorIfBad(uStatus);

	uStatus = OpcUa_Mutex_Create(&(pEndpointInt->Mutex));
	OpcUa_GotoErrorIfBad(uStatus);

	/* initialize supported services */
	uStatus = OpcUa_ServiceTable_AddTypes(&(pEndpointInt->SupportedServices), pSupportedServiceTypes);
	OpcUa_GotoErrorIfBad(uStatus);

	*a_phEndpoint = pEndpointInt;

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	OpcUa_Endpoint_Delete(a_phEndpoint);

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_Endpoint_GetMessageSecureChannelId
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Endpoint_GetMessageSecureChannelId(  OpcUa_Endpoint  a_hEndpoint,
															OpcUa_Handle    a_hContext,
															OpcUa_UInt32*   a_pSecureChannelId)
{
	OpcUa_EndpointContext* pContext = (OpcUa_EndpointContext*)a_hContext;

	OpcUa_ReturnErrorIfArgumentNull(a_hEndpoint);
	OpcUa_ReturnErrorIfArgumentNull(a_hContext);
	OpcUa_ReturnErrorIfArgumentNull(a_pSecureChannelId);

	*a_pSecureChannelId = pContext->uSecureChannelId;

	return OpcUa_Good;
}

/*============================================================================
 * OpcUa_Endpoint_GetMessageSecureChannelSecurityPolicy
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Endpoint_GetMessageSecureChannelSecurityPolicy(
	OpcUa_Endpoint                                  a_hEndpoint,
	OpcUa_Handle                                    a_hContext,
	OpcUa_Endpoint_SecurityPolicyConfiguration*     a_pSecurityPolicy)
{
	OpcUa_EndpointInternal* pEndpointInt    = (OpcUa_EndpointInternal*)a_hEndpoint;
	OpcUa_EndpointContext*  pContext        = (OpcUa_EndpointContext*)a_hContext;

	OpcUa_ReturnErrorIfArgumentNull(a_hEndpoint);
	OpcUa_ReturnErrorIfArgumentNull(a_hContext);
	OpcUa_ReturnErrorIfArgumentNull(a_pSecurityPolicy);
	OpcUa_ReturnErrorIfArgumentNull(pEndpointInt->SecureListener);

	return OpcUa_Listener_GetSecurityPolicyConfiguration(   pEndpointInt->SecureListener,
															pContext->pIstrm,
															(OpcUa_Listener_SecurityPolicyConfiguration*)a_pSecurityPolicy);
}

/*============================================================================
 * OpcUa_Endpoint_Delete
 *===========================================================================*/
OpcUa_Void OpcUa_Endpoint_Delete(OpcUa_Endpoint* a_phEndpoint)
{
	if(a_phEndpoint != OpcUa_Null && *a_phEndpoint != OpcUa_Null)
	{
		OpcUa_EndpointInternal* pEndpointInt = (OpcUa_EndpointInternal*)*a_phEndpoint;
		*a_phEndpoint = OpcUa_Null;

		OpcUa_Mutex_Lock(pEndpointInt->Mutex);
		OpcUa_Listener_Delete(&pEndpointInt->TransportListener);
		OpcUa_Listener_Delete(&pEndpointInt->SecureListener);
		OpcUa_Encoder_Delete(&pEndpointInt->Encoder);
		OpcUa_Decoder_Delete(&pEndpointInt->Decoder);
		OpcUa_String_Clear(&pEndpointInt->Url);
		OpcUa_ServiceTable_Clear(&pEndpointInt->SupportedServices);

		OpcUa_Mutex_Unlock(pEndpointInt->Mutex);

		OpcUa_Mutex_Delete(&(pEndpointInt->Mutex));

		OpcUa_Free(pEndpointInt);

		 /* decrement counter */
		OpcUa_ProxyStub_DeRegisterEndpoint();
	}
}

/*============================================================================
 * OpcUa_Endpoint_OnNotify
 *==========================================================================*/
OpcUa_StatusCode OpcUa_Endpoint_OnNotify(   OpcUa_Listener*     a_pListener,
											OpcUa_Void*         a_pCallbackData,
											OpcUa_ListenerEvent a_eEvent,
											OpcUa_Handle        a_hConnection,
											OpcUa_InputStream** a_ppIstrm,
											OpcUa_StatusCode    a_uOperationStatus)
{
	OpcUa_EndpointInternal* pEndpointInt = (OpcUa_EndpointInternal*)a_pCallbackData;

OpcUa_InitializeStatus(OpcUa_Module_Endpoint, "OnNotify");

	OpcUa_ReturnErrorIfArgumentNull(a_pListener);
	OpcUa_ReturnErrorIfArgumentNull(a_pCallbackData);

	/* acquire the lock on the endpoint structure */
	OpcUa_Mutex_Lock(pEndpointInt->Mutex);

	switch(a_eEvent)
	{
	case OpcUa_ListenerEvent_Open:
		{
			pEndpointInt->Status = a_uOperationStatus;
			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Endpoint_OnNotify: Underlying listener raised open event!\n");
			break;
		}
	case OpcUa_ListenerEvent_Close:
		{
			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Endpoint_OnNotify: Underlying listener raised close event!\n");
			break;
		}
	case OpcUa_ListenerEvent_Request:
		{
			if(pEndpointInt->State == eOpcUa_Endpoint_State_Closed)
			{
				OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Endpoint_OnNotify: Shutting down! Ignoring request!\n");
				OpcUa_GotoErrorWithStatus(OpcUa_BadShutdown);
			}

			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Endpoint_OnNotify: Underlying listener raised Request event!\n");

			uStatus = OpcUa_Endpoint_BeginProcessRequest(
				pEndpointInt,   /* the endpoint holding the service table */
				a_hConnection,  /* the connection object for identifiyng the communication partner */
				a_ppIstrm);      /* the stream for reading the request */
			OpcUa_GotoErrorIfBad(uStatus);

			break;
		}
	case OpcUa_ListenerEvent_RawRequest:
		{
			if(pEndpointInt->State == eOpcUa_Endpoint_State_Closed)
			{
				OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Endpoint_OnNotify: Shutting down! Ignoring request!\n");
				OpcUa_GotoErrorWithStatus(OpcUa_BadShutdown);
			}

			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Endpoint_OnNotify: Underlying listener raised RawRequest event!\n");

			uStatus = OpcUa_Endpoint_BeginProcessRawRequest(
				pEndpointInt,
				a_hConnection,
				a_ppIstrm);

			OpcUa_GotoErrorIfBad(uStatus);

			break;
		}
	case OpcUa_ListenerEvent_UnexpectedError:
		{
			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Endpoint_OnNotify: Underlying listener raised UnexpectedError event!\n");
			break;
		}
	case OpcUa_ListenerEvent_Invalid:
		{
			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Endpoint_OnNotify: Underlying listener raised invalid event!\n");
			break;
		}
	case OpcUa_ListenerEvent_ChannelOpened:
		{
			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Endpoint_OnNotify: Underlying listener raised ChannelOpened event!\n");
			break;
		}
	case OpcUa_ListenerEvent_ChannelClosed:
		{
			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Endpoint_OnNotify: Underlying listener raised ChannelClosed event!\n");
			break;
		}
	case OpcUa_ListenerEvent_RequestPartial:
		{
			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Endpoint_OnNotify: Underlying listener raised RequestPartial event!\n");
			break;
		}
	case OpcUa_ListenerEvent_RequestAbort:
		{
			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Endpoint_OnNotify: Underlying listener raised RequestAbort event!\n");
			break;
		}
	case OpcUa_ListenerEvent_AsyncWriteComplete:
		{
			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Endpoint_OnNotify: Underlying listener raised AsyncWriteComplete event!\n");
			break;
		}
	default:
		{
			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Endpoint_OnNotify: Underlying listener raised unknown event!\n");
		}
	}

	OpcUa_Mutex_Unlock(pEndpointInt->Mutex);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	OpcUa_Mutex_Unlock(pEndpointInt->Mutex);

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_Endpoint_OnNotify
 *==========================================================================*/
OpcUa_StatusCode OpcUa_Endpoint_OnSecureChannelEvent(   OpcUa_UInt32                            a_uSecureChannelId,
														OpcUa_SecureListener_SecureChannelEvent a_eSecureChannelEvent,
														OpcUa_StatusCode                        a_uStatus,
														OpcUa_ByteString*                       a_pbsClientCertificate,
														OpcUa_String*                           a_sSecurityPolicy,
														OpcUa_UInt16                            a_uMessageSecurityModes,
														OpcUa_UInt32                            a_uRequestedLifetime,
														OpcUa_Void*                             a_pCallbackData)
{
	OpcUa_EndpointInternal* pEndpoint = OpcUa_Null;
	OpcUa_StatusCode uStatus=OpcUa_Good;

	/* get current endpoint */
	if(a_pCallbackData != OpcUa_Null)
	{
		pEndpoint = (OpcUa_EndpointInternal*)a_pCallbackData;

		/* call the endpoints callback function */
		if(pEndpoint->pfEndpointCallback)
		{
			switch(a_eSecureChannelEvent)
			{
				case eOpcUa_SecureListener_SecureChannelOpen:
				{
					/* the .NET wrapper needs to have the security policy information passed when the channel
					   is created because it does have access to the endpoint context that normal applications use */
					OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Endpoint_OnSecureChannelEvent: SecureChannel opened!\n");
					uStatus = pEndpoint->pfEndpointCallback(
						pEndpoint,
						pEndpoint->pvEndpointCallbackData,
						eOpcUa_Endpoint_Event_SecureChannelOpened,
						a_uStatus,
						a_uSecureChannelId,
						OpcUa_Null,
						a_pbsClientCertificate,
						a_sSecurityPolicy,
						a_uMessageSecurityModes,
						a_uRequestedLifetime);

					break;
				}
				case eOpcUa_SecureListener_SecureChannelClose:
				{
					OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Endpoint_OnSecureChannelEvent: SecureChannel closed!\n");
					uStatus=pEndpoint->pfEndpointCallback(
						pEndpoint,
						pEndpoint->pvEndpointCallbackData,
						eOpcUa_Endpoint_Event_SecureChannelClosed,
						a_uStatus,
						a_uSecureChannelId,
						OpcUa_Null,
						OpcUa_Null,
						OpcUa_Null,
						0,
						0);

					
					break;
				}
				case eOpcUa_SecureListener_SecureChannelRenew:
				{
					OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Endpoint_OnSecureChannelEvent: SecureChannel renewed!\n");
					uStatus = pEndpoint->pfEndpointCallback(
						pEndpoint,
						pEndpoint->pvEndpointCallbackData,
						eOpcUa_Endpoint_Event_SecureChannelRenewed,
						a_uStatus,
						a_uSecureChannelId,
						OpcUa_Null,
						OpcUa_Null,
						OpcUa_Null,
						0,
						a_uRequestedLifetime);
					
					break;
				}
				case eOpcUa_SecureListener_SecureChannelOpenVerifyCertificate:
				{
					OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Endpoint_OnSecureChannelEvent: SecureChannel open certificate verification request!\n");
					uStatus = pEndpoint->pfEndpointCallback(
						pEndpoint,
						pEndpoint->pvEndpointCallbackData,
						eOpcUa_Endpoint_Event_SecureChannelOpenVerifyCertificate,
						a_uStatus,
						a_uSecureChannelId,
						OpcUa_Null,
						a_pbsClientCertificate,
						OpcUa_Null,
						0,
						0);

					break;
				}
				case eOpcUa_SecureListener_SecureChannelRenewVerifyCertificate:
				{
					OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Endpoint_OnSecureChannelEvent: SecureChannel renew certificate verification request!\n");
					uStatus = pEndpoint->pfEndpointCallback(
						pEndpoint,
						pEndpoint->pvEndpointCallbackData,
						eOpcUa_Endpoint_Event_SecureChannelRenewVerifyCertificate,
						a_uStatus,
						a_uSecureChannelId,
						OpcUa_Null,
						a_pbsClientCertificate,
						OpcUa_Null,
						0,
						0);

					break;
				}
				case eOpcUa_SecureListener_SecureChannelLostTransportConnection:
				{
					OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Endpoint_OnSecureChannelEvent: SecureChannel lost transport connection!\n");
					pEndpoint->pfEndpointCallback(
						pEndpoint,
						pEndpoint->pvEndpointCallbackData,
						eOpcUa_Endpoint_Event_TransportConnectionClosed,
						a_uStatus,
						a_uSecureChannelId,
						OpcUa_Null,
						OpcUa_Null,
						OpcUa_Null,
						0,
						0);
					break;
				}
				default:
				{
					OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Endpoint_OnSecureChannelEvent: unknown SecureChannel event!\n");
					pEndpoint->pfEndpointCallback(
						pEndpoint,
						pEndpoint->pvEndpointCallbackData,
						eOpcUa_Endpoint_Event_Invalid,
						a_uStatus,
						a_uSecureChannelId,
						OpcUa_Null,
						OpcUa_Null,
						OpcUa_Null,
						0,
						0);
					
					break;
				}
			}
		}
	}

	return OpcUa_Good;
}

/*============================================================================
 * OpcUa_Endpoint_Open
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Endpoint_Open(   OpcUa_Endpoint                              a_hEndpoint,
										OpcUa_StringA                               a_sUrl,
										OpcUa_StringA                               a_sTransportProfileUri,
										OpcUa_Endpoint_PfnEndpointCallback*         a_pfEndpointCallback,
										OpcUa_Void*                                 a_pvEndpointCallbackData,
										OpcUa_ByteString*                           a_pServerCertificate,
										OpcUa_Key*                                  a_pServerPrivateKey,
										OpcUa_Void*                                 a_pPKIConfig,
										OpcUa_UInt32                                a_nNoOfSecurityPolicies,
										OpcUa_Endpoint_SecurityPolicyConfiguration* a_pSecurityPolicies)
{
	OpcUa_EndpointInternal* pEndpointInt = OpcUa_Null;
	OpcUa_String* szTcp = OpcUa_Null;
	OpcUa_String* szHttp = OpcUa_Null;
OpcUa_InitializeStatus(OpcUa_Module_Endpoint, "Open");

	OpcUa_ReturnErrorIfArgumentNull(a_hEndpoint);
	OpcUa_ReturnErrorIfArgumentNull(a_sUrl);
	OpcUa_ReturnErrorIfArgumentNull(a_sTransportProfileUri);

	OpcUa_String* pszTransportProfileUri = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszTransportProfileUri);
	OpcUa_String_AttachCopy(pszTransportProfileUri, a_sTransportProfileUri);

	pEndpointInt = (OpcUa_EndpointInternal*)a_hEndpoint;
	
	/* acquire the lock on the endpoint structure */
	OpcUa_Mutex_Lock(pEndpointInt->Mutex);

	pEndpointInt->State = eOpcUa_Endpoint_State_Open;
		
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
	szTcp = OpcUa_String_FromCString("opc.tcp:");
	if (szTcp)
	{
		if (!OpcUa_String_StrnCmp(&(pEndpointInt->Url),
			szTcp,
			(OpcUa_UInt32)8,
			OpcUa_True))
		{
			OpcUa_String* pszTransportProfileUaTcp = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
			OpcUa_String_Initialize(pszTransportProfileUaTcp);
			OpcUa_String_AttachCopy(pszTransportProfileUaTcp, OpcUa_TransportProfile_UaTcp);
			if (!OpcUa_String_StrnCmp(pszTransportProfileUri,pszTransportProfileUaTcp,OPCUA_STRING_LENDONTCARE,OpcUa_True))
			{
				uStatus = OpcUa_TcpListener_Create(a_pServerCertificate,
					a_pServerPrivateKey,
					a_pPKIConfig,
					&pEndpointInt->TransportListener);
				OpcUa_GotoErrorIfBad(uStatus);
			}
			else
			{
				OpcUa_GotoErrorWithStatus(OpcUa_BadNotSupported);
			}
			if (pszTransportProfileUaTcp)
			{
				OpcUa_String_Clear(pszTransportProfileUaTcp);
				OpcUa_Free(pszTransportProfileUaTcp);
				pszTransportProfileUaTcp = OpcUa_Null;
			}
		}		
#if OPCUA_HAVE_HTTPS || OPCUA_HAVE_SOAPHTTP
		else
		{
			szHttp = OpcUa_String_FromCString("http:");
			if (szHttp)
			{
				if (!OpcUa_String_StrnCmp(&(pEndpointInt->Url),
					szHttp,
					(OpcUa_UInt32)4,
					OpcUa_True))
				{
#if OPCUA_HAVE_HTTPS
					OpcUa_String* pszTransportProfileHttpsBinary = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
					OpcUa_String_Initialize(pszTransportProfileHttpsBinary);
					OpcUa_String_AttachCopy(pszTransportProfileHttpsBinary, OpcUa_TransportProfile_HttpsBinary);
					if (!OpcUa_String_StrnCmp(pszTransportProfileUri,pszTransportProfileHttpsBinary,OPCUA_STRING_LENDONTCARE,OpcUa_True))
					{
						uStatus = OpcUa_HttpsListener_Create(a_pServerCertificate,
							a_pServerPrivateKey,
							a_pPKIConfig,
							(OpcUa_HttpsListener_PfnSecureChannelCallback*)OpcUa_Endpoint_OnSecureChannelEvent,
							(OpcUa_Void*)pEndpointInt,
							&pEndpointInt->SecureListener);
						OpcUa_GotoErrorIfBad(uStatus);
					}
					else
					{
#endif /* OPCUA_HAVE_HTTPS */
						OpcUa_GotoErrorWithStatus(OpcUa_BadNotSupported);
					}
					if (pszTransportProfileHttpsBinary)
					{
						OpcUa_String_Clear(pszTransportProfileHttpsBinary);
						OpcUa_Free(pszTransportProfileHttpsBinary);
						pszTransportProfileHttpsBinary = OpcUa_Null;
					}
				}
#endif /* OPCUA_HAVE_HTTPS || OPCUA_HAVE_SOAPHTTP */
				/* the endpoint type is not supported */
				else
				{
					OpcUa_GotoErrorWithStatus(OpcUa_BadNotSupported);
				}
				OpcUa_String_Clear(szHttp);
			}
			else
				OpcUa_GotoErrorWithStatus(OpcUa_BadOutOfMemory);
		}
		OpcUa_String_Clear(szTcp);
		OpcUa_Free(szTcp);
	}
	else
	{
		OpcUa_GotoErrorWithStatus(OpcUa_BadOutOfMemory);
	}
	pEndpointInt->pfEndpointCallback        = a_pfEndpointCallback;
	pEndpointInt->pvEndpointCallbackData    = a_pvEndpointCallbackData;

	if(pEndpointInt->SecureListener == OpcUa_Null)
	{
		uStatus = OpcUa_SecureListener_Create(  pEndpointInt->TransportListener,
												pEndpointInt->Decoder,
												pEndpointInt->Encoder,
												&OpcUa_ProxyStub_g_NamespaceUris,
												&OpcUa_ProxyStub_g_EncodeableTypes,
												a_pServerCertificate,
												a_pServerPrivateKey,
												a_pPKIConfig,
												a_nNoOfSecurityPolicies,
												(OpcUa_SecureListener_SecurityPolicyConfiguration*)a_pSecurityPolicies,
												OpcUa_Endpoint_OnSecureChannelEvent,
												(OpcUa_Void*)pEndpointInt,
												&pEndpointInt->SecureListener);
	}
	OpcUa_GotoErrorIfBad(uStatus);

	pEndpointInt->Status = OpcUa_BadWaitingForResponse;

	/* open the endpoint. */
	uStatus = OpcUa_Listener_Open(  pEndpointInt->SecureListener,
									&(pEndpointInt->Url),
									OpcUa_Endpoint_OnNotify,
									(OpcUa_Void*)a_hEndpoint);

	OpcUa_GotoErrorIfBad(uStatus);

	/* release lock */
	OpcUa_Mutex_Unlock(pEndpointInt->Mutex);

	if (pszTransportProfileUri)
	{
		OpcUa_String_Clear(pszTransportProfileUri);
		OpcUa_Free(pszTransportProfileUri);
		pszTransportProfileUri = OpcUa_Null;
	}

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
	OpcUa_String_Clear(szTcp);
	OpcUa_String_Clear(szHttp);
	OpcUa_Listener_Delete(&pEndpointInt->TransportListener);
	OpcUa_Listener_Delete(&pEndpointInt->SecureListener);
	OpcUa_Encoder_Delete(&pEndpointInt->Encoder);
	OpcUa_Decoder_Delete(&pEndpointInt->Decoder);
	OpcUa_Mutex_Unlock(pEndpointInt->Mutex);

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_Endpoint_Close
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Endpoint_Close(OpcUa_Endpoint a_hEndpoint)
{
	OpcUa_EndpointInternal* pEndpointInt = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_Endpoint, "Close");

	OpcUa_ReturnErrorIfArgumentNull(a_hEndpoint);

	pEndpointInt = (OpcUa_EndpointInternal*)a_hEndpoint;

	/* acquire the lock on the endpoint structure */
	OpcUa_Mutex_Lock(pEndpointInt->Mutex);

	OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Endpoint_Close: Cleaning up!\n");

	pEndpointInt->State = eOpcUa_Endpoint_State_Closed;

	/* close listener */
	OpcUa_Mutex_Unlock(pEndpointInt->Mutex); /* unlocking for potential callbacks */
	uStatus = OpcUa_Listener_Close(pEndpointInt->SecureListener);
	OpcUa_Mutex_Lock(pEndpointInt->Mutex);
	OpcUa_GotoErrorIfBad(uStatus);

	OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Endpoint_Close: Done!\n");

	/* release lock */
	OpcUa_Mutex_Unlock(pEndpointInt->Mutex);


OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	OpcUa_Mutex_Unlock(pEndpointInt->Mutex);

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_Endpoint_ReadRequest
 *===========================================================================*/
static OpcUa_StatusCode OpcUa_Endpoint_ReadRequest(
	OpcUa_Endpoint          a_hEndpoint,
	OpcUa_InputStream*      a_pIstrm,
	OpcUa_Void**            a_ppRequest,
	OpcUa_EncodeableType**  a_ppRequestType)
{
	OpcUa_Decoder*          pDecoder    = OpcUa_Null;
	OpcUa_MessageContext    cContext;
	OpcUa_Handle            hDecodeContext = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_Endpoint, "ReadRequest");

	OpcUa_ReturnErrorIfArgumentNull(a_hEndpoint);
	OpcUa_ReturnErrorIfArgumentNull(a_pIstrm);
	OpcUa_ReturnErrorIfArgumentNull(a_ppRequest);
	OpcUa_ReturnErrorIfArgumentNull(a_ppRequestType);

	pDecoder = ((OpcUa_EndpointInternal*)a_hEndpoint)->Decoder;
	OpcUa_ReturnErrorIfArgumentNull(pDecoder);

	*a_ppRequest     = OpcUa_Null;
	*a_ppRequestType = OpcUa_Null;

	/* initialize context */
	OpcUa_MessageContext_Initialize(&cContext);

	cContext.KnownTypes    = &OpcUa_ProxyStub_g_EncodeableTypes;
	cContext.NamespaceUris = &OpcUa_ProxyStub_g_NamespaceUris;

	/* create decoder */
	uStatus = pDecoder->Open(pDecoder, a_pIstrm, &cContext, &hDecodeContext);
	OpcUa_GotoErrorIfBad(uStatus);

	/* decode message */
	uStatus = pDecoder->ReadMessage((struct _OpcUa_Decoder *)hDecodeContext, a_ppRequestType, a_ppRequest);
	OpcUa_GotoErrorIfBad(uStatus);

	/* close decoder */
	OpcUa_Decoder_Close(pDecoder, &hDecodeContext);
	OpcUa_MessageContext_Clear(&cContext);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	OpcUa_Decoder_Close(pDecoder, &hDecodeContext);
	OpcUa_MessageContext_Clear(&cContext);
	OpcUa_EncodeableObject_Delete(*a_ppRequestType, a_ppRequest);

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_Endpoint_WriteResponse
 *===========================================================================*/
static OpcUa_StatusCode OpcUa_Endpoint_WriteResponse(
	OpcUa_Endpoint          a_hEndpoint,
	OpcUa_OutputStream**    a_ppOstrm,
	OpcUa_StatusCode        a_uStatus,
	OpcUa_Void*             a_pResponse,
	OpcUa_EncodeableType*   a_pResponseType)
{
	OpcUa_EndpointInternal* pEndpointInt        = OpcUa_Null;
	OpcUa_Encoder*          pEncoder            = OpcUa_Null;
	OpcUa_MessageContext    cContext;
	OpcUa_Handle            hEncodeContext      = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_Endpoint, "WriteResponse");

	OpcUa_ReturnErrorIfArgumentNull(a_hEndpoint);
	OpcUa_ReturnErrorIfArgumentNull(a_ppOstrm);
	OpcUa_ReturnErrorIfArgumentNull(*a_ppOstrm);

	pEndpointInt = (OpcUa_EndpointInternal*)a_hEndpoint;

	OpcUa_MessageContext_Initialize(&cContext);

	if(OpcUa_IsGood(a_uStatus))
	{
		OpcUa_ReturnErrorIfArgumentNull(a_pResponse);
		OpcUa_ReturnErrorIfArgumentNull(a_pResponseType);

		pEncoder = pEndpointInt->Encoder;
		OpcUa_ReturnErrorIfArgumentNull(pEncoder);

		cContext.KnownTypes         = &OpcUa_ProxyStub_g_EncodeableTypes;
		cContext.NamespaceUris      = &OpcUa_ProxyStub_g_NamespaceUris;
		cContext.AlwaysCheckLengths = OPCUA_SERIALIZER_CHECKLENGTHS;

		/* create encoder */
		uStatus = pEncoder->Open(pEncoder, *a_ppOstrm, &cContext, &hEncodeContext);
		OpcUa_ReturnErrorIfBad(uStatus);

		/* encode message */
		uStatus = pEncoder->WriteMessage((struct _OpcUa_Encoder*)hEncodeContext, a_pResponse, a_pResponseType);

		/* delete encoder */
		OpcUa_Encoder_Close(pEncoder, &hEncodeContext);

		OpcUa_MessageContext_Clear(&cContext);

		OpcUa_GotoErrorIfBad(uStatus);
	}
	else
	{
		/* no encoding */
	}

	/* send response */
	uStatus = OpcUa_Listener_EndSendResponse(   pEndpointInt->SecureListener,
												a_uStatus,
												a_ppOstrm);
	OpcUa_GotoErrorIfBad(uStatus);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_Endpoint_DeleteContext
 *===========================================================================*/
static OpcUa_Void OpcUa_Endpoint_DeleteContext( OpcUa_Endpoint a_hEndpoint,
												OpcUa_Handle*  a_phContext)
{
	if (a_hEndpoint != OpcUa_Null && a_phContext != OpcUa_Null)
	{
		OpcUa_EndpointContext* pContext = OpcUa_Null;

		pContext = (OpcUa_EndpointContext*)*a_phContext;
		/* Begin debug code */
		/*
		if ((pContext->ServiceType.RequestTypeId == 824))
			PublishRequestCounter--;
		if ((pContext->ServiceType.RequestTypeId == 827))
			PublishResponseCounter--;
		printf("pContext=%p requestCounter=%u responseCounter=%u\n", pContext, PublishRequestCounter, PublishResponseCounter);
		*/
		/* End debug code*/
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Endpoint_DeleteContext!\n");

		/* deletes the secure stream of this context */
		/* the inner stream should already be deleted by the endsendresponse */

		OpcUa_Stream_Delete((OpcUa_Stream**)&pContext->pOstrm);
		OpcUa_Stream_Delete((OpcUa_Stream**)&pContext->pIstrm);
		OpcUa_Free(pContext);

		*a_phContext = OpcUa_Null;
	}
	else
	{
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_WARNING, "OpcUa_Endpoint_DeleteContext: NULL!\n");
	}
}

#if OPCUA_ENDPOINT_USE_SERVICE_FAULT
/*============================================================================
 * OpcUa_Endpoint_SendServiceFault
 *===========================================================================*/
static OpcUa_StatusCode OpcUa_Endpoint_SendServiceFault(    OpcUa_Endpoint          a_hEndpoint,
															OpcUa_RequestHeader*    a_pRequestHeader,
															OpcUa_EndpointContext*  a_pContext,
															OpcUa_StatusCode        a_uStatus)
{
	OpcUa_Void*             pFault      = OpcUa_Null;
	OpcUa_EncodeableType*   pFaultType  = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_Endpoint, "SendServiceFault");

	/* create a fault */
	uStatus = OpcUa_ServerApi_CreateFault(  a_pRequestHeader,
											a_uStatus,
											OpcUa_Null,
											OpcUa_Null,
											OpcUa_Null,
										   &pFault,
										   &pFaultType);
	OpcUa_GotoErrorIfBad(uStatus);

	/* send the response */
	uStatus = OpcUa_Endpoint_EndSendResponse(   a_hEndpoint,
												(OpcUa_Handle*)(&a_pContext),
												OpcUa_Good,
												pFault,
												pFaultType);
	OpcUa_GotoErrorIfBad(uStatus);

	OpcUa_EncodeableObject_Delete(  pFaultType,
									(OpcUa_Void**)&pFault);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	if(pFault != OpcUa_Null)
	{
		OpcUa_EncodeableObject_Delete(  pFaultType,
										(OpcUa_Void**)&pFault);
	}

OpcUa_FinishErrorHandling;
}
#endif /* OPCUA_ENDPOINT_USE_SERVICE_FAULT */


/*============================================================================
 * OpcUa_Endpoint_BeginProcessRawRequest
 *===========================================================================*/
/* INFO: Endpoint is locked during this call. */
static OpcUa_StatusCode OpcUa_Endpoint_BeginProcessRawRequest(  OpcUa_Endpoint      a_hEndpoint,
																OpcUa_Handle        a_hConnection,
																OpcUa_InputStream** a_ppIstrm)
{
	OpcUa_EndpointInternal* pEndpointInt    = OpcUa_Null;
	OpcUa_EndpointContext*  pContext        = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_Endpoint, "BeginProcessRawRequest");

	/* check arguments */
	OpcUa_ReturnErrorIfArgumentNull(a_hEndpoint);
	OpcUa_ReturnErrorIfArgumentNull(a_hConnection);
	OpcUa_ReturnErrorIfArgumentNull(a_ppIstrm);

	pEndpointInt = (OpcUa_EndpointInternal*)a_hEndpoint;

	/* check state of endpoint - reject requests if not open; we're not trying to answer here */
	if(pEndpointInt->State != eOpcUa_Endpoint_State_Open)
	{
		(*a_ppIstrm)->Delete((OpcUa_Stream **)a_ppIstrm);
		OpcUa_ReturnErrorIfTrue(pEndpointInt->State != eOpcUa_Endpoint_State_Open, OpcUa_Good);
	}

	pContext = (OpcUa_EndpointContext*)OpcUa_Alloc(sizeof(OpcUa_EndpointContext));
	OpcUa_ReturnErrorIfAllocFailed(pContext);
	OpcUa_MemSet(pContext, 0, sizeof(OpcUa_EndpointContext));

	/* Next call is only valid if OPC UA Secure Conversation is used. */
	/* In case of HTTPS, the transport listener is not set. */
	if(pEndpointInt->TransportListener != OpcUa_Null)
	{
		/* Get securechannel Id */
		uStatus = OpcUa_SecureListener_GetChannelId(pEndpointInt->SecureListener,
													*a_ppIstrm,
													&pContext->uSecureChannelId);
	}
	OpcUa_GotoErrorIfBad(uStatus);

	pContext->hConnection = a_hConnection;
	pContext->pIstrm = *a_ppIstrm;
	*a_ppIstrm = OpcUa_Null;

	OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Endpoint_BeginProcessRawRequest: Invoking service handler with context 0x%p!\n", pContext);

	OpcUa_Mutex_Unlock(pEndpointInt->Mutex);

	uStatus = pEndpointInt->pfEndpointCallback( pEndpointInt,
												pEndpointInt->pvEndpointCallbackData,
												eOpcUa_Endpoint_Event_RawRequest,
												uStatus,
												pContext->uSecureChannelId,
												(OpcUa_Handle*)&pContext,
												OpcUa_Null,
												OpcUa_Null,
												OPCUA_ENDPOINT_MESSAGESECURITYMODE_INVALID,
												0);

	/* begin invoke must always call OpcUa_Endpoint_EndSendResponse which frees the context */
	OpcUa_Mutex_Lock(pEndpointInt->Mutex);

	OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Endpoint_BeginProcessRawRequest: Service handler returned! (0x%08X)\n", uStatus);
	OpcUa_GotoErrorIfBad(uStatus);

	/* delete message context and regain pointer to inputstream for lower level to clean up */
	if(pContext != OpcUa_Null)
	{
		*a_ppIstrm = pContext->pIstrm; /* may be null if already cleaned up */
		pContext->pIstrm = OpcUa_Null; /* prevents following call from deleting stream */
		OpcUa_Endpoint_DeleteContext(a_hEndpoint, (OpcUa_Handle*)&pContext);
	}

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	/* delete message context */
	if(pContext != OpcUa_Null)
	{
		*a_ppIstrm = pContext->pIstrm;
		pContext->pIstrm = OpcUa_Null; /* prevents following call from deleting stream */
		OpcUa_Endpoint_DeleteContext(a_hEndpoint, (OpcUa_Handle*)&pContext);
	}

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_Endpoint_BeginProcessRequest
 *===========================================================================*/
/* INFO: Endpoint is locked during this call. */
static OpcUa_StatusCode OpcUa_Endpoint_BeginProcessRequest( OpcUa_Endpoint      a_hEndpoint,
															OpcUa_Handle        a_hConnection,
															OpcUa_InputStream** a_ppIstrm)
{
	OpcUa_EndpointInternal* pEndpointInt    = OpcUa_Null;
	OpcUa_Void*             pRequest        = OpcUa_Null;
	OpcUa_EncodeableType*   pRequestType    = OpcUa_Null;
	OpcUa_EndpointContext*  pContext        = OpcUa_Null;
	OpcUa_Buffer            Buffer;

#if OPCUA_ENDPOINT_USE_SERVICE_FAULT
	OpcUa_Boolean           bServiceFault   = OpcUa_False;
#endif /* OPCUA_ENDPOINT_USE_SERVICE_FAULT */

OpcUa_InitializeStatus(OpcUa_Module_Endpoint, "BeginProcessRequest");

	/* check arguments */
	OpcUa_ReturnErrorIfArgumentNull(a_hEndpoint);
	OpcUa_ReturnErrorIfArgumentNull(a_hConnection);
	OpcUa_ReturnErrorIfArgumentNull(a_ppIstrm);

	pEndpointInt = (OpcUa_EndpointInternal*)a_hEndpoint;

	/* check state of endpoint - reject requests if not open; we're not trying to answer here */
	if(pEndpointInt->State != eOpcUa_Endpoint_State_Open)
	{
		(*a_ppIstrm)->Delete((OpcUa_Stream **)a_ppIstrm);
		OpcUa_ReturnErrorIfTrue(pEndpointInt->State != eOpcUa_Endpoint_State_Open, OpcUa_Good);
	}

	pContext = (OpcUa_EndpointContext*)OpcUa_Alloc(sizeof(OpcUa_EndpointContext));
	OpcUa_ReturnErrorIfAllocFailed(pContext);
	OpcUa_MemSet(pContext, 0, sizeof(OpcUa_EndpointContext));

	/* decode the request */
	uStatus = OpcUa_Endpoint_ReadRequest(   a_hEndpoint,
											*a_ppIstrm,
											&pRequest,
											&pRequestType);

	if((OpcUa_IsBad(uStatus)) || (pRequest == OpcUa_Null) || (pRequestType == OpcUa_Null))
	{
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_WARNING, "OpcUa_Endpoint_BeginProcessRequest: ERROR READING REQUEST! Status 0x%08X \n", uStatus);
		pEndpointInt->pfEndpointCallback(   pEndpointInt,
											pEndpointInt->pvEndpointCallbackData,
											eOpcUa_Endpoint_Event_DecoderError,
											uStatus,
											0,
											OpcUa_Null,
											OpcUa_Null,
											OpcUa_Null,
											OPCUA_ENDPOINT_MESSAGESECURITYMODE_INVALID,
											0);
		OpcUa_GotoError;
	}

	/* Next call is only valid if OPC UA Secure Conversation is used. */
	/* In case of HTTPS, the transport listener is not set. */
	if(pEndpointInt->TransportListener != OpcUa_Null)
	{
		/* Get securechannel Id */
		uStatus = OpcUa_SecureListener_GetChannelId(pEndpointInt->SecureListener,
													*a_ppIstrm,
													&pContext->uSecureChannelId);
	}
	OpcUa_GotoErrorIfBad(uStatus);

	/* find out whether the endpoint supports the service. */
	uStatus = OpcUa_ServiceTable_FindService(&pEndpointInt->SupportedServices, pRequestType->TypeId, &pContext->ServiceType);
	if(OpcUa_IsEqual(OpcUa_BadServiceUnsupported))
	{
#if !OPCUA_ENCODEABLE_OBJECT_OMIT_TYPE_NAME
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Endpoint_BeginProcessRequest: Unsupported Service with RequestTypeId %u requested! (HINT: %s)\n", pRequestType->TypeId, pRequestType->TypeName);
#else /* !OPCUA_ENCODEABLE_OBJECT_OMIT_TYPE_NAME */
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Endpoint_BeginProcessRequest: Unsupported Service with RequestTypeId %u requested!\n", pRequestType->TypeId);
#endif /* !OPCUA_ENCODEABLE_OBJECT_OMIT_TYPE_NAME */
		pEndpointInt->pfEndpointCallback(   pEndpointInt,
											pEndpointInt->pvEndpointCallbackData,
											eOpcUa_Endpoint_Event_UnsupportedServiceRequested,
											uStatus,
											pContext->uSecureChannelId,
											OpcUa_Null,
											OpcUa_Null,
											OpcUa_Null,
											OPCUA_ENDPOINT_MESSAGESECURITYMODE_INVALID,
											0);
	}
	else if(OpcUa_IsBad(uStatus))
	{
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_WARNING, "OpcUa_Endpoint_BeginProcessRequest: Could not find service handler (0x%08X)\n", uStatus);
	}
	else
	{
#if !OPCUA_ENCODEABLE_OBJECT_OMIT_TYPE_NAME
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Endpoint_BeginProcessRequest: Service with RequestTypeId %u called! (Request: %s)\n", pRequestType->TypeId, pRequestType->TypeName);
#else /* !OPCUA_ENCODEABLE_OBJECT_OMIT_TYPE_NAME */
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Endpoint_BeginProcessRequest: Service with RequestTypeId %u called!\n", pRequestType->TypeId);
#endif /* !OPCUA_ENCODEABLE_OBJECT_OMIT_TYPE_NAME */
	}

#if OPCUA_ENDPOINT_USE_SERVICE_FAULT
	if(OpcUa_IsBad(uStatus))
	{
		bServiceFault = OpcUa_True;
	}
#else /* OPCUA_ENDPOINT_USE_SERVICE_FAULT */
	OpcUa_GotoErrorIfBad(uStatus);
#endif /* OPCUA_ENDPOINT_USE_SERVICE_FAULT */

	pContext->hConnection = a_hConnection;
	pContext->pIstrm = *a_ppIstrm;
	*a_ppIstrm = OpcUa_Null;

	/* Begin debug code */
	/*
	if ((pContext->ServiceType.RequestTypeId == 824))
		PublishRequestCounter++;
	if ((pContext->ServiceType.RequestTypeId == 827))
		PublishResponseCounter++;
	printf("pContext=%p requestCounter=%u responseCounter=%u\n", pContext, PublishRequestCounter, PublishResponseCounter);
	*/
	/* End debug code*/

	/* clean up the stream buffers */
	pContext->pIstrm->DetachBuffer((OpcUa_Stream *)pContext->pIstrm, &Buffer, OpcUa_Null);
	OpcUa_Buffer_Clear(&Buffer);

	OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Endpoint_BeginProcessRequest: Invoking service handler with context 0x%p!\n", pContext);
	
	OpcUa_Mutex_Unlock(pEndpointInt->Mutex);

#if OPCUA_ENDPOINT_USE_SERVICE_FAULT
	if(bServiceFault != OpcUa_True)
	{
#endif /* OPCUA_ENDPOINT_USE_SERVICE_FAULT */
		uStatus = pContext->ServiceType.BeginInvoke(    a_hEndpoint, 
														pContext, 
														&pRequest, 
														pRequestType);
#if OPCUA_ENDPOINT_USE_SERVICE_FAULT
	}
	else
	{
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Endpoint_BeginProcessRequest: Sending service fault with status 0x%08X!\n", uStatus);
		uStatus = OpcUa_Endpoint_SendServiceFault(  a_hEndpoint,
													(OpcUa_RequestHeader*)pRequest,
													pContext,
													uStatus);
	}
#endif /* OPCUA_ENDPOINT_USE_SERVICE_FAULT */

	/* begin invoke must always call OpcUa_Endpoint_EndSendResponse which frees the context */
	pContext = OpcUa_Null;
	OpcUa_Mutex_Lock(pEndpointInt->Mutex);
	if ((uStatus != OpcUa_Good) )
	{
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Endpoint_BeginProcessRequest: Service handler returned! (0x%08X)\n", uStatus);
		OpcUa_GotoErrorIfBad(uStatus);
	}
	/* does nothing if callee before nulled the parameter */
	if(pRequest != OpcUa_Null)
	{
		OpcUa_EncodeableObject_Delete(pRequestType, &pRequest);
	}
OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	/* no cleaning up, if status is good -> everything handled before */
	if(OpcUa_IsBad(uStatus)  && !OpcUa_IsEqual(OpcUa_BadDisconnect) && !OpcUa_IsEqual(OpcUa_BadConnectionClosed))
	{
		if(pContext != OpcUa_Null && pContext->pOstrm != OpcUa_Null)
		{
			/* undo BeginSendResponse */
			uStatus = OpcUa_Listener_AbortSendResponse( pEndpointInt->SecureListener,
														uStatus,
														OpcUa_Null,
														(OpcUa_OutputStream**)&(pContext->pOstrm));
		}
		else
		{
			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Endpoint_BeginProcessRequest: Not able to create/send response. (0x%08X)\n", uStatus);
		}

		/* delete message context */
		if(pContext != OpcUa_Null)
		{
			OpcUa_Endpoint_DeleteContext(a_hEndpoint, (OpcUa_Handle*)&pContext);
		}
	}

	/* delete deserialized request */
	if(pRequest != OpcUa_Null)
	{
		OpcUa_EncodeableObject_Delete(pRequestType, &pRequest);
	}

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_Endpoint_BeginSendResponse
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Endpoint_BeginSendResponse(  OpcUa_Endpoint         a_hEndpoint,
													OpcUa_Handle           a_hContext,
													OpcUa_Void**           a_ppResponse,
													OpcUa_EncodeableType** a_ppResponseType)
{
	OpcUa_EndpointContext* pContext = OpcUa_Null;

	OpcUa_StatusCode uStatus = OpcUa_Good;

	/* check arguments */
	if (a_hEndpoint)
	{
		if (a_hContext)
		{
			if (a_ppResponse)
			{
				*a_ppResponse = OpcUa_Null;
				*a_ppResponseType = OpcUa_Null;

				pContext = (OpcUa_EndpointContext*)a_hContext;

				OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Endpoint_BeginSendResponse: Context 0x%p\n", pContext);
				/* get the response type */
				*a_ppResponseType = pContext->ServiceType.ResponseType;

				/* allocate instance of the encodeable type */
				uStatus = OpcUa_EncodeableObject_Create(*a_ppResponseType, a_ppResponse);	
				if (uStatus != OpcUa_Good)
				{
					*a_ppResponse = OpcUa_Null;
					*a_ppResponseType = OpcUa_Null;
				}
				//else
				//{
				//	if ((pContext->ServiceType.RequestTypeId == 824))
				//		PublishRequestCounter++;
				//	if ((pContext->ServiceType.RequestTypeId == 827))
				//		PublishResponseCounter++;
				//	printf("SecureChannelId=%u request=%u responseCounter=%u\n", pContext->uSecureChannelId, PublishRequestCounter, PublishResponseCounter);
				//}
			}
			else
				uStatus = OpcUa_BadInvalidArgument;
		}
		else
			uStatus = OpcUa_BadInvalidArgument;
	}
	else
		uStatus = OpcUa_BadInvalidArgument;

	return uStatus;
}

/*============================================================================
 * OpcUa_Endpoint_CancelSendResponse
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Endpoint_CancelSendResponse(
	OpcUa_Endpoint        a_hEndpoint,
	OpcUa_StatusCode      a_uStatus,
	OpcUa_String*         a_psReason,
	OpcUa_Handle*         a_phContext)
{
	OpcUa_EndpointContext*  pContext     = OpcUa_Null;
	OpcUa_EndpointInternal* pEndpointInt = (OpcUa_EndpointInternal*)a_hEndpoint;

OpcUa_InitializeStatus(OpcUa_Module_Endpoint, "CancelSendResponse");

	OpcUa_ReturnErrorIfArgumentNull(a_hEndpoint);
	OpcUa_ReturnErrorIfArgumentNull(a_phContext);
	OpcUa_ReturnErrorIfArgumentNull(*a_phContext);

	pContext = (OpcUa_EndpointContext*)*a_phContext;
	OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Endpoint_CancelSendResponse: Context 0x%p\n", pContext);
	if(pContext->pOstrm != OpcUa_Null)
	{
		OpcUa_Listener_AbortSendResponse(   pEndpointInt->SecureListener,
											a_uStatus,
											a_psReason,
											&(pContext->pOstrm));

		//if ((pContext->ServiceType.RequestTypeId == 824))
		//	PublishRequestCounter--;
		//if ((pContext->ServiceType.RequestTypeId == 827))
		//	PublishResponseCounter--;
		//printf("SecureChannelId=%u request=%u responseCounter=%u\n", pContext->uSecureChannelId, PublishRequestCounter, PublishResponseCounter);
	}

	OpcUa_Endpoint_DeleteContext(a_hEndpoint, a_phContext);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_Endpoint_EndSendResponse
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Endpoint_EndSendResponse(
	OpcUa_Endpoint        a_hEndpoint,
	OpcUa_Handle*         a_phContext,
	OpcUa_StatusCode      a_uStatusCode,
	OpcUa_Void*           a_pResponse,
	OpcUa_EncodeableType* a_pResponseType)
{
	OpcUa_EndpointContext* pContext = OpcUa_Null;

	OpcUa_StatusCode uStatus=OpcUa_Good;

	/* check arguments */
	if (a_hEndpoint)
	{
		if (a_phContext)
		{
			if (*a_phContext)
			{
				OPCUA_ENDPOINT_CHECKOPEN(a_hEndpoint);

				if (OpcUa_IsBad(a_uStatusCode))
				{
					OpcUa_Endpoint_CancelSendResponse(a_hEndpoint,
						a_uStatusCode,
						OpcUa_Null,
						a_phContext);
				}
				else
				{
					/* get the context */
					pContext = (OpcUa_EndpointContext*)*a_phContext;

					OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Endpoint_EndSendResponse: Status 0x%08X, Context 0x%p!\n", a_uStatusCode, pContext);
					uStatus = OpcUa_Listener_BeginSendResponse(((OpcUa_EndpointInternal*)a_hEndpoint)->SecureListener,
						pContext->hConnection,
						&pContext->pIstrm,
						&pContext->pOstrm);
					if (OpcUa_IsBad(uStatus))
					{
						OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Listener_BeginSendResponse failed (0x%08X)\n", uStatus);
						OpcUa_Endpoint_CancelSendResponse(a_hEndpoint,
							OpcUa_Good,
							OpcUa_Null,
							a_phContext);
					}
					else
					{

						//if ((pContext->ServiceType.RequestTypeId == 824) )
						//	PublishRequestCounter--;
						//if ((pContext->ServiceType.RequestTypeId == 827))
						//	PublishResponseCounter--;
						//printf("SecureChannelId=%u request=%u responseCounter=%u\n", pContext->uSecureChannelId, PublishRequestCounter, PublishResponseCounter);
						/* send the response */
						uStatus = OpcUa_Endpoint_WriteResponse(a_hEndpoint,
							&(pContext->pOstrm),
							a_uStatusCode,
							a_pResponse,
							a_pResponseType);
						if (uStatus == OpcUa_Good)
							OpcUa_Endpoint_DeleteContext(a_hEndpoint, a_phContext);
						else
						{
							OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Endpoint_WriteResponse failed: (0x%08X)\n", uStatus);
							if (uStatus != OpcUa_BadNotFound)
							{
								OpcUa_Endpoint_CancelSendResponse(a_hEndpoint,
									OpcUa_Good,
									OpcUa_Null,
									a_phContext);
							}
						}
					}
				}
			}
			else
				uStatus = OpcUa_BadInvalidArgument;
		}
		else
			uStatus = OpcUa_BadInvalidArgument;
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}

/*============================================================================
 * OpcUa_Endpoint_GetServiceFunction
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Endpoint_GetServiceFunction( OpcUa_Endpoint           a_hEndpoint,
													OpcUa_Handle             a_hContext,
													OpcUa_PfnInvokeService** a_ppInvoke)
{
	OpcUa_EndpointContext* pContext = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_Endpoint, "GetServiceFunction");

	/* check arguments */
	OpcUa_ReturnErrorIfArgumentNull(a_hEndpoint);
	OpcUa_ReturnErrorIfArgumentNull(a_hContext);
	OpcUa_ReturnErrorIfArgumentNull(a_ppInvoke);

	/* get the context */
	pContext = (OpcUa_EndpointContext*)a_hContext;

	*a_ppInvoke = pContext->ServiceType.Invoke;

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_Endpoint_UpdateServiceFunctions
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Endpoint_UpdateServiceFunctions( OpcUa_Endpoint              a_hEndpoint,
														OpcUa_UInt32                 a_uRequestTypeId,
														OpcUa_PfnBeginInvokeService* a_pBeginInvoke,
														OpcUa_PfnInvokeService*      a_pInvoke)
{
	OpcUa_UInt32            ii              = 0;
	OpcUa_ServiceTable*     pTable          = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_Endpoint, "UpdateServiceFunctions");

	OpcUa_ReturnErrorIfArgumentNull(a_hEndpoint);

	pTable = &(((OpcUa_EndpointInternal*)a_hEndpoint)->SupportedServices);

	/* find out whether the endpoint supports the service. */
	for(ii = 0; ii < pTable->Count; ii++)
	{
		if(pTable->Entries[ii].RequestTypeId == a_uRequestTypeId)
		{
			/* update asynchronous function */
			if(a_pBeginInvoke != OpcUa_Null)
			{
				pTable->Entries[ii].BeginInvoke = a_pBeginInvoke;
			}

			/* update synchronous function */
			if(a_pInvoke != OpcUa_Null)
			{
				pTable->Entries[ii].Invoke = a_pInvoke;
			}

			break;
		}
	}

	/* service not found */
	if(ii == pTable->Count)
	{
		OpcUa_GotoErrorWithStatus(OpcUa_BadNotFound);
	}

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_Endpoint_GetCallbackData
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Endpoint_GetCallbackData(OpcUa_Endpoint  a_hEndpoint,
												OpcUa_Void**    a_ppvCallbackData)
{
	OpcUa_ReturnErrorIfArgumentNull(a_hEndpoint);
	OpcUa_ReturnErrorIfArgumentNull(a_ppvCallbackData);

	*a_ppvCallbackData = OpcUa_Null;

	OpcUa_Mutex_Lock(((OpcUa_EndpointInternal*)a_hEndpoint)->Mutex);

	*a_ppvCallbackData = ((OpcUa_EndpointInternal*)a_hEndpoint)->pvEndpointCallbackData;

	OpcUa_Mutex_Unlock(((OpcUa_EndpointInternal*)a_hEndpoint)->Mutex);

	return OpcUa_Good;
}

/*============================================================================
 * OpcUa_Endpoint_GetRawRequest
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Endpoint_GetRawRequest(  OpcUa_Endpoint      a_hEndpoint,
												OpcUa_Handle        a_hContext,
												OpcUa_InputStream** a_ppInputStream)
{
	OpcUa_EndpointContext* pContext = OpcUa_Null;

	OpcUa_ReturnErrorIfArgumentNull(a_hEndpoint);
	OpcUa_ReturnErrorIfArgumentNull(a_hContext);
	OpcUa_ReturnErrorIfArgumentNull(a_ppInputStream);

	/* get the context */
	pContext = (OpcUa_EndpointContext*)a_hContext;

	*a_ppInputStream = pContext->pIstrm;

	return OpcUa_Good;
}

/*============================================================================
 * OpcUa_Endpoint_BeginSendRawResponse
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Endpoint_BeginSendRawResponse(
	OpcUa_Endpoint          a_hEndpoint,
	OpcUa_Handle            a_hContext,
	OpcUa_OutputStream**    a_ppOutputStream)
{
	OpcUa_EndpointInternal* pEndpointInt    = OpcUa_Null;
	OpcUa_EndpointContext*  pContext        = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_Endpoint, "BeginSendRawResponse");

	/* check arguments */
	OpcUa_ReturnErrorIfArgumentNull(a_hEndpoint);
	OpcUa_ReturnErrorIfArgumentNull(a_hContext);
	OpcUa_ReturnErrorIfArgumentNull(a_ppOutputStream);

	pEndpointInt = (OpcUa_EndpointInternal*)a_hEndpoint;
	pContext     = (OpcUa_EndpointContext*)a_hContext;

	uStatus = OpcUa_Listener_BeginSendResponse( pEndpointInt->SecureListener,
												pContext->hConnection,
												&pContext->pIstrm,
												&pContext->pOstrm);
	if(OpcUa_IsBad(uStatus))
	{
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_WARNING, "OpcUa_Endpoint_BeginSendRawResponse: Could not allocate response stream! (0x%08X)\n", uStatus);
		OpcUa_GotoError;
	}

	*a_ppOutputStream = pContext->pOstrm;

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_Endpoint_EndSendRawResponse
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Endpoint_EndSendRawResponse(
	OpcUa_Endpoint      a_hEndpoint,
	OpcUa_StatusCode    a_uStatus,
	OpcUa_Handle*       a_phContext)
{
	OpcUa_EndpointInternal* pEndpointInt    = OpcUa_Null;
	OpcUa_EndpointContext*  pContext        = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_Endpoint, "EndSendRawResponse");

	OpcUa_ReturnErrorIfArgumentNull(a_hEndpoint);
	OpcUa_ReturnErrorIfArgumentNull(a_phContext);
	OpcUa_ReturnErrorIfArgumentNull(*a_phContext);

	pEndpointInt = (OpcUa_EndpointInternal*)a_hEndpoint;
	pContext     = (OpcUa_EndpointContext*)*a_phContext;
	*a_phContext = OpcUa_Null;

	uStatus = OpcUa_Listener_EndSendResponse(   pEndpointInt->SecureListener,
												a_uStatus,
											   &pContext->pOstrm);
	if(OpcUa_IsBad(uStatus))
	{
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_WARNING, "OpcUa_Endpoint_EndSendRawResponse: Could not send response stream! (0x%08X)\n", uStatus);
		OpcUa_GotoError;
	}

	OpcUa_Endpoint_DeleteContext(a_hEndpoint, (OpcUa_Handle*)&pContext);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	OpcUa_Endpoint_DeleteContext(a_hEndpoint, (OpcUa_Handle*)&pContext);

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_Endpoint_GetPeerInfoFromContext
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Endpoint_GetPeerInfoFromContext(
	OpcUa_Endpoint  a_hEndpoint,
	OpcUa_Handle    a_hContext,
	OpcUa_String*   a_psPeerInfo)
{
	OpcUa_EndpointContext*  pContext        = (OpcUa_EndpointContext*)a_hContext;
	OpcUa_EndpointInternal* pEndpointInt    = (OpcUa_EndpointInternal*)a_hEndpoint;

	/* check arguments */
	OpcUa_ReturnErrorIfArgumentNull(a_hEndpoint);
	OpcUa_ReturnErrorIfArgumentNull(a_hContext);

	return  OpcUa_Listener_GetPeerInfo( pEndpointInt->SecureListener,
										pContext->hConnection,
										a_psPeerInfo);
}

/*============================================================================
 * OpcUa_Endpoint_GetPeerInfoBySecureChannelId
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Endpoint_GetPeerInfoBySecureChannelId(
	OpcUa_Endpoint  a_hEndpoint,
	OpcUa_UInt32    a_uSecureChannelId,
	OpcUa_String*   a_psPeerInfo)
{
	OpcUa_EndpointInternal* pEndpointInt = (OpcUa_EndpointInternal*)a_hEndpoint;
	OpcUa_StatusCode uStatus = OpcUa_Good;
	/* check arguments */
	OpcUa_String* pszEndPointHeader = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszEndPointHeader);
	OpcUa_String_AttachCopy(pszEndPointHeader, "opc.tcp:");
	if (a_hEndpoint)
	{
		/* check if endpoint url supports this call. */
		if (!OpcUa_String_StrnCmp(&(pEndpointInt->Url), pszEndPointHeader,OpcUa_String_StrLen(pszEndPointHeader),OpcUa_True))
		{
			uStatus = OpcUa_SecureListener_GetPeerInfoBySecureChannelId(pEndpointInt->SecureListener,
				a_uSecureChannelId,
				a_psPeerInfo);
		}
		else
		{
			/* only opc.tcp endpoints support connection lookup by secure channel id up to now. */
			uStatus = OpcUa_BadNotSupported;
		}
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	//
	if (pszEndPointHeader)
	{
		OpcUa_String_Clear(pszEndPointHeader);
		OpcUa_Free(pszEndPointHeader);
		pszEndPointHeader = OpcUa_Null;
	}
	return uStatus;
}

/*============================================================================
 * OpcUa_Endpoint_CloseSecureChannel
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Endpoint_CloseSecureChannel(
	OpcUa_Endpoint      a_hEndpoint,
	OpcUa_UInt32        a_uSecureChannelId,
	OpcUa_StatusCode    a_uStatus)
{
	OpcUa_EndpointInternal* pEndpointInt    = (OpcUa_EndpointInternal*)a_hEndpoint;
	OpcUa_StatusCode        uStatus         = OpcUa_Good;
	OpcUa_Handle            hConnection     = OpcUa_Null;

	/* check arguments */
	OpcUa_ReturnErrorIfArgumentNull(pEndpointInt);

	OpcUa_ReferenceParameter(a_uSecureChannelId);

	/* translate secure channel id into channel handle. */
	uStatus = OpcUa_SecureListener_GetConnectionHandleBySecureChannelId(
				pEndpointInt->SecureListener,
				a_uSecureChannelId,
			   &hConnection);

	/* close  */
	if(OpcUa_IsGood(uStatus))
	{
		uStatus = OpcUa_Listener_CloseConnection(   pEndpointInt->SecureListener,
													hConnection,
													a_uStatus);
	}

	return uStatus;
}

#endif /* OPCUA_HAVE_SERVERAPI */
