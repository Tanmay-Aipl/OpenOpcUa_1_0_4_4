//**************************************************************************
//
//  Copyright (c) 4CE Industry 1999-2014, All Rights Reserved
//
//**************************************************************************
//
//  Filename   :  VpiNodeId.cpp
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
#include "VpiNodeId.h"
using namespace VpiBuiltinType;
/*============================================================================
* Vpi_NodeId_Initialize
*===========================================================================*/
Vpi_Void VPI_DLLCALL Vpi_NodeId_Initialize(Vpi_NodeId* a_pValue)
{
	if (a_pValue == Vpi_Null)
	{
		return;
	}

	a_pValue->IdentifierType = Vpi_IdentifierType_Numeric;
	a_pValue->Identifier.Numeric = 0;
	a_pValue->NamespaceIndex = 0;
}

/*============================================================================
* Vpi_NodeId_Clear
*===========================================================================*/
Vpi_Void VPI_DLLCALL Vpi_NodeId_Clear(Vpi_NodeId* a_pValue)
{
	if (a_pValue == Vpi_Null)
	{
		return;
	}

	switch (a_pValue->IdentifierType)
	{
	default:
	case Vpi_IdentifierType_Numeric:
	{
		break;
	}

	case Vpi_IdentifierType_String:
	{
		Vpi_String_Clear(&(a_pValue->Identifier.String));
		break;
	}

	case Vpi_IdentifierType_Guid:
	{
		Vpi_Free(a_pValue->Identifier.Guid);
		break;
	}

	case Vpi_IdentifierType_Opaque:
	{
		Vpi_ByteString_Clear(&a_pValue->Identifier.ByteString);
		break;
	}
	}

	a_pValue->IdentifierType = Vpi_IdentifierType_Numeric;
	a_pValue->Identifier.Numeric = 0;
	a_pValue->NamespaceIndex = 0;
}

/*============================================================================
* Vpi_NodeId_CopyTo
*===========================================================================*/
Vpi_StatusCode VPI_DLLCALL Vpi_NodeId_CopyTo(const Vpi_NodeId* a_pSource, Vpi_NodeId* a_pDestination)
{
	Vpi_StatusCode uStatus = Vpi_Good;
	if (a_pDestination)
	{
		if (a_pSource)
		{

			Vpi_NodeId_Initialize(a_pDestination);

			a_pDestination->IdentifierType = a_pSource->IdentifierType;
			a_pDestination->NamespaceIndex = a_pSource->NamespaceIndex;

			switch (a_pSource->IdentifierType)
			{
				case Vpi_IdentifierType_Numeric:
				{
					a_pDestination->Identifier.Numeric = a_pSource->Identifier.Numeric;
					break;
				}
				case Vpi_IdentifierType_String:
				{
					Vpi_String_Initialize(&(a_pDestination->Identifier.String));
					Vpi_String_StrnCpy(&a_pDestination->Identifier.String, &a_pSource->Identifier.String, VPI_STRING_LENDONTCARE);
					break;
				}
				case Vpi_IdentifierType_Opaque:
				{
					a_pDestination->Identifier.ByteString.Length = a_pSource->Identifier.ByteString.Length;

					if (a_pDestination->Identifier.ByteString.Length > 0)
					{
						a_pDestination->Identifier.ByteString.Data = (Vpi_Byte*)Vpi_Alloc(a_pSource->Identifier.ByteString.Length);
						if (a_pDestination->Identifier.ByteString.Data)
							Vpi_MemCpy(a_pDestination->Identifier.ByteString.Data, a_pDestination->Identifier.ByteString.Length, a_pSource->Identifier.ByteString.Data, a_pSource->Identifier.ByteString.Length);
						else
							uStatus = Vpi_BadOutOfMemory;
					}
					else
					{
						a_pDestination->Identifier.ByteString.Data = Vpi_Null;
					}
					break;
				}
				case Vpi_IdentifierType_Guid:
				{
					if (a_pSource->Identifier.Guid != Vpi_Null)
					{
						a_pDestination->Identifier.Guid = (Vpi_Guid*)Vpi_Alloc(sizeof(Vpi_Guid));
						if (a_pDestination->Identifier.Guid)
							Vpi_Guid_CopyTo(a_pSource->Identifier.Guid, &(a_pDestination->Identifier.Guid));
						else
							uStatus = Vpi_BadOutOfMemory;
					}
					else
					{
						a_pDestination->Identifier.Guid = Vpi_Null;
					}
					break;
				}
				default:
				{
					uStatus= Vpi_BadInvalidArgument;
				}
			}

		}
		else
			uStatus = Vpi_BadInvalidArgument;
		if (uStatus!=Vpi_Good)
			Vpi_NodeId_Clear(a_pDestination);
	}
	else
		uStatus = Vpi_BadInvalidArgument;
	return uStatus;
}