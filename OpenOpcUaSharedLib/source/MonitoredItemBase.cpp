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
#include "MonitoredItemBase.h"
#include "NumericRange.h"
//#include "MonitoredItemQueue.h"
#include "Subscription.h"
//#include "AggregateCalculator.h"

using namespace OpenOpcUa;
using namespace UASharedLib;


CMonitoredItemBase::CMonitoredItemBase(void)
{
	m_ClassName=("UASharedLib::CMonitoredItemBase");
	
	OpcUa_Mutex_Create(&m_MonitoredItemMutex);
	OpcUa_Mutex_Lock(m_MonitoredItemMutex);
	m_pValue=(OpcUa_DataValue*)OpcUa_Alloc(sizeof(OpcUa_DataValue));
	if (m_pValue)
		OpcUa_DataValue_Initialize(m_pValue);
	m_attributeId = OpcUa_Attributes_Value;
	m_bChange = OpcUa_False;
	m_bDiscardOldest = OpcUa_False;
	m_bOverflow = OpcUa_False;
	m_clientHandle = 0;
	OpcUa_QualifiedName_Initialize(&m_dataEncoding);
	m_iDiagnosticsMasks = 0;
	m_pIndexRange = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(m_pIndexRange);
	m_lastError = OpcUa_Good;
	m_MonitoredItemId = 0;
	m_pMonitoredItemName=(OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	if (m_pMonitoredItemName)
		OpcUa_String_Initialize(m_pMonitoredItemName);
	m_monitoringMode=OpcUa_MonitoringMode_Disabled; 
	OpcUa_NodeId_Initialize(&m_NodeId);
	m_pSession=OpcUa_Null;
	m_pUserData=OpcUa_Null;
	m_queueSize = 0;
	m_samplingInterval = 100;
	m_eTimestampsToReturn = OpcUa_TimestampsToReturn_Both;
	OpcUa_Mutex_Unlock(m_MonitoredItemMutex);
}

CMonitoredItemBase::~CMonitoredItemBase(void)
{
	OpcUa_QualifiedName_Clear(&m_dataEncoding);
	if (m_pMonitoredItemName)
	{
		OpcUa_String_Clear(m_pMonitoredItemName);
		OpcUa_Free(m_pMonitoredItemName);
		m_pMonitoredItemName=OpcUa_Null;
	}
	OpcUa_Mutex_Lock(m_MonitoredItemMutex);
	if (m_pValue)
	{
		OpcUa_DataValue_Clear(m_pValue);
		OpcUa_Free(m_pValue);
		m_pValue = OpcUa_Null;
	}
	OpcUa_NodeId_Clear(&m_NodeId);
	OpcUa_Mutex_Unlock(m_MonitoredItemMutex);
	OpcUa_Mutex_Delete(&m_MonitoredItemMutex);
	if (m_pIndexRange)
	{
		OpcUa_String_Clear(m_pIndexRange);
		OpcUa_Free(m_pIndexRange);
		m_pIndexRange = OpcUa_Null;
	}
	m_ClassName.clear();
}
OpcUa_String* CMonitoredItemBase::GetMonitoredItemName() 
{ 
	return m_pMonitoredItemName; 
}
void CMonitoredItemBase::SetMonitoredItemName(OpcUa_String strName)
{
	if (m_pMonitoredItemName)
		OpcUa_String_Clear(m_pMonitoredItemName);
	OpcUa_String_Initialize(m_pMonitoredItemName);
	OpcUa_String_CopyTo(&strName, m_pMonitoredItemName);
}
OpcUa_DataValue* CMonitoredItemBase::GetValue() 
{ 
	return m_pValue; 
}
void CMonitoredItemBase::SetValue(OpcUa_DataValue* pVal)
{
	if (pVal)
	{
		OpcUa_DataValue_Clear(m_pValue);
		if (pVal->Value.ArrayType == OpcUa_VariantArrayType_Scalar)
		{
			if (pVal->Value.Datatype == OpcUaType_ExtensionObject)
			{
				if (pVal->Value.Value.ExtensionObject)
				{
					if (pVal->Value.Value.ExtensionObject->Encoding == OpcUa_ExtensionObjectEncoding_EncodeableObject)
					{
						if (m_pValue)
						{
							OpcUa_DataValue_Clear(m_pValue);
							OpcUa_Free(m_pValue);
							m_pValue = OpcUa_Null;
						}
						m_pValue = Utils::Copy(pVal);
						m_bChange = OpcUa_True;
					}
					else
					{
						OpcUa_DataValue_CopyTo(pVal, m_pValue);
						m_bChange = OpcUa_True;
					}
				}
			}
			else
			{
				OpcUa_DataValue_CopyTo(pVal, m_pValue);
				m_bChange = OpcUa_True;
			}
		}
		else
		{
			if (OpcUa_DataValue_CopyTo(pVal, m_pValue) == OpcUa_Good)
				m_bChange = OpcUa_True;
		}
	}
}

OpcUa_TimestampsToReturn   CMonitoredItemBase::GetTimestampsToReturn() 
{ 
	return m_eTimestampsToReturn; 
}
void CMonitoredItemBase::SetTimestampsToReturn(OpcUa_TimestampsToReturn eTimestampsToReturn) 
{ 
	m_eTimestampsToReturn = eTimestampsToReturn; 
}
OpcUa_NodeId CMonitoredItemBase::GetNodeId()
{
	return m_NodeId;
}
void CMonitoredItemBase::SetNodeId(OpcUa_NodeId aNodeId)
{
	OpcUa_NodeId_Initialize(&m_NodeId);
	OpcUa_NodeId_CopyTo(&aNodeId, &m_NodeId);
}
OpcUa_UInt32 CMonitoredItemBase::GetQueueSize() { return m_queueSize; }
void CMonitoredItemBase::SetQueueSize(OpcUa_UInt32 uiVal) { m_queueSize = uiVal; }
OpcUa_UInt32 CMonitoredItemBase::GetAttributeId() { return m_attributeId; }
void CMonitoredItemBase::SetAttributeId(OpcUa_UInt32 uiVal) { m_attributeId = uiVal; }
OpcUa_String* CMonitoredItemBase::GetIndexRange() 
{ 
	return m_pIndexRange; 
}
void CMonitoredItemBase::SetIndexRange(OpcUa_String* pIndexRange)
{
	if (OpcUa_String_StrLen(pIndexRange) > 0)
	{
		if (m_pIndexRange)
		{
			OpcUa_String_Initialize(m_pIndexRange);
			OpcUa_String_CopyTo(pIndexRange, m_pIndexRange);
		}
	}
}
OpcUa_QualifiedName CMonitoredItemBase::GetDataEncoding() { return m_dataEncoding; }
void CMonitoredItemBase::SetDataEncoding(OpcUa_QualifiedName qName)
{
	OpcUa_QualifiedName_Initialize(&m_dataEncoding);
	OpcUa_QualifiedName_CopyTo(&qName, &m_dataEncoding);
}

OpcUa_UInt32 CMonitoredItemBase::GetClientHandle() 
{ 
	return m_clientHandle; 
}
void CMonitoredItemBase::SetClientHandle(OpcUa_UInt32 uiVal) 
{ 
	m_clientHandle = uiVal; 
}
OpcUa_Boolean CMonitoredItemBase::IsDiscardOldest() 
{ 
	return m_bDiscardOldest; 
}
void CMonitoredItemBase::DiscardOldest(OpcUa_Boolean bVal) 
{ 
	m_bDiscardOldest = bVal; 
}
//OpcUa_ExtensionObject CMonitoredItemBase::GetFilterToUse() 
//{ 
//	return m_filterToUse; 
//}
//void CMonitoredItemBase::SetFilterToUse(OpcUa_ExtensionObject exObj) 
//{ 
//	OpcUa_ExtensionObject_CopyTo(&exObj, &m_filterToUse);
//}
OpcUa_StatusCode CMonitoredItemBase::GetLastError() { return m_lastError; }
void CMonitoredItemBase::SetLastError(OpcUa_StatusCode uStatus) { m_lastError = uStatus; }
OpcUa_Mutex CMonitoredItemBase::GetMonitoredItemMutex() { return m_MonitoredItemMutex; }
void CMonitoredItemBase::SetMonitoredItemCS(OpcUa_Mutex* critSect) { m_MonitoredItemMutex = critSect; }
OpcUa_Double CMonitoredItemBase::GetSamplingInterval() { return m_samplingInterval; }
void CMonitoredItemBase::SetSamplingInterval(OpcUa_Double dblVal) { m_samplingInterval = dblVal; }
OpcUa_MonitoringMode CMonitoredItemBase::GetMonitoringMode() { return m_monitoringMode; }
void CMonitoredItemBase::SetMonitoringMode(OpcUa_MonitoringMode mode)  { m_monitoringMode = mode; }
CSessionBase* CMonitoredItemBase::GetSession() { return m_pSession; }
void CMonitoredItemBase::SetSession(CSessionBase* pSession) { m_pSession = pSession; }
void* CMonitoredItemBase::GetUserData() { return m_pUserData; }
void CMonitoredItemBase::SetUserData(void* pData) { m_pUserData = pData; }

CMonitoredItemBase* CMonitoredItemBase::operator=(CMonitoredItemBase* pItem)
{
	m_pSession = pItem->m_pSession;
	// NodeId
	OpcUa_NodeId_Initialize(&m_NodeId);
	OpcUa_NodeId_CopyTo(&(pItem->m_NodeId), &m_NodeId);
	// 
	m_iDiagnosticsMasks = pItem->m_iDiagnosticsMasks;
	m_eTimestampsToReturn = pItem->m_eTimestampsToReturn;
	m_clientHandle = pItem->m_clientHandle;
	m_monitoringMode = pItem->m_monitoringMode;
	m_samplingInterval = pItem->m_samplingInterval;
	m_bDiscardOldest = pItem->m_bDiscardOldest;
	m_lastError = pItem->m_lastError;
	m_bOverflow = pItem->m_bOverflow;
	m_pUserData = pItem->m_pUserData;
	m_MonitoredItemMutex = pItem->m_MonitoredItemMutex; //
	OpcUa_QualifiedName_CopyTo(&(pItem->m_dataEncoding), &m_dataEncoding);
	OpcUa_String_CopyTo(pItem->m_pIndexRange, m_pIndexRange);	
	m_queueSize = pItem->m_queueSize;
	m_attributeId = pItem->m_attributeId;
	OpcUa_String_CopyTo(pItem->m_pMonitoredItemName, m_pMonitoredItemName);
	OpcUa_DataValue_CopyTo(pItem->m_pValue, m_pValue);
	m_MonitoredItemId = pItem->m_MonitoredItemId;
	return this;
}
OpcUa_Boolean CMonitoredItemBase::IsChanged() { return m_bChange; }
void CMonitoredItemBase::Changed(OpcUa_Boolean bVal) { m_bChange = bVal; }
void CMonitoredItemBase::SetClassName(std::string className) { m_ClassName = className; }
std::string CMonitoredItemBase::GetClassName() { return m_ClassName; }
OpcUa_UInt32	CMonitoredItemBase::GetMonitoredItemId() { return m_MonitoredItemId; }
void CMonitoredItemBase::SetMonitoredItemId(OpcUa_UInt32 uiMonitoredItemId) { m_MonitoredItemId = uiMonitoredItemId; }