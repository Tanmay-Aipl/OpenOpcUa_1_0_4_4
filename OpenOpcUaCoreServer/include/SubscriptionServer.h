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

using namespace UASharedLib;
namespace OpenOpcUa
{
	namespace UACoreServer 
	{		
		class CSubscriptionServer :	public CSubscription
		{
		public:
			CSubscriptionServer(void);
			CSubscriptionServer(CSessionServer* pSession,double dblSubscriptionInterval);
			~CSubscriptionServer(void);
			// DataChangeNotificationList
			OpcUa_Mutex	GetDataChangeNotificationListMutex();
			//CUADataChangeNotificationList*	GetDataChangeNotificationList();
			CUADataChangeNotification* GetUnackedDataChangeNotification();
			// StatusChangeNotificationList
			OpcUa_Mutex GetStatusChangeNotificationListMutex();
			CUAStatusChangeNotificationList* GetStatusChangeNotificationList();
			// EventNotificationListList
			CUAEventNotificationListList* GetEventNotificationListList();
			OpcUa_Mutex GetEventNotificationListListMutex();
			OpcUa_UInt32 GetEventNotificationListListCurrentPos();
			void SetEventNotificationListListCurrentPos(OpcUa_UInt32 uiVal);
			OpcUa_StatusCode AddEventNotificationList(CUAEventNotificationList* pUAEventNotificationList);
			OpcUa_StatusCode GetEventNotificationListFromEventId(OpcUa_ByteString* pByteString, CUAEventNotificationList** pUAEventNotificationList);
			OpcUa_StatusCode GetEventNotificationListFromEventDefinition(CEventDefinition* pEventDefinition, CUAEventNotificationListList* pUAEventNotificationListList);
			OpcUa_StatusCode HandleSystemStatusChangeEvent(CMonitoredItemServer* pMonitoredItem); // prepare, if needed a new SystemStatusChangeEvent
			OpcUa_StatusCode HandleAlreadyNotifiedEvent(); // Handle already notified event
			OpcUa_UInt32 GetCurrentSequenceNumber();
			void SetCurrentSequenceNumber(OpcUa_UInt32 iVal);
			void SetLastPublishAnswer(OpcUa_UInt32 uiVal);
			OpcUa_Boolean  IsSomethingNotify();
			// Accessor permettant d'indiquer qu'il y a eu un changement sur cette souscription
			OpcUa_Boolean IsChanged();
			void Changed(OpcUa_Boolean bVal);
			// Change the monitoring mode of the subscription
			OpcUa_MonitoringMode GetMonitoringMode();
			void SetMonitoringMode(OpcUa_MonitoringMode iMonitoringMode);
			OpcUa_UInt32 GetDataChangeNotificationCurrentPos();
			void SetDataChangeNotificationCurrenPos(OpcUa_UInt32 uiVal);
			OpcUa_UInt32 GetStatusChangeNotificationCurrentPos();
			void SetStatusChangeNotificationCurrenPos(OpcUa_UInt32 uiVal);
			CSessionServer* GetSession();
			void SetSession(CSessionServer* pSession);
			OpcUa_StatusCode ProcessQueuedPublishRequestEx(CQueuedPublishMessage* pRequest,  OpcUa_Boolean bAbort);	
			//OpcUa_StatusCode ProcessQueuedPublishRequest(CQueuedPublishMessage* pRequest,  OpcUa_Boolean bAbort);					
			// Prise en charge de la liste des items monitoré pour cette souscription
			CMonitoredItemServerList* GetMonitoredItemList();
			OpcUa_StatusCode AddMonitoredItem(CMonitoredItemServer* pMonitoredItem);
			OpcUa_StatusCode DeleteMonitoredItems(/*OpcUa_UInt32 uiNbResults,OpcUa_StatusCode** pMyResults*/);
			OpcUa_StatusCode FindMonitoredItemById(OpcUa_UInt32 Id,CMonitoredItemServer** pItem);
			OpcUa_StatusCode DeleteMonitoredItemById(OpcUa_UInt32 Id);		
			OpcUa_StatusCode DeleteDataChangeNotfication(OpcUa_UInt32 uiClientHandle);// Suppression des OpcUa_DataChangeNotfication associé a ce clientHandle
			OpcUa_StatusCode DeleteAllDataChangeNotfication();

			OpcUa_Boolean IsMonitoredItemFromUABaseEqual(CUABase* pUABase,CMonitoredItemServer* pMonitoredItemserver);
			OpcUa_Boolean IsMonitoredItemFromUAObjectEqual(CUAObject* pUAObject,CMonitoredItemServer* pMonitoredItemserver);
			OpcUa_Boolean IsMonitoredItemFromUAVariableEqual(CUAVariable* pUAVariable,CMonitoredItemServer* pMonitoredItemserver);
			// Method to manage SequenceNumber
			OpcUa_Boolean AvailableSequenceNumberAdd(OpcUa_UInt32 uiSequenceNumber, CUADataChangeNotification* pDataChangeNotification);
			void AvailableSequenceNumberDelete(OpcUa_UInt32 uiSequenceNumber);
			CAvailableSequenceNumbers AvailableSequenceNumberGet();
			OpcUa_StatusCode AvailableSequenceNumberGetByDataChangeNotification(CUADataChangeNotification* pDataChangeNotification);
			OpcUa_StatusCode AvailableSequenceNumberGet(OpcUa_UInt32 uiSequenceNumber,CUADataChangeNotification* pDataChangeNotification);
			OpcUa_Boolean IsValueEqualWithDataChangeFilter(OpcUa_DataValue* pUAVariableValue, OpcUa_DataValue* pMonitoredValue, OpcUa_DataChangeFilter* pDataChangeFilter, OpcUa_String * pRange);
			OpcUa_StatusCode AckDataChangeNotification(OpcUa_UInt32 uiSequence); // acquittement des DataChangeNotification de cette Souscription
			CSubscriptionDiagnosticsDataType* GetSubscriptionDiagnosticsDataType();
			void SetSubscriptionDiagnosticsDataType(CSubscriptionDiagnosticsDataType* pSubscriptionDiagnosticsDataType);
			OpcUa_StatusCode UpdateSubscriptionDiagnosticsDataType();
			OpcUa_Mutex	GetMonitoredItemListMutex() {return m_hMonitoredItemListMutex; }
			OpcUa_StatusCode ModifySubscription(
										OpcUa_Double               a_nRequestedPublishingInterval,
										OpcUa_UInt32               a_nRequestedLifetimeCount,
										OpcUa_UInt32               a_nRequestedMaxKeepAliveCount,
										OpcUa_UInt32               a_nMaxNotificationsPerPublish,
										OpcUa_Byte                 a_nPriority,
										OpcUa_Double*              a_pRevisedPublishingInterval,
										OpcUa_UInt32*              a_pRevisedLifetimeCount,
										OpcUa_UInt32*              a_pRevisedMaxKeepAliveCount);
			OpcUa_Boolean IsInColdState();
			void SetInColdState(OpcUa_Boolean bVal);
			// Force la reveil de l' UpdateDataSubscriptionThread
			void WakeupUpdateSubscriptionThread();
			OpcUa_StatusCode Republish( OpcUa_UInt32 a_nRetransmitSequenceNumber,
										OpcUa_NotificationMessage** a_pNotificationMessage);
			//OpcUa_Semaphore	GetStopUpdateSubscriptionSem();
			OpcUa_Boolean isInLate();
			void InLate(OpcUa_Boolean bVal);
			OpcUa_Boolean WasKeepAlive();
			void SetWasKeepAlive(OpcUa_Boolean bVal);
			OpcUa_Boolean IsKeepAliveRequired();
			// indique que (m_pRevisedLifetimeCount * m_pRevisedPublishingInterval) est atteint et que l'on doit arrété cette souscritpion
			OpcUa_Boolean IsLifetimeCountReach();
			OpcUa_StatusCode AddStatusChangeNotificationMessage(OpcUa_DiagnosticInfo aDiagInfo, OpcUa_StatusCode uNewStatus);
			OpcUa_StatusCode FindMonitoredItemByClientHandle(OpcUa_UInt32 clientHandle,CMonitoredItemServer** pItem);
			void ResetLifeTimeCountCounter();
			//void ResetKeepAliveCounter();
			OpcUa_StatusCode  ConditionRefresh(); // method to do the Event refresh for this subscription
			OpcUa_Boolean IsOnStage(CEventDefinition* pEventDefinition); // Check if the current CEventDefiniton is "OnStage" for this subscription
			OpcUa_Double GetFastestSamplingInterval();
			void UpdateFastestSamplingInterval();
			OpcUa_StatusCode RemoveAckedDataChangeNotification();
			OpcUa_StatusCode RemoveAckedStatusChangeNotification();
			OpcUa_StatusCode RemoveAckedEventNotificationList();
			void SetKeepAlive(OpcUa_Boolean bVal);
			// Last time we send something to the client on behlf of this subscription
			void SetLastTransmissionTime(OpcUa_DateTime dtVal);
			OpcUa_DateTime GetLastTransmissionTime(void);

		private:
			OpcUa_StatusCode CompareMonitoredItemToUAObject(CUADataChangeNotification** pDataChangeNotification,CUABase* pUABase,CMonitoredItemServer* pMonitoredItem,OpcUa_Boolean* pbPreIncKeepAliveCounter);
			OpcUa_StatusCode CompareMonitoredItemToUAVariable(CUADataChangeNotification** pDataChangeNotification, 
																CUAVariable* pUAVariable, 
																CMonitoredItemServer* pMonitoredItem, 
																OpcUa_Boolean* pbPreIncKeepAliveCounter, 
																OpcUa_Boolean* pbRecycledDataChangeNotification);
			OpcUa_StatusCode RemoveStatusChangeNotification(CUAStatusChangeNotification* pStatusChangeNotification);
			OpcUa_StatusCode RemoveEventNotificationList(CUAEventNotificationList* pEventNotificationList);
		protected:
			void StartUpdateSubscriptionThread();
			OpcUa_StatusCode StopUpdateSubscriptionThread();
			static void /*__stdcall*/ UpdateDataSubscriptionThread(LPVOID arg);
		private:
			OpcUa_Mutex									m_AvailableSequenceNumbersMutex;
			CAvailableSequenceNumbers					m_AvailableSequenceNumbers; // this map contains the list of available (unack) CUADataChangeNotification sequence n° for this subcription
																					// the key of the map is the sequence number
																					// it's made of <n° sequence,CUADataChangeNotification*>
			OpcUa_UInt32								m_uiCurrentSequenceNumber; // N° de la sequence en cours (derniere transmise)
			CMonitoredItemServerList*					m_pMonitoredItemList; // Liste de monitoredItems pris en charge par cette souscription
			OpcUa_Mutex									m_hMonitoredItemListMutex; // Protège l'accès a m_pMonitoredItemList
			OpcUa_Semaphore								m_MonitoredItemListSem;
			CSessionServer*								m_pSession; // Session associé a cette souscription
			OpcUa_Boolean								m_bChanged; // Bool permettant d'indiquer qu'il y a eu un changement sur cette souscription
			// variables for the publishing thread
			OpcUa_Thread								m_hUpdateDataSubscriptionThread;
			OpcUa_Boolean								m_bRunUpdateDataSubscriptionThread;
			OpcUa_Semaphore								m_hStopUpdateDataSubscriptionSem; // Semaphore qui synchronise l'arrêt de la thread de mise a jour (UpdateDataSubscriptionThread)
			OpcUa_Semaphore								m_hUpdateSubscriptionThreadSafeSem; // EventSem qui permet d'indiquer que la  thread de mise a jour (UpdateDataSubscriptionThread) peut être arrêté
			// Variable de classe utilisé pour la notification des changement d'état sur les données
			OpcUa_Mutex									m_hDataChangeNotificationListMutex; // mutex de protection de m_pDataChangeNotificationList
			//CUADataChangeNotificationList*				m_pDataChangeNotificationList;
			OpcUa_UInt32								m_DataChangeNotificationCurrentPos; // indice du dernier CUADataChangeNotification non acquité dans la CUADataChangeNotificationList (m_pDataChangeNotificationList)
			// Variable de classe utilisé pour la notification des changement d'état interne du serveur
			OpcUa_Mutex									m_hStatusChangeNotificationListMutex;
			CUAStatusChangeNotificationList*			m_pStatusChangeNotificationList;
			OpcUa_UInt32								m_StatusChangeNotificationCurrentPos;
			// Variable de classe utilisé pour la notification des evenements
			OpcUa_Mutex									m_hEventNotificationListListMutex;
			CUAEventNotificationListList*				m_pEventNotificationListList; // This crappy name comes from the UA spec part 9 editor. 
																					  // This is in fact the Event cache. This means the list of event active for this subscription
			OpcUa_UInt32								m_EventNotificationListCurrentPos; // indice du dernier CUAEventNotificationList non acquité dans la CUAEventNotificationListList (m_pEventNotificationListList)
			//
			OpcUa_Int32									m_iLastSentMessage; // Dernier message transmis			
			CSubscriptionDiagnosticsDataType*			m_pSubscriptionDiagnosticsDataType;
			OpcUa_Boolean								m_bSubscriptionInColdState; //permet d'indiquer que la souscription vient d'être lancé et n'a jamais répondue a un Publish
			OpcUa_Boolean								m_bKeepAlive; // A boolean that indicate the we will send a beepAlive for this subscription (internal use for sequence number)
			OpcUa_Boolean								m_bWasKeepAlive; // tells that the last notificationMessage was a keepAlive
			OpcUa_DateTime								m_LastTransmissionTime;
			OpcUa_Boolean								m_bInLate; // Bool indiquant que l'abonnement est en retard. C'est a dire que l'on a essayé de notifier le client mais qu'aucun Publish n'était disponible dans le queue.
																   // Dans ce cas on notifiera aussitot le prochain publish arrivé.
			OpcUa_Boolean								m_bLifetimeCountReach; // indique que (m_pRevisedLifetimeCount * m_pRevisedPublishingInterval) est atteint et que l'on doit arrété cette souscritpion
																			   // cela sera réalisé au niveau de CSessionServer dans la Thread SubscriptionsLifeTimeCountThread
			OpcUa_UInt32								m_uiLifeTimeCountCounter;  // compteur de LifeTimecount. Quand cette valeur sera égale ou supérieur a xxx le m_bLifetimeCountReach sera positionné a OpcUa_True afin que la souscription soit détruite par la session a laquelle elle appartient 
			OpcUa_UInt32								m_uiLastPublishAnswer;
			OpcUa_ServerState							m_LastKnownServerState; // This attribute contains the last notified ServerState. This is used to notify SystemStatusChangeEventType
			OpcUa_Double								m_fastestSamplingInterval;
		};	
		typedef std::vector<CSubscriptionServer*> CSubscriptionList;  // version STL
	}
}