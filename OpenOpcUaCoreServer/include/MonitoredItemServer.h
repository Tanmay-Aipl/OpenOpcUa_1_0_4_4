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
// Classe CMonitoredItem implémentée côté serveur
//
#pragma once
using namespace OpenOpcUa;
using namespace UASharedLib;
using namespace UAAddressSpace;
// Fin STL support for list
namespace OpenOpcUa
{
	namespace UACoreServer 
	{
		// This is a structure that hold the index of each eventField send to the client
		// Those index are used to handle the transition from one state to another
		// The indexes are initialized when the event is created. ie: ActiveStategoes to true
		// and change when it's acked or when ActiveState return to false
		// The indexes value are initialized in CMonitoredItemServer::InitializeEventsFields
		// the instance CUAEventNotificationList::m_FastEventFieldIndex store the indexes value for a Event
		typedef struct FastEventFieldIndex
		{
			OpcUa_Int32 iDateTime;
			OpcUa_Int32 iMessage;
			OpcUa_Int32 iActiveState;
			OpcUa_Int32 iActiveStateId;
			OpcUa_Int32 iActiveStateEffectiveDisplayName;
			OpcUa_Int32 iAckedState;
			OpcUa_Int32 iAckedStateId;
			OpcUa_Int32 iConfirmedState;
			OpcUa_Int32 iConfirmedStateId;
			OpcUa_Int32 iRetain;
			OpcUa_Int32 iSystemStateIndex; // index du CurrentState relatif à un SystemStatusChangeEventType
			OpcUa_Int32 iComment; // index of the eventfield the contains the comment. This will be filled by the client call Acknowledge
		} _FastEventFieldIndex;
		class CSubscriptionServer;
		class CUAEventNotificationList;
		class CMonitoredItemServer : public CMonitoredItemBase
		{
		public:
			CMonitoredItemServer(void);
			~CMonitoredItemServer(void);
			void AddTriggeredItemId(CMonitoredItemServer* pMonitoredItemServer);
			OpcUa_StatusCode RemoveTriggeredItemId(CMonitoredItemServer* pMonitoredItemServer);
			OpcUa_Boolean IsTriggeredItemId();
			vector<OpcUa_MonitoredItemNotification*> GetTriggeredMonitoredItemNotificationList();
			CSubscriptionServer* GetSubscription();
			void SetSubscription(CSubscriptionServer* pSubscription);

			OpcUa_Double GetSamplingInterval();
			void SetSamplingInterval(OpcUa_Double dblVal);
			OpcUa_ExtensionObject*	GetMonitoringFilter();
			void SetMonitoringFilter(OpcUa_ExtensionObject* ExObjVal);
			void SetMonitoringFilter(OpcUa_ExtensionObject ExObjVal);
			CMonitoredItemServer* operator=(CMonitoredItemServer* pItem);
			void SetNodeId(OpcUa_NodeId aNodeId);
			OpcUa_NodeClass	GetNodeClass();
			OpcUa_QualifiedName* GetBrowseName();
			OpcUa_LocalizedText GetDisplayName();
			OpcUa_LocalizedText GetDescription();
			OpcUa_UInt32 GetWriteMask();
			OpcUa_UInt32 GetUserWriteMask();
			void SetCold(OpcUa_Boolean bVal);
			OpcUa_Boolean IsCold();
			OpcUa_DateTime	GetLastNotificationTime();
			void SetLastNotificationTime(OpcUa_DateTime dtVal);
			CUABase* GetUABase();
			void SetUABase(CUABase*	pBase);
			OpcUa_StatusCode GetStatusCode();
			void SetStatusCode(OpcUa_StatusCode uStatus);
			//OpcUa_TimestampsToReturn   GetTimestampsToReturn() {return m_eTimestampsToReturn;}
			//void SetTimestampsToReturn(OpcUa_TimestampsToReturn eTimestampsToReturn) {m_eTimestampsToReturn=eTimestampsToReturn;}
			OpcUa_DataChangeFilter*	GetDataChangeFilter();
			void SetDataChangeFilter(OpcUa_DataChangeFilter* pDataChangeFilter);
			OpcUa_EventFilter*	GetEventFilter();
			void SetEventFilter(OpcUa_EventFilter* pEventFilter);
			vector<OpcUa_SimpleAttributeOperand*> GetEventFields();
			OpcUa_StatusCode GenerateDynamicEventField(OpcUa_String browsePath, CUAEventNotificationList** ppUAEventNotificationList);
			OpcUa_StatusCode GenerateFromAddressSpaceEventField(OpcUa_String browsePath, OpcUa_SimpleAttributeOperand* pSimpleAttributeOperand,CUAEventNotificationList** ppUAEventNotificationList);
			OpcUa_StatusCode InitializeEventsFieldsEx(CEventDefinition* pEventDefinition, CUAEventNotificationList** ppUAEventNotificationList);			
			OpcUa_StatusCode GetAlarmField(CUAObject* pConditionNode, OpcUa_SimpleAttributeOperand* pSimpleAttributeOperand, OpcUa_Variant** pValue);
			OpcUa_StatusCode GetDataTypeForBrowsePath(CUAObjectType* pObjectType, OpcUa_QualifiedName qualifiedName, OpcUa_NodeId* dataType);
		public:
			OpcUa_ExtensionObject*		m_pMonitoringFilter;
		private:
			CSubscriptionServer*		m_pSubscription;
			OpcUa_Boolean				m_IsCold; // permet d'indiquer que l'item vient d'être placer dans la souscritpion et qu'aucune notification n'a encore été transmise au client
			OpcUa_DateTime				m_LastNotificationTime; // heure de la dernière notification au client. Cette valeur est utilisée pour le mode Sampling. Afin de savoir s'il faut transmettre ou non au client. 
			vector<OpcUa_SimpleAttributeOperand*>		m_EventFields; // This collection is suppose to contains the EventFields require by the client for this CMonitoredItemServer
																   // those eventfields will be stored in a OpcUa_SimpleAttributeOperand
			CUABase*					m_pBase; // related CUABase
			OpcUa_StatusCode			m_StatusCode; // contient le code d'erreur retourné lors de l'appel CreateMonitoredItems
			OpcUa_EventFilter*			m_pEventFilter;
			OpcUa_DataChangeFilter*		m_pDataChangeFilter;
			//OpcUa_TimestampsToReturn	m_eTimestampsToReturn; // Timestamp demandé par le client lors du CreateMonitoredItems
			OpcUa_Mutex					m_TriggeredItemIdListMutex;
			std::vector<CMonitoredItemServer*>	m_TriggeredItemIdList; // List of ItemId trigger when this one changes

		};
		typedef vector<CMonitoredItemServer*> CMonitoredItemServerList;
	}
}