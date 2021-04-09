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
		// classe wrapper for OpcUa_SessionSecurityDiagnosticsDataType voir detail part 5 Table 135
		class  SHAREDLIB_EXPORT CSessionDiagnosticsDataType
		{
		public:
			CSessionDiagnosticsDataType(void);
			CSessionDiagnosticsDataType(OpcUa_SessionDiagnosticsDataType* pSessionSecurityDiagnosticsDataType);
			~CSessionDiagnosticsDataType(void);
			void UpdateInternalSessionDiagnosticsDataType();
			void UpdateVariables();
			OpcUa_UInt32 GetInstanceSize();
			void UpdateInstanceSize();
			OpcUa_NodeId GetSessionId();
			void SetSessionId(OpcUa_NodeId* aNodeId);
			OpcUa_String GetSessionName();
			void SetSessionName(OpcUa_String strVal);
			OpcUa_String GetServerUri();
			void SetServerUri(OpcUa_String strVal);
			OpcUa_String GetEndpointUrl();
			void SetEndpointUrl(OpcUa_String* strVal);
			OpcUa_ApplicationDescription GetClientDescription();
			void SetClientDescription(const OpcUa_ApplicationDescription* pAppDescription);
			OpcUa_SessionDiagnosticsDataType* GetInternalPtr();
		private:
			/*
			OpcUa_ApplicationDescription		m_ClientDescription;
			OpcUa_String						m_ServerUri;
			OpcUa_String						m_EndpointUrl;
			OpcUa_Int32							m_NoOfLocaleIds;
			OpcUa_String*						m_LocaleIds;
			OpcUa_Double						m_ActualSessionTimeout;
			OpcUa_UInt32						m_MaxResponseMessageSize;
			OpcUa_DateTime						m_ClientConnectionTime;
			OpcUa_DateTime						m_ClientLastContactTime;
			OpcUa_UInt32						m_CurrentSubscriptionsCount;
			OpcUa_UInt32						m_CurrentMonitoredItemsCount;
			OpcUa_UInt32						m_CurrentPublishRequestsInQueue;
			OpcUa_ServiceCounterDataType		m_TotalRequestCount;
			OpcUa_UInt32						m_UnauthorizedRequestCount;
			OpcUa_ServiceCounterDataType		m_ReadCount;
			OpcUa_ServiceCounterDataType		m_HistoryReadCount;
			OpcUa_ServiceCounterDataType		m_WriteCount;
			OpcUa_ServiceCounterDataType		m_HistoryUpdateCount;
			OpcUa_ServiceCounterDataType		m_CallCount;
			OpcUa_ServiceCounterDataType		m_CreateMonitoredItemsCount;
			OpcUa_ServiceCounterDataType		m_ModifyMonitoredItemsCount;
			OpcUa_ServiceCounterDataType		m_SetMonitoringModeCount;
			OpcUa_ServiceCounterDataType		m_SetTriggeringCount;
			OpcUa_ServiceCounterDataType		m_DeleteMonitoredItemsCount;
			OpcUa_ServiceCounterDataType		m_CreateSubscriptionCount;
			OpcUa_ServiceCounterDataType		m_ModifySubscriptionCount;
			OpcUa_ServiceCounterDataType		m_SetPublishingModeCount;
			OpcUa_ServiceCounterDataType		m_PublishCount;
			OpcUa_ServiceCounterDataType		m_RepublishCount;
			OpcUa_ServiceCounterDataType		m_TransferSubscriptionsCount;
			OpcUa_ServiceCounterDataType		m_DeleteSubscriptionsCount;
			OpcUa_ServiceCounterDataType		m_AddNodesCount;
			OpcUa_ServiceCounterDataType		m_AddReferencesCount;
			OpcUa_ServiceCounterDataType		m_DeleteNodesCount;
			OpcUa_ServiceCounterDataType		m_DeleteReferencesCount;
			OpcUa_ServiceCounterDataType		m_BrowseCount;
			OpcUa_ServiceCounterDataType		m_BrowseNextCount;
			OpcUa_ServiceCounterDataType		m_TranslateBrowsePathsToNodeIdsCount;
			OpcUa_ServiceCounterDataType		m_QueryFirstCount;
			OpcUa_ServiceCounterDataType		m_QueryNextCount;
			OpcUa_ServiceCounterDataType		m_RegisterNodesCount;
			OpcUa_ServiceCounterDataType		m_UnregisterNodesCount;     
			*/
		protected:
			OpcUa_SessionDiagnosticsDataType*	m_pInternalSessionDiagnosticsDataType;
		};
		typedef std::vector<CSessionDiagnosticsDataType*> CSessionDiagnosticsDataTypeList;
	}
}