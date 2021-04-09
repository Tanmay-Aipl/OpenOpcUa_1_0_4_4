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

	///////////////////////////////////////////////////////////////////////////////
	/*============================================================================
	* The DataChangeTrigger enumeration.
	*===========================================================================*/
	enum _OpcUa_DataChangeTrigger
	{
		OpcUa_DataChangeTrigger_Status = 0,
		OpcUa_DataChangeTrigger_StatusValue = 1,
		OpcUa_DataChangeTrigger_StatusValueTimestamp = 2
	};

	// Structure for monitoredItemParams
	typedef struct _OpenOpcUa_MonitoredItemParam_
	{
		OpcUa_Byte		m_aTimestampsToReturn;
		OpcUa_Boolean	m_bDiscardOldest;
		OpcUa_UInt32	m_uiQueueSize;
		OpcUa_Double	m_dblSamplingInterval;
		OpcUa_UInt32	m_DeadbandType;
		OpcUa_Double	m_dblDeadbandValue;
		OpcUa_Byte		m_DataChangeTrigger;
	} OpenOpcUa_MonitoredItemParam;

	/// <summary>
	/// OpenOpcUa_InternalNode
	/// </summary>
	typedef struct _OpenOpcUa_InternalNode_
	{
		OpcUa_NodeId					m_NodeId;
		OpcUa_String					m_BrowseName;
		OpcUa_DataValue					m_DataValue;
		OpcUa_Handle					m_hMonitoredItem; // related MonitoredItem
		void*							m_UserData;
		OpenOpcUa_MonitoredItemParam*	m_pMonitoredItemParam;
	} OpenOpcUa_InternalNode;
	// Handle identification
	enum OpenOpcUa_HandleType
	{
		OPENOPCUA_APPLICATION,
		OPENOPCUA_SESSION,
		OPENOPCUA_SUBSCRIPTION,
		OPENOPCUA_MONITOREDITEM,
		OPENOPCUA_UNKNOWN
	};

	/*============================================================================
	* The DeadbandType enumeration.
	*===========================================================================*/
	enum OpcUa_DeadbandType
	{
		OpcUa_DeadbandType_None = 0,
		OpcUa_DeadbandType_Absolute = 1,
		OpcUa_DeadbandType_Percent = 2
	};
	/*============================================================================
	* Subscription related structure
	*===========================================================================*/
	typedef struct _OpenOpcUa_SubscriptionParam
	{
		OpcUa_UInt32	uiSubscriptionId;
		OpcUa_Double	dblPublishingInterval;
		OpcUa_UInt32	uiLifetimeCount;
		OpcUa_UInt32	uiMaxKeepAliveCount;
		OpcUa_Boolean	bPublishingEnabled;
		OpcUa_Byte		aPriority;
		OpcUa_UInt32	uiMaxNotificationsPerPublish;
	} OpenOpcUa_SubscriptionParam;
	/*============================================================================
	* The History related structure and enumeration.
	*===========================================================================*/
	typedef struct _OpenOpcUa_ReadRawDetails
	{
		OpcUa_DateTime StartTime;
		OpcUa_DateTime EndTime;
		OpcUa_UInt32   NumValuesPerNode;
		OpcUa_Boolean  ReturnBounds;
	}
	OpenOpcUa_ReadRawDetails;
	typedef struct _OpenOpcUa_HistoryReadValueId
	{
		OpcUa_NodeId        NodeId;
		OpcUa_String        IndexRange;
	}
	OpenOpcUa_HistoryReadValueId;
	/*============================================================================
	* The HistoryData structure.
	*===========================================================================*/
	typedef struct _OpcUa_HistoryData
	{
		OpcUa_Int32      NoOfDataValues;
		OpcUa_DataValue* DataValues;
	}
	OpcUa_HistoryData;
	typedef struct _OpenOpcUa_HistoryReadResult
	{
		OpcUa_StatusCode    StatusCode;
		OpcUa_HistoryData** ppHistoryData;
	}
	OpenOpcUa_HistoryReadResult;
	///////////////////////////////////////////////////////////////////////////////
	/*============================================================================
	 * The ApplicationType enumeration.
	 *===========================================================================*/
	typedef enum _OpcUa_ApplicationType
	{
		OpcUa_ApplicationType_Server = 0,
		OpcUa_ApplicationType_Client = 1,
		OpcUa_ApplicationType_ClientAndServer = 2,
		OpcUa_ApplicationType_DiscoveryServer = 3
#if OPCUA_FORCE_INT32_ENUMS
		,_OpcUa_ApplicationType_MaxEnumerationValue = OpcUa_Int32_Max
#endif
	}
	OpcUa_ApplicationType;
	/*============================================================================
	 * The ApplicationDescription structure.
	 *===========================================================================*/
	typedef struct _OpcUa_ApplicationDescription
	{
		OpcUa_String          ApplicationUri;
		OpcUa_String          ProductUri;
		OpcUa_LocalizedText   ApplicationName;
		OpcUa_ApplicationType ApplicationType;
		OpcUa_String          GatewayServerUri;
		OpcUa_String          DiscoveryProfileUri;
		OpcUa_Int32           NoOfDiscoveryUrls;
		OpcUa_String*         DiscoveryUrls;
	}
	OpcUa_ApplicationDescription;

	/*============================================================================
	 * The MessageSecurityMode enumeration.
	 *===========================================================================*/
	typedef enum _OpcUa_MessageSecurityMode
	{
		OpcUa_MessageSecurityMode_Invalid = 0,
		OpcUa_MessageSecurityMode_None = 1,
		OpcUa_MessageSecurityMode_Sign = 2,
		OpcUa_MessageSecurityMode_SignAndEncrypt = 3
#if OPCUA_FORCE_INT32_ENUMS
		,_OpcUa_MessageSecurityMode_MaxEnumerationValue = OpcUa_Int32_Max
#endif
	}
	OpcUa_MessageSecurityMode;

	/*============================================================================
	 * The UserTokenType enumeration.
	 *===========================================================================*/
	typedef enum _OpcUa_UserTokenType
	{
		OpcUa_UserTokenType_Anonymous = 0,
		OpcUa_UserTokenType_UserName = 1,
		OpcUa_UserTokenType_Certificate = 2,
		OpcUa_UserTokenType_IssuedToken = 3
#if OPCUA_FORCE_INT32_ENUMS
		,_OpcUa_UserTokenType_MaxEnumerationValue = OpcUa_Int32_Max
#endif
	}
	OpcUa_UserTokenType;
	/*============================================================================
	 * The UserTokenPolicy structure.
	 *===========================================================================*/
	typedef struct _OpcUa_UserTokenPolicy
	{
		OpcUa_String        PolicyId;
		OpcUa_UserTokenType TokenType;
		OpcUa_String        IssuedTokenType;
		OpcUa_String        IssuerEndpointUrl;
		OpcUa_String        SecurityPolicyUri;
	}
	OpcUa_UserTokenPolicy;


	/*============================================================================
	 * The EndpointDescription structure.
	 *===========================================================================*/
	typedef struct _OpcUa_EndpointDescription
	{
		OpcUa_String                 EndpointUrl;
		OpcUa_ApplicationDescription Server;
		OpcUa_ByteString             ServerCertificate;
		OpcUa_MessageSecurityMode    SecurityMode;
		OpcUa_String                 SecurityPolicyUri;
		OpcUa_Int32                  NoOfUserIdentityTokens;
		OpcUa_UserTokenPolicy*       UserIdentityTokens;
		OpcUa_String                 TransportProfileUri;
		OpcUa_Byte                   SecurityLevel;
	}
	OpcUa_EndpointDescription;
	OPCUA_BEGIN_EXTERN_C
	OpcUa_Void OpcUa_EndpointDescription_Initialize(OpcUa_EndpointDescription* pValue);

	OpcUa_Void OpcUa_EndpointDescription_Clear(OpcUa_EndpointDescription* pValue);
	OPCUA_END_EXTERN_C
	/*============================================================================
	 * The ReadValueId structure.
	*===========================================================================*/
	typedef struct _OpcUa_ReadValueId
	{
		OpcUa_NodeId        NodeId;
		OpcUa_UInt32        AttributeId;
		OpcUa_String        IndexRange;
		OpcUa_QualifiedName DataEncoding;
	}
	OpcUa_ReadValueId;
	OPCUA_BEGIN_EXTERN_C
	OpcUa_Void OpcUa_ReadValueId_Initialize(OpcUa_ReadValueId* pValue);

	OpcUa_Void OpcUa_ReadValueId_Clear(OpcUa_ReadValueId* pValue);
	OPCUA_END_EXTERN_C
	/*============================================================================
	* The MonitoringMode enumeration.
	*===========================================================================*/
	typedef enum _OpcUa_MonitoringMode
	{
		OpcUa_MonitoringMode_Disabled = 0,
		OpcUa_MonitoringMode_Sampling = 1,
		OpcUa_MonitoringMode_Reporting = 2
#if OPCUA_FORCE_INT32_ENUMS
		,_OpcUa_MonitoringMode_MaxEnumerationValue = OpcUa_Int32_Max
#endif
	}
	OpcUa_MonitoringMode;


	/*============================================================================
	 * The MonitoringParameters structure.
	 *===========================================================================*/
	typedef struct _OpcUa_MonitoringParameters
	{
		OpcUa_UInt32          ClientHandle;
		OpcUa_Double          SamplingInterval;
		OpcUa_ExtensionObject Filter;
		OpcUa_UInt32          QueueSize;
		OpcUa_Boolean         DiscardOldest;
	}
	OpcUa_MonitoringParameters;
	/*============================================================================
	 * The MonitoredItemCreateRequest structure.
	 *===========================================================================*/
	typedef struct _OpcUa_MonitoredItemCreateRequest
	{
		OpcUa_ReadValueId          ItemToMonitor;
		OpcUa_MonitoringMode       MonitoringMode;
		OpcUa_MonitoringParameters RequestedParameters;
	}
	OpcUa_MonitoredItemCreateRequest;
	OPCUA_BEGIN_EXTERN_C
	OpcUa_Void OpcUa_MonitoredItemCreateRequest_Initialize(OpcUa_MonitoredItemCreateRequest* pValue);

	OpcUa_Void OpcUa_MonitoredItemCreateRequest_Clear(OpcUa_MonitoredItemCreateRequest* pValue);
	OPCUA_END_EXTERN_C
		/*============================================================================
		 * The MonitoredItemCreateResult structure.
		 *===========================================================================*/
	typedef struct _OpcUa_MonitoredItemCreateResult
	{
		OpcUa_StatusCode      StatusCode;
		OpcUa_UInt32          MonitoredItemId;
		OpcUa_Double          RevisedSamplingInterval;
		OpcUa_UInt32          RevisedQueueSize;
		OpcUa_ExtensionObject FilterResult;
	}
	OpcUa_MonitoredItemCreateResult;

	/*============================================================================
	 * The TimestampsToReturn enumeration.
	 *===========================================================================*/
	typedef enum _OpcUa_TimestampsToReturn
	{
		OpcUa_TimestampsToReturn_Source = 0,
		OpcUa_TimestampsToReturn_Server = 1,
		OpcUa_TimestampsToReturn_Both = 2,
		OpcUa_TimestampsToReturn_Neither = 3
#if OPCUA_FORCE_INT32_ENUMS
		,_OpcUa_TimestampsToReturn_MaxEnumerationValue = OpcUa_Int32_Max
#endif
	}
	OpcUa_TimestampsToReturn;
	/*============================================================================
	* Structure for CreateMonitoredItemEx
	*===========================================================================*/
	typedef struct _OpcUa_MonitoredItemToCreate
	{
		OpcUa_NodeId				m_NodeId;
		OpcUa_UInt32				m_AttributeId;
		OpcUa_String				m_IndexRange;
		OpcUa_MonitoringMode		m_MonitoringMode;
		OpcUa_UInt32				m_ClientHandle;
		OpcUa_Double				m_SamplingInterval;
		OpcUa_UInt32				m_DeadbandType;
		OpcUa_Double				m_DeadbandValue;
		OpcUa_Byte					m_DatachangeTrigger;
		OpcUa_UInt32				m_QueueSize;
		OpcUa_Boolean				m_DiscardOldest;
		OpcUa_TimestampsToReturn	m_TimestampsToReturn;
	}
	OpcUa_MonitoredItemToCreate;
	/*============================================================================
	* structure resulting of the call to CreateMonitoredItemEx
	*===========================================================================*/
	typedef struct _OpcUa_MonitoredItemCreated
	{
		OpcUa_StatusCode			m_Result;
		OpcUa_UInt32				m_MonitoredItemId;
		OpcUa_Double				m_RevisedSamplingInterval;
		OpcUa_UInt32				m_pRevisedQueueSize;
		OpcUa_Handle				m_hMonitoredItem;
	} OpcUa_MonitoredItemCreated;
	/*============================================================================
	 * The WriteValue structure.
	 *===========================================================================*/
	typedef struct _OpcUa_WriteValue
	{
		OpcUa_NodeId    NodeId;
		OpcUa_UInt32    AttributeId;
		OpcUa_String    IndexRange;
		OpcUa_DataValue Value;
	}
	OpcUa_WriteValue;
	OPCUA_BEGIN_EXTERN_C
	OpcUa_Void OpcUa_WriteValue_Initialize(OpcUa_WriteValue* pValue);

	OpcUa_Void OpcUa_WriteValue_Clear(OpcUa_WriteValue* pValue);
	OPCUA_END_EXTERN_C
	/*============================================================================
	* The CallMethodRequest structure.
	*===========================================================================*/
	typedef struct _OpcUa_CallMethodRequest
	{
		OpcUa_NodeId   ObjectId;
		OpcUa_NodeId   MethodId;
		OpcUa_Int32    NoOfInputArguments;
		OpcUa_Variant* InputArguments;
	}
	OpcUa_CallMethodRequest;

	/*============================================================================
	 * The MonitoredItemNotification structure.
	 *===========================================================================*/
	typedef struct _OpcUa_MonitoredItemNotification
	{
		OpcUa_UInt32    ClientHandle;
		OpcUa_DataValue Value;
	}
	OpcUa_MonitoredItemNotification;

	/*============================================================================
	 * The CallMethodResult structure.
	 *===========================================================================*/
	typedef struct _OpcUa_CallMethodResult
	{
		OpcUa_StatusCode      StatusCode;
		OpcUa_Int32           NoOfInputArgumentResults;
		OpcUa_StatusCode*     InputArgumentResults;
		OpcUa_Int32           NoOfInputArgumentDiagnosticInfos;
		OpcUa_DiagnosticInfo* InputArgumentDiagnosticInfos;
		OpcUa_Int32           NoOfOutputArguments;
		OpcUa_Variant*        OutputArguments;
	}
	OpcUa_CallMethodResult;

	/*============================================================================
	 * The BrowseDirection enumeration.
	 *===========================================================================*/
	typedef enum _OpcUa_BrowseDirection
	{
		OpcUa_BrowseDirection_Forward = 0,
		OpcUa_BrowseDirection_Inverse = 1,
		OpcUa_BrowseDirection_Both = 2
#if OPCUA_FORCE_INT32_ENUMS
		,_OpcUa_BrowseDirection_MaxEnumerationValue = OpcUa_Int32_Max
#endif
	}
	OpcUa_BrowseDirection;
	/*============================================================================
	 * The BrowseDescription structure.
	 *===========================================================================*/
	typedef struct _OpcUa_BrowseDescription
	{
		OpcUa_NodeId          NodeId;
		OpcUa_BrowseDirection BrowseDirection;
		OpcUa_NodeId          ReferenceTypeId;
		OpcUa_Boolean         IncludeSubtypes;
		OpcUa_UInt32          NodeClassMask;
		OpcUa_UInt32          ResultMask;
	}
	OpcUa_BrowseDescription;
	OPCUA_BEGIN_EXTERN_C
	OpcUa_Void OpcUa_BrowseDescription_Initialize(OpcUa_BrowseDescription* pValue);

	OpcUa_Void OpcUa_BrowseDescription_Clear(OpcUa_BrowseDescription* pValue);
	OPCUA_END_EXTERN_C
		/*============================================================================
		 * The BrowseResultMask enumeration.
		 *===========================================================================*/
	typedef enum _OpcUa_BrowseResultMask
	{
		OpcUa_BrowseResultMask_None = 0,
		OpcUa_BrowseResultMask_ReferenceTypeId = 1,
		OpcUa_BrowseResultMask_IsForward = 2,
		OpcUa_BrowseResultMask_NodeClass = 4,
		OpcUa_BrowseResultMask_BrowseName = 8,
		OpcUa_BrowseResultMask_DisplayName = 16,
		OpcUa_BrowseResultMask_TypeDefinition = 32,
		OpcUa_BrowseResultMask_All = 63,
		OpcUa_BrowseResultMask_ReferenceTypeInfo = 3,
		OpcUa_BrowseResultMask_TargetInfo = 60
#if OPCUA_FORCE_INT32_ENUMS
		,_OpcUa_BrowseResultMask_MaxEnumerationValue = OpcUa_Int32_Max
#endif
	}
	OpcUa_BrowseResultMask;
	/*============================================================================
	 * The NodeClass enumeration.
	 *===========================================================================*/
	typedef enum _OpcUa_NodeClass
	{
		OpcUa_NodeClass_Unspecified = 0,
		OpcUa_NodeClass_Object = 1,
		OpcUa_NodeClass_Variable = 2,
		OpcUa_NodeClass_Method = 4,
		OpcUa_NodeClass_ObjectType = 8,
		OpcUa_NodeClass_VariableType = 16,
		OpcUa_NodeClass_ReferenceType = 32,
		OpcUa_NodeClass_DataType = 64,
		OpcUa_NodeClass_View = 128
#if OPCUA_FORCE_INT32_ENUMS
		,_OpcUa_NodeClass_MaxEnumerationValue = OpcUa_Int32_Max
#endif
	}
	OpcUa_NodeClass;

	/*============================================================================
	 * The ReferenceDescription structure.
	 *===========================================================================*/
	typedef struct _OpcUa_ReferenceDescription
	{
		OpcUa_NodeId         ReferenceTypeId;
		OpcUa_Boolean        IsForward;
		OpcUa_ExpandedNodeId NodeId;
		OpcUa_QualifiedName  BrowseName;
		OpcUa_LocalizedText  DisplayName;
		OpcUa_NodeClass      NodeClass;
		OpcUa_ExpandedNodeId TypeDefinition;
	}
	OpcUa_ReferenceDescription;
	OPCUA_BEGIN_EXTERN_C
		OpcUa_Void OpcUa_ReferenceDescription_Initialize(OpcUa_ReferenceDescription* pValue);

	OpcUa_Void OpcUa_ReferenceDescription_Clear(OpcUa_ReferenceDescription* pValue);
	OPCUA_END_EXTERN_C

	/*============================================================================
	 * The ExtensionObject functions.
	 *===========================================================================*/
	OPCUA_BEGIN_EXTERN_C
	OpcUa_Void OpcUa_ExtensionObject_Initialize(OpcUa_ExtensionObject* value);
	OpcUa_Void OpcUa_ExtensionObject_Clear(OpcUa_ExtensionObject* value);
	OpcUa_StatusCode OpcUa_ExtensionObject_CopyTo(const OpcUa_ExtensionObject* pSource, OpcUa_ExtensionObject* pDestination);
	OPCUA_END_EXTERN_C