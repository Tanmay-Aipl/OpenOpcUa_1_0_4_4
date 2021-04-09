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
	namespace UACoreServer 
	{
		//typedef enum _EventTransactionStatus
		//{
		//	Unknown, // 0
		//	SentToClient, //1
		//	AckedByClient // 2
		//} EventTransactionStatus;
		// This the class that contains the list of eventField to notify to the client
		// Each instance represent an event notification
		class CUAEventNotificationList
		{
		public:
			CUAEventNotificationList(void);
			CUAEventNotificationList(OpcUa_UInt32 uiSequenceNumber);
			~CUAEventNotificationList(void);
			OpcUa_Int32 GetNoOfEvents();
			OpcUa_EventFieldList* GetEventField(OpcUa_Int32 index);
			OpcUa_UInt32 GetSequenceNumber();
			void SetSequenceNumber(OpcUa_UInt32 uiVal);
			OpcUa_Boolean IsTransactionAcked();
			void TransactionAcked(OpcUa_Boolean bVal);
			OpcUa_Boolean IsAlarmAcked();
			void AlarmAcked();
			OpcUa_Boolean IsKeepAlive();
			NotificationMessageType GetNotificationType();
			void SetNotificationType(NotificationMessageType aDataChangeNotificationType);
			CMonitoredItemServer* GetMonitoredItem();
			void SetMonitoredItem(CMonitoredItemServer* pMonitoredItem);
			OpcUa_EventNotificationList* GetInternalEventNotificationList();
			void SetInternalEventNotificationList(OpcUa_EventNotificationList* pInternalEventNotificationList);
			void SetEventDefinition(CEventDefinition* pRelatedEventDefinition);
			CEventDefinition* GetEventDefinition();
			OpcUa_StatusCode UpdateInternalAckedState(OpcUa_Boolean bNewState);
			OpcUa_StatusCode UpdateInternalActiveState(OpcUa_Boolean bNewState);
			OpcUa_StatusCode UpdateInternalConfirmedState(OpcUa_Boolean bNewState);
			OpcUa_StatusCode UpdateInternalDateTime(OpcUa_DateTime dtValue);
			OpcUa_StatusCode UpdateInternalMessage();
			OpcUa_StatusCode GetValueForPlaceHolder(OpcUa_String szPlaceHolder, OpcUa_String* pszValue);
			FastEventFieldIndex GetFastEventFieldIndex();
			void SetFastEventFieldIndex(FastEventFieldIndex *pFastEventFieldIndex);
			OpcUa_Int32 GetCurrentStateIndex();
			// LastNotifiedActiveState
			void SetLastNotifiedActiveState(OpcUa_Boolean bVal);
			OpcUa_Boolean GetLastNotifiedActiveState();
			// Terminated
			OpcUa_Boolean IsTerminated();
			void Terminate();
			// LastNotifiedAlarmAcked
			void SetLastNotifiedAlarmAcked(OpcUa_Boolean bVal);
			OpcUa_Boolean GetLastNotifiedAlarmAcked();
			OpcUa_StatusCode AddComment(const OpcUa_LocalizedText comment); // Add the comment sent by the client during the acknowledgement
			// Confirm
			OpcUa_Boolean IsConfirmed();
			void Confirmed();
			OpcUa_Boolean GetLastNotifiedConfirmed();
			void SetLastNotifiedConfirmed(OpcUa_Boolean bVal);
			// base on a couple of bool determine if the EventList need to be notified or if it was already done
			OpcUa_Boolean IsEventListNotifiable();
			OpcUa_UInt32 GetTransactionStatus();
			void SetTransactionStatus(OpcUa_UInt32 eStatus);
		private:
			OpcUa_EventNotificationList*	m_pInternalEventNotificationList;
			//typedef struct _OpcUa_EventNotificationList
			//{
			//	OpcUa_Int32           NoOfEvents;
			//	OpcUa_EventFieldList* Events;
			//}
			//OpcUa_EventNotificationList;
			//typedef struct _OpcUa_EventFieldList
			//{
			//	OpcUa_UInt32   ClientHandle;
			//	OpcUa_Int32    NoOfEventFields;
			//	OpcUa_Variant* EventFields;
			//}
			//OpcUa_EventFieldList;
			OpcUa_UInt32					m_uiSequenceNumber; // n° de séquence de ce EventNotificationList
			OpcUa_UInt32					m_eTransactionStatus; // indicates whether the sequence number associated with this notification was acquitted.
																 // This is the OPC UA transaction ack
																 // if false the Event will notifed to the client
			OpcUa_Boolean					m_bAlarmAcked; // Reflect the calls of the method Acknowledge
			OpcUa_Boolean					m_bTerminated; // True if the alarm is terminated <==> Ackowledge and not active
			OpcUa_Boolean					m_bConfirmed;  // Reflect the call of the confirm method. 
														   // When the client call Confirm Method the m_bConfirmed will because true and it will be notified to the client
			OpcUa_Boolean					m_bLastNotifiedConfirmed;
			NotificationMessageType			m_NotificationDataType;
			CMonitoredItemServer*			m_pMonitoredItemServer;
			CEventDefinition*				m_pRelatedEventDefinition; // for internal use do not release
			FastEventFieldIndex				m_FastEventFieldIndex; // index permettant la mise à jour rapide des eventFields
			OpcUa_Boolean					m_bLastNotifiedActiveState; // indicate the LastActiveState notified to the client. This boolean is only used to avoid bounce
			OpcUa_Boolean					m_bLastNotifiedAlarmAcked; // indicate the LastNotifiedAlarmAcked notified to the client. This boolean is only used to avoid bounce
		};
		typedef vector<CUAEventNotificationList*> CUAEventNotificationListList;
	}
}