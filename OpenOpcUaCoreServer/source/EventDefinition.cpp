#include "stdafx.h"
#include "UAReferenceType.h"
#include "UAObjectType.h"
#include "UAVariable.h"
#include "Field.h"
#include "Definition.h"
#include "UADataType.h"
#include "UAVariableType.h"
#include "UAMethod.h"
#include "UAView.h"
#include "UAObject.h"
#include "EventDefinition.h"

using namespace OpenOpcUa;
using namespace UAEvents;
using namespace UAAddressSpace;
using namespace UACoreServer;
CEventDefinition::CEventDefinition()
{
	SetActiveState(OpcUa_False);
	m_pAckedStateUAVariable = OpcUa_Null;
	m_pAckedStateIdUAVariable = OpcUa_Null;
	Enable(OpcUa_True); // by default the CEventDefinition is enabled
	OnStage(OpcUa_False);
	m_pRelatedActiveState = OpcUa_Null;
	m_pRelatedActiveStateId = OpcUa_Null;
	m_pRelatedActiveStateEffectiveDisplayName = OpcUa_Null;
	m_pRetainVariable = OpcUa_Null;
	m_pRelatedEnabledStateUAVariable = OpcUa_Null;
	m_pRelatedEnabledStateUAVariableId = OpcUa_Null;
	m_pRelatedEnableMethod = OpcUa_Null;
	m_pRelatedDisableMethod = OpcUa_Null;
	m_pConfirmedStateUAVariable = OpcUa_Null;
	m_pSourceName = OpcUa_Null;
	m_pSourceNode = OpcUa_Null;
	m_pEventMessage = OpcUa_Null;
	m_pMonitoringObject = OpcUa_Null;
	// LimitAlarm initialization
	m_pLimitAlarmValues = (UALimitsAlarm*)OpcUa_Alloc(sizeof(UALimitsAlarm));
	m_pLimitAlarmValues->pUAVariableHighHighLimit = OpcUa_Null;
	m_pLimitAlarmValues->pUAVariableHighLimit = OpcUa_Null;
	m_pLimitAlarmValues->pUAVariableLowLimit = OpcUa_Null;
	m_pLimitAlarmValues->pUAVariableLowLowLimit = OpcUa_Null;
	m_pEventMessage = OpcUa_Null;
}

/// <summary>
/// Initializes a new instance of the <see cref="CEventDefinition"/> class.
/// </summary>
/// <param name="pMonitoringObject">The p monitoring object.</param>
/// <param name="pConditionNode">This is the Alarm definition. So called the condition node.</param>
/// <param name="pSourceNode">The p source node.</param>
/// <param name="pEventType">Type of the p event.</param>
CEventDefinition::CEventDefinition(CUABase* pMonitoringObject,
								CUAObject* pConditionNode,
								CUAVariable*	pSourceNode,
								CUAObjectType*	pEventType)
{
	m_pMonitoringObject=pMonitoringObject; 
	m_pConditionNode=pConditionNode; 
	m_pSourceNode=pSourceNode; 
	m_pEventType=pEventType;
	m_bActiveState = OpcUa_False;
	m_bPreviousActiveState=OpcUa_True;
	// let's initialize according to m_pSourceNode
	if (m_pSourceNode)
	{
		OpcUa_Byte builtInType = m_pSourceNode->GetBuiltInType();
		switch (builtInType)
		{
		case OpcUaType_Boolean:
			SetActiveState(m_pSourceNode->GetValue()->GetValue().Value.Boolean);
			break;
		default:
			SetActiveState(OpcUa_False);
			break;
		}		 		
	}
	else
		SetActiveState(OpcUa_False);
	// End initialzation
	Enable(OpcUa_True); // by default the CEventDefinition is enabled
	OnStage(OpcUa_False);
	//
	m_pAckedStateUAVariable = OpcUa_Null;
	m_pAckedStateIdUAVariable = OpcUa_Null;
	m_pRelatedActiveState = OpcUa_Null;
	m_pRelatedActiveStateId = OpcUa_Null;
	m_pRelatedActiveStateEffectiveDisplayName = OpcUa_Null;
	m_pRetainVariable = OpcUa_Null;
	m_pRelatedEnabledStateUAVariable = OpcUa_Null;
	m_pRelatedEnabledStateUAVariableId = OpcUa_Null;
	m_pConfirmedStateUAVariable = OpcUa_Null;
	m_pSourceName = OpcUa_Null;
	m_pEventMessage = OpcUa_Null;
	// Let retrieve the SourceName based on the SourceNode BrowseName
	if (pSourceNode)
	{
		SetSourceName(pSourceNode);
	}
	// LimitAlarm initialization
	m_pLimitAlarmValues = (UALimitsAlarm*)OpcUa_Alloc(sizeof(UALimitsAlarm));
	m_pLimitAlarmValues->pUAVariableHighHighLimit = OpcUa_Null;
	m_pLimitAlarmValues->pUAVariableHighLimit = OpcUa_Null;
	m_pLimitAlarmValues->pUAVariableLowLimit = OpcUa_Null;
	m_pLimitAlarmValues->pUAVariableLowLowLimit = OpcUa_Null;
}

CEventDefinition::~CEventDefinition()
{ 
	OpcUa_Free(m_pLimitAlarmValues);
	m_pLimitAlarmValues = OpcUa_Null;
}
CUAVariable*	CEventDefinition::GetSourceNode() 
{ 
	return m_pSourceNode; 
}
/// <summary>
/// Evaluates the State machine for the CEventDefinition
/// This will change the ActiveState and related attribute
/// </summary>
/// <returns></returns>
OpcUa_StatusCode CEventDefinition::EvaluateStateMachine()
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_NodeId OffNormalAlarmType;
	OpcUa_NodeId_Initialize(&OffNormalAlarmType);
	OffNormalAlarmType.Identifier.Numeric = 10637;
	OpcUa_NodeId LimitAlarmType;
	OpcUa_NodeId_Initialize(&LimitAlarmType);
	LimitAlarmType.Identifier.Numeric = 2955;

	if ((m_pRelatedActiveState) && (m_pRelatedActiveStateId) )
	{
		CDataValue* pDataValueActiveState = m_pRelatedActiveState->GetValue();
		CDataValue* pDataValueActiveStateId = m_pRelatedActiveStateId->GetValue();
		CDataValue* pDataValueAckedStateId = m_pAckedStateIdUAVariable->GetValue();
		CDataValue* pDataValueActiveStateEffectiveDisplayName = OpcUa_Null;
		if (m_pRelatedActiveStateEffectiveDisplayName)
			pDataValueActiveStateEffectiveDisplayName =m_pRelatedActiveStateEffectiveDisplayName->GetValue();
		// Is there are a change
		if (m_pSourceNode)
		{
			CDataValue* pDataValue = m_pSourceNode->GetValue();
			// What is the machine state for the m_pEventType ?
			OpcUa_NodeId* pEventType = m_pEventType->GetNodeId();
			if (OpcUa_NodeId_Compare(&LimitAlarmType, pEventType) == 0)
			{
				if (m_pLimitAlarmValues)
				{
					// Extract the current value in the sourceNode. First we transform it in a double
					// in case the sourceNode contains something else
					OpcUa_Double dblSourceNodeValue;
					if (pDataValue->GetAsDouble(&dblSourceNodeValue) == OpcUa_Good)
					{
						OpcUa_Double dblLowLowLimit = m_pLimitAlarmValues->pUAVariableLowLowLimit->GetValue()->GetValue().Value.Double;
						OpcUa_Double dblLowLimit = m_pLimitAlarmValues->pUAVariableLowLimit->GetValue()->GetValue().Value.Double;
						OpcUa_Double dblHighLimit = m_pLimitAlarmValues->pUAVariableHighLimit->GetValue()->GetValue().Value.Double;
						OpcUa_Double dblHighHighLimit = m_pLimitAlarmValues->pUAVariableHighHighLimit->GetValue()->GetValue().Value.Double;
						// Check LowLowLimit 
						if ( dblSourceNodeValue<dblLowLowLimit)
						{
							// Trigger a lowlowAlarm
							m_CurrentThreshold = LimitAlarmThreshold::LowLowThreshold;
							SetActiveState(OpcUa_True);
							TurnToActive();
							if (GetTransactionStatus()&OPCUA_EVENTSTATUS_SENDTOCLIENT) 
								SetTransactionStatus(OPCUA_EVENTSTATUS_TRANSITION);
							else
								SetTransactionStatus(OPCUA_EVENTSTATUS_NEWSTATUS);
						}
						else
						{
							// Check LowLowLimit -> LowLimit 
							if ((dblSourceNodeValue<dblLowLimit) && (dblSourceNodeValue>dblLowLowLimit))
							{
								// Trigger a lowAlarm
								m_CurrentThreshold = LimitAlarmThreshold::LowThreshold;
								SetActiveState(OpcUa_True);
								TurnToActive();
								if (GetTransactionStatus()&OPCUA_EVENTSTATUS_SENDTOCLIENT)
									SetTransactionStatus(OPCUA_EVENTSTATUS_TRANSITION);
								else
									SetTransactionStatus(OPCUA_EVENTSTATUS_NEWSTATUS);
							}
							else
							{
								// Check LowLimit -> HighLimit (Normal state)
								if ((dblSourceNodeValue>dblLowLimit) && (dblSourceNodeValue<dblHighLimit))
								{
									m_CurrentThreshold = LimitAlarmThreshold::NormalThreshold;
									// normal state. Nothing to trigger (End Alarm)
									SetActiveState(OpcUa_False);
									TurnToInactive();
								}
								else
								{
									// Check HighLimit -> HighHighLimit 
									if ((dblSourceNodeValue>dblHighLimit) && (dblSourceNodeValue<dblHighHighLimit))
									{
										// Trigger a HighAlarm
										m_CurrentThreshold = LimitAlarmThreshold::HighThreshold;
										SetActiveState(OpcUa_True);
										TurnToActive();
										// Force the notification. In case we switch from a previousAlarm State
										if (GetTransactionStatus()&OPCUA_EVENTSTATUS_SENDTOCLIENT)
											SetTransactionStatus(OPCUA_EVENTSTATUS_TRANSITION);
										else
											SetTransactionStatus(OPCUA_EVENTSTATUS_NEWSTATUS);
									}
									else
									{
										// Check HighHighLimit
										if (dblSourceNodeValue>dblHighHighLimit)
										{
											// Trigger a HighHighAlarm
											m_CurrentThreshold = LimitAlarmThreshold::HighHighThreshold;
											SetActiveState(OpcUa_True);
											TurnToActive();
											if (GetTransactionStatus()&OPCUA_EVENTSTATUS_SENDTOCLIENT)
												SetTransactionStatus(OPCUA_EVENTSTATUS_TRANSITION);
											else
												SetTransactionStatus(OPCUA_EVENTSTATUS_NEWSTATUS);
										}
										else
										{
											m_CurrentThreshold = LimitAlarmThreshold::NormalThreshold;
											SetActiveState(OpcUa_False);
											TurnToInactive();
										}
									}
								}
							}
						}
					}	
				}
			}
			else
			{
				if (OpcUa_NodeId_Compare(&OffNormalAlarmType, pEventType) == 0)
				{
					OpcUa_Boolean bSourceState = pDataValue->GetValue().Value.Boolean;
					//if (GetPreviousActiveState() != GetActiveState())
					{
						if (m_pSourceNode)
						{
							SetActiveState(bSourceState);
							if (bSourceState == (OpcUa_Boolean)OpcUa_True)
								TurnToActive();
							else
								TurnToInactive();
						}
					}
				}
				else
				{
					char* szNodeId = OpcUa_Null;
					Utils::NodeId2String(pEventType, &szNodeId);
					// Attribute
					if (szNodeId)
					{
						OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "The EventType %s  is not supported by this version of OpenOpcUa. Please contact the core team\n", szNodeId);
						free(szNodeId);
					}
				}
			}
		}
	}
	
	return uStatus;
}
OpcUa_Boolean	 CEventDefinition::GetActiveState() 
{ 
	return m_bActiveState; 
}
void CEventDefinition::SetActiveState(OpcUa_Boolean bVal)
{
	SetPreviousActiveState(m_bActiveState);
	m_bActiveState = bVal;
}
void CEventDefinition::SetPreviousActiveState(OpcUa_Boolean bVal) 
{ 
	m_bPreviousActiveState = bVal; 
}
OpcUa_Boolean	CEventDefinition::GetPreviousActiveState() 
{ 
	return m_bPreviousActiveState; 
}
/// <summary>
/// Determines whether this instance is virtual. 
/// A CEventDefinition is consider Virtual is there are no MonitoringObject ConditionNode SourceNode
/// associated with it. Such Event are generaly pure Event like RefreshStarteventType, RefreshEndEventType, etc.
/// Virtual EventDefinition are regulary remove from the EventDefinitionList
/// </summary>
/// <returns>A boolean to indicate if the CEventDefinition is virtual</returns>
OpcUa_Boolean CEventDefinition::IsVirtual()
{
	OpcUa_Boolean bResult=OpcUa_False;
	if ((m_pMonitoringObject == OpcUa_Null) && (m_pConditionNode == OpcUa_Null) && (m_pSourceNode == OpcUa_Null))
		bResult=OpcUa_True;
	return bResult;
}

void CEventDefinition::Enable(OpcUa_Boolean bVal) 
{ 
	m_bEnable = bVal; 
}
OpcUa_Boolean CEventDefinition::IsEnable() 
{ 
	return m_bEnable; 
}

OpcUa_Boolean	CEventDefinition::IsOnStage() 
{ 
	return m_bOnStage; 
}
void CEventDefinition::OnStage(OpcUa_Boolean bVal) 
{ 
	m_bOnStage = bVal; 
}

CUAVariable*	CEventDefinition::GetSourceName() 
{ 
	return m_pSourceName; 
}
void CEventDefinition::SetSourceName(CUAVariable* pVal) 
{ 
	m_pSourceName = pVal; 
}

CUAMethod* CEventDefinition::GetEnableMethod() 
{ 
	return m_pRelatedEnableMethod; 
}
void CEventDefinition::SetEnableMethod(CUAMethod* pMethod) 
{ 
	m_pRelatedEnableMethod = pMethod; 
}
CUAMethod* CEventDefinition::GetDisableMethod() 
{ 
	return m_pRelatedDisableMethod; 
}
void CEventDefinition::SetDisableMethod(CUAMethod* pMethod) 
{ 
	m_pRelatedDisableMethod = pMethod; 
}

CUAVariable* CEventDefinition::GetConfirmedStateUAVariable() 
{ 
	return m_pConfirmedStateUAVariable; 
}
void CEventDefinition::SetConfirmedStateUAVariable(CUAVariable* pConfirmedStateUAVariable) 
{ 
	m_pConfirmedStateUAVariable = pConfirmedStateUAVariable; 
}
CUAVariable* CEventDefinition::GetConfirmedStateIdUAVariable() 
{ 
	return m_pConfirmedStateIdUAVariable; 
}
void CEventDefinition::SetConfirmedStateIdUAVariable(CUAVariable* pConfirmedStateIdUAVariable) 
{ 
	m_pConfirmedStateIdUAVariable = pConfirmedStateIdUAVariable; 
}

void CEventDefinition::SetRelatedActiveStateEffectiveDisplayName(CUAVariable* pUAVariable) 
{ 
	m_pRelatedActiveStateEffectiveDisplayName = pUAVariable; 
}
void CEventDefinition::SetRelativeEnabledStateUAVariable(CUAVariable* pUAVariable) 
{ 
	m_pRelatedEnabledStateUAVariable = pUAVariable; 
}
CUAVariable* CEventDefinition::GetRelativeEnabledStateUAVariable()
{
	return m_pRelatedEnabledStateUAVariable;
}
void CEventDefinition::SetRelativeEnabledStateIdUAVariable(CUAVariable* pUAVariable) 
{ 
	m_pRelatedEnabledStateUAVariableId = pUAVariable; 
}
CUAVariable* CEventDefinition::GetRelativeEnabledStateIdUAVariable() 
{ 
	return m_pRelatedEnabledStateUAVariableId; 
}

CUAVariable* CEventDefinition::GetRelatedActiveState() 
{ 
	return m_pRelatedActiveState; 
}
void CEventDefinition::SetRelatedActiveState(CUAVariable* pUAVariable) 
{ 
	m_pRelatedActiveState = pUAVariable; 
}
CUAVariable* CEventDefinition::GetRelatedActiveStateId() 
{ 
	return m_pRelatedActiveStateId; 
}
void CEventDefinition::SetRelatedActiveStateId(CUAVariable* pUAVariable) 
{
	m_pRelatedActiveStateId = pUAVariable; 
}
CUAVariable* CEventDefinition::GetRelatedActiveStateEffectiveDisplayName() 
{ 
	return m_pRelatedActiveStateEffectiveDisplayName; 
}

CUAVariable*	CEventDefinition::GetAckedStateUAVariable() 
{ 
	return m_pAckedStateUAVariable; 
}
void CEventDefinition::SetAckedStateUAVariable(CUAVariable* pVal) 
{ 
	m_pAckedStateUAVariable = pVal; 
}
CUAVariable*	CEventDefinition::GetAckedStateIdUAVariable() 
{ 
	return m_pAckedStateIdUAVariable;
}
void CEventDefinition::SetAckedStateIdUAVariable(CUAVariable* pVal) 
{ 
	m_pAckedStateIdUAVariable = pVal;
}

CUAObjectType*	CEventDefinition::GetEventType() 
{ 
	return m_pEventType; 
}
CUAObject* CEventDefinition::GetConditionNode() 
{ 
	return m_pConditionNode; 
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Gets monitoring object. </summary>
///
/// <remarks>	Michel, 01/09/2016. </remarks>
///
/// <returns>	null if it fails, else the monitoring object. </returns>
///-------------------------------------------------------------------------------------------------

CUABase* OpenOpcUa::UAEvents::CEventDefinition::GetMonitoringObject(void)
{	
	return m_pMonitoringObject;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Access the LimitAlarmValues.  </summary>
///
/// <remarks>	Michel, 07/09/2016. </remarks>
///
/// <returns>	null if it fails, else the limit alarm values. </returns>
///-------------------------------------------------------------------------------------------------

UALimitsAlarm* OpenOpcUa::UAEvents::CEventDefinition::GetLimitAlarmValues(void)
{	
	return m_pLimitAlarmValues;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Sets limit alarm values. </summary>
///
/// <remarks>	Michel, 07/09/2016. </remarks>
///
/// <param name="limitAlarmValues">	[in,out] If non-null, the limit alarm values. </param>
///-------------------------------------------------------------------------------------------------

void OpenOpcUa::UAEvents::CEventDefinition::SetLimitAlarmValues(UALimitsAlarm* pLimitAlarmValues)
{
	if (pLimitAlarmValues)
	{
		m_pLimitAlarmValues->pUAVariableHighHighLimit = pLimitAlarmValues->pUAVariableHighHighLimit;
		m_pLimitAlarmValues->pUAVariableHighLimit = pLimitAlarmValues->pUAVariableHighLimit;

		m_pLimitAlarmValues->pUAVariableLowLimit = pLimitAlarmValues->pUAVariableLowLimit;
		m_pLimitAlarmValues->pUAVariableLowLowLimit = pLimitAlarmValues->pUAVariableLowLowLimit;
	}
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Turn to active. </summary>
///
/// <remarks>	Michel, 07/09/2016. </remarks>
///-------------------------------------------------------------------------------------------------

void OpenOpcUa::UAEvents::CEventDefinition::TurnToActive(void)
{
	CDataValue* pDataValueActiveState = m_pRelatedActiveState->GetValue();
	CDataValue* pDataValueActiveStateId = m_pRelatedActiveStateId->GetValue();
	CDataValue* pDataValueAckedStateId = m_pAckedStateIdUAVariable->GetValue();
	CDataValue* pDataValueRetain = m_pRetainVariable->GetValue();
	CDataValue* pDataValueActiveStateEffectiveDisplayName = OpcUa_Null;

	if (m_pRelatedActiveStateEffectiveDisplayName)
		pDataValueActiveStateEffectiveDisplayName = m_pRelatedActiveStateEffectiveDisplayName->GetValue();
	OpcUa_LocalizedText ActiveState;
	OpcUa_LocalizedText_Initialize(&ActiveState);

	OpcUa_Variant retainValue=pDataValueRetain->GetValue();
	retainValue.Value.Boolean = OpcUa_True;
	pDataValueRetain->SetValue(retainValue);

	OpcUa_String_AttachCopy(&(ActiveState.Locale), "en-us");
	OpcUa_String_AttachCopy(&(ActiveState.Text), "Active");
	if (pDataValueActiveState->GetBuiltInDataType() == OpcUaType_LocalizedText)
	{
		OpcUa_Variant aVariant = pDataValueActiveState->GetValue();
		OpcUa_LocalizedText_CopyTo(&ActiveState, aVariant.Value.LocalizedText);
	}
	if (pDataValueActiveStateEffectiveDisplayName)
	{
		if (pDataValueActiveStateEffectiveDisplayName->GetBuiltInDataType() == OpcUaType_LocalizedText)
		{
			OpcUa_Variant aVariant = pDataValueActiveStateEffectiveDisplayName->GetValue();
			OpcUa_LocalizedText_CopyTo(&ActiveState, aVariant.Value.LocalizedText);
		}
	}
	if (pDataValueActiveStateId->GetBuiltInDataType() == OpcUaType_Boolean)
	{
		OpcUa_Variant aVariant = pDataValueActiveStateId->GetValue();
		aVariant.Value.Boolean = OpcUa_True;
		pDataValueActiveStateId->SetValue(aVariant);
	}
	if (m_pAckedStateIdUAVariable)
	{
		if (pDataValueAckedStateId->GetBuiltInDataType() == OpcUaType_Boolean)
		{
			OpcUa_Variant aVariant = pDataValueAckedStateId->GetValue();
			if (aVariant.Value.Boolean == (OpcUa_Boolean)OpcUa_True)
			{
				aVariant.Value.Boolean = OpcUa_False;
				pDataValueAckedStateId->SetValue(aVariant);
			}
		}
	}
	OpcUa_LocalizedText_Clear(&ActiveState);
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Turn to inactive. </summary>
///
/// <remarks>	Michel, 07/09/2016. </remarks>
///-------------------------------------------------------------------------------------------------

void OpenOpcUa::UAEvents::CEventDefinition::TurnToInactive(void)
{
	CDataValue* pDataValueActiveState = m_pRelatedActiveState->GetValue();
	CDataValue* pDataValueActiveStateId = m_pRelatedActiveStateId->GetValue();
	CDataValue* pDataValueAckedStateId = m_pAckedStateIdUAVariable->GetValue();
	CDataValue* pDataValueActiveStateEffectiveDisplayName = OpcUa_Null;
	CDataValue* pDataValueRetain = m_pRetainVariable->GetValue();

	if (m_pRelatedActiveStateEffectiveDisplayName)
		pDataValueActiveStateEffectiveDisplayName = m_pRelatedActiveStateEffectiveDisplayName->GetValue();
	OpcUa_LocalizedText ActiveState;
	OpcUa_LocalizedText_Initialize(&ActiveState);

	OpcUa_Variant retainValue = pDataValueRetain->GetValue();
	retainValue.Value.Boolean = OpcUa_False;
	pDataValueRetain->SetValue(retainValue);

	OpcUa_String_AttachCopy(&(ActiveState.Locale), "en-us");
	OpcUa_String_AttachCopy(&(ActiveState.Text), "Inactive");
	OpcUa_Variant aVariant;

	if (pDataValueActiveState->GetBuiltInDataType() == OpcUaType_LocalizedText)
	{
		OpcUa_Variant_Initialize(&aVariant);
		aVariant = pDataValueActiveState->GetValue();
		OpcUa_LocalizedText_CopyTo(&ActiveState, aVariant.Value.LocalizedText);
	}
	if (pDataValueActiveStateEffectiveDisplayName)
	{
		if (pDataValueActiveStateEffectiveDisplayName->GetBuiltInDataType() == OpcUaType_LocalizedText)
		{
			aVariant = pDataValueActiveStateEffectiveDisplayName->GetValue();
			OpcUa_LocalizedText_CopyTo(&ActiveState, aVariant.Value.LocalizedText);
		}
	}
	if (pDataValueActiveStateId->GetBuiltInDataType() == OpcUaType_Boolean)
	{
		OpcUa_Variant_Initialize(&aVariant);
		aVariant = pDataValueActiveStateId->GetValue();
		aVariant.Value.Boolean = OpcUa_False;
		pDataValueActiveStateId->SetValue(aVariant);
	}
	OpcUa_LocalizedText_Clear(&ActiveState);
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Sets event message. </summary>
///
/// <remarks>	Michel, 08/09/2016. </remarks>
///
/// <param name="pUAVariableMessage">	[in,out] If non-null, message describing the UA variable.
/// </param>
///-------------------------------------------------------------------------------------------------

void OpenOpcUa::UAEvents::CEventDefinition::SetEventMessage(CUAVariable*pUAVariableMessage)
{
	m_pEventMessage = pUAVariableMessage;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Gets event message. </summary>
///
/// <remarks>	Michel, 08/09/2016. </remarks>
///
/// <returns>	null if it fails, else the event message. </returns>
///-------------------------------------------------------------------------------------------------

CUAVariable* OpenOpcUa::UAEvents::CEventDefinition::GetEventMessage(void)
{
	
	return m_pEventMessage;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Gets transaction status. </summary>
///
/// <remarks>	Michel, 09/09/2016. </remarks>
///
/// <returns>	The transaction status. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_UInt32 OpenOpcUa::UAEvents::CEventDefinition::GetTransactionStatus(void)
{
	return m_eTransactionStatus;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Sets transaction status. </summary>
///
/// <remarks>	Michel, 09/09/2016. </remarks>
///
/// <param name="eStatus">	The status. </param>
///-------------------------------------------------------------------------------------------------

void OpenOpcUa::UAEvents::CEventDefinition::SetTransactionStatus(OpcUa_UInt32 eStatus)
{
	m_eTransactionStatus = eStatus;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Gets current threshold. </summary>
///
/// <remarks>	Michel, 22/09/2016. </remarks>
///
/// <returns>	The current threshold. </returns>
///-------------------------------------------------------------------------------------------------

LimitAlarmThreshold OpenOpcUa::UAEvents::CEventDefinition::GetCurrentThreshold(void)
{
	return m_CurrentThreshold;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Retain Variable.  </summary>
///
/// <remarks>	Michel, 27/09/2016. </remarks>
///
/// <returns>	null if it fails, else the retain variable. </returns>
///-------------------------------------------------------------------------------------------------

CUAVariable* OpenOpcUa::UAEvents::CEventDefinition::GetRetainVariable(void)
{	
	return m_pRetainVariable;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Sets retain variable. </summary>
///
/// <remarks>	Michel, 27/09/2016. </remarks>
///
/// <param name="pRetainVariable">	[in,out] If non-null, the retain variable. </param>
///-------------------------------------------------------------------------------------------------

void OpenOpcUa::UAEvents::CEventDefinition::SetRetainVariable(CUAVariable* pRetainVariable)
{
	m_pRetainVariable = pRetainVariable;
}

