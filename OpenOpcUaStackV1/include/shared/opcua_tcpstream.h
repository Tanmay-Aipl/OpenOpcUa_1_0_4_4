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

#ifndef _OpcUa_TcpStream_H_
#define _OpcUa_TcpStream_H_ 1

#include <opcua_stream.h>

OPCUA_BEGIN_EXTERN_C

/*============================================================================
 * OpcUa_TcpStream_MessageType
 *===========================================================================*/
/** @brief The length of the tcp protocol message signature. (ie. HELF) */
#define OPCUA_TCPSTREAM_SIGNATURE_LENGTH        4

/*============================================================================
 * OpcUa_TcpStream_MessageType
 *===========================================================================*/
#define OPCUA_TCPSTREAM_MESSAGELENGTH_LENGTH    sizeof(OpcUa_UInt32)

/*============================================================================
 * OpcUa_TcpStream_MessageType
 *===========================================================================*/
#define OPCUA_TCPSTREAM_MESSAGEHEADER_LENGTH    (OPCUA_TCPSTREAM_SIGNATURE_LENGTH + OPCUA_TCPSTREAM_MESSAGELENGTH_LENGTH)

/*============================================================================
 * OpcUa_TcpStream_MessageType
 *===========================================================================*/
/** @brief Message signatures for protocol commands. */
#define OPCUA_TCPSTREAM_SIGNATURE_HELLO         "HELF"
#define OPCUA_TCPSTREAM_SIGNATURE_ACKNOWLEDGE   "ACKF"
#define OPCUA_TCPSTREAM_SIGNATURE_ERROR         "ERRF"

#define OPCUA_TCPSTREAM_SIGNATURE_STARTBYTES    "HAE"

/*============================================================================
 * OpcUa_TcpStream_MessageType
 *===========================================================================*/
/** @brief Message signatures for securechannel messages. Those are 3 byte signatures */
#define OPCUA_TCPSTREAM_SIGNATURE_SECURECHANNEL_OPEN        "OPN"
#define OPCUA_TCPSTREAM_SIGNATURE_SECURECHANNEL_CLOSE       "CLO"
#define OPCUA_TCPSTREAM_SIGNATURE_SECURECHANNEL_MESSAGE     "MSG"

#define OPCUA_TCPSTREAM_SIGNATURE_SECURECHANNEL_STARTBYTES  "OCM"

/*============================================================================
 * OpcUa_TcpStream_NotifyDisconnect
 *===========================================================================*/
/** @brief Notify the owner about a tcp disconnect event. (ie. during write) */
typedef OpcUa_Void (OpcUa_TcpStream_PfnNotifyDisconnect)(OpcUa_Handle hConnection);

/*============================================================================
 * OpcUa_TcpStream_MessageType
 *===========================================================================*/
/** @brief Types for binary protocol messages. */
enum _OpcUa_TcpStream_MessageType
{
    /** @brief Unknown Message Type */
    OpcUa_TcpStream_MessageType_Unknown,

    /** @brief Invalid Message Type */
    OpcUa_TcpStream_MessageType_Invalid,

    /** @brief Establishes a new connection with the server. This message is only sent by the client. */
    OpcUa_TcpStream_MessageType_Hello,
    
    /** @brief Acknowledges a new virtual connection. This message is only sent by the server. */
    OpcUa_TcpStream_MessageType_Acknowledge,

    /** @brief Tells the client, that the last message contained an error. */
    OpcUa_TcpStream_MessageType_Error,

    /** @brief All messages */
    OpcUa_TcpStream_MessageType_SecureChannel

};
typedef enum _OpcUa_TcpStream_MessageType OpcUa_TcpStream_MessageType;

/*============================================================================
 * OpcUa_TcpStream_State
 *===========================================================================*/
/** @brief Types for binary protocol messages. */
enum _OpcUa_TcpStream_State
{
    /** @brief Stream has no data. */
    OpcUa_TcpStream_State_Empty,
    /** @brief The signature has begun but not finished. */
    OpcUa_TcpStream_State_HeaderStarted,
    /** @brief All headerdata has been received. */
    OpcUa_TcpStream_State_HeaderComplete,
    /** @brief The complete message has been received. */
    OpcUa_TcpStream_State_MessageComplete
};

typedef enum _OpcUa_TcpStream_State OpcUa_TcpStream_State;

/*============================================================================
 * OpcUa_TcpInputStream
 *===========================================================================*/
/** @brief Private data structure for an OpcUa_Stream that allows reading from
  * a socket.
  */
struct _OpcUa_TcpInputStream
{
    /** @brief Inherited Fields from OpcUa_InputStream. @see OpcUa_InputStream */
    OpcUa_Int32                         Type;
    OpcUa_Handle                        Handle;
    OpcUa_Stream_PfnGetPosition*        GetPosition;
    OpcUa_Stream_PfnSetPosition*        SetPosition;
    OpcUa_Stream_PfnGetChunkLength*     GetChunkLength;
    OpcUa_Stream_PfnDetachBuffer*       DetachBuffer;
    OpcUa_Stream_PfnAttachBuffer*       AttachBuffer;
    OpcUa_Stream_PfnClose*              Close;
    OpcUa_Stream_PfnDelete*             Delete;
    OpcUa_Boolean                       CanSeek;
    OpcUa_Stream_PfnRead*               Read;
    OpcUa_Boolean                       NonBlocking;

    /* Subclass Fields */
    /** @brief Check type of interface implementation. */
    OpcUa_UInt32                        SanityCheck;
    /** @brief Holds the type of the received message chunk. @see OpcUa_TcpStream_MessageType */
    OpcUa_TcpStream_MessageType         MessageType;
    /** @brief The length of the message as parsed from the message header. */
    OpcUa_UInt32                        MessageLength; 
    /** @brief The communication handle with which the message was received from. */
    OpcUa_Socket                        Socket;
    /** @brief Tells wether the stream is closed for reading. */
    OpcUa_Boolean                       Closed;
    /** @brief The function to call if the stream has new data. */
    OpcUa_Stream_PfnOnReadyToRead*      Callback;
    /** @brief Data to return with the given callback function. */
    OpcUa_Void*                         CallbackData;
    /** @brief The current state of the stream. @see OpcUa_TcpStream_State */
    OpcUa_TcpStream_State               State;
    /** @brief The size of the internal buffer. */
    OpcUa_UInt32                        BufferSize;
    /** @brief The internal buffer. */
    OpcUa_Buffer                        Buffer;
    /** @brief True if the stream contains the last chunk of a message. */
    OpcUa_Boolean                       IsFinal;
    /** @brief True if the stream contains an abort message. */
    OpcUa_Boolean                       IsAbort;
};
typedef struct _OpcUa_TcpInputStream OpcUa_TcpInputStream;

/*============================================================================
 * OpcUa_TcpOutputStream
 *===========================================================================*/
/** @brief Private data structure for an OpcUa_Stream that allows writing to
  * a socket.
  */
struct _OpcUa_TcpOutputStream
{
    /** @brief Inherited Fields from OpcUa_OutputStream. @see OpcUa_OutputStream */
    OpcUa_Int32                         Type;
    OpcUa_Handle                        Handle;
    OpcUa_Stream_PfnGetPosition*        GetPosition;
    OpcUa_Stream_PfnSetPosition*        SetPosition;
    OpcUa_Stream_PfnGetChunkLength*     GetChunkLength;
    OpcUa_Stream_PfnDetachBuffer*       DetachBuffer;
    OpcUa_Stream_PfnAttachBuffer*       AttachBuffer;
    OpcUa_Stream_PfnClose*              Close;
    OpcUa_Stream_PfnDelete*             Delete;
    OpcUa_Boolean                       CanSeek;
    OpcUa_Stream_PfnWrite*              Write;
    OpcUa_Stream_PfnFlush*              Flush;

    /* Subclass (Tcp) Fields */
    /** @brief Check type of interface implementation. */
    OpcUa_UInt32                        SanityCheck;
    /** @brief Holds the type of the received message chunk. @see OpcUa_TcpStream_MessageType */
    OpcUa_TcpStream_MessageType         MessageType;
    /** @brief The communication handle with which the message is sent. */
    OpcUa_Socket                        Socket;
    /** @brief Tells wether the stream is closed for reading. */
    OpcUa_Boolean                       Closed;
    /** @brief The function to call if the stream is ready for new data. */
    OpcUa_Stream_PfnOnReadyToWrite*     Callback;
    /** @brief The function to call if the stream has new data. */
    OpcUa_Void*                         CallbackData;
    /** @brief Handle of the underlying connection. */
    OpcUa_Void*                         hConnection;
    /** @brief Size of the internal message buffer. */
    OpcUa_UInt32                        BufferSize;
    /** @brief The internal message buffer. */
    OpcUa_Buffer                        Buffer;
    /** @brief Number of times this stream has flushed its buffer. */
    OpcUa_UInt32                        NoOfFlushes;
    /** @brief Maximum number of times this stream may flush its buffer. */
    OpcUa_UInt32                        MaxNoOfFlushes;
    /** @brief Disconnect notification callback. */
    OpcUa_TcpStream_PfnNotifyDisconnect* NotifyDisconnect;
};
typedef struct _OpcUa_TcpOutputStream OpcUa_TcpOutputStream;

/*============================================================================
 * OpcUa_TcpStream_CreateInput
 *===========================================================================*/
/** @brief Allocates a new stream to read a message from a socket.
 *  @param socket [in]  The socket to read from.
 *  @param istrm  [in]  The buffer size of the stream.
 *  @param istrm  [out] The new input stream.
 */
OPCUA_EXPORT OpcUa_StatusCode OpcUa_TcpStream_CreateInput(
    OpcUa_Socket        hSocket,
    OpcUa_UInt32        uBufferSize,
    OpcUa_InputStream** istrm);

/*============================================================================
 * OpcUa_TcpStream_CreateOutput
 *===========================================================================*/
/** @brief Allocates a new stream to write a message to a socket.
 *  @param hSocket        [in]  The socket to write to.
 *  @param eMessageType   [in]  The type of the message, used for signature.
 *  @param ppAttachBuffer [bi]  A buffer to use for writing. Nulled on return.
 *  @param uiBufferSize   [in]  The size of the receiving buffer.
 *  @param pfnDisconnectCB[in]  Function called when the stream recognizes a disconnect during flush.
 *  @param uMaxNoOfFlushes[in]  Maximum number of flushes allowed.
 *  @param ostrm          [out] The new output stream.
 */
OPCUA_EXPORT OpcUa_StatusCode OpcUa_TcpStream_CreateOutput(
    OpcUa_Socket                        hSocket,
    OpcUa_TcpStream_MessageType         eMessageType,
    OpcUa_Byte**                        ppAttachBuffer,
    OpcUa_UInt32                        uiBufferSize,
    OpcUa_TcpStream_PfnNotifyDisconnect pfnDisconnectCB,
    OpcUa_UInt32                        uMaxNoOfFlushes,
    OpcUa_OutputStream**                ppOstrm);

/*============================================================================
 * OpcUa_Stream_Read
 *===========================================================================*/
/** @brief Read data from the streams internal buffer. */
OpcUa_StatusCode OpcUa_TcpStream_Read(
    OpcUa_InputStream*              istrm, 
    OpcUa_Byte*                     buffer, 
    OpcUa_UInt32*                   count,
    OpcUa_Stream_PfnOnReadyToRead*  callback, /* not supported in this implementation! */ 
    OpcUa_Void*                     callbackData);

/*============================================================================
 * OpcUa_Stream_Write
 *===========================================================================*/
/** @brief Write the given data into the stream. This operation may cause 
  * one or more flushes of the buffered data to the underlying socket if 
  * the given data is larger than the free buffer space or larger than
  * the buffer itself.
  */
OpcUa_StatusCode OpcUa_TcpStream_Write(
    OpcUa_OutputStream* ostrm, 
    OpcUa_Byte*         buffer, 
    OpcUa_UInt32        count);

/*============================================================================
 * OpcUa_Stream_Flush
 *===========================================================================*/
/** @brief Send the buffered data to the socket. */
OpcUa_StatusCode OpcUa_TcpStream_Flush(
    OpcUa_OutputStream* ostrm,
    OpcUa_Boolean       lastCall);

/*============================================================================
 * OpcUa_Stream_Close
 *===========================================================================*/
/** @brief Closes the stream. To be called before delete. Causes the sending
 *  of the buffered data if the parameter is an output stream.
 */
OpcUa_StatusCode OpcUa_TcpStream_Close(OpcUa_Stream* strm);

/*============================================================================
 * OpcUa_Stream_Delete
 *===========================================================================*/
/** @brief Delete the stream and all associated ressources. */
OpcUa_Void OpcUa_TcpStream_Delete(OpcUa_Stream** strm);

/*============================================================================
 * OpcUa_Stream_GetPosition
 *===========================================================================*/
/** @brief Get the position of the internal read/write pointer. */
OpcUa_StatusCode OpcUa_TcpStream_GetPosition(
    OpcUa_Stream* strm, 
    OpcUa_UInt32* position);

/*============================================================================
 * OpcUa_Stream_SetPosition
 *===========================================================================*/
/** @brief Set position of the internal read/write pointer. */
OpcUa_StatusCode OpcUa_TcpStream_SetPosition(
    OpcUa_Stream* strm, 
    OpcUa_UInt32 position);

/*============================================================================
 * OpcUa_TcpStream_DataReady
 *===========================================================================*/
/** @brief A lower layer tells the stream, that a read operation is possible. */
OpcUa_StatusCode OpcUa_TcpStream_DataReady(OpcUa_InputStream* istrm);

/*============================================================================
 * OpcUa_TcpStream_DetachBuffer
 *===========================================================================*/
/** @brief Detach data from the stream object. */
OpcUa_StatusCode OpcUa_TcpStream_DetachBuffer(  OpcUa_Stream*   pStream,
                                                OpcUa_Buffer*   pBuffer,
                                                OpcUa_Boolean*  pbMoreData);

/*============================================================================
 * OpcUa_ReturnErrorIfInvalidStream
 *===========================================================================*/
/** @brief check instance */
#define OpcUa_ReturnErrorIfInvalidStream(xStrm, xMethod) \
if (((((OpcUa_TcpInputStream*)(xStrm->Handle))->SanityCheck != OpcUa_TcpInputStream_SanityCheck) && (((OpcUa_TcpOutputStream*)(xStrm->Handle))->SanityCheck != OpcUa_TcpOutputStream_SanityCheck)) || xStrm->xMethod != OpcUa_TcpStream_##xMethod) \
{ \
    return OpcUa_BadInvalidArgument; \
}

OPCUA_END_EXTERN_C

#endif /* _OpcUa_TcpStream_H_ */
