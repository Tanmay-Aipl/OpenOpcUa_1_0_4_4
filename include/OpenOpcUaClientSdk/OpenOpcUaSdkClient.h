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
#ifdef _GNUC_
#ifndef __cdecl
#define __cdecl
#endif

#ifndef __stdcall
#define __stdcall
#endif
#endif
#define OPCUA_EXPORT
#ifdef __cplusplus
# define OPCUA_BEGIN_EXTERN_C extern "C" {
# define OPCUA_END_EXTERN_C }
#else
# define OPCUA_BEGIN_EXTERN_C
# define OPCUA_END_EXTERN_C
#endif
#include "OpenOpcUa_p_types.h"
#include "OpenOpcUa_builtintypes.h"
#include "OpenOpcUa_types.h"
#include "OpenOpcUa_Errors.h"
#include "OpenOpcUa_Attributes.h"
#include "OpenOpcUa_Identifiers.h"

#ifdef __linux
#define __stdcall
#endif


/*============================================================================
* Trace Levels
*===========================================================================*/
/* Output for the trace */
#define OPCUA_TRACE_OUTPUT_FILE			0x00010000
#define OPCUA_TRACE_OUTPUT_CONSOLE		0x00010001
#define OPCUA_TRACE_OUTPUT_NONE			0x00010002
/* predefined trace levels */
#define OPCUA_TRACE_LEVEL_ALWAYS        0x00002000 /* Always code. permet de toujours afficher un message 2^13 (8192)*/
// Trace message from the stack
// 
#define OPCUA_TRACE_STACK_NONE			0x00000004 // 0
#define OPCUA_TRACE_STACK_INFO			0x00000002 // 2^1
#define OPCUA_TRACE_STACK_WARNING		0x00000004 // 2^2
#define OPCUA_TRACE_STACK_ERROR			0x00000008 // 2^3
// 
// Trace message from the server 
// 
#define OPCUA_TRACE_SERVER_NONE			0x00000000 // 0
#define OPCUA_TRACE_SERVER_INFO			0x00000010 // 2^4
#define OPCUA_TRACE_SERVER_WARNING		0x00000020 // 2^5
#define OPCUA_TRACE_SERVER_ERROR		0x00000040 // 2^6
// 
// Trace message from the client
// 
#define OPCUA_TRACE_CLIENT_NONE			0x00000000 // 0
#define OPCUA_TRACE_CLIENT_INFO			0x00000080 // 2^7
#define OPCUA_TRACE_CLIENT_WARNING		0x00000100 // 2^8
#define OPCUA_TRACE_CLIENT_ERROR		0x00000200 // 2^9 
// 
// Trace message from the extra componenet. This include Lua, Xml and SharedLibserver
// 
#define OPCUA_TRACE_EXTRA_NONE			0x00000000 // 0
#define OPCUA_TRACE_EXTRA_INFO			0x00000400 // 2^10
#define OPCUA_TRACE_EXTRA_WARNING		0x00000800 // 2^11
#define OPCUA_TRACE_EXTRA_ERROR			0x00001000 // 2^12 
// 
/* trace level packages */
// 
// trace used in the function OPCUA_TRACE
#define OPCUA_TRACE_STACK_LEVEL_DEBUG   (OPCUA_TRACE_STACK_WARNING | OPCUA_TRACE_STACK_INFO | OPCUA_TRACE_STACK_ERROR )
#define OPCUA_TRACE_STACK_LEVEL_ERROR   (OPCUA_TRACE_STACK_ERROR)
#define OPCUA_TRACE_STACK_LEVEL_WARNING (OPCUA_TRACE_STACK_WARNING)
#define OPCUA_TRACE_STACK_LEVEL_INFO    (OPCUA_TRACE_STACK_INFO )

#define OPCUA_TRACE_SERVER_LEVEL_DEBUG   (OPCUA_TRACE_SERVER_WARNING | OPCUA_TRACE_SERVER_INFO | OPCUA_TRACE_SERVER_ERROR )
#define OPCUA_TRACE_SERVER_LEVEL_ERROR   (OPCUA_TRACE_SERVER_ERROR)
#define OPCUA_TRACE_SERVER_LEVEL_WARNING (OPCUA_TRACE_SERVER_WARNING)
#define OPCUA_TRACE_SERVER_LEVEL_INFO    (OPCUA_TRACE_SERVER_INFO )

#define OPCUA_TRACE_CLIENT_LEVEL_DEBUG   (OPCUA_TRACE_CLIENT_WARNING | OPCUA_TRACE_CLIENT_INFO | OPCUA_TRACE_CLIENT_ERROR )
#define OPCUA_TRACE_CLIENT_LEVEL_ERROR   (OPCUA_TRACE_CLIENT_ERROR)
#define OPCUA_TRACE_CLIENT_LEVEL_WARNING (OPCUA_TRACE_CLIENT_WARNING)
#define OPCUA_TRACE_CLIENT_LEVEL_INFO    (OPCUA_TRACE_CLIENT_INFO )

#define OPCUA_TRACE_EXTRA_LEVEL_DEBUG   (OPCUA_TRACE_EXTRA_WARNING | OPCUA_TRACE_EXTRA_INFO | OPCUA_TRACE_EXTRA_ERROR )
#define OPCUA_TRACE_EXTRA_LEVEL_ERROR   (OPCUA_TRACE_EXTRA_ERROR)
#define OPCUA_TRACE_EXTRA_LEVEL_WARNING (OPCUA_TRACE_EXTRA_WARNING)
#define OPCUA_TRACE_EXTRA_LEVEL_INFO    (OPCUA_TRACE_EXTRA_INFO )
////////////////////////////////////////////////////////////////////
// Use to setup the trace itself
#define OPCUA_TRACE_OUTPUT_ALL_ERROR		(OPCUA_TRACE_STACK_LEVEL_DEBUG |  OPCUA_TRACE_SERVER_LEVEL_DEBUG | OPCUA_TRACE_CLIENT_LEVEL_DEBUG | OPCUA_TRACE_EXTRA_LEVEL_DEBUG)
#define OPCUA_TRACE_OUTPUT_STACK_ERROR		OPCUA_TRACE_STACK_LEVEL_ERROR
#define OPCUA_TRACE_OUTPUT_SERVER_ERROR		OPCUA_TRACE_SERVER_LEVEL_ERROR
#define OPCUA_TRACE_OUTPUT_CLIENT_ERROR		OPCUA_TRACE_CLIENT_ERROR
#define OPCUA_TRACE_OUTPUT_EXTRA_ERROR		OPCUA_TRACE_EXTRA_LEVEL_ERROR

#define OPCUA_TRACE_OUTPUT_ALL_DEBUG		(OPCUA_TRACE_STACK_LEVEL_DEBUG |  OPCUA_TRACE_SERVER_LEVEL_DEBUG | OPCUA_TRACE_EXTRA_LEVEL_DEBUG)
#define OPCUA_TRACE_OUTPUT_STACK_DEBUG		OPCUA_TRACE_STACK_LEVEL_DEBUG
#define OPCUA_TRACE_OUTPUT_SERVER_DEBUG		OPCUA_TRACE_SERVER_LEVEL_DEBUG
#define OPCUA_TRACE_OUTPUT_EXTRA_DEBUG		OPCUA_TRACE_EXTRA_LEVEL_DEBUG

#define OPCUA_TRACE_OUTPUT_ALL_WARNING		(OPCUA_TRACE_STACK_LEVEL_WARNING |  OPCUA_TRACE_SERVER_LEVEL_WARNING | OPCUA_TRACE_EXTRA_LEVEL_WARNING)
#define OPCUA_TRACE_OUTPUT_STACK_WARNING	OPCUA_TRACE_STACK_LEVEL_WARNING
#define OPCUA_TRACE_OUTPUT_SERVER_WARNING	OPCUA_TRACE_SERVER_LEVEL_WARNING
#define OPCUA_TRACE_OUTPUT_EXTRA_WARNING	OPCUA_TRACE_EXTRA_LEVEL_WARNING

#define OPCUA_TRACE_OUTPUT_ALL_INFO			(OPCUA_TRACE_STACK_LEVEL_INFO | OPCUA_TRACE_SERVER_LEVEL_INFO | OPCUA_TRACE_EXTRA_LEVEL_INFO)
#define OPCUA_TRACE_OUTPUT_STACK_INFO		OPCUA_TRACE_STACK_LEVEL_INFO
#define OPCUA_TRACE_OUTPUT_SERVER_INFO		OPCUA_TRACE_SERVER_LEVEL_INFO
#define OPCUA_TRACE_OUTPUT_EXTRA_INFO		OPCUA_TRACE_EXTRA_LEVEL_INFO

#define OPCUA_TRACE_OUTPUT_LEVEL_ALL     (0xFFFFFFFF)
#define OPCUA_TRACE_OUTPUT_LEVEL_NONE    (0x00000000)

	// InitializeAbstractionLayer
	OpcUa_StatusCode OpenOpcUa_InitializeAbstractionLayer(OpcUa_CharA* szApplicationName, OpcUa_Handle* hApplication, OpcUa_UInt32 uiDefaultTimeout=15000);
	// InitializeSecurity
	OpcUa_StatusCode OpenOpcUa_InitializeSecurity(OpcUa_Handle hApplication, OpcUa_String szCertificateStore);
	// Clear all the ressoucrces allocated by the application
	OpcUa_StatusCode OpenOpcUa_ClearAbstractionLayer(OpcUa_Handle hApplication);
	// FindServers
	OpcUa_StatusCode OpenOpcUa_FindServers(OpcUa_Handle hApplication,
		OpcUa_String* hostName,
		OpcUa_Int32* uiNoOfServer,
		OpcUa_ApplicationDescription** pServers);
	// GetEndpoints
	OpcUa_StatusCode OpenOpcUa_GetEndpoints(OpcUa_Handle hApplication, OpcUa_String* discoveryUrl, OpcUa_UInt32* uiNoOfEndpointDescription, OpcUa_EndpointDescription** ppEndpointDescription);

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
	OpcUa_StatusCode OpenOpcUa_ActivateSession(OpcUa_Handle hApplication, OpcUa_Handle hSession,  OpcUa_EndpointDescription* pEndpointDescription,const char * AuthenticationMode, const char * sUserName, const char * sPassword);
	// Close Session
	OpcUa_StatusCode OpenOpcUa_CloseSession(OpcUa_Handle hApplication, OpcUa_Handle hSession);
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
	OpcUa_StatusCode OpenOpcUa_DeleteSubscription(OpcUa_Handle hApplication, OpcUa_Handle hSession, OpcUa_Handle hSubscription);
	// Modify Subscription
	OpcUa_StatusCode OpenOpcUa_ModifySubscription(OpcUa_Handle hApplication, OpcUa_Handle hSession, OpcUa_Handle hSubscription,
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
	OpcUa_StatusCode OpenOpcUa_CreateMonitoredItems(OpcUa_Handle hApplication, OpcUa_Handle hSession, OpcUa_Handle hSubscription,
													OpcUa_Byte aTimestampsToReturn,
													OpcUa_UInt32 NoOfItemsToCreate,
													OpcUa_MonitoredItemCreateRequest* pItemsToCreate,
													OpcUa_MonitoredItemCreateResult** ppResult,
													OpcUa_Handle** hMonitoredItems);
	// Create MonitoredItems easier and more powerfull function
	OpcUa_StatusCode OpenOpcUa_CreateMonitoredItemsEx(OpcUa_Handle						hApplication,// in
														OpcUa_Handle					hSession, // in
														OpcUa_Handle					hSubscription,// in
														OpcUa_UInt32					NoOfItemsToCreate, // in
														OpcUa_MonitoredItemToCreate*	pItemsToCreate, // in
														OpcUa_MonitoredItemCreated**	ppItemsCreated); // out
	// Delete MonitoredItems
	OpcUa_StatusCode OpenOpcUa_DeleteMonitoredItems(OpcUa_Handle hApplication, OpcUa_Handle hSession, OpcUa_Handle hSubscription,
		OpcUa_Int32 iNoOfMonitoredItem,
		OpcUa_Handle* hMonitoredItems,
		OpcUa_StatusCode** ppStatusCode);
	// Modify MonitoredItems
	OpcUa_StatusCode OpenOpcUa_ModifyMonitoredItems(OpcUa_Handle	hApplication,
		OpcUa_Handle	hSession,
		OpcUa_Handle	hSubscription,
		OpcUa_Handle	hMonitoredItems,
		OpcUa_Byte		aTimestampsToReturn,
		OpcUa_Boolean	bDiscardOldest,
		OpcUa_UInt32	uiQueueSize,
		OpcUa_Double	dblSamplingInterval,
		OpcUa_UInt32	DeadbandType,
		OpcUa_Double	DeadbandValue,
		OpcUa_Byte		DatachangeTrigger);

	// SetMonitoringMode
	OpcUa_StatusCode OpenOpcUa_SetMonitoringMode(OpcUa_Handle hApplication, OpcUa_Handle hSession, OpcUa_Handle hSubscription,
		OpcUa_Int32			iNoOfMonitoredItem,
		OpcUa_Handle*			hMonitoredItems,
		OpcUa_MonitoringMode	eMonitoringMode);
	// Give a string representation of an StatusCode
	OpcUa_StatusCode OpenOpcUa_StatusCodeToString(OpcUa_StatusCode uStatus, OpcUa_String** ppszStatusCode);
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
	OpcUa_StatusCode OpenOpcUa_GetSessionOfSubscription(OpcUa_Handle hSubscription, OpcUa_Handle* hSession);
	// Get the OpenOpcUa_InternalNode based on the hMonitoredItem
	OpcUa_StatusCode OpenOpcUa_GetInternalNode(OpcUa_Handle hApplication,// In
		OpcUa_Handle hSession,// In
		OpcUa_Handle hSubscription,// In
		OpcUa_Handle hMonitoredItem,// In
		OpenOpcUa_InternalNode** pInternalNode);// Out
	// Get the InternalNode on based on the MonitoredItemId
	OpcUa_StatusCode OpenOpcUa_GetInternalNodeByMonitoredItemId(OpcUa_Handle hApplication, // In
		OpcUa_Handle hSession, // In
		OpcUa_Handle hSubscription, // In
		OpcUa_UInt32 MonitoredItemId, // In
		OpenOpcUa_InternalNode** pInternalNode); // Out
	OpcUa_StatusCode OpenOpcUa_GetInternalNodeByClientHandle(OpcUa_Handle hApplication,
		OpcUa_Handle hSession,
		OpcUa_Handle hSubscription,
		OpcUa_UInt32 ClientHandle,
		OpenOpcUa_InternalNode** pInternalNode);
	//Release the memory allocated for this InternalNode. Every call to GetInternalNode or to GetInternalNodeByMonitoredItemId require a call to ReleaseInternalNode
	OpcUa_StatusCode OpenOpcUa_ReleaseInternalNode(OpenOpcUa_InternalNode* pInternalNode);

	//////////////////////////////////////////////////////////////////////////////////////////
	// Callback functions
	typedef OpcUa_StatusCode( *PFUNC)(
		OpcUa_Handle hSubscription,
		OpcUa_Int32 NoOfMonitoredItems,
		OpcUa_MonitoredItemNotification* MonitoredItems, void* pParamData);

	OpcUa_StatusCode OpenOpcUa_SetPublishCallback(OpcUa_Handle hApplication,
		OpcUa_Handle hSession,
		OpcUa_Handle hSubscription,
		PFUNC lpCallback, void* pCallbackData);
	typedef OpcUa_StatusCode( *PFUNCSHUTDOWN)(
		OpcUa_Handle hApplication, OpcUa_Handle hSession,
		OpcUa_String strShutdownReason,
		void* pParamData);
	OpcUa_StatusCode OpenOpcUa_SetShutdownCallback(OpcUa_Handle hApplication,
		OpcUa_Handle hSession,
		PFUNCSHUTDOWN lpCallback, void* pCallbackData);
	/////////////////////////////////////////////////////////////////////////////
	// Helper Function
	OpcUa_StatusCode OpenOpcUa_VariantToString(OpcUa_Variant& Var, OpcUa_String** strValue);
	OpcUa_StatusCode OpenOpcUa_DateTimeToString(OpcUa_DateTime aDateTime, OpcUa_String** strTime);
	OpcUa_StatusCode OpenOpcUa_StringToNodeId(OpcUa_String strNodeId, OpcUa_NodeId* pNodeId);
	OpcUa_StatusCode OpenOpcUa_NodeIdToString(OpcUa_NodeId aNodeId, OpcUa_String* strNodeId);
	OpcUa_StatusCode OpenOpcUa_Browse(OpcUa_Handle hApplication,
		OpcUa_Handle hSession,
		OpcUa_Int32 iNoOfNodesToBrowse,
		const OpcUa_BrowseDescription* pNodesToBrowse,
		OpcUa_Int32* iNoOfReferenceDescription,
		OpcUa_ReferenceDescription** pReferenceList);
	//
	// Function for configuration file manipulation. Configuration file are conform to OpenOpcUaClientConfig.xsd
	OpcUa_StatusCode OpenOpcUa_LoadConfig(OpcUa_Handle hApplication, OpcUa_String szConfigFileName);
	OpcUa_StatusCode OpenOpcUa_SaveConfig(OpcUa_Handle hApplication, OpcUa_String szConfigFileName);
	// Retreive hSessions for a hApplication
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
	OpcUa_StatusCode OpenOpcUa_Trace(OpcUa_Handle hApplication, OpcUa_UInt32 a_uTraceLevel, const OpcUa_CharA* format, ...);
	// this function will provide the details for an attributes. This is not related to hApplication or hSession 
	// This function is just an helper
	OpcUa_StatusCode OpenOpcUa_GetAttributeDetails(OpcUa_Int32 iAttributeId, OpcUa_String** szAttributeName, OpcUa_String** szAttributeDescription);
	// This function will provide detail for BuiltInTypes. This is just an helper function
	// Application have to release allocated ressources, szBuiltInTypeName, szBuiltInTypeDescription
	OpcUa_StatusCode OpenOpcUa_GetBuiltInTypeDetails(OpcUa_Int32 iBuiltInTypeId, OpcUa_String** szBuiltInTypeName, OpcUa_String** szBuiltInTypeDescription);
	// This function will lookup the className from the enum number to a OpcUa_String
	// Application have to release allocated ressources, szBuiltInTypeName, szBuiltInTypeDescription
	OpcUa_StatusCode OpenOpcUa_GetNodeClassDetails(OpcUa_Int32 iNodeClass, OpcUa_String** szNodeClassName);
	// Retrieve the dataType of the UAVariable (aNodeId) 
	OpcUa_StatusCode OpenOpcUa_GetUAVariableDatatype(OpcUa_Handle hApplication, OpcUa_Handle hSession, OpcUa_NodeId aNodeId, OpcUa_NodeId* pDataType);
	// Transform an OpcUa_String to OpcUa_Variant according to the requested DataType
	OpcUa_StatusCode OpenOpcUa_StringToVariant(OpcUa_String* pString, OpcUa_Int32 iDataType, OpcUa_Variant** pVariant);
	// This function will lookup the userAccessLevel and accessLevel
	OpcUa_StatusCode OpenOpcUa_GetAccessLevel(OpcUa_Byte AccessLevel, OpcUa_String** szAccessLevel);
	OpcUa_StatusCode OpenOpcUa_GetUserAccessLevel(OpcUa_Byte AccessLevel, OpcUa_String** szAccessLevel);
	// Create a Filter Extension Object according to the parameter receive
	OpcUa_StatusCode OpenOpcUa_CreateFilterObject(OpcUa_UInt32 DeadbandType, OpcUa_Double DeadbandValue, OpcUa_Byte DatachangeTrigger, OpcUa_ExtensionObject** ppExtensionObject);
#endif // OPENOPCUA_CLIENT_LIBRARY
	// for debug purpose with Visual Detector
#ifdef SEEK_LEAKER
#include "vld.h"
#endif


#if OPCUA_TRACE_ENABLE
#if OPCUA_TRACE_FILE_LINE_INFO
#define OpcUa_Trace(xLevel, xFormat, ...) OpcUa_Trace_Imp(xLevel, xFormat, __FILE__, __LINE__, __VA_ARGS__)
#else /* OPCUA_TRACE_FILE_LINE_INFO */
#define OpcUa_Trace OpcUa_Trace_Imp
#endif /* OPCUA_TRACE_FILE_LINE_INFO */
#else /* OPCUA_TRACE_ENABLE */
#define OpcUa_Trace(xLevel, xFormat, ...) 
#endif /* OPCUA_TRACE_ENABLE */

	OpcUa_Boolean OpcUa_Trace_Imp(
		OpcUa_UInt32 uTraceLevel,
		const OpcUa_CharA* sFormat,
#if OPCUA_TRACE_FILE_LINE_INFO
		const OpcUa_CharA* sFile,
		OpcUa_UInt32 sLine,
#endif /* OPCUA_TRACE_FILE_LINE_INFO */
		...);


	/* shortcuts for often used memory functions */
#define OpcUa_Alloc(xSize)                              malloc(xSize)
	//#define OpcUa_Alloc(xSize)                              OpcUa_Memory_Alloc(xSize)
#define OpcUa_ReAlloc(xSrc, xSize)                      OpcUa_Memory_ReAlloc(xSrc, xSize)
	//#define OpcUa_Free(xSrc)                                OpcUa_Memory_Free(xSrc)
#define OpcUa_Free(xSrc)                                free(xSrc)
#define OpcUa_MemCpy(xDst, xDstSize, xSrc, xCount)      OpcUa_Memory_MemCpy(xDst, xDstSize, xSrc, xCount)

	OPCUA_BEGIN_EXTERN_C
	OpcUa_Void* OpcUa_Memory_Alloc(OpcUa_UInt32 nSize);
	OpcUa_Void OpcUa_Memory_Free(OpcUa_Void* pvBuffer);
	OpcUa_DateTime OpcUa_DateTime_UtcNow();
OPCUA_END_EXTERN_C
