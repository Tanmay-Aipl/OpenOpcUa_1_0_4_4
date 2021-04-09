// VPIOperatingSystem.cpp : définit les fonctions exportées pour l'application DLL.
//

#include <stdafx.h>

using namespace std;
using namespace UASubSystem;
CVPINullEx::CVPINullEx()
{
	m_hVPINullExThread = Vpi_Null;
	m_bRunVPINullExThread = Vpi_False;
	Vpi_Semaphore_Create(&m_hVPINullExThread, 0, 0x100);
	StartVPINullExThread();
}
CVPINullEx::~CVPINullEx()
{
	StopVPINullExThread();
	if (m_hVPINullExThread)
		Vpi_Semaphore_Delete(&m_hVPINullExThread);

}

void CVPINullEx::VPINullExThread(LPVOID arg)
{
	CVPINullEx* pVPINullEx = (CVPINullEx *)arg;
	DWORD dwSleepTime;
	// Will wait until the cold start is finished
	Vpi_Semaphore_TimedWait(pVPINullEx->m_hStopVPINullExThreadSem, INFINITE);
	while (pVPINullEx->m_bRunVPINullExThread)
	{
		DWORD dwStart = GetTickCount();
		DWORD dwPollingMin = 5000;
		PFUNCNOTIFYCALLBACK pFuncNotifyCallback = (PFUNCNOTIFYCALLBACK)pVPINullEx->GetNotifyCallback();
		CVpiInternalData* pVpiInternalData = pVPINullEx->GetVpiInternalData();
		if (pVpiInternalData)
		{
			Vpi_String szAddress;
			Vpi_String_Initialize(&szAddress);
			// Enter a protected section
			Vpi_Mutex_Lock(pVpiInternalData->m_OpcUaSourceObjectMutex);
			CSourceObject* pSourceSource = pVpiInternalData->GetSourceObject(szAddress);
			Vpi_NodeId aNodeId; // Fill it with the appropriate NodeId
			Vpi_NodeId_Initialize(&aNodeId);

			Vpi_UInt32 uiSizeToSend = 1; // TODO provide the numOfChange to send
			Vpi_DataValue* pValue = (Vpi_DataValue*)malloc(sizeof(Vpi_DataValue)*uiSizeToSend);
			// Todo Fill pValue
			// Do someting in the protected section

			Vpi_Mutex_Unlock(pVpiInternalData->m_OpcUaSourceObjectMutex);
			// Notify the Server			 
			pFuncNotifyCallback(uiSizeToSend, &aNodeId, pValue);
		} 
		// Calculate the delay to wait before the next tick
		DWORD dwEnd = GetTickCount();
		DWORD dwCountedTime = dwEnd - dwStart;
		if (dwPollingMin>dwCountedTime)
			dwSleepTime = dwPollingMin - dwCountedTime;
		else
			dwSleepTime = 0;
		// wait
		Vpi_Semaphore_TimedWait(pVPINullEx->m_hStopVPINullExThreadSem, dwSleepTime);
	} 
	printf("VPINullExThread was termined\n");
	Vpi_Semaphore_Post(pVPINullEx->m_hStopVPINullExThreadSem, 1);
}

void CVPINullEx::StartVPINullExThread()
{
	if (m_hVPINullExThread == Vpi_Null)
	{
		m_bRunVPINullExThread = Vpi_True;
		if (Vpi_Thread_Create(&m_hVPINullExThread, VPINullExThread, this) == Vpi_Good)
			Vpi_Thread_Start(m_hVPINullExThread);
		else
		{
			printf("Create VPINullExThread Failed\n");
			return;
		}
	}
}

Vpi_StatusCode CVPINullEx::StopVPINullExThread()
{
	Vpi_StatusCode uStatus = Vpi_Good;
	if (!m_bRunVPINullExThread)
		return uStatus;
	m_bRunVPINullExThread = FALSE;
	Vpi_Semaphore_Post(m_hStopVPINullExThreadSem, 1);
	Vpi_StatusCode dwTimeoutReason = Vpi_Semaphore_TimedWait(m_hStopVPINullExThreadSem, OPC_TIMEOUT * 2); // 10 secondes max.
	if (dwTimeoutReason == WAIT_TIMEOUT)
	{
		// on force la fin du thread de simulation
		printf("Impossible to stop the VPINullExThread. Timeout\n");
		uStatus = Vpi_BadInternalError;
	}
	return uStatus;
}