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
using namespace OpenOpcUa;
using namespace UAAddressSpace;
#include "UAVariable.h"
#ifdef _GNUC_
#include <dlfcn.h>
#endif
#include "VpiFuncCaller.h"
#include "VpiTag.h"
#include "VpiWriteObject.h"
#include "VpiDevice.h"
//#include "Utils.h"
#include "Field.h"
#include "UAReferenceType.h"
#include "UAObjectType.h"
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
#include "SecureChannel.h"

#include "VfiDataValue.h"
#include "UAHistorianVariable.h"
#include "HaEngine.h"
using namespace UAHistoricalAccess;

#include "ServerApplication.h"

CHaEngine::CHaEngine(void)
{
	m_uiArchiveEngineFrequency = 100;
	m_hStorageThread=OpcUa_Null;
	m_hVfi=OpcUa_Null;
	// Initialisation des fonctions du Vfi
	VfiGlobalStart=OpcUa_Null;
	VfiColdStart=OpcUa_Null;
	VfiWarmStart=OpcUa_Null;
	VfiHistoryRead=OpcUa_Null;
	VfiHistoryWrite=OpcUa_Null;
	VfiParseAddId=OpcUa_Null;
	VfiParseRemoveId=OpcUa_Null;
	VfiSetNotifyCallback=OpcUa_Null;
	m_bVfiInitialized=OpcUa_False;
	m_pLibraryName=OpcUa_Null;
	m_pUAHistorianVariableList=new CUAHistorianVariableList();
	OpcUa_Semaphore_Create(&m_StorageThreadSem,0,0x100);
	OpcUa_Mutex_Create(&m_StorageVfiCallMutex);
	OpcUa_Mutex_Create(&m_hStorageThreadMutex);
	StartStorageThread();
}

CHaEngine::~CHaEngine(void)
{
	StopStorageThread();
	if (m_pUAHistorianVariableList)
	{
		m_pUAHistorianVariableList->clear();
		delete m_pUAHistorianVariableList;
	}
	OpcUa_Semaphore_Delete(&m_StorageThreadSem);
	OpcUa_Mutex_Delete(&m_StorageVfiCallMutex);
	OpcUa_Mutex_Delete(&m_hStorageThreadMutex);
}

void CHaEngine::StartStorageThread(void)
{
	if (m_hStorageThread==OpcUa_Null)
	{
		m_bRunStorageThread = OpcUa_True;
		OpcUa_StatusCode uStatus  = OpcUa_Thread_Create(&m_hStorageThread,StorageThread,this);

		if(uStatus != OpcUa_Good)
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Create StorageThread Failed");
		else
			OpcUa_Thread_Start(m_hStorageThread);
	}
}

void CHaEngine::StopStorageThread(void)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_Mutex_Lock(m_hStorageThreadMutex);
	if (m_bRunStorageThread)
	{
		m_bRunStorageThread=OpcUa_False;
		OpcUa_Semaphore_Post(m_StorageThreadSem,1); 
		uStatus = OpcUa_Semaphore_TimedWait( m_StorageThreadSem,OPC_TIMEOUT*2); // INFINITE - OPC_TIMEOUT*2= 15 secondes max.
		if (uStatus == OpcUa_GoodNonCriticalTimeout)
		{
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Impossible to stop the StorageThread. Timeout\n");
		}
		else
		{
			OpcUa_Thread_Delete(m_hStorageThread);
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_WARNING, "StorageThread stopped properly\n");
		}
	}
	OpcUa_Mutex_Unlock(m_hStorageThreadMutex);
	return ;
}
// The storage thread call Raw VFI function to store information from the UA Nodes

void CHaEngine::StorageThread(LPVOID arg)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	
	OpcUa_UInt32 uiSleepTime=0;
	CHaEngine* pHaEngine=(CHaEngine*)arg;
	OpcUa_UInt32 dwInterval = 10;
	if (pHaEngine)
		dwInterval = pHaEngine->GetArchiveEngineFrequency();
	OpcUa_Semaphore_TimedWait(pHaEngine->m_StorageThreadSem,INFINITE);
	while (pHaEngine->m_bRunStorageThread)
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
		OpcUa_Mutex_Lock(pHaEngine->m_hStorageThreadMutex);
		OpcUa_UInt32 uiNbOfValueToWrite=1;
		// Parcours de la liste de node a archiver
		for (OpcUa_UInt32 i=0;i<pHaEngine->m_pUAHistorianVariableList->size();i++)
		{
			CUAHistorianVariable* pUAHistorianVariable =pHaEngine->m_pUAHistorianVariableList->at(i);
			if (pUAHistorianVariable->IsTime())
			{
				// pour chaque node a archiver dont le timer d'archivage est échu
				// appel de la fonction VfiHistoryWriteValue
				if (pHaEngine->VfiHistoryWrite)
				{
					CUAVariable* pUAVariable=pUAHistorianVariable->GetInternalUAVariable();
					OpcUa_NodeId* pNodeId=pUAVariable->GetNodeId();
					CDataValue* pDataValue=pUAVariable->GetValue();
					OpcUa_DataValue* pInternalDataValue=pDataValue->GetInternalDataValue();
					OpcUa_DataValue* pUADataValue = (OpcUa_DataValue*)OpcUa_Alloc(sizeof(OpcUa_DataValue));
					if (pUADataValue)
					{
						OpcUa_DataValue_Initialize(pUADataValue);
						OpcUa_DataValue_CopyTo(pInternalDataValue, pUADataValue);
						OpcUa_Mutex_Lock(pHaEngine->m_StorageVfiCallMutex);	
						OpcUa_NodeId aNodeId4Vfi;
						OpcUa_NodeId_Initialize(&aNodeId4Vfi);
						OpcUa_NodeId_CopyTo(pNodeId, &aNodeId4Vfi);
						pHaEngine->VfiHistoryWrite(pHaEngine->m_hVfi, aNodeId4Vfi, uiNbOfValueToWrite, pUADataValue);
						OpcUa_Mutex_Unlock(pHaEngine->m_StorageVfiCallMutex);
						OpcUa_DataValue_Clear(pUADataValue);
						OpcUa_NodeId_Clear(&aNodeId4Vfi);
						OpcUa_Free(pUADataValue);
					}
				}
				// Reset this timer interval
				pUAHistorianVariable->ResetTimerInterval();
			}
			else
			{
				// Mise a jour de l'ensemble de timer d'archivage
				DWORD dwEnd=GetTickCount();
				OpcUa_Double dblVal=((OpcUa_Double)dwInterval-(dwEnd-dwStart));
				pUAHistorianVariable->UpdateTimerInterval(dblVal);
			}
		}
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
		OpcUa_Mutex_Unlock(pHaEngine->m_hStorageThreadMutex);
		// Attente de la prochaine echeance du timer
		uStatus=OpcUa_Semaphore_TimedWait(pHaEngine->m_StorageThreadSem,uiSleepTime);
	}
	OpcUa_Semaphore_Post(pHaEngine->m_StorageThreadSem, 1);
	OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "StorageThread Leave\n");
}

OpcUa_StatusCode CHaEngine::InitHaEngine(void)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	CUAInformationModel* pInformationModel=CServerApplication::m_pTheAddressSpace;
	// Scan all UAVariables for AccessLevel attribute
	// If the AccessLevel attribute is HistoryRead and/or HistoryWrite
	// so we put it in the HaEngine loop for store processing
	CUAVariableList* pUAVariableList=pInformationModel->GetVariableList();
	OpcUa_Mutex_Lock(pInformationModel->GetServerCacheMutex());
	CUAVariableList::iterator it;
	for (it = pUAVariableList->begin(); it != pUAVariableList->end(); it++)
	{
		CUAVariable* pUAVariable = it->second;
		if ( (pUAVariable->IsHistoryReadable()) || (pUAVariable->IsHistoryWritable()) )
		{
			CUAReferenceList* pUAVariableReferences=pUAVariable->GetReferenceNodeList();
			// Step 1
			// Search for the References of type HistoricalDataConfigurationType
			OpcUa_Boolean bHDCfgTypeFound=OpcUa_False;
			CUAObject* pUAObject=OpcUa_Null;
			for (OpcUa_UInt32 iii=0;iii<pUAVariableReferences->size();iii++)
			{
				CUAReference* pReference=pUAVariableReferences->at(iii);
				if (pReference->IsHistoricalConfiguration())
				{
					OpcUa_ExpandedNodeId aExpNodeId=pReference->GetTargetId();
					CUABase* pUABase=OpcUa_Null;
					if (pInformationModel->GetNodeIdFromDictionnary(aExpNodeId.NodeId,&pUABase)==OpcUa_Good)
					{
						if (pUABase->GetNodeClass()==OpcUa_NodeClass_Object)
						{
							OpcUa_NodeId aTypeDef=((CUAObject*)pUABase)->GetTypeDefinition();
							OpcUa_NodeId aHDCType;
							OpcUa_NodeId_Initialize(&aHDCType);
							aHDCType.Identifier.Numeric=2318; // HistoricalDataConfigurationType
							if (Utils::IsEqual(&aTypeDef,&aHDCType))
							{
								// Ok the NodeId point to a HistoricalDataConfigurationType. 
								// Step 1 is Ok> Let's setup the values requested by the HAEngine (Step 2)
								bHDCfgTypeFound=OpcUa_True;
								pUAObject=(CUAObject*)pUABase;
								break;
							}
						}
					}
				}
			}
			if (bHDCfgTypeFound)
			{
				// Step 2
				// Ok we got the definition for the Historical Configuration
				// Let initialize the valuewe need by our engine
				CUAHistorianVariable* pUAHistorianVariable=new CUAHistorianVariable(pUAVariable);
				CUAReferenceList* pUAObjectReferences=pUAObject->GetReferenceNodeList();
				CUAVariable* pUAVariable00=OpcUa_Null;
				CUABase* pUABase=OpcUa_Null;
				for (OpcUa_UInt32 iii=0;iii<pUAObjectReferences->size();iii++)
				{
					pUABase=OpcUa_Null;
					pUAVariable00=OpcUa_Null;
					CUAReference* pReference=pUAObjectReferences->at(iii);
					OpcUa_ExpandedNodeId aExpNodeId=pReference->GetTargetId();
					if (pInformationModel->GetNodeIdFromDictionnary(aExpNodeId.NodeId,&pUABase)==OpcUa_Good)
					{
						if (pUABase->GetNodeClass()==OpcUa_NodeClass_Variable)
						{
							pUAVariable00=(CUAVariable*)pUABase;
							OpcUa_String aString=pUAVariable00->GetBrowseName()->Name;
							// Stepped
							if (OpcUa_StriCmpA(OpcUa_String_GetRawString(&aString),"Stepped")==0)
							{
								if (pUAVariable00->GetValue()->GetValue().Datatype==OpcUaType_Boolean)
								{
									OpcUa_Boolean bVal=pUAVariable00->GetValue()->GetValue().Value.Boolean;
									pUAHistorianVariable->SetStepped(bVal);
								}
								else
									OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"HA Configuration error Stepped Variable contains the wrong DataType\n");
							}
							// Definition
							if (OpcUa_StriCmpA(OpcUa_String_GetRawString(&aString),"Definition")==0)
							{
								if (pUAVariable00->GetValue()->GetValue().Datatype==OpcUaType_String)
								{
									OpcUa_String szVal=pUAVariable00->GetValue()->GetValue().Value.String;
									pUAHistorianVariable->SetDefinition(szVal);
								}
								else
									OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"HA Configuration error Definition Variable contains the wrong DataType\n");
							}
							// MaxTimeInterval
							if (OpcUa_StriCmpA(OpcUa_String_GetRawString(&aString),"MaxTimeInterval")==0)
							{
								if (pUAVariable00->GetBuiltInType()==OpcUaType_Double)
								{
									OpcUa_Double dblVal=pUAVariable00->GetValue()->GetValue().Value.Double;
									pUAHistorianVariable->SetMaxTimeInterval(dblVal);
								}
								else
									OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"HA Configuration error MaxTimeInterval Variable contains the wrong DataType\n");
							}
							// MinTimeInterval
							if (OpcUa_StriCmpA(OpcUa_String_GetRawString(&aString),"MinTimeInterval")==0)
							{
								if (pUAVariable00->GetBuiltInType()==OpcUaType_Double)
								{
									OpcUa_Double dblVal=pUAVariable00->GetValue()->GetValue().Value.Double;
									pUAHistorianVariable->SetMinTimeInterval(dblVal);
								}
								else
									OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"HA Configuration error MinTimeInterval Variable contains the wrong DataType\n");
							}
							// ExceptionDeviation
							if (OpcUa_StriCmpA(OpcUa_String_GetRawString(&aString),"ExceptionDeviation")==0)
							{
								if (pUAVariable00->GetBuiltInType()==OpcUaType_Double)
								{
									OpcUa_Double dblVal=pUAVariable00->GetValue()->GetValue().Value.Double;
									pUAHistorianVariable->SetExceptionDeviation(dblVal);
								}
								else
									OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"HA Configuration error ExceptionDeviation Variable contains the wrong DataType\n");
							}
							// ExceptionDeviationFormat
							if (OpcUa_StriCmpA(OpcUa_String_GetRawString(&aString),"ExceptionDeviationFormat")==0)
							{
								if (pUAVariable00->GetBuiltInType()==OpcUaType_Int32) // Just for debug i=890 -->Int32
								{
									OpcUa_UInt32 uiVal=pUAVariable00->GetValue()->GetValue().Value.UInt32;
									pUAHistorianVariable->SetExceptionDeviationFormat((ExceptionDeviationFormat)uiVal);
								}
								else
									OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"HA Configuration error ExceptionDeviationFormat Variable contains the wrong DataType\n");
							}
							// StartOfArchive
							if (OpcUa_StriCmpA(OpcUa_String_GetRawString(&aString),"StartOfArchive")==0)
							{
								if (pUAVariable00->GetBuiltInType()==OpcUaType_DateTime)
								{
									OpcUa_DateTime dtVal=pUAVariable00->GetValue()->GetValue().Value.DateTime;
									pUAHistorianVariable->SetStartOfArchive(dtVal);
								}
								else
									OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"HA Configuration error StartOfArchive Variable contains the wrong DataType\n");
							}
							// StartOfOnlineArchive
							if (OpcUa_StriCmpA(OpcUa_String_GetRawString(&aString),"StartOfOnlineArchive")==0)
							{
								if (pUAVariable00->GetBuiltInType()==OpcUaType_DateTime)
								{
									OpcUa_DateTime dtVal=pUAVariable00->GetValue()->GetValue().Value.DateTime;
									pUAHistorianVariable->SetStartOfOnlineArchive(dtVal);
								}
								else
									OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"HA Configuration error StartOfOnlineArchive Variable contains the wrong DataType\n");
							}
						}
					}
				}
				m_pUAHistorianVariableList->push_back(pUAHistorianVariable);
			}
			else
			{
				char* szNodeId=OpcUa_Null;
				Utils::NodeId2String(pUAVariable->GetNodeId(), &szNodeId);
				// Attribute
				if (szNodeId)
				{
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "InitHaEngine>The Historical Configuration of %s  is not correct. Please check your Nodeset files\n", szNodeId);
					free(szNodeId);
				}
			}
		}
	}
	OpcUa_Mutex_Unlock(pInformationModel->GetServerCacheMutex());

	return uStatus;
}

OpcUa_StatusCode CHaEngine::LoadVfi()
{
	OpcUa_StatusCode uStatus=OpcUa_LoadLibrary(m_pLibraryName,(void**)&m_phInst);
	if (uStatus==OpcUa_Good)
	{
#ifdef _WIN32_WCE
			VfiGlobalStart = (PFUNCVFIGLOBALSTART)OpcUa_GetProcAddress(m_phInst,L"VfiGlobalStart");
#else
			VfiGlobalStart = (PFUNCVFIGLOBALSTART)OpcUa_GetProcAddress(m_phInst,"VfiGlobalStart");
#endif
			if (VfiGlobalStart)
			{
	#ifdef _WIN32_WCE
				VfiColdStart = (PFUNCVFICOLDSTART)OpcUa_GetProcAddress(m_phInst,L"VfiColdStart");
	#else
				VfiColdStart = (PFUNCVFICOLDSTART)OpcUa_GetProcAddress(m_phInst,"VfiColdStart");
	#endif
				if (VfiColdStart)
				{
		#ifdef _WIN32_WCE
					VfiWarmStart = (PFUNCVFIWARMSTART)OpcUa_GetProcAddress(m_phInst,L"VfiWarmStart");
		#else
					VfiWarmStart = (PFUNCVFIWARMSTART)OpcUa_GetProcAddress(m_phInst,"VfiWarmStart");
		#endif
					if (VfiWarmStart)
					{
			#ifdef _WIN32_WCE
						VfiParseAddId = (PFUNCVFIPARSEADDID)OpcUa_GetProcAddress(m_phInst,L"VfiParseAddId");
			#else
						VfiParseAddId = (PFUNCVFIPARSEADDID)OpcUa_GetProcAddress(m_phInst,"VfiParseAddId");
			#endif
						if (VfiParseAddId)
						{
				#ifdef _WIN32_WCE
							VfiParseRemoveId = (PFUNCVFIPARSEREMOVEID)OpcUa_GetProcAddress(m_phInst,L"VfiParseRemoveId");
				#else
							VfiParseRemoveId = (PFUNCVFIPARSEREMOVEID)OpcUa_GetProcAddress(m_phInst,"VfiParseRemoveId");
				#endif
							if (VfiParseRemoveId)
							{
					#ifdef _WIN32_WCE
								VfiHistoryRead = (PFUNCVFIHISTORYREAD)OpcUa_GetProcAddress(m_phInst,L"VfiHistoryRead");
					#else
								VfiHistoryRead = (PFUNCVFIHISTORYREAD)OpcUa_GetProcAddress(m_phInst,"VfiHistoryRead");
					#endif
								if (VfiHistoryRead)
								{
						#ifdef _WIN32_WCE
									VfiHistoryWrite = (PFUNCVFIHISTORYWRITE)OpcUa_GetProcAddress(m_phInst,L"VfiHistoryWrite");
						#else
									VfiHistoryWrite = (PFUNCVFIHISTORYWRITE)OpcUa_GetProcAddress(m_phInst,"VfiHistoryWrite");
						#endif
									if (VfiHistoryWrite)
									{
							#ifdef _WIN32_WCE
										VfiSetNotifyCallback = (PFUNCVFISETNOTFIFYCALLBACK)OpcUa_GetProcAddress(m_phInst,L"VfiSetNotifyCallback");
							#else
										VfiSetNotifyCallback = (PFUNCVFISETNOTFIFYCALLBACK)OpcUa_GetProcAddress(m_phInst,"VfiSetNotifyCallback");
							#endif
										if (VfiSetNotifyCallback)
											m_bVfiInitialized=OpcUa_True;
										else
										{
											OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"LoadVfiLibrary failed %s VfiSetNotifyCallback error: 0x%05x\n",OpcUa_String_GetRawString(m_pLibraryName),OpcUa_GetLastError());
											uStatus=OpcUa_BadInternalError;
										}
									}
									else
									{
										OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"LoadVfiLibrary failed %s VfiWriteValue error: 0x%05x\n",OpcUa_String_GetRawString(m_pLibraryName),OpcUa_GetLastError());
										uStatus=OpcUa_BadInternalError;
									}
								}
								else
								{
									OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"LoadVfiLibrary failed %s VfiReadValue error: 0x%05x\n",OpcUa_String_GetRawString(m_pLibraryName),OpcUa_GetLastError());
									uStatus=OpcUa_BadInternalError;
								}
							}
							else
							{
								OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"LoadVfiLibrary failed %s VfiParseRemoveId error: 0x%05x\n",OpcUa_String_GetRawString(m_pLibraryName),OpcUa_GetLastError());
								uStatus=OpcUa_BadInternalError;
							}
						}
						else
						{
							OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"LoadVfiLibrary failed %s VfiParseAddId error: 0x%05x\n",OpcUa_String_GetRawString(m_pLibraryName),OpcUa_GetLastError());
							uStatus=OpcUa_BadInternalError;
						}
					}
					else
					{
						OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"LoadVfiLibrary failed %s VfiWarmStart error: 0x%05x\n",OpcUa_String_GetRawString(m_pLibraryName),OpcUa_GetLastError());
						uStatus=OpcUa_BadInternalError;
					}
				}
				else
				{
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"LoadVfiLibrary failed %s VfiColdStart error: 0x%05x\n",OpcUa_String_GetRawString(m_pLibraryName),OpcUa_GetLastError());
					uStatus=OpcUa_BadInternalError;
				}
			}
			else
			{
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"LoadVfiLibrary failed %s VfiGlobalStart error: 0x%05x\n",OpcUa_String_GetRawString(m_pLibraryName),OpcUa_GetLastError());
				uStatus=OpcUa_BadInternalError;
			}
	}
	else
	{
		uStatus=OpcUa_BadFileNotFound;
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"LoadVfiLibrary failed %s error: 0x%05x\n",OpcUa_String_GetRawString(m_pLibraryName),OpcUa_GetLastError());
	}
	return uStatus;
}
OpcUa_String* CHaEngine::GetLibraryName() 
{
	return m_pLibraryName;
}
void CHaEngine::SetLibraryName(OpcUa_String libraryName) 
{
	OpcUa_Int32 iLen=OpcUa_String_StrLen(&libraryName);
	if (iLen)
	{
		if (m_pLibraryName)
			OpcUa_String_Clear(m_pLibraryName);
		else
			m_pLibraryName=(OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
		
		OpcUa_String_Initialize(m_pLibraryName);
		OpcUa_String_CopyTo(&libraryName,m_pLibraryName);
	}
}


void CHaEngine::SetArchiveEngineFrequency(OpcUa_UInt32 uiVal)
{
	m_uiArchiveEngineFrequency = uiVal;
}
OpcUa_UInt32 CHaEngine::GetArchiveEngineFrequency()
{
	return m_uiArchiveEngineFrequency;
}