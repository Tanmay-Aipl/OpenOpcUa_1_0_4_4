/*****************************************************************************
	  Author
		©. Michel Condemine, 4CE Industry (2010-2014)
	  
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
using namespace VpiBuiltinType;
#endif
#define Vpi_UnInitialized 0x83040000
#define Vpi_UncertainInitialValue 0x40920000
namespace UASubSystem 
{	
	class CVpiDataValue
	{
	public:
		CVpiDataValue(void)
		{
			m_pValue=Vpi_Null;
			SetStatusCode(Vpi_UncertainInitialValue);
		}
		CVpiDataValue(Vpi_Byte Datatype, Vpi_UInt32 iNbElt)
		{
			m_pValue = (Vpi_Variant*)malloc(sizeof(Vpi_Variant));
			if (m_pValue)
			{
				ZeroMemory(m_pValue, sizeof(Vpi_Variant));
				m_pValue->Datatype = Datatype;
				if (iNbElt)
					m_pValue->ArrayType = Vpi_VariantArrayType_Array;
				else
					m_pValue->ArrayType = Vpi_VariantArrayType_Scalar;
				m_pValue->Value.Int32 = 0;
				m_pValue->Value.String.strContent = Vpi_Null;
			}
			SetStatusCode(Vpi_UncertainInitialValue);
		}
		~CVpiDataValue(void)
		{
			if (m_pValue)
			{
				free(m_pValue);
			}
		}

		Vpi_Variant* GetValue() { return m_pValue; }
		void SetValue(Vpi_Variant* aValue)
		{
			if (!m_pValue)
			{
				m_pValue = (Vpi_Variant*)malloc(sizeof(Vpi_Variant));
				if (m_pValue)
				{
					ZeroMemory(m_pValue, sizeof(Vpi_Variant));
					m_pValue->Datatype = aValue->Datatype;
					m_pValue->ArrayType = aValue->ArrayType;
					m_pValue->Datatype=aValue->Datatype;
					m_pValue->ArrayType=aValue->ArrayType;
					//m_pValue->Value=aValue->Value;
					switch(m_pValue->Datatype)
					{
					case VpiType_Boolean:
						m_pValue->Value.Boolean=aValue->Value.Boolean;
						break;
					case VpiType_Byte:
						m_pValue->Value.Byte=aValue->Value.Byte;
						break;
					case VpiType_DateTime:
						m_pValue->Value.DateTime=aValue->Value.DateTime;
						break;
					case VpiType_Double:
						m_pValue->Value.Double=aValue->Value.Double;
						break;
					case VpiType_Float:
						m_pValue->Value.Float=aValue->Value.Float;
						break;
					case VpiType_Int16:
						m_pValue->Value.Int16=aValue->Value.Int16;
						break;
					case VpiType_Int32:
						m_pValue->Value.Int32=aValue->Value.Int32;
						break;
					case VpiType_Int64:
						m_pValue->Value.Int32=aValue->Value.Int32;
						break;
					case VpiType_UInt16:
						m_pValue->Value.UInt16=aValue->Value.UInt16;
						break;
					case VpiType_UInt32:
						m_pValue->Value.UInt32=aValue->Value.UInt32;
						break;
					case VpiType_UInt64:
						m_pValue->Value.UInt64=aValue->Value.UInt64;
						break;
					case VpiType_String:
						// libération si déja alloué
						if (m_pValue->Value.String.strContent)
							free(m_pValue->Value.String.strContent);
						// Allocation et copie
						m_pValue->Value.String.strContent = (Vpi_CharA*)malloc(aValue->Value.String.uLength + 1);
						if (m_pValue->Value.String.strContent)
						{
							ZeroMemory(m_pValue->Value.String.strContent, aValue->Value.String.uLength + 1);
							memcpy(&(m_pValue->Value.String.strContent), &(aValue->Value.String.strContent), aValue->Value.String.uLength);
							m_pValue->Value.String.uLength = aValue->Value.String.uLength;
						}
						break;
					default:
						break;
					}
				}
			}
		}
		void SetArrayType(Vpi_Byte aArrayType)
		{ 
			if (m_pValue)
				m_pValue->ArrayType=aArrayType;
		}
		void SetSourceTimestamp(Vpi_DateTime dateTime) { m_SourceTimestamp = dateTime; }
		Vpi_DateTime   GetSourceTimestamp() { return m_SourceTimestamp; }
		Vpi_UInt16 GetSourcePicosecond() { return m_SourcePicoseconds; }
		void SetSourcePicosecond(Vpi_UInt16 uiVal) { m_SourcePicoseconds = uiVal; }
		Vpi_StatusCode GetStatusCode() { return m_StatusCode; }
		void SetStatusCode(Vpi_StatusCode status) { m_StatusCode = status; }

		void SetArraySize(Vpi_Int32 iArraySize)
		{
			if (m_pValue)
			{
				m_pValue->ArrayType=Vpi_VariantArrayType_Array;
				m_pValue->Value.Array.Length=iArraySize;
				switch (m_pValue->Datatype)
				{
				case VpiType_Boolean:
					if (m_pValue->Value.Array.Value.BooleanArray==Vpi_Null)
						m_pValue->Value.Array.Value.BooleanArray = (Vpi_Boolean*)malloc(sizeof(Vpi_Boolean)*iArraySize);
					break;
				case VpiType_Byte:
					if (m_pValue->Value.Array.Value.ByteArray==Vpi_Null)
						m_pValue->Value.Array.Value.ByteArray = (Vpi_Byte*)malloc(sizeof(Vpi_Byte)*iArraySize);
					break;
				case VpiType_DateTime:
					if (m_pValue->Value.Array.Value.DateTimeArray==Vpi_Null)
						m_pValue->Value.Array.Value.DateTimeArray = (Vpi_DateTime*)malloc(sizeof(Vpi_DateTime)*iArraySize);
					break;
				case VpiType_Double:
					if (m_pValue->Value.Array.Value.DoubleArray==Vpi_Null)
						m_pValue->Value.Array.Value.DoubleArray = (Vpi_Double*)malloc(sizeof(Vpi_Double)*iArraySize);
					break;
				case VpiType_Float:
					if (m_pValue->Value.Array.Value.FloatArray==Vpi_Null)
						m_pValue->Value.Array.Value.FloatArray = (Vpi_Float*)malloc(sizeof(Vpi_Float)*iArraySize);
					break;
				case VpiType_Int16:
					if (m_pValue->Value.Array.Value.Int16Array==Vpi_Null)
						m_pValue->Value.Array.Value.Int16Array = (Vpi_Int16*)malloc(sizeof(Vpi_Int16)*iArraySize);
					break;
				case VpiType_Int32:
					if (m_pValue->Value.Array.Value.Int32Array==Vpi_Null)
						m_pValue->Value.Array.Value.Int32Array = (Vpi_Int32*)malloc(sizeof(Vpi_Int32)*iArraySize);
					break;
				case VpiType_String:
					if (m_pValue->Value.Array.Value.StringArray==Vpi_Null)
						m_pValue->Value.Array.Value.StringArray = (Vpi_String*)malloc(sizeof(Vpi_String)*iArraySize);
					break;
				case VpiType_UInt16:
					if (m_pValue->Value.Array.Value.UInt16Array==Vpi_Null)
						m_pValue->Value.Array.Value.UInt16Array = (Vpi_UInt16*)malloc(sizeof(Vpi_UInt16)*iArraySize);
					break;
				case VpiType_UInt32:
					if (m_pValue->Value.Array.Value.UInt32Array==Vpi_Null)
						m_pValue->Value.Array.Value.UInt32Array = (Vpi_UInt32*)malloc(sizeof(Vpi_UInt32)*iArraySize);
					break;
				default:
					break;
				}
			}
		}
		Vpi_Int32 GetArraySize() { return m_pValue->Value.Array.Length; }
		CVpiDataValue operator=(CVpiDataValue* pVpiDataValue)
		{
			m_SourcePicoseconds=pVpiDataValue->GetSourcePicosecond();
			m_SourceTimestamp=pVpiDataValue->GetSourceTimestamp();
			m_StatusCode=pVpiDataValue->GetStatusCode();
			m_pValue=pVpiDataValue->GetValue();
			return *this;
		}
	protected:
		// dans un VPI seul les m_Value de type scalaire peuvent resider dans la cache du VPI
		// Les tableaux (vecteur) ne doivent pas
		Vpi_Variant*   m_pValue;
		Vpi_UInt16     m_SourcePicoseconds;
		Vpi_DateTime   m_SourceTimestamp;
		Vpi_StatusCode m_StatusCode;
	};
}

