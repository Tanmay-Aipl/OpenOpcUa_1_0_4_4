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

#ifndef _OpcUa_SecureListener_H_
#define _OpcUa_SecureListener_H_ 1

#ifdef OPCUA_HAVE_SERVERAPI

#include <opcua_listener.h>
#include <opcua_encoder.h>
#include <opcua_decoder.h>
#include <opcua_securechannel_types.h>

OPCUA_BEGIN_EXTERN_C

/**
 * @brief Types of events that can occur on secure channels.
*/
typedef enum eOpcUa_SecureListener_SecureChannelEvent
{
    eOpcUa_SecureListener_SecureChannelOpen,
    eOpcUa_SecureListener_SecureChannelClose,
    eOpcUa_SecureListener_SecureChannelRenew,
    eOpcUa_SecureListener_SecureChannelOpenVerifyCertificate,
    eOpcUa_SecureListener_SecureChannelRenewVerifyCertificate,
    eOpcUa_SecureListener_SecureChannelLostTransportConnection,
    eOpcUa_SecureListener_SecureChannelUnkown
} OpcUa_SecureListener_SecureChannelEvent;


/** 
 * @brief Associates a supported security policy with message security modes.
 */
struct _OpcUa_SecureListener_SecurityPolicyConfiguration
{
    /** @brief The URI of a supported security policy. */
    OpcUa_String        sSecurityPolicy;
    /** @brief The message security modes allowed for the security policy. (bitmask) */
    OpcUa_UInt16        uMessageSecurityModes;   
    /** @brief The client certificate, if provided. */
    OpcUa_ByteString*   pbsClientCertificate;
};

typedef struct _OpcUa_SecureListener_SecurityPolicyConfiguration OpcUa_SecureListener_SecurityPolicyConfiguration;

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
typedef OpcUa_StatusCode (OpcUa_SecureListener_PfnSecureChannelCallback)(
    OpcUa_UInt32                                        uSecureChannelId,
    OpcUa_SecureListener_SecureChannelEvent             eSecureChannelEvent,
    OpcUa_StatusCode                                    uStatus,
    OpcUa_ByteString*                                   pbsClientCertificate,
    OpcUa_String*                                       sSecurityPolicy,
    OpcUa_UInt16                                        uMessageSecurityModes,
    OpcUa_UInt32                                        uRequestedLifetime,
    OpcUa_Void*                                         pCallbackData);

/** 
  @brief Creates a new secure listener object.

  @param pInnerListener                 [in]  The inner the listener is attached to.
  @param pDecoder                       [in]
  @param pEncoder                       [in]
  @param pNamespaceUris                 [in]
  @param pKnownTypes                    [in]
  @param pServerCertificate             [in]
  @param pServerPrivateKey              [in]
  @param pPKIConfig                     [in]
  @param nNoSecurityPolicies            [in]
  @param pSecurityPolicyConfigurations  [in]
  @param pfSecureChannelCallback        [in]
  @param pSecureChannelCallbackData     [in]
  @param ppSecureListener               [out] The new listener.
*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_SecureListener_Create(
    OpcUa_Listener*                                     pInnerListener,
    OpcUa_Decoder*                                      pDecoder,
    OpcUa_Encoder*                                      pEncoder,
    OpcUa_StringTable*                                  pNamespaceUris,
    OpcUa_EncodeableTypeTable*                          pKnownTypes,
    OpcUa_ByteString*                                   pServerCertificate,
    OpcUa_Key*                                          pServerPrivateKey,
    OpcUa_Void*                                         pPKIConfig,
    OpcUa_UInt32                                        nNoSecurityPolicies,
    OpcUa_SecureListener_SecurityPolicyConfiguration*   pSecurityPolicyConfigurations,
    OpcUa_SecureListener_PfnSecureChannelCallback*      pfSecureChannelCallback,
    OpcUa_Void*                                         pSecureChannelCallbackData,
    OpcUa_Listener**                                    ppSecureListener);

/**
 * @brief Returns ID of the secure channel on which the data in the input stream was received.
 *
 * @param pListener     [in] Listener responsible for the input stream.
 * @param pIstrm        [in] The input stream.
 * @param puChannelId  [out] The id of the secure channel after return.
 */
OPCUA_EXPORT OpcUa_StatusCode OpcUa_SecureListener_GetChannelId(
    OpcUa_Listener*                                     pListener,
    OpcUa_InputStream*                                  pIstrm,
    OpcUa_UInt32*                                       puChannelId);

/** 
 * @brief Returns a set of security parameters used by the secure channel with the given stream.
 *
 * @param pListener                     [in] Listener responsible for the secure channel.
 * @param pIstrm                        [in] The input stream.
 * @param pSecurityPolicyConfiguration [out] The requested security settings.
*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_SecureListener_GetSecurityPolicyConfiguration(
    OpcUa_Listener*                             pListener,
    OpcUa_InputStream*                          pIstrm,
    OpcUa_Listener_SecurityPolicyConfiguration* pSecurityPolicyConfiguration);

/**
 * @brief Returns a set of security parameters used by the secure channel with the given stream.
 *
 * @param pListener                     [in] Listener responsible for the secure channel.
 * @param uChannelId                    [in] The secure channel id.
 * @param pSecurityPolicyConfiguration [out] The requested security settings.
*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_SecureListener_GetSecureChannelSecurityPolicyConfiguration(
    OpcUa_Listener*                                     pListener,
    OpcUa_UInt32                                        uChannelId,
    OpcUa_SecureListener_SecurityPolicyConfiguration*   pSecurityPolicyConfiguration);

/**
 * @brief Returns a set of security parameters used by the secure channel with the given stream.
 *
 * @param pListener     [in] Listener responsible for the secure channel.
 * @param uChannelId    [in] Id of the secure channel.
 * @param psPeerInfo   [out] String containing connection information.
*/
OpcUa_StatusCode OpcUa_SecureListener_GetPeerInfoBySecureChannelId(
    OpcUa_Listener*                             pListener,
    OpcUa_UInt32                                uChannelId,
    OpcUa_String*                               psPeerInfo);

/**
 * @brief Close the connection to the client identified by the secure channel id.
 *
 * @param pListener      [in] Listener managing the secure channel.
 * @param uChannelId     [in] Id of the searched secure channel.
 * @param phConnection  [out] Resulting connection handle.
*/
OpcUa_StatusCode OpcUa_SecureListener_GetConnectionHandleBySecureChannelId(
    OpcUa_Listener*                             pListener,
    OpcUa_UInt32                                uChannelId,
    OpcUa_Handle*                               phConnection);

OPCUA_END_EXTERN_C

#endif /* OPCUA_HAVE_SERVERAPI */

#endif /* _OpcUa_SecureListener_H_ */
