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

#ifndef _OpcUa_P_OpenSSL_H_
#define _OpcUa_P_OpenSSL_H_ 1

OPCUA_BEGIN_EXTERN_C

/**
  @brief Initializes the OpenSSL library.
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_Initialize();

/**
  @brief cleans up the OpenSSL library.
*/
void OpcUa_P_OpenSSL_Cleanup();

/**
  @brief seeds pseudo-random-number-generator of openssl.
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_SeedPRNG(OpcUa_Int bytes);


/**
  @brief Encrypts data using Advanced Encryption Standard (AES) with the Cipher Block Chaining (CBC) mode.

   keylen = blocksize => 128
   message length = outputlength

  @param pProvider              [in]  Provider handle.
  @param pPlainText             [in]  Plain text to encrypt.
  @param plainTextLen           [in]  The length of the plain text in bytes.
  @param key                    [in]  The encryption/decryption key.
  @param pInitalVector          [in]  The initial vector.

  @param pCipherText            [out] The encrypted text.
  @param pCipherTextLen         [out] The length of the encrypted text
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_AES_128_CBC_Encrypt(
    OpcUa_CryptoProvider*   pProvider,
    OpcUa_Byte*             pPlainText,
    OpcUa_UInt32            plainTextLen, /* message length = outputlength */
    OpcUa_Key*              key,
    OpcUa_Byte*             pInitalVector,
    OpcUa_Byte*             pCipherText,
    OpcUa_UInt32*           pCipherTextLen);

/**
  @brief Decrypts encrypted data using Advanced Encryption Standard (AES) with the Cipher Block Chaining (CBC) mode.

  keylen = blocksize => 128
  synchronous!

  @param pProvider              [in]  Provider handle.
  @param pCipherText            [in]  Cipher text to decrypt.
  @param cipherTextLen          [in]  The length of the cipher text in bytes.
  @param key                    [in]  The encryption/decryption key.
  @param pInitalVector          [in]  The initial vector.

  @param pPlainText             [out] The decrypted text.
  @param pPlainTextLen          [out] The length of the decrypted text
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_AES_128_CBC_Decrypt(
    OpcUa_CryptoProvider*   pProvider,
    OpcUa_Byte*             pCipherText,
    OpcUa_UInt32            cipherTextLen, /* cipher length */
    OpcUa_Key*              key,
    OpcUa_Byte*             pInitalVector,
    OpcUa_Byte*             pPlainText,
    OpcUa_UInt32*           pPlainTextLen);

/**
  @brief Encrypts data using Advanced Encryption Standard (AES) with the Cipher Block Chaining (CBC) mode.

   keylen = blocksize => 256
   message length = outputlength

  @param pProvider              [in]  Provider handle.
  @param pPlainText             [in]  Plain text to encrypt.
  @param plainTextLen           [in]  The length of the plain text in bytes.
  @param key                    [in]  The encryption/decryption key.
  @param pInitalVector          [in]  The initial vector.

  @param pCipherText            [out] The encrypted text.
  @param pCipherTextLen         [out] The length of the encrypted text
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_AES_256_CBC_Encrypt(
    OpcUa_CryptoProvider*   pProvider,
    OpcUa_Byte*             pPlainText,
    OpcUa_UInt32            plainTextLen, /* message length = outputlength */
    OpcUa_Key*              key,
    OpcUa_Byte*             pInitalVector,
    OpcUa_Byte*             pCipherText,
    OpcUa_UInt32*           pCipherTextLen);

/**
  @brief Decrypts encrypted data using Advanced Encryption Standard (AES) with the Cipher Block Chaining (CBC) mode.

  keylen = blocksize => 128
  synchronous!

  @param pProvider              [in]  Provider handle.
  @param pCipherText            [in]  Cipher text to decrypt.
  @param cipherTextLen          [in]  The length of the cipher text in bytes.
  @param key                    [in]  The encryption/decryption key.
  @param pInitalVector          [in]  The initial vector.

  @param pPlainText             [out] The decrypted text.
  @param pPlainTextLen          [out] The length of the decrypted text
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_AES_256_CBC_Decrypt(
    OpcUa_CryptoProvider*   pProvider,
    OpcUa_Byte*             pCipherText,
    OpcUa_UInt32            cipherTextLen, /* cipher length */
    OpcUa_Key*              key,
    OpcUa_Byte*             pInitalVector,
    OpcUa_Byte*             pPlainText,
    OpcUa_UInt32*           pCipherTextLen);

/**
  @brief Encrypts data using (RSA)<NAME> with the public key of the appropriate key pair.

  RSA_PKCS1_PADDING
  synchronous!

  @param pProvider         [in]  The crypto provider handle.
  @param pPlainText        [in]  The plain text to encrypt.
  @param plainTextLen      [in]  The length of the plain text to encrypt.
  @param publicKey         [in]  The public key used to encrypt the plain text.

  @param pCipherText       [out] The encrypted text.
  @param pCipherTextLen    [out] The length of the encrypted text.
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_RSA_PKCS1_V15_Encrypt(
    OpcUa_CryptoProvider*   pProvider,
    OpcUa_Byte*             pPlainText,
    OpcUa_UInt32            plainTextLen,
    OpcUa_Key*              publicKey,
    OpcUa_Byte*             pCipherText,
    OpcUa_UInt32*           pCipherTextLen);

/**
  @brief Decrypts encrypted data using <NAME>(RSA) with the private key of the appropriate key pair.

  RSA_PKCS1_PADDING
  synchonous!

  @param pProvider         [in]  The crypto provider handle.
  @param pCipherText       [in]  The cipher text to decrypt.
  @param cipherTextLen     [in]  The length of the cipher text to decrypt.
  @param privateKey        [in]  The private key used to decrypt the plain text.

  @param pPlainText        [out] The decrypted text.
  @param pPlainTextLen     [out] The length of the decrypted text.
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_RSA_PKCS1_V15_Decrypt(
    OpcUa_CryptoProvider*   pProvider,
    OpcUa_Byte*             pCipherText,
    OpcUa_UInt32            cipherTextLen,
    OpcUa_Key*              privateKey,
    OpcUa_Byte*             pPlainText,
    OpcUa_UInt32*           pPlainTextLen);

/**
  @brief Encrypts data using (RSA)<NAME> with the public key of the appropriate key pair.

  RSA_PKCS1_OAEP_PADDING
  synchronous!

  @param pProvider         [in]  The crypto provider handle.
  @param pPlainText        [in]  The plain text to encrypt.
  @param plainTextLen      [in]  The length of the plain text to encrypt.
  @param publicKey         [in]  The public key used to encrypt the plain text.

  @param pCipherText       [out] The encrypted text.
  @param pCipherTextLen    [out] The length of the encrypted text.
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_RSA_OAEP_Encrypt(
    OpcUa_CryptoProvider*   pProvider,
    OpcUa_Byte*             pPlainText,
    OpcUa_UInt32            plainTextLen,
    OpcUa_Key*              publicKey,
    OpcUa_Byte*             pCipherText,
    OpcUa_UInt32*           pCipherTextLen);

/**
  @brief Decrypts encrypted data using <NAME>(RSA) with the private key of the appropriate key pair.

  RSA_PKCS1_OAEP_PADDING
  synchonous!

  @param pProvider         [in]  The crypto provider handle.
  @param pCipherText       [in]  The cipher text to decrypt.
  @param cipherTextLen     [in]  The length of the cipher text to decrypt.
  @param privateKey        [in]  The private key used to decrypt the plain text.

  @param pPlainText        [out] The decrypted text.
  @param pPlainTextLen     [out] The length of the decrypted text.
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_RSA_OAEP_Decrypt(
    OpcUa_CryptoProvider*   pProvider,
    OpcUa_Byte*             pCipherText,
    OpcUa_UInt32            cipherTextLen,
    OpcUa_Key*              privateKey,
    OpcUa_Byte*             pPlainText,
    OpcUa_UInt32*           pPlainTextLen);

/**
  @param pProvider        [in]  The crypto provider handle.
  @param pData            [in]  The data for the MAC generation.
  @param dataLen          [in]  The length data for the MAC generation.
  @param key              [in]  The key for the MAC generation.

  @param pSignature       [out] The resulting signature (MAC).
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_HMAC_SHA1_Sign(
    OpcUa_CryptoProvider* pProvider,
    OpcUa_Byte*           pData,
    OpcUa_UInt32          dataLen,
    OpcUa_Key*            key,
    OpcUa_ByteString*     pSignature);

/**
  @param pProvider              [in]  The crypto provider handle.
  @param pData                  [in]  The data for the MAC generation.
  @param dataLen                [in]  The length data for the MAC generation.
  @param key                    [in]  The key for the MAC generation.
  @param pSignature             [in]  The resulting signature (MAC).
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_HMAC_SHA1_Verify(
    OpcUa_CryptoProvider* pProvider,
    OpcUa_Byte*           pData,
    OpcUa_UInt32          dataLen,
    OpcUa_Key*            key,
    OpcUa_ByteString*     pSignature);

/**
  @param pProvider        [in]  The crypto provider handle.
  @param pData            [in]  The data for the MAC generation.
  @param dataLen          [in]  The length data for the MAC generation.
  @param key              [in]  The key for the MAC generation.

  @param pSignature       [out] The resulting signature (MAC).
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_HMAC_SHA256_Sign(
    OpcUa_CryptoProvider* pProvider,
    OpcUa_Byte*           pData,
    OpcUa_UInt32          dataLen,
    OpcUa_Key*            key,
    OpcUa_ByteString*     pSignature);

/**
  @param pProvider              [in]  The crypto provider handle.
  @param pData                  [in]  The data for the MAC generation.
  @param dataLen                [in]  The length data for the MAC generation.
  @param key                    [in]  The key for the MAC generation.
  @param pSignature             [in]  The resulting signature (MAC).
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_HMAC_SHA256_Verify(
    OpcUa_CryptoProvider* pProvider,
    OpcUa_Byte*           pData,
    OpcUa_UInt32          dataLen,
    OpcUa_Key*            key,
    OpcUa_ByteString*     pSignature);

/**@brief Signs data using <NAME>(RSA) with the private key of the appropriate key pair.

  @param pProvider         [in]  The crypto provider handle.
  @param data              [in]  The data to sign.
  @param privateKey        [in]  The private key used to sign the data.

  @param pSignature        [out] The signature of the data.
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_RSA_PKCS1_V15_SHA1_Sign(
    OpcUa_CryptoProvider* pProvider,
    OpcUa_ByteString      data,
    OpcUa_Key*            privateKey,
    OpcUa_ByteString*     pSignature);      /* minimum length = key length */

/**
  @brief Verifies signed data using <NAME>(RSA) with the public key of the appropriate key pair.

  @param pProvider                  [in]  The crypto provider handle.
  @param data                       [in]  The data that was signed.
  @param publicKey                  [in]  The public key used to verify the signature.
  @param pSignature                 [in]  The signature of the data that should be verified.
 */
OpcUa_StatusCode OpcUa_P_OpenSSL_RSA_PKCS1_V15_SHA1_Verify(
    OpcUa_CryptoProvider* pProvider,
    OpcUa_ByteString      data,
    OpcUa_Key*            publicKey,
    OpcUa_ByteString*     pSignature);

/**@brief Signs data using <NAME>(RSA) with the private key of the appropriate key pair.

  @param pProvider         [in]  The crypto provider handle.
  @param data              [in]  The data to sign.
  @param privateKey        [in]  The private key used to sign the data.

  @param pSignature        [out] The signature of the data.
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_RSA_PKCS1_V15_SHA256_Sign(
    OpcUa_CryptoProvider* pProvider,
    OpcUa_ByteString      data,
    OpcUa_Key*            privateKey,
    OpcUa_ByteString*     pSignature);      /* minimum length = key length */

/**
  @brief Verifies signed data using <NAME>(RSA) with the public key of the appropriate key pair.

  @param pProvider                  [in]  The crypto provider handle.
  @param data                       [in]  The data that was signed.
  @param publicKey                  [in]  The public key used to verify the signature.
  @param pSignature                 [in]  The signature of the data that should be verified.
 */
OpcUa_StatusCode OpcUa_P_OpenSSL_RSA_PKCS1_V15_SHA256_Verify(
    OpcUa_CryptoProvider* pProvider,
    OpcUa_ByteString      data,
    OpcUa_Key*            publicKey,
    OpcUa_ByteString*     pSignature);

/**@brief Signs data using <NAME>(RSA) with the private key of the appropriate key pair.

  @param pProvider         [in]  The crypto provider handle.
  @param data              [in]  The data to sign.
  @param privateKey        [in]  The private key used to sign the data.

  @param pSignature        [out] The signature of the data.
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_RSA_PKCS1_OAEP_SHA1_Sign(
    OpcUa_CryptoProvider* pProvider,
    OpcUa_ByteString      data,
    OpcUa_Key*            privateKey,
    OpcUa_ByteString*     pSignature);      /* minimum length = key length */

/**
  @brief Verifies signed data using <NAME>(RSA) with the public key of the appropriate key pair.

  @param pProvider                  [in]  The crypto provider handle.
  @param data                       [in]  The data that was signed.
  @param publicKey                  [in]  The public key used to verify the signature.
  @param pSignature                  [in]  The signature of the data that should be verified.
 */
OpcUa_StatusCode OpcUa_P_OpenSSL_RSA_PKCS1_OAEP_SHA1_Verify(
    OpcUa_CryptoProvider* pProvider,
    OpcUa_ByteString      data,
    OpcUa_Key*            publicKey,
    OpcUa_ByteString*     pSignature);

/**@brief Signs data using <NAME>(RSA) with the private key of the appropriate key pair.

  @param pProvider         [in]  The crypto provider handle.
  @param data              [in]  The data to sign.
  @param privateKey        [in]  The private key used to sign the data.

  @param pSignature        [out] The signature of the data.
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_RSA_PKCS1_OAEP_SHA256_Sign(
    OpcUa_CryptoProvider* pProvider,
    OpcUa_ByteString      data,
    OpcUa_Key*            privateKey,
    OpcUa_ByteString*     pSignature);      /* minimum length = key length */

/**
  @brief Verifies signed data using <NAME>(RSA) with the public key of the appropriate key pair.

  @param pProvider                  [in]  The crypto provider handle.
  @param data                       [in]  The data that was signed.
  @param publicKey                  [in]  The public key used to verify the signature.
  @param pSignature                  [in]  The signature of the data that should be verified.
 */
OpcUa_StatusCode OpcUa_P_OpenSSL_RSA_PKCS1_OAEP_SHA256_Verify(
    OpcUa_CryptoProvider* pProvider,
    OpcUa_ByteString      data,
    OpcUa_Key*            publicKey,
    OpcUa_ByteString*     pSignature);

/**
  @brief

  if keyLen > 0 then an encryption key, signing key and a IV of the given length is generated for client and server.
  if keyLen == 0 then nothing will be generated.
  if keyLen < 0 then default setting from the CryptoProvider is used.

 */
OpcUa_StatusCode OpcUa_P_OpenSSL_DeriveChannelKeysets(
    OpcUa_CryptoProvider*           pCryptoProvider,
    OpcUa_ByteString                clientNonce,
    OpcUa_ByteString                serverNonce,
    OpcUa_Int32                     keySize,
    OpcUa_SecurityKeyset*           pClientKeyset,
    OpcUa_SecurityKeyset*           pServerKeyset);

/**
  @brief
 */
OpcUa_StatusCode OpcUa_P_OpenSSL_GenerateAsymmetricKeyPair(
    OpcUa_CryptoProvider*   pProvider,
    OpcUa_UInt              type,
    OpcUa_UInt32            bytes,
    OpcUa_Key*              pPublicKey,
    OpcUa_Key*              pPrivateKey);

/*** CERTIFICATES ***/

/**
  @brief Creates a new X509 selfsigned certificate object.

  @param pProvider                [in]  The crypto provider handle.
  @param validFrom                [in]  The validation start time information.
  @param validTo                  [in]  The validation end time information.
  @param pNameEntries             [in]  Name entries for the certificate.
  @param nameEntriesCount         [in]  The count of name entries located at the address in pNameEntries.
  @param pSubjectPublicKey        [in]  The subject's public key.
  @param pExtensions              [in]  The extensions for the desired certificate.
  @param extensionsCount          [in]  The count of extension at the address in pExtensions.
  @param signatureHashAlgorithm   [in]  The hash algorithm for calculating the signature.
  @param pIssuerCertificate       [in]  The certificate for the certificate authority.
  @param pIssuerPrivateKey        [in]  The private key of the certificate authority.

  @param ppCertificate           [out] The new self-signed certificate.
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_X509_SelfSigned_Create(
    OpcUa_CryptoProvider*       pProvider,
    const OpcUa_Int32           serialNumber,
    OpcUa_DateTime              validFrom,
    OpcUa_DateTime              validTo,
    OpcUa_Crypto_NameEntry*     pNameEntries,
    OpcUa_UInt                  nameEntriesCount,  /* will be used for issuer and subject thus it's selfigned cert */
    OpcUa_Key                   pSubjectPublicKey, /* EVP_PKEY* - type defines also public key algorithm */
    OpcUa_Crypto_Extension*     pExtensions,
    OpcUa_UInt                  extensionsCount,
    const OpcUa_UInt            signatureHashAlgorithm, /* EVP_sha1(),... */
    OpcUa_Certificate*          pIssuerCertificate,
    OpcUa_Key                   pIssuerPrivateKey, /* EVP_PKEY* - type defines also signature algorithm */
    OpcUa_Certificate**         ppCertificate);     /* this has to be changed to OpcUa_Certificate** */

/**
  @brief Creates a new X509 selfsigned certificate object.

  @param pProvider                [in]  The crypto provider handle.
  @param validFrom                [in]  The validation start time information.
  @param validTo                  [in]  The validation end time information.
  @param pNameEntries             [in]  Name entries for the certificate.
  @param nameEntriesCount         [in]  The count of name entries located at the address in pNameEntries.
  @param pSubjectPublicKey        [in]  The subject's public key.
  @param pExtensions              [in]  The extensions for the desired certificate.
  @param extensionsCount          [in]  The count of extension at the address in pExtensions.
  @param signatureHashAlgorithm   [in]  The hash algorithm for calculating the signature.
  @param pIssuerPrivateKey        [in]  The private key of the certificate authority.

  @param ppCertificate           [out] The new self-signed certificate.
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_X509_SelfSigned_Custom_Create(
    OpcUa_CryptoProvider*       pProvider,
    const OpcUa_Int32           serialNumber,
    OpcUa_DateTime              validFrom,
    OpcUa_DateTime              validTo,
    OpcUa_Crypto_NameEntry*     pNameEntries,
    OpcUa_UInt                  nameEntriesCount,  /* will be used for issuer and subject thus it's selfigned cert */
    OpcUa_Key                   pSubjectPublicKey, /* EVP_PKEY* - type defines also public key algorithm */
    OpcUa_Crypto_Extension*     pExtensions,
    OpcUa_UInt                  extensionsCount,
    const OpcUa_UInt            signatureHashAlgorithm, /* EVP_sha1(),... */
    OpcUa_Certificate*          pIssuerCertificate, 
    OpcUa_Key                   pIssuerPrivateKey, /* EVP_PKEY* - type defines also signature algorithm */
    OpcUa_Certificate**         ppCertificate);     /* this has to be changed to OpcUa_Certificate** */
/**
  @brief Gets the public key from a given certificate.

  @param pProvider               [in]  A pointer to a crypto provider.
  @param pCertificate            [in]  A pointer to a DER encoded ByteString representation of the certificate.
  @param password                [in]  Password for certificate. Only used when certificate is password protected. (Not used in current implementation)

  @param pPublicKey              [out] The read out public key of the certificate.
*/

OpcUa_StatusCode OpcUa_P_OpenSSL_X509_GetPublicKey(
    OpcUa_CryptoProvider*       pProvider,
    OpcUa_ByteString*           pCertificate,
    OpcUa_StringA               password,
    OpcUa_Key*                  pPublicKey);

/**
  @brief Gets the private key from a given certificate.

  This is done with PKCS #12 Standard.

  @param pProvider                [in]  A pointer to a crypto provider.
  @param certificate              [in]  The passed in certificate.
  @param password                 [in]  Password for certificate. Only used when certificate is password protected. (Not used in current implementation)

  @param pPrivateKey              [out] The read out private key of the certificate.
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_X509_GetPrivateKey(
    OpcUa_CryptoProvider*       pProvider,
    OpcUa_StringA               certificateFileName,
    OpcUa_StringA               password,
    OpcUa_Key*                  pPrivateKey);

/**
  @brief Gets the signature from a given certificate.

  @param pProvider                [in]  A pointer to a crypto provider.
  @param certificate              [in]  The passed in certificate.

  @param pSignature               [out] The read out signature of the certificate.
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_X509_GetSignature(
    OpcUa_CryptoProvider*       pProvider,
    OpcUa_ByteString*           pCertificate,
    OpcUa_Signature*            pSignature);

/**
  @brief Gets the thumbprint of a given certificate.

  @param pProvider              [in]  A pointer to a crypto provider.
  @param pCertificate           [in]  The passed in certificate.

  @param pCertificateThumprint  [out] The SHA-1 thumbprint of the certificate.
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_X509_GetCertificateThumbprint(
    OpcUa_CryptoProvider*       pProvider,
    OpcUa_ByteString*           pCertificate,
    OpcUa_ByteString*           pCertificateThumprint);

/**
  @brief Saves a given certificate in a given file (-location).

  @param pProvider                [in]  A pointer to a crypto provider.
  @param fileName                 [in]  The passed in file location + file name.
  @param certificate              [in]  The passed in certificate.
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_X509_SaveToFile(
    OpcUa_CryptoProvider*       pProvider,
    OpcUa_StringA                fileName,
    OpcUa_Certificate           certificate);


/*** AES SYMMETRIC ENCRYPTION ***/

/**
  @brief Encrypts data using Advanced Encryption Standard (AES) with the Cipher Block Chaining (CBC) mode.

   keylen = blocksize => fixed sizes of 128 = 10 rounds, 192 = 12 rounds, 256 = 14 rounds
   message length = outputlength

  @param pProvider              [in]  Provider handle.
  @param pPlainText             [in]  Plain text to encrypt.
  @param plainTextLen           [in]  The length of the plain text in bytes. (message length = outputlength)
  @param key                    [in]  The encryption/decryption key.
  @param pInitalVector          [in]  The initial vector.

  @param pCipherText            [out] The encrypted text.
  @param pCipherTextLen         [out] The length of the encrypted text
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_AES_CBC_Encrypt(
    OpcUa_CryptoProvider*   pProvider,
    OpcUa_Byte*             pPlainText,
    OpcUa_UInt32            plainTextLen,
    OpcUa_Key*              key,
    OpcUa_Byte*             pInitalVector,
    OpcUa_Byte*             pCipherText,
    OpcUa_UInt32*           pCipherTextLen);

/**
  @brief Decrypts encrypted data using Advanced Encryption Standard (AES) with the Cipher Block Chaining (CBC) mode.

  If PlainText is null, then the cipherTextLen is returned in pPlainTextLen, since this is the maximum output size after decryption.

  synchronous!

  @param pProvider              [in]  Provider handle.
  @param pPlainText             [in]  Plain text to encrypt.
  @param plainTextLen           [in]  The length of the plain text in bytes. (message length = outputlength)
  @param key                    [in]  The encryption/decryption key.
  @param pInitalVector          [in]  The initial vector.

  @param pPlainText             [out] The plaintext.
  @param pPlainTextLen          [out] The length of the plaintext
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_AES_CBC_Decrypt(
    OpcUa_CryptoProvider*   pProvider,
    OpcUa_Byte*             pCipherText,
    OpcUa_UInt32            cipherTextLen,
    OpcUa_Key*              key,
    OpcUa_Byte*             pInitalVector,
    OpcUa_Byte*             pPlainText,
    OpcUa_UInt32*           pPlainTextLen);


/*** RSA ASYMMETRIC ENCRYPTION ***/
/**
  @brief
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_RSA_GenerateKeys(
    OpcUa_CryptoProvider*   pProvider,
    OpcUa_UInt32            bytes,
    OpcUa_Key*              pPublicKey,
    OpcUa_Key*              pPrivateKey);

/**
  @brief
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_RSA_Public_GetKeyLength(
    OpcUa_CryptoProvider*   pProvider,
    OpcUa_Key               publicKey,
    OpcUa_UInt32*           pKeyLen);

/**
  @brief Encrypts data using (RSA)<NAME> with the public key of the appropriate key pair.

  synchronous!

  @param pProvider         [in]  The crypto provider handle.
  @param pPlainText        [in]  The plain text to encrypt.
  @param plainTextLen      [in]  The length of the plain text to encrypt.
  @param publicKey         [in]  The public key used to encrypt the plain text.
  @param padding           [in]  The paddin scheme used for filling empty bytes after encryption.

  @param pCipherText       [out] The encrypted text.
  @param pCipherTextLen    [out] The length of the encrypted text.
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_RSA_Public_Encrypt(
    OpcUa_CryptoProvider*   pProvider,
    OpcUa_Byte*             pPlainText,
    OpcUa_UInt32            plainTextLen,
    OpcUa_Key*              publicKey,
    OpcUa_Int16             padding,
    OpcUa_Byte*             pCipherText,
    OpcUa_UInt32*           pCipherTextLen);

/**
  @brief Decrypts encrypted data using <NAME>(RSA) with the private key of the appropriate key pair.

  synchonous!

  @param pProvider         [in]  The crypto provider handle.
  @param pCipherText       [in]  The cipher text to decrypt.
  @param cipherTextLen     [in]  The length of the cipher text to decrypt.
  @param privateKey        [in]  The private key used to decrypt the plain text.
  @param padding           [in]  The paddin scheme used for filling empty bytes after encryption.

  @param pPlainText        [out] The decrypted text.
  @param pPlainTextLen     [out] The length of the decrypted text.
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_RSA_Private_Decrypt(
    OpcUa_CryptoProvider*   pProvider,
    OpcUa_Byte*             pCipherText,
    OpcUa_UInt32            cipherTextLen,
    OpcUa_Key*              privateKey,
    OpcUa_Int16             padding,
    OpcUa_Byte*             pPlainText,
    OpcUa_UInt32*           pPlainTextLen);


/*** RSA ASYMMETRIC SIGNATURE ***/

/**
  @brief Signs data using <NAME>(RSA) with the private key of the appropriate key pair.

  @param pProvider         [in]  The crypto provider handle.
  @param data              [in]  The data to sign.
  @param privateKey        [in]  The private key used to sign the data.
  @param padding           [in]  The paddin scheme used for filling empty bytes after encryption. (e.g. RSA_PKCS1_PADDING)

  @param pSignature        [out] The signature of the data.
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_RSA_Private_Sign(
    OpcUa_CryptoProvider* pProvider,
    OpcUa_ByteString      data,
    OpcUa_Key*            privateKey,
    OpcUa_Int16           padding,
    OpcUa_ByteString*     pSignature);

/**
  @brief Verifies signed data using <NAME>(RSA) with the public key of the appropriate key pair.

  @param pProvider                  [in]  The crypto provider handle.
  @param data                       [in]  The data that was signed.
  @param publicKey                  [in]  The public key used to verify the signature.
  @param padding                    [in]  The padding scheme used for filling empty bytes after encryption.
  @param pSignature                  [in]  The signature of the data that should be verified.
 */
OpcUa_StatusCode OpcUa_P_OpenSSL_RSA_Public_Verify(
    OpcUa_CryptoProvider* pProvider,
    OpcUa_ByteString      data,
    OpcUa_Key*            publicKey,
    OpcUa_Int16           padding,
    OpcUa_ByteString*     pSignature);


/*** ECDSA ASYMMETRIC SIGNATURE ***/

/**
  @brief
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_ECDSA_GenerateKeys(
    OpcUa_CryptoProvider*   pProvider,
    OpcUa_UInt32            bytes,
    OpcUa_Key*              pPublicKey,
    OpcUa_Key*              pPrivaeKey);

/**
  @brief Signs data using Elliptic Curves Digital Signature Algorithm(ECDSA) with the private key of the appropriate key pair
  and a Elliptic Curves Signature.

  @param pProvider                  [in]  The crypto provider handle.
  @param data                       [in]  The data that has to be signed.
  @param privateKey                 [in]  The private key to sign the data.
  @param padding                    [in]  The padding scheme used for filling empty bytes after signing. (not used for ECDSA)

  @param pSignature                 [out]  The signature of the data.
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_ECDSA_Private_Sign(
    OpcUa_CryptoProvider* pProvider,
    OpcUa_ByteString      data,
    OpcUa_Key*            privateKey,
    OpcUa_Int16           padding,
    OpcUa_ByteString*     pSignature);

/**
  @brief Verifies signed data using Elliptic Curves Digital Signature Algorithm(ECDSA) with the public key of the appropriate key pair
  and a Elliptic Curves Signature.

  @param pProvider                  [in]  The crypto provider handle.
  @param data                       [in]  The data that was signed.
  @param publicKey                  [in]  The public key to verify the signature.
  @param padding                    [in]  The padding scheme used for filling empty bytes after signing. (not used for ECDSA)
  @param signature                  [in]  The signature of data.
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_ECDSA_Public_Verify(
    OpcUa_CryptoProvider* pProvider,
    OpcUa_ByteString      data,
    OpcUa_Key*            publicKey,
    OpcUa_Int16           padding,
    OpcUa_ByteString      signature);

/**
  @brief Generates a session key using secret input data.

    if keyLen > 0 then random data of the given length is generated.
    if keyLen == 0 then nothing will be generated.
    if keyLen < 0 then default setting from the CryptoProvider is used.

  @param pProvider        [in]  The crypto provider handle.
  @param secret           [in]  The secret information to create a random key. (clientnonce | servernonce, servernonce | clientnonce)
  @param seed             [in]  The seed to create a random key. (seed)
  @param keyLen           [in]  The desired length of the random key. (output len)

  @param pKey             [out] The derived random key.
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_Random_Key_Derive(
    OpcUa_CryptoProvider* pProvider,
    OpcUa_ByteString      secret,
    OpcUa_ByteString      seed,
    OpcUa_Int32           keyLen,
    OpcUa_Key*            pKey);

/**
  @brief Adds random data to the destination buffer..

    if keyLen > 0 then random data of the given length is generated.
    if keyLen == 0 then nothing will be generated.
    if keyLen < 0 then default setting from the CryptoProvider is used.

    if there are no default settings then an error is returned.

  @param pProvider        [in]  The crypto provider handle.
  @param keyLen           [in]  The desired length of the random key.

  @param pKey             [out] The generated random key.
 */
OpcUa_StatusCode OpcUa_P_OpenSSL_Random_Key_Generate(
    OpcUa_CryptoProvider* pProvider,
    OpcUa_Int32           keyLen,
    OpcUa_Key*            pKey);


/*** MESSAGE DIGEST ***/

/**
  @brief Generates a 20 Bytes message digest of the given input buffer.

  SHA-1: 160 Bits output

  synchronous!

  @param pProvider        [in]  The crypto provider handle.
  @param pData            [in]  The data for the hash generation.
  @param dataLen          [in]  The length of the data for the hash generation.

  @param pMessageDigest   [out] The resulting message digest (hash).
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_SHA1_Generate(
    OpcUa_CryptoProvider*         pProvider,
    OpcUa_Byte*                   pData,
    OpcUa_UInt32                  dataLen,
    OpcUa_Byte*                   pMessageDigest);

/**
  @brief Generates variant bytes message digest of the given input buffer.

  SHA-1: 160 Bits output

  synchronous!

  @param pProvider        [in]  The crypto provider handle.
  @param pData            [in]  The data for the hash generation.
  @param dataLen          [in]  The length data for the hash generation.

  @param pMessageDigest   [out] The resulting message digest (hash).
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_SHA1_160_Generate(
    OpcUa_CryptoProvider*         pProvider,
    OpcUa_Byte*                   pData,
    OpcUa_UInt32                  dataLen,
    OpcUa_Byte*                   pMessageDigest);

/**
  @brief Generates variant bytes message digest of the given input buffer.

  SHA-2: 224 Bits output

  synchronous!

  @param pProvider        [in]  The crypto provider handle.
  @param pData            [in]  The data for the hash generation.
  @param dataLen          [in]  The length data for the hash generation.

  @param pMessageDigest   [out] The resulting message digest (hash).
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_SHA2_224_Generate(
    OpcUa_CryptoProvider*         pProvider,
    OpcUa_Byte*                   pData,
    OpcUa_UInt32                  dataLen,
    OpcUa_Byte*                   pMessageDigest);

/**
  @brief Generates variant bytes message digest of the given input buffer.

  SHA-2: 256 Bits output

  synchronous!

  @param pProvider        [in]  The crypto provider handle.
  @param pData            [in]  The data for the hash generation.
  @param dataLen          [in]  The length data for the hash generation.

  @param pMessageDigest   [out] The resulting message digest (hash).
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_SHA2_256_Generate(
    OpcUa_CryptoProvider*         pProvider,
    OpcUa_Byte*                   pData,
    OpcUa_UInt32                  dataLen,
    OpcUa_Byte*                   pMessageDigest);

/**
  @brief Generates variant bytes message digest of the given input buffer.

  SHA-2: 384 Bits output

  synchronous!

  @param pProvider        [in]  The crypto provider handle.
  @param pData            [in]  The data for the hash generation.
  @param dataLen          [in]  The length data for the hash generation.

  @param pMessageDigest   [out] The resulting message digest (hash).
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_SHA2_384_Generate(
    OpcUa_CryptoProvider*         pProvider,
    OpcUa_Byte*                   pData,
    OpcUa_UInt32                  dataLen,
    OpcUa_Byte*                   pMessageDigest);

/**
  @brief Generates variant bytes message digest of the given input buffer.

  SHA-2: 512 Bits output

  synchronous!

  @param pProvider        [in]  The crypto provider handle.
  @param pData            [in]  The data for the hash generation.
  @param dataLen          [in]  The length data for the hash generation.

  @param pMessageDigest   [out] The resulting message digest (hash).
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_SHA2_512_Generate(
    OpcUa_CryptoProvider*         pProvider,
    OpcUa_Byte*                   pData,
    OpcUa_UInt32                  dataLen,
    OpcUa_Byte*                   pMessageDigest);


/*** MESSAGE AUTHENTICATION CODE ***/

/**
  @brief Generates s 20 Bytes Message Authentication Code (MAC) of the given input buffer and a secret key.

  HMAC-SHA-1: 160 Bits output

  synchronous!

  @param pProvider        [in]  The crypto provider handle.
  @param pData            [in]  The data for the MAC generation.
  @param dataLen          [in]  The length data for the MAC generation.
  @param key              [in]  The key for the MAC generation.

  @param pMac             [out] The resulting messsage authentication code (MAC).
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_HMAC_SHA1_Generate(
    OpcUa_CryptoProvider* pProvider,
    OpcUa_Byte*           pData,
    OpcUa_UInt32          dataLen,
    OpcUa_Key*            key,
    OpcUa_ByteString*     pMac);

/**
  @brief Generates s variant Bytes Message Authentication Code (MAC) of the given input buffer and a secret key.

  HMAC-SHA-1: 160 Bits output

  synchronous!

  @param pProvider        [in]  The crypto provider handle.
  @param pData            [in]  The data for the MAC generation.
  @param dataLen          [in]  The length data for the MAC generation.
  @param key              [in]  The key for the MAC generation.

  @param pMac             [out] The resulting messsage authentication code (MAC).
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_HMAC_SHA1_160_Generate(
    OpcUa_CryptoProvider* pProvider,
    OpcUa_Byte*           pData,
    OpcUa_UInt32          dataLen,
    OpcUa_Key             key,
    OpcUa_ByteString*     pMac);

/**
  @brief Generates s variant Bytes Message Authentication Code (MAC) of the given input buffer and a secret key.

  HMAC-SHA-2: 224 Bits output

  synchronous!

  @param pProvider        [in]  The crypto provider handle.
  @param pData            [in]  The data for the MAC generation.
  @param dataLen          [in]  The length data for the MAC generation.
  @param key              [in]  The key for the MAC generation.

  @param pMac             [out] The resulting messsage authentication code (MAC).
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_HMAC_SHA2_224_Generate(
    OpcUa_CryptoProvider* pProvider,
    OpcUa_Byte*           pData,
    OpcUa_UInt32          dataLen,
    OpcUa_Key             key,
    OpcUa_ByteString*     pMac);

/**
  @brief Generates s variant Bytes Message Authentication Code (MAC) of the given input buffer and a secret key.

  HMAC-SHA-2: 256 Bits output

  synchronous!

  @param pProvider        [in]  The crypto provider handle.
  @param pData            [in]  The data for the MAC generation.
  @param dataLen          [in]  The length data for the MAC generation.
  @param key              [in]  The key for the MAC generation.

  @param pMac             [out] The resulting messsage authentication code (MAC).
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_HMAC_SHA2_256_Generate(
    OpcUa_CryptoProvider* pProvider,
    OpcUa_Byte*           pData,
    OpcUa_UInt32          dataLen,
    OpcUa_Key*            key,
    OpcUa_ByteString*     pMac);

/**
  @brief Generates s variant Bytes Message Authentication Code (MAC) of the given input buffer and a secret key.

  HMAC-SHA-2: 384 Bits output

  synchronous!

  @param pProvider        [in]  The crypto provider handle.
  @param pData            [in]  The data for the MAC generation.
  @param dataLen          [in]  The length data for the MAC generation.
  @param key              [in]  The key for the MAC generation.

  @param pMac             [out] The resulting messsage authentication code (MAC).
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_HMAC_SHA2_384_Generate(
    OpcUa_CryptoProvider* pProvider,
    OpcUa_Byte*           pData,
    OpcUa_UInt32          dataLen,
    OpcUa_Key             key,
    OpcUa_ByteString*     pMac);

/**
  @brief Generates s variant Bytes Message Authentication Code (MAC) of the given input buffer and a secret key.

  HMAC-SHA-2: 512 Bits output

  synchronous!

  @param pProvider        [in]  The crypto provider handle.
  @param pData            [in]  The data for the MAC generation.
  @param dataLen          [in]  The length data for the MAC generation.
  @param key              [in]  The key for the MAC generation.

  @param pMac             [out] The resulting messsage authentication code (MAC).
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_HMAC_SHA2_512_Generate(
    OpcUa_CryptoProvider* pProvider,
    OpcUa_Byte*           pData,
    OpcUa_UInt32          dataLen,
    OpcUa_Key             key,
    OpcUa_ByteString*     pMac);


OpcUa_StatusCode OpcUa_P_OpenSSL_3DES_Encrypt(
    OpcUa_CryptoProvider*   pProvider,
    OpcUa_Byte*             pPlainText,
    OpcUa_UInt32            plainTextLen,
    OpcUa_Key               key,
    OpcUa_Byte*             pInitalVector,
    OpcUa_Byte*             pCipherText,
    OpcUa_UInt32*           pCipherTextLen);

OpcUa_StatusCode OpcUa_P_OpenSSL_3DES_Decrypt(
    OpcUa_CryptoProvider*   pProvider,
    OpcUa_Byte*             pCipherText,
    OpcUa_UInt32            cipherTextLen,
    OpcUa_Key               key,
    OpcUa_Byte*             pInitalVector,
    OpcUa_Byte*             pPlainText,
    OpcUa_UInt32*           pPlainTextLen);


OPCUA_END_EXTERN_C

#endif /* _OpcUa_Crypto_OpenSsl_H_ */
