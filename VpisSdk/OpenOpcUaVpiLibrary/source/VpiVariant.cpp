//**************************************************************************
//
//  Copyright (c) 4CE Industry 1999-2014, All Rights Reserved
//
//**************************************************************************
//
//  Filename   :  VpiVariant.cpp
//  $Author    :  Michel Condemine
//
//  Description:  This file is part of the OpenOpcUaLibrary. 
//                This compoment and all related file are not Open Source
//		  This must be use with the autorisation of Michel Condemine
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

#include "VpiVariant.h"
using namespace VpiBuiltinType;
Vpi_StatusCode VPI_DLLCALL Vpi_Variant_Initialize(Vpi_Variant* pVariant)
{
	Vpi_StatusCode uStatus = Vpi_Good;
	if (pVariant)
		memset(pVariant, 0, sizeof(Vpi_Variant));
	else
		uStatus = Vpi_BadInvalidArgument;
	return uStatus;
}
Vpi_StatusCode VPI_DLLCALL Vpi_Variant_Clear(Vpi_Variant* pVariant)
{
	Vpi_StatusCode uStatus = Vpi_Good;

	if (pVariant)
	{
		if (pVariant->ArrayType == Vpi_VariantArrayType_Scalar)
		{
			Vpi_VariantUnion_Clear(pVariant->Datatype, &pVariant->Value);
		}
		else
		{
			if (pVariant->ArrayType == Vpi_VariantArrayType_Array)
			{
				Vpi_VariantArrayValue_Clear(
					pVariant->Datatype,
					pVariant->Value.Array.Length,
					&pVariant->Value.Array.Value);
			}
			else
			{
				Vpi_MemSet(pVariant, 0, sizeof(Vpi_Variant));
			}
		}
		pVariant->ArrayType = 0;
		pVariant->Datatype = 0;
	}
	return uStatus;
}

/*============================================================================
* Vpi_VariantUnion_Clear
*===========================================================================*/
Vpi_Void Vpi_VariantUnion_Clear(Vpi_UInt16 datatype, Vpi_VariantUnion* a_pValue)
{
	if (a_pValue == Vpi_Null)
	{
		return;
	}

	switch (datatype)
	{
		case VpiType_Null:
		case VpiType_Boolean:
		case VpiType_SByte:
		case VpiType_Byte:
		case VpiType_Int16:
		case VpiType_UInt16:
		case VpiType_Int32:
		case VpiType_UInt32:
		case VpiType_Int64:
		case VpiType_UInt64:
		case VpiType_Float:
		case VpiType_Double:
		case VpiType_DateTime:
		case VpiType_StatusCode:
		default:
		{
			break;
		}

		case VpiType_Guid:
		{
			Vpi_Free(a_pValue->Guid);
			break;
		}

		case VpiType_String:
		{
			Vpi_String_Clear(&(a_pValue->String));
			break;
		}

		case VpiType_ByteString:
		{
			Vpi_ByteString_Clear(&a_pValue->ByteString);
			break;
		}

		case VpiType_XmlElement:
		{
			Vpi_XmlElement_Clear(&a_pValue->XmlElement);
			break;
		}

		case VpiType_NodeId:
		{
			Vpi_NodeId_Clear(a_pValue->NodeId);
			Vpi_Free(a_pValue->NodeId);
			break;
		}

		case VpiType_ExpandedNodeId:
		{
			Vpi_ExpandedNodeId_Clear(a_pValue->ExpandedNodeId);
			Vpi_Free(a_pValue->ExpandedNodeId);
			break;
		}

		case VpiType_QualifiedName:
		{
			Vpi_QualifiedName_Clear(a_pValue->QualifiedName);
			Vpi_Free(a_pValue->QualifiedName);
			break;
		}

		case VpiType_LocalizedText:
		{
			Vpi_LocalizedText_Clear(a_pValue->LocalizedText);
			Vpi_Free(a_pValue->LocalizedText);
			break;
		}

		case VpiType_DataValue:
		{
			Vpi_DataValue_Clear(a_pValue->DataValue);
			Vpi_Free(a_pValue->DataValue);
			break;
		}
		// Not supported in the Vpi at the moment
		//case VpiType_ExtensionObject:
		//{
		//	Vpi_ExtensionObject_Delete(&a_pValue->ExtensionObject);
		//	break;
		//}
	}
}

/*============================================================================
* Vpi_VariantArrayValue_Clear
*===========================================================================*/
Vpi_Void Vpi_VariantArrayValue_Clear(
	Vpi_UInt16             a_uDatatype,
	Vpi_Int32              a_iLength,
	Vpi_VariantArrayUnion* a_pValue)
{
	Vpi_Int32 ii = 0;

	if (a_pValue == Vpi_Null)
	{
		return;
	}

	/* clear each element in the array */
	for (ii = 0; ii < a_iLength; ii++)
	{
		switch (a_uDatatype)
		{
		case VpiType_Null:
		case VpiType_Boolean:
		case VpiType_SByte:
		case VpiType_Byte:
		case VpiType_Int16:
		case VpiType_UInt16:
		case VpiType_Int32:
		case VpiType_UInt32:
		case VpiType_Int64:
		case VpiType_UInt64:
		case VpiType_Float:
		case VpiType_Double:
		case VpiType_DateTime:
		case VpiType_Guid:
		case VpiType_StatusCode:
		default:
		{
			break;
		}

		case VpiType_String:
		{
			Vpi_String_Clear(&(a_pValue->StringArray[ii]));
			break;
		}

		case VpiType_ByteString:
		{
			Vpi_ByteString_Clear(&a_pValue->ByteStringArray[ii]);
			break;
		}

		case VpiType_XmlElement:
		{
			Vpi_XmlElement_Clear(&a_pValue->XmlElementArray[ii]);
			break;
		}

		case VpiType_NodeId:
		{
			Vpi_NodeId_Clear(&a_pValue->NodeIdArray[ii]);
			break;
		}

		case VpiType_ExpandedNodeId:
		{
			Vpi_ExpandedNodeId_Clear(&a_pValue->ExpandedNodeIdArray[ii]);
			break;
		}

		case VpiType_QualifiedName:
		{
			Vpi_QualifiedName_Clear(&a_pValue->QualifiedNameArray[ii]);
			break;
		}

		case VpiType_LocalizedText:
		{
			Vpi_LocalizedText_Clear(&a_pValue->LocalizedTextArray[ii]);
			break;
		}
		/*
		case VpiType_ExtensionObject:
		{
			Vpi_ExtensionObject_Clear(&a_pValue->ExtensionObjectArray[ii]);
			break;
		}
		*/
		case VpiType_DataValue:
		{
			Vpi_DataValue_Clear(&a_pValue->DataValueArray[ii]);
			break;
		}

		case VpiType_Variant:
		{
			Vpi_Variant_Clear(&a_pValue->VariantArray[ii]);
			break;
		}
		}
	}

	/* free the memory allocated for the array. */
	Vpi_Free(a_pValue->Array);
}


/*============================================================================
* Vpi_Variant_CopyTo
*===========================================================================*/
Vpi_StatusCode VPI_DLLCALL Vpi_Variant_CopyTo(const Vpi_Variant* a_pSource, Vpi_Variant* a_pDestination)
{
	Vpi_StatusCode uStatus=Vpi_Good;

	if (a_pSource)
	{
		if (a_pDestination)
		{

			Vpi_Variant_Initialize(a_pDestination);

			Vpi_Field_CopyToScalar(Byte, Datatype);
			Vpi_Field_CopyToScalar(Byte, ArrayType);

			if (a_pSource->ArrayType == Vpi_VariantArrayType_Scalar)
			{
				return Vpi_VariantUnion_CopyTo(a_pSource->Datatype,
					&a_pSource->Value,
					&a_pDestination->Value);
			}
			else
			{
				if (a_pSource->ArrayType == Vpi_VariantArrayType_Array)
				{
					Vpi_Field_CopyToScalar(Int32, Value.Array.Length);

					uStatus = Vpi_VariantArrayValue_CreateArray(a_pDestination->Datatype,
						a_pDestination->Value.Array.Length,
						&a_pDestination->Value.Array.Value.Array);
					if (uStatus == Vpi_Good)
						uStatus = Vpi_VariantArrayValue_CopyTo(a_pSource->Datatype,
						a_pSource->Value.Array.Length,
						&a_pSource->Value.Array.Value,
						&a_pDestination->Value.Array.Value);
				}
			}
		}
		else
		{
			uStatus = Vpi_BadInvalidArgument;
			Vpi_Variant_Clear(a_pDestination);
		}
	}
	else
	{
		uStatus = Vpi_BadInvalidArgument;
		Vpi_Variant_Clear(a_pDestination);
	}
	return uStatus;
}
/*============================================================================
* Vpi_VariantUnion_CopyTo
*===========================================================================*/
Vpi_StatusCode Vpi_VariantUnion_CopyTo(Vpi_UInt16 a_Datatype, const Vpi_VariantUnion* a_pSource, Vpi_VariantUnion* a_pDestination)
{
	Vpi_StatusCode uStatus = Vpi_Good;

	switch (a_Datatype)
	{
		case VpiType_Boolean:
		{
			Vpi_Field_CopyToScalar(Boolean, Boolean);
			break;
		}
		case VpiType_SByte:
		{
			Vpi_Field_CopyToScalar(SByte, SByte);
			break;
		}
		case VpiType_Byte:
		{
			Vpi_Field_CopyToScalar(Byte, Byte);
			break;
		}
		case VpiType_Int16:
		{
			Vpi_Field_CopyToScalar(Int16, Int16);
			break;
		}
		case VpiType_UInt16:
		{
			Vpi_Field_CopyToScalar(UInt16, UInt16);
			break;
		}
		case VpiType_Int32:
		{
			Vpi_Field_CopyToScalar(Int32, Int32);
			break;
		}
		case VpiType_UInt32:
		{
			Vpi_Field_CopyToScalar(UInt32, UInt32);
			break;
		}
		case VpiType_Int64:
		{
			Vpi_Field_CopyToScalar(Int64, Int64);
			break;
		}
		case VpiType_UInt64:
		{
			Vpi_Field_CopyToScalar(UInt64, UInt64);
			break;
		}
		case VpiType_Float:
		{
			Vpi_Field_CopyToScalar(Float, Float);
			break;
		}
		case VpiType_Double:
		{
			Vpi_Field_CopyToScalar(Double, Double);
			break;
		}
		case VpiType_DateTime:
		{
			Vpi_Field_CopyToScalar(DateTime, DateTime);
			break;
		}
		case VpiType_StatusCode:
		{
			Vpi_Field_CopyToScalar(StatusCode, StatusCode);
			break;
		}
		case VpiType_Guid:
		{
			uStatus = Vpi_Guid_CopyTo(a_pSource->Guid, &a_pDestination->Guid);
			break;
		}

		case VpiType_String:
		{
			Vpi_Field_CopyTo(String, String);
			break;
		}

		case VpiType_ByteString:
		{
			Vpi_Field_CopyTo(ByteString, ByteString);
			break;
		}

		case VpiType_XmlElement:
		{
			Vpi_Field_CopyTo(XmlElement, XmlElement);
			break;
		}

		//case VpiType_NodeId:
		//{
		//	Vpi_Field_Copy(NodeId, NodeId);
		//	break;
		//}

		//case VpiType_ExpandedNodeId:
		//{
		//	Vpi_Field_Copy(ExpandedNodeId, ExpandedNodeId);
		//	break;
		//}

		case VpiType_QualifiedName:
		{
			Vpi_Field_Copy(QualifiedName, QualifiedName);
			break;
		}

		//case VpiType_LocalizedText:
		//{
		//	Vpi_Field_Copy(LocalizedText, LocalizedText);
		//	break;
		//}

		//case VpiType_DataValue:
		//{
		//	Vpi_Field_Copy(DataValue, DataValue);
		//	break;
		//}

		//case VpiType_ExtensionObject:
		//{
		//	Vpi_Field_Copy(ExtensionObject, ExtensionObject);
		//	break;
		//}
	}

	return uStatus;
}

/*============================================================================
* Vpi_VariantArrayValue_CopyTo
*===========================================================================*/
Vpi_StatusCode Vpi_VariantArrayValue_CopyTo(Vpi_UInt16 a_Datatype,
												Vpi_Int32                     a_iLength,
												const Vpi_VariantArrayUnion*  a_pSource,
												const Vpi_VariantArrayUnion*  a_pDestination)
{
	Vpi_Int32 ii = 0;

	Vpi_StatusCode uStatus = Vpi_Good;

	for (ii = 0; ii < a_iLength; ii++)
	{
		switch (a_Datatype)
		{
			/* Todo: Scalar -> memcpy */
			case VpiType_Boolean:
			{
				Vpi_Field_CopyToScalar(Boolean, BooleanArray[ii]);
				break;
			}
			case VpiType_SByte:
			{
				Vpi_Field_CopyToScalar(SByte, SByteArray[ii]);
				break;
			}
			case VpiType_Byte:
			{
				Vpi_Field_CopyToScalar(Byte, ByteArray[ii]);
				break;
			}
			case VpiType_Int16:
			{
				Vpi_Field_CopyToScalar(Int16, Int16Array[ii]);
				break;
			}
			case VpiType_UInt16:
			{
				Vpi_Field_CopyToScalar(UInt16, UInt16Array[ii]);
				break;
			}
			case VpiType_Int32:
			{
				Vpi_Field_CopyToScalar(Int32, Int32Array[ii]);
				break;
			}
			case VpiType_UInt32:
			{
				Vpi_Field_CopyToScalar(UInt32, UInt32Array[ii]);
				break;
			}
			case VpiType_Int64:
			{
				Vpi_Field_CopyToScalar(Int64, Int64Array[ii]);
				break;
			}
			case VpiType_UInt64:
			{
				Vpi_Field_CopyToScalar(UInt64, UInt64Array[ii]);
				break;
			}
			case VpiType_Float:
			{
				Vpi_Field_CopyToScalar(Float, FloatArray[ii]);
				break;
			}
			case VpiType_Double:
			{
				Vpi_Field_CopyToScalar(Double, DoubleArray[ii]);
				break;
			}
			case VpiType_DateTime:
			{
				Vpi_Field_CopyToScalar(DateTime, DateTimeArray[ii]);
				break;
			}
			case VpiType_StatusCode:
			{
				Vpi_Field_CopyToScalar(StatusCode, StatusCodeArray[ii]);
				break;
			}
			//case VpiType_Guid:
			//{
			//	Vpi_Field_CopyTo(Guid, GuidArray[ii]);
			//	break;
			//}

			case VpiType_String:
			{
				Vpi_Field_CopyTo(String, StringArray[ii]);
				break;
			}

			case VpiType_ByteString:
			{
				Vpi_Field_CopyTo(ByteString, ByteStringArray[ii]);
				break;
			}

			case VpiType_XmlElement:
			{
				Vpi_Field_CopyTo(XmlElement, XmlElementArray[ii]);
				break;
			}

			case VpiType_NodeId:
			{
				Vpi_Field_CopyTo(NodeId, NodeIdArray[ii]);
				break;
			}

			case VpiType_ExpandedNodeId:
			{
				Vpi_Field_CopyTo(ExpandedNodeId, ExpandedNodeIdArray[ii]);
				break;
			}

			case VpiType_QualifiedName:
			{
				Vpi_Field_CopyTo(QualifiedName, QualifiedNameArray[ii]);
				break;
			}

			case VpiType_LocalizedText:
			{
				Vpi_Field_CopyTo(LocalizedText, LocalizedTextArray[ii]);
				break;
			}

			case VpiType_DataValue:
			{
				Vpi_Field_CopyTo(DataValue, DataValueArray[ii]);
				break;
			}

			case VpiType_Variant:
			{
				Vpi_Field_CopyTo(Variant, VariantArray[ii]);
				break;
			}

			//case VpiType_ExtensionObject:
			//{
			//	Vpi_Field_CopyTo(ExtensionObject, ExtensionObjectArray[ii]);
			//	break;
			//}
		}
	}

	return uStatus;
}

/*============================================================================
* Vpi_VariantArrayValue_CreateArray
*===========================================================================*/
Vpi_StatusCode Vpi_VariantArrayValue_CreateArray(Vpi_UInt16    a_Datatype,
	Vpi_Int32     a_iNumberOfElements,
	Vpi_Void**    a_ppArray)
{
	Vpi_Int iTypeSize = 0;

	Vpi_StatusCode uStatus = Vpi_Good;

	switch (a_Datatype)
	{
		case VpiType_Boolean:
		{
			iTypeSize = sizeof(Vpi_Boolean);
			break;
		}
		case VpiType_SByte:
		{
			iTypeSize = sizeof(Vpi_SByte);
			break;
		}
		case VpiType_Byte:
		{
			iTypeSize = sizeof(Vpi_Byte);
			break;
		}
		case VpiType_Int16:
		{
			iTypeSize = sizeof(Vpi_Int16);
			break;
		}
		case VpiType_UInt16:
		{
			iTypeSize = sizeof(Vpi_UInt16);
			break;
		}
		case VpiType_Int32:
		{
			iTypeSize = sizeof(Vpi_Int32);
			break;
		}
		case VpiType_UInt32:
		{
			iTypeSize = sizeof(Vpi_UInt32);
			break;
		}
		case VpiType_Int64:
		{
			iTypeSize = sizeof(Vpi_Int64);
			break;
		}
		case VpiType_UInt64:
		{
			iTypeSize = sizeof(Vpi_UInt64);
			break;
		}
		case VpiType_Float:
		{
			iTypeSize = sizeof(Vpi_Float);
			break;
		}
		case VpiType_Double:
		{
			iTypeSize = sizeof(Vpi_Double);
			break;
		}
		case VpiType_DateTime:
		{
			iTypeSize = sizeof(Vpi_DateTime);
			break;
		}
		case VpiType_StatusCode:
		{
			iTypeSize = sizeof(Vpi_StatusCode);
			break;
		}
		case VpiType_Guid:
		{
			iTypeSize = sizeof(Vpi_Guid);
			break;
		}

		case VpiType_String:
		{
			iTypeSize = sizeof(Vpi_String);
			break;
		}

		case VpiType_ByteString:
		{
			iTypeSize = sizeof(Vpi_ByteString);
			break;
		}

		case VpiType_XmlElement:
		{
			iTypeSize = sizeof(Vpi_XmlElement);
			break;
		}

		case VpiType_NodeId:
		{
			iTypeSize = sizeof(Vpi_NodeId);
			break;
		}

		case VpiType_ExpandedNodeId:
		{
			iTypeSize = sizeof(Vpi_ExpandedNodeId);
			break;
		}

		case VpiType_QualifiedName:
		{
			iTypeSize = sizeof(Vpi_QualifiedName);
			break;
		}

		case VpiType_LocalizedText:
		{
			iTypeSize = sizeof(Vpi_LocalizedText);
			break;
		}

		case VpiType_DataValue:
		{
			iTypeSize = sizeof(Vpi_DataValue);
			break;
		}

		case VpiType_ExtensionObject:
		{
			iTypeSize = sizeof(Vpi_ExtensionObject);
			break;
		}

		case VpiType_Variant:
		{
			iTypeSize = sizeof(Vpi_Variant);
			break;
		}

		default:
		{
			uStatus=Vpi_BadInvalidArgument;
		}
	}
	if (uStatus == Vpi_Good)
	{
		*a_ppArray = Vpi_Alloc(iTypeSize * a_iNumberOfElements);
		if (*a_ppArray)
			Vpi_MemSet(*a_ppArray, 0, iTypeSize * a_iNumberOfElements);
	}
	return uStatus;
}
