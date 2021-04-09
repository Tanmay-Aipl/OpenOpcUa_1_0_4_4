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
#include <map>
#include "InternalVfiRecord.h"
#ifdef _GNUC_
typedef struct _SYSTEMTIME {
	WORD wYear;
	WORD wMonth;
	WORD wDayOfWeek;
	WORD wDay;
	WORD wHour;
	WORD wMinute;
	WORD wSecond;
	WORD wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;
#endif

namespace UAHistoricalAccess 
{
	class CVfiInternalData
	{
	public:
		CVfiInternalData() 
		{
			m_pVfiHandle = OpcUa_Null;
			m_InternalStatus=OpcUa_Vfi_UnInitialized;

			m_pVfiXxx= new CVfiXxx();
			m_pVfiXxx->SetVfiInternalData(this);
			m_pInternalRecordList = new CInternalVfiRecordList();
			m_pInternalRecordList->clear();
		}
		~CVfiInternalData() 
		{
			delete m_pVfiXxx;
			delete m_pInternalRecordList;
		}
		OpcUa_Handle GetVfiHandle() {return m_pVfiHandle;}
		void SetVfiHandle(OpcUa_Handle hVal) {m_pVfiHandle=hVal;}

		OpcUa_String GetArchiveName() {return m_szArchiveName;}
		void SetArchiveName(OpcUa_String szVal) 
		{
		}
		OpcUa_NodeId GetArchiveId() {m_ArchiveId;}
		void SetArchiveId(OpcUa_NodeId id);
		CVfiXxx* GetVfiXxx() {return m_pVfiXxx;}
		OpcUa_Vfi_StatusCode GetInternalStatus() {return m_InternalStatus;}
		void SetInternalStatus(OpcUa_Vfi_StatusCode uStatus) {m_InternalStatus=uStatus;}
		CInternalVfiRecordList* GetInternalVfiRecordList() { return m_pInternalRecordList; }
		void AddRecord(CInternalVfiRecord* pInternalVfiRecord); // Ajoute un enregistrement dans la cache des archive
	private:
		OpcUa_Handle m_pVfiHandle;
		OpcUa_String m_szArchiveName;
		OpcUa_NodeId m_ArchiveId;
		CVfiXxx* m_pVfiXxx;
		OpcUa_Vfi_StatusCode m_InternalStatus; // permet d'indiquer l'etat interne du Vfi
		CInternalVfiRecordList* m_pInternalRecordList;
	};
	#define TIMESPEC_TO_FILETIME_OFFSET (((LONGLONG)27111902 << 32) + (LONGLONG)3577643008)
	void	TimeToOpcUaDateTime(struct timeval tVal, FILETIME *ft);
	OpcUa_DateTime Vfi_DateTime_UtcNow();
	OpcUa_Vfi_StatusCode Vfi_FileTimeToSystemTime(const FILETIME *lpFileTime, LPSYSTEMTIME lpSystemTime);
	OpcUa_Vfi_StatusCode Vfi_SystemTimeToFileTime(const LPSYSTEMTIME lpSystemTime, FILETIME *lpFileTime);
}
