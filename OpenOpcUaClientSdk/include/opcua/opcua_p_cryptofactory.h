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

#ifndef _OpcUa_P_CryptoFactory_H_
#define _OpcUa_P_CryptoFactory_H_ 1

OPCUA_BEGIN_EXTERN_C

/**
  @brief OpcUa_CryptoFactory_CreateCryptoProvider.

  @param securityPolicy     [in]  The security policy.
  @param pProvider          [out] The resulting CryptoProvider.
*/
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_P_CryptoFactory_CreateCryptoProvider(  OpcUa_StringA           Uri,
                                                                            OpcUa_CryptoProvider*   pProvider);

/**
  @brief OpcUa_CryptoFactory_DeleteCryptoProvider.

  @param pProvider         [out] The resulting CryptoProvider.
*/
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_P_CryptoFactory_DeleteCryptoProvider(  OpcUa_CryptoProvider* pProvider);

/*============================================================================
 * The OpcUa_CryptoProvider interface.
 *===========================================================================*/

typedef struct _OpcUa_CryptoProviderConfig
{
    OpcUa_Int32  SymmetricKeyLength;
    OpcUa_UInt32 MinimumAsymmetricKeyLength;
    OpcUa_UInt32 MaximumAsymmetricKeyLength;
    OpcUa_UInt32 DerivedEncryptionKeyLength;
    OpcUa_UInt32 DerivedSignatureKeyLength;

    OpcUa_Void*  WindowsCryptoProvHandle;   /* Windows CryptoProvider Handle ->wincrypt.h */
}
OpcUa_CryptoProviderConfig;


OPCUA_END_EXTERN_C

#endif
