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
#include "UAHistorianVariable.h"
using namespace UAHistoricalAccess;

CUAHistorianVariable::CUAHistorianVariable()
{
	m_pUAVariable=OpcUa_Null;
	m_TimerInterval=0.0;
	m_bStepped=OpcUa_False;
	OpcUa_String_Initialize(&m_Definition);
	m_MinTimeInterval=0.0;
	m_MaxTimeInterval=5000; // default 5 sec
	m_dblExceptionDeviation=0;
	m_eExceptionDeviationFormat=UNKNOWN;
	ZeroMemory(&m_dtStartOfArchive,sizeof(OpcUa_DateTime));
	ZeroMemory(&m_StartOfOnlineArchive,sizeof(OpcUa_DateTime));

}
CUAHistorianVariable::CUAHistorianVariable(CUAVariable* pUAVariable)
{
	m_pUAVariable=pUAVariable;
	m_bStepped=OpcUa_False;
	OpcUa_String_Initialize(&m_Definition);
}
CUAHistorianVariable::~CUAHistorianVariable()
{
	OpcUa_String_Clear(&m_Definition);
}
OpcUa_Boolean CUAHistorianVariable::IsTime()
{
	OpcUa_Boolean bRes=OpcUa_False;
	if (m_TimerInterval<m_MinTimeInterval)
		bRes=OpcUa_True;
	return bRes;
}
void CUAHistorianVariable::ResetTimerInterval()
{
	m_TimerInterval = m_MaxTimeInterval;
}
void CUAHistorianVariable::UpdateTimerInterval(OpcUa_Double dblVal)
{
	if (m_TimerInterval>=dblVal)
		m_TimerInterval-=dblVal;
	else
		m_TimerInterval=0;
}
CUAVariable* CUAHistorianVariable::GetInternalUAVariable() 
{ 
	return m_pUAVariable; 
}
OpcUa_Boolean	CUAHistorianVariable::GetStepped() 
{ 
	return m_bStepped; 
}
void CUAHistorianVariable::SetStepped(OpcUa_Boolean bVal) 
{ 
	m_bStepped = bVal; 
}
OpcUa_String	CUAHistorianVariable::GetDefinition() 
{ 
	return m_Definition; 
}
void CUAHistorianVariable::SetDefinition(OpcUa_String szVal) 
{ 
	OpcUa_String_CopyTo(&szVal, &m_Definition); 
}
OpcUa_Double CUAHistorianVariable::GetMaxTimeInterval() 
{ 
	return m_MaxTimeInterval; 
}
void CUAHistorianVariable::SetMaxTimeInterval(OpcUa_Double dblVal) 
{ 
	m_MaxTimeInterval = dblVal; 
}
OpcUa_Double CUAHistorianVariable::GetMinTimeInterval() 
{ 
	return m_MinTimeInterval; 
}
void CUAHistorianVariable::SetMinTimeInterval(OpcUa_Double dblVal) 
{ 
	m_MinTimeInterval = dblVal; 
}
OpcUa_Double CUAHistorianVariable::GetExceptionDeviation() 
{ 
	return m_dblExceptionDeviation; 
}
void CUAHistorianVariable::SetExceptionDeviation(OpcUa_Double dblVal) 
{ 
	m_dblExceptionDeviation = dblVal; 
}
ExceptionDeviationFormat CUAHistorianVariable::GetExceptionDeviationFormat() 
{ 
	return m_eExceptionDeviationFormat;
}
void CUAHistorianVariable::SetExceptionDeviationFormat(ExceptionDeviationFormat eVal)
{ 
	m_eExceptionDeviationFormat = eVal; 
}
OpcUa_DateTime CUAHistorianVariable::GetStartOfArchive()
{
	return m_dtStartOfArchive; 
}
void CUAHistorianVariable::SetStartOfArchive(OpcUa_DateTime dtVal) 
{ 
	m_dtStartOfArchive = dtVal; 
}
OpcUa_DateTime CUAHistorianVariable::GetStartOfOnlineArchive() 
{ 
	return m_StartOfOnlineArchive; 
}
void CUAHistorianVariable::SetStartOfOnlineArchive(OpcUa_DateTime dtVal) 
{ 
	m_StartOfOnlineArchive = dtVal; 
}