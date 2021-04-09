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

#ifndef _OpcUa_Endpoint_Ex_H_
#define _OpcUa_Endpoint_Ex_H_ 1
#ifdef OPCUA_HAVE_SERVERAPI

#include <opcua_endpoint.h>
#include <opcua_listener.h>

OPCUA_BEGIN_EXTERN_C

/** 
 * @brief Starts accepting connections for the endpoint on the given URL.
 *
 * @param hEndpoint [in] The endpoint to open.
 * @param sUrl      [in] The URL for this endpoint. (String must be zeroterminated.)
 */
OPCUA_EXPORT
OpcUa_StatusCode OpcUa_Endpoint_OpenEx( OpcUa_Endpoint                              hEndpoint,
                                        OpcUa_StringA                               sUrl,
                                        OpcUa_Endpoint_PfnEndpointCallback          pfEndpointCallback,
                                        OpcUa_Void*                                 pvEndpointCallbackData,
                                        OpcUa_ByteString*                           pServerCertificate,
                                        OpcUa_Key*                                  pPrivateKey,
                                        OpcUa_Void*                                 pPKIConfig,
                                        OpcUa_UInt32                                nNoOfSecurityPolicies,
                                        OpcUa_Endpoint_SecurityPolicyConfiguration* pSecurityPolicies,
                                        OpcUa_Listener_PfnOnNotify*                 pfListenerCallback,
                                        OpcUa_Void*                                 pListenerCallbackData);

/** 
 * @brief Opens the output stream used to write the response.
 * 
 * @param hEndpoint   [in]  The endpoint handle.
 * @param pConnection [in]  The connection handle.
 * @param pIstrm      [in]  The input stream used for the request.
 * @param ppOstrm     [out] The output stream used for the response.
 */
OPCUA_EXPORT
OpcUa_StatusCode OpcUa_Endpoint_BeginSendEncodedResponse( 
    OpcUa_Endpoint       hEndpoint,
    OpcUa_Handle         pConnection,
    OpcUa_InputStream**  ppIstrm,
    OpcUa_OutputStream** ppOstrm);

/** 
 * @brief Closes the output stream used to write the response.
 * 
 * @param hEndpoint   [in]     The endpoint handle.
 * @param uStatus     [in]     Any error that occurred processing the response.
 * @param ppOstrm     [in/out] The output stream used for the response.
 */
OPCUA_EXPORT
OpcUa_StatusCode OpcUa_Endpoint_EndSendEncodedResponse( 
    OpcUa_Endpoint       hEndpoint,
    OpcUa_StatusCode     uStatus,
    OpcUa_OutputStream** ppOstrm);

/** 
 * @brief Returns the secure channel id associated with the input stream.
 * 
 * @param pSecureIstrm     [in]     The handle of the input stream.
 * @param pSecureChannelId [in/out] The secure channel id associated with the stream.
 */
OpcUa_StatusCode OpcUa_SecureListener_GetSecureChannelId(
    OpcUa_InputStream* pSecureIstrm,
    OpcUa_UInt32*      pSecureChannelId);

OPCUA_END_EXTERN_C

#endif /* OPCUA_HAVE_SERVERAPI */
#endif /* _OpcUa_Endpoint_Ex_H_ */
