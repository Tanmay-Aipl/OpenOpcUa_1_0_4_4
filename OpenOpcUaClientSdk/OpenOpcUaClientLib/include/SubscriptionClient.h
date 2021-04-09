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
	namespace UACoreClient 
	{

		class CSessionClient;
		class CSubscriptionClient :	public OpenOpcUa::UASharedLib::CSubscription
		{
		public:
			CSubscriptionClient(void);
			~CSubscriptionClient(void);
			// création d'un MonitoredItem
			OpcUa_StatusCode CreateMonitoredItems(
								OpcUa_TimestampsToReturn                a_eTimestampsToReturn,
								OpcUa_Int32                             a_nNoOfItemsToCreate,
								OpcUa_MonitoredItemCreateRequest*		pItemsToCreate,
								OpcUa_MonitoredItemCreateResult** ppResult,
												OpcUa_Handle** hMonitoredItems);
			// Modify a ModnitoredItem
			OpcUa_StatusCode ModifyMonitoredItems(OpcUa_Handle	hMonitoredItems,
																		OpcUa_Byte		aTimestampsToReturn,
																		OpcUa_Boolean	bDiscardOldest,
																		OpcUa_UInt32	uiQueueSize,
																		OpcUa_Double	dblSamplingInterval,
																		OpcUa_UInt32	DeadbandType,
																		OpcUa_Double	DeadbandValue,
																		OpcUa_Byte		DatachangeTrigger);
			// Change the monitoring mode of the subscription
			OpcUa_StatusCode SetMonitoringMode(OpcUa_MonitoringMode iMonitoringMode, OpcUa_Int32 iNoOfMonitoredItems, OpcUa_UInt32* hMonitoredItems);
			// Change the subscription parameters
			OpcUa_StatusCode ModifySubscription(OpcUa_Double nRequestedPublishingInterval,
												OpcUa_UInt32 nRequestedLifetimeCount,
												OpcUa_UInt32 nRequestedMaxKeepAliveCount,
												OpcUa_UInt32 nMaxNotificationsPerPublish,
												OpcUa_Byte   nPriority);
			// Change the publishing mdoe for the subscription
			OpcUa_StatusCode SetPublishingMode(OpcUa_Boolean bMode);
			// Sequenc enumber manipulation
			void AddSequenceNumber(OpcUa_UInt32 uiVal);
			void SequenceNumberClearAll();
			OpcUa_Boolean IsInSequenceNumber(OpcUa_UInt32 uiVal);
			//
			OpcUa_StatusCode AddMonitoredItem(CMonitoredItemClient* pMonitoredItem);
			OpcUa_StatusCode DeleteMonitoredItemById(OpcUa_UInt32 Id);
			CMonitoredItemClient* FindMonitoredItemById(OpcUa_UInt32 Id);
			CMonitoredItemClient* FindMonitoredItemByHandle(OpcUa_UInt32 uiClientHandle);
			OpcUa_StatusCode DeleteMonitoredItems();
			OpcUa_StatusCode DeleteMonitoredItems(CMonitoredItemClientList* pMonitoredItemList);
			OpcUa_StatusCode DeleteMonitoredItem(CMonitoredItemClient* pMonitoredItemClient);
			OpcUa_Mutex GetMonitoredItemListMutex() {return m_MonitoredItemListMutex;}
			CMonitoredItemClientList* GetMonitoredItemList() {return m_pMonitoredItemList;}
			CSessionClient* GetSession() {return m_pSession;}
			void SetSession(CSessionClient* pSession) 
			{
				m_pSession=pSession;
				UpdateSubscriptionDiagnosticsDataType();
			}
			void UpdateSubscriptionDiagnosticsDataType();
			OpenOpcUa::UASharedLib::CSubscriptionDiagnosticsDataType* GetSubscriptionDiagnosticsDataType() 
			{
				UpdateSubscriptionDiagnosticsDataType();
				return	m_pSubscriptionDiagnosticsDataType;
			}
			OpcUa_Handle GetStopMonitoredItemsThreadHandle() {return m_hStopMonitoredItemsNotificationSem;}

			PFUNC GetNotificationCallback() {return m_pNotificationCallback;}
			void SetNotificationCallback(PFUNC pNotificationCallback) {m_pNotificationCallback=pNotificationCallback;}
			void* GetNotificationCallbackData() {return m_pNotificationCallbackData;}
			void SetNotificationCallbackData(void* pCallbackData) {m_pNotificationCallbackData=pCallbackData;}
			OpcUa_StatusCode AddMonitoredItemNotification(CMonitoredItemsNotification* pMonitoredItemsNotification);
			OpcUa_StatusCode CleanMonitoredItemNotificationQueue();
			OpcUa_UInt32*  GetAvailableSequenceNumbers(); 
			OpcUa_Int32    GetNoOfAvailableSequenceNumbers();
			OpcUa_StatusCode GetXmlAttributes(vector<OpcUa_String*>* szAttributes);
			OpcUa_StatusCode GetMonitoredItems(OpcUa_UInt32* uiNoOfMonitoredItems, OpcUa_Handle** hMonitoredItems);
			OpcUa_StatusCode DeleteFromMonitoredItemList(CMonitoredItemClient* pMonitoredItem);
		protected:
			void StartMonitoredItemsNotificationThread();// stub pour la creation de la thread pour la notification de MonitoredItem
			static void MonitoredItemsNotificationThread(LPVOID arg);// fonction associé a la thread pour la notification de MonitoredItem
			OpcUa_StatusCode StopMonitoredItemsNotificationThread(); // Méhtode permettant de stopper la thread de notification des MonitoredItem
			//
		private:
			OpcUa_Thread							m_hMonitoredItemsNotificationThread; // Thread pour la notification de MonitoredItem
			OpcUa_Boolean							m_bMonitoredItemsNotificationThread;
			OpcUa_Semaphore							m_hStopMonitoredItemsNotificationSem;
			OpcUa_Semaphore							m_hMonitoredItemsNotificationSem;
			OpcUa_Mutex								m_MonitoredItemsNotificationListMutex; // mutex de protection de pMonitoredItems
			CMonitoredItemsNotificationList*		m_pMonitoredItemsNotificationList; // Liste des monitoredItems a notifier
			CSessionClient*							m_pSession;
			CMonitoredItemClientList*				m_pMonitoredItemList; // Liste des MonitoredItems associé a cette souscription
			OpcUa_Mutex								m_MonitoredItemListMutex; // Protège l'accès a m_pMonitoredItemList
			OpenOpcUa::UASharedLib::CSubscriptionDiagnosticsDataType*	m_pSubscriptionDiagnosticsDataType;
			
			PFUNC									m_pNotificationCallback;
			void*									m_pNotificationCallbackData;			       
		private:
			OpcUa_Int32								m_NoOfAvailableSequenceNumbers; // retourné par la dernière notification.. -1 à l'init
			OpcUa_UInt32*							m_pAvailableSequenceNumbers;  // retourné par la dernière notification. NULL a l'init
			vector<OpcUa_UInt32>					m_vectOfAvailableSequenceNumber;
		};
		typedef std::vector<CSubscriptionClient*> CSubscriptionList;  // version STL
	}
}
