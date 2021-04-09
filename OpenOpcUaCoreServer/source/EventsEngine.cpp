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

using namespace OpenOpcUa;
using namespace UAEvents;
using namespace UAAddressSpace;
using namespace UACoreServer;
CEventsEngine::CEventsEngine()
{
	m_EventDefinitionIndex = 0;
	m_hEventsThread = OpcUa_Null;
	m_pEventDefinitionList = new CEventDefinitionList();
	OpcUa_Semaphore_Create(&m_EventsThreadSem, 0, 0x100);
	OpcUa_Mutex_Create(&m_pEventDefinitionListMutex);
	OpcUa_Mutex_Create(&m_hEventsThreadMutex);
	StartEventsThread();
}


CEventsEngine::~CEventsEngine()
{
	StopEventsThread();
	OpcUa_Semaphore_Delete(&m_EventsThreadSem);
	OpcUa_Mutex_Lock(m_pEventDefinitionListMutex);
	CEventDefinitionList::iterator itEvtL = m_pEventDefinitionList->begin();
	while (itEvtL!=m_pEventDefinitionList->end())
	{
		CEventDefinition* pEventDefinition = *itEvtL;
		delete pEventDefinition;
		pEventDefinition = OpcUa_Null;
		itEvtL++;
	}
	m_pEventDefinitionList->clear();
	delete m_pEventDefinitionList;
	m_pEventDefinitionList = OpcUa_Null;
	OpcUa_Mutex_Unlock(m_pEventDefinitionListMutex);
	OpcUa_Mutex_Delete(&m_pEventDefinitionListMutex);
	OpcUa_Mutex_Delete(&m_hEventsThreadMutex);
}
void CEventsEngine::StartEventsThread(void)
{
	if (m_hEventsThread == OpcUa_Null)
	{
		m_bRunEventsThread = OpcUa_True;
		OpcUa_StatusCode uStatus = OpcUa_Thread_Create(&m_hEventsThread, EventsThread, this);

		if (uStatus != OpcUa_Good)
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Create EventsThread Failed");
		else
			OpcUa_Thread_Start(m_hEventsThread);
	}
}
void CEventsEngine::StopEventsThread(void)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_Mutex_Lock(m_hEventsThreadMutex);
	if (m_bRunEventsThread)
	{
		m_bRunEventsThread = OpcUa_False;
		OpcUa_Semaphore_Post(m_EventsThreadSem, 1); // StopUpdateSubscriptionThread()
		uStatus = OpcUa_Semaphore_TimedWait(m_EventsThreadSem, OPC_TIMEOUT * 2); // INFINITE - OPC_TIMEOUT*2= 15 secondes max.
		if (uStatus == OpcUa_GoodNonCriticalTimeout)
		{
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Impossible to stop the EventsThread. Timeout\n");
		}
		else
		{
			OpcUa_Thread_Delete(m_hEventsThread);
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_WARNING, "EventsThread stopped properly\n");
		}
	}
	OpcUa_Mutex_Unlock(m_hEventsThreadMutex);
	return;
}
void CEventsEngine::EventsThread(LPVOID arg)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	CUAInformationModel* pInformationModel = CServerApplication::m_pTheAddressSpace;

	OpcUa_UInt32 uiSleepTime = 0;
	OpcUa_UInt32 dwInterval = 100;
	CEventsEngine* pEventsEngine = (CEventsEngine*)arg;
	//CUAInformationModel* pInformationModel = CServerApplication::m_pTheAddressSpace;
	OpcUa_Semaphore_TimedWait(pEventsEngine->m_EventsThreadSem, INFINITE);
	while (pEventsEngine->m_bRunEventsThread)
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
		// Go through the eventList if it exist
		OpcUa_Mutex_Lock(pEventsEngine->m_hEventsThreadMutex);
		OpcUa_Mutex_Lock(pEventsEngine->m_pEventDefinitionListMutex);
		if (pEventsEngine->m_pEventDefinitionList)
		{
			CEventDefinitionList::iterator it=pEventsEngine->m_pEventDefinitionList->begin();
			for (OpcUa_UInt32 i = 0; i < pEventsEngine->m_pEventDefinitionList->size(); i++)
			{
				it++;
				CEventDefinition* pEventDefinition = pEventsEngine->m_pEventDefinitionList->at(i);
				if (pEventDefinition)
				{
					// If this event definition is enabled
					// This means 
					if (pEventDefinition->IsEnable())
					{
						// Evaluate the state machine for this eventDefinition
						//OpcUa_Mutex_Lock(pInformationModel->GetServerCacheMutex());
						//uStatus = pEventDefinition->EvaluateStateMachine();
						//OpcUa_Mutex_Unlock(pInformationModel->GetServerCacheMutex());
						// Cleanup virtual CEventDefinition
						if (pEventDefinition->IsVirtual())
						{
							// remove the CEventDefinition from thelist
							delete pEventDefinition;
							pEventsEngine->m_pEventDefinitionList->erase(it);
						}
					}
				}
			}
		}
		OpcUa_Mutex_Unlock(pEventsEngine->m_pEventDefinitionListMutex);
		OpcUa_Mutex_Unlock(pEventsEngine->m_hEventsThreadMutex);
		// Calcul du temps de traitement
#if _WIN32_WINNT >= 0x0600
		ULONGLONG dwEnd = GetTickCount64();
		ULONGLONG dwCountedTime = dwEnd - dwStart;
		if (dwInterval>dwCountedTime)
			uiSleepTime = (OpcUa_UInt32)(dwInterval - dwCountedTime);
		else
			uiSleepTime = 0;
#else 
	#if _WIN32_WINNT == 0x0501
		DWORD dwEnd = GetTickCount();
		DWORD dwCountedTime = dwEnd - dwStart;
		if (dwInterval>dwCountedTime)
			uiSleepTime = dwInterval - dwCountedTime;
		else
			uiSleepTime = 0;
	#endif
#endif

#ifdef _GNUC_
		DWORD dwEnd = GetTickCount();
		DWORD dwCountedTime = dwEnd - dwStart;
		if (dwInterval>dwCountedTime)
			uiSleepTime = dwInterval - dwCountedTime;
		else
			uiSleepTime = 0;
#endif
		// Attente de la prochaine echeance du timer
		uStatus = OpcUa_Semaphore_TimedWait(pEventsEngine->m_EventsThreadSem, uiSleepTime);
	}
	OpcUa_Semaphore_Post(pEventsEngine->m_EventsThreadSem, 1);
	OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "EventsThread Leave\n");
}


void CEventsEngine::AddEventDefinition(CEventDefinition* pEventDefinition)
{
	OpcUa_Mutex_Lock(m_pEventDefinitionListMutex);
	if (m_pEventDefinitionList)
		m_pEventDefinitionList->push_back(pEventDefinition);
	OpcUa_Mutex_Unlock(m_pEventDefinitionListMutex);
}
/// <summary>
/// Gets the CEventDefinition with the index iEvtIndex.
/// the index is the position in the vector m_pEventDefinitionList
/// </summary>
/// <param name="iEvtIndex">Index to return.</param>
/// <param name="ppEventDefinition">The CEventDefinition found.</param>
/// <returns></returns>
OpcUa_StatusCode CEventsEngine::GetEventDefinition(OpcUa_UInt32 iEvtIndex,CEventDefinition** ppEventDefinition)
{
	OpcUa_StatusCode uStatus = OpcUa_BadNotFound;
	// Must be locked by the caller
	if (iEvtIndex<=m_pEventDefinitionList->size())
	{
		(*ppEventDefinition)=m_pEventDefinitionList->at(iEvtIndex);
		uStatus = OpcUa_Good;
	}
	return uStatus;
}
OpcUa_StatusCode CEventsEngine::RemoveEventDefinition(CEventDefinition* pEventDefinition)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	if (pEventDefinition)
	{
		if (m_pEventDefinitionList)
		{
			;
		}
		else
			uStatus = OpcUa_BadInvalidArgument;
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}
OpcUa_Mutex	CEventsEngine::GetEventDefinitionListMutex()
{
	return m_pEventDefinitionListMutex;
}
OpcUa_UInt32 CEventsEngine::GetEventDefinitionListSize()
{
	if (!m_pEventDefinitionList)
		return 0;
	else
		return m_pEventDefinitionList->size();
}
OpcUa_StatusCode CEventsEngine::SearchForEventDefinitionOnMethod(CUAMethod* pUAMethod,CEventDefinition** ppEventDefinition)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_Mutex_Lock(m_pEventDefinitionListMutex);
	if (m_pEventDefinitionList)
	{
		CEventDefinitionList::iterator it = m_pEventDefinitionList->begin();
		for (OpcUa_UInt32 i = 0; i < m_pEventDefinitionList->size(); i++)
		{
			CEventDefinition* pEventDefinition = m_pEventDefinitionList->at(i);
			if ((pEventDefinition->GetDisableMethod() == pUAMethod) || (pEventDefinition->GetEnableMethod() == pUAMethod))
			{
				*ppEventDefinition = pEventDefinition;
				break;
			}
		}
	}
	OpcUa_Mutex_Unlock(m_pEventDefinitionListMutex);
	return uStatus;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Gets event definition. </summary>
///
/// <remarks>	Michel, 01/09/2016. </remarks>
///
/// <param name="aNodeId">			   	Identifier for the node. </param>
/// <param name="pEventDefinitionList">	[in,out] If non-null, list of event definitions. </param>
///
/// <returns>	The event definition. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode OpenOpcUa::UAEvents::CEventsEngine::GetEventDefinition(OpcUa_NodeId aNodeId, CEventDefinitionList* pEventDefinitionList)
{
	OpcUa_StatusCode uStatus = OpcUa_BadNotFound;
	// Must be locked by the caller
	if (pEventDefinitionList)
	{
		CUAInformationModel* pInformationModel = CServerApplication::m_pTheAddressSpace;
		CUABase* pUABase = OpcUa_Null;
		if (pInformationModel->GetNodeIdFromDictionnary(aNodeId, &pUABase) == OpcUa_Good)
		{
			for (OpcUa_UInt32 i = 0; i < m_pEventDefinitionList->size(); i++)
			{
				CEventDefinition* pEventDefinition = m_pEventDefinitionList->at(i);
				CUABase* pMonitoringObject=pEventDefinition->GetMonitoringObject();
				OpcUa_NodeId* pNodeId = pMonitoringObject->GetNodeId();
				if (pInformationModel->IsInUABaseHierarchy(aNodeId, *pNodeId))
				{
					uStatus = OpcUa_Good;
					pEventDefinitionList->push_back(pEventDefinition);
				}
			}
		}
	}
	return uStatus;	
}

