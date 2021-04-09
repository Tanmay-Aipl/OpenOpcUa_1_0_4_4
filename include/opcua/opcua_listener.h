/* ========================================================================
 * Copyright (c) 2005-2011 The OPC Foundation, Inc. All rights reserved.
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

#ifndef _OpcUa_Listener_H_
#define _OpcUa_Listener_H_ 1

#include <opcua_stream.h>

OPCUA_BEGIN_EXTERN_C

/** @brief uFlags parameters for AddToSendQueue */
#define OPCUA_LISTENER_CLOSE_WHEN_DONE        0x01
#define OPCUA_LISTENER_NO_RCV_UNTIL_DONE      0x02

struct _OpcUa_Listener;

/** 
  @brief The types of events that could be reported by a listener.
*/
typedef enum _OpcUa_ListenerEvent
{
    /*! @brief Invalid default. */
    OpcUa_ListenerEvent_Invalid,

    /*! @brief The listener is now open and is ready to receive messages. */
    OpcUa_ListenerEvent_Open,

    /*! @brief The listener is now closed. */
    OpcUa_ListenerEvent_Close,

    /*! @brief A channel managed by the listener has been opened. */
    OpcUa_ListenerEvent_ChannelOpened,

    /*! @brief A channel managed by the listener has been closed. */
    OpcUa_ListenerEvent_ChannelClosed,

    /*! @brief A request is available for processing. */
    OpcUa_ListenerEvent_Request,

    /*! @brief A request is available for processing. */
    OpcUa_ListenerEvent_RawRequest,

    /*! @brief A partial request is available for processing. */
    OpcUa_ListenerEvent_RequestPartial,

    /*! @brief A message currently being received was aborted. */
    OpcUa_ListenerEvent_RequestAbort,

    /** @brief A message currently being sent was finished. */
    OpcUa_ListenerEvent_AsyncWriteComplete,

    /*! @brief An unexpected error occurred and the listener is no longer useable. */
    OpcUa_ListenerEvent_UnexpectedError
}
OpcUa_ListenerEvent;

/**
 * @brief Associates a supported security policy with message security modes.
 */
struct _OpcUa_Listener_SecurityPolicyConfiguration
{
    /** @brief The URI of a supported security policy. */
    OpcUa_String        sSecurityPolicy;
    /** @brief The message security modes allowed for the security policy. (bitmask) */
    OpcUa_UInt16        uMessageSecurityModes;
    /** @brief The client certificate, if provided. */
    OpcUa_ByteString*   pbsClientCertificate;
};

typedef struct _OpcUa_Listener_SecurityPolicyConfiguration OpcUa_Listener_SecurityPolicyConfiguration;

/** 
  @brief Called by the listener to report an listener event.

  A listener may have many active connections which each have a unique handle. If a connection
  is closed, the listener calls this function and passes a bad status indicating why the 
  connection was closed. If the listener itself encounters problems that prevent it from 
  receiving incoming connections the the listener calls this function with the connection 
  handle set to NULL and a status indicating what the problem. 
 
  @param pListener        [in] The listener.
  @param pCallbackData    [in] The callback data specifed when the server created the listener.
  @param eEvent           [in] The event that occurred.
  @param hConnection      [in] An opaque handle that identifies the source of the message.
  @param ppIstrm          [in] The input stream that must be used to read the request.
  @param uOperationStatus [in] The status associated with the event.
*/
typedef OpcUa_StatusCode (OpcUa_Listener_PfnOnNotify)(
    struct _OpcUa_Listener* pListener,
    OpcUa_Void*             pCallbackData,
    OpcUa_ListenerEvent     eEvent,
    OpcUa_Handle            hConnection,
    OpcUa_InputStream**     ppIstrm,
    OpcUa_StatusCode        uOperationStatus);

/** 
  @brief Starts listening for incoming messages.
 
  @param pListener     [in] The listener to open.
  @param sUrl          [in] URL of the endpoint to listen on.
  @param pCallback     [in] The callback to use when a message arrives.
  @param pCallbackData [in] The data to return with the callback.
*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_Listener_Open(
    struct _OpcUa_Listener*     pListener,
    OpcUa_String*               sUrl,
    OpcUa_Listener_PfnOnNotify* pCallback,
    OpcUa_Void*                 pCallbackData);

typedef OpcUa_StatusCode (OpcUa_Listener_PfnOpen)(
    struct _OpcUa_Listener*     pListener,
    OpcUa_String*               sUrl,
    OpcUa_Listener_PfnOnNotify* pCallback,
    OpcUa_Void*                 pCallbackData);

/** 
  @brief Stops listening for incoming messages.
 
  @param pListener [in] The listener to close.
*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_Listener_Close(struct _OpcUa_Listener* pListener);

typedef OpcUa_StatusCode (OpcUa_Listener_PfnClose)(struct _OpcUa_Listener* pListener);

/** 
  @brief Finishes reading a request and create a stream to write the response.

  The caller is responsible for calling Delete on the returned stream.
 
  @param pListener   [in]  The listener.
  @param hConnection [in]  The connection to write to.
  @param ppIstrm     [in]  The input stream to delete. Handle invalid after return.
  @param ppOstrm     [out] The output stream used to send the response.
*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_Listener_BeginSendResponse(
    struct _OpcUa_Listener* pListener,
    OpcUa_Handle            hConnection,
    OpcUa_InputStream**     ppIstrm,
    OpcUa_OutputStream**    ppOstrm);

typedef OpcUa_StatusCode (OpcUa_Listener_PfnBeginSendResponse)(
    struct _OpcUa_Listener* pListener,
    OpcUa_Handle            hConnection,
    OpcUa_InputStream**     ppIstrm,
    OpcUa_OutputStream**    ppOstrm);
/** 
  @brief Tells the listener that the response is complete and it can be sent. Cleans up too.
 
  @param pListener [in] The listener.
  @param uStatus   [in] The status of the prior operations.
  @param ppOstrm   [bi] The output stream to send and delete.
*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_Listener_EndSendResponse(
    struct _OpcUa_Listener* pListener,
    OpcUa_StatusCode        uStatus,
    OpcUa_OutputStream**    ppOstrm);

typedef OpcUa_StatusCode (OpcUa_Listener_PfnEndSendResponse)(
    struct _OpcUa_Listener* pListener,
    OpcUa_StatusCode        uStatus,
    OpcUa_OutputStream**    pOstrm);

/** 
  @brief Tells the listener that the response is cancelled.
 
  Resources get cleaned up without data being sent.

  @param pListener [in] The listener.
  @param uStatus   [in] The status of the cancellation.
  @param psReason  [in] The reason for the cancellation.
  @param ppOstrm   [bi] The output stream to abort and delete.
*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_Listener_AbortSendResponse(
    struct _OpcUa_Listener* pListener,
    OpcUa_StatusCode        uStatus,
    OpcUa_String*           psReason,
    OpcUa_OutputStream**    ppOstrm);

typedef OpcUa_StatusCode (OpcUa_Listener_PfnAbortSendResponse)(
    struct _OpcUa_Listener* pListener,
    OpcUa_StatusCode        uStatus,
    OpcUa_String*           psReason,
    OpcUa_OutputStream**    pOstrm);

/** 
  @brief Retrive the recieve buffer size of a particular connection.
 
  @param pListener   [in] The listener.
  @param hConnection [in] The handle of the connection whose buffer size is requested.
  @param pBufferSize [bi] Pointer to OpcUa_UInt32 for storing the buffer size.
*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_Listener_GetReceiveBufferSize(
    struct _OpcUa_Listener* pListener,
    OpcUa_Handle            hConnection,
    OpcUa_UInt32*           pBufferSize);

typedef OpcUa_StatusCode (OpcUa_Listener_PfnGetReceiveBufferSize)(
    struct _OpcUa_Listener* pListener,
    OpcUa_Handle            hConnection,
    OpcUa_UInt32*           pBufferSize);
/** 
  @brief Retrive the peer info of a particular connection.
 
  @param pListener   [in] The listener.
  @param hConnection [in] The handle of the connection whose buffer size is requested.
  @param pPeerInfo   [bi] Pointer to OpcUa_String for storing the peer information
*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_Listener_GetPeerInfo(
    struct _OpcUa_Listener* pListener,
    OpcUa_Handle            hConnection,
    OpcUa_String*           pPeerInfo);

typedef OpcUa_StatusCode (OpcUa_Listener_PfnGetPeerInfo)(
    struct _OpcUa_Listener* pListener,
    OpcUa_Handle            hConnection,
    OpcUa_String*           pPeerInfo);

/** 
 * @brief Returns a set of security parameters used by the secure channel with the given stream.
 *
 * @param pListener                     [in] Listener responsible for the secure channel.
 * @param pIstrm                        [in] The input stream.
 * @param pSecurityPolicyConfiguration [out] The requested security settings.
*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_Listener_GetSecurityPolicyConfiguration(
    struct _OpcUa_Listener*                     pListener,
    OpcUa_InputStream*                          pIstrm,
    OpcUa_Listener_SecurityPolicyConfiguration* pSecurityPolicyConfiguration);

typedef OpcUa_StatusCode (OpcUa_Listener_PfnGetSecurityPolicyConfiguration)(
    struct _OpcUa_Listener*                     pListener,
    OpcUa_InputStream*                          pIstrm,
    OpcUa_Listener_SecurityPolicyConfiguration* pSecurityPolicyConfiguration);

/** 
  @brief Tells the listener to close a particular connection.
 
  @param pListener   [in] The listener.
  @param hConnection [in] The connection to close.
  @param uStatus     [in] The status of the prior operations.
*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_Listener_CloseConnection(
    struct _OpcUa_Listener* pListener,
    OpcUa_Handle            hConnection,
    OpcUa_StatusCode        uStatus);

typedef OpcUa_StatusCode (OpcUa_Listener_PfnCloseConnection)(
    struct _OpcUa_Listener* pListener,
    OpcUa_Handle            hConnection,
    OpcUa_StatusCode        uStatus);

/** 
  @brief Frees the listener structure.
 
  @param ppListener [in] The listener.
*/
OPCUA_EXPORT OpcUa_Void OpcUa_Listener_Delete(struct _OpcUa_Listener** ppListener);

typedef OpcUa_Void (OpcUa_Listener_PfnDelete)(struct _OpcUa_Listener** ppListener);

/** 
  @brief Forward a buffer list to the transport for sending.

  @param pListener   [in] The listener.
  @param hConnection [in] The connection.
  @param pBufferList [in] Pointer to a list of send buffers.
  @param uFlags      [in] operation flags.
*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_Listener_AddToSendQueue(
    struct _OpcUa_Listener*  pListener,
    OpcUa_Handle             hConnection,
    OpcUa_BufferListElement* pBufferList,
    OpcUa_UInt32             uFlags);

typedef OpcUa_StatusCode (OpcUa_Listener_PfnAddToSendQueue)(
    struct _OpcUa_Listener*  pListener,
    OpcUa_Handle             hConnection,
    OpcUa_BufferListElement* pBufferList,
    OpcUa_UInt32             uFlags);

/**
  @brief A generic listener for an endpoint.
*/
typedef struct _OpcUa_Listener
{
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

    /*! @brief Aborts an response. */
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

    /*! @brief Retrive the peer information of a particular connection. */
    OpcUa_Listener_PfnGetPeerInfo* GetPeerInfo;
}
OpcUa_Listener;

OPCUA_END_EXTERN_C

#endif /* _OpcUa_Listener_H_ */
