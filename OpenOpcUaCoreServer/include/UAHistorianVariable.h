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
		typedef enum ExceptionDeviationFormat
		{
			ABSOLUTE_VALUE,
			PERCENT_OF_VALUE,
			PERCENT_OF_RANGE,
			PERCENT_OF_EU_RANGE,
			UNKNOWN
		} _ExceptionDeviationFormat;
		class CUAHistorianVariable
		{
		public:
			CUAHistorianVariable();
			CUAHistorianVariable(CUAVariable* pUAVariable);
			~CUAHistorianVariable();
			OpcUa_Boolean IsTime();
			void ResetTimerInterval();
			void UpdateTimerInterval(OpcUa_Double dblVal);
			CUAVariable* GetInternalUAVariable();
			OpcUa_Boolean	GetStepped();
			void SetStepped(OpcUa_Boolean bVal);
			OpcUa_String	GetDefinition();
			void SetDefinition(OpcUa_String szVal);
			OpcUa_Double GetMaxTimeInterval();
			void SetMaxTimeInterval(OpcUa_Double dblVal);
			OpcUa_Double GetMinTimeInterval();
			void SetMinTimeInterval(OpcUa_Double dblVal);
			OpcUa_Double GetExceptionDeviation();
			void SetExceptionDeviation(OpcUa_Double dblVal);
			ExceptionDeviationFormat GetExceptionDeviationFormat();
			void SetExceptionDeviationFormat(ExceptionDeviationFormat eVal);
			OpcUa_DateTime GetStartOfArchive();
			void SetStartOfArchive(OpcUa_DateTime dtVal);
			OpcUa_DateTime GetStartOfOnlineArchive();
			void SetStartOfOnlineArchive(OpcUa_DateTime dtVal);
		private:
			CUAVariable*				m_pUAVariable; // Related UAVariable
			// Below are parameter to archive this UAVariable
			OpcUa_Boolean				m_bStepped;
			OpcUa_String				m_Definition;
			OpcUa_Double				m_MaxTimeInterval;
			OpcUa_Double				m_MinTimeInterval;
			OpcUa_Double				m_dblExceptionDeviation; // Specifies the minimum amount that the data for the HistoricalDataNode
																 //  must change in order for the change to be reported to the history database.
			ExceptionDeviationFormat	m_eExceptionDeviationFormat; // specifies how the ExceptionDeviation is determined.
			OpcUa_DateTime				m_dtStartOfArchive;
			OpcUa_DateTime				m_StartOfOnlineArchive;
			// Below are runnning variable for Archiving this UAVariable*
			OpcUa_Double				m_TimerInterval;
		};
		typedef vector<CUAHistorianVariable*> CUAHistorianVariableList;
	}
}