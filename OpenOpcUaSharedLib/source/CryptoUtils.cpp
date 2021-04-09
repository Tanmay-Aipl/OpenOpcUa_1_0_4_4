/*****************************************************************************
      Author
        ©. Michel Condemine, 4CE Industry (2010-2012)
      
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

#include "stdafx.h"
#include "CryptoUtils.h"
using namespace OpenOpcUa;
using namespace UASharedLib;
//============================================================================
// Utils::VerifySignature
//============================================================================
// Types utilisés par cette methode
//typedef struct _OpcUa_SignatureData
//{
//    OpcUa_String     Algorithm;
//    OpcUa_ByteString Signature;
//}
//OpcUa_SignatureData;
void CryptoUtils::VerifySignature(
	OpcUa_CryptoProvider* pProvider,  // in fournisseur de crypto. Il s'agit d'un ensemble de PTR de fonctions utilisé pour le crpt(ici OpenSSL API)
	const OpcUa_ByteString* pReceiverCertificate, // certificat de l'application a verifier (client ou serveur)
	const OpcUa_ByteString* pNonce, 
	const OpcUa_ByteString* pSigningCertificate, 
	const OpcUa_SignatureData* pSignature)
{
    OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_Key tKey;
	OpcUa_ByteString tData;
	std::vector<unsigned char> data;

    try
    {
	    OpcUa_Key_Initialize(&tKey);
	    OpcUa_ByteString_Initialize(&tData);

	    // determine the length of the public key.
	    uStatus = OpcUa_Crypto_GetPublicKeyFromCert(
		    pProvider,
		    (OpcUa_ByteString*)pSigningCertificate,
		    0,
		    &tKey);

        ThrowIfBad(uStatus, (OpcUa_CharA*)"Could not get the size of the public key from the certificate.");

	    tKey.Key.Data = (OpcUa_Byte*)OpcUa_Alloc(tKey.Key.Length);
	    OpcUa_MemSet(tKey.Key.Data, 0, tKey.Key.Length);

	    // extract the public key from the certificate.
	    uStatus = OpcUa_Crypto_GetPublicKeyFromCert(
		    pProvider,
		    (OpcUa_ByteString*)pSigningCertificate,
		    0,
		    &tKey);

        ThrowIfBad(uStatus, (OpcUa_CharA*)"Could not get the public key from the certificate.");

	    // append the nonce to the certificate data.
	    data.reserve(pReceiverCertificate->Length + pNonce->Length);

	    for (int ii = 0; ii < pReceiverCertificate->Length; ii++)
	    {
		    data.push_back(pReceiverCertificate->Data[ii]);
	    }
    	
	    for (int ii = 0; ii < pNonce->Length; ii++)
	    {
		    data.push_back(pNonce->Data[ii]);
	    }

	    tData = Utils::Copy(data);

	    // verify signature.
	    uStatus = OpcUa_Crypto_AsymmetricVerify(
		    pProvider,
		    tData,
		    &tKey,
		    (OpcUa_ByteString*)&pSignature->Signature);
    	
        if (uStatus!=OpcUa_Good)
			OpcUa_Trace(OPCUA_TRACE_EXTRA_LEVEL_ERROR, "OpcUa_Crypto_AsymmetricVerify failed>OpenSSL Could not verify digital signature.");

	    OpcUa_Key_Clear(&tKey);
	    OpcUa_ByteString_Clear(&tData);
    }
    catch (...)
    {
	    OpcUa_Key_Clear(&tKey);
	    OpcUa_ByteString_Clear(&tData);
        throw;
    }
}

//============================================================================
// Utils::CreateSignature
//============================================================================
OpcUa_StatusCode CryptoUtils::CreateSignature(
	OpcUa_CryptoProvider* pProvider, 
	const OpcUa_ByteString* pReceiverCertificate, 
	const OpcUa_ByteString* pNonce, 
	const OpcUa_ByteString* pSigningCertificate, 
	const OpcUa_Key* pSigningPrivateKey, 
	OpcUa_SignatureData* pSignature)
{
    OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_Key tKey;
	OpcUa_ByteString tData;
	std::vector<unsigned char> data;
	OpcUa_UInt32 uKeySize = 0;
	if (pProvider)
	{
	    OpcUa_Key_Initialize(&tKey);
	    OpcUa_ByteString_Initialize(&tData);
	    OpcUa_SignatureData_Initialize(pSignature);

	    // determine the length of the public key.
	    uStatus = OpcUa_Crypto_GetPublicKeyFromCert(
		    pProvider,
		    (OpcUa_ByteString*)pSigningCertificate,
		    0,
		    &tKey);
    	if (uStatus!=OpcUa_Good)
			OpcUa_Trace(OPCUA_TRACE_EXTRA_LEVEL_ERROR,"CryptoUtils::CreateSignature>Could not get the size of the public key from the certificate.:0x%05x.",uStatus);
		else
		{
			tKey.Key.Data = (OpcUa_Byte*)OpcUa_Alloc(tKey.Key.Length);
			OpcUa_MemSet(tKey.Key.Data, 0, tKey.Key.Length);

			// extract the public key from the certificate.
			uStatus = OpcUa_Crypto_GetPublicKeyFromCert(
				pProvider,
				(OpcUa_ByteString*)pSigningCertificate,
				0,
				&tKey);
    		if (uStatus!=OpcUa_Good)
				OpcUa_Trace(OPCUA_TRACE_EXTRA_LEVEL_ERROR,"CryptoUtils::CreateSignature>Could not get the public key from the certificate:0x%05x.",uStatus);
			else
			{

				uStatus = OpcUa_Crypto_GetAsymmetricKeyLength(pProvider, tKey, &uKeySize);
    			if (uStatus!=OpcUa_Good)
					OpcUa_Trace(OPCUA_TRACE_EXTRA_LEVEL_ERROR,"CryptoUtils::CreateSignature>Could not get length of the key in the certificate:0x%05x.",uStatus);
				else
				{
					OpcUa_Key_Clear(&tKey);

					// append the nonce to the certificate data.
					data.reserve(pReceiverCertificate->Length + pNonce->Length);

					for (int ii = 0; ii < pReceiverCertificate->Length; ii++)
					{
						data.push_back(pReceiverCertificate->Data[ii]);
					}
			    	
					for (int ii = 0; ii < pNonce->Length; ii++)
					{
						data.push_back(pNonce->Data[ii]);
					}

					tData = Utils::Copy(data);

					// fill in signature information.
					uStatus = OpcUa_String_AttachToString(
						(OpcUa_CharA*)OpcUa_AlgorithmUri_Signature_RsaSha1,
						OPCUA_STRINGLENZEROTERMINATED, 
						0,
						OpcUa_False, 
						OpcUa_False, 
						&pSignature->Algorithm);

    				if (uStatus!=OpcUa_Good)
						OpcUa_Trace(OPCUA_TRACE_EXTRA_LEVEL_ERROR,"CryptoUtils::CreateSignature>OpcUa_String_AttachToString failed:0x%05x.",uStatus);
					else
					{
						// create signature.
						pSignature->Signature.Length = uKeySize/8;
						pSignature->Signature.Data = (OpcUa_Byte*)OpcUa_Alloc(pSignature->Signature.Length);
						OpcUa_MemSet(pSignature->Signature.Data, 0, pSignature->Signature.Length);

						uStatus = OpcUa_Crypto_AsymmetricSign(
							pProvider,
							tData,
							(OpcUa_Key*)pSigningPrivateKey,
							&pSignature->Signature);
    					if (uStatus!=OpcUa_Good)
							OpcUa_Trace(OPCUA_TRACE_EXTRA_LEVEL_ERROR,"CryptoUtils::CreateSignature>Could not create digital signature 0x%05x.",uStatus);
					}
					OpcUa_Key_Clear(&tKey);
					OpcUa_ByteString_Clear(&tData);
				}
			}
		}
	}
	else
		uStatus=OpcUa_BadInvalidArgument;
	return uStatus;
}
