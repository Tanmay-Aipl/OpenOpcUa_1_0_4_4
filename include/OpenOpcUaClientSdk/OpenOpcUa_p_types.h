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

#ifndef _OpcUa_P_Types_H_
#define _OpcUa_P_Types_H_ 1

#define OPCUA_P_NATIVE64 1

	/*============================================================================
	* Type definitions for basic data types.
	*===========================================================================*/

#ifndef __x86_64__
	typedef int                 OpcUa_Int;
	typedef unsigned int        OpcUa_UInt;
	typedef void                OpcUa_Void;
	typedef void*               OpcUa_Handle;
	typedef unsigned char       OpcUa_Boolean;
	typedef char                OpcUa_SByte;
	typedef unsigned char       OpcUa_Byte;
	typedef short               OpcUa_Int16;
	typedef unsigned short      OpcUa_UInt16;
	typedef long                OpcUa_Int32;
	typedef unsigned long       OpcUa_UInt32;
	typedef float               OpcUa_Float;
	typedef double              OpcUa_Double;
	typedef char                OpcUa_CharA;
	typedef unsigned char       OpcUa_UCharA;
	typedef OpcUa_CharA*        OpcUa_StringA;
	typedef unsigned short      OpcUa_Char;
#else
	typedef int                 OpcUa_Int;
	typedef unsigned int        OpcUa_UInt;
	typedef void                OpcUa_Void;
	typedef void*               OpcUa_Handle;
	typedef unsigned char       OpcUa_Boolean;
	typedef char                OpcUa_SByte;
	typedef unsigned char       OpcUa_Byte;
	typedef short               OpcUa_Int16;
	typedef unsigned short      OpcUa_UInt16;
	typedef int                 OpcUa_Int32;
	typedef unsigned int        OpcUa_UInt32;
	typedef unsigned long		OpcUa_ULong;
	typedef float               OpcUa_Float;
	typedef double              OpcUa_Double;
	typedef char                OpcUa_CharA;
	typedef unsigned char       OpcUa_UCharA;
	typedef OpcUa_CharA*        OpcUa_StringA;
	typedef unsigned short      OpcUa_Char;
#endif

#if OPCUA_P_NATIVE64
	/**
	 * @brief Convert OpcUa_DateTime to OpcUa_Int64. (i64 = OpcUa_DateTime_ToInt64(xDT))
	 */
#define OpcUa_DateTime_ToInt64(xDT)     (*((OpcUa_Int64*)&xDT))

	/**
	 * @brief Convert OpcUa_Int64 to OpcUa_DateTime. (DT = OpcUa_DateTime_FromInt64(x64))
	 */
#define OpcUa_DateTime_FromInt64(x64)   *((OpcUa_DateTime*)&x64)

#ifdef _MSC_VER
	typedef __int64             OpcUa_Int64;
	typedef unsigned __int64    OpcUa_UInt64;
#else /* _MSC_VER */
	typedef long long           OpcUa_Int64;
	typedef unsigned long long  OpcUa_UInt64;
#endif /* _MSC_VER */
#else
	struct _OpcUa_Int64 {
		OpcUa_UInt32 dwLowQuad;
		OpcUa_UInt32 dwHighQuad;
	};
	typedef struct _OpcUa_Int64 OpcUa_Int64;
	struct _OpcUa_UInt64 {
		OpcUa_UInt32 dwLowQuad;
		OpcUa_UInt32 dwHighQuad;
	};
	typedef struct _OpcUa_UInt64 OpcUa_UInt64;
#endif

	struct _OpcUa_DateTime
	{
		OpcUa_UInt32 dwLowDateTime;
		OpcUa_UInt32 dwHighDateTime;
	};
	typedef struct _OpcUa_DateTime OpcUa_DateTime;



	/**
	 * @brief OpcUa_SocketManager Type
	 */
	typedef OpcUa_Void*     OpcUa_SocketManager;

	/**
	 * @brief OpcUa_Socket Type
	 */
	typedef OpcUa_Void*     OpcUa_Socket;

	/**
	 * @brief OpcUa_Thread Type
	 */
	typedef OpcUa_Void*     OpcUa_Thread;


	/**
	 * @brief Internally used thread main entry function.
	 */
	typedef OpcUa_Void(OpcUa_PfnInternalThreadMain)(OpcUa_Void* pArguments);

	/**
	 * @brief The handle for the platform thread.
	 */
#ifndef __x86_64__
	typedef OpcUa_UInt32    OpcUa_StatusCode;
#else
	typedef OpcUa_ULong    OpcUa_StatusCode;
#endif

	/**
	 * @brief The handle for the mutex.
	 */
	typedef OpcUa_Void*     OpcUa_Mutex;

	/**
	 * @brief The handle for the semaphore.
	 */
	typedef OpcUa_Void*     OpcUa_Semaphore;

	/**
	 * @brief The handle for a timer.
	 */
	typedef OpcUa_Void*     OpcUa_Timer;

	/*============================================================================
	* Type definitions for data types on the wire.
	*===========================================================================*/
	typedef OpcUa_Boolean       OpcUa_Boolean_Wire;
	typedef OpcUa_SByte         OpcUa_SByte_Wire;
	typedef OpcUa_Byte          OpcUa_Byte_Wire;
	typedef OpcUa_Int16         OpcUa_Int16_Wire;
	typedef OpcUa_UInt16        OpcUa_UInt16_Wire;
	typedef OpcUa_Int32         OpcUa_Int32_Wire;
	typedef OpcUa_UInt32        OpcUa_UInt32_Wire;
	typedef OpcUa_Int64         OpcUa_Int64_Wire;
	typedef OpcUa_UInt64        OpcUa_UInt64_Wire;
	typedef OpcUa_Float         OpcUa_Float_Wire;
	typedef OpcUa_Double        OpcUa_Double_Wire;
	typedef OpcUa_CharA         OpcUa_Char_Wire;
	typedef OpcUa_Char_Wire*    OpcUa_String_Wire;
	typedef OpcUa_DateTime      OpcUa_DateTime_Wire;

	/*============================================================================
	* Type definitions for structured data types.
	*===========================================================================*/
#define OPCUA_GUID_STATICINITIALIZER {0, 0, 0, {0,0,0,0,0,0,0,0}}
	typedef struct _OpcUa_Guid
	{
		OpcUa_UInt32    Data1;
		OpcUa_UInt16    Data2;
		OpcUa_UInt16    Data3;
		OpcUa_UCharA    Data4[8];
	} OpcUa_Guid, *OpcUa_pGuid, OpcUa_Guid_Wire, *pOpcUa_Guid_Wire;

#ifdef _DEBUG
	typedef struct _OpcUa_String
	{
		OpcUa_UInt16 flags;
		OpcUa_UInt32 uLength;
		OpcUa_CharA* strContent;
	} OpcUa_String, *OpcUa_pString;
#else //DEBUG
	typedef struct _OpcUa_String
	{
		OpcUa_UInt16        uReserved1;     /* Content is private to String Implementation */
		OpcUa_UInt32        uReserved2;     /* Content is private to String Implementation */
		OpcUa_Void*         uReserved4;     /* Content is private to String Implementation */
	} OpcUa_String, *OpcUa_pString;
#endif
	OPCUA_BEGIN_EXTERN_C
	OpcUa_StatusCode OpcUa_String_AttachCopy(OpcUa_String* pDst, const OpcUa_CharA* pSrc);
	OpcUa_CharA* OpcUa_String_GetRawString(const OpcUa_String* pString);
#define OpcUa_String_CopyTo(xSource, xDestination) OpcUa_String_StrnCpy(xDestination, xSource, OPCUA_STRING_LENDONTCARE)
	OpcUa_StatusCode OpcUa_String_Initialize( /*  bi */ OpcUa_String* pString);
	OpcUa_Void OpcUa_String_Clear(OpcUa_String* pString);
	OpcUa_StatusCode OpcUa_String_StrnCpy(OpcUa_String*       pDestString,
		const OpcUa_String* pSrcString,
		OpcUa_UInt32        uLength);
	OpcUa_UInt32 OpcUa_String_StrLen(const OpcUa_String* pString);
	OpcUa_Int32 OpcUa_String_StrnCmp(const OpcUa_String* pString1,
		const OpcUa_String* pString2,
		OpcUa_UInt32        uLength,
		OpcUa_Boolean       bIgnoreCase);
#define OPCUA_STRINGLENZEROTERMINATED   0xffffffffL
#define OPCUA_STRING_LENDONTCARE        OPCUA_STRINGLENZEROTERMINATED
#define OpcUa_String_Compare(xValue1, xValue2) OpcUa_String_StrnCmp(xValue1, xValue2, OPCUA_STRING_LENDONTCARE, OpcUa_False)
	OPCUA_END_EXTERN_C
	typedef struct _OpcUa_ByteString
	{
		OpcUa_Int32 Length;
		OpcUa_Byte* Data;
	} OpcUa_ByteString;


	/**
	* @brief Holds a time value with a maximum resolution of micro seconds.
	*/
	typedef struct _OpcUa_TimeVal OpcUa_TimeVal;

	struct _OpcUa_TimeVal
	{
		/** @brief The number of full seconds since 1970. */
		OpcUa_UInt32 uintSeconds;
		/** @brief The fraction of the last second. */
		OpcUa_UInt32 uintMicroSeconds;
	};

	OPCUA_BEGIN_EXTERN_C
		OpcUa_Void OpcUa_Thread_Sleep(OpcUa_UInt32    msecTimeout);
	OPCUA_END_EXTERN_C
		/*============================================================================
		* constant definitions.
		*===========================================================================*/
#define OpcUa_Ignore        0           /* Ignore signal */

#define OpcUa_False         0
#define OpcUa_True          (!OpcUa_False)

#ifdef __cplusplus
#define OpcUa_Null           0
#else
#define OpcUa_Null          (OpcUa_Void*)0
#endif

#define OpcUa_SByte_Min     (OpcUa_SByte)-128
#define OpcUa_SByte_Max     (OpcUa_SByte)127
#define OpcUa_Byte_Min      (OpcUa_Byte)0
#define OpcUa_Byte_Max      (OpcUa_Byte)255
#define OpcUa_Int16_Min     (OpcUa_Int16)-32768
#define OpcUa_Int16_Max     (OpcUa_Int16)32767
#define OpcUa_UInt16_Min    (OpcUa_UInt16)0
#define OpcUa_UInt16_Max    (OpcUa_UInt16)65535
#define OpcUa_Int32_Min     (OpcUa_Int32)(-2147483647L-1)
#define OpcUa_Int32_Max     (OpcUa_Int32)2147483647L
#define OpcUa_UInt32_Min    (OpcUa_UInt32)0UL
#define OpcUa_UInt32_Max    (OpcUa_UInt32)4294967295UL
#define OpcUa_Int64_Min     (OpcUa_Int64)(-9223372036854775807i64-1)
#define OpcUa_Int64_Max     (OpcUa_Int64)9223372036854775807i64
#define OpcUa_UInt64_Min    (OpcUa_UInt64)0
#define OpcUa_UInt64_Max    (OpcUa_UInt64)18446744073709551615i64
		/* defined as FLT_MIN in "%ProgramFiles\Microsoft Visual Studio 8\VC\include\float.h" */
		/* #define FLT_MIN         1.175494351e-38F */
#define OpcUa_Float_Min     (OpcUa_Float)1.175494351e-38F
		/* defined as FLT_MAX in "%ProgramFiles\Microsoft Visual Studio 8\VC\include\float.h" */
		/* #define FLT_MAX         3.402823466e+38F */
#define OpcUa_Float_Max     (OpcUa_Float)3.402823466e+38F
		/* defined as DBL_MIN in "%ProgramFiles\Microsoft Visual Studio 8\VC\include\float.h" */
		/* #define DBL_MIN         2.2250738585072014e-308 */
#define OpcUa_Double_Min    (OpcUa_Double)2.2250738585072014e-308
		/* defined as DBL_MAX in "%ProgramFiles\Microsoft Visual Studio 8\VC\include\float.h" */
		/* #define DBL_MAX         1.7976931348623158e+308 */
#define OpcUa_Double_Max    (OpcUa_Double)1.7976931348623158e+308

#define OpcUa_DateTime_Min  0
#define OpcUa_DateTime_Max  3155378975999999999

		/* set to OPCUA_CONFIG_YES to use the untested optimized byteswap */
#define OPCUA_SWAP_ALTERNATIVE OPCUA_CONFIG_NO

#if OPCUA_SWAP_ALTERNATIVE

#ifdef _LITTLE_ENDIAN

		/* this is the wire format */

#define OpcUa_SwapBytes_2(xDst, xSrc) \
	    { \
        *xDst = *xSrc; \
	    }

#define OpcUa_SwapBytes_4(xDst, xSrc) \
	    { \
        *xDst = *xSrc; \
	    }

#define OpcUa_SwapBytes_8(xDst, xSrc) \
	    { \
        *xDst = *xSrc; \
	    }

#else

#define OpcUa_SwapBytes_2(xDst, xSrc) \
	    { \
        ((unsigned char*)xDst)[0] = ((unsigned char*)xSrc)[1]; \
        ((unsigned char*)xDst)[1] = ((unsigned char*)xSrc)[0]; \
	    }

#define OpcUa_SwapBytes_4(xDst, xSrc) \
	    { \
        ((unsigned char*)xDst)[0] = ((unsigned char*)xSrc)[3]; \
        ((unsigned char*)xDst)[1] = ((unsigned char*)xSrc)[2]; \
        ((unsigned char*)xDst)[2] = ((unsigned char*)xSrc)[1]; \
        ((unsigned char*)xDst)[3] = ((unsigned char*)xSrc)[0]; \
	    }

#define OpcUa_SwapBytes_8(xDst, xSrc) \
	    { \
        ((unsigned char*)xDst)[0] = ((unsigned char*)xSrc)[7]; \
        ((unsigned char*)xDst)[1] = ((unsigned char*)xSrc)[6]; \
        ((unsigned char*)xDst)[2] = ((unsigned char*)xSrc)[5]; \
        ((unsigned char*)xDst)[3] = ((unsigned char*)xSrc)[4]; \
        ((unsigned char*)xDst)[4] = ((unsigned char*)xSrc)[3]; \
        ((unsigned char*)xDst)[5] = ((unsigned char*)xSrc)[2]; \
        ((unsigned char*)xDst)[6] = ((unsigned char*)xSrc)[1]; \
        ((unsigned char*)xDst)[7] = ((unsigned char*)xSrc)[0]; \
	    }

#endif

#else /* OPCUA_SWAP_ALTERNATIVE */

#ifdef _LITTLE_ENDIAN
#define OpcUa_SwapBytes(xDst, xSrc, xCount) \
	    { \
        memcpy(xDst, xSrc, xCount); \
	    }
#else
#define OpcUa_SwapBytes(xDst, xSrc, xCount) \
	    { \
        OpcUa_UInt32 ii = 0; \
        OpcUa_UInt32 jj = xCount-1; \
        OpcUa_Byte* dst = (OpcUa_Byte*)xDst; \
        OpcUa_Byte* src = (OpcUa_Byte*)xSrc; \
        \
        for (; ii < xCount; ii++, jj--) \
		        { \
            dst[ii] = src[jj]; \
		        } \
	    }
#endif

#endif /* OPCUA_SWAP_ALTERNATIVE */

#endif /* _OpcUa_PlatformDefs_H_ */
/*----------------------------------------------------------------------------------------------------*\
|   End of File                                                                          End of File   |
\*----------------------------------------------------------------------------------------------------*/
