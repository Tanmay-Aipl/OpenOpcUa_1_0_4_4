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
//#include "MonitoredItemBase.h"
#include "Subscription.h"

//#ifdef WIN32
//	#ifndef _WIN32_WCE
//	#include "atltrace.h"
//	#endif
//#endif
using namespace OpenOpcUa;
using namespace UASharedLib;
//
CSubscription::CSubscription(void)
{
	m_pSubscriptionId=0;
	m_Priority=0;
	m_pRevisedPublishingInterval=0;
	m_pRevisedMaxKeepAliveCount=0;
	m_pRevisedLifetimeCount=0;
	m_uiMaxNotificationsPerPublish=1;
	m_iMonitoringMode=OpcUa_MonitoringMode_Disabled;
	m_hSubscription=OpcUa_Null;
	m_pChannel=OpcUa_Null;
	m_pRevisedPublishingInterval=100; // 100 ms par défaut
	m_ClassName=std::string("UASharedLib::CSubscription");	
	m_bPublishingEnabled=OpcUa_False;
}

CSubscription::~CSubscription(void)
{
}
void CSubscription::SetChannel(CChannel* pChannel) 
{ 
	m_pChannel = pChannel; 
}
CChannel* CSubscription::GetChannel() 
{ 
	return m_pChannel; 
}
OpcUa_Handle CSubscription::GetHandle() 
{ 
	return m_hSubscription; 
}
void CSubscription::SetHandle(OpcUa_Handle hVal) 
{ 
	m_hSubscription = hVal; 
}
OpcUa_Boolean CSubscription::GetPublishingEnabled() 
{ 
	return m_bPublishingEnabled; 
}
void CSubscription::SetPublishingEnabled(OpcUa_Boolean bPublishingEnabled) 
{ 
	m_bPublishingEnabled = bPublishingEnabled; 
}
OpcUa_Int32 CSubscription::GetDefaultMonitoringMode()
{
	return m_iMonitoringMode;
}
void CSubscription::SetDefaultMonitoringMode(OpcUa_Int32 iVal)
{
	m_iMonitoringMode = (OpcUa_MonitoringMode)iVal;
}

OpcUa_UInt32 CSubscription::GetSubscriptionId() 
{ 
	return m_pSubscriptionId; 
}
void CSubscription::SetSubscriptionId(OpcUa_UInt32 val)
{
	m_pSubscriptionId = val;
}
// PublishingInterval
OpcUa_Double CSubscription::GetPublishingInterval() 
{ 
	return m_pRevisedPublishingInterval; 
}
void CSubscription::SetPublishingInterval(OpcUa_Double val)
{
	m_pRevisedPublishingInterval = val;
}
// LifetimeCount
OpcUa_UInt32 CSubscription::GetLifetimeCount() 
{ 
	return m_pRevisedLifetimeCount; 
}
void CSubscription::SetLifetimeCount(OpcUa_UInt32 val) 
{ 
	m_pRevisedLifetimeCount = val; 
}
// MaxKeepAliveCount
OpcUa_UInt32 CSubscription::GetMaxKeepAliveCount() 
{ 
	return m_pRevisedMaxKeepAliveCount; 
}
void CSubscription::SetMaxKeepAliveCount(OpcUa_UInt32 val)
{
	m_pRevisedMaxKeepAliveCount = val;
}
// Priority
OpcUa_Byte CSubscription::GetPriority()
{
	return m_Priority;
}
void CSubscription::SetPriority(OpcUa_Byte bVal)
{
	m_Priority = bVal;
}
// MaxNotificationsPerPublish;
OpcUa_UInt32 CSubscription::GetMaxNotificationsPerPublish() 
{ 
	return m_uiMaxNotificationsPerPublish; 
}
void CSubscription::SetMaxNotificationsPerPublish(OpcUa_UInt32 uiVal)
{
	if (uiVal>1000)
		uiVal = 1000;
	m_uiMaxNotificationsPerPublish = uiVal;
}