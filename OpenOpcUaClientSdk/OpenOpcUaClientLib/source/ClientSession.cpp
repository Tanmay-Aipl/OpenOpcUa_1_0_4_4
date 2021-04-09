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
#include "OpenOpcUaClientLib.h"


#include "MonitoredItemClient.h"
#include "MonitoredItemsNotification.h"
#include "SubscriptionClient.h"
#include "ClientSession.h"
#include "LoggerMessage.h"
#include "ClientApplication.h"
#include "StatusCodeException.h"
using namespace OpenOpcUa;
using namespace UASharedLib;
using namespace UACoreClient;

//BOOL CSessionClient::m_bRunPublishingThread;
//OpcUa_Handle CSessionClient::m_hStopPublishingThread;
struct PublishAsyncCallbackData
{
	//CSubscriptionClient*			pSubscription;
	CSessionClient*					pClientSession;
	CPendingPublish*				pPendingPublish;
	//CClientApplication				pClientApplication;
	//PfnSessionClient_OnPublish*				pCallback;
	//OpcUa_Int32								NoOfAvailableSequenceNumbers; // retourné par la dernière notification.. -1 à l'init
	//OpcUa_UInt32*							pAvailableSequenceNumbers;  // retourné par la dernière notification. NULL a l'init
};
//typedef struct _OpcUa_PublishResponse
//{
//    OpcUa_ResponseHeader      ResponseHeader;
//    OpcUa_UInt32              SubscriptionId;
//    OpcUa_Int32               NoOfAvailableSequenceNumbers;
//    OpcUa_UInt32*             AvailableSequenceNumbers;
//    OpcUa_Boolean             MoreNotifications;
//    OpcUa_NotificationMessage NotificationMessage;
//    OpcUa_Int32               NoOfResults;
//    OpcUa_StatusCode*         Results;
//    OpcUa_Int32               NoOfDiagnosticInfos;
//    OpcUa_DiagnosticInfo*     DiagnosticInfos;
//}
//OpcUa_PublishResponse;
// Thread qui recoit le reponse aux message publish qui ont éét envoyé par la thread de PublishingThread
OpcUa_StatusCode PublishAsyncCallback(
	OpcUa_Channel           a_hChannel,
	OpcUa_Void*             a_pResponse,
	OpcUa_EncodeableType*   a_pResponseType,
	OpcUa_Void*             a_pCallbackData,
	OpcUa_StatusCode        a_uStatus)
{
	OpcUa_StatusCode    uStatus              = OpcUa_Good; 
	OpcUa_ReferenceParameter(OpcUa_Module_Client);
	OpcUa_PublishResponse* pResponse = (OpcUa_PublishResponse*)a_pResponse;
	PublishAsyncCallbackData* pData = (PublishAsyncCallbackData*)a_pCallbackData;

	OpcUa_ReferenceParameter(a_hChannel);
	if (a_pResponseType)
	{
		if (OpcUa_StrCmpA(a_pResponseType->TypeName, "ServiceFault") == 0)
		{
			OpcUa_EncodeableObject_Delete(a_pResponseType, &a_pResponse);
			return OpcUa_BadInternalError;
		}
		CSessionClient* pSessionClient=(CSessionClient*)pData->pClientSession;
		if (pSessionClient)
		{
			pSessionClient->DecPendingPublish();
			pSessionClient->RemovePendingPublish(pData->pPendingPublish);
			OpcUa_Free(pData);
			//pSessionClient->RemoveAllPendingPublish();
			// get the related subscription
			CSubscriptionClient* pSubscription = OpcUa_Null;//(CSubscriptionClient*)pData->pSubscription;

			if (OpcUa_IsBad(a_uStatus))
			{
				uStatus = a_uStatus;
			}
			else
			{
				//
				if (OpcUa_IsBad(pResponse->ResponseHeader.ServiceResult))
				{
					uStatus = pResponse->ResponseHeader.ServiceResult;
					if (uStatus == OpcUa_BadTooManyPublishRequests)
					{
						pSessionClient->m_internalPublishStatus = OpcUa_BadTooManyPublishRequests;
						uStatus = OpcUa_Good;
					}
				}
				else
					pSessionClient->m_internalPublishStatus = OpcUa_Good;
				if (uStatus==OpcUa_Good)
				{
					
					if (pResponse->NoOfResults > 0)
					{
						for (OpcUa_Int32 iii = 0; iii < pResponse->NoOfResults; iii++)
						{
							if (pResponse->Results[iii] != OpcUa_Good)
							{
								uStatus = pResponse->Results[iii];
							}
						}
					}
					// N° de séquence disponible, transmise par le serveur
					int iSequenceNumber = 1;
					if (pResponse->AvailableSequenceNumbers)
					{
						OpcUa_Mutex_Lock(pSessionClient->m_SubscriptionListMutex);
						if (pSessionClient->FindSubscription(pResponse->SubscriptionId, &pSubscription) == OpcUa_Good)
						{
							iSequenceNumber = pResponse->NoOfAvailableSequenceNumbers;
							if (pSubscription)
							{
								for (int iii = 0; iii < iSequenceNumber; iii++)
								{
									//OpcUa_Mutex_Lock(pSubscription->GetMonitoredItemListMutex());
									pSubscription->AddSequenceNumber(pResponse->AvailableSequenceNumbers[iii]);
									//OpcUa_Mutex_Unlock(pSubscription->GetMonitoredItemListMutex());
								}
							}
						}
						OpcUa_Mutex_Unlock(pSessionClient->m_SubscriptionListMutex);
					}
					else
					{
						OpcUa_Mutex_Lock(pSessionClient->m_SubscriptionListMutex);
						if (pSessionClient->FindSubscription(pResponse->SubscriptionId, &pSubscription) == OpcUa_Good)
						{
							//OpcUa_Mutex_Lock(pSubscription->GetMonitoredItemListMutex());
							if (pResponse->NotificationMessage.SequenceNumber >= 0)
								pSubscription->AddSequenceNumber(pResponse->NotificationMessage.SequenceNumber);
							//OpcUa_Mutex_Unlock(pSubscription->GetMonitoredItemListMutex());
						}
						else
						{
							if (pResponse->NotificationMessage.SequenceNumber >= 0)
								pSessionClient->SetSequenceNumber(pResponse->NotificationMessage.SequenceNumber);
						}
						OpcUa_Mutex_Unlock(pSessionClient->m_SubscriptionListMutex);
					}

					// traitement du message de notification
					if (pResponse->NotificationMessage.NoOfNotificationData)
					{
						for (int ii = 0; ii < pResponse->NotificationMessage.NoOfNotificationData; ii++)
						{
							// check du type de réponse reçu
							if (pResponse->NotificationMessage.NotificationData[ii].Body.EncodeableObject.Type)
							{
								// qu'est ce que l'on a recu comme notification
								if (pResponse->NotificationMessage.NotificationData[ii].Body.EncodeableObject.Type->TypeId == OpcUaId_DataChangeNotification)
								{
									// reception d'une DataChangeNotification
									OpcUa_DataChangeNotification* pNotificationResponse = OpcUa_Null;
									pNotificationResponse = (OpcUa_DataChangeNotification*)(pResponse->NotificationMessage.NotificationData[ii].Body.EncodeableObject.Object);
									// Transfert dans une collection
									CMonitoredItemsNotification* pMonitoredItems = new CMonitoredItemsNotification();
									pMonitoredItems->SetNoOfMonitoredItems(pNotificationResponse->NoOfMonitoredItems);
									//pMonitoredItems->m_bDone = OpcUa_False;
									pMonitoredItems->SetMonitoredItemNotification(pNotificationResponse->NoOfMonitoredItems, pNotificationResponse->MonitoredItems);
									// remplissage de la collection m_pMonitoredItems
									if (pSubscription)
										pSubscription->AddMonitoredItemNotification(pMonitoredItems);
								}
								if (pResponse->NotificationMessage.NotificationData[ii].Body.EncodeableObject.Type->TypeId == OpcUaId_EventNotificationList)
								{
									OpcUa_EventNotificationList* pEventNotificationList = OpcUa_Null;
									pEventNotificationList = (OpcUa_EventNotificationList*)(pResponse->NotificationMessage.NotificationData[ii].Body.EncodeableObject.Object);
								}
								if (pResponse->NotificationMessage.NotificationData[ii].Body.EncodeableObject.Type->TypeId == OpcUaId_StatusChangeNotification)
								{
									// reception d'une StatusChangeNotification
									OpcUa_StatusChangeNotification* pStatusChangeNotification = OpcUa_Null;
									pStatusChangeNotification = (OpcUa_StatusChangeNotification*)(pResponse->NotificationMessage.NotificationData[ii].Body.EncodeableObject.Object);
									pSessionClient->SetInternalServerStatus(pStatusChangeNotification->Status);
								}
							}
						}
					}
					else
					{
						// this is probably a keepAlive sent by the server.
						//We consider in the 1.0.2.0 SDK of OpenOpcUaClientLib that the host appnt can be interest by it
						CMonitoredItemsNotification* pMonitoredItems = new CMonitoredItemsNotification();
						if (pSubscription)
							pSubscription->AddMonitoredItemNotification(pMonitoredItems);
					}
				}
			}
		}
		else
			uStatus = OpcUa_BadSessionClosed;
		OpcUa_EncodeableObject_Delete(a_pResponseType, &a_pResponse);
	
		// Let's indicate the Client Host the current state of the connection
		if (pSessionClient)
			pSessionClient->SetInternalServerStatus(a_uStatus);
	}
	else
	{
		if (pData)
		{
			CSessionClient* pSessionClient = (CSessionClient*)pData->pClientSession;
			pSessionClient->RemovePendingPublish(pData->pPendingPublish);
		}
	}
	return uStatus;
}

CSessionClient::CSessionClient()
{
	m_internalPublishStatus = OpcUa_Good;
	m_uiPendingPublish = 0;
	m_bFromWatchingThread = OpcUa_False;
	m_pApplication = OpcUa_Null;
	OpcUa_Mutex_Create(&m_PendingPublishListMutex);
	OpcUa_Mutex_Create(&m_SubscriptionListMutex);
	OpcUa_Mutex_Create(&m_hWatchingMutex);
	m_wSessionState =SESSION_STATE_UNDETERMINED;
	m_ClassName=std::string("UAQuickClient::CSessionClient");
	OpcUa_Semaphore_Create(&m_WatchingSem,0,0x100);
	m_hInternalWatchingThread=OpcUa_Null;
	m_bRunWatchingThread=OpcUa_False;
	m_pShutdownCallback=OpcUa_Null;
	m_pChannel=OpcUa_Null;	
	// Publishing thread related
	m_bRunPublishingThread=TRUE;
	m_hPublishingThread=NULL;
	m_uiSequenceNumber=1;
	OpcUa_Semaphore_Create(&m_hStopPublishingThread,0,0x100);
	SetInternalServerStatus(OpcUa_BadServerNotConnected);
	// Channel related
	OpcUa_Mutex_Create(&m_ChannelMutex);
	// Publish adjustement val
	m_AdjustPublishVal = 1;
}
CSessionClient::CSessionClient(CClientApplication* pApplication) 
{
	m_internalPublishStatus = OpcUa_Good;
	m_uiPendingPublish = 0;
	m_bFromWatchingThread = OpcUa_False;
	m_pApplication = OpcUa_Null;
	SetInternalServerStatus(OpcUa_BadServerNotConnected);
	OpcUa_Mutex_Create(&m_PendingPublishListMutex);
	OpcUa_Mutex_Create(&m_SubscriptionListMutex);
	OpcUa_Mutex_Create(&m_hWatchingMutex);
	m_ClassName=std::string("UAQuickClient::CSessionClient");
	m_pApplication = pApplication;
	m_nSessionTimeout = 0;
	m_pChannel = new CChannel(pApplication);
	OpcUa_NodeId_Initialize(&m_tSessionId);
	//OpcUa_NodeId_Initialize(&m_tAuthenticationToken);
	OpcUa_ByteString_Initialize(&m_tServerNonce);
	OpcUa_ByteString_Initialize(&m_tServerCertificate);
	OpcUa_MemSet(&m_tCryptoProvider, 0, sizeof(OpcUa_CryptoProvider));
	OpcUa_Semaphore_Create(&m_WatchingSem,0,0x100);
	m_hInternalWatchingThread=OpcUa_Null;
	m_bRunWatchingThread=OpcUa_False;
	m_pShutdownCallback=OpcUa_Null;
	// Publishing thread related
	m_bRunPublishingThread=TRUE;
	m_hPublishingThread=NULL;
	m_uiSequenceNumber=1;
	OpcUa_Semaphore_Create(&m_hStopPublishingThread,0,0x100);
	// Channel related
	OpcUa_Mutex_Create(&m_ChannelMutex);
	// Publish adjustement val
	m_AdjustPublishVal = 1;
}

CSessionClient::~CSessionClient(void)
{
	//// Publishing thread related
	OpcUa_StatusCode uStatus;
	RemoveAllPendingPublish();
	OpcUa_Mutex_Delete(&m_hWatchingMutex);
	m_pApplication = OpcUa_Null;
	uStatus = Close(); // Call client API to close explicitly the session
	if (uStatus != OpcUa_Good)
		OpcUa_Trace(OPCUA_TRACE_CLIENT_LEVEL_ERROR, "Close failed 0x%05x", uStatus);

	Delete();
	OpcUa_Semaphore_Delete(&m_hStopPublishingThread);
	OpcUa_Semaphore_Delete(&m_WatchingSem); 
	OpcUa_Mutex_Delete(&m_ChannelMutex);
	OpcUa_Mutex_Delete(&m_PendingPublishListMutex);
}

void CSessionClient::Delete()
{

	OpcUa_NodeId_Clear(&m_tSessionId);
	OpcUa_ByteString_Clear(&m_tServerNonce);
	OpcUa_ByteString_Clear(&m_tServerCertificate);

	if (m_tCryptoProvider.Handle != 0)
	{
		OpcUa_CryptoProvider_Delete(&m_tCryptoProvider);
	}
	OpcUa_Mutex_Lock(m_ChannelMutex);
	if (m_pChannel)
	{
		m_pChannel->Disconnect();
		delete m_pChannel;
		m_pChannel = OpcUa_Null;
	}
	OpcUa_Mutex_Unlock(m_ChannelMutex);
	OpcUa_Mutex_Delete(&m_SubscriptionListMutex);
}


// Function name   : CSessionClient::Create
// Description     : Création de la session ==> OpcUa_CryptoProvider_Create + OpcUa_Crypto_GenerateKey + OpcUa_ClientApi_CreateSession
// Return type     : HRESULT 
// Argument        : CEndpointDescription endpoint. Endpoint of the server to connect to
// Argument        : std::string CSessionClientName

OpcUa_StatusCode CSessionClient::Create(CEndpointDescription* pEndpoint, OpcUa_String* ClientSessionName,OpcUa_Double nRequestedClientSessionTimeout)
{
	CStatusCodeException e;
	OpcUa_RequestHeader                tRequestHeader;
	OpcUa_ApplicationDescription       tClientDescription;
	//sServerUri : This value is only specified if the EndpointDescription has a gatewayServerUri.
	//	This value is the applicationUri from the EndpointDescription which is the applicationUri for the underlying Server.
	OpcUa_String                       sServerUri;
	OpcUa_String                       sEndpointUrl;
	OpcUa_String                       sClientSessionName;
	OpcUa_Key				           tClientNonce;
	
	OpcUa_UInt32                       nMaxResponseMessageSize = 0;
	OpcUa_ResponseHeader               tResponseHeader;
	OpcUa_NodeId                       tClientSessionId;
	OpcUa_NodeId                       tAuthenticationToken;
	OpcUa_Double                       nRevisedSessionTimeout;
	OpcUa_ByteString                   tServerNonce;
	OpcUa_ByteString                   tServerCertificate;
	OpcUa_Int32                        nNoOfServerEndpoints = 0;
	OpcUa_EndpointDescription*         pServerEndpoints = 0;
	OpcUa_Int32                        nNoOfServerSoftwareCertificates = 0;
	OpcUa_SignedSoftwareCertificate*   pServerSoftwareCertificates = 0;
	OpcUa_SignatureData                tServerSignature;
	OpcUa_UInt32                       nMaxRequestMessageSize = 0;

	OpcUa_StatusCode uStatus=OpcUa_Good;

	OpcUa_RequestHeader_Initialize(&tRequestHeader);
	OpcUa_ApplicationDescription_Initialize(&tClientDescription);
	OpcUa_String_Initialize(&sServerUri);
	OpcUa_String_Initialize(&sEndpointUrl);
	OpcUa_String_Initialize(&sClientSessionName);
	OpcUa_Key_Initialize(&tClientNonce);
	OpcUa_ResponseHeader_Initialize(&tResponseHeader);
	OpcUa_NodeId_Initialize(&tClientSessionId);
	OpcUa_NodeId_Initialize(&tAuthenticationToken);
	OpcUa_ByteString_Initialize(&tServerNonce);
	OpcUa_ByteString_Initialize(&tServerCertificate);
	OpcUa_SignatureData_Initialize(&tServerSignature);
	// connect to the server.
	OpcUa_Mutex_Lock(m_ChannelMutex);
	uStatus=m_pChannel->Connect(pEndpoint);
	if (uStatus==OpcUa_Good)
	{
		// fill in request header.
		tRequestHeader.TimeoutHint = g_ServiceCallTimeout;// UTILS_DEFAULT_TIMEOUT;
		tRequestHeader.Timestamp   = OpcUa_DateTime_UtcNow();
		tRequestHeader.RequestHandle = CClientApplication::m_uiRequestHandle++;
		// fill in application description.
		OpcUa_LocalizedText aLocalizedText;
		OpcUa_LocalizedText_Initialize(&aLocalizedText);
		CApplicationDescription* pClientApplicationDescription=m_pApplication->GetApplicationDescription();
		if (pClientApplicationDescription)
		{
			aLocalizedText=pClientApplicationDescription->GetApplicationName();
			OpcUa_LocalizedText_CopyTo(&aLocalizedText,&tClientDescription.ApplicationName);

			tClientDescription.ApplicationType      = m_pApplication->GetApplicationDescription()->GetApplicationType();//  OpcUa_ApplicationType_Client;			
			OpcUa_String* pString = m_pApplication->GetApplicationDescription()->GetApplicationUri();
			OpcUa_String_CopyTo(pString, &(tClientDescription.ApplicationUri));

			// Let try to fill the sServerUri;
			// Read the gatewayServerUri
			OpcUa_ApplicationDescription* pApplicationDescription = pEndpoint->GetApplicationDescription();
			if (pApplicationDescription)
			{
				if (OpcUa_String_StrLen(&(pApplicationDescription->GatewayServerUri)) > 0)
				{
					OpcUa_String_StrnCpy(&sServerUri, 
						&(pApplicationDescription->ApplicationUri), 
						OpcUa_String_StrLen(&(pApplicationDescription->ApplicationUri)));					
				}
			}
			OpcUa_String_CopyTo(pEndpoint->GetEndpointUrl(),&sEndpointUrl);
			OpcUa_String_CopyTo(ClientSessionName,&sClientSessionName);

			if (pEndpoint->GetSecurityMode() != OpcUa_MessageSecurityMode_None)
			{
				// create crypto provider.
				uStatus = OpcUa_CryptoProvider_Create((OpcUa_StringA)OpcUa_String_GetRawString(pEndpoint->GetSecurityPolicyUri()), &m_tCryptoProvider);

				if (OpcUa_IsBad(uStatus))
				{
					OpcUa_Trace(OPCUA_TRACE_CLIENT_LEVEL_ERROR,"Could not create crypto provider.uStatus=0x%05x\n",uStatus);
				}
				else
				{
					// generate a nonce.
					tClientNonce.Key.Length = 32;
					tClientNonce.Key.Data = (OpcUa_Byte*)OpcUa_Alloc(tClientNonce.Key.Length);

					uStatus = OpcUa_Crypto_GenerateKey(&m_tCryptoProvider, tClientNonce.Key.Length, &tClientNonce);

					if (OpcUa_IsBad(uStatus))
					{
						OpcUa_Trace(OPCUA_TRACE_CLIENT_LEVEL_ERROR,"Could not create client nonce. uStatus=0x%05x\n",uStatus);
					}
				}
			}
			if (uStatus==OpcUa_Good)
			{
				// create the ClientSession.
				OpcUa_Channel aChannel = m_pChannel->GetInternalHandle();
				uStatus = OpcUa_ClientApi_CreateSession(  
					aChannel,
					&tRequestHeader,
					&tClientDescription,
					&sServerUri,
					&sEndpointUrl, 
					&sClientSessionName,
					&tClientNonce.Key, 
					m_pApplication->GetCertificate(),
					nRequestedClientSessionTimeout,
					nMaxResponseMessageSize,
					&tResponseHeader,
					&tClientSessionId,
					&tAuthenticationToken,
					&nRevisedSessionTimeout,
					&tServerNonce,
					&tServerCertificate,
					&nNoOfServerEndpoints,
					&pServerEndpoints,
					&nNoOfServerSoftwareCertificates,
					&pServerSoftwareCertificates,
					&tServerSignature,
					&nMaxRequestMessageSize);

				if ( (uStatus != OpcUa_Good) || (tResponseHeader.ServiceResult!=OpcUa_Good) )
					OpcUa_Trace(OPCUA_TRACE_CLIENT_LEVEL_ERROR, "Could not create a new session. uStatus=0x%05x ServiceResult=0x%05x\n", uStatus, tResponseHeader.ServiceResult);
				else
				{
					SetInternalServerStatus(OpcUa_Good);
					// verify server signature.
					if (pEndpoint->GetSecurityMode() != OpcUa_MessageSecurityMode_None)
					{	
						OpcUa_ByteString* pByteString= pEndpoint->GetServerCertificate();
						bool match = (int)pByteString->Length == tServerCertificate.Length;
						if (match)
						{
							for (int ii = 0; ii < (int)pByteString->Length; ii++)
							{
								if (tServerCertificate.Data[ii] != pByteString->Data[ii])
								{
									match = false;
									uStatus=OpcUa_BadCertificateInvalid;
									break;
								}
							}
						}

						if (!match)
							OpcUa_Trace(OPCUA_TRACE_CLIENT_LEVEL_ERROR,"Server returned a certificate does not match the expected certificate. uStatus=0x%05x\n",uStatus);
						else
							CryptoUtils::VerifySignature(&m_tCryptoProvider, m_pApplication->GetCertificate(), &tClientNonce.Key, &tServerCertificate, &tServerSignature);
					}
					if (uStatus==OpcUa_Good)
					{
						// save session information.
						m_tSessionId = tClientSessionId;
						SetAuthenticationToken(&tAuthenticationToken);
						m_nSessionTimeout = nRevisedSessionTimeout;
						m_tServerCertificate = tServerCertificate;
						m_tServerNonce = tServerNonce;

						CEndpointDescription* pMyEndpointDescription=new CEndpointDescription(pEndpoint);
						SetEndpointDescription(pMyEndpointDescription);
						m_wSessionState =SESSION_STATE_ON_INACTIVE;
						SetSessionName(&sClientSessionName);
					}

					Utils_ClearArray(pServerEndpoints, nNoOfServerEndpoints, OpcUa_EndpointDescription);
					Utils_ClearArray(pServerSoftwareCertificates, nNoOfServerSoftwareCertificates, OpcUa_SignedSoftwareCertificate);
				}
			}
		}
		else
		{
			OpcUa_Trace(OPCUA_TRACE_CLIENT_LEVEL_ERROR,"Critical error>ApplicationDescription uninitialized\n");
			uStatus=OpcUa_BadInvalidArgument;
		}
	}
	OpcUa_Mutex_Unlock(m_ChannelMutex);
	// clean up.
	OpcUa_RequestHeader_Clear(&tRequestHeader);
	OpcUa_ApplicationDescription_Clear(&tClientDescription);
	OpcUa_String_Clear(&sServerUri);
	OpcUa_String_Clear(&sEndpointUrl);
	OpcUa_String_Clear(&sClientSessionName);
	OpcUa_Key_Clear(&tClientNonce);
	OpcUa_ResponseHeader_Clear(&tResponseHeader);
	OpcUa_NodeId_Clear(&tAuthenticationToken);
	return uStatus;
}


// Function name   : ClientSession::Activate
// Description     : 
// Return type     : HRESULT 

OpcUa_StatusCode CSessionClient::Activate(OpcUa_EndpointDescription* pEndpointDescription, const char* AuthenticationMode, const char* sUserName, const char* sPassword )
{
	OpcUa_StatusCode uStatus=OpcUa_Good;

	OpcUa_RequestHeader                tRequestHeader;
	OpcUa_SignatureData                tClientSignature;
	OpcUa_Int32                        nNoOfClientSoftwareCertificates = 0;
	OpcUa_SignedSoftwareCertificate    tClientSoftwareCertificates;
	OpcUa_Int32                        nNoOfLocaleIds = 0;
	OpcUa_String                       sLocaleIds;
	OpcUa_ExtensionObject              tUserIdentityToken;
	OpcUa_SignatureData                tUserTokenSignature;
	OpcUa_ResponseHeader               tResponseHeader;
	OpcUa_ByteString                   tServerNonce;
	OpcUa_Int32                        nNoOfResults = 0;
	OpcUa_StatusCode*                  pResults = 0;
	OpcUa_Int32                        nNoOfDiagnosticInfos = 0;
	OpcUa_DiagnosticInfo*              pDiagnosticInfos = 0;


	OpcUa_RequestHeader_Initialize(&tRequestHeader);
	OpcUa_SignatureData_Initialize(&tClientSignature);
	OpcUa_SignedSoftwareCertificate_Initialize(&tClientSoftwareCertificates);
	OpcUa_String_Initialize(&sLocaleIds);
	OpcUa_ExtensionObject_Initialize(&tUserIdentityToken);
	OpcUa_SignatureData_Initialize(&tUserTokenSignature);
	OpcUa_ResponseHeader_Initialize(&tResponseHeader);
	OpcUa_ByteString_Initialize(&tServerNonce);

	// fill in request header.
	tRequestHeader.TimeoutHint = g_ServiceCallTimeout;// UTILS_DEFAULT_TIMEOUT;
	tRequestHeader.Timestamp           = OpcUa_DateTime_UtcNow();
	tRequestHeader.RequestHandle = CClientApplication::m_uiRequestHandle++;
	OpcUa_NodeId_CopyTo(m_pAuthenticationToken,&(tRequestHeader.AuthenticationToken));
	if (pEndpointDescription->SecurityMode != OpcUa_MessageSecurityMode_None)
	{
		CryptoUtils::CreateSignature(
			&m_tCryptoProvider, 
			&m_tServerCertificate, 
			&m_tServerNonce,
			m_pApplication->GetCertificate(), 
			m_pApplication->GetPrivateKey(), 
			&tClientSignature);
	}
	bool bSpecified = false;
	if (strcmp(AuthenticationMode, "anonymous") == 0)
	{
		OpcUa_AnonymousIdentityToken* pAnonymousIdentityToken = (OpcUa_AnonymousIdentityToken*)OpcUa_Alloc(sizeof(OpcUa_AnonymousIdentityToken));
		if (pAnonymousIdentityToken)
		{
			OpcUa_AnonymousIdentityToken_Initialize(pAnonymousIdentityToken);
			OpcUa_String_AttachCopy(&(pAnonymousIdentityToken->PolicyId), "0");
		}

		tUserIdentityToken.Encoding = OpcUa_ExtensionObjectEncoding_None;
		//tUserIdentityToken.Body.EncodeableObject.Type->TypeName = "AnonymousIdentityToken";
		//OpcUa_AnonymousIdentityToken_EncodeableType.NamespaceUri
		//OpcUa_AnonymousIdentityToken_EncodeableType.NamespaceUri = "urn:Pavan-PC.Aipl.local:OPCUA:SimulationServer";
		tUserIdentityToken.Body.EncodeableObject.Type = &OpcUa_AnonymousIdentityToken_EncodeableType;
		tUserIdentityToken.Body.EncodeableObject.Object = pAnonymousIdentityToken;
		tUserIdentityToken.BodySize = sizeof(pAnonymousIdentityToken);
		bSpecified = true;
	}
	else if (strcmp(AuthenticationMode, "user") == 0)
	{
		OpcUa_UserNameIdentityToken* pUserNameIdentityToken = (OpcUa_UserNameIdentityToken*)OpcUa_Alloc(sizeof(OpcUa_UserNameIdentityToken));
		if (pUserNameIdentityToken)
		{
			OpcUa_UserNameIdentityToken_Initialize(pUserNameIdentityToken);
			// PolicyId
			OpcUa_String_AttachCopy(&(pUserNameIdentityToken->PolicyId), "username_basic256");
			//OpcUa_String_AttachCopy(&(pUserNameIdentityToken->EncryptionAlgorithm), "http://www.w3.org/2001/04/xmlenc#rsa-1_5");
			// UserName
			OpcUa_String_Initialize(&(pUserNameIdentityToken->UserName));
			OpcUa_String_AttachCopy(&(pUserNameIdentityToken->UserName), sUserName);
			// Password
			pUserNameIdentityToken->Password.Data = (OpcUa_Byte*)OpcUa_Alloc(10);
			if (pUserNameIdentityToken->Password.Data)
			{
				ZeroMemory(pUserNameIdentityToken->Password.Data, sizeof(sPassword)+1);
				OpcUa_MemCpy(pUserNameIdentityToken->Password.Data, sizeof(sPassword), (OpcUa_Byte*)sPassword, sizeof(sPassword));
				pUserNameIdentityToken->Password.Length = sizeof(sPassword);
			}
			else
			{
				OpcUa_Free(pUserNameIdentityToken);
				uStatus = OpcUa_BadOutOfMemory;
			}
			/*OpcUa_UserNameIdentityToken_Initialize(pUserNameIdentityToken);
			// PolicyId
			OpcUa_String_AttachCopy(&(pUserNameIdentityToken->PolicyId), "username_basic256");
			OpcUa_String_AttachCopy(&(pUserNameIdentityToken->EncryptionAlgorithm), "http://www.w3.org/2001/04/xmlenc#rsa-oaep");
			// UserName
			OpcUa_String_Initialize(&(pUserNameIdentityToken->UserName));
			OpcUa_String_AttachCopy(&(pUserNameIdentityToken->UserName), sUserName);
			// Password
			pUserNameIdentityToken->Password.Data = (OpcUa_Byte*)OpcUa_Alloc(6);
			if (pUserNameIdentityToken->Password.Data)
			{
				ZeroMemory(pUserNameIdentityToken->Password.Data, 6);
				OpcUa_MemCpy(pUserNameIdentityToken->Password.Data, 5, (OpcUa_Byte*)sPassword, sizeof(sPassword));
				pUserNameIdentityToken->Password.Length = 5;
			}
			else
			{
				OpcUa_Free(pUserNameIdentityToken);
				uStatus = OpcUa_BadOutOfMemory;
			}*/
		}
		tUserIdentityToken.Encoding = OpcUa_ExtensionObjectEncoding_Binary;
		tUserIdentityToken.Body.EncodeableObject.Type = &OpcUa_UserNameIdentityToken_EncodeableType;
		tUserIdentityToken.Body.EncodeableObject.Object = pUserNameIdentityToken;
		tUserIdentityToken.BodySize = sizeof(pUserNameIdentityToken);
		bSpecified = true;
	}
	if (bSpecified)
	{
		// fill in request header.
		tRequestHeader.TimeoutHint = UTILS_DEFAULT_TIMEOUT;
		tRequestHeader.Timestamp = OpcUa_DateTime_UtcNow();
		OpcUa_NodeId_CopyTo(m_pAuthenticationToken, &(tRequestHeader.AuthenticationToken));
	}

	// create the ClientSession.
	OpcUa_Channel aChannel = m_pChannel->GetInternalHandle();
	uStatus = OpcUa_ClientApi_ActivateSession(
		aChannel, 
		&tRequestHeader,
		&tClientSignature,
		nNoOfClientSoftwareCertificates,
		&tClientSoftwareCertificates,
		nNoOfLocaleIds,
		&sLocaleIds,
		&tUserIdentityToken,
		&tUserTokenSignature,
		&tResponseHeader,
		&tServerNonce,
		&nNoOfResults,
		&pResults,
		&nNoOfDiagnosticInfos,
		&pDiagnosticInfos);

	if ((uStatus == OpcUa_Good) && (tResponseHeader.ServiceResult==OpcUa_Good))
	{
		// save ClientSession information.
		m_tServerNonce = tServerNonce;
		m_wSessionState =SESSION_STATE_ON_ACTIVE;
		OpcUa_Semaphore_Post(CSessionClient::m_hStopPublishingThread,1);
	}

	// clean up.
	OpcUa_RequestHeader_Clear(&tRequestHeader);
	OpcUa_SignatureData_Clear(&tClientSignature);
	OpcUa_SignedSoftwareCertificate_Clear(&tClientSoftwareCertificates);
	OpcUa_String_Clear(&sLocaleIds);
	OpcUa_ExtensionObject_Clear(&tUserIdentityToken);
	OpcUa_SignatureData_Clear(&tUserTokenSignature);
	OpcUa_ResponseHeader_Clear(&tResponseHeader);

	Utils_ClearArray(pResults, nNoOfResults, OpcUa_StatusCode);
	Utils_ClearArray(pDiagnosticInfos, nNoOfDiagnosticInfos, OpcUa_DiagnosticInfo);

	return tResponseHeader.ServiceResult;
}


// Function name   : ClientSession::Close
// Description     : 
// Return type     : HRESULT 

OpcUa_StatusCode CSessionClient::Close()
{
	
	OpcUa_RequestHeader  tRequestHeader;
	OpcUa_ResponseHeader tResponseHeader;

	OpcUa_StatusCode uStatus=OpcUa_Good;

	OpcUa_RequestHeader_Initialize(&tRequestHeader);
	OpcUa_ResponseHeader_Initialize(&tResponseHeader);
	StopPublishingThread();
	// fill in request header.
	tRequestHeader.TimeoutHint = g_ServiceCallTimeout;// UTILS_DEFAULT_TIMEOUT;
	tRequestHeader.Timestamp           = OpcUa_DateTime_UtcNow();
	tRequestHeader.RequestHandle = CClientApplication::m_uiRequestHandle++;
	OpcUa_NodeId* pNodeId = GetAuthenticationToken();
	if (pNodeId)
	{
		OpcUa_NodeId_CopyTo(pNodeId, &(tRequestHeader.AuthenticationToken));
		// close the ClientSession.
		OpcUa_Channel aChannel = m_pChannel->GetInternalHandle();
		uStatus = OpcUa_ClientApi_CloseSession(
			aChannel,
			&tRequestHeader,
			OpcUa_False,
			&tResponseHeader);
		if (uStatus != OpcUa_Good)
			OpcUa_Trace(OPCUA_TRACE_CLIENT_ERROR, "OpcUa_ClientApi_CloseSession failed 0x%05x\n", uStatus);
	}
	else
		OpcUa_Trace(OPCUA_TRACE_CLIENT_ERROR, "Session corrupted. Wrong AuthenticationToken\n");

	// clean up.
	OpcUa_RequestHeader_Clear(&tRequestHeader);
	OpcUa_ResponseHeader_Clear(&tResponseHeader);

	//Delete();
	return uStatus;
}


struct ReadAsyncCallbackData
{
	CSessionClient* pClientSession;
	PfnSessionClient_OnReadValue* pCallback;
	OpcUa_Void* pCallbackData;
};
// Callback function 
// these call are receive from the stack
static OpcUa_StatusCode ReadAsyncCallback(
	OpcUa_Channel           a_hChannel,
	OpcUa_Void*             a_pResponse,
	OpcUa_EncodeableType*   a_pResponseType,
	OpcUa_Void*             a_pCallbackData,
	OpcUa_StatusCode        a_uStatus)
{
	OpcUa_ReadResponse* pResponse = (OpcUa_ReadResponse*)a_pResponse;
	ReadAsyncCallbackData* pData = (ReadAsyncCallbackData*)a_pCallbackData;

	OpcUa_InitializeStatus(OpcUa_Module_Client, "ReadAsyncCallback");

	OpcUa_ReferenceParameter(a_hChannel);

	if (OpcUa_IsBad(a_uStatus))
	{
		uStatus = a_uStatus;
		OpcUa_GotoError;
	}

	if (OpcUa_IsBad(pResponse->ResponseHeader.ServiceResult))
	{
		uStatus = pResponse->ResponseHeader.ServiceResult;
		OpcUa_GotoError;
	}
	
	if (pResponse->NoOfResults != 1)
	{
		uStatus = OpcUa_BadUnknownResponse;
		OpcUa_GotoError;
	}

	if (OpcUa_IsBad(pResponse->Results[0].StatusCode))
	{
		uStatus = pResponse->Results[0].StatusCode;
		OpcUa_GotoError;
	}

	if (pData != 0)
	{
		pData->pCallback(
			pData->pClientSession,
			pData->pCallbackData,
			OpcUa_Good,
			&pResponse->Results[0].Value);

		delete pData;
	}

	OpcUa_EncodeableObject_Delete(a_pResponseType, &a_pResponse);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

	if (pData != 0)
	{
		pData->pCallback(
			pData->pClientSession,
			pData->pCallbackData,
			uStatus,
			0);

		delete pData;
	}

	OpcUa_EncodeableObject_Delete(a_pResponseType, &a_pResponse);

OpcUa_FinishErrorHandling;
}
OpcUa_StatusCode CSessionClient::Write(OpcUa_Int32 iNoNodeToWrite, OpcUa_WriteValue* pValue, OpcUa_StatusCode** pResults)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_RequestHeader		tRequestHeader;
	OpcUa_ResponseHeader	tResponseHeader;
	OpcUa_Int32             NoOfResults=0;
	OpcUa_Int32				tNoOfDiagnosticInfos=0;
	OpcUa_DiagnosticInfo*   tDiagnosticInfos=OpcUa_Null;
	if (pValue)
	{
		// fill in request header.
		OpcUa_RequestHeader_Initialize(&tRequestHeader);
		tRequestHeader.TimeoutHint = g_ServiceCallTimeout;// UTILS_DEFAULT_TIMEOUT;
		tRequestHeader.Timestamp = OpcUa_DateTime_UtcNow();
		tRequestHeader.RequestHandle = CClientApplication::m_uiRequestHandle++;
		//tRequestHeader.AuthenticationToken = *Utils::Copy(GetAuthenticationToken());
		OpcUa_NodeId_CopyTo(GetAuthenticationToken(), &(tRequestHeader.AuthenticationToken));

		// Init the response header	
		OpcUa_ResponseHeader_Initialize(&tResponseHeader);
		OpcUa_Channel aChannel = m_pChannel->GetInternalHandle();
		uStatus = OpcUa_ClientApi_Write(
			aChannel,
			&tRequestHeader,
			iNoNodeToWrite, // will write only one nodeId
			pValue,
			&tResponseHeader,
			&NoOfResults,
			pResults,
			&tNoOfDiagnosticInfos,
			&tDiagnosticInfos);

		if (tDiagnosticInfos)
		{
			for (OpcUa_UInt32 i = 0; i <NoOfResults; i++)
				OpcUa_DiagnosticInfo_Clear(&tDiagnosticInfos[i]);
			OpcUa_Free(tDiagnosticInfos);
		}
		OpcUa_RequestHeader_Clear(&tRequestHeader);
		OpcUa_ResponseHeader_Clear(&tResponseHeader);
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}

// Function name   : CSessionClient::BeginReadValue
// Description     : Cette méthode réalise un lecture asynchrone des la valeur d'une node (OpcUa_Attributes_Value 13)
// Return type     : void 
// Argument        : OpcUa_NodeId nodeId : Id de la node à lire
// Argument        : PfnSessionClient_OnReadValue* pCallback : Il s'agit de la fonction délégué qui recevra la réponse
// Argument        : OpcUa_Void* pCallbackData : Il s'agit des paramètres associé a la fonction délégué

OpcUa_StatusCode CSessionClient::BeginReadValue(OpcUa_NodeId nodeId, PfnSessionClient_OnReadValue* pCallback, OpcUa_Void* pCallbackData)
{
	OpcUa_RequestHeader tRequestHeader;
	OpcUa_ReadValueId   tNodesToRead;

	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_RequestHeader_Initialize(&tRequestHeader);
	OpcUa_ReadValueId_Initialize(&tNodesToRead);

	// select attribute.
	tNodesToRead.NodeId = nodeId;
	tNodesToRead.AttributeId = OpcUa_Attributes_Value;

	// fill in request header.
	tRequestHeader.TimeoutHint = g_ServiceCallTimeout;// UTILS_DEFAULT_TIMEOUT;
	tRequestHeader.Timestamp           = OpcUa_DateTime_UtcNow();
	OpcUa_NodeId* pNodeId = GetAuthenticationToken();
	OpcUa_NodeId_CopyTo(pNodeId, &(tRequestHeader.AuthenticationToken));
	tRequestHeader.RequestHandle = CClientApplication::m_uiRequestHandle++;
	ReadAsyncCallbackData* pData = new ReadAsyncCallbackData();

	pData->pClientSession = this;
	pData->pCallback = pCallback;
	pData->pCallbackData = pCallbackData;

	// create the ClientSession.
	OpcUa_Channel aChannel = m_pChannel->GetInternalHandle();
	uStatus = OpcUa_ClientApi_BeginRead(
		aChannel, 
		&tRequestHeader,
		0,
		OpcUa_TimestampsToReturn_Both,
		1,
		&tNodesToRead,
		ReadAsyncCallback,
		pData);

	
	// clean up.
	OpcUa_RequestHeader_Clear(&tRequestHeader);
	OpcUa_ReadValueId_Clear(&tNodesToRead);

	return uStatus;
}
OpcUa_StatusCode CSessionClient::Browse(OpcUa_Int32 a_nNoOfNodesToBrowse,
										const OpcUa_BrowseDescription*  pNodesToBrowse,
										OpcUa_ReferenceDescriptionList* pReferenceList)
{
	OpcUa_StatusCode		uStatus=OpcUa_Good;
	OpcUa_RequestHeader     tRequestHeader;
	OpcUa_ViewDescription   tView;
	OpcUa_UInt32            a_nRequestedMaxReferencesPerNode=100; // 0 for no limite
	OpcUa_ResponseHeader	tResponseHeader;
	OpcUa_Int32             NoOfResults=0;
	OpcUa_BrowseResult*     pResults=OpcUa_Null;
	OpcUa_Int32				tNoOfDiagnosticInfos=0;
	OpcUa_Int32				nNoOfContinuationPoints=0;
	OpcUa_DiagnosticInfo*   pDiagnosticInfos;
	// init DiagInfo
	pDiagnosticInfos=OpcUa_Null;// (OpcUa_DiagnosticInfo*)OpcUa_Alloc(sizeof(OpcUa_DiagnosticInfo));
	//OpcUa_DiagnosticInfo_Initialize(tDiagnosticInfos);

	// init BrowseResult
	pResults= NULL;
	
	 OpcUa_RequestHeader_Initialize(&tRequestHeader);
	// fill in request header.
	 tRequestHeader.TimeoutHint = g_ServiceCallTimeout;// UTILS_DEFAULT_TIMEOUT;
	tRequestHeader.Timestamp   = OpcUa_DateTime_UtcNow();
	tRequestHeader.AuthenticationToken.IdentifierType=OpcUa_IdentifierType_Numeric;
	tRequestHeader.AuthenticationToken.NamespaceIndex=0;
	OpcUa_NodeId* pAuthenticationToken = GetAuthenticationToken();
	if (pAuthenticationToken)
	{
		OpcUa_NodeId_CopyTo(pAuthenticationToken, &(tRequestHeader.AuthenticationToken));
		tRequestHeader.RequestHandle = CClientApplication::m_uiRequestHandle++;
		OpcUa_ViewDescription_Initialize(&tView);
		OpcUa_NodeId_Initialize(&(tView.ViewId));
		tView.ViewId.Identifier.Numeric = 0; // For Entire AddressSpace	

		OpcUa_ResponseHeader_Initialize(&tResponseHeader);
		OpcUa_Channel aChannel = m_pChannel->GetInternalHandle();
		uStatus = OpcUa_ClientApi_Browse(
			aChannel,
			&tRequestHeader,
			&tView, // 
			a_nRequestedMaxReferencesPerNode, // 0 for nolimite
			a_nNoOfNodesToBrowse,
			pNodesToBrowse,
			&tResponseHeader,
			&NoOfResults,
			&pResults,
			&tNoOfDiagnosticInfos,
			&pDiagnosticInfos);
		if ((uStatus == 0) && (tResponseHeader.ServiceResult == 0))
		{
			//OpcUa_ReferenceDescriptionList aReferenceList;
			OpcUa_ByteStringList continuationPoints;
			OpcUa_Int32 ii = 0;
			for (ii = 0; ii < NoOfResults; ii++)
			{
				if (pResults[ii].StatusCode == 0)
				{
					for (int iii = 0; iii < pResults[ii].NoOfReferences; iii++)
					{
						// copy the reference description to be able to release the memory allocated by the server
						OpcUa_ReferenceDescription* pNewReferenceDescription = (OpcUa_ReferenceDescription*)OpcUa_Alloc(sizeof(OpcUa_ReferenceDescription));
						if (pNewReferenceDescription)
						{
							OpcUa_ReferenceDescription_Initialize(pNewReferenceDescription);
							OpcUa_QualifiedName_CopyTo(&(pResults[ii].References[iii].BrowseName), &(pNewReferenceDescription->BrowseName));
							OpcUa_LocalizedText_CopyTo(&(pResults[ii].References[iii].DisplayName), &(pNewReferenceDescription->DisplayName));
							pNewReferenceDescription->IsForward = pResults[ii].References[iii].IsForward;
							pNewReferenceDescription->NodeClass = pResults[ii].References[iii].NodeClass;
							OpcUa_ExpandedNodeId_CopyTo(&(pResults[ii].References[iii].NodeId), &(pNewReferenceDescription->NodeId));
							OpcUa_NodeId_CopyTo(&(pResults[ii].References[iii].ReferenceTypeId), &(pNewReferenceDescription->ReferenceTypeId));
							OpcUa_ExpandedNodeId_CopyTo(&(pResults[ii].References[iii].TypeDefinition), &(pNewReferenceDescription->TypeDefinition));
							pReferenceList->push_back(pNewReferenceDescription);
							// free ressources
							OpcUa_ReferenceDescription_Clear(&(pResults[ii].References[iii]));
							if (pResults[ii].ContinuationPoint.Length > 0)
							{
								OpcUa_ByteString* pByteString = (OpcUa_ByteString*)OpcUa_Alloc(sizeof(OpcUa_ByteString));
								OpcUa_ByteString_Initialize(pByteString);
								OpcUa_ByteString_CopyTo(&(pResults[ii].ContinuationPoint), pByteString);
								continuationPoints.push_back(pByteString);
							}
							//OpcUa_Free(&(pResults[ii].References[iii]));
						}
					}
				}
				OpcUa_BrowseResult_Clear(&pResults[ii]);
				OpcUa_Free(&pResults[ii]);
			}
			//OpcUa_Free(pResults);
			ii = 0;
			OpcUa_ByteStringList::iterator it;
			while (!continuationPoints.empty())
			{
				tRequestHeader.RequestHandle = CClientApplication::m_uiRequestHandle++;
				nNoOfContinuationPoints = 1;
				it = continuationPoints.begin();
				OpcUa_ByteString* pByteString = *it;
				OpcUa_Channel aChannel = m_pChannel->GetInternalHandle();
				uStatus = OpcUa_ClientApi_BrowseNext(
					aChannel,
					&tRequestHeader, FALSE,
					nNoOfContinuationPoints,
					pByteString,
					&tResponseHeader,
					&NoOfResults,
					&pResults,
					&tNoOfDiagnosticInfos,
					&pDiagnosticInfos);
				OpcUa_DiagnosticInfo_Clear(pDiagnosticInfos);
				// release memory allocated for the continuationPoint
				OpcUa_ByteString_Clear(pByteString);
				continuationPoints.erase(it);
			}
		}
		// Send back the ServiceResult to the client
		if ((uStatus == 0) && (tResponseHeader.ServiceResult != 0))
			uStatus = tResponseHeader.ServiceResult;
		OpcUa_RequestHeader_Clear(&tRequestHeader);
		if (pDiagnosticInfos)
		{
			OpcUa_DiagnosticInfo_Clear(pDiagnosticInfos);
			OpcUa_Free(pDiagnosticInfos);
		}
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}

// Function name   : CSessionClient::Read
// Description     : Réalise un lecture synchrone d'attribut dans une Node
// Return type     : OpcUa_StatusCode 
// Argument        : OpcUa_ReadValueIdList ReadValues list des couple (node,attributs) a lire
// Argument        : OpcUa_DataValueList* Results

OpcUa_StatusCode CSessionClient::Read(OpcUa_ReadValueIdList ReadValues,OpcUa_TimestampsToReturn	eTimestampsToReturn,OpcUa_DataValueList* Results)
{
	OpcUa_StatusCode			uStatus=OpcUa_BadNothingToDo;
	OpcUa_RequestHeader			tRequestHeader;
	OpcUa_Double				dblMaxAge=0; // to force the server to read a new value from the DataSource	
	OpcUa_Int32					nbValueToRead;
	OpcUa_ReadValueId*			pNodesToRead=NULL;
	OpcUa_ResponseHeader		tResponseHeader;
	OpcUa_Int32					NoOfResults=0;
	OpcUa_DataValue*			pResults= OpcUa_Null;
	OpcUa_Int32					tNoOfDiagnosticInfos=0;
	OpcUa_DiagnosticInfo*		pDiagnosticInfos=OpcUa_Null;


	nbValueToRead=ReadValues.size();
	if (nbValueToRead > 0)
	{
		pNodesToRead = (OpcUa_ReadValueId*)OpcUa_Alloc(nbValueToRead*sizeof(OpcUa_ReadValueId));
		if (pNodesToRead)
		{
			for (OpcUa_UInt32 ii = 0; ii < ReadValues.size(); ii++)
			{
				OpcUa_ReadValueId* pReadValue = ReadValues.at(ii);
				OpcUa_ReadValueId_Initialize(&pNodesToRead[ii]);
				pNodesToRead[ii].AttributeId = pReadValue->AttributeId; // contient l'attribut à lire pour cet NodeId
				pNodesToRead[ii].DataEncoding = pReadValue->DataEncoding;
				pNodesToRead[ii].IndexRange = pReadValue->IndexRange;
				OpcUa_NodeId_CopyTo(&(pReadValue->NodeId), &(pNodesToRead[ii].NodeId));
			}

			// fill in request header.
			OpcUa_RequestHeader_Initialize(&tRequestHeader);
			tRequestHeader.TimeoutHint = g_ServiceCallTimeout;// UTILS_DEFAULT_TIMEOUT;
			tRequestHeader.Timestamp = OpcUa_DateTime_UtcNow();
			OpcUa_NodeId* pAuthenticationToken = GetAuthenticationToken();
			if (pAuthenticationToken)
			{
				OpcUa_NodeId_CopyTo(pAuthenticationToken, &(tRequestHeader.AuthenticationToken));
				tRequestHeader.RequestHandle = CClientApplication::m_uiRequestHandle++;
				// Initialize response header
				OpcUa_ResponseHeader_Initialize(&tResponseHeader);
				OpcUa_Channel aChannel = m_pChannel->GetInternalHandle();
				uStatus = OpcUa_ClientApi_Read(
					aChannel,
					&tRequestHeader,
					dblMaxAge,
					eTimestampsToReturn,
					nbValueToRead,
					pNodesToRead,
					&tResponseHeader,
					&NoOfResults,
					&pResults,
					&tNoOfDiagnosticInfos,
					&pDiagnosticInfos);
				if (uStatus == OpcUa_Good)
				{
					if (tResponseHeader.ServiceResult == OpcUa_Good)
					{
						for (OpcUa_Int32 iii = 0; iii < NoOfResults; iii++)
						{
							OpcUa_DataValue* pDataValue = (OpcUa_DataValue*)OpcUa_Alloc(sizeof(OpcUa_DataValue));
							OpcUa_DataValue_Initialize(pDataValue);
							OpcUa_DataValue_CopyTo(&pResults[iii], pDataValue);
							Results->push_back(pDataValue);
							OpcUa_DataValue_Clear(&pResults[iii]);
						}
					}
					else
						uStatus = tResponseHeader.ServiceResult;
					if (pResults)
						OpcUa_Free(pResults);
				}
				else
					OpcUa_Trace(OPCUA_TRACE_CLIENT_LEVEL_ERROR, "OpcUa_ClientApi_Read failed 0x%05x\n", uStatus);
				if (pDiagnosticInfos)
					OpcUa_DiagnosticInfo_Clear(pDiagnosticInfos);
				OpcUa_ResponseHeader_Clear(&tResponseHeader);
				OpcUa_NodeId_Clear(&(tRequestHeader.AuthenticationToken));
				OpcUa_RequestHeader_Clear(&tRequestHeader);
			}
			else
				uStatus = OpcUa_BadInvalidArgument;
			for (OpcUa_Int32 i = 0; i < nbValueToRead; i++)
				OpcUa_ReadValueId_Clear(&pNodesToRead[i]);
			OpcUa_Free(pNodesToRead);
		}
		else
			uStatus = OpcUa_BadOutOfMemory;
	}
	return uStatus;
}
OpcUa_StatusCode CSessionClient::GetSubscription(OpcUa_Handle hSubscription,CSubscriptionClient** pSubscription)
{
	OpcUa_StatusCode uStatus=OpcUa_BadInvalidArgument;
	CSubscriptionClient* pLocalSubscription=OpcUa_Null;
	for (OpcUa_UInt32 ii=0;ii<m_SubscriptionList.size();ii++)
	{
		pLocalSubscription=m_SubscriptionList.at(ii);
		if (pLocalSubscription==(CSubscriptionClient*)hSubscription)
		{
			*pSubscription=pLocalSubscription;
			uStatus=OpcUa_Good;
			break;
		}
	}
	return uStatus;
}
// Function name   : CSessionClient::GetSubscription
// Description     : Get a subscripiton from the list of the subscripiton for this session
// Return type     : CSubscription* 
// Argument        : CSubscription* pSubscription
CSubscription* CSessionClient::GetSubscriptionById(OpcUa_UInt32 aSubscriptionId)
{
	CSubscription* pSubscription=OpcUa_Null;
	for (OpcUa_UInt32 ii=0;ii<m_SubscriptionList.size();ii++)
	{
		pSubscription=m_SubscriptionList.at(ii);
		if (pSubscription->GetSubscriptionId()==aSubscriptionId)
			return pSubscription;
	}
	return NULL;
}
OpcUa_StatusCode CSessionClient::DeleteSubscription(CSubscriptionClient* pSubscription)
{
	OpcUa_StatusCode		uStatus=OpcUa_Bad;
	OpcUa_RequestHeader		tRequestHeader;
	OpcUa_ResponseHeader	tResponseHeader;
	OpcUa_UInt32            a_pSubscriptionId;
	OpcUa_Int32				a_nNoOfSubscriptionIds=1; // only one subscription at time
	OpcUa_Int32             nNoOfResults = 0;
	OpcUa_StatusCode*       pResults = 0;
	OpcUa_Int32             nNoOfDiagnosticInfos = 0;
	OpcUa_DiagnosticInfo*   pDiagnosticInfos = 0;

	if (pSubscription)
	{
		
		a_pSubscriptionId=pSubscription->GetSubscriptionId();
		OpcUa_Mutex_Lock(m_SubscriptionListMutex);
		// Check that the pSubscription is in the m_SubscriptionList
		if (GetSubscriptionById(a_pSubscriptionId)==pSubscription)
		{
			// First we need to remove all monitoredItem from this subscription
			uStatus=pSubscription->DeleteMonitoredItems();
			if (uStatus != OpcUa_Good)
				OpcUa_Trace(OPCUA_TRACE_CLIENT_LEVEL_ERROR, "DeleteMonitoredItems failed uStatus=0x%05x\n", uStatus);
			// fill in request header.
			OpcUa_RequestHeader_Initialize(&tRequestHeader);
			tRequestHeader.TimeoutHint = g_ServiceCallTimeout;// UTILS_DEFAULT_TIMEOUT;
			tRequestHeader.Timestamp   = OpcUa_DateTime_UtcNow();
			OpcUa_NodeId* pAuthenticationToken = GetAuthenticationToken();
			OpcUa_NodeId_CopyTo(pAuthenticationToken, &(tRequestHeader.AuthenticationToken));
			tRequestHeader.RequestHandle = CClientApplication::m_uiRequestHandle++;
			OpcUa_ResponseHeader_Initialize(&tResponseHeader);
			OpcUa_Channel aChannel = m_pChannel->GetInternalHandle();
			if (aChannel)
			{
				uStatus = OpcUa_ClientApi_DeleteSubscriptions(aChannel,
					&tRequestHeader,
					a_nNoOfSubscriptionIds,
					&a_pSubscriptionId,
					&tResponseHeader,
					&nNoOfResults,
					&pResults,
					&nNoOfDiagnosticInfos,
					&pDiagnosticInfos);
				if ((uStatus == OpcUa_Good) || (uStatus == OpcUa_BadInvalidState))
				{
					// Supprimer la pSubscription de la m_SubscriptionList 
					uStatus = RemoveSubscription(pSubscription);
				}
				if (pResults)
					OpcUa_Free(pResults);
			}
			else
				uStatus = OpcUa_BadInvalidArgument;
			OpcUa_RequestHeader_Clear(&tRequestHeader);
		}
		else
			uStatus=OpcUa_BadNotFound;
		OpcUa_Mutex_Unlock(m_SubscriptionListMutex);
	}

	return uStatus;
}
OpcUa_StatusCode CSessionClient::CreateSubscription(  OpcUa_Double* dblPublishingInterval, // In/Out param
													  OpcUa_UInt32* uiLifetimeCount, // In/Out param
													  OpcUa_UInt32* uiMaxKeepAliveCount,// In/Out param
													  OpcUa_UInt32 uiMaxNotificationsPerPublish,
													  OpcUa_Boolean bPublishingEnabled,
													  OpcUa_Byte aPriority,
													  OpcUa_Handle* hSubscription)
{
	//HRESULT					hr=S_OK;
	OpcUa_StatusCode		uStatus;
	OpcUa_RequestHeader		tRequestHeader;
	OpcUa_Double            a_nRequestedPublishingInterval;
	OpcUa_UInt32            a_nRequestedLifetimeCount;
	OpcUa_UInt32            a_nRequestedMaxKeepAliveCount;
	OpcUa_UInt32            a_nMaxNotificationsPerPublish;
	OpcUa_Boolean           a_bPublishingEnabled;
	OpcUa_Byte              a_nPriority;
	OpcUa_ResponseHeader	tResponseHeader;
	OpcUa_UInt32            a_pSubscriptionId;
	OpcUa_Double            a_pRevisedPublishingInterval;
	OpcUa_UInt32            a_pRevisedLifetimeCount;
	OpcUa_UInt32            a_pRevisedMaxKeepAliveCount;
	
	// fill in request header.
	OpcUa_RequestHeader_Initialize(&tRequestHeader);
	tRequestHeader.TimeoutHint = g_ServiceCallTimeout;// UTILS_DEFAULT_TIMEOUT;
	tRequestHeader.Timestamp   = OpcUa_DateTime_UtcNow();
	tRequestHeader.AuthenticationToken.IdentifierType=OpcUa_IdentifierType_Numeric;
	tRequestHeader.AuthenticationToken.NamespaceIndex=0;
	OpcUa_NodeId* pNodeId = GetAuthenticationToken();
	OpcUa_NodeId_CopyTo(pNodeId,&(tRequestHeader.AuthenticationToken));
	tRequestHeader.RequestHandle = CClientApplication::m_uiRequestHandle++;
	// RequestedPublishingInterval
	a_nRequestedPublishingInterval=*dblPublishingInterval; // 1 seconde
	a_nRequestedLifetimeCount=*uiLifetimeCount;
	a_nRequestedMaxKeepAliveCount=*uiMaxKeepAliveCount;
	a_nMaxNotificationsPerPublish=uiMaxNotificationsPerPublish; // no limit
	a_bPublishingEnabled=bPublishingEnabled;
	a_nPriority=aPriority; // 0 means not important for the client
	CSubscriptionClient* pNewSubscription=new CSubscriptionClient();
	OpcUa_Channel aChannel = m_pChannel->GetInternalHandle();
	uStatus=OpcUa_ClientApi_CreateSubscription(
								aChannel,
								&tRequestHeader,
								a_nRequestedPublishingInterval,
								a_nRequestedLifetimeCount,
								a_nRequestedMaxKeepAliveCount,a_nMaxNotificationsPerPublish,a_bPublishingEnabled,
								a_nPriority,&tResponseHeader,&a_pSubscriptionId,&a_pRevisedPublishingInterval,
								&a_pRevisedLifetimeCount,&a_pRevisedMaxKeepAliveCount);

	if (OpcUa_IsNotBad(uStatus))
	{
		*dblPublishingInterval=a_pRevisedPublishingInterval;
		*uiLifetimeCount=a_pRevisedLifetimeCount;
		*uiMaxKeepAliveCount=a_pRevisedMaxKeepAliveCount;
		// 
		pNewSubscription->SetSession(this);
		pNewSubscription->SetPriority(a_nPriority);
		pNewSubscription->SetMaxNotificationsPerPublish(a_nMaxNotificationsPerPublish);
		pNewSubscription->SetSubscriptionId(a_pSubscriptionId);
		pNewSubscription->SetPublishingInterval(a_pRevisedPublishingInterval);
		pNewSubscription->SetLifetimeCount(a_pRevisedLifetimeCount);
		pNewSubscription->SetMaxKeepAliveCount(a_pRevisedMaxKeepAliveCount);
		pNewSubscription->SetChannel(m_pChannel);
		pNewSubscription->SetHandle((OpcUa_Handle)pNewSubscription);
		pNewSubscription->SetPublishingEnabled(a_bPublishingEnabled);
		m_SubscriptionList.push_back(pNewSubscription);
		//
		*hSubscription=(OpcUa_Handle)pNewSubscription;
		m_wSessionState =SESSION_STATE_SUBSCRIBED;

	}
	OpcUa_RequestHeader_Clear(&tRequestHeader);
	return uStatus;
}

OpcUa_StatusCode CSessionClient::FindSubscription(OpcUa_UInt32 aSubscriptionId, CSubscriptionClient** ppSubscription)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	if (m_SubscriptionList.empty())
		uStatus=OpcUa_BadSubscriptionIdInvalid;
	else
	{
		for (OpcUa_UInt32 ii=0;ii<m_SubscriptionList.size();ii++)
		{
				CSubscriptionClient* pSubscription=m_SubscriptionList.at(ii);
				if (pSubscription->GetSubscriptionId()==aSubscriptionId)
				{
					*ppSubscription=pSubscription;
					return OpcUa_Good;
				}
		}
		uStatus=OpcUa_BadSubscriptionIdInvalid;
	}
	return uStatus;
}
OpcUa_StatusCode CSessionClient::FindSubscription(OpcUa_Handle hSubscription, CSubscriptionClient** ppSubscription)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	if (m_SubscriptionList.empty())
		uStatus = OpcUa_BadSubscriptionIdInvalid;
	else
	{
		for (OpcUa_UInt32 ii = 0; ii<m_SubscriptionList.size(); ii++)
		{
			CSubscriptionClient* pSubscription = m_SubscriptionList.at(ii);
			if (pSubscription->GetHandle() == hSubscription)
			{
				*ppSubscription = pSubscription;
				return OpcUa_Good;
			}
		}
		uStatus = OpcUa_BadSubscriptionIdInvalid;
	}
	return uStatus;
}
OpcUa_StatusCode CSessionClient::StartWatchingThread()
{
	OpcUa_StatusCode uStatus=OpcUa_Good;	
	if (m_hInternalWatchingThread==NULL)
	{
		m_bRunWatchingThread=TRUE;
		OpcUa_StatusCode uStatus  = OpcUa_Thread_Create(&m_hInternalWatchingThread,(CSessionClient::WatchingThread),this);

		if(uStatus != OpcUa_Good)
			OpcUa_Trace(OPCUA_TRACE_CLIENT_LEVEL_ERROR,"Create InternalWatchingThread Failed");
		else
			OpcUa_Thread_Start(m_hInternalWatchingThread);
	}
	return uStatus;
}
OpcUa_StatusCode CSessionClient::StopWatchingThread()
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	if (m_bRunWatchingThread)
	{
		if ((m_hInternalWatchingThread) && (!m_bFromWatchingThread))
		{
			m_bRunWatchingThread = OpcUa_False;
			OpcUa_Semaphore_Post(m_WatchingSem, 1);
			uStatus = OpcUa_Semaphore_TimedWait(m_WatchingSem, 15000); // 15 secondes max.
			if (uStatus == OpcUa_GoodNonCriticalTimeout)
			{
				// on force la fin du thread de simulation
				OpcUa_Trace(OPCUA_TRACE_CLIENT_LEVEL_ERROR, "Impossible to stop the InternalWatchingThread. Timeout");
				//OpcUa_Thread_Delete(&m_hInternalWatchingThread);
				uStatus = OpcUa_Bad;
			}
			else
			{
				OpcUa_Thread_Delete(m_hInternalWatchingThread);
				OpcUa_Trace(OPCUA_TRACE_CLIENT_LEVEL_ERROR, "InternalWatchingThread stopped properly\n");
			}
		}
		//else
		//	OpcUa_Thread_Delete(m_hInternalWatchingThread);
	}
	else
		OpcUa_Trace(OPCUA_TRACE_CLIENT_LEVEL_ERROR, "InternalWatchingThread was already stopped\n");
	return uStatus;
}
/// <summary>
/// Watchings the thread.
// Thread for watching the internal availability of of the client/server connection
// To do it. The OpenOpcUaClientLib read the Value of CurrentTime node (2258)
// If the read failed then the API will  notifiy the HostApplication.
// This thread also watch the ServerStatus Node. If the server statuChange the OpenOpcUaClientLib will notifiy the HostApplication
/// </summary>
/// <param name="arg">The argument.</param>
void CSessionClient::WatchingThread(LPVOID arg)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	CSessionClient* pSession=(CSessionClient*)arg;
	OpcUa_StatusCode uStatusLastNotified=OpcUa_Bad; // initialement on provoque une discordance afin qu'une notification initiale soit réalisée
	OpcUa_UInt32 uiTimeout = 5000;
	if (pSession)
	{
		while (pSession->m_bRunWatchingThread)
		{
			if (pSession)
			{
				// Let's try to read the currentTime on the server
				switch (pSession->m_wSessionState)
				{
				case SESSION_STATE_ON_ACTIVE:
				case SESSION_STATE_SUBSCRIBED:
				{
					OpcUa_DataValue* pResults = OpcUa_Null;
					OpcUa_ReadValueId* pNodesToRead = OpcUa_Null;
					OpcUa_Int32 iNodNodesToRead = 0;
					OpcUa_NodeId aNodeId;
					aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
					aNodeId.Identifier.Numeric = 2258; // CurrentTime
					aNodeId.NamespaceIndex = 0;

					iNodNodesToRead = 1;
					pNodesToRead = (OpcUa_ReadValueId*)OpcUa_Alloc(iNodNodesToRead*sizeof(OpcUa_ReadValueId));
					OpcUa_ReadValueId_Initialize(&pNodesToRead[0]);
					pNodesToRead[0].AttributeId = OpcUa_Attributes_Value;
					pNodesToRead[0].NodeId = aNodeId;
					OpcUa_Handle hSession = (OpcUa_Handle)pSession;
					// notify last status
					if (pSession->NotifyStatusChange(uStatusLastNotified) == OpcUa_Good)
						uStatusLastNotified = pSession->GetInternalServerStatus();
					// Read attribute (value)
					uStatus = OpenOpcUa_ReadAttributes(
						(OpcUa_Handle)pSession->m_pApplication,
						hSession,
						OpcUa_TimestampsToReturn_Both,
						iNodNodesToRead,
						pNodesToRead,
						&pResults);
					// si la lecture aboutie on attends
					if (uStatus == OpcUa_Good)
					{
						if (pResults)
						{
							OpcUa_DataValue_Clear(pResults);
							OpcUa_Free(pResults);
							pResults = OpcUa_Null;
						}
					}
					else
					{
						pSession->SetInternalServerStatus(uStatus);
						void* pCallbackData = pSession->GetShutdownCallbackData();
						PFUNCSHUTDOWN pFuncShutdown = pSession->GetShutdownCallback();
						if (pFuncShutdown)
						{
							OpcUa_CharA* buf = (OpcUa_CharA*)OpcUa_Alloc(15);
							ZeroMemory(buf, 15);
							sprintf(buf, "0x%05x", (unsigned int)uStatus);
							OpcUa_String strMessage;
							OpcUa_String_Initialize(&strMessage);
							OpcUa_String_AttachCopy(&strMessage, buf);
							uStatus = OpcUa_BadTimeout;
							//// Let's stop the watching thread
							//pSession->m_bRunWatchingThread = OpcUa_False;
							// Call shutdown delegate with a SessionTimeout message
							pSession->m_bFromWatchingThread = OpcUa_True;
							pSession->m_bRunWatchingThread = OpcUa_False;
							pFuncShutdown(
								(OpcUa_Handle)pSession->m_pApplication,
								hSession,
								strMessage, (void*)pCallbackData);
							OpcUa_Trace(OPCUA_TRACE_CLIENT_ERROR, "Shutdown sent to the client host application. OpenOpcUa_ReadAttributes failed %s\n", buf);
							OpcUa_String_Clear(&strMessage);
							OpcUa_Free(buf);
						}
					}
					OpcUa_ReadValueId_Clear(&pNodesToRead[0]);
					OpcUa_Free(pNodesToRead);
					if (pSession->m_nSessionTimeout<uiTimeout)
						uiTimeout = (OpcUa_UInt32)pSession->m_nSessionTimeout;
					// Limit the timeout 
					if (uiTimeout == 0)
						uiTimeout = 5000;
					//if (pSession->NotifyStatusChange(uStatusLastNotified) == OpcUa_Good)
					//	uStatusLastNotified = pSession->GetInternalServerStatus();
				}
				break;
				case SESSION_STATE_ON_INACTIVE:
				{

				}
				break;
				case SESSION_STATE_UNDETERMINED:
					break;
				default:
					break;
				}
				/*
				if ((pSession->m_wSessionState&SESSION_STATE_ON_ACTIVE) == SESSION_STATE_ON_ACTIVE)
				{
					OpcUa_DataValue* pResults = OpcUa_Null;
					OpcUa_ReadValueId* pNodesToRead = OpcUa_Null;
					OpcUa_Int32 iNodNodesToRead = 0;
					OpcUa_NodeId aNodeId;
					aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
					aNodeId.Identifier.Numeric = 2258; // CurrentTime
					aNodeId.NamespaceIndex = 0;

					iNodNodesToRead = 1;
					pNodesToRead = (OpcUa_ReadValueId*)OpcUa_Alloc(iNodNodesToRead*sizeof(OpcUa_ReadValueId));
					OpcUa_ReadValueId_Initialize(&pNodesToRead[0]);
					pNodesToRead[0].AttributeId = OpcUa_Attributes_Value;
					pNodesToRead[0].NodeId = aNodeId;
					OpcUa_Handle hSession = (OpcUa_Handle)pSession;
					uStatus = OpenOpcUa_ReadAttributes(
						(OpcUa_Handle)pSession->m_pApplication,
						hSession,
						OpcUa_TimestampsToReturn_Both,
						iNodNodesToRead,
						pNodesToRead,
						&pResults);
					// si la lecture aboutie on attends
					if (uStatus == OpcUa_Good)
					{
						OpcUa_Free(pResults);
					}
					else
					{
						uStatus = OpcUa_BadTimeout;
						PFUNCSHUTDOWN pFuncShutdown = pSession->GetShutdownCallback();
						if (pFuncShutdown)
						{
							OpcUa_String strMessage;
							OpcUa_String_Initialize(&strMessage);
							OpcUa_String_AttachCopy(&strMessage, "Session Timeout");
							uStatus = OpcUa_BadTimeout;
							// Call shutdown delegate with a SessionTimeout message
							pFuncShutdown(
								(OpcUa_Handle)pSession->m_pApplication,
								hSession,
								strMessage, (void*)uStatus);
							OpcUa_String_Clear(&strMessage);
						}
					}
					OpcUa_ReadValueId_Clear(&pNodesToRead[0]);
					OpcUa_Free(pNodesToRead);
				}
				else
				{
					if ((pSession->m_wSessionState&SESSION_STATE_ON_INACTIVE) == SESSION_STATE_ON_INACTIVE)
					{

					}
				}
				*/
				// Let's try to check if the ServerStatus changed

				/*
				if (pSession->GetInternalServerStatus() != uStatusLastNotified)
				{
					uStatus = OpcUa_BadTimeout;
					PFUNCSHUTDOWN pFuncShutdown = pSession->GetShutdownCallback();
					if (pFuncShutdown)
					{
						uiTimeout = (OpcUa_UInt32)pSession->m_nSessionTimeout;
						uStatusLastNotified = pSession->GetInternalServerStatus();
						OpcUa_CharA* buf = (OpcUa_CharA*)OpcUa_Alloc(15);
						ZeroMemory(buf, 15);
						OpcUa_String strMessage;
						OpcUa_String_Initialize(&strMessage);
						OpcUa_String_AttachCopy(&strMessage, "ServerStatus Changed");
						sprintf(buf, "0x%05x", (OpcUa_UInt32)uStatusLastNotified);
						OpcUa_String aString;
						OpcUa_String_Initialize(&aString);
						OpcUa_String_AttachCopy(&aString, buf);
						OpcUa_Free(buf);
						OpcUa_String_StrnCat(&strMessage, &aString, OpcUa_String_StrLen(&aString));
						OpcUa_Handle hSession = (OpcUa_Handle)pSession;
						// Call shutdown delegate with a ServerStatus Changed message
						PFUNCSHUTDOWN pFuncShutdown = pSession->GetShutdownCallback();
						if (pFuncShutdown)
						{
							pFuncShutdown(
								(OpcUa_Handle)pSession->m_pApplication,
								hSession,
								strMessage, (void*)uStatusLastNotified);
						}
						OpcUa_String_Clear(&strMessage);
					}
				}
				*/
				// Let's check the internal status of the client/server connection
				if (!(pSession->m_bRunWatchingThread))
				{
					uStatus = pSession->GetInternalServerStatus();
					if ((uStatus != OpcUa_Good) && (uStatus != OpcUa_BadTooManyPublishRequests))
					{
						//uStatus = OpcUa_BadTimeout;
						void* pCallbackData = pSession->GetShutdownCallbackData();
						PFUNCSHUTDOWN pFuncShutdown = pSession->GetShutdownCallback();
						if (pFuncShutdown)
						{
							OpcUa_CharA* buf = (OpcUa_CharA*)OpcUa_Alloc(15);
							ZeroMemory(buf, 15);
							sprintf(buf, "0x%05x", (unsigned int)uStatus);
							OpcUa_Handle hSession = (OpcUa_Handle)pSession;
							OpcUa_String strMessage;
							OpcUa_String_Initialize(&strMessage);
							OpcUa_String_AttachCopy(&strMessage, buf);
							// Call shutdown delegate with a SessionTimeout message
							pFuncShutdown(
								(OpcUa_Handle)pSession->m_pApplication,
								hSession,
								strMessage, (void*)pCallbackData);
							OpcUa_String_Clear(&strMessage);
							OpcUa_Trace(OPCUA_TRACE_CLIENT_ERROR, "Shutdown sent to the client host application. InternalServerStatus changed %s\n", buf);
							OpcUa_Free(buf);
						}
					}
				}
			}
			if ((pSession) && (pSession->m_bRunWatchingThread) )
			{
				// Mise en attente
				OpcUa_Semaphore_TimedWait(pSession->m_WatchingSem, uiTimeout); //pSession->m_nSessionTimeout
			}
		}
		OpcUa_Trace(OPCUA_TRACE_CLIENT_LEVEL_ERROR, "InternalWatchingThread stopped\n");
		OpcUa_Semaphore_Post(pSession->m_WatchingSem, 1);
	}
}
OpcUa_StatusCode CSessionClient::NotifyStatusChange(OpcUa_StatusCode uLastNotifiedStatus)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	// Let's try to check if the ServerStatus changed
	if (GetInternalServerStatus() != uLastNotifiedStatus)
	{
		uStatus = OpcUa_BadTimeout;
		PFUNCSHUTDOWN pFuncShutdown = GetShutdownCallback();
		if (pFuncShutdown)
		{
			uLastNotifiedStatus = GetInternalServerStatus();
			OpcUa_CharA* buf = (OpcUa_CharA*)OpcUa_Alloc(15);
			ZeroMemory(buf, 15);
			sprintf(buf, "0x%05x", (unsigned int)uLastNotifiedStatus);
			OpcUa_String strMessage;
			OpcUa_String_Initialize(&strMessage);

			OpcUa_String_AttachCopy(&strMessage, buf);
			OpcUa_Free(buf);
			//OpcUa_String_StrnCat(&strMessage, &aString, OpcUa_String_StrLen(&aString));
			OpcUa_Handle hSession = (OpcUa_Handle)this;
			// Call shutdown delegate with a ServerStatus Changed message
			void* pCallbackData = GetShutdownCallbackData();
			uStatus=pFuncShutdown(
				(OpcUa_Handle)m_pApplication,
				hSession,
				strMessage, (void*)pCallbackData);
			OpcUa_Trace(OPCUA_TRACE_CLIENT_ERROR, "NotifyStatusChange. Last Status is 0x%05x\n", uLastNotifiedStatus);
			OpcUa_String_Clear(&strMessage);
		}
	}
	return uStatus;
}
OpcUa_StatusCode CSessionClient::DeleteAllSubscriptions()
{	
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_RequestHeader		tRequestHeader;
	OpcUa_ResponseHeader	tResponseHeader;
	OpcUa_UInt32*           pSubscriptionId = OpcUa_Null;
	OpcUa_Int32				a_nNoOfSubscriptionIds = 1; // only one subscription at time
	OpcUa_Int32             nNoOfResults = 0;
	OpcUa_StatusCode*       pResults = 0;
	OpcUa_Int32             nNoOfDiagnosticInfos = 0;
	OpcUa_DiagnosticInfo*   pDiagnosticInfos = 0;

	OpcUa_Mutex_Lock(m_SubscriptionListMutex);
	a_nNoOfSubscriptionIds = m_SubscriptionList.size();
	if (a_nNoOfSubscriptionIds>0)
	{
		pSubscriptionId = (OpcUa_UInt32*)OpcUa_Alloc(sizeof(OpcUa_UInt32)*a_nNoOfSubscriptionIds);
		if (pSubscriptionId)
		{
			for (OpcUa_Int32 i = 0; i < a_nNoOfSubscriptionIds; i++)
			{
				CSubscriptionClient* pSubscription = m_SubscriptionList.at(i);
				uStatus = pSubscription->DeleteMonitoredItems();
				if (uStatus != OpcUa_Good)
					OpcUa_Trace(OPCUA_TRACE_CLIENT_LEVEL_ERROR, "DeleteMonitoredItems failed uStatus=0x%05x\n", uStatus);
				pSubscriptionId[i] = pSubscription->GetSubscriptionId();;
			}

			// fill in request header.
			OpcUa_RequestHeader_Initialize(&tRequestHeader);
			tRequestHeader.TimeoutHint = CLIENTLIB_DEFAULT_TIMEOUT;
			tRequestHeader.Timestamp = OpcUa_DateTime_UtcNow();
			OpcUa_NodeId* pAuthenticationToken = GetAuthenticationToken();
			OpcUa_NodeId_CopyTo(pAuthenticationToken, &(tRequestHeader.AuthenticationToken));
			tRequestHeader.RequestHandle = CClientApplication::m_uiRequestHandle++;
			OpcUa_ResponseHeader_Initialize(&tResponseHeader);
			OpcUa_Channel aChannel = m_pChannel->GetInternalHandle();
			if (aChannel)
			{
				uStatus = OpcUa_ClientApi_DeleteSubscriptions(aChannel,
					&tRequestHeader,
					a_nNoOfSubscriptionIds,
					pSubscriptionId,
					&tResponseHeader,
					&nNoOfResults,
					&pResults,
					&nNoOfDiagnosticInfos,
					&pDiagnosticInfos);
				if ((uStatus == OpcUa_Good) || (uStatus == OpcUa_BadInvalidState))
				{
					if (nNoOfResults == m_SubscriptionList.size())
					{
						for (OpcUa_Int32 i = 0; i < m_SubscriptionList.size(); i++)
						{
							if (pResults[i] == OpcUa_Good)
							{
								CSubscriptionClient* pSubscription = m_SubscriptionList.at(i);
								if (pSubscription)
								{
									// Supprimer la pSubscription de la m_SubscriptionList 
									uStatus = RemoveSubscription(pSubscription);
								}
							}
						}
					}
				}
			}
			else
				uStatus = OpcUa_BadInvalidArgument;
			OpcUa_RequestHeader_Clear(&tRequestHeader);
			OpcUa_Free(pSubscriptionId);
		}
	}
	OpcUa_Mutex_Unlock(m_SubscriptionListMutex);
	return uStatus;
}

///////////////////////////////////////////////
// Publishing mecanism thread
//

void CSessionClient::StartPublishingThread()
{
	if (m_hPublishingThread==NULL)
	{
		m_bRunPublishingThread=TRUE;
		OpcUa_StatusCode uStatus  = OpcUa_Thread_Create(&m_hPublishingThread,PublishingThread,this);

		if(uStatus != OpcUa_Good)
			OpcUa_Trace(OPCUA_TRACE_CLIENT_LEVEL_ERROR,"Create PublishingThread Failed\n");
		else
			OpcUa_Thread_Start(m_hPublishingThread);
	}
}
// Thread en charge du publish.
// Elle traite la demande et transmet le publish. 
// la réponse aux publish sera recu de manière asynchrone par la thread PublishAsyncCallback
void CSessionClient::PublishingThread(LPVOID arg)
{
	DWORD dwSleepTime;
	DWORD dwStart=0;
	DWORD dwInterval=0;
	CSessionClient* pSessionClient=(CSessionClient*)arg;

	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_Double dblInterval=1000.0;
	// On attend que la session soit effectivement créée, activée ou terminée
	OpcUa_Semaphore_TimedWait(pSessionClient->m_hStopPublishingThread, INFINITE);
	PublishAsyncCallbackData* pData = OpcUa_Null; // (PublishAsyncCallbackData*)OpcUa_Alloc(sizeof(PublishAsyncCallbackData));
	/*
	ZeroMemory(pData,sizeof(PublishAsyncCallbackData));
	pData->pClientSession=pSessionClient;
	*/
	while (pSessionClient->m_bRunPublishingThread)
	{
		dwStart=GetTickCount();
		OpcUa_RequestHeader tRequestHeader;
		OpcUa_RequestHeader_Initialize(&tRequestHeader);
		tRequestHeader.TimeoutHint = g_ServiceCallTimeout;// UTILS_DEFAULT_TIMEOUT;
		tRequestHeader.Timestamp   = OpcUa_DateTime_UtcNow();
		tRequestHeader.RequestHandle = CClientApplication::m_uiRequestHandle++;
		OpcUa_NodeId* pNodeId=pSessionClient->GetAuthenticationToken();
		if (pNodeId)
		{
			OpcUa_NodeId_CopyTo(pNodeId, &(tRequestHeader.AuthenticationToken));

			OpcUa_SubscriptionAcknowledgement* tSubscriptionAcknowledgments=OpcUa_Null; // strucutre décrivant la souscription à acquitter

			OpcUa_ResponseHeader tResponseHeader;
			OpcUa_ResponseHeader_Initialize(&tResponseHeader);
			// Ok on va traiter l'ensemble des souscription de cette session
			CSubscriptionClient* pSubscription=OpcUa_Null;

			// début NEW code
			OpcUa_Int32 iNoOfAvailableSequenceNumbers=0;
			OpcUa_Mutex_Lock(pSessionClient->m_SubscriptionListMutex);	
			OpcUa_StatusCode internalStatus=pSessionClient->GetInternalServerStatus();
			if (internalStatus == OpcUa_Good)
			{
				if (pSessionClient->GetPendingPublishSize() <= (pSessionClient->m_SubscriptionList.size() + (pSessionClient->m_AdjustPublishVal )))
				{
					for (OpcUa_UInt32 ii = 0; ii < pSessionClient->m_SubscriptionList.size(); ii++)
					{
						tSubscriptionAcknowledgments = OpcUa_Null;
						pSubscription = pSessionClient->m_SubscriptionList.at(ii);
						if (pSubscription)
						{
							iNoOfAvailableSequenceNumbers = pSubscription->GetNoOfAvailableSequenceNumbers();
							if (iNoOfAvailableSequenceNumbers>0)
							{
								tSubscriptionAcknowledgments =
									(OpcUa_SubscriptionAcknowledgement*)OpcUa_Alloc(iNoOfAvailableSequenceNumbers*sizeof(OpcUa_SubscriptionAcknowledgement));
								if (tSubscriptionAcknowledgments)
								{
									OpcUa_Mutex_Lock(pSubscription->GetMonitoredItemListMutex());
									OpcUa_UInt32* uiAvailableSequenceNumbers = pSubscription->GetAvailableSequenceNumbers();
									for (OpcUa_Int32 iii = 0; iii < iNoOfAvailableSequenceNumbers; iii++)
									{
										tSubscriptionAcknowledgments[iii].SubscriptionId = pSubscription->GetSubscriptionId();
										tSubscriptionAcknowledgments[iii].SequenceNumber = uiAvailableSequenceNumbers[iii];
									}
									OpcUa_Mutex_Unlock(pSubscription->GetMonitoredItemListMutex());
								}
							}
						}
						// Transmission du publish au client
						CChannel* pChannel = pSessionClient->GetChannel();
						if (pChannel)
						{
							// in order to not overload servers we just send one publish at the time.
							// The fact is that we don't need more publish.
							// one is enough
							//if (pSessionClient->GetPendingPublish()==0)
							{
								// version asynchrone
								if (pSessionClient->m_internalPublishStatus == OpcUa_Good)
								{
									//for (OpcUa_UInt32 iii = 0; iii < pSessionClient->m_AdjustPublishVal+1; iii++)
									{
										// regular publish
										CPendingPublish* pRegularPendingPublish = new CPendingPublish(tRequestHeader.TimeoutHint);
										pSessionClient->AddPendingPublish(pRegularPendingPublish);
										pData = (PublishAsyncCallbackData*)OpcUa_Alloc(sizeof(PublishAsyncCallbackData));
										pData->pClientSession = pSessionClient;
										pData->pPendingPublish = pRegularPendingPublish;
										tRequestHeader.RequestHandle = CClientApplication::m_uiRequestHandle++;
										uStatus = OpcUa_ClientApi_BeginPublish(pChannel->GetInternalHandle(),
											&tRequestHeader,
											iNoOfAvailableSequenceNumbers, // nombre de structure SubscriptionAcknowledgment
											tSubscriptionAcknowledgments, //Structure contenant les couples subscriptionId/sequences a acquitter
											PublishAsyncCallback, // fonction callback
											pData);	// paramètres de la fonction callback

										if (uStatus != OpcUa_Good)
										{
											// The Publish failed. So we consider the Client/Server in Shutdown. 
											// We post this to the hostApplication
											OpcUa_String strMessage;
											OpcUa_String_Initialize(&strMessage);
											if ((uStatus == OpcUa_BadSessionClosed) /*|| (uStatus == OpcUa_BadInvalidState)*/)  // 0x80260000 - 0x80AF0000
											{
												pSessionClient->StopPublishingThread();//m_bRunPublishingThread=OpcUa_False;
												if (pSessionClient)
												{
													uStatus = OpcUa_BadShutdown;
													OpcUa_CharA* buf = (OpcUa_CharA*)OpcUa_Alloc(15);
													ZeroMemory(buf, 15);
													sprintf(buf, "0x%05x", (unsigned int)uStatus);
													OpcUa_String_AttachCopy(&strMessage, buf);
													void* pCallbackData = pSessionClient->GetShutdownCallbackData();
													PFUNCSHUTDOWN pFuncShutdown = pSessionClient->GetShutdownCallback();
													if (pFuncShutdown)
														pFuncShutdown(
														pSessionClient->GetApplicationHandle(),
														(OpcUa_Handle)pSessionClient,
														strMessage, (void*)pCallbackData);
													OpcUa_Free(buf);
												}
											}
											else
											{
												if (uStatus == OpcUa_BadInvalidState)
													pSessionClient->m_bRunPublishingThread = OpcUa_False;//pSessionClient->StopPublishingThread();
												if (uStatus == OpcUa_BadTooManyPublishRequests)
												{
													// The current Sequence number was rejected by the session
													// let adjust the it.
												}
												if (pSessionClient)
												{
													OpcUa_CharA* buf = (OpcUa_CharA*)OpcUa_Alloc(15);
													ZeroMemory(buf, 15);
													sprintf(buf, "0x%05x", (unsigned int)uStatus);
													OpcUa_String_AttachCopy(&strMessage, buf);
													OpcUa_String_AttachCopy(&strMessage, buf);
													void* pCallbackData = pSessionClient->GetShutdownCallbackData();
													PFUNCSHUTDOWN pFuncShutdown = pSessionClient->GetShutdownCallback();
													if (pFuncShutdown)
														pFuncShutdown(
														pSessionClient->GetApplicationHandle(),
														(OpcUa_Handle)pSessionClient,
														strMessage, (void*)pCallbackData);
													OpcUa_Free(buf);
												}
											}
											OpcUa_String_Clear(&strMessage);
											delete pRegularPendingPublish;
											OpcUa_Free(pData);
											// We need to leave from here 
											//return; // This return is removed in the 1.0.4.0 RC6
										}
										else
										{ 
											//pSessionClient->IncPendingPublish();
											pSubscription->SequenceNumberClearAll();
											if (pSessionClient->GetPendingPublish() <= pSessionClient->m_SubscriptionList.size()+1)
											{
												CPendingPublish* pStartupPendingPublish = new CPendingPublish(tRequestHeader.TimeoutHint);
												pSessionClient->AddPendingPublish(pStartupPendingPublish);
												pData = (PublishAsyncCallbackData*)OpcUa_Alloc(sizeof(PublishAsyncCallbackData));
												pData->pClientSession = pSessionClient;
												pData->pPendingPublish = pStartupPendingPublish;
												tRequestHeader.RequestHandle = CClientApplication::m_uiRequestHandle++;
												// publish to accomodate the startup
												uStatus = OpcUa_ClientApi_BeginPublish(pChannel->GetInternalHandle(),
													&tRequestHeader,
													0, // nombre de structure SubscriptionAcknowledgment
													OpcUa_Null, //Structure contenant les couples subscriptionId/sequences a acquitter
													PublishAsyncCallback, // fonction callback
													pData);	// paramètres de la fonction callback

												if (uStatus == OpcUa_Good)
												{
													pSessionClient->IncPendingPublish();
												}
												else
												{
													delete pStartupPendingPublish;
													OpcUa_Free(pData);
												}
											}
											// 
										}
										OpcUa_Thread_Sleep(10);
									}
								}
								else
									OpcUa_Trace(OPCUA_TRACE_CLIENT_ERROR, "Internal error in the PublishingThread. Session status is 0x%05x\n", pSessionClient->m_internalPublishStatus);
							}
							//if (tSubscriptionAcknowledgments)
							//{
							//	OpcUa_SubscriptionAcknowledgement_Clear(tSubscriptionAcknowledgments);
							//	OpcUa_Free(tSubscriptionAcknowledgments);
							//	tSubscriptionAcknowledgments=OpcUa_Null;
							//}
						}
						else
							OpcUa_Trace(OPCUA_TRACE_CLIENT_ERROR, "Internal error in the PublishingThread. SecureChannel is broken\n");
						if (tSubscriptionAcknowledgments)
						{
							OpcUa_SubscriptionAcknowledgement_Clear(tSubscriptionAcknowledgments);
							OpcUa_Free(tSubscriptionAcknowledgments);
							tSubscriptionAcknowledgments = OpcUa_Null;
						}
					}
				}
			}
			OpcUa_Mutex_Unlock(pSessionClient->m_SubscriptionListMutex);
		
		}

		OpcUa_RequestHeader_Clear(&tRequestHeader);
		// Fin de la region relative a l'appel de OpcUa_ClientApi_Publish
		if (pSessionClient->GetFastestPublishInterval(&dblInterval)==OpcUa_Good)
			dwInterval=(DWORD)(dblInterval);
		else
			dwInterval=1000;
		DWORD dwEnd=GetTickCount();
		DWORD dwCountedTime=dwEnd-dwStart;
		if (dwInterval>dwCountedTime)
			dwSleepTime=dwInterval-dwCountedTime;
		else
			dwSleepTime=0;
		// 
		if (dwSleepTime==0)
			dwSleepTime=(DWORD)dblInterval;
		OpcUa_Semaphore_TimedWait(pSessionClient->m_hStopPublishingThread, dwSleepTime);
	}
	/*
	OpcUa_Free(pData);
	pData = OpcUa_Null;
	*/
	OpcUa_Semaphore_Post(pSessionClient->m_hStopPublishingThread, 1);
	OpcUa_Trace(OPCUA_TRACE_CLIENT_LEVEL_ERROR, "PublishingThread stopped\n");
}
OpcUa_StatusCode CSessionClient::StopPublishingThread()
{
	m_bRunPublishingThread = FALSE; 
	OpcUa_StatusCode uStatus = OpcUa_Good;
	if (m_hPublishingThread)
	{
		OpcUa_Semaphore_Post(m_hStopPublishingThread, 1);
		uStatus = OpcUa_Semaphore_TimedWait(m_hStopPublishingThread, OPC_TIMEOUT * 2); // 15 secondes max.
		if (uStatus == OpcUa_GoodNonCriticalTimeout)
		{
			// on force la fin du thread de simulation
			OpcUa_Trace(OPCUA_TRACE_CLIENT_LEVEL_ERROR, "Impossible to stop the PublishingThread. Timeout\n");
			//OpcUa_Thread_Delete(&m_hPublishingThread);
		}
		else
		{
			OpcUa_Thread_Delete(m_hPublishingThread);
			m_hPublishingThread = OpcUa_Null;
			OpcUa_Trace(OPCUA_TRACE_CLIENT_LEVEL_ERROR, "PublishingThread stopped properly\n");
		}
	}
	return uStatus;
}
OpcUa_StatusCode CSessionClient::GetFastestPublishInterval(OpcUa_Double* dblFastestInterval)
{
	OpcUa_StatusCode uStatus=OpcUa_BadNotFound;
	if (dblFastestInterval)
	{
		OpcUa_Mutex_Lock(m_SubscriptionListMutex);	
		OpcUa_Double dblFastest=65535.00;
		for (OpcUa_UInt32 ii=0;ii<m_SubscriptionList.size();ii++)
		{
			CSubscriptionClient* pSubscriptionClient=m_SubscriptionList.at(ii);
			if (dblFastest>pSubscriptionClient->GetPublishingInterval())
			{
				(*dblFastestInterval)=pSubscriptionClient->GetPublishingInterval();
				dblFastest=(*dblFastestInterval);
				uStatus=OpcUa_Good;
			}
		}
		OpcUa_Mutex_Unlock(m_SubscriptionListMutex);	
	}
	else
		uStatus=OpcUa_BadInvalidArgument;
	return uStatus;
}

OpcUa_Handle CSessionClient::GetApplicationHandle() 
{ 
	return (OpcUa_Handle)m_pApplication; 
}

OpcUa_StatusCode CSessionClient::RemoveSubscription(CSubscriptionClient* pSubscription)
{
	OpcUa_StatusCode uStatus = OpcUa_BadNotFound;
	if (pSubscription)
	{
		//OpcUa_Mutex_Lock(m_SubscriptionListMutex);
		for (CSubscriptionList::iterator it = m_SubscriptionList.begin(); it != m_SubscriptionList.end(); it++)
		{
			if (*it == pSubscription)
			{
				delete pSubscription;
				m_SubscriptionList.erase(it);
				uStatus = OpcUa_Good;
				break;
			}
		}
		//OpcUa_Mutex_Unlock(m_SubscriptionListMutex);
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}

OpcUa_StatusCode CSessionClient::GetXmlAttributes(vector<OpcUa_String*>* szAttributes/*OpcUa_CharA** atts*/)
{
	OpcUa_StatusCode uStatus = OpcUa_BadNotFound;
	OpcUa_String* pszLocalAtts=OpcUa_Null;
	OpcUa_String szTmpstring;
	OpcUa_String_Initialize(&szTmpstring);
	OpcUa_String szTmpstring1;
	OpcUa_String_Initialize(&szTmpstring1);

	// 
	//vector<OpcUa_String*> szAttributes;
	CEndpointDescription* pEndpointDescription = GetEndpointDescription();
	// EndpointUrl
	// Attribute
	OpcUa_String_AttachCopy(&szTmpstring, "EndpointUrl");
	pszLocalAtts = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszLocalAtts);
	OpcUa_String_StrnCpy(pszLocalAtts, &szTmpstring, OpcUa_String_StrLen(&szTmpstring));
	szAttributes->push_back(pszLocalAtts);
	// Attribute value
	OpcUa_String_Clear(&szTmpstring);
	OpcUa_String* pEndpointUrl=pEndpointDescription->GetEndpointUrl();
	OpcUa_String_StrnCpy(&szTmpstring, pEndpointUrl, OpcUa_String_StrLen(pEndpointUrl));
	pszLocalAtts = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszLocalAtts);
	OpcUa_String_StrnCpy(pszLocalAtts, &szTmpstring, OpcUa_String_StrLen(&szTmpstring));
	szAttributes->push_back(pszLocalAtts);

	// SecurityMode
	OpcUa_String_Clear(&szTmpstring);
	// Attribute
	OpcUa_String_AttachCopy(&szTmpstring, "SecurityMode");
	pszLocalAtts = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszLocalAtts);
	OpcUa_String_StrnCpy(pszLocalAtts, &szTmpstring, OpcUa_String_StrLen(&szTmpstring));
	szAttributes->push_back(pszLocalAtts);
	// Attribute value
	OpcUa_String_Clear(&szTmpstring);
	OpcUa_MessageSecurityMode eSecurityMode = pEndpointDescription->GetSecurityMode(); // string	
	switch (eSecurityMode)
	{
	case OpcUa_MessageSecurityMode_None:
		OpcUa_String_AttachCopy(&szTmpstring1, "1");//None
		OpcUa_String_StrnCat(&szTmpstring, &szTmpstring1, OpcUa_String_StrLen(&szTmpstring1));
		break;
	case OpcUa_MessageSecurityMode_Sign:
		OpcUa_String_AttachCopy(&szTmpstring1, "2");//Sign
		OpcUa_String_StrnCat(&szTmpstring, &szTmpstring1, OpcUa_String_StrLen(&szTmpstring1));
		break;
	case OpcUa_MessageSecurityMode_SignAndEncrypt:
		OpcUa_String_AttachCopy(&szTmpstring1, "3");//Sign&Encrypt
		OpcUa_String_StrnCat(&szTmpstring, &szTmpstring1, OpcUa_String_StrLen(&szTmpstring1));
		break;
	default:
		uStatus = OpcUa_BadNotSupported;
		break;
	}
	pszLocalAtts = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszLocalAtts);
	OpcUa_String_StrnCpy(pszLocalAtts, &szTmpstring, OpcUa_String_StrLen(&szTmpstring));
	szAttributes->push_back(pszLocalAtts);

	OpcUa_String_Clear(&szTmpstring);
	// SecurityPolicy
	// Attribute
	OpcUa_String_AttachCopy(&szTmpstring, "SecurityPolicy");
	pszLocalAtts = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszLocalAtts);
	OpcUa_String_StrnCpy(pszLocalAtts, &szTmpstring, OpcUa_String_StrLen(&szTmpstring));
	szAttributes->push_back(pszLocalAtts);
	// Attribute value
	OpcUa_String_Clear(&szTmpstring);
	OpcUa_String* pSecurityPolicy=pEndpointDescription->GetSecurityPolicyUri();
	OpcUa_String_StrnCpy(&szTmpstring, pSecurityPolicy, OpcUa_String_StrLen(pSecurityPolicy));
	pszLocalAtts = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszLocalAtts);
	OpcUa_String_StrnCpy(pszLocalAtts, &szTmpstring, OpcUa_String_StrLen(&szTmpstring));
	szAttributes->push_back(pszLocalAtts);

	OpcUa_String_Clear(&szTmpstring);
	//Name; // string
	// Attribute
	OpcUa_String_AttachCopy(&szTmpstring, "Name");
	pszLocalAtts = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszLocalAtts);
	OpcUa_String_StrnCpy(pszLocalAtts, &szTmpstring, OpcUa_String_StrLen(&szTmpstring));
	szAttributes->push_back(pszLocalAtts);
	// Attribute value
	OpcUa_String_Clear(&szTmpstring);
	OpcUa_String* pSessionName = GetSessionName();
	OpcUa_String_StrnCpy(&szTmpstring, pSessionName, OpcUa_String_StrLen(pSessionName));
	pszLocalAtts = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszLocalAtts);
	OpcUa_String_StrnCpy(pszLocalAtts, &szTmpstring, OpcUa_String_StrLen(&szTmpstring));
	szAttributes->push_back(pszLocalAtts);

	// Timeout
	// Attribute
	OpcUa_String_AttachCopy(&szTmpstring, "Timeout");
	pszLocalAtts = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszLocalAtts);
	OpcUa_String_StrnCpy(pszLocalAtts, &szTmpstring, OpcUa_String_StrLen(&szTmpstring));
	szAttributes->push_back(pszLocalAtts);
	// Attribute value
	OpcUa_String_Clear(&szTmpstring);
	//OpcUa_Double Timeout = m_nSessionTimeout; // double	
	// Tranform m_nSessionTimeout in a string and add to the szLocalAtts
	OpcUa_String SessionTimeout;
	OpcUa_String_Initialize(&SessionTimeout);
	char* buf = (char*)malloc(20);
	if (buf)
	{
		memset(buf, 0, 20);
		sprintf(buf, "%lf", m_nSessionTimeout);
		OpcUa_String_AttachCopy(&SessionTimeout, buf);
		OpcUa_Free(buf);
	}
	OpcUa_String_StrnCpy(&szTmpstring, &SessionTimeout, OpcUa_String_StrLen(&SessionTimeout));
	pszLocalAtts = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszLocalAtts);
	OpcUa_String_StrnCpy(pszLocalAtts, &szTmpstring, OpcUa_String_StrLen(&szTmpstring));
	szAttributes->push_back(pszLocalAtts);

	//
	//if (pszLocalAtts)
	//	OpcUa_String_Clear(pszLocalAtts);
	OpcUa_String_Clear(&szTmpstring);
	OpcUa_String_Clear(&szTmpstring1);
	return uStatus;
}

OpcUa_StatusCode CSessionClient::GetSubscriptions(OpcUa_UInt32* uiNoOfSubscriptions, OpcUa_Handle** hSubscriptions)
{
	OpcUa_StatusCode uStatus = OpcUa_BadInvalidArgument;
	OpcUa_Mutex_Lock(m_SubscriptionListMutex);
	*uiNoOfSubscriptions = m_SubscriptionList.size();
	if (*uiNoOfSubscriptions)
	{
		*hSubscriptions = (OpcUa_Handle*)OpcUa_Alloc(m_SubscriptionList.size()*sizeof(OpcUa_Handle));
		ZeroMemory(*hSubscriptions, m_SubscriptionList.size()*sizeof(OpcUa_Handle));
		for (OpcUa_UInt32 ii = 0; ii < m_SubscriptionList.size(); ii++)
		{
			CSubscriptionClient* pSubscriptionClient = m_SubscriptionList.at(ii);
			(*hSubscriptions)[ii] = (OpcUa_Handle)pSubscriptionClient;
		}
		uStatus = OpcUa_Good;
	}
	else
		uStatus = OpcUa_BadNothingToDo;
	OpcUa_Mutex_Unlock(m_SubscriptionListMutex);
	return uStatus;
}

OpcUa_StatusCode CSessionClient::GetSubscriptionOfMonitoredItem(OpcUa_Handle hMonitoredItem, CSubscriptionClient** pSubscriptionClient)
{

	OpcUa_StatusCode uStatus = OpcUa_BadInvalidArgument;
	OpcUa_Mutex_Lock(m_SubscriptionListMutex);
	for (OpcUa_UInt32 ii = 0; ii < m_SubscriptionList.size(); ii++)
	{
		CSubscriptionClient* pInternalSubscriptionClient = m_SubscriptionList.at(ii);
		if (pInternalSubscriptionClient)
		{
			CMonitoredItemClient* pMonitoredItemClient = pInternalSubscriptionClient->FindMonitoredItemByHandle((OpcUa_UInt32)(uintptr_t)hMonitoredItem);
			if (pMonitoredItemClient)
			{
				(*pSubscriptionClient) = pInternalSubscriptionClient;
				uStatus = OpcUa_Good;
				break;
			}
			else
				uStatus = OpcUa_BadNotFound;
		}
	}
	OpcUa_Mutex_Unlock(m_SubscriptionListMutex);
	return uStatus;
}

WORD CSessionClient::GetSessionState()
{
	return m_wSessionState;
}

PFUNCSHUTDOWN CSessionClient::GetShutdownCallback() { return m_pShutdownCallback; }
void CSessionClient::SetShutdownCallback(PFUNCSHUTDOWN pShutdownCallback) { m_pShutdownCallback = pShutdownCallback; }
void* CSessionClient::GetShutdownCallbackData() { return m_pShutdownCallbackData; }
void CSessionClient::SetShutdownCallbackData(void* pCallbackData) { m_pShutdownCallbackData = pCallbackData; }

CChannel* CSessionClient::GetChannel() { return m_pChannel; }
void CSessionClient::SetChannel(CChannel* pChannel) { m_pChannel = pChannel; }

// Etat interne du serveur
OpcUa_StatusCode CSessionClient::GetInternalServerStatus() { return m_InternalServerStatus; }
void CSessionClient::SetInternalServerStatus(OpcUa_StatusCode uStatus) { m_InternalServerStatus = uStatus; }
OpcUa_UInt32 CSessionClient::GetSequenceNumber() { return m_uiSequenceNumber; }
void CSessionClient::SetSequenceNumber(OpcUa_UInt32 uiVal) { m_uiSequenceNumber = uiVal; }
CClientApplication* CSessionClient::GetClientApplication() 
{ 
	return m_pApplication; 
}
void CSessionClient::IncPendingPublish()
{ 
	m_uiPendingPublish++; 
}
void CSessionClient::DecPendingPublish()
{ 
	m_uiPendingPublish--; 
}
OpcUa_UInt32 CSessionClient::GetPendingPublish() 
{ 
	return m_uiPendingPublish; 
}
OpcUa_Boolean CSessionClient::IsWatchingThreadDetectError()
{
	return m_bFromWatchingThread;
}

void CSessionClient::AdjustPusblish(OpcUa_Int32 iVal)
{
	m_AdjustPublishVal += iVal;
	// Limit the value
	if (m_AdjustPublishVal >10)
	{
		// Reset all
		m_AdjustPublishVal = 1;
		m_uiPendingPublish = 1;
		m_internalPublishStatus = OpcUa_Good;
	}
}
OpcUa_UInt32 CSessionClient::GetPendingPublishSize()
{
	OpcUa_UInt32 iSize=0;
	CleanupPendingPublish();
	OpcUa_Mutex_Lock(m_PendingPublishListMutex);
	iSize=m_PendingPublishList.size();
	OpcUa_Mutex_Unlock(m_PendingPublishListMutex);
	return iSize;
}
///-------------------------------------------------------------------------------------------------
/// <summary>	PendingPublish management v1.0.4.0.  </summary>
///
/// <remarks>	Michel, 14/02/2016. </remarks>
///
/// <param name="pPendingPublish">	[in,out] If non-null, the pending publish. </param>
///-------------------------------------------------------------------------------------------------

void CSessionClient::AddPendingPublish(CPendingPublish* pPendingPublish)
{
	if (pPendingPublish)
	{
		OpcUa_Mutex_Lock(m_PendingPublishListMutex);
		m_PendingPublishList.push_back(pPendingPublish);
		OpcUa_Mutex_Unlock(m_PendingPublishListMutex);
	}
}
void CSessionClient::CleanupPendingPublish()
{
	OpcUa_Mutex_Lock(m_PendingPublishListMutex);
	CPendingPublishList tmpPendingPublishList;
	CPendingPublishList::iterator it;
	for (it = m_PendingPublishList.begin(); it != m_PendingPublishList.end();++it)
	{
		CPendingPublish*pTmpPendingPublish = *it;
		if (pTmpPendingPublish->IsTimeouted())
		{
			delete pTmpPendingPublish;
			pTmpPendingPublish = OpcUa_Null;
			if (m_internalPublishStatus==OpcUa_BadTooManyPublishRequests)
				m_internalPublishStatus = OpcUa_Good;
		}
		else
			tmpPendingPublishList.push_back(pTmpPendingPublish);
	}
	m_PendingPublishList.clear();
	m_PendingPublishList.swap(tmpPendingPublishList);
	if ( (m_PendingPublishList.size() == 0) && (m_internalPublishStatus == OpcUa_BadTooManyPublishRequests) )
		m_internalPublishStatus = OpcUa_Good;
	OpcUa_Mutex_Unlock(m_PendingPublishListMutex);
}
///-------------------------------------------------------------------------------------------------
/// <summary>	Removes the pending publish described by pPendingPublish. </summary>
///
/// <remarks>	Michel, 14/02/2016. </remarks>
///
/// <param name="pPendingPublish">	[in,out] If non-null, the pending publish. </param>
///-------------------------------------------------------------------------------------------------

void CSessionClient::RemovePendingPublish(CPendingPublish*pPendingPublish)
{
	OpcUa_Boolean bOneDeleted = OpcUa_False;
	if (pPendingPublish)
	{
		OpcUa_Mutex_Lock(m_PendingPublishListMutex);
		CPendingPublishList::iterator it;
		for (it = m_PendingPublishList.begin(); it != m_PendingPublishList.end();++it)
		{
			CPendingPublish*pTmpPendingPublish = *it;
			if (pPendingPublish == pTmpPendingPublish)
			{
				m_PendingPublishList.erase(it);
				delete pPendingPublish;
				pPendingPublish = OpcUa_Null;
				bOneDeleted = OpcUa_True;
				break;
			}
		}
		OpcUa_Mutex_Unlock(m_PendingPublishListMutex);
	}
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Removes all pending publish. </summary>
///
/// <remarks>	Michel, 14/02/2016. </remarks>
///-------------------------------------------------------------------------------------------------

void CSessionClient::RemoveAllPendingPublish(void)
{
		OpcUa_Mutex_Lock(m_PendingPublishListMutex);
		CPendingPublishList::iterator it;
		for (it = m_PendingPublishList.begin(); it != m_PendingPublishList.end();++it)
			delete *it;
		m_PendingPublishList.clear();
		OpcUa_Mutex_Unlock(m_PendingPublishListMutex);
}


///-------------------------------------------------------------------------------------------------
// End of source\ClientSession.cpp
///-------------------------------------------------------------------------------------------------
CPendingPublish::CPendingPublish(OpcUa_UInt64 TimeoutHint) :m_TimeoutHint(TimeoutHint)
{
	m_publishTime = OpcUa_DateTime_UtcNow();
	m_hPublish = (OpcUa_Handle)this;
	m_bDeleted = OpcUa_False;
}
CPendingPublish::~CPendingPublish()
{
	m_hPublish = (OpcUa_Handle)OpcUa_Null;
}
OpcUa_Boolean CPendingPublish::IsTimeouted()
{
	OpcUa_Boolean bResult = OpcUa_False;
	OpcUa_DateTime utcNow = OpcUa_DateTime_UtcNow();

	OpcUa_UInt64 HeaderTimestamp = m_publishTime.dwHighDateTime;
	HeaderTimestamp <<= 32;
	HeaderTimestamp += m_publishTime.dwLowDateTime;
	OpcUa_UInt64 utcNow64 = utcNow.dwHighDateTime;
	utcNow64 <<= 32;
	utcNow64 += utcNow.dwLowDateTime;


		OpcUa_UInt64 ullTimeout = utcNow64 - HeaderTimestamp;

		if (m_TimeoutHint * 10000 < ullTimeout)
		{
			SetDeleted(OpcUa_True);
			bResult = OpcUa_True;
		}

	return bResult;
}

void CPendingPublish::SetDeleted(OpcUa_Boolean bVal)
{
	m_bDeleted = bVal;
}
