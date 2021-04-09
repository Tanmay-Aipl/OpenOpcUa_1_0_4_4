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
#ifdef _GNUC_
#include <dlfcn.h>
#endif
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
#include "StackCallbacks.h"
#include "MurmurHash3.h" // for random generation

using namespace OpenOpcUa;
using namespace UACoreServer;
extern OpcUa_Boolean g_bRun;
OpcUa_UInt32 GenerateRandonHandle(OpcUa_UInt32 uiHashVal)
{
	OpcUa_UInt32 uiHandle32=0;
	if (uiHashVal == 0)
		uiHashVal = 1;
	clock_t clockVal=OpcUa_Clock();
	OpcUa_Double dblVal = exp((clockVal*clockVal) / (2 * uiHashVal));
	uiHandle32 = (OpcUa_UInt32) dblVal * 1000000;
	return uiHandle32;
}


#ifdef WIN32
// A macro that declares the function mapping for a UA service.
#define DECLARE_SUPPORTED_SYNCHRONOUS_SERVICE(xService) \
OpcUa_ServiceType g_##xService##Service  = \
{ \
	OpcUaId_##xService##Request, \
	 &OpcUa_##xService##Response_EncodeableType, \
	(OpcUa_PfnBeginInvokeService*)OpcUa_Server_Begin##xService##, \
	(OpcUa_PfnInvokeService*)Server_##xService## \
}; 

#define DECLARE_SUPPORTED_ASYNCHRONOUS_SERVICE(xService) \
OpcUa_ServiceType g_##xService##Service  = \
{ \
	OpcUaId_##xService##Request, \
	 &OpcUa_##xService##Response_EncodeableType, \
	(OpcUa_PfnBeginInvokeService*)Server_Begin##xService##, \
	OpcUa_Null \
}; 
#endif
#ifdef _GNUC_
// A macro that declares the function mapping for a UA service.
#define DECLARE_SUPPORTED_SYNCHRONOUS_SERVICE(xService) \
OpcUa_ServiceType g_##xService##Service  = \
{ \
	  OpcUaId_##xService##Request, \
	   &OpcUa_##xService##Response_EncodeableType, \
	  (OpcUa_PfnBeginInvokeService*)OpcUa_Server_Begin##xService, \
	  (OpcUa_PfnInvokeService*)Server_##xService \
};

#define DECLARE_SUPPORTED_ASYNCHRONOUS_SERVICE(xService) \
OpcUa_ServiceType g_##xService##Service  = \
{ \
	  OpcUaId_##xService##Request, \
	   &OpcUa_##xService##Response_EncodeableType, \
	  (OpcUa_PfnBeginInvokeService*)Server_Begin##xService, \
	  OpcUa_Null \
};
#endif

// Declarations for all services supported by the Server.
DECLARE_SUPPORTED_SYNCHRONOUS_SERVICE(FindServers);
DECLARE_SUPPORTED_SYNCHRONOUS_SERVICE(GetEndpoints);
DECLARE_SUPPORTED_SYNCHRONOUS_SERVICE(CreateSession);
DECLARE_SUPPORTED_SYNCHRONOUS_SERVICE(ActivateSession);
DECLARE_SUPPORTED_SYNCHRONOUS_SERVICE(CloseSession);
DECLARE_SUPPORTED_SYNCHRONOUS_SERVICE(Cancel);
DECLARE_SUPPORTED_ASYNCHRONOUS_SERVICE(Read);
DECLARE_SUPPORTED_SYNCHRONOUS_SERVICE(Write);
// View Services
DECLARE_SUPPORTED_SYNCHRONOUS_SERVICE(Browse);
DECLARE_SUPPORTED_SYNCHRONOUS_SERVICE(BrowseNext);
DECLARE_SUPPORTED_SYNCHRONOUS_SERVICE(TranslateBrowsePathsToNodeIds);
DECLARE_SUPPORTED_SYNCHRONOUS_SERVICE(RegisterNodes);
DECLARE_SUPPORTED_SYNCHRONOUS_SERVICE(UnregisterNodes);
// Subscription
DECLARE_SUPPORTED_SYNCHRONOUS_SERVICE(CreateSubscription);
DECLARE_SUPPORTED_SYNCHRONOUS_SERVICE(ModifySubscription);
DECLARE_SUPPORTED_SYNCHRONOUS_SERVICE(DeleteSubscriptions);
DECLARE_SUPPORTED_SYNCHRONOUS_SERVICE(TransferSubscriptions);
DECLARE_SUPPORTED_SYNCHRONOUS_SERVICE(SetPublishingMode);
DECLARE_SUPPORTED_ASYNCHRONOUS_SERVICE(Publish);
DECLARE_SUPPORTED_SYNCHRONOUS_SERVICE(Republish);
// monitoring item
DECLARE_SUPPORTED_SYNCHRONOUS_SERVICE(CreateMonitoredItems);
DECLARE_SUPPORTED_SYNCHRONOUS_SERVICE(ModifyMonitoredItems);
DECLARE_SUPPORTED_SYNCHRONOUS_SERVICE(DeleteMonitoredItems);
DECLARE_SUPPORTED_SYNCHRONOUS_SERVICE(SetMonitoringMode);
DECLARE_SUPPORTED_SYNCHRONOUS_SERVICE(SetTriggering);
// Method call
DECLARE_SUPPORTED_ASYNCHRONOUS_SERVICE(Call);
// HistoryRead
DECLARE_SUPPORTED_ASYNCHRONOUS_SERVICE(HistoryRead);
//QueryFirst
DECLARE_SUPPORTED_ASYNCHRONOUS_SERVICE(QueryFirst);
// QueryNext
DECLARE_SUPPORTED_ASYNCHRONOUS_SERVICE(QueryNext);

// A null terminated table of services that is used by the stack to dispatch incoming requests.
OpcUa_ServiceType* g_SupportedServices[] = 
{ 
	&g_FindServersService,
	&g_GetEndpointsService,
	&g_CreateSessionService,
	&g_ActivateSessionService,
	&g_CloseSessionService,
	&g_CancelService,
	&g_ReadService,
	&g_BrowseService,
	&g_BrowseNextService,
	&g_CreateSubscriptionService,
	&g_ModifySubscriptionService,
	&g_DeleteSubscriptionsService,
	&g_TransferSubscriptionsService,
	&g_SetPublishingModeService,
	&g_PublishService,
	&g_RepublishService,
	&g_CreateMonitoredItemsService,
	&g_ModifyMonitoredItemsService,
	&g_DeleteMonitoredItemsService,
	&g_SetMonitoringModeService,
	&g_SetTriggeringService,
	&g_TranslateBrowsePathsToNodeIdsService,
	&g_WriteService,
	&g_CallService,
	&g_HistoryReadService,
	&g_RegisterNodesService,
	&g_UnregisterNodesService,
	&g_QueryFirstService,
	&g_QueryNextService,
	OpcUa_Null
};
OpcUa_StatusCode Server_FindServers(
	OpcUa_Endpoint                 a_hEndpoint, // in
	OpcUa_Handle                   a_hContext, // in
	const OpcUa_RequestHeader*     a_pRequestHeader, // in
	const OpcUa_String*            a_pEndpointUrl, // in
	OpcUa_Int32                    a_nNoOfLocaleIds, // in
	const OpcUa_String*            a_pLocaleIds, // in
	OpcUa_Int32                    a_nNoOfServerUris, // in
	const OpcUa_String*            a_pServerUris, // in
	OpcUa_ResponseHeader*          a_pResponseHeader,
	OpcUa_Int32*                   a_pNoOfServers,
	OpcUa_ApplicationDescription** a_pServers)
{
	OpcUa_ReferenceParameter(a_pEndpointUrl);
	OpcUa_StatusCode uStatus=OpcUa_Good;
	CServerApplication* pServerApplication = 0;
	OpcUa_UInt32 uSecureChannelId = 0;
	//
	uStatus = OpcUa_Endpoint_GetCallbackData(a_hEndpoint, (OpcUa_Void**)&pServerApplication);
	if (uStatus==OpcUa_Good)
	{
		// Get the secure channel being used.
		uStatus = OpcUa_Endpoint_GetMessageSecureChannelId(a_hEndpoint, a_hContext, &uSecureChannelId);
		if (uStatus==OpcUa_Good)
		{
			(*a_pNoOfServers)=0; 

			CApplicationDescription* pAppDescription=pServerApplication->GetApplicationDescription();
			// filter sent by the client
			// Filtre sur ServerUris
			OpcUa_Boolean bFound=OpcUa_True;
			if (a_nNoOfServerUris>0)
			{
				bFound=OpcUa_False;
				for (OpcUa_Int32 ii=0;ii<a_nNoOfServerUris;ii++)
				{
					OpcUa_String* pString=pAppDescription->GetApplicationUri();
					if (OpcUa_String_StrnCmp(&a_pServerUris[ii],pString,OpcUa_String_StrLen(pString),OpcUa_False)==0)
						bFound=OpcUa_True;
				}
				if (bFound)
				{
					OpcUa_ApplicationDescription* pApplicationDescription = pAppDescription->GetInternalApplicationDescription();
					if (pApplicationDescription)
					{
						(*a_pServers) = Utils::Copy(pApplicationDescription);
						if ((*a_pServers))
							(*a_pNoOfServers)++;
					}
				}
			}
			// Filtre sur LocaleIds
			if (a_nNoOfLocaleIds>0)
			{
				bFound=OpcUa_False;
				for (OpcUa_Int32 ii=0;ii<a_nNoOfLocaleIds;ii++)
				{
					OpcUa_String aString=pAppDescription->GetApplicationName().Locale;
					if (OpcUa_String_StrnCmp(&a_pLocaleIds[ii],&aString,OpcUa_String_StrLen(&aString),OpcUa_False)==0)
						bFound=OpcUa_True;
				}
				OpcUa_ApplicationDescription* pApplicationDescription = pAppDescription->GetInternalApplicationDescription();
				if (pApplicationDescription)
				{
					(*a_pServers) = Utils::Copy(pApplicationDescription);
					if ((*a_pServers))
						(*a_pNoOfServers)++;
				}
			}

			//si aucun AppDescription ne correspond aux paramètres on retourne celle par défaut
			if ( (*a_pNoOfServers==0) && (a_nNoOfServerUris==0))
			{
				OpcUa_ApplicationDescription* pApplicationDescription = pAppDescription->GetInternalApplicationDescription();
				if (pApplicationDescription)
				{
					(*a_pServers) = Utils::Copy(pApplicationDescription);
					if ((*a_pServers))
						(*a_pNoOfServers)++;
				}
			}
		}
	}
	// Update the response header.
	a_pResponseHeader->Timestamp     = OpcUa_DateTime_UtcNow();
	a_pResponseHeader->RequestHandle = a_pRequestHeader->RequestHandle;
	a_pResponseHeader->ServiceResult = uStatus;

	
	return uStatus;
}
// Called when a GetEndpoints request arrives from a Client.

// Function name   : Server_GetEndpoints
// Description     : 
// Return type     : OpcUa_StatusCode 
// Argument        : pcUa_Endpoint              a_hEndpoint
// Argument        : OpcUa_Handle                a_hContext
// Argument        : const OpcUa_RequestHeader*  a_pRequestHeader
// Argument        : const OpcUa_String*         a_pEndpointUrl
// Argument        : OpcUa_Int32                 a_nNoOfLocaleIds
// Argument        : const OpcUa_String*         a_pLocaleIds
// Argument        : OpcUa_Int32                 a_nNoOfProfileUris
// Argument        : const OpcUa_String*         a_pProfileUris
// Argument        : OpcUa_ResponseHeader*       a_pResponseHeader
// Argument        : OpcUa_Int32*                a_pNoOfEndpoints
// Argument        : OpcUa_EndpointDescription** a_pEndpoints

OpcUa_StatusCode Server_GetEndpoints(
	OpcUa_Endpoint              a_hEndpoint, // in
	OpcUa_Handle                a_hContext, // in
	const OpcUa_RequestHeader*  a_pRequestHeader, // in
	const OpcUa_String*         a_pEndpointUrl, // in
	OpcUa_Int32                 a_nNoOfLocaleIds, // in
	const OpcUa_String*         a_pLocaleIds, // in 
	OpcUa_Int32                 a_nNoOfProfileUris, // in
	const OpcUa_String*         a_pProfileUris, // in
	OpcUa_ResponseHeader*       a_pResponseHeader, // out
	OpcUa_Int32*                a_pNoOfEndpoints, // out
	OpcUa_EndpointDescription** a_pEndpoints) // out
{
	OpcUa_UInt32 uSecureChannelId = 0;
	CServerApplication* pServerApplication = 0;

	OpcUa_StatusCode uStatus=OpcUa_Good;

	if (a_hEndpoint)
	{
		if (a_hContext)
		{
			if (a_pRequestHeader)
			{
				if (a_pEndpointUrl)
				{
					//if (a_nNoOfLocaleIds>0)
					//	OpcUa_ReturnErrorIfArgumentNull(a_pLocaleIds);
					//OpcUa_ReferenceParameter(a_nNoOfProfileUris);
					//OpcUa_ReturnErrorIfArgumentNull(a_pProfileUris);
					if (a_pResponseHeader)
					{
						if (a_pNoOfEndpoints)
						{
							if (a_pEndpoints)
							{
								// Get the server application object.
								uStatus = OpcUa_Endpoint_GetCallbackData(a_hEndpoint, (OpcUa_Void**)&pServerApplication);
								if (uStatus==OpcUa_Good)
								{
									// Get the secure channel being used.
									uStatus = OpcUa_Endpoint_GetMessageSecureChannelId(a_hEndpoint, a_hContext, &uSecureChannelId);
									if (uStatus==OpcUa_Good)
									{
										// Process the request.
										uStatus=pServerApplication->GetEndpoints(
											uSecureChannelId,
											a_pRequestHeader,
											a_pEndpointUrl,
											a_nNoOfLocaleIds,
											a_pLocaleIds,
											a_nNoOfProfileUris,
											a_pProfileUris,
											a_pResponseHeader,
											a_pNoOfEndpoints,
											a_pEndpoints);
										if (uStatus!=OpcUa_Good)
											OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Server_GetEndpoints failed 0x%05x\n",uStatus);
										// Update the response header.
										a_pResponseHeader->Timestamp     = OpcUa_DateTime_UtcNow();
										a_pResponseHeader->RequestHandle = a_pRequestHeader->RequestHandle;
										a_pResponseHeader->ServiceResult = OpcUa_Good;
									}
									else
										OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"OpcUa_Endpoint_GetMessageSecureChannelId failed 0x%05x\n",uStatus);
								}
								else
									OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"OpcUa_Endpoint_GetCallbackData failed 0x%05x\n",uStatus);
							}
							else
								uStatus=OpcUa_BadInvalidArgument;
						}
						else
							uStatus=OpcUa_BadInvalidArgument;
					}
					else
						uStatus=OpcUa_BadInvalidArgument;
				}
				else
					uStatus=OpcUa_BadInvalidArgument;
			}
			else
				uStatus=OpcUa_BadInvalidArgument;
		}
		else
			uStatus=OpcUa_BadInvalidArgument;
	}
	else
		uStatus=OpcUa_BadInvalidArgument;
	return uStatus;
}

// Called when a CreateSession request arrives from a Client.

// Function name   : Server_CreateSession
// Description     : 
// Return type     : OpcUa_StatusCode 
// Argument        : pcUa_Endpoint                      a_hEndpoint
// Argument        : OpcUa_Handle                        a_hContext
// Argument        : const OpcUa_RequestHeader*          a_pRequestHeader
// Argument        : const OpcUa_ApplicationDescription* a_pClientDescription
// Argument        : const OpcUa_String*                 a_pServerUri
// Argument        : const OpcUa_String*                 a_pEndpointUrl
// Argument        : const OpcUa_String*                 a_pSessionName
// Argument        : const OpcUa_ByteString*             a_pClientNonce
// Argument        : const OpcUa_ByteString*             a_pClientCertificate
// Argument        : OpcUa_Double                        a_nRequestedSessionTimeout
// Argument        : OpcUa_UInt32                        a_nMaxResponseMessageSize
// Argument        : OpcUa_ResponseHeader*               a_pResponseHeader
// Argument        : OpcUa_NodeId*                       a_pSessionId
// Argument        : OpcUa_NodeId*                       a_pAuthenticationToken
// Argument        : OpcUa_Double*                       a_pRevisedSessionTimeout
// Argument        : OpcUa_ByteString*                   a_pServerNonce
// Argument        : OpcUa_ByteString*                   a_pServerCertificate
// Argument        : OpcUa_Int32*                        a_pNoOfServerEndpoints
// Argument        : OpcUa_EndpointDescription**         a_pServerEndpoints
// Argument        : OpcUa_Int32*                        a_pNoOfServerSoftwareCertificates
// Argument        : OpcUa_SignedSoftwareCertificate**   a_pServerSoftwareCertificates
// Argument        : OpcUa_SignatureData*                a_pServerSignature
// Argument        : OpcUa_UInt32*                       a_pMaxRequestMessageSize
	
OpcUa_StatusCode Server_CreateSession(
	OpcUa_Endpoint                      a_hEndpoint, // in
	OpcUa_Handle                        a_hContext,// in
	const OpcUa_RequestHeader*          a_pRequestHeader,// in
	const OpcUa_ApplicationDescription* a_pClientDescription,// in
	const OpcUa_String*                 a_pServerUri,// in
	const OpcUa_String*                 a_pEndpointUrl,// in
	const OpcUa_String*                 a_pSessionName,// in
	const OpcUa_ByteString*             a_pClientNonce,// in
	const OpcUa_ByteString*             a_pClientCertificate,// in
	OpcUa_Double                        a_nRequestedSessionTimeout,// in
	OpcUa_UInt32                        a_nMaxResponseMessageSize,// in
	OpcUa_ResponseHeader*               a_pResponseHeader,// out
	OpcUa_NodeId*                       a_pSessionId,// out
	OpcUa_NodeId*                       a_pAuthenticationToken,// out
	OpcUa_Double*                       a_pRevisedSessionTimeout,// out
	OpcUa_ByteString*                   a_pServerNonce,// out
	OpcUa_ByteString*                   a_pServerCertificate,// out
	OpcUa_Int32*                        a_pNoOfServerEndpoints,// out
	OpcUa_EndpointDescription**         a_pServerEndpoints,// out
	OpcUa_Int32*                        a_pNoOfServerSoftwareCertificates,// out
	OpcUa_SignedSoftwareCertificate**   a_pServerSoftwareCertificates,// out
	OpcUa_SignatureData*                a_pServerSignature,// out
	OpcUa_UInt32*                       a_pMaxRequestMessageSize)// out
{
	OpcUa_UInt32 uSecureChannelId = 0;
	CServerApplication* pServerApplication = OpcUa_Null;
	CUAInformationModel* pInformationModel = CServerApplication::m_pTheAddressSpace;
	OpcUa_StatusCode uStatus=OpcUa_Good;
	if (!g_bRun)
		uStatus = OpcUa_BadServerHalted;
	else
	{
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_INFO, "Server_CreateSession receive from the client\n");
		// Check input parameters and set uStatus
		{
			if (!a_hEndpoint)
				uStatus = OpcUa_BadInvalidArgument;
			if (!a_hContext)
				uStatus = OpcUa_BadInvalidArgument;
			if (!a_pRequestHeader)
				uStatus = OpcUa_BadInvalidArgument;
			if (!a_pClientDescription)
				uStatus = OpcUa_BadInvalidArgument;
			if (!a_pServerUri)
				uStatus = OpcUa_BadInvalidArgument;
			if (!a_pEndpointUrl)
				uStatus = OpcUa_BadInvalidArgument;
			if (!a_pSessionName)
				uStatus = OpcUa_BadInvalidArgument;
			if (!a_pClientNonce)
				uStatus = OpcUa_BadInvalidArgument;
			if (!a_pClientCertificate)
				uStatus = OpcUa_BadInvalidArgument;
			if (a_nRequestedSessionTimeout == 0)
				*a_pRevisedSessionTimeout = 600000; // 10mn

			if (!a_pResponseHeader)
				uStatus = OpcUa_BadInvalidArgument;
			if (!a_pSessionId)
				uStatus = OpcUa_BadInvalidArgument;
			if (!a_pAuthenticationToken)
				uStatus = OpcUa_BadInvalidArgument;
			if (!a_pRevisedSessionTimeout)
				uStatus = OpcUa_BadInvalidArgument;
			if (!a_pServerNonce)
				uStatus = OpcUa_BadInvalidArgument;
			if (!a_pServerCertificate)
				uStatus = OpcUa_BadInvalidArgument;
			if (!a_pNoOfServerEndpoints)
				uStatus = OpcUa_BadInvalidArgument;
			if (!a_pServerEndpoints)
				uStatus = OpcUa_BadInvalidArgument;
			if (!a_pNoOfServerSoftwareCertificates)
				uStatus = OpcUa_BadInvalidArgument;
			if (!a_pServerSoftwareCertificates)
				uStatus = OpcUa_BadInvalidArgument;
			if (!a_pServerSignature)
				uStatus = OpcUa_BadInvalidArgument;
			if (!a_pMaxRequestMessageSize)
				uStatus = OpcUa_BadInvalidArgument;
			else
				*a_pMaxRequestMessageSize = OpcUa_Null;
		}
		if (uStatus == OpcUa_Good)
		{
			// Get the server application object.
			uStatus = OpcUa_Endpoint_GetCallbackData(a_hEndpoint, (OpcUa_Void**)&pServerApplication);
			if (uStatus == OpcUa_Good)
			{
				// Get the secure channel being used.
				uStatus = OpcUa_Endpoint_GetMessageSecureChannelId(a_hEndpoint, a_hContext, &uSecureChannelId);
				if (uStatus == OpcUa_Good)
				{
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_WARNING, "The currentSecureChannelId for this session will be %u\n", uSecureChannelId);
					CSecureChannel* pSecureChannel = pServerApplication->FindSecureChannel(uSecureChannelId);
					if (pSecureChannel)
					{
						// Get security Mode.
						OpcUa_MessageSecurityMode eSecurityMode = (OpcUa_MessageSecurityMode)pSecureChannel->GetSecurityMode();
						if (eSecurityMode != OpcUa_MessageSecurityMode_None)
						{
							// Check ClientNonce and Certificate content (AppUri)
							// Check that client nonce have the minumum size. On accepte les ClientNonce NULL (Length<=0)							
							if (a_pClientNonce->Length < 32)
								uStatus = OpcUa_BadNonceInvalid;
							else
							{
								// On va verifier que le certificat du serveur est bien formé. 
								// On ne fera cette verification que si le certificat est fournit par le client
								// En securité none cette verification n'est pas réalisée
								if (a_pClientCertificate->Data)
								{
									// extract the information from the certificate.
									OpcUa_CharA* sThumbprint = OpcUa_Null;
									OpcUa_CharA* lApplicationUri = OpcUa_Null;
									OpcUa_CharA* psCommonName = OpcUa_Null;
									uStatus = OpcUa_Certificate_GetInfo(
										(OpcUa_ByteString*)a_pClientCertificate,
										OpcUa_Null,
										OpcUa_Null,
										&psCommonName,
										&sThumbprint,
										&lApplicationUri,
										OpcUa_Null,
										OpcUa_Null);

									if (uStatus != OpcUa_Good)
									{
										OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Could not extract information from application certificate.");
										uStatus = OpcUa_BadCertificateUriInvalid;
									}
									else
									{
										// Maintenant on verifie que lApplicationUri et a_pClientDescription->ApplicationUri sont identique
										OpcUa_String aAppUri;
										OpcUa_String_Initialize(&aAppUri);
										OpcUa_String_AttachCopy(&aAppUri, lApplicationUri);
										if (OpcUa_String_StrnCmp(&(a_pClientDescription->ApplicationUri), &aAppUri, OpcUa_String_StrLen(&aAppUri), OpcUa_False) != 0)
											uStatus = OpcUa_BadCertificateUriInvalid; // Le certificat du client existe et il est mal formé
										OpcUa_String_Clear(&aAppUri);
										if (lApplicationUri)
										{
											OpcUa_Free(lApplicationUri);
											lApplicationUri = OpcUa_Null;
										}
									}
									if (psCommonName)
									{
										OpcUa_Free(psCommonName);
										psCommonName = OpcUa_Null;
									}
									if (sThumbprint)
									{
										OpcUa_Free(sThumbprint);
										sThumbprint = OpcUa_Null;
									}
								}
								else
									uStatus = OpcUa_BadCertificateUriInvalid;
							}
						}
						if (uStatus == OpcUa_Good)
						{
							// Get security policy.
							OpcUa_String* pSecurityPolicy = Utils::Copy(pSecureChannel->GetSecurityPolicy());
							if (pSecurityPolicy)
							{
								// Process the request.
								uStatus = pServerApplication->CreateSession(
									uSecureChannelId,
									eSecurityMode,
									*pSecurityPolicy,
									a_pRequestHeader,
									a_pClientDescription,
									a_pServerUri,
									a_pEndpointUrl,
									a_pSessionName,
									a_pClientNonce,
									a_pClientCertificate,
									a_nRequestedSessionTimeout,
									a_nMaxResponseMessageSize,
									a_pResponseHeader,
									a_pSessionId,
									a_pAuthenticationToken,
									a_pRevisedSessionTimeout,
									a_pServerNonce,
									a_pServerCertificate,
									a_pNoOfServerEndpoints,
									a_pServerEndpoints,
									a_pServerSignature,
									a_pMaxRequestMessageSize);
								if (uStatus != OpcUa_Good)
									OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "pServer->CreateSession failed 0x%05x\n", uStatus);
								else
								{
									*a_pNoOfServerSoftwareCertificates = 1;
									OpcUa_ServerDiagnosticsSummaryDataType* pServerDiagnosticsSummaryDataType= pInformationModel->GetServerDiagnosticsSummaryDataType();
									if (pServerDiagnosticsSummaryDataType)
									{
										pServerDiagnosticsSummaryDataType->CumulatedSessionCount++;
										pServerDiagnosticsSummaryDataType->CurrentSessionCount++;
										pInformationModel->UpdateServerDiagnosticsSummary();
									}
								}
								OpcUa_String_Clear(pSecurityPolicy);
								OpcUa_Free(pSecurityPolicy);
							}
							else
								uStatus = OpcUa_BadOutOfMemory;
						}
					}
					else
						OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "FindSecureChannel failed for channel %u\n", uSecureChannelId);
				}
				else
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "OpcUa_Endpoint_GetMessageSecureChannelId failed 0x%05x\n", uStatus);

			}
			else
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "OpcUa_Endpoint_GetCallbackData failed uStatus=0x%05x\n", uStatus);
		}
		else
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "InvalidParameter receive\n");

		// Update the response header.
		if (a_pResponseHeader)
		{
			a_pResponseHeader->Timestamp = OpcUa_DateTime_UtcNow();
			if (a_pRequestHeader)
				a_pResponseHeader->RequestHandle = a_pRequestHeader->RequestHandle;
			else
			{
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "RequestHeader received from the client is invalid\n");
				a_pResponseHeader->RequestHandle = OpcUa_Null;
				uStatus = OpcUa_BadInternalError;
			}
			a_pResponseHeader->ServiceResult = uStatus;
		}
		else
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "ResponseHeader received from the client is invalid\n");
		//OpcUa_EndpointContext* pContext=(OpcUa_EndpointContext*)a_hContext;
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_WARNING, "Server_CreateSession done uStatus=0x%x\n", uStatus);
	}
	return uStatus;
}

// Called when a ActivateSession request arrives from a Client.

// Function name   : Server_ActivateSession
// Description     : 
// Return type     : OpcUa_StatusCode 
// Argument        : pcUa_Endpoint                         a_hEndpoint
// Argument        : OpcUa_Handle                           a_hContext
// Argument        : const OpcUa_RequestHeader*             a_pRequestHeader
// Argument        : const OpcUa_SignatureData*             a_pClientSignature
// Argument        : OpcUa_Int32                            a_nNoOfClientSoftwareCertificates
// Argument        : const OpcUa_SignedSoftwareCertificate* a_pClientSoftwareCertificates
// Argument        : OpcUa_Int32                            a_nNoOfLocaleIds
// Argument        : const OpcUa_String*                    a_pLocaleIds
// Argument        : const OpcUa_ExtensionObject*           a_pUserIdentityToken
// Argument        : const OpcUa_SignatureData*             a_pUserTokenSignature
// Argument        : OpcUa_ResponseHeader*                  a_pResponseHeader
// Argument        : OpcUa_ByteString*                      a_pServerNonce
// Argument        : OpcUa_Int32*                           a_pNoOfResults
// Argument        : OpcUa_StatusCode**                     a_pResults
// Argument        : OpcUa_Int32*                           a_pNoOfDiagnosticInfos
// Argument        : OpcUa_DiagnosticInfo**                 a_pDiagnosticInfos

OpcUa_StatusCode Server_ActivateSession(
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
	OpcUa_DiagnosticInfo**                 a_pDiagnosticInfos)
{
	OpcUa_ReferenceParameter(a_pLocaleIds);
	OpcUa_ReferenceParameter(a_nNoOfLocaleIds);
	OpcUa_UInt32 uSecureChannelId = 0;
	CServerApplication* pServer = 0;
	//OpcUa_Endpoint_SecurityPolicyConfiguration tSecurityPolicy;
	OpcUa_StatusCode uStatus=OpcUa_Good;

	OpcUa_ReturnErrorIfArgumentNull(a_hEndpoint);
	OpcUa_ReturnErrorIfArgumentNull(a_hContext);
	OpcUa_ReturnErrorIfArgumentNull(a_pRequestHeader);
	OpcUa_ReturnErrorIfArgumentNull(a_pClientSignature);
	OpcUa_ReferenceParameter(a_nNoOfClientSoftwareCertificates);
	//OpcUa_ReturnErrorIfArgumentNull(a_pClientSoftwareCertificates); // Attention supprimé pour faire plaisir au CTT
	//OpcUa_ReferenceParameter(a_nNoOfLocaleIds);
	//OpcUa_ReturnErrorIfArgumentNull(a_pLocaleIds);
	OpcUa_ReturnErrorIfArgumentNull(a_pUserIdentityToken);
	OpcUa_ReturnErrorIfArgumentNull(a_pUserTokenSignature);
	OpcUa_ReturnErrorIfArgumentNull(a_pResponseHeader);
	OpcUa_ReturnErrorIfArgumentNull(a_pServerNonce);
	OpcUa_ReturnErrorIfArgumentNull(a_pNoOfResults);
	OpcUa_ReturnErrorIfArgumentNull(a_pResults);
	OpcUa_ReturnErrorIfArgumentNull(a_pNoOfDiagnosticInfos);
	OpcUa_ReturnErrorIfArgumentNull(a_pDiagnosticInfos);

	// Get the server application object.
	uStatus = OpcUa_Endpoint_GetCallbackData(a_hEndpoint, (OpcUa_Void**)&pServer);
	if (uStatus==OpcUa_Good)
	{
		// Get the secure channel being used.
		uStatus = OpcUa_Endpoint_GetMessageSecureChannelId(a_hEndpoint, a_hContext, &uSecureChannelId);
		if (uStatus==OpcUa_Good)
		{
			CSecureChannel* pSecureChannel=pServer->FindSecureChannel(uSecureChannelId);
			if (pSecureChannel)
			{
				OpcUa_MessageSecurityMode eSecurityMode = (OpcUa_MessageSecurityMode)pSecureChannel->GetSecurityMode();
				OpcUa_String* pLocalSecurityPolicy=pSecureChannel->GetSecurityPolicy();
				if (pLocalSecurityPolicy)
				{
					OpcUa_String aSecurityPolicy; //= Utils::Copy(pLocalSecurityPolicy);
					OpcUa_String_Initialize(&aSecurityPolicy);
					if (pLocalSecurityPolicy)
					{
						OpcUa_String_CopyTo(pLocalSecurityPolicy, &aSecurityPolicy);
						uStatus = pServer->ActivateSession(
							uSecureChannelId,
							eSecurityMode,
							&aSecurityPolicy,
							a_nNoOfClientSoftwareCertificates,
							a_pClientSoftwareCertificates,
							a_pUserIdentityToken,
							a_pRequestHeader,
							a_pClientSignature,
							a_pResponseHeader,
							a_pServerNonce);
					}
					else
						uStatus = OpcUa_BadInvalidArgument;
					OpcUa_String_Clear(&aSecurityPolicy);
				}
				else
					uStatus=OpcUa_BadSecurityPolicyRejected;
			}
		}
		//// Get the secure channel being used.
		//uStatus = OpcUa_Endpoint_GetMessageSecureChannelId(a_hEndpoint, a_hContext, &uSecureChannelId);
		//if (uStatus==OpcUa_Good)
		//{
		//	
		//	// Get security policy.
		//	uStatus = OpcUa_Endpoint_GetMessageSecureChannelSecurityPolicy(a_hEndpoint, a_hContext, &tSecurityPolicy);
		//	if (uStatus==OpcUa_Good)
		//	{
		//		// The stack should return teh same bitmask that was passed to OpcUa_Endpoint_Open. This is a work around until the stack is fixed.
		//		OpcUa_MessageSecurityMode eSecurityMode = (OpcUa_MessageSecurityMode)tSecurityPolicy.uMessageSecurityModes;

		//		uStatus=pServer->ActivateSession(
		//			uSecureChannelId,
		//			eSecurityMode,
		//			&(tSecurityPolicy.sSecurityPolicy),
		//			a_nNoOfClientSoftwareCertificates,
		//			a_pClientSoftwareCertificates,
		//			a_pRequestHeader,
		//			a_pClientSignature,
		//			a_pResponseHeader,
		//			a_pServerNonce);
		//	}
		//}
	}
	return uStatus;
}

// Called when a CloseSession request arrives from a Client.

// Function name   : Server_CloseSession
// Description     : 
// Return type     : OpcUa_StatusCode 
// Argument        : pcUa_Endpoint             a_hEndpoint
// Argument        : OpcUa_Handle               a_hContext
// Argument        : const OpcUa_RequestHeader* a_pRequestHeader
// Argument        : OpcUa_Boolean              a_bDeleteSubscriptions
// Argument        : OpcUa_ResponseHeader*      a_pResponseHeader

OpcUa_StatusCode Server_CloseSession(
	OpcUa_Endpoint             a_hEndpoint,
	OpcUa_Handle               a_hContext,
	const OpcUa_RequestHeader* a_pRequestHeader,
	OpcUa_Boolean              a_bDeleteSubscriptions,
	OpcUa_ResponseHeader*      a_pResponseHeader)
{
	OpcUa_UInt32 uSecureChannelId = 0;
	CServerApplication* pServer = 0;
	OpcUa_StatusCode uStatus=OpcUa_Good;
	
	OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_INFO, "Server_CloseSession receive from the client\n");
	CUAInformationModel* pInformationModel = CServerApplication::m_pTheAddressSpace;
	if (!a_hEndpoint)
		uStatus=OpcUa_BadInvalidArgument;
	else
	{
		if (!a_hContext)
			uStatus=OpcUa_BadInvalidArgument;
		else
		{
			if (!a_pRequestHeader)
				uStatus=OpcUa_BadInvalidArgument;
			else
			{
				// Get the server application object.
				uStatus = OpcUa_Endpoint_GetCallbackData(a_hEndpoint, (OpcUa_Void**)&pServer);
				if (uStatus==OpcUa_Good)
				{
					// Get the secure channel being used.
					uStatus = OpcUa_Endpoint_GetMessageSecureChannelId(a_hEndpoint, a_hContext, &uSecureChannelId);
					if (uStatus==OpcUa_Good)
					{
						OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_INFO, "Server_CloseSession receive from the client\n");
						uStatus=pServer->CloseSession(	uSecureChannelId,
														a_pRequestHeader,
														a_bDeleteSubscriptions,
														a_pResponseHeader);
						if (uStatus!=OpcUa_Good)
							OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Critical error>CloseSession failed 0x%05x\n",uStatus);
						else
						{
							OpcUa_ServerDiagnosticsSummaryDataType* pServerDiagnosticsSummaryDataType = pInformationModel->GetServerDiagnosticsSummaryDataType();
							if (pServerDiagnosticsSummaryDataType)
							{
								pServerDiagnosticsSummaryDataType->CurrentSessionCount--;
								pInformationModel->UpdateServerDiagnosticsSummary();
							}
						}
					}
				}
			}
		}
	}
	return uStatus;
}
OpcUa_StatusCode Server_Cancel(
	OpcUa_Endpoint             a_hEndpoint,
	OpcUa_Handle               a_hContext,
	const OpcUa_RequestHeader* a_pRequestHeader,
	OpcUa_UInt32               a_nRequestHandle,
	OpcUa_ResponseHeader*      a_pResponseHeader,
	OpcUa_UInt32*              a_pCancelCount)
{
	OpcUa_UInt32 uSecureChannelId = 0;
	CServerApplication* pServerApplication = 0;
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_StatusCode uServiceStatus=OpcUa_Good;
	CSessionServer* pSession = OpcUa_Null;

	(void)a_pCancelCount;
	(void)a_nRequestHandle;
	if (!a_hEndpoint)
		uStatus=OpcUa_BadInvalidArgument;
	else
	{
		if (!a_hContext)
			uStatus=OpcUa_BadInvalidArgument;
		else
		{
			if (!a_pRequestHeader)
				uStatus=OpcUa_BadInvalidArgument;
			else
			{
				// Get the server application object.
				uStatus = OpcUa_Endpoint_GetCallbackData(a_hEndpoint, (OpcUa_Void**)&pServerApplication);
				if (uStatus==OpcUa_Good)
				{
					// Get the secure channel being used.
					uStatus = OpcUa_Endpoint_GetMessageSecureChannelId(a_hEndpoint, a_hContext, &uSecureChannelId);
					if (uStatus==OpcUa_Good)
					{
							// find the session.
						uStatus=pServerApplication->FindSession(uSecureChannelId,&a_pRequestHeader->AuthenticationToken,&pSession);
						if (uStatus==OpcUa_Good)
						{
							uServiceStatus=OpcUa_BadRequestCancelledByClient;
						}
					}
				}
			}
		}
	}
	// Update the response header.
	if (a_pResponseHeader)
	{
		a_pResponseHeader->Timestamp = OpcUa_DateTime_UtcNow();
		if (a_pRequestHeader)
			a_pResponseHeader->RequestHandle = a_pRequestHeader->RequestHandle;
		a_pResponseHeader->ServiceResult = OpcUa_Good;
	}
	return uStatus;
}

// Function name   : Server_Browse
// Description     : 
// Return type     : OpcUa_StatusCode 
// Argument        : pcUa_Endpoint                 a_hEndpoint
// Argument        : OpcUa_Handle                   a_hContext
// Argument        : const OpcUa_RequestHeader*     a_pRequestHeader
// Argument        : const OpcUa_ViewDescription*   a_pView
// Argument        : OpcUa_UInt32                   a_nRequestedMaxReferencesPerNode
// Argument        : OpcUa_Int32                    a_nNoOfNodesToBrowse
// Argument        : const OpcUa_BrowseDescription* a_pNodesToBrowse
// Argument        : OpcUa_ResponseHeader*          a_pResponseHeader
// Argument        : OpcUa_Int32*                   a_pNoOfResults
// Argument        : OpcUa_BrowseResult**           a_pResults
// Argument        : OpcUa_Int32*                   a_pNoOfDiagnosticInfos
// Argument        : OpcUa_DiagnosticInfo**         a_pDiagnosticInfos

OpcUa_StatusCode Server_Browse(
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
	OpcUa_DiagnosticInfo**         a_pDiagnosticInfos)
{
	OpcUa_UInt32 uSecureChannelId = 0;
	CServerApplication* pServer = 0;
	OpcUa_StatusCode uStatus=OpcUa_Good;

	if (a_hEndpoint)
	{
		if (a_hContext)
		{
			if (a_pRequestHeader)
			{
					if (a_pView)
					{
						if (a_pNodesToBrowse>0)
						{
							if (a_pResponseHeader)
							{	
								// verification que l'AuthenticationToken ne soit pas null
								if ( (a_pRequestHeader->AuthenticationToken.IdentifierType==0) 
									&& (a_pRequestHeader->AuthenticationToken.Identifier.Numeric==0) 
									&& (a_pRequestHeader->AuthenticationToken.NamespaceIndex==0) )
								{
									uStatus=OpcUa_BadSessionIdInvalid;//OpcUa_BadSecurityChecksFailed;
									// Update the response header.
									a_pResponseHeader->ServiceResult=OpcUa_BadSessionIdInvalid;
									a_pResponseHeader->Timestamp     = OpcUa_DateTime_UtcNow();
									a_pResponseHeader->RequestHandle = a_pRequestHeader->RequestHandle;
								}
								else
								{						
									// Get the server application object.
									uStatus = OpcUa_Endpoint_GetCallbackData(a_hEndpoint, (OpcUa_Void**)&pServer);
									if (uStatus==OpcUa_Good)
									{							
										// Get the secure channel being used.
										uStatus = OpcUa_Endpoint_GetMessageSecureChannelId(a_hEndpoint, a_hContext, &uSecureChannelId);
										if (uStatus==OpcUa_Good)
										{			
											if (!pServer->FindSecureChannel(uSecureChannelId))
											{
												uStatus=OpcUa_BadSecureChannelIdInvalid;
												// Update the response header.
												a_pResponseHeader->Timestamp     = OpcUa_DateTime_UtcNow();
												a_pResponseHeader->RequestHandle = a_pRequestHeader->RequestHandle;
												a_pResponseHeader->ServiceResult = OpcUa_BadSecureChannelIdInvalid;
											}
											else
											{
												//OpcUa_BrowseResult**           pResults=OpcUa_Null;
												OpcUa_BrowseResult*           pResults=OpcUa_Null;
												//(*pResults)=(OpcUa_BrowseResult*)OpcUa_Alloc((a_nNoOfNodesToBrowse)*sizeof(OpcUa_BrowseResult*));
												uStatus=pServer->Browse(uSecureChannelId,
													a_pRequestHeader,
													a_pView,
													a_nRequestedMaxReferencesPerNode,
													a_nNoOfNodesToBrowse,
													a_pNodesToBrowse,
													a_pResponseHeader,
													a_pNoOfResults,
													&pResults,
													a_pNoOfDiagnosticInfos,
													a_pDiagnosticInfos);
												if (uStatus==OpcUa_Good)
												{
													int iSize=sizeof(OpcUa_BrowseResult);
													(*a_pResults)=(OpcUa_BrowseResult*)OpcUa_Alloc((*a_pNoOfResults)*iSize);
													for (int iii=0;iii<(*a_pNoOfResults);iii++)
													{
														memcpy(&((OpcUa_Byte*)*a_pResults)[iii*iSize],	/**/						
																	&pResults[iii],
																	iSize);
													}
													//OpcUa_BrowseResult_Clear(pResults);
													OpcUa_Free(pResults);
												}
											}
										}
										else
										{
											// Update the response header.
											a_pResponseHeader->Timestamp     = OpcUa_DateTime_UtcNow();
											a_pResponseHeader->RequestHandle = a_pRequestHeader->RequestHandle;
											a_pResponseHeader->ServiceResult = uStatus;
										}

									}
								}
							}
							else
								uStatus=OpcUa_BadInvalidArgument;
						}
						else
							uStatus=OpcUa_BadNothingToDo;
					}
					else
						uStatus=OpcUa_BadInvalidArgument;			
			}
			else
				uStatus=OpcUa_BadInvalidArgument;
		}
		else
			uStatus=OpcUa_BadInvalidArgument;
	}
	else
		uStatus=OpcUa_BadInvalidArgument;
	return uStatus;
}
OpcUa_StatusCode Server_BrowseNext(
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
	OpcUa_DiagnosticInfo**     a_pDiagnosticInfos)
{
	OpcUa_ReferenceParameter(a_pNoOfDiagnosticInfos);
	OpcUa_ReferenceParameter(a_pDiagnosticInfos);
	OpcUa_ReferenceParameter(a_pResults);
	OpcUa_ReferenceParameter(a_pNoOfResults);
	OpcUa_ReferenceParameter(a_nNoOfContinuationPoints);
	OpcUa_ReferenceParameter(a_bReleaseContinuationPoints);
	OpcUa_ReferenceParameter(a_pContinuationPoints);

	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_StatusCode uServiceResult=OpcUa_Good;
	CServerApplication* pServerApplication = 0;
	OpcUa_UInt32 uSecureChannelId = 0;
	//
	if (!Utils::IsNodeIdNull(a_pRequestHeader->AuthenticationToken))
	{
		uStatus = OpcUa_Endpoint_GetCallbackData(a_hEndpoint, (OpcUa_Void**)&pServerApplication);
		if (uStatus == OpcUa_Good)
		{
			// Get the secure channel being used.
			uStatus = OpcUa_Endpoint_GetMessageSecureChannelId(a_hEndpoint, a_hContext, &uSecureChannelId);
			if (uStatus == OpcUa_Good)
			{
				if (a_nNoOfContinuationPoints == 0)
					uStatus = OpcUa_BadNothingToDo;
				else
				{
					uServiceResult = pServerApplication->BrowseNext(uSecureChannelId,
						a_pRequestHeader,
						a_bReleaseContinuationPoints,
						a_nNoOfContinuationPoints,
						a_pContinuationPoints,
						a_pResponseHeader,
						a_pNoOfResults,
						a_pResults,
						a_pNoOfDiagnosticInfos,
						a_pDiagnosticInfos);
				}
			}
		}
	}
	else
	{
		uStatus = OpcUa_BadSessionIdInvalid;// OpcUa_BadSecurityChecksFailed;
		uServiceResult = OpcUa_BadSecurityChecksFailed;// OpcUa_BadSessionIdInvalid;
	}
	// Update the response header.
	a_pResponseHeader->Timestamp     = OpcUa_DateTime_UtcNow();
	a_pResponseHeader->RequestHandle = a_pRequestHeader->RequestHandle;
	a_pResponseHeader->ServiceResult = uServiceResult;
	return uStatus;
}
// Called when a  Asynchrone Publish request is received from the Client.
OpcUa_StatusCode Server_BeginPublish(
	OpcUa_Endpoint        a_hEndpoint,
	OpcUa_Handle          a_hContext,
	OpcUa_Void**          a_ppRequest,
	OpcUa_EncodeableType* a_pRequestType)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	CSessionServer* pSession = OpcUa_Null;
	if (!a_ppRequest)
		uStatus=OpcUa_BadInvalidArgument;
	else
	{
		if (!a_hEndpoint)
			uStatus=OpcUa_BadInvalidArgument;
		else
		{
			if (!a_hContext)
				uStatus=OpcUa_BadInvalidArgument;
			else
			{
				if (!(*a_ppRequest))
					uStatus=OpcUa_BadInvalidArgument;
				else
				{
					OpcUa_PublishRequest* pRequest = (OpcUa_PublishRequest*)*a_ppRequest;
					CServerApplication* pServerApplication = OpcUa_Null;
					
					if (!a_pRequestType)
						uStatus=OpcUa_BadInvalidArgument;
					if (uStatus==OpcUa_Good)
					{
						if (a_pRequestType->TypeId != OpcUaId_PublishRequest)
							uStatus= OpcUa_BadInvalidArgument;
						else
						{
							// Get the server application object.
							uStatus = OpcUa_Endpoint_GetCallbackData(a_hEndpoint, (OpcUa_Void**)&pServerApplication);
							if (uStatus==OpcUa_Good)
							{
								OpcUa_UInt32 uSecureChannelId;
								uStatus = OpcUa_Endpoint_GetMessageSecureChannelId(a_hEndpoint, a_hContext, &uSecureChannelId);
								if (uStatus==OpcUa_Good)
								{
									// find the session.
									OpcUa_NodeId AuthenticationToken=pRequest->RequestHeader.AuthenticationToken;
									uStatus=pServerApplication->FindSession(uSecureChannelId,&AuthenticationToken,&pSession);
									if (uStatus==OpcUa_Good)
									{
										// Signal that the session is alive
										OpcUa_Semaphore_Post(pSession->m_SessionTimeoutSem,1);
										// Process the publish. We will ack and queue it
										uStatus = pSession->ProcessPublishRequest(pRequest, a_hEndpoint, a_hContext, a_pRequestType);
										// Must set this to null to prevent the caller from deleting it.
										// otherwise it will be delete in the function OpcUa_Endpoint_BeginProcessRequest
										// by a call to OpcUa_EncodeableObject_Delete(pRequestType, &pRequest); line 1238
										if ((uStatus == OpcUa_Good) /*|| (uStatus == OpcUa_BadWaitingForResponse)*/)
											*a_ppRequest = OpcUa_Null;
										else
											OpcUa_Trace(OPCUA_TRACE_SERVER_INFO, "Server_BeginPublish>ProcessPublishRequest failed 0x%05x\n", uStatus);
									}
								}
							}
						}
					}
				}
				if ((uStatus != OpcUa_Good) /*&& (uStatus != OpcUa_BadWaitingForResponse) */)
				{
					// Send an error response.
					OpcUa_Endpoint_EndSendResponse(a_hEndpoint, &a_hContext, uStatus, OpcUa_Null, OpcUa_Null);
				}
			}
		}
	}
	return uStatus;
}

// Called when a QueryFirst is called by the client
OpcUa_StatusCode Server_BeginQueryFirst(
	OpcUa_Endpoint        a_hEndpoint,
	OpcUa_Handle          a_hContext,
	OpcUa_Void**          a_ppRequest,
	OpcUa_EncodeableType* a_pRequestType)
{
	CSessionServer* pSessionServer = 0;
	OpcUa_QueryFirstRequest* pQueryFirstRequest = OpcUa_Null;
	CServerApplication* pServerApplication = 0;

	OpcUa_StatusCode uStatus = OpcUa_Good;

	if (!a_hEndpoint)
		uStatus = OpcUa_BadInvalidArgument;
	if (!a_hContext)
		uStatus = OpcUa_BadInvalidArgument;
	if (!a_ppRequest)
		uStatus = OpcUa_BadInvalidArgument;
	if (*a_ppRequest == NULL)
		uStatus = OpcUa_BadInvalidArgument;
	if (!a_pRequestType)
		uStatus = OpcUa_BadInvalidArgument;
	else
	{
		if (a_pRequestType->TypeId != OpcUaId_QueryFirstRequest)
			uStatus = OpcUa_BadInvalidArgument;
	}
	if (uStatus == OpcUa_Good)
	{
		pQueryFirstRequest = (OpcUa_QueryFirstRequest*)*a_ppRequest;


		// Get the server application object.
		uStatus = OpcUa_Endpoint_GetCallbackData(a_hEndpoint, (OpcUa_Void**)&pServerApplication);
		if (uStatus == OpcUa_Good)
		{
			OpcUa_UInt32 uSecureChannelId;
			uStatus = OpcUa_Endpoint_GetMessageSecureChannelId(a_hEndpoint, a_hContext, &uSecureChannelId);
			if (uStatus == OpcUa_Good)
			{
				// find the session.
				OpcUa_NodeId AuthenticationToken = pQueryFirstRequest->RequestHeader.AuthenticationToken;
				uStatus = pServerApplication->FindSession(uSecureChannelId, &AuthenticationToken, &pSessionServer);
				if (uStatus == OpcUa_Good)
				{
					// Signal that the session is alive
					OpcUa_Semaphore_Post(pSessionServer->m_SessionTimeoutSem, 1);
					// Effacement des requetes de lecture déjà traitée
					if (pSessionServer->RemoveAllQueryFirstRequestDeleted() != OpcUa_Good)
						OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "RemoveAllQueryFirstRequest failed\n");
					// queue the request.
					CQueuedQueryFirstMessage* pQueuedQueryFirstMessage = new CQueuedQueryFirstMessage(pQueryFirstRequest, a_hEndpoint, a_hContext, a_pRequestType);
					// Add to queue
					pSessionServer->QueueQueryFirstMessage(pQueuedQueryFirstMessage);
					// Must set this to null to prevent the caller from deleting it.
					*a_ppRequest = OpcUa_Null;
				}
			}
			else
				// Send an error response.	
				OpcUa_Endpoint_EndSendResponse(a_hEndpoint, &a_hContext, uStatus, OpcUa_Null, OpcUa_Null);
		}
		else
			// Send an error response.	
			OpcUa_Endpoint_EndSendResponse(a_hEndpoint, &a_hContext, uStatus, OpcUa_Null, OpcUa_Null);
	}
	else
		// Send an error response.	
		OpcUa_Endpoint_EndSendResponse(a_hEndpoint, &a_hContext, uStatus, OpcUa_Null, OpcUa_Null);
	return uStatus;
}

// Called when a QueryNext is called by the client
OpcUa_StatusCode Server_BeginQueryNext(
	OpcUa_Endpoint        a_hEndpoint,
	OpcUa_Handle          a_hContext,
	OpcUa_Void**          a_ppRequest,
	OpcUa_EncodeableType* a_pRequestType)
{
	CSessionServer* pSessionServer = 0;
	OpcUa_QueryNextRequest* pQueryNextRequest = OpcUa_Null;
	CServerApplication* pServerApplication = 0;

	OpcUa_StatusCode uStatus = OpcUa_Good;

	if (!a_hEndpoint)
		uStatus = OpcUa_BadInvalidArgument;
	if (!a_hContext)
		uStatus = OpcUa_BadInvalidArgument;
	if (!a_ppRequest)
		uStatus = OpcUa_BadInvalidArgument;
	if (*a_ppRequest == NULL)
		uStatus = OpcUa_BadInvalidArgument;
	if (!a_pRequestType)
		uStatus = OpcUa_BadInvalidArgument;
	else
	{
		if (a_pRequestType->TypeId != OpcUaId_QueryNextRequest)
			uStatus = OpcUa_BadInvalidArgument;
	}
	if (uStatus == OpcUa_Good)
	{
		pQueryNextRequest = (OpcUa_QueryNextRequest*)*a_ppRequest;


		// Get the server application object.
		uStatus = OpcUa_Endpoint_GetCallbackData(a_hEndpoint, (OpcUa_Void**)&pServerApplication);
		if (uStatus == OpcUa_Good)
		{
			OpcUa_UInt32 uSecureChannelId;
			uStatus = OpcUa_Endpoint_GetMessageSecureChannelId(a_hEndpoint, a_hContext, &uSecureChannelId);
			if (uStatus == OpcUa_Good)
			{
				// find the session.
				OpcUa_NodeId AuthenticationToken = pQueryNextRequest->RequestHeader.AuthenticationToken;
				uStatus = pServerApplication->FindSession(uSecureChannelId, &AuthenticationToken, &pSessionServer);
				if (uStatus == OpcUa_Good)
				{
					// Signal that the session is alive
					OpcUa_Semaphore_Post(pSessionServer->m_SessionTimeoutSem, 1);
					// Effacement des requetes de lecture déjà traitée
					if (pSessionServer->RemoveAllQueryNextRequestDeleted() != OpcUa_Good)
						OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "RemoveAllQueryNextRequest failed\n");
					// queue the request.
					CQueuedQueryNextMessage* pQueuedQueryNextMessage = new CQueuedQueryNextMessage(pQueryNextRequest, a_hEndpoint, a_hContext, a_pRequestType);
					// Add to queue
					pSessionServer->QueueQueryNextMessage(pQueuedQueryNextMessage);
					// Must set this to null to prevent the caller from deleting it.
					*a_ppRequest = OpcUa_Null;
				}
			}
		}
	}
	return uStatus;
}

// Called when a Read request is received from the Client.
OpcUa_StatusCode Server_BeginRead(
	OpcUa_Endpoint        a_hEndpoint,
	OpcUa_Handle          a_hContext,
	OpcUa_Void**          a_ppRequest,
	OpcUa_EncodeableType* a_pRequestType)
{
	CSessionServer* pSessionServer = 0;
	OpcUa_ReadRequest* pReadRequest = OpcUa_Null;
	CServerApplication* pServerApplication = 0;

	OpcUa_StatusCode uStatus=OpcUa_Good;

	if (!a_hEndpoint)
		uStatus=OpcUa_BadInvalidArgument;
	if(!a_hContext)
		uStatus=OpcUa_BadInvalidArgument;
	if(!a_ppRequest)
		uStatus=OpcUa_BadInvalidArgument;
	if(*a_ppRequest==NULL)
		uStatus=OpcUa_BadInvalidArgument;
	if (!a_pRequestType)
		uStatus=OpcUa_BadInvalidArgument;
	else
	{
		if (a_pRequestType->TypeId != OpcUaId_ReadRequest)
			uStatus= OpcUa_BadInvalidArgument;
	}
	if (uStatus==OpcUa_Good)
	{
		pReadRequest = (OpcUa_ReadRequest*)*a_ppRequest;
		

		// Get the server application object.
		uStatus = OpcUa_Endpoint_GetCallbackData(a_hEndpoint, (OpcUa_Void**)&pServerApplication);
		if (uStatus==OpcUa_Good)
		{
			OpcUa_UInt32 uSecureChannelId=0;
			uStatus = OpcUa_Endpoint_GetMessageSecureChannelId(a_hEndpoint, a_hContext, &uSecureChannelId);
			if (uStatus==OpcUa_Good)
			{
				// find the session.
				OpcUa_NodeId AuthenticationToken=pReadRequest->RequestHeader.AuthenticationToken;
				uStatus=pServerApplication->FindSession(uSecureChannelId,&AuthenticationToken,&pSessionServer);
				if (uStatus == OpcUa_Good)
				{
					// Signal that the session is alive
					OpcUa_Semaphore_Post(pSessionServer->m_SessionTimeoutSem, 1);
					// Effacement des requetes de lecture déjà traitée
					if (pSessionServer->RemoveAllReadRequestDeleted() != OpcUa_Good)
						OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "RemoveAllReadRequest failed\n");
					// queue the request.
					CQueuedReadMessage* pQueuedReadMessage = new CQueuedReadMessage(pReadRequest, a_hEndpoint, a_hContext, a_pRequestType);
					// Add to queue
					pSessionServer->QueueReadMessage(pQueuedReadMessage);
					// Must set this to null to prevent the caller from deleting it.
					*a_ppRequest = OpcUa_Null;
					// Wakeup the AsyncThread for the related session
					pSessionServer->WakeupAsyncRequestThread();
				}
				else
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Read failed because sessionId %u doesn't exist\n", uSecureChannelId);
			}
			else
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Read failed because no SecureChannel Id exist for this context : %x \n", a_hContext);
		}
		else
			// Send an error response.
			OpcUa_Endpoint_EndSendResponse(a_hEndpoint, &a_hContext, uStatus, OpcUa_Null, OpcUa_Null);
	}
	else
		// Send an error response.
		OpcUa_Endpoint_EndSendResponse(a_hEndpoint, &a_hContext, uStatus, OpcUa_Null, OpcUa_Null);

	return uStatus;
}


OpcUa_StatusCode Server_Write(
	OpcUa_Endpoint             a_hEndpoint,
	OpcUa_Handle               a_hContext,
	const OpcUa_RequestHeader* a_pRequestHeader,
	OpcUa_Int32                a_nNoOfNodesToWrite,
	const OpcUa_WriteValue*    a_pNodesToWrite,
	OpcUa_ResponseHeader*      a_pResponseHeader,
	OpcUa_Int32*               a_pNoOfResults,
	OpcUa_StatusCode**         a_pResults,
	OpcUa_Int32*               a_pNoOfDiagnosticInfos,
	OpcUa_DiagnosticInfo**     a_pDiagnosticInfos)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_StatusCode uServiceResult=OpcUa_Good;
	CServerApplication* pServerApplication = OpcUa_Null;
	OpcUa_UInt32 uSecureChannelId = 0;
	CSessionServer* pSession = 0;

	if (a_hEndpoint)
	{
		if (a_hContext)
		{
			if (a_pRequestHeader)
			{
				if (a_pNodesToWrite)
				{
					if (a_pNodesToWrite==0)
						uStatus=OpcUa_BadNothingToDo;
					else
					{
						// Get the server application object.
						uStatus = OpcUa_Endpoint_GetCallbackData(a_hEndpoint, (OpcUa_Void**)&pServerApplication);
						if (uStatus==OpcUa_Good)
						{
							// Get the secure channel being used.
							uStatus = OpcUa_Endpoint_GetMessageSecureChannelId(a_hEndpoint, a_hContext, &uSecureChannelId);
							if (uStatus==OpcUa_Good)
							{
								try
								{
									// find the session.
									uStatus=pServerApplication->FindSession(uSecureChannelId,&a_pRequestHeader->AuthenticationToken,&pSession);
									if (uStatus==OpcUa_Good)
									{
										*a_pResults=(OpcUa_StatusCode*)OpcUa_Alloc(a_nNoOfNodesToWrite*sizeof(OpcUa_StatusCode));
										*a_pNoOfResults=a_nNoOfNodesToWrite;
										CUAInformationModel* pInformationModel=CServerApplication::m_pTheAddressSpace;
										OpcUa_Mutex_Lock(pInformationModel->GetServerCacheMutex());
										for (OpcUa_Int32 ii=0;ii<a_nNoOfNodesToWrite;ii++)
										{
											CUABase* pUABase=OpcUa_Null;
											uStatus = pInformationModel->GetNodeIdFromDictionnary(a_pNodesToWrite[ii].NodeId, &pUABase);
											if (uStatus == OpcUa_Good)
											{
												switch (pUABase->GetNodeClass())
												{
												case OpcUa_NodeClass_DataType:
													break;
												case OpcUa_NodeClass_ReferenceType:
													break;
												case OpcUa_NodeClass_Object:
													(*a_pResults)[ii] = ((CUAObject*)pUABase)->Write(
														a_pNodesToWrite[ii].AttributeId,
														a_pNodesToWrite[ii].IndexRange,
														a_pNodesToWrite[ii].Value);
													break;
												case OpcUa_NodeClass_ObjectType:
													break;
												case OpcUa_NodeClass_Unspecified:
													break;
												case OpcUa_NodeClass_Variable:
													{
														CUAVariable* pUAVariable=(CUAVariable*)pUABase;
														if (pUAVariable->GetPData())
														{
															// on a faire une variable reliée a un VPI
															CVpiDevice* pDevice=OpcUa_Null;
															CVpiTag* pSignal=(CVpiTag*)((CUAVariable*)pUABase)->GetPData();
															if (pSignal)
															{
																pDevice = pSignal->GetDevice();
																if (pDevice)
																{
																	CVpiFuncCaller* pFuncCaller = pDevice->GetVpiFuncCaller();
																	OpcUa_Byte bBuiltInType = pSignal->GetUAVariable()->GetBuiltInType();
																	CVpiDataValue* pDataValue = new CVpiDataValue(bBuiltInType, pSignal->GetNbElement(),OpcUa_False);
																	pDataValue->SetArrayType(OpcUa_VariantArrayType_Scalar);
																	if (pSignal)
																		pDataValue->SetDataType((OpcUa_Byte)pSignal->GetDataType().Identifier.Numeric);
																	OpcUa_Variant aVariant = a_pNodesToWrite[ii].Value.Value;
																	pDataValue->SetValue(&aVariant);
																	// support for Latin1 to Utf8 encoding
																	//if (aVariant.Datatype == OpcUaType_String)
																	//{
																	//	// Check if we need to convert from Latin1ToUtf8
																	//	pDataValue->Utf8ToLatin1();
																	//}
																	CVpiWriteObject* pVpiWriteObject = new CVpiWriteObject(a_pNodesToWrite[ii].NodeId, pDataValue, pFuncCaller, ((CUAVariable*)pUABase));
																	if (pVpiWriteObject)
																	{
																		pDevice->AddWriteObject(pVpiWriteObject);
																	}
																	else
																		uStatus = OpcUa_BadOutOfMemory;
																}
																else
																	uStatus = OpcUa_BadInternalError;
															}
															else
																uStatus = OpcUa_BadInternalError;
														}
														(*a_pResults)[ii]=((CUAVariable*)pUABase)->Write(
															a_pNodesToWrite[ii].AttributeId,
															a_pNodesToWrite[ii].IndexRange,
															a_pNodesToWrite[ii].Value);
													}
													break;
												case OpcUa_NodeClass_VariableType:
													break;
												case OpcUa_NodeClass_View:
													break;
												case OpcUa_NodeClass_Method:
													(*a_pResults)[ii] = ((CUAMethod*)pUABase)->Write(
														a_pNodesToWrite[ii].AttributeId,
														a_pNodesToWrite[ii].IndexRange,
														a_pNodesToWrite[ii].Value);
													break;
												default:
													uStatus=OpcUa_BadInvalidArgument;
													break;
												}
											}
											else
											{
												(*a_pResults)[ii] = OpcUa_BadNodeIdUnknown;
												if (uStatus == OpcUa_BadInvalidArgument) // If we got de default error code the we return Good according to the CTT
													uStatus = OpcUa_Good; 
											}
										}
										// wake up all subscription thread where this UAVariable is monitored
										OpcUa_Mutex_Unlock(pInformationModel->GetServerCacheMutex());
										//pServerApplication->WakeupAllSubscription();
									}
									else
										uStatus=OpcUa_BadSessionIdInvalid;
								}
								catch (CStatusCodeException* e)
								{
									uStatus = e->GetCode();
								}
							}
							else
								uStatus=OpcUa_BadNotConnected;
						}
						else
							uStatus=OpcUa_BadNotConnected;
					}
				}
				else
					uStatus=OpcUa_BadNothingToDo;
			}
			else
				uStatus=OpcUa_BadInvalidArgument;
		}
		else
			uStatus=OpcUa_BadInvalidArgument;
	}
	else
		uStatus=OpcUa_BadInvalidArgument;
	
	// Update the response header.
	a_pResponseHeader->Timestamp     = OpcUa_DateTime_UtcNow();
	if (a_pRequestHeader)
		a_pResponseHeader->RequestHandle = a_pRequestHeader->RequestHandle;
	a_pResponseHeader->ServiceResult = uServiceResult;

	// diagnostic info
	if (!a_pNoOfDiagnosticInfos)
		a_pNoOfDiagnosticInfos=(OpcUa_Int32*)OpcUa_Alloc(sizeof(OpcUa_Int32));
	*a_pNoOfDiagnosticInfos=0;
	(*a_pDiagnosticInfos) = OpcUa_Null;//(OpcUa_DiagnosticInfo*)OpcUa_Alloc(sizeof(OpcUa_DiagnosticInfo));
	//OpcUa_DiagnosticInfo_Initialize((*a_pDiagnosticInfos)[0]);
	return uStatus;
}
//
// Il s'agit du point central de reception des evenements transmis par la stack
// 
//OpcUa_StatusCode Server_EndpointCallback( 
//    OpcUa_Endpoint				a_hEndpoint,
//	OpcUa_Void*					a_pCallbackData,
//    OpcUa_Endpoint_Event		a_eEvent,
//    OpcUa_StatusCode			a_uStatus,
//    OpcUa_UInt32				a_uSecureChannelId,
//    OpcUa_ByteString*			a_pbsClientCertificate,
//	OpcUa_String*				a_pSecurityPolicy,
//	OpcUa_MessageSecurityMode	a_uSecurityMode)

OpcUa_StatusCode Server_EndpointCallback( 
	OpcUa_Endpoint          a_hEndpoint,
	OpcUa_Void*             a_pCallbackData,
	OpcUa_Endpoint_Event    a_eEvent,
	OpcUa_StatusCode        a_uStatus,
	OpcUa_UInt32            a_uSecureChannelId,
	OpcUa_Handle*           a_phRawRequestContext,
	OpcUa_ByteString*       a_pbsClientCertificate,
	OpcUa_String*           a_pSecurityPolicy,
	OpcUa_UInt16            a_uSecurityMode,
	OpcUa_UInt32            a_uRequestedLifetime)
{
	OpcUa_StatusCode uStatus=OpcUa_BadInvalidArgument;
	if (a_pCallbackData)
	{
		CServerApplication* pServerApplication = (CServerApplication*)a_pCallbackData;
		if (pServerApplication)
		{
			OpcUa_ReferenceParameter(a_hEndpoint);
			uStatus=pServerApplication->OnEndpointCallback(
				a_eEvent,
				a_uStatus,
				a_uSecureChannelId,
				a_phRawRequestContext,
				a_pbsClientCertificate,
				a_pSecurityPolicy,
				(OpcUa_MessageSecurityMode)a_uSecurityMode,
				a_uRequestedLifetime);
		}
	}
	return uStatus;
}


// Function name   : Server_CreateSubscription
// Description     : 
// Return type     : OpcUa_StatusCode 
// Argument        : pcUa_Endpoint             a_hEndpoint
// Argument        : OpcUa_Handle               a_hContext
// Argument        : const OpcUa_RequestHeader* a_pRequestHeader
// Argument        : OpcUa_Double               a_nRequestedPublishingInterval
// Argument        : OpcUa_UInt32               a_nRequestedLifetimeCount
// Argument        : OpcUa_UInt32               a_nRequestedMaxKeepAliveCount
// Argument        : OpcUa_UInt32               a_nMaxNotificationsPerPublish
// Argument        : OpcUa_Boolean              a_bPublishingEnabled
// Argument        : OpcUa_Byte                 a_nPriority
// Argument        : OpcUa_ResponseHeader*      a_pResponseHeader
// Argument        : OpcUa_UInt32*              a_pSubscriptionId
// Argument        : OpcUa_Double*              a_pRevisedPublishingInterval
// Argument        : OpcUa_UInt32*              a_pRevisedLifetimeCount
// Argument        : OpcUa_UInt32*              a_pRevisedMaxKeepAliveCount
//CreateSubscription> RequestedPublishingInterval={a_nRequestedPublishingInterval} MaxNotificationsPerPublish={a_nMaxNotificationsPerPublish}
OpcUa_StatusCode Server_CreateSubscription(
	OpcUa_Endpoint             a_hEndpoint,
	OpcUa_Handle               a_hContext,
	const OpcUa_RequestHeader* a_pRequestHeader,
	OpcUa_Double               a_nRequestedPublishingInterval,
	OpcUa_UInt32               a_nRequestedLifetimeCount,
	OpcUa_UInt32               a_nRequestedMaxKeepAliveCount,
	OpcUa_UInt32               a_nMaxNotificationsPerPublish,
	OpcUa_Boolean              a_bPublishingEnabled,
	OpcUa_Byte                 a_nPriority, //
	OpcUa_ResponseHeader*      a_pResponseHeader, // out
	OpcUa_UInt32*              a_pSubscriptionId, // out
	OpcUa_Double*              a_pRevisedPublishingInterval, // out
	OpcUa_UInt32*              a_pRevisedLifetimeCount, // out 
	OpcUa_UInt32*              a_pRevisedMaxKeepAliveCount) // out
{
	OpcUa_UInt32 uSecureChannelId = 0;
	CServerApplication* pServerApplication = 0;
	CSessionServer* pSession = OpcUa_Null;

	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_StatusCode uServiceResult=OpcUa_Good;

	OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_INFO, "Server_CreateSubscription receive from the client\n");
	// Get the server application object.
	uStatus = OpcUa_Endpoint_GetCallbackData(a_hEndpoint, (OpcUa_Void**)&pServerApplication);
	if (uStatus==OpcUa_Good)
	{

		// Get the secure channel being used.
		uStatus = OpcUa_Endpoint_GetMessageSecureChannelId(a_hEndpoint, a_hContext, &uSecureChannelId);
		if (uStatus==OpcUa_Good)
		{
			// find the session.
			uStatus= pServerApplication->FindSession(uSecureChannelId,&a_pRequestHeader->AuthenticationToken,&pSession);
			if ( (uStatus==OpcUa_Good)&& (pSession))
			{
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_WARNING, "Server_CreateSubscription receive from the client\n");
				CSubscriptionServer* pSubscription=NULL;
				OpcUa_Semaphore_Post(pSession->m_SessionTimeoutSem,1);
				// Maintenant on peut réaliser la création de la souscription
				uServiceResult=pSession->CreateSubscription(uSecureChannelId,
													 a_nRequestedPublishingInterval,
													 a_nRequestedLifetimeCount,
													 a_nRequestedMaxKeepAliveCount,
													 a_nMaxNotificationsPerPublish,
													 a_bPublishingEnabled,
													 a_nPriority,
													 &pSubscription); // out
				if (uServiceResult==OpcUa_Good)
				{
					*a_pSubscriptionId=pSubscription->GetSubscriptionId();
					*a_pRevisedPublishingInterval=pSubscription->GetPublishingInterval();
					*a_pRevisedLifetimeCount=pSubscription->GetLifetimeCount();
					*a_pRevisedMaxKeepAliveCount=pSubscription->GetMaxKeepAliveCount();
				}
				else
					uStatus=OpcUa_Good;
			}
			else
				uStatus=OpcUa_BadSessionIdInvalid;;
		}		
	}
	a_pResponseHeader->RequestHandle=a_pRequestHeader->RequestHandle;
	a_pResponseHeader->NoOfStringTable=0;
	a_pResponseHeader->StringTable=NULL;
	a_pResponseHeader->Timestamp= OpcUa_DateTime_UtcNow();
	a_pResponseHeader->RequestHandle = a_pRequestHeader->RequestHandle;
	a_pResponseHeader->ServiceResult = uServiceResult;

	return uStatus;
}


// Function name   : Server_ModifySubscription
// Description     : 
// Return type     : OpcUa_StatusCode 
// Argument        : pcUa_Endpoint             a_hEndpoint
// Argument        : OpcUa_Handle               a_hContext
// Argument        : const OpcUa_RequestHeader* a_pRequestHeader
// Argument        : OpcUa_UInt32               a_nSubscriptionId
// Argument        : OpcUa_Double               a_nRequestedPublishingInterval
// Argument        : OpcUa_UInt32               a_nRequestedLifetimeCount
// Argument        : OpcUa_UInt32               a_nRequestedMaxKeepAliveCount
// Argument        : OpcUa_UInt32               a_nMaxNotificationsPerPublish
// Argument        : OpcUa_Byte                 a_nPriority
// Argument        : OpcUa_ResponseHeader*      a_pResponseHeader
// Argument        : OpcUa_Double*              a_pRevisedPublishingInterval
// Argument        : OpcUa_UInt32*              a_pRevisedLifetimeCount
// Argument        : OpcUa_UInt32*              a_pRevisedMaxKeepAliveCount

OpcUa_StatusCode Server_ModifySubscription(
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
	OpcUa_UInt32*              a_pRevisedMaxKeepAliveCount)
{
	OpcUa_UInt32 uSecureChannelId = 0;
	CServerApplication* pServerApplication = 0;
	CSessionServer* pSession = 0;
	OpcUa_StatusCode uStatus=OpcUa_Good;
	// Get the server application object.
	uStatus = OpcUa_Endpoint_GetCallbackData(a_hEndpoint, (OpcUa_Void**)&pServerApplication);
	if (uStatus==OpcUa_Good)
	{
		// Get the secure channel being used.
		uStatus = OpcUa_Endpoint_GetMessageSecureChannelId(a_hEndpoint, a_hContext, &uSecureChannelId);
		if (uStatus==OpcUa_Good)
		{
			uStatus = OpcUa_Endpoint_GetMessageSecureChannelId(a_hEndpoint, a_hContext, &uSecureChannelId);
			if (uStatus==OpcUa_Good)
			{
				// find the session.
				OpcUa_NodeId aSessionId=a_pRequestHeader->AuthenticationToken;
				uStatus= pServerApplication->FindSession(uSecureChannelId,&aSessionId,&pSession);
				if (uStatus==OpcUa_Good)
				{
					uStatus=pSession->ModifySubscription(a_nSubscriptionId,
														a_nRequestedPublishingInterval,
														a_nRequestedLifetimeCount,
														a_nRequestedMaxKeepAliveCount,
														a_nMaxNotificationsPerPublish,
														a_nPriority,
														a_pRevisedPublishingInterval,a_pRevisedLifetimeCount,a_pRevisedMaxKeepAliveCount);
				}
				else
					uStatus=OpcUa_BadSessionIdInvalid;
			}
		}
	}
	// Update the response header.
	a_pResponseHeader->Timestamp     = OpcUa_DateTime_UtcNow();
	a_pResponseHeader->RequestHandle = a_pRequestHeader->RequestHandle;
	a_pResponseHeader->ServiceResult = uStatus;
	return uStatus;
}


// Function name   : Server_DeleteSubscriptions
// Description     : 
// Return type     : OpcUa_StatusCode 
// Argument        : pcUa_Endpoint             a_hEndpoint
// Argument        : OpcUa_Handle               a_hContext
// Argument        : const OpcUa_RequestHeader* a_pRequestHeader
// Argument        : OpcUa_Int32                a_nNoOfSubscriptionIds
// Argument        : const OpcUa_UInt32*        a_pSubscriptionIds
// Argument        : OpcUa_ResponseHeader*      a_pResponseHeader
// Argument        : OpcUa_Int32*               a_pNoOfResults
// Argument        : OpcUa_StatusCode**         a_pResults
// Argument        : OpcUa_Int32*               a_pNoOfDiagnosticInfos
// Argument        : OpcUa_DiagnosticInfo**     a_pDiagnosticInfos

OpcUa_StatusCode Server_DeleteSubscriptions(
	OpcUa_Endpoint             a_hEndpoint,
	OpcUa_Handle               a_hContext,
	const OpcUa_RequestHeader* a_pRequestHeader,
	OpcUa_Int32                a_nNoOfSubscriptionIds,
	const OpcUa_UInt32*        a_pSubscriptionIds,
	OpcUa_ResponseHeader*      a_pResponseHeader,
	OpcUa_Int32*               a_pNoOfResults,
	OpcUa_StatusCode**         a_pResults,
	OpcUa_Int32*               a_pNoOfDiagnosticInfos,
	OpcUa_DiagnosticInfo**     a_pDiagnosticInfos)
{
	OpcUa_ReferenceParameter(a_pNoOfDiagnosticInfos);
	OpcUa_ReferenceParameter(a_pDiagnosticInfos);
	OpcUa_UInt32 uSecureChannelId = 0;
	CServerApplication* pServerApplication = 0;
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_StatusCode uServiceResult=OpcUa_Good;
	CSessionServer* pSession = 0;
	OpcUa_StatusCode*         pMyResults=OpcUa_Null;

	OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_INFO, "Server_DeleteSubscriptions receive from the client\n");
	if (a_nNoOfSubscriptionIds>0)
	{
		pMyResults=(OpcUa_StatusCode*)OpcUa_Alloc(a_nNoOfSubscriptionIds*sizeof(OpcUa_StatusCode));
		// Get the server application object.
		uStatus = OpcUa_Endpoint_GetCallbackData(a_hEndpoint, (OpcUa_Void**)&pServerApplication);
		if (uStatus==OpcUa_Good)
		{
			OpcUa_NodeId AuthenticationToken=a_pRequestHeader->AuthenticationToken;
			// Get the secure channel being used.
			uStatus = OpcUa_Endpoint_GetMessageSecureChannelId(a_hEndpoint, a_hContext, &uSecureChannelId);
			if (uStatus==OpcUa_Good)
			{
				// find the session.
				uStatus = pServerApplication->FindSession(uSecureChannelId,&AuthenticationToken,&pSession);
				if (uStatus==OpcUa_Good)
				{
					OpcUa_Semaphore_Post(pSession->m_SessionTimeoutSem,1);
					for (OpcUa_Int32 ii=0;ii<a_nNoOfSubscriptionIds;ii++)
					{
						uServiceResult=pSession->RemoveSubscriptionBySubscriptionId(a_pSubscriptionIds[ii]);	
						pMyResults[ii]=uServiceResult;
					}
					pSession->UpdateAsyncThreadInterval();
					// 
					pSession->UpdateNotificationDataThreadInterval();
				}
			}
		}
		*a_pNoOfResults=a_nNoOfSubscriptionIds;
		if (a_nNoOfSubscriptionIds > 0)
		{
			(*a_pResults) = (OpcUa_StatusCode*)OpcUa_Alloc(a_nNoOfSubscriptionIds*sizeof(OpcUa_StatusCode));
			for (OpcUa_Int32 ii = 0; ii < a_nNoOfSubscriptionIds; ii++)
			{
				(*a_pResults)[ii] = pMyResults[ii];
			}
		}
		OpcUa_Free(pMyResults);
	}
	else
		uStatus=OpcUa_BadNothingToDo;
	// Update the response header.
	a_pResponseHeader->Timestamp     = OpcUa_DateTime_UtcNow();
	a_pResponseHeader->RequestHandle = a_pRequestHeader->RequestHandle;
	a_pResponseHeader->ServiceResult = uStatus;
	return uStatus;
}


// Function name   : Server_TransferSubscriptions
// Description     : 
// Return type     : OpcUa_StatusCode 
// Argument        : pcUa_Endpoint             a_hEndpoint
// Argument        : OpcUa_Handle               a_hContext
// Argument        : const OpcUa_RequestHeader* a_pRequestHeader
// Argument        : OpcUa_Int32                a_nNoOfSubscriptionIds
// Argument        : const OpcUa_UInt32*        a_pSubscriptionIds
// Argument        : OpcUa_Boolean              a_bSendInitialValues
// Argument        : OpcUa_ResponseHeader*      a_pResponseHeader
// Argument        : OpcUa_Int32*               a_pNoOfResults
// Argument        : OpcUa_TransferResult**     a_pResults
// Argument        : OpcUa_Int32*               a_pNoOfDiagnosticInfos
// Argument        : OpcUa_DiagnosticInfo**     a_pDiagnosticInfos

OpcUa_StatusCode Server_TransferSubscriptions(
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
	OpcUa_DiagnosticInfo**     a_pDiagnosticInfos)
{
	OpcUa_ReferenceParameter(a_pDiagnosticInfos);
	OpcUa_ReferenceParameter(a_pNoOfDiagnosticInfos);
	OpcUa_ReferenceParameter(a_pResults);
	OpcUa_ReferenceParameter(a_pNoOfResults);
	OpcUa_ReferenceParameter(a_pResponseHeader);
	OpcUa_ReferenceParameter(a_pSubscriptionIds);
	OpcUa_ReferenceParameter(a_nNoOfSubscriptionIds);
	OpcUa_ReferenceParameter(a_pRequestHeader);
	OpcUa_ReferenceParameter(a_bSendInitialValues);
	OpcUa_UInt32 uSecureChannelId = 0;
	CServerApplication* pServerApplication = 0;
	CSessionServer* pSession = 0;
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_StatusCode uServiceResult = OpcUa_Good;
	// Get the server application object.
	uStatus = OpcUa_Endpoint_GetCallbackData(a_hEndpoint, (OpcUa_Void**)&pServerApplication);
	if (uStatus==OpcUa_Good)
	{
		// Get the secure channel being used.
		uStatus = OpcUa_Endpoint_GetMessageSecureChannelId(a_hEndpoint, a_hContext, &uSecureChannelId);
		if (uStatus==OpcUa_Good)
		{
			// find the session.
			OpcUa_NodeId aSessionId = a_pRequestHeader->AuthenticationToken;
			uStatus = pServerApplication->FindSession(uSecureChannelId, &aSessionId, &pSession);
			if (uStatus == OpcUa_Good)
			{
				(*a_pResults)=(OpcUa_TransferResult*)OpcUa_Alloc(sizeof(OpcUa_TransferResult)*a_nNoOfSubscriptionIds);
				for (OpcUa_Int32 i = 0; i < a_nNoOfSubscriptionIds; i++)
				{
					OpcUa_TransferResult_Initialize(&(*a_pResults)[i]);
					// Implement the messahe in CSessionServer
					//pSession->TransfertSubscription();
				}
				*a_pNoOfResults = a_nNoOfSubscriptionIds;
			}
		}
	}
	// Update the response header.
	a_pResponseHeader->Timestamp = OpcUa_DateTime_UtcNow();
	a_pResponseHeader->RequestHandle = a_pRequestHeader->RequestHandle;
	a_pResponseHeader->ServiceResult = uServiceResult;
	return uStatus;
}


// Function name   : Server_SetPublishingMode
// Description     : 
// Return type     : OpcUa_StatusCode 
// Argument        : pcUa_Endpoint             a_hEndpoint
// Argument        : OpcUa_Handle               a_hContext
// Argument        : const OpcUa_RequestHeader* a_pRequestHeader
// Argument        : OpcUa_Boolean              a_bPublishingEnabled
// Argument        : OpcUa_Int32                a_nNoOfSubscriptionIds
// Argument        : const OpcUa_UInt32*        a_pSubscriptionIds
// Argument        : OpcUa_ResponseHeader*      a_pResponseHeader
// Argument        : OpcUa_Int32*               a_pNoOfResults
// Argument        : OpcUa_StatusCode**         a_pResults
// Argument        : OpcUa_Int32*               a_pNoOfDiagnosticInfos
// Argument        : OpcUa_DiagnosticInfo**     a_pDiagnosticInfos

OpcUa_StatusCode Server_SetPublishingMode(
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
	OpcUa_DiagnosticInfo**     a_pDiagnosticInfos)
{
	CServerApplication* pServerApplication = 0;
	CSessionServer* pSession = 0;
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_StatusCode uServiceResult=OpcUa_Good;
	// Get the server application object.
	uStatus = OpcUa_Endpoint_GetCallbackData(a_hEndpoint, (OpcUa_Void**)&pServerApplication);
	if (uStatus==OpcUa_Good)
	{
		// Get the secure channel being used.

		OpcUa_UInt32 uSecureChannelId=0;
		uStatus = OpcUa_Endpoint_GetMessageSecureChannelId(a_hEndpoint, a_hContext, &uSecureChannelId);
		if (uStatus==OpcUa_Good)
		{
			// find the session.
			OpcUa_NodeId aSessionId=a_pRequestHeader->AuthenticationToken;
			uStatus = pServerApplication->FindSession(uSecureChannelId,&aSessionId,&pSession);
			if (uStatus==OpcUa_Good)
			{
				uServiceResult=pSession->SetPublishingMode(a_bPublishingEnabled,
													a_nNoOfSubscriptionIds,
													a_pSubscriptionIds,													
													a_pNoOfResults,
													a_pResults);
			}
		}
	}
	// Mise en forme des OpcUa_DiagnosticInfo
	//a_pNoOfDiagnosticInfos =  (OpcUa_Int32*)OpcUa_Alloc(sizeof(OpcUa_Int32));
	*a_pNoOfDiagnosticInfos=0;
	*a_pDiagnosticInfos = OpcUa_Null; //(OpcUa_DiagnosticInfo*)OpcUa_Alloc(sizeof(OpcUa_DiagnosticInfo));
	//ZeroMemory(*a_pDiagnosticInfos,sizeof(OpcUa_DiagnosticInfo));
	// Update the response header.
	a_pResponseHeader->Timestamp     = OpcUa_DateTime_UtcNow();
	a_pResponseHeader->RequestHandle = a_pRequestHeader->RequestHandle;
	a_pResponseHeader->ServiceResult = uServiceResult;
	return uStatus;
}

OpcUa_StatusCode Server_Republish(
	OpcUa_Endpoint             a_hEndpoint,
	OpcUa_Handle               a_hContext,
	const OpcUa_RequestHeader* a_pRequestHeader,
	OpcUa_UInt32               a_nSubscriptionId,
	OpcUa_UInt32               a_nRetransmitSequenceNumber,
	OpcUa_ResponseHeader*      a_pResponseHeader,
	OpcUa_NotificationMessage* a_pNotificationMessage)
{
	OpcUa_ReferenceParameter(a_pRequestHeader);
	OpcUa_ReferenceParameter(a_nRetransmitSequenceNumber);
	OpcUa_ReferenceParameter(a_nSubscriptionId);
	OpcUa_ReferenceParameter(a_pResponseHeader);
	OpcUa_ReferenceParameter(a_pNotificationMessage);
	OpcUa_UInt32 uSecureChannelId = 0;
	CServerApplication* pServerApplication = 0;
	CSessionServer* pSession = 0;
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_StatusCode uServiceResult=OpcUa_Good;
	// Get the server application object.
	uStatus = OpcUa_Endpoint_GetCallbackData(a_hEndpoint, (OpcUa_Void**)&pServerApplication);
	if (uStatus==OpcUa_Good)
	{
		// Get the secure channel being used.
		uStatus = OpcUa_Endpoint_GetMessageSecureChannelId(a_hEndpoint, a_hContext, &uSecureChannelId);
		if (uStatus==OpcUa_Good)
		{
			// find the session.
			OpcUa_NodeId aSessionId=a_pRequestHeader->AuthenticationToken;
			uStatus = pServerApplication->FindSession(uSecureChannelId,&aSessionId,&pSession);
			if (uStatus==OpcUa_Good)
			{
				uServiceResult=pSession->Republish(a_nSubscriptionId,a_nRetransmitSequenceNumber,&a_pNotificationMessage);
			}
		}
	}
	// Update the response header.
	a_pResponseHeader->Timestamp     = OpcUa_DateTime_UtcNow();
	a_pResponseHeader->RequestHandle = a_pRequestHeader->RequestHandle;
	a_pResponseHeader->ServiceResult = uServiceResult;
	return uStatus;
}

OpcUa_StatusCode Server_CreateMonitoredItems(
	OpcUa_Endpoint                          a_hEndpoint, // in
	OpcUa_Handle                            a_hContext, // in
	const OpcUa_RequestHeader*              a_pRequestHeader, // in
	OpcUa_UInt32                            a_nSubscriptionId, // in
	OpcUa_TimestampsToReturn                a_eTimestampsToReturn, // in
	OpcUa_Int32                             a_nNoOfItemsToCreate, // in
	const OpcUa_MonitoredItemCreateRequest* a_pItemsToCreate, // in
	OpcUa_ResponseHeader*                   a_pResponseHeader, // out
	OpcUa_Int32*                            a_pNoOfResults,// out
	OpcUa_MonitoredItemCreateResult**       a_pResults,// out
	OpcUa_Int32*                            a_pNoOfDiagnosticInfos,// out
	OpcUa_DiagnosticInfo**                  a_pDiagnosticInfos)// out
{
	OpcUa_ReferenceParameter(a_pNoOfDiagnosticInfos);
	OpcUa_ReferenceParameter(a_pDiagnosticInfos);
	OpcUa_ReferenceParameter(a_eTimestampsToReturn);
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_UInt32 uSecureChannelId = 0;
	CServerApplication* pServerApplication = OpcUa_Null;
	CSessionServer* pSession=NULL;
	CSubscriptionServer* pSubscription=NULL;
	OpcUa_StatusCode uServiceResult=OpcUa_Good;

	OpcUa_ReturnErrorIfArgumentNull(a_hEndpoint);
	OpcUa_ReturnErrorIfArgumentNull(a_hContext);
	OpcUa_ReturnErrorIfArgumentNull(a_pRequestHeader);


	OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_INFO, "Server_CreateMonitoredItems receive for %u items from a client on SubscriptionId=%u\n", a_nNoOfItemsToCreate, a_nSubscriptionId);
	// check a_nNoOfItemsToCreate
	if (a_nNoOfItemsToCreate==0)
	 uServiceResult=OpcUa_BadNothingToDo;
	else
	{
		// Check a_eTimestampsToReturn
		if ( (a_eTimestampsToReturn != OpcUa_TimestampsToReturn_Source) 
			&& ( a_eTimestampsToReturn!= OpcUa_TimestampsToReturn_Server) 
			&& (a_eTimestampsToReturn!=OpcUa_TimestampsToReturn_Both) 
			&& (a_eTimestampsToReturn!=OpcUa_TimestampsToReturn_Neither) )
			uServiceResult=OpcUa_BadTimestampsToReturnInvalid;
		else
		{
			//OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"in: a_pResults:%x\n",a_pResults);
			// Get the server application object.
			uStatus = OpcUa_Endpoint_GetCallbackData(a_hEndpoint, (OpcUa_Void**)&pServerApplication);
			if (uStatus==OpcUa_Good)
			{
				// Get the secure channel being used.
				uStatus = OpcUa_Endpoint_GetMessageSecureChannelId(a_hEndpoint, a_hContext, &uSecureChannelId);
				if (uStatus==OpcUa_Good)
				{
					CUAInformationModel* pInformationModel=CServerApplication::m_pTheAddressSpace;
					// find the session.
					uStatus = pServerApplication->FindSession(uSecureChannelId, &a_pRequestHeader->AuthenticationToken, &pSession);
					if (uStatus==OpcUa_Good)
					{
						OpcUa_Semaphore_Post(pSession->m_SessionTimeoutSem,1);
						// Find the subscription
						OpcUa_Mutex aSubscriptionListMutex=pSession->GetSubscriptionListMutex();
						OpcUa_Mutex_Lock(aSubscriptionListMutex);
						uStatus=pSession->FindSubscription(a_nSubscriptionId,&pSubscription);
						if ( (uStatus==OpcUa_Good) && (pSubscription))
						{
							pSubscription->SetLastTransmissionTime(OpcUa_DateTime_UtcNow());
							vector<CMonitoredItemServer*> aMonitoredItemBaseList; // il s'agit de la liste des items qui seront réèllement notifiés au client
							for (int ii = 0; ii<a_nNoOfItemsToCreate; ii++)
							{
								// Will first check that a_pItemsToCreate[ii].RequestedParameters.ClientHandle is not already handle in this subscription
								CMonitoredItemServer* pMonitoredItemserver = OpcUa_Null;
								if (pSubscription->FindMonitoredItemByClientHandle(a_pItemsToCreate[ii].RequestedParameters.ClientHandle, &pMonitoredItemserver) == OpcUa_Good)
									OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critical Error. This client handle %u is already handle by this subscription\n", a_pItemsToCreate[ii].RequestedParameters.ClientHandle);
								//if (a_pItemsToCreate[ii].RequestedParameters.Filter.Encoding==OpcUa_ExtensionObjectEncoding_None)
								//	OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Check filter: OpcUa_ExtensionObjectEncoding_None was define. What does it  means ?\n");
								OpcUa_Boolean bInputParamsOk=OpcUa_True;
								OpcUa_StatusCode uStatusPreChecked=OpcUa_Good;
								// Prise en compte d'une demande d'Index Range
								CNumericRange* pNumericRange=OpcUa_Null;
								if (OpcUa_String_StrLen(&(a_pItemsToCreate[ii].ItemToMonitor.IndexRange))>0)
								{
									pNumericRange=new CNumericRange((OpcUa_String*)&(a_pItemsToCreate[ii].ItemToMonitor.IndexRange));
									uStatusPreChecked=pNumericRange->GetStatusCode();
								}

								// Check Filter : OpcUa_DataChangeFilter or OpcUa_EventFilter
								// We start to check the OpcUa_DataChangeFilter
								if (a_pItemsToCreate[ii].RequestedParameters.Filter.Body.EncodeableObject.Type)
								{
									if (a_pItemsToCreate[ii].RequestedParameters.Filter.Body.EncodeableObject.Type->TypeId == OpcUaId_DataChangeFilter) // DataChangeFilter
									{
										
										OpcUa_DataChangeFilter* pDataChangeFilter=
											(OpcUa_DataChangeFilter*)a_pItemsToCreate[ii].RequestedParameters.Filter.Body.EncodeableObject.Object;
										if (a_pItemsToCreate[ii].ItemToMonitor.AttributeId==OpcUa_Attributes_Value)
										{
											if (pDataChangeFilter->Trigger==OpcUa_DataChangeTrigger_Status)
												OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_INFO, "Request to notify only if StatusCode Change\n");
											if (pDataChangeFilter->Trigger==OpcUa_DataChangeTrigger_StatusValue)
												OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_INFO, "Request to notify only if StatusCode or Value Change\n");
											if (pDataChangeFilter->Trigger==OpcUa_DataChangeTrigger_StatusValueTimestamp)
												OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_INFO, "Request to notify only if StatusCode or Value or TimeStamp Change\n");

											if (pDataChangeFilter->DeadbandType==OpcUa_DeadbandType_None)
											{
												OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_INFO, "Request no deadband calculation apply\n");
											}
											if (pDataChangeFilter->DeadbandType==OpcUa_DeadbandType_Absolute)
											{
												OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_INFO, "Request absolute deadband calculation apply\n");
											}
											if (pDataChangeFilter->DeadbandType==OpcUa_DeadbandType_Percent)
											{
												OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_INFO, "Request Percent deadband calculation apply\n");
											}
											OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_INFO, "Check OpcUa_DataChangeFilter\n");
										}
										else
										{
											bInputParamsOk=OpcUa_False;
											uStatus=OpcUa_Good;
											uServiceResult = OpcUa_Good;
											if ((pDataChangeFilter->DeadbandType == OpcUa_DeadbandType_Absolute)
												&& (a_pItemsToCreate[ii].ItemToMonitor.AttributeId != OpcUa_Attributes_Value))
											{
												CMonitoredItemServer* pMonitoredItemServer = new CMonitoredItemServer();
												pMonitoredItemServer->SetStatusCode(OpcUa_BadFilterNotAllowed);
												OpcUa_Mutex aMutex = pSubscription->GetMonitoredItemListMutex();
												OpcUa_Mutex_Lock(aMutex);
												aMonitoredItemBaseList.push_back(pMonitoredItemServer);
												OpcUa_Mutex_Unlock(aMutex);
											}
										}
									}
									else
									{
										//
										// We continue with the OpcUa_EventFilter
										//
										if (a_pItemsToCreate[ii].RequestedParameters.Filter.Body.EncodeableObject.Type->TypeId == OpcUaId_EventFilter) // EventFilter
										{
											uStatus=OpcUa_Good;
											uServiceResult=OpcUa_Good;
										}
									}
								}
								if (uStatusPreChecked==OpcUa_Good)
								{
									if (a_pItemsToCreate[ii].ItemToMonitor.AttributeId<23)
									{
										if (bInputParamsOk)
										{
											CUABase* pBase=OpcUa_Null;
											uStatus = pInformationModel->GetNodeIdFromDictionnary(a_pItemsToCreate[ii].ItemToMonitor.NodeId, &pBase);
											if (uStatus == OpcUa_Good)
											{
												// Allocation et remplissage du CMonitoredItemServer
												CMonitoredItemServer* pMonitoredItemServer=new CMonitoredItemServer();
												pMonitoredItemServer->SetUABase(pBase);
												pMonitoredItemServer->SetMonitoringMode(a_pItemsToCreate[ii].MonitoringMode);
												// 
												OpcUa_UInt32 uiItemId = 0;
												uiItemId = (OpcUa_UInt32)pMonitoredItemServer;
												pMonitoredItemServer->SetMonitoredItemId(uiItemId);
												//
												pMonitoredItemServer->SetTimestampsToReturn(a_eTimestampsToReturn);
												pMonitoredItemServer->SetSubscription(pSubscription);
												pMonitoredItemServer->SetNodeId(a_pItemsToCreate[ii].ItemToMonitor.NodeId);
												pMonitoredItemServer->SetAttributeId(a_pItemsToCreate[ii].ItemToMonitor.AttributeId);
												pMonitoredItemServer->SetDataEncoding(a_pItemsToCreate[ii].ItemToMonitor.DataEncoding);
												pMonitoredItemServer->SetIndexRange((OpcUa_String*)&(a_pItemsToCreate[ii].ItemToMonitor.IndexRange));
												pMonitoredItemServer->SetClientHandle(a_pItemsToCreate[ii].RequestedParameters.ClientHandle);
												if (a_pItemsToCreate[ii].RequestedParameters.QueueSize>MONITOREDITEM_MAX_QUEUESIZE)
													pMonitoredItemServer->SetQueueSize(MONITOREDITEM_MAX_QUEUESIZE);
												else
													pMonitoredItemServer->SetQueueSize(a_pItemsToCreate[ii].RequestedParameters.QueueSize);// TODO check que la taille correspond aux capacité du serveur
												// 
												if (a_pItemsToCreate[ii].RequestedParameters.QueueSize==0)
													pMonitoredItemServer->SetQueueSize(MONITOREDITEM_DEFAULT_QUEUESIZE);
												// check the minimum value for the sampling interval associated with the MonitoredItem
												if (a_pItemsToCreate[ii].RequestedParameters.SamplingInterval<CServerApplication::m_dblMiniSamplingInterval)
													pMonitoredItemServer->SetSamplingInterval(CServerApplication::m_dblMiniSamplingInterval);
												else
													pMonitoredItemServer->SetSamplingInterval(a_pItemsToCreate[ii].RequestedParameters.SamplingInterval);
												// check the maximum value for the sampling interval associated with the MonitoredItem
												if (a_pItemsToCreate[ii].RequestedParameters.SamplingInterval>CServerApplication::m_dblMaxSamplingInterval)
													pMonitoredItemServer->SetSamplingInterval(CServerApplication::m_dblMaxSamplingInterval);
												// Handle  Sampling interval diagnostic 
												if (pInformationModel->IsEnabledServerDiagnosticsDefaultValue())
												{
													OpcUa_Double dblSampleInterval = pMonitoredItemServer->GetSamplingInterval();
													OpcUa_SamplingIntervalDiagnosticsDataType* pSamplingIntervalDiagnostic = OpcUa_Null;
													if (!pInformationModel->IsSamplingIntervalDiagnosticsExist(dblSampleInterval, &pSamplingIntervalDiagnostic))
													{
														pSamplingIntervalDiagnostic = (OpcUa_SamplingIntervalDiagnosticsDataType*)OpcUa_Alloc(sizeof(OpcUa_SamplingIntervalDiagnosticsDataType));
														OpcUa_SamplingIntervalDiagnosticsDataType_Initialize(pSamplingIntervalDiagnostic);
														pSamplingIntervalDiagnostic->SamplingInterval = dblSampleInterval;
														pSamplingIntervalDiagnostic->MonitoredItemCount++;
														pSamplingIntervalDiagnostic->MaxMonitoredItemCount++;
														pInformationModel->AddInSamplingIntervalDiagnosticsArray(pSamplingIntervalDiagnostic);
													}
													else
													{
														if (pSamplingIntervalDiagnostic)
														{
															pSamplingIntervalDiagnostic->MonitoredItemCount++;
															// Update the internal Extension object of the node 2289
															OpcUa_NodeId aNodeId;
															OpcUa_NodeId_Initialize(&aNodeId);
															aNodeId.Identifier.Numeric = 2289;
															pInformationModel->UpdateSamplingIntervalDiagnosticsArray(aNodeId);
															OpcUa_NodeId_Clear(&aNodeId);
														}
														else
															OpcUa_Trace(OPCUA_TRACE_SERVER_ERROR, "Internal error on SamplingIntervalDiagnosticsArray management");
													}
												}
												//
												pMonitoredItemServer->DiscardOldest(a_pItemsToCreate[ii].RequestedParameters.DiscardOldest);
												OpcUa_ExtensionObject_Initialize(pMonitoredItemServer->m_pMonitoringFilter);
												//// analyse de Range
												if (pNumericRange)
												{
													if (pBase->GetNodeClass()==OpcUa_NodeClass_Variable)
													{
														CUAVariable* pUAVariable=(CUAVariable*)pBase;
														if (pUAVariable)
														{
															CDataValue* pDataValue= pUAVariable->GetValue();
															if (pDataValue)
															{
																OpcUa_Byte ArrayType=pDataValue->GetArrayType();
																if ( pNumericRange->IsMultiDimensional() )
																{	
																	switch (ArrayType)
																	{
																	case OpcUa_VariantArrayType_Matrix:
																			uStatus=OpcUa_Good;
																			uServiceResult=OpcUa_Good;
																		break;
																	case OpcUa_VariantArrayType_Array:
																		{
																			switch (pDataValue->GetBuiltInDataType())
																			{
																			case OpcUaType_ByteString:
																				uStatus=OpcUa_Good;
																				uServiceResult=OpcUa_Good;
																				break;
																			case OpcUaType_String:
																				uStatus=OpcUa_Good;
																				uServiceResult=OpcUa_Good;
																				break;
																			default:
																				uStatus=OpcUa_Good;
																				uServiceResult=OpcUa_Good;
																				pMonitoredItemServer->SetStatusCode(OpcUa_BadIndexRangeNoData);
																				break;
																			}
																		}
																		break;
																	case OpcUa_VariantArrayType_Scalar:
																		{
																			uStatus=OpcUa_Good;
																			uServiceResult = OpcUa_BadOutOfRange; // OpcUa_BadIndexRangeInvalid;
																		}
																		break;
																	default:
																		uStatus=OpcUa_Good;
																		uServiceResult = OpcUa_BadOutOfRange; // OpcUa_BadIndexRangeInvalid;
																		break;
																	}
																}
															}
															else
																uStatus=OpcUa_BadInvalidArgument;
														}
													}
												}
												// sauvegarde du filtre 
												pMonitoredItemServer->SetMonitoringFilter((a_pItemsToCreate[ii].RequestedParameters.Filter));
												if (a_pItemsToCreate[ii].RequestedParameters.Filter.Body.EncodeableObject.Type)
												{
													if (a_pItemsToCreate[ii].RequestedParameters.Filter.Body.EncodeableObject.Type->TypeId == OpcUaId_DataChangeFilter) // DataChangeFilter
													{
														OpcUa_DataChangeFilter* pDataChangeFilter = (OpcUa_DataChangeFilter*)a_pItemsToCreate[ii].RequestedParameters.Filter.Body.EncodeableObject.Object;
														if (pDataChangeFilter)
														{
															pMonitoredItemServer->SetDataChangeFilter(pDataChangeFilter);
															// deadband is not supported on some particular data type. Below the list
															if ((pBase->GetNodeClass() == OpcUa_NodeClass_Variable) && (pDataChangeFilter->DeadbandType!=OpcUa_DeadbandType_None))
															{
																if ((((CUAVariable*)pBase)->GetDataType().Identifier.Numeric == OpcUaType_String) // STRING
																	|| (((CUAVariable*)pBase)->GetDataType().Identifier.Numeric == 1) // Boolean
																	|| (((CUAVariable*)pBase)->GetDataType().Identifier.Numeric == 14) // GUID
																	|| (((CUAVariable*)pBase)->GetDataType().Identifier.Numeric == 15)  // BYTESTRING
																	|| (((CUAVariable*)pBase)->GetDataType().Identifier.Numeric == 20)  // QualifiedName
																	|| (((CUAVariable*)pBase)->GetDataType().Identifier.Numeric == 21)) //localizedText
																{
																	uStatus = OpcUa_Good;
																	uServiceResult = OpcUa_Good;
																	pMonitoredItemServer->SetStatusCode(OpcUa_BadFilterNotAllowed);
																}
															}
															if ((pDataChangeFilter->DeadbandType == OpcUa_DeadbandType_Absolute) )
															{
																if (((CUAVariable*)pBase)->GetDataType().Identifier.Numeric == OpcUaType_String)
																{
																	uStatus = OpcUa_Good;
																	uServiceResult = OpcUa_Good;
																	pMonitoredItemServer->SetStatusCode(OpcUa_BadFilterNotAllowed);
																}
															}
														}
													}
													if (a_pItemsToCreate[ii].RequestedParameters.Filter.Body.EncodeableObject.Type->TypeId == OpcUaId_EventFilter) // EventFilter
													{
														OpcUa_EventFilter* pEventFilter=(OpcUa_EventFilter*)a_pItemsToCreate[ii].RequestedParameters.Filter.Body.EncodeableObject.Object;
														if (pEventFilter)
															pMonitoredItemServer->SetEventFilter(pEventFilter);
													}
												}

												// ajout a la liste complète
												if (pMonitoredItemServer->GetUABase())
												{
													if (pSubscription->AddMonitoredItem(pMonitoredItemServer)==OpcUa_Good)
													{
														// Ajout a la liste pour retransmission au client
														// Cette liste (aMonitoredItemBaseList) est une copie de la liste prise en charge par le serveur (m_pMonitoredItemList)
														// Cette copie est réalisé car la stack efface le contenu qu'elle transporte
														OpcUa_Mutex aMonitoredItemListMutex=pSubscription->GetMonitoredItemListMutex();
														OpcUa_Mutex_Lock(aMonitoredItemListMutex);
														aMonitoredItemBaseList.push_back(pMonitoredItemServer);
														OpcUa_Mutex_Unlock(aMonitoredItemListMutex);
													}
												}
												// If it's a UAVariable attached to a Vpi let's update the Sampling interval
												
												if (pBase->GetNodeClass() == OpcUa_NodeClass_Variable)
												{
													CUAVariable* pUAVariable = (CUAVariable*)pBase;
													CVpiTag* pSignal = (CVpiTag*)(pUAVariable->GetPData());
													if (pSignal)
													{
														CVpiDevice* pDevice = pSignal->GetDevice();
														if (pDevice)
														{
															// Update the value of the fastest sampling interval
															pSubscription->UpdateFastestSamplingInterval();
															OpcUa_Double fastestSamplingInterval = pSubscription->GetFastestSamplingInterval();
															pDevice->SetSamplingInterval(fastestSamplingInterval);
														}
													}
												}												
											}
											else
											{
												uStatus=OpcUa_Good;
												uServiceResult=OpcUa_Good;
												CMonitoredItemServer* pMonitoredItemServer=new CMonitoredItemServer();
												pMonitoredItemServer->SetStatusCode(OpcUa_BadNodeIdUnknown);
												OpcUa_Mutex aMonitoredItemListMutex=pSubscription->GetMonitoredItemListMutex();
												OpcUa_Mutex_Lock(aMonitoredItemListMutex);
												aMonitoredItemBaseList.push_back(pMonitoredItemServer);
												OpcUa_Mutex_Unlock(aMonitoredItemListMutex);
											}
										}
									}
									else
									{
										uStatus=OpcUa_Good;
										uServiceResult=OpcUa_Good;
										CMonitoredItemServer* pMonitoredItemServer=new CMonitoredItemServer();
										pMonitoredItemServer->SetStatusCode(OpcUa_BadAttributeIdInvalid);
										OpcUa_Mutex aMutex=pSubscription->GetMonitoredItemListMutex();
										OpcUa_Mutex_Lock(aMutex);
										aMonitoredItemBaseList.push_back(pMonitoredItemServer);
										OpcUa_Mutex_Unlock(aMutex);
									}
								}
								else
								{
									uStatus=OpcUa_Good;
									uServiceResult=OpcUa_Good;
									CMonitoredItemServer* pMonitoredItemServer=new CMonitoredItemServer();
									pMonitoredItemServer->SetStatusCode(uStatusPreChecked);
									OpcUa_Mutex aMutex=pSubscription->GetMonitoredItemListMutex();
									OpcUa_Mutex_Lock(aMutex);
									aMonitoredItemBaseList.push_back(pMonitoredItemServer);
									OpcUa_Mutex_Unlock(aMutex);
								}
								// release ressources
								if (pNumericRange)
									delete pNumericRange;
							}

							
							// mise en forme du résultat
							if (a_nNoOfItemsToCreate>0)
							{								
								(*a_pResults)=(OpcUa_MonitoredItemCreateResult*)OpcUa_Alloc(a_nNoOfItemsToCreate*sizeof(OpcUa_MonitoredItemCreateResult));						
								
								for (OpcUa_UInt32 iii=0;iii<aMonitoredItemBaseList.size();iii++)
								{
									OpcUa_MonitoredItemCreateResult_Initialize(&((*a_pResults)[iii]));
									CMonitoredItemServer* pMonitoredItemServer=aMonitoredItemBaseList.at(iii);				
									if (pMonitoredItemServer->GetUABase())
									{					
										((*a_pResults)[iii]).StatusCode = pMonitoredItemServer->GetStatusCode();
										((*a_pResults)[iii]).MonitoredItemId=(OpcUa_UInt32)pMonitoredItemServer->GetMonitoredItemId();
										((*a_pResults)[iii]).RevisedQueueSize=pMonitoredItemServer->GetQueueSize(); 
										((*a_pResults)[iii]).RevisedSamplingInterval=pMonitoredItemServer->GetSamplingInterval();
										OpcUa_ExtensionObject_Initialize((&((*a_pResults)[iii]).FilterResult));
										OpcUa_ExtensionObject* aExtObj=pMonitoredItemServer->GetMonitoringFilter();
										Copy(&aExtObj,&(((*a_pResults)[iii]).FilterResult)); // check Alloc of that 28-3-2014
									}
									else
									{
										((*a_pResults)[iii]).StatusCode = pMonitoredItemServer->GetStatusCode();
										delete pMonitoredItemServer;
										pMonitoredItemServer = OpcUa_Null;
									}
								}
							}
							aMonitoredItemBaseList.clear();

							(*a_pNoOfResults)=a_nNoOfItemsToCreate;
						}
						OpcUa_Mutex_Unlock(aSubscriptionListMutex);				
					}
					else
					{
						OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"FindSession failed\n");
						uServiceResult=uStatus;
					}
				}
				else
				{
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"GetMessageSecureChannelId failed 0x%05x\n",uStatus);
					uServiceResult=uStatus;
				}
			}
		}
	}
	// Update the response header.
	a_pResponseHeader->Timestamp     = OpcUa_DateTime_UtcNow();
	a_pResponseHeader->RequestHandle = a_pRequestHeader->RequestHandle;
	a_pResponseHeader->ServiceResult = uServiceResult;
	//OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"out:a_pResults:%x\n",a_pResults);
	return uStatus;
}

OpcUa_StatusCode Server_ModifyMonitoredItems(
	OpcUa_Endpoint                          a_hEndpoint, // in
	OpcUa_Handle                            a_hContext, // in 
	const OpcUa_RequestHeader*              a_pRequestHeader, // in
	OpcUa_UInt32                            a_nSubscriptionId, // in
	OpcUa_TimestampsToReturn                a_eTimestampsToReturn, // in
	OpcUa_Int32                             a_nNoOfItemsToModify, // in
	const OpcUa_MonitoredItemModifyRequest* a_pItemsToModify, // in
	OpcUa_ResponseHeader*                   a_pResponseHeader, // out
	OpcUa_Int32*                            a_pNoOfResults, // out
	OpcUa_MonitoredItemModifyResult**       a_pResults, // out
	OpcUa_Int32*                            a_pNoOfDiagnosticInfos, // out
	OpcUa_DiagnosticInfo**                  a_pDiagnosticInfos) // out
{
	CSessionServer* pSession = OpcUa_Null;
	OpcUa_UInt32 uSecureChannelId = 0;
	CServerApplication* pServer = 0;
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_StatusCode uServiceResult=OpcUa_Good;

	OpcUa_ReferenceParameter(a_pRequestHeader);
	OpcUa_ReferenceParameter(a_nSubscriptionId);
	OpcUa_ReferenceParameter(a_pNoOfDiagnosticInfos);
	OpcUa_ReferenceParameter(a_pDiagnosticInfos);
	OpcUa_ReferenceParameter(a_pResults);
	OpcUa_ReferenceParameter(a_pNoOfResults);
	OpcUa_ReferenceParameter(a_pResponseHeader);
	OpcUa_ReferenceParameter(a_pItemsToModify);
	OpcUa_ReferenceParameter(a_nNoOfItemsToModify);
	OpcUa_ReferenceParameter(a_eTimestampsToReturn);

	if (a_nNoOfItemsToModify>0)
	{
		//CUAInformationModel* pInformationModel = CServerApplication::m_pTheAddressSpace;
		// Get the server application object.
		uStatus = OpcUa_Endpoint_GetCallbackData(a_hEndpoint, (OpcUa_Void**)&pServer);
		if (uStatus==OpcUa_Good)
		{
			OpcUa_NodeId AuthenticationToken=a_pRequestHeader->AuthenticationToken;
			// Get the secure channel being used.
			uStatus = OpcUa_Endpoint_GetMessageSecureChannelId(a_hEndpoint, a_hContext, &uSecureChannelId);
			if (uStatus==OpcUa_Good)
			{
				// find the session.
				uStatus = pServer->FindSession(uSecureChannelId,&AuthenticationToken,&pSession);
				if (uStatus==OpcUa_Good)
				{
					CSubscriptionServer* pSubscription= OpcUa_Null;
					OpcUa_Mutex aSubscriptionListMutex = pSession->GetSubscriptionListMutex();
					OpcUa_Mutex_Lock(aSubscriptionListMutex);
					uStatus=pSession->FindSubscription(a_nSubscriptionId,&pSubscription);
					if ( (uStatus==OpcUa_Good) && (pSubscription))
					{
						pSubscription->SetLastTransmissionTime(OpcUa_DateTime_UtcNow());
						*a_pNoOfResults=a_nNoOfItemsToModify;// The length has been tested previously
						*a_pResults=(OpcUa_MonitoredItemModifyResult*)OpcUa_Alloc(sizeof(OpcUa_MonitoredItemModifyResult)*a_nNoOfItemsToModify);
						if (*a_pResults)
						{
							OpcUa_Mutex aMonitoredItemListMutex = pSubscription->GetMonitoredItemListMutex();
							OpcUa_Mutex_Lock(aMonitoredItemListMutex);
							for (OpcUa_Int32 ii = 0; ii < a_nNoOfItemsToModify; ii++)
							{
								OpcUa_MonitoredItemModifyResult_Initialize(&((*a_pResults)[ii]));
								(*a_pResults)[ii].StatusCode = OpcUa_Good; // default Value
								CMonitoredItemServer* pItem = OpcUa_Null;
								uServiceResult = pSubscription->FindMonitoredItemById(a_pItemsToModify[ii].MonitoredItemId, &pItem);
								if (uServiceResult == OpcUa_Good)
								{
									if ((a_eTimestampsToReturn != OpcUa_TimestampsToReturn_Source)
										&& (a_eTimestampsToReturn != OpcUa_TimestampsToReturn_Server)
										&& (a_eTimestampsToReturn != OpcUa_TimestampsToReturn_Both)
										&& (a_eTimestampsToReturn != OpcUa_TimestampsToReturn_Neither))
										uServiceResult = OpcUa_BadTimestampsToReturnInvalid;
									else
									{
										// Mise a jour du TimestampsToReturn
										pItem->SetTimestampsToReturn(a_eTimestampsToReturn);
									}
									// Mise a jour du clientHandle
									pItem->SetClientHandle((OpcUa_UInt32)a_pItemsToModify[ii].RequestedParameters.ClientHandle);
									// Mise a jour de DiscardOldest
									pItem->DiscardOldest(a_pItemsToModify[ii].RequestedParameters.DiscardOldest);
									// Mise ajour du filtre
									if (a_pItemsToModify[ii].RequestedParameters.Filter.Body.EncodeableObject.Type)
									{
										if (a_pItemsToModify[ii].RequestedParameters.Filter.Body.EncodeableObject.Type->TypeId == OpcUaId_DataChangeFilter) // DataChangeFilter
										{
											OpcUa_DataChangeFilter* pDataChangeFilter = (OpcUa_DataChangeFilter*)a_pItemsToModify[ii].RequestedParameters.Filter.Body.EncodeableObject.Object;
											if (pDataChangeFilter)
											{
												if ((pDataChangeFilter->DeadbandType == OpcUa_DeadbandType_Absolute))
												{
													if (pItem->GetAttributeId() == OpcUa_Attributes_Value)
													{
														(*a_pResults)[ii].StatusCode = OpcUa_Good;
														if (pItem->GetValue()->Value.Datatype == OpcUaType_String)
														{
															uStatus = OpcUa_Good;
															uServiceResult = OpcUa_Good;
															(*a_pResults)[ii].StatusCode = OpcUa_BadFilterNotAllowed;
														}
													}
													else
													{
														uStatus = OpcUa_Good;
														uServiceResult = OpcUa_Good;
														(*a_pResults)[ii].StatusCode = OpcUa_BadFilterNotAllowed;
													}
												}
												if ((*a_pResults)[ii].StatusCode==OpcUa_Good)
													pItem->SetDataChangeFilter(pDataChangeFilter);
											}
										}
										else
										{
											if (a_pItemsToModify[ii].RequestedParameters.Filter.Body.EncodeableObject.Type->TypeId == OpcUaId_EventFilter)
											{
												OpcUa_EventFilter* pEventFilter = (OpcUa_EventFilter*)a_pItemsToModify[ii].RequestedParameters.Filter.Body.EncodeableObject.Object;
												if (pEventFilter)
													pItem->SetEventFilter(pEventFilter);
											}
										}
									}
									// Mise a jour de la QueueSize
									if (a_pItemsToModify[ii].RequestedParameters.QueueSize == 0)
										pItem->SetQueueSize(1000);
									else
									{
										if (a_pItemsToModify[ii].RequestedParameters.QueueSize > 0xAAAAAAAA)
											pItem->SetQueueSize(0xAAAAAAAA);
										else
											pItem->SetQueueSize(a_pItemsToModify[ii].RequestedParameters.QueueSize);
									}
									// Mise a jour du SamplingInterval
									pItem->SetSamplingInterval(a_pItemsToModify[ii].RequestedParameters.SamplingInterval);
									// If it's a UAVariable attached to a Vpi let's update the Sampling interval
									CUABase* pBase = pItem->GetUABase();
									if (pBase)
									{
										if (pBase->GetNodeClass() == OpcUa_NodeClass_Variable)
										{
											CUAVariable* pUAVariable = (CUAVariable*)pBase;
											CVpiTag* pSignal = (CVpiTag*)(pUAVariable->GetPData());
											if (pSignal)
											{
												CVpiDevice* pDevice = pSignal->GetDevice();
												if (pDevice)
												{
													// Update the value of the fastest sampling interval
													pSubscription->UpdateFastestSamplingInterval();
													OpcUa_Double fastestSamplingInterval = pSubscription->GetFastestSamplingInterval();
													pDevice->SetSamplingInterval(fastestSamplingInterval);
												}
											}
										}
									}
									// Update output result
									//(*a_pResults)[ii].FilterResult;
									(*a_pResults)[ii].RevisedQueueSize = pItem->GetQueueSize();
									(*a_pResults)[ii].RevisedSamplingInterval = pItem->GetSamplingInterval();
									//(*a_pResults)[ii].StatusCode = OpcUa_Good;
									// will delete the pending notification. This will be implemented that way to please the CTT 
									//pSubscription->DeleteDataChangeNotfication(pItem->GetClientHandle()); // just removed temporarly to please the CTT 
								}
								else
								{
									//
									uServiceResult = OpcUa_Good;
									(*a_pResults)[ii].StatusCode = OpcUa_BadMonitoredItemIdInvalid;
								}
							}
							// Update the value of the fastest sampling interval
							//pSubscription->UpdateFastestSamplingInterval();
							//OpcUa_Double fastestSamplingInterval = pSubscription->GetFastestSamplingInterval();
							//CVPIScheduler* pVpiScheduler = pInformationModel->GetVPIScheduler();
							//pVpiScheduler->SetSamplingInterval(fastestSamplingInterval);
							*a_pNoOfResults = a_nNoOfItemsToModify;
							OpcUa_Mutex_Unlock(aMonitoredItemListMutex);
						}
						else
							uServiceResult = OpcUa_BadOutOfMemory;
					}
					OpcUa_Mutex_Unlock(aSubscriptionListMutex);
				}
			}
		}
	}
	else
		uServiceResult=OpcUa_BadNothingToDo;
	// Mise en forme des OpcUa_DiagnosticInfo
	*a_pNoOfDiagnosticInfos = 0;
	*a_pDiagnosticInfos = OpcUa_Null;
	// Update the response header.
	a_pResponseHeader->Timestamp     = OpcUa_DateTime_UtcNow();
	a_pResponseHeader->RequestHandle = a_pRequestHeader->RequestHandle;
	a_pResponseHeader->ServiceResult = uServiceResult;

	return uStatus;
}

OpcUa_StatusCode Server_DeleteMonitoredItems(
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
	OpcUa_DiagnosticInfo**     a_pDiagnosticInfos)
{
	OpcUa_UInt32 uSecureChannelId = 0;
	CServerApplication* pServerApplication = 0;
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_StatusCode uServiceResult=OpcUa_Good;
	CSessionServer* pSession = OpcUa_Null;
	*a_pNoOfResults=a_nNoOfMonitoredItemIds;
	OpcUa_StatusCode*         pMyResults=OpcUa_Null;
	// Verouillage de la cache du serveur
	CUAInformationModel* pInformationModel=CServerApplication::m_pTheAddressSpace;

	*a_pNoOfResults = 0;
	OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_INFO, "Server_DeleteMonitoredItems receive from the client\n");
	if (a_nNoOfMonitoredItemIds>0)
	{
		pMyResults=(OpcUa_StatusCode*)OpcUa_Alloc(a_nNoOfMonitoredItemIds*sizeof(OpcUa_StatusCode));
		ZeroMemory(pMyResults, a_nNoOfMonitoredItemIds*sizeof(OpcUa_StatusCode));
		uStatus = OpcUa_Endpoint_GetCallbackData(a_hEndpoint, (OpcUa_Void**)&pServerApplication);
		if (uStatus==OpcUa_Good)
		{
			OpcUa_NodeId AuthenticationToken=a_pRequestHeader->AuthenticationToken;
			// Get the secure channel being used.
			uStatus = OpcUa_Endpoint_GetMessageSecureChannelId(a_hEndpoint, a_hContext, &uSecureChannelId);
			if (uStatus==OpcUa_Good)
			{
				// find the session.
				uStatus = pServerApplication->FindSession(uSecureChannelId, &AuthenticationToken, &pSession);
				if (uStatus==OpcUa_Good)
				{
					OpcUa_Semaphore_Post(pSession->m_SessionTimeoutSem,1);
					OpcUa_Mutex aSubscriptionListMutex = pSession->GetSubscriptionListMutex();
					OpcUa_Mutex_Lock(aSubscriptionListMutex);
					CSubscriptionServer* pSubscription= OpcUa_Null;
					uStatus=pSession->FindSubscription(a_nSubscriptionId,&pSubscription);
					if ( (uStatus==OpcUa_Good) && (pSubscription))
					{
						pSubscription->SetLastTransmissionTime(OpcUa_DateTime_UtcNow());
						*a_pNoOfResults=a_nNoOfMonitoredItemIds;
						// indicate to the subscription that the client is is alive
						pSubscription->ResetLifeTimeCountCounter();
						for (OpcUa_Int32 ii = 0; ii < a_nNoOfMonitoredItemIds; ii++)
						{
							OpcUa_Double removedSamplingInterval = 0.0;
							if (pInformationModel->IsEnabledServerDiagnosticsDefaultValue())
							{
								// Save the sampling interval of the removed monitoredItem
								CMonitoredItemServer* pItem = OpcUa_Null;
								OpcUa_Mutex monitoredItemListMutex = pSubscription->GetMonitoredItemListMutex();
								OpcUa_Mutex_Lock(monitoredItemListMutex);								
								if (pSubscription->FindMonitoredItemById(a_pMonitoredItemIds[ii], &pItem)==OpcUa_Good)
									removedSamplingInterval = pItem->GetSamplingInterval();
								OpcUa_Mutex_Unlock(monitoredItemListMutex);
							}
							// Delete monitoredItem
							if (uStatus == OpcUa_Good)
							{
								pMyResults[ii] = pSubscription->DeleteMonitoredItemById(a_pMonitoredItemIds[ii]);
							}
							// Update DiagnosticsSampling
							if (pInformationModel->IsEnabledServerDiagnosticsDefaultValue())
							{
								OpcUa_SamplingIntervalDiagnosticsDataType* pSamplingIntervalDiagnostic = OpcUa_Null;
								if (pInformationModel->IsSamplingIntervalDiagnosticsExist(removedSamplingInterval, &pSamplingIntervalDiagnostic))
								{
									uStatus = pInformationModel->RemoveInSamplingIntervalDiagnosticsArray(pSamplingIntervalDiagnostic);
								}
							}
						}
						// Update the value of the fastest sampling interval
						pSubscription->UpdateFastestSamplingInterval();

					}
					else
					{
						uServiceResult = OpcUa_BadSubscriptionIdInvalid;
						OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "DeleteMonitoredItems>Subscription %u does not exist.\n", a_nSubscriptionId);
					}
					OpcUa_Mutex_Unlock(aSubscriptionListMutex);
				}
				else
				{
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Session does not exist.\n");
				}
			}
		}
		// Mise en forme du Results. Dans ce cas il s'agit d'un pointeur sur des OpcUa_StatusCode
		(*a_pResults)=(OpcUa_StatusCode*)OpcUa_Alloc(a_nNoOfMonitoredItemIds*sizeof(OpcUa_StatusCode));
		OpcUa_MemCpy((*a_pResults),
			(a_nNoOfMonitoredItemIds*sizeof(OpcUa_StatusCode)),
			pMyResults,
			(a_nNoOfMonitoredItemIds*sizeof(OpcUa_StatusCode)));
	}
	else
		uServiceResult=OpcUa_BadNothingToDo;
	if (pMyResults)
		OpcUa_Free(pMyResults);
	// Mise en forme des OpcUa_DiagnosticInfo
	*a_pNoOfDiagnosticInfos=0;
	*a_pDiagnosticInfos = OpcUa_Null; 
	// Update the response header.
	a_pResponseHeader->Timestamp     = OpcUa_DateTime_UtcNow();
	a_pResponseHeader->RequestHandle = a_pRequestHeader->RequestHandle;
	a_pResponseHeader->ServiceResult = uServiceResult;
	return uStatus;
}

OpcUa_StatusCode Server_SetMonitoringMode(
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
	OpcUa_DiagnosticInfo**     a_pDiagnosticInfos)
{
	OpcUa_ReferenceParameter(a_nNoOfMonitoredItemIds);
	OpcUa_ReferenceParameter(a_pMonitoredItemIds);
	OpcUa_ReferenceParameter(a_pResponseHeader);
	OpcUa_ReferenceParameter(a_pNoOfResults);
	OpcUa_ReferenceParameter(a_pResults);
	OpcUa_ReferenceParameter(a_pNoOfDiagnosticInfos);
	OpcUa_ReferenceParameter(a_pDiagnosticInfos);
	OpcUa_UInt32 uSecureChannelId = 0;
	CServerApplication* pServer = 0;
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_StatusCode uServiceResult = OpcUa_Good;
	CSessionServer* pSession=NULL;
	CSubscriptionServer* pSubscription;
	// Get the server application object.
	uStatus = OpcUa_Endpoint_GetCallbackData(a_hEndpoint, (OpcUa_Void**)&pServer);
	if (uStatus==OpcUa_Good)
	{
		// Get the secure channel being used.
		uStatus = OpcUa_Endpoint_GetMessageSecureChannelId(a_hEndpoint, a_hContext, &uSecureChannelId);
		if (uStatus==OpcUa_Good)
		{
			uStatus = pServer->FindSession(uSecureChannelId,&a_pRequestHeader->AuthenticationToken,&pSession);
			if (uStatus==OpcUa_Good)
			{
				OpcUa_Semaphore_Post(pSession->m_SessionTimeoutSem,1);
				OpcUa_Mutex aSubscriptionListMutex = pSession->GetSubscriptionListMutex();
				OpcUa_Mutex_Lock(aSubscriptionListMutex);
				uServiceResult=pSession->FindSubscription(a_nSubscriptionId,&pSubscription);
				if (uServiceResult==OpcUa_Good)
				{
					pSubscription->SetMonitoringMode(a_eMonitoringMode);
					CSubscriptionDiagnosticsDataType* pSubscriptionDiagnosticsDataType= pSubscription->GetSubscriptionDiagnosticsDataType();
					if (pSubscriptionDiagnosticsDataType)
					{
						if (a_eMonitoringMode == OpcUa_MonitoringMode_Disabled)
						{
							OpcUa_UInt32 uiDisabledMonitoredItemCount=pSubscriptionDiagnosticsDataType->GetDisabledMonitoredItemCount();
							pSubscriptionDiagnosticsDataType->SetDisabledMonitoredItemCount(uiDisabledMonitoredItemCount + a_nNoOfMonitoredItemIds);
						}
					}
					// On met a jour maitenant le monitoring mode des Monitoreditems
					*a_pNoOfResults=a_nNoOfMonitoredItemIds;
					if (a_nNoOfMonitoredItemIds>0)
					{
						*a_pResults=(OpcUa_StatusCode*)OpcUa_Alloc(a_nNoOfMonitoredItemIds*sizeof(OpcUa_StatusCode));
						OpcUa_Mutex aMonitoredItemListMutex=pSubscription->GetMonitoredItemListMutex();
						OpcUa_Mutex_Lock(aMonitoredItemListMutex);
						for (OpcUa_Int32 ii=0;ii<a_nNoOfMonitoredItemIds;ii++)
						{
							CMonitoredItemServer* pItem=OpcUa_Null;
							uStatus=pSubscription->FindMonitoredItemById(a_pMonitoredItemIds[ii],&pItem);
							if (uStatus==OpcUa_Good)
							{
								pItem->SetMonitoringMode(a_eMonitoringMode);
								(*a_pResults)[ii]=OpcUa_Good;
								if (a_eMonitoringMode == OpcUa_MonitoringMode_Reporting)
									pSubscription->WakeupUpdateSubscriptionThread();
								// will delete the pending notification. This will be implemented that way to please the CTT 
								//pSubscription->DeleteDataChangeNotfication(pItem);
							}
							else
								(*a_pResults)[ii]=OpcUa_BadMonitoredItemIdInvalid;	
						}
						OpcUa_Mutex_Unlock(aMonitoredItemListMutex);
					}
					else
						uServiceResult=OpcUa_BadNothingToDo;
				}
				OpcUa_Mutex_Unlock(aSubscriptionListMutex);
			}
		}
	}
	// Update the response header.
	a_pResponseHeader->Timestamp     = OpcUa_DateTime_UtcNow();
	a_pResponseHeader->RequestHandle = a_pRequestHeader->RequestHandle;
	a_pResponseHeader->ServiceResult = uServiceResult;
	return OpcUa_Good;
}

OpcUa_StatusCode Server_SetTriggering(
	OpcUa_Endpoint             a_hEndpoint, // in
	OpcUa_Handle               a_hContext, // in
	const OpcUa_RequestHeader* a_pRequestHeader, // in
	OpcUa_UInt32               a_nSubscriptionId, // in
	OpcUa_UInt32               a_nTriggeringItemId, // in 
	OpcUa_Int32                a_nNoOfLinksToAdd, // in
	const OpcUa_UInt32*        a_pLinksToAdd, // in
	OpcUa_Int32                a_nNoOfLinksToRemove, // in
	const OpcUa_UInt32*        a_pLinksToRemove, // in
	OpcUa_ResponseHeader*      a_pResponseHeader,
	OpcUa_Int32*               a_pNoOfAddResults,
	OpcUa_StatusCode**         a_pAddResults,
	OpcUa_Int32*               a_pNoOfAddDiagnosticInfos,
	OpcUa_DiagnosticInfo**     a_pAddDiagnosticInfos,
	OpcUa_Int32*               a_pNoOfRemoveResults,
	OpcUa_StatusCode**         a_pRemoveResults,
	OpcUa_Int32*               a_pNoOfRemoveDiagnosticInfos,
	OpcUa_DiagnosticInfo**     a_pRemoveDiagnosticInfos)
{
	OpcUa_ReferenceParameter(a_pNoOfRemoveResults);
	OpcUa_ReferenceParameter(a_pRemoveResults);
	OpcUa_ReferenceParameter(a_pNoOfRemoveDiagnosticInfos);
	OpcUa_ReferenceParameter(a_pRemoveDiagnosticInfos);
	OpcUa_ReferenceParameter(a_nTriggeringItemId);
	OpcUa_ReferenceParameter(a_pLinksToAdd);
	OpcUa_ReferenceParameter(a_pLinksToRemove);
	OpcUa_ReferenceParameter(a_pResponseHeader);
	OpcUa_ReferenceParameter(a_pNoOfAddResults);
	OpcUa_ReferenceParameter(a_pAddResults);
	OpcUa_ReferenceParameter(a_pNoOfAddDiagnosticInfos);
	OpcUa_ReferenceParameter(a_pAddDiagnosticInfos);
	OpcUa_ReferenceParameter(a_nNoOfLinksToRemove);
	OpcUa_ReferenceParameter(a_nNoOfLinksToAdd);
	OpcUa_UInt32 uSecureChannelId = 0;
	CServerApplication* pServer = 0;
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_StatusCode uServiceResult = OpcUa_Good;
	//CSessionServer* pSession=NULL;
	CSubscriptionServer* pSubscription;
	// Get the server application object.
	uStatus = OpcUa_Endpoint_GetCallbackData(a_hEndpoint, (OpcUa_Void**)&pServer);
	if (uStatus==OpcUa_Good)
	{
		// Get the secure channel being used.
		uStatus = OpcUa_Endpoint_GetMessageSecureChannelId(a_hEndpoint, a_hContext, &uSecureChannelId);
		if (uStatus==OpcUa_Good)
		{
			CSessionServer* pSession = OpcUa_Null;
			uStatus=pServer->FindSession(uSecureChannelId,&a_pRequestHeader->AuthenticationToken,&pSession);
			if (uStatus==OpcUa_Good)
			{
				OpcUa_Mutex aSubscriptionListMutex = pSession->GetSubscriptionListMutex();
				OpcUa_Mutex_Lock(aSubscriptionListMutex);
				uStatus=pSession->FindSubscription(a_nSubscriptionId,&pSubscription);
				if (uStatus == OpcUa_Good)
				{
					uServiceResult = OpcUa_BadMonitoredItemIdInvalid;
					OpcUa_Mutex aMonitoredItemListMutex = pSubscription->GetMonitoredItemListMutex();
					OpcUa_Mutex_Lock(aMonitoredItemListMutex);
					CMonitoredItemServerList* pMonitoredItemServerList = pSubscription->GetMonitoredItemList(); // Server_SetTriggering
					for (OpcUa_UInt32 i = 0; i < pMonitoredItemServerList->size(); i++)
					{
						CMonitoredItemServer* pMonitoredItemServer = pMonitoredItemServerList->at(i);
						if (pMonitoredItemServer->GetMonitoredItemId() == a_nTriggeringItemId)
						{
							uServiceResult = OpcUa_Good;
							if ( (a_nNoOfLinksToRemove==0) && (a_nNoOfLinksToAdd == 0))
								uServiceResult = OpcUa_BadNothingToDo;
							else
							{
								if (a_nNoOfLinksToRemove > 0)
								{
									(*a_pNoOfRemoveResults) = a_nNoOfLinksToRemove;
									(*a_pRemoveResults) = (OpcUa_StatusCode*)OpcUa_Alloc(sizeof(OpcUa_StatusCode)*a_nNoOfLinksToRemove);
									for (OpcUa_Int32 ii = 0; ii < a_nNoOfLinksToRemove; ii++)
									{
										CMonitoredItemServer* pTriggeredMonitoredItemServer = OpcUa_Null;
										(*a_pRemoveResults)[ii] = pSubscription->FindMonitoredItemById(a_pLinksToRemove[ii], &pTriggeredMonitoredItemServer);
										if ((*a_pRemoveResults)[ii] == OpcUa_Good)
											(*a_pRemoveResults)[ii] = pMonitoredItemServer->RemoveTriggeredItemId(pTriggeredMonitoredItemServer);
										//else
										//	uServiceResult = OpcUa_BadMonitoredItemIdInvalid;
									}
								}
								if (a_nNoOfLinksToAdd > 0)
								{
									(*a_pNoOfAddResults) = a_nNoOfLinksToAdd;
									(*a_pAddResults) = (OpcUa_StatusCode*)OpcUa_Alloc(sizeof(OpcUa_StatusCode)*a_nNoOfLinksToAdd);
									for (OpcUa_Int32 ii = 0; ii < a_nNoOfLinksToAdd; ii++)
									{
										CMonitoredItemServer* pTriggeredMonitoredItemServer = OpcUa_Null;
										(*a_pAddResults)[ii] = pSubscription->FindMonitoredItemById(a_pLinksToAdd[ii], &pTriggeredMonitoredItemServer);
										if ((*a_pAddResults)[ii] == OpcUa_Good)
											pMonitoredItemServer->AddTriggeredItemId(pTriggeredMonitoredItemServer);
										//else
										//	uServiceResult = OpcUa_BadMonitoredItemIdInvalid;
									}
								}
							}
							break;
						}
					}
					OpcUa_Mutex_Unlock(aMonitoredItemListMutex);
				}
				else
					uServiceResult = OpcUa_BadSubscriptionIdInvalid;
				OpcUa_Mutex_Unlock(aSubscriptionListMutex);
			}
		}
	}
	// Update the response header.
	a_pResponseHeader->Timestamp = OpcUa_DateTime_UtcNow();
	a_pResponseHeader->RequestHandle = a_pRequestHeader->RequestHandle;
	a_pResponseHeader->ServiceResult = uServiceResult;
	return uStatus;
}


OpcUa_StatusCode Server_TranslateBrowsePathsToNodeIds(
	OpcUa_Endpoint             a_hEndpoint, // in
	OpcUa_Handle               a_hContext, // in 
	const OpcUa_RequestHeader* a_pRequestHeader,//in 
	OpcUa_Int32                a_nNoOfBrowsePaths, // in
	const OpcUa_BrowsePath*    a_pBrowsePaths, // in
	OpcUa_ResponseHeader*      a_pResponseHeader, // out
	OpcUa_Int32*               a_pNoOfResults, // out
	OpcUa_BrowsePathResult**   a_pResults, // out
	OpcUa_Int32*               a_pNoOfDiagnosticInfos, // out
	OpcUa_DiagnosticInfo**     a_pDiagnosticInfos) // out
{
	OpcUa_UInt32 uSecureChannelId = 0;
	CServerApplication* pServer = 0;
	CSessionServer* pSession=OpcUa_Null;

	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_StatusCode uServiceResult=OpcUa_Good;
	// Get the server application object.
	uStatus = OpcUa_Endpoint_GetCallbackData(a_hEndpoint, (OpcUa_Void**)&pServer);
	if (uStatus!=OpcUa_Good)
		return uStatus;
	// Validate the request


	if ((a_pBrowsePaths) && (a_nNoOfBrowsePaths > 0))
	{
		// Get the secure channel being used.
		uStatus = OpcUa_Endpoint_GetMessageSecureChannelId(a_hEndpoint, a_hContext, &uSecureChannelId);
		if (uStatus == OpcUa_Good)
		{
			OpcUa_NodeId  aAuthenticationToken;
			OpcUa_NodeId_Initialize(&aAuthenticationToken);
			aAuthenticationToken = a_pRequestHeader->AuthenticationToken;
			uStatus = pServer->FindSession(uSecureChannelId, &aAuthenticationToken, &pSession);
			if (uStatus == OpcUa_Good)
			{
				CUAInformationModel* pInformationModel = CServerApplication::m_pTheAddressSpace;
				// Recherche de resultat
				// on recherche quoi ? on cherche des TargetId
				OpcUa_BrowsePathResult* pResults = *a_pResults;
				pResults = (OpcUa_BrowsePathResult*)OpcUa_Alloc(sizeof(OpcUa_BrowsePathResult)*(a_nNoOfBrowsePaths));
				if (pResults)
				{
					(*a_pNoOfResults) = a_nNoOfBrowsePaths; // d'après la spec et le CTT. Il doit y avoir autant de reponse qu'il y a de demande.
					for (OpcUa_Int32 ii = 0; ii < a_nNoOfBrowsePaths; ii++)
					{
						// Allocation des objets contenant le résultat provisoire.
						//(pResults[ii]) = (OpcUa_BrowsePathResult*)OpcUa_Alloc(sizeof(OpcUa_BrowsePathResult));
						OpcUa_BrowsePathResult_Initialize(&pResults[ii]);
						// verification que l'on a a faire a une nodeId dont le format est valide.
						uStatus = IsNodeIdValid(a_pBrowsePaths[ii].StartingNode);
						if (uStatus == OpcUa_Good)
						{
							CUABase* pStartingNodeBase = NULL;
							uStatus = pInformationModel->GetNodeIdFromDictionnary(a_pBrowsePaths[ii].StartingNode, &pStartingNodeBase);
							if (uStatus != OpcUa_Good)
							{
								pResults[ii].StatusCode = OpcUa_BadNodeIdUnknown;
								uServiceResult = OpcUa_Good;
								uStatus = OpcUa_Good;
							}
							else
							{
								// pStartingNodeBase contient le noeud de départ (StartingNode)
								// 
								std::vector<OpcUa_ExpandedNodeId*> aExandedNodeIdList;
								OpcUa_ExpandedNodeId aExpandedNodeId;
								OpcUa_ExpandedNodeId_Initialize(&aExpandedNodeId);
								OpcUa_UInt16 uiSucceeded = 0;

								uServiceResult = pInformationModel->TranslateRelativePathToNodeId(pStartingNodeBase, a_pBrowsePaths[ii].RelativePath, &uiSucceeded, &aExandedNodeIdList);
								if (aExandedNodeIdList.size() > 0)
								{
									uServiceResult = OpcUa_Good;
									OpcUa_UInt32 iSize = aExandedNodeIdList.size();
									(pResults[ii]).Targets = (OpcUa_BrowsePathTarget*)OpcUa_Alloc(iSize*sizeof(OpcUa_BrowsePathTarget));
									if ((pResults[ii]).Targets)
									{
										(pResults[ii]).NoOfTargets = iSize;
										for (OpcUa_UInt32 iIndex = 0; iIndex < iSize; iIndex++)
										{
											OpcUa_BrowsePathTarget_Initialize(&((pResults[ii]).Targets[iIndex]));
											OpcUa_ExpandedNodeId* pExpandedNodeId = aExandedNodeIdList.at(iIndex);
											if (pExpandedNodeId)
											{
												OpcUa_ExpandedNodeId_CopyTo(pExpandedNodeId, &((pResults[ii]).Targets[iIndex].TargetId));
												OpcUa_ExpandedNodeId_Clear(pExpandedNodeId);
												OpcUa_Free(pExpandedNodeId);
											}
											(pResults[ii]).Targets[iIndex].RemainingPathIndex = 0xffffffff;
											(pResults[ii]).StatusCode = uServiceResult;
										}
									}
									else
									{
										(pResults[ii]).NoOfTargets = 0;
										(pResults[ii]).Targets = OpcUa_Null;
										(pResults[ii]).StatusCode = uServiceResult;
										uServiceResult = OpcUa_Good;
										uStatus = OpcUa_Good;
									}
								}
								else
								{
									(pResults[ii]).NoOfTargets = 0;
									(pResults[ii]).Targets = OpcUa_Null;
									(pResults[ii]).StatusCode = uServiceResult;
									uServiceResult = OpcUa_Good;
									uStatus = OpcUa_Good;
								}
							}
						}
						else
						{
							uStatus = OpcUa_Good;
							uServiceResult = OpcUa_Good;//OpcUa_BadNodeIdInvalid
							(pResults[ii]).StatusCode = OpcUa_BadNodeIdInvalid;
						}
					} // fin for (int ii=0;ii<a_nNoOfBrowsePaths;ii++)
					*a_pResults = pResults;
					//fin du transfert des données
				}
				else
					uStatus = OpcUa_BadOutOfMemory;
			}
			else
			{
				uServiceResult = OpcUa_BadSecurityChecksFailed;
			}
		}
	}
	else
	{
		uServiceResult = OpcUa_BadNothingToDo;
	}
	a_pResponseHeader->Timestamp= OpcUa_DateTime_UtcNow();
	a_pResponseHeader->RequestHandle = a_pRequestHeader->RequestHandle;
	a_pResponseHeader->ServiceResult = uServiceResult;
	a_pNoOfDiagnosticInfos=OpcUa_Null;
	*a_pDiagnosticInfos=OpcUa_Null;
	return uStatus;
 
}


OpcUa_StatusCode Server_RegisterNodes(
	OpcUa_Endpoint             a_hEndpoint,
	OpcUa_Handle               a_hContext,
	const OpcUa_RequestHeader* a_pRequestHeader,
	OpcUa_Int32                a_nNoOfNodesToRegister,
	const OpcUa_NodeId*        a_pNodesToRegister,
	OpcUa_ResponseHeader*      a_pResponseHeader,
	OpcUa_Int32*               a_pNoOfRegisteredNodeIds,
	OpcUa_NodeId**             a_pRegisteredNodeIds)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_UInt32 uSecureChannelId = 0;
	CServerApplication* pServerApplication = 0;
	//OpcUa_Endpoint_SecurityPolicyConfiguration tSecurityPolicy;

	OpcUa_ReturnErrorIfArgumentNull(a_hEndpoint);
	OpcUa_ReturnErrorIfArgumentNull(a_hContext);
	OpcUa_ReturnErrorIfArgumentNull(a_pRequestHeader);
	OpcUa_ReturnErrorIfArgumentNull(a_pResponseHeader);

	// Get the server application object.
	uStatus = OpcUa_Endpoint_GetCallbackData(a_hEndpoint, (OpcUa_Void**)&pServerApplication);
	if (uStatus==OpcUa_Good)
	{

		// Get the secure channel being used.
		uStatus = OpcUa_Endpoint_GetMessageSecureChannelId(a_hEndpoint, a_hContext, &uSecureChannelId);
		if (uStatus==OpcUa_Good)
		{
			// Get security policy.
			//uStatus = OpcUa_Endpoint_GetMessageSecureChannelSecurityPolicy(a_hEndpoint, a_hContext, &tSecurityPolicy);
			//if (uStatus==OpcUa_Good)
			{
				uStatus=pServerApplication->RegisterNodes(uSecureChannelId,a_pRequestHeader,
														  a_nNoOfNodesToRegister,
														  a_pNodesToRegister,
														  a_pResponseHeader,
														  a_pNoOfRegisteredNodeIds,
														  a_pRegisteredNodeIds);
			}
		}
	}
	return uStatus;
}
OpcUa_StatusCode Server_UnregisterNodes(
	OpcUa_Endpoint             a_hEndpoint,
	OpcUa_Handle               a_hContext,
	const OpcUa_RequestHeader* a_pRequestHeader,
	OpcUa_Int32                a_nNoOfNodesToUnregister,
	const OpcUa_NodeId*        a_pNodesToUnregister,
	OpcUa_ResponseHeader*      a_pResponseHeader)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_UInt32 uSecureChannelId = 0;
	CServerApplication* pServerApplication = 0;
	//OpcUa_Endpoint_SecurityPolicyConfiguration tSecurityPolicy;

	OpcUa_ReturnErrorIfArgumentNull(a_hEndpoint);
	OpcUa_ReturnErrorIfArgumentNull(a_hContext);
	OpcUa_ReturnErrorIfArgumentNull(a_pRequestHeader);
	OpcUa_ReturnErrorIfArgumentNull(a_pResponseHeader);

	// Get the server application object.
	uStatus = OpcUa_Endpoint_GetCallbackData(a_hEndpoint, (OpcUa_Void**)&pServerApplication);
	if (uStatus==OpcUa_Good)
	{

		// Get the secure channel being used.
		uStatus = OpcUa_Endpoint_GetMessageSecureChannelId(a_hEndpoint, a_hContext, &uSecureChannelId);
		if (uStatus==OpcUa_Good)
		{
			// Get security policy.
			//uStatus = OpcUa_Endpoint_GetMessageSecureChannelSecurityPolicy(a_hEndpoint, a_hContext, &tSecurityPolicy);
			//if (uStatus==OpcUa_Good)
			{
				uStatus=pServerApplication->UnregisterNodes(uSecureChannelId,a_pRequestHeader,
														  a_nNoOfNodesToUnregister,
														  a_pNodesToUnregister,
														  a_pResponseHeader);
			}
		}
	}
	return uStatus;
}
/// <summary>
/// Implemntation of the OPC UA Call service.
/// </summary>
/// <param name="a_hEndpoint">The a_h endpoint.</param>
/// <param name="a_hContext">The a_h context.</param>
/// <param name="a_ppRequest">The a_pp request.</param>
/// <param name="a_pRequestType">Type of the a_p request.</param>
/// <returns></returns>
OpcUa_StatusCode Server_BeginCall(
	OpcUa_Endpoint        a_hEndpoint,
	OpcUa_Handle          a_hContext,
	OpcUa_Void**          a_ppRequest,
	OpcUa_EncodeableType* a_pRequestType)
{
	CSessionServer* pSessionServer = 0;
	OpcUa_CallRequest* pCallRequest = OpcUa_Null;
	CServerApplication* pServerApplication = 0;

	OpcUa_StatusCode uStatus = OpcUa_Good;

	if (!a_hEndpoint)
		uStatus = OpcUa_BadInvalidArgument;
	if (!a_hContext)
		uStatus = OpcUa_BadInvalidArgument;
	if (!a_ppRequest)
		uStatus = OpcUa_BadInvalidArgument;
	if (*a_ppRequest == NULL)
		uStatus = OpcUa_BadInvalidArgument;
	if (!a_pRequestType)
		uStatus = OpcUa_BadInvalidArgument;
	else
	{
		if (a_pRequestType->TypeId != OpcUaId_CallRequest)
			uStatus = OpcUa_BadInvalidArgument;
	}
	if (uStatus == OpcUa_Good)
	{
		pCallRequest = (OpcUa_CallRequest*)*a_ppRequest;


		// Get the server application object.
		uStatus = OpcUa_Endpoint_GetCallbackData(a_hEndpoint, (OpcUa_Void**)&pServerApplication);
		if (uStatus == OpcUa_Good)
		{
			OpcUa_UInt32 uSecureChannelId;
			uStatus = OpcUa_Endpoint_GetMessageSecureChannelId(a_hEndpoint, a_hContext, &uSecureChannelId);
			if (uStatus == OpcUa_Good)
			{
				// find the session.
				OpcUa_NodeId AuthenticationToken = pCallRequest->RequestHeader.AuthenticationToken;
				uStatus = pServerApplication->FindSession(uSecureChannelId, &AuthenticationToken, &pSessionServer);
				if (uStatus == OpcUa_Good)
				{
					// Signal that the session is alive
					OpcUa_Semaphore_Post(pSessionServer->m_SessionTimeoutSem, 1);
					// Remove all Call already handled
					if (pSessionServer->RemoveAllCallRequestDeleted() != OpcUa_Good)
						OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "RemoveAllCallRequest failed\n");
					// queue the request.
					CQueuedCallMessage* pQueuedCallMessage = new CQueuedCallMessage(pCallRequest, a_hEndpoint, a_hContext, a_pRequestType);
					// Add to queue
					uStatus=pSessionServer->QueueCallMessage(pQueuedCallMessage);
					if (uStatus == OpcUa_Good)
					{
						// Must set this to null to prevent the caller from deleting it.
						*a_ppRequest = OpcUa_Null;
					}
					else
						OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "QueueCallMessage failed uStatus=0x%05x\n",uStatus);
				}
			}
		}
		else
			// Send an error response.
			OpcUa_Endpoint_EndSendResponse(a_hEndpoint, &a_hContext, uStatus, OpcUa_Null, OpcUa_Null);
	}
	else
		// Send an error response.
		OpcUa_Endpoint_EndSendResponse(a_hEndpoint, &a_hContext, uStatus, OpcUa_Null, OpcUa_Null);
	return uStatus;
}

//OpcUa_StatusCode Server_Call(
//    OpcUa_Endpoint                 a_hEndpoint,
//    OpcUa_Handle                   a_hContext,
//    const OpcUa_RequestHeader*     a_pRequestHeader,
//    OpcUa_Int32                    a_nNoOfMethodsToCall,
//    const OpcUa_CallMethodRequest* a_pMethodsToCall,
//    OpcUa_ResponseHeader*          a_pResponseHeader,
//    OpcUa_Int32*                   a_pNoOfResults,
//    OpcUa_CallMethodResult**       a_pResults,
//    OpcUa_Int32*                   a_pNoOfDiagnosticInfos,
//    OpcUa_DiagnosticInfo**         a_pDiagnosticInfos)
//{
//	OpcUa_ReferenceParameter(a_pNoOfResults);
//	OpcUa_ReferenceParameter(a_pResults);
//	OpcUa_ReferenceParameter(a_pNoOfDiagnosticInfos);
//	OpcUa_ReferenceParameter(a_pDiagnosticInfos);
//	OpcUa_ReferenceParameter(a_nNoOfMethodsToCall);
//	OpcUa_ReferenceParameter(a_pMethodsToCall);
//	OpcUa_StatusCode uStatus=OpcUa_Good;
//	CServerApplication* pServer = 0;
//	OpcUa_UInt32 uSecureChannelId = 0;
//	//
//	uStatus = OpcUa_Endpoint_GetCallbackData(a_hEndpoint, (OpcUa_Void**)&pServer);
//	if (uStatus==OpcUa_Good)
//	{
//		// Get the secure channel being used.
//		uStatus = OpcUa_Endpoint_GetMessageSecureChannelId(a_hEndpoint, a_hContext, &uSecureChannelId);
//		if (uStatus==OpcUa_Good)
//		{
//
//		}
//	}
//	// Update the response header.
//	a_pResponseHeader->Timestamp     = OpcUa_DateTime_UtcNow();
//	a_pResponseHeader->RequestHandle = a_pRequestHeader->RequestHandle;
//	a_pResponseHeader->ServiceResult = uStatus;
//	return uStatus;
//}

OpcUa_StatusCode Server_BeginHistoryRead(
	OpcUa_Endpoint        a_hEndpoint,
	OpcUa_Handle          a_hContext,
	OpcUa_Void**          a_ppRequest,
	OpcUa_EncodeableType* a_pRequestType)
{
	CSessionServer* pSessionServer = 0;
	OpcUa_HistoryReadRequest* pHistoryReadRequest = OpcUa_Null;
	CServerApplication* pServerApplication = 0;

	OpcUa_StatusCode uStatus=OpcUa_Good;

	if (!a_hEndpoint)
		uStatus=OpcUa_BadInvalidArgument;
	if(!a_hContext)
		uStatus=OpcUa_BadInvalidArgument;
	if(!a_ppRequest)
		uStatus=OpcUa_BadInvalidArgument;
	else
	{
		if (*a_ppRequest == NULL)
			uStatus = OpcUa_BadInvalidArgument;
	}
	if (!a_pRequestType)
		uStatus=OpcUa_BadInvalidArgument;
	else
	{
		if (a_pRequestType->TypeId != OpcUaId_HistoryReadRequest)
			uStatus= OpcUa_BadInvalidArgument;
	}
	if (uStatus==OpcUa_Good)
	{
		pHistoryReadRequest = (OpcUa_HistoryReadRequest*)*a_ppRequest;


		// Get the server application object.
		uStatus = OpcUa_Endpoint_GetCallbackData(a_hEndpoint, (OpcUa_Void**)&pServerApplication);
		if (uStatus==OpcUa_Good)
		{
			OpcUa_UInt32 uSecureChannelId=0;
			uStatus = OpcUa_Endpoint_GetMessageSecureChannelId(a_hEndpoint, a_hContext, &uSecureChannelId);
			if (uStatus==OpcUa_Good)
			{
				// find the session.
				OpcUa_NodeId AuthenticationToken=pHistoryReadRequest->RequestHeader.AuthenticationToken;
				uStatus = pServerApplication->FindSession(uSecureChannelId,&AuthenticationToken,&pSessionServer);
				if (uStatus==OpcUa_Good)
				{
					// Signal that the session is alive
					OpcUa_Semaphore_Post(pSessionServer->m_SessionTimeoutSem, 1);
					// Effacement des requetes de lecture déjà traitée
					if (pSessionServer->RemoveAllHistoryReadRequestDeleted() != OpcUa_Good)
						OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "RemoveAllHistoryReadRequest failed\n");
					//// Create and Queue the History Request.
					CQueuedHistoryReadMessage* pQueuedHistoryReadMessage = new CQueuedHistoryReadMessage(pHistoryReadRequest, a_hEndpoint, a_hContext, a_pRequestType);
					//// Add to queue
					pSessionServer->QueueHistoryReadMessage(pQueuedHistoryReadMessage);
					// Must set this to null to prevent the caller from deleting it.
					*a_ppRequest = OpcUa_Null;
				}
			}
			else
				uStatus=OpcUa_BadSessionIdInvalid;
		}
		else
			// Send an error response.
			OpcUa_Endpoint_EndSendResponse(a_hEndpoint, &a_hContext, uStatus, OpcUa_Null, OpcUa_Null);
	}
	else
		// Send an error response.
		OpcUa_Endpoint_EndSendResponse(a_hEndpoint, &a_hContext, uStatus, OpcUa_Null, OpcUa_Null);

	return uStatus;
}