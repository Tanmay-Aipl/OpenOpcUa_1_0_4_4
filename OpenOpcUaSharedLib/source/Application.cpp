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
#include "Application.h"

#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/err.h>
using namespace OpenOpcUa;
using namespace UASharedLib;

#define CLIENT_SERIALIZER_MAXALLOC                   16777216
#define CLIENT_ENCODER_MAXSTRINGLENGTH               ((OpcUa_UInt32)16777216)
#define CLIENT_ENCODER_MAXARRAYLENGTH                ((OpcUa_UInt32)65536)
#define CLIENT_ENCODER_MAXBYTESTRINGLENGTH           ((OpcUa_UInt32)16777216)
#define CLIENT_ENCODER_MAXMESSAGELENGTH              ((OpcUa_UInt32)16777216)
#define CLIENT_SECURELISTENER_THREADPOOL_MINTHREADS  5
#define CLIENT_SECURELISTENER_THREADPOOL_MAXTHREADS  5
#define CLIENT_SECURELISTENER_THREADPOOL_MAXJOBS     20
#define CLIENT_SECURITYTOKEN_LIFETIME_MAX            3600000
#define CLIENT_SECURITYTOKEN_LIFETIME_MIN            60000
#define CLIENT_TCPLISTENER_DEFAULTCHUNKSIZE          ((OpcUa_UInt32)65536)
#define CLIENT_TCPCONNECTION_DEFAULTCHUNKSIZE        ((OpcUa_UInt32)65536)

//============================================================================
// CApplication::Constructor
//============================================================================
CApplication::CApplication(void)
{
	m_hPlatformLayer = 0;
	m_pPkiConfig = (OpcUa_CertificateStoreConfiguration*)OpcUa_Alloc(sizeof(OpcUa_CertificateStoreConfiguration));
	OpcUa_CertificateStoreConfiguration_Initialize(m_pPkiConfig);
	m_pUserX509PkiConfig = (OpcUa_CertificateStoreConfiguration*)OpcUa_Alloc(sizeof(OpcUa_CertificateStoreConfiguration));
	OpcUa_CertificateStoreConfiguration_Initialize(m_pUserX509PkiConfig);
	ZeroMemory(&m_tConfiguration, sizeof(OpcUa_ProxyStubConfiguration));
	m_pPkiProvider = (OpcUa_PKIProvider*)OpcUa_Alloc(sizeof(OpcUa_PKIProvider));
	memset(m_pPkiProvider, 0, sizeof(OpcUa_PKIProvider));
	m_pUserX509PkiProvider = (OpcUa_PKIProvider*)OpcUa_Alloc(sizeof(OpcUa_PKIProvider));
	m_pCertificate = (OpcUa_ByteString*)OpcUa_Alloc(sizeof(OpcUa_ByteString));
	if (m_pCertificate)
		OpcUa_ByteString_Initialize(m_pCertificate);
	m_pPrivateKey = OpcUa_Null;
	m_pThumbprint=(OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	if (m_pThumbprint)
		OpcUa_String_Initialize(m_pThumbprint);
	
	OpcUa_String_Initialize(&m_certificateStorePath);
	m_pApplicationName=(OpcUa_LocalizedText*)OpcUa_Alloc(sizeof(OpcUa_LocalizedText));
	if (m_pApplicationName)
		OpcUa_LocalizedText_Initialize(m_pApplicationName);
	m_bPfxDerValide=OpcUa_False;
}

//============================================================================
// CApplication::Destructor
//============================================================================
CApplication::~CApplication(void)
{
	if (m_pPrivateKey)
	{
		OpcUa_Key_Clear(m_pPrivateKey);
		OpcUa_Free(m_pPrivateKey);
	}
	if (m_pApplicationName)
	{
		OpcUa_LocalizedText_Clear(m_pApplicationName);
		OpcUa_Free(m_pApplicationName);
		m_pApplicationName = OpcUa_Null;
	}
	if (m_pCertificate)
	{
		OpcUa_ByteString_Clear(m_pCertificate);
		OpcUa_Free(m_pCertificate);
		m_pCertificate = OpcUa_Null;
	}
	if (m_pPkiProvider)
	{
		OpcUa_PKIProvider_Delete(m_pPkiProvider);
		OpcUa_Free(m_pPkiProvider);
	}
	if (m_pUserX509PkiProvider)
	{
		OpcUa_PKIProvider_Delete(m_pUserX509PkiProvider);
		OpcUa_Free(m_pUserX509PkiProvider);
	}
	OpcUa_CertificateStoreConfiguration_Clear(m_pPkiConfig);
	OpcUa_Free(m_pPkiConfig);
	OpcUa_CertificateStoreConfiguration_Clear(m_pUserX509PkiConfig);
	OpcUa_Free(m_pUserX509PkiConfig);
	if (m_pThumbprint)
	{
		OpcUa_String_Clear(m_pThumbprint);
		OpcUa_Free(m_pThumbprint);	
	}
	OpcUa_String_Clear(&m_certificateStorePath);
	if (m_hPlatformLayer != 0)
	{
		OpcUa_ProxyStub_Clear();
		OpcUa_P_Clean(&m_hPlatformLayer);
		m_hPlatformLayer = 0;
	}
	OpcUa_EncodeableTypeTable_Delete(&m_tTypeTable);
}

//============================================================================
// CApplication::Uninitialize
//============================================================================
void CApplication::Uninitialize(void)
{
	//Cleanup();
}

//============================================================================
// CApplication::Initialize the abstraction layer. 
//============================================================================
void CApplication::InitializeAbstractionLayer(void)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	
	try
	{
		// initialize the abstraction layer. 
		m_hPlatformLayer = 0;
		uStatus = OpcUa_P_Initialize(&m_hPlatformLayer);
		ThrowIfBad(uStatus, (OpcUa_CharA*)"Could not initialize platform layer.");
		// initialisation du ProxyStub.
		// Il s'agit en fait des paramètres généraux de la stack mise en place par cette application
		// these parameters control tracing.
		m_tConfiguration.uProxyStub_Trace_Output				= OPCUA_TRACE_OUTPUT_CONSOLE;
		m_tConfiguration.uProxyStub_Trace_Level					= OPCUA_TRACE_OUTPUT_ALL_ERROR;

		// these parameters are used to protect against buffer overflows caused by bad data.
		// they may need to be adjusted depending on the needs of the CApplication.
		// the server also sets these limits which means errors could occur even if these limits are raised. 
		m_tConfiguration.iSerializer_MaxAlloc                  = CLIENT_SERIALIZER_MAXALLOC;
		m_tConfiguration.iSerializer_MaxStringLength           = CLIENT_ENCODER_MAXSTRINGLENGTH;
		m_tConfiguration.iSerializer_MaxByteStringLength       = CLIENT_ENCODER_MAXARRAYLENGTH;
		m_tConfiguration.iSerializer_MaxArrayLength            = CLIENT_ENCODER_MAXBYTESTRINGLENGTH;
		m_tConfiguration.iSerializer_MaxMessageSize            = CLIENT_ENCODER_MAXMESSAGELENGTH;

		// the thread pool is only used in a server to dispatch incoming requests.
		m_tConfiguration.bSecureListener_ThreadPool_Enabled    = OpcUa_False;
		m_tConfiguration.iSecureListener_ThreadPool_MinThreads = CLIENT_SECURELISTENER_THREADPOOL_MINTHREADS;
		m_tConfiguration.iSecureListener_ThreadPool_MaxThreads = CLIENT_SECURELISTENER_THREADPOOL_MAXTHREADS;
		m_tConfiguration.iSecureListener_ThreadPool_MaxJobs    = CLIENT_SECURELISTENER_THREADPOOL_MAXJOBS;
		m_tConfiguration.bSecureListener_ThreadPool_BlockOnAdd = OpcUa_True;
		m_tConfiguration.uSecureListener_ThreadPool_Timeout    = OPCUA_INFINITE;

		// these parameters are used to tune performance. larger chunks == more memory, slower performance.
		m_tConfiguration.iTcpListener_DefaultChunkSize         = CLIENT_TCPLISTENER_DEFAULTCHUNKSIZE;
		m_tConfiguration.iTcpConnection_DefaultChunkSize       = CLIENT_TCPCONNECTION_DEFAULTCHUNKSIZE;
		m_tConfiguration.iTcpTransport_MaxMessageLength        = CLIENT_ENCODER_MAXMESSAGELENGTH;
		m_tConfiguration.iTcpTransport_MaxChunkCount           = -1;
		m_tConfiguration.bTcpListener_ClientThreadsEnabled     = OpcUa_False;
		m_tConfiguration.bTcpStream_ExpectWriteToBlock         = OpcUa_True;
		// Initialize the default Trace buffer
		m_tConfiguration.hOutFileNoOfEntriesMax=5;
		m_tConfiguration.hOutFileNoOfEntries=0;
		// initialize the stack.		
		uStatus = OpcUa_ProxyStub_Initialize(m_hPlatformLayer, &m_tConfiguration); //OpcUa_ProxyStubConfiguration
		ThrowIfBad(uStatus, (OpcUa_CharA*)"Could not initialize proxy/stubs.");

		// create type table.	
		uStatus = OpcUa_EncodeableTypeTable_Create(&m_tTypeTable);
		ThrowIfBad(uStatus, (OpcUa_CharA*)"Could not create type table.");

		// initialize table with known types.
		OpcUa_Int32 iCount=0;
		uStatus = OpcUa_EncodeableTypeTable_AddTypes(&m_tTypeTable, iCount, OpcUa_KnownEncodeableTypes);
		ThrowIfBad(uStatus, (OpcUa_CharA*)"Could initializes type table.");
	}
	catch (...)
	{
		throw;
	}
}
OpcUa_StatusCode CApplication::InitializeTrace()
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_String	strFileName;
	OpcUa_String_Initialize(&strFileName);
	OpcUa_Trace_GetTraceFile(&strFileName);
	/* initialize tracer */
	if (m_tConfiguration.uProxyStub_Trace_Output!=OPCUA_TRACE_OUTPUT_NONE)
	{
		FILE* hFile=OpcUa_Null;
		uStatus = OpcUa_Trace_Initialize(m_tConfiguration.uProxyStub_Trace_Level, m_tConfiguration.uProxyStub_Trace_Output,strFileName,&hFile);
		if (uStatus==OpcUa_Good)
		{
			OpcUa_Trace(OPCUA_TRACE_EXTRA_LEVEL_WARNING, "OpcUa_ProxyStub_Initialize: Tracer has been initialized!\n");
			m_tConfiguration.hProxyStub_OutFile=hFile;
		}/*
		else
			OpcUa_Trace(OPCUA_TRACE_EXTRA_LEVEL_INFO, "OpcUa_ProxyStub_Initialize: Tracer initialization failed!: 0x%05x\n", uStatus);*/
	}
	OpcUa_String_Clear(&strFileName);
	return uStatus;
}
OpcUa_StatusCode CApplication::LoadPFXCertificate()
{
	OpcUa_StatusCode				uStatus = OpcUa_Good;
	WIN32_FIND_DATA					tFindFileData;
	OpcUa_StringA					filePath=(OpcUa_StringA)OpcUa_Alloc(1024);
	wchar_t* 						szFilter = OpcUa_Null;
	std::string						targetPath = OpcUa_String_GetRawString(&m_certificateStorePath);
	OpcUa_StringA					asFilename = OpcUa_Null;
	OpcUa_ByteString				aCertificate ;
	OpcUa_StringA					a_sPassword=OpcUa_Null;
	OpcUa_Key						aPrivateKey;
	OpcUa_ByteString_Initialize(&aCertificate);
	OpcUa_Key_Initialize(&aPrivateKey);
	OpcUa_Certificate_FindContext*	pContext = (OpcUa_Certificate_FindContext*)OpcUa_Alloc(sizeof(OpcUa_Certificate_FindContext));
	
	if (pContext)
	{
		ZeroMemory(pContext,sizeof(OpcUa_Certificate_FindContext));
		ZeroMemory(filePath,1024);
		OpcUa_UInt32 iLen=OpcUa_String_StrLen(&m_certificateStorePath);
		OpcUa_MemCpy(filePath,iLen,OpcUa_String_GetRawString(&m_certificateStorePath),iLen);
		// Chemin UNIX Vs Chemin Win32
#ifndef _GNUC_
		OpcUa_StrnCatA(&filePath[iLen],1024,"\\private\\*.pfx",OpcUa_StrLenA("\\private\\*.pfx"));
#else
		OpcUa_StrnCatA(&filePath[iLen],1024,"//private//*.pfx",OpcUa_StrLenA("//private//*.pfx"));
#endif
		OpcUa_MemSet(&tFindFileData, 0, sizeof(tFindFileData));
		uStatus = OpcUa_String_AtoW((OpcUa_StringA)filePath, (OpcUa_CharW**)&szFilter);
		if (uStatus == OpcUa_Good)
		{
			// maintenant on recherche le fichier
#ifdef _GNUC_
			pContext->File = FindFirstFile((char*)filePath, &tFindFileData);
#else
#ifdef UNICODE
			pContext->File = FindFirstFile((wchar_t*)szFilter, &tFindFileData);
#else
			pContext->File = FindFirstFile((char*)filePath, &tFindFileData);
#endif
#endif
			if (szFilter)
				OpcUa_Free(szFilter);
			if (INVALID_HANDLE_VALUE != pContext->File)
			{
				OpcUa_Boolean bResult = OpcUa_True;
				while ((INVALID_HANDLE_VALUE != pContext->File) && (bResult))
				{
					targetPath.clear();
					targetPath = OpcUa_String_GetRawString(&m_certificateStorePath);
					// check the certificate info
#ifndef _GNUC_		
#ifdef UNICODE
					OpcUa_String_WtoA((OpcUa_CharW*)tFindFileData.cFileName, &asFilename);
					targetPath.append("\\private\\");
					targetPath.append(asFilename);
#else
					targetPath.append("//private//");
					targetPath.append(tFindFileData.cFileName);
#endif
#else
					OpcUa_String_WtoA((OpcUa_CharW*)tFindFileData->d_name,&asFilename);
					targetPath.append("//private//");
					targetPath.append(tFindFileData->d_name);
#endif

					uStatus = OpcUa_Certificate_LoadPrivateKeyFromFile(
						(OpcUa_StringA)targetPath.c_str(),
						OpcUa_Crypto_Encoding_PKCS12,
						a_sPassword,
						&aCertificate,
						&aPrivateKey);
					if (uStatus == OpcUa_Good)
					{
						SetCertificate(&aCertificate);
						OpcUa_ByteString_Clear(&aCertificate);
						SetPrivateKey(aPrivateKey);
						OpcUa_ByteString_Clear(&(aPrivateKey.Key));
						break; // will break the will search loop
					}
					// Get the next certificate
#ifdef _GNUC_		
					bResult=(OpcUa_Boolean)FindNextFile(pContext->File, &tFindFileData);
#else 
					bResult = (OpcUa_Boolean)FindNextFile(pContext->File, &tFindFileData);
#endif  
					targetPath.clear();
				}
				FindClose(pContext->File);
			}
			else
			{
				// Not found in first call so we have to create the certificate
				uStatus = OpcUa_BadNotFound;
			}
		}
		OpcUa_Free(pContext);
	}
	else
		uStatus=OpcUa_BadOutOfMemory;

	if (asFilename)
		OpcUa_Free(asFilename);
	if (filePath)
		OpcUa_Free(filePath);
	return uStatus;
}
OpcUa_StatusCode CApplication::LoadDERCertificate()
{
	OpcUa_StatusCode				uStatus = OpcUa_Good;
	WIN32_FIND_DATA					tFindFileData;
	OpcUa_StringA					filePath=(OpcUa_StringA)OpcUa_Alloc(1024);
	LPWSTR*							pszFilter = OpcUa_Null;
	std::string						targetPath = OpcUa_String_GetRawString(&m_certificateStorePath);
#ifndef _GNUC_
	#ifdef UNICODE
		OpcUa_CharA*					asFilename = OpcUa_Null;
	#endif
#endif
	OpcUa_ByteString				aCertificate ;

	OpcUa_ByteString_Initialize(&aCertificate);
	OpcUa_Certificate_FindContext*	pContext = (OpcUa_Certificate_FindContext*)OpcUa_Alloc(sizeof(OpcUa_Certificate_FindContext));
	ZeroMemory(pContext,sizeof(OpcUa_Certificate_FindContext));
	if (pContext)
	{
		ZeroMemory(filePath,1024);
		OpcUa_UInt32 iLen=OpcUa_String_StrLen(&m_certificateStorePath);
		OpcUa_MemCpy(filePath,iLen,OpcUa_String_GetRawString(&m_certificateStorePath),iLen);
		// Chemin UNIX Vs Chemin Win32
#ifndef _GNUC_
		OpcUa_StrnCatA(&filePath[iLen],1024,"\\certs\\*.der",OpcUa_StrLenA("\\certs\\*.der"));
#else
		OpcUa_StrnCatA(&filePath[iLen],1024,"//certs//*.der",OpcUa_StrLenA("//certs//*.der"));
#endif
		OpcUa_MemSet(&tFindFileData, 0, sizeof(tFindFileData));
		//OpcUa_String_AtoW((OpcUa_StringA)filePath,(OpcUa_CharW**)&pszFilter);
		// maintenant on recherche le fichier
#ifdef _GNUC_
		pContext->File = FindFirstFile((char*)filePath, &tFindFileData);
#else
	#ifdef UNICODE
		OpcUa_String_AtoW((OpcUa_StringA)filePath, (OpcUa_CharW**)&pszFilter);
		pContext->File = FindFirstFile((wchar_t*)pszFilter, &tFindFileData);
	#else
		pContext->File = FindFirstFile((char*)filePath, &tFindFileData);
	#endif
#endif
		if (INVALID_HANDLE_VALUE != pContext->File)
		{
			OpcUa_Boolean bResult=OpcUa_True;
			while ( (INVALID_HANDLE_VALUE != pContext->File) && (bResult) )
			{
				targetPath.clear();
				targetPath= OpcUa_String_GetRawString(&m_certificateStorePath);
				// check the certificate info
	#ifndef _GNUC_		
		#ifdef UNICODE
				OpcUa_String_WtoA((OpcUa_CharW*)tFindFileData.cFileName,&asFilename);
				targetPath.append("\\certs\\");
				targetPath.append(asFilename);
				if (asFilename)
				{
					ZeroMemory(asFilename,strlen(asFilename));
					OpcUa_Free(asFilename);
					asFilename=OpcUa_Null;
				}
		#else
				targetPath.append("//certs//");
				targetPath.append(tFindFileData.cFileName);
		#endif
	#else
				//OpcUa_String_WtoA((OpcUa_CharW*)tFindFileData->d_name,&asFilename);
				targetPath.append("//certs//");
				targetPath.append(tFindFileData->d_name);
	#endif
				char* pszTargetPath = (char*)targetPath.c_str();
				uStatus = OpcUa_ReadFile(pszTargetPath, &aCertificate);
				if ( (uStatus==OpcUa_BadDecodingError) || (uStatus==OpcUa_BadInvalidArgument))
				{
					OpcUa_ByteString_Clear(&aCertificate);
					break;
				}
				if (uStatus==OpcUa_Good)
				{
					// Il faut comparer cette clé a la clé contenu dans le certificat privée (pfx en l'occurence)
					OpcUa_ByteString* pCertificateFromPFX=GetCertificate();
					if (Utils::IsEqual(pCertificateFromPFX,&aCertificate) )
					{
						m_bPfxDerValide=OpcUa_True;
						break;
					}
					else
						uStatus=OpcUa_BadCertificateInvalid;
					OpcUa_ByteString_Clear(&aCertificate);
				}
				// Get the next certificate
#ifdef _GNUC_		
				bResult=(OpcUa_Boolean)FindNextFile(pContext->File, &tFindFileData);
#else 
				bResult=(OpcUa_Boolean)FindNextFile(pContext->File, &tFindFileData);
#endif
			}
			FindClose(pContext->File);
		}
		else
		{
			// Not found in first call so we have to create the certificate
			uStatus=OpcUa_BadNotFound;
		}
		if (pszFilter)
			OpcUa_Free(pszFilter);
		OpcUa_Free(pContext);
	}
	else
		uStatus=OpcUa_BadOutOfMemory;
	targetPath.clear();
	OpcUa_ByteString_Clear(&aCertificate);
	OpcUa_Free(filePath);
	return uStatus;
}
//============================================================================
// CApplication::InitializeSecurity
// Recherche du certificat dans la repository en cours d'utilisation
// Il peut s'agir de la repository Windows ou d'un mécanisme basé sur des fichiers
// et des repertoires. Si le certificat n'existe pas celui ci est créé.
// OpcUa_LocalizedText* sApplicationName paramètre en in/out. 
// Permet de créer le certificat et/ou de récupérer le CommonName dans le certificat precedement créé
//============================================================================
OpcUa_StatusCode CApplication::InitializeSecurity( 
												  OpcUa_String* sApplicationUri,
												  OpcUa_LocalizedText* sApplicationName)
{	
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_ByteString tCertificate;
	OpcUa_String aFullFileName;
	OpcUa_Key tPrivateKey;
	//OpcUa_Handle pContext = OpcUa_Null;
	OpcUa_CharA* sThumbprint = OpcUa_Null;
	OpcUa_CharA* lApplicationUri = OpcUa_Null;
	OpcUa_CharA* psCommonName=OpcUa_Null;
	//OpcUa_CharA* trustedStorePath=OpcUa_Null;
	//OpcUa_CharA* issuerStorePath=OpcUa_Null;
	OpcUa_ByteString_Initialize(&tCertificate);
	OpcUa_Key_Initialize(&tPrivateKey);

	(void)sApplicationName;
	//

	OpcUa_String_Initialize(&aFullFileName);
	if (OpcUa_String_StrLen(&m_certificateStorePath)>0)
	{
		// extract the information from the certificate
		if (m_pCertificate)
		{
			uStatus = OpcUa_Certificate_GetInfo(
				m_pCertificate,
				OpcUa_Null,
				OpcUa_Null,
				&psCommonName,
				&sThumbprint,
				&lApplicationUri,
				OpcUa_Null,
				OpcUa_Null);

			if (uStatus != OpcUa_Good)
				OpcUa_Trace(OPCUA_TRACE_EXTRA_LEVEL_ERROR, "Could not extract information from application certificate.");
			else
			{
				// transfert de l'applicationUri
				OpcUa_String_AttachCopy(sApplicationUri, lApplicationUri);
				OpcUa_Free(lApplicationUri);

				if (uStatus != OpcUa_Good)
					OpcUa_Trace(OPCUA_TRACE_EXTRA_LEVEL_ERROR, "Could not initialize PKI provider.");
				else
				{
					// save information for later use.
					if (!m_pThumbprint)
						m_pThumbprint = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
					if (m_pThumbprint)
					{
						OpcUa_String_Initialize(m_pThumbprint);
						if (sThumbprint)
							OpcUa_String_AttachCopy(m_pThumbprint, sThumbprint);
					}
				}
			}
		}
		else
			uStatus = OpcUa_BadNoValidCertificates;

	}
	else
		uStatus=OpcUa_BadInvalidArgument;
	if (psCommonName)
		OpcUa_Free(psCommonName);
	if (sThumbprint)
		OpcUa_Free(sThumbprint);
	OpcUa_String_Clear(&aFullFileName);
	return uStatus;
}

//============================================================================
// CApplication::DiscoverEndpoints
// récupération des EndPoints du LDS
// Cette Methode renvoie un vecteur de CEndpointDescription 
//  qui contient les EndPoints du LDS.
//============================================================================
OpcUa_StatusCode CApplication::DiscoverEndpoints(const OpcUa_String& discoveryUrl,CEndpointDescriptionList* pEndpoints)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_RequestHeader tRequestHeader;
	OpcUa_String sDiscoveryUrl;
	OpcUa_String sLocaleIds;
	OpcUa_String sProfileUris;
	OpcUa_ResponseHeader tResponseHeader;
	OpcUa_Int32 nNoOfEndpoints = 0;
	OpcUa_EndpointDescription* pUAEndpoints = OpcUa_Null;//(OpcUa_EndpointDescription*)OpcUa_Alloc(sizeof(OpcUa_EndpointDescription));
	//if (pUAEndpoints)
	{
		CChannel channel(this);
		OpcUa_RequestHeader_Initialize(&tRequestHeader);
		tRequestHeader.RequestHandle = 1;
		OpcUa_String_Initialize(&sDiscoveryUrl);
		OpcUa_String_Initialize(&sLocaleIds);
		OpcUa_String_Initialize(&sProfileUris);
		OpcUa_ResponseHeader_Initialize(&tResponseHeader);
		OpcUa_MessageSecurityMode eSecurityMode = OpcUa_MessageSecurityMode_None;
		OpcUa_String szSecurityPolicy;
		OpcUa_String_Initialize(&szSecurityPolicy);
		// connect to the server.
		uStatus=channel.Connect(discoveryUrl,eSecurityMode,szSecurityPolicy);
		if (uStatus==OpcUa_Good)
		{
			// Just for testing behavior of LDS

			OpcUa_String_AttachCopy(&sLocaleIds,"fr-FR");
			// get the endpoints.
			tRequestHeader.TimeoutHint = 15000;// UTILS_DEFAULT_TIMEOUT; We will use a 15 sec timeout for the discovery. This is enough because the LDS is a local server
			tRequestHeader.Timestamp   = OpcUa_DateTime_UtcNow();

			// need to wrap the DiscoveryUrl with a OpcUa_String structure.
			OpcUa_String_CopyTo(&discoveryUrl, &sDiscoveryUrl);

			uStatus = OpcUa_ClientApi_GetEndpoints(  
				channel.GetInternalHandle(), 
				&tRequestHeader,
				&sDiscoveryUrl,
				1,
				&sLocaleIds, 
				0,
				&sProfileUris, 
				&tResponseHeader,
				&nNoOfEndpoints,
				&pUAEndpoints);

			if (uStatus==OpcUa_Good)
			{
				// copy enpoints to a vector.
				if (pUAEndpoints != OpcUa_Null)
				{
					CEndpointDescriptionList aEndpoints;
					for (OpcUa_Int32 ii = 0; ii < nNoOfEndpoints; ii++)
					{ 
						CEndpointDescription* pEndpointDescription=new CEndpointDescription(&pUAEndpoints[ii]);
						if (pEndpointDescription)
							pEndpoints->push_back(pEndpointDescription);;
					} 
				}
			}
			else
				OpcUa_Trace(OPCUA_TRACE_EXTRA_LEVEL_ERROR,"DiscoverEndpoints>OpcUa_ClientApi_GetEndpoints from LDS failed 0x%05x\n",uStatus);
			// clean up.
			OpcUa_RequestHeader_Clear(&tRequestHeader);
			OpcUa_String_Clear(&sDiscoveryUrl);
			OpcUa_String_Clear(&sLocaleIds);
			OpcUa_String_Clear(&sProfileUris);
			OpcUa_ResponseHeader_Clear(&tResponseHeader);
			// on va desallouer le tableau de OpcUa_EndpointDescription

			if (pUAEndpoints != OpcUa_Null)
			{
				for (OpcUa_Int32 ii = 0; ii < nNoOfEndpoints; ii++)
				{
					OpcUa_EndpointDescription_Clear(&pUAEndpoints[ii]);
				}
			}
			// disconnect. No need the disconnect will be made in the destructor of CChannel object
		}
		else
			OpcUa_Trace(OPCUA_TRACE_EXTRA_LEVEL_ERROR,"DiscoverEndpoints>Channel connect to LDS failed 0x%05x\n",uStatus);
		if (pUAEndpoints)
		{
			OpcUa_EndpointDescription_Clear(pUAEndpoints);
			OpcUa_Free(pUAEndpoints);
		}
	}
	return uStatus;
}

//============================================================================
// CApplication::TrustCertificate
//============================================================================
OpcUa_StatusCode CApplication::TrustCertificate(OpcUa_ByteString* pCertificate)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;

	// sauvegarde du certificat pCertificate dans le magasin m_certificateStorePath de l'application qui utilise l'OpenOpcUaSharedLib
	uStatus = OpcUa_Certificate_SavePublicKeyInStore(
		OpcUa_String_GetRawString(&m_certificateStorePath),
		pCertificate,
		NULL);

	if (uStatus!=OpcUa_Good)
		OpcUa_Trace(OPCUA_TRACE_EXTRA_LEVEL_ERROR,"Could not add certificate to the application's trusted store\n");
	return uStatus;
}

OpcUa_StatusCode CApplication::CreateCertificate()
{
	OpcUa_StatusCode	uStatus = OpcUa_Good;
	OpcUa_Key			tPrivateKey;
	OpcUa_LocalizedText aAppName;
	OpcUa_String		certificateStorePath;
	OpcUa_String		sApplicationUri;
	OpcUa_ByteString	tCertificate;

	OpcUa_LocalizedText_Initialize(&aAppName);
	OpcUa_LocalizedText_CopyTo(m_pApplicationName, &aAppName);
	OpcUa_String_Initialize(&certificateStorePath);
	OpcUa_String_CopyTo(&m_certificateStorePath, &certificateStorePath);
	// create 2 new self-signed certificate. One pfx in \private and one der in \certs
	if (!m_pPrivateKey)
	{
		OpcUa_ByteString_Initialize(&tCertificate);
		OpcUa_Key_Initialize(&tPrivateKey);
		uStatus = OpcUa_Certificate_Create(
			   OpcUa_String_GetRawString(&certificateStorePath),
				(OpcUa_StringA)OpcUa_String_GetRawString(&(aAppName.Text)),
				(OpcUa_StringA)OpcUa_String_GetRawString(&sApplicationUri),
				OpcUa_Null,
				OpcUa_Null,
				0,
				OpcUa_Null,
				0,
				1024,
				60,// validité du certificat en mois
				OpcUa_False,
				OpcUa_Crypto_Encoding_PKCS12,
				OpcUa_Null, // The DER encoded issuer certificate - OpcUa_ByteString
				OpcUa_Null, // The issuer's private key - OpcUa_Key
				OpcUa_Null, // Password used to protect the key file - OpcUa_StringA
				&tCertificate,
				OpcUa_Null,
				&tPrivateKey,
				OpcUa_Null);
		if (uStatus!=OpcUa_Good)
			OpcUa_Trace(OPCUA_TRACE_EXTRA_LEVEL_ERROR,"CApplication::CreateCertificate>Could not create self signed certificates (pfx and der) uStatus=0x%05x\n",uStatus);
		else
		{
			SetCertificate(&tCertificate);
			OpcUa_ByteString_Clear(&tCertificate);
			SetPrivateKey(tPrivateKey);
		}
	}
	else
	{
		// We have the privateKey from the PFX
		// So we can create the DER
		OpcUa_ByteString* pCertificate=GetCertificate();
		if (pCertificate)
		{
			OpcUa_StringA a_pCertificateFilePath=OpcUa_Null;
			uStatus = OpcUa_Certificate_SavePublicKeyInStore(
				OpcUa_String_GetRawString(&certificateStorePath),
				pCertificate,
				&a_pCertificateFilePath);
			if (a_pCertificateFilePath)
				OpcUa_Free(a_pCertificateFilePath);
		}
		else
			uStatus=OpcUa_BadInvalidArgument;
	}
	OpcUa_LocalizedText_Clear(&aAppName);
	OpcUa_String_Clear(&certificateStorePath);
	return uStatus;
}

// Returns the CApplication instance certificate.
OpcUa_ByteString* CApplication::GetCertificate()
{
	return m_pCertificate;
}
void  CApplication::SetCertificate(OpcUa_ByteString* pCertificate)
{
	if (pCertificate)
	{
		if (m_pCertificate)
			OpcUa_ByteString_Clear(m_pCertificate);
		OpcUa_ByteString_CopyTo(pCertificate, m_pCertificate);

	}
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Reject certificate. 
/// 			This mean to  copy certificate (pCertificate) in the rejected folder</summary>
///
/// <remarks>	Michel, 11/02/2016. </remarks>
///
/// <param name="pCertificate">	[in,out] If non-null, the certificate. </param>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode CApplication::RejectCertificate(OpcUa_ByteString* pCertificate)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_Int32 iResult;
	BIO* pPublicKeyFile = OpcUa_Null;
	if (pCertificate)
	{
		OpcUa_CharA* sCommonName = OpcUa_Null;
		uStatus = OpcUa_Certificate_GetCommonName(pCertificate, &sCommonName);
		if (uStatus == OpcUa_Good)
		{
			OpcUa_CharA* sThumbprint = OpcUa_Null;
			uStatus = OpcUa_Certificate_GetThumbprint(pCertificate, &sThumbprint);
			if (uStatus == OpcUa_Good)
			{
				// save the received certificate in the rejected folder
				//
				OpcUa_String certificateStorePath = GetCertificateStorePath();
				///////////////
				// build file path.
				std::string filePath;
				//char* pPos = OpcUa_Null;
				filePath += OpcUa_String_GetRawString(&certificateStorePath);
				filePath += "\\rejected\\";
				filePath += sCommonName;
				filePath += " [";
				filePath += sThumbprint;
				filePath += "]";
				// OpcUa_Crypto_Encoding_DER
				filePath += ".der";
				///////////////////////////////////////////////////////////
				pPublicKeyFile = BIO_new_file((const char*)filePath.c_str(), "wb");
				if (pPublicKeyFile)
				{
					iResult = BIO_write(pPublicKeyFile, pCertificate->Data, pCertificate->Length);

					if (iResult)
						OpcUa_Good;
					else
						uStatus = OpcUa_BadEncodingError;					
					BIO_free(pPublicKeyFile);
				}
				else
					uStatus = OpcUa_BadInvalidArgument;
			}
			if (sThumbprint)
				OpcUa_Free(sThumbprint);
		}
		if (sCommonName)
			OpcUa_Free(sCommonName);
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}
// Returns the CApplication instance certificate's private key.
OpcUa_Key* CApplication::GetPrivateKey()
{
	return m_pPrivateKey;
}
void CApplication::SetPrivateKey(OpcUa_Key aKey)
{
	if (m_pPrivateKey)
	{
		OpcUa_Key_Clear(m_pPrivateKey);
		OpcUa_Free(m_pPrivateKey);
	}
	m_pPrivateKey=(OpcUa_Key*)OpcUa_Alloc(sizeof(OpcUa_Key));
	if (m_pPrivateKey)
	{
		OpcUa_Key_Initialize(m_pPrivateKey);
		m_pPrivateKey->fpClearHandle=aKey.fpClearHandle;
		m_pPrivateKey->Type=aKey.Type;
		OpcUa_ByteString_CopyTo(&(aKey.Key),&(m_pPrivateKey->Key));
	}
}
// Returns the configuration for the PKI provider used by the CApplication.
OpcUa_CertificateStoreConfiguration* CApplication::GetPkiConfig()
{
	return m_pPkiConfig;
}
// Returns the PKI provider used by the CApplication.
OpcUa_PKIProvider* CApplication::GetPkiProvider()
{
	return m_pPkiProvider;
}
// Returns the type table used by the CApplication.
OpcUa_EncodeableTypeTable* CApplication::GetTypeTable()
{
	return &m_tTypeTable;
}
//CApplicationDescriptionList* CApplication::GetApplicationDescriptions() 
//{
//	return m_pApplicationDescriptions;
//}
void CApplication::SetApplicationName(OpcUa_LocalizedText* Val)
{
	if (!m_pApplicationName)
		m_pApplicationName=(OpcUa_LocalizedText*)OpcUa_Alloc(sizeof(OpcUa_LocalizedText));
	else
		OpcUa_LocalizedText_Clear(m_pApplicationName);
	OpcUa_LocalizedText_Initialize(m_pApplicationName);
	OpcUa_LocalizedText_CopyTo(Val,m_pApplicationName);
}
OpcUa_LocalizedText* CApplication::GetApplicationName() 
{
	return m_pApplicationName;
}
// Returns the certificate store path.
OpcUa_String CApplication::GetCertificateStorePath()
{
	return m_certificateStorePath;
}
// Sets the certificate store path.
void CApplication::SetCertificateStorePath(OpcUa_String NewCertificateStorePath)
{
	OpcUa_String certificateStorePath;
	OpcUa_String CertFolder;
	OpcUa_String PrivateFolder;
	OpcUa_String AuthFolder;
	OpcUa_String RejectedFolder;

	OpcUa_String_Initialize(&certificateStorePath);
	OpcUa_String_Initialize(&CertFolder);
	OpcUa_String_Initialize(&PrivateFolder);
	OpcUa_String_Initialize(&AuthFolder);
	OpcUa_String_Initialize(&RejectedFolder);
#ifndef _GNUC_
	DWORD dwError;
#endif
#ifndef _GNUC_
	OpcUa_String_AttachCopy(&CertFolder,"\\certs");
	OpcUa_String_AttachCopy(&PrivateFolder,"\\private");
	OpcUa_String_AttachCopy(&AuthFolder,"\\auth");
	OpcUa_String_AttachCopy(&RejectedFolder,"\\rejected");	// Revoked location
#else
	OpcUa_String_AttachCopy(&CertFolder,"//certs");
	OpcUa_String_AttachCopy(&PrivateFolder,"//private");
	OpcUa_String_AttachCopy(&AuthFolder,"//auth");
	OpcUa_String_AttachCopy(&RejectedFolder,"//rejected");
#endif
	// sauvegarde du repertoire racine
	OpcUa_String_Clear(&m_certificateStorePath);
	OpcUa_String_StrnCpy(&m_certificateStorePath,&NewCertificateStorePath,OpcUa_String_StrLen(&NewCertificateStorePath));
#ifndef _GNUC_
	#ifdef _WIN32_WCE
	 OpcUa_CharW *pcs = NULL;
	 OpcUa_String_AtoW(OpcUa_String_GetRawString(&m_certificateStorePath), &pcs);
	 if (!CreateDirectory((LPCWSTR)pcs, NULL))
	  dwError=GetLastError();
	#else
	 if (!CreateDirectoryA(OpcUa_String_GetRawString(&m_certificateStorePath), NULL))
	   dwError=GetLastError();
	#endif
#else
	mkdir(OpcUa_String_GetRawString(&m_certificateStorePath), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
	// Repertoire racine
	OpcUa_String_Clear(&certificateStorePath);
	OpcUa_String_StrnCpy(&certificateStorePath,&NewCertificateStorePath,OpcUa_String_StrLen(&NewCertificateStorePath));
	// ajout du nouveau sous-repertoire
	OpcUa_String_StrnCat(&certificateStorePath,&CertFolder,OpcUa_String_StrLen(&CertFolder));
#ifndef _GNUC_
	#ifdef _WIN32_WCE
	 pcs = NULL;
	 OpcUa_String_AtoW(OpcUa_String_GetRawString(&m_certificateStorePath), &pcs);
	 if (!CreateDirectory((LPCWSTR)pcs, NULL))
	  dwError=GetLastError();
	 if (pcs)
		OpcUa_Free(pcs);
	#else
	 if (!CreateDirectoryA(OpcUa_String_GetRawString(&certificateStorePath), NULL))
	   dwError=GetLastError();
	#endif
#else
	mkdir(OpcUa_String_GetRawString(&certificateStorePath), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
		// Repertoire racine
	OpcUa_String_Clear(&certificateStorePath);
	OpcUa_String_StrnCpy(&certificateStorePath,&NewCertificateStorePath,OpcUa_String_StrLen(&NewCertificateStorePath));
	// ajout du nouveau sous-repertoire cert
	OpcUa_String_StrnCat(&certificateStorePath,&CertFolder,OpcUa_String_StrLen(&CertFolder));
#ifndef _GNUC_
	#ifdef _WIN32_WCE
	 pcs = NULL;
	 OpcUa_String_AtoW(OpcUa_String_GetRawString(&m_certificateStorePath), &pcs);
	 if (!CreateDirectory((LPCWSTR)pcs, NULL))
	  dwError=GetLastError();
	 if (pcs)
		 OpcUa_Free(pcs);
	#else
	 if (!CreateDirectoryA(OpcUa_String_GetRawString(&certificateStorePath), NULL))
	   dwError=GetLastError();
	#endif
#else
	mkdir(OpcUa_String_GetRawString(&certificateStorePath), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
		// Repertoire racine
	OpcUa_String_Clear(&certificateStorePath);
	OpcUa_String_StrnCpy(&certificateStorePath,&NewCertificateStorePath,OpcUa_String_StrLen(&NewCertificateStorePath));
	// ajout du nouveau sous-repertoire private
	OpcUa_String_StrnCat(&certificateStorePath,&PrivateFolder,OpcUa_String_StrLen(&PrivateFolder));
#ifndef _GNUC_
	#ifdef _WIN32_WCE
	 pcs = NULL;
	 OpcUa_String_AtoW(OpcUa_String_GetRawString(&m_certificateStorePath), &pcs);
	 if (!CreateDirectory((LPCWSTR)pcs, NULL))
	  dwError=GetLastError();
	 if (pcs)
		 OpcUa_Free(pcs);
	#else
	 if (!CreateDirectoryA(OpcUa_String_GetRawString(&certificateStorePath), NULL))
	   dwError=GetLastError();
	#endif
#else
	mkdir(OpcUa_String_GetRawString(&certificateStorePath), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
		// Repertoire racine
	OpcUa_String_Clear(&certificateStorePath);
	OpcUa_String_StrnCpy(&certificateStorePath,&NewCertificateStorePath,OpcUa_String_StrLen(&NewCertificateStorePath));
	// ajout du nouveau sous-repertoire auth
	OpcUa_String_StrnCat(&certificateStorePath,&AuthFolder,OpcUa_String_StrLen(&AuthFolder));
#ifndef _GNUC_
	#ifdef _WIN32_WCE
	 pcs = NULL;
	 OpcUa_String_AtoW(OpcUa_String_GetRawString(&m_certificateStorePath), &pcs);
	 if (!CreateDirectory((LPCWSTR)pcs, NULL))
	  dwError=GetLastError();
	 if (pcs)
		 OpcUa_Free(pcs);
	#else
	 if (!CreateDirectoryA(OpcUa_String_GetRawString(&certificateStorePath), NULL))
	   dwError=GetLastError();
	#endif
#else
	mkdir(OpcUa_String_GetRawString(&certificateStorePath), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
		// Repertoire racine
	OpcUa_String_Clear(&certificateStorePath);
	OpcUa_String_StrnCpy(&certificateStorePath,&NewCertificateStorePath,OpcUa_String_StrLen(&NewCertificateStorePath));
	// ajout du nouveau sous-repertoire
	OpcUa_String_StrnCat(&certificateStorePath,&RejectedFolder,OpcUa_String_StrLen(&RejectedFolder));
#ifndef _GNUC_
	#ifdef _WIN32_WCE
	 pcs = NULL;
	 OpcUa_String_AtoW(OpcUa_String_GetRawString(&m_certificateStorePath), &pcs);
	 if (!CreateDirectory((LPCWSTR)pcs, NULL))
	  dwError=GetLastError();
	 if (pcs)
		 OpcUa_Free(pcs);
	#else
	 if (!CreateDirectoryA(OpcUa_String_GetRawString(&certificateStorePath), NULL))
	   dwError=GetLastError();
	#endif
#else
	mkdir(OpcUa_String_GetRawString(&certificateStorePath), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
	// Let create the PkiProvide for application certificate
	OpcUa_StatusCode uStatus=OpcUa_Certificate_CreatePkiProvider(OpcUa_String_GetRawString(&NewCertificateStorePath), 
		OpcUa_String_GetRawString(&CertFolder), 
		OpcUa_String_GetRawString(&RejectedFolder),
		OpcUa_String_GetRawString(&AuthFolder), 
		OpcUa_String_GetRawString(&RejectedFolder),
		&m_pPkiConfig,
		&m_pPkiProvider);
	if (uStatus != OpcUa_Good)
		OpcUa_Trace(OPCUA_TRACE_SERVER_ERROR, "Pki error> uStatus= 0x%05x cannot create PkiProvider for X509 Application certificate %s", uStatus, OpcUa_String_GetRawString(&NewCertificateStorePath));
	m_pUserX509PkiConfig->pvOverride = m_pPkiConfig->pvOverride;
	m_pUserX509PkiConfig->strIssuerCertificateStoreLocation = m_pPkiConfig->strIssuerCertificateStoreLocation;
	m_pUserX509PkiConfig ->strPkiType = m_pPkiConfig ->strPkiType ;
	m_pUserX509PkiConfig ->strRevokedCertificateListLocation = m_pPkiConfig ->strRevokedCertificateListLocation ;
	m_pUserX509PkiConfig ->strRevokedIssuerCertificateListLocation = m_pPkiConfig ->strRevokedIssuerCertificateListLocation ;
	m_pUserX509PkiConfig ->strTrustedCertificateListLocation = m_pPkiConfig ->strTrustedCertificateListLocation ;
	m_pUserX509PkiConfig ->uFlags = m_pPkiConfig ->uFlags ;

	// Let create the PkiProvide for user X509 certificate
	uStatus=OpcUa_Certificate_CreatePkiProvider(OpcUa_String_GetRawString(&NewCertificateStorePath), 
		OpcUa_String_GetRawString(&CertFolder), 
		OpcUa_String_GetRawString(&RejectedFolder),
		OpcUa_String_GetRawString(&AuthFolder), 
		OpcUa_String_GetRawString(&RejectedFolder),
		&m_pUserX509PkiConfig,
		&m_pUserX509PkiProvider);
	if (uStatus != OpcUa_Good)
		OpcUa_Trace(OPCUA_TRACE_SERVER_ERROR, "Pki error> uStatus= 0x%05x cannot create PkiProvider for X509 user certificate %s", uStatus, OpcUa_String_GetRawString(&NewCertificateStorePath));
	OpcUa_String_Clear(&certificateStorePath);
	OpcUa_String_Clear(&CertFolder);
	OpcUa_String_Clear(&PrivateFolder);
	OpcUa_String_Clear(&AuthFolder);
	OpcUa_String_Clear(&RejectedFolder);
}
OpcUa_UInt32 CApplication::GetTraceLevel() 
{ 
	return m_tConfiguration.uProxyStub_Trace_Level; 
}
void CApplication::SetTraceLevel(OpcUa_UInt32 iTraceLevel)
{ 
	m_tConfiguration.uProxyStub_Trace_Level = iTraceLevel; 
	OpcUa_Trace_SetTraceLevel(iTraceLevel);
}
OpcUa_UInt32 CApplication::GetTraceOutput() 
{ 
	return m_tConfiguration.uProxyStub_Trace_Output; 
}
void CApplication::SetTraceOutput(OpcUa_UInt32 iTraceOutput)
{ 
	m_tConfiguration.uProxyStub_Trace_Output = iTraceOutput; 
}
OpcUa_Boolean CApplication::IsPfxDerValide() 
{ 
	return m_bPfxDerValide; 
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Get x coordinate 509 user pki provider. </summary>
///
/// <remarks>	Michel, 05/06/2016. </remarks>
///
/// <returns>	null if it fails, else the x coordinate 509 user pki provider. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_PKIProvider* CApplication::GetX509UserPkiProvider()
{
	
	return m_pUserX509PkiProvider;
}

