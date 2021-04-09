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

#ifndef _OpcUa_P_OpenSSL_PKI_H_
#define _OpcUa_P_OpenSSL_PKI_H_ 1

OPCUA_BEGIN_EXTERN_C

/**
  @brief Creates a certificate store object.

  @param pProvider                  [in]  The crypto provider handle.

  @param ppCertificateStore         [out] The handle to the certificate store. Type depends on store implementation.
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_PKI_OpenCertificateStore(
    OpcUa_PKIProvider*          pProvider,
    OpcUa_Void**                ppCertificateStore);

/**
  @brief frees a certificate store object.

  @param pProvider             [in]  The crypto provider handle.
  @param ppCertificateStore    [in] The certificate store object. Type depends on store implementation.
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_PKI_CloseCertificateStore(
    OpcUa_PKIProvider*       pProvider,
    OpcUa_Void**             ppCertificateStore);

/**
  @brief Validates a given X509 certificate object.

   Validation:
   - Subject/Issuer
   - Path
   - Certificate Revocation List (CRL)
   - Certificate Trust List (CTL)

  @param pProvider                [in]  The crypto provider handle.
  @param pCertificate             [in]  The certificate that should be validated.
  @param pCertificateStore        [in]  The certificate store that validates the passed in certificate.

  @param pValidationCode          [out] The validation code, that gives information about the validation result. Validation return codes from OpenSSL are used.
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_PKI_ValidateCertificate(
    OpcUa_PKIProvider*          pProvider,
    OpcUa_ByteString*           pCertificate,
    OpcUa_Void*                 pCertificateStore,
    OpcUa_Int*                  pValidationCode);

/**
  @brief imports a given certificate into given certificate store.

  @param pProvider                [in]  The crypto provider handle.
  @param pCertificateStore        [in]  The certificate store that should store the passed in certificate.
  @param pCertificate             [in]  The certificate that should be stored in the certificate store.
  @param pSaveHandle              [out]  The handle that indicates the save location of the certificate within then certificate store.
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_PKI_SaveCertificate(
    OpcUa_PKIProvider*          pProvider,
    OpcUa_ByteString*           pCertificate,
    OpcUa_Void*                 pCertificateStore,
    OpcUa_Void*                 pSaveHandle);

/**
  @brief exports a certain certificate from a given certificate store.

  @param pProvider                [in]  The crypto provider handle.
  @param pLoadHandle              [in]  The handle that indicates the load location of the certificate within then certificate store.
  @param ppCertificateStore       [in]  The certificate store that contains the desired certificate.

  @param pCertificate             [out] The desired certificate.
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_PKI_LoadCertificate(
    OpcUa_PKIProvider*          pProvider,
    OpcUa_Void*                 pLoadHandle,
    OpcUa_Void*                 pCertificateStore,
    OpcUa_ByteString*           pCertificate);

/**
  @brief Splits certificate chain into its elements.

  The returned byte strings reference the memory blocks in the input parameter.
  The data pointers become invalid if the chain is freed! Also, freeing the memory
  referenced by the chain elements is not allowed!

  @param pCertificateChain      [in]  Byte string consisting of concatenated DER certificates.

  @param pNumberOfChainElements [out] Number of certificates found in the chain.
  @param pabyChainElements      [out] Array of byte strings referencing certificates in pCertificateChain (don't free contents).
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_PKI_SplitCertificateChain(
    OpcUa_ByteString*           pCertificateChain,
    OpcUa_UInt32*               pNumberOfChainElements,
    OpcUa_ByteString**          pabyChainElements);

OPCUA_END_EXTERN_C

#endif /* _OpcUa_Crypto_OpenSsl_H_ */
