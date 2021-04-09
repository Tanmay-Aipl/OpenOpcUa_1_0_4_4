//**************************************************************************
//
//  Copyright (c) 4CE Industry 1999-2014, All Rights Reserved
//
//**************************************************************************
//
//  Filename   :  VpiString.cpp
//  $Author    :  Michel Condemine
//
//  Description:  This file is part of the OpenOpcUaLibrary. 
//                This compoment and all related file are not Open Source
//				  This must be use with the autorisation of Michel Condemine
//**************************************************************************
/*
 *
 * Permission is hereby granted, for a commerciale use of this 
 * software and associated documentation files (the "Software")
 *
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
 * ======================================================================*/
#include "stdafx.h"
using namespace VpiBuiltinType;
Vpi_Boolean _Vpi_IsUaString(const Vpi_Void* a_strCString)
{
	Vpi_Boolean bResult=Vpi_False;
	if(a_strCString)
	{
		Vpi_StringA pBuf=(Vpi_StringA)a_strCString;
		if(pBuf[0]==0x00)
			bResult = Vpi_True;
	}
	return bResult;
}
Vpi_StatusCode VPI_DLLCALL Vpi_String_Clear(Vpi_String* a_pString)
{
	Vpi_StatusCode uStatus=Vpi_Good;
	Vpi_pStringInternal pStringInt = (Vpi_pStringInternal) a_pString;

	if(_Vpi_IsUaString( (Vpi_StringA) a_pString) == Vpi_False)
	{
		uStatus= Vpi_BadInvalidArgument;;
	}
	else
	{
		if(pStringInt->bFreeSecondMem != Vpi_False)
		{
			free((Vpi_Void*)(pStringInt->strContent));

	#ifdef VPI_STRING_PARANOID_MEMORY
			pStringInt->strContent = Vpi_Null;
	#endif /* VPI_STRING_PARANOID_MEMORY */
		}

		Vpi_String_Initialize(a_pString);
	}
	return uStatus;
}

Vpi_StatusCode VPI_DLLCALL Vpi_String_StrnCpy( Vpi_String*       a_pDestString,
									   const Vpi_String* a_pSrcString,
									   Vpi_UInt32        a_uLength) 
{
	Vpi_StringA           strRawSrc   = Vpi_Null;
	Vpi_UInt32            uiSrcLen    = 0;
	Vpi_StringInternal*   pStringInt  = (Vpi_StringInternal*)a_pSrcString;

	Vpi_StatusCode        uStatus     = Vpi_Good;

	if (a_pDestString)
	{
		/* check src string */

		Vpi_String_Clear(a_pDestString);

		if(Vpi_String_IsNull(a_pSrcString))
		{
			return Vpi_Good;
		}   

		strRawSrc  = Vpi_String_GetRawString(a_pSrcString);

		if(a_uLength == VPI_STRING_LENDONTCARE)
		{
			/* size in bytes */
			uiSrcLen  = Vpi_String_StrSize(a_pSrcString);
		}
		else
		{
			/* min of given maximum number of bytes and the real length */
			uiSrcLen  = (a_uLength < pStringInt->uLength)?a_uLength:pStringInt->uLength;
		}

		uStatus = Vpi_String_AttachToString(  strRawSrc, 
												uiSrcLen, 
												0, 
												Vpi_True, 
												Vpi_True, /* since we copy, this is irrelevant... */
												a_pDestString);
	}
	else
		uStatus=Vpi_BadInvalidArgument;
	return uStatus;
}


Vpi_StatusCode VPI_DLLCALL Vpi_String_Initialize( /*  bi */ Vpi_String* a_pString )
{
	Vpi_pStringInternal   pStringInt  = (Vpi_pStringInternal)   a_pString;

	pStringInt->uMagic          = Vpi_uiMagic;
	pStringInt->bFreeSecondMem  = Vpi_True;
	/*pStringInt->uReserved       = 0;*/
	pStringInt->uLength         = 0;
	pStringInt->strContent      = Vpi_Null;

	return Vpi_Good;
}


/*============================================================================
* Get number of characters.
*===========================================================================*/
Vpi_UInt32 VPI_DLLCALL Vpi_String_StrLen(const Vpi_String*  a_pString)
{
	Vpi_UInt32            uRawLen         = 0;
	Vpi_StringA           strRawString    = Vpi_Null;
	Vpi_UInt32            uCount          = 0;
	Vpi_UInt32            uiOctetCount    = 0;
	Vpi_UInt32            uLen            = 0;
	Vpi_Byte              byMask          = 0;

	static const Vpi_UCharA   byUTF8Mask[6] = {   0x00,                  /* Mask for 1 Byte UTF 0x0xxxxxxx */
													0xC0,                  /* Mask for 2 Byte UTF 0x110xxxxx */
													0xE0,                  /* Mask for 3 Byte UTF 0x1110xxxx */
													0xF0,                  /* Mask for 4 Byte UTF 0x11110xxx */
													0xF8,                  /* Mask for 5 Byte UTF 0x111110xx */
													0xFC                   /* Mask for 6 Byte UTF 0x1111110x */
												};

	if(a_pString)
	{
		strRawString = Vpi_String_GetRawString(a_pString);

		if(strRawString)
		{	
			uRawLen = Vpi_String_StrSize(a_pString);
			uLen = uRawLen;
			// This below calculate the len for an UTF8 string. The row len is better
			//for(uCount = 0; uCount < uRawLen; uCount++)
			//{
			//	if ((strRawString[uCount]&0x80) !=0 )
			//	{
			//		uiOctetCount = 7;  /* Maximum BYTES of UTF Char ist 6, we ne 1 one, cause we make a -1 in the next step. */
			//		do
			//		{
			//			uiOctetCount--;
			//			byMask = (Vpi_Byte) ((0xff >> (8 - uiOctetCount)) << (8 - uiOctetCount));
			//		} while ((uiOctetCount>1) && ((strRawString[uCount] & byMask) != byUTF8Mask[uiOctetCount-1]));
			//		if (uiOctetCount==1)
			//		{
			//			/* Todo Vpi_BadSyntaxError might be better, but is not yet defined */
			//			/* Vpi_BadInvalidArgument; */
			//		}
			//		uCount+=uiOctetCount-1;
			//	}
			//	uLen++;
			//}
		}
	}
	return uLen;
}

/// <summary>
/// Get number of bytes of the Vpi_String.
/// </summary>
/// <param name="a_pString">The a_pString.</param>
/// <returns>Get number of bytes</returns>
Vpi_UInt32 VPI_DLLCALL Vpi_String_StrSize(const Vpi_String* a_pString)
{
	Vpi_UInt32          uLen       = 0;
	Vpi_pStringInternal pStringInt = (Vpi_pStringInternal)a_pString;

	if(pStringInt)
	{      
		if( _Vpi_IsUaString(a_pString) != Vpi_False)
		{
			if(pStringInt->strContent)
				uLen = pStringInt->uLength;
		}
		if (a_pString->strContent)
			uLen = (Vpi_Int32) strlen((Vpi_StringA)a_pString->strContent);
	}
	
	return uLen; 
}
/*============================================================================
* Get pointer to internal raw string.
*===========================================================================*/
Vpi_CharA* VPI_DLLCALL Vpi_String_GetRawString(const Vpi_String* a_pString)
{
	return _Vpi_String_GetRawString(a_pString);
}
/*============================================================================
* 
*===========================================================================*/
Vpi_StatusCode VPI_DLLCALL Vpi_String_AttachToString(  /* in */ Vpi_StringA              a_strSource,
											   /* in */ Vpi_UInt32               a_uLength,
											   /* in */ Vpi_UInt32               a_uBufferSize,
											   /* in */ Vpi_Boolean              a_bDoCopy,
											   /* in */ Vpi_Boolean              a_bFreeOnClear,
											   /* bi */ Vpi_String*              a_pNewString)
{
	Vpi_StatusCode uStatus=Vpi_Good;
	Vpi_pStringInternal   pStringInt = (Vpi_pStringInternal)a_pNewString;

	if (a_strSource)
	{
		if (a_pNewString)
		{
			Vpi_String_Clear(a_pNewString);

			if(a_uLength == VPI_STRINGLENZEROTERMINATED)
			{
				a_uLength = strlen(a_strSource);
			}

			pStringInt->uMagic          = Vpi_uiMagic;
			/*pStringInt->uReserved       = 0;*/
			pStringInt->bFreeSecondMem  = a_bFreeOnClear;
			pStringInt->uLength         = a_uLength;

			if(a_bDoCopy != Vpi_False)
			{
				/* attach copied string, free it on clearing */
				pStringInt->bFreeSecondMem = Vpi_True;
				pStringInt->strContent     = (Vpi_StringA)malloc(a_uLength+1);

				if (pStringInt->strContent)
				{
					memcpy( pStringInt->strContent, 
							a_strSource, 
							a_uLength);

					pStringInt->strContent[a_uLength]   = '\0';
				}
				else
					uStatus=Vpi_BadOutOfMemory;
			}
			else
			{
				/* attach external string; memory does get freed on request! */
				pStringInt->bFreeSecondMem  = a_bFreeOnClear;
				pStringInt->strContent      = a_strSource;
			}


			a_pNewString = (Vpi_String*)pStringInt;
		}
		else
			uStatus=Vpi_BadInvalidArgument; 
	}
	else
		uStatus=Vpi_BadInvalidArgument;
	return Vpi_Good;
}
/*============================================================================
* Check if string is null.
*===========================================================================*/
Vpi_Boolean VPI_DLLCALL Vpi_String_IsNull(const Vpi_String* a_pString)
{
	Vpi_Boolean bRes = Vpi_False;
	Vpi_pStringInternal pStringInt  = (Vpi_pStringInternal)a_pString;

	if(pStringInt == Vpi_Null)
		bRes = Vpi_True;
	else
	{
		if(((Vpi_StringA)pStringInt)[0] == 0x00)
		{
			if(pStringInt->strContent == Vpi_Null)
				bRes = Vpi_True;
		}
	}
	return bRes;
}

/*============================================================================
 * Vpi_String_AttachCopy
 *===========================================================================*/
Vpi_StatusCode VPI_DLLCALL Vpi_String_AttachCopy(Vpi_String* a_pDst, const Vpi_CharA* a_pSrc)
{
	Vpi_StatusCode uStatus = Vpi_String_AttachToString(
		(Vpi_StringA)a_pSrc, /* const is guaranteed by flags */
		VPI_STRINGLENZEROTERMINATED, 
		0,
		Vpi_True, /* copy the given string */
		Vpi_True, /* irrelevant due to copy; could be Vpi_False as well */
		a_pDst);

	return uStatus;
}

/*============================================================================
* Vpi_String_AtoW
*===========================================================================*/

Vpi_StatusCode VPI_DLLCALL Vpi_String_AtoW(const Vpi_CharA* aStrIn, Vpi_CharW** wStrOut)
{
	Vpi_CharW* localwStr = NULL;
	Vpi_StatusCode uStatus = Vpi_BadInvalidArgument;
	int ii;

	if (aStrIn)
	{
		int iLen = strlen((const char*)aStrIn);

		if ((*wStrOut) == Vpi_Null)
		{
			*wStrOut = (Vpi_CharW*)malloc((iLen + 1)* sizeof(Vpi_CharW));
			if ((*wStrOut))
			{
				memset(*wStrOut, 0, (iLen + 1)* sizeof(Vpi_CharW));
				localwStr = (Vpi_CharW*)malloc((iLen + 1)* sizeof(Vpi_CharW));
				if (uStatus)
				{
					memset(localwStr, 0, (iLen + 1)* sizeof(Vpi_CharW));
					for (ii = 0; ii<iLen; ii++)
					{
						localwStr[ii] = (Vpi_CharW)aStrIn[ii];
					}
					Vpi_WcsCpy((wchar_t*)*wStrOut, (wchar_t*)localwStr);
					Vpi_Free(localwStr);
					uStatus = Vpi_Good;
				}
				else
					uStatus = Vpi_BadOutOfMemory;
			}
			else
				uStatus = Vpi_BadOutOfMemory;
		}
	}
	return uStatus;
}

/*============================================================================
* Vpi_String_WtoA
*===========================================================================*/
Vpi_StatusCode VPI_DLLCALL Vpi_String_WtoA(Vpi_CharW* wStrIn, Vpi_CharA** aStrOut)
{
	char* localStr = NULL;
	Vpi_StatusCode uStatus = Vpi_BadInvalidArgument;
	int ii;

	if (wStrIn)
	{
		// sizeof the source to convert
		int iLen = wcslen((wchar_t*)wStrIn);
		// Check the we receive  a Null ptr
		if ((*aStrOut) == Vpi_Null)
		{
			*aStrOut = (Vpi_CharA *)malloc(iLen + 1);
			if (*aStrOut)
			{
				memset(*aStrOut, 0, iLen + 1);
				localStr = (char*)malloc(iLen + 1);
				if (localStr)
				{
					memset(localStr, 0, iLen + 1);
					for (ii = 0; ii<iLen; ii++)
					{
						localStr[ii] = (char)wStrIn[ii];
					}
					strcpy((char *)*aStrOut, localStr);
					Vpi_Free(localStr);
					uStatus = Vpi_Good;
				}
				else
					uStatus = Vpi_BadOutOfMemory;
			}
			else
				uStatus = Vpi_BadOutOfMemory;
		}
	}
	return uStatus;
}
/*============================================================================
* Compare two Vpi_Strings Case Sensitive
*===========================================================================*/
Vpi_Int32 VPI_DLLCALL Vpi_String_strncmp(Vpi_StringA       a_string1,
	Vpi_StringA       a_string2,
	Vpi_UInt32        a_uiLength)
{
	return (Vpi_Int32)strncmp(a_string1, a_string2, a_uiLength);
}
/*============================================================================
* Compare two Vpi_Strings
*===========================================================================*/
Vpi_Int32 VPI_DLLCALL Vpi_String_StrnCmp(const Vpi_String* a_pLeftString,
	const Vpi_String* a_pRightString,
	Vpi_UInt32        a_uLength,
	Vpi_Boolean       a_bIgnoreCase)
{
	Vpi_StringA   strRawLeft = Vpi_Null;
	Vpi_StringA   strRawRight = Vpi_Null;
	Vpi_UInt32    uiLeftLen = 0;
	Vpi_UInt32    uiRightLen = 0;
	Vpi_UInt32    uiTempLen = 0;
	Vpi_Int32     nRetVal = 0;

	/* '0' a_pLeftString identical to a_pRightString  */
	if (a_pLeftString == Vpi_Null && a_pRightString == Vpi_Null) return 0;
	/* '< 0' a_pLeftString less than a_pRightString */
	if (a_pLeftString == Vpi_Null) return -1;
	/* '> 0' a_pLeftString greater than a_pRightString */
	if (a_pRightString == Vpi_Null) return 1;

	strRawLeft = Vpi_String_GetRawString(a_pLeftString);
	strRawRight = Vpi_String_GetRawString(a_pRightString);

	/* '0' a_pLeftString identical to a_pRightString  */
	if (strRawLeft == strRawRight) return 0;
	/* '< 0' a_pLeftString less than a_pRightString */
	if (strRawLeft == Vpi_Null) return -1;
	/* '> 0' a_pLeftString greater than a_pRightString */
	if (strRawRight == Vpi_Null) return 1;

	uiLeftLen = Vpi_String_StrSize(a_pLeftString);
	uiRightLen = Vpi_String_StrSize(a_pRightString);

	/* Re-Calculate Minimum length */
	uiTempLen = (uiLeftLen > uiRightLen) ? uiRightLen : uiLeftLen;

	if (uiTempLen >= a_uLength)
	{
		uiTempLen = a_uLength;
	}
	else
	{
		/* the compare length is longer than at least one of the given strings */
		/* we can check the lengths a priori */
		if (uiLeftLen < uiRightLen) return -1;
		if (uiLeftLen > uiRightLen) return 1;
	}

	if (a_uLength == VPI_STRING_LENDONTCARE)
	{
		/* we can check the lengths a priori */
		if (uiLeftLen < uiRightLen) return -1;
		if (uiLeftLen > uiRightLen) return 1;
	}

	/* need to provide strnicmp with valid lengths, because the raw strings may not be zero terminated! */
	if (a_bIgnoreCase != Vpi_False)
	{
		nRetVal = Vpi_String_strncmp(strRawLeft, strRawRight, uiTempLen);
	}
	else
	{
		nRetVal = Vpi_String_strncmp(strRawLeft, strRawRight, uiTempLen);
	}

	return nRetVal;
}
