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
using namespace OpenOpcUa;
using namespace UASharedLib;

namespace OpenOpcUa
{
	namespace UAAddressSpace 
	{
		class CUAVariable :	public CUABase
		{
		public:
			CUAVariable(void);
			CUAVariable(OpcUa_NodeClass aNodeClass,const char **atts);
			~CUAVariable(void);
			// Value
			CDataValue* GetValue();
			void SetValue(CDataValue* pValue);
			void SetValue(OpcUa_DataValue* pDataValue);
			OpcUa_StatusCode SetValue(OpcUa_Variant aVariantValue);
			// DataType
			OpcUa_NodeId GetDataType();
			void SetDataType(OpcUa_NodeId aValue);
			// ValueRank
			OpcUa_Int32 GetValueRank();
			void SetValueRank(OpcUa_Int32 aValue);
			std::vector<OpcUa_UInt32>* GetArrayDimensions();
			// NoOfArrayDimensions
			OpcUa_Int32 GetNoOfArrayDimensions();
			OpcUa_UInt32 operator [](int index);
			OpcUa_StatusCode Write(OpcUa_UInt32 AttributeId, OpcUa_String szIndexRange, OpcUa_DataValue Value);
			OpcUa_Boolean IsWritable();
			OpcUa_Boolean IsReadable();
			OpcUa_Boolean IsHistoryWritable();
			OpcUa_Boolean IsHistoryReadable();
			OpcUa_Byte GetAccessLevel();
			void  SetAccessLevel(OpcUa_Byte bAccessLevel);
			OpcUa_Byte GetUserAccessLevel();
			void  SetUserAccessLevel(OpcUa_Byte bUserAccessLevel);
			OpcUa_Double  GetMinimumSamplingInterval();
			void SetMinimumSamplingInterval(OpcUa_Double dblVal);
			OpcUa_Boolean GetHistorizing();
			void  SetHistorizing(OpcUa_Boolean bHistorizing);
			void*	GetPData();
			void SetPData(void* pData);
			// Accesseur BuiltInType
			OpcUa_Byte	GetBuiltInType();
			void SetBuiltInType(OpcUa_Byte bVal);
			// initialisation de la DataValue encapsulé.
			// Remarque : si la Datavalue de l'UAVariable est initialisé dans le fichier XML 
			// le canal de construction est différent
			OpcUa_StatusCode InitializeDataValue();
			void Lock();
			void UnLock();
			OpcUa_Void* Dump(); // extract the binary content of the encapsulated m_pDataValue
			void AddInternalEventDefinition(void* pInternalEventDefinition);
			void EvaluateEventDefinitionActiveState();
		private:
			CDataValue*					m_pDataValue;
			OpcUa_Mutex					m_DataValueMutex;
			OpcUa_NodeId				m_DataType;
			OpcUa_Byte					m_BuiltInType; // correspond au type natif associé au DataType.
			OpcUa_Int32					m_iValueRank;
			std::vector<OpcUa_UInt32>	m_ArrayDimensions;
			OpcUa_Byte					m_bAccessLevel;
			OpcUa_Byte					m_UserAccessLevel;
			OpcUa_Double				m_dblMinimumSamplingInterval;
			OpcUa_Boolean				m_bHistorizing;
			void*						m_pData; // internal server Data. This will contains the pointer to related CVpiTag
			std::vector<void*>			m_InternalEventDefinitionList; // internal server data. It will contains a vector will the CEventDefinition that use this UAVariable as a SourceNode
		};
		//struct LessNodeId 
		//{
		//	bool operator()(const OpcUa_NodeId* p1, const OpcUa_NodeId* p2) const
		//	{
		//		return OpcUa_NodeId_Compare(p1, p2) < 0;
		//	}
		//};
		typedef map<OpcUa_NodeId*, CUAVariable*, LessNodeId> CUAVariableList;
	}
}