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
#include "UAObjectType.h"
#include "UAVariable.h"
#include "Field.h"
#include "Definition.h"
//#include "Field.h"
#include "Definition.h"
#include "UADataType.h"
#include "UAVariableType.h"
#include "UAMethod.h"
#include "UAView.h"
#include "UAObject.h"
#include "UAMonitoredItemNotification.h"
#include "UADataChangeNotification.h"
#include "EventDefinition.h"
using namespace UAEvents;
#include "MonitoredItemServer.h"
#include "UAEventNotificationList.h"
#include "QueuedPublishRequest.h"
#include "QueuedCallRequest.h"
#include "QueuedReadRequest.h"
#include "ContinuationPoint.h"
#include "UAVariable.h"
#include "UAReferenceType.h"

#include "SubscriptionServer.h"
#include "SessionSecurityDiagnosticsDataType.h"
#include "SessionDiagnosticsDataType.h"
#include "QueuedHistoryReadRequest.h"
#include "QueuedQueryFirstRequest.h"
#include "QueuedQueryNextRequest.h"
#include "SessionServer.h"

using namespace OpenOpcUa;
using namespace UACoreServer;
using namespace UASharedLib;
CUADataChangeNotification::CUADataChangeNotification(void)
{
	OpcUa_Mutex_Create(&m_hInternalDataChangeNotificationMutex);
	m_DataChangeNotificationType = NOTIFICATION_MESSAGE_UNKNOW;
	m_bAcked=OpcUa_False;
	m_UAMonitoredItemNotificationList.clear();
	m_uiSequenceNumber=1;
	m_uiMaxQueueSize=0;
	OpcUa_DateTime_Initialize(&m_PublishTime);
}
CUADataChangeNotification::CUADataChangeNotification(OpcUa_UInt32 uiSequenceNumber) :m_uiSequenceNumber(uiSequenceNumber)
{
	OpcUa_Mutex_Create(&m_hInternalDataChangeNotificationMutex);
	m_DataChangeNotificationType = NOTIFICATION_MESSAGE_UNKNOW;
	m_bAcked=OpcUa_False;
	m_UAMonitoredItemNotificationList.clear();
	m_uiMaxQueueSize=0;
	OpcUa_DateTime_Initialize(&m_PublishTime);
}

CUADataChangeNotification::~CUADataChangeNotification(void)
{
	OpcUa_Mutex_Lock(m_hInternalDataChangeNotificationMutex);
	CUAMonitoredItemNotificationList::iterator it;
	for (it = m_UAMonitoredItemNotificationList.begin(); it != m_UAMonitoredItemNotificationList.end();++it)
	{
		CUAMonitoredItemNotification* pMonitoredItemNotification = *it;
		delete pMonitoredItemNotification;
	}
	m_UAMonitoredItemNotificationList.clear();
	//
	m_uiQueueSizesMap.clear();
	OpcUa_Mutex_Unlock(m_hInternalDataChangeNotificationMutex);
	OpcUa_Mutex_Delete(&m_hInternalDataChangeNotificationMutex);
}
// Delete the MonitoredItemNotification Already sent to the client
OpcUa_Boolean CUADataChangeNotification::RemoveSentMonitoredItemNotification()
{
	OpcUa_Boolean bResult = OpcUa_False;
	CUAMonitoredItemNotificationList tmpUAMonitoredItemNotificationList;
	OpcUa_Mutex_Lock(m_hInternalDataChangeNotificationMutex);
	CUAMonitoredItemNotificationList::iterator it;
	it = m_UAMonitoredItemNotificationList.begin();
	while (it != m_UAMonitoredItemNotificationList.end())
	{
		CUAMonitoredItemNotification* pUAMonitoredItemNotification= *it;
		if (pUAMonitoredItemNotification->IsMonitoredNotificationSent())
		{
			OpcUa_UInt32 uiClientHandle = pUAMonitoredItemNotification->GetMonitoredItemNotification()->ClientHandle;
			map<OpcUa_UInt32, OpcUa_UInt32>::iterator iteratorQueueSize;
			iteratorQueueSize = m_uiQueueSizesMap.find(uiClientHandle);
			if (iteratorQueueSize != m_uiQueueSizesMap.end())
				m_uiQueueSizesMap.erase(iteratorQueueSize);
			delete pUAMonitoredItemNotification;
			pUAMonitoredItemNotification = OpcUa_Null;
		}
		else
		{
			tmpUAMonitoredItemNotificationList.push_back(pUAMonitoredItemNotification);
		}
		it++;
	}
	m_UAMonitoredItemNotificationList.clear();
	if (tmpUAMonitoredItemNotificationList.size()>0)
		m_UAMonitoredItemNotificationList.swap(tmpUAMonitoredItemNotificationList);
	if (m_UAMonitoredItemNotificationList.empty())
		bResult = OpcUa_True;
	OpcUa_Mutex_Unlock(m_hInternalDataChangeNotificationMutex);
	return bResult;
}
void CUADataChangeNotification::SetMaxQueueSize(OpcUa_UInt32 uiVal)
{ 
	m_uiMaxQueueSize=uiVal; 
}
OpcUa_StatusCode CUADataChangeNotification::AddMonitoredItemNotification(OpcUa_MonitoredItemNotification* pVal, OpcUa_UInt32	uiQueueSize, OpcUa_Boolean bDiscardOldest)
{
	OpcUa_StatusCode uStatus = OpcUa_BadNothingToDo;
	if (pVal)
	{
		//RemoveSentMonitoredItemNotification();
		OpcUa_Mutex_Lock(m_hInternalDataChangeNotificationMutex);
		SetMaxQueueSize(uiQueueSize);
		if (!IsAlreadyInTheTransmissionList(pVal))
		{
			uStatus = OpcUa_Good;
			OpcUa_UInt32 uiCurrentQueueSize = 0;
			map<OpcUa_UInt32, OpcUa_UInt32>::iterator iteratorQueueSize;
			iteratorQueueSize = m_uiQueueSizesMap.find(pVal->ClientHandle);
			if (iteratorQueueSize != m_uiQueueSizesMap.end())
				uiCurrentQueueSize = iteratorQueueSize->second;
			if ((uiCurrentQueueSize >= uiQueueSize) && (uiQueueSize != 0))
			{
				uStatus=OpcUa_GoodResultsMayBeIncomplete; // this report that the Queue was overflow
				if (uiQueueSize>1)
					pVal->Value.StatusCode ^= 1152; // mean DataValue and Info bit 1024(bit10) for DataValue and 128 (bit7) for overflow
				// Need to adjust the size to the queue
				if (!bDiscardOldest)
				{
					OpcUa_UInt32 uiClientHandle = iteratorQueueSize->first;
					CUAMonitoredItemNotification* pUALastMonitoredItemNotification = OpcUa_Null;
					// Because DiscardOldest==False we search for the last CUAMonitoredItemNotification for the uiClientHandle
					for (OpcUa_UInt32 i = 0; i < m_UAMonitoredItemNotificationList.size(); i++)
					{
						CUAMonitoredItemNotification* pUACurrentMonitoredItemNotification=m_UAMonitoredItemNotificationList.at(i);
						if (pUACurrentMonitoredItemNotification->GetMonitoredItemNotification()->ClientHandle == uiClientHandle)
						{
							pUALastMonitoredItemNotification = pUACurrentMonitoredItemNotification;
						}
					}

					if (pUALastMonitoredItemNotification)
					{
						OpcUa_MonitoredItemNotification* pMonitoredItemNotification = pUALastMonitoredItemNotification->GetMonitoredItemNotification();
						OpcUa_DataValue_CopyTo(&(pVal->Value), &(pMonitoredItemNotification->Value));
						pUALastMonitoredItemNotification->MonitoredNotificationSent(OpcUa_False);
					}
					else
						OpcUa_Trace(OPCUA_TRACE_SERVER_ERROR, "cannot find this clientHandle in m_UAMonitoredItemNotificationList\n");
				}
				else
				{
					CUAMonitoredItemNotificationList TmpUAMonitoredItemNotificationList;
					// remove on from the regular list
					OpcUa_UInt32 uiClientHandle = iteratorQueueSize->first;
					/*
					CUAMonitoredItemNotification* pUALastMonitoredItemNotification = OpcUa_Null;
					// Because DiscardOldest==False we search for the last CUAMonitoredItemNotification for the uiClientHandle
					for (OpcUa_UInt32 i = 0; i < m_UAMonitoredItemNotificationList.size(); i++)
					{
						CUAMonitoredItemNotification* pUACurrentMonitoredItemNotification = m_UAMonitoredItemNotificationList.at(i);
						if (pUACurrentMonitoredItemNotification->GetMonitoredItemNotification()->ClientHandle == uiClientHandle)
						{
							pUALastMonitoredItemNotification = pUACurrentMonitoredItemNotification;
							break;
						}
					}

					if (pUALastMonitoredItemNotification)
					{
						OpcUa_MonitoredItemNotification* pMonitoredItemNotification = pUALastMonitoredItemNotification->GetMonitoredItemNotification();
						OpcUa_DataValue_CopyTo(&(pVal->Value), &(pMonitoredItemNotification->Value));
						pUALastMonitoredItemNotification->MonitoredNotificationSent(OpcUa_False);
					}
					else
						OpcUa_Trace(OPCUA_TRACE_SERVER_ERROR, "cannot find this clientHandle in m_UAMonitoredItemNotificationList\n");
						*/


					OpcUa_Boolean bFound = OpcUa_False;
					CUAMonitoredItemNotificationList::iterator it;
					CUAMonitoredItemNotification* pUACurrentMonitoredItemNotification = OpcUa_Null;
					for (it = m_UAMonitoredItemNotificationList.begin(); it!=m_UAMonitoredItemNotificationList.end(); it++)
					{
						pUACurrentMonitoredItemNotification = *it;
						if (pUACurrentMonitoredItemNotification->GetMonitoredItemNotification()->ClientHandle == uiClientHandle)
						{
							bFound = OpcUa_True;
							delete pUACurrentMonitoredItemNotification;
							m_UAMonitoredItemNotificationList.erase(it);
							break;
						}
					} 
					//if (bFound)
					//{
					//	delete pUACurrentMonitoredItemNotification;
					//	m_UAMonitoredItemNotificationList.erase(it);
					//	CUAMonitoredItemNotificationList tmpUAMonitoredItemNotificationList;
					//	tmpUAMonitoredItemNotificationList.swap(m_UAMonitoredItemNotificationList);

					//	for (OpcUa_UInt32 i = 0; i < tmpUAMonitoredItemNotificationList.size(); i++)
					//		m_UAMonitoredItemNotificationList.push_back(tmpUAMonitoredItemNotificationList.at(i));
					//	tmpUAMonitoredItemNotificationList.clear();
					//}
					// Now add the new one
					CUAMonitoredItemNotification* pNewMonitoredItemNotification = new CUAMonitoredItemNotification(pVal, uiQueueSize);
					pNewMonitoredItemNotification->DiscardOldest(bDiscardOldest);
					m_UAMonitoredItemNotificationList.push_back(pNewMonitoredItemNotification);
				}
			}
			else
			{
				CUAMonitoredItemNotification* pNewMonitoredItemNotification = OpcUa_Null; 
				// update the m_uiQueueSizesMap
				if (iteratorQueueSize != m_uiQueueSizesMap.end())
					iteratorQueueSize->second++;
				else
					m_uiQueueSizesMap.insert(std::pair<OpcUa_UInt32, OpcUa_UInt32>(pVal->ClientHandle, uiCurrentQueueSize + 1));

				// Add in the m_UAMonitoredItemNotificationList
				pNewMonitoredItemNotification=new CUAMonitoredItemNotification(pVal, uiQueueSize);
				pNewMonitoredItemNotification->DiscardOldest(bDiscardOldest);
				m_UAMonitoredItemNotificationList.push_back(pNewMonitoredItemNotification);
			}
		}
		OpcUa_Mutex_Unlock(m_hInternalDataChangeNotificationMutex);
	}
	return uStatus;
}
OpcUa_Boolean CUADataChangeNotification::IsAlreadyInTheTransmissionList(OpcUa_MonitoredItemNotification* pVal)
{
	OpcUa_Boolean bResult = OpcUa_False;
	if (!IsAcked())
	{
		for (OpcUa_UInt32 ii = 0; ii < m_UAMonitoredItemNotificationList.size(); ii++)
		{
			CUAMonitoredItemNotification* pMonitoredItemNotification = m_UAMonitoredItemNotificationList.at(ii);
			if (pMonitoredItemNotification)
			{
				OpcUa_DataValue aDataValue = pMonitoredItemNotification->GetMonitoredItemNotification()->Value;
				if (pMonitoredItemNotification->GetMonitoredItemNotification()->ClientHandle == pVal->ClientHandle)
				{
					// We can't use this one because we must ignore some bits 1024(bit10) for DataValue and 128 (bit7) for overflow
					//if (OpcUa_DataValue_Compare(&(pVal->Value), &aDataValue) == 0) 
					OpcUa_StatusCode uMaskedStatusCode1 = pVal->Value.StatusCode & 2943; // 2943 = not 1152
					OpcUa_StatusCode uMaskedStatusCode2 = aDataValue.StatusCode & 2943;
					if ((OpcUa_Variant_Compare(&(pVal->Value.Value), &aDataValue.Value) == 0) 
						&& (OpcUa_DateTime_Compare(&pVal->Value.ServerTimestamp,&aDataValue.ServerTimestamp )==0)
						&& (OpcUa_DateTime_Compare(&pVal->Value.SourceTimestamp, &aDataValue.SourceTimestamp) == 0)
						&& (uMaskedStatusCode1 == uMaskedStatusCode2))
					{
						bResult = OpcUa_True;
						break;
					}
				}
			}
		}
	}
	return bResult;
}
// Will look for CUAMonitoredItemNotification with ClientHandle== clientHandle
// and remove it from the m_UAMonitoredItemNotificationList
// return : OpcUa_BadNoMatch is the CUADataChangeNotification is not acked
//			OpcUa_Good is all run correctly
OpcUa_StatusCode CUADataChangeNotification::RemoveMonitoredItemNotification(OpcUa_UInt32 clientHandle)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_Mutex_Lock(m_hInternalDataChangeNotificationMutex);
	if (IsAcked()) // we delete only the Acked CUADataChangeNotification. Confirm this with Nathan 
	{
		CUAMonitoredItemNotificationList tmpUAMonitoredItemNotificationList;
		CUAMonitoredItemNotificationList::iterator it;
		for (it = m_UAMonitoredItemNotificationList.begin(); it != m_UAMonitoredItemNotificationList.end();it++)
		{
			CUAMonitoredItemNotification* pMonitoredItemNotification=*it;
			if (pMonitoredItemNotification->IsMonitoredNotificationSent())
			{
				OpcUa_MonitoredItemNotification* pUAMonitoredItemNotification = pMonitoredItemNotification->GetMonitoredItemNotification();
				if (pUAMonitoredItemNotification->ClientHandle == clientHandle)
				{
					delete pMonitoredItemNotification;
					pMonitoredItemNotification = OpcUa_Null;
				}
				else
					tmpUAMonitoredItemNotificationList.push_back(pMonitoredItemNotification);
			}
			else
				tmpUAMonitoredItemNotificationList.push_back(pMonitoredItemNotification);
		}
		m_UAMonitoredItemNotificationList.clear();
		if (tmpUAMonitoredItemNotificationList.size()>0)
			m_UAMonitoredItemNotificationList.swap(tmpUAMonitoredItemNotificationList);
	}
	OpcUa_Mutex_Unlock(m_hInternalDataChangeNotificationMutex);
	return uStatus;
}
OpcUa_Boolean CUADataChangeNotification::IsAcked() 
{ 
	return m_bAcked; 
}
void CUADataChangeNotification::Acked()
{
	m_bAcked = OpcUa_True;
}
void CUADataChangeNotification::UnAcked()
{
	m_bAcked = OpcUa_False;
}
OpcUa_Boolean CUADataChangeNotification::IsKeepAlive()
{
	if (m_DataChangeNotificationType == NOTIFICATION_MESSAGE_KEEPALIVE)
		return OpcUa_True;
	else
		return OpcUa_False;
}
// Check that CUADataChangeNotification in unacked and contains 
// unsent CUAMonitoredItemNotification
// 
OpcUa_Boolean CUADataChangeNotification::IsSomethingToNotify()
{
	OpcUa_Boolean bResult=OpcUa_False;
	if (!IsAcked())
	{
		for (OpcUa_UInt32 ii = 0; ii < m_UAMonitoredItemNotificationList.size(); ii++)
		{
			CUAMonitoredItemNotification* pMonitoredItemNotification = m_UAMonitoredItemNotificationList.at(ii);
			if (pMonitoredItemNotification)
			{
				if (!pMonitoredItemNotification->IsMonitoredNotificationSent())
				{
					bResult = OpcUa_True;
					break;
				}
			}
		}
	}
	return bResult;
}
// Determine if the use of this handle in this DataChangeNotification is allowed according to the queueSize
//OpcUa_Boolean CUADataChangeNotification::IsItFitQueueSize(OpcUa_UInt32 clientHandle)
//{
//	OpcUa_Boolean bResult=OpcUa_True;
//	//return bResult;
//	OpcUa_Mutex_Lock(m_hInternalDataChangeNotificationMutex);
//	for (OpcUa_UInt32 ii = 0; ii < m_UAMonitoredItemNotificationList.size(); ii++)
//	{
//		CUAMonitoredItemNotification* pMonitoredItemNotification = m_UAMonitoredItemNotificationList.at(ii);
//		OpcUa_MonitoredItemNotification* pInternalMonitoredItemNotification = pMonitoredItemNotification->GetMonitoredItemNotification();
//		if (pInternalMonitoredItemNotification->ClientHandle == clientHandle)
//		{
//			OpcUa_UInt32 uiQueueSize = pMonitoredItemNotification->GetQueueSize();
//			if (uiQueueSize == 0)
//				uiQueueSize = OpcUa_UInt32_Max;
//			if ((uiQueueSize - 1) < m_UAMonitoredItemNotificationList.size())
//			{
//				if (!pMonitoredItemNotification->IsDiscardOldest())
//				{
//					bResult = OpcUa_False;
//					break;
//				}
//			}
//		}
//		//else
//		//{
//		//	if (pMonitoredItemNotification->IsDiscardOldest())
//		//	{
//		//		bResult = OpcUa_False;
//		//		break;
//		//	}
//		//}
//	}
//	OpcUa_Mutex_Unlock(m_hInternalDataChangeNotificationMutex);
//	return bResult;
//}

///-------------------------------------------------------------------------------------------------
/// <summary>	Gets client handle. </summary>
///
/// <remarks>	Michel, 25/01/2016. </remarks>
///
/// <returns>	The client handle. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_UInt32 CUADataChangeNotification::GetClientHandle(void)
{
	OpcUa_UInt32 clientHandle=0;
	if (m_UAMonitoredItemNotificationList.size() > 0)
	{
		OpcUa_MonitoredItemNotification* pMonitoredItemNotification = m_UAMonitoredItemNotificationList.at(0)->GetMonitoredItemNotification();
		clientHandle = pMonitoredItemNotification->ClientHandle;
	}
	return clientHandle;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Gets maximum queue size. </summary>
///
/// <remarks>	Michel, 25/01/2016. </remarks>
///
/// <param name="ClientHandle">	Handle of the client. </param>
///
/// <returns>	The maximum queue size. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_Int32 OpenOpcUa::UACoreServer::CUADataChangeNotification::GetMaxQueueSize(OpcUa_UInt32 ClientHandle)
{
	OpcUa_Int32 iQueueSize = 0;
	OpcUa_Mutex_Lock(m_hInternalDataChangeNotificationMutex);
	for (OpcUa_UInt32 ii = 0; ii < m_UAMonitoredItemNotificationList.size(); ii++)
	{
		CUAMonitoredItemNotification* pMonitoredItemNotification = m_UAMonitoredItemNotificationList.at(ii);
		OpcUa_MonitoredItemNotification* pInternalMonitoredItemNotification = pMonitoredItemNotification->GetMonitoredItemNotification();
		if (pInternalMonitoredItemNotification->ClientHandle == ClientHandle)
		{
			pMonitoredItemNotification->GetQueueSize();
		}
	}
	return iQueueSize;
}

OpcUa_UInt32 CUADataChangeNotification::GetSequenceNumber() 
{
	return m_uiSequenceNumber; 
}
void CUADataChangeNotification::SetSequenceNumber(OpcUa_UInt32 uiSequenceNumber)
{
	m_uiSequenceNumber = uiSequenceNumber;
}