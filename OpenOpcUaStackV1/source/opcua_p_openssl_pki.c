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
#include <opcua_p_memory.h>

#include <opcua_trace.h>
#if OPCUA_REQUIRE_OPENSSL

#ifndef _CRT_SECURE_NO_DEPRECATE
	#define _CRT_SECURE_NO_DEPRECATE
#endif /* _CRT_SECURE_NO_DEPRECATE */

#ifdef WIN32
#pragma warning( disable : 4985 )
#endif

/* System Headers */
#include <openssl/pem.h>
#include <openssl/x509_vfy.h>
#include <openssl/bio.h>
#include <openssl/err.h>
/* Stack Headers */
#include <opcua_p_string.h>
#include "opcua_string.h"
/* own headers */
#include <opcua_p_openssl_pki.h>

/* WORKAROUND */
#include <opcua_p_os.h>
#include <string.h>
#include <stdio.h>
#include <opcua_p_utilities.h>


/* Prototypes */
//OpcUa_Void OpcUa_P_ByteString_Initialize(OpcUa_ByteString* a_pValue);
//OpcUa_Void OpcUa_P_ByteString_Clear(OpcUa_ByteString* a_pValue);

#ifdef _MSC_VER
#pragma warning(disable:4748) /* suppress /GS can not protect parameters and local variables from local buffer overrun because optimizations are disabled in function */
#endif /* _MSC_VER */

/*============================================================================
 * path utility
 *===========================================================================*/
OpcUa_StatusCode OpcUa_P_OpenSSL_BuildFullPath( /*  in */ char*         a_pPath,
												/*  in */ char*         a_pFileName,
												/*  in */ unsigned int  a_uiFullPathBufferLength,
												/* out */ char*         a_pFullPath)
{
	unsigned int uiPathLength = 0;
	unsigned int uiFileLength = 0;

	OpcUa_ReturnErrorIfArgumentNull(a_pPath);
	OpcUa_ReturnErrorIfArgumentNull(a_pFileName);
	OpcUa_ReturnErrorIfArgumentNull(a_pFullPath);

	uiPathLength = (unsigned int)strlen((const char*)a_pPath);
	uiFileLength = (unsigned int)strlen(a_pFileName);

	if((uiPathLength + uiFileLength + 3) > a_uiFullPathBufferLength)
	{
		return OpcUa_BadInvalidArgument;
	}

#ifdef WIN32
#if OPCUA_USE_SAFE_FUNCTIONS
	strncpy_s(a_pFullPath, a_uiFullPathBufferLength-1, (const char*)a_pPath, uiPathLength + 1);
	strncat_s(a_pFullPath, a_uiFullPathBufferLength-1, "\\", 2);
	strncat_s(a_pFullPath, a_uiFullPathBufferLength-1, a_pFileName, uiFileLength);
#else /* OPCUA_USE_SAFE_FUNCTIONS */
	strncpy(a_pFullPath, (const char*)a_pPath, uiPathLength + 1);
	strncat(a_pFullPath, "\\", 2);
	strncat(a_pFullPath, a_pFileName, uiFileLength);
#endif /* OPCUA_USE_SAFE_FUNCTIONS */
#endif /* WIN32 */
	
#ifdef _GNUC_
	 strncpy(a_pFullPath, (const char*)a_pPath, uiPathLength + 1);  
	 strncat(a_pFullPath, "/", 1);
	 strncat(a_pFullPath, a_pFileName, uiFileLength);
#endif


	return OpcUa_Good;
}

/*============================================================================
 * verify_callback
 *===========================================================================*/
OpcUa_Int OpcUa_P_OpenSSL_CertificateStore_Verify_Callback(int a_ok, X509_STORE_CTX* a_pStore)
{
	OpcUa_Int iRes = a_ok;
	OpcUa_ReferenceParameter(a_pStore);

	if(a_ok == 0)
	{
		/* certificate not ok */
		char    buf[256];
		X509*   err_cert    = NULL;
		int     err         = 0;
		int     depth       = 0;

		err_cert = X509_STORE_CTX_get_current_cert(a_pStore);
		err      = X509_STORE_CTX_get_error(a_pStore);
		depth    = X509_STORE_CTX_get_error_depth(a_pStore);

		/* This serious error is generated while looking for CAs in a store. It must be ignored. */
		if (err != X509_V_ERR_SUBJECT_ISSUER_MISMATCH)
		{


			X509_NAME_oneline(X509_get_subject_name(err_cert), buf, 256);
			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "\nverify error:\n\tnum=%d:%s\n\tdepth=%d\n\t%s\n", err, X509_verify_cert_error_string(err), depth, buf);

			//if (err == X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT)
			if (err != X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT)
			{
				X509_NAME_oneline(X509_get_issuer_name(a_pStore->current_cert), buf, 256);
				OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "Issued certificate: \tissuer=%s\n", buf);
			}

		}
	}

	return iRes;
}

/* stores the locations of the trusted certificate and the untrusted CA certificates. */
typedef struct OpcUa_P_OpenSSL_CertificateThumbprint
{
	OpcUa_Byte Data[SHA_DIGEST_LENGTH];
}
OpcUa_P_OpenSSL_CertificateThumbprint;

/* stores the merged trust list and a list of explicitly trusted certificates. */
typedef struct OpcUa_P_OpenSSL_CertificateStore
{
	X509_STORE* MergedTrustList;
	OpcUa_P_OpenSSL_CertificateThumbprint* ExplicitTrustList;
	OpcUa_UInt32 ExplicitTrustListCount;
	OpcUa_UInt32 ExplicitTrustListCapacity;
}
OpcUa_P_OpenSSL_CertificateStore;

/* allocates a certificate store handle. */
static OpcUa_P_OpenSSL_CertificateStore* OpcUa_P_OpenSSL_CertificateStore_Alloc()
{
	OpcUa_P_OpenSSL_CertificateStore* pCertificateStore = OpcUa_Null;
	pCertificateStore = (OpcUa_P_OpenSSL_CertificateStore*)OpcUa_P_Memory_Alloc(sizeof(OpcUa_P_OpenSSL_CertificateStore));

	if (pCertificateStore != OpcUa_Null)
	{
		OpcUa_MemSet(pCertificateStore, 0, sizeof(OpcUa_P_OpenSSL_CertificateStore));
	}

	return pCertificateStore;
}

/* frees a certificate store handle. */
static void OpcUa_P_OpenSSL_CertificateStore_Free(OpcUa_P_OpenSSL_CertificateStore** a_ppCertificateStore)
{
	OpcUa_P_OpenSSL_CertificateStore* pCertificateStore = OpcUa_Null;

	if (a_ppCertificateStore != OpcUa_Null)
	{
		pCertificateStore = *a_ppCertificateStore;

		if (pCertificateStore->MergedTrustList != OpcUa_Null)
		{
			X509_STORE_free((X509_STORE*)pCertificateStore->MergedTrustList);
			pCertificateStore->MergedTrustList = OpcUa_Null;
		}

		if(pCertificateStore->MergedTrustList != OpcUa_Null)
		{
			OpcUa_P_Memory_Free(pCertificateStore->ExplicitTrustList);
			pCertificateStore->ExplicitTrustList = OpcUa_Null;
		}

		OpcUa_P_Memory_Free(pCertificateStore);
		*a_ppCertificateStore = OpcUa_Null;
	}
}

/* add a certificate to a store. */
static OpcUa_StatusCode OpcUa_P_OpenSSL_CertificateStore_AddCertificate(
	OpcUa_P_OpenSSL_CertificateStore* a_pStore,
	OpcUa_ByteString* a_pCertificate,
	OpcUa_Boolean a_bExplicitlyTrusted)
{
	OpcUa_Byte* pPosition = OpcUa_Null;
	X509* pX509Certificate = OpcUa_Null;

	OpcUa_StatusCode uStatus = OpcUa_Good;

	if (a_pStore)
	{
		if (a_pCertificate)
		{
			/* convert public key to X509 structure. */
			pPosition = a_pCertificate->Data;
			pX509Certificate = d2i_X509((X509**)OpcUa_Null, (const unsigned char**)&pPosition, a_pCertificate->Length);
			if (!pX509Certificate)
				uStatus = OpcUa_Bad;
			else
			{
				/* add to store */
				X509_STORE_add_cert(a_pStore->MergedTrustList, pX509Certificate);

				/* release certificate */
				X509_free(pX509Certificate);

				/* add to trustlist */
				if (a_bExplicitlyTrusted)
				{
					if (a_pStore->ExplicitTrustListCount == a_pStore->ExplicitTrustListCapacity)
					{
						OpcUa_P_OpenSSL_CertificateThumbprint* pTrustList = OpcUa_Null;
						a_pStore->ExplicitTrustListCapacity += 10;
						pTrustList = OpcUa_P_Memory_ReAlloc(a_pStore->ExplicitTrustList, sizeof(OpcUa_P_OpenSSL_CertificateThumbprint)*a_pStore->ExplicitTrustListCapacity);
						if (pTrustList)
							a_pStore->ExplicitTrustList = pTrustList;
						else
							uStatus = OpcUa_BadOutOfMemory;
					}
					if (uStatus==OpcUa_Good)
						SHA1(a_pCertificate->Data, a_pCertificate->Length, a_pStore->ExplicitTrustList[a_pStore->ExplicitTrustListCount++].Data);
				}
			}
		}
		else
			uStatus = OpcUa_BadInvalidArgument;
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}

/* checks if the certificate is explicitly trusted. */
static OpcUa_StatusCode OpcUa_P_OpenSSL_CertificateStore_IsExplicitlyTrusted(
	OpcUa_P_OpenSSL_CertificateStore* a_pStore,
	X509_STORE_CTX* a_pX509Context,
	X509* a_pX509Certificate,
	OpcUa_Boolean* a_pExplicitlyTrusted)
{
	X509* x = a_pX509Certificate;
	X509* xtmp = OpcUa_Null;
	int iResult = 0;
	OpcUa_UInt32 jj = 0;
	OpcUa_ByteString tBuffer;
	OpcUa_Byte* pPosition = OpcUa_Null;
	OpcUa_P_OpenSSL_CertificateThumbprint tThumbprint;

OpcUa_InitializeStatus(OpcUa_Module_P_OpenSSL, "CertificateStore_IsExplicitlyTrusted");

	OpcUa_ReturnErrorIfArgumentNull(a_pStore);
	OpcUa_ReturnErrorIfArgumentNull(a_pX509Context);
	OpcUa_ReturnErrorIfArgumentNull(a_pX509Certificate);
	OpcUa_ReturnErrorIfArgumentNull(a_pExplicitlyTrusted);

	OpcUa_P_ByteString_Initialize(&tBuffer);

	*a_pExplicitlyTrusted = OpcUa_False;

	/* follow the trust chain. */
	while (!*a_pExplicitlyTrusted)
	{
		/* need to convert to DER encoded certificate. */
		int iLength = i2d_X509(x, NULL);

		if (iLength > tBuffer.Length)
		{
			tBuffer.Length = iLength;
			tBuffer.Data = OpcUa_P_Memory_ReAlloc(tBuffer.Data, iLength);
			OpcUa_GotoErrorIfAllocFailed(tBuffer.Data);
		}

		pPosition = tBuffer.Data;
		iResult = i2d_X509((X509*)x, &pPosition);

		if (iResult <= 0)
		{
			OpcUa_GotoErrorWithStatus(OpcUa_BadEncodingError);
		}

		/* compute the hash */
		SHA1(tBuffer.Data, iLength, tThumbprint.Data);

		/* check for thumbprint in explicit trust list. */
		for (jj = 0; jj < a_pStore->ExplicitTrustListCount; jj++)
		{
			if (OpcUa_MemCmp(a_pStore->ExplicitTrustList[jj].Data, tThumbprint.Data, SHA_DIGEST_LENGTH) == 0)
			{
				*a_pExplicitlyTrusted = OpcUa_True;
				break;
			}
		}

		if (*a_pExplicitlyTrusted)
		{
			break;
		}

		/* end of chain if self signed. */
		if (a_pX509Context->check_issued(a_pX509Context, x, x))
		{
			break;
		}

		/* look in the store for the issuer. */
		iResult = a_pX509Context->get_issuer(&xtmp, a_pX509Context, x);

		if (iResult == 0)
		{
			break;
		}

		/* oops - unexpected error */
		if (iResult < 0)
		{
			OpcUa_GotoErrorWithStatus(OpcUa_Bad);
		}

		/* goto next link in chain. */
		x = xtmp;
		X509_free(xtmp);
	}
	//if(x)
	//{
	//	X509_free(x);
	//	x=OpcUa_Null;
	//}

	OpcUa_P_ByteString_Clear(&tBuffer);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
	if(x)
	{
		X509_free(x);
		x=OpcUa_Null;
	}

	OpcUa_P_ByteString_Clear(&tBuffer);

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_P_OpenSSL_ReadFile
 *===========================================================================*/
static OpcUa_StatusCode OpcUa_P_OpenSSL_ReadFile(
	OpcUa_StringA     a_sFilePath,
	OpcUa_ByteString* a_pBuffer)
{
	FILE* pFile = OpcUa_Null;
	BYTE* pBuffer = OpcUa_Null;
	int iResult = 0;
#ifdef WIN32
	fpos_t iLength = 0;
#endif

//#ifdef _GNUC_
//    // Sous GNU/Linux ne peux pas être initialisé
//    fpos_t iLength;
//#endif
	BYTE* pPosition = OpcUa_Null;

#ifdef _GNUC_
	OpcUa_UInt32 iSize = 0;
#endif

OpcUa_InitializeStatus(OpcUa_Module_P_OpenSSL, "ReadFile");

	OpcUa_ReturnErrorIfArgumentNull(a_sFilePath);
	OpcUa_ReturnErrorIfArgumentNull(a_pBuffer);

	OpcUa_P_ByteString_Initialize(a_pBuffer);

	/* read the file. */
#ifdef WIN32
	iResult = fopen_s(&pFile, (const char*)a_sFilePath, "rb");
	if (iResult == 0)
	{
		/* get the length. */
		iResult = fseek(pFile, 0, SEEK_END);

		if (iResult == 0)
		{
			iResult = fgetpos(pFile, &iLength);

			if (iResult != 0)
			{
				OpcUa_GotoErrorWithStatus(OpcUa_BadDecodingError);
			}

			fseek(pFile, 0, SEEK_SET);
		}

		/* allocate buffer. */
		pBuffer = (BYTE*)OpcUa_P_Memory_Alloc((OpcUa_UInt32)iLength);
		memset(pBuffer, 0, (size_t)iLength);
		/* read blocks. */
		pPosition = pBuffer;
		iResult = 1; // just for init purpose
		while (iResult!=0)
		{
			iResult = (int)fread(pPosition, 1, (size_t)(iLength - (pPosition - pBuffer)), pFile);
			if (iResult <= 0)
				break;
			pPosition += iResult;
		}
		//
		fclose(pFile);
		pFile = NULL;
		a_pBuffer->Data = pBuffer;
		a_pBuffer->Length = (OpcUa_Int32)(pPosition - pBuffer);
	}
	else
		uStatus=OpcUa_BadDecodingError;
#endif
	
#ifdef _GNUC_
	   pFile = fopen((const char*)a_sFilePath, "rb"); 
	   if (pFile)
	   {
		   /* get the length. */
		   iResult = fseek(pFile, 0, SEEK_END);
		   if (iResult == 0)
		   {
			   iSize = ftell(pFile);
			   fseek(pFile, 0, SEEK_SET);

			   /* allocate buffer. */
			   pBuffer = (BYTE*)OpcUa_P_Memory_Alloc((OpcUa_UInt32)iSize);
			   memset(pBuffer, 0, (size_t)iSize);
			   /* read blocks. */
			   pPosition = pBuffer;
			   while (pFile != NULL)
			   {
				   iResult = (int)fread(pPosition, 1, (size_t)(iSize - (pPosition - pBuffer)), pFile);
				   if (iResult <= 0)
					   break;
				   pPosition += iResult;
			   }
			   fclose(pFile);
			   pFile = NULL;

			   if (pPosition == pBuffer)
				   uStatus = OpcUa_BadDecodingError;
			   else
			   {
				   a_pBuffer->Data = pBuffer;
				   a_pBuffer->Length = (OpcUa_Int32)(pPosition - pBuffer);
			   }
		   }
		   else
		   {
			   uStatus = OpcUa_BadDecodingError;
			   fclose(pFile);
		   }
	   }
#endif



OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	if (pFile != OpcUa_Null)
	{
		fclose(pFile);
	}

	if (pBuffer != OpcUa_Null)
	{
		OpcUa_P_Memory_Free(pBuffer);
	}

	OpcUa_P_ByteString_Initialize(a_pBuffer);

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_P_OpenSSL_CertificateStore_PopulateStore
 *===========================================================================*/
static OpcUa_StatusCode OpcUa_P_OpenSSL_CertificateStore_PopulateStore(
	OpcUa_P_OpenSSL_CertificateStore* a_pStore,
	OpcUa_CharA* a_pStorePath,
	OpcUa_Boolean a_bIsExplicitlyTrusted)
{
	/* ToDo: THIS IS A WORKAROUND->Better solution has to be found */
	HANDLE hFind = INVALID_HANDLE_VALUE;

	char CertFile[MAX_PATH]; //
	DWORD dwError;
	int iLen;
	int i;
	char *pFilename;
	int length;
	OpcUa_ByteString tBuffer;
	char DirSpec[MAX_PATH]; /* Will contains the directory name and extension to search*/
	OpcUa_StatusCode uStatus=OpcUa_Good;

#ifndef _GNUC_
	WIN32_FIND_DATA FindFileData;
#endif
#ifdef _GNUC_
	struct dirent* FindFileData;	
#endif

#if defined (_WIN32_WCE) ||defined(WIN32)
	wchar_t *wDirSpec=OpcUa_Null;
#endif
	if (a_pStorePath)
	{
		ZeroMemory(DirSpec, MAX_PATH);


		iLen = (int)strlen(DirSpec);
		OpcUa_P_String_strncpy(DirSpec, MAX_PATH - 1, a_pStorePath, strlen(a_pStorePath));
		OpcUa_P_ByteString_Initialize(&tBuffer);

		/* ToDo: THIS IS A WORKAROUND->Better solution has to be found, since X509_LOOKUP_add_dir does not work properly */

		ZeroMemory(CertFile, MAX_PATH);

//#ifdef WIN32
//#ifdef UNICODE
//		if (a_pStorePath[iLen-1] == '\\')
//		{
//			OpcUa_P_String_strncat(wDirSpec, MAX_PATH-1, "*.der", MAX_PATH-1);
//		}
//		else
//		{
//			OpcUa_P_String_strncat(wDirSpec, MAX_PATH-1, "\\*.der", MAX_PATH-1);
//		}
//#else
//		if (a_pStorePath[iLen - 1] == '\\')
//		{
//			OpcUa_P_String_strncat(DirSpec, MAX_PATH - 1, "*.der", MAX_PATH - 1);
//		}
//		else
//		{
//			OpcUa_P_String_strncat(DirSpec, MAX_PATH - 1, "\\*.der", MAX_PATH - 1);
//		}
//#endif
//#endif
#if defined (_WIN32_WCE) ||defined(WIN32)
		if (a_pStorePath[iLen - 1] == '\\')
		{
			OpcUa_P_String_strncat(DirSpec, MAX_PATH - 1, "*.der", MAX_PATH - 1);
		}
		else
		{
			OpcUa_P_String_strncat(DirSpec, MAX_PATH - 1, "\\*.der", MAX_PATH - 1);
		}
#endif
#ifdef _GNUC_
		if (a_pStorePath[iLen-1] == '/')
		{
			OpcUa_P_String_strncat(DirSpec, MAX_PATH-1, "*.der", MAX_PATH-1);
		}
		else
		{
			OpcUa_P_String_strncat(DirSpec, MAX_PATH-1, "/*.der", MAX_PATH-1);
		}
#endif
		/* Search for the first file in the folder that fit the criteria (CertificateStore\certs\*.der) */
#ifdef _WIN32_WCE
		length = strlen(DirSpec) + 1;
		wDirSpec = (wchar_t*)calloc(sizeof(wchar_t) , length);
		if(wDirSpec)
			for(i=0; i<length; i++) wDirSpec[i] = (wchar_t) DirSpec[i];
		hFind = FindFirstFile(wDirSpec, &FindFileData);
		if (wDirSpec)
			free(wDirSpec);
#else
	#ifdef WIN32
		#ifdef UNICODE
				length = strlen(DirSpec) + 1;
				wDirSpec=(wchar_t*)calloc(sizeof(wchar_t),_MAX_PATH);
				if(wDirSpec)
					for(i=0; i<length; i++) wDirSpec[i] = (wchar_t) DirSpec[i];
				hFind = FindFirstFile(wDirSpec, &FindFileData);
				if (wDirSpec)
					free(wDirSpec);
		#else
				hFind = FindFirstFileA(DirSpec, &FindFileData);
		#endif
	#else
		#ifdef _GNUC_
				(void)length;
				(void)pFilename;
				(void)i;
				hFind = FindFirstFile(DirSpec, &FindFileData);
		#endif
	#endif
#endif

		if (hFind != INVALID_HANDLE_VALUE)
		{

#ifdef _WIN32_WCE
			length = wcslen(FindFileData.cFileName) + 1;
#else
	#ifdef WIN32
		#ifdef UNICODE
					length = wcslen(FindFileData.cFileName) + 1;
		#else
					length = strlen(FindFileData.cFileName) + 1;
		#endif	
	#endif
#endif
			/* BuildFullPath for the firstFile will then call to OpcUa_P_OpenSSL_BuildFullPath */
#if defined (WIN32) || defined (_WIN32_WCE)
			pFilename = (char*)calloc(sizeof(char), length + 1);
			if (pFilename)
			{
				for (i = 0; i < length; i++) pFilename[i] = (char)FindFileData.cFileName[i];
			}

			uStatus = OpcUa_P_OpenSSL_BuildFullPath(a_pStorePath, pFilename, MAX_PATH, CertFile);
			if (pFilename)
				free(pFilename);
#endif

#ifdef _GNUC_
			uStatus = OpcUa_P_OpenSSL_BuildFullPath(a_pStorePath, FindFileData->d_name, MAX_PATH, CertFile);
#endif
			if (uStatus == OpcUa_Good)
			{
				/* Extract The certificate key from the file */
				uStatus = OpcUa_P_OpenSSL_ReadFile(CertFile, &tBuffer);
				if (uStatus == OpcUa_Good)
				{
					/* Add the certificate in the OpenSSL CertificateStore */
					uStatus = OpcUa_P_OpenSSL_CertificateStore_AddCertificate(a_pStore, &tBuffer, a_bIsExplicitlyTrusted);
					if (uStatus == OpcUa_Good)
					{
						OpcUa_P_ByteString_Clear(&tBuffer);
						/* Search for the next file in the folder that fit the criteria (CertificateStore\certs\*.der) */
						while (FindNextFile(hFind, &FindFileData) != 0)
						{
							/* BuildFullPath for the buildFile will then call to OpcUa_P_OpenSSL_BuildFullPath */
#ifdef _WIN32_WCE
							length = wcslen(FindFileData.cFileName) + 1;
#else
	#ifdef WIN32
		#ifdef UNICODE
									length = wcslen(FindFileData.cFileName) + 1;
		#else
									length = strlen(FindFileData.cFileName) + 1;
		#endif	
	#endif
#endif
#if defined (WIN32) || defined (_WIN32_WCE)
							pFilename = (char*)calloc(sizeof(char), length + 1);
							if (pFilename)
								for (i = 0; i < length; i++) pFilename[i] = (char)FindFileData.cFileName[i];
							uStatus = OpcUa_P_OpenSSL_BuildFullPath(a_pStorePath, pFilename, MAX_PATH, CertFile);
							if (pFilename)
								free(pFilename);
#endif

#ifdef _GNUC_
							uStatus = OpcUa_P_OpenSSL_BuildFullPath(a_pStorePath, FindFileData->d_name, MAX_PATH, CertFile);
#endif
							if (uStatus == OpcUa_Good)
							{
								/* Extract The certificate key from the file */
								uStatus = OpcUa_P_OpenSSL_ReadFile(CertFile, &tBuffer);
								if (uStatus == OpcUa_Good)
								{
									/* Add the certificate in the OpenSSL CertificateStore */
									uStatus = OpcUa_P_OpenSSL_CertificateStore_AddCertificate(a_pStore, &tBuffer, a_bIsExplicitlyTrusted);
									OpcUa_P_ByteString_Clear(&tBuffer);
								}
								else
									OpcUa_P_ByteString_Clear(&tBuffer);
							}
							else
								OpcUa_P_ByteString_Clear(&tBuffer);
						}

						dwError = GetLastError();
						FindClose(hFind);
#ifdef WIN32
						if (dwError != ERROR_NO_MORE_FILES)
						{
							OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "unexpected error loading certificates! %d\n", dwError);
							uStatus=OpcUa_BadFileNotFound;
							OpcUa_P_ByteString_Clear(&tBuffer);
						}
#endif
					}
				}
				else
					OpcUa_P_ByteString_Clear(&tBuffer);
			}
			else
				OpcUa_P_ByteString_Clear(&tBuffer);
		}
	}
	else
		uStatus = OpcUa_BadInvalidArgument;

	return uStatus;
}


/*============================================================================
 * OpcUa_P_OpenSSL_CertificateStore_LoadCRLs
 *===========================================================================*/
static OpcUa_StatusCode OpcUa_P_OpenSSL_CertificateStore_LoadCRLs(
	X509_LOOKUP* pLookup,
	OpcUa_CharA* a_pCrlPath)
{
	HANDLE hFind = INVALID_HANDLE_VALUE;
	char DirSpec[MAX_PATH];
	char CertFile[MAX_PATH];
	DWORD dwError;
	int iLen;
#ifndef _GNUC_
	int i;
	char *pFilename;
	int length;
#endif

#ifdef _WIN32_WCE
	wchar_t *wDirSpec;
	WIN32_FIND_DATA FindFileData;
#else
	#ifdef WIN32
		#ifdef UNICODE
			wchar_t *wDirSpec;
			WIN32_FIND_DATA FindFileData;
		#else
			WIN32_FIND_DATAA FindFileData;
		#endif
	#endif
#endif

#ifdef _GNUC_
	struct dirent* FindFileData;
#endif

OpcUa_InitializeStatus(OpcUa_Module_P_OpenSSL, "CertificateStore_LoadCRLs");

	OpcUa_ReturnErrorIfArgumentNull(a_pCrlPath);

	OpcUa_P_String_strncpy(DirSpec, MAX_PATH-1, a_pCrlPath, MAX_PATH);

	iLen = (int)strlen(DirSpec);

//TODO : Correctif pour _GNUC_ : les chemin avec '/' pour GNU/_GNUC_ ou '\\' pour WIN32
#ifdef WIN32
	if (a_pCrlPath[iLen-1] == '\\')
	{
		OpcUa_P_String_strncat(DirSpec, MAX_PATH-1, "*.crl", MAX_PATH-1);
	}
	else
	{
		OpcUa_P_String_strncat(DirSpec, MAX_PATH-1, "\\*.crl", MAX_PATH-1);
	}
#endif

#ifdef _GNUC_
	if (a_pCrlPath[iLen-1] == '/')
	{
		OpcUa_P_String_strncat(DirSpec, MAX_PATH-1, "*.crl", MAX_PATH-1);
	}
	else
	{
		OpcUa_P_String_strncat(DirSpec, MAX_PATH-1, "/.crl", MAX_PATH-1);
	}
	hFind = FindFirstFile(DirSpec, &FindFileData);
#endif

#ifdef _WIN32_WCE
	length = strlen(DirSpec) + 1;
	wDirSpec = (wchar_t*)calloc(sizeof(wchar_t) , length);
	if(wDirSpec)
	{
		for(i=0; i<length; i++) wDirSpec[i] = (wchar_t) DirSpec[i];
	}
	hFind = FindFirstFile(wDirSpec, &FindFileData);
	if (wDirSpec)
	{
		free(wDirSpec);
	}
#else
	#ifdef WIN32
		#ifdef UNICODE
			length = strlen(DirSpec) + 1;
			wDirSpec = (wchar_t*)calloc(sizeof(wchar_t) , length);
			if(wDirSpec)
				for(i=0; i<length; i++) wDirSpec[i] = (wchar_t) DirSpec[i];
			hFind = FindFirstFile(wDirSpec, &FindFileData);
		#else
			hFind = FindFirstFileA(DirSpec, &FindFileData);
		#endif
	#endif
#endif


	if (hFind != INVALID_HANDLE_VALUE)
	{
#ifdef _WIN32_WCE
			length = wcslen(FindFileData.cFileName) + 1;
#else
		#ifdef WIN32
			#ifdef UNICODE
				length = wcslen(FindFileData.cFileName) + 1;
			#else
				length = strlen(FindFileData.cFileName) + 1;
			#endif
		#endif
#endif

#if defined (WIN32) || defined (_WIN32_WCE)
			pFilename = (char*)calloc(sizeof(char), length+1);
			if (pFilename)
			{
				for(i=0; i<length; i++) pFilename[i] = (char) FindFileData.cFileName[i];
			}
			uStatus = OpcUa_P_OpenSSL_BuildFullPath(a_pCrlPath, pFilename, MAX_PATH, CertFile);
			if (pFilename)
				free(pFilename);
#endif

#ifdef _GNUC_
		uStatus = OpcUa_P_OpenSSL_BuildFullPath(a_pCrlPath, FindFileData->d_name, MAX_PATH, CertFile);
#endif
		OpcUa_GotoErrorIfBad(uStatus);

		if (X509_load_crl_file(pLookup, CertFile, X509_FILETYPE_ASN1) != 1)
		{
			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "unexpected error X509_load_crl_file! %s\n", CertFile);
			OpcUa_GotoErrorWithStatus(OpcUa_Bad);
		}

		//TODO MEF
		while (FindNextFile(hFind, &FindFileData) != 0)
		{
#if defined (WIN32) || defined (_WIN32_WCE)
			pFilename = (char*)calloc(sizeof(char), length+1);
			if (pFilename)
			{
				for(i=0; i<length; i++) pFilename[i] = (char) FindFileData.cFileName[i];
			}
			uStatus = OpcUa_P_OpenSSL_BuildFullPath(a_pCrlPath, pFilename, MAX_PATH, CertFile);
			if (pFilename)
				free(pFilename);
#endif


#ifdef _GNUC_
			uStatus = OpcUa_P_OpenSSL_BuildFullPath(a_pCrlPath, FindFileData->d_name, MAX_PATH, CertFile);
#endif
			OpcUa_GotoErrorIfBad(uStatus);

			if (X509_load_crl_file(pLookup, CertFile, X509_FILETYPE_ASN1) != 1)
			{
				OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "unexpected error X509_load_crl_file! %s\n", CertFile);
				OpcUa_GotoErrorWithStatus(OpcUa_Bad);
			}
		}

		dwError = GetLastError();
		FindClose(hFind);

		if (dwError != ERROR_NO_MORE_FILES)
		{
			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "unexpected error loading CRL files! %d\n", dwError);            OpcUa_GotoErrorWithStatus(OpcUa_Bad);
		}
	}

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	/* nothing to do */

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_P_OpenSSL_CertificateStore_Open
 *===========================================================================*/
OpcUa_StatusCode OpcUa_P_OpenSSL_PKI_OpenCertificateStore(
	OpcUa_PKIProvider*          a_pProvider,
	OpcUa_Void**                a_ppCertificateStore)
{
	X509_LOOKUP* pLookup = OpcUa_Null; /* files or hash dirs */
	OpcUa_CertificateStoreConfiguration* pCertificateStoreCfg = OpcUa_Null;
	OpcUa_P_OpenSSL_CertificateStore* pCertificateStore  = OpcUa_Null;

	OpcUa_CharA pTrustedCertificateStorePath[MAX_PATH];
	OpcUa_CharA pTrustedCertificateCrlPath[MAX_PATH];
	OpcUa_CharA pIssuerCertificateStorePath[MAX_PATH];
	OpcUa_CharA pIssuerCertificateCrlPath[MAX_PATH];
	OpcUa_Int32 nRootPathLength = 0;
	OpcUa_Int32 iFlags = 0;

OpcUa_InitializeStatus(OpcUa_Module_P_OpenSSL, "PKI_OpenCertificateStore");

	OpcUa_ReturnErrorIfArgumentNull(a_pProvider);
	OpcUa_ReturnErrorIfArgumentNull(a_pProvider->Handle);

	*a_ppCertificateStore = OpcUa_Null;

	ZeroMemory(pTrustedCertificateStorePath,MAX_PATH);
	pTrustedCertificateStorePath[0] = '\0';
	ZeroMemory(pTrustedCertificateCrlPath,MAX_PATH);
	pTrustedCertificateCrlPath[0] = '\0';
	ZeroMemory(pIssuerCertificateStorePath,MAX_PATH);
	pIssuerCertificateStorePath[0] = '\0';
	ZeroMemory(pIssuerCertificateCrlPath,MAX_PATH);
	pIssuerCertificateCrlPath[0] = '\0';

	pCertificateStoreCfg = (OpcUa_CertificateStoreConfiguration*)a_pProvider->Handle;

	/* check the path length. */
	nRootPathLength = (OpcUa_Int32)strlen(pCertificateStoreCfg->strTrustedCertificateListLocation);

	if (nRootPathLength >= MAX_PATH-6)
	{
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "Certificate store file path length is too long: %d!\n", nRootPathLength);
		OpcUa_GotoErrorWithStatus(OpcUa_Bad);
	}

	OpcUa_P_String_strncpy(pTrustedCertificateStorePath, MAX_PATH-1, pCertificateStoreCfg->strTrustedCertificateListLocation, nRootPathLength);

	/* remove any trailing slashes */
#ifdef WIN32
	while(pTrustedCertificateStorePath[nRootPathLength-1] == '\\' && nRootPathLength > 0)
	{
		pTrustedCertificateStorePath[--nRootPathLength] = '\0';
	}

	OpcUa_P_String_strncpy(pTrustedCertificateCrlPath, MAX_PATH-1, pCertificateStoreCfg->strTrustedCertificateListLocation, nRootPathLength);
	OpcUa_P_String_strncat(pTrustedCertificateCrlPath, MAX_PATH-1, "\\crl", 4);

	if (pCertificateStoreCfg->strIssuerCertificateStoreLocation != OpcUa_Null)
	{
		OpcUa_Int32 nLength = (OpcUa_Int32)strlen(pCertificateStoreCfg->strIssuerCertificateStoreLocation);
		OpcUa_P_String_strncpy(pIssuerCertificateStorePath, MAX_PATH-1, pCertificateStoreCfg->strIssuerCertificateStoreLocation, nLength);

		/* remove any trailing slashes */
		while(pIssuerCertificateStorePath[nLength-1] == '\\' && nLength > 0)
		{
			pIssuerCertificateStorePath[--nLength] = '\0';
		}

		OpcUa_P_String_strncat(pIssuerCertificateStorePath, MAX_PATH-1, "\\certs", 6);
		OpcUa_P_String_strncpy(pIssuerCertificateCrlPath, MAX_PATH-1, pCertificateStoreCfg->strIssuerCertificateStoreLocation, nLength);
		OpcUa_P_String_strncat(pIssuerCertificateCrlPath, MAX_PATH-1, "\\crl", 4);
	}
#endif

#ifdef _GNUC_
	while(pTrustedCertificateStorePath[nRootPathLength-1] == '/' && nRootPathLength > 0)
	{
		pTrustedCertificateStorePath[--nRootPathLength] = '\0';
	}

	//OpcUa_P_String_strncat(pTrustedCertificateStorePath, MAX_PATH-1, "/certs", 6);
	OpcUa_P_String_strncpy(pTrustedCertificateCrlPath, MAX_PATH-1, pCertificateStoreCfg->strTrustedCertificateListLocation, nRootPathLength);
	OpcUa_P_String_strncat(pTrustedCertificateCrlPath, MAX_PATH-1, "/crl", 4);

	if (pCertificateStoreCfg->strIssuerCertificateStoreLocation != OpcUa_Null)
	{
		OpcUa_Int32 nLength = (OpcUa_Int32)strlen(pCertificateStoreCfg->strIssuerCertificateStoreLocation);
		OpcUa_P_String_strncpy(pIssuerCertificateStorePath, MAX_PATH-1, pCertificateStoreCfg->strIssuerCertificateStoreLocation, nLength);

		/* remove any trailing slashes */
		while(pIssuerCertificateStorePath[nLength-1] == '/' && nLength > 0)
		{
			pIssuerCertificateStorePath[--nLength] = '\0';
		}

		OpcUa_P_String_strncat(pIssuerCertificateStorePath, MAX_PATH-1, "/certs", 6);
		OpcUa_P_String_strncpy(pIssuerCertificateCrlPath, MAX_PATH-1, pCertificateStoreCfg->strIssuerCertificateStoreLocation, nLength);
		OpcUa_P_String_strncat(pIssuerCertificateCrlPath, MAX_PATH-1, "/crl", 4);
	}
#endif

	/************ Create Store Configuration ************/

	/* allocate the handle. */
	pCertificateStore = OpcUa_P_OpenSSL_CertificateStore_Alloc();
	OpcUa_GotoErrorIfAllocFailed(pCertificateStore);

	/* create a new store */
	pCertificateStore->MergedTrustList = X509_STORE_new();
	if(!pCertificateStore->MergedTrustList)
	{
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "error at X509_STORE_new!\n");
		OpcUa_GotoErrorWithStatus(OpcUa_Bad);
	}

	/* set the verification callback */
	X509_STORE_set_verify_cb_func(pCertificateStore->MergedTrustList, OpcUa_P_OpenSSL_CertificateStore_Verify_Callback);
	if((pCertificateStoreCfg->uFlags & OpcUa_P_PKI_OPENSSL_SET_DEFAULT_PATHS) == OpcUa_P_PKI_OPENSSL_SET_DEFAULT_PATHS)
	{
		if(X509_STORE_set_default_paths(pCertificateStore->MergedTrustList) != 1)
		{
			OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "error at X509_STORE_set_default_paths!\n");
			OpcUa_GotoErrorWithStatus(OpcUa_Bad);
		}
	}
	/* how to search for certificate & CRLs */
	pLookup = X509_STORE_add_lookup(pCertificateStore->MergedTrustList, X509_LOOKUP_file());
	if(!pLookup)
	{
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "error at X509_STORE_add_lookup!\n");
		OpcUa_GotoErrorWithStatus(OpcUa_Bad);
	}

	/* open the trusted certificates store. */
	uStatus = OpcUa_P_OpenSSL_CertificateStore_PopulateStore(pCertificateStore, pTrustedCertificateStorePath, OpcUa_True);
	OpcUa_GotoErrorIfBad(uStatus);

	/* open the untrusted CA certificates store. */
	if (pIssuerCertificateStorePath != OpcUa_Null && pIssuerCertificateStorePath[0] != '\0')
	{
		uStatus = OpcUa_P_OpenSSL_CertificateStore_PopulateStore(pCertificateStore, pIssuerCertificateStorePath, OpcUa_False);
		OpcUa_GotoErrorIfBad(uStatus);
	}

	/* how to search for certificate & CRLs */
	pLookup = X509_STORE_add_lookup(pCertificateStore->MergedTrustList, X509_LOOKUP_hash_dir());
	if(!pLookup)
	{
		uStatus = OpcUa_Bad;
		OpcUa_GotoErrorIfBad(uStatus);
	}

	/* add CTL lookup */
	if(X509_LOOKUP_add_dir(pLookup, pTrustedCertificateStorePath, X509_FILETYPE_ASN1) != 1)
	{
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "unexpected error at X509_LOOKUP_add_dir!\n");
		OpcUa_GotoErrorWithStatus(OpcUa_Bad);
	}

	/* how to search for certificate & CRLs */
	pLookup = X509_STORE_add_lookup(pCertificateStore->MergedTrustList, X509_LOOKUP_file());
	if(!pLookup)
	{
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "unexpected error X509_STORE_add_lookup!\n");
		OpcUa_GotoErrorWithStatus(OpcUa_Bad);
	}

	iFlags = X509_V_FLAG_CB_ISSUER_CHECK;

	/* add CRL lookup */
	if ((pCertificateStoreCfg->uFlags & OpcUa_P_PKI_OPENSSL_CHECK_REVOCATION_STATUS) == OpcUa_P_PKI_OPENSSL_CHECK_REVOCATION_STATUS)
	{
		iFlags |= X509_V_FLAG_CRL_CHECK;

		if (pTrustedCertificateCrlPath != OpcUa_Null && pTrustedCertificateCrlPath[0] != '\0')
		{
			uStatus = OpcUa_P_OpenSSL_CertificateStore_LoadCRLs(pLookup, pTrustedCertificateCrlPath);
			OpcUa_GotoErrorIfBad(uStatus);
		}

		if (pIssuerCertificateCrlPath != OpcUa_Null && pIssuerCertificateCrlPath[0] != '\0')
		{
			uStatus = OpcUa_P_OpenSSL_CertificateStore_LoadCRLs(pLookup, pIssuerCertificateCrlPath);
			OpcUa_GotoErrorIfBad(uStatus);
		}
	}

	/* set the flags of the store so that CRLs are consulted */
	/* ToDo: Time check fails: X509_V_FLAG_USE_CHECK_TIME ==> X509_V_ERR_CERT_NOT_YET_VALID */
#if (OPENSSL_VERSION_NUMBER > 0x00908000L)
	if(X509_STORE_set_flags(pCertificateStore->MergedTrustList, iFlags) != 1)
	{
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "unexpected X509_STORE_set_flags X509_load_crl_file!\n");
		OpcUa_GotoErrorWithStatus(OpcUa_Bad);
	}
#else
	X509_STORE_set_flags(pCertificateStore->MergedTrustList, iFlags);
#endif

	/* clean up */
	pLookup = OpcUa_Null;

	*a_ppCertificateStore = pCertificateStore;

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	OpcUa_P_OpenSSL_CertificateStore_Free(&pCertificateStore);
	pLookup = OpcUa_Null;

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_P_OpenSSL_CertificateStore_Close
 *===========================================================================*/
OpcUa_StatusCode OpcUa_P_OpenSSL_PKI_CloseCertificateStore(
	OpcUa_PKIProvider*          a_pProvider,
	OpcUa_Void**                a_ppCertificateStore) /* type depends on store implementation */
{
OpcUa_InitializeStatus(OpcUa_Module_P_OpenSSL, "PKI_CloseCertificateStore");

	OpcUa_ReferenceParameter(a_pProvider);

	OpcUa_P_OpenSSL_CertificateStore_Free((OpcUa_P_OpenSSL_CertificateStore**)a_ppCertificateStore);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}
OpcUa_StatusCode OpcUa_X509_CheckCommonName(X509_STORE_CTX *ctx, X509 *pCertificate)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_String entries[25]; // max will be 25
	OpcUa_UInt32 ii = 0;
	//
	for (ii = 0; ii < 25; ii++)
		OpcUa_String_Initialize(&entries[ii]);

	// get the subject name.
	OpcUa_CharA sBuffer[MAX_PATH * 10];
	ZeroMemory(&sBuffer[0],MAX_PATH * 10);
	if (X509_get_subject_name(pCertificate))
	{
		X509_NAME_oneline(X509_get_subject_name(pCertificate), sBuffer, sizeof(sBuffer));

		OpcUa_CharA* newBuffer = OpcUa_Null;
		int iPos = strlen(sBuffer);
		newBuffer = (OpcUa_CharA*)malloc(iPos+1);
		ZeroMemory(newBuffer, iPos + 1);
		memcpy(&newBuffer[0], &sBuffer[0], iPos + 1);
		char * pch = strrchr(newBuffer, '/');
		OpcUa_UInt16 i = 0;
		while (pch)
		{
			if (i < 24)
			{
				int iPos = pch - newBuffer + 1;
				//
				OpcUa_String_Initialize(&entries[i]);
				OpcUa_String_AttachCopy(&entries[i++], pch);
				free(newBuffer);
				newBuffer = OpcUa_Null;
				newBuffer = (OpcUa_CharA*)malloc(iPos);
				ZeroMemory(newBuffer, iPos);
				memcpy(&newBuffer[0], &sBuffer[0], iPos - 1); // -1 to remove the /
				pch = strrchr(newBuffer, '/');
			}
			else
				break;
		}
		if (newBuffer)
		{
			free(newBuffer);
			newBuffer = OpcUa_Null;
		}
		for (ii = 0; ii < 25; ii++)
			OpcUa_String_Clear(&entries[ii]);
	}
	else
		uStatus = OpcUa_BadDecodingError;
	return uStatus;
}

OpcUa_StatusCode OpcUa_X509_CheckCertTime(X509_STORE_CTX *ctx, X509 *x)
{
	time_t *ptime;
	OpcUa_StatusCode uStatus=OpcUa_Good;
	int i = 0;
	if (ctx->param)
	{
		if (ctx->param->flags & X509_V_FLAG_USE_CHECK_TIME)
			ptime = &ctx->param->check_time;
		else
			ptime = NULL;

		i = X509_cmp_time(X509_get_notBefore(x), ptime);
		if (i == 0) 
		{
			ctx->error = X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD;
			ctx->current_cert = x;
			if (!ctx->verify_cb(0, ctx))
				return uStatus;
		}

		if (i > 0) 
		{
			ctx->error = X509_V_ERR_CERT_NOT_YET_VALID;
			ctx->current_cert = x;
			if (!ctx->verify_cb(0, ctx))
				return uStatus;
		}

		i = X509_cmp_time(X509_get_notAfter(x), ptime);
		if (i == 0) 
		{
			ctx->error = X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD;
			ctx->current_cert = x;
			if (!ctx->verify_cb(0, ctx))
				return uStatus;
		}

		if (i < 0) 
		{
			ctx->error = X509_V_ERR_CERT_HAS_EXPIRED;
			ctx->current_cert = x;
			if (!ctx->verify_cb(0, ctx))
				return uStatus;
		}
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return 1;
}
/*============================================================================
 * OpcUa_P_OpenSSL_PKI_ValidateCertificate
 *===========================================================================*/
OpcUa_StatusCode OpcUa_P_OpenSSL_PKI_ValidateCertificate(
	OpcUa_PKIProvider*          a_pProvider,
	OpcUa_ByteString*           a_pCertificate,
	OpcUa_Void*                 a_pCertificateStore,
	OpcUa_Int*                  a_pValidationCode /* Validation return codes from OpenSSL */
)
{
	OpcUa_Byte* pPosition = OpcUa_Null;
	OpcUa_P_OpenSSL_CertificateStore* pStore = OpcUa_Null;
	OpcUa_Boolean bExplicitlyTrusted = OpcUa_False;

	X509* pX509Certificate = OpcUa_Null;
	X509_STORE* pX509Store = OpcUa_Null;
	X509_STORE_CTX* pContext = OpcUa_Null;    /* holds data used during verification process */
	OpcUa_Int iErrorCode = 0;
	OpcUa_StatusCode uStatus = OpcUa_Good;

	OpcUa_ReferenceParameter(a_pProvider);
	if (a_pCertificate)
	{
		if (a_pCertificateStore)
		{
			if (a_pValidationCode)
			{

				pStore = (OpcUa_P_OpenSSL_CertificateStore*)a_pCertificateStore;
				pX509Store = pStore->MergedTrustList;

				/* convert public key to X509 structure. */
				pPosition = a_pCertificate->Data;
				pX509Certificate = d2i_X509((X509**)OpcUa_Null, (const unsigned char**)&pPosition, a_pCertificate->Length);
				if (pX509Certificate)
				{
					/* create verification context and initialize it. */
					pContext = X509_STORE_CTX_new();
					if (pContext)
					{
						if (X509_STORE_CTX_init(pContext, pX509Store, pX509Certificate, NULL) == 1)
						{
							OpcUa_X509_CheckCommonName(pContext, pX509Certificate);
							// 
							pPosition = a_pCertificate->Data;
							//pX509Certificate = d2i_X509(&pX509Certificate, &pPosition, a_pCertificate->Length);
							//
							X509_get_notBefore(pX509Certificate);
							ASN1_TIME* notAfter = pX509Certificate->cert_info->validity->notAfter;
							ASN1_TIME* notBefore = pX509Certificate->cert_info->validity->notBefore;
							// 
							// 
							OpcUa_StatusCode uCheckTimeStatus=OpcUa_Good;
							OpcUa_StatusCode uIntegrityStatusCode = OpcUa_Good;
							OpcUa_StatusCode uChainStatusCode = OpcUa_Good;
							/* Test for date verification */
							int iCheckTimeResult = OpcUa_X509_CheckCertTime(pContext, pX509Certificate);
							if (iCheckTimeResult >= 0)
							{
								switch (pContext->error)
								{
								case X509_V_OK:
									uCheckTimeStatus = OpcUa_Good;
									break;
								case X509_V_ERR_CERT_HAS_EXPIRED:
								case X509_V_ERR_CERT_NOT_YET_VALID:
								case X509_V_ERR_CRL_NOT_YET_VALID:
								case X509_V_ERR_CRL_HAS_EXPIRED:
								case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
								case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
								case X509_V_ERR_ERROR_IN_CRL_LAST_UPDATE_FIELD:
								case X509_V_ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD:
									uCheckTimeStatus = OpcUa_BadCertificateTimeInvalid;
									break;
								case X509_V_ERR_SUBJECT_ISSUER_MISMATCH:
									uCheckTimeStatus = OpcUa_BadCertificateRevoked;
									break;
								case X509_V_ERR_CERT_REVOKED:
									uCheckTimeStatus = OpcUa_BadCertificateRevoked;
									break;
								default:
									break;
								}
							}
							/* */
							/* verify the certificate */
							if (uStatus == OpcUa_Good)
							{
								if (X509_verify_cert(pContext) == 1)
								{
									switch (pContext->error)
									{
									case X509_V_OK:
										uIntegrityStatusCode = OpcUa_Good;
										break;
									case X509_V_ERR_CERT_HAS_EXPIRED:
										uIntegrityStatusCode = OpcUa_BadCertificateTimeInvalid; // change for 033.js of Security Certificate Validation
										break;
									case X509_V_ERR_CERT_NOT_YET_VALID:
									case X509_V_ERR_CRL_NOT_YET_VALID:
									case X509_V_ERR_CRL_HAS_EXPIRED:
									case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
									case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
									case X509_V_ERR_ERROR_IN_CRL_LAST_UPDATE_FIELD:
									case X509_V_ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD:
										uIntegrityStatusCode = OpcUa_BadCertificateTimeInvalid;
										break;
									case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
										uIntegrityStatusCode = OpcUa_BadSecurityChecksFailed;// OpcUa_BadCertificateInvalid;
										break;
									case X509_V_ERR_CERT_REVOKED:
										uIntegrityStatusCode = OpcUa_BadCertificateRevoked;
										break;
									case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY:
										uIntegrityStatusCode = OpcUa_BadSecurityChecksFailed;
										break;
									case X509_V_ERR_CERT_UNTRUSTED:
										uIntegrityStatusCode = OpcUa_BadCertificateUntrusted;
										break;
									case X509_V_ERR_CERT_SIGNATURE_FAILURE:
										uIntegrityStatusCode = OpcUa_BadSecurityChecksFailed;
										break;
									case X509_V_ERR_AKID_SKID_MISMATCH:
										uIntegrityStatusCode = OpcUa_BadSecurityChecksFailed;
										break;
									default:
										uIntegrityStatusCode = X509_STORE_CTX_get_error(pContext); // 18 =X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT
										uIntegrityStatusCode = OpcUa_BadCertificateInvalid;
										break;
									}
								}
								else
								{
									iErrorCode = X509_STORE_CTX_get_error(pContext);
									if (iErrorCode == X509_V_ERR_CRL_NOT_YET_VALID)
										uIntegrityStatusCode = OpcUa_BadCertificateTimeInvalid;
									else
										uIntegrityStatusCode =OpcUa_BadSecurityChecksFailed; // Workaround
								}
							}
							/* now must verify that certificate has been explicitly trusted */
							if (uCheckTimeStatus == OpcUa_Good || uIntegrityStatusCode == OpcUa_Good)
							{
								uChainStatusCode = OpcUa_P_OpenSSL_CertificateStore_IsExplicitlyTrusted(pStore, pContext, pX509Certificate, &bExplicitlyTrusted);
								switch (pContext->error)
								{
								case X509_V_ERR_CERT_HAS_EXPIRED:
									uStatus = OpcUa_BadSecurityChecksFailed;
									break;
								case X509_V_ERR_SUBJECT_ISSUER_MISMATCH:
									uStatus = OpcUa_BadCertificateRevoked;
									break;
								case X509_V_ERR_AKID_SKID_MISMATCH:
									if (!bExplicitlyTrusted)
										uStatus = OpcUa_BadCertificateRevoked;
									break;
								default:
									break;
								}
								//if (!bExplicitlyTrusted)
								//{
								//	uStatus = OpcUa_BadCertificateUntrusted;
								//}
							}
							if (uCheckTimeStatus != OpcUa_Good || uIntegrityStatusCode != OpcUa_Good || uChainStatusCode != OpcUa_Good)
							{
								if ((uCheckTimeStatus == OpcUa_BadCertificateTimeInvalid) && (uIntegrityStatusCode == OpcUa_BadSecurityChecksFailed))
									uStatus = uIntegrityStatusCode;
								else
								{
									if ((uChainStatusCode != OpcUa_Good) && (uIntegrityStatusCode != OpcUa_Good))
										uStatus = uChainStatusCode;
									else
										uStatus = uIntegrityStatusCode;
								}
							}
							//X509_free(pX509Certificate);
							//X509_STORE_CTX_free(pContext);

							ERR_remove_state(0);
						}
						else
							uStatus=OpcUa_BadInternalError;
						// Free the context 
						X509_STORE_CTX_free(pContext);
					}
					else
						uStatus=OpcUa_Bad;
					X509_free(pX509Certificate);
				}
				else
					uStatus = OpcUa_BadOutOfMemory;
			}
			else
				uStatus = OpcUa_BadInvalidArgument;
		}
		else
			uStatus = OpcUa_BadInvalidArgument;
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}

/*============================================================================
 * OpcUa_P_OpenSSL_PKI_SaveCertificate
 *===========================================================================*/
/*
	ToDo:   Create Access to OpenSSL certificate store
			=> Only API to In-Memory-Store is available for version 0.9.8x
			=> Wait until Directory- and/or File-Store is available
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_PKI_SaveCertificate(
	OpcUa_PKIProvider*          a_pProvider,
	OpcUa_ByteString*           a_pCertificate,
	OpcUa_Void*                 a_pCertificateStore,
	OpcUa_Void*                 a_pSaveHandle)      /* Index or number within store/destination filepath */
{
	X509*                   pX509Certificate    = OpcUa_Null;
	BIO*                    pCertificateFile    = OpcUa_Null;
	const unsigned char*    pTemp               = OpcUa_Null;
	OpcUa_UInt32            i;

OpcUa_InitializeStatus(OpcUa_Module_P_OpenSSL, "PKI_SaveCertificate");

	OpcUa_ReturnErrorIfArgumentNull(a_pProvider);
	OpcUa_ReturnErrorIfArgumentNull(a_pProvider->Handle);
	OpcUa_ReturnErrorIfArgumentNull(a_pCertificate);
	OpcUa_ReturnErrorIfArgumentNull(a_pCertificateStore);
	OpcUa_ReturnErrorIfArgumentNull(a_pSaveHandle);

	/* copy DER encoded certificate, since d2i_X509 modifies the passed buffer */
	pTemp = a_pCertificate->Data;

	/* convert openssl X509 certificate to DER encoded bytestring certificate */
	pX509Certificate = d2i_X509((X509**)OpcUa_Null, &pTemp, (long)a_pCertificate->Length);
	if(pX509Certificate == OpcUa_Null)
	{
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "Error writing %s\n", a_pSaveHandle);
		OpcUa_GotoErrorWithStatus(OpcUa_BadInvalidArgument);
	}

	/* save DER certificate */
	pCertificateFile = BIO_new_file((const char*)a_pSaveHandle, "w");
	OpcUa_GotoErrorIfArgumentNull(pCertificateFile);

	i = i2d_X509_bio(pCertificateFile, pX509Certificate);

	if(i < 1)
	{
		uStatus = OpcUa_BadUnexpectedError;
	}

	BIO_free(pCertificateFile);
	X509_free(pX509Certificate);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	if(pCertificateFile)
	{
		BIO_free(pCertificateFile);
	}

	if(pX509Certificate)
	{
		X509_free(pX509Certificate);
	}

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_P_OpenSSL_CertificateStore_Certificate_Load
 *===========================================================================*/
/*
	ToDo:   Create Access to OpenSSL certificate store
			=> Only API to In-Memory-Store is available for version 0.9.8x
			=> Wait until Directory- and/or File-Store is available
*/
OpcUa_StatusCode OpcUa_P_OpenSSL_PKI_LoadCertificate(
	OpcUa_PKIProvider*          a_pProvider,
	OpcUa_Void*                 a_pLoadHandle,
	OpcUa_Void*                 a_pCertificateStore,
	OpcUa_ByteString*           a_pCertificate)
{
	OpcUa_Byte*     buf                 = OpcUa_Null;
	OpcUa_Byte*     p                   = OpcUa_Null;
	BIO*            pCertificateFile    = OpcUa_Null;
	X509*           pTmpCert            = OpcUa_Null;

	OpcUa_InitializeStatus(OpcUa_Module_P_OpenSSL, "PKI_LoadCertificate");

	OpcUa_ReferenceParameter(a_pProvider);
	OpcUa_ReferenceParameter(a_pCertificateStore);

	OpcUa_ReturnErrorIfArgumentNull(a_pLoadHandle);
	OpcUa_ReturnErrorIfArgumentNull(a_pCertificate);

	/* read DER certificates */
	pCertificateFile = BIO_new_file((const char*)a_pLoadHandle, "r");
	OpcUa_ReturnErrorIfArgumentNull(pCertificateFile);
	pTmpCert = d2i_X509_bio(pCertificateFile, (X509**)OpcUa_Null);
	if (!pTmpCert)
	{
		uStatus = OpcUa_Bad;
		OpcUa_GotoErrorIfBad(uStatus);
	}

	BIO_free(pCertificateFile);

	a_pCertificate->Length = i2d_X509(pTmpCert, NULL);
	buf = (OpcUa_Byte*)OpcUa_P_Memory_Alloc(a_pCertificate->Length);
	OpcUa_GotoErrorIfAllocFailed(buf);
	p = buf;
	i2d_X509(pTmpCert, &p);

	a_pCertificate->Data = (OpcUa_Byte*)OpcUa_P_Memory_Alloc(a_pCertificate->Length*sizeof(OpcUa_Byte));
	OpcUa_GotoErrorIfAllocFailed(a_pCertificate->Data);

	uStatus = OpcUa_P_Memory_MemCpy(a_pCertificate->Data, a_pCertificate->Length, buf, a_pCertificate->Length);
	OpcUa_GotoErrorIfBad(uStatus);

	if(pTmpCert != OpcUa_Null)
	{
		X509_free(pTmpCert);
		pTmpCert = OpcUa_Null;
	}

	if(buf != OpcUa_Null)
	{
		OpcUa_P_Memory_Free(buf);
		buf = OpcUa_Null;
		p = OpcUa_Null;
	}

	pCertificateFile = NULL;

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	if(pTmpCert != OpcUa_Null)
	{
		X509_free(pTmpCert);
		pTmpCert = OpcUa_Null;
	}

	if(a_pCertificate != OpcUa_Null)
	{
		if(a_pCertificate->Data != OpcUa_Null)
		{
			OpcUa_P_Memory_Free(a_pCertificate->Data);
			a_pCertificate->Data = OpcUa_Null;
			a_pCertificate->Length = -1;
		}
	}

	if(buf != OpcUa_Null)
	{
		OpcUa_P_Memory_Free(buf);
		buf = OpcUa_Null;
		p = OpcUa_Null;
	}

OpcUa_FinishErrorHandling;
}
/*============================================================================
 * OpcUa_P_OpenSSL_PKI_SplitCertificateChain
 *===========================================================================*/
/* This implementation has a rather high overhead due DER parsing and memory
   reallocation. Can be improved by manual content parsing. However, this way
   the content gets validated. */
OpcUa_StatusCode OpcUa_P_OpenSSL_PKI_SplitCertificateChain(
	OpcUa_ByteString*           a_pCertificateChain,
	OpcUa_UInt32*               a_pNumberOfChainElements,
	OpcUa_ByteString**          a_pabyChainElements)
{
	OpcUa_Byte* pPosition               = OpcUa_Null;
	OpcUa_Byte* pTemp                   = OpcUa_Null;
	X509*       pX509ChainCertificate   = OpcUa_Null;
	OpcUa_Void* pMem                    = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_P_OpenSSL, "OpenSSL_PKI_SplitCertificateChain");

	OpcUa_ReturnErrorIfArgumentNull(a_pCertificateChain);
	OpcUa_ReturnErrorIfArgumentNull(a_pNumberOfChainElements);

	*a_pNumberOfChainElements   = 0;

	/* only count elements */
	if(a_pabyChainElements != OpcUa_Null)
	{
		*a_pabyChainElements = OpcUa_Null;
	}

	/* (temporarily) convert first certificate. */
	pPosition = a_pCertificateChain->Data;
	pX509ChainCertificate = d2i_X509((X509**)OpcUa_Null, (const unsigned char**)&pPosition, a_pCertificateChain->Length);
	OpcUa_GotoErrorIfNull(pX509ChainCertificate, OpcUa_Bad);
	X509_free(pX509ChainCertificate);

	/* increment count */
	++(*a_pNumberOfChainElements);

	/* only count elements */
	if(a_pabyChainElements != OpcUa_Null)
	{
		/* allocate index buffer */
		*a_pabyChainElements = OpcUa_P_Memory_Alloc(sizeof(OpcUa_ByteString)* (*a_pNumberOfChainElements));
		OpcUa_GotoErrorIfAllocFailed((*a_pabyChainElements));

		/* set first index buffer */
		((*a_pabyChainElements)[0]).Data    = a_pCertificateChain->Data;
		((*a_pabyChainElements)[0]).Length  = pPosition - a_pCertificateChain->Data;
	}

	/* process chain */
	while(pPosition - a_pCertificateChain->Data < a_pCertificateChain->Length)
	{
		/* (temporarily) convert next certificate. */
		pTemp = pPosition;
		pX509ChainCertificate = d2i_X509((X509**)OpcUa_Null, (const unsigned char**)&pPosition, a_pCertificateChain->Length);
		OpcUa_GotoErrorIfNull(pX509ChainCertificate, OpcUa_BadInvalidArgument);
		X509_free(pX509ChainCertificate);

		++(*a_pNumberOfChainElements);

		/* only count elements */
		if(a_pabyChainElements != OpcUa_Null)
		{
			/* (re)allocate index buffer */
			pMem = OpcUa_P_Memory_ReAlloc(*a_pabyChainElements, sizeof(OpcUa_ByteString)* (*a_pNumberOfChainElements));
			OpcUa_GotoErrorIfAllocFailed(pMem);
			*a_pabyChainElements = pMem;

			/* set next index buffer */
			((*a_pabyChainElements)[*a_pNumberOfChainElements-1]).Data    = pTemp;
			((*a_pabyChainElements)[*a_pNumberOfChainElements-1]).Length  = pPosition - pTemp;
		}
	}

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	if(a_pabyChainElements != OpcUa_Null && *a_pabyChainElements != OpcUa_Null)
	{
		OpcUa_P_Memory_Free(*a_pabyChainElements);
		*a_pabyChainElements = OpcUa_Null;
	}

	if(a_pNumberOfChainElements != OpcUa_Null)
	{
		*a_pNumberOfChainElements   = 0;
	}

OpcUa_FinishErrorHandling;
}

OpcUa_StatusCode OpcUa_CertificateStoreConfiguration_Initialize(OpcUa_CertificateStoreConfiguration* pConfiguration)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	if (pConfiguration)
	{
		pConfiguration->strIssuerCertificateStoreLocation = OpcUa_Null;
		pConfiguration->strPkiType = OpcUa_Null;
		pConfiguration->strRevokedCertificateListLocation = OpcUa_Null;
		pConfiguration->strRevokedIssuerCertificateListLocation = OpcUa_Null;
		pConfiguration->strTrustedCertificateListLocation = OpcUa_Null;
		pConfiguration->uFlags = 0;
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}
OpcUa_StatusCode OpcUa_CertificateStoreConfiguration_Clear(OpcUa_CertificateStoreConfiguration* pConfiguration)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	if (pConfiguration)
	{
		if (pConfiguration->strIssuerCertificateStoreLocation)
		{
			OpcUa_P_Memory_Free(pConfiguration->strIssuerCertificateStoreLocation);
			pConfiguration->strIssuerCertificateStoreLocation = OpcUa_Null;
		}
		if (pConfiguration->strPkiType)
		{
			// OpcUa_P_Memory_Free(pConfiguration->strPkiType);
			pConfiguration->strPkiType = OpcUa_Null;
		}
		if (pConfiguration->strRevokedCertificateListLocation)
		{
			OpcUa_P_Memory_Free(pConfiguration->strRevokedCertificateListLocation);
			pConfiguration->strRevokedCertificateListLocation = OpcUa_Null;
		}
		if (pConfiguration->strRevokedIssuerCertificateListLocation)
		{
			OpcUa_P_Memory_Free(pConfiguration->strRevokedIssuerCertificateListLocation);
			pConfiguration->strRevokedIssuerCertificateListLocation = OpcUa_Null;
		}
		if (pConfiguration->strTrustedCertificateListLocation)
		{
			OpcUa_P_Memory_Free(pConfiguration->strTrustedCertificateListLocation);
			pConfiguration->strTrustedCertificateListLocation = OpcUa_Null;
		}
		pConfiguration->uFlags = 0;
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}


#endif /* OPCUA_REQUIRE_OPENSSL */
