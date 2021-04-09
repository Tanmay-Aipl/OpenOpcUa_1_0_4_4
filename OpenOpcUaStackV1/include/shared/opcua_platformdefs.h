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

/* This are the win32 platform definitions! */
#ifndef _OpcUa_PlatformDefs_H_
#define _OpcUa_PlatformDefs_H_ 1

#define OPCUA_HAVE_OPENSSL 1
#define _LITTLE_ENDIAN OPCUA_CONFIG_YES
#ifdef _MSC_VER
  #pragma warning( push )
  #pragma warning( disable : 4001 )
#endif /* _MSC_VER */

/* System Headers */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
//#ifdef WIN32
//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>
//#endif
#ifdef _MSC_VER
  #pragma warning( pop )
  #pragma warning (disable: 4115)
  #pragma warning (disable: 4102)
#endif /* _MSC_VER */

#ifdef __cplusplus
# define OPCUA_BEGIN_EXTERN_C extern "C" {
# define OPCUA_END_EXTERN_C }
#else
# define OPCUA_BEGIN_EXTERN_C
# define OPCUA_END_EXTERN_C
#endif

/*============================================================================
* Types and Mapping
*===========================================================================*/

OPCUA_BEGIN_EXTERN_C
#define OPCUA_P_LITTLE_ENDIAN 1234
#define OPCUA_P_BIG_ENDIAN    4321
#define OPCUA_P_BYTE_ORDER OPCUA_P_LITTLE_ENDIAN
/* marks functions that need to be exported via functiontable */
#if defined(_UA_STACK_BUILD_DLL)
  #define OPCUA_EXPORT __declspec(dllexport)
#else
  #define OPCUA_EXPORT 
#endif

#define OPCUA_EXPORT_SYNC_SERVER_API OPCUA_EXPORT

#if defined(_UA_STACK_USE_DLL)
  #define OPCUA_IMPORT __declspec(dllimport)
#else
  #define OPCUA_IMPORT 
#endif

#if defined(_UA_STACK_BUILD_DLL)
  #define OPCUA_IMEXPORT __declspec(dllexport)
#elif defined(_UA_STACK_USE_DLL)
  #define OPCUA_IMEXPORT __declspec(dllimport)
#else
  /* build/using static lib */
  #define OPCUA_IMEXPORT 
#endif

/* BEGIN Montpellier Workshop */
/* calling convention used by stack functions that explicitely use cdecl */
/* TODO clarify if we have to define that on Linux */
#ifdef _GNUC_
#define OPCUA_CDECL
#define OPCUA_DLLCALL
#endif
	
#if defined(WIN32) || defined (_WIN32_WCE)
	#ifdef _WIN32_WCE
		#define OPCUA_CDECL __cdecl
		#define OPCUA_DLLCALL __cdecl
	#else
		#define OPCUA_CDECL __cdecl
		/* call exported functions by stdcall convention */
		#define OPCUA_DLLCALL __stdcall
	#endif
#endif

/* END Montpellier Workshop */
/* used ie. for unlimited timespans */
#define OPCUA_INFINITE 0xFFFFFFFF

/* helper macro to convert a numeric define into a string (only use OPCUA_TOSTRING) */
#ifndef OPCUA_TOSTRING_HELP
# define OPCUA_TOSTRING_HELP(xNumber)  #xNumber
#endif

#ifndef OPCUA_TOSTRING
# define OPCUA_TOSTRING(xNumber) OPCUA_TOSTRING_HELP(xNumber)
#endif

/* typedefs used by the C++ implementation */
#if defined(_GNUC_)
typedef void* LPVOID;
#endif

/*============================================================================
* Additional basic headers
*===========================================================================*/
#include <string.h>

/* get compiler name and version */
#include <opcua_p_compilerinfo.h>

/*============================================================================
 * platform layer version information
 *===========================================================================*/
/* helper to get build time through buildsystem */
#ifndef OPCUA_P_BUILDINFO_BUILD_TIMESTAMP
#if (defined(__DATE__) && defined(__TIME__))
#define OPCUA_P_TIMESTAMP __DATE__ " " __TIME__
#define OPCUA_P_BUILDINFO_BUILD_TIMESTAMP OPCUA_P_TIMESTAMP
#endif /* __DATE__ && __TIME__ */
#endif /* OPCUA_P_BUILDINFO_BUILD_TIMESTAMP */

/** @brief Name of the platform or operating system this platform layer was created for. */
#ifndef OPCUA_P_BUILDINFO_PLATFORMNAME
/* TODO Add others platfoms as requested */
#ifdef _GNUC_
#define OPCUA_P_BUILDINFO_PLATFORMNAME             "Linux/TLS"
#else
# define OPCUA_P_BUILDINFO_PLATFORMNAME             "Windows/TLS"
#endif
#endif /* OPCUA_P_BUILDINFO_PLATFORMNAME */

/** @brief The date and time when the source was last modified. */
#ifndef OPCUA_P_BUILDINFO_SOURCE_TIMESTAMP
# define OPCUA_P_BUILDINFO_SOURCE_TIMESTAMP         "OPCUA_P_BUILDINFO_SOURCE_TIMESTAMP not set"
#endif /* OPCUA_P_BUILDINFO_SOURCE_TIMESTAMP */

/** @brief The date and time when the binary is build. */
#ifndef OPCUA_P_BUILDINFO_BUILD_TIMESTAMP
# define OPCUA_P_BUILDINFO_BUILD_TIMESTAMP          "OPCUA_P_BUILDINFO_BUILD_TIMESTAMP not set"
#endif /* OPCUA_P_BUILDINFO_BUILD_TIMESTAMP */

/** @brief The name of the company which build the binary. */
#ifndef OPCUA_P_BUILDINFO_VENDOR_NAME
# define OPCUA_P_BUILDINFO_VENDOR_NAME              "OPCUA_P_BUILDINFO_VENDOR_NAME not set"
#endif /* OPCUA_P_BUILDINFO_VENDOR_NAME */

/** @brief Additional information from the company, ie. internal revision number. */
#ifndef OPCUA_P_BUILDINFO_VENDOR_INFO
# define OPCUA_P_BUILDINFO_VENDOR_INFO              "OPCUA_P_BUILDINFO_VENDOR_INFO not set"
#endif /* OPCUA_P_BUILDINFO_VENDOR_INFO */

/** @brief Additional information from the company, ie. internal revision number. */
#ifndef OPCUA_P_BUILDINFO_COMPILER
# define OPCUA_P_BUILDINFO_COMPILER                 OPCUA_P_COMPILERINFO
#endif /* OPCUA_P_BUILDINFO_COMPILER */

/** @brief Separator used between different fields in the version string. */
#define OPCUA_P_VERSIONSTRING_SEPARATOR             "\\"

/** @brief The versionstring returned by OpcUa_P_GetVersion. */
#define OPCUA_P_VERSIONSTRING                       OPCUA_P_BUILDINFO_PLATFORMNAME      OPCUA_P_VERSIONSTRING_SEPARATOR \
													OPCUA_P_BUILDINFO_SOURCE_TIMESTAMP  OPCUA_P_VERSIONSTRING_SEPARATOR \
													OPCUA_P_BUILDINFO_BUILD_TIMESTAMP   OPCUA_P_VERSIONSTRING_SEPARATOR \
													OPCUA_P_BUILDINFO_VENDOR_NAME       OPCUA_P_VERSIONSTRING_SEPARATOR \
													OPCUA_P_BUILDINFO_VENDOR_INFO       OPCUA_P_VERSIONSTRING_SEPARATOR \
													OPCUA_P_BUILDINFO_COMPILER

/*============================================================================
* Additional basic headers
*===========================================================================*/
/* configuration switches */
#include <opcua_config.h>

/* basic type mapping */
#include "opcua_p_types.h"

/**********************************************************************************/
/*/  Security Configuration section.                                             /*/
/**********************************************************************************/
#ifndef OPCUA_SUPPORT_SECURITYPOLICY_BASIC128RSA15
#define OPCUA_SUPPORT_SECURITYPOLICY_BASIC128RSA15  OPCUA_CONFIG_YES
#endif  /* OPCUA_SUPPORT_SECURITYPOLICY_BASIC128RSA15 */

#ifndef OPCUA_SUPPORT_SECURITYPOLICY_BASIC256
#define OPCUA_SUPPORT_SECURITYPOLICY_BASIC256       OPCUA_CONFIG_YES
#endif /* OPCUA_SUPPORT_SECURITYPOLICY_BASIC256 */

#ifndef OPCUA_SUPPORT_SECURITYPOLICY_NONE
#define OPCUA_SUPPORT_SECURITYPOLICY_NONE           OPCUA_CONFIG_YES
#endif  /* OPCUA_SUPPORT_SECURITYPOLICY_NONE */

#ifndef OPCUA_SUPPORT_PKI
#define OPCUA_SUPPORT_PKI                           OPCUA_CONFIG_YES
#endif  /* OPCUA_SUPPORT_PKI */

#if OPCUA_SUPPORT_PKI
#define OPCUA_SUPPORT_PKI_OVERRIDE                  OPCUA_CONFIG_NO
#define OPCUA_SUPPORT_PKI_OPENSSL                   OPCUA_CONFIG_YES
#ifdef WIN32
#define OPCUA_SUPPORT_PKI_WIN32                     OPCUA_CONFIG_YES
#endif
#endif /* OPCUA_SUPPORT_PKI */

/*============================================================================
 * Memory allocation functions.
 *
 * Note: Realloc and Free behave gracefully if passed a NULL pointer. Changing
 * these functions to a macro call to free will cause problems.
 *===========================================================================*/

/* shortcuts for often used memory functions */
#define OpcUa_Alloc(xSize)                              OpcUa_Memory_Alloc(xSize)
#define OpcUa_ReAlloc(xSrc, xSize)                      OpcUa_Memory_ReAlloc(xSrc, xSize)
#define OpcUa_Free(xSrc)                                OpcUa_Memory_Free(xSrc)
#define OpcUa_MemCpy(xDst, xDstSize, xSrc, xCount)      OpcUa_Memory_MemCpy(xDst, xDstSize, xSrc, xCount)

/* import prototype for direct mapping on memset */
#define OpcUa_MemSet(xDst, xValue, xDstSize)            memset(xDst, xValue, xDstSize)
#define OpcUa_MemSetD(xDst, xValue, xDstSize)           memset(xDst, xValue, xDstSize)

/* import prototype for direct mapping on memcmp */
#define OpcUa_MemCmp(xBuf1, xBuf2, xBufSize)            memcmp(xBuf1, xBuf2, xBufSize)
#define OpcUa_WcsCpy									wcscpy
#define OpcUa_WcsCat									wcscat
/*============================================================================
 * String handling functions.
 *===========================================================================*/

/* mapping of ansi string functions on lib functions: */
#define OpcUa_StrCmpA(xStr1, xStr2)                     strcmp(xStr1, xStr2)
#define OpcUa_StrnCmpA(xStr1, xStr2, xCount)            strncmp(xStr1, xStr2, xCount)
#ifdef _GNUC_
#define OpcUa_StriCmpA(xStr1, xStr2)                    stricmp(xStr1, xStr2)
#define OpcUa_StrinCmpA(xStr1, xStr2, xCount)           strnicmp(xStr1, xStr2, xCount)
#else
#define OpcUa_StriCmpA(xStr1, xStr2)                    _stricmp(xStr1, xStr2)
#define OpcUa_StrinCmpA(xStr1, xStr2, xCount)           _strnicmp(xStr1, xStr2, xCount)
#endif
#define OpcUa_StrLenA(xStr)                             (OpcUa_UInt32)strlen(xStr)
#define OpcUa_StrChrA(xString, xChar)                   strchr(xString, (OpcUa_CharA)xChar)
#define OpcUa_StrrChrA(xString, xChar)                  strrchr(xString, (OpcUa_CharA)xChar)
#define OpcUa_StrStrA(xString, xSubstring)              strstr(xString, xSubstring)

#ifndef _INC_STDIO
/* import prototype for direct mapping on sprintf for files which are not allowed to include
   system headers */
OPCUA_IMPORT OpcUa_Int sprintf(OpcUa_CharA* buffer, const OpcUa_CharA* format, ...);
#endif /* _INC_STDIO */

#ifdef _GNUC_
	#define OpcUa_StrnCpyA(xDst, xDstSize, xSrc, xCount) strncpy(xDst, xSrc, xCount)
	#define OpcUa_StrnCatA(xDst, xDstSize, xSrc, xCount) strncat(xDst, xSrc, xCount)
	#define OpcUa_SPrintfA                               sprintf
	#define OpcUa_SnPrintfA                              snprintf
	#define OpcUa_SScanfA                                sscanf
	#define OpcUa_SWPrintf								 swprintf
#else
	/* NOT _GNUC_ CODE */
	#if OPCUA_USE_SAFE_FUNCTIONS
	#define OpcUa_StrnCpyA(xDst, xDstSize, xSrc, xCount) strncpy_s(xDst, xDstSize, xSrc, xCount)
	#define OpcUa_StrnCatA(xDst, xDstSize, xSrc, xCount) strncat_s(xDst, xDstSize, xSrc, xCount)
	#define OpcUa_SPrintfA                               sprintf_s
	#define OpcUa_SnPrintfA                              _snprintf_s
	#define OpcUa_SScanfA                                sscanf_s
	#else
	#define OpcUa_StrnCpyA(xDst, xDstSize, xSrc, xCount) strncpy(xDst, xSrc, xCount)
	#define OpcUa_StrnCatA(xDst, xDstSize, xSrc, xCount) strncat(xDst, xSrc, xCount)
	#define OpcUa_SPrintfA                               sprintf
	#define OpcUa_SnPrintfA                              _snprintf
	#define OpcUa_SScanfA                                sscanf
	#define OpcUa_SScanfA                                sscanf
#ifndef _WIN32_WCE
	#define OpcUa_SWPrintf								swprintf
#endif
	#endif
#endif

/* import prototype for direct mapping on sscanf for files which are not allowed to include
   system headers */
#ifndef _INC_STDIO
OPCUA_IMPORT OpcUa_Int sscanf(const OpcUa_CharA* buffer, const OpcUa_CharA* format, ... );
#endif /* _INC_STDIO */

/* shortcuts to OpcUa_String functions */
#define OpcUa_StrLen(xStr)                              OpcUa_String_StrLen(xStr)
/* DLL and function handling*/
#if defined(WIN32) || defined(_WIN32_WCE)
	#define OpcUa_GetProcAddress							GetProcAddress
#else
	#ifdef _GNUC_
		#define OpcUa_GetProcAddress								dlsym
	#endif
#endif
#ifdef _WIN32_WCE
	#define OpcUa_Clock										GetTickCount
#else
	#define	OpcUa_Clock										clock
#endif

OPCUA_END_EXTERN_C

#endif /* _OpcUa_PlatformDefs_H_ */
