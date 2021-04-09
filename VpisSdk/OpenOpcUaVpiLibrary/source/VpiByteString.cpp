//**************************************************************************
//
//  Copyright (c) 4CE Industry 1999-2014, All Rights Reserved
//
//**************************************************************************
//
//  Filename   :  VpiByteString.cpp
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
/*============================================================================
* Vpi_ByteString_Initialize
*===========================================================================*/
using namespace VpiBuiltinType;
Vpi_Void VPI_DLLCALL Vpi_ByteString_Initialize(Vpi_ByteString* a_pValue)
{
	if (a_pValue == Vpi_Null)
	{
		return;
	}

	a_pValue->Data = Vpi_Null;
	a_pValue->Length = -1;
}

/*============================================================================
* Vpi_ByteString_Clear
*===========================================================================*/
Vpi_Void VPI_DLLCALL Vpi_ByteString_Clear(Vpi_ByteString* a_pValue)
{
	if (a_pValue == Vpi_Null)
	{
		return;
	}

	Vpi_Free(a_pValue->Data);
	a_pValue->Data = Vpi_Null;
	a_pValue->Length = -1;
}

/*============================================================================
* Vpi_ByteString_CopyTo
*===========================================================================*/
//Vpi_StatusCode Vpi_ByteString_CopyTo(const Vpi_ByteString* a_pSource, Vpi_ByteString* a_pDestination)
//{
//	Vpi_StatusCode uStatus=Vpi_Good;
//
//	a_pDestination->Length = a_pSource->Length;
//
//	if (a_pDestination->Length > 0)
//	{
//		a_pDestination->Data = (Vpi_Byte*)Vpi_Alloc(a_pDestination->Length);
//		if (a_pDestination->Data)
//			Vpi_MemCpy(a_pDestination->Data, a_pDestination->Length, a_pSource->Data, a_pSource->Length);
//		else
//		{
//			uStatus = Vpi_BadOutOfMemory;
//			Vpi_ByteString_Clear(a_pDestination);
//		}
//	}
//	else
//		Vpi_ByteString_Clear(a_pDestination);
//
//	return uStatus;
//}

/*============================================================================
* Vpi_ByteString_CopyTo
*===========================================================================*/
Vpi_StatusCode VPI_DLLCALL Vpi_ByteString_CopyTo(const Vpi_ByteString* a_pSource, Vpi_ByteString* a_pDestination)
{
	Vpi_StatusCode uStatus = Vpi_Good;
	if (a_pDestination)
	{
		a_pDestination->Length = a_pSource->Length;

		if (a_pDestination->Length > 0)
		{
			a_pDestination->Data = (Vpi_Byte*)Vpi_Alloc(a_pDestination->Length);
			if (a_pDestination->Data)
				Vpi_MemCpy(a_pDestination->Data, a_pDestination->Length, a_pSource->Data, a_pSource->Length);
			else
				uStatus = Vpi_BadOutOfMemory;
		}

	}
	else
		uStatus = Vpi_BadInvalidArgument;
	if (uStatus!=Vpi_Good)	
		Vpi_ByteString_Clear(a_pDestination);

	return uStatus;
}