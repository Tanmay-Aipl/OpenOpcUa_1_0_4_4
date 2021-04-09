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
#include "MurmurHash3.h"
using namespace OpenOpcUa;
using namespace UASharedLib;
using namespace UACoreClient;
#define ASYNC_PUBLISH 1

//OpcUa_Boolean CSubscriptionClient::m_bMonitoredItemsNotificationThread;
//OpcUa_Handle CSubscriptionClient::m_hStopMonitoredItemsNotificationThread;


CSubscriptionClient::CSubscriptionClient(void)
{
	m_pSession=OpcUa_Null;;
	m_ClassName=std::string("UAQuickClient::CSubscriptionClient");
	m_pAvailableSequenceNumbers=OpcUa_Null;
	m_NoOfAvailableSequenceNumbers=-1;
	OpcUa_Mutex_Create(&m_MonitoredItemListMutex);
	m_pSubscriptionDiagnosticsDataType=new CSubscriptionDiagnosticsDataType();
	m_pMonitoredItemList=new CMonitoredItemClientList();
	OpcUa_Mutex_Create(&m_MonitoredItemsNotificationListMutex);
	//
	m_pNotificationCallback=OpcUa_Null;
	m_pNotificationCallbackData=OpcUa_Null;
	m_hMonitoredItemsNotificationThread=OpcUa_Null;
	OpcUa_Semaphore_Create(&m_hMonitoredItemsNotificationSem, 0, 0x100);
	OpcUa_Semaphore_Create(&m_hStopMonitoredItemsNotificationSem,0,0x100);
	m_pMonitoredItemsNotificationList=new CMonitoredItemsNotificationList();
	m_pMonitoredItemsNotificationList->clear();
	StartMonitoredItemsNotificationThread();
}

CSubscriptionClient::~CSubscriptionClient(void)
{
	OpcUa_Mutex_Lock(m_MonitoredItemsNotificationListMutex);
	StopMonitoredItemsNotificationThread();
	delete m_pSubscriptionDiagnosticsDataType;
	CMonitoredItemsNotificationList::iterator it = m_pMonitoredItemsNotificationList->begin();
	while (it != m_pMonitoredItemsNotificationList->end())
	{
		delete *it;
		it++;
	}
	if (m_pAvailableSequenceNumbers)
	{
		OpcUa_Free(m_pAvailableSequenceNumbers);
		m_pAvailableSequenceNumbers = OpcUa_Null;
	}
	m_pMonitoredItemsNotificationList->clear();
	delete m_pMonitoredItemsNotificationList;
	DeleteMonitoredItems();
	SequenceNumberClearAll();
	OpcUa_Mutex_Unlock(m_MonitoredItemsNotificationListMutex);
	OpcUa_Mutex_Delete(&m_MonitoredItemsNotificationListMutex);
	OpcUa_Semaphore_Delete(&m_hMonitoredItemsNotificationSem);
	OpcUa_Semaphore_Delete(&m_hStopMonitoredItemsNotificationSem);
	OpcUa_Mutex_Delete(&m_MonitoredItemListMutex);
	delete m_pMonitoredItemList;
}
OpcUa_StatusCode CSubscriptionClient::CreateMonitoredItems(
							OpcUa_TimestampsToReturn                a_eTimestampsToReturn,
							OpcUa_Int32                             a_nNoOfItemsToCreate,
							OpcUa_MonitoredItemCreateRequest*		pItemsToCreate,
							OpcUa_MonitoredItemCreateResult**		ppResult,
							OpcUa_Handle**							hMonitoredItems)
{
	OpcUa_StatusCode uStatus;
	OpcUa_RequestHeader tRequestHeader;
	OpcUa_ResponseHeader tResponseHeader;
	OpcUa_Int32 tNoOfResults=0;
	OpcUa_Int32 tNoOfDiagnosticInfos=0;
	OpcUa_DiagnosticInfo*	tDiagnosticInfos=NULL;

	OpcUa_ResponseHeader_Initialize(&tResponseHeader);

	OpcUa_RequestHeader_Initialize(&tRequestHeader);
	tRequestHeader.TimeoutHint = g_ServiceCallTimeout;// UTILS_DEFAULT_TIMEOUT;
	tRequestHeader.Timestamp   = OpcUa_DateTime_UtcNow();
	OpcUa_NodeId* pNodeId = m_pSession->GetAuthenticationToken();
	OpcUa_NodeId_CopyTo(pNodeId, &(tRequestHeader.AuthenticationToken));
	tRequestHeader.RequestHandle = CClientApplication::m_uiRequestHandle++;
	OpcUa_MonitoredItemCreateResult_Initialize(*ppResult);

	OpcUa_DiagnosticInfo_Initialize(tDiagnosticInfos);

	if (m_pChannel)
	{
		if (a_nNoOfItemsToCreate > 0)
		{
			*hMonitoredItems = (OpcUa_Handle*)OpcUa_Alloc(sizeof(OpcUa_Handle)*a_nNoOfItemsToCreate);
			for (int i = 0; i < a_nNoOfItemsToCreate; i++)
			{
				CMonitoredItemClient* pMonitoredItem = new CMonitoredItemClient();
				(*hMonitoredItems)[i] = (OpcUa_Handle)pMonitoredItem;
				OpcUa_UInt32 uiClientHandle = 0;
				uiClientHandle = (OpcUa_UInt32)(uintptr_t)pMonitoredItem;
				pItemsToCreate[i].RequestedParameters.ClientHandle = uiClientHandle; // (OpcUa_UInt32)pMonitoredItem;
				//pItemsToCreate[i].RequestedParameters.DiscardOldest;
				//pItemsToCreate[i].RequestedParameters.Filter;
				//pItemsToCreate[i].RequestedParameters.QueueSize;
				//pItemsToCreate[i].RequestedParameters.SamplingInterval=0;

			}
			OpcUa_Channel aChannel = m_pChannel->GetInternalHandle();
			uStatus = OpcUa_ClientApi_CreateMonitoredItems(
				aChannel,
				&tRequestHeader,
				m_pSubscriptionId,
				a_eTimestampsToReturn,
				a_nNoOfItemsToCreate,
				pItemsToCreate,
				&tResponseHeader,
				&tNoOfResults,
				ppResult,
				&tNoOfDiagnosticInfos,
				&tDiagnosticInfos);
			// si la fonction a aboutie. On stock le résultat dans une instance de ...TODO a définir
			if ((OpcUa_IsGood(uStatus)) && (*ppResult))
			{

				//
				if (tNoOfResults != a_nNoOfItemsToCreate)
					uStatus = OpcUa_BadInternalError;
				else
				{
					for (int ii = 0; ii < tNoOfResults; ii++)
					{
						CMonitoredItemClient* pMonitoredItem = (CMonitoredItemClient*)(*hMonitoredItems)[ii];
						pMonitoredItem->SetClientHandle((OpcUa_UInt32)(uintptr_t)(*hMonitoredItems)[ii]);

						OpcUa_NodeId aNodeId;
						aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
						aNodeId.NamespaceIndex = 0;
						aNodeId.Identifier.Numeric = (*ppResult)[ii].MonitoredItemId;
						// TODO prendre en compte l'heure ... 
						//pMonitoredItem->SetDateTime(pItemsToCreate[ii]->GetDateTime());
						pMonitoredItem->SetClientHandle(pItemsToCreate[ii].RequestedParameters.ClientHandle);
						pMonitoredItem->SetSession(GetSession());
						pMonitoredItem->SetNodeId(pItemsToCreate[ii].ItemToMonitor.NodeId);
						pMonitoredItem->SetFilterToUse(&((*ppResult)[ii].FilterResult));
						pMonitoredItem->SetSamplingInterval((*ppResult)[ii].RevisedSamplingInterval);
						pMonitoredItem->SetLastError((*ppResult)[ii].StatusCode);
						pMonitoredItem->SetQueueSize((*ppResult)[ii].RevisedQueueSize);
						pMonitoredItem->SetMonitoredItemId((*ppResult)[ii].MonitoredItemId);
						pMonitoredItem->SetMonitoringMode(pItemsToCreate[ii].MonitoringMode);
						AddMonitoredItem(pMonitoredItem);
					}
				}
			}
			else
			{
				uStatus = OpcUa_BadInternalError;
				OpcUa_Trace(OPCUA_TRACE_CLIENT_LEVEL_ERROR, "CreateMonitoredItems Failed\n");
			}
		}
		else
			uStatus=OpcUa_BadNothingToDo;

	}
	else
		uStatus=OpcUa_BadInvalidArgument;
	OpcUa_RequestHeader_Clear(&tRequestHeader);
	return uStatus;
}

void CSubscriptionClient::StartMonitoredItemsNotificationThread()
{
	if (m_hMonitoredItemsNotificationThread==OpcUa_Null)
	{
		m_bMonitoredItemsNotificationThread=OpcUa_True;
		OpcUa_StatusCode uStatus  = OpcUa_Thread_Create(&m_hMonitoredItemsNotificationThread,OpenOpcUa::UACoreClient::CSubscriptionClient::MonitoredItemsNotificationThread,this);

		if(uStatus != OpcUa_Good)
			OpcUa_Trace(OPCUA_TRACE_CLIENT_LEVEL_ERROR,"Create MonitoredItemsNotificationThread Failed\n");
		else
			OpcUa_Thread_Start(m_hMonitoredItemsNotificationThread);
	}
}
// Cette thread vide la collection m_pMonitoredItems et appel 
// la fonction callback qui est implémenté par l'application qui utilise la DLL
void CSubscriptionClient::MonitoredItemsNotificationThread(LPVOID arg)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	void* pCallbackData=OpcUa_Null;
	PFUNC pFunc=OpcUa_Null;
	CSubscriptionClient* pSubscriptionClient=(CSubscriptionClient*)arg;
	DWORD dwInterval=50;
	DWORD dwSleepTime=0;
	DWORD dwStart=0;
	while (pSubscriptionClient->m_bMonitoredItemsNotificationThread)
	{
		dwStart=GetTickCount();
		if (!pFunc)
			pFunc=pSubscriptionClient->GetNotificationCallback();
		if (pFunc)
		{
			if (!pCallbackData)
				pCallbackData=pSubscriptionClient->GetNotificationCallbackData();
			if (pCallbackData)
			{
				// Notification au client
				OpcUa_Int32 iNoOfMonitoredItems=pSubscriptionClient->m_pMonitoredItemsNotificationList->size();
				iNoOfMonitoredItems=pSubscriptionClient->m_pMonitoredItemsNotificationList->size();
				OpcUa_Mutex_Lock(pSubscriptionClient->m_MonitoredItemsNotificationListMutex);
				for (OpcUa_UInt32 ii=0;ii<pSubscriptionClient->m_pMonitoredItemsNotificationList->size();ii++)
				{
					CMonitoredItemsNotification* pOpenOpcUaMonitoredItems=pSubscriptionClient->m_pMonitoredItemsNotificationList->at(ii);
					if (pOpenOpcUaMonitoredItems->IsDone()==OpcUa_False)
					{
						OpcUa_Mutex_Unlock(pSubscriptionClient->m_MonitoredItemsNotificationListMutex);
						OpcUa_MonitoredItemNotification* pMonitoredItemNotification=pOpenOpcUaMonitoredItems->GetMonitoredItemNotification();
						OpcUa_Int32 iNoOfMonitoredItems = pOpenOpcUaMonitoredItems->GetNoOfMonitoredItems();
						uStatus=pFunc((OpcUa_Handle)pSubscriptionClient,
							iNoOfMonitoredItems,
							pMonitoredItemNotification,
							pCallbackData);
						OpcUa_Mutex_Lock(pSubscriptionClient->m_MonitoredItemsNotificationListMutex);
						// Si l'application qui utilise la DLL a bien reçu la notification.
						// On marque cette notification comme traitée <--> bDone=OpcUa_True
						// Elle sera supprimée en fin de traitement
						if (uStatus==OpcUa_Good)
							pOpenOpcUaMonitoredItems->Done();
					}
				}
				OpcUa_Mutex_Unlock(pSubscriptionClient->m_MonitoredItemsNotificationListMutex);
			}
		}
		// On efface la collection des notifications traité. C'est a dire celle marquée <--> bDone=OpcUa_True
		pSubscriptionClient->CleanMonitoredItemNotificationQueue();
		DWORD dwEnd=GetTickCount();
		DWORD dwCountedTime=dwEnd-dwStart;
		if (dwInterval>dwCountedTime)
			dwSleepTime=dwInterval-dwCountedTime;
		else
			dwSleepTime=0;
		OpcUa_Semaphore_TimedWait(pSubscriptionClient->m_hMonitoredItemsNotificationSem, dwSleepTime);
	}
	OpcUa_Semaphore_Post(pSubscriptionClient->m_hStopMonitoredItemsNotificationSem,1);
	OpcUa_Trace(OPCUA_TRACE_CLIENT_LEVEL_ERROR, "MonitoredItemsNotificationThread stopped\n");
}
OpcUa_StatusCode CSubscriptionClient::StopMonitoredItemsNotificationThread()
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	if (m_bMonitoredItemsNotificationThread)
	{
		m_bMonitoredItemsNotificationThread = OpcUa_False;
		OpcUa_Mutex_Lock(m_MonitoredItemsNotificationListMutex);
		//OpcUa_StatusCode uStatus = OpcUa_Semaphore_TimedWait( m_hMonitoredItemsNotificationThreadStoppable,OPC_TIMEOUT*5);
		OpcUa_Semaphore_Post(m_hMonitoredItemsNotificationSem, 1);
		uStatus = OpcUa_Semaphore_TimedWait(m_hStopMonitoredItemsNotificationSem, OPC_TIMEOUT * 2); // 15 secondes max.
		if (uStatus == OpcUa_GoodNonCriticalTimeout)
		{
			// on force la fin du thread de simulation
			OpcUa_Trace(OPCUA_TRACE_CLIENT_LEVEL_ERROR, "Impossible to stop the MonitoredItemsNotificationThread. Timeout\n");
			//OpcUa_Thread_Delete(&m_hMonitoredItemsNotificationThread);
		}
		else
		{
			OpcUa_Thread_Delete(m_hMonitoredItemsNotificationThread);
			//m_hMonitoredItemsNotificationThread = OpcUa_Null;
			OpcUa_Trace(OPCUA_TRACE_CLIENT_LEVEL_ERROR, "MonitoredItemsNotificationThread stopped properly\n");
		}
		OpcUa_Mutex_Unlock(m_MonitoredItemsNotificationListMutex);
	}
	return uStatus;
}

/// <summary>
/// Sets the monitoring mode. This method change the Monitoring mode for hMonitoredItems list in this subscription
/// The new monitoring mode will be iMonitoringMode. It could be 
///													OpcUa_MonitoringMode_Disabled = 0,
///													OpcUa_MonitoringMode_Sampling = 1,
///													OpcUa_MonitoringMode_Reporting = 2
/// </summary>
/// <param name="iMonitoringMode">The i monitoring mode.</param>
/// <returns></returns>
OpcUa_StatusCode CSubscriptionClient::SetMonitoringMode(OpcUa_MonitoringMode iMonitoringMode, OpcUa_Int32 iNoOfMonitoredItems, OpcUa_UInt32* hMonitoredItems)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	if (m_pChannel)
	{
		OpcUa_RequestHeader tRequestHeader;
		OpcUa_RequestHeader_Initialize(&tRequestHeader);
		tRequestHeader.TimeoutHint = g_ServiceCallTimeout;// UTILS_DEFAULT_TIMEOUT;
		tRequestHeader.Timestamp   = OpcUa_DateTime_UtcNow();
		OpcUa_NodeId* pNodeId = GetSession()->GetAuthenticationToken();
		OpcUa_NodeId_CopyTo(pNodeId,&(tRequestHeader.AuthenticationToken));
		tRequestHeader.RequestHandle = CClientApplication::m_uiRequestHandle++;
		OpcUa_Int32 iNoOfResults=0;
		OpcUa_StatusCode* pResults=(OpcUa_StatusCode*)OpcUa_Alloc(sizeof(OpcUa_StatusCode));
		OpcUa_StatusCode_Initialize(pResults);
		OpcUa_Int32 iNoOfDiag=0;
		OpcUa_DiagnosticInfo* pDiag = OpcUa_Null;// (OpcUa_DiagnosticInfo*)OpcUa_Alloc(sizeof(OpcUa_DiagnosticInfo));
		OpcUa_DiagnosticInfo_Initialize(pDiag);
		// 
		OpcUa_Int32 nNumMonitoredItems=m_pMonitoredItemList->size();
		if (nNumMonitoredItems)
		{
			OpcUa_UInt32* pMonitoredItemIds = (OpcUa_UInt32*)OpcUa_Alloc(sizeof(OpcUa_UInt32)*iNoOfMonitoredItems);
			// Let's prepare the list of monitored items to change
			for (OpcUa_Int32 ii = 0; ii < iNoOfMonitoredItems; ii++)
			{
				CMonitoredItemBase* pMonitoredItem = FindMonitoredItemByHandle(hMonitoredItems[ii]);
				if (pMonitoredItem)
					pMonitoredItemIds[ii] = pMonitoredItem->GetMonitoredItemId();
			}

			OpcUa_ResponseHeader tResponseHeader;
			OpcUa_ResponseHeader_Initialize(&tResponseHeader);
			OpcUa_Channel hChannel = m_pChannel->GetInternalHandle();
			uStatus = OpcUa_ClientApi_SetMonitoringMode(hChannel,&tRequestHeader, GetSubscriptionId(),
														(OpcUa_MonitoringMode)iMonitoringMode,
														nNumMonitoredItems, pMonitoredItemIds,
														&tResponseHeader,&iNoOfResults, &pResults,
														&iNoOfDiag, &pDiag);
			if (uStatus==OpcUa_Good)
				OpcUa_ResponseHeader_Clear(&tResponseHeader);
			OpcUa_RequestHeader_Clear(&tRequestHeader);

			OpcUa_Free(pMonitoredItemIds);
		}
		else
			uStatus = OpcUa_BadInvalidArgument;
		if (pResults)
			OpcUa_Free(pResults);
		if (pDiag)
			OpcUa_Free(pDiag);
	}
	else
		uStatus = OpcUa_BadInternalError;
	return uStatus;
}
OpcUa_StatusCode CSubscriptionClient::SetPublishingMode(OpcUa_Boolean bMode)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_Channel           hChannel=m_pChannel->GetInternalHandle();
	OpcUa_RequestHeader		tRequestHeader;
	OpcUa_UInt32            nSubscriptionId=GetSubscriptionId();
	OpcUa_Int32			aNumOfResults=0;
	OpcUa_StatusCode*		pResults=OpcUa_Null;
	OpcUa_Int32			aNumOfDiagInfos=0;
	OpcUa_DiagnosticInfo*	pDiagInfos=OpcUa_Null;

	OpcUa_ResponseHeader	tResponseHeader;
	OpcUa_RequestHeader_Initialize(&tRequestHeader);
	tRequestHeader.TimeoutHint = g_ServiceCallTimeout;// UTILS_DEFAULT_TIMEOUT;
	tRequestHeader.Timestamp   = OpcUa_DateTime_UtcNow();
	OpcUa_NodeId* pNodeId = GetSession()->GetAuthenticationToken();
	OpcUa_NodeId_CopyTo(pNodeId, &(tRequestHeader.AuthenticationToken));
	tRequestHeader.RequestHandle = CClientApplication::m_uiRequestHandle++;
	//
	OpcUa_ResponseHeader_Initialize(&tResponseHeader);
	uStatus = OpcUa_ClientApi_SetPublishingMode(
		hChannel,
		&tRequestHeader,
		bMode,
		1,
		&nSubscriptionId,
		&tResponseHeader,
		&aNumOfResults,
		&pResults,
		&aNumOfDiagInfos,
		&pDiagInfos);
	if (uStatus == OpcUa_Good)
		SetPublishingEnabled(bMode);
	if (pResults)
		OpcUa_Free(pResults);
	OpcUa_NodeId_Clear(&(tRequestHeader.AuthenticationToken));
	OpcUa_ResponseHeader_Clear(&tResponseHeader);
	return uStatus;
}
OpcUa_StatusCode CSubscriptionClient::ModifySubscription(OpcUa_Double nRequestedPublishingInterval,
														 OpcUa_UInt32 nRequestedLifetimeCount,
														 OpcUa_UInt32 nRequestedMaxKeepAliveCount,
														 OpcUa_UInt32 nMaxNotificationsPerPublish,
														 OpcUa_Byte   nPriority)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_Channel           hChannel=m_pChannel->GetInternalHandle();
	OpcUa_RequestHeader		tRequestHeader;
	OpcUa_UInt32            nSubscriptionId=GetSubscriptionId();
	OpcUa_ResponseHeader	tResponseHeader;
	OpcUa_Double            tRevisedPublishingInterval;
	OpcUa_UInt32            tRevisedLifetimeCount;
	OpcUa_UInt32            tRevisedMaxKeepAliveCount;

	OpcUa_RequestHeader_Initialize(&tRequestHeader);
	tRequestHeader.TimeoutHint = g_ServiceCallTimeout;// UTILS_DEFAULT_TIMEOUT;
	tRequestHeader.Timestamp   = OpcUa_DateTime_UtcNow();
	OpcUa_NodeId* pNodeId = GetSession()->GetAuthenticationToken();
	OpcUa_NodeId_CopyTo(pNodeId, &(tRequestHeader.AuthenticationToken));
	tRequestHeader.RequestHandle = CClientApplication::m_uiRequestHandle++;
	//
	OpcUa_ResponseHeader_Initialize(&tResponseHeader);

	uStatus=OpcUa_ClientApi_ModifySubscription(
		hChannel,
		&tRequestHeader,
		nSubscriptionId,
		nRequestedPublishingInterval,
		nRequestedLifetimeCount,
		nRequestedMaxKeepAliveCount,
		nMaxNotificationsPerPublish,
		nPriority,
		&tResponseHeader, // out
		&tRevisedPublishingInterval, // out
		&tRevisedLifetimeCount, // out
		&tRevisedMaxKeepAliveCount); // out

	if (uStatus==OpcUa_Good)
	{
		// Tranfert resultats
		SetLifetimeCount(tRevisedLifetimeCount);
		SetPublishingInterval(tRevisedPublishingInterval);
		SetMaxKeepAliveCount(tRevisedMaxKeepAliveCount);
		SetPriority(nPriority);
		SetMaxNotificationsPerPublish(nMaxNotificationsPerPublish);
	}
	return uStatus;
}
void CSubscriptionClient::AddSequenceNumber(OpcUa_UInt32 uiVal)
{
	if (!IsInSequenceNumber(uiVal))
	{
		OpcUa_Mutex_Lock(m_MonitoredItemListMutex);
		m_vectOfAvailableSequenceNumber.push_back(uiVal);
		OpcUa_Mutex_Unlock(m_MonitoredItemListMutex);
	}
}
OpcUa_Boolean CSubscriptionClient::IsInSequenceNumber(OpcUa_UInt32 uiVal)
{
	OpcUa_Boolean bResult = OpcUa_False;
	OpcUa_Mutex_Lock(m_MonitoredItemListMutex);
	for (OpcUa_UInt32 i = 0; i < m_vectOfAvailableSequenceNumber.size(); i++)
	{
		if (m_vectOfAvailableSequenceNumber.at(i) == uiVal)
		{
			bResult = OpcUa_True;
			break;
		}
	}
	OpcUa_Mutex_Unlock(m_MonitoredItemListMutex);
	return bResult;
}
void CSubscriptionClient::SequenceNumberClearAll()
{
	OpcUa_Mutex_Lock(m_MonitoredItemListMutex);
	m_vectOfAvailableSequenceNumber.clear();
	OpcUa_Mutex_Unlock(m_MonitoredItemListMutex);
}
OpcUa_StatusCode CSubscriptionClient::AddMonitoredItem(CMonitoredItemClient* pMonitoredItem)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	if (pMonitoredItem)
	{

		CMonitoredItemClientList* pMonitoredItemList =GetMonitoredItemList();
		if (pMonitoredItemList)
		{
			OpcUa_Mutex_Lock(m_MonitoredItemListMutex);
			pMonitoredItemList->push_back(pMonitoredItem);
			OpcUa_Mutex_Unlock(m_MonitoredItemListMutex);
		}
	}
	else
		uStatus=OpcUa_BadInvalidArgument;
	return uStatus;
}
OpcUa_StatusCode CSubscriptionClient::DeleteMonitoredItemById(OpcUa_UInt32 Id)
{
	OpcUa_StatusCode uStatus = OpcUa_Bad;
	CMonitoredItemClient* pItem = FindMonitoredItemById(Id);
	if (pItem)
	{
		OpcUa_Mutex_Lock(m_MonitoredItemListMutex);		
		for (CMonitoredItemClientList::iterator it = m_pMonitoredItemList->begin(); it != m_pMonitoredItemList->end(); it++)
		{
			if (*it == pItem)
			{
				uStatus = DeleteMonitoredItem(pItem);
				if (uStatus == OpcUa_Good)
				{
					delete pItem;
					m_pMonitoredItemList->erase(it);
				}
				break;
			}
		}
		OpcUa_Mutex_Unlock(m_MonitoredItemListMutex);
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}

OpcUa_StatusCode CSubscriptionClient::DeleteMonitoredItems(CMonitoredItemClientList* pMonitoredItemList)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_RequestHeader		tRequestHeader;
	OpcUa_ResponseHeader*	pResponseHeader = (OpcUa_ResponseHeader*)OpcUa_Alloc(sizeof(OpcUa_ResponseHeader));
	OpcUa_Int32				NoOfResults = 0;
	OpcUa_StatusCode*		pResults = OpcUa_Null;
	OpcUa_Int32             nNoOfDiagnosticInfos = 0;
	OpcUa_DiagnosticInfo*   pDiagnosticInfos = 0;
	OpcUa_UInt32			nSubscriptionId = GetSubscriptionId();
	OpcUa_Int32				nNoOfMonitoredItemIds = -1;
	OpcUa_UInt32*			pMonitoredItemIds = OpcUa_Null;

	OpcUa_RequestHeader_Initialize(&tRequestHeader);
	// fill in request header.
	tRequestHeader.TimeoutHint = CLIENTLIB_DEFAULT_TIMEOUT; 
	tRequestHeader.Timestamp = OpcUa_DateTime_UtcNow();
	OpcUa_NodeId* pAuthenticationToken=m_pSession->GetAuthenticationToken();
	OpcUa_NodeId_CopyTo(pAuthenticationToken, &(tRequestHeader.AuthenticationToken));
	tRequestHeader.RequestHandle = CClientApplication::m_uiRequestHandle++;
	OpcUa_ResponseHeader_Initialize(pResponseHeader);
	if (pMonitoredItemList)
	{
		OpcUa_Mutex_Lock(m_MonitoredItemListMutex);
		nNoOfMonitoredItemIds = pMonitoredItemList->size();
		if (nNoOfMonitoredItemIds > 0)
		{
			pMonitoredItemIds = (OpcUa_UInt32*)OpcUa_Alloc(sizeof(OpcUa_UInt32)*nNoOfMonitoredItemIds);
			if (pMonitoredItemIds)
			{
				for (OpcUa_Int32 i = 0; i < nNoOfMonitoredItemIds; i++)
				{
					CMonitoredItemClient* pMonitoredItemClient = pMonitoredItemList->at(i);
					pMonitoredItemIds[i] = pMonitoredItemClient->GetMonitoredItemId();
				}
				// Now let's ask to server to delete those items
				CChannel* pChannel = m_pSession->GetChannel();
				if (pChannel)
				{
					OpcUa_Handle hChannel = pChannel->GetInternalHandle();
					uStatus = OpcUa_ClientApi_DeleteMonitoredItems(
						hChannel,
						&tRequestHeader,
						nSubscriptionId,
						nNoOfMonitoredItemIds,
						pMonitoredItemIds,
						pResponseHeader,
						&NoOfResults,
						&pResults,
						&nNoOfDiagnosticInfos,
						&pDiagnosticInfos);
					if (uStatus == OpcUa_Good)
					{
						if (pResults)
						{
							// Delete the item itself
							CMonitoredItemClientList::iterator it;
							for (OpcUa_UInt32 ii = 0; ii < pMonitoredItemList->size(); ii++)
							{
								CMonitoredItemClient* pMonitoredItemClient = pMonitoredItemList->at(ii);

								for (it = m_pMonitoredItemList->begin(); it != m_pMonitoredItemList->end(); it++)
								{
									if (pResults[ii] == OpcUa_Good)
									{
										if (pMonitoredItemClient == (*it))
										{
											delete pMonitoredItemClient;
											m_pMonitoredItemList->erase(it);
											break;
										}
									}
								}
							}
						}
						pMonitoredItemList->clear();
						if (pResults)
							OpcUa_Free(pResults);
					}
				}
				else
					uStatus = OpcUa_BadNotConnected;
				OpcUa_Free(pMonitoredItemIds);
			}
			else
				uStatus = OpcUa_BadOutOfMemory;
		}
		OpcUa_Mutex_Unlock(m_MonitoredItemListMutex);
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	OpcUa_RequestHeader_Clear(&tRequestHeader);
	OpcUa_Free(pResponseHeader);
	return uStatus;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Invoke the Server service to delete th monitoredItem. </summary>
///
/// <remarks>	Michel, 16/03/2016. </remarks>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode CSubscriptionClient::DeleteMonitoredItems()
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	uStatus=DeleteMonitoredItems(m_pMonitoredItemList);
	return uStatus;
}
OpcUa_StatusCode CSubscriptionClient::DeleteMonitoredItem(CMonitoredItemClient* pMonitoredItemClient)
{
	OpcUa_StatusCode		uStatus=OpcUa_Good;
	OpcUa_RequestHeader		tRequestHeader;
	OpcUa_ResponseHeader*	pResponseHeader = (OpcUa_ResponseHeader*)OpcUa_Alloc(sizeof(OpcUa_ResponseHeader));
	OpcUa_Int32				NoOfResults=0;
	OpcUa_StatusCode*		pResults = OpcUa_Null;
	OpcUa_Int32             nNoOfDiagnosticInfos = 0;
	OpcUa_DiagnosticInfo*   pDiagnosticInfos = 0;
	OpcUa_UInt32			nSubscriptionId = GetSubscriptionId();
	OpcUa_Int32				nNoOfMonitoredItemIds = 1;
	OpcUa_UInt32*			pMonitoredItemIds = (OpcUa_UInt32*)OpcUa_Alloc(sizeof(OpcUa_UInt32)*nNoOfMonitoredItemIds);
	pMonitoredItemIds[0] = pMonitoredItemClient->GetMonitoredItemId();
	OpcUa_RequestHeader_Initialize(&tRequestHeader);
	// fill in request header.
	tRequestHeader.TimeoutHint = g_ServiceCallTimeout;// UTILS_DEFAULT_TIMEOUT;
	tRequestHeader.Timestamp = OpcUa_DateTime_UtcNow();

	OpcUa_ResponseHeader_Initialize(pResponseHeader);
	if (m_pSession)
	{
		OpcUa_NodeId* pNodeId = GetSession()->GetAuthenticationToken();
		OpcUa_NodeId_CopyTo(pNodeId, &(tRequestHeader.AuthenticationToken));
		tRequestHeader.RequestHandle = CClientApplication::m_uiRequestHandle++;
		CChannel* pChannel = m_pSession->GetChannel();
		if (pChannel)
		{
			OpcUa_Handle hChannel = pChannel->GetInternalHandle();
			uStatus = OpcUa_ClientApi_DeleteMonitoredItems(
				hChannel,
				&tRequestHeader,
				nSubscriptionId,
				nNoOfMonitoredItemIds,
				pMonitoredItemIds,
				pResponseHeader,
				&NoOfResults,
				&pResults,
				&nNoOfDiagnosticInfos,
				& pDiagnosticInfos);
		}
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	if (pMonitoredItemIds)
		OpcUa_Free(pMonitoredItemIds);
	if (pResponseHeader)
	{
		OpcUa_ResponseHeader_Clear(pResponseHeader);
		OpcUa_Free(pResponseHeader);
	}
	OpcUa_RequestHeader_Clear(&tRequestHeader);
	Utils_ClearArray(pResults, NoOfResults, OpcUa_StatusCode);
	Utils_ClearArray(pDiagnosticInfos, nNoOfDiagnosticInfos, OpcUa_DiagnosticInfo);
	return uStatus;
}
CMonitoredItemClient* CSubscriptionClient::FindMonitoredItemById(OpcUa_UInt32 Id)
{
	for (OpcUa_UInt32 ii=0;ii<m_pMonitoredItemList->size();ii++)
	{
		CMonitoredItemClient* pItem=m_pMonitoredItemList->at(ii);
		if (pItem->GetMonitoredItemId()==Id)
			return pItem;
	}
	return NULL;
}

CMonitoredItemClient* CSubscriptionClient::FindMonitoredItemByHandle(OpcUa_UInt32 uiClientHandle)
{
	CMonitoredItemClient* pMonitoredItemClient=OpcUa_Null;
	for (OpcUa_UInt32 ii=0;ii<m_pMonitoredItemList->size();ii++)
	{
		CMonitoredItemClient* pItem=m_pMonitoredItemList->at(ii);
		if (pItem->GetClientHandle() == uiClientHandle)
		{
			return pItem;
			break;
		}
	}
	return pMonitoredItemClient ;
}

void CSubscriptionClient::UpdateSubscriptionDiagnosticsDataType()
{
	m_pSubscriptionDiagnosticsDataType->SetSessionId(GetSession()->GetSessionId());
}

OpcUa_StatusCode CSubscriptionClient::AddMonitoredItemNotification(CMonitoredItemsNotification* pMonitoredItemsNotification)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_Mutex_Lock(m_MonitoredItemsNotificationListMutex);
	m_pMonitoredItemsNotificationList->push_back(pMonitoredItemsNotification);
	OpcUa_Mutex_Unlock(m_MonitoredItemsNotificationListMutex);
	return uStatus;
}
OpcUa_StatusCode CSubscriptionClient::CleanMonitoredItemNotificationQueue()
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	CMonitoredItemsNotificationList tmpMonitoredItemsNotificationList;
	CMonitoredItemsNotificationList::iterator it;
	OpcUa_Mutex_Lock(m_MonitoredItemsNotificationListMutex);
	if (m_pMonitoredItemsNotificationList->size()>0)
	{
		for (it = m_pMonitoredItemsNotificationList->begin(); it != m_pMonitoredItemsNotificationList->end();it++)
		{
			CMonitoredItemsNotification* pMonitoredItemsNotification = *it;// (CMonitoredItemsNotification*)*(m_pMonitoredItemsNotificationList->begin());
			if (pMonitoredItemsNotification->IsDone())
				delete pMonitoredItemsNotification;
			else
				tmpMonitoredItemsNotificationList.push_back(pMonitoredItemsNotification);
		}
		m_pMonitoredItemsNotificationList->clear();
		m_pMonitoredItemsNotificationList->swap(tmpMonitoredItemsNotificationList);
	}
	OpcUa_Mutex_Unlock(m_MonitoredItemsNotificationListMutex);
	return uStatus;
}
OpcUa_UInt32*  CSubscriptionClient::GetAvailableSequenceNumbers()
{
	OpcUa_Mutex_Lock(m_MonitoredItemsNotificationListMutex);
	if (m_pAvailableSequenceNumbers)
	{
		OpcUa_Free(m_pAvailableSequenceNumbers);
		m_pAvailableSequenceNumbers = OpcUa_Null;
	}
	m_NoOfAvailableSequenceNumbers=0;
	// tranfert de m_vectOfAvailableSequenceNumber dans m_pAvailableSequenceNumbers
	m_pAvailableSequenceNumbers=(OpcUa_UInt32*)OpcUa_Alloc(sizeof(OpcUa_UInt32)*m_vectOfAvailableSequenceNumber.size());
	for (OpcUa_UInt32 ii=0;ii<m_vectOfAvailableSequenceNumber.size();ii++)
	{
		OpcUa_UInt32 uiSeq=m_vectOfAvailableSequenceNumber.at(ii);
		m_pAvailableSequenceNumbers[ii]=uiSeq;
	}
	m_NoOfAvailableSequenceNumbers=m_vectOfAvailableSequenceNumber.size();
	OpcUa_Mutex_Unlock(m_MonitoredItemsNotificationListMutex);
	return m_pAvailableSequenceNumbers;
}
OpcUa_Int32 CSubscriptionClient::GetNoOfAvailableSequenceNumbers()
{
	m_NoOfAvailableSequenceNumbers = m_vectOfAvailableSequenceNumber.size();
	return  m_NoOfAvailableSequenceNumbers;
}
/// <summary>
/// Gets the subscription as a list of XML attributes.
/// </summary>
/// <param name="atts">The atts.</param>
/// <returns></returns>
OpcUa_StatusCode CSubscriptionClient::GetXmlAttributes(vector<OpcUa_String*>* szAttributes)
{
	OpcUa_StatusCode uStatus = OpcUa_BadNotFound;
	OpcUa_String* pszLocalAtts = OpcUa_Null;
	OpcUa_String szTmpstring;
	OpcUa_String_Initialize(&szTmpstring);
	//vector<OpcUa_String*> szAttributes;
	char* buf = OpcUa_Null;

	////////////////////////////////////////////////////
	// PublishInterval double
	// Attribute
	OpcUa_String_AttachCopy(&szTmpstring, "PublishInterval");
	pszLocalAtts = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszLocalAtts);
	OpcUa_String_StrnCpy(pszLocalAtts, &szTmpstring, OpcUa_String_StrLen(&szTmpstring));
	szAttributes->push_back(pszLocalAtts);
	// Attribute value
	OpcUa_String_Clear(&szTmpstring);
	OpcUa_String szPublishingInterval;
	OpcUa_String_Initialize(&szPublishingInterval);
	buf = (char*)malloc(20);
	if (buf)
	{
		ZeroMemory(buf, 20);
		sprintf(buf, "%lf", m_pRevisedPublishingInterval);
		OpcUa_String_AttachCopy(&szPublishingInterval, buf);
		OpcUa_Free(buf);
	}
	OpcUa_String_StrnCpy(&szTmpstring, &szPublishingInterval, OpcUa_String_StrLen(&szPublishingInterval));
	pszLocalAtts = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszLocalAtts);
	OpcUa_String_StrnCpy(pszLocalAtts, &szTmpstring, OpcUa_String_StrLen(&szTmpstring));
	szAttributes->push_back(pszLocalAtts);

	OpcUa_String_Clear(&szTmpstring);
	////////////////////////////////////////////////////
	// LifetimeCount uint
	// Attribute
	OpcUa_String_AttachCopy(&szTmpstring, "LifetimeCount");
	pszLocalAtts = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszLocalAtts);
	OpcUa_String_StrnCpy(pszLocalAtts, &szTmpstring, OpcUa_String_StrLen(&szTmpstring));
	szAttributes->push_back(pszLocalAtts);
	// Attribute value
	OpcUa_String_Clear(&szTmpstring);
	
	OpcUa_String szLifetimeCount;
	OpcUa_String_Initialize(&szLifetimeCount);
	buf = (char*)malloc(20);
	if (buf)
	{
		ZeroMemory(buf, 20);
		sprintf(buf, "%u", (unsigned int)m_pRevisedLifetimeCount);
		OpcUa_String_AttachCopy(&szLifetimeCount, buf);
		OpcUa_Free(buf);
	}
	OpcUa_String_StrnCpy(&szTmpstring, &szLifetimeCount, OpcUa_String_StrLen(&szLifetimeCount));
	pszLocalAtts = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszLocalAtts);
	OpcUa_String_StrnCpy(pszLocalAtts, &szTmpstring, OpcUa_String_StrLen(&szTmpstring));
	szAttributes->push_back(pszLocalAtts);

	OpcUa_String_Clear(&szTmpstring);
	////////////////////////////////////////////////////
	// MaxKeepaliveCount uint
	// Attribute
	OpcUa_String_AttachCopy(&szTmpstring, "MaxKeepaliveCount");
	pszLocalAtts = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszLocalAtts);
	OpcUa_String_StrnCpy(pszLocalAtts, &szTmpstring, OpcUa_String_StrLen(&szTmpstring));
	szAttributes->push_back(pszLocalAtts);
	// Attribute value
	OpcUa_String_Clear(&szTmpstring);
	
	OpcUa_String szMaxKeepaliveCount;
	OpcUa_String_Initialize(&szMaxKeepaliveCount);
	buf = (char*)malloc(20);
	if (buf)
	{
		ZeroMemory(buf, 20);
		sprintf(buf, "%u", (unsigned int)m_pRevisedMaxKeepAliveCount);
		OpcUa_String_AttachCopy(&szMaxKeepaliveCount, buf);
		OpcUa_Free(buf);
	}
	OpcUa_String_StrnCpy(&szTmpstring, &szMaxKeepaliveCount, OpcUa_String_StrLen(&szMaxKeepaliveCount));
	pszLocalAtts = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszLocalAtts);
	OpcUa_String_StrnCpy(pszLocalAtts, &szTmpstring, OpcUa_String_StrLen(&szTmpstring));
	szAttributes->push_back(pszLocalAtts);

	OpcUa_String_Clear(&szTmpstring);
	////////////////////////////////////////////////////
	// PublishEnable bool
	// Attribute
	OpcUa_String_AttachCopy(&szTmpstring, "PublishEnable");
	pszLocalAtts = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszLocalAtts);
	OpcUa_String_StrnCpy(pszLocalAtts, &szTmpstring, OpcUa_String_StrLen(&szTmpstring));
	szAttributes->push_back(pszLocalAtts);
	// Attribute value
	OpcUa_String_Clear(&szTmpstring);	
	
	OpcUa_String szPublishEnable;
	OpcUa_String_Initialize(&szPublishEnable);
	buf = (char*)malloc(20);
	if (buf)
	{
		ZeroMemory(buf, 20);
		if (m_bPublishingEnabled)
			memcpy(buf, "true",4);
		else
			memcpy(buf, "false",5);
		OpcUa_String_AttachCopy(&szPublishEnable, buf);
		OpcUa_Free(buf);
	}
	OpcUa_String_StrnCpy(&szTmpstring, &szPublishEnable, OpcUa_String_StrLen(&szPublishEnable));
	pszLocalAtts = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszLocalAtts);
	OpcUa_String_StrnCpy(pszLocalAtts, &szTmpstring, OpcUa_String_StrLen(&szTmpstring));
	szAttributes->push_back(pszLocalAtts);
	////////////////////////////////////////////////////
	// m_uiMaxNotificationsPerPublish OpcUa_UInt32
	// Attribute
	OpcUa_String_AttachCopy(&szTmpstring, "MaxNotificationsPerPublish");
	pszLocalAtts = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszLocalAtts);
	OpcUa_String_StrnCpy(pszLocalAtts, &szTmpstring, OpcUa_String_StrLen(&szTmpstring));
	szAttributes->push_back(pszLocalAtts);
	// Attribute value
	OpcUa_String_Clear(&szTmpstring);

	OpcUa_String szMaxNotificationsPerPublish;
	OpcUa_String_Initialize(&szMaxNotificationsPerPublish);
	buf = (char*)malloc(20);
	if (buf)
	{
		ZeroMemory(buf, 20);
		sprintf(buf, "%u", m_uiMaxNotificationsPerPublish);
		OpcUa_String_AttachCopy(&szMaxNotificationsPerPublish, buf);
		OpcUa_Free(buf);
	}
	OpcUa_String_StrnCpy(&szTmpstring, &szMaxNotificationsPerPublish, OpcUa_String_StrLen(&szMaxNotificationsPerPublish));
	pszLocalAtts = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszLocalAtts);
	OpcUa_String_StrnCpy(pszLocalAtts, &szTmpstring, OpcUa_String_StrLen(&szTmpstring));
	szAttributes->push_back(pszLocalAtts);


	////////////////////////////////////////////////////
	// m_Priority OpcUa_Byte
	// Attribute
	OpcUa_String_AttachCopy(&szTmpstring, "Priority");
	pszLocalAtts = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszLocalAtts);
	OpcUa_String_StrnCpy(pszLocalAtts, &szTmpstring, OpcUa_String_StrLen(&szTmpstring));
	szAttributes->push_back(pszLocalAtts);
	// Attribute value
	OpcUa_String_Clear(&szTmpstring);

	OpcUa_String szPriority;
	OpcUa_String_Initialize(&szPriority);
	buf = (char*)malloc(20);
	if (buf)
	{
		ZeroMemory(buf, 20);
		sprintf(buf, "%u", m_Priority);
		OpcUa_String_AttachCopy(&szPriority, buf);
		OpcUa_Free(buf);
	}
	OpcUa_String_StrnCpy(&szTmpstring, &szPriority, OpcUa_String_StrLen(&szPriority));
	pszLocalAtts = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszLocalAtts);
	OpcUa_String_StrnCpy(pszLocalAtts, &szTmpstring, OpcUa_String_StrLen(&szTmpstring));
	szAttributes->push_back(pszLocalAtts);

	////////////////////////////////////////////////////
	// m_iMonitoringMode enum in a OpcUa_Int32 
	// Attribute
	OpcUa_String_AttachCopy(&szTmpstring, "MonitoringMode");
	pszLocalAtts = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszLocalAtts);
	OpcUa_String_StrnCpy(pszLocalAtts, &szTmpstring, OpcUa_String_StrLen(&szTmpstring));
	szAttributes->push_back(pszLocalAtts);
	// Attribute value
	OpcUa_String_Clear(&szTmpstring);

	OpcUa_String szMonitoringMode;
	OpcUa_String_Initialize(&szMonitoringMode);
	buf = (char*)malloc(20);
	if (buf)
	{
		ZeroMemory(buf, 20);
		sprintf(buf, "%u", m_iMonitoringMode);
		OpcUa_String_AttachCopy(&szMonitoringMode, buf);
		OpcUa_Free(buf);
	}
	OpcUa_String_StrnCpy(&szTmpstring, &szMonitoringMode, OpcUa_String_StrLen(&szMonitoringMode));
	pszLocalAtts = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszLocalAtts);
	OpcUa_String_StrnCpy(pszLocalAtts, &szTmpstring, OpcUa_String_StrLen(&szTmpstring));
	szAttributes->push_back(pszLocalAtts);

	//if (pszLocalAtts)
	//	OpcUa_String_Clear(pszLocalAtts);
	OpcUa_String_Clear(&szTmpstring);

	return uStatus;
}

OpcUa_StatusCode CSubscriptionClient::GetMonitoredItems(OpcUa_UInt32* uiNoOfMonitoredItems, OpcUa_Handle** hMonitoredItems)
{
	OpcUa_StatusCode uStatus = OpcUa_BadInvalidArgument;
	if (hMonitoredItems)
	{
		OpcUa_Mutex_Lock(m_MonitoredItemListMutex);
		*uiNoOfMonitoredItems = m_pMonitoredItemList->size();
		if ((*uiNoOfMonitoredItems) > 0)
		{
			*hMonitoredItems = (OpcUa_Handle*)OpcUa_Alloc(sizeof(OpcUa_Handle)*(*uiNoOfMonitoredItems));
			ZeroMemory(*hMonitoredItems, sizeof(OpcUa_Handle)*(*uiNoOfMonitoredItems));
			for (OpcUa_UInt32 ii = 0; ii < m_pMonitoredItemList->size(); ii++)
			{
				CMonitoredItemClient* pMonitoredItemClient = m_pMonitoredItemList->at(ii);
				if (pMonitoredItemClient)
					(*hMonitoredItems)[ii] = (OpcUa_Handle)pMonitoredItemClient;
				else
					(*hMonitoredItems)[ii] = OpcUa_Null;
			}
			uStatus = OpcUa_Good;
		}
		else
			uStatus = OpcUa_BadNothingToDo;
		OpcUa_Mutex_Unlock(m_MonitoredItemListMutex);
	}
	return uStatus;
}
OpcUa_StatusCode CSubscriptionClient::DeleteFromMonitoredItemList(CMonitoredItemClient* pMonitoredItem)
{
	OpcUa_StatusCode uStatus = OpcUa_BadInvalidArgument;
	OpcUa_Mutex_Lock(m_MonitoredItemListMutex);
	for (CMonitoredItemClientList::iterator it = m_pMonitoredItemList->begin(); it != m_pMonitoredItemList->end(); it++)
	{
		uStatus = OpcUa_BadNotFound;
		if (*it == pMonitoredItem)
		{
			m_pMonitoredItemList->erase(it);
			uStatus = OpcUa_Good;
			break;
		}
	}
	OpcUa_Mutex_Unlock(m_MonitoredItemListMutex);
	return uStatus;
}
OpcUa_StatusCode CSubscriptionClient::ModifyMonitoredItems(OpcUa_Handle	hMonitoredItems,
															OpcUa_Byte		aTimestampsToReturn,
															OpcUa_Boolean	bDiscardOldest,
															OpcUa_UInt32	uiQueueSize,
															OpcUa_Double	dblSamplingInterval,
															OpcUa_UInt32	DeadbandType,
															OpcUa_Double	DeadbandValue,
															OpcUa_Byte		DatachangeTrigger)
{
	OpcUa_StatusCode					uStatus = OpcUa_BadInvalidArgument;
	OpcUa_RequestHeader					tRequestHeader;
	OpcUa_ResponseHeader				tResponseHeader;
	OpcUa_Int32							tNoOfResults = 0;
	OpcUa_Int32							tNoOfDiagnosticInfos = 0;
	OpcUa_DiagnosticInfo*				tDiagnosticInfos = OpcUa_Null;

	OpcUa_ResponseHeader_Initialize(&tResponseHeader);

	OpcUa_MonitoredItemModifyRequest* pMonitoredItemModifyRequest;
	OpcUa_MonitoredItemModifyResult* pMonitoredItemModifyResult = OpcUa_Null;
	OpcUa_RequestHeader_Initialize(&tRequestHeader);
	tRequestHeader.TimeoutHint = g_ServiceCallTimeout;// UTILS_DEFAULT_TIMEOUT;
	tRequestHeader.Timestamp = OpcUa_DateTime_UtcNow();

	pMonitoredItemModifyRequest = (OpcUa_MonitoredItemModifyRequest*)OpcUa_Alloc(sizeof(OpcUa_MonitoredItemModifyRequest));
	OpcUa_MonitoredItemModifyResult_Initialize(pMonitoredItemModifyResult);
	OpcUa_NodeId_CopyTo(m_pSession->GetAuthenticationToken(), &(tRequestHeader.AuthenticationToken));
	tRequestHeader.RequestHandle = CClientApplication::m_uiRequestHandle++;
	OpcUa_DiagnosticInfo_Initialize(tDiagnosticInfos);

	if (m_pChannel)
	{
		CMonitoredItemClient* pMonitoredItem = FindMonitoredItemByHandle((OpcUa_UInt32)(uintptr_t)hMonitoredItems);
		if (pMonitoredItem)
		{
			OpcUa_TimestampsToReturn eTimestampsToReturn = (OpcUa_TimestampsToReturn)aTimestampsToReturn;
			pMonitoredItemModifyRequest->MonitoredItemId = pMonitoredItem->GetMonitoredItemId();
			pMonitoredItemModifyRequest->RequestedParameters.ClientHandle = pMonitoredItem->GetClientHandle();
			pMonitoredItemModifyRequest->RequestedParameters.DiscardOldest = bDiscardOldest;
			pMonitoredItemModifyRequest->RequestedParameters.QueueSize = uiQueueSize;
			pMonitoredItemModifyRequest->RequestedParameters.SamplingInterval = dblSamplingInterval;
			OpcUa_ExtensionObject DataChangeFilter;
			OpcUa_ExtensionObject_Initialize(&DataChangeFilter);
			DataChangeFilter.TypeId.NodeId.Identifier.Numeric = OpcUaId_DataChangeFilter_Encoding_DefaultBinary;
			DataChangeFilter.Encoding = OpcUa_ExtensionObjectEncoding_EncodeableObject;
			DataChangeFilter.Body.EncodeableObject.Type = (OpcUa_EncodeableType*)OpcUa_Alloc(sizeof(OpcUa_EncodeableType));
			DataChangeFilter.Body.EncodeableObject.Type =&OpcUa_DataChangeFilter_EncodeableType;
			//DataChangeFilter.Body.EncodeableObject.Type->TypeId = OpcUaId_DataChangeFilter;
			OpcUa_DataChangeFilter* pDataChangeFilter = (OpcUa_DataChangeFilter*)OpcUa_Alloc(sizeof(OpcUa_DataChangeFilter));
			pDataChangeFilter->DeadbandType = DeadbandType;
			pDataChangeFilter->DeadbandValue = DeadbandValue;
			pDataChangeFilter->Trigger = (OpcUa_DataChangeTrigger)DatachangeTrigger;
			DataChangeFilter.Body.EncodeableObject.Object = (void*)pDataChangeFilter;
			pMonitoredItemModifyRequest->RequestedParameters.Filter = DataChangeFilter;
			OpcUa_Channel	hChannel = m_pChannel->GetInternalHandle();
			uStatus = OpcUa_ClientApi_ModifyMonitoredItems(hChannel,
				&tRequestHeader,
				m_pSubscriptionId,
				eTimestampsToReturn,
				1,
				pMonitoredItemModifyRequest,
				&tResponseHeader,
				&tNoOfResults,
				&pMonitoredItemModifyResult,
				&tNoOfDiagnosticInfos,
				&tDiagnosticInfos);
			if (uStatus == OpcUa_Good)
			{
				if (tNoOfResults > 0)
				{
					// update internal CMonitoredItemClient
					pMonitoredItem->DiscardOldest(bDiscardOldest);
					pMonitoredItem->SetFilterToUse(&DataChangeFilter);
					pMonitoredItem->SetQueueSize(pMonitoredItemModifyResult[0].RevisedQueueSize);
					pMonitoredItem->SetSamplingInterval(pMonitoredItemModifyResult[0].RevisedSamplingInterval);
					pMonitoredItem->SetTimestampsToReturn((OpcUa_TimestampsToReturn)aTimestampsToReturn);
				}
			}
			OpcUa_DataChangeFilter_Clear(pDataChangeFilter);
			//OpcUa_Free(pDataChangeFilter);
			OpcUa_ExtensionObject_Clear(&DataChangeFilter);
		}
		else
			uStatus = OpcUa_BadMonitoredItemIdInvalid;
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}