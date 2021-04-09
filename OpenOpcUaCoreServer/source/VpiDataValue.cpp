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
#include "VpiDataValue.h"
using namespace OpenOpcUa;
using namespace UASubSystem;
CVpiDataValue:: CVpiDataValue(void)
{
	m_bLatin1 = OpcUa_False;
	m_pValue = OpcUa_Null;
	SetStatusCode(0x40920000);
}

CVpiDataValue::CVpiDataValue(OpcUa_Byte Datatype, OpcUa_UInt32 iNbElt, OpcUa_Boolean bLatin1) :m_bLatin1(bLatin1)
{
	m_pValue = (OpcUa_Variant*)OpcUa_Alloc(sizeof(OpcUa_Variant));
	OpcUa_Variant_Initialize(m_pValue);
	m_pValue->Datatype = Datatype;
	if (iNbElt)
		m_pValue->ArrayType = OpcUa_VariantArrayType_Array;
	else
		m_pValue->ArrayType = OpcUa_VariantArrayType_Scalar;
	SetStatusCode(0x40920000);
}
CVpiDataValue::~CVpiDataValue(void)
{
	OpcUa_Variant_Clear(m_pValue);
	if (m_pValue)
		OpcUa_Free(m_pValue);
}

OpcUa_Variant* CVpiDataValue::GetValue() { return m_pValue; }
OpcUa_StatusCode CVpiDataValue::SetValue(OpcUa_Variant* aValue)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	m_pValue->Datatype = aValue->Datatype;
	m_pValue->ArrayType = aValue->ArrayType;
	if (m_pValue->ArrayType == OpcUa_VariantArrayType_Scalar)
		SetScalarValue(aValue);
	else
	{
		if (m_pValue->ArrayType == OpcUa_VariantArrayType_Array)
		{
			m_pValue->Value.Array.Length = aValue->Value.Array.Length;
			SetArrayValue(aValue);
		}
		else
			if (m_pValue->ArrayType == OpcUa_VariantArrayType_Matrix)
				uStatus = OpcUa_BadNotSupported; // Not supported
	}
	return uStatus;
}
OpcUa_StatusCode CVpiDataValue::SetArrayValue(OpcUa_Variant* aValue)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	if (m_pValue->Value.Array.Length>0)
	{
		OpcUa_Int32 iLen = m_pValue->Value.Array.Length;
		for (OpcUa_Int32 ii = 0; ii<iLen; ii++)
		{
			switch (m_pValue->Datatype)
			{
			case OpcUaType_Boolean:
				if (m_pValue->Value.Array.Value.BooleanArray == OpcUa_Null)
					m_pValue->Value.Array.Value.BooleanArray = (OpcUa_Boolean*)OpcUa_Alloc(sizeof(OpcUa_Boolean)*iLen);
				m_pValue->Value.Array.Value.BooleanArray[ii] = aValue->Value.Array.Value.BooleanArray[ii];
				break;
			case OpcUaType_Byte:
				if (m_pValue->Value.Array.Value.ByteArray == OpcUa_Null)
					m_pValue->Value.Array.Value.ByteArray = (OpcUa_Byte*)OpcUa_Alloc(sizeof(OpcUa_Byte)*iLen);
				m_pValue->Value.Array.Value.ByteArray[ii] = aValue->Value.Array.Value.ByteArray[ii];
				break;
			case OpcUaType_DateTime:
				if (m_pValue->Value.Array.Value.DateTimeArray == OpcUa_Null)
					m_pValue->Value.Array.Value.DateTimeArray = (OpcUa_DateTime*)OpcUa_Alloc(sizeof(OpcUa_DateTime)*iLen);
				m_pValue->Value.Array.Value.DateTimeArray[ii] = aValue->Value.Array.Value.DateTimeArray[ii];
				break;
			case OpcUaType_Double:
				if (m_pValue->Value.Array.Value.DoubleArray == OpcUa_Null)
					m_pValue->Value.Array.Value.DoubleArray = (OpcUa_Double*)OpcUa_Alloc(sizeof(OpcUa_Double)*iLen);
				m_pValue->Value.Array.Value.DoubleArray[ii] = aValue->Value.Array.Value.DoubleArray[ii];
				break;
			case OpcUaType_Float:
				if (m_pValue->Value.Array.Value.FloatArray == OpcUa_Null)
					m_pValue->Value.Array.Value.FloatArray = (OpcUa_Float*)OpcUa_Alloc(sizeof(OpcUa_Float)*iLen);
				m_pValue->Value.Array.Value.FloatArray[ii] = aValue->Value.Array.Value.FloatArray[ii];
				break;
			case OpcUaType_Int16:
				if (m_pValue->Value.Array.Value.Int16Array == OpcUa_Null)
					m_pValue->Value.Array.Value.Int16Array = (OpcUa_Int16*)OpcUa_Alloc(sizeof(OpcUa_Int16)*iLen);
				m_pValue->Value.Array.Value.Int16Array[ii] = aValue->Value.Array.Value.Int16Array[ii];
				break;
			case OpcUaType_Int32:
				if (m_pValue->Value.Array.Value.Int32Array == OpcUa_Null)
					m_pValue->Value.Array.Value.Int32Array = (OpcUa_Int32*)OpcUa_Alloc(sizeof(OpcUa_Int32)*iLen);
				m_pValue->Value.Array.Value.Int32Array[ii] = aValue->Value.Array.Value.Int32Array[ii];
				break;
			case OpcUaType_Int64:
				if (m_pValue->Value.Array.Value.Int64Array == OpcUa_Null)
					m_pValue->Value.Array.Value.Int64Array = (OpcUa_Int64*)OpcUa_Alloc(sizeof(OpcUa_Int64)*iLen);
				m_pValue->Value.Array.Value.Int64Array[ii] = aValue->Value.Array.Value.Int64Array[ii];
				break;
			case OpcUaType_UInt16:
				if (m_pValue->Value.Array.Value.UInt16Array == OpcUa_Null)
					m_pValue->Value.Array.Value.UInt16Array = (OpcUa_UInt16*)OpcUa_Alloc(sizeof(OpcUa_UInt16)*iLen);
				m_pValue->Value.Array.Value.UInt16Array[ii] = aValue->Value.Array.Value.UInt16Array[ii];
				break;
			case OpcUaType_UInt32:
				if (m_pValue->Value.Array.Value.UInt32Array == OpcUa_Null)
					m_pValue->Value.Array.Value.UInt32Array = (OpcUa_UInt32*)OpcUa_Alloc(sizeof(OpcUa_UInt32)*iLen);
				m_pValue->Value.Array.Value.UInt32Array[ii] = aValue->Value.Array.Value.UInt32Array[ii];
				break;
			case OpcUaType_UInt64:
				if (m_pValue->Value.Array.Value.UInt64Array == OpcUa_Null)
					m_pValue->Value.Array.Value.UInt64Array = (OpcUa_UInt64*)OpcUa_Alloc(sizeof(OpcUa_UInt64)*iLen);
				m_pValue->Value.Array.Value.UInt64Array[ii] = aValue->Value.Array.Value.UInt64Array[ii];
				break;
			case OpcUaType_String:
				if (m_pValue->Value.Array.Value.StringArray == OpcUa_Null)
					m_pValue->Value.Array.Value.StringArray = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String)*iLen);
				OpcUa_String_StrnCpy(&(m_pValue->Value.Array.Value.StringArray[ii]),
					&(aValue->Value.Array.Value.StringArray[ii]),
					OpcUa_String_StrLen(&(aValue->Value.Array.Value.StringArray[ii])));
				break;
			default:
				uStatus = OpcUa_BadInvalidArgument;
				break;
			}
		}
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}
OpcUa_StatusCode CVpiDataValue::SetScalarValue(OpcUa_Variant* aValue)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	switch (m_pValue->Datatype)
	{
	case OpcUaType_Boolean:
		m_pValue->Value.Boolean = aValue->Value.Boolean;
		break;
	case OpcUaType_Byte:
		m_pValue->Value.Byte = aValue->Value.Byte;
		break;
	case OpcUaType_DateTime:
		m_pValue->Value.DateTime = aValue->Value.DateTime;
		break;
	case OpcUaType_Double:
		m_pValue->Value.Double = aValue->Value.Double;
		break;
	case OpcUaType_Float:
		m_pValue->Value.Float = aValue->Value.Float;
		break;
	case OpcUaType_Int16:
		m_pValue->Value.Int16 = aValue->Value.Int16;
		break;
	case OpcUaType_Int32:
		m_pValue->Value.Int32 = aValue->Value.Int32;
		break;
	case OpcUaType_Int64:
		m_pValue->Value.Int32 = aValue->Value.Int32;
		break;
	case OpcUaType_UInt16:
		m_pValue->Value.UInt16 = aValue->Value.UInt16;
		break;
	case OpcUaType_UInt32:
		m_pValue->Value.UInt32 = aValue->Value.UInt32;
		break;
	case OpcUaType_UInt64:
		m_pValue->Value.UInt64 = aValue->Value.UInt64;
		break;
	case OpcUaType_String:
		{
			OpcUa_CharA* szSource = OpcUa_String_GetRawString(&(aValue->Value.String));
			OpcUa_String_AttachCopy(&(m_pValue->Value.String), szSource);
			Utf8ToLatin1();
		}
		break;
	default:
		uStatus = OpcUa_BadInvalidArgument;
		break;
	}
	return uStatus;
}
void CVpiDataValue::SetArrayType(OpcUa_Byte aArrayType) 
{ 
	m_pValue->ArrayType = aArrayType; 
}
void CVpiDataValue::SetSourceTimestamp(OpcUa_DateTime dateTime) 
{ 
	m_SourceTimestamp = dateTime; 
}
OpcUa_DateTime   CVpiDataValue::GetSourceTimestamp() 
{ 
	return m_SourceTimestamp; 
}
OpcUa_UInt16 CVpiDataValue::GetSourcePicosecond() 
{ 
	return m_SourcePicoseconds; 
}
void CVpiDataValue::SetSourcePicosecond(OpcUa_UInt16 uiVal) 
{ 
	m_SourcePicoseconds = uiVal; 
}
OpcUa_StatusCode CVpiDataValue::GetStatusCode() 
{ 
	return m_StatusCode; 
}
void CVpiDataValue::SetStatusCode(OpcUa_StatusCode status) 
{ 
	m_StatusCode = status; 
}
void CVpiDataValue::SetDataType(OpcUa_Byte	datatype) 
{ 
	m_pValue->Datatype = datatype; 
}
OpcUa_Byte	CVpiDataValue::GetDataType() 
{ 
	return m_pValue->Datatype; 
}
void CVpiDataValue::SetArraySize(OpcUa_Int32 iArraySize)
{
	m_pValue->ArrayType = OpcUa_VariantArrayType_Array;
	m_pValue->Value.Array.Length = iArraySize;
	switch (m_pValue->Datatype)
	{
	case OpcUaType_Boolean:
		if (m_pValue->Value.Array.Value.BooleanArray == OpcUa_Null)
			m_pValue->Value.Array.Value.BooleanArray = (OpcUa_Boolean*)OpcUa_Alloc(sizeof(OpcUa_Boolean)*iArraySize);
		break;
	case OpcUaType_Byte:
		if (m_pValue->Value.Array.Value.ByteArray == OpcUa_Null)
			m_pValue->Value.Array.Value.ByteArray = (OpcUa_Byte*)OpcUa_Alloc(sizeof(OpcUa_Byte)*iArraySize);
		break;
	case OpcUaType_DateTime:
		if (m_pValue->Value.Array.Value.DateTimeArray == OpcUa_Null)
			m_pValue->Value.Array.Value.DateTimeArray = (OpcUa_DateTime*)OpcUa_Alloc(sizeof(OpcUa_DateTime)*iArraySize);
		break;
	case OpcUaType_Double:
		if (m_pValue->Value.Array.Value.DoubleArray == OpcUa_Null)
			m_pValue->Value.Array.Value.DoubleArray = (OpcUa_Double*)OpcUa_Alloc(sizeof(OpcUa_Double)*iArraySize);
		break;
	case OpcUaType_Float:
		if (m_pValue->Value.Array.Value.FloatArray == OpcUa_Null)
			m_pValue->Value.Array.Value.FloatArray = (OpcUa_Float*)OpcUa_Alloc(sizeof(OpcUa_Float)*iArraySize);
		break;
	case OpcUaType_Int16:
		if (m_pValue->Value.Array.Value.Int16Array == OpcUa_Null)
			m_pValue->Value.Array.Value.Int16Array = (OpcUa_Int16*)OpcUa_Alloc(sizeof(OpcUa_Int16)*iArraySize);
		break;
	case OpcUaType_Int32:
		if (m_pValue->Value.Array.Value.Int32Array == OpcUa_Null)
			m_pValue->Value.Array.Value.Int32Array = (OpcUa_Int32*)OpcUa_Alloc(sizeof(OpcUa_Int32)*iArraySize);
		break;
	case OpcUaType_String:
		if (m_pValue->Value.Array.Value.StringArray == OpcUa_Null)
			m_pValue->Value.Array.Value.StringArray = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String)*iArraySize);
		break;
	case OpcUaType_UInt16:
		if (m_pValue->Value.Array.Value.UInt16Array == OpcUa_Null)
			m_pValue->Value.Array.Value.UInt16Array = (OpcUa_UInt16*)OpcUa_Alloc(sizeof(OpcUa_UInt16)*iArraySize);
		break;
	case OpcUaType_UInt32:
		if (m_pValue->Value.Array.Value.UInt32Array == OpcUa_Null)
			m_pValue->Value.Array.Value.UInt32Array = (OpcUa_UInt32*)OpcUa_Alloc(sizeof(OpcUa_UInt32)*iArraySize);
		break;
	default:
		break;
	}
}
OpcUa_Int32 CVpiDataValue::GetArraySize() 
{ 
	return m_pValue->Value.Array.Length; 
}
///-------------------------------------------------------------------------------------------------
/// <summary>	UTF 8 to latin 1. </summary>
///
/// <remarks>	Michel, 06/07/2016. </remarks>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode CVpiDataValue::Utf8ToLatin1()
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	int i = 0;
	int j = 0;
	if (!m_bLatin1)
	{
		if (m_pValue)
		{
			if (m_pValue->Datatype == OpcUaType_String)
			{
				OpcUa_CharA* src = OpcUa_String_GetRawString(&(m_pValue->Value.String));
				OpcUa_CharA* dest = (OpcUa_CharA*)OpcUa_Alloc(2048);
				ZeroMemory(dest, 2048);
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
				OpcUa_String_Clear(&(m_pValue->Value.String));
				OpcUa_String_AttachCopy(&(m_pValue->Value.String), dest);
				OpcUa_Free(dest);
				m_bLatin1 = OpcUa_True;
			}
			else
				uStatus = OpcUa_BadInvalidArgument;
		}
		else
			uStatus = OpcUa_BadOutOfMemory;
	}
	return uStatus;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Is latin 1. </summary>
///
/// <remarks>	Michel, 06/07/2016. </remarks>
///
/// <returns>	An OpcUa_Boolean. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_Boolean OpenOpcUa::UASubSystem::CVpiDataValue::IsLatin1(void)
{
	return m_bLatin1;
}

