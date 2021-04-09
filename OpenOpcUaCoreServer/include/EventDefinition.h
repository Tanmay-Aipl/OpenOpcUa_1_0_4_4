#pragma once
namespace OpenOpcUa
{ 
	namespace UAEvents
	{
		// This enum is used by notifification mecanism 
		// in the class CUAEventNotificationList. 
		// It show the status of the last notifed event
		//typedef enum _EventTransactionStatus
		//{
		//	Unknown, // 0
		//	NewStatus, // 1
		//	Transition, // 2
		//	SentToClient, // 3
		//	AckedByClient // 4
		//} EventTransactionStatus;
		#define OPCUA_EVENTSTATUS_UNKNOWN 1
		#define OPCUA_EVENTSTATUS_NEWSTATUS 2
		#define OPCUA_EVENTSTATUS_TRANSITION 4
		#define OPCUA_EVENTSTATUS_SENDTOCLIENT 8
		#define OPCUA_EVENTSTATUS_ACKEDBYCLIENT 16
		typedef struct UALimitsAlarm		
		{
			CUAVariable* pUAVariableLowLimit;
			CUAVariable* pUAVariableLowLowLimit;
			CUAVariable* pUAVariableHighLimit;
			CUAVariable* pUAVariableHighHighLimit;
		} _UALimitsAlarm;
		typedef enum _LimitAlarmThreshold
		{
			LowThreshold,
			LowLowThreshold,
			NormalThreshold,
			HighThreshold,
			HighHighThreshold
		} LimitAlarmThreshold;

		class CEventDefinition
		{
		public:
			CEventDefinition(CUABase* pMonitoringObject,
				CUAObject* pConditionNode,
				CUAVariable*	pSourceNode,
				CUAObjectType*	pEventType);
			CEventDefinition();
			~CEventDefinition();
			CUABase* GetMonitoringObject();
			OpcUa_Boolean	GetActiveState();
			void SetActiveState(OpcUa_Boolean bVal);
			OpcUa_Boolean	GetPreviousActiveState();
			void SetPreviousActiveState(OpcUa_Boolean bVal);
			OpcUa_StatusCode EvaluateStateMachine();
			// Type and definition
			CUAObjectType*	GetEventType();
			CUAObject* GetConditionNode();
			// Internal method to access the Alarming CUAVariable
			CUAVariable*	GetAckedStateUAVariable();
			void SetAckedStateUAVariable(CUAVariable* pVal);
			CUAVariable*	GetAckedStateIdUAVariable();
			void SetAckedStateIdUAVariable(CUAVariable* pVal);
			// 
			CUAVariable* GetRelatedActiveState();
			void SetRelatedActiveState(CUAVariable* pUAVariable);
			CUAVariable* GetRelatedActiveStateId();
			void SetRelatedActiveStateId(CUAVariable* pUAVariable);
			CUAVariable* GetRelatedActiveStateEffectiveDisplayName();
			// EnabledState and Id
			void SetRelatedActiveStateEffectiveDisplayName(CUAVariable* pUAVariable);
			void SetRelativeEnabledStateUAVariable(CUAVariable* pUAVariable);
			CUAVariable* GetRelativeEnabledStateUAVariable();
			void SetRelativeEnabledStateIdUAVariable(CUAVariable* pUAVariable);
			CUAVariable* GetRelativeEnabledStateIdUAVariable();
			// ConfirmedState and Id
			CUAVariable* GetConfirmedStateUAVariable();
			void SetConfirmedStateUAVariable(CUAVariable* pConfirmedStateUAVariable);
			CUAVariable* GetConfirmedStateIdUAVariable();
			void SetConfirmedStateIdUAVariable(CUAVariable* pConfirmedStateIdUAVariable);
			// Retain Variable
			CUAVariable* GetRetainVariable();
			void SetRetainVariable(CUAVariable* pRetainVariable);
			// Method
			CUAMethod* GetEnableMethod();
			void SetEnableMethod(CUAMethod* pMethod);
			CUAMethod* GetDisableMethod();
			void SetDisableMethod(CUAMethod* pMethod);

			OpcUa_Boolean	IsOnStage();
			void OnStage(OpcUa_Boolean bVal);
			CUAVariable*	GetSourceNode();
			OpcUa_Boolean IsVirtual();
			// Enable
			void Enable(OpcUa_Boolean bVal);
			OpcUa_Boolean IsEnable();
			// SourceName
			CUAVariable*	GetSourceName();
			void SetSourceName(CUAVariable* pVal);
			// Access the LimitAlarmValues
			UALimitsAlarm* GetLimitAlarmValues();
			void SetLimitAlarmValues(UALimitsAlarm* limitAlarmValues);
			void TurnToActive();
			void TurnToInactive();
			CUAVariable* GetEventMessage();
			void SetEventMessage(CUAVariable* pUAVariableMessage);
			OpcUa_UInt32 GetTransactionStatus();
			void SetTransactionStatus(OpcUa_UInt32 eStatus);
			LimitAlarmThreshold		GetCurrentThreshold();
		private:
			CUABase*					m_pMonitoringObject; // This contains the UANode that monitor this definition.
			CUAObject*					m_pConditionNode; // This is the UANode that is the target of the hasCondition reference
											  // This is the NodeSet definition of the Alarm (GroundFaultAlarm)
			CUAVariable*				m_pSourceNode; // This is the SourceNode for the Event. The trigger
			CUAVariable*				m_pSourceName; // This is the UAVariable that contain the sourceName of the EventDefinition
										   // SourceName is one of the mandatory EventField
			CUAVariable*				m_pEventMessage;
			CUAObjectType*				m_pEventType;
			UALimitsAlarm*				m_pLimitAlarmValues;
			LimitAlarmThreshold			m_CurrentThreshold;
			OpcUa_Boolean				m_bEnable;			// indicate if the CEventDefinition is Enable or not. If not it will not be notify to client 
			OpcUa_Boolean				m_bPreviousActiveState; // Indicate the previous ActiveState that was notified
			OpcUa_Boolean				m_bActiveState; // Indicate the current ActiveState to notify. 
														// If true the CEventDefinition will be instantiate and send to the client
			
			OpcUa_UInt32				m_eTransactionStatus;
			
			OpcUa_Boolean				m_bOnStage;
			// FastAccess to AckedState
			CUAVariable*				m_pAckedStateUAVariable;
			CUAVariable*				m_pAckedStateIdUAVariable; // UAVariable that contains the AckedStateId
			// FastAccess to ActiveState
			CUAVariable*				m_pRelatedActiveState; // for internal use do not release. Contains the CUAVariable with the ActiveState. This UAVariable must be of dataType LocalizedText
			CUAVariable*				m_pRelatedActiveStateId; // for internal use do not release. Contains the CUAVariable with the ActiveState Id. This UAVariable must be of dataType Boolean
			CUAVariable*				m_pRelatedActiveStateEffectiveDisplayName; // for internal use do not release. Contains the CUAVariable with the ActiveState. This UAVariable must be of dataType LocalizedText
			CUAVariable*				m_pRetainVariable;
			CUAVariable*				m_pRelatedEnabledStateUAVariable; // For internal use. do not release. Contain the CUAvariable with the Enabled/DisabledState (The value must contains a LocalizedText)
			CUAVariable*				m_pRelatedEnabledStateUAVariableId; // For internal use. do not release. Contain the CUAvariable with the Enabled/DisabledState Id (The value must contains a Boolean)
			CUAVariable*				m_pConfirmedStateUAVariable;
			CUAVariable*				m_pConfirmedStateIdUAVariable;
			CUAMethod*					m_pRelatedEnableMethod; // For internal use. do not release. Contain the CUAMethod used for Enabling the CEventDefinition
			CUAMethod*					m_pRelatedDisableMethod; // For internal use. do not release. Contain the CUAMethod used for Enabling the CEventDefinition
		};
		typedef std::vector<CEventDefinition*> CEventDefinitionList;
	}
}