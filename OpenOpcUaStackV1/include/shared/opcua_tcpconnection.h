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

/* HINT: This implementation will become a opaque handle! */

#ifndef _OpcUa_TcpConnection_H_
#define _OpcUa_TcpConnection_H_ 1

#include <opcua_connection.h>

OPCUA_BEGIN_EXTERN_C

#define OPCUA_TCPCONNECTION_DEFAULT_PROTOCOLV   ((OpcUa_UInt32)1)    /* Version 1.0 */
#define OPCUA_TCPCONNECTION_DEFAULT_LIFETIME    ((OpcUa_UInt32)600)  /* 10 minutes */

/* default value is 1, the alternative is a research implementation, which needs to be tested! */
#define OPCUA_TCPCONNECTION_DELETE_REQUEST_STREAM 1

/*============================================================================
 * OpcUa_SecureConnectionState
 *===========================================================================*/
 /** @brief The states a TcpConnection can be in. */
typedef enum _OpcUa_TcpConnectionState
{
    /** @brief Error state. */
    OpcUa_TcpConnectionState_Invalid,
    /** @brief Connection object connecting. */
    OpcUa_TcpConnectionState_Connecting,
    /** @brief Connection is established, communication to the server is possible. */
    OpcUa_TcpConnectionState_Connected,
    /** @brief The session was closed gracefully with a disconnect message. */
    OpcUa_TcpConnectionState_Disconnected,
    /** @brief An error message was received and the connection is inactive. */
    OpcUa_TcpConnectionState_Error    
} OpcUa_TcpConnectionState;


/*============================================================================
 * OpcUa_TcpConnection
 *===========================================================================*/
/** @brief Holds all data needed to manage a tcp binary connection to an ua server. */
typedef struct _OpcUa_TcpConnection
{
    /** @brief Internal helper to verify instances. */
    OpcUa_UInt32                    SanityCheck;
    /** @brief The state of the connection to the server. */
    OpcUa_TcpConnectionState        ConnectionState;
    /** @brief The socket holding the connection to the server. */
    OpcUa_Socket                    Socket; 
    /** @brief Messaging events to the holder of this connection. */
    OpcUa_Connection_PfnOnNotify*   NotifyCallback;
    /** @brief Data to pass back with the callback. */
    OpcUa_Void*                     CallbackData;
    /** @brief Synchronizing access to this connection. */
    OpcUa_Mutex                     Mutex;
    /** @brief An active datastream being received (message). */
    OpcUa_InputStream*              IncomingStream;
    /** @brief An active datastream being sent (message). */
    OpcUa_OutputStream*             OutgoingStream;
#if !OPCUA_TCPCONNECTION_DELETE_REQUEST_STREAM
    OpcUa_Boolean                   bOutgoingStreamIsUsed;
#endif /* !OPCUA_TCPCONNECTION_DELETE_REQUEST_STREAM */
    /** @brief The time when the connection was established. */
    OpcUa_DateTime                  ConnectTime;        
    /** @brief The time when the client disconnected. */
    OpcUa_DateTime                  DisconnectTime;     
    /** @brief The buffer size for receiving data on this connection. */
    OpcUa_UInt32                    ReceiveBufferSize;
    /** @brief The buffer size for sending data over this connection. */
    OpcUa_UInt32                    SendBufferSize;
    /** @brief The maximum message size accepted by this connection. */
    OpcUa_UInt32                    MaxMessageSize;
    /** @brief The maximum number of chunks per message accepted by this connection. */
    OpcUa_UInt32                    MaxChunkCount;
    /** @brief There was a limit violation and the connection is waiting for the last chunk to reset. */
    OpcUa_Boolean                   bChunkOverflow;
    /** @brief The current number of chunks in an message. If 0, the connection is waiting for the next message. */
    OpcUa_UInt32                    uCurrentChunk;
    OpcUa_String                    sURL;
#if OPCUA_MULTITHREADED
    /*! @brief Holds the socket for this connection, the thread and is the central waiting point. */
    OpcUa_SocketManager             SocketManager;
#endif /* OPCUA_MULTITHREADED */
    /*! @brief The protocol version used for this connection. */
    OpcUa_UInt32                    uProtocolVersion;
    /*! @brief Private key, if the underlying transport is secured. */
    OpcUa_Key                       PrivateKey;
    /** @brief The queued list of data blocks to be sent. */
    OpcUa_BufferListElement*        pSendQueue;
}
OpcUa_TcpConnection;

/*============================================================================
 * OpcUa_TcpConnection_Create
 *===========================================================================*/
OpcUa_StatusCode OpcUa_TcpConnection_Create(OpcUa_Connection** connection);

/*============================================================================
 * OpcUa_TcpConnection_SanityCheck
 *===========================================================================*/
#define OpcUa_TcpConnection_SanityCheck 0x4FCC07CB

/*============================================================================
 * OpcUa_ReturnErrorIfInvalidConnection
 *===========================================================================*/
#define OpcUa_ReturnErrorIfInvalidConnection(xConnection) \
if (((OpcUa_TcpConnection*)xConnection->Handle)->SanityCheck != OpcUa_TcpConnection_SanityCheck) \
{ \
    return OpcUa_BadInvalidArgument; \
}

OPCUA_END_EXTERN_C

#endif /* _OpcUa_TcpConnection_H_ */
