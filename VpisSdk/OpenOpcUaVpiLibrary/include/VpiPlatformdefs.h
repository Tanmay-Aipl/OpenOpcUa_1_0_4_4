//**************************************************************************
//
//  Copyright (c) 4CE Industry 1999-2014, All Rights Reserved
//
//**************************************************************************
//
//  Filename   :  VpiPlatformdefs.h
//  $Author    :  Michel Condemine
//
//  Description:  This file is part of the OpenOpcUaLibrary. 
//                This compoment and all related file are not Open Source
//				  This must be use with the autorisation of Michel Condemine
//**************************************************************************

/* This are the win32 platform definitions! */
#ifndef _Vpi_PlatformDefs_H_
#define _Vpi_PlatformDefs_H_ 1

#define VPI_HAVE_OPENSSL 1
#define _LITTLE_ENDIAN VPI_CONFIG_YES
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
# define VPI_BEGIN_EXTERN_C extern "C" {
# define VPI_END_EXTERN_C }
#else
# define VPI_BEGIN_EXTERN_C
# define VPI_END_EXTERN_C
#endif

/*============================================================================
* Types and Mapping
*===========================================================================*/

VPI_BEGIN_EXTERN_C
#define VPI_P_LITTLE_ENDIAN 1234
#define VPI_P_BIG_ENDIAN    4321
#define VPI_P_BYTE_ORDER VPI_P_LITTLE_ENDIAN
/* marks functions that need to be exported via functiontable */
#if defined(_VPI_BUILD_DLL)
  #define VPILIBRARY_EXPORT __declspec(dllexport)
#else
  #define VPILIBRARY_EXPORT 
#endif

#define VPILIBRARY_EXPORT_SYNC_SERVER_API VPILIBRARY_EXPORT

#if defined(_VPI_USE_DLL)
  #define VPILIBRARY_IMPORT __declspec(dllimport)
#else
  #define VPILIBRARY_IMPORT 
#endif

#if defined(_VPI_BUILD_DLL)
  #define VPI_IMEXPORT __declspec(dllexport)
#elif defined(_VPI_USE_DLL)
  #define VPI_IMEXPORT __declspec(dllimport)
#else
  /* build/using static lib */
  #define VPI_IMEXPORT 
#endif

/* BEGIN Montpellier Workshop */
/* calling convention used by stack functions that explicitely use cdecl */
/* TODO clarify if we have to define that on Linux */
#ifdef _GNUC_
#define VPI_CDECL
#define VPI_DLLCALL
#endif
	
#if defined(WIN32) || defined (_WIN32_WCE)
	#ifdef _WIN32_WCE
		#define VPI_CDECL __cdecl
		#define VPI_DLLCALL __cdecl
	#else
		#define VPI_CDECL __cdecl
		/* call exported functions by stdcall convention */
		#define VPI_DLLCALL __stdcall
	#endif
#endif

/* END Montpellier Workshop */
/* used ie. for unlimited timespans */
#define VPI_INFINITE 0xFFFFFFFF

/* helper macro to convert a numeric define into a string (only use VPI_TOSTRING) */
#ifndef VPI_TOSTRING_HELP
# define VPI_TOSTRING_HELP(xNumber)  #xNumber
#endif

#ifndef VPI_TOSTRING
# define VPI_TOSTRING(xNumber) VPI_TOSTRING_HELP(xNumber)
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
//#include <opcua_p_compilerinfo.h>

/*============================================================================
 * platform layer version information
 *===========================================================================*/
/* helper to get build time through buildsystem */
#ifndef VPI_P_BUILDINFO_BUILD_TIMESTAMP
#if (defined(__DATE__) && defined(__TIME__))
#define VPI_P_TIMESTAMP __DATE__ " " __TIME__
#define VPI_P_BUILDINFO_BUILD_TIMESTAMP VPI_P_TIMESTAMP
#endif /* __DATE__ && __TIME__ */
#endif /* VPI_P_BUILDINFO_BUILD_TIMESTAMP */

/** @brief Name of the platform or operating system this platform layer was created for. */
#ifndef VPI_P_BUILDINFO_PLATFORMNAME
/* TODO Add others platfoms as requested */
#ifdef _GNUC_
#define VPI_P_BUILDINFO_PLATFORMNAME             "Linux/TLS"
#else
# define VPI_P_BUILDINFO_PLATFORMNAME             "Windows/TLS"
#endif
#endif /* VPI_P_BUILDINFO_PLATFORMNAME */

/** @brief The date and time when the source was last modified. */
#ifndef VPI_P_BUILDINFO_SOURCE_TIMESTAMP
# define VPI_P_BUILDINFO_SOURCE_TIMESTAMP         "VPI_P_BUILDINFO_SOURCE_TIMESTAMP not set"
#endif /* VPI_P_BUILDINFO_SOURCE_TIMESTAMP */

/** @brief The date and time when the binary is build. */
#ifndef VPI_P_BUILDINFO_BUILD_TIMESTAMP
# define VPI_P_BUILDINFO_BUILD_TIMESTAMP          "VPI_P_BUILDINFO_BUILD_TIMESTAMP not set"
#endif /* VPI_P_BUILDINFO_BUILD_TIMESTAMP */

/** @brief The name of the company which build the binary. */
#ifndef VPI_P_BUILDINFO_VENDOR_NAME
# define VPI_P_BUILDINFO_VENDOR_NAME              "VPI_P_BUILDINFO_VENDOR_NAME not set"
#endif /* VPI_P_BUILDINFO_VENDOR_NAME */

/** @brief Additional information from the company, ie. internal revision number. */
#ifndef VPI_P_BUILDINFO_VENDOR_INFO
# define VPI_P_BUILDINFO_VENDOR_INFO              "VPI_P_BUILDINFO_VENDOR_INFO not set"
#endif /* VPI_P_BUILDINFO_VENDOR_INFO */

/** @brief Additional information from the company, ie. internal revision number. */
#ifndef VPI_P_BUILDINFO_COMPILER
# define VPI_P_BUILDINFO_COMPILER                 VPI_P_COMPILERINFO
#endif /* VPI_P_BUILDINFO_COMPILER */

/** @brief Separator used between different fields in the version string. */
#define VPI_P_VERSIONSTRING_SEPARATOR             "\\"

/** @brief The versionstring returned by Vpi_P_GetVersion. */
#define VPI_P_VERSIONSTRING                       VPI_P_BUILDINFO_PLATFORMNAME      VPI_P_VERSIONSTRING_SEPARATOR \
													VPI_P_BUILDINFO_SOURCE_TIMESTAMP  VPI_P_VERSIONSTRING_SEPARATOR \
													VPI_P_BUILDINFO_BUILD_TIMESTAMP   VPI_P_VERSIONSTRING_SEPARATOR \
													VPI_P_BUILDINFO_VENDOR_NAME       VPI_P_VERSIONSTRING_SEPARATOR \
													VPI_P_BUILDINFO_VENDOR_INFO       VPI_P_VERSIONSTRING_SEPARATOR \
													VPI_P_BUILDINFO_COMPILER

/*============================================================================
* Additional basic headers
*===========================================================================*/
/* configuration switches */
#include <VpiConfig.h>

/* basic type mapping */
//#include "opcua_p_types.h"

/**********************************************************************************/
/*/  Security Configuration section.                                             /*/
/**********************************************************************************/
#ifndef VPI_SUPPORT_SECURITYPOLICY_BASIC128RSA15
#define VPI_SUPPORT_SECURITYPOLICY_BASIC128RSA15  VPI_CONFIG_YES
#endif  /* VPI_SUPPORT_SECURITYPOLICY_BASIC128RSA15 */

#ifndef VPI_SUPPORT_SECURITYPOLICY_BASIC256
#define VPI_SUPPORT_SECURITYPOLICY_BASIC256       VPI_CONFIG_YES
#endif /* VPI_SUPPORT_SECURITYPOLICY_BASIC256 */

#ifndef VPI_SUPPORT_SECURITYPOLICY_NONE
#define VPI_SUPPORT_SECURITYPOLICY_NONE           VPI_CONFIG_YES
#endif  /* VPI_SUPPORT_SECURITYPOLICY_NONE */

#ifndef VPI_SUPPORT_PKI
#define VPI_SUPPORT_PKI                           VPI_CONFIG_YES
#endif  /* VPI_SUPPORT_PKI */

#if VPI_SUPPORT_PKI
#define VPI_SUPPORT_PKI_OVERRIDE                  VPI_CONFIG_NO
#define VPI_SUPPORT_PKI_OPENSSL                   VPI_CONFIG_YES
#ifdef WIN32
#define VPI_SUPPORT_PKI_WIN32                     VPI_CONFIG_YES
#endif
#endif /* VPI_SUPPORT_PKI */

/*============================================================================
 * Memory allocation functions.
 *
 * Note: Realloc and Free behave gracefully if passed a NULL pointer. Changing
 * these functions to a macro call to free will cause problems.
 *===========================================================================*/

/* shortcuts for often used memory functions */
#define Vpi_Alloc(xSize)                              malloc(xSize)
#define Vpi_ReAlloc(xSrc, xSize)                      Vpi_Memory_ReAlloc(xSrc, xSize)
#define Vpi_Free(xSrc)                                free(xSrc)
#define Vpi_MemCpy(xDst, xDstSize, xSrc, xCount)      Vpi_Memory_MemCpy(xDst, xDstSize, xSrc, xCount)

/* import prototype for direct mapping on memset */
#define Vpi_MemSet(xDst, xValue, xDstSize)            memset(xDst, xValue, xDstSize)
#define Vpi_MemSetD(xDst, xValue, xDstSize)           memset(xDst, xValue, xDstSize)

/* import prototype for direct mapping on memcmp */
#define Vpi_MemCmp(xBuf1, xBuf2, xBufSize)            memcmp(xBuf1, xBuf2, xBufSize)
#define Vpi_WcsCpy									wcscpy
#define Vpi_WcsCat									wcscat
/*============================================================================
 * String handling functions.
 *===========================================================================*/

/* mapping of ansi string functions on lib functions: */
#define Vpi_StrCmpA(xStr1, xStr2)                     strcmp(xStr1, xStr2)
#define Vpi_StrnCmpA(xStr1, xStr2, xCount)            strncmp(xStr1, xStr2, xCount)
#ifdef _GNUC_
#define Vpi_StriCmpA(xStr1, xStr2)                    stricmp(xStr1, xStr2)
#define Vpi_StrinCmpA(xStr1, xStr2, xCount)           strnicmp(xStr1, xStr2, xCount)
#else
#define Vpi_StriCmpA(xStr1, xStr2)                    _stricmp(xStr1, xStr2)
#define Vpi_StrinCmpA(xStr1, xStr2, xCount)           _strnicmp(xStr1, xStr2, xCount)
#endif
#define Vpi_StrLenA(xStr)                             (Vpi_UInt32)strlen(xStr)
#define Vpi_StrChrA(xString, xChar)                   strchr(xString, (Vpi_CharA)xChar)
#define Vpi_StrrChrA(xString, xChar)                  strrchr(xString, (Vpi_CharA)xChar)
#define Vpi_StrStrA(xString, xSubstring)              strstr(xString, xSubstring)

#ifndef _INC_STDIO
/* import prototype for direct mapping on sprintf for files which are not allowed to include
   system headers */
VPI_IMPORT Vpi_Int sprintf(Vpi_CharA* buffer, const Vpi_CharA* format, ...);
#endif /* _INC_STDIO */

#ifdef _GNUC_
	#define Vpi_StrnCpyA(xDst, xDstSize, xSrc, xCount) strncpy(xDst, xSrc, xCount)
	#define Vpi_StrnCatA(xDst, xDstSize, xSrc, xCount) strncat(xDst, xSrc, xCount)
	#define Vpi_SPrintfA                               sprintf
	#define Vpi_SnPrintfA                              snprintf
	#define Vpi_SScanfA                                sscanf
	#define Vpi_SWPrintf								 swprintf
#else
	/* NOT _GNUC_ CODE */
	#if VPI_USE_SAFE_FUNCTIONS
	#define Vpi_StrnCpyA(xDst, xDstSize, xSrc, xCount) strncpy_s(xDst, xDstSize, xSrc, xCount)
	#define Vpi_StrnCatA(xDst, xDstSize, xSrc, xCount) strncat_s(xDst, xDstSize, xSrc, xCount)
	#define Vpi_SPrintfA                               sprintf_s
	#define Vpi_SnPrintfA                              _snprintf_s
	#define Vpi_SScanfA                                sscanf_s
	#else
	#define Vpi_StrnCpyA(xDst, xDstSize, xSrc, xCount) strncpy(xDst, xSrc, xCount)
	#define Vpi_StrnCatA(xDst, xDstSize, xSrc, xCount) strncat(xDst, xSrc, xCount)
	#define Vpi_SPrintfA                               sprintf
	#define Vpi_SnPrintfA                              _snprintf
	#define Vpi_SScanfA                                sscanf
	#define Vpi_SScanfA                                sscanf
#ifndef _WIN32_WCE
	#define Vpi_SWPrintf								swprintf
#endif
	#endif
#endif

/* import prototype for direct mapping on sscanf for files which are not allowed to include
   system headers */
#ifndef _INC_STDIO
VPI_IMPORT Vpi_Int sscanf(const Vpi_CharA* buffer, const Vpi_CharA* format, ... );
#endif /* _INC_STDIO */

/* shortcuts to Vpi_String functions */
#define Vpi_StrLen(xStr)                              Vpi_String_StrLen(xStr)
/* DLL and function handling*/
#if defined(WIN32) || defined(_WIN32_WCE)
	#define Vpi_GetProcAddress							GetProcAddress
#else
	#ifdef _GNUC_
		#define Vpi_GetProcAddress								dlsym
	#endif
#endif
#ifdef _WIN32_WCE
	#define Vpi_Clock										GetTickCount
#else
	#define	Vpi_Clock										clock
#endif

VPI_END_EXTERN_C

#endif /* _Vpi_PlatformDefs_H_ */
