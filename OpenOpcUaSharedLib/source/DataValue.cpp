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
#include "stdafx.h"
#include "DataValue.h"
using namespace OpenOpcUa;
using namespace UASharedLib;
//////////////////////////////////////////////////////////////////////////
//
//
#define OpcUa_ExtensionObject_Scalar_Copy() \
{\
if (m_pInternalDataValue->Value.Value.ExtensionObject)\
	OpcUa_Free(m_pInternalDataValue->Value.Value.ExtensionObject); \
	m_pInternalDataValue->Value.Value.ExtensionObject=(OpcUa_ExtensionObject*)OpcUa_Alloc(sizeof(OpcUa_ExtensionObject)); \
	m_pInternalDataValue->Value.Datatype=OpcUaType_ExtensionObject; \
	m_pInternalDataValue->Value.Value.ExtensionObject->BodySize=0; \
	m_pInternalDataValue->Value.Value.ExtensionObject->Encoding=OpcUa_ExtensionObjectEncoding_EncodeableObject; \
	OpcUa_ExpandedNodeId_Initialize(&(m_pInternalDataValue->Value.Value.ExtensionObject->TypeId)); \
	m_pInternalDataValue->Value.Value.ExtensionObject->TypeId.ServerIndex=0; \
	m_pInternalDataValue->Value.Value.ExtensionObject->TypeId.NamespaceUri; \
	m_pInternalDataValue->Value.Value.ExtensionObject->TypeId.NodeId.Identifier.Numeric=pEncodeableType->TypeId; \
	m_pInternalDataValue->Value.Value.ExtensionObject->TypeId.NodeId.IdentifierType=OpcUa_IdentifierType_Numeric; \
	m_pInternalDataValue->Value.Value.ExtensionObject->TypeId.NodeId.NamespaceIndex=0; \
	m_pInternalDataValue->Value.Value.ExtensionObject->Body.EncodeableObject.Type=pEncodeableType; \
	if (m_pInternalDataValue->Value.Value.ExtensionObject->Body.EncodeableObject.Object)\
		OpcUa_Free(m_pInternalDataValue->Value.Value.ExtensionObject->Body.EncodeableObject.Object);\
		m_pInternalDataValue->Value.Value.ExtensionObject->Body.EncodeableObject.Object=OpcUa_Alloc(pEncodeableType->AllocationSize); \
	m_pInternalDataValue->Value.Value.ExtensionObject->Body.EncodeableObject.Object=Value.Value.ExtensionObject->Body.EncodeableObject.Object; \
}

#define TransfertArray(name)\
	if (Value.Value.Array.Length)\
		{\
		m_pInternalDataValue->Value.Value.Array.Value.name##Array = Value.Value.Array.Value.name##Array; \
		m_pInternalDataValue->Value.Value.Array.Length = Value.Value.Array.Length; \
		}

#define InitializeVariantArray(name)\
{\
	m_pInternalDataValue->Value.Datatype=builtInType;\
	if (uiVal)\
	{\
		if (m_pInternalDataValue->Value.Value.Array.Value.name##Array)\
			OpcUa_Free(m_pInternalDataValue->Value.Value.Array.Value.name##Array);\
		m_pInternalDataValue->Value.Value.Array.Value.name##Array = (OpcUa_##name*)OpcUa_Alloc(uiVal*dataTypeSize);\
		for (OpcUa_UInt32 ii=0;ii<uiVal;ii++)\
			OpcUa_##name##_Initialize(&(m_pInternalDataValue->Value.Value.Array.Value.name##Array[ii]));\
	}\
	else\
		m_pInternalDataValue->Value.Value.Array.Value.name##Array=OpcUa_Null;\
}

#define TransfertCopyArray(name)\
	if (Value.Value.Array.Length)\
	{\
		m_pInternalDataValue->Value.Value.Array.Value.name##Array=Value.Value.Array.Value.name##Array;\
		m_pInternalDataValue->Value.Value.Array.Length=Value.Value.Array.Length;\
	}
CDataValue::CDataValue(void)
{
	m_pInternalDataValue=(OpcUa_DataValue*)OpcUa_Alloc(sizeof(OpcUa_DataValue));
	if (m_pInternalDataValue)
	{
		OpcUa_DataValue_Initialize(m_pInternalDataValue);
		m_pInternalDataValue ->StatusCode= OpcUa_UncertainInitialValue;
	}
	// will set up the timesptamp
	SetServerTimestamp(OpcUa_DateTime_UtcNow());
	SetSourceTimestamp(OpcUa_DateTime_UtcNow());
	m_ServerPicoseconds=0;
	m_SourcePicoseconds=0;
}
CDataValue::CDataValue(OpcUa_DataValue* pDataValue)
{
	if (pDataValue)
	{
		m_pInternalDataValue = OpcUa_Null;
		m_pInternalDataValue = (OpcUa_DataValue*)OpcUa_Alloc(sizeof(OpcUa_DataValue));
		OpcUa_DataValue_Initialize(m_pInternalDataValue);
		OpcUa_Boolean bOk = OpcUa_False;
		if (pDataValue->Value.Datatype == OpcUaType_ExtensionObject)
		{
			if ((pDataValue->Value.ArrayType == OpcUa_VariantArrayType_Scalar) && (pDataValue->Value.Value.ExtensionObject))
				bOk = OpcUa_True;
			else
			{
				if ((pDataValue->Value.ArrayType == OpcUa_VariantArrayType_Array) && (pDataValue->Value.Value.Array.Value.ExtensionObjectArray))
					bOk = OpcUa_True;
			}
		}
		else
			bOk = OpcUa_True;
		if (bOk)
			OpcUa_DataValue_CopyTo(pDataValue, m_pInternalDataValue);
		else
		{
			SetServerTimestamp(pDataValue->ServerTimestamp);
			SetServerPicoseconds(pDataValue->ServerPicoseconds);
			SetSourceTimestamp(pDataValue->SourceTimestamp);
			SetSourcePicoseconds(pDataValue->SourcePicoseconds);
			SetStatusCode(OpcUa_BadWaitingForInitialData);
		}
	}
	else
		m_pInternalDataValue = OpcUa_Null;
}
CDataValue::CDataValue(OpcUa_Byte builtInType)
{
	m_ServerPicoseconds = 0;
	m_SourcePicoseconds = 0;
	m_pInternalDataValue = (OpcUa_DataValue*)OpcUa_Alloc(sizeof(OpcUa_DataValue));
	if (m_pInternalDataValue)
	{
		OpcUa_DataValue_Initialize(m_pInternalDataValue);
		OpcUa_Variant_Initialize(&(m_pInternalDataValue->Value));
		if ((builtInType<24) && (builtInType>0))
			Initialize(builtInType, -1);
	}
}
CDataValue::~CDataValue(void)
{
	if (m_pInternalDataValue)
	{
		/*
		if (m_pInternalDataValue->Value.Datatype == OpcUaType_ExtensionObject)
		{
		if (m_pInternalDataValue->Value.Value.ExtensionObject)
		{
		if (m_pInternalDataValue->Value.ArrayType == OpcUa_VariantArrayType_Scalar)
		{
		if (m_pInternalDataValue->Value.Value.ExtensionObject->Body.EncodeableObject.Type)
		{
		OpcUa_Free(m_pInternalDataValue->Value.Value.ExtensionObject->Body.EncodeableObject.Type);// modif mai 2015
		m_pInternalDataValue->Value.Value.ExtensionObject->Body.EncodeableObject.Type = OpcUa_Null;
		}
		if (m_pInternalDataValue->Value.Value.ExtensionObject->Body.EncodeableObject.Object)
		{
		OpcUa_Free(m_pInternalDataValue->Value.Value.ExtensionObject->Body.EncodeableObject.Object);
		m_pInternalDataValue->Value.Value.ExtensionObject->Body.EncodeableObject.Object = OpcUa_Null;
		}
		}
		else
		{
		if (m_pInternalDataValue->Value.ArrayType == OpcUa_VariantArrayType_Array)
		{
		for (OpcUa_Int32 i = 0; i < m_pInternalDataValue->Value.Value.Array.Length; i++)
		{
		if (m_pInternalDataValue->Value.Value.Array.Value.ExtensionObjectArray[i].Body.EncodeableObject.Type)
		{
		OpcUa_Free(m_pInternalDataValue->Value.Value.Array.Value.ExtensionObjectArray[i].Body.EncodeableObject.Type); // modif mai 2015
		m_pInternalDataValue->Value.Value.Array.Value.ExtensionObjectArray[i].Body.EncodeableObject.Type = OpcUa_Null;
		}
		if (m_pInternalDataValue->Value.Value.Array.Value.ExtensionObjectArray[i].Body.EncodeableObject.Object)
		{
		OpcUa_Free(m_pInternalDataValue->Value.Value.Array.Value.ExtensionObjectArray[i].Body.EncodeableObject.Object);
		m_pInternalDataValue->Value.Value.Array.Value.ExtensionObjectArray[i].Body.EncodeableObject.Object = OpcUa_Null;
		}
		}
		}
		}
		}
		}
		*/
		OpcUa_DataValue_Clear(m_pInternalDataValue);
		OpcUa_Free(m_pInternalDataValue);
		m_pInternalDataValue = OpcUa_Null;
	}
}
OpcUa_StatusCode CDataValue::Initialize(OpcUa_Byte builtInType,
	OpcUa_Int32 ValueRank,
	std::vector<OpcUa_UInt32>* ArrayDim)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	if (m_pInternalDataValue)
	{
		// will set up the timesptamp
		SetServerTimestamp(OpcUa_DateTime_UtcNow());
		SetSourceTimestamp(OpcUa_DateTime_UtcNow());
		m_pInternalDataValue->StatusCode = OpcUa_BadWaitingForInitialData;
		switch (ValueRank)
		{
		case -3:
			OpcUa_Variant_Initialize(&(m_pInternalDataValue->Value));
			break;
		case -2:
		{
			// Il peut s'agir d'un scalaire ou d'un tableau de n'importe quel dimension (vecteur ou matrice)
			// ATTENTION ATTENTION
			// Decision arbitraire
			// a défaut d'indication supplémentaire je considère que j'ai à faire à un scalaire
			m_pInternalDataValue->Value.ArrayType = OpcUa_VariantArrayType_Scalar;
			m_pInternalDataValue->Value.Value.Array.Length = 0; // un seul element dans le tableau
			uStatus = InitializeScalar(/*aNodeId,*/builtInType);
		}
		break;
		case -1:
		{
			// il ne s'agit pas d'un tableau
			m_pInternalDataValue->Value.ArrayType = OpcUa_VariantArrayType_Scalar;
			uStatus = InitializeScalar(/*aNodeId,*/builtInType);
		}
		break;
		case 0:
		{
			// il s'agit pas d'un tableau à une ou plusieurs dimension
			m_pInternalDataValue->Value.ArrayType = OpcUa_VariantArrayType_Scalar;
			uStatus = InitializeScalar(/*aNodeId,*/builtInType);
		}
		break;
		case 1:
		{
			// il agit d'un tableau à 1 dimension (vecteur)
			// Initialisation du vecteur
			m_pInternalDataValue->Value.ArrayType = OpcUa_VariantArrayType_Array;
			if (ArrayDim)
			{
				if (ArrayDim->size() > 0)
				{
					for (OpcUa_UInt32 ii = 0; ii < ArrayDim->size(); ii++)
					{
						OpcUa_UInt32 uiVal = ArrayDim->at(ii);
						m_pInternalDataValue->Value.Value.Array.Length = uiVal*ValueRank;
						uStatus = InitializeArray(builtInType, uiVal);
					}
				}
				else
					uStatus = InitializeArray(builtInType, 0);
			}
			else
			{
				// on initialise un tableau avec un seul element
				// il s'agit sans doute d'un tableau dynamique
				uStatus = InitializeArray(/*aNodeId,*/builtInType, 1);
			}
		}
		break;
		default:
			OpcUa_Variant_Initialize(&(m_pInternalDataValue->Value));
			break;
		}
		if (ValueRank > 1)
		{
			// il s'agit d'un tableau de dimension ValueRank (Matrice)
			m_pInternalDataValue->Value.ArrayType = OpcUa_VariantArrayType_Matrix;
		}
	}
	else
		uStatus = OpcUa_BadOutOfMemory;
	return uStatus;
}
OpcUa_StatusCode CDataValue::InitializeScalar(/*OpcUa_NodeId DataType,*/OpcUa_Byte builtInType)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	if (m_pInternalDataValue)
	{
		OpcUa_Variant_Initialize(&(m_pInternalDataValue->Value));
		switch (builtInType)
		{
		case OpcUaType_Boolean: //boolean
			m_pInternalDataValue->Value.Datatype = OpcUaType_Boolean;
			break;
		case OpcUaType_SByte:
			m_pInternalDataValue->Value.Datatype = OpcUaType_SByte;
			break;
		case OpcUaType_Byte:
			m_pInternalDataValue->Value.Datatype = OpcUaType_Byte;
			break;
		case OpcUaType_Int16:
			m_pInternalDataValue->Value.Datatype = OpcUaType_Int16;
			break;
		case OpcUaType_UInt16:
			m_pInternalDataValue->Value.Datatype = OpcUaType_UInt16;
			break;
		case OpcUaType_Int32:
			m_pInternalDataValue->Value.Datatype = OpcUaType_Int32;
			break;
		case OpcUaType_UInt32:
			m_pInternalDataValue->Value.Datatype = OpcUaType_UInt32;
			break;
		case OpcUaType_Int64:
			m_pInternalDataValue->Value.Datatype = OpcUaType_Int64;
			break;
		case OpcUaType_UInt64:
			m_pInternalDataValue->Value.Datatype = OpcUaType_UInt64;
			break;
		case OpcUaType_Float:
			m_pInternalDataValue->Value.Datatype = OpcUaType_Float;
			break;
		case OpcUaType_Double:
			m_pInternalDataValue->Value.Datatype = OpcUaType_Double;
			break;
		case OpcUaType_String:
			m_pInternalDataValue->Value.Datatype = OpcUaType_String;
			break;
		case OpcUaType_DateTime:
			m_pInternalDataValue->Value.Datatype = OpcUaType_DateTime;
			break;
		case OpcUaType_Guid:
			m_pInternalDataValue->Value.Datatype = OpcUaType_Guid;
			m_pInternalDataValue->Value.Value.Guid = (OpcUa_Guid*)OpcUa_Alloc(sizeof(OpcUa_Guid));
			OpcUa_Guid_Initialize(m_pInternalDataValue->Value.Value.Guid);
			break;
		case OpcUaType_ByteString:
			OpcUa_ByteString_Initialize(&(m_pInternalDataValue->Value.Value.ByteString));
			m_pInternalDataValue->Value.Datatype = OpcUaType_ByteString;
			break;
		case OpcUaType_XmlElement:
			m_pInternalDataValue->Value.Datatype = OpcUaType_XmlElement;
			break;
		case OpcUaType_NodeId:
			m_pInternalDataValue->Value.Datatype = OpcUaType_NodeId;
			m_pInternalDataValue->Value.Value.NodeId = (OpcUa_NodeId*)OpcUa_Alloc(sizeof(OpcUa_NodeId));
			OpcUa_NodeId_Initialize(m_pInternalDataValue->Value.Value.NodeId);
			break;
		case OpcUaType_ExpandedNodeId:
			m_pInternalDataValue->Value.Datatype = OpcUaType_ExpandedNodeId;
			break;
		case OpcUaType_StatusCode:
			m_pInternalDataValue->Value.Datatype = OpcUaType_StatusCode;
			break;
		case OpcUaType_QualifiedName:
			m_pInternalDataValue->Value.Datatype = OpcUaType_QualifiedName;
			m_pInternalDataValue->Value.Value.QualifiedName = (OpcUa_QualifiedName*)OpcUa_Alloc(sizeof(OpcUa_QualifiedName));
			OpcUa_QualifiedName_Initialize(m_pInternalDataValue->Value.Value.QualifiedName);
			//OpcUa_String_AttachCopy(&(m_pInternalDataValue->Value.Value.QualifiedName->Name), "-");
			break;
		case OpcUaType_LocalizedText:
			m_pInternalDataValue->Value.Datatype = OpcUaType_LocalizedText;
			m_pInternalDataValue->Value.Value.LocalizedText = (OpcUa_LocalizedText*)OpcUa_Alloc(sizeof(OpcUa_LocalizedText));
			OpcUa_LocalizedText_Initialize(m_pInternalDataValue->Value.Value.LocalizedText);
			//OpcUa_String_AttachCopy(&(m_pInternalDataValue->Value.Value.LocalizedText->Locale), "en");
			//OpcUa_String_AttachCopy(&(m_pInternalDataValue->Value.Value.LocalizedText->Text), "*");
			break;
		case OpcUaType_ExtensionObject:
			m_pInternalDataValue->Value.Datatype = OpcUaType_ExtensionObject;
			OpcUa_ExtensionObject_Create(&(m_pInternalDataValue->Value.Value.ExtensionObject));
			OpcUa_ExtensionObject_Initialize(m_pInternalDataValue->Value.Value.ExtensionObject);
			//m_Value.Value.ExtensionObject=(OpcUa_ExtensionObject*)OpcUa_Alloc(sizeof(OpcUa_ExtensionObject));
			//OpcUa_ExtensionObject_Initialize(m_Value.Value.ExtensionObject);
			break;
		case OpcUaType_DataValue:
			m_pInternalDataValue->Value.Datatype = OpcUaType_DataValue;
			break;
		case OpcUaType_Variant:
			m_pInternalDataValue->Value.Datatype = OpcUaType_Variant;
			break;
		case OpcUaType_DiagnosticInfo:
			m_pInternalDataValue->Value.Datatype = OpcUaType_DiagnosticInfo;
			break;
		default:
			uStatus = OpcUa_Bad;
			break;
		}
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}

// Function name   : CDataValue::InitializeArray
// Description     : Initialise un tableau à une dimension (vecteur) contenant des builtInType
// Return type     : OpcUa_StatusCode 
// Argument        : OpcUa_Byte builtInType : Type element dans le vecteur
// Argument        : OpcUa_UInt32 uiVal : Nombre d'element dans le vecteur

OpcUa_StatusCode CDataValue::InitializeArray(/*OpcUa_NodeId DataType,*/OpcUa_Byte builtInType, OpcUa_UInt32 uiVal)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_Int16 dataTypeSize=0;
	if (m_pInternalDataValue)
	{
		OpcUa_Variant_InitializeArray(&m_pInternalDataValue->Value, 1);
		m_pInternalDataValue->Value.ArrayType = OpcUa_VariantArrayType_Array;
		if (Utils::GetDataTypesize(builtInType, &dataTypeSize) == OpcUa_Good)
		{
			switch (builtInType)
			{
			case     OpcUaType_Null:
				if (uiVal > 0)
					m_pInternalDataValue->Value.Value.Array.Value.Array = OpcUa_Alloc(uiVal*dataTypeSize);
				else
					m_pInternalDataValue->Value.Value.Array.Value.Array = OpcUa_Null;
				m_pInternalDataValue->Value.Datatype = builtInType;
				break;
			case	OpcUaType_Boolean:
				InitializeVariantArray(Boolean);
				break;
			case	OpcUaType_SByte:
				InitializeVariantArray(SByte);
				break;
			case	OpcUaType_Byte:
				InitializeVariantArray(Byte);
				break;
			case	OpcUaType_Int16:
				InitializeVariantArray(Int16);
				break;
			case	OpcUaType_UInt16:
				InitializeVariantArray(UInt16);
				break;
			case	OpcUaType_Int32:
				InitializeVariantArray(Int32);
				break;
			case	OpcUaType_UInt32:
				InitializeVariantArray(UInt32);
				break;
			case	OpcUaType_Int64:
				InitializeVariantArray(Int64);
				break;
			case	OpcUaType_UInt64:
				InitializeVariantArray(UInt64);
				break;
			case	OpcUaType_Float:
				InitializeVariantArray(Float);
				break;
			case	OpcUaType_Double:
				InitializeVariantArray(Double);
				break;
			case	OpcUaType_String:
				InitializeVariantArray(String);
				break;
			case	OpcUaType_DateTime:
				InitializeVariantArray(DateTime);
				break;
			case	OpcUaType_Guid:
				InitializeVariantArray(Guid);
				break;
			case	OpcUaType_ByteString:
				InitializeVariantArray(ByteString);
				break;
			case	OpcUaType_XmlElement:
				InitializeVariantArray(XmlElement);
				break;
			case	OpcUaType_NodeId:
				InitializeVariantArray(NodeId);
				break;
			case	OpcUaType_ExpandedNodeId:
				InitializeVariantArray(ExpandedNodeId);
				break;
			case	OpcUaType_StatusCode:
				InitializeVariantArray(StatusCode);
				break;
			case	OpcUaType_QualifiedName:
				InitializeVariantArray(QualifiedName);
				break;
			case	OpcUaType_LocalizedText:
				InitializeVariantArray(LocalizedText);
				break;
			case	OpcUaType_ExtensionObject:
				InitializeVariantArray(ExtensionObject);
				break;
			case	OpcUaType_DataValue:
				InitializeVariantArray(DataValue);
				break;
			case	OpcUaType_Variant:
				InitializeVariantArray(Variant);
				break;
			default:
				uStatus = OpcUa_BadInvalidArgument;
				break;
			}
			if (uStatus == OpcUa_Good)
				m_pInternalDataValue->Value.Value.Array.Length = uiVal;
		}
		else
			uStatus = OpcUa_BadInvalidArgument;
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}
OpcUa_Variant CDataValue::GetValue()
{
	return m_pInternalDataValue->Value;
}
// renvoi le pointeur sur l'ExtensionObjext a l'interieur du m_Value
// si il contient un autre type retourne une erreur
// si il ne s'agit pas d'un scalaire retourne une erreur
OpcUa_ExtensionObject* CDataValue::GetInternalExtensionObject()
{
	OpcUa_ExtensionObject* pExtensionObject = OpcUa_Null;
	if (m_pInternalDataValue)
	{
		if (m_pInternalDataValue->Value.ArrayType == OpcUa_VariantArrayType_Scalar)
		{
			pExtensionObject = m_pInternalDataValue->Value.Value.Array.Value.ExtensionObjectArray;
		}
	}
	return pExtensionObject;
}
// renvoi le pointeur sur l'ExtensionObjext uiIndex a l'interieur du m_Value
// si il contient un autre type retourne une erreur
// si il ne s'agit pas d'un tableau retourne une erreur
OpcUa_ExtensionObject* CDataValue::GetInternalExtensionObjectArray(OpcUa_UInt16 uiIndex)
{
	OpcUa_ExtensionObject* pExtensionObject = OpcUa_Null;
	if (m_pInternalDataValue)
	{
		if ((m_pInternalDataValue->Value.ArrayType == OpcUa_VariantArrayType_Array) && (m_pInternalDataValue->Value.Value.Array.Length >= uiIndex))
		{
			pExtensionObject = &(m_pInternalDataValue->Value.Value.Array.Value.ExtensionObjectArray[uiIndex]);
		}
	}
	return pExtensionObject;
}
#define Alloc_And_Copy_Array(name)\
{\
	if (m_pInternalDataValue->Value.Value.Array.Value.name##Array)\
	{\
		OpcUa_Free(m_pInternalDataValue->Value.Value.Array.Value.name##Array);\
		m_pInternalDataValue->Value.Value.Array.Value.name##Array=OpcUa_Null;\
	}\
	m_pInternalDataValue->Value.Value.Array.Value.name##Array = (OpcUa_##name*)OpcUa_Alloc(sizeof(OpcUa_##name)*Value.Value.Array.Length);\
	for (OpcUa_Int32 i = 0; i < Value.Value.Array.Length; i++)\
	{\
		OpcUa_##name##_Initialize(&m_pInternalDataValue->Value.Value.Array.Value.name##Array[i]);\
		OpcUa_##name##_CopyTo(&Value.Value.Array.Value.name##Array[i], &m_pInternalDataValue->Value.Value.Array.Value.name##Array[i]);\
	}\
	m_pInternalDataValue->Value.Value.Array.Length=	Value.Value.Array.Length;\
}
// Cette méthode affecte un OpcUa_Variant (Value) dans le OpcUa_Variant (m_Value) encapsulé dans la classe.
// aucune vérification de compatibilité n'est réalisé
OpcUa_StatusCode CDataValue::SetValue(OpcUa_Variant Value) 
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	if (m_pInternalDataValue)
	{
		if (GetBuiltInDataType() != Value.Datatype)
		{
			if ((GetBuiltInDataType() != OpcUaType_Byte) && (GetBuiltInDataType() != OpcUaType_ByteString))
				SetBuiltInDataType(Value.Datatype);
		}
		switch (Value.Datatype)
		{
		case OpcUaType_Boolean:
		{
			if (Value.ArrayType == OpcUa_VariantArrayType_Array)
			{
				TransfertArray(Boolean);
				//Alloc_And_Copy_Array(Boolean);
			}
			else
				m_pInternalDataValue->Value.Value.Boolean = Value.Value.Boolean;
		}
		break;
		case OpcUaType_Byte:
		{
			if (Value.ArrayType == OpcUa_VariantArrayType_Array)
			{
				TransfertArray(Byte);
				//Alloc_And_Copy_Array(Byte);
			}
			else
				m_pInternalDataValue->Value.Value.Byte = Value.Value.Byte;
		}
		break;
		case OpcUaType_SByte:
		{
			if (Value.ArrayType == OpcUa_VariantArrayType_Array)
			{
				TransfertArray(SByte);
				//Alloc_And_Copy_Array(SByte);
			}
			else
				m_pInternalDataValue->Value.Value.SByte = Value.Value.SByte;
		}
		break;
		case OpcUaType_Int16:
		{
			if (Value.ArrayType == OpcUa_VariantArrayType_Array)
			{
				TransfertArray(Int16);
				//Alloc_And_Copy_Array(Int16);
			}
			else
				m_pInternalDataValue->Value.Value.Int16 = Value.Value.Int16;
		}
		break;
		case OpcUaType_UInt16:
		{
			if (Value.ArrayType == OpcUa_VariantArrayType_Array)
			{
				TransfertArray(UInt16);
				//Alloc_And_Copy_Array(UInt16);
			}
			else
				m_pInternalDataValue->Value.Value.UInt16 = Value.Value.UInt16;
		}
		break;
		case OpcUaType_Int32:
		{
			if (Value.ArrayType == OpcUa_VariantArrayType_Array)
			{
				TransfertArray(Int32);
				//Alloc_And_Copy_Array(Int32);
			}
			else
				m_pInternalDataValue->Value.Value.Int32 = Value.Value.Int32;
		}
		break;
		case OpcUaType_UInt32:
		{
			if (Value.ArrayType == OpcUa_VariantArrayType_Array)
			{
				TransfertArray(UInt32);
				//Alloc_And_Copy_Array(UInt32);
			}
			else
				m_pInternalDataValue->Value.Value.UInt32 = Value.Value.UInt32;
		}
		break;
		case OpcUaType_Int64:
		{
			if (Value.ArrayType == OpcUa_VariantArrayType_Array)
			{
				TransfertArray(Int64);
				//Alloc_And_Copy_Array(Int64);
			}
			else
				m_pInternalDataValue->Value.Value.Int64 = Value.Value.Int64;
		}					
		break;
		case OpcUaType_UInt64:
		{
			if (Value.ArrayType == OpcUa_VariantArrayType_Array)
			{
				TransfertArray(UInt64);
				//Alloc_And_Copy_Array(UInt64);
			}
			else
				m_pInternalDataValue->Value.Value.UInt64 = Value.Value.UInt64;
		}
		break;
		case OpcUaType_Float:
		{
			if (Value.ArrayType == OpcUa_VariantArrayType_Array)
			{
				TransfertArray(Float);
				//Alloc_And_Copy_Array(Float);
			}
			else
				m_pInternalDataValue->Value.Value.Float = Value.Value.Float;
		}
		break;
		case OpcUaType_Double:
		{
			if (Value.ArrayType == OpcUa_VariantArrayType_Array)
			{
				TransfertArray(Double);
				//Alloc_And_Copy_Array(Double);
			}
			else
				m_pInternalDataValue->Value.Value.Double = Value.Value.Double;
		}
		break;
		case OpcUaType_ByteString:
		{
			if (Value.ArrayType == OpcUa_VariantArrayType_Array)
			{
				TransfertCopyArray(ByteString);
				//Alloc_And_Copy_Array(ByteString);
			}
			else
			{
				if ((Value.Value.ByteString.Length > 0) && (Value.ArrayType == OpcUa_VariantArrayType_Scalar))
				{
					if (m_pInternalDataValue->Value.Datatype == OpcUaType_Byte)
					{
						// Exeption requise par la specification. Conversion ByteString en tableau Byte
						// On commence par vérifier que m_Value est bien un tableau de byte;
						if ((m_pInternalDataValue->Value.Datatype == OpcUaType_Byte) && (m_pInternalDataValue->Value.ArrayType == OpcUa_VariantArrayType_Array))
						{
							if (m_pInternalDataValue->Value.Value.Array.Value.ByteArray)
								OpcUa_Free(m_pInternalDataValue->Value.Value.Array.Value.ByteArray);
							m_pInternalDataValue->Value.Value.Array.Value.ByteArray = (OpcUa_Byte*)OpcUa_Alloc(Value.Value.ByteString.Length);
							for (OpcUa_Int32 ii = 0; ii < Value.Value.ByteString.Length; ii++)
							{
								m_pInternalDataValue->Value.Value.Array.Value.ByteArray[ii] = Value.Value.ByteString.Data[ii];
							}
							m_pInternalDataValue->Value.Value.Array.Length = Value.Value.ByteString.Length;
						}
						else
							uStatus = OpcUa_BadTypeMismatch;
					}
					else
					{
						//m_pInternalDataValue->Value.Value.ByteString.Data = Value.Value.ByteString.Data;
						//m_pInternalDataValue->Value.Value.ByteString.Length = Value.Value.ByteString.Length;
						if (m_pInternalDataValue->Value.Value.ByteString.Data)
							OpcUa_ByteString_Clear(&(m_pInternalDataValue->Value.Value.ByteString));
						// Modif mai 2015
						m_pInternalDataValue->Value.ArrayType = OpcUa_VariantArrayType_Scalar;
						m_pInternalDataValue->Value.Datatype = OpcUaType_ByteString;
						OpcUa_ByteString_CopyTo(&(Value.Value.ByteString), &(m_pInternalDataValue->Value.Value.ByteString)); 
						//OpcUa_ByteString_Clear(&Value.Value.ByteString);
						// Fin Modif mai 2015
					}
				}
				else
				{
					if (Value.Value.ByteString.Length == -1)
						uStatus = OpcUa_BadOutOfRange;
					if (Value.ArrayType != OpcUa_VariantArrayType_Scalar)
						OpcUa_Trace(OPCUA_TRACE_EXTRA_LEVEL_ERROR, "Not implemented OpcUaType_ByteString...Contact Michel Condemine\n");
				}
			}
		}
		break;
		case OpcUaType_DateTime:
		{
			if (Value.ArrayType == OpcUa_VariantArrayType_Array)
			{
				TransfertArray(DateTime);
				//Alloc_And_Copy_Array(DateTime);
			}
			else
				m_pInternalDataValue->Value.Value.DateTime = Value.Value.DateTime;
		}
		break;
		case OpcUaType_Guid:
		{
			if (Value.ArrayType == OpcUa_VariantArrayType_Array)
			{
				TransfertArray(Guid);
				//Alloc_And_Copy_Array(Guid);
			}
			else
			{
				if (m_pInternalDataValue->Value.Value.Guid)
					OpcUa_Guid_Clear(m_pInternalDataValue->Value.Value.Guid);
				OpcUa_Guid_Copy(Value.Value.Guid, &(m_pInternalDataValue->Value.Value.Guid));
				if (Value.Value.Guid)
				{
					OpcUa_Guid_Clear(Value.Value.Guid);
				}
				//m_pInternalDataValue->Value.Value.Guid->Data1 = Value.Value.Guid->Data1;
				//m_pInternalDataValue->Value.Value.Guid->Data2 = Value.Value.Guid->Data2;
				//m_pInternalDataValue->Value.Value.Guid->Data3 = Value.Value.Guid->Data3;
				//OpcUa_MemCpy(m_pInternalDataValue->Value.Value.Guid->Data4, 8, Value.Value.Guid->Data4, 8);
			}
		}
		break;
		case OpcUaType_String:
		{
			if (Value.ArrayType == OpcUa_VariantArrayType_Scalar)
			{
				if (OpcUa_String_StrLen(&m_pInternalDataValue->Value.Value.String)>0)
					OpcUa_String_Clear(&m_pInternalDataValue->Value.Value.String);
				OpcUa_String_Initialize(&(m_pInternalDataValue->Value.Value.String));
				OpcUa_String_CopyTo(&(Value.Value.String), &(m_pInternalDataValue->Value.Value.String));
				//OpcUa_String_Clear(&(Value.Value.String));
				//OpcUa_Int32 iLen = OpcUa_String_StrLen(&(Value.Value.String));
				//if (iLen > 0)
				//	OpcUa_String_StrnCpy(&(m_pInternalDataValue->Value.Value.String), &(Value.Value.String), iLen);
			}
			else
			{
				if (Value.ArrayType == OpcUa_VariantArrayType_Array)
				{
					TransfertCopyArray(String);
					//Alloc_And_Copy_Array(String);
				}
			}
		}
		break;
		case OpcUaType_QualifiedName:
		{
			if (Value.ArrayType == OpcUa_VariantArrayType_Scalar)
			{
				if (!m_pInternalDataValue->Value.Value.QualifiedName)
					m_pInternalDataValue->Value.Value.QualifiedName = (OpcUa_QualifiedName*)OpcUa_Alloc(sizeof(OpcUa_QualifiedName));
				else
					OpcUa_QualifiedName_Clear(m_pInternalDataValue->Value.Value.QualifiedName);
				OpcUa_QualifiedName_Clear(m_pInternalDataValue->Value.Value.QualifiedName);
				OpcUa_QualifiedName_CopyTo(Value.Value.QualifiedName, m_pInternalDataValue->Value.Value.QualifiedName);
			}
			else
			{
				if (Value.ArrayType == OpcUa_VariantArrayType_Array)
				{
					TransfertCopyArray(QualifiedName);
				}
			}
		}
		break;
		case OpcUaType_LocalizedText:
		{
			if (Value.ArrayType == OpcUa_VariantArrayType_Scalar)
			{
				if (!m_pInternalDataValue->Value.Value.LocalizedText)
				{
					m_pInternalDataValue->Value.Value.LocalizedText = (OpcUa_LocalizedText*)OpcUa_Alloc(sizeof(OpcUa_LocalizedText));
					if (m_pInternalDataValue->Value.Value.LocalizedText)
						OpcUa_LocalizedText_Initialize(m_pInternalDataValue->Value.Value.LocalizedText);
				}
				else
					OpcUa_LocalizedText_Clear(m_pInternalDataValue->Value.Value.LocalizedText);
				if (Value.Value.LocalizedText)
				{
					OpcUa_LocalizedText_CopyTo(Value.Value.LocalizedText, m_pInternalDataValue->Value.Value.LocalizedText);
					//Locale
					if (OpcUa_String_StrLen(&(Value.Value.LocalizedText->Locale)) > 0)
						OpcUa_String_CopyTo(&(Value.Value.LocalizedText->Locale), &(m_pInternalDataValue->Value.Value.LocalizedText->Locale));
					else
					{
						if (m_pInternalDataValue->Value.Value.LocalizedText)
							OpcUa_String_AttachCopy(&(m_pInternalDataValue->Value.Value.LocalizedText->Locale), "");//en-us
					}
					//Text
					if (OpcUa_String_StrLen(&(Value.Value.LocalizedText->Text)) > 0)
						OpcUa_String_CopyTo(&(Value.Value.LocalizedText->Text), &(m_pInternalDataValue->Value.Value.LocalizedText->Text));
					else
					{
						if (m_pInternalDataValue->Value.Value.LocalizedText)
							OpcUa_String_AttachCopy(&(m_pInternalDataValue->Value.Value.LocalizedText->Text), "");
					}
				}
			}
			else
			{
				if (Value.ArrayType == OpcUa_VariantArrayType_Array)
				{
					TransfertCopyArray(LocalizedText);
					//Alloc_And_Copy_Array(LocalizedText);
					//if (m_pInternalDataValue->Value.Value.Array.Value.LocalizedTextArray)
					//{
					//	for (OpcUa_Int32 i = 0; i < m_pInternalDataValue->Value.Value.Array.Length; i++)
					//	{
					//		OpcUa_LocalizedText_Clear(&m_pInternalDataValue->Value.Value.Array.Value.LocalizedTextArray[i]);
					//	}
					//	OpcUa_Free(m_pInternalDataValue->Value.Value.Array.Value.LocalizedTextArray);
					//	m_pInternalDataValue->Value.Value.Array.Value.LocalizedTextArray = OpcUa_Null;
					//}
					//m_pInternalDataValue->Value.Value.Array.Value.LocalizedTextArray = (OpcUa_LocalizedText*)OpcUa_Alloc(sizeof(OpcUa_LocalizedText)*Value.Value.Array.Length);
					//for (OpcUa_Int32 i = 0; i < Value.Value.Array.Length; i++)
					//{
					//OpcUa_LocalizedText_Initialize(&m_pInternalDataValue->Value.Value.Array.Value.LocalizedTextArray[i]);
					//OpcUa_LocalizedText_CopyTo(&Value.Value.Array.Value.LocalizedTextArray[i], &m_pInternalDataValue->Value.Value.Array.Value.LocalizedTextArray[i]);
					//}
					//m_pInternalDataValue->Value.Value.Array.Length=	Value.Value.Array.Length;
				}
			}
		}
		break;
		case OpcUaType_ExtensionObject:
		{
			if (Value.ArrayType == OpcUa_VariantArrayType_Scalar)
			{
				if (Value.Value.ExtensionObject)
				{
					OpcUa_EncodeableType* pEncodeableType = Value.Value.ExtensionObject->Body.EncodeableObject.Type;
					if (pEncodeableType)
					{
						//OpcUa_ExtensionObject_Scalar_Copy();
						if (m_pInternalDataValue->Value.Value.ExtensionObject)
						{
							OpcUa_ExtensionObject_Clear(m_pInternalDataValue->Value.Value.ExtensionObject);
							OpcUa_Free(m_pInternalDataValue->Value.Value.ExtensionObject);
						}
						m_pInternalDataValue->Value.Value.ExtensionObject = (OpcUa_ExtensionObject*)OpcUa_Alloc(sizeof(OpcUa_ExtensionObject));
						OpcUa_ExtensionObject_Initialize(m_pInternalDataValue->Value.Value.ExtensionObject);
						OpcUa_ExtensionObject_CopyTo(Value.Value.ExtensionObject, m_pInternalDataValue->Value.Value.ExtensionObject);
						/*
						OpcUa_ExtensionObject_Initialize(m_pInternalDataValue->Value.Value.ExtensionObject);
						m_pInternalDataValue->Value.Datatype = OpcUaType_ExtensionObject;
						m_pInternalDataValue->Value.Value.ExtensionObject->BodySize = 0;
						m_pInternalDataValue->Value.Value.ExtensionObject->Encoding = OpcUa_ExtensionObjectEncoding_EncodeableObject;
						OpcUa_ExpandedNodeId_Initialize(&(m_pInternalDataValue->Value.Value.ExtensionObject->TypeId));
						m_pInternalDataValue->Value.Value.ExtensionObject->TypeId.ServerIndex = 0;
						OpcUa_String_Initialize(&(m_pInternalDataValue->Value.Value.ExtensionObject->TypeId.NamespaceUri));
						m_pInternalDataValue->Value.Value.ExtensionObject->TypeId.NodeId.Identifier.Numeric = pEncodeableType->BinaryEncodingTypeId;
						m_pInternalDataValue->Value.Value.ExtensionObject->TypeId.NodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
						m_pInternalDataValue->Value.Value.ExtensionObject->TypeId.NodeId.NamespaceIndex = 0;
						m_pInternalDataValue->Value.Value.ExtensionObject->Body.EncodeableObject.Type = pEncodeableType; //  Utils::Copy(pEncodeableType); // modif mai 2015
						if (m_pInternalDataValue->Value.Value.ExtensionObject->Body.EncodeableObject.Object)
							OpcUa_Free(m_pInternalDataValue->Value.Value.ExtensionObject->Body.EncodeableObject.Object);
						//m_pInternalDataValue->Value.Value.ExtensionObject->Body.EncodeableObject.Object = OpcUa_Alloc(pEncodeableType->AllocationSize);// modif mai 2015
						m_pInternalDataValue->Value.Value.ExtensionObject->Body.EncodeableObject.Object = Value.Value.ExtensionObject->Body.EncodeableObject.Object;
						OpcUa_MemCpy(m_pInternalDataValue->Value.Value.ExtensionObject->Body.EncodeableObject.Object,
							pEncodeableType->AllocationSize,
							Value.Value.ExtensionObject->Body.EncodeableObject.Object,
							pEncodeableType->AllocationSize); // Modif mai 2015
						*/
					}
				}
				else
					OpcUa_Trace(OPCUA_TRACE_EXTRA_LEVEL_ERROR, "CDataValue::SetValue> ExtensionObject is NULL\n");
			}
			else
			{
				if (Value.ArrayType == OpcUa_VariantArrayType_Array)
				{
					if (Value.Value.Array.Length > 0)
					{
						m_pInternalDataValue->Value.ArrayType = OpcUa_VariantArrayType_Array;
						TransfertCopyArray(ExtensionObject);
						/*
						int iSize = Value.Value.Array.Length;
						if (m_pInternalDataValue->Value.Value.Array.Value.ExtensionObjectArray)
							OpcUa_Free(m_pInternalDataValue->Value.Value.Array.Value.ExtensionObjectArray);
						m_pInternalDataValue->Value.Value.Array.Value.ExtensionObjectArray = (OpcUa_ExtensionObject*)OpcUa_Alloc(iSize*sizeof(OpcUa_ExtensionObject));
						if (m_pInternalDataValue->Value.Value.Array.Value.ExtensionObjectArray)
						{
							//
							m_pInternalDataValue->Value.Datatype = OpcUaType_ExtensionObject;
							m_pInternalDataValue->Value.ArrayType = OpcUa_VariantArrayType_Array;
							for (int ii = 0; ii < iSize; ii++)
							{
								if (Value.Value.Array.Value.ExtensionObjectArray)
								{
									OpcUa_EncodeableType* pEncodeableType = Value.Value.Array.Value.ExtensionObjectArray[ii].Body.EncodeableObject.Type;
									if (pEncodeableType)
									{
										OpcUa_ExtensionObject_Initialize(&(m_pInternalDataValue->Value.Value.Array.Value.ExtensionObjectArray[ii]));
										OpcUa_ExtensionObject_CopyTo(&(Value.Value.Array.Value.ExtensionObjectArray[ii]), &(m_pInternalDataValue->Value.Value.Array.Value.ExtensionObjectArray[ii]));
									}
									else
										OpcUa_Trace(OPCUA_TRACE_EXTRA_LEVEL_ERROR, "CDataValue::SetValue> OpcUa_EncodeableType is NULL\n");
								}
							}
							m_pInternalDataValue->Value.Value.Array.Length = Value.Value.Array.Length;
						}
						else
							uStatus = OpcUa_BadOutOfMemory;
						*/
					}
					else
					{
						m_pInternalDataValue->Value.Value.Array.Length = 0;
						m_pInternalDataValue->Value.ArrayType = OpcUa_VariantArrayType_Array;
						m_pInternalDataValue->Value.Datatype = OpcUaType_ExtensionObject;
						OpcUa_ExtensionObject_Initialize(m_pInternalDataValue->Value.Value.Array.Value.ExtensionObjectArray);
					}
				}
			}
		}
		break;
		default:
		{
			//OpcUa_Variant_CopyTo(&Value, &m_pInternalDataValue->Value); // 1.0.4.0
			m_pInternalDataValue->Value = Value;
		}
		break;
		}
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}

// Cette méthode affecte une valeur dans un variant.
// Si on passe un tableau (vecteur ou matrice). 
// Le variant passé en paramètre doit avoir la même taille que le destinataire
OpcUa_StatusCode CDataValue::UpdateValue(OpcUa_Variant Value) 
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	// on commence par vérifier la compatibilité du type à écrire

	if (m_pInternalDataValue)
	{
		if (Value.ArrayType != m_pInternalDataValue->Value.ArrayType)
		{
			// la specification accepte des exceptions
			// par exemple ByteStrirng to Byte
			if ((Value.ArrayType == OpcUa_VariantArrayType_Scalar) && (Value.Datatype == OpcUaType_ByteString) && (m_pInternalDataValue->Value.Datatype == OpcUaType_Byte))
				uStatus = SetValue(Value);
			else
				uStatus = OpcUa_BadTypeMismatch;
		}
		else
		{
			if (Value.ArrayType == OpcUa_VariantArrayType_Array)
			{
				if (Value.Value.Array.Length != m_pInternalDataValue->Value.Value.Array.Length)
					uStatus = OpcUa_BadTypeMismatch;
				else
					uStatus = SetValue(Value);
			}
			else
			{
				uStatus = SetValue(Value);
			}
		}
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}
// cette méthode met a jour m_pInternalDataValue a partir des variables de classe de CDataValue
OpcUa_StatusCode CDataValue::UpdateInternalDataValue()
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	if (m_pInternalDataValue)
	{
		m_pInternalDataValue->ServerPicoseconds = m_ServerPicoseconds;
		m_pInternalDataValue->ServerTimestamp = m_ServerTimestamp;
		m_pInternalDataValue->SourcePicoseconds = m_SourcePicoseconds;
		m_pInternalDataValue->SourceTimestamp = m_SourceTimestamp;
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}
OpcUa_DataValue* CDataValue::GetInternalDataValue()
{
	UpdateInternalDataValue();
	return m_pInternalDataValue;
}

void CDataValue::SetStatusCode(OpcUa_StatusCode uStatus)
{
	if (m_pInternalDataValue)
		m_pInternalDataValue->StatusCode = uStatus;
}
OpcUa_StatusCode CDataValue::GetStatusCode() 
{ 
	if (m_pInternalDataValue)
		return m_pInternalDataValue->StatusCode;
	else
		return OpcUa_BadWaitingForInitialData;
}
OpcUa_DateTime CDataValue::GetSourceTimeStamp() 
{ 
	return m_SourceTimestamp; 
}
void CDataValue::SetSourceTimestamp(OpcUa_DateTime dtValue)
{
	m_SourceTimestamp.dwHighDateTime = dtValue.dwHighDateTime;
	m_SourceTimestamp.dwLowDateTime = dtValue.dwLowDateTime;
}
OpcUa_UInt16 CDataValue::GetSourcePicoseconds() 
{ 
	return m_SourcePicoseconds;
}
void CDataValue::SetSourcePicoseconds(OpcUa_UInt16 iVal)
{
	m_SourcePicoseconds = iVal;
}
OpcUa_DateTime CDataValue::GetServerTimestamp() 
{ 
	return m_ServerTimestamp; 
}
void CDataValue::SetServerTimestamp(OpcUa_DateTime dtValue)
{
	m_ServerTimestamp.dwHighDateTime = dtValue.dwHighDateTime;
	m_ServerTimestamp.dwLowDateTime = dtValue.dwLowDateTime;
}
OpcUa_UInt16 CDataValue::GetServerPicoseconds() 
{ 
	return m_ServerPicoseconds; 
}
void CDataValue::SetServerPicoseconds(OpcUa_UInt16 iVal)
{
	m_ServerPicoseconds = iVal;
}
OpcUa_Byte CDataValue::GetBuiltInDataType() 
{ 
	OpcUa_Byte bType = OpcUaType_Null;
	if (m_pInternalDataValue)
		bType=m_pInternalDataValue->Value.Datatype; 
	return bType;
}
void CDataValue::SetBuiltInDataType(OpcUa_Byte datatype)
{
	if (m_pInternalDataValue)
		m_pInternalDataValue->Value.Datatype = datatype;
}
OpcUa_Byte CDataValue::GetArrayType() 
{ 
	OpcUa_Byte bType = OpcUaType_Null;
	if (m_pInternalDataValue)
		bType=m_pInternalDataValue->Value.ArrayType; 
	return bType;
}
void CDataValue::SetArrayType(OpcUa_Byte ArrayType)
{	
	if (m_pInternalDataValue)
		m_pInternalDataValue->Value.ArrayType = ArrayType;
}
OpcUa_Int32 CDataValue::GetArraySize()
{
	return m_pInternalDataValue->Value.Value.Array.Length;
}
void CDataValue::SetArraySize(OpcUa_Int32 iArraySize)
{	
	if (m_pInternalDataValue)
		m_pInternalDataValue->Value.Value.Array.Length = iArraySize;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	UTF 8 to latin 1. </summary>
///
/// <remarks>	Michel, 06/07/2016. </remarks>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode CDataValue::Utf8ToLatin1()
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	int i = 0;
	int j = 0;
	if (m_pInternalDataValue->Value.Datatype == OpcUaType_String)
	{
		OpcUa_CharA* src = OpcUa_String_GetRawString(&(m_pInternalDataValue->Value.Value.String));
		OpcUa_UInt32 uiLen = OpcUa_String_StrLen(&(m_pInternalDataValue->Value.Value.String));
		OpcUa_CharA* dest = (OpcUa_CharA*)OpcUa_Alloc(uiLen);
		OpcUa_UCharA c = src[i++];
		while (c)
		{
			// one-byte code
			if ((c & 0x80) == 0x00)
			{
				dest[j++] = c;
				c = src[i++];
			}
			// two-byte code
			else
			{
				if ((c & 0xE0) == 0xC0)
				{
					OpcUa_UCharA c2 = src[i++];
					if (c2 == 0) // incomplete character
						break;
					OpcUa_UInt16 res = ((c & 0x1F) << 6) | (c2 & 0x3F);
					if (res < 0x00A0 || res > 0x00FF) // not a Latin-1 character
						dest[j++] = '*';
					else
						dest[j++] = (res & 0xFF);
					c = src[i++];
				}
				// 3-byte or 4-byte code (not a Latin-1 character)
				else
				{
					do
					{
						c = src[i++];
					} while ((c != 0) && ((c & 0xC0) == 0x80));
					dest[j++] = '*';
				}
			}
		}
		dest[j] = 0;
		OpcUa_String_Clear(&(m_pInternalDataValue->Value.Value.String));
		OpcUa_String_AttachCopy(&(m_pInternalDataValue->Value.Value.String), dest);
		OpcUa_Free(dest);
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Gets as double. </summary>
///
/// <remarks>	Michel, 07/09/2016. </remarks>
///
/// <returns>	as double. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode OpenOpcUa::UASharedLib::CDataValue::GetAsDouble(OpcUa_Double* pDblResult)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	if (pDblResult)
	{
		if (m_pInternalDataValue->Value.ArrayType == OpcUa_VariantArrayType_Scalar)
		{
			switch (m_pInternalDataValue->Value.Datatype)
			{
			case OpcUaType_Double:
				(*pDblResult) = m_pInternalDataValue->Value.Value.Double;
				break;
			case OpcUaType_Float:
				(*pDblResult) = m_pInternalDataValue->Value.Value.Float;
				break;
			case OpcUaType_Int16:
				(*pDblResult) = m_pInternalDataValue->Value.Value.Int16;
				break;
			case OpcUaType_UInt16:
				(*pDblResult) = m_pInternalDataValue->Value.Value.UInt16;
				break;
			case OpcUaType_Int32:
				(*pDblResult) = m_pInternalDataValue->Value.Value.UInt32;
				break;
			case OpcUaType_UInt32:
				(*pDblResult) = m_pInternalDataValue->Value.Value.Int32;
				break;
			default:
				uStatus = OpcUa_BadInvalidArgument;
				break;
			}
		}
		else
			uStatus = OpcUa_BadInvalidArgument;
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}

