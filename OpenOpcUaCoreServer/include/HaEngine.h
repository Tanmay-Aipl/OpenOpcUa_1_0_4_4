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
namespace OpenOpcUa
{
	namespace UAHistoricalAccess
	{
		typedef OpcUa_UInt32 OpcUa_Vfi_StatusCode;
		typedef void* OpcUa_Vfi_Handle;
	#ifdef __cplusplus
	extern "C" {
	#endif
		typedef OpcUa_StatusCode (*PFUNCVFIGLOBALSTART)(OpcUa_String szSubSystemName, OpcUa_NodeId VfiId, OpcUa_Vfi_Handle* hVfi);
		typedef OpcUa_StatusCode (*PFUNCVFICOLDSTART)(OpcUa_Vpi_Handle hVfi);
		typedef OpcUa_StatusCode (*PFUNCVFIWARMSTART)(OpcUa_Vpi_Handle hVfi);
		typedef OpcUa_StatusCode (*PFUNCVFIHISTORYREAD)(OpcUa_Vfi_Handle hVfi,
														OpcUa_NodeId Id, 
														OpcUa_DateTime dtFrom, 
														OpcUa_DateTime dtTo,
														OpcUa_UInt32* pUiNbOfValueRead,
														OpcUa_DataValue** pValue);
		typedef OpcUa_StatusCode (*PFUNCVFIHISTORYWRITE)(OpcUa_Vfi_Handle hVfi,
														 OpcUa_NodeId Id, 
														 OpcUa_UInt32 uiNbOfValueToWrite,
														 OpcUa_DataValue* pValue);
		typedef OpcUa_StatusCode (*PFUNCVFIPARSEADDID)(OpcUa_Vpi_Handle hVfi,OpcUa_NodeId Id, OpcUa_Byte Datatype, OpcUa_UInt32 iNbElt,OpcUa_Byte AccessRight, OpcUa_String ParsedAddress );
		typedef OpcUa_StatusCode (*PFUNCVFIPARSEREMOVEID)(OpcUa_Vpi_Handle hVfi,OpcUa_NodeId Id);

		typedef OpcUa_Vfi_StatusCode(__stdcall *PFUNCVFINOTIFYCALLBACK)(OpcUa_UInt32 uiNoOfNotifiedObject, OpcUa_NodeId* Id, OpcUa_DataValue* pDataValue);
		typedef OpcUa_StatusCode (*PFUNCVFISETNOTFIFYCALLBACK)(OpcUa_Vpi_Handle hVfi,PFUNCNOTIFYCALLBACK func);
	#ifdef __cplusplus
	}
	#endif

		class CHaEngine
		{
		public:
			CHaEngine(void);
			~CHaEngine(void);
			OpcUa_StatusCode LoadVfi();
			OpcUa_String* GetLibraryName();
			void SetLibraryName(OpcUa_String LibraryName);
			OpcUa_StatusCode InitHaEngine(void);
			OpcUa_Vfi_Handle GetVfiHandle() { return	m_hVfi; }
			void SetVfiHandle(OpcUa_Vfi_Handle hVfi) {m_hVfi=hVfi;}
			void SetArchiveEngineFrequency(OpcUa_UInt32 uiVal);
			OpcUa_UInt32 GetArchiveEngineFrequency();
		protected:
			void StartStorageThread(void);
			void StopStorageThread(void);
			static void StorageThread(LPVOID arg);
		public:
			OpcUa_Semaphore					m_StorageThreadSem;
			OpcUa_Mutex						m_StorageVfiCallMutex; // this mutex protect the access to Vfi functions. Read and Write cannot be call at the same time
		private:
			OpcUa_Boolean					m_bRunStorageThread;
			CUAHistorianVariableList*		m_pUAHistorianVariableList;
			OpcUa_Thread					m_hStorageThread;
			OpcUa_Mutex						m_hStorageThreadMutex;
			OpcUa_String*					m_pLibraryName; // Name of the related Library in charge of DB 
#ifdef WIN32
			HINSTANCE			m_phInst;
#endif

#ifdef _GNUC_
			void*				m_phInst;
#endif
			OpcUa_Vfi_Handle			m_hVfi;
		public:
			PFUNCVFIGLOBALSTART			VfiGlobalStart;
			PFUNCVFICOLDSTART			VfiColdStart;
			PFUNCVFIHISTORYREAD			VfiHistoryRead;
		private:
			PFUNCVFIWARMSTART			VfiWarmStart;
			PFUNCVFIPARSEADDID			VfiParseAddId;
			PFUNCVFIPARSEREMOVEID		VfiParseRemoveId;
			PFUNCVFIHISTORYWRITE		VfiHistoryWrite;
			PFUNCVFISETNOTFIFYCALLBACK	VfiSetNotifyCallback;
			OpcUa_Boolean				m_bVfiInitialized;
			OpcUa_UInt32				m_uiArchiveEngineFrequency;
		};
	}
}

