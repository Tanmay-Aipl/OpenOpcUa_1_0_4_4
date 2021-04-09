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
#ifndef OPENOPCUA_CLIENT_LIBRARY
#define OPENOPCUA_CLIENT_LIBRARY
#include <opcua.h>
#include <opcua_types.h>

extern OpcUa_UInt32 g_ServiceCallTimeout;
// Structure for CreateMonitoredItemEx
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
// structre resulting of the call to CreateMonitoredItemEx
typedef struct _OpcUa_MonitoredItemCreated
{
	OpcUa_StatusCode			m_Result;
	OpcUa_UInt32				m_MonitoredItemId;
	OpcUa_Double				m_RevisedSamplingInterval;
	OpcUa_UInt32				m_pRevisedQueueSize;
	OpcUa_Handle				m_hMonitoredItem;
} OpcUa_MonitoredItemCreated;

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
typedef struct _OpenOpcUa_HistoryReadResult
{
	OpcUa_StatusCode    StatusCode;
	OpcUa_HistoryData** ppHistoryData;
}
OpenOpcUa_HistoryReadResult;

// InitializeAbstractionLayer
OpcUa_StatusCode OpenOpcUa_InitializeAbstractionLayer(OpcUa_CharA* szApplicationName, OpcUa_Handle* hApplication, OpcUa_UInt32 uiDefaultTimeout = 15000);
// InitializeSecurity
OpcUa_StatusCode OpenOpcUa_InitializeSecurity(OpcUa_Handle hApplication,OpcUa_String szCertificateStore);
// Clear all the ressoucrces allocated by the application
OpcUa_StatusCode OpenOpcUa_ClearAbstractionLayer(OpcUa_Handle hApplication);
// FindServers
OpcUa_StatusCode OpenOpcUa_FindServers(OpcUa_Handle hApplication,
									   OpcUa_String* hostName,
									   OpcUa_Int32* uiNoOfServer,
									   OpcUa_ApplicationDescription** pServers);
// GetEndpoints
OpcUa_StatusCode OpenOpcUa_GetEndpoints(OpcUa_Handle hApplication,OpcUa_String* discoveryUrl,OpcUa_UInt32* uiNoOfEndpointDescription,OpcUa_EndpointDescription** ppEndpointDescription);

// Create Session
OpcUa_StatusCode OpenOpcUa_CreateSession(OpcUa_Handle hApplication,
										 OpcUa_EndpointDescription* pEndpointDescription,
										 OpcUa_Double nRequestedClientSessionTimeout,
										 OpcUa_String aSessionName, 
										 OpcUa_Handle* hSession);
// GeApplicationDescription
OpcUa_StatusCode OpenOpcUa_GetApplicationDescription(OpcUa_Handle hApplication, OpcUa_ApplicationDescription* pAppDescription);
// GeEndpointDescription
OpcUa_StatusCode OpenOpcUa_GetEndpointDescription(OpcUa_Handle hApplication, OpcUa_Handle hSession, OpcUa_EndpointDescription** pEndpointDescription);
// Activate Session
OpcUa_StatusCode OpenOpcUa_ActivateSession(OpcUa_Handle hApplication, OpcUa_Handle hSession, OpcUa_EndpointDescription* pEndpointDescription, const char* AuthenticationMode, const char* sUserName, const char* sPassword);
// Close Session
OpcUa_StatusCode OpenOpcUa_CloseSession(OpcUa_Handle hApplication,OpcUa_Handle hSession);
// Create Subscription
OpcUa_StatusCode OpenOpcUa_CreateSubscription(OpcUa_Handle hApplication,
																	OpcUa_Handle hSession,
											  OpcUa_Double* dblPublishingInterval, // In/Out param
											  OpcUa_UInt32* uiLifetimeCount, // In/Out param
											  OpcUa_UInt32* uiMaxKeepAliveCount,// In/Out param
											  OpcUa_UInt32 uiMaxNotificationsPerPublish,
											  OpcUa_Boolean bPublishingEnabled,
											  OpcUa_Byte aPriority,
																	OpcUa_Handle* hSubscription);
// Delete Subscription
OpcUa_StatusCode OpenOpcUa_DeleteSubscription(OpcUa_Handle hApplication,OpcUa_Handle hSession,OpcUa_Handle hSubscription);
// Modify Subscription
OpcUa_StatusCode OpenOpcUa_ModifySubscription(OpcUa_Handle hApplication,OpcUa_Handle hSession,OpcUa_Handle hSubscription,
											  OpcUa_Double* dblPublishingInterval, // In/Out param
											  OpcUa_UInt32* uiLifetimeCount, // In/Out param
											  OpcUa_UInt32* uiMaxKeepAliveCount,// In/Out param
											  OpcUa_UInt32 uiMaxNotificationsPerPublish,
											  OpcUa_Byte aPriority);
// retrieve the subscription attached to this monitoredItem
OpcUa_StatusCode OpenOpcUa_GetSubscriptionOfMonitoredItem(OpcUa_Handle hApplication,
													OpcUa_Handle hSession,
													OpcUa_Handle hMonitoredItem, 
													OpcUa_Handle* hSubscription);
// Retieve the subscription params of the given subscription (hSubscription)
OpcUa_StatusCode OpenOpcUa_GetSubscriptionParams(OpcUa_Handle hApplication,
													OpcUa_Handle hSession,
													OpcUa_Handle hSubscription,
													OpenOpcUa_SubscriptionParam* pSubscriptionParam);
// Modify the publishing mode for a subscription
OpcUa_StatusCode OpenOpcUa_SetPublishingMode(OpcUa_Handle hApplication,
											  OpcUa_Handle hSession,
											  OpcUa_Handle hSubscription,
											  OpcUa_Boolean bPublishMode);
// Create MonitoredItems
OpcUa_StatusCode OpenOpcUa_CreateMonitoredItems(OpcUa_Handle hApplication,OpcUa_Handle hSession,OpcUa_Handle hSubscription,
												OpcUa_Byte aTimestampsToReturn,
												OpcUa_UInt32 NoOfItemsToCreate,
												OpcUa_MonitoredItemCreateRequest* pItemsToCreate,
												OpcUa_MonitoredItemCreateResult** ppResult,
												OpcUa_Handle** hMonitoredItems);
// Create MonitoredItems
OpcUa_StatusCode OpenOpcUa_CreateMonitoredItemsEx(	OpcUa_Handle					 hApplication,// in
													OpcUa_Handle					hSession, // in
													OpcUa_Handle					hSubscription,// in
													OpcUa_UInt32					NoOfItemsToCreate, // in
													OpcUa_MonitoredItemToCreate*	pItemsToCreate, // in
													OpcUa_MonitoredItemCreated**	ppItemsCreated); // out
// Delete MonitoredItems
OpcUa_StatusCode OpenOpcUa_DeleteMonitoredItems(OpcUa_Handle hApplication,OpcUa_Handle hSession,OpcUa_Handle hSubscription,
												OpcUa_Int32 iNoOfMonitoredItem,
												OpcUa_Handle* hMonitoredItems,
												OpcUa_StatusCode** ppStatusCode);
// Modify MonitoredItems
OpcUa_StatusCode OpenOpcUa_ModifyMonitoredItems(OpcUa_Handle	hApplication,
												OpcUa_Handle	hSession,
												OpcUa_Handle	hSubscription,
												OpcUa_Handle	hMonitoredItem,
												OpcUa_Byte		aTimestampsToReturn,
												OpcUa_Boolean	bDiscardOldest,
												OpcUa_UInt32	uiQueueSize,
												OpcUa_Double	dblSamplingInterval,
												OpcUa_UInt32	DeadbandType,
												OpcUa_Double	DeadbandValue,
												OpcUa_Byte		DatachangeTrigger);
// SetMonitoringMode
OpcUa_StatusCode OpenOpcUa_SetMonitoringMode(OpcUa_Handle hApplication,OpcUa_Handle hSession,OpcUa_Handle hSubscription,
											 OpcUa_Int32			iNoOfMonitoredItem,
											 OpcUa_Handle*			hMonitoredItems,
											 OpcUa_MonitoringMode	eMonitoringMode);

// Read Attributes
OpcUa_StatusCode OpenOpcUa_ReadAttributes(OpcUa_Handle hApplication,
										  OpcUa_Handle hSession,
										  OpcUa_TimestampsToReturn eTimestampsToReturn,
										  OpcUa_Int32 iNoOfNodesToRead, 
										  OpcUa_ReadValueId* pNodesToRead,
										  OpcUa_DataValue** ppResults);
// HistoryRead
OpcUa_StatusCode OpenOpcUa_HistoryReadRaw(OpcUa_Handle					hApplication,
										  OpcUa_Handle					hSession,
										  OpcUa_Boolean					bModified,
										  OpcUa_TimestampsToReturn      eTimestampsToReturn,
										  OpcUa_Int32                   iNoOfNodesToRead,
										  OpenOpcUa_HistoryReadValueId* pNodesToRead,
										  OpcUa_Int32*                  ipNoOfResults,
										  OpenOpcUa_HistoryReadResult** ppHistoryResults);
// Write Attributes
OpcUa_StatusCode OpenOpcUa_WriteAttributes(OpcUa_Handle hApplication,
										   OpcUa_Handle hSession,
										   OpcUa_Int32 iNoOfNodesToWrite, 
										   OpcUa_WriteValue* pNodesToWrite,
										   OpcUa_StatusCode** ppStatusCode);
// Call
OpcUa_StatusCode OpenOpcUa_Call(OpcUa_Handle hApplication,
												OpcUa_Handle hSession,
								OpcUa_Int32 iNoOfMethodsToCall,
								OpcUa_CallMethodRequest* pMethodsToCall,
								OpcUa_CallMethodResult** ppResults);
// détermine le type d'un handle
OpcUa_StatusCode OpenOpcUa_WhatIsIt(OpcUa_Handle hHandle, OpenOpcUa_HandleType* aHandleType);
// recupère la session a laquelle appartient un souscription
OpcUa_StatusCode OpenOpcUa_GetSessionOfSubscription(OpcUa_Handle hSubscription,OpcUa_Handle* hSession);
// Récupère le OpenOpcUa_InternalNode associé a ce hMonitoredItem
OpcUa_StatusCode OpenOpcUa_GetInternalNode(OpcUa_Handle hApplication,
											OpcUa_Handle hSession,
											OpcUa_Handle hSubscription,
											OpcUa_Handle hMonitoredItem,
										    OpenOpcUa_InternalNode** pInternalNode);
OpcUa_StatusCode OpenOpcUa_GetInternalNodeByMonitoredItemId(OpcUa_Handle hApplication,
											OpcUa_Handle hSession,
											OpcUa_Handle hSubscription,
											OpcUa_UInt32 MonitoredItemId,
											OpenOpcUa_InternalNode** pInternalNode);
OpcUa_StatusCode OpenOpcUa_GetInternalNodeByClientHandle(OpcUa_Handle hApplication,
											OpcUa_Handle hSession,
											OpcUa_Handle hSubscription,
											OpcUa_UInt32 ClientHandle,
											OpenOpcUa_InternalNode** pInternalNode);
// libère le contenu de l'internal Node sans supprimer le hMonitoredItem
OpcUa_StatusCode OpenOpcUa_ReleaseInternalNode(OpenOpcUa_InternalNode* pInternalNode);

//////////////////////////////////////////////////////////////////////////////////////////
// Callback functions
typedef OpcUa_StatusCode (__stdcall *PFUNC)(OpcUa_Handle hSubscription,
								OpcUa_Int32 NoOfMonitoredItems,
								OpcUa_MonitoredItemNotification* MonitoredItems,void* pParamData);

OpcUa_StatusCode OpenOpcUa_SetPublishCallback(OpcUa_Handle hApplication,
																	OpcUa_Handle hSession,
																	OpcUa_Handle hSubscription,
											  PFUNC lpCallback,void* pCallbackData);
typedef OpcUa_StatusCode (__stdcall *PFUNCSHUTDOWN)(
												OpcUa_Handle hApplication,OpcUa_Handle hSession,
								OpcUa_String strShutdownReason, 
								void* pParamData);
OpcUa_StatusCode OpenOpcUa_SetShutdownCallback( OpcUa_Handle hApplication,
												OpcUa_Handle hSession,
												PFUNCSHUTDOWN lpCallback, void* pCallbackData);
/////////////////////////////////////////////////////////////////////////////
// Helper Function
OpcUa_StatusCode OpenOpcUa_VariantToString(OpcUa_Variant& Var,OpcUa_String** strValue);
OpcUa_StatusCode OpenOpcUa_DateTimeToString(OpcUa_DateTime aDateTime,OpcUa_String** strTime);
OpcUa_StatusCode OpenOpcUa_StringToNodeId(OpcUa_String strNodeId,OpcUa_NodeId* pNodeId);
OpcUa_StatusCode OpenOpcUa_NodeIdToString(OpcUa_NodeId aNodeId,OpcUa_String* strNodeId);
OpcUa_StatusCode OpenOpcUa_Browse(OpcUa_Handle hApplication,
								  OpcUa_Handle hSession,
								  OpcUa_Int32 iNoOfNodesToBrowse,
								  const OpcUa_BrowseDescription* pNodesToBrowse,
								  OpcUa_Int32* iNoOfReferenceDescription,
								  OpcUa_ReferenceDescription** pReferenceList);
// Function for configuration file manipulation. Configuration file are conform to OpenOpcUaClientConfig.xsd
OpcUa_StatusCode OpenOpcUa_LoadConfig(OpcUa_Handle hApplication, OpcUa_String szConfigFileName);
OpcUa_StatusCode OpenOpcUa_SaveConfig(OpcUa_Handle hApplication, OpcUa_String szConfigFileName);
// Retrieve hSessions for a hApplication
OpcUa_StatusCode OpenOpcUa_GetSessions(OpcUa_Handle hApplication, 
									   OpcUa_UInt32* uiNoOfSessions, // out
									   OpcUa_Handle** hSessions); // out
// retrieve the hSubscriptions for a hSession
OpcUa_StatusCode OpenOpcUa_GetSubscriptions(OpcUa_Handle hApplication, 
											OpcUa_Handle hSession, 
											OpcUa_UInt32* uiNoOfSubscription, // out
											OpcUa_Handle** hSubscription); // out
// retrieve the hMonitoredItems for a hSubscription
OpcUa_StatusCode OpenOpcUa_GetMonitoredItems(OpcUa_Handle hApplication,
											OpcUa_Handle hSession,
											OpcUa_Handle hSubscription,
											OpcUa_UInt32* uiNoOfMonitoredItems, // out
											OpcUa_Handle** hMonitoredItems); // out
// Trace function
OpcUa_StatusCode OpenOpcUa_Trace(OpcUa_Handle hApplication,OpcUa_UInt32 a_uTraceLevel, const OpcUa_CharA* format,...);
// this function will provide the details for an attributes. This is not related to hApplication or hSession 
// This function is just an helper. Application have to release allocated ressources, szAttributeName and szAttributeDescription
OpcUa_StatusCode OpenOpcUa_GetAttributeDetails(OpcUa_Int32 iAttributeId, OpcUa_String** szAttributeName, OpcUa_String** szAttributeDescription);
// This function will lookup the userAccessLevel and accessLevel
OpcUa_StatusCode OpenOpcUa_GetAccessLevel(OpcUa_Byte AccessLevel, OpcUa_String** szAccessLevel);
OpcUa_StatusCode OpenOpcUa_GetUserAccessLevel(OpcUa_Byte AccessLevel, OpcUa_String** szAccessLevel);
// This function will provide detail for BuiltInTypes. This is just an helper function
// Application have to release allocated ressources, szBuiltInTypeName, szBuiltInTypeDescription
OpcUa_StatusCode OpenOpcUa_GetBuiltInTypeDetails(OpcUa_Int32 iBuiltInTypeId, OpcUa_String** szBuiltInTypeName, OpcUa_String** szBuiltInTypeDescription);
// This function will lookup the className from the enum number to a OpcUa_String
// Application have to release allocated ressources, szBuiltInTypeName, szBuiltInTypeDescription
OpcUa_StatusCode OpenOpcUa_GetNodeClassDetails(OpcUa_Int32 iNodeClass, OpcUa_String** szNodeClassName);
// Transform an OpcUa_String to OpcUa_Variant according to the requested DataType
OpcUa_StatusCode OpenOpcUa_StringToVariant(OpcUa_String* pString, OpcUa_Int32 iDataType,OpcUa_Variant** pVariant);
// Retrieve the dataType of the UAVariable (aNodeId)  
OpcUa_StatusCode OpenOpcUa_GetUAVariableDatatype(OpcUa_Handle hApplication, OpcUa_Handle hSession, OpcUa_NodeId aNodeId, OpcUa_NodeId* pDataType);
// Create a Filter Extension Object according to the parameter receive
OpcUa_StatusCode OpenOpcUa_CreateFilterObject(OpcUa_UInt32 DeadbandType, OpcUa_Double DeadbandValue, OpcUa_Byte DatachangeTrigger, OpcUa_ExtensionObject** pExtensionObject);
// Give a string representation of an StatusCode
OpcUa_StatusCode OpenOpcUa_StatusCodeToString(OpcUa_StatusCode uStatus, OpcUa_String** pszStatusCode);
#endif // OPENOPCUA_CLIENT_LIBRARY
// for debug purpose with Visual Detector
#ifdef SEEK_LEAKER
#include "vld.h"
#endif
