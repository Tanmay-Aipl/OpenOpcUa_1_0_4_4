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
#include "UAVariable.h"
#include "Field.h"
#include "Definition.h"

#include "UADataType.h"
#include "UAMethod.h"
#include "UAView.h"
#include "UAVariableType.h"
#include "UAObjectType.h"
#include "UAObject.h"
#include "MonitoredItemBase.h"
#include "UAVariable.h"
#ifdef _GNUC_
#include <dlfcn.h>
#endif
#include "VpiFuncCaller.h"
#include "VpiTag.h"
#include "VpiWriteObject.h"
#include "VpiDevice.h"
#include "UAReferenceType.h"
#include "Field.h"
#include "Definition.h"
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

//#include "MonitoredItemServer.h"
#include "UAMonitoredItemNotification.h"
#include "UADataChangeNotification.h"
#include "EventDefinition.h"
#include "EventsEngine.h"
using namespace UAEvents;
#include "UAInformationModel.h"
#include "MonitoredItemServer.h"
#include "UAEventNotificationList.h"
#include "QueuedPublishRequest.h"

#include "UAStatusChangeNotification.h"
#include "SubscriptionServer.h"
#include "QueuedCallRequest.h"
#include "QueuedReadRequest.h"
#include "QueuedHistoryReadRequest.h"
#include "QueuedQueryFirstRequest.h"
#include "QueuedQueryNextRequest.h"
#include "SessionServer.h"
#include "UABinding.h"
#include "VfiDataValue.h"
#include "UAHistorianVariable.h"
#include "HaEngine.h"
using namespace UAHistoricalAccess;
#include "ServerApplication.h"

using namespace OpenOpcUa;
using namespace UACoreServer;
CMonitoredItemServer::CMonitoredItemServer(void):CMonitoredItemBase()
{
	m_IsCold=OpcUa_True;
	m_pBase=OpcUa_Null;
	m_StatusCode=OpcUa_Good;
	m_pMonitoringFilter=OpcUa_Null;
	m_pDataChangeFilter=OpcUa_Null;
	m_pEventFilter=OpcUa_Null;
	ZeroMemory(&m_LastNotificationTime,sizeof(OpcUa_DateTime));
	m_LastNotificationTime = OpcUa_DateTime_UtcNow();
	m_pSubscription=OpcUa_Null;	
	//m_eTimestampsToReturn = OpcUa_TimestampsToReturn_Both;
	OpcUa_Mutex_Create(&m_TriggeredItemIdListMutex);
}

CMonitoredItemServer::~CMonitoredItemServer(void)
{
	// Clear TriggedItem attach to this one
	OpcUa_Mutex_Lock(m_TriggeredItemIdListMutex);
	m_TriggeredItemIdList.clear();
	OpcUa_Mutex_Unlock(&m_TriggeredItemIdListMutex);
	OpcUa_Mutex_Delete(&m_TriggeredItemIdListMutex);
	//
	if (m_pSubscription)
		OpcUa_Mutex_Lock(m_pSubscription->GetDataChangeNotificationListMutex());
	if (m_pMonitoringFilter)
	{
		OpcUa_ExtensionObject_Clear(m_pMonitoringFilter);
		OpcUa_Free(m_pMonitoringFilter);
		m_pMonitoringFilter = OpcUa_Null;
	}
	if (m_pDataChangeFilter)
	{
		OpcUa_DataChangeFilter_Clear(m_pDataChangeFilter);
		OpcUa_Free(m_pDataChangeFilter);
		m_pDataChangeFilter = OpcUa_Null;
	}
	if (m_pSubscription)
		OpcUa_Mutex_Unlock(m_pSubscription->GetDataChangeNotificationListMutex());
	// Free the EventFilter
	if (m_pEventFilter)
	{
		for (OpcUa_UInt32 i = 0; i < m_EventFields.size(); i++)
		{
			OpcUa_SimpleAttributeOperand* pSimpleOperand = m_EventFields.at(i);
			if (pSimpleOperand)
			{
				OpcUa_SimpleAttributeOperand_Clear(pSimpleOperand);
				OpcUa_Free(pSimpleOperand);
				/*pSimpleOperand = OpcUa_Null;*/
			}
		}
		m_EventFields.clear();

		/*
		for (OpcUa_Int32 i = 0; i < m_pEventFilter->WhereClause.NoOfElements; i++)
		{
			OpcUa_ContentFilterElement aContentFilterElement = m_pEventFilter->WhereClause.Elements[i];
			OpcUa_ContentFilterElement_Clear(&aContentFilterElement);
		}*/
		OpcUa_EventFilter_Clear(m_pEventFilter);
		OpcUa_Free(m_pEventFilter);
		m_pEventFilter = OpcUa_Null;
	}
}
void CMonitoredItemServer::SetNodeId(OpcUa_NodeId aNodeId)
{
	CMonitoredItemBase::SetNodeId(aNodeId);
}
OpcUa_EventFilter*	CMonitoredItemServer::GetEventFilter() 
{ 
	return m_pEventFilter; 
}
// This method will save a raw version (copy) of the OpcUa_EventFilter and 
// Fill the m_EventFields. m_EventFields is an stl collection of OpcUa_Variant 
// each variant is suppose to contains an EventField
//
void CMonitoredItemServer::SetEventFilter(OpcUa_EventFilter* pEventFilter) 
{
	//
	if (!m_pEventFilter)
	{
		m_pEventFilter = (OpcUa_EventFilter*)OpcUa_Alloc(sizeof(OpcUa_EventFilter));
		OpcUa_EventFilter_Initialize(m_pEventFilter);
	}
	else
	{
		// Clear the previously allocated select clauses
		for (OpcUa_UInt32 i = 0; i < m_EventFields.size(); i++)
		{
			OpcUa_SimpleAttributeOperand* pSimpleOperand=m_EventFields.at(i);
			OpcUa_SimpleAttributeOperand_Clear(pSimpleOperand);
			OpcUa_Free(pSimpleOperand); 
		}
		m_EventFields.clear();
		/*
		// Clear the previously allocated where clauses
		for (OpcUa_Int32 i = 0; i < m_pEventFilter->WhereClause.NoOfElements; i++)
		{
			OpcUa_ContentFilterElement aContentFilterElement = m_pEventFilter->WhereClause.Elements[i];
			OpcUa_ContentFilterElement_Clear(&aContentFilterElement);
		}
		*/
		OpcUa_EventFilter_Clear(m_pEventFilter);
	}

	OpcUa_EventFilter_Initialize(m_pEventFilter);
	
	if (pEventFilter->NoOfSelectClauses>0)
	{
		// NoOfSelectClauses
		m_pEventFilter->NoOfSelectClauses = pEventFilter->NoOfSelectClauses;
		// SelectClauses
		m_pEventFilter->SelectClauses = (OpcUa_SimpleAttributeOperand*)OpcUa_Alloc(sizeof(OpcUa_SimpleAttributeOperand)*(pEventFilter->NoOfSelectClauses));
	}
	// let's counting the number of EventField require by the client for the monitoredItem
	OpcUa_Int32 uiEventField = 0;
	for (OpcUa_Int32 iClauses = 0; iClauses < pEventFilter->NoOfSelectClauses; iClauses++)
	{
		OpcUa_SimpleAttributeOperand aSimpleAttributeOperand = pEventFilter->SelectClauses[iClauses];
		uiEventField+=aSimpleAttributeOperand.NoOfBrowsePath;
	}
	if (uiEventField>0)
	{
		//OpcUa_SimpleAttributeOperand* pEventField = (OpcUa_SimpleAttributeOperand*)OpcUa_Alloc(uiEventField*sizeof(OpcUa_SimpleAttributeOperand));

		OpcUa_Int32 i = 0;
		for (OpcUa_Int32 iClauses = 0; iClauses <pEventFilter->NoOfSelectClauses && i < uiEventField; iClauses++)
		{
			OpcUa_SimpleAttributeOperand* pEventField = (OpcUa_SimpleAttributeOperand*)OpcUa_Alloc(sizeof(OpcUa_SimpleAttributeOperand));
			//OpcUa_SimpleAttributeOperand_Initialize(&pEventField[i]);
			OpcUa_SimpleAttributeOperand_Initialize(pEventField);
			OpcUa_SimpleAttributeOperand aSimpleAttributeOperand = pEventFilter->SelectClauses[iClauses];
			// AttributeId
			m_pEventFilter->SelectClauses[iClauses].AttributeId = aSimpleAttributeOperand.AttributeId;
			// TypeDefinitionId
			OpcUa_NodeId_Initialize(&(m_pEventFilter->SelectClauses[iClauses].TypeDefinitionId));
			OpcUa_NodeId_CopyTo(&(aSimpleAttributeOperand.TypeDefinitionId), &(m_pEventFilter->SelectClauses[iClauses].TypeDefinitionId)); // NodeId of an EventType 
			// IndexRange
			OpcUa_String_Initialize(&(m_pEventFilter->SelectClauses[iClauses].IndexRange));
			OpcUa_String_CopyTo(&(aSimpleAttributeOperand.IndexRange), &(m_pEventFilter->SelectClauses[iClauses].IndexRange));
			// save the EventField
			pEventField->AttributeId = aSimpleAttributeOperand.AttributeId;
			OpcUa_NodeId_Initialize(&(pEventField->TypeDefinitionId));
			OpcUa_NodeId_CopyTo(&(aSimpleAttributeOperand.TypeDefinitionId), &(pEventField->TypeDefinitionId));
			OpcUa_String_Initialize(&(pEventField->IndexRange));
			OpcUa_String_CopyTo(&(aSimpleAttributeOperand.IndexRange), &(pEventField->IndexRange));
			pEventField->NoOfBrowsePath = aSimpleAttributeOperand.NoOfBrowsePath;
			//
			if (aSimpleAttributeOperand.NoOfBrowsePath > 0)
			{
				m_pEventFilter->SelectClauses[iClauses].BrowsePath = (OpcUa_QualifiedName*)OpcUa_Alloc(sizeof(OpcUa_QualifiedName)*aSimpleAttributeOperand.NoOfBrowsePath);
				pEventField->BrowsePath = (OpcUa_QualifiedName*)OpcUa_Alloc(sizeof(OpcUa_QualifiedName)*aSimpleAttributeOperand.NoOfBrowsePath);
				m_pEventFilter->SelectClauses[iClauses].NoOfBrowsePath = aSimpleAttributeOperand.NoOfBrowsePath;
				for (OpcUa_Int32 iBrowsePath = 0; iBrowsePath < aSimpleAttributeOperand.NoOfBrowsePath; iBrowsePath++)
				{
					OpcUa_QualifiedName aBrowsePath = aSimpleAttributeOperand.BrowsePath[iBrowsePath]; // a path to an InstanceDeclaration
					OpcUa_QualifiedName_Initialize(&(m_pEventFilter->SelectClauses[iClauses].BrowsePath[iBrowsePath]));
					OpcUa_QualifiedName_CopyTo(&aBrowsePath, &(m_pEventFilter->SelectClauses[iClauses].BrowsePath[iBrowsePath]));
					// Let check the BrowsePath to initialize the OpcUa_Variant
					OpcUa_QualifiedName_Initialize(&(pEventField->BrowsePath[iBrowsePath]));
					OpcUa_QualifiedName_CopyTo(&aBrowsePath, &(pEventField->BrowsePath[iBrowsePath]));
				}
			}
			else
			{
				pEventField->BrowsePath = OpcUa_Null;
				m_pEventFilter->SelectClauses[iClauses].BrowsePath = OpcUa_Null;
			}
			m_EventFields.push_back(pEventField);
			i++;
		}
		// WhereClause
		OpcUa_ContentFilter_Initialize(&(m_pEventFilter->WhereClause));
		// NoOfElements
		m_pEventFilter->WhereClause.NoOfElements = pEventFilter->WhereClause.NoOfElements;
		// Elements
		if (pEventFilter->WhereClause.NoOfElements)
		{
			m_pEventFilter->WhereClause.Elements =
				(OpcUa_ContentFilterElement*)OpcUa_Alloc(sizeof(OpcUa_ContentFilterElement)*(pEventFilter->WhereClause.NoOfElements));
			for (OpcUa_Int32 ii = 0; ii < pEventFilter->WhereClause.NoOfElements; ii++)
			{
				OpcUa_ContentFilterElement_Initialize(&(m_pEventFilter->WhereClause.Elements[ii]));
				//FilterOperator
				m_pEventFilter->WhereClause.Elements[ii].FilterOperator = pEventFilter->WhereClause.Elements[ii].FilterOperator;
				//NoOfFilterOperands
				m_pEventFilter->WhereClause.Elements[ii].NoOfFilterOperands = pEventFilter->WhereClause.Elements[ii].NoOfFilterOperands;
				//FilterOperands

				m_pEventFilter->WhereClause.Elements[ii].FilterOperands =
					(OpcUa_ExtensionObject*)OpcUa_Alloc(sizeof(OpcUa_ExtensionObject)*(m_pEventFilter->WhereClause.Elements[ii].NoOfFilterOperands));
				for (OpcUa_Int32 iii = 0; iii < pEventFilter->WhereClause.Elements[ii].NoOfFilterOperands; iii++)
				{
					OpcUa_ExtensionObject_Initialize(&(m_pEventFilter->WhereClause.Elements[ii].FilterOperands[iii]));
					OpcUa_ExtensionObject_CopyTo(&(pEventFilter->WhereClause.Elements[ii].FilterOperands[iii]), &(m_pEventFilter->WhereClause.Elements[ii].FilterOperands[iii]));
					//OpcUa_ExtensionObject* pExtensionObject = m_pEventFilter->WhereClause.Elements[ii].FilterOperands;
					//Copy(&pExtensionObject,
					//	&(pEventFilter->WhereClause.Elements[ii].FilterOperands[iii]));
				}
			}
		}
	}
}
#define Initialize_String_Content(name)\
{\
	OpcUa_String_Initialize(&name);\
	OpcUa_String_AttachCopy(&name, OpcUa_BrowseName_##name);\
}

// This method fill EventField based on the value read from the AddressSpace 
// ppUAEventNotificationList I/O parameter
OpcUa_StatusCode CMonitoredItemServer::GenerateFromAddressSpaceEventField(
															OpcUa_String browsePath, 
															OpcUa_SimpleAttributeOperand* pSimpleAttributeOperand, 
															CUAEventNotificationList** ppUAEventNotificationList)
{
	OpcUa_StatusCode uStatus = OpcUa_BadNotFound;
	OpcUa_String Id;
	// SimpleEvents
	OpcUa_String Message;
	OpcUa_String Severity;
	// ConditionType
	OpcUa_String ConditionName;
	OpcUa_String ConditionClassId;
	OpcUa_String ConditionClassName;
	OpcUa_String Retain;
	OpcUa_String EnabledState;
	// AcknowledgeableConditionType
	OpcUa_String AckedState;
	OpcUa_String ConfirmedState;
	// AlarmConditionType
	OpcUa_String ActiveState;
	OpcUa_String EffectiveDisplayName;

	// Initialization of the AddressSpace

	{
		Initialize_String_Content(Message);
		Initialize_String_Content(Severity);
		Initialize_String_Content(ConditionName);
		Initialize_String_Content(ConditionClassId);
		Initialize_String_Content(ConditionClassName);
		Initialize_String_Content(Retain);
		Initialize_String_Content(AckedState);
		Initialize_String_Content(Id);
		Initialize_String_Content(ConfirmedState);
		Initialize_String_Content(ActiveState);
		Initialize_String_Content(EffectiveDisplayName);
		Initialize_String_Content(EnabledState);

	}
	OpcUa_Variant* pOutField = OpcUa_Null;
	CEventDefinition* pEventDefinition = (*ppUAEventNotificationList)->GetEventDefinition();
	if (pEventDefinition)
	{
		// First Field that will not change on confirm, Ack, Comment,etc.
		// Message - Severity - ConditionName - ConditionClassId - ConditionClassName
		if ( (OpcUa_String_Compare(&browsePath, &Message) == 0)
			|| (OpcUa_String_Compare(&browsePath, &Severity) == 0)
			|| /*(OpcUa_String_Compare(&browsePath, &ConditionName) == 0)
			||*/  (OpcUa_String_Compare(&browsePath, &ConditionClassId) == 0)
			|| (OpcUa_String_Compare(&browsePath, &ConditionClassName) == 0))
		{
			OpcUa_Variant* pVarVal = OpcUa_Null;
			CUAObject* pConditionNode = pEventDefinition->GetConditionNode();
			if (GetAlarmField(pConditionNode, pSimpleAttributeOperand, &pVarVal) == OpcUa_Good)
			{

				if (pVarVal)
				{
					pOutField = (OpcUa_Variant*)OpcUa_Alloc(sizeof(OpcUa_Variant));
					OpcUa_Variant_Initialize(pOutField);
					OpcUa_UInt32 iResult = pSimpleAttributeOperand->NoOfBrowsePath - 1;
					OpcUa_Variant_Initialize(pOutField);
					OpcUa_Variant_CopyTo(&pVarVal[iResult], pOutField);
					for (OpcUa_UInt16 i = 0; i<pSimpleAttributeOperand->NoOfBrowsePath; i++)
						OpcUa_Variant_Clear(&pVarVal[i]);
					OpcUa_Free(pVarVal);
					pVarVal = OpcUa_Null;
					uStatus = OpcUa_Good;
				}

			}
		}
		// 
		// Retain
		if (OpcUa_String_Compare(&browsePath, &Retain) == 0)
		{
			OpcUa_Variant* pVarVal = OpcUa_Null;
			CUAObject* pConditionNode = pEventDefinition->GetConditionNode();
			if (GetAlarmField(pConditionNode, pSimpleAttributeOperand, &pVarVal) == OpcUa_Good)
			{
				if (pVarVal)
				{
					OpcUa_UInt32 iResult = pSimpleAttributeOperand->NoOfBrowsePath - 1;
					pOutField = (OpcUa_Variant*)OpcUa_Alloc(sizeof(OpcUa_Variant));
					OpcUa_Variant_Initialize(pOutField);
					OpcUa_Variant_CopyTo(&pVarVal[iResult], pOutField);
					OpcUa_String aTmpName =
						pSimpleAttributeOperand->BrowsePath[pSimpleAttributeOperand->NoOfBrowsePath - 1].Name;
					// Mise à jour de l'index interne relatif au Retain
					OpcUa_EventNotificationList* pEventNotificationList = (*ppUAEventNotificationList)->GetInternalEventNotificationList();
					if (pEventNotificationList)
					{
						FastEventFieldIndex aFastEventFieldIndex = (*ppUAEventNotificationList)->GetFastEventFieldIndex();
						OpcUa_Int32 iNoOfEventFields = pEventNotificationList->Events[0].NoOfEventFields;
						// Mise à jour de l'index interne relatif au Retain
						if ((OpcUa_String_Compare(&Retain, &aTmpName) == 0)
							&& (pVarVal[iResult].Datatype == OpcUaType_Boolean))
							aFastEventFieldIndex.iRetain = iNoOfEventFields;
						(*ppUAEventNotificationList)->SetFastEventFieldIndex(&aFastEventFieldIndex);
					}
					for (OpcUa_UInt16 i = 0; i<pSimpleAttributeOperand->NoOfBrowsePath; i++)
						OpcUa_Variant_Clear(&pVarVal[i]);
					OpcUa_Free(pVarVal);
					pVarVal = OpcUa_Null;
					uStatus = OpcUa_Good;
				}
			}
		}
		// AckedState
		if (OpcUa_String_Compare(&browsePath, &AckedState) == 0)
		{
			OpcUa_Variant* pVarVal = OpcUa_Null;
			CUAObject* pConditionNode = pEventDefinition->GetConditionNode();
			if (GetAlarmField(pConditionNode, pSimpleAttributeOperand, &pVarVal) == OpcUa_Good)
			{
				if (pVarVal)
				{
					OpcUa_UInt32 iResult = pSimpleAttributeOperand->NoOfBrowsePath - 1;
					pOutField = (OpcUa_Variant*)OpcUa_Alloc(sizeof(OpcUa_Variant));
					OpcUa_Variant_Initialize(pOutField);
					OpcUa_Variant_CopyTo(&pVarVal[iResult], pOutField);
					OpcUa_String aTmpName =
						pSimpleAttributeOperand->BrowsePath[pSimpleAttributeOperand->NoOfBrowsePath - 1].Name;
					// Mise à jour de l'index interne relatif au Ackedstate
					OpcUa_EventNotificationList* pEventNotificationList = (*ppUAEventNotificationList)->GetInternalEventNotificationList();
					if (pEventNotificationList)
					{
						FastEventFieldIndex aFastEventFieldIndex = (*ppUAEventNotificationList)->GetFastEventFieldIndex();
						OpcUa_Int32 iNoOfEventFields = pEventNotificationList->Events[0].NoOfEventFields;
						if ((OpcUa_String_Compare(&AckedState, &aTmpName) == 0)
							&& (pVarVal[iResult].Datatype == OpcUaType_LocalizedText))
							aFastEventFieldIndex.iAckedState = iNoOfEventFields;
						// Mise à jour de l'index interne relatif au Ackedstate Id
						if ((OpcUa_String_Compare(&Id, &aTmpName) == 0)
							&& (pVarVal[iResult].Datatype == OpcUaType_Boolean))
							aFastEventFieldIndex.iAckedStateId = iNoOfEventFields;
						(*ppUAEventNotificationList)->SetFastEventFieldIndex(&aFastEventFieldIndex);
					}
					for (OpcUa_UInt16 i = 0; i<pSimpleAttributeOperand->NoOfBrowsePath; i++)
						OpcUa_Variant_Clear(&pVarVal[i]);
					OpcUa_Free(pVarVal);
					uStatus = OpcUa_Good;
				}
			}
		}
		// ConfirmedState
		if (OpcUa_String_Compare(&browsePath, &ConfirmedState) == 0)
		{
			OpcUa_Variant* pVarVal = OpcUa_Null;
			CUAObject* pConditionNode = pEventDefinition->GetConditionNode();
			if (GetAlarmField(pConditionNode, pSimpleAttributeOperand, &pVarVal) == OpcUa_Good)
			{
				if (pVarVal)
				{
					OpcUa_UInt32 iResult = pSimpleAttributeOperand->NoOfBrowsePath - 1;
					pOutField = (OpcUa_Variant*)OpcUa_Alloc(sizeof(OpcUa_Variant));
					OpcUa_Variant_Initialize(pOutField);
					OpcUa_Variant_CopyTo(&pVarVal[iResult], pOutField);
					OpcUa_String aTmpName =
						pSimpleAttributeOperand->BrowsePath[pSimpleAttributeOperand->NoOfBrowsePath - 1].Name;
					// Mise à jour de l'index interne relatif au Ackedstate
					OpcUa_EventNotificationList* pEventNotificationList = (*ppUAEventNotificationList)->GetInternalEventNotificationList();
					if (pEventNotificationList)
					{
						FastEventFieldIndex aFastEventFieldIndex = (*ppUAEventNotificationList)->GetFastEventFieldIndex();
						OpcUa_Int32 iNoOfEventFields = pEventNotificationList->Events[0].NoOfEventFields;
						if ((OpcUa_String_Compare(&ConfirmedState, &aTmpName) == 0)
							&& (pVarVal[iResult].Datatype == OpcUaType_LocalizedText))
							aFastEventFieldIndex.iConfirmedState = iNoOfEventFields;
						// Mise à jour de l'index interne relatif au Ackedstate Id
						if ((OpcUa_String_Compare(&Id, &aTmpName) == 0)
							&& (pVarVal[iResult].Datatype == OpcUaType_Boolean))
							aFastEventFieldIndex.iConfirmedStateId = iNoOfEventFields;
						(*ppUAEventNotificationList)->SetFastEventFieldIndex(&aFastEventFieldIndex);
					}
					for (OpcUa_UInt16 i = 0; i<pSimpleAttributeOperand->NoOfBrowsePath; i++)
						OpcUa_Variant_Clear(&pVarVal[i]);
					OpcUa_Free(pVarVal);
					pVarVal = OpcUa_Null;
					uStatus = OpcUa_Good;
				}
			}
		}
		// ActiveState
		if (OpcUa_String_Compare(&browsePath, &ActiveState) == 0)
		{
			OpcUa_Variant* pVarVal = OpcUa_Null;
			CUAObject* pConditionNode = pEventDefinition->GetConditionNode();
			uStatus = GetAlarmField(pConditionNode, pSimpleAttributeOperand, &pVarVal);
			if (uStatus == OpcUa_Good)
			{
				if (pVarVal)
				{
					OpcUa_UInt32 iResult = pSimpleAttributeOperand->NoOfBrowsePath - 1;
					pOutField = (OpcUa_Variant*)OpcUa_Alloc(sizeof(OpcUa_Variant));
					OpcUa_Variant_Initialize(pOutField);
					OpcUa_Variant_CopyTo(&pVarVal[iResult], pOutField);
					OpcUa_String aTmpName =
						pSimpleAttributeOperand->BrowsePath[pSimpleAttributeOperand->NoOfBrowsePath - 1].Name;
					// Mise à jour de l'index interne relatif au Ackedstate
					OpcUa_EventNotificationList* pEventNotificationList = (*ppUAEventNotificationList)->GetInternalEventNotificationList();
					if (pEventNotificationList)
					{
						FastEventFieldIndex aFastEventFieldIndex = (*ppUAEventNotificationList)->GetFastEventFieldIndex();
						OpcUa_Int32 iNoOfEventFields = pEventNotificationList->Events[0].NoOfEventFields;
						// Mise à jour de l'index interne relatif au ActiveState
						if ((OpcUa_String_Compare(&ActiveState, &aTmpName) == 0)
							&& (pVarVal[iResult].Datatype == OpcUaType_LocalizedText))
							aFastEventFieldIndex.iActiveState = iNoOfEventFields;
						// Mise à jour de l'index interne relatif au ActiveState Id
						if ((OpcUa_String_Compare(&Id, &aTmpName) == 0)
							&& (pVarVal[iResult].Datatype == OpcUaType_Boolean))
							aFastEventFieldIndex.iActiveStateId = iNoOfEventFields;
						// Mise à jour de l'index interne relatif au ActiveState EffectiveDisplayName
						if ((OpcUa_String_Compare(&EffectiveDisplayName, &aTmpName) == 0)
							&& (pVarVal[iResult].Datatype == OpcUaType_LocalizedText))
							aFastEventFieldIndex.iActiveStateEffectiveDisplayName = iNoOfEventFields;
						(*ppUAEventNotificationList)->SetFastEventFieldIndex(&aFastEventFieldIndex);
					}
					for (OpcUa_UInt16 i = 0; i<pSimpleAttributeOperand->NoOfBrowsePath; i++)
						OpcUa_Variant_Clear(&pVarVal[i]);
					OpcUa_Free(pVarVal);
					pVarVal = OpcUa_Null;
					uStatus = OpcUa_Good;
				}
			}
			else
				uStatus = OpcUa_BadNotFound;
		}
		if (OpcUa_String_Compare(&browsePath, &EffectiveDisplayName) == 0)
		{
			// Mise à jour de l'index interne relatif au Ackedstate
			OpcUa_EventNotificationList* pEventNotificationList = (*ppUAEventNotificationList)->GetInternalEventNotificationList();
			if (pEventNotificationList)
			{
				FastEventFieldIndex aFastEventFieldIndex = (*ppUAEventNotificationList)->GetFastEventFieldIndex();
				if (aFastEventFieldIndex.iActiveStateId != -1)
				{
					OpcUa_Int32 iNoOfEventFields = pEventNotificationList->Events[0].NoOfEventFields;
					//CUAObject* pConditionNode = pEventDefinition->GetConditionNode();
					pOutField = (OpcUa_Variant*)OpcUa_Alloc(sizeof(OpcUa_Variant));
					OpcUa_Variant_Initialize(pOutField);
					pOutField->Datatype = OpcUaType_LocalizedText;
					pOutField->Value.LocalizedText = (OpcUa_LocalizedText*)OpcUa_Alloc(sizeof(OpcUa_LocalizedText));
					OpcUa_LocalizedText_Initialize(pOutField->Value.LocalizedText);
					OpcUa_String_AttachCopy(&(pOutField->Value.LocalizedText->Locale), "en-us");
					OpcUa_Boolean bActiveStateId = pEventNotificationList->Events[0].EventFields[aFastEventFieldIndex.iActiveStateId].Value.Boolean;
					if (bActiveStateId)
						OpcUa_String_AttachCopy(&(pOutField->Value.LocalizedText->Text), "Active");
					else
						OpcUa_String_AttachCopy(&(pOutField->Value.LocalizedText->Text), "InActive");

					// Mise à jour de l'index interne relatif au EffectiveDisplayName
					aFastEventFieldIndex.iActiveStateEffectiveDisplayName = iNoOfEventFields;
					uStatus = OpcUa_Good;
				}
			}
		}
		// Transfert in the OpcUa_EventNotificationList of the CUAEventNotificationList (ppUAEventNotificationList)
		// ppUAEventNotificationList is an I/O parameter of the method.
		if (uStatus == OpcUa_Good)
		{
			std::vector<OpcUa_Variant*> localEventFields;
			OpcUa_EventNotificationList* pEventNotificationList = (*ppUAEventNotificationList)->GetInternalEventNotificationList();
			if (pEventNotificationList)
			{
				OpcUa_Int32 iNoOfEventFields = pEventNotificationList->Events[0].NoOfEventFields;

				for (OpcUa_Int32 i = 0; i < iNoOfEventFields; i++)
				{
					OpcUa_Variant* pVariant = (OpcUa_Variant*)OpcUa_Alloc(sizeof(OpcUa_Variant));
					OpcUa_Variant_Initialize(pVariant);
					OpcUa_Variant_CopyTo(&pEventNotificationList->Events[0].EventFields[i], pVariant);
					localEventFields.push_back(pVariant);
					OpcUa_Variant_Clear(&pEventNotificationList->Events[0].EventFields[i]);
				}
				if (pEventNotificationList->Events[0].EventFields)
					OpcUa_Free(pEventNotificationList->Events[0].EventFields);
				// Add the new one
				if (pOutField)
					localEventFields.push_back(pOutField);
				// update the size
				iNoOfEventFields = localEventFields.size();
				pEventNotificationList->Events[0].NoOfEventFields = iNoOfEventFields;
				pEventNotificationList->Events[0].EventFields = (OpcUa_Variant*)OpcUa_Alloc(sizeof(OpcUa_Variant)*iNoOfEventFields);
				for (OpcUa_Int32 i = 0; i < iNoOfEventFields; i++)
				{
					OpcUa_Variant* apOutField = localEventFields.at(i);
					OpcUa_Variant_Initialize(&(pEventNotificationList->Events[0].EventFields[i]));
					OpcUa_Variant_CopyTo(apOutField, &(pEventNotificationList->Events[0].EventFields[i]));
					//OpcUa_Variant_Clear(apOutField);
				}
			}
			else
			{
				uStatus = OpcUa_BadInternalError;
				OpcUa_Variant_Clear(pOutField);
				OpcUa_Free(pOutField);
				pOutField = OpcUa_Null;
			}
			std::vector<OpcUa_Variant*>::iterator it  ;
			for (it = localEventFields.begin(); it != localEventFields.end(); it++)
			{
				OpcUa_Variant* pVarVal = *it;
				OpcUa_Variant_Clear(pVarVal);
				OpcUa_Free(pVarVal);
			}
			localEventFields.clear();
		}
		else
		{
			OpcUa_Variant_Clear(pOutField);
			OpcUa_Free(pOutField);
			pOutField = OpcUa_Null;
		}
	}
	{
		OpcUa_String_Clear(&Message);
		OpcUa_String_Clear(&Severity);
		OpcUa_String_Clear(&ConditionName);
		OpcUa_String_Clear(&ConditionClassId);
		OpcUa_String_Clear(&ConditionClassName);
		OpcUa_String_Clear(&Retain);
		OpcUa_String_Clear(&AckedState);
		OpcUa_String_Clear(&Id);
		OpcUa_String_Clear(&ConfirmedState);
		OpcUa_String_Clear(&ActiveState);
		OpcUa_String_Clear(&EffectiveDisplayName);
		OpcUa_String_Clear(&EnabledState);

	}
	return uStatus;
}
// The method fill EventField based on CEventDefinition, infered value or calculated values
// ppUAEventNotificationList I/O parameter
OpcUa_StatusCode CMonitoredItemServer::GenerateDynamicEventField(OpcUa_String browsePath, CUAEventNotificationList** ppUAEventNotificationList)
{
	OpcUa_StatusCode uStatus = OpcUa_BadNotFound;
	// Parameter for SimpleEvents
	OpcUa_String EventId;
	OpcUa_String EventType;
	OpcUa_String SourceNode;
	OpcUa_String SourceName;
	OpcUa_String Time;
	OpcUa_String ReceiveTime;
	OpcUa_String LocalTime;
	OpcUa_String Message;
	OpcUa_String Severity;
	OpcUa_String BranchId;
	OpcUa_String Comment;
	OpcUa_String HighHighLimit;
	OpcUa_String HighLimit;
	OpcUa_String LowLowLimit;
	OpcUa_String LowLimit;

	// Parameter for ConditionType
	OpcUa_String ConditionName;
	OpcUa_String SystemState;

	OpcUa_Variant* pOutField = OpcUa_Null;
	// Access the pEventDefinition associated with this ppUAEventNotificationList
	CEventDefinition* pEventDefinition= (*ppUAEventNotificationList)->GetEventDefinition();
	if (pEventDefinition)
	{
		// initialize string with name of localyInitialize EventField
		{
			// SimpleEvent
			Initialize_String_Content(EventId);
			Initialize_String_Content(EventType);
			Initialize_String_Content(SourceNode);
			Initialize_String_Content(SourceName);
			Initialize_String_Content(Time);
			Initialize_String_Content(ReceiveTime);
			Initialize_String_Content(LocalTime);
			Initialize_String_Content(Message);
			Initialize_String_Content(Severity);			
			// ConditionType
			Initialize_String_Content(ConditionName);
			Initialize_String_Content(BranchId);
			Initialize_String_Content(LowLimit);
			Initialize_String_Content(LowLowLimit);
			Initialize_String_Content(HighLimit);
			Initialize_String_Content(HighHighLimit);
			// SystemStatusChangeEvent
			OpcUa_String_Initialize(&SystemState);
			OpcUa_String_AttachCopy(&SystemState, "SystemState");
			// Comment
			Initialize_String_Content(Comment);			
		}
		if (ppUAEventNotificationList)
		{
			// Generate value
			if (OpcUa_String_Compare(&browsePath, &EventId) == 0)
			{
				// EventId
				pOutField = (OpcUa_Variant*)OpcUa_Alloc(sizeof(OpcUa_Variant));
				OpcUa_Variant_Initialize(pOutField);
				pOutField->ArrayType = OpcUa_VariantArrayType_Scalar;
				pOutField->Datatype = OpcUaType_ByteString;
				OpcUa_ByteString_Initialize(&(pOutField->Value.ByteString));
				// Generate a ByteString
				pOutField->Value.ByteString.Length = 16;
				pOutField->Value.ByteString.Data = (OpcUa_Byte*)OpcUa_Alloc(16);
				ZeroMemory(pOutField->Value.ByteString.Data, 16);
				OpcUa_Int32 iRandVal = rand();
				OpcUa_UInt32 ui32EventId = (OpcUa_UInt32)(iRandVal*OpcUa_Clock());
				char* buff = (char*)malloc(16);
				ZeroMemory(buff, 16);
				sprintf(buff, "%x", (unsigned int)ui32EventId);
				memcpy(pOutField->Value.ByteString.Data, buff, 15);
				free(buff);
				uStatus = OpcUa_Good;
			}
			// Extract from the CEventDefinition
			if (OpcUa_String_Compare(&browsePath, &EventType) == 0)
			{
				// EventType
				CUAObjectType* pEventType = pEventDefinition->GetEventType();
				if (pEventType)
				{
					OpcUa_NodeId* pEventTypeNodeId = pEventType->GetNodeId();
					pOutField = (OpcUa_Variant*)OpcUa_Alloc(sizeof(OpcUa_Variant));
					OpcUa_Variant_Initialize(pOutField);
					pOutField->Datatype = OpcUaType_NodeId;
					pOutField->Value.NodeId = (OpcUa_NodeId*)OpcUa_Alloc(sizeof(OpcUa_NodeId));
					OpcUa_NodeId_Initialize(pOutField->Value.NodeId);
					OpcUa_NodeId_CopyTo(pEventTypeNodeId, pOutField->Value.NodeId);
					uStatus = OpcUa_Good;
				}
				else
					uStatus = OpcUa_BadInvalidArgument;
			}
			// Extract from the CEventDefinition
			if (OpcUa_String_Compare(&browsePath, &SourceNode) == 0)
			{
				// SourceNode
				OpcUa_NodeId* pSourceNodeId=OpcUa_Null;
				CUAVariable* pSourceNode = pEventDefinition->GetSourceNode();
				if (pSourceNode)
					pSourceNodeId = pSourceNode->GetNodeId();
				if (pSourceNodeId)
				{
					pOutField = (OpcUa_Variant*)OpcUa_Alloc(sizeof(OpcUa_Variant));
					OpcUa_Variant_Initialize(pOutField);
					pOutField->Datatype = OpcUaType_NodeId;
					pOutField->Value.NodeId = (OpcUa_NodeId*)OpcUa_Alloc(sizeof(OpcUa_NodeId));
					OpcUa_NodeId_Initialize(pOutField->Value.NodeId);
					OpcUa_NodeId_CopyTo(pSourceNodeId, pOutField->Value.NodeId);
					uStatus = OpcUa_Good;
				}
				else
					uStatus = OpcUa_BadInvalidArgument;
			}
			// inferred value
			if (OpcUa_String_Compare(&browsePath, &SourceName) == 0)
			{
				// SourceName
				OpcUa_String sourceName;
				OpcUa_String_Initialize(&sourceName);
				CUAVariable* pSourceName = pEventDefinition->GetSourceName();
				if (pSourceName)
					sourceName = pSourceName->GetBrowseName()->Name;
				pOutField = (OpcUa_Variant*)OpcUa_Alloc(sizeof(OpcUa_Variant));
				OpcUa_Variant_Initialize(pOutField);
				pOutField->Datatype = OpcUaType_String;
				OpcUa_String_Initialize(&(pOutField->Value.String));
				OpcUa_String_CopyTo(&sourceName, &(pOutField->Value.String));
				uStatus = OpcUa_Good;
			}
			// Utc Now
			if (OpcUa_String_Compare(&browsePath, &Time) == 0)
			{
				pOutField = (OpcUa_Variant*)OpcUa_Alloc(sizeof(OpcUa_Variant));
				if (pOutField)
				{
					OpcUa_Variant_Initialize(pOutField);
					pOutField->ArrayType = OpcUa_VariantArrayType_Scalar;
					pOutField->Datatype = OpcUaType_DateTime;
					OpcUa_DateTime_Initialize(&(pOutField->Value.DateTime));
					pOutField->Value.DateTime = OpcUa_DateTime_UtcNow();
					OpcUa_EventNotificationList* pEventNotificationList = (*ppUAEventNotificationList)->GetInternalEventNotificationList();
					if (pEventNotificationList)
					{
						FastEventFieldIndex aFastEventIndexField = (*ppUAEventNotificationList)->GetFastEventFieldIndex();
						aFastEventIndexField.iDateTime = pEventNotificationList->Events[0].NoOfEventFields;
						(*ppUAEventNotificationList)->SetFastEventFieldIndex(&aFastEventIndexField);
					}
					uStatus = OpcUa_Good;
				}
				else
					uStatus = OpcUa_BadOutOfMemory;
			}
			// Utc Now
			if (OpcUa_String_Compare(&browsePath, &ReceiveTime) == 0)
			{
				pOutField = (OpcUa_Variant*)OpcUa_Alloc(sizeof(OpcUa_Variant));
				OpcUa_Variant_Initialize(pOutField);
				pOutField->ArrayType = OpcUa_VariantArrayType_Scalar;
				pOutField->Datatype = OpcUaType_DateTime;
				OpcUa_DateTime_Initialize(&(pOutField->Value.DateTime));
				pOutField->Value.DateTime = OpcUa_DateTime_UtcNow();
				uStatus = OpcUa_Good;
			}
			// Utc Now
			if (OpcUa_String_Compare(&browsePath, &LocalTime) == 0)
			{
				pOutField = (OpcUa_Variant*)OpcUa_Alloc(sizeof(OpcUa_Variant));
				OpcUa_Variant_Initialize(pOutField);
				pOutField->ArrayType = OpcUa_VariantArrayType_Scalar;
				pOutField->Datatype = OpcUaType_DateTime;
				OpcUa_DateTime_Initialize(&(pOutField->Value.DateTime));
				pOutField->Value.DateTime = OpcUa_DateTime_UtcNow();
				uStatus = OpcUa_Good;
			}
			// Message
			if (OpcUa_String_Compare(&browsePath, &Message) == 0)
			{
				pOutField = (OpcUa_Variant*)OpcUa_Alloc(sizeof(OpcUa_Variant));
				if (pOutField)
				{
					OpcUa_Variant_Initialize(pOutField);
					pOutField->ArrayType = OpcUa_VariantArrayType_Scalar;
					pOutField->Datatype = OpcUaType_LocalizedText;
					OpcUa_DateTime_Initialize(&(pOutField->Value.LocalizedText));
					OpcUa_EventNotificationList* pEventNotificationList = (*ppUAEventNotificationList)->GetInternalEventNotificationList();
					if (pEventNotificationList)
					{
						FastEventFieldIndex aFastEventIndexField = (*ppUAEventNotificationList)->GetFastEventFieldIndex();
						aFastEventIndexField.iMessage = pEventNotificationList->Events[0].NoOfEventFields;
						(*ppUAEventNotificationList)->SetFastEventFieldIndex(&aFastEventIndexField);
					}
					uStatus = OpcUa_Good;
				}
				else
					uStatus = OpcUa_BadOutOfMemory;
			}
			// Empty value BranchId
			if (OpcUa_String_Compare(&browsePath, &BranchId) == 0)
			{
				pOutField = (OpcUa_Variant*)OpcUa_Alloc(sizeof(OpcUa_Variant));
				OpcUa_Variant_Initialize(pOutField);
				pOutField->ArrayType = OpcUa_VariantArrayType_Scalar;
				pOutField->Datatype = OpcUaType_NodeId;
				OpcUa_NodeId_Initialize(pOutField->Value.NodeId);			
				uStatus = OpcUa_Good;
			}
			// ConditionName
			if (OpcUa_String_Compare(&browsePath, &ConditionName) == 0)
			{
				pOutField = (OpcUa_Variant*)OpcUa_Alloc(sizeof(OpcUa_Variant));
				OpcUa_Variant_Initialize(pOutField);
				pOutField->ArrayType = OpcUa_VariantArrayType_Scalar;
				pOutField->Datatype = OpcUaType_String;		
				OpcUa_String sourceName;
				CUAVariable* pSourceName = pEventDefinition->GetSourceName();
				if (pSourceName)
					sourceName = pSourceName->GetBrowseName()->Name;
				OpcUa_String_Initialize(&(pOutField->Value.String));
				OpcUa_String_CopyTo(&sourceName, &(pOutField->Value.String));
				uStatus = OpcUa_Good;
			}
			// Unknow value. The correct value will be placed later
			if (OpcUa_String_Compare(&browsePath, &SystemState) == 0)
			{
				// SystemState
				OpcUa_EventNotificationList* pEventNotificationList = (*ppUAEventNotificationList)->GetInternalEventNotificationList();
				if (pEventNotificationList)
				{
					pOutField = (OpcUa_Variant*)OpcUa_Alloc(sizeof(OpcUa_Variant));
					OpcUa_Variant_Initialize(pOutField);
					pOutField->Datatype = OpcUaType_Int32;
					pOutField->Value.NodeId = (OpcUa_NodeId*)OpcUa_Alloc(sizeof(OpcUa_NodeId));
					pOutField->Value.Int32 = OpcUa_ServerState_Unknown; // Unknown as a default value. The real value will be set in the caller method
					FastEventFieldIndex aFastEventIndexField = (*ppUAEventNotificationList)->GetFastEventFieldIndex();
					aFastEventIndexField.iSystemStateIndex = pEventNotificationList->Events[0].NoOfEventFields;
					(*ppUAEventNotificationList)->SetFastEventFieldIndex(&aFastEventIndexField);
					uStatus = OpcUa_Good;
				}
			}
			// Empty Value. Use to setup the FastEventIndexField
			if (OpcUa_String_Compare(&browsePath, &Comment) == 0)
			{
				// SystemState
				OpcUa_EventNotificationList* pEventNotificationList = (*ppUAEventNotificationList)->GetInternalEventNotificationList();
				if (pEventNotificationList)
				{
					pOutField = (OpcUa_Variant*)OpcUa_Alloc(sizeof(OpcUa_Variant));
					OpcUa_Variant_Initialize(pOutField);
					pOutField->Datatype = OpcUaType_LocalizedText;
					pOutField->Value.LocalizedText = (OpcUa_LocalizedText*)OpcUa_Alloc(sizeof(OpcUa_LocalizedText));
					OpcUa_LocalizedText_Initialize(pOutField->Value.LocalizedText);
					FastEventFieldIndex aFastEventIndexField = (*ppUAEventNotificationList)->GetFastEventFieldIndex();
					aFastEventIndexField.iComment = pEventNotificationList->Events[0].NoOfEventFields;
					(*ppUAEventNotificationList)->SetFastEventFieldIndex(&aFastEventIndexField);
					uStatus = OpcUa_Good;
				}
			}
			CUAObjectType* pEventType = pEventDefinition->GetEventType();
			if (pEventType)
			{
				OpcUa_NodeId LimitAlarmType;
				OpcUa_NodeId_Initialize(&LimitAlarmType);
				LimitAlarmType.Identifier.Numeric = 2955;
				OpcUa_NodeId* pEventTypeNodeId=pEventType->GetNodeId();
				if (pEventTypeNodeId)
				{
					// Limit can be a field only for LimitAlarm and related
					if (OpcUa_NodeId_Compare(pEventTypeNodeId, &LimitAlarmType) == 0)
					{
						// Extract from the CEventDefinition
						if (OpcUa_String_Compare(&browsePath, &LowLowLimit) == 0)
						{
							// LowLowLimit
							pOutField = (OpcUa_Variant*)OpcUa_Alloc(sizeof(OpcUa_Variant));
							OpcUa_Variant_Initialize(pOutField);// 
							pOutField->Datatype = OpcUaType_Double;
							//OpcUa_NodeId* pSourceNodeId = OpcUa_Null;
							UALimitsAlarm* pLimitsAlarm = pEventDefinition->GetLimitAlarmValues();
							if (pLimitsAlarm)
							{
								CUAVariable* pLimitAlarmVariable = pLimitsAlarm->pUAVariableLowLowLimit;
								if (pLimitAlarmVariable)
								{
									//pOutField = (OpcUa_Variant*)OpcUa_Alloc(sizeof(OpcUa_Variant));
									CDataValue* pDataValue = pLimitAlarmVariable->GetValue();
									if (pDataValue)
									{
										OpcUa_Variant limitVal = pDataValue->GetValue();
										//OpcUa_Variant_Initialize(pOutField);
										pOutField->Value.Double = limitVal.Value.Double;
										uStatus = OpcUa_Good;
									}
								}
							}
							else
								uStatus = OpcUa_BadInvalidArgument;
						}
						// Extract from the CEventDefinition
						if (OpcUa_String_Compare(&browsePath, &LowLimit) == 0)
						{
							// LowLimit

							pOutField = (OpcUa_Variant*)OpcUa_Alloc(sizeof(OpcUa_Variant));
							OpcUa_Variant_Initialize(pOutField);// 
							pOutField->Datatype = OpcUaType_Double;// 
							//OpcUa_NodeId* pSourceNodeId = OpcUa_Null;
							UALimitsAlarm* pLimitsAlarm = pEventDefinition->GetLimitAlarmValues();
							if (pLimitsAlarm)
							{
								CUAVariable* pLimitAlarmVariable = pLimitsAlarm->pUAVariableLowLimit;
								if (pLimitAlarmVariable)
								{
									//pOutField = (OpcUa_Variant*)OpcUa_Alloc(sizeof(OpcUa_Variant));
									CDataValue* pDataValue = pLimitAlarmVariable->GetValue();
									if (pDataValue)
									{
										OpcUa_Variant limitVal = pDataValue->GetValue();
										//OpcUa_Variant_Initialize(pOutField);
										//pOutField->Datatype = OpcUaType_Double;
										pOutField->Value.Double = limitVal.Value.Double;
										uStatus = OpcUa_Good;
									}
								}
							}
							else
								uStatus = OpcUa_BadInvalidArgument;
						}

						// Extract from the CEventDefinition
						if (OpcUa_String_Compare(&browsePath, &HighLimit) == 0)
						{
							// HighLimit
							pOutField = (OpcUa_Variant*)OpcUa_Alloc(sizeof(OpcUa_Variant));
							OpcUa_Variant_Initialize(pOutField);// 
							pOutField->Datatype = OpcUaType_Double;// 
							//OpcUa_NodeId* pSourceNodeId = OpcUa_Null;
							UALimitsAlarm* pLimitsAlarm = pEventDefinition->GetLimitAlarmValues();
							if (pLimitsAlarm)
							{
								CUAVariable* pLimitAlarmVariable = pLimitsAlarm->pUAVariableHighLimit;
								if (pLimitAlarmVariable)
								{
									//pOutField = (OpcUa_Variant*)OpcUa_Alloc(sizeof(OpcUa_Variant));
									CDataValue* pDataValue = pLimitAlarmVariable->GetValue();
									if (pDataValue)
									{
										OpcUa_Variant limitVal = pDataValue->GetValue();
										//OpcUa_Variant_Initialize(pOutField);
										//pOutField->Datatype = OpcUaType_Double;
										pOutField->Value.Double = limitVal.Value.Double;
										uStatus = OpcUa_Good;
									}
								}
							}
							else
								uStatus = OpcUa_BadInvalidArgument;
						}


						// Extract from the CEventDefinition
						if (OpcUa_String_Compare(&browsePath, &HighHighLimit) == 0)
						{
							// HighHighLimit
							pOutField = (OpcUa_Variant*)OpcUa_Alloc(sizeof(OpcUa_Variant));
							OpcUa_Variant_Initialize(pOutField);// 
							pOutField->Datatype = OpcUaType_Double;// 
							//OpcUa_NodeId* pSourceNodeId = OpcUa_Null;
							UALimitsAlarm* pLimitsAlarm = pEventDefinition->GetLimitAlarmValues();
							if (pLimitsAlarm)
							{
								CUAVariable* pLimitAlarmVariable = pLimitsAlarm->pUAVariableHighHighLimit;
								if (pLimitAlarmVariable)
								{
									//pOutField = (OpcUa_Variant*)OpcUa_Alloc(sizeof(OpcUa_Variant));
									CDataValue* pDataValue = pLimitAlarmVariable->GetValue();
									if (pDataValue)
									{
										OpcUa_Variant limitVal = pDataValue->GetValue();
										//OpcUa_Variant_Initialize(pOutField);
										//pOutField->Datatype = OpcUaType_Double;
										pOutField->Value.Double = limitVal.Value.Double;
										uStatus = OpcUa_Good;
									}
								}
							}
							else
								uStatus = OpcUa_BadInvalidArgument;
						}
					}

				}
				OpcUa_NodeId_Clear(&LimitAlarmType);
			}
			//if (OpcUa_String_Compare(&browsePath, &EventId) == 0)
			//{
			//}
			if (uStatus == OpcUa_Good)
			{
				// Transfert
				std::vector<OpcUa_Variant*> localEventFields;
				OpcUa_EventNotificationList* pEventNotificationList = (*ppUAEventNotificationList)->GetInternalEventNotificationList();
				if (pEventNotificationList)
				{
					OpcUa_Int32 iNoOfEventFields = pEventNotificationList->Events[0].NoOfEventFields;

					for (OpcUa_Int32 i = 0; i < iNoOfEventFields; i++)
					{
						OpcUa_Variant* pTmpVarField = (OpcUa_Variant*)OpcUa_Alloc(sizeof(OpcUa_Variant));
						OpcUa_Variant_Initialize(pTmpVarField);
						OpcUa_Variant_CopyTo(&pEventNotificationList->Events[0].EventFields[i], pTmpVarField);
						localEventFields.push_back(pTmpVarField);
						OpcUa_Variant_Clear(&pEventNotificationList->Events[0].EventFields[i]);
					}
					if (pEventNotificationList->Events[0].EventFields)
						OpcUa_Free(pEventNotificationList->Events[0].EventFields);
					// Add the new one
					localEventFields.push_back(pOutField);
					// update the size
					iNoOfEventFields = localEventFields.size();
					pEventNotificationList->Events[0].NoOfEventFields = iNoOfEventFields;
					pEventNotificationList->Events[0].EventFields = (OpcUa_Variant*)OpcUa_Alloc(sizeof(OpcUa_Variant)*iNoOfEventFields);
					for (OpcUa_Int32 i = 0; i < iNoOfEventFields; i++)
					{
						OpcUa_Variant* apOutField = localEventFields.at(i);
						OpcUa_Variant_Initialize(&(pEventNotificationList->Events[0].EventFields[i]));
						OpcUa_Variant_CopyTo(apOutField, &(pEventNotificationList->Events[0].EventFields[i]));
						//OpcUa_Variant_Clear(apOutField);
					}
				}
				else
					uStatus = OpcUa_BadInternalError;
				// Release ressources
				std::vector<OpcUa_Variant*>::iterator it;
				for (it = localEventFields.begin(); it != localEventFields.end(); it++)
				{
					OpcUa_Variant* pVarVal = *it;
					OpcUa_Variant_Clear(pVarVal);
					OpcUa_Free(pVarVal);
					pVarVal = OpcUa_Null;
				}
				localEventFields.clear();
			}
			//if (pOutField)
			//{
			//	//OpcUa_Variant_Clear(pOutField);
			//	OpcUa_Free(pOutField);
			//}
			//else
			//	OpcUa_Trace(OPCUA_TRACE_LEVEL_ERROR, "GenerateDynamicEventField browsePath not found %s\n", OpcUa_String_GetRawString(&browsePath));
		}
		else
			uStatus = OpcUa_BadInvalidArgument;
		OpcUa_String_Clear(&EventId);
		OpcUa_String_Clear(&EventType);
		OpcUa_String_Clear(&SourceNode);
		OpcUa_String_Clear(&SourceName);
		OpcUa_String_Clear(&Time);
		OpcUa_String_Clear(&ReceiveTime);
		OpcUa_String_Clear(&LocalTime);
		OpcUa_String_Clear(&Message);
		OpcUa_String_Clear(&Severity);
		OpcUa_String_Clear(&ConditionName);
		OpcUa_String_Clear(&BranchId);
		OpcUa_String_Clear(&SystemState);
		OpcUa_String_Clear(&Comment);
		OpcUa_String_Clear(&LowLimit);
		OpcUa_String_Clear(&LowLowLimit);
		OpcUa_String_Clear(&HighLimit);
		OpcUa_String_Clear(&HighHighLimit);
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	OpcUa_String_Clear(&EventId);
	return uStatus;
}
/// <summary>
/// Initializes the events fields ex.
/// </summary>
/// <param name="pEventDefinition">The pEventDefinition is an input parameter. 
///								It contains information about the event or alarm to initialize.</param>
/// <param name="ppUAEventNotificationList">The pp ua event notification list.</param>
/// <returns></returns>
OpcUa_StatusCode CMonitoredItemServer::InitializeEventsFieldsEx(CEventDefinition* pEventDefinition, CUAEventNotificationList** ppUAEventNotificationList)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	
	if (ppUAEventNotificationList)
	{
		OpcUa_NodeId BaseEventType;
		OpcUa_NodeId_Initialize(&BaseEventType);
		BaseEventType.IdentifierType = OpcUa_IdentifierType_Numeric;
		BaseEventType.Identifier.Numeric = OpcUaId_BaseEventType; // 2041;

		OpcUa_NodeId ConditionType;
		OpcUa_NodeId_Initialize(&ConditionType);
		ConditionType.IdentifierType = OpcUa_IdentifierType_Numeric;
		ConditionType.Identifier.Numeric = OpcUaId_ConditionType; // 2782;
		CUAObject* pConditionNode = pEventDefinition->GetConditionNode();
		// Get the eventfields for this CMonitoredItemServer
		vector<OpcUa_SimpleAttributeOperand*> requireFieldList = GetEventFields();
		if (requireFieldList.size() > 1)
		{
			// Prepare the extraction
			std::vector<OpcUa_Variant*> localEventFields;
			for (OpcUa_UInt32 h = 0; h < requireFieldList.size(); h++)
			{
				OpcUa_SimpleAttributeOperand* pSimpleAttributeOperand = requireFieldList.at(h);
				// A BaseEventType was require
				if (OpcUa_NodeId_Compare(&(pSimpleAttributeOperand->TypeDefinitionId), &BaseEventType) == 0)
				{
					OpcUa_Int32 i = 0;
					if (pSimpleAttributeOperand->NoOfBrowsePath == 0)
					{
						// Special case use by OPC Foundation sample client A&C
						OpcUa_Variant* pOutField=(OpcUa_Variant*)OpcUa_Alloc(sizeof(OpcUa_Variant));
						if (pSimpleAttributeOperand->AttributeId == OpcUa_Attributes_NodeId)
						{
							OpcUa_Variant_Initialize(pOutField);
							pOutField->Datatype = OpcUaType_NodeId;
							CUAObjectType* pObjectType = pEventDefinition->GetEventType();
							OpcUa_NodeId* pNodeId=pObjectType->GetNodeId();
							pOutField->Value.NodeId = (OpcUa_NodeId*)OpcUa_Alloc(sizeof(OpcUa_NodeId));
							OpcUa_NodeId_Initialize(pOutField->Value.NodeId);
							OpcUa_NodeId_CopyTo(pNodeId, pOutField->Value.NodeId);
							
							localEventFields.push_back(pOutField);
							uStatus = OpcUa_Good;
						}
						///*---------------------------------------------------------*/
						//// Transfert
						std::vector<OpcUa_Variant*> localTmpEventFields;
						OpcUa_EventNotificationList* pEventNotificationList = (*ppUAEventNotificationList)->GetInternalEventNotificationList();
						if (pEventNotificationList)
						{
							OpcUa_Int32 iNoOfEventFields = pEventNotificationList->Events[0].NoOfEventFields;

							for (OpcUa_Int32 ii = 0; ii < iNoOfEventFields; ii++)
							{
								OpcUa_Variant* pTmpVarField = (OpcUa_Variant*)OpcUa_Alloc(sizeof(OpcUa_Variant));
								OpcUa_Variant_Initialize(pTmpVarField);
								OpcUa_Variant_CopyTo(&pEventNotificationList->Events[0].EventFields[ii], pTmpVarField);
								localTmpEventFields.push_back(pTmpVarField);
								OpcUa_Variant_Clear(&pEventNotificationList->Events[0].EventFields[ii]);
							}
							if (pEventNotificationList->Events[0].EventFields)
								OpcUa_Free(pEventNotificationList->Events[0].EventFields);
							// Add the new one
							localTmpEventFields.push_back(pOutField);
							// update the size
							iNoOfEventFields = localTmpEventFields.size();
							pEventNotificationList->Events[0].NoOfEventFields = iNoOfEventFields;
							pEventNotificationList->Events[0].EventFields = (OpcUa_Variant*)OpcUa_Alloc(sizeof(OpcUa_Variant)*iNoOfEventFields);
							for (OpcUa_Int32 iii = 0; iii < iNoOfEventFields; iii++)
							{
								OpcUa_Variant* pTmpOutField = localTmpEventFields.at(iii);
								OpcUa_Variant_Initialize(&(pEventNotificationList->Events[0].EventFields[iii]));
								OpcUa_Variant_CopyTo(pTmpOutField, &(pEventNotificationList->Events[0].EventFields[iii]));
							}
						}
						else
						{
							OpcUa_Variant_Clear(pOutField);
						}
						// release ressources
						std::vector<OpcUa_Variant*>::iterator it;
						for (it = localTmpEventFields.begin(); it != localTmpEventFields.end(); it++)
						{
							OpcUa_Variant* pVarVal = *it;
							OpcUa_Variant_Clear(pVarVal);
							OpcUa_Free(pVarVal);
						}
						localTmpEventFields.clear();
						///*---------------------------------------------------------*/
					}
					else
					{
						for (i = 0; i < pSimpleAttributeOperand->NoOfBrowsePath; i++)
						{
							// We need to distinct 
							// 1- Values generated Dynamicaly
							uStatus = GenerateDynamicEventField(pSimpleAttributeOperand->BrowsePath[i].Name, ppUAEventNotificationList);
							if (uStatus == OpcUa_BadNotFound)
							{
								// 2- Value read from the NodeSet (Read in the AdddressSpace)
								uStatus = GenerateFromAddressSpaceEventField(pSimpleAttributeOperand->BrowsePath[i].Name, pSimpleAttributeOperand, ppUAEventNotificationList);
								if (uStatus == OpcUa_BadNotFound)
								{
									// Fill with an empty field
									OpcUa_Variant* pOutField= (OpcUa_Variant*)OpcUa_Alloc(sizeof(OpcUa_Variant));
									OpcUa_Variant_Initialize(pOutField);
									localEventFields.push_back(pOutField);
									uStatus = OpcUa_Good;
									///*---------------------------------------------------------*/
									//// Transfert
									std::vector<OpcUa_Variant*> localTmpEventFields;
									OpcUa_EventNotificationList* pEventNotificationList = (*ppUAEventNotificationList)->GetInternalEventNotificationList();
									if (pEventNotificationList)
									{
										OpcUa_Int32 iNoOfEventFields = pEventNotificationList->Events[0].NoOfEventFields;

										for (OpcUa_Int32 ii = 0; ii < iNoOfEventFields; ii++)
										{
											OpcUa_Variant* pTmpVarField = (OpcUa_Variant*)OpcUa_Alloc(sizeof(OpcUa_Variant));
											OpcUa_Variant_Initialize(pTmpVarField);
											OpcUa_Variant_CopyTo(&pEventNotificationList->Events[0].EventFields[ii], pTmpVarField);
											localTmpEventFields.push_back(pTmpVarField);
											OpcUa_Variant_Clear(&pEventNotificationList->Events[0].EventFields[ii]);
										}		
										if (pEventNotificationList->Events[0].EventFields)
											OpcUa_Free(pEventNotificationList->Events[0].EventFields);
										// Add the new one
										localTmpEventFields.push_back(pOutField);
										// update the size
										iNoOfEventFields = localTmpEventFields.size();
										pEventNotificationList->Events[0].NoOfEventFields = iNoOfEventFields;
										pEventNotificationList->Events[0].EventFields = (OpcUa_Variant*)OpcUa_Alloc(sizeof(OpcUa_Variant)*iNoOfEventFields);
										for (OpcUa_Int32 iii = 0; iii < iNoOfEventFields; iii++)
										{
											OpcUa_Variant* pTmpOutField = localTmpEventFields.at(iii);
											OpcUa_Variant_Initialize(&(pEventNotificationList->Events[0].EventFields[iii]));
											OpcUa_Variant_CopyTo(pTmpOutField, &pEventNotificationList->Events[0].EventFields[iii]);
											OpcUa_Variant_Clear(pTmpOutField);
										}
									}
									else
									{
										OpcUa_Variant_Clear(pOutField);
									}
									// Release ressources
									std::vector<OpcUa_Variant*>::iterator it;
									for (it = localTmpEventFields.begin(); it != localTmpEventFields.end(); it++)
									{
										OpcUa_Variant* pVarVal = *it;
										OpcUa_Variant_Clear(pVarVal);
										OpcUa_Free(pVarVal);
									}
									localTmpEventFields.clear();
									///*---------------------------------------------------------*/
								}
							}
							// workaround
							if (pSimpleAttributeOperand->NoOfBrowsePath >1)
								i += (pSimpleAttributeOperand->NoOfBrowsePath - 1);
						}
					}
				}
				if (OpcUa_NodeId_Compare(&(pSimpleAttributeOperand->TypeDefinitionId), &ConditionType) == 0)
				{
					if (pConditionNode)
					{
						OpcUa_Variant* pOutField= (OpcUa_Variant*)OpcUa_Alloc(sizeof(OpcUa_Variant));
						OpcUa_Variant_Initialize(pOutField);
						//OpcUa_String sourceName = GetUABase()->GetBrowseName()->Name;
						pOutField->Datatype = OpcUaType_NodeId;
						pOutField->Value.NodeId = (OpcUa_NodeId*)OpcUa_Alloc(sizeof(OpcUa_NodeId));;
						OpcUa_NodeId_Initialize(pOutField->Value.NodeId);
						OpcUa_NodeId* pTmpNodeId = pConditionNode->GetNodeId();
						OpcUa_NodeId_CopyTo(pTmpNodeId, pOutField->Value.NodeId);	
						/*---------------------------------------------------------*/
						// Transfert
						std::vector<OpcUa_Variant*> localTmpEventFields;
						OpcUa_EventNotificationList* pEventNotificationList = (*ppUAEventNotificationList)->GetInternalEventNotificationList();
						if (pEventNotificationList)
						{
							OpcUa_Int32 iNoOfEventFields = pEventNotificationList->Events[0].NoOfEventFields;

							for (OpcUa_Int32 ii = 0; ii < iNoOfEventFields; ii++)
							{
								OpcUa_Variant* pTmpVarField = (OpcUa_Variant*)OpcUa_Alloc(sizeof(OpcUa_Variant));
								OpcUa_Variant_Initialize(pTmpVarField);
								OpcUa_Variant_CopyTo(&pEventNotificationList->Events[0].EventFields[ii], pTmpVarField);
								localTmpEventFields.push_back(pTmpVarField);
								OpcUa_Variant_Clear(&pEventNotificationList->Events[0].EventFields[ii]);
							}
							if (pEventNotificationList->Events[0].EventFields)
								OpcUa_Free(pEventNotificationList->Events[0].EventFields);
							// Add the new one
							localTmpEventFields.push_back(pOutField);
							// update the size
							iNoOfEventFields = localTmpEventFields.size();
							pEventNotificationList->Events[0].NoOfEventFields = iNoOfEventFields;
							pEventNotificationList->Events[0].EventFields = (OpcUa_Variant*)OpcUa_Alloc(sizeof(OpcUa_Variant)*iNoOfEventFields);
							if (pEventNotificationList->Events[0].EventFields)
							{
								for (OpcUa_Int32 iii = 0; iii < iNoOfEventFields; iii++)
								{
									OpcUa_Variant* apTmpOutField = localTmpEventFields.at(iii);
									OpcUa_Variant_Initialize(&(pEventNotificationList->Events[0].EventFields[iii]));
									OpcUa_Variant_CopyTo(apTmpOutField, &(pEventNotificationList->Events[0].EventFields[iii]));
								}
							}
						}
						else
						{
							OpcUa_Variant_Clear(pOutField);
							OpcUa_Free(pOutField);
							uStatus = OpcUa_BadInternalError;
						}
						// Release ressources
						std::vector<OpcUa_Variant*>::iterator it;
						for (it = localTmpEventFields.begin(); it != localTmpEventFields.end(); it++)
						{
							OpcUa_Variant* pVarVal = *it;
							OpcUa_Variant_Clear(pVarVal);
							OpcUa_Free(pVarVal);
						}
						localTmpEventFields.clear();
						/*---------------------------------------------------------*/
					}
				}
			}
		}
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}

/// <summary>
/// Gets the alarm field in pConditionNode for the attribute pointed by pSimpleAttributeOperand.
/// The value of the Attribute is read in the addressSpace and put in pValue
/// </summary>
/// <param name="pConditionNode">(In) This is the NodeSet definition of the Alarm.</param>
/// <param name="pSimpleAttributeOperand">(In) The browseName of the attribute to extract from the pConditionNode.</param>
/// <param name="pValue"> (Out) The Value of the extracted attribute.</param>
/// <returns></returns>
OpcUa_StatusCode CMonitoredItemServer::GetAlarmField(CUAObject* pConditionNode, OpcUa_SimpleAttributeOperand* pSimpleAttributeOperand, OpcUa_Variant** pValue)
{
	OpcUa_StatusCode uStatus = OpcUa_Bad;
	OpcUa_UInt32 uiNoOfVariant = 0;
	OpcUa_String FieldName;
	OpcUa_String_Initialize(&FieldName);
	CUAInformationModel* pInformationModel = CServerApplication::m_pTheAddressSpace;
	// search in the reference the reference with the targetName == pFieldName
	if (pConditionNode)
	{
		if (pSimpleAttributeOperand)
		{
			uiNoOfVariant = pSimpleAttributeOperand->NoOfBrowsePath;
			if ( uiNoOfVariant > 0)
			{
				(*pValue) = (OpcUa_Variant*)OpcUa_Alloc(sizeof(OpcUa_Variant)*uiNoOfVariant);
				CUAReferenceList* pRefList = pConditionNode->GetReferenceNodeList();
				if (pRefList)
				{
					OpcUa_Boolean bFirstOk = OpcUa_False;
					for (OpcUa_UInt32 i = 0; i < pRefList->size(); i++)
					{
						CUAReference* pReference = pRefList->at(i);
						OpcUa_NodeId aTargetId = pReference->GetTargetId().NodeId;
						CUAVariable* pUAVariable = OpcUa_Null;
						if (pInformationModel->GetNodeIdFromVariableList(aTargetId, &pUAVariable) == OpcUa_Good)
						{
							OpcUa_QualifiedName* pQualName = pUAVariable->GetBrowseName();
							if (pQualName)
							{
								if (pSimpleAttributeOperand->NoOfBrowsePath == 1)
								{
									OpcUa_String_CopyTo(&(pSimpleAttributeOperand->BrowsePath[0].Name), &FieldName);
									if (OpcUa_String_Compare(&(pQualName->Name), &FieldName) == 0)
									{
										OpcUa_Variant aVariant = pUAVariable->GetValue()->GetValue();
										OpcUa_Variant_Initialize(&(*pValue)[0]);
										OpcUa_Variant_CopyTo(&aVariant, &((*pValue)[0]));
										uStatus = OpcUa_Good;
										break;
									}
								}
								else
								{
									if (pSimpleAttributeOperand->NoOfBrowsePath == 2)
									{
										OpcUa_String_Clear(&FieldName);
										OpcUa_String_Initialize(&FieldName);
										OpcUa_String_CopyTo(&(pSimpleAttributeOperand->BrowsePath[0].Name), &FieldName);
										if (OpcUa_String_Compare(&(pQualName->Name), &FieldName) == 0)
										{
											OpcUa_Variant aVariant = pUAVariable->GetValue()->GetValue();
											OpcUa_Variant_Initialize(&(*pValue)[0]);
											OpcUa_Variant_CopyTo(&aVariant, &((*pValue)[0]));
											bFirstOk = OpcUa_True;
										}
										if (bFirstOk)
										{
											CUAReferenceList* pEmbeddedRefList = pUAVariable->GetReferenceNodeList();
											if (pEmbeddedRefList)
											{
												OpcUa_String_Clear(&FieldName);
												OpcUa_String_Initialize(&FieldName);
												OpcUa_String_CopyTo(&(pSimpleAttributeOperand->BrowsePath[1].Name), &FieldName);
												for (OpcUa_UInt32 ii = 0; ii < pEmbeddedRefList->size(); ii++)
												{
													CUAReference* pEmbeddedReference = pEmbeddedRefList->at(ii);
													OpcUa_NodeId aEmbeddedTargetId = pEmbeddedReference->GetTargetId().NodeId;
													CUAVariable* pEmbeddedUAVariable = OpcUa_Null;
													if (pInformationModel->GetNodeIdFromVariableList(aEmbeddedTargetId, &pEmbeddedUAVariable) == OpcUa_Good)
													{
														OpcUa_QualifiedName* pEmbeddedQualName = pEmbeddedUAVariable->GetBrowseName();
														if (pEmbeddedQualName)
														{
															if (OpcUa_String_Compare(&(pEmbeddedQualName->Name), &FieldName) == 0)
															{
																OpcUa_Variant aVariant = pEmbeddedUAVariable->GetValue()->GetValue();
																OpcUa_Variant_Initialize(&(*pValue)[1]);
																OpcUa_Variant_CopyTo(&aVariant, &((*pValue)[1]));
																uStatus = OpcUa_Good;
																OpcUa_String_Clear(&FieldName);
																return uStatus;
															}
														}
													}
												}
												OpcUa_String_Clear(&FieldName);
											}
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
			}
			else
				uStatus = OpcUa_BadInvalidArgument;
		}
		else
			uStatus = OpcUa_BadInvalidArgument;
	}
	OpcUa_String_Clear(&FieldName);
	return uStatus;
}
OpcUa_StatusCode CMonitoredItemServer::GetDataTypeForBrowsePath(CUAObjectType* pObjectType, OpcUa_QualifiedName qualifiedName, OpcUa_NodeId* dataType)
{

	OpcUa_StatusCode uStatus = OpcUa_BadNotFound;
	CUAInformationModel* pInformationModel = CServerApplication::m_pTheAddressSpace;
	if (pObjectType)
	{
		CUAReferenceList* pReferences = pObjectType->GetReferenceNodeList();
		for (OpcUa_UInt32 hh = 0; hh < pReferences->size(); hh++)
		{
			CUAReference* pReference = pReferences->at(hh);
			CUABase* pUABase = OpcUa_Null;
			OpcUa_ExpandedNodeId aExpNodeId = pReference->GetTargetId();
			if (pInformationModel->GetNodeIdFromDictionnary(aExpNodeId.NodeId, &pUABase) == OpcUa_Good)
			{
				OpcUa_QualifiedName* pQual2 = pUABase->GetBrowseName();
				if (OpcUa_QualifiedName_Compare(&qualifiedName, pQual2) == 0)
				{
					// now pUABase->GetNodeId() must target a UAVariable
					OpcUa_NodeId* pNodeId = pUABase->GetNodeId();
					CUAVariable* pUAVariable = OpcUa_Null;
					if (pInformationModel->GetNodeIdFromVariableList(*pNodeId, &pUAVariable) == OpcUa_Null)
					{
						(*dataType) = pUAVariable->GetDataType();
						uStatus = OpcUa_Good;
						break;
					}
				}
			}
		}
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}
OpcUa_DataChangeFilter*	CMonitoredItemServer::GetDataChangeFilter() 
{ 
	return m_pDataChangeFilter; 
}
void CMonitoredItemServer::SetDataChangeFilter(OpcUa_DataChangeFilter* pDataChangeFilter) 
{ 
	if (pDataChangeFilter)
	{
		m_pDataChangeFilter = Utils::Copy(pDataChangeFilter); 
	}
}
CSubscriptionServer* CMonitoredItemServer::GetSubscription() 
{ 
	return	m_pSubscription; 
}
void CMonitoredItemServer::SetSubscription(CSubscriptionServer* pSubscription) 
{ 
	m_pSubscription = pSubscription; 
}

OpcUa_Double CMonitoredItemServer::GetSamplingInterval() 
{ 
	return m_samplingInterval; 
}
void CMonitoredItemServer::SetSamplingInterval(OpcUa_Double dblVal) 
{ 
	m_samplingInterval = dblVal; 
}
OpcUa_ExtensionObject*	CMonitoredItemServer::GetMonitoringFilter()
{
	return	m_pMonitoringFilter;
}
void CMonitoredItemServer::SetMonitoringFilter(OpcUa_ExtensionObject* ExObjVal)
{
	if (ExObjVal)
	{
		if (!m_pMonitoringFilter)
			m_pMonitoringFilter = (OpcUa_ExtensionObject*)OpcUa_Alloc(sizeof(OpcUa_ExtensionObject));
		else
			OpcUa_ExtensionObject_Clear(m_pMonitoringFilter);
		OpcUa_ExtensionObject_Initialize(m_pMonitoringFilter);
		OpcUa_ExtensionObject_CopyTo(ExObjVal, m_pMonitoringFilter);
	}
}
void CMonitoredItemServer::SetMonitoringFilter(OpcUa_ExtensionObject ExObjVal)
{
	if (!m_pMonitoringFilter)
		m_pMonitoringFilter = (OpcUa_ExtensionObject*)OpcUa_Alloc(sizeof(OpcUa_ExtensionObject));
	else
		OpcUa_ExtensionObject_Clear(m_pMonitoringFilter);
	OpcUa_ExtensionObject_Initialize(m_pMonitoringFilter);
	OpcUa_ExtensionObject_CopyTo(&ExObjVal, m_pMonitoringFilter);
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Adds a triggered item identifier. </summary>
///
/// <remarks>	Michel, 16/05/2016. </remarks>
///
/// <param name="ItemId">	Identifier for the item. </param>
///-------------------------------------------------------------------------------------------------

void OpenOpcUa::UACoreServer::CMonitoredItemServer::AddTriggeredItemId(CMonitoredItemServer* pMonitoredItemServer)
{
	OpcUa_Mutex_Lock(m_TriggeredItemIdListMutex);
	m_TriggeredItemIdList.push_back(pMonitoredItemServer);
	OpcUa_Mutex_Unlock(m_TriggeredItemIdListMutex);
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Removes the triggered item identifier described by ItemId. </summary>
///
/// <remarks>	Michel, 16/05/2016. </remarks>
///
/// <param name="ItemId">	Identifier for the item. </param>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode OpenOpcUa::UACoreServer::CMonitoredItemServer::RemoveTriggeredItemId(CMonitoredItemServer* pMonitoredItemServer)
{
	OpcUa_StatusCode uStatus = OpcUa_BadMonitoredItemIdInvalid;
	OpcUa_Mutex_Lock(m_TriggeredItemIdListMutex);
	std::vector<CMonitoredItemServer*>::iterator it;
	for (it = m_TriggeredItemIdList.begin(); it != m_TriggeredItemIdList.end(); it++)
	{
		CMonitoredItemServer* pTriggeredItem = *it;
		if (pMonitoredItemServer == pTriggeredItem)
		{
			m_TriggeredItemIdList.erase(it);
			uStatus = OpcUa_Good;
			break;
		}
	}
	OpcUa_Mutex_Unlock(m_TriggeredItemIdListMutex);	
	return uStatus;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Check if this MonitoredItem trigger other. </summary>
///
/// <remarks>	Michel, 17/05/2016. </remarks>
///
/// <returns>	An OpcUa_Boolean. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_Boolean OpenOpcUa::UACoreServer::CMonitoredItemServer::IsTriggeredItemId(void)
{
	OpcUa_Boolean bResult = OpcUa_True;
	OpcUa_Mutex_Lock(m_TriggeredItemIdListMutex);
	bResult = (OpcUa_Boolean)!m_TriggeredItemIdList.empty();
	OpcUa_Mutex_Unlock(m_TriggeredItemIdListMutex);	
	return bResult;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Gets triggered monitored item notification list. </summary>
///
/// <remarks>	Michel, 17/05/2016. </remarks>
///
/// <returns>	null if it fails, else the triggered monitored item notification list. </returns>
///-------------------------------------------------------------------------------------------------

vector<OpcUa_MonitoredItemNotification*> OpenOpcUa::UACoreServer::CMonitoredItemServer::GetTriggeredMonitoredItemNotificationList(void)
{
	vector<OpcUa_MonitoredItemNotification*> triggeredMonitoredItemNotification;
	OpcUa_Mutex_Lock(m_TriggeredItemIdListMutex);
	for (OpcUa_UInt32 i = 0; i < m_TriggeredItemIdList.size(); i++)
	{
		OpcUa_MonitoredItemNotification* pMonitoredItemNotification = (OpcUa_MonitoredItemNotification*)OpcUa_Alloc(sizeof(OpcUa_MonitoredItemNotification));
		CMonitoredItemBase* pTriggerItemServer = m_TriggeredItemIdList.at(i);
		pMonitoredItemNotification->ClientHandle = pTriggerItemServer->GetClientHandle();
		OpcUa_DataValue* pDataValue = pTriggerItemServer->GetValue();
		if (pDataValue)
		{
			OpcUa_DataValue_CopyTo(pDataValue, &(pMonitoredItemNotification->Value));
			triggeredMonitoredItemNotification.push_back(pMonitoredItemNotification);
		}
		else
			OpcUa_Free(pMonitoredItemNotification);
	}
	OpcUa_Mutex_Unlock(m_TriggeredItemIdListMutex);	
	return triggeredMonitoredItemNotification;
}

CMonitoredItemServer* CMonitoredItemServer::operator=(CMonitoredItemServer* pItem)
{
	m_pSubscription = pItem->GetSubscription();
	m_attributeId = pItem->m_attributeId;
	m_bChange = pItem->m_bChange;
	m_bDiscardOldest = pItem->m_bDiscardOldest;
	m_bOverflow = pItem->m_bOverflow;
	m_ClassName = pItem->m_ClassName;//
	m_clientHandle = pItem->m_clientHandle;
	OpcUa_QualifiedName_CopyTo(&(pItem->m_dataEncoding), &m_dataEncoding);//
	pItem->m_eTimestampsToReturn = m_eTimestampsToReturn;
	//pItem->m_events;// TODO
	//OpcUa_ExtensionObject_CopyTo(&(pItem->m_filterToUse),&m_filterToUse);//
	m_iDiagnosticsMasks = pItem->m_iDiagnosticsMasks;
	OpcUa_String_CopyTo(pItem->m_pIndexRange, m_pIndexRange);
	m_IsCold = pItem->m_IsCold;
	m_lastError = pItem->m_lastError;
	m_LastNotificationTime = pItem->m_LastNotificationTime;//
	m_MonitoredItemId = pItem->m_MonitoredItemId;
	OpcUa_String_CopyTo(pItem->m_pMonitoredItemName, m_pMonitoredItemName);//
	OpcUa_NodeId_CopyTo(&(pItem->m_NodeId), &m_NodeId);//

	//pItem->m_parsedIndexRange;// TODO
	//pItem->m_pBase;// TODO
	//pItem->m_pDataChangeFilter;// TODO
	//pItem->m_pEventFilter;//TODO
	//pItem->m_pMonitoredItemCS;// TODO
	//pItem->m_pMonitoringFilter;// TODO

	m_pUserData = pItem->m_pUserData;
	m_queueSize = pItem->m_queueSize;
	m_samplingInterval = pItem->m_samplingInterval;
	m_StatusCode = pItem->m_StatusCode;
	OpcUa_DataValue_CopyTo(pItem->m_pValue, m_pValue);
	return this;
}
OpcUa_NodeClass	CMonitoredItemServer::GetNodeClass()
{
	if (m_pBase)
		return m_pBase->GetNodeClass();
	else
		return OpcUa_NodeClass_Unspecified;
}
OpcUa_QualifiedName* CMonitoredItemServer::GetBrowseName()
{
	if (m_pBase)
		return m_pBase->GetBrowseName();
	else
		return OpcUa_Null;
}
OpcUa_LocalizedText CMonitoredItemServer::GetDisplayName()
{
	return m_pBase->GetDisplayName();

}
OpcUa_LocalizedText CMonitoredItemServer::GetDescription()
{
	return m_pBase->GetDescription();
}
OpcUa_UInt32 CMonitoredItemServer::GetWriteMask()
{
	if (m_pBase)
		return m_pBase->GetWriteMask();
	else
		return 0;
}
OpcUa_UInt32 CMonitoredItemServer::GetUserWriteMask()
{
	if (m_pBase)
		return m_pBase->GetUserWriteMask();
	else
		return 0;
}
void CMonitoredItemServer::SetCold(OpcUa_Boolean bVal) 
{ 
	m_IsCold = bVal; 
}
OpcUa_Boolean CMonitoredItemServer::IsCold() 
{ 
	return m_IsCold; 
}
OpcUa_DateTime	CMonitoredItemServer::GetLastNotificationTime() 
{ 
	return m_LastNotificationTime; 
}
void CMonitoredItemServer::SetLastNotificationTime(OpcUa_DateTime dtVal) 
{ 
	m_LastNotificationTime = dtVal; 
}
CUABase* CMonitoredItemServer::GetUABase() 
{ 
	return m_pBase; 
}
void CMonitoredItemServer::SetUABase(CUABase*	pBase)
{ 
	m_pBase = pBase; 
}
OpcUa_StatusCode CMonitoredItemServer::GetStatusCode() 
{ 
	return m_StatusCode; 
}
void CMonitoredItemServer::SetStatusCode(OpcUa_StatusCode uStatus)
{
	m_StatusCode = uStatus;
}

vector<OpcUa_SimpleAttributeOperand*> CMonitoredItemServer::GetEventFields() 
{ 
	return m_EventFields; 
}