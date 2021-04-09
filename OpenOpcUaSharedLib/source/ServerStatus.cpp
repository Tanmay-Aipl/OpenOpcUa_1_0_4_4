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
using namespace UASharedLib;
CServerStatus::CServerStatus(void)
{
	//m_pBuildInfo = OpcUa_Null;
	// allocation du OpcUa_ServerStatusDataType encaspulé
	m_pInternalServerStatus=(OpcUa_ServerStatusDataType*)OpcUa_Alloc(sizeof(OpcUa_ServerStatusDataType));
	if (m_pInternalServerStatus)
	{
		OpcUa_ServerStatusDataType_Initialize(m_pInternalServerStatus);

		// allocation de la classe CBuildInfo associée
		//m_pBuildInfo = new CBuildInfo();
		//if (m_pBuildInfo)
		{	
			//OpcUa_BuildInfo* pBuildInfo = m_pBuildInfo->GetInternalBuildInfo();
			//m_pInternalServerStatus->BuildInfo.BuildDate.dwHighDateTime = pBuildInfo->BuildDate.dwHighDateTime;
			//m_pInternalServerStatus->BuildInfo.BuildDate.dwLowDateTime = pBuildInfo->BuildDate.dwLowDateTime;
			//m_pInternalServerStatus->BuildInfo.BuildNumber = pBuildInfo->BuildNumber;
			//OpcUa_String_CopyTo(&(pBuildInfo->ManufacturerName),&(m_pInternalServerStatus->BuildInfo.ManufacturerName));
			//OpcUa_String_CopyTo(&(pBuildInfo->ManufacturerName),&(m_pInternalServerStatus->BuildInfo.ProductName));
			//OpcUa_String_CopyTo(&(pBuildInfo->ProductUri),&(m_pInternalServerStatus->BuildInfo.ProductUri));
			//m_pInternalServerStatus->BuildInfo.SoftwareVersion = pBuildInfo->SoftwareVersion;
		}
		OpcUa_DateTime_Initialize(&(m_pInternalServerStatus->CurrentTime));
		OpcUa_LocalizedText_Initialize(&(m_pInternalServerStatus->ShutdownReason));
		OpcUa_DateTime_Initialize(&(m_pInternalServerStatus->StartTime));
	}
}
CServerStatus::CServerStatus(OpcUa_ServerStatusDataType* pServerStatus)
{
	//m_pBuildInfo = OpcUa_Null;
	m_pInternalServerStatus = OpcUa_Null;
	if (!m_pInternalServerStatus)
		m_pInternalServerStatus = Utils::Copy(pServerStatus);
}
CServerStatus::~CServerStatus(void)
{
	if (m_pInternalServerStatus)
	{
		OpcUa_ServerStatusDataType_Clear(m_pInternalServerStatus);
		OpcUa_Free(m_pInternalServerStatus);
		m_pInternalServerStatus = OpcUa_Null;
	}
}
OpcUa_BuildInfo *CServerStatus::GetInternalBuildInfo()
{
	if (m_pInternalServerStatus)
		return &(m_pInternalServerStatus->BuildInfo);
	else
		return OpcUa_Null;
}
OpcUa_DateTime CServerStatus::GetStartTime()
{
	OpcUa_DateTime dtVal;
	OpcUa_DateTime_Initialize(&dtVal);
	if (m_pInternalServerStatus)
		return m_pInternalServerStatus->StartTime;
	else
		return dtVal;
}
void CServerStatus::SetStartTime(OpcUa_DateTime date)
{
	if (m_pInternalServerStatus)
		m_pInternalServerStatus->StartTime=date;
}
// CurrentTime
OpcUa_DateTime CServerStatus::GetInternalCurrentTime()
{
	if (m_pInternalServerStatus)
		return m_pInternalServerStatus->CurrentTime;
	else
	{
		OpcUa_DateTime aNullDateTime;
		OpcUa_DateTime_Initialize(&aNullDateTime);
		return aNullDateTime;
	}
}
void CServerStatus::SetInternalCurrentTime(OpcUa_DateTime date)
{
	if (m_pInternalServerStatus)
		m_pInternalServerStatus->CurrentTime=date;
}
// ServerState
OpcUa_ServerState CServerStatus::GetServerState()
{
	if (m_pInternalServerStatus)
		return m_pInternalServerStatus->State;
	else
		return OpcUa_ServerState_NoConfiguration;
}
void CServerStatus::SetServerState(OpcUa_ServerState pState)
{
	if (m_pInternalServerStatus)
		m_pInternalServerStatus->State=pState;
}
// BuildInfo (from the C++ wrapper class)
//CBuildInfo* CServerStatus::GetBuildInfo()
//{	
//	if (m_pBuildInfo)
//		return m_pBuildInfo;
//	else
//		return OpcUa_Null;
//}
//void CServerStatus::SetBuildInfo(CBuildInfo* pBuildInfo)
//{
//	if (m_pInternalServerStatus)
//		OpcUa_MemCpy(&(m_pInternalServerStatus->BuildInfo),sizeof(OpcUa_BuildInfo),pBuildInfo->GetInternalBuildInfo(),sizeof(OpcUa_BuildInfo));
//	else
//		throw std::exception();
//}
// SecondsTillShutdown
OpcUa_UInt32 CServerStatus::GetSecondsTillShutdown()
{
	if (m_pInternalServerStatus)
		return m_pInternalServerStatus->SecondsTillShutdown;
	else
		throw std::exception();
}
void CServerStatus::SetSecondsTillShutdown(OpcUa_Int32 uiVal)
{
	if (m_pInternalServerStatus)
		m_pInternalServerStatus->SecondsTillShutdown=uiVal;
}
// ShutdownReason
OpcUa_LocalizedText CServerStatus::GetShutdownReason()
{
	if (m_pInternalServerStatus)
		return m_pInternalServerStatus->ShutdownReason;
	else
		throw std::exception();
}
void CServerStatus::SetShutdownReason(OpcUa_LocalizedText LtVal)
{

	if (m_pInternalServerStatus)
	{
		OpcUa_LocalizedText_Clear(&(m_pInternalServerStatus->ShutdownReason));
		OpcUa_LocalizedText_CopyTo(&LtVal,&(m_pInternalServerStatus->ShutdownReason));
	}
	else
		throw std::exception();
}
OpcUa_ServerStatusDataType* CServerStatus::GetInternalServerStatus() 
{ 
	return m_pInternalServerStatus; 
}