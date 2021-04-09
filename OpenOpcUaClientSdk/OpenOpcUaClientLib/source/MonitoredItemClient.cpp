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
#include "OpenOpcUaClientLib.h"

#include "MonitoredItemBase.h"
#include "MonitoredItemClient.h"
#include "MonitoredItemsNotification.h"
#include "SubscriptionClient.h"
#include "ClientSession.h"
using namespace OpenOpcUa;
using namespace UASharedLib;
using namespace UACoreClient;
CMonitoredItemClient::CMonitoredItemClient(void)
{
	m_ClassName=std::string("UASharedLib::CMonitoredItemClient");
	m_pMonitoringFilter = OpcUa_Null;
}

CMonitoredItemClient::~CMonitoredItemClient(void)
{
	if (m_pMonitoringFilter)
	{
		OpcUa_ExtensionObject_Clear(m_pMonitoringFilter);
		OpcUa_Free(m_pMonitoringFilter);
	}
}
OpcUa_StatusCode CMonitoredItemClient::TranslateBrowsePathsToNodeIds(void)
{
	CChannel* pChannel=NULL;
	OpcUa_StatusCode uStatus;
	OpcUa_RequestHeader tRequestHeader;
	// OpcUa_TimestampsToReturn  a_eTimestampsToReturn;
	// OpcUa_TimestampsToReturn_Source  = 0
	// OpcUa_TimestampsToReturn_Server  = 1
	// OpcUa_TimestampsToReturn_Both    = 2
	// OpcUa_TimestampsToReturn_Neither = 3
	OpcUa_ResponseHeader tResponseHeader;
	//OpcUa_Int32 tNoOfResults=0;
	OpcUa_MonitoredItemCreateResult* tResults=NULL;
	OpcUa_Int32 tNoOfDiagnosticInfos=0;
	OpcUa_DiagnosticInfo*	tDiagnosticInfos=NULL;

	OpcUa_QualifiedName_Initialize(&m_dataEncoding);
	OpcUa_ResponseHeader_Initialize(&tResponseHeader);
	OpcUa_String_Initialize(m_pIndexRange);
	OpcUa_RequestHeader_Initialize(&tRequestHeader);
	tRequestHeader.TimeoutHint = g_ServiceCallTimeout;// UTILS_DEFAULT_TIMEOUT;
	tRequestHeader.Timestamp   = OpcUa_DateTime_UtcNow();
	OpcUa_NodeId* pNodeId= m_pSession->GetAuthenticationToken();
	if (pNodeId)
		OpcUa_NodeId_CopyTo(pNodeId, &(tRequestHeader.AuthenticationToken));
	//tRequestHeader.RequestHandle = CClientApplication::m_uiRequestHandle++;
	OpcUa_MonitoredItemCreateResult_Initialize(tResults);

	OpcUa_DiagnosticInfo_Initialize(tDiagnosticInfos);
	pChannel=((CSessionClient*)m_pSession)->GetChannel();
	if (pChannel)
	{
		// We add a new call the service TranslateBrowsePathsToNodeIds
		OpcUa_Int32 iNoOfBrowsePaths=1;
		OpcUa_Int32 NoOfBPResults;
		OpcUa_BrowsePath aBrowsePaths;
		OpcUa_BrowsePath_Initialize(&aBrowsePaths);
		OpcUa_BrowsePathResult* pBPResults;
		aBrowsePaths.StartingNode=GetNodeId();
		aBrowsePaths.RelativePath.NoOfElements=1;
		aBrowsePaths.RelativePath.Elements=(OpcUa_RelativePathElement*)malloc(sizeof(OpcUa_RelativePathElement));
		aBrowsePaths.RelativePath.Elements[0].IncludeSubtypes=true;
		aBrowsePaths.RelativePath.Elements[0].IsInverse=false;
		// NodeId à traduire
		OpcUa_NodeId aNodeId;
		aNodeId.Identifier.Numeric=33; // HierarchicalReferences
		aNodeId.IdentifierType=OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex=0;
		aBrowsePaths.RelativePath.Elements[0].ReferenceTypeId=aNodeId;
		// Target Name
		OpcUa_String_CopyTo(m_pMonitoredItemName, &(aBrowsePaths.RelativePath.Elements[0].TargetName.Name));		
		aBrowsePaths.RelativePath.Elements[0].TargetName.NamespaceIndex=aBrowsePaths.StartingNode.NamespaceIndex;
		 
		uStatus=OpcUa_ClientApi_TranslateBrowsePathsToNodeIds(
									pChannel->GetInternalHandle(),
									&tRequestHeader,
									iNoOfBrowsePaths,
									&aBrowsePaths,
									&tResponseHeader,
									&NoOfBPResults,
									&pBPResults,
									&tNoOfDiagnosticInfos,
									&tDiagnosticInfos);
		if (uStatus!=OpcUa_Good)
			OpcUa_Trace(OPCUA_TRACE_CLIENT_LEVEL_ERROR, "OpcUa_ClientApi_TranslateBrowsePathsToNodeIds failed. uStatus 0x%X\n", uStatus);
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}
/// <summary>
/// Gets the subscription as a list of XML attributes.
/// </summary>
/// <param name="atts">The atts.</param>
/// <returns></returns>
OpcUa_StatusCode CMonitoredItemClient::GetXmlAttributes(vector<OpcUa_String*>* szAttributes)
{
	OpcUa_StatusCode uStatus = OpcUa_BadNotFound;
	OpcUa_String* pszLocalAtts = OpcUa_Null;
	OpcUa_String szTmpstring;
	OpcUa_String_Initialize(&szTmpstring);

	//vector<OpcUa_String*> szAttributes;
	char* buf = OpcUa_Null;

	// NodeId
	OpcUa_String szNodeId;
	OpcUa_String_Initialize(&szNodeId);
	OpenOpcUa_NodeIdToString(m_NodeId, &szNodeId);
	// Attribute
	OpcUa_String_AttachCopy(&szTmpstring, "NodeId");
	pszLocalAtts = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszLocalAtts);
	OpcUa_String_StrnCpy(pszLocalAtts, &szTmpstring, OpcUa_String_StrLen(&szTmpstring));
	szAttributes->push_back(pszLocalAtts);
	// Attribute value
	OpcUa_String_Clear(&szTmpstring);
	OpcUa_String_StrnCpy(&szTmpstring, &szNodeId, OpcUa_String_StrLen(&szNodeId));
	pszLocalAtts = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszLocalAtts);
	OpcUa_String_StrnCpy(pszLocalAtts, &szTmpstring, OpcUa_String_StrLen(&szTmpstring));
	szAttributes->push_back(pszLocalAtts);

	OpcUa_String_Clear(&szTmpstring);
	// AttributeId int
	// Attribute
	OpcUa_String_AttachCopy(&szTmpstring, "AttributeId");
	pszLocalAtts = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszLocalAtts);
	OpcUa_String_StrnCpy(pszLocalAtts, &szTmpstring, OpcUa_String_StrLen(&szTmpstring));
	szAttributes->push_back(pszLocalAtts);
	// Attribute value
	OpcUa_String_Clear(&szTmpstring);

	OpcUa_String szAttributeId;
	OpcUa_String_Initialize(&szAttributeId);
	buf = (char*)malloc(20);
	if (buf)
	{
		ZeroMemory(buf, 20);
		sprintf(buf, "%u", (unsigned int)m_attributeId);
		OpcUa_String_AttachCopy(&szAttributeId, buf);
		OpcUa_Free(buf);
	}
	OpcUa_String_StrnCpy(&szTmpstring, &szAttributeId, OpcUa_String_StrLen(&szAttributeId));
	pszLocalAtts = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszLocalAtts);
	OpcUa_String_StrnCpy(pszLocalAtts, &szTmpstring, OpcUa_String_StrLen(&szTmpstring));
	szAttributes->push_back(pszLocalAtts);

	OpcUa_String_Clear(&szTmpstring);
	// IndexRange string
	// Attribute
	OpcUa_String_AttachCopy(&szTmpstring, "IndexRange");
	pszLocalAtts = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszLocalAtts);
	OpcUa_String_StrnCpy(pszLocalAtts, &szTmpstring, OpcUa_String_StrLen(&szTmpstring));
	szAttributes->push_back(pszLocalAtts);
	// Attribute value
	OpcUa_String_Clear(&szTmpstring);

	OpcUa_String_StrnCpy(&szTmpstring, m_pIndexRange, OpcUa_String_StrLen(m_pIndexRange));
	pszLocalAtts = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszLocalAtts);
	OpcUa_String_StrnCpy(pszLocalAtts, m_pIndexRange, OpcUa_String_StrLen(m_pIndexRange));
	szAttributes->push_back(pszLocalAtts);

	OpcUa_String_Clear(&szTmpstring);
	// MonitoringMode int 
	// Attribute
	OpcUa_String_AttachCopy(&szTmpstring, "MonitoringMode");
	pszLocalAtts = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszLocalAtts);
	OpcUa_String_StrnCpy(pszLocalAtts, &szTmpstring, OpcUa_String_StrLen(&szTmpstring));
	szAttributes->push_back(pszLocalAtts);
	// Attribute value
	OpcUa_String_Clear(&szTmpstring);

	OpcUa_String szMonitoringMode;
	OpcUa_String_Initialize(&szMonitoringMode);
	buf = (char*)malloc(20);
	if (buf)
	{
		ZeroMemory(buf, 20);
		sprintf(buf, "%d", m_monitoringMode);
		OpcUa_String_AttachCopy(&szMonitoringMode, buf);
		OpcUa_Free(buf);
	}
	OpcUa_String_StrnCpy(&szTmpstring, &szMonitoringMode, OpcUa_String_StrLen(&szMonitoringMode));
	pszLocalAtts = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszLocalAtts);
	OpcUa_String_StrnCpy(pszLocalAtts, &szTmpstring, OpcUa_String_StrLen(&szTmpstring));
	szAttributes->push_back(pszLocalAtts);

	OpcUa_String_Clear(&szTmpstring);
	// SamplingInterval double
	// Attribute
	OpcUa_String_AttachCopy(&szTmpstring, "SamplingInterval");
	pszLocalAtts = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszLocalAtts);
	OpcUa_String_StrnCpy(pszLocalAtts, &szTmpstring, OpcUa_String_StrLen(&szTmpstring));
	szAttributes->push_back(pszLocalAtts);
	// Attribute value
	OpcUa_String_Clear(&szTmpstring);

	OpcUa_String szSamplingInterval;
	OpcUa_String_Initialize(&szSamplingInterval);
	buf = (char*)malloc(20);
	if (buf)
	{
		ZeroMemory(buf, 20);
		sprintf(buf, "%lf", m_samplingInterval);
		OpcUa_String_AttachCopy(&szSamplingInterval, buf);
		OpcUa_Free(buf);
	}
	OpcUa_String_StrnCpy(&szTmpstring, &szSamplingInterval, OpcUa_String_StrLen(&szSamplingInterval));
	pszLocalAtts = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszLocalAtts);
	OpcUa_String_StrnCpy(pszLocalAtts, &szTmpstring, OpcUa_String_StrLen(&szTmpstring));
	szAttributes->push_back(pszLocalAtts);

	OpcUa_String_Clear(&szTmpstring);
	// QueueSize int 
	// Attribute
	OpcUa_String_AttachCopy(&szTmpstring, "QueueSize");
	pszLocalAtts = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszLocalAtts);
	OpcUa_String_StrnCpy(pszLocalAtts, &szTmpstring, OpcUa_String_StrLen(&szTmpstring));
	szAttributes->push_back(pszLocalAtts);
	// Attribute value
	OpcUa_String_Clear(&szTmpstring);

	OpcUa_String szQueueSize;
	OpcUa_String_Initialize(&szQueueSize);
	buf = (char*)malloc(20);
	if (buf)
	{
		ZeroMemory(buf, 20);
		sprintf(buf, "%u", (unsigned int)m_queueSize);
		OpcUa_String_AttachCopy(&szQueueSize, buf);
		OpcUa_Free(buf);
	}
	OpcUa_String_StrnCpy(&szTmpstring, &szQueueSize, OpcUa_String_StrLen(&szQueueSize));
	pszLocalAtts = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszLocalAtts);
	OpcUa_String_StrnCpy(pszLocalAtts, &szTmpstring, OpcUa_String_StrLen(&szTmpstring));
	szAttributes->push_back(pszLocalAtts);

	OpcUa_String_Clear(&szTmpstring);
	// DiscardOldest boolean
	// Attribute
	OpcUa_String_AttachCopy(&szTmpstring, "DiscardOldest");
	pszLocalAtts = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszLocalAtts);
	OpcUa_String_StrnCpy(pszLocalAtts, &szTmpstring, OpcUa_String_StrLen(&szTmpstring));
	szAttributes->push_back(pszLocalAtts);
	// Attribute value
	OpcUa_String_Clear(&szTmpstring);

	OpcUa_String szDiscardOldest;
	OpcUa_String_Initialize(&szDiscardOldest);
	buf = (char*)malloc(20);
	if (buf)
	{
		memset(buf, 0, 20);
		sprintf(buf, "%u", m_bDiscardOldest);
		OpcUa_String_AttachCopy(&szDiscardOldest, buf);
		OpcUa_Free(buf);
	}
	OpcUa_String_StrnCpy(&szTmpstring, &szDiscardOldest, OpcUa_String_StrLen(&szDiscardOldest));
	pszLocalAtts = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszLocalAtts);
	OpcUa_String_StrnCpy(pszLocalAtts, &szTmpstring, OpcUa_String_StrLen(&szTmpstring));
	szAttributes->push_back(pszLocalAtts);

	//
	//if (pszLocalAtts)
	//	OpcUa_String_Clear(pszLocalAtts);
	OpcUa_String_Clear(&szTmpstring);

	return uStatus;
}

void CMonitoredItemClient::SetFilterToUse(OpcUa_ExtensionObject* pExtensionObject)
{
	if (m_pMonitoringFilter)
		OpcUa_ExtensionObject_Clear(m_pMonitoringFilter);
	else
		m_pMonitoringFilter = (OpcUa_ExtensionObject*)OpcUa_Alloc(sizeof(OpcUa_ExtensionObject));
	OpcUa_ExtensionObject_Initialize(m_pMonitoringFilter);
	OpcUa_ExtensionObject_CopyTo(pExtensionObject, m_pMonitoringFilter);
}
OpcUa_ExtensionObject* CMonitoredItemClient::GetFilterToUse()
{
	return m_pMonitoringFilter;
}