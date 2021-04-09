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

#ifndef _OpcUa_P_PKI_H_
#define _OpcUa_P_PKI_H_ 1

OPCUA_BEGIN_EXTERN_C

#define OPCUA_P_CERTIFICATESTORECONFIGURATION_DATA_SIZE 0

/**
  @brief The PK infrastructure systems supported by the platform layer.
*/
#define OPCUA_P_PKI_TYPE_OPENSSL            "OpenSSL"
#define OPCUA_P_PKI_TYPE_WIN32              "WIN32"

/**
  @brief The WIN32 PKI specific store flags.
*/
#define OPCUA_P_PKI_WIN32_STORE_USER                    1
#define OPCUA_P_PKI_WIN32_STORE_MACHINE                 2
#define OPCUA_P_PKI_WIN32_STORE_SERVICES                4

/**
  @brief The OpenSSL PKI specific store flags.
*/
#define OpcUa_P_PKI_OPENSSL_CHECK_REVOCATION_STATUS     1
#define OpcUa_P_PKI_OPENSSL_SET_DEFAULT_PATHS           2
#define OpcUa_P_PKI_OPENSSL_CERT_PATH_IS_HASH_DIRECTORY 4
#define OpcUa_P_PKI_OPENSSL_CRL_PATH_IS_HASH_DIRECTORY  8
#define OpcUa_P_PKI_OPENSSL_CRL_PATH_IS_MULTI_CRL_FILE  16


/**
  @brief The certificate und key format enumeration.
*/
typedef enum _OpcUa_P_FileFormat
{
    OpcUa_Crypto_Encoding_Invalid   = 0,
    OpcUa_Crypto_Encoding_DER       = 1,
    OpcUa_Crypto_Encoding_PEM       = 2,
    OpcUa_Crypto_Encoding_PKCS12    = 3
}
OpcUa_P_FileFormat;

/**
  @brief Loads a X.509 certificate from the specified file.
  */
OpcUa_StatusCode OpcUa_P_OpenSSL_X509_LoadFromFile(
    OpcUa_StringA               fileName,
    OpcUa_P_FileFormat          fileFormat,
    OpcUa_StringA               sPassword,      /* optional: just for OpcUa_PKCS12 */
    OpcUa_ByteString*           pCertificate);

/**
  @brief Loads a RSA private key from the specified file.
  */
OpcUa_StatusCode OpcUa_P_OpenSSL_RSA_LoadPrivateKeyFromFile(
    OpcUa_StringA           privateKeyFile,
    OpcUa_P_FileFormat      fileFormat,
    OpcUa_StringA           password,
    OpcUa_ByteString*       pPrivateKey);

OpcUa_StatusCode OpcUa_P_OpenSSL_RSA_SavePrivateKeyToFile(
    OpcUa_StringA           privateKeyFile,
    OpcUa_P_FileFormat      fileFormat,
    OpcUa_StringA           password,
    OpcUa_ByteString*       pPrivateKey);

OPCUA_END_EXTERN_C

#endif