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

/******************************************************************************************************/
/* Platform Portability Layer                                                                         */
/* Modify the content of this file according to the socket implementation on your system.             */
/******************************************************************************************************/

/* System Headers */
//#include <opcua_p_os.h>
#include <stdlib.h>


/* UA platform definitions */
#include <opcua_p_internal.h>
#include <opcua_p_memory.h>

/* own headers */
#include <opcua_p_utilities.h>

#ifdef _MSC_VER
#pragma warning(disable:4748) /* suppress /GS can not protect parameters and local variables from local buffer overrun because optimizations are disabled in function */
#endif /* _MSC_VER */

/* maximum number of characters per port including \0 */
#define MAX_PORT_LENGTH 16


/*============================================================================
 * OpcUa_P_ByteString_Copy
 *===========================================================================*/
OpcUa_StatusCode OpcUa_P_ByteString_Copy(   OpcUa_ByteString* a_pSrc,
                                            OpcUa_ByteString* a_pDst)
{
OpcUa_InitializeStatus(OpcUa_Module_P_Win32, "OpcUa_P_ByteString_Copy");

    OpcUa_ReturnErrorIfArgumentNull(a_pSrc);
    OpcUa_ReturnErrorIfArgumentNull(a_pDst);

    a_pDst->Length = a_pSrc->Length;

    if(a_pSrc->Data != OpcUa_Null && a_pSrc->Length > 0)
    {
        a_pDst->Data = OpcUa_P_Memory_Alloc(a_pDst->Length);
        OpcUa_GotoErrorIfAllocFailed(a_pDst->Data);
        OpcUa_P_Memory_MemCpy(a_pDst->Data, a_pDst->Length, a_pSrc->Data, a_pSrc->Length);
    }
    else
    {
        a_pDst->Data = OpcUa_Null;
    }

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    a_pDst->Length = -1;

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_Key_Copy
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Key_Copy(OpcUa_Key* a_pSrc,
                                OpcUa_Key* a_pDst)
{
OpcUa_InitializeStatus(OpcUa_Module_P_Win32, "OpcUa_Key_Copy");

    OpcUa_GotoErrorIfTrue((OPCUA_CRYPTO_KEY_ISHANDLE(a_pSrc)), OpcUa_BadInvalidArgument);

    a_pDst->fpClearHandle = 0;
    a_pDst->Type = a_pSrc->Type;

    uStatus = OpcUa_P_ByteString_Copy(  &a_pSrc->Key,
                                        &a_pDst->Key);
    OpcUa_GotoErrorIfBad(uStatus);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * Quick Sort
 *===========================================================================*/
OpcUa_Void OPCUA_DLLCALL OpcUa_P_QSort( OpcUa_Void*       pElements,
                                        OpcUa_UInt32      nElementCount,
                                        OpcUa_UInt32      nElementSize,
                                        OpcUa_PfnCompare* pfnCompare,
                                        OpcUa_Void*       pContext)
{
    /*qsort_s(pElements, nElementCount, nElementSize, pfnCompare, pContext);*/
    OpcUa_ReferenceParameter(pContext);
    qsort(pElements, nElementCount, nElementSize, pfnCompare);
}

/*============================================================================
 * Binary Search on sorted array
 *===========================================================================*/
OpcUa_Void* OPCUA_DLLCALL OpcUa_P_BSearch(  OpcUa_Void*       pKey,
                                            OpcUa_Void*       pElements,
                                            OpcUa_UInt32      nElementCount,
                                            OpcUa_UInt32      nElementSize,
                                            OpcUa_PfnCompare* pfnCompare,
                                            OpcUa_Void*       pContext)
{
    /*return bsearch_s(pKey, pElements, nElementCount, nElementSize, pfnCompare, pContext);*/
    OpcUa_ReferenceParameter(pContext);

	// Modified by Philippe (JRC) 01/09/2010
    // code copied from stlib_extras

	while (nElementCount-- > 0)
	{
		if (pfnCompare(pKey, pElements) == 0)
			return (void*)pElements;

		pElements = (char*)pElements + nElementSize;
	}

	return NULL;
    //return bsearch(pKey, pElements, nElementCount, nElementSize, pfnCompare);
}

/*============================================================================
 * Access to errno
 *===========================================================================*/
OpcUa_UInt32 OPCUA_DLLCALL OpcUa_P_GetLastError()
{
#ifdef WIN32
	return GetLastError();
#endif
#ifdef _GNUC_
    return errno;
#endif
}

/*============================================================================
 * OpcUa_GetTickCount
 *===========================================================================*/
OpcUa_UInt32 OPCUA_DLLCALL OpcUa_P_GetTickCount()
{
    return GetTickCount();
}

/*============================================================================
 * OpcUa_CharAToInt
 *===========================================================================*/
OpcUa_Int32 OPCUA_DLLCALL OpcUa_P_CharAToInt(OpcUa_StringA sValue)
{
    return (OpcUa_Int32)atoi(sValue);
}

/*============================================================================
 * OpcUa_P_ParseUrl
 *===========================================================================*/
OpcUa_StatusCode OpcUa_P_ParseUrl(  OpcUa_StringA   a_psUrl,
                                    OpcUa_StringA*  a_psIpAdress,
                                    OpcUa_UInt16*   a_puPort,
                                    OpcUa_Boolean*  a_pbTLS)
{
    OpcUa_UInt32    uUrlLength        = 0;

    OpcUa_StringA   sHostName         = OpcUa_Null;
    OpcUa_UInt32    uHostNameLength   = 0;

    OpcUa_CharA*    pcCursor          = OpcUa_Null;

    OpcUa_Int       nIndex1           = 0;
    OpcUa_Int       nIpStart          = 0;

#ifdef _WIN32_WCE
	LPADDRINFO pAddrInfo = OpcUa_Null;
	struct sockaddr_in  *pIpInfo = OpcUa_Null;
#else
    struct hostent* pHostEnt          = OpcUa_Null;
#endif

OpcUa_InitializeStatus(OpcUa_Module_Utilities, "P_ParseUrl");

    OpcUa_ReturnErrorIfArgumentNull(a_psUrl);
    OpcUa_ReturnErrorIfArgumentNull(a_psIpAdress);
    OpcUa_ReturnErrorIfArgumentNull(a_puPort);

    *a_psIpAdress = OpcUa_Null;

    uUrlLength = (OpcUa_UInt32)strlen(a_psUrl);

    /* check for // (end of protocol header) */
    pcCursor = strstr(a_psUrl, "//");

    if(pcCursor != OpcUa_Null)
    {
        /* begin of host address */
        pcCursor += 2;
        nIndex1 = (OpcUa_Int)(pcCursor - a_psUrl);
    }
    else
    {
        uStatus = OpcUa_BadSyntaxError;
        OpcUa_ReturnStatusCode;
    }
    if(strncmp(a_psUrl, "https:", 6) == 0)
    {
        *a_pbTLS = OpcUa_True;
    }
    else
    {
        *a_pbTLS = OpcUa_False;
    }
    /* skip protocol prefix and store beginning of ip adress */
    nIpStart = nIndex1;

    /* skip host address */
    while(      a_psUrl[nIndex1] != ':'
            &&  a_psUrl[nIndex1] != '/'
            &&  a_psUrl[nIndex1] != 0
            &&  nIndex1          <  (OpcUa_Int32)uUrlLength)
    {
        nIndex1++;
    }

    uHostNameLength = nIndex1 - nIpStart;
    sHostName       = (OpcUa_StringA)malloc(uHostNameLength + 1);
    if(sHostName == NULL)
    {
        return OpcUa_BadOutOfMemory;
    }

    memcpy(sHostName, &a_psUrl[nIpStart], uHostNameLength);
    sHostName[uHostNameLength] = '\0';

#ifdef _WIN32_WCE
	getaddrinfo(sHostName,NULL,NULL,&pAddrInfo);
	pIpInfo = (struct sockaddr_in *) pAddrInfo->ai_addr;
#else
	pHostEnt = gethostbyname(sHostName);
#endif

    free(sHostName);

#ifdef _WIN32_WCE    
    if(pAddrInfo == NULL)
    {
        /* hostname could not be resolved */
        return OpcUa_BadHostUnknown;
    }
#else
    if(pHostEnt == NULL)
    {
        /* hostname could not be resolved */
        return OpcUa_BadHostUnknown;
    }
#endif

    nIpStart = 0;
    *a_psIpAdress = (OpcUa_StringA)OpcUa_P_Memory_Alloc(16);
    memset(*a_psIpAdress, 0, 16);

#ifdef _WIN32_WCE
#if OPCUA_USE_SAFE_FUNCTIONS
    nIpStart += sprintf_s(&(*a_psIpAdress)[0],         16,"%u", pIpInfo->sin_addr.S_un.S_un_b.s_b1);
    (*a_psIpAdress)[nIpStart++] = '.';
    nIpStart += sprintf_s(&(*a_psIpAdress)[nIpStart],  12,"%u", pIpInfo->sin_addr.S_un.S_un_b.s_b2);
    (*a_psIpAdress)[nIpStart++] = '.';
    nIpStart += sprintf_s(&(*a_psIpAdress)[nIpStart],   8,"%u", pIpInfo->sin_addr.S_un.S_un_b.s_b3);
    (*a_psIpAdress)[nIpStart++] = '.';
    nIpStart += sprintf_s(&(*a_psIpAdress)[nIpStart],   4,"%u", pIpInfo->sin_addr.S_un.S_un_b.s_b4);
#else /* OPCUA_USE_SAFE_FUNCTIONS */
    nIpStart += sprintf(&(*a_psIpAdress)[0], "%u", pIpInfo->sin_addr.S_un.S_un_b.s_b1);
    (*a_psIpAdress)[nIpStart++] = '.';
    nIpStart += sprintf(&(*a_psIpAdress)[nIpStart], "%u", pIpInfo->sin_addr.S_un.S_un_b.s_b2);
    (*a_psIpAdress)[nIpStart++] = '.';
    nIpStart += sprintf(&(*a_psIpAdress)[nIpStart], "%u", pIpInfo->sin_addr.S_un.S_un_b.s_b3);
    (*a_psIpAdress)[nIpStart++] = '.';
    nIpStart += sprintf(&(*a_psIpAdress)[nIpStart], "%u", pIpInfo->sin_addr.S_un.S_un_b.s_b4);
#endif /* OPCUA_USE_SAFE_FUNCTIONS */
#else /* WIN32_WCE */
#ifdef WIN32
#if OPCUA_USE_SAFE_FUNCTIONS
    nIpStart += sprintf_s(&(*a_psIpAdress)[0],         16,"%u", (unsigned char)(*((*pHostEnt).h_addr_list))[0]);
    (*a_psIpAdress)[nIpStart++] = '.';
    nIpStart += sprintf_s(&(*a_psIpAdress)[nIpStart],  12,"%u", (unsigned char)(*((*pHostEnt).h_addr_list))[1]);
    (*a_psIpAdress)[nIpStart++] = '.';
    nIpStart += sprintf_s(&(*a_psIpAdress)[nIpStart],   8,"%u", (unsigned char)(*((*pHostEnt).h_addr_list))[2]);
    (*a_psIpAdress)[nIpStart++] = '.';
    nIpStart += sprintf_s(&(*a_psIpAdress)[nIpStart],   4,"%u", (unsigned char)(*((*pHostEnt).h_addr_list))[3]);
#else /* OPCUA_USE_SAFE_FUNCTIONS */
    nIpStart += sprintf(&(*a_psIpAdress)[0], "%u", (unsigned char)(*((*pHostEnt).h_addr_list))[0]);
    (*a_psIpAdress)[nIpStart++] = '.';
    nIpStart += sprintf(&(*a_psIpAdress)[nIpStart], "%u", (unsigned char)(*((*pHostEnt).h_addr_list))[1]);
    (*a_psIpAdress)[nIpStart++] = '.';
    nIpStart += sprintf(&(*a_psIpAdress)[nIpStart], "%u", (unsigned char)(*((*pHostEnt).h_addr_list))[2]);
    (*a_psIpAdress)[nIpStart++] = '.';
    nIpStart += sprintf(&(*a_psIpAdress)[nIpStart], "%u", (unsigned char)(*((*pHostEnt).h_addr_list))[3]);
#endif /* OPCUA_USE_SAFE_FUNCTIONS */
#endif /* WIN32 */
#endif
    
#ifdef _GNUC_
      nIpStart += sprintf(&(*a_psIpAdress)[0], "%u", (unsigned char)(*((*pHostEnt).h_addr_list))[0]);
    (*a_psIpAdress)[nIpStart++] = '.';
    nIpStart += sprintf(&(*a_psIpAdress)[nIpStart], "%u", (unsigned char)(*((*pHostEnt).h_addr_list))[1]);
    (*a_psIpAdress)[nIpStart++] = '.';
    nIpStart += sprintf(&(*a_psIpAdress)[nIpStart], "%u", (unsigned char)(*((*pHostEnt).h_addr_list))[2]);
    (*a_psIpAdress)[nIpStart++] = '.';
    nIpStart += sprintf(&(*a_psIpAdress)[nIpStart], "%u", (unsigned char)(*((*pHostEnt).h_addr_list))[3]);  
#endif
    
    /* scan port */
    if(a_psUrl[nIndex1] == ':')
    {
        OpcUa_Int       nIndex2 = 0;
        OpcUa_CharA*    sPort   = OpcUa_Null;
        OpcUa_CharA sBuffer[MAX_PORT_LENGTH];

        /* skip delimiter */
        nIndex1++;

        /* store beginning of port */
        sPort = &a_psUrl[nIndex1];

        /* search for end of port */
        while(      a_psUrl[nIndex1] != '/'
                &&  a_psUrl[nIndex1] != 0
                &&  nIndex2          <  6)
        {
            nIndex1++;
            nIndex2++;
        }

        /* convert port */
        OpcUa_P_Memory_MemCpy(sBuffer, MAX_PORT_LENGTH-1, sPort, nIndex2);
        sBuffer[nIndex2] = 0;
        *a_puPort = (OpcUa_UInt16)OpcUa_P_CharAToInt(sBuffer);
    }
    else
    {
        /* return default port */
        if(strncmp(a_psUrl, "opc.tcp:", 5) == 0)
        {
            *a_puPort = OPCUA_TCP_DEFAULT_PORT;
        }
        else
        {
            if(strncmp(a_psUrl, "https:", 6) == 0)
            {
                *a_puPort = OPCUA_HTTPS_DEFAULT_PORT;
            }
            else
            {
                if(strncmp(a_psUrl, "http:", 5) == 0)
                {
                    *a_puPort = OPCUA_HTTP_DEFAULT_PORT;
                }
                else
                {
                    OpcUa_GotoErrorWithStatus(OpcUa_BadInvalidArgument);
                }
            }
        }
    }

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

#ifdef _GNUC_
int lastError;
char *folder;
char *file;
char *currentFile;
DIR *dp;

char *str_sub (const char *s, unsigned int start, unsigned int end)
{
   char *new_s = NULL;

   if (s != NULL && start < end)
   {

      new_s = malloc (sizeof (*new_s) * (end - start + 2));
      if (new_s != NULL)
      {
         int i;

         for (i = start; i <= end; i++)
         {
            new_s[i-start] = s[i];
         }
         new_s[i-start] = '\0';
      }
      else
      {
         exit (EXIT_FAILURE);
      }
   }
   return new_s;
}

// implementation of the WIN32 function FindFirstFile : return a file by findfiledata a file from lpFileName
// (filename can be included into this path, support of wildcard works)
OPCUA_EXPORT int OPCUA_DLLCALL FindFirstFile(char *lpFileName,struct dirent **findfiledata)
{

	int indexOfLastBackSlash = 0;
	int length,i;

	length = strlen(lpFileName) + 1;

	for(i = 0; i<length;++i){
		if(lpFileName[i] == '/'){
			indexOfLastBackSlash = i;
		}
	}

	// The folder path is the left part
	folder = calloc(sizeof(char*),indexOfLastBackSlash);
	folder = strncpy(folder,lpFileName,indexOfLastBackSlash);

	// The file name if exist
	file = calloc(sizeof(char*),length - indexOfLastBackSlash + 1);
	file = str_sub(lpFileName,indexOfLastBackSlash+1,length);

	// Is there a wildcard in the filename
	BOOL wildcard = FALSE;

	if(strspn(file,"*") != 0){
		wildcard = TRUE;
	}

	dp = opendir(folder);

	if(dp==NULL){

		lastError = 42;
		return -1;
	}

	BOOL found = FALSE;
	// If the file name is not a wildcard or null
	if( (strcmp(file,"*") == 0) || (strcmp(file,"") == 0) || ( (file[0]=='*') && (file[1] == '.') ) ){

		// return the first file (not necessary the one from a ls or dir command)
		while(found==FALSE){
		  		  
			if( (*findfiledata = readdir(dp)) != NULL){
			  if( (strcmp((*findfiledata)->d_name,".")!=0) && (strcmp((*findfiledata)->d_name,"..")!=0) )
			  {
				  found = TRUE;
			  }
			}else{
			  break;
			}
		}
		
		if(found==TRUE){
		  seekdir(dp,telldir(dp));
		  return 0;
		}
	}else{
		// else we need to find a file that match with the filename		
		while( (*findfiledata = readdir(dp)) )
		{
			// if no wildcard in filename
			if(!wildcard)
			{
				// raw compare
				if(strcmp((*findfiledata)->d_name,file) == 0)
				{
					found = TRUE;
					seekdir(dp,telldir(dp));
					return i;
				}
			}else{
			//else
				//regex compare
				if(fnmatch(file,(*findfiledata)->d_name,FNM_PERIOD)==0)
				{
					found = TRUE;
					seekdir(dp,telldir(dp));
					return i;
				}
			}
		}
		if(!found)
		{
			//if not found return the first file
			closedir(dp);
			dp = opendir(folder);
			*findfiledata = readdir(dp);
			seekdir(dp,telldir(dp));
			return 0;
		}
	}
	return -1;
}

OPCUA_EXPORT int OPCUA_DLLCALL GetLastError()
{
	return lastError;
}

OPCUA_EXPORT void OPCUA_DLLCALL SetLastError(int errorCode)
{
	lastError = errorCode;
}

OPCUA_EXPORT BOOL OPCUA_DLLCALL FindNextFile(HANDLE hFind, struct dirent **findfiledata)
{
	BOOL bResult=FALSE;
	struct stat entrystat;
	if (findfiledata)
	{
		*findfiledata = readdir(dp);
		if(*findfiledata)
		{
			while (!stat((*findfiledata)->d_name,&entrystat))
			{
				if (!S_ISDIR( entrystat.st_mode ))
				{
					seekdir(dp,telldir(dp));
					bResult=TRUE;
				}
				else
					*findfiledata = readdir(dp);
			}
		}
	}
	return bResult;
}

OPCUA_EXPORT void OPCUA_DLLCALL FindClose(HANDLE handleToClose)
{
	closedir(dp);
}

#endif
