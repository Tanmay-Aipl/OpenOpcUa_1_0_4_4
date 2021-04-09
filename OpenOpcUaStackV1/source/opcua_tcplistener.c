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

#include <opcua.h>

#ifdef OPCUA_HAVE_SERVERAPI

#include <opcua_mutex.h>
#include <opcua_string.h>
#include <opcua_datetime.h>
#include <opcua_socket.h>
#include <opcua_statuscodes.h>
#include <opcua_list.h>
#include <opcua_utilities.h>

#include <opcua_tcpstream.h>
#include <opcua_binaryencoder.h>

#include <opcua_tcplistener.h>

typedef struct _OpcUa_TcpListener OpcUa_TcpListener;

#include <opcua_tcplistener_connectionmanager.h>

/* for debugging reasons */
#include <opcua_p_binary.h>
#include <opcua_memorystream.h>

extern OpcUa_Guid OpcUa_Guid_Null;

/*============================================================================
 * Prototypes
 *===========================================================================*/
OpcUa_StatusCode OpcUa_TcpListener_Open(
	struct _OpcUa_Listener*         a_Listener,
	OpcUa_String*                   a_Url,
	OpcUa_Listener_PfnOnNotify*     a_Callback,
	OpcUa_Void*                     a_CallbackData);

OpcUa_StatusCode OpcUa_TcpListener_Close(
	OpcUa_Listener*                 a_Listener);

OpcUa_StatusCode OpcUa_TcpListener_BeginSendResponse(
	OpcUa_Listener*                 a_Listener,
	OpcUa_Handle                    a_pConnection,
	OpcUa_InputStream**             a_istrm,
	OpcUa_OutputStream**            a_ostrm);

OpcUa_StatusCode OpcUa_TcpListener_EndSendResponse(
	struct _OpcUa_Listener*         a_Listener,
	OpcUa_StatusCode                a_uStatus,
	OpcUa_OutputStream**            a_ostrm);

OpcUa_StatusCode OpcUa_TcpListener_AbortSendResponse(
	struct _OpcUa_Listener*         a_Listener,
	OpcUa_StatusCode                a_uStatus,
	OpcUa_String*                   a_psReason,
	OpcUa_OutputStream**            a_ostrm);

OpcUa_StatusCode OpcUa_TcpListener_CloseConnection(
	struct _OpcUa_Listener*         a_pListener,
	OpcUa_Handle                    a_hConnection,
	OpcUa_StatusCode                a_uStatus);

OpcUa_StatusCode OpcUa_TcpListener_ProcessDisconnect(   
	OpcUa_Listener*                 a_pListener, 
	OpcUa_TcpListener_Connection*   a_pTcpConnection);

static OpcUa_StatusCode OpcUa_TcpListener_SendErrorMessage( 
	OpcUa_Listener*                 a_pListener, 
	OpcUa_TcpListener_Connection*   a_pTcpConnection,
	OpcUa_StatusCode                a_uStatus,
	OpcUa_String*                   a_sReason);

OpcUa_StatusCode OpcUa_TcpListener_GetReceiveBufferSize(
	OpcUa_Listener*                 a_pListener,
	OpcUa_Handle                    a_hConnection,
	OpcUa_UInt32*                   a_pBufferSize);

OpcUa_StatusCode OpcUa_TcpListener_GetPeerInfo(
	OpcUa_Listener*                 a_pListener,
	OpcUa_Handle                    a_hConnection,
	OpcUa_String*                   a_sPeerInfo);

OpcUa_StatusCode OpcUa_TcpListener_AddToSendQueue(
	OpcUa_Listener*                 a_pListener,
	OpcUa_Handle                    a_hConnection,
	OpcUa_BufferListElement*        a_pBufferList,
	OpcUa_UInt32                    a_uFlags);
/*============================================================================
 * OpcUa_TcpListener_SanityCheck
 *===========================================================================*/
#define OpcUa_TcpListener_SanityCheck 0xE339EF96

/*============================================================================
 * OpcUa_TcpListener
 *===========================================================================*/
 /** @brief This struct represents a listener for tcp transport. */
struct _OpcUa_TcpListener
{
/* This is inherited from the OpcUa_Listener. */

	/*! @brief An opaque handle that contain data specific to the listener implementation. */
	OpcUa_Handle Handle;
	/*! @brief Begins listening at an endpoint. */
	OpcUa_Listener_PfnOpen* Open;
	/*! @brief Stops listening at an endpoint. */
	OpcUa_Listener_PfnClose* Close;
	/*! @brief Finishes reading an incoming request and starts writing the response. */
	OpcUa_Listener_PfnBeginSendResponse* BeginSendResponse;
	/*! @brief Finishes writing an outgoing response. */
	OpcUa_Listener_PfnEndSendResponse* EndSendResponse;
	/*! @brief Cancels writing an outgoing response. */
	OpcUa_Listener_PfnAbortSendResponse* AbortSendResponse;
	/*! @brief Retrive the recieve buffer size of a particular connection. */
	OpcUa_Listener_PfnGetReceiveBufferSize* GetReceiveBufferSize;
	/*! @brief Retrive the recieve security settings in use for the given stream. */
	OpcUa_Listener_PfnGetSecurityPolicyConfiguration* GetSecurityPolicyConfiguration;
	/*! @brief Close a particular connection. */
	OpcUa_Listener_PfnCloseConnection* CloseConnection;
	/*! @brief Frees the structure. */
	OpcUa_Listener_PfnDelete* Delete;
	/*! @brief Forward a buffer list to the transport for sending. */
	OpcUa_Listener_PfnAddToSendQueue* AddToSendQueue;
	/*! @brief Retrive the peer info of a particular connection. */
	OpcUa_Listener_PfnGetPeerInfo* GetPeerInfo;

/* End inherited from the OpcUa_Listener. */

	/** @brief Internal control value. */
	OpcUa_UInt32                            SanityCheck;
	/** @brief Synchronize access to the listener. */
	OpcUa_Mutex                             Mutex;
	/** @brief The listen socket (either part of the global or the own socket list). */
	OpcUa_Socket                            Socket;
#if OPCUA_MULTITHREADED
	/** @brief In multithreaded environments, each listener manages its own list of sockets. */
	OpcUa_SocketManager                     SocketManager;
#endif /* OPCUA_MULTITHREADED */
	/** @brief The function which receives notifications about listener events. */
	OpcUa_Listener_PfnOnNotify*             Callback;
	/** @brief Data passed with the callback function. */
	OpcUa_Void*                             CallbackData;
	/** @brief The default message chunk size for communicating with this listener. */
	OpcUa_UInt32                            DefaultChunkSize;
	/** @brief This Listener should shut down. */
	OpcUa_Boolean                           bShutdown;
	/** @brief This list contains all pending requests, which are not fully received
	 *  yet. Once a request is completely received, it gets dispatched to the
	 *  upper layer. */
	OpcUa_List*                             PendingMessages;
	/** @brief Holds the information about connected clients and helps verifying requests. */
	OpcUa_TcpListener_ConnectionManager*    ConnectionManager;
	/** @brief The maximum message size accepted by a connection at this listener. */
	OpcUa_UInt32                            MaxMessageSize;
	/** @brief The maximum number of chunks per message accepted by a connection at this listener. */
	OpcUa_UInt32                            MaxChunkCount;
	/** @brief Counts the number of unacknowledged accepted sockets. */
	OpcUa_UInt32                            PendingConnects;
	/** @brief Certificate used for SSL/TLS connections. */
	OpcUa_ByteString*                       pCertificate;
	/** @brief Private key used for SSL/TLS connections.*/
	OpcUa_Key*                              pPrivateKey;
	/** @brief PKI configuration for SSL/TLS connections. */
	OpcUa_Void*                             pPKIConfig;
};


/*============================================================================
 * OpcUa_TcpListener_Delete
 *===========================================================================*/
OpcUa_Void OpcUa_TcpListener_Delete(OpcUa_Listener** a_ppListener)
{
	OpcUa_TcpListener* pTcpListener = OpcUa_Null;
	OpcUa_InputStream* pInputStream = OpcUa_Null;

	if (a_ppListener == OpcUa_Null || *a_ppListener == OpcUa_Null)
	{
		return;
	}

	pTcpListener = (OpcUa_TcpListener*)(*a_ppListener)->Handle;

	if(pTcpListener != OpcUa_Null)
	{
		OpcUa_Mutex_Lock(pTcpListener->Mutex);
		pTcpListener->SanityCheck = 0;

		/* delete all pending messages */
		OpcUa_List_Enter(pTcpListener->PendingMessages);
		OpcUa_List_ResetCurrent(pTcpListener->PendingMessages);
		pInputStream = (OpcUa_InputStream *)OpcUa_List_GetCurrentElement(pTcpListener->PendingMessages);
		while(pInputStream != OpcUa_Null)
		{
			OpcUa_List_DeleteCurrentElement(pTcpListener->PendingMessages);
			pInputStream->Close((OpcUa_Stream*)pInputStream);
			pInputStream->Delete((OpcUa_Stream**)&pInputStream);
			pInputStream = (OpcUa_InputStream *)OpcUa_List_GetCurrentElement(pTcpListener->PendingMessages);
		}
		OpcUa_List_Leave(pTcpListener->PendingMessages);
		OpcUa_List_Delete(&(pTcpListener->PendingMessages));

		OpcUa_TcpListener_ConnectionManager_Delete(&(pTcpListener->ConnectionManager));

		OpcUa_Mutex_Unlock(pTcpListener->Mutex);
		OpcUa_Mutex_Delete(&(pTcpListener->Mutex));

		OpcUa_Free(pTcpListener);
	}

	*a_ppListener = OpcUa_Null;
}

/*============================================================================
 * OpcUa_TcpListener_Create
 *===========================================================================*/
OpcUa_StatusCode OpcUa_TcpListener_Create(  OpcUa_ByteString*   a_pServerCertificate,
											OpcUa_Key*          a_pServerPrivateKey,
											OpcUa_Void*         a_pPKIConfig,
											OpcUa_Listener**    a_pListener)
{
	OpcUa_TcpListener*  pTcpListener = OpcUa_Null;
	OpcUa_StatusCode uStatus=OpcUa_Good;

	if (a_pListener)
	{

		/* allocate listener object */
		*a_pListener = (OpcUa_Listener*)OpcUa_Alloc(sizeof(OpcUa_TcpListener));
		if (*a_pListener)
		{
			OpcUa_MemSet(*a_pListener, 0, sizeof(OpcUa_TcpListener));
			pTcpListener = (OpcUa_TcpListener*)*a_pListener;
			
			/* initialize listener pTcpListener */
			pTcpListener->SanityCheck = OpcUa_TcpListener_SanityCheck;
			pTcpListener->DefaultChunkSize = OpcUa_ProxyStub_g_Configuration.iTcpListener_DefaultChunkSize;
			pTcpListener->MaxChunkCount = OpcUa_ProxyStub_g_Configuration.iTcpTransport_MaxChunkCount;
			pTcpListener->MaxMessageSize = OpcUa_ProxyStub_g_Configuration.iTcpTransport_MaxMessageLength;

			pTcpListener->pPrivateKey   = a_pServerPrivateKey;
			pTcpListener->pCertificate  = a_pServerCertificate;
			pTcpListener->pPKIConfig    = a_pPKIConfig;

			uStatus = OpcUa_Mutex_Create(&(pTcpListener->Mutex));
			if (uStatus==OpcUa_Good)
			{
				uStatus = OpcUa_List_Create(&(pTcpListener->PendingMessages));
				if (uStatus==OpcUa_Good)
				{
					uStatus = OpcUa_TcpListener_ConnectionManager_Create(&(pTcpListener->ConnectionManager));
					if (uStatus==OpcUa_Good)
					{
						pTcpListener->ConnectionManager->Listener = *a_pListener;

						pTcpListener->PendingConnects           = 0;

						/* HINT: socket and socket list get managed in open/close */
						
						/* initialize listener object */
						(*a_pListener)->Handle                  = pTcpListener;
						(*a_pListener)->Open                    = OpcUa_TcpListener_Open;
						(*a_pListener)->Close                   = OpcUa_TcpListener_Close;
						(*a_pListener)->BeginSendResponse       = OpcUa_TcpListener_BeginSendResponse;
						(*a_pListener)->EndSendResponse         = OpcUa_TcpListener_EndSendResponse;
						(*a_pListener)->AbortSendResponse       = OpcUa_TcpListener_AbortSendResponse;
						(*a_pListener)->CloseConnection         = OpcUa_TcpListener_CloseConnection;
						(*a_pListener)->GetReceiveBufferSize    = OpcUa_TcpListener_GetReceiveBufferSize;
						(*a_pListener)->Delete                  = OpcUa_TcpListener_Delete;
						(*a_pListener)->AddToSendQueue          = OpcUa_TcpListener_AddToSendQueue;
						(*a_pListener)->GetPeerInfo             = OpcUa_TcpListener_GetPeerInfo;
					}
				}
			}
			if (uStatus!=OpcUa_Good)
			{
				if(pTcpListener->Mutex != OpcUa_Null)
				{
					OpcUa_Mutex_Delete(&(pTcpListener->Mutex));
				}

				OpcUa_TcpListener_ConnectionManager_Delete(&(pTcpListener->ConnectionManager));

				if(pTcpListener->PendingMessages != OpcUa_Null)
				{
					OpcUa_List_Delete(&(pTcpListener->PendingMessages));
				}

				OpcUa_Free(*a_pListener);
				*a_pListener = OpcUa_Null;
			}
		}
		else
			uStatus=OpcUa_BadOutOfMemory;
	}
	else
		uStatus=OpcUa_BadInvalidArgument;
	return uStatus;
}

/*============================================================================
 * OpcUa_TcpListener_GetReceiveBufferSize
 *===========================================================================*/
OpcUa_StatusCode OpcUa_TcpListener_GetReceiveBufferSize(OpcUa_Listener*     a_pListener,
														OpcUa_Handle        a_hConnection,
														OpcUa_UInt32*       a_pBufferSize)
{
	OpcUa_TcpListener_Connection* pTcpListenerConnection    = OpcUa_Null;
	OpcUa_TcpListener*            pTcpListener              = (OpcUa_TcpListener*)a_pListener->Handle;

OpcUa_InitializeStatus(OpcUa_Module_TcpListener, "GetReceiveBufferSize");

	OpcUa_ReturnErrorIfArgumentNull(a_pBufferSize);

	OpcUa_Mutex_Lock(pTcpListener->Mutex);

	uStatus = OpcUa_TcpListener_ConnectionManager_GetConnectionByHandle(    pTcpListener->ConnectionManager,
																			a_hConnection,
																		   &pTcpListenerConnection);
	if(OpcUa_IsGood(uStatus))
	{
		*a_pBufferSize = pTcpListenerConnection->ReceiveBufferSize;
	}
	else
	{
		*a_pBufferSize = 0;
	}

	OpcUa_Mutex_Unlock(pTcpListener->Mutex);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_TcpListener_GetPeerInfo
 *===========================================================================*/
OpcUa_StatusCode OpcUa_TcpListener_GetPeerInfo(OpcUa_Listener*     a_pListener,
											   OpcUa_Handle        a_hConnection,
											   OpcUa_String*       a_psPeerInfo)
{
	OpcUa_TcpListener_Connection*   pTcpListenerConnection  = OpcUa_Null;
	OpcUa_TcpListener*              pTcpListener            = (OpcUa_TcpListener*)a_pListener->Handle;
#if OPCUA_P_SOCKETGETPEERINFO_V2 == OPCUA_CONFIG_NO
	OpcUa_CharA* buff = OpcUa_Null;
#endif
OpcUa_InitializeStatus(OpcUa_Module_TcpListener, "GetPeerInfo");

	OpcUa_ReferenceParameter(a_pListener);
	OpcUa_ReturnErrorIfArgumentNull(a_psPeerInfo);

	OpcUa_Mutex_Lock(pTcpListener->Mutex);
	uStatus = OpcUa_TcpListener_ConnectionManager_GetConnectionByHandle(    pTcpListener->ConnectionManager,
																			a_hConnection,
																		   &pTcpListenerConnection);

	OpcUa_Mutex_Unlock(pTcpListener->Mutex);

	OpcUa_GotoErrorIfBad(uStatus);

	OpcUa_Mutex_Lock(pTcpListenerConnection->Mutex);
#if	OPCUA_P_SOCKETGETPEERINFO_V2
	OpcUa_String_AttachCopy(a_psPeerInfo, pTcpListenerConnection->achPeerInfo);
#else
	buff = OpcUa_Alloc(50);
	OpcUa_MemSet(buff,0, 50);
	OpcUa_SPrintfA(buff, "%d.%d.%d.%d:%d",
		(OpcUa_Int)(pTcpListenerConnection->PeerIp >> 24) & 0xFF,
		(OpcUa_Int)(pTcpListenerConnection->PeerIp >> 16) & 0xFF,
		(OpcUa_Int)(pTcpListenerConnection->PeerIp >> 8) & 0xFF,
		(OpcUa_Int)pTcpListenerConnection->PeerIp & 0xFF,
		pTcpListenerConnection->PeerPort);
	OpcUa_String_AttachCopy(a_psPeerInfo, buff);
	OpcUa_Free(buff);
#endif

	OpcUa_GotoErrorIfBad(uStatus);

	OpcUa_Mutex_Unlock(pTcpListenerConnection->Mutex);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}
/*============================================================================
 * OpcUa_TcpListener_AddToSendQueue
 *===========================================================================*/
OpcUa_StatusCode OpcUa_TcpListener_AddToSendQueue(OpcUa_Listener*          a_pListener,
												  OpcUa_Handle             a_hConnection,
												  OpcUa_BufferListElement* a_pBufferList,
												  OpcUa_UInt32             a_uFlags)
{
	OpcUa_TcpListener_Connection*   pTcpListenerConnection  = (OpcUa_TcpListener_Connection*)a_hConnection;
	OpcUa_TcpListener*              pTcpListener            = (OpcUa_TcpListener*)a_pListener->Handle;

OpcUa_InitializeStatus(OpcUa_Module_TcpListener, "AddToSendQueue");

	OpcUa_ReturnErrorIfArgumentNull(a_hConnection);
	OpcUa_ReferenceParameter(a_pListener);

	OpcUa_Mutex_Lock(pTcpListener->Mutex);
	uStatus = OpcUa_TcpListener_ConnectionManager_GetConnectionByHandle(    pTcpListener->ConnectionManager,
																			a_hConnection,
																		   &pTcpListenerConnection);

	OpcUa_Mutex_Unlock(pTcpListener->Mutex);

	OpcUa_GotoErrorIfTrue((pTcpListenerConnection == OpcUa_Null), OpcUa_BadInvalidArgument);

	OpcUa_Mutex_Lock(pTcpListenerConnection->Mutex);

	if(pTcpListenerConnection->pSendQueue == OpcUa_Null)
	{
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_TcpListener_AddToSendQueue: Set buffer list of connection 0x%08X to 0x%08X.\n", pTcpListenerConnection, a_pBufferList);
		pTcpListenerConnection->pSendQueue = a_pBufferList;
	}
	else
	{
		OpcUa_BufferListElement* pLastEntry = pTcpListenerConnection->pSendQueue;
		while(pLastEntry->pNext != OpcUa_Null)
		{
			pLastEntry = pLastEntry->pNext;
		}

		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_TcpListener_AddToSendQueue: Append buffer list 0x%08X to connection 0x%08X.\n", a_pBufferList, pTcpListenerConnection);

		pLastEntry->pNext = a_pBufferList;
	}

	if(a_uFlags & OPCUA_LISTENER_CLOSE_WHEN_DONE)
	{
		pTcpListenerConnection->bCloseWhenDone = OpcUa_True;
	}

	if(a_uFlags & OPCUA_LISTENER_NO_RCV_UNTIL_DONE)
	{
		pTcpListenerConnection->bNoRcvUntilDone = OpcUa_True;
	}

	OpcUa_Mutex_Unlock(pTcpListenerConnection->Mutex);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}
/*============================================================================
 * OpcUa_TcpListener_CloseConnection
 *===========================================================================*/
/** @brief Close a particular connection of this listener. */
OpcUa_StatusCode OpcUa_TcpListener_CloseConnection( OpcUa_Listener*     a_pListener,
													OpcUa_Handle        a_hConnection,
													OpcUa_StatusCode    a_uStatus)
{
	OpcUa_TcpListener_Connection*   pTcpListenerConnection  = OpcUa_Null;
	OpcUa_TcpListener*              pTcpListener            = (OpcUa_TcpListener*)a_pListener->Handle;

OpcUa_InitializeStatus(OpcUa_Module_TcpListener, "CloseConnection");

	OpcUa_ReturnErrorIfArgumentNull(a_hConnection);

	OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_TcpListener_CloseConnection: Connection %p is being closed with status 0x%08X\n", a_hConnection, a_uStatus);

	OpcUa_Mutex_Lock(pTcpListener->Mutex);
	uStatus = OpcUa_TcpListener_ConnectionManager_GetConnectionByHandle(    pTcpListener->ConnectionManager,
																			a_hConnection,
																		   &pTcpListenerConnection);
	if(     OpcUa_IsBad(uStatus)
		||  (pTcpListenerConnection == OpcUa_Null))
	{
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_WARNING, "OpcUa_TcpListener_CloseConnection: Connection %p not found.\n", a_hConnection);
		OpcUa_GotoErrorWithStatus(OpcUa_BadInvalidArgument);
	}

	OpcUa_Mutex_Lock(pTcpListenerConnection->Mutex);

	if(pTcpListenerConnection->pSendQueue != OpcUa_Null)
	{
		/* the connection is currently in use by another thread - delay deletion */
		pTcpListenerConnection->bDelete = OpcUa_True;
		pTcpListenerConnection->bCloseWhenDone = OpcUa_True;
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_TcpListener_CloseConnection: Delaying deletion. Connection is in use.\n");
		OpcUa_Mutex_Unlock(pTcpListenerConnection->Mutex);
	}
	else
	{
		pTcpListenerConnection->bDelete = OpcUa_True;
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_TcpListener_CloseConnection: Closing socket.\n");
		pTcpListenerConnection->bConnected = OpcUa_False;
		OPCUA_P_SOCKET_CLOSE(pTcpListenerConnection->Socket);
		pTcpListenerConnection->Socket = OpcUa_Null;
		OpcUa_Mutex_Unlock(pTcpListenerConnection->Mutex);
		OpcUa_TcpListener_ConnectionManager_DeleteConnection(pTcpListener->ConnectionManager, &pTcpListenerConnection);
	}

	OpcUa_Mutex_Unlock(pTcpListener->Mutex);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	OpcUa_Mutex_Unlock(pTcpListener->Mutex);

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_TcpListener_ConnectionDisconnectCB
 *===========================================================================*/
/** @brief Gets called by an outstream if the connection is lost. */
static OpcUa_Void OpcUa_TcpListener_ConnectionDisconnectCB(OpcUa_Handle a_hConnection)
{
	OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_TcpListener_ConnectionDisconnectCB: Connection with id 0x%p is being reported as disconnected!\n", a_hConnection);
}

/*============================================================================
 * OpcUa_TcpListener_BeginSendResponse
 *===========================================================================*/
/* prepare a response (out) stream for a certain connection and related to   */
/* a certain request (in) stream                                             */

OpcUa_StatusCode OpcUa_TcpListener_BeginSendResponse(
	OpcUa_Listener*                 a_pListener,            
	OpcUa_Handle                    a_hConnection,
	OpcUa_InputStream**             a_ppTransportIStrm,
	OpcUa_OutputStream**            a_ppOstrm)
{
	OpcUa_TcpListener*              pTcpListener            = OpcUa_Null;
	OpcUa_TcpListener_Connection*   pTcpListenerConnection  = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_TcpListener, "BeginSendResponse");

	OpcUa_ReturnErrorIfArgumentNull(a_pListener);
	OpcUa_ReturnErrorIfArgumentNull(a_hConnection);
	OpcUa_ReturnErrorIfArgumentNull(a_ppTransportIStrm);
	OpcUa_ReturnErrorIfArgumentNull(a_ppOstrm);

	OpcUa_ReturnErrorIfArgumentNull(a_pListener->BeginSendResponse);

	/* initialize outparameter */
	*a_ppOstrm = OpcUa_Null;
	
	pTcpListener = (OpcUa_TcpListener*)a_pListener->Handle;

	OpcUa_Mutex_Lock(pTcpListener->Mutex);
	uStatus = OpcUa_TcpListener_ConnectionManager_GetConnectionByHandle(    pTcpListener->ConnectionManager,
																			a_hConnection,
																		   &pTcpListenerConnection);

	OpcUa_Mutex_Unlock(pTcpListener->Mutex);

	OpcUa_GotoErrorIfBad(uStatus);

	OpcUa_Mutex_Lock(pTcpListenerConnection->Mutex);

	if(pTcpListenerConnection->SendBufferSize > (OpcUa_UInt32)OpcUa_ProxyStub_g_Configuration.iTcpListener_DefaultChunkSize)
	{
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_TcpListener_BeginSendResponse: Stored send buffer size of %u is too large in connection %p. Cancelling!\n", pTcpListenerConnection->SendBufferSize, pTcpListenerConnection);
		OpcUa_Mutex_Unlock(pTcpListenerConnection->Mutex);
		OpcUa_GotoErrorWithStatus(OpcUa_BadInternalError);
	}

	if((pTcpListenerConnection->Socket == OpcUa_Null) || (pTcpListenerConnection->bConnected == OpcUa_False))
	{
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_WARNING, "OpcUa_TcpListener_BeginSendResponse: Connection %p is not connected!\n", pTcpListenerConnection);
		OpcUa_Mutex_Unlock(pTcpListenerConnection->Mutex);
		OpcUa_GotoErrorWithStatus(OpcUa_BadDisconnect);
	}

	/* create buffer for writing */
	uStatus = OpcUa_TcpStream_CreateOutput( pTcpListenerConnection->Socket,            /* create stream on that socket */
											OpcUa_TcpStream_MessageType_SecureChannel, /* initialize as chunk */ 
											OpcUa_Null,                                /* no buffer to attach */
											pTcpListenerConnection->SendBufferSize,    /* flush border size */
											OpcUa_TcpListener_ConnectionDisconnectCB,  /* function to call, if stream detects disconnect */
											pTcpListenerConnection->MaxChunkCount,     /* maximum number of chunks allowed */ 
											a_ppOstrm);                                /* use that handle */

	if(OpcUa_IsBad(uStatus))
	{
		OpcUa_Mutex_Unlock(pTcpListenerConnection->Mutex);
		OpcUa_GotoError;
	}

	/* close and delete the incoming stream - double close is ignored (uncritical) */
	(*a_ppTransportIStrm)->Close((OpcUa_Stream*)(*a_ppTransportIStrm));
	(*a_ppTransportIStrm)->Delete((OpcUa_Stream**)a_ppTransportIStrm);

	((OpcUa_TcpOutputStream*)((*a_ppOstrm)->Handle))->hConnection = a_hConnection;

	OpcUa_Mutex_Unlock(pTcpListenerConnection->Mutex);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	if(*a_ppOstrm != OpcUa_Null)
	{
		(*a_ppOstrm)->Delete((OpcUa_Stream**)a_ppOstrm);
	}

OpcUa_FinishErrorHandling;
}
/*============================================================================
 * OpcUa_TcpListener_AddStreamToSendQueue
 *===========================================================================*/
OpcUa_StatusCode OpcUa_TcpListener_AddStreamToSendQueue(
	OpcUa_Listener*                 a_pListener,
	OpcUa_TcpListener_Connection*   a_pListenerConnection,
	OpcUa_OutputStream*             a_pOutputStream)
{
	OpcUa_Buffer             tempBuffer;
	OpcUa_BufferListElement* pEntry = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_TcpListener, "OpcUa_TcpListener_AddStreamToSendQueue");

	OpcUa_MemSet(&tempBuffer, 0, sizeof(OpcUa_Buffer));

	pEntry = (OpcUa_BufferListElement*)OpcUa_Alloc(sizeof(OpcUa_BufferListElement));
	OpcUa_ReturnErrorIfAllocFailed(pEntry);
	OpcUa_MemSet(pEntry, 0, sizeof(OpcUa_BufferListElement));

	/* try to detach buffer from stream into temporary buffer */
	uStatus = a_pOutputStream->DetachBuffer((OpcUa_Stream*)a_pOutputStream, &tempBuffer, OpcUa_Null);
	OpcUa_GotoErrorIfBad(uStatus);

	pEntry->Buffer              = tempBuffer;
	pEntry->Buffer.FreeBuffer   = OpcUa_True;
	pEntry->pNext               = OpcUa_Null;

	pEntry->Buffer.Data         = (OpcUa_Byte*)OpcUa_Alloc(pEntry->Buffer.Size);
	OpcUa_GotoErrorIfAllocFailed(pEntry->Buffer.Data);

	/* copy buffer content from detached buffer to new buffer */
	OpcUa_MemCpy(   pEntry->Buffer.Data,
					pEntry->Buffer.EndOfData,
					tempBuffer.Data,
					tempBuffer.EndOfData);

	uStatus = OpcUa_TcpListener_AddToSendQueue( a_pListener,
												OPCUA_TCPLISTENER_CONNECTIONMANAGER_HANDLE_GET(a_pListenerConnection),
												pEntry,
												0);
	OpcUa_GotoErrorIfBad(uStatus);

	/* clear temporary buffer */
	OpcUa_Buffer_Clear(&tempBuffer);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	OpcUa_Buffer_Clear(&tempBuffer);

	if(pEntry != OpcUa_Null)
	{
		OpcUa_Buffer_Clear(&pEntry->Buffer);
		OpcUa_Free(pEntry);
	}

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * Send an Error message.
 *===========================================================================*/
static OpcUa_StatusCode OpcUa_TcpListener_SendErrorMessage( OpcUa_Listener*                 a_pListener, 
															OpcUa_TcpListener_Connection*   a_pTcpConnection,
															OpcUa_StatusCode                a_uStatus,
															OpcUa_String*                   a_sReason)
{
	OpcUa_TcpListener*      pTcpListener        = OpcUa_Null;
	OpcUa_OutputStream*     pOutputStream       = OpcUa_Null;
	OpcUa_String            sReason             = OPCUA_STRING_STATICINITIALIZER;

OpcUa_InitializeStatus(OpcUa_Module_TcpListener, "SendErrorMessage");

	OpcUa_GotoErrorIfArgumentNull(a_pListener);
	OpcUa_GotoErrorIfArgumentNull(a_pTcpConnection);

	pTcpListener = (OpcUa_TcpListener*)a_pListener;

	if((pTcpListener->bShutdown != OpcUa_False) && (a_pTcpConnection->bConnected != OpcUa_False))
	{
		OpcUa_ReturnStatusCode;
	}
#if	OPCUA_P_SOCKETGETPEERINFO_V2
	OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR,
				"OpcUa_TcpListener_SendErrorMessage: to %s (socket %X) with StatusCode 0x%08X\n",
				a_pTcpConnection->achPeerInfo,
				a_pTcpConnection->Socket,
				a_uStatus);
#else
	OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR,
		"OpcUa_TcpListener_SendErrorMessage: to %d.%d.%d.%d:%d (socket %X) with StatusCode 0x%08X\n",
		(OpcUa_Int)(a_pTcpConnection->PeerIp >> 24) & 0xFF,
		(OpcUa_Int)(a_pTcpConnection->PeerIp >> 16) & 0xFF,
		(OpcUa_Int)(a_pTcpConnection->PeerIp >> 8) & 0xFF,
		(OpcUa_Int)a_pTcpConnection->PeerIp & 0xFF,
		a_pTcpConnection->PeerPort, a_pTcpConnection->Socket,a_uStatus);
#endif
	/* create the output stream for the errormessage */
	uStatus = OpcUa_TcpStream_CreateOutput( a_pTcpConnection->Socket,
											OpcUa_TcpStream_MessageType_Error,
											OpcUa_Null,
											pTcpListener->DefaultChunkSize,
											OpcUa_TcpListener_ConnectionDisconnectCB,
											a_pTcpConnection->MaxChunkCount,
											&pOutputStream);
	OpcUa_GotoErrorIfBad(uStatus);

	/* encode the body of an Error message */

	/* status code */
	uStatus = OpcUa_UInt32_BinaryEncode(a_uStatus, pOutputStream);
	OpcUa_GotoErrorIfBad(uStatus);

	uStatus = OpcUa_String_BinaryEncode(a_sReason?a_sReason:&sReason, pOutputStream);
	OpcUa_GotoErrorIfBad(uStatus);

	if(a_pTcpConnection->pSendQueue == OpcUa_Null)
	{
		uStatus = pOutputStream->Flush(pOutputStream, OpcUa_True);
	}

	if(a_pTcpConnection->pSendQueue != OpcUa_Null || OpcUa_IsEqual(OpcUa_BadWouldBlock))
	{
		uStatus = OpcUa_TcpListener_AddStreamToSendQueue(a_pListener, a_pTcpConnection, pOutputStream);
	}
	OpcUa_GotoErrorIfBad(uStatus);

	/* finish stream and delete it */
	uStatus = pOutputStream->Close((OpcUa_Stream*)pOutputStream);
	OpcUa_GotoErrorIfBad(uStatus);
	pOutputStream->Delete((OpcUa_Stream**)&pOutputStream);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	if((pOutputStream != OpcUa_Null) && (pOutputStream->Delete != OpcUa_Null))
	{
		pOutputStream->Delete((OpcUa_Stream**)&pOutputStream);
	}

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_TcpListener_EndSendResponse
 *===========================================================================*/
/* a bad status means, that the operation is to be abandoned */
OpcUa_StatusCode OpcUa_TcpListener_EndSendResponse(
	struct _OpcUa_Listener* a_pListener,
	OpcUa_StatusCode        a_uStatus,
	OpcUa_OutputStream**    a_ppOstrm)
{
	OpcUa_TcpOutputStream*          pTcpOutputStream        = OpcUa_Null;
	OpcUa_TcpListener_Connection*   pTcpListenerConnection  = OpcUa_Null;
	OpcUa_TcpListener*              pTcpListener            = (OpcUa_TcpListener*)a_pListener->Handle;

OpcUa_InitializeStatus(OpcUa_Module_TcpListener, "OpcUa_TcpListener_EndSendResponse");

	OpcUa_ReturnErrorIfArgumentNull(a_pListener);
	OpcUa_ReturnErrorIfArgumentNull(a_ppOstrm);
	OpcUa_ReturnErrorIfArgumentNull(*a_ppOstrm);

	OpcUa_ReturnErrorIfArgumentNull(a_pListener->EndSendResponse);
	
	pTcpOutputStream = (OpcUa_TcpOutputStream*)(*a_ppOstrm)->Handle;

	OpcUa_Mutex_Lock(pTcpListener->Mutex);
	uStatus = OpcUa_TcpListener_ConnectionManager_GetConnectionByHandle(    pTcpListener->ConnectionManager,
																			pTcpOutputStream->hConnection,
																		   &pTcpListenerConnection);

	OpcUa_Mutex_Unlock(pTcpListener->Mutex);

	OpcUa_GotoErrorIfBad(uStatus);

	OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_TcpListener_EndSendResponse: Status 0x%08X, Stream %p, Connection %p\n", a_uStatus, *a_ppOstrm, pTcpListenerConnection);

	/* lock connection and close the socket. */
	OpcUa_Mutex_Lock(pTcpListener->Mutex);
	if(pTcpListener->bShutdown != OpcUa_False)
	{
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_WARNING, "OpcUa_TcpListener_EndSendResponse: Listener %p is shutting down. Aborting response!\n", pTcpListener);
		OpcUa_TcpStream_Delete((OpcUa_Stream**)a_ppOstrm);
		OpcUa_Mutex_Unlock(pTcpListener->Mutex);
		OpcUa_GotoErrorWithStatus(OpcUa_Good);
	}
	OpcUa_Mutex_Unlock(pTcpListener->Mutex);

	OpcUa_Mutex_Lock(pTcpListenerConnection->Mutex);

	if(pTcpListenerConnection->bConnected != OpcUa_False)
	{
		OpcUa_Mutex_Unlock(pTcpListenerConnection->Mutex);

		/* trigger error message */
		if(OpcUa_IsBad(a_uStatus) && (a_uStatus != OpcUa_BadDisconnect))
		{
			/* create and send UAME */
			OpcUa_TcpListener_SendErrorMessage( a_pListener,
												pTcpListenerConnection,
												a_uStatus,
												OpcUa_Null);

			pTcpListenerConnection->bValid = OpcUa_False;

			OpcUa_Mutex_Lock(pTcpListenerConnection->Mutex);
			OPCUA_P_SOCKET_CLOSE(pTcpListenerConnection->Socket);
			pTcpListenerConnection->Socket = OpcUa_Null;
			OpcUa_Mutex_Unlock(pTcpListenerConnection->Mutex);

			OpcUa_TcpListener_ConnectionManager_DeleteConnection(pTcpListener->ConnectionManager, &pTcpListenerConnection);
		}
		else
		{
			/* close stream (flushes the Content on the wire). */
			uStatus = (*a_ppOstrm)->Close((OpcUa_Stream*)*a_ppOstrm);
		}
	}
	else
	{
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_TcpListener_EndSendResponse: Client connection %p with socket %p already disconnected!\n", pTcpListenerConnection, pTcpListenerConnection->Socket);
		if(pTcpListenerConnection->bDelete != OpcUa_False)
		{
			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_TcpListener_EndSendResponse: Finalize deletion.\n");
			uStatus = OpcUa_Good;
			OPCUA_P_SOCKET_CLOSE(pTcpListenerConnection->Socket);
			pTcpListenerConnection->Socket = OpcUa_Null;
			OpcUa_Mutex_Unlock(pTcpListenerConnection->Mutex);
			OpcUa_TcpListener_ConnectionManager_DeleteConnection(pTcpListener->ConnectionManager, &pTcpListenerConnection);
			OpcUa_ReturnStatusCode;
		}
		else
		{
			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_TcpListener_EndSendResponse: Ignore deletion.\n");
			OpcUa_Mutex_Unlock(pTcpListenerConnection->Mutex);
		}
	}    

	/* delete stream */
	OpcUa_TcpStream_Delete((OpcUa_Stream**)a_ppOstrm);
		
OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_TcpListener_AbortSendResponse
 *===========================================================================*/
/* a bad status means, that the operation is to be abandoned */
OpcUa_StatusCode OpcUa_TcpListener_AbortSendResponse(
	struct _OpcUa_Listener* a_pListener,
	OpcUa_StatusCode        a_uStatus,
	OpcUa_String*           a_psReason,
	OpcUa_OutputStream**    a_ppOutputStream)
{
OpcUa_InitializeStatus(OpcUa_Module_TcpListener, "OpcUa_TcpListener_AbortSendResponse");

	OpcUa_ReturnErrorIfArgumentNull(a_pListener);
	OpcUa_ReturnErrorIfArgumentNull(a_pListener->AbortSendResponse);

	if((a_ppOutputStream != OpcUa_Null) && (*a_ppOutputStream != OpcUa_Null))
	{
		OpcUa_TcpOutputStream*          pTcpOutputStream        = OpcUa_Null;
		OpcUa_TcpListener*              pTcpListener            = OpcUa_Null;
		OpcUa_TcpListener_Connection*   pTcpListenerConnection  = OpcUa_Null;

		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_TcpListener_AbortSendResponse: called for stream %p\n", *a_ppOutputStream);

		pTcpOutputStream = (OpcUa_TcpOutputStream*)((*a_ppOutputStream)->Handle);
		OpcUa_GotoErrorIfArgumentNull(pTcpOutputStream);
		pTcpListener     = (OpcUa_TcpListener*)a_pListener->Handle;

		OpcUa_Mutex_Lock(pTcpListener->Mutex);
		uStatus = OpcUa_TcpListener_ConnectionManager_GetConnectionByHandle(    pTcpListener->ConnectionManager,
																				pTcpOutputStream->hConnection,
																			   &pTcpListenerConnection);

		OpcUa_Mutex_Unlock(pTcpListener->Mutex);

		OpcUa_GotoErrorIfBad(uStatus);

		OpcUa_Mutex_Lock(pTcpListenerConnection->Mutex);
		if(pTcpListenerConnection->bConnected != OpcUa_False)
		{
			OpcUa_Mutex_Unlock(pTcpListenerConnection->Mutex);
		}
		else
		{
			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_TcpListener_AbortSendResponse: Client connection %p with socket %p marked as disconnected! Finalize deletion.\n", pTcpListenerConnection, pTcpListenerConnection->Socket);
			OPCUA_P_SOCKET_CLOSE(pTcpListenerConnection->Socket);
			pTcpListenerConnection->Socket = OpcUa_Null;
			OpcUa_Mutex_Unlock(pTcpListenerConnection->Mutex);
			OpcUa_TcpListener_ConnectionManager_DeleteConnection(pTcpListener->ConnectionManager, &pTcpListenerConnection);
		}

		/* clean up */
		OpcUa_TcpStream_Delete((OpcUa_Stream**)a_ppOutputStream);
	}
	else
	{
		/* no insecure abort messages implemented and allowed! */
		OpcUa_ReferenceParameter(a_uStatus);
		OpcUa_ReferenceParameter(a_psReason);
	}
		
OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_TcpListener_LookForPendingMessage
 *===========================================================================*/
/* should be handled by the connection manager, since no interleaving is possible by design! */
OpcUa_StatusCode OpcUa_TcpListener_LookForPendingMessage(   OpcUa_TcpListener*  a_pTcpListener, 
															OpcUa_Socket        a_pSocket, 
															OpcUa_InputStream** a_pInputStream)
{
	OpcUa_InputStream*      pInputStream    = OpcUa_Null;
	OpcUa_TcpInputStream*   pTcpInputStream = OpcUa_Null;

	OpcUa_DeclareErrorTraceModule(OpcUa_Module_TcpListener);

	OpcUa_ReturnErrorIfArgumentNull(a_pTcpListener);
	OpcUa_ReturnErrorIfArgumentNull(a_pSocket);
	OpcUa_ReturnErrorIfArgumentNull(a_pInputStream);

	OpcUa_List_Enter(a_pTcpListener->PendingMessages);

	*a_pInputStream = OpcUa_Null;

	OpcUa_List_ResetCurrent(a_pTcpListener->PendingMessages);
	pInputStream = (OpcUa_InputStream*)OpcUa_List_GetCurrentElement(a_pTcpListener->PendingMessages);

	while(pInputStream != OpcUa_Null)
	{
		pTcpInputStream = (OpcUa_TcpInputStream *)pInputStream->Handle;

		if(pTcpInputStream != OpcUa_Null && pTcpInputStream->Socket == a_pSocket)
		{
			/* found */
			OpcUa_List_DeleteElement(a_pTcpListener->PendingMessages, (OpcUa_Void*)pInputStream);
			*a_pInputStream = pInputStream;
			OpcUa_List_Leave(a_pTcpListener->PendingMessages);
			return OpcUa_Good;
		}
		else
		{
			/* get next element */
			pInputStream = (OpcUa_InputStream*)OpcUa_List_GetNextElement(a_pTcpListener->PendingMessages);
		}
	}

	OpcUa_List_Leave(a_pTcpListener->PendingMessages);

	return OpcUa_BadNotFound;
}

/*============================================================================
 * OpcUa_TcpListener_ProcessRequest
 *===========================================================================*/
/**
* @brief Handles a UATM (Request), UATC (Request Chunk) message.
*/
OpcUa_StatusCode OpcUa_TcpListener_ProcessRequest(
	OpcUa_Listener*                 a_pListener, 
	OpcUa_TcpListener_Connection*   a_pTcpConnection,
	OpcUa_InputStream**             a_ppInputStream)
{
	OpcUa_TcpListener*      pTcpListener    = OpcUa_Null;
	OpcUa_TcpInputStream*   pTcpInputStream = OpcUa_Null;
	OpcUa_ListenerEvent     eEvent          = OpcUa_ListenerEvent_Invalid;

OpcUa_InitializeStatus(OpcUa_Module_TcpListener, "ProcessRequest");

	OpcUa_ReturnErrorIfArgumentNull(a_pListener);
	OpcUa_ReturnErrorIfArgumentNull(a_ppInputStream);
	OpcUa_ReturnErrorIfArgumentNull(*a_ppInputStream);

	pTcpListener     = (OpcUa_TcpListener*)a_pListener->Handle;
	pTcpInputStream  = (OpcUa_TcpInputStream*)(*a_ppInputStream)->Handle;

	if(pTcpInputStream->IsAbort != OpcUa_False)
	{
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_TcpListener_ProcessRequest: Message aborted after %u received chunks while %u are allowed!\n", a_pTcpConnection->uCurrentChunk, a_pTcpConnection->MaxChunkCount);
		eEvent = OpcUa_ListenerEvent_RequestAbort;
		a_pTcpConnection->uCurrentChunk = 0;
	}
	else
	{
		if(pTcpInputStream->IsFinal != OpcUa_False)
		{
			/* last chunk in message, reset counter */
			eEvent = OpcUa_ListenerEvent_Request;
			a_pTcpConnection->uCurrentChunk = 0;
		}
		else
		{
			/* intermediary chunk, test for limit and increment */
			a_pTcpConnection->uCurrentChunk++;
			eEvent = OpcUa_ListenerEvent_RequestPartial;
			if((a_pTcpConnection->MaxChunkCount != 0) && (a_pTcpConnection->uCurrentChunk >= a_pTcpConnection->MaxChunkCount))
			{
				/* this message will be too large */
				OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_WARNING, "OpcUa_TcpListener_ProcessRequest: Chunk count limit exceeded!\n");
				eEvent = OpcUa_ListenerEvent_Request;  /* message final */
				uStatus = OpcUa_BadTcpMessageTooLarge; /* with error */
			}
		}
	}

	if(OpcUa_IsGood(uStatus))
	{
		/* send notification that request is ready to be read. */
		/* this call goes most probably to the secure channel handler. */
		if(pTcpListener->Callback != OpcUa_Null)
		{
			a_pTcpConnection->uNoOfRequestsTotal++;

			/* the securechannel needs all data, including headers */
			pTcpInputStream->Buffer.Position = 0;

			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_TcpListener_ProcessRequest: Handing TcpInputStream %p to upper layer!\n", pTcpInputStream);

			uStatus = pTcpListener->Callback(
				a_pListener,                            /* the event source          */
				(OpcUa_Void*)pTcpListener->CallbackData,/* the callback data         */
				eEvent,                                 /* the event type            */
				OPCUA_TCPLISTENER_CONNECTIONMANAGER_HANDLE_GET(a_pTcpConnection), /* handle for the connection */
				a_ppInputStream,                        /* the ready input stream    */
				OpcUa_Good);                            /* event status code         */
		}
		else
		{
			/* delete and close input stream */
			OpcUa_TcpStream_Close((OpcUa_Stream*)(*a_ppInputStream));
			OpcUa_TcpStream_Delete((OpcUa_Stream**)a_ppInputStream);
		}
	}
	else
	{
		/* an error occured - inform the owner of this listener */

		/* delete and close input stream immediately */
		OpcUa_TcpStream_Close((OpcUa_Stream*)(*a_ppInputStream));
		OpcUa_TcpStream_Delete((OpcUa_Stream**)a_ppInputStream);

		if(pTcpListener->Callback != OpcUa_Null)
		{
			a_pTcpConnection->uNoOfRequestsTotal++;

			uStatus = pTcpListener->Callback(
				a_pListener,                            /* the event source          */
				(OpcUa_Void*)pTcpListener->CallbackData,/* the callback data         */
				eEvent,
				OPCUA_TCPLISTENER_CONNECTIONMANAGER_HANDLE_GET(a_pTcpConnection), /* handle for the connection */
				OpcUa_Null,                             /* the ready input stream    */
				uStatus);                               /* event status code         */
			
			/* done */
			uStatus = OpcUa_Good;
		}
	}

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
} /* OpcUa_TcpListener_ProcessRequest */

/*============================================================================
 * OpcUa_TcpListener_SendAcknowledgeMessage
 *===========================================================================*/
/**
* @brief Handles the response to a hello message.
*/
OpcUa_StatusCode OpcUa_TcpListener_SendAcknowledgeMessage(
	OpcUa_Listener*                 a_pListener, 
	OpcUa_TcpListener_Connection*   a_pTcpConnection)
{
	OpcUa_OutputStream*     pOutputStream    = OpcUa_Null;
	OpcUa_String            altEndpoint      = OPCUA_STRING_STATICINITIALIZER;

OpcUa_InitializeStatus(OpcUa_Module_TcpListener, "SendAcknowledgeMessage");

	OpcUa_ReturnErrorIfArgumentNull(a_pListener);
	OpcUa_ReturnErrorIfArgumentNull(a_pListener->Handle);
	OpcUa_ReturnErrorIfArgumentNull(a_pTcpConnection);

	OpcUa_String_Initialize(&altEndpoint);

	uStatus = OpcUa_TcpStream_CreateOutput( a_pTcpConnection->Socket,
											OpcUa_TcpStream_MessageType_Acknowledge,
											OpcUa_Null,
											a_pTcpConnection->SendBufferSize,
											OpcUa_TcpListener_ConnectionDisconnectCB,
											a_pTcpConnection->MaxChunkCount,
											&pOutputStream);
	OpcUa_GotoErrorIfBad(uStatus);

	/* encode acknowledge fields */

	/* revised protocol version */
	uStatus = OpcUa_UInt32_BinaryEncode((a_pTcpConnection->uProtocolVersion), pOutputStream);
	OpcUa_GotoErrorIfBad(uStatus);

	/* revised receivebuffer */
	uStatus = OpcUa_UInt32_BinaryEncode((a_pTcpConnection->ReceiveBufferSize), pOutputStream);
	OpcUa_GotoErrorIfBad(uStatus);

	/* revised sendbuffer */
	uStatus = OpcUa_UInt32_BinaryEncode((a_pTcpConnection->SendBufferSize), pOutputStream);
	OpcUa_GotoErrorIfBad(uStatus);
 
	/* send max message size */
	uStatus = OpcUa_UInt32_BinaryEncode((a_pTcpConnection->MaxMessageSize), pOutputStream);
	OpcUa_GotoErrorIfBad(uStatus);

	/* send max chunk count */
	uStatus = OpcUa_UInt32_BinaryEncode((a_pTcpConnection->MaxChunkCount), pOutputStream);
	OpcUa_GotoErrorIfBad(uStatus);

	uStatus = pOutputStream->Flush(pOutputStream, OpcUa_True);
	if(OpcUa_IsEqual(OpcUa_BadWouldBlock))
	{
		uStatus = OpcUa_TcpListener_AddStreamToSendQueue(a_pListener, a_pTcpConnection, pOutputStream);
	}
	OpcUa_GotoErrorIfBad(uStatus);

	uStatus = pOutputStream->Close((OpcUa_Stream*)pOutputStream);
	OpcUa_GotoErrorIfBad(uStatus);

	pOutputStream->Delete((OpcUa_Stream**)&pOutputStream);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	if(pOutputStream != OpcUa_Null)
	{
		pOutputStream->Delete((OpcUa_Stream**)&pOutputStream);
	}

OpcUa_FinishErrorHandling;
}


/*============================================================================
 * OpcUa_TcpListener_ProcessHelloMessage
 *===========================================================================*/
/**
 * @brief Handles a Hello message.
 *
 * @param pListener      The listener that hosts the socket from which the message is being received.
 * @param istrm          The stream containing the UAMH.
 */
OpcUa_StatusCode OpcUa_TcpListener_ProcessHelloMessage(
	OpcUa_Listener*                 a_pListener, 
	OpcUa_InputStream*              a_istrm)
{
	OpcUa_TcpListener*              pTcpListener    = OpcUa_Null;
	OpcUa_TcpInputStream*           pTcpInputStream = OpcUa_Null;
	OpcUa_TcpListener_Connection*   pConnection     = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_TcpListener, "ProcessHelloMessage");

	OpcUa_ReturnErrorIfArgumentNull(a_pListener);
	pTcpListener = (OpcUa_TcpListener*)a_pListener->Handle;
	OpcUa_ReturnErrorIfArgumentNull(pTcpListener);
	OpcUa_ReturnErrorIfArgumentNull(a_istrm);
	pTcpInputStream = (OpcUa_TcpInputStream*)a_istrm->Handle;
	OpcUa_ReturnErrorIfArgumentNull(pTcpInputStream);
 
#if OPCUA_TCPLISTENER_USEEXTRAMAXCONNSOCKET
	{
		OpcUa_UInt32 uConnections = 0;

		uStatus = OpcUa_TcpListener_ConnectionManager_GetConnectionCount(pTcpListener->ConnectionManager,
																		 &uConnections);

		OpcUa_GotoErrorIfTrue(uConnections >= OPCUA_TCPLISTENER_MAXCONNECTIONS, OpcUa_BadMaxConnectionsReached);
	}
#endif /* OPCUA_TCPLISTENER_USEEXTRAMAXCONNSOCKET */

	/* check, if there is already a connection with this object */
	OpcUa_TcpListener_ConnectionManager_GetConnectionBySocket(  pTcpListener->ConnectionManager,
																pTcpInputStream->Socket,
																&pConnection);

	/* if no connection exists create a new one */
	if(pConnection == OpcUa_Null)
	{
		/* create and add a new connection object for the accepted connection  */
		uStatus = OpcUa_TcpListener_ConnectionManager_CreateConnection( pTcpListener->ConnectionManager,
																		&pConnection);
		OpcUa_GotoErrorIfBad(uStatus);
		
		pConnection->Socket          = pTcpInputStream->Socket;
		pConnection->pListenerHandle = (OpcUa_Listener*)a_pListener;

		OpcUa_Mutex_Lock(pTcpListener->Mutex);
		pTcpListener->PendingConnects--;
		OpcUa_Mutex_Unlock(pTcpListener->Mutex);

#if OPCUA_P_SOCKETGETPEERINFO_V2
		uStatus = OPCUA_P_SOCKET_GETPEERINFO(pConnection->Socket, pConnection->achPeerInfo, OPCUA_P_PEERINFO_MIN_SIZE);
		if(OpcUa_IsGood(uStatus))
		{
			/* Give some debug information. */
			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, 
						"OpcUa_TcpListener_ProcessHelloMessage: Transport connection from %s accepted on socket %X!\n", 
						pConnection->achPeerInfo,
						pConnection->Socket);
		}
		else
		{
			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_WARNING, "OpcUa_TcpListener_ProcessHelloMessage: Could not retrieve connection information for socket %X!\n", pConnection->Socket);
		}
#else /* OPCUA_P_SOCKETGETPEERINFO_V2 */
		uStatus = OPCUA_P_SOCKET_GETPEERINFO(pTcpInputStream->Socket, &(pConnection->PeerIp), &(pConnection->PeerPort));
		if(OpcUa_IsGood(uStatus))
		{
			/* Give some debug information. */
			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, 
						"OpcUa_TcpListener_ProcessHelloMessage: %d.%d.%d.%d:%d (socket %X) accepted!\n", 
						(OpcUa_Int)(pConnection->PeerIp>>24)&0xFF, 
						(OpcUa_Int)(pConnection->PeerIp>>16)&0xFF, 
						(OpcUa_Int)(pConnection->PeerIp>>8) &0xFF,
						(OpcUa_Int) pConnection->PeerIp     &0xFF, 
						pConnection->PeerPort, 
						pConnection->Socket);
		}
		else
		{
			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_TcpListener_ProcessHelloMessage: Could not retrieve connection information for socket %X!\n", pConnection->Socket);
		}
#endif /* OPCUA_P_SOCKETGETPEERINFO_V2 */

		pConnection->bValid = OpcUa_True;
		OpcUa_GotoErrorIfBad(uStatus);
	}
	else
	{
		OpcUa_GotoErrorWithStatus(OpcUa_BadUnexpectedError);
	}

	/* protocol version */
	uStatus = OpcUa_UInt32_BinaryDecode(&pConnection->uProtocolVersion, a_istrm);
	OpcUa_GotoErrorIfBad(uStatus);

	/* requested send buffer size (this is the receive buffer in the server) */
	uStatus = OpcUa_UInt32_BinaryDecode(&(pConnection->ReceiveBufferSize), a_istrm);
	OpcUa_GotoErrorIfBad(uStatus);

	/* requested receive buffer size (this is the send buffer in the server) */
	uStatus = OpcUa_UInt32_BinaryDecode(&(pConnection->SendBufferSize), a_istrm);
	OpcUa_GotoErrorIfBad(uStatus);

	/* requested max message size */
	uStatus = OpcUa_UInt32_BinaryDecode(&(pConnection->MaxMessageSize), a_istrm);
	OpcUa_GotoErrorIfBad(uStatus);

	/* requested max chunk count */
	uStatus = OpcUa_UInt32_BinaryDecode(&(pConnection->MaxChunkCount), a_istrm);
	OpcUa_GotoErrorIfBad(uStatus);

	/* requested receive buffer size (this is the send buffer in the server) */
	uStatus = OpcUa_String_BinaryDecode(&(pConnection->sURL), 4096, a_istrm);
	OpcUa_GotoErrorIfBad(uStatus);

	/* parsing finished */
	OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "Requested: PV:%u SB:%u RB:%u MMS:%u MCC:%u\n",    
		pConnection->uProtocolVersion,
		pConnection->SendBufferSize, 
		pConnection->ReceiveBufferSize,
		pConnection->MaxMessageSize,
		pConnection->MaxChunkCount);    

	pConnection->SendBufferSize     = (pConnection->SendBufferSize    > (OpcUa_UInt32)OpcUa_ProxyStub_g_Configuration.iTcpListener_DefaultChunkSize)?(OpcUa_UInt32)OpcUa_ProxyStub_g_Configuration.iTcpListener_DefaultChunkSize:pConnection->SendBufferSize;
	pConnection->SendBufferSize     = (pConnection->SendBufferSize    < 8192)?8192:pConnection->SendBufferSize;
	pConnection->ReceiveBufferSize  = (pConnection->ReceiveBufferSize > (OpcUa_UInt32)OpcUa_ProxyStub_g_Configuration.iTcpListener_DefaultChunkSize)?(OpcUa_UInt32)OpcUa_ProxyStub_g_Configuration.iTcpListener_DefaultChunkSize:pConnection->ReceiveBufferSize;
	pConnection->ReceiveBufferSize  = (pConnection->ReceiveBufferSize < 8192)?8192:pConnection->ReceiveBufferSize;
	pConnection->MaxMessageSize     = (pConnection->MaxMessageSize    > (OpcUa_UInt32)OpcUa_ProxyStub_g_Configuration.iTcpTransport_MaxMessageLength)?(OpcUa_UInt32)OpcUa_ProxyStub_g_Configuration.iTcpTransport_MaxMessageLength:pConnection->MaxMessageSize;
	
	if(     pConnection->MaxChunkCount  == 0 
		||  pConnection->MaxChunkCount  >  (OpcUa_UInt32)OpcUa_ProxyStub_g_Configuration.iTcpTransport_MaxChunkCount)
	{
		pConnection->MaxChunkCount = (OpcUa_UInt32)OpcUa_ProxyStub_g_Configuration.iTcpTransport_MaxChunkCount;
	}

	OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "Set:            SB:%u RB:%u MMS:%u MCC:%u\n", 
		pConnection->SendBufferSize, 
		pConnection->ReceiveBufferSize,
		pConnection->MaxMessageSize,
		pConnection->MaxChunkCount);

	pConnection->bConnected = OpcUa_True;

	/* the request is verified and an acknowledge can be sent to the new client */
	OpcUa_TcpListener_SendAcknowledgeMessage(a_pListener, pConnection);

	pTcpListener->Callback( a_pListener,                        /* the source of the event          */
							pTcpListener->CallbackData,         /* the callback data                */
							OpcUa_ListenerEvent_ChannelOpened,  /* the event that occured           */
							OPCUA_TCPLISTENER_CONNECTIONMANAGER_HANDLE_GET(pConnection), /* the handle for the connection    */
							OpcUa_Null,                         /* the non existing stream          */
							OpcUa_Good);                        /* status                           */

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	if(pConnection != OpcUa_Null)
	{
		/* ignore result; it doesnt matter, if it was not yet registered */
		pConnection->bValid = OpcUa_False;
		OpcUa_TcpListener_ConnectionManager_DeleteConnection(pTcpListener->ConnectionManager, &pConnection);
	}
	else
	{
		OPCUA_P_SOCKET_CLOSE(pTcpInputStream->Socket);
	}

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_TcpListener_EventHandler Type
 *===========================================================================*/
OpcUa_StatusCode OpcUa_TcpListener_ProcessDisconnect(   OpcUa_Listener*                 a_pListener, 
														OpcUa_TcpListener_Connection*   a_pTcpConnection)
{
	OpcUa_TcpListener* pTcpListener = (OpcUa_TcpListener*)a_pListener;

OpcUa_InitializeStatus(OpcUa_Module_TcpListener, "ProcessDisconnect");

	/* must be called with already closed socket! */

	OpcUa_ReturnErrorIfArgumentNull(a_pListener);
	OpcUa_ReturnErrorIfArgumentNull(a_pTcpConnection);

	OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_TcpListener_ProcessDisconnect: Connection %p reported as lost!\n", a_pTcpConnection);

	/* now, that the upper layers are informed, we can safely remove the resources for the broken connection. */
	a_pTcpConnection->bValid = OpcUa_False;

	if(OpcUa_IsBad(uStatus))
	{
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_TcpListener_ProcessDisconnect: Client connection %p already removed!\n", a_pTcpConnection);
		uStatus = OpcUa_Good;
		OpcUa_ReturnStatusCode;
	}

	OpcUa_Mutex_Lock(a_pTcpConnection->Mutex);
	
	if(a_pTcpConnection->bConnected == OpcUa_False)
	{
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_TcpListener_ProcessDisconnect: Client connection %p with socket %p already set to disconnected!\n", a_pTcpConnection, a_pTcpConnection->Socket);
		uStatus = OpcUa_Good;
		OpcUa_Mutex_Unlock(a_pTcpConnection->Mutex);
		OpcUa_ReturnStatusCode;
	}

	a_pTcpConnection->bConnected = OpcUa_False;

	OpcUa_Mutex_Unlock(a_pTcpConnection->Mutex);

	/* notify about successful closing of the listener */
	pTcpListener->Callback( a_pListener,                        /* the source of the event          */
							pTcpListener->CallbackData,         /* the callback data                */
							OpcUa_ListenerEvent_ChannelClosed,  /* the event that occured           */
							OPCUA_TCPLISTENER_CONNECTIONMANAGER_HANDLE_GET(a_pTcpConnection), /* the handle for the connection    */
							OpcUa_Null,                         /* the non existing stream          */
							OpcUa_Good);                        /* status                           */

	OpcUa_TcpListener_ConnectionManager_DeleteConnection(pTcpListener->ConnectionManager, &a_pTcpConnection);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}



/*============================================================================
 * OpcUa_TcpListener_TimeoutEventHandler
 *===========================================================================*/
/**
* @brief Gets called in case of a timeout on the socket.
*/
OpcUa_StatusCode OpcUa_TcpListener_TimeoutEventHandler(OpcUa_Listener* a_pListener, OpcUa_Socket    a_pSocket)
{
	OpcUa_TcpListener*              pTcpListener            = OpcUa_Null;
	OpcUa_TcpListener_Connection*   pTcpListenerConnection  = OpcUa_Null;
	OpcUa_InputStream*              pInputStream            = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_TcpListener, "TimeoutEventHandler");

	OpcUa_ReturnErrorIfArgumentNull(a_pListener);
	OpcUa_ReturnErrorIfArgumentNull(a_pSocket);
	pTcpListener = (OpcUa_TcpListener*)a_pListener->Handle;
	OpcUa_ReturnErrorIfArgumentNull(pTcpListener);

	/******************************************************************************************************/

	/* look if an active connection is available for the socket. */
	uStatus = OpcUa_TcpListener_ConnectionManager_GetConnectionBySocket(pTcpListener->ConnectionManager,
																		a_pSocket,
																		&pTcpListenerConnection);
	if(OpcUa_IsBad(uStatus) && OpcUa_IsNotEqual(OpcUa_BadNotFound))
	{
		OpcUa_GotoError;
	}

	/******************************************************************************************************/

	/* try to find started stream either in pTcpListenerConnection or in floating messages list */
	if(pTcpListenerConnection != OpcUa_Null)
	{
		/* A connection object exists for this socket. (Hello message was received and validated.) */
		OpcUa_Mutex_Lock(pTcpListenerConnection->Mutex);
		pInputStream = pTcpListenerConnection->pInputStream;
		pTcpListenerConnection->pInputStream = OpcUa_Null;
	}
	else
	{
		/* no connection object is available, so this is the first message (and most probably a hello message) */
		/* look if a pending hello message for this socket exists; the connection gets created after the hello message is validated */
		uStatus = OpcUa_TcpListener_LookForPendingMessage(pTcpListener, a_pSocket, &pInputStream);
		if(OpcUa_IsBad(uStatus) && OpcUa_IsNotEqual(OpcUa_BadNotFound))
		{
			/* something unexpected happened */
			OpcUa_GotoError;
		}
	}

	/******************************************************************************************************/

	OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_TcpListener_TimeoutEventHandler: socket %p\n", a_pSocket);

	OpcUa_TcpStream_Close((OpcUa_Stream*)pInputStream);
	OpcUa_TcpStream_Delete((OpcUa_Stream**)&pInputStream);

	if(pTcpListenerConnection != OpcUa_Null)
	{
		OPCUA_P_SOCKET_CLOSE(pTcpListenerConnection->Socket);
		pTcpListenerConnection->Socket = OpcUa_Null;

		OpcUa_Mutex_Unlock(pTcpListenerConnection->Mutex);

		OpcUa_TcpListener_ProcessDisconnect(    a_pListener,
												pTcpListenerConnection);
	}

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_TcpListener_EventHandler Type
 *===========================================================================*/
/** @brief Internal handler prototype. */
typedef OpcUa_StatusCode (*OpcUa_TcpListener_EventHandler)(OpcUa_Listener*  a_pListener, 
														   OpcUa_Socket     a_pSocket);

/*============================================================================
 * OpcUa_TcpListener_NeedBufferEventHandler
 *===========================================================================*/
/**
* @brief TEST IMPLEMENTATION!
*/
OpcUa_StatusCode OpcUa_TcpListener_NeedBufferEventHandler(
	OpcUa_Listener* a_pListener, 
	OpcUa_Socket    a_pSocket)
{
	OpcUa_Byte*                     pBuffer                 = OpcUa_Null;
	OpcUa_UInt32                    nLength                 = 0;

OpcUa_InitializeStatus(OpcUa_Module_TcpListener, "NeedBufferEventHandler");

	OpcUa_ReferenceParameter(a_pListener);

	/******************************************************************************************************/

	/* todo: handle 0? */
	pBuffer = (OpcUa_Byte*)OpcUa_Alloc(OpcUa_ProxyStub_g_Configuration.iTcpListener_DefaultChunkSize);

	uStatus = OPCUA_P_SOCKET_READ(  a_pSocket,
									pBuffer, 
									OpcUa_ProxyStub_g_Configuration.iTcpListener_DefaultChunkSize, 
									&nLength);

	OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_TcpListener_NeedBufferEventHandler: Socket %08x requested buffer! Got 0x%p and returned 0x%08X\n", a_pSocket, pBuffer, uStatus);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_TcpListener_FreeBufferEventHandler
 *===========================================================================*/
/**
* @brief 
*/
OpcUa_StatusCode OpcUa_TcpListener_FreeBufferEventHandler(
	OpcUa_Listener* a_pListener, 
	OpcUa_Socket    a_pSocket)
{
	OpcUa_TcpListener*              pTcpListener            = (OpcUa_TcpListener*)a_pListener->Handle;
	OpcUa_TcpListener_Connection*   pTcpListenerConnection  = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_TcpListener, "FreeBufferEventHandler");

	/******************************************************************************************************/

	/* look if an active connection is available for the socket. */
	uStatus = OpcUa_TcpListener_ConnectionManager_GetConnectionBySocket(pTcpListener->ConnectionManager, 
																		a_pSocket, 
																		&pTcpListenerConnection);
	if(OpcUa_IsBad(uStatus) && OpcUa_IsNotEqual(OpcUa_BadNotFound))
	{ 
		OpcUa_GotoError;
	}

	/******************************************************************************************************/

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_TcpListener_ReadEventHandler
 *===========================================================================*/
/**
* @brief Gets called if data is available on the socket.
*/
OpcUa_StatusCode OpcUa_TcpListener_ReadEventHandler(
	OpcUa_Listener* a_pListener, 
	OpcUa_Socket    a_pSocket)
{
	OpcUa_TcpListener*              pTcpListener            = OpcUa_Null;
	OpcUa_TcpListener_Connection*   pTcpListenerConnection  = OpcUa_Null;
	OpcUa_InputStream*              pInputStream            = OpcUa_Null;
	OpcUa_TcpInputStream*           pTcpInputStream         = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_TcpListener, "ReadEventHandler");

	OpcUa_ReturnErrorIfArgumentNull(a_pListener);
	OpcUa_ReturnErrorIfArgumentNull(a_pSocket);
	pTcpListener = (OpcUa_TcpListener *)a_pListener->Handle;
	OpcUa_ReturnErrorIfArgumentNull(pTcpListener);

	/******************************************************************************************************/

	/* look if an active connection is available for the socket. */
	uStatus = OpcUa_TcpListener_ConnectionManager_GetConnectionBySocket(pTcpListener->ConnectionManager,
																		a_pSocket,
																		&pTcpListenerConnection);

	/* ignore OpcUa_BadNotFound (normal before successful Hello) */
	if(OpcUa_IsBad(uStatus) && OpcUa_IsNotEqual(OpcUa_BadNotFound))
	{
		OpcUa_GotoError;
	}

	/******************************************************************************************************/

	/* try to find started stream either in pTcpListenerConnection or in floating messages list */
	if(pTcpListenerConnection != OpcUa_Null)
	{
		/* A connection object exists for this socket. (Hello message was received and validated.) */
		if(pTcpListenerConnection->bNoRcvUntilDone == OpcUa_True)
		{
			pTcpListenerConnection->bRcvDataPending = OpcUa_True;
			OpcUa_ReturnStatusCode;
		}
		pInputStream = pTcpListenerConnection->pInputStream;
		pTcpListenerConnection->pInputStream = OpcUa_Null;
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_TcpListener_ReadEventHandler: Connection %p.\n", pTcpListenerConnection);
	}
	else
	{
		/* no connection object is available, so this is the first message (and most probably a hello message) */
		/* look if a pending hello message for this socket exists; the connection gets created after the hello message is validated */
		uStatus = OpcUa_TcpListener_LookForPendingMessage(pTcpListener, a_pSocket, &pInputStream);
		if(OpcUa_IsBad(uStatus) && OpcUa_IsNotEqual(OpcUa_BadNotFound))
		{ 
			/* something unexpected happened */
			OpcUa_GotoError;
		}
	}

	/******************************************************************************************************/

	/* create stream if no one was found */
	if(pInputStream == OpcUa_Null)
	{
		/* set the receiving buffer size to its default size */
		if(pTcpListenerConnection != OpcUa_Null)
		{
			uStatus = OpcUa_TcpStream_CreateInput(  a_pSocket, 
													pTcpListenerConnection->ReceiveBufferSize,
													&pInputStream);
			OpcUa_ReturnErrorIfBad(uStatus);
		}
		else
		{
			uStatus = OpcUa_TcpStream_CreateInput(  a_pSocket, 
													OpcUa_ProxyStub_g_Configuration.iTcpListener_DefaultChunkSize,
													&pInputStream);
			OpcUa_ReturnErrorIfBad(uStatus);
		}
	}

	/******************************************************************************************************/

	/* now, we have a stream -> read the available data; further processing takes place in the callback */
	uStatus = OpcUa_TcpStream_DataReady(pInputStream);

	/******************************************************************************************************/

	if(OpcUa_IsEqual(OpcUa_GoodCallAgain))
	{
		/* prepare to append further data later */

		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_TcpListener_ReadEventHandler: CallAgain result for stream %p on socket %p!\n", pInputStream, a_pSocket);

		if(pTcpListenerConnection != 0)
		{
			/* if we reach this point, the message cannot be a uamh */
			pTcpListenerConnection->pInputStream = pInputStream;
		}
		else
		{
			/* no pTcpListenerConnection to append it, so store it in our temporary list; must be uamh */
			OpcUa_List_Enter(pTcpListener->PendingMessages);
			uStatus = OpcUa_List_AddElement(pTcpListener->PendingMessages, pInputStream);
			OpcUa_List_Leave(pTcpListener->PendingMessages);
		}
	}
	else /* process message */
	{
		pTcpInputStream = (OpcUa_TcpInputStream*)pInputStream->Handle;

		if(OpcUa_IsBad(uStatus))
		{
			OpcUa_CharA* sError = OpcUa_Null;

			/* Error happened... */
			switch(uStatus)
			{
			case OpcUa_BadDecodingError:
				{
					sError = (OpcUa_CharA*)"OpcUa_BadDecodingError";
					break;
				}
			case OpcUa_BadCommunicationError:
				{
					sError = (OpcUa_CharA*)"OpcUa_BadCommunicationError";
					break;
				}
			case OpcUa_BadDisconnect:
				{
					sError = (OpcUa_CharA*)"OpcUa_BadDisconnect";
					break;
				}
			case OpcUa_BadConnectionClosed:
				{
					sError = (OpcUa_CharA*)"OpcUa_BadConnectionClosed";
					break;
				}
			case OpcUa_BadRequestTooLarge:
				{
					sError = (OpcUa_CharA*)"OpcUa_BadRequestTooLarge";
					break;
				}
			default:
				{
					sError = (OpcUa_CharA*)"unmapped";
				}
			}

			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_TcpListener_ReadEventHandler: socket %p; status 0x%08X (%s)\n", a_pSocket, uStatus, sError);

			OPCUA_P_SOCKET_CLOSE(a_pSocket);

			OpcUa_TcpStream_Close((OpcUa_Stream*)pInputStream);
			OpcUa_TcpStream_Delete((OpcUa_Stream**)&pInputStream);

			if(pTcpListenerConnection != OpcUa_Null)
			{
				pTcpListenerConnection->Socket = OpcUa_Null;

				/* Notify about connection loss. */
				OpcUa_TcpListener_ProcessDisconnect(    a_pListener,
														pTcpListenerConnection);
			}
		}
		else /* Message can be processed. */
		{
			/* process message */
			switch(pTcpInputStream->MessageType)
			{
			case OpcUa_TcpStream_MessageType_Hello:
				{
					OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_TcpListener_ReadEventHandler: MessageType HELLO\n");
					uStatus = OpcUa_TcpListener_ProcessHelloMessage(a_pListener, pInputStream);
					OpcUa_TcpStream_Close((OpcUa_Stream*)pInputStream);
					OpcUa_TcpStream_Delete((OpcUa_Stream**)&pInputStream);
					break;
				}
			case OpcUa_TcpStream_MessageType_SecureChannel:
				{
					/* This is the standard message used during communication.  */
					/* Abort is used here to rollback the data pipe up to the seclayer. */
					/* Maybe we will need a own handler for this. */

					OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_TcpListener_ReadEventHandler: MessageType SecureChannel Message\n");

					if(pTcpListenerConnection != OpcUa_Null)
					{
						/* unlink the now complete message from the temporary link */
						pTcpListenerConnection->pInputStream = OpcUa_Null;
						
						uStatus = OpcUa_TcpListener_ProcessRequest( a_pListener, 
																	pTcpListenerConnection, 
																	&pInputStream);
						
						if(pInputStream != OpcUa_Null)
						{
							OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_TcpListener_ReadEventHandler: InputStream wasn't correctly released! Deleting it!\n");
							OpcUa_TcpStream_Close((OpcUa_Stream*)pInputStream);
							OpcUa_TcpStream_Delete((OpcUa_Stream**)&pInputStream);
						}

						if(OpcUa_IsBad(uStatus))
						{
							/* this is probably intended: mask trace to make it not look like an error */
							if(OpcUa_IsNotEqual(OpcUa_BadDisconnect))
							{
								OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_WARNING, "OpcUa_TcpListener_ReadEventHandler: Process Request returned an error (0x%08X)!\n", uStatus);
							}

							if(pTcpListener->bShutdown == OpcUa_False)
							{
								if(OpcUa_IsEqual(OpcUa_BadDisconnect))
								{
									OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_TcpListener_ReadEventHandler: Closing network connection! 0x%05x\n", uStatus);
									OpcUa_TcpListener_CloseConnection(  a_pListener,
																		OPCUA_TCPLISTENER_CONNECTIONMANAGER_HANDLE_GET(pTcpListenerConnection),
																		OpcUa_Good); /* prevent error message */
								}
								else
								{
									/* send error message */
									uStatus = OpcUa_TcpListener_SendErrorMessage( a_pListener,
																				  pTcpListenerConnection,
																				  uStatus,
																				  OpcUa_Null);

									OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_WARNING, "OpcUa_TcpListener_ReadEventHandler: Closing socket (0x%08X)!\n", uStatus);
									OpcUa_TcpListener_CloseConnection(  a_pListener,
																		OPCUA_TCPLISTENER_CONNECTIONMANAGER_HANDLE_GET(pTcpListenerConnection),
																		uStatus); /* send error message in case of bad status */
								}
							}
						}
					}
					else
					{
						OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_WARNING, "OpcUa_TcpListener_ReadEventHandler: Received request for nonexisting connection!\n");
						OPCUA_P_SOCKET_CLOSE(pTcpInputStream->Socket);
						OpcUa_TcpStream_Close((OpcUa_Stream*)pInputStream);
						OpcUa_TcpStream_Delete((OpcUa_Stream**)&pInputStream);
						
					}
					
					break;
				}
			default:
				{
					OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_TcpListener_ReadEventHandler: Invalid MessageType (%d)\n", pTcpInputStream->MessageType);
					OpcUa_TcpStream_Close((OpcUa_Stream*)pInputStream);
					OpcUa_TcpStream_Delete((OpcUa_Stream**)&pInputStream);
					break;
				}
			}
		}
	} /* if(OpcUa_IsEqual(OpcUa_GoodCallAgain)) */

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}



/*============================================================================
 * OpcUa_TcpListener_WriteEventHandler
 *===========================================================================*/
/**
* @brief Gets called if data can be written to the socket.
*/
OpcUa_StatusCode OpcUa_TcpListener_WriteEventHandler(
	OpcUa_Listener* a_pListener, 
	OpcUa_Socket   a_pSocket)
{
	OpcUa_TcpListener*              pTcpListener            = OpcUa_Null;
	OpcUa_TcpListener_Connection*   pTcpListenerConnection  = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_TcpListener, "WriteEventHandler");

	OpcUa_ReturnErrorIfArgumentNull(a_pListener);
	OpcUa_ReturnErrorIfArgumentNull(a_pSocket);
	pTcpListener = (OpcUa_TcpListener *)a_pListener->Handle;
	OpcUa_ReturnErrorIfArgumentNull(pTcpListener);

	/******************************************************************************************************/

	/* look if an active connection is available for the socket. */
	uStatus = OpcUa_TcpListener_ConnectionManager_GetConnectionBySocket(pTcpListener->ConnectionManager, 
																		a_pSocket, 
																		&pTcpListenerConnection);
	if(OpcUa_IsBad(uStatus))
	{
		/* no connection available */
		OpcUa_GotoError;
	}

	/******************************************************************************************************/

	/* look for pending output stream */
	if(pTcpListenerConnection != OpcUa_Null)
	{
		do
		{
			while(pTcpListenerConnection->pSendQueue != OpcUa_Null)
			{
				OpcUa_BufferListElement* pCurrentBuffer = pTcpListenerConnection->pSendQueue;
				OpcUa_Int32              iDataLength    = pCurrentBuffer->Buffer.EndOfData - pCurrentBuffer->Buffer.Position;
				OpcUa_Int32              iDataWritten   = OPCUA_P_SOCKET_WRITE(
																a_pSocket,
																&pCurrentBuffer->Buffer.Data[pCurrentBuffer->Buffer.Position],
																iDataLength,
																OpcUa_False);
				if(iDataWritten < 0)
				{
					return OpcUa_TcpListener_TimeoutEventHandler(a_pListener, a_pSocket);
				}
				else if(iDataWritten == 0)
				{
					OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_TcpListener_WriteEventHandler: no data sent\n");
					uStatus = OpcUa_GoodCallAgain;
					OpcUa_ReturnStatusCode;
				}
				else if(iDataWritten < iDataLength)
				{
					pCurrentBuffer->Buffer.Position += iDataWritten;

					OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_TcpListener_WriteEventHandler: data partially sent (%i bytes)!\n", iDataWritten);

					if((pTcpListenerConnection->bNoRcvUntilDone == OpcUa_False) &&
					   (pTcpListenerConnection->bRcvDataPending == OpcUa_True))
					{
						pTcpListenerConnection->bRcvDataPending = OpcUa_False;
						uStatus = OpcUa_TcpListener_ReadEventHandler(a_pListener, a_pSocket);
						OpcUa_GotoErrorIfBad(uStatus);
					}

					uStatus = OpcUa_GoodCallAgain;
					OpcUa_ReturnStatusCode;
				}
				else
				{
					OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_TcpListener_WriteEventHandler: data sent!\n");
					pTcpListenerConnection->pSendQueue = pCurrentBuffer->pNext;
					OpcUa_Buffer_Clear(&pCurrentBuffer->Buffer);
					OpcUa_Free(pCurrentBuffer);
				}
			} /* end while */

			if(pTcpListenerConnection->bCloseWhenDone == OpcUa_True)
			{
				break;
			}

			pTcpListenerConnection->bNoRcvUntilDone = OpcUa_False;

			pTcpListener->Callback(
				a_pListener,                            /* the event source */
				(OpcUa_Void*)pTcpListener->CallbackData,/* the callback data */
				OpcUa_ListenerEvent_AsyncWriteComplete,    /* the event that occured */ 
				OPCUA_TCPLISTENER_CONNECTIONMANAGER_HANDLE_GET(pTcpListenerConnection),                 /* a connection handle */
				OpcUa_Null,                             /* the input stream for the event (none in this case) */
				uStatus);                               /* a status code for the event */

		} while(pTcpListenerConnection->pSendQueue != OpcUa_Null);

		if(pTcpListenerConnection->bCloseWhenDone == OpcUa_True)
		{
			uStatus = OpcUa_TcpListener_TimeoutEventHandler(a_pListener, a_pSocket);
		}
		else if((pTcpListenerConnection->bNoRcvUntilDone == OpcUa_False) &&
				(pTcpListenerConnection->bRcvDataPending == OpcUa_True))
		{
			pTcpListenerConnection->bRcvDataPending = OpcUa_False;
			uStatus = OpcUa_TcpListener_ReadEventHandler(a_pListener, a_pSocket);
		}
	}

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}


/*============================================================================
 * OpcUa_TcpListener_ExceptEventHandler
 *===========================================================================*/
OpcUa_StatusCode OpcUa_TcpListener_ExceptEventHandler(  OpcUa_Listener*  a_pListener, 
														OpcUa_Socket     a_pSocket)
{
	OpcUa_TcpListener*              pTcpListener    = OpcUa_Null;
	OpcUa_TcpListener_Connection*   pTcpConnection  = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_TcpListener, "ExceptEventHandler");

	OpcUa_ReturnErrorIfArgumentNull(a_pListener);
	pTcpListener = (OpcUa_TcpListener*)a_pListener->Handle;

	/* get connection for the socket. */
	uStatus = OpcUa_TcpListener_ConnectionManager_GetConnectionBySocket(pTcpListener->ConnectionManager, 
																		a_pSocket, 
																		&pTcpConnection);
	if(pTcpConnection != OpcUa_Null)
	{
			OpcUa_Mutex_Lock(pTcpConnection->Mutex);
			pTcpConnection->Socket = OpcUa_Null;
			OpcUa_Mutex_Unlock(pTcpConnection->Mutex);

		OpcUa_TcpListener_ProcessDisconnect(a_pListener,
											pTcpConnection);
	}

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_TcpListener_EventCallback
 *===========================================================================*/
OpcUa_StatusCode OpcUa_TcpListener_EventCallback(
	OpcUa_Socket    a_pSocket, 
	OpcUa_UInt32    a_uSocketEvent, 
	OpcUa_Void*     a_pUserData, 
	OpcUa_UInt16    a_uPortNumber, 
	OpcUa_Boolean   a_bIsSSL)
{
	OpcUa_StringA                   strEvent        = OpcUa_Null;
	OpcUa_Listener*                 listener        = (OpcUa_Listener*)a_pUserData;
	OpcUa_TcpListener*              pTcpListener    = (OpcUa_TcpListener*)listener->Handle;
	OpcUa_TcpListener_EventHandler  fEventHandler   = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_TcpListener, "SocketEventCallback");

	OpcUa_ReferenceParameter(a_bIsSSL);
	OpcUa_ReferenceParameter(a_uPortNumber);

	OpcUa_GotoErrorIfArgumentNull(a_pSocket);


#if 1 /* debug code */
	switch(a_uSocketEvent)
	{
	case OPCUA_SOCKET_NO_EVENT:
		{
			strEvent = "OPCUA_SOCKET_NO_EVENT";
			break;
		}
	case OPCUA_SOCKET_READ_EVENT:
		{
			strEvent = "OPCUA_SOCKET_READ_EVENT";
			break;
		}
	case OPCUA_SOCKET_WRITE_EVENT:
		{
			strEvent = "OPCUA_SOCKET_WRITE_EVENT";
			break;
		}
	case OPCUA_SOCKET_EXCEPT_EVENT:
		{
			strEvent = "OPCUA_SOCKET_EXCEPT_EVENT";
			break;
		}
	case OPCUA_SOCKET_TIMEOUT_EVENT:
		{
			strEvent = "OPCUA_SOCKET_TIMEOUT_EVENT";
			break;
		}
	case OPCUA_SOCKET_CLOSE_EVENT:
		{
			strEvent = "OPCUA_SOCKET_CLOSE_EVENT";
			break;
		}
	case OPCUA_SOCKET_CONNECT_EVENT:
		{
			strEvent = "OPCUA_SOCKET_CONNECT_EVENT";
			break;
		}
	case OPCUA_SOCKET_ACCEPT_EVENT:
		{
			strEvent = "OPCUA_SOCKET_ACCEPT_EVENT";
			break;
		}
	case OPCUA_SOCKET_SHUTDOWN_EVENT:
		{
			strEvent = "OPCUA_SOCKET_SHUTDOWN_EVENT";
			break;
		}
	case OPCUA_SOCKET_NEED_BUFFER_EVENT:
		{
			strEvent = "OPCUA_SOCKET_NEED_BUFFER_EVENT";
			break;
		}
	case OPCUA_SOCKET_FREE_BUFFER_EVENT:
		{
			strEvent = "OPCUA_SOCKET_FREE_BUFFER_EVENT";
			break;
		}
	default:
		{
			strEvent = "ERROR DEFAULT!";
			break;
		}
	}
	OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, " * OpcUa_TcpListener_EventCallback: Socket(%p), Port(%d), Data(%d), Event(%s)\n", a_pSocket, a_uPortNumber, a_pUserData, strEvent);
	/* debug code end */
#endif

	if(pTcpListener->bShutdown)
	{
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, " * OpcUa_TcpListener_EventCallback: Shutting down, ignoring Event.\n");
		OpcUa_ReturnStatusCode;
	}

	switch(a_uSocketEvent)
	{
	case OPCUA_SOCKET_READ_EVENT:
		{
			/* notifies an existing stream about new data or creates a new stream */
			fEventHandler = OpcUa_TcpListener_ReadEventHandler;
			break;
		}
	case OPCUA_SOCKET_EXCEPT_EVENT:
		{
			/* socket handle except events are handled as disconnects */
			fEventHandler = OpcUa_TcpListener_ExceptEventHandler;
			break;
		}
	case OPCUA_SOCKET_WRITE_EVENT:
		{
			fEventHandler = OpcUa_TcpListener_WriteEventHandler;
			break;
		}
	case OPCUA_SOCKET_NEED_BUFFER_EVENT:
		{
			fEventHandler = OpcUa_TcpListener_NeedBufferEventHandler;
			break;
		}
	case OPCUA_SOCKET_FREE_BUFFER_EVENT:
		{
			fEventHandler = OpcUa_TcpListener_FreeBufferEventHandler;
			break;
		}
	case OPCUA_SOCKET_NO_EVENT:
	case OPCUA_SOCKET_TIMEOUT_EVENT:
	case OPCUA_SOCKET_CLOSE_EVENT:
	case OPCUA_SOCKET_SHUTDOWN_EVENT:
		{
			break;
		}
	case OPCUA_SOCKET_ACCEPT_EVENT:
		{
			//OpcUa_Mutex_Lock(pTcpListener->Mutex);
			//pTcpListener->PendingConnects++;
			//OpcUa_Mutex_Unlock(pTcpListener->Mutex);
			break;
		}
	default:
		{
			/* unexpected error, report to upper layer. */
			pTcpListener->Callback(
				listener,                               /* the event source */
				(OpcUa_Void*)pTcpListener->CallbackData, /* the callback data */
				OpcUa_ListenerEvent_UnexpectedError,    /* the event that occured */ 
				OpcUa_Null,                             /* a connection handle */
				OpcUa_Null,                             /* the input stream for the event (none in this case) */
				uStatus);                               /* a status code for the event */

			break;
		}
	}

	/* call the internal specialized event handler */
	if(fEventHandler != OpcUa_Null)
	{
		uStatus = fEventHandler(listener, a_pSocket);
	}

	OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, " * OpcUa_TcpListener_EventCallback: Event Handler returned.\n");

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_TcpListener_ConnectionCloseCallback
 *===========================================================================*/
 /** @brief Callback function for the Connection Manager on connection deletion.
  *
  *  @param Listener The listener the tcp connection belongs to.
  *  @param TcpConnection The tcp connection that is being deleted.
  */
OpcUa_Void OpcUa_TcpListener_ConnectionDeleteCallback(  OpcUa_Listener*                 a_pListener,
														OpcUa_TcpListener_Connection*   a_pTcpConnection)
{
	OpcUa_ReferenceParameter(a_pListener);

#if OPCUA_P_SOCKETGETPEERINFO_V2
	OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, 
			"OpcUa_TcpListener_ConnectionDeleteCallback: Connection %p to peer %s (socket %X) gets closed!!\n", 
			a_pTcpConnection,
			a_pTcpConnection->achPeerInfo,
			a_pTcpConnection->Socket);
#else /* OPCUA_P_SOCKETGETPEERINFO_V2 */
	OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, 
			"OpcUa_TcpListener_ConnectionDeleteCallback: Connection %p to peer %d.%d.%d.%d:%d (socket %X) gets closed!!\n", 
			a_pTcpConnection,
			(OpcUa_Int)(a_pTcpConnection->PeerIp>>24)&0xFF, 
			(OpcUa_Int)(a_pTcpConnection->PeerIp>>16)&0xFF, 
			(OpcUa_Int)(a_pTcpConnection->PeerIp>>8) &0xFF,
			(OpcUa_Int) a_pTcpConnection->PeerIp     &0xFF, 
			a_pTcpConnection->PeerPort, 
			a_pTcpConnection->Socket);
#endif /* OPCUA_P_SOCKETGETPEERINFO_V2 */

	if(a_pTcpConnection->Socket != OpcUa_Null)
	{
		OPCUA_P_SOCKET_CLOSE(a_pTcpConnection->Socket);
		a_pTcpConnection->Socket = OpcUa_Null;
	}

	if(a_pTcpConnection->pInputStream != OpcUa_Null)
	{
		OpcUa_Stream_Close((OpcUa_Stream*)a_pTcpConnection->pInputStream);
		OpcUa_Stream_Delete((OpcUa_Stream**)&a_pTcpConnection->pInputStream);
	}
	
	/* TODO: consider invoking owner callback and tell about the closing. */

	return;
}

/*============================================================================
 * OpcUa_TcpListener_Close
 *===========================================================================*/
OpcUa_StatusCode OpcUa_TcpListener_Close(OpcUa_Listener* a_pListener)
{
	OpcUa_TcpListener* pTcpListener = OpcUa_Null;
	OpcUa_InputStream* pInputStream = OpcUa_Null;

	OpcUa_DeclareErrorTraceModule(OpcUa_Module_TcpListener);

	OpcUa_ReturnErrorIfArgumentNull(a_pListener);
	OpcUa_ReturnErrorIfInvalidObject(OpcUa_TcpListener, a_pListener, Close);

	pTcpListener     = (OpcUa_TcpListener*)a_pListener->Handle;

	/* lock connection and close the socket. */
	OpcUa_Mutex_Lock(pTcpListener->Mutex);

	/* mark listener as being in shutdown mode; certain calls are no longer accepted. */
	pTcpListener->bShutdown = OpcUa_True;

	/* check if already stopped */
	if(pTcpListener->Socket != OpcUa_Null)
	{
		/* only close listening socket, which should be in the global list. */
		OPCUA_P_SOCKET_CLOSE(pTcpListener->Socket);
		pTcpListener->Socket = OpcUa_Null;
	}

	/* cleanup all connections */
	OpcUa_TcpListener_ConnectionManager_RemoveConnections(  pTcpListener->ConnectionManager, 
															OpcUa_TcpListener_ConnectionDeleteCallback);

#if OPCUA_MULTITHREADED

	OpcUa_Mutex_Unlock(pTcpListener->Mutex);

	/* check if socket list handle is valid */
	if(pTcpListener->SocketManager != OpcUa_Null)
	{
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_TcpListener_Close: Stopping communication.\n");

		/* stops the thread and closes socket */
		OPCUA_P_SOCKETMANAGER_DELETE(&(pTcpListener->SocketManager));

		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_TcpListener_Close: Communication stopped.\n");
	}

	/* lock connection and close the socket. */
	OpcUa_Mutex_Lock(pTcpListener->Mutex);

#endif /* OPCUA_MULTITHREADED */

	OpcUa_List_Enter(pTcpListener->PendingMessages);
	OpcUa_List_ResetCurrent(pTcpListener->PendingMessages);
	pInputStream = (OpcUa_InputStream *)OpcUa_List_GetCurrentElement(pTcpListener->PendingMessages);
	while(pInputStream != OpcUa_Null)
	{
		OpcUa_List_DeleteCurrentElement(pTcpListener->PendingMessages);
		pInputStream->Close((OpcUa_Stream*)pInputStream);
		pInputStream->Delete((OpcUa_Stream**)&pInputStream);
		pInputStream = (OpcUa_InputStream *)OpcUa_List_GetCurrentElement(pTcpListener->PendingMessages);
	}
	OpcUa_List_Leave(pTcpListener->PendingMessages);

	OpcUa_Mutex_Unlock(pTcpListener->Mutex);

	/* notify about successful closing of the listener */
	pTcpListener->Callback( a_pListener,                /* the source of the event          */
							pTcpListener->CallbackData, /* the callback data                */
							OpcUa_ListenerEvent_Close,  /* the event that occured           */
							OpcUa_Null,                 /* the handle for the connection    */
							OpcUa_Null,                 /* the non existing stream          */
							OpcUa_Good);                /* status                           */

	return OpcUa_Good;
}

/*============================================================================
 * OpcUa_TcpListener_Open
 *===========================================================================*/
OpcUa_StatusCode OpcUa_TcpListener_Open(
	struct _OpcUa_Listener*     a_pListener,
	OpcUa_String*               a_sUrl,
	OpcUa_Listener_PfnOnNotify* a_pfnCallback,
	OpcUa_Void*                 a_pCallbackData)
{
	OpcUa_TcpListener*  pTcpListener        = OpcUa_Null;
	OpcUa_UInt32        uSocketManagerFlags = OPCUA_SOCKET_NO_FLAG;

OpcUa_InitializeStatus(OpcUa_Module_TcpListener, "Open");

	OpcUa_ReturnErrorIfArgumentNull(a_pListener);
	OpcUa_ReturnErrorIfArgumentNull(a_sUrl);

	OpcUa_ReturnErrorIfInvalidObject(OpcUa_TcpListener, a_pListener, Open);

	pTcpListener = (OpcUa_TcpListener*)a_pListener->Handle;
	OpcUa_ReturnErrorIfArgumentNull(pTcpListener);

	if(OpcUa_ProxyStub_g_Configuration.bTcpListener_ClientThreadsEnabled != OpcUa_False)
	{
		uSocketManagerFlags |= OPCUA_SOCKET_SPAWN_THREAD_ON_ACCEPT | OPCUA_SOCKET_REJECT_ON_NO_THREAD;
	}

	pTcpListener->bShutdown = OpcUa_False;

	/********************************************************************/

	/* lock listener while thread is starting */
	OpcUa_Mutex_Lock(pTcpListener->Mutex);

	/* check if thread already started */
	if(pTcpListener->Socket != OpcUa_Null)
	{
		OpcUa_Mutex_Unlock(pTcpListener->Mutex);
		return OpcUa_BadInvalidState;
	}

	pTcpListener->Callback     = a_pfnCallback;
	pTcpListener->CallbackData = a_pCallbackData;

	/********************************************************************/

	/* start up socket handling for this listener */
#if OPCUA_MULTITHREADED    
	/* check if socket list handle not yet set */
	if(pTcpListener->SocketManager != OpcUa_Null)
	{
		OpcUa_Mutex_Unlock(pTcpListener->Mutex);
		return OpcUa_BadInvalidState;
	}

	uStatus = OPCUA_P_SOCKETMANAGER_CREATE( &(pTcpListener->SocketManager), 
											OPCUA_TCPLISTENER_MAXCONNECTIONS + 1, /* add one for listen socket */
											uSocketManagerFlags);
	OpcUa_GotoErrorIfBad(uStatus);

	uStatus = OPCUA_P_SOCKETMANAGER_CREATESERVER(   pTcpListener->SocketManager,
													OpcUa_String_GetRawString(a_sUrl),
													pTcpListener->pCertificate,
													pTcpListener->pPrivateKey,
													pTcpListener->pPKIConfig,
													OpcUa_TcpListener_EventCallback,
													(OpcUa_Void*)a_pListener,
													&(pTcpListener->Socket));

#else /* OPCUA_MULTITHREADED */

	/* single thread socket created on global socket manager */
	uStatus = OPCUA_P_SOCKETMANAGER_CREATESERVER(   OpcUa_Null,
													OpcUa_String_GetRawString(a_sUrl),
													pTcpListener->pCertificate,
													pTcpListener->pPrivateKey,
													pTcpListener->pPKIConfig,
													OpcUa_TcpListener_EventCallback,
													(OpcUa_Void*)a_pListener,
													&(pTcpListener->Socket));

#endif /* OPCUA_MULTITHREADED */

	if(OpcUa_IsBad(uStatus))
	{
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_WARNING, "OpcUa_TcpListener_Open: Failed to create listen socket!\n");
		OpcUa_GotoError;
	}

	/********************************************************************/

	/* notify about successful opening of the listener */
	pTcpListener->Callback( a_pListener,                /* the source of the event          */
							pTcpListener->CallbackData,  /* the callback data                */
							OpcUa_ListenerEvent_Open,   /* the event that occured           */
							OpcUa_Null,                 /* the handle for the connection    */
							OpcUa_Null,                 /* the non existing stream          */
							OpcUa_Good);                /* status                           */

	OpcUa_Mutex_Unlock(pTcpListener->Mutex);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
	
	OpcUa_TcpListener_Close(a_pListener);

	OpcUa_Mutex_Unlock(pTcpListener->Mutex);

OpcUa_FinishErrorHandling;
}

#endif /* OPCUA_HAVE_SERVERAPI */
