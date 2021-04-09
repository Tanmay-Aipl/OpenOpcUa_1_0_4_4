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

// A null terminated table of services that is used by the stack to dispatch incoming requests.
extern OpcUa_ServiceType* g_SupportedServices[];


// Called when a FindServers request arrives from a Client.
extern OpcUa_StatusCode Server_FindServers(
	OpcUa_Endpoint                 a_hEndpoint,
	OpcUa_Handle                   a_hContext,
	const OpcUa_RequestHeader*     a_pRequestHeader,
	const OpcUa_String*            a_pEndpointUrl,
	OpcUa_Int32                    a_nNoOfLocaleIds,
	const OpcUa_String*            a_pLocaleIds,
	OpcUa_Int32                    a_nNoOfServerUris,
	const OpcUa_String*            a_pServerUris,
	OpcUa_ResponseHeader*          a_pResponseHeader,
	OpcUa_Int32*                   a_pNoOfServers,
	OpcUa_ApplicationDescription** a_pServers);

// Called when a CreateSession request arrives from a Client.
extern OpcUa_StatusCode Server_GetEndpoints(
	OpcUa_Endpoint              a_hEndpoint,
	OpcUa_Handle                a_hContext,
	const OpcUa_RequestHeader*  a_pRequestHeader,
	const OpcUa_String*         a_pEndpointUrl,
	OpcUa_Int32                 a_nNoOfLocaleIds,
	const OpcUa_String*         a_pLocaleIds,
	OpcUa_Int32                 a_nNoOfProfileUris,
	const OpcUa_String*         a_pProfileUris,
	OpcUa_ResponseHeader*       a_pResponseHeader,
	OpcUa_Int32*                a_pNoOfEndpoints,
	OpcUa_EndpointDescription** a_pEndpoints);

// Called when a CreateSession request arrives from a Client.
extern OpcUa_StatusCode Server_CreateSession(
	OpcUa_Endpoint                      a_hEndpoint,
	OpcUa_Handle                        a_hContext,
	const OpcUa_RequestHeader*          a_pRequestHeader,
	const OpcUa_ApplicationDescription* a_pClientDescription,
	const OpcUa_String*                 a_pServerUri,
	const OpcUa_String*                 a_pEndpointUrl,
	const OpcUa_String*                 a_pSessionName,
	const OpcUa_ByteString*             a_pClientNonce,
	const OpcUa_ByteString*             a_pClientCertificate,
	OpcUa_Double                        a_nRequestedSessionTimeout,
	OpcUa_UInt32                        a_nMaxResponseMessageSize,
	OpcUa_ResponseHeader*               a_pResponseHeader,
	OpcUa_NodeId*                       a_pSessionId,
	OpcUa_NodeId*                       a_pAuthenticationToken,
	OpcUa_Double*                       a_pRevisedSessionTimeout,
	OpcUa_ByteString*                   a_pServerNonce,
	OpcUa_ByteString*                   a_pServerCertificate,
	OpcUa_Int32*                        a_pNoOfServerEndpoints,
	OpcUa_EndpointDescription**         a_pServerEndpoints,
	OpcUa_Int32*                        a_pNoOfServerSoftwareCertificates,
	OpcUa_SignedSoftwareCertificate**   a_pServerSoftwareCertificates,
	OpcUa_SignatureData*                a_pServerSignature,
	OpcUa_UInt32*                       a_pMaxRequestMessageSize);

// Called when a ActivateSession request arrives from a Client.
extern OpcUa_StatusCode Server_ActivateSession(
	OpcUa_Endpoint                         a_hEndpoint,
	OpcUa_Handle                           a_hContext,
	const OpcUa_RequestHeader*             a_pRequestHeader,
	const OpcUa_SignatureData*             a_pClientSignature,
	OpcUa_Int32                            a_nNoOfClientSoftwareCertificates,
	const OpcUa_SignedSoftwareCertificate* a_pClientSoftwareCertificates,
	OpcUa_Int32                            a_nNoOfLocaleIds,
	const OpcUa_String*                    a_pLocaleIds,
	const OpcUa_ExtensionObject*           a_pUserIdentityToken,
	const OpcUa_SignatureData*             a_pUserTokenSignature,
	OpcUa_ResponseHeader*                  a_pResponseHeader,
	OpcUa_ByteString*                      a_pServerNonce,
	OpcUa_Int32*                           a_pNoOfResults,
	OpcUa_StatusCode**                     a_pResults,
	OpcUa_Int32*                           a_pNoOfDiagnosticInfos,
	OpcUa_DiagnosticInfo**                 a_pDiagnosticInfos);

// Called when a CloseSession request arrives from a Client.
extern OpcUa_StatusCode Server_CloseSession(
	OpcUa_Endpoint             a_hEndpoint,
	OpcUa_Handle               a_hContext,
	const OpcUa_RequestHeader* a_pRequestHeader,
	OpcUa_Boolean              a_bDeleteSubscriptions,
	OpcUa_ResponseHeader*      a_pResponseHeader);

// Called to cancel outstandign service request
extern OpcUa_StatusCode Server_Cancel(
	OpcUa_Endpoint             a_hEndpoint,
	OpcUa_Handle               a_hContext,
	const OpcUa_RequestHeader* a_pRequestHeader,
	OpcUa_UInt32               a_nRequestHandle,
	OpcUa_ResponseHeader*      a_pResponseHeader,
	OpcUa_UInt32*              a_pCancelCount);

// Called when a Publish request is received from the Client.
extern OpcUa_StatusCode Server_BeginPublish(
	OpcUa_Endpoint        a_hEndpoint,
	OpcUa_Handle          a_hContext,
	OpcUa_Void**          a_ppRequest,
	OpcUa_EncodeableType* a_pRequestType);

// Called when a QueryFirst is called by the client
OpcUa_StatusCode Server_BeginQueryFirst(
	OpcUa_Endpoint        a_hEndpoint,
	OpcUa_Handle          a_hContext,
	OpcUa_Void**          a_ppRequest,
	OpcUa_EncodeableType* a_pRequestType);

// Called when a QueryNext is called by the client
OpcUa_StatusCode Server_BeginQueryNext(
	OpcUa_Endpoint        a_hEndpoint,
	OpcUa_Handle          a_hContext,
	OpcUa_Void**          a_ppRequest,
	OpcUa_EncodeableType* a_pRequestType);

// Called when a Read request is received from the Client.
extern OpcUa_StatusCode Server_BeginRead(
	OpcUa_Endpoint        a_hEndpoint,
	OpcUa_Handle          a_hContext,
	OpcUa_Void**          a_ppRequest,
	OpcUa_EncodeableType* a_pRequestType);

// Called when a Write request is received from the Client. This call is a Synchronous Write
extern OpcUa_StatusCode Server_Write(
	OpcUa_Endpoint             a_hEndpoint,
	OpcUa_Handle               a_hContext,
	const OpcUa_RequestHeader* a_pRequestHeader,
	OpcUa_Int32                a_nNoOfNodesToWrite,
	const OpcUa_WriteValue*    a_pNodesToWrite,
	OpcUa_ResponseHeader*      a_pResponseHeader,
	OpcUa_Int32*               a_pNoOfResults,
	OpcUa_StatusCode**         a_pResults,
	OpcUa_Int32*               a_pNoOfDiagnosticInfos,
	OpcUa_DiagnosticInfo**     a_pDiagnosticInfos);

//// Called when the request timer expires.
//extern OpcUa_StatusCode OPCUA_DLLCALL Server_RequestTimerExpired( 
//	OpcUa_Void*  a_pvCallbackData, 
//	OpcUa_Timer  a_hTimer,
//	OpcUa_UInt32 a_msecElapsed);
//
//// Called when the request timer is killed.
//extern OpcUa_StatusCode OPCUA_DLLCALL Server_RequestTimerKilled( 
//	OpcUa_Void*  a_pvCallbackData, 
//	OpcUa_Timer  a_hTimer,
//	OpcUa_UInt32 a_msecElapsed);

// A function that receives endpoint events from the stack.
//extern OpcUa_StatusCode Server_EndpointCallback( 
//    OpcUa_Endpoint          a_hEndpoint,
//	OpcUa_Void*             a_pCallbackData,
//    OpcUa_Endpoint_Event    a_eEvent,
//    OpcUa_StatusCode        a_uStatus,
//    OpcUa_UInt32            a_uSecureChannelId,
//    OpcUa_ByteString*       a_pbsClientCertificate,
//	OpcUa_String*           a_pSecurityPolicy,
//	OpcUa_MessageSecurityMode            a_uSecurityMode);
extern OpcUa_StatusCode Server_EndpointCallback( 
	OpcUa_Endpoint          a_hEndpoint,
	OpcUa_Void*             a_pvCallbackData,
	OpcUa_Endpoint_Event    a_eEvent,
	OpcUa_StatusCode        a_uStatus,
	OpcUa_UInt32            a_uSecureChannelId,
	OpcUa_Handle*           a_phRawRequestContext,
	OpcUa_ByteString*       a_pbsClientCertificate,
	OpcUa_String*           a_pSecurityPolicy,
	OpcUa_UInt16            a_uSecurityMode,
	OpcUa_UInt32            a_uRequestedLifetime);

// A function that handle the browse request from the stack
extern OpcUa_StatusCode Server_Browse(
	OpcUa_Endpoint                 a_hEndpoint,
	OpcUa_Handle                   a_hContext,
	const OpcUa_RequestHeader*     a_pRequestHeader,
	const OpcUa_ViewDescription*   a_pView,
	OpcUa_UInt32                   a_nRequestedMaxReferencesPerNode,
	OpcUa_Int32                    a_nNoOfNodesToBrowse,
	const OpcUa_BrowseDescription* a_pNodesToBrowse,
	OpcUa_ResponseHeader*          a_pResponseHeader,
	OpcUa_Int32*                   a_pNoOfResults,
	OpcUa_BrowseResult**           a_pResults,
	OpcUa_Int32*                   a_pNoOfDiagnosticInfos,
	OpcUa_DiagnosticInfo**         a_pDiagnosticInfos);
// 
extern OpcUa_StatusCode Server_BrowseNext(
	OpcUa_Endpoint             a_hEndpoint,
	OpcUa_Handle               a_hContext,
	const OpcUa_RequestHeader* a_pRequestHeader,
	OpcUa_Boolean              a_bReleaseContinuationPoints,
	OpcUa_Int32                a_nNoOfContinuationPoints,
	const OpcUa_ByteString*    a_pContinuationPoints,
	OpcUa_ResponseHeader*      a_pResponseHeader,
	OpcUa_Int32*               a_pNoOfResults,
	OpcUa_BrowseResult**       a_pResults,
	OpcUa_Int32*               a_pNoOfDiagnosticInfos,
	OpcUa_DiagnosticInfo**     a_pDiagnosticInfos);
// A function that handle the create subscription coming from clients through the stack
extern OpcUa_StatusCode Server_CreateSubscription(
	OpcUa_Endpoint             a_hEndpoint,
	OpcUa_Handle               a_hContext,
	const OpcUa_RequestHeader* a_pRequestHeader,
	OpcUa_Double               a_nRequestedPublishingInterval,
	OpcUa_UInt32               a_nRequestedLifetimeCount,
	OpcUa_UInt32               a_nRequestedMaxKeepAliveCount,
	OpcUa_UInt32               a_nMaxNotificationsPerPublish,
	OpcUa_Boolean              a_bPublishingEnabled,
	OpcUa_Byte                 a_nPriority,
	OpcUa_ResponseHeader*      a_pResponseHeader,
	OpcUa_UInt32*              a_pSubscriptionId,
	OpcUa_Double*              a_pRevisedPublishingInterval,
	OpcUa_UInt32*              a_pRevisedLifetimeCount,
	OpcUa_UInt32*              a_pRevisedMaxKeepAliveCount);

extern OpcUa_StatusCode Server_ModifySubscription(
	OpcUa_Endpoint             a_hEndpoint,
	OpcUa_Handle               a_hContext,
	const OpcUa_RequestHeader* a_pRequestHeader,
	OpcUa_UInt32               a_nSubscriptionId,
	OpcUa_Double               a_nRequestedPublishingInterval,
	OpcUa_UInt32               a_nRequestedLifetimeCount,
	OpcUa_UInt32               a_nRequestedMaxKeepAliveCount,
	OpcUa_UInt32               a_nMaxNotificationsPerPublish,
	OpcUa_Byte                 a_nPriority,
	OpcUa_ResponseHeader*      a_pResponseHeader,
	OpcUa_Double*              a_pRevisedPublishingInterval,
	OpcUa_UInt32*              a_pRevisedLifetimeCount,
	OpcUa_UInt32*              a_pRevisedMaxKeepAliveCount);

extern OpcUa_StatusCode Server_DeleteSubscriptions(
	OpcUa_Endpoint             a_hEndpoint,
	OpcUa_Handle               a_hContext,
	const OpcUa_RequestHeader* a_pRequestHeader,
	OpcUa_Int32                a_nNoOfSubscriptionIds,
	const OpcUa_UInt32*        a_pSubscriptionIds,
	OpcUa_ResponseHeader*      a_pResponseHeader,
	OpcUa_Int32*               a_pNoOfResults,
	OpcUa_StatusCode**         a_pResults,
	OpcUa_Int32*               a_pNoOfDiagnosticInfos,
	OpcUa_DiagnosticInfo**     a_pDiagnosticInfos);

extern OpcUa_StatusCode Server_TransferSubscriptions(
	OpcUa_Endpoint             a_hEndpoint,
	OpcUa_Handle               a_hContext,
	const OpcUa_RequestHeader* a_pRequestHeader,
	OpcUa_Int32                a_nNoOfSubscriptionIds,
	const OpcUa_UInt32*        a_pSubscriptionIds,
	OpcUa_Boolean              a_bSendInitialValues,
	OpcUa_ResponseHeader*      a_pResponseHeader,
	OpcUa_Int32*               a_pNoOfResults,
	OpcUa_TransferResult**     a_pResults,
	OpcUa_Int32*               a_pNoOfDiagnosticInfos,
	OpcUa_DiagnosticInfo**     a_pDiagnosticInfos);

extern OpcUa_StatusCode Server_SetPublishingMode(
	OpcUa_Endpoint             a_hEndpoint,
	OpcUa_Handle               a_hContext,
	const OpcUa_RequestHeader* a_pRequestHeader,
	OpcUa_Boolean              a_bPublishingEnabled,
	OpcUa_Int32                a_nNoOfSubscriptionIds,
	const OpcUa_UInt32*        a_pSubscriptionIds,
	OpcUa_ResponseHeader*      a_pResponseHeader,
	OpcUa_Int32*               a_pNoOfResults,
	OpcUa_StatusCode**         a_pResults,
	OpcUa_Int32*               a_pNoOfDiagnosticInfos,
	OpcUa_DiagnosticInfo**     a_pDiagnosticInfos);

//extern OpcUa_StatusCode Server_Publish(
//    OpcUa_Endpoint                           a_hEndpoint,
//    OpcUa_Handle                             a_hContext,
//    const OpcUa_RequestHeader*               a_pRequestHeader,
//    OpcUa_Int32                              a_nNoOfSubscriptionAcknowledgements,
//    OpcUa_SubscriptionAcknowledgement*		 a_pSubscriptionAcknowledgements,
//    OpcUa_ResponseHeader*                    a_pResponseHeader,
//    OpcUa_UInt32*                            a_pSubscriptionId,
//    OpcUa_Int32*                             a_pNoOfAvailableSequenceNumbers,
//    OpcUa_UInt32**                           a_pAvailableSequenceNumbers,
//    OpcUa_Boolean*                           a_pMoreNotifications,
//    OpcUa_NotificationMessage*               a_pNotificationMessage,
//    OpcUa_Int32*                             a_pNoOfResults,
//    OpcUa_StatusCode**                       a_pResults,
//    OpcUa_Int32*                             a_pNoOfDiagnosticInfos,
//    OpcUa_DiagnosticInfo**                   a_pDiagnosticInfos);

extern OpcUa_StatusCode Server_Republish(
	OpcUa_Endpoint             a_hEndpoint,
	OpcUa_Handle               a_hContext,
	const OpcUa_RequestHeader* a_pRequestHeader,
	OpcUa_UInt32               a_nSubscriptionId,
	OpcUa_UInt32               a_nRetransmitSequenceNumber,
	OpcUa_ResponseHeader*      a_pResponseHeader,
	OpcUa_NotificationMessage* a_pNotificationMessage);

extern OpcUa_StatusCode Server_CreateMonitoredItems(
	OpcUa_Endpoint                          a_hEndpoint,
	OpcUa_Handle                            a_hContext,
	const OpcUa_RequestHeader*              a_pRequestHeader,
	OpcUa_UInt32                            a_nSubscriptionId,
	OpcUa_TimestampsToReturn                a_eTimestampsToReturn,
	OpcUa_Int32                             a_nNoOfItemsToCreate,
	const OpcUa_MonitoredItemCreateRequest* a_pItemsToCreate,
	OpcUa_ResponseHeader*                   a_pResponseHeader,
	OpcUa_Int32*                            a_pNoOfResults,
	OpcUa_MonitoredItemCreateResult**       a_pResults,
	OpcUa_Int32*                            a_pNoOfDiagnosticInfos,
	OpcUa_DiagnosticInfo**                  a_pDiagnosticInfos);

extern OpcUa_StatusCode Server_ModifyMonitoredItems(
	OpcUa_Endpoint                          a_hEndpoint,
	OpcUa_Handle                            a_hContext,
	const OpcUa_RequestHeader*              a_pRequestHeader,
	OpcUa_UInt32                            a_nSubscriptionId,
	OpcUa_TimestampsToReturn                a_eTimestampsToReturn,
	OpcUa_Int32                             a_nNoOfItemsToModify,
	const OpcUa_MonitoredItemModifyRequest* a_pItemsToModify,
	OpcUa_ResponseHeader*                   a_pResponseHeader,
	OpcUa_Int32*                            a_pNoOfResults,
	OpcUa_MonitoredItemModifyResult**       a_pResults,
	OpcUa_Int32*                            a_pNoOfDiagnosticInfos,
	OpcUa_DiagnosticInfo**                  a_pDiagnosticInfos);

extern OpcUa_StatusCode Server_DeleteMonitoredItems(
	OpcUa_Endpoint             a_hEndpoint,
	OpcUa_Handle               a_hContext,
	const OpcUa_RequestHeader* a_pRequestHeader,
	OpcUa_UInt32               a_nSubscriptionId,
	OpcUa_Int32                a_nNoOfMonitoredItemIds,
	const OpcUa_UInt32*        a_pMonitoredItemIds,
	OpcUa_ResponseHeader*      a_pResponseHeader,
	OpcUa_Int32*               a_pNoOfResults,
	OpcUa_StatusCode**         a_pResults,
	OpcUa_Int32*               a_pNoOfDiagnosticInfos,
	OpcUa_DiagnosticInfo**     a_pDiagnosticInfos);

extern OpcUa_StatusCode Server_SetMonitoringMode(
	OpcUa_Endpoint             a_hEndpoint,
	OpcUa_Handle               a_hContext,
	const OpcUa_RequestHeader* a_pRequestHeader,
	OpcUa_UInt32               a_nSubscriptionId,
	OpcUa_MonitoringMode       a_eMonitoringMode,
	OpcUa_Int32                a_nNoOfMonitoredItemIds,
	const OpcUa_UInt32*        a_pMonitoredItemIds,
	OpcUa_ResponseHeader*      a_pResponseHeader,
	OpcUa_Int32*               a_pNoOfResults,
	OpcUa_StatusCode**         a_pResults,
	OpcUa_Int32*               a_pNoOfDiagnosticInfos,
	OpcUa_DiagnosticInfo**     a_pDiagnosticInfos);

extern OpcUa_StatusCode Server_SetTriggering(
	OpcUa_Endpoint             a_hEndpoint,
	OpcUa_Handle               a_hContext,
	const OpcUa_RequestHeader* a_pRequestHeader,
	OpcUa_UInt32               a_nSubscriptionId,
	OpcUa_UInt32               a_nTriggeringItemId,
	OpcUa_Int32                a_nNoOfLinksToAdd,
	const OpcUa_UInt32*        a_pLinksToAdd,
	OpcUa_Int32                a_nNoOfLinksToRemove,
	const OpcUa_UInt32*        a_pLinksToRemove,
	OpcUa_ResponseHeader*      a_pResponseHeader,
	OpcUa_Int32*               a_pNoOfAddResults,
	OpcUa_StatusCode**         a_pAddResults,
	OpcUa_Int32*               a_pNoOfAddDiagnosticInfos,
	OpcUa_DiagnosticInfo**     a_pAddDiagnosticInfos,
	OpcUa_Int32*               a_pNoOfRemoveResults,
	OpcUa_StatusCode**         a_pRemoveResults,
	OpcUa_Int32*               a_pNoOfRemoveDiagnosticInfos,
	OpcUa_DiagnosticInfo**     a_pRemoveDiagnosticInfos);

extern OpcUa_StatusCode Server_TranslateBrowsePathsToNodeIds(
	OpcUa_Endpoint             a_hEndpoint,
	OpcUa_Handle               a_hContext,
	const OpcUa_RequestHeader* a_pRequestHeader,
	OpcUa_Int32                a_nNoOfBrowsePaths,
	const OpcUa_BrowsePath*    a_pBrowsePaths,
	OpcUa_ResponseHeader*      a_pResponseHeader,
	OpcUa_Int32*               a_pNoOfResults,
	OpcUa_BrowsePathResult**   a_pResults,
	OpcUa_Int32*               a_pNoOfDiagnosticInfos,
	OpcUa_DiagnosticInfo**     a_pDiagnosticInfos);

extern OpcUa_StatusCode Server_RegisterNodes(
	OpcUa_Endpoint             a_hEndpoint,
	OpcUa_Handle               a_hContext,
	const OpcUa_RequestHeader* a_pRequestHeader,
	OpcUa_Int32                a_nNoOfNodesToRegister,
	const OpcUa_NodeId*        a_pNodesToRegister,
	OpcUa_ResponseHeader*      a_pResponseHeader,
	OpcUa_Int32*               a_pNoOfRegisteredNodeIds,
	OpcUa_NodeId**             a_pRegisteredNodeIds);

OpcUa_StatusCode Server_UnregisterNodes(
	OpcUa_Endpoint             a_hEndpoint,
	OpcUa_Handle               a_hContext,
	const OpcUa_RequestHeader* a_pRequestHeader,
	OpcUa_Int32                a_nNoOfNodesToUnregister,
	const OpcUa_NodeId*        a_pNodesToUnregister,
	OpcUa_ResponseHeader*      a_pResponseHeader);

//extern OpcUa_StatusCode Server_Call(
//    OpcUa_Endpoint                 a_hEndpoint,
//    OpcUa_Handle                   a_hContext,
//    const OpcUa_RequestHeader*     a_pRequestHeader,
//    OpcUa_Int32                    a_nNoOfMethodsToCall,
//    const OpcUa_CallMethodRequest* a_pMethodsToCall,
//    OpcUa_ResponseHeader*          a_pResponseHeader,
//    OpcUa_Int32*                   a_pNoOfResults,
//    OpcUa_CallMethodResult**       a_pResults,
//    OpcUa_Int32*                   a_pNoOfDiagnosticInfos,
//    OpcUa_DiagnosticInfo**         a_pDiagnosticInfos);
extern OpcUa_StatusCode Server_BeginCall(
	OpcUa_Endpoint        a_hEndpoint,
	OpcUa_Handle          a_hContext,
	OpcUa_Void**          a_ppRequest,
	OpcUa_EncodeableType* a_pRequestType);

extern OpcUa_StatusCode Server_BeginHistoryRead(
	OpcUa_Endpoint        a_hEndpoint,
	OpcUa_Handle          a_hContext,
	OpcUa_Void**          a_ppRequest,
	OpcUa_EncodeableType* a_pRequestType);