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
//#include "MonitoredItemServer.h"
/////////////////////////////////////////////////////////////
#include "UAVariable.h"
#include "VpiFuncCaller.h"
#include "VpiTag.h"
#include "VpiWriteObject.h"
#include "VpiDevice.h"
#include "UAReferenceType.h"
#include "UAObjectType.h"
#include "Field.h"
#include "Definition.h"
#include "UADataType.h"
#include "UAVariableType.h"
#include "UAMethod.h"
#include "UAView.h"
#include "UAObject.h"
#include "Alias.h"
#include "SimulatedNode.h"
#include "SimulatedGroup.h"
#include "BuildInfo.h"
#include "ServerStatus.h"
#include "VPIScheduler.h"
#include "NamespaceUri.h"
#include "StatusCodeException.h"
#include "ContinuationPoint.h"
#include "SessionSecurityDiagnosticsDataType.h"
#include "SessionDiagnosticsDataType.h"
#include "luainc.h"
#include "LuaVirtualMachine.h"
#include "LuaScript.h"
#include "OpenOpcUaScript.h"
using namespace UAScript;

#include "UAMonitoredItemNotification.h"
#include "UADataChangeNotification.h"
/////////////////////////////////////////////////////////////
#include "EventDefinition.h"
using namespace UAEvents;
#include "UAInformationModel.h"
#include "MonitoredItemServer.h"
#include "UAEventNotificationList.h"
using namespace OpenOpcUa;
using namespace UACoreServer;
using namespace UASharedLib;
CUAEventNotificationList::CUAEventNotificationList(void)
{
	m_pInternalEventNotificationList=OpcUa_Null;
	m_pMonitoredItemServer = OpcUa_Null;
	m_eTransactionStatus = OPCUA_EVENTSTATUS_UNKNOWN;
	m_bAlarmAcked = OpcUa_False;
	m_uiSequenceNumber = 1;
	m_bTerminated = OpcUa_False;
	m_bConfirmed = OpcUa_False;
	m_bLastNotifiedActiveState = OpcUa_False;
	m_bLastNotifiedAlarmAcked = OpcUa_False;
	m_bLastNotifiedConfirmed = OpcUa_False;
	m_pRelatedEventDefinition = OpcUa_Null;
	m_FastEventFieldIndex.iDateTime = -1;
	m_FastEventFieldIndex.iMessage = -1;
	m_FastEventFieldIndex.iAckedState = -1;
	m_FastEventFieldIndex.iAckedStateId = -1;
	m_FastEventFieldIndex.iActiveState = -1;
	m_FastEventFieldIndex.iActiveStateId = -1;
	m_FastEventFieldIndex.iConfirmedState = -1;
	m_FastEventFieldIndex.iConfirmedStateId = -1;
	m_FastEventFieldIndex.iSystemStateIndex = -1;
	m_FastEventFieldIndex.iActiveStateEffectiveDisplayName =-1;
	m_FastEventFieldIndex.iRetain =-1;
	m_FastEventFieldIndex.iComment = -1;
	// Let's initialize the internal
	m_pInternalEventNotificationList = (OpcUa_EventNotificationList*)OpcUa_Alloc(sizeof(OpcUa_EventNotificationList));
	OpcUa_EventNotificationList_Initialize(m_pInternalEventNotificationList);
	m_pInternalEventNotificationList->NoOfEvents = 1;
	m_pInternalEventNotificationList->Events = (OpcUa_EventFieldList*)OpcUa_Alloc(sizeof(OpcUa_EventFieldList));
	OpcUa_EventFieldList_Initialize(&m_pInternalEventNotificationList->Events[0]);// only one EventFieldList per EventNotificationList
}
CUAEventNotificationList::CUAEventNotificationList(OpcUa_UInt32 uiSequenceNumber) :m_uiSequenceNumber(uiSequenceNumber)
{
	m_pInternalEventNotificationList = OpcUa_Null;
	m_pMonitoredItemServer = OpcUa_Null;
	m_eTransactionStatus = OPCUA_EVENTSTATUS_UNKNOWN;
	m_bAlarmAcked = OpcUa_False;
	m_uiSequenceNumber = 1;
	m_bTerminated = OpcUa_False;
	m_bConfirmed = OpcUa_False;
	m_bLastNotifiedActiveState = OpcUa_False;
	m_bLastNotifiedAlarmAcked = OpcUa_False;
	m_bLastNotifiedConfirmed = OpcUa_False;
	m_pRelatedEventDefinition = OpcUa_Null;
	m_FastEventFieldIndex.iDateTime = -1;
	m_FastEventFieldIndex.iMessage = -1;
	m_FastEventFieldIndex.iAckedState = -1;
	m_FastEventFieldIndex.iAckedStateId = -1;
	m_FastEventFieldIndex.iActiveState = -1;
	m_FastEventFieldIndex.iActiveStateId = -1;
	m_FastEventFieldIndex.iConfirmedState = -1;
	m_FastEventFieldIndex.iConfirmedStateId = -1;
	m_FastEventFieldIndex.iSystemStateIndex = -1;
	m_FastEventFieldIndex.iActiveStateEffectiveDisplayName = -1;
	m_FastEventFieldIndex.iRetain = -1;
	m_FastEventFieldIndex.iComment = -1;
	// Let's initialize the internal
	m_pInternalEventNotificationList = (OpcUa_EventNotificationList*)OpcUa_Alloc(sizeof(OpcUa_EventNotificationList));
	OpcUa_EventNotificationList_Initialize(m_pInternalEventNotificationList);
	m_pInternalEventNotificationList->NoOfEvents = 1;
	m_pInternalEventNotificationList->Events = (OpcUa_EventFieldList*)OpcUa_Alloc(sizeof(OpcUa_EventFieldList));
	OpcUa_EventFieldList_Initialize(&m_pInternalEventNotificationList->Events[0]);// only one EventFieldList per EventNotificationList
}
CUAEventNotificationList::~CUAEventNotificationList(void)
{
	if (m_pInternalEventNotificationList)
	{
		//if (m_pInternalEventNotificationList->Events)
		//{
		//	OpcUa_EventFieldList_Clear(&m_pInternalEventNotificationList->Events[0]); // only one EventFieldList per EventNotificationList
		//	OpcUa_Free(m_pInternalEventNotificationList->Events);
		//}
		OpcUa_EventNotificationList_Clear(m_pInternalEventNotificationList);
		OpcUa_Free(m_pInternalEventNotificationList);
	}
	// On ne supprime que les EventDefinition dynamique.
	// c'est a dire celle qui n'ont pas de definition dans le NodeSet
	if (m_pRelatedEventDefinition)
	{
		if ((m_pRelatedEventDefinition->GetConditionNode() == OpcUa_Null) && (m_pRelatedEventDefinition->GetSourceNode()==OpcUa_Null) )
			delete m_pRelatedEventDefinition;
	}
}
// Renvoi le nombre d'events dans le OpcUa_EventNotificationList encapsulé
OpcUa_Int32 CUAEventNotificationList::GetNoOfEvents()
{
	OpcUa_Int32 iSize;
	if (m_pInternalEventNotificationList)
		iSize = m_pInternalEventNotificationList->NoOfEvents;
	else
		iSize = 0;
	return iSize;
}
OpcUa_EventFieldList* CUAEventNotificationList::GetEventField(OpcUa_Int32 index)
{
	OpcUa_EventFieldList* pEventFieldList = OpcUa_Null;
	if (m_pInternalEventNotificationList)
		OpcUa_MemCpy(pEventFieldList, sizeof(OpcUa_EventFieldList),&m_pInternalEventNotificationList->Events[index], sizeof(OpcUa_EventFieldList));
	return pEventFieldList;
}
OpcUa_UInt32 CUAEventNotificationList::GetSequenceNumber() 
{ 
	return m_uiSequenceNumber; 
}
void CUAEventNotificationList::SetSequenceNumber(OpcUa_UInt32 uiVal) 
{ 
	m_uiSequenceNumber=uiVal; 
}
OpcUa_Boolean CUAEventNotificationList::IsTransactionAcked()
{ 
	if (m_eTransactionStatus == OPCUA_EVENTSTATUS_ACKEDBYCLIENT)
		return OpcUa_True;
	else
		return OpcUa_False;
}
void CUAEventNotificationList::TransactionAcked(OpcUa_Boolean bVal)
{
	if (bVal)
		m_eTransactionStatus = OPCUA_EVENTSTATUS_ACKEDBYCLIENT;
	else
		m_eTransactionStatus = OPCUA_EVENTSTATUS_UNKNOWN;
}
OpcUa_Boolean CUAEventNotificationList::IsAlarmAcked()
{
	return m_bAlarmAcked;
}
void CUAEventNotificationList::AlarmAcked()
{
	m_bAlarmAcked= OpcUa_True;
}
OpcUa_Boolean CUAEventNotificationList::IsKeepAlive()
{
	if (m_NotificationDataType == NOTIFICATION_MESSAGE_KEEPALIVE)
		return OpcUa_True;
	else
		return OpcUa_False;
}
NotificationMessageType CUAEventNotificationList::GetNotificationType()
{ 
	return m_NotificationDataType; 
}
void CUAEventNotificationList::SetNotificationType(NotificationMessageType aDataChangeNotificationType)
{ 
	m_NotificationDataType = aDataChangeNotificationType; 
}
CMonitoredItemServer* CUAEventNotificationList::GetMonitoredItem()
{
	return m_pMonitoredItemServer;
}
void CUAEventNotificationList::SetMonitoredItem(CMonitoredItemServer* pMonitoredItem)
{
	if (pMonitoredItem)
	{
		m_pMonitoredItemServer = pMonitoredItem;
		if (m_pInternalEventNotificationList)
			m_pInternalEventNotificationList->Events[0].ClientHandle = pMonitoredItem->GetClientHandle();
	}
}
/// <summary>
/// Updates the EventsField already sent to the client (m_pInternalEventNotificationList::Events)
/// The updated field are :
/// AckedState : m_pInternalEventNotificationList::Events[0].EventFields[m_FastEventFieldIndex.iAckedState].
/// AckedStateId: m_pInternalEventNotificationList::Events[0].EventFields[m_FastEventFieldIndex.iAckedStateId]
/// </summary>
/// <returns></returns>
OpcUa_StatusCode CUAEventNotificationList::UpdateInternalAckedState(OpcUa_Boolean bNewState)
{
	OpcUa_StatusCode uStatus= OpcUa_Good;
	
	OpcUa_LocalizedText AckedState;
	OpcUa_LocalizedText_Initialize(&AckedState);
	if (bNewState)
	{
		OpcUa_String_AttachCopy(&(AckedState.Locale), "en-us");
		OpcUa_String_AttachCopy(&(AckedState.Text), "Acknowledged");
	}
	else
	{
		OpcUa_String_AttachCopy(&(AckedState.Locale), "en-us");
		OpcUa_String_AttachCopy(&(AckedState.Text), "Unacknowledged");
	}
	// We will update both AckedState and AckedStateId
	// AckedState
	if (m_FastEventFieldIndex.iAckedState != -1)
		OpcUa_LocalizedText_CopyTo(&AckedState,
			m_pInternalEventNotificationList->Events[0].EventFields[m_FastEventFieldIndex.iAckedState].Value.LocalizedText);
	// AckedStateId
	if (m_FastEventFieldIndex.iAckedStateId != -1)
		m_pInternalEventNotificationList->Events[0].EventFields[m_FastEventFieldIndex.iAckedStateId].Value.Boolean = bNewState;
	OpcUa_LocalizedText_Clear(&AckedState);
	return uStatus;
}
/// <summary>
/// Updates the ConfirmedState. The confirmedState is a TwoStateVariableType
/// We will update just the madatory attribute
/// </summary>
/// <param name="bNewState">New state of the b.</param>
/// <returns></returns>
OpcUa_StatusCode CUAEventNotificationList::UpdateInternalConfirmedState(OpcUa_Boolean bNewState)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;

	OpcUa_LocalizedText ConfirmedState;
	OpcUa_LocalizedText_Initialize(&ConfirmedState);
	if (bNewState)
	{
		OpcUa_String_AttachCopy(&(ConfirmedState.Locale), "en-us");
		OpcUa_String_AttachCopy(&(ConfirmedState.Text), "Confirmed");
	}
	else
	{
		OpcUa_String_AttachCopy(&(ConfirmedState.Locale), "en-us");
		OpcUa_String_AttachCopy(&(ConfirmedState.Text), "Unconfirmed");
	}
	// We will update both ConfirmedState and ConfirmedStateId
	// ConfirmedState
	if (m_FastEventFieldIndex.iConfirmedState != -1)
		OpcUa_LocalizedText_CopyTo(&ConfirmedState,
		m_pInternalEventNotificationList->Events[0].EventFields[m_FastEventFieldIndex.iConfirmedState].Value.LocalizedText);

	// ConfirmedStateId
	if (m_FastEventFieldIndex.iConfirmedStateId != -1)
		m_pInternalEventNotificationList->Events[0].EventFields[m_FastEventFieldIndex.iConfirmedStateId].Value.Boolean = bNewState;
	OpcUa_LocalizedText_Clear(&ConfirmedState);
	return uStatus;
}
OpcUa_StatusCode CUAEventNotificationList::UpdateInternalActiveState(OpcUa_Boolean bNewState)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;

	OpcUa_LocalizedText ActiveState;
	OpcUa_LocalizedText_Initialize(&ActiveState);
	if (bNewState)
	{ 
		OpcUa_String_AttachCopy(&(ActiveState.Locale), "en-us");
		OpcUa_String_AttachCopy(&(ActiveState.Text), "Active");
	}
	else
	{
		OpcUa_String_AttachCopy(&(ActiveState.Locale), "en-us");
		OpcUa_String_AttachCopy(&(ActiveState.Text), "Inactive");
	}
	// We will update both ActiveState and ActiveStateId
	// ActiveState
	if (m_pInternalEventNotificationList->Events[0].NoOfEventFields > m_FastEventFieldIndex.iActiveState)
	{
		if (m_FastEventFieldIndex.iActiveState != -1)
			OpcUa_LocalizedText_CopyTo(&ActiveState,
			m_pInternalEventNotificationList->Events[0].EventFields[m_FastEventFieldIndex.iActiveState].Value.LocalizedText);
	}
	else
	{
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "UpdateInternalActiveState is in error. Invalid index, Max is %d iActiveState is %d", 
									m_pInternalEventNotificationList->Events[0].NoOfEventFields, 
									m_FastEventFieldIndex.iActiveState);
		uStatus = OpcUa_BadIndexRangeInvalid;
	}
	// ActiveState EffectiveDisplayName
	if (m_pInternalEventNotificationList->Events[0].NoOfEventFields > m_FastEventFieldIndex.iActiveStateEffectiveDisplayName)
	{	
		if (m_FastEventFieldIndex.iActiveStateEffectiveDisplayName != -1)
			OpcUa_LocalizedText_CopyTo(&ActiveState,
				m_pInternalEventNotificationList->Events[0].EventFields[m_FastEventFieldIndex.iActiveStateEffectiveDisplayName].Value.LocalizedText);
	}
	else
	{
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "UpdateInternalActiveState is in error. Invalid index, Max is %d iActiveStateEffectiveDisplayName is %d", 
									m_pInternalEventNotificationList->Events[0].NoOfEventFields, 
									m_FastEventFieldIndex.iActiveStateEffectiveDisplayName);
		uStatus = OpcUa_BadIndexRangeInvalid;
	}
	// ActiveStateId
	if (m_pInternalEventNotificationList->Events[0].NoOfEventFields > m_FastEventFieldIndex.iActiveStateId)
	{
		if (m_FastEventFieldIndex.iActiveStateId != -1)
			m_pInternalEventNotificationList->Events[0].EventFields[m_FastEventFieldIndex.iActiveStateId].Value.Boolean = bNewState;
	}
	else
	{
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "UpdateInternalActiveState is in error. Invalid index, Max is %d iActiveStateId is %d", 
									m_pInternalEventNotificationList->Events[0].NoOfEventFields, 
									m_FastEventFieldIndex.iActiveStateId);
		uStatus = OpcUa_BadIndexRangeInvalid;
	}
	// Retain
	if (m_pInternalEventNotificationList->Events[0].NoOfEventFields > m_FastEventFieldIndex.iRetain)
	{
		if (m_FastEventFieldIndex.iRetain != -1)
			m_pInternalEventNotificationList->Events[0].EventFields[m_FastEventFieldIndex.iRetain].Value.Boolean = bNewState;
	}
	else
	{
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "UpdateInternalActiveState is in error. Invalid index, Max is %d iRetain is %d",
			m_pInternalEventNotificationList->Events[0].NoOfEventFields,
			m_FastEventFieldIndex.iRetain);
		uStatus = OpcUa_BadIndexRangeInvalid;
	}
	OpcUa_LocalizedText_Clear(&ActiveState);
	return uStatus;
}

FastEventFieldIndex CUAEventNotificationList::GetFastEventFieldIndex()
{
	return m_FastEventFieldIndex;
}
// Update the internal FastEventFieldIndex structure.
void CUAEventNotificationList::SetFastEventFieldIndex(FastEventFieldIndex *pFastEventFieldIndex)
{
	if (pFastEventFieldIndex)
	{
		m_FastEventFieldIndex.iDateTime = pFastEventFieldIndex->iDateTime;
		m_FastEventFieldIndex.iMessage = pFastEventFieldIndex->iMessage;
		m_FastEventFieldIndex.iAckedState =pFastEventFieldIndex->iAckedState;
		m_FastEventFieldIndex.iAckedStateId = pFastEventFieldIndex->iAckedStateId;
		m_FastEventFieldIndex.iActiveState = pFastEventFieldIndex->iActiveState;
		m_FastEventFieldIndex.iActiveStateId = pFastEventFieldIndex->iActiveStateId;
		m_FastEventFieldIndex.iConfirmedState = pFastEventFieldIndex->iConfirmedState;
		m_FastEventFieldIndex.iConfirmedStateId = pFastEventFieldIndex->iConfirmedStateId;
		m_FastEventFieldIndex.iActiveStateEffectiveDisplayName = pFastEventFieldIndex->iActiveStateEffectiveDisplayName;
		m_FastEventFieldIndex.iSystemStateIndex = pFastEventFieldIndex->iSystemStateIndex;
		m_FastEventFieldIndex.iRetain = pFastEventFieldIndex->iRetain;
		m_FastEventFieldIndex.iComment = pFastEventFieldIndex->iComment;
	}
}

OpcUa_Boolean CUAEventNotificationList::IsTerminated()
{
	return m_bTerminated;
}
void CUAEventNotificationList::Terminate()
{
	m_bTerminated = OpcUa_True;
}
OpcUa_StatusCode CUAEventNotificationList::AddComment(const OpcUa_LocalizedText comment)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	if (m_FastEventFieldIndex.iComment != -1)
		OpcUa_LocalizedText_CopyTo(&comment,
			m_pInternalEventNotificationList->Events[0].EventFields[m_FastEventFieldIndex.iComment].Value.LocalizedText);
	return uStatus;
}

OpcUa_Int32 CUAEventNotificationList::GetCurrentStateIndex() 
{ 
	return m_FastEventFieldIndex.iSystemStateIndex; 
}
// LastNotifiedActiveState
void CUAEventNotificationList::SetLastNotifiedActiveState(OpcUa_Boolean bVal)
{
	m_bLastNotifiedActiveState = bVal;
}
OpcUa_Boolean CUAEventNotificationList::GetLastNotifiedActiveState() 
{ 
	return m_bLastNotifiedActiveState; 
}
OpcUa_EventNotificationList* CUAEventNotificationList::GetInternalEventNotificationList() 
{ 
	return m_pInternalEventNotificationList; 
}
void CUAEventNotificationList::SetInternalEventNotificationList(OpcUa_EventNotificationList* pInternalEventNotificationList) 
{ 
	m_pInternalEventNotificationList = pInternalEventNotificationList; 
}
void CUAEventNotificationList::SetEventDefinition(CEventDefinition* pRelatedEventDefinition) 
{ 
	m_pRelatedEventDefinition = pRelatedEventDefinition; 
}
CEventDefinition* CUAEventNotificationList::GetEventDefinition() 
{ 
	return m_pRelatedEventDefinition; 
}

OpcUa_Boolean CUAEventNotificationList::IsConfirmed() 
{ 
	return m_bConfirmed; 
}
void CUAEventNotificationList::Confirmed() 
{ 
	m_bConfirmed = OpcUa_True; 
}
OpcUa_Boolean CUAEventNotificationList::GetLastNotifiedConfirmed() 
{ 
	return m_bLastNotifiedConfirmed;
}
void CUAEventNotificationList::SetLastNotifiedConfirmed(OpcUa_Boolean bVal) 
{ 
	m_bLastNotifiedConfirmed = bVal; 
}

void CUAEventNotificationList::SetLastNotifiedAlarmAcked(OpcUa_Boolean bVal)
{
	m_bLastNotifiedAlarmAcked = bVal;
}
OpcUa_Boolean CUAEventNotificationList::GetLastNotifiedAlarmAcked() 
{ 
	return m_bLastNotifiedAlarmAcked; 
}

///-------------------------------------------------------------------------------------------------
/// <summary>
/// 	base on a couple of bool determine if the EventList need to be notified or if it was
/// 	already done. 
/// </summary>
///
/// <remarks>	Michel, 31/08/2016. </remarks>
///
/// <returns>	An OpcUa_Boolean. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_Boolean OpenOpcUa::UACoreServer::CUAEventNotificationList::IsEventListNotifiable(void)
{
	OpcUa_Boolean bResult = OpcUa_False;
	if(!(GetTransactionStatus() & OPCUA_EVENTSTATUS_SENDTOCLIENT)) // not send to client
	{
		if ((GetTransactionStatus() == OPCUA_EVENTSTATUS_TRANSITION) || (GetTransactionStatus()==OPCUA_EVENTSTATUS_UNKNOWN) )
		{
			if (m_bAlarmAcked != m_bLastNotifiedAlarmAcked)
				bResult = OpcUa_True;
			else
			{
				if (m_bConfirmed != m_bLastNotifiedConfirmed)
					bResult = OpcUa_True;
				else
				{
					if (m_pRelatedEventDefinition->GetActiveState() != m_bLastNotifiedActiveState)
						bResult = OpcUa_True;
					else
					{
						//if (GetTransactionStatus() != m_pRelatedEventDefinition->GetTransactionStatus())
							bResult = OpcUa_True;
					}
				}
			}
		}
	}
	return bResult;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Gets transaction status. </summary>
///
/// <remarks>	Michel, 05/09/2016. </remarks>
///
/// <returns>	The transaction status. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_UInt32 OpenOpcUa::UACoreServer::CUAEventNotificationList::GetTransactionStatus(void)
{
	return m_eTransactionStatus;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Sets transaction status. </summary>
///
/// <remarks>	Michel, 05/09/2016. </remarks>
///
/// <param name="eStatus">	The status. </param>
///-------------------------------------------------------------------------------------------------

void OpenOpcUa::UACoreServer::CUAEventNotificationList::SetTransactionStatus(OpcUa_UInt32 eStatus)
{
	m_eTransactionStatus = eStatus;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Updates the internal date time described by dtValue. </summary>
///
/// <remarks>	Michel, 06/09/2016. </remarks>
///
/// <param name="dtValue">	The dt value. </param>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode OpenOpcUa::UACoreServer::CUAEventNotificationList::UpdateInternalDateTime(OpcUa_DateTime dtValue)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	if (m_FastEventFieldIndex.iDateTime != -1)
		OpcUa_DateTime_CopyTo(&dtValue,
		&(m_pInternalEventNotificationList->Events[0].EventFields[m_FastEventFieldIndex.iDateTime].Value.DateTime));
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Updates the message send to the client for this instance. 
/// 			According to the placeHolder in specify in the message UAVariable declare in the NodeSet
/// 			The supported placeHolder are •	%SourceNode • %SourceNodeValue • %SourceName • %Time
/// 			PlaceHolders are lookup in the method GetValueForPlaceHolder
///				The message must finish with a space (0x20)
/// 			</summary>
///
/// <remarks>	Michel, 08/09/2016. </remarks>
///
/// <returns>	An OpcUa_StatusCode : OpcUa_Good, OpcUa_BadInvalidArgument. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode OpenOpcUa::UACoreServer::CUAEventNotificationList::UpdateInternalMessage(void)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	if (m_FastEventFieldIndex.iMessage != -1)
	{
		CUAVariable* pEventMessageVariable=m_pRelatedEventDefinition->GetEventMessage();
		if (pEventMessageVariable)
		{
			CDataValue *pDataValue=pEventMessageVariable->GetValue();
			if (pDataValue)
			{
				std::vector<OpcUa_String*> placeHolderList;
				OpcUa_LocalizedText* pszMessage=pDataValue->GetInternalDataValue()->Value.Value.LocalizedText;
				OpcUa_CharA* szPlaceHolder = (OpcUa_CharA*)OpcUa_Alloc(50);
				ZeroMemory(szPlaceHolder, 50);
				// Search for PlaceHolder
				OpcUa_UInt32 iLen = OpcUa_String_StrLen(&(pszMessage->Text));
				OpcUa_CharA* szRawMessage = OpcUa_String_GetRawString(&(pszMessage->Text));
				OpcUa_UInt32 ii = 0;
				OpcUa_Boolean bFoundPourcent=OpcUa_False;
				for (OpcUa_UInt32 i = 0; i < iLen; i++)
				{
					if (szRawMessage[i] == 0x25)
					{
						OpcUa_String* strPlaceHolder = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
						OpcUa_String_Initialize(strPlaceHolder);
						OpcUa_String_AttachCopy(strPlaceHolder, szPlaceHolder);
						bFoundPourcent = OpcUa_True;
						ii = 0;
						placeHolderList.push_back(strPlaceHolder);
						ZeroMemory(szPlaceHolder, 50);
					}
					else
					{
						if (bFoundPourcent)
						{
							if (szRawMessage[i] == 0x20)
							{
								ii=0;
								OpcUa_String strPlaceHolder;
								OpcUa_String_Initialize(&strPlaceHolder);
								OpcUa_String* strValue = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
								OpcUa_String_Initialize(strValue);
								OpcUa_String_AttachCopy(&strPlaceHolder, szPlaceHolder);
								if (GetValueForPlaceHolder(strPlaceHolder, strValue) == OpcUa_Good)
								{
									placeHolderList.push_back(strValue);
								}
								else
								{
									OpcUa_String_Clear(strValue);
									OpcUa_Free(strValue);
								}
								OpcUa_String_Clear(&strPlaceHolder);
								ZeroMemory(szPlaceHolder, 50);
								bFoundPourcent = OpcUa_False;
							}
							else
								szPlaceHolder[ii++] = szRawMessage[i];
						} 
						else
							szPlaceHolder[ii++] = szRawMessage[i];
					}
				}
				// Now according to the placeHolder and Fix Text we will create the full message
				if (placeHolderList.size() > 0)
				{
					OpcUa_LocalizedText szMessage;
					OpcUa_LocalizedText_Initialize(&szMessage);
					for (OpcUa_UInt32 iii = 0; iii < placeHolderList.size(); iii++)
					{
						OpcUa_String* strPlaceHolder = placeHolderList.at(iii);
						OpcUa_String_StrnCat(&szMessage.Text, strPlaceHolder, OpcUa_String_StrLen(strPlaceHolder));
					}
					if (OpcUa_String_StrLen(&szMessage.Text) > 0)
					{
						if (m_FastEventFieldIndex.iMessage != -1)
							OpcUa_LocalizedText_CopyTo(&szMessage,
							m_pInternalEventNotificationList->Events[0].EventFields[m_FastEventFieldIndex.iMessage].Value.LocalizedText);
					}
					OpcUa_LocalizedText_Clear(&szMessage);
				}
				else
					OpcUa_LocalizedText_CopyTo(pszMessage,
						m_pInternalEventNotificationList->Events[0].EventFields[m_FastEventFieldIndex.iMessage].Value.LocalizedText);
				//  Release ressources
				OpcUa_Free(szPlaceHolder);
				for (OpcUa_UInt32 iii = 0; iii < placeHolderList.size(); iii++)
				{
					OpcUa_String* strPlaceHolder=placeHolderList.at(iii);
					OpcUa_String_Clear(strPlaceHolder);
					OpcUa_Free(strPlaceHolder);
				}
			}
		}
		else
			uStatus = OpcUa_BadInvalidArgument;
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;	
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Gets value for place holder. </summary>
///
/// <remarks>	Michel, 08/09/2016. </remarks>
///
/// <param name="szPlaceHolder">	The place holder. </param>
/// <param name="pszValue">			[in,out] If non-null, the value. </param>
///
/// <returns>	The value for place holder. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode OpenOpcUa::UACoreServer::CUAEventNotificationList::GetValueForPlaceHolder(OpcUa_String szPlaceHolder, OpcUa_String* pszValue)
{
	OpcUa_StatusCode uStatus = OpcUa_BadNotFound;
	// SourceNode
	OpcUa_String szSourceNode;
	OpcUa_String_Initialize(&szSourceNode);
	OpcUa_String_AttachCopy(&szSourceNode, "SOURCENODE");
	// SourceNodeValue
	OpcUa_String szSourceNodeValue;
	OpcUa_String_Initialize(&szSourceNodeValue);
	OpcUa_String_AttachCopy(&szSourceNodeValue, "SOURCENODEVALUE");
	// SourceName
	OpcUa_String szSourceName;
	OpcUa_String_Initialize(&szSourceName);
	OpcUa_String_AttachCopy(&szSourceName, "SOURCENAME");
	// Time
	OpcUa_String szTime;
	OpcUa_String_Initialize(&szTime);
	OpcUa_String_AttachCopy(&szTime, "TIME");
	// Limit
	OpcUa_String szLimit;
	OpcUa_String_Initialize(&szLimit);
	OpcUa_String_AttachCopy(&szLimit, "LIMIT");
	if (m_pRelatedEventDefinition)
	{
		if (OpcUa_String_Compare(&szPlaceHolder, &szSourceNode) == 0)
		{
			// Retrieve sourceNodeId as transform in a string
			CUAVariable* pSourceNode=m_pRelatedEventDefinition->GetSourceNode();
			OpcUa_NodeId* pNodeId = pSourceNode->GetNodeId();
			char* szNodeId = OpcUa_Null;
			Utils::NodeId2String(pNodeId, &szNodeId);
			if (szNodeId)
			{
				OpcUa_String_AttachCopy(pszValue, szNodeId);
				free(szNodeId);
				uStatus = OpcUa_Good;
			}
		}
		else
		{
			if (OpcUa_String_Compare(&szPlaceHolder, &szSourceNodeValue) == 0)
			{
				// Retrieve sourceNodeId as transform in a string
				CUAVariable* pSourceNode = m_pRelatedEventDefinition->GetSourceNode();
				CDataValue* pDataValue=pSourceNode->GetValue();
				OpcUa_String* strVal = OpcUa_Null;
				if (Utils::OpcUaVariantToString(pDataValue->GetValue(), &strVal) == OpcUa_Good)
				{
					OpcUa_String_CopyTo(strVal, pszValue);
					uStatus = OpcUa_Good;
				}
				if (strVal)
				{
					OpcUa_String_Clear(strVal);
					OpcUa_Free(strVal);
					strVal = OpcUa_Null;
				}
			}
			else
			{
				if (OpcUa_String_Compare(&szPlaceHolder, &szSourceName) == 0)
				{
					// Get the value of the UAVariable pointing to sourceName.
					// This UAVariable must contains a OpcUa_String
					CUAVariable* pSourceNode = m_pRelatedEventDefinition->GetSourceNode();
					OpcUa_QualifiedName* szSourceName=pSourceNode->GetBrowseName();
					OpcUa_String_CopyTo(&(szSourceName->Name), pszValue);
					uStatus = OpcUa_Good;
				}
				else
				{
					if (OpcUa_String_Compare(&szPlaceHolder, &szTime) == 0)
					{
						// GetUtcTime change it to String
						OpcUa_String* strNow = OpcUa_Null;
						OpcUa_DateTime now;
						if (Utils::OpcUaDateTimeToString(now, &strNow) == OpcUa_Good)
						{
							OpcUa_String_CopyTo(strNow, pszValue);
							uStatus = OpcUa_Good;
						}
					}
					else
					{
						if (OpcUa_String_Compare(&szPlaceHolder, &szLimit) == 0)
						{
							switch (m_pRelatedEventDefinition->GetCurrentThreshold())
							{
							case LimitAlarmThreshold::LowThreshold:
								OpcUa_String_AttachCopy(pszValue, "Low");
								uStatus = OpcUa_Good;
								break;
							case LimitAlarmThreshold::LowLowThreshold:
								OpcUa_String_AttachCopy(pszValue, "LowLow");
								uStatus = OpcUa_Good;
								break;
							case LimitAlarmThreshold::NormalThreshold:
								OpcUa_String_AttachCopy(pszValue, "Normal");
								uStatus = OpcUa_Good;
								break;
							case LimitAlarmThreshold::HighThreshold:
								OpcUa_String_AttachCopy(pszValue, "High");
								uStatus = OpcUa_Good;
								break;
							case LimitAlarmThreshold::HighHighThreshold:
								OpcUa_String_AttachCopy(pszValue, "HighHigh");
								uStatus = OpcUa_Good;
								break;
							default:
								uStatus = OpcUa_BadNotSupported;
								break;
							}
						}
						else
							uStatus = OpcUa_BadNotSupported;
					}
				}
			}
		}
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	OpcUa_String_Clear(&szTime);
	OpcUa_String_Clear(&szSourceName);
	OpcUa_String_Clear(&szSourceNode);
	return uStatus;		
}

