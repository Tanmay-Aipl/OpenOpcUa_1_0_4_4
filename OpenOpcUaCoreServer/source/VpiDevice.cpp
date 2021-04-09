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
#include "UAVariable.h"
#include "VpiTag.h"
#include "VpiFuncCaller.h"
#include "VpiWriteObject.h"
#include "VpiDevice.h"
#ifdef _GNUC_
#include <dlfcn.h>
#endif
#include "VPIScheduler.h"
#include "VPIScheduler.h"
using namespace OpenOpcUa;
using namespace UASubSystem;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CVpiDevice::CVpiDevice(const char **atts,CVpiFuncCaller* pVpiFuncCaller)
{
	OpcUa_String szName;
	OpcUa_String szVpiName;
	OpcUa_String szLibExtName;
	OpcUa_String_Initialize(&szVpiName);
	OpcUa_String_Initialize(&szLibExtName);
	m_InternalStatus = OpcUa_Uncertain;
	m_pVPIScheduler = new CVPIScheduler(this);
	SetVpiFuncCaller(pVpiFuncCaller);
	// Instance of the class that contains the the TagList handled by this Vpi
	OpcUa_Mutex_Create(&m_TagsMutex);
	m_pTags = new CVpiTagList();
	m_pTags->clear();
	int ii=0;
	while (atts[ii])
	{
		if (OpcUa_StrCmpA(atts[ii],"VpiName")==0)
		{
			OpcUa_String_AttachCopy(&szVpiName, atts[ii+1]);
			CVpiFuncCaller* pFuncCaller=GetVpiFuncCaller();
			if (pFuncCaller)
			{
#ifdef WIN32
				OpcUa_String_AttachCopy(&szLibExtName, ".dll");
				OpcUa_String_StrnCat(&szVpiName,&szLibExtName,4);
#else
				OpcUa_String_AttachCopy(&szLibExtName, ".so");
				OpcUa_String_StrnCat(&szVpiName,&szLibExtName,3);
#endif
				pFuncCaller->SetLibraryName(szVpiName);
			}
			else
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Critical error. Cannot initialize Low level layer\n");
		}
		if (OpcUa_StrCmpA(atts[ii],"SubSystemName")==0)
		{
			OpcUa_String_Initialize(&szName);
			OpcUa_String_AttachCopy(&szName, atts[ii+1]);
			SetName(szName);
		}
		if (OpcUa_StrCmpA(atts[ii],"SubSystemId")==0)
		{
			const char* szNodeId = atts[ii + 1];
			OpcUa_NodeId aNodeId;
			OpcUa_NodeId_Initialize(&aNodeId);					
			if (ParseNodeId(szNodeId, &aNodeId) == OpcUa_Good)
				SetSubSystemId(aNodeId);
			else
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"CVpiDevice::CVpiDevice() cannot parse this alias or this nodeId. Your configuration file is corruped\n");
			OpcUa_NodeId_Clear(&aNodeId);
		}
		if (OpcUa_StrCmpA(atts[ii],"AccessMode")==0)
		{
			if (OpcUa_StrCmpA(atts[ii+1],"Poll")==0)
				SetAccessDataMode(UASubSystem::POLL);
			if (OpcUa_StrCmpA(atts[ii+1],"Subscribe")==0)
				SetAccessDataMode(UASubSystem::SUBSCRIBE);
			if (OpcUa_StrCmpA(atts[ii+1],"Both")==0)
				SetAccessDataMode(UASubSystem::BOTH);
		}
		ii+=2;
	}
	OpcUa_String_Clear(&szVpiName);
	OpcUa_String_Clear(&szLibExtName);
}

CVpiDevice::~CVpiDevice()
{
	if (m_pVPIScheduler)
		delete m_pVPIScheduler;
	// delete the caller Object associate with the VpiDevice
	OpcUa_Mutex_Lock(m_TagsMutex);
	if (m_pTags)
	{
		for (OpcUa_UInt32 ii=0;ii<m_pTags->size();ii++)
		{
			CVpiTag* pSignal=m_pTags->at(ii);
			delete pSignal;
		}
		m_pTags->clear();
		delete m_pTags;
		m_pTags = OpcUa_Null;
	}
	OpcUa_Mutex_Unlock(m_TagsMutex);
	OpcUa_Mutex_Delete(&m_TagsMutex);
	CVpiFuncCaller* pVpiFunctionCaller = GetVpiFuncCaller();
	if (pVpiFunctionCaller!= OpcUa_Null)
		delete pVpiFunctionCaller;
}

void CVpiDevice::SetName(OpcUa_String szName) 
{ 
	m_szName = szName; 
}
OpcUa_String CVpiDevice::GetName()
{ 
	return m_szName; 
}
OpcUa_NodeId CVpiDevice::GetSubSystemId() 
{ 
	return m_SubSystemId; 
}
void CVpiDevice::SetSubSystemId(OpcUa_NodeId aNodeId)
{
	OpcUa_NodeId_CopyTo(&aNodeId, &m_SubSystemId);
}
CVpiFuncCaller* CVpiDevice::GetVpiFuncCaller() 
{ 
	return m_pVpiFuncCaller; 
}
void CVpiDevice::SetVpiFuncCaller(CVpiFuncCaller* pVpiFuncCaller) 
{ 
	m_pVpiFuncCaller = pVpiFuncCaller; 
}

void CVpiDevice::AddTag(CVpiTag* pTag)
{
	OpcUa_Mutex_Lock(m_TagsMutex);
	m_pTags->push_back(pTag);
	OpcUa_Mutex_Unlock(m_TagsMutex);
}
CVpiTagList* CVpiDevice::GetTags() 
{ 
	return m_pTags; 
}
unsigned int CVpiDevice::GetAccessDataMode() 
{ 
	return m_uiAccessDataMode; 
}
void CVpiDevice::SetAccessDataMode(unsigned int	 uiVal) 
{ 
	m_uiAccessDataMode = uiVal; 
}
CVPIScheduler* CVpiDevice::GetVPIScheduler()
{ 
	return m_pVPIScheduler; 
}
void CVpiDevice::AddWriteObject(CVpiWriteObject* pVpiWriteObject)
{
	if (m_pVPIScheduler)
		m_pVPIScheduler->AddWriteObject(pVpiWriteObject);
}
void CVpiDevice::SetSamplingInterval(OpcUa_Double dblVal)
{
	if (m_pVPIScheduler)
		m_pVPIScheduler-> SetSamplingInterval(dblVal);
}

void CVpiDevice::Wakeup()
{
	// Wake up Read Thread
	OpcUa_Semaphore_Post(m_pVPIScheduler->m_hVpiReaderInitialized, 1);
	// Wake up Write Thread
	OpcUa_Semaphore_Post(m_pVPIScheduler->m_hVpiWriterInitialized, 1);
}

OpcUa_StatusCode CVpiDevice::RemoveTag(CVpiTag* pTag)
{
	OpcUa_StatusCode uStatus = OpcUa_BadNotFound;
	OpcUa_Mutex_Lock(m_TagsMutex);
	if (m_pTags)
	{
		CVpiTagList::iterator it = m_pTags->begin();
		for (OpcUa_UInt32 i = 0; i < m_pTags->size(); i++)
		{
			CVpiTag* pVpiTag = m_pTags->at(i);
			if (pVpiTag == pTag)
			{
				delete pTag;
				m_pTags->erase(it);				
				uStatus = OpcUa_Good;
				break;
			}
		}
	}
	OpcUa_Mutex_Unlock(m_TagsMutex);
	return uStatus;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Gets internal status. </summary>
///
/// <remarks>	Michel, 05/08/2016. </remarks>
///
/// <returns>	The internal status. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode OpenOpcUa::UASubSystem::CVpiDevice::GetInternalStatus(void)
{
	return m_InternalStatus;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Sets internal status. </summary>
///
/// <remarks>	Michel, 05/08/2016. </remarks>
///
/// <param name="uStatus">	The status. </param>
///-------------------------------------------------------------------------------------------------

void OpenOpcUa::UASubSystem::CVpiDevice::SetInternalStatus(OpcUa_StatusCode uStatus)
{
	m_InternalStatus = uStatus;
}

