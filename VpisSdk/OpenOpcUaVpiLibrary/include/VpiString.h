//**************************************************************************
//
//  Copyright (c) 4CE Industry 1999-2014, All Rights Reserved
//
//**************************************************************************
//
//  Filename   :  AddItemDlg.cpp
//  $Author    :  Michel Condemine
//
//  Description:  This file is part of the OpenOpcUaLibrary. 
//                This compoment and all related file are not Open Source
//				  This must be use with the autorisation of Michel Condemine
//**************************************************************************
#ifndef VPISTRING
#define VPISTRING
#define VPI_STRINGLENZEROTERMINATED   0xffffffffL
#define VPI_STRING_LENDONTCARE        VPI_STRINGLENZEROTERMINATED
#define Vpi_uiMagic       0

extern "C"
{
/////////////////////////////////////////////////////////////////////////////
typedef struct _Vpi_StringInternal
{
	Vpi_UInt          uMagic          : 8;    /* ==0x00 -> Ua String, != 0x00 -> C-String, an empty string result in a null pointer! */
	Vpi_UInt          bFreeSecondMem  : 1;    /* strContent is a Pointer to the string */
	Vpi_UInt          uReserved       : 7;
	Vpi_UInt32        uLength;                /* Length without terminating '\0' */
	Vpi_CharA*        strContent;             /* Pointer or start of the string or 4 first unsignend chars (mind bFreeSecondMem) */
} Vpi_StringInternal, *Vpi_pStringInternal;
/////////////////////////////////////////////////////////////////////////////
#define Vpi_String_Compare(xValue1, xValue2) Vpi_String_StrnCmp(xValue1, xValue2, VPI_STRING_LENDONTCARE, Vpi_False)
#define Vpi_String_CopyTo(xSource, xDestination) Vpi_String_StrnCpy(xDestination, xSource, VPI_STRING_LENDONTCARE)

	VPILIBRARY_EXPORT
		VpiBuiltinType::Vpi_StatusCode VPI_DLLCALL Vpi_String_Initialize(Vpi_String* pString);

	VPILIBRARY_EXPORT
		VpiBuiltinType::Vpi_StatusCode VPI_DLLCALL Vpi_String_Clear(Vpi_String* a_pString);
	VPILIBRARY_EXPORT
		VpiBuiltinType::Vpi_StatusCode VPI_DLLCALL Vpi_String_StrnCpy(Vpi_String*       pDestString,
		const Vpi_String* pSrcString,
		Vpi_UInt32        uLength);
	VPILIBRARY_EXPORT
		Vpi_Int32 VPI_DLLCALL Vpi_String_StrnCmp(const Vpi_String* pString1,
		const Vpi_String* pString2,
		Vpi_UInt32        uLength,
		Vpi_Boolean       bIgnoreCase);
	//VPILIBRARY_EXPORT
	VpiBuiltinType::Vpi_StatusCode   VPI_DLLCALL  Vpi_String_AttachToString(Vpi_StringA strSource,
		Vpi_UInt32  uLength,
		Vpi_UInt32  uBufferSize,
		Vpi_Boolean bDoCopy,
		Vpi_Boolean bFreeOnClear,
		Vpi_String* pString);
	VPILIBRARY_EXPORT
		VpiBuiltinType::Vpi_StatusCode VPI_DLLCALL Vpi_String_AttachCopy(Vpi_String* a_pDst, const Vpi_CharA* a_pSrc);

	VPILIBRARY_EXPORT
		Vpi_UInt32 VPI_DLLCALL Vpi_String_StrLen(const Vpi_String* pString);

	//VPILIBRARY_EXPORT
	Vpi_UInt32 VPI_DLLCALL Vpi_String_StrSize(const Vpi_String* pString);

	VPILIBRARY_EXPORT
		Vpi_CharA* VPI_DLLCALL Vpi_String_GetRawString(const Vpi_String* pString);

	VPILIBRARY_EXPORT
		Vpi_Boolean VPI_DLLCALL Vpi_String_IsNull(const Vpi_String* pString);

	VPILIBRARY_EXPORT
		VpiBuiltinType::Vpi_StatusCode VPI_DLLCALL Vpi_String_WtoA(Vpi_CharW* wStrIn, Vpi_CharA** aStrOut);

	VPILIBRARY_EXPORT
		VpiBuiltinType::Vpi_StatusCode VPI_DLLCALL Vpi_String_AtoW(const Vpi_CharA* aStrIn, Vpi_CharW** wStrOut);
	////////////////////////////////////////////////////////////////
	// Internal function
#define _Vpi_String_GetRawString(x)   ( (((Vpi_StringA)x)[0]=='\0')?(Vpi_StringA)(((Vpi_pStringInternal)x)->strContent):((Vpi_StringA)x))
	Vpi_Boolean _Vpi_IsUaString(const Vpi_Void* a_strCString);
}

#endif