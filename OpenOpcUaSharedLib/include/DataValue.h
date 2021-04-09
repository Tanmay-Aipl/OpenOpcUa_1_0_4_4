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

namespace OpenOpcUa
{
	namespace UASharedLib 
	{
		class SHAREDLIB_EXPORT CDataValue
		{
		public:
			CDataValue(void);
			CDataValue(OpcUa_DataValue* pDataValue);
			// instancie un CDataValue a partir du type stocké dans un NodeId
			CDataValue(OpcUa_Byte builtInType);
			~CDataValue(void);
			// initialise le CDataValue a partir du type passé en paramètre au travers du NodeId
			// Le NodeId contient le type par exemple ns=0;i=1 ==> initialisation d'un bool
			OpcUa_StatusCode Initialize(OpcUa_Byte builtInType,
				OpcUa_Int32 ValueRank,
				std::vector<OpcUa_UInt32>* ArrayDim = OpcUa_Null);
			OpcUa_StatusCode InitializeScalar(OpcUa_Byte builtInType);
			OpcUa_StatusCode InitializeArray(OpcUa_Byte builtInType, OpcUa_UInt32 uiVal);
			OpcUa_DataValue* GetInternalDataValue();
			OpcUa_Variant GetValue();
			OpcUa_StatusCode SetValue(OpcUa_Variant Value) ;
			OpcUa_StatusCode UpdateValue(OpcUa_Variant Value) ; // Il s'agit d'un SetValue qui réalise des verification de compatibilité sur variant à affecter
			OpcUa_StatusCode GetStatusCode();
			void SetStatusCode(OpcUa_StatusCode uStatus);
			OpcUa_DateTime GetSourceTimeStamp();
			void SetSourceTimestamp(OpcUa_DateTime dtValue);
			OpcUa_UInt16 GetSourcePicoseconds();
			void SetSourcePicoseconds(OpcUa_UInt16 iVal);
			OpcUa_DateTime GetServerTimestamp();
			void SetServerTimestamp(OpcUa_DateTime dtValue);
			OpcUa_UInt16 GetServerPicoseconds();
			void SetServerPicoseconds(OpcUa_UInt16 iVal);
			OpcUa_Byte GetBuiltInDataType();
			void SetBuiltInDataType(OpcUa_Byte datatype);
			OpcUa_Byte GetArrayType();
			void SetArrayType(OpcUa_Byte ArrayType);
			OpcUa_Int32 GetArraySize();
			void SetArraySize(OpcUa_Int32 iArraySize);
			OpcUa_ExtensionObject* GetInternalExtensionObject();
			OpcUa_ExtensionObject* GetInternalExtensionObjectArray(OpcUa_UInt16 uiIndex);
			OpcUa_StatusCode Utf8ToLatin1();
			OpcUa_StatusCode GetAsDouble(OpcUa_Double* pDblResult);
		private:
			OpcUa_StatusCode UpdateInternalDataValue();
		protected:
			//OpcUa_StatusCode m_StatusCode;
			OpcUa_DateTime   m_SourceTimestamp;
			OpcUa_DateTime   m_ServerTimestamp;
			OpcUa_UInt16     m_SourcePicoseconds;
			OpcUa_UInt16     m_ServerPicoseconds;
			OpcUa_DataValue* m_pInternalDataValue;
		};
	}
}