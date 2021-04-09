#ifndef _VPIOPERATINGSYSTEM_H
#define _VPIOPERATINGSYSTEM_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "SourceObject.h"


namespace UASubSystem 
{
	class CVpiInternalData;
	class CVPINullEx
	{
	public:

		CVPINullEx();
		~CVPINullEx();
		// Mecanisme de lecture indépendant du nombre de client connecté
		static void				VPINullExThread(LPVOID arg); // Thread pour la lecture des données au niveau du PLC
		void					StartVPINullExThread(); // methode d'appel de la thread d'accès au PLC
		Vpi_StatusCode	StopVPINullExThread();
		PVOID	GetNotifyCallback() { return m_NotifyCallback; }
		void SetNotifyCallback(PVOID pNotifyCallback) { m_NotifyCallback = pNotifyCallback; }
		void SetVpiInternalData(CVpiInternalData* pInternalData) {m_pInternalData=pInternalData;}
		CVpiInternalData* GetVpiInternalData()
		{
			return m_pInternalData;
		}
		Vpi_Handle GetVpiHandle() {return m_pVpiHandle;}
		void SetVpiHandle(Vpi_Handle hVal) {m_pVpiHandle=hVal;}
		CSourceObject* GetSourceObject(Vpi_String szAddress)
		{
			// TODO
			return NULL;
		}
		Vpi_String GetSubSystemName() {return m_szSubSystemName;}
		void SetSubSystemName(Vpi_String szVal) 
		{
			(void)szVal;
		}
		Vpi_NodeId GetSubsystemId() {return m_SubsystemId;}
		void SetSubsystemId(Vpi_NodeId id) {m_SubsystemId=id;}
		Vpi_Semaphore	GetStopVPINullExThreadSem() { return m_hStopVPINullExThreadSem; }
	private:
		CVpiInternalData*	m_pInternalData;
		Vpi_Handle		m_pVpiHandle;
		Vpi_String		m_szSubSystemName;
		Vpi_NodeId		m_SubsystemId;
		Vpi_Semaphore	m_hStopVPINullExThreadSem;
		Vpi_Boolean		m_bRunVPINullExThread;
		Vpi_Thread		m_hVPINullExThread;
		PVOID				m_NotifyCallback;
	};
}
#endif // _VPIOPERATINGSYSTEM_H
