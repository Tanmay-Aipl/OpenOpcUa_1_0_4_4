#pragma once
namespace OpenOpcUa
{
	namespace UASharedLib 
	{
		class SHAREDLIB_EXPORT CSubscriptionDiagnosticsDataType
		{
		public:
			CSubscriptionDiagnosticsDataType(void);
			CSubscriptionDiagnosticsDataType(OpcUa_SubscriptionDiagnosticsDataType* pSubscriptionDiagnosticsDataType);
			~CSubscriptionDiagnosticsDataType(void);
			OpcUa_SubscriptionDiagnosticsDataType* GetInternalSubscriptionDiagnosticsDataType();
			void SetInternalSubscriptionDiagnosticsDataType(OpcUa_SubscriptionDiagnosticsDataType* pInternalSubscriptionDiagnosticsDataType);
			// SessionId
			OpcUa_NodeId  GetSessionId();
			void SetSessionId(OpcUa_NodeId aNodeId);
			// SubscriptionId
			OpcUa_UInt32  GetSubscriptionId();
			void SetSubscriptionId(OpcUa_UInt32  uiSubscriptionId);
			// Priority
			OpcUa_Byte    GetPriority();
			void SetPriority(OpcUa_Byte    Priority);
			// PublishingInterval
			OpcUa_Double  GetPublishingInterval();
			void SetPublishingInterval(OpcUa_Double  PublishingInterval);
			// MaxKeepAliveCount
			OpcUa_UInt32  GetMaxKeepAliveCount();
			void SetMaxKeepAliveCount(OpcUa_UInt32  MaxKeepAliveCount);
			// MaxLifetimeCount
			OpcUa_UInt32  GetMaxLifetimeCount();
			void SetMaxLifetimeCount(OpcUa_UInt32  MaxLifetimeCount);
			// MaxNotificationsPerPublish
			OpcUa_UInt32  GetMaxNotificationsPerPublish();
			void SetMaxNotificationsPerPublish(OpcUa_UInt32  MaxNotificationsPerPublish);
			// PublishingEnabled
			OpcUa_Boolean GetPublishingEnabled();
			void SetPublishingEnabled(OpcUa_Boolean PublishingEnabled);
			// ModifyCount
			OpcUa_UInt32  GetModifyCount();
			void SetModifyCount(OpcUa_UInt32  ModifyCount);
			// EnableCount
			OpcUa_UInt32  GetEnableCount();
			void SetEnableCount(OpcUa_UInt32  EnableCount);
			// DisableCount
			OpcUa_UInt32  GetDisableCount();
			void SetDisableCount(OpcUa_UInt32  DisableCount);
			// RepublishRequestCount
			OpcUa_UInt32  GetRepublishRequestCount();
			void SetRepublishRequestCount(OpcUa_UInt32  RepublishRequestCount);
			// RepublishMessageRequestCount
			OpcUa_UInt32  GetRepublishMessageRequestCount();
			void SetRepublishMessageRequestCount(OpcUa_UInt32  RepublishMessageRequestCount); 
			// RepublishMessageCount
			OpcUa_UInt32  GetRepublishMessageCount();
			void SetRepublishMessageCount(OpcUa_UInt32  RepublishMessageCount);
			//TransferRequestCount
			OpcUa_UInt32  GetTransferRequestCount();
			void SetTransferRequestCount(OpcUa_UInt32  TransferRequestCount);
			// TransferredToAltClientCount
			OpcUa_UInt32  GetTransferredToAltClientCount();
			void SetTransferredToAltClientCount(OpcUa_UInt32  TransferredToAltClientCount);
			//TransferredToSameClientCount
			OpcUa_UInt32  GetTransferredToSameClientCount();
			void SetTransferredToSameClientCount(OpcUa_UInt32  TransferredToSameClientCount);
			//
			OpcUa_UInt32  GetPublishRequestCount();
			void SetPublishRequestCount(OpcUa_UInt32  PublishRequestCount);
			//
			OpcUa_UInt32  GetDataChangeNotificationsCount();
			void SetDataChangeNotificationsCount(OpcUa_UInt32  DataChangeNotificationsCount);
			//
			OpcUa_UInt32  GetEventNotificationsCount();
			void SetEventNotificationsCount(OpcUa_UInt32  EventNotificationsCount);
			//
			OpcUa_UInt32  GetNotificationsCount();
			void SetNotificationsCount(OpcUa_UInt32  NotificationsCount);
			//
			OpcUa_UInt32  GetLatePublishRequestCount();
			void SetLatePublishRequestCount(OpcUa_UInt32  LatePublishRequestCount);
			//
			OpcUa_UInt32  GetCurrentKeepAliveCount();
			void SetCurrentKeepAliveCount(OpcUa_UInt32  CurrentKeepAliveCount);
			//
			OpcUa_UInt32  GetCurrentLifetimeCount();
			void SetCurrentLifetimeCount(OpcUa_UInt32  CurrentLifetimeCount);
			//
			OpcUa_UInt32  GetUnacknowledgedMessageCount();
			void SetUnacknowledgedMessageCount(OpcUa_UInt32  UnacknowledgedMessageCount);
			//
			OpcUa_UInt32  GetDiscardedMessageCount();
			void SetDiscardedMessageCount(OpcUa_UInt32  DiscardedMessageCount);
			//
			OpcUa_UInt32  GetMonitoredItemCount();
			void SetMonitoredItemCount(OpcUa_UInt32  MonitoredItemCount);
			//
			OpcUa_UInt32  GetDisabledMonitoredItemCount();
			void SetDisabledMonitoredItemCount(OpcUa_UInt32  DisabledMonitoredItemCount);
			//
			OpcUa_UInt32  GetMonitoringQueueOverflowCount();
			void SetMonitoringQueueOverflowCount(OpcUa_UInt32  MonitoringQueueOverflowCount);
			//
			OpcUa_UInt32  GetNextSequenceNumber();
			void SetNextSequenceNumber(OpcUa_UInt32  NextSequenceNumber);
			//
			OpcUa_UInt32  GetEventQueueOverFlowCount();
			void SetEventQueueOverFlowCount(OpcUa_UInt32  EventQueueOverFlowCount);


		private:
			OpcUa_SubscriptionDiagnosticsDataType* m_pInternalSubscriptionDiagnosticsDataType; // from the stack
		};
		typedef std::vector<CSubscriptionDiagnosticsDataType*> CSubscriptionDiagnosticsDataTypeList;
	}
}