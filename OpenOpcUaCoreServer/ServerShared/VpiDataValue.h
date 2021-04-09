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
#define OpcUa_Vpi_UnInitialized 0x83040000
#define OpcUa_Vpi_UncertainInitialValue 0x40920000
namespace OpenOpcUa
{
	namespace UASubSystem 
	{	
		class CVpiDataValue
		{
		public:
			CVpiDataValue(void);

			CVpiDataValue(OpcUa_Byte Datatype, OpcUa_UInt32 iNbElt, OpcUa_Boolean bLatin1);
			~CVpiDataValue(void);
			OpcUa_Variant* GetValue();
			OpcUa_StatusCode SetValue(OpcUa_Variant* aValue);
			OpcUa_StatusCode SetArrayValue(OpcUa_Variant* aValue);
			OpcUa_StatusCode SetScalarValue(OpcUa_Variant* aValue);
			void SetArrayType(OpcUa_Byte aArrayType);
			void SetSourceTimestamp(OpcUa_DateTime dateTime);
			OpcUa_DateTime   GetSourceTimestamp();
			OpcUa_UInt16 GetSourcePicosecond();
			void SetSourcePicosecond(OpcUa_UInt16 uiVal);
			OpcUa_StatusCode GetStatusCode();
			void SetStatusCode(OpcUa_StatusCode status);
			void SetDataType(OpcUa_Byte	datatype);
			OpcUa_Byte	GetDataType();
			void SetArraySize(OpcUa_Int32 iArraySize);
			OpcUa_Int32 GetArraySize();
			OpcUa_StatusCode Utf8ToLatin1();
			OpcUa_Boolean	 IsLatin1();
		protected:
			OpcUa_Boolean	 m_bLatin1; // Alloc to convert to Latin1 only once
			OpcUa_Variant*   m_pValue;
			OpcUa_UInt16     m_SourcePicoseconds;
			OpcUa_DateTime   m_SourceTimestamp;
			OpcUa_StatusCode m_StatusCode;
		};
	}
}