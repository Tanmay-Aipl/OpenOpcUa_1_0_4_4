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

#include "SimulatedNode.h"
#include "SimulatedGroup.h"
using namespace UASimulation;
#include "UAObject.h"
#include "VfiDataValue.h"
#include "UAHistorianVariable.h"
#include "HaEngine.h"
using namespace UAHistoricalAccess;
#include "ServerApplication.h"
#include <math.h>
using namespace OpenOpcUa;
using namespace UACoreServer;
//
// Simulation thread static variables declaration
//
#define Random() (float)(rand()*fltDem)

BOOL CSimulatedGroup::m_bRunSimulationThread;
//OpcUa_Semaphore CSimulatedGroup::m_hStopSimulationThread;
CSimulatedGroup::CSimulatedGroup(void)
{
	OpcUa_Mutex_Create(&m_SimulatedGroupMutex);
	m_hSimulationThread=OpcUa_Null;
	m_pSimulatedNodes = new CSimulatedNodeList();
	OpcUa_Semaphore_Create(&m_hStopSimulationThreadSem, 0, 0x100);
	StartSimulationThread();
}
CSimulatedGroup::CSimulatedGroup(const char **atts)
{
	OpcUa_Mutex_Create(&m_SimulatedGroupMutex);
	m_hSimulationThread=OpcUa_Null;
	m_pSimulatedNodes =new CSimulatedNodeList();
	int ii=0;
	while (atts[ii])
	{
		if (OpcUa_StrCmpA(atts[ii],"Rate")==0)
		{
			m_dwRate=atoi(atts[ii+1]);
		}
		ii+=2;
	}
	OpcUa_Semaphore_Create(&m_hStopSimulationThreadSem, 0, 0x100);
	StartSimulationThread();
}
CSimulatedGroup::~CSimulatedGroup(void)
{
	StopSimulationThread();
	RemoveAllSimulatedNode();
	delete m_pSimulatedNodes;
	OpcUa_Semaphore_Delete(&m_hStopSimulationThreadSem);
	OpcUa_Mutex_Delete(&m_SimulatedGroupMutex);
}
void CSimulatedGroup::AddSimulatedNode(CSimulatedNode* pSimulatedNode)
{
	OpcUa_Mutex_Lock(m_SimulatedGroupMutex);
	m_pSimulatedNodes->push_back(pSimulatedNode);
	OpcUa_Mutex_Unlock(m_SimulatedGroupMutex);
}
void CSimulatedGroup::RemoveAllSimulatedNode()
{
	OpcUa_Mutex_Lock(m_SimulatedGroupMutex);
	CSimulatedNodeList::iterator it;
	while (!m_pSimulatedNodes->empty())
	{
		it = m_pSimulatedNodes->begin();
		delete *it;
		m_pSimulatedNodes->erase(it);
	}
	OpcUa_Mutex_Unlock(m_SimulatedGroupMutex);
}
// Thread en charge de la simulation
//
void CSimulatedGroup::StartSimulationThread()
{
	if (m_hSimulationThread==OpcUa_Null)
	{
		m_bRunSimulationThread=TRUE;
		OpcUa_StatusCode uStatus  = OpcUa_Thread_Create(&m_hSimulationThread,SimulationThread,this);

		if(uStatus != OpcUa_Good)
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Create SimulationThread Failed\n");
		else
			OpcUa_Thread_Start(m_hSimulationThread);
	}
}
void /*__stdcall*/ CSimulatedGroup::SimulationThread(LPVOID arg)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	DWORD dwSleepTime=0;
//#if _WIN32_WINNT >= 0x0600
//		ULONGLONG dwStart = GetTickCount64();
//#else 
//	#if _WIN32_WINNT == 0x0501
//		DWORD dwStart = GetTickCount();
//	#endif
//#endif
//#ifdef _GNUC_
//	DWORD dwStart = GetTickCount();
//#endif
	DWORD dwInterval;

	CUAInformationModel* pInformationModel=CServerApplication::m_pTheAddressSpace;
	CSimulatedGroup* pSimulatedGroup=(CSimulatedGroup*)arg;
	dwInterval=pSimulatedGroup->m_dwRate;
	// Will wait for a simulation Event to be posted of the SimulationThread end.
	//while( (OpcUa_Semaphore_TimedWait(pInformationModel->GetSimulationEvent(),100)==OpcUa_GoodNonCriticalTimeout) )
	//{
	//	if (!pSimulatedGroup->m_bRunSimulationThread)
	//		break;
	//}
	OpcUa_Semaphore_TimedWait(pInformationModel->GetSimulationEvent(), INFINITE);
	srand(1);
	while (pSimulatedGroup->m_bRunSimulationThread)
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
		// Simulation
		CSimulatedNodeList* pSimulatedNodes=pSimulatedGroup->GetSimulatedNodeList();
		OpcUa_Mutex_Lock(pSimulatedGroup->m_SimulatedGroupMutex);
		for (OpcUa_UInt32 ii=0;ii<pSimulatedNodes->size();ii++)
		{
			CSimulatedNode* pSimulatedNode=pSimulatedNodes->at(ii);
			if (pSimulatedNode)
			{
				CUAVariable* pUAVariable = OpcUa_Null;
				OpcUa_NodeId* pNodeId = pSimulatedNode->GetNodeId();
				OpcUa_NodeId aNodeId;
				OpcUa_NodeId_Initialize(&aNodeId);
				OpcUa_NodeId_CopyTo(pNodeId, &aNodeId);
				OpcUa_Mutex_Lock(pInformationModel->GetServerCacheMutex());
				uStatus = pInformationModel->GetNodeIdFromVariableList(aNodeId, &pUAVariable);
				if (uStatus == OpcUa_Good)
				{
					switch (pSimulatedNode->GetSimType())
					{
					case SIMTYPE_NONE:
						break;
					case SIMTYPE_RAMP:
						pSimulatedGroup->SimulateRamp(pUAVariable);
						break;
					case SIMTYPE_RANDOM:
						pSimulatedGroup->SimulateRandom(pUAVariable);
						break;
					case SIMTYPE_SINE:
						pSimulatedGroup->SimulateSine(pUAVariable);
						break;
					default:
						break;
					}
				}
				else
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Simulation configuration is corrupted GetNodeIdFromVariableList failed uStatus=0x%05x\n",uStatus);
				OpcUa_NodeId_Clear(&aNodeId);
				OpcUa_Mutex_Unlock(pInformationModel->GetServerCacheMutex());
			}
			else
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Simulation configuration is corrupted pSimulatedNode not found : 0x%05x\n",uStatus);
		}
		OpcUa_Mutex_Unlock(pSimulatedGroup->m_SimulatedGroupMutex);
		// Fin simulation
		DWORD dwEnd=GetTickCount();
		DWORD dwCountedTime=dwEnd-dwStart;
		if (dwInterval>dwCountedTime)
			dwSleepTime=dwInterval-dwCountedTime;
		else
			dwSleepTime=0;
		OpcUa_Semaphore_TimedWait(pSimulatedGroup->m_hStopSimulationThreadSem, dwSleepTime);
	}
	OpcUa_Semaphore_Post(pSimulatedGroup->m_hStopSimulationThreadSem, 1);
}
void CSimulatedGroup::StopSimulationThread()
{
	m_bRunSimulationThread=FALSE;
	OpcUa_Semaphore_Post(m_hStopSimulationThreadSem, 1);
	OpcUa_StatusCode uStatus = OpcUa_Semaphore_TimedWait(m_hStopSimulationThreadSem, OPC_TIMEOUT * 2); // 15 secondes max.
	if (uStatus == OpcUa_GoodNonCriticalTimeout)
	{
		// on force la fin du thread de simulation
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Impossible to stop the SimulationThread. Timeout\n");
		//
	}
	else
	{
		OpcUa_Thread_Delete(m_hSimulationThread);
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "SimulationThread stopped properly\n");
	}
}

void CSimulatedGroup::SimulateRamp(CUAVariable* pUAVariable)
{
	CDataValue* pValue=pUAVariable->GetValue();
	if (pValue)
	{
		OpcUa_Variant aVariant = pValue->GetValue();
		switch (aVariant.Datatype)
		{
		case OpcUaType_Boolean:
			if (aVariant.Value.Boolean == (OpcUa_Boolean)OpcUa_True)
				aVariant.Value.Boolean = OpcUa_False;
			else
				aVariant.Value.Boolean = OpcUa_True;
			break;
		case OpcUaType_Byte:
			aVariant.Value.Byte += 1;
			break;
		case OpcUaType_Int16:
			aVariant.Value.Int16 += 1;
			break;
		case OpcUaType_UInt16:
			aVariant.Value.Int16 += 1;
			break;
		case OpcUaType_Int32:
			aVariant.Value.Int32 += 1;
			break;
		case OpcUaType_UInt32:
			aVariant.Value.UInt32 += 1;
			break;
		case OpcUaType_Float:
			aVariant.Value.Float += 1.0;
			break;
		case OpcUaType_Double:
			aVariant.Value.Double += 1.0;
			break;
		case OpcUaType_DateTime:
			aVariant.Value.DateTime = OpcUa_DateTime_UtcNow();
			break;
		default:
			break;
		}
		pValue->SetStatusCode(OpcUa_Good);
		// initialisation de l'heure
		pValue->SetServerTimestamp(OpcUa_DateTime_UtcNow());
		pValue->SetSourceTimestamp(OpcUa_DateTime_UtcNow());
		pValue->SetValue(aVariant);
		pUAVariable->SetValue(pValue);
	}
}
void CSimulatedGroup::SimulateSine(CUAVariable* pUAVariable)
{
	CDataValue* pValue=pUAVariable->GetValue();
	OpcUa_Variant aVariant=pValue->GetValue();
	OpcUa_Double aDbl=0.0;
	OpcUa_DateTime aDateTime=OpcUa_DateTime_UtcNow();
	
	switch (aVariant.Datatype)
	{
	case OpcUaType_Boolean:
		if (aVariant.Value.Boolean==(OpcUa_Boolean)OpcUa_True)
			aVariant.Value.Boolean=OpcUa_False;
		else
			aVariant.Value.Boolean=OpcUa_True;
		break;
	case OpcUaType_Byte:
		aDbl=255.0;
		aVariant.Value.Byte=(OpcUa_Byte)(aDbl*sin((float)aDateTime.dwLowDateTime));
		break;
	case OpcUaType_Int16:
		aDbl=255.0;
		aVariant.Value.Int16=(OpcUa_Int16)(aDbl*sin((float)aDateTime.dwLowDateTime));
		break;
	case OpcUaType_UInt16:
		aDbl=255.0;
		aVariant.Value.UInt16=(OpcUa_UInt16)(aDbl*sin((float)aDateTime.dwLowDateTime));
		break;
	case OpcUaType_Int32:
		aDbl=255.0;
		aVariant.Value.Int32=(OpcUa_Int32)(aDbl*sin((float)aDateTime.dwLowDateTime));
		break;
	case OpcUaType_UInt32:
		aDbl=255.0;
		aVariant.Value.UInt32=(OpcUa_UInt32)(aDbl*sin((float)aDateTime.dwLowDateTime));
		break;
	case OpcUaType_Float:
		aDbl=255.0;
		aVariant.Value.Float=(OpcUa_Float)aDbl*sin((float)aDateTime.dwLowDateTime);
		break;
	case OpcUaType_Double:
		aDbl=255.0;
		aVariant.Value.Double=(OpcUa_Double)aDbl*sin((float)aDateTime.dwLowDateTime);
		break;
	default:
		break;
	}
	pValue->SetStatusCode(OpcUa_Good);
	// initialisation de l'heure
	pValue->SetServerTimestamp(OpcUa_DateTime_UtcNow());
	pValue->SetSourceTimestamp(OpcUa_DateTime_UtcNow());
	pValue->SetValue(aVariant);
	pUAVariable->SetValue(pValue);
}
void CSimulatedGroup::SimulateRandom(CUAVariable* pUAVariable)
{
	CDataValue* pValue=pUAVariable->GetValue();
	OpcUa_Variant aVariant=pValue->GetValue();
	float fltDem=0.0;
	switch (aVariant.Datatype)
	{
	case OpcUaType_Boolean:
		if (aVariant.Value.Boolean==(OpcUa_Boolean)OpcUa_True)
			aVariant.Value.Boolean=OpcUa_False;
		else
			aVariant.Value.Boolean=OpcUa_True;
		break;
	case OpcUaType_Byte:
		fltDem=(float)(aVariant.Value.Byte/RAND_MAX+1);
		aVariant.Value.Byte=(OpcUa_Byte)Random();
		break;
	case OpcUaType_Int16:
		fltDem=(float)(aVariant.Value.Int16/RAND_MAX+1);
		aVariant.Value.Int16=(OpcUa_Int16)Random();
		break;
	case OpcUaType_UInt16:
		fltDem=(float)(aVariant.Value.UInt16/RAND_MAX+1);
		aVariant.Value.UInt16=(OpcUa_UInt16)Random();
		break;
	case OpcUaType_Int32:
		fltDem=(float)(aVariant.Value.Int32/RAND_MAX+1);
		aVariant.Value.Int32=(OpcUa_Int32)Random();
		break;
	case OpcUaType_UInt32:
		fltDem=(float)(aVariant.Value.UInt32/RAND_MAX+1);
		aVariant.Value.UInt32=(OpcUa_UInt32)Random();
		break;
	case OpcUaType_Float:
		fltDem=(float)(aVariant.Value.Float/RAND_MAX+1);
		aVariant.Value.Float=(OpcUa_Float)Random();
		break;
	case OpcUaType_Double:
		fltDem=(float)(aVariant.Value.Double/RAND_MAX+1);
		aVariant.Value.Double=(OpcUa_Double)Random();
		break;
	default:
		break;
	}
	pValue->SetStatusCode(OpcUa_Good);
	// initialisation de l'heure
	pValue->SetServerTimestamp(OpcUa_DateTime_UtcNow());
	pValue->SetSourceTimestamp(OpcUa_DateTime_UtcNow());
	pValue->SetValue(aVariant);
	pUAVariable->SetValue(pValue);
}