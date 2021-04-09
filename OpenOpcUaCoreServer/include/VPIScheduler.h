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
#pragma once
//#include "VpiDataValue.h"
//#include "VpiDevice.h"
//#include "VpiFuncCaller.h"
//OpcUa_Vpi_StatusCode VpiNotifyCallback(OpcUa_NodeId Id, CVpiDataValue* pValue); // fonction chagré de recevoir les notification depuis les Vpi
//#include "VpiDevice.h"
namespace OpenOpcUa
{
	namespace UASubSystem 
	{		
		class CVPIScheduler
		{
		public:
			CVPIScheduler(void);
			CVPIScheduler(CVpiDevice* pVpiDevice);
			~CVPIScheduler(void);
			OpcUa_UInt32 GetSamplingInterval() { return m_uiSamplingInterval; }
			void SetSamplingInterval(OpcUa_Double dblVal)  { m_uiSamplingInterval = (OpcUa_UInt32)dblVal; }
			//VpiFuncCallerList* GetVpisFuncCaller() {return m_pVPIsFuncCaller;}
			//CVpiDeviceList*  GetVpiDeviceList() {return m_pVpiDevices;}
			void AddWriteObject(CVpiWriteObject* pVpiWriteObject);
		private:
			// Thread prenant en charge l'accès aux fonctions VPI
			void StartVpiReaderThread();
			OpcUa_StatusCode StopVpiReaderThread();
			static void VpiReaderThread(LPVOID arg);
			// Thread d'ecriture dans les Vpis
			void StartVpiWriterThread();
			void StopVpiWriterThread(void);
			static void VpiWriterThread(LPVOID arg);
			OpcUa_UInt32 GetVpiWriterInterval() {return m_uiVpiWriterInterval;}
			OpcUa_StatusCode Utf8ToLatin1(OpcUa_CharA* src, OpcUa_CharA* dest);
		public:
			OpcUa_Semaphore				m_hVpiReaderInitialized;
			OpcUa_Semaphore				m_hVpiWriterInitialized;
		private:
			// variables for the Scheduler thread
			OpcUa_Thread				m_hSchedulerThread;
			OpcUa_UInt32				m_uiSamplingInterval;
			OpcUa_Boolean				m_bRunSchedulerThread;
			OpcUa_Semaphore				m_hStopSchedulerThreadSem;
			OpcUa_Mutex					m_VpiReaderThreadMutex;
			// Variable for the Writer thread
			OpcUa_Boolean				m_bRunVpiWriterThread;
			OpcUa_Thread				m_hVpiWriterThread;
			OpcUa_Semaphore				m_VpiWriterThreadSem;
			OpcUa_UInt32				m_uiVpiWriterInterval;
			CVpiWriteObjectList			m_VpiWriteObjects; // List of the object to write
			OpcUa_Mutex					m_VpiWriteObjectsMutex;
			CVpiDevice*					m_pVpiDevice;
			OpcUa_Semaphore				m_InterColdWarmStartSem; // this semaphore is used between two call to ColdStart/WarmStart in case of failer
		};
	}
}