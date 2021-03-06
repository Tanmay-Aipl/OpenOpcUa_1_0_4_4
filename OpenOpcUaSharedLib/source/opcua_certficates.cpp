/*****************************************************************************
	  Author
		?. Michel Condemine, 4CE Industry (2010-2012)
	  
	  Contributors
	? Guillaume Lemarchand, Euriware, Linux and Linux Embedded build (2011)
	? Philippe Buchet, JRC (2011) - Windows CE build

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

#define WIN32_LEAN_AND_MEAN
#include "stdafx.h"
#ifdef WIN32
#include <windows.h>
#include <wincrypt.h>
#include <Ws2tcpip.h>
#endif
#ifdef _GNUC_
#include <dlfcn.h>
#endif

#include <string>
#include <vector>
#include <opcua.h>
#include <opcua_core.h>
#include <openssl/x509.h>
#include <Utils.h>
#include <opcua_certificates.h>
#include <opcua_p_binary.h>
#include <opcua_p_utilities.h>
#include <opcua_mutex.h>

#include <openssl/x509v3.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/pkcs12.h>
#include <openssl/conf.h>
#include <openssl/ssl.h>
 
//static char OID_AUTHORITY_KEY_IDENTIFIER[] = { 85, 29, 1 };
static char OID_SUBJECT_ALT_NAME[] = { 85, 29, 7 };
using namespace OpenOpcUa;
using namespace UASharedLib;

/*============================================================================
 * OpcUa_ReadFile
 *===========================================================================*/
// Read public key from certificate
OpcUa_StatusCode OpcUa_ReadFile(OpcUa_StringA a_sFilePath,OpcUa_ByteString* a_pBuffer)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	FILE* pFile = OpcUa_Null;
	BYTE* pBuffer = OpcUa_Null;
	OpcUa_Int iResult = 0;
	BYTE* pPosition = OpcUa_Null;
#ifdef WIN32
	fpos_t iLength = 0;
#endif

#ifdef _GNUC_
	// Sous GNU/Linux ne peux pas pre-initi
	fpos_t iLength;
#endif


	if (a_sFilePath)
	{
		if (a_pBuffer)
		{
			OpcUa_ByteString_Initialize(a_pBuffer);

			// read the file.
			pFile = fopen((const char*)a_sFilePath, "rb");
			if (pFile)
			{
				iResult = fseek(pFile, 0, SEEK_END);
				if (iResult == 0)
				{
					iResult = fgetpos(pFile, &iLength);
					if (!iResult)
						fseek(pFile, 0, SEEK_SET);
				}
				if (!iResult)
				{
					// allocate buffer.
				#ifdef _GNUC_
					pBuffer = (BYTE*)OpcUa_Alloc((OpcUa_UInt32)iLength.__pos);
					memset(pBuffer, 0, (size_t)iLength.__pos);
				#else
					pBuffer = (BYTE*)OpcUa_Alloc((OpcUa_UInt32)iLength);
					memset(pBuffer, 0, (size_t)iLength);
				#endif

					// read blocks.
					pPosition = pBuffer;

					while (pFile != NULL)
					{
				#ifdef _GNUC_
						iResult = fread(pPosition, 1, (size_t)(iLength.__pos-(pPosition-pBuffer)), pFile);
				#else
						iResult = fread(pPosition, 1, (size_t)1, pFile);
				#endif
						if (iResult <= 0)
							break;
						pPosition += iResult;
					}

					fclose(pFile);

					pFile = NULL;
					
					a_pBuffer->Length = pPosition - pBuffer;
					a_pBuffer->Data=(OpcUa_Byte*)OpcUa_Alloc(a_pBuffer->Length);
					OpcUa_MemCpy(a_pBuffer->Data,a_pBuffer->Length, pBuffer,a_pBuffer->Length );
				}
				else
					uStatus=OpcUa_BadDecodingError;
			}
			else
				uStatus=OpcUa_BadDecodingError;
		}
		else
			uStatus=OpcUa_BadInvalidArgument;
	}
	else
		uStatus=OpcUa_BadInvalidArgument;
	if (pFile != OpcUa_Null)
		fclose(pFile);

	if (pBuffer != OpcUa_Null)
		OpcUa_Free(pBuffer);
	return uStatus;    
}

/*============================================================================
 * OpcUa_WriteFile
 *===========================================================================*/
OpcUa_StatusCode OpcUa_WriteFile(
	OpcUa_StringA  a_sFilePath,
	OpcUa_Byte*    a_pBuffer,
	OpcUa_UInt32   a_uBufferLength)
{
	FILE* pFile = NULL;
	int iResult=0;
OpcUa_InitializeStatus(OpcUa_Module_Utilities, "OpcUa_WriteFile");

	OpcUa_ReturnErrorIfArgumentNull(a_sFilePath);
	OpcUa_ReturnErrorIfArgumentNull(a_pBuffer);

	pFile = fopen((const char*)a_sFilePath, "wb");
	if (!pFile)
	{
		OpcUa_GotoErrorWithStatus(OpcUa_BadEncodingError);
	}

	iResult = fwrite(a_pBuffer, 1, (size_t)a_uBufferLength, pFile);

	if (iResult <= 0)
	{
		OpcUa_GotoErrorWithStatus(OpcUa_BadEncodingError);
	}

	fclose(pFile);
	pFile = NULL;
	
OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	if (pFile != OpcUa_Null)
	{
		fclose(pFile);
	}

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_Certificate_CopyStrings
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Certificate_CopyStrings(
	std::vector<OpcUa_String*> src, 
	OpcUa_StringA**          pStrings, 
	OpcUa_UInt32*            pNoOfStrings)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_Int iLength;

	if (pStrings)
	{
		if (pNoOfStrings)
		{
			*pStrings = OpcUa_Null;
			*pNoOfStrings = src.size();

			iLength = src.size()*sizeof(OpcUa_StringA);

			*pStrings = (OpcUa_StringA*)OpcUa_Alloc(iLength);
			if (*pStrings)
			{
				OpcUa_MemSet(*pStrings, 0, iLength);

				for (unsigned int ii = 0; ii < src.size(); ii++)
				{
					iLength = OpcUa_String_StrLen(src[ii])+1;
					(*pStrings)[ii] = (OpcUa_StringA)OpcUa_Alloc(iLength);
					if ((*pStrings)[ii])
						strcpy((*pStrings)[ii], OpcUa_String_GetRawString(src[ii]));
					else
					{
						uStatus=OpcUa_BadOutOfMemory;
						break;
					}
				}
			}
			else
				uStatus=OpcUa_BadOutOfMemory;
		}
		else
			uStatus=OpcUa_BadInvalidArgument;
	}
	else
		uStatus=OpcUa_BadInvalidArgument;
	//
	if (uStatus!=OpcUa_Good)
	{
		if (pStrings)
		{
			if (*pStrings)
			{
				if (pNoOfStrings)
				{
					for (unsigned int ii = 0; ii < *pNoOfStrings; ii++)
					{
						OpcUa_Free((*pStrings)[ii]);
					}
				}
				OpcUa_Free(*pStrings);
				*pStrings = OpcUa_Null;
			}
			if (pNoOfStrings)
				*pNoOfStrings = 0;
		}
	}
	return uStatus;
}

/*============================================================================
 * OpcUa_Certificate_CreateCryptoProviders
 *===========================================================================*/
static OpcUa_StatusCode OpcUa_Certificate_CreateCryptoProviders(
	OpcUa_PKIProvider* a_pPkiProvider,
	OpcUa_CryptoProvider* a_pCryptoProvider)
{
	OpcUa_CertificateStoreConfiguration tPkiConfiguration;
	
OpcUa_InitializeStatus(OpcUa_Module_Crypto, "OpcUa_FindCertificateInWindowsStore");

	OpcUa_ReturnErrorIfArgumentNull(a_pPkiProvider);
	OpcUa_ReturnErrorIfArgumentNull(a_pCryptoProvider);

	OpcUa_MemSet(a_pPkiProvider, 0, sizeof(OpcUa_PKIProvider));
	OpcUa_MemSet(a_pCryptoProvider, 0, sizeof(OpcUa_CryptoProvider));

	// create the certificate in an OpenSSL store.
	tPkiConfiguration.strPkiType						    = (OpcUa_StringA)OPCUA_P_PKI_TYPE_OPENSSL;    
	tPkiConfiguration.uFlags							    = 0;
	tPkiConfiguration.strIssuerCertificateStoreLocation = NULL;
	tPkiConfiguration.strTrustedCertificateListLocation	= NULL;

	uStatus = OpcUa_PKIProvider_Create(&tPkiConfiguration, a_pPkiProvider);
	OpcUa_GotoErrorIfBad(uStatus);

	// create the provider.
	uStatus = OpcUa_CryptoProvider_Create((OpcUa_StringA)OpcUa_SecurityPolicy_Basic128Rsa15, a_pCryptoProvider);
	OpcUa_GotoErrorIfBad(uStatus);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	OpcUa_CryptoProvider_Delete(a_pCryptoProvider);
	OpcUa_PKIProvider_Delete(a_pPkiProvider);

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_Certificate_DeleteCryptoProviders
 *===========================================================================*/
static OpcUa_StatusCode OpcUa_Certificate_DeleteCryptoProviders(
	OpcUa_PKIProvider* a_pPkiProvider,
	OpcUa_CryptoProvider* a_pCryptoProvider)
{
OpcUa_InitializeStatus(OpcUa_Module_Crypto, "OpcUa_Certificate_DeleteCryptoProviders");

	OpcUa_ReturnErrorIfArgumentNull(a_pPkiProvider);
	OpcUa_ReturnErrorIfArgumentNull(a_pCryptoProvider);

	OpcUa_CryptoProvider_Delete(a_pCryptoProvider);
	OpcUa_PKIProvider_Delete(a_pPkiProvider);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	// nothing to do.

OpcUa_FinishErrorHandling;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Opc UA certificate get file path for certificate. </summary>
///
/// <remarks>	Michel, 11/02/2016. </remarks>
///
/// <param name="a_sStorePath">   	Full pathname of the store file. </param>
/// <param name="a_pCertificate"> 	[in,out] If non-null, the certificate. </param>
/// <param name="a_eFileFormat">  	The file format. </param>
/// <param name="a_bCreateAlways">	The create always. </param>
///
/// <returns>	A std::string. </returns>
///-------------------------------------------------------------------------------------------------

static std::string OpcUa_Certificate_GetFilePathForCertificate(
	OpcUa_StringA      a_sStorePath,
	OpcUa_ByteString*  a_pCertificate,
	OpcUa_P_FileFormat a_eFileFormat,
	OpcUa_Boolean      a_bCreateAlways)
{
	OpcUa_StringA sCommonName=NULL;
	OpcUa_StringA sThumbprint=NULL;
#ifdef WIN32
	LPWSTR        wszDirectoryName=NULL;
#endif
	OpcUa_Boolean bCreateDirectory;

	std::string filePath;
	char* pPos=OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_Crypto, "OpcUa_Certificate_GetFilePathForCertificate");

	OpcUa_GotoErrorIfArgumentNull(a_sStorePath);
	OpcUa_GotoErrorIfArgumentNull(a_pCertificate);
	
	uStatus = OpcUa_Certificate_GetCommonName(a_pCertificate, &sCommonName);
	OpcUa_GotoErrorIfBad(uStatus);

	uStatus = OpcUa_Certificate_GetThumbprint(a_pCertificate, &sThumbprint);
	OpcUa_GotoErrorIfBad(uStatus);

	// build file path.
	filePath = a_sStorePath;

	if (a_eFileFormat == OpcUa_Crypto_Encoding_DER)
	{
#ifndef _GNUC_
		filePath += "\\certs\\";
#else
		filePath += "/certs/";
#endif
	}
	else
	{
#ifndef _GNUC_
		filePath += "\\\\private\\";
#else
		filePath += "/private/";
#endif
	}

	if (a_bCreateAlways)
	{
		for (unsigned int ii = 0; ii < filePath.size(); ii++)
		{
			char ch = filePath[ii];

#ifdef WIN32
			if (ch != '/' && ch != '\\')
			{
				continue;
			}
#else
			if (ch != '/')
			{
				continue;
			}
#endif
			// Create a directory according to the content of parent (std::string parent)
			std::string parent = filePath.substr(0, ii);

			if (parent.empty() || parent.size() <= 0 || parent[parent.size()-1] == ':')
			{
				continue;
			}
#ifndef _GNUC_
			wszDirectoryName = NULL;
			OpcUa_String_AtoW((char*)parent.c_str(),(OpcUa_CharW**)&wszDirectoryName);
	#ifdef UNICODE
			bCreateDirectory =(OpcUa_Boolean)CreateDirectory((LPWSTR)wszDirectoryName, NULL);
	#else
			bCreateDirectory =(OpcUa_Boolean)CreateDirectory((LPCSTR)parent.c_str(), NULL);
	#endif
			OpcUa_Free(wszDirectoryName);
#else
			bCreateDirectory =(OpcUa_Boolean)mkdir(parent.c_str(),S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif

			if (!bCreateDirectory)
			{
#ifdef WIN32
				if (ERROR_ALREADY_EXISTS != GetLastError())
				{
					OpcUa_GotoErrorWithStatus(OpcUa_BadUserAccessDenied);
				}
#endif
#ifdef _GNUC_
				if (EEXIST != OpcUa_GetLastError())
				{
					//OpcUa_GotoErrorWithStatus(OpcUa_BadUserAccessDenied);
				}
#endif
			}
		}
	}
	
	// remove any special characters.
	pPos = sCommonName;

	while (*pPos != '\0')
	{
		char* pMatch = (char*)"<>:\"/\\|?*";

		while (*pMatch != '\0')
		{
			if (*pMatch == *pPos)
			{
				*pPos = '+';
				break;
			}

			pMatch++; 
		}

		pPos++;
	}

	filePath += sCommonName;
	filePath += " [";
	filePath += sThumbprint;
	filePath += "]";

	// select the appropriate extension.
	switch(a_eFileFormat)
	{
		case OpcUa_Crypto_Encoding_DER:
		{
			filePath += ".der";
			break;
		}

		case OpcUa_Crypto_Encoding_PEM:
		{
			filePath += ".pem";
			break;
		}

		case OpcUa_Crypto_Encoding_PKCS12:
		{
			filePath += ".pfx";
			break;
		}

		default:
		{
			OpcUa_GotoErrorWithStatus(OpcUa_BadNotSupported);
		}
	}

	OpcUa_Free(sCommonName);
	OpcUa_Free(sThumbprint);

	return filePath;

OpcUa_BeginErrorHandling;

	OpcUa_Free(sCommonName);
	OpcUa_Free(sThumbprint);

	return filePath;
}

/*============================================================================
 * OpcUa_Certificate_Create
 Cette fonction cr?er la cl? priv?e et la cl? public pour les sauvegardes toutes les deux.
 *===========================================================================*/
//OPCUA_EXPORT OpcUa_StatusCode OpcUa_Certificate_Create(
OpcUa_StatusCode OpcUa_Certificate_Create(
	OpcUa_StringA      a_sStorePath,
	OpcUa_StringA      a_sApplicationName,
	OpcUa_StringA      a_sApplicationUri,
	OpcUa_StringA      a_sOrganization,
	OpcUa_StringA      a_sSubjectName,
	OpcUa_UInt32       a_uNoOfDomainNames,//in
	OpcUa_StringA*     a_pDomainNames, // in
	OpcUa_UInt32       a_uKeyType, // in can be OpcUa_Crypto_Rsa_Id (19) or OpcUa_Crypto_Ec_Id (20)
	OpcUa_UInt32       a_uKeySize, // in
	OpcUa_UInt32       a_uLifetimeInMonths, // in
	OpcUa_Boolean      a_bIsCA,
	OpcUa_P_FileFormat a_eFileFormat,
	OpcUa_ByteString*  a_pIssuerCertificate,   
	OpcUa_Key*         a_pIssuerPrivateKey,
	OpcUa_StringA      a_sPassword,
	OpcUa_ByteString*  a_pCertificate,   
	OpcUa_StringA*     a_pCertificateFilePath,  
	OpcUa_Key*         a_pPrivateKey,
	OpcUa_StringA*     a_pPrivateKeyFilePath)
{
	OpcUa_CryptoProvider tCryptoProvider;
	OpcUa_PKIProvider tPkiProvider;
	OpcUa_DateTime tValidFrom;
	OpcUa_DateTime tValidTo;
	OpcUa_Key tPublicKey;
	OpcUa_Crypto_NameEntry* pSubjectNameFields = OpcUa_Null; 
	OpcUa_Crypto_Extension pExtensions[6];        // on en utilisera que 6
	OpcUa_Certificate* pX509Certificate = OpcUa_Null; 
	OpcUa_Certificate* pX509IssuerCertificate = OpcUa_Null;
	
	//
	if (a_pDomainNames)
	{
		for (OpcUa_UInt32 ii = 0; ii < a_uNoOfDomainNames; ii++)
			OpcUa_Trace(OPCUA_TRACE_EXTRA_LEVEL_ERROR, "will create a certificate for DomainName %s ApplicationUri=%s\n", a_pDomainNames[ii], a_sApplicationUri);
	}
	std::string applicationUri;
	std::string subjectAltName;    
	std::vector<std::string> fieldNames;
	std::vector<std::string> fieldValues;
	std::string subjectName;
	OpcUa_StatusCode uStatus=OpcUa_Good;

	OpcUa_ReturnErrorIfArgumentNull(a_sStorePath);
	OpcUa_ReturnErrorIfArgumentNull(a_sApplicationName)
	OpcUa_ReturnErrorIfArgumentNull(a_pCertificate);
	OpcUa_ReturnErrorIfArgumentNull(a_pPrivateKey);

	OpcUa_ByteString_Initialize(a_pCertificate);
	OpcUa_Key_Initialize(a_pPrivateKey);

	if (a_pCertificateFilePath != NULL) 
		*a_pCertificateFilePath = NULL;
	if (a_pPrivateKeyFilePath != NULL) 
		*a_pPrivateKeyFilePath = NULL;

	OpcUa_MemSet(&tCryptoProvider, 0, sizeof(OpcUa_CryptoProvider));
	OpcUa_MemSet(&tPkiProvider, 0, sizeof(OpcUa_PKIProvider));
	OpcUa_Key_Initialize(&tPublicKey);
	OpcUa_MemSet(&pExtensions, 0, sizeof(pExtensions));
	OpcUa_ByteString_Initialize(a_pCertificate);
	OpcUa_Key_Initialize(a_pPrivateKey);

	// set default key type.
	if (a_uKeyType == 0)
		a_uKeyType = OpcUa_Crypto_Rsa_Id;
	// Create the ApplicationUri
	uStatus=OpcUa_CreateApplication_Uri(a_sApplicationName,&applicationUri);

	// remove invalid chars from uri.
	if (applicationUri.size() > 0)
	{
		int length = applicationUri.size();
		std::string updated;

		for (int ii = 0; ii < length; ii++)
		{
			unsigned char ch = applicationUri[ii];

			bool escape = !isprint(ch) || ch == '%' || ch == ',';

			if (escape)
			{
				char szBuf[4];
				OpcUa_SnPrintfA(szBuf, 4, "%%%2X", ch);
				//sprintf_s(szBuf, 4, "%%%2X", ch);
				updated += szBuf;
			}
			else
			{
				if (isspace(ch))
					updated += ' ';
				else
					updated += ch;
			}
		}

		applicationUri = updated;
	}

	// parse the subject name.
	if (a_sSubjectName != OpcUa_Null && strlen(a_sSubjectName) > 0)
	{
		std::string subjectName = a_sSubjectName;

		int length = strlen(a_sSubjectName);

		int start = 0;
		int end = 0;
		bool nameExtracted = false;
		std::string name;
		std::string value;
		
		for (int ii = 0; ii < length;)
		{
			// check if the start of name found.
			if (!nameExtracted)
			{
				// skip leading white space.
				while (ii < length && isspace(a_sSubjectName[ii]))
					ii++;

				start = ii;

				// read name.
				while (ii < length && isalpha(a_sSubjectName[ii]))
					ii++;

				end = ii;

				if (end > start)
				{
					name = subjectName.substr(start, end-start);

					// skip trailing white space.
					while (ii < length && isspace(a_sSubjectName[ii]))
						ii++;

					// move past equal.
					if (ii < length && a_sSubjectName[ii] == '=')
						ii++;
					nameExtracted = true;
				}
			}

			else
			{
				// skip leading white space.
				while (ii < length && isspace(a_sSubjectName[ii]))
					ii++;

				bool quoted = false;

				// check for quote.
				if (ii < length && a_sSubjectName[ii] == '"')
				{
					ii++;
					quoted = true;
				}

				start = ii;

				if (quoted)
				{
					// check for end quote.
					while (ii < length && a_sSubjectName[ii] != '"')
						ii++;

					end = ii;

					// skip trailing white space.
					while (ii < length && isspace(a_sSubjectName[ii]))
						ii++;
				}

				// check for end separator.
				while (ii < length && a_sSubjectName[ii] != '/')
					ii++;

				if (!quoted)
					end = ii;

				if (end > start)
				{
					value = subjectName.substr(start, end-start);
					
					// add the pair to the list.
					fieldNames.push_back(name);
					fieldValues.push_back(value);
					nameExtracted = false;
				}

				ii++;
			}
		}
	}
	// Obtain the domain Name
	std::string domainName;
	OpcUa_StringA pDomainName = OpcUa_Null;
	// look up the domain name for the current machine.
	uStatus = OpcUa_LookupDomainName((OpcUa_StringA)"127.0.0.1", &pDomainName);        
	if (OpcUa_IsBad(uStatus))
	{
		// use the computer name if no domain name.
		CHAR sBuffer[MAX_PATH+1];
		gethostname(sBuffer,MAX_PATH);
		domainName = sBuffer;
	}
	else
	{
		// copy the domain name.
		domainName = pDomainName;
		OpcUa_Free(pDomainName);
	}
	// create a default subject name.
	if (fieldNames.size() == 0)
	{
		fieldNames.push_back("CN");
		fieldValues.push_back(a_sApplicationName);

		// ensure organization is present.
		if (a_sOrganization != NULL && strlen(a_sOrganization) > 0)
		{
			fieldNames.push_back("O");
			fieldValues.push_back(a_sOrganization);
		}

		// ensure domain is present.
		if (!a_bIsCA)
		{
			fieldNames.push_back("DC");
			fieldValues.push_back(domainName);
		}
	}

	// create the provider.
	uStatus = OpcUa_Certificate_CreateCryptoProviders(&tPkiProvider, &tCryptoProvider);
	if (uStatus==OpcUa_Good)
	{
		// set the current date as the start of the validity period.
		tValidFrom = OpcUa_DateTime_UtcNow();

		// ensure the valid from date is in the future.
		LONGLONG llNow = *((LONGLONG*)&tValidFrom);
		tValidFrom = *((OpcUa_DateTime*)(&llNow));

		// add the lifetime to the current time.
		llNow += 30*24*3600*(LONGLONG)a_uLifetimeInMonths*10000000;
		tValidTo = *((OpcUa_DateTime*)(&llNow));
						 
		// determine size of public key.
		uStatus = OpcUa_Crypto_GenerateAsymmetricKeypair(
			&tCryptoProvider,
			a_uKeyType,
			a_uKeySize,
			&tPublicKey,
			a_pPrivateKey);
			
		if (uStatus==OpcUa_Good)
		{
			// allocate public key buffer.
			tPublicKey.Key.Data = (OpcUa_Byte*)OpcUa_Alloc(tPublicKey.Key.Length);
			if (tPublicKey.Key.Data==NULL)
				uStatus=OpcUa_BadOutOfMemory;
			else
			{

				// determine size of private key.
				uStatus = OpcUa_Crypto_GenerateAsymmetricKeypair(
					&tCryptoProvider,
					a_uKeyType,
					a_uKeySize,
					&tPublicKey,
					a_pPrivateKey);
					
				if (uStatus==OpcUa_Good)
				{

					// allocate private key buffer.
					a_pPrivateKey->Key.Data = (OpcUa_Byte*)OpcUa_Alloc(a_pPrivateKey->Key.Length);
					if (a_pPrivateKey->Key.Data==NULL)
						uStatus=OpcUa_BadOutOfMemory;
					else
					{
						// generate a new key pair.
						uStatus = OpcUa_Crypto_GenerateAsymmetricKeypair(
							&tCryptoProvider,
							a_uKeyType,
							a_uKeySize,
							&tPublicKey,
							a_pPrivateKey);
								
						if (uStatus==OpcUa_Good)
						{
							// create the subject name fields.
							pSubjectNameFields = (OpcUa_Crypto_NameEntry*)OpcUa_Alloc(fieldNames.size()*sizeof(OpcUa_Crypto_NameEntry));
							if (!pSubjectNameFields)
								uStatus=OpcUa_BadOutOfMemory;
							else
							{
								memset(pSubjectNameFields, 0, fieldNames.size()*sizeof(OpcUa_Crypto_NameEntry));
								
								// reverse order.
								for (int ii = (int)fieldNames.size()-1; ii >= 0; ii--)
								{
									int index = (int)fieldNames.size()-1-ii;
									pSubjectNameFields[index].key = (char*)fieldNames[ii].c_str();
									pSubjectNameFields[index].value = (char*)fieldValues[ii].c_str();
								}

								pExtensions[0].key = (char*)SN_subject_key_identifier;
								pExtensions[0].value = (char*)"hash";

								pExtensions[1].key = (char*)SN_authority_key_identifier;
								pExtensions[1].value = (char*)"keyid, issuer:always";

								if (!a_bIsCA)
								{
									pExtensions[2].key = (char*)SN_basic_constraints;
									pExtensions[2].value = (char*)"critical, CA:FALSE";

									pExtensions[3].key = (char*)SN_key_usage;
									pExtensions[3].value = (char*)"critical, nonRepudiation, digitalSignature, keyEncipherment, dataEncipherment, keyCertSign";

									pExtensions[4].key = (char*)SN_ext_key_usage;
									pExtensions[4].value = (char*)"critical, serverAuth, clientAuth";

									// Add the subject alternate name extension.
									subjectAltName += "URI:";
									subjectAltName += applicationUri;

									//for (DWORD ii = 0; ii < domainNames.size(); ii++)
									//{
									//	std::string domainName = domainNames[ii];

									OpcUa_UInt32 iResult = inet_addr(domainName.c_str());

									if (iResult != INADDR_NONE)
									{
										subjectAltName += ",IP:";
									}
									else
									{
										subjectAltName += ",DNS:";
									}

									subjectAltName += domainName;
									//}
									
									pExtensions[5].key = (char*)SN_subject_alt_name;
									pExtensions[5].value = (LPSTR)subjectAltName.c_str();
								}
								else
								{
									pExtensions[2].key = (char*)SN_basic_constraints;
									pExtensions[2].value = (char*)"critical, CA:TRUE";

									pExtensions[3].key = (char*)SN_key_usage;
									pExtensions[3].value = (char*)"critical, digitalSignature, keyCertSign, cRLSign";
								}

								OpcUa_Byte* pPosition = NULL;
								OpcUa_Key* pPrivateKey = a_pPrivateKey;

								// decode the issuer certificate.
								if (a_pIssuerCertificate != NULL && a_pIssuerCertificate->Length > 0)
								{
									pPosition = a_pIssuerCertificate->Data;
									pX509IssuerCertificate = (OpcUa_Certificate*)d2i_X509(NULL, (const unsigned char**)&pPosition, a_pIssuerCertificate->Length);

									if (pX509IssuerCertificate == NULL)
										uStatus=OpcUa_BadDecodingError;
									else
									{
										if (a_pIssuerPrivateKey != NULL && a_pIssuerPrivateKey->Key.Length > 0)
										{			
											// hack to get around the fact that the load private key and the create key functions use 
											// different constants to identify the RS public keys.
											a_pIssuerPrivateKey->Type = OpcUa_Crypto_Rsa_Alg_Id;

											// use the issuer key for signing.
											pPrivateKey = a_pIssuerPrivateKey;
										}
									}
								}

								// create the certificate.
								uStatus = OpcUa_Crypto_CreateCertificate(
									&tCryptoProvider,
									0,
									tValidFrom,
									tValidTo,
									pSubjectNameFields,
									fieldNames.size(),
									tPublicKey,
									pExtensions,
									(a_bIsCA)?4:6,
									OPCUA_P_SHA_160,
									pX509IssuerCertificate,
									*pPrivateKey,
									&pX509Certificate);
											
								if (uStatus==OpcUa_Good)
								{

									// need to convert to DER encoded certificate.
									a_pCertificate->Length = i2d_X509((X509*)pX509Certificate, NULL);

									if (a_pCertificate->Length <= 0)
										uStatus=OpcUa_BadEncodingError;
									else
									{
										a_pCertificate->Data = (OpcUa_Byte*)OpcUa_Alloc(a_pCertificate->Length);
										if(a_pCertificate->Data==NULL)
											uStatus=OpcUa_BadOutOfMemory;
										else
										{
											// OpenSSL likes to modify input parameters.
											pPosition = a_pCertificate->Data;
											int iResult = i2d_X509((X509*)pX509Certificate, &pPosition);

											if (iResult <= 0)
												uStatus=OpcUa_BadEncodingError;
											else
											{
												// save the certificate.
												uStatus = OpcUa_Certificate_SavePrivateKeyInStore(
													a_sStorePath,
													a_eFileFormat,
													a_sPassword,
													a_pCertificate,
													a_pPrivateKey,
													a_pPrivateKeyFilePath);

												if (uStatus==OpcUa_Good)
												{
													// save the public key certificate.
													uStatus = OpcUa_Certificate_SavePublicKeyInStore(
														a_sStorePath,
														a_pCertificate,
														a_pCertificateFilePath);
								
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	// clean up.
	if (uStatus==OpcUa_Good)
	{

		X509_free((X509*)pX509IssuerCertificate);
		X509_free((X509*)pX509Certificate);
		OpcUa_Free(pSubjectNameFields);
		OpcUa_Key_Clear(&tPublicKey);
		OpcUa_Certificate_DeleteCryptoProviders(&tPkiProvider, &tCryptoProvider);
	}
	else
	{
		if (pX509IssuerCertificate != NULL)
			X509_free((X509*)pX509IssuerCertificate);

		if (pX509Certificate != NULL)
			X509_free((X509*)pX509Certificate);

		OpcUa_Free(pSubjectNameFields);
		OpcUa_Key_Clear(a_pPrivateKey);
		OpcUa_ByteString_Clear(a_pCertificate);
		OpcUa_Key_Clear(&tPublicKey);
		OpcUa_Certificate_DeleteCryptoProviders(&tPkiProvider, &tCryptoProvider);
	}
	return uStatus;
}

/*============================================================================
 * OpcUa_CreateApplication_Uri
 *===========================================================================*/
OpcUa_StatusCode OpcUa_CreateApplication_Uri(OpcUa_StringA a_sApplicationName, std::string* ApplicationUri)
{
	std::string domainName;
	OpcUa_StatusCode uStatus=OpcUa_Good;

	OpcUa_StringA pDomainName = OpcUa_Null;
	// look up the domain name for the current machine.
	uStatus = OpcUa_LookupDomainName((OpcUa_StringA)"127.0.0.1", &pDomainName);        
	if (OpcUa_IsBad(uStatus))
	{
		// use the computer name if no domain name.
		CHAR sBuffer[MAX_PATH+1];
		gethostname(sBuffer,MAX_PATH);
		domainName = sBuffer;
	}
	else
	{
		// copy the domain name.
		domainName = pDomainName;
		OpcUa_Free(pDomainName);
	}
	if (a_sApplicationName)
	{
		*ApplicationUri = "urn:";
		*ApplicationUri += domainName;
		*ApplicationUri += ":OpenOpcUa";
		*ApplicationUri += ":";
		*ApplicationUri += a_sApplicationName;
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}
OpcUa_StatusCode OpcUa_ASN1ToDateTime(ASN1_TIME* asn1Time, OpcUa_DateTime* pDateTime)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	SYSTEMTIME sysTime;
	FILETIME ftTime;
	ZeroMemory(&sysTime,sizeof(SYSTEMTIME));
	const char* str = (const char*) asn1Time->data;
	//size_t i = 0;
	if (pDateTime)
	{
		if (asn1Time->type == V_ASN1_UTCTIME) /* two digit year */
		{
			int iRes=sscanf(str,"%02hu%02hu%02hu%02hu%02hu%02hu",
				&(sysTime.wYear),
				&(sysTime.wMonth),&(sysTime.wDay),
				&(sysTime.wHour),&(sysTime.wMinute),
				&(sysTime.wSecond));
			sysTime.wYear +=2000;
			if (iRes != 6)
				uStatus = OpcUa_BadInternalError;
		}
		else
		{
			if (asn1Time->type == V_ASN1_GENERALIZEDTIME) /* four digit year */
			{
				int iRes=sscanf(str,"%04hu%02hu%02hu%02hu%02hu%02hu",
					&(sysTime.wYear),
					&(sysTime.wMonth),&(sysTime.wDay),
					&(sysTime.wHour),&(sysTime.wMinute),
					&(sysTime.wSecond));	
				if (iRes != 6)
					uStatus = OpcUa_BadInternalError;
			}
		}
		if (SystemTimeToFileTime(&sysTime,&ftTime))
		{
			pDateTime->dwHighDateTime=ftTime.dwHighDateTime;
			pDateTime->dwLowDateTime=ftTime.dwLowDateTime;
		}
		else
		{
#ifdef WIN32
			OpcUa_Trace(OPCUA_TRACE_EXTRA_LEVEL_ERROR,"SystemTimeToFileTime critical error = 0x%05x\n",GetLastError());
#endif
			uStatus=OpcUa_BadInvalidArgument;
		}
	}
	else
		uStatus=OpcUa_BadInvalidArgument;
	return uStatus;
}
/*============================================================================
 * OpcUa_Certificate_GetDateBound
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Certificate_GetDateBound(OpcUa_ByteString* a_pCertificate,OpcUa_DateTime* ValidFrom,OpcUa_DateTime* ValidTo)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_Byte pThumbprint[SHA_DIGEST_LENGTH];
	OpcUa_CharA sBuffer[MAX_PATH*10];
	X509* pCertificate = OpcUa_Null;
	const unsigned char* pPosition = NULL;
	//GENERAL_NAMES* subjectAltName = NULL;
	if (a_pCertificate)
	{
		// initialize local storage.
		OpcUa_MemSet(pThumbprint, 0, SHA_DIGEST_LENGTH);
		OpcUa_MemSet(sBuffer, 0, sizeof(sBuffer));

		// decode the certifcate.
		pPosition = a_pCertificate->Data;
		pCertificate = d2i_X509(&pCertificate, &pPosition, a_pCertificate->Length);
		//
		if (pCertificate)
		{
			//#ifdef _GNUC_ //    ssl .098
			X509_get_notBefore(pCertificate);
			//#else
			//		OpcUa_UInt32 uiValidFrom=0;
			//		X509_get_notBefore(pCertificate,uiValidFrom);
			//#endif
			ASN1_TIME* notAfter = pCertificate->cert_info->validity->notAfter;
			ASN1_TIME* notBefore = pCertificate->cert_info->validity->notBefore;
			uStatus = OpcUa_ASN1ToDateTime(notBefore, ValidFrom);
			uStatus = OpcUa_ASN1ToDateTime(notAfter, ValidTo);
			X509_free(pCertificate);
		}
		else
			uStatus = OpcUa_BadCertificateInvalid;
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}
/*============================================================================
 * OpcUa_Certificate_GetInfo
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Certificate_GetInfo(
	OpcUa_ByteString* a_pCertificate,
	OpcUa_StringA**   a_psNameEntries,
	OpcUa_UInt32*     a_puNoOfNameEntries,
	OpcUa_StringA*    a_psCommonName,
	OpcUa_CharA**	  a_psThumbprint,
	OpcUa_CharA**     a_psApplicationUri,
	OpcUa_StringA**   a_psDomains,
	OpcUa_UInt32*     a_puNoOfDomains)
{
	
	OpcUa_Byte pThumbprint[SHA_DIGEST_LENGTH];
	OpcUa_CharA sBuffer[MAX_PATH*10];
	X509* pCertificate = OpcUa_Null;
	const unsigned char* pPosition = OpcUa_Null;
	std::vector<OpcUa_String*> entries;
	std::string fullName;
	STACK_OF(CONF_VALUE)* subjectAltNameEntries = OpcUa_Null;
	GENERAL_NAMES* subjectAltName = OpcUa_Null;
	OpcUa_StatusCode uStatus = OpcUa_Good;

	
	if (a_pCertificate)
	{
		if (a_pCertificate->Data)
		{
			// initialize output parameters.
			if (a_psNameEntries)
			{
				*a_psNameEntries = OpcUa_Null;
				if (a_puNoOfNameEntries)
					*a_puNoOfNameEntries = 0;
				else
					uStatus = OpcUa_BadInvalidArgument;
			}
			

			if (uStatus==OpcUa_Good)
			{
				if (a_psDomains)
				{
					if (a_puNoOfDomains)
					{
						*a_psDomains = OpcUa_Null;
						*a_puNoOfDomains = 0;
					}
					else
						uStatus = OpcUa_BadInvalidArgument;
				}
				if (uStatus == OpcUa_Good)
				{
					if (a_psCommonName != OpcUa_Null)
						*a_psCommonName = OpcUa_Null;

					if (a_psThumbprint != OpcUa_Null)
						*a_psThumbprint = OpcUa_Null;

					if (a_psApplicationUri != OpcUa_Null)
						*a_psApplicationUri = OpcUa_Null;

					// initialize local storage.
					OpcUa_MemSet(pThumbprint, 0, SHA_DIGEST_LENGTH);
					OpcUa_MemSet(sBuffer, 0, sizeof(sBuffer));

					// decode the certifcate.
					pPosition = a_pCertificate->Data;
					pCertificate = d2i_X509(OpcUa_Null, &pPosition, a_pCertificate->Length);
					if (pCertificate)
					{

						if (a_psThumbprint != NULL)
						{
							// compute the hash.
							SHA1(a_pCertificate->Data, a_pCertificate->Length, pThumbprint);

							// allocate string to return.
							int iLength = (2 * SHA_DIGEST_LENGTH + 1)*sizeof(OpcUa_CharA);
							*a_psThumbprint = (OpcUa_CharA*)OpcUa_Alloc(iLength);
							OpcUa_MemSet(*a_psThumbprint, 0, iLength);

							// convert to a string.
							for (int ii = 0; ii < SHA_DIGEST_LENGTH; ii++)
							{
		#ifndef _GNUC_
								OpcUa_SPrintfA(*a_psThumbprint + ii * 2, "%02X", pThumbprint[ii]);
		#else
								OpcUa_SnPrintfA(*a_psThumbprint+ii*2, iLength-ii*2, "%02X", pThumbprint[ii]);
		#endif
							}
						}

						if (a_psNameEntries != NULL || a_psCommonName != NULL)
						{
							// get the subject name.
							X509_name_st* pName = X509_get_subject_name(pCertificate);

							if (pName)
							{

								X509_NAME_oneline(pName, sBuffer, sizeof(sBuffer));

								// parse the fields.
								fullName = sBuffer;

								size_t iStart = 0;
								size_t iEnd = fullName.find_first_of('/');

								while (iStart != std::string::npos)
								{
									if (iEnd == std::string::npos)
									{
										if (iStart < (size_t)fullName.size())
										{
											OpcUa_String* pString = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
											OpcUa_String_Initialize(pString);
											if (OpcUa_String_AttachCopy(pString, fullName.substr(iStart).c_str()) == OpcUa_Good)
												entries.push_back(pString);
											else
											{
												OpcUa_String_Clear(pString);
												OpcUa_Free(pString);
												pString = OpcUa_Null;
											}
										}

										break;
									}

									if (iEnd > iStart)
									{
										OpcUa_String* pString = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
										OpcUa_String_Initialize(pString);
										if (OpcUa_String_AttachCopy(pString, fullName.substr(iStart, iEnd - iStart).c_str())==OpcUa_Good)
											entries.push_back(pString);
										else
										{
											OpcUa_String_Clear(pString);
											OpcUa_Free(pString);
											pString = OpcUa_Null;
										}
									}

									iStart = iEnd + 1;
									iEnd = fullName.find_first_of('/', iStart);
								} 

								// extract the name entries.
								if (a_psNameEntries)
									uStatus = OpcUa_Certificate_CopyStrings(entries, a_psNameEntries, a_puNoOfNameEntries);

								// extract the common name.
								if ( (a_psCommonName != NULL) && (uStatus==OpcUa_Good) )
								{
									for (unsigned int ii = 0; ii < entries.size(); ii++)
									{
										OpcUa_String* entry=entries[ii];
										OpcUa_StringA pStringA = OpcUa_String_GetRawString(entry);
										if (OpcUa_StrStrA(pStringA, "CN="))
										{
											int iLength = OpcUa_String_StrLen(entry) + 1;
											*a_psCommonName = (OpcUa_StringA)OpcUa_Alloc(iLength);
											if (*a_psCommonName)
											{
												strcpy(*a_psCommonName, pStringA + 3);
												OpcUa_String_Clear(entry);
												OpcUa_Free(entry);
												entry = OpcUa_Null;
											}
											else
											{
												uStatus = OpcUa_BadOutOfMemory;
												OpcUa_String_Clear(entry);
												OpcUa_Free(entry);
												entry = OpcUa_Null;
											}
										}
										if (entry)
										{
											OpcUa_String_Clear(entry);
											OpcUa_Free(entry);
											entry = OpcUa_Null;
										}
									}
									entries.clear();
								}
							}
							else
								uStatus=OpcUa_BadDecodingError;
						}
						if (uStatus == OpcUa_Good)
						{
							if (a_psApplicationUri != NULL || a_psDomains != NULL)
							{
								// find the subject alt name extension.
								STACK_OF(X509_EXTENSION)* pExtensions = pCertificate->cert_info->extensions;

								for (int ii = 0; ii < sk_X509_EXTENSION_num(pExtensions); ii++)
								{
									X509_EXTENSION* pExtension = sk_X509_EXTENSION_value(pExtensions, ii);

									// get the internal id for the extension.
									int nid = OBJ_obj2nid(pExtension->object);

									if (nid == 0)
									{
										// check for obsolete name.
										ASN1_OBJECT* oid = (ASN1_OBJECT*)pExtension->object;

										if (memcmp(oid->data, ::OID_SUBJECT_ALT_NAME, 3) == 0)
										{
											oid->nid = nid = NID_subject_alt_name;
										}
									}

									if (nid == NID_subject_alt_name)
									{
										subjectAltName = (GENERAL_NAMES*)X509V3_EXT_d2i(pExtension);
									}
								}

								// extract the fields from the subject alt name extension.
								if (subjectAltName != NULL)
								{
									entries.clear();
									subjectAltNameEntries = i2v_GENERAL_NAMES(NULL, subjectAltName, NULL);

									for (int ii = 0; ii < sk_CONF_VALUE_num(subjectAltNameEntries); ii++)
									{
										CONF_VALUE* conf = sk_CONF_VALUE_value(subjectAltNameEntries, ii);

										if (conf)
										{
											// check for URI.
											if (a_psApplicationUri != NULL)
											{
												// copy the application uri.
												if (*a_psApplicationUri == NULL && strcmp(conf->name, "URI") == 0)
												{
													int iLength = strlen(conf->value) + 1;
													*a_psApplicationUri = (OpcUa_CharA*)OpcUa_Alloc(iLength);
													if (*a_psApplicationUri)
														strcpy(*a_psApplicationUri, conf->value);
													else
													{
														uStatus = OpcUa_BadOutOfMemory;
														break;
													}
												}
											}

											// check for domain.
											if (a_psDomains != NULL)
											{
												if (strcmp(conf->name, "DNS") == 0)
												{
													OpcUa_String* pString = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
													OpcUa_String_Initialize(pString);
													OpcUa_String_AttachCopy(pString, conf->value);
													entries.push_back(pString);
												}

												if (strcmp(conf->name, "IP Address") == 0)
												{
													OpcUa_String* pString = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
													OpcUa_String_Initialize(pString);
													OpcUa_String_AttachCopy(pString, conf->value);
													entries.push_back(pString);
												}
											}
										}
									}

									sk_CONF_VALUE_pop_free(subjectAltNameEntries, X509V3_conf_free);
									subjectAltNameEntries = NULL;

									sk_GENERAL_NAME_pop_free(subjectAltName, GENERAL_NAME_free);
									subjectAltName = NULL;

									// copy domains.
									if (a_psDomains)
										uStatus = OpcUa_Certificate_CopyStrings(entries, a_psDomains, a_puNoOfDomains);
								}
							}
							if (pCertificate)
								X509_free(pCertificate);
						}
						for (unsigned int ii = 0; ii < entries.size(); ii++)
						{
							OpcUa_String_Clear(entries[ii]);
							OpcUa_Free(entries[ii]);
						}
						entries.clear();
					}
					else
						uStatus=OpcUa_BadDecodingError;
				}

			}
		}
		else
			uStatus = OpcUa_BadInvalidArgument;
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	if (uStatus != OpcUa_Good)
	{
		if (pCertificate != NULL)
			X509_free(pCertificate);

		if (subjectAltNameEntries != NULL)
			sk_CONF_VALUE_pop_free(subjectAltNameEntries, X509V3_conf_free);

		if (subjectAltName != NULL)
			sk_GENERAL_NAME_pop_free(subjectAltName, GENERAL_NAME_free);

		if (a_psNameEntries != NULL && *a_psNameEntries != NULL)
		{
			for (unsigned int ii = 0; ii < *a_puNoOfNameEntries; ii++)
				OpcUa_Free((*a_psNameEntries)[ii]);
			OpcUa_Free(*a_psNameEntries);
			*a_psNameEntries = NULL;
		}

		if (a_psCommonName != NULL && *a_psCommonName != NULL)
		{
			OpcUa_Free(*a_psCommonName);
			*a_psCommonName = NULL;
		}

		if (a_psThumbprint != NULL && *a_psThumbprint != NULL)
		{
			OpcUa_Free(*a_psThumbprint);
			*a_psThumbprint = NULL;
		}

		if (a_psApplicationUri != NULL && *a_psApplicationUri != NULL)
		{
			OpcUa_Free(*a_psApplicationUri);
			*a_psApplicationUri = NULL;
		}

		if (a_psDomains != NULL && *a_psDomains != NULL)
		{
			for (unsigned int ii = 0; ii < *a_puNoOfDomains; ii++)
				OpcUa_Free((*a_psDomains)[ii]);
			OpcUa_Free(*a_psDomains);
			*a_psDomains = NULL;
		}

	}
	return uStatus;
}

/*============================================================================
 * OpcUa_Certificate_GetThumbprint
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Certificate_GetThumbprint(
	OpcUa_ByteString* a_pCertificate,
	OpcUa_StringA*    a_pThumbprint)
{
	return OpcUa_Certificate_GetInfo(a_pCertificate, NULL, NULL, NULL, a_pThumbprint, NULL, NULL, NULL);
}

/*============================================================================
 * OpcUa_Certificate_GetCommonName
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Certificate_GetCommonName(
	OpcUa_ByteString* a_pCertificate,
	OpcUa_StringA*    m_pCommonName)
{
	return OpcUa_Certificate_GetInfo(a_pCertificate, NULL, NULL, m_pCommonName, NULL, NULL, NULL, NULL);
}

/*============================================================================
 * OpcUa_Certificate_SavePublicKeyInStore
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Certificate_SavePublicKeyInStore(
	OpcUa_StringA     a_sStorePath,
	OpcUa_ByteString* a_pCertificate,   
	OpcUa_StringA*    a_pFilePath)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	BIO* pPublicKeyFile = OpcUa_Null;
	std::string filePath;
	OpcUa_Int iResult ;

	OpcUa_ReturnErrorIfArgumentNull(a_sStorePath);
	OpcUa_ReturnErrorIfArgumentNull(a_pCertificate);

	if (a_pFilePath != OpcUa_Null) *a_pFilePath = OpcUa_Null;

	// get the file name for the certificate.
	filePath = OpcUa_Certificate_GetFilePathForCertificate(
		a_sStorePath, 
		a_pCertificate, 
		OpcUa_Crypto_Encoding_DER,
		OpcUa_True);

	if (!filePath.empty())
	{

		pPublicKeyFile = BIO_new_file((const char*)filePath.c_str(), "wb");
		if (pPublicKeyFile)
		{
			iResult = BIO_write(pPublicKeyFile, a_pCertificate->Data, a_pCertificate->Length);

			if (iResult)
			{
				// return the file path.
				if (a_pFilePath != OpcUa_Null)
				{
					*a_pFilePath = (OpcUa_StringA)OpcUa_Alloc(filePath.size() + 1);
					if (*a_pFilePath)
						strcpy(*a_pFilePath, filePath.c_str());
					else
						uStatus = OpcUa_BadOutOfMemory;
				}
			}
			else
				uStatus = OpcUa_BadEncodingError;
			BIO_free(pPublicKeyFile);
		}
		else
			uStatus = OpcUa_BadInvalidArgument;
	}
	else
		uStatus=OpcUa_BadNotSupported;
	return uStatus;
}

/*============================================================================
 * OpcUa_Certificate_SavePrivateKeyInStore
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Certificate_SavePrivateKeyInStore(
	OpcUa_StringA      a_sStorePath,
	OpcUa_P_FileFormat a_eFileFormat,
	OpcUa_StringA      a_sPassword,   
	OpcUa_ByteString*  a_pCertificate,    
	OpcUa_Key*         a_pPrivateKey,   
	OpcUa_StringA*     a_pFilePath)
{
	BIO*      pPrivateKeyFile     = OpcUa_Null;
	RSA*      pRsaPrivateKey      = OpcUa_Null;
	EVP_PKEY* pEvpKey             = OpcUa_Null;
	X509*     pX509Certificate    = OpcUa_Null;
	int iResult ;
	std::string filePath;
	OpcUa_StringA sCommonName = OpcUa_Null;
	BYTE* pPosition ;
	const unsigned char* pPos;
OpcUa_InitializeStatus(OpcUa_Module_Crypto, "OpcUa_Certificate_SavePrivateKeyInStore");

	OpcUa_ReturnErrorIfArgumentNull(a_sStorePath);
	OpcUa_ReturnErrorIfArgumentNull(a_pCertificate);
	OpcUa_ReturnErrorIfArgumentNull(a_pPrivateKey);

	if (a_pFilePath != NULL) *a_pFilePath = NULL;

	// check for supported format.
	if (a_eFileFormat == OpcUa_Crypto_Encoding_Invalid)
	{
		return OpcUa_BadInvalidArgument;
	}

	// check for supported key type.
	if (a_pPrivateKey->Type != OpcUa_Crypto_Rsa_Alg_Id && a_pPrivateKey->Type != OpcUa_Crypto_KeyType_Rsa_Private)
	{
		return OpcUa_BadInvalidArgument;
	}

	// get the file name for the certificate.
	filePath = OpcUa_Certificate_GetFilePathForCertificate(
		a_sStorePath, 
		a_pCertificate, 
		a_eFileFormat,
		OpcUa_True);

	if (filePath.empty())
	{
		OpcUa_GotoErrorWithStatus(OpcUa_BadNotSupported);
	}

	pPrivateKeyFile = BIO_new_file((const char*)filePath.c_str(), "wb");
	OpcUa_GotoErrorIfNull(pPrivateKeyFile, OpcUa_BadEncodingError);

	// convert DER encoded data to RSA data.
	pPos = a_pPrivateKey->Key.Data;
	pRsaPrivateKey = d2i_RSAPrivateKey(NULL, &pPos, a_pPrivateKey->Key.Length);
	OpcUa_GotoErrorIfAllocFailed(pRsaPrivateKey);

	pEvpKey = EVP_PKEY_new();

	// convert to intermediary openssl struct
	if (!EVP_PKEY_set1_RSA(pEvpKey, pRsaPrivateKey))
	{
		OpcUa_GotoErrorWithStatus(OpcUa_BadEncodingError);
	}

	// convert public key to X509 structure.
	pPosition = a_pCertificate->Data;    
	pX509Certificate = d2i_X509((X509**)OpcUa_Null, (const unsigned char**)&pPosition, a_pCertificate->Length);
	OpcUa_GotoErrorIfNull(pX509Certificate, OpcUa_Bad);

	switch(a_eFileFormat)
	{
		case OpcUa_Crypto_Encoding_PEM:
		{
			// select encryption algorithm.
			const EVP_CIPHER* pCipher = NULL;
			char* pPassword = NULL;

			if (a_sPassword != NULL)
			{
				pCipher = EVP_des_ede3_cbc();
				pPassword = a_sPassword;
			}

			// write to file.
			iResult = PEM_write_bio_PrivateKey(
				pPrivateKeyFile,   
				pEvpKey,
				pCipher, 
				NULL, 
				0, 
				0, 
				pPassword);

			if (iResult == 0)
			{
				OpcUa_GotoErrorWithStatus(OpcUa_BadEncodingError);
			}

			break;
		}

		case OpcUa_Crypto_Encoding_PKCS12:
		{
			// use the common name as the friendly name.
			uStatus = OpcUa_Certificate_GetCommonName(a_pCertificate, &sCommonName);
			OpcUa_GotoErrorIfBad(uStatus);

			// create certificate.
			PKCS12* pPkcs12 = PKCS12_create(
				a_sPassword,
				sCommonName,
				pEvpKey,
				pX509Certificate,
				0,
				0,
				0,
				0,
				0,
				0);
	
			OpcUa_GotoErrorIfNull(pPkcs12, OpcUa_Bad);

			// write to file.
			iResult = i2d_PKCS12_bio(pPrivateKeyFile, pPkcs12);

			// free certificate.
			PKCS12_free(pPkcs12);

			if (iResult == 0)
			{
				OpcUa_GotoErrorWithStatus(OpcUa_BadEncodingError);
			}

			break;
		}

		case OpcUa_Crypto_Encoding_DER:
		default:
		{
			uStatus = OpcUa_BadNotSupported;
			OpcUa_GotoError;
		}
	}

	// return the file path.
	if (a_pFilePath != NULL) 
	{
		*a_pFilePath = (OpcUa_StringA)OpcUa_Alloc(filePath.size()+1);
		OpcUa_GotoErrorIfAllocFailed(*a_pFilePath);
		//strcpy_s(*a_pFilePath, filePath.size()+1, filePath.c_str());
		strcpy(*a_pFilePath, filePath.c_str());
	}

	// free memory.
	EVP_PKEY_free(pEvpKey);
	RSA_free(pRsaPrivateKey);
	BIO_free(pPrivateKeyFile);
	X509_free(pX509Certificate);
	OpcUa_Free(sCommonName);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	if (pPrivateKeyFile != NULL)
	{
		BIO_free(pPrivateKeyFile);
	}

	if (pEvpKey != NULL)
	{
		EVP_PKEY_free(pEvpKey);
	}

	if (pRsaPrivateKey != NULL)
	{
		RSA_free(pRsaPrivateKey);
	}

	if (pX509Certificate != NULL)
	{
		X509_free(pX509Certificate);
	}

	if (sCommonName != NULL)
	{
		OpcUa_Free(sCommonName);
	}

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_Certificate_LoadPrivateKeyFromFile
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Certificate_LoadPrivateKeyFromFile(
	OpcUa_StringA      a_sFilePath,		// in
	OpcUa_P_FileFormat a_eFileFormat,	// in
	OpcUa_StringA      a_sPassword,		// in
	OpcUa_ByteString*  a_pCertificate,	// out contient la cl? priv? X509 extrait du certificat priv?  
	OpcUa_Key*         a_pPrivateKey)	// out contient la RSAPrivateKey
{
	OpcUa_StatusCode	uStatus=OpcUa_Good;
	BIO*				pPrivateKeyFile = OpcUa_Null;
	RSA*				pRsaPrivateKey = OpcUa_Null;
	EVP_PKEY*			pEvpKey = OpcUa_Null;
	PKCS12*				pPkcs12 = OpcUa_Null;
	X509*				pX509 = OpcUa_Null;
	OpcUa_Byte*			pPosition;
	int					iResult = 0;


	if (a_sFilePath)
	{
		if (a_pCertificate)
		{
			if (a_pPrivateKey)
			{
				OpcUa_ByteString_Initialize(a_pCertificate);
				OpcUa_Key_Initialize(a_pPrivateKey);

				// check for supported format.
				if (a_eFileFormat != OpcUa_Crypto_Encoding_Invalid)
				{
					// creates a new file BIO with mode rb = read binary
					pPrivateKeyFile = BIO_new_file(a_sFilePath, "rb");
					if(pPrivateKeyFile)
					{
						switch(a_eFileFormat)
						{
							case OpcUa_Crypto_Encoding_PEM:
							{
								// read from file.
								pEvpKey = PEM_read_bio_PrivateKey( 
									pPrivateKeyFile,   
									NULL,           
									0,        
									a_sPassword); 
								if (!pEvpKey)
									uStatus=OpcUa_BadNotSupported;							
							}
							break;
							case OpcUa_Crypto_Encoding_PKCS12:
							{
								// read from file.
								pPkcs12 = d2i_PKCS12_bio(pPrivateKeyFile, NULL);				            
								if (!pPkcs12)
									uStatus=OpcUa_BadEncodingError;
								else
								{
									// parse the certificate.
									iResult = PKCS12_parse(pPkcs12, a_sPassword, &pEvpKey, &pX509, NULL);
									if (!iResult)
										uStatus=OpcUa_BadEncodingError;
									// free certificate.
									PKCS12_free(pPkcs12);
								}
								pPkcs12 = OpcUa_Null;							
							}
							break;
							case OpcUa_Crypto_Encoding_DER:
							default:
								uStatus = OpcUa_BadNotSupported;							
							break;
						}
						
						// get the private key embedded in the certificate
						if ( (pX509 != OpcUa_Null) && (uStatus==OpcUa_Good) )
						{
							// need to convert to DER encoded certificate.
							a_pCertificate->Length = i2d_X509((X509*)pX509, NULL);

							if (a_pCertificate->Length > 0)
							{								
								a_pCertificate->Data = (OpcUa_Byte*)OpcUa_Alloc(a_pCertificate->Length);
								if (a_pCertificate->Data)
								{
									// OpenSSL likes to modify input parameters.
									pPosition = a_pCertificate->Data;
									iResult = i2d_X509((X509*)pX509, &pPosition);

									if (iResult <= 0)
										uStatus=OpcUa_BadEncodingError;				
									X509_free((X509*)pX509);
									pX509 = OpcUa_Null;
								}
								else
									uStatus=OpcUa_BadOutOfMemory;
							}
							else
								uStatus=OpcUa_BadEncodingError;
						}
						if (uStatus==OpcUa_Good)
						{
							// get the private key.
							pRsaPrivateKey = EVP_PKEY_get1_RSA(pEvpKey);
							if (pRsaPrivateKey)
							{
								// convert DER encoded data to RSA data.
								a_pPrivateKey->Type = OpcUa_Crypto_KeyType_Rsa_Private;
								a_pPrivateKey->Key.Length = i2d_RSAPrivateKey(pRsaPrivateKey, NULL);

								if (a_pPrivateKey->Key.Length > 0)
								{
									// allocate key.
									a_pPrivateKey->Key.Data = (OpcUa_Byte*)OpcUa_Alloc(a_pPrivateKey->Key.Length);
									if (a_pPrivateKey->Key.Data)
									{
										memset(a_pPrivateKey->Key.Data, 0, a_pPrivateKey->Key.Length);
										
										pPosition = a_pPrivateKey->Key.Data;
										//  decode and encode a PKCS#1 RSAPrivateKey structure.
										iResult = i2d_RSAPrivateKey(pRsaPrivateKey, &pPosition);

										if (iResult <= 0)
											uStatus=OpcUa_BadDecodingError;
										// free memory.
										EVP_PKEY_free(pEvpKey);
										RSA_free(pRsaPrivateKey);
										//BIO_free(pPrivateKeyFile);
									}
									else
										uStatus=OpcUa_BadOutOfMemory;
								}
								else
									uStatus=OpcUa_BadDecodingError;
							}
							else
								uStatus=OpcUa_BadCertificateInvalid;
						}
						if (pPrivateKeyFile)
							BIO_free(pPrivateKeyFile);
					}
					else
						uStatus=OpcUa_BadFileNotFound;
				}
				else
					uStatus=OpcUa_BadInvalidArgument;
			}
			else
				uStatus=OpcUa_BadInvalidArgument;
		}
		else
			uStatus=OpcUa_BadInvalidArgument;
	}
	else
		uStatus=OpcUa_BadInvalidArgument;
	if( uStatus!=OpcUa_Good)
	{
		OpcUa_ByteString_Clear(a_pCertificate);
		OpcUa_Key_Clear(a_pPrivateKey);

		if (pPrivateKeyFile != NULL)
			BIO_free(pPrivateKeyFile);

		if (pEvpKey != NULL)
			EVP_PKEY_free(pEvpKey);

		if (pX509 != NULL)
			X509_free((X509*)pX509);

		if (pRsaPrivateKey != NULL)
			RSA_free(pRsaPrivateKey);

		if (pPkcs12 != NULL)
			PKCS12_free(pPkcs12);
	}
	return uStatus;
}

/*============================================================================
 * OpcUa_Certificate_LoadPrivateKeyFromStore
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Certificate_LoadPrivateKeyFromStore(
	OpcUa_StringA      a_sStorePath,
	OpcUa_P_FileFormat a_eFileFormat,
	OpcUa_StringA      a_sPassword,    
	OpcUa_ByteString*  a_pCertificate,   
	OpcUa_Key*         a_pPrivateKey)
{
	std::string filePath; 
	OpcUa_ByteString tCertificate;

OpcUa_InitializeStatus(OpcUa_Module_Crypto, "OpcUa_Certificate_LoadPrivateKeyFromStore");

	OpcUa_ReturnErrorIfArgumentNull(a_sStorePath);
	OpcUa_ReturnErrorIfArgumentNull(a_pCertificate);
	OpcUa_ReturnErrorIfArgumentNull(a_pPrivateKey);

	OpcUa_ByteString_Initialize(&tCertificate);
	OpcUa_Key_Initialize(a_pPrivateKey);

	// check for supported format.
	if (a_eFileFormat == OpcUa_Crypto_Encoding_Invalid)
	{
		return OpcUa_BadInvalidArgument;
	}

	// get the file name for the certificate.
	filePath = OpcUa_Certificate_GetFilePathForCertificate(
		a_sStorePath, 
		a_pCertificate, 
		a_eFileFormat,
		OpcUa_False);

	if (filePath.empty())
	{
		OpcUa_GotoErrorWithStatus(OpcUa_BadNotSupported);
	}

	// load from file.
	uStatus = OpcUa_Certificate_LoadPrivateKeyFromFile(
		(OpcUa_StringA)filePath.c_str(),
		a_eFileFormat,
		a_sPassword,
		&tCertificate,
		a_pPrivateKey);

	OpcUa_GotoErrorIfBad(uStatus);
	OpcUa_ByteString_Clear(&tCertificate);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	OpcUa_ByteString_Clear(&tCertificate);
	OpcUa_Key_Clear(a_pPrivateKey);

OpcUa_FinishErrorHandling;
}

///*============================================================================
// * OpcUa_Certificate_FindContext
// *===========================================================================*/
//struct OpcUa_Certificate_FindContext
//{
//    HCERTSTORE Store;
//    HANDLE File;
//#ifndef _GNUC_
//    PCCERT_CONTEXT Context;
//#else
//    SSL_CTX *Context;
//#endif
//};

/*============================================================================
 * OpcUa_Certificate_CheckForMatch
 *===========================================================================*/
bool OpcUa_Certificate_CheckForMatch(
	OpcUa_ByteString* a_pCertificate,
	OpcUa_StringA     a_sCommonName,
	OpcUa_StringA     a_sThumbprint)
{
	bool match = true;
	OpcUa_StringA sMatchString = NULL;

	// check for a match on the thumbprint.
	if (a_sThumbprint != NULL && strlen(a_sThumbprint) > 0)
	{
		OpcUa_StatusCode uStatus = OpcUa_Certificate_GetThumbprint(a_pCertificate, &sMatchString);
		
		if (OpcUa_IsBad(uStatus))
		{
			return false;
		}

		if (OpcUa_StriCmpA(sMatchString, a_sThumbprint) != 0)
		{
			match = false;
		}

		OpcUa_Free(sMatchString);
		sMatchString = NULL;
	}

	// check for a match on the common name.
	if (match && a_sCommonName != NULL && strlen(a_sCommonName) > 0)
	{
		OpcUa_StatusCode uStatus = OpcUa_Certificate_GetCommonName(a_pCertificate, &sMatchString);
		
		if (OpcUa_IsBad(uStatus))
		{
			return false;
		}

		if (OpcUa_StriCmpA(sMatchString, a_sCommonName) != 0)
		{
			match = false;
		}

		OpcUa_Free(sMatchString);
		sMatchString = NULL;
	}

	return match;
}
#ifndef _GNUC_
/*============================================================================
 * OpcUa_Certificate_FindCertificateInWindowsStore
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Certificate_FindCertificateInWindowsStore(
	OpcUa_Handle*     a_pContext,    
	OpcUa_Boolean     a_bUseMachineStore,
	OpcUa_StringA     a_sStoreName,
	OpcUa_StringA     a_sCommonName,
	OpcUa_StringA     a_sThumbprint,
	OpcUa_ByteString* a_pCertificate)
{
	LPWSTR wszStoreName = NULL;
	OpcUa_Certificate_FindContext* pContext = NULL;

OpcUa_InitializeStatus(OpcUa_Module_Crypto, "OpcUa_Certificate_FindCertificateInWindowsStore");

	OpcUa_ReturnErrorIfArgumentNull(a_pContext);
	OpcUa_ReturnErrorIfArgumentNull(a_sStoreName);
	OpcUa_ReturnErrorIfArgumentNull(a_pCertificate);

	OpcUa_ByteString_Initialize(a_pCertificate);

	if (*a_pContext != NULL)
	{
		pContext = (OpcUa_Certificate_FindContext*)*a_pContext;
	}

	// create a new context.
	if (pContext == NULL)
	{
		uStatus = OpcUa_String_AtoW(a_sStoreName, (OpcUa_CharW**)&wszStoreName);
		OpcUa_GotoErrorIfBad(uStatus);

		pContext = new OpcUa_Certificate_FindContext();

		// open the certificate store.
		DWORD dwFlags = 0;
		
		if (a_bUseMachineStore)
		{
			dwFlags |= CERT_SYSTEM_STORE_LOCAL_MACHINE;
		}
		else
		{
			dwFlags |= CERT_SYSTEM_STORE_CURRENT_USER;
		}

		// open the store.
		pContext->Store = CertOpenStore(
		   CERT_STORE_PROV_SYSTEM,
		   0, 
		   0,                           
		   dwFlags, 
		   wszStoreName);

		if (pContext->Store == 0)
		{
			OpcUa_GotoErrorWithStatus(OpcUa_BadUnexpectedError);
		}

		OpcUa_Free(wszStoreName);
	}

	// Find the certificates in the system store. 
	// WinCrypt call (Crypt32.dll)
	pContext->Context = CertEnumCertificatesInStore(pContext->Store, pContext->Context);
	while (pContext->Context)
	{
		OpcUa_ByteString tCertificate;
		tCertificate.Data = pContext->Context->pbCertEncoded;
		tCertificate.Length = pContext->Context->cbCertEncoded;

		// check for match.
		bool match = OpcUa_Certificate_CheckForMatch(&tCertificate, a_sCommonName, a_sThumbprint);
 
		// copy certificate if match found.
		if (match)
		{
			a_pCertificate->Data = (OpcUa_Byte*)OpcUa_Alloc(tCertificate.Length);
			OpcUa_GotoErrorIfAllocFailed(a_pCertificate->Data);
			OpcUa_MemCpy(a_pCertificate->Data, tCertificate.Length, tCertificate.Data, tCertificate.Length);
			a_pCertificate->Length = tCertificate.Length;
			break;
		}
		pContext->Context = CertEnumCertificatesInStore(pContext->Store, pContext->Context);
	}

	// check if nothing found.
	if (pContext->Context == NULL)
	{
		CertCloseStore(pContext->Store, 0);
		delete pContext;
		*a_pContext = NULL;
	}

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	OpcUa_ByteString_Clear(a_pCertificate);
	OpcUa_Certificate_FreeFindContext((OpcUa_Handle*)&pContext);

	if (wszStoreName != NULL)
	{
		OpcUa_Free(wszStoreName);
	}

OpcUa_FinishErrorHandling;
}
#endif
/*============================================================================
 * OpcUa_Certificate_FindCertificateInStore
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Certificate_FindCertificateInStore(
	OpcUa_Certificate_FindContext**     a_pContext,    
	OpcUa_StringA     a_sStorePath,
	OpcUa_Boolean     a_sHasPrivateKey,// if true will ask for DER file, if false will search for PFX file
	OpcUa_StringA     a_sPassword,
	OpcUa_StringA     a_sCommonName,
	OpcUa_StringA     a_sThumbprint,
	OpcUa_ByteString* a_pCertificate,
	OpcUa_Key*        a_pPrivateKey/*,  
	OpcUa_String*     a_FullFileName*/)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_Certificate_FindContext* pContext = NULL;
	WIN32_FIND_DATA tFindFileData;
	OpcUa_StringA filePath=(OpcUa_StringA)OpcUa_Alloc(1024);
	OpcUa_StringA asFilename = NULL;
	LPWSTR		  *pszFilter = NULL;
	OpcUa_Boolean match;
	ZeroMemory(filePath,1024);

	OpcUa_ReturnErrorIfArgumentNull(a_pContext);
	OpcUa_ReturnErrorIfArgumentNull(a_sStorePath);
	OpcUa_ReturnErrorIfArgumentNull(a_pCertificate);

	OpcUa_ByteString_Initialize(a_pCertificate);
	OpcUa_Key_Initialize(a_pPrivateKey);
	OpcUa_MemSet(&tFindFileData, 0, sizeof(tFindFileData));

	if (*a_pContext != NULL)
	{
		pContext = *a_pContext; //(OpcUa_Certificate_FindContext*)*a_pContext;
	}

	// create a new context.
	if (pContext == NULL)
	{
		pContext = (OpcUa_Certificate_FindContext*)OpcUa_Alloc(sizeof(OpcUa_Certificate_FindContext));

		// specify the search criteria.
		OpcUa_UInt32 iLen=OpcUa_StrLenA(a_sStorePath);
		//filePath=(OpcUa_StringA*)OpcUa_Alloc(iLen)
		OpcUa_MemCpy(filePath,iLen, a_sStorePath, iLen);

		// Chemin UNIX Vs Chemin Win32
#ifndef _GNUC_
		if (a_sHasPrivateKey)
		{
			OpcUa_StrnCatA(&filePath[iLen],1024,"\\private\\*.pfx",OpcUa_StrLenA("\\private\\*.pfx"));
			//filePath += "\\private\\*.pfx";
		}
		else
		{
			OpcUa_StrnCatA(&filePath[iLen],1024,"\\certs\\*.der",OpcUa_StrLenA("\\certs\\*.der"));
			//filePath += "\\certs\\*.der";
		}
#else
		if (a_sHasPrivateKey)
		{
			OpcUa_StrnCatA(&filePath[iLen],1024,"\\private\\*.pfx",OpcUa_StrLenA("\\private\\*.pfx"));
			// filePath += "/private/*.pfx";
		}
		else
		{
			OpcUa_StrnCatA(&filePath[iLen],1024,"//certs//*.der",OpcUa_StrLenA("//certs//*.der"));
			// filePath += "/certs/*.der";
		}
#endif

		OpcUa_String_AtoW((OpcUa_StringA)filePath,(OpcUa_CharW**)&pszFilter);
#ifdef _GNUC_
		pContext->File = FindFirstFile((char*)filePath, &tFindFileData);
#else
	#ifdef UNICODE
		pContext->File = FindFirstFile((wchar_t*)pszFilter, &tFindFileData);
	#else
		pContext->File = FindFirstFile((char*)filePath, &tFindFileData);
	#endif
#endif
		if (pszFilter)
		{
			OpcUa_Free(pszFilter);
			pszFilter=OpcUa_Null;
		}
		if (INVALID_HANDLE_VALUE == pContext->File)
		{
			OpcUa_Free(pContext);
			*a_pContext = NULL;
			return OpcUa_BadFileNotFound;
		}
	}

	// process existing context.
	else
	{
#ifdef WIN32
		if (!FindNextFile(pContext->File, &tFindFileData))
		{
			FindClose(pContext->File);
			OpcUa_Free(pContext);
			*a_pContext = NULL;
			return OpcUa_Good;
		}
#endif

#ifdef _GNUC
		if (!FindNextFile(pContext->File, &tFindFileData))
		{
			FindClose(pContext->File);
			OpcUa_Free(pContext);
			*a_pContext = NULL;
			return OpcUa_Good;
		}
#endif
	}

	match = false;

	do 
	{
		// build target path.
		std::string targetPath = a_sStorePath;

#ifndef _GNUC_
		if (a_sHasPrivateKey)
		{
			targetPath += "\\private\\";
		}
		else
		{
			targetPath += "\\certs\\";
		}
#else
		if (a_sHasPrivateKey)
		{
			targetPath += "/private/";
		}
		else
		{
			targetPath += "/certs/";
		}
#endif
#ifndef _GNUC_		
	#ifdef UNICODE
		OpcUa_String_WtoA((OpcUa_CharW*)tFindFileData.cFileName,&asFilename);
		targetPath.append(asFilename);
	#else
		targetPath.append(tFindFileData.cFileName);
	#endif
#else
		OpcUa_String_WtoA((OpcUa_CharW*)tFindFileData->d_name,&asFilename);
		targetPath.append(tFindFileData->d_name);
#endif
		// load private key from file.
		if (a_sHasPrivateKey)
		{
			uStatus = OpcUa_Certificate_LoadPrivateKeyFromFile(
				(OpcUa_StringA)targetPath.c_str(),
				OpcUa_Crypto_Encoding_PKCS12,
				a_sPassword,
				a_pCertificate,
				a_pPrivateKey);

			if (OpcUa_IsBad(uStatus))
			{
				continue;
			}
		}
		
		// load public key from file.
		else
		{
			uStatus = OpcUa_ReadFile((OpcUa_StringA)targetPath.c_str(), a_pCertificate);

			if (OpcUa_IsBad(uStatus))
			{
				continue;
			}
		}            
		// ici le CommonName dans le certificat doit correspondre au nom de l'application
		// autrement dit le nom du serveur
		// check for match.
		match = OpcUa_Certificate_CheckForMatch(a_pCertificate, a_sCommonName, a_sThumbprint);

		if (match)
		{
			//a_FullFileName=Utils::Copy(targetPath);
			targetPath.clear();
			break;
		}

		OpcUa_ByteString_Clear(a_pCertificate);
		OpcUa_Key_Clear(a_pPrivateKey);
	}

#ifdef WIN32
	while (FindNextFile(pContext->File, &tFindFileData));
#endif

#ifdef _GNUC_
	while (FindNextFile(pContext->File, &tFindFileData));
#endif

	// check if nothing found.
	if (!match)
	{
		FindClose(pContext->File);
		OpcUa_Free(pContext);
		*a_pContext = NULL;
		uStatus = OpcUa_BadNotFound;
	}
	else
		*a_pContext=/*(OpcUa_Handle*)*/pContext;
	OpcUa_Free(filePath);
	OpcUa_Free(asFilename);
	if (pszFilter)
	{
		OpcUa_Free(pszFilter);
		pszFilter=OpcUa_Null;
	}
	return uStatus;
}

/*============================================================================
 * OpcUa_Certificate_FreeFindContext
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Certificate_FreeFindContext(
	OpcUa_Handle* a_pContext)
{
	OpcUa_Certificate_FindContext* pContext = NULL;

OpcUa_InitializeStatus(OpcUa_Module_Crypto, "OpcUa_Certificate_FreeFindContext");

	OpcUa_ReturnErrorIfArgumentNull(a_pContext);

	if (*a_pContext != NULL)
	{
		pContext = (OpcUa_Certificate_FindContext*)*a_pContext;
	}

	if (pContext != NULL)
	{
		if (pContext->Context != NULL)
		{
#ifndef _GNUC_
			CertFreeCertificateContext(pContext->Context);
#else
			SSL_CTX_free(pContext->Context);
#endif
		}

		if (pContext->Store != NULL)
		{
#ifndef _GNUC_
			CertCloseStore(pContext->Store, 0);
#endif
		}

		if (pContext->File)
		{
			FindClose(pContext->File);
		}

		delete pContext;
	}

	*a_pContext = NULL;

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	// nothing to do.

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_Certificate_ExportPrivateKeyFromWindowsStore
 *===========================================================================*/
#ifdef WIN32
OpcUa_StatusCode OpcUa_Certificate_ExportPrivateKeyFromWindowsStore(
	OpcUa_Boolean     a_bUseMachineStore,
	OpcUa_StringA     a_sStoreName,
	OpcUa_ByteString* a_pCertificate,
	OpcUa_StringA     a_sPassword,
	OpcUa_StringA     a_sTargetStorePath,
	OpcUa_Key*        a_pPrivateKey)
{ 
	HCERTSTORE hMemoryStore = NULL;
	HCERTSTORE hCertificateStore = NULL;
	PCCERT_CONTEXT pCertContext = NULL;
	PCCERT_CONTEXT pCertContext2 = NULL;
	LPWSTR wszStoreName = NULL;
	LPWSTR wszPassword = NULL;
	std::string privateKeyFile;

	CRYPT_HASH_BLOB tThumbprint;
	CRYPT_DATA_BLOB tPfxData;
	OpcUa_Byte pHashBuffer[SHA_DIGEST_LENGTH];

	memset(&tPfxData,0,sizeof(CRYPT_DATA_BLOB));

	OpcUa_StatusCode uStatus = OpcUa_Good;

	if (a_sStoreName)
	{
		if (a_pCertificate)
		{
			if (a_sTargetStorePath)
			{
				if (a_pPrivateKey)
				{
					memset(&tThumbprint, 0, sizeof(tThumbprint));
					memset(&tPfxData, 0, sizeof(tPfxData));
					memset(&pHashBuffer, 0, sizeof(pHashBuffer));
					OpcUa_Key_Initialize(a_pPrivateKey);

					// open the certificate store.
					DWORD dwFlags = 0;

					if (a_bUseMachineStore)
						dwFlags |= CERT_SYSTEM_STORE_LOCAL_MACHINE;
					else
						dwFlags |= CERT_SYSTEM_STORE_CURRENT_USER;

					uStatus = OpcUa_String_AtoW(a_sStoreName, (OpcUa_CharW**)&wszStoreName);
					if (uStatus == OpcUa_Good)
					{
						// open the store.
						hCertificateStore = CertOpenStore(
							CERT_STORE_PROV_SYSTEM,
							0,
							0,
							dwFlags,
							wszStoreName);

						if (hCertificateStore)	
						{
							// compute the hash.
							SHA1(a_pCertificate->Data, a_pCertificate->Length, pHashBuffer);

							tThumbprint.pbData = pHashBuffer;
							tThumbprint.cbData = SHA_DIGEST_LENGTH;

							// find the certificate with the specified hash.
							pCertContext = CertFindCertificateInStore(
								hCertificateStore,
								X509_ASN_ENCODING,
								0,
								CERT_FIND_HASH,
								&tThumbprint,
								NULL);

							if (pCertContext )
							{
								// create memory store.
								hMemoryStore = CertOpenStore(
									CERT_STORE_PROV_MEMORY,
									0,
									0,
									0,
									OpcUa_Null);

								if (hMemoryStore)
								{								// create a link to the original certificate.
									BOOL bResult = CertAddCertificateLinkToStore(
										hMemoryStore,
										pCertContext,
										CERT_STORE_ADD_REPLACE_EXISTING,
										&pCertContext2);

									if (bResult)
									{
										// convert the password to unicode.
										if (a_sPassword != NULL)
											uStatus = OpcUa_String_AtoW(a_sPassword, (OpcUa_CharW**)&wszPassword);
										else
											uStatus = OpcUa_BadInvalidArgument;
										if (uStatus == OpcUa_Good)
										{
											// determine the size of the blob.
											bResult = PFXExportCertStoreEx(
												hMemoryStore,
												&tPfxData,
												wszPassword,
												NULL,
												EXPORT_PRIVATE_KEYS);

											if (bResult)												
											{
												// allocate memory.
												tPfxData.pbData = (BYTE*)OpcUa_Alloc(tPfxData.cbData);
												if (tPfxData.pbData)
												{
													memset(tPfxData.pbData, 0, tPfxData.cbData);

													// export the PFX blob.
													bResult = PFXExportCertStoreEx(
														hMemoryStore,
														&tPfxData,
														wszPassword,
														0,
														EXPORT_PRIVATE_KEYS);

													if (bResult)
													{

														// get the file name for the certificate.
														privateKeyFile = OpcUa_Certificate_GetFilePathForCertificate(
															a_sTargetStorePath,
															a_pCertificate,
															OpcUa_Crypto_Encoding_PKCS12,
															OpcUa_True);

														if (privateKeyFile.empty())
															uStatus = OpcUa_BadUnexpectedError;
														else
														{
															// write to the file.
															uStatus = OpcUa_WriteFile((OpcUa_StringA)privateKeyFile.c_str(), tPfxData.pbData, tPfxData.cbData);
															if (uStatus == OpcUa_Good)
															{
																// load the certificate that was just saved.
																uStatus = OpcUa_Certificate_LoadPrivateKeyFromStore(
																	a_sTargetStorePath,
																	OpcUa_Crypto_Encoding_PKCS12,
																	a_sPassword,
																	a_pCertificate,
																	a_pPrivateKey);

																if (uStatus == OpcUa_Good)
																{
																	// clean up.
																	CertCloseStore(hMemoryStore, 0);
																	CertCloseStore(hCertificateStore, 0);
																	OpcUa_Free(tPfxData.pbData);
																	OpcUa_Free(wszStoreName);
																	OpcUa_Free(wszPassword);
																}
															}
														}
													}
													else
														uStatus = OpcUa_BadUnexpectedError;
												}
												else
													uStatus = OpcUa_BadOutOfMemory;
											}
											else
												uStatus = OpcUa_BadUnexpectedError;
										}
									}
									else
										uStatus = OpcUa_BadUnexpectedError;
								}
								else
									uStatus = OpcUa_BadNotFound;
							}
							else
								uStatus = OpcUa_BadNotFound;
						}
						else
							uStatus = OpcUa_BadUnexpectedError;
					}
				}
				else
					uStatus = OpcUa_BadInvalidArgument;
			}
			else
				uStatus = OpcUa_BadInvalidArgument;
		}
		else
			uStatus = OpcUa_BadInvalidArgument;
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	if (uStatus != OpcUa_Good)
	{
		if (pCertContext)
			CertFreeCertificateContext(pCertContext);

		if (pCertContext2)
			CertFreeCertificateContext(pCertContext2);


		if (hMemoryStore)
			CertCloseStore(hMemoryStore, 0);


		if (hCertificateStore)
			CertCloseStore(hCertificateStore, 0);


		OpcUa_Key_Clear(a_pPrivateKey);
		if (tPfxData.pbData)
			OpcUa_Free(tPfxData.pbData);
		if (wszStoreName)
			OpcUa_Free(wszStoreName);
		if (wszPassword)
			OpcUa_Free(wszPassword);
	}
	return uStatus;
}

/*============================================================================
 * OpcUa_Certificate_ImportToWindowsStore
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Certificate_ImportToWindowsStore(
	OpcUa_ByteString* a_pCertificate,
	OpcUa_Boolean     a_bUseMachineStore,
	OpcUa_StringA     a_sStoreName)
{   
	HCERTSTORE hCertificateStore = NULL;
	LPWSTR wszStoreName = NULL;

OpcUa_InitializeStatus(OpcUa_Module_Crypto, "OpcUa_Certificate_ImportToWindowsStore");

	OpcUa_ReturnErrorIfArgumentNull(a_pCertificate);

	// import certificate.
	DWORD dwFlags = CERT_STORE_OPEN_EXISTING_FLAG;

	if (a_bUseMachineStore)
	{
		dwFlags |= CERT_SYSTEM_STORE_LOCAL_MACHINE;
	}
	else
	{
		dwFlags |= CERT_SYSTEM_STORE_CURRENT_USER;
	}

	uStatus = OpcUa_String_AtoW(a_sStoreName, (OpcUa_CharW**)&wszStoreName);
	OpcUa_GotoErrorIfBad(uStatus);

	// open the store.
	hCertificateStore = CertOpenStore(
	   CERT_STORE_PROV_SYSTEM,
	   0, 
	   0,                           
	   dwFlags, 
	   wszStoreName);

	if (hCertificateStore == 0)
	{
		OpcUa_GotoErrorWithStatus(OpcUa_BadUnexpectedError);
	}

	// add certificate to store.
	BOOL bResult = CertAddEncodedCertificateToStore(
		hCertificateStore,
		X509_ASN_ENCODING,
		a_pCertificate->Data,
		a_pCertificate->Length,
		CERT_STORE_ADD_REPLACE_EXISTING,
		NULL);

	if (!bResult)
	{
		OpcUa_GotoErrorWithStatus(OpcUa_BadDecodingError);
	}

	// clean up.
	CertCloseStore(hCertificateStore, 0);
	OpcUa_Free(wszStoreName);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	if (hCertificateStore != NULL)
	{
		CertCloseStore(hCertificateStore, 0);
	}

	OpcUa_Free(wszStoreName);

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_Certificate_ImportPrivateKeyToWindowsStore
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Certificate_ImportPrivateKeyToWindowsStore(
	OpcUa_StringA     a_sSourceStorePath,
	OpcUa_ByteString* a_pCertificate,
	OpcUa_StringA     a_sPassword,
	OpcUa_Boolean     a_bUseMachineStore,
	OpcUa_StringA     a_sStoreName)
{
	HCERTSTORE hFileStore = NULL;
	HCERTSTORE hCertificateStore = NULL;
	PCCERT_CONTEXT pCertContext = NULL;
	PCCERT_CONTEXT pCertContext2 = NULL;
	LPWSTR wszStoreName = NULL;
	LPWSTR wszPassword = NULL;
	CRYPT_DATA_BLOB tCertificateData;
	OpcUa_ByteString tFileData;
	std::string privateKeyFile;
	memset(&tCertificateData,0,sizeof(CRYPT_DATA_BLOB));
	OpcUa_InitializeStatus(OpcUa_Module_Crypto, "OpcUa_Certificate_ImportPrivateKeyToWindowsStore");

	OpcUa_ReturnErrorIfArgumentNull(a_sSourceStorePath);

	memset(&tCertificateData, 0, sizeof(CRYPT_DATA_BLOB));
	OpcUa_ByteString_Initialize(&tFileData);

	// get the file name for the certificate.
	privateKeyFile = OpcUa_Certificate_GetFilePathForCertificate(
		a_sSourceStorePath, 
		a_pCertificate, 
		OpcUa_Crypto_Encoding_PKCS12,
		OpcUa_False);

	if (privateKeyFile.empty() == OpcUa_True)
	{
		uStatus = OpcUa_BadUnexpectedError;
		goto Error;
	}
	else
	{
		// read the certificate from disk.
		uStatus = OpcUa_ReadFile((OpcUa_StringA)privateKeyFile.c_str(), &tFileData);
		OpcUa_GotoErrorIfBad(uStatus);

		// import certificate.
		DWORD dwFlags = CRYPT_EXPORTABLE;

		if (a_bUseMachineStore)
			dwFlags |= CRYPT_MACHINE_KEYSET;
		else
			dwFlags |= CRYPT_USER_KEYSET;

		uStatus = OpcUa_String_AtoW(a_sPassword, (OpcUa_CharW**)&wszPassword);
		OpcUa_GotoErrorIfBad(uStatus);

		tCertificateData.pbData = tFileData.Data;
		tCertificateData.cbData = tFileData.Length;

		hFileStore = PFXImportCertStore(&tCertificateData, wszPassword, dwFlags);

		if (hFileStore == 0)
		{
			if (wszPassword == NULL)
			{
				hFileStore = PFXImportCertStore(&tCertificateData, L"", dwFlags);
			}
			else
			{
				if (wszPassword[0] == '\0')
					hFileStore = PFXImportCertStore(&tCertificateData, wszPassword, dwFlags);
			}
			if (hFileStore == 0)
				OpcUa_GotoErrorWithStatus(OpcUa_BadDecodingError);
		}

		// open the certificate store.
		dwFlags = 0;

		if (a_bUseMachineStore)
			dwFlags |= CERT_SYSTEM_STORE_LOCAL_MACHINE;
		else
			dwFlags |= CERT_SYSTEM_STORE_CURRENT_USER;

		uStatus = OpcUa_String_AtoW(a_sStoreName, (OpcUa_CharW**)&wszStoreName);
		OpcUa_GotoErrorIfBad(uStatus);

		// open the store.
		hCertificateStore = CertOpenStore(
			CERT_STORE_PROV_SYSTEM,
			0,
			0,
			dwFlags,
			wszStoreName);

		if (hCertificateStore == 0)
		{
			uStatus = OpcUa_BadUnexpectedError;
			goto Error;
		}
		else
		{
			// Find the certificates in the system store. 
			pCertContext = CertEnumCertificatesInStore(hFileStore, pCertContext);
			while (pCertContext)
			{
				// add back into store.
				BOOL bResult = CertAddCertificateContextToStore(
					hCertificateStore,
					pCertContext,
					CERT_STORE_ADD_REPLACE_EXISTING,
					&pCertContext2);

				if (bResult == 0)
				{
					OpcUa_GotoErrorWithStatus(OpcUa_BadUnexpectedError);
				}

				CertFreeCertificateContext(pCertContext2);
				pCertContext2 = NULL;
				pCertContext = CertEnumCertificatesInStore(hFileStore, pCertContext);
			}

			// clean up.
			CertCloseStore(hFileStore, 0);
			CertCloseStore(hCertificateStore, 0);
			OpcUa_Free(tCertificateData.pbData);
			OpcUa_Free(wszStoreName);
			OpcUa_Free(wszPassword);
			OpcUa_ByteString_Clear(&tFileData);
		}
	}
OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	if (pCertContext != NULL)
		CertFreeCertificateContext(pCertContext);

	if (pCertContext2 != NULL)
		CertFreeCertificateContext(pCertContext2);

	if (hFileStore != NULL)
		CertCloseStore(hFileStore, 0);

	if (hCertificateStore != NULL)
		CertCloseStore(hCertificateStore, 0);
	OpcUa_ByteString_Clear(&tFileData);
	OpcUa_Free(tCertificateData.pbData);
	OpcUa_Free(wszStoreName);
	OpcUa_Free(wszPassword);

OpcUa_FinishErrorHandling;
}
#endif
/*============================================================================
 * OpcUa_Certificate_LookupDomainName
 *===========================================================================*/
OpcUa_StatusCode OpcUa_LookupDomainName(
	OpcUa_StringA  a_sAddress,
	OpcUa_StringA* a_pDomainName)
{
	struct sockaddr_in tAddress;
	char sHostname[NI_MAXHOST];
	OpcUa_StatusCode uStatus=OpcUa_Good;

	OpcUa_ReturnErrorIfArgumentNull(a_sAddress);
	OpcUa_ReturnErrorIfArgumentNull(a_pDomainName);

	*a_pDomainName = NULL;

	OpcUa_MemSet(&tAddress, 0, sizeof(tAddress));
	OpcUa_MemSet(sHostname, 0, sizeof(sHostname));

	tAddress.sin_family = AF_INET;
	tAddress.sin_addr.s_addr = inet_addr(a_sAddress);
	tAddress.sin_port = htons(0);

	int iResult = getnameinfo(
		(struct sockaddr*)&tAddress,
		sizeof(sockaddr_in),
		sHostname,
		NI_MAXHOST, 
		NULL, 
		0, 
		NI_NAMEREQD);

	if (iResult != 0) 
		uStatus=OpcUa_BadCommunicationError;
	else
	{
		int iLength = strlen(sHostname)+1;
		*a_pDomainName = (OpcUa_StringA)OpcUa_Alloc(iLength);
		if (!a_pDomainName)
		{
			uStatus=OpcUa_BadOutOfMemory;
		}
		else
		{
			// TODO : toujours strcpy
#ifdef WIN32
			strcpy_s(*a_pDomainName, iLength, sHostname);
#else
			strcpy(*a_pDomainName, sHostname);
#endif
		}

	}
	return uStatus;
}


/*============================================================================
 * OpcUa_Certificate_LookupLocalhostNames
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Certificate_LookupLocalhostNames(
	OpcUa_StringA** a_pHostNames,
	OpcUa_UInt32*   a_pNoOfHostNames)
{
	char sBuffer[NI_MAXHOST];
	std::vector<OpcUa_String*> hostnames;    
	struct addrinfo* pResult = NULL;
	struct addrinfo tHints;
	OpcUa_Int32 iResult=0;
	OpcUa_StatusCode uStatus=OpcUa_Good;

	if (a_pHostNames)
	{
		if (a_pNoOfHostNames)
		{
			*a_pHostNames = NULL;
			*a_pNoOfHostNames = 0;

			memset(&tHints, 0, sizeof(tHints));

			if (gethostname(sBuffer, sizeof(sBuffer)) == SOCKET_ERROR) 			
				uStatus=OpcUa_BadCommunicationError;
			else
			{
				OpcUa_String* pString=(OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
				OpcUa_String_Initialize(pString);
				OpcUa_String_AttachCopy(pString,sBuffer);
				hostnames.push_back(pString);

				tHints.ai_family = AF_UNSPEC;
				tHints.ai_socktype = SOCK_STREAM;
				tHints.ai_protocol = IPPROTO_TCP;

				iResult = getaddrinfo(sBuffer, NULL, &tHints, &pResult);

				if (iResult != 0) 				
					uStatus=OpcUa_BadCommunicationError;
				else
				{
					for (struct addrinfo* ptr = pResult; ptr != NULL ; ptr = ptr->ai_next) 
					{
						if (ptr->ai_family == AF_INET)
						{
							struct sockaddr_in tAddress;
							memcpy(&tAddress, ptr->ai_addr, sizeof(struct sockaddr_in));
							//
							pString=(OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
							OpcUa_String_Initialize(pString);
							OpcUa_String_AttachCopy(pString,inet_ntoa(tAddress.sin_addr));
							hostnames.push_back(pString);
						}
					}

					freeaddrinfo(pResult);
					pResult = NULL;
					if (pString)
						OpcUa_String_Clear(pString);
					uStatus = OpcUa_Certificate_CopyStrings(hostnames, a_pHostNames, a_pNoOfHostNames);
					if (uStatus==OpcUa_Good)
					{
						for (OpcUa_UInt32 iii=0;iii<hostnames.size();iii++)
						{
							pString=hostnames[iii];
							if (pString)
							{
								OpcUa_String_Clear(pString);
								OpcUa_Free(pString);
							}
						}
						hostnames.clear();
					}
				}
			}
		}
		else
			uStatus=OpcUa_BadInvalidArgument;
	}
	else
		uStatus=OpcUa_BadInvalidArgument;
	if (uStatus!=OpcUa_Good)
	{
		for (OpcUa_UInt32 iii=0;iii<hostnames.size();iii++)
		{
			OpcUa_String* pStringErr=hostnames[iii];
			if (pStringErr)
			{
				OpcUa_String_Clear(pStringErr);
				OpcUa_Free(pStringErr);
			}
		}
		hostnames.clear();
		if (!pResult)
			freeaddrinfo(pResult);
	}
	return uStatus;
}
// Charge un biblioth?que
// LibraryName = Nom de la librairie a charger
// hInst = output handle associ? a la librairie charg?
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_LoadLibrary(const OpcUa_String* LibraryName, void** hInst)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
#ifdef WIN32
	#ifdef UNICODE
	LPWSTR pStr=NULL;
	OpcUa_String_AtoW(OpcUa_String_GetRawString(LibraryName),(OpcUa_CharW**)&pStr);
	*hInst=LoadLibrary(pStr);
	#else
	*hInst=LoadLibrary(OpcUa_String_GetRawString(LibraryName));
	#endif
	if (!(*hInst))
	{
		DWORD dwError=GetLastError();
		if (dwError==126)
			uStatus=OpcUa_BadFileNotFound;
		else
			uStatus=OpcUa_Bad;
	}
	//OpcUa_Free(pStr);
#else
	#ifdef _GNUC_
		*hInst=dlopen(OpcUa_String_GetRawString(LibraryName),RTLD_NOW);
		if (!(*hInst))
			uStatus=OpcUa_Bad;
	#endif
#endif
	return uStatus;
}
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_FreeLibrary(void* hInst)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	if (hInst)
	{
#ifdef WIN32
		HINSTANCE hInstance=(HINSTANCE)hInst;
		if (!FreeLibrary(hInstance))
			uStatus=OpcUa_BadInternalError;
#endif

#ifdef _GNUC_
		dlclose(hInst);
#endif
	}
	else
		uStatus=OpcUa_BadInvalidArgument;
	return uStatus;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Opc UA certificate create pki provide. </summary>
///
/// <remarks>	Michel, 05/06/2016. </remarks>
///
/// <param name="szRootStoreLocation">	  	[in,out] If non-null, the root store location. </param>
/// <param name="szTrustedLocation">	  	[in,out] If non-null, the trusted location. </param>
/// <param name="szRevokedLocation">	  	[in,out] If non-null, the revoked location. </param>
/// <param name="szIssuerLocation">		  	[in,out] If non-null, the issuer location. </param>
/// <param name="szRevokedIssuerLocation">	[in,out] If non-null, the revoked issuer location. </param>
/// <param name="ppPkiConfig">			  	[in,out] If non-null, the pki configuration. </param>
/// <param name="pPkiProvide">			  	[in,out] If non-null, the pki provide. </param>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode OpcUa_Certificate_CreatePkiProvider(OpcUa_CharA* szRootStoreLocation,
													OpcUa_CharA* szTrustedLocation,
													OpcUa_CharA* szRevokedLocation,
													OpcUa_CharA* szIssuerLocation,
													OpcUa_CharA* szRevokedIssuerLocation,
													OpcUa_CertificateStoreConfiguration** ppPkiConfig,
													OpcUa_PKIProvider** pPkiProvide)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;

	// Check the output parameter
	if (ppPkiConfig)
	{
		if (pPkiProvide)
		{ 
			if (!(*ppPkiConfig))			
				(*ppPkiConfig) = (OpcUa_CertificateStoreConfiguration*)OpcUa_Alloc(sizeof(OpcUa_CertificateStoreConfiguration));
			OpcUa_CertificateStoreConfiguration_Initialize((*ppPkiConfig));

			// specify the trust list to used by the stack to validate certificates. 
			(*ppPkiConfig)->strPkiType = (OpcUa_StringA)OPCUA_P_PKI_TYPE_OPENSSL;
			(*ppPkiConfig)->uFlags = 0; //OpcUa_PKI_CheckRevocationStatus;

			OpcUa_CharA* szTmpChar = (OpcUa_CharA*)OpcUa_Alloc(1024);
			// Trusted Location
			ZeroMemory(szTmpChar, 1024);
			OpcUa_StrnCpyA(szTmpChar, OpcUa_StrLenA(szTmpChar), szRootStoreLocation, OpcUa_StrLenA(szRootStoreLocation));
			// Same call for Windows and Linux. The caller function shall support "/" or "\" . \ 
			OpcUa_StrnCatA(szTmpChar, OpcUa_StrLenA(szTmpChar), szTrustedLocation, OpcUa_StrLenA(szTrustedLocation)); // \certs

			(*ppPkiConfig)->strTrustedCertificateListLocation = (OpcUa_CharA*)OpcUa_Alloc(OpcUa_StrLenA(szTmpChar) + 1);
			ZeroMemory((*ppPkiConfig)->strTrustedCertificateListLocation, OpcUa_StrLenA(szTmpChar) + 1);
			OpcUa_StrnCpyA(
				(*ppPkiConfig)->strTrustedCertificateListLocation, OpcUa_StrLenA((*ppPkiConfig)->strTrustedCertificateListLocation),
				szTmpChar, OpcUa_StrLenA(szTmpChar));

			// Revoked Location
			ZeroMemory(szTmpChar, 1024);
			OpcUa_StrnCpyA(szTmpChar, OpcUa_StrLenA(szTmpChar), szRootStoreLocation, OpcUa_StrLenA(szRootStoreLocation));
			// Same call for Windows and Linux. The caller function shall support "/" or "\" . \ 
			OpcUa_StrnCatA(szTmpChar, OpcUa_StrLenA(szTmpChar), szRevokedLocation, OpcUa_StrLenA(szRevokedLocation)); // \rejected

			(*ppPkiConfig)->strRevokedCertificateListLocation = (OpcUa_CharA*)OpcUa_Alloc(OpcUa_StrLenA(szTmpChar) + 1);
			ZeroMemory((*ppPkiConfig)->strRevokedCertificateListLocation, OpcUa_StrLenA(szTmpChar) + 1);
			OpcUa_StrnCpyA(
				(*ppPkiConfig)->strRevokedCertificateListLocation, OpcUa_StrLenA((*ppPkiConfig)->strRevokedCertificateListLocation),
				szTmpChar, OpcUa_StrLenA(szTmpChar));

			// Issuer Location
			ZeroMemory(szTmpChar, 1024);
			OpcUa_StrnCpyA(szTmpChar, OpcUa_StrLenA(szTmpChar), szRootStoreLocation, OpcUa_StrLenA(szRootStoreLocation));
			// Same call for Windows and Linux. The caller function shall support "/" or "\" . \ 
			OpcUa_StrnCatA(szTmpChar, OpcUa_StrLenA(szTmpChar), szIssuerLocation, OpcUa_StrLenA(szIssuerLocation)); // \auth

			(*ppPkiConfig)->strIssuerCertificateStoreLocation = (OpcUa_CharA*)OpcUa_Alloc(OpcUa_StrLenA(szTmpChar) + 1);
			ZeroMemory((*ppPkiConfig)->strIssuerCertificateStoreLocation, OpcUa_StrLenA(szTmpChar) + 1);
			OpcUa_StrnCpyA(
				(*ppPkiConfig)->strIssuerCertificateStoreLocation, OpcUa_StrLenA((*ppPkiConfig)->strIssuerCertificateStoreLocation),
				szTmpChar, OpcUa_StrLenA(szTmpChar));

			// RevokedIssuer Location
			ZeroMemory(szTmpChar, 1024);
			OpcUa_StrnCpyA(szTmpChar, OpcUa_StrLenA(szTmpChar), szRootStoreLocation, OpcUa_StrLenA(szRootStoreLocation));
			// Same call for Windows and Linux. The caller function shall support "/" or "\" . \ 
			OpcUa_StrnCatA(szTmpChar, OpcUa_StrLenA(szTmpChar), szRevokedIssuerLocation, OpcUa_StrLenA(szRevokedIssuerLocation)); // \rejected

			(*ppPkiConfig)->strRevokedIssuerCertificateListLocation = (OpcUa_CharA*)OpcUa_Alloc(OpcUa_StrLenA(szTmpChar) + 1);
			ZeroMemory((*ppPkiConfig)->strRevokedIssuerCertificateListLocation, OpcUa_StrLenA(szTmpChar) + 1);
			OpcUa_StrnCpyA(
				(*ppPkiConfig)->strRevokedIssuerCertificateListLocation, OpcUa_StrLenA((*ppPkiConfig)->strRevokedIssuerCertificateListLocation),
				szTmpChar, OpcUa_StrLenA(szTmpChar));
			OpcUa_Free(szTmpChar);
			// Now let create the PkiProvider
			uStatus = OpcUa_PKIProvider_Create((*ppPkiConfig), (*pPkiProvide));

			if (uStatus != OpcUa_Good)
				OpcUa_Trace(OPCUA_TRACE_EXTRA_LEVEL_ERROR, "OpcUa_Certificate_CreatePkiProvider uStatus=0x%05x",uStatus);
		}
		else
			uStatus = OpcUa_BadInvalidArgument;
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}