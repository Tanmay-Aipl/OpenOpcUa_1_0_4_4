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
#include "stdafx.h"
#include "SubscriptionDiagnosticsDataType.h"
using namespace OpenOpcUa;
using namespace UASharedLib;
CSubscriptionDiagnosticsDataType::CSubscriptionDiagnosticsDataType(void)
{
	m_pInternalSubscriptionDiagnosticsDataType=(OpcUa_SubscriptionDiagnosticsDataType*)OpcUa_Alloc(sizeof(OpcUa_SubscriptionDiagnosticsDataType));
	OpcUa_SubscriptionDiagnosticsDataType_Initialize(m_pInternalSubscriptionDiagnosticsDataType);
}
CSubscriptionDiagnosticsDataType::CSubscriptionDiagnosticsDataType(OpcUa_SubscriptionDiagnosticsDataType* pSubscriptionDiagnosticsDataType)
{
	if (pSubscriptionDiagnosticsDataType)
		m_pInternalSubscriptionDiagnosticsDataType = Utils::Copy(pSubscriptionDiagnosticsDataType);
	else
		m_pInternalSubscriptionDiagnosticsDataType = OpcUa_Null;
}
CSubscriptionDiagnosticsDataType::~CSubscriptionDiagnosticsDataType(void)
{
	if (m_pInternalSubscriptionDiagnosticsDataType)
	{
		OpcUa_SubscriptionDiagnosticsDataType_Clear(m_pInternalSubscriptionDiagnosticsDataType);
		OpcUa_Free(m_pInternalSubscriptionDiagnosticsDataType);
		m_pInternalSubscriptionDiagnosticsDataType = OpcUa_Null;
	}
}
OpcUa_NodeId  CSubscriptionDiagnosticsDataType::GetSessionId() 
{ 
	return m_pInternalSubscriptionDiagnosticsDataType->SessionId; 
}
void CSubscriptionDiagnosticsDataType::SetSessionId(OpcUa_NodeId aNodeId) 
{ 
	m_pInternalSubscriptionDiagnosticsDataType->SessionId = aNodeId; 
}

OpcUa_SubscriptionDiagnosticsDataType* CSubscriptionDiagnosticsDataType::GetInternalSubscriptionDiagnosticsDataType()
{
	return m_pInternalSubscriptionDiagnosticsDataType;
}
void CSubscriptionDiagnosticsDataType::SetInternalSubscriptionDiagnosticsDataType(OpcUa_SubscriptionDiagnosticsDataType* pInternalSubscriptionDiagnosticsDataType)
{
	m_pInternalSubscriptionDiagnosticsDataType = pInternalSubscriptionDiagnosticsDataType;
}

// SubscriptionId
OpcUa_UInt32  CSubscriptionDiagnosticsDataType::GetSubscriptionId()  { return m_pInternalSubscriptionDiagnosticsDataType->SubscriptionId; }
void CSubscriptionDiagnosticsDataType::SetSubscriptionId(OpcUa_UInt32  uiSubscriptionId) { m_pInternalSubscriptionDiagnosticsDataType->SubscriptionId = uiSubscriptionId; }
// Priority
OpcUa_Byte    CSubscriptionDiagnosticsDataType::GetPriority()  { return m_pInternalSubscriptionDiagnosticsDataType->Priority; }
void CSubscriptionDiagnosticsDataType::SetPriority(OpcUa_Byte    Priority) { m_pInternalSubscriptionDiagnosticsDataType->Priority = Priority; }
// PublishingInterval
OpcUa_Double  CSubscriptionDiagnosticsDataType::GetPublishingInterval()  { return m_pInternalSubscriptionDiagnosticsDataType->PublishingInterval; }
void CSubscriptionDiagnosticsDataType::SetPublishingInterval(OpcUa_Double  PublishingInterval) { m_pInternalSubscriptionDiagnosticsDataType->PublishingInterval = PublishingInterval; }
// MaxKeepAliveCount
OpcUa_UInt32  CSubscriptionDiagnosticsDataType::GetMaxKeepAliveCount()  { return m_pInternalSubscriptionDiagnosticsDataType->MaxKeepAliveCount; }
void CSubscriptionDiagnosticsDataType::SetMaxKeepAliveCount(OpcUa_UInt32  MaxKeepAliveCount) { m_pInternalSubscriptionDiagnosticsDataType->MaxKeepAliveCount = MaxKeepAliveCount; }
// MaxLifetimeCount
OpcUa_UInt32  CSubscriptionDiagnosticsDataType::GetMaxLifetimeCount()  { return m_pInternalSubscriptionDiagnosticsDataType->MaxLifetimeCount; }
void CSubscriptionDiagnosticsDataType::SetMaxLifetimeCount(OpcUa_UInt32  MaxLifetimeCount) { m_pInternalSubscriptionDiagnosticsDataType->MaxLifetimeCount = MaxLifetimeCount; }
// MaxNotificationsPerPublish
OpcUa_UInt32  CSubscriptionDiagnosticsDataType::GetMaxNotificationsPerPublish()  { return m_pInternalSubscriptionDiagnosticsDataType->MaxNotificationsPerPublish; }
void CSubscriptionDiagnosticsDataType::SetMaxNotificationsPerPublish(OpcUa_UInt32  MaxNotificationsPerPublish) { m_pInternalSubscriptionDiagnosticsDataType->MaxNotificationsPerPublish = MaxNotificationsPerPublish; }
// PublishingEnabled
OpcUa_Boolean CSubscriptionDiagnosticsDataType::GetPublishingEnabled()  { return m_pInternalSubscriptionDiagnosticsDataType->PublishingEnabled; }
void CSubscriptionDiagnosticsDataType::SetPublishingEnabled(OpcUa_Boolean PublishingEnabled) { m_pInternalSubscriptionDiagnosticsDataType->PublishingEnabled = PublishingEnabled; }
// ModifyCount
OpcUa_UInt32  CSubscriptionDiagnosticsDataType::GetModifyCount()  { return m_pInternalSubscriptionDiagnosticsDataType->ModifyCount; }
void CSubscriptionDiagnosticsDataType::SetModifyCount(OpcUa_UInt32  ModifyCount) { m_pInternalSubscriptionDiagnosticsDataType->ModifyCount = ModifyCount; }
// EnableCount
OpcUa_UInt32  CSubscriptionDiagnosticsDataType::GetEnableCount()  { return m_pInternalSubscriptionDiagnosticsDataType->EnableCount; }
void CSubscriptionDiagnosticsDataType::SetEnableCount(OpcUa_UInt32  EnableCount) { m_pInternalSubscriptionDiagnosticsDataType->EnableCount = EnableCount; }
// DisableCount
OpcUa_UInt32  CSubscriptionDiagnosticsDataType::GetDisableCount() { return m_pInternalSubscriptionDiagnosticsDataType->DisableCount; }
void CSubscriptionDiagnosticsDataType::SetDisableCount(OpcUa_UInt32  DisableCount) { m_pInternalSubscriptionDiagnosticsDataType->DisableCount = DisableCount; }
// RepublishRequestCount
OpcUa_UInt32  CSubscriptionDiagnosticsDataType::GetRepublishRequestCount()  { return m_pInternalSubscriptionDiagnosticsDataType->RepublishRequestCount; }
void CSubscriptionDiagnosticsDataType::SetRepublishRequestCount(OpcUa_UInt32  RepublishRequestCount) { m_pInternalSubscriptionDiagnosticsDataType->RepublishRequestCount = RepublishRequestCount; }
// RepublishMessageRequestCount
OpcUa_UInt32  CSubscriptionDiagnosticsDataType::GetRepublishMessageRequestCount()  { return m_pInternalSubscriptionDiagnosticsDataType->RepublishMessageRequestCount; }
void CSubscriptionDiagnosticsDataType::SetRepublishMessageRequestCount(OpcUa_UInt32  RepublishMessageRequestCount) { m_pInternalSubscriptionDiagnosticsDataType->RepublishMessageRequestCount = RepublishMessageRequestCount; }
// RepublishMessageCount
OpcUa_UInt32  CSubscriptionDiagnosticsDataType::GetRepublishMessageCount()  { return m_pInternalSubscriptionDiagnosticsDataType->RepublishMessageCount; }
void CSubscriptionDiagnosticsDataType::SetRepublishMessageCount(OpcUa_UInt32  RepublishMessageCount) { m_pInternalSubscriptionDiagnosticsDataType->RepublishMessageCount = RepublishMessageCount; }
//TransferRequestCount
OpcUa_UInt32  CSubscriptionDiagnosticsDataType::GetTransferRequestCount()  { return m_pInternalSubscriptionDiagnosticsDataType->TransferRequestCount; }
void CSubscriptionDiagnosticsDataType::SetTransferRequestCount(OpcUa_UInt32  TransferRequestCount) { m_pInternalSubscriptionDiagnosticsDataType->TransferRequestCount = TransferRequestCount; }
// TransferredToAltClientCount
OpcUa_UInt32  CSubscriptionDiagnosticsDataType::GetTransferredToAltClientCount()  { return m_pInternalSubscriptionDiagnosticsDataType->TransferredToAltClientCount; }
void CSubscriptionDiagnosticsDataType::SetTransferredToAltClientCount(OpcUa_UInt32  TransferredToAltClientCount) { m_pInternalSubscriptionDiagnosticsDataType->TransferredToAltClientCount = TransferredToAltClientCount; }
//TransferredToSameClientCount
OpcUa_UInt32  CSubscriptionDiagnosticsDataType::GetTransferredToSameClientCount()  { return m_pInternalSubscriptionDiagnosticsDataType->TransferredToSameClientCount; }
void CSubscriptionDiagnosticsDataType::SetTransferredToSameClientCount(OpcUa_UInt32  TransferredToSameClientCount) { m_pInternalSubscriptionDiagnosticsDataType->TransferredToSameClientCount = TransferredToSameClientCount; }
//
OpcUa_UInt32  CSubscriptionDiagnosticsDataType::GetPublishRequestCount()  { return m_pInternalSubscriptionDiagnosticsDataType->PublishRequestCount; }
void CSubscriptionDiagnosticsDataType::SetPublishRequestCount(OpcUa_UInt32  PublishRequestCount) { m_pInternalSubscriptionDiagnosticsDataType->PublishRequestCount = PublishRequestCount; }
//
OpcUa_UInt32  CSubscriptionDiagnosticsDataType::GetDataChangeNotificationsCount()  { return m_pInternalSubscriptionDiagnosticsDataType->DataChangeNotificationsCount; }
void CSubscriptionDiagnosticsDataType::SetDataChangeNotificationsCount(OpcUa_UInt32  DataChangeNotificationsCount) { m_pInternalSubscriptionDiagnosticsDataType->DataChangeNotificationsCount = DataChangeNotificationsCount; }
//
OpcUa_UInt32  CSubscriptionDiagnosticsDataType::GetEventNotificationsCount()  { return m_pInternalSubscriptionDiagnosticsDataType->EventNotificationsCount; }
void CSubscriptionDiagnosticsDataType::SetEventNotificationsCount(OpcUa_UInt32  EventNotificationsCount) { m_pInternalSubscriptionDiagnosticsDataType->EventNotificationsCount = EventNotificationsCount; }
//
OpcUa_UInt32  CSubscriptionDiagnosticsDataType::GetNotificationsCount()  { return m_pInternalSubscriptionDiagnosticsDataType->NotificationsCount; }
void CSubscriptionDiagnosticsDataType::SetNotificationsCount(OpcUa_UInt32  NotificationsCount) { m_pInternalSubscriptionDiagnosticsDataType->NotificationsCount = NotificationsCount; }
//
OpcUa_UInt32  CSubscriptionDiagnosticsDataType::GetLatePublishRequestCount()  { return m_pInternalSubscriptionDiagnosticsDataType->LatePublishRequestCount; }
void CSubscriptionDiagnosticsDataType::SetLatePublishRequestCount(OpcUa_UInt32  LatePublishRequestCount) { m_pInternalSubscriptionDiagnosticsDataType->LatePublishRequestCount = LatePublishRequestCount; }
//
OpcUa_UInt32  CSubscriptionDiagnosticsDataType::GetCurrentKeepAliveCount()  { return m_pInternalSubscriptionDiagnosticsDataType->CurrentKeepAliveCount; }
void CSubscriptionDiagnosticsDataType::SetCurrentKeepAliveCount(OpcUa_UInt32  CurrentKeepAliveCount) { m_pInternalSubscriptionDiagnosticsDataType->CurrentKeepAliveCount = CurrentKeepAliveCount; }
//
OpcUa_UInt32  CSubscriptionDiagnosticsDataType::GetCurrentLifetimeCount()  { return m_pInternalSubscriptionDiagnosticsDataType->CurrentLifetimeCount; }
void CSubscriptionDiagnosticsDataType::SetCurrentLifetimeCount(OpcUa_UInt32  CurrentLifetimeCount) { m_pInternalSubscriptionDiagnosticsDataType->CurrentLifetimeCount = CurrentLifetimeCount; }
//
OpcUa_UInt32  CSubscriptionDiagnosticsDataType::GetUnacknowledgedMessageCount()  { return m_pInternalSubscriptionDiagnosticsDataType->UnacknowledgedMessageCount; }
void CSubscriptionDiagnosticsDataType::SetUnacknowledgedMessageCount(OpcUa_UInt32  UnacknowledgedMessageCount) { m_pInternalSubscriptionDiagnosticsDataType->UnacknowledgedMessageCount = UnacknowledgedMessageCount; }
//
OpcUa_UInt32  CSubscriptionDiagnosticsDataType::GetDiscardedMessageCount()  { return m_pInternalSubscriptionDiagnosticsDataType->DiscardedMessageCount; }
void CSubscriptionDiagnosticsDataType::SetDiscardedMessageCount(OpcUa_UInt32  DiscardedMessageCount) { m_pInternalSubscriptionDiagnosticsDataType->DiscardedMessageCount = DiscardedMessageCount; }
//
OpcUa_UInt32  CSubscriptionDiagnosticsDataType::GetMonitoredItemCount()  { return m_pInternalSubscriptionDiagnosticsDataType->MonitoredItemCount; }
void CSubscriptionDiagnosticsDataType::SetMonitoredItemCount(OpcUa_UInt32  MonitoredItemCount) { m_pInternalSubscriptionDiagnosticsDataType->MonitoredItemCount = MonitoredItemCount; }
//
OpcUa_UInt32  CSubscriptionDiagnosticsDataType::GetDisabledMonitoredItemCount()  { return m_pInternalSubscriptionDiagnosticsDataType->DisabledMonitoredItemCount; }
void CSubscriptionDiagnosticsDataType::SetDisabledMonitoredItemCount(OpcUa_UInt32  DisabledMonitoredItemCount) { m_pInternalSubscriptionDiagnosticsDataType->DisabledMonitoredItemCount = DisabledMonitoredItemCount; }
//
OpcUa_UInt32  CSubscriptionDiagnosticsDataType::GetMonitoringQueueOverflowCount()  { return m_pInternalSubscriptionDiagnosticsDataType->MonitoringQueueOverflowCount; }
void CSubscriptionDiagnosticsDataType::SetMonitoringQueueOverflowCount(OpcUa_UInt32  MonitoringQueueOverflowCount) { m_pInternalSubscriptionDiagnosticsDataType->MonitoringQueueOverflowCount = MonitoringQueueOverflowCount; }
//
OpcUa_UInt32  CSubscriptionDiagnosticsDataType::GetNextSequenceNumber()  { return m_pInternalSubscriptionDiagnosticsDataType->NextSequenceNumber; }
void CSubscriptionDiagnosticsDataType::SetNextSequenceNumber(OpcUa_UInt32  NextSequenceNumber) { m_pInternalSubscriptionDiagnosticsDataType->NextSequenceNumber = NextSequenceNumber; }
//
OpcUa_UInt32  CSubscriptionDiagnosticsDataType::GetEventQueueOverFlowCount()  { return m_pInternalSubscriptionDiagnosticsDataType->EventQueueOverFlowCount; }
void CSubscriptionDiagnosticsDataType::SetEventQueueOverFlowCount(OpcUa_UInt32  EventQueueOverFlowCount) { m_pInternalSubscriptionDiagnosticsDataType->EventQueueOverFlowCount = EventQueueOverFlowCount; }