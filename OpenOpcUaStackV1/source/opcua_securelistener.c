/*****************************************************************************
	  Author
		?. Michel Condemine, 4CE Industry (2010-2012)
	  
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

#ifdef OPCUA_HAVE_SERVERAPI

#include <opcua_mutex.h>
#include <opcua_pkifactory.h>
#include <opcua_cryptofactory.h>
#include <opcua_datetime.h>
#include <opcua_list.h>
#include <opcua_thread.h>
#include <opcua_threadpool.h>
#include <opcua_p_thread.h>
#include <opcua_utilities.h>

/* stackcore */
#include <opcua_identifiers.h>
#include <opcua_binaryencoder.h>
#include <opcua_securechannel.h>
#include <opcua_listener.h>

/* security */
#include <opcua_tcpsecurechannel.h>
#include <opcua_soapsecurechannel.h>
#include <opcua_securelistener_channelmanager.h>
#include <opcua_securechannel_types.h>
#include <opcua_securestream.h>

/* header */
#include <opcua_securelistener.h>
#include <opcua_securelistener_policymanager.h>

/** @brief Map enumeration OpcUa_MessageSecurityMode to bitfield */
#define OPCUA_ENDPOINT_MESSAGESECURITYMODE_FROM_ENUM(xMode) ((OpcUa_UInt16)((xMode == 0) ? 1 : (xMode << 1)))

/** @brief internal configuration - only for test */
#define OPCUA_SECURELISTENER_ALLOW_NOPKI OPCUA_CONFIG_YES

/** @brief Return OpcUa_BadDisconnect after a the service for closing the securechannel was called. Fixes an issue with clients that do nothing after sending the CSC. */
#define OPCUA_SECURELISTENER_CLOSE_CONNECTION_ON_CLOSE_SECURECHANNEL OPCUA_CONFIG_YES

#if(OPCUA_SECURELISTENER_INACTIVECHANNELTIMEOUT < 1)
# error "OPCUA_SECURELISTENER_CHANNELTIMEOUT must be larger or equal than 1!"
#endif

#ifndef OPCUA_P_BYTE_ORDER
#error OPCUA_P_BYTE_ORDER not defined
#endif

/*============================================================================
 * Prototypes
 *===========================================================================*/

/*============================================================================
 * OpcUa_SecureListener_BeginSendOpenSecureChannelResponse
 *===========================================================================*/
static OpcUa_StatusCode OpcUa_SecureListener_BeginSendOpenSecureChannelResponse(
	OpcUa_Listener*             a_pListener,
	OpcUa_SecureChannel*        a_pSecureChannel,
	OpcUa_InputStream*          a_pSecureIstrm,
	OpcUa_String*               a_pSecurityPolicyUri,
	OpcUa_MessageSecurityMode   a_MessageSecurityMode,
	OpcUa_CryptoProvider*       a_pCryptoProvider,
	OpcUa_ByteString*           a_pServerCertificate,
	OpcUa_ByteString*           a_pServerCertificateChain,
	OpcUa_ByteString*           a_pClientCertificateThumbprint,
	OpcUa_OutputStream**        a_ppSecureOstrm);

/*============================================================================
 * OpcUa_SecureListener_EndSendOpenSecureChannelResponse
 *===========================================================================*/
static OpcUa_StatusCode OpcUa_SecureListener_EndSendOpenSecureChannelResponse(
	OpcUa_Listener*             pListener,
	OpcUa_OutputStream**        ppSecureOstrm,
	OpcUa_StatusCode            uStatus);

/*============================================================================
 * OpcUa_SecureListener_ProcessOpenSecureChannelRequest
 *===========================================================================*/
OpcUa_StatusCode OpcUa_SecureListener_ProcessOpenSecureChannelRequest(
	OpcUa_Listener*                 a_pListener,
	OpcUa_Handle                    a_hConnection,
	OpcUa_InputStream**             a_ppTransportIstrm,
	OpcUa_Boolean                   a_bRequestComplete);

/*============================================================================
 * OpcUa_SecureListener_ProcessCloseSecureChannelRequest
 *===========================================================================*/
OpcUa_StatusCode OpcUa_SecureListener_ProcessCloseSecureChannelRequest(
	OpcUa_Listener*                 pListener,
	OpcUa_Handle                    hConnection,
	OpcUa_InputStream**             ppTransportIstrm,
	OpcUa_Boolean                   bRequestComplete);

/*============================================================================
 * OpcUa_SecureListener_ProcessSessionCallRequest
 *===========================================================================*/
OpcUa_StatusCode OpcUa_SecureListener_ProcessSessionCallRequest(
	OpcUa_Listener*                 pListener,
	OpcUa_Handle                    hConnection,
	OpcUa_InputStream**             ppTransportIstrm,
	OpcUa_Boolean                   bRequestComplete);

/*============================================================================
 * OpcUa_SecureListener_OnNotify
 *===========================================================================*/
static OpcUa_StatusCode OpcUa_SecureListener_OnNotify(
	struct _OpcUa_Listener*         pListener,
	OpcUa_Void*                     pCallbackData,
	OpcUa_ListenerEvent             eEvent,
	OpcUa_Handle                    hConnection,
	OpcUa_InputStream**             ppIstrm,
	OpcUa_StatusCode                uOperationStatus);

/*============================================================================
 * OpcUa_SecureListener_Open
 *===========================================================================*/
OpcUa_StatusCode OpcUa_SecureListener_Open(
	struct _OpcUa_Listener*         pListener,
	OpcUa_String*                   sUrl,
	OpcUa_Listener_PfnOnNotify*     pCallback,
	OpcUa_Void*                     pCallbackData);

/*============================================================================
 * OpcUa_SecureListener_ProcessRequest
 *===========================================================================*/
static OpcUa_StatusCode OpcUa_SecureListener_ProcessRequest(
	OpcUa_Listener*                 pListener,
	OpcUa_Handle                    hConnection,
	OpcUa_InputStream**             ppIstrm,
	OpcUa_Boolean                   bRequestComplete);

/*============================================================================
 * OpcUa_SecureListener_GetPeerInfo
 *===========================================================================*/
OpcUa_StatusCode OpcUa_SecureListener_GetPeerInfo(
	OpcUa_Listener*                 pListener,
	OpcUa_Handle                    hConnection,
	OpcUa_String*                   psPeerInfo);

/*============================================================================
 * OpcUa_SecureListener_SanityCheck
 *===========================================================================*/
#define OpcUa_SecureListener_SanityCheck 0xA0A40F79

/*============================================================================
 * OpcUa_SecureListenerState
 *===========================================================================*/
typedef enum _OpcUa_SecureListenerState
{
	OpcUa_SecureListenerState_Open,
	OpcUa_SecureListenerState_Closed,
	OpcUa_SecureListenerState_Unknown
}
OpcUa_SecureListenerState;

/*============================================================================
 * OpcUa_SecureListener_ThreadPoolJobArgument
 *===========================================================================*/
#if OPCUA_SECURELISTENER_SUPPORT_THREADPOOL
typedef struct _OpcUa_SecureListener_ThreadPoolJobArgument
{
	/** @brief the transport connection */
	OpcUa_Handle                    hConnection;
	/** @brief secure listener interface */
	OpcUa_Listener*                 pListener;
	/** @brief transport input stream for last append */
	OpcUa_InputStream*              pTransportIstrm;
	/** @brief unfinished secure input stream waiting for last append */
	OpcUa_InputStream*              pSecureIstrm;
	/** @brief application callback function */
	OpcUa_Void*                     pCallback;
	/** @brief application callback function data */
	OpcUa_Void*                     pCallbackData;
	/** @brief whether the job can only handle discovery service calls */
	OpcUa_Boolean                   bDiscoveryOnly;
	/** @brief the security token id */
	OpcUa_UInt32                    uTokenId;
	/** @brief the security token id */
	OpcUa_UInt32                    uSecureChannelId;
} OpcUa_SecureListener_ThreadPoolJobArgument;
#endif /* OPCUA_SECURELISTENER_SUPPORT_THREADPOOL */

/*============================================================================
 * OpcUa_SecureListener
 *===========================================================================*/
typedef struct _OpcUa_SecureListener
{
	OpcUa_UInt32                                    SanityCheck;
	OpcUa_Mutex                                     Mutex;
	OpcUa_Listener*                                 TransportListener;
	OpcUa_Listener_PfnOnNotify*                     Callback;
	OpcUa_Void*                                     CallbackData;
	OpcUa_SecureListener_PfnSecureChannelCallback*  SecureChannelCallback;
	OpcUa_Void*                                     SecureChannelCallbackData;
	OpcUa_SecureListenerState                       State;
	OpcUa_SecureListener_ChannelManager*            ChannelManager;
	OpcUa_SecureListener_PolicyManager*             PolicyManager;
	OpcUa_PKIProvider*                              ServerPKIProvider;
	OpcUa_Decoder*                                  Decoder;
	OpcUa_Encoder*                                  Encoder;
	OpcUa_StringTable*                              NamespaceUris;
	OpcUa_EncodeableTypeTable*                      KnownTypes;
	OpcUa_ByteString*                               pServerCertificateChain;
	OpcUa_ByteString*                               pServerCertificate;
	OpcUa_UInt32                                    uNumberOfChainElements;
	OpcUa_ByteString*                               absServerCertificateChain;
	OpcUa_Key                                       ServerPrivateKey;
	OpcUa_UInt32                                    uNextSecureChannelId;
#if OPCUA_SECURELISTENER_SUPPORT_THREADPOOL
	OpcUa_ThreadPool                                hThreadPool;
#endif /* OPCUA_SECURELISTENER_SUPPORT_THREADPOOL */
}
OpcUa_SecureListener;

/*============================================================================
 * OpcUa_SecureListener_Open
 *===========================================================================*/
OpcUa_StatusCode OpcUa_SecureListener_Open(
	OpcUa_Listener*             a_pListener,
	OpcUa_String*               a_sUrl,
	OpcUa_Listener_PfnOnNotify* a_pCallback,
	OpcUa_Void*                 a_pCallbackData)
{
	OpcUa_SecureListener* pSecureListener = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_SecureListener, "Open");

	OpcUa_ReturnErrorIfArgumentNull(a_pListener);
	OpcUa_ReturnErrorIfArgumentNull(a_sUrl);
	OpcUa_ReturnErrorIfArgumentNull(a_pCallback);

	OpcUa_ReturnErrorIfInvalidObject(OpcUa_SecureListener, a_pListener, Open);

	pSecureListener = (OpcUa_SecureListener*)a_pListener->Handle;

	/* acquire lock until open is complete */
	OpcUa_Mutex_Lock(pSecureListener->Mutex);

	if (pSecureListener->State != OpcUa_SecureListenerState_Closed)
	{
		uStatus = OpcUa_BadInvalidState;
		OpcUa_GotoErrorIfBad(uStatus);
	}

	pSecureListener->Callback     = a_pCallback;
	pSecureListener->CallbackData = a_pCallbackData;
	pSecureListener->State        = OpcUa_SecureListenerState_Unknown;

	/* open the non-secure listener */
	uStatus = OpcUa_Listener_Open(  pSecureListener->TransportListener,
									a_sUrl,
									OpcUa_SecureListener_OnNotify,
									a_pListener);
	OpcUa_GotoErrorIfBad(uStatus); 

	/* release lock */
	OpcUa_Mutex_Unlock(pSecureListener->Mutex);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	/* release lock on failure */
	OpcUa_Mutex_Unlock(pSecureListener->Mutex);

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_SecureListener_ProcessRequest
 *===========================================================================*/
/* looks at the message type header field in the input stream and calls the 
   appropriate handler */
static OpcUa_StatusCode OpcUa_SecureListener_ProcessRequest(
	OpcUa_Listener*         a_pSecureListenerInterface,
	OpcUa_Handle            a_hTransportConnection,
	OpcUa_InputStream**     a_ppTransportIstrm,
	OpcUa_Boolean           a_bRequestComplete)
{
	OpcUa_SecureListener*       pSecureListener                 = OpcUa_Null;
	OpcUa_SecureMessageType     requestType                     = OpcUa_SecureMessageType_UN;

OpcUa_InitializeStatus(OpcUa_Module_SecureListener, "ProcessRequest");

	/*** get SecureListener handle ***/
	pSecureListener = (OpcUa_SecureListener*)a_pSecureListenerInterface->Handle;
	if (pSecureListener)
	{
		/*** acquire lock until callback is complete. ***/
		OpcUa_Mutex_Lock(pSecureListener->Mutex); 

		OpcUa_ReturnErrorIfArgumentNull(a_pSecureListenerInterface);
		OpcUa_ReturnErrorIfArgumentNull(a_hTransportConnection);
		OpcUa_ReturnErrorIfArgumentNull(a_ppTransportIstrm);
		OpcUa_ReturnErrorIfArgumentNull(*a_ppTransportIstrm);


		/* shutdown check */
		OpcUa_GotoErrorIfTrue((pSecureListener->State != OpcUa_SecureListenerState_Open), OpcUa_BadShutdown);

		/*** check type of incoming service request ***/
		uStatus = OpcUa_SecureStream_CheckInputHeaderType(  *a_ppTransportIstrm, 
															&requestType);
		OpcUa_GotoErrorIfBad(uStatus);

		/* based on the service type, take the appropriate handler. */
		switch(requestType)
		{
		/* OpenSecureChannel */
		case OpcUa_SecureMessageType_SO:
			{
				uStatus = OpcUa_SecureListener_ProcessOpenSecureChannelRequest(     a_pSecureListenerInterface,
																					a_hTransportConnection,
																					a_ppTransportIstrm,
																					a_bRequestComplete);
				break;
			}
		/* CloseSecureChannel */
		case OpcUa_SecureMessageType_SC:
			{
				uStatus = OpcUa_SecureListener_ProcessCloseSecureChannelRequest(    a_pSecureListenerInterface,
																					a_hTransportConnection,
																					a_ppTransportIstrm,
																					a_bRequestComplete);
				break;
			}
		/* SecureMessage - standard protocol message */
		case OpcUa_SecureMessageType_SM:
			{
				uStatus = OpcUa_SecureListener_ProcessSessionCallRequest(           a_pSecureListenerInterface,
																					a_hTransportConnection,
																					a_ppTransportIstrm,
																					a_bRequestComplete);
				break;

			}
		/* Undefined - Error Status */
		default:
			{
				OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_WARNING, "ProcessRequest: Invalid message header detected!\n");
				OpcUa_GotoErrorWithStatus(OpcUa_Bad);
			}
		}

		OpcUa_GotoErrorIfBad(uStatus);
		/*** release lock. ***/
		OpcUa_Mutex_Unlock(pSecureListener->Mutex);
	}
	else
		uStatus=OpcUa_BadInvalidState;


OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	/*** release lock. ***/
	OpcUa_Mutex_Unlock(pSecureListener->Mutex);

	if(a_ppTransportIstrm != OpcUa_Null && *a_ppTransportIstrm != OpcUa_Null)
	{
		OpcUa_Stream_Close((OpcUa_Stream*)*a_ppTransportIstrm);
		OpcUa_Stream_Delete((OpcUa_Stream**)a_ppTransportIstrm);
	}

	if(OpcUa_IsBad(uStatus))
	{
		OpcUa_SecureChannel* pSecureChannel = OpcUa_Null;

		OpcUa_SecureListener_ChannelManager_GetChannelByTransportConnection(
			pSecureListener->ChannelManager,
			a_hTransportConnection,
			&pSecureChannel);

		if(pSecureChannel != OpcUa_Null)
		{
			if ((OpcUa_SecureMessageType_SC != requestType) &&
				(OpcUa_SecureMessageType_UN != requestType))
			{
				/*** invoke callback ***/
				if(pSecureListener->SecureChannelCallback)
				{
					/*OpcUa_Mutex_Unlock(pSecureListener->Mutex);*/
					pSecureListener->SecureChannelCallback( pSecureChannel->SecureChannelId,
															eOpcUa_SecureListener_SecureChannelClose,
															uStatus,
															OpcUa_Null,
															OpcUa_Null,
															0,
															0,
															pSecureListener->SecureChannelCallbackData);
					/*OpcUa_Mutex_Lock(pSecureListener->Mutex);*/
				}
				OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_ProcessRequest: Closing channel due error 0x%08X!\n", uStatus);
				pSecureChannel->Close(pSecureChannel);
			}

			pSecureChannel->LockWriteMutex(pSecureChannel);
			if(pSecureChannel->bAsyncWriteInProgress)
			{
				OpcUa_StatusCode uTempStatus = OpcUa_Good;
				uTempStatus = OpcUa_Listener_AddToSendQueue(pSecureListener->TransportListener,
															pSecureChannel->TransportConnection,
															pSecureChannel->pPendingSendBuffers,
															0);
				if(OpcUa_IsGood(uTempStatus))
				{
					pSecureChannel->bAsyncWriteInProgress = OpcUa_False;
					pSecureChannel->pPendingSendBuffers = OpcUa_Null;
				}
			}
			pSecureChannel->TransportConnection = OpcUa_Null;
			pSecureChannel->UnlockWriteMutex(pSecureChannel);
		}

		OpcUa_SecureListener_ChannelManager_ReleaseChannel(
			pSecureListener->ChannelManager,
			&pSecureChannel);
	}
	else
	{
		if(OpcUa_SecureMessageType_SC != requestType)
		{
			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_ProcessRequest: NOT closing channel due error 0x%08X! Ignored!\n", uStatus);
		}
	}

	/*** release lock. ***/
	/*OpcUa_Mutex_Unlock(pSecureListener->Mutex);*/

OpcUa_FinishErrorHandling;
}


/*============================================================================
 * OpcUa_SecureListener_AbortRequestAndClose
 *===========================================================================*/
/** @brief Delete the stream currently being received and close underlying connection if needed.  */
static OpcUa_StatusCode OpcUa_SecureListener_AbortRequestAndClose(  
	OpcUa_Listener*     a_pSecureListenerInterface, 
	OpcUa_Handle        a_hTransportConnection,
	OpcUa_StatusCode    a_uOperationStatus)
{
	OpcUa_SecureListener*   pSecureListener = OpcUa_Null;
	OpcUa_InputStream*      pSecureIStrm    = OpcUa_Null;
	OpcUa_SecureChannel*    pSecureChannel  = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_SecureListener, "AbortRequestAndClose");

	OpcUa_ReturnErrorIfArgumentNull(a_pSecureListenerInterface);
	OpcUa_ReturnErrorIfArgumentNull(a_hTransportConnection);

	OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_WARNING, "OpcUa_SecureListener_AbortRequestAndClose: Clearing current input stream. 0x%08X\n", a_uOperationStatus);

	/*** get listener handle ***/
	pSecureListener = (OpcUa_SecureListener*)a_pSecureListenerInterface->Handle;
	OpcUa_ReturnErrorIfNull(pSecureListener, OpcUa_BadInvalidState);

	uStatus = OpcUa_SecureListener_ChannelManager_GetChannelByTransportConnection(
				pSecureListener->ChannelManager,
				a_hTransportConnection,
				&pSecureChannel);
	OpcUa_GotoErrorIfBad(uStatus);

	/* look if there is a pending stream */
	uStatus = OpcUa_SecureChannel_GetPendingInputStream(    pSecureChannel,
															&pSecureIStrm);

	uStatus = OpcUa_SecureChannel_SetPendingInputStream(pSecureChannel,
														OpcUa_Null);

	if(pSecureIStrm != OpcUa_Null)
	{
		OpcUa_Stream_Delete((OpcUa_Stream**)&pSecureIStrm);
	}

	pSecureChannel->LockWriteMutex(pSecureChannel);
	if(pSecureChannel->bAsyncWriteInProgress)
	{
		uStatus = OpcUa_Listener_AddToSendQueue(pSecureListener->TransportListener,
												pSecureChannel->TransportConnection,
												pSecureChannel->pPendingSendBuffers,
												0);
		if(OpcUa_IsGood(uStatus))
		{
			pSecureChannel->bAsyncWriteInProgress = OpcUa_False;
			pSecureChannel->pPendingSendBuffers = OpcUa_Null;
		}
	}
	pSecureChannel->TransportConnection = OpcUa_Null;
	pSecureChannel->UnlockWriteMutex(pSecureChannel);

	OpcUa_SecureListener_ChannelManager_ReleaseChannel(
			pSecureListener->ChannelManager,
			&pSecureChannel);

	uStatus = pSecureListener->TransportListener->CloseConnection(  pSecureListener->TransportListener,
																	a_hTransportConnection,
																	a_uOperationStatus);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}


/*============================================================================
 * OpcUa_SecureListener_AbortRequest
 *===========================================================================*/
/** @brief Check the input stream and delete the stream currently being received
		   if everything is ok. Close underlying connection if needed. This is the 
		   handler for abort messages. */
static OpcUa_StatusCode OpcUa_SecureListener_AbortRequest(  
	OpcUa_Listener*         a_pSecureListenerInterface, 
	OpcUa_Handle            a_hTransportConnection,
	OpcUa_InputStream**     a_ppTransportIstrm,
	OpcUa_StatusCode        a_uOperationStatus)
{
	OpcUa_SecureListener*   pSecureListener         = OpcUa_Null;
	OpcUa_InputStream*      pSecureIStrm            = OpcUa_Null;
	OpcUa_SecureChannel*    pSecureChannel          = OpcUa_Null;
	OpcUa_SecureMessageType requestType             = OpcUa_SecureMessageType_UN;
	OpcUa_UInt32            uTokenId                = 0;
	OpcUa_UInt32            uSecureChannelId        = OPCUA_SECURECHANNEL_ID_INVALID;
	OpcUa_Boolean           bLock                   = OpcUa_False;
	OpcUa_SecurityKeyset*   pReceivingKeyset        = OpcUa_Null;
	OpcUa_CryptoProvider*   pCryptoProvider         = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_SecureListener, "AbortRequest");

	OpcUa_ReturnErrorIfArgumentNull(a_pSecureListenerInterface);
	OpcUa_ReturnErrorIfArgumentNull(a_hTransportConnection);
	OpcUa_ReturnErrorIfArgumentNull(a_ppTransportIstrm);

	OpcUa_ReferenceParameter(a_uOperationStatus);

	/*** get listener handle ***/
	pSecureListener = (OpcUa_SecureListener*)a_pSecureListenerInterface->Handle;
	OpcUa_ReturnErrorIfNull(pSecureListener, OpcUa_BadInvalidState);

	/*** check type of incoming service request ***/
	uStatus = OpcUa_SecureStream_CheckInputHeaderType(  *a_ppTransportIstrm, 
														&requestType);
	OpcUa_GotoErrorIfBad(uStatus);

	/* parse stream header */
	uStatus = OpcUa_SecureStream_DecodeSymmetricSecurityHeader( *a_ppTransportIstrm,
																&uSecureChannelId,
																&uTokenId);
	OpcUa_GotoErrorIfBad(uStatus);

	OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_AbortRequest: SID %u, TID %u, Status 0x%08X\n", uSecureChannelId, uTokenId, a_uOperationStatus);

	/*** acquire lock until callback is complete. ***/
	OpcUa_Mutex_Lock(pSecureListener->Mutex);
	bLock = OpcUa_True;

	uStatus = OpcUa_SecureListener_ChannelManager_GetChannelByTransportConnection(
				pSecureListener->ChannelManager,
				a_hTransportConnection,
				&pSecureChannel);
	OpcUa_GotoErrorIfBad(uStatus);

	/* look if there is a pending stream */
	uStatus = OpcUa_SecureChannel_GetPendingInputStream(pSecureChannel,
														&pSecureIStrm);
	OpcUa_GotoErrorIfBad(uStatus);

	/* Get reference to keyset for requested token id */
	uStatus = pSecureChannel->GetSecuritySet(   pSecureChannel,
												uTokenId,
												&pReceivingKeyset,
												OpcUa_Null,
												&pCryptoProvider);
	OpcUa_GotoErrorIfBad(uStatus);

	/* this is the final chunk */
	uStatus = OpcUa_SecureStream_AppendInput(   a_ppTransportIstrm,
												pSecureIStrm,
												&pReceivingKeyset->SigningKey,
												&pReceivingKeyset->EncryptionKey,
												&pReceivingKeyset->InitializationVector,
												pCryptoProvider);
	/* release reference to security set */
	pSecureChannel->ReleaseSecuritySet(   pSecureChannel,
										  uTokenId);

	OpcUa_GotoErrorIfBad(uStatus);

	OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_WARNING, "OpcUa_SecureListener_AbortRequest: Clearing current input stream.\n");

	OpcUa_SecureChannel_SetPendingInputStream(  pSecureChannel,
												OpcUa_Null);

	/* unlock resources */
	OpcUa_Mutex_Unlock(pSecureListener->Mutex);
	bLock = OpcUa_False;
	OpcUa_SecureListener_ChannelManager_ReleaseChannel(
				pSecureListener->ChannelManager,
				&pSecureChannel);

	OpcUa_Stream_Delete((OpcUa_Stream**)&pSecureIStrm);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	OpcUa_SecureListener_ChannelManager_ReleaseChannel(
			pSecureListener->ChannelManager,
			&pSecureChannel);

	if(a_ppTransportIstrm != OpcUa_Null)
	{
		(*a_ppTransportIstrm)->Delete((OpcUa_Stream**)a_ppTransportIstrm);
	}

	uStatus = OpcUa_SecureListener_AbortRequestAndClose(a_pSecureListenerInterface,
														a_hTransportConnection,
														uStatus);

	if(bLock)
	{
		OpcUa_Mutex_Unlock(pSecureListener->Mutex);
	}

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_SecureListener_OnNotify
 *===========================================================================*/
/* Gets called from the non secure listener on events. */
static OpcUa_StatusCode OpcUa_SecureListener_OnNotify(
	OpcUa_Listener*         a_pTransportListener,
	OpcUa_Void*             a_pCallbackData,
	OpcUa_ListenerEvent     a_eEvent,
	OpcUa_Handle            a_hTransportConnection,
	OpcUa_InputStream**     a_ppTransportIstrm,
	OpcUa_StatusCode        a_uOperationStatus)
{
	OpcUa_SecureListener*   pSecureListener = OpcUa_Null;
	OpcUa_SecureChannel*    pSecureChannel  = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_SecureListener, "OnNotify");
	
	OpcUa_ReturnErrorIfArgumentNull(a_pTransportListener);
	OpcUa_ReturnErrorIfArgumentNull(a_pCallbackData);

	/* this check must fail if this function is called after the listener is deleted. */
	OpcUa_ReturnErrorIfInvalidObject(OpcUa_SecureListener, (OpcUa_Listener*)a_pCallbackData, Open);

	pSecureListener = (OpcUa_SecureListener*)((OpcUa_Listener*)a_pCallbackData)->Handle;

	switch(a_eEvent)
	{
	case OpcUa_ListenerEvent_Open:
		{
			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_OnNotify: Transport Open\n");

			if(pSecureListener->Callback != OpcUa_Null)
			{
				pSecureListener->Callback(  (OpcUa_Listener*)a_pCallbackData,
											pSecureListener->CallbackData,
											a_eEvent,
											a_hTransportConnection,
											OpcUa_Null,
											a_uOperationStatus);
			}

			break;
		}
	case OpcUa_ListenerEvent_Close:
		{
			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_OnNotify: Transport Close\n");

			if(pSecureListener->Callback != OpcUa_Null)
			{
				pSecureListener->Callback(  (OpcUa_Listener*)a_pCallbackData,
											pSecureListener->CallbackData,
											a_eEvent,
											a_hTransportConnection,
											OpcUa_Null,
											a_uOperationStatus);
			}

			break;
		}
	case OpcUa_ListenerEvent_ChannelOpened:
		{
			OpcUa_UInt32 uReceiveBufferSize = 0;

			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_OnNotify: Transport Connection Opened\n");

			/* create TcpSecureChannel */
			uStatus = OpcUa_TcpSecureChannel_Create(&pSecureChannel);
			OpcUa_GotoErrorIfBad(uStatus);

			pSecureChannel->SecureChannelId = OPCUA_SECURECHANNEL_ID_INVALID;
			pSecureChannel->uOverlapCounter = (OpcUa_UInt32)OPCUA_SECURELISTENER_INACTIVECHANNELTIMEOUT;

			/* Calculate max number of chunks per message. */
			uStatus = OpcUa_Listener_GetReceiveBufferSize(  a_pTransportListener,
															a_hTransportConnection,
														   &uReceiveBufferSize);

			if(     OpcUa_IsBad(uStatus)
				||  (uReceiveBufferSize == 0))
			{
				OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_OnNotify (ChannelOpened): INTERNAL ERROR! Could not retrieve receive buffer size! Status 0x%08X, Size %u\n", uStatus, uReceiveBufferSize);
				OpcUa_GotoErrorWithStatus(OpcUa_BadInternalError);
			}

			pSecureChannel->nMaxBuffersPerMessage = OpcUa_ProxyStub_g_Configuration.iSerializer_MaxMessageSize/uReceiveBufferSize + 1;

			pSecureChannel->TransportConnection = a_hTransportConnection;

			/* Get the peer information from transport listener here - the listener is definitely valid in this context */ 
			uStatus = OpcUa_Listener_GetPeerInfo(pSecureListener->TransportListener,
												 pSecureChannel->TransportConnection,
												 &pSecureChannel->sPeerInfo);
			OpcUa_GotoErrorIfBad(uStatus);

			/* add SecureChannel to SecureChannelManager */
			uStatus = OpcUa_SecureListener_ChannelManager_AddChannel(pSecureListener->ChannelManager, pSecureChannel);
			OpcUa_GotoErrorIfBad(uStatus);

			break;
		}
	case OpcUa_ListenerEvent_ChannelClosed:
		{
			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_OnNotify: Transport Connection %p closed\n", a_hTransportConnection);
			
			/* get securechannel for the connection */
			uStatus = OpcUa_SecureListener_ChannelManager_GetChannelByTransportConnection(  pSecureListener->ChannelManager,
																							a_hTransportConnection,
																							&pSecureChannel);
			OpcUa_GotoErrorIfBad(uStatus);

			/* unlink connection handle from securechannel id! */
			/* this securechannel is invalid until a service with the corresponding securechannel id is called! */
			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_OnNotify: Transport Connection %p lost for SecureChannel %u\n", a_hTransportConnection, pSecureChannel->SecureChannelId);

			pSecureChannel->LockWriteMutex(pSecureChannel);
			if(pSecureChannel->bAsyncWriteInProgress)
			{
				uStatus = OpcUa_Listener_AddToSendQueue(pSecureListener->TransportListener,
														pSecureChannel->TransportConnection,
														pSecureChannel->pPendingSendBuffers,
														OPCUA_LISTENER_CLOSE_WHEN_DONE);
				if(OpcUa_IsGood(uStatus))
				{
					pSecureChannel->bAsyncWriteInProgress = OpcUa_False;
					pSecureChannel->pPendingSendBuffers = OpcUa_Null;
				}
			}
			pSecureChannel->TransportConnection = OpcUa_Null;
			pSecureChannel->UnlockWriteMutex(pSecureChannel);

			if(     OpcUa_Null != pSecureListener
				&&  OpcUa_Null != pSecureListener->SecureChannelCallback)
			{
				pSecureListener->SecureChannelCallback( pSecureChannel->SecureChannelId,
														eOpcUa_SecureListener_SecureChannelLostTransportConnection,
														OpcUa_Good,
														OpcUa_Null,
														OpcUa_Null,
														0,
														0,
														pSecureListener->SecureChannelCallbackData);
			}

			OpcUa_SecureListener_ChannelManager_ReleaseChannel( pSecureListener->ChannelManager,
																&pSecureChannel);

			break;
		}
	case OpcUa_ListenerEvent_Request:
		{
			if(OpcUa_IsBad(a_uOperationStatus))
			{
				/* called in case if an error occurs on lower layers */
				OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_WARNING, "OpcUa_SecureListener_OnNotify: Request with 0x%08X; aborting and closing connection ...\n", a_uOperationStatus);
				return OpcUa_SecureListener_AbortRequestAndClose(   (OpcUa_Listener*)a_pCallbackData, 
																	a_hTransportConnection,
																	a_uOperationStatus);
			}
			else
			{
				/* this call handles a standard request message */
				OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_OnNotify: Request\n");
				return OpcUa_SecureListener_ProcessRequest( (OpcUa_Listener*)a_pCallbackData, 
															a_hTransportConnection, 
															a_ppTransportIstrm,
															OpcUa_True);
			}
		}
	case OpcUa_ListenerEvent_RequestPartial:
		{
			if(OpcUa_IsBad(a_uOperationStatus))
			{
				/* called in case if an error occurs on lower layers */
				OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_WARNING, "OpcUa_SecureListener_OnNotify: RequestPartial with 0x%08X; aborting and closing connection ...\n", a_uOperationStatus);
				return OpcUa_SecureListener_AbortRequestAndClose(   (OpcUa_Listener*)a_pCallbackData, 
																	a_hTransportConnection,
																	a_uOperationStatus);
			}
			else
			{
				OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_OnNotify: RequestPartial\n");
				/* this call handles a standard request message */
				return OpcUa_SecureListener_ProcessRequest((OpcUa_Listener*)a_pCallbackData, 
															a_hTransportConnection, 
															a_ppTransportIstrm,
															OpcUa_False);
			}
		}
	case OpcUa_ListenerEvent_RequestAbort:
		{
			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_OnNotify: RequestAbort!\n");
			/* received an abort message for the current request stream */
			return OpcUa_SecureListener_AbortRequest(   (OpcUa_Listener*)a_pCallbackData, 
														a_hTransportConnection,
														a_ppTransportIstrm,
														a_uOperationStatus);
		}
	case OpcUa_ListenerEvent_UnexpectedError:
		{
			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_OnNotify: Unexpected error for transport connection %p\n", a_hTransportConnection);
			break;
		}
	case OpcUa_ListenerEvent_AsyncWriteComplete:
		{
			/* get securechannel for the connection */
			uStatus = OpcUa_SecureListener_ChannelManager_GetChannelByTransportConnection(  pSecureListener->ChannelManager,
																							a_hTransportConnection,
																							&pSecureChannel);
			OpcUa_GotoErrorIfBad(uStatus);

			pSecureChannel->LockWriteMutex(pSecureChannel);

			if(pSecureChannel->pPendingSendBuffers != OpcUa_Null)
			{
				OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_OnNotify: AsyncWriteComplete: Adding bufferlist to transport listener\n");
				uStatus = OpcUa_Listener_AddToSendQueue(pSecureListener->TransportListener,
														pSecureChannel->TransportConnection,
														pSecureChannel->pPendingSendBuffers,
														0);
				if(OpcUa_IsGood(uStatus))
				{
					pSecureChannel->pPendingSendBuffers     = OpcUa_Null;
					pSecureChannel->bAsyncWriteInProgress   = OpcUa_True;
					uStatus                                 = OpcUa_GoodCallAgain;
				}
			}
			else
			{
				OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_OnNotify: AsyncWriteComplete: Emptying bufferlist of transport listener\n");
				uStatus = OpcUa_Listener_AddToSendQueue(pSecureListener->TransportListener,
														pSecureChannel->TransportConnection,
														OpcUa_Null,
														0);
				if(OpcUa_IsGood(uStatus))
				{
					pSecureChannel->bAsyncWriteInProgress   = OpcUa_False;
					pSecureChannel->pPendingSendBuffers     = OpcUa_Null;
				}
			}

			pSecureChannel->UnlockWriteMutex(pSecureChannel);

			OpcUa_SecureListener_ChannelManager_ReleaseChannel( pSecureListener->ChannelManager,
																&pSecureChannel);

			break;
		}
	default:
		{
			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_OnNotify: Default\n");
			break;
		}
	}

	/* all events except Requests: */
	/* acquire lock until callback is complete. */
	OpcUa_Mutex_Lock(pSecureListener->Mutex);

	/* no longer open due to either an error or a close. */
	if (OpcUa_IsBad(a_uOperationStatus))
	{
		pSecureListener->State = OpcUa_SecureListenerState_Closed;
	}
	else
	{
		/* set state to open if opened successfully. */
		if(a_eEvent == OpcUa_ListenerEvent_Open)
		{
			pSecureListener->State = OpcUa_SecureListenerState_Open;
		}
	}

	/* release lock. */
	OpcUa_Mutex_Unlock(pSecureListener->Mutex);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_SecureListener_Close
 *===========================================================================*/
OpcUa_StatusCode OpcUa_SecureListener_Close(OpcUa_Listener* a_pListener)
{
	OpcUa_SecureListener* pSecureListener = OpcUa_Null;
	OpcUa_Boolean bLock = OpcUa_False;
OpcUa_InitializeStatus(OpcUa_Module_SecureListener, "Close");
	
	OpcUa_ReturnErrorIfArgumentNull(a_pListener);
	OpcUa_ReturnErrorIfInvalidObject(OpcUa_SecureListener, a_pListener, Close);

	pSecureListener = (OpcUa_SecureListener*)a_pListener->Handle;

	/* acquire lock until open is complete */
	OpcUa_Mutex_Lock(pSecureListener->Mutex);
	bLock = OpcUa_True;
	if (pSecureListener->State != OpcUa_SecureListenerState_Open)
	{
		uStatus = OpcUa_BadInvalidState;
		OpcUa_GotoErrorIfBad(uStatus);
	}

	pSecureListener->State = OpcUa_SecureListenerState_Closed;

	/* cleanup all channels */

	/* release lock */
	OpcUa_Mutex_Unlock(pSecureListener->Mutex);
	bLock = OpcUa_False;
	/* close the non-secure listener */
	uStatus = OpcUa_Listener_Close(pSecureListener->TransportListener);
	OpcUa_ReturnErrorIfBad(uStatus);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	/* release lock on failure */
	if (bLock)
		OpcUa_Mutex_Unlock(pSecureListener->Mutex);

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_SecureListener_GetSecureChannelId
 *===========================================================================*/
OpcUa_StatusCode OpcUa_SecureListener_GetSecureChannelId(
	OpcUa_InputStream*  a_pSecureIstrm,
	OpcUa_UInt32*       a_pSecureChannelId)
{
	OpcUa_SecureStream* pSecureStream = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_SecureListener, "GetSecureChannelId");
	
	OpcUa_ReturnErrorIfArgumentNull(a_pSecureIstrm);
	OpcUa_ReturnErrorIfArgumentNull(a_pSecureChannelId);

	pSecureStream = (OpcUa_SecureStream*)a_pSecureIstrm->Handle;

	*a_pSecureChannelId = pSecureStream->SecureChannelId;

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_SecureListener_BeginSendResponse
 *===========================================================================*/
OpcUa_StatusCode OpcUa_SecureListener_BeginSendResponse(OpcUa_Listener*      a_pListener,
														OpcUa_Handle         a_hSecureConnection,
														OpcUa_InputStream**  a_ppSecureIstrm,
														OpcUa_OutputStream** a_ppSecureOstrm)
{
	OpcUa_SecureListener*   pSecureListener         = OpcUa_Null;
	OpcUa_SecureStream*     pSecureInputStream      = OpcUa_Null;
	OpcUa_OutputStream*     pTransportOstrm         = OpcUa_Null;
	OpcUa_SecureChannel*    pSecureChannel          = OpcUa_Null;
	OpcUa_Boolean           bStreamCountIncreased   = OpcUa_False;

OpcUa_InitializeStatus(OpcUa_Module_SecureListener, "BeginSendResponse");
	
	OpcUa_ReturnErrorIfArgumentNull(a_pListener);
	OpcUa_ReturnErrorIfArgumentNull(a_ppSecureIstrm);
	OpcUa_ReturnErrorIfArgumentNull(a_ppSecureOstrm);
	OpcUa_ReturnErrorIfArgumentNull(*a_ppSecureIstrm);
	OpcUa_ReturnErrorIfInvalidObject(OpcUa_SecureListener, a_pListener, BeginSendResponse);

	*a_ppSecureOstrm    = OpcUa_Null;
	pSecureListener     = (OpcUa_SecureListener*)a_pListener->Handle;
	pSecureInputStream  = (OpcUa_SecureStream*)(*a_ppSecureIstrm)->Handle;
	if (!pSecureInputStream)
	{
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_BeginSendResponse: Internal error.pSecureInputStream is NULL\n");
		if (pSecureChannel)
			OPCUA_SECURECHANNEL_UNLOCK(pSecureChannel);
		OpcUa_GotoErrorWithStatus(OpcUa_BadInternalError);
	}
	/*** get appropriate SecureChannel ***/
	uStatus = OpcUa_SecureListener_ChannelManager_GetChannelBySecureChannelID(  pSecureListener->ChannelManager,
																				pSecureInputStream->SecureChannelId,
																				&pSecureChannel);
	OpcUa_GotoErrorIfBad(uStatus);

	/* close incoming stream */
	uStatus = OpcUa_Stream_Close((OpcUa_Stream*)(*a_ppSecureIstrm));
	OpcUa_GotoErrorIfBad(uStatus);

	/* check whether SecureChannel is in a correct state */
	OPCUA_SECURECHANNEL_LOCK(pSecureChannel);

	pSecureChannel->uNumberOfOutputStreams++;
	OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_BeginSendResponse: %u streams now active at channel %u\n", pSecureChannel->uNumberOfOutputStreams, pSecureChannel->SecureChannelId);
	bStreamCountIncreased = OpcUa_True;

	if(pSecureChannel->uNumberOfOutputStreams == 0)
	{
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_BeginSendResponse: Internal error. Number of output data streams too high! (0x%08X)", uStatus, pSecureChannel->uNumberOfOutputStreams);
		OPCUA_SECURECHANNEL_UNLOCK(pSecureChannel);
		OpcUa_GotoErrorWithStatus(OpcUa_BadInternalError);
	}

	if(pSecureChannel->State  != OpcUa_SecureChannelState_Opened)
	{
		OPCUA_SECURECHANNEL_UNLOCK(pSecureChannel);
		OpcUa_GotoErrorWithStatus(OpcUa_BadInvalidState);
	}

	/* check for valid transport connection */
	if((pSecureChannel->TransportConnection == OpcUa_Null))
	{
		OPCUA_SECURECHANNEL_UNLOCK(pSecureChannel);
		OpcUa_GotoErrorWithStatus(OpcUa_BadNotConnected);
	}

	OPCUA_SECURECHANNEL_UNLOCK(pSecureChannel);

	/* create inner (transport) stream */
	uStatus = OpcUa_Listener_BeginSendResponse( pSecureListener->TransportListener,
												a_hSecureConnection, 
												(OpcUa_InputStream**)&pSecureInputStream->InnerStrm,
												&pTransportOstrm);
	OpcUa_GotoErrorIfBad(uStatus);

	/* create output stream */
	uStatus = OpcUa_SecureStream_CreateOutput(  pTransportOstrm,
												eOpcUa_SecureStream_Types_StandardMessage,
												pSecureInputStream->RequestId,
												pSecureChannel,
												a_ppSecureOstrm);
	OpcUa_GotoErrorIfBad(uStatus);

	/* delete stream */
	OpcUa_Stream_Delete((OpcUa_Stream**)a_ppSecureIstrm);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	OpcUa_SecureListener_ChannelManager_ReleaseChannel(
			pSecureListener->ChannelManager,
			&pSecureChannel);

	OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_BeginSendResponse: fail with 0x%08X\n", uStatus);

	if(pSecureChannel != OpcUa_Null && bStreamCountIncreased != OpcUa_False)
	{
		OPCUA_SECURECHANNEL_LOCK(pSecureChannel);
		pSecureChannel->uNumberOfOutputStreams--;
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_BeginSendResponse: %u streams remaining at channel %u.\n", pSecureChannel->uNumberOfOutputStreams, pSecureChannel->SecureChannelId);
		OPCUA_SECURECHANNEL_UNLOCK(pSecureChannel);
	}

	if (pSecureInputStream->InnerStrm != OpcUa_Null)
	{
		pSecureInputStream->InnerStrm->Close(pSecureInputStream->InnerStrm);
		pSecureInputStream->InnerStrm->Delete(&pSecureInputStream->InnerStrm);
	}

	/* delete stream */
	OpcUa_Stream_Delete((OpcUa_Stream**)a_ppSecureIstrm);

	if(pTransportOstrm != OpcUa_Null)
	{
		OpcUa_Stream_Delete((OpcUa_Stream**)&pTransportOstrm);
	}

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_SecureListener_WriteResponse
 *===========================================================================*/
static OpcUa_StatusCode OpcUa_SecureListener_WriteResponse( OpcUa_SecureListener*   a_pSecureListener,
															OpcUa_OutputStream**    a_ppOstrm,
															OpcUa_Void*             a_pResponse,    
															OpcUa_EncodeableType*   a_pResponseType)
{
	OpcUa_Encoder*          pEncoder        = OpcUa_Null;
	OpcUa_MessageContext    cContext;
	OpcUa_Handle            hEncodeContext  = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_SecureListener, "WriteResponse");
	
	OpcUa_ReturnErrorIfArgumentNull(a_pResponse);
	OpcUa_ReturnErrorIfArgumentNull(a_ppOstrm);
	OpcUa_ReturnErrorIfArgumentNull(*a_ppOstrm);
	OpcUa_ReturnErrorIfArgumentNull(a_pResponse);
	OpcUa_ReturnErrorIfArgumentNull(a_pResponseType);

	pEncoder = a_pSecureListener->Encoder;
	OpcUa_ReturnErrorIfArgumentNull(pEncoder);
	
	OpcUa_MessageContext_Initialize(&cContext);
	
	cContext.KnownTypes         = a_pSecureListener->KnownTypes;
	cContext.NamespaceUris      = a_pSecureListener->NamespaceUris;
	cContext.AlwaysCheckLengths = OPCUA_SERIALIZER_CHECKLENGTHS; 
	
	/* open encoder */
	uStatus = pEncoder->Open(pEncoder, *a_ppOstrm, &cContext, &hEncodeContext);
	OpcUa_GotoErrorIfBad(uStatus);

	/* encode message */
	uStatus = pEncoder->WriteMessage((struct _OpcUa_Encoder*)hEncodeContext, a_pResponse, a_pResponseType);
	OpcUa_GotoErrorIfBad(uStatus);

	/* delete encoder */
	OpcUa_Encoder_Close(pEncoder, &hEncodeContext);
	OpcUa_MessageContext_Clear(&cContext);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	OpcUa_Encoder_Close(pEncoder, &hEncodeContext);
	OpcUa_MessageContext_Clear(&cContext);

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_SecureListener_SendDelayedOpenSecureChannelResponse
 *===========================================================================*/
static OpcUa_StatusCode OpcUa_SecureListener_SendDelayedOpenSecureChannelResponse(  OpcUa_Listener*         a_pListener,
																					OpcUa_SecureChannel*    a_pSecureChannel)
{
	OpcUa_OutputStream*                 pSecureOstrm                = OpcUa_Null;
	OpcUa_SecureListener*               pSecureListener             = (OpcUa_SecureListener*)a_pListener->Handle;
	OpcUa_OpenSecureChannelResponse*    pResponse                   = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_SecureListener, "SendDelayedOpenSecureChannelResponse");

	if(a_pSecureChannel->pOSCOutputStream == OpcUa_Null)
	{
		/*OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_SendDelayedOpenSecureChannelResponse: No response to send!\n");*/
		OpcUa_ReturnStatusCode;
	}

	OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_SendDelayedOpenSecureChannelResponse: Sending response!\n");

	pSecureOstrm                        = (OpcUa_OutputStream*) a_pSecureChannel->pOSCOutputStream;
	pResponse                           = (OpcUa_OpenSecureChannelResponse*) a_pSecureChannel->pOSCMessage;
	a_pSecureChannel->pOSCOutputStream  = OpcUa_Null;
	a_pSecureChannel->pOSCMessage       = OpcUa_Null;

	/*** encode response body ***/
	uStatus = OpcUa_SecureListener_WriteResponse(   pSecureListener,
													&pSecureOstrm,
													(OpcUa_Void*)pResponse, 
													&OpcUa_OpenSecureChannelResponse_EncodeableType);
	OpcUa_GotoErrorIfBad(uStatus);

	/*** end send response ***/
	uStatus = OpcUa_SecureListener_EndSendOpenSecureChannelResponse(    a_pListener,
																		&pSecureOstrm,
																		OpcUa_Good);
	OpcUa_GotoErrorIfBad(uStatus);

	a_pSecureChannel->bCurrentTokenActive = OpcUa_True;

	if(pResponse != OpcUa_Null)
	{
		OpcUa_OpenSecureChannelResponse_Clear(pResponse);
		OpcUa_Free(pResponse);
	}

	OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_SendDelayedOpenSecureChannelResponse: Done!\n");

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_SendDelayedOpenSecureChannelResponse: Error 0x%08X!\n", uStatus);

	if(pResponse != OpcUa_Null)
	{
		OpcUa_OpenSecureChannelResponse_Clear(pResponse);
		OpcUa_Free(pResponse);
	}

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_SecureListener_EndSendResponse
 *===========================================================================*/
OpcUa_StatusCode OpcUa_SecureListener_EndSendResponse(
	OpcUa_Listener*         a_pListener,
	OpcUa_StatusCode        a_uStatus,
	OpcUa_OutputStream**    a_ppOstrm)
{
	OpcUa_SecureListener*   pSecureListener = OpcUa_Null;
	OpcUa_SecureStream*     pSecureStream   = OpcUa_Null;
	OpcUa_SecureChannel*    pSecureChannel  = OpcUa_Null;
	OpcUa_Boolean           bListenerLocked = OpcUa_False;

OpcUa_InitializeStatus(OpcUa_Module_SecureListener, "EndSendResponse");

	OpcUa_ReturnErrorIfArgumentNull(a_pListener);
	OpcUa_ReturnErrorIfArgumentNull(a_ppOstrm);

	OpcUa_ReturnErrorIfInvalidObject(OpcUa_SecureListener, a_pListener, EndSendResponse);

	pSecureListener     = (OpcUa_SecureListener*)a_pListener->Handle;
	pSecureStream       = (OpcUa_SecureStream*)(*a_ppOstrm)->Handle;

	OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_EndSendResponse: ID %u, Status 0x%08X\n", pSecureStream->RequestId, a_uStatus);

	/*** acquire lock until callback is complete. ***/
	OpcUa_Mutex_Lock(pSecureListener->Mutex);
	bListenerLocked = OpcUa_True;

	/* find securechannel and look for transport validation */
	uStatus = OpcUa_SecureListener_ChannelManager_GetChannelBySecureChannelID(  pSecureListener->ChannelManager,
																				pSecureStream->SecureChannelId,
																				&pSecureChannel);
	OpcUa_GotoErrorIfBad(uStatus);    

	/* check if this message context has a valid secure channel */
	if(pSecureChannel == OpcUa_Null)
	{
		OpcUa_GotoErrorWithStatus(OpcUa_BadTcpSecureChannelUnknown);
	}

	/* check if the channel has a valid transport connection. */
	if(pSecureChannel->TransportConnection == OpcUa_Null)
	{
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_EndSendResponse: SecureChannel %u has no transport connection!\n", pSecureChannel->SecureChannelId);
		OpcUa_GotoErrorWithStatus(OpcUa_BadNoCommunication);
	}

	OpcUa_Mutex_Unlock(pSecureListener->Mutex);
	bListenerLocked = OpcUa_False;

	OPCUA_SECURECHANNEL_LOCK(pSecureChannel);
	if(pSecureChannel->uNumberOfOutputStreams == 0)
	{
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_EndSendResponse: Inconsistent state! No data streams active!\n");
		OPCUA_SECURECHANNEL_UNLOCK(pSecureChannel);
		OpcUa_GotoErrorWithStatus(OpcUa_BadInternalError);
	}

	pSecureChannel->uNumberOfOutputStreams--;
	OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_EndSendResponse: %u stream remaining at channel %u!\n", pSecureChannel->uNumberOfOutputStreams, pSecureChannel->SecureChannelId);
	OPCUA_SECURECHANNEL_UNLOCK(pSecureChannel);

	if(OpcUa_IsGood(a_uStatus))
	{
		/*** close secure stream - triggers send of last message chunk -> no network access after this call. ***/
		uStatus = ((OpcUa_Stream*)(*a_ppOstrm))->Close((OpcUa_Stream*)(*a_ppOstrm));

		/* check if last flush was successful */
		if(OpcUa_IsBad(uStatus))
		{
			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_WARNING, "OpcUa_SecureListener_EndSendResponse: closing transport stream reported error 0x%08X!\n", uStatus);

			/*** finish call at the transport listener - clean transport stream object ***/
			OpcUa_Listener_EndSendResponse( pSecureListener->TransportListener,
											OpcUa_BadDisconnect,
											(OpcUa_OutputStream**)&pSecureStream->InnerStrm);
		}
		else
		{
			/*** finish call at the transport listener - clean transport stream object ***/
			uStatus = OpcUa_Listener_EndSendResponse(   pSecureListener->TransportListener,
														OpcUa_Good,
														(OpcUa_OutputStream**)&pSecureStream->InnerStrm);
		}
	}
	else
	{
		/* bad status code means, the application could not handle a problem -> trigger error message */
		/*** finish call at the transport listener - clean transport stream object ***/
		uStatus = OpcUa_Listener_EndSendResponse(   pSecureListener->TransportListener,
													a_uStatus,
													(OpcUa_OutputStream**)&pSecureStream->InnerStrm);
	}

	((OpcUa_Stream*)(*a_ppOstrm))->Delete((OpcUa_Stream**)(a_ppOstrm));

	/* check for delayed OSC response */
	if(OpcUa_IsGood(uStatus))
	{
		OPCUA_SECURECHANNEL_LOCK(pSecureChannel);
		OpcUa_SecureListener_SendDelayedOpenSecureChannelResponse(  a_pListener,
																	pSecureChannel);
		OPCUA_SECURECHANNEL_UNLOCK(pSecureChannel);
	}
	else
	{
		OpcUa_GotoError;
	}

	OpcUa_SecureListener_ChannelManager_ReleaseChannel(  pSecureListener->ChannelManager,
														&pSecureChannel);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	OpcUa_SecureListener_ChannelManager_ReleaseChannel(  pSecureListener->ChannelManager,
														&pSecureChannel);

	OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_EndSendResponse: Error 0x%08X while sending message\n", uStatus);
	if(bListenerLocked)
		OpcUa_Mutex_Unlock(pSecureListener->Mutex);

OpcUa_FinishErrorHandling;
} /* OpcUa_SecureListener_EndSendResponse */

/*============================================================================
 * OpcUa_SecureListener_AbortSendResponse
 *===========================================================================*/
OpcUa_StatusCode OpcUa_SecureListener_AbortSendResponse(
	OpcUa_Listener*         a_pListener,
	OpcUa_StatusCode        a_uStatus,
	OpcUa_String*           a_psReason,
	OpcUa_OutputStream**    a_ppOstrm)
{
	OpcUa_SecureListener*   pSecureListener = OpcUa_Null;
	OpcUa_SecureStream*     pSecureStream   = OpcUa_Null;
	OpcUa_SecureChannel*    pSecureChannel  = OpcUa_Null;
	OpcUa_String            sReason         = OPCUA_STRING_STATICINITIALIZER;

OpcUa_InitializeStatus(OpcUa_Module_SecureListener, "AbortSendResponse");

	OpcUa_ReturnErrorIfArgumentNull(a_pListener);
	OpcUa_ReturnErrorIfArgumentNull(a_ppOstrm);
	OpcUa_ReturnErrorIfArgumentNull(*a_ppOstrm);

	OpcUa_ReturnErrorIfInvalidObject(OpcUa_SecureListener, a_pListener, AbortSendResponse);

	pSecureListener     = (OpcUa_SecureListener*)a_pListener->Handle;
	pSecureStream       = (OpcUa_SecureStream*)(*a_ppOstrm)->Handle;
	OpcUa_ReturnErrorIfArgumentNull(pSecureStream);

	OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_AbortSendResponse: called for used stream! Triggering Abort Message!\n");

	OpcUa_Mutex_Lock(pSecureListener->Mutex);

	/* find securechannel and look for transport validation */
	OpcUa_SecureListener_ChannelManager_GetChannelBySecureChannelID(pSecureListener->ChannelManager,
																	pSecureStream->SecureChannelId,
																	&pSecureChannel);

	OpcUa_Mutex_Unlock(pSecureListener->Mutex);

	if(pSecureChannel != OpcUa_Null)
	{
		OPCUA_SECURECHANNEL_LOCK(pSecureChannel);
		if(pSecureChannel->uNumberOfOutputStreams > 0)
		{
			pSecureChannel->uNumberOfOutputStreams--;
			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_AbortSendResponse: %u streams remaining at channel %u!\n", pSecureChannel->uNumberOfOutputStreams, pSecureChannel->SecureChannelId);
		}
		OPCUA_SECURECHANNEL_UNLOCK(pSecureChannel);
	}

	/* check if abort message needs to be sent. */
	if(pSecureStream->uNoOfFlushes != 0 && OpcUa_IsBad(a_uStatus))
	{
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_AbortSendResponse: called for used stream! Triggering Abort Message!\n");

		/* mark as abort type message */
		pSecureStream->Buffers[0].Data[3] = 'A';

		/* set internal pointer right after header elements */
		OpcUa_Buffer_SetPosition(&pSecureStream->Buffers[0], pSecureStream->uBeginOfRequestBody);
		pSecureStream->Buffers[0].EndOfData = pSecureStream->uBeginOfRequestBody;
		OpcUa_UInt32_BinaryEncode(a_uStatus, (*a_ppOstrm));
		OpcUa_String_BinaryEncode(a_psReason?a_psReason:&sReason, (*a_ppOstrm));
		uStatus = (*a_ppOstrm)->Close((OpcUa_Stream*)(*a_ppOstrm));
		if(OpcUa_IsBad(uStatus))
		{
			/* if abort message could not be sent, the message pipeline is corrupted */
			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_AbortSendResponse: Could not send abort message!\n");
		}
	}

	/* delete transport stream */
	pSecureListener->TransportListener->AbortSendResponse(  pSecureListener->TransportListener,
															a_uStatus,
															a_psReason?a_psReason:&sReason,
															(OpcUa_OutputStream**)&pSecureStream->InnerStrm);
	OpcUa_Stream_Delete((OpcUa_Stream**)a_ppOstrm);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
} /* OpcUa_SecureListener_AbortSendResponse */

/*============================================================================
 * OpcUa_SecureListener_BeginSendOpenSecureChannelResponse
 *===========================================================================*/
static OpcUa_StatusCode OpcUa_SecureListener_BeginSendOpenSecureChannelResponse(
	OpcUa_Listener*             a_pListener,
	OpcUa_SecureChannel*        a_pSecureChannel,
	OpcUa_InputStream*          a_pSecureIstrm,
	OpcUa_String*               a_pSecurityPolicyUri,
	OpcUa_MessageSecurityMode   a_MessageSecurityMode,
	OpcUa_CryptoProvider*       a_pCryptoProvider,
	OpcUa_ByteString*           a_pServerCertificate,
	OpcUa_ByteString*           a_pServerCertificateChain,
	OpcUa_ByteString*           a_pClientCertificateThumbprint,
	OpcUa_OutputStream**        a_ppSecureOstrm)
{
	OpcUa_SecureListener*   pSecureListener = OpcUa_Null;
	OpcUa_SecureStream*     pSecureStream   = OpcUa_Null;
	OpcUa_OutputStream*     pInnerOstrm     = OpcUa_Null;
	
OpcUa_InitializeStatus(OpcUa_Module_SecureListener, "BeginSendOpenSecureChannelResponse");

	*a_ppSecureOstrm = OpcUa_Null;
	
	pSecureListener = (OpcUa_SecureListener*)a_pListener->Handle;
	pSecureStream   = (OpcUa_SecureStream*)a_pSecureIstrm->Handle;

	/* close incoming stream */
	uStatus = a_pSecureIstrm->Close((OpcUa_Stream*)a_pSecureIstrm);
	OpcUa_GotoErrorIfBad(uStatus);

	uStatus = pSecureListener->TransportListener->BeginSendResponse(    pSecureListener->TransportListener,
																		a_pSecureChannel->TransportConnection,
																		(OpcUa_InputStream**)&pSecureStream->InnerStrm,
																		&pInnerOstrm);
	OpcUa_GotoErrorIfBad(uStatus);
	
	/* create output stream */
	uStatus = OpcUa_SecureStream_CreateOpenSecureChannelOutput( pInnerOstrm,
																a_pSecureChannel,
																pSecureStream->RequestId,
																a_pSecurityPolicyUri,
																a_MessageSecurityMode,
																a_pCryptoProvider,
																a_pServerCertificate,
																a_pServerCertificateChain,
																&pSecureListener->ServerPrivateKey,
																&a_pSecureChannel->ClientCertificate,
																a_pClientCertificateThumbprint,
																a_ppSecureOstrm);
	OpcUa_GotoErrorIfBad(uStatus);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	if(pInnerOstrm != OpcUa_Null)
	{
		OpcUa_Stream_Delete((OpcUa_Stream**)&pInnerOstrm);
		pInnerOstrm = OpcUa_Null;
	}

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_SecureListener_EndSendOpenSecureChannelResponse
 *===========================================================================*/
static OpcUa_StatusCode OpcUa_SecureListener_EndSendOpenSecureChannelResponse(
	OpcUa_Listener*             a_pListener,
	OpcUa_OutputStream**        a_ppOstrm,
	OpcUa_StatusCode            a_uStatus)
{
	OpcUa_SecureListener*   pSecureListener     = OpcUa_Null;
	OpcUa_SecureStream*     pSecureStream       = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_SecureListener, "EndSendOpenSecureChannelResponse");

	OpcUa_ReturnErrorIfArgumentNull(a_pListener);    
	OpcUa_ReturnErrorIfArgumentNull(a_ppOstrm);
	OpcUa_ReturnErrorIfArgumentNull(*a_ppOstrm);

	OpcUa_ReturnErrorIfInvalidObject(OpcUa_SecureListener, a_pListener, EndSendResponse);

	pSecureListener = (OpcUa_SecureListener*)a_pListener->Handle;
	pSecureStream   = (OpcUa_SecureStream*)(*a_ppOstrm)->Handle;

	/*** close the secure stream ***/
	uStatus = (*a_ppOstrm)->Close((OpcUa_Stream*)(*a_ppOstrm));
	OpcUa_GotoErrorIfBad(uStatus);

	/*** send response over the wire ***/
	uStatus = pSecureListener->TransportListener->EndSendResponse(  pSecureListener->TransportListener,
																	a_uStatus,
																	(OpcUa_OutputStream**)&(pSecureStream->InnerStrm));
	OpcUa_GotoErrorIfBad(uStatus);

	/*** cleanup ***/
	if((*a_ppOstrm) != OpcUa_Null)
	{
		(*a_ppOstrm)->Delete((OpcUa_Stream**)a_ppOstrm);
	}

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	if((*a_ppOstrm) != OpcUa_Null)
	{
		(*a_ppOstrm)->Delete((OpcUa_Stream**)a_ppOstrm);
	}

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_SecureListener_Delete
 *===========================================================================*/
OpcUa_Void OpcUa_SecureListener_Delete(OpcUa_Listener** a_ppListener)
{
	OpcUa_SecureListener*   pSecureListener = OpcUa_Null;

	if (a_ppListener == OpcUa_Null || *a_ppListener == OpcUa_Null)
	{
		return;
	}

	pSecureListener = (OpcUa_SecureListener*)(*a_ppListener)->Handle;

	OpcUa_Mutex_Lock(pSecureListener->Mutex);

#if OPCUA_SECURELISTENER_SUPPORT_THREADPOOL
	if(OpcUa_ProxyStub_g_Configuration.bSecureListener_ThreadPool_Enabled != OpcUa_False)
	{
		OpcUa_ThreadPool_Delete(&pSecureListener->hThreadPool);
	}
#endif /* OPCUA_SECURELISTENER_SUPPORT_THREADPOOL */

	if(pSecureListener->ChannelManager != OpcUa_Null)
	{
		OpcUa_SecureListener_ChannelManager_Delete(&pSecureListener->ChannelManager);
	}

	if(pSecureListener->PolicyManager != OpcUa_Null)
	{
		OpcUa_SecureListener_PolicyManager_Delete(&pSecureListener->PolicyManager);
	}

	if(pSecureListener->ServerPKIProvider != OpcUa_Null)
	{
		OPCUA_P_PKIFACTORY_DELETEPKIPROVIDER(pSecureListener->ServerPKIProvider);
		OpcUa_Free(pSecureListener->ServerPKIProvider);
		pSecureListener->ServerPKIProvider = OpcUa_Null;
	}

	if(pSecureListener->absServerCertificateChain != OpcUa_Null)
	{
		OpcUa_Free(pSecureListener->absServerCertificateChain);
	}
	pSecureListener->pServerCertificate = OpcUa_Null;
	pSecureListener->SanityCheck = 0;

	OpcUa_Mutex_Unlock(pSecureListener->Mutex);

	OpcUa_Mutex_Delete(&(pSecureListener->Mutex));
	
	OpcUa_Free(pSecureListener);
	pSecureListener = OpcUa_Null;

	OpcUa_Free(*a_ppListener);
	*a_ppListener = OpcUa_Null;
}

/*============================================================================
 * OpcUa_SecureListener_ChannelRemovedCallback
 *===========================================================================*/
OpcUa_Void OPCUA_DLLCALL OpcUa_SecureListener_ChannelRemovedCallback(   OpcUa_SecureChannel*    a_pSecureChannel,
																		OpcUa_Void*             a_pvCallbackData)
{
	OpcUa_SecureListener* pSecureListener = (OpcUa_SecureListener*)a_pvCallbackData;

	OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_ChannelRemovedCallback: SecureChannel %u timed out!\n", a_pSecureChannel->SecureChannelId);

	if(a_pSecureChannel->TransportConnection != OpcUa_Null)
	{
		pSecureListener->TransportListener->CloseConnection(    pSecureListener->TransportListener,
																a_pSecureChannel->TransportConnection,
																OpcUa_Good);
	}
	else
	{
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_ChannelRemovedCallback: SecureChannel %u has no transport connection set!\n", a_pSecureChannel->SecureChannelId);
	}

	if(     OpcUa_Null != pSecureListener 
		&&  OpcUa_Null != pSecureListener->SecureChannelCallback)
	{
		pSecureListener->SecureChannelCallback( a_pSecureChannel->SecureChannelId,
												eOpcUa_SecureListener_SecureChannelClose,
												OpcUa_BadSecureChannelClosed,
												OpcUa_Null,
												OpcUa_Null,
												0,
												0,
												pSecureListener->SecureChannelCallbackData);
	}
}

/*============================================================================
 * OpcUa_SecureListener_CloseConnection
 *===========================================================================*/
OpcUa_StatusCode OpcUa_SecureListener_CloseConnection(
	OpcUa_Listener*         a_pListener,
	OpcUa_Handle            a_hConnection,
	OpcUa_StatusCode        a_uStatus)
{
	OpcUa_SecureListener*   pSecureListener     = OpcUa_Null;
	OpcUa_SecureChannel*    pSecureChannel      = OpcUa_Null;
	OpcUa_UInt32            uSecureChannelId    = (OpcUa_UInt32)a_hConnection;

OpcUa_InitializeStatus(OpcUa_Module_SecureListener, "CloseConnection");

	OpcUa_ReturnErrorIfArgumentNull(a_pListener);
	OpcUa_ReferenceParameter(a_uStatus);

	pSecureListener = (OpcUa_SecureListener*)a_pListener->Handle;

	/* get the securechannel with the given channel id */
	uStatus = OpcUa_SecureListener_ChannelManager_GetChannelBySecureChannelID(  pSecureListener->ChannelManager,
																				uSecureChannelId,
																			   &pSecureChannel);

	if(OpcUa_IsGood(uStatus) && pSecureChannel != OpcUa_Null)
	{
		uStatus = pSecureChannel->Close(pSecureChannel);
		if(OpcUa_IsBad(uStatus))
		{
			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_WARNING, "OpcUa_SecureListener_CloseConnection: Could not close secure channel with id %u found.\n", uSecureChannelId);
		}

		OpcUa_SecureListener_ChannelManager_ReleaseChannel(  pSecureListener->ChannelManager,
															&pSecureChannel);
	}
	else
	{
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_WARNING, "OpcUa_SecureListener_CloseConnection: No secure channel with id %u found.\n", uSecureChannelId);
	}

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_SecureListener_Create
 *===========================================================================*/
OpcUa_StatusCode OpcUa_SecureListener_Create(   
	OpcUa_Listener*                                     a_pInnerListener,
	OpcUa_Decoder*                                      a_pDecoder,
	OpcUa_Encoder*                                      a_pEncoder,
	OpcUa_StringTable*                                  a_pNamespaceUris,
	OpcUa_EncodeableTypeTable*                          a_pKnownTypes,
	OpcUa_ByteString*                                   a_pServerCertificate,
	OpcUa_Key*                                          a_pServerPrivateKey,
	OpcUa_Void*                                         a_pPKIConfig,
	OpcUa_UInt32                                        a_nNoSecurityPolicies,
	OpcUa_SecureListener_SecurityPolicyConfiguration*   a_pSecurityPolicyConfigurations,
	OpcUa_SecureListener_PfnSecureChannelCallback*      a_pfSecureChannelCallback,
	OpcUa_Void*                                         a_SecureChannelCallbackData,
	OpcUa_Listener**                                    a_ppListener)
{
	OpcUa_SecureListener*                pSecureListener    = OpcUa_Null;
	OpcUa_SecureListener_ChannelManager* pChannelManager    = OpcUa_Null;
	OpcUa_SecureListener_PolicyManager*  pPolicyManager     = OpcUa_Null;

	OpcUa_UInt32                         nCounter           = 0;

OpcUa_InitializeStatus(OpcUa_Module_SecureListener, "Create");
	
	OpcUa_ReturnErrorIfArgumentNull(a_pInnerListener);
	OpcUa_ReturnErrorIfArgumentNull(a_pDecoder);
	OpcUa_ReturnErrorIfArgumentNull(a_pEncoder);
	OpcUa_ReturnErrorIfArgumentNull(a_pNamespaceUris);
	OpcUa_ReturnErrorIfArgumentNull(a_pKnownTypes);
	OpcUa_ReturnErrorIfArgumentNull(a_pSecurityPolicyConfigurations);
	OpcUa_ReturnErrorIfArgumentNull(a_ppListener);    
	OpcUa_ReturnErrorIfArgumentNull(a_pPKIConfig); 

#if !OPCUA_SECURELISTENER_ALLOW_NOPKI
	OpcUa_ReturnErrorIfArgumentNull(a_pServerCertificate);
	OpcUa_ReturnErrorIfArgumentNull(a_pServerPrivateKey);
	OpcUa_ReturnErrorIfTrue((a_pServerPrivateKey->Key.Data == OpcUa_Null),  OpcUa_BadInvalidArgument);
#endif /* OPCUA_SECURELISTENER_ALLOW_NOPKI */

	/* allocate listener object */
	*a_ppListener = (OpcUa_Listener*)OpcUa_Alloc(sizeof(OpcUa_Listener));
	OpcUa_GotoErrorIfAllocFailed(*a_ppListener);
	OpcUa_MemSet(*a_ppListener, 0, sizeof(OpcUa_Listener));

	/* allocate listener handle */
	pSecureListener = (OpcUa_SecureListener*)OpcUa_Alloc(sizeof(OpcUa_SecureListener));
	OpcUa_GotoErrorIfAllocFailed(pSecureListener);
	OpcUa_MemSet(pSecureListener, 0, sizeof(OpcUa_SecureListener));

	/* create PKI provider for the server */
	pSecureListener->ServerPKIProvider = (OpcUa_PKIProvider*)OpcUa_Alloc(sizeof(OpcUa_PKIProvider));
	OpcUa_GotoErrorIfAllocFailed(pSecureListener->ServerPKIProvider);

	uStatus = OPCUA_P_PKIFACTORY_CREATEPKIPROVIDER( a_pPKIConfig, pSecureListener->ServerPKIProvider);
	OpcUa_GotoErrorIfBad(uStatus);

	/* dissect certificate chain */
	if(     a_pServerCertificate            != OpcUa_Null
		&&  a_pServerCertificate->Length    >  0
		&&  a_pServerCertificate->Data      != OpcUa_Null)
	{   
		pSecureListener->pServerCertificateChain = a_pServerCertificate;

		/* split chain */
		uStatus = pSecureListener->ServerPKIProvider->SplitCertificateChain(
								pSecureListener->pServerCertificateChain,
							   &pSecureListener->uNumberOfChainElements,
							   &pSecureListener->absServerCertificateChain);
		if(OpcUa_IsBad(uStatus))
		{
			OpcUa_GotoErrorIfTrue((OpcUa_IsNotEqual(OpcUa_BadNotSupported)), uStatus);
		}

		/* first in chain must be the server certificate */
		pSecureListener->pServerCertificate = &pSecureListener->absServerCertificateChain[0];
	}
	else
	{
		pSecureListener->pServerCertificate = OpcUa_Null;
	}

#if OPCUA_SECURELISTENER_SUPPORT_THREADPOOL
	/* create ThreadPool */
	if(OpcUa_ProxyStub_g_Configuration.bSecureListener_ThreadPool_Enabled != OpcUa_False)
	{
		uStatus = OpcUa_ThreadPool_Create(  &pSecureListener->hThreadPool, 
											OpcUa_ProxyStub_g_Configuration.iSecureListener_ThreadPool_MinThreads,
											OpcUa_ProxyStub_g_Configuration.iSecureListener_ThreadPool_MaxThreads,
											OpcUa_ProxyStub_g_Configuration.iSecureListener_ThreadPool_MaxJobs,
											OpcUa_ProxyStub_g_Configuration.bSecureListener_ThreadPool_BlockOnAdd,
											OpcUa_ProxyStub_g_Configuration.uSecureListener_ThreadPool_Timeout);
		OpcUa_GotoErrorIfBad(uStatus);
	}
#endif /* OPCUA_SECURELISTENER_SUPPORT_THREADPOOL */

	/* create SecureChannelManager */
	uStatus = OpcUa_SecureListener_ChannelManager_Create(
		OpcUa_SecureListener_ChannelRemovedCallback,
		(OpcUa_Void*)pSecureListener,
		&pChannelManager);
	OpcUa_GotoErrorIfBad(uStatus);

	/* create PolicyManager */
	uStatus = OpcUa_SecureListener_PolicyManager_Create(&pPolicyManager);
	OpcUa_GotoErrorIfBad(uStatus);

	/* Add security policies */
	for(nCounter = 0; nCounter < a_nNoSecurityPolicies; nCounter++)
	{
		/* TODO: error check and proper handling */
		uStatus = OpcUa_SecureListener_PolicyManager_AddSecurityPolicyConfiguration(
			pPolicyManager,
			&(a_pSecurityPolicyConfigurations[nCounter]));
		OpcUa_GotoErrorIfBad(uStatus);
	}

	/* initialize listener handle */
	pSecureListener->Decoder            = a_pDecoder;
	pSecureListener->Encoder            = a_pEncoder;
	pSecureListener->NamespaceUris      = a_pNamespaceUris;
	pSecureListener->KnownTypes         = a_pKnownTypes;

	if(a_pServerPrivateKey != OpcUa_Null)
	{
		pSecureListener->ServerPrivateKey = *a_pServerPrivateKey;
	}

	pSecureListener->SanityCheck                = OpcUa_SecureListener_SanityCheck;
	pSecureListener->TransportListener          = a_pInnerListener;
	pSecureListener->State                      = OpcUa_SecureListenerState_Closed;
	pSecureListener->ChannelManager             = pChannelManager;
	pSecureListener->PolicyManager              = pPolicyManager;
	pSecureListener->SecureChannelCallback      = a_pfSecureChannelCallback;
	pSecureListener->SecureChannelCallbackData  = a_SecureChannelCallbackData;
	pSecureListener->uNextSecureChannelId       = OpcUa_GetTickCount();

	/* make sure that id != 0 */
	if(pSecureListener->uNextSecureChannelId == 0)
	{
		++pSecureListener->uNextSecureChannelId;
	}
		
	/* create mutex */
	uStatus = OpcUa_Mutex_Create(&(pSecureListener->Mutex));
	OpcUa_GotoErrorIfBad(uStatus);

	/* initialize listener object */
	(*a_ppListener)->Handle                         = pSecureListener;
	(*a_ppListener)->Open                           = OpcUa_SecureListener_Open;
	(*a_ppListener)->Close                          = OpcUa_SecureListener_Close;
	(*a_ppListener)->BeginSendResponse              = OpcUa_SecureListener_BeginSendResponse;
	(*a_ppListener)->EndSendResponse                = OpcUa_SecureListener_EndSendResponse;
	(*a_ppListener)->AbortSendResponse              = OpcUa_SecureListener_AbortSendResponse;
	(*a_ppListener)->GetReceiveBufferSize           = OpcUa_Null;
	(*a_ppListener)->GetSecurityPolicyConfiguration = OpcUa_SecureListener_GetSecurityPolicyConfiguration;
	(*a_ppListener)->GetPeerInfo                    = OpcUa_SecureListener_GetPeerInfo;
	(*a_ppListener)->CloseConnection                = OpcUa_SecureListener_CloseConnection;
	(*a_ppListener)->Delete                         = OpcUa_SecureListener_Delete;

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	if(pSecureListener != OpcUa_Null)
	{
		OpcUa_Mutex_Delete(&(pSecureListener->Mutex));

		if(pSecureListener->absServerCertificateChain != OpcUa_Null)
		{
			OpcUa_Free(pSecureListener->absServerCertificateChain);
		}

		OpcUa_Key_Clear(&pSecureListener->ServerPrivateKey);
		if (pSecureListener->ServerPKIProvider != OpcUa_Null)
		{
			OPCUA_P_PKIFACTORY_DELETEPKIPROVIDER(pSecureListener->ServerPKIProvider);
			OpcUa_Free(pSecureListener->ServerPKIProvider);
		}
		OpcUa_Free(pSecureListener);
		pSecureListener = OpcUa_Null;
	}

	if(pChannelManager != OpcUa_Null)
	{
		OpcUa_SecureListener_ChannelManager_Delete(&pChannelManager);
	}

	if(pPolicyManager != OpcUa_Null)
	{
		OpcUa_SecureListener_PolicyManager_Delete(&pPolicyManager);
	}


	OpcUa_Free(*a_ppListener);
	*a_ppListener = OpcUa_Null;

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_SecureListener_ReadRequest
 *===========================================================================*/
OpcUa_StatusCode OpcUa_SecureListener_ReadRequest(  OpcUa_SecureListener*   a_pSecureListener,
													OpcUa_InputStream*      a_pIstrm,
													OpcUa_UInt32            a_expectedTypeId,
													OpcUa_Void**            a_ppRequest)
{
	OpcUa_Decoder*          pDecoder        = OpcUa_Null;
	OpcUa_EncodeableType*   pRequestType    = OpcUa_Null;
	OpcUa_Handle            hDecodeContext  = OpcUa_Null;
	OpcUa_MessageContext    cContext;

OpcUa_InitializeStatus(OpcUa_Module_SecureListener, "ReadRequest");
		
	OpcUa_ReturnErrorIfArgumentNull(a_pSecureListener);
	OpcUa_ReturnErrorIfArgumentNull(a_pIstrm);

	*a_ppRequest    = OpcUa_Null;
	
	pDecoder        = a_pSecureListener->Decoder;

	OpcUa_MessageContext_Initialize(&cContext);
	
	cContext.KnownTypes         = a_pSecureListener->KnownTypes;
	cContext.NamespaceUris      = a_pSecureListener->NamespaceUris;
	cContext.AlwaysCheckLengths = OPCUA_SERIALIZER_CHECKLENGTHS;

	/* open decoder */
	uStatus = pDecoder->Open(pDecoder, a_pIstrm, &cContext, &hDecodeContext);
	OpcUa_GotoErrorIfBad(uStatus);

	/* decode message */
	uStatus = pDecoder->ReadMessage((struct _OpcUa_Decoder*)hDecodeContext, &pRequestType, a_ppRequest);
	OpcUa_GotoErrorIfBad(uStatus);

	if (pRequestType->TypeId != a_expectedTypeId)
	{
		OpcUa_GotoErrorWithStatus(OpcUa_BadUnexpectedError);
	}

	/* close decoder */
	OpcUa_Decoder_Close(pDecoder, &hDecodeContext);
	OpcUa_MessageContext_Clear(&cContext);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	OpcUa_Decoder_Close(pDecoder, &hDecodeContext);
	OpcUa_MessageContext_Clear(&cContext);
	OpcUa_EncodeableObject_Delete(pRequestType, a_ppRequest);

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_SecureListener_SendOpenSecureChannelResponse
 *===========================================================================*/
static OpcUa_StatusCode OpcUa_SecureListener_SendOpenSecureChannelResponse(
	OpcUa_Listener*                     a_pListener,
	OpcUa_SecureChannel**               a_ppSecureChannel,
	OpcUa_CryptoProvider*               a_pCryptoProvider,
	OpcUa_InputStream*                  a_pSecureIstrm,
	OpcUa_String*                       a_pSecurityPolicyUri,
	OpcUa_ChannelSecurityToken*         a_pSecurityToken,
	OpcUa_Key*                          a_pServerNonce,
	OpcUa_OpenSecureChannelRequest*     a_pRequest,
	OpcUa_ByteString*                   a_pClientCertificate)
{
	OpcUa_OutputStream*                 pSecureOstrm                = OpcUa_Null;
	OpcUa_SecureListener*               pSecureListener             = (OpcUa_SecureListener*)a_pListener->Handle;
	OpcUa_SecureChannel*                pSecureChannel              = *a_ppSecureChannel;
	OpcUa_OpenSecureChannelResponse*    pResponse                   = OpcUa_Null;
	OpcUa_ByteString                    clientCertificateThumbprint = OPCUA_BYTESTRING_STATICINITIALIZER;
	OpcUa_MessageSecurityMode           eSecurityMode               = OpcUa_MessageSecurityMode_None;

OpcUa_InitializeStatus(OpcUa_Module_SecureListener, "SendOpenSecureChannelResponse");

	if(     a_pClientCertificate            != OpcUa_Null
		&&  a_pClientCertificate->Length    > 0
		&&  a_pClientCertificate->Data      != OpcUa_Null)
	{
		/*** get the client's certificate thumbprint for the OpenSecureChannelResponse header ***/
		uStatus = a_pCryptoProvider->GetCertificateThumbprint(  a_pCryptoProvider,
																a_pClientCertificate,
																&clientCertificateThumbprint);
		OpcUa_GotoErrorIfBad(uStatus);

		if(clientCertificateThumbprint.Length > 0)
		{
			clientCertificateThumbprint.Data = (OpcUa_Byte*)OpcUa_Alloc(clientCertificateThumbprint.Length*sizeof(OpcUa_Byte));
			OpcUa_GotoErrorIfAllocFailed(clientCertificateThumbprint.Data);

			uStatus = a_pCryptoProvider->GetCertificateThumbprint(  a_pCryptoProvider,
																	a_pClientCertificate,
																	&clientCertificateThumbprint);
			OpcUa_GotoErrorIfBad(uStatus);
		}
		else
		{
			uStatus = OpcUa_Bad;
			OpcUa_GotoErrorIfBad(uStatus);
		}
	}
   
	/* must always sign and encrypt open secure channel requests. */
	eSecurityMode = pSecureChannel->MessageSecurityMode;

	if (eSecurityMode != OpcUa_MessageSecurityMode_None)
	{
		eSecurityMode = OpcUa_MessageSecurityMode_SignAndEncrypt;
	}

	/*** fill in OpenSecureChannelResponse ***/
	uStatus = OpcUa_EncodeableObject_Create(    &OpcUa_OpenSecureChannelResponse_EncodeableType,
												(OpcUa_Void**)&pResponse);
	OpcUa_GotoErrorIfBad(uStatus);

	/*** fill response header ***/
	OpcUa_OpenSecureChannelResponse_Initialize(pResponse);

	/* the same session id is used like in the request */
	pResponse->ResponseHeader.RequestHandle             = a_pRequest->RequestHeader.RequestHandle;
	pResponse->ResponseHeader.Timestamp                 = OPCUA_P_DATETIME_UTCNOW();

	OpcUa_ExtensionObject_Initialize(&pResponse->ResponseHeader.AdditionalHeader);
	pResponse->ResponseHeader.ServiceResult             = OpcUa_Good;

	/*** fill response body ***/

	/* copy byte string content and clear pointers in source */
	pResponse->ServerNonce = a_pServerNonce->Key;
	OpcUa_ByteString_Initialize(&(a_pServerNonce->Key));

	OpcUa_MemCpy(   &pResponse->SecurityToken,
					sizeof(OpcUa_ChannelSecurityToken),
					a_pSecurityToken,
					sizeof(OpcUa_ChannelSecurityToken));

	/*** start response ***/
	uStatus = OpcUa_SecureListener_BeginSendOpenSecureChannelResponse(  a_pListener,
																		pSecureChannel,
																		a_pSecureIstrm,
																		a_pSecurityPolicyUri,
																		eSecurityMode,
																		a_pCryptoProvider,
																		pSecureListener->pServerCertificate,
																		pSecureListener->pServerCertificateChain,
																		&clientCertificateThumbprint,
																		&pSecureOstrm);
	OpcUa_GotoErrorIfBad(uStatus);

	/* pSecureChannel is now owned by pSecureOstrm */
	*a_ppSecureChannel = OpcUa_Null;

	/* check if a stream is active - this function is called while the securechannel is locked! */
	if(pSecureChannel->uNumberOfOutputStreams == 0)
	{
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_SendOpenSecureChannelResponse: Sending response for channel %u!\n", pSecureChannel->SecureChannelId);

		/*** encode response body ***/
		uStatus = OpcUa_SecureListener_WriteResponse(   pSecureListener,
														&pSecureOstrm,
														(OpcUa_Void*)pResponse,
														&OpcUa_OpenSecureChannelResponse_EncodeableType);
		OpcUa_GotoErrorIfBad(uStatus);

		/*** end send response ***/
		uStatus = OpcUa_SecureListener_EndSendOpenSecureChannelResponse(    a_pListener,
																			&pSecureOstrm,
																			OpcUa_Good);
		OpcUa_GotoErrorIfBad(uStatus);

		/* immediately activate new token */
		pSecureChannel->bCurrentTokenActive = OpcUa_True;

		OpcUa_OpenSecureChannelResponse_Clear(pResponse);
		OpcUa_Free(pResponse);
	}
	else
	{
		/* delaying sending of this response until theres a gap between two application responses */
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_SendOpenSecureChannelResponse: Delaying response for channel %u! %u streams active.\n", pSecureChannel->SecureChannelId, pSecureChannel->uNumberOfOutputStreams);

		pSecureChannel->pOSCOutputStream  = (OpcUa_Void*)pSecureOstrm;
		pSecureChannel->pOSCMessage       = (OpcUa_Void*)pResponse;

		uStatus = OpcUa_GoodCompletesAsynchronously;
	}

	OpcUa_ByteString_Clear(&clientCertificateThumbprint);
	
OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	if(pSecureOstrm != OpcUa_Null)
	{
		pSecureOstrm->Delete((OpcUa_Stream**)&pSecureOstrm);
	}

	if(pResponse != OpcUa_Null)
	{
		OpcUa_OpenSecureChannelResponse_Clear(pResponse);
		OpcUa_Free(pResponse);
	}

	OpcUa_ByteString_Clear(&clientCertificateThumbprint);

OpcUa_FinishErrorHandling;
}

/*============================================================================
* OpcUa_SecureListener_ValidateCertificate
*===========================================================================*/
/** @brief Internal helper for the certificate validation process. */
static OpcUa_StatusCode OpcUa_SecureListener_ValidateCertificate(   OpcUa_SecureListener* a_pSecureListener,
																	OpcUa_ByteString*     a_pCertificate)
{
	OpcUa_Int           iValidationCode     = 0;
	OpcUa_PKIProvider*  pPkiProvider        = (OpcUa_PKIProvider*)a_pSecureListener->ServerPKIProvider;
	OpcUa_Void*         pCertificateStore   = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_SecureListener, "ValidateCertificate");

	/* open certificate store */
	uStatus = pPkiProvider->OpenCertificateStore(   pPkiProvider,
													&pCertificateStore);

	if(OpcUa_IsBad(uStatus))
	{
		if(OpcUa_IsNotEqual(OpcUa_BadNotSupported))
		{
			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_WARNING, "OpcUa_SecureListener_ValidateCertificate: Could not open certificate store!\n");

			if(OpcUa_IsEqual(OpcUa_Bad))
			{
				/* map unspecified error */
				OpcUa_GotoErrorWithStatus(OpcUa_BadSecurityConfig);
			}
			else
			{
				OpcUa_GotoError;
			}
		}
	}

	/* validate certificate */
	/* Depending on the PKiProvider type will call the ValidateCertificate callback function */
	uStatus = pPkiProvider->ValidateCertificate(pPkiProvider,
												a_pCertificate,
												pCertificateStore,
												&iValidationCode);

	/* mask valid errorcode */
	if(OpcUa_IsBad(uStatus))
	{
		if(uStatus == OpcUa_BadNotSupported)
		{
			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_WARNING, "OpcUa_SecureListener_ValidateCertificate: not supported\n");
			uStatus = OpcUa_Good;
		}
		else
		{
			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_WARNING, "OpcUa_SecureListener_ValidateCertificate: Validation failed with 0x%08X\n", uStatus);
		}
	}
	else
	{
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_ValidateCertificate: success\n");
	}

	/* close certificate store */
	pPkiProvider->CloseCertificateStore(pPkiProvider,
										&pCertificateStore);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
* OpcUa_SecureListener_ProcessOpenSecureChannelRequest
*===========================================================================*/
OpcUa_StatusCode OpcUa_SecureListener_ProcessOpenSecureChannelRequest(  
	OpcUa_Listener*     a_pSecureListenerInterface,
	OpcUa_Handle        a_hTransportConnection,
	OpcUa_InputStream** a_ppTransportIstrm,
	OpcUa_Boolean       a_bRequestComplete)
{
	OpcUa_SecureListener*                               pSecureListener                 = OpcUa_Null;
	OpcUa_InputStream*                                  pSecureIStrm                    = OpcUa_Null;
	OpcUa_SecureStream*                                 pSecureStream                   = OpcUa_Null;
	OpcUa_SecureChannel*                                pSecureChannel                  = OpcUa_Null;
	OpcUa_CryptoProvider*                               pCryptoProvider                 = OpcUa_Null;
	OpcUa_UInt32                                        uSecureChannelId                = OPCUA_SECURECHANNEL_ID_INVALID;

	OpcUa_String                                        sSecurityPolicyUri;
	OpcUa_StringA                                       szPolicyUri                     = OpcUa_Null;

	OpcUa_ByteString                                    SenderCertificateChain          = OPCUA_BYTESTRING_STATICINITIALIZER;
	OpcUa_ByteString                                    ReceiverCertificateThumbprint   = OPCUA_BYTESTRING_STATICINITIALIZER;
	OpcUa_UInt32                                        uNumberOfSenderChainElements    = 0;
	OpcUa_ByteString*                                   pSenderCertificateChain         = OpcUa_Null;
	OpcUa_ByteString*                                   pSenderCertificate              = OpcUa_Null;

	OpcUa_Key                                           serverNonce;
	OpcUa_ChannelSecurityToken*                         pSecurityToken                  = OpcUa_Null;

	OpcUa_Boolean                                       bRenewChannel                   = OpcUa_False;
	OpcUa_OpenSecureChannelRequest*                     pRequest                        = OpcUa_Null;

	OpcUa_SecurityKeyset*                               pReceivingKeyset                = OpcUa_Null;
	OpcUa_SecurityKeyset*                               pSendingKeyset                  = OpcUa_Null;
	OpcUa_MessageSecurityMode                           eRequestedSecurityMode          = OpcUa_MessageSecurityMode_Invalid;
	OpcUa_Key*                                          pSigningKey                     = OpcUa_Null;
	OpcUa_Key*                                          pEncryptionKey                  = OpcUa_Null;

	OpcUa_SecureListener_SecureChannelEvent             eSecureChannelEvent             = eOpcUa_SecureListener_SecureChannelUnkown;
	OpcUa_SecureListener_SecurityPolicyConfiguration    secConfig;

	OpcUa_SecureListener_PfnSecureChannelCallback*      pfSecureChannelCallback         = OpcUa_Null;
	OpcUa_Void*                                         pvSecureChannelCallbackData     = OpcUa_Null;
	OpcUa_UInt32                                        uiSecureChannelId               = 0;

OpcUa_InitializeStatus(OpcUa_Module_SecureListener, "ProcessOpenSecureChannelRequest");

	OpcUa_ReturnErrorIfArgumentNull(a_pSecureListenerInterface);
	OpcUa_ReturnErrorIfArgumentNull(a_hTransportConnection);
	OpcUa_ReturnErrorIfArgumentNull(a_ppTransportIstrm);

	OpcUa_Key_Initialize(&serverNonce);

	/*** get listener handle ***/
	pSecureListener = (OpcUa_SecureListener*)a_pSecureListenerInterface->Handle;
	OpcUa_ReturnErrorIfNull(pSecureListener, OpcUa_BadInvalidState);

	/* parse stream header */
	uStatus = OpcUa_SecureStream_DecodeAsymmetricSecurityHeader(*a_ppTransportIstrm,
																&uSecureChannelId,
																&sSecurityPolicyUri,
																&SenderCertificateChain,
																&ReceiverCertificateThumbprint);
	OpcUa_GotoErrorIfBad(uStatus);

	OpcUa_Trace(    OPCUA_TRACE_STACK_LEVEL_ERROR,
					"ProcessOpenSecureChannelRequest: SID %u, SURI \"%*.*s\"\n",
					uSecureChannelId,
					OpcUa_String_StrLen(&sSecurityPolicyUri),
					OpcUa_String_StrLen(&sSecurityPolicyUri),
					OpcUa_String_GetRawString(&sSecurityPolicyUri));

	OpcUa_MemSet(&secConfig, 0, sizeof(OpcUa_SecureListener_SecurityPolicyConfiguration));

	/* if security is turned on then the sender and receiver certificates must be specified. */
	if((SenderCertificateChain.Data != OpcUa_Null) && (ReceiverCertificateThumbprint.Data != OpcUa_Null))
	{
		uStatus = pSecureListener->ServerPKIProvider->SplitCertificateChain(
			&SenderCertificateChain,
			&uNumberOfSenderChainElements,
			&pSenderCertificateChain);
		if(OpcUa_IsBad(uStatus))
		{
			OpcUa_GotoErrorIfTrue((OpcUa_IsNotEqual(OpcUa_BadNotSupported)), uStatus);
		}

		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR,
					"ProcessOpenSecureChannelRequest: Client certificate chain has %u elements.\n",
					uNumberOfSenderChainElements);

		pSenderCertificate = &pSenderCertificateChain[0];

		secConfig.uMessageSecurityModes = OPCUA_SECURECHANNEL_MESSAGESECURITYMODE_SIGNANDENCRYPT;
		eRequestedSecurityMode = OpcUa_MessageSecurityMode_SignAndEncrypt;
		OpcUa_String* pszSecurityPolicyNone = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
		OpcUa_String_Initialize(pszSecurityPolicyNone);
		OpcUa_String_AttachCopy(pszSecurityPolicyNone, OpcUa_SecurityPolicy_None);
		if (!OpcUa_String_StrnCmp(pszSecurityPolicyNone, &sSecurityPolicyUri, OPCUA_STRING_LENDONTCARE, OpcUa_True))
		{
			/* OpcUa_SecurityPolicy_None is not allowed together with OpcUa_MessageSecurityMode_SignAndEncrypt */
			OpcUa_GotoErrorWithStatus(OpcUa_Bad);
		}
		OpcUa_String_Clear(pszSecurityPolicyNone);
		OpcUa_Free(pszSecurityPolicyNone);
		pszSecurityPolicyNone = OpcUa_Null;
	}
	/* if security is turned off then the sender and receiver certificates must be null. */
	else if((SenderCertificateChain.Data == OpcUa_Null) && (ReceiverCertificateThumbprint.Data == OpcUa_Null))
	{
		secConfig.uMessageSecurityModes = OPCUA_SECURECHANNEL_MESSAGESECURITYMODE_NONE;
		eRequestedSecurityMode = OpcUa_MessageSecurityMode_None;
	}
	else
	{
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "ProcessOpenSecureChannelRequest: Invalid sender and/or receiver certificates specified!\n");
		OpcUa_GotoErrorWithStatus(OpcUa_Bad);
	}

	/* not very nice */
	OpcUa_MemCpy(&secConfig.sSecurityPolicy, sizeof(OpcUa_String), &sSecurityPolicyUri, sizeof(OpcUa_String));

	if(uSecureChannelId == OPCUA_SECURECHANNEL_ID_INVALID)
	{
		uStatus = OpcUa_SecureListener_ChannelManager_GetChannelByTransportConnection(  pSecureListener->ChannelManager,
																						a_hTransportConnection,
																						&pSecureChannel);
		bRenewChannel = OpcUa_False;
	}
	else
	{
		uStatus = OpcUa_SecureListener_ChannelManager_GetChannelBySecureChannelID(  pSecureListener->ChannelManager,
																					uSecureChannelId,
																					&pSecureChannel);

		if(pSecureChannel == OpcUa_Null)
		{
			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_WARNING, "OpcUa_SecureListener_ProcessOpenSecureChannelRequest: No secure channel found for the given id!\n");
			OpcUa_GotoErrorWithStatus(OpcUa_BadSecureChannelIdInvalid);
		}
		else if(    pSecureChannel->TransportConnection != OpcUa_Null
				&&  pSecureChannel->TransportConnection != a_hTransportConnection)
		{
			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_WARNING, "OpcUa_SecureListener_ProcessOpenSecureChannelRequest: SecureChannel id used by wrong transport connection!\n");
			OpcUa_GotoErrorWithStatus(OpcUa_BadSecureChannelIdInvalid);
		}
		
		bRenewChannel = OpcUa_True;
	}

	OpcUa_GotoErrorIfBad(uStatus);

	/* look if there is a pending stream */
	uStatus = OpcUa_SecureChannel_GetPendingInputStream(pSecureChannel,
														&pSecureIStrm);
	OpcUa_GotoErrorIfBad(uStatus);

	if(pSecureIStrm == OpcUa_Null)
	{
		/** check for a valid security policy ***/
		uStatus = OpcUa_SecureListener_PolicyManager_IsValidSecurityPolicy(
			pSecureListener->PolicyManager,
			&secConfig.sSecurityPolicy);

		if(OpcUa_IsBad(uStatus))
		{
			/* check if a discovery only channel can be openned */
			OpcUa_Boolean discoveryOnly = OpcUa_False;

			if (OpcUa_String_IsNull(&pSecureChannel->SecurityPolicyUri))
			{
				OpcUa_StatusCode    uStatusTemp = OpcUa_Good;
				OpcUa_String        sPolicyNone = OPCUA_STRING_STATICINITIALIZER;

				uStatusTemp = OpcUa_String_AttachReadOnly(&sPolicyNone, OpcUa_SecurityPolicy_None);
				OpcUa_GotoErrorIfTrue(OpcUa_IsBad(uStatusTemp), uStatusTemp);

				if((OpcUa_String_StrnCmp(&sPolicyNone, &sSecurityPolicyUri, OpcUa_String_StrLen(&sPolicyNone), OpcUa_False) == 0))
				{
					discoveryOnly = OpcUa_True;
				}
			}

			if(!discoveryOnly)
			{
				OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "ProcessOpenSecureChannelRequest: Invalid Policy Configuration requested!\n");
				OpcUa_GotoError;
			}

			pSecureChannel->DiscoveryOnly = OpcUa_True;

			uStatus = OpcUa_String_StrnCpy( &pSecureChannel->SecurityPolicyUri,
											&sSecurityPolicyUri,
											OPCUA_STRING_LENDONTCARE);
			OpcUa_GotoErrorIfBad(uStatus);
			
			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "ProcessOpenSecureChannelRequest: Discovery Only Channel Created\n");
		}

		if(bRenewChannel != OpcUa_False)
		{
			/* check if the security policy is being changed. */
			if (OpcUa_String_IsNull(&pSecureChannel->SecurityPolicyUri) && !OpcUa_String_IsNull(&sSecurityPolicyUri))
			{
				OpcUa_GotoErrorWithStatus(OpcUa_BadSecurityPolicyRejected);
			}
			else
			{
				if ((OpcUa_String_StrnCmp(&pSecureChannel->SecurityPolicyUri, &sSecurityPolicyUri, OpcUa_String_StrLen(&sSecurityPolicyUri), OpcUa_False) != 0))
				{
					OpcUa_GotoErrorWithStatus(OpcUa_BadSecurityPolicyRejected);
				}
			}

			/* must check that the client certificate is not changed. */
			if (pSecureChannel->ClientCertificate.Length > 0 &&
				pSecureChannel->ClientCertificate.Data != OpcUa_Null)
			{
				if(pSenderCertificate != OpcUa_Null)
				{
					if (pSenderCertificate->Length != pSecureChannel->ClientCertificate.Length ||
						pSenderCertificate->Data == OpcUa_Null)
					{
						OpcUa_GotoErrorWithStatus(OpcUa_BadCertificateInvalid);
					}

					if (OpcUa_MemCmp(pSenderCertificate->Data, pSecureChannel->ClientCertificate.Data, pSenderCertificate->Length) != 0)
					{
						OpcUa_GotoErrorWithStatus(OpcUa_BadCertificateInvalid);
					}
				}
			}
			else
			{
				if(     pSenderCertificate          != OpcUa_Null
					&&  pSenderCertificate->Length  >  0)
				{
					OpcUa_GotoErrorWithStatus(OpcUa_BadCertificateInvalid);
				}
			}
		}
		else /* assign the security policy to the channel */
		{
			uStatus = OpcUa_String_StrnCpy( &pSecureChannel->SecurityPolicyUri,
											&sSecurityPolicyUri,
											OPCUA_STRING_LENDONTCARE);
			OpcUa_GotoErrorIfBad(uStatus);
		}

		/*** create CryptoProvider ***/
		pCryptoProvider = (OpcUa_CryptoProvider*)OpcUa_Alloc(sizeof(OpcUa_CryptoProvider));
		OpcUa_GotoErrorIfAllocFailed(pCryptoProvider);

		szPolicyUri = OpcUa_String_GetRawString(&sSecurityPolicyUri);

		uStatus = OPCUA_P_CRYPTOFACTORY_CREATECRYPTOPROVIDER(szPolicyUri, pCryptoProvider);
		OpcUa_GotoErrorIfBad(uStatus);

		if( (eRequestedSecurityMode == OpcUa_MessageSecurityMode_SignAndEncrypt) ||
			(eRequestedSecurityMode == OpcUa_MessageSecurityMode_Sign))
		{
			/*** validate certificate ***/
			uStatus = OpcUa_SecureListener_ValidateCertificate( pSecureListener,
															   &SenderCertificateChain);

			if (OpcUa_IsGood(uStatus))
			{
				OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "ProcessOpenSecureChannelRequest: Client Certificate validated! (0x%08X)\n", uStatus);
			}
			else
			{
				OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "ProcessOpenSecureChannelRequest: Client Certificate not valid! (0x%08X)\n", uStatus);
			}

			/* inform upper layer */
			if (pSecureListener->SecureChannelCallback != OpcUa_Null)
			{
				OpcUa_String        sTempUri    = OPCUA_STRING_STATICINITIALIZER;
				OpcUa_StatusCode    uTempStatus = OpcUa_Good;

				OpcUa_String_AttachReadOnly(&sTempUri, szPolicyUri);

				uTempStatus = pSecureListener->SecureChannelCallback(   uSecureChannelId,
																		(bRenewChannel)?eOpcUa_SecureListener_SecureChannelRenewVerifyCertificate:eOpcUa_SecureListener_SecureChannelOpenVerifyCertificate,
																		uStatus,
																	   &SenderCertificateChain,
																	   &sTempUri,
																		secConfig.uMessageSecurityModes,
																		0,
																		pSecureListener->SecureChannelCallbackData);

				/* bad temp status means upper layer wants to override validation result */
				if(OpcUa_IsBad(uTempStatus))
				{
					if(OpcUa_IsEqualTo(uTempStatus, OpcUa_BadContinue))
					{
						/* application accepts certificate despite internal validation result */
						OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "ProcessOpenSecureChannelRequest: Client Certificate result ignored by callback!\n");
						uStatus = OpcUa_Good;
					}
					else
					{
						/* application rejects certificate */
						OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "ProcessOpenSecureChannelRequest: Client Certificate could not be validated by callback! (0x%08X)\n", uTempStatus);
						uStatus = uTempStatus;
					}
				} /* else use current uStatus */
			}

			if(OpcUa_IsBad(uStatus))
			{
				if(pSecureListener->SecureChannelCallback != OpcUa_Null)
				{
					OpcUa_String sTempUri = OPCUA_STRING_STATICINITIALIZER;

					OpcUa_String_AttachReadOnly(&sTempUri, szPolicyUri);
					pSecureListener->SecureChannelCallback(
						uSecureChannelId,
						(bRenewChannel)?eOpcUa_SecureListener_SecureChannelRenew:eOpcUa_SecureListener_SecureChannelOpen,
						uStatus,
					   &SenderCertificateChain,
					   &sTempUri,
						secConfig.uMessageSecurityModes,
						0,
						pSecureListener->SecureChannelCallbackData);
				}

				OpcUa_GotoError;
			}

			/** create server nonce **/
			OpcUa_ByteString_Initialize(&serverNonce.Key);

			/* get the length for memory allocation */
			uStatus = pCryptoProvider->GenerateKey(pCryptoProvider, -1, &serverNonce);
			OpcUa_GotoErrorIfBad(uStatus);

			serverNonce.Key.Data = (OpcUa_Byte*)OpcUa_Alloc(serverNonce.Key.Length);
			OpcUa_GotoErrorIfAllocFailed(serverNonce.Key.Data);

			/* generate ServerNonce */
			uStatus = pCryptoProvider->GenerateKey( pCryptoProvider, serverNonce.Key.Length, &serverNonce);
			OpcUa_GotoErrorIfBad(uStatus);
		}
		else /* policy none */
		{
			/* create fake server nonce */
			
			/* generate fake server nonce that is sent to the client */
			serverNonce.Type            = OpcUa_Crypto_KeyType_Random;
			serverNonce.Key.Length      = 1;
			serverNonce.fpClearHandle   = OpcUa_Null;
			serverNonce.Key.Data        = (OpcUa_Byte*)OpcUa_Alloc(sizeof(OpcUa_Byte));
			OpcUa_GotoErrorIfAllocFailed(serverNonce.Key.Data);
			*(serverNonce.Key.Data)     = 0x01;
		}

		/* create inputstream and check if processing can be started or delayed until last chunk is received. */
		uStatus = OpcUa_SecureStream_CreateOpenSecureChannelInput(  pCryptoProvider,
																	eRequestedSecurityMode,
																	pSecureListener->pServerCertificate,
																	&pSecureListener->ServerPrivateKey,
																	pSenderCertificate,
																	&ReceiverCertificateThumbprint,
																	1,
																	&pSecureIStrm);
		OpcUa_GotoErrorIfBad(uStatus);

		pSecureStream  = (OpcUa_SecureStream*)pSecureIStrm->Handle;
		pSigningKey    = pSecureStream->pSenderPublicKey;
		pEncryptionKey = &pSecureListener->ServerPrivateKey;

		uStatus = OpcUa_SecureStream_AppendInput(   a_ppTransportIstrm,
													pSecureIStrm,
													pSigningKey,    /* (pSecureStream->eMessageType == eOpcUa_SecureStream_Types_OpenSecureChannel) ? pSecureStream->pSenderPublicKey : &pSecureStream->pKeyset->SigningKey */
													pEncryptionKey, /* (pSecureStream->eMessageType == eOpcUa_SecureStream_Types_OpenSecureChannel) ? pSecureStream->pPrivateKey : &pSecureStream->pKeyset->EncryptionKey */
													OpcUa_Null,
													pCryptoProvider);
		OpcUa_GotoErrorIfBad(uStatus);

		pSecureStream                   = (OpcUa_SecureStream*)pSecureIStrm->Handle;
		pSecureStream->SecureChannelId  = uSecureChannelId;
		pSecureStream->eMessageType     = eOpcUa_SecureStream_Types_StandardMessage;
	}
	else /* we had a pending input stream */
	{
		/* Reset the pending request stream. */
		OpcUa_SecureChannel_SetPendingInputStream(  pSecureChannel,
													OpcUa_Null);

		OpcUa_GotoErrorWithStatus(OpcUa_BadNotSupported);
	}

	/* look if we can start processing the stream */
	if(a_bRequestComplete == OpcUa_False)
	{
		/* Reset the pending request stream. */
		OpcUa_SecureChannel_SetPendingInputStream(  pSecureChannel,
													OpcUa_Null);

		OpcUa_GotoErrorWithStatus(OpcUa_BadNotSupported);
	}
	else /* preprocess the stream */
	{
		/* Reset the pending request stream. */
		OpcUa_SecureChannel_SetPendingInputStream(  pSecureChannel,
													OpcUa_Null);

		/* reset buffer index to start reading from the first buffer */
		pSecureStream->nCurrentReadBuffer = 0;

		/*** read request ***/
		uStatus = OpcUa_SecureListener_ReadRequest( pSecureListener,
													pSecureIStrm,
													OpcUaId_OpenSecureChannelRequest,
													(OpcUa_Void**)&pRequest);
		OpcUa_GotoErrorIfBad(uStatus);

		/* check that the message has been properly secured */
		if (pRequest->SecurityMode != OpcUa_MessageSecurityMode_None)
		{
			if (secConfig.uMessageSecurityModes != OPCUA_SECURECHANNEL_MESSAGESECURITYMODE_SIGNANDENCRYPT)
			{
				OpcUa_GotoErrorWithStatus(OpcUa_BadSecurityChecksFailed);
			}
		}

		/* verify request type and security mode. */
		if (bRenewChannel)
		{       
			if (pRequest->RequestType != OpcUa_SecurityTokenRequestType_Renew)
			{
				OpcUa_GotoErrorWithStatus(OpcUa_BadRequestTypeInvalid);
			}

			if (pRequest->SecurityMode != pSecureChannel->MessageSecurityMode)
			{
				OpcUa_GotoErrorWithStatus(OpcUa_BadSecurityPolicyRejected);
			}
		}
		else
		{
			if (pRequest->RequestType != OpcUa_SecurityTokenRequestType_Issue)
			{
				OpcUa_GotoErrorWithStatus(OpcUa_BadRequestTypeInvalid);
			}

			/* update and validate the security mode. */
			switch (pRequest->SecurityMode)
			{
				default:
				case OpcUa_MessageSecurityMode_None:
				{
					secConfig.uMessageSecurityModes = OPCUA_SECURECHANNEL_MESSAGESECURITYMODE_NONE;
					break;
				}

				case OpcUa_MessageSecurityMode_Sign:
				{
					secConfig.uMessageSecurityModes = OPCUA_SECURECHANNEL_MESSAGESECURITYMODE_SIGN;
					break;
				}

				case OpcUa_MessageSecurityMode_SignAndEncrypt:
				{
					secConfig.uMessageSecurityModes = OPCUA_SECURECHANNEL_MESSAGESECURITYMODE_SIGNANDENCRYPT;
					break;
				}
			}

			uStatus = OpcUa_SecureListener_PolicyManager_IsValidSecurityPolicyConfiguration(
				pSecureListener->PolicyManager,
				&secConfig);

			if(pSecureChannel->DiscoveryOnly == OpcUa_False)
			{
				OpcUa_GotoErrorIfBad(uStatus);
			}
				
			pSecureChannel->MessageSecurityMode = pRequest->SecurityMode;
		}

		/*** derive channel keys ***/
		uStatus = OpcUa_SecureChannel_DeriveKeys(   pSecureChannel->MessageSecurityMode,
													pCryptoProvider,
													&pRequest->ClientNonce,
													&serverNonce.Key,
													&pReceivingKeyset, /* Client key is used for receiving on this side. */
													&pSendingKeyset);  /* Server key is used for sending on this side.*/
		OpcUa_GotoErrorIfBad(uStatus);

		/* check whether new or existing securechannel */
		if((uSecureChannelId == OPCUA_SECURECHANNEL_ID_INVALID) && (pRequest->RequestType == OpcUa_SecurityTokenRequestType_Issue))
		{
			OpcUa_SecureChannel* pTempSecureChannel = OpcUa_Null;

			/*** new securechannel ***/
			pSecureChannel->SecureChannelId = (pSecureListener->uNextSecureChannelId)++;

			/* skip 0 */
			if(pSecureListener->uNextSecureChannelId == 0)
			{
				(pSecureListener->uNextSecureChannelId)++;
			}

			/* test if channel id is already in use */
			OpcUa_SecureListener_ChannelManager_GetChannelBySecureChannelID( pSecureListener->ChannelManager,
																			 pSecureListener->uNextSecureChannelId,
																			&pTempSecureChannel);
			while(pTempSecureChannel != OpcUa_Null)
			{
				(pSecureListener->uNextSecureChannelId)++;

				OpcUa_SecureListener_ChannelManager_GetChannelBySecureChannelID( pSecureListener->ChannelManager,
																				 pSecureListener->uNextSecureChannelId,
																				&pTempSecureChannel);
			};

			/* AddChannel */

			/* generate SecurityToken */
			uStatus = pSecureChannel->GenerateSecurityToken(pSecureChannel,
															pRequest->RequestedLifetime,
															&pSecurityToken);
			OpcUa_GotoErrorIfBad(uStatus);

			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "ProcessOpenSecureChannelRequest: Open: Revised Lifetime of Channel %u from %u to %u ms!\n", pSecureChannel->SecureChannelId, pRequest->RequestedLifetime, pSecurityToken->RevisedLifetime);

			/* open SecureChannel */
			uStatus = pSecureChannel->Open( pSecureChannel,
											a_hTransportConnection,
											*pSecurityToken,
											pRequest->SecurityMode,
											pSenderCertificate,
											pSecureListener->pServerCertificate,
											pReceivingKeyset, /* Client key set is used for receiving on this side. */
											pSendingKeyset,   /* Server key set is used for receiving on this side. */
											pCryptoProvider);
			OpcUa_GotoErrorIfBad(uStatus);

			eSecureChannelEvent = eOpcUa_SecureListener_SecureChannelOpen;
		}
		else if(pRequest->RequestType == OpcUa_SecurityTokenRequestType_Renew)
		{
			/*** renew SecureChannel ***/

			if(pSecureChannel->SecureChannelId != uSecureChannelId || uSecureChannelId == 0)
			{
				OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "ProcessOpenSecureChannelRequest: The message header securechannel id (%u) does not match the id of the channel assigned to the transport handle (%u)!\n", uSecureChannelId, pSecureChannel->SecureChannelId);
			}

			uStatus = pSecureChannel->RenewSecurityToken(   pSecureChannel,
															&pSecureChannel->CurrentChannelSecurityToken,
															pRequest->RequestedLifetime,
															&pSecurityToken);
			OpcUa_GotoErrorIfBad(uStatus);

			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "ProcessOpenSecureChannelRequest: Renew: Revised Lifetime of channel %u from %u to %u ms!\n", pSecureChannel->SecureChannelId, pRequest->RequestedLifetime, pSecurityToken->RevisedLifetime);

			if(pSecureChannel->TransportConnection != a_hTransportConnection)
			{
				OpcUa_SecureChannel* pTempSecureChannel = OpcUa_Null;
				OpcUa_SecureListener_ChannelManager_GetChannelByTransportConnection(  pSecureListener->ChannelManager,
																					  a_hTransportConnection,
																					  &pTempSecureChannel);
				if(pTempSecureChannel != OpcUa_Null)
				{
					pTempSecureChannel->TransportConnection = OpcUa_Null;
					OpcUa_SecureListener_ChannelManager_ReleaseChannel(
							pSecureListener->ChannelManager,
							&pTempSecureChannel);
				}
			}

			uStatus = pSecureChannel->Renew(pSecureChannel,
											a_hTransportConnection,
											*pSecurityToken,
											pRequest->SecurityMode,
											pSenderCertificate,
											pSecureListener->pServerCertificate,
											pReceivingKeyset, /* Client key set is used for receiving on this side. */
											pSendingKeyset,   /* Server key set is used for receiving on this side. */
											pCryptoProvider);
			OpcUa_GotoErrorIfBad(uStatus);

			eSecureChannelEvent = eOpcUa_SecureListener_SecureChannelRenew;
		}
		else
		{
			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "ProcessOpenSecureChannelRequest: Invalid Operation!\n");
			OpcUa_GotoErrorWithStatus(OpcUa_BadRequestTypeInvalid);
		}

		/* store information for later callback */
		pfSecureChannelCallback     = pSecureListener->SecureChannelCallback;
		pvSecureChannelCallbackData = pSecureListener->SecureChannelCallbackData;
		uiSecureChannelId           = pSecureChannel->SecureChannelId;

		/* send response */
		uStatus = OpcUa_SecureListener_SendOpenSecureChannelResponse(   a_pSecureListenerInterface,
																	   &pSecureChannel,
																		pCryptoProvider,
																		pSecureIStrm,
																		&sSecurityPolicyUri,
																		pSecurityToken,
																		&serverNonce,
																		pRequest,
																		pSenderCertificate);

		OpcUa_ByteString_Clear(&ReceiverCertificateThumbprint);
		OpcUa_Key_Clear(&serverNonce);
	} /* if(a_bRequestComplete == OpcUa_False) */

	/* inform upper layer about event */
	if(pfSecureChannelCallback != OpcUa_Null)
	{
		OpcUa_String sTempUri = OPCUA_STRING_STATICINITIALIZER;

		OpcUa_String_AttachReadOnly(&sTempUri, szPolicyUri);

		pfSecureChannelCallback( uiSecureChannelId,
								 eSecureChannelEvent,
								 uStatus,
								 &SenderCertificateChain,
								 &sTempUri,
								 secConfig.uMessageSecurityModes,
								 pRequest->RequestedLifetime,
								 pvSecureChannelCallbackData);
	}

	/*** clean up ***/
	if(pSecureChannel != OpcUa_Null)
	{
		OpcUa_SecureListener_ChannelManager_ReleaseChannel(  pSecureListener->ChannelManager,
															&pSecureChannel);
	}

	if(pSenderCertificateChain != OpcUa_Null)
	{
		OpcUa_Free(pSenderCertificateChain);
	}

	OpcUa_ByteString_Clear(&SenderCertificateChain);

	if(pRequest != OpcUa_Null)
	{
		OpcUa_OpenSecureChannelRequest_Clear(pRequest);
		OpcUa_Free(pRequest);
		pRequest = OpcUa_Null;
	}

	if(pSecurityToken != OpcUa_Null)
	{
		OpcUa_ChannelSecurityToken_Clear(pSecurityToken);
		OpcUa_Free(pSecurityToken);
		pSecurityToken = OpcUa_Null;
	}

	if(pSecureIStrm != OpcUa_Null)
	{
		OpcUa_Stream_Close((OpcUa_Stream*)pSecureIStrm);
		OpcUa_Stream_Delete((OpcUa_Stream**)&pSecureIStrm);
	}

	OpcUa_String_Clear(&sSecurityPolicyUri);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	if(pSenderCertificateChain != OpcUa_Null)
	{
		OpcUa_Free(pSenderCertificateChain);
	}

	OpcUa_ByteString_Clear(&SenderCertificateChain);
	OpcUa_ByteString_Clear(&ReceiverCertificateThumbprint);
	OpcUa_Key_Clear(&serverNonce);

	OpcUa_SecureListener_ChannelManager_ReleaseChannel(
			pSecureListener->ChannelManager,
			&pSecureChannel);

	if(pRequest != OpcUa_Null)
	{
		OpcUa_OpenSecureChannelRequest_Clear(pRequest);
		OpcUa_Free(pRequest);
		pRequest = OpcUa_Null;
	}

	if(pSecurityToken != OpcUa_Null)
	{
		OpcUa_Free(pSecurityToken);
		pSecurityToken = OpcUa_Null;
	}

	if(pCryptoProvider)
	{
		OPCUA_P_CRYPTOFACTORY_DELETECRYPTOPROVIDER(pCryptoProvider);
		OpcUa_Free(pCryptoProvider);
		pCryptoProvider = OpcUa_Null;
	}

	if((pSecureStream != OpcUa_Null) && pSecureStream->InnerStrm != OpcUa_Null)
	{
		pSecureStream->InnerStrm->Close((OpcUa_Stream*)pSecureStream->InnerStrm);
		pSecureStream->InnerStrm->Delete((OpcUa_Stream**)&(pSecureStream->InnerStrm));
		*a_ppTransportIstrm = OpcUa_Null;
	}

	if(pSecureIStrm != OpcUa_Null)
	{
		OpcUa_Stream_Close((OpcUa_Stream*)pSecureIStrm);
		OpcUa_Stream_Delete((OpcUa_Stream**)&pSecureIStrm);
	}

	if(a_ppTransportIstrm != OpcUa_Null && (*a_ppTransportIstrm) != OpcUa_Null)
	{
		(*a_ppTransportIstrm)->Close((OpcUa_Stream*)(*a_ppTransportIstrm));
		(*a_ppTransportIstrm)->Delete((OpcUa_Stream**)a_ppTransportIstrm);
	}

	OpcUa_String_Clear(&sSecurityPolicyUri);

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_SecureListener_ProcessCloseSecureChannelRequest
 *===========================================================================*/
OpcUa_StatusCode OpcUa_SecureListener_ProcessCloseSecureChannelRequest(
	OpcUa_Listener*                     a_pSecureListenerInterface,
	OpcUa_Handle                        a_hTransportConnection,
	OpcUa_InputStream**                 a_ppTransportIstrm,
	OpcUa_Boolean                       a_bRequestComplete)
{
	OpcUa_SecureListener*               pSecureListener         = OpcUa_Null;
	OpcUa_InputStream*                  pSecureIstrm            = OpcUa_Null;
	OpcUa_SecureStream*                 pSecureStream           = OpcUa_Null;
	OpcUa_SecureChannel*                pSecureChannel          = OpcUa_Null;
	OpcUa_CryptoProvider*               pCryptoProvider         = OpcUa_Null;

	OpcUa_CloseSecureChannelRequest*    pRequest                = OpcUa_Null;

	OpcUa_UInt32                        uTokenId                = 0;
	OpcUa_UInt32                        uSecureChannelId        = 0;

	OpcUa_SecurityKeyset*               pReceivingKeyset        = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_SecureListener, "ProcessCloseSecureChannelRequest");

	OpcUa_ReturnErrorIfArgumentNull(a_pSecureListenerInterface);
	OpcUa_ReturnErrorIfArgumentNull(a_hTransportConnection);
	OpcUa_ReturnErrorIfArgumentNull(a_ppTransportIstrm);

	/*** get listener handle ***/
	pSecureListener = (OpcUa_SecureListener*)a_pSecureListenerInterface->Handle;
	OpcUa_ReturnErrorIfNull(pSecureListener, OpcUa_BadInvalidState);
	
	/* parse stream header */
	uStatus = OpcUa_SecureStream_DecodeSymmetricSecurityHeader( *a_ppTransportIstrm,
																&uSecureChannelId,
																&uTokenId);
	OpcUa_GotoErrorIfBad(uStatus);

	OpcUa_Trace(    OPCUA_TRACE_STACK_LEVEL_ERROR,
					"ProcessCloseSecureChannelRequest: SecureChannelId %u, SecurityTokenId %u\n",
					uSecureChannelId,
					uTokenId);

	/*** find appropriate channel in channel manager or create CryptoProvider with SecurityPolicy ***/
	uStatus = OpcUa_SecureListener_ChannelManager_GetChannelBySecureChannelID(  pSecureListener->ChannelManager,
																				uSecureChannelId,
																				&pSecureChannel);
	OpcUa_GotoErrorIfBad(uStatus);

	/* check for correct transport connection */
	if(pSecureChannel->TransportConnection != a_hTransportConnection)
	{
		OpcUa_GotoErrorWithStatus(OpcUa_BadSecureChannelIdInvalid);
	}

	/* look if there is a pending stream */
	uStatus = OpcUa_SecureChannel_GetPendingInputStream(    pSecureChannel,
															&pSecureIstrm);
	OpcUa_GotoErrorIfBad(uStatus);

	/* Get reference to keyset for requested token id */
	uStatus = pSecureChannel->GetSecuritySet(   pSecureChannel,
												uTokenId,
												&pReceivingKeyset,
												OpcUa_Null,
												&pCryptoProvider);
	OpcUa_GotoErrorIfBad(uStatus);

	if(pSecureIstrm == OpcUa_Null)
	{
		/* create inputstream and check if processing can be started or delayed until last chunk is received. */
		uStatus = OpcUa_SecureStream_CreateInput(   pCryptoProvider,
													pSecureChannel->MessageSecurityMode,
													pSecureChannel->nMaxBuffersPerMessage,
													&pSecureIstrm);
		if(OpcUa_IsBad(uStatus))
		{
			/* release reference to security set */
			pSecureChannel->ReleaseSecuritySet(   pSecureChannel,
												  uTokenId);

			OpcUa_GotoError;
		}

		/* append keep the secure stream pending and waiting for further chunks */
		uStatus = OpcUa_SecureStream_AppendInput(   a_ppTransportIstrm,
													pSecureIstrm,
													&pReceivingKeyset->SigningKey,
													&pReceivingKeyset->EncryptionKey,
													&pReceivingKeyset->InitializationVector,
													pCryptoProvider);

		/* release reference to security set */
		pSecureChannel->ReleaseSecuritySet(   pSecureChannel,
											  uTokenId);

		OpcUa_GotoErrorIfBad(uStatus);

		/* delete transport stream */
		if(*a_ppTransportIstrm != OpcUa_Null)
		{
			(*a_ppTransportIstrm)->Delete((OpcUa_Stream**)a_ppTransportIstrm);
		}

		pSecureStream                       = (OpcUa_SecureStream*)pSecureIstrm->Handle;
		pSecureStream->SecureChannelId      = uSecureChannelId;
		pSecureStream->eMessageType         = eOpcUa_SecureStream_Types_StandardMessage;
	}
	else
	{
		/* append keep the secure stream pending and waiting for further chunks */
		uStatus = OpcUa_SecureStream_AppendInput(   a_ppTransportIstrm,
													pSecureIstrm,
													&pReceivingKeyset->SigningKey,
													&pReceivingKeyset->EncryptionKey,
													&pReceivingKeyset->InitializationVector,
													pCryptoProvider);

		/* release reference to security set */
		pSecureChannel->ReleaseSecuritySet(   pSecureChannel,
											  uTokenId);

		OpcUa_GotoErrorIfBad(uStatus);

		pSecureStream = (OpcUa_SecureStream*)pSecureIstrm->Handle;

		/* buffer content is saved, instream can be deleted */
		if(*a_ppTransportIstrm != OpcUa_Null)
		{
			(*a_ppTransportIstrm)->Delete((OpcUa_Stream**)a_ppTransportIstrm);
		}
	}

	/* look if we can start processing the stream */
	if(a_bRequestComplete == OpcUa_False)
	{
		/* Set the current stream as the pending request stream and leave. */
		OpcUa_SecureChannel_SetPendingInputStream(  pSecureChannel,
													pSecureIstrm);

		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "ProcessCloseSecureChannelRequest: Waiting for more chunks!\n");

		pSecureIstrm = OpcUa_Null;
		pSecureStream = OpcUa_Null;
	}
	else /* preprocess the stream */
	{
		/* Reset the pending request stream. */
		OpcUa_SecureChannel_SetPendingInputStream(  pSecureChannel,
													OpcUa_Null);

		/* get uTokenId */
		if(uTokenId != pSecureChannel->CurrentChannelSecurityToken.TokenId)
		{
			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_WARNING, "ProcessCloseSecureChannelRequest: TokenId does not match!\n");
		}

		/* reset buffer index to start reading from the first buffer */
		pSecureStream->nCurrentReadBuffer = 0;

		/*** read encoded request ***/
		uStatus = OpcUa_SecureListener_ReadRequest( pSecureListener,
													pSecureIstrm,
													OpcUaId_CloseSecureChannelRequest,
													(OpcUa_Void**)&pRequest);
		OpcUa_GotoErrorIfBad(uStatus);

		/*** invoke callback ***/
		if(pSecureListener->SecureChannelCallback)
		{
			OpcUa_Mutex_Unlock(pSecureListener->Mutex);
			pSecureListener->SecureChannelCallback( pSecureChannel->SecureChannelId,
													eOpcUa_SecureListener_SecureChannelClose,
													OpcUa_Good,
													OpcUa_Null,
													OpcUa_Null,
													0,
													0,
													pSecureListener->SecureChannelCallbackData);
			OpcUa_Mutex_Lock(pSecureListener->Mutex);
		}

		/* close the SecureChannel */
		uStatus = pSecureChannel->Close(pSecureChannel);
		OpcUa_GotoErrorIfBad(uStatus);
	}

	/*** clean up ***/
	OpcUa_SecureListener_ChannelManager_ReleaseChannel(
			pSecureListener->ChannelManager,
			&pSecureChannel);

	if(pRequest != OpcUa_Null)
	{
		OpcUa_CloseSecureChannelRequest_Clear(pRequest);
		OpcUa_Free(pRequest);
	}

	if(pSecureIstrm != OpcUa_Null)
	{
		OpcUa_Stream_Close((OpcUa_Stream*)pSecureIstrm);
		OpcUa_Stream_Delete((OpcUa_Stream**)&pSecureIstrm);
	}

#if OPCUA_SECURELISTENER_CLOSE_CONNECTION_ON_CLOSE_SECURECHANNEL
	/* transport layer closes the connection on this status code */
	uStatus = OpcUa_BadDisconnect;
#endif /* OPCUA_SECURELISTENER_CLOSE_CONNECTION_ON_CLOSE_SECURECHANNEL */

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
	
	OpcUa_SecureListener_ChannelManager_ReleaseChannel(
			pSecureListener->ChannelManager,
			&pSecureChannel);

	if(pRequest != OpcUa_Null)
	{
		OpcUa_CloseSecureChannelRequest_Clear(pRequest);
		OpcUa_Free(pRequest);
		pRequest = OpcUa_Null;
	}

#if 1
	if(a_ppTransportIstrm != OpcUa_Null && (*a_ppTransportIstrm) != OpcUa_Null)
	{
		(*a_ppTransportIstrm)->Delete((OpcUa_Stream**)a_ppTransportIstrm);
	}
#endif

	if(pSecureIstrm != OpcUa_Null)
	{
		OpcUa_Stream_Close((OpcUa_Stream*)pSecureIstrm);
		OpcUa_Stream_Delete((OpcUa_Stream**)&pSecureIstrm);
	}

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_SecureListener_ValidateDiscoveryChannel
 *===========================================================================*/
#if OPCUA_P_BYTE_ORDER == OPCUA_P_LITTLE_ENDIAN
#define OpcUa_MakeFourByteNodeId(x) (0x00000001 | (((x)&0x0000FFFF)<<16))
#else
#define OpcUa_MakeFourByteNodeId(x) (0x01000000 | (((x)&0xFF00)>>8) | (((x)&0xFF)<<8))
#endif

OpcUa_StatusCode OpcUa_SecureListener_ValidateDiscoveryChannel(OpcUa_InputStream* a_pStream)
{
	OpcUa_UInt32 uTypeId = 0;
	OpcUa_UInt32 uTypeIdSize = sizeof(uTypeId);
	OpcUa_UInt32 uPosition = 0;

OpcUa_InitializeStatus(OpcUa_Module_SecureListener, "ValidateDiscoveryChannel");

	OpcUa_ReturnErrorIfArgumentNull(a_pStream);

	uStatus = OpcUa_Stream_GetPosition((OpcUa_Stream*)a_pStream, &uPosition);
	OpcUa_GotoErrorIfBad(uStatus);

	uStatus = OpcUa_Stream_Read(a_pStream, (OpcUa_Byte*)&uTypeId, &uTypeIdSize, OpcUa_Null, OpcUa_Null);
	OpcUa_GotoErrorIfBad(uStatus);

	if (uTypeId != OpcUa_MakeFourByteNodeId(OpcUaId_GetEndpointsRequest_Encoding_DefaultBinary) && uTypeId != OpcUa_MakeFourByteNodeId(OpcUaId_FindServersRequest_Encoding_DefaultBinary))
	{
		OpcUa_GotoErrorWithStatus(OpcUa_BadSecurityPolicyRejected);
	}

	uStatus = OpcUa_Stream_SetPosition((OpcUa_Stream*)a_pStream, uPosition);
	OpcUa_GotoErrorIfBad(uStatus);
	
OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_SecureListener_ThreadPoolJobMain
 *===========================================================================*/
#if OPCUA_SECURELISTENER_SUPPORT_THREADPOOL
static OpcUa_Void OpcUa_SecureListener_ThreadPoolJobMain(OpcUa_Void* a_pArgument)
{
	OpcUa_StatusCode                            uStatus             = OpcUa_Good;
	OpcUa_SecureListener_ThreadPoolJobArgument* pArg                = (OpcUa_SecureListener_ThreadPoolJobArgument*)a_pArgument;
	OpcUa_SecureListener*                       pSecureListener     = OpcUa_Null;
	OpcUa_SecureStream*                         pSecureStream       = OpcUa_Null;
	OpcUa_SecureChannel*                        pSecureChannel      = OpcUa_Null;
	OpcUa_CryptoProvider*                       pCryptoProvider     = OpcUa_Null;
	OpcUa_SecurityKeyset*                       pReceivingKeyset    = OpcUa_Null;

	if(pArg == OpcUa_Null)
	{
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "SecureListener: Empty Arg!!\n");
		return;
	}
	else
	{
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_ThreadPoolJobMain started processing of job %p\n", pArg);
	}

	pSecureListener = (OpcUa_SecureListener*)pArg->pListener->Handle;

	/*** find appropriate channel in channel manager or create CryptoProvider with SecurityPolicy ***/
	uStatus = OpcUa_SecureListener_ChannelManager_GetChannelBySecureChannelID(  pSecureListener->ChannelManager,
																				pArg->uSecureChannelId,
																				&pSecureChannel);

	if(OpcUa_IsGood(uStatus))
	{
		/* Get reference to keyset for requested token id */
		uStatus = pSecureChannel->GetSecuritySet(   pSecureChannel,
													pArg->uTokenId,
													&pReceivingKeyset,
													OpcUa_Null,
													&pCryptoProvider);
	}

	if(OpcUa_IsGood(uStatus) && pReceivingKeyset != OpcUa_Null)
	{
		/* this is the final chunk */
		uStatus = OpcUa_SecureStream_AppendInput(   &pArg->pTransportIstrm,
													 pArg->pSecureIstrm,
													&pReceivingKeyset->SigningKey,
													&pReceivingKeyset->EncryptionKey,
													&pReceivingKeyset->InitializationVector,
													 pCryptoProvider);

		/* release reference to security set */
		pSecureChannel->ReleaseSecuritySet( pSecureChannel,
											pArg->uTokenId);

		if(OpcUa_IsGood(uStatus))
		{
			pSecureStream = (OpcUa_SecureStream*)pArg->pSecureIstrm->Handle;

			/* reset buffer index to start reading from the first buffer */
			pSecureStream->nCurrentReadBuffer = 0;

			/* ensure that discovery only channels don't process non-discovery requests */
			if(pArg->bDiscoveryOnly != OpcUa_False)
			{
				uStatus = OpcUa_SecureListener_ValidateDiscoveryChannel(pArg->pSecureIstrm);
			}

			if(OpcUa_IsGood(uStatus))
			{
				/*** invoke callback */
				/* this goes to the owner of the secure listener, which is the endpoint ***/
				if(pSecureListener->Callback != OpcUa_Null)
				{
					uStatus = pSecureListener->Callback(    pArg->pListener,                /* the source of the event          */
															pSecureListener->CallbackData,  /* the callback data                */
															OpcUa_ListenerEvent_Request,    /* the type of the event            */
															pArg->hConnection,              /* the handle for the connection    */
															&pArg->pSecureIstrm,            /* the stream to read from          */
															OpcUa_Good);                    /* the event status                 */
				}

				OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_ThreadPoolJobMain: Endpoint returned with status 0x%08X\n", uStatus);
			}
			else
			{
				OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_ThreadPoolJobMain: OpcUa_SecureListener_ValidateDiscoveryChannel failed with status 0x%08X\n", uStatus);
			}

		}
		else
		{
			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_ThreadPoolJobMain: OpcUa_SecureStream_AppendInput failed with status 0x%08X\n", uStatus);
		}
	}
	else
	{
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_ThreadPoolJobMain: Could not get securechannel for id %u: 0x%08X\n", pArg->uSecureChannelId, uStatus);
	}

	OpcUa_SecureListener_ChannelManager_ReleaseChannel(
			pSecureListener->ChannelManager,
			&pSecureChannel);

	/* delete stream */
	OpcUa_Stream_Delete((OpcUa_Stream**)&(pArg->pSecureIstrm));

	/* delete argument */
	OpcUa_Free(pArg);

	return;
}
#endif /* OPCUA_SECURELISTENER_SUPPORT_THREADPOOL */

/*============================================================================
 * OpcUa_SecureListener_ProcessSessionCallRequest
 *===========================================================================*/
/* HINT: Function assumes that its called with SecureListener object locked once! 
		 SecureListener is released during callback. */
OpcUa_StatusCode OpcUa_SecureListener_ProcessSessionCallRequest(
	OpcUa_Listener*                 a_pListener,
	OpcUa_Handle                    a_hConnection,
	OpcUa_InputStream**             a_ppTransportIstrm,
	OpcUa_Boolean                   a_bRequestComplete)
{
	OpcUa_SecureListener*   pSecureListener         = OpcUa_Null;
	OpcUa_InputStream*      pSecureIStrm            = OpcUa_Null;
	OpcUa_SecureStream*     pSecureStream           = OpcUa_Null;
	OpcUa_SecureChannel*    pSecureChannel          = OpcUa_Null;
	OpcUa_CryptoProvider*   pCryptoProvider         = OpcUa_Null;

	OpcUa_UInt32            uTokenId                = 0;
	OpcUa_UInt32            uSecureChannelId        = OPCUA_SECURECHANNEL_ID_INVALID;
	OpcUa_SecurityKeyset*   pReceivingKeyset        = OpcUa_Null;

#if OPCUA_SECURELISTENER_SUPPORT_THREADPOOL
	OpcUa_SecureListener_ThreadPoolJobArgument* pPoolJob = OpcUa_Null;
#endif /* OPCUA_SECURELISTENER_SUPPORT_THREADPOOL */

OpcUa_InitializeStatus(OpcUa_Module_SecureListener, "OpcUa_SecureListener_ProcessSessionCallRequest");
	
	OpcUa_ReturnErrorIfArgumentNull(a_pListener);
	OpcUa_ReturnErrorIfArgumentNull(a_hConnection);
	OpcUa_ReturnErrorIfArgumentNull(a_ppTransportIstrm);
	OpcUa_ReturnErrorIfArgumentNull(*a_ppTransportIstrm);

	/*** get listener handle ***/
	pSecureListener = (OpcUa_SecureListener*)a_pListener->Handle;
	OpcUa_ReturnErrorIfNull(pSecureListener, OpcUa_BadInvalidState);
	
	/* parse stream header */
	uStatus = OpcUa_SecureStream_DecodeSymmetricSecurityHeader( *a_ppTransportIstrm,
																&uSecureChannelId,
																&uTokenId);
	OpcUa_GotoErrorIfBad(uStatus);

	OpcUa_Trace(    OPCUA_TRACE_STACK_LEVEL_ERROR,
					"OpcUa_SecureListener_ProcessSessionCallRequest: SecureChannelId %u, SecurityTokenId %u\n",
					uSecureChannelId,
					uTokenId);

	/*** find appropriate channel in channel manager or create CryptoProvider with SecurityPolicy ***/
	uStatus = OpcUa_SecureListener_ChannelManager_GetChannelBySecureChannelID(  pSecureListener->ChannelManager,
																				uSecureChannelId,
																				&pSecureChannel);
	OpcUa_GotoErrorIfBad(uStatus);

	/* check for correct transport connection */
	if(pSecureChannel->TransportConnection != a_hConnection)
	{
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_WARNING, "OpcUa_SecureListener_ProcessSessionCallRequest: Call from different transport connection!\n");
		OpcUa_SecureListener_ChannelManager_ReleaseChannel( pSecureListener->ChannelManager,
														   &pSecureChannel);
		OpcUa_GotoErrorWithStatus(OpcUa_BadSecureChannelIdInvalid);
	}

	/* look if there is a pending stream */
	uStatus = OpcUa_SecureChannel_GetPendingInputStream(pSecureChannel,
														&pSecureIStrm);
	OpcUa_GotoErrorIfBad(uStatus);

	/* Get reference to keyset for requested token id */
	uStatus = pSecureChannel->GetSecuritySet(   pSecureChannel,
												uTokenId,
												&pReceivingKeyset,
												OpcUa_Null,
												&pCryptoProvider);
	OpcUa_GotoErrorIfBad(uStatus);

	if(pSecureIStrm == OpcUa_Null)
	{
		/* create inputstream and check if processing can be started or delayed until last chunk is received. */
		uStatus = OpcUa_SecureStream_CreateInput(   pCryptoProvider,
													pSecureChannel->MessageSecurityMode,
													pSecureChannel->nMaxBuffersPerMessage,
													&pSecureIStrm);
		if(OpcUa_IsBad(uStatus))
		{
			pSecureChannel->ReleaseSecuritySet(   pSecureChannel,
												  uTokenId);
			OpcUa_GotoError;
		}

		pSecureStream                       = (OpcUa_SecureStream*)pSecureIStrm->Handle;
		pSecureStream->SecureChannelId      = uSecureChannelId;
		pSecureStream->eMessageType         = eOpcUa_SecureStream_Types_StandardMessage;
		pSecureStream->eMessageSecurityMode = pSecureChannel->MessageSecurityMode;
	}
	else
	{
		pSecureStream = (OpcUa_SecureStream*)pSecureIStrm->Handle;
	}

	/* look if we can start processing the stream */
	if(a_bRequestComplete == OpcUa_False)
	{
		/* this is not the final chunk */
		uStatus = OpcUa_SecureStream_AppendInput(   a_ppTransportIstrm,
													pSecureIStrm,
													&pReceivingKeyset->SigningKey,
													&pReceivingKeyset->EncryptionKey,
													&pReceivingKeyset->InitializationVector,
													pCryptoProvider);
		/* release reference to security set */
		pSecureChannel->ReleaseSecuritySet(   pSecureChannel,
											  uTokenId);

		if(OpcUa_IsBad(uStatus))
		{
			/* Clear the pending input stream marker */
			OpcUa_SecureChannel_SetPendingInputStream(  pSecureChannel,
														OpcUa_Null);
			OpcUa_GotoError;
		}

		/* Set the current stream as the pending request stream and leave. */
		uStatus = OpcUa_SecureChannel_SetPendingInputStream(    pSecureChannel,
																pSecureIStrm);

		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_ProcessSessionCallRequest: Waiting for more chunks!\n");

		/* buffer has been saved; transport stream wont be used again; release it */
		if(*a_ppTransportIstrm != OpcUa_Null)
		{
			(*a_ppTransportIstrm)->Delete((OpcUa_Stream**)a_ppTransportIstrm);
		}

		pSecureStream = OpcUa_Null;
	}
	else /* preprocess the stream */
	{
		/* Clear the pending input stream marker */
		uStatus = OpcUa_SecureChannel_SetPendingInputStream(    pSecureChannel,
																OpcUa_Null);
		OpcUa_GotoErrorIfBad(uStatus);

		/* HINT: add + 1 since the last chunks is not appended yet. */
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_SecureListener_ProcessSessionCallRequest: All %u chunks received; start processing!\n", pSecureStream->nBuffers + 1);

		pSecureStream->SecureChannelId = pSecureChannel->SecureChannelId;

#if OPCUA_SECURELISTENER_SUPPORT_THREADPOOL
		if(OpcUa_ProxyStub_g_Configuration.bSecureListener_ThreadPool_Enabled == OpcUa_False)
		{
#endif /* OPCUA_SECURELISTENER_SUPPORT_THREADPOOL */

			/* this is the final chunk */
			uStatus = OpcUa_SecureStream_AppendInput(   a_ppTransportIstrm,
														pSecureIStrm,
														&pReceivingKeyset->SigningKey,
														&pReceivingKeyset->EncryptionKey,
														&pReceivingKeyset->InitializationVector,
														pCryptoProvider);
			/* release reference to security set */
			pSecureChannel->ReleaseSecuritySet( pSecureChannel,
												uTokenId);

			OpcUa_GotoErrorIfBad(uStatus);

			/* reset buffer index to start reading from the first buffer */
			pSecureStream->nCurrentReadBuffer = 0;

			/* ensure that discovery only channels don't process non-discovery requests */
			if (pSecureChannel->DiscoveryOnly)
			{
				uStatus = OpcUa_SecureListener_ValidateDiscoveryChannel(pSecureIStrm);
				if(OpcUa_IsBad(uStatus))
				{
					OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_WARNING, "OpcUa_SecureListener_ProcessSessionCallRequest: NonDiscovery Service requested through non secure channel.\n");
					OpcUa_GotoError;
				}
			}

			/*** invoke callback */
			/* this goes to the owner of the secure listener, which is the endpoint ***/
			if(pSecureListener->Callback != OpcUa_Null)
			{
				/*** release lock. ***/
				OpcUa_Mutex_Unlock(pSecureListener->Mutex);

				uStatus = pSecureListener->Callback(
					a_pListener,                        /* the source of the event          */
					pSecureListener->CallbackData,      /* the callback data                */
					OpcUa_ListenerEvent_Request,        /* the type of the event            */
					a_hConnection,                      /* the handle for the connection    */
					&pSecureIStrm,                      /* the stream to read from          */
					uStatus);                           /* the event status                 */

				if(pSecureIStrm == OpcUa_Null)
				{
					/* transport stream is deleted with secure stream */
					*a_ppTransportIstrm = OpcUa_Null;
				}

				/*** acquire lock until callback is complete. ***/
				OpcUa_Mutex_Lock(pSecureListener->Mutex);
				OpcUa_GotoErrorIfBad(uStatus);
			}
#if OPCUA_SECURELISTENER_SUPPORT_THREADPOOL
		}
		else
		{
			/* release reference to security set */
			pSecureChannel->ReleaseSecuritySet( pSecureChannel,
												uTokenId);

			/* Clear the pending input stream marker */
			uStatus = OpcUa_SecureChannel_SetPendingInputStream(  pSecureChannel,
																  OpcUa_Null);
			OpcUa_GotoErrorIfBad(uStatus);

			/* create a threadpool job */
			pPoolJob = (OpcUa_SecureListener_ThreadPoolJobArgument*)OpcUa_Alloc(sizeof(OpcUa_SecureListener_ThreadPoolJobArgument));
			OpcUa_GotoErrorIfAllocFailed(pPoolJob);
			OpcUa_MemSet(pPoolJob, 0, sizeof(OpcUa_SecureListener_ThreadPoolJobArgument));

			pPoolJob->pListener         = a_pListener;
			pPoolJob->hConnection       = a_hConnection;
			pPoolJob->pTransportIstrm   = *a_ppTransportIstrm;
			pPoolJob->pSecureIstrm      = pSecureIStrm;
			pPoolJob->pCallback         = pSecureListener->CallbackData;
			pPoolJob->pCallbackData     = pSecureListener->CallbackData;
			pPoolJob->bDiscoveryOnly    = pSecureChannel->DiscoveryOnly;
			pPoolJob->uTokenId          = uTokenId;
			pPoolJob->uSecureChannelId  = uSecureChannelId;

			*a_ppTransportIstrm = OpcUa_Null;

			/*** release lock. ***/
			OpcUa_Mutex_Unlock(pSecureListener->Mutex);

			/* add a threadpool job */
			uStatus = OpcUa_ThreadPool_AddJob(  pSecureListener->hThreadPool,
												OpcUa_SecureListener_ThreadPoolJobMain,
												pPoolJob);

			/*** acquire lock until add job is complete. ***/
			OpcUa_Mutex_Lock(pSecureListener->Mutex);

			if(OpcUa_IsEqual(OpcUa_BadWouldBlock))
			{
				uStatus = OpcUa_BadTcpServerTooBusy;
			}

			OpcUa_GotoErrorIfBad(uStatus);
		}
#endif /* OPCUA_SECURELISTENER_SUPPORT_THREADPOOL */
	}

	OpcUa_SecureListener_ChannelManager_ReleaseChannel(
		pSecureListener->ChannelManager,
		&pSecureChannel);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
	
#if OPCUA_SECURELISTENER_SUPPORT_THREADPOOL
	if(pPoolJob != OpcUa_Null)
	{
		OpcUa_Free(pPoolJob);
	}
#endif  /* OPCUA_SECURELISTENER_SUPPORT_THREADPOOL */

	/* clear pending input stream  */
	OpcUa_SecureChannel_SetPendingInputStream(  pSecureChannel,
												OpcUa_Null);

	OpcUa_SecureListener_ChannelManager_ReleaseChannel(
			pSecureListener->ChannelManager,
			&pSecureChannel);

	/* delete stream */
	OpcUa_Stream_Delete((OpcUa_Stream**)&pSecureIStrm);

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_SecureListener_GetChannelId
 *===========================================================================*/
OpcUa_StatusCode OpcUa_SecureListener_GetChannelId(
	OpcUa_Listener*     a_pListener, 
	OpcUa_InputStream*  a_pIstrm, 
	OpcUa_UInt32*       a_puChannelId)
{
	OpcUa_SecureStream* pStream = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_SecureListener, "OpcUa_SecureListener_GetChannelId");

	OpcUa_ReturnErrorIfArgumentNull(a_puChannelId);
	OpcUa_ReturnErrorIfArgumentNull(a_pIstrm);
	pStream = (OpcUa_SecureStream*)a_pIstrm->Handle;
	OpcUa_ReturnErrorIfArgumentNull(pStream);

	OpcUa_ReferenceParameter(a_pListener);

	*a_puChannelId = pStream->SecureChannelId;

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_SecureListener_GetSecurityPolicyConfiguration
 *===========================================================================*/
OpcUa_StatusCode OpcUa_SecureListener_GetSecurityPolicyConfiguration(
	OpcUa_Listener*                             a_pListener,
	OpcUa_InputStream*                          a_pIstrm,
	OpcUa_Listener_SecurityPolicyConfiguration* a_pSecurityPolicyConfiguration)
{
	OpcUa_SecureListener*   pSecureListener = (OpcUa_SecureListener*)a_pListener->Handle;
	OpcUa_SecureChannel*    pSecureChannel  = OpcUa_Null;
	OpcUa_UInt32            uChannelId      = 0;

OpcUa_InitializeStatus(OpcUa_Module_SecureListener, "GetSecurityPolicyConfiguration");

	OpcUa_ReturnErrorIfArgumentNull(a_pListener);
	OpcUa_ReturnErrorIfArgumentNull(a_pSecurityPolicyConfiguration);

	uStatus = OpcUa_SecureListener_GetChannelId(a_pListener,
												a_pIstrm,
												&uChannelId);
	OpcUa_GotoErrorIfBad(uStatus);

	/* get the securechannel with the given channel id and return the */
	uStatus = OpcUa_SecureListener_ChannelManager_GetChannelBySecureChannelID(
		pSecureListener->ChannelManager,
		uChannelId,
		&pSecureChannel);
	OpcUa_GotoErrorIfBad(uStatus);

	/* maybe this could be the wrong provider if the channel gets renewed between
	   the message is received and the call to this function ... the only ways to
	   solve this problem would be to store or reference the policy uri in the
	   message context (pretty safe, because the referenced data is) or to use the
	   channel security token. */
	OpcUa_String_AttachToString(pSecureChannel->pCurrentCryptoProvider->Name,
								OPCUA_STRING_LENDONTCARE,
								0,
								OpcUa_False,
								OpcUa_False,
								&a_pSecurityPolicyConfiguration->sSecurityPolicy);

	/* the source enum only has values that fit into 16 bit, so this is safe */
	a_pSecurityPolicyConfiguration->uMessageSecurityModes = (OpcUa_UInt16)pSecureChannel->MessageSecurityMode;
	a_pSecurityPolicyConfiguration->pbsClientCertificate = OpcUa_Null;

	OpcUa_SecureListener_ChannelManager_ReleaseChannel(
			pSecureListener->ChannelManager,
			&pSecureChannel);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_SecureListener_GetSecureChannelSecurityPolicyConfiguration
 *===========================================================================*/
OpcUa_StatusCode OpcUa_SecureListener_GetSecureChannelSecurityPolicyConfiguration(
	OpcUa_Listener*                                     a_pListener,
	OpcUa_UInt32                                        a_uChannelId,
	OpcUa_SecureListener_SecurityPolicyConfiguration*   a_pSecurityPolicyConfiguration)
{
	OpcUa_SecureListener*       pSecureListener     = (OpcUa_SecureListener*)a_pListener->Handle;
	OpcUa_SecureChannel*        pSecureChannel      = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_SecureListener, "GetSecureChannelSecurityPolicyConfiguration");

	OpcUa_ReturnErrorIfArgumentNull(a_pListener);
	OpcUa_ReturnErrorIfArgumentNull(a_pSecurityPolicyConfiguration);

	/* get the securechannel with the given channel id and return the */
	uStatus = OpcUa_SecureListener_ChannelManager_GetChannelBySecureChannelID(
		pSecureListener->ChannelManager,
		a_uChannelId,
		&pSecureChannel);
	OpcUa_GotoErrorIfBad(uStatus);

	/* maybe this could be the wrong provider if the channel gets renewed between
	   the message is received and the call to this function ... the only ways to
	   solve this problem would be to store or reference the policy uri in the
	   message context (pretty safe, because the referenced data is) or to use the
	   channel security token. */
	OpcUa_String_AttachToString(pSecureChannel->pCurrentCryptoProvider->Name,
								OPCUA_STRING_LENDONTCARE,
								0,
								OpcUa_False,
								OpcUa_False,
								&a_pSecurityPolicyConfiguration->sSecurityPolicy);

	/* the source enum only has values that fit into 16 bit, so this is safe */
	a_pSecurityPolicyConfiguration->uMessageSecurityModes = (OpcUa_UInt16)pSecureChannel->MessageSecurityMode;
	a_pSecurityPolicyConfiguration->pbsClientCertificate = OpcUa_Null;

	OpcUa_SecureListener_ChannelManager_ReleaseChannel(
			pSecureListener->ChannelManager,
			&pSecureChannel);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_SecureListener_GetPeerInfoBySecureChannelId
 *===========================================================================*/
OpcUa_StatusCode OpcUa_SecureListener_GetPeerInfoBySecureChannelId(
	OpcUa_Listener* a_pListener,
	OpcUa_UInt32    a_uChannelId,
	OpcUa_String*   a_pPeerInfo)
{
	OpcUa_SecureListener*   pSecureListener = OpcUa_Null;
	OpcUa_SecureChannel*    pSecureChannel  = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_SecureListener, "GetPeerInfoBySecureChannelId");

	OpcUa_ReturnErrorIfArgumentNull(a_pListener);
	OpcUa_ReturnErrorIfArgumentNull(a_pPeerInfo);

	pSecureListener = (OpcUa_SecureListener*)a_pListener->Handle;

	/* get the securechannel with the given channel id */
	uStatus = OpcUa_SecureListener_ChannelManager_GetChannelBySecureChannelID(  pSecureListener->ChannelManager,
																				a_uChannelId,
																			   &pSecureChannel);
	OpcUa_GotoErrorIfBad(uStatus);

	/* return the peer info of the secure channel */
	uStatus = OpcUa_String_StrnCpy( a_pPeerInfo,
								   &pSecureChannel->sPeerInfo,
									OPCUA_STRING_LENDONTCARE);

	OpcUa_SecureListener_ChannelManager_ReleaseChannel( pSecureListener->ChannelManager,
													   &pSecureChannel);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_SecureListener_GetPeerInfo
 *===========================================================================*/
OpcUa_StatusCode OpcUa_SecureListener_GetPeerInfo(
	OpcUa_Listener*                             a_pListener,
	OpcUa_Handle                                a_hConnection,
	OpcUa_String*                               a_psPeerInfo)
{
	OpcUa_SecureListener* pSecureListener = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_SecureListener, "GetPeerInfoBySecureChannelId");

	OpcUa_ReturnErrorIfArgumentNull(a_pListener);

	pSecureListener = (OpcUa_SecureListener*)a_pListener->Handle;

	uStatus = OpcUa_Listener_GetPeerInfo(   pSecureListener->TransportListener,
											a_hConnection,
											a_psPeerInfo);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_SecureListener_GetConnectionHandleBySecureChannelId
 *===========================================================================*/
OpcUa_StatusCode OpcUa_SecureListener_GetConnectionHandleBySecureChannelId(
	OpcUa_Listener*                             a_pListener,
	OpcUa_UInt32                                a_uChannelId,
	OpcUa_Handle*                               a_phConnection)
{
OpcUa_InitializeStatus(OpcUa_Module_SecureListener, "GetConnectionHandleBySecureChannelId");

	OpcUa_ReturnErrorIfArgumentNull(a_pListener);
	OpcUa_ReturnErrorIfArgumentNull(a_phConnection);

	/* It is not safe to return a pointer to the secure channel because  */
	/* the ref count would not be handled safely outside this module.    */
	/* The generic listener API does not fit the current needs any more, */
	/* because of the specific secure channel id addressing, while the   */
	/* generic listener API works with handles.                          */
	*a_phConnection = (OpcUa_Handle)a_uChannelId;

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

#endif /* OPCUA_HAVE_SERVERAPI */
