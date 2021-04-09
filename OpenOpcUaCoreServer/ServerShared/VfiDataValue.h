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
#pragma once
#ifndef OPC_UASERVER
using namespace UABuiltinType;
#endif
#define OpcUa_Vfi_UnInitialized 0x83040000
namespace OpenOpcUa
{
	namespace UAHistoricalAccess 
	{	
		//class CVfiDataValue
		//{
		//public:
		//	CVfiDataValue(void)
		//	{
		//		m_pValue=OpcUa_Null;
		//		SetStatusCode(0x83040000);
		//	}

		//	CVfiDataValue(OpcUa_Byte Datatype, OpcUa_UInt32 iNbElt)
		//	{
		//		m_pValue=(OpcUa_Variant*)OpcUa_Alloc(sizeof(OpcUa_Variant));
		//		OpcUa_Variant_Initialize(m_pValue);
		//		m_pValue->Datatype=Datatype;
		//		if (iNbElt)
		//			m_pValue->ArrayType=OpcUa_VariantArrayType_Array;
		//		else
		//			m_pValue->ArrayType=OpcUa_VariantArrayType_Scalar;
		//		SetStatusCode(0x83040000);
		//	}
		//	~CVfiDataValue(void)
		//	{
		//		OpcUa_Variant_Clear(m_pValue);
		//		if (m_pValue)
		//			OpcUa_Free(m_pValue);
		//	}

		//	OpcUa_Variant* GetValue() {return m_pValue;}
		//	void SetValue(OpcUa_Variant* aValue) 
		//	{
		//		m_pValue->Datatype=aValue->Datatype;
		//		m_pValue->ArrayType=aValue->ArrayType;
		//		switch(m_pValue->Datatype)
		//		{
		//		case OpcUaType_Boolean:
		//			m_pValue->Value.Boolean=aValue->Value.Boolean;
		//			break;
		//		case OpcUaType_Byte:
		//			m_pValue->Value.Byte=aValue->Value.Byte;
		//			break;
		//		case OpcUaType_DateTime:
		//			m_pValue->Value.DateTime=aValue->Value.DateTime;
		//			break;
		//		case OpcUaType_Double:
		//			m_pValue->Value.Double=aValue->Value.Double;
		//			break;
		//		case OpcUaType_Float:
		//			m_pValue->Value.Float=aValue->Value.Float;
		//			break;
		//		case OpcUaType_Int16:
		//			m_pValue->Value.Int16=aValue->Value.Int16;
		//			break;
		//		case OpcUaType_Int32:
		//			m_pValue->Value.Int32=aValue->Value.Int32;
		//			break;
		//		case OpcUaType_Int64:
		//			m_pValue->Value.Int32=aValue->Value.Int32;
		//			break;
		//		case OpcUaType_UInt16:
		//			m_pValue->Value.UInt16=aValue->Value.UInt16;
		//			break;
		//		case OpcUaType_UInt32:
		//			m_pValue->Value.UInt32=aValue->Value.UInt32;
		//			break;
		//		case OpcUaType_UInt64:
		//			m_pValue->Value.UInt64=aValue->Value.UInt64;
		//			break;
		//		case OpcUaType_String:
		//			OpcUa_String_StrnCpy(&(m_pValue->Value.String),&(aValue->Value.String),OpcUa_String_StrLen(&(aValue->Value.String)));
		//			break;
		//		default:
		//			break;
		//		}
		//	}
		//	void SetArrayType(OpcUa_Byte aArrayType) { m_pValue->ArrayType=aArrayType;}
		//	// Source
		//	void SetSourceTimestamp(OpcUa_DateTime dateTime) {m_SourceTimestamp=dateTime;}
		//	void SetSourcePicosecond(OpcUa_UInt16 uiVal) {m_SourcePicoseconds=uiVal;}
		//	OpcUa_DateTime   GetSourceTimestamp() {return m_SourceTimestamp;}
		//	OpcUa_UInt16 GetSourcePicosecond() {return m_SourcePicoseconds;}
		//	// Server
		//	OpcUa_DateTime   GetServerTimestamp() {return m_ServerTimestamp;}
		//	OpcUa_UInt16 GetServerPicosecond() {return m_ServerPicoseconds;}
		//	void SetServerTimestamp(OpcUa_DateTime dateTime) {m_ServerTimestamp=dateTime;}
		//	void SetServerPicosecond(OpcUa_UInt16 uiVal) {m_ServerPicoseconds=uiVal;}
		//	OpcUa_StatusCode GetStatusCode() {return m_StatusCode;}
		//	void SetStatusCode(OpcUa_StatusCode status) {m_StatusCode=status;}
		//	void SetDataType(OpcUa_Byte	datatype) {m_pValue->Datatype=datatype; }
		//	OpcUa_Byte	GetDataType() {return m_pValue->Datatype;}
		//	void SetArraySize(OpcUa_Int32 iArraySize) 
		//	{
		//		m_pValue->ArrayType=OpcUa_VariantArrayType_Array;
		//		m_pValue->Value.Array.Length=iArraySize;
		//		switch (m_pValue->Datatype)
		//		{
		//		case OpcUaType_Boolean:
		//			if (m_pValue->Value.Array.Value.BooleanArray==OpcUa_Null)
		//				m_pValue->Value.Array.Value.BooleanArray=(OpcUa_Boolean*)OpcUa_Alloc(sizeof(OpcUa_Boolean)*iArraySize);
		//			break;
		//		case OpcUaType_Byte:
		//			if (m_pValue->Value.Array.Value.ByteArray==OpcUa_Null)
		//				m_pValue->Value.Array.Value.ByteArray=(OpcUa_Byte*)OpcUa_Alloc(sizeof(OpcUa_Byte)*iArraySize);
		//			break;
		//		case OpcUaType_DateTime:
		//			if (m_pValue->Value.Array.Value.DateTimeArray==OpcUa_Null)
		//				m_pValue->Value.Array.Value.DateTimeArray=(OpcUa_DateTime*)OpcUa_Alloc(sizeof(OpcUa_DateTime)*iArraySize);
		//			break;
		//		case OpcUaType_Double:
		//			if (m_pValue->Value.Array.Value.DoubleArray==OpcUa_Null)
		//				m_pValue->Value.Array.Value.DoubleArray=(OpcUa_Double*)OpcUa_Alloc(sizeof(OpcUa_Double)*iArraySize);
		//			break;
		//		case OpcUaType_Float:
		//			if (m_pValue->Value.Array.Value.FloatArray==OpcUa_Null)
		//				m_pValue->Value.Array.Value.FloatArray=(OpcUa_Float*)OpcUa_Alloc(sizeof(OpcUa_Float)*iArraySize);
		//			break;
		//		case OpcUaType_Int16:
		//			if (m_pValue->Value.Array.Value.Int16Array==OpcUa_Null)
		//				m_pValue->Value.Array.Value.Int16Array=(OpcUa_Int16*)OpcUa_Alloc(sizeof(OpcUa_Int16)*iArraySize);
		//			break;
		//		case OpcUaType_Int32:
		//			if (m_pValue->Value.Array.Value.Int32Array==OpcUa_Null)
		//				m_pValue->Value.Array.Value.Int32Array=(OpcUa_Int32*)OpcUa_Alloc(sizeof(OpcUa_Int32)*iArraySize);
		//			break;
		//		case OpcUaType_String:
		//			if (m_pValue->Value.Array.Value.StringArray==OpcUa_Null)
		//				m_pValue->Value.Array.Value.StringArray=(OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String)*iArraySize);
		//			break;
		//		case OpcUaType_UInt16:
		//			if (m_pValue->Value.Array.Value.UInt16Array==OpcUa_Null)
		//				m_pValue->Value.Array.Value.UInt16Array=(OpcUa_UInt16*)OpcUa_Alloc(sizeof(OpcUa_UInt16)*iArraySize);
		//			break;
		//		case OpcUaType_UInt32:
		//			if (m_pValue->Value.Array.Value.UInt32Array==OpcUa_Null)
		//				m_pValue->Value.Array.Value.UInt32Array=(OpcUa_UInt32*)OpcUa_Alloc(sizeof(OpcUa_UInt32)*iArraySize);
		//			break;
		//		default:
		//			break;
		//		}
		//	}
		//	OpcUa_Int32 GetArraySize() {return m_pValue->Value.Array.Length;}

		//protected:
		//	OpcUa_Variant*    m_pValue;
		//	OpcUa_UInt16     m_SourcePicoseconds;
		//	OpcUa_DateTime   m_SourceTimestamp;
		//	OpcUa_UInt16     m_ServerPicoseconds;
		//	OpcUa_DateTime   m_ServerTimestamp;
		//	OpcUa_StatusCode m_StatusCode;
		//};
	}
}