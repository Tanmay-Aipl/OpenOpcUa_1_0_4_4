/* ========================================================================
 * Copyright (c) 2005-2010 The OPC Foundation, Inc. All rights reserved.
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

#ifndef _OpcUa_HttpsListener_H_
#define _OpcUa_HttpsListener_H_ 1

#include <opcua_listener.h>


OPCUA_BEGIN_EXTERN_C

/** 
 * @brief Types of events that can occur on secure channels.
*/
typedef enum eOpcUa_HttpsListener_SecureChannelEvent
{
    eOpcUa_HttpsListener_SecureChannelOpen,
    eOpcUa_HttpsListener_SecureChannelClose,
    eOpcUa_HttpsListener_SecureChannelRenew,
    eOpcUa_HttpsListener_SecureChannelUnkown
} OpcUa_HttpsListener_SecureChannelEvent;

/** 
 * @brief Function, that needs to be implemented to receive notifications about secure channel events.
 *
 * @param uSecureChannelId      [in] The id assigned to the secure channel.
 * @param eEvent                [in] What type of event on the secure channel occured.
 * @param uStatus               [in] The result of the operation.
 * @param pbsClientCertificate  [in] The certificate of the client.
 * @param sSecurityPolicy       [in] The security policy in case of open or renew.
 * @param eMessageSecurityMode  [in] What type of event on the secure channel occured.
 * @param uRequestedLifetime    [in] The requested securechannel lifetime.
 * @param pCallbackData         [in] Data pointer received at creation.
 */
typedef OpcUa_StatusCode (OpcUa_HttpsListener_PfnSecureChannelCallback)(
    OpcUa_UInt32                                        uSecureChannelId,
    OpcUa_HttpsListener_SecureChannelEvent              eSecureChannelEvent,
    OpcUa_StatusCode                                    uStatus,
    OpcUa_ByteString*                                   pbsClientCertificate,
    OpcUa_String*                                       sSecurityPolicy,
    OpcUa_UInt16                                        uMessageSecurityModes,
    OpcUa_UInt32                                        uRequestedLifetime,
    OpcUa_Void*                                         pCallbackData);


/**
  @brief Creates a new http listener object.

  @param a_ppListener [out] The new listener.
*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_HttpsListener_Create(   OpcUa_ByteString*                               a_pServerCertificate,
                                                            OpcUa_Key*                                      a_pServerPrivateKey,
                                                            OpcUa_Void*                                     a_pPKIConfig,
                                                            OpcUa_HttpsListener_PfnSecureChannelCallback*   a_pfSecureChannelCallback,
                                                            OpcUa_Void*                                     a_pSecureChannelCallbackData,
                                                            OpcUa_Listener**                                a_pListener);

/**
  @brief Sends an immediate http response.

  @param a_pListener        [in]  The listener.
  @param a_hConnection      [in]  The connection to write to.
  @param a_nStatusCode      [in]  The status code of the response.
  @param a_sReasonPhrase    [in]  The textual description of the status code.
  @param a_sResponseHeaders [in]  The response headers.
  @param a_pResponseData    [in]  The response data to be sent.
  @param a_uResponseLength  [in]  The length of the response data.
*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_HttpsListener_SendImmediateResponse(
    OpcUa_Listener*     a_pListener,
    OpcUa_Handle        a_hConnection,
    OpcUa_StatusCode    a_uStatusCode,
    OpcUa_StringA       a_sReasonPhrase,
    OpcUa_StringA       a_sResponseHeaders,
    OpcUa_Byte*         a_pResponseData,
    OpcUa_UInt32        a_uResponseLength);

OPCUA_END_EXTERN_C

#endif /* _OpcUa_HttpListener_H_ */
