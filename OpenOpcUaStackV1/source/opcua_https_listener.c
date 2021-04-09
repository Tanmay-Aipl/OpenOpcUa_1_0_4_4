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
#ifdef OPCUA_HAVE_HTTPSAPI

#include <opcua_mutex.h>
#include <opcua_string.h>
#include <opcua_datetime.h>
#include <opcua_socket.h>
#include <opcua_statuscodes.h>
#include <opcua_list.h>
#include <opcua_utilities.h>

#include <opcua_httpsstream.h>
#include <opcua_https_listener.h>
#include <opcua_https_internal.h>

typedef struct _OpcUa_HttpsListener OpcUa_HttpsListener;

#include <opcua_https_listener_connectionmanager.h>

/* for debugging reasons */
#include <opcua_p_binary.h>
#include <opcua_memorystream.h>

extern OpcUa_Guid OpcUa_Guid_Null;

#define OPCUA_HTTPSLISTENER_USEEXTRAMAXCONNSOCKET       OPCUA_CONFIG_NO

/** @brief Close socket immediately after response (incl. connection:close header). */
#define OPCUA_HTTPSLISTENER_CLOSE_SOCKET_AFTER_RESPONSE OPCUA_CONFIG_NO
OpcUa_StatusCode OpcUa_HttpsListener_AddStreamToSendQueue(
    OpcUa_Listener*                 a_pListener,
    OpcUa_HttpsListener_Connection* a_pListenerConnection,
    OpcUa_OutputStream*             a_pOutputStream);
/*============================================================================
 * Prototypes
 *===========================================================================*/
OpcUa_StatusCode OpcUa_HttpsListener_Open(
    OpcUa_Listener*                 a_pListener,
    OpcUa_String*                   a_sUrl,
    OpcUa_Listener_PfnOnNotify*     a_pfnCallback,
    OpcUa_Void*                     a_pCallbackData);

OpcUa_StatusCode OpcUa_HttpsListener_Close(
    OpcUa_Listener*                 a_pListener);

OpcUa_StatusCode OpcUa_HttpsListener_BeginSendResponse(
    OpcUa_Listener*                 a_pListener,
    OpcUa_Handle                    a_pConnection,
    OpcUa_InputStream**             a_ppInputStream,
    OpcUa_OutputStream**            a_ppOutputStream);

OpcUa_StatusCode OpcUa_HttpsListener_EndSendResponse(
    OpcUa_Listener*                 a_pListener,
    OpcUa_StatusCode                a_uStatus,
    OpcUa_OutputStream**            a_ppOutputStream);

OpcUa_StatusCode OpcUa_HttpsListener_AbortSendResponse(
    OpcUa_Listener*                 a_pListener,
    OpcUa_StatusCode                a_uStatus,
    OpcUa_String*                   a_sReason,
    OpcUa_OutputStream**            a_ppOutputStream);

OpcUa_StatusCode OpcUa_HttpsListener_CloseConnection(
    OpcUa_Listener*                 a_pListener,
    OpcUa_Handle                    a_hConnection,
    OpcUa_StatusCode                a_uStatus);

OpcUa_StatusCode OpcUa_HttpsListener_ProcessDisconnect(
    OpcUa_Listener*                  a_pListener,
    OpcUa_HttpsListener_Connection** a_ppListenerConnection);

OpcUa_StatusCode OpcUa_HttpsListener_GetReceiveBufferSize(
    OpcUa_Listener*                 a_pListener,
    OpcUa_Handle                    a_hConnection,
    OpcUa_UInt32*                   a_pBufferSize);

OPCUA_EXPORT OpcUa_StatusCode OpcUa_HttpsListener_GetSecurityPolicyConfiguration(
    OpcUa_Listener*                             a_pListener,
    OpcUa_InputStream*                          a_pIstrm,
    OpcUa_Listener_SecurityPolicyConfiguration* a_pSecurityPolicyConfiguration);

OpcUa_Void OpcUa_HttpsListener_ConnectionDeleteCallback(
    OpcUa_Listener*                 a_pListener,
    OpcUa_HttpsListener_Connection* a_pListenerConnection);

OpcUa_StatusCode OpcUa_HttpsListener_GetPeerInfo(
    OpcUa_Listener*                 a_pListener,
    OpcUa_Handle                    a_hConnection,
    OpcUa_String*                   a_sPeerInfo);

OpcUa_StatusCode OpcUa_HttpsListener_AddToSendQueue(
    OpcUa_Listener*                 a_pListener,
    OpcUa_Handle                    a_hConnection,
    OpcUa_BufferListElement*        a_pBufferList,
    OpcUa_UInt32                    a_uFlags);

OpcUa_StatusCode OpcUa_HttpsListener_ReadEventHandler(
    OpcUa_Listener* a_pListener,
    OpcUa_Socket    a_hSocket);
/*============================================================================
 * OpcUa_HttpsListener_SanityCheck
 *===========================================================================*/
#define OpcUa_HttpsListener_SanityCheck 0x596E77D5

/*============================================================================
 * OpcUa_HttpsListener
 *===========================================================================*/
 /** @brief This struct represents a listener for http transport. */
struct _OpcUa_HttpsListener
{
/* This is inherited from the OpcUa_Listener. */

    /*! @brief An opaque handle that contain data specific to the listener implementation. */
    OpcUa_Handle                                        Handle;
    /*! @brief Begins listening at an endpoint. */
    OpcUa_Listener_PfnOpen*                             Open;
    /*! @brief Stops listening at an endpoint. */
    OpcUa_Listener_PfnClose*                            Close;
    /*! @brief Finishes reading an incoming request and starts writing the response. */
    OpcUa_Listener_PfnBeginSendResponse*                BeginSendResponse;
    /*! @brief Finishes writing an outgoing response. */
    OpcUa_Listener_PfnEndSendResponse*                  EndSendResponse;
    /*! @brief Cancels writing an outgoing response. */
    OpcUa_Listener_PfnAbortSendResponse*                AbortSendResponse;
    /*! @brief Retrive the recieve buffer size of a particular connection. */
    OpcUa_Listener_PfnGetReceiveBufferSize*             GetReceiveBufferSize;
    /*! @brief Retrive the recieve security settings in use for the given stream. */
    OpcUa_Listener_PfnGetSecurityPolicyConfiguration*   GetSecurityPolicyConfiguration;
    /*! @brief Close a particular connection. */
    OpcUa_Listener_PfnCloseConnection*                  CloseConnection;
    /*! @brief Frees the structure. */
    OpcUa_Listener_PfnDelete*                           Delete;
    /*! @brief Retrive the peer info of a particular connection. */
    OpcUa_Listener_PfnGetPeerInfo*                      GetPeerInfo;

/* End inherited from the OpcUa_Listener. */

    /** @brief Internal control value. */
    OpcUa_UInt32                                        SanityCheck;
    /** @brief Synchronize access to the listener. */
    OpcUa_Mutex                                         Mutex;
    /** @brief The listen socket (either part of the global or the own socket list). */
    OpcUa_Socket                                        Socket;
#if OPCUA_MULTITHREADED
    /** @brief In multithreaded environments, each listener manages its own list of sockets. */
    OpcUa_SocketManager                                 SocketManager;
#endif /* OPCUA_MULTITHREADED */
    /** @brief The function which receives notifications about listener events. */
    OpcUa_Listener_PfnOnNotify*                         pfListenerCallback;
    /** @brief Data passed with the callback function. */
    OpcUa_Void*                                         pvListenerCallbackData;
    /** @brief This Listener should shut down. */
    OpcUa_Boolean                                       bShutdown;
    /** @brief This list contains all pending requests, which are not fully received
     *  yet. Once a request is completely received, it gets dispatched to the
     *  upper layer. */
    OpcUa_List*                                         pPendingMessages;
    /** @brief Holds the information about connected clients and helps verifying requests. */
    OpcUa_HttpsListener_ConnectionManager*              pConnectionManager;
    /** @brief Certificate used for SSL/TLS connections. */
    OpcUa_ByteString*                                   pCertificate;
    /** @brief Private key used for SSL/TLS connections.*/
    OpcUa_Key*                                          pPrivateKey;
    /** @brief PKI configuration for SSL/TLS connections. */
    OpcUa_Void*                                         pPKIConfig;
    /** @brief Callback for channel event notification. Only called in Open and Close function. */
    OpcUa_HttpsListener_PfnSecureChannelCallback*       pfSecureChannelCallback;
    /** @brief User data returned in pfSecureChannelCallback. */
    OpcUa_Void*                                         pvSecureChannelCallbackData;
};


/*============================================================================
 * OpcUa_HttpsListener_Delete
 *===========================================================================*/
OpcUa_Void OpcUa_HttpsListener_Delete(OpcUa_Listener** a_ppListener)
{
    OpcUa_HttpsListener* pHttpsListener = OpcUa_Null;
    OpcUa_InputStream*   pInputStream   = OpcUa_Null;

    if (a_ppListener == OpcUa_Null || *a_ppListener == OpcUa_Null)
    {
        return;
    }

    pHttpsListener = (OpcUa_HttpsListener*)(*a_ppListener)->Handle;

    if(pHttpsListener != OpcUa_Null)
    {
        OpcUa_Mutex_Lock(pHttpsListener->Mutex);
        pHttpsListener->SanityCheck = 0;

        /* delete all pending messages */
        OpcUa_List_Enter(pHttpsListener->pPendingMessages);
        OpcUa_List_ResetCurrent(pHttpsListener->pPendingMessages);
        pInputStream = (OpcUa_InputStream *)OpcUa_List_GetCurrentElement(pHttpsListener->pPendingMessages);
        while(pInputStream != OpcUa_Null)
        {
            OpcUa_List_DeleteCurrentElement(pHttpsListener->pPendingMessages);
            pInputStream->Close((OpcUa_Stream*)pInputStream);
            pInputStream->Delete((OpcUa_Stream**)&pInputStream);
            pInputStream = (OpcUa_InputStream *)OpcUa_List_GetCurrentElement(pHttpsListener->pPendingMessages);
        }
        OpcUa_List_Leave(pHttpsListener->pPendingMessages);
        OpcUa_List_Delete(&(pHttpsListener->pPendingMessages));

        OpcUa_HttpsListener_ConnectionManager_Delete(&(pHttpsListener->pConnectionManager));

        OpcUa_Mutex_Unlock(pHttpsListener->Mutex);
        OpcUa_Mutex_Delete(&(pHttpsListener->Mutex));

        OpcUa_Free(pHttpsListener);
    }

    *a_ppListener = OpcUa_Null;
}

/*============================================================================
 * OpcUa_HttpsListener_Create
 *===========================================================================*/
OpcUa_StatusCode OpcUa_HttpsListener_Create(OpcUa_ByteString*                               a_pServerCertificate,
                                            OpcUa_Key*                                      a_pServerPrivateKey,
                                            OpcUa_Void*                                     a_pPKIConfig,
                                            OpcUa_HttpsListener_PfnSecureChannelCallback*   a_pfSecureChannelCallback,
                                            OpcUa_Void*                                     a_pSecureChannelCallbackData,
                                            OpcUa_Listener**                                a_ppListener)
{
    OpcUa_HttpsListener*  pHttpsListener = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_HttpListener, "Create");

    OpcUa_ReturnErrorIfArgumentNull(a_ppListener);

    OpcUa_ReferenceParameter(a_pServerCertificate);
    OpcUa_ReferenceParameter(a_pServerPrivateKey);
    OpcUa_ReferenceParameter(a_pPKIConfig);

    /* allocate listener object */
    *a_ppListener = (OpcUa_Listener*)OpcUa_Alloc(sizeof(OpcUa_HttpsListener));
    OpcUa_GotoErrorIfAllocFailed(*a_ppListener);
    OpcUa_MemSet(*a_ppListener, 0, sizeof(OpcUa_HttpsListener));
    pHttpsListener = (OpcUa_HttpsListener*)*a_ppListener;

    /* initialize listener pHttpsListener */
    pHttpsListener->SanityCheck                 = OpcUa_HttpsListener_SanityCheck;
    pHttpsListener->pfSecureChannelCallback     = a_pfSecureChannelCallback;
    pHttpsListener->pvSecureChannelCallbackData = a_pSecureChannelCallbackData;

    /* security credentials */
    pHttpsListener->pPrivateKey                 = a_pServerPrivateKey;
    pHttpsListener->pCertificate                = a_pServerCertificate;
    pHttpsListener->pPKIConfig                  = a_pPKIConfig;

    uStatus = OpcUa_Mutex_Create(&(pHttpsListener->Mutex));
    OpcUa_GotoErrorIfBad(uStatus);

    uStatus = OpcUa_List_Create(&(pHttpsListener->pPendingMessages));
    OpcUa_GotoErrorIfBad(uStatus);

    uStatus = OpcUa_HttpsListener_ConnectionManager_Create( OpcUa_HttpsListener_ConnectionDeleteCallback,
                                                            &(pHttpsListener->pConnectionManager));
    pHttpsListener->pConnectionManager->Listener = *a_ppListener;

    /* HINT: socket and socket list get managed in open/close */

    /* initialize listener object */
    (*a_ppListener)->Handle                         = pHttpsListener;
    (*a_ppListener)->Open                           = OpcUa_HttpsListener_Open;
    (*a_ppListener)->Close                          = OpcUa_HttpsListener_Close;
    (*a_ppListener)->BeginSendResponse              = OpcUa_HttpsListener_BeginSendResponse;
    (*a_ppListener)->EndSendResponse                = OpcUa_HttpsListener_EndSendResponse;
    (*a_ppListener)->AbortSendResponse              = OpcUa_HttpsListener_AbortSendResponse;
    (*a_ppListener)->CloseConnection                = OpcUa_HttpsListener_CloseConnection;
    (*a_ppListener)->GetReceiveBufferSize           = OpcUa_HttpsListener_GetReceiveBufferSize;
    (*a_ppListener)->GetSecurityPolicyConfiguration = OpcUa_HttpsListener_GetSecurityPolicyConfiguration;
    (*a_ppListener)->GetPeerInfo                    = OpcUa_HttpsListener_GetPeerInfo;
    (*a_ppListener)->Delete                         = OpcUa_HttpsListener_Delete;
    (*a_ppListener)->AddToSendQueue                 = OpcUa_HttpsListener_AddToSendQueue;

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    if (*a_ppListener != OpcUa_Null)
    {
        OpcUa_Free(*a_ppListener);
        *a_ppListener = OpcUa_Null;
    }

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_HttpsListener_GetSecureChannelSecurityPolicyConfiguration
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_HttpsListener_GetSecurityPolicyConfiguration(
    OpcUa_Listener*                             a_pListener,
    OpcUa_InputStream*                          a_pIstrm,
    OpcUa_Listener_SecurityPolicyConfiguration* a_pSecurityPolicyConfiguration)
{
    OpcUa_HttpsListener*            pHttpsListener      = OpcUa_Null;
    OpcUa_Socket                    pSocket             = OpcUa_Null;
    OpcUa_HttpsListener_Connection* pHttpsConnection    = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_HttpListener, "GetSecurityPolicyConfiguration");

    OpcUa_ReturnErrorIfArgumentNull(a_pListener);
    OpcUa_ReturnErrorIfArgumentNull(a_pListener->Handle);
    OpcUa_ReturnErrorIfArgumentNull(a_pIstrm);
    OpcUa_ReturnErrorIfArgumentNull(a_pSecurityPolicyConfiguration);

    pHttpsListener  = (OpcUa_HttpsListener*)a_pListener->Handle;

    /* get connection object */
    OpcUa_HttpsStream_GetSocket((OpcUa_Stream*)a_pIstrm,
                                &pSocket);

    OpcUa_HttpsListener_ConnectionManager_GetConnectionBySocket(    pHttpsListener->pConnectionManager,
                                                                    pSocket,
                                                                    &pHttpsConnection);
    OpcUa_GotoErrorIfNull(pHttpsConnection, OpcUa_BadInvalidArgument);

    /* header is optional, check if it was set and default to policy none if not */
    if(OpcUa_String_IsNull(&pHttpsConnection->sSecurityPolicy))
    {
        uStatus = OpcUa_String_AttachToString(  OpcUa_SecurityPolicy_None,
                                                OPCUA_STRING_LENDONTCARE,
                                                0,
                                                OpcUa_False,
                                                OpcUa_False,
                                                &a_pSecurityPolicyConfiguration->sSecurityPolicy);
        OpcUa_GotoErrorIfBad(uStatus);
    }
    else
    {
        uStatus = OpcUa_String_AttachToString(  OpcUa_String_GetRawString(&pHttpsConnection->sSecurityPolicy),
                                                OpcUa_String_StrLen(&pHttpsConnection->sSecurityPolicy),
                                                0,
                                                OpcUa_False,
                                                OpcUa_False,
                                                &a_pSecurityPolicyConfiguration->sSecurityPolicy);
        OpcUa_GotoErrorIfBad(uStatus);
    }

    a_pSecurityPolicyConfiguration->uMessageSecurityModes = OpcUa_MessageSecurityMode_None;
    a_pSecurityPolicyConfiguration->pbsClientCertificate  = OpcUa_Null;

    OpcUa_HttpsListener_ConnectionManager_ReleaseConnection(    pHttpsListener->pConnectionManager,
                                                                &pHttpsConnection);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    if(pHttpsConnection != OpcUa_Null)
    {
        OpcUa_HttpsListener_ConnectionManager_ReleaseConnection(    pHttpsListener->pConnectionManager,
                                                                    &pHttpsConnection);
    }

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_HttpsListener_GetReceiveBufferSize
 *===========================================================================*/
OpcUa_StatusCode OpcUa_HttpsListener_GetReceiveBufferSize(
    OpcUa_Listener* a_pListener,
    OpcUa_Handle    a_hConnection,
    OpcUa_UInt32*   a_pBufferSize)
{
OpcUa_InitializeStatus(OpcUa_Module_HttpListener, "GetReceiveBufferSize");

    OpcUa_ReturnErrorIfArgumentNull(a_hConnection);
    OpcUa_ReturnErrorIfArgumentNull(a_pBufferSize);

    OpcUa_ReferenceParameter(a_pListener);

    /* buffer size is unlimited */
    *a_pBufferSize = OpcUa_UInt32_Max;

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_HttpsListener_GetPeerInfo
 *===========================================================================*/
OpcUa_StatusCode OpcUa_HttpsListener_GetPeerInfo(OpcUa_Listener*    a_pListener,
                                                 OpcUa_Handle       a_hConnection,
                                                 OpcUa_String*      a_psPeerInfo)
{
    OpcUa_HttpsListener_Connection* pHttpsListenerConnection  = (OpcUa_HttpsListener_Connection*)a_hConnection;

OpcUa_InitializeStatus(OpcUa_Module_HttpListener, "GetPeerInfo");

    OpcUa_ReferenceParameter(a_pListener);
    OpcUa_GotoErrorIfArgumentNull(a_hConnection);
    OpcUa_GotoErrorIfArgumentNull(a_psPeerInfo);

    uStatus = OpcUa_String_StrnCpy( a_psPeerInfo,
                                    OpcUa_String_FromCString(pHttpsListenerConnection->achPeerInfo),
                                    OPCUA_STRING_LENDONTCARE);
    OpcUa_GotoErrorIfBad(uStatus);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_HttpsListener_CloseConnection
 *===========================================================================*/
/** @brief Close a particular connection of this listener. */
OpcUa_StatusCode OpcUa_HttpsListener_CloseConnection(
    OpcUa_Listener*     a_pListener,
    OpcUa_Handle        a_hConnection,
    OpcUa_StatusCode    a_uStatus)
{
    OpcUa_HttpsListener_Connection*  pListenerConnection    = OpcUa_Null;
    OpcUa_HttpsListener*             pHttpsListener         = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_HttpListener, "CloseConnection");

    OpcUa_ReturnErrorIfArgumentNull(a_pListener);
    OpcUa_ReturnErrorIfArgumentNull(a_hConnection);

    pHttpsListener      = (OpcUa_HttpsListener*)a_pListener->Handle;
    pListenerConnection = (OpcUa_HttpsListener_Connection*)a_hConnection;

    OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR,
                "OpcUa_HttpsListener_CloseConnection: Connection %p is being closed! 0x%08X\n",
                a_hConnection,
                a_uStatus);

    if(OpcUa_IsBad(a_uStatus))
    {
        OpcUa_HttpsListener_SendImmediateResponse(  a_pListener,
                                                    a_hConnection,
                                                    500,
                                                    "Internal Server Error",
                                                    "Server: OPC-ANSI-C-HTTPS-API/0.1\r\n"
                                                    "Content-Type: application/octet-stream\r\n",
                                                    OpcUa_Null,
                                                    0);
    }

    OPCUA_P_SOCKET_CLOSE(pListenerConnection->Socket);
    pListenerConnection->Socket = OpcUa_Null;
    OpcUa_HttpsListener_ConnectionManager_ReleaseConnection(    pHttpsListener->pConnectionManager,
                                                               &pListenerConnection);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_HttpsListener_ConnectionDisconnectCB
 *===========================================================================*/
/** @brief Gets called by an outstream if the connection is lost. */
static OpcUa_Void OpcUa_HttpsListener_ConnectionDisconnectCB(OpcUa_Handle a_hConnection)
{
    OpcUa_ReferenceParameter(a_hConnection);

    OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_HttpsListener_ConnectionDisconnectCB: Connection %p is being reported as disconnected!\n", a_hConnection);
}

/*============================================================================
 * OpcUa_HttpsListener_BeginSendResponse
 *===========================================================================*/
/* prepare a response (out) stream for a certain connection and related to   */
/* a certain request (in) stream                                             */
OpcUa_StatusCode OpcUa_HttpsListener_BeginSendResponse(
    OpcUa_Listener*         a_pListener,
    OpcUa_Handle            a_hConnection,
    OpcUa_InputStream**     a_ppInputStream,
    OpcUa_OutputStream**    a_ppOutputStream)
{
    OpcUa_HttpsListener_Connection* pListenerConnection = OpcUa_Null;
    OpcUa_HttpsStream_Method        eMethod             = OpcUa_HttpsStream_Method_Invalid;

OpcUa_InitializeStatus(OpcUa_Module_HttpListener, "BeginSendResponse");

    OpcUa_ReturnErrorIfArgumentNull(a_pListener);
    OpcUa_ReturnErrorIfArgumentNull(a_hConnection);
    OpcUa_ReturnErrorIfArgumentNull(a_ppInputStream);
    OpcUa_ReturnErrorIfArgumentNull(a_ppOutputStream);

    OpcUa_ReturnErrorIfArgumentNull(a_pListener->BeginSendResponse);

    /* initialize outparameter */
    *a_ppOutputStream = OpcUa_Null;

    pListenerConnection = (OpcUa_HttpsListener_Connection*)a_hConnection;

    uStatus = OpcUa_HttpsStream_GetMethod( (OpcUa_Stream*)(*a_ppInputStream),
                                           &eMethod);
    OpcUa_GotoErrorIfBad(uStatus);

    /* close and delete the incoming stream - double close is ignored (uncritical) */
    OpcUa_Stream_Close((OpcUa_Stream*)(*a_ppInputStream));
    OpcUa_Stream_Delete((OpcUa_Stream**)a_ppInputStream);

    if(eMethod != OpcUa_HttpsStream_Method_Get)
    {
        /* create buffer for writing */
        uStatus = OpcUa_HttpsStream_CreateResponse( pListenerConnection->Socket,
                                                    200,
                                                    "OK",
                                                    "Server: OPC-ANSI-C-HTTPS-API/0.1\r\n"
#if OPCUA_HTTPSLISTENER_CLOSE_SOCKET_AFTER_RESPONSE
                                                    "Connection: close\r\n"
#endif
                                                    "Content-Type: application/octet-stream\r\n",
                                                    a_hConnection,
                                                    OpcUa_HttpsListener_ConnectionDisconnectCB,
                                                    a_ppOutputStream);
    }
    else
    {
        /* create buffer for writing */
        uStatus = OpcUa_HttpsStream_CreateResponse( pListenerConnection->Socket,
                                                    200,
                                                    "OK",
                                                    "Server: OPC-ANSI-C-HTTPS-API/0.1\r\n"
#if OPCUA_HTTPSLISTENER_CLOSE_SOCKET_AFTER_RESPONSE
                                                    "Connection: close\r\n"
#endif
                                                    ,
                                                    a_hConnection,
                                                    OpcUa_HttpsListener_ConnectionDisconnectCB,
                                                    a_ppOutputStream);
    }

    OpcUa_ReturnErrorIfBad(uStatus);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * Send an HTTP response.
 *===========================================================================*/
OpcUa_StatusCode OpcUa_HttpsListener_SendImmediateResponse(
    OpcUa_Listener*     a_pListener,
    OpcUa_Handle        a_hConnection,
    OpcUa_StatusCode    a_uStatusCode,
    OpcUa_StringA       a_sReasonPhrase,
    OpcUa_StringA       a_sResponseHeaders,
    OpcUa_Byte*         a_pResponseData,
    OpcUa_UInt32        a_uResponseLength)
{
    OpcUa_HttpsListener*             pHttpsListener         = OpcUa_Null;
    OpcUa_HttpsListener_Connection*  pListenerConnection    = OpcUa_Null;
    OpcUa_OutputStream*              pOutputStream          = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_HttpListener, "SendImmediateResponse");

    OpcUa_ReturnErrorIfArgumentNull(a_pListener);
    OpcUa_ReturnErrorIfArgumentNull(a_hConnection);

    OpcUa_ReturnErrorIfTrue(    a_uResponseLength > 0
                             && a_pResponseData == OpcUa_Null,
                            OpcUa_BadInvalidArgument);

    pHttpsListener       = (OpcUa_HttpsListener*)a_pListener->Handle;
    pListenerConnection = (OpcUa_HttpsListener_Connection*)a_hConnection;

    if(    (pHttpsListener->bShutdown != OpcUa_False)
        && (pListenerConnection->bConnected != OpcUa_False))
    {
        OpcUa_ReturnStatusCode;
    }

    OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR,
                "OpcUa_HttpsListener_SendImmediateResponse: to %s (socket %X) with StatusCode %d!\n",
                pListenerConnection->achPeerInfo,
                pListenerConnection->Socket,
                a_uStatusCode);

    /* create the output stream for the response */
    uStatus = OpcUa_HttpsStream_CreateResponse( pListenerConnection->Socket,
                                                a_uStatusCode,
                                                a_sReasonPhrase,
                                                a_sResponseHeaders,
                                                a_hConnection,
                                                OpcUa_HttpsListener_ConnectionDisconnectCB,
                                                &pOutputStream);
    OpcUa_GotoErrorIfBad(uStatus);

    /* write response data if any */
    if(a_pResponseData != OpcUa_Null)
    {
        uStatus = pOutputStream->Write(pOutputStream, a_pResponseData, a_uResponseLength);
        OpcUa_GotoErrorIfBad(uStatus);
    }

    /* send stream if possible or queue for delayed sending */
    if(pListenerConnection->pSendQueue == OpcUa_Null)
    {
        uStatus = pOutputStream->Close((OpcUa_Stream*)pOutputStream);
    }
    else
    {
        uStatus = OpcUa_BadWouldBlock;
    }

    if(OpcUa_IsEqual(OpcUa_BadWouldBlock))
    {
        /* try to put stream content into buffer queue for delayed sending */
        uStatus = OpcUa_HttpsListener_AddStreamToSendQueue(a_pListener, pListenerConnection, pOutputStream);
    }

    OpcUa_GotoErrorIfBad(uStatus);

    pOutputStream->Delete((OpcUa_Stream**)&pOutputStream);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    OpcUa_Stream_Delete((OpcUa_Stream**)&pOutputStream);

OpcUa_FinishErrorHandling;
}
/*============================================================================
 * OpcUa_HttpsListener_AddToSendQueue
 *===========================================================================*/
OpcUa_StatusCode OpcUa_HttpsListener_AddToSendQueue(OpcUa_Listener*          a_pListener,
                                                    OpcUa_Handle             a_hConnection,
                                                    OpcUa_BufferListElement* a_pBufferList,
                                                    OpcUa_UInt32             a_uFlags)
{
    OpcUa_HttpsListener_Connection* pListenerConnection = (OpcUa_HttpsListener_Connection*)a_hConnection;

OpcUa_InitializeStatus(OpcUa_Module_HttpListener, "AddToSendQueue");

    OpcUa_ReturnErrorIfArgumentNull(a_hConnection);
    OpcUa_ReferenceParameter(a_pListener);

    OpcUa_Mutex_Lock(pListenerConnection->Mutex);

    if(pListenerConnection->pSendQueue == OpcUa_Null)
    {
        OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_HttpsListener_AddToSendQueue: Set buffer list of connection 0x%08X to 0x%08X.\n", pListenerConnection, a_pBufferList);
        pListenerConnection->pSendQueue = a_pBufferList;
    }
    else
    {
        OpcUa_BufferListElement* pLastEntry = pListenerConnection->pSendQueue;
        while(pLastEntry->pNext != OpcUa_Null)
        {
            pLastEntry = pLastEntry->pNext;
        }

        OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_HttpsListener_AddToSendQueue: Append buffer list 0x%08X to connection 0x%08X.\n", a_pBufferList, pListenerConnection);

        pLastEntry->pNext = a_pBufferList;
    }

    if(a_uFlags & OPCUA_LISTENER_CLOSE_WHEN_DONE)
    {
        pListenerConnection->bCloseWhenDone = OpcUa_True;
    }

    if(a_uFlags & OPCUA_LISTENER_NO_RCV_UNTIL_DONE)
    {
        pListenerConnection->bNoRcvUntilDone = OpcUa_True;
    }

    OpcUa_Mutex_Unlock(pListenerConnection->Mutex);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_HttpsListener_AddStreamToSendQueue
 *===========================================================================*/
OpcUa_StatusCode OpcUa_HttpsListener_AddStreamToSendQueue(
    OpcUa_Listener*                 a_pListener,
    OpcUa_HttpsListener_Connection* a_pListenerConnection,
    OpcUa_OutputStream*             a_pOutputStream)
{
    OpcUa_BufferListElement* pEntry = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_HttpListener, "OpcUa_HttpsListener_AddStreamToSendQueue");

    pEntry = OpcUa_Alloc(sizeof(OpcUa_BufferListElement));
    OpcUa_GotoErrorIfAllocFailed(pEntry);

    do
    {
        if(pEntry != OpcUa_Null)
        {
            pEntry->pNext = OpcUa_Null;
            uStatus = a_pOutputStream->DetachBuffer((OpcUa_Stream*)a_pOutputStream, &pEntry->Buffer, OpcUa_Null);

            if(OpcUa_IsGood(uStatus))
            {
                uStatus = OpcUa_HttpsListener_AddToSendQueue(   a_pListener,
                                                                a_pListenerConnection,
                                                                pEntry,
                                                                0);
                if(OpcUa_IsBad(uStatus))
                {
                    /* free Entry */
                    OpcUa_Buffer_Clear(&pEntry->Buffer);
                    OpcUa_Free(pEntry);
                    break;
                }
                else
                {
                    /* continue with next entry */
                    pEntry = OpcUa_Alloc(sizeof(OpcUa_BufferListElement));
                }
            }
            else
            {
                /* free entry */
                OpcUa_Free(pEntry);

                if(OpcUa_IsEqual(OpcUa_BadNoData))
                {
                    /* mask error - everything went fine, all buffers detached */
                    uStatus = OpcUa_Good;
                }
                break;
            }
        }
        else
        {
            uStatus = OpcUa_BadOutOfMemory;
        }

    } while(OpcUa_IsGood(uStatus));

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_HttpsListener_EndSendResponse
 *===========================================================================*/
/* a bad status means, that the operation is to be abandoned */
OpcUa_StatusCode OpcUa_HttpsListener_EndSendResponse(
    OpcUa_Listener*         a_pListener,
    OpcUa_StatusCode        a_uStatus,
    OpcUa_OutputStream**    a_ppOutputStream)
{
    OpcUa_HttpsListener*             pHttpsListener         = OpcUa_Null;
    OpcUa_HttpsListener_Connection*  pListenerConnection    = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_HttpListener, "OpcUa_HttpsListener_EndSendResponse");

    OpcUa_ReturnErrorIfArgumentNull(a_pListener);
    OpcUa_ReturnErrorIfArgumentNull(a_ppOutputStream);
    OpcUa_ReturnErrorIfArgumentNull(*a_ppOutputStream);

    OpcUa_ReturnErrorIfArgumentNull(a_pListener->EndSendResponse);

    pHttpsListener = (OpcUa_HttpsListener*)a_pListener->Handle;

    OpcUa_HttpsStream_GetConnection(*a_ppOutputStream, (OpcUa_Handle*)&pListenerConnection);

    OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_HttpsListener_EndSendResponse: Status 0x%08X\n", a_uStatus);

    /* trigger error message */
    if(OpcUa_IsBad(a_uStatus))
    {
        /* create and send response */
        OpcUa_HttpsListener_SendImmediateResponse(  a_pListener,
                                                    pListenerConnection,
                                                    OPCUA_HTTP_STATUS_INTERNAL_SERVER_ERROR,
                                                    OPCUA_HTTP_STATUS_INTERNAL_SERVER_ERROR_TEXT,
                                                    "Server: OPC-ANSI-C-HTTPS-API/0.1\r\n",
                                                    OpcUa_Null,
                                                    0);

        OpcUa_HttpsListener_ProcessDisconnect(a_pListener, &pListenerConnection);

        /* delete without flushing and decrement request count */
        OpcUa_HttpsStream_Delete((OpcUa_Stream**)a_ppOutputStream);
    }
    else
    {
        /* close stream (flushes the Content on the wire). */
        uStatus = (*a_ppOutputStream)->Close((OpcUa_Stream*)*a_ppOutputStream);

        /* delete without flushing and decrement request count */
        OpcUa_HttpsStream_Delete((OpcUa_Stream**)a_ppOutputStream);

#if OPCUA_HTTPSLISTENER_CLOSE_SOCKET_AFTER_RESPONSE
        OpcUa_HttpsListener_ProcessDisconnect(a_pListener, pListenerConnection);
#else
        OpcUa_HttpsListener_ConnectionManager_ReleaseConnection( pHttpsListener->pConnectionManager,
                                                                &pListenerConnection);
#endif
    }

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_HttpsListener_AbortSendResponse
 *===========================================================================*/
/* a bad status means, that the operation is to be abandoned */
OpcUa_StatusCode OpcUa_HttpsListener_AbortSendResponse(
    OpcUa_Listener*         a_pListener,
    OpcUa_StatusCode        a_uStatus,
    OpcUa_String*           a_sReason,
    OpcUa_OutputStream**    a_ppOutputStream)
{
OpcUa_InitializeStatus(OpcUa_Module_HttpListener, "OpcUa_HttpsListener_AbortSendResponse");

    OpcUa_ReturnErrorIfArgumentNull(a_pListener);
    OpcUa_ReturnErrorIfArgumentNull(a_pListener->AbortSendResponse);

    if(a_ppOutputStream != OpcUa_Null)
    {
        /* clean up */
        OpcUa_HttpsStream_Delete((OpcUa_Stream**)a_ppOutputStream);
    }
    else
    {
        /* no insecure abort messages implemented and allowed! */
        OpcUa_ReferenceParameter(a_uStatus);
        OpcUa_ReferenceParameter(a_sReason);
    }

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_HttpsListener_LookForPendingMessage
 *===========================================================================*/
/* should be handled by the connection manager, since no interleaving is possible by design! */
static OpcUa_StatusCode OpcUa_HttpsListener_LookForPendingMessage(
    OpcUa_HttpsListener*    a_pHttpsListener,
    OpcUa_Socket            a_hSocket,
    OpcUa_InputStream**     a_ppInputStream)
{
    OpcUa_InputStream*  pInputStream    = OpcUa_Null;
    OpcUa_Socket        hSocket         = OpcUa_Null;

    OpcUa_DeclareErrorTraceModule(OpcUa_Module_HttpListener);

    OpcUa_ReturnErrorIfArgumentNull(a_pHttpsListener);
    OpcUa_ReturnErrorIfArgumentNull(a_hSocket);
    OpcUa_ReturnErrorIfArgumentNull(a_ppInputStream);

    OpcUa_List_Enter(a_pHttpsListener->pPendingMessages);

    *a_ppInputStream = OpcUa_Null;

    OpcUa_List_ResetCurrent(a_pHttpsListener->pPendingMessages);
    pInputStream = (OpcUa_InputStream*)OpcUa_List_GetCurrentElement(a_pHttpsListener->pPendingMessages);

    while(pInputStream != OpcUa_Null)
    {
        OpcUa_HttpsStream_GetSocket((OpcUa_Stream*)pInputStream, &hSocket);

        if(hSocket == a_hSocket)
        {
            /* found */
            OpcUa_List_DeleteElement(a_pHttpsListener->pPendingMessages, (OpcUa_Void*)pInputStream);
            *a_ppInputStream = pInputStream;
            OpcUa_List_Leave(a_pHttpsListener->pPendingMessages);
            return OpcUa_Good;
        }
        else
        {
            /* get next element */
            pInputStream = (OpcUa_InputStream*)OpcUa_List_GetNextElement(a_pHttpsListener->pPendingMessages);
        }
    }

    OpcUa_List_Leave(a_pHttpsListener->pPendingMessages);

    return OpcUa_BadNotFound;
}

/*============================================================================
 * OpcUa_HttpsListener_ProcessRequest
 *===========================================================================*/
/**
 * @brief Handles an HTTP message.
 */
static OpcUa_StatusCode OpcUa_HttpsListener_ProcessRequest(
    OpcUa_Listener*                 a_pListener,
    OpcUa_HttpsListener_Connection* a_pListenerConnection,
    OpcUa_InputStream**             a_ppInputStream)
{
    OpcUa_HttpsListener*        pHttpsListener  = OpcUa_Null;
    OpcUa_HttpsStream_Method    eHttpMethod     = OpcUa_HttpsStream_Method_Invalid;

OpcUa_InitializeStatus(OpcUa_Module_HttpListener, "ProcessRequest");

    OpcUa_ReturnErrorIfArgumentNull(a_pListener);
    OpcUa_ReturnErrorIfArgumentNull(a_pListenerConnection);
    OpcUa_ReturnErrorIfArgumentNull(a_ppInputStream);
    OpcUa_ReturnErrorIfArgumentNull(*a_ppInputStream);

    pHttpsListener     = (OpcUa_HttpsListener*)a_pListener->Handle;

    /* check method */
    uStatus = OpcUa_HttpsStream_GetMethod(  (OpcUa_Stream*)*a_ppInputStream,
                                            &eHttpMethod);
    OpcUa_GotoErrorIfBad(uStatus);

    switch(eHttpMethod)
    {
    case OpcUa_HttpsStream_Method_Post:
        {
            OpcUa_String HeaderValue = OPCUA_STRING_STATICINITIALIZER;

            /* check particular headers */
            uStatus = OpcUa_HttpsStream_GetHeader(  (OpcUa_Stream*)*a_ppInputStream,
                                                    OpcUa_String_FromCString("Content-Type"),
                                                    OpcUa_False,
                                                   &HeaderValue);
            if(     OpcUa_IsBad(uStatus)
                ||  OpcUa_String_IsEmpty(   &HeaderValue)
                ||  OpcUa_String_IsNull(    &HeaderValue)
                ||  OpcUa_String_StrnCmp(   &HeaderValue,
                                            OpcUa_String_FromCString("application/octet-stream"),
                                            OpcUa_StrLenA("application/octet-stream"),
                                            OpcUa_False)
              )
            {
                OpcUa_String_Clear(&HeaderValue);

                OpcUa_HttpsListener_SendImmediateResponse(  a_pListener,
                                                            (OpcUa_Handle)a_pListenerConnection,
                                                            OPCUA_HTTP_STATUS_UNSUPPORTED_MEDIA_TYPE,       /* "Unsupported Media Type" */
                                                            OPCUA_HTTP_STATUS_UNSUPPORTED_MEDIA_TYPE_TEXT,  /* reason phrase */
                                                            OpcUa_Null, /* no further headers */
                                                            OpcUa_Null, /* no content */
                                                            0);         /* no content */

                /* delete and close input stream */
                OpcUa_HttpsStream_Close((OpcUa_Stream*)(*a_ppInputStream));
                OpcUa_HttpsStream_Delete((OpcUa_Stream**)a_ppInputStream);
                OpcUa_HttpsListener_ProcessDisconnect(a_pListener, &a_pListenerConnection);

                /* Release reference to stream */
                OpcUa_HttpsListener_ConnectionManager_ReleaseConnection(pHttpsListener->pConnectionManager,
                                                                        &a_pListenerConnection);

                break;
            }

            OpcUa_String_Clear(&HeaderValue);

            /* send notification that request is ready to be read. */
            /* this call goes most probably to the endpoint handler. */
            if(pHttpsListener->pfListenerCallback != OpcUa_Null)
            {
                a_pListenerConnection->uNoOfRequestsTotal++;

                /* store security policy */
                OpcUa_HttpsStream_GetHeader(    (OpcUa_Stream*)*a_ppInputStream,
                                                OpcUa_String_FromCString(OPCUA_HTTPS_SECURITYPOLICYHEADER),
                                                OpcUa_True,
                                                &a_pListenerConnection->sSecurityPolicy);

                uStatus = pHttpsListener->pfListenerCallback(   a_pListener,                                        /* the event source          */
                                                                (OpcUa_Void*)pHttpsListener->pvListenerCallbackData,/* the callback data         */
                                                                OpcUa_ListenerEvent_Request,                        /* the event type            */
                                                                (OpcUa_Handle)a_pListenerConnection,                /* handle for the connection */
                                                                a_ppInputStream,                                    /* the ready input stream    */
                                                                OpcUa_Good);                                        /* event status code         */
            }
            else
            {
                /* delete and close input stream */
                OpcUa_HttpsStream_Close((OpcUa_Stream*)(*a_ppInputStream));
                OpcUa_HttpsStream_Delete((OpcUa_Stream**)a_ppInputStream);
                OpcUa_HttpsListener_ProcessDisconnect(a_pListener, &a_pListenerConnection);

                /* Release reference to stream */
                OpcUa_HttpsListener_ConnectionManager_ReleaseConnection(pHttpsListener->pConnectionManager,
                                                                        &a_pListenerConnection);
            }

            break;
        }
    case OpcUa_HttpsStream_Method_Head:
        {
            OpcUa_HttpsListener_SendImmediateResponse(  a_pListener,
                                                        (OpcUa_Handle)a_pListenerConnection,
                                                        OPCUA_HTTP_STATUS_NO_CONTENT,
                                                        OPCUA_HTTP_STATUS_NO_CONTENT_TEXT,
                                                        "Server: OPC-ANSI-C-HTTPS-API/0.1\r\n"
                                                        "Content-Type: application/octet-stream\r\n",
                                                        OpcUa_Null, /* no content */
                                                        0);         /* no content */

            /* delete and close input stream */
            OpcUa_HttpsStream_Close((OpcUa_Stream*)(*a_ppInputStream));
            OpcUa_HttpsStream_Delete((OpcUa_Stream**)a_ppInputStream);
            OpcUa_HttpsListener_ProcessDisconnect(a_pListener, &a_pListenerConnection);

            /* Release reference to stream */
            OpcUa_HttpsListener_ConnectionManager_ReleaseConnection(pHttpsListener->pConnectionManager,
                                                                    &a_pListenerConnection);

            break;
        }
    case OpcUa_HttpsStream_Method_Options:
        {
            OpcUa_HttpsListener_SendImmediateResponse(  a_pListener,
                                                        (OpcUa_Handle)a_pListenerConnection,
                                                        OPCUA_HTTP_STATUS_NOT_IMPLEMENTED,
                                                        OPCUA_HTTP_STATUS_NOT_IMPLEMENTED_TEXT,
                                                        OpcUa_Null, /* no further headers */
                                                        OpcUa_Null, /* no content */
                                                        0);         /* no content */

            /* delete and close input stream */
            OpcUa_HttpsStream_Close((OpcUa_Stream*)(*a_ppInputStream));
            OpcUa_HttpsStream_Delete((OpcUa_Stream**)a_ppInputStream);
            OpcUa_HttpsListener_ProcessDisconnect(a_pListener, &a_pListenerConnection);

            /* Release reference to stream */
            OpcUa_HttpsListener_ConnectionManager_ReleaseConnection(pHttpsListener->pConnectionManager,
                                                                    &a_pListenerConnection);

            break;
        }
    case OpcUa_HttpsStream_Method_Get:
        {
#if OPCUA_HTTPS_ALLOW_GET

            if(pHttpsListener->pfListenerCallback != OpcUa_Null)
            {
                a_pListenerConnection->uNoOfRequestsTotal++;

                /* store security policy */
                OpcUa_HttpsStream_GetHeader(    (OpcUa_Stream*)*a_ppInputStream,
                                                OpcUa_String_FromCString(OPCUA_HTTPS_SECURITYPOLICYHEADER),
                                                OpcUa_True,
                                                &a_pListenerConnection->sSecurityPolicy);

                uStatus = pHttpsListener->pfListenerCallback(   a_pListener,                                        /* the event source          */
                                                                (OpcUa_Void*)pHttpsListener->pvListenerCallbackData,/* the callback data         */
                                                                OpcUa_ListenerEvent_RawRequest,                     /* the event type            */
                                                                (OpcUa_Handle)a_pListenerConnection,                /* handle for the connection */
                                                                a_ppInputStream,                                    /* the ready input stream    */
                                                                OpcUa_Good);                                        /* event status code         */

                if(OpcUa_IsBad(uStatus))
                {
                    if(    uStatus != OpcUa_BadDisconnect
                        && uStatus != OpcUa_BadConnectionClosed
                        && uStatus != OpcUa_BadCommunicationError)
                    {
                        OpcUa_HttpsListener_SendImmediateResponse(  a_pListener,
                                                                    (OpcUa_Handle)a_pListenerConnection,
                                                                    OPCUA_HTTP_STATUS_NOT_FOUND,
                                                                    OPCUA_HTTP_STATUS_NOT_FOUND_TEXT,
                                                                    OpcUa_Null, /* no further headers */
                                                                    OpcUa_Null, /* no content */
                                                                    0);         /* no content */

                        if(*a_ppInputStream != OpcUa_Null)
                        {
                            /* delete and close input stream */
                            OpcUa_HttpsStream_Close((OpcUa_Stream*)(*a_ppInputStream));
                            OpcUa_HttpsStream_Delete((OpcUa_Stream**)a_ppInputStream);

                            /* Release reference to stream */
                            OpcUa_HttpsListener_ConnectionManager_ReleaseConnection(pHttpsListener->pConnectionManager,
                                                                                    &a_pListenerConnection);
                        }
                    }

                    OpcUa_HttpsListener_ProcessDisconnect(a_pListener, &a_pListenerConnection);

                    /* case handled */
                    uStatus = OpcUa_Good;
                }
            }
            else
            {
                /* delete and close input stream */
                OpcUa_HttpsStream_Close((OpcUa_Stream*)(*a_ppInputStream));
                OpcUa_HttpsStream_Delete((OpcUa_Stream**)a_ppInputStream);
                OpcUa_HttpsListener_ProcessDisconnect(a_pListener, &a_pListenerConnection);

                /* Release reference to stream */
                OpcUa_HttpsListener_ConnectionManager_ReleaseConnection(pHttpsListener->pConnectionManager,
                                                                        &a_pListenerConnection);
            }

#else /* OPCUA_HTTPS_ALLOW_GET */

            OpcUa_HttpsListener_SendImmediateResponse(  a_pListener,
                                                        (OpcUa_Handle)a_pListenerConnection,
                                                        OPCUA_HTTP_STATUS_NOT_IMPLEMENTED,
                                                        OPCUA_HTTP_STATUS_NOT_IMPLEMENTED_TEXT,
                                                        OpcUa_Null, /* no further headers */
                                                        OpcUa_Null, /* no content */
                                                        0);         /* no content */

            /* delete and close input stream */
            OpcUa_HttpsStream_Close((OpcUa_Stream*)(*a_ppInputStream));
            OpcUa_HttpsStream_Delete((OpcUa_Stream**)a_ppInputStream);
            OpcUa_HttpsListener_ProcessDisconnect(a_pListener, &a_pListenerConnection);

            /* Release reference to stream */
            OpcUa_HttpsListener_ConnectionManager_ReleaseConnection(pHttpsListener->pConnectionManager,
                                                                    &a_pListenerConnection);

#endif /* OPCUA_HTTPS_ALLOW_GET */
            break;
        }
    default:
        {
            OpcUa_HttpsListener_SendImmediateResponse(  a_pListener,
                                                        (OpcUa_Handle)a_pListenerConnection,
                                                        OPCUA_HTTP_STATUS_INTERNAL_SERVER_ERROR,
                                                        OPCUA_HTTP_STATUS_INTERNAL_SERVER_ERROR_TEXT,
                                                        "Server: OPC-ANSI-C-HTTPS-API/0.1\r\n"
                                                        "Content-Type: application/octet-stream\r\n",
                                                        OpcUa_Null, /* no content */
                                                        0);         /* no content */

            /* delete and close input stream */
            OpcUa_HttpsStream_Close((OpcUa_Stream*)(*a_ppInputStream));
            OpcUa_HttpsStream_Delete((OpcUa_Stream**)a_ppInputStream);
            OpcUa_HttpsListener_ProcessDisconnect(a_pListener, &a_pListenerConnection);

            /* Release reference to stream */
            OpcUa_HttpsListener_ConnectionManager_ReleaseConnection(pHttpsListener->pConnectionManager,
                                                                    &a_pListenerConnection);

            break;
        }
    }

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}


/*============================================================================
 * OpcUa_HttpsListener_EventHandler Type
 *===========================================================================*/
OpcUa_StatusCode OpcUa_HttpsListener_ProcessDisconnect(
    OpcUa_Listener*                   a_pListener,
    OpcUa_HttpsListener_Connection**  a_ppListenerConnection)
{
    OpcUa_HttpsListener*            pHttpsListener      = OpcUa_Null;
    OpcUa_HttpsListener_Connection* pListenerConnection = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_HttpListener, "ProcessDisconnect");

    OpcUa_ReturnErrorIfArgumentNull(a_pListener);
    OpcUa_ReturnErrorIfArgumentNull(a_ppListenerConnection);

    pHttpsListener      = (OpcUa_HttpsListener*)a_pListener;
    pListenerConnection = (OpcUa_HttpsListener_Connection*)(*a_ppListenerConnection);

    OpcUa_ReturnErrorIfArgumentNull(pListenerConnection);

    /* now, that the upper layers are informed, we can safely remove the resources for the broken connection. */
    uStatus = OpcUa_HttpsListener_ConnectionManager_RemoveConnection(pHttpsListener->pConnectionManager,
                                                                     pListenerConnection);

    if(OpcUa_IsBad(uStatus))
    {
        OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR,
                    "OpcUa_HttpsListener_ProcessDisconnect: "
                    "Client connection 0x%08X already removed!\n",
                    pListenerConnection);

        uStatus = OpcUa_Good;
        OpcUa_ReturnStatusCode;
    }

    OpcUa_Mutex_Lock(pListenerConnection->Mutex);

    OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR,
                "OpcUa_HttpsListener_ProcessDisconnect: "
                "Connection 0x%08X with socket 0x%08X reported as lost!\n",
                pListenerConnection,
                pListenerConnection->Socket);


    if(pListenerConnection->bConnected == OpcUa_False)
    {
        OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR,
                    "OpcUa_HttpsListener_ProcessDisconnect: "
                    "Client connection 0x%08X with socket 0x%08X already set to disconnected!\n",
                    pListenerConnection,
                    pListenerConnection->Socket);

        uStatus = OpcUa_Good;

        OpcUa_Mutex_Unlock(pListenerConnection->Mutex);

        OpcUa_ReturnStatusCode;
    }

    pListenerConnection->bConnected = OpcUa_False;
    pListenerConnection->uDisconnectTime = OpcUa_GetTickCount();

    if(pListenerConnection->Socket != OpcUa_Null)
    {
        OPCUA_P_SOCKET_CLOSE(pListenerConnection->Socket);
        pListenerConnection->Socket = OpcUa_Null;
    }

    /* notify about successful closing of the listener */
    pHttpsListener->pfListenerCallback( a_pListener,                            /* the source of the event       */
                                        pHttpsListener->pvListenerCallbackData, /* the callback data             */
                                        OpcUa_ListenerEvent_ChannelClosed,      /* the event that occured        */
                                        (OpcUa_Handle)pListenerConnection,      /* the handle for the connection */
                                        OpcUa_Null,                             /* the non existing stream       */
                                        OpcUa_Good);                            /* status                        */

    if(pListenerConnection->pOutputStream != OpcUa_Null)
    {
        OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_WARNING,
                    "OpcUa_HttpsListener_ProcessDisconnect: "
                    "Deleting client connection 0x%08X with socket 0x%08X and active stream %p!\n",
                    pListenerConnection,
                    pListenerConnection->Socket,
                    pListenerConnection->pOutputStream);
    }
    else
    {
        OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR,
                    "OpcUa_HttpsListener_ProcessDisconnect: "
                    "Deleting client connection 0x%08X with socket %p!\n",
                    pListenerConnection,
                    pListenerConnection->Socket);
    }

    OpcUa_Mutex_Unlock(pListenerConnection->Mutex);

    OpcUa_HttpsListener_ConnectionManager_ReleaseConnection( pHttpsListener->pConnectionManager,
                                                             a_ppListenerConnection);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_HttpsListener_WriteEventHandler
 *===========================================================================*/
/**
* @brief Gets called if data can be written to the socket.
*/
OpcUa_StatusCode OpcUa_HttpsListener_WriteEventHandler(
    OpcUa_Listener* a_pListener,
    OpcUa_Socket*   a_hSocket)
{
    OpcUa_HttpsListener*             pHttpsListener         = OpcUa_Null;
    OpcUa_HttpsListener_Connection*  pListenerConnection    = OpcUa_Null;
    OpcUa_OutputStream*              pOutputStream          = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_HttpListener, "WriteEventHandler");

    OpcUa_ReturnErrorIfArgumentNull(a_pListener);
    OpcUa_ReturnErrorIfArgumentNull(a_pListener->Handle);
    OpcUa_ReturnErrorIfArgumentNull(a_hSocket);

    pHttpsListener = (OpcUa_HttpsListener*)a_pListener->Handle;

    /******************************************************************************************************/

    /* look if an active connection is available for the socket. */
    uStatus = OpcUa_HttpsListener_ConnectionManager_GetConnectionBySocket(pHttpsListener->pConnectionManager,
                                                                          a_hSocket,
                                                                          &pListenerConnection);
    if(OpcUa_IsBad(uStatus) && OpcUa_IsNotEqual(OpcUa_BadNotFound))
    {
        /* no connection available */
        OpcUa_GotoError;
    }

    /******************************************************************************************************/

    /* look for pending output stream */
    if(pListenerConnection != OpcUa_Null)
    {
        pOutputStream = pListenerConnection->pOutputStream;
    }
    /******************************************************************************************************/

    if(pOutputStream != OpcUa_Null && pOutputStream->Handle != OpcUa_Null)
    {
        OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_WARNING,
                    "OpcUa_HttpsListener_WriteEventHandler: "
                    "called without active stream\n");
        /* unregister write event */

#if 0 /* for future extensions */
        OPCUA_P_SOCKET_CHANGEEVENTLIST(pHttpsOutputStream->Socket, OPCUA_SOCKET_WRITE_EVENT);
#endif
    }

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_HttpsListener_EventHandler Type
 *===========================================================================*/
/** @brief Internal handler prototype. */
typedef OpcUa_StatusCode (*OpcUa_HttpsListener_EventHandler)(OpcUa_Listener*  a_pListener,
                                                             OpcUa_Socket     a_hSocket);

/*============================================================================
 * OpcUa_HttpsListener_NeedBufferEventHandler
 *===========================================================================*/
/**
* @brief TEST IMPLEMENTATION!
*/
OpcUa_StatusCode OpcUa_HttpsListener_NeedBufferEventHandler(
    OpcUa_Listener* a_pListener,
    OpcUa_Socket    a_hSocket)
{
    OpcUa_Byte*     pBuffer = OpcUa_Null;
    OpcUa_UInt32    nLength = 0;

OpcUa_InitializeStatus(OpcUa_Module_HttpListener, "NeedBufferEventHandler");

    OpcUa_ReferenceParameter(a_pListener);

    /******************************************************************************************************/

    /* todo: handle 0? */
    pBuffer = (OpcUa_Byte*)OpcUa_Alloc(OpcUa_ProxyStub_g_Configuration.iTcpListener_DefaultChunkSize);

    uStatus = OPCUA_P_SOCKET_READ(a_hSocket,
                                  pBuffer,
                                  OpcUa_ProxyStub_g_Configuration.iTcpListener_DefaultChunkSize,
                                  &nLength);

    OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR,
                "OpcUa_HttpsListener_NeedBufferEventHandler: "
                "Socket 0x%08x requested buffer! (0x%08X)\n",
                a_hSocket,
                uStatus);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_HttpsListener_FreeBufferEventHandler
 *===========================================================================*/
/**
* @brief
*/
OpcUa_StatusCode OpcUa_HttpsListener_FreeBufferEventHandler(
    OpcUa_Listener* a_pListener,
    OpcUa_Socket    a_hSocket)
{
    OpcUa_HttpsListener*              pHttpsListener        = OpcUa_Null;
    OpcUa_HttpsListener_Connection*   pListenerConnection   = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_HttpListener, "FreeBufferEventHandler");

    OpcUa_ReturnErrorIfArgumentNull(a_pListener);

    pHttpsListener = (OpcUa_HttpsListener*)a_pListener->Handle;

    /* look if an active connection is available for the socket. */
    uStatus = OpcUa_HttpsListener_ConnectionManager_GetConnectionBySocket(pHttpsListener->pConnectionManager,
                                                                          a_hSocket,
                                                                          &pListenerConnection);
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
 * OpcUa_HttpsListener_ReadEventHandler
 *===========================================================================*/
/**
 * @brief Gets called if data is available on the socket.
 */
OpcUa_StatusCode OpcUa_HttpsListener_ReadEventHandler(
    OpcUa_Listener* a_pListener,
    OpcUa_Socket    a_hSocket)
{
    OpcUa_InputStream*                pInputStream           = OpcUa_Null;
    OpcUa_HttpsListener*              pHttpsListener          = OpcUa_Null;
    OpcUa_HttpsListener_Connection*   pListenerConnection    = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_HttpListener, "ReadEventHandler");

    OpcUa_ReturnErrorIfArgumentNull(a_pListener);
    OpcUa_ReturnErrorIfArgumentNull(a_hSocket);
    OpcUa_ReturnErrorIfArgumentNull(a_pListener->Handle)

    pHttpsListener = (OpcUa_HttpsListener *)a_pListener->Handle;

    /******************************************************************************************************/

    /* look if an active connection is available for the socket. */
    uStatus = OpcUa_HttpsListener_ConnectionManager_GetConnectionBySocket(  pHttpsListener->pConnectionManager,
                                                                            a_hSocket,
                                                                            &pListenerConnection);
    if(OpcUa_IsBad(uStatus))
    {
        OPCUA_P_SOCKET_CLOSE(a_hSocket);
        OpcUa_GotoError;
    }

    /* get last access timestamp */
    pListenerConnection->uLastReceiveTime =  OpcUa_GetTickCount();

    /* look if a pending message for this socket exists */
    uStatus = OpcUa_HttpsListener_LookForPendingMessage(pHttpsListener, a_hSocket, &pInputStream);
    if(OpcUa_IsBad(uStatus) && OpcUa_IsNotEqual(OpcUa_BadNotFound))
    {
        /* something unexpected happened */
        OpcUa_GotoError;
    }

    /******************************************************************************************************/

    /* create stream if no one was found */
    if(pInputStream == OpcUa_Null)
    {
        uStatus = OpcUa_HttpsStream_CreateInput(a_hSocket, OpcUa_HttpsStream_MessageType_Request, &pInputStream);
        OpcUa_ReturnErrorIfBad(uStatus);
    }
    else
    {
        /* release reference from pending message list */
        OpcUa_HttpsListener_ConnectionManager_ReleaseConnection(pHttpsListener->pConnectionManager,
                                                                &pListenerConnection);
    }

    /******************************************************************************************************/

    /* now, we have a stream -> read the available data; further processing takes place in the callback */
    uStatus = OpcUa_HttpsStream_DataReady(pInputStream);

    /******************************************************************************************************/

    if(OpcUa_IsEqual(OpcUa_GoodCallAgain))
    {
        /* prepare to append further data later */
        OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR,
                    "OpcUa_HttpsListener_ReadEventHandler: "
                    "CallAgain result for stream %d on socket 0x%08X!\n",
                    pInputStream,
                    a_hSocket);

        /* store the stream in the temporary list -> reference count is not decremented in this case! */
        OpcUa_List_Enter(pHttpsListener->pPendingMessages);
        uStatus = OpcUa_List_AddElement(pHttpsListener->pPendingMessages, pInputStream);
        OpcUa_List_Leave(pHttpsListener->pPendingMessages);

        /* bind stream to connection */
        pListenerConnection->pInputStream       = pInputStream;
        /* set data received timestamp */
        pListenerConnection->uLastReceiveTime   = OpcUa_GetTickCount();
    }
    else /* process message */
    {
        if(OpcUa_IsBad(uStatus))
        {
            OpcUa_StringA   sError          = OpcUa_Null;
            OpcUa_UInt32    uHttpStatus     = OPCUA_HTTP_STATUS_BAD_REQUEST;
            OpcUa_StringA   sHttpStatusText = OPCUA_HTTP_STATUS_BAD_REQUEST_TEXT;

            /* Error happened... */
            switch(uStatus)
            {
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
                case OpcUa_BadDecodingError:
                {
                    sError = (OpcUa_CharA*)"OpcUa_BadDecodingError";
                    break;
                }
                case OpcUa_BadEncodingError:
                {
                    sError = (OpcUa_CharA*)"OpcUa_BadEncodingError";
                    break;
                }
                case OpcUa_BadEncodingLimitsExceeded:
                {
                    sError = (OpcUa_CharA*)"OpcUa_BadEncodingLimitsExceeded";
                    break;
                }
                case OpcUa_BadHttpMethodNotAllowed:
                {
                    sError          = (OpcUa_CharA*)"OpcUa_BadHttpMethodNotAllowed";
                    uHttpStatus     = OPCUA_HTTP_STATUS_METHOD_NOT_ALLOWED;
                    sHttpStatusText = OPCUA_HTTP_STATUS_METHOD_NOT_ALLOWED_TEXT;
                    break;
                }
                default:
                {
                    sError = (OpcUa_CharA*)"unmapped";
                }
            }

            if(    uStatus != OpcUa_BadDisconnect
                && uStatus != OpcUa_BadConnectionClosed
                && uStatus != OpcUa_BadCommunicationError)
            {
                /* Malformed request has been received, notify the client. */
                OpcUa_HttpsListener_SendImmediateResponse(  a_pListener,
                                                            (OpcUa_Handle)pListenerConnection,
                                                            uHttpStatus,
                                                            sHttpStatusText,
                                                            "Server: OPC-ANSI-C-HTTPS-API/0.1\r\n",
                                                            OpcUa_Null,
                                                            0);
            }

            OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_HttpsListener_ReadEventHandler: socket %p; status 0x%08X (%s)\n", a_hSocket, uStatus, sError);
            OPCUA_P_SOCKET_CLOSE(a_hSocket);
            OpcUa_HttpsStream_Close((OpcUa_Stream*)pInputStream);
            OpcUa_HttpsStream_Delete((OpcUa_Stream**)&pInputStream);

            if(pListenerConnection != OpcUa_Null)
            {
                pListenerConnection->pInputStream = OpcUa_Null;

                /* Notify about connection loss. */
                OpcUa_HttpsListener_ConnectionManager_ReleaseConnection(pHttpsListener->pConnectionManager,
                                                                        &pListenerConnection);

                OpcUa_HttpsListener_ProcessDisconnect(a_pListener, &pListenerConnection);
            }
        }
        else /* Message can be processed. */
        {
            OpcUa_HttpsStream_MessageType eMessageType = OpcUa_HttpsStream_MessageType_Unknown;
            OpcUa_HttpsStream_GetMessageType((OpcUa_Stream*)pInputStream, &eMessageType);

            /* set data received timestamp */
            pListenerConnection->uLastReceiveTime   = OpcUa_GetTickCount();

            /* process message */
            if(eMessageType == OpcUa_HttpsStream_MessageType_Request)
            {
                if(pListenerConnection != OpcUa_Null)
                {
                    /* unlink the now complete message from the temporary link */
                    pListenerConnection->pInputStream = OpcUa_Null;

                    uStatus = OpcUa_HttpsListener_ProcessRequest(a_pListener, pListenerConnection, &pInputStream);

                    if(pInputStream != OpcUa_Null)
                    {
						OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_HttpsListener_ReadEventHandler: InputStream wasn't correctly released! Deleting it!\n", uStatus);
                        OpcUa_HttpsStream_Close((OpcUa_Stream*)pInputStream);
                        OpcUa_HttpsStream_Delete((OpcUa_Stream**)&pInputStream);

                        /* release reference between stream and connection */
                        OpcUa_HttpsListener_ConnectionManager_ReleaseConnection(pHttpsListener->pConnectionManager,
                                                                                &pListenerConnection);
                    }

                    if(OpcUa_IsBad(uStatus))
                    {
                        OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_WARNING, "OpcUa_HttpsListener_ReadEventHandler: Process Request returned an error (0x%08X)!\n", uStatus);

                        if(pHttpsListener->bShutdown == OpcUa_False)
                        {
                            if(OpcUa_IsEqual(OpcUa_BadDisconnect))
                            {
                                OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_WARNING, "OpcUa_HttpsListener_ReadEventHandler: Closing socket because handler returned OpcUa_BadDisconnect!\n");
                            }
                            else
                            {
                                /* send error message */
                                OpcUa_HttpsListener_SendImmediateResponse(  a_pListener,
                                                                            (OpcUa_Handle)pListenerConnection,
                                                                            OPCUA_HTTP_STATUS_INTERNAL_SERVER_ERROR,
                                                                            OPCUA_HTTP_STATUS_INTERNAL_SERVER_ERROR_TEXT,
                                                                            "Server: OPC-ANSI-C-HTTPS-API/0.1\r\n"
                                                                            "Content-Type: application/octet-stream\r\n",
                                                                            OpcUa_Null,
                                                                            0);

                                OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_WARNING, "OpcUa_HttpsListener_ReadEventHandler: Closing socket (0x%08X)!\n", uStatus);
                            }

                            OpcUa_HttpsListener_ProcessDisconnect(a_pListener, &pListenerConnection);
                        }
                    }
                }
                else /* no connection object for this socket */
                {
                    OpcUa_Socket hSocket = OpcUa_Null;

                    OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_WARNING, "OpcUa_HttpsListener_ReadEventHandler: Received request for nonexisting connection!\n");

                    OpcUa_HttpsStream_GetSocket((OpcUa_Stream*)pInputStream, &hSocket);
                    OPCUA_P_SOCKET_CLOSE(hSocket);

                    OpcUa_HttpsStream_Close((OpcUa_Stream*)pInputStream);
                    OpcUa_HttpsStream_Delete((OpcUa_Stream**)&pInputStream);
                }
            }
            else
            {
                OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_HttpsListener_ReadEventHandler: Invalid MessageType (%d)\n", eMessageType);

                OpcUa_HttpsStream_Close((OpcUa_Stream*)pInputStream);
                OpcUa_HttpsStream_Delete((OpcUa_Stream**)&pInputStream);
            }
        }
    } /* if(OpcUa_IsEqual(OpcUa_GoodCallAgain)) */

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_HttpsListener_AcceptEventHandler
 *===========================================================================*/
/**
 * @brief Gets called if remote node has connected to this socket.
 */
OpcUa_StatusCode OpcUa_HttpsListener_AcceptEventHandler(
    OpcUa_Listener* a_pListener,
    OpcUa_Socket    a_hSocket)
{
    OpcUa_HttpsListener*             pHttpsListener         = OpcUa_Null;
    OpcUa_HttpsListener_Connection*  pListenerConnection    = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_HttpListener, "AcceptEventHandler");

    OpcUa_ReturnErrorIfArgumentNull(a_hSocket);
    OpcUa_ReturnErrorIfArgumentNull(a_pListener);
    OpcUa_ReturnErrorIfArgumentNull(a_pListener->Handle);

    pHttpsListener = (OpcUa_HttpsListener *)a_pListener->Handle;

#if OPCUA_HTTPLISTENER_USEEXTRAMAXCONNSOCKET
    {
        OpcUa_UInt32 uConnectionCount = 0;

        uStatus = OpcUa_HttpsListener_ConnectionManager_GetConnectionCount(pHttpsListener->pConnectionManager,
                                                                          &uConnectionCount);

        OpcUa_GotoErrorIfTrue(uConnectionCount >= OPCUA_HTTPLISTENER_MAXCONNECTIONS, OpcUa_BadMaxConnectionsReached);
    }
#endif /* OPCUA_HTTPLISTENER_USEEXTRAMAXCONNSOCKET */

    /* check, if there is already a connection with this object */
    OpcUa_HttpsListener_ConnectionManager_GetConnectionBySocket(    pHttpsListener->pConnectionManager,
                                                                    a_hSocket,
                                                                   &pListenerConnection);

    /* if no connection exists create a new one */
    if(pListenerConnection == OpcUa_Null)
    {
        /* create and add a new connection object for the accepted connection  */
        uStatus = OpcUa_HttpsListener_Connection_Create(&pListenerConnection);
        OpcUa_GotoErrorIfBad(uStatus);

        pListenerConnection->Socket           = a_hSocket;
        pListenerConnection->pListenerHandle  = (OpcUa_Listener*)a_pListener;
        pListenerConnection->uLastReceiveTime = OpcUa_GetTickCount();

        uStatus = OPCUA_P_SOCKET_GETPEERINFO(a_hSocket, (OpcUa_CharA*)&(pListenerConnection->achPeerInfo), OPCUA_P_PEERINFO_MIN_SIZE);
        if(OpcUa_IsGood(uStatus))
        {
            /* Give some debug information. */
            OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR,
                        "OpcUa_HttpsListener_AcceptEventHandler: Transport connection 0x%08X from %s accepted on socket %X!\n",
                        pListenerConnection,
                        pListenerConnection->achPeerInfo,
                        pListenerConnection->Socket);
        }
        else
        {
            OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_WARNING, "OpcUa_HttpsListener_AcceptEventHandler: Could not retrieve connection information for socket %X!\n", pListenerConnection->Socket);
        }

        uStatus = OpcUa_HttpsListener_ConnectionManager_AddConnection(pHttpsListener->pConnectionManager, pListenerConnection);
        OpcUa_GotoErrorIfBad(uStatus);
    }

    pListenerConnection->bConnected = OpcUa_True;

    pHttpsListener->pfListenerCallback( a_pListener,                            /* the source of the event       */
                                        pHttpsListener->pvListenerCallbackData, /* the callback data             */
                                        OpcUa_ListenerEvent_ChannelOpened,      /* the event that occured        */
                                        (OpcUa_Handle)pListenerConnection,      /* the handle for the connection */
                                        OpcUa_Null,                             /* the non existing stream       */
                                        OpcUa_Good);                            /* status                        */

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    if(pListenerConnection != OpcUa_Null)
    {
        /* ignore result; it doesnt matter, if it was not yet registered */
        OpcUa_HttpsListener_Connection_Delete(&pListenerConnection);
    }

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_HttpsListener_EventCallback
 *===========================================================================*/
OpcUa_StatusCode OpcUa_HttpsListener_EventCallback(
    OpcUa_Socket    a_hSocket,
    OpcUa_UInt32    a_uSocketEvent,
    OpcUa_Void*     a_pUserData,
    OpcUa_UInt16    a_uPortNumber,
    OpcUa_Boolean   a_bIsSSL)
{
    OpcUa_StringA                    strEvent       = OpcUa_Null;
    OpcUa_HttpsListener*             pHttpsListener = OpcUa_Null;
    OpcUa_HttpsListener_EventHandler fEventHandler  = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_HttpListener, "SocketEventCallback");

    OpcUa_ReturnErrorIfArgumentNull(a_hSocket);
    OpcUa_ReturnErrorIfArgumentNull(a_pUserData);
    OpcUa_ReturnErrorIfArgumentNull(((OpcUa_Listener*)a_pUserData)->Handle);

    OpcUa_ReferenceParameter(a_bIsSSL);
    OpcUa_ReferenceParameter(a_uPortNumber);

    pHttpsListener = (OpcUa_HttpsListener*)((OpcUa_Listener*)a_pUserData)->Handle;

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
    OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, " * OpcUa_HttpsListener_EventCallback: Socket(%x), Port(%d), Data(%d), Event(%s)\n", a_hSocket, a_uPortNumber, a_pUserData, strEvent);
    /* debug code end */
#endif

    switch(a_uSocketEvent)
    {
        case OPCUA_SOCKET_READ_EVENT:
        {
            /* notifies an existing stream about new data or creates a new stream */
            fEventHandler = OpcUa_HttpsListener_ReadEventHandler;
            break;
        }
        case OPCUA_SOCKET_EXCEPT_EVENT:
        {
            break;
        }
        case OPCUA_SOCKET_WRITE_EVENT:
        {
            /* fEventHandler = OpcUa_HttpsListener_WriteEventHandler */
            break;
        }
        case OPCUA_SOCKET_NEED_BUFFER_EVENT:
        {
            fEventHandler = OpcUa_HttpsListener_NeedBufferEventHandler;
            break;
        }
        case OPCUA_SOCKET_FREE_BUFFER_EVENT:
        {
            fEventHandler = OpcUa_HttpsListener_FreeBufferEventHandler;
            break;
        }
        case OPCUA_SOCKET_ACCEPT_EVENT:
        {
            fEventHandler = OpcUa_HttpsListener_AcceptEventHandler;
            break;
        }
        case OPCUA_SOCKET_NO_EVENT:
        case OPCUA_SOCKET_TIMEOUT_EVENT:
        case OPCUA_SOCKET_CLOSE_EVENT:
        case OPCUA_SOCKET_SHUTDOWN_EVENT:
        {
            break;
        }
        default:
        {
            /* unexpected error, report to upper layer. */
            pHttpsListener->pfListenerCallback( (OpcUa_Listener*)a_pUserData,                       /* the event source */
                                                (OpcUa_Void*)pHttpsListener->pvListenerCallbackData,/* the callback data */
                                                OpcUa_ListenerEvent_UnexpectedError,                /* the event that occured */
                                                OpcUa_Null,                                         /* a connection handle */
                                                OpcUa_Null,                                         /* the input stream for the event (none in this case) */
                                                uStatus);                                           /* a status code for the event */

            break;
        }
    }

    /* call the internal specialized event handler */
    if(fEventHandler != OpcUa_Null)
    {
        uStatus = fEventHandler((OpcUa_Listener*)a_pUserData, a_hSocket);
    }

    OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, " * OpcUa_HttpsListener_EventCallback: Event Handler returned.\n");

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_HttpsListener_ConnectionCloseCallback
 *===========================================================================*/
 /** @brief Callback function for the Connection Manager on connection deletion.
  *
  *  @param Listener The listener the http connection belongs to.
  *  @param HttpConnection The http connection that is being deleted.
  */
OpcUa_Void OpcUa_HttpsListener_ConnectionDeleteCallback(
    OpcUa_Listener*                  a_pListener,
    OpcUa_HttpsListener_Connection*  a_pListenerConnection)
{
    OpcUa_ReferenceParameter(a_pListener);

    OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR,
                "OpcUa_HttpsListener_ConnectionDeleteCallback: Connection to peer %s (socket %X) gets closed!\n",
                a_pListenerConnection->achPeerInfo,
                a_pListenerConnection->Socket);

    if(a_pListenerConnection->Socket != OpcUa_Null)
    {
        OPCUA_P_SOCKET_CLOSE(a_pListenerConnection->Socket);
        a_pListenerConnection->Socket = OpcUa_Null;
    }

    return;
}

/*============================================================================
 * OpcUa_HttpsListener_Close
 *===========================================================================*/
OpcUa_StatusCode OpcUa_HttpsListener_Close(OpcUa_Listener* a_pListener)
{
    OpcUa_HttpsListener*    pHttpsListener  = OpcUa_Null;
    OpcUa_InputStream*      pInputStream    = OpcUa_Null;

    OpcUa_DeclareErrorTraceModule(OpcUa_Module_HttpListener);

    OpcUa_ReturnErrorIfArgumentNull(a_pListener);
    OpcUa_ReturnErrorIfInvalidObject(OpcUa_HttpsListener, a_pListener, Close);

    pHttpsListener       = (OpcUa_HttpsListener*)a_pListener->Handle;

    /* lock connection and close the socket. */
    OpcUa_Mutex_Lock(pHttpsListener->Mutex);

    /* mark listener as being in shutdown mode; certain calls are no longer accepted. */
    pHttpsListener->bShutdown = OpcUa_True;

    /* check if already stopped */
    if(pHttpsListener->Socket != OpcUa_Null)
    {
        /* only close listening socket, which should be in the global list. */
        OPCUA_P_SOCKET_CLOSE(pHttpsListener->Socket);
        pHttpsListener->Socket = OpcUa_Null;
    }

    /* cleanup all connections */
    OpcUa_HttpsListener_ConnectionManager_RemoveConnections(pHttpsListener->pConnectionManager);

#if OPCUA_MULTITHREADED

    OpcUa_Mutex_Unlock(pHttpsListener->Mutex);

    /* check if socket list handle is valid */
    if(pHttpsListener->SocketManager != OpcUa_Null)
    {
        /* stops the thread and closes socket */
        OPCUA_P_SOCKETMANAGER_DELETE(&(pHttpsListener->SocketManager));
    }

    /* lock connection and close the socket. */
    OpcUa_Mutex_Lock(pHttpsListener->Mutex);

#endif /* OPCUA_MULTITHREADED */

    OpcUa_List_Enter(pHttpsListener->pPendingMessages);
    OpcUa_List_ResetCurrent(pHttpsListener->pPendingMessages);
    pInputStream = (OpcUa_InputStream *)OpcUa_List_GetCurrentElement(pHttpsListener->pPendingMessages);
    while(pInputStream != OpcUa_Null)
    {
        OpcUa_List_DeleteCurrentElement(pHttpsListener->pPendingMessages);
        pInputStream->Close((OpcUa_Stream*)pInputStream);
        pInputStream->Delete((OpcUa_Stream**)&pInputStream);
        pInputStream = (OpcUa_InputStream *)OpcUa_List_GetCurrentElement(pHttpsListener->pPendingMessages);
    }
    OpcUa_List_Leave(pHttpsListener->pPendingMessages);

    OpcUa_Mutex_Unlock(pHttpsListener->Mutex);

    pHttpsListener->pfSecureChannelCallback(    0,                                           /* channel id - this should be a reserved one */
                                                eOpcUa_HttpsListener_SecureChannelClose,     /* event type */
                                                OpcUa_Good,                                  /* event status */
              /* to be removed */               OpcUa_Null,                                  /* client certificate */
              /* to be removed */               OpcUa_Null,                                  /* security policy */
              /* to be removed */               0,                                           /* message security mode */
                                                0,
                                                pHttpsListener->pvSecureChannelCallbackData);/* callback data */

    /* notify about successful closing of the listener */
    pHttpsListener->pfListenerCallback( a_pListener,                            /* the source of the event       */
                                        pHttpsListener->pvListenerCallbackData, /* the callback data             */
                                        OpcUa_ListenerEvent_Close,              /* the event that occured        */
                                        OpcUa_Null,                             /* the handle for the connection */
                                        OpcUa_Null,                             /* the non existing stream       */
                                        OpcUa_Good);                            /* status                        */

    return OpcUa_Good;
}

/*============================================================================
 * OpcUa_HttpsListener_Open
 *===========================================================================*/
OpcUa_StatusCode OpcUa_HttpsListener_Open(
    OpcUa_Listener*             a_pListener,
    OpcUa_String*               a_sUrl,
    OpcUa_Listener_PfnOnNotify* a_pfnCallback,
    OpcUa_Void*                 a_pCallbackData)
{
    OpcUa_HttpsListener* pHttpsListener         = OpcUa_Null;
    OpcUa_UInt32         uSocketManagerFlags    = OPCUA_SOCKET_NO_FLAG;

OpcUa_InitializeStatus(OpcUa_Module_HttpListener, "Open");

    OpcUa_ReturnErrorIfArgumentNull(a_pListener);
    OpcUa_ReturnErrorIfArgumentNull(a_sUrl);

    OpcUa_ReturnErrorIfInvalidObject(OpcUa_HttpsListener, a_pListener, Open);

    pHttpsListener = (OpcUa_HttpsListener*)a_pListener->Handle;
    OpcUa_ReturnErrorIfArgumentNull(pHttpsListener);

    if(OpcUa_ProxyStub_g_Configuration.bTcpListener_ClientThreadsEnabled != OpcUa_False)
    {
        uSocketManagerFlags |= OPCUA_SOCKET_SPAWN_THREAD_ON_ACCEPT | OPCUA_SOCKET_REJECT_ON_NO_THREAD;
    }

    pHttpsListener->bShutdown = OpcUa_False;

    /********************************************************************/

    /* lock listener while thread is starting */
    OpcUa_Mutex_Lock(pHttpsListener->Mutex);

    /* check if thread already started */
    if(pHttpsListener->Socket != OpcUa_Null)
    {
        OpcUa_Mutex_Unlock(pHttpsListener->Mutex);
        return OpcUa_BadInvalidState;
    }

    pHttpsListener->pfListenerCallback     = a_pfnCallback;
    pHttpsListener->pvListenerCallbackData = a_pCallbackData;

    /********************************************************************/

    /* start up socket handling for this listener */
#if OPCUA_MULTITHREADED
    /* check if socket list handle not yet set */
    if(pHttpsListener->SocketManager != OpcUa_Null)
    {
        OpcUa_Mutex_Unlock(pHttpsListener->Mutex);
        return OpcUa_BadInvalidState;
    }

    uStatus = OPCUA_P_SOCKETMANAGER_CREATE(&(pHttpsListener->SocketManager),
                                           OPCUA_HTTPSLISTENER_MAXCONNECTIONS + 1, /* add one for listen socket */
                                           uSocketManagerFlags);
    OpcUa_GotoErrorIfBad(uStatus);

    uStatus = OPCUA_P_SOCKETMANAGER_CREATESERVER(pHttpsListener->SocketManager,
                                                 OpcUa_String_GetRawString(a_sUrl),
                                                 pHttpsListener->pCertificate,
                                                 pHttpsListener->pPrivateKey,
                                                 pHttpsListener->pPKIConfig,
                                                 OpcUa_HttpsListener_EventCallback,
                                                 (OpcUa_Void*)a_pListener,
                                                 &(pHttpsListener->Socket));

#else /* OPCUA_MULTITHREADED */

    /* single thread socket created on global socket manager */
    uStatus = OPCUA_P_SOCKETMANAGER_CREATESERVER(OpcUa_Null,
                                                 OpcUa_String_GetRawString(a_sUrl),
                                                 pHttpsListener->pCertificate,
                                                 pHttpsListener->pPrivateKey,
                                                 pHttpsListener->pPKIConfig,
                                                 OpcUa_HttpsListener_EventCallback,
                                                 (OpcUa_Void*)a_pListener,
                                                 &(pHttpsListener->Socket));

#endif /* OPCUA_MULTITHREADED */
    OpcUa_GotoErrorIfBad(uStatus);

    /********************************************************************/

    /* notify about successful opening of the listener */
    pHttpsListener->pfListenerCallback( a_pListener,                            /* source of the event       */
                                        pHttpsListener->pvListenerCallbackData, /* callback data             */
                                        OpcUa_ListenerEvent_Open,               /* event that occured        */
                                        OpcUa_Null,                             /* handle for the connection */
                                        OpcUa_Null,                             /* non existing stream       */
                                        OpcUa_Good);                            /* event status              */

    /* immediately create reserved securechannel */
    pHttpsListener->pfSecureChannelCallback(    0,                                          /* channel id - this should be a reserved one */
                                                eOpcUa_HttpsListener_SecureChannelOpen,     /* event type */
                                                OpcUa_Good,                                 /* event status */
              /* to be removed */               OpcUa_Null,                                 /* client certificate */
              /* to be removed */               OpcUa_Null,                                 /* security policy */
              /* to be removed */               0,                                          /* message security mode */
                                                0,
                                                pHttpsListener->pvSecureChannelCallbackData);/* callback data */

    OpcUa_Mutex_Unlock(pHttpsListener->Mutex);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    OpcUa_HttpsListener_Close(a_pListener);

    OpcUa_Mutex_Unlock(pHttpsListener->Mutex);

OpcUa_FinishErrorHandling;
}

#endif /* OPCUA_HAVE_HTTPSAPI */
#endif /* OPCUA_HAVE_SERVERAPI */
