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

/* UA platform definitions */
#include <opcua_p_internal.h>
#include <opcua_p_string.h>
#include <opcua_p_memory.h>

/* own headers */
#include <opcua_p_cryptofactory.h>

#if OPCUA_REQUIRE_OPENSSL
#include <opcua_p_openssl.h>
#endif /* OPCUA_REQUIRE_OPENSSL */

#if OPCUA_SUPPORT_SECURITYPOLICY_NONE
#include <opcua_p_securitypolicy_none.h>
#endif /* OPCUA_SUPPORT_SECURITYPOLICY_NONE */

#include <opcua_p_wincrypt.h>

/*============================================================================
 * OpcUa_P_CryptoFactory_CreateCryptoProvider
 *===========================================================================*/
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_P_CryptoFactory_CreateCryptoProvider(  OpcUa_StringA           a_Uri,
                                                                            OpcUa_CryptoProvider*   a_pProvider)

{
    OpcUa_CryptoProviderConfig* pConfig = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_P_CryptoFactory, "CreateCryptoProvider");

    OpcUa_ReturnErrorIfArgumentNull(a_pProvider);
    OpcUa_ReturnErrorIfArgumentNull(a_Uri);

    OpcUa_MemSet(a_pProvider, 0, sizeof(OpcUa_CryptoProvider));

    a_pProvider->Name = (OpcUa_StringA)malloc(strlen(a_Uri) + 1);
    OpcUa_GotoErrorIfAllocFailed(a_pProvider->Name);

    OpcUa_P_String_strncpy( a_pProvider->Name,
                            strlen(a_Uri),
                            a_Uri,
                            strlen(a_Uri));

    a_pProvider->Name[strlen(a_Uri)] = '\0';

    /* Check whether a noSecurityPolicy is passed in */
    if(a_Uri == OpcUa_Null)
    {
        OpcUa_GotoErrorWithStatus(OpcUa_BadInvalidArgument);
    }
#if OPCUA_SUPPORT_SECURITYPOLICY_NONE
    else if((OpcUa_P_String_strncmp(OpcUa_SecurityPolicy_None, a_Uri, strlen(OpcUa_SecurityPolicy_None)) == 0))
    {
        a_pProvider->Handle = OpcUa_Null;

        a_pProvider->SymmetricKeyLength         = 0;
        a_pProvider->DerivedEncryptionKeyLength = 0;
        a_pProvider->DerivedSignatureKeyLength  = 0;
        a_pProvider->SignatureDataLength        = 0;

        a_pProvider->MaximumAsymmetricKeyLength = 0;
        a_pProvider->MinimumAsymmetricKeyLength = 0;

        a_pProvider->AsymmetricSignatureAlgorithmId  = 0;
        a_pProvider->AsymmetricEncryptionAlgorithmId = 0;
        a_pProvider->SymmetricSignatureAlgorithmId   = 0;
        a_pProvider->SymmetricEncryptionAlgorithmId  = 0;

        /* asymmetric key generation */
        a_pProvider->GenerateAsymmetricKeypair  = OpcUa_P_Crypto_NoSecurity_GenerateAsymmetricKeyPair;

        /* get the length of an asymmetric key */
        a_pProvider->GetAsymmetricKeyLength     = OpcUa_P_Crypto_NoSecurity_GetAsymmetricKeyLength;

        /* symmetric signature algorithm */
        a_pProvider->SymmetricSign              = OpcUa_P_Crypto_NoSecurity_SymmetricSign;
        a_pProvider->SymmetricVerify            = OpcUa_P_Crypto_NoSecurity_SymmetricVerify;

        /* symmetric encryption algorithm */
        a_pProvider->SymmetricEncrypt           = OpcUa_P_Crypto_NoSecurity_SymmetricEncrypt;
        a_pProvider->SymmetricDecrypt           = OpcUa_P_Crypto_NoSecurity_SymmetricDecrypt;

        /* asymmetric signature algorithm */
        a_pProvider->AsymmetricSign             = OpcUa_P_Crypto_NoSecurity_AsymmetricSign;
        a_pProvider->AsymmetricVerify           = OpcUa_P_Crypto_NoSecurity_AsymmetricVerify;

        /* asymmetric encryption algorithm */
        a_pProvider->AsymmetricEncrypt          = OpcUa_P_Crypto_NoSecurity_AsymmetricEncrypt;
        a_pProvider->AsymmetricDecrypt          = OpcUa_P_Crypto_NoSecurity_AsymmetricDecrypt;

        /* key derivation algorithm */
        a_pProvider->DeriveChannelKeysets       = OpcUa_P_Crypto_NoSecurity_DeriveChannelKeysets;
        a_pProvider->DeriveKey                  = OpcUa_P_Crypto_NoSecurity_DeriveKey;

        /* random key generation */
        a_pProvider->GenerateKey                = OpcUa_P_Crypto_NoSecurity_GenerateKey;

        /* certificate functions */
        a_pProvider->CreateCertificate          = OpcUa_P_Crypto_NoSecurity_CreateCertificate;
        a_pProvider->GetPrivateKeyFromCert      = OpcUa_P_Crypto_NoSecurity_GetPrivateKeyFromCert;
        a_pProvider->GetPublicKeyFromCert       = OpcUa_P_Crypto_NoSecurity_GetPublicKeyFromCert;
        a_pProvider->GetSignatureFromCert       = OpcUa_P_Crypto_NoSecurity_GetSignatureFromCert;
        a_pProvider->GetCertificateThumbprint   = OpcUa_P_Crypto_NoSecurity_GetCertificateThumbprint;
    }
#endif /* OPCUA_SUPPORT_SECURITYPOLICY_NONE */
#if OPCUA_SUPPORT_SECURITYPOLICY_BASIC128RSA15
    else if(OpcUa_P_String_strncmp( OpcUa_SecurityPolicy_Basic128Rsa15,
                                    a_Uri,
                                    strlen(OpcUa_SecurityPolicy_Basic128Rsa15))==0)
    {
        pConfig = (OpcUa_CryptoProviderConfig*)malloc(sizeof(OpcUa_CryptoProviderConfig));

        pConfig->DerivedEncryptionKeyLength = 16;
        pConfig->DerivedSignatureKeyLength  = 16;
        pConfig->MaximumAsymmetricKeyLength = 512;
        pConfig->MinimumAsymmetricKeyLength = 128;
        pConfig->SymmetricKeyLength         = 16;

        a_pProvider->Handle = (OpcUa_ProviderHandle)pConfig;

        a_pProvider->SymmetricKeyLength         = 16;
        a_pProvider->DerivedEncryptionKeyLength = 16;
        a_pProvider->DerivedSignatureKeyLength  = 16;
        a_pProvider->SignatureDataLength        = 20;

        a_pProvider->MaximumAsymmetricKeyLength = 512;
        a_pProvider->MinimumAsymmetricKeyLength = 128;

        a_pProvider->AsymmetricSignatureAlgorithmId  = OpcUa_P_RSA_PKCS1_V15_SHA1_Id;
        a_pProvider->AsymmetricEncryptionAlgorithmId = OpcUa_P_RSA_PKCS1_V15_Id;
        a_pProvider->SymmetricSignatureAlgorithmId   = OpcUa_P_HMAC_SHA1_Id;
        a_pProvider->SymmetricEncryptionAlgorithmId  = OpcUa_P_AES_128_CBC_Id;

        pConfig = OpcUa_Null;

        /* asymmetric key generation */
        a_pProvider->GenerateAsymmetricKeypair  = OpcUa_P_OpenSSL_GenerateAsymmetricKeyPair;

        /* get the length of an asymmetric key */
        a_pProvider->GetAsymmetricKeyLength     = OpcUa_P_OpenSSL_RSA_Public_GetKeyLength;

        /* symmetric signature algorithm */
        a_pProvider->SymmetricSign              = OpcUa_P_OpenSSL_HMAC_SHA1_Sign;
        a_pProvider->SymmetricVerify            = OpcUa_P_OpenSSL_HMAC_SHA1_Verify;

        /* symmetric encryption algorithm */
        a_pProvider->SymmetricEncrypt           = OpcUa_P_OpenSSL_AES_128_CBC_Encrypt;
        a_pProvider->SymmetricDecrypt           = OpcUa_P_OpenSSL_AES_128_CBC_Decrypt;

        /* asymmetric signature algorithm */
        a_pProvider->AsymmetricSign             = OpcUa_P_OpenSSL_RSA_PKCS1_V15_SHA1_Sign;
        a_pProvider->AsymmetricVerify           = OpcUa_P_OpenSSL_RSA_PKCS1_V15_SHA1_Verify;

        /* asymmetric encryption algorithm */
        a_pProvider->AsymmetricEncrypt          = OpcUa_P_OpenSSL_RSA_PKCS1_V15_Encrypt;
        a_pProvider->AsymmetricDecrypt          = OpcUa_P_OpenSSL_RSA_PKCS1_V15_Decrypt;

        /* key derivation algorithm */
        a_pProvider->DeriveChannelKeysets       = OpcUa_P_OpenSSL_DeriveChannelKeysets;
        a_pProvider->DeriveKey                  = OpcUa_P_OpenSSL_Random_Key_Derive;

        /* random key generation */
        // TODO : Wincrypt Vs OpenSSL analyser
#ifdef WIN32
        a_pProvider->GenerateKey                = OpcUa_P_WinCrypt_Random_Key_Generate;
#endif
#ifdef _GNUC_
        a_pProvider->GenerateKey				= OpcUa_P_OpenSSL_Random_Key_Generate;
#endif

        /* certificate functions */
        a_pProvider->CreateCertificate          = OpcUa_P_OpenSSL_X509_SelfSigned_Custom_Create;
        a_pProvider->GetPrivateKeyFromCert      = OpcUa_P_OpenSSL_X509_GetPrivateKey;
        a_pProvider->GetPublicKeyFromCert       = OpcUa_P_OpenSSL_X509_GetPublicKey;
        a_pProvider->GetSignatureFromCert       = OpcUa_P_OpenSSL_X509_GetSignature;
        a_pProvider->GetCertificateThumbprint   = OpcUa_P_OpenSSL_X509_GetCertificateThumbprint;
    }
#endif /* OPCUA_SUPPORT_SECURITYPOLICY_BASIC128RSA15 */
#if OPCUA_SUPPORT_SECURITYPOLICY_BASIC256
    else if(OpcUa_P_String_strncmp( OpcUa_SecurityPolicy_Basic256,
                                    a_Uri,
                                    strlen(OpcUa_SecurityPolicy_Basic256))==0)
    {
        pConfig = (OpcUa_CryptoProviderConfig*)malloc(sizeof(OpcUa_CryptoProviderConfig));

        pConfig->DerivedEncryptionKeyLength = 32;
        pConfig->DerivedSignatureKeyLength  = 24;
        pConfig->MaximumAsymmetricKeyLength = 512;
        pConfig->MinimumAsymmetricKeyLength = 128;
        pConfig->SymmetricKeyLength         = 32;

        a_pProvider->Handle = (OpcUa_ProviderHandle)pConfig;

        a_pProvider->SymmetricKeyLength         = 32;
        a_pProvider->DerivedEncryptionKeyLength = 32;
        a_pProvider->DerivedSignatureKeyLength  = 24;
        a_pProvider->SignatureDataLength        = 20;

        a_pProvider->MaximumAsymmetricKeyLength = 512;
        a_pProvider->MinimumAsymmetricKeyLength = 128;

        a_pProvider->AsymmetricSignatureAlgorithmId  = OpcUa_P_RSA_PKCS1_OAEP_SHA1_Id;
        a_pProvider->AsymmetricEncryptionAlgorithmId = OpcUa_P_RSA_OAEP_Id;
        a_pProvider->SymmetricSignatureAlgorithmId   = OpcUa_P_HMAC_SHA1_Id;
        a_pProvider->SymmetricEncryptionAlgorithmId  = OpcUa_P_AES_256_CBC_Id;

        pConfig = OpcUa_Null;

        /* asymmetric key generation */
        a_pProvider->GenerateAsymmetricKeypair  = OpcUa_P_OpenSSL_GenerateAsymmetricKeyPair;

        /* get the length of an asymmetric key */
        a_pProvider->GetAsymmetricKeyLength     = OpcUa_P_OpenSSL_RSA_Public_GetKeyLength;

        /* symmetric signature algorithm */
        a_pProvider->SymmetricSign              = OpcUa_P_OpenSSL_HMAC_SHA1_Sign;
        a_pProvider->SymmetricVerify            = OpcUa_P_OpenSSL_HMAC_SHA1_Verify;

        /* symmetric encryption algorithm */
        a_pProvider->SymmetricEncrypt           = OpcUa_P_OpenSSL_AES_256_CBC_Encrypt;
        a_pProvider->SymmetricDecrypt           = OpcUa_P_OpenSSL_AES_256_CBC_Decrypt;

        /* asymmetric signature algorithm */
        a_pProvider->AsymmetricSign             = OpcUa_P_OpenSSL_RSA_PKCS1_OAEP_SHA1_Sign;
        a_pProvider->AsymmetricVerify           = OpcUa_P_OpenSSL_RSA_PKCS1_OAEP_SHA1_Verify;

        /* asymmetric encryption algorithm */
        a_pProvider->AsymmetricEncrypt          = OpcUa_P_OpenSSL_RSA_OAEP_Encrypt;
        a_pProvider->AsymmetricDecrypt          = OpcUa_P_OpenSSL_RSA_OAEP_Decrypt;

        /* key derivation algorithm */
        a_pProvider->DeriveChannelKeysets       = OpcUa_P_OpenSSL_DeriveChannelKeysets;
        a_pProvider->DeriveKey                  = OpcUa_P_OpenSSL_Random_Key_Derive;

        /* random key generation */
        // TODO : Wincrypt Vs OpenSSL : à analyser
#ifdef WIN32
        a_pProvider->GenerateKey                = OpcUa_P_WinCrypt_Random_Key_Generate;
#endif
#ifdef _GNUC_
        a_pProvider->GenerateKey					= OpcUa_P_OpenSSL_Random_Key_Generate;
#endif

        /* certificate functions */
        a_pProvider->CreateCertificate          = OpcUa_P_OpenSSL_X509_SelfSigned_Custom_Create;
        a_pProvider->GetPrivateKeyFromCert      = OpcUa_P_OpenSSL_X509_GetPrivateKey;
        a_pProvider->GetPublicKeyFromCert       = OpcUa_P_OpenSSL_X509_GetPublicKey;
        a_pProvider->GetSignatureFromCert       = OpcUa_P_OpenSSL_X509_GetSignature;
        a_pProvider->GetCertificateThumbprint   = OpcUa_P_OpenSSL_X509_GetCertificateThumbprint;
    }
#endif /* OPCUA_SUPPORT_SECURITYPOLICY_BASIC256 */
    else
    {
        uStatus = OpcUa_BadSecurityPolicyRejected;
        OpcUa_GotoError;
    }

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    if(a_pProvider->Handle != OpcUa_Null)
    {
        free(a_pProvider->Handle);
        a_pProvider->Handle = OpcUa_Null;
    }

    if(a_pProvider->Name != OpcUa_Null)
    {
        free(a_pProvider->Name);
        a_pProvider->Name = OpcUa_Null;
    }

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_P_CryptoFactory_DeleteCryptoProvider
 *===========================================================================*/
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_P_CryptoFactory_DeleteCryptoProvider(OpcUa_CryptoProvider* a_pProvider)
{
OpcUa_InitializeStatus(OpcUa_Module_P_CryptoFactory, "DeleteCryptoProvider");

    OpcUa_ReturnErrorIfArgumentNull(a_pProvider);

    if(a_pProvider->Handle != OpcUa_Null)
    {
        OpcUa_P_Memory_Free(a_pProvider->Handle);
        a_pProvider->Handle = OpcUa_Null;
    }

    if(a_pProvider->Name != OpcUa_Null)
    {
        OpcUa_P_Memory_Free(a_pProvider->Name);
        a_pProvider->Name = OpcUa_Null;
    }

    /* asymmetric key generation */
    a_pProvider->GenerateAsymmetricKeypair    = OpcUa_Null;

    /* symmetric signature algorithm */
    a_pProvider->SymmetricSign                = OpcUa_Null;
    a_pProvider->SymmetricVerify              = OpcUa_Null;

    /* symmetric encryption algorithm */
    a_pProvider->SymmetricEncrypt             = OpcUa_Null;
    a_pProvider->SymmetricDecrypt             = OpcUa_Null;

    /* asymmetric signature algorithm */
    a_pProvider->AsymmetricSign               = OpcUa_Null;
    a_pProvider->AsymmetricVerify             = OpcUa_Null;

    /* asymmetric encryption algorithm */
    a_pProvider->AsymmetricEncrypt            = OpcUa_Null;
    a_pProvider->AsymmetricDecrypt            = OpcUa_Null;

    /* key derivation algorithm */
    a_pProvider->DeriveChannelKeysets         = OpcUa_Null;
    a_pProvider->DeriveKey                    = OpcUa_Null;

    /* random key generation */
    a_pProvider->GenerateKey                  = OpcUa_Null;

    /* certificate functions */
    a_pProvider->CreateCertificate            = OpcUa_Null;
    a_pProvider->GetPrivateKeyFromCert        = OpcUa_Null;
    a_pProvider->GetPublicKeyFromCert         = OpcUa_Null;
    a_pProvider->GetSignatureFromCert         = OpcUa_Null;

    a_pProvider->GetCertificateThumbprint     = OpcUa_Null;

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}
