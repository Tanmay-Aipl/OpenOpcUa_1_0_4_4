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
#include <list>
using namespace std;

namespace OpenOpcUa
{
	namespace UASharedLib 
	{
		class SHAREDLIB_EXPORT CMonitoredItemBase
		{
		public:
			CMonitoredItemBase(void);
			virtual ~CMonitoredItemBase(void);
			// Name
			OpcUa_String* GetMonitoredItemName();
			void SetMonitoredItemName(OpcUa_String strName);
			// Value
			OpcUa_DataValue* GetValue();
			void SetValue(OpcUa_DataValue* pVal);
			// variables

			OpcUa_TimestampsToReturn   GetTimestampsToReturn();
			void SetTimestampsToReturn(OpcUa_TimestampsToReturn eTimestampsToReturn);
			OpcUa_NodeId GetNodeId();
			void SetNodeId(OpcUa_NodeId aNodeId);
			OpcUa_UInt32 GetQueueSize();
			void SetQueueSize(OpcUa_UInt32 uiVal);
			OpcUa_UInt32 GetAttributeId();
			void SetAttributeId(OpcUa_UInt32 uiVal);
			OpcUa_String* GetIndexRange();
			void SetIndexRange(OpcUa_String* pIndexRange);
			OpcUa_QualifiedName GetDataEncoding();
			void SetDataEncoding(OpcUa_QualifiedName qName);

			OpcUa_UInt32 GetClientHandle();
			void SetClientHandle(OpcUa_UInt32 uiVal);
			OpcUa_Boolean IsDiscardOldest();
			void DiscardOldest(OpcUa_Boolean bVal);
			OpcUa_ExtensionObject GetFilterToUse();
			void SetFilterToUse(OpcUa_ExtensionObject exObj);
			OpcUa_StatusCode GetLastError();
			void SetLastError(OpcUa_StatusCode uStatus);
			OpcUa_Mutex GetMonitoredItemMutex();
			void SetMonitoredItemCS(OpcUa_Mutex* critSect);
			OpcUa_Double GetSamplingInterval();
			void SetSamplingInterval(OpcUa_Double dblVal);
			OpcUa_MonitoringMode GetMonitoringMode();
			void SetMonitoringMode(OpcUa_MonitoringMode mode);
			CSessionBase* GetSession();
			void SetSession(CSessionBase* pSession);
			void* GetUserData();
			void SetUserData(void* pData);
			CMonitoredItemBase* operator=(CMonitoredItemBase* pItem);
			OpcUa_Boolean IsChanged();
			void Changed(OpcUa_Boolean bVal);
			void SetClassName(std::string className);
			std::string GetClassName();
			OpcUa_UInt32	GetMonitoredItemId();
			void SetMonitoredItemId(OpcUa_UInt32 uiMonitoredItemId);
		protected:
			CSessionBase*				m_pSession;
			OpcUa_UInt32				m_MonitoredItemId;
			OpcUa_NodeId				m_NodeId; // nodeId de cet item
			OpcUa_Boolean				m_bChange;
			OpcUa_Int32					m_iDiagnosticsMasks; // il serait préférable de déclarer un "DiagnosticsMasks cf stack .Net"
			OpcUa_TimestampsToReturn	m_eTimestampsToReturn;
			OpcUa_UInt32				m_clientHandle;
			OpcUa_MonitoringMode		m_monitoringMode;
			OpcUa_Double				m_samplingInterval;
			OpcUa_Boolean				m_bDiscardOldest;  
			OpcUa_StatusCode			m_lastError; // Le type utilisé etait une classe ServiceResult. Elle combine en C# statuscode et diagInfo
			OpcUa_Boolean				m_bOverflow;
			void*						m_pUserData; // handle to the item that contain a specific user data
			OpcUa_Mutex					m_MonitoredItemMutex; //
			//
			OpcUa_QualifiedName			m_dataEncoding;
			OpcUa_String*				m_pIndexRange;
			OpcUa_UInt32				m_queueSize;
			OpcUa_UInt32				m_attributeId;
			OpcUa_String*				m_pMonitoredItemName;// is the DisplayName get from the OpcUa_ReferenceDescription
			OpcUa_DataValue*			m_pValue;    
			std::string					m_ClassName;         
		};
	}
}