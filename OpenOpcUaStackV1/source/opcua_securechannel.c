/*****************************************************************************
      Author
        �. Michel Condemine, 4CE Industry (2010-2012)
      
      Contributors


	This software is a computer program whose purpose is to 
	        implement behavior describe in the OPC UA specification.
		see wwww.opcfoundation.org for more details about OPC.
	This software is governed by the CeCILL-C license under French law and
	abiding by the rules of distribution of free software.  You can  use, 
	modify and/ or redistribute the software under the terms of the CeCILL-C
	license as circulated by CEA, CNRS and INRIA at the following URL
	"http://www.cecill.info". 

	As a counterpart to the access to the source code and  rights to copy,
	modify and redistribute granted by the license, users are provided only
	with a limited warranty  and the software's author,  the holder of the
	economic rights,  and the successive licensors  have only  limited
	liability. 

	In this respect, the user's attention is drawn to the risks associated
	with loading,  using,  modifying and/or developing or reproducing the
	software by the user in light of its specific status of free software,
	that may mean  that it is complicated to manipulate,  and  that  also
	therefore means  that it is reserved for developers  and  experienced
	professionals having in-depth computer knowledge. Users are therefore
	encouraged to load and test the software's suitability as regards their
	requirements in conditions enabling the security of their systems and/or 
	data to be ensured and,  more generally, to use and operate it in the 
	same conditions as regards security. 

	The fact that you are presently reading this means that you have had
        knowledge of the CeCILL-C license and that you accept its terms.

*****************************************************************************/

#include <opcua.h>
#include <opcua_stream.h>
#include <opcua_securechannel_types.h>
#include <opcua_securechannel.h>
#include <opcua_securestream.h>

/*============================================================================
 * OpcUa_SecureChannel_Open
 *===========================================================================*/
OpcUa_StatusCode OpcUa_SecureChannel_Open(
    OpcUa_SecureChannel*            a_pSecureChannel,
    OpcUa_Handle                    a_hTransportConnection,
    OpcUa_ChannelSecurityToken      a_channelSecurityToken,
    OpcUa_MessageSecurityMode       a_messageSecurityMode,
    OpcUa_ByteString*               a_clientCertificate,
    OpcUa_ByteString*               a_serverCertificate,
    OpcUa_SecurityKeyset*           a_pReceivingKeyset,
    OpcUa_SecurityKeyset*           a_pSendingKeyset,
    OpcUa_CryptoProvider*           a_pCryptoProvider)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_SecureChannel);
    OpcUa_ReturnErrorIfArgumentNull(a_pSecureChannel);

    return a_pSecureChannel->Open(  a_pSecureChannel,
                                    a_hTransportConnection,
                                    a_channelSecurityToken,
                                    a_messageSecurityMode,
                                    a_clientCertificate,
                                    a_serverCertificate,
                                    a_pReceivingKeyset,
                                    a_pSendingKeyset,
                                    a_pCryptoProvider);
}

/*============================================================================
 * OpcUa_SecureChannel_Renew
 *===========================================================================*/
OpcUa_StatusCode OpcUa_SecureChannel_Renew(
    OpcUa_SecureChannel*            a_pSecureChannel,
    OpcUa_Handle                    a_hTransportConnection,
    OpcUa_ChannelSecurityToken      a_channelSecurityToken,
    OpcUa_MessageSecurityMode       a_messageSecurityMode,
    OpcUa_ByteString*               a_clientCertificate,
    OpcUa_ByteString*               a_serverCertificate,
    OpcUa_SecurityKeyset*           a_pReceivingKeyset,
    OpcUa_SecurityKeyset*           a_pSendingKeyset,
    OpcUa_CryptoProvider*           a_pCryptoProvider)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_SecureChannel);
    OpcUa_ReturnErrorIfArgumentNull(a_pSecureChannel);

    return a_pSecureChannel->Renew( a_pSecureChannel,
                                    a_hTransportConnection,
                                    a_channelSecurityToken,
                                    a_messageSecurityMode,
                                    a_clientCertificate,
                                    a_serverCertificate,
                                    a_pReceivingKeyset,
                                    a_pSendingKeyset,
                                    a_pCryptoProvider);
}

/*============================================================================
 * OpcUa_SecureChannel_Close
 *===========================================================================*/
OpcUa_StatusCode OpcUa_SecureChannel_Close( OpcUa_SecureChannel*   a_pSecureChannel)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_SecureChannel);
    OpcUa_ReturnErrorIfArgumentNull(a_pSecureChannel);

    return a_pSecureChannel->Close(a_pSecureChannel);
}

/*============================================================================
 * OpcUa_SecureChannel_GenerateSecurityToken
 *===========================================================================*/
OpcUa_StatusCode OpcUa_SecureChannel_GenerateSecurityToken(
    OpcUa_SecureChannel*            a_pSecureChannel,
    OpcUa_UInt32                     a_tokenLifeTime,
    OpcUa_ChannelSecurityToken**    a_ppSecurityToken)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_SecureChannel);
    OpcUa_ReturnErrorIfArgumentNull(a_pSecureChannel);

    return a_pSecureChannel->GenerateSecurityToken( a_pSecureChannel,
                                                    a_tokenLifeTime,
                                                    a_ppSecurityToken);
}

/*============================================================================
 * OpcUa_SecureChannel_RenewSecurityToken
 *===========================================================================*/
OpcUa_StatusCode OpcUa_SecureChannel_RenewSecurityToken(
    OpcUa_SecureChannel*            a_pSecureChannel,
    OpcUa_ChannelSecurityToken*     a_pSecurityToken,
    OpcUa_UInt32                     a_tokenLifeTime,
    OpcUa_ChannelSecurityToken**    a_ppSecurityToken)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_SecureChannel);
    OpcUa_ReturnErrorIfArgumentNull(a_pSecureChannel);
    OpcUa_ReturnErrorIfArgumentNull(a_pSecurityToken);

    return a_pSecureChannel->RenewSecurityToken(a_pSecureChannel,
                                                a_pSecurityToken,
                                                a_tokenLifeTime,
                                                a_ppSecurityToken);
}

/*============================================================================
 * OpcUa_SecureChannel_GetPendingInputStream
 *===========================================================================*/
OpcUa_StatusCode OpcUa_SecureChannel_GetPendingInputStream(
    OpcUa_SecureChannel* a_pSecureChannel,
    OpcUa_InputStream**  a_ppSecureIStream)
{
    OpcUa_ReferenceParameter(a_pSecureChannel);
    OpcUa_ReferenceParameter(a_ppSecureIStream);

    *a_ppSecureIStream = a_pSecureChannel->pPendingSecureIStream;

    return OpcUa_Good;
}

/*============================================================================
 * OpcUa_SecureChannel_SetPendingInputStream
 *===========================================================================*/
OpcUa_StatusCode OpcUa_SecureChannel_SetPendingInputStream(
    OpcUa_SecureChannel*    a_pSecureChannel,
    OpcUa_InputStream*      a_pSecureIStream)
{
OpcUa_InitializeStatus(OpcUa_Module_SecureChannel, "SetPendingInputStream");

    OpcUa_ReturnErrorIfArgumentNull(a_pSecureChannel);

    a_pSecureChannel->pPendingSecureIStream = a_pSecureIStream;

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_SecureChannel_DeriveKeys
 *===========================================================================*/
/* static helper function */
OpcUa_StatusCode OpcUa_SecureChannel_DeriveKeys(    OpcUa_MessageSecurityMode   uSecurityMode,
                                                    OpcUa_CryptoProvider*       pCryptoProvider,
                                                    OpcUa_ByteString*           pClientNonce,
                                                    OpcUa_ByteString*           pServerNonce,
                                                    OpcUa_SecurityKeyset**      ppClientKeyset,
                                                    OpcUa_SecurityKeyset**      ppServerKeyset)
{
    OpcUa_SecurityKeyset*               pClientKeyset               = OpcUa_Null;
    OpcUa_SecurityKeyset*               pServerKeyset               = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_SecureListener, "GetOpenSecurechannelInputHeaderInformation");

    /* allocate memory for the session key structures */
    pClientKeyset = (OpcUa_SecurityKeyset*)OpcUa_Alloc(sizeof(OpcUa_SecurityKeyset));
    OpcUa_GotoErrorIfAllocFailed(pClientKeyset);

    OpcUa_SecurityKeyset_Initialize(pClientKeyset);

    pServerKeyset = (OpcUa_SecurityKeyset*)OpcUa_Alloc(sizeof(OpcUa_SecurityKeyset));
    OpcUa_GotoErrorIfAllocFailed(pServerKeyset);

    OpcUa_SecurityKeyset_Initialize(pServerKeyset);

    if((uSecurityMode == OpcUa_MessageSecurityMode_SignAndEncrypt) || (uSecurityMode == OpcUa_MessageSecurityMode_Sign))
    {
        /* get the lengths for the keys */
        uStatus = pCryptoProvider->DeriveChannelKeysets(    pCryptoProvider,
                                                            *pClientNonce,  /* client nonce */
                                                            *pServerNonce,  /* server nonce */
                                                            -1,             /* key size default */
                                                            pClientKeyset,  /* outparam */
                                                            pServerKeyset); /* outparam */
        OpcUa_GotoErrorIfBad(uStatus);

        /* allocate memory for keys in the keyset */
        /* client keys */
        pClientKeyset->SigningKey.Key.Data = (OpcUa_Byte*)OpcUa_Alloc(pClientKeyset->SigningKey.Key.Length*sizeof(OpcUa_Byte));
        OpcUa_GotoErrorIfAllocFailed(pClientKeyset->SigningKey.Key.Data);

        pClientKeyset->EncryptionKey.Key.Data = (OpcUa_Byte*)OpcUa_Alloc(pClientKeyset->EncryptionKey.Key.Length*sizeof(OpcUa_Byte));
        OpcUa_GotoErrorIfAllocFailed(pClientKeyset->EncryptionKey.Key.Data);

        pClientKeyset->InitializationVector.Key.Data = (OpcUa_Byte*)OpcUa_Alloc(pClientKeyset->InitializationVector.Key.Length*sizeof(OpcUa_Byte));
        OpcUa_GotoErrorIfAllocFailed(pClientKeyset->InitializationVector.Key.Data);

        /* server keys */
        pServerKeyset->SigningKey.Key.Data = (OpcUa_Byte*)OpcUa_Alloc(pServerKeyset->SigningKey.Key.Length*sizeof(OpcUa_Byte));
        OpcUa_GotoErrorIfAllocFailed(pServerKeyset->SigningKey.Key.Data);

        pServerKeyset->EncryptionKey.Key.Data = (OpcUa_Byte*)OpcUa_Alloc(pServerKeyset->EncryptionKey.Key.Length*sizeof(OpcUa_Byte));
        OpcUa_GotoErrorIfAllocFailed(pServerKeyset->EncryptionKey.Key.Data);

        pServerKeyset->InitializationVector.Key.Data = (OpcUa_Byte*)OpcUa_Alloc(pServerKeyset->InitializationVector.Key.Length*sizeof(OpcUa_Byte));
        OpcUa_GotoErrorIfAllocFailed(pServerKeyset->InitializationVector.Key.Data);

        /* derive keys with settings from cryptoprovider */
        uStatus = pCryptoProvider->DeriveChannelKeysets(    pCryptoProvider,
                                                            *pClientNonce,  /* client nonce */
                                                            *pServerNonce,  /* server nonce */
                                                            -1,             /* key size default */
                                                            pClientKeyset,  /* outparam */
                                                            pServerKeyset); /* outparam */
        OpcUa_GotoErrorIfBad(uStatus);
    }
    else
    {
        /*** OPCUA_SECURECHANNEL_MESSAGESECURITYMODE_NONE ***/

        /* generate fake client and server keyset */
        pClientKeyset->SigningKey.Type                  = OpcUa_Crypto_KeyType_Symmetric;
        pClientKeyset->SigningKey.Key.Length            = 1;
        pClientKeyset->SigningKey.Key.Data              = (OpcUa_Byte*)OpcUa_Alloc(sizeof(OpcUa_Byte));
        OpcUa_GotoErrorIfAllocFailed(pClientKeyset->SigningKey.Key.Data);

        pClientKeyset->EncryptionKey.Type               = OpcUa_Crypto_KeyType_Symmetric;
        pClientKeyset->EncryptionKey.Key.Length         = 1;
        pClientKeyset->EncryptionKey.Key.Data           = (OpcUa_Byte*)OpcUa_Alloc(sizeof(OpcUa_Byte));
        OpcUa_GotoErrorIfAllocFailed(pClientKeyset->EncryptionKey.Key.Data);

        pClientKeyset->InitializationVector.Type        = OpcUa_Crypto_KeyType_Random;
        pClientKeyset->InitializationVector.Key.Length  = 1;
        pClientKeyset->InitializationVector.Key.Data    = (OpcUa_Byte*)OpcUa_Alloc(sizeof(OpcUa_Byte));
        OpcUa_GotoErrorIfAllocFailed(pClientKeyset->InitializationVector.Key.Data);

        pServerKeyset->SigningKey.Type                  = OpcUa_Crypto_KeyType_Symmetric;
        pServerKeyset->SigningKey.Key.Length            = 1;
        pServerKeyset->SigningKey.Key.Data              = (OpcUa_Byte*)OpcUa_Alloc(sizeof(OpcUa_Byte));
        OpcUa_GotoErrorIfAllocFailed(pServerKeyset->SigningKey.Key.Data);

        pServerKeyset->EncryptionKey.Type               = OpcUa_Crypto_KeyType_Symmetric;
        pServerKeyset->EncryptionKey.Key.Length         = 1;
        pServerKeyset->EncryptionKey.Key.Data           = (OpcUa_Byte*)OpcUa_Alloc(sizeof(OpcUa_Byte));
        OpcUa_GotoErrorIfAllocFailed(pServerKeyset->EncryptionKey.Key.Data);

        pServerKeyset->InitializationVector.Type        = OpcUa_Crypto_KeyType_Random;
        pServerKeyset->InitializationVector.Key.Length  = 1;
        pServerKeyset->InitializationVector.Key.Data    = (OpcUa_Byte*)OpcUa_Alloc(sizeof(OpcUa_Byte));
        OpcUa_GotoErrorIfAllocFailed(pServerKeyset->InitializationVector.Key.Data);
   }

    *ppClientKeyset = pClientKeyset;
    *ppServerKeyset = pServerKeyset;

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    OpcUa_SecurityKeyset_Clear(pServerKeyset);
    OpcUa_Free(pServerKeyset);
    OpcUa_SecurityKeyset_Clear(pClientKeyset);
    OpcUa_Free(pClientKeyset);

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_SecureChannel_IsOpen
 *===========================================================================*/
OpcUa_Boolean OpcUa_SecureChannel_IsOpen(OpcUa_SecureChannel* a_pSecureChannel)
{
    if(a_pSecureChannel != OpcUa_Null)
    {
        return (a_pSecureChannel->State == OpcUa_SecureChannelState_Opened)?OpcUa_True:OpcUa_False;
    }
    else
    {
        return OpcUa_False;
    }
}
