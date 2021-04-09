/* ========================================================================
 * Copyright (c) 2005-2009 The OPC Foundation, Inc. All rights reserved.
 *
 * OPC Foundation MIT License 1.00
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * The complete license agreement can be found here:
 * http://opcfoundation.org/License/MIT/1.00/
 * ======================================================================*/
#define OPCUA_TCPLISTENER_CONNECTIONMANAGER_HANDLE_GET(xConnection) ((OpcUa_Handle)xConnection->iConnectionHandle)
/*==============================================================================*/
/* OpcUa_TcpListener_Connection                                                 */
/*==============================================================================*/
/**
* @brief Symbolizes a single client connection for the binary protocol layer.
*/
struct _OpcUa_TcpListener_Connection
{
    /** @brief The socket to the client. */
    OpcUa_Socket        Socket;
#if OPCUA_P_SOCKETGETPEERINFO_V2
    /** @brief Information about the peer. */
    OpcUa_CharA         achPeerInfo[OPCUA_P_PEERINFO_MIN_SIZE];
#else /* OPCUA_P_SOCKETGETPEERINFO_V2 */
    /** @brief The IP address of the client. */
    OpcUa_UInt32        PeerIp;
    /** @brief The port for the socket. */
    OpcUa_UInt16        PeerPort;
#endif /* OPCUA_P_SOCKETGETPEERINFO_V2 */
    /** @brief The time when the connection was made. */
    OpcUa_DateTime      ConnectTime;
    /** @brief True if the object can be deleted. */
    OpcUa_Boolean       bDelete;
    /** @brief True, as long as the connection is established. */
    OpcUa_Boolean       bConnected;
    /** @brief The size of the incoming buffer. */
    OpcUa_UInt32        ReceiveBufferSize;
    /** @brief The size of the outgoing buffer. */
    OpcUa_UInt32        SendBufferSize;
    /** @brief Backlink to the listener which hosts the connection. */
    OpcUa_Void*         pListenerHandle;
    /** @brief Holds a reference to a not fully received stream message. */
    OpcUa_InputStream*  pInputStream;
    /** @brief Holds a reference to a not fully sent stream message. */
    //OpcUa_OutputStream* pOutputStream;
    /** @brief Mutex for granting mutually exlcusive access to the connection object */
#if OPCUA_USE_SYNCHRONISATION
    OpcUa_Mutex         Mutex;
#endif /* OPCUA_USE_SYNCHRONISATION */
    /** @brief Number of request being issued over this connection. */
    OpcUa_UInt32        uNoOfRequestsTotal;
    /** @brief The maximum message size accepted by this connection. */
    OpcUa_UInt32        MaxMessageSize;
    /** @brief The maximum number of chunks per message accepted by this connection. */
    OpcUa_UInt32        MaxChunkCount;
    /** @brief The current number of chunks in an message. If 0, the connection is waiting for the next message. */
    OpcUa_UInt32        uCurrentChunk;
    /** @brief URL supplied by the client during transport handshake. */
    OpcUa_String        sURL;
    /** @brief The version of the binary protocol used over this connection. */
    OpcUa_UInt32        uProtocolVersion;
    /** @brief The queued list of data blocks to be sent. */
    OpcUa_BufferListElement* pSendQueue;
    /** @brief Should this connection close when the send completes. */
    OpcUa_Boolean            bCloseWhenDone;
    /** @brief Should this connection block the receiver until the send completes. */
    OpcUa_Boolean            bNoRcvUntilDone;
    /** @brief Tells wether data has been delayed because of bNoRcvUntilDone. */
    OpcUa_Boolean            bRcvDataPending;
    OpcUa_Boolean       bUsed;
    OpcUa_Boolean       bValid;
    OpcUa_Int           iConnectionHandle;
};
typedef struct _OpcUa_TcpListener_Connection OpcUa_TcpListener_Connection;

/** @brief Allocate and initialize an TcpListener_Connection. */
//OpcUa_StatusCode        OpcUa_TcpListener_Connection_Create(
//    OpcUa_TcpListener_Connection**   ppConnection);

/** @brief Initialize an TcpListener_Connection. */
OpcUa_StatusCode        OpcUa_TcpListener_Connection_Initialize(
    OpcUa_TcpListener_Connection*    pValue);

/** @brief Clear and free an TcpListener_Connection. */
//OpcUa_Void              OpcUa_TcpListener_Connection_Delete(
//    OpcUa_TcpListener_Connection**   pValue);

/** @brief Clear an TcpListener_Connection. */
OpcUa_Void              OpcUa_TcpListener_Connection_Clear(
    OpcUa_TcpListener_Connection*    pValue);

/*==============================================================================*/
/* OpcUa_TcpListener_ConnectionManager                                              */
/*==============================================================================*/
/**
* @brief Being part of a specific TcpListener, it manages the ressources for all clients connected to an endpoint.
*/
struct _OpcUa_TcpListener_ConnectionManager
{
#if OPCUA_USE_SYNCHRONISATION
    OpcUa_Mutex                     pMutex;
#endif /* OPCUA_USE_SYNCHRONISATION */
    OpcUa_UInt32                    uUsedConnections;
    /** @brief A list with current connections of type OpcUa_TcpListener_Connection. */
    OpcUa_TcpListener_Connection    Connections[OPCUA_TCPLISTENER_MAXCONNECTIONS];
    /** @brief Backlink to the listener to which the connection manager belongs to. */
    OpcUa_Listener*                 Listener;
};

typedef struct _OpcUa_TcpListener_ConnectionManager OpcUa_TcpListener_ConnectionManager;

typedef OpcUa_Void (*OpcUa_TcpListener_ConnectionDeleteCB)( 
    OpcUa_Listener*                         pListener,
    OpcUa_TcpListener_Connection*           pTcpConnection);

/* @brief Allocate and initialize a new connection. */
OpcUa_StatusCode        OpcUa_TcpListener_ConnectionManager_Create(
    OpcUa_TcpListener_ConnectionManager**   ppConnectionManager);

/* @brief Initialize an allocated connection. */
OpcUa_StatusCode        OpcUa_TcpListener_ConnectionManager_Initialize(
    OpcUa_TcpListener_ConnectionManager*    ConnectionManager);

/* @brief Clear the internals of a connection. */
OpcUa_Void              OpcUa_TcpListener_ConnectionManager_Clear(
    OpcUa_TcpListener_ConnectionManager*    ConnectionManager);

/* @brief Clear and delete a connection. */
OpcUa_Void              OpcUa_TcpListener_ConnectionManager_Delete(
    OpcUa_TcpListener_ConnectionManager**   ppConnectionManager);

/* @brief Add a new connection object to the list of managed connections. */
//OpcUa_StatusCode        OpcUa_TcpListener_ConnectionManager_AddConnection(
//    OpcUa_TcpListener_ConnectionManager*    ConnectionManager, 
//    OpcUa_TcpListener_Connection*           Connection);

/* @brief Retrieve the connection object identified by the socket. */
OpcUa_StatusCode        OpcUa_TcpListener_ConnectionManager_GetConnectionBySocket(
    OpcUa_TcpListener_ConnectionManager*    ConnectionManager,
    OpcUa_Socket                            Socket,
    OpcUa_TcpListener_Connection**          Connection);

/* @brief Retrieve the connection object identified by the socket. */
OpcUa_StatusCode        OpcUa_TcpListener_ConnectionManager_GetConnectionByHandle(
    OpcUa_TcpListener_ConnectionManager*    ConnectionManager,
    OpcUa_Handle                            ConnectionHandle,
    OpcUa_TcpListener_Connection**          Connection);

/* @brief Remove a connection identified by the connection object itself (if no id was assigned ie. pre validation) */
//OpcUa_StatusCode        OpcUa_TcpListener_ConnectionManager_RemoveConnection(
//    OpcUa_TcpListener_ConnectionManager*    ConnectionManager, 
//    OpcUa_TcpListener_Connection*           pConnection);

/* @brief Remove all connections managed by the listener and call the given function for everyone. */
OpcUa_StatusCode        OpcUa_TcpListener_ConnectionManager_RemoveConnections(
    OpcUa_TcpListener_ConnectionManager*    ConnectionManager, 
    OpcUa_TcpListener_ConnectionDeleteCB    ConnectionDeleteCB);
OpcUa_StatusCode OpcUa_TcpListener_ConnectionManager_CreateConnection(
    OpcUa_TcpListener_ConnectionManager*    a_pConnectionManager, 
    OpcUa_TcpListener_Connection**          a_ppConnection);

OpcUa_Void OpcUa_TcpListener_ConnectionManager_DeleteConnection(
    OpcUa_TcpListener_ConnectionManager*    a_pConnectionManager, 
    OpcUa_TcpListener_Connection**          a_ppConnection);
/* @brief Get the number of connections added to the connection manager. */
OpcUa_StatusCode        OpcUa_TcpListener_ConnectionManager_GetConnectionCount(
    OpcUa_TcpListener_ConnectionManager*    ConnectionManager, 
    OpcUa_UInt32*                           pNoOfConnections);
