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
#include <float.h>

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
using namespace OpenOpcUa;
using namespace UAAddressSpace;
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
#include "LuaDebugger.h"
#include "LuaVirtualMachine.h"
#include "LuaScript.h"
#include "OpenOpcUaScript.h"
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
#include "QueueRequest.h"
#include "QueuedCallRequest.h"
#include "QueuedReadRequest.h"
#include "QueuedPublishRequest.h"
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
#include "opcua_securechannel_types.h"
#include "Utils.h"
#include "CryptoUtils.h"


using namespace OpenOpcUa;
using namespace UAAddressSpace;
using namespace UACoreServer;
using namespace UASharedLib;
using namespace UAScript;
#define CHECK_RANGE()\
if (pNumericRange)\
{\
	if (pNumericRange->GetBeginIndex()<pNumericRange->GetEndIndex())\
	{\
		if ((pNumericRange->GetBeginIndex()>0) && (pNumericRange->GetEndIndex()>0) &&(pVariant->Value.Array.Length<=pNumericRange->GetEndIndex()))\
			uStatusFinal=OpcUa_BadIndexRangeNoData;\
		if ((pNumericRange->GetBeginIndex()<0) || (pNumericRange->GetEndIndex()<0))\
			uStatusFinal=OpcUa_BadOutOfRange;\
	}\
	else\
		uStatusFinal=OpcUa_BadOutOfRange;\
}\
else\
	uStatusFinal=OpcUa_Good;

//BOOL CSessionServer::m_bRunAsyncRequestThread;
//HANDLE CSessionServer::m_hStopAsyncRequestThread;
//IMPLIMENT_CHILD_DYNCLASS(Session,Server,Base)
CSessionServer::CSessionServer()
{	
	OpcUa_String_Initialize(&m_SessionName);
	OpcUa_DateTime_Initialize(&m_LastTransmissionTime);
	OpcUa_StatusCode uStatus=OpcUa_Good;
	m_pContinuationPointList=new CContinuationPointList();
	// Publish
	uStatus = OpcUa_Mutex_Create(&m_hPublishMutex);
	OpcUa_Mutex_Lock(m_hPublishMutex);
	m_PublishRequests.clear();
	OpcUa_Mutex_Unlock(m_hPublishMutex);
	//
	CUAInformationModel* pInformationModel = CServerApplication::m_pTheAddressSpace;
	
	m_bServerDiagnosticsEnabled=pInformationModel->IsEnabledServerDiagnosticsDefaultValue();
	m_bRunSessionTimeoutThread=OpcUa_True;
	m_pClientNonce=OpcUa_Null;
	m_bSessionActive=OpcUa_False;
	m_pSecureChannel=OpcUa_Null;
	m_hSessionTimeoutThread=OpcUa_Null;
	OpcUa_Mutex_Create(&m_hSubscriptionListMutex);
	// Call
	uStatus = OpcUa_Mutex_Create(&m_hCallMutex);
	m_pCallMessages=new CQueuedCallMessages();
	m_pCallMessages->clear();
	// Read
	uStatus = OpcUa_Mutex_Create(&m_hReadMutex);
	m_pReadMessages=new CQueuedReadMessages();
	m_pReadMessages->clear();
	// HistoryRead
	uStatus = OpcUa_Mutex_Create(&m_hHistoryReadMutex);
	m_pHistoryReadMessages=new CQueuedHistoryReadMessages();
	m_pHistoryReadMessages->clear();
	// QueryFirst
	uStatus = OpcUa_Mutex_Create(&m_hQueryFirstMutex);
	m_pQueryFirstMessages = new CQueuedQueryFirstMessages();
	m_pQueryFirstMessages->clear();
	// QueryNext
	uStatus = OpcUa_Mutex_Create(&m_hQueryNextMutex);
	m_pQueryNextMessages = new CQueuedQueryNextMessages();
	m_pQueryNextMessages->clear();
	//
	m_pServerNonce = (OpcUa_ByteString*)OpcUa_Alloc(sizeof(OpcUa_ByteString));
	OpcUa_ByteString_Initialize(m_pServerNonce);
	m_SessionTimeout=0.0;
	if (pInformationModel->IsEnabledServerDiagnosticsDefaultValue())
	{
		// Session security
		m_pSessionSecurityDiagnostics = new CSessionSecurityDiagnosticsDataType();
		// Initialisation de l'objet assurant la surveillance de la node
		m_pSessionDiagnostics = new CSessionDiagnosticsDataType();
	}
	else
	{
		m_pSessionSecurityDiagnostics = OpcUa_Null;
		m_pSessionDiagnostics = OpcUa_Null;
	}
	//m_pSecureChannel=OpcUa_Null;
	OpcUa_Semaphore_Create(&m_hPublishSem,0,0x100);
	m_dwAsyncThreadInterval=PUBLISHING_INTERVAL_MINI; // valeur par défaut de fonctionnement de la thread de traitement de demandes asyncrhones
	// AsyncRequest Thread attribute initialisation
	m_hAsyncRequestThread=OpcUa_Null;
	OpcUa_Semaphore_Create(&m_hAsyncRequest,0,0x100);
	OpcUa_Semaphore_Create(&m_hStopAsyncRequestThreadSem,0,0x100);
	// Start the thread in charge of Read, HistroryRead,Call, Query
	StartAsyncRequestThread();
	// NotificationData Thread attribute initialisation
	m_hNotificationDataThread = OpcUa_Null;
	m_dwNotificationDataThreadInterval = DEFAULT_NOTIFICATION_INTERVAL;
	OpcUa_Semaphore_Create(&m_hNotificationDataSem, 0, 0x100);
	OpcUa_Semaphore_Create(&m_hStopNotificationDataThreadSem, 0, 0x100);
	// Start the thread in charge of Read, HistroryRead,Call, Query
	StartNotificationDataThread();
	
	// Start the thread to monitor timeout for this session
	StartSessionTimeoutThread();
	// Start the thread to monitor the Subscription of this session
	OpcUa_Semaphore_Create(&m_SubscriptionsLifeTimeCountSem,0,0x100);
	OpcUa_Semaphore_Create(&m_hStopSubscriptionsLifeTimeCountThreadSem,0,0x100);
	m_hSubscriptionsLifeTimeCountThread=OpcUa_Null;
	StartSubscriptionsLifeTimeCountThread();
}

CSessionServer::CSessionServer(UACoreServer::CServerApplication* pApplication,
	OpcUa_UInt32						uSecureChannelId, // in
	OpcUa_UInt16						securityMode, // in
	OpcUa_String						securityPolicyUri, // in
	const OpcUa_ApplicationDescription* pClientDescription, // in
	const OpcUa_String*					pSessionName, // in
	const OpcUa_ByteString*				pClientNonce, // in
	const OpcUa_ByteString*				pClientCertificate,
	OpcUa_Double						nRequestedSessionTimeout,
	OpcUa_UInt32						nMaxResponseMessageSize,
	OpcUa_NodeId*						pSessionId,
	OpcUa_NodeId*						pAuthenticationToken, // in. Il a été créer par l'appelant
	OpcUa_Double*						pRevisedSessionTimeout,
	OpcUa_ByteString*					pServerNonce,
	OpcUa_SignatureData*				pServerSignature,
	OpcUa_UInt32*						pMaxRequestMessageSize)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_Key uaKeyServerNonce;

	OpcUa_DateTime_Initialize(&m_LastTransmissionTime);
	m_bRunSessionTimeoutThread=OpcUa_True;
	m_bServerDiagnosticsEnabled=OpcUa_False;
	if (pClientDescription)
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_WARNING, "SessionServer%s\n", OpcUa_String_GetRawString(&(pClientDescription->ApplicationName.Text)));
	try
	{
		OpcUa_String_Initialize(&m_SessionName);
		// ContinuationPoint
		m_pContinuationPointList=new CContinuationPointList();
		
		// ClientNonce
		if ( (pClientNonce) && (pClientNonce->Data))
		{
			m_pClientNonce=(OpcUa_ByteString*)OpcUa_Alloc(sizeof(OpcUa_ByteString));
			OpcUa_ByteString_Initialize(m_pClientNonce);
			OpcUa_ByteString_CopyTo(pClientNonce,m_pClientNonce);
		}
		else
			m_pClientNonce=OpcUa_Null;
		OpcUa_ReferenceParameter(pSessionName);
		//////////////////////////////////////////////////////////////////////
		CUAInformationModel* pInformationModel = CServerApplication::m_pTheAddressSpace;
		m_bSessionActive = OpcUa_False; // pInformationModel->IsEnabledServerDiagnosticsDefaultValue();
		m_hSessionTimeoutThread=OpcUa_Null;
		OpcUa_Mutex_Create(&m_hSubscriptionListMutex);
		//
		m_SessionTimeout=0.0;
		// Diagnostic information 
		if (pInformationModel->IsEnabledServerDiagnosticsDefaultValue())
		{
			m_pSessionSecurityDiagnostics = new CSessionSecurityDiagnosticsDataType();
			// Initialisation de l'objet assurant la surveillance de la node
			m_pSessionDiagnostics = new CSessionDiagnosticsDataType();
		}
		else
		{
			m_pSessionSecurityDiagnostics = OpcUa_Null;
			m_pSessionDiagnostics = OpcUa_Null;
		}
		// Call
		uStatus = OpcUa_Mutex_Create(&m_hCallMutex);
		m_pCallMessages = new CQueuedCallMessages();
		m_pCallMessages->clear();
		// Read
		uStatus = OpcUa_Mutex_Create(&m_hReadMutex);
		m_pReadMessages = new CQueuedReadMessages();
		m_pReadMessages->clear();
		// HistoryRead
		uStatus = OpcUa_Mutex_Create(&m_hHistoryReadMutex);
		m_pHistoryReadMessages = new CQueuedHistoryReadMessages();
		m_pHistoryReadMessages->clear();
		// QueryFirst
		uStatus = OpcUa_Mutex_Create(&m_hQueryFirstMutex);
		m_pQueryFirstMessages = new CQueuedQueryFirstMessages();
		m_pQueryFirstMessages->clear();
		// QueryNext
		uStatus = OpcUa_Mutex_Create(&m_hQueryNextMutex);
		m_pQueryNextMessages = new CQueuedQueryNextMessages();
		m_pQueryNextMessages->clear();
		// publish
		uStatus = OpcUa_Mutex_Create(&m_hPublishMutex);
		OpcUa_Mutex_Lock(m_hPublishMutex);
		m_PublishRequests.clear();
		OpcUa_Mutex_Unlock(m_hPublishMutex);
		OpcUa_Semaphore_Create(&m_hPublishSem,0,0x100);
		m_dwAsyncThreadInterval=PUBLISHING_INTERVAL_MINI; // valeur par défaut de fonctionnement de la thread de traitement de demandes asyncrhones
		// association du secureChannel uSecureChannelId avec le SecureChannel de cette Session
		m_pApplication = pApplication;
		m_pSecureChannel=pApplication->FindSecureChannel(uSecureChannelId);
		// Will try to setup the ClientCertificate if it was not already done during the secureChannel creation
		if(m_pSecureChannel)
		{
			OpcUa_ByteString* pTmpClientCert=m_pSecureChannel->GetClientCertificate();
			if (!pTmpClientCert)
				m_pSecureChannel->SetClientCertificate(pClientCertificate);
		}
		// 
		m_nMaxResponseMessageSize = 0;
		m_nMaxResponseMessageSize = 0;

		OpcUa_NodeId_Initialize(&m_tSessionId);
		m_pAuthenticationToken = (OpcUa_NodeId*)OpcUa_Alloc(sizeof(OpcUa_NodeId));
		OpcUa_NodeId_Initialize(m_pAuthenticationToken);
		m_pServerNonce = (OpcUa_ByteString*)OpcUa_Alloc(sizeof(OpcUa_ByteString));
		OpcUa_ByteString_Initialize(m_pServerNonce);

		OpcUa_MemSet(&m_tCryptoProvider, 0, sizeof(OpcUa_CryptoProvider));

		// AsyncRequest Thread attribute initialisation
		m_hAsyncRequestThread = OpcUa_Null;
		OpcUa_Semaphore_Create(&m_hAsyncRequest, 0, 0x100);
		OpcUa_Semaphore_Create(&m_hStopAsyncRequestThreadSem, 0, 0x100);
		// Start the thread in charge of Read, HistroryRead,Call, Query
		StartAsyncRequestThread();
		// NotificationData Thread attribute initialisation
		m_hNotificationDataThread = OpcUa_Null;
		m_dwNotificationDataThreadInterval = DEFAULT_NOTIFICATION_INTERVAL;
		OpcUa_Semaphore_Create(&m_hNotificationDataSem, 0, 0x100);
		OpcUa_Semaphore_Create(&m_hStopNotificationDataThreadSem, 0, 0x100);
		// Start the thread in charge of NotificationMessage
		StartNotificationDataThread();

		// Start the thread to monitor timeout for this session
		StartSessionTimeoutThread();
		// Start the thread to monitor the Subscription of this session
		OpcUa_Semaphore_Create(&m_SubscriptionsLifeTimeCountSem, 0, 0x100);
		OpcUa_Semaphore_Create(&m_hStopSubscriptionsLifeTimeCountThreadSem, 0, 0x100);
		m_hSubscriptionsLifeTimeCountThread = OpcUa_Null;
		StartSubscriptionsLifeTimeCountThread();
		//////////////////////////////////////////////////////////////////////
		// ServerNonce		
		OpcUa_ByteString_Initialize(pServerNonce);
		pServerNonce->Length=0;
		pServerNonce->Data=OpcUa_Null;
		// ServerSignature
		// clientNonce+clientCertificate==>Chaine de d'octet
		// cette chaine d'octet doit être signée en utilisant l'algorythme de signature asymetrique 
		// indiqué dans la politique de sécurité du endPoint
		OpcUa_SignatureData_Initialize(pServerSignature);
		OpcUa_Key_Initialize(&uaKeyServerNonce);

		// save the session data.
		// SessionName

		SetSessionName((OpcUa_String*)pSessionName);
		// SessionId
		SetSessionId(pSessionId);
		// AuthenticationToken
		OpcUa_NodeId_CopyTo(pAuthenticationToken, m_pAuthenticationToken);
		
		if (!m_pAuthenticationToken)
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Critical Error:>AuthenticationToken corrupted\n");
		// SessionTimeout. We limit it to reasonable value 100ms - 1800 sec
		if ((nRequestedSessionTimeout <= 100) || (nRequestedSessionTimeout>1800000))
			SetSessionTimeout(600000);
		else
			SetSessionTimeout(nRequestedSessionTimeout);
		*pRevisedSessionTimeout=GetSessionTimeout();
		SetMaxResponseMessageSize(nMaxResponseMessageSize);

		// return the revised paramters.
		(*pRevisedSessionTimeout) = GetSessionTimeout();
		if (!pMaxRequestMessageSize)
			pMaxRequestMessageSize=(OpcUa_UInt32*)OpcUa_Alloc(sizeof(OpcUa_UInt32));
		*pMaxRequestMessageSize = 0; //0 pour no-limit

		// create the signature and nonce.
		if (securityMode != OpcUa_MessageSecurityMode_None)//m_pApplication->GetSecureChannel()->GetSecurityMode()
		{
			// create crypto provider.
			uStatus = OpcUa_CryptoProvider_Create(OpcUa_String_GetRawString(&securityPolicyUri), &m_tCryptoProvider);
			if (uStatus!=OpcUa_Good)
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Could not create crypto provider\n");
			else
			{
				// create a signature to return to the client.
				uStatus=CryptoUtils::CreateSignature(
					&m_tCryptoProvider, 
					pClientCertificate, 
					pClientNonce,
					m_pApplication->GetCertificate(), 
					m_pApplication->GetPrivateKey(), 
					pServerSignature);
				if (uStatus!=OpcUa_Good)
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"CryptoUtils::CreateSignature failed 0x%05x.\n",uStatus);
				else
				{
					// generate a nonce.
					// un nonce est une chaine de'octet qui est créer par  la CryptoAPI via OpcUa_Crypto_GenerateKey
					// Elle sert a identifier la session en cours
					uaKeyServerNonce.Key.Length = 32;
					uaKeyServerNonce.Key.Data = (OpcUa_Byte*)OpcUa_Alloc(uaKeyServerNonce.Key.Length);

					uStatus = OpcUa_Crypto_GenerateKey(&m_tCryptoProvider, uaKeyServerNonce.Key.Length, &uaKeyServerNonce);
					if (uStatus!=OpcUa_Good)
						OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Could not create server nonce.\n");
					else
					{
						OpcUa_ByteString_Clear(m_pServerNonce);
						OpcUa_ByteString_CopyTo(&(uaKeyServerNonce.Key), m_pServerNonce);
						if (pServerNonce)
						{
							OpcUa_ByteString_Clear(pServerNonce);
							OpcUa_ByteString_CopyTo(m_pServerNonce,pServerNonce);
						}
						else
							OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Input parameter is NULL .\n");
						OpcUa_ByteString_Clear(&(uaKeyServerNonce.Key));
						OpcUa_Free(uaKeyServerNonce.Key.Data);
					}
				}
			}
		}
		//// initialisation du serverDiagnosticsDataType associé a cette session
		//CUAInformationModel* pInformationModel=CServerApplication::m_pTheAddressSpace;
		//if (pInformationModel)
		//{
		//	m_bServerDiagnosticsEnabled = pInformationModel->IsEnabledServerDiagnosticsDefaultValue();
		//	if (m_bServerDiagnosticsEnabled)
		//	{
		//		m_pSessionDiagnostics->SetClientDescription(pClientDescription);
		//		uStatus = InitSessionDiagnosticsDataType();
		//		if (uStatus != OpcUa_Good)
		//			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "InitSessionDiagnosticsDataType failed 0x%05x\n", uStatus);
		//		else
		//		{
		//			// initialisation du SessionSecurityDiagnosticsDataType associé a cette session
		//			uStatus = InitSessionSecurityDiagnosticsDataType();
		//			if (uStatus != OpcUa_Good)
		//				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "InitSessionSecurityDiagnosticsDataType failed 0x%05x\n", uStatus);
		//		}
		//	}
		//}
	}
	catch (...)
	{
		OpcUa_Key_Clear(&uaKeyServerNonce);
		throw std::exception();
	}
}

CSessionServer::~CSessionServer(void)
{
	CUAInformationModel* pInformationModel=CServerApplication::m_pTheAddressSpace;

	if (m_pClientNonce)
	{
		OpcUa_ByteString_Clear(m_pClientNonce);
		OpcUa_Free(m_pClientNonce);
	}
	// Suppression des subscriptions
	RemoveSubscriptions();
	// Stop Threads
	StopSessionTimeoutThread();
	OpcUa_Semaphore_Delete(&m_SessionTimeoutSem);
	OpcUa_Semaphore_Delete(&m_StopSessionTimeoutSem);
	// Arret de la thread de surveillance des souscription de cette session
	StopSubscriptionsLifeTimeCount();
	OpcUa_Semaphore_Delete(&m_SubscriptionsLifeTimeCountSem);
	OpcUa_Semaphore_Delete(&m_hStopSubscriptionsLifeTimeCountThreadSem);
	// Session Name
	OpcUa_String_Clear(&m_SessionName);
	// Delete all pending publish request
	RemoveAllPublishRequest();
	// destruction de la semaphore de synchronisation de accès a la publishlist
	OpcUa_Semaphore_Delete(&m_hPublishSem);
	// Suppression de tous les pending messages de lecture
	RemoveAllReadRequest();
	// Remove all the HistoryReadRequest pending
	RemoveAllHistoryReadRequest();
	// Remove all Call method pending
	RemoveAllCallRequest();
	// Remove all QueryFirst pending
	RemoveAllQueryFirstRequest();
	// Remove all QueryNext pending
	RemoveAllQueryNextRequest();
	// Stop the AsyncRequestThread
	if (StopAsyncRequestThread()!=OpcUa_Good)
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"AsyncRequestThread failed to stop\n");
	//Stop the NotificationDataThread
	if (StopNotificationDataThread() != OpcUa_Good)
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"NotificationDataThread failed to stop\n");
	OpcUa_Semaphore_Delete(&m_hStopNotificationDataThreadSem);
	// Read
	OpcUa_Mutex_Lock(m_hReadMutex);
	delete m_pReadMessages;
	OpcUa_Mutex_Unlock(m_hReadMutex);
	// Call
	OpcUa_Mutex_Lock(m_hCallMutex);
	delete m_pCallMessages;
	OpcUa_Mutex_Unlock(m_hCallMutex);
	// HistoryRead
	OpcUa_Mutex_Lock(m_hHistoryReadMutex);
	delete m_pHistoryReadMessages;
	OpcUa_Mutex_Unlock(m_hHistoryReadMutex);
	//	
	delete m_pQueryNextMessages;
	//
	delete m_pQueryFirstMessages;
	// desctuction de la mutex... on en aura plus besoin
	OpcUa_Mutex_Delete(&m_hReadMutex);
	OpcUa_Mutex_Delete(&m_hCallMutex);
	OpcUa_Mutex_Delete(&m_hQueryFirstMutex);
	OpcUa_Mutex_Delete(&m_hQueryNextMutex);
	OpcUa_Mutex_Delete(&m_hHistoryReadMutex);
	OpcUa_NodeId_Clear(&m_tSessionId);
	// clear the m_tAuthenticationToken
	//OpcUa_NodeId_Clear(m_pAuthenticationToken);
	// clear the m_tServerNonce
	if (m_pServerNonce)
	{
		OpcUa_ByteString_Clear(m_pServerNonce);
		OpcUa_Free(m_pServerNonce);
	}
	// Delete the crytpoProvider
	if (m_tCryptoProvider.Handle != 0)
	{
		OpcUa_CryptoProvider_Delete(&m_tCryptoProvider);
	}
	// desctuction de la mutex... on en aura plus besoin
	OpcUa_Mutex_Delete(&m_hSubscriptionListMutex);
	//
	OpcUa_Semaphore_Delete(&m_hAsyncRequest);
	OpcUa_Semaphore_Delete(&m_hStopAsyncRequestThreadSem);
	OpcUa_Semaphore_Delete(&m_hNotificationDataSem);
	if (m_pSessionSecurityDiagnostics)
	{
		if (pInformationModel)
			pInformationModel->RemoveInSessionSecurityDiagnosticsArray(m_pSessionSecurityDiagnostics);
		delete m_pSessionSecurityDiagnostics;
		m_pSessionSecurityDiagnostics = OpcUa_Null;
	}
	if (m_pSessionDiagnostics)
	{
		if (pInformationModel)
		{
			if (pInformationModel->RemoveInSessionDiagnosticsArray(m_pSessionDiagnostics) == OpcUa_Good)
			{
				if (m_pSessionDiagnostics)
				{
					delete m_pSessionDiagnostics;
					m_pSessionDiagnostics = OpcUa_Null;
				}
			}
		}
	}
	// suppression des CContinuationPoint résiduel
	if (m_pContinuationPointList)
	{
		CContinuationPointList::iterator it;
		while (!m_pContinuationPointList->empty())
		{
			it=m_pContinuationPointList->begin();
			delete *it;
			m_pContinuationPointList->erase(it);
		}
		delete m_pContinuationPointList;
	}
	// suppression des m_PublishRequests residuel
	OpcUa_Mutex_Lock(m_hPublishMutex);
	CQueuedPublishMessages::iterator it;
	while (!m_PublishRequests.empty())
	{
		it=m_PublishRequests.begin();
		delete *it;
		m_PublishRequests.erase(it);
	}
	OpcUa_Mutex_Unlock(m_hPublishMutex);
	// desctuction de la mutex... on en aura plus besoin
	OpcUa_Mutex_Delete(&m_hPublishMutex);
	// suppression des NodeRegitered
	m_RegistredNodes.clear();
}


//============================================================================
// CSessionServer::Activate
//============================================================================
OpcUa_StatusCode CSessionServer::Activate(
	OpcUa_UInt32							uSecureChannelId,
	OpcUa_UInt16							securityMode,
	OpcUa_String*							securityPolicyUri,
	OpcUa_Int32								nNoOfClientSoftwareCertificates, // in
	const OpcUa_SignedSoftwareCertificate*	pClientSoftwareCertificates, //in 
	const OpcUa_SignatureData*				pClientSignature,
	const OpcUa_ExtensionObject*            a_pUserIdentityToken,
	OpcUa_ByteString*						pServerNonce)
{
	OpcUa_ReferenceParameter(nNoOfClientSoftwareCertificates);
	OpcUa_ReferenceParameter(pClientSoftwareCertificates);
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_Key tServerNonce;


	OpcUa_Key_Initialize(&tServerNonce);

	// check that the secure channel matches the original.
	// complete servers allow the client to change the secure channel provided the certificate signature matches.
	if (m_pSecureChannel->GetSecureChannelId() != uSecureChannelId)
	{	
		uStatus=Reconnect(uSecureChannelId);
		if (uStatus==OpcUa_Good)
		{
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"CSessionServer::Activate>Reconnect suceeded\n");
		}
	}

	// check for a match on security mode.
	if (m_pSecureChannel->GetSecurityMode() != securityMode)
	{	
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Cannot change the security mode.\n");
		uStatus= OpcUa_BadSecurityModeRejected;
	}
	else
	{
		// check for a match on security policy.
		if (OpcUa_String_StrnCmp(m_pSecureChannel->GetSecurityPolicy(),securityPolicyUri,OpcUa_String_StrLen(securityPolicyUri),OpcUa_False))
		{	
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Cannot change the security policy. \n");
			uStatus= OpcUa_BadSecurityPolicyRejected;
		}
		else
		{
			uStatus=OpcUa_Good;
			// Verify the userIdentityToken
			OpcUa_Boolean bUserIdentityTokenValue = OpcUa_False; // use to indicate if the UserIdentityToken is accepted of not by the server
			if (a_pUserIdentityToken)
			{

				if (a_pUserIdentityToken->Encoding == OpcUa_ExtensionObjectEncoding_EncodeableObject)
				{
					if (a_pUserIdentityToken->Body.EncodeableObject.Type)
					{
						OpcUa_String szAuthenticationToken;
						OpcUa_String_Initialize(&szAuthenticationToken);
						if (OpcUa_StrCmpA(a_pUserIdentityToken->Body.EncodeableObject.Type->TypeName, "AnonymousIdentityToken") == 0)
						{
							OpcUa_String_AttachCopy(&szAuthenticationToken, "AnonymousIdentityToken");
							
							bUserIdentityTokenValue = OpcUa_True;
						}
						else
						{
							if (OpcUa_StrCmpA(a_pUserIdentityToken->Body.EncodeableObject.Type->TypeName, "UserNameIdentityToken") == 0)
							{
								OpcUa_String_AttachCopy(&szAuthenticationToken, "UserNameIdentityToken");
								OpcUa_UserNameIdentityToken* pUserNameIdentityToken = (OpcUa_UserNameIdentityToken*)a_pUserIdentityToken->Body.EncodeableObject.Object;
								if (pUserNameIdentityToken)
								{
									// Check that the PolicyId is valid
									uStatus = OpcUa_BadIdentityTokenInvalid;
									if (OpcUa_String_IsNull(&(pUserNameIdentityToken->PolicyId))==OpcUa_False)
									{
										for (OpcUa_UInt32 i = 0; i < m_pApplication->m_UserNameIdentityTokenList.size(); i++)
										{
											OpcUa_UserNameIdentityToken* pValidUserIdentityToken = m_pApplication->m_UserNameIdentityTokenList.at(i);
											if (OpcUa_String_Compare(&(pValidUserIdentityToken->PolicyId), &(pUserNameIdentityToken->PolicyId)) == 0)
											{
												uStatus = OpcUa_Good;
												break;
											}
										}
									}
									else
										uStatus = OpcUa_Good;

									if (uStatus == OpcUa_Good)
									{
										// Check other parameters
										//
										if (OpcUa_String_IsNull(&(pUserNameIdentityToken->UserName)) && (pUserNameIdentityToken->Password.Data == OpcUa_Null))
											uStatus = OpcUa_BadIdentityTokenInvalid;
										else
										{
											// Check the securityPolicyUri is set according to pUserNameIdentityToken->EncryptionAlgorithm
											OpcUa_String szPolicyNone;
											OpcUa_String_Initialize(&szPolicyNone);
											OpcUa_String_AttachCopy(&szPolicyNone, OpcUa_SecurityPolicy_None);
											if (OpcUa_String_Compare(securityPolicyUri, &szPolicyNone) == 0)
											{
												if ((OpcUa_String_IsNull(&(pUserNameIdentityToken->EncryptionAlgorithm)) == OpcUa_False))
													uStatus = OpcUa_BadIdentityTokenInvalid;
											}
											else
											{
												OpcUa_String szPolicyBasic128Rsa15;
												OpcUa_String_Initialize(&szPolicyBasic128Rsa15);
												OpcUa_String_AttachCopy(&szPolicyBasic128Rsa15, OpcUa_SecurityPolicy_Basic128Rsa15);
												if (OpcUa_String_Compare(securityPolicyUri, &szPolicyBasic128Rsa15) == 0)
												{
													if ((OpcUa_String_IsNull(&(pUserNameIdentityToken->EncryptionAlgorithm))))
														uStatus = OpcUa_BadIdentityTokenInvalid;
												}
												else
												{
													OpcUa_String szPolicyBasic256;
													OpcUa_String_Initialize(&szPolicyBasic256);
													OpcUa_String_AttachCopy(&szPolicyBasic256, OpcUa_SecurityPolicy_Basic256);
													if (OpcUa_String_Compare(securityPolicyUri, &szPolicyBasic256) == 0)
													{
														if ((OpcUa_String_IsNull(&(pUserNameIdentityToken->EncryptionAlgorithm))))
															uStatus = OpcUa_BadIdentityTokenInvalid;
														if (OpcUa_String_Compare(&(pUserNameIdentityToken->EncryptionAlgorithm), &szPolicyNone) == 0)
															uStatus = OpcUa_BadIdentityTokenInvalid;
													}
													else
														uStatus = OpcUa_BadIdentityTokenInvalid;
													OpcUa_String_Clear(&szPolicyBasic256);
												}
												OpcUa_String_Clear(&szPolicyBasic128Rsa15);
											}
											// Release resource
											OpcUa_String_Clear(&szPolicyNone);
											if (uStatus == OpcUa_Good)
											{
												uStatus = OpcUa_BadUserAccessDenied;
												for (OpcUa_UInt32 i = 0; i < m_pApplication->m_UserNameIdentityTokenList.size(); i++)
												{
													if (bUserIdentityTokenValue)
														break;
													OpcUa_UserNameIdentityToken* pValidUserIdentityToken = m_pApplication->m_UserNameIdentityTokenList.at(i);
													// Check username
													if (OpcUa_String_Compare(&(pValidUserIdentityToken->UserName), &(pUserNameIdentityToken->UserName)) == 0)
													{
														// Check password
														OpcUa_ByteString aDecodedPassword;
														OpcUa_ByteString_Initialize(&aDecodedPassword);
														//Check cypher suite RSA_OAEP
														OpcUa_String szRSA_OAEP;
														OpcUa_String_Initialize(&szRSA_OAEP);
														OpcUa_String_AttachCopy(&szRSA_OAEP, "http://www.w3.org/2001/04/xmlenc#rsa-oaep");
														OpcUa_String szRSA_15;
														OpcUa_String_Initialize(&szRSA_15);
														OpcUa_String_AttachCopy(&szRSA_15, "http://www.w3.org/2001/04/xmlenc#rsa-1_5");
														// Do we need to decrypt the password ?
														OpcUa_Key* privateKey = m_pApplication->GetPrivateKey();
														if ((OpcUa_String_IsEmpty(&(pUserNameIdentityToken->EncryptionAlgorithm))) || OpcUa_String_IsNull(&(pUserNameIdentityToken->EncryptionAlgorithm)))
															OpcUa_ByteString_CopyTo(&(pUserNameIdentityToken->Password), &aDecodedPassword);
														else
														{
															if ((OpcUa_String_Compare(&(pUserNameIdentityToken->EncryptionAlgorithm), &szRSA_15) == 0)
																|| (OpcUa_String_Compare(&(pUserNameIdentityToken->EncryptionAlgorithm), &szRSA_OAEP) == 0))
															{
																// pUserNameIdentityToken->Password contains an encryptyed password.
																// The password was encrypted using the SecurityPolicy use by the server (in this session)
																OpcUa_Byte* pPlainText = OpcUa_Null;
																OpcUa_UInt32  uiLen = 0;
																if (pUserNameIdentityToken->Password.Length)
																{
																	pPlainText = (OpcUa_Byte*)OpcUa_Alloc(pUserNameIdentityToken->Password.Length);
																	if (pPlainText)
																		ZeroMemory(pPlainText, pUserNameIdentityToken->Password.Length);
																}
																if (m_tCryptoProvider.AsymmetricDecrypt)
																{
																	uStatus = m_tCryptoProvider.AsymmetricDecrypt(&(m_tCryptoProvider),
																		pUserNameIdentityToken->Password.Data,
																		pUserNameIdentityToken->Password.Length,
																		privateKey, pPlainText, &uiLen);
																	if (uStatus != OpcUa_Good)
																		OpcUa_ByteString_CopyTo(&(pValidUserIdentityToken->Password), &aDecodedPassword);
																	else
																	{
																		OpcUa_UInt32 uiEncryptedUserIdentityTokenFullLen = 0;
																		OpcUa_UInt32 uiEncryptedUserIdentityTokenLen = 0;
																		OpcUa_UInt32 uiEncryptedUserIdentityNonceLen = 0;
																		if (pPlainText)
																		{
																			// Extract first the full length from the pPlainText
																			OpcUa_MemCpy(&uiEncryptedUserIdentityTokenFullLen, 4, &pPlainText[0], 4);
																			if (uiEncryptedUserIdentityTokenFullLen > 0)
																			{
																				if (m_pServerNonce)
																				{
																					// Extract the tokenData 
																					uiEncryptedUserIdentityTokenLen = uiEncryptedUserIdentityTokenFullLen - m_pServerNonce->Length;
																					aDecodedPassword.Data = (OpcUa_Byte*)OpcUa_Alloc(uiEncryptedUserIdentityTokenLen);
																					ZeroMemory(aDecodedPassword.Data, uiEncryptedUserIdentityTokenLen);
																					OpcUa_MemCpy(aDecodedPassword.Data, uiEncryptedUserIdentityTokenLen, &pPlainText[4], uiEncryptedUserIdentityTokenLen);
																					aDecodedPassword.Length = uiEncryptedUserIdentityTokenLen;
																					// Extract the serverNonce
																					OpcUa_ByteString EncryptedNonce;
																					OpcUa_ByteString_Initialize(&EncryptedNonce);
																					uiEncryptedUserIdentityNonceLen = uiEncryptedUserIdentityTokenFullLen - uiEncryptedUserIdentityTokenLen;
																					EncryptedNonce.Data = (OpcUa_Byte*)OpcUa_Alloc(uiEncryptedUserIdentityNonceLen);
																					ZeroMemory(EncryptedNonce.Data, uiEncryptedUserIdentityNonceLen);
																					OpcUa_MemCpy(EncryptedNonce.Data, uiEncryptedUserIdentityNonceLen, &pPlainText[uiEncryptedUserIdentityTokenLen + 4], uiEncryptedUserIdentityNonceLen);
																					EncryptedNonce.Length = uiEncryptedUserIdentityNonceLen;
																					// Now compare the received nonce with the server Nonce
																					if (OpcUa_ByteString_Compare(m_pServerNonce, &EncryptedNonce) != 0)
																						uStatus = OpcUa_BadUserSignatureInvalid;
																					// Clear ressources
																					OpcUa_ByteString_Clear(&EncryptedNonce);
																				}
																				else
																					uStatus = OpcUa_BadInvalidArgument;
																			}
																		}
																	}
																}
																if (pPlainText)
																	OpcUa_Free(pPlainText);
															}
															else
															{
																OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "EncryptionAlgorithm not supported: [ %s ]",
																	OpcUa_String_GetRawString(&(pUserNameIdentityToken->EncryptionAlgorithm)));
															}
															//OpcUa_UInt32  uiLen = 0;
															//if ((OpcUa_String_Compare(&(pUserNameIdentityToken->EncryptionAlgorithm), &szRSA_15) == 0)
															//	|| (OpcUa_String_Compare(&(pUserNameIdentityToken->EncryptionAlgorithm), &szRSA_OAEP) == 0) )
															//{
															//}
															//else
															//{
															//	OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "EncryptionAlgorithm not supported: [ %s ]", OpcUa_String_GetRawString(&(pUserNameIdentityToken->EncryptionAlgorithm)));
															//}
														}
														if (OpcUa_ByteString_Compare(&(pValidUserIdentityToken->Password), &(aDecodedPassword)) == 0)
														{
															bUserIdentityTokenValue = OpcUa_True;
															uStatus = OpcUa_Good;
														}
														OpcUa_ByteString_Clear(&aDecodedPassword);
														OpcUa_String_Clear(&szRSA_15);
														OpcUa_String_Clear(&szRSA_OAEP);
													}
												}
											}
										}
									}
								}
							}
							else
							{
								if (OpcUa_StrCmpA(a_pUserIdentityToken->Body.EncodeableObject.Type->TypeName, "X509IdentityToken") == 0)
								{
									OpcUa_String_AttachCopy(&szAuthenticationToken, "X509IdentityToken");
									OpcUa_X509IdentityToken* pX509IdentityToken = (OpcUa_X509IdentityToken*)a_pUserIdentityToken->Body.EncodeableObject.Object;
									if (pX509IdentityToken)
									{
										// Check that the PolicyId is valid
										uStatus = OpcUa_BadIdentityTokenInvalid;
										if (OpcUa_String_IsNull(&(pX509IdentityToken->PolicyId)) == OpcUa_False)
										{
											for (OpcUa_UInt32 i = 0; i < m_pApplication->m_X509IdentityTokenList.size(); i++)
											{
												OpcUa_X509IdentityToken* pValidX509IdentityToken = m_pApplication->m_X509IdentityTokenList.at(i);
												if (OpcUa_String_Compare(&(pValidX509IdentityToken->PolicyId), &(pX509IdentityToken->PolicyId)) == 0)
												{
													uStatus = OpcUa_Good;
													break;
												}
											}
										}
										else
											uStatus = OpcUa_Good;
										if (uStatus == OpcUa_Good)
										{
											// Now check the certificate date
											{
												OpcUa_DateTime ValidFrom, ValidTo;
												OpcUa_DateTime_Initialize(&ValidFrom);
												OpcUa_DateTime_Initialize(&ValidTo);
												uStatus = OpcUa_Certificate_GetDateBound(
													&(pX509IdentityToken->CertificateData),
													&ValidFrom, &ValidTo);
												if (uStatus == OpcUa_Good)
												{
													OpcUa_DateTime utcNow = OpcUa_DateTime_UtcNow();
													OpcUa_UInt32 uDiff = 0;
													uStatus = OpcUa_P_GetDateTimeDiffInSeconds32(utcNow, ValidTo, &uDiff);
													if ((uStatus != OpcUa_BadOutOfRange) && (uStatus != OpcUa_BadInvalidArgument))
													{
														// Il n'est peut etre pas encore valide
														uStatus = OpcUa_P_GetDateTimeDiffInSeconds32(ValidFrom, utcNow, &uDiff);
														if ((uStatus != OpcUa_BadOutOfRange) && (uStatus != OpcUa_BadInvalidArgument))
														{
															// Il est peut être mal formé
															uStatus = OpcUa_P_GetDateTimeDiffInSeconds32(ValidFrom, ValidTo, &uDiff);
															if ((uStatus == OpcUa_BadOutOfRange) || (uStatus == OpcUa_BadInvalidArgument))
															{
															}
														}
														else
														{
															OpcUa_Trace(OPCUA_TRACE_LEVEL_ALWAYS, "User certificate is corrupted.\n");
															uStatus = OpcUa_BadIdentityTokenRejected;
														}
													}
													else
													{
														OpcUa_Trace(OPCUA_TRACE_LEVEL_ALWAYS, "User certificate is not yet valid. \n");
														uStatus = OpcUa_BadIdentityTokenRejected;
													}
												}
												else
												{
													OpcUa_Trace(OPCUA_TRACE_LEVEL_ALWAYS, "User certificate is expired. \n");
													uStatus = OpcUa_BadCertificateTimeInvalid;
												}
												OpcUa_DateTime_Clear(&ValidFrom);
												OpcUa_DateTime_Clear(&ValidTo);
											}
											if (uStatus == OpcUa_Good)
											{
												// Verify the certificate provided itself
												OpcUa_CharA* sThumbprint = OpcUa_Null;
												OpcUa_CharA* lApplicationUri = OpcUa_Null;// 
												OpcUa_CharA* psCommonName = OpcUa_Null;
												OpcUa_Int    iValidationCode = 0;
												if (pX509IdentityToken->CertificateData.Data)
												{
													uStatus = OpcUa_Certificate_GetInfo(
														&(pX509IdentityToken->CertificateData),
														NULL,
														NULL,
														&psCommonName,
														&sThumbprint,
														&lApplicationUri,
														NULL,
														NULL);
													if (uStatus == OpcUa_Good)
													{
														OpcUa_PKIProvider* pPKIProvider = m_pApplication->GetX509UserPkiProvider();
														if (pPKIProvider)
														{
															/* open certificate store */
															OpcUa_Void*         pCertificateStore = OpcUa_Null;
															uStatus = pPKIProvider->OpenCertificateStore(pPKIProvider,
																&pCertificateStore);
															if (uStatus == OpcUa_Good)
															{
																uStatus = pPKIProvider->ValidateCertificate(pPKIProvider, &(pX509IdentityToken->CertificateData), pCertificateStore, &iValidationCode);
																/* close certificate store */
																if (uStatus!=OpcUa_BadCertificateUntrusted) // shity workaround
																	pPKIProvider->CloseCertificateStore(pPKIProvider,
																		&pCertificateStore);
																// adjust the StatusCode
																if (uStatus != OpcUa_Good)
																	uStatus = OpcUa_BadIdentityTokenRejected;
															}
														}
													}
												}
											}
											// Just in case compare the Certificate with the application certificate.
											// This is not allowed
											// 
											OpcUa_ByteString* applicationCertificate=m_pSecureChannel->GetClientCertificate();
											if (Utils::IsEqual(&(pX509IdentityToken->CertificateData), applicationCertificate))
												uStatus = OpcUa_BadIdentityTokenRejected;
											// 
											//// Now check the certificate date
											//if (uStatus == OpcUa_Good)
											//{
											//	OpcUa_DateTime ValidFrom, ValidTo;
											//	OpcUa_DateTime_Initialize(&ValidFrom);
											//	OpcUa_DateTime_Initialize(&ValidTo);
											//	uStatus = OpcUa_Certificate_GetDateBound(
											//		&(pX509IdentityToken->CertificateData),
											//		&ValidFrom, &ValidTo);
											//	if (uStatus == OpcUa_Good)
											//	{
											//		OpcUa_DateTime utcNow = OpcUa_DateTime_UtcNow();
											//		OpcUa_UInt32 uDiff = 0;
											//		uStatus = OpcUa_P_GetDateTimeDiffInSeconds32(utcNow, ValidTo, &uDiff);
											//		if ((uStatus != OpcUa_BadOutOfRange) && (uStatus != OpcUa_BadInvalidArgument))
											//		{
											//			// Il n'est peut etre pas encore valide
											//			uStatus = OpcUa_P_GetDateTimeDiffInSeconds32(ValidFrom, utcNow, &uDiff);
											//			if ((uStatus != OpcUa_BadOutOfRange) && (uStatus != OpcUa_BadInvalidArgument))
											//			{
											//				// Il est peut être mal formé
											//				uStatus = OpcUa_P_GetDateTimeDiffInSeconds32(ValidFrom, ValidTo, &uDiff);
											//				if ((uStatus == OpcUa_BadOutOfRange) || (uStatus == OpcUa_BadInvalidArgument))
											//				{
											//				}
											//			}
											//			else
											//			{
											//				OpcUa_Trace(OPCUA_TRACE_LEVEL_ALWAYS, "User certificate is corrupted.\n");
											//				uStatus = OpcUa_BadCertificateInvalid;
											//			}
											//		}
											//		else
											//		{
											//			OpcUa_Trace(OPCUA_TRACE_LEVEL_ALWAYS, "User certificate is not yet valid. \n");
											//			uStatus = OpcUa_BadCertificateTimeInvalid;
											//		}
											//	}
											//	else
											//	{
											//		OpcUa_Trace(OPCUA_TRACE_LEVEL_ALWAYS, "User certificate is expired. \n");
											//		uStatus = OpcUa_BadCertificateTimeInvalid;
											//	}
											//	OpcUa_DateTime_Clear(&ValidFrom);
											//	OpcUa_DateTime_Clear(&ValidTo);
											//}
											pX509IdentityToken->PolicyId;
											bUserIdentityTokenValue = OpcUa_True;
										}
									}
								}
								else
								{
									uStatus = OpcUa_BadIdentityTokenInvalid;
									OpcUa_String_AttachCopy(&szAuthenticationToken, "IdentityTokenInvalid");
								}
							}
						}
						if (m_pSessionSecurityDiagnostics)
							m_pSessionSecurityDiagnostics->SetAuthenticationMechanism(&szAuthenticationToken);
						OpcUa_String_Clear(&szAuthenticationToken);
					}
					else
						uStatus = OpcUa_BadInvalidArgument;
				}
			}
			//if (bUserIdentityTokenValue)
			if (uStatus==OpcUa_Good)
			{
				m_bSessionActive = OpcUa_True;
				// create the signature and nonce.
				if (m_pSecureChannel->GetSecurityMode() != OPCUA_SECURECHANNEL_MESSAGESECURITYMODE_NONE)
				{
					// validate a signature provided by client.
					OpcUa_ByteString* pClientCertificate = m_pSecureChannel->GetClientCertificate();
					if (pClientCertificate)
					{
						CryptoUtils::VerifySignature(
							&m_tCryptoProvider,
							m_pApplication->GetCertificate(), // certificat de l'application a verifier (client ou serveur)
							m_pServerNonce,
							pClientCertificate,
							pClientSignature);


						// generate a new nonce.
						tServerNonce.Key.Length = 32;
						tServerNonce.Key.Data = (OpcUa_Byte*)OpcUa_Alloc(tServerNonce.Key.Length);

						uStatus = OpcUa_Crypto_GenerateKey(&m_tCryptoProvider, tServerNonce.Key.Length, &tServerNonce);
						if (uStatus != OpcUa_Good)
						{
							OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Could not create server nonce.\n");
							m_bSessionActive = OpcUa_False;
						}
						else
						{
							OpcUa_ByteString_Clear(m_pServerNonce);
							OpcUa_ByteString_CopyTo(&(tServerNonce.Key), m_pServerNonce);
							OpcUa_ByteString_CopyTo(m_pServerNonce, pServerNonce);
							if (m_bServerDiagnosticsEnabled)
							{
								// update m_pSessionSecurityDiagnostics
								m_pSessionSecurityDiagnostics->SetSecurityMode((OpcUa_MessageSecurityMode)securityMode); // 
								m_pSessionSecurityDiagnostics->SetSecurityPolicyUri(securityPolicyUri);
								m_pSessionSecurityDiagnostics->SetClientCertificate(pClientCertificate);
							}

							if (!bUserIdentityTokenValue)
							{
								if (uStatus==OpcUa_Good)
									uStatus = OpcUa_BadIdentityTokenInvalid;
								m_bSessionActive = OpcUa_False;
							}
								
						}
						OpcUa_Key_Clear(&tServerNonce);
					}
					else
					{
						m_bSessionActive = OpcUa_False;
						uStatus = OpcUa_BadCertificateInvalid;
					}
				}
			}
			//else
			//	uStatus = OpcUa_BadUserAccessDenied;
		}
	}
	return uStatus;
}

//============================================================================
// CSessionServer::IsAuthenticationToken
//============================================================================
OpcUa_Boolean CSessionServer::IsAuthenticationToken(const OpcUa_NodeId* pAuthenticationToken)
{
	return Utils::IsEqual(pAuthenticationToken, m_pAuthenticationToken);
}
OpcUa_StatusCode CSessionServer::CreateSubscription(OpcUa_UInt32 uSecureChannelId,
													OpcUa_Double dblRequestedPublishingInterval,
													OpcUa_UInt32 uiRequestedLifetimeCount,
													OpcUa_UInt32 uiRequestedMaxKeepAliveCount,
													OpcUa_UInt32 uiMaxNotificationsPerPublish,
													OpcUa_Boolean bPublishingEnabled,
													OpcUa_Byte cPriority,
													CSubscriptionServer** pSubscription)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	CUAInformationModel* pInformationModel = CServerApplication::m_pTheAddressSpace;
	OpcUa_ReferenceParameter(bPublishingEnabled);
	OpcUa_ReferenceParameter(uSecureChannelId);
	if (m_SubscriptionList.size()<MAX_SUBSCRIPTION_PER_SESSION)
	{
		CSubscriptionServer* pNewSubscription=new CSubscriptionServer(this,dblRequestedPublishingInterval);
		if (pNewSubscription)
		{
			// save the subscriptionId
			pNewSubscription->SetHandle((OpcUa_Handle)pNewSubscription);
			// 
			pNewSubscription->SetPublishingEnabled(bPublishingEnabled);
			// Ajuste le PublishingInterval
			if (dblRequestedPublishingInterval==0) // 
				pNewSubscription->SetPublishingInterval((OpcUa_Double)PUBLISHING_INTERVAL_MINI);
			else
			{
				// Verification que l'intervalle de publication demandé n'est pas inferieur a PUBLISHING_INTERVAL_MINI
				if (dblRequestedPublishingInterval<(OpcUa_Double)PUBLISHING_INTERVAL_MINI) // 
					pNewSubscription->SetPublishingInterval((OpcUa_Double)PUBLISHING_INTERVAL_MINI);
				else
				{
					if (dblRequestedPublishingInterval>(OpcUa_Double)PUBLISHING_INTERVAL_MAX) // 49 jours max
						pNewSubscription->SetPublishingInterval((OpcUa_Double)PUBLISHING_INTERVAL_MAX);
					else
					{
#ifdef WIN32
						if (_isnan(dblRequestedPublishingInterval))
#endif
#ifdef _GNUC_
						if (isnan(dblRequestedPublishingInterval))
#endif
							pNewSubscription->SetPublishingInterval((OpcUa_Double)PUBLISHING_INTERVAL_MINI);
						else
							pNewSubscription->SetPublishingInterval(dblRequestedPublishingInterval);
					}
				}
			}

			// MaxKeepAliveCount
			if (uiRequestedMaxKeepAliveCount>MAX_KEEPALIVE_COUNT_MAX)
				pNewSubscription->SetMaxKeepAliveCount(MAX_KEEPALIVE_COUNT_MAX);
			else
			{
				if (uiRequestedMaxKeepAliveCount==0)
					pNewSubscription->SetMaxKeepAliveCount(MAX_KEEPALIVE_COUNT_MINI);
				else
					pNewSubscription->SetMaxKeepAliveCount(uiRequestedMaxKeepAliveCount);
			}
			OpcUa_UInt32 RevisedMaxKeepAliveCount=pNewSubscription->GetMaxKeepAliveCount();
			// LifetimeCount;
			if ( ((RevisedMaxKeepAliveCount)*3)<=(uiRequestedLifetimeCount))
				pNewSubscription->SetLifetimeCount(uiRequestedLifetimeCount);
			else
			{
				if ((((RevisedMaxKeepAliveCount)*3)<uiRequestedLifetimeCount) && ( ((RevisedMaxKeepAliveCount)*3)<MAX_KEEPALIVE_COUNT_MAX) )
					pNewSubscription->SetLifetimeCount(uiRequestedLifetimeCount);
				else
					pNewSubscription->SetLifetimeCount((RevisedMaxKeepAliveCount)*3);
			}
			pNewSubscription->SetPriority(cPriority);
			pNewSubscription->SetMaxNotificationsPerPublish(uiMaxNotificationsPerPublish);
			// Monitoring mode par défaut
			//OpcUa_MonitoringMode_Disabled  = 0
			//OpcUa_MonitoringMode_Sampling  = 1
			//OpcUa_MonitoringMode_Reporting = 2
			pNewSubscription->SetMonitoringMode(OpcUa_MonitoringMode_Reporting);
			// On doit être certain que le n° de souscription est unique pour cette session
			uStatus=AddSubscription(pNewSubscription);
			if (uStatus == OpcUa_Good)
			{
				UpdateAsyncThreadInterval();
				//
				UpdateNotificationDataThreadInterval();
				SortSubscription();
			}
			// save the subscription to answer the client
			*pSubscription=pNewSubscription;
			OpcUa_Boolean bServerDiagnosticsEnabled = pInformationModel->IsEnabledServerDiagnosticsDefaultValue();
			if (bServerDiagnosticsEnabled)
			{
				// Transfert dans le node d'audit
				uStatus = InitSubscriptionDiagnosticsDataType(pNewSubscription);
				if (uStatus != OpcUa_Good)
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "InitSubscriptionDiagnosticsDataType failed: 0x%05x\n", uStatus);
				else
					pNewSubscription->UpdateSubscriptionDiagnosticsDataType();
			}
			// On reveil la thread de surveillance de LifeTimeCount afin qu'elle mette a jour son timer
			OpcUa_Semaphore_Post(m_SubscriptionsLifeTimeCountSem,1);
		}
		else
			uStatus=OpcUa_BadOutOfMemory;
	}
	else
		uStatus=OpcUa_BadTooManySubscriptions;

	return uStatus;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Searches for the first subscription with the subscriptionId passed in parameter. 
/// 			here the mutex m_hSubscriptionListMutex is supposed to be locked by the caller</summary>
///
/// <remarks>	Michel, 01/02/2016. </remarks>
///
/// <param name="aSubscriptionId">	Identifier for the subscription. </param>
/// <param name="ppSubscription"> 	[in,out] If non-null, the subscription. </param>
///
/// <returns>	The found subscription. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode CSessionServer::FindSubscription(OpcUa_UInt32 aSubscriptionId, CSubscriptionServer** ppSubscription)
{
	OpcUa_StatusCode uStatus=OpcUa_BadSubscriptionIdInvalid;
	if (m_SubscriptionList.empty())
		uStatus=OpcUa_BadSubscriptionIdInvalid;
	else
	{
		for (OpcUa_UInt32 ii=0;ii<m_SubscriptionList.size();ii++)
		{
			CSubscriptionServer* pSubscription=m_SubscriptionList.at(ii);
			if (pSubscription->GetSubscriptionId()==aSubscriptionId)
			{
				*ppSubscription=pSubscription;
				uStatus=OpcUa_Good;
				break;
			}
		}
	}
	return uStatus;
}

OpcUa_StatusCode CSessionServer::FindNextSubscriptionToNotify(CSubscriptionServer** ppSubscription)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	if (m_SubscriptionList.empty())
		uStatus=OpcUa_BadNoData;
	for (OpcUa_UInt32 ii=0;ii<m_SubscriptionList.size();ii++)
	{
		CSubscriptionServer* pSubscription=m_SubscriptionList.at(ii);
		if (pSubscription->IsChanged())
		{
			*ppSubscription=pSubscription;
			return OpcUa_Good;
		}
	}

	return OpcUa_BadNoData;
}
void CSessionServer::StartSessionTimeoutThread()
{
	if (m_hSessionTimeoutThread==NULL)
	{
		OpcUa_Semaphore_Create(&m_StopSessionTimeoutSem, 0, 0x100);
		OpcUa_Semaphore_Create(&m_SessionTimeoutSem,0,0x100);
		m_bRunSessionTimeoutThread=OpcUa_True;
		OpcUa_StatusCode uStatus  = OpcUa_Thread_Create(&m_hSessionTimeoutThread,(CSessionServer::SessionTimeoutThread),this);

		if(uStatus != OpcUa_Good)
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Create SessionTimeoutThread Failed");
		else
			OpcUa_Thread_Start(m_hSessionTimeoutThread);
	}
}
void CSessionServer::StopSessionTimeoutThread()
{
	//if (m_bRunSessionTimeoutThread)
	{
		m_bRunSessionTimeoutThread=OpcUa_False;
		OpcUa_Semaphore_Post(m_SessionTimeoutSem,1);
		OpcUa_StatusCode uStatus = OpcUa_Semaphore_TimedWait(m_StopSessionTimeoutSem, OPC_TIMEOUT * 2); // 15 secondes max.
		if (uStatus == OpcUa_GoodNonCriticalTimeout)
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Impossible to stop the SessionTimeoutThread. Timeout\n");
		else
		{
			OpcUa_Thread_Delete(m_hSessionTimeoutThread);
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_INFO, "SessionTimeoutThread stopped properly\n");
		}
	}
}
void CSessionServer::SessionTimeoutThread(LPVOID arg)
{
	CSessionServer* pSessionServer=(CSessionServer*)arg;
	OpcUa_StatusCode uStatus=OpcUa_Good;
	double dblInterval=pSessionServer->GetSessionTimeout();
	OpcUa_UInt32 uiInterval = (OpcUa_UInt32)dblInterval;
	if (uiInterval==0)
		uiInterval=30000; // 30 sec
	OpcUa_UInt32 uiSleepTime=100;
	while(pSessionServer->m_bRunSessionTimeoutThread)
	{
#if _WIN32_WINNT >= 0x0600
		ULONGLONG dwStart = GetTickCount64();
#else 
	#if _WIN32_WINNT == 0x0501
		DWORD dwStart = GetTickCount();
	#endif
#endif
#ifdef _GNUC_
		DWORD dwStart = GetTickCount();
#endif
		// calcul du nouvel interval relatif (dwSleepTime)
#if _WIN32_WINNT >= 0x0600
		ULONGLONG dwEnd = GetTickCount64();
		ULONGLONG dwCountedTime = dwEnd - dwStart;
		if (uiInterval>dwCountedTime)
			uiSleepTime = (OpcUa_UInt32)(uiInterval - dwCountedTime);
		else
			uiSleepTime = 0;
#else 
	#if _WIN32_WINNT == 0x0501
		DWORD dwEnd = GetTickCount();
		DWORD dwCountedTime = dwEnd - dwStart;
		if (uiInterval>dwCountedTime)
			uiSleepTime = uiInterval - dwCountedTime;
		else
			uiSleepTime = 0;
	#endif
#endif
#ifdef _GNUC_
		DWORD dwEnd=GetTickCount();
		DWORD dwCountedTime=dwEnd-dwStart;
		if (uiInterval>dwCountedTime)
			uiSleepTime=uiInterval-dwCountedTime;
		else
			uiSleepTime=0;
#endif
		
		uStatus=OpcUa_Semaphore_TimedWait( pSessionServer->m_SessionTimeoutSem, uiSleepTime );  // on attend jusqu'a qu'une demande async soit posé dans la queue
		if (uStatus == OpcUa_GoodNonCriticalTimeout)
		{
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "A session is in timeout. It will be deleted in the next loop of the CServerApplication::SessionTimeoutThread\n");
			pSessionServer->m_bRunSessionTimeoutThread = OpcUa_False; // we just reach the timeout. so the session will die
		}
		uiInterval=(DWORD)pSessionServer->GetSessionTimeout();		
	}
	OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_INFO, "CSessionServer::SessionTimeoutThread stopped\n");
	OpcUa_Semaphore_Post( pSessionServer->m_StopSessionTimeoutSem,1);
}


// Function name   : CSessionServer::UpdateAsyncThreadInterval
// Description     : Mise a jour de la cadence de rafraissement des demandes asynchrones.
//					 Il s'agit de la vitesse à laquelle la thread AsyncRequestThread est executée
// Return type     : void 

void CSessionServer::UpdateAsyncThreadInterval()
{
	OpcUa_Double dblArbitraryIntervalValue = 30000.00;
	OpcUa_Mutex_Lock(m_hSubscriptionListMutex);
	for (OpcUa_UInt32 ii=0;ii<m_SubscriptionList.size();ii++)
	{
		CSubscriptionServer* pSubscription=m_SubscriptionList.at(ii);
		if (dblArbitraryIntervalValue>pSubscription->GetPublishingInterval())
		{
			dblArbitraryIntervalValue = pSubscription->GetPublishingInterval();
		}
	}
	if (m_SubscriptionList.size()==0)
		dblArbitraryIntervalValue = MAX_ASYNC_THREAD_INTERVAL;
	SetAsyncThreadInterval((DWORD)dblArbitraryIntervalValue);
	OpcUa_Mutex_Unlock(m_hSubscriptionListMutex);
}

DWORD CSessionServer::GetAsyncThreadInterval() 
{ 
	return m_dwAsyncThreadInterval; 
}
DWORD CSessionServer::GetNotificationDataInterval()
{ 
	return m_dwNotificationDataThreadInterval;
}
void CSessionServer::UpdateNotificationDataThreadInterval()
{
	OpcUa_Double dblArbitraryIntervalValue = 30000.00;
	OpcUa_Mutex_Lock(m_hSubscriptionListMutex);
	for (OpcUa_UInt32 ii = 0; ii<m_SubscriptionList.size(); ii++)
	{
		CSubscriptionServer* pSubscription = m_SubscriptionList.at(ii);
		if (dblArbitraryIntervalValue>pSubscription->GetPublishingInterval())
		{
			dblArbitraryIntervalValue = pSubscription->GetPublishingInterval();
		}
	}
	if (m_SubscriptionList.size() == 0)
		dblArbitraryIntervalValue = DEFAULT_NOTIFICATION_INTERVAL;
	m_dwNotificationDataThreadInterval=(DWORD)dblArbitraryIntervalValue;
	OpcUa_Mutex_Unlock(m_hSubscriptionListMutex);
}
void CSessionServer::SetAsyncThreadInterval(DWORD dwVal) 
{ 
	m_dwAsyncThreadInterval = dwVal; 
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Notification data thread. </summary>
///				This thread is in charge of sending notification message to the client. 
/// <remarks>	Michel, 26/01/2016. </remarks>
///
/// <param name="arg">	This is a pointer to a CSessionServer. </param>
///-------------------------------------------------------------------------------------------------

void CSessionServer::NotificationDataThread(LPVOID arg)
{
	CSessionServer* pSessionServer = (CSessionServer*)arg;
	OpcUa_StatusCode uStatus = OpcUa_Good;
	DWORD uiInterval = pSessionServer->GetNotificationDataInterval();
	DWORD uiSleepTime = 1000;
	OpcUa_Boolean bImmediateNotification = OpcUa_False;
	while (pSessionServer->m_bRunAsyncRequestThread)
	{
		// Management of the publish Response
		pSessionServer->CleanupTimeoutedPublishRequest();
		OpcUa_Mutex_Lock(pSessionServer->m_hSubscriptionListMutex);
		uiInterval = pSessionServer->GetNotificationDataInterval();
#if _WIN32_WINNT >= 0x0600
		ULONGLONG dwStart = GetTickCount64();
#else 
#if _WIN32_WINNT == 0x0501
		DWORD dwStart = GetTickCount();
#endif
#endif
#ifdef _GNUC_
		DWORD dwStart = GetTickCount();
#endif	
		OpcUa_UInt32 uiNoOfSubscription = pSessionServer->m_SubscriptionList.size();
		if (uiNoOfSubscription > 0)
		{
			// Now prepare the Notification
			for (OpcUa_UInt16 ii = 0; ii < uiNoOfSubscription; ii++)
			{
				CSubscriptionServer* pSubscription = pSessionServer->m_SubscriptionList.at(ii);				

				CQueuedPublishMessage* pRequest = pSessionServer->GetFirstPublishRequest();
				if (pRequest)
				{
					OpcUa_Mutex_Lock(pSessionServer->m_hPublishMutex);
					if (pSessionServer->m_PublishRequests.size() >= MAX_PUBLISH_PER_SESSION)
					{
						pRequest->SetServiceResult(OpcUa_BadTooManyPublishRequests);
					}
					// check for abort the request.
					OpcUa_Handle pContextHandle = pRequest->GetContext();
					if (!pContextHandle)
						OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_WARNING, "CSubscriptionServer::PQPR>m_hContext NULL critical error\n");
					else
					{
						uStatus = OpcUa_Good;
						OpcUa_Boolean bAbort = OpcUa_False; // check to imlement Abort according to the call of Server_Cancel
						if (bAbort)
							uStatus = pRequest->CancelSendResponse();
						else
						{
							uStatus = pRequest->BeginSendResponse();
							if (uStatus == OpcUa_Good)
							{
								// Now prepare the Notification
								uStatus = pSubscription->ProcessQueuedPublishRequestEx(pRequest, OpcUa_False);
								if (uStatus != OpcUa_BadNothingToDo)
								{
									if ((pRequest->GetInternalPublishResponse()->NotificationMessage.NoOfNotificationData > 0) || (pRequest->IsKeepAlive()))
									{
										// Transmission 
										uStatus = pRequest->EndSendResponse();
										if (uStatus != OpcUa_Good)
										{
											OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_WARNING, "PQPR>EndSendResponse failed. Status 0x%08X\n", uStatus);
											// Mark the request for deletion
											pRequest->SetDeleted(OpcUa_True);
										}
										else
										{
											// Subscription DiagnosticData
											
											CSubscriptionDiagnosticsDataType* pSubscriptionDiagnosticsDataType= pSubscription->GetSubscriptionDiagnosticsDataType();
											if (pSubscriptionDiagnosticsDataType)
											{
												pSubscriptionDiagnosticsDataType->SetDataChangeNotificationsCount(pSubscriptionDiagnosticsDataType->GetDataChangeNotificationsCount() + 1);
												pSubscriptionDiagnosticsDataType->SetNotificationsCount(pSubscriptionDiagnosticsDataType->GetNotificationsCount() + 1);
												pSubscriptionDiagnosticsDataType->SetUnacknowledgedMessageCount(pSubscriptionDiagnosticsDataType->GetUnacknowledgedMessageCount() + 1);
												pSubscription->UpdateSubscriptionDiagnosticsDataType();
											}
											bImmediateNotification = OpcUa_False;
											// On vient d'envoyer un notification avec des données ou un keepAlive. On reset le LifeTimeCounter
											pSubscription->SetKeepAlive(OpcUa_False);
											pSubscription->SetInColdState(OpcUa_False);
											pSubscription->ResetLifeTimeCountCounter();
											pSubscription->SetLastPublishAnswer(GetTickCount());
											OpcUa_DateTime dtTransmission = OpcUa_DateTime_UtcNow();
											OpcUa_CharA* pBufTime;
											pBufTime = (OpcUa_CharA*)OpcUa_Alloc(50);
											ZeroMemory(pBufTime, 50);
											OpcUa_DateTime_GetStringFromDateTime(dtTransmission, pBufTime, 50);
											pSessionServer->SetLastTransmissionTime(dtTransmission);
											pSubscription->SetLastTransmissionTime(dtTransmission);
											OpcUa_Free(pBufTime);
											pSubscription->InLate(OpcUa_False);
											pSubscription->SetInColdState(OpcUa_False);
											pRequest->SetDeleted(OpcUa_True);
											////////////////////////////////////////
											// cleanup the Request
											pRequest->EncodeableObjectDelete();
											// Delete the Acknowledged DataChangeNotification 
											pSubscription->RemoveAckedDataChangeNotification();
											pSubscription->RemoveAckedStatusChangeNotification();
											pSubscription->RemoveAckedEventNotificationList();
										}
									}
									else
									{
										pRequest->ResetPublishResponse();
										bImmediateNotification = OpcUa_False;
									}
								}
								else
								{
									bImmediateNotification = OpcUa_False;
									//if (pRequest->IsTimeouted())
									//	pRequest->CancelSendResponse();
									//else
									pRequest->ResetPublishResponse();
									/* Workaround efficace
									pRequest->FillResponseHeader();
									pRequest->EndSendResponse();
									pRequest->SetDeleted(OpcUa_True);
									pRequest->EncodeableObjectDelete();
									*/
								}
							}
							else
							{
								OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "PQPR>BeginSendResponse failed 0x%05x. The publish request will be deleted\n", uStatus);
							}
						}
					}
					OpcUa_Mutex_Unlock(pSessionServer->m_hPublishMutex);
				}
				else
				{
					//OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "AsyncRequestThread. Cannot find a publish request in the queue for subscription %u\n", pSubscription->GetSubscriptionId());
					if (pSessionServer->m_SubscriptionList.size()>0)
					{
						bImmediateNotification = OpcUa_True;
						pSubscription->InLate(OpcUa_True);
					}
					else
						bImmediateNotification = OpcUa_False;
					//uiSleepTime = 50;
				}
			}
		}
		// End Management of the publish Response

		// Check if the new value of the interval changed. 
		// Mainly if it is now bigger. 
		// If it become smaller we show be wake up sleeping
		//OpcUa_UInt32 uiOldInterval = uiInterval;
		uiInterval = pSessionServer->GetNotificationDataInterval();// 
		OpcUa_Mutex_Unlock(pSessionServer->m_hSubscriptionListMutex);
		if (!bImmediateNotification)
		{
#if _WIN32_WINNT >= 0x0600
			ULONGLONG dwEnd = GetTickCount64();
			ULONGLONG dwCountedTime = dwEnd - dwStart;
			if (uiInterval > dwCountedTime)
				uiSleepTime = (OpcUa_UInt32)(uiInterval - dwCountedTime);
			else
				uiSleepTime = 0;
#else 
#if _WIN32_WINNT == 0x0501
			DWORD dwEnd = GetTickCount();
			DWORD dwCountedTime = dwEnd - dwStart;
			if (uiInterval>dwCountedTime)
				uiSleepTime = uiInterval - dwCountedTime;
			else
				uiSleepTime = 0;
#endif
#endif
#ifdef _GNUC_
			DWORD dwEnd = GetTickCount();
			DWORD dwCountedTime = dwEnd - dwStart;
			if (uiInterval > dwCountedTime)
				uiSleepTime = uiInterval - dwCountedTime;
			else
				uiSleepTime = 0;
#endif	
		}
		else
			uiSleepTime = uiInterval;
		if (pSessionServer->m_hPublishSem)
			OpcUa_Semaphore_Post(pSessionServer->m_hPublishSem, 1);
		if (pSessionServer->m_hNotificationDataSem)
		{
			OpcUa_Semaphore_TimedWait(pSessionServer->m_hNotificationDataSem, uiSleepTime);  //  We wait until timeout or a m_hNotificationDataSem is posted
		}
	}
	OpcUa_Semaphore_Post(pSessionServer->m_hStopNotificationDataThreadSem, 1);
	OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_WARNING, "CSessionServer::NotificationDataThread stopped\n");
}
// This Thread support asynchronous call for Read, HistoryRead, Call, PublishResponse, QueryFirst, QueryNext
void CSessionServer::AsyncRequestThread(LPVOID arg)
{
	CSessionServer* pSessionServer=(CSessionServer*)arg;
	OpcUa_StatusCode uStatus=OpcUa_Good;
	DWORD uiInterval=pSessionServer->GetAsyncThreadInterval();
	DWORD uiSleepTime=100;
	OpcUa_Boolean bImmediateNotification = OpcUa_False;
	while(pSessionServer->m_bRunAsyncRequestThread)
	{
		uiInterval = pSessionServer->GetAsyncThreadInterval();
		bImmediateNotification = OpcUa_False;
#if _WIN32_WINNT >= 0x0600
		ULONGLONG dwStart = GetTickCount64();
#else 
	#if _WIN32_WINNT == 0x0501
		DWORD dwStart = GetTickCount();
	#endif
#endif
#ifdef _GNUC_
		DWORD dwStart = GetTickCount();
#endif	
		try
		{
			//  Management of asynchrone request for Read
			OpcUa_Mutex_Lock(pSessionServer->m_hReadMutex);
			CQueuedReadMessages* pReadMessages=pSessionServer->m_pReadMessages;
			OpcUa_UInt32 uiSize = pReadMessages->size();
			for (OpcUa_UInt32 ii=0;ii<uiSize;ii++)
			{
				CQueuedReadMessage* pRequest =pReadMessages->at(ii);
				if (!pRequest->IsDeleted())
				{
					pSessionServer->ProcessQueuedReadRequest(pRequest,OpcUa_False);
					//break;
				}
			}
			OpcUa_Mutex_Unlock(pSessionServer->m_hReadMutex);
			//  Management of asynchrone request for HistoryRead
			OpcUa_Mutex_Lock(pSessionServer->m_hHistoryReadMutex);
			CQueuedHistoryReadMessages* pHistoryReadMessages=pSessionServer->m_pHistoryReadMessages;
			uiSize=pHistoryReadMessages->size();
			for (OpcUa_UInt32 ii = 0; ii < uiSize; ii++)
			{
				CQueuedHistoryReadMessage* pHistoryReadMessage = pHistoryReadMessages->at(ii);
				if (!pHistoryReadMessage->IsDeleted())
				{
					pSessionServer->ProcessQueuedHistoryReadMessage(pHistoryReadMessage, OpcUa_False);
					break;
				}
			}
			OpcUa_Mutex_Unlock(pSessionServer->m_hHistoryReadMutex);
			// Management of asynchrone request for Method call
			OpcUa_Mutex_Lock(pSessionServer->m_hCallMutex);
			CQueuedCallMessages* pCallMessages = pSessionServer->m_pCallMessages;
			uiSize = pCallMessages->size();
			for (OpcUa_UInt32 ii = 0; ii < uiSize; ii++)
			{
				CQueuedCallMessage* pCallMessage = pCallMessages->at(ii);
				if (!pCallMessage->IsDeleted())
				{
					pSessionServer->ProcessQueuedCallRequest(pCallMessage, OpcUa_False);
					break;
				}
			}
			OpcUa_Mutex_Unlock(pSessionServer->m_hCallMutex);
			// Management of asynchrone request for QueryFirst
			OpcUa_Mutex_Lock(pSessionServer->m_hQueryFirstMutex);
			CQueuedQueryFirstMessages* pQueryFirstMessages = pSessionServer->m_pQueryFirstMessages;
			uiSize = pQueryFirstMessages->size();
			for (OpcUa_UInt32 ii = 0; ii < uiSize; ii++)
			{
				CQueuedQueryFirstMessage* pQueryFirstMessage = pQueryFirstMessages->at(ii);
				if (!pQueryFirstMessage->IsDeleted())
				{
					pSessionServer->ProcessQueuedQueryFirstRequest(pQueryFirstMessage, OpcUa_False);
					break;
				}
			}
			OpcUa_Mutex_Unlock(pSessionServer->m_hQueryFirstMutex);
			// Management of asynchrone request for QueryNext
			OpcUa_Mutex_Lock(pSessionServer->m_hQueryNextMutex);
			CQueuedQueryNextMessages* pQueryNextMessages = pSessionServer->m_pQueryNextMessages;
			uiSize = pQueryNextMessages->size();
			for (OpcUa_UInt32 ii = 0; ii < uiSize; ii++)
			{
				CQueuedQueryNextMessage* pQueryNextMessage = pQueryNextMessages->at(ii);
				if (!pQueryNextMessage->IsDeleted())
				{
					pSessionServer->ProcessQueuedQueryNextRequest(pQueryNextMessage, OpcUa_False);
					break;
				}
			}
			OpcUa_Mutex_Unlock(pSessionServer->m_hQueryNextMutex);

#if _WIN32_WINNT >= 0x0600
				ULONGLONG dwEnd = GetTickCount64();
				ULONGLONG dwCountedTime = dwEnd - dwStart;
				if (uiInterval>dwCountedTime)
					uiSleepTime = (OpcUa_UInt32)(uiInterval - dwCountedTime);
				else
					uiSleepTime = 0;
#else 
#if _WIN32_WINNT == 0x0501
				DWORD dwEnd = GetTickCount();
				DWORD dwCountedTime = dwEnd - dwStart;
				if (uiInterval>dwCountedTime)
					uiSleepTime = uiInterval - dwCountedTime;
				else
					uiSleepTime = 0;
#endif
#endif
#ifdef _GNUC_
				DWORD dwEnd = GetTickCount();
				DWORD dwCountedTime = dwEnd - dwStart;
				if (uiInterval > dwCountedTime)
					uiSleepTime = uiInterval - dwCountedTime;
				else
					uiSleepTime = 0;
#endif	
			if (pSessionServer->m_hPublishSem)
				OpcUa_Semaphore_Post(pSessionServer->m_hPublishSem, 1);
			OpcUa_Semaphore_TimedWait(pSessionServer->m_hAsyncRequest, uiSleepTime);  //  We wait until timeout or a m_hAsyncRequest is posted
		}
		catch (CStatusCodeException e)
		{
			uStatus = e.GetCode();
		}	
	}
	OpcUa_Semaphore_Post( pSessionServer->m_hStopAsyncRequestThreadSem,1);
	OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_WARNING,"CSessionServer::AsyncRequestThread stopped\n");
}
// Stub Method to start the ASyncRequestThread
void CSessionServer::StartAsyncRequestThread()
{
	if (m_hAsyncRequestThread == OpcUa_Null)
	{
		m_bRunAsyncRequestThread = OpcUa_True;
		OpcUa_StatusCode uStatus = OpcUa_Thread_Create(&m_hAsyncRequestThread, (CSessionServer::AsyncRequestThread), this);

		if (uStatus != OpcUa_Good)
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Create AsyncRequestThread Failed");
		else
			OpcUa_Thread_Start(m_hAsyncRequestThread);
	}
}
// Method to Stop the ASyncRequestThread
OpcUa_StatusCode CSessionServer::StopAsyncRequestThread()
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	m_bRunAsyncRequestThread=OpcUa_False;
	OpcUa_Mutex_Lock(m_hReadMutex);

	OpcUa_Semaphore_Post(m_hAsyncRequest,1);
	uStatus = OpcUa_Semaphore_TimedWait( m_hStopAsyncRequestThreadSem,OPC_TIMEOUT*2); // 15 secondes max.
	if (uStatus == OpcUa_GoodNonCriticalTimeout)
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Impossible to stop the AsyncRequestThread. Timeout");
	else
	{
		OpcUa_Thread_Delete(m_hAsyncRequestThread);
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_WARNING, "AsyncRequestThread stopped properly\n");
	}
	OpcUa_Mutex_Unlock(m_hReadMutex);
	return uStatus;
}
// Stub Method to start the ASyncRequestThread
void CSessionServer::StartNotificationDataThread()
{
	if (m_hNotificationDataThread == OpcUa_Null)
	{
		m_bRunNotificationDataThread = OpcUa_True;
		OpcUa_StatusCode uStatus = OpcUa_Thread_Create(&m_hNotificationDataThread, (CSessionServer::NotificationDataThread), this);

		if (uStatus != OpcUa_Good)
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Create AsyncRequestThread Failed");
		else
			OpcUa_Thread_Start(m_hNotificationDataThread);
	}
}
// Method to Stop the ASyncRequestThread
OpcUa_StatusCode CSessionServer::StopNotificationDataThread()
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	m_bRunNotificationDataThread = OpcUa_False;

	OpcUa_Semaphore_Post(m_hNotificationDataSem, 1);
	uStatus = OpcUa_Semaphore_TimedWait(m_hStopNotificationDataThreadSem, OPC_TIMEOUT * 2); // 15 secondes max.
	if (uStatus == OpcUa_GoodNonCriticalTimeout)
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Impossible to stop the NotificationDataThread. Timeout");
	else
	{
		OpcUa_Thread_Delete(m_hNotificationDataThread);
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_WARNING, "NotificationDataThread stopped properly\n");
	}
	return uStatus;
}
OpcUa_StatusCode CSessionServer::NotifyWhenNoSubscription(CQueuedPublishMessage* pPublishMessage)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	// Traitement du cas ou l'on fait un publish alors qu"aucune souscription n'existe sur ce serveur (CTT Compliance)
	pPublishMessage->SetKeepAlive(OpcUa_True);
	if (pPublishMessage->GetInternalPublisRequest()->NoOfSubscriptionAcknowledgements == 0)
		pPublishMessage->SetServiceResult(OpcUa_BadNoSubscription); //   OpcUa_Good
	else
		pPublishMessage->SetServiceResult(OpcUa_BadNoSubscription);
	uStatus = pPublishMessage->BeginSendResponse();
	if (uStatus == OpcUa_Good)
	{
		// Check that some m_StatusCodes corresponding to StatusChangeNotification need to be notify to the client
		if (m_StatusCodes.size() > 0)
		{
			// will add one by one StatusChangeNotification
			// préparation d'un OpcUa_StatusChangeNotification. Ils seront placés dans un NotificationData (OpcUa_ExtensionObject)
			OpcUa_StatusChangeNotification* pNotification =
				(OpcUa_StatusChangeNotification*)OpcUa_Alloc(sizeof(OpcUa_StatusChangeNotification));
			OpcUa_StatusChangeNotification_Initialize(pNotification);
			OpcUa_UInt32 iSize = m_StatusCodes.size();
			//for (OpcUa_UInt32 i=0; i<iSize; i++)
			if (iSize > 0)
			{
				OpcUa_DiagnosticInfo_Initialize(&(pNotification->DiagnosticInfo));
				pNotification->Status = m_StatusCodes.at(0); // (i)
				m_StatusCodes.erase(m_StatusCodes.begin());
				//break;
			}
			pPublishMessage->FillStatusChangeNotificationMessage(pNotification);
		}
		uStatus = pPublishMessage->FillNotificationMessage(0);
		if (uStatus != OpcUa_Good)
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Publish but no subscription>FillNotificationMessage failed 0x%05x\n", uStatus);
		uStatus = pPublishMessage->EndSendResponse();
		if (uStatus != OpcUa_Good)
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Publish but no subscription>EndSendResponse failed 0x%05x\n", uStatus);
	}
	else
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Publish but no subscription>BeginSendResponse failed 0x%05x\n", uStatus);
	return uStatus;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Process the publish request. </summary>
///
/// <remarks>	Michel, 01/02/2016. </remarks>
///
/// <param name="pPublishRequest">	[in,out] If non-null, the publish request. </param>
/// <param name="hEndpoint">	  	The endpoint. </param>
/// <param name="hContext">		  	The context. </param>
/// <param name="pRequestType">   	[in,out] If non-null, type of the request. </param>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode CSessionServer::ProcessPublishRequest(OpcUa_PublishRequest* pPublishRequest, OpcUa_Endpoint hEndpoint, OpcUa_Handle hContext, OpcUa_EncodeableType* pRequestType)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	if (GetPublishRequestsSize()>0)
	{
		if (OpcUa_Semaphore_TimedWait(m_hPublishSem, OPC_TIMEOUT * 2) == OpcUa_GoodNonCriticalTimeout)
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "AckAndQueuePublishRequest LEAVE on TIMEOUT... this is not good. Contact Michel Condemine\n");
	}
	CQueuedPublishMessage* pPublishMessage = new CQueuedPublishMessage(pPublishRequest, hEndpoint, hContext, pRequestType);
	if (pPublishMessage)
	{
		/////////////////////////////////////////////
		if (GetSubscriptionListSize() == 0)
		{
			uStatus = NotifyWhenNoSubscription(pPublishMessage);
			delete pPublishMessage;
			pPublishMessage = OpcUa_Null;
		}
		else
		{
			/////////////////////////////////////////////
			OpcUa_Mutex_Lock(m_hSubscriptionListMutex);
			OpcUa_Mutex_Lock(m_hPublishMutex);
			uStatus = AckPublishRequest(pPublishMessage);
			if (uStatus != OpcUa_Good)
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_WARNING, "Ack-Notification Message failed: ustatus=0x%x\n", uStatus);
			// Now Queue the request for the Notification process
			uStatus = QueuePublishRequest(pPublishMessage);
			if ((uStatus != OpcUa_Good) /*&& (uStatus != OpcUa_BadWaitingForResponse)*/)
			{
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_WARNING, "QueuePublishRequest failed: ustatus=0x%x\n", uStatus);
				// Let properly cancel the request before deleting the CQueuePublishMessage
				OpcUa_EncodeableType*	pRequestType=pPublishMessage->GetRequestType();
				OpcUa_PublishResponse* pPublisResponse=pPublishMessage->GetInternalPublishResponse();
				OpcUa_Endpoint_EndSendResponse(hEndpoint, &hContext, uStatus, pPublisResponse, pRequestType);
				// Delete the remaing ressurces
				delete pPublishMessage;
				pPublishMessage = OpcUa_Null;
				uStatus = OpcUa_Good;
			}
			OpcUa_Mutex_Unlock(m_hPublishMutex);
			OpcUa_Mutex_Unlock(m_hSubscriptionListMutex);
		}
	}
	else
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critical error>ProcessPublishRequest failed. Out of memory");
	// Clean up the publish message queue
	RemoveAllPublishRequestDeleted();
	// 
	OpcUa_Mutex_Lock(m_hSubscriptionListMutex);
	// If needed update the Subscription diagnosticsData
	if (m_bServerDiagnosticsEnabled)
	{
		CSubscriptionServer* pSubscription = OpcUa_Null;
		for (CSubscriptionList::iterator it = m_SubscriptionList.begin(); it != m_SubscriptionList.end(); it++)
		{
			pSubscription = *it;
			if (pSubscription)
			{
				pSubscription->UpdateSubscriptionDiagnosticsDataType();
			}
		}
	}
	// Now will also wake up the NotificationDataThread
	WakeupNotificationDataThread();
	OpcUa_Mutex_Unlock(m_hSubscriptionListMutex);
	return uStatus;
}
OpcUa_StatusCode CSessionServer::AckPublishRequest(CQueuedPublishMessage* pPublishMessage)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	if (pPublishMessage)
	{
		OpcUa_PublishRequest* pPublishRequest = pPublishMessage->GetInternalPublisRequest();
		if (pPublishRequest)
		{
			if (pPublishRequest->NoOfSubscriptionAcknowledgements > 0)
			{
				// prépare une structure qui contiendra les sequences acquittées. Subscription Basic : Err-014.js	
				OpcUa_UInt32* pSequenceNumber = (OpcUa_UInt32*)OpcUa_Alloc(sizeof(OpcUa_UInt32)*(pPublishRequest->NoOfSubscriptionAcknowledgements));
				if (pSequenceNumber)
				{
					ZeroMemory(pSequenceNumber, sizeof(OpcUa_UInt32)*pPublishRequest->NoOfSubscriptionAcknowledgements);
					map<OpcUa_UInt32, OpcUa_UInt32> SubscriptionMap;// a map made of SubscriptionId, uiSequence
					for (OpcUa_Int32 ii = 0; ii < pPublishRequest->NoOfSubscriptionAcknowledgements; ii++)
					{
						OpcUa_UInt32 uiSequence = pPublishRequest->SubscriptionAcknowledgements[ii].SequenceNumber;
						CSubscriptionServer* pSubscription = OpcUa_Null;
						uStatus = FindSubscription(pPublishRequest->SubscriptionAcknowledgements[ii].SubscriptionId,
							&pSubscription);
						if (uStatus == OpcUa_Good)
						{		
							CAvailableSequenceNumbers  availableSequenceNumbers= pSubscription->AvailableSequenceNumberGet();
							if ((availableSequenceNumbers.size() > 0) || (pSubscription->GetEventNotificationListList()->size()>0) )
							{
								// check that this sequence was not in the current call Err-014.js
								map<OpcUa_UInt32, OpcUa_UInt32>::iterator it;
								for (it = SubscriptionMap.begin(); it != SubscriptionMap.end(); ++it)
								{
									if ((it->first == pPublishRequest->SubscriptionAcknowledgements[ii].SubscriptionId) && (it->second == uiSequence))
									{
										uStatus = OpcUa_BadSequenceNumberUnknown;
										break;
									}
								}
								if (uStatus == OpcUa_Good)
								{
									// Subscription DiagnosticData
									CSubscriptionDiagnosticsDataType* pSubscriptionDiagnosticsDataType = pSubscription->GetSubscriptionDiagnosticsDataType();
									if (pSubscriptionDiagnosticsDataType)
									{
										pSubscriptionDiagnosticsDataType->SetUnacknowledgedMessageCount(pSubscriptionDiagnosticsDataType->GetUnacknowledgedMessageCount() - 1);
										pSubscription->UpdateSubscriptionDiagnosticsDataType();
									}
									SubscriptionMap.insert(pair<OpcUa_UInt32, OpcUa_UInt32>(pPublishRequest->SubscriptionAcknowledgements[ii].SubscriptionId, uiSequence));
									uStatus = pSubscription->AckDataChangeNotification(uiSequence);
								}
							}
							else
								uStatus = OpcUa_BadSequenceNumberUnknown;
							pPublishMessage->AddStatusCode(uStatus);
						}
						else
							pPublishMessage->AddStatusCode(uStatus);
						// Leak test 6-2-2016
						OpcUa_SubscriptionAcknowledgement_Clear(&(pPublishRequest->SubscriptionAcknowledgements[ii]));
					}
					OpcUa_Free(pSequenceNumber);
					pSequenceNumber = OpcUa_Null;	
					// 
					SubscriptionMap.clear();
					//printf("Free :%p\n", pPublishRequest->SubscriptionAcknowledgements);
					//OpcUa_Free(pPublishRequest->SubscriptionAcknowledgements); // leak test 6-2-2016
					//pPublishRequest->NoOfSubscriptionAcknowledgements = 0;
					//pPublishRequest->SubscriptionAcknowledgements = OpcUa_Null;
					// on signal que la semaphore m_hPublishSem est disponible
					OpcUa_Semaphore_Post(m_hPublishSem, 1);
				}
				else
					uStatus = OpcUa_BadOutOfMemory;
			}
		}
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}
OpcUa_StatusCode CSessionServer::QueuePublishRequest(CQueuedPublishMessage* pPublishMessage)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	if (pPublishMessage)
	{
		OpcUa_Semaphore_Post(m_hPublishSem, 1);
		if (OpcUa_Semaphore_TimedWait(m_hPublishSem, OPC_TIMEOUT * 2) == OpcUa_GoodNonCriticalTimeout)
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "AckAndQueuePublishRequest LEAVE on TIMEOUT... this is not good. Contact Michel Condemine\n");
		//OpcUa_Mutex_Lock(m_hPublishMutex);
		// add to queue.
		// let's inform the client that he is in late
		m_PublishRequests.push_back(pPublishMessage);
		//OpcUa_Mutex_Unlock(m_hPublishMutex);
		// on signal que la semaphore m_hPublishSem est disponible
		OpcUa_Semaphore_Post(m_hPublishSem, 1);
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}

OpcUa_StatusCode CSessionServer::Reconnect(OpcUa_UInt32 uSecureChannelId)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_ReferenceParameter(uSecureChannelId);
	// First will close the session and will restart a new one
	//OpcUa_RequestHeader*    a_pRequestHeader;
	//OpcUa_ResponseHeader* a_pResponseHeader;
	// wiull use the existing securityMode
	//OpcUa_MessageSecurityMode eSecurityMode=m_pSecureChannel->GetSecurityMode();
	//OpcUa_String* sSecurityPolicy=m_pSecureChannel->GetSecurityPolicy() ;
	//RemoveAllPublishRequest();
	//SetSecureChannelId(uSecureChannelId);
	// so will start a new one
	//Create(
	//	uSecureChannelId,
	//	eSecurityMode,
	//	sSecurityPolicy,
	//	a_pRequestHeader,
	//	a_pClientDescription,
	//	a_pServerUri,
	//	a_pEndpointUrl,
	//	a_pSessionName,
	//	a_pClientNonce,
	//	a_pClientCertificate,
	//	a_nRequestedSessionTimeout,
	//	a_nMaxResponseMessageSize,
	//	a_pResponseHeader,
	//	a_pSessionId,
	//	a_pAuthenticationToken,
	//	a_pRevisedSessionTimeout,
	//	a_pServerNonce,
	//	a_pServerCertificate,
	//	a_pNoOfServerEndpoints,
	//	a_pServerEndpoints,
	//	a_pServerSignature,
	//	a_pMaxRequestMessageSize);
	return uStatus;
}
OpcUa_StatusCode CSessionServer::InitSubscriptionDiagnosticsDataType(CSubscriptionServer* pSubscription)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	CUAInformationModel* pInformationModel = CServerApplication::m_pTheAddressSpace;
	if (pInformationModel->IsEnabledServerDiagnosticsDefaultValue())
	{
		if (pSubscription)
		{
			// Fabriquer un OpcUa_SubscriptionDiagnosticsDataType
			CSubscriptionDiagnosticsDataType* pSubscriptionDiagnosticsDataType = pSubscription->GetSubscriptionDiagnosticsDataType();
			if (pSubscriptionDiagnosticsDataType)
			{
				OpcUa_SubscriptionDiagnosticsDataType* pInternalSubscriptionDiagnosticsDataType = pSubscriptionDiagnosticsDataType->GetInternalSubscriptionDiagnosticsDataType();
				if (!pInternalSubscriptionDiagnosticsDataType)
				{
					pInternalSubscriptionDiagnosticsDataType = (OpcUa_SubscriptionDiagnosticsDataType*)OpcUa_Alloc(sizeof(OpcUa_SubscriptionDiagnosticsDataType));
					OpcUa_SubscriptionDiagnosticsDataType_Initialize(pInternalSubscriptionDiagnosticsDataType);
					pSubscriptionDiagnosticsDataType->SetInternalSubscriptionDiagnosticsDataType(pInternalSubscriptionDiagnosticsDataType);
				}
				pSubscriptionDiagnosticsDataType->SetCurrentKeepAliveCount(0);
				pSubscriptionDiagnosticsDataType->SetCurrentLifetimeCount(0);
				pSubscriptionDiagnosticsDataType->SetDataChangeNotificationsCount(0);
				pSubscriptionDiagnosticsDataType->SetDisableCount(0);
				pSubscriptionDiagnosticsDataType->SetDisabledMonitoredItemCount(0);
				pSubscriptionDiagnosticsDataType->SetDiscardedMessageCount(0);
				pSubscriptionDiagnosticsDataType->SetEnableCount(0);
				pSubscriptionDiagnosticsDataType->SetEventNotificationsCount(0);
				pSubscriptionDiagnosticsDataType->SetEventQueueOverFlowCount(0);
				pSubscriptionDiagnosticsDataType->SetLatePublishRequestCount(0);
				pSubscriptionDiagnosticsDataType->SetMaxKeepAliveCount(pSubscription->GetMaxKeepAliveCount());
				pSubscriptionDiagnosticsDataType->SetMaxLifetimeCount(0);
				pSubscriptionDiagnosticsDataType->SetMaxNotificationsPerPublish(pSubscription->GetMaxNotificationsPerPublish());
				pSubscriptionDiagnosticsDataType->SetModifyCount(0);
				pSubscriptionDiagnosticsDataType->SetMonitoredItemCount(0);
				pSubscriptionDiagnosticsDataType->SetMonitoringQueueOverflowCount(0);
				pSubscriptionDiagnosticsDataType->SetNextSequenceNumber(0);
				pSubscriptionDiagnosticsDataType->SetNotificationsCount(0);
				pSubscriptionDiagnosticsDataType->SetCurrentKeepAliveCount(pSubscription->GetPriority());
				pSubscriptionDiagnosticsDataType->SetPublishingEnabled(pSubscription->GetPublishingEnabled());
				pSubscriptionDiagnosticsDataType->SetPublishingInterval(pSubscription->GetPublishingInterval());
				pSubscriptionDiagnosticsDataType->SetPublishRequestCount(GetPublishRequestsSize());
				pSubscriptionDiagnosticsDataType->SetRepublishMessageCount(0);
				pSubscriptionDiagnosticsDataType->SetRepublishRequestCount(0);
				pSubscriptionDiagnosticsDataType->SetSessionId(pSubscription->GetSession()->GetSessionId());
				pSubscriptionDiagnosticsDataType->SetSubscriptionId(pSubscription->GetSubscriptionId());
				pSubscriptionDiagnosticsDataType->SetTransferredToAltClientCount(0);
				pSubscriptionDiagnosticsDataType->SetTransferredToSameClientCount(0);
				pSubscriptionDiagnosticsDataType->SetTransferRequestCount(0);
				pSubscriptionDiagnosticsDataType->SetUnacknowledgedMessageCount(0);
				// L'ajouter dans le Node Approprié 
				if (pSubscriptionDiagnosticsDataType)
				{
					uStatus = pInformationModel->AddInSubscriptionDiagnosticsArray(pSubscriptionDiagnosticsDataType);
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
	return uStatus;
}

// Function name   : CSessionServer::InitSessionDiagnosticsDataType
// Description     : Initialise la variable de classe m_pSessionDiagnostics (CSessionDiagnosticsDataType)
// Return type     : OpcUa_StatusCode 
// Argument        : OpcUa_ApplicationDescription* pClientDescription

/// <summary>
/// Initializes the type of the session diagnostics data.
/// </summary>
/// <returns></returns>
OpcUa_StatusCode CSessionServer::InitSessionDiagnosticsDataType()
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	{
		CUAInformationModel* pInformationModel=CServerApplication::m_pTheAddressSpace;
		if (m_pSessionDiagnostics)
		{
			//
			// OpcUa_SessionDiagnosticsDataType* pSessionDiagnosticsDataType=(OpcUa_SessionDiagnosticsDataType*)OpcUa_Alloc(sizeof(OpcUa_SessionDiagnosticsDataType));
			OpcUa_SessionDiagnosticsDataType* pSessionDiagnosticsDataType = m_pSessionDiagnostics->GetInternalPtr();
			if (pSessionDiagnosticsDataType)
			{
				// SessionId
				pSessionDiagnosticsDataType->SessionId = m_tSessionId;
				// SessionName
				if (OpcUa_String_StrLen(&m_SessionName))
					OpcUa_String_StrnCpy(&(pSessionDiagnosticsDataType->SessionName), &m_SessionName, OpcUa_String_StrLen(&m_SessionName));

				CEndpointDescription* pEndPointDescription = GetEndpointDescription();
				// EndpointUrl
				OpcUa_String* pEndpointUrl = pEndPointDescription->GetEndpointUrl();
				m_pSessionDiagnostics->SetEndpointUrl(pEndpointUrl);
				// ServerUri : Handled with the ApplicationDescription
				// 
				// LocaleIds
				// Le serveur supportera anglais et francais ==> 2 LocalIds
				pSessionDiagnosticsDataType->NoOfLocaleIds = 2;
				pSessionDiagnosticsDataType->LocaleIds = (OpcUa_String*)OpcUa_Alloc(2 * sizeof(OpcUa_String));
				if (pSessionDiagnosticsDataType->LocaleIds)
				{
					// Le premier
					OpcUa_String_Initialize(&(pSessionDiagnosticsDataType->LocaleIds[0]));
					OpcUa_String_AttachCopy(
						&(pSessionDiagnosticsDataType->LocaleIds[0]), (OpcUa_CharA*)"en-EN");
					// le second
					OpcUa_String_Initialize(&(pSessionDiagnosticsDataType->LocaleIds[1]));
					OpcUa_String_AttachCopy(
						&(pSessionDiagnosticsDataType->LocaleIds[1]), (OpcUa_CharA*)"fr-FR");
					//
					pSessionDiagnosticsDataType->ActualSessionTimeout = 0;
					//
					pSessionDiagnosticsDataType->MaxResponseMessageSize = 0;

					// Les autres attributs de la structure seront mise à jour durant le fonctionnement du serveur 
					// et pendant les evenements particulier de son cylce de vie. CreateSubscription, MonitorItem, publish, etc.
					// Transfert dans les nodeid i=3707 de type ns=0;i=865
					uStatus = pInformationModel->AddInSessionDiagnosticsArray(m_pSessionDiagnostics);
				}
				else
					uStatus = OpcUa_BadOutOfMemory;

				if (uStatus != OpcUa_Good)
					OpcUa_SessionDiagnosticsDataType_Clear(pSessionDiagnosticsDataType);
			}
			else
				uStatus = OpcUa_BadOutOfMemory;
		}
		else
			OpcUa_Trace(OPCUA_TRACE_SERVER_ERROR, "Internal error>CSessionDiagnosticsDataType in not initialize\n");
	}
	return uStatus;
}
//typedef struct _OpcUa_SessionSecurityDiagnosticsDataType
//{
//    OpcUa_NodeId              SessionId;
//    OpcUa_String              ClientUserIdOfSession;
//    OpcUa_Int32               NoOfClientUserIdHistory;
//    OpcUa_String*             ClientUserIdHistory;
//    OpcUa_String              AuthenticationMechanism;
//    OpcUa_String              Encoding;
//    OpcUa_String              TransportProtocol;
//    OpcUa_MessageSecurityMode SecurityMode;
//    OpcUa_String              SecurityPolicyUri;
//    OpcUa_ByteString          ClientCertificate;
//}
//OpcUa_SessionSecurityDiagnosticsDataType;
OpcUa_StatusCode CSessionServer::InitSessionSecurityDiagnosticsDataType()
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	CEndpointDescription* pEndpointDescription = GetEndpointDescription();
	if (pEndpointDescription)
	{
		OpcUa_MessageSecurityMode securityMode = pEndpointDescription->GetSecurityMode();
		CSecureChannel*	pSecureChannel = GetSecureChannel();
		if (pSecureChannel)
		{
			OpcUa_ByteString* pClientCertificate = pSecureChannel->GetClientCertificate();
			if (!m_pSessionSecurityDiagnostics)
				m_pSessionSecurityDiagnostics=new CSessionSecurityDiagnosticsDataType();// 
			if (m_pSessionSecurityDiagnostics)
			{
				//OpcUa_SessionSecurityDiagnosticsDataType_Initialize(m_pSessionSecurityDiagnostics);
				// Il faut maitenant remplir le pSessionSecurityDiagnostics
				// SessionId
				m_pSessionSecurityDiagnostics->SetSessionId(&m_tSessionId);
				//ClientUserIdOfSession
				//OpcUa_String_Initialize(&(m_pSessionSecurityDiagnostics->ClientUserIdOfSession));
				//m_pSessionSecurityDiagnostics->ClientUserIdOfSession;
				// NoOfClientUserIdHistory
				// See OPC UA 1.02 Part 5 Table 146 for more detail

				// ClientUserIdHistory
				// See OPC UA 1.02 Part 5 Table 146 for more detail
		
				// AuthenticationMechanism
				//OpcUa_String_Initialize(&(m_pSessionSecurityDiagnostics->AuthenticationMechanism));
				//m_pSessionSecurityDiagnostics->AuthenticationMechanism; // TODO

				// 
				// Extract from EndpointUrl
				//OpcUa_String* pszEndpointUrl = pEndpointDescription->GetEndpointUrl();
				// TransportProtocol and Encoding
				OpcUa_String szTcp;
				OpcUa_String_Initialize(&szTcp);
				OpcUa_String_AttachCopy(&szTcp, "TCP");
				OpcUa_String szHttp;
				OpcUa_String_Initialize(&szHttp);
				OpcUa_String_AttachCopy(&szHttp, "HTTP");
				OpcUa_String szHttps;
				OpcUa_String_Initialize(&szHttps);
				OpcUa_String_AttachCopy(&szHttps, "HTTPS");
				OpcUa_String szBinary;
				OpcUa_String_Initialize(&szBinary);
				OpcUa_String_AttachCopy(&szBinary, "Binary");
				OpcUa_String szXml;
				OpcUa_String_Initialize(&szXml);
				OpcUa_String_AttachCopy(&szXml, "XML");
				// Get the Transport From Endpoint
				OpcUa_String szTransportProfileUri=pEndpointDescription->GetTransportProfileUri();
				OpcUa_String szTcpBinary;
				OpcUa_String_Initialize(&szTcpBinary);
				OpcUa_String_AttachCopy(&szTcpBinary, OpcUa_TransportProfile_UaTcp);
				if (OpcUa_String_Compare(&szTransportProfileUri, &szTcpBinary) == 0)
				{
					m_pSessionSecurityDiagnostics->SetTransportProtocol(&szTcp);
					m_pSessionSecurityDiagnostics->SetEncoding(&szBinary);
				}
				OpcUa_String_Clear(&szTcp);
				OpcUa_String_Clear(&szHttp);
				OpcUa_String_Clear(&szXml);
				OpcUa_String_Clear(&szTcpBinary);
				/*
				OpcUa_String szHttpBinary;
				OpcUa_String_Initialize(&szHttpBinary);
				OpcUa_String_AttachCopy(&szHttpBinary, OpcUa_TransportProfile_SoapHttpBinary);
				if (OpcUa_String_Compare(&szTransportProfileUri, &szHttpBinary) == 0)
				{
					m_pSessionSecurityDiagnostics->SetTransportProtocol(&szHttp);
					m_pSessionSecurityDiagnostics->SetEncoding(&szBinary);
				}
				*/
				/*
				OpcUa_String szHttpXml;
				OpcUa_String_Initialize(&szHttpXml);
				OpcUa_String_AttachCopy(&szHttpXml, OpcUa_TransportProfile_SoapHttpsUaXml);
				if (OpcUa_String_Compare(&szTransportProfileUri, &szHttpXml) == 0)
				{
					m_pSessionSecurityDiagnostics->SetTransportProtocol(&szHttp);
					m_pSessionSecurityDiagnostics->SetEncoding(&szXml);
				}
				*/
				OpcUa_String szHttpsBinary;
				OpcUa_String_Initialize(&szHttpsBinary);
				OpcUa_String_AttachCopy(&szHttpsBinary, OpcUa_TransportProfile_HttpsBinary);
				if (OpcUa_String_Compare(&szTransportProfileUri, &szHttpsBinary) == 0)
				{
					m_pSessionSecurityDiagnostics->SetTransportProtocol(&szHttps);
					m_pSessionSecurityDiagnostics->SetEncoding(&szBinary);
				}
				OpcUa_String_Clear(&szHttps);
				OpcUa_String_Clear(&szBinary);
				OpcUa_String_Clear(&szHttpsBinary);
				// SecurityMode
				m_pSessionSecurityDiagnostics->SetSecurityMode((OpcUa_MessageSecurityMode)securityMode);
				//m_pSessionSecurityDiagnostics->SecurityMode=(OpcUa_MessageSecurityMode)securityMode; // ATTENTION il y a un problème dans la stack sur cette correspondance
				// SecurityPolicyUri
				OpcUa_String* pszSecurityPolicy = pSecureChannel->GetSecurityPolicy();
				m_pSessionSecurityDiagnostics->SetSecurityPolicyUri(pszSecurityPolicy);

				// ClientCertificate
				if (pClientCertificate->Length>0)
					m_pSessionSecurityDiagnostics->SetClientCertificate(pClientCertificate);

				CUAInformationModel* pInformationModel=CServerApplication::m_pTheAddressSpace;
				// Transfert dans les nodeid de type ns=0;i=865
				uStatus=pInformationModel->AddInSessionSecurityDiagnosticsArray(m_pSessionSecurityDiagnostics);
				// Il faut aussi initialiser le ns=0;i=2031
			}
			else
				uStatus=OpcUa_BadInvalidArgument;
	
		}
		else
			uStatus = OpcUa_BadInvalidArgument;
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}

//============================================================================
// CServerApplication::QueueHistoryReadMessage
// Place la demande de lecture synchrone dans la Queue
//============================================================================
OpcUa_StatusCode CSessionServer::QueueHistoryReadMessage(CQueuedHistoryReadMessage* pRequest)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_Mutex_Lock(m_hHistoryReadMutex);
	// add to queue.
	m_pHistoryReadMessages->push_back(pRequest);
	OpcUa_Mutex_Unlock(m_hHistoryReadMutex);
	return uStatus;
}

//============================================================================
// CServerApplication::QueueReadRequest
// Place la demande de lecture synchrone dans la Queue
//============================================================================
OpcUa_StatusCode CSessionServer::QueueReadMessage(CQueuedReadMessage* pRequest)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_Mutex_Lock(m_hReadMutex);
	// add to queue.
	m_pReadMessages->push_back(pRequest);
	OpcUa_Mutex_Unlock(m_hReadMutex);
	return uStatus;
}

// Function name   : CServerApplication::ProcessQueuedReadRequest
// Description     : Appelée par CServerApplication::ProcessQueuedµReadRequests, 
//					 permet de transmettre la reponse à la fonction callback du client.
// Return type     : void 
// Argument        : CQueuedReadMessage* pRequest
// Argument        : bool abort

void CSessionServer::ProcessQueuedReadRequest(CQueuedReadMessage* pReadMessage, OpcUa_Boolean bAbort)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;

	try
	{
		// check for abort the request.
		//OpcUa_Handle hContext=pReadMessage->GetContext();
		if (bAbort)
		{
			uStatus =pReadMessage->CancelSendResponse();
		}
		else
		{
			// Create a context to use for sending a response.
			uStatus =pReadMessage->BeginSendResponse();
			if (uStatus!=OpcUa_Good)
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "CSessionServer::ProcessQueuedReadRequest>Stack could not initialize response.\n");
			else
			{
				// Réalise la lecture dans la cache du serveur
				pReadMessage->FillResponseHeader();
				uStatus=ProcessReadRequest(pReadMessage);

				// Send the response to the client.
				uStatus =pReadMessage->EndSendResponse();
				if (OpcUa_IsBad(uStatus))
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "CSessionServer::ProcessQueuedReadRequest>Could not send response to client. Status 0x%08X!\r\n", uStatus);
				// Mark it for deletion
				pReadMessage->SetDeleted(OpcUa_True);

			}
		}
		pReadMessage->EncodeableObjectDelete();		
	}
	catch (...)
	{
		pReadMessage->EncodeableObjectDelete();	
	}
}
//============================================================================
// CSessionServer::ProcessHistoryReadRequest
//============================================================================
void CSessionServer::ProcessQueuedHistoryReadMessage(CQueuedHistoryReadMessage* pHistoryReadMessage, OpcUa_Boolean bAbort)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;	
	if (bAbort)
	{
		uStatus = pHistoryReadMessage->CancelSendResponse();
	}
	else
	{
		// Create a context to use for sending a response.
		uStatus = pHistoryReadMessage->BeginSendResponse();
		if (uStatus != OpcUa_Good)
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "CSessionServer::pHistoryReadMessage>Stack could not initialize response.\n");
		else
		{
			// Réalise la lecture dans la cache du serveur
			pHistoryReadMessage->FillResponseHeader();
			uStatus = ProcessHistoryReadMessage(pHistoryReadMessage);

			// Send the response to the client.
			uStatus = pHistoryReadMessage->EndSendResponse();
			if (OpcUa_IsBad(uStatus))
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "CSessionServer::pHistoryReadMessage>Could not send response to client. Status 0x%08X!\r\n", uStatus);
			// Mark it for deletion
			pHistoryReadMessage->SetDeleted(OpcUa_True);

		}
	}
	pHistoryReadMessage->EncodeableObjectDelete();
}
//============================================================================
// CSessionServer::ProcessHistoryReadRequest
// Result of the request goes in 
	/*typedef struct _OpcUa_HistoryReadResult
	{
		OpcUa_StatusCode      StatusCode;
		OpcUa_ByteString      ContinuationPoint;
		OpcUa_ExtensionObject HistoryData;
	}
	OpcUa_HistoryReadResult;*/
//============================================================================
OpcUa_StatusCode CSessionServer::ProcessHistoryReadMessage(CQueuedHistoryReadMessage* pHistoryReadMessage)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	//CUAInformationModel* pInformationModel = CServerApplication::m_pTheAddressSpace;
	if (pHistoryReadMessage)
	{
		// Si besoin demarrage du moteur d'archivage
		CHaEngine* pHaEngine = m_pApplication->GetHaEngine();

		// 
		OpcUa_HistoryReadRequest* pReadRequest = pHistoryReadMessage->GetInternalReadRequest();
		//pReadRequest->HistoryReadDetails;
		//pReadRequest->NoOfNodesToRead;
		//pReadRequest->NodesToRead;
		//pReadRequest->ReleaseContinuationPoints;
		//pReadRequest->RequestHeader;
		//pReadRequest->TimestampsToReturn;
		OpcUa_HistoryReadResponse* pReadResponse = pHistoryReadMessage->GetInternalReadResponse();
		pReadResponse->Results = (OpcUa_HistoryReadResult*)OpcUa_Alloc(sizeof(OpcUa_HistoryReadResult)*(pReadRequest->NoOfNodesToRead));
		pReadResponse->ResponseHeader.RequestHandle = pReadRequest->RequestHeader.RequestHandle;
		pReadResponse->NoOfResults = pReadRequest->NoOfNodesToRead;
		for (OpcUa_Int32 i = 0; i < pReadRequest->NoOfNodesToRead; i++)
		{
			OpcUa_HistoryReadResult_Initialize(&pReadResponse->Results[i]);

			if (!IsActive())
			{
				pReadResponse->Results[i].StatusCode = OpcUa_BadSessionNotActivated;
				pReadResponse->ResponseHeader.ServiceResult = OpcUa_BadSessionNotActivated;
				pReadResponse->ResponseHeader.Timestamp = OpcUa_DateTime_UtcNow();
				uStatus = OpcUa_BadSessionNotActivated;
				m_bRunSessionTimeoutThread = OpcUa_False; 
			}
			else
			{
				if (pHaEngine)
				{
					OpcUa_DateTime dtFrom, dtTo;
					OpcUa_Vfi_Handle hVfi = pHaEngine->GetVfiHandle();
					OpcUa_UInt32	uiNoOfValueRead = 1;
					OpcUa_DataValue* pValue = OpcUa_Null;
					if (hVfi)
					{
						// ReadRawModifiedDetails
						if ((pReadRequest->HistoryReadDetails.TypeId.NodeId.IdentifierType == OpcUa_IdentifierType_Numeric)
							&& (pReadRequest->HistoryReadDetails.TypeId.NodeId.Identifier.Numeric == OpcUaId_ReadRawModifiedDetails_Encoding_DefaultBinary))// 649
						{
							OpcUa_ReadRawModifiedDetails* pReadRawsModifedDetails = (OpcUa_ReadRawModifiedDetails*)pReadRequest->HistoryReadDetails.Body.EncodeableObject.Object;
							dtFrom = pReadRawsModifedDetails->StartTime;
							dtTo = pReadRawsModifedDetails->EndTime;
							// The Vfi Call will always retrun bound. The server will remove it... Let think about it
							OpcUa_Mutex_Lock(pHaEngine->m_StorageVfiCallMutex);
							uStatus = pHaEngine->VfiHistoryRead(hVfi, pReadRequest->NodesToRead[i].NodeId, dtFrom, dtTo, &uiNoOfValueRead, &pValue);
							OpcUa_Mutex_Unlock(pHaEngine->m_StorageVfiCallMutex);
							if (uStatus == OpcUa_Good)
							{
								if (uiNoOfValueRead > 0)
								{
									pReadResponse->NoOfResults = pReadRequest->NoOfNodesToRead;
									OpcUa_HistoryData* pHistoryData = (OpcUa_HistoryData*)OpcUa_Alloc(sizeof(OpcUa_HistoryData));
									OpcUa_HistoryData_Initialize(pHistoryData);
									pHistoryData->NoOfDataValues = uiNoOfValueRead;
									pHistoryData->DataValues = (OpcUa_DataValue*)OpcUa_Alloc(uiNoOfValueRead*sizeof(OpcUa_DataValue));
									for (OpcUa_UInt32 ii = 0; ii < uiNoOfValueRead; ii++)
									{
										OpcUa_DataValue_Initialize(&(pHistoryData->DataValues[ii]));
										pHistoryData->DataValues[ii].StatusCode = pValue[ii].StatusCode;
										pHistoryData->DataValues[ii].ServerPicoseconds = pValue[ii].ServerPicoseconds;
										pHistoryData->DataValues[ii].ServerTimestamp = pValue[ii].ServerTimestamp;
										pHistoryData->DataValues[ii].SourcePicoseconds = pValue[ii].SourcePicoseconds;
										pHistoryData->DataValues[ii].SourceTimestamp = pValue[ii].SourceTimestamp;
										//
										OpcUa_Variant_CopyTo(&(pValue[ii].Value), &(pHistoryData->DataValues[ii].Value));
									}
									OpcUa_EncodeableType* pEncodeableType;
									OpcUa_UInt32 iTypeId = OpcUaId_HistoryData; // OpcUaId_HistoryData_Encoding_DefaultBinary;
									if (m_pApplication->LookupEncodeableType(iTypeId, &pEncodeableType) == OpcUa_Good)
									{
										pReadResponse->Results[i].HistoryData.TypeId.NodeId.Identifier.Numeric = pEncodeableType->BinaryEncodingTypeId;
										pReadResponse->Results[i].HistoryData.TypeId.NodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
										pReadResponse->Results[i].HistoryData.TypeId.NodeId.NamespaceIndex = 0;
										pReadResponse->Results[i].HistoryData.BodySize = 0;
										pReadResponse->Results[i].HistoryData.Encoding = OpcUa_ExtensionObjectEncoding_EncodeableObject;
										pReadResponse->Results[i].HistoryData.Body.EncodeableObject.Type = pEncodeableType;// Utils::Copy(pEncodeableType);
										pReadResponse->Results[i].HistoryData.Body.EncodeableObject.Object = pHistoryData;
										pReadResponse->Results[i].ContinuationPoint.Data = OpcUa_Null;
										pReadResponse->Results[i].ContinuationPoint.Length = 0;
									}
									else
										OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "ProcessHistoryReadMessage>LookupEncodeableType failed for iTypeId=%u\n", iTypeId);
								}
							}

						}
					}
					else
						pReadResponse->Results[i].StatusCode = OpcUa_BadNotReadable;
				}
				else
					pReadResponse->Results[i].StatusCode = OpcUa_BadNotReadable;
			}

		}
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	
	return uStatus;
}
//============================================================================
// CSessionServer::ProcessReadRequest
//============================================================================
OpcUa_StatusCode CSessionServer::ProcessReadRequest(CQueuedReadMessage* pRequest)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	CUAInformationModel* pInformationModel=CServerApplication::m_pTheAddressSpace;
	if (pInformationModel)
	{
		OpcUa_ReadResponse* pResponse=pRequest->GetInternalReadResponse();
		// Fill in results list.
		pResponse->NoOfResults = pRequest->GetNoOfNodesToRead();
		// Il faut allouer
		OpcUa_Int32 iNbN2R=pRequest->GetNoOfNodesToRead();
		pResponse->Results = (OpcUa_DataValue*)OpcUa_Alloc(sizeof(OpcUa_DataValue)*iNbN2R);
	
		if (iNbN2R==0)
			pResponse->ResponseHeader.ServiceResult=OpcUa_BadNothingToDo;
		for (OpcUa_Int32 ii = 0; ii < iNbN2R; ii++)
		{
			OpcUa_DataValue_Initialize(&(pResponse->Results[ii]));
			OpcUa_Double dblMaxAge=pRequest->GetMaxAge();
			if (dblMaxAge<0)
			{
				pResponse->Results[ii].StatusCode=OpcUa_BadMaxAgeInvalid;
				pResponse->ResponseHeader.ServiceResult=OpcUa_BadMaxAgeInvalid;
			}
			else
			{
				OpcUa_TimestampsToReturn aTimeStampToReturn= pRequest->GetTimestampsToReturn();
				if ((aTimeStampToReturn<=3) && (aTimeStampToReturn>=0) )
				{
					//
					OpcUa_ReadValueId* nodeToRead = pRequest->GetReadValueId(ii);
					//OpcUa_String xmlDataEncoding;
					//OpcUa_String_Initialize(&xmlDataEncoding);
					//OpcUa_String_AttachCopy(&xmlDataEncoding,"Xml");
					OpcUa_CharA* szCurrentEncoding = OpcUa_String_GetRawString(&nodeToRead->DataEncoding.Name);
					pResponse->Results[ii].StatusCode = OpcUa_Good;
					if (szCurrentEncoding)
					{
						if (OpcUa_StrCmpA(szCurrentEncoding, "Xml") == 0)
							//if (OpcUa_String_StrnCmp(&(nodeToRead->DataEncoding.Name),&xmlDataEncoding,OpcUa_String_StrLen(&xmlDataEncoding),OpcUa_False)==0)
							pResponse->Results[ii].StatusCode = OpcUa_BadDataEncodingInvalid;
					}
					if (pResponse->Results[ii].StatusCode==OpcUa_Good)
					{
						CUAVariable* pUAVariable=OpcUa_Null;
						CUAObject* pUAObject = OpcUa_Null;
						CUAObjectType* pUAObjectType = OpcUa_Null;
						CUABase* pUABase = OpcUa_Null;
						BOOL bOk=FALSE; // flag permettant d'indiquer que l'on peu réaliser les traitement commun
						//if ( (nodeToRead->NodeId.NamespaceIndex==2) && (nodeToRead->NodeId.Identifier.Numeric==16) )
						//	printf("Debug purpose\n");

						uStatus = pInformationModel->GetNodeIdFromDictionnary(nodeToRead->NodeId, &pUABase);
						if (uStatus != OpcUa_Good)
						{
							pResponse->Results[ii].StatusCode = OpcUa_BadNodeIdUnknown;
							continue;
						}
						else
						{
							// On commence par determiner les Status code en fonction de la demande du client
							// Il s'agit d'une sorte de pre-statusCode
							OpcUa_StatusCode uStatusFinal=OpcUa_BadOutOfRange;  // OpcUa_BadIndexRangeInvalid
							OpcUa_StatusCode uStatusPreChecked=PreCheckedStatusCode(nodeToRead,pUABase);

							// On commence par le traitement de la partie spécifique a  chaque classe
							switch (pUABase->GetNodeClass())
							{
							case OpcUa_NodeClass_DataType:
							{
								CUADataType* pUADataType=(CUADataType*)pUABase;
								// par défaut on considère que le client nous demande un attribut invalide :(
								pResponse->Results[ii].StatusCode=OpcUa_BadAttributeIdInvalid;
								if (nodeToRead->AttributeId == OpcUa_Attributes_IsAbstract)
								{
									pResponse->Results[ii].Value.ArrayType=0;
									pResponse->Results[ii].Value.Datatype=OpcUaType_Boolean;
									pResponse->Results[ii].Value.Value.Boolean=pUADataType->IsAbstract();
									if (uStatusPreChecked == OpcUa_BadOutOfRange)
										pResponse->Results[ii].StatusCode=uStatusPreChecked;	
									else
										pResponse->Results[ii].StatusCode=OpcUa_Good;
								}	
								// Compliant timestamp handling... 
								ZeroMemory(&(pResponse->Results[ii].SourceTimestamp),sizeof(OpcUa_DateTime));
								if (pUAVariable)
								{
									if (pRequest->IsServerTimestampRequested())
										pResponse->Results[ii].ServerTimestamp = pUAVariable->GetValue()->GetServerTimestamp();
									else
										ZeroMemory(&(pResponse->Results[ii].ServerTimestamp),sizeof(OpcUa_DateTime));
								}
								else
									pResponse->Results[ii].ServerTimestamp = OpcUa_DateTime_UtcNow();
								bOk=TRUE;
							}
							break;
							case OpcUa_NodeClass_ReferenceType:
							{
								CUAReferenceType* pUAReferenceType=(CUAReferenceType*)pUABase;
								// par défaut on considère que le client nous demande un attribut invalide :(
								pResponse->Results[ii].StatusCode=OpcUa_BadAttributeIdInvalid;
								// Compliant timestamp handling... 
								ZeroMemory(&(pResponse->Results[ii].SourceTimestamp),sizeof(OpcUa_DateTime));
								if (pUAVariable)
								{
									if (pRequest->IsServerTimestampRequested())
										pResponse->Results[ii].ServerTimestamp = pUAVariable->GetValue()->GetServerTimestamp();
									else
										ZeroMemory(&(pResponse->Results[ii].ServerTimestamp),sizeof(OpcUa_DateTime));
								}
								else
									pResponse->Results[ii].ServerTimestamp = OpcUa_DateTime_UtcNow();
								if (nodeToRead->AttributeId == OpcUa_Attributes_IsAbstract)
								{
									pResponse->Results[ii].Value.ArrayType=0;
									pResponse->Results[ii].Value.Datatype=OpcUaType_Boolean;
									pResponse->Results[ii].Value.Value.Boolean=pUAReferenceType->IsAbstract();
									if (uStatusPreChecked == OpcUa_BadOutOfRange)
										pResponse->Results[ii].StatusCode=uStatusPreChecked;	
									else
										pResponse->Results[ii].StatusCode=OpcUa_Good;	
								}
								if (nodeToRead->AttributeId == OpcUa_Attributes_Symmetric)
								{
									pResponse->Results[ii].Value.ArrayType=0;
									pResponse->Results[ii].Value.Datatype=OpcUaType_Boolean;
									pResponse->Results[ii].Value.Value.Boolean=pUAReferenceType->IsSymetric();
									if (uStatusPreChecked == OpcUa_BadOutOfRange)
										pResponse->Results[ii].StatusCode=uStatusPreChecked;	
									else
										pResponse->Results[ii].StatusCode=OpcUa_Good;	
								}
								if (nodeToRead->AttributeId == OpcUa_Attributes_InverseName)
								{
									pResponse->Results[ii].Value.Datatype = OpcUaType_LocalizedText;
									pResponse->Results[ii].Value.ArrayType=0;
									OpcUa_LocalizedText aLocalizedText= pUABase->GetDisplayName();
									pResponse->Results[ii].Value.Value.LocalizedText=Utils::Copy(&aLocalizedText);

									if (uStatusPreChecked == OpcUa_BadOutOfRange)
										pResponse->Results[ii].StatusCode=uStatusPreChecked;	
									else
										pResponse->Results[ii].StatusCode=OpcUa_Good;	
								}
								bOk=TRUE;
							}
							break;
							case OpcUa_NodeClass_Method:
							{
								CUAMethod* pUAMethod=(CUAMethod*)pUABase;
								// par défaut on considère que le client nous demande un attribut invalide :(
								pResponse->Results[ii].StatusCode=OpcUa_BadAttributeIdInvalid;
								// Compliant timestamp handling... 
								ZeroMemory(&(pResponse->Results[ii].SourceTimestamp),sizeof(OpcUa_DateTime));
								if (pUAVariable)
								{
									if (pRequest->IsServerTimestampRequested())
										pResponse->Results[ii].ServerTimestamp = pUAVariable->GetValue()->GetServerTimestamp();
									else
										ZeroMemory(&(pResponse->Results[ii].ServerTimestamp),sizeof(OpcUa_DateTime));
								}
								else
									pResponse->Results[ii].ServerTimestamp = OpcUa_DateTime_UtcNow();
								if (nodeToRead->AttributeId == OpcUa_Attributes_UserExecutable)
								{
									pResponse->Results[ii].Value.ArrayType=0;
									pResponse->Results[ii].Value.Datatype=OpcUaType_Boolean;
									pResponse->Results[ii].Value.Value.Boolean=pUAMethod->IsUserExecutable();
									if (uStatusPreChecked == OpcUa_BadOutOfRange)
										pResponse->Results[ii].StatusCode=uStatusPreChecked;	
									else
										pResponse->Results[ii].StatusCode=OpcUa_Good;			
								}
								if (nodeToRead->AttributeId == OpcUa_Attributes_Executable)
								{
									pResponse->Results[ii].Value.ArrayType=0;
									pResponse->Results[ii].Value.Datatype=OpcUaType_Boolean;
									pResponse->Results[ii].Value.Value.Boolean=pUAMethod->IsExecutable();
									if (uStatusPreChecked==OpcUa_BadOutOfRange)
										pResponse->Results[ii].StatusCode=uStatusPreChecked;	
									else
										pResponse->Results[ii].StatusCode=OpcUa_Good;		
								}
								bOk=TRUE;
							}
							break;
							case OpcUa_NodeClass_ObjectType:
							{
								pUAObjectType=(CUAObjectType*)pUABase;
								// par défaut on considère que le client nous demande un attribut invalide :(
								pResponse->Results[ii].StatusCode=OpcUa_BadAttributeIdInvalid;
								// Compliant timestamp handling... 
								ZeroMemory(&(pResponse->Results[ii].SourceTimestamp),sizeof(OpcUa_DateTime));
								if (pUAVariable)
								{
									if (pRequest->IsServerTimestampRequested())
										pResponse->Results[ii].ServerTimestamp = pUAVariable->GetValue()->GetServerTimestamp();
									else
										ZeroMemory(&(pResponse->Results[ii].ServerTimestamp),sizeof(OpcUa_DateTime));
								}
								else
									pResponse->Results[ii].ServerTimestamp = OpcUa_DateTime_UtcNow();
								if (nodeToRead->AttributeId == OpcUa_Attributes_IsAbstract)
								{
									pResponse->Results[ii].Value.ArrayType=0;
									pResponse->Results[ii].Value.Datatype=OpcUaType_Boolean;
									pResponse->Results[ii].Value.Value.Boolean=pUAObjectType->IsAbstract();
									if (uStatusPreChecked==OpcUa_BadOutOfRange)
										pResponse->Results[ii].StatusCode=uStatusPreChecked;	
									else
										pResponse->Results[ii].StatusCode=OpcUa_Good;
								}		
								bOk=TRUE;
							}
							break;
							case OpcUa_NodeClass_Object:
							{
								// par défaut on considère que le client nous demande un attribut invalide :(
								pResponse->Results[ii].StatusCode=OpcUa_BadAttributeIdInvalid;
								// Compliant timestamp handling... 
								ZeroMemory(&(pResponse->Results[ii].SourceTimestamp),sizeof(OpcUa_DateTime));
								if (pUAVariable)
								{
									if (pRequest->IsServerTimestampRequested())
										pResponse->Results[ii].ServerTimestamp = pUAVariable->GetValue()->GetServerTimestamp();
									else
										ZeroMemory(&(pResponse->Results[ii].ServerTimestamp),sizeof(OpcUa_DateTime));
								}
								else
									pResponse->Results[ii].ServerTimestamp = OpcUa_DateTime_UtcNow();
								pUAObject=(CUAObject*)pUABase;				
								if (nodeToRead->AttributeId == OpcUa_Attributes_EventNotifier)
								{
									pResponse->Results[ii].Value.ArrayType=0;
									pResponse->Results[ii].Value.Datatype=OpcUaType_Byte;
									pResponse->Results[ii].Value.Value.Byte=pUAObject->GetEventNotifier();
									if (uStatusPreChecked==OpcUa_BadOutOfRange)
										pResponse->Results[ii].StatusCode=uStatusPreChecked;	
									else
										pResponse->Results[ii].StatusCode=OpcUa_Good;
								}
								bOk=TRUE;			
							}
							break;
							case OpcUa_NodeClass_VariableType:
							{
								CUAVariableType* pUAVariableType=(CUAVariableType*)pUABase;
								// par défaut on considère que le client nous demande un attribut invalide :(
								pResponse->Results[ii].StatusCode=OpcUa_BadAttributeIdInvalid;	
								// Compliant timestamp handling... 
								ZeroMemory(&(pResponse->Results[ii].SourceTimestamp),sizeof(OpcUa_DateTime));
								if (pUAVariable)
								{
									if (pRequest->IsServerTimestampRequested())
										pResponse->Results[ii].ServerTimestamp =OpcUa_DateTime_UtcNow();
									else
										ZeroMemory(&(pResponse->Results[ii].ServerTimestamp),sizeof(OpcUa_DateTime));
								}
								else
									pResponse->Results[ii].ServerTimestamp = OpcUa_DateTime_UtcNow();
								if (nodeToRead->AttributeId == OpcUa_Attributes_DataType)
								{
									pResponse->Results[ii].Value.Value.NodeId=(OpcUa_NodeId*)OpcUa_Alloc(sizeof(OpcUa_NodeId));
									OpcUa_NodeId_Initialize(pResponse->Results[ii].Value.Value.NodeId);
									pResponse->Results[ii].Value.Datatype = OpcUaType_NodeId;
									OpcUa_NodeId aDataTypeNodeId=pUAVariableType->GetDataType();
									OpcUa_NodeId_CopyTo(&aDataTypeNodeId,pResponse->Results[ii].Value.Value.NodeId);
									//
									if (uStatusPreChecked==OpcUa_BadOutOfRange)
										pResponse->Results[ii].StatusCode=uStatusPreChecked;	
									else
										pResponse->Results[ii].StatusCode=OpcUa_Good;
								}
								if (nodeToRead->AttributeId == OpcUa_Attributes_ValueRank)
								{
									pResponse->Results[ii].Value.Datatype = OpcUaType_Int32;
									pResponse->Results[ii].Value.ArrayType=0;
									pResponse->Results[ii].Value.Value.Int32 = pUAVariableType->GetValueRank();
									//
									if (uStatusPreChecked==OpcUa_BadOutOfRange)
										pResponse->Results[ii].StatusCode=uStatusPreChecked;	
									else
										pResponse->Results[ii].StatusCode=OpcUa_Good;			
								}
								if (nodeToRead->AttributeId == OpcUa_Attributes_IsAbstract)
								{
									pResponse->Results[ii].Value.ArrayType=0;
									pResponse->Results[ii].Value.Datatype=OpcUaType_Boolean;
									pResponse->Results[ii].Value.Value.Boolean=pUAVariableType->IsAbstract();
									if (uStatusPreChecked==OpcUa_BadOutOfRange)
										pResponse->Results[ii].StatusCode=uStatusPreChecked;	
									else
										pResponse->Results[ii].StatusCode=OpcUa_Good;		
								}		
								bOk=TRUE;
							}
							break;
							case OpcUa_NodeClass_Variable:
							{
								// par défaut on considère que le client nous demande un attribut invalide :(
								pResponse->Results[ii].StatusCode=OpcUa_BadAttributeIdInvalid;
								ZeroMemory(&(pResponse->Results[ii].SourceTimestamp),sizeof(OpcUa_DateTime));
								//
								pUAVariable=(CUAVariable*)pUABase;
								if (pUAVariable)
								{		
									// Compliant timestamp handling... 
									if (pRequest->IsServerTimestampRequested())
									{
										CDataValue* pDataValue = pUAVariable->GetValue();
										if (pDataValue)
										{
											if (dblMaxAge == 0)
												pDataValue->SetServerTimestamp(OpcUa_DateTime_UtcNow());
											pResponse->Results[ii].ServerTimestamp = pDataValue->GetServerTimestamp();
										}
									}
									else
									{
										ZeroMemory(&(pResponse->Results[ii].ServerTimestamp),sizeof(OpcUa_DateTime));
										/////////////////////////////////////////////////////////////////////////
										//std::string strTime;
										//hr=Utils::OpcUaDateTimeToString(pResponse->Results[ii].ServerTimestamp,&strTime);
										//OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Not Requested: ServerTimestamp: %s\n",strTime);
										/////////////////////////////////////////////////////////////////////////
									}
									if (nodeToRead->AttributeId==OpcUa_Attributes_MinimumSamplingInterval)
									{
										pResponse->Results[ii].Value.Datatype = OpcUaType_Double;
										pResponse->Results[ii].Value.ArrayType=0;
										pResponse->Results[ii].Value.Value.Double = pUAVariable->GetMinimumSamplingInterval();
										//
										if (uStatusPreChecked==OpcUa_BadOutOfRange)
											pResponse->Results[ii].StatusCode=uStatusPreChecked;	
										else
											pResponse->Results[ii].StatusCode=OpcUa_Good;
									}
									if (nodeToRead->AttributeId == OpcUa_Attributes_DataType)
									{
										pResponse->Results[ii].Value.Value.NodeId=(OpcUa_NodeId*)OpcUa_Alloc(sizeof(OpcUa_NodeId));
										OpcUa_NodeId_Initialize(pResponse->Results[ii].Value.Value.NodeId);
										pResponse->Results[ii].Value.Datatype = OpcUaType_NodeId;
										OpcUa_NodeId aNodeId=pUAVariable->GetDataType();
										OpcUa_NodeId_CopyTo(&aNodeId,pResponse->Results[ii].Value.Value.NodeId);
										if (uStatusPreChecked==OpcUa_BadOutOfRange)
											pResponse->Results[ii].StatusCode=OpcUa_BadIndexRangeNoData; 	
										else
											pResponse->Results[ii].StatusCode=OpcUa_Good;
									}	
									if (nodeToRead->AttributeId == OpcUa_Attributes_ValueRank)
									{
										pResponse->Results[ii].Value.Datatype = OpcUaType_Int32;
										pResponse->Results[ii].Value.ArrayType=0;
										pResponse->Results[ii].Value.Value.Int32 = pUAVariable->GetValueRank();
										//
										if (uStatusPreChecked==OpcUa_BadOutOfRange)
											pResponse->Results[ii].StatusCode=OpcUa_BadIndexRangeNoData; //uStatusPreChecked;
										else
											pResponse->Results[ii].StatusCode=OpcUa_Good;			
									}
									if (nodeToRead->AttributeId == OpcUa_Attributes_ArrayDimensions)
									{
										std::vector<OpcUa_UInt32>* pDimensions = pUAVariable->GetArrayDimensions();
										if (pDimensions)
										{
											if (pDimensions->size()>0)
											{
												pResponse->Results[ii].Value.Datatype = OpcUaType_UInt32;
												pResponse->Results[ii].Value.ArrayType=OpcUa_VariantArrayType_Array;
												int iSize=pDimensions->size();
												pResponse->Results[ii].Value.Value.Array.Length=iSize;
												pResponse->Results[ii].Value.Value.Array.Value.UInt32Array
																=(OpcUa_UInt32*)OpcUa_Alloc(iSize*sizeof(OpcUa_UInt32));										
												for (OpcUa_UInt32 iii=0;iii<pDimensions->size();iii++)
												{
													OpcUa_UInt32 ui32=pDimensions->at(iii);
													pResponse->Results[ii].Value.Value.Array.Value.UInt32Array[iii]=ui32;
												}
											}
											else
											{
												// Here we are facing a dynamic array. So let's try to figure out the size
												CDataValue* pDataValue = pUAVariable->GetValue();
												if (pDataValue)
												{
													if (pDataValue->GetArrayType() == OpcUa_VariantArrayType_Array)
													{
														pResponse->Results[ii].Value.Datatype = OpcUaType_UInt32;
														pResponse->Results[ii].Value.ArrayType = OpcUa_VariantArrayType_Array;
														pResponse->Results[ii].Value.Value.Array.Length = ((CUAVariable*)pUABase)->GetValueRank();
														pResponse->Results[ii].Value.Value.Array.Value.UInt32Array
															= (OpcUa_UInt32*)OpcUa_Alloc(sizeof(OpcUa_UInt32));
														pResponse->Results[ii].Value.Value.Array.Value.UInt32Array[0] = pDataValue->GetArraySize();
													}
												}
												else
													OpcUa_Variant_Initialize(&(pResponse->Results[ii].Value));
											}
										}
										if (uStatusPreChecked==OpcUa_BadOutOfRange)
											pResponse->Results[ii].StatusCode=uStatusPreChecked;	
										else
											pResponse->Results[ii].StatusCode=OpcUa_Good;			
									}
									if (nodeToRead->AttributeId == OpcUa_Attributes_Value)
									{
										CDataValue* pLocalDataValue = pUAVariable->GetValue();
										if (pLocalDataValue)
										{
											// VPI
											CVpiDevice* pDevice = OpcUa_Null;
											if (pUAVariable->GetPData())
											{
												// on a faire une variable reliée a un VPI
												CVpiTag* pSignal = (CVpiTag*)(pUAVariable->GetPData());
												if (pSignal)
												{
													pDevice = pSignal->GetDevice();
													CVpiFuncCaller* pFuncCaller = pDevice->GetVpiFuncCaller();
													OpcUa_NodeId* pNodeId = pUAVariable->GetNodeId();
													OpcUa_DataValue* pReadDataValue = (OpcUa_DataValue*)OpcUa_Alloc(sizeof(OpcUa_DataValue));
													pReadDataValue->Value.Datatype = pSignal->GetUAVariable()->GetBuiltInType();
													if (pSignal->GetNbElement() > 0)
													{
														pReadDataValue->Value.ArrayType = OpcUa_VariantArrayType_Array;
														pReadDataValue->Value.Value.Array.Length = pSignal->GetNbElement();
													}
													else
														pReadDataValue->Value.ArrayType = OpcUa_VariantArrayType_Scalar;
													if (pFuncCaller->m_hVpi)
													{
														uStatus = pFuncCaller->ReadValue(pFuncCaller->m_hVpi, 1, pNodeId, &pReadDataValue);
														if (uStatus == OpcUa_Good)//OpcUa_Vpi_Good
														{
															CDataValue* pDataValue = pUAVariable->GetValue();
															///////////////////////////////////////////////////
															if ((pUAVariable->GetBuiltInType() == OpcUaType_String) && (pSignal->GetNbElement()>0) && (pSignal->GetDataType().Identifier.Numeric == OpcUaType_Byte))
															{
																OpcUa_CharA* szValue = (OpcUa_CharA*)OpcUa_Alloc(pReadDataValue->Value.Value.Array.Length);
																ZeroMemory(szValue, pReadDataValue->Value.Value.Array.Length);
																OpcUa_MemCpy(szValue, 
																	pReadDataValue->Value.Value.Array.Length,
																	(OpcUa_CharA*)(pReadDataValue->Value.Value.Array.Value.ByteArray),
																	pReadDataValue->Value.Value.Array.Length);
																
																OpcUa_String strValue = pDataValue->GetValue().Value.String;
																OpcUa_String_Initialize(&strValue);
																OpcUa_String_AttachCopy(&strValue, szValue);
															}
															else
																///////////////////////////////////////////////////
																pUAVariable->SetValue(pReadDataValue->Value);

															// affectation de l'heure transmise par le VPI
															pLocalDataValue->SetSourceTimestamp(pReadDataValue->SourceTimestamp);
															// Qualité
															pLocalDataValue->SetStatusCode(pReadDataValue->StatusCode);
														}
													}
													else
														uStatus = OpcUa_Bad;
													OpcUa_DataValue_Clear(pReadDataValue);
													OpcUa_Free(pReadDataValue);
													pReadDataValue = OpcUa_Null;
												}
											}
											//int iSize = sizeof(pLocalDataValue->GetValue());
											pResponse->Results[ii].Value.Datatype = pLocalDataValue->GetValue().Datatype;
											// Array
											if (pLocalDataValue->GetValue().ArrayType == OpcUa_VariantArrayType_Array)
											{
												pUAVariable->Lock();
												if (pLocalDataValue)
												{
													OpcUa_DataValue* pDataValue = pLocalDataValue->GetInternalDataValue();
													if (pDataValue)
													{
														OpcUa_Variant* pVariant = &(pDataValue->Value); // ATTENTION CODE A REVISER DANS LA V2 (MC COMMENT)
														pResponse->Results[ii].Value.ArrayType = OpcUa_VariantArrayType_Array;
														pResponse->Results[ii].Value.Datatype = pVariant->Datatype;
														// Prise en compte d'une demande d'Index Range
														CNumericRange* pNumericRange = NULL;
														if (OpcUa_String_StrLen(&(nodeToRead->IndexRange)) > 0)
														{
															//OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Range receive=%s\n",OpcUa_String_GetRawString(&(nodeToRead->IndexRange)));
															pNumericRange = new CNumericRange(&(nodeToRead->IndexRange));
															uStatusPreChecked = pNumericRange->GetStatusCode();
														}
														OpcUa_Int32 iArrayLen = 0;
														if (pNumericRange)
															iArrayLen = (pNumericRange->GetEndIndex() - pNumericRange->GetBeginIndex()) + 1;
														else
															iArrayLen = pVariant->Value.Array.Length;
														// indice pour le comptage des élements réèlements ajoutés
														int jj = 0;
														// Analyse du type encapsulé dans le tableau
														switch (pVariant->Datatype)
														{
														case OpcUaType_Boolean:
														{
															iArrayLen = pVariant->Value.Array.Length;
															pResponse->Results[ii].Value.Value.Array.Value.BooleanArray
																= (OpcUa_Boolean*)OpcUa_Alloc(iArrayLen*sizeof(OpcUa_Boolean));

															pResponse->Results[ii].Value.Value.Array.Length = iArrayLen;
															for (OpcUa_Int32 iii = 0; iii < pVariant->Value.Array.Length; iii++)
															{
																OpcUa_Boolean bOk00 = OpcUa_True;
																if (pNumericRange)
																{
																	if (!pNumericRange->IsInRange(iii))
																		bOk00 = OpcUa_False;
																}
																if (bOk00)
																{
																	//uStatusPreChecked=OpcUa_Good;	
																	if (pVariant->Value.Array.Value.BooleanArray)
																		pResponse->Results[ii].Value.Value.Array.Value.BooleanArray[jj] = pVariant->Value.Array.Value.BooleanArray[iii];
																	jj++;
																}
															}
															pResponse->Results[ii].Value.Value.Array.Length = jj;
															if (jj == 0)
															{
																OpcUa_Free(pResponse->Results[ii].Value.Value.Array.Value.BooleanArray);
																pResponse->Results[ii].Value.Value.Array.Value.BooleanArray = NULL;
																pResponse->Results[ii].Value.Datatype = OpcUaType_Null;
																uStatusPreChecked = OpcUa_BadOutOfRange;
																CHECK_RANGE();
															}
														}
														break;
														case OpcUaType_Byte:
														{
															iArrayLen = pVariant->Value.Array.Length;
															pResponse->Results[ii].Value.Value.Array.Value.ByteArray
																= (OpcUa_Byte*)OpcUa_Alloc(iArrayLen*sizeof(OpcUa_Byte));

															pResponse->Results[ii].Value.Value.Array.Length = iArrayLen;
															for (int iii = 0; iii < pVariant->Value.Array.Length; iii++)
															{
																OpcUa_Boolean bOk00 = OpcUa_True;
																if (pNumericRange)
																{
																	if (!pNumericRange->IsInRange(iii))
																		bOk00 = OpcUa_False;
																}
																if (bOk00)
																{
																	//uStatusPreChecked=OpcUa_Good;	
																	if (pVariant->Value.Array.Value.ByteArray)
																		pResponse->Results[ii].Value.Value.Array.Value.ByteArray[jj] = pVariant->Value.Array.Value.ByteArray[iii];
																	jj++;
																}
															}
															pResponse->Results[ii].Value.Value.Array.Length = jj;
															if (jj == 0)
															{
																OpcUa_Free(pResponse->Results[ii].Value.Value.Array.Value.ByteArray);
																pResponse->Results[ii].Value.Value.Array.Value.ByteArray = NULL;
																pResponse->Results[ii].Value.Datatype = OpcUaType_Null;
																uStatusPreChecked = OpcUa_BadOutOfRange;
																CHECK_RANGE();
															}
														}
														break;
														case OpcUaType_SByte:
														{
															iArrayLen = pVariant->Value.Array.Length;
															pResponse->Results[ii].Value.Value.Array.Value.SByteArray
																= (OpcUa_SByte*)OpcUa_Alloc(iArrayLen*sizeof(OpcUa_SByte));

															pResponse->Results[ii].Value.Value.Array.Length = iArrayLen;
															for (int iii = 0; iii < pVariant->Value.Array.Length; iii++)
															{
																OpcUa_Boolean bOk00 = OpcUa_True;
																if (pNumericRange)
																{
																	if (!pNumericRange->IsInRange(iii))
																		bOk00 = OpcUa_False;
																}
																if (bOk00)
																{
																	//uStatusPreChecked=OpcUa_Good;	
																	if (pVariant->Value.Array.Value.SByteArray)
																		pResponse->Results[ii].Value.Value.Array.Value.SByteArray[jj] = pVariant->Value.Array.Value.SByteArray[iii];
																	jj++;
																}
															}
															pResponse->Results[ii].Value.Value.Array.Length = jj;
															if (jj == 0)
															{
																OpcUa_Free(pResponse->Results[ii].Value.Value.Array.Value.ByteArray);
																pResponse->Results[ii].Value.Value.Array.Value.SByteArray = NULL;
																pResponse->Results[ii].Value.Datatype = OpcUaType_Null;
																uStatusPreChecked = OpcUa_BadOutOfRange;
																CHECK_RANGE();
															}
														}
														break;
														case OpcUaType_ByteString:
														{
															iArrayLen = pVariant->Value.Array.Length;
															pResponse->Results[ii].Value.Value.Array.Value.ByteStringArray
																= (OpcUa_ByteString*)OpcUa_Alloc(iArrayLen*sizeof(OpcUa_ByteString));

															pResponse->Results[ii].Value.Value.Array.Length = iArrayLen;
															for (int iii = 0; iii < pVariant->Value.Array.Length; iii++)
															{
																OpcUa_Boolean bOk00 = OpcUa_True;
																if (pNumericRange)
																{
																	if (!pNumericRange->IsInRange(iii))
																		bOk00 = OpcUa_False;
																}
																if (bOk00)
																{
																	if (pVariant->Value.Array.Value.ByteStringArray[iii].Data)
																	{
																		OpcUa_ByteString_CopyTo(&(pVariant->Value.Array.Value.ByteStringArray[iii]),
																			&(pResponse->Results[ii].Value.Value.Array.Value.ByteStringArray[jj]));
																	}
																	else
																	{
																		pResponse->Results[ii].Value.Value.Array.Value.ByteStringArray[jj].Data = OpcUa_Null;
																		pResponse->Results[ii].Value.Value.Array.Value.ByteStringArray[jj].Length = 0;
																	}
																	jj++;
																}
															}
															pResponse->Results[ii].Value.Value.Array.Length = jj;
															if (jj == 0)
															{
																OpcUa_Free(pResponse->Results[ii].Value.Value.Array.Value.ByteStringArray);
																pResponse->Results[ii].Value.Value.Array.Value.ByteStringArray = NULL;
																pResponse->Results[ii].Value.Datatype = OpcUaType_Null;
																uStatusPreChecked = OpcUa_BadOutOfRange;
																CHECK_RANGE();
															}
														}
														break;
														case OpcUaType_Int16:
														{
															iArrayLen = pVariant->Value.Array.Length;
															pResponse->Results[ii].Value.Value.Array.Value.Int16Array
																= (OpcUa_Int16*)OpcUa_Alloc(iArrayLen*sizeof(OpcUa_Int16));

															pResponse->Results[ii].Value.Value.Array.Length = iArrayLen;
															for (int iii = 0; iii < pVariant->Value.Array.Length; iii++)
															{
																OpcUa_Boolean bOk00 = OpcUa_True;
																if (pNumericRange)
																{
																	if (!pNumericRange->IsInRange(iii))
																		bOk00 = OpcUa_False;
																}
																if (bOk00)
																{
																	//uStatusPreChecked=OpcUa_Good;	
																	if (pVariant->Value.Array.Value.Int16Array)
																		pResponse->Results[ii].Value.Value.Array.Value.Int16Array[jj] = pVariant->Value.Array.Value.Int16Array[iii];
																	jj++;
																}
															}
															pResponse->Results[ii].Value.Value.Array.Length = jj;
															if (jj == 0)
															{
																OpcUa_Free(pResponse->Results[ii].Value.Value.Array.Value.Int16Array);
																pResponse->Results[ii].Value.Value.Array.Value.Int16Array = NULL;
																pResponse->Results[ii].Value.Datatype = OpcUaType_Null;
																uStatusPreChecked = OpcUa_BadOutOfRange;
																CHECK_RANGE();
															}
														}
														break;
														case OpcUaType_Int32:
														{
															iArrayLen = pVariant->Value.Array.Length;
															pResponse->Results[ii].Value.Value.Array.Value.Int32Array
																= (OpcUa_Int32*)OpcUa_Alloc(iArrayLen*sizeof(OpcUa_Int32));

															pResponse->Results[ii].Value.Value.Array.Length = iArrayLen;
															for (int iii = 0; iii < pVariant->Value.Array.Length; iii++)
															{
																OpcUa_Boolean bOk00 = OpcUa_True;
																if (pNumericRange)
																{
																	if (!pNumericRange->IsInRange(iii))
																		bOk00 = OpcUa_False;
																}
																if (bOk00)
																{
																	//uStatusPreChecked=OpcUa_Good;	
																	if (pVariant->Value.Array.Value.Int32Array)
																		pResponse->Results[ii].Value.Value.Array.Value.Int32Array[jj] = pVariant->Value.Array.Value.Int32Array[iii];
																	jj++;
																}
															}
															pResponse->Results[ii].Value.Value.Array.Length = jj;
															if (jj == 0)
															{
																OpcUa_Free(pResponse->Results[ii].Value.Value.Array.Value.Int32Array);
																pResponse->Results[ii].Value.Value.Array.Value.Int32Array = NULL;
																pResponse->Results[ii].Value.Datatype = OpcUaType_Null;
																uStatusPreChecked = OpcUa_BadOutOfRange;
																CHECK_RANGE();
															}
														}
														break;
														case OpcUaType_UInt16:
														{
															iArrayLen = pVariant->Value.Array.Length;
															pResponse->Results[ii].Value.Value.Array.Value.UInt16Array
																= (OpcUa_UInt16*)OpcUa_Alloc(iArrayLen*sizeof(OpcUa_UInt16));

															pResponse->Results[ii].Value.Value.Array.Length = iArrayLen;
															for (int iii = 0; iii < pVariant->Value.Array.Length; iii++)
															{
																OpcUa_Boolean bOk00 = OpcUa_True;
																if (pNumericRange)
																{
																	if (!pNumericRange->IsInRange(iii))
																		bOk00 = OpcUa_False;
																}
																if (bOk00)
																{
																	//uStatusPreChecked=OpcUa_Good;	
																	if (pVariant->Value.Array.Value.UInt16Array)
																		pResponse->Results[ii].Value.Value.Array.Value.UInt16Array[jj] = pVariant->Value.Array.Value.UInt16Array[iii];
																	jj++;
																}
															}
															pResponse->Results[ii].Value.Value.Array.Length = jj;
															if (jj == 0)
															{
																OpcUa_Free(pResponse->Results[ii].Value.Value.Array.Value.Int16Array);
																pResponse->Results[ii].Value.Value.Array.Value.UInt16Array = NULL;
																pResponse->Results[ii].Value.Datatype = OpcUaType_Null;
																uStatusPreChecked = OpcUa_BadOutOfRange;
																CHECK_RANGE();
															}
														}
														break;
														case OpcUaType_UInt32:
														{
															iArrayLen = pVariant->Value.Array.Length;
															pResponse->Results[ii].Value.Value.Array.Value.UInt32Array
																= (OpcUa_UInt32*)OpcUa_Alloc(iArrayLen*sizeof(OpcUa_UInt32));

															pResponse->Results[ii].Value.Value.Array.Length = iArrayLen;
															for (int iii = 0; iii < pVariant->Value.Array.Length; iii++)
															{
																OpcUa_Boolean bOk00 = OpcUa_True;
																if (pNumericRange)
																{
																	if (!pNumericRange->IsInRange(iii))
																		bOk00 = OpcUa_False;
																}
																if (bOk00)
																{
																	//uStatusPreChecked=OpcUa_Good;	
																	if (pVariant->Value.Array.Value.UInt32Array)
																		pResponse->Results[ii].Value.Value.Array.Value.UInt32Array[jj] = pVariant->Value.Array.Value.UInt32Array[iii];
																	jj++;
																}
															}
															pResponse->Results[ii].Value.Value.Array.Length = jj;
															if (jj == 0)
															{
																OpcUa_Free(pResponse->Results[ii].Value.Value.Array.Value.Int32Array);
																pResponse->Results[ii].Value.Value.Array.Value.UInt32Array = NULL;
																pResponse->Results[ii].Value.Datatype = OpcUaType_Null;
																uStatusPreChecked = OpcUa_BadOutOfRange;
																CHECK_RANGE();
															}
														}
														break;
														case OpcUaType_Float:
														{
															iArrayLen = pVariant->Value.Array.Length;
															pResponse->Results[ii].Value.Value.Array.Value.FloatArray
																= (OpcUa_Float*)OpcUa_Alloc(iArrayLen*sizeof(OpcUa_Float));

															pResponse->Results[ii].Value.Value.Array.Length = iArrayLen;
															for (int iii = 0; iii < pVariant->Value.Array.Length; iii++)
															{
																OpcUa_Boolean bOk00 = OpcUa_True;
																if (pNumericRange)
																{
																	if (!pNumericRange->IsInRange(iii))
																		bOk00 = OpcUa_False;
																}
																if (bOk00)
																{
																	//uStatusPreChecked=OpcUa_Good;	
																	if (pVariant->Value.Array.Value.FloatArray)
																		pResponse->Results[ii].Value.Value.Array.Value.FloatArray[jj] = pVariant->Value.Array.Value.FloatArray[iii];
																	jj++;
																}
															}
															pResponse->Results[ii].Value.Value.Array.Length = jj;
															if (jj == 0)
															{
																OpcUa_Free(pResponse->Results[ii].Value.Value.Array.Value.FloatArray);
																pResponse->Results[ii].Value.Value.Array.Value.FloatArray = NULL;
																pResponse->Results[ii].Value.Datatype = OpcUaType_Null;
																uStatusPreChecked = OpcUa_BadOutOfRange;
																CHECK_RANGE();
															}
														}
														break;
														case OpcUaType_Double:
														{
															iArrayLen = pVariant->Value.Array.Length;
															pResponse->Results[ii].Value.Value.Array.Value.DoubleArray
																= (OpcUa_Double*)OpcUa_Alloc(iArrayLen*sizeof(OpcUa_Double));

															pResponse->Results[ii].Value.Value.Array.Length = iArrayLen;
															for (int iii = 0; iii < pVariant->Value.Array.Length; iii++)
															{
																OpcUa_Boolean bOk00 = OpcUa_True;
																if (pNumericRange)
																{
																	if (!pNumericRange->IsInRange(iii))
																		bOk00 = OpcUa_False;
																}
																if (bOk00)
																{
																	if (pVariant->Value.Array.Value.DoubleArray)
																		pResponse->Results[ii].Value.Value.Array.Value.DoubleArray[jj] = pVariant->Value.Array.Value.DoubleArray[iii];
																	jj++;
																}
															}
															pResponse->Results[ii].Value.Value.Array.Length = jj;
															if (jj == 0)
															{
																OpcUa_Free(pResponse->Results[ii].Value.Value.Array.Value.DoubleArray);
																pResponse->Results[ii].Value.Value.Array.Value.DoubleArray = NULL;
																pResponse->Results[ii].Value.Datatype = OpcUaType_Null;
																uStatusPreChecked = OpcUa_BadOutOfRange;
																CHECK_RANGE();
															}
														}
														break;
														case OpcUaType_Guid:
														{
															iArrayLen = pVariant->Value.Array.Length;
															pResponse->Results[ii].Value.Value.Array.Value.GuidArray
																= (OpcUa_Guid*)OpcUa_Alloc(iArrayLen*sizeof(OpcUa_Guid));

															pResponse->Results[ii].Value.Value.Array.Length = iArrayLen;
															for (int iii = 0; iii < pVariant->Value.Array.Length; iii++)
															{
																OpcUa_Boolean bOk00 = OpcUa_True;
																if (pNumericRange)
																{
																	if (!pNumericRange->IsInRange(iii))
																		bOk00 = OpcUa_False;
																}
																if (bOk00)
																{
																	if (pVariant->Value.Array.Value.GuidArray)
																		pResponse->Results[ii].Value.Value.Array.Value.GuidArray[jj] = pVariant->Value.Array.Value.GuidArray[iii];
																	jj++;
																}
															}
															pResponse->Results[ii].Value.Value.Array.Length = jj;
															if (jj == 0)
															{
																OpcUa_Free(pResponse->Results[ii].Value.Value.Array.Value.GuidArray);
																pResponse->Results[ii].Value.Value.Array.Value.GuidArray = NULL;
																pResponse->Results[ii].Value.Datatype = OpcUaType_Null;
																uStatusPreChecked = OpcUa_BadOutOfRange;
																CHECK_RANGE();
															}
														}
														break;
														case OpcUaType_DateTime:
														{
															iArrayLen = pVariant->Value.Array.Length;
															pResponse->Results[ii].Value.Value.Array.Value.DateTimeArray
																= (OpcUa_DateTime*)OpcUa_Alloc(iArrayLen*sizeof(OpcUa_DateTime));

															pResponse->Results[ii].Value.Value.Array.Length = iArrayLen;
															for (int iii = 0; iii < pVariant->Value.Array.Length; iii++)
															{
																OpcUa_Boolean bOk00 = OpcUa_True;
																if (pNumericRange)
																{
																	if (!pNumericRange->IsInRange(iii))
																		bOk00 = OpcUa_False;
																}
																if (bOk00)
																{
																	OpcUa_DateTime_CopyTo(&(pVariant->Value.Array.Value.DateTimeArray[iii]),
																		&(pResponse->Results[ii].Value.Value.Array.Value.DateTimeArray[jj]));
																	jj++;
																}
															}
															pResponse->Results[ii].Value.Value.Array.Length = jj;
															if (jj == 0)
															{
																OpcUa_Free(pResponse->Results[ii].Value.Value.Array.Value.DateTimeArray);
																pResponse->Results[ii].Value.Value.Array.Value.DateTimeArray = NULL;
																pResponse->Results[ii].Value.Datatype = OpcUaType_Null;
																uStatusPreChecked = OpcUa_BadOutOfRange;
																CHECK_RANGE();
															}
														}
														break;
														case OpcUaType_LocalizedText:
														{
															iArrayLen = pVariant->Value.Array.Length;
															pResponse->Results[ii].Value.Value.Array.Value.LocalizedTextArray
																= (OpcUa_LocalizedText*)OpcUa_Alloc(iArrayLen*sizeof(OpcUa_LocalizedText));
															for (int iii = 0; iii < pVariant->Value.Array.Length; iii++)
															{
																OpcUa_Boolean bOk00 = OpcUa_True;
																if (pNumericRange)
																{
																	if (!pNumericRange->IsInRange(iii))
																		bOk00 = OpcUa_False;
																}
																if (bOk00)
																{
																	OpcUa_LocalizedText_CopyTo(&(pVariant->Value.Array.Value.LocalizedTextArray[iii]),
																		&(pResponse->Results[ii].Value.Value.Array.Value.LocalizedTextArray[jj]));
																	jj++;
																}
															}
															pResponse->Results[ii].Value.Value.Array.Length = jj;
															if (jj == 0)
															{
																OpcUa_Free(pResponse->Results[ii].Value.Value.Array.Value.LocalizedTextArray);
																pResponse->Results[ii].Value.Value.Array.Value.LocalizedTextArray = NULL;
																pResponse->Results[ii].Value.Datatype = OpcUaType_Null;
																uStatusPreChecked = OpcUa_BadOutOfRange;
																CHECK_RANGE();
															}
														}
														break;
														case OpcUaType_Variant:
														{
															iArrayLen = pVariant->Value.Array.Length;
															pResponse->Results[ii].Value.Value.Array.Value.Int32Array
																= (OpcUa_Int32*)OpcUa_Alloc(iArrayLen*sizeof(OpcUa_Int32));

															for (int iii = 0; iii < pVariant->Value.Array.Length; iii++)
															{
																OpcUa_Boolean bOk00 = OpcUa_True;
																if (pNumericRange)
																{
																	if (!pNumericRange->IsInRange(iii))
																		bOk00 = OpcUa_False;
																}
																if (bOk00)
																{
																	//uStatusPreChecked=OpcUa_Good;
																	pResponse->Results[ii].Value.Value.Array.Value.Int32Array[jj] = 0;
																	jj++;
																}
															}
															pResponse->Results[ii].Value.Value.Array.Length = jj;
															if (jj == 0)
															{
																OpcUa_Free(pResponse->Results[ii].Value.Value.Array.Value.VariantArray);
																pResponse->Results[ii].Value.Value.Array.Value.VariantArray = NULL;
																pResponse->Results[ii].Value.Datatype = OpcUaType_Null;
																uStatusPreChecked = OpcUa_BadOutOfRange;
																CHECK_RANGE();
															}
														}
														break;
														case OpcUaType_String:
														{
															iArrayLen = pVariant->Value.Array.Length;
															pResponse->Results[ii].Value.Value.Array.Value.StringArray
																= (OpcUa_String*)OpcUa_Alloc(iArrayLen*sizeof(OpcUa_String));
															for (int iii = 0; iii < pVariant->Value.Array.Length; iii++)
															{
																OpcUa_Boolean bOk00 = OpcUa_True;
																if (pNumericRange)
																{
																	if (!pNumericRange->IsInRange(iii))
																		bOk00 = OpcUa_False;
																}
																if (bOk00)
																{
																	// transfert du contenu
																	OpcUa_String_Initialize(&(pResponse->Results[ii].Value.Value.Array.Value.StringArray[jj]));
																	OpcUa_String_CopyTo(&(pVariant->Value.Array.Value.StringArray[iii]),
																		&(pResponse->Results[ii].Value.Value.Array.Value.StringArray[jj]));
																	//OpcUa_String_AttachCopy(&(pResponse->Results[ii].Value.Value.Array.Value.StringArray[jj]),
																	//	OpcUa_String_GetRawString(&(pVariant->Value.Array.Value.StringArray[iii])) );
																	jj++;
																}
															}
															pResponse->Results[ii].Value.Value.Array.Length = jj;
															if (jj == 0)
															{
																OpcUa_Free(pResponse->Results[ii].Value.Value.Array.Value.StringArray);
																pResponse->Results[ii].Value.Value.Array.Value.StringArray = NULL;
																pResponse->Results[ii].Value.Datatype = OpcUaType_Null;
																uStatusPreChecked = OpcUa_BadOutOfRange;
																CHECK_RANGE();
															}
														}
														break;
														case OpcUaType_ExtensionObject:
														{
															pResponse->Results[ii].Value.Value.Array.Value.ExtensionObjectArray
																= (OpcUa_ExtensionObject*)OpcUa_Alloc(iArrayLen*sizeof(OpcUa_ExtensionObject));
															pResponse->Results[ii].StatusCode = OpcUa_Good;
															for (int iii = 0; iii < pVariant->Value.Array.Length; iii++)
															{
																OpcUa_Boolean bOk00 = OpcUa_True;
																if (pNumericRange)
																{
																	if (!pNumericRange->IsInRange(iii))
																		bOk00 = OpcUa_False;
																}
																if (bOk00)
																{
																	//uStatusPreChecked=OpcUa_Good;
																	OpcUa_ExtensionObject* pExtensionObject = OpcUa_Null;// &(pVariant->Value.Array.Value.ExtensionObjectArray[iii]);
																	uStatus = Copy(&pExtensionObject,//&(pResponse->Results[ii].Value.Value.Array.Value.ExtensionObjectArray)
																		&(pVariant->Value.Array.Value.ExtensionObjectArray[iii]));
																	if (uStatus != OpcUa_Good)
																		//OpcUa_ExtensionObject aExtensionObject=*Utils::Copy(&(pVariant->Value.Array.Value.ExtensionObjectArray[iii]));
																		//OpcUa_EncodeableType* pEncodeableType=aExtensionObject.Body.EncodeableObject.Type;
																		//if (pEncodeableType)
																		//{
																		//		if (OpcUa_StrCmpA(pEncodeableType->TypeName,"BuildInfo")==0)
																		//			OpcUa_ExtensionObject_Array_Copy(BuildInfo);
																		//		if (OpcUa_StrCmpA(pEncodeableType->TypeName,"Argument")==0)
																		//			OpcUa_ExtensionObject_Array_Copy(Argument);
																		//		if (OpcUa_StrCmpA(pEncodeableType->TypeName,"SessionDiagnosticsDataType")==0)
																		//			OpcUa_ExtensionObject_Array_Copy(SessionDiagnosticsDataType);
																		//		if (OpcUa_StrCmpA(pEncodeableType->TypeName,"SessionSecurityDiagnosticsDataType")==0)
																		//			OpcUa_ExtensionObject_Array_Copy(SessionSecurityDiagnosticsDataType);
																		//		if (OpcUa_StrCmpA(pEncodeableType->TypeName,"SamplingIntervalDiagnosticsDataType")==0)
																		//			OpcUa_ExtensionObject_Array_Copy(SamplingIntervalDiagnosticsDataType);
																		//		if (OpcUa_StrCmpA(pEncodeableType->TypeName,"RedundantServerDataType")==0)
																		//			OpcUa_ExtensionObject_Array_Copy(RedundantServerDataType);
																		//		if (OpcUa_StrCmpA(pEncodeableType->TypeName,"SemanticChangeStructureDataType")==0)
																		//			OpcUa_ExtensionObject_Array_Copy(SemanticChangeStructureDataType);
																		//		if (OpcUa_StrCmpA(pEncodeableType->TypeName,"ServerStatusDataType")==0)
																		//			OpcUa_ExtensionObject_Array_Copy(ServerStatusDataType);
																		//		if (OpcUa_StrCmpA(pEncodeableType->TypeName,"ServerDiagnosticsSummaryDataType")==0)
																		//			OpcUa_ExtensionObject_Array_Copy(ServerDiagnosticsSummaryDataType);
																		//		if (OpcUa_StrCmpA(pEncodeableType->TypeName,"ServiceCounterDataType")==0)
																		//			OpcUa_ExtensionObject_Array_Copy(ServiceCounterDataType);
																		//		if (OpcUa_StrCmpA(pEncodeableType->TypeName,"SignedSoftwareCertificate")==0)
																		//			OpcUa_ExtensionObject_Array_Copy(SignedSoftwareCertificate);
																		//		if (OpcUa_StrCmpA(pEncodeableType->TypeName,"SoftwareCertificate")==0)
																		//			OpcUa_ExtensionObject_Array_Copy(SoftwareCertificate);
																		//		if (OpcUa_StrCmpA(pEncodeableType->TypeName,"SubscriptionDiagnosticsDataType")==0)
																		//			OpcUa_ExtensionObject_Array_Copy(SubscriptionDiagnosticsDataType);
																		//		if (OpcUa_StrCmpA(pEncodeableType->TypeName,"SupportedProfile")==0)
																		//			OpcUa_ExtensionObject_Array_Copy(SupportedProfile);
																		//		if (OpcUa_StrCmpA(pEncodeableType->TypeName,"Range")==0)
																		//			OpcUa_ExtensionObject_Array_Copy(Range);
																		//		if (OpcUa_StrCmpA(pEncodeableType->TypeName,"EUInformation")==0)
																		//			OpcUa_ExtensionObject_Array_Copy(EUInformation);
																		//		if (OpcUa_StrCmpA(pEncodeableType->TypeName,"UserIdentityToken")==0)
																		//			OpcUa_ExtensionObject_Array_Copy(UserIdentityToken);
																		//		if (OpcUa_StrCmpA(pEncodeableType->TypeName,"AddNodesItem")==0)
																		//			OpcUa_ExtensionObject_Array_Copy(AddNodesItem);
																		//		if (OpcUa_StrCmpA(pEncodeableType->TypeName,"DeleteNodesItem")==0)
																		//			OpcUa_ExtensionObject_Array_Copy(DeleteNodesItem);
																		//		if (OpcUa_StrCmpA(pEncodeableType->TypeName,"AddReferencesItem")==0)
																		//			OpcUa_ExtensionObject_Array_Copy(AddReferencesItem);
																		//		if (OpcUa_StrCmpA(pEncodeableType->TypeName,"DeleteReferencesItem")==0)
																		//			OpcUa_ExtensionObject_Array_Copy(DeleteReferencesItem);
																		//		if (OpcUa_StrCmpA(pEncodeableType->TypeName,"ModelChangeStructureDataType")==0)
																		//			OpcUa_ExtensionObject_Array_Copy(ModelChangeStructureDataType);
																		//		if (OpcUa_StrCmpA(pEncodeableType->TypeName,"SemanticChangeStructureDataType")==0)
																		//			OpcUa_ExtensionObject_Array_Copy(SemanticChangeStructureDataType);
																		//		if (OpcUa_StrCmpA(pEncodeableType->TypeName,"ApplicationDescription")==0)
																		//			OpcUa_ExtensionObject_Array_Copy(ApplicationDescription);
																		//		if (OpcUa_StrCmpA(pEncodeableType->TypeName,"StatusResult")==0)
																		//			OpcUa_ExtensionObject_Array_Copy(StatusResult);
																		//		if (OpcUa_StrCmpA(pEncodeableType->TypeName,"DiagnosticInfo")==0)
																		//			OpcUa_ExtensionObject_Array_Copy(DiagnosticInfo);
																		//		if (OpcUa_StrCmpA(pEncodeableType->TypeName,"ProgramDiagnosticDataType")==0)
																		//			OpcUa_ExtensionObject_Array_Copy(ProgramDiagnosticDataType);
																		//		if (OpcUa_StrCmpA(pEncodeableType->TypeName,"EventFilter")==0)
																		//			OpcUa_ExtensionObject_Array_Copy(EventFilterResult);
																		//		if (OpcUa_StrCmpA(pEncodeableType->TypeName,"HistoryEventFieldList")==0)
																		//			OpcUa_ExtensionObject_Array_Copy(HistoryEventFieldList);
																		//		if (OpcUa_StrCmpA(pEncodeableType->TypeName,"TimeZoneDataType")==0)
																		//			OpcUa_ExtensionObject_Array_Copy(TimeZoneDataType);
																		//		if (OpcUa_StrCmpA(pEncodeableType->TypeName,"EnumValueType")==0)
																		//			OpcUa_ExtensionObject_Array_Copy(EnumValueType);
																		//		if (OpcUa_StrCmpA(pEncodeableType->TypeName,"RedundantServerDataType")==0)
																		//			OpcUa_ExtensionObject_Array_Copy(RedundantServerDataType);
																		//}
																		//else
																	{
																		char* szNodeId = OpcUa_Null;
																		OpcUa_NodeId nodeId = pUAVariable->GetDataType();
																		Utils::NodeId2String(&nodeId, &szNodeId);
																		if (szNodeId)
																		{
																			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "ProcessReadRequest>Unsupported Array EncodeableType %s\n", szNodeId);
																			free(szNodeId);
																		}
																		pResponse->Results[ii].StatusCode = OpcUa_BadNotSupported;
																	}
																	else
																	{
																		if (pExtensionObject)
																		{
																			OpcUa_ExtensionObject_CopyTo(pExtensionObject, &pResponse->Results[ii].Value.Value.Array.Value.ExtensionObjectArray[jj]);
																			OpcUa_ExtensionObject_Clear(pExtensionObject);
																			OpcUa_Free(pExtensionObject);
																		}
																		else
																			pResponse->Results[ii].StatusCode = OpcUa_BadNotSupported;
																	}
																	jj++;
																}
															}
															pResponse->Results[ii].Value.Value.Array.Length = jj;
															if (jj == 0)
															{
																OpcUa_Free(pResponse->Results[ii].Value.Value.Array.Value.ExtensionObjectArray);
																pResponse->Results[ii].Value.Value.Array.Value.ExtensionObjectArray = OpcUa_Null;
																pResponse->Results[ii].Value.ArrayType = OpcUa_VariantArrayType_Array;
																pResponse->Results[ii].Value.Value.Array.Length = 0;
																pResponse->Results[ii].Value.Datatype = OpcUaType_ExtensionObject;// OpcUaType_Null;
																uStatusPreChecked = OpcUa_BadOutOfRange;
																CHECK_RANGE();
															}
														}
														break;
														default:
															OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "pVariant->Datatype not supported yet: %u\n", pVariant->Datatype);
														}
														// release ressources
														if (pNumericRange)
															delete pNumericRange;
													}
													else
														OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critical Error>You are trying to read an unintialized UAVariable(OpcUa_DataValue)\n");
												}
												else
													OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critical Error>You are trying to read an unintialized UAVariable (CDataValue)\n");
												pUAVariable->UnLock();
											}
											else // autre chose qu'un array
											{
												// Matrix
												if (pLocalDataValue->GetValue().ArrayType == OpcUa_VariantArrayType_Matrix)
												{
													// non supporté
													OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "OpcUa_VariantArrayType_Matrix not supported yet\n");
												}
												// Scalar
												else
												{
													if (pLocalDataValue->GetValue().ArrayType == OpcUa_VariantArrayType_Scalar)
													{
														CNumericRange* pNumericRange = NULL;
														if (OpcUa_String_StrLen(&(nodeToRead->IndexRange)) > 0)
														{
															pNumericRange = new CNumericRange(&(nodeToRead->IndexRange));
															//OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Range receive=%s\n",OpcUa_String_GetRawString(&(nodeToRead->IndexRange)));
														}
														switch (pUAVariable->GetBuiltInType())
														{
															case OpcUaType_NodeId:
															{
																pResponse->Results[ii].Value.ArrayType = 0;
																pResponse->Results[ii].Value.Datatype = OpcUaType_NodeId;
																pResponse->Results[ii].Value.Value.NodeId = Utils::Copy(pLocalDataValue->GetValue().Value.NodeId);
															}
															break;
															case OpcUaType_Variant:
																// Cas particulier.
																// La stack ne sais pas encoder un variant de variant
																// Je vais donc mettre un int32 a l'interieur
															{
																OpcUa_Variant aVariant = pLocalDataValue->GetValue();
																if (aVariant.Datatype)
																{
																	pResponse->Results[ii].Value.ArrayType = 0;
																	pResponse->Results[ii].Value.Datatype = aVariant.Datatype;
																	OpcUa_Variant_CopyTo(&aVariant, &(pResponse->Results[ii].Value));
																}
															}
															break;
															case OpcUaType_String:
															{
																OpcUa_String aString = pLocalDataValue->GetValue().Value.String;
																if (pNumericRange)
																{
																	if (OpcUa_String_StrLen(&aString) > 0)
																	{
																		std::string tmpStr(OpcUa_String_GetRawString(&aString));
																		if (tmpStr.size() > 0)
																		{
																			if ((pNumericRange->GetEndIndex() - pNumericRange->GetBeginIndex()) <= (OpcUa_Int32)tmpStr.size())
																			{
																				std::string tmpStr2 = tmpStr.substr(pNumericRange->GetBeginIndex(), pNumericRange->GetEndIndex());
																				if (tmpStr2.size() > 0)
																					OpcUa_String_AttachCopy(&(pResponse->Results[ii].Value.Value.String), tmpStr2.c_str());
																				else
																				{
																					OpcUa_String_Initialize(&(pResponse->Results[ii].Value.Value.String));
																					pResponse->Results[ii].Value.Datatype = OpcUaType_Null;
																					uStatusPreChecked = OpcUa_BadOutOfRange;
																					uStatusFinal = OpcUa_BadIndexRangeNoData;
																				}
																			}
																			else
																			{
																				OpcUa_String_Initialize(&(pResponse->Results[ii].Value.Value.String));
																				pResponse->Results[ii].Value.Datatype = OpcUaType_Null;
																				uStatusPreChecked = OpcUa_BadOutOfRange;
																				uStatusFinal = OpcUa_BadIndexRangeNoData;
																			}
																		}
																	}
																	else
																	{
																		OpcUa_String_Initialize(&(pResponse->Results[ii].Value.Value.String));
																		pResponse->Results[ii].Value.Datatype = OpcUaType_Null;
																		uStatusPreChecked = OpcUa_BadOutOfRange;
																		uStatusFinal = OpcUa_BadIndexRangeNoData;
																	}
																}
																else
																{
																	OpcUa_String_StrnCpy(
																		&(pResponse->Results[ii].Value.Value.String),
																		&aString,
																		OpcUa_String_StrLen(&aString));
																}
															}
															break;
															case OpcUaType_LocalizedText:
															{
																// locale
																if (pLocalDataValue->GetValue().Value.LocalizedText)
																{
																	pResponse->Results[ii].Value.Value.LocalizedText = (OpcUa_LocalizedText*)OpcUa_Alloc(sizeof(OpcUa_LocalizedText));
																	if (pResponse->Results[ii].Value.Value.LocalizedText)
																	{
																		OpcUa_LocalizedText_Initialize(pResponse->Results[ii].Value.Value.LocalizedText);
																		OpcUa_Variant aVariant = pLocalDataValue->GetValue();
																		OpcUa_LocalizedText_CopyTo(aVariant.Value.LocalizedText, pResponse->Results[ii].Value.Value.LocalizedText);
																	}
																	else
																		uStatus = OpcUa_BadOutOfMemory;
																}
																else
																{
																	char* szNodeId = OpcUa_Null;
																	// NodeId
																	OpcUa_NodeId* pNodeId = pUAVariable->GetNodeId();
																	Utils::NodeId2String(pNodeId, &szNodeId);
																	if (szNodeId)
																	{
																		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "NodeID: %s was not initialize.\n", szNodeId);
																		free(szNodeId);
																	}
																}
															}
															break;
															case OpcUaType_ExtensionObject:
															{
																uStatus = ReadScalarExtensionObject(pUAVariable, ii, pResponse);
															}
															break;
															case OpcUaType_QualifiedName:
															{
																pResponse->Results[ii].Value.Value.QualifiedName = Utils::Copy(pLocalDataValue->GetValue().Value.QualifiedName);
																if (pNumericRange)
																{

																	uStatusPreChecked = OpcUa_BadOutOfRange;
																	uStatusFinal = OpcUa_BadOutOfRange;
																}
															}
															break;
															case OpcUaType_Guid:
															{
																pResponse->Results[ii].Value.Value.Guid = Utils::Copy(pLocalDataValue->GetValue().Value.Guid);
																if (pNumericRange)
																{

																	uStatusPreChecked = OpcUa_BadOutOfRange;
																	uStatusFinal = OpcUa_BadOutOfRange;
																}
															}
															break;
															case OpcUaType_XmlElement:
															case OpcUaType_ByteString:
															{
																OpcUa_Int32 iLen = pLocalDataValue->GetValue().Value.ByteString.Length;
																if (iLen)
																{
																	pResponse->Results[ii].Value.Value.ByteString.Data = (OpcUa_Byte*)OpcUa_Alloc(iLen);
																	pResponse->Results[ii].Value.Value.ByteString.Length = iLen;
																	OpcUa_MemCpy((pResponse->Results[ii].Value.Value.ByteString.Data), iLen, (pLocalDataValue->GetValue().Value.ByteString.Data), iLen);
																}
																else
																{
																	OpcUa_ByteString_Initialize(&(pResponse->Results[ii].Value.Value.ByteString));
																	pResponse->Results[ii].Value.Value.ByteString.Length = 0;
																	OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Read Operatio:OpcUaType_ByteString is empty nothing to return\n");
																}
																if (pNumericRange)
																{

																	uStatusPreChecked = OpcUa_BadOutOfRange;
																	uStatusFinal = OpcUa_BadOutOfRange;
																}
															}
															break;
															case OpcUaType_DateTime:
															{
																OpcUa_Variant aVariant = pLocalDataValue->GetValue();
																OpcUa_DateTime_CopyTo(&(aVariant.Value.DateTime), &(pResponse->Results[ii].Value.Value.DateTime)); // = *pDateTime;
																if (pNumericRange)
																{
																	OpcUa_DateTime_Initialize(&(aVariant.Value.DateTime));
																	pResponse->Results[ii].Value.Datatype = OpcUaType_Null;
																	uStatusPreChecked = OpcUa_BadOutOfRange;
																	uStatusFinal = OpcUa_BadOutOfRange;
																}
															}
															break;
															default:
															{
																OpcUa_Variant aVariant = pLocalDataValue->GetValue();
																OpcUa_Variant_CopyTo(&aVariant, &(pResponse->Results[ii].Value)); // modif 12-02-2016
																//OpcUa_Int32 iSize = sizeof(pLocalDataValue->GetValue());
																//OpcUa_MemCpy(&(pResponse->Results[ii].Value), iSize, &aVariant, iSize);
															}
															break;
														}
														// release ressources
														if (pNumericRange)
															delete pNumericRange;
													}
													else
														OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "ProcessReadRequest>Not supported ArrayType\n");
												}
											}
											// update StatusCode
											if (uStatusPreChecked == OpcUa_BadOutOfRange)
												pResponse->Results[ii].StatusCode = uStatusFinal;
											else
											{
												// prise en compte de l'AccessLevel
												if ((pUAVariable->GetAccessLevel()&OpcUa_AccessLevels_CurrentRead) != OpcUa_AccessLevels_CurrentRead)
													pResponse->Results[ii].StatusCode = OpcUa_BadNotReadable;
												else
													pResponse->Results[ii].StatusCode = pUAVariable->GetValue()->GetStatusCode(); //OpcUa_Good;				
											}
											// Now will update the timestamps

											if (pRequest->IsSourceTimestampRequested())
											{
												pResponse->Results[ii].SourceTimestamp = pLocalDataValue->GetSourceTimeStamp();
												///////////////////////////////////////////////////////////////////////
												//std::string strTime;
												//hr=Utils::OpcUaDateTimeToString(pResponse->Results[ii].SourceTimestamp,&strTime);
												//OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"SourceRequested: SourceTimestamp: %s\n",strTime.c_str());
												///////////////////////////////////////////////////////////////////////
											}
											else
											{
												ZeroMemory(&(pResponse->Results[ii].SourceTimestamp), sizeof(OpcUa_DateTime));
												/////////////////////////////////////////////////////////////////////////
												//std::string strTime;
												//hr=Utils::OpcUaDateTimeToString(pResponse->Results[ii].ServerTimestamp,&strTime);
												//OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"SourceNotRequested: ServerTimestamp: %s\n",strTime.c_str());
												/////////////////////////////////////////////////////////////////////////
											}
											// ServerTimestampRequested
											if (pRequest->IsServerTimestampRequested())
											{
												OpcUa_DateTime dt = OpcUa_DateTime_UtcNow();
												pLocalDataValue->SetServerTimestamp(dt);
												pResponse->Results[ii].ServerTimestamp = pLocalDataValue->GetServerTimestamp();
												pResponse->Results[ii].ServerTimestamp = OpcUa_DateTime_UtcNow();
												/////////////////////////////////////////////////////////////////////////
												//std::string strTime;
												//hr=Utils::OpcUaDateTimeToString(pResponse->Results[ii].ServerTimestamp,&strTime);
												//OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"ServerRequested: ServerTimestamp: %s\n",strTime.c_str());
												/////////////////////////////////////////////////////////////////////////
											}
											else
											{
												ZeroMemory(&(pResponse->Results[ii].ServerTimestamp), sizeof(OpcUa_DateTime));
												/////////////////////////////////////////////////////////////////////////
												//std::string strTime;
												//hr=Utils::OpcUaDateTimeToString(pResponse->Results[ii].ServerTimestamp,&strTime);
												//OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"ServerNotRequested: ServerTimestamp: %s\n",strTime.c_str());
												/////////////////////////////////////////////////////////////////////////
											}
										}
									}
									if (nodeToRead->AttributeId == OpcUa_Attributes_AccessLevel)
									{
										pResponse->Results[ii].Value.Datatype = OpcUaType_Byte;
										pResponse->Results[ii].Value.ArrayType=0;
										pResponse->Results[ii].Value.Value.Byte = ((CUAVariable*)pUABase)->GetAccessLevel();
										//
										if (uStatusPreChecked==OpcUa_BadOutOfRange)
											pResponse->Results[ii].StatusCode=OpcUa_BadIndexRangeNoData; //uStatusPreChecked;	
										else
											pResponse->Results[ii].StatusCode=OpcUa_Good;		
									}		
									if (nodeToRead->AttributeId == OpcUa_Attributes_UserAccessLevel)
									{
										pResponse->Results[ii].Value.Datatype = OpcUaType_Byte;
										pResponse->Results[ii].Value.ArrayType=0;
										pResponse->Results[ii].Value.Value.Byte = ((CUAVariable*)pUABase)->GetUserAccessLevel();
										//
										if (uStatusPreChecked==OpcUa_BadOutOfRange)
											pResponse->Results[ii].StatusCode=OpcUa_BadIndexRangeNoData; //uStatusPreChecked;	
										else
											pResponse->Results[ii].StatusCode=OpcUa_Good;		

									}		
									if (nodeToRead->AttributeId == OpcUa_Attributes_Historizing)
									{
										pResponse->Results[ii].Value.Datatype = OpcUaType_Boolean;
										pResponse->Results[ii].Value.ArrayType=0;
										pResponse->Results[ii].Value.Value.Byte = ((CUAVariable*)pUABase)->GetUserAccessLevel();
										//
										if (uStatusPreChecked==OpcUa_BadOutOfRange)
											pResponse->Results[ii].StatusCode=OpcUa_BadIndexRangeNoData; //uStatusPreChecked;
										else
											pResponse->Results[ii].StatusCode=OpcUa_Good;			

									}		
									bOk=OpcUa_True;
								}
								else
									pResponse->Results[ii].ServerTimestamp = OpcUa_DateTime_UtcNow(); 
							}
							break;
							case OpcUa_NodeClass_View:
							{
								CUAView* pUAView=(CUAView*)pUABase;
								// par défaut on considère que le client nous demande un attribut invalide :(
								pResponse->Results[ii].StatusCode=OpcUa_BadAttributeIdInvalid;
								// Compliant timestamp handling... 
								ZeroMemory(&(pResponse->Results[ii].SourceTimestamp),sizeof(OpcUa_DateTime));
								if (pUAVariable)
								{
									CDataValue* pDataValue = pUAVariable->GetValue();
									if (pDataValue)
									{
										if (pRequest->IsServerTimestampRequested())
											pResponse->Results[ii].ServerTimestamp = pDataValue->GetServerTimestamp();
										else
											ZeroMemory(&(pResponse->Results[ii].ServerTimestamp), sizeof(OpcUa_DateTime));
									}
								}
								else
									pResponse->Results[ii].ServerTimestamp = OpcUa_DateTime_UtcNow();
								if (nodeToRead->AttributeId == OpcUa_Attributes_ContainsNoLoops)
								{
									pResponse->Results[ii].Value.ArrayType=0;
									pResponse->Results[ii].Value.Datatype=OpcUaType_Boolean;
									pResponse->Results[ii].Value.Value.Boolean=pUAView->ContainsNoLoops();
									if (uStatusPreChecked==OpcUa_BadOutOfRange)
										pResponse->Results[ii].StatusCode=uStatusPreChecked;	
									else
										pResponse->Results[ii].StatusCode=OpcUa_Good;		
								}	
								if (nodeToRead->AttributeId == OpcUa_Attributes_EventNotifier)
								{
									pResponse->Results[ii].Value.ArrayType=0;
									pResponse->Results[ii].Value.Datatype=OpcUaType_Boolean;
									pResponse->Results[ii].Value.Value.Boolean=pUAView->GetEventNotifier();
									if (uStatusPreChecked==OpcUa_BadOutOfRange)
										pResponse->Results[ii].StatusCode=uStatusPreChecked;	
									else
										pResponse->Results[ii].StatusCode=OpcUa_Good;			
								}							
								bOk=TRUE;
							}
							break;
							default:
								pResponse->Results[ii].StatusCode = OpcUa_BadNodeIdUnknown;
								break;
							}
							// Traitement de la partie commune			
							if (bOk)
							{
								// index Range ne peut être demandé que pour les Noeuds de type Variable et VariableType

								int iSize;
								switch(nodeToRead->AttributeId)
								{
								case OpcUa_Attributes_NodeId:
								{
									//iSize=sizeof(pUABase->GetNodeId());
									pResponse->Results[ii].Value.Value.NodeId=(OpcUa_NodeId*)OpcUa_Alloc(sizeof(OpcUa_NodeId));
									if (pResponse->Results[ii].Value.Value.NodeId)
									{
										OpcUa_NodeId_Initialize(pResponse->Results[ii].Value.Value.NodeId);
										pResponse->Results[ii].Value.Datatype = OpcUaType_NodeId;
										OpcUa_NodeId* pNodeId = pUABase->GetNodeId();
										OpcUa_NodeId_CopyTo(pNodeId, pResponse->Results[ii].Value.Value.NodeId);
										if (uStatusPreChecked == OpcUa_BadOutOfRange)
											pResponse->Results[ii].StatusCode = uStatusPreChecked;
										else
											pResponse->Results[ii].StatusCode = OpcUa_Good;
									}
									else
										pResponse->Results[ii].StatusCode = OpcUa_BadOutOfMemory;
									// Compliant timestamp handling... 
									ZeroMemory(&(pResponse->Results[ii].SourceTimestamp), sizeof(OpcUa_DateTime));
									if (pUAVariable)
									{
										CDataValue* pDataValue = pUAVariable->GetValue();
										if (pDataValue)
										{
											if (pRequest->IsServerTimestampRequested())
												pResponse->Results[ii].ServerTimestamp = pDataValue->GetServerTimestamp();
											else
												ZeroMemory(&(pResponse->Results[ii].ServerTimestamp), sizeof(OpcUa_DateTime));
										}
									}
									else
										pResponse->Results[ii].ServerTimestamp = OpcUa_DateTime_UtcNow();

								}
								break;
								case OpcUa_Attributes_NodeClass:
								{
									pResponse->Results[ii].Value.Datatype = OpcUaType_Int32;
									pResponse->Results[ii].Value.ArrayType=0;
									pResponse->Results[ii].Value.Value.Int32= pUABase->GetNodeClass();
									if (uStatusPreChecked==OpcUa_BadOutOfRange)
										pResponse->Results[ii].StatusCode=OpcUa_BadIndexRangeNoData; //uStatusPreChecked;	
									else
										pResponse->Results[ii].StatusCode=OpcUa_Good;
									// Compliant timestamp handling... 
									ZeroMemory(&(pResponse->Results[ii].SourceTimestamp),sizeof(OpcUa_DateTime));
									if (pUAVariable)
									{
										CDataValue* pDataValue = pUAVariable->GetValue();
										if (pDataValue)
										{
											if (pRequest->IsServerTimestampRequested())
												pResponse->Results[ii].ServerTimestamp = pDataValue->GetServerTimestamp();
											else
												ZeroMemory(&(pResponse->Results[ii].ServerTimestamp), sizeof(OpcUa_DateTime));
										}
									}
									else
										pResponse->Results[ii].ServerTimestamp = OpcUa_DateTime_UtcNow();
								}
								break;
								case OpcUa_Attributes_BrowseName:
								{
									pResponse->Results[ii].Value.Datatype = OpcUaType_QualifiedName;
									pResponse->Results[ii].Value.ArrayType=0;
									iSize=OpcUa_String_StrLen(&(pUABase->GetBrowseName()->Name)); 
									pResponse->Results[ii].Value.Value.QualifiedName=(OpcUa_QualifiedName*)OpcUa_Alloc(sizeof(OpcUa_QualifiedName));
									if (pResponse->Results[ii].Value.Value.QualifiedName)
									{
										OpcUa_QualifiedName_Initialize(pResponse->Results[ii].Value.Value.QualifiedName);
										OpcUa_QualifiedName* pQualifiedName = pUABase->GetBrowseName();
										OpcUa_QualifiedName_CopyTo(pQualifiedName, pResponse->Results[ii].Value.Value.QualifiedName);
									}
									if (uStatusPreChecked==OpcUa_BadOutOfRange)
										pResponse->Results[ii].StatusCode=OpcUa_BadIndexRangeNoData; //uStatusPreChecked;	
									else
										pResponse->Results[ii].StatusCode=OpcUa_Good;	
									// Compliant timestamp handling... 
									ZeroMemory(&(pResponse->Results[ii].SourceTimestamp),sizeof(OpcUa_DateTime));
									if (pUAVariable)
									{
										CDataValue* pDataValue = pUAVariable->GetValue();
										if (pDataValue)
										{
											if (pRequest->IsServerTimestampRequested())
												pResponse->Results[ii].ServerTimestamp = pDataValue->GetServerTimestamp();
											else
												ZeroMemory(&(pResponse->Results[ii].ServerTimestamp), sizeof(OpcUa_DateTime));
										}
									}
									else
										pResponse->Results[ii].ServerTimestamp = OpcUa_DateTime_UtcNow();
								}
								break;
								case OpcUa_Attributes_DisplayName:
								{
									pResponse->Results[ii].Value.Datatype = OpcUaType_LocalizedText;
									pResponse->Results[ii].Value.ArrayType=0;
									OpcUa_LocalizedText aLocalizedText=pUABase->GetDisplayName();
									//pResponse->Results[ii].Value.Value.LocalizedText=Utils::Copy(&aLocalizedText);
									pResponse->Results[ii].Value.Value.LocalizedText = (OpcUa_LocalizedText*)OpcUa_Alloc(sizeof(OpcUa_LocalizedText));
									OpcUa_LocalizedText_Initialize(pResponse->Results[ii].Value.Value.LocalizedText);
									OpcUa_LocalizedText_CopyTo(&aLocalizedText, pResponse->Results[ii].Value.Value.LocalizedText);
									if (uStatusPreChecked==OpcUa_BadOutOfRange)
										pResponse->Results[ii].StatusCode=OpcUa_BadIndexRangeNoData; //uStatusPreChecked;	
									else
										pResponse->Results[ii].StatusCode=OpcUa_Good;			
									// Compliant timestamp handling... 
									ZeroMemory(&(pResponse->Results[ii].SourceTimestamp),sizeof(OpcUa_DateTime));
									if (pUAVariable)
									{
										CDataValue* pDataValue = pUAVariable->GetValue();
										if (pDataValue)
										{
											if (pRequest->IsServerTimestampRequested())
												pResponse->Results[ii].ServerTimestamp = pDataValue->GetServerTimestamp();
											else
												ZeroMemory(&(pResponse->Results[ii].ServerTimestamp), sizeof(OpcUa_DateTime));
										}
									}
									else
										pResponse->Results[ii].ServerTimestamp = OpcUa_DateTime_UtcNow();
								}
								break;
								case OpcUa_Attributes_Description:
								{
									pResponse->Results[ii].Value.Datatype = OpcUaType_LocalizedText;
									pResponse->Results[ii].Value.ArrayType=0;
									OpcUa_LocalizedText aLocalizedText=pUABase->GetDescription();
									pResponse->Results[ii].Value.Value.LocalizedText = (OpcUa_LocalizedText*)OpcUa_Alloc(sizeof(OpcUa_LocalizedText));
									OpcUa_LocalizedText_Initialize(pResponse->Results[ii].Value.Value.LocalizedText);
									OpcUa_LocalizedText_CopyTo(&aLocalizedText, pResponse->Results[ii].Value.Value.LocalizedText);

									if (uStatusPreChecked==OpcUa_BadOutOfRange)
										pResponse->Results[ii].StatusCode=uStatusPreChecked;	
									else
										pResponse->Results[ii].StatusCode=OpcUa_Good;		
									// Compliant timestamp handling... 
									ZeroMemory(&(pResponse->Results[ii].SourceTimestamp),sizeof(OpcUa_DateTime));
									if (pUAVariable)
									{
										CDataValue* pDataValue = pUAVariable->GetValue();
										if (pDataValue)
										{
											if (pRequest->IsServerTimestampRequested())
												pResponse->Results[ii].ServerTimestamp = pDataValue->GetServerTimestamp();
											else
												ZeroMemory(&(pResponse->Results[ii].ServerTimestamp), sizeof(OpcUa_DateTime));
										}
									}
									else
										pResponse->Results[ii].ServerTimestamp = OpcUa_DateTime_UtcNow();
								}
								break;
								case OpcUa_Attributes_WriteMask:
								{
									pResponse->Results[ii].Value.Datatype = OpcUaType_UInt32;
									pResponse->Results[ii].Value.ArrayType=0;
									pResponse->Results[ii].Value.Value.UInt32 = pUABase->GetWriteMask();
									if (uStatusPreChecked==OpcUa_BadOutOfRange)
										pResponse->Results[ii].StatusCode=uStatusPreChecked;	
									else
										pResponse->Results[ii].StatusCode=OpcUa_Good;					
									// Compliant timestamp handling... 
									ZeroMemory(&(pResponse->Results[ii].SourceTimestamp),sizeof(OpcUa_DateTime));
									if (pUAVariable)
									{
										CDataValue* pDataValue = pUAVariable->GetValue();
										if (pDataValue)
										{
											if (pRequest->IsServerTimestampRequested())
												pResponse->Results[ii].ServerTimestamp = pDataValue->GetServerTimestamp();
											else
												ZeroMemory(&(pResponse->Results[ii].ServerTimestamp), sizeof(OpcUa_DateTime));
										}
									}
									else
										pResponse->Results[ii].ServerTimestamp = OpcUa_DateTime_UtcNow();
								}
								break;
								case OpcUa_Attributes_UserWriteMask:
								{
									pResponse->Results[ii].Value.Datatype = OpcUaType_UInt32;
									pResponse->Results[ii].Value.ArrayType=0;
									pResponse->Results[ii].Value.Value.UInt32 = pUABase->GetUserWriteMask();
									if (uStatusPreChecked==OpcUa_BadOutOfRange)
										pResponse->Results[ii].StatusCode=uStatusPreChecked;	
									else
										pResponse->Results[ii].StatusCode=OpcUa_Good;		
									// Compliant timestamp handling... 
									ZeroMemory(&(pResponse->Results[ii].SourceTimestamp),sizeof(OpcUa_DateTime));
									if (pUAVariable)
									{
										CDataValue* pDataValue = pUAVariable->GetValue();
										if (pDataValue)
										{
											if (pRequest->IsServerTimestampRequested())
												pResponse->Results[ii].ServerTimestamp = pDataValue->GetServerTimestamp();
											else
												ZeroMemory(&(pResponse->Results[ii].ServerTimestamp), sizeof(OpcUa_DateTime));
										}
									}
									else
										pResponse->Results[ii].ServerTimestamp = OpcUa_DateTime_UtcNow();
								}
								break;
								default:
									break;
								}
							}
							else
								OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"CServerApplication::ProcessReadRequest: Cannot process the common part\n");
						}
					}
					//OpcUa_String_Clear(&xmlDataEncoding);
				}
				else
				{
					pResponse->Results[ii].StatusCode=OpcUa_BadTimestampsToReturnInvalid;
					pResponse->ResponseHeader.ServiceResult=OpcUa_BadTimestampsToReturnInvalid;
				}
			}

	

			if (!IsActive())			
			{
				pResponse->Results[ii].StatusCode=OpcUa_BadSessionNotActivated;
				pResponse->ResponseHeader.ServiceResult=OpcUa_BadSessionNotActivated;
				pResponse->ResponseHeader.Timestamp=OpcUa_DateTime_UtcNow();
				uStatus=OpcUa_BadSessionNotActivated;
				m_bRunSessionTimeoutThread=OpcUa_False; // On force la session en timeout afin que la thread CServerApplication::SessionsTimeoutThread l'arrête. 
													   // Cela conformément a la demande du CTT dans SessionBase err-004.js
			}		
		}
	}
	else
		uStatus = OpcUa_BadInternalError;
	return uStatus;
}

// Queue Call Message
OpcUa_StatusCode CSessionServer::QueueCallMessage(CQueuedCallMessage* pCallMessage)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_Mutex_Lock(m_hCallMutex);
	// add to queue.
	m_pCallMessages->push_back(pCallMessage);
	OpcUa_Mutex_Unlock(m_hCallMutex);
	return uStatus;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Processes a queued call request. </summary>
///
/// <remarks>	Michel, 29/07/2016. </remarks>
///
/// <param name="pCallMessage">	[in,out] If non-null, message describing the call. </param>
/// <param name="bAbort">	   	The abort. </param>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode CSessionServer::ProcessQueuedCallRequest(CQueuedCallMessage* pCallMessage, OpcUa_Boolean bAbort)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	if (bAbort)
	{
		uStatus = pCallMessage->CancelSendResponse();
	}
	else
	{
		// Create a context to use for sending a response.
		uStatus = pCallMessage->BeginSendResponse();
		if (uStatus != OpcUa_Good)
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "CSessionServer::ProcessQueuedCallRequest>Stack could not initialize response.\n");
		else
		{
			// Réalise la lecture dans la cache du serveur
			pCallMessage->FillResponseHeader();
			uStatus = ProcessCallRequest(pCallMessage);

			// Send the response to the client.
			uStatus = pCallMessage->EndSendResponse();
			if (OpcUa_IsBad(uStatus))
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "CSessionServer::ProcessQueuedCallRequest>Could not send response to client. Status 0x%08X!\r\n", uStatus);
			// Mark it for deletion
			pCallMessage->SetDeleted(OpcUa_True);

		}
	}
	pCallMessage->EncodeableObjectDelete();
	return uStatus;
}
///-------------------------------------------------------------------------------------------------
/// <summary>	Process the call request described by pCallMessage. </summary>
///
/// <remarks>	Michel, 02/02/2016. </remarks>
///
/// <param name="pCallMessage">	[in,out] If non-null, message describing the call. </param>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode CSessionServer::ProcessCallRequest(CQueuedCallMessage* pCallMessage)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	CUAInformationModel* pInformationModel = CServerApplication::m_pTheAddressSpace;
	// List well known methods
	OpcUa_NodeId ConditionRefresh;
	OpcUa_NodeId_Initialize(&ConditionRefresh);
	ConditionRefresh.IdentifierType = OpcUa_IdentifierType_Numeric;
	ConditionRefresh.Identifier.Numeric = 3875;

	OpcUa_NodeId Acknowledge;
	OpcUa_NodeId_Initialize(&Acknowledge);
	Acknowledge.IdentifierType = OpcUa_IdentifierType_Numeric;
	Acknowledge.Identifier.Numeric = 9111;

	OpcUa_NodeId Confirm;
	OpcUa_NodeId_Initialize(&Confirm);
	Confirm.IdentifierType = OpcUa_IdentifierType_Numeric;
	Confirm.Identifier.Numeric = 9113;

	OpcUa_NodeId AddComment;
	OpcUa_NodeId_Initialize(&AddComment);
	AddComment.IdentifierType = OpcUa_IdentifierType_Numeric;
	AddComment.Identifier.Numeric = 9029;

	OpcUa_NodeId Enable;
	OpcUa_NodeId_Initialize(&Enable);
	Enable.IdentifierType = OpcUa_IdentifierType_Numeric;
	Enable.Identifier.Numeric = 9027;

	OpcUa_NodeId Disable;
	OpcUa_NodeId_Initialize(&Disable);
	Disable.IdentifierType = OpcUa_IdentifierType_Numeric;
	Disable.Identifier.Numeric = 9028;

	OpcUa_NodeId GetMonitoredItems;
	OpcUa_NodeId_Initialize(&GetMonitoredItems);
	GetMonitoredItems.IdentifierType = OpcUa_IdentifierType_Numeric;
	GetMonitoredItems.Identifier.Numeric = OpcUaId_Server_GetMonitoredItems; // 11492
	// 
	// End of well known method list
	OpcUa_Int32 iNbM2C = pCallMessage->GetNoOfMethodsToCall();
	// Now let's process asynchronously the Call
	OpcUa_CallResponse* pResponse = pCallMessage->GetInternalCallResponse();
	//OpcUa_CallRequest* pRequest = pCallMessage->GetInternalCallRequest();
	if (iNbM2C == 0)
		pResponse->ResponseHeader.ServiceResult = OpcUa_BadNothingToDo;
	else
	{
		pResponse->NoOfResults = iNbM2C;
		pResponse->Results = (OpcUa_CallMethodResult*)OpcUa_Alloc(sizeof(OpcUa_CallMethodResult)*iNbM2C);
		OpcUa_CallMethodRequest* pCallMethodRequest = pCallMessage->GetMethodsToCall();
		for (OpcUa_Int32 ii = 0; ii < iNbM2C; ii++)
		{
			OpcUa_CallMethodResult_Initialize(&(pResponse->Results[ii]));
			// Let's process this call
			// Verify if the method is a well known method
			CUAMethod* pUAMethodCalled = OpcUa_Null;
			uStatus = pInformationModel->GetNodeIdFromMethodList(pCallMethodRequest[ii].MethodId, &pUAMethodCalled);
			/////////////////////////////////////////////////////////////////////////////
			// Process a ConditionRefresh
			if (OpcUa_NodeId_Compare(&(pCallMethodRequest[ii].MethodId), &ConditionRefresh) == 0)
			{
				// Let's proceed a ConditionRefresh
				// First post the appropriate event RefreshStartEvent		
				if (pCallMethodRequest[ii].NoOfInputArguments > 0)
				{
					pResponse->Results[ii].InputArgumentResults = (OpcUa_StatusCode*)OpcUa_Alloc(sizeof(OpcUa_StatusCode)*pCallMethodRequest[ii].NoOfInputArguments);
					for (OpcUa_Int32 iii = 0; iii < pCallMethodRequest[ii].NoOfInputArguments; iii++)
					{
						OpcUa_Variant InputArgument = pCallMethodRequest[ii].InputArguments[iii];
						if (InputArgument.Datatype == OpcUaType_UInt32)
						{
							CSubscriptionServer* pSubscription = OpcUa_Null;
							uStatus = FindSubscription(InputArgument.Value.UInt32, &pSubscription);
							if (uStatus == OpcUa_Good)
							{
								// the ObjectId shall be the well known ObjectId of the ConditionType Object
								//pCallMethodRequest[ii].ObjectId;
								CMonitoredItemServerList* pMonitoredItemServerList = pSubscription->GetMonitoredItemList();
								for (OpcUa_UInt32 j = 0; j < pMonitoredItemServerList->size(); j++)
								{
									CMonitoredItemServer* pMonitoredItem = pMonitoredItemServerList->at(j);
									OpcUa_Boolean bNotifier = OpcUa_False;
									CUABase* pUABase = pMonitoredItem->GetUABase();
									if (pUABase)
									{
										switch (pUABase->GetNodeClass())
										{
										case OpcUa_NodeClass_Object:
										{
											CUAObject* pObject = (CUAObject*)pUABase;
											bNotifier = pObject->GetEventNotifier();
										}
										break;
										case OpcUa_NodeClass_View:
										{
											CUAObject* pObject = (CUAObject*)pUABase;
											bNotifier = pObject->GetEventNotifier();
										}
										break;
										default:
											break;
										}
									}
									if (bNotifier)
									{
										// construct the RefreshStartEvent
										OpcUa_NodeId RefreshStartEvent;
										OpcUa_NodeId_Initialize(&RefreshStartEvent);
										RefreshStartEvent.IdentifierType = OpcUa_IdentifierType_Numeric;
										RefreshStartEvent.Identifier.Numeric = 2787;
										// Search this EventType in the AddressSpace
										CUAObjectType* pUAObjectType = OpcUa_Null;
										if (pInformationModel->GetNodeIdFromObjectTypeList(RefreshStartEvent, &pUAObjectType) == OpcUa_Good)
										{
											// This is a virtual EventDefinition
											CEventDefinition* pRefreshStartEventDefinition = new CEventDefinition(OpcUa_Null, OpcUa_Null, OpcUa_Null, pUAObjectType);
											CUAEventNotificationList* pUAEventNotificationList = new CUAEventNotificationList();
											if (pUAEventNotificationList)
											{
												// Set the associated MonitoredItem
												pUAEventNotificationList->SetMonitoredItem(pMonitoredItem);
												// Set the associate EventDefinition
												pUAEventNotificationList->SetEventDefinition(pRefreshStartEventDefinition);
												// Set the NotificationType
												pUAEventNotificationList->SetNotificationType(NOTIFICATION_MESSAGE_EVENT);
												uStatus = pMonitoredItem->InitializeEventsFieldsEx(pRefreshStartEventDefinition, &pUAEventNotificationList);
												if (uStatus == OpcUa_Good)
												{
													pSubscription->AddEventNotificationList(pUAEventNotificationList);
												}
											}
										}

										// TODO Notify the Retained Conditions. 
										uStatus = pSubscription->ConditionRefresh();
										// now send a RefreshEndEventType
										OpcUa_NodeId RefreshEndEvent;
										OpcUa_NodeId_Initialize(&RefreshEndEvent);
										RefreshEndEvent.IdentifierType = OpcUa_IdentifierType_Numeric;
										RefreshEndEvent.Identifier.Numeric = 2788;
										// Search this EventType in the AddressSpace
										pUAObjectType = OpcUa_Null;
										if (pInformationModel->GetNodeIdFromObjectTypeList(RefreshEndEvent, &pUAObjectType) == OpcUa_Good)
										{
											// This is a virtual EventDefinition
											CEventDefinition* pRefreshEndEventDefinition = new CEventDefinition(OpcUa_Null, OpcUa_Null, OpcUa_Null, pUAObjectType);
											CUAEventNotificationList* pUAEventNotificationList = new CUAEventNotificationList();
											if (pUAEventNotificationList)
											{
												// Set the associated MonitoredItem
												pUAEventNotificationList->SetMonitoredItem(pMonitoredItem);
												// Set the associate EventDefinition
												pUAEventNotificationList->SetEventDefinition(pRefreshEndEventDefinition);
												// Set the NotificationType
												pUAEventNotificationList->SetNotificationType(NOTIFICATION_MESSAGE_EVENT);

												uStatus = pMonitoredItem->InitializeEventsFieldsEx(pRefreshEndEventDefinition, &pUAEventNotificationList);
												if (uStatus == OpcUa_Good)
												{
													pSubscription->AddEventNotificationList(pUAEventNotificationList);
												}
											}
										}
									}
								}
								pResponse->Results[ii].InputArgumentResults[iii] = OpcUa_Good;
							}
							else
								pResponse->Results[ii].InputArgumentResults[iii] = OpcUa_BadSubscriptionIdInvalid;
						}
						else
							pResponse->Results[ii].InputArgumentResults[iii] = OpcUa_BadInvalidArgument;
					}
				}
			}
			else
			{
				// Process a Acknowledge. The Acknowledgement will associated with the Event identify by the EventId
				if (OpcUa_NodeId_Compare(&(pCallMethodRequest[ii].MethodId), &Acknowledge) == 0)
				{
					// Let's proceed a Acknowledge

					if (pCallMethodRequest[ii].NoOfInputArguments == 2)
					{
						pResponse->Results[ii].InputArgumentResults = (OpcUa_StatusCode*)OpcUa_Alloc(sizeof(OpcUa_StatusCode)*pCallMethodRequest[ii].NoOfInputArguments);

						OpcUa_Variant InputArgumentEventId = pCallMethodRequest[ii].InputArguments[0];
						OpcUa_Variant InputArgumentComment = pCallMethodRequest[ii].InputArguments[1];
						// Parameter one
						if (InputArgumentEventId.Datatype == OpcUaType_ByteString)
						{
							pResponse->Results[ii].InputArgumentResults[0] = OpcUa_Good;
							// Let's try to retrieve the CUAEventNotificationList based on this EventId (ByteString)
							OpcUa_ByteString EventId = InputArgumentEventId.Value.ByteString;

							CUAEventNotificationList* pUAEventNotificationList = OpcUa_Null;
							CEventDefinition* pEventDefinition = OpcUa_Null;
							for (OpcUa_UInt32 iiii = 0; iiii < m_SubscriptionList.size(); iiii++)
							{
								CSubscriptionServer* pSubscription = m_SubscriptionList.at(iiii);
								OpcUa_Mutex aMutex = pSubscription->GetEventNotificationListListMutex();
								OpcUa_Mutex_Lock(aMutex);
								if (pSubscription->GetEventNotificationListFromEventId(&EventId, &pUAEventNotificationList) == OpcUa_Good)
								{
									if (pUAEventNotificationList)
									{
										if (!pUAEventNotificationList->IsAlarmAcked())
										{
											// Save the related CEventDefinition
											pEventDefinition = pUAEventNotificationList->GetEventDefinition();
											if (pEventDefinition)
											{
												// Each EventId is unique in the context of the server.
												// But when we aknowledge we need to information all the client who received the notification that the Event was acknowlege
												// so if we found a CUAEventNotificationList in a subscription  where this EventId is used. 
												// We will call CServerApplicaton::AckApplicationEvents in order to realize the real Acknowledgement
												// 
												//break;
											}
										}
									}
								}
								OpcUa_Mutex_Unlock(aMutex);
							}
							// Let's call the method that will make the acknowledgment
							if (pEventDefinition)
							{
								OpcUa_LocalizedText comment;
								OpcUa_LocalizedText_Initialize(&comment);
								// Parameter two
								if (InputArgumentComment.Datatype == OpcUaType_LocalizedText)
								{
									pResponse->Results[ii].InputArgumentResults[1] = OpcUa_Good;
									OpcUa_LocalizedText_CopyTo(InputArgumentComment.Value.LocalizedText, &comment);
								}
								m_pApplication->AckCommentConfirmApplicationEvents(pEventDefinition, comment, UAAddressSpace::Acknowledge);
								OpcUa_LocalizedText_Clear(&comment);
							}
						}
					}
					else
						pResponse->Results[ii].StatusCode = OpcUa_BadInvalidArgument;
				}
				else
				{
					// Process a Confirm. The Confirmation will associated with the Event identify by the EventId
					if (OpcUa_NodeId_Compare(&(pCallMethodRequest[ii].MethodId), &Confirm) == 0)
					{
						// Let's proceed a Confirm
						if (pCallMethodRequest[ii].NoOfInputArguments == 2)
						{
							pResponse->Results[ii].InputArgumentResults = (OpcUa_StatusCode*)OpcUa_Alloc(sizeof(OpcUa_StatusCode)*pCallMethodRequest[ii].NoOfInputArguments);
							OpcUa_Variant InputArgumentEventId = pCallMethodRequest[ii].InputArguments[0];
							OpcUa_Variant InputArgumentComment = pCallMethodRequest[ii].InputArguments[1];
							// Parameter one
							if (InputArgumentEventId.Datatype == OpcUaType_ByteString)
							{
								pResponse->Results[ii].InputArgumentResults[0] = OpcUa_Good;
								// Let's try to retrieve the CUAEventNotificationList based on this EventId (ByteString)
								OpcUa_ByteString EventId = InputArgumentEventId.Value.ByteString;

								CUAEventNotificationList* pUAEventNotificationList = OpcUa_Null;
								CEventDefinition* pEventDefinition = OpcUa_Null;
								for (OpcUa_UInt32 iiii = 0; iiii < m_SubscriptionList.size(); iiii++)
								{
									CSubscriptionServer* pSubscription = m_SubscriptionList.at(iiii);
									OpcUa_Mutex aMutex = pSubscription->GetEventNotificationListListMutex();
									OpcUa_Mutex_Lock(aMutex);
									if (pSubscription->GetEventNotificationListFromEventId(&EventId, &pUAEventNotificationList) == OpcUa_Good)
									{
										if (pUAEventNotificationList)
										{
											// Save the related CEventDefinition
											pEventDefinition = pUAEventNotificationList->GetEventDefinition();
										}
									}
									OpcUa_Mutex_Unlock(aMutex);
								}
								if (pEventDefinition)
								{
									OpcUa_LocalizedText comment;
									OpcUa_LocalizedText_Initialize(&comment);
									// Parameter two
									if (InputArgumentComment.Datatype == OpcUaType_LocalizedText)
									{
										pResponse->Results[ii].InputArgumentResults[1] = OpcUa_Good;
										OpcUa_LocalizedText_CopyTo(InputArgumentComment.Value.LocalizedText, &comment);
									}
									m_pApplication->AckCommentConfirmApplicationEvents(pEventDefinition, comment, UAAddressSpace::Confirm);
									OpcUa_LocalizedText_Clear(&comment);
								}
							}
						}
						else
							pResponse->Results[ii].StatusCode = OpcUa_BadInvalidArgument;
					}
					else
					{
						// Process a AddComment. The comment will associated with the Event identify by the EventId
						if (OpcUa_NodeId_Compare(&(pCallMethodRequest[ii].MethodId), &AddComment) == 0)
						{
							// Let's proceed a AddComment
							if (pCallMethodRequest[ii].NoOfInputArguments == 2)
							{
								pResponse->Results[ii].InputArgumentResults = (OpcUa_StatusCode*)OpcUa_Alloc(sizeof(OpcUa_StatusCode)*pCallMethodRequest[ii].NoOfInputArguments);
								OpcUa_Variant InputArgumentEventId = pCallMethodRequest[ii].InputArguments[0];
								OpcUa_Variant InputArgumentComment = pCallMethodRequest[ii].InputArguments[1];
								// Parameter one
								if (InputArgumentEventId.Datatype == OpcUaType_ByteString)
								{
									pResponse->Results[ii].InputArgumentResults[0] = OpcUa_Good;
									// Let's try to retrieve the CUAEventNotificationList based on this EventId (ByteString)
									OpcUa_ByteString EventId = InputArgumentEventId.Value.ByteString;

									CUAEventNotificationList* pUAEventNotificationList = OpcUa_Null;
									CEventDefinition* pEventDefinition = OpcUa_Null;
									for (OpcUa_UInt32 iiii = 0; iiii < m_SubscriptionList.size(); iiii++)
									{
										CSubscriptionServer* pSubscription = m_SubscriptionList.at(iiii);
										OpcUa_Mutex aMutex = pSubscription->GetEventNotificationListListMutex();
										OpcUa_Mutex_Lock(aMutex);
										if (pSubscription->GetEventNotificationListFromEventId(&EventId, &pUAEventNotificationList) == OpcUa_Good)
										{
											if (pUAEventNotificationList)
											{
												// Save the related CEventDefinition
												pEventDefinition = pUAEventNotificationList->GetEventDefinition();
											}
										}
										OpcUa_Mutex_Unlock(aMutex);
									}
									if (pEventDefinition)
									{
										OpcUa_LocalizedText comment;
										OpcUa_LocalizedText_Initialize(&comment);
										// Parameter two
										if (InputArgumentComment.Datatype == OpcUaType_LocalizedText)
										{
											pResponse->Results[ii].InputArgumentResults[1] = OpcUa_Good;
											OpcUa_LocalizedText_CopyTo(InputArgumentComment.Value.LocalizedText, &comment);
										}
										// We will add the comment
										m_pApplication->AckCommentConfirmApplicationEvents(pEventDefinition, comment, UAAddressSpace::Comment);
										OpcUa_LocalizedText_Clear(&comment);
									}
								}
							}
							else
								pResponse->Results[ii].StatusCode = OpcUa_BadInvalidArgument;
						}
						else
						{
							if (OpcUa_NodeId_Compare(&(pCallMethodRequest[ii].MethodId), &GetMonitoredItems) == 0)
							{
								// The first argument is SubscriptionId
								if (pCallMethodRequest[ii].InputArguments)
								{
									OpcUa_Variant InputArgumentEventId = pCallMethodRequest[ii].InputArguments[0]; // UInt32
									if (InputArgumentEventId.Datatype == OpcUaType_UInt32)
									{
										CSubscriptionServer* pSubscription = OpcUa_Null;
										pResponse->Results[ii].StatusCode = FindSubscription(InputArgumentEventId.Value.UInt32, &pSubscription);
										if (pResponse->Results[ii].StatusCode == OpcUa_Good)
										{
											// Then 2 output argument 
											OpcUa_Mutex aMutex = pSubscription->GetMonitoredItemListMutex();
											OpcUa_Mutex_Lock(aMutex);
											CMonitoredItemServerList* pMonitoredItemserverList = pSubscription->GetMonitoredItemList();
											pResponse->Results[ii].NoOfOutputArguments = 2;
											pResponse->Results[ii].OutputArguments = (OpcUa_Variant*)OpcUa_Alloc(sizeof(OpcUa_Variant) * 2);
											// ServerHandles
											OpcUa_Variant_Initialize(&(pResponse->Results[ii].OutputArguments[0]));
											pResponse->Results[ii].OutputArguments[0].Value.Array.Length = pMonitoredItemserverList->size();
											pResponse->Results[ii].OutputArguments[0].Value.Array.Value.UInt32Array = (OpcUa_UInt32*)OpcUa_Alloc(sizeof(OpcUa_UInt32)*pMonitoredItemserverList->size());
											// ClientHandles
											OpcUa_Variant_Initialize(&(pResponse->Results[ii].OutputArguments[1]));
											pResponse->Results[ii].OutputArguments[1].Value.Array.Length = pMonitoredItemserverList->size();
											pResponse->Results[ii].OutputArguments[1].Value.Array.Value.UInt32Array = (OpcUa_UInt32*)OpcUa_Alloc(sizeof(OpcUa_UInt32)*pMonitoredItemserverList->size());
											for (OpcUa_UInt32 i = 0; i < pMonitoredItemserverList->size(); i++)
											{
												CMonitoredItemServer* pMonitoredItemServer = pMonitoredItemserverList->at(i);
												// ServerHandles
												pResponse->Results[ii].OutputArguments[0].Value.Array.Value.UInt32Array[i] = (OpcUa_UInt32)pMonitoredItemServer;
												// ClientHandles
												pResponse->Results[ii].OutputArguments[0].Value.Array.Value.UInt32Array[i] = pMonitoredItemServer->GetClientHandle();
											}
											OpcUa_Mutex_Unlock(aMutex);
										}
										else
											;
									}
									else
										pResponse->Results[ii].StatusCode = OpcUa_BadInvalidArgument;
								}
								else
									pResponse->Results[ii].StatusCode = OpcUa_BadInvalidArgument;
							}
							else
							{
								// An other method or an unknow method
								// Let's try to find the lua script associated with this method							
								uStatus = CallLuaMethod(pUAMethodCalled, &pCallMethodRequest[ii], &pResponse);
							}
						}
					}
				}
			}
			if (pUAMethodCalled)
			{
				OpcUa_QualifiedName* pMethodBrowseName = pUAMethodCalled->GetBrowseName();
				OpcUa_String aString;
				// Process a Enable
				OpcUa_String_Initialize(&aString);
				OpcUa_String_AttachCopy(&aString, "Enable");
				if (OpcUa_String_Compare(&(pMethodBrowseName->Name), &aString) == 0)
				{
					// no parameter here. The ObjectId contaisn the NodeId of the Instance to Enable
					CUAMethod* pUAMethod = pUAMethodCalled;
					CEventDefinition* pEventDefinition = OpcUa_Null;
					//if (pInformationModel->GetNodeIdFromMethodList(pCallMethodRequest[ii].ObjectId, &pUAMethod) == OpcUa_Good)
					if (pUAMethod)
					{
						// Change EnableState and EnableState Id
						// First look for the CEventDefiniton associated with the pUABase
						CEventsEngine*	pEventsEngine = m_pApplication->GetEventsEngine();
						if (pEventsEngine)
						{
							if (pEventsEngine->SearchForEventDefinitionOnMethod(pUAMethod, &pEventDefinition) == OpcUa_Good)
							{
								// We have the CEventDefinition. We can Enable it.
								CUAVariable* pEnabledState = pEventDefinition->GetRelativeEnabledStateUAVariable();
								if (pEnabledState)
								{
									OpcUa_Variant aVariant;
									OpcUa_Variant_Initialize(&aVariant);
									aVariant.Datatype = OpcUaType_LocalizedText;
									aVariant.Value.LocalizedText = (OpcUa_LocalizedText*)OpcUa_Alloc(sizeof(OpcUa_LocalizedText));
									OpcUa_LocalizedText_Initialize(aVariant.Value.LocalizedText);
									OpcUa_String_AttachCopy(&(aVariant.Value.LocalizedText->Locale), "en-us");
									OpcUa_String_AttachCopy(&(aVariant.Value.LocalizedText->Text), "Enabled");
									pEnabledState->SetValue(aVariant);
									OpcUa_LocalizedText_Clear(aVariant.Value.LocalizedText);
									//OpcUa_Free(aVariant.Value.LocalizedText);
									OpcUa_Variant_Clear(&aVariant);
								}
								CUAVariable* pEnabledStateId = pEventDefinition->GetRelativeEnabledStateIdUAVariable();
								if (pEnabledStateId)
								{
									OpcUa_Variant aVariant;
									OpcUa_Variant_Initialize(&aVariant);
									aVariant.Datatype = OpcUaType_Boolean;
									aVariant.Value.Boolean = OpcUa_True;
									pEnabledStateId->SetValue(aVariant);
									OpcUa_Variant_Clear(&aVariant);
									pEventDefinition->Enable(aVariant.Value.Boolean);
								}
							}
						}
					}
				}
				OpcUa_String_Clear(&aString);
				// Process a Disable
				OpcUa_String_Initialize(&aString);
				OpcUa_String_AttachCopy(&aString, "Disable");
				if (OpcUa_String_Compare(&(pMethodBrowseName->Name), &aString) == 0)
				{
					// no parameter here. The ObjectId contaisn the NodeId of the Instance to Disable
					CUAMethod* pUAMethod = pUAMethodCalled;
					CEventDefinition* pEventDefinition = OpcUa_Null;
					//if (pInformationModel->GetNodeIdFromMethodList(pCallMethodRequest[ii].ObjectId, &pUAMethod) == OpcUa_Good)
					if (pUAMethod)
					{
						// Change EnableState and EnableState Id
						// First look for the CEventDefiniton associated with the pUABase
						CEventsEngine*	pEventsEngine = m_pApplication->GetEventsEngine();
						if (pEventsEngine)
						{
							if (pEventsEngine->SearchForEventDefinitionOnMethod(pUAMethod, &pEventDefinition) == OpcUa_Good)
							{
								// We have the CEventDefinition. We can Enable it.
								CUAVariable* pEnabledState = pEventDefinition->GetRelativeEnabledStateUAVariable();
								if (pEnabledState)
								{
									OpcUa_Variant aVariant;
									OpcUa_Variant_Initialize(&aVariant);
									aVariant.Datatype = OpcUaType_LocalizedText;
									aVariant.Value.LocalizedText = (OpcUa_LocalizedText*)OpcUa_Alloc(sizeof(OpcUa_LocalizedText));
									OpcUa_LocalizedText_Initialize(aVariant.Value.LocalizedText);
									OpcUa_String_AttachCopy(&(aVariant.Value.LocalizedText->Locale), "en-us");
									OpcUa_String_AttachCopy(&(aVariant.Value.LocalizedText->Text), "Disabled");
									pEnabledState->SetValue(aVariant);
									OpcUa_LocalizedText_Clear(aVariant.Value.LocalizedText);
									//OpcUa_Free(aVariant.Value.LocalizedText);
									OpcUa_Variant_Clear(&aVariant);
								}
								CUAVariable* pEnabledStateId = pEventDefinition->GetRelativeEnabledStateIdUAVariable();
								if (pEnabledStateId)
								{
									OpcUa_Variant aVariant;
									OpcUa_Variant_Initialize(&aVariant);
									aVariant.Datatype = OpcUaType_Boolean;
									aVariant.Value.Boolean = OpcUa_False;
									pEnabledStateId->SetValue(aVariant);
									OpcUa_Variant_Clear(&aVariant);
									pEventDefinition->Enable(aVariant.Value.Boolean);
								}
							}
						}
					}
				}
				OpcUa_String_Clear(&aString);
			}
			else
			{
				pResponse->Results[ii].StatusCode = OpcUa_BadNothingToDo;
				pResponse->ResponseHeader.ServiceResult = OpcUa_Good;
			}
			// Check active state
			if (!IsActive())
			{
				pResponse->Results[ii].StatusCode = OpcUa_BadSessionNotActivated;
				pResponse->ResponseHeader.ServiceResult = OpcUa_BadSessionNotActivated;
				pResponse->ResponseHeader.Timestamp = OpcUa_DateTime_UtcNow();
				uStatus = OpcUa_BadSessionNotActivated;
				m_bRunSessionTimeoutThread = OpcUa_False; // On force la session en timeout afin que la thread CServerApplication::SessionsTimeoutThread l'arrête. 
				// Cela conformément a la demande du CTT dans SessionBase err-004.js
			}
		}
	}

	OpcUa_NodeId_Clear(&GetMonitoredItems);
	OpcUa_NodeId_Clear(&Disable);
	OpcUa_NodeId_Clear(&Enable);
	OpcUa_NodeId_Clear(&AddComment);
	OpcUa_NodeId_Clear(&Confirm);
	OpcUa_NodeId_Clear(&Acknowledge);
	OpcUa_NodeId_Clear(&ConditionRefresh);
	return uStatus;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Call lua method. </summary>
///
/// <remarks>	Michel, 02/02/2016. </remarks>
///
/// <param name="pUAMethodCalled">   	[in,out] If non-null, the UA method called. </param>
/// <param name="pCallMethodRequest">	[in,out] If non-null, the call method request. </param>
/// <param name="ppResponse">		 	[in,out] If non-null, the response. </param>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode CSessionServer::CallLuaMethod(CUAMethod* pUAMethodCalled, OpcUa_CallMethodRequest* pCallMethodRequest, OpcUa_CallResponse** ppResponse)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	//OpcUa_CharA* pszFileName = OpcUa_Null;
	if (pUAMethodCalled)
	{
		OpcUa_QualifiedName* pBrowseName= pUAMethodCalled->GetBrowseName();
		if (pBrowseName)
		{
			CUAInformationModel* pInformationModel = CServerApplication::m_pTheAddressSpace;
			OpcUa_CharA* pszLuaMethodName=OpcUa_String_GetRawString(&(pBrowseName->Name));
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_INFO, "The OpenOpcUaCoreServer will run the LUA script %s", pszLuaMethodName);
			
			CLuaVirtualMachine* pVm=pInformationModel->GetLuaVirtualMachine();
			COpenOpcUaScript* pMs = pInformationModel->GetOpenOpcUaScript();
			// 
			int iTopS = lua_gettop((lua_State *)pVm->GetLuaState());

			if (pMs->ScriptHasFunction(pszLuaMethodName))
			{
				if (pMs->SelectScriptFunction(pszLuaMethodName))
				{
					//pUAMethodCalled;
					for (OpcUa_Int32 iii = 0; iii < pCallMethodRequest->NoOfInputArguments; iii++)
					{
						OpcUa_Variant InputArgument = pCallMethodRequest->InputArguments[iii];
						switch (InputArgument.Datatype)
						{
						case OpcUaType_Boolean:
							pMs->AddParam((lua_Number)InputArgument.Value.Boolean);
							break;
						case OpcUaType_Byte:
							pMs->AddParam((lua_Number)InputArgument.Value.Byte);
							break;
						case OpcUaType_DateTime:
						{
							// Convert DataTime to string and call AddParam with the converted char*
							char* szDataTime = OpcUa_Null;
							pMs->AddParam(szDataTime);
						}
						break;
						case OpcUaType_Double:
							pMs->AddParam((lua_Number)InputArgument.Value.Double);
							break;
						case OpcUaType_Float:
							pMs->AddParam((lua_Number)InputArgument.Value.Float);
							break;
						case OpcUaType_UInt16:
							pMs->AddParam((lua_Number)InputArgument.Value.UInt16);
							break;
						case OpcUaType_UInt32:
							pMs->AddParam((lua_Number)InputArgument.Value.UInt32);
							break;
						case OpcUaType_Int16:
							pMs->AddParam((lua_Number)InputArgument.Value.Int16);
							break;
						case OpcUaType_Int32:
							pMs->AddParam((lua_Number)InputArgument.Value.Int32);
							break;
						case OpcUaType_LocalizedText:
							pMs->AddParam((lua_Number)InputArgument.Value.Byte);
							break;
						case OpcUaType_NodeId:
						{
							char* szParamNodeId = OpcUa_Null;
							Utils::NodeId2String(InputArgument.Value.NodeId, &szParamNodeId);
							pMs->AddParam(szParamNodeId);
						}
						break;
						case OpcUaType_QualifiedName:
						{
							OpcUa_CharA* szString = (OpcUa_CharA*)OpcUa_Alloc(OpcUa_String_StrLen(&InputArgument.Value.QualifiedName->Name));
							szString = OpcUa_String_GetRawString(&(InputArgument.Value.QualifiedName->Name));
							pMs->AddParam(szString);
						}
						break;
						case OpcUaType_SByte:
							pMs->AddParam((lua_Number)InputArgument.Value.SByte);
							break;
						case OpcUaType_String:
						{
							OpcUa_CharA* szString = (OpcUa_CharA*)OpcUa_Alloc(OpcUa_String_StrLen(&InputArgument.Value.String));
							szString = OpcUa_String_GetRawString(&(InputArgument.Value.String));
							pMs->AddParam(szString);
						}
						break;
						default:
							OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "An unsupported parameter was receive in the method call\n");
							break;
						}
					}					
					if (!pMs->Go(1))
					{
						OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Cannot execute the script %s\n", pszLuaMethodName);
						(*ppResponse)->NoOfResults = 1;
						if ((*ppResponse)->Results)
							(*ppResponse)->Results[0].StatusCode = OpcUa_BadInternalError;
					}
					else
					{
						OpcUa_Variant varRes=pMs->GetVariantFromLuaFunction();
						(*ppResponse)->NoOfResults = 1;
						(*ppResponse)->Results = (OpcUa_CallMethodResult*)OpcUa_Alloc(sizeof(OpcUa_CallMethodResult)*(*ppResponse)->NoOfResults);
						if ((*ppResponse)->Results)
						{
							OpcUa_CallMethodResult_Initialize(&((*ppResponse)->Results[0]));
							// Prepare the answer to inform the caller that the method parameters was correct or not
							(*ppResponse)->Results[0].NoOfInputArgumentResults= pCallMethodRequest->NoOfInputArguments;
							(*ppResponse)->Results[0].InputArgumentResults = (OpcUa_StatusCode*)OpcUa_Alloc(sizeof(OpcUa_StatusCode)*pCallMethodRequest->NoOfInputArguments);
							for (OpcUa_UInt32 i = 0; i < pCallMethodRequest->NoOfInputArguments; i++)
								(*ppResponse)->Results[0].InputArgumentResults[i] = OpcUa_Good;
							//
							(*ppResponse)->Results[0].NoOfOutputArguments=1;
							(*ppResponse)->Results[0].OutputArguments = (OpcUa_Variant*)OpcUa_Alloc(sizeof(OpcUa_Variant)*(*ppResponse)->Results[0].NoOfOutputArguments);
							if ((*ppResponse)->Results[0].OutputArguments)
							{
								OpcUa_Variant_Initialize(&((*ppResponse)->Results[0].OutputArguments[0]));
								OpcUa_Variant_CopyTo(&varRes, &((*ppResponse)->Results[0].OutputArguments[0]));
							}
							(*ppResponse)->Results[0].StatusCode = OpcUa_Good;
						}
					}
				}
			}
			else
				pVm->ReportError(1);
		}
		else
			uStatus = OpcUa_BadInternalError;
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}
OpcUa_StatusCode CSessionServer::RemoveAllCallRequest()
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_Mutex_Lock(m_hCallMutex);
	for (CQueuedCallMessages::iterator it = m_pCallMessages->begin(); it != m_pCallMessages->end(); it++)
	{
		CQueuedCallMessage* pRequest = *it;
		if (pRequest)
			delete pRequest;
	}
	m_pCallMessages->clear();
	OpcUa_Mutex_Unlock(m_hCallMutex);
	return uStatus;
}
OpcUa_StatusCode CSessionServer::RemoveAllCallRequestDeleted()
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	if (m_pCallMessages)
	{
		OpcUa_Mutex_Lock(m_hCallMutex);
		CQueuedCallMessages::iterator it;
		CQueuedCallMessages	tmpCallRequests;
		tmpCallRequests.clear();
		for (it = m_pCallMessages->begin(); it != m_pCallMessages->end(); it++)
		{
			if ((*it)->IsDeleted())
				delete *it;
			else
				tmpCallRequests.push_back(*it);
		}
		m_pCallMessages->clear();
		m_pCallMessages->swap(tmpCallRequests);

		OpcUa_Mutex_Unlock(m_hCallMutex);
	}
	return uStatus;
}

// Queue QueryFirst Message
OpcUa_StatusCode CSessionServer::QueueQueryFirstMessage(CQueuedQueryFirstMessage* pQueryFirstMessage)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_Mutex_Lock(m_hQueryFirstMutex);
	// add to queue.
	m_pQueryFirstMessages->push_back(pQueryFirstMessage);
	OpcUa_Mutex_Unlock(m_hQueryFirstMutex);
	return uStatus;
}
OpcUa_StatusCode CSessionServer::RemoveAllQueryFirstRequest()
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_Mutex_Lock(m_hQueryFirstMutex);
	for (CQueuedQueryFirstMessages::iterator it = m_pQueryFirstMessages->begin(); it != m_pQueryFirstMessages->end(); it++)
	{
		CQueuedQueryFirstMessage* pRequest = *it;
		if (pRequest)
			delete pRequest;
	}
	m_pQueryFirstMessages->clear();
	OpcUa_Mutex_Unlock(m_hQueryFirstMutex);
	return uStatus;
}
OpcUa_StatusCode CSessionServer::RemoveAllQueryFirstRequestDeleted()
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	if (m_pQueryFirstMessages)
	{
		OpcUa_Mutex_Lock(m_hQueryFirstMutex);
		CQueuedQueryFirstMessages::iterator it;
		CQueuedQueryFirstMessages	tmpQueryFirstRequests;
		tmpQueryFirstRequests.clear();
		for (it = m_pQueryFirstMessages->begin(); it != m_pQueryFirstMessages->end(); it++)
		{
			if ((*it)->IsDeleted())
				delete *it;
			else
				tmpQueryFirstRequests.push_back(*it);
		}
		m_pQueryFirstMessages->clear();
		m_pQueryFirstMessages->swap(tmpQueryFirstRequests);

		OpcUa_Mutex_Unlock(m_hQueryFirstMutex);
	}
	return uStatus;
}
// Processes a queued QueryFirst request.		
OpcUa_StatusCode CSessionServer::ProcessQueuedQueryFirstRequest(CQueuedQueryFirstMessage* pQueryFirstMessage, OpcUa_Boolean bAbort)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	if (bAbort)
	{
		uStatus = pQueryFirstMessage->CancelSendResponse();
	}
	else
	{
		// Create a context to use for sending a response.
		uStatus = pQueryFirstMessage->BeginSendResponse();
		if (uStatus != OpcUa_Good)
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "CSessionServer::ProcessQueuedQueryFirstRequest>Stack could not initialize response.\n");
		else
		{
			// Réalise la lecture dans la cache du serveur
			pQueryFirstMessage->FillResponseHeader();
			//uStatus = ProcessQueryFirstRequest(pQueryFirstMessage);

			// Send the response to the client.
			uStatus = pQueryFirstMessage->EndSendResponse();
			if (OpcUa_IsBad(uStatus))
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "CSessionServer::ProcessQueuedQueryFirstRequest>Could not send response to client. Status 0x%08X!\r\n", uStatus);
			// Mark it for deletion
			pQueryFirstMessage->SetDeleted(OpcUa_True);

		}
	}
	pQueryFirstMessage->EncodeableObjectDelete();
	return uStatus;
}
// Queue QueryFirst Message
OpcUa_StatusCode CSessionServer::QueueQueryNextMessage(CQueuedQueryNextMessage* pQueryNextMessage)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_Mutex_Lock(m_hQueryNextMutex);
	// add to queue.
	m_pQueryNextMessages->push_back(pQueryNextMessage);
	OpcUa_Mutex_Unlock(m_hQueryNextMutex);
	return uStatus;
}
OpcUa_StatusCode CSessionServer::RemoveAllQueryNextRequest()
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_Mutex_Lock(m_hQueryNextMutex);
	for (CQueuedQueryNextMessages::iterator it = m_pQueryNextMessages->begin(); it != m_pQueryNextMessages->end(); it++)
	{
		CQueuedQueryNextMessage* pRequest = *it;
		if (pRequest)
			delete pRequest;
	}
	m_pQueryNextMessages->clear();
	OpcUa_Mutex_Unlock(m_hQueryNextMutex);
	return uStatus;
}
OpcUa_StatusCode CSessionServer::RemoveAllQueryNextRequestDeleted()
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	if (m_pQueryNextMessages)
	{
		OpcUa_Mutex_Lock(m_hQueryNextMutex);
		CQueuedQueryNextMessages::iterator it;
		CQueuedQueryNextMessages	tmpQueryNextRequests;
		tmpQueryNextRequests.clear();
		for (it = m_pQueryNextMessages->begin(); it != m_pQueryNextMessages->end(); it++)
		{
			if ((*it)->IsDeleted())
				delete *it;
			else
				tmpQueryNextRequests.push_back(*it);
		}
		m_pQueryNextMessages->clear();
		m_pQueryNextMessages->swap(tmpQueryNextRequests);

		OpcUa_Mutex_Unlock(m_hQueryNextMutex);
	}
	return uStatus;
}
// Processes a queued QueryNext request.		
OpcUa_StatusCode CSessionServer::ProcessQueuedQueryNextRequest(CQueuedQueryNextMessage* pQueryNextMessage, OpcUa_Boolean bAbort)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	if (bAbort)
	{
		uStatus = pQueryNextMessage->CancelSendResponse();
	}
	else
	{
		// Create a context to use for sending a response.
		uStatus = pQueryNextMessage->BeginSendResponse();
		if (uStatus != OpcUa_Good)
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "CSessionServer::ProcessQueuedQueryNextRequest>Stack could not initialize response.\n");
		else
		{
			// Réalise la lecture dans la cache du serveur
			pQueryNextMessage->FillResponseHeader();
			//uStatus = ProcessQueryNextRequest(pQueryNextMessage);

			// Send the response to the client.
			uStatus = pQueryNextMessage->EndSendResponse();
			if (OpcUa_IsBad(uStatus))
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "CSessionServer::ProcessQueuedQueryNextRequest>Could not send response to client. Status 0x%08X!\r\n", uStatus);
			// Mark it for deletion
			pQueryNextMessage->SetDeleted(OpcUa_True);

		}
	}
	pQueryNextMessage->EncodeableObjectDelete();
	return uStatus;
}
void CSessionServer::CleanupTimeoutedPublishRequest()
{	
	OpcUa_Mutex_Lock(m_hPublishMutex);
	if (!m_PublishRequests.empty())
	{
		CQueuedPublishMessages::iterator it = m_PublishRequests.begin();
		while (it != m_PublishRequests.end())
		{
			CQueuedPublishMessage* pPublishRequest = *it;
			// Need to check that
			if ((*it)->IsTimeouted())
			{
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_INFO, "CleanupTimeoutedPublishRequest>Context=%p\n", pPublishRequest->GetContext());
				pPublishRequest->EndSendResponse();
				it = m_PublishRequests.erase(it);
				delete pPublishRequest;
			}
			else
				++it;
		}
	}
	OpcUa_Mutex_Unlock(m_hPublishMutex);
}
// Retrieve the first publish request available in the m_PublishRequests
// return OpcUa_Null if no publish request was found
CQueuedPublishMessage* CSessionServer::GetFirstPublishRequest()
{
	CQueuedPublishMessage* pQueuedPublishRequest = OpcUa_Null;
	OpcUa_Mutex_Lock(m_hPublishMutex);
	if (!m_PublishRequests.empty())
	{
		//OpcUa_UInt32 iSize = m_PublishRequests.size();
		for (CQueuedPublishMessages::iterator it = m_PublishRequests.begin(); it != m_PublishRequests.end(); it++)
		{
			// Need to check that 
			if (!(*it)->IsTimeouted())
			{
				if (!(*it)->IsDeleted())
				{
					pQueuedPublishRequest = *it;
					break;
				}
			}
		}
	}
	else
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_WARNING, "GetFirstPublishRequest>m_PublishRequests late situation detected\n");
	OpcUa_Mutex_Unlock(m_hPublishMutex);
	return pQueuedPublishRequest;
}
// Retrieve the last publish request available in the m_PublishRequests
// return OpcUa_Null if no publish request was found
CQueuedPublishMessage* CSessionServer::GetLastPublishRequest() 
{
	CQueuedPublishMessage* pQueuedPublishRequest=OpcUa_Null;
	OpcUa_Mutex_Lock(m_hPublishMutex);
	if (!m_PublishRequests.empty())
	{
		for (CQueuedPublishMessages::reverse_iterator it=m_PublishRequests.rbegin();it!=m_PublishRequests.rend();it++)
		{
			if (!(*it)->IsDeleted()) 
			{
				pQueuedPublishRequest=*it;
				break;
			}
		}
	}
	else
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_WARNING, "m_PublishRequests late situation detected\n");
	OpcUa_Mutex_Unlock(m_hPublishMutex);
	return pQueuedPublishRequest;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Removes all the pending publish request for this subscription publish request. </summary>
///
/// <remarks>	Michel, 06/02/2016. </remarks>
///-------------------------------------------------------------------------------------------------

void CSessionServer::RemoveAllPublishRequest()
{
	OpcUa_Mutex_Lock(m_hPublishMutex);
	if (!m_PublishRequests.empty())
	{
		OpcUa_StatusCode uStatus = OpcUa_Semaphore_TimedWait(m_hPublishSem, OPC_TIMEOUT * 2);
		if (uStatus == OpcUa_GoodNonCriticalTimeout)
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "RemoveAllPublishRequestDeleted LEAVE on TIMEOUT... this is not good. Contact Michel Condemine\n");

		CQueuedPublishMessages::iterator it = m_PublishRequests.begin();
		while (!m_PublishRequests.empty())
		{
			CQueuedPublishMessage* pPublishRequest = *it;
			if (pPublishRequest->GetInternalPublishResponse() == OpcUa_Null)
			{
				if (!pPublishRequest->IsDeleted())
				{
					pPublishRequest->SetServiceResult(OpcUa_BadNoSubscription);//OpcUa_BadNoSubscription - OpcUa_Good
					if (pPublishRequest->BeginSendResponse() == OpcUa_Good)
					{
						pPublishRequest->SetKeepAlive(OpcUa_True);
						//uStatus = pPublishRequest->FillNotificationMessage(1);
						uStatus = pPublishRequest->EndSendResponse();
					}
				}
				//	
			}
			else
			{
				pPublishRequest->SetKeepAlive(OpcUa_True);
				uStatus = pPublishRequest->FillNotificationMessage(1);
				if (uStatus == OpcUa_Good)
				{
					pPublishRequest->SetServiceResult(OpcUa_BadNoSubscription);
					OpcUa_Handle hHandle = pPublishRequest->GetContext();
					if (!pPublishRequest->IsDeleted())
					{
						OpcUa_Endpoint_CancelSendResponse(pPublishRequest->GetEndpoint(),
							OpcUa_Good,
							OpcUa_Null,
							&hHandle);
					}
				}
			}
			delete *it;
			m_PublishRequests.erase(it);
			it = m_PublishRequests.begin();
		}
	}
	OpcUa_Mutex_Unlock(m_hPublishMutex);
	// on signal que la semaphore m_hPublishSem est disponible
	OpcUa_Semaphore_Post(m_hPublishSem, 1);
}
OpcUa_StatusCode CSessionServer::RemoveAllPublishRequestDeleted()
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	if (OpcUa_Semaphore_TimedWait(m_hPublishSem, OPC_TIMEOUT) == OpcUa_GoodNonCriticalTimeout)
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"RemoveAllPublishRequestDeleted LEAVE on TIMEOUT... this is not good. Contact Michel Condemine\n");
	OpcUa_Mutex_Lock(m_hPublishMutex);
	CQueuedPublishMessages::iterator it;
	CQueuedPublishMessages	tmpPublishRequests;
	tmpPublishRequests.clear();
	for (it = m_PublishRequests.begin(); it != m_PublishRequests.end(); it++)
	{
		if ((*it)->IsDeleted())
			delete *it;
		else
			tmpPublishRequests.push_back(*it);
	}
	m_PublishRequests.clear();
	tmpPublishRequests.swap(m_PublishRequests);

	OpcUa_Mutex_Unlock(m_hPublishMutex);
	// on signal que la semaphore m_hPublishSem est disponible
	OpcUa_Semaphore_Post(m_hPublishSem,1);
	return uStatus;
}
OpcUa_StatusCode CSessionServer::RemoveAllHistoryReadRequest()
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_Mutex_Lock(m_hHistoryReadMutex);
	for (CQueuedHistoryReadMessages::iterator it = m_pHistoryReadMessages->begin(); it != m_pHistoryReadMessages->end(); it++)
	{
		CQueuedHistoryReadMessage* pRequest = *it;
		if (pRequest)
			delete pRequest;
	}
	m_pHistoryReadMessages->clear();
	OpcUa_Mutex_Unlock(m_hHistoryReadMutex);
	return uStatus;
}
OpcUa_StatusCode CSessionServer::RemoveAllHistoryReadRequestDeleted()
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	if (m_pHistoryReadMessages)
	{
		OpcUa_Mutex_Lock(m_hHistoryReadMutex);
		CQueuedHistoryReadMessages::iterator it;
		CQueuedHistoryReadMessages	tmpHistoryReadRequests;
		tmpHistoryReadRequests.clear();
		for (it = m_pHistoryReadMessages->begin(); it != m_pHistoryReadMessages->end(); it++)
		{
			if ((*it)->IsDeleted())
				delete *it;
			else
				tmpHistoryReadRequests.push_back(*it);
		}
		m_pHistoryReadMessages->clear();
		m_pHistoryReadMessages->swap(tmpHistoryReadRequests);

		OpcUa_Mutex_Unlock(m_hHistoryReadMutex);
	}

	/*
	OpcUa_Mutex_Lock(m_hHistoryReadMutex);
	OpcUa_UInt32 ii = 0;
	CQueuedHistoryReadMessages* myQueuedHistoryReadRequests = new CQueuedHistoryReadMessages();
	for (ii = 0; ii<m_pHistoryReadMessages->size(); ii++)
	{
		CQueuedHistoryReadMessage* pHistoryReadMessage = (m_pHistoryReadMessages->at(ii));
		if (pHistoryReadMessage->IsDeleted())
			delete pHistoryReadMessage;
		else
			myQueuedHistoryReadRequests->push_back(pHistoryReadMessage);
	}
	m_pHistoryReadMessages->clear();
	// retransfert
	for (ii = 0; ii<myQueuedHistoryReadRequests->size(); ii++)
		m_pHistoryReadMessages->push_back(myQueuedHistoryReadRequests->at(ii));
	myQueuedHistoryReadRequests->clear();
	delete myQueuedHistoryReadRequests;
	OpcUa_Mutex_Unlock(m_hHistoryReadMutex);
	*/
	return uStatus;
}
OpcUa_StatusCode CSessionServer::RemoveAllReadRequest()
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_Mutex_Lock(m_hReadMutex);
	for (CQueuedReadMessages::iterator it=m_pReadMessages->begin();it!=m_pReadMessages->end();it++)
	{
		CQueuedReadMessage* pRequest =*it;
		if (pRequest)
			delete pRequest;
	}
	m_pReadMessages->clear();
	OpcUa_Mutex_Unlock(m_hReadMutex);
	return uStatus;
}
OpcUa_StatusCode CSessionServer::RemoveAllReadRequestDeleted()
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	if (m_pReadMessages)
	{
		OpcUa_Mutex_Lock(m_hReadMutex);
		CQueuedReadMessages::iterator it;
		CQueuedReadMessages	tmpReadRequests;
		tmpReadRequests.clear();
		for (it = m_pReadMessages->begin(); it != m_pReadMessages->end(); it++)
		{
			if ((*it)->IsDeleted())
				delete *it;
			else
				tmpReadRequests.push_back(*it);
		}
		m_pReadMessages->clear();
		m_pReadMessages->swap(tmpReadRequests);

		OpcUa_Mutex_Unlock(m_hReadMutex);
	}

	/*
	OpcUa_Mutex_Lock(m_hReadMutex);
	OpcUa_UInt32 ii=0;
	CQueuedReadMessages* myQueuedReadRequests=new CQueuedReadMessages();
	for (ii=0;ii<m_pReadMessages->size();ii++)
	{
		CQueuedReadMessage* pRequest =(m_pReadMessages->at(ii));
		if (pRequest->IsDeleted())
			delete pRequest;
		else
			myQueuedReadRequests->push_back(pRequest);
	}
	m_pReadMessages->clear();
	// retransfert
	for (ii=0;ii<myQueuedReadRequests->size();ii++)
		m_pReadMessages->push_back(myQueuedReadRequests->at(ii));
	myQueuedReadRequests->clear();
	delete myQueuedReadRequests;
	OpcUa_Mutex_Unlock(m_hReadMutex);
	*/
	return uStatus;
}

OpcUa_StatusCode CSessionServer::RemoveReadRequest(CQueuedReadMessage* pReadRequest)
{
	OpcUa_StatusCode uStatus=OpcUa_BadInvalidArgument;
	OpcUa_Mutex_Lock(m_hReadMutex);
	for (CQueuedReadMessages::iterator it=m_pReadMessages->begin();it!=m_pReadMessages->end();it++)
	{
		CQueuedReadMessage* pTmpRequest =*it;
		if (pTmpRequest==pReadRequest)
		{			
			m_pReadMessages->erase(it);
			delete pReadRequest;
			uStatus=OpcUa_Good;
			break;
		}
	}
	OpcUa_Mutex_Unlock(m_hReadMutex);
	return uStatus;
}
OpcUa_StatusCode CSessionServer::ModifySubscription(
										OpcUa_UInt32               a_nSubscriptionId,
										OpcUa_Double               a_nRequestedPublishingInterval,
										OpcUa_UInt32               a_nRequestedLifetimeCount,
										OpcUa_UInt32               a_nRequestedMaxKeepAliveCount,
										OpcUa_UInt32               a_nMaxNotificationsPerPublish,
										OpcUa_Byte                 a_nPriority,
										OpcUa_Double*              a_pRevisedPublishingInterval,
										OpcUa_UInt32*              a_pRevisedLifetimeCount,
										OpcUa_UInt32*              a_pRevisedMaxKeepAliveCount)
{
	OpcUa_StatusCode uStatus=OpcUa_BadInvalidArgument;
	OpcUa_Mutex_Lock(m_hSubscriptionListMutex);
	CSubscriptionServer* pSubscription= NULL;
	uStatus=FindSubscription(a_nSubscriptionId,&pSubscription);
	if ((uStatus == OpcUa_Good) && (pSubscription))
	{
		pSubscription->SetLastTransmissionTime(OpcUa_DateTime_UtcNow());
		uStatus = pSubscription->ModifySubscription(a_nRequestedPublishingInterval,
			a_nRequestedLifetimeCount,
			a_nRequestedMaxKeepAliveCount,
			a_nMaxNotificationsPerPublish,
			a_nPriority,
			a_pRevisedPublishingInterval, a_pRevisedLifetimeCount, a_pRevisedMaxKeepAliveCount);
		if (uStatus == OpcUa_Good)
		{
			UpdateAsyncThreadInterval();
			// 
			UpdateNotificationDataThreadInterval();
			//
			SortSubscription();
			//WakeupNotificationDataThread();
		}
	}
	OpcUa_Mutex_Unlock(m_hSubscriptionListMutex);
	return uStatus;
}
OpcUa_StatusCode CSessionServer::SetPublishingMode( OpcUa_Boolean              a_bPublishingEnabled,
													OpcUa_Int32                a_nNoOfSubscriptionIds,
													const OpcUa_UInt32*		   a_pSubscriptionIds,
													OpcUa_Int32*               a_pNoOfResults,
													OpcUa_StatusCode**         a_pResults)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_StatusCode uServiceResult=OpcUa_Good;
	if (a_pSubscriptionIds)
	{
		if (a_pResults)
		{
			CSubscriptionServer* pSubscription=OpcUa_Null;
			OpcUa_Mutex_Lock(m_hSubscriptionListMutex);
			*a_pResults=(OpcUa_StatusCode*)OpcUa_Alloc(a_nNoOfSubscriptionIds*sizeof(OpcUa_StatusCode));
			*a_pNoOfResults=a_nNoOfSubscriptionIds;
			for (OpcUa_Int32 ii=0;ii<a_nNoOfSubscriptionIds;ii++)
			{
				uServiceResult=FindSubscription(a_pSubscriptionIds[ii],&pSubscription);
				if (uServiceResult==OpcUa_Good)
				{
					pSubscription->SetLastTransmissionTime(OpcUa_DateTime_UtcNow());
					pSubscription->SetPublishingEnabled(a_bPublishingEnabled);
					// Subscription DiagnosticData
					CSubscriptionDiagnosticsDataType* pSubscriptionDiagnosticsDataType = pSubscription->GetSubscriptionDiagnosticsDataType();
					if (pSubscriptionDiagnosticsDataType)
					{
						pSubscriptionDiagnosticsDataType->SetPublishingEnabled(a_bPublishingEnabled);
						pSubscription->UpdateSubscriptionDiagnosticsDataType();
					}
					(*a_pResults)[ii]=uServiceResult;
				}
				else
					(*a_pResults)[ii]=OpcUa_BadSubscriptionIdInvalid;
			}
			OpcUa_Mutex_Unlock(m_hSubscriptionListMutex);
		}
	}

	return uStatus;
}
OpcUa_StatusCode CSessionServer::Republish( OpcUa_UInt32 a_nSubscriptionId,
							OpcUa_UInt32 a_nRetransmitSequenceNumber,
							OpcUa_NotificationMessage** a_pNotificationMessage)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	CSubscriptionServer* pSubscription=OpcUa_Null;
	OpcUa_Mutex_Lock(m_hSubscriptionListMutex);
	uStatus=FindSubscription(a_nSubscriptionId,&pSubscription);
	if (uStatus==OpcUa_Good)
	{
		pSubscription->SetLastTransmissionTime(OpcUa_DateTime_UtcNow());
		uStatus=pSubscription->Republish(a_nRetransmitSequenceNumber,a_pNotificationMessage);
	}
	OpcUa_Mutex_Unlock(m_hSubscriptionListMutex);
	return uStatus;
}

OpcUa_StatusCode CSessionServer::RegisterNodes(OpcUa_Int32 a_nNoOfNodesToRegister,
											   const OpcUa_NodeId* a_pNodesToRegister,
											   OpcUa_Int32* a_pNoOfRegisteredNodeIds,
											   OpcUa_NodeId** a_pRegisteredNodeIds)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;

	CUAInformationModel* pInformationModel=CServerApplication::m_pTheAddressSpace;
	if (a_pNodesToRegister)
	{
		if (a_nNoOfNodesToRegister>0)
		{
			(*a_pRegisteredNodeIds)=(OpcUa_NodeId*)OpcUa_Alloc(sizeof(OpcUa_NodeId)*a_nNoOfNodesToRegister);
			(*a_pNoOfRegisteredNodeIds)=a_nNoOfNodesToRegister;
			for (OpcUa_Int32 ii=0;ii<a_nNoOfNodesToRegister;ii++)
			{
				CUABase* pUABase=NULL; 
				uStatus = pInformationModel->GetNodeIdFromDictionnary(a_pNodesToRegister[ii], &pUABase);
				if (uStatus == OpcUa_Good)
				{

					//(*a_pRegisteredNodeIds)[ii]=a_pNodesToRegister[ii];
					OpcUa_NodeId_CopyTo(&a_pNodesToRegister[ii], &((*a_pRegisteredNodeIds)[ii]));
					// Il ne reste qu'a placer pUABase dans un dictionnaire à acès rapide
				}
				else
				{
					//OpcUa_NodeId* pNodeId =(OpcUa_NodeId*)OpcUa_Alloc(sizeof(OpcUa_NodeId)); // Utils::Copy((OpcUa_NodeId*)&a_pNodesToRegister[ii]);
					//if (pNodeId)
					//{
					//	OpcUa_NodeId_Initialize(pNodeId);
					//	uStatus = OpcUa_NodeId_CopyTo(&a_pNodesToRegister[ii], pNodeId);
						OpcUa_NodeId_Initialize(&((*a_pRegisteredNodeIds)[ii]));
						OpcUa_NodeId_CopyTo(&a_pNodesToRegister[ii], &((*a_pRegisteredNodeIds)[ii]));
					//}
					uStatus=IsNodeIdValid(a_pNodesToRegister[ii]);
					if (uStatus==OpcUa_Good)
						m_RegistredNodes.push_back(a_pNodesToRegister[ii]);
					
				}
			}
		}
		else
			uStatus=OpcUa_BadNothingToDo;
	}
	else
		uStatus=OpcUa_BadNothingToDo;
	return uStatus;
}
OpcUa_StatusCode CSessionServer::UnregisterNodes(OpcUa_Int32 a_nNoOfNodesToUnregister,
												 const OpcUa_NodeId* a_pNodesToUnregister)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	if (a_pNodesToUnregister)
	{
		std::vector<OpcUa_NodeId> tmpNodeIds;
		if (a_nNoOfNodesToUnregister>0)
		{
			// On supprime les elements demandé
			for (OpcUa_Int32 i=0;i<a_nNoOfNodesToUnregister;i++)
			{
				if (!Utils::IsNodeIdNull(a_pNodesToUnregister[i]))
				{
					for (OpcUa_UInt32 ii=0;ii<m_RegistredNodes.size();ii++)
					{
						OpcUa_NodeId aNodeId=m_RegistredNodes.at(ii);	
						if (!Utils::IsNodeIdNull(aNodeId))
						{
							if (!Utils::IsEqual(&a_pNodesToUnregister[i],&aNodeId))
								tmpNodeIds.push_back(aNodeId);
						}
						else
							uStatus=OpcUa_BadNodeIdInvalid;
					}
					// On reconstruit la liste finale
					m_RegistredNodes.clear();
					for (OpcUa_UInt32 ii=0;ii<tmpNodeIds.size();ii++)
						m_RegistredNodes.push_back(tmpNodeIds.at(ii));
					tmpNodeIds.clear();
				}
				else
					uStatus=OpcUa_BadNodeIdInvalid;
			}
		}
		else
			uStatus=OpcUa_BadNothingToDo;
	}
	else
		uStatus=OpcUa_BadNothingToDo;
	return uStatus;
}

OpcUa_StatusCode CSessionServer::RemoveSubscriptions()
{
	OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_WARNING, "OpenOpcUaCore server is removing all subscription for this session.\n");
	//OpcUa_Mutex_Lock(m_hPublishMutex);
	OpcUa_Mutex_Lock(m_hSubscriptionListMutex);
	for (OpcUa_UInt16 ii=0;ii<m_SubscriptionList.size();ii++)
	{
		CSubscriptionServer* pSubscription=m_SubscriptionList.at(ii);
		// will be remove in the pSubscription destructor
		delete pSubscription;
		pSubscription = OpcUa_Null;
	}
	m_SubscriptionList.clear();
	OpcUa_Mutex_Unlock(m_hSubscriptionListMutex);
	//OpcUa_Mutex_Unlock(m_hPublishMutex);
	return OpcUa_Good;
}
OpcUa_StatusCode CSessionServer::RemoveSubscriptionBySubscriptionId(OpcUa_UInt32 SubscriptionId)
{
	OpcUa_StatusCode uStatus=OpcUa_BadNotFound;
	OpcUa_Mutex_Lock(m_hSubscriptionListMutex);
	//OpcUa_Mutex_Lock(m_hPublishMutex); // 1.0.4.0
	CSubscriptionServer* pSubscription = OpcUa_Null;
	uStatus = FindSubscription(SubscriptionId, &pSubscription);
	if ((uStatus == OpcUa_Good) && (pSubscription))
		uStatus = RemoveSubscription(pSubscription);
	//OpcUa_Mutex_Unlock(m_hPublishMutex);
	OpcUa_Mutex_Unlock(m_hSubscriptionListMutex);
	return uStatus;
}
OpcUa_StatusCode CSessionServer::RemoveSubscription(CSubscriptionServer* pSubscription)
{
	OpcUa_StatusCode uStatus=OpcUa_BadNotFound;
	if (pSubscription)
	{
		CSubscriptionList::iterator it;
		for (it=m_SubscriptionList.begin();it!=m_SubscriptionList.end();it++)
		{			
			if (*it==pSubscription)
			{
				delete pSubscription;
				pSubscription = OpcUa_Null;
				m_SubscriptionList.erase(it);
				uStatus=OpcUa_Good;
				break;
			}
		}
		if (uStatus == OpcUa_Good)
		{
			// CTT workaround 002.js - Subscription Publish Min 02
			if (m_SubscriptionList.size() == 0)
				RemoveAllPublishRequest();
			// End workaround
		}
	}
	else
		uStatus=OpcUa_BadInvalidArgument;
	return uStatus;
}

void CSessionServer::StartSubscriptionsLifeTimeCountThread()
{
	if (m_hSubscriptionsLifeTimeCountThread==OpcUa_Null)
	{
		m_bSubscriptionLifeTimeCountThread=OpcUa_True;
		OpcUa_StatusCode uStatus  = OpcUa_Thread_Create(&m_hSubscriptionsLifeTimeCountThread,(CSessionServer::SubscriptionsLifeTimeCountThread),this);

		if(uStatus != OpcUa_Good)
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Create SubscriptionsLifeTimeCountThread Failed");
		else
		{
			OpcUa_Thread_Start(m_hSubscriptionsLifeTimeCountThread);
		}
	}
}
OpcUa_StatusCode CSessionServer::StopSubscriptionsLifeTimeCount()
{
	OpcUa_StatusCode uStatus=OpcUa_BadInvalidArgument;
	m_bSubscriptionLifeTimeCountThread=OpcUa_False;
	OpcUa_Semaphore_Post(m_SubscriptionsLifeTimeCountSem,1);
	uStatus = OpcUa_Semaphore_TimedWait( m_hStopSubscriptionsLifeTimeCountThreadSem,OPC_TIMEOUT*2); // 15 secondes max.
	if (uStatus == OpcUa_GoodNonCriticalTimeout)
	{
		// on force la fin du thread de surverillance des subscriptions
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Impossible to stop the SubscriptionsLifeTimeCount. Timeout");
	}
	else
	{
		OpcUa_Thread_Delete(m_hSubscriptionsLifeTimeCountThread);
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_WARNING, "SubscriptionsLifeTimeCount stopped properly\n");
	}
	return uStatus;
}
void CSessionServer::SubscriptionsLifeTimeCountThread(LPVOID arg)
{
	CSessionServer* pSessionServer=(CSessionServer*)arg;
	DWORD dwSleepTime=0;
	DWORD dwInterval=0;
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_Boolean bCold = OpcUa_True;
	while (pSessionServer->m_bSubscriptionLifeTimeCountThread)
	{
#if _WIN32_WINNT >= 0x0600
		ULONGLONG dwStart = GetTickCount64();
#else 
	#if _WIN32_WINNT == 0x0501
		DWORD dwStart = GetTickCount();
	#endif
#endif
#ifdef _GNUC_
		DWORD dwStart = GetTickCount();
#endif		
		// Check que la souscription ne doit pas être arrêtée
		OpcUa_Mutex_Lock(pSessionServer->m_hSubscriptionListMutex);
		if (!bCold)
		{
			for (OpcUa_UInt32 ii = 0; ii < pSessionServer->m_SubscriptionList.size(); ii++)
			{
				CSubscriptionServer* pSubscriptionServer = pSessionServer->m_SubscriptionList.at(ii);
				if (pSubscriptionServer->IsLifetimeCountReach())
				{
					// We have to post a StatusChangeNotification
					//OpcUa_Semaphore publishSem=pSessionServer->GetPublishSem();
					//uStatus=OpcUa_Semaphore_TimedWait(publishSem,OPC_TIMEOUT*2);
					//if (uStatus == OpcUa_GoodNonCriticalTimeout)
					//	OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"CSessionServer::SubscriptionsLifeTimeCountThread LEAVE on TIMEOUT... this is not good. Contact Michel Condemine\n");
					//OpcUa_Mutex publishMutex=pSessionServer->GetPublishMutex();
					//if (publishMutex)
					//{
					//	OpcUa_Mutex_Lock(publishMutex);
					//	// Recherche de la dernière publishRequest disponible pour la notification au client
					//	CQueuedPublishMessage* pPublishRequest= pSessionServer->GetLastPublishRequest();
					//	if (pPublishRequest)
					//	{
					//		// Prepare and send the statusChangeNotification
					//		pPublishRequest->SetKeepAlive(OpcUa_False);
					//		pPublishRequest->SetServiceResult(OpcUa_BadTimeout);
					//		uStatus=pPublishRequest->BeginSendResponse();
					//		OpcUa_UInt32 uiSeq=pSubscriptionServer->GetCurrentSequenceNumber();
					//		uStatus=pPublishRequest->FillNotificationMessage(uiSeq);
					//		uStatus=pPublishRequest->EndSendResponse();
					//		delete pPublishRequest;
					//	}
					//	OpcUa_Mutex_Unlock(publishMutex);
					//}
					//OpcUa_Semaphore_Post(publishSem,1);
					// Will now delete the subscription
					///////////////////////////////////////////////////////////////////////////
					// On va fabriquer un StatusChangeNotification
					pSessionServer->m_StatusCodes.push_back(OpcUa_BadTimeout);
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_WARNING, "Subscriptions will be removed for timeout\n");
					// OpcUa_Mutex_Unlock(pSessionServer->m_hSubscriptionListMutex);
					pSessionServer->RemoveSubscription(pSubscriptionServer);
					break;
					//OpcUa_UInt32 uiSubscriptionId = pSubscriptionServer->GetSubscriptionId();
					//if (pSessionServer->RemoveSubscriptionBySubscriptionId(uiSubscriptionId) == OpcUa_Good)
					//{
					//	OpcUa_Mutex_Lock(pSessionServer->m_hSubscriptionListMutex);
					//	break;
					//}
					//else
					//	OpcUa_Mutex_Lock(pSessionServer->m_hSubscriptionListMutex);
				}
			}
		}
		else
			bCold = OpcUa_False;
		OpcUa_Mutex_Unlock(pSessionServer->m_hSubscriptionListMutex);
		dwInterval=(DWORD)pSessionServer->GetFastestSubscriptionPublishingInterval();
		// calcul du nouvel interval relatif (dwSleepTime)
		DWORD dwEnd=GetTickCount();
		DWORD dwCountedTime=dwEnd-dwStart;
		if (dwInterval>dwCountedTime)
			dwSleepTime=dwInterval-dwCountedTime;
		else
			dwSleepTime=0;
		uStatus=OpcUa_Semaphore_TimedWait( pSessionServer->m_SubscriptionsLifeTimeCountSem, dwSleepTime );  // on attend jusqu'a qu'une demande async soit posé dans la queue

	}
	OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_WARNING,"SubscriptionsLifeTimeCountThread stopped\n");
	OpcUa_Semaphore_Post(pSessionServer->m_hStopSubscriptionsLifeTimeCountThreadSem,1);
}
// trouve la Publsh interval le plus rapide pour les souscription prise en charge par cette session
OpcUa_Double CSessionServer::GetFastestSubscriptionPublishingInterval()
{
	OpcUa_Double dblFastestSubscription=60000.0; // 1mn
	OpcUa_Mutex_Lock(m_hSubscriptionListMutex);
	for (OpcUa_UInt32 ii=0;ii<m_SubscriptionList.size();ii++)
	{
		CSubscriptionServer* pSubscriptionServer=m_SubscriptionList.at(ii);
		if (pSubscriptionServer->GetPublishingInterval()<dblFastestSubscription)
			dblFastestSubscription=pSubscriptionServer->GetPublishingInterval();
	}
	OpcUa_Mutex_Unlock(m_hSubscriptionListMutex);
	return dblFastestSubscription;
}
// permet de savoir si on a à faire a une session créée pour un client UA 1.01 ou 1.02
// cf : ERRATA 1.02 UA Specification :
// http://www.opcfoundation.org/DownloadFile.aspx?CM=3&RI=944&CN=KEY&CI=283&CU=10
OpcUa_Boolean CSessionServer::Is101Session()
{
	return OpcUa_True;
	// Le code ci-dessous devrait être utilisé. Cependant le CTT pour rester compatible avec les client 101 se comporte différement.
	// Il s'agit peut être d'un pb dans le CTT. Voir avec Nathan
	//OpcUa_Boolean bResult=OpcUa_False;
	//if (m_pSecureChannel)
	//{
	//	OpcUa_ByteString*  pClientCertificate=m_pSecureChannel->GetClientCertificate();
	//	if (pClientCertificate)
	//	{
	//		if (pClientCertificate->Data)
	//			bResult= OpcUa_True;
	//	}

	//}
	//return bResult;
}
OpcUa_StatusCode CSessionServer::ReadScalarExtensionObject(CUAVariable* pUAVariable,OpcUa_Int32 ii,OpcUa_ReadResponse* pResponse)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	if (pResponse)
	{
		if (pUAVariable)
		{
			CDataValue* pDataValue=pUAVariable->GetValue();
			if (pDataValue)
			{
				OpcUa_ExtensionObject* pExtensionObj=pDataValue->GetValue().Value.ExtensionObject;
				// Read the dataType of the current instance pUAVariable
				OpcUa_NodeId aNodeId=pUAVariable->GetDataType();;
				CUADataType* pUADataType=OpcUa_Null;
				CUAInformationModel* pInformationModel=CServerApplication::m_pTheAddressSpace;

				// Look for the dataType
				uStatus=pInformationModel->GetNodeIdFromDataTypeList(aNodeId,&pUADataType);
				if (uStatus==OpcUa_Good)
				{
					// Get the definition for this DataType
					CDefinition* pDefinition=pUADataType->GetDefinition();
					if (pDefinition)
					{
						OpcUa_Int32 iInstanceComputedSize=-1;
						uStatus=pDefinition->GetInstanceSize(&iInstanceComputedSize);
						OpcUa_ExtensionObject* pNewExtensionObject=OpcUa_Null;
						uStatus=pDefinition->DuplicateExtensionObject(pExtensionObj,&pNewExtensionObject);
						if (uStatus==OpcUa_Good)
						{
							pResponse->Results[ii].Value.Value.ExtensionObject=(OpcUa_ExtensionObject*)OpcUa_Alloc(sizeof(OpcUa_ExtensionObject));
							OpcUa_ExtensionObject_Initialize(pResponse->Results[ii].Value.Value.ExtensionObject);
							OpcUa_ExtensionObject_CopyTo(pNewExtensionObject,pResponse->Results[ii].Value.Value.ExtensionObject);
							if (pNewExtensionObject)
							{
								OpcUa_ExtensionObject_Clear(pNewExtensionObject);
								OpcUa_Free(pNewExtensionObject);
								pNewExtensionObject = OpcUa_Null;
							}
							//pResponse->Results[ii].Value.Datatype=OpcUaType_ExtensionObject;
							//pResponse->Results[ii].Value.Value.ExtensionObject=pNewExtensionObject;
						}
						else
						{
							pResponse->Results[ii].Value.Datatype = OpcUaType_Null;
							pResponse->Results[ii].Value.Value.ExtensionObject = OpcUa_Null;
							OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_WARNING, "ReadScalarExtensionObject>DuplicateExtensionObject failed uStatus=0x%05x\n", uStatus);
							uStatus = OpcUa_Good;
						}
					}
					else
					{
						char* szNodeId = OpcUa_Null;
						Utils::NodeId2String(&aNodeId, &szNodeId);
						if (szNodeId)
						{
							OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critical error in you NodeSetFile the nodeId %s haven't any datatype\n", szNodeId);
							OpcUa_Free(szNodeId);
						}
					}
				}
				else
				{
					uStatus=OpcUa_BadInvalidArgument;
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"ReadScalarExtensionObject>GetNodeIdFromDataTypeList failed\n");
				}
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

/// <summary>
/// Acks the session events.
/// </summary>
/// <param name="pEventDefinition">The p event definition.</param>
/// <returns></returns>
OpcUa_StatusCode CSessionServer::AckCommentConfirmSessionEvents(CEventDefinition* pEventDefinition, const OpcUa_LocalizedText comment, METHOD_NAME eBehavior)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	CUAEventNotificationListList UAEventNotificationListListExtra;
	for (OpcUa_UInt32 i = 0; i < m_SubscriptionList.size(); i++)
	{
		CSubscriptionServer* pSubscription = m_SubscriptionList.at(i);
		OpcUa_Mutex aMutex = pSubscription->GetEventNotificationListListMutex();
		OpcUa_Mutex_Lock(aMutex);
		if (pSubscription->GetEventNotificationListFromEventDefinition(pEventDefinition, &UAEventNotificationListListExtra) == OpcUa_Good)
		{			
			for (OpcUa_UInt32 ii = 0; ii < UAEventNotificationListListExtra.size(); ii++)
			{
				CUAEventNotificationList* pUAEventNotificationListExtra = OpcUa_Null;
				pUAEventNotificationListExtra = UAEventNotificationListListExtra.at(ii);
				if (pUAEventNotificationListExtra)
				{
					if (pUAEventNotificationListExtra->GetEventDefinition() == pEventDefinition)
					{
						switch (eBehavior)
						{
							case UAAddressSpace::Acknowledge:
							{
								if (!pUAEventNotificationListExtra->IsAlarmAcked())
								{
									pUAEventNotificationListExtra->AlarmAcked();
									pUAEventNotificationListExtra->AddComment(comment);
									// mark as not sent
									pUAEventNotificationListExtra->TransactionAcked(OpcUa_False);
								}
							}
							break;
							case UAAddressSpace::Comment:
							{
								pUAEventNotificationListExtra->AddComment(comment);
								// mark as not sent
								pUAEventNotificationListExtra->TransactionAcked(OpcUa_False);
							}
							break;
							case UAAddressSpace::Confirm:
							{

								if (!pUAEventNotificationListExtra->IsConfirmed())
								{
									pUAEventNotificationListExtra->Confirmed();
									pUAEventNotificationListExtra->AddComment(comment);
									// mark as not sent
									pUAEventNotificationListExtra->TransactionAcked(OpcUa_False);
								}
							}
							break;
							default:
								uStatus = OpcUa_BadInvalidArgument;
								break;
						}
					}
				}
			}
		}
		OpcUa_Mutex_Unlock(aMutex);
	}
	return uStatus;
}

OpcUa_String CSessionServer::GetSessionName() 
{ 
	return m_SessionName; 
}
void CSessionServer::SetSessionName(OpcUa_String*  pStrVal)
{
	if (pStrVal)
	{
		if (OpcUa_String_StrLen(pStrVal) > 0)
		{
			OpcUa_String_Initialize(&m_SessionName);
			OpcUa_String_StrnCpy(&m_SessionName, pStrVal, OpcUa_String_StrLen(pStrVal));
		}
	}
}
CSessionDiagnosticsDataType*	CSessionServer::GetSessionDiagnosticsDataType() 
{ 
	return m_pSessionDiagnostics; 
} 
CSessionSecurityDiagnosticsDataType* CSessionServer::GetSessionSecurityDiagnosticsDataType() 
{ 
	return m_pSessionSecurityDiagnostics; 
}
OpcUa_Double CSessionServer::GetSessionTimeout() 
{ 
	return m_SessionTimeout; 
}
void CSessionServer::SetSessionTimeout(OpcUa_Double val) 
{ 
	m_SessionTimeout = val; 
}
OpcUa_StatusCode  CSessionServer::AddSubscription(CSubscriptionServer* pSubscription)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	if (pSubscription)
	{
		OpcUa_Mutex_Lock(m_hSubscriptionListMutex);
		m_SubscriptionList.push_back(pSubscription);
		pSubscription->SetLastTransmissionTime(OpcUa_DateTime_UtcNow());
		pSubscription->InLate(OpcUa_True);
		OpcUa_Mutex_Unlock(m_hSubscriptionListMutex);
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}

CSecureChannel*	CSessionServer::GetSecureChannel() 
{ 
	return	m_pSecureChannel; 
}
void CSessionServer::SetSecureChannel(CSecureChannel* pSecureChannel)
{ 
	m_pSecureChannel = pSecureChannel; 
}
OpcUa_Mutex CSessionServer::GetPublishMutex()
{ 
	return m_hPublishMutex;
}
OpcUa_Semaphore CSessionServer::GetPublishSem() 
{ 
	return m_hPublishSem;
}
OpcUa_UInt32 CSessionServer::GetPublishRequestsSize() 
{ 
	return m_PublishRequests.size(); 
}
OpcUa_Boolean CSessionServer::IsSubscription() // indique s'il y a des subscription dans cette session
{
	if (m_SubscriptionList.size()>0)
		return OpcUa_True;
	else
		return OpcUa_False;
}

OpcUa_Boolean CSessionServer::IsActive() 
{ 
	return m_bSessionActive; 
}
// indique si la session est en timeout
OpcUa_Boolean CSessionServer::IsTimeouted() 
{ 
	return !m_bRunSessionTimeoutThread; 
} 
CServerApplication*	CSessionServer::GetServerApplication() 
{ 
	return m_pApplication; 
}
OpcUa_ByteString* CSessionServer::GetServerNonce() 
{ 
	return m_pServerNonce;
}
OpcUa_ByteString* CSessionServer::GetClientNonce() 
{ 
	return	m_pClientNonce; 
}
OpcUa_UInt32 CSessionServer::GetSubscriptionListSize()
{
	return m_SubscriptionList.size();
}
OpcUa_Mutex CSessionServer::GetSubscriptionListMutex() 
{ 
	return m_hSubscriptionListMutex; 
}

OpcUa_Boolean CSessionServer::IsServerDiagnosticsEnabled() 
{ 
	return m_bServerDiagnosticsEnabled; 
} 

///-------------------------------------------------------------------------------------------------
/// <summary>	This function is called from CServerApplication::EnableServerDiagnostics in order to enable/disable the ServerDiagnostics. </summary>
///
/// <remarks>	Michel, 24/05/2016. </remarks>
///
/// <param name="bVal">	The value. </param>
///-------------------------------------------------------------------------------------------------

void CSessionServer::EnableServerDiagnostics(OpcUa_Boolean bVal) 
{ 
	OpcUa_StatusCode uStatus = OpcUa_Good;
	CUAInformationModel* pInformationModel = CServerApplication::m_pTheAddressSpace;
	m_bServerDiagnosticsEnabled = bVal; 
	// in the same time we will activate/deactivate the global indicator
	//pInformationModel->EnableServerDiagnosticsDefaultValue(bVal);
	OpcUa_ApplicationDescription* pUAApplicationDescription = OpcUa_Null;
	// Update the content of the node
	if (m_bServerDiagnosticsEnabled)
	{
		CApplicationDescription* pApplicationDescription = m_pApplication->GetApplicationDescription();
		pUAApplicationDescription = pApplicationDescription->GetInternalApplicationDescription();
		if (pUAApplicationDescription)
		{
			if (!m_pSessionDiagnostics)
				m_pSessionDiagnostics = new CSessionDiagnosticsDataType();
			m_pSessionDiagnostics->SetClientDescription(pUAApplicationDescription);	
			InitSessionDiagnosticsDataType();
			if (uStatus != OpcUa_Good)
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "InitSessionDiagnosticsDataType failed 0x%05x\n", uStatus);
			else
			{			
				if (!m_pSessionSecurityDiagnostics)
					m_pSessionSecurityDiagnostics = new CSessionSecurityDiagnosticsDataType();
				// initialisation du SessionSecurityDiagnosticsDataType associé a cette session
				uStatus = InitSessionSecurityDiagnosticsDataType();
				if (uStatus != OpcUa_Good)
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, 
						"InitSessionSecurityDiagnosticsDataType failed while Enabling ServerDiagnostics>0x%05x\n", uStatus);
			}
		}
		// Scan all existing subscription and add in the subscription diagnostics node
		OpcUa_Mutex_Lock(m_hSubscriptionListMutex);
		for (CSubscriptionList::iterator it = m_SubscriptionList.begin(); it != m_SubscriptionList.end(); it++)
		{
			CSubscriptionServer* pSubscription=*it;
			CSubscriptionDiagnosticsDataType* pSubscriptionDiagnosticsDataType=pSubscription->GetSubscriptionDiagnosticsDataType();
			if (pSubscriptionDiagnosticsDataType)
			{
				OpcUa_SubscriptionDiagnosticsDataType* pInternalSubscriptionDiagnosticsDataType = pSubscriptionDiagnosticsDataType->GetInternalSubscriptionDiagnosticsDataType();
				if (!pInternalSubscriptionDiagnosticsDataType)
				{
					pInternalSubscriptionDiagnosticsDataType = (OpcUa_SubscriptionDiagnosticsDataType*)OpcUa_Alloc(sizeof(OpcUa_SubscriptionDiagnosticsDataType));
					pSubscriptionDiagnosticsDataType->SetInternalSubscriptionDiagnosticsDataType(pInternalSubscriptionDiagnosticsDataType);
				}
			}
			else
			{
				pSubscriptionDiagnosticsDataType = new CSubscriptionDiagnosticsDataType();
				pSubscription->SetSubscriptionDiagnosticsDataType(pSubscriptionDiagnosticsDataType);
			}
			uStatus = InitSubscriptionDiagnosticsDataType(pSubscription);
			pSubscription->UpdateSubscriptionDiagnosticsDataType();
		}
		OpcUa_Mutex_Unlock(m_hSubscriptionListMutex);
	}
	else
	{
		if (m_pSessionDiagnostics)
		{
			if (pInformationModel->RemoveInSessionDiagnosticsArray(m_pSessionDiagnostics) == OpcUa_Good)
			{
				delete m_pSessionDiagnostics;
				m_pSessionDiagnostics = OpcUa_Null;
			}
		}
		//
		if (m_pSessionSecurityDiagnostics)
		{
			if (pInformationModel->RemoveInSessionSecurityDiagnosticsArray(m_pSessionSecurityDiagnostics) == OpcUa_Good)
			{
				delete m_pSessionSecurityDiagnostics;
				m_pSessionSecurityDiagnostics = OpcUa_Null;
			}
		}
		//		
		uStatus = pInformationModel->RemoveAllSubscriptionDiagnosticsArray();
		OpcUa_Mutex_Lock(m_hSubscriptionListMutex);
		// Scan all existing subscription and add in the subscription diagnostics node
		for (CSubscriptionList::iterator it = m_SubscriptionList.begin(); it != m_SubscriptionList.end(); it++)
		{
			CSubscriptionServer* pSubscription = *it;
			//OpcUa_Double dblSampleInterval = pSubscription->GetPublishingInterval();
			//OpcUa_SamplingIntervalDiagnosticsDataType* pSamplingIntervalDiagnostic = OpcUa_Null;
			//if (pInformationModel->IsSamplingIntervalDiagnosticsExist(dblSampleInterval, &pSamplingIntervalDiagnostic))
			//{
			//	uStatus = pInformationModel->RemoveInSamplingIntervalDiagnosticsArray(pSamplingIntervalDiagnostic);
			//}
			//CSubscriptionDiagnosticsDataType* pSubscriptionDiagnosticsDataType = pSubscription->GetSubscriptionDiagnosticsDataType();
			//delete pSubscriptionDiagnosticsDataType; // The pSubscriptionDiagnosticsDataType was delete in the call to RemoveAllSubscriptionDiagnosticsArray
			pSubscription->SetSubscriptionDiagnosticsDataType(OpcUa_Null);
		}
		OpcUa_Mutex_Unlock(m_hSubscriptionListMutex);
	}
}// correspond au contenu du NodeId i=2294

OpcUa_Semaphore CSessionServer::GetStopSubscriptionsLifeTimeCountSem() 
{ 
	return m_hStopSubscriptionsLifeTimeCountThreadSem; 
}
OpcUa_Semaphore CSessionServer::GetSubscriptionsLifeTimeCountSem() 
{ 
	return m_SubscriptionsLifeTimeCountSem; 
}

OpcUa_StatusCode CSessionServer::WakeupAllSubscription()
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	for (OpcUa_UInt32 ii = 0; ii < m_SubscriptionList.size(); ii++)
	{
		CSubscriptionServer* pSubscription = m_SubscriptionList.at(ii);
		pSubscription->WakeupUpdateSubscriptionThread();
	}
	return uStatus;
}

void CSessionServer::WakeupAsyncRequestThread()
{
	OpcUa_Semaphore_Post(m_hAsyncRequest, 1);
}
void CSessionServer::WakeupNotificationDataThread()
{
	OpcUa_Semaphore_Post(m_hNotificationDataSem, 1);
}
void CSessionServer::SortSubscription()
{
	// Sort and transfert back the subscriptionlist
	OpcUa_Mutex_Lock(m_hSubscriptionListMutex);
	CSubscriptionList theSortedSubscriptionList;
	SortSubscriptionsByPriority(255, &theSortedSubscriptionList);
	m_SubscriptionList.clear();
	for (OpcUa_UInt16 ii = 0; ii < theSortedSubscriptionList.size(); ii++)
	{
		CSubscriptionServer* pSubscription = theSortedSubscriptionList.at(ii);
		m_SubscriptionList.push_back(pSubscription);
	}
	theSortedSubscriptionList.clear();
	OpcUa_Mutex_Unlock(m_hSubscriptionListMutex);
}
// Organize Subscription by priority
void CSessionServer::SortSubscriptionsByPriority(OpcUa_Byte currentLowerPriority,CSubscriptionList* pSortedSubscriptionList)
{
	OpcUa_UInt32 uiNoOfSubscription = m_SubscriptionList.size();
	OpcUa_Byte refPriority = currentLowerPriority;
	for (OpcUa_UInt16 ii = 0; ii < uiNoOfSubscription; ii++)
	{
		CSubscriptionServer* pSubscription = m_SubscriptionList.at(ii);
		OpcUa_Byte subscriptionPriority = pSubscription->GetPriority();
		if (subscriptionPriority == refPriority)
		{
			refPriority = subscriptionPriority;
			break;
		}
	}
	for (OpcUa_UInt16 ii = 0; ii < uiNoOfSubscription; ii++)
	{
		CSubscriptionServer* pSubscription = m_SubscriptionList.at(ii);
		if (pSubscription->GetPriority() == refPriority)
			pSortedSubscriptionList->push_back(pSubscription);
	}
	if (pSortedSubscriptionList->size() != uiNoOfSubscription)
		SortSubscriptionsByPriority(currentLowerPriority - 1, pSortedSubscriptionList);
}

// lastTrasmissionTime manipulation
void CSessionServer::SetLastTransmissionTime(OpcUa_DateTime dtVal)
{
	OpcUa_DateTime_CopyTo(&dtVal, &m_LastTransmissionTime);
}
OpcUa_DateTime CSessionServer::GetLastTransmissionTime()
{
	return m_LastTransmissionTime;
}