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
#include "UAInformationModel.h"
using namespace UAEvents;
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
using namespace UAAddressSpace;
using namespace UASubSystem;
using namespace UACoreServer;
#include "VpiWriteObject.h"

//OpcUa_Boolean	CVPIScheduler::m_bRunSchedulerThread;
//OpcUa_Semaphore	CVPIScheduler::m_hStopSchedulerThread;
CVPIScheduler::CVPIScheduler(void)
{
	m_uiSamplingInterval=1000; // default value for the polling interval (1000)
	m_hSchedulerThread = OpcUa_Null;
	m_pVpiDevice = OpcUa_Null;
	//// Initialisation de la couche de communication dans le cas d'une implementation pour une Gateway
	//m_pVpiDevices= new CVpiDeviceList();
	//m_pVpiDevices->clear();
	// Scheduler variable and thread init
	// 
	OpcUa_Semaphore_Create(&m_InterColdWarmStartSem, 0, 0x100);
	OpcUa_Semaphore_Create(&m_hStopSchedulerThreadSem,0,0x100);
	OpcUa_Semaphore_Create(&m_hVpiReaderInitialized, 0, 1);
	OpcUa_Semaphore_Create(&m_hVpiWriterInitialized, 0, 1);
	StartVpiReaderThread();
	// Write thread variable and thread init
	m_uiVpiWriterInterval=100;
	m_hVpiWriterThread=OpcUa_Null;
	OpcUa_Semaphore_Create(&m_VpiWriterThreadSem,0,0x100);
	OpcUa_Mutex_Create(&m_VpiWriteObjectsMutex);
	OpcUa_Mutex_Create(&m_VpiReaderThreadMutex);
	m_VpiWriteObjects.clear();
	StartVpiWriterThread();
}

CVPIScheduler::CVPIScheduler(CVpiDevice* pVpiDevice) :m_pVpiDevice(pVpiDevice)
{	
	m_uiSamplingInterval=1000; // default value for the polling interval (1000)
	m_hSchedulerThread=NULL;
	OpcUa_Semaphore_Create(&m_InterColdWarmStartSem, 0, 0x100);
	OpcUa_Semaphore_Create(&m_hStopSchedulerThreadSem, 0, 0x100);
	OpcUa_Semaphore_Create(&m_hVpiReaderInitialized, 0, 1);
	OpcUa_Semaphore_Create(&m_hVpiWriterInitialized, 0, 1);
	StartVpiReaderThread();
	// Write thread variable and thread init
	m_uiVpiWriterInterval=100;
	m_hVpiWriterThread=OpcUa_Null;
	OpcUa_Semaphore_Create(&m_VpiWriterThreadSem,0,0x100);
	OpcUa_Mutex_Create(&m_VpiWriteObjectsMutex);
	OpcUa_Mutex_Create(&m_VpiReaderThreadMutex);
	m_VpiWriteObjects.clear();
	StartVpiWriterThread();
}
CVPIScheduler::~CVPIScheduler(void)
{
	//if (m_pVpiDevice)
	//{
	//	CVpiFuncCaller* pFuncCaller = m_pVpiDevice->GetVpiFuncCaller();
	//	pFuncCaller->VpiInitialized(OpcUa_False);
	//}
	StopVpiWriterThread();
	StopVpiReaderThread();
	//CVpiDeviceList*  pVpiDeviceList=GetVpiDeviceList();
	//for (OpcUa_UInt16 ii=0;ii<pVpiDeviceList->size();ii++)
	//{
	//	CVpiDevice* pVpiDevice=pVpiDeviceList->at(ii);
	//	delete pVpiDevice;
	//}
	//delete m_pVpiDevices;
	OpcUa_Semaphore_Delete(&m_InterColdWarmStartSem);
	OpcUa_Semaphore_Delete(&m_hStopSchedulerThreadSem);
	OpcUa_Semaphore_Delete(&m_hVpiReaderInitialized);
	OpcUa_Semaphore_Delete(&m_hVpiWriterInitialized);
	OpcUa_Mutex_Delete(&m_VpiWriteObjectsMutex);
	OpcUa_Mutex_Delete(&m_VpiReaderThreadMutex);
}
void CVPIScheduler::StartVpiReaderThread()
{
	if (m_hSchedulerThread==NULL)
	{
		m_bRunSchedulerThread=OpcUa_True;
		OpcUa_StatusCode uStatus  = OpcUa_Thread_Create(&m_hSchedulerThread,VpiReaderThread,this);

		if(uStatus != OpcUa_Good)
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Create SchedulerThread Failed\n");
		else
			OpcUa_Thread_Start(m_hSchedulerThread);
	}
}

// Function name   : CVPIScheduler::SchedulerThread
// Description     : This thread initialize and read Vpi. Its shared between Vpis 
//					 
//				   : 
// Return type     : 
// Argument        : LPVOID arg

void CVPIScheduler::VpiReaderThread(LPVOID arg)
{
	//OpcUa_Boolean bPendingData=OpcUa_False;
	OpcUa_UInt32 uiSleepTime;
	
	OpcUa_UInt32 dwInterval = 0;
	
	CVPIScheduler* pVPIScheduler=(CVPIScheduler*)arg;
	OpcUa_Semaphore_Wait(pVPIScheduler->m_hVpiReaderInitialized);
	
	while(pVPIScheduler->m_bRunSchedulerThread)
	{
		dwInterval=((DWORD)pVPIScheduler->GetSamplingInterval());
#if _WIN32_WINNT >= 0x0600
		ULONGLONG dwStart = GetTickCount64();
#else 
	#if _WIN32_WINNT == 0x0501
		DWORD dwStart = GetTickCount();
	#endif
#endif
#ifdef _GNUC_
		DWORD dwStart= GetTickCount();
#endif
		
		// Initialisation if needed
		OpcUa_Vpi_StatusCode uStatus=OpcUa_Bad;
		CVpiDevice* pVpiDevice = pVPIScheduler->m_pVpiDevice;
		if (pVpiDevice)
		{
			CVpiFuncCaller* pFuncCaller = pVpiDevice->GetVpiFuncCaller();
			if (pFuncCaller)
			{
				OpcUa_Mutex_Lock(pVPIScheduler->m_VpiReaderThreadMutex);
				// Let's verify that the ColdStart was already called once
				if ((pFuncCaller->IsVpiInitialized() == (OpcUa_Boolean)OpcUa_True) && (pFuncCaller->IsColdStarted() == (OpcUa_Boolean)OpcUa_False))
				{
					// Call the ColdStart
					uStatus = pFuncCaller->ColdStart((pFuncCaller->m_hVpi));
					if (uStatus == OpcUa_Good)//OpcUa_Vpi_Good
						pFuncCaller->ColdStarted(OpcUa_True);
					else
						OpcUa_Semaphore_TimedWait(pVPIScheduler->m_InterColdWarmStartSem, 5000);
					pVpiDevice->SetInternalStatus(uStatus);
				}
				else
				{
					// Let's check if the Vpi require a WarmStart but never call the Cold and Warmstart in the same loop
					if (pVpiDevice->GetInternalStatus() == 0x83050000) // OpcUa_Vpi_WarmStartNeed
					{
						uStatus = pFuncCaller->WarmStart(pFuncCaller->m_hVpi);
						if (uStatus == OpcUa_Good)
							OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_WARNING, "WarmStart request succeeded\n");
						else
							OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "WarmStart failed 0x%05x\n", uStatus);
						pVpiDevice->SetInternalStatus(uStatus);
					}
				}
				// Now we will read all Tags attached to this Vpi
				if ((pVPIScheduler->m_pVpiDevice->GetAccessDataMode() == UASubSystem::BOTH) || (pVPIScheduler->m_pVpiDevice->GetAccessDataMode() == UASubSystem::POLL))
				{
					if (pFuncCaller->IsVpiInitialized())
					{
						CVpiTagList* pSignals = pVPIScheduler->m_pVpiDevice->GetTags();
						if (pSignals)
						{
							OpcUa_UInt32 uiNbOfValueRead = pSignals->size();
							if (uiNbOfValueRead)
							{
								// Allocation of the content. So the VPi is not suppose to release it but just to fill it according to the requested dataType
								OpcUa_DataValue* pReadDataValue = (OpcUa_DataValue*)OpcUa_Alloc(sizeof(OpcUa_DataValue)*uiNbOfValueRead);
								if (pReadDataValue)
								{
									OpcUa_NodeId* Ids = (OpcUa_NodeId*)OpcUa_Alloc(sizeof(OpcUa_NodeId)*uiNbOfValueRead);
									if (Ids)
									{
										for (OpcUa_UInt16 ii = 0; ii < uiNbOfValueRead; ii++)
										{
											OpcUa_DataValue_Initialize(&pReadDataValue[ii]);
											OpcUa_NodeId_Initialize(&Ids[ii]);
											CVpiTag* pSignal = pSignals->at(ii);
											CUAVariable* pUAVariable = pSignal->GetUAVariable();
											if (pUAVariable)
											{
												OpcUa_Byte bBuiltInType = pSignal->GetUAVariable()->GetBuiltInType();
												pReadDataValue[ii].Value.Datatype = bBuiltInType;
												if (pSignal->GetNbElement() > 0)
												{
													// Affectation de la taille
													pReadDataValue[ii].Value.ArrayType = OpcUa_VariantArrayType_Array;
													pReadDataValue[ii].Value.Value.Array.Length = pSignal->GetNbElement();
												}
												else
													pReadDataValue[ii].Value.ArrayType = OpcUa_VariantArrayType_Scalar;
											}
											else
											{
												// workaround permettant de s'adapter a la dynamique de chargement des configurations
												pSignal->SyncUAVariable();
												// fin workaround
											}
											// NodeIds
											OpcUa_NodeId tmpNodeId = pSignal->GetNodeId();
											OpcUa_NodeId_CopyTo(&tmpNodeId, &Ids[ii]);
										}
										/////////////////////////////////////////////////////////////////////
										//
										// Call the Vpi to make the read
										uStatus = pFuncCaller->ReadValue(pFuncCaller->m_hVpi, uiNbOfValueRead, Ids, &pReadDataValue);
										if (uStatus == OpcUa_Good)
										{
											pVPIScheduler->m_pVpiDevice->SetInternalStatus(uStatus);
											// Transfert result to the server cache
											for (OpcUa_UInt16 ii = 0; ii < uiNbOfValueRead; ii++)
											{
												CVpiTag* pSignal = pSignals->at(ii);
												CUAVariable* pUAVariable = pSignal->GetUAVariable();
												if (pUAVariable)
												{
													CDataValue* pDataValue = pUAVariable->GetValue();
													if (pDataValue)
													{
														///////////////////////////////////////////////////
														if ((pUAVariable->GetBuiltInType() == OpcUaType_String) && (pSignal->GetNbElement()>0) && (pSignal->GetDataType().Identifier.Numeric == OpcUaType_Byte))
														{
															OpcUa_String strValue = pDataValue->GetValue().Value.String;
															OpcUa_String_Initialize(&strValue);
															OpcUa_String_AttachCopy(&strValue, (OpcUa_CharA*)pReadDataValue->Value.Value.Array.Value.ByteArray);
														}
														else
															///////////////////////////////////////////////////
															pUAVariable->SetValue(pReadDataValue[ii].Value);

														if ((pReadDataValue[ii].SourceTimestamp.dwLowDateTime == 0) && (pReadDataValue[ii].SourceTimestamp.dwHighDateTime == 0))
														{
															// Affectation de l'heure a partir de l'heure du PC
															pDataValue->SetServerTimestamp(OpcUa_DateTime_UtcNow());
														}
														else
														{
															pDataValue->SetServerTimestamp(OpcUa_DateTime_UtcNow());
															// affectation de l'heure transmise par le VPI
															pDataValue->SetSourceTimestamp(pReadDataValue[ii].SourceTimestamp);
														}
														// Qualité
														pDataValue->SetStatusCode(pReadDataValue[ii].StatusCode);
													}
													else
														OpcUa_Trace(OPCUA_TRACE_SERVER_ERROR, "Internal error in the VpiDataValue configuration: Server Side\n");
													OpcUa_Variant_Clear(&pReadDataValue[ii].Value);
												}
											}
										}
										else
										{
											if (uStatus == 0x83050000)
											{
												pVPIScheduler->m_pVpiDevice->SetInternalStatus(uStatus);
												//// appel du WarmStart
												//uStatus = pFuncCaller->WarmStart(pFuncCaller->m_hVpi);
												//if (uStatus == OpcUa_Good)//OpcUa_Vpi_Good
												//{
												//	pFuncCaller->WarmStarted(OpcUa_True);
												//	if (uStatus == OpcUa_Good)//OpcUa_Vpi_Good
												//	{
												//		pFuncCaller->ColdStarted(OpcUa_True);
												//		pFuncCaller->WarmStarted(OpcUa_True);
												//	}
												//}
											}
											else
											{
												// quality bad
												for (OpcUa_UInt16 ii = 0; ii < uiNbOfValueRead; ii++)
												{
													CVpiTag* pSignal = pSignals->at(ii);
													CUAVariable* pUAVariable = pSignal->GetUAVariable();
													if (pUAVariable)
													{
														CDataValue* pDataValue = pUAVariable->GetValue();
														if (pDataValue)
															pDataValue->SetStatusCode(OpcUa_BadDeviceFailure);
													}
												}
											}
										}
										// Release ressources
										for (OpcUa_UInt16 ii = 0; ii < uiNbOfValueRead; ii++)
										{
											OpcUa_NodeId_Clear(&Ids[ii]);
										}
										OpcUa_Free(Ids);
									}
									else
										uStatus = OpcUa_BadOutOfMemory;
									OpcUa_Free(pReadDataValue);
								}
								else
									uStatus = OpcUa_BadOutOfMemory;
							}
						}
					}
				}
				OpcUa_Mutex_Unlock(pVPIScheduler->m_VpiReaderThreadMutex);
			}
		}
		// fin implementation
		

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
		DWORD dwEnd=GetTickCount();
		DWORD dwCountedTime = dwEnd - dwStart;
		if (dwInterval>dwCountedTime)
			uiSleepTime = dwInterval - dwCountedTime;
		else
			uiSleepTime = 0;
#endif

		OpcUa_Semaphore_TimedWait(pVPIScheduler->m_hStopSchedulerThreadSem, uiSleepTime);
	}

	if (pVPIScheduler->m_pVpiDevice)
	{
		CVpiFuncCaller* pFuncCaller = pVPIScheduler->m_pVpiDevice->GetVpiFuncCaller();
		if (pFuncCaller)
		{
			// appel du GlobalStop
			if ( (pFuncCaller->IsVpiInitialized()==(OpcUa_Boolean)OpcUa_True))
			{
				pFuncCaller->VpiInitialized(OpcUa_False);
				OpcUa_StatusCode uStatus= pFuncCaller->GlobalStop((pFuncCaller->m_hVpi));
				if (uStatus != OpcUa_Good)
					pFuncCaller->VpiInitialized(OpcUa_True);
				else
				{
					pFuncCaller->GlobalStop = OpcUa_Null;
					pFuncCaller->GlobalStart = OpcUa_Null;
					pFuncCaller->ColdStart = OpcUa_Null;
					pFuncCaller->WarmStart = OpcUa_Null;
					pFuncCaller->ReadValue = OpcUa_Null;
					pFuncCaller->WriteValue = OpcUa_Null;
					pFuncCaller->ParseAddId = OpcUa_Null;
					pFuncCaller->ParseRemoveId = OpcUa_Null;
					pFuncCaller->SetNotifyCallback = OpcUa_Null;
				}
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "GlobalStop was called to stop the Vpi uStatus=0x%05x\n", uStatus);
			}
		}
	}
	OpcUa_Semaphore_Post(pVPIScheduler->m_hStopSchedulerThreadSem,1);
}
OpcUa_StatusCode CVPIScheduler::StopVpiReaderThread()
{
	OpcUa_String vpiName = m_pVpiDevice->GetName();
	OpcUa_Mutex_Lock(m_VpiReaderThreadMutex);
	m_bRunSchedulerThread=OpcUa_False;
	OpcUa_Semaphore_Post(m_hStopSchedulerThreadSem,1);
	OpcUa_StatusCode uStatus = OpcUa_Semaphore_TimedWait( m_hStopSchedulerThreadSem,OPC_TIMEOUT*2); // 15 secondes max.
	if (uStatus == OpcUa_GoodNonCriticalTimeout)
	{
		// on force la fin du thread de simulation
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Impossible to stop the SchedulerThread for Vpi %s. Timeout\n", OpcUa_String_GetRawString(&vpiName));
	}
	else
	{
		OpcUa_Thread_Delete(m_hSchedulerThread);
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "SchedulerThread for Vpi %s stopped properly\n", OpcUa_String_GetRawString(&vpiName));
	}
	OpcUa_Mutex_Unlock(&m_VpiReaderThreadMutex);
	return uStatus;
}
/////////////////////////////////////////////////////////////////////////////////
//
// Thread en charge des Ecritures dans les Vpi
// Stub chargé de demarrer la thread de traitement des demandes de lecture asynchrone
void CVPIScheduler::StartVpiWriterThread()
{
	if (m_hVpiWriterThread==OpcUa_Null)
	{
		m_bRunVpiWriterThread=OpcUa_True;
		OpcUa_StatusCode uStatus  = OpcUa_Thread_Create(&m_hVpiWriterThread,(CVPIScheduler::VpiWriterThread),this);

		if(uStatus != OpcUa_Good)
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Create VpiWriterThread Failed");
		else
			OpcUa_Thread_Start(m_hVpiWriterThread);
	}
}
void CVPIScheduler::StopVpiWriterThread(void)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	if (m_bRunVpiWriterThread)
	{
		OpcUa_Mutex_Lock(m_VpiWriteObjectsMutex);
		m_bRunVpiWriterThread=OpcUa_False;
		OpcUa_Semaphore_Post(m_VpiWriterThreadSem,1); 
		uStatus = OpcUa_Semaphore_TimedWait( m_VpiWriterThreadSem,OPC_TIMEOUT*2); // INFINITE - OPC_TIMEOUT*2= 15 secondes max.
		if (uStatus == OpcUa_GoodNonCriticalTimeout)
		{
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Impossible to stop the VpiWriterThread. Timeout\n");
		}
		else
		{
			OpcUa_Thread_Delete(m_hVpiWriterThread);
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_WARNING, "VpiWriterThread stopped properly\n");
		}
		OpcUa_Mutex_Unlock(m_VpiWriteObjectsMutex);
	}
	return ;
}
// thread de traitement des demandes d"ecriture dans les Vpi
void CVPIScheduler::VpiWriterThread(LPVOID arg)
{
	CVPIScheduler* pVpiScheduler=(CVPIScheduler*)arg;
	OpcUa_Semaphore_Wait(pVpiScheduler->m_hVpiWriterInitialized);
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_UInt32 dwInterval = pVpiScheduler->GetVpiWriterInterval();
	OpcUa_UInt32 uiSleepTime = 100;
	while(pVpiScheduler->m_bRunVpiWriterThread)
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
		CVpiFuncCaller* pFuncCaller = pVpiScheduler->m_pVpiDevice->GetVpiFuncCaller();
		if (pFuncCaller->IsVpiInitialized())
		{
			OpcUa_UInt32 UiNbOfValueWrite = pVpiScheduler->m_VpiWriteObjects.size();
			if (UiNbOfValueWrite)
			{
				OpcUa_NodeId* Ids = (OpcUa_NodeId*)OpcUa_Alloc(sizeof(OpcUa_NodeId) * UiNbOfValueWrite);
				if (Ids)
				{
					OpcUa_DataValue* pValue = (OpcUa_DataValue*)OpcUa_Alloc(sizeof(OpcUa_DataValue) * UiNbOfValueWrite);
					if (pValue)
					{
						OpcUa_Mutex_Lock(pVpiScheduler->m_VpiWriteObjectsMutex);
						CVpiFuncCaller* pFuncCaller = OpcUa_Null;
						for (OpcUa_UInt32 i = 0; i < UiNbOfValueWrite; i++)
						{
							OpcUa_DataValue_Initialize(&pValue[i]);
							OpcUa_NodeId_Initialize(&Ids[i]);
							CVpiWriteObject* pVpiWriteObject = pVpiScheduler->m_VpiWriteObjects.at(i);
							if (pVpiWriteObject)
							{
								CVpiDataValue* pVpiDataValue = pVpiWriteObject->GetVpiDataValue();
								OpcUa_Variant* pVariant = pVpiDataValue->GetValue();
								// Fill the Ids
								OpcUa_NodeId id = pVpiWriteObject->GetNodeId();
								OpcUa_NodeId_CopyTo(&id, &Ids[i]);
								// Fill the OpcUa_DataValue
								OpcUa_Variant_Initialize(&(pValue[i].Value));
								OpcUa_Variant_CopyTo(pVariant, &(pValue[i].Value));
								pValue[i].SourceTimestamp = pVpiDataValue->GetSourceTimestamp();
								pValue[i].SourcePicoseconds = pVpiDataValue->GetSourcePicosecond();
								pValue[i].StatusCode = pVpiDataValue->GetStatusCode();
							}
						}
						// Call the Vpis
						if (pFuncCaller == OpcUa_Null)
							pFuncCaller = pVpiScheduler->m_pVpiDevice->GetVpiFuncCaller();
						if (pFuncCaller)
						{
							uStatus = pFuncCaller->WriteValue(pFuncCaller->m_hVpi, UiNbOfValueWrite, Ids, &pValue);
							if (uStatus == 0x83050000) // OpcUa_Vpi_WarmStartNeed
								pFuncCaller->WarmStart(pFuncCaller->m_hVpi);
							else
							{
								for (OpcUa_UInt32 i = 0; i < UiNbOfValueWrite; i++)
								{

									CVpiWriteObject* pVpiWriteObject = pVpiScheduler->m_VpiWriteObjects.at(i);
									if (pVpiWriteObject)
									{
										CUAVariable* pUAVariable = pVpiWriteObject->GetUAVariable();
										if (pUAVariable)
										{
											CDataValue* pDataValue = pUAVariable->GetValue();
											// update Statuscode. It will be update from the Vpi low level StatusCode (pValue->StatusCode) or 
											//  from the Vpi global uStatus if the function failed.
											if (uStatus != OpcUa_Good)
											{
												pDataValue->SetStatusCode(uStatus);
												//pVpiWriteObject->Wrote(); // mark as written
											}
											else
											{
												pDataValue->SetStatusCode(pValue->StatusCode);
												pVpiWriteObject->Wrote(); // mark as written
											}
											// Update Timestamp
											pDataValue->SetSourceTimestamp(pValue->SourceTimestamp);
											pDataValue->SetSourcePicoseconds(pValue->SourcePicoseconds);
										}
									}
								}
							}
							//clear the m_VpiWriteObjects
							CVpiWriteObjectList::iterator it;
							OpcUa_UInt32 iCounter = 0;
							while ((pVpiScheduler->m_VpiWriteObjects.empty() == OpcUa_False))
							{
								if (iCounter == pVpiScheduler->m_VpiWriteObjects.size() && (iCounter > 0))
									break;
								{
									it = pVpiScheduler->m_VpiWriteObjects.begin();
									CVpiWriteObject* pVpiWriteObject = *it;
									if (pVpiWriteObject->WasWrote())
									{
										delete pVpiWriteObject;
										pVpiScheduler->m_VpiWriteObjects.erase(it);
									}
									else
										iCounter++;
								}
							}
						}
						OpcUa_Mutex_Unlock(pVpiScheduler->m_VpiWriteObjectsMutex);
						OpcUa_Free(pValue);
					}
					// Release ressources
					for (OpcUa_UInt16 ii = 0; ii < UiNbOfValueWrite; ii++)
					{
						OpcUa_NodeId_Clear(&Ids[ii]);
					}
					OpcUa_Free(Ids);
				}
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
		// Attente de la prochaine echeance du timer
		uStatus=OpcUa_Semaphore_TimedWait(pVpiScheduler->m_VpiWriterThreadSem,uiSleepTime);
	}
	OpcUa_Semaphore_Post(pVpiScheduler->m_VpiWriterThreadSem,1);
}
void CVPIScheduler::AddWriteObject(CVpiWriteObject* pVpiWriteObject)
{
	OpcUa_Mutex_Lock(m_VpiWriteObjectsMutex);
	m_VpiWriteObjects.push_back(pVpiWriteObject);
	OpcUa_Mutex_Unlock(m_VpiWriteObjectsMutex);
	// Wake up the VpiWriterThread 
	OpcUa_Semaphore_Post(m_VpiWriterThreadSem, 1);
}

OpcUa_StatusCode CVPIScheduler::Utf8ToLatin1(OpcUa_CharA* src, OpcUa_CharA* dest)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	int i = 0;
	int j = 0;
	OpcUa_UCharA c = src[i++];
	while (c)
	{
		// one-byte code
		if ((c & 0x80) == 0x00)
		{
			dest[j++] = c;
			c = src[i++];
		}
		// two-byte code
		else
		{
			if ((c & 0xE0) == 0xC0)
			{
				OpcUa_UCharA c2 = src[i++];
				if (c2 == 0) // incomplete character
					break;
				OpcUa_UInt16 res = ((c & 0x1F) << 6) | (c2 & 0x3F);
				if (res < 0x00A0 || res > 0x00FF) // not a Latin-1 character
					dest[j++] = '*';
				else
					dest[j++] = (res & 0xFF);
				c = src[i++];
			}
			// 3-byte or 4-byte code (not a Latin-1 character)
			else
			{
				do
				{
					c = src[i++];
				} while ((c != 0) && ((c & 0xC0) == 0x80));
				dest[j++] = '*';
			}
		}
	}
	dest[j] = 0;
	return uStatus;
}
