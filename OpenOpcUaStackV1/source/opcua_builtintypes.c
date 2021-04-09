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

#include <opcua.h>
#include <opcua_string.h>
#include <opcua_guid.h>
#include <opcua_builtintypes.h>
#include <opcua_statuscodes.h>
#include <opcua_encodeableobject.h>
#include <opcua_memorystream.h>

/*============================================================================
 * OpcUa_VariantUnion_Clear
 *===========================================================================*/
static OpcUa_Void OpcUa_VariantUnion_Clear(OpcUa_UInt16 uDatatype, OpcUa_VariantUnion* a_pValue);

/*============================================================================
 * OpcUa_VariantArrayValue_Clear
 *===========================================================================*/
static OpcUa_Void OpcUa_VariantArrayValue_Clear(OpcUa_UInt16 uDatatype, OpcUa_Int32 iLength, OpcUa_VariantArrayUnion* pValue);

/*============================================================================
 * OpcUa_VariantMatrixValue_GetElementCount
 *===========================================================================*/
OpcUa_Int32 OpcUa_VariantMatrix_GetElementCount(const OpcUa_VariantMatrixValue* a_pValue)
{
	OpcUa_Int32 ii = 0;
	OpcUa_Int32 iLength = 1;

	if (a_pValue == OpcUa_Null)
	{
		return 0;
	}

	if (a_pValue->NoOfDimensions == 0 || a_pValue->Dimensions == OpcUa_Null)
	{
		return 0;
	}

	for (ii = 0; ii < a_pValue->NoOfDimensions; ii++)
	{
		iLength *= a_pValue->Dimensions[ii];
	}

	return iLength;
}

/*============================================================================
 * OpcUa_ByteString_Initialize
 *===========================================================================*/
OpcUa_Void OpcUa_ByteString_Initialize(OpcUa_ByteString* a_pValue)
{
	if(a_pValue == OpcUa_Null)
	{
		return;
	}
		
	a_pValue->Data = OpcUa_Null;
	a_pValue->Length = -1;
}

/*============================================================================
 * OpcUa_ByteString_Clear
 *===========================================================================*/
OpcUa_Void OpcUa_ByteString_Clear(OpcUa_ByteString* a_pValue)
{
	if(a_pValue == OpcUa_Null)
	{
		return;
	}

	OpcUa_Free(a_pValue->Data);
	a_pValue->Data = OpcUa_Null;
	a_pValue->Length = -1;
}

/*============================================================================
 * OpcUa_ByteString_Compare
 *===========================================================================*/
OpcUa_Int OpcUa_ByteString_Compare(const OpcUa_ByteString* a_pValue1, const OpcUa_ByteString* a_pValue2)
{
	if(a_pValue1 == a_pValue2)
	{
		return OPCUA_EQUAL;
	}

	if((a_pValue1 == OpcUa_Null) || (a_pValue2 == OpcUa_Null))
	{
		return 1;
	}

	OpcUa_Field_Compare(Int32, Length);

	if(a_pValue1->Length > 0)
	{
		return OpcUa_MemCmp(a_pValue1->Data, a_pValue2->Data, a_pValue1->Length);
	}
	else
	{
		return OPCUA_EQUAL;
	}
}

/*============================================================================
 * OpcUa_ByteString_CopyTo
 *===========================================================================*/
OpcUa_StatusCode OpcUa_ByteString_CopyTo(const OpcUa_ByteString* a_pSource, OpcUa_ByteString* a_pDestination)
{
OpcUa_InitializeStatus(OpcUa_Module_ProxyStub, "OpcUa_ByteString_CopyTo");

	a_pDestination->Length = a_pSource->Length;

	if(a_pDestination->Length > 0)
	{
		a_pDestination->Data = (OpcUa_Byte*)OpcUa_Alloc(a_pDestination->Length);
		OpcUa_GotoErrorIfAllocFailed(a_pDestination->Data);
		OpcUa_MemCpy(a_pDestination->Data, a_pDestination->Length, a_pSource->Data, a_pSource->Length);
	}

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	OpcUa_ByteString_Clear(a_pDestination);

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_ByteString_Concatenate
 *===========================================================================*/
OpcUa_StatusCode OpcUa_ByteString_Concatenate(const OpcUa_ByteString* a_pSource, OpcUa_ByteString* a_pDestination, OpcUa_Int a_iLen)
{
	OpcUa_Int   iBytesToCopy    = a_iLen;
	OpcUa_Int   iNewLength      = 0;
	OpcUa_Byte* pTemp           = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_ProxyStub, "OpcUa_ByteString_Concatenate");

	OpcUa_ReturnErrorIfArgumentNull(a_pSource);
	OpcUa_ReturnErrorIfArgumentNull(a_pDestination);
	OpcUa_ReturnErrorIfTrue((a_pDestination->Length <= 0), OpcUa_BadInvalidArgument);
	OpcUa_ReturnErrorIfTrue((a_pDestination == a_pSource), OpcUa_BadInvalidArgument);

	if(iBytesToCopy <= 0)
	{
		iBytesToCopy = a_pSource->Length;
	}

	OpcUa_ReturnErrorIfTrue(iBytesToCopy <= 0, OpcUa_BadInvalidArgument);

	iNewLength  = a_pDestination->Length + iBytesToCopy;

	pTemp = OpcUa_ReAlloc(a_pDestination->Data, iNewLength);
	OpcUa_ReturnErrorIfAllocFailed(pTemp);

	a_pDestination->Data = pTemp;

	OpcUa_MemCpy(   &a_pDestination->Data[a_pDestination->Length],
					iBytesToCopy,
					a_pSource->Data,
					iBytesToCopy);

	a_pDestination->Length += iBytesToCopy;

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_NodeId_Initialize
 *===========================================================================*/
OpcUa_Void OpcUa_NodeId_Initialize(OpcUa_NodeId* a_pValue)
{
	if (a_pValue == OpcUa_Null)
	{
		return;
	}

	a_pValue->IdentifierType = OpcUa_IdentifierType_Numeric;
	a_pValue->Identifier.Numeric = 0;
	a_pValue->NamespaceIndex = 0;
}

/*============================================================================
 * OpcUa_NodeId_Clear
 *===========================================================================*/
OpcUa_Void OpcUa_NodeId_Clear(OpcUa_NodeId* a_pValue)
{
	if (a_pValue == OpcUa_Null)
	{
		return;
	}

	switch (a_pValue->IdentifierType)
	{
		default:
		case OpcUa_IdentifierType_Numeric:
		{
			break;
		}

		case OpcUa_IdentifierType_String:
		{
			OpcUa_String_Clear(&(a_pValue->Identifier.String));
			break;
		}

		case OpcUa_IdentifierType_Guid:
		{
			OpcUa_Free(a_pValue->Identifier.Guid);
			break;
		}

		case OpcUa_IdentifierType_Opaque:
		{
			OpcUa_ByteString_Clear(&a_pValue->Identifier.ByteString);
			break;
		}
	}

	a_pValue->IdentifierType = OpcUa_IdentifierType_Numeric;
	a_pValue->Identifier.Numeric = 0;
	a_pValue->NamespaceIndex = 0;
}

/*============================================================================
 * OpcUa_NodeId_IsNull
 *===========================================================================*/
OpcUa_Boolean OpcUa_NodeId_IsNull(OpcUa_NodeId* a_pValue)
{
	if (a_pValue == OpcUa_Null)
	{
		return OpcUa_True;
	}

	if (a_pValue->NamespaceIndex != 0)
	{
		return OpcUa_False;
	}

	switch (a_pValue->IdentifierType)
	{
		case OpcUa_IdentifierType_Numeric:
		{
			if (a_pValue->Identifier.Numeric != 0)
			{
				return OpcUa_False;
			}

			break;
		}

		case OpcUa_IdentifierType_String:
		{
			if (!OpcUa_String_IsNull(&(a_pValue->Identifier.String)) && OpcUa_StrLen(&(a_pValue->Identifier.String)) > 0)
			{
				return OpcUa_False;
			}

			break;
		}
		
		case OpcUa_IdentifierType_Guid:
		{
			return OpcUa_Guid_IsNull(a_pValue->Identifier.Guid);
		}
		
		case OpcUa_IdentifierType_Opaque:
		{
			if (a_pValue->Identifier.ByteString.Length > 0)
			{
				return OpcUa_False;
			}

			break;
		}
	}

	return OpcUa_True;
}

/*============================================================================
 * OpcUa_NodeId_Compare
 *===========================================================================*/
OpcUa_Int OpcUa_NodeId_Compare(const OpcUa_NodeId* a_pValue1, const OpcUa_NodeId* a_pValue2)
{
	if(a_pValue1 == a_pValue2)
	{
		return OPCUA_EQUAL;
	}

	if((a_pValue1 == OpcUa_Null) || (a_pValue2 == OpcUa_Null))
	{
		return 1;
	}

	OpcUa_Field_Compare(UInt16, IdentifierType);

	OpcUa_Field_Compare(UInt16, NamespaceIndex);

	switch(a_pValue1->IdentifierType)
	{
	case OpcUa_IdentifierType_Numeric:
		{
			return OpcUa_UInt32_Compare(&a_pValue1->Identifier.Numeric, &a_pValue2->Identifier.Numeric);
		}
	case OpcUa_IdentifierType_String:
		{
			return OpcUa_String_Compare(&a_pValue1->Identifier.String, &a_pValue2->Identifier.String);
		}
	case OpcUa_IdentifierType_Opaque:
		{
			return OpcUa_ByteString_Compare(&a_pValue1->Identifier.ByteString, &a_pValue2->Identifier.ByteString);
		}
	case OpcUa_IdentifierType_Guid:
		{
			return OpcUa_Guid_Compare(a_pValue1->Identifier.Guid, a_pValue2->Identifier.Guid);
		}
	default:
		{
			return 1;
		}
	}
}

/*============================================================================
 * OpcUa_NodeId_CopyTo
 *===========================================================================*/
OpcUa_StatusCode OpcUa_NodeId_CopyTo(const OpcUa_NodeId* a_pSource, OpcUa_NodeId* a_pDestination)
{
OpcUa_InitializeStatus(OpcUa_Module_ProxyStub, "OpcUa_NodeId_CopyTo");

	OpcUa_ReturnErrorIfArgumentNull(a_pSource);
	OpcUa_ReturnErrorIfArgumentNull(a_pDestination);

	OpcUa_NodeId_Initialize(a_pDestination);

	a_pDestination->IdentifierType = a_pSource->IdentifierType;
	a_pDestination->NamespaceIndex = a_pSource->NamespaceIndex;

	switch(a_pSource->IdentifierType)
	{
		case OpcUa_IdentifierType_Numeric:
			{
				a_pDestination->Identifier.Numeric = a_pSource->Identifier.Numeric;
				break;
			}
		case OpcUa_IdentifierType_String:
			{
				OpcUa_String_Initialize(&(a_pDestination->Identifier.String));
				OpcUa_String_StrnCpy(&a_pDestination->Identifier.String, &a_pSource->Identifier.String, OPCUA_STRING_LENDONTCARE);
				break;
			}
		case OpcUa_IdentifierType_Opaque:
			{
				a_pDestination->Identifier.ByteString.Length = a_pSource->Identifier.ByteString.Length;

				if(a_pDestination->Identifier.ByteString.Length > 0)
				{
					a_pDestination->Identifier.ByteString.Data = (OpcUa_Byte*)OpcUa_Alloc(a_pSource->Identifier.ByteString.Length);
					OpcUa_GotoErrorIfAllocFailed(a_pDestination->Identifier.ByteString.Data);
					OpcUa_MemCpy(a_pDestination->Identifier.ByteString.Data, a_pDestination->Identifier.ByteString.Length, a_pSource->Identifier.ByteString.Data, a_pSource->Identifier.ByteString.Length);
				}
				else
				{
					a_pDestination->Identifier.ByteString.Data = OpcUa_Null;
				}
				break;
			}
		case OpcUa_IdentifierType_Guid:
			{
				if(a_pSource->Identifier.Guid != OpcUa_Null)
				{
					a_pDestination->Identifier.Guid = (OpcUa_Guid*)OpcUa_Alloc(sizeof(OpcUa_Guid));
					OpcUa_GotoErrorIfAllocFailed(a_pDestination->Identifier.Guid);
					OpcUa_Guid_CopyTo(a_pSource->Identifier.Guid, a_pDestination->Identifier.Guid);
				}
				else
				{
					a_pDestination->Identifier.Guid = OpcUa_Null;
				}
				break;
			}
		default:
			{
				OpcUa_GotoErrorWithStatus(OpcUa_BadInvalidArgument);
			}
	}

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	OpcUa_NodeId_Clear(a_pDestination);

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_ExpandedNodeId_Initialize
 *===========================================================================*/
OpcUa_Void OpcUa_ExpandedNodeId_Initialize(OpcUa_ExpandedNodeId* a_pValue)
{   
	if (a_pValue == OpcUa_Null)
	{
		return;
	}

	OpcUa_MemSet(a_pValue, 0, sizeof(OpcUa_ExpandedNodeId));
	OpcUa_NodeId_Initialize(&a_pValue->NodeId);
}

/*============================================================================
 * OpcUa_ExpandedNodeId_Clear
 *===========================================================================*/
OpcUa_Void OpcUa_ExpandedNodeId_Clear(OpcUa_ExpandedNodeId* a_pValue)
{   
	if (a_pValue == OpcUa_Null)
	{
		return;
	}

	OpcUa_NodeId_Clear(&a_pValue->NodeId);
	OpcUa_String_Clear(&(a_pValue->NamespaceUri));
	OpcUa_MemSet(a_pValue, 0, sizeof(OpcUa_ExpandedNodeId));
}

/*============================================================================
 * OpcUa_ExpandedNodeId_Compare
 *===========================================================================*/
OpcUa_Int OpcUa_ExpandedNodeId_Compare(const OpcUa_ExpandedNodeId* a_pValue1, const OpcUa_ExpandedNodeId* a_pValue2)
{
	if(a_pValue1 == a_pValue2)
	{
		return OPCUA_EQUAL;
	}

	if((a_pValue1 == OpcUa_Null) || (a_pValue2 == OpcUa_Null))
	{
		return 1;
	}

	OpcUa_Field_Compare(UInt32, ServerIndex);
	OpcUa_Field_Compare(String, NamespaceUri);
	OpcUa_Field_Compare(NodeId, NodeId);

	return OPCUA_EQUAL;
}

/*============================================================================
 * OpcUa_ExpandedNodeId_CopyTo
 *===========================================================================*/
OpcUa_StatusCode OpcUa_ExpandedNodeId_CopyTo(const OpcUa_ExpandedNodeId* a_pSource, OpcUa_ExpandedNodeId* a_pDestination)
{
OpcUa_InitializeStatus(OpcUa_Module_ProxyStub, "OpcUa_ExpandedNodeId_CopyTo");

	OpcUa_Field_CopyToScalar(UInt32, ServerIndex);
	OpcUa_Field_CopyTo(String, NamespaceUri);
	OpcUa_Field_CopyTo(NodeId, NodeId);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	OpcUa_ExpandedNodeId_Clear(a_pDestination);

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_ExpandedNodeId_IsNull
 *===========================================================================*/
OpcUa_Boolean OpcUa_ExpandedNodeId_IsNull(OpcUa_ExpandedNodeId* a_pValue)
{
	if (a_pValue == OpcUa_Null)
	{
		return OpcUa_True;
	}

	if((!OpcUa_String_IsNull(&(a_pValue->NamespaceUri))) && (OpcUa_StrLen(&(a_pValue->NamespaceUri)) > 0))
	{
		return OpcUa_False;
	}

	return OpcUa_NodeId_IsNull(&a_pValue->NodeId);
}

/*============================================================================
 * OpcUa_DiagnosticInfo_Initialize
 *===========================================================================*/
OpcUa_Void OpcUa_DiagnosticInfo_Initialize(OpcUa_DiagnosticInfo* a_pValue)
{   
	if (a_pValue == OpcUa_Null)
	{
		return;
	}

	OpcUa_MemSet(a_pValue, 0, sizeof(OpcUa_DiagnosticInfo));

	a_pValue->SymbolicId    = -1;
	a_pValue->NamespaceUri  = -1;
	a_pValue->Locale        = -1;
	a_pValue->LocalizedText = -1;
}

/*============================================================================
 * OpcUa_DiagnosticInfo_Clear
 *===========================================================================*/
OpcUa_Void OpcUa_DiagnosticInfo_Clear(OpcUa_DiagnosticInfo* a_pValue)
{   
	if (a_pValue == OpcUa_Null)
	{
		return;
	}

	OpcUa_String_Clear(&(a_pValue->AdditionalInfo));

	if(a_pValue->InnerDiagnosticInfo != OpcUa_Null)
	{
		OpcUa_DiagnosticInfo_Clear(a_pValue->InnerDiagnosticInfo);
		OpcUa_Free(a_pValue->InnerDiagnosticInfo);
	}

	OpcUa_MemSet(a_pValue, 0, sizeof(OpcUa_DiagnosticInfo));
}

/*============================================================================
 * OpcUa_DiagnosticInfo_Compare
 *===========================================================================*/
OpcUa_Int OpcUa_DiagnosticInfo_Compare(const OpcUa_DiagnosticInfo* a_pValue1, const OpcUa_DiagnosticInfo* a_pValue2)
{
	if(a_pValue1 == a_pValue2)
	{
		return OPCUA_EQUAL;
	}

	if((a_pValue1 == OpcUa_Null) || (a_pValue2 == OpcUa_Null))
	{
		return 1;
	}

	OpcUa_Field_Compare(Int32, SymbolicId);
	OpcUa_Field_Compare(Int32, NamespaceUri);
	OpcUa_Field_Compare(Int32, Locale);
	OpcUa_Field_Compare(Int32, LocalizedText);
	OpcUa_Field_Compare(String, AdditionalInfo);
	OpcUa_Field_Compare(StatusCode, InnerStatusCode);

	return OpcUa_DiagnosticInfo_Compare(a_pValue1->InnerDiagnosticInfo, a_pValue2->InnerDiagnosticInfo);
}

/*============================================================================
 * OpcUa_DiagnosticInfo_CopyTo
 *===========================================================================*/
OpcUa_StatusCode OpcUa_DiagnosticInfo_CopyTo(const OpcUa_DiagnosticInfo* a_pSource, OpcUa_DiagnosticInfo* a_pDestination)
{
OpcUa_InitializeStatus(OpcUa_Module_ProxyStub, "OpcUa_DiagnosticInfo_CopyTo");

	OpcUa_Field_CopyToScalar(Int32, SymbolicId);
	OpcUa_Field_CopyToScalar(Int32, NamespaceUri);
	OpcUa_Field_CopyToScalar(Int32, Locale);
	OpcUa_Field_CopyToScalar(Int32, LocalizedText);
	OpcUa_Field_CopyTo(String, AdditionalInfo);
	OpcUa_Field_CopyToScalar(StatusCode, InnerStatusCode);
	
	if(a_pSource->InnerDiagnosticInfo != OpcUa_Null)
	{
		OpcUa_DiagnosticInfo* pTemp = (OpcUa_DiagnosticInfo*)OpcUa_Alloc(sizeof(OpcUa_DiagnosticInfo));
		OpcUa_GotoErrorIfAllocFailed(pTemp);
		OpcUa_DiagnosticInfo_Initialize(pTemp);
		uStatus = OpcUa_DiagnosticInfo_CopyTo(a_pSource->InnerDiagnosticInfo, pTemp);

		if(OpcUa_IsBad(uStatus))
		{
			OpcUa_Free(pTemp);
			OpcUa_GotoError;
		}

		a_pDestination->InnerDiagnosticInfo = pTemp;
	}
	else
	{
		a_pDestination->InnerDiagnosticInfo = OpcUa_Null;
	}

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	OpcUa_DiagnosticInfo_Clear(a_pDestination);

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_LocalizedText_Initialize
 *===========================================================================*/
void OpcUa_LocalizedText_Initialize(OpcUa_LocalizedText* a_pValue)
{
	if (a_pValue != OpcUa_Null)
	{
		OpcUa_Field_Initialize(String, Locale);
		OpcUa_Field_Initialize(String, Text);
	}   
}

/*============================================================================
 * OpcUa_LocalizedText_Clear
 *===========================================================================*/
void OpcUa_LocalizedText_Clear(OpcUa_LocalizedText* a_pValue)
{
	if (a_pValue != OpcUa_Null)
	{
		OpcUa_Field_Clear(String, Locale);
		OpcUa_Field_Clear(String, Text);
	}   
}
/*============================================================================
* OpcUa_EncodeableObject_Default_Copy
*===========================================================================*/
static OpcUa_Int OpcUa_EncodeableObject_Default_Copy(const OpcUa_Void* a_pValue1, OpcUa_Void** a_ppValue2,OpcUa_EncodeableType *a_pType)
{
	OpcUa_StatusCode      uStatus = OpcUa_Good;
	OpcUa_Void*           pObject = (OpcUa_Void*)a_pValue1;
	OpcUa_OutputStream*   pOutputStream;
	OpcUa_InputStream*    pInputStream;
	OpcUa_Byte*           pBuffer;
	OpcUa_UInt32          uBufferSize;
	OpcUa_UInt32          uInputSize;

	if ((a_pValue1 == OpcUa_Null) || (a_ppValue2 == OpcUa_Null) || (a_pType == OpcUa_Null))
	{
		return OpcUa_BadInvalidArgument;
	}

	*a_ppValue2 = OpcUa_Null;

	/* Duplicate the extension object: we first serialize it... */
	uStatus = OpcUa_MemoryStream_CreateWriteable(4096, 0, &pOutputStream);
	if (OpcUa_IsBad(uStatus))
		return uStatus;

	uStatus = OpcUa_EncodeableObject_Encode(a_pType, pObject, OpcUa_Null, pOutputStream);
	if (OpcUa_IsBad(uStatus))
	{
		pOutputStream->Delete((OpcUa_Stream**)&pOutputStream);
		return uStatus;
	}

	uStatus = pOutputStream->Close((OpcUa_Stream*)pOutputStream);
	if (OpcUa_IsBad(uStatus))
	{
		pOutputStream->Delete((OpcUa_Stream**)&pOutputStream);
		return uStatus;
	}

	uStatus = pOutputStream->GetPosition((OpcUa_Stream*)pOutputStream, &uInputSize);
	if (OpcUa_IsBad(uStatus))
	{
		pOutputStream->Delete((OpcUa_Stream**)&pOutputStream);
		return uStatus;
	}

	uStatus = OpcUa_MemoryStream_DetachBuffer(pOutputStream, &pBuffer, &uBufferSize);
	if (OpcUa_IsBad(uStatus))
	{
		pOutputStream->Delete((OpcUa_Stream**)&pOutputStream);
		return uStatus;
	}

	pOutputStream->Delete((OpcUa_Stream**)&pOutputStream);

	/* Now, de-serialize it again... */
	uStatus = OpcUa_MemoryStream_CreateReadable(pBuffer, uInputSize, OpcUa_True, &pInputStream);
	if (OpcUa_IsBad(uStatus))
	{
		OpcUa_Free(pBuffer);
		return uStatus;
	}

	pObject = OpcUa_Alloc(a_pType->AllocationSize);
	if (!pObject)
	{
		pInputStream->Delete((OpcUa_Stream**)&pInputStream);
		return OpcUa_BadOutOfMemory;
	}

	uStatus = OpcUa_EncodeableObject_Decode(a_pType, pInputStream, OpcUa_Null, pObject);
	if (OpcUa_IsBad(uStatus))
	{
		pInputStream->Delete((OpcUa_Stream**)&pInputStream);
		OpcUa_Free(pObject);
		return uStatus;
	}

	uStatus = pInputStream->Close((OpcUa_Stream*)pInputStream);
	if (OpcUa_IsBad(uStatus))
	{
		pInputStream->Delete((OpcUa_Stream**)&pInputStream);
		a_pType->Clear(pObject);
		OpcUa_Free(pObject);
		return uStatus;
	}

	pInputStream->Delete((OpcUa_Stream**)&pInputStream);

	*a_ppValue2 = pObject;
	return OpcUa_Good;
}
/*============================================================================
 * OpcUa_LocalizedText_Compare
 *===========================================================================*/
OpcUa_Int OpcUa_LocalizedText_Compare(const OpcUa_LocalizedText* a_pValue1, const OpcUa_LocalizedText* a_pValue2)
{
	if(a_pValue1 == a_pValue2)
	{
		return OPCUA_EQUAL;
	}

	if((a_pValue1 == OpcUa_Null) || (a_pValue2 == OpcUa_Null))
	{
		return 1;
	}

	OpcUa_Field_Compare(String, Locale);
	OpcUa_Field_Compare(String, Text);

	return OPCUA_EQUAL;
}

/*============================================================================
 * OpcUa_LocalizedText_CopyTo
 *===========================================================================*/
OpcUa_StatusCode OpcUa_LocalizedText_CopyTo(const OpcUa_LocalizedText* a_pSource, OpcUa_LocalizedText* a_pDestination)
{
OpcUa_InitializeStatus(OpcUa_Module_ProxyStub, "OpcUa_LocalizedText_CopyTo");
OpcUa_GotoErrorIfArgumentNull(a_pSource);
	OpcUa_Field_CopyTo(String, Locale);
	OpcUa_Field_CopyTo(String, Text);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	OpcUa_LocalizedText_Clear(a_pDestination);

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_QualifiedName_Initialize
 *===========================================================================*/
void OpcUa_QualifiedName_Initialize(OpcUa_QualifiedName* a_pValue)
{
	if (a_pValue != OpcUa_Null)
	{
		OpcUa_Field_Initialize(Int32, NamespaceIndex);
		OpcUa_Field_Initialize(String, Name);
	}   
}

/*============================================================================
 * OpcUa_QualifiedName_Clear
 *===========================================================================*/
void OpcUa_QualifiedName_Clear(OpcUa_QualifiedName* a_pValue)
{
	if (a_pValue != OpcUa_Null)
	{
		OpcUa_Field_Clear(Int32, NamespaceIndex);
		OpcUa_Field_Clear(String, Name);
	}   
}

/*============================================================================
 * OpcUa_QualifiedName_Compare
 *===========================================================================*/
OpcUa_Int OpcUa_QualifiedName_Compare(const OpcUa_QualifiedName* a_pValue1, const OpcUa_QualifiedName* a_pValue2)
{
	if(a_pValue1 == a_pValue2)
	{
		return OPCUA_EQUAL;
	}

	if((a_pValue1 == OpcUa_Null) || (a_pValue2 == OpcUa_Null))
	{
		return 1;
	}

	OpcUa_Field_Compare(String, Name);
	OpcUa_Field_Compare(UInt16, NamespaceIndex);

	return OPCUA_EQUAL;
}

/*============================================================================
 * OpcUa_QualifiedName_CopyTo
 *===========================================================================*/
OpcUa_StatusCode OpcUa_QualifiedName_CopyTo(const OpcUa_QualifiedName* a_pSource, OpcUa_QualifiedName* a_pDestination)
{
OpcUa_InitializeStatus(OpcUa_Module_ProxyStub, "OpcUa_QualifiedName_CopyTo");

	OpcUa_Field_CopyTo(String, Name);
	OpcUa_Field_CopyToScalar(UInt16, NamespaceIndex);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	OpcUa_QualifiedName_Clear(a_pDestination);

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_DataValue_Initialize
 *===========================================================================*/
OpcUa_Void OpcUa_DataValue_Initialize(OpcUa_DataValue* a_pValue)
{
	if (a_pValue == OpcUa_Null)
	{
		return;
	}

	OpcUa_MemSet(a_pValue, 0, sizeof(OpcUa_DataValue));
}

/*============================================================================
 * OpcUa_DataValue_Clear
 *===========================================================================*/
OpcUa_Void OpcUa_DataValue_Clear(OpcUa_DataValue* a_pValue)
{
	if (a_pValue == OpcUa_Null)
	{
		return;
	}

	OpcUa_Variant_Clear(&a_pValue->Value);
	OpcUa_MemSet(a_pValue, 0, sizeof(OpcUa_DataValue));
}

/*============================================================================
 * OpcUa_DataValue_Compare
 *===========================================================================*/
OpcUa_Int OpcUa_DataValue_Compare(const OpcUa_DataValue* a_pValue1, const OpcUa_DataValue* a_pValue2)
{
	if(a_pValue1 == a_pValue2)
	{
		return OPCUA_EQUAL;
	}

	if((a_pValue1 == OpcUa_Null) || (a_pValue2 == OpcUa_Null))
	{
		return 1;
	}

	OpcUa_Field_Compare(StatusCode, StatusCode);
	OpcUa_Field_Compare(DateTime, ServerTimestamp);
#if !OPCUA_DATAVALUE_OMIT_PICOSECONDS
	OpcUa_Field_Compare(UInt16, ServerPicoseconds);
#endif /* !OPCUA_DATAVALUE_OMIT_PICOSECONDS */
	OpcUa_Field_Compare(DateTime, SourceTimestamp);
#if !OPCUA_DATAVALUE_OMIT_PICOSECONDS
	OpcUa_Field_Compare(UInt16, SourcePicoseconds);
#endif /* !OPCUA_DATAVALUE_OMIT_PICOSECONDS */
	OpcUa_Field_Compare(Variant, Value);

	return OPCUA_EQUAL;
}

/*============================================================================
 * OpcUa_DataValue_CopyTo
 *===========================================================================*/
OpcUa_StatusCode OpcUa_DataValue_CopyTo(const OpcUa_DataValue* a_pSource, OpcUa_DataValue* a_pDestination)
{
OpcUa_InitializeStatus(OpcUa_Module_ProxyStub, "OpcUa_DataValue_CopyTo");

	OpcUa_Field_CopyToScalar(StatusCode, StatusCode);
	OpcUa_Field_CopyToScalar(DateTime, ServerTimestamp);
#if !OPCUA_DATAVALUE_OMIT_PICOSECONDS
	OpcUa_Field_CopyToScalar(DateTime, ServerPicoseconds);
#endif /* !OPCUA_DATAVALUE_OMIT_PICOSECONDS */
	OpcUa_Field_CopyToScalar(DateTime, SourceTimestamp);
#if !OPCUA_DATAVALUE_OMIT_PICOSECONDS
	OpcUa_Field_CopyToScalar(DateTime, SourcePicoseconds);
#endif /* !OPCUA_DATAVALUE_OMIT_PICOSECONDS */
	OpcUa_Field_CopyTo(Variant, Value);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	OpcUa_DataValue_Clear(a_pDestination);

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_ExtensionObject_Create
 *===========================================================================*/
OpcUa_Void OpcUa_ExtensionObject_Create(OpcUa_ExtensionObject** a_ppValue)
{
	if (a_ppValue != OpcUa_Null)
	{
		*a_ppValue = (OpcUa_ExtensionObject*)OpcUa_Alloc(sizeof(OpcUa_ExtensionObject));
		if(*a_ppValue != OpcUa_Null)
		{
			OpcUa_ExtensionObject_Initialize(*a_ppValue);
		}
	}
}

/*============================================================================
 * OpcUa_ExtensionObject_Initialize
 *===========================================================================*/
OpcUa_Void OpcUa_ExtensionObject_Initialize(OpcUa_ExtensionObject* a_pValue)
{
	if (a_pValue != OpcUa_Null)
	{
		OpcUa_MemSet(a_pValue, 0, sizeof(OpcUa_ExtensionObject));
	}
}

/*============================================================================
 * OpcUa_ExtensionObject_Clear
 *===========================================================================*/
OpcUa_Void OpcUa_ExtensionObject_Clear(OpcUa_ExtensionObject* a_pValue)
{
	if (a_pValue != OpcUa_Null)
	{
		OpcUa_ExpandedNodeId_Clear(&a_pValue->TypeId);

		switch (a_pValue->Encoding)
		{
			case OpcUa_ExtensionObjectEncoding_Binary:
			{
				OpcUa_ByteString_Clear(&a_pValue->Body.Binary);
				break;
			}

			case OpcUa_ExtensionObjectEncoding_Xml:
			{
				OpcUa_XmlElement_Clear(&a_pValue->Body.Xml);
				break;
			}

			case OpcUa_ExtensionObjectEncoding_EncodeableObject:
			{
				OpcUa_EncodeableObject_Delete(a_pValue->Body.EncodeableObject.Type, &a_pValue->Body.EncodeableObject.Object);
				break;
			}
			default:
			{
				break;
			}
		}

		OpcUa_MemSet(a_pValue, 0, sizeof(OpcUa_ExtensionObject));
	}
}

/*============================================================================
 * OpcUa_ExtensionObject_Delete
 *===========================================================================*/
OpcUa_Void OpcUa_ExtensionObject_Delete(OpcUa_ExtensionObject** a_ppValue)
{
	if (a_ppValue != OpcUa_Null && *a_ppValue != OpcUa_Null)
	{
		OpcUa_ExtensionObject_Clear(*a_ppValue);
		OpcUa_Free(*a_ppValue);
		*a_ppValue = OpcUa_Null;
	}
}

/*============================================================================
 * OpcUa_ExtensionObject_Compare
 *===========================================================================*/
OpcUa_Int OpcUa_ExtensionObject_Compare(const OpcUa_ExtensionObject* a_pValue1, const OpcUa_ExtensionObject* a_pValue2)
{
	if(a_pValue1 == a_pValue2)
	{
		return OPCUA_EQUAL;
	}

	if((a_pValue1 == OpcUa_Null) || (a_pValue2 == OpcUa_Null))
	{
		return 1;
	}

	OpcUa_Field_Compare(ExpandedNodeId, TypeId);
	OpcUa_Field_CompareEnumerated(ExtensionObjectEncoding, Encoding);
	OpcUa_Field_Compare(UInt32, BodySize);

	switch(a_pValue1->Encoding)
	{
	case OpcUa_ExtensionObjectEncoding_Binary:
		{
			return OpcUa_ByteString_Compare(&a_pValue1->Body.Binary, &a_pValue2->Body.Binary);
		}
	case OpcUa_ExtensionObjectEncoding_Xml:
		{
			return OpcUa_XmlElement_Compare(&a_pValue1->Body.Xml, &a_pValue2->Body.Xml);
		}
	case OpcUa_ExtensionObjectEncoding_EncodeableObject:
		{
#if OPCUA_ENCODEABLE_OBJECT_COMPARE_SUPPORTED
			if(a_pValue1->Body.EncodeableObject.Type->Compare != a_pValue2->Body.EncodeableObject.Type->Compare)
			{
				return 1;
			}

			return a_pValue1->Body.EncodeableObject.Type->Compare(a_pValue1->Body.EncodeableObject.Object, a_pValue2->Body.EncodeableObject.Object);
#else /* OPCUA_ENCODEABLE_OBJECT_COMPARE_SUPPORTED */
			return 1;
#endif /* OPCUA_ENCODEABLE_OBJECT_COMPARE_SUPPORTED */
		}
	case OpcUa_ExtensionObjectEncoding_None:
		{
			return OPCUA_EQUAL;
		}
	default:
		{
			return 1;
		}
	}
}

/*============================================================================
 * OpcUa_ExtensionObject_CopyTo
 *===========================================================================*/
OpcUa_StatusCode OpcUa_ExtensionObject_CopyTo(const OpcUa_ExtensionObject* a_pSource, OpcUa_ExtensionObject* a_pDestination)
{
OpcUa_InitializeStatus(OpcUa_Module_ProxyStub, "OpcUa_ExtensionObject_CopyTo");

	OpcUa_Field_CopyTo(ExpandedNodeId, TypeId);
	OpcUa_Field_CopyToEnumerated(ExtensionObjectEncoding, Encoding);
	OpcUa_Field_CopyToScalar(UInt32, BodySize);

	switch(a_pSource->Encoding)
	{
	case OpcUa_ExtensionObjectEncoding_Binary:
		{
			OpcUa_GotoErrorIfBad(OpcUa_ByteString_CopyTo(&a_pSource->Body.Binary, &a_pDestination->Body.Binary));
			break;
		}
	case OpcUa_ExtensionObjectEncoding_Xml:
		{
			OpcUa_GotoErrorIfBad(OpcUa_XmlElement_CopyTo(&a_pSource->Body.Xml, &a_pDestination->Body.Xml));
			break;
		}
	case OpcUa_ExtensionObjectEncoding_EncodeableObject:
		{
#if OPCUA_ENCODEABLE_OBJECT_COPY_SUPPORTED
			a_pDestination->Body.EncodeableObject.Type = a_pSource->Body.EncodeableObject.Type;
			uStatus = a_pSource->Body.EncodeableObject.Type->Copy(a_pSource->Body.EncodeableObject.Object, &a_pDestination->Body.EncodeableObject.Object);
			OpcUa_GotoErrorIfBad(uStatus);
			break;
#else /* OPCUA_ENCODEABLE_OBJECT_COPY_SUPPORTED */
			a_pDestination->Body.EncodeableObject.Type = a_pSource->Body.EncodeableObject.Type;
			uStatus = OpcUa_EncodeableObject_Default_Copy(a_pSource->Body.EncodeableObject.Object, &a_pDestination->Body.EncodeableObject.Object, a_pSource->Body.EncodeableObject.Type);
			OpcUa_GotoErrorIfBad(uStatus);
			break;
#endif /* OPCUA_ENCODEABLE_OBJECT_COPY_SUPPORTED */
			break;

		}
	case OpcUa_ExtensionObjectEncoding_None:
		{
			break;
		}
	default:
		{
			OpcUa_GotoErrorWithStatus(OpcUa_BadInvalidArgument);
		}
	}

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	OpcUa_ExtensionObject_Clear(a_pDestination);

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_Variant_Initialize
 *===========================================================================*/
OpcUa_Void OpcUa_Variant_Initialize(OpcUa_Variant* a_pValue)
{
	if (a_pValue == OpcUa_Null)
	{
		return;
	}

	OpcUa_MemSet(a_pValue, 0, sizeof(OpcUa_Variant));
}

/*============================================================================
 * OpcUa_Variant_Clear
 *===========================================================================*/
OpcUa_Void OpcUa_Variant_Clear(OpcUa_Variant* a_pValue)
{
	if (a_pValue == OpcUa_Null)
	{
		return;
	}

	if (a_pValue->ArrayType == OpcUa_VariantArrayType_Scalar)
	{
		OpcUa_VariantUnion_Clear(a_pValue->Datatype, &a_pValue->Value);
	}
	else if (a_pValue->ArrayType == OpcUa_VariantArrayType_Array)
	{
		OpcUa_VariantArrayValue_Clear(
			a_pValue->Datatype, 
			a_pValue->Value.Array.Length, 
			&a_pValue->Value.Array.Value);
	}
#if !OPCUA_VARIANT_OMIT_MATRIX
	else if (a_pValue->ArrayType == OpcUa_VariantArrayType_Matrix)
	{
		OpcUa_VariantArrayValue_Clear(
			a_pValue->Datatype, 
			OpcUa_VariantMatrix_GetElementCount(&a_pValue->Value.Matrix), 
			&a_pValue->Value.Matrix.Value);

		if (a_pValue->Value.Matrix.Dimensions != OpcUa_Null)
		{
			OpcUa_Free(a_pValue->Value.Matrix.Dimensions);
		}
	}
#endif /* !OPCUA_VARIANT_OMIT_MATRIX */
	else
	{
		OpcUa_MemSet(a_pValue, 0, sizeof(OpcUa_Variant));
		return;
	}

	a_pValue->ArrayType = 0;
	a_pValue->Datatype = 0;
}

/*============================================================================
 * OpcUa_VariantArrayValue_Clear
 *===========================================================================*/
static OpcUa_Void OpcUa_VariantArrayValue_Clear(
	OpcUa_UInt16             a_uDatatype, 
	OpcUa_Int32              a_iLength, 
	OpcUa_VariantArrayUnion* a_pValue)
{
	OpcUa_Int32 ii = 0;

	if (a_pValue == OpcUa_Null)
	{
		return;
	}

	/* clear each element in the array */
	for (ii = 0; ii < a_iLength; ii++)
	{
		switch (a_uDatatype)
		{   
			case OpcUaType_Null:
			case OpcUaType_Boolean:
			case OpcUaType_SByte:
			case OpcUaType_Byte:
			case OpcUaType_Int16:
			case OpcUaType_UInt16:
			case OpcUaType_Int32:
			case OpcUaType_UInt32:
			case OpcUaType_Int64:
			case OpcUaType_UInt64:
			case OpcUaType_Float:
			case OpcUaType_Double:
			case OpcUaType_DateTime:
			case OpcUaType_Guid:
			case OpcUaType_StatusCode:
			default:
			{
				break;
			}

			case OpcUaType_String:
			{
				OpcUa_String_Clear(&(a_pValue->StringArray[ii]));
				break;
			}

			case OpcUaType_ByteString:
			{
				OpcUa_ByteString_Clear(&a_pValue->ByteStringArray[ii]);
				break;
			}

			case OpcUaType_XmlElement:
			{
				OpcUa_XmlElement_Clear(&a_pValue->XmlElementArray[ii]);
				break;
			}

			case OpcUaType_NodeId:
			{
				OpcUa_NodeId_Clear(&a_pValue->NodeIdArray[ii]);
				break;
			}

			case OpcUaType_ExpandedNodeId:
			{
				OpcUa_ExpandedNodeId_Clear(&a_pValue->ExpandedNodeIdArray[ii]);
				break;
			}

			case OpcUaType_QualifiedName:
			{
				OpcUa_QualifiedName_Clear(&a_pValue->QualifiedNameArray[ii]);
				break;
			}

			case OpcUaType_LocalizedText:
			{
				OpcUa_LocalizedText_Clear(&a_pValue->LocalizedTextArray[ii]);
				break;
			}

			case OpcUaType_ExtensionObject:
			{
				OpcUa_ExtensionObject_Clear(&a_pValue->ExtensionObjectArray[ii]);
				break;
			}

			case OpcUaType_DataValue:
			{
				OpcUa_DataValue_Clear(&a_pValue->DataValueArray[ii]);
				break;
			}

			case OpcUaType_Variant:
			{
				OpcUa_Variant_Clear(&a_pValue->VariantArray[ii]);
				break;
			}
		}
	}

	/* free the memory allocated for the array. */
	OpcUa_Free(a_pValue->Array);
}

/*============================================================================
 * OpcUa_VariantUnion_Clear
 *===========================================================================*/
static OpcUa_Void OpcUa_VariantUnion_Clear(OpcUa_UInt16 datatype, OpcUa_VariantUnion* a_pValue)
{
	if (a_pValue == OpcUa_Null)
	{
		return;
	}

	switch (datatype)
	{   
		case OpcUaType_Null:
		case OpcUaType_Boolean:
		case OpcUaType_SByte:
		case OpcUaType_Byte:
		case OpcUaType_Int16:
		case OpcUaType_UInt16:
		case OpcUaType_Int32:
		case OpcUaType_UInt32:
		case OpcUaType_Int64:
		case OpcUaType_UInt64:
		case OpcUaType_Float:
		case OpcUaType_Double:
		case OpcUaType_DateTime:
		case OpcUaType_StatusCode:
		default:
		{
			break;
		}

		case OpcUaType_Guid:
		{
			OpcUa_Guid_Clear(a_pValue->Guid);
			OpcUa_Free(a_pValue->Guid);
			break;
		}

		case OpcUaType_String:
		{
			OpcUa_String_Clear(&(a_pValue->String));
			break;
		}

		case OpcUaType_ByteString:
		{
			OpcUa_ByteString_Clear(&a_pValue->ByteString);
			break;
		}

		case OpcUaType_XmlElement:
		{
			OpcUa_XmlElement_Clear(&a_pValue->XmlElement);
			break;
		}

		case OpcUaType_NodeId:
		{
			OpcUa_NodeId_Clear(a_pValue->NodeId);
			OpcUa_Free(a_pValue->NodeId);
			break;
		}

		case OpcUaType_ExpandedNodeId:
		{
			OpcUa_ExpandedNodeId_Clear(a_pValue->ExpandedNodeId);
			OpcUa_Free(a_pValue->ExpandedNodeId);
			break;
		}

		case OpcUaType_QualifiedName:
		{
			OpcUa_QualifiedName_Clear(a_pValue->QualifiedName);
			OpcUa_Free(a_pValue->QualifiedName);
			break;
		}

		case OpcUaType_LocalizedText:
		{
			OpcUa_LocalizedText_Clear(a_pValue->LocalizedText);
			OpcUa_Free(a_pValue->LocalizedText);
			break;
		}

		case OpcUaType_DataValue:
		{
			OpcUa_DataValue_Clear(a_pValue->DataValue);
			OpcUa_Free(a_pValue->DataValue);
			break;
		}

		case OpcUaType_ExtensionObject:
		{
			OpcUa_ExtensionObject_Delete(&a_pValue->ExtensionObject);
			break;
		}
	}
}

/*============================================================================
 * OpcUa_VariantUnion_Compare
 *===========================================================================*/
static OpcUa_Int OpcUa_VariantUnion_Compare(OpcUa_UInt16 a_Datatype, const  OpcUa_VariantUnion* a_pValue1, const OpcUa_VariantUnion* a_pValue2)
{
	switch(a_Datatype)
	{
	case OpcUaType_Null:
		{
			return OPCUA_EQUAL;
		}
	case OpcUaType_Boolean:
		{
			OpcUa_Field_Compare(Boolean, Boolean);
			break;
		}
	case OpcUaType_SByte:
		{
			OpcUa_Field_Compare(SByte, SByte);
			break;
		}
	case OpcUaType_Byte:
		{
			OpcUa_Field_Compare(Byte, Byte);
			break;
		}
	case OpcUaType_Int16:
		{
			OpcUa_Field_Compare(Int16, Int16);
			break;
		}
	case OpcUaType_UInt16:
		{
			OpcUa_Field_Compare(UInt16, UInt16);
			break;
		}
	case OpcUaType_Int32:
		{
			OpcUa_Field_Compare(Int32, Int32);
			break;
		}
	case OpcUaType_UInt32:
		{
			OpcUa_Field_Compare(UInt32, UInt32);
			break;
		}
	case OpcUaType_Int64:
		{
			OpcUa_Field_Compare(Int64, Int64);
			break;
		}
	case OpcUaType_UInt64:
		{
			OpcUa_Field_Compare(UInt64, UInt64);
			break;
		}
	case OpcUaType_Float:
		{
			OpcUa_Field_Compare(Float, Float);
			break;
		}
	case OpcUaType_Double:
		{
			OpcUa_Field_Compare(Double, Double);
			break;
		}
	case OpcUaType_DateTime:
		{
			OpcUa_Field_Compare(DateTime, DateTime);
			break;
		}
	case OpcUaType_StatusCode:
		{
			OpcUa_Field_Compare(StatusCode, StatusCode);
			break;
		}
	case OpcUaType_Guid:
		{
			return OpcUa_Guid_Compare(a_pValue1->Guid, a_pValue2->Guid);
		}
	case OpcUaType_String:
		{
			OpcUa_Field_Compare(String, String);
			break;
		}
	case OpcUaType_ByteString:
		{
			OpcUa_Field_Compare(ByteString, ByteString);
			break;
		}
	case OpcUaType_XmlElement:
		{
			OpcUa_Field_Compare(XmlElement, XmlElement);
			break;
		}
	case OpcUaType_NodeId:
		{
			return OpcUa_NodeId_Compare(a_pValue1->NodeId, a_pValue2->NodeId);
		}
	case OpcUaType_ExpandedNodeId:
		{
			return OpcUa_ExpandedNodeId_Compare(a_pValue1->ExpandedNodeId, a_pValue2->ExpandedNodeId);
		}
	case OpcUaType_QualifiedName:
		{
			return OpcUa_QualifiedName_Compare(a_pValue1->QualifiedName, a_pValue2->QualifiedName);
		}
	case OpcUaType_LocalizedText:
		{
			return OpcUa_LocalizedText_Compare(a_pValue1->LocalizedText, a_pValue2->LocalizedText);
		}
	case OpcUaType_DataValue:
		{
			return OpcUa_DataValue_Compare(a_pValue1->DataValue, a_pValue2->DataValue);
		}
	case OpcUaType_ExtensionObject:
		{
			return OpcUa_ExtensionObject_Compare(a_pValue1->ExtensionObject, a_pValue2->ExtensionObject);
		}
	}

	return OPCUA_EQUAL;
}

/*============================================================================
 * OpcUa_VariantUnion_CopyTo
 *===========================================================================*/
static OpcUa_StatusCode OpcUa_VariantUnion_CopyTo(OpcUa_UInt16 a_Datatype, const OpcUa_VariantUnion* a_pSource, OpcUa_VariantUnion* a_pDestination)
{
OpcUa_InitializeStatus(OpcUa_Module_ProxyStub, "OpcUa_VariantUnion_CopyTo");

	switch(a_Datatype)
	{   
	case OpcUaType_Boolean:
		{
			OpcUa_Field_CopyToScalar(Boolean, Boolean);
			break;
		}
	case OpcUaType_SByte:
		{
			OpcUa_Field_CopyToScalar(SByte, SByte);
			break;
		}
	case OpcUaType_Byte:
		{
			OpcUa_Field_CopyToScalar(Byte, Byte);
			break;
		}
	case OpcUaType_Int16:
		{
			OpcUa_Field_CopyToScalar(Int16, Int16);
			break;
		}
	case OpcUaType_UInt16:
		{
			OpcUa_Field_CopyToScalar(UInt16, UInt16);
			break;
		}
	case OpcUaType_Int32:
		{
			OpcUa_Field_CopyToScalar(Int32, Int32);
			break;
		}
	case OpcUaType_UInt32:
		{
			OpcUa_Field_CopyToScalar(UInt32, UInt32);
			break;
		}
	case OpcUaType_Int64:
		{
			OpcUa_Field_CopyToScalar(Int64, Int64);
			break;
		}
	case OpcUaType_UInt64:
		{
			OpcUa_Field_CopyToScalar(UInt64, UInt64);
			break;
		}
	case OpcUaType_Float:
		{
			OpcUa_Field_CopyToScalar(Float, Float);
			break;
		}
	case OpcUaType_Double:
		{
			OpcUa_Field_CopyToScalar(Double, Double);
			break;
		}
	case OpcUaType_DateTime:
		{
			OpcUa_Field_CopyToScalar(DateTime, DateTime);
			break;
		}
	case OpcUaType_StatusCode:
		{
			OpcUa_Field_CopyToScalar(StatusCode, StatusCode);
			break;
		}
	case OpcUaType_Guid:
		{
			OpcUa_Field_Copy(Guid, Guid);
			//uStatus = OpcUa_Guid_Copy(a_pSource->Guid, &a_pDestination->Guid); // modif mai 2015
			//OpcUa_GotoErrorIfBad(uStatus);
			break;
		}

	case OpcUaType_String:
		{
			OpcUa_Field_CopyTo(String, String);
			break;
		}

	case OpcUaType_ByteString:
		{
			OpcUa_Field_CopyTo(ByteString, ByteString);
			break;
		}

	case OpcUaType_XmlElement:
		{
			OpcUa_Field_CopyTo(XmlElement, XmlElement);
			break;
		}

	case OpcUaType_NodeId:
		{
			OpcUa_Field_Copy(NodeId, NodeId);
			break;
		}

	case OpcUaType_ExpandedNodeId:
		{
			OpcUa_Field_Copy(ExpandedNodeId, ExpandedNodeId);
			break;
		}

	case OpcUaType_QualifiedName:
		{
			OpcUa_Field_Copy(QualifiedName, QualifiedName);
			break;
		}

	case OpcUaType_LocalizedText:
		{
			OpcUa_Field_Copy(LocalizedText, LocalizedText);
			break;
		}

	case OpcUaType_DataValue:
		{
			OpcUa_Field_Copy(DataValue, DataValue);
			break;
		}

	case OpcUaType_ExtensionObject:
		{
			OpcUa_Field_Copy(ExtensionObject, ExtensionObject);
			break;
		}
	}

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_VariantArrayValue_Compare
 *===========================================================================*/
static OpcUa_Int OpcUa_VariantArrayValue_Compare(OpcUa_UInt16                    a_Datatype,
												 OpcUa_Int32                     a_iLength,
												 const OpcUa_VariantArrayUnion*  a_pValue1,
												 const OpcUa_VariantArrayUnion*  a_pValue2)
{
	OpcUa_Int32 ii = 0;

	for(ii = 0; ii < a_iLength; ii++)
	{
		switch(a_Datatype)
		{
			/* Todo: Scalar -> memcmp */
		case OpcUaType_Boolean:
			{
				OpcUa_Field_Compare(Boolean, BooleanArray[ii]);
				break;
			}
		case OpcUaType_SByte:
			{
				OpcUa_Field_Compare(SByte, SByteArray[ii]);
				break;
			}
		case OpcUaType_Byte:
			{
				OpcUa_Field_Compare(Byte, ByteArray[ii]);
				break;
			}
		case OpcUaType_Int16:
			{
				OpcUa_Field_Compare(Int16, Int16Array[ii]);
				break;
			}
		case OpcUaType_UInt16:
			{
				OpcUa_Field_Compare(UInt16, UInt16Array[ii]);
				break;
			}
		case OpcUaType_Int32:
			{
				OpcUa_Field_Compare(Int32, Int32Array[ii]);
				break;
			}
		case OpcUaType_UInt32:
			{
				OpcUa_Field_Compare(UInt32, UInt32Array[ii]);
				break;
			}
		case OpcUaType_Int64:
			{
				OpcUa_Field_Compare(Int64, Int64Array[ii]);
				break;
			}
		case OpcUaType_UInt64:
			{
				OpcUa_Field_Compare(UInt64, UInt64Array[ii]);
				break;
			}
		case OpcUaType_Float:
			{
				OpcUa_Field_Compare(Float, FloatArray[ii]);
				break;
			}
		case OpcUaType_Double:
			{
				OpcUa_Field_Compare(Double, DoubleArray[ii]);
				break;
			}
		case OpcUaType_DateTime:
			{
				OpcUa_Field_Compare(DateTime, DateTimeArray[ii]);
				break;
			}
		case OpcUaType_StatusCode:
			{
				OpcUa_Field_Compare(StatusCode, StatusCodeArray[ii]);
				break;
			}
		case OpcUaType_Guid:
			{
				OpcUa_Field_Compare(Guid, GuidArray[ii]);
				break;
			}

		case OpcUaType_String:
			{
				OpcUa_Field_Compare(String, StringArray[ii]);
				break;
			}

		case OpcUaType_ByteString:
			{
				OpcUa_Field_Compare(ByteString, ByteStringArray[ii]);
				break;
			}

		case OpcUaType_XmlElement:
			{
				OpcUa_Field_Compare(XmlElement, XmlElementArray[ii]);
				break;
			}

		case OpcUaType_NodeId:
			{
				OpcUa_Field_Compare(NodeId, NodeIdArray[ii]);
				break;
			}

		case OpcUaType_ExpandedNodeId:
			{
				OpcUa_Field_Compare(ExpandedNodeId, ExpandedNodeIdArray[ii]);
				break;
			}

		case OpcUaType_QualifiedName:
			{
				OpcUa_Field_Compare(QualifiedName, QualifiedNameArray[ii]);
				break;
			}

		case OpcUaType_LocalizedText:
			{
				OpcUa_Field_Compare(LocalizedText, LocalizedTextArray[ii]);
				break;
			}

		case OpcUaType_DataValue:
			{
				OpcUa_Field_Compare(DataValue, DataValueArray[ii]);
				break;
			}

		case OpcUaType_Variant:
			{
				OpcUa_Field_Compare(Variant, VariantArray[ii]);
				break;
			}

		case OpcUaType_ExtensionObject:
			{
				OpcUa_Field_Compare(ExtensionObject, ExtensionObjectArray[ii]);
				break;
			}
		}
	}

	return OPCUA_EQUAL;
}

/*============================================================================
 * OpcUa_VariantArrayValue_CreateArray
 *===========================================================================*/
static OpcUa_StatusCode OpcUa_VariantArrayValue_CreateArray(OpcUa_UInt16    a_Datatype,
															OpcUa_Int32     a_iNumberOfElements,
															OpcUa_Void**    a_ppArray)
{
	OpcUa_Int iTypeSize = 0;

OpcUa_InitializeStatus(OpcUa_Module_ProxyStub, "OpcUa_VariantArrayValue_CreateArray");

	switch(a_Datatype)
	{
	case OpcUaType_Boolean:
		{
			iTypeSize = sizeof(OpcUa_Boolean);
			break;
		}
	case OpcUaType_SByte:
		{
			iTypeSize = sizeof(OpcUa_SByte);
			break;
		}
	case OpcUaType_Byte:
		{
			iTypeSize = sizeof(OpcUa_Byte);
			break;
		}
	case OpcUaType_Int16:
		{
			iTypeSize = sizeof(OpcUa_Int16);
			break;
		}
	case OpcUaType_UInt16:
		{
			iTypeSize = sizeof(OpcUa_UInt16);
			break;
		}
	case OpcUaType_Int32:
		{
			iTypeSize = sizeof(OpcUa_Int32);
			break;
		}
	case OpcUaType_UInt32:
		{
			iTypeSize = sizeof(OpcUa_UInt32);
			break;
		}
	case OpcUaType_Int64:
		{
			iTypeSize = sizeof(OpcUa_Int64);
			break;
		}
	case OpcUaType_UInt64:
		{
			iTypeSize = sizeof(OpcUa_UInt64);
			break;
		}
	case OpcUaType_Float:
		{
			iTypeSize = sizeof(OpcUa_Float);
			break;
		}
	case OpcUaType_Double:
		{
			iTypeSize = sizeof(OpcUa_Double);
			break;
		}
	case OpcUaType_DateTime:
		{
			iTypeSize = sizeof(OpcUa_DateTime);
			break;
		}
	case OpcUaType_StatusCode:
		{
			iTypeSize = sizeof(OpcUa_StatusCode);
			break;
		}
	case OpcUaType_Guid:
		{
			iTypeSize = sizeof(OpcUa_Guid);
			break;
		}

	case OpcUaType_String:
		{
			iTypeSize = sizeof(OpcUa_String);
			break;
		}

	case OpcUaType_ByteString:
		{
			iTypeSize = sizeof(OpcUa_ByteString);
			break;
		}

	case OpcUaType_XmlElement:
		{
			iTypeSize = sizeof(OpcUa_XmlElement);
			break;
		}

	case OpcUaType_NodeId:
		{
			iTypeSize = sizeof(OpcUa_NodeId);
			break;
		}

	case OpcUaType_ExpandedNodeId:
		{
			iTypeSize = sizeof(OpcUa_ExpandedNodeId);
			break;
		}

	case OpcUaType_QualifiedName:
		{
			iTypeSize = sizeof(OpcUa_QualifiedName);
			break;
		}

	case OpcUaType_LocalizedText:
		{
			iTypeSize = sizeof(OpcUa_LocalizedText);
			break;
		}

	case OpcUaType_DataValue:
		{
			iTypeSize = sizeof(OpcUa_DataValue);
			break;
		}

	case OpcUaType_ExtensionObject:
		{
			iTypeSize = sizeof(OpcUa_ExtensionObject);
			break;
		}

	case OpcUaType_Variant:
		{
			iTypeSize = sizeof(OpcUa_Variant);
			break;
		}

	default:
		{
			OpcUa_GotoErrorWithStatus(OpcUa_BadInvalidArgument);
		}
	}

	*a_ppArray = OpcUa_Alloc(iTypeSize * a_iNumberOfElements);
	OpcUa_GotoErrorIfAllocFailed(*a_ppArray);
	OpcUa_MemSet(*a_ppArray, 0, iTypeSize * a_iNumberOfElements);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_VariantArrayValue_CopyTo
 *===========================================================================*/
static OpcUa_StatusCode OpcUa_VariantArrayValue_CopyTo( OpcUa_UInt16                    a_Datatype,
														OpcUa_Int32                     a_iLength,
														const OpcUa_VariantArrayUnion*  a_pSource,
														const OpcUa_VariantArrayUnion*  a_pDestination)
{
	OpcUa_Int32 ii = 0;

OpcUa_InitializeStatus(OpcUa_Module_ProxyStub, "OpcUa_VariantArrayValue_CopyTo");

	for(ii = 0; ii < a_iLength; ii++)
	{
		switch(a_Datatype)
		{
			/* Todo: Scalar -> memcpy */
		case OpcUaType_Boolean:
			{
				OpcUa_Field_CopyToScalar(Boolean, BooleanArray[ii]);
				break;
			}
		case OpcUaType_SByte:
			{
				OpcUa_Field_CopyToScalar(SByte, SByteArray[ii]);
				break;
			}
		case OpcUaType_Byte:
			{
				OpcUa_Field_CopyToScalar(Byte, ByteArray[ii]);
				break;
			}
		case OpcUaType_Int16:
			{
				OpcUa_Field_CopyToScalar(Int16, Int16Array[ii]);
				break;
			}
		case OpcUaType_UInt16:
			{
				OpcUa_Field_CopyToScalar(UInt16, UInt16Array[ii]);
				break;
			}
		case OpcUaType_Int32:
			{
				OpcUa_Field_CopyToScalar(Int32, Int32Array[ii]);
				break;
			}
		case OpcUaType_UInt32:
			{
				OpcUa_Field_CopyToScalar(UInt32, UInt32Array[ii]);
				break;
			}
		case OpcUaType_Int64:
			{
				OpcUa_Field_CopyToScalar(Int64, Int64Array[ii]);
				break;
			}
		case OpcUaType_UInt64:
			{
				OpcUa_Field_CopyToScalar(UInt64, UInt64Array[ii]);
				break;
			}
		case OpcUaType_Float:
			{
				OpcUa_Field_CopyToScalar(Float, FloatArray[ii]);
				break;
			}
		case OpcUaType_Double:
			{
				OpcUa_Field_CopyToScalar(Double, DoubleArray[ii]);
				break;
			}
		case OpcUaType_DateTime:
			{
				OpcUa_Field_CopyToScalar(DateTime, DateTimeArray[ii]);
				break;
			}
		case OpcUaType_StatusCode:
			{
				OpcUa_Field_CopyToScalar(StatusCode, StatusCodeArray[ii]);
				break;
			}
		case OpcUaType_Guid:
			{
				OpcUa_Field_CopyTo(Guid, GuidArray[ii]);
				break;
			}

		case OpcUaType_String:
			{
				OpcUa_Field_CopyTo(String, StringArray[ii]);
				break;
			}

		case OpcUaType_ByteString:
			{
				OpcUa_Field_CopyTo(ByteString, ByteStringArray[ii]);
				break;
			}

		case OpcUaType_XmlElement:
			{
				OpcUa_Field_CopyTo(XmlElement, XmlElementArray[ii]);
				break;
			}

		case OpcUaType_NodeId:
			{
				OpcUa_Field_CopyTo(NodeId, NodeIdArray[ii]);
				break;
			}

		case OpcUaType_ExpandedNodeId:
			{
				OpcUa_Field_CopyTo(ExpandedNodeId, ExpandedNodeIdArray[ii]);
				break;
			}

		case OpcUaType_QualifiedName:
			{
				OpcUa_Field_CopyTo(QualifiedName, QualifiedNameArray[ii]);
				break;
			}

		case OpcUaType_LocalizedText:
			{
				OpcUa_Field_CopyTo(LocalizedText, LocalizedTextArray[ii]);
				break;
			}

		case OpcUaType_DataValue:
			{
				OpcUa_Field_CopyTo(DataValue, DataValueArray[ii]);
				break;
			}

		case OpcUaType_Variant:
			{
				OpcUa_Field_CopyTo(Variant, VariantArray[ii]);
				break;
			}

		case OpcUaType_ExtensionObject:
			{
				OpcUa_Field_CopyTo(ExtensionObject, ExtensionObjectArray[ii]);
				break;
			}
		}
	}

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_Variant_Compare
 *===========================================================================*/
OpcUa_Int OpcUa_Variant_Compare(const OpcUa_Variant* a_pValue1, const OpcUa_Variant* a_pValue2)
{
	if(a_pValue1 == a_pValue2)
	{
		return OPCUA_EQUAL;
	}

	if((a_pValue1 == OpcUa_Null) || (a_pValue2 == OpcUa_Null))
	{
		return 1;
	}

	OpcUa_Field_Compare(Byte, Datatype);
	OpcUa_Field_Compare(Byte, ArrayType);
   
	if(a_pValue1->ArrayType == OpcUa_VariantArrayType_Scalar)
	{
		return OpcUa_VariantUnion_Compare(  a_pValue1->Datatype,
										   &a_pValue1->Value,
										   &a_pValue2->Value);
	}
	else if(a_pValue1->ArrayType == OpcUa_VariantArrayType_Array)
	{
		OpcUa_Field_Compare(Int32, Value.Array.Length);

		return OpcUa_VariantArrayValue_Compare( a_pValue1->Datatype,
												a_pValue1->Value.Array.Length,
											   &a_pValue1->Value.Array.Value,
											   &a_pValue2->Value.Array.Value);
	}
#if !OPCUA_VARIANT_OMIT_MATRIX
	else if (a_pValue1->ArrayType == OpcUa_VariantArrayType_Matrix)
	{
		OpcUa_Int32 iElementCount1 = OpcUa_VariantMatrix_GetElementCount(&a_pValue1->Value.Matrix);
		OpcUa_Int32 iElementCount2 = OpcUa_VariantMatrix_GetElementCount(&a_pValue2->Value.Matrix);

		if(iElementCount1 != iElementCount2)
		{
			return 1;
		}

		return OpcUa_VariantArrayValue_Compare( a_pValue1->Datatype,
												iElementCount1, 
											   &a_pValue1->Value.Matrix.Value,
											   &a_pValue2->Value.Matrix.Value);
	}
#endif /* !OPCUA_VARIANT_OMIT_MATRIX */

	return OPCUA_EQUAL;
}

/*============================================================================
 * OpcUa_Variant_CopyTo
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Variant_CopyTo(const OpcUa_Variant* a_pSource, OpcUa_Variant* a_pDestination)
{
OpcUa_InitializeStatus(OpcUa_Module_ProxyStub, "OpcUa_Variant_CopyTo");

	OpcUa_ReturnErrorIfArgumentNull(a_pSource);
	OpcUa_ReturnErrorIfArgumentNull(a_pDestination);

	OpcUa_Variant_Initialize(a_pDestination);

	OpcUa_Field_CopyToScalar(Byte, Datatype);
	OpcUa_Field_CopyToScalar(Byte, ArrayType);

	if(a_pSource->ArrayType == OpcUa_VariantArrayType_Scalar)
	{
		return OpcUa_VariantUnion_CopyTo(   a_pSource->Datatype,
										   &a_pSource->Value,
										   &a_pDestination->Value);
	}
	else if(a_pSource->ArrayType == OpcUa_VariantArrayType_Array)
	{
		OpcUa_Field_CopyToScalar(Int32, Value.Array.Length);

		OpcUa_GotoErrorIfBad(OpcUa_VariantArrayValue_CreateArray(   a_pDestination->Datatype,
																	a_pDestination->Value.Array.Length,
																   &a_pDestination->Value.Array.Value.Array));

		OpcUa_GotoErrorIfBad(OpcUa_VariantArrayValue_CopyTo(    a_pSource->Datatype,
																a_pSource->Value.Array.Length,
															   &a_pSource->Value.Array.Value,
															   &a_pDestination->Value.Array.Value));
	}
#if !OPCUA_VARIANT_OMIT_MATRIX
	else if (a_pSource->ArrayType == OpcUa_VariantArrayType_Matrix)
	{
		OpcUa_Int32 iElementCount1 = OpcUa_VariantMatrix_GetElementCount(&a_pSource->Value.Matrix);

		a_pDestination->Value.Matrix.NoOfDimensions = a_pSource->Value.Matrix.NoOfDimensions;

		if(a_pDestination->Value.Matrix.NoOfDimensions > 0)
		{
			a_pDestination->Value.Matrix.Dimensions = (OpcUa_Int32*)OpcUa_Alloc(a_pDestination->Value.Matrix.NoOfDimensions * sizeof(OpcUa_Int32));
			OpcUa_GotoErrorIfAllocFailed(a_pDestination->Value.Matrix.Dimensions);

			OpcUa_MemCpy(   a_pDestination->Value.Matrix.Dimensions,
							a_pDestination->Value.Matrix.NoOfDimensions * sizeof(OpcUa_Int32),
							a_pSource->Value.Matrix.Dimensions,
							a_pSource->Value.Matrix.NoOfDimensions * sizeof(OpcUa_Int32));
		}

		OpcUa_GotoErrorIfBad(OpcUa_VariantArrayValue_CreateArray(   a_pDestination->Datatype,
																	iElementCount1,
																   &a_pDestination->Value.Matrix.Value.Array));

		OpcUa_GotoErrorIfBad(OpcUa_VariantArrayValue_CopyTo(a_pSource->Datatype,
															iElementCount1, 
														   &a_pSource->Value.Matrix.Value,
														   &a_pDestination->Value.Matrix.Value));
	}
#endif /* !OPCUA_VARIANT_OMIT_MATRIX */

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	OpcUa_Variant_Clear(a_pDestination);

OpcUa_FinishErrorHandling;
}
