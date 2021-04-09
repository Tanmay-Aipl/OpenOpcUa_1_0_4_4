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
#ifdef _GNUC_
#include <dlfcn.h>
#endif
#include "VpiFuncCaller.h"
#include "VpiTag.h"
#include "VpiWriteObject.h"
#include "VpiDevice.h"
#include "UAReferenceType.h"
#include "UAObjectType.h"
#include "Field.h"
#include "Definition.h"
#include "UADataType.h"
#include "UAVariableType.h"
#include "UAMethod.h"
#include "UAView.h"
#include "UAObject.h"
#include "Alias.h"
#include "SimulatedNode.h"
#include "SimulatedGroup.h"
#include "BuildInfo.h"
#include "ServerStatus.h"
#include "VPIScheduler.h"
#include "NamespaceUri.h"
#include "StatusCodeException.h"
#include "ContinuationPoint.h"
#include "SessionSecurityDiagnosticsDataType.h"
#include "SessionDiagnosticsDataType.h"
#include "luainc.h"
#include "LuaDebugger.h"
#include "LuaVirtualMachine.h"
#include "LuaScript.h"
#include "OpenOpcUaScript.h"
using namespace UAScript;
#include "FileVersionInfo.h"

#include "UAMonitoredItemNotification.h"
#include "UADataChangeNotification.h"
#include "EventDefinition.h"
#include "EventsEngine.h"
using namespace UAEvents;
#include "UAInformationModel.h"
#include "MonitoredItemServer.h"
#include "UAEventNotificationList.h"
#include "QueuedPublishRequest.h"
#include "UAStatusChangeNotification.h"
#include "SubscriptionServer.h"
#include "QueuedCallRequest.h"
#include "QueuedReadRequest.h"
#include "QueuedHistoryReadRequest.h"
#include "QueuedQueryFirstRequest.h"
#include "QueuedQueryNextRequest.h"
#include "SessionServer.h"
#include "UABinding.h"
#include "VfiDataValue.h"
#include "UAHistorianVariable.h"
#include "HaEngine.h"
using namespace UAHistoricalAccess;
#include "ServerApplication.h"
#include "EventDefinition.h"

using namespace OpenOpcUa;
using namespace UAAddressSpace;
using namespace UACoreServer;
using namespace UAEvents;
extern CServerApplication* g_pTheApplication;

OpcUa_Mutex CUAInformationModel::m_ServerCacheMutex;// section critique permettant de protéger la cache du serveur
CServerStatus* CUAInformationModel::m_pInternalServerStatus=NULL;
CUAInformationModel::CUAInformationModel()
{
	// Initialize LUA layer and load script file. 
	// We use one script file for all OpenOpcUa
	m_pLuaVm = new CLuaVirtualMachine();
	if (m_pLuaVm)
	{
		m_pLuaVm->InitialiseVM();
		m_pOpenOpcUaScript= new COpenOpcUaScript(m_pLuaVm);
		// load the lua default script file asscociated with the server
		if (m_pOpenOpcUaScript->CompileFile("OpenOpcUaCoreServer.lua"))
			OpcUa_Trace(OPCUA_TRACE_LEVEL_ALWAYS, "OpenOpcUaCoreServer.lua  was loaded properly\n");
		else
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "OpenOpcUaCoreServer.lua  was not loaded properly. Verify it's in the project folder.\n");
		// 
		lua_gettop((lua_State *)m_pLuaVm);
	}
	//
	m_pServerDiagnosticsSummaryDataType = (OpcUa_ServerDiagnosticsSummaryDataType*)OpcUa_Alloc(sizeof(OpcUa_ServerDiagnosticsSummaryDataType));
	OpcUa_ServerDiagnosticsSummaryDataType_Initialize(m_pServerDiagnosticsSummaryDataType);
	m_bServerDiagnosticsDefaultValue=OpcUa_False;
	// server diagnostics mutex
	OpcUa_Mutex_Create(&m_SessionDiagnosticsMutex);
	OpcUa_Mutex_Create(&m_SessionSecurityDiagnosticsMutex);
	OpcUa_Mutex_Create(&m_SamplingIntervalDiagnosticsArrayMutex);
	OpcUa_Mutex_Create(&m_SubscriptionDiagnosticsMutex);
	//
	OpcUa_Mutex_Create(&m_OnLoadNamespaceUrisMutex);
	OpcUa_Mutex_Create(&m_ServerCacheMutex);  // section critique permettant de protéger la cache du serveur
	OpcUa_Semaphore_Create(&m_SimulationSem,0,0x100);
	// 
	OpcUa_Semaphore_Create(&m_PostThreatmentSem, 0, 0x100);
	// initialisation des vecteurs qui contiendront l'informationModel
	m_pEventTypeList = new CUAObjectTypeList();
	m_pReferenceTypeList	=new CUAReferenceTypeList();
	m_pReferenceTypeList->clear();
	m_pUAObjectList			=new CUAObjectList();
	m_pUAObjectList->clear();
	m_pUAObjectTypeList		=new CUAObjectTypeList();
	m_pUAObjectTypeList->clear();
	m_pUAVariableTypeList	=new CUAVariableTypeList();
	m_pUAVariableTypeList->clear();
	m_pUAVariableList		=new CUAVariableList();
	m_pUAVariableList->clear();
	m_pAliasList			=new CAliasList();
	m_pAliasList->clear();
	m_pUAMethodList			=new CUAMethodList();
	m_pUAMethodList->clear();
	m_pUAViewList			= new CUAViewList();
	m_pUAViewList->clear();
	m_pUADataTypeList		=new CUADataTypeList();
	m_pUADataTypeList->clear();
	// initialisation de information sur le serveur a partir des ressources associé au serveur
#ifdef WIN32
	m_pFileVersionInfo = OpcUa_Null;
	m_pFileVersionInfo = new CFileVersionInfo();
	// Information sur la version du fichier (EXE)
	CFileVersionInfo* pFileVersion = GetFileVersionInfo();
	if (pFileVersion)
	{
		if (pFileVersion->Create())
		{
			WORD ind0 = pFileVersion->GetFileVersion(0);
			WORD ind1 = pFileVersion->GetFileVersion(1);
			WORD ind2 = pFileVersion->GetFileVersion(2);
			WORD ind3 = pFileVersion->GetFileVersion(3);
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "OpenOpcUaCoreServer %u.%u.%u.%u\n", ind3, ind2, ind1, ind0);
		}
	}
	else
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critical error> CFileVersionInfo is not initialized");
#endif
	// initialisation du m_InternalServerStatus
	CUAInformationModel::m_pInternalServerStatus=new CServerStatus();
	CUAInformationModel::m_pInternalServerStatus->SetStartTime(OpcUa_DateTime_UtcNow());
	CUAInformationModel::m_pInternalServerStatus->SetServerState(OpcUa_ServerState_Unknown);

	// Demarrage de la thread qui assure le mise à jour des valeurs mandatory
	m_bRunUpdateMandatoryNodesThread=TRUE;
	m_hUpdateMandatoryNodesThread=NULL;
	OpcUa_Semaphore_Create(&m_hStopUpdateMandatoryNodesThreadSem,1,1);
	OpcUa_Semaphore_Create(&m_SemMandatoryEvent,0,0x100);
	StartUpdateMandatoryNodesThread();
	// Simulated member
	OpcUa_Mutex_Create(&m_SimulatedGroupMutex);
	m_pSimulatedGroupList = new CSimulatedGroupList();

	// initialisagtion des deux premier namespace uri
	// celui de la fondation OPC et celui du serveur local
	// préparation des elements du tableau (2 element)
	// Remplissage du namespace de l'OPC Foundation index=0
	OpcUa_String* opcfString=(OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	if (opcfString)
	{
		OpcUa_String_Initialize(opcfString);
		OpcUa_String_AttachCopy(opcfString, "http://opcfoundation.org/UA/");
		// creation du CNamespaceUri Associé
		CNamespaceUri* pNamespaceUri = new CNamespaceUri();
		pNamespaceUri->SetUriName(opcfString);
		pNamespaceUri->SetRelativeIndex(0);
		AddNamespaceUri(pNamespaceUri);
		OpcUa_String_Clear(opcfString);
		OpcUa_Free(opcfString);
	}
	// Remplissage du namespace du serveur local index=1
	OpcUa_CharA* szMyNameSpace = OpcUa_Null;//{"urn:MARS:UA_CPP_SDK:UA4CEServer"};
	BuildApplicationUri(&szMyNameSpace);
	OpcUa_String myString;
	OpcUa_String_Initialize(&myString);	
	if (szMyNameSpace)
	{
		OpcUa_String_AttachCopy(&myString, (OpcUa_StringA)szMyNameSpace);
		// creation du CNamespaceUri Associé

		CNamespaceUri* pNamespaceUri1=new CNamespaceUri();
		pNamespaceUri1->SetUriName(&myString);
		pNamespaceUri1->SetRelativeIndex(1);
		AddNamespaceUri(pNamespaceUri1);
		OpcUa_Free(szMyNameSpace);
	}
	OpcUa_String_Clear(&myString);
	// Préparation de la zone d'accès rapide
	
	for (OpcUa_UInt16 ii=0;ii<UAINFORMATIONMODEL_FASTACCESS_SIZE;ii++)
	{
		CUAInformationModelFastAccess* pFastAccessElt=new CUAInformationModelFastAccess();
		m_UAInformationModelFastAccessList.push_back(pFastAccessElt);
	}
}
CUAInformationModel::CUAInformationModel(const char **atts)
{
	// Initialize LUA layer and load script file. 
	// We use one script file for all OpenOpcUa
	//m_pDbg = OpcUa_Null;
	m_pLuaVm = new CLuaVirtualMachine();
	if (m_pLuaVm)
	{
		m_pLuaVm->InitialiseVM();
		CLuaDebugger* pDbg = new CLuaDebugger(m_pLuaVm);
		pDbg->SetCount(10);
		m_pOpenOpcUaScript = new COpenOpcUaScript(m_pLuaVm);
		// load the lua default script file asscociated with the server
		if (m_pOpenOpcUaScript->CompileFile("OpenOpcUaCoreServer.lua"))
			OpcUa_Trace(OPCUA_TRACE_LEVEL_ALWAYS, "OpenOpcUaCoreServer.lua  was loaded properly\n");
		else
			OpcUa_Trace(OPCUA_TRACE_LEVEL_ALWAYS, "OpenOpcUaCoreServer.lua  was not found or was not loaded properly\n");
		// 
	}
	//
	m_pServerDiagnosticsSummaryDataType = (OpcUa_ServerDiagnosticsSummaryDataType*)OpcUa_Alloc(sizeof(OpcUa_ServerDiagnosticsSummaryDataType));
	OpcUa_ServerDiagnosticsSummaryDataType_Initialize(m_pServerDiagnosticsSummaryDataType);
	// server diagnostics mutex
	OpcUa_Mutex_Create(&m_SessionDiagnosticsMutex);
	OpcUa_Mutex_Create(&m_SessionSecurityDiagnosticsMutex);
	OpcUa_Mutex_Create(&m_SamplingIntervalDiagnosticsArrayMutex);
	OpcUa_Mutex_Create(&m_SubscriptionDiagnosticsMutex);

	m_bServerDiagnosticsDefaultValue=OpcUa_False;
	OpcUa_ReferenceParameter(atts);
	OpcUa_Mutex_Create(&m_OnLoadNamespaceUrisMutex);
	OpcUa_Mutex_Create(&m_ServerCacheMutex); // section critique permettant de protéger la cache du serveur
	OpcUa_Semaphore_Create(&m_SimulationSem,0,0x100);
	//
	OpcUa_Semaphore_Create(&m_PostThreatmentSem, 0, 0x100);
	// initialisation des vecteurs qui contiendront l'informationModel
	m_pEventTypeList = new CUAObjectTypeList();
	m_pReferenceTypeList=new CUAReferenceTypeList();
	m_pReferenceTypeList->clear();
	m_pUAObjectList=new CUAObjectList();
	m_pUAObjectList->clear();
	m_pUAObjectTypeList=new CUAObjectTypeList();
	m_pUAObjectTypeList->clear();
	m_pUAVariableTypeList=new CUAVariableTypeList();
	m_pUAVariableTypeList->clear();
	m_pUAVariableList=new CUAVariableList();
	m_pUAVariableList->clear();	
	m_pAliasList=new CAliasList();
	m_pAliasList->clear();
	m_pUAMethodList=new CUAMethodList();
	m_pUAMethodList->clear();
	m_pUAViewList= new CUAViewList();
	m_pUAViewList->clear();
	m_pUADataTypeList=new CUADataTypeList();
	m_pUADataTypeList->clear();
	// initialisation de information sur le serveur a partir des ressources associé au serveur
#ifdef WIN32
	m_pFileVersionInfo = OpcUa_Null;
	m_pFileVersionInfo = new CFileVersionInfo();
	// Information sur la version du fichier (EXE)
	if ( GetFileVersionInfo()->Create())
	{
		WORD ind0= GetFileVersionInfo()->GetFileVersion(0);
		WORD ind1= GetFileVersionInfo()->GetFileVersion(1);
		WORD ind2= GetFileVersionInfo()->GetFileVersion(2);
		WORD ind3= GetFileVersionInfo()->GetFileVersion(3);
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"OpenOpcUaCoreServer %u.%u.%u.%u\n",ind3,ind2,ind1,ind0);
	}
#endif
	// Demarrage de la thread qui assure le mise à jour des valeurs mandatory
	m_bRunUpdateMandatoryNodesThread=TRUE;
	m_hUpdateMandatoryNodesThread=NULL;
	OpcUa_Semaphore_Create(&m_hStopUpdateMandatoryNodesThreadSem,1,1);
	OpcUa_Semaphore_Create(&m_SemMandatoryEvent,0,0x100);
	StartUpdateMandatoryNodesThread();
	// Simulated member
	OpcUa_Mutex_Create(&m_SimulatedGroupMutex);
	m_pSimulatedGroupList = new CSimulatedGroupList();
	// initialisagtion des deux premier namespace uri
	// celui de la fondation OPC et celui du serveur local
	// préparation des elements du tableau (2 element)
	// Remplissage du namespace de l'OPC Foundation index=0
	//char* szOPCFNameSpace={"http://opcfoundation.org/UA/"};
	OpcUa_String* opcfString=(OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(opcfString);
	OpcUa_String_AttachCopy(opcfString, "http://opcfoundation.org/UA/");
	// creation du CNamespaceUri Associé
	CNamespaceUri* pNamespaceUri=new CNamespaceUri();
	pNamespaceUri->SetUriName(opcfString);
	pNamespaceUri->SetRelativeIndex(0);
	AddNamespaceUri(pNamespaceUri);
	// Remplissage du namespace du serveur local index=1
	char* szMyNameSpace=NULL;//{"urn:MARS:UA_CPP_SDK:UA4CEServer"};
	BuildApplicationUri((OpcUa_CharA**)&szMyNameSpace);
	OpcUa_String myString;
	OpcUa_String_Initialize(&myString);
	//myString.strContent=(OpcUa_CharA*)OpcUa_Alloc(strlen(szMyNameSpace)+1);
	OpcUa_String_AttachCopy(&myString, (OpcUa_StringA)szMyNameSpace);
	// creation du CNamespaceUri Associé
	CNamespaceUri* pNamespaceUri1=new CNamespaceUri();
	pNamespaceUri1->SetUriName(&myString);
	pNamespaceUri1->SetRelativeIndex(1);
	//pNamespaceUri1->SetIndexReference(1);
	//m_NamespaceUris.push_back(pNamespaceUri1);
	AddNamespaceUri(pNamespaceUri1);
	// Préparation de la zone d'accès rapide
	for (OpcUa_UInt16 ii=0;ii<UAINFORMATIONMODEL_FASTACCESS_SIZE;ii++)
	{
		CUAInformationModelFastAccess* pFastAccessElt=new CUAInformationModelFastAccess();
		m_UAInformationModelFastAccessList.push_back(pFastAccessElt);
	}
	OpcUa_Free(opcfString);
}
CUAInformationModel::~CUAInformationModel() 
{
#ifdef WIN32
	delete m_pFileVersionInfo;
#endif
	//
	if (m_pServerDiagnosticsSummaryDataType)
	{
		OpcUa_ServerDiagnosticsSummaryDataType_Clear(m_pServerDiagnosticsSummaryDataType);
		OpcUa_Free(m_pServerDiagnosticsSummaryDataType);
		m_pServerDiagnosticsSummaryDataType = OpcUa_Null;
	}
	OpcUa_Semaphore_Delete(&m_PostThreatmentSem);
	//
	StopUpdateMandatoryNodesThread();
	OpcUa_Semaphore_Delete(&m_hStopUpdateMandatoryNodesThreadSem);
	delete m_pOpenOpcUaScript;
	//if (m_pDbg)
	//	delete m_pDbg;
	// Delete lua Virtual Machine
	if (m_pLuaVm)
	{
		delete m_pLuaVm;
		m_pLuaVm = OpcUa_Null;
	}
	// supression de m_pSimulatedGroupList
	OpcUa_Mutex_Lock(m_SimulatedGroupMutex);
	CSimulatedGroupList::iterator itSGL;
	while (!m_pSimulatedGroupList->empty())
	{
		itSGL = m_pSimulatedGroupList->begin();
		delete *itSGL;
		m_pSimulatedGroupList->erase(itSGL);
	}
	delete m_pSimulatedGroupList;
	OpcUa_Mutex_Unlock(m_SimulatedGroupMutex);
	OpcUa_Mutex_Delete(&m_SimulatedGroupMutex);
	// 
	// suppression de la fastAccessList
	CUAInformationModelFastAccessList::iterator it=m_UAInformationModelFastAccessList.begin();
	while (it!=m_UAInformationModelFastAccessList.end())
	{
		CUAInformationModelFastAccess* pFastAccessElt=*it;
		delete pFastAccessElt;
		it++;
	}
	m_UAInformationModelFastAccessList.clear();
	// supression de m_pReferenceTypeList
	CUAReferenceTypeList::iterator itRL;
	while (!m_pReferenceTypeList->empty())
	{
		itRL=m_pReferenceTypeList->begin();
		delete *itRL;
		m_pReferenceTypeList->erase(itRL);
	}
	delete m_pReferenceTypeList;
	// supression de m_pUAObjectList
	CUAObjectList::iterator itOL;
	for (itOL = m_pUAObjectList->begin(); itOL != m_pUAObjectList->end(); itOL++)
	{
		CUAObject* pUAObject = itOL->second;
		delete pUAObject;
		pUAObject = OpcUa_Null;
	}
	m_pUAObjectList->clear();
	delete m_pUAObjectList;
	// supression de m_pUAObjectTypeList
	CUAObjectTypeList::iterator itOTL;
	while (!m_pUAObjectTypeList->empty())
	{
		itOTL=m_pUAObjectTypeList->begin();
		delete *itOTL;
		m_pUAObjectTypeList->erase(itOTL);
	}
	delete m_pUAObjectTypeList;
	// supression de m_pUAVariableTypeList
	CUAVariableTypeList::iterator itUVTL;
	while (!m_pUAVariableTypeList->empty())
	{
		itUVTL=m_pUAVariableTypeList->begin();
		delete *itUVTL;
		m_pUAVariableTypeList->erase(itUVTL);
	}
	delete m_pUAVariableTypeList;
	// delete UAVariableList
	CUAVariableList::iterator itVL;
	for (itVL = m_pUAVariableList->begin(); itVL != m_pUAVariableList->end();itVL++)
	{
		CUAVariable* pUAVariable=itVL->second; 
		delete pUAVariable;
		pUAVariable = OpcUa_Null;
	}
	m_pUAVariableList->clear();
	delete m_pUAVariableList;
	// supression de m_pUAViewList
	CUAViewList::iterator itViewL;
	while (!m_pUAViewList->empty())
	{
		itViewL=m_pUAViewList->begin();
		delete *itViewL;
		m_pUAViewList->erase(itViewL);
	}
	delete m_pUAViewList;
	// supression de m_pUAMethodList
	CUAMethodList::iterator itML;
	while (!m_pUAMethodList->empty())
	{
		itML=m_pUAMethodList->begin();
		delete *itML;
		m_pUAMethodList->erase(itML);
	}
	delete m_pUAMethodList;
	// supression de m_pAliasList
	CAliasList::iterator itAL;
	while (!m_pAliasList->empty())
	{
		itAL=m_pAliasList->begin();
		delete *itAL;
		m_pAliasList->erase(itAL);
	}
	delete m_pAliasList;

	// supression de m_pUADataTypeList
	CUADataTypeList::iterator itDTL;
	while (!m_pUADataTypeList->empty())
	{
		itDTL = m_pUADataTypeList->begin();
		delete *itDTL;
		m_pUADataTypeList->erase(itDTL);
	}
	delete m_pUADataTypeList;
	// supression de m_NamespaceUris
	std::vector<CNamespaceUri*>::iterator itNSU;
	while (!m_NamespaceUris.empty())
	{
		itNSU=m_NamespaceUris.begin();
		delete *itNSU;
		m_NamespaceUris.erase(itNSU);
	}
	// we just delete the m_pEventTypeList the content was already freed
	// the content will not be deleted because it was already deleted as part of m_pUAObjectTypeList
	if (m_pEventTypeList)
	{
		m_pEventTypeList->clear(); 
		delete m_pEventTypeList;
	}
	// 
	CUANamespaceUris::iterator itNsUri;
	for (itNsUri = m_OnLoadNamespaceUris.begin(); itNsUri != m_OnLoadNamespaceUris.end(); itNsUri++)
	{
		char* chFirst=itNsUri->first;
		if (chFirst)
		{
			OpcUa_Free(chFirst);
			chFirst = OpcUa_Null;
		}

	}
	m_OnLoadNamespaceUris.clear();
	OpcUa_Semaphore_Delete(&m_SimulationSem);
	// Delete the embedded server status and buildinfo
	delete CUAInformationModel::m_pInternalServerStatus;
	OpcUa_Mutex_Delete(&m_ServerCacheMutex);
	OpcUa_Mutex_Delete(&m_SubscriptionDiagnosticsMutex);
	// m_SamplingIntervalDiagnosticsArray
	std::vector<OpcUa_SamplingIntervalDiagnosticsDataType*>::iterator itSIDA;
	OpcUa_Mutex_Lock(m_SamplingIntervalDiagnosticsArrayMutex);
	for (itSIDA = m_SamplingIntervalDiagnosticsArray.begin(); itSIDA != m_SamplingIntervalDiagnosticsArray.end(); itSIDA++)
	{
		OpcUa_SamplingIntervalDiagnosticsDataType_Clear(*itSIDA);
		OpcUa_Free(*itSIDA);
	}
	m_SamplingIntervalDiagnosticsArray.clear();
	OpcUa_Mutex_Unlock(m_SamplingIntervalDiagnosticsArrayMutex);
	OpcUa_Mutex_Delete(&m_SamplingIntervalDiagnosticsArrayMutex);
	OpcUa_Mutex_Delete(&m_SessionDiagnosticsMutex);
	OpcUa_Mutex_Delete(&m_SessionSecurityDiagnosticsMutex);
	// Clear the OnLoad NamespaceUri list and delete the related mutex
	OnLoadNamespaceUriEraseAll();
	OpcUa_Mutex_Delete(&m_OnLoadNamespaceUrisMutex);
}
OpcUa_StatusCode CUAInformationModel::GetNodeIdFromFastAccessList(const OpcUa_NodeId iNodeId, CUABase** pUABase)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	// Tentative de recherche dans la FastAccess
	if (!m_UAInformationModelFastAccessList.empty())
	{
		if ((iNodeId.IdentifierType == OpcUa_IdentifierType_Numeric) && (iNodeId.NamespaceIndex == 0))
		{
			*pUABase = m_UAInformationModelFastAccessList[iNodeId.Identifier.Numeric]->GetUABase();
			if (*pUABase)
				return uStatus;
			else
				uStatus = OpcUa_BadNotFound;
		}
		else
			uStatus = OpcUa_BadNotFound;
	}
	else
		uStatus = OpcUa_BadNotFound;
	return uStatus;
}
// Function name   : CUAInformationModel::GetNodeIdFromDictionnary
// Description     : Recherche un node dans le dictionnaire
// Return type     : OpcUa_StatusCode 
// Argument        : OpcUa_NodeId iNodeId
// Argument        : CUABase** pUABase
// Argument        : int* iIndex

OpcUa_StatusCode CUAInformationModel::GetNodeIdFromDictionnary(const OpcUa_NodeId iNodeId, CUABase** pUABase)
{
	OpcUa_StatusCode uStatus=OpcUa_BadInvalidArgument ;
	// Tentative de recherche dans la FastAccess
	if (GetNodeIdFromFastAccessList(iNodeId,pUABase)==OpcUa_Good)
			return OpcUa_Good;
	
	// Recherche dans les Object
	CUAObject* pUAObject=NULL;
	uStatus=GetNodeIdFromObjectList(iNodeId,&pUAObject);
	if (uStatus!=OpcUa_Good)
	{
		// Recherche dans les ObjectType
		CUAObjectType* pUAObjectType=NULL;
		uStatus=GetNodeIdFromObjectTypeList(iNodeId,&pUAObjectType);
		if (uStatus==OpcUa_Good)
			*pUABase=(CUABase*)pUAObjectType;
		else
		{
			// Recherche dans les VariableType
			CUAVariableType* pUAVariableType=NULL;
			uStatus=GetNodeIdFromVariableTypeList(iNodeId,&pUAVariableType);
			if (uStatus==OpcUa_Good)
				*pUABase=(CUABase*)pUAVariableType;
			else
			{
				// Recherche dans les ReferenceType
				CUAReferenceType* pUAReferenceType=NULL;
				uStatus=GetNodeIdFromReferenceTypeList(iNodeId,&pUAReferenceType);
				if (uStatus==OpcUa_Good)
					*pUABase=(CUABase*)pUAReferenceType;
				else
				{
					// Recherche dans les Variables
					CUAVariable* pUAVariable=NULL;
					uStatus=GetNodeIdFromVariableList(iNodeId,&pUAVariable);
					if (uStatus==OpcUa_Good)
						*pUABase=(CUABase*)pUAVariable;
					else
					{
						// Recherche dans les Method
						CUAMethod* pUAMethod=NULL;
						uStatus=GetNodeIdFromMethodList(iNodeId,&pUAMethod);
						if (uStatus==OpcUa_Good)
							*pUABase=(CUABase*)pUAMethod;
						else
						{
							CUAView* pUAView=NULL;
							uStatus=GetNodeIdFromViewList(iNodeId,&pUAView);
							if (uStatus==OpcUa_Good)
								*pUABase=(CUABase*)pUAView;
							else
							{
								CUADataType* pUADataType=NULL;
								uStatus=GetNodeIdFromDataTypeList(iNodeId,&pUADataType);
								if (uStatus==OpcUa_Good)
									*pUABase=pUADataType;
							}
						}
					}
				}
			}
		}
	}
	else
		*pUABase=(CUABase*)pUAObject;
	return uStatus;
}
//parcours l'ensemble de l'espace d'adressage a la recherche du Node qui porte le browseName indiqué.
// cette recherche se déroule au sein d'un namespace.
OpcUa_StatusCode CUAInformationModel::GetNodeFromDictionnary(OpcUa_QualifiedName szBrowseName,CUABase** pUABase)
{
	OpcUa_StatusCode uStatus=OpcUa_BadInvalidArgument;
	CUAObject* pUAObject=OpcUa_Null;
	uStatus=GetNodeObjectFromBrowseName(szBrowseName,&pUAObject);
	if (uStatus!=OpcUa_Good)
	{
		CUAObjectType* pUAObjectType=NULL;
		uStatus=GetNodeObjectTypeFromBrowseName(szBrowseName,&pUAObjectType);
		if (uStatus!=OpcUa_Good)
		{
			CUAVariableType* pUAVariableType=NULL;
			uStatus=GetNodeVariableTypeFromBrowseName(szBrowseName,&pUAVariableType);
			if (uStatus!=OpcUa_Good)
			{
				CUAVariable* pUAVariable=NULL;
				uStatus=GetNodeVariableFromBrowseName(szBrowseName,&pUAVariable);
				if (uStatus!=OpcUa_Good)
				{
					CUAReferenceType* pUAReferenceType=NULL;
					uStatus=GetNodeReferenceTypeFromBrowseName(szBrowseName,&pUAReferenceType);
					if (uStatus!=OpcUa_Good)
					{
						CUAMethod* pUAMethod=NULL;
						uStatus=GetNodeMethodFromBrowseName(szBrowseName,&pUAMethod);
						if (uStatus!=OpcUa_Good)
							*pUABase=(CUABase*)pUAMethod;
						else
						{
							CUAView* pUAView=NULL;
							uStatus=GetNodeViewFromBrowseName(szBrowseName,&pUAView);
							if (uStatus!=OpcUa_Good)
								*pUABase=(CUABase*)pUAView;
						}
					}
					else
						*pUABase=(CUABase*)pUAReferenceType;
				}
			else
				*pUABase=(CUABase*)pUAVariable;

			}
			else
				*pUABase=(CUABase*)pUAVariableType;
		}
		else
			*pUABase=(CUABase*)pUAObjectType;
	}
	else
		*pUABase=(CUABase*)pUAObject;
	return uStatus;
}
// Function name   : CUAInformationModel::GetNodeIdObjectFromBrowseName
// Description     : Cette méthode retrouve l'UAObject associé a un browseName
// Return type     : OpcUa_StatusCode S_OK si trouvé, S_FALSE sinon trouvé, E_INVALIDARG en cas de param incorrecte
// Argument        : OpcUa_QualifiedName szBrowseName
// Argument        : CUAObject** pUAObject

OpcUa_StatusCode CUAInformationModel::GetNodeObjectFromBrowseName(OpcUa_QualifiedName szBrowseName, CUAObject** pUAObject)
{
	OpcUa_StatusCode uStatus = OpcUa_BadInvalidArgument;

	CUAObject* pTmpUAObject=NULL;
	CUAObjectList::iterator itOL;
	for (itOL = m_pUAObjectList->begin(); itOL != m_pUAObjectList->end(); itOL++)
	{
		pTmpUAObject = itOL->second;
		char* chTmp1=OpcUa_String_GetRawString(&(szBrowseName.Name));
		char* chTmp2=OpcUa_String_GetRawString(&(pTmpUAObject->GetBrowseName()->Name));
		size_t iLen1=OpcUa_String_StrLen(&(szBrowseName.Name));
		size_t iLen2=OpcUa_String_StrLen(&(pTmpUAObject->GetBrowseName()->Name));
		if ( (pTmpUAObject->GetBrowseName()->NamespaceIndex==szBrowseName.NamespaceIndex) && 
			(strncmp(chTmp1,chTmp2,iLen1)==0) & (iLen1==iLen2) ) 
		{
			*pUAObject=pTmpUAObject;
			//OpcUa_StatusCode uStatus = OpcUa_BadInvalidArgument;
			uStatus = OpcUa_Good;
			return uStatus;
		}
	}
	return uStatus;
}
// Function name   : CUAInformationModel::GetNodeIdObjectTypeFromBrowseName
// Description     : Cette méthode retrouve l'UAObject associé a un browseName
// Return type     : OpcUa_StatusCode S_OK si trouvé, S_FALSE sinon trouvé, E_INVALIDARG en cas de param incorrecte
// Argument        : OpcUa_QualifiedName szBrowseName
// Argument        : CUAObject** pUAObject

OpcUa_StatusCode CUAInformationModel::GetNodeObjectTypeFromBrowseName(OpcUa_QualifiedName szBrowseName, CUAObjectType** pUAObjectType)
{
	OpcUa_StatusCode uStatus = OpcUa_BadInvalidArgument;
	CUAObjectTypeList* pUAObjectTypeList=GetObjectTypeList();
	int iSize=pUAObjectTypeList->size();
	CUAObjectType* pTmpUAObjectType=NULL;
	for (OpcUa_Int32 iii=0;iii<iSize;iii++)
	{
		pTmpUAObjectType=((*pUAObjectTypeList)[iii]);
		char* chTmp1=OpcUa_String_GetRawString(&(szBrowseName.Name));
		char* chTmp2=OpcUa_String_GetRawString(&(pTmpUAObjectType->GetBrowseName()->Name));
		size_t iLen1=OpcUa_String_StrLen(&(szBrowseName.Name));
		size_t iLen2=OpcUa_String_StrLen(&(pTmpUAObjectType->GetBrowseName()->Name));

		if ( (pTmpUAObjectType->GetBrowseName()->NamespaceIndex==szBrowseName.NamespaceIndex) && 			
			(strncmp(chTmp1,chTmp2,iLen1)==0) & (iLen1==iLen2) ) 
		{
			*pUAObjectType=pTmpUAObjectType;
			uStatus = OpcUa_Good;
			return uStatus;
		}
	}
	return uStatus;
}

// Function name   : CUAInformationModel::GetNodeIdObjectTypeFromBrowseName
// Description     : Cette méthode retrouve l'AUObject associé a un browseName
// Return type     : OpcUa_StatusCode S_OK si trouvé, S_FALSE sinon trouvé, E_INVALIDARG en cas de param incorrecte
// Argument        : OpcUa_QualifiedName szBrowseName
// Argument        : CUAObject** pUAObject

OpcUa_StatusCode CUAInformationModel::GetNodeReferenceTypeFromBrowseName(OpcUa_QualifiedName szBrowseName, CUAReferenceType** pUAReferenceType)
{
	OpcUa_StatusCode uStatus = OpcUa_BadInvalidArgument;
	CUAReferenceTypeList* pUAReferenceTypeList=GetReferenceTypeList();
	int iSize=pUAReferenceTypeList->size();
	CUAReferenceType* pTmpUAReferenceType=NULL;
	
	for (int iii=0;iii<iSize;iii++)
	{
		pTmpUAReferenceType=((*pUAReferenceTypeList)[iii]);
		char* chTmp1=OpcUa_String_GetRawString(&(szBrowseName.Name));
		char* chTmp2=OpcUa_String_GetRawString(&(pTmpUAReferenceType->GetBrowseName()->Name));
		size_t iLen1=OpcUa_String_StrLen(&(szBrowseName.Name));
		size_t iLen2=OpcUa_String_StrLen(&(pTmpUAReferenceType->GetBrowseName()->Name));
		if ( (pTmpUAReferenceType->GetBrowseName()->NamespaceIndex==szBrowseName.NamespaceIndex) && 			
			(strncmp(chTmp1,chTmp2,iLen1)==0) & (iLen1==iLen2) ) 
			//(OpcUa_String_StrnCmp(&(szBrowseName.Name),&(pTmpUAReferenceType->GetBrowseName()->Name),OpcUa_String_StrLen(&(szBrowseName.Name)),OpcUa_False)==0) ) 
		{
			*pUAReferenceType=pTmpUAReferenceType;
			uStatus = OpcUa_Good;
			return uStatus;
		}
	}
	return uStatus;
}
// Function name   : CUAInformationModel::GetNodeIdObjectTypeFromBrowseName
// Description     : Cette méthode retrouve l'AUObject associé a un browseName
// Return type     : OpcUa_StatusCode S_OK si trouvé, S_FALSE sinon trouvé, E_INVALIDARG en cas de param incorrecte
// Argument        : OpcUa_QualifiedName szBrowseName
// Argument        : CUAObject** pUAObject

OpcUa_StatusCode CUAInformationModel::GetNodeVariableFromBrowseName(OpcUa_QualifiedName szBrowseName, CUAVariable** pUAVariable)
{
	OpcUa_StatusCode uStatus = OpcUa_BadInvalidArgument;
	CUAVariableList* pUAVariableList=GetVariableList();
	CUAVariable* pTmpUAVariable=NULL;
	
	CUAVariableList::iterator it;
	for (it = pUAVariableList->begin(); it != pUAVariableList->end(); it++)
	{
		pTmpUAVariable = it->second;
		char* chTmp1=OpcUa_String_GetRawString(&(szBrowseName.Name));
		char* chTmp2=OpcUa_String_GetRawString(&(pTmpUAVariable->GetBrowseName()->Name));
		size_t iLen1=OpcUa_String_StrLen(&(szBrowseName.Name));
		size_t iLen2=OpcUa_String_StrLen(&(pTmpUAVariable->GetBrowseName()->Name));
		if ( (pTmpUAVariable->GetBrowseName()->NamespaceIndex==szBrowseName.NamespaceIndex) && 			
			(strncmp(chTmp1,chTmp2,iLen1)==0) & (iLen1==iLen2) ) 
		{
			*pUAVariable=pTmpUAVariable;
			uStatus = OpcUa_Good;
			break;
		}
	}
	return uStatus;
}
// Function name   : CUAInformationModel::GetNodeIdObjectTypeFromBrowseName
// Description     : Cette méthode retrouve l'AUObject associé a un browseName
// Return type     : OpcUa_StatusCode S_OK si trouvé, S_FALSE sinon trouvé, E_INVALIDARG en cas de param incorrecte
// Argument        : OpcUa_QualifiedName szBrowseName
// Argument        : CUAObject** pUAObject

OpcUa_StatusCode CUAInformationModel::GetNodeVariableTypeFromBrowseName(OpcUa_QualifiedName szBrowseName, CUAVariableType** pUAVariableType)
{
	OpcUa_StatusCode uStatus = OpcUa_BadInvalidArgument;
	CUAVariableTypeList* pUAVariableTypeList=GetVariableTypeList();
	int iSize=pUAVariableTypeList->size();
	CUAVariableType* pTmpUAVariableType=NULL;
	
	for (int iii=0;iii<iSize;iii++)
	{
		pTmpUAVariableType=((*pUAVariableTypeList)[iii]);
		char* chTmp1=OpcUa_String_GetRawString(&(szBrowseName.Name));
		char* chTmp2=OpcUa_String_GetRawString(&(pTmpUAVariableType->GetBrowseName()->Name));
		size_t iLen1=OpcUa_String_StrLen(&(szBrowseName.Name));
		size_t iLen2=OpcUa_String_StrLen(&(pTmpUAVariableType->GetBrowseName()->Name));
		if ( (pTmpUAVariableType->GetBrowseName()->NamespaceIndex==szBrowseName.NamespaceIndex) && 		
			(strncmp(chTmp1,chTmp2,iLen1)==0) & (iLen1==iLen2) ) 
			//	(OpcUa_String_StrnCmp(&(szBrowseName.Name),&(pTmpUAVariableType->GetBrowseName()->Name),OpcUa_String_StrLen(&(szBrowseName.Name)),OpcUa_False)==0) ) 
		{
			*pUAVariableType=pTmpUAVariableType;
			return OpcUa_Good;
		}
	}
	return uStatus;
}
// Function name   : CUAInformationModel::GetNodeIdObjectTypeFromBrowseName
// Description     : Cette méthode retrouve l'AUObject associé a un browseName
// Return type     : OpcUa_StatusCode S_OK si trouvé, S_FALSE sinon trouvé, E_INVALIDARG en cas de param incorrecte
// Argument        : OpcUa_QualifiedName szBrowseName
// Argument        : CUAObject** pUAObject

OpcUa_StatusCode CUAInformationModel::GetNodeMethodFromBrowseName(OpcUa_QualifiedName szBrowseName, CUAMethod** pUAMethod)
{
	OpcUa_StatusCode uStatus = OpcUa_BadInvalidArgument;
	CUAMethodList* pUAMethodList=GetMethodList();
	int iSize=pUAMethodList->size();
	CUAMethod* pTmpUAMethod=NULL;
	
	for (int iii=0;iii<iSize;iii++)
	{
		pTmpUAMethod=((*pUAMethodList)[iii]);
		char* chTmp1=OpcUa_String_GetRawString(&(szBrowseName.Name));
		char* chTmp2=OpcUa_String_GetRawString(&(pTmpUAMethod->GetBrowseName()->Name));
		size_t iLen1=OpcUa_String_StrLen(&(szBrowseName.Name));
		size_t iLen2=OpcUa_String_StrLen(&(pTmpUAMethod->GetBrowseName()->Name));
		if ( (pTmpUAMethod->GetBrowseName()->NamespaceIndex==szBrowseName.NamespaceIndex) && 			
			(strncmp(chTmp1,chTmp2,iLen1)==0) & (iLen1==iLen2) ) 
			//(OpcUa_String_StrnCmp(&(szBrowseName.Name),&(pTmpUAMethod->GetBrowseName()->Name),OpcUa_String_StrLen(&(szBrowseName.Name)),OpcUa_False)==0) ) 
		{
			*pUAMethod=pTmpUAMethod;
			return OpcUa_Good;
		}
	}
	return uStatus;
}

// Function name   : CUAInformationModel::GetNodeIdObjectTypeFromBrowseName
// Description     : Cette méthode retrouve l'AUObject associé a un browseName
// Return type     : OpcUa_StatusCode S_OK si trouvé, S_FALSE sinon trouvé, E_INVALIDARG en cas de param incorrecte
// Argument        : OpcUa_QualifiedName szBrowseName
// Argument        : CUAObject** pUAObject

OpcUa_StatusCode CUAInformationModel::GetNodeViewFromBrowseName(OpcUa_QualifiedName szBrowseName, CUAView** pUAView)
{
	OpcUa_StatusCode uStatus = OpcUa_BadInvalidArgument;
	CUAViewList* pUAViewList=GetViewList();
	int iSize=pUAViewList->size();
	CUAView* pTmpUAView=NULL;
	
	for (int iii=0;iii<iSize;iii++)
	{
		pTmpUAView=((*pUAViewList)[iii]);
		char* chTmp1=OpcUa_String_GetRawString(&(szBrowseName.Name));
		char* chTmp2=OpcUa_String_GetRawString(&(pTmpUAView->GetBrowseName()->Name));
		size_t iLen1=OpcUa_String_StrLen(&(szBrowseName.Name));
		size_t iLen2=OpcUa_String_StrLen(&(pTmpUAView->GetBrowseName()->Name));
		if ( (pTmpUAView->GetBrowseName()->NamespaceIndex==szBrowseName.NamespaceIndex) && 			
			(strncmp(chTmp1,chTmp2,iLen1)==0) & (iLen1==iLen2) ) 
			//(OpcUa_String_StrnCmp(&(szBrowseName.Name),&(pTmpUAView->GetBrowseName()->Name),OpcUa_String_StrLen(&(szBrowseName.Name)),OpcUa_False)==0) ) 
		{
			*pUAView=pTmpUAView;
			return OpcUa_Good;
		}
	}
	return uStatus;
}
// Function name   : CUAInformationModel::GetNodeIdFromBaseList
// Description     : 
// Return type     : OpcUa_StatusCode 
// Argument        : OpcUa_NodeId iNodeId - NodeId to retrieve
// Argument        : CUABase** pUABase - pUABase found for the requested NodeId
// Argument        : int* iIndex - index of the NodeId in the pUABaseList

OpcUa_StatusCode CUAInformationModel::GetNodeIdFromObjectList(const OpcUa_NodeId iNodeId, CUAObject** pUAObject)
{
	OpcUa_StatusCode uStatus = OpcUa_BadInvalidArgument;
	// Tentative de recherche dans la FastAccess
	CUABase* pUABase=OpcUa_Null;
	if (GetNodeIdFromFastAccessList(iNodeId,&pUABase)==OpcUa_Good)
	{
		if (pUABase->GetNodeClass()==OpcUa_NodeClass_Object)
		{
			(*pUAObject)=(CUAObject*)pUABase;
			return OpcUa_Good;
		}
	}
	CUAObjectList* pUAObjectList=GetObjectList();
	OpcUa_NodeId aNodeId = iNodeId;
	CUAObjectList::iterator it = pUAObjectList->find(&aNodeId);
	if (it != pUAObjectList->end())
	{
		uStatus = OpcUa_Good;
		(*pUAObject) = it->second;
	}

	return uStatus;
}
OpcUa_StatusCode CUAInformationModel::GetNodeIdFromObjectTypeList(const OpcUa_NodeId iNodeId, CUAObjectType** pUAObjectType)
{
	OpcUa_StatusCode uStatus = OpcUa_BadInvalidArgument;
	// Tentative de recherche dans la FastAccess
	CUABase* pUABase=OpcUa_Null;
	if (GetNodeIdFromFastAccessList(iNodeId,&pUABase)==OpcUa_Good)
	{
		if (pUABase->GetNodeClass()==OpcUa_NodeClass_ObjectType)
		{
			(*pUAObjectType)=(CUAObjectType*)pUABase;
			return OpcUa_Good;
		}
	}
	CUAObjectTypeList* pUAObjectTypeList=GetObjectTypeList();
	int iSize=pUAObjectTypeList->size();
	for (int iii=0;iii<iSize;iii++)
	{
		OpcUa_NodeId* pNodeId = ((*pUAObjectTypeList)[iii])->GetNodeId();
		if (pNodeId->IdentifierType == iNodeId.IdentifierType)
		{
			switch (iNodeId.IdentifierType)
			{
			case OpcUa_IdentifierType_Numeric:
				{
					if ((iNodeId.Identifier.Numeric == pNodeId->Identifier.Numeric) && (iNodeId.NamespaceIndex == pNodeId->NamespaceIndex))
					{
						*pUAObjectType=((*pUAObjectTypeList)[iii]);
						return OpcUa_Good;
					}
				}
				break;
			case OpcUa_IdentifierType_String:
				{
					if ((OpcUa_StriCmpA(OpcUa_String_GetRawString(&(iNodeId.Identifier.String)), OpcUa_String_GetRawString(&(pNodeId->Identifier.String))) == 0)
						&& (iNodeId.NamespaceIndex == pNodeId->NamespaceIndex))
					{
						*pUAObjectType=((*pUAObjectTypeList)[iii]);
						return OpcUa_Good;
					}
				}
				break;
			default:
				// other IdentifierType not supported yet
				uStatus = OpcUa_Bad;
				break;
			}
		}
	}
	return uStatus;
}
// Look for the CUADataType* associate with a NodeId
// Return a OpcUa_StatusCode OpcUa_Good if succeeded OpcUa_BadNotFound or OpcUa_BadInvalidArgument if it failed
OpcUa_StatusCode CUAInformationModel::GetNodeIdFromDataTypeList(const OpcUa_NodeId iNodeId, CUADataType** pUADataType)
{
	OpcUa_StatusCode uStatus=OpcUa_BadInvalidArgument ;
	// Tentative de recherche dans la FastAccess
	CUABase* pUABase=OpcUa_Null;
	if (GetNodeIdFromFastAccessList(iNodeId,&pUABase)==OpcUa_Good)
	{
		if (pUABase->GetNodeClass()==OpcUa_NodeClass_DataType)
		{
			(*pUADataType)=(CUADataType*)pUABase;
			uStatus=OpcUa_Good;
		}
	}
	if (uStatus!=OpcUa_Good)
	{
		CUADataTypeList* pUADataTypeList=GetDataTypeList();
		int iSize=pUADataTypeList->size();
		for (int iii=0;iii<iSize;iii++)
		{
			OpcUa_NodeId* pNodeId= ((*pUADataTypeList)[iii])->GetNodeId();
			if (pNodeId->IdentifierType == iNodeId.IdentifierType)
			{
				switch (iNodeId.IdentifierType)
				{
				case OpcUa_IdentifierType_Numeric:
					{
						if ((iNodeId.Identifier.Numeric == pNodeId->Identifier.Numeric) && (iNodeId.NamespaceIndex == pNodeId->NamespaceIndex))
						{
							*pUADataType = ((*pUADataTypeList)[iii]);
							return OpcUa_Good;
						}
					}
					break;
				case OpcUa_IdentifierType_String:
					{
						if ((OpcUa_StriCmpA(OpcUa_String_GetRawString(&(iNodeId.Identifier.String)), OpcUa_String_GetRawString(&(pNodeId->Identifier.String))) == 0)
							&& (iNodeId.NamespaceIndex == pNodeId->NamespaceIndex))
							{
								*pUADataType = ((*pUADataTypeList)[iii]);
								return OpcUa_Good;
							}
						// other IdentifierType not supported yet
						//uStatus=OpcUa_BadNotFound; 
					}
					break;
				default:
					uStatus = OpcUa_BadNotFound; 
					break;
				}
			}
		}
	}
	return uStatus;
}
OpcUa_StatusCode CUAInformationModel::GetNodeIdFromVariableTypeList(const OpcUa_NodeId iNodeId, CUAVariableType** pUAVariableType)
{
	OpcUa_StatusCode uStatus=OpcUa_BadInvalidArgument ;
	// Tentative de recherche dans la FastAccess
	CUABase* pUABase=OpcUa_Null;
	if (GetNodeIdFromFastAccessList(iNodeId,&pUABase)==OpcUa_Good)
	{
		if (pUABase->GetNodeClass()==OpcUa_NodeClass_VariableType)
		{
			(*pUAVariableType)=(CUAVariableType*)pUABase;
			return OpcUa_Good;
		}
	}
	CUAVariableTypeList* pUAVariableTypeList=GetVariableTypeList();
	int iSize=pUAVariableTypeList->size();
	for (int iii=0;iii<iSize;iii++)
	{
		OpcUa_NodeId* pNodeId=((*pUAVariableTypeList)[iii])->GetNodeId();
		if (pNodeId->IdentifierType == iNodeId.IdentifierType)
		{
			switch (iNodeId.IdentifierType)
			{
			case OpcUa_IdentifierType_Numeric:
				{
					if ((iNodeId.Identifier.Numeric == pNodeId->Identifier.Numeric) && (iNodeId.NamespaceIndex == pNodeId->NamespaceIndex))
					{
						*pUAVariableType=((*pUAVariableTypeList)[iii]);
						return OpcUa_Good;
					}
				}
				break;
			case OpcUa_IdentifierType_String:
				{
					if ((OpcUa_StriCmpA(OpcUa_String_GetRawString(&(iNodeId.Identifier.String)), OpcUa_String_GetRawString(&(pNodeId->Identifier.String))) == 0)
						&& (iNodeId.NamespaceIndex == pNodeId->NamespaceIndex))
					{
						*pUAVariableType=((*pUAVariableTypeList)[iii]);
						return OpcUa_Good;
					}
				}
				break;
			default:
				// other IdentifierType not supported yet
				uStatus=OpcUa_BadInvalidArgument; 
				break;
			}
		}
	}
	return uStatus;
}
OpcUa_StatusCode CUAInformationModel::GetNodeIdFromVariableList(const OpcUa_NodeId iNodeId,CUAVariable** pUAVariable)
{
	OpcUa_StatusCode uStatus=OpcUa_BadInvalidArgument ;
	// Tentative de recherche dans la FastAccess
	CUABase* pUABase=OpcUa_Null;
	if (GetNodeIdFromFastAccessList(iNodeId,&pUABase)==OpcUa_Good)
	{
		if (pUABase->GetNodeClass()==OpcUa_NodeClass_Variable)
		{
			(*pUAVariable)=(CUAVariable*)pUABase;
			return OpcUa_Good;
		}
	}
	CUAVariableList* pUAVariableList=GetVariableList();
	if (pUAVariableList)
	{
		OpcUa_NodeId aNodeId = iNodeId;
		CUAVariableList::iterator it= pUAVariableList->find(&aNodeId);
		if (it != pUAVariableList->end())
		{
			uStatus = OpcUa_Good;
			*pUAVariable = it->second;
		}
	}
	
	//int iSize=pUAVariableList->size();
	//for (int iii=0;iii<iSize;iii++)
	//{
	//	CUAVariable* pTmpUAVariable=(*pUAVariableList)[iii];
	//	if (pTmpUAVariable)
	//	{
	//		//OpcUa_NodeId aNodeId=((*pUAVariableList)[iii])->GetNodeId();
	//		OpcUa_NodeId* pNodeId=pTmpUAVariable->GetNodeId();
	//		if (pNodeId->IdentifierType == iNodeId.IdentifierType)
	//		{
	//			switch (iNodeId.IdentifierType)
	//			{
	//			case OpcUa_IdentifierType_Numeric:
	//				{
	//					if ((iNodeId.Identifier.Numeric == pNodeId->Identifier.Numeric) && (iNodeId.NamespaceIndex == pNodeId->NamespaceIndex))
	//					{
	//						*pUAVariable = ((*pUAVariableList)[iii]);
	//						return OpcUa_Good;
	//					}
	//				}
	//				break;
	//			case OpcUa_IdentifierType_String:
	//				{
	//					if (OpcUa_NodeId_Compare(&iNodeId,pNodeId)==0)
	//					{
	//						*pUAVariable = ((*pUAVariableList)[iii]);
	//						return OpcUa_Good;
	//					}
	//				}
	//				break;
	//			default:
	//					// other IdentifierType not supported yet
	//					uStatus = OpcUa_BadNotSupported;
	//					break;
	//			}
	//		}
	//	}
	//}
	return uStatus;
}

OpcUa_StatusCode CUAInformationModel::GetNodeIdFromReferenceTypeList(const OpcUa_NodeId iNodeId, CUAReferenceType** pUAReferenceType)
{
	OpcUa_StatusCode uStatus = OpcUa_BadReferenceTypeIdInvalid;
	// Tentative de recherche dans la FastAccess
	CUABase* pUABase=OpcUa_Null;
	if (GetNodeIdFromFastAccessList(iNodeId,&pUABase)==OpcUa_Good)
	{
		if (pUABase->GetNodeClass()==OpcUa_NodeClass_ReferenceType)
		{
			(*pUAReferenceType)=(CUAReferenceType*)pUABase;
			return OpcUa_Good;
		}
	}
	CUAReferenceTypeList* pUAReferenceTypeList=GetReferenceTypeList();
	int iSize=pUAReferenceTypeList->size();
	for (int iii=0;iii<iSize;iii++)
	{
		OpcUa_NodeId* pNodeId=((*pUAReferenceTypeList)[iii])->GetNodeId();
		if (pNodeId->IdentifierType == iNodeId.IdentifierType)
		{
			switch (iNodeId.IdentifierType)
			{
			case OpcUa_IdentifierType_Numeric:
				{
					if ((iNodeId.Identifier.Numeric == pNodeId->Identifier.Numeric) && (iNodeId.NamespaceIndex == pNodeId->NamespaceIndex))
					{
						*pUAReferenceType=((*pUAReferenceTypeList)[iii]);
						return OpcUa_Good;
					}
				}
				break;
			case OpcUa_IdentifierType_String:
				{
					if ((OpcUa_StriCmpA(OpcUa_String_GetRawString(&(iNodeId.Identifier.String)), OpcUa_String_GetRawString(&(pNodeId->Identifier.String))) == 0)
						&& (iNodeId.NamespaceIndex == pNodeId->NamespaceIndex))
					{
						*pUAReferenceType=((*pUAReferenceTypeList)[iii]);
						return OpcUa_Good;
					}
				}
				break;
			default:
				// other IdentifierType not supported yet
				uStatus=OpcUa_BadReferenceTypeIdInvalid; 
				break;
			}
		}
	}
	return uStatus;
}
OpcUa_StatusCode CUAInformationModel::GetNodeIdFromMethodList(const OpcUa_NodeId iNodeId, CUAMethod** pUAMethod)
{
	OpcUa_StatusCode uStatus=OpcUa_BadInvalidArgument ;
	// Tentative de recherche dans la FastAccess
	CUABase* pUABase=OpcUa_Null;
	if (GetNodeIdFromFastAccessList(iNodeId,&pUABase)==OpcUa_Good)
	{
		if (pUABase->GetNodeClass()==OpcUa_NodeClass_Method)
		{
			(*pUAMethod)=(CUAMethod*)pUABase;
			return OpcUa_Good;
		}
	}
	CUAMethodList* pUAMethodList=GetMethodList();
	int iSize=pUAMethodList->size();
	for (int iii=0;iii<iSize;iii++)
	{
		OpcUa_NodeId* pNodeId=((*pUAMethodList)[iii])->GetNodeId();
		if (pNodeId->IdentifierType == iNodeId.IdentifierType)
		{
			switch (iNodeId.IdentifierType)
			{
			case OpcUa_IdentifierType_Numeric:
				{
					if ((iNodeId.Identifier.Numeric == pNodeId->Identifier.Numeric) && (iNodeId.NamespaceIndex == pNodeId->NamespaceIndex))
					{
						*pUAMethod=((*pUAMethodList)[iii]);
						uStatus= OpcUa_Good;
						break;
					}
				}
				break;
			case OpcUa_IdentifierType_String:
				{
					if ((OpcUa_StriCmpA(OpcUa_String_GetRawString(&(iNodeId.Identifier.String)), OpcUa_String_GetRawString(&(pNodeId->Identifier.String))) == 0)
						&& (iNodeId.NamespaceIndex == pNodeId->NamespaceIndex))
					{
						*pUAMethod=((*pUAMethodList)[iii]);
						uStatus = OpcUa_Good;
						break;
					}
				}
				break;
			default:
				// other IdentifierType not supported yet
				uStatus=OpcUa_BadInvalidArgument;
				break;
			}
		}
	}
	return uStatus;
}
OpcUa_StatusCode CUAInformationModel::GetNodeIdFromViewList(const OpcUa_NodeId iNodeId, CUAView** pUAView)
{
	OpcUa_StatusCode uStatus=OpcUa_BadInvalidArgument ;
	// Tentative de recherche dans la FastAccess
	CUABase* pUABase=OpcUa_Null;
	if (GetNodeIdFromFastAccessList(iNodeId,&pUABase)==OpcUa_Good)
	{
		if (pUABase->GetNodeClass()==OpcUa_NodeClass_View)
		{
			(*pUAView)=(CUAView*)pUABase;
			return OpcUa_Good;
		}
	}
	CUAViewList* pUAViewList=GetViewList();
	int iSize=pUAViewList->size();
	for (int iii=0;iii<iSize;iii++)
	{
		OpcUa_NodeId* pNodeId=((*pUAViewList)[iii])->GetNodeId();
		if (pNodeId->IdentifierType == iNodeId.IdentifierType)
		{
			switch (iNodeId.IdentifierType)
			{
			case OpcUa_IdentifierType_Numeric:
				{
					if ((iNodeId.Identifier.Numeric == pNodeId->Identifier.Numeric) && (iNodeId.NamespaceIndex == pNodeId->NamespaceIndex))
					{
						*pUAView=((*pUAViewList)[iii]);
						return OpcUa_Good;
					}
				}
				break;
			case OpcUa_IdentifierType_String:
				{
					if ((OpcUa_StriCmpA(OpcUa_String_GetRawString(&(iNodeId.Identifier.String)), OpcUa_String_GetRawString(&(pNodeId->Identifier.String))) == 0)
						&& (iNodeId.NamespaceIndex == pNodeId->NamespaceIndex))
					{
						*pUAView=((*pUAViewList)[iii]);
						return OpcUa_Good;
					}
				}
				break;
			default:
				// other IdentifierType not supported yet
				uStatus=OpcUa_BadInvalidArgument;
				break;
			}
		}
	}
	return uStatus;
}

OpcUa_StatusCode CUAInformationModel::GetAliasFromNodeId(const OpcUa_NodeId iNodeId, CAlias** pAlias)
{
	OpcUa_StatusCode uStatus=OpcUa_BadInvalidArgument ;
	CAliasList* pAliasList=GetAliasList();
	int iSize=pAliasList->size();
	for (int iii=0;iii<iSize;iii++)
	{
		OpcUa_NodeId aNodeId=((*pAliasList)[iii])->GetNodeId();
		if (aNodeId.IdentifierType==iNodeId.IdentifierType)
		{
			if (iNodeId.IdentifierType==OpcUa_IdentifierType_Numeric)
			{
				if ( (iNodeId.Identifier.Numeric==aNodeId.Identifier.Numeric)  && (iNodeId.NamespaceIndex==aNodeId.NamespaceIndex)) 
				{
					*pAlias=((*pAliasList)[iii]);
					return OpcUa_Good;
				}
			}
			else
			{
				if (iNodeId.IdentifierType==OpcUa_IdentifierType_String)
				{
					if (OpcUa_StriCmpA(OpcUa_String_GetRawString(&(iNodeId.Identifier.String)),OpcUa_String_GetRawString(&(aNodeId.Identifier.String)))==0)
					{
						*pAlias=((*pAliasList)[iii]);
						return OpcUa_Good;
					}
				}
				// other IdentifierType not supported yet
				uStatus=OpcUa_BadInvalidArgument;
			}
		}
	}
	return uStatus;
}
OpcUa_StatusCode CUAInformationModel::GetAliasFromAliasName(const OpcUa_String szNodeId, CAlias** pAlias)
{
	OpcUa_StatusCode uStatus=OpcUa_BadInvalidArgument ;
	if (OpcUa_String_StrLen(&szNodeId)>0)
	{
		CAliasList* pAliasList=GetAliasList();
		if (pAliasList)
		{
			int iSize=pAliasList->size();
			for (int iii=0;iii<iSize;iii++)
			{
				CAlias* pAliasTmp=(*pAliasList)[iii];
				if (pAliasTmp)
				{
					OpcUa_String szNodeName=pAliasTmp->GetAliasName();
					if (OpcUa_StriCmpA(OpcUa_String_GetRawString(&szNodeId),OpcUa_String_GetRawString(&szNodeName))==0)
					{
						*pAlias=((*pAliasList)[iii]);
						return OpcUa_Good;
					}
				}
			}
		}
	}
	return uStatus;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Is in reference hierarchy. </summary>
///
/// <remarks>	Michel, 01/09/2016. </remarks>
///
/// <param name="idReferenceSource">	The identifier reference source. </param>
/// <param name="idReferenceTarget">	The identifier reference target. </param>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode CUAInformationModel::IsInReferenceHierarchy(OpcUa_NodeId idReferenceSource, OpcUa_NodeId idReferenceTarget)
{
	OpcUa_StatusCode uStatus=OpcUa_BadInvalidArgument;
	vector<OpcUa_NodeId> referenceHierachy; // Liste des references appartenant a cette hirarchie (idReference et ses enfants)
	// La reference de base est toujours dans la hierarchie
	referenceHierachy.push_back(idReferenceSource);
	// Si la idReferenceSource = 0 On renvoi toujours ok. cad que l'on considère idRefrenceTarget dans la hierarchie
	if (idReferenceSource.Identifier.Numeric==0)
		uStatus= OpcUa_Good;
	else
	{
		// remplissage de la hierarchy des references 
		uStatus = FillReferenceNodeIdHierachy(idReferenceSource, &referenceHierachy);
		if (uStatus == OpcUa_Good)
		{
			uStatus = OpcUa_BadInvalidArgument;
			for (OpcUa_UInt32 ii = 0; ii < referenceHierachy.size(); ii++)
			{
				OpcUa_NodeId aNodeId = referenceHierachy.at(ii);
				if (Utils::IsEqual(&aNodeId, &idReferenceTarget))
				{
					uStatus = OpcUa_Good;
					break;
				}
			}
		}
	}
	return uStatus;
}
// Methode hierarchique de peuplement d'une branche.
// Cette méthode ne traite que les references
// On passe a cette methode un nodeId et elle rempli la branche de reference à laquelle elle appartient
//
OpcUa_StatusCode CUAInformationModel::FillReferenceNodeIdHierachy(OpcUa_NodeId idReference, vector<OpcUa_NodeId>* pHierachy)
{
	OpcUa_StatusCode uStatus=OpcUa_BadInvalidArgument;
	CUAReferenceType* pRelativeRoot=NULL;
	if (pHierachy)
	{
		uStatus = GetNodeIdFromReferenceTypeList(idReference, &pRelativeRoot);
		if (uStatus == OpcUa_Good)
		{
			CUAReferenceList::iterator myIterator;
			for (myIterator = pRelativeRoot->GetReferenceNodeList()->begin(); myIterator != pRelativeRoot->GetReferenceNodeList()->end(); myIterator++)
			{
				CUAReference* pReferenceNode = *myIterator;
				OpcUa_ExpandedNodeId aExpandedNodeId = pReferenceNode->GetTargetId();
				OpcUa_Boolean bFound = false;
				for (OpcUa_UInt32 ii = 0; ii < pHierachy->size(); ii++)
				{
					OpcUa_NodeId pNodeId = pHierachy->at(ii);
					if (Utils::IsEqual(&pNodeId, &(aExpandedNodeId.NodeId)))
					{
						bFound = true;
						break;
					}
				}
				if ((!bFound) && (!(pReferenceNode->IsInverse())))
				{
					pHierachy->push_back(pReferenceNode->GetTargetId().NodeId);
				}
				if (!(pReferenceNode->IsInverse()))
				{
					// workaround for the Odd case where idReference== TargetId
					OpcUa_NodeId aTargetId = pReferenceNode->GetTargetId().NodeId;
					if (OpcUa_NodeId_Compare(&aTargetId, &idReference) == 0)
					{
						char* szNodeId = OpcUa_Null;
						Utils::NodeId2String(&aTargetId, &szNodeId);
						if (szNodeId)
						{
							// Attribute
							OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, 
								"FillReferenceNodeIdHierachy>An invalid reference definition was detected. Source and target[%s] node are equal\n",szNodeId);
							free(szNodeId);
						}
					}
					else
						FillReferenceNodeIdHierachy(aTargetId, pHierachy);
				}
			}
		}
	}
	return uStatus;
}
// Methode hierarchique de peuplement d'une branche.
// Cette méthode ne traite que les Events
// On passe a cette methode un nodeId et elle rempli la branche d'event à laquelle elle appartient
//
OpcUa_StatusCode CUAInformationModel::FillEventTypeNodeIdHierachy(OpcUa_NodeId idEvent, vector<OpcUa_NodeId>* pHierachy)
{
	OpcUa_StatusCode uStatus = OpcUa_BadInvalidArgument;
	CUAObjectType* pRelativeRoot = NULL;
	if (pHierachy)
	{
		uStatus = GetNodeIdFromObjectTypeList(idEvent, &pRelativeRoot);
		if (uStatus == OpcUa_Good)
		{
			CUAReferenceList::iterator myIterator;
			for (myIterator = pRelativeRoot->GetReferenceNodeList()->begin(); myIterator != pRelativeRoot->GetReferenceNodeList()->end(); myIterator++)
			{
				CUAReference* pReferenceNode = *myIterator;
				OpcUa_ExpandedNodeId aExpandedNodeId = pReferenceNode->GetTargetId();
				OpcUa_Boolean bFound = false;
				// Check if the target of the reference is already in the list
				for (OpcUa_UInt32 ii = 0; ii < pHierachy->size(); ii++)
				{
					OpcUa_NodeId pNodeId = pHierachy->at(ii);
					if (Utils::IsEqual(&pNodeId, &(aExpandedNodeId.NodeId)))
					{
						bFound = true;
						break;
					}
				}
				// Find the NodeClass
				// if not found and forward.
				if ((!bFound) && (!(pReferenceNode->IsInverse())))
				{
					CUAObjectType* pUABase;
					uStatus = GetNodeIdFromObjectTypeList(aExpandedNodeId.NodeId, &pUABase);
					if (uStatus == OpcUa_Good)
					{
						//OpcUa_ExpandedNodeId aNodeId = pReferenceNode->GetTargetId();
						pHierachy->push_back(pReferenceNode->GetTargetId().NodeId);
					}
				}
				// if forwsard we continue looking for
				if (!(pReferenceNode->IsInverse()))
					FillEventTypeNodeIdHierachy(pReferenceNode->GetTargetId().NodeId, pHierachy);
			}
		}
	}
	return uStatus;
}
OpcUa_StatusCode CUAInformationModel::FindNodes(
												CUABase* pStartingNode,
												OpcUa_String szTargetNode, 
												OpcUa_NodeId aReferenceTypeId, 
												OpcUa_Boolean bInverse, 
												OpcUa_Boolean bIncludeSubTypes,
												vector<CUABase*>** pNodes)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;

	// Recherche de la cible
	CUABase* pUABase=pStartingNode; 
	{
		// parcours de reference d'un objet
		CUAReferenceList::iterator myIterator;
		for (myIterator=pUABase->GetReferenceNodeList()->begin();myIterator!=pUABase->GetReferenceNodeList()->end();myIterator++)
		{
			CUAReference* pReferenceNode=*myIterator;
			// si la reference cible est égale au type de referenc demandé
			if (IsInReferenceHierarchy(aReferenceTypeId,pReferenceNode->GetReferenceTypeId())==OpcUa_Good)
			{
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"yes, Is In\n");
				// si cette refenrence va dans le sens indiquée
				if (pReferenceNode->IsInverse()==bInverse)
				{
					// recherche de la reference cible
					CUABase* pUABaseTarget=NULL;
					uStatus = GetNodeIdFromDictionnary(pReferenceNode->GetTargetId().NodeId, &pUABaseTarget);
					if (uStatus == OpcUa_Good)
					{
						// vérification que la cible correspond au la target demandée
						if (OpcUa_String_StrnCmp(&(pUABaseTarget->GetBrowseName()->Name),&szTargetNode,OpcUa_String_StrLen(&szTargetNode),false)==0)
						{

							(*pNodes)->push_back(pUABaseTarget);
						}
						else
						{
							// appel recursif
							if (bIncludeSubTypes)
							{
								uStatus=FindNodes(pUABaseTarget,
									szTargetNode,
									aReferenceTypeId,
									bInverse,
									bIncludeSubTypes,
									pNodes );
							}
						}
					}
					else
						uStatus=OpcUa_BadNodeIdInvalid;
				}
			}
		}
	}
	return uStatus;
}
// Initialisation de RedundantServerArray i=2038. 
// Il s'agit d'un tableau à 1 dimension qui contient de RedundantServerDataType
// Les UAVariable 11310 et 2038 sont concernée
OpcUa_StatusCode CUAInformationModel::InitRedundantServerArray()
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_NodeId aNodeId;
	OpcUa_NodeId_Initialize(&aNodeId);
	aNodeId.Identifier.Numeric = OpcUaId_Server_ServerRedundancy_RedundantServerArray; // RedundantServerArray
	aNodeId.IdentifierType=OpcUa_IdentifierType_Numeric;
	aNodeId.NamespaceIndex=0; // opc foundation namespace
	CUAVariable* pUAVariable=NULL;
	{
		OpcUa_Mutex_Lock(GetServerCacheMutex());
		uStatus = GetNodeIdFromVariableList(aNodeId, &pUAVariable);
		if (uStatus == OpcUa_Good)
		{
			CDataValue* pDataValue=pUAVariable->GetValue();
			if (pDataValue)
			{
				//// Recherche de l'encodeableObject associé
				OpcUa_EncodeableType* pEncodeableType=OpcUa_Null;
				uStatus=g_pTheApplication->LookupEncodeableType(pUAVariable->GetDataType().Identifier.Numeric,&pEncodeableType);
				if (uStatus==OpcUa_Good)
				{
					/*OpcUa_Variant aVariant=pDataValue->GetValue();
					if (aVariant.Value.Array.Value.ExtensionObjectArray==OpcUa_Null)
					{
						pDataValue->InitializeArray(pUAVariable->GetBuiltInType(),1);
						aVariant=pDataValue->GetValue();
					}*/
					OpcUa_DataValue* pInternalDataValue = pDataValue->GetInternalDataValue();
					
					OpcUa_RedundantServerDataType* pRedundantServerData=(OpcUa_RedundantServerDataType*)OpcUa_Alloc(sizeof(OpcUa_RedundantServerDataType));
					if (pRedundantServerData)
					{
						OpcUa_RedundantServerDataType_Initialize(pRedundantServerData);
						OpcUa_String_AttachCopy(&(pRedundantServerData->ServerId),"OpenOpcUa_RedundantServer");
						CServerStatus* pServerStatus=GetInternalServerStatus();
						pRedundantServerData->ServerState=pServerStatus->GetServerState();
						pRedundantServerData->ServiceLevel=255;
						if (!pInternalDataValue->Value.Value.Array.Value.ExtensionObjectArray)
							pInternalDataValue->Value.Value.Array.Value.ExtensionObjectArray = (OpcUa_ExtensionObject*)OpcUa_Alloc(sizeof(OpcUa_ExtensionObject));
						OpcUa_ExtensionObject_Initialize(pInternalDataValue->Value.Value.Array.Value.ExtensionObjectArray);
						pInternalDataValue->Value.Datatype = OpcUaType_ExtensionObject;
						pInternalDataValue->Value.ArrayType = OpcUa_VariantArrayType_Array;
						// Only one elt in this Array
						pInternalDataValue->Value.Value.Array.Length = 1;
						pInternalDataValue->Value.Value.Array.Value.ExtensionObjectArray[0].TypeId.ServerIndex = 0;
						OpcUa_String_Initialize(&(pInternalDataValue->Value.Value.Array.Value.ExtensionObjectArray[0].TypeId.NamespaceUri));
						pInternalDataValue->Value.Value.Array.Value.ExtensionObjectArray[0].TypeId.NodeId.Identifier.Numeric = pEncodeableType->BinaryEncodingTypeId;; // OpcUaId_RedundantServerDataType;  
						pInternalDataValue->Value.Value.Array.Value.ExtensionObjectArray[0].TypeId.NodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
						pInternalDataValue->Value.Value.Array.Value.ExtensionObjectArray[0].TypeId.NodeId.NamespaceIndex = 0;
						pInternalDataValue->Value.Value.Array.Value.ExtensionObjectArray[0].BodySize = 0;
						pInternalDataValue->Value.Value.Array.Value.ExtensionObjectArray[0].Encoding = OpcUa_ExtensionObjectEncoding_EncodeableObject;
						pInternalDataValue->Value.Value.Array.Value.ExtensionObjectArray[0].Body.EncodeableObject.Type = pEncodeableType; // Utils::Copy(pEncodeableType);
						//pInternalDataValue->Value.Value.Array.Value.ExtensionObjectArray[0].Body.EncodeableObject.Object = OpcUa_Alloc(pEncodeableType->AllocationSize);
						pInternalDataValue->Value.Value.Array.Value.ExtensionObjectArray[0].Body.EncodeableObject.Object = pRedundantServerData;
		
					}
					else
					{
						pInternalDataValue->Value.Value.Array.Length = 0;
						pInternalDataValue->Value.Value.Array.Value.ExtensionObjectArray[0].Body.EncodeableObject.Object = OpcUa_Null;
						//if (pEncodeableType)
						//{
						//	OpcUa_Free(pEncodeableType);
						//	pEncodeableType = OpcUa_Null;
						//}
					}
				}
				else				
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Critical error RedundantServerArray(i=2038) OpcUa_EncodeableType cannot be found.\n");

			}
		}
		else
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Configuration error. Some nodes are probably missing check for ns=0;i=11313\n");
		OpcUa_Mutex_Unlock(GetServerCacheMutex());
	}
	aNodeId.Identifier.Numeric = OpcUaId_TransparentRedundancyType_RedundantServerArray; // RedundantServerArray
	{
		OpcUa_Mutex_Lock(GetServerCacheMutex());
		uStatus = GetNodeIdFromVariableList(aNodeId, &pUAVariable);
		if (uStatus == OpcUa_Good)
		{
			CDataValue* pDataValue=pUAVariable->GetValue();
			if (pDataValue)
			{
				InitializeEncodeableObject(pUAVariable);
			}
		}
		else
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Critical error RedundantServerArray(i=2038) cannot be found in your AddressSpace.\n");
		OpcUa_Mutex_Unlock(GetServerCacheMutex());
	}
	return uStatus;
}
// Initialisation de SamplingIntervalDiagnosticsArray 2022
// il s'agit de mettre d'allouer un tableau à 1 dimension
OpcUa_StatusCode CUAInformationModel::InitSamplingIntervalDiagnosticsArray()
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_NodeId aNodeId;
	OpcUa_NodeId_Initialize(&aNodeId);
	aNodeId.Identifier.Numeric=2289; // SamplingIntervalDiagnosticsArray
	aNodeId.IdentifierType=OpcUa_IdentifierType_Numeric;
	aNodeId.NamespaceIndex=0; // opc foundation namespace
	CUAVariable* pUAVariable=NULL;

	OpcUa_Mutex_Lock(m_ServerCacheMutex);
	uStatus = GetNodeIdFromVariableList(aNodeId, &pUAVariable);
	if (uStatus == OpcUa_Good)
	{
		CDataValue* pDataValue=pUAVariable->GetValue();
		if (pDataValue)
		{
			if (InitializeEncodeableObject(pUAVariable) != OpcUa_Good)
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "InitializeEncodeableObject failed for i=2022 (SamplingIntervalDiagnosticsArray)");
		}
	}
	else
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Critical error SamplingIntervalDiagnosticsArray(i=2022) cannot be found in your AddressSpace.\n");
	OpcUa_Mutex_Unlock(m_ServerCacheMutex);

	return uStatus;
}
//typedef struct _OpcUa_SamplingIntervalDiagnosticsDataType
//{
//	OpcUa_Double SamplingInterval;
//	OpcUa_UInt32 MonitoredItemCount;
//	OpcUa_UInt32 MaxMonitoredItemCount;
//	OpcUa_UInt32 DisabledMonitoredItemCount;
//}
//OpcUa_SamplingIntervalDiagnosticsDataType;
OpcUa_StatusCode CUAInformationModel::AddInSamplingIntervalDiagnosticsArray(OpcUa_SamplingIntervalDiagnosticsDataType* pSamplingIntervalDiagnostic)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	if (pSamplingIntervalDiagnostic)
	{
		OpcUa_NodeId aNodeId;
		OpcUa_NodeId_Initialize(&aNodeId);
		aNodeId.Identifier.Numeric = 2289;
		OpcUa_Mutex_Lock(m_SamplingIntervalDiagnosticsArrayMutex);
		m_SamplingIntervalDiagnosticsArray.push_back(pSamplingIntervalDiagnostic);
		UpdateSamplingIntervalDiagnosticsArray(aNodeId);
		OpcUa_Mutex_Unlock(m_SamplingIntervalDiagnosticsArrayMutex);
		OpcUa_NodeId_Clear(&aNodeId);
		// Now add the new sampling interval as a UAVariable in the addressSpace
		uStatus = AddSamplingIntervalnTheAddressSpace(pSamplingIntervalDiagnostic);
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}
OpcUa_StatusCode CUAInformationModel::RemoveInSamplingIntervalDiagnosticsArray(OpcUa_SamplingIntervalDiagnosticsDataType* pSamplingIntervalDiagnostic)
{
	OpcUa_StatusCode uStatus=OpcUa_BadNotFound;
	if (pSamplingIntervalDiagnostic)
	{
		OpcUa_NodeId aNodeId;
		OpcUa_NodeId_Initialize(&aNodeId);
		aNodeId.Identifier.Numeric = 2289;
		OpcUa_Mutex_Lock(m_SamplingIntervalDiagnosticsArrayMutex);
		std::vector<OpcUa_SamplingIntervalDiagnosticsDataType*>::iterator it;
		for (it = m_SamplingIntervalDiagnosticsArray.begin(); it != m_SamplingIntervalDiagnosticsArray.end(); it++)
		{
			OpcUa_SamplingIntervalDiagnosticsDataType* pSamplingIntervalDiagnosticsDataType = *it;
			if (pSamplingIntervalDiagnosticsDataType == pSamplingIntervalDiagnostic)
			{
				m_SamplingIntervalDiagnosticsArray.erase(it);
				uStatus = OpcUa_Good;
				break;
			}
		}
		if (uStatus==OpcUa_Good)
			UpdateSamplingIntervalDiagnosticsArray(aNodeId);
		OpcUa_Mutex_Unlock(m_SamplingIntervalDiagnosticsArrayMutex);
		OpcUa_NodeId_Clear(&aNodeId);
		// Now let remove the related Node (UAVariable from the addressSpace
		uStatus = RemoveSamplingIntervalnTheAddressSpace(pSamplingIntervalDiagnostic);
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}
///-------------------------------------------------------------------------------------------------
/// <summary>	Updates the sampling interval diagnostics array described by aNodeId. </summary>
///
/// <remarks>	Michel, 24/05/2016. </remarks>
///
/// <param name="aNodeId">	Identifier for the node. </param>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode OpenOpcUa::UAAddressSpace::CUAInformationModel::UpdateSamplingIntervalDiagnosticsArray(OpcUa_NodeId aNodeId)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	CUAVariable* pUAVariable = OpcUa_Null;
	// recupération de l'UAVariable associé au paramètre aNodeId dans la BD du serveur
	uStatus = GetNodeIdFromVariableList(aNodeId, &pUAVariable);
	if (uStatus == OpcUa_Good)
	{
		// recuperation de la serie actuellement stockée
		CDataValue* pDataValue = pUAVariable->GetValue();

		OpcUa_Int32 iSessionDiagnosticsSize = m_SamplingIntervalDiagnosticsArray.size();
		OpcUa_EncodeableType* pEncodeableType = OpcUa_Null;
		uStatus = g_pTheApplication->LookupEncodeableType(pUAVariable->GetDataType().Identifier.Numeric, &pEncodeableType);
		if (uStatus == OpcUa_Good)
		{
			if (pEncodeableType)
			{
				// First we will empty the content of the OpcUa_Variant
				OpcUa_DataValue* pInternalDataValue = pDataValue->GetInternalDataValue();
				if (pInternalDataValue->Value.Value.Array.Length > 0)
				{
					// The OpcUa_Variant was not empty
					for (OpcUa_Int32 ii = 0; ii < pInternalDataValue->Value.Value.Array.Length; ii++)
					{
						OpcUa_ExtensionObject_Clear(&(pInternalDataValue->Value.Value.Array.Value.ExtensionObjectArray[ii]));
					}
					if (pInternalDataValue->Value.Value.Array.Value.ExtensionObjectArray)
						OpcUa_Free(pInternalDataValue->Value.Value.Array.Value.ExtensionObjectArray);
					pInternalDataValue->Value.Value.Array.Value.ExtensionObjectArray = OpcUa_Null;
					pInternalDataValue->Value.Value.Array.Length = 0;
				}
				// Now let's handling the new content
				if (iSessionDiagnosticsSize > 0)
				{
					OpcUa_Variant varNewVal;
					OpcUa_Variant_Initialize(&varNewVal);
					varNewVal.Datatype = OpcUaType_ExtensionObject;
					varNewVal.ArrayType = OpcUa_VariantArrayType_Array;
					if (iSessionDiagnosticsSize > 0)
						varNewVal.Value.Array.Value.ExtensionObjectArray = (OpcUa_ExtensionObject*)OpcUa_Alloc(iSessionDiagnosticsSize*sizeof(OpcUa_ExtensionObject));
					varNewVal.Value.Array.Length = iSessionDiagnosticsSize;
					for (OpcUa_UInt16 iii = 0; iii < iSessionDiagnosticsSize; iii++)
					{
						OpcUa_SamplingIntervalDiagnosticsDataType* pSamplingIntervalDiagnosticsDataType = m_SamplingIntervalDiagnosticsArray.at(iii);
						if (pSamplingIntervalDiagnosticsDataType)
						{
							OpcUa_ExtensionObject_Initialize(&(varNewVal.Value.Array.Value.ExtensionObjectArray[iii]));
							varNewVal.Value.Array.Value.ExtensionObjectArray[iii].BodySize = 0;
							varNewVal.Value.Array.Value.ExtensionObjectArray[iii].Encoding = OpcUa_ExtensionObjectEncoding_EncodeableObject;
							OpcUa_ExpandedNodeId_Initialize(&(varNewVal.Value.Array.Value.ExtensionObjectArray[iii].TypeId));
							varNewVal.Value.Array.Value.ExtensionObjectArray[iii].TypeId.NodeId.Identifier.Numeric = pEncodeableType->BinaryEncodingTypeId; //  OpcUaId_SessionDiagnosticsArrayType;
							varNewVal.Value.Array.Value.ExtensionObjectArray[iii].TypeId.NodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
							varNewVal.Value.Array.Value.ExtensionObjectArray[iii].TypeId.NodeId.NamespaceIndex = 0;;
							varNewVal.Value.Array.Value.ExtensionObjectArray[iii].TypeId.ServerIndex = 0;
							varNewVal.Value.Array.Value.ExtensionObjectArray[iii].Body.EncodeableObject.Type = pEncodeableType; 
							varNewVal.Value.Array.Value.ExtensionObjectArray[iii].Body.EncodeableObject.Object = Utils::Copy(pSamplingIntervalDiagnosticsDataType);
						}
					}
					pDataValue->SetValue(varNewVal);
				}
				pDataValue->SetStatusCode(OpcUa_Good);
			}
		}
		else
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "CUAInformationModel::UpdateSamplingIntervalDiagnosticsArray>LookupEncodeableType failed uStatus=0x%05x\n", uStatus);
	}
	else
		uStatus = OpcUa_Bad;
	return uStatus;
}
///-------------------------------------------------------------------------------------------------
/// <summary>	Is sampling interval diagnostics exist. </summary>
///
/// <remarks>	Michel, 24/05/2016. </remarks>
///
/// <param name="dblSamplingInterval">	The sampling interval. </param>
///
/// <returns>	An OpcUa_Boolean. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_Boolean OpenOpcUa::UAAddressSpace::CUAInformationModel::IsSamplingIntervalDiagnosticsExist(OpcUa_Double dblSamplingInterval, OpcUa_SamplingIntervalDiagnosticsDataType** ppSamplingIntervalDiagnostic)
{
	OpcUa_Boolean bResult=OpcUa_False;
	OpcUa_Mutex_Lock(m_SamplingIntervalDiagnosticsArrayMutex);
	std::vector<OpcUa_SamplingIntervalDiagnosticsDataType*>::iterator it;
	for (it = m_SamplingIntervalDiagnosticsArray.begin(); it != m_SamplingIntervalDiagnosticsArray.end(); it++)
	{
		OpcUa_SamplingIntervalDiagnosticsDataType* pSamplingIntervalDiagnosticsDataType = *it;
		if (pSamplingIntervalDiagnosticsDataType->SamplingInterval == dblSamplingInterval)
		{
			bResult = OpcUa_True;
			(*ppSamplingIntervalDiagnostic) = pSamplingIntervalDiagnosticsDataType;
			break;
		}
	}
	OpcUa_Mutex_Unlock(m_SamplingIntervalDiagnosticsArrayMutex);
	return bResult;
}
void CUAInformationModel::AddSimulatedGroup(CSimulatedGroup* pSimulatedGroup)
{
	OpcUa_Mutex_Lock(m_SimulatedGroupMutex);
	m_pSimulatedGroupList->push_back(pSimulatedGroup);
	OpcUa_Mutex_Unlock(m_SimulatedGroupMutex);
}
/// <summary>
/// Initializes the AllowNulls node (i=3070) with bVal
/// </summary>
/// <param name="bVal">The boolean value to put in the i=3070.</param>
/// <returns></returns>
OpcUa_StatusCode CUAInformationModel::InitAllowNulls(OpcUa_Boolean bVal)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;

	OpcUa_NodeId aNodeId;
	OpcUa_NodeId_Initialize(&aNodeId);
	aNodeId.Identifier.Numeric=3070; // ServerArray
	aNodeId.IdentifierType=OpcUa_IdentifierType_Numeric;
	aNodeId.NamespaceIndex=0; // opc foundation namespace
	CUAVariable* pUAVariable=NULL;
	{
		OpcUa_Mutex_Lock(GetServerCacheMutex());
		uStatus = GetNodeIdFromVariableList(aNodeId, &pUAVariable);
		if (uStatus == OpcUa_Good)
		{
			OpcUa_Variant varVal;
			OpcUa_Variant_Initialize(&varVal);
			// céation d'un CDataValue pour mettre en forme la donnée
			CDataValue* pValue=pUAVariable->GetValue();
			if (pValue)
			{
				// Mise en place de la qualité
				pValue->SetStatusCode(OpcUa_Good);
				// initialisation de l'heure
				pValue->SetServerTimestamp(OpcUa_DateTime_UtcNow());
				pValue->SetSourceTimestamp(OpcUa_DateTime_UtcNow());
				// Création du Opcua_Variant encapsulé
				varVal = pValue->GetValue();
				// Ensure the DataType
				varVal.Datatype = OpcUaType_Boolean;
				// Set the value
				varVal.Value.Boolean = bVal;
				// transfert dans le CDataValue
				pValue->SetValue(varVal);
				// transfert dans la Node (CUAVariable)
				pUAVariable->SetValue(pValue);
			}
			else
				uStatus = OpcUa_BadInvalidArgument;
		}
		OpcUa_Mutex_Unlock(GetServerCacheMutex());
	}
	return uStatus;
}
// initialisation et remplissage du nodeID 2254 (ServerArray)
OpcUa_StatusCode CUAInformationModel::InitServerArray(OpcUa_CharA* szServerArray)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_NodeId aNodeId;
	OpcUa_NodeId_Initialize(&aNodeId);
	aNodeId.Identifier.Numeric=2254; // ServerArray
	aNodeId.IdentifierType=OpcUa_IdentifierType_Numeric;
	aNodeId.NamespaceIndex=0; // opc foundation namespace
	CUAVariable* pUAVariable=NULL;
	if (szServerArray)
	{
		OpcUa_Mutex_Lock(GetServerCacheMutex());
		uStatus = GetNodeIdFromVariableList(aNodeId, &pUAVariable);
		if (uStatus == OpcUa_Good)
		{
			OpcUa_Variant varVal;
			OpcUa_Variant_Initialize(&varVal);
			int iSize = -1;
			// céation d'un CDataValue pour mettre en forme la donnée
			CDataValue* pValue = pUAVariable->GetValue();
			OpcUa_DataValue* pInternalDataValue = pValue->GetInternalDataValue();
			// Mise en place de la qualité
			pInternalDataValue->StatusCode = OpcUa_Good;
			// initialisation de l'heure
			pInternalDataValue->ServerTimestamp = OpcUa_DateTime_UtcNow();
			pInternalDataValue->SourceTimestamp = OpcUa_DateTime_UtcNow();
			// Création du Opcua_Variant encapsulé
			//varVal=pValue->GetValue();
			// Initialisation en tant que Tableau (vecteur)			
			pInternalDataValue->Value.ArrayType = OpcUa_VariantArrayType_Array;
			pInternalDataValue->Value.Datatype = OpcUaType_String;
			pInternalDataValue->Value.Value.Array.Length = 1;
			// préparation des elements du tableau (1 element)

			OpcUa_String myString;
			OpcUa_String_Initialize(&myString);
			OpcUa_String_AttachCopy(&myString, (OpcUa_StringA)szServerArray);
			iSize = strlen(szServerArray);
			//
			// allocation d'un tableau de 1 chaine
			pInternalDataValue->Value.Value.Array.Value.StringArray = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
			OpcUa_String_Initialize(pInternalDataValue->Value.Value.Array.Value.StringArray);
			// transfert du contenu
			OpcUa_String_AttachCopy(&pInternalDataValue->Value.Value.Array.Value.StringArray[0], (OpcUa_StringA)szServerArray);

			// transfert dans le CDataValue
			//pValue->SetValue(varVal);
			OpcUa_Variant_Clear(&varVal);
			OpcUa_String_Clear(&myString);
		}
		OpcUa_Mutex_Unlock(GetServerCacheMutex());
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}
///-------------------------------------------------------------------------------------------------
/// <summary>	Initialises the internal serverStatus CUAInformationModel::m_InternalServerStatus. </summary>
///
/// <remarks>	Michel, 29/07/2016. </remarks>
///
/// <param name="aNodeId">	  	Identifier for the node. </param>
/// <param name="dtStartTime">	The dt start time. </param>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode CUAInformationModel::InitServerStatus(OpcUa_NodeId aNodeId, OpcUa_DateTime dtStartTime )
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	CServerStatus* pServerStatus=GetInternalServerStatus();
	if (pServerStatus)
	{
		OpcUa_Variant varVal;
		OpcUa_Variant_Initialize(&varVal);
		
		// Initialisation du BuildInfo encapsulé
		OpcUa_BuildInfo* pBuildInfo = pServerStatus->GetInternalBuildInfo();
		if (pBuildInfo)
		{
//			// Warnign this is a Windows only piece of code
#ifdef WIN32
			CFileVersionInfo* pFileVersion=GetFileVersionInfo();
			if (pFileVersion)
			{
				pBuildInfo->BuildDate = pFileVersion->GetFileDate();
				OpcUa_String aString=pFileVersion->GetFileVersion();
				OpcUa_String_StrnCpy( &(pBuildInfo->BuildNumber), &aString, OpcUa_String_StrLen(&aString));
				OpcUa_String companyName=pFileVersion->GetCompanyName();
				OpcUa_String_StrnCpy( &(pBuildInfo->ManufacturerName), &companyName,OpcUa_String_StrLen(&companyName));
				OpcUa_String productName=pFileVersion->GetProductName();
				OpcUa_String_StrnCpy( &(pBuildInfo->ProductName),&productName, OpcUa_String_StrLen(&productName));
				OpcUa_String_StrnCpy( &(pBuildInfo->ProductUri), &productName,OpcUa_String_StrLen(&productName));
				OpcUa_String fileVersion=pFileVersion->GetFileVersion();
				OpcUa_String_StrnCpy(&(pBuildInfo->SoftwareVersion) ,&fileVersion, OpcUa_String_StrLen(&fileVersion));
			}
#endif
			// SetCurrentTime
			pServerStatus->SetInternalCurrentTime(OpcUa_DateTime_UtcNow());
			// SetStartTime
			pServerStatus->SetStartTime(dtStartTime);
			// SetServerState
			pServerStatus->SetServerState(OpcUa_ServerState_Running); // OpcUa_ServerState_NoConfiguration
			// SecondsTillShutdown
			pServerStatus->SetSecondsTillShutdown(0);
			OpcUa_LocalizedText ltVal;
			OpcUa_LocalizedText_Initialize(&ltVal);
			// Remplissage de ltVal avec un valeur par défaut
			OpcUa_String_AttachCopy(&(ltVal.Text), "*");
			OpcUa_String_AttachCopy(&(ltVal.Locale), "en-us");
			pServerStatus->SetShutdownReason(ltVal);
			OpcUa_LocalizedText_Clear(&ltVal);
			// Maintenant que le serverStatus interne est initialisé on va initialiser le Node associé

			CUAVariable* pUAVariable = NULL;
			OpcUa_Mutex_Lock(GetServerCacheMutex());
			uStatus = GetNodeIdFromVariableList(aNodeId, &pUAVariable);
			if (uStatus == OpcUa_Good)
			{	//
				CDataValue* pDataValue = pUAVariable->GetValue();
				if (pDataValue)
				{
					// 
					OpcUa_EncodeableType* pEncodeableType;

					uStatus = g_pTheApplication->LookupEncodeableType(pUAVariable->GetDataType().Identifier.Numeric, &pEncodeableType);
					if (uStatus != OpcUa_Good)
					{
						char* szNodeId = OpcUa_Null;
						Utils::NodeId2String(&aNodeId, &szNodeId);
						if (szNodeId)
						{
							OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "CUAInformationModel::InitServerStatus> an unknown EncodeableType on nodeId=%s\n", szNodeId);
							free(szNodeId);
						}
					}
					else
					{
						if (pEncodeableType)
						{
							OpcUa_DataValue* pInternalDataValue=pDataValue->GetInternalDataValue();
							pInternalDataValue->Value.ArrayType= OpcUa_VariantArrayType_Scalar;
							pInternalDataValue->Value.Datatype = OpcUaType_ExtensionObject;
							OpcUa_ServerStatusDataType* pInternalServerStatus=pServerStatus->GetInternalServerStatus();
							if (!pInternalDataValue->Value.Value.ExtensionObject)
								pInternalDataValue->Value.Value.ExtensionObject = (OpcUa_ExtensionObject*)OpcUa_Alloc(sizeof(OpcUa_ExtensionObject));
							OpcUa_ExtensionObject_Initialize(pInternalDataValue->Value.Value.ExtensionObject);
							pInternalDataValue->Value.Value.ExtensionObject->Body.EncodeableObject.Type = pEncodeableType;
							OpcUa_ServerStatusDataType* pServerStatusDataType = Utils::Copy(pInternalServerStatus);
							if (pServerStatusDataType)
								pInternalDataValue->Value.Value.ExtensionObject->Body.EncodeableObject.Object = pServerStatusDataType;// 
							OpcUa_ExpandedNodeId_Initialize(&(pInternalDataValue->Value.Value.ExtensionObject->TypeId));
							// initialisation de l'OpcUa_ExpandedNodeId qui contient le type d'encodage utilisé
							pInternalDataValue->Value.Value.ExtensionObject->TypeId.ServerIndex = 0;
							OpcUa_String_Initialize(&(pInternalDataValue->Value.Value.ExtensionObject->TypeId.NamespaceUri));
							pInternalDataValue->Value.Value.ExtensionObject->TypeId.NodeId.Identifier.Numeric = pEncodeableType->BinaryEncodingTypeId; //OpcUaId_ServerStatusDataType;
							pInternalDataValue->Value.Value.ExtensionObject->TypeId.NodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
							pInternalDataValue->Value.Value.ExtensionObject->TypeId.NodeId.NamespaceIndex = 0;
							pInternalDataValue->Value.Value.ExtensionObject->Encoding = OpcUa_ExtensionObjectEncoding_EncodeableObject;
							pUAVariable->GetValue()->SetStatusCode(OpcUa_Good);
						}
					}
				}
			}
			OpcUa_Mutex_Unlock(GetServerCacheMutex());
		}
		else
		{
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critical error>BuildInfo class is not properly initialized.\n");
			uStatus = OpcUa_BadOutOfMemory;
		}
	}
	else
		uStatus=OpcUa_BadInvalidArgument;
	return uStatus;
}
// Initialisation des NodeId Mandatory
OpcUa_StatusCode CUAInformationModel::InitMandatoryNodeId()
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_Variant varVal;
	OpcUa_Variant_Initialize(&varVal);
	CServerStatus* pServerStatus=GetInternalServerStatus();
	pServerStatus->SetStartTime(OpcUa_DateTime_UtcNow());
	// Init des LocaleIdArray 2271
	InitLocaleIdArray();
	// Allows Null value in the server
	InitAllowNulls(OpcUa_False);
	// initialisation du SamplingIntervalDiagnosticsArray
	InitSamplingIntervalDiagnosticsArray();
	// Initialisation du RedundantServerArray
	InitRedundantServerArray();

	OpcUa_CharA* szServerArray=OpcUa_Null;//{"urn:MARS:UA_CPP_SDK:UA4CEServer"};	
	BuildApplicationUri((OpcUa_CharA**)&szServerArray);
	if (szServerArray)
	{
		InitServerArray(szServerArray);
		OpcUa_Free(szServerArray);
		szServerArray = OpcUa_Null;
	}
	//  initialisation du serveur status
	// ATTENTION il existe deux instance du serveur status 2256 et 2007
	OpcUa_NodeId aNodeId;
	OpcUa_NodeId_Initialize(&aNodeId);
	aNodeId.IdentifierType=OpcUa_IdentifierType_Numeric;
	aNodeId.Identifier.Numeric=2256; // ServerStatus InitMandatoryNodeId
	aNodeId.NamespaceIndex=0; // opc foundation namespace
	OpcUa_DateTime dtStartTime=OpcUa_DateTime_UtcNow();
	uStatus=InitServerStatus(aNodeId,dtStartTime);
	// Deuxième instance du serverStatus (no need to init this one)
	//aNodeId.Identifier.Numeric=2007; // ServerStatus 
	//uStatus=InitServerStatus(aNodeId,dtStartTime);
	// 
	CUABase* pUABase = OpcUa_Null;
	CUAVariable* pUAVariable = OpcUa_Null;
	OpcUa_NodeId_Initialize(&aNodeId);
	// StartTime
	aNodeId.Identifier.Numeric=2257; 
	aNodeId.IdentifierType=OpcUa_IdentifierType_Numeric;
	aNodeId.NamespaceIndex=0; // opc foundation namespace
	//
	pUAVariable=OpcUa_Null;
	{
		OpcUa_Mutex_Lock(GetServerCacheMutex());
		uStatus = GetNodeIdFromVariableList(aNodeId, &pUAVariable);
		if (uStatus == OpcUa_Good)
		{
			OpcUa_Variant_Initialize(&varVal);
			
			CDataValue* pValue=pUAVariable->GetValue();
			varVal=pValue->GetValue();
			varVal.Datatype=OpcUaType_DateTime;
			varVal.Value.DateTime=pServerStatus->GetStartTime();
			pValue->SetValue(varVal);
			// Mise en place de la qualité
			pValue->SetStatusCode(OpcUa_Good);
			// initialisation de l'heure
			pValue->SetServerTimestamp(OpcUa_DateTime_UtcNow());
			pValue->SetSourceTimestamp(OpcUa_DateTime_UtcNow());
			// transfert de la valeur mise à jour dans le dictionnaire
			pUAVariable->SetValue(pValue);
		}
		OpcUa_Mutex_Unlock(GetServerCacheMutex());
	}
	// CurrentTime 2258
	OpcUa_NodeId_Initialize(&aNodeId);
	aNodeId.Identifier.Numeric=2258; // CurrentTime
	aNodeId.IdentifierType=OpcUa_IdentifierType_Numeric;
	aNodeId.NamespaceIndex=0; // opc foundation namespace
	//
	{
		OpcUa_Mutex_Lock(GetServerCacheMutex());
		uStatus = GetNodeIdFromVariableList(aNodeId, &pUAVariable);
		if (uStatus == OpcUa_Good)
		{
			OpcUa_Variant_Initialize(&varVal);
			
			CDataValue* pValue=pUAVariable->GetValue();
			varVal=pValue->GetValue();
			varVal.Datatype=OpcUaType_DateTime;
			varVal.Value.DateTime=pServerStatus->GetInternalCurrentTime(); //OpcUa_DateTime_UtcNow();
			pValue->SetValue(varVal);
			// Mise en place de la qualité
			pValue->SetStatusCode(OpcUa_Good);
			// initialisation de l'heure
			pValue->SetServerTimestamp(OpcUa_DateTime_UtcNow());
			pValue->SetSourceTimestamp(OpcUa_DateTime_UtcNow());
			// transfert de la valeur mise à jour dans le dictionnaire
			pUAVariable->SetValue(pValue);
		}
		OpcUa_Mutex_Unlock(GetServerCacheMutex());
	}
	// State 2259. Il s'agit d'une enumération qui contient l'etat du serveur
	pUABase = OpcUa_Null;
	OpcUa_NodeId_Initialize(&aNodeId);
	aNodeId.IdentifierType=OpcUa_IdentifierType_Numeric;
	aNodeId.Identifier.Numeric=2259; // State
	{
		OpcUa_Mutex_Lock(GetServerCacheMutex());
		uStatus = GetNodeIdFromDictionnary(aNodeId, &pUABase);
		if (uStatus == OpcUa_Good)
		{
			
			pUAVariable=(CUAVariable*)pUABase;
			// céation d'un CDataValue pour mettre en forme la donnée
			CDataValue* pValue=pUAVariable->GetValue();
			OpcUa_Variant aVariant=pValue->GetValue();
			OpcUa_Variant_Initialize(&aVariant);
			aVariant.Datatype=pUAVariable->GetBuiltInType(); // OpcUaType_Int32;
			aVariant.Value.Int32=OpcUa_ServerState_Running;
			pValue->SetValue(aVariant);
			// Mise en place de la qualité
			pValue->SetStatusCode(OpcUa_Good);
			pUAVariable->SetValueRank(-1);
			// initialisation de l'heure
			pValue->SetServerTimestamp(OpcUa_DateTime_UtcNow());
			pValue->SetSourceTimestamp(OpcUa_DateTime_UtcNow());
			// Création du Opcua_Variant encapsulé
			pUAVariable->SetValue(pValue);
		}
		OpcUa_Mutex_Unlock(GetServerCacheMutex());
	}
	// MaxBrowseContinuationPoints
	pUABase = OpcUa_Null;
	OpcUa_NodeId_Initialize(&aNodeId);
	aNodeId.IdentifierType=OpcUa_IdentifierType_Numeric;
	aNodeId.Identifier.Numeric=2735; // MaxBrowseContinuationPoints
	{
		OpcUa_Mutex_Lock(GetServerCacheMutex());
		uStatus = GetNodeIdFromDictionnary(aNodeId, &pUABase);
		if (uStatus == OpcUa_Good)
		{
			
			pUAVariable=(CUAVariable*)pUABase;
			// céation d'un CDataValue pour mettre en forme la donnée
			CDataValue* pValue=pUAVariable->GetValue();
			OpcUa_Variant aVariant=pValue->GetValue();
			OpcUa_Variant_Initialize(&aVariant);
			aVariant.Datatype=OpcUaType_Int16;
			aVariant.Value.Int16=OPENOPCUA_MAX_BROWSE_CP; // 1 continuationPoint
			pValue->SetValue(aVariant);
			// Mise en place de la qualité
			pValue->SetStatusCode(OpcUa_Good);
			pUAVariable->SetValueRank(-1);
			// initialisation de l'heure
			pValue->SetServerTimestamp(OpcUa_DateTime_UtcNow());
			pValue->SetSourceTimestamp(OpcUa_DateTime_UtcNow());
			// Création du Opcua_Variant encapsulé
			pUAVariable->SetValue(pValue);
		}
		OpcUa_Mutex_Unlock(GetServerCacheMutex());
	}
	// BuildInfo
	uStatus=InitBuildInfo();
	if (uStatus!=OpcUa_Good)
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"InitBuildInfo failed 0x%05x\n",uStatus);
	// ServerDiagnostics
	uStatus=InitServerDiagnostics();

	return uStatus;
}
// Initialisation de LocaleIdArray 2271
// il s'agit d'allouer un tableau à 1 dimension
// Ce tableau contiendra les langues (locale) supportées par ce serveur
OpcUa_StatusCode CUAInformationModel::InitLocaleIdArray()
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_NodeId aNodeId;
	OpcUa_NodeId_Initialize(&aNodeId);
	aNodeId.Identifier.Numeric = OpcUaId_Server_ServerCapabilities_LocaleIdArray; // 2271
	aNodeId.IdentifierType=OpcUa_IdentifierType_Numeric;
	aNodeId.NamespaceIndex=0; // opc foundation namespace
	CUAVariable* pUAVariable=NULL;

	{
		OpcUa_Mutex_Lock(GetServerCacheMutex());
		uStatus = GetNodeIdFromVariableList(aNodeId, &pUAVariable);
		if (uStatus == OpcUa_Good)
		{
			CDataValue* pDataValue=pUAVariable->GetValue();
			if (pDataValue)
			{
				OpcUa_Variant aVariant=pDataValue->GetValue();
				if (aVariant.ArrayType==OpcUa_VariantArrayType_Array)
				{
					aVariant.Value.Array.Length=2;
					aVariant.Value.Array.Value.StringArray=(OpcUa_String*)OpcUa_Alloc(2*sizeof(OpcUa_String));
					OpcUa_String_Initialize(&(aVariant.Value.Array.Value.StringArray[0]));
					OpcUa_String_AttachCopy(
						&(aVariant.Value.Array.Value.StringArray[0]),(OpcUa_CharA*)"en-EN");
					OpcUa_String_Initialize(&(aVariant.Value.Array.Value.StringArray[1]));
					OpcUa_String_AttachCopy(
						&(aVariant.Value.Array.Value.StringArray[1]),(OpcUa_CharA*)"fr-FR");

					pDataValue->SetValue(aVariant);
					pDataValue->SetStatusCode(OpcUa_Good);
				}
			}
		}
		else
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Critical error LocaleIdArray(i=2271) cannot be found in your AddressSpace.\n");
		OpcUa_Mutex_Unlock(GetServerCacheMutex());
	}

	return uStatus;
}

// Function name   : CUAInformationModel::InitBuildInfo
// Description     : Initialisation de la BuildInfo 2260
// Return type     : OpcUa_StatusCode 

OpcUa_StatusCode CUAInformationModel::InitBuildInfo()
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_NodeId aNodeId;
	CServerStatus* pServerStatus=GetInternalServerStatus();
	if (pServerStatus)
	{
		//CBuildInfo* pBuildInfo=pServerStatus->GetBuildInfo();
		//if (pBuildInfo)
		{
			aNodeId.Identifier.Numeric=2260; // BuildInfo
			aNodeId.IdentifierType=OpcUa_IdentifierType_Numeric;
			aNodeId.NamespaceIndex=0; // opc foundation namespace
			CUAVariable* pUAVariable=NULL;
			// BuildInfo
			{
				OpcUa_Mutex_Lock(GetServerCacheMutex());
				uStatus = GetNodeIdFromVariableList(aNodeId, &pUAVariable);
				if (uStatus == OpcUa_Good)
				{
					CDataValue* pDataValue=pUAVariable->GetValue();
					if (pDataValue)
					{
						OpcUa_BuildInfo* pInternalBuildInfo = pServerStatus->GetInternalBuildInfo();
						if (pInternalBuildInfo)
						{
							OpcUa_Variant aVariant;
							OpcUa_Variant_Initialize(&aVariant);
							aVariant.Datatype=OpcUaType_ExtensionObject;
							aVariant.ArrayType=OpcUa_VariantArrayType_Scalar;
							aVariant.Value.ExtensionObject=(OpcUa_ExtensionObject*)OpcUa_Alloc(sizeof(OpcUa_ExtensionObject));
							// recherche dans la liste des OpcUa_EncodeableType (OpcUa_EncodeableTypeTable)
							OpcUa_EncodeableType* pEncodeableType=OpcUa_Null;
							uStatus=g_pTheApplication->LookupEncodeableType(pUAVariable->GetDataType().Identifier.Numeric,&pEncodeableType);
							if (uStatus!=OpcUa_Good)
								OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Critical Error:> an unknown EncodeableType\n");
							else
							{
								if (pEncodeableType)
								{
									aVariant.Value.ExtensionObject->Body.EncodeableObject.Type = pEncodeableType;// Utils::Copy(pEncodeableType);
									aVariant.Value.ExtensionObject->Body.EncodeableObject.Object = (void*)Utils::Copy(pInternalBuildInfo);
									OpcUa_ExpandedNodeId_Initialize(&(aVariant.Value.ExtensionObject->TypeId));
									// initialisation de l'OpcUa_ExpandedNodeId qui contient le type d'encodage utilisé
									// par défaut on utilisera l'encodage binaire.. prévoir un switch pour l'encodageXML
									aVariant.Value.ExtensionObject->TypeId.ServerIndex=0;
									OpcUa_String_Initialize(&(aVariant.Value.ExtensionObject->TypeId.NamespaceUri));
									aVariant.Value.ExtensionObject->TypeId.NodeId.Identifier.Numeric = pEncodeableType->BinaryEncodingTypeId;//
									aVariant.Value.ExtensionObject->TypeId.NodeId.IdentifierType=OpcUa_IdentifierType_Numeric;
									aVariant.Value.ExtensionObject->TypeId.NodeId.NamespaceIndex=0;

									aVariant.Value.ExtensionObject->BodySize=0;
									aVariant.Value.ExtensionObject->Encoding=OpcUa_ExtensionObjectEncoding_EncodeableObject;
									// transfert dans le CDataValue
									pDataValue->SetValue(aVariant);
									pDataValue->SetStatusCode(OpcUa_Good);
									//OpcUa_ExtensionObject_Clear(aVariant.Value.ExtensionObject);
									//OpcUa_Free(aVariant.Value.ExtensionObject);
									//OpcUa_Free(pEncodeableType);
								}
							}
							OpcUa_Variant_Clear(&aVariant);
						}
						else
						{
							uStatus=OpcUa_BadInvalidArgument;
							OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Critical Error:> Internal BuildInfo structure was not properly initialized\n");
						}
					}
				}
				OpcUa_Mutex_Unlock(GetServerCacheMutex());
			}
			if (uStatus==OpcUa_Good)
			{
				// On va maintenant initialiser les composants(Node) de buildInfo
				OpcUa_BuildInfo* pBuildInfo = pServerStatus->GetInternalBuildInfo();
				if (pBuildInfo)
				{
					// ProductName
					aNodeId.Identifier.Numeric = 2261;
					{
						OpcUa_Mutex_Lock(GetServerCacheMutex());
						uStatus = GetNodeIdFromVariableList(aNodeId, &pUAVariable);
						if (uStatus == OpcUa_Good)
						{
							OpcUa_Variant varVal;
							OpcUa_Variant_Initialize(&varVal);

							CDataValue* pValue = pUAVariable->GetValue();
							varVal = pValue->GetValue();
							varVal.Datatype = OpcUaType_String;
							OpcUa_String_StrnCpy(&(varVal.Value.String), &(pBuildInfo->ProductName), OpcUa_String_StrLen(&(pBuildInfo->ProductName)));
							pValue->SetValue(varVal);
							// Mise en place de la qualité
							pValue->SetStatusCode(OpcUa_Good);
							// initialisation de l'heure
							pValue->SetServerTimestamp(OpcUa_DateTime_UtcNow());
							pValue->SetSourceTimestamp(OpcUa_DateTime_UtcNow());
							// transfert de la valeur mise à jour dans le dictionnaire
							//pUAVariable->SetValue(pValue);
							OpcUa_Variant_Clear(&varVal);
						}
						OpcUa_Mutex_Unlock(GetServerCacheMutex());
					}
					// ProductUri
					aNodeId.Identifier.Numeric = 2262;
					{
						OpcUa_Mutex_Lock(GetServerCacheMutex());
						uStatus = GetNodeIdFromVariableList(aNodeId, &pUAVariable);
						if (uStatus == OpcUa_Good)
						{
							OpcUa_Variant varVal;
							OpcUa_Variant_Initialize(&varVal);

							CDataValue* pValue = pUAVariable->GetValue();
							varVal = pValue->GetValue();
							varVal.Datatype = OpcUaType_String;
							OpcUa_String_StrnCpy(&(varVal.Value.String), &(pBuildInfo->ProductUri), OpcUa_String_StrLen(&(pBuildInfo->ProductUri)));
							//varVal.Value.String = pBuildInfo->GetProductUri();
							pValue->SetValue(varVal);
							// Mise en place de la qualité
							pValue->SetStatusCode(OpcUa_Good);
							// initialisation de l'heure
							pValue->SetServerTimestamp(OpcUa_DateTime_UtcNow());
							pValue->SetSourceTimestamp(OpcUa_DateTime_UtcNow());
							// transfert de la valeur mise à jour dans le dictionnaire
							//pUAVariable->SetValue(pValue);
							OpcUa_Variant_Clear(&varVal);
						}
						OpcUa_Mutex_Unlock(GetServerCacheMutex());
					}
					// ManufacturerName
					aNodeId.Identifier.Numeric = 2263;
					{
						OpcUa_Mutex_Lock(GetServerCacheMutex());
						uStatus = GetNodeIdFromVariableList(aNodeId, &pUAVariable);
						if (uStatus == OpcUa_Good)
						{
							OpcUa_Variant varVal;
							OpcUa_Variant_Initialize(&varVal);

							CDataValue* pValue = pUAVariable->GetValue();
							varVal = pValue->GetValue();
							varVal.Datatype = OpcUaType_String;
							OpcUa_String_StrnCpy(&(varVal.Value.String), &(pBuildInfo->ManufacturerName), OpcUa_String_StrLen(&(pBuildInfo->ManufacturerName)));
							//varVal.Value.String = pBuildInfo->GetManufacturerName();
							pValue->SetValue(varVal);
							// Mise en place de la qualité
							pValue->SetStatusCode(OpcUa_Good);
							// initialisation de l'heure
							pValue->SetServerTimestamp(OpcUa_DateTime_UtcNow());
							pValue->SetSourceTimestamp(OpcUa_DateTime_UtcNow());
							// transfert de la valeur mise à jour dans le dictionnaire
							//pUAVariable->SetValue(pValue);
							OpcUa_Variant_Clear(&varVal);
						}
						OpcUa_Mutex_Unlock(GetServerCacheMutex());
					}
					// SoftwareVersion
					aNodeId.Identifier.Numeric = 2264;
					{
						OpcUa_Mutex_Lock(GetServerCacheMutex());
						uStatus = GetNodeIdFromVariableList(aNodeId, &pUAVariable);
						if (uStatus == OpcUa_Good)
						{
							OpcUa_Variant varVal;
							OpcUa_Variant_Initialize(&varVal);

							CDataValue* pValue = pUAVariable->GetValue();
							varVal = pValue->GetValue();
							varVal.Datatype = OpcUaType_String;
							OpcUa_String_StrnCpy(&(varVal.Value.String), &(pBuildInfo->SoftwareVersion), OpcUa_String_StrLen(&(pBuildInfo->SoftwareVersion)));
							//varVal.Value.String = pBuildInfo->GetSoftwareVersion();
							pValue->SetValue(varVal);
							// Mise en place de la qualité
							pValue->SetStatusCode(OpcUa_Good);
							// initialisation de l'heure
							pValue->SetServerTimestamp(OpcUa_DateTime_UtcNow());
							pValue->SetSourceTimestamp(OpcUa_DateTime_UtcNow());
							// transfert de la valeur mise à jour dans le dictionnaire
							//pUAVariable->SetValue(pValue);
							OpcUa_Variant_Clear(&varVal);
						}
						OpcUa_Mutex_Unlock(GetServerCacheMutex());
					}
					// BuildNumber
					aNodeId.Identifier.Numeric = 2265;
					{
						OpcUa_Mutex_Lock(GetServerCacheMutex());
						uStatus = GetNodeIdFromVariableList(aNodeId, &pUAVariable);
						if (uStatus == OpcUa_Good)
						{
							OpcUa_Variant varVal;
							OpcUa_Variant_Initialize(&varVal);

							CDataValue* pValue = pUAVariable->GetValue();
							varVal = pValue->GetValue();
							varVal.Datatype = OpcUaType_String;
							OpcUa_String_StrnCpy(&(varVal.Value.String), &(pBuildInfo->BuildNumber), OpcUa_String_StrLen(&(pBuildInfo->BuildNumber)));
							//varVal.Value.String = pBuildInfo->GetBuildNumber();
							pValue->SetValue(varVal);
							// Mise en place de la qualité
							pValue->SetStatusCode(OpcUa_Good);
							// initialisation de l'heure
							pValue->SetServerTimestamp(OpcUa_DateTime_UtcNow());
							pValue->SetSourceTimestamp(OpcUa_DateTime_UtcNow());
							// transfert de la valeur mise à jour dans le dictionnaire
							//pUAVariable->SetValue(pValue);
							OpcUa_Variant_Clear(&varVal);
						}
						OpcUa_Mutex_Unlock(GetServerCacheMutex());
					}
					// BuildDate
					aNodeId.Identifier.Numeric = 2266;
					{
						OpcUa_Mutex_Lock(GetServerCacheMutex());
						uStatus = GetNodeIdFromVariableList(aNodeId, &pUAVariable);
						if (uStatus == OpcUa_Good)
						{
							OpcUa_Variant varVal;
							OpcUa_Variant_Initialize(&varVal);

							CDataValue* pValue = pUAVariable->GetValue();
							varVal = pValue->GetValue();
							varVal.Datatype = OpcUaType_DateTime;
							
							varVal.Value.DateTime = pBuildInfo->BuildDate; // pBuildInfo->GetBuildDate();
							pValue->SetValue(varVal);
							// Mise en place de la qualité
							pValue->SetStatusCode(OpcUa_Good);
							// initialisation de l'heure
							pValue->SetServerTimestamp(OpcUa_DateTime_UtcNow());
							pValue->SetSourceTimestamp(OpcUa_DateTime_UtcNow());
							// transfert de la valeur mise à jour dans le dictionnaire
							//pUAVariable->SetValue(pValue);
							OpcUa_Variant_Clear(&varVal);
						}
						OpcUa_Mutex_Unlock(GetServerCacheMutex());
					}
				}
				else
					uStatus = OpcUa_BadInvalidArgument;
			}
		}
		//else
		//	uStatus=OpcUa_BadInvalidArgument;
	}
	else
		uStatus=OpcUa_BadInvalidArgument;
	return uStatus;
}
// Thread pour la mise à jour des Nodes oblgatoires
// 

void CUAInformationModel::StartUpdateMandatoryNodesThread()
{
	if (m_hUpdateMandatoryNodesThread==NULL)
	{
		m_bRunUpdateMandatoryNodesThread = TRUE;
		OpcUa_StatusCode uStatus  = OpcUa_Thread_Create(&m_hUpdateMandatoryNodesThread,UpdateMandatoryNodesThread,this);

		if(uStatus != OpcUa_Good)
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Create UpdateMandatoryNodesThread Failed");
		else
			OpcUa_Thread_Start(m_hUpdateMandatoryNodesThread);
	}
}

///-------------------------------------------------------------------------------------------------
/// <summary>	This thread willl update some mandatory node :
///				2257, 2258, 2259.  In the same loop the ServerDiagnosticSummary will be updated.
///				This include 2275, 2276,2277,2278,2279,2281,2282,2284,2285,2286,2287,2288, 3705
///				The default updat rate is 100 ms</summary>
///
/// <remarks>	Michel, 26/05/2016. </remarks>
///
/// <param name="arg">	The argument. </param>
///-------------------------------------------------------------------------------------------------

void CUAInformationModel::UpdateMandatoryNodesThread(LPVOID arg)
{
	//DWORD dwInterval=0;
	OpcUa_StatusCode uStatus = OpcUa_Good;

	CUAInformationModel* pInformationModel=(CUAInformationModel*)arg;
	OpcUa_Double dblInterval=100; // par défaut mise a jour à 100 ms

	OpcUa_Semaphore_TimedWait(pInformationModel->m_SemMandatoryEvent,OPCUA_INFINITE);
	// initialisation de la valeur par défaut de certaines Node issue du fichier NodeSet2.xml
	pInformationModel->InitMandatoryNodeId();
	while (pInformationModel->m_bRunUpdateMandatoryNodesThread)
	{
#if _WIN32_WINNT >= 0x0600
		ULONGLONG dwStart = GetTickCount64();
#else 
	#if _WIN32_WINNT == 0x0501
		DWORD dwStart = GetTickCount();
	#endif
#endif
#ifdef _GNUC_
		DWORD dwStart = GetTickCount();
#endif
		OpcUa_UInt32 uiSleepTime = ((OpcUa_UInt32)dblInterval);
		// Mise à jour des variables mandatory

		// Update the internal ServerStatus
		CServerStatus* pServerStatus=pInformationModel->GetInternalServerStatus();		
		pServerStatus->SetInternalCurrentTime(OpcUa_DateTime_UtcNow());
		pServerStatus->SetServerState(OpcUa_ServerState_Running);
		// CurrentTime 2258
		OpcUa_NodeId aNodeId;
		OpcUa_NodeId_Initialize(&aNodeId);
		aNodeId.Identifier.Numeric=2258; // CurrentTime
		aNodeId.IdentifierType=OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex=0; // opc foundation namespace
		////
		CUAVariable* pUAVariable=NULL;
		OpcUa_Mutex_Lock(pInformationModel->GetServerCacheMutex());
		uStatus = pInformationModel->GetNodeIdFromVariableList(aNodeId, &pUAVariable);
		if (uStatus == OpcUa_Good)
		{
			OpcUa_Variant varVal;
			OpcUa_Variant_Initialize(&varVal);
			CDataValue* pValue=pUAVariable->GetValue();
			varVal=pValue->GetValue();
			varVal.Datatype=OpcUaType_DateTime;
			varVal.Value.DateTime=pServerStatus->GetInternalCurrentTime();
			pValue->SetValue(varVal);
			// Mise en place de la qualité
			pValue->SetStatusCode(OpcUa_Good);
			// initialisation de l'heure
			pValue->SetServerTimestamp(pServerStatus->GetInternalCurrentTime());
			pValue->SetSourceTimestamp(pServerStatus->GetInternalCurrentTime());
			// transfert de la valeur mise à jour dans le dictionnaire
			pUAVariable->SetValue(pValue);
		}
		// Update the serverDiagnosticsSummary 2275, etc.
		pInformationModel->UpdateServerDiagnostics();
		OpcUa_Mutex_Unlock(pInformationModel->GetServerCacheMutex());

		///////////////////////////////////////////
		//
		// ServerStatus
		//
		//////////////////////////////////////////
		pUAVariable=NULL;
		OpcUa_NodeId_Initialize(&aNodeId);
		aNodeId.Identifier.Numeric = OpcUaId_Server_ServerStatus; // ServerStatus UpdateMandatoryNodesThread
		aNodeId.IdentifierType=OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex=0; // opc foundation namespace
		{
			OpcUa_Mutex_Lock(pInformationModel->GetServerCacheMutex());
			uStatus = pInformationModel->GetNodeIdFromVariableList(aNodeId, &pUAVariable);
			if (uStatus == OpcUa_Good)
			{
				///////////////////////////////////////////////////////////////////////
				CDataValue* pDataValue=pUAVariable->GetValue();
				OpcUa_Variant aVariant=pDataValue->GetValue();
				if (aVariant.Value.ExtensionObject)
				{
					OpcUa_ServerStatusDataType* pInternalServerStatus=(OpcUa_ServerStatusDataType*)aVariant.Value.ExtensionObject->Body.EncodeableObject.Object;
					pInternalServerStatus->CurrentTime=pServerStatus->GetInternalCurrentTime();

					if (pDataValue)
					{
						if( pDataValue->GetValue().Value.ExtensionObject)
						{
							if (pServerStatus)
							{
								// 
								// mise a jour des nodes encapsulées 2257,2258,2259,2260
								OpcUa_NodeId aTmpNodeId;
								OpcUa_NodeId_Initialize(&aTmpNodeId);
								CUAVariable* pUATmpVariable=NULL;
								aTmpNodeId.IdentifierType=OpcUa_IdentifierType_Numeric;
								OpcUa_Variant varVal;
								OpcUa_Variant_Initialize(&varVal);
								aTmpNodeId.Identifier.Numeric=2258; // CurrentTime
								uStatus = pInformationModel->GetNodeIdFromVariableList(aTmpNodeId, &pUATmpVariable);
								if (uStatus == OpcUa_Good)
								{
									CDataValue* pTmpDataValue=pUATmpVariable->GetValue();
									varVal.Datatype=OpcUaType_DateTime;
									varVal.Value.DateTime=pServerStatus->GetInternalCurrentTime();
									pTmpDataValue->SetValue(varVal);
								}
								aTmpNodeId.Identifier.Numeric=2257; // StartTime
								uStatus = pInformationModel->GetNodeIdFromVariableList(aTmpNodeId, &pUATmpVariable);
								if (uStatus == OpcUa_Good)
								{
									CDataValue* pTmpDataValue = pUATmpVariable->GetValue();
									varVal.Datatype=OpcUaType_DateTime;
									varVal.Value.DateTime=pServerStatus->GetStartTime();
									pTmpDataValue->SetValue(varVal);
								}
								OpcUa_Variant_Initialize(&varVal);
								varVal.Datatype=OpcUaType_Int32;
								aTmpNodeId.Identifier.Numeric=2259; // State
								uStatus = pInformationModel->GetNodeIdFromVariableList(aTmpNodeId, &pUATmpVariable);
								if (uStatus==OpcUa_Good)
								{
									CDataValue* pTmpDataValue = pUATmpVariable->GetValue();
									varVal.Datatype=OpcUaType_Int32;
									varVal.Value.Int32=pServerStatus->GetServerState();
									pTmpDataValue->SetValue(varVal);
								}

								pServerStatus->GetSecondsTillShutdown();
								pServerStatus->GetShutdownReason();
								OpcUa_NodeId_Clear(&aTmpNodeId);
								OpcUa_Variant_Clear(&varVal);
							}
						}
					}
				}
			}
			OpcUa_Mutex_Unlock(pInformationModel->GetServerCacheMutex());
		}
		// Calcul du temps écoulé et pause
#if _WIN32_WINNT >= 0x0600
		ULONGLONG dwEnd = GetTickCount64();
		ULONGLONG dwCountedTime = dwEnd - dwStart;
		if (uiSleepTime>dwCountedTime)
			uiSleepTime = (OpcUa_UInt32)(uiSleepTime - dwCountedTime);
		else
			uiSleepTime = 0;
		OpcUa_Semaphore_TimedWait(pInformationModel->m_hStopUpdateMandatoryNodesThreadSem, uiSleepTime);
#else 
	#if _WIN32_WINNT == 0x0501
		DWORD dwEnd = GetTickCount();
		DWORD dwCountedTime = dwEnd - dwStart;
		if (uiSleepTime>dwCountedTime)
			uiSleepTime = uiSleepTime - dwCountedTime;
		else
			uiSleepTime = 0;
		OpcUa_Semaphore_TimedWait(pInformationModel->m_hStopUpdateMandatoryNodesThreadSem, uiSleepTime);
	#endif
#endif
#ifdef _GNUC_
		DWORD dwEnd=GetTickCount();
		ULONGLONG dwCountedTime = dwEnd - dwStart;
		if (uiSleepTime > dwCountedTime)
			uiSleepTime = (OpcUa_UInt32)(uiSleepTime - dwCountedTime);
		else
		{
			uiSleepTime = (OpcUa_UInt32)0;
		}
		OpcUa_Semaphore_TimedWait(pInformationModel->m_hStopUpdateMandatoryNodesThreadSem, uiSleepTime);
#endif 
	}
	OpcUa_Semaphore_Post(pInformationModel->m_hStopUpdateMandatoryNodesThreadSem, 1);
	OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"UpdateMandatoryNodesThread stopped\n");
}
OpcUa_StatusCode CUAInformationModel::StopUpdateMandatoryNodesThread()
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	if (m_bRunUpdateMandatoryNodesThread)
	{
		m_bRunUpdateMandatoryNodesThread = FALSE;
		OpcUa_Semaphore_Post(m_SemMandatoryEvent, 1); // Just in case
		OpcUa_Semaphore_Post(m_hStopUpdateMandatoryNodesThreadSem, 1);
		uStatus = OpcUa_Semaphore_TimedWait(m_hStopUpdateMandatoryNodesThreadSem, OPC_TIMEOUT * 2); // 15 secondes max.
		if (uStatus == OpcUa_GoodNonCriticalTimeout)
		{
			// on force la fin du thread de simulation
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Impossible to stop the UpdateMandatoryNodesThread. Timeout\n", 0);
		}
		else
		{
			OpcUa_Thread_Delete(m_hUpdateMandatoryNodesThread);
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "UpdateMandatoryNodesThread stopped properly\n");
		}
		OpcUa_Semaphore_Delete(&m_SemMandatoryEvent);
	}
	return uStatus;
}


// Function name   : CUAInformationModel::UpdateNamespaceArray
// Description     : Update the content of NamespaceArray, NodeId (ns=0;i=2255), according to m_NamespaceUris, vector<CNamespaceUri*>
// Return type     : OpcUa_StatusCode
// Argument        : 

OpcUa_StatusCode CUAInformationModel::UpdateNamespaceArray()
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	CUABase* pUABase=NULL;
	OpcUa_NodeId aNodeId;
	OpcUa_NodeId_Initialize(&aNodeId);
	aNodeId.IdentifierType=OpcUa_IdentifierType_Numeric;
	aNodeId.Identifier.Numeric=OpcUaId_Server_NamespaceArray; //2255;
	{
		OpcUa_Mutex_Lock(GetServerCacheMutex());
		uStatus = GetNodeIdFromDictionnary(aNodeId, &pUABase);
		if (uStatus==OpcUa_Good)
		{
			if (pUABase->GetNodeClass() == OpcUa_NodeClass_Variable)
			{
				CUAVariable* pUAVariable = (CUAVariable*)pUABase;
				if (pUAVariable)
				{
					CDataValue* pDataValue = pUAVariable->GetValue();
					if (!pDataValue)
					{
						// Il s'agit d'un cas spécial dans lequel on doit créer La CDataValue encaspuslé avec le post traitement
						uStatus = pUAVariable->InitializeDataValue();
						pDataValue = pUAVariable->GetValue();
					}
					// re-check after the creation
					if (pDataValue)
					{
						OpcUa_DataValue* pInternalDataValue=pDataValue->GetInternalDataValue();
						// recuperation de la valeur en cours
						//OpcUa_Variant* pVarVal = (OpcUa_Variant*)OpcUa_Alloc(sizeof(OpcUa_Variant));

						pInternalDataValue->Value.ArrayType = OpcUa_VariantArrayType_Array;
						pInternalDataValue->Value.Datatype = OpcUaType_String;
						pInternalDataValue->Value.Value.Array.Length = m_NamespaceUris.size();
						// 
						pInternalDataValue->Value.Value.Array.Value.StringArray = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String)*pInternalDataValue->Value.Value.Array.Length);
						// parcours des namespaceUris et transfert dans la structure temporaire
						for (OpcUa_UInt32 ii = 0; ii < m_NamespaceUris.size(); ii++)
						{
							CNamespaceUri* pNamespaceUri = m_NamespaceUris.at(ii);
							OpcUa_String* pString = pNamespaceUri->GetUriName();
							OpcUa_String_Initialize(&(pInternalDataValue->Value.Value.Array.Value.StringArray[ii]));
							OpcUa_String_CopyTo(pString, &(pInternalDataValue->Value.Value.Array.Value.StringArray[ii]));
						}
						// transfert dans le CUAVariable
						pInternalDataValue->ServerTimestamp = OpcUa_DateTime_UtcNow();
						pInternalDataValue->SourceTimestamp = OpcUa_DateTime_UtcNow();
						pInternalDataValue->StatusCode=OpcUa_Good;
					}
					else
						OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "CUAInformationModel::UpdateNamespaceArray>Critical error pDataValue is Null\n");
				}
				else
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "CUAInformationModel::UpdateNamespaceArray>Critical error pUAVariable is Null\n");
			}
			else
			{
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "CUAInformationModel::UpdateNamespaceArray>Critical error ns=0;i=2255 have not the correct NodeClass");
				uStatus = OpcUa_BadInternalError;
			}
		}
		else
			uStatus=OpcUa_BadNodeIdUnknown;
		OpcUa_Mutex_Unlock(GetServerCacheMutex());
	}
	return uStatus;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Update buildInType for all UAVariables. </summary>
///
/// <remarks>	Michel, 19/09/2015. </remarks>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode CUAInformationModel::UpdateUAVariablesBuiltinType()
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	CUAVariableList* pUAVariableList=GetVariableList();
	OpcUa_Byte bType;
	CUAVariableList::iterator it;
	for (it = (*pUAVariableList).begin(); it != (*pUAVariableList).end();it++)
	{
		CUAVariable* pUAVariable = it->second; 
		OpcUa_NodeId aNodeId = pUAVariable->GetDataType(); 
		// Special case for DataType not declare in the UANodeSet file
		if ( (aNodeId.Identifier.Numeric==0) && (aNodeId.NamespaceIndex==0) )
		{
			OpcUa_NodeId baseDataType;
			OpcUa_NodeId_Initialize(&baseDataType);
			baseDataType.Identifier.Numeric=24;
			baseDataType.NamespaceIndex=0;
			baseDataType.IdentifierType=OpcUa_IdentifierType_Numeric;
			pUAVariable->SetDataType(baseDataType);
		}
		// search for builtInType
		OpcUa_StatusCode uLocalStatus=FindBuiltinType(aNodeId,&bType);
		if (uLocalStatus==OpcUa_Good)
			pUAVariable->SetBuiltInType(bType);
		else
		{
			if (uLocalStatus==OpcUa_BadNotFound)
			{
				char* szNodeId = OpcUa_Null;
				if (Utils::NodeId2String(&aNodeId, &szNodeId) == OpcUa_Good)
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "UpdateBuiltinType>You specify an incorrect NodeId: %s BrowseName=%s\n", szNodeId, OpcUa_String_GetRawString(&(pUAVariable->GetBrowseName()->Name)));
				else
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "UpdateBuiltinType>You specify an incorrect BrowseName=%ls\n", OpcUa_String_GetRawString(&(pUAVariable->GetBrowseName()->Name)));
				if (szNodeId)
					free(szNodeId);
			}
			uStatus=OpcUa_BadDataTypeIdUnknown;
		}
	}
	return uStatus;
}
///-------------------------------------------------------------------------------------------------
/// <summary>	Update EncodeableObject for all UAVariable with datatype i=24 (ExtensionObject) . </summary>
///
/// <remarks>	Michel, 19/09/2015. </remarks>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode CUAInformationModel::UpdateUAVariablesEncodeableObject()
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	CUAVariableList* pUAVariableList=GetVariableList();
	CUAVariableList::iterator it;
	for (it = pUAVariableList->begin(); it != pUAVariableList->end(); it++)
	{
		CUAVariable* pUAVariable = it->second;
		if (pUAVariable->GetBuiltInType()==OpcUaType_ExtensionObject) // 
		{	
			CDataValue* pValue=pUAVariable->GetValue();
			// Attention il ne faut traiter que les extensionObject qui n'ont pas été initialisé a partir du fichier XML
			if (pValue==OpcUa_Null)
			{
				// We need to verify that the extension object is not an abstract dataType
				OpcUa_NodeId aDataType=pUAVariable->GetDataType();
				CUADataType* pUADataType=OpcUa_Null;
				uStatus=GetNodeIdFromDataTypeList(aDataType,&pUADataType);
				if (uStatus!=OpcUa_Good)
				{
					if (pUADataType->IsAbstract())
					{
						char* szNodeId = OpcUa_Null;
						OpcUa_NodeId* pNodeId = pUAVariable->GetNodeId();
						Utils::NodeId2String(pNodeId, &szNodeId);
						if (szNodeId)
						{
							// Attribute
							OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "UpdateUAVariablesEncodeableObject>Error %s use an abstract UADataType\n", szNodeId);
							free(szNodeId);
						}
					}
				}
				uStatus=InitializeEncodeableObject(pUAVariable);
				if (uStatus!=OpcUa_Good)
				{
					char* szNodeId = OpcUa_Null;
					OpcUa_NodeId aNodeId = pUAVariable->GetDataType();
					Utils::NodeId2String(&aNodeId, &szNodeId);
					if (szNodeId)
					{
						OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "InitializeEncodeableObject failed for NodeId %s\n", szNodeId);
						free(szNodeId);
					}
				}
			}
		}
	}
	return uStatus;
}

// initialisation de la node ServerDiagnostics
OpcUa_StatusCode CUAInformationModel::InitServerDiagnostics( )
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	return uStatus;
}
// Mise à jour de la node 2274 et de son contenu
// Il s'agit de mettre a jour les valeurs
OpcUa_StatusCode CUAInformationModel::UpdateServerDiagnostics()
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	if (IsEnabledServerDiagnosticsDefaultValue())
		UpdateServerDiagnosticsSummary();
	return uStatus;
}
// Update the nodes aggregates in 2275
// this include 2276,2277,2278,2279,2281,2282,2284,2285,2286,2287,2288, 3705
OpcUa_StatusCode CUAInformationModel::UpdateServerDiagnosticsSummary()
{
	OpcUa_StatusCode uStatus=OpcUa_BadNotFound;
	OpcUa_NodeId aNodeId;
	OpcUa_NodeId_Initialize(&aNodeId);
	CUABase* pUABase=OpcUa_Null;
	CDataValue* pDataValue=OpcUa_Null;
	
	OpcUa_ServerDiagnosticsSummaryDataType *pServerDiagnosticsSummaryDataType = GetServerDiagnosticsSummaryDataType();
	// ServerDiagnostocsSummary 2275
	aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
	aNodeId.NamespaceIndex = 0;
	aNodeId.Identifier.Numeric = 2275;
	uStatus = GetNodeIdFromFastAccessList(aNodeId, &pUABase);
	if (uStatus == OpcUa_Good)
	{
		pDataValue=((CUAVariable*)pUABase)->GetValue();
		if (pDataValue)
		{
			OpcUa_EncodeableType* pEncodeableType;
			uStatus = g_pTheApplication->LookupEncodeableType(((CUAVariable*)pUABase)->GetDataType().Identifier.Numeric, &pEncodeableType);
			if (uStatus == OpcUa_Good)
			{
				//OpcUa_Variant variantVal = pDataValue->GetValue();
				OpcUa_DataValue* pInternalDataValue = pDataValue->GetInternalDataValue();
				
				if (!pInternalDataValue->Value.Value.ExtensionObject)
					pInternalDataValue->Value.Value.ExtensionObject = (OpcUa_ExtensionObject*)OpcUa_Alloc(sizeof(OpcUa_ExtensionObject));
				else
				{
					if (pInternalDataValue->Value.Value.ExtensionObject->Body.EncodeableObject.Object)
					{
						OpcUa_Free(pInternalDataValue->Value.Value.ExtensionObject->Body.EncodeableObject.Object);
						pInternalDataValue->Value.Value.ExtensionObject->Body.EncodeableObject.Object = OpcUa_Null;
					}
					OpcUa_ExtensionObject_Clear(pInternalDataValue->Value.Value.ExtensionObject);
				}
				OpcUa_ExtensionObject_Initialize(pInternalDataValue->Value.Value.ExtensionObject);
				pInternalDataValue->Value.Value.ExtensionObject->Body.EncodeableObject.Object= (void*)Utils::Copy(pServerDiagnosticsSummaryDataType);
				pInternalDataValue->Value.ArrayType = OpcUa_VariantArrayType_Scalar;
				pInternalDataValue->Value.Datatype = OpcUaType_ExtensionObject;
				
				pInternalDataValue->Value.Value.ExtensionObject->Body.EncodeableObject.Type = pEncodeableType;

				OpcUa_ExpandedNodeId_Initialize(&(pInternalDataValue->Value.Value.ExtensionObject->TypeId));
				// initialisation de l'OpcUa_ExpandedNodeId qui contient le type d'encodage utilisé
				pInternalDataValue->Value.Value.ExtensionObject->TypeId.ServerIndex = 0;
				OpcUa_String_Initialize(&(pInternalDataValue->Value.Value.ExtensionObject->TypeId.NamespaceUri));
				pInternalDataValue->Value.Value.ExtensionObject->TypeId.NodeId.Identifier.Numeric = pEncodeableType->BinaryEncodingTypeId; //OpcUaId_ServerStatusDataType;
				pInternalDataValue->Value.Value.ExtensionObject->TypeId.NodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
				pInternalDataValue->Value.Value.ExtensionObject->TypeId.NodeId.NamespaceIndex = 0;
				pInternalDataValue->Value.Value.ExtensionObject->Encoding = OpcUa_ExtensionObjectEncoding_EncodeableObject;
				// Status code and tiemstamp
				pDataValue->SetStatusCode(OpcUa_Good);
				pDataValue->SetServerTimestamp(OpcUa_DateTime_UtcNow());
				pDataValue->SetSourceTimestamp(OpcUa_DateTime_UtcNow());
			}
		}
	}
	// ServerViewCount 2276
	aNodeId.IdentifierType=OpcUa_IdentifierType_Numeric;
	aNodeId.NamespaceIndex=0;
	aNodeId.Identifier.Numeric=2276;
	uStatus=GetNodeIdFromFastAccessList(aNodeId,&pUABase);
	if (uStatus==OpcUa_Good)
	{
		pDataValue=((CUAVariable*)pUABase)->GetValue();
		OpcUa_DataValue* pInternalDataValue = pDataValue->GetInternalDataValue();
		pInternalDataValue->Value.ArrayType=OpcUa_VariantArrayType_Scalar;
		pInternalDataValue->Value.Value.UInt32 = m_pUAViewList->size();
		pInternalDataValue->Value.Datatype = OpcUaType_UInt32;
		// 
		pInternalDataValue->StatusCode=OpcUa_Good;
		pInternalDataValue->ServerTimestamp= OpcUa_DateTime_UtcNow();
		pInternalDataValue->ServerPicoseconds=0;
		pInternalDataValue->SourceTimestamp =OpcUa_DateTime_UtcNow();
		pInternalDataValue->SourcePicoseconds = 0;
	}
	// CurrentSessionCount 2277
	aNodeId.Identifier.Numeric=2277;
	uStatus=GetNodeIdFromFastAccessList(aNodeId,&pUABase);
	if (uStatus==OpcUa_Good)
	{
		pDataValue = ((CUAVariable*)pUABase)->GetValue();
		OpcUa_DataValue* pInternalDataValue = pDataValue->GetInternalDataValue();
		pInternalDataValue->Value.ArrayType = OpcUa_VariantArrayType_Scalar;
		pInternalDataValue->Value.Value.UInt32 = pServerDiagnosticsSummaryDataType->CurrentSessionCount;
		pInternalDataValue->Value.Datatype = OpcUaType_UInt32;
		// 
		pInternalDataValue->StatusCode = OpcUa_Good;
		pInternalDataValue->ServerTimestamp = OpcUa_DateTime_UtcNow();
		pInternalDataValue->ServerPicoseconds = 0;
		pInternalDataValue->SourceTimestamp = OpcUa_DateTime_UtcNow();
		pInternalDataValue->SourcePicoseconds = 0;
	}
	// CumulatedSessionCount 2278
	aNodeId.Identifier.Numeric = 2278;
	uStatus = GetNodeIdFromFastAccessList(aNodeId, &pUABase);
	if (uStatus == OpcUa_Good)
	{
		pDataValue = ((CUAVariable*)pUABase)->GetValue();
		OpcUa_DataValue* pInternalDataValue = pDataValue->GetInternalDataValue();
		pInternalDataValue->Value.ArrayType = OpcUa_VariantArrayType_Scalar;
		pInternalDataValue->Value.Value.UInt32 = pServerDiagnosticsSummaryDataType->CumulatedSessionCount;
		pInternalDataValue->Value.Datatype = OpcUaType_UInt32;
		// 
		pInternalDataValue->StatusCode = OpcUa_Good;
		pInternalDataValue->ServerTimestamp = OpcUa_DateTime_UtcNow();
		pInternalDataValue->ServerPicoseconds = 0;
		pInternalDataValue->SourceTimestamp = OpcUa_DateTime_UtcNow();
		pInternalDataValue->SourcePicoseconds = 0;
	}
	// SecurityRejectedSessionCount 2279
	aNodeId.Identifier.Numeric = 2279;
	uStatus = GetNodeIdFromFastAccessList(aNodeId, &pUABase);
	if (uStatus == OpcUa_Good)
	{
		pDataValue = ((CUAVariable*)pUABase)->GetValue();
		OpcUa_DataValue* pInternalDataValue = pDataValue->GetInternalDataValue();
		pInternalDataValue->Value.ArrayType = OpcUa_VariantArrayType_Scalar;
		pInternalDataValue->Value.Value.UInt32 = pServerDiagnosticsSummaryDataType->SecurityRejectedSessionCount;
		pInternalDataValue->Value.Datatype = OpcUaType_UInt32;
		// 
		pInternalDataValue->StatusCode = OpcUa_Good;
		pInternalDataValue->ServerTimestamp = OpcUa_DateTime_UtcNow();
		pInternalDataValue->ServerPicoseconds = 0;
		pInternalDataValue->SourceTimestamp = OpcUa_DateTime_UtcNow();
		pInternalDataValue->SourcePicoseconds = 0;
	}	
	// RejectedSessionCount 3705
	aNodeId.Identifier.Numeric = 3705;
	uStatus = GetNodeIdFromFastAccessList(aNodeId, &pUABase);
	if (uStatus == OpcUa_Good)
	{
		pDataValue = ((CUAVariable*)pUABase)->GetValue();
		OpcUa_DataValue* pInternalDataValue = pDataValue->GetInternalDataValue();
		pInternalDataValue->Value.ArrayType = OpcUa_VariantArrayType_Scalar;
		pInternalDataValue->Value.Value.UInt32 = pServerDiagnosticsSummaryDataType->RejectedSessionCount;
		pInternalDataValue->Value.Datatype = OpcUaType_UInt32;
		// 
		pInternalDataValue->StatusCode = OpcUa_Good;
		pInternalDataValue->ServerTimestamp = OpcUa_DateTime_UtcNow();
		pInternalDataValue->ServerPicoseconds = 0;
		pInternalDataValue->SourceTimestamp = OpcUa_DateTime_UtcNow();
		pInternalDataValue->SourcePicoseconds = 0;
	}
	// SessionTimeoutCount 2281
	aNodeId.Identifier.Numeric = 2281;
	uStatus = GetNodeIdFromFastAccessList(aNodeId, &pUABase);
	if (uStatus == OpcUa_Good)
	{
		pDataValue = ((CUAVariable*)pUABase)->GetValue();
		OpcUa_DataValue* pInternalDataValue = pDataValue->GetInternalDataValue();
		pInternalDataValue->Value.ArrayType = OpcUa_VariantArrayType_Scalar;
		pInternalDataValue->Value.Value.UInt32 = pServerDiagnosticsSummaryDataType->SessionTimeoutCount;
		pInternalDataValue->Value.Datatype = OpcUaType_UInt32;
		// 
		pInternalDataValue->StatusCode = OpcUa_Good;
		pInternalDataValue->ServerTimestamp = OpcUa_DateTime_UtcNow();
		pInternalDataValue->ServerPicoseconds = 0;
		pInternalDataValue->SourceTimestamp = OpcUa_DateTime_UtcNow();
		pInternalDataValue->SourcePicoseconds = 0;
	}
	// SessionAbortCount 2282
	aNodeId.Identifier.Numeric = 2282;
	uStatus = GetNodeIdFromFastAccessList(aNodeId, &pUABase);
	if (uStatus == OpcUa_Good)
	{
		pDataValue = ((CUAVariable*)pUABase)->GetValue();
		OpcUa_DataValue* pInternalDataValue = pDataValue->GetInternalDataValue();
		pInternalDataValue->Value.ArrayType = OpcUa_VariantArrayType_Scalar;
		pInternalDataValue->Value.Value.UInt32 = pServerDiagnosticsSummaryDataType->SessionAbortCount;
		pInternalDataValue->Value.Datatype = OpcUaType_UInt32;
		// 
		pInternalDataValue->StatusCode = OpcUa_Good;
		pInternalDataValue->ServerTimestamp = OpcUa_DateTime_UtcNow();
		pInternalDataValue->ServerPicoseconds = 0;
		pInternalDataValue->SourceTimestamp = OpcUa_DateTime_UtcNow();
		pInternalDataValue->SourcePicoseconds = 0;
	}
	// PublishingIntervalCount 2284
	aNodeId.Identifier.Numeric = 2284;
	uStatus = GetNodeIdFromFastAccessList(aNodeId, &pUABase);
	if (uStatus == OpcUa_Good)
	{
		pDataValue = ((CUAVariable*)pUABase)->GetValue();
		OpcUa_DataValue* pInternalDataValue = pDataValue->GetInternalDataValue();
		pInternalDataValue->Value.ArrayType = OpcUa_VariantArrayType_Scalar;
		pInternalDataValue->Value.Value.UInt32 = pServerDiagnosticsSummaryDataType->PublishingIntervalCount;
		pInternalDataValue->Value.Datatype = OpcUaType_UInt32;
		// 
		pInternalDataValue->StatusCode = OpcUa_Good;
		pInternalDataValue->ServerTimestamp = OpcUa_DateTime_UtcNow();
		pInternalDataValue->ServerPicoseconds = 0;
		pInternalDataValue->SourceTimestamp = OpcUa_DateTime_UtcNow();
		pInternalDataValue->SourcePicoseconds = 0;
	}
	// CurrentSubscriptionCount 2285
	aNodeId.Identifier.Numeric = 2285;
	uStatus = GetNodeIdFromFastAccessList(aNodeId, &pUABase);
	if (uStatus == OpcUa_Good)
	{
		pDataValue = ((CUAVariable*)pUABase)->GetValue();
		OpcUa_DataValue* pInternalDataValue = pDataValue->GetInternalDataValue();
		pInternalDataValue->Value.ArrayType = OpcUa_VariantArrayType_Scalar;
		pInternalDataValue->Value.Value.UInt32 = pServerDiagnosticsSummaryDataType->CurrentSubscriptionCount;
		pInternalDataValue->Value.Datatype = OpcUaType_UInt32;
		// 
		pInternalDataValue->StatusCode = OpcUa_Good;
		pInternalDataValue->ServerTimestamp = OpcUa_DateTime_UtcNow();
		pInternalDataValue->ServerPicoseconds = 0;
		pInternalDataValue->SourceTimestamp = OpcUa_DateTime_UtcNow();
		pInternalDataValue->SourcePicoseconds = 0;
	}
	// CumulatedSubscriptionCount 2286
	aNodeId.Identifier.Numeric = 2286;
	uStatus = GetNodeIdFromFastAccessList(aNodeId, &pUABase);
	if (uStatus == OpcUa_Good)
	{
		pDataValue = ((CUAVariable*)pUABase)->GetValue();
		OpcUa_DataValue* pInternalDataValue = pDataValue->GetInternalDataValue();
		pInternalDataValue->Value.ArrayType = OpcUa_VariantArrayType_Scalar;
		pInternalDataValue->Value.Value.UInt32 = pServerDiagnosticsSummaryDataType->CumulatedSubscriptionCount;
		pInternalDataValue->Value.Datatype = OpcUaType_UInt32;
		// 
		pInternalDataValue->StatusCode = OpcUa_Good;
		pInternalDataValue->ServerTimestamp = OpcUa_DateTime_UtcNow();
		pInternalDataValue->ServerPicoseconds = 0;
		pInternalDataValue->SourceTimestamp = OpcUa_DateTime_UtcNow();
		pInternalDataValue->SourcePicoseconds = 0;
	}
	// SecurityRejectedRequestsCount 2287
	aNodeId.Identifier.Numeric = 2287;
	uStatus = GetNodeIdFromFastAccessList(aNodeId, &pUABase);
	if (uStatus == OpcUa_Good)
	{
		pDataValue = ((CUAVariable*)pUABase)->GetValue();
		OpcUa_DataValue* pInternalDataValue = pDataValue->GetInternalDataValue();
		pInternalDataValue->Value.ArrayType = OpcUa_VariantArrayType_Scalar;
		pInternalDataValue->Value.Value.UInt32 = pServerDiagnosticsSummaryDataType->SecurityRejectedRequestsCount;
		pInternalDataValue->Value.Datatype = OpcUaType_UInt32;
		// 
		pInternalDataValue->StatusCode = OpcUa_Good;
		pInternalDataValue->ServerTimestamp = OpcUa_DateTime_UtcNow();
		pInternalDataValue->ServerPicoseconds = 0;
		pInternalDataValue->SourceTimestamp = OpcUa_DateTime_UtcNow();
		pInternalDataValue->SourcePicoseconds = 0;
	}
	// RejectedRequestsCount 2288
	aNodeId.Identifier.Numeric = 2288;
	uStatus = GetNodeIdFromFastAccessList(aNodeId, &pUABase);
	if (uStatus == OpcUa_Good)
	{
		pDataValue = ((CUAVariable*)pUABase)->GetValue();
		OpcUa_DataValue* pInternalDataValue = pDataValue->GetInternalDataValue();
		pInternalDataValue->Value.ArrayType = OpcUa_VariantArrayType_Scalar;
		pInternalDataValue->Value.Value.UInt32 = pServerDiagnosticsSummaryDataType->RejectedRequestsCount;
		pInternalDataValue->Value.Datatype = OpcUaType_UInt32;
		// 
		pInternalDataValue->StatusCode = OpcUa_Good;
		pInternalDataValue->ServerTimestamp = OpcUa_DateTime_UtcNow();
		pInternalDataValue->ServerPicoseconds = 0;
		pInternalDataValue->SourceTimestamp = OpcUa_DateTime_UtcNow();
		pInternalDataValue->SourcePicoseconds = 0;
	}
	OpcUa_NodeId_Clear(&aNodeId);
	return uStatus;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Add a new CSessionDiagnosticsDataType (pSessionDiagnosticsDataType) in the m_SessionDiagnostics (CSessionDiagnosticsDataTypeList).
/// 			Then call  UpdateSessionDiagnosticsArray to update the content of the nodeid i=3707 which contains 
/// 			the extensionObject of an array of OpcUa_SessionDiagnosticsDataType</summary>
///
/// <remarks>	Michel, 23/05/2016. </remarks>
///
/// <param name="pSessionDiagnosticsDataType">	[in,out] If non-null, type of the session
/// 											diagnostics data.
/// </param>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode CUAInformationModel::AddInSessionDiagnosticsArray(CSessionDiagnosticsDataType* pSessionDiagnosticsDataType) // 3707
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	//return OpcUa_Good;
	if (!pSessionDiagnosticsDataType)
		uStatus=OpcUa_BadInvalidArgument;
	else
	{
		// 3707
		OpcUa_NodeId aNodeId;
		OpcUa_NodeId_Initialize(&aNodeId);
		aNodeId.IdentifierType=OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex=0;
		aNodeId.Identifier.Numeric=3707;
		OpcUa_Mutex_Lock(m_SessionDiagnosticsMutex);
		m_SessionDiagnosticList.push_back(pSessionDiagnosticsDataType);
		uStatus=UpdateSessionDiagnosticsArray(aNodeId);
		if (uStatus != OpcUa_Good)
			RemoveInSessionDiagnosticsArray(pSessionDiagnosticsDataType);
		OpcUa_Mutex_Unlock(m_SessionDiagnosticsMutex);
		OpcUa_NodeId_Clear(&aNodeId);
		// Create a new node for the newly created session
		// This node will be a CUAObject with the browseName of the sessionName
		OpcUa_String sessionName=pSessionDiagnosticsDataType->GetSessionName();
		AddSessionNameInAddressSpace(sessionName);
	}
	return uStatus;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Add a new UAObject in the addressSpace. This UAObject is a child of 3706 SessionsDiagnosticsSummaryType
/// 			See 6.3.4 OPC UA part 5 for more detail. </summary>
///
/// <remarks>	Michel, 27/05/2016. </remarks>
///
/// <param name="sessionName">	Name of the session. </param>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode CUAInformationModel::AddSessionNameInAddressSpace(OpcUa_String sessionName)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_Mutex_Lock(m_ServerCacheMutex);
	OpcUa_QualifiedName aName;
	OpcUa_LocalizedText aLocalizedText;
	OpcUa_NodeId aNodeId;
	if (OpcUa_String_StrLen(&sessionName) > 0)
	{
		// Ils seront tous fils de 
		CUAObject* pUAObject = new CUAObject();
		CUAReferenceList* pReferences = OpcUa_Null;
		// BrowseName
		OpcUa_QualifiedName_Initialize(&aName);
		aName.NamespaceIndex = 1;
		OpcUa_String_CopyTo(&sessionName, &(aName.Name));
		pUAObject->SetBrowseName(&aName);
		OpcUa_QualifiedName_Clear(&aName);
		// Description
		OpcUa_LocalizedText_Initialize(&aLocalizedText);
		OpcUa_String_AttachCopy(&(aLocalizedText.Locale), "en-us");
		OpcUa_String_CopyTo(&sessionName, &(aLocalizedText.Text));
		pUAObject->SetDescription(aLocalizedText);
		OpcUa_LocalizedText_Clear(&aLocalizedText);
		// DisplayName
		OpcUa_LocalizedText_Initialize(&aLocalizedText);
		OpcUa_String_AttachCopy(&(aLocalizedText.Locale), "en-us");
		OpcUa_String_CopyTo(&sessionName, &(aLocalizedText.Text));
		pUAObject->SetDisplayName(&aLocalizedText);
		OpcUa_LocalizedText_Clear(&aLocalizedText);
		// NodeClass
		pUAObject->SetNodeClass(OpcUa_NodeClass_Object);
		// NodeId (ns=1;s=sessionName)
		OpcUa_NodeId_Initialize(&aNodeId);
		OpcUa_String_CopyTo(&sessionName, &(aNodeId.Identifier.String));
		aNodeId.IdentifierType = OpcUa_IdentifierType_String;
		aNodeId.NamespaceIndex = 1;
		pUAObject->SetNodeId(aNodeId);

		// TypeDefinition
		OpcUa_NodeId_Clear(&aNodeId);
		aNodeId.Identifier.Numeric = 58; // BaseObjectType is the parent of this new Node
		pUAObject->SetTypeDefinition(&aNodeId);
		// WriteMask
		pUAObject->SetWriteMask(OpcUa_AccessLevels_CurrentRead);
		// UserWriteMask
		pUAObject->SetUserWriteMask(OpcUa_AccessLevels_CurrentRead);
		// References
		pReferences = pUAObject->GetReferenceNodeList();
		// HasTypeDefinition
		CUAReference* pReference = new CUAReference();
		pReference->SetInverse(OpcUa_True);
		aNodeId.Identifier.Numeric = OpcUaId_HasTypeDefinition;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 0;
		pReference->SetReferenceTypeId(&aNodeId);
		OpcUa_NodeId_Clear(&aNodeId);
		aNodeId = pUAObject->GetTypeDefinition();
		pReference->SetTargetId(&aNodeId);
		pReferences->push_back(pReference);
		// Get 3706 the purpose is to add the new UAObject in 3706
		// So we create a forward reference to the new UAObject
		CUAObject* pUAObject3706 = OpcUa_Null;
		OpcUa_NodeId_Clear(&aNodeId);
		aNodeId.Identifier.Numeric = 3706;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 0;
		if (GetNodeIdFromObjectList(aNodeId, &pUAObject3706) == OpcUa_Good)
		{
			pReference = new CUAReference();
			pReference->SetInverse(OpcUa_False);
			OpcUa_NodeId_Clear(&aNodeId);
			aNodeId.Identifier.Numeric = OpcUaId_HasComponent;
			aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
			aNodeId.NamespaceIndex = 0;
			pReference->SetReferenceTypeId(&aNodeId);
			CUAReferenceList* pReferences3706 = pUAObject3706->GetReferenceNodeList();
			pReference->SetTargetId(pUAObject->GetNodeId());
			//// Add in the AddressSpace
			PopulateInformationModel(pUAObject);
			pReferences3706->push_back(pReference);
		}
		else
		{
			// cleanup ressources
			delete pUAObject;
			delete pReference;
		}
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	OpcUa_Mutex_Unlock(m_ServerCacheMutex);
	return uStatus;
}
OpcUa_StatusCode CUAInformationModel::RemoveSessionNameInAddressSpace(OpcUa_String sessionName)
{
	OpcUa_StatusCode uStatus=OpcUa_BadNotFound;
	OpcUa_Mutex_Lock(m_ServerCacheMutex);
	OpcUa_NodeId aNodeId;
	CUAObjectList* pUAObjectList=GetObjectList();
	CUAObject* pUAObject = OpcUa_Null;
	// NodeId (ns=1;s=sessionName)
	OpcUa_NodeId_Initialize(&aNodeId);
	OpcUa_String_CopyTo(&sessionName, &(aNodeId.Identifier.String));
	aNodeId.IdentifierType = OpcUa_IdentifierType_String;
	aNodeId.NamespaceIndex = 1;

	CUAObjectList::iterator it = pUAObjectList->find(&aNodeId);
	if (it != pUAObjectList->end())
	{
		pUAObject = it->second;
		delete pUAObject;
		pUAObjectList->erase(it);
		uStatus = OpcUa_Good;
	}
	OpcUa_NodeId_Clear(&aNodeId);
	OpcUa_Mutex_Unlock(m_ServerCacheMutex);
	return uStatus;
}
///-------------------------------------------------------------------------------------------------
/// <summary>Remove the existing CSessionDiagnosticsDataType* (pSessionDiagnosticsDataType) from m_SessionDiagnostics (CSessionDiagnosticsDataTypeList).
/// 		Then call  UpdateSessionDiagnosticsArray to update the content of the nodeid i=3707 which contains
/// 		the extensionObject of an array of OpcUa_SessionDiagnosticsDataType
/// </summary>
///
/// <remarks>	Michel, 23/05/2016. </remarks>
///
/// <param name="pSessionDiagnosticsDataType">	[in,out] If non-null, type of the session
/// 											diagnostics data.
/// </param>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode CUAInformationModel::RemoveInSessionDiagnosticsArray(CSessionDiagnosticsDataType* pSessionDiagnosticsDataType)
{
	OpcUa_StatusCode uStatus=OpcUa_BadNotFound;
	
	if (pSessionDiagnosticsDataType)
	{
		// First remove from the addressSpace
		OpcUa_String sessionName = pSessionDiagnosticsDataType->GetSessionName();
		RemoveSessionNameInAddressSpace(sessionName);
		// 
		OpcUa_Mutex_Lock(m_SessionDiagnosticsMutex);
		OpcUa_NodeId aNodeId1=pSessionDiagnosticsDataType->GetSessionId();
		if (!OpcUa_NodeId_IsNull(&aNodeId1))
		{
			CSessionDiagnosticsDataTypeList::iterator myIterator;
			for (myIterator = m_SessionDiagnosticList.begin(); myIterator != m_SessionDiagnosticList.end(); myIterator++)
			{
				CSessionDiagnosticsDataType* pLocalSessionDiagnosticsDataType = *myIterator;
				OpcUa_NodeId aNodeId2 = pLocalSessionDiagnosticsDataType->GetSessionId();
				if (!OpcUa_NodeId_IsNull(&aNodeId2))
				{
					if (Utils::IsEqual(&aNodeId1, &aNodeId2))
					{
						// la ressource OpcUa_SessionDiagnosticsDataType sera liberée pendant lors de la destruction du CSessionServer
						//delete pLocalSessionDiagnosticsDataType;
						//pLocalSessionDiagnosticsDataType = OpcUa_Null;
						m_SessionDiagnosticList.erase(myIterator);
						uStatus = OpcUa_Good;
						break;
					}
				}
			}
			if (uStatus == OpcUa_Good)
			{
				// i=3707 (SessionDiagnosticsArray)
				OpcUa_NodeId aNodeId;
				OpcUa_NodeId_Initialize(&aNodeId);
				aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
				aNodeId.NamespaceIndex = 0;
				aNodeId.Identifier.Numeric = 3707; // SessionDiagnosticsArray
				uStatus = UpdateSessionDiagnosticsArray(aNodeId);
				OpcUa_NodeId_Clear(&aNodeId);
			}
		}
		else
			uStatus = OpcUa_BadNothingToDo;
		OpcUa_Mutex_Unlock(m_SessionDiagnosticsMutex);
	}
	else
		uStatus=OpcUa_BadInvalidArgument;
	return uStatus;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Updates the subscription diagnostics array. </summary>
///
/// <remarks>	Michel, 23/05/2016. </remarks>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode CUAInformationModel::UpdateSubscriptionDiagnosticsArray()
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	
	OpcUa_NodeId aNodeId;
	OpcUa_NodeId_Initialize(&aNodeId);
	aNodeId.IdentifierType=OpcUa_IdentifierType_Numeric;
	aNodeId.NamespaceIndex=0;
	aNodeId.Identifier.Numeric=2290; // SubscriptionDiagnosticsArray
	CUAVariable* pUAVariable=OpcUa_Null;
	uStatus=GetNodeIdFromVariableList(aNodeId,&pUAVariable);
	if (uStatus == OpcUa_Good)
	{
		// recupération du variant qui contient la serie en cours
		OpcUa_EncodeableType* pEncodeableType=OpcUa_Null;
		uStatus = g_pTheApplication->LookupEncodeableType(pUAVariable->GetDataType().Identifier.Numeric, &pEncodeableType);
		if (uStatus == OpcUa_Good)
		{
			if (pEncodeableType)
			{
				CDataValue* pDataValue = pUAVariable->GetValue();
				if (pDataValue)
				{
					// First we will empty the content of the OpcUa_Variant
					//OpcUa_Variant varCurrentVal = pDataValue->GetValue();
					OpcUa_DataValue* pInternalDataValue=pDataValue->GetInternalDataValue();
					if (pInternalDataValue->Value.Value.Array.Length > 0)
					{
						//OpcUa_Variant_Clear(&(pInternalDataValue->Value));
						// The OpcUa_Variant was not empty
						for (OpcUa_Int32 ii = 0; ii < pInternalDataValue->Value.Value.Array.Length; ii++)
						{
							OpcUa_ExtensionObject_Clear(&(pInternalDataValue->Value.Value.Array.Value.ExtensionObjectArray[ii]));
						}
						OpcUa_Free(pInternalDataValue->Value.Value.Array.Value.ExtensionObjectArray);
						pInternalDataValue->Value.Value.Array.Value.ExtensionObjectArray = OpcUa_Null;
						pInternalDataValue->Value.Value.Array.Length = 0;
					}
					//pDataValue->SetValue(varCurrentVal);
					// allocation de iSize extensionObject
					int iSize = m_SubscriptionDiagnosticList.size();
					OpcUa_Variant varNewVal;// = pDataValue->GetValue();
					OpcUa_Variant_Initialize(&varNewVal);
					varNewVal.Datatype = OpcUaType_ExtensionObject;
					varNewVal.ArrayType = OpcUa_VariantArrayType_Array;

					if (iSize > 0)
					{
						varNewVal.Value.Array.Value.ExtensionObjectArray = (OpcUa_ExtensionObject*)OpcUa_Alloc(iSize*sizeof(OpcUa_ExtensionObject));
						varNewVal.Value.Array.Length = iSize;
						for (OpcUa_UInt32 ii = 0; ii < m_SubscriptionDiagnosticList.size(); ii++)
						{
							OpcUa_SubscriptionDiagnosticsDataType* pCurrentSubscriptionDiagnosticsDataType = m_SubscriptionDiagnosticList.at(ii)->GetInternalSubscriptionDiagnosticsDataType();
							OpcUa_ExtensionObject_Initialize(&(varNewVal.Value.Array.Value.ExtensionObjectArray[ii]));
							varNewVal.Value.Array.Value.ExtensionObjectArray[ii].BodySize = 0;
							varNewVal.Value.Array.Value.ExtensionObjectArray[ii].Encoding = OpcUa_ExtensionObjectEncoding_EncodeableObject;
							OpcUa_ExpandedNodeId_Initialize(&(varNewVal.Value.Array.Value.ExtensionObjectArray[ii].TypeId));
							varNewVal.Value.Array.Value.ExtensionObjectArray[ii].TypeId.ServerIndex = 0;
							OpcUa_String_Initialize(&(varNewVal.Value.Array.Value.ExtensionObjectArray[ii].TypeId.NamespaceUri));
							varNewVal.Value.Array.Value.ExtensionObjectArray[ii].TypeId.NodeId.Identifier.Numeric = pEncodeableType->BinaryEncodingTypeId; //OpcUaId_SubscriptionDiagnosticsArrayType; 
							varNewVal.Value.Array.Value.ExtensionObjectArray[ii].TypeId.NodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
							varNewVal.Value.Array.Value.ExtensionObjectArray[ii].TypeId.NodeId.NamespaceIndex = 0; 
							varNewVal.Value.Array.Value.ExtensionObjectArray[ii].Body.EncodeableObject.Type = pEncodeableType;// Utils::Copy(pEncodeableType);
							varNewVal.Value.Array.Value.ExtensionObjectArray[ii].Body.EncodeableObject.Object =  Utils::Copy(pCurrentSubscriptionDiagnosticsDataType); // pCurrentSubscriptionDiagnosticsDataType; // modif mai 2015
						}
					}
					else
					{
						varNewVal.Value.Array.Length = iSize;
						varNewVal.Value.Array.Value.ExtensionObjectArray = OpcUa_Null;
					}
					// transfert dans le CDataValue
					pDataValue->SetValue(varNewVal);
					//OpcUa_Variant_Clear(&varNewVal); 
					pDataValue->SetStatusCode(OpcUa_Good);
				}
				//OpcUa_Free(pEncodeableType);
			}
		}
	}
	else
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "i=2290 is not define in your AddressSpace. This is a critical configuration error. ServerDiagnostics cannot be handled properly\n");
	return uStatus;
}
OpcUa_StatusCode CUAInformationModel::AddInSubscriptionDiagnosticsArray(CSubscriptionDiagnosticsDataType* pSubscriptionDiagnosticsDataType)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_Mutex_Lock(m_SubscriptionDiagnosticsMutex);
	if (pSubscriptionDiagnosticsDataType)
	{
		if (pSubscriptionDiagnosticsDataType->GetInternalSubscriptionDiagnosticsDataType())
		{
			m_SubscriptionDiagnosticList.push_back(pSubscriptionDiagnosticsDataType);
			uStatus=UpdateSubscriptionDiagnosticsArray();
		}
	}
	else
		uStatus=OpcUa_BadInvalidArgument;
	OpcUa_Mutex_Unlock(m_SubscriptionDiagnosticsMutex);
	// Populate the addressSpace
	AddSubscriptionDiagnosticsInAddressSpace(pSubscriptionDiagnosticsDataType);
	return uStatus;
}
OpcUa_StatusCode CUAInformationModel::RemoveInSubscriptionDiagnosticsArray(CSubscriptionDiagnosticsDataType* pSubscriptionDiagnosticsDataType)
{
	OpcUa_StatusCode uStatus=OpcUa_BadNotFound;

	OpcUa_Mutex_Lock(m_SubscriptionDiagnosticsMutex);
	if (pSubscriptionDiagnosticsDataType)
	{
		if (pSubscriptionDiagnosticsDataType->GetInternalSubscriptionDiagnosticsDataType())
		{
			CSubscriptionDiagnosticsDataTypeList::iterator myIterator;
			for (myIterator=m_SubscriptionDiagnosticList.begin();myIterator!=m_SubscriptionDiagnosticList.end();myIterator++)
			{
				CSubscriptionDiagnosticsDataType* pTmpSubscriptionDiagnosticsDataType = *myIterator;
				if (pSubscriptionDiagnosticsDataType == pTmpSubscriptionDiagnosticsDataType)
				{
					m_SubscriptionDiagnosticList.erase(myIterator);
					uStatus=OpcUa_Good;
					break;
				}
			}
			if (uStatus==OpcUa_Good)
				uStatus=UpdateSubscriptionDiagnosticsArray();
		}
		else
			uStatus=OpcUa_BadInvalidArgument;
	}
	OpcUa_Mutex_Unlock(m_SubscriptionDiagnosticsMutex);
	RemoveSubscriptionInAddressSpace(pSubscriptionDiagnosticsDataType);
	return uStatus;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Removes all subscription diagnostics array. </summary>
///
/// <remarks>	Michel, 24/05/2016. </remarks>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode OpenOpcUa::UAAddressSpace::CUAInformationModel::RemoveAllSubscriptionDiagnosticsArray(void)
{
	OpcUa_StatusCode uStatus=OpcUa_BadNotFound;
	// Scan all existing subscription and remove in the subscription diagnostics node
	OpcUa_Mutex_Lock(m_SubscriptionDiagnosticsMutex);
	CSubscriptionDiagnosticsDataTypeList::iterator myIterator;
	for (myIterator = m_SubscriptionDiagnosticList.begin(); myIterator != m_SubscriptionDiagnosticList.end(); myIterator++)
	{
		CSubscriptionDiagnosticsDataType* pTmpSubscriptionDiagnosticsDataType = *myIterator;
		delete pTmpSubscriptionDiagnosticsDataType;
		pTmpSubscriptionDiagnosticsDataType = OpcUa_Null;
	}
	m_SubscriptionDiagnosticList.clear();
	uStatus = UpdateSubscriptionDiagnosticsArray();
	OpcUa_Mutex_Unlock(m_SubscriptionDiagnosticsMutex);
	return uStatus;
}
OpcUa_StatusCode CUAInformationModel::AddInSessionSecurityDiagnosticsArray(CSessionSecurityDiagnosticsDataType* pSessionSecurityDiagnostics) // 3708
{
	OpcUa_StatusCode uStatus=OpcUa_Good;

	if (!pSessionSecurityDiagnostics)
		uStatus=OpcUa_BadInvalidArgument;
	else
	{
		OpcUa_Mutex_Lock(m_SessionSecurityDiagnosticsMutex);
		m_SessionSecurityDiagnosticList.push_back(pSessionSecurityDiagnostics);
		//  2028, 3113, 3130, 3708
		OpcUa_NodeId aNodeId;
		OpcUa_NodeId_Initialize(&aNodeId);
		aNodeId.IdentifierType=OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex=0;
		aNodeId.Identifier.Numeric=3708;
		uStatus=UpdateSessionSecurityDiagnosticsArray(aNodeId);
		if (uStatus != OpcUa_Good)
		{
			OpcUa_Mutex_Unlock(m_SessionSecurityDiagnosticsMutex);
			RemoveInSessionSecurityDiagnosticsArray(pSessionSecurityDiagnostics);
			OpcUa_Mutex_Lock(m_SessionSecurityDiagnosticsMutex);
		}
		OpcUa_Mutex_Unlock(m_SessionSecurityDiagnosticsMutex);
		OpcUa_NodeId_Clear(&aNodeId);
	}
	return uStatus;
}

// supprime un OpcUa_SessionSecurityDiagnosticsDataType (pSessionSecurityDiagnosticsDataType) au sein du tableau qui les stockebleau
// ensuite on met a jour le NodeId qui expose ce tableau dans l'adressSpace
OpcUa_StatusCode CUAInformationModel::RemoveInSessionSecurityDiagnosticsArray(CSessionSecurityDiagnosticsDataType* pSessionSecurityDiagnosticsDataType)
{
	OpcUa_StatusCode uStatus=OpcUa_BadNotFound;
	//return OpcUa_Good;
	// 
	if (pSessionSecurityDiagnosticsDataType)
	{
		OpcUa_NodeId aNodeId1=pSessionSecurityDiagnosticsDataType->GetSessionId();
		OpcUa_Mutex_Lock(m_SessionSecurityDiagnosticsMutex);
		CSessionSecurityDiagnosticsDataTypeList::iterator myIterator;
		for (myIterator=m_SessionSecurityDiagnosticList.begin();myIterator!=m_SessionSecurityDiagnosticList.end();myIterator++)
		{
			CSessionSecurityDiagnosticsDataType* pSessionSecurityDiagnostics=*myIterator;
			if (pSessionSecurityDiagnostics)
			{
				OpcUa_NodeId aNodeId2=pSessionSecurityDiagnostics->GetSessionId();
				if (Utils::IsEqual(&aNodeId1,&aNodeId2) )
				{
					// la ressource OpcUa_SessionSecurityDiagnosticsDataType sera liberée pendant lors de la destruction du CSessionServer
					m_SessionSecurityDiagnosticList.erase(myIterator);
					uStatus=OpcUa_Good;
					break;
				}
			}
		}
		if (uStatus==OpcUa_Good)
		{
			OpcUa_NodeId aNodeId;
			OpcUa_NodeId_Initialize(&aNodeId);
			aNodeId.IdentifierType=OpcUa_IdentifierType_Numeric;
			aNodeId.NamespaceIndex=0;
			//aNodeId.Identifier.Numeric=2028;
			//uStatus=UpdateSessionSecurityDiagnosticsArray(aNodeId);
			//aNodeId.Identifier.Numeric=3113;
			//uStatus=UpdateSessionSecurityDiagnosticsArray(aNodeId);
			//aNodeId.Identifier.Numeric=3130;
			//uStatus=UpdateSessionSecurityDiagnosticsArray(aNodeId);
			aNodeId.Identifier.Numeric=3708;
			uStatus=UpdateSessionSecurityDiagnosticsArray(aNodeId);
		}
		OpcUa_Mutex_Unlock(m_SessionSecurityDiagnosticsMutex);
	}
	return uStatus;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Updates the session diagnostics array. </summary>
///
/// <remarks>	Michel, 23/05/2016. </remarks>
///
/// <param name="aNodeId">	a node identifier. </param>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode CUAInformationModel::UpdateSessionDiagnosticsArray(OpcUa_NodeId aNodeId)// 3707
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	//

	CUAVariable* pUAVariable = OpcUa_Null;
	// recupération de l'UAVariable associé au paramètre aNodeId dans la BD du serveur
	uStatus = GetNodeIdFromVariableList(aNodeId, &pUAVariable);
	if (uStatus == OpcUa_Good)
	{
		// recuperation de la serie actuellement stockée
		CDataValue* pDataValue = pUAVariable->GetValue();

		OpcUa_Int32 iSessionDiagnosticsSize = m_SessionDiagnosticList.size();
		OpcUa_EncodeableType* pEncodeableType = OpcUa_Null;
		uStatus = g_pTheApplication->LookupEncodeableType(pUAVariable->GetDataType().Identifier.Numeric, &pEncodeableType);
		if (uStatus == OpcUa_Good)
		{
			if (pEncodeableType)
			{
				// First we will empty the content of the OpcUa_Variant
				//OpcUa_Variant varCurrentVal = pDataValue->GetValue();
				OpcUa_DataValue* pInternalDataValue = pDataValue->GetInternalDataValue();
				if (pInternalDataValue->Value.Value.Array.Length > 0)
				{
					//OpcUa_Variant_Clear(&(pInternalDataValue->Value));
					// The OpcUa_Variant was not empty
					for (OpcUa_Int32 ii = 0; ii < pInternalDataValue->Value.Value.Array.Length; ii++)
					{
						OpcUa_ExtensionObject_Clear(&(pInternalDataValue->Value.Value.Array.Value.ExtensionObjectArray[ii]));
					}
					OpcUa_Free(pInternalDataValue->Value.Value.Array.Value.ExtensionObjectArray);
					pInternalDataValue->Value.Value.Array.Value.ExtensionObjectArray = OpcUa_Null;
					pInternalDataValue->Value.Value.Array.Length = 0;
				}
				//pDataValue->SetValue(varCurrentVal);
				// Now let's handling the new content
				if (iSessionDiagnosticsSize > 0)
				{
					OpcUa_Variant varNewVal;
					OpcUa_Variant_Initialize(&varNewVal);
					varNewVal.Datatype = OpcUaType_ExtensionObject;
					varNewVal.ArrayType = OpcUa_VariantArrayType_Array;
					if (iSessionDiagnosticsSize > 0)
						varNewVal.Value.Array.Value.ExtensionObjectArray = (OpcUa_ExtensionObject*)OpcUa_Alloc(iSessionDiagnosticsSize*sizeof(OpcUa_ExtensionObject));
					varNewVal.Value.Array.Length = iSessionDiagnosticsSize;
					for (OpcUa_UInt16 iii = 0; iii < iSessionDiagnosticsSize; iii++)
					{
						CSessionDiagnosticsDataType* pSessionDiagnosticsDataType = m_SessionDiagnosticList.at(iii);
						if (pSessionDiagnosticsDataType)
						{
							OpcUa_ExtensionObject_Initialize(&(varNewVal.Value.Array.Value.ExtensionObjectArray[iii]));
							varNewVal.Value.Array.Value.ExtensionObjectArray[iii].BodySize = 0;
							varNewVal.Value.Array.Value.ExtensionObjectArray[iii].Encoding = OpcUa_ExtensionObjectEncoding_EncodeableObject;
							OpcUa_ExpandedNodeId_Initialize(&(varNewVal.Value.Array.Value.ExtensionObjectArray[iii].TypeId));
							varNewVal.Value.Array.Value.ExtensionObjectArray[iii].TypeId.NodeId.Identifier.Numeric = pEncodeableType->BinaryEncodingTypeId; //  OpcUaId_SessionDiagnosticsArrayType;
							varNewVal.Value.Array.Value.ExtensionObjectArray[iii].TypeId.NodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
							varNewVal.Value.Array.Value.ExtensionObjectArray[iii].TypeId.NodeId.NamespaceIndex = 0;;
							varNewVal.Value.Array.Value.ExtensionObjectArray[iii].TypeId.ServerIndex = 0;
							varNewVal.Value.Array.Value.ExtensionObjectArray[iii].Body.EncodeableObject.Type = pEncodeableType; // Utils::Copy(pEncodeableType);
							//varNewVal.Value.Array.Value.ExtensionObjectArray[iii].Body.EncodeableObject.Object = OpcUa_Alloc(pEncodeableType->AllocationSize);
							varNewVal.Value.Array.Value.ExtensionObjectArray[iii].Body.EncodeableObject.Object = Utils::Copy(pSessionDiagnosticsDataType->GetInternalPtr());
						}
					}
					pDataValue->SetValue(varNewVal);
				}
				//OpcUa_Variant_Clear(&varNewVal);
				pDataValue->SetStatusCode(OpcUa_Good);
				//OpcUa_Free(pEncodeableType);
			}
		}
		else
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "CUAInformationModel::UpdateSessionDiagnosticsArray>LookupEncodeableType failed uStatus=0x%05x\n", uStatus);
	}
	else
		uStatus = OpcUa_Bad;
	return uStatus;
}
// le paramètre passé à la méthode contient le nodeId qui sera mise à jour.
// en effet l'addressSpace d'un serveur UA contient plusieurs Node de type i=868 (SessionSecurityDiagnosticsDataType)
// 2031,  2028, 2243, 2244, 3113, 3130, 3708, 12142.
// L'objet de cette méthode est de mettre a jour le contenu des Node dont le type de donnée est SessionSecurityDiagnosticsArray
// pour cela la methode vide la contenu de la variable et la reconstruit
OpcUa_StatusCode CUAInformationModel::UpdateSessionSecurityDiagnosticsArray(OpcUa_NodeId aNodeId)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	//

	CUAVariable* pUAVariable = OpcUa_Null;
	// recupération de l'UAVariable associé au paramètre aNodeId dans la BD du serveur
	uStatus = GetNodeIdFromVariableList(aNodeId, &pUAVariable);
	if (uStatus == OpcUa_Good)
	{
		//OpcUa_Variant aVariant;
		// recuperation de la serie actuellement stockée
		CDataValue* pDataValue = pUAVariable->GetValue();
		// recupération du nombre d'element dans le vecteur m_SessionSecurityDiagnostics
		OpcUa_Int32 iSessionSecurityDiagnosticsSize = m_SessionSecurityDiagnosticList.size();
		// recherche de l'encodeableType associé au DataType de cette UAVariable
		OpcUa_EncodeableType* pEncodeableType = OpcUa_Null;
		uStatus = g_pTheApplication->LookupEncodeableType(pUAVariable->GetDataType().Identifier.Numeric, &pEncodeableType);
		if (uStatus == OpcUa_Good)
		{
			if (pEncodeableType)
			{
				// First we will empty the content of the OpcUa_Variant
				OpcUa_DataValue* pInternalDataValue = pDataValue->GetInternalDataValue();
				if (pInternalDataValue->Value.Value.Array.Length > 0)
				{
					// The OpcUa_Variant was not empty
					for (OpcUa_Int32 ii = 0; ii < pInternalDataValue->Value.Value.Array.Length; ii++)
					{
						OpcUa_ExtensionObject_Clear(&(pInternalDataValue->Value.Value.Array.Value.ExtensionObjectArray[ii]));
					}
					OpcUa_Free(pInternalDataValue->Value.Value.Array.Value.ExtensionObjectArray);
					pInternalDataValue->Value.Value.Array.Value.ExtensionObjectArray = OpcUa_Null;
					pInternalDataValue->Value.Value.Array.Length = 0;
					//OpcUa_Variant_Clear(&(pInternalDataValue->Value));
				}
				//pDataValue->SetValue(varCurrentVal);
				// Now let's handling the new content
				////////////////////////////////////////////////////////////////////////////////////////////////
				if (iSessionSecurityDiagnosticsSize)
				{
					OpcUa_Variant varNewVal;
					OpcUa_Variant_Initialize(&varNewVal);
					varNewVal.Datatype = OpcUaType_ExtensionObject;
					varNewVal.ArrayType = OpcUa_VariantArrayType_Array;
					varNewVal.Value.Array.Value.ExtensionObjectArray = (OpcUa_ExtensionObject*)OpcUa_Alloc(iSessionSecurityDiagnosticsSize*sizeof(OpcUa_ExtensionObject));
					varNewVal.Value.Array.Length = iSessionSecurityDiagnosticsSize;
					for (OpcUa_UInt16 iii = 0; iii < iSessionSecurityDiagnosticsSize; iii++)
					{
						CSessionSecurityDiagnosticsDataType* pSessionSecurityDiagnosticsDataType = m_SessionSecurityDiagnosticList.at(iii);
						OpcUa_ExtensionObject_Initialize(&(varNewVal.Value.Array.Value.ExtensionObjectArray[iii]));
						varNewVal.Value.Array.Value.ExtensionObjectArray[iii].BodySize = 0;
						varNewVal.Value.Array.Value.ExtensionObjectArray[iii].Encoding = OpcUa_ExtensionObjectEncoding_EncodeableObject;
						OpcUa_ExpandedNodeId_Initialize(&(varNewVal.Value.Array.Value.ExtensionObjectArray[iii].TypeId));
						varNewVal.Value.Array.Value.ExtensionObjectArray[iii].TypeId.NodeId.Identifier.Numeric = pEncodeableType->BinaryEncodingTypeId;// OpcUaId_SessionSecurityDiagnosticsArrayType; 
						varNewVal.Value.Array.Value.ExtensionObjectArray[iii].TypeId.NodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
						varNewVal.Value.Array.Value.ExtensionObjectArray[iii].TypeId.NodeId.NamespaceIndex = 0;
						varNewVal.Value.Array.Value.ExtensionObjectArray[iii].TypeId.ServerIndex = 0;
						varNewVal.Value.Array.Value.ExtensionObjectArray[iii].Body.EncodeableObject.Type = pEncodeableType;// Utils::Copy(pEncodeableType);
						OpcUa_SessionSecurityDiagnosticsDataType* pInternalSessionSecurityDiagnosticsDataType = pSessionSecurityDiagnosticsDataType->GetInternalPtr();
						if (pInternalSessionSecurityDiagnosticsDataType)
							varNewVal.Value.Array.Value.ExtensionObjectArray[iii].Body.EncodeableObject.Object = Utils::Copy(pInternalSessionSecurityDiagnosticsDataType);
					}
					pDataValue->SetValue(varNewVal);
				}
				//OpcUa_Variant_Clear(&varNewVal);
				pDataValue->SetStatusCode(OpcUa_Good);
				//OpcUa_Free(pEncodeableType); // verify if it not generate a leak
			}
		}
		else
		{
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "CUAInformationModel::UpdateSessionSecurityDiagnosticsArray>LookupEncodeableType failed uStatus=0x%05x\n", uStatus);
			uStatus = OpcUa_BadInvalidArgument;
		}
	}
	else
		uStatus = OpcUa_Bad;
	return uStatus;
}

OpcUa_StatusCode OpenOpcUa::UAAddressSpace::CUAInformationModel::BuildApplicationUri(OpcUa_CharA** szApplicationUri)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	std::string ApplicationUri;
	OpcUa_StringA pDomainName = OpcUa_Null;
	ApplicationUri.empty();
	// look up the domain name for the current machine.
	uStatus = OpcUa_LookupDomainName((OpcUa_StringA)"127.0.0.1", &pDomainName);        
	if (OpcUa_IsBad(uStatus))
	{
		// use the computer name if no domain name.
		OpcUa_CharA sBuffer[MAX_PATH + 1];
		ZeroMemory(sBuffer, MAX_PATH + 1);
		gethostname(sBuffer, MAX_PATH);
		OpcUa_Int32 iSize = OpcUa_StrLenA(sBuffer);
		(*szApplicationUri) = (OpcUa_CharA*)OpcUa_Alloc(iSize + 1);
		if ((*szApplicationUri))
		{
			ZeroMemory(*szApplicationUri, iSize + 1);
			OpcUa_MemCpy(*szApplicationUri, iSize, (void*)sBuffer, iSize);
		}
		else
			uStatus = OpcUa_BadOutOfMemory;
	}
	else
	{
		std::string domainName(pDomainName);
		// copy the domain name.
		OpcUa_Free(pDomainName);
	 
		ApplicationUri = "urn:";
		ApplicationUri += domainName;
		ApplicationUri += ":OpenOpcUa";
		ApplicationUri += ":";
		ApplicationUri += "OpenOpcUaCoreServer";	
		int iSize=ApplicationUri.length();
		(*szApplicationUri)=(OpcUa_CharA*)OpcUa_Alloc(iSize+1);
		ZeroMemory(*szApplicationUri,iSize+1);
		OpcUa_MemCpy(*szApplicationUri,ApplicationUri.length(),(void*)ApplicationUri.c_str(),ApplicationUri.length());			
	}
	return uStatus;
}


// Retourne une reference a partir de son type et du nom de sa cible
// aRefId contient la reference HasComponent, HasChild, Etc
// aTargetName contient le nom de la cible
// la méthode retournera OpcUa_Null si rien n'a eté trouve.
// sinon elle retournera l'OpcUa_ReferenceNode correspondante
CUAReference* CUAInformationModel::GetReferenceTarget(CUABase** pStartingNodeBase,OpcUa_NodeId aRefId,OpcUa_QualifiedName aTargetName)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	CUAReference* pReferenceNode=OpcUa_Null;
	CUAReferenceList* pReferences=(*pStartingNodeBase)->GetReferenceNodeList();
	if (pReferences)
	{
		for (OpcUa_UInt32 i=0;i<pReferences->size();i++)
		{
			CUAReference* pTmpReferenceNode=pReferences->at(i);
			OpcUa_NodeId aNodeId=pTmpReferenceNode->GetReferenceTypeId();
			if (Utils::IsEqual(&aNodeId,&aRefId) )
			{
				// Recherche du nom de la reference cible: pTmpReferenceNode->TargetId
				CUABase* pTmpUABase=NULL; 
				uStatus = GetNodeIdFromDictionnary(pTmpReferenceNode->GetTargetId().NodeId, &pTmpUABase);
				if (uStatus==OpcUa_Good)
				{
					OpcUa_QualifiedName* pQualifiedName1=pTmpUABase->GetBrowseName();
					if (Utils::IsEqual(&aTargetName,pQualifiedName1))
					{
						pReferenceNode=pTmpReferenceNode;
						*pStartingNodeBase=pTmpUABase;
						break;
					}
				}
			}

		}
	}
	return pReferenceNode;
}
/// <summary>
/// Translates the relative path to node identifier.
/// </summary>
/// <param name="pStartingNodeBase">The First node where to start the translation.</param>
/// <param name="aRelativePath">A relative path.</param>
/// <param name="uiSucceeded">Counter of succeeded tranlation.</param>
/// <param name="pExpandedNodeIdList">vector that contains the translated NodeIds.</param>
/// <returns></returns>
OpcUa_StatusCode CUAInformationModel::TranslateRelativePathToNodeId(CUABase* pStartingNodeBase,OpcUa_RelativePath aRelativePath,OpcUa_UInt16* uiSucceeded,std::vector<OpcUa_ExpandedNodeId*>* pExpandedNodeIdList)
{
	OpcUa_StatusCode uStatus=OpcUa_Bad;
	OpcUa_UInt16 uiTmpSucceeded=0;
	if (aRelativePath.NoOfElements>0)
	{
		// Parcours de tous les elements dans aRelativePath
		for (OpcUa_UInt16 ii=(*uiSucceeded); ii<aRelativePath.NoOfElements;ii++)
		{
			CUAReferenceList* pReferences=pStartingNodeBase->GetReferenceNodeList();
			if (pReferences)
			{
				// Parcours de toutes les references de pStartingNodeBase (CUABase*)
				for (OpcUa_UInt32 i=0;i<pReferences->size();i++)
				{
					CUAReference* pTmpReferenceNode=pReferences->at(i);
					OpcUa_NodeId aReferenceTypeId = pTmpReferenceNode->GetReferenceTypeId();
					if (pTmpReferenceNode->IsInverse()==aRelativePath.Elements[ii].IsInverse)
					{
						if (OpcUa_String_IsNull(&(aRelativePath.Elements[ii].TargetName.Name)))
							return  OpcUa_BadBrowseNameInvalid;
						else
						{
							OpcUa_Boolean bValid = OpcUa_False;
							// On verifie que l'on a bien a faire au même type de reference
							// ou qu'il s'agit d'une reference NULL et que les sous-types ne sont pas inclus (CTT)
							if (Utils::IsEqual(&aReferenceTypeId, &(aRelativePath.Elements[ii].ReferenceTypeId))
								|| (Utils::IsNodeIdNull(aRelativePath.Elements[ii].ReferenceTypeId) && (aRelativePath.Elements[ii].IncludeSubtypes == OpcUa_False)))
								bValid = OpcUa_True;
							if ((aRelativePath.Elements[ii].IncludeSubtypes == (OpcUa_Boolean)OpcUa_True)) // Attention il s'agit d'un bricolage. Normalement il faudrait verifier que 
							{
								if (IsInReferenceHierarchy(pTmpReferenceNode->GetReferenceTypeId(), aRelativePath.Elements[ii].ReferenceTypeId))
									bValid = OpcUa_True;
							}
							if (bValid)
							{
								// Recherche du nom de la reference cible: pTmpReferenceNode->TargetId
								CUABase* pTargetUABase = NULL;
								OpcUa_NodeId aTargetNodeId = pTmpReferenceNode->GetTargetId().NodeId;
								uStatus = GetNodeIdFromDictionnary(aTargetNodeId, &pTargetUABase);
								if (uStatus == OpcUa_Good)
								{
									// on filtre les paternités directe
									OpcUa_NodeId* pNodeId1 = pStartingNodeBase->GetNodeId();
									OpcUa_NodeId* pNodeId2 = pTargetUABase->GetNodeId();
									if (!Utils::IsEqual(pNodeId1, pNodeId2))
									{
										// On verifie que la cible est bien la même
										OpcUa_QualifiedName* pTargetUABaseBrowseName = pTargetUABase->GetBrowseName();
										OpcUa_QualifiedName aRelativePathTargetName = aRelativePath.Elements[ii].TargetName;
										if (OpcUa_String_StrLen(&(aRelativePath.Elements[ii].TargetName.Name)) > 0) // test suivant pour err-007.js du CTT
										{
											if (Utils::IsEqual(&(aRelativePathTargetName.Name), &(pTargetUABaseBrowseName->Name)))
											{
												uiTmpSucceeded++;
												if (ii < aRelativePath.NoOfElements - 1)
												{
													uStatus = OpcUa_Good; // on en a trouvé au moins 1
													// Ok donc on l'ajoute...
													// Appel recursif
													(*uiSucceeded) = ii++;
													uStatus = TranslateRelativePathToNodeId(pTargetUABase, aRelativePath, uiSucceeded, pExpandedNodeIdList);
													if (uStatus == OpcUa_Good)
														return uStatus;
												}
												else
												{
													// Transfert du resultat
													OpcUa_ExpandedNodeId* pExpandedNodeId = (OpcUa_ExpandedNodeId*)OpcUa_Alloc(sizeof(OpcUa_ExpandedNodeId));
													OpcUa_ExpandedNodeId_Initialize(pExpandedNodeId);
													OpcUa_NodeId* pNodeId3 = pTargetUABase->GetNodeId();
													OpcUa_NodeId_CopyTo(pNodeId3, &(pExpandedNodeId->NodeId));
													OpcUa_String_AttachCopy(&(pExpandedNodeId->NamespaceUri), "");
													pExpandedNodeId->ServerIndex = 0;//pTmpUABase->GetNodeId().NamespaceIndex;	
													pExpandedNodeIdList->push_back(pExpandedNodeId);
													return OpcUa_Good;
												}
											}
											else
											{
												if (pExpandedNodeIdList->size() > 0)
													uStatus = OpcUa_Good;
												else
													uStatus = OpcUa_BadNoMatch;
											}
										}
										else
										{
											uStatus = OpcUa_BadBrowseNameInvalid;
											if ((aRelativePath.NoOfElements) > ii)	// test pour CTT err-008.js 
												return uStatus;
										}
									}
								}
							}
							else
							{
								uStatus = OpcUa_BadNoMatch;
							}
						}
					}
				}
			}
		}
	}
	else
		uStatus=OpcUa_BadNothingToDo;//OpcUa_BadNoMatch;
	return uStatus;
}

OpcUa_StatusCode CUAInformationModel::IsReferenceTypeIdValid(OpcUa_NodeId aRefTypeId, CUAReferenceType** pUAReferenceType)
{
	OpcUa_StatusCode uStatus=OpcUa_BadReferenceTypeIdInvalid;
	
	if ( (aRefTypeId.IdentifierType==OpcUa_IdentifierType_Numeric) && (aRefTypeId.Identifier.Numeric==0) )
		uStatus=OpcUa_Good;
	else
	{
		uStatus = GetNodeIdFromReferenceTypeList(aRefTypeId, pUAReferenceType);
	}
	return uStatus;
}
// 
// 
// Function name   : CUAInformationModel::BrowseOneNode
// Description     : parcours une node aNodeToBrowse. En fonction des paramètre passé dans l'OpcUa_BrowseDescription.
//					 Le resultat est retournés dans OpcUa_BrowseResult**. Le Continuation point place dans un CContinuationPoint**
// Return type     : OpcUa_StatusCode 
// Argument        : OpcUa_BrowseDescription aNodeToBrowse
// Argument        : OpcUa_UInt32 uiRequestedMaxReferencesPerNode
// Argument        : OpcUa_BrowseResult** pBrowseResult
// Argument        : CContinuationPoint** pContinuationPoint
OpcUa_StatusCode CUAInformationModel::BrowseOneNode(OpcUa_BrowseDescription aNodeToBrowse,
													OpcUa_UInt32 uiRequestedMaxReferencesPerNode,
													OpcUa_BrowseResult** pBrowseResult,
													CContinuationPoint** pContinuationPoint)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	CUABase* pUABase=NULL;
	uStatus = GetNodeIdFromDictionnary(aNodeToBrowse.NodeId, &pUABase);
	if (uStatus==OpcUa_Good)
	{	
		if (pUABase->GetReferenceNodeList()->size() > 0)
		{
			// Parcours des references de ce noeud
			(*pBrowseResult)->References = (OpcUa_ReferenceDescription*)OpcUa_Alloc((pUABase->GetReferenceNodeList()->size())*sizeof(OpcUa_ReferenceDescription));
			//OpcUa_ReferenceDescription_Initialize((*pBrowseResult)->References);
			// extraction de l'index dans le continuationPoint
			OpcUa_Int32 index = 0;
			if (*pContinuationPoint)
				index = (*pContinuationPoint)->GetCurrentIndex();
			OpcUa_UInt32 iRef = 0;
			for (OpcUa_UInt32 ii = index; ii < pUABase->GetReferenceNodeList()->size(); ii++)
			{
				OpcUa_ReferenceDescription_Initialize(&((*pBrowseResult)->References[iRef]));
				CUAReference* pReference = pUABase->GetReferenceNodeList()->at(ii);
				// filtre sur le nombre max de reference par node
				if ((iRef < uiRequestedMaxReferencesPerNode) || (uiRequestedMaxReferencesPerNode == 0))
				{
					// filter sur le sens de la reference
					if (aNodeToBrowse.BrowseDirection <= OpcUa_BrowseDirection_Both)
					{
						if (((aNodeToBrowse.BrowseDirection == OpcUa_BrowseDirection_Both)
							|| (aNodeToBrowse.BrowseDirection == OpcUa_BrowseDirection_Forward))
							&& (!pReference->IsInverse()))
						{
							CUABase* pUABaseTarget = NULL;
							if (GetNodeIdFromDictionnary(pReference->GetTargetId().NodeId, &pUABaseTarget) == OpcUa_Good)
							{
								// filtre sur le NodeClass de la cible
								OpcUa_Boolean bNodeclassOk = OpcUa_False;
								OpcUa_NodeClass ncClass = (OpcUa_NodeClass)aNodeToBrowse.NodeClassMask;
								if (((ncClass&pUABaseTarget->GetNodeClass()) == pUABaseTarget->GetNodeClass())
									|| (OpcUa_NodeClass_Unspecified == aNodeToBrowse.NodeClassMask))
									bNodeclassOk = OpcUa_True;
								// filtre sur l'inclusion des sous-types ou non
								OpcUa_Boolean bReferenceTypeIdOk = OpcUa_False;
								BrowseIsIncludeSubtypes(aNodeToBrowse, pReference->GetReferenceTypeId(), &bReferenceTypeIdOk);

								if ((bReferenceTypeIdOk) && (bNodeclassOk))
								{
									if ((aNodeToBrowse.NodeClassMask&pUABaseTarget->GetNodeClass())
										|| (aNodeToBrowse.NodeClassMask == OpcUa_NodeClass_Unspecified))
									{
										AddReferenceInBrowseResult(aNodeToBrowse, pUABase, pUABaseTarget, pReference, pBrowseResult, &iRef);
									}
								}
							}
							else
							{
								// SourceNodeId
								char* szSourceNodeId = OpcUa_Null;								
								if (Utils::NodeId2String(&(aNodeToBrowse.NodeId), &szSourceNodeId) == OpcUa_Good)
								{
									// NodeId
									char* szTargetNodeId = OpcUa_Null;									
									OpcUa_NodeId aTargetNodeId = pReference->GetTargetId().NodeId;
									if (Utils::NodeId2String(&aTargetNodeId, &szTargetNodeId) == OpcUa_Good)
									{
										// ReferenceType
										char* szReferenceNodeId = OpcUa_Null;										
										OpcUa_NodeId aRefNodeId = pReference->GetReferenceTypeId();
										if (Utils::NodeId2String(&aRefNodeId, &szReferenceNodeId) == OpcUa_Good)
										{
											// Show the error message
											if (pReference->IsInverse())
												OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "configuration file inconsistancy. Cannot find inverse reference %s %s -->%s \n", szReferenceNodeId, szSourceNodeId, szTargetNodeId);
											else
												OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "configuration file inconsistancy. Cannot find forward reference %s %s -->%s \n", szReferenceNodeId, szSourceNodeId, szTargetNodeId);
										}
										else
											OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "NodeId2String failed on ReferenceNodeId\n");
										if (szReferenceNodeId)
											OpcUa_Free(szReferenceNodeId);
									}
									else
										OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "NodeId2String failed on TargetNodeId\n");
									if (szTargetNodeId)
										OpcUa_Free(szTargetNodeId);
								}
								else
									OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "NodeId2String failed on NodeToBrowse\n");
								if (szSourceNodeId)
									OpcUa_Free(szSourceNodeId);
							}
						} // fin du filtre sur le inverse or not
						else
						{
							if (((aNodeToBrowse.BrowseDirection == OpcUa_BrowseDirection_Both)
								|| (aNodeToBrowse.BrowseDirection == OpcUa_BrowseDirection_Inverse))
								&& (pReference->IsInverse()))
							{
								// Il faut repérer la définition du noeud ns:000-id:40 (HasTypeDefinition et chercher la target
								// afin de renseigner le type du noeud
								CUABase* pUABaseTarget = NULL;
								if (GetNodeIdFromDictionnary(pReference->GetTargetId().NodeId, &pUABaseTarget) == OpcUa_Good)
								{
									// filtre sur le NodeClass de la cible
									OpcUa_Boolean bNodeclassOk = OpcUa_False;
									OpcUa_NodeClass ncClass = (OpcUa_NodeClass)aNodeToBrowse.NodeClassMask;
									if (((ncClass&pUABaseTarget->GetNodeClass()) == pUABaseTarget->GetNodeClass())
										|| (ncClass == OpcUa_NodeClass_Unspecified))
										bNodeclassOk = OpcUa_True;
									// filtre sur le ReferenceTypeId
									OpcUa_Boolean bReferenceTypeIdOk = OpcUa_False;
									BrowseIsIncludeSubtypes(aNodeToBrowse, pReference->GetReferenceTypeId(), &bReferenceTypeIdOk);
									if ((bReferenceTypeIdOk) && (bNodeclassOk))
									{
										if ((aNodeToBrowse.NodeClassMask&pUABaseTarget->GetNodeClass())
											|| (aNodeToBrowse.NodeClassMask == OpcUa_NodeClass_Unspecified))
										{
											if (!Utils::IsNodeIdNull(pReference->GetTargetId().NodeId)) // on ne prend pas les NodeNull
												AddReferenceInBrowseResult(aNodeToBrowse, pUABase, pUABaseTarget, pReference, pBrowseResult, &iRef);
										}
									}
								}
								else
								{
									char* szNodeId = OpcUa_Null;
									// NodeId
									OpcUa_NodeId aNodeId = pReference->GetTargetId().NodeId;
									Utils::NodeId2String(&aNodeId, &szNodeId);
									if (szNodeId)
									{
										OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critical Error>configuration file inconsistancy:%s \n", szNodeId);
										free(szNodeId);
									}
								}
							}
						}
					}
					else
					{
						// Traitement du cas ou l'on a reçu une BrowseDirection invalide
						OpcUa_QualifiedName_Initialize(&((*pBrowseResult)->References[iRef].BrowseName));
						// DisplayName
						OpcUa_LocalizedText_Initialize(&((*pBrowseResult)->References[iRef].DisplayName));
						// ReferenceTytpeId
						OpcUa_NodeId_Initialize(&((*pBrowseResult)->References[iRef].ReferenceTypeId));
						OpcUa_NodeId_Initialize(&((*pBrowseResult)->References[iRef].TypeDefinition.NodeId));
						OpcUa_String_Initialize(&((*pBrowseResult)->References[iRef].TypeDefinition.NamespaceUri));
						(*pBrowseResult)->References[iRef].TypeDefinition.ServerIndex = 0;
						// Extra Info
						(*pBrowseResult)->ContinuationPoint.Data = OpcUa_Null;
						(*pBrowseResult)->ContinuationPoint.Length = 0;
						(*pBrowseResult)->StatusCode = OpcUa_BadBrowseDirectionInvalid;
						(*pBrowseResult)->NoOfReferences++;
						iRef++;
					}
				}
				else
				{
					if ((iRef >= uiRequestedMaxReferencesPerNode) && (uiRequestedMaxReferencesPerNode > 0))
					{
						// Fill continuationPoint
						(*pContinuationPoint)->SetCurrentIndex(ii);
						(*pContinuationPoint)->SetBrowseDescription(&aNodeToBrowse);
						(*pContinuationPoint)->SetRequestedMaxReferencesPerNode(uiRequestedMaxReferencesPerNode);
						// mise a jour de la valeur du continuationPoint. Soit la valeur de l'index courant+1
						OpcUa_UInt32 iVal = ii;
						(*pBrowseResult)->ContinuationPoint.Data = (OpcUa_Byte*)OpcUa_Alloc(sizeof(OpcUa_UInt32));
						ZeroMemory((*pBrowseResult)->ContinuationPoint.Data, sizeof(OpcUa_UInt32));
						OpcUa_MemCpy((*pBrowseResult)->ContinuationPoint.Data, sizeof(OpcUa_UInt32), (OpcUa_Byte*)&iVal, sizeof(OpcUa_UInt32));
						(*pBrowseResult)->ContinuationPoint.Length = sizeof(OpcUa_UInt32);
						(*pBrowseResult)->StatusCode = OpcUa_Good;
						break;
					}
				}
			} // fin for references
			(*pBrowseResult)->NoOfReferences = iRef;
		}
	}
	else
	{
		(*pBrowseResult)->StatusCode=OpcUa_BadNodeIdUnknown;
		uStatus=OpcUa_Good;
	}
	return uStatus;
}

// methode permettant de savoir si le client a demandé que les sous-types soit inclus dans le browse
OpcUa_StatusCode CUAInformationModel::BrowseIsIncludeSubtypes(OpcUa_BrowseDescription aNodesToBrowse,OpcUa_NodeId aReferenceTypeId,OpcUa_Boolean *bReferenceTypeIdOk)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	CUAInformationModel* pInformationModel=CServerApplication::m_pTheAddressSpace;
	if (pInformationModel)
	{
		// premier cas ou le client a demandé que les sous-types soit inclus
		if (aNodesToBrowse.IncludeSubtypes)
		{
			if ( (pInformationModel->IsInReferenceHierarchy(aNodesToBrowse.ReferenceTypeId,aReferenceTypeId)==OpcUa_Good)
				|| (aNodesToBrowse.ReferenceTypeId.Identifier.Numeric==0))
				*bReferenceTypeIdOk=OpcUa_True;
		}
		else
		{
			// deuxième cas le client ne veut pas que les sous-types soit inclus
			if (Utils::IsEqual(&(aNodesToBrowse.ReferenceTypeId),&(aReferenceTypeId))
				|| (aNodesToBrowse.ReferenceTypeId.Identifier.Numeric==0))
				*bReferenceTypeIdOk=OpcUa_True;
		}
	}
	else
		uStatus=OpcUa_BadInvalidArgument;
	return uStatus;
}
// Rempli l'OpcUa_BrowseResult* d'indice iRef  
OpcUa_StatusCode CUAInformationModel::AddReferenceInBrowseResult(const OpcUa_BrowseDescription aNodesToBrowse, // in
																const CUABase* pUABase,
																const CUABase* pUABaseTarget, // in
																CUAReference* pReference, //in
																OpcUa_BrowseResult** ppNewBrowseResult,  // in/out
																OpcUa_UInt32 *iRef) // in/out
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	CUAInformationModel* pInformationModel=CServerApplication::m_pTheAddressSpace;
	if (pInformationModel)
	{
		// Preparation du filtre sur les attributs qui seront transmis au client
		OpcUa_Boolean bNoBrowseName=OpcUa_False;
		OpcUa_Boolean bNoTypeDefinition=OpcUa_False;
		OpcUa_Boolean bNoNodeClass=OpcUa_False;
		OpcUa_Boolean bNoReferenceTypeId=OpcUa_False;
		OpcUa_Boolean bNoDisplayName=OpcUa_False;
		OpcUa_Boolean bNoForward=OpcUa_False;
		if ( (aNodesToBrowse.ResultMask&OpcUa_BrowseResultMask_BrowseName)!=OpcUa_BrowseResultMask_BrowseName)
			bNoBrowseName=OpcUa_True;
		if ( (aNodesToBrowse.ResultMask&OpcUa_BrowseResultMask_TypeDefinition)!=OpcUa_BrowseResultMask_TypeDefinition)
			bNoTypeDefinition=OpcUa_True;
		if ( (aNodesToBrowse.ResultMask&OpcUa_BrowseResultMask_NodeClass)!=OpcUa_BrowseResultMask_NodeClass)
			bNoNodeClass=OpcUa_True;
		if ( (aNodesToBrowse.ResultMask&OpcUa_BrowseResultMask_ReferenceTypeId)!=OpcUa_BrowseResultMask_ReferenceTypeId)
			bNoReferenceTypeId=OpcUa_True;
		if ( (aNodesToBrowse.ResultMask&OpcUa_BrowseResultMask_DisplayName)!=OpcUa_BrowseResultMask_DisplayName)
			bNoDisplayName=OpcUa_True;
		if ( (aNodesToBrowse.ResultMask&OpcUa_BrowseResultMask_IsForward)!=OpcUa_BrowseResultMask_IsForward)
			bNoForward=OpcUa_True;
		// BrowseName
		OpcUa_QualifiedName_Initialize(&((*ppNewBrowseResult)->References[*iRef].BrowseName));
		if (!bNoBrowseName)
		{
			OpcUa_QualifiedName* pQName=pUABaseTarget->GetBrowseName();
			if (pQName)
			{
				OpcUa_QualifiedName_Initialize(&((*ppNewBrowseResult)->References[*iRef].BrowseName));
				OpcUa_QualifiedName_CopyTo(pQName, &((*ppNewBrowseResult)->References[*iRef].BrowseName));
			}
		}																							
		// DisplayName
		OpcUa_LocalizedText_Initialize(&((*ppNewBrowseResult)->References[*iRef].DisplayName));
		if (!bNoDisplayName)
		{
			OpcUa_LocalizedText aLocalizedText=pUABaseTarget->GetDisplayName();
			OpcUa_LocalizedText_Initialize(&((*ppNewBrowseResult)->References[*iRef].DisplayName));
			OpcUa_LocalizedText_CopyTo(&aLocalizedText,&((*ppNewBrowseResult)->References[*iRef].DisplayName));
		}
		// Forward
		(*ppNewBrowseResult)->References[*iRef].IsForward=OpcUa_False;
		if (!bNoForward)
		{
			OpcUa_Boolean bVal=pReference->IsInverse();
			if (bVal)
				(*ppNewBrowseResult)->References[*iRef].IsForward=OpcUa_False;
			else
				(*ppNewBrowseResult)->References[*iRef].IsForward=OpcUa_True; 
		}
		// NodeClass
		if (!bNoNodeClass)
			(*ppNewBrowseResult)->References[*iRef].NodeClass=pUABaseTarget->GetNodeClass(); 
		else
			(*ppNewBrowseResult)->References[*iRef].NodeClass=OpcUa_NodeClass_Unspecified;
		// NodeId
		OpcUa_ExpandedNodeId aExpNodeId = pReference->GetTargetId();
		OpcUa_ExpandedNodeId_CopyTo(&aExpNodeId,&((*ppNewBrowseResult)->References[*iRef].NodeId));
		// ReferenceTypeId
		OpcUa_NodeId_Initialize(&((*ppNewBrowseResult)->References[*iRef].ReferenceTypeId));
		if (!bNoReferenceTypeId)
		{ 
			OpcUa_NodeId aReferenceTypeId = pReference->GetReferenceTypeId();
			OpcUa_NodeId_CopyTo(&aReferenceTypeId, &((*ppNewBrowseResult)->References[*iRef].ReferenceTypeId));
		}
		// TypeDefinition
		OpcUa_NodeId_Initialize(&((*ppNewBrowseResult)->References[*iRef].TypeDefinition.NodeId));											
		OpcUa_String_Initialize( &((*ppNewBrowseResult)->References[*iRef].TypeDefinition.NamespaceUri));
		(*ppNewBrowseResult)->References[*iRef].TypeDefinition.ServerIndex=0;
		if (!bNoTypeDefinition)
		{
			// Il faut rechercher le TypeDefinition de ce NodeId
			CUABase* pTmpNode=OpcUa_Null;
			if (pInformationModel->GetNodeIdFromDictionnary((*ppNewBrowseResult)->References[*iRef].NodeId.NodeId,&pTmpNode)==OpcUa_Good)
			{
				OpcUa_NodeId aTypeDefinition = pTmpNode->GetTypeDefinition();
				OpcUa_NodeId_CopyTo(&aTypeDefinition, &((*ppNewBrowseResult)->References[*iRef].TypeDefinition.NodeId));
			}
			else
			{
				OpcUa_NodeId aTypeDefinition = pUABase->GetTypeDefinition();
				OpcUa_NodeId_CopyTo(&aTypeDefinition, &((*ppNewBrowseResult)->References[*iRef].TypeDefinition.NodeId));
			}
		}
		// Extra Info
		(*ppNewBrowseResult)->ContinuationPoint.Data=OpcUa_Null;
		(*ppNewBrowseResult)->ContinuationPoint.Length=0;
		(*ppNewBrowseResult)->StatusCode=OpcUa_Good;
		(*ppNewBrowseResult)->NoOfReferences++;
		(*iRef)++;
	}
	else
		uStatus=OpcUa_BadInvalidArgument;
	return uStatus;
}

///-------------------------------------------------------------------------------------------------
/// <summary>
/// 	Add inverse references not already added from the NodeSet file. This method is called
/// 	from the PostProcessing function. This will allow the browsing inverse of the addressSpace
/// 	
/// 	 Please note that treatment slowed server initialization.
/// </summary>
///
/// <remarks>	Michel, 13/06/2016. </remarks>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode CUAInformationModel::InvertForwardReferences()
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	//scan de l'ensemble de l'espace d'adressage pour toutes les variables
	// Traitement de la liste de referenceType
	for (OpcUa_UInt32 ii=0;ii<m_pReferenceTypeList->size();ii++)
	{
		CUAReferenceType* pReferenceType=m_pReferenceTypeList->at(ii);
		if (pReferenceType)
		{
			for (OpcUa_Int32 iii=0;iii<pReferenceType->GetNoOfReferences();iii++)
			{
				CUAReference* pReference=pReferenceType->GetReferenceNodeList()->at(iii);
			
				// ReferenceTypeId
				// If faut inverser la reference pReference->ReferenceTypeId
				OpcUa_NodeId aReferenceTypeId;
				OpcUa_NodeId_Initialize(&aReferenceTypeId);
				uStatus=OpcUa_BadNothingToDo;
				if (!pReference->IsInverse()) // reference forward
				{
					aReferenceTypeId=pReference->GetReferenceTypeId();
					uStatus=OpcUa_Good;
				}
				if (uStatus==OpcUa_Good)
				{
					// recherche de pReference->TargetId
					CUABase* pUABase=NULL;
					OpcUa_ExpandedNodeId aExpNodeId;
					OpcUa_ExpandedNodeId_Initialize(&aExpNodeId);
					aExpNodeId = pReference->GetTargetId();
					uStatus = GetNodeIdFromDictionnary(aExpNodeId.NodeId, &pUABase);
					if (uStatus==OpcUa_Good)
					{
						CUAReference* pNewReference=new CUAReference(); //(OpcUa_ReferenceNode*)OpcUa_Alloc(sizeof(OpcUa_ReferenceNode));
						
						pNewReference->SetInverse(!pReference->IsInverse()); // on inverse la reference de base
						pNewReference->SetReferenceTypeId(&aReferenceTypeId); 
						// TargetId
						OpcUa_NodeId* pNodeId=pReferenceType->GetNodeId();
						pNewReference->SetTargetId(pNodeId);
						if (pUABase->IsReferenceExist(pNewReference)==OpcUa_Good)
							pUABase->GetReferenceNodeList()->push_back(pNewReference);
						else
							delete pNewReference;
					}
					else
					{
						char* szNodeId = OpcUa_Null;
						char* szTargetNodeId = OpcUa_Null;
						OpcUa_NodeId* pNodeId = pReferenceType->GetNodeId();
						Utils::NodeId2String(pNodeId,&szNodeId);
						Utils::NodeId2String(&(aExpNodeId.NodeId), &szTargetNodeId);
						if ( (szNodeId) && (szTargetNodeId))
							OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"UpdateAllInverseReferences>Critical error uStatus=0x%05x. %s contains a forward reference with a not found target : %s\n",uStatus,szNodeId,szTargetNodeId);
						if (szNodeId)
							OpcUa_Free(szNodeId);
						if (szTargetNodeId)
							OpcUa_Free(szTargetNodeId);
					}
					// OpcUa_ExpandedNodeId_Clear(&aExpNodeId); // No need to clear it because it's a reference define by the stack in OpcUa_Reference Structure
				}
				//OpcUa_NodeId_Clear(&aReferenceTypeId);
				
			}
		}
	}
	// traitement de la liste de Object
	CUAObjectList::iterator itOL;
	for (itOL = m_pUAObjectList->begin(); itOL != m_pUAObjectList->end();itOL++)
	{
		CUAObject* pObject=itOL->second;
		if (pObject)
		{
			for (OpcUa_Int32 iii=0;iii<pObject->GetNoOfReferences();iii++)
			{
				CUAReference* pReference=pObject->GetReferenceNodeList()->at(iii);
				// ReferenceTypeId
				// If faut inverser la reference pReference->ReferenceTypeId
				OpcUa_NodeId aReferenceTypeId;
				OpcUa_NodeId_Initialize(&aReferenceTypeId);
				uStatus=OpcUa_BadNothingToDo;
				if (!pReference->IsInverse()) // reference forward
				{
					aReferenceTypeId=pReference->GetReferenceTypeId();
					uStatus=OpcUa_Good;
				}
				if (uStatus==OpcUa_Good)
				{
					// recherche de pReference->TargetId
					CUABase* pUABase=NULL;
					OpcUa_ExpandedNodeId aExpNodeId;
					OpcUa_ExpandedNodeId_Initialize(&aExpNodeId);
					aExpNodeId=pReference->GetTargetId();
					uStatus = GetNodeIdFromDictionnary(aExpNodeId.NodeId, &pUABase);
					if (uStatus==OpcUa_Good)
					{
						CUAReference* pNewReference=new CUAReference(); //(OpcUa_ReferenceNode*)OpcUa_Alloc(sizeof(OpcUa_ReferenceNode));
						pNewReference->SetInverse(!pReference->IsInverse()); // on inverse la reference de base
						pNewReference->SetReferenceTypeId(&aReferenceTypeId); 
						// TargetId
						OpcUa_NodeId* pNodeId = pObject->GetNodeId();
						pNewReference->SetTargetId(pNodeId);
						if (pUABase->IsReferenceExist(pNewReference)==OpcUa_Good)
							pUABase->GetReferenceNodeList()->push_back(pNewReference);
						else
							delete pNewReference;
					}
					else
					{
						//char* szNodeId=(char*)malloc(1024*sizeof(char));
						//memset(szNodeId,0,1024*sizeof(char));
						char* szNodeId = OpcUa_Null; 
						OpcUa_NodeId* pNodeId = pObject->GetNodeId();
						Utils::NodeId2String(pNodeId,&szNodeId);
						//char* szTargetNodeId=(char*)malloc(1024*sizeof(char));
						//memset(szTargetNodeId,0,1024*sizeof(char));
						char* szTargetNodeId = OpcUa_Null;
						Utils::NodeId2String(&(aExpNodeId.NodeId),&szTargetNodeId);
						OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "UpdateAllInverseReferences>Critical error uStatus=0x%05x. %s contains a forward reference with a not found target : %s\n", uStatus, szNodeId, szTargetNodeId);
						if (szNodeId)
							OpcUa_Free(szNodeId);
						if (szTargetNodeId)
							OpcUa_Free(szTargetNodeId);
					}
					//OpcUa_ExpandedNodeId_Clear(&aNodeId);
				}
				//OpcUa_NodeId_Clear(&aReferenceTypeId);
			}
		}
	}
	// traitement de la liste de ObjectType
	for (OpcUa_UInt32 ii=0;ii<m_pUAObjectTypeList->size();ii++)
	{
		CUAObjectType* pObjectType=m_pUAObjectTypeList->at(ii);
		if (pObjectType)
		{
			for (OpcUa_Int32 iii=0;iii<pObjectType->GetNoOfReferences();iii++)
			{
				CUAReference* pReference=pObjectType->GetReferenceNodeList()->at(iii);
				// ReferenceTypeId
				// If faut inverser la reference pReference->ReferenceTypeId
				OpcUa_NodeId aReferenceTypeId;
				OpcUa_NodeId_Initialize(&aReferenceTypeId);
				uStatus=OpcUa_BadNothingToDo;
				if (!pReference->IsInverse()) // reference forward
				{
					aReferenceTypeId=pReference->GetReferenceTypeId();
					uStatus=OpcUa_Good;
				}
				if (uStatus==OpcUa_Good)
				{
					// recherche de pReference->TargetId
					CUABase* pUABase=NULL;
					OpcUa_ExpandedNodeId aExpNodeId;
					OpcUa_ExpandedNodeId_Initialize(&aExpNodeId);
					aExpNodeId=pReference->GetTargetId();
					uStatus = GetNodeIdFromDictionnary(aExpNodeId.NodeId, &pUABase);
					if (uStatus==OpcUa_Good)
					{
						CUAReference* pNewReference=new CUAReference(); //(OpcUa_ReferenceNode*)OpcUa_Alloc(sizeof(OpcUa_ReferenceNode));
						pNewReference->SetInverse(!pReference->IsInverse()); // on inverse la reference de base
						pNewReference->SetReferenceTypeId(&aReferenceTypeId);
						// TargetId
						OpcUa_NodeId* pNodeId=pObjectType->GetNodeId();
						pNewReference->SetTargetId(pNodeId);
						if (pUABase->IsReferenceExist(pNewReference)==OpcUa_Good)
							pUABase->GetReferenceNodeList()->push_back(pNewReference);
						else
							delete pNewReference;
					}
					else
					{
						char* szNodeId = OpcUa_Null;
						char* szTargetNodeId = OpcUa_Null;
						OpcUa_NodeId* pNodeId = pObjectType->GetNodeId();
						Utils::NodeId2String(pNodeId,&szNodeId);
						Utils::NodeId2String(&(aExpNodeId.NodeId),&szTargetNodeId);
						if ((szNodeId) && (szTargetNodeId))
							OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "UpdateAllInverseReferences>Critical error uStatus=0x%05x. %s contains a forward reference with a not found target : %s\n", uStatus, szNodeId, szTargetNodeId);
						if (szNodeId)
							OpcUa_Free(szNodeId);
						if (szTargetNodeId)
							OpcUa_Free(szTargetNodeId);
					}
					//OpcUa_ExpandedNodeId_Clear(&aExpNodeId);
				}
				//OpcUa_NodeId_Clear(&aReferenceTypeId);
			}
		}
	}
	// traitement de la liste de DataType	
	for (OpcUa_UInt32 ii=0;ii<m_pUADataTypeList->size();ii++)
	{
		CUADataType* pDataType=m_pUADataTypeList->at(ii);
		if (pDataType)
		{
			for (OpcUa_Int32 iii=0;iii<pDataType->GetNoOfReferences();iii++)
			{
				CUAReference* pReference=pDataType->GetReferenceNodeList()->at(iii);
				// ReferenceTypeId
				// If faut inverser la reference pReference->ReferenceTypeId
				OpcUa_NodeId aReferenceTypeId;
				OpcUa_NodeId_Initialize(&aReferenceTypeId);
				uStatus=OpcUa_BadNothingToDo;
				if (!pReference->IsInverse()) // reference forward
				{
					aReferenceTypeId=pReference->GetReferenceTypeId();
					uStatus=OpcUa_Good;
				}
				if (uStatus==OpcUa_Good)
				{
					// recherche de pReference->TargetId
					CUABase* pUABase=NULL;
					OpcUa_ExpandedNodeId aExpNodeId;
					OpcUa_ExpandedNodeId_Initialize(&aExpNodeId);
					aExpNodeId=pReference->GetTargetId();
					uStatus = GetNodeIdFromDictionnary(aExpNodeId.NodeId, &pUABase);
					if (uStatus==OpcUa_Good)
					{
						CUAReference* pNewReference=new CUAReference(); //(OpcUa_ReferenceNode*)OpcUa_Alloc(sizeof(OpcUa_ReferenceNode));
						pNewReference->SetInverse(!pReference->IsInverse()); // on inverse la reference de base
						pNewReference->SetReferenceTypeId(&aReferenceTypeId);
						// TargetId
						OpcUa_NodeId* pNodeId=pDataType->GetNodeId();
						pNewReference->SetTargetId(pNodeId);

						if (pUABase->IsReferenceExist(pNewReference)==OpcUa_Good)
							pUABase->GetReferenceNodeList()->push_back(pNewReference);
						else
							delete pNewReference;
					}
					else
					{
						char* szNodeId = OpcUa_Null;
						char* szTargetNodeId = OpcUa_Null;
						OpcUa_NodeId* pNodeId = pDataType->GetNodeId();
						Utils::NodeId2String(pNodeId,&szNodeId);
						Utils::NodeId2String(&(aExpNodeId.NodeId),&szTargetNodeId);
						OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "UpdateAllInverseReferences>Critical error uStatus=0x%05x. %s contains a forward reference with a not found target : %s\n", uStatus, szNodeId, szTargetNodeId);
						OpcUa_Free(szNodeId);
						OpcUa_Free(szTargetNodeId);
					}
					//OpcUa_ExpandedNodeId_Clear(&aExpNodeId);
				}
				//OpcUa_NodeId_Clear(&aReferenceTypeId);
			}
		}
	}
	// traitement de la liste de VariableType
	for (OpcUa_UInt32 ii=0;ii<m_pUAVariableTypeList->size();ii++)
	{
		CUAVariableType* pVariableType=m_pUAVariableTypeList->at(ii);
		if (pVariableType)
		{
			for (OpcUa_Int32 iii=0;iii<pVariableType->GetNoOfReferences();iii++)
			{
				CUAReference* pReference=pVariableType->GetReferenceNodeList()->at(iii);
				// ReferenceTypeId
				// If faut inverser la reference pReference->ReferenceTypeId
				OpcUa_NodeId aReferenceTypeId;
				OpcUa_NodeId_Initialize(&aReferenceTypeId);
				uStatus=OpcUa_BadNothingToDo;
				if (!pReference->IsInverse()) // reference forward
				{
					aReferenceTypeId=pReference->GetReferenceTypeId();
					uStatus=OpcUa_Good;
				}
				if (uStatus==OpcUa_Good)
				{
					// recherche de pReference->TargetId
					CUABase* pUABase=NULL;
					OpcUa_ExpandedNodeId aExpNodeId;
					OpcUa_ExpandedNodeId_Initialize(&aExpNodeId);
					aExpNodeId=pReference->GetTargetId();
					uStatus = GetNodeIdFromDictionnary(aExpNodeId.NodeId, &pUABase);
					if (uStatus==OpcUa_Good)
					{
						CUAReference* pNewReference=new CUAReference(); //(OpcUa_ReferenceNode*)OpcUa_Alloc(sizeof(OpcUa_ReferenceNode));
						pNewReference->SetInverse(!pReference->IsInverse()); // on inverse la reference de base
						pNewReference->SetReferenceTypeId(&aReferenceTypeId);
						// TargetId
						OpcUa_NodeId* pNodeId=pVariableType->GetNodeId();
						pNewReference->SetTargetId(pNodeId);
						if (pUABase->IsReferenceExist(pNewReference)==OpcUa_Good)
							pUABase->GetReferenceNodeList()->push_back(pNewReference);
						else
							delete pNewReference;
					}
					else
					{
						char* szNodeId = OpcUa_Null;
						char* szTargetNodeId = OpcUa_Null;
						OpcUa_NodeId* pNodeId = pVariableType->GetNodeId();
						Utils::NodeId2String(pNodeId,&szNodeId);
						Utils::NodeId2String(&(aExpNodeId.NodeId),&szTargetNodeId);
						if ((szNodeId) && (szTargetNodeId))
							OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "UpdateAllInverseReferences>Critical error uStatus=0x%05x. %s contains a forward reference with a not found target : %s\n", uStatus, szNodeId, szTargetNodeId);
						if (szNodeId)
							OpcUa_Free(szNodeId);
						if (szTargetNodeId)
							OpcUa_Free(szTargetNodeId);
					}
					//OpcUa_ExpandedNodeId_Clear(&aExpNodeId);
				}
				//OpcUa_NodeId_Clear(&aReferenceTypeId);
			}
		}
	}
	// traitement de la liste de Variable
	CUAVariableList::iterator itVL;
	for (itVL = m_pUAVariableList->begin(); itVL != m_pUAVariableList->end();itVL++)
	{
		CUAVariable* pVariable = itVL->second;
		if (pVariable)
		{
			for (OpcUa_Int32 iii=0;iii<pVariable->GetNoOfReferences();iii++)
			{
				CUAReference* pReference=pVariable->GetReferenceNodeList()->at(iii);
				// ReferenceTypeId
				// If faut inverser la reference pReference->ReferenceTypeId
				OpcUa_NodeId aReferenceTypeId;
				OpcUa_NodeId_Initialize(&aReferenceTypeId);
				uStatus=OpcUa_BadNothingToDo;
				if (!pReference->IsInverse()) // reference forward
				{
					aReferenceTypeId=pReference->GetReferenceTypeId();
					uStatus=OpcUa_Good;
				}
				if (uStatus==OpcUa_Good)
				{
					// recherche de pReference->TargetId
					CUABase* pUABase=NULL;
					OpcUa_ExpandedNodeId aExpNodeId;
					OpcUa_ExpandedNodeId_Initialize(&aExpNodeId);
					aExpNodeId=pReference->GetTargetId();
					uStatus = GetNodeIdFromDictionnary(aExpNodeId.NodeId, &pUABase);
					if (uStatus==OpcUa_Good)
					{
						CUAReference* pNewReference=new CUAReference(); 
						pNewReference->SetInverse(!pReference->IsInverse()); // on inverse la reference de base
						pNewReference->SetReferenceTypeId(&aReferenceTypeId);
						// TargetId
						OpcUa_NodeId* pNodeId=pVariable->GetNodeId();
						pNewReference->SetTargetId(pNodeId);

						if (pUABase->IsReferenceExist(pNewReference)==OpcUa_Good)
							pUABase->GetReferenceNodeList()->push_back(pNewReference);
						else
							delete pNewReference;
					}
					else
					{
						char* szNodeId = OpcUa_Null;
						char* szTargetNodeId = OpcUa_Null;
						OpcUa_NodeId* pNodeId = pVariable->GetNodeId();
						Utils::NodeId2String(pNodeId,&szNodeId);
						Utils::NodeId2String(&(aExpNodeId.NodeId),&szTargetNodeId);
						if ((szNodeId) && (szTargetNodeId))
							OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "UpdateAllInverseReferences>Critical error uStatus=0x%05x. %s contains a forward reference with a not found target : %s\n", uStatus, szNodeId, szTargetNodeId);
						if (szNodeId)
							OpcUa_Free(szNodeId);
						if (szTargetNodeId)
							OpcUa_Free(szTargetNodeId);
					}
					//OpcUa_ExpandedNodeId_Clear(&aExpNodeId);
				}
				//OpcUa_NodeId_Clear(&aReferenceTypeId);
			}
		}
	}
	// traitement de la liste de View
	for (OpcUa_UInt32 ii=0;ii<m_pUAViewList->size();ii++)
	{
		CUAView* pView=m_pUAViewList->at(ii);
		if (pView)
		{
			for (OpcUa_Int32 iii=0;iii<pView->GetNoOfReferences();iii++)
			{
				CUAReference* pReference=pView->GetReferenceNodeList()->at(iii);
				// ReferenceTypeId
				// If faut inverser la reference pReference->ReferenceTypeId
				OpcUa_NodeId aReferenceTypeId;
				OpcUa_NodeId_Initialize(&aReferenceTypeId);
				uStatus=OpcUa_BadNothingToDo;
				if (!pReference->IsInverse()) // reference forward
				{
					aReferenceTypeId=pReference->GetReferenceTypeId();
					uStatus=OpcUa_Good;
				}
				if (uStatus==OpcUa_Good)
				{
					// recherche de pReference->TargetId
					CUABase* pUABase=NULL;
					OpcUa_ExpandedNodeId aExpNodeId;
					OpcUa_ExpandedNodeId_Initialize(&aExpNodeId);
					aExpNodeId=pReference->GetTargetId();
					uStatus = GetNodeIdFromDictionnary(aExpNodeId.NodeId, &pUABase);
					if (uStatus==OpcUa_Good)
					{
						CUAReference* pNewReference=new CUAReference(); //(OpcUa_ReferenceNode*)OpcUa_Alloc(sizeof(OpcUa_ReferenceNode));
						pNewReference->SetInverse(!pReference->IsInverse()); // on inverse la reference de base
						pNewReference->SetReferenceTypeId(&aReferenceTypeId);
						// TargetId
						OpcUa_NodeId* pNodeId=pView->GetNodeId();
						pNewReference->SetTargetId(pNodeId);
						if (pUABase->IsReferenceExist(pNewReference)==OpcUa_Good)
							pUABase->GetReferenceNodeList()->push_back(pNewReference);
						else
							delete pNewReference;
					}
					else
					{
						char* szNodeId = OpcUa_Null;
						char* szTargetNodeId = OpcUa_Null;
						OpcUa_NodeId* pNodeId = pView->GetNodeId();
						Utils::NodeId2String(pNodeId,&szNodeId);
						Utils::NodeId2String(&(aExpNodeId.NodeId),&szTargetNodeId);
						if ((szNodeId) && (szTargetNodeId))
							OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "UpdateAllInverseReferences>Critical error uStatus=0x%05x. %s contains a forward reference with a not found target : %s\n", uStatus, szNodeId, szTargetNodeId);
						if (szNodeId)
							OpcUa_Free(szNodeId);
						if (szTargetNodeId)
							OpcUa_Free(szTargetNodeId);
					}
					//OpcUa_ExpandedNodeId_Clear(&aExpNodeId);
				}
				//OpcUa_NodeId_Clear(&aReferenceTypeId);
			}
		}
	}
	// traitement de la liste de Method
	for (OpcUa_UInt32 ii=0;ii<m_pUAMethodList->size();ii++)
	{
		CUAMethod* pMethod=m_pUAMethodList->at(ii);
		if (pMethod)
		{
			for (OpcUa_Int32 iii=0;iii<pMethod->GetNoOfReferences();iii++)
			{
				CUAReference* pReference=pMethod->GetReferenceNodeList()->at(iii);
				// ReferenceTypeId
				// If faut inverser la reference pReference->ReferenceTypeId
				OpcUa_NodeId aReferenceTypeId;
				OpcUa_NodeId_Initialize(&aReferenceTypeId);
				uStatus=OpcUa_BadNothingToDo;
				if (!pReference->IsInverse()) // reference forward
				{
					aReferenceTypeId=pReference->GetReferenceTypeId();
					uStatus=OpcUa_Good;
				}

				if (uStatus==OpcUa_Good)
				{
					// recherche de pReference->TargetId
					CUABase* pUABase=NULL;
					OpcUa_ExpandedNodeId aExpNodeId;
					OpcUa_ExpandedNodeId_Initialize(&aExpNodeId);
					aExpNodeId=pReference->GetTargetId();
					uStatus = GetNodeIdFromDictionnary(aExpNodeId.NodeId, &pUABase);
					if (uStatus==OpcUa_Good)
					{
						CUAReference* pNewReference=new CUAReference(); //(OpcUa_ReferenceNode*)OpcUa_Alloc(sizeof(OpcUa_ReferenceNode));
						pNewReference->SetInverse(!pReference->IsInverse()); // on inverse la reference de base
						pNewReference->SetReferenceTypeId(&aReferenceTypeId);
						// TargetId
						OpcUa_NodeId* pNodeId=pMethod->GetNodeId();
						pNewReference->SetTargetId(pNodeId);
						if (pUABase->IsReferenceExist(pNewReference)==OpcUa_Good)
							pUABase->GetReferenceNodeList()->push_back(pNewReference);
						else
							delete pNewReference;
					}
					else
					{
						char* szNodeId = OpcUa_Null;
						OpcUa_NodeId* pNodeId = pMethod->GetNodeId();
						Utils::NodeId2String(pNodeId, &szNodeId);
						if (szNodeId)
						{
							char* szTargetNodeId = OpcUa_Null;
							Utils::NodeId2String(&(aExpNodeId.NodeId), &szTargetNodeId);
							if (szTargetNodeId)
							{
								OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "UpdateAllInverseReferences>Critical error uStatus=0x%05x. %s contains a forward reference with a not found target : %s\n", uStatus, szNodeId, szTargetNodeId);
								OpcUa_Free(szTargetNodeId);
							}
							OpcUa_Free(szNodeId);
						}
					}
					//OpcUa_ExpandedNodeId_Clear(&aExpNodeId);
				}
				//OpcUa_NodeId_Clear(&aReferenceTypeId);
			}
		}
	}
	return uStatus;
}


// Function name   : CUAInformationModel::InvertInverseReferences
// Description     : Search for all inverse references and make them forward keeping the inverse
//					 So we add an inverse reference for each forward detected
// Return type     : OpcUa_StatusCode 

OpcUa_StatusCode CUAInformationModel::InvertInverseReferences()
{
	OpcUa_StatusCode uStatus=OpcUa_BadNothingToDo;
	//scan de l'ensemble de l'espace d'adressage pour toutes les variables
	// Traitement de la liste de referenceType
	for (OpcUa_UInt32 ii=0;ii<m_pReferenceTypeList->size();ii++)
	{
		CUAReferenceType* pReferenceType=m_pReferenceTypeList->at(ii);
		if (pReferenceType)
		{
			for (OpcUa_Int32 iii=0;iii<pReferenceType->GetNoOfReferences();iii++)
			{
				CUAReference* pReference=pReferenceType->GetReferenceNodeList()->at(iii);
			
				// ReferenceTypeId
				// If faut inverser la reference pReference->ReferenceTypeId
				OpcUa_NodeId aReferenceTypeId;
				OpcUa_NodeId_Initialize(&aReferenceTypeId);
				uStatus=OpcUa_BadNothingToDo;
				if (pReference->IsInverse()) // reference Not Forward (Inverse)
				{
					aReferenceTypeId=pReference->GetReferenceTypeId();
					uStatus=OpcUa_Good;
				}
				if (uStatus==OpcUa_Good)
				{
					// recherche de pReference->TargetId
					CUABase* pUABase=NULL;
					uStatus = GetNodeIdFromDictionnary(pReference->GetTargetId().NodeId, &pUABase);
					if (uStatus==OpcUa_Good)
					{
						CUAReference* pNewReference=new CUAReference();
						pNewReference->SetInverse(!pReference->IsInverse()); // on inverse la reference de base
						pNewReference->SetReferenceTypeId(&aReferenceTypeId); 
						// TargetId
						OpcUa_NodeId* pNodeId=pReferenceType->GetNodeId();
						pNewReference->SetTargetId(pNodeId);
						if (pUABase->IsReferenceExist(pNewReference)==OpcUa_Good)
							pUABase->GetReferenceNodeList()->push_back(pNewReference);
						else
							delete pNewReference; // this reference already exists so will delete the temporay one
					}
					else
						OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"InvertNotForwardReferences>GetNodeIdFromDictionnary failed in ReferenceTypeList\n");
				}
				
			}
		}
	}
	// traitement de la liste de Object
	CUAObjectList::iterator itOL;
	for (itOL = m_pUAObjectList->begin(); itOL != m_pUAObjectList->end(); itOL++)
	{
		CUAObject* pObject = itOL->second;
		if (pObject)
		{
			for (OpcUa_Int32 iii=0;iii<pObject->GetNoOfReferences();iii++)
			{
				CUAReference* pReference=pObject->GetReferenceNodeList()->at(iii);
				// ReferenceTypeId
				// If faut inverser la reference pReference->ReferenceTypeId
				OpcUa_NodeId aReferenceTypeId;
				OpcUa_NodeId_Initialize(&aReferenceTypeId);
				uStatus=OpcUa_BadNothingToDo;
				if (pReference->IsInverse()) // reference inverse
				{
					aReferenceTypeId=pReference->GetReferenceTypeId();
					uStatus=OpcUa_Good;
				}
				if (uStatus==OpcUa_Good)
				{
					// recherche de pReference->TargetId
					CUABase* pUABase=NULL;
					uStatus = GetNodeIdFromDictionnary(pReference->GetTargetId().NodeId, &pUABase);
					if (uStatus==OpcUa_Good)
					{
						CUAReference* pNewReference=new CUAReference(); 
						pNewReference->SetInverse(!pReference->IsInverse()); // on inverse la reference de base 
						pNewReference->SetReferenceTypeId(&aReferenceTypeId); 
						// TargetId
						OpcUa_NodeId* pNodeId=pObject->GetNodeId();
						pNewReference->SetTargetId(pNodeId);
						if (pUABase->IsReferenceExist(pNewReference)==OpcUa_Good)
							pUABase->GetReferenceNodeList()->push_back(pNewReference);
						else
							delete pNewReference;
					}
					else
					{
						char* szNodeId = OpcUa_Null;
						// NodeId
						OpcUa_NodeId* pNodeId = pObject->GetNodeId();
						Utils::NodeId2String(pNodeId,&szNodeId);
						if (szNodeId)
						{
							// Attribute
							OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "InvertNotForwardReferences>GetNodeIdFromDictionnary failed %s in ObjectList\n", szNodeId);
							free(szNodeId);
						}
					}
				}
			}
		}
	}
	// traitement de la liste de ObjectType
	for (OpcUa_UInt32 ii=0;ii<m_pUAObjectTypeList->size();ii++)
	{
		CUAObjectType* pObjectType=m_pUAObjectTypeList->at(ii);
		if (pObjectType)
		{
			for (OpcUa_Int32 iii=0;iii<pObjectType->GetNoOfReferences();iii++)
			{
				CUAReference* pReference=pObjectType->GetReferenceNodeList()->at(iii);
				// ReferenceTypeId
				// If faut inverser la reference pReference->ReferenceTypeId
				OpcUa_NodeId aReferenceTypeId;
				OpcUa_NodeId_Initialize(&aReferenceTypeId);
				uStatus=OpcUa_BadNothingToDo;
				if (pReference->IsInverse()) // reference forward
				{
					aReferenceTypeId=pReference->GetReferenceTypeId();
					uStatus=OpcUa_Good;
				}
				if (uStatus==OpcUa_Good)
				{
					// recherche de pReference->TargetId
					CUABase* pUABase=NULL;
					uStatus = GetNodeIdFromDictionnary(pReference->GetTargetId().NodeId, &pUABase);
					if (uStatus==OpcUa_Good)
					{
						CUAReference* pNewReference=new CUAReference(); 
						pNewReference->SetInverse(!pReference->IsInverse()); // on inverse la reference de base
						pNewReference->SetReferenceTypeId(&aReferenceTypeId); 
						// TargetId
						OpcUa_NodeId* pNodeId=pObjectType->GetNodeId();
						pNewReference->SetTargetId(pNodeId);
						if (pUABase->IsReferenceExist(pNewReference)==OpcUa_Good)
							pUABase->GetReferenceNodeList()->push_back(pNewReference);
						else
							delete pNewReference;
					}
					else
					{
						char* szNodeId = OpcUa_Null;
						// NodeId
						OpcUa_NodeId* pNodeId = pObjectType->GetNodeId();
						Utils::NodeId2String(pNodeId,&szNodeId);
						if (szNodeId)
						{
							// Attribute
							OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "InvertNotForwardReferences>GetNodeIdFromDictionnary failed %s in ObjectTypeList\n", szNodeId);
							free(szNodeId);
						}
					}
				}
			}
		}
	}
	// traitement de la liste de DataType	
	for (OpcUa_UInt32 ii=0;ii<m_pUADataTypeList->size();ii++)
	{
		CUADataType* pDataType=m_pUADataTypeList->at(ii);
		if (pDataType)
		{
			for (OpcUa_Int32 iii=0;iii<pDataType->GetNoOfReferences();iii++)
			{
				CUAReference* pReference=pDataType->GetReferenceNodeList()->at(iii);
				// ReferenceTypeId
				// If faut inverser la reference pReference->ReferenceTypeId
				OpcUa_NodeId aReferenceTypeId;
				OpcUa_NodeId_Initialize(&aReferenceTypeId);
				uStatus=OpcUa_BadNothingToDo;
				if (pReference->IsInverse()) // reference forward
				{
					aReferenceTypeId=pReference->GetReferenceTypeId();
					uStatus=OpcUa_Good;
				}
				if (uStatus==OpcUa_Good)
				{
					// recherche de pReference->TargetId
					CUABase* pUABase=NULL;
					uStatus = GetNodeIdFromDictionnary(pReference->GetTargetId().NodeId, &pUABase);
					if (uStatus==OpcUa_Good)
					{
						CUAReference* pNewReference=new CUAReference(); 
						pNewReference->SetInverse(!pReference->IsInverse()); // on inverse la reference de base
						pNewReference->SetReferenceTypeId(&aReferenceTypeId); // modif mai 2015 
						// TargetId
						OpcUa_NodeId* pNodeId=pDataType->GetNodeId();
						pNewReference->SetTargetId(pNodeId);
						if (pUABase->IsReferenceExist(pNewReference)==OpcUa_Good)
							pUABase->GetReferenceNodeList()->push_back(pNewReference);
						else
							delete pNewReference;
					}
					else
					{
						char* szNodeId = OpcUa_Null;
						// NodeId
						OpcUa_NodeId aNodeId = pReference->GetTargetId().NodeId;
						Utils::NodeId2String(&aNodeId,&szNodeId);
						if (szNodeId)
						{
							OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "InvertNotForwardReferences>GetNodeIdFromDictionnary failed in DataTypeList %s\n", szNodeId);
							free(szNodeId);
						}
					}
				}
			}
		}
	}
	// traitement de la liste de VariableType
	for (OpcUa_UInt32 ii=0;ii<m_pUAVariableTypeList->size();ii++)
	{
		CUAVariableType* pVariableType=m_pUAVariableTypeList->at(ii);
		if (pVariableType)
		{
			for (OpcUa_Int32 iii=0;iii<pVariableType->GetNoOfReferences();iii++)
			{
				CUAReference* pReference=pVariableType->GetReferenceNodeList()->at(iii);
				// ReferenceTypeId
				// If faut inverser la reference pReference->ReferenceTypeId
				OpcUa_NodeId aReferenceTypeId;
				OpcUa_NodeId_Initialize(&aReferenceTypeId);
				uStatus=OpcUa_BadNothingToDo;
				if (pReference->IsInverse()) // reference forward
				{
					aReferenceTypeId=pReference->GetReferenceTypeId();
					uStatus=OpcUa_Good;
				}
				if (uStatus==OpcUa_Good)
				{
					// recherche de pReference->TargetId
					CUABase* pUABase=NULL;
					uStatus = GetNodeIdFromDictionnary(pReference->GetTargetId().NodeId, &pUABase);
					if (uStatus==OpcUa_Good)
					{
						CUAReference* pNewReference=new CUAReference(); 
						pNewReference->SetInverse(!pReference->IsInverse()); // on inverse la reference de base
						pNewReference->SetReferenceTypeId(&aReferenceTypeId); 
						// TargetId
						OpcUa_NodeId* pNodeId=pVariableType->GetNodeId();
						pNewReference->SetTargetId(pNodeId);

						if (pUABase->IsReferenceExist(pNewReference)==OpcUa_Good)
							pUABase->GetReferenceNodeList()->push_back(pNewReference);
						else
							delete pNewReference;
					}
					else
					{
						// SourceNodeId
						char* szSourceNodeId = OpcUa_Null;
						OpcUa_NodeId* pNodeId = pVariableType->GetNodeId();
						Utils::NodeId2String(pNodeId,&szSourceNodeId);
						// TargetNodeId
						char* szTargetNodeId = OpcUa_Null;
						OpcUa_NodeId aNodeId = pReference->GetTargetId().NodeId;
						Utils::NodeId2String(&aNodeId,&szTargetNodeId);
						if ((szSourceNodeId) && (szTargetNodeId))
						{
							if (pReference->IsInverse())
								OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "InvertNotForwardReferences>Undefined Inverse %s->%s in VariableTypeList\n", szTargetNodeId, szSourceNodeId);
							else
								OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "InvertNotForwardReferences>Undefined Forward Target %s->%s in VariableTypeList\n", szSourceNodeId, szTargetNodeId);
						}
						if (szSourceNodeId)
							free(szSourceNodeId);		
						if (szTargetNodeId)
							free(szTargetNodeId);		
					}
				}
			}
		}
	}
	// traitement de la liste de Variable
	CUAVariableList::iterator it;
	for (it = m_pUAVariableList->begin(); it != m_pUAVariableList->end(); it++)
	{
		CUAVariable* pVariable = it->second;
		if (pVariable)
		{
			for (OpcUa_Int32 iii=0;iii<pVariable->GetNoOfReferences();iii++)
			{
				CUAReference* pReference=pVariable->GetReferenceNodeList()->at(iii);
				// ReferenceTypeId
				// If faut inverser la reference pReference->ReferenceTypeId
				OpcUa_NodeId aReferenceTypeId;
				OpcUa_NodeId_Initialize(&aReferenceTypeId);
				uStatus=OpcUa_BadNothingToDo;
				if (pReference->IsInverse()) // reference forward
				{
					aReferenceTypeId=pReference->GetReferenceTypeId();
					uStatus=OpcUa_Good;
				}
				if (uStatus==OpcUa_Good)
				{
					// recherche de pReference->TargetId
					CUABase* pUABase=NULL;
					uStatus = GetNodeIdFromDictionnary(pReference->GetTargetId().NodeId, &pUABase);
					if (uStatus==OpcUa_Good)
					{
						CUAReference* pNewReference=new CUAReference(); 
						pNewReference->SetInverse(!pReference->IsInverse()); 
						pNewReference->SetReferenceTypeId(&aReferenceTypeId);
						// TargetId
						OpcUa_NodeId* pNodeId=pVariable->GetNodeId();
						pNewReference->SetTargetId(pNodeId);
						if (pUABase->IsReferenceExist(pNewReference)==OpcUa_Good)
							pUABase->GetReferenceNodeList()->push_back(pNewReference);
						else
							delete pNewReference;
					}
					else
					{
						// SourceNodeId
						char* szSourceNodeId = OpcUa_Null;
						OpcUa_NodeId* pNodeId = pVariable->GetNodeId();
						Utils::NodeId2String(pNodeId,&szSourceNodeId);
						// TargetNodeId
						OpcUa_NodeId aTargetNodeId=pReference->GetTargetId().NodeId;
						char* szTargetNodeId = OpcUa_Null;
						Utils::NodeId2String(&aTargetNodeId,&szTargetNodeId);
						if ((szSourceNodeId) && (szTargetNodeId))
						{
							if (pReference->IsInverse())
								OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "InvertNotForwardReferences>Undefined Inverse [%s]->%s in VariableList\n", szTargetNodeId, szSourceNodeId);
							else
								OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "InvertNotForwardReferences>Undefined Forward Target %s->[%s] in VariableList\n", szSourceNodeId, szTargetNodeId);
						}

						if (szSourceNodeId)
							free(szSourceNodeId);	
						if (szTargetNodeId)
							free(szTargetNodeId);					
					}
				}
			}
		}
	}
	// traitement de la liste de View
	for (OpcUa_UInt32 ii=0;ii<m_pUAViewList->size();ii++)
	{
		CUAView* pView=m_pUAViewList->at(ii);
		if (pView)
		{
			for (OpcUa_Int32 iii=0;iii<pView->GetNoOfReferences();iii++)
			{
				CUAReference* pReference=pView->GetReferenceNodeList()->at(iii);
				// ReferenceTypeId
				// If faut inverser la reference pReference->ReferenceTypeId
				OpcUa_NodeId aReferenceTypeId;
				OpcUa_NodeId_Initialize(&aReferenceTypeId);
				uStatus=OpcUa_BadNothingToDo;
				if (pReference->IsInverse()) // reference forward
				{
					aReferenceTypeId=pReference->GetReferenceTypeId();
					uStatus=OpcUa_Good;
				}
				if (uStatus==OpcUa_Good)
				{
					// recherche de pReference->TargetId
					CUABase* pUABase=NULL;
					uStatus = GetNodeIdFromDictionnary(pReference->GetTargetId().NodeId, &pUABase);
					if (uStatus==OpcUa_Good)
					{
						CUAReference* pNewReference=new CUAReference(); 
						pNewReference->SetInverse(!pReference->IsInverse()); // on inverse la reference de base
						pNewReference->SetReferenceTypeId(&aReferenceTypeId); 
						// TargetId
						OpcUa_NodeId* pNodeId=pView->GetNodeId();
						pNewReference->SetTargetId(pNodeId);
						if (pUABase->IsReferenceExist(pNewReference)==OpcUa_Good)
							pUABase->GetReferenceNodeList()->push_back(pNewReference);
						else
							delete pNewReference;
					}
					else
						OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"InvertNotForwardReferences>GetNodeIdFromDictionnary failed in ViewList\n");
				}
			}
		}
	}
	// traitement de la liste de Method
	for (OpcUa_UInt32 ii=0;ii<m_pUAMethodList->size();ii++)
	{
		CUAMethod* pMethod=m_pUAMethodList->at(ii);
		if (pMethod)
		{
			for (OpcUa_Int32 iii=0;iii<pMethod->GetNoOfReferences();iii++)
			{
				CUAReference* pReference=pMethod->GetReferenceNodeList()->at(iii);
				// ReferenceTypeId
				// If faut inverser la reference pReference->ReferenceTypeId
				OpcUa_NodeId aReferenceTypeId;
				OpcUa_NodeId_Initialize(&aReferenceTypeId);
				uStatus=OpcUa_BadNothingToDo;
				if (pReference->IsInverse()) // reference forward
				{
					aReferenceTypeId=pReference->GetReferenceTypeId();
					uStatus=OpcUa_Good;
				}
				if (uStatus==OpcUa_Good)
				{
					// recherche de pReference->TargetId
					CUABase* pUABase=NULL;
					uStatus = GetNodeIdFromDictionnary(pReference->GetTargetId().NodeId, &pUABase);
					if (uStatus==OpcUa_Good)
					{
						CUAReference* pNewReference=new CUAReference(); 
						pNewReference->SetInverse(!pReference->IsInverse()); // on inverse la reference de base
						pNewReference->SetReferenceTypeId(&aReferenceTypeId); 
						// TargetId
						OpcUa_NodeId* pNodeId=pMethod->GetNodeId();
						pNewReference->SetTargetId(pNodeId);

						if (pUABase->IsReferenceExist(pNewReference)==OpcUa_Good)
							pUABase->GetReferenceNodeList()->push_back(pNewReference);
						else
							delete pNewReference;
					}
					else
						OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"InvertNotForwardReferences>GetNodeIdFromDictionnary failed in MethodList\n");
				}
			}
		}
	}
	return uStatus;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Updates all initialize UAVariableType DataType with BaseDataType (i=24). </summary>
///
/// <remarks>	Michel, 19/09/2015. </remarks>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode CUAInformationModel::UpdateUAVariableTypeDataType()
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	CUAVariableTypeList* pUAVariableTypeList = GetVariableTypeList();
	int iVariableTypeListSize = pUAVariableTypeList->size();
	for (int iii = 0; iii<iVariableTypeListSize; iii++)
	{
		//OpcUa_Boolean bFound=OpcUa_False;
		CUAVariableType* pUAVariableType = ((*pUAVariableTypeList)[iii]);
		// Verification que l'initialisation du type n'a pas été faites
		OpcUa_NodeId aDataType = pUAVariableType->GetDataType();
		if ((aDataType.Identifier.Numeric == 0) && (aDataType.NamespaceIndex == 0))
		{
			OpcUa_NodeId baseDataType;
			OpcUa_NodeId_Initialize(&baseDataType);
			baseDataType.IdentifierType = OpcUa_IdentifierType_Numeric;
			baseDataType.Identifier.Numeric = 24;
			baseDataType.NamespaceIndex = 0;
			// Affection du type
			pUAVariableType->SetDataType(baseDataType);
		}
	}
	return uStatus;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Initialize all uninitialize CDataValue of UAVariable. </summary>
///
/// <remarks>	Michel, 19/09/2015. </remarks>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode CUAInformationModel::UpdatePendingVariableDatatype()
{
	OpcUa_StatusCode uStatus = OpcUa_BadNothingToDo;
	CUAVariableList* pUAVariableList = GetVariableList();
	CUAVariableList::iterator it;
	for (it = pUAVariableList->begin(); it != pUAVariableList->end(); it++)
	{
		CUAVariable* pUAVariable = it->second;
		if (pUAVariable->GetValue() == NULL)
		{
			uStatus = pUAVariable->InitializeDataValue();
		}
	}
	return uStatus;
}
// mise à jour de la zone d'accès rapide au node m_UAInformationModelFastAccessList

///-------------------------------------------------------------------------------------------------
/// <summary>	Updates the Node fast access list (m_UAInformationModelFastAccessList). </summary>
///
/// <remarks>	Michel, 19/09/2015. </remarks>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode CUAInformationModel::UpdateInformationModelFastAccessList()
{
	OpcUa_StatusCode uStatus=OpcUa_BadNothingToDo;
	// Traitement de la liste de referenceType
	for (OpcUa_UInt32 ii=0;ii<m_pReferenceTypeList->size();ii++)
	{
		CUAReferenceType* pReferenceType=m_pReferenceTypeList->at(ii);
		if (pReferenceType)
		{
			OpcUa_NodeId* pNodeId=pReferenceType->GetNodeId();
			if (pNodeId->NamespaceIndex==0)
			{
				CUAInformationModelFastAccess* pUAInformationModelFastAccess
					=new CUAInformationModelFastAccess((CUABase*)pReferenceType);
				if (pUAInformationModelFastAccess->GetStatusCode() == OpcUa_Good)
				{
					OpcUa_UInt32 uiNumeric = pUAInformationModelFastAccess->GetNumericNodeId();
					delete m_UAInformationModelFastAccessList[uiNumeric];
					m_UAInformationModelFastAccessList[uiNumeric] = pUAInformationModelFastAccess;
				}
				else
					delete pUAInformationModelFastAccess;
			}
		}
	}
	// traitement de la liste de Object
	CUAObjectList::iterator itOL;
	for (itOL = m_pUAObjectList->begin(); itOL != m_pUAObjectList->end(); itOL++)
	{
		CUAObject* pObject = itOL->second;
		if (pObject)
		{
			OpcUa_NodeId* pNodeId=pObject->GetNodeId();
			if (pNodeId->NamespaceIndex==0)
			{
				CUAInformationModelFastAccess* pUAInformationModelFastAccess
					=new CUAInformationModelFastAccess((CUABase*)pObject);
				if (pUAInformationModelFastAccess->GetStatusCode()==OpcUa_Good)
				{
					OpcUa_UInt32 uiNumeric=pUAInformationModelFastAccess->GetNumericNodeId();
					delete m_UAInformationModelFastAccessList[uiNumeric];
					m_UAInformationModelFastAccessList[uiNumeric]=pUAInformationModelFastAccess;
				}
				else
					delete pUAInformationModelFastAccess;
			}
		}
	}
	// traitement de la liste de ObjectType
	for (OpcUa_UInt32 ii=0;ii<m_pUAObjectTypeList->size();ii++)
	{
		CUAObjectType* pObjectType=m_pUAObjectTypeList->at(ii);
		if (pObjectType)
		{
			OpcUa_NodeId* pNodeId=pObjectType->GetNodeId();
			if (pNodeId->NamespaceIndex==0)
			{
				CUAInformationModelFastAccess* pUAInformationModelFastAccess
					=new CUAInformationModelFastAccess((CUABase*)pObjectType);
				if (pUAInformationModelFastAccess->GetStatusCode()==OpcUa_Good)
				{
					OpcUa_UInt32 uiNumeric=pUAInformationModelFastAccess->GetNumericNodeId();
					delete m_UAInformationModelFastAccessList[uiNumeric];
					m_UAInformationModelFastAccessList[uiNumeric]=pUAInformationModelFastAccess;
				}
				else
					delete pUAInformationModelFastAccess;
			}
		}
	}
	// traitement de la liste de DataType	
	for (OpcUa_UInt32 ii=0;ii<m_pUADataTypeList->size();ii++)
	{
		CUADataType* pDataType=m_pUADataTypeList->at(ii);
		if (pDataType)
		{
			OpcUa_NodeId* pNodeId=pDataType->GetNodeId();
			if (pNodeId->NamespaceIndex==0)
			{
				CUAInformationModelFastAccess* pUAInformationModelFastAccess
					=new CUAInformationModelFastAccess((CUABase*)pDataType);
				if (pUAInformationModelFastAccess->GetStatusCode()==OpcUa_Good)
				{
					OpcUa_UInt32 uiNumeric=pUAInformationModelFastAccess->GetNumericNodeId();
					delete m_UAInformationModelFastAccessList[uiNumeric];
					m_UAInformationModelFastAccessList[uiNumeric]=pUAInformationModelFastAccess;
				}
				else
					delete pUAInformationModelFastAccess;
			}
		}
	}
	// traitement de la liste de VariableType
	for (OpcUa_UInt32 ii=0;ii<m_pUAVariableTypeList->size();ii++)
	{
		CUAVariableType* pVariableType=m_pUAVariableTypeList->at(ii);
		if (pVariableType)
		{
			OpcUa_NodeId* pNodeId=pVariableType->GetNodeId();
			if (pNodeId->NamespaceIndex == 0)
			{
				CUAInformationModelFastAccess* pUAInformationModelFastAccess
					=new CUAInformationModelFastAccess((CUABase*)pVariableType);
				if (pUAInformationModelFastAccess->GetStatusCode()==OpcUa_Good)
				{
					OpcUa_UInt32 uiNumeric=pUAInformationModelFastAccess->GetNumericNodeId();
					delete m_UAInformationModelFastAccessList[uiNumeric];
					m_UAInformationModelFastAccessList[uiNumeric]=pUAInformationModelFastAccess;
				}
				else
					delete pUAInformationModelFastAccess;
			}
		}
	}
	// traitement de la liste de Variable
	CUAVariableList::iterator it;
	for (it = m_pUAVariableList->begin(); it != m_pUAVariableList->end(); it++)
	{
		CUAVariable* pVariable = it->second;
		if (pVariable)
		{
			OpcUa_NodeId* pNodeId=pVariable->GetNodeId();
			if (pNodeId->NamespaceIndex == 0)
			{
				CUAInformationModelFastAccess* pUAInformationModelFastAccess
					=new CUAInformationModelFastAccess((CUABase*)pVariable);
				if (pUAInformationModelFastAccess->GetStatusCode()==OpcUa_Good)
				{
					OpcUa_UInt32 uiNumeric=pUAInformationModelFastAccess->GetNumericNodeId();
					delete m_UAInformationModelFastAccessList[uiNumeric];
					m_UAInformationModelFastAccessList[uiNumeric]=pUAInformationModelFastAccess;
				}
				else
					delete pUAInformationModelFastAccess;
			}
		}
	}
	// traitement de la liste de View
	for (OpcUa_UInt32 ii=0;ii<m_pUAViewList->size();ii++)
	{
		CUAView* pView=m_pUAViewList->at(ii);
		if (pView)
		{
			OpcUa_NodeId* pNodeId=pView->GetNodeId();
			if (pNodeId->NamespaceIndex == 0)
			{
				CUAInformationModelFastAccess* pUAInformationModelFastAccess
					=new CUAInformationModelFastAccess((CUABase*)pView);
				if (pUAInformationModelFastAccess->GetStatusCode()==OpcUa_Good)
				{
					OpcUa_UInt32 uiNumeric=pUAInformationModelFastAccess->GetNumericNodeId();
					delete m_UAInformationModelFastAccessList[uiNumeric];
					m_UAInformationModelFastAccessList[uiNumeric]=pUAInformationModelFastAccess;
				}
				else
					delete pUAInformationModelFastAccess;
			}
		}
	}
	// traitement de la liste de Method
	for (OpcUa_UInt32 ii=0;ii<m_pUAMethodList->size();ii++)
	{
		CUAMethod* pMethod=m_pUAMethodList->at(ii);
		if (pMethod)
		{
			OpcUa_NodeId* pNodeId=pMethod->GetNodeId();
			if (pNodeId->NamespaceIndex == 0)
			{
				CUAInformationModelFastAccess* pUAInformationModelFastAccess
					=new CUAInformationModelFastAccess((CUABase*)pMethod);
				if (pUAInformationModelFastAccess->GetStatusCode()==OpcUa_Good)
				{
					OpcUa_UInt32 uiNumeric=pUAInformationModelFastAccess->GetNumericNodeId();
					delete m_UAInformationModelFastAccessList[uiNumeric];
					m_UAInformationModelFastAccessList[uiNumeric]=pUAInformationModelFastAccess;
				}
				else
					delete pUAInformationModelFastAccess;
			}
		}
	}
	uStatus=OpcUa_Good;
	return uStatus;
}

// Function name   : CUAInformationModel::InitializeEncodeableObject
// Description     : Initialize a CUAVariable based on its EncodeableObject
//					 The encapsulated "Object" will be initialize to 0 on AllocationSize byte
//					 The method handle both scalar and array
// Return type     : OpcUa_StatusCode 
// Argument        : CUAVariable* pUAVariable

OpcUa_StatusCode CUAInformationModel::InitializeEncodeableObject(CUAVariable* pUAVariable)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	if (pUAVariable->GetBuiltInType()==OpcUaType_ExtensionObject)
	{
		CDataValue* pValue=pUAVariable->GetValue();
		{
			// initialize if not already done
			pUAVariable->InitializeDataValue();
			//
			pValue=pUAVariable->GetValue();
			OpcUa_EncodeableType* pEncodeableType=OpcUa_Null;
			uStatus=g_pTheApplication->LookupEncodeableType(pUAVariable->GetDataType().Identifier.Numeric,&pEncodeableType);
			if (uStatus==OpcUa_Good)
			{
				if (pEncodeableType)
				{
					OpcUa_Variant aVariant = pValue->GetValue();
					OpcUa_Variant_Initialize(&aVariant);
					// on recupère l'etat courant du OpcUa_Variant dans l'UAVariable
					aVariant=pValue->GetValue();
					if (aVariant.ArrayType==OpcUa_VariantArrayType_Array)
					{
						// On a besoin de connaitre l'indice du tableau en cours.
						int iCurrentElt=aVariant.Value.Array.Length;
						if (iCurrentElt > 0)
						{
							for (int ii = 0; ii < iCurrentElt; ii++)
							{
								aVariant.Value.Array.Value.ExtensionObjectArray[ii].BodySize = 0;
								aVariant.Value.Array.Value.ExtensionObjectArray[ii].Encoding = OpcUa_ExtensionObjectEncoding_EncodeableObject;
								OpcUa_ExpandedNodeId_Initialize(&(aVariant.Value.Array.Value.ExtensionObjectArray[ii].TypeId)); //iCurrentElt
								aVariant.Value.Array.Value.ExtensionObjectArray[ii].TypeId.NodeId.Identifier.Numeric = pEncodeableType->TypeId;
								aVariant.Value.Array.Value.ExtensionObjectArray[ii].TypeId.NodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
								aVariant.Value.Array.Value.ExtensionObjectArray[ii].TypeId.ServerIndex = 0;
								aVariant.Value.Array.Value.ExtensionObjectArray[ii].Body.EncodeableObject.Type = pEncodeableType;// Utils::Copy(pEncodeableType);
								aVariant.Value.Array.Value.ExtensionObjectArray[ii].Body.EncodeableObject.Object = OpcUa_Alloc(pEncodeableType->AllocationSize);
								OpcUa_MemSet(aVariant.Value.Array.Value.ExtensionObjectArray[ii].Body.EncodeableObject.Object, 0, pEncodeableType->AllocationSize);
							}
						}
						//else
						//	OpcUa_Free(pEncodeableType);
					}
					else
					{
						if (aVariant.ArrayType==OpcUa_VariantArrayType_Scalar)
						{
							aVariant.Datatype=OpcUaType_ExtensionObject;
							aVariant.Value.ExtensionObject=(OpcUa_ExtensionObject*)OpcUa_Alloc(sizeof(OpcUa_ExtensionObject));
							OpcUa_ExtensionObject_Clear(aVariant.Value.ExtensionObject);
							aVariant.Value.ExtensionObject->BodySize=0;
							aVariant.Value.ExtensionObject->Encoding=OpcUa_ExtensionObjectEncoding_EncodeableObject;
							OpcUa_ExpandedNodeId_Initialize(&(aVariant.Value.ExtensionObject->TypeId));
							aVariant.Value.ExtensionObject->TypeId.NodeId.IdentifierType=OpcUa_IdentifierType_Numeric;
							aVariant.Value.ExtensionObject->TypeId.ServerIndex=0;
							aVariant.Value.ExtensionObject->TypeId.NodeId.Identifier.Numeric=pEncodeableType->TypeId;
							aVariant.Value.ExtensionObject->Body.EncodeableObject.Object=(void*)OpcUa_Alloc(pEncodeableType->AllocationSize);
							aVariant.Value.ExtensionObject->Body.EncodeableObject.Type = pEncodeableType; // Utils::Copy(pEncodeableType); // modif mai 2015
							OpcUa_MemSet(aVariant.Value.ExtensionObject->Body.EncodeableObject.Object,0,pEncodeableType->AllocationSize); // par défaut le contenu de ExtensionObject est NULL
						}
						else
							if (aVariant.ArrayType==OpcUa_VariantArrayType_Matrix)
								OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"OpcUa_VariantArrayType_Matrix  not supported\n");
					}
					pValue->SetValue(aVariant);
					//OpcUa_Variant_Clear(&aVariant);
				}
			}
			else
			{
				//
				// Search for the definition of this new UADataType in list of DataType
				CUADataType* pUADataType=OpcUa_Null;
				if (GetNodeIdFromDataTypeList(pUAVariable->GetDataType(),&pUADataType)==OpcUa_Good)
				{
					// here we need to create a new EncodeableObject
					pEncodeableType=OpcUa_Null;
					uStatus=AddNewEncodeableObject(pUADataType,&pEncodeableType);
					if (uStatus==OpcUa_Good)
					{
						OpcUa_Int32 iCount=1;
						OpcUa_EncodeableTypeTable* pTypeTable=g_pTheApplication->GetTypeTable();
						uStatus=OpcUa_EncodeableTypeTable_AddTypes(pTypeTable,iCount,&pEncodeableType);
						if (uStatus==OpcUa_Good)
						{
							OpcUa_Variant aVariant;
							OpcUa_Variant_Initialize(&aVariant);
							// on recupère l'etat courant du OpcUa_Variant dans l'UAVariable
							aVariant=pValue->GetValue();
							if (aVariant.ArrayType==OpcUa_VariantArrayType_Array)
							{
								// On a besoin de connaitre l'indice du tableau en cours.
								int iCurrentElt=aVariant.Value.Array.Length;	
								for (int ii=0;ii<iCurrentElt;ii++)
								{
									aVariant.Value.Array.Value.ExtensionObjectArray[ii].BodySize=0;
									aVariant.Value.Array.Value.ExtensionObjectArray[ii].Encoding=OpcUa_ExtensionObjectEncoding_EncodeableObject;
									OpcUa_ExpandedNodeId_Initialize(&(aVariant.Value.Array.Value.ExtensionObjectArray[ii].TypeId)); //iCurrentElt
									aVariant.Value.Array.Value.ExtensionObjectArray[ii].TypeId.NodeId.Identifier.Numeric=pEncodeableType->TypeId;
									aVariant.Value.Array.Value.ExtensionObjectArray[ii].TypeId.NodeId.IdentifierType=OpcUa_IdentifierType_Numeric;
									aVariant.Value.Array.Value.ExtensionObjectArray[ii].TypeId.ServerIndex=0;
									aVariant.Value.Array.Value.ExtensionObjectArray[ii].Body.EncodeableObject.Type = pEncodeableType;// Utils::Copy(pEncodeableType);
									aVariant.Value.Array.Value.ExtensionObjectArray[ii].Body.EncodeableObject.Object=OpcUa_Alloc(pEncodeableType->AllocationSize);
									OpcUa_MemSet(aVariant.Value.Array.Value.ExtensionObjectArray[ii].Body.EncodeableObject.Object,0,pEncodeableType->AllocationSize);
								}
							}
							else
							{
								if (aVariant.ArrayType==OpcUa_VariantArrayType_Scalar)
								{
									aVariant.Datatype=OpcUaType_ExtensionObject;
									aVariant.Value.ExtensionObject=(OpcUa_ExtensionObject*)OpcUa_Alloc(sizeof(OpcUa_ExtensionObject));
									aVariant.Value.ExtensionObject->BodySize=0;
									aVariant.Value.ExtensionObject->Encoding=OpcUa_ExtensionObjectEncoding_EncodeableObject;
									OpcUa_ExpandedNodeId_Initialize(&(aVariant.Value.ExtensionObject->TypeId));
									aVariant.Value.ExtensionObject->TypeId.NodeId.IdentifierType=OpcUa_IdentifierType_Numeric;
									aVariant.Value.ExtensionObject->TypeId.ServerIndex=0;
									aVariant.Value.ExtensionObject->TypeId.NodeId.Identifier.Numeric=pEncodeableType->TypeId;
									aVariant.Value.ExtensionObject->Body.EncodeableObject.Object=(void*)OpcUa_Alloc(pEncodeableType->AllocationSize);
									aVariant.Value.ExtensionObject->Body.EncodeableObject.Type = pEncodeableType;// Utils::Copy(pEncodeableType);
									OpcUa_MemSet(aVariant.Value.ExtensionObject->Body.EncodeableObject.Object,0,pEncodeableType->AllocationSize); // par défaut le contenu de ExtensionObject est NULL
								}
								else
									if (aVariant.ArrayType==OpcUa_VariantArrayType_Matrix)
										OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"OpcUa_VariantArrayType_Matrix  not supported\n");
							}
							pValue->SetValue(aVariant);
						}
					}
					else
					{
						char* szNodeId = OpcUa_Null;
						OpcUa_NodeId aNodeId = pUAVariable->GetDataType();
						Utils::NodeId2String(&aNodeId, &szNodeId);
						if (szNodeId)
						{
							OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "AddNewEncodeableObject failed. We cannot handle the new UADataType %s uStatus=0x%05x\n", szNodeId, uStatus);
							free(szNodeId);
						}
					}
				}
				else
				{
					// alternative init for some special Node without known OpcUa_EncodeableType
					OpcUa_Variant aVariant;
					OpcUa_Variant_Initialize(&aVariant);
					aVariant.Datatype=OpcUaType_ExtensionObject;
					aVariant.Value.ExtensionObject=(OpcUa_ExtensionObject*)OpcUa_Alloc(sizeof(OpcUa_ExtensionObject));
					OpcUa_ExtensionObject_Initialize(aVariant.Value.ExtensionObject);				
					aVariant.Value.ExtensionObject->BodySize=0;
					aVariant.Value.ExtensionObject->Encoding=OpcUa_ExtensionObjectEncoding_EncodeableObject;
					pValue->SetValue(aVariant);
					// Trace
					char* szNodeId = OpcUa_Null;
					OpcUa_NodeId aNodeId = pUAVariable->GetDataType();
					Utils::NodeId2String(&aNodeId, &szNodeId);
					if (szNodeId)
					{
						OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critical error.GetNodeIdFromDataTypeList failed Your configuration files are probably corrupted. %s is unknown\n", szNodeId);
						free(szNodeId);
					}
					pValue->SetValue(aVariant);
				}
			}
		}
		//else
		//{
		//	wchar_t* szNodeId=(wchar_t*)malloc(1024*sizeof(wchar_t));
		//	memset(szNodeId,0,1024*sizeof(wchar_t));
		//	Utils::NodeId2String(pUAVariable->GetDataType(),&szNodeId);
		//	OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"ExtensionObject %ls Already initialized \n",szNodeId);
		//	free(szNodeId);
		//}
	}
	else
		uStatus=OpcUa_BadInvalidArgument;
	return uStatus;
}
 
// Function name   : CUAInformationModel::TraceAddressSpace
// Description     : Show the size of the AddressSpace
// Return type     : void 
void CUAInformationModel::TraceAddressSpace() 
{
	OpcUa_UInt32 iTotalNodes=	
		m_pUAObjectList->size()+
		m_pUAVariableList->size()+
		m_pUAViewList->size()+
		m_pUAMethodList->size()+
		m_pUAObjectTypeList->size()+
		m_pReferenceTypeList->size()+
		m_pUADataTypeList->size()+
		m_pUAVariableTypeList->size();

	OpcUa_Trace(OPCUA_TRACE_LEVEL_ALWAYS,"%u Nodes in the addressSpace split in \n\t%u Objects %u Variables %u Views %u Methods \n\t%u ObjectTypes %u ReferenceTypes %u DataTypes %u VariableTypes\n",
		iTotalNodes,
		m_pUAObjectList->size(),
		m_pUAVariableList->size(),
		m_pUAViewList->size(),
		m_pUAMethodList->size(),
		m_pUAObjectTypeList->size(),
		m_pReferenceTypeList->size(),
		m_pUADataTypeList->size(),
		m_pUAVariableTypeList->size());
}

OpcUa_StatusCode CUAInformationModel::AddNewEncodeableObject(CUADataType* pDataType, OpcUa_EncodeableType** pEncodeableType)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	if (pDataType)
	{
		(*pEncodeableType)=(OpcUa_EncodeableType*)OpcUa_Alloc(sizeof(OpcUa_EncodeableType));
		if ((*pEncodeableType))
		{
			ZeroMemory((*pEncodeableType),sizeof(OpcUa_EncodeableType));
			OpcUa_String aString=pDataType->GetBrowseName()->Name;
			(*pEncodeableType)->TypeName=OpcUa_String_GetRawString(&aString);
			OpcUa_NodeId* pNodeId = pDataType->GetNodeId();
			if (pNodeId)
			{
				(*pEncodeableType)->TypeId = pNodeId->Identifier.Numeric;
				// Get the namespace Uri 
				switch (pNodeId->IdentifierType)
				{
				case OpcUa_IdentifierType_Numeric:
				{
					CNamespaceUri* pNameSpaceUri = GetNamespaceUri(pNodeId->NamespaceIndex);
					if (pNameSpaceUri)
					{
						OpcUa_String* pStringUri = pNameSpaceUri->GetUriName();
						if (pStringUri)
						{
							OpcUa_UInt32 iSize = OpcUa_String_StrLen(pStringUri);
							(*pEncodeableType)->NamespaceUri = (OpcUa_CharA*)OpcUa_Alloc(iSize) + 1;
							ZeroMemory((*pEncodeableType)->NamespaceUri, iSize + 1);
							OpcUa_CharA* pchStringUri = OpcUa_String_GetRawString(pStringUri);
							OpcUa_MemCpy((*pEncodeableType)->NamespaceUri, iSize, pchStringUri, iSize),
								(*pEncodeableType)->AllocationSize = pDataType->GetSize();
							// Attention cette initialisation est probablement invalide
							// We have to search the HasEncoding reference for this DataType (pDataType->GetNodeId())
							(*pEncodeableType)->BinaryEncodingTypeId = pNodeId->Identifier.Numeric + 1;
							(*pEncodeableType)->XmlEncodingTypeId = pNodeId->Identifier.Numeric + 2;
							// Now will initialize the function pointer
							(*pEncodeableType)->Clear = OpcUa_EncodeableObjectGeneric_Clear;
							(*pEncodeableType)->Decode = OpcUa_EncodeableObjectGeneric_Decode;
							(*pEncodeableType)->Encode = OpcUa_EncodeableObjectGeneric_Encode;
							(*pEncodeableType)->GetSize = OpcUa_EncodeableObjectGeneric_GetSize;
							(*pEncodeableType)->Initialize = OpcUa_EncodeableObjectGeneric_Initialize;
						}
						else
							uStatus = OpcUa_BadInternalError;
					}
					else
						uStatus = OpcUa_BadInternalError;
				}
				break;
				case OpcUa_IdentifierType_String:
				{
					CNamespaceUri* pNameSpaceUri = GetNamespaceUri(pNodeId->NamespaceIndex);
					if (pNameSpaceUri)
					{
						OpcUa_String* pStringUri = pNameSpaceUri->GetUriName();
						if (pStringUri)
						{
							OpcUa_UInt32 iSize = OpcUa_String_StrLen(pStringUri);
							(*pEncodeableType)->NamespaceUri = (OpcUa_CharA*)OpcUa_Alloc(iSize) + 1;
							ZeroMemory((*pEncodeableType)->NamespaceUri, iSize + 1);
							OpcUa_CharA* pchStringUri = OpcUa_String_GetRawString(pStringUri);
							OpcUa_MemCpy((*pEncodeableType)->NamespaceUri, iSize, pchStringUri, iSize),
								(*pEncodeableType)->AllocationSize = pDataType->GetSize();
							// Attention cette initialisation est probablement invalide
							// We have to search the HasEncoding reference for this DataType (pDataType->GetNodeId())
							(*pEncodeableType)->BinaryEncodingTypeId = /*pDataType->GetNodeId().Identifier.Numeric +*/ 1;
							(*pEncodeableType)->XmlEncodingTypeId = /*pDataType->GetNodeId().Identifier.Numeric +*/ 2;
							// Now will initialize the function pointer
							(*pEncodeableType)->Clear = OpcUa_EncodeableObjectGeneric_Clear;
							(*pEncodeableType)->Decode = OpcUa_EncodeableObjectGeneric_Decode;
							(*pEncodeableType)->Encode = OpcUa_EncodeableObjectGeneric_Encode;
							(*pEncodeableType)->GetSize = OpcUa_EncodeableObjectGeneric_GetSize;
							(*pEncodeableType)->Initialize = OpcUa_EncodeableObjectGeneric_Initialize;
						}
						else
							uStatus = OpcUa_BadInternalError;
					}
					else
						uStatus = OpcUa_BadInternalError;

				}
				break;
				default:
					uStatus = OpcUa_BadInvalidArgument;
					break;
				}
			}
			else
				uStatus = OpcUa_BadInvalidArgument;
		}
		else
			uStatus=OpcUa_BadOutOfMemory;
	}
	else
		uStatus=OpcUa_BadInvalidArgument;
	return uStatus;
}
// Retrieve the EncodingNoding for the receive DataType
// The inputNodeId NodeId must be a DataType
OpcUa_StatusCode CUAInformationModel::GetEncodingNodeId(OpcUa_NodeId inputNodeId, OpcUa_NodeId* pOutEncodingNodeId)
{
	OpcUa_StatusCode uStatus = OpcUa_BadNotFound;
	/*
	CUAObjectList* pUAObjectList = GetObjectList();
	OpcUa_QualifiedName DefaultBinary;
	OpcUa_QualifiedName_Initialize(&DefaultBinary);
	DefaultBinary.NamespaceIndex = 0;
	OpcUa_String_AttachCopy(&(DefaultBinary.Name), "Default Binary");
	OpcUa_Int32 iSize = pUAObjectList->size();
	for (OpcUa_Int32 i = 0; i < iSize; i++)
	{
		CUAObject* pObject = (*pUAObjectList)[i];
		OpcUa_QualifiedName* pQualifiedName = pObject->GetBrowseName();
		
		if (OpcUa_QualifiedName_Compare(pQualifiedName, &DefaultBinary)==0)
		{
			OpcUa_NodeId* pNodeId = pObject->GetNodeId();
			//if (OpcUa_NodeId_Compare(&aNodeId, &inputNodeId) == 0)
			{
				CUAReferenceList* pReferenceList = pObject->GetReferenceNodeList();
				for (OpcUa_UInt32 ii = 0; ii < pReferenceList->size(); ii++)
				{
					CUAReference* pReference = pReferenceList->at(ii);
					if ((pReference->GetReferenceTypeId().IdentifierType == OpcUa_IdentifierType_Numeric)
						&& (pReference->GetReferenceTypeId().Identifier.Numeric == 38)) // 38 == HasEndcoding
					{
						OpcUa_NodeId aTargetNodeId = pReference->GetTargetId().NodeId;
						if (OpcUa_NodeId_Compare(&aTargetNodeId, &inputNodeId) == 0)
						{
							OpcUa_NodeId_CopyTo(pNodeId, pOutEncodingNodeId);
							uStatus = OpcUa_Good;
							break;
						}
					}
				}
			}
		}
	}
	OpcUa_QualifiedName_Clear(&DefaultBinary);
	*/
	return uStatus;
}

// Scan the entire addressSpace looking for UANode with a 
// hasTypeDefinition pointing to a BaseEventType or to a child of BaseEventType
// As explain in part 5 §6.4
// OPC UA defines standard EventTypes.
// They are represented in the AddressSpace as ObjectTypes.
// The EventTypes are already defined in Part 3. 
// Once found we will store those EventType in an STL collection
OpcUa_StatusCode CUAInformationModel::SearchEventsDefinition()
{
	OpcUa_StatusCode uStatus = OpcUa_BadNotFound;
	OpcUa_NodeId BaseEventType;
	OpcUa_NodeId_Initialize(&BaseEventType);
	BaseEventType.IdentifierType = OpcUa_IdentifierType_Numeric;
	BaseEventType.Identifier.Numeric = OpcUaId_BaseEventType; // 2041;

	OpcUa_NodeId HasTypeDefinition;
	OpcUa_NodeId_Initialize(&HasTypeDefinition);
	HasTypeDefinition.IdentifierType = OpcUa_IdentifierType_Numeric;
	HasTypeDefinition.Identifier.Numeric = 40;

	OpcUa_NodeId HasSubtype;
	OpcUa_NodeId_Initialize(&HasSubtype);
	HasSubtype.IdentifierType = OpcUa_IdentifierType_Numeric;
	HasSubtype.Identifier.Numeric = 45;

	// OffNormalAlarmType
	OpcUa_NodeId OffNormalAlarmType;
	OpcUa_NodeId_Initialize(&OffNormalAlarmType);
	OffNormalAlarmType.Identifier.Numeric = 10637;
	// LimitAlarmType
	OpcUa_NodeId LimitAlarmType;
	OpcUa_NodeId_Initialize(&LimitAlarmType);
	LimitAlarmType.Identifier.Numeric = 2955;
	// First Step
	// populate m_pEventTypeList
	// We are looking for reference HasSubtype in CUAObjectType List targeting a EvenType Node
	CUAObjectTypeList* pUAObjectTypeList = GetObjectTypeList();
	OpcUa_Int32 iSize = pUAObjectTypeList->size();
	for (OpcUa_Int32 i = 0; i < iSize; i++)
	{
		// Check reference of pUAObject
		CUAObjectType* pUAObjectType= pUAObjectTypeList->at(i);
		if (pUAObjectType)
		{
			// Check reference of pUAObject
			CUAReferenceList* pRefList = pUAObjectType->GetReferenceNodeList();
			OpcUa_Int32 iNoOfRef = pUAObjectType->GetNoOfReferences();
			for (OpcUa_Int32 ii = 0; ii < iNoOfRef; ii++)
			{
				CUAReference* pReference = pRefList->at(ii);
				if (pReference)
				{
					OpcUa_NodeId aReferenceTypeId = pReference->GetReferenceTypeId();
					if (OpcUa_NodeId_Compare(&HasSubtype, &aReferenceTypeId) == 0)
					{
						OpcUa_ExpandedNodeId aTargetId = pReference->GetTargetId();
						// is aTargetId a subtype of EventType ?
						if (IsInEventTypeHierarchy(BaseEventType, aTargetId.NodeId) == OpcUa_Good)
						{
							// Ok we have one Event definition. 
							// pUAObjectType is an EventType;
							// Let's read its Source Node

							// Let's get the sourceNode for this Event
							// Let search for the reference with the name SourceName
							OpcUa_Boolean bFound = OpcUa_False;
							for (OpcUa_UInt32 iTmp = 0; iTmp < m_pEventTypeList->size(); iTmp++)
							{
								CUABase* pBase=m_pEventTypeList->at(iTmp);
								OpcUa_NodeId* pBaseNodeId = pBase->GetNodeId();
								OpcUa_NodeId* pObjectTypeNodeId = pUAObjectType->GetNodeId();
								if (OpcUa_NodeId_Compare(pBaseNodeId, pObjectTypeNodeId) == 0)
								{
									bFound = OpcUa_True;
									break;
								}
							}
							if (!bFound)
							{
								m_pEventTypeList->push_back(pUAObjectType);
							}
						}
					}
				}
			}
		}
	}
	
	// Second Step
	// now let search of Node that use one of those EventType
	// This means: Because EventType instance are always UAObject
	// with a HasEventSource reference pointing to EventType Instances
	CUAObjectList* pUAObjectList = GetObjectList();
	CUAObjectList::iterator itOL;
	for (itOL = pUAObjectList->begin(); itOL != pUAObjectList->end();itOL++)
	{
		CUAObject* pUAObject = itOL->second;
		if (pUAObject)
		{
			std::vector<OpcUa_ExpandedNodeId*> aTargetEventSourceList;
			uStatus = SearchForHasEventSourceNodeList(pUAObject, &aTargetEventSourceList);
			if (uStatus == OpcUa_Good)
			{
				for (OpcUa_UInt32 i = 0; i < aTargetEventSourceList.size(); i++)
				{
					OpcUa_ExpandedNodeId* pTargetEventSource = aTargetEventSourceList.at(i);

					OpcUa_ExpandedNodeId aTargetCondition;
					OpcUa_ExpandedNodeId_Initialize(&aTargetCondition);
					CUABase* pUABase = OpcUa_Null;
					if (GetNodeIdFromDictionnary(pTargetEventSource->NodeId, &pUABase) == OpcUa_Good)
					{
						CUAObjectType* pUAObjectType = OpcUa_Null;
						uStatus = SearchHasCondition(pUABase, &aTargetCondition);
						if (uStatus == OpcUa_Good)
						{
							// Get the HasTypeDefinition of aTargetCondition
							CUAObject* pUAObjectFound = OpcUa_Null;
							if (GetNodeIdFromObjectList(aTargetCondition.NodeId, &pUAObjectFound) == OpcUa_Good)
							{
								// Check that this typeDefinition is in the m_pEventTypeList
								uStatus = IsInEventTypeList(pUAObjectFound->GetTypeDefinition(), &pUAObjectType); // It should have a HasTypeDefinition that is in the m_pEventTypeList
								if (uStatus == OpcUa_Good)
								{
									CEventsEngine*	pEventsEngine = g_pTheApplication->GetEventsEngine();
									if (pEventsEngine)
									{
										// Let check that pUABase is a UAVariable
										CUAVariable* pUaVariableEventSource = OpcUa_Null;
										if (pUABase->GetNodeClass() == OpcUa_NodeClass_Variable)
											pUaVariableEventSource = (CUAVariable*)pUABase;
										// 
										// We are suppose to dynamicaly instanciate pUAFoundObject. So each CEventDEfinition pour to a unique EventType or template
										// 

										CUAObject* pNewConditionInstance = OpcUa_Null;
										// Instanciate with SourceNode, Condition to receive a new Condition
										if (InstanciateConditionType(pUaVariableEventSource, pUAObjectFound, &pNewConditionInstance) == OpcUa_Good)
										{
											//// Add in the AddressSpace
											//PopulateInformationModel(pNewConditionInstance);
											CEventDefinition* pEventDefinition = new CEventDefinition(pUAObject, pNewConditionInstance, pUaVariableEventSource, pUAObjectType);
											if (pEventDefinition)
											{
												// Save the Event definition in the SourceNode (UAVariable)  that will trigger the event
												pUaVariableEventSource->AddInternalEventDefinition(pEventDefinition);
												////////////////////////////////////////////////
												//
												// Now let find the Methods
												//
												// Enable
												CUAMethod* pMethod = OpcUa_Null;
												OpcUa_QualifiedName MethodBrowseName;
												OpcUa_QualifiedName_Initialize(&MethodBrowseName);
												OpcUa_String_AttachCopy(&(MethodBrowseName.Name), "Enable");
												uStatus = SearchForMethod(pUAObjectFound, MethodBrowseName, &pMethod);
												if (uStatus == OpcUa_Good)
												{
													pEventDefinition->SetEnableMethod(pMethod);
												}
												// Disable
												pMethod = OpcUa_Null;
												OpcUa_QualifiedName_Clear(&MethodBrowseName);
												OpcUa_QualifiedName_Initialize(&MethodBrowseName);
												OpcUa_String_AttachCopy(&(MethodBrowseName.Name), "Disable");
												uStatus = SearchForMethod(pUAObjectFound, MethodBrowseName, &pMethod);
												if (uStatus == OpcUa_Good)
												{
													pEventDefinition->SetDisableMethod(pMethod);
												}
												OpcUa_QualifiedName_Clear(&MethodBrowseName);
												// AddComment

												// ConditionRefresh

												// Acknowledge

												// Confirm
												////////////////////////////////////////////////
												//
												// Now let find SourceName Variable
												//
												//CUAVariable* pSourceNameUAVariable = OpcUa_Null;
												//OpcUa_QualifiedName SourceName;
												//OpcUa_QualifiedName_Initialize(&SourceName);
												//OpcUa_String_AttachCopy(&(SourceName.Name), "SourceName");
												//uStatus = SearchForState(pUAObjectFound, SourceName, &pSourceNameUAVariable);
												//if (uStatus == OpcUa_Good)
												//{
												//	// This should be a LocalizedText. 
												//	if (pSourceNameUAVariable->GetBuiltInType() == OpcUaType_String) // String
												//	{
												//		pEventDefinition->SetSourceName(pSourceNameUAVariable);
												//	}
												//}
												//OpcUa_QualifiedName_Clear(&SourceName);
												//
												// Now let find Message Variable
												//
												CUAVariable* pMessageUAVariable = OpcUa_Null;
												OpcUa_QualifiedName szMessage;
												OpcUa_QualifiedName_Initialize(&szMessage);
												OpcUa_String_AttachCopy(&(szMessage.Name), "Message");
												uStatus = SearchForState(pNewConditionInstance, szMessage, &pMessageUAVariable);
												if (uStatus == OpcUa_Good)
												{
													// This should be a LocalizedText. 
													if (pMessageUAVariable->GetBuiltInType() == OpcUaType_LocalizedText) // LocalizedText
													{
														pEventDefinition->SetEventMessage(pMessageUAVariable);
													}
												}
												OpcUa_QualifiedName_Clear(&szMessage);
												////////////////////////////////////////////////
												//
												// Now let find the AckedState and Id Variable
												//
												CUAVariable* pAckedStateUAVariable = OpcUa_Null;
												OpcUa_QualifiedName AckedState;
												OpcUa_QualifiedName_Initialize(&AckedState);
												OpcUa_String_AttachCopy(&(AckedState.Name), "AckedState");
												uStatus = SearchForState(pNewConditionInstance, AckedState, &pAckedStateUAVariable);
												if (uStatus == OpcUa_Good)
												{
													// This should be a LocalizedText. 
													if (pAckedStateUAVariable->GetBuiltInType() == OpcUaType_LocalizedText) // LocalizedText
													{
														pEventDefinition->SetAckedStateUAVariable(pAckedStateUAVariable);
														// This UAVariable should point to a reference call Id
														CUAVariable* pAckedStateIdUAVariable = OpcUa_Null;

														OpcUa_QualifiedName AckedStateId;
														OpcUa_QualifiedName_Initialize(&AckedStateId);
														OpcUa_String_AttachCopy(&(AckedStateId.Name), "Id");
														if (SearchForState(pAckedStateUAVariable, AckedStateId, &pAckedStateIdUAVariable) == OpcUa_Good)
														{
															if (pAckedStateIdUAVariable->GetBuiltInType() == OpcUaType_Boolean)
															{
																pEventDefinition->SetAckedStateIdUAVariable(pAckedStateIdUAVariable);
															}
														}
														OpcUa_QualifiedName_Clear(&AckedStateId);
													}
												}
												OpcUa_QualifiedName_Clear(&AckedState);
												////////////////////////////////////////////////
												//
												// Now let find the ConfirmedState and Id Variable
												//
												CUAVariable* pConfirmedStateUAVariable = OpcUa_Null;
												OpcUa_QualifiedName ConfirmedState;
												OpcUa_QualifiedName_Initialize(&ConfirmedState);
												OpcUa_String_AttachCopy(&(ConfirmedState.Name), "ConfirmedState");
												uStatus = SearchForState(pNewConditionInstance, ConfirmedState, &pConfirmedStateUAVariable);
												if (uStatus == OpcUa_Good)
												{
													// This should be a LocalizedText. 
													if (pConfirmedStateUAVariable->GetBuiltInType() == OpcUaType_LocalizedText) // LocalizedText
													{
														pEventDefinition->SetConfirmedStateUAVariable(pAckedStateUAVariable);
														// This UAVariable should point to a reference call Id
														CUAVariable* pConfirmedStateIdUAVariable = OpcUa_Null;

														OpcUa_QualifiedName ConfirmedStateId;
														OpcUa_QualifiedName_Initialize(&ConfirmedStateId);
														OpcUa_String_AttachCopy(&(ConfirmedStateId.Name), "Id");
														if (SearchForState(pConfirmedStateUAVariable, ConfirmedStateId, &pConfirmedStateIdUAVariable) == OpcUa_Good)
														{
															if (pConfirmedStateIdUAVariable->GetBuiltInType() == OpcUaType_Boolean)
															{
																pEventDefinition->SetConfirmedStateIdUAVariable(pConfirmedStateIdUAVariable);
															}
														}
														OpcUa_QualifiedName_Clear(&ConfirmedStateId);
													}
												}
												OpcUa_QualifiedName_Clear(&ConfirmedState);
												////////////////////////////////////////////////
												//
												// Now let find the EnabledState and Id Variables
												//
												CUAVariable* pEnabledStateUAVariable = OpcUa_Null;
												OpcUa_QualifiedName EnabledState;
												OpcUa_QualifiedName_Initialize(&EnabledState);
												OpcUa_String_AttachCopy(&(EnabledState.Name), "EnabledState");
												uStatus = SearchForState(pNewConditionInstance, EnabledState, &pEnabledStateUAVariable);
												if (uStatus == OpcUa_Good)
												{
													// This should be a LocalizedText. 
													if (pEnabledStateUAVariable->GetBuiltInType() == OpcUaType_LocalizedText) // LocalizedText
													{
														pEventDefinition->SetRelativeEnabledStateUAVariable(pEnabledStateUAVariable);
														// This UAVariable should point to a reference call Id
														CUAVariable* pEnabledStateIdUAVariable = OpcUa_Null;

														OpcUa_QualifiedName EnabledStateId;
														OpcUa_QualifiedName_Initialize(&EnabledStateId);
														OpcUa_String_AttachCopy(&(EnabledStateId.Name), "Id");
														if (SearchForState(pEnabledStateUAVariable, EnabledStateId, &pEnabledStateIdUAVariable) == OpcUa_Good)
														{
															if (pEnabledStateIdUAVariable->GetBuiltInType() == OpcUaType_Boolean)
															{
																pEventDefinition->SetRelativeEnabledStateIdUAVariable(pEnabledStateIdUAVariable);
															}
														}
														OpcUa_QualifiedName_Clear(&EnabledStateId);
													}
												}
												OpcUa_QualifiedName_Clear(&EnabledState);
												////////////////////////////////////////////////
												//
												// Now let find the ActiveState, Id and EffectiveDisplayName Variable
												//
												CUAVariable* pActiveStateUAVariable = OpcUa_Null;
												OpcUa_QualifiedName ActiveState;
												OpcUa_QualifiedName_Initialize(&ActiveState);
												OpcUa_String_AttachCopy(&(ActiveState.Name), "ActiveState");
												uStatus = SearchForState(pNewConditionInstance, ActiveState, &pActiveStateUAVariable);
												if (uStatus == OpcUa_Good)
												{
													// This should be a LocalizedText. 
													if (pActiveStateUAVariable->GetBuiltInType() == OpcUaType_LocalizedText) // LocalizedText
													{
														pEventDefinition->SetRelatedActiveState(pActiveStateUAVariable);
														// This UAVariable should point to a reference call Id
														CUAVariable* pActiveStateIdUAVariable = OpcUa_Null;

														OpcUa_QualifiedName ActiveStateId;
														OpcUa_QualifiedName_Initialize(&ActiveStateId);
														OpcUa_String_AttachCopy(&(ActiveStateId.Name), "Id");
														//if (SearchForState(pActiveStateUAVariable, ActiveStateId, &pActiveStateIdUAVariable) == OpcUa_Good)
														if (SearchForState(pActiveStateUAVariable, ActiveStateId, &pActiveStateIdUAVariable) == OpcUa_Good)
														{
															if (pActiveStateIdUAVariable->GetBuiltInType() == OpcUaType_Boolean)
															{
																pEventDefinition->SetRelatedActiveStateId(pActiveStateIdUAVariable);
															}
														}
														OpcUa_QualifiedName_Clear(&ActiveStateId);

														// This UAVariable should point to a reference call Id
														CUAVariable* pActiveStateEffectiveDisplayNameUAVariable = OpcUa_Null;
														OpcUa_QualifiedName ActiveStateEffectiveDisplayName;
														OpcUa_QualifiedName_Initialize(&ActiveStateEffectiveDisplayName);
														OpcUa_String_AttachCopy(&(ActiveStateEffectiveDisplayName.Name), "EffectiveDisplayName");
														if (SearchForState(pActiveStateUAVariable, ActiveStateEffectiveDisplayName, &pActiveStateEffectiveDisplayNameUAVariable) == OpcUa_Good)
														{
															if (pActiveStateEffectiveDisplayNameUAVariable->GetBuiltInType() == OpcUaType_LocalizedText)
															{
																pEventDefinition->SetRelatedActiveStateEffectiveDisplayName(pActiveStateEffectiveDisplayNameUAVariable);
															}
														}
														OpcUa_QualifiedName_Clear(&ActiveStateEffectiveDisplayName);
													}
												}
												OpcUa_QualifiedName_Clear(&ActiveState);

												CUAVariable* pRetainUAVariable = OpcUa_Null;
												OpcUa_QualifiedName szRetain;
												OpcUa_QualifiedName_Initialize(&szRetain);
												OpcUa_String_AttachCopy(&(szRetain.Name), "Retain");
												uStatus = SearchForState(pNewConditionInstance, szRetain, &pRetainUAVariable);
												if (uStatus == OpcUa_Good)
												{
													// This should be a Boolean. 
													if (pRetainUAVariable->GetBuiltInType() == OpcUaType_Boolean) // LocalizedText
													{
														pEventDefinition->SetRetainVariable(pRetainUAVariable);
													}
												}
												OpcUa_QualifiedName_Clear(&szRetain);

												// According to the EventType search for some specific reference
												OpcUa_NodeId* pEventTypeNodeId = pUAObjectType->GetNodeId();
												if (OpcUa_NodeId_Compare(pEventTypeNodeId,&LimitAlarmType)==0)
												{
													// Search for Limits
													UALimitsAlarm aLimitAlarm;
													if (SearchForLimits(pUAObjectFound, &aLimitAlarm) == OpcUa_Good)
													{
														// Copy the template Limits to the Instance Limits
														UALimitsAlarm aLimitAlarmInstance;
														if (SearchForLimits(pNewConditionInstance, &aLimitAlarmInstance) == OpcUa_Good)
														{
															//
															CDataValue* pHighHighLimit=aLimitAlarm.pUAVariableHighHighLimit->GetValue();
															aLimitAlarmInstance.pUAVariableHighHighLimit->SetValue(pHighHighLimit);
															// 
															CDataValue* pHighLimit=aLimitAlarm.pUAVariableHighLimit->GetValue();
															aLimitAlarmInstance.pUAVariableHighLimit->SetValue(pHighLimit);
															// 
															CDataValue* pLowLimit=aLimitAlarm.pUAVariableLowLimit->GetValue();
															aLimitAlarmInstance.pUAVariableLowLimit->SetValue(pLowLimit);
															//
															CDataValue* pLowLowLimit=aLimitAlarm.pUAVariableLowLowLimit->GetValue();
															aLimitAlarmInstance.pUAVariableLowLowLimit->SetValue(pLowLowLimit);
															//
															pEventDefinition->SetLimitAlarmValues(&aLimitAlarmInstance);
														}
													}
												}											
												// Add this EventDefinition to the EventEngine
												pEventsEngine->AddEventDefinition(pEventDefinition);
											}
										}
										// 
									}
								}
							}
						}
					}
					OpcUa_ExpandedNodeId_Clear(&aTargetCondition);
				}
				// free resources
				for (OpcUa_UInt32 i = 0; i < aTargetEventSourceList.size(); i++)
				{
					OpcUa_ExpandedNodeId* pTargetEventSource = aTargetEventSourceList.at(i);
					OpcUa_ExpandedNodeId_Clear(pTargetEventSource);
					OpcUa_Free(pTargetEventSource);
				}
				aTargetEventSourceList.clear();
			}
			
		}
	}

	OpcUa_NodeId_Clear(&HasTypeDefinition);
	OpcUa_NodeId_Clear(&BaseEventType);
	OpcUa_NodeId_Clear(&HasSubtype);
	OpcUa_NodeId_Clear(&OffNormalAlarmType);
	OpcUa_NodeId_Clear(&LimitAlarmType);
	return uStatus;
}
// Check that the aNodeId is in the m_pEventTypeList
// return OpcUa_Good if found OpcUa_BadNotFound if not
// pUAObjectType will contains the found pUAObjectType
OpcUa_StatusCode CUAInformationModel::IsInEventTypeList(OpcUa_NodeId aNodeId, CUAObjectType** pUAObjectType)
{
	OpcUa_StatusCode uStatus = OpcUa_BadNotFound;
	for (OpcUa_UInt32 iii = 0; iii < m_pEventTypeList->size(); iii++)
	{
		CUAObjectType* pLocalUAObjectType=m_pEventTypeList->at(iii);
		OpcUa_NodeId* pObjecTypeNodeId = pLocalUAObjectType->GetNodeId();
		if (OpcUa_NodeId_Compare(pObjecTypeNodeId, &aNodeId) == 0)
		{
			(*pUAObjectType) = pLocalUAObjectType;
			uStatus = OpcUa_Good;
		}
	}
	return uStatus;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Searches for the sourceNode of the CUANode (pUANode) passed in parameter.
/// 			The pUANode must be a UAObject and the sourceNode is a UAVariable with a reference 
/// 			targeting a node with name SourceNode and a DataType NodeId. The valeu is the nodeId</summary>
///
/// <remarks>	Michel, 30/08/2016. </remarks>
///
/// <param name="pUANode">	  	[in,out] If non-null, the UA node. </param>
/// <param name="pSourceNode">	[in,out] If non-null, source node. </param>
///
/// <returns>	The found source node. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode CUAInformationModel::SearchForSourceNode(CUABase* pUANode, OpcUa_NodeId* pSourceNode)
{
	OpcUa_StatusCode uStatus = OpcUa_BadNotFound;
	OpcUa_String szSourceNode;
	OpcUa_String_Initialize(&szSourceNode);
	OpcUa_String_AttachCopy(&szSourceNode, "SourceNode");
	if (pUANode)
	{
		CUAReferenceList* pRefList = pUANode->GetReferenceNodeList();
		OpcUa_Int32 iNoOfRef = pUANode->GetNoOfReferences();
		for (OpcUa_Int32 ii = 0; ii < iNoOfRef; ii++)
		{
			CUAReference* pReference = pRefList->at(ii);
			if (pReference)
			{
				// Prepare the TargetId
				OpcUa_ExpandedNodeId aReferenceTargetId = pReference->GetTargetId();
				// Compare the BrowseName
				CUAVariable* pUAVariable = OpcUa_Null;
				if (GetNodeIdFromVariableList(aReferenceTargetId.NodeId, &pUAVariable) == OpcUa_Good)
				{
					OpcUa_QualifiedName* pBrowseName=pUAVariable->GetBrowseName();
					if (OpcUa_String_Compare(&(pBrowseName->Name), &szSourceNode) == 0)
					{
						OpcUa_NodeId_CopyTo(&aReferenceTargetId.NodeId, pSourceNode);
						uStatus = OpcUa_Good;
						break;
					}
				}
			}
		}
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	OpcUa_String_Clear(&szSourceNode);
	return uStatus;
}
// Search for HasEventSource in the references of the pUANode and return the target source in pEventSourceList
OpcUa_StatusCode CUAInformationModel::SearchForHasEventSourceNodeList(CUABase* pUANode, std::vector<OpcUa_ExpandedNodeId*>* pEventSourceList)
{
	OpcUa_NodeId HasEventSource;
	OpcUa_NodeId_Initialize(&HasEventSource);
	HasEventSource.IdentifierType = OpcUa_IdentifierType_Numeric;
	HasEventSource.Identifier.Numeric = 36;

	OpcUa_StatusCode uStatus = OpcUa_BadNotFound;
	CUAReferenceList* pRefList = pUANode->GetReferenceNodeList();
	OpcUa_Int32 iNoOfRef = pUANode->GetNoOfReferences();
	for (OpcUa_Int32 ii = 0; ii < iNoOfRef; ii++)
	{
		CUAReference* pReference = pRefList->at(ii);
		if (pReference)
		{
			// Prepare the TargetId
			OpcUa_ExpandedNodeId aReferenceTargetId = pReference->GetTargetId();
			// Compare the ReferenceTypeIds
			OpcUa_NodeId aReferenceTypeId = pReference->GetReferenceTypeId();
			if (OpcUa_NodeId_Compare(&aReferenceTypeId, &HasEventSource) == 0)
			{
				OpcUa_ExpandedNodeId * pEventSource=(OpcUa_ExpandedNodeId*)OpcUa_Alloc(sizeof(OpcUa_ExpandedNodeId));
				OpcUa_ExpandedNodeId_Initialize(pEventSource);
				OpcUa_ExpandedNodeId_CopyTo(&aReferenceTargetId, pEventSource);
				pEventSourceList->push_back(pEventSource);
				uStatus = OpcUa_Good;
			}
		}
	}
	return uStatus;
}
// Search for HasCondition in the references of the pUANode and return the target source in pEventSource
OpcUa_StatusCode CUAInformationModel::SearchHasCondition(CUABase* pUANode, OpcUa_ExpandedNodeId* pEventConditionId)
{
	OpcUa_NodeId HasCondition;
	OpcUa_NodeId_Initialize(&HasCondition);
	HasCondition.IdentifierType = OpcUa_IdentifierType_Numeric;
	HasCondition.Identifier.Numeric = 9006;

	OpcUa_StatusCode uStatus = OpcUa_BadNotFound;
	CUAReferenceList* pRefList = pUANode->GetReferenceNodeList();
	OpcUa_Int32 iNoOfRef = pUANode->GetNoOfReferences();
	for (OpcUa_Int32 ii = 0; ii < iNoOfRef; ii++)
	{
		CUAReference* pReference = pRefList->at(ii);
		if (pReference)
		{
			if (!pReference->IsInverse())
			{
				OpcUa_NodeId aReferenceTypeId = pReference->GetReferenceTypeId();
				if (OpcUa_NodeId_Compare(&aReferenceTypeId, &HasCondition) == 0)
				{
					OpcUa_ExpandedNodeId aExpNodeId = pReference->GetTargetId();
					OpcUa_ExpandedNodeId_CopyTo(&aExpNodeId, pEventConditionId);
					uStatus = OpcUa_Good;
				}
			}
		}
	}
	return uStatus;
}
// Search for the CUAVariable with the BrowseName == AckedState
// This name is mandatory in the part 9 of the specification
// We will look from from pUABaseFound Node
OpcUa_StatusCode CUAInformationModel::SearchForState(CUABase* pUABaseFound, OpcUa_QualifiedName SearchedState,CUAVariable** pAckedStateUAVariable)
{
	OpcUa_StatusCode uStatus = OpcUa_BadNotFound;
	if (pUABaseFound)
	{
		CUAReferenceList* pRefList = pUABaseFound->GetReferenceNodeList();
		OpcUa_Int32 iNoOfRef = pUABaseFound->GetNoOfReferences();
		for (OpcUa_Int32 ii = 0; ii < iNoOfRef; ii++)
		{
			CUAReference* pReference = pRefList->at(ii);
			if (pReference)
			{
				OpcUa_QualifiedName aTargetBrowseName;
				OpcUa_QualifiedName_Initialize(&aTargetBrowseName);

				OpcUa_ExpandedNodeId aReferenceTargetId = pReference->GetTargetId();
				CUAVariable* pUAVariable = OpcUa_Null;
				if (GetNodeIdFromVariableList(aReferenceTargetId.NodeId, &pUAVariable) == OpcUa_Good)
				{
					OpcUa_QualifiedName_CopyTo(pUAVariable->GetBrowseName(), &aTargetBrowseName);
					if (OpcUa_String_Compare(&SearchedState.Name, &aTargetBrowseName.Name) == 0) // We cannot use the full browsename because the namespace index could be different. It shoudl be interesting to double chech that with the UA WG
					{
						(*pAckedStateUAVariable) = pUAVariable;
						uStatus = OpcUa_Good;
						OpcUa_QualifiedName_Clear(&aTargetBrowseName);
						break;
					}
					OpcUa_QualifiedName_Clear(&aTargetBrowseName);

				}
			}
		}
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}
OpcUa_StatusCode CUAInformationModel::SearchForMethod(CUABase* pUABaseFound, OpcUa_QualifiedName MethodName, CUAMethod** ppMethod)
{
	OpcUa_StatusCode uStatus = OpcUa_BadNotFound;
	if (pUABaseFound)
	{
		CUAReferenceList* pRefList = pUABaseFound->GetReferenceNodeList();
		OpcUa_Int32 iNoOfRef = pUABaseFound->GetNoOfReferences();
		for (OpcUa_Int32 ii = 0; ii < iNoOfRef; ii++)
		{
			CUAReference* pReference = pRefList->at(ii);
			if (pReference)
			{
				OpcUa_QualifiedName aTargetBrowseName;
				OpcUa_QualifiedName_Initialize(&aTargetBrowseName);

				OpcUa_ExpandedNodeId aReferenceTargetId = pReference->GetTargetId();
				CUAMethod* pUAMethod = OpcUa_Null;
				if (GetNodeIdFromMethodList(aReferenceTargetId.NodeId, &pUAMethod) == OpcUa_Good)
				{
					OpcUa_QualifiedName_CopyTo(pUAMethod->GetBrowseName(), &aTargetBrowseName);
					if (OpcUa_String_Compare(&MethodName.Name, &aTargetBrowseName.Name) == 0)// We cannot use the full browsename because the namespace index could be different. It shoudl be interesting to double chech that with the UA WG
					{
						(*ppMethod) = pUAMethod;
						uStatus = OpcUa_Good;
						OpcUa_QualifiedName_Clear(&aTargetBrowseName);
						break;
					}
					OpcUa_QualifiedName_Clear(&aTargetBrowseName);

				}
			}
		}
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Determine if the idEventTarget is in the hierarchy of IdEventSource. 
/// 			So we first create the list of eventType below the IdEventSource.
/// 			The we compare this list with IdEventTargert</summary>
///
/// <remarks>	Michel, 16/03/2016. </remarks>
///
/// <param name="idEventSource">	The identifier event source. </param>
/// <param name="idEventTarget">	The identifier event target. </param>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode CUAInformationModel::IsInEventTypeHierarchy(OpcUa_NodeId idEventSource, OpcUa_NodeId idEventTarget)
{
	OpcUa_StatusCode uStatus = OpcUa_BadNotFound;
	vector<OpcUa_NodeId> eventTypeHierachy; // Liste des references appartenant a cette hirarchie (idReference et ses enfants)
	// La reference de base est toujours dans la hierarchie
	eventTypeHierachy.push_back(idEventSource);
	// Si la idReferenceSource = 0 On renvoi toujours ok. cad que l'on considère idRefrenceTarget dans la hierarchie
	if (idEventSource.Identifier.Numeric == 0)
		uStatus = OpcUa_Good;
	else
	{
		// remplissage de la hierarchy des references 
		uStatus = FillEventTypeNodeIdHierachy(idEventSource, &eventTypeHierachy);
		if (uStatus == OpcUa_Good)
		{
			uStatus = OpcUa_BadInvalidArgument;
			for (OpcUa_UInt32 ii = 0; ii < eventTypeHierachy.size(); ii++)
			{
				OpcUa_NodeId aNodeId = eventTypeHierachy.at(ii);
				if (Utils::IsEqual(&aNodeId, &idEventTarget))
				{
					uStatus = OpcUa_Good;
					break;
				}
			}
		}
	}
	return uStatus;
}

void CUAInformationModel::AddNamespaceUri(CNamespaceUri* aNamespaceUri)
{
	if (aNamespaceUri)
	{
		m_NamespaceUris.push_back(aNamespaceUri);
		OpcUa_UInt32 iCurrentIndex = m_NamespaceUris.size();
		if (iCurrentIndex == 0)
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critical Error:Uri error> Please Check your configuration or contact Michel Condemine\n");
		else
			aNamespaceUri->SetAbsoluteIndex(iCurrentIndex - 1);
	}
}

/// <summary>
/// Gets the namespaceUri index for a specified URI string.
/// </summary>
/// <param name="aNamespaceUri">a namespace URI.</param>
/// <param name="pIndex">Index of the p.</param>
/// <returns></returns>
OpcUa_StatusCode CUAInformationModel::GetNamespaceUri(OpcUa_String aNamespaceUri, OpcUa_Int32* pIndex)
{
	*pIndex = -1;
	int ii = 0;
	for (OpcUa_UInt16 iii = 0; iii<m_NamespaceUris.size(); iii++)
	{
		CNamespaceUri* pLocalNameSpaceUri = m_NamespaceUris.at(iii);
		if (pLocalNameSpaceUri)
		{
			OpcUa_String* pString = pLocalNameSpaceUri->GetUriName();
			if (pString)
			{
				if (OpcUa_String_Compare(&aNamespaceUri, pString) == 0)
				{
					*pIndex = ii;
					return OpcUa_Good;
				}
			}
		}
		ii++;
	}
	return OpcUa_Bad;
}


CNamespaceUri* CUAInformationModel::GetNamespaceUri(OpcUa_UInt32 index)
{
	if (m_NamespaceUris.size()>index)
		return m_NamespaceUris[index];
	else
	{
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "GetNamespaceUri>IndexRangeInvalid:%u\n", index);
		return OpcUa_Null;
	}
}

// Add System nodes . Those nodes  are not loaded through nodeset files.
/// <summary>
/// Add System nodes . Those nodes  are not loaded through nodeset files..
/// </summary>
/// <returns></returns>
OpcUa_StatusCode CUAInformationModel::AddinternalOpenOpcUaSystemNodes()
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	// Ils sont tous défini dans le namespace du server local ns=1;
	CUAObjectTypeList* pUAObjectTypeList = GetObjectTypeList();

	// Node for typedefinition ns=1;i=10 (OpenOpcUaSystemType)
	OpcUa_NodeId aNodeId;
	OpcUa_NodeId* pNodeId = OpcUa_Null;
	OpcUa_QualifiedName aName;
	CUAObject* pUAObject = OpcUa_Null;
	CUAObjectType* pUAObjectType=OpcUa_Null;
	CUAVariable* pUAVariable = OpcUa_Null;
	CUAReferenceList* pReferences = OpcUa_Null;
	CUAReference* pReference = OpcUa_Null;
	OpcUa_LocalizedText aLocalizedText;
	////////////////////////////////////////////////////////////////
	//
	//ns=1;i=10 OpenOpcUaSystemType
	//
	{
		pUAObjectType = new CUAObjectType();
		//BrowseName
		OpcUa_QualifiedName_Initialize(&aName);
		aName.NamespaceIndex = 1;
		OpcUa_String_AttachCopy(&(aName.Name), "OpenOpcUaSystemType");
		pUAObjectType->SetBrowseName(&aName);
		OpcUa_QualifiedName_Clear(&aName);

		// Description
		OpcUa_LocalizedText_Initialize(&aLocalizedText);
		OpcUa_String_AttachCopy(&(aLocalizedText.Locale), "en-us");
		OpcUa_String_AttachCopy(&(aLocalizedText.Text), "ObjectType for th root of the System's OpenOpcUaCoreServer Nodes");
		pUAObjectType->SetDescription(aLocalizedText);
		OpcUa_LocalizedText_Clear(&aLocalizedText);
		// DisplayName
		OpcUa_LocalizedText_Initialize(&aLocalizedText);
		OpcUa_String_AttachCopy(&(aLocalizedText.Locale), "en-us");
		OpcUa_String_AttachCopy(&(aLocalizedText.Text), "OpenOpcUaSystemType");
		pUAObjectType->SetDisplayName(&aLocalizedText);
		OpcUa_LocalizedText_Clear(&aLocalizedText);
		//
		pUAObjectType->SetNodeClass(OpcUa_NodeClass_ObjectType);
		// NodeId (ns=1;i=1)
		OpcUa_NodeId_Initialize(&aNodeId);
		aNodeId.Identifier.Numeric = 10;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 1;
		pUAObjectType->SetNodeId(aNodeId);
		OpcUa_NodeId_Clear(&aNodeId);
		// TypeDefinition
		//aNodeId.Identifier.Numeric = OpcUaId_BaseObjectType;
		//aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		//aNodeId.NamespaceIndex = 0;
		pUAObjectType->SetTypeDefinition(&aNodeId);
		OpcUa_NodeId_Clear(&aNodeId);
		// WriteMask
		pUAObjectType->SetWriteMask(OpcUa_AccessLevels_CurrentRead);
		// UserWriteMask
		pUAObjectType->SetUserWriteMask(OpcUa_AccessLevels_CurrentRead);
		// References 
		// First one [BaseObjectType HasSubtype pUAObjectType]
		pReference = new CUAReference();
		pReference->SetInverse(OpcUa_True);
		aNodeId.Identifier.Numeric = OpcUaId_HasSubtype;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 0;
		pReference->SetReferenceTypeId(&aNodeId);
		OpcUa_NodeId_Clear(&aNodeId);
		aNodeId.Identifier.Numeric = OpcUaId_BaseObjectType;
		pReference->SetTargetId(&aNodeId);
		pReferences = pUAObjectType->GetReferenceNodeList();
		pReferences->push_back(pReference);
		// Add to ObjectTypeList
		pUAObjectTypeList->push_back(pUAObjectType);
	}
	////////////////////////////////////////////////////////////////
	//ns=1;i=1 (OpenOpcUaSystem)
	{
		// Ils seront tous fils de 
		pUAObject = new CUAObject();
		// BrowseName
		OpcUa_QualifiedName_Initialize(&aName);
		aName.NamespaceIndex = 1;
		OpcUa_String_AttachCopy(&(aName.Name), "OpenOpcUaSystem");
		pUAObject->SetBrowseName(&aName);
		OpcUa_QualifiedName_Clear(&aName);
		// Description
		OpcUa_LocalizedText_Initialize(&aLocalizedText);
		OpcUa_String_AttachCopy(&(aLocalizedText.Locale), "en-us");
		OpcUa_String_AttachCopy(&(aLocalizedText.Text), "Root of the System's OpenOpcUaCoreServer Nodes");
		pUAObject->SetDescription(aLocalizedText);
		OpcUa_LocalizedText_Clear(&aLocalizedText);
		// DisplayName
		OpcUa_LocalizedText_Initialize(&aLocalizedText);
		OpcUa_String_AttachCopy(&(aLocalizedText.Locale), "en-us");
		OpcUa_String_AttachCopy(&(aLocalizedText.Text), "OpenOpcUaSystem");
		pUAObject->SetDisplayName(&aLocalizedText);
		OpcUa_LocalizedText_Clear(&aLocalizedText);
		// NodeClass
		pUAObject->SetNodeClass(OpcUa_NodeClass_Object);
		// NodeId (ns=1;i=1)
		OpcUa_NodeId_Initialize(&aNodeId);
		aNodeId.Identifier.Numeric = 1;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 1;
		pUAObject->SetNodeId(aNodeId);
		OpcUa_NodeId_Clear(&aNodeId);
		// TypeDefinition
		pNodeId = pUAObjectType->GetNodeId();
		pUAObject->SetTypeDefinition(pNodeId);
		// WriteMask
		pUAObject->SetWriteMask(OpcUa_AccessLevels_CurrentRead);
		// UserWriteMask
		pUAObject->SetUserWriteMask(OpcUa_AccessLevels_CurrentRead);
		// References
		pReferences = pUAObject->GetReferenceNodeList();
		// HasTypeDefinition
		pReference = new CUAReference();
		pReference->SetInverse(OpcUa_True);
		aNodeId.Identifier.Numeric = OpcUaId_HasTypeDefinition;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 0;
		pReference->SetReferenceTypeId(&aNodeId);
		OpcUa_NodeId_Clear(&aNodeId);
		aNodeId = pUAObject->GetTypeDefinition();
		pReference->SetTargetId(&aNodeId);
		pReferences->push_back(pReference);
		// Organizes
		pReference = new CUAReference();
		pReference->SetInverse(OpcUa_True);
		aNodeId.Identifier.Numeric = OpcUaId_Organizes;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 0;
		pReference->SetReferenceTypeId(&aNodeId);
		OpcUa_NodeId_Clear(&aNodeId);
		aNodeId.Identifier.Numeric = OpcUaId_ObjectsFolder;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 0;
		pReference->SetTargetId(&aNodeId);
		pReferences->push_back(pReference);
		//// Add in the AddressSpace
		PopulateInformationModel(pUAObject);
	}
	
	//////////////////////////////////////////////////////////////////
	// Object reltif à la trace ns=1;i=2 (TRACE)
	{
		pUAObject = new CUAObject();
		// BrowseName
		OpcUa_QualifiedName_Initialize(&aName);
		aName.NamespaceIndex = 1;
		OpcUa_String_AttachCopy(&(aName.Name), "Trace");
		pUAObject->SetBrowseName(&aName);
		OpcUa_QualifiedName_Clear(&aName);
		// Description
		OpcUa_LocalizedText_Initialize(&aLocalizedText);
		OpcUa_String_AttachCopy(&(aLocalizedText.Locale), "en-us");
		OpcUa_String_AttachCopy(&(aLocalizedText.Text), "Trace object");
		pUAObject->SetDescription(aLocalizedText);
		OpcUa_LocalizedText_Clear(&aLocalizedText);
		// DisplayName
		OpcUa_LocalizedText_Initialize(&aLocalizedText);
		OpcUa_String_AttachCopy(&(aLocalizedText.Locale), "en-us");
		OpcUa_String_AttachCopy(&(aLocalizedText.Text), "Trace");
		pUAObject->SetDisplayName(&aLocalizedText);
		OpcUa_LocalizedText_Clear(&aLocalizedText);
		// NodeClass
		pUAObject->SetNodeClass(OpcUa_NodeClass_Object);
		// NodeId (ns=1;i=2)
		OpcUa_NodeId_Initialize(&aNodeId);
		aNodeId.Identifier.Numeric = 2;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 1;
		pUAObject->SetNodeId(aNodeId);
		// TypeDefinition
		OpcUa_NodeId_Clear(&aNodeId);
		pNodeId = pUAObjectType->GetNodeId();
		pUAObject->SetTypeDefinition(pNodeId);
		// WriteMask
		pUAObject->SetWriteMask(OpcUa_AccessLevels_CurrentRead);
		// UserWriteMask
		pUAObject->SetUserWriteMask(OpcUa_AccessLevels_CurrentRead);
		// References
		pReferences = pUAObject->GetReferenceNodeList();
		// Trace HasTypeDefinition BaseObjectType
		pReference = new CUAReference();
		pReference->SetInverse(OpcUa_True);
		aNodeId.Identifier.Numeric = OpcUaId_HasTypeDefinition;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 0;
		pReference->SetReferenceTypeId(&aNodeId);
		OpcUa_NodeId_Clear(&aNodeId);
		aNodeId.Identifier.Numeric = OpcUaId_BaseObjectType;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 0;
		pReference->SetTargetId(&aNodeId);
		pReferences->push_back(pReference);

		// OpenOpcUaSystem HasComponent Trace
		pReference = new CUAReference();
		pReference->SetInverse(OpcUa_True);
		OpcUa_NodeId_Clear(&aNodeId);
		aNodeId.Identifier.Numeric = OpcUaId_HasComponent;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 0;
		pReference->SetReferenceTypeId(&aNodeId);
		OpcUa_NodeId_Clear(&aNodeId);
		aNodeId.Identifier.Numeric = 1;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 1;
		pReference->SetTargetId(&aNodeId);
		pReferences->push_back(pReference);
		//// Add in the AddressSpace
		PopulateInformationModel(pUAObject);
	}
	//////////////////////////////////////////////////////////////////
	// Variable indiquant le niveau de trace requit ns=1;i=3 (TraceLevel)
	// Internaly this is m_tConfiguration.uProxyStub_Trace_Level
	{
		pUAVariable = new CUAVariable();
		// BrowseName
		OpcUa_QualifiedName_Initialize(&aName);
		aName.NamespaceIndex = 1;
		OpcUa_String_AttachCopy(&(aName.Name), "TraceLevel");
		pUAVariable->SetBrowseName(&aName);
		OpcUa_QualifiedName_Clear(&aName);
		// Description
		OpcUa_LocalizedText_Initialize(&aLocalizedText);
		OpcUa_String_AttachCopy(&(aLocalizedText.Locale), "en-us");
		OpcUa_String_AttachCopy(&(aLocalizedText.Text), "Trace Level require for this server");
		pUAVariable->SetDescription(aLocalizedText);
		OpcUa_LocalizedText_Clear(&aLocalizedText);
		// DisplayName
		OpcUa_LocalizedText_Initialize(&aLocalizedText);
		OpcUa_String_AttachCopy(&(aLocalizedText.Locale), "en-us");
		OpcUa_String_AttachCopy(&(aLocalizedText.Text), "TraceLevel");
		pUAVariable->SetDisplayName(&aLocalizedText);
		OpcUa_LocalizedText_Clear(&aLocalizedText);
		// NodeClass
		pUAVariable->SetNodeClass(OpcUa_NodeClass_Variable);
		// NodeId (ns=1;i=3)
		OpcUa_NodeId_Initialize(&aNodeId);
		aNodeId.Identifier.Numeric = 3;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 1;
		pUAVariable->SetNodeId(aNodeId);
		// TypeDefinition
		OpcUa_NodeId_Clear(&aNodeId);
		aNodeId.Identifier.Numeric = OpcUaId_BaseVariableType;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 0;
		pUAVariable->SetTypeDefinition(&aNodeId);
		// WriteMask
		pUAVariable->SetWriteMask(OpcUa_AccessLevels_CurrentRead);
		// UserWriteMask
		pUAVariable->SetUserWriteMask(OpcUa_AccessLevels_CurrentRead);
		// AccessLevel
		pUAVariable->SetAccessLevel(OpcUa_AccessLevels_CurrentRead | OpcUa_AccessLevels_CurrentWrite);
		// UserAccessLevel
		pUAVariable->SetUserAccessLevel(OpcUa_AccessLevels_CurrentRead | OpcUa_AccessLevels_CurrentWrite);
		// BuiltInType
		pUAVariable->SetBuiltInType(OpcUaType_UInt32);
		// DataType
		OpcUa_NodeId_Clear(&aNodeId);
		aNodeId.Identifier.Numeric = OpcUaType_UInt32;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 0;
		pUAVariable->SetDataType(aNodeId);
		// Historizing
		pUAVariable->SetHistorizing(OpcUa_False);
		pUAVariable->SetMinimumSamplingInterval(5000);

		// References
		pReferences = pUAVariable->GetReferenceNodeList();
		// TraceLevel HasTypeDefinition BaseVariableType
		pReference = new CUAReference();
		pReference->SetInverse(OpcUa_True);
		aNodeId.Identifier.Numeric = OpcUaId_HasTypeDefinition;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 0;
		pReference->SetReferenceTypeId(&aNodeId);
		OpcUa_NodeId_Clear(&aNodeId);
		aNodeId.Identifier.Numeric = OpcUaId_BaseVariableType;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 0;
		pReference->SetTargetId(&aNodeId);
		pReferences->push_back(pReference);
		// OpenOpcUaSystem HasComponent Trace
		pReference = new CUAReference();
		pReference->SetInverse(OpcUa_True);
		OpcUa_NodeId_Clear(&aNodeId);
		aNodeId.Identifier.Numeric = OpcUaId_HasComponent;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 0;
		pReference->SetReferenceTypeId(&aNodeId);
		OpcUa_NodeId_Clear(&aNodeId);
		aNodeId.Identifier.Numeric = 2;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 1;
		pReference->SetTargetId(&aNodeId);
		pReferences->push_back(pReference);
		//// Add in the AddressSpace
		//pUAVariableList->push_back(pUAVariable);
		uStatus = PopulateInformationModel(pUAVariable);
		if (uStatus != OpcUa_Good)
			delete pUAVariable;
		else
		{
			// Initiale value from the configuration file
			CDataValue* pDataValue = pUAVariable->GetValue();
			if (!pDataValue)
			{
				pDataValue = new CDataValue(pUAVariable->GetBuiltInType());
				pUAVariable->SetValue(pDataValue);
				delete pDataValue;
			}
			pDataValue = pUAVariable->GetValue();
			OpcUa_Variant aVariant = pDataValue->GetValue();
			OpcUa_Variant_Initialize(&aVariant);
			aVariant.Datatype = OpcUaType_UInt32;
			aVariant.Value.UInt32 = g_pTheApplication->GetTraceLevel();
			pUAVariable->SetValue(aVariant);
			OpcUa_Variant_Clear(&aVariant);
			pDataValue->SetStatusCode(OpcUa_Good);
			pDataValue->SetServerTimestamp(OpcUa_DateTime_UtcNow());
			pDataValue->SetSourceTimestamp(OpcUa_DateTime_UtcNow());
		}
	}
	//////////////////////////////////////////////////////////////////
	//
	// Variable indiquant le niveau de trace requit ns=1;i=3 (TraceLevel)
	{
		// Internaly this is m_tConfiguration.uProxyStub_Trace_Level
		pUAVariable = new CUAVariable();
		// BrowseName
		OpcUa_QualifiedName_Initialize(&aName);
		aName.NamespaceIndex = 1;
		OpcUa_String_AttachCopy(&(aName.Name), "TraceOutput");
		pUAVariable->SetBrowseName(&aName);
		OpcUa_QualifiedName_Clear(&aName);
		// Description
		OpcUa_LocalizedText_Initialize(&aLocalizedText);
		OpcUa_String_AttachCopy(&(aLocalizedText.Locale), "en-us");
		OpcUa_String_AttachCopy(&(aLocalizedText.Text), "Trace output used by this server");
		pUAVariable->SetDescription(aLocalizedText);
		OpcUa_LocalizedText_Clear(&aLocalizedText);
		// DisplayName
		OpcUa_LocalizedText_Initialize(&aLocalizedText);
		OpcUa_String_AttachCopy(&(aLocalizedText.Locale), "en-us");
		OpcUa_String_AttachCopy(&(aLocalizedText.Text), "TraceOutput");
		pUAVariable->SetDisplayName(&aLocalizedText);
		OpcUa_LocalizedText_Clear(&aLocalizedText);
		// NodeClass
		pUAVariable->SetNodeClass(OpcUa_NodeClass_Variable);
		// NodeId (ns=1;i=3)
		OpcUa_NodeId_Initialize(&aNodeId);
		aNodeId.Identifier.Numeric = 4;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 1;
		pUAVariable->SetNodeId(aNodeId);
		// TypeDefinition
		OpcUa_NodeId_Clear(&aNodeId);
		aNodeId.Identifier.Numeric = OpcUaId_BaseVariableType;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 0;
		pUAVariable->SetTypeDefinition(&aNodeId);
		// WriteMask
		pUAVariable->SetWriteMask(OpcUa_AccessLevels_CurrentRead);
		// UserWriteMask
		pUAVariable->SetUserWriteMask(OpcUa_AccessLevels_CurrentRead);
		// AccessLevel
		pUAVariable->SetAccessLevel(OpcUa_AccessLevels_CurrentRead | OpcUa_AccessLevels_CurrentWrite);
		// UserAccessLevel
		pUAVariable->SetUserAccessLevel(OpcUa_AccessLevels_CurrentRead | OpcUa_AccessLevels_CurrentWrite);
		// BuiltInType
		pUAVariable->SetBuiltInType(OpcUaType_UInt32);
		// DataType
		OpcUa_NodeId_Clear(&aNodeId);
		aNodeId.Identifier.Numeric = OpcUaType_UInt32;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 0;
		pUAVariable->SetDataType(aNodeId);
		// Historizing
		pUAVariable->SetHistorizing(OpcUa_False);
		pUAVariable->SetMinimumSamplingInterval(5000);

		// References
		pReferences = pUAVariable->GetReferenceNodeList();
		// TraceLevel HasTypeDefinition BaseVariableType
		pReference = new CUAReference();
		pReference->SetInverse(OpcUa_True);
		aNodeId.Identifier.Numeric = OpcUaId_HasTypeDefinition;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 0;
		pReference->SetReferenceTypeId(&aNodeId);
		OpcUa_NodeId_Clear(&aNodeId);
		aNodeId.Identifier.Numeric = OpcUaId_BaseVariableType;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 0;
		pReference->SetTargetId(&aNodeId);
		pReferences->push_back(pReference);
		// OpenOpcUaSystem HasComponent Trace
		pReference = new CUAReference();
		pReference->SetInverse(OpcUa_True);
		OpcUa_NodeId_Clear(&aNodeId);
		aNodeId.Identifier.Numeric = OpcUaId_HasComponent;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 0;
		pReference->SetReferenceTypeId(&aNodeId);
		OpcUa_NodeId_Clear(&aNodeId);
		aNodeId.Identifier.Numeric = 2;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 1;
		pReference->SetTargetId(&aNodeId);
		pReferences->push_back(pReference);
		//// Add in the AddressSpace
		//pUAVariableList->push_back(pUAVariable);
		uStatus = PopulateInformationModel(pUAVariable);
		if (uStatus != OpcUa_Good)
			delete pUAVariable;
		else
		{
			// Initiale value from the configuration file
			CDataValue* pDataValue = pUAVariable->GetValue();
			if (!pDataValue)
			{
				pDataValue = new CDataValue(pUAVariable->GetBuiltInType());
				pUAVariable->SetValue(pDataValue);
				delete pDataValue;
			}
			pDataValue = pUAVariable->GetValue();
			OpcUa_Variant aVariant = pDataValue->GetValue();
			OpcUa_Variant_Initialize(&aVariant);
			aVariant.Datatype = OpcUaType_UInt32;
			aVariant.Value.UInt32 = g_pTheApplication->GetTraceOutput();
			pUAVariable->SetValue(aVariant);
			OpcUa_Variant_Clear(&aVariant);
			pDataValue->SetStatusCode(OpcUa_Good);
			pDataValue->SetServerTimestamp(OpcUa_DateTime_UtcNow());
			pDataValue->SetSourceTimestamp(OpcUa_DateTime_UtcNow());
		}
	}
	
	//////////////////////////////////////////////////////////////////
	// Object relatif au LDS ns=1;i=5 (LDS)
	{
		pUAObject = new CUAObject();
		// BrowseName
		OpcUa_QualifiedName_Initialize(&aName);
		aName.NamespaceIndex = 1;
		OpcUa_String_AttachCopy(&(aName.Name), "LDS");
		pUAObject->SetBrowseName(&aName);
		OpcUa_QualifiedName_Clear(&aName);
		// Description
		OpcUa_LocalizedText_Initialize(&aLocalizedText);
		OpcUa_String_AttachCopy(&(aLocalizedText.Locale), "en-us");
		OpcUa_String_AttachCopy(&(aLocalizedText.Text), "LDS object");
		pUAObject->SetDescription(aLocalizedText);
		OpcUa_LocalizedText_Clear(&aLocalizedText);
		// DisplayName
		OpcUa_LocalizedText_Initialize(&aLocalizedText);
		OpcUa_String_AttachCopy(&(aLocalizedText.Locale), "en-us");
		OpcUa_String_AttachCopy(&(aLocalizedText.Text), "LDS");
		pUAObject->SetDisplayName(&aLocalizedText);
		OpcUa_LocalizedText_Clear(&aLocalizedText);
		// NodeClass
		pUAObject->SetNodeClass(OpcUa_NodeClass_Object);
		// NodeId (ns=1;i=2)
		OpcUa_NodeId_Initialize(&aNodeId);
		aNodeId.Identifier.Numeric = 5;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 1;
		pUAObject->SetNodeId(aNodeId);
		// TypeDefinition
		OpcUa_NodeId_Clear(&aNodeId);
		pNodeId = pUAObjectType->GetNodeId();
		pUAObject->SetTypeDefinition(pNodeId);
		// WriteMask
		pUAObject->SetWriteMask(OpcUa_AccessLevels_CurrentRead);
		// UserWriteMask
		pUAObject->SetUserWriteMask(OpcUa_AccessLevels_CurrentRead);
		// References
		pReferences = pUAObject->GetReferenceNodeList();
		// Trace HasTypeDefinition BaseObjectType
		pReference = new CUAReference();
		pReference->SetInverse(OpcUa_True);
		aNodeId.Identifier.Numeric = OpcUaId_HasTypeDefinition;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 0;
		pReference->SetReferenceTypeId(&aNodeId);
		OpcUa_NodeId_Clear(&aNodeId);
		aNodeId.Identifier.Numeric = OpcUaId_BaseObjectType;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 0;
		pReference->SetTargetId(&aNodeId);
		pReferences->push_back(pReference);

		// OpenOpcUaSystem HasComponent LDS
		pReference = new CUAReference();
		pReference->SetInverse(OpcUa_True);
		OpcUa_NodeId_Clear(&aNodeId);
		aNodeId.Identifier.Numeric = OpcUaId_HasComponent;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 0;
		pReference->SetReferenceTypeId(&aNodeId);
		OpcUa_NodeId_Clear(&aNodeId);
		aNodeId.Identifier.Numeric = 1;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 1;
		pReference->SetTargetId(&aNodeId);
		pReferences->push_back(pReference);
		//// Add in the AddressSpace
		PopulateInformationModel(pUAObject);
	}
	{
		// LDS Registration is Active
		pUAVariable = new CUAVariable();
		// BrowseName
		OpcUa_QualifiedName_Initialize(&aName);
		aName.NamespaceIndex = 1;
		OpcUa_String_AttachCopy(&(aName.Name), "RegistrationActive");
		pUAVariable->SetBrowseName(&aName);
		OpcUa_QualifiedName_Clear(&aName);
		// Description
		OpcUa_LocalizedText_Initialize(&aLocalizedText);
		OpcUa_String_AttachCopy(&(aLocalizedText.Locale), "en-us");
		OpcUa_String_AttachCopy(&(aLocalizedText.Text), "RegistrationActive on this server");
		pUAVariable->SetDescription(aLocalizedText);
		OpcUa_LocalizedText_Clear(&aLocalizedText);
		// DisplayName
		OpcUa_LocalizedText_Initialize(&aLocalizedText);
		OpcUa_String_AttachCopy(&(aLocalizedText.Locale), "en-us");
		OpcUa_String_AttachCopy(&(aLocalizedText.Text), "RegistrationActive");
		pUAVariable->SetDisplayName(&aLocalizedText);
		OpcUa_LocalizedText_Clear(&aLocalizedText);
		// PData
		pUAVariable->SetPData(OpcUa_Null);
		// NodeClass
		pUAVariable->SetNodeClass(OpcUa_NodeClass_Variable);
		// NodeId (ns=1;i=3)
		OpcUa_NodeId_Initialize(&aNodeId);
		aNodeId.Identifier.Numeric = 6;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 1;
		pUAVariable->SetNodeId(aNodeId);
		// TypeDefinition
		OpcUa_NodeId_Clear(&aNodeId);
		aNodeId.Identifier.Numeric = OpcUaId_BaseVariableType;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 0;
		pUAVariable->SetTypeDefinition(&aNodeId);
		// WriteMask
		pUAVariable->SetWriteMask(OpcUa_AccessLevels_CurrentRead);
		// UserWriteMask
		pUAVariable->SetUserWriteMask(OpcUa_AccessLevels_CurrentRead);
		// AccessLevel
		pUAVariable->SetAccessLevel(OpcUa_AccessLevels_CurrentRead | OpcUa_AccessLevels_CurrentWrite);
		// UserAccessLevel
		pUAVariable->SetUserAccessLevel(OpcUa_AccessLevels_CurrentRead | OpcUa_AccessLevels_CurrentWrite);
		// BuiltInType
		pUAVariable->SetBuiltInType(OpcUaType_Boolean);
		// DataType
		OpcUa_NodeId_Clear(&aNodeId);
		aNodeId.Identifier.Numeric = OpcUaType_Boolean;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 0;
		pUAVariable->SetDataType(aNodeId);
		// Historizing
		pUAVariable->SetHistorizing(OpcUa_False);
		pUAVariable->SetMinimumSamplingInterval(5000);

		// References
		pReferences = pUAVariable->GetReferenceNodeList();
		// TraceLevel HasTypeDefinition BaseVariableType
		pReference = new CUAReference();
		pReference->SetInverse(OpcUa_True);
		aNodeId.Identifier.Numeric = OpcUaId_HasTypeDefinition;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 0;
		pReference->SetReferenceTypeId(&aNodeId);
		OpcUa_NodeId_Clear(&aNodeId);
		aNodeId.Identifier.Numeric = OpcUaId_BaseVariableType;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 0;
		pReference->SetTargetId(&aNodeId);
		pReferences->push_back(pReference);
		// OpenOpcUaSystem HasComponent Trace
		pReference = new CUAReference();
		pReference->SetInverse(OpcUa_True);
		OpcUa_NodeId_Clear(&aNodeId);
		aNodeId.Identifier.Numeric = OpcUaId_HasComponent;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 0;
		pReference->SetReferenceTypeId(&aNodeId);
		OpcUa_NodeId_Clear(&aNodeId);
		aNodeId.Identifier.Numeric = 5;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 1;
		pReference->SetTargetId(&aNodeId);
		pReferences->push_back(pReference);
		//// Add in the AddressSpace
		uStatus = PopulateInformationModel(pUAVariable);
		if (uStatus != OpcUa_Good)
			delete pUAVariable;
		else
		{
			// Initiale value from the configuration file
			CDataValue* pDataValue = pUAVariable->GetValue();
			if (!pDataValue)
			{
				pDataValue = new CDataValue(pUAVariable->GetBuiltInType());
				pUAVariable->SetValue(pDataValue);
				delete pDataValue;
			}
			pDataValue = pUAVariable->GetValue();
			OpcUa_Variant aVariant = pDataValue->GetValue();
			OpcUa_Variant_Initialize(&aVariant);
			aVariant.Datatype = OpcUaType_Boolean;
			aVariant.Value.Boolean = g_pTheApplication->IsLDSRegistrationActive();
			pUAVariable->SetValue(aVariant);
			OpcUa_Variant_Clear(&aVariant);
			pDataValue->SetStatusCode(OpcUa_Good);
			pDataValue->SetServerTimestamp(OpcUa_DateTime_UtcNow());
			pDataValue->SetSourceTimestamp(OpcUa_DateTime_UtcNow());
		}
	}
	
	{
		// LDS Registration is Active
		pUAVariable = new CUAVariable();
		// BrowseName
		OpcUa_QualifiedName_Initialize(&aName);
		aName.NamespaceIndex = 1;
		OpcUa_String_AttachCopy(&(aName.Name), "RegistrationInterval");
		pUAVariable->SetBrowseName(&aName);
		OpcUa_QualifiedName_Clear(&aName);
		// Description
		OpcUa_LocalizedText_Initialize(&aLocalizedText);
		OpcUa_String_AttachCopy(&(aLocalizedText.Locale), "en-us");
		OpcUa_String_AttachCopy(&(aLocalizedText.Text), "Registration Interval to the LDS");
		pUAVariable->SetDescription(aLocalizedText);
		OpcUa_LocalizedText_Clear(&aLocalizedText);
		// DisplayName
		OpcUa_LocalizedText_Initialize(&aLocalizedText);
		OpcUa_String_AttachCopy(&(aLocalizedText.Locale), "en-us");
		OpcUa_String_AttachCopy(&(aLocalizedText.Text), "RegistrationInterval");
		pUAVariable->SetDisplayName(&aLocalizedText);
		OpcUa_LocalizedText_Clear(&aLocalizedText);
		// PData
		pUAVariable->SetPData(OpcUa_Null);
		// NodeClass
		pUAVariable->SetNodeClass(OpcUa_NodeClass_Variable);
		// NodeId (ns=1;i=3)
		OpcUa_NodeId_Initialize(&aNodeId);
		aNodeId.Identifier.Numeric = 7;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 1;
		pUAVariable->SetNodeId(aNodeId);
		// TypeDefinition
		OpcUa_NodeId_Clear(&aNodeId);
		aNodeId.Identifier.Numeric = OpcUaId_BaseVariableType;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 0;
		pUAVariable->SetTypeDefinition(&aNodeId);
		// WriteMask
		pUAVariable->SetWriteMask(OpcUa_AccessLevels_CurrentRead);
		// UserWriteMask
		pUAVariable->SetUserWriteMask(OpcUa_AccessLevels_CurrentRead);
		// AccessLevel
		pUAVariable->SetAccessLevel(OpcUa_AccessLevels_CurrentRead | OpcUa_AccessLevels_CurrentWrite);
		// UserAccessLevel
		pUAVariable->SetUserAccessLevel(OpcUa_AccessLevels_CurrentRead | OpcUa_AccessLevels_CurrentWrite);
		// BuiltInType
		pUAVariable->SetBuiltInType(OpcUaType_UInt32);
		// DataType
		OpcUa_NodeId_Clear(&aNodeId);
		aNodeId.Identifier.Numeric = OpcUaType_UInt32;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 0;
		pUAVariable->SetDataType(aNodeId);
		// Historizing
		pUAVariable->SetHistorizing(OpcUa_False);
		pUAVariable->SetMinimumSamplingInterval(5000);

		// References
		pReferences = pUAVariable->GetReferenceNodeList();
		// TraceLevel HasTypeDefinition BaseVariableType
		pReference = new CUAReference();
		pReference->SetInverse(OpcUa_True);
		aNodeId.Identifier.Numeric = OpcUaId_HasTypeDefinition;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 0;
		pReference->SetReferenceTypeId(&aNodeId);
		OpcUa_NodeId_Clear(&aNodeId);
		aNodeId.Identifier.Numeric = OpcUaId_BaseVariableType;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 0;
		pReference->SetTargetId(&aNodeId);
		pReferences->push_back(pReference);
		// OpenOpcUaSystem HasComponent Trace
		pReference = new CUAReference();
		pReference->SetInverse(OpcUa_True);
		OpcUa_NodeId_Clear(&aNodeId);
		aNodeId.Identifier.Numeric = OpcUaId_HasComponent;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 0;
		pReference->SetReferenceTypeId(&aNodeId);
		OpcUa_NodeId_Clear(&aNodeId);
		aNodeId.Identifier.Numeric = 5;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 1;
		pReference->SetTargetId(&aNodeId);
		pReferences->push_back(pReference);
		//// Add in the AddressSpace
		uStatus = PopulateInformationModel(pUAVariable);
		if (uStatus != OpcUa_Good)
			delete pUAVariable;
		else
		{
			// Initiale value from the configuration file
			CDataValue* pDataValue = pUAVariable->GetValue();
			if (!pDataValue)
			{
				pDataValue = new CDataValue(pUAVariable->GetBuiltInType());
				pUAVariable->SetValue(pDataValue);
				delete pDataValue;
			}
			pDataValue = pUAVariable->GetValue();
			OpcUa_Variant aVariant = pDataValue->GetValue();
			OpcUa_Variant_Initialize(&aVariant);
			aVariant.Datatype = OpcUaType_UInt32;
			aVariant.Value.UInt32 = g_pTheApplication->GetLDSRegistrationInterval();
			pUAVariable->SetValue(aVariant);
			OpcUa_Variant_Clear(&aVariant);
			pDataValue->SetStatusCode(OpcUa_Good);
			pDataValue->SetServerTimestamp(OpcUa_DateTime_UtcNow());
			pDataValue->SetSourceTimestamp(OpcUa_DateTime_UtcNow());
		}
	}
	// Shutdown Node
	// This Node allows to stop the server on a client request.
	{
		// LDS Registration is Active
		pUAVariable = new CUAVariable();
		// BrowseName
		OpcUa_QualifiedName_Initialize(&aName);
		aName.NamespaceIndex = 1;
		OpcUa_String_AttachCopy(&(aName.Name), "Shutdown");
		pUAVariable->SetBrowseName(&aName);
		OpcUa_QualifiedName_Clear(&aName);
		// Description
		OpcUa_LocalizedText_Initialize(&aLocalizedText);
		OpcUa_String_AttachCopy(&(aLocalizedText.Locale), "en-us");
		OpcUa_String_AttachCopy(&(aLocalizedText.Text), "Node to require a Shutdown ");
		pUAVariable->SetDescription(aLocalizedText);
		OpcUa_LocalizedText_Clear(&aLocalizedText);
		// DisplayName
		OpcUa_LocalizedText_Initialize(&aLocalizedText);
		OpcUa_String_AttachCopy(&(aLocalizedText.Locale), "en-us");
		OpcUa_String_AttachCopy(&(aLocalizedText.Text), "Shutdown");
		pUAVariable->SetDisplayName(&aLocalizedText);
		OpcUa_LocalizedText_Clear(&aLocalizedText);
		// PData
		pUAVariable->SetPData(OpcUa_Null);
		// NodeClass
		pUAVariable->SetNodeClass(OpcUa_NodeClass_Variable);
		// NodeId (ns=1;i=3)
		OpcUa_NodeId_Initialize(&aNodeId);
		aNodeId.Identifier.Numeric = 8;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 1;
		pUAVariable->SetNodeId(aNodeId);
		// TypeDefinition
		OpcUa_NodeId_Clear(&aNodeId);
		aNodeId.Identifier.Numeric = OpcUaId_BaseVariableType;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 0;
		pUAVariable->SetTypeDefinition(&aNodeId);
		// WriteMask
		pUAVariable->SetWriteMask(OpcUa_AccessLevels_CurrentRead);
		// UserWriteMask
		pUAVariable->SetUserWriteMask(OpcUa_AccessLevels_CurrentRead);
		// AccessLevel
		pUAVariable->SetAccessLevel(OpcUa_AccessLevels_CurrentRead | OpcUa_AccessLevels_CurrentWrite);
		// UserAccessLevel
		pUAVariable->SetUserAccessLevel(OpcUa_AccessLevels_CurrentRead | OpcUa_AccessLevels_CurrentWrite);
		// BuiltInType
		pUAVariable->SetBuiltInType(OpcUaType_Boolean);
		// DataType
		OpcUa_NodeId_Clear(&aNodeId);
		aNodeId.Identifier.Numeric = OpcUaType_Boolean;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 0;
		pUAVariable->SetDataType(aNodeId);
		// Historizing
		pUAVariable->SetHistorizing(OpcUa_False);
		pUAVariable->SetMinimumSamplingInterval(5000);

		// References
		pReferences = pUAVariable->GetReferenceNodeList();
		// TraceLevel HasTypeDefinition BaseVariableType
		pReference = new CUAReference();
		pReference->SetInverse(OpcUa_True);
		aNodeId.Identifier.Numeric = OpcUaId_HasTypeDefinition;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 0;
		pReference->SetReferenceTypeId(&aNodeId);
		OpcUa_NodeId_Clear(&aNodeId);
		aNodeId.Identifier.Numeric = OpcUaId_BaseVariableType;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 0;
		pReference->SetTargetId(&aNodeId);
		pReferences->push_back(pReference);
		// OpenOpcUaSystem HasComponent Trace
		pReference = new CUAReference();
		pReference->SetInverse(OpcUa_True);
		OpcUa_NodeId_Clear(&aNodeId);
		aNodeId.Identifier.Numeric = OpcUaId_HasComponent;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 0;
		pReference->SetReferenceTypeId(&aNodeId);
		OpcUa_NodeId_Clear(&aNodeId);
		aNodeId.Identifier.Numeric = 1;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 1;
		pReference->SetTargetId(&aNodeId);
		pReferences->push_back(pReference);
		//// Add in the AddressSpace
		uStatus = PopulateInformationModel(pUAVariable);
		if (uStatus != OpcUa_Good)
			delete pUAVariable;
		else
		{
			// Initiale value from the configuration file
			CDataValue* pDataValue = pUAVariable->GetValue();
			if (!pDataValue)
			{
				pDataValue = new CDataValue(pUAVariable->GetBuiltInType());
				pUAVariable->SetValue(pDataValue);
				delete pDataValue;
			}
			pDataValue = pUAVariable->GetValue();
			OpcUa_Variant aVariant = pDataValue->GetValue();
			OpcUa_Variant_Initialize(&aVariant);
			aVariant.Datatype = OpcUaType_Boolean;
			aVariant.Value.Boolean = OpcUa_False;
			pUAVariable->SetValue(aVariant);
			OpcUa_Variant_Clear(&aVariant);
			pDataValue->SetStatusCode(OpcUa_Good);
			pDataValue->SetServerTimestamp(OpcUa_DateTime_UtcNow());
			pDataValue->SetSourceTimestamp(OpcUa_DateTime_UtcNow());
		}
	}
	
	return uStatus;
}

OpcUa_Boolean CUAInformationModel::IsEnabledServerDiagnosticsDefaultValue() 
{ 
	return m_bServerDiagnosticsDefaultValue; 
}
void CUAInformationModel::EnableServerDiagnosticsDefaultValue(OpcUa_Boolean bVal) 
{
	m_bServerDiagnosticsDefaultValue = bVal; 
}

OpcUa_UInt32 CUAInformationModel::OnLoadNamespaceUriGetSize()
{
	OpcUa_UInt32 iSize = 0;
	OpcUa_Mutex_Lock(m_OnLoadNamespaceUrisMutex);
	iSize = m_OnLoadNamespaceUris.size();
	OpcUa_Mutex_Unlock(m_OnLoadNamespaceUrisMutex);
	return iSize;
}
void CUAInformationModel::OnLoadNamespaceUriEraseAll()
{
	OpcUa_Mutex_Lock(m_OnLoadNamespaceUrisMutex);
	CUANamespaceUris::iterator it=m_OnLoadNamespaceUris.begin();
	while (it != m_OnLoadNamespaceUris.end())
	{
		char* szUri=it->first;
		if (szUri)
		{
			OpcUa_Free(szUri);
			szUri = OpcUa_Null;
		}
		++it;
	}
	m_OnLoadNamespaceUris.clear();
	OpcUa_Mutex_Unlock(m_OnLoadNamespaceUrisMutex);
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Executes the load namespace URI add URI action. </summary>
///
/// <remarks>	Michel, 20/02/2016. </remarks>
///
/// <param name="aString">	The string. </param>
/// <param name="index">  	Zero-based index of the. </param>
///-------------------------------------------------------------------------------------------------

void CUAInformationModel::OnLoadNamespaceUriAddUri(OpcUa_String aString, OpcUa_UInt32 index)
{
	OpcUa_Mutex_Lock(m_OnLoadNamespaceUrisMutex);
	OpcUa_UInt32 iSize = OpcUa_String_StrLen(&aString) ;
	if (iSize > 0)
	{
		OpcUa_CharA* szUri = (OpcUa_CharA*)OpcUa_Alloc(iSize + 1);
		ZeroMemory(szUri, iSize + 1);
		memcpy(szUri, OpcUa_String_GetRawString(&aString), iSize);		
		m_OnLoadNamespaceUris.insert(std::pair<char*, OpcUa_UInt32>(szUri, index));
	}
	OpcUa_Mutex_Unlock(m_OnLoadNamespaceUrisMutex);
}

OpcUa_StatusCode CUAInformationModel::OnLoadNamespaceUriToAbsoluteNamespaceUri(OpcUa_UInt32 iNamespace, OpcUa_Int32* pIndexNS)
{
	OpcUa_StatusCode uStatus = OpcUa_BadNotFound;
	CUANamespaceUris::iterator it;
	for (it = m_OnLoadNamespaceUris.begin(); it != m_OnLoadNamespaceUris.end(); it++)
	{
		if (it->second == iNamespace)
		{
			OpcUa_String aString;
			OpcUa_String_Initialize(&aString);
			OpcUa_String_AttachCopy(&aString, it->first);
			uStatus = GetNamespaceUri(aString, pIndexNS);
			OpcUa_String_Clear(&aString);
		}
	}
	return uStatus;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Check if an Uri is in the OnLoad NamespaceUriList. </summary>
///
/// <remarks>	Michel, 26/08/2015. </remarks>
///
/// <param name="uriName">	Name of the URI. </param>
///
/// <returns>	An OpcUa_Boolean true if the Uri is in the OnLoad NamespaceUriList false otherwise. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_Boolean CUAInformationModel::OnLoadNamespaceUriContains(OpcUa_String uriName)
{
	OpcUa_Boolean bResult = OpcUa_True;
	CUANamespaceUris::iterator it;
	it = m_OnLoadNamespaceUris.find(OpcUa_String_GetRawString(&uriName));
	if (it == m_OnLoadNamespaceUris.end())
		bResult = OpcUa_False;
	return bResult;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Updae the index of an Uri in the OnLoad NamespaceUriList. </summary>
///
/// <remarks>	Michel, 26/08/2015. </remarks>
///
/// <param name="uriName">			Name of the URI. </param>
/// <param name="uiNewIndexVal">	The new index value. </param>
///-------------------------------------------------------------------------------------------------

void CUAInformationModel::OnLoadNamespaceUriUpdateIndex(OpcUa_String uriName, OpcUa_UInt32 uiNewIndexVal)
{
	CUANamespaceUris::iterator it;
	it = m_OnLoadNamespaceUris.find(OpcUa_String_GetRawString(&uriName));
	if (it != m_OnLoadNamespaceUris.end())
		it->second = uiNewIndexVal;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	This method check a Uri exist or not in the AbsoluteNamespaceUriList. 
///				If it not exist. The Uri is added in both Absolute and OnLoad NamespaceUriList
///				When add to the AbsoluteList we create a new CNamespaceUri</summary>
///
/// <remarks>	Michel, 26/08/2015. </remarks>
///
/// <param name="aNamespaceUri">	URI of the namespace. </param>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode CUAInformationModel::OnLoadNamespaceUriVerifyUri(OpcUa_String aNamespaceUri)
{
	OpcUa_StatusCode uStatus = OpcUa_BadInvalidArgument;
	OpcUa_Int32 index = 0;
	// est ce que ce NamespaceUri est déja pris en charge ?
	if (GetNamespaceUri(aNamespaceUri, &index) == OpcUa_Bad)
	{
		CNamespaceUri* pNamespaceUri = new CNamespaceUri();
		pNamespaceUri->SetUriName(&aNamespaceUri);

		// Add the new uris in the list
		AddNamespaceUri(pNamespaceUri);
		// Search for the absolute index number set for this NamespaceUri during the AddNamespaceUri.
		if (GetNamespaceUri(aNamespaceUri, &index) == OpcUa_Good)
			pNamespaceUri->SetAbsoluteIndex(index);
		else
		{
			// mise a jour de l'index courant
			OpcUa_Int32 indexCurrent = GetNumOfNamespaceUris();
			pNamespaceUri->SetAbsoluteIndex(indexCurrent - 1);
		}
		OpcUa_Int32 iIndex1 = OnLoadNamespaceUriGetSize() + 1;
		OnLoadNamespaceUriAddUri(aNamespaceUri, iIndex1);
		// Update RelativeIndex value
		// initialize the RelativeIndex in the CNamespaceUri instance
		pNamespaceUri->SetRelativeIndex(iIndex1);
	}
	else
	{
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_WARNING, "info>This namespaceUri %s is already handled by the server.\n", OpcUa_String_GetRawString(&aNamespaceUri));
		// 
		CNamespaceUri* pNamespaceUri = GetNamespaceUri(index); // ici on a le veritable index
		if (!pNamespaceUri)
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critical error in the configuration  of the namespaceUris\n");
		OpcUa_String* pString = pNamespaceUri->GetUriName();
		OpcUa_String atmpUriName;
		OpcUa_String_Initialize(&atmpUriName);
		OpcUa_String_CopyTo(pString, &atmpUriName);
		OpcUa_UInt32 iIndex1 = OnLoadNamespaceUriGetSize() + 1;
		if (OnLoadNamespaceUriContains(atmpUriName) == OpcUa_False)
			OnLoadNamespaceUriAddUri(atmpUriName, iIndex1);
		else
		{
			// Update the element for the NodeSet loading process
			// This mean that we will update the index of the OnLoadNamespaceUriList
			OnLoadNamespaceUriUpdateIndex(atmpUriName, iIndex1);
		}
		OpcUa_String_Clear(&atmpUriName);
		// mise à jour de la valeur d'index absolue
		pNamespaceUri->SetAbsoluteIndex(index);
	}
	return uStatus;
}
CLuaVirtualMachine* CUAInformationModel::GetLuaVirtualMachine()
{
	return m_pLuaVm;
}
COpenOpcUaScript* CUAInformationModel::GetOpenOpcUaScript()
{
	return m_pOpenOpcUaScript;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Adds an UA object to 'szBrowseName'. </summary>
///
/// <remarks>	Michel, 03/02/2016. </remarks>
///
/// <param name="aNodeId">	   	Identifier for the node. </param>
/// <param name="szBrowseName">	Name of the browse. </param>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode CUAInformationModel::AddUAObject(OpcUa_NodeId aNodeId, OpcUa_String szBrowseName, OpcUa_NodeId typeDefinition)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	CUAObject* pUAObject = OpcUa_Null; 
	OpcUa_QualifiedName aName;
	OpcUa_LocalizedText aLocalizedText;
	// Let check that the new nodeId is node already in the addressSpace
	CUABase* pBase = OpcUa_Null;
	if (GetNodeIdFromDictionnary(aNodeId, &pBase) != OpcUa_Good) // MUST fail
	{
		CUAObjectList* pUAObjectList = GetObjectList();
		if (pUAObjectList)
		{
			// Ils seront tous fils de 
			pUAObject = new CUAObject();
			if (pUAObject)
			{
				// BrowseName
				OpcUa_QualifiedName_Initialize(&aName);
				aName.NamespaceIndex = aNodeId.NamespaceIndex;
				OpcUa_String_CopyTo(&szBrowseName, &(aName.Name));
				pUAObject->SetBrowseName(&aName);
				OpcUa_QualifiedName_Clear(&aName);
				// Description
				OpcUa_LocalizedText_Initialize(&aLocalizedText);
				OpcUa_String_AttachCopy(&(aLocalizedText.Locale), "en-us");
				OpcUa_String_CopyTo(&szBrowseName, &(aLocalizedText.Text));
				pUAObject->SetDescription(aLocalizedText);
				OpcUa_LocalizedText_Clear(&aLocalizedText);
				// DisplayName
				OpcUa_LocalizedText_Initialize(&aLocalizedText);
				OpcUa_String_AttachCopy(&(aLocalizedText.Locale), "en-us");
				OpcUa_String_CopyTo(&szBrowseName, &(aLocalizedText.Text));
				pUAObject->SetDisplayName(&aLocalizedText);
				OpcUa_LocalizedText_Clear(&aLocalizedText);
				// NodeClass
				pUAObject->SetNodeClass(OpcUa_NodeClass_Object);
				// NodeId 
				pUAObject->SetNodeId(aNodeId);
				// TypeDefinition
				pUAObject->SetTypeDefinition(&typeDefinition);
				// WriteMask
				pUAObject->SetWriteMask(OpcUa_AccessLevels_CurrentRead);
				// UserWriteMask
				pUAObject->SetUserWriteMask(OpcUa_AccessLevels_CurrentRead);
				//// Add in the AddressSpace
				PopulateInformationModel(pUAObject);
			}
			else
				uStatus = OpcUa_BadOutOfMemory;
		}
		else
			uStatus = OpcUa_BadInternalError;
	}
	else
		uStatus = OpcUa_BadNodeIdExists;
	return uStatus;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Adds an UA variable. </summary>
///
/// <remarks>	Michel, 03/02/2016. </remarks>
///
/// <param name="aNodeId">		 	Identifier for the node. </param>
/// <param name="szBrowseName">  	Name of the browse. </param>
/// <param name="typeDefinition">	The type definition. </param>
/// <param name="internalValue"> 	The internal value. </param>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode CUAInformationModel::AddUAVariable(OpcUa_NodeId aNodeId, OpcUa_String szBrowseName, OpcUa_NodeId typeDefinition, OpcUa_Variant internalValue, OpcUa_Byte bAccessLevel, OpcUa_Boolean bHistorizing)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	CUAVariable* pUAVariable = OpcUa_Null;
	OpcUa_QualifiedName aName;
	OpcUa_QualifiedName_Initialize(&aName);
	OpcUa_LocalizedText aLocalizedText;
	OpcUa_LocalizedText_Initialize(&aLocalizedText);

	// Let check that the new nodeId is node already in the addressSpace
	CUABase* pBase = OpcUa_Null;
	if (GetNodeIdFromDictionnary(aNodeId, &pBase) != OpcUa_Good) // MUST fail
	{
		CUAVariableList* pUAVariableList = GetVariableList();
		if (pUAVariableList)
		{
			// Ils seront tous fils de 
			pUAVariable = new CUAVariable();
			if (pUAVariable)
			{
				// BrowseName
				aName.NamespaceIndex = aNodeId.NamespaceIndex;
				OpcUa_String_CopyTo(&szBrowseName, &(aName.Name));
				pUAVariable->SetBrowseName(&aName);				
				// Description				
				OpcUa_String_AttachCopy(&(aLocalizedText.Locale), "en-us");
				OpcUa_String_CopyTo(&szBrowseName, &(aLocalizedText.Text));
				pUAVariable->SetDescription(aLocalizedText);
				OpcUa_LocalizedText_Clear(&aLocalizedText);
				// DisplayName
				OpcUa_LocalizedText_Initialize(&aLocalizedText);
				OpcUa_String_AttachCopy(&(aLocalizedText.Locale), "en-us");
				OpcUa_String_CopyTo(&szBrowseName, &(aLocalizedText.Text));
				pUAVariable->SetDisplayName(&aLocalizedText);
				OpcUa_LocalizedText_Clear(&aLocalizedText);
				OpcUa_LocalizedText_Initialize(&aLocalizedText);
				// NodeClass
				pUAVariable->SetNodeClass(OpcUa_NodeClass_Variable);
				// NodeId 
				pUAVariable->SetNodeId(aNodeId);
				// TypeDefinition
				pUAVariable->SetTypeDefinition(&typeDefinition);
				// WriteMask
				pUAVariable->SetWriteMask(OpcUa_AccessLevels_CurrentRead);
				// UserWriteMask
				pUAVariable->SetUserWriteMask(OpcUa_AccessLevels_CurrentRead);
				// AccessLevel
				pUAVariable->SetAccessLevel(bAccessLevel);
				// UserAccessLevel. with generation from API we will use the same value for AccessLevel and UserAccessLevel
				pUAVariable->SetUserAccessLevel(bAccessLevel);
				// DataType
				OpcUa_NodeId dataType;
				OpcUa_NodeId_Initialize(&dataType);
				if (internalValue.Datatype == OpcUaType_ExtensionObject)
				{
					if (internalValue.Value.ExtensionObject)
						pUAVariable->SetDataType(internalValue.Value.ExtensionObject->TypeId.NodeId);
					pUAVariable->SetBuiltInType(OpcUaType_ExtensionObject);
				}
				else
				{
					dataType.Identifier.Numeric = internalValue.Datatype;
					OpcUa_Byte typeDefiniton;
					if (FindBuiltinType(dataType, &typeDefiniton) == OpcUa_Good)
					{
						// BuiltInType
						pUAVariable->SetBuiltInType(typeDefiniton);
					}
					else
						pUAVariable->SetBuiltInType(OpcUaType_Boolean); // default type
					pUAVariable->SetDataType(dataType);
				}
				OpcUa_NodeId_Clear(&dataType);
				// DataValue
				OpcUa_DataValue* pValue = (OpcUa_DataValue*)OpcUa_Alloc(sizeof(OpcUa_DataValue));
				OpcUa_DataValue_Initialize(pValue);
				OpcUa_Variant_CopyTo(&internalValue, &(pValue->Value));
				//pValue->Value.Datatype = internalValue.Datatype;
				//pValue->Value.ArrayType = internalValue.ArrayType;
				pUAVariable->SetValue(pValue);
				OpcUa_DataValue_Clear(pValue);
				OpcUa_Free(pValue);
				// Historizing
				pUAVariable->SetHistorizing(bHistorizing);
				// Minimum sampling interval. Here we set an arbitrary value of 50ms for the minimum sampling interval
				pUAVariable->SetMinimumSamplingInterval(50);				
				//// Add in the AddressSpace
				uStatus = PopulateInformationModel(pUAVariable);
				if (uStatus != OpcUa_Good)
					delete pUAVariable;
			}
			else
				uStatus = OpcUa_BadOutOfMemory;
		}
		else
			OpcUa_BadInternalError;
	}
	else
		uStatus = OpcUa_BadNodeIdExists;
	OpcUa_LocalizedText_Clear(&aLocalizedText);
	OpcUa_QualifiedName_Clear(&aName);
	return uStatus;	
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Adds a Tag in a VpiDevice. </summary>
///
/// <remarks>	Michel, 03/02/2016. </remarks>
///
/// <param name="nodeId">	   	Identifier for the node. </param>
/// <param name="deviceNodeId">	Identifier for the device node. </param>
/// <param name="szAddress">   	The address. </param>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode CUAInformationModel::AddVpiUATag(OpcUa_NodeId nodeId, OpcUa_NodeId deviceNodeId, OpcUa_String szAddress)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	CUAVariable* pUAVariable = OpcUa_Null;
	if (GetNodeIdFromVariableList(nodeId, &pUAVariable) == OpcUa_Good)
	{
		// Find the CVpiDevice matching the deviceNodeId
		CVpiDevice* pVpiDevice=OpcUa_Null;
		uStatus=g_pTheApplication->GetVpiDeviceById(deviceNodeId,&pVpiDevice);
		if (uStatus == OpcUa_Good)
		{
			// Call the parser to check the  parameter
			CVpiFuncCaller* pFuncCaller = pVpiDevice->GetVpiFuncCaller();
			if (pFuncCaller)
			{
				OpcUa_Byte Datatype = pUAVariable->GetBuiltInType();
				std::vector<OpcUa_UInt32>* arrayDimensions = pUAVariable->GetArrayDimensions();
				OpcUa_UInt32 iNbElt = 0;
				if (arrayDimensions)
					if (arrayDimensions->size()>0)
						iNbElt = arrayDimensions->at(0); // we support on vector. so only the first dimension
				OpcUa_Byte AccessRight = pUAVariable->GetAccessLevel();
				uStatus = pFuncCaller->ParseAddId(pFuncCaller->m_hVpi, nodeId, Datatype, iNbElt, AccessRight, szAddress);
				if (uStatus == OpcUa_Good)
				{
					// Create a new CVpiTag		
					CVpiTag* pVpiTag = new CVpiTag(pVpiDevice,pUAVariable,szAddress);
					pVpiTag->SetNbElement(iNbElt);
					pVpiTag->SetDimension(0);
					pUAVariable->SetPData((void*)pVpiTag);
					// Add the new CVpiTag in the VpiDevice
					pVpiDevice->AddTag(pVpiTag);
				}
			}
		}
	}
	else
		uStatus = OpcUa_BadNodeIdUnknown;
	return uStatus;	
}
///-------------------------------------------------------------------------------------------------
/// <summary>	Adds an UA reference. </summary>
///
/// <remarks>	Michel, 03/02/2016. </remarks>
///
/// <param name="sourceNodeId">   	Identifier for the source node. </param>
/// <param name="targetNodeid">   	Target nodeid. </param>
/// <param name="referenceTypeId">	Identifier for the reference type. </param>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode CUAInformationModel::AddUAReference(OpcUa_NodeId sourceNodeId, OpcUa_NodeId targetNodeid, OpcUa_NodeId referenceTypeId)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	// References
	CUABase* pSourceNode = OpcUa_Null;
	//CUABase* pTargetNode = OpcUa_Null;
	//CUABase* pReferenceNode = OpcUa_Null;
	uStatus = GetNodeIdFromDictionnary(sourceNodeId, &pSourceNode);
	if (uStatus == OpcUa_Good)
	{
		CUAReferenceList* pReferences = pSourceNode->GetReferenceNodeList();
		// HasTypeDefinition
		CUAReference* pReference = new CUAReference();
		//pReference->SetInverse(OpcUa_True);
		//aNodeId.Identifier.Numeric = OpcUaId_HasTypeDefinition;
		//aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		//aNodeId.NamespaceIndex = 0;
		//pReference->SetReferenceTypeId(&aNodeId);
		//OpcUa_NodeId_Clear(&aNodeId);
		//aNodeId = pSourceNode->GetTypeDefinition();
		//pReference->SetTargetId(&aNodeId);
		//pReferences->push_back(pReference);
		// Organizes
		//pReference = new CUAReference();
		pReference->SetInverse(OpcUa_False);
		pReference->SetReferenceTypeId(&referenceTypeId);

		pReference->SetTargetId(&targetNodeid);
		pReferences->push_back(pReference);
	}
	return uStatus;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Method to Add a new UAVariable in the AddressSpace.  </summary>
///
/// <remarks>	Michel, 25/02/2016. </remarks>
///
/// <param name="pUAVariable">	[in,out] If non-null, the UA variable. </param>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode OpenOpcUa::UAAddressSpace::CUAInformationModel::PopulateInformationModel(CUAVariable* pUAVariable)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	CUAVariableList* pUAVariableList = GetVariableList();
	OpcUa_NodeId* pNodeId = pUAVariable->GetNodeId();
	CUAVariableList::iterator it= pUAVariableList->find(pNodeId);
	if (it == pUAVariableList->end())
		pUAVariableList->insert(std::pair<OpcUa_NodeId*, CUAVariable*>(pUAVariable->GetNodeId(), pUAVariable));
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Populate information model. </summary>
///
/// <remarks>	Michel, 25/02/2016. </remarks>
///
/// <param name="pUAObject">	[in,out] If non-null, the UA object. </param>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode OpenOpcUa::UAAddressSpace::CUAInformationModel::PopulateInformationModel(CUAObject*pUAObject)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	CUAObjectList* pUAObjectList = GetObjectList();
	OpcUa_NodeId* pNodeId = pUAObject->GetNodeId();
	CUAObjectList::iterator it = pUAObjectList->find(pNodeId);
	if (it == pUAObjectList->end())
		pUAObjectList->insert(std::pair<OpcUa_NodeId*, CUAObject*>(pUAObject->GetNodeId(), pUAObject));
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Adds a subscription rate in the address space. </summary>
///
/// <remarks>	Michel, 28/05/2016. </remarks>
///
/// <param name="szSubscriptionRate">	The subscription rate. </param>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode OpenOpcUa::UAAddressSpace::CUAInformationModel::AddSamplingIntervalnTheAddressSpace(OpcUa_SamplingIntervalDiagnosticsDataType* pSamplingIntervalDiagnosticsDataType)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_Mutex_Lock(m_ServerCacheMutex);
	// Name 
	OpcUa_CharA* pszSamplingInterval=(OpcUa_CharA*)OpcUa_Alloc(20);
	OpcUa_String szSamplingInterval;
	OpcUa_String_Initialize(&szSamplingInterval);
	ZeroMemory(pszSamplingInterval, 20);
	sprintf(pszSamplingInterval,"%lf",pSamplingIntervalDiagnosticsDataType->SamplingInterval);
	OpcUa_String_AttachCopy(&szSamplingInterval, pszSamplingInterval);
	// NodeId
	OpcUa_NodeId aNodeId;
	OpcUa_NodeId_Initialize(&aNodeId);
	OpcUa_String_CopyTo(&szSamplingInterval, &(aNodeId.Identifier.String));
	aNodeId.IdentifierType = OpcUa_IdentifierType_String;
	aNodeId.NamespaceIndex = 1;
	// TypeDefinition
	OpcUa_NodeId typeDefinition;
	OpcUa_NodeId_Initialize(&typeDefinition);// 
	typeDefinition.Identifier.Numeric = OpcUaId_SamplingIntervalDiagnosticsType; // SamplingIntervalDiagnosticsType is the typedef of the UAVariable
	// Value
	OpcUa_Variant varVal;
	OpcUa_Variant_Initialize(&varVal);
	uStatus=AddUAVariable(aNodeId, szSamplingInterval, typeDefinition, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
	if (uStatus == OpcUa_Good)
	{
		// HasComponent
		OpcUa_NodeId aNodeIdHasComponent;
		OpcUa_NodeId_Initialize(&aNodeIdHasComponent);
		aNodeIdHasComponent.Identifier.Numeric = OpcUaId_HasComponent;
		aNodeIdHasComponent.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeIdHasComponent.NamespaceIndex = 0;
		// 2289
		OpcUa_NodeId aNodeId2289;
		OpcUa_NodeId_Initialize(&aNodeId2289);
		aNodeId2289.Identifier.Numeric = 2289;
		aNodeId2289.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId2289.NamespaceIndex = 0;
		uStatus = AddUAReference(aNodeId2289, aNodeId, aNodeIdHasComponent);
		if (uStatus == OpcUa_Good)
		{
			// Now let's add reference for the newly created UAVariable
			// First let create the UAVariable. 
			// Each of them will be a target of the HasComponent reference
			// HasComponent SamplingInterval 
			OpcUa_String SamplingInterval;
			OpcUa_String_Initialize(&SamplingInterval);
			OpcUa_String_AttachCopy(&SamplingInterval, "SamplingInterval");
			OpcUa_NodeId aNodeIdSamplingInterval;
			OpcUa_NodeId_Initialize(&aNodeIdSamplingInterval);
			OpcUa_String_CopyTo(&SamplingInterval, &(aNodeIdSamplingInterval.Identifier.String));
			aNodeIdSamplingInterval.IdentifierType = OpcUa_IdentifierType_String;
			aNodeIdSamplingInterval.NamespaceIndex = 1;
			//
			typeDefinition.Identifier.Numeric = OpcUaId_BaseDataVariableType;
			//
			OpcUa_Variant_Clear(&varVal);
			OpcUa_Variant_Initialize(&varVal);
			varVal.Datatype = OpcUaId_Double;
			varVal.Value.Double = pSamplingIntervalDiagnosticsDataType->SamplingInterval;
			uStatus = AddUAVariable(aNodeIdSamplingInterval, SamplingInterval, typeDefinition, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
			if ((uStatus == OpcUa_Good) || (uStatus == OpcUa_BadNodeIdExists)) // If the node exist we will add a new reference to it
			{		 
				// SamplingInterval
				uStatus = AddUAReference(aNodeId, aNodeIdSamplingInterval, aNodeIdHasComponent);
				if (uStatus == OpcUa_Good)
				{
					// HasComponent SampledMonitoredItemsCount
					// HasComponent MaxSampledMonitoredItemsCount
					// HasComponent DisabledMonitoredItemsSamplingCount
				}	
			}
			OpcUa_String_Clear(&SamplingInterval);
			OpcUa_NodeId_Clear(&aNodeIdSamplingInterval);
		}
		OpcUa_NodeId_Clear(&aNodeId2289);
		OpcUa_NodeId_Clear(&aNodeIdHasComponent);
	}
	OpcUa_Variant_Clear(&varVal);
	OpcUa_NodeId_Clear(&typeDefinition);
	OpcUa_String_Clear(&szSamplingInterval);
	OpcUa_NodeId_Clear(&aNodeId);
	OpcUa_Free(pszSamplingInterval);
	OpcUa_Mutex_Unlock(m_ServerCacheMutex);
	/*
	OpcUa_LocalizedText aLocalizedText;
	OpcUa_NodeId aNodeId;
	CUAReferenceList* pReferences = OpcUa_Null;
	CUAReference* pReference = OpcUa_Null;
	// Create the UAVariable for pSamplingIntervalDiagnosticsDataType
	OpcUa_CharA* pszSamplingInterval=(OpcUa_CharA*)OpcUa_Alloc(20);
	OpcUa_String szSamplingInterval;
	OpcUa_String_Initialize(&szSamplingInterval);
	ZeroMemory(pszSamplingInterval, 20);
	sprintf(pszSamplingInterval,"%lf",pSamplingIntervalDiagnosticsDataType->SamplingInterval);
	OpcUa_String_AttachCopy(&szSamplingInterval, pszSamplingInterval);
	CUAVariable* pUAVariable = new CUAVariable();
	// BrowseName
	OpcUa_QualifiedName szBrowseName;
	OpcUa_QualifiedName_Initialize(&szBrowseName);
	OpcUa_String_CopyTo(&szSamplingInterval, &(szBrowseName.Name));
	szBrowseName.NamespaceIndex = 1;
	pUAVariable->SetBrowseName(&szBrowseName);
	OpcUa_QualifiedName_Clear(&szBrowseName);
	// Description
	OpcUa_LocalizedText_Initialize(&aLocalizedText);
	OpcUa_String_AttachCopy(&(aLocalizedText.Locale), "en-us");
	OpcUa_String_CopyTo(&szSamplingInterval, &(aLocalizedText.Text));
	pUAVariable->SetDescription(aLocalizedText);
	OpcUa_LocalizedText_Clear(&aLocalizedText);
	// DisplayName
	OpcUa_LocalizedText_Initialize(&aLocalizedText);
	OpcUa_String_AttachCopy(&(aLocalizedText.Locale), "en-us");
	OpcUa_String_CopyTo(&szSamplingInterval, &(aLocalizedText.Text));
	pUAVariable->SetDisplayName(&aLocalizedText);
	OpcUa_LocalizedText_Clear(&aLocalizedText);
	// NodeClass
	pUAVariable->SetNodeClass(OpcUa_NodeClass_Variable);
	// NodeId (ns=1;s=szSamplingInterval)
	OpcUa_NodeId_Initialize(&aNodeId);
	OpcUa_String_CopyTo(&szSamplingInterval, &(aNodeId.Identifier.String));
	aNodeId.IdentifierType = OpcUa_IdentifierType_String;
	aNodeId.NamespaceIndex = 1;
	pUAVariable->SetNodeId(aNodeId);

	// TypeDefinition
	OpcUa_NodeId_Clear(&aNodeId);
	aNodeId.Identifier.Numeric = OpcUaId_SamplingIntervalDiagnosticsType; // SamplingIntervalDiagnosticsType is the typedef of the UAVariable
	pUAVariable->SetTypeDefinition(&aNodeId);
	// WriteMask
	pUAVariable->SetWriteMask(OpcUa_AccessLevels_CurrentRead);
	// UserWriteMask
	pUAVariable->SetUserWriteMask(OpcUa_AccessLevels_CurrentRead);
	//
	// 
	// References
	pReferences = pUAVariable->GetReferenceNodeList();
	// HasTypeDefinition
	//
	// Get 2289 UAVariable. The purpose is to add the new UAVariable in 2289
	// So we create a forward reference to the new UAVariable
	CUAVariable* pUAVariable2289 = OpcUa_Null;
	OpcUa_NodeId_Clear(&aNodeId);
	aNodeId.Identifier.Numeric = 2289;
	aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
	aNodeId.NamespaceIndex = 0;
	if (GetNodeIdFromVariableList(aNodeId, &pUAVariable2289) == OpcUa_Good)
	{
		// Add the new UAVariable as a child ref of 2289
		pReference = new CUAReference();
		pReference->SetInverse(OpcUa_False);
		OpcUa_NodeId_Clear(&aNodeId);
		aNodeId.Identifier.Numeric = OpcUaId_HasComponent;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 0;
		pReference->SetReferenceTypeId(&aNodeId);
		CUAReferenceList* pReferences2289 = pUAVariable2289->GetReferenceNodeList();
		pReference->SetTargetId(pUAVariable->GetNodeId());
		//// Add in the AddressSpace
		pReferences2289->push_back(pReference);
		// Now let's add reference for the newly created UAVariable
		// HasTypeDefinition
		CUAReference* pReference = new CUAReference();
		pReference->SetInverse(OpcUa_True);
		aNodeId.Identifier.Numeric = OpcUaId_HasTypeDefinition;
		aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId.NamespaceIndex = 0;
		pReference->SetReferenceTypeId(&aNodeId);
		OpcUa_NodeId_Clear(&aNodeId);
		aNodeId = pUAVariable->GetTypeDefinition();
		pReference->SetTargetId(&aNodeId);
		pReferences->push_back(pReference);
		// HasComponent SamplingInterval 
		// HasComponent SampledMonitoredItemsCount
		// HasComponent MaxSampledMonitoredItemsCount
		// HasComponent DisabledMonitoredItemsSamplingCount
		PopulateInformationModel(pUAVariable);
	}
	else
	{
		OpcUa_Trace(OPCUA_TRACE_SERVER_ERROR, "critical error. i=2289 is not int the addressSpace\n");
		delete pReference;
		delete pUAVariable;
	}
	*/
	return uStatus;
}

///-------------------------------------------------------------------------------------------------
/// <summary>
/// 	Removes the subscription rate in the address space described by szSubscriptionRate.
/// </summary>
///
/// <remarks>	Michel, 28/05/2016. </remarks>
///
/// <param name="szSubscriptionRate">	The subscription rate. </param>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode OpenOpcUa::UAAddressSpace::CUAInformationModel::RemoveSamplingIntervalnTheAddressSpace(OpcUa_SamplingIntervalDiagnosticsDataType* pSamplingIntervalDiagnosticsDataType)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_Mutex_Lock(m_ServerCacheMutex);
	CUAVariable* pUAVariable = OpcUa_Null;
	if (pSamplingIntervalDiagnosticsDataType)
	{
		// Name 
		OpcUa_CharA* pszSamplingInterval = (OpcUa_CharA*)OpcUa_Alloc(20);
		OpcUa_String szSamplingInterval;
		OpcUa_String_Initialize(&szSamplingInterval);
		ZeroMemory(pszSamplingInterval, 20);
		sprintf(pszSamplingInterval, "%lf", pSamplingIntervalDiagnosticsDataType->SamplingInterval);
		OpcUa_String_AttachCopy(&szSamplingInterval, pszSamplingInterval);
		// NodeId
		OpcUa_NodeId aNodeIdSamplingIntervalValue;
		OpcUa_NodeId_Initialize(&aNodeIdSamplingIntervalValue);
		OpcUa_String_CopyTo(&szSamplingInterval, &(aNodeIdSamplingIntervalValue.Identifier.String));
		aNodeIdSamplingIntervalValue.IdentifierType = OpcUa_IdentifierType_String;
		aNodeIdSamplingIntervalValue.NamespaceIndex = 1;
		// SamplingInterval
		//OpcUa_String SamplingInterval;
		//OpcUa_String_Initialize(&SamplingInterval);
		//OpcUa_String_AttachCopy(&SamplingInterval, "SamplingInterval");
		//OpcUa_NodeId aNodeIdSamplingInterval;
		//OpcUa_NodeId_Initialize(&aNodeIdSamplingInterval);
		//OpcUa_String_CopyTo(&SamplingInterval, &(aNodeIdSamplingInterval.Identifier.String));
		//aNodeIdSamplingInterval.IdentifierType = OpcUa_IdentifierType_String;
		//aNodeIdSamplingInterval.NamespaceIndex = 1;
		// search for CSessionServer
		CUAVariableList* pUAVariableList = GetVariableList();
		if (pUAVariableList)
		{
			CUAVariableList::iterator it = pUAVariableList->find(&aNodeIdSamplingIntervalValue);
			if (it != pUAVariableList->end())
			{
				pUAVariable = it->second;
				pUAVariableList->erase(it);
				//
				CUAReferenceList* pUAReferenceList = pUAVariable->GetReferenceNodeList();
				if (pUAReferenceList)
				{
					CUAReferenceList::iterator itRef;
					for (itRef = pUAReferenceList->begin(); itRef != pUAReferenceList->end(); itRef++)
					{
						CUAReference* pReference = *itRef;
						OpcUa_ExpandedNodeId expandedNodeId = pReference->GetTargetId();
						CUAVariableList::iterator itTarget = pUAVariableList->find(&(expandedNodeId.NodeId));
						if (itTarget != pUAVariableList->end())
						{
							CUAVariable* pUATargetVariable = itTarget->second;
							delete pUATargetVariable;
							pUAVariableList->erase(itTarget);
						}
					}
				}
				// 
				delete pUAVariable;
				pUAVariable = OpcUa_Null;
			}
		}
	}
	OpcUa_Mutex_Unlock(m_ServerCacheMutex);
	return uStatus;	
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Add subscription diagnostics Node in the addressSpace. </summary>
///
/// <remarks>	Michel, 29/05/2016. </remarks>
///
/// <param name="pSubscriptionDiagnosticsDataType">	[in,out] If non-null, type of the
/// 												subscription diagnostics data.
/// </param>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode OpenOpcUa::UAAddressSpace::CUAInformationModel::AddSubscriptionDiagnosticsInAddressSpace(CSubscriptionDiagnosticsDataType* pSubscriptionDiagnosticsDataType)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_Mutex_Lock(m_ServerCacheMutex);
	OpcUa_UInt32 uiSubscriptionId=pSubscriptionDiagnosticsDataType->GetSubscriptionId();
	// Name 
	OpcUa_CharA* pszSubscriptionId = (OpcUa_CharA*)OpcUa_Alloc(20);
	ZeroMemory(pszSubscriptionId, 20);
	sprintf(pszSubscriptionId, "%u", uiSubscriptionId);
	// NodeId
	OpcUa_NodeId aNodeId;
	OpcUa_NodeId_Initialize(&aNodeId);
	aNodeId.IdentifierType = OpcUa_IdentifierType_String;
	aNodeId.NamespaceIndex = 1;
	// SubscriptionId
	OpcUa_String szSubscriptionId;
	OpcUa_String_Initialize(&szSubscriptionId);
	OpcUa_String_AttachCopy(&szSubscriptionId, pszSubscriptionId);
	OpcUa_String_CopyTo(&szSubscriptionId, &(aNodeId.Identifier.String));
	// TypeDefinition
	OpcUa_NodeId typeDefinition;
	OpcUa_NodeId_Initialize(&typeDefinition);// 
	typeDefinition.Identifier.Numeric = OpcUaId_SubscriptionDiagnosticsDataType; // SubscriptionDiagnosticsDataType is the typedef of the UAVariable
	// Value
	OpcUa_Variant varVal;
	OpcUa_Variant_Initialize(&varVal);
	//varVal.Value.ExtensionObject = (OpcUa_ExtensionObject*)OpcUa_Alloc(sizeof(OpcUa_ExtensionObject));
	OpcUa_ExtensionObject_Initialize(varVal.Value.ExtensionObject);
	varVal.Datatype = OpcUaType_ExtensionObject;
	OpcUa_EncodeableType* pEncodeableType = OpcUa_Null;
	uStatus = g_pTheApplication->LookupEncodeableType(OpcUaId_SubscriptionDiagnosticsDataType, &pEncodeableType);// pUAVariable->GetDataType().Identifier.Numeric
	if (uStatus == OpcUa_Good)
	{
		varVal.Value.ExtensionObject = (OpcUa_ExtensionObject*)OpcUa_Alloc(sizeof(OpcUa_ExtensionObject));
		OpcUa_ExpandedNodeId_Initialize(&(varVal.Value.ExtensionObject->TypeId));
		varVal.Value.ExtensionObject->Encoding = OpcUa_ExtensionObjectEncoding_EncodeableObject;
		// TypeId
		OpcUa_ExpandedNodeId_Initialize(&varVal.Value.ExtensionObject->TypeId);
		varVal.Value.ExtensionObject->TypeId.NodeId.Identifier.Numeric = pEncodeableType->TypeId;
		varVal.Value.ExtensionObject->TypeId.NodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		varVal.Value.ExtensionObject->TypeId.ServerIndex = 0;
		varVal.Value.ExtensionObject->TypeId.NodeId.Identifier.Numeric = pEncodeableType->TypeId;
		varVal.Value.ExtensionObject->Body.EncodeableObject.Type = pEncodeableType;
		varVal.Value.ExtensionObject->Body.EncodeableObject.Object = (void*)Utils::Copy(pSubscriptionDiagnosticsDataType->GetInternalSubscriptionDiagnosticsDataType());
	}
	uStatus = AddUAVariable(aNodeId, szSubscriptionId, typeDefinition, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
	if (uStatus == OpcUa_Good)
	{
		// HasComponent
		OpcUa_NodeId aNodeIdHasComponent;
		OpcUa_NodeId_Initialize(&aNodeIdHasComponent);
		aNodeIdHasComponent.Identifier.Numeric = OpcUaId_HasComponent;
		aNodeIdHasComponent.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeIdHasComponent.NamespaceIndex = 0;
		// 2290
		OpcUa_NodeId aNodeId2290;
		OpcUa_NodeId_Initialize(&aNodeId2290);
		aNodeId2290.Identifier.Numeric = 2290;
		aNodeId2290.IdentifierType = OpcUa_IdentifierType_Numeric;
		aNodeId2290.NamespaceIndex = 0;
		uStatus = AddUAReference(aNodeId2290, aNodeId, aNodeIdHasComponent);
		if (uStatus == OpcUa_Good)
		{
			// Now let's add reference for the newly created UAVariable
			// First let create the UAVariable. 
			// Each of them will be a target of the HasComponent reference
			// HasComponent SessionId
			OpcUa_CharA* szNodeId = (OpcUa_CharA*)OpcUa_Alloc(50);
			ZeroMemory(szNodeId, 50);
			sprintf(szNodeId, "SessionId-%u-%u", pSubscriptionDiagnosticsDataType->GetSessionId().Identifier.Numeric, pSubscriptionDiagnosticsDataType->GetSubscriptionId());
			OpcUa_String SessionId;
			OpcUa_String_Initialize(&SessionId);
			OpcUa_String_AttachCopy(&SessionId, szNodeId);
			// NodeId of the SessionId node
			OpcUa_NodeId aNodeIdSessionId;
			OpcUa_NodeId_Initialize(&aNodeIdSessionId);
			aNodeIdSessionId.IdentifierType = OpcUa_IdentifierType_String;
			aNodeIdSessionId.NamespaceIndex = 1;
			OpcUa_String_CopyTo(&SessionId, &(aNodeIdSessionId.Identifier.String));
			// BrowseName 
			OpcUa_String_Clear(&SessionId);
			OpcUa_String_Initialize(&SessionId);
			OpcUa_String_AttachCopy(&SessionId, "SessionId");
			// typeDefinition
			typeDefinition.Identifier.Numeric = OpcUaId_BaseDataVariableType;
			/////////////////////////////////////////////////////////////////////////////////////////////////
			// 
			// Add reference in the new subscriptionDiagnostic Node - see Part 5 Table 73 for the list of node
			OpcUa_Variant_Clear(&varVal);
			OpcUa_Variant_Initialize(&varVal);
			varVal.Datatype = OpcUaType_NodeId;
			varVal.Value.NodeId = (OpcUa_NodeId*)OpcUa_Alloc(sizeof(OpcUa_NodeId));
			OpcUa_NodeId_Initialize(varVal.Value.NodeId);
			OpcUa_NodeId tmpNodeId=pSubscriptionDiagnosticsDataType->GetSessionId();
			OpcUa_NodeId_CopyTo(&tmpNodeId,varVal.Value.NodeId);
			uStatus = AddUAVariable(aNodeIdSessionId, SessionId, typeDefinition, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
			if (uStatus == OpcUa_Good)
			{
				OpcUa_NodeId_Clear(varVal.Value.NodeId);
				OpcUa_Free(varVal.Value.NodeId);
				uStatus = AddUAReference(aNodeId, aNodeIdSessionId, aNodeIdHasComponent);
				if (uStatus == OpcUa_Good)
				{
					// Adjust timeStamp and statuscode
					InitTimeStampAndStatusCode(aNodeIdSessionId);
					// HasComponent SubscriptionId
					OpcUa_String_Clear(&szSubscriptionId);
					OpcUa_String_Initialize(&szSubscriptionId);
					// NodeId
					OpcUa_NodeId subscriptionId;
					OpcUa_NodeId_Initialize(&subscriptionId);
					ZeroMemory(szNodeId, 50);
					sprintf(szNodeId, "SubscriptionId-%u",  pSubscriptionDiagnosticsDataType->GetSubscriptionId());
					OpcUa_String_AttachCopy(&szSubscriptionId, szNodeId);
					subscriptionId.IdentifierType = OpcUa_IdentifierType_String;
					subscriptionId.NamespaceIndex = 1;
					OpcUa_String_CopyTo(&szSubscriptionId, &subscriptionId.Identifier.String);
					// BrowseName
					OpcUa_String_Clear(&szSubscriptionId);
					OpcUa_String_Initialize(&szSubscriptionId);
					OpcUa_String_AttachCopy(&szSubscriptionId, "SubscriptionId");
					// Value
					OpcUa_Variant_Initialize(&varVal);
					varVal.Datatype = OpcUaType_UInt32;
					varVal.Value.UInt32 = pSubscriptionDiagnosticsDataType->GetSubscriptionId();
					uStatus = AddUAVariable(subscriptionId, szSubscriptionId, typeDefinition, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
					if (uStatus == OpcUa_Good)
					{
						uStatus = AddUAReference(aNodeId, subscriptionId, aNodeIdHasComponent);
						if (uStatus == OpcUa_Good)
						{
							// Adjust timeStamp and statuscode
							InitTimeStampAndStatusCode(subscriptionId);
							// HasComponent Priority
							OpcUa_String szPriority;
							OpcUa_String_Initialize(&szPriority);
							OpcUa_NodeId priorityId;
							OpcUa_NodeId_Initialize(&priorityId);
							ZeroMemory(szNodeId, 50);
							sprintf(szNodeId, "priority-%u", pSubscriptionDiagnosticsDataType->GetSubscriptionId());
							OpcUa_String_AttachCopy(&szPriority, szNodeId);
							priorityId.IdentifierType = OpcUa_IdentifierType_String;
							priorityId.NamespaceIndex = 1;
							OpcUa_String_CopyTo(&szPriority, &priorityId.Identifier.String);
							// BrowseName
							OpcUa_String_Clear(&szPriority);
							OpcUa_String_Initialize(&szPriority);
							OpcUa_String_AttachCopy(&szPriority, "Priority");
							// Value
							OpcUa_Variant_Clear(&varVal);
							OpcUa_Variant_Initialize(&varVal);
							varVal.Datatype = OpcUaType_Byte;
							varVal.Value.Byte = pSubscriptionDiagnosticsDataType->GetPriority();
							uStatus = AddUAVariable(priorityId, szPriority, typeDefinition, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
							if (uStatus == OpcUa_Good)
							{
								uStatus = AddUAReference(aNodeId, priorityId, aNodeIdHasComponent);
								if (uStatus == OpcUa_Good)
								{
									// Adjust timeStamp and statuscode
									InitTimeStampAndStatusCode(priorityId);
									// HasComponent PublishingEnabled
									OpcUa_String szPublishingEnabled;
									OpcUa_String_Initialize(&szPublishingEnabled);
									OpcUa_NodeId PublishingEnabledId;
									OpcUa_NodeId_Initialize(&PublishingEnabledId);
									ZeroMemory(szNodeId, 50);
									sprintf(szNodeId, "PublishingEnabled-%u", pSubscriptionDiagnosticsDataType->GetSubscriptionId());
									OpcUa_String_AttachCopy(&szPublishingEnabled, szNodeId);
									PublishingEnabledId.IdentifierType = OpcUa_IdentifierType_String;
									PublishingEnabledId.NamespaceIndex = 1;
									OpcUa_String_CopyTo(&szPublishingEnabled, &PublishingEnabledId.Identifier.String);
									// BrowseName
									OpcUa_String_Clear(&szPublishingEnabled);
									OpcUa_String_Initialize(&szPublishingEnabled);
									OpcUa_String_AttachCopy(&szPublishingEnabled, "PublishingEnabled");
									// Value
									OpcUa_Variant_Clear(&varVal);
									OpcUa_Variant_Initialize(&varVal);
									varVal.Datatype = OpcUaType_Boolean;
									varVal.Value.Boolean = pSubscriptionDiagnosticsDataType->GetPublishingEnabled();
									uStatus = AddUAVariable(PublishingEnabledId, szPublishingEnabled, typeDefinition, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
									if (uStatus == OpcUa_Good)
									{
										uStatus = AddUAReference(aNodeId, PublishingEnabledId, aNodeIdHasComponent);
										if (uStatus == OpcUa_Good)
										{
											// Adjust timeStamp and statuscode
											InitTimeStampAndStatusCode(PublishingEnabledId);
											// HasComponent MaxNotificationsPerPublish
											OpcUa_String szMaxNotificationsPerPublish;
											OpcUa_String_Initialize(&szMaxNotificationsPerPublish);
											OpcUa_NodeId MaxNotificationsPerPublishId;
											OpcUa_NodeId_Initialize(&MaxNotificationsPerPublishId);
											ZeroMemory(szNodeId, 50);
											sprintf(szNodeId, "MaxNotificationsPerPublish-%u", pSubscriptionDiagnosticsDataType->GetSubscriptionId());
											OpcUa_String_AttachCopy(&szMaxNotificationsPerPublish, szNodeId);
											MaxNotificationsPerPublishId.IdentifierType = OpcUa_IdentifierType_String;
											MaxNotificationsPerPublishId.NamespaceIndex = 1;
											OpcUa_String_CopyTo(&szMaxNotificationsPerPublish, &MaxNotificationsPerPublishId.Identifier.String);
											// BrowseName
											OpcUa_String_Clear(&szMaxNotificationsPerPublish);
											OpcUa_String_Initialize(&szMaxNotificationsPerPublish);
											OpcUa_String_AttachCopy(&szMaxNotificationsPerPublish, "MaxNotificationsPerPublish");
											// Value
											OpcUa_Variant_Clear(&varVal);
											OpcUa_Variant_Initialize(&varVal);
											varVal.Datatype = OpcUaType_UInt32;
											varVal.Value.UInt32 = pSubscriptionDiagnosticsDataType->GetMaxNotificationsPerPublish();
											uStatus = AddUAVariable(MaxNotificationsPerPublishId, szMaxNotificationsPerPublish, typeDefinition, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
											if (uStatus == OpcUa_Good)
											{
												uStatus = AddUAReference(aNodeId, MaxNotificationsPerPublishId, aNodeIdHasComponent);
												if (uStatus == OpcUa_Good)
												{
													// Adjust timeStamp and statuscode
													InitTimeStampAndStatusCode(MaxNotificationsPerPublishId);
													// HasComponent PublishingInterval
													OpcUa_String szPublishingInterval;
													OpcUa_String_Initialize(&szPublishingInterval);
													OpcUa_NodeId PublishingIntervalId;
													OpcUa_NodeId_Initialize(&PublishingIntervalId);
													ZeroMemory(szNodeId, 50);
													sprintf(szNodeId, "PublishingInterval-%u", pSubscriptionDiagnosticsDataType->GetSubscriptionId());
													OpcUa_String_AttachCopy(&szPublishingInterval, szNodeId);
													PublishingIntervalId.IdentifierType = OpcUa_IdentifierType_String;
													PublishingIntervalId.NamespaceIndex = 1;
													OpcUa_String_CopyTo(&szPublishingInterval, &PublishingIntervalId.Identifier.String);
													// BrowseName
													OpcUa_String_Clear(&szPublishingInterval);
													OpcUa_String_Initialize(&szPublishingInterval);
													OpcUa_String_AttachCopy(&szPublishingInterval, "PublishingInterval");
													// value
													OpcUa_Variant_Clear(&varVal);
													OpcUa_Variant_Initialize(&varVal);
													varVal.Datatype = OpcUaType_Double;
													varVal.Value.Double = pSubscriptionDiagnosticsDataType->GetPublishingInterval();
													uStatus = AddUAVariable(PublishingIntervalId, szPublishingInterval, typeDefinition, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
													if (uStatus == OpcUa_Good)
													{
														uStatus = AddUAReference(aNodeId, PublishingIntervalId, aNodeIdHasComponent);
														if (uStatus == OpcUa_Good)
														{
															// Adjust timeStamp and statuscode
															InitTimeStampAndStatusCode(PublishingIntervalId);
															// HasComponent MaxLifetimeCount
															OpcUa_String szMaxLifetimeCount;
															OpcUa_String_Initialize(&szMaxLifetimeCount);
															OpcUa_NodeId MaxLifetimeCountId;
															OpcUa_NodeId_Initialize(&MaxLifetimeCountId);
															ZeroMemory(szNodeId, 50);
															sprintf(szNodeId, "MaxLifetimeCount-%u", pSubscriptionDiagnosticsDataType->GetSubscriptionId());
															OpcUa_String_AttachCopy(&szMaxLifetimeCount, szNodeId);
															MaxLifetimeCountId.IdentifierType = OpcUa_IdentifierType_String;
															MaxLifetimeCountId.NamespaceIndex = 1;
															OpcUa_String_CopyTo(&szMaxLifetimeCount, &MaxLifetimeCountId.Identifier.String);
															// BrowseName
															OpcUa_String_Clear(&szMaxLifetimeCount);
															OpcUa_String_Initialize(&szMaxLifetimeCount);
															OpcUa_String_AttachCopy(&szMaxLifetimeCount, "MaxLifetimeCount");
															// value
															OpcUa_Variant_Clear(&varVal);
															OpcUa_Variant_Initialize(&varVal);
															varVal.Datatype = OpcUaType_UInt32;
															varVal.Value.UInt32 = pSubscriptionDiagnosticsDataType->GetMaxLifetimeCount();
															uStatus = AddUAVariable(MaxLifetimeCountId, szMaxLifetimeCount, typeDefinition, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
															if (uStatus == OpcUa_Good)
															{
																uStatus = AddUAReference(aNodeId, MaxLifetimeCountId, aNodeIdHasComponent);
																if (uStatus == OpcUa_Good)
																{
																	// Adjust timeStamp and statuscode
																	InitTimeStampAndStatusCode(MaxLifetimeCountId);
																	// HasComponent MaxKeepAliveCount
																	OpcUa_String szMaxKeepAliveCount;
																	OpcUa_String_Initialize(&szMaxKeepAliveCount);
																	OpcUa_NodeId MaxKeepAliveCountId;
																	OpcUa_NodeId_Initialize(&MaxKeepAliveCountId);
																	ZeroMemory(szNodeId, 50);
																	sprintf(szNodeId, "MaxKeepAliveCount-%u", pSubscriptionDiagnosticsDataType->GetSubscriptionId());
																	OpcUa_String_AttachCopy(&szMaxKeepAliveCount, szNodeId);
																	MaxKeepAliveCountId.IdentifierType = OpcUa_IdentifierType_String;
																	MaxKeepAliveCountId.NamespaceIndex = 1;
																	OpcUa_String_CopyTo(&szMaxKeepAliveCount, &MaxKeepAliveCountId.Identifier.String);
																	// BrowseName
																	OpcUa_String_Clear(&szMaxKeepAliveCount);
																	OpcUa_String_Initialize(&szMaxKeepAliveCount);
																	OpcUa_String_AttachCopy(&szMaxKeepAliveCount, "MaxKeepAliveCount");
																	// Value
																	OpcUa_Variant_Clear(&varVal);
																	OpcUa_Variant_Initialize(&varVal);
																	varVal.Datatype = OpcUaType_UInt32;
																	varVal.Value.UInt32 = pSubscriptionDiagnosticsDataType->GetMaxKeepAliveCount();
																	uStatus = AddUAVariable(MaxKeepAliveCountId, szMaxKeepAliveCount, typeDefinition, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
																	if (uStatus == OpcUa_Good)
																	{
																		uStatus = AddUAReference(aNodeId, MaxKeepAliveCountId, aNodeIdHasComponent);
																		if (uStatus == OpcUa_Good)
																		{
																			// Adjust timeStamp and statuscode
																			InitTimeStampAndStatusCode(MaxKeepAliveCountId);
																			// HasComponent PublishRequestCount
																			OpcUa_String szPublishRequestCount;
																			OpcUa_String_Initialize(&szPublishRequestCount);
																			OpcUa_NodeId PublishRequestCountId;
																			OpcUa_NodeId_Initialize(&PublishRequestCountId);
																			ZeroMemory(szNodeId, 50);
																			sprintf(szNodeId, "PublishRequestCount-%u", pSubscriptionDiagnosticsDataType->GetSubscriptionId());
																			OpcUa_String_AttachCopy(&szPublishRequestCount, szNodeId);
																			PublishRequestCountId.IdentifierType = OpcUa_IdentifierType_String;
																			PublishRequestCountId.NamespaceIndex = 1;
																			OpcUa_String_CopyTo(&szPublishRequestCount, &PublishRequestCountId.Identifier.String);
																			// BrowseName
																			OpcUa_String_Clear(&szPublishRequestCount);
																			OpcUa_String_Initialize(&szPublishRequestCount);
																			OpcUa_String_AttachCopy(&szPublishRequestCount, "PublishRequestCount");
																			// Value
																			OpcUa_Variant_Clear(&varVal);
																			OpcUa_Variant_Initialize(&varVal);
																			varVal.Datatype = OpcUaType_UInt32;
																			varVal.Value.UInt32 = pSubscriptionDiagnosticsDataType->GetPublishRequestCount();
																			uStatus = AddUAVariable(PublishRequestCountId, szPublishRequestCount, typeDefinition, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
																			if (uStatus == OpcUa_Good)
																			{
																				uStatus = AddUAReference(aNodeId, PublishRequestCountId, aNodeIdHasComponent);
																				if (uStatus == OpcUa_Good)
																				{
																					// Adjust timeStamp and statuscode
																					InitTimeStampAndStatusCode(PublishRequestCountId);
																					// HasComponent DataChangeNotificationsCount
																					OpcUa_String szDataChangeNotificationsCount;
																					OpcUa_String_Initialize(&szDataChangeNotificationsCount);
																					OpcUa_NodeId DataChangeNotificationsCountId;
																					OpcUa_NodeId_Initialize(&DataChangeNotificationsCountId);
																					ZeroMemory(szNodeId, 50);
																					sprintf(szNodeId, "DataChangeNotificationsCount-%u", pSubscriptionDiagnosticsDataType->GetSubscriptionId());
																					OpcUa_String_AttachCopy(&szDataChangeNotificationsCount, szNodeId);
																					DataChangeNotificationsCountId.IdentifierType = OpcUa_IdentifierType_String;
																					DataChangeNotificationsCountId.NamespaceIndex = 1;
																					OpcUa_String_CopyTo(&szDataChangeNotificationsCount, &DataChangeNotificationsCountId.Identifier.String);
																					// BrowseName
																					OpcUa_String_Clear(&szDataChangeNotificationsCount);
																					OpcUa_String_Initialize(&szDataChangeNotificationsCount);
																					OpcUa_String_AttachCopy(&szDataChangeNotificationsCount, "DataChangeNotificationsCount");
																					// Value
																					OpcUa_Variant_Clear(&varVal);
																					OpcUa_Variant_Initialize(&varVal);
																					varVal.Datatype = OpcUaType_UInt32;
																					varVal.Value.UInt32 = pSubscriptionDiagnosticsDataType->GetDataChangeNotificationsCount();
																					uStatus = AddUAVariable(DataChangeNotificationsCountId, szDataChangeNotificationsCount, typeDefinition, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
																					if (uStatus == OpcUa_Good)
																					{
																						uStatus = AddUAReference(aNodeId, DataChangeNotificationsCountId, aNodeIdHasComponent);
																						if (uStatus == OpcUa_Good)
																						{
																							// Adjust timeStamp and statuscode
																							InitTimeStampAndStatusCode(DataChangeNotificationsCountId);
																							// HasComponent NotificationsCount
																							OpcUa_String szNotificationsCount;
																							OpcUa_String_Initialize(&szNotificationsCount);
																							OpcUa_NodeId NotificationsCountId;
																							OpcUa_NodeId_Initialize(&NotificationsCountId);
																							ZeroMemory(szNodeId, 50);
																							sprintf(szNodeId, "NotificationsCount-%u", pSubscriptionDiagnosticsDataType->GetSubscriptionId());
																							OpcUa_String_AttachCopy(&szNotificationsCount, szNodeId);
																							NotificationsCountId.IdentifierType = OpcUa_IdentifierType_String;
																							NotificationsCountId.NamespaceIndex = 1;
																							OpcUa_String_CopyTo(&szNotificationsCount, &NotificationsCountId.Identifier.String);
																							// BrowseName
																							OpcUa_String_Clear(&szNotificationsCount);
																							OpcUa_String_Initialize(&szNotificationsCount);
																							OpcUa_String_AttachCopy(&szNotificationsCount, "NotificationsCount");
																							// value
																							OpcUa_Variant_Clear(&varVal);
																							OpcUa_Variant_Initialize(&varVal);
																							varVal.Datatype = OpcUaType_UInt32;
																							varVal.Value.UInt32 = pSubscriptionDiagnosticsDataType->GetNotificationsCount();
																							uStatus = AddUAVariable(NotificationsCountId, szNotificationsCount, typeDefinition, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
																							if (uStatus == OpcUa_Good)
																							{
																								uStatus = AddUAReference(aNodeId, NotificationsCountId, aNodeIdHasComponent);
																								if (uStatus == OpcUa_Good)
																								{
																									// Adjust timeStamp and statuscode
																									InitTimeStampAndStatusCode(NotificationsCountId);
																									// HasComponent UnacknowledgedMessageCount
																									OpcUa_String szUnacknowledgedMessageCount;
																									OpcUa_String_Initialize(&szUnacknowledgedMessageCount);
																									OpcUa_NodeId UnacknowledgedMessageCountId;
																									OpcUa_NodeId_Initialize(&UnacknowledgedMessageCountId);
																									ZeroMemory(szNodeId, 50);
																									sprintf(szNodeId, "UnacknowledgedMessageCount-%u", pSubscriptionDiagnosticsDataType->GetSubscriptionId());
																									OpcUa_String_AttachCopy(&szUnacknowledgedMessageCount, szNodeId);
																									UnacknowledgedMessageCountId.IdentifierType = OpcUa_IdentifierType_String;
																									UnacknowledgedMessageCountId.NamespaceIndex = 1;
																									OpcUa_String_CopyTo(&szUnacknowledgedMessageCount, &UnacknowledgedMessageCountId.Identifier.String);
																									// BrowseName
																									OpcUa_String_Clear(&szUnacknowledgedMessageCount);
																									OpcUa_String_Initialize(&szUnacknowledgedMessageCount);
																									OpcUa_String_AttachCopy(&szUnacknowledgedMessageCount, "UnacknowledgedMessageCount");
																									// Value
																									OpcUa_Variant_Clear(&varVal);
																									OpcUa_Variant_Initialize(&varVal);
																									varVal.Datatype = OpcUaType_UInt32;
																									varVal.Value.UInt32 = pSubscriptionDiagnosticsDataType->GetUnacknowledgedMessageCount();
																									uStatus = AddUAVariable(UnacknowledgedMessageCountId, szUnacknowledgedMessageCount, typeDefinition, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
																									if (uStatus == OpcUa_Good)
																									{
																										uStatus = AddUAReference(aNodeId, UnacknowledgedMessageCountId, aNodeIdHasComponent);
																										if (uStatus == OpcUa_Good)
																										{
																											// Adjust timeStamp and statuscode
																											InitTimeStampAndStatusCode(UnacknowledgedMessageCountId);
																											// ModifyCount
																											OpcUa_String szModifyCount;
																											OpcUa_String_Initialize(&szModifyCount);
																											OpcUa_NodeId ModifyCountId;
																											OpcUa_NodeId_Initialize(&ModifyCountId);
																											ZeroMemory(szNodeId, 50);
																											sprintf(szNodeId, "ModifyCount-%u", pSubscriptionDiagnosticsDataType->GetSubscriptionId());
																											OpcUa_String_AttachCopy(&szModifyCount, szNodeId);
																											ModifyCountId.IdentifierType = OpcUa_IdentifierType_String;
																											ModifyCountId.NamespaceIndex = 1;
																											OpcUa_String_CopyTo(&szModifyCount, &ModifyCountId.Identifier.String);
																											// BrowseName
																											OpcUa_String_Clear(&szModifyCount);
																											OpcUa_String_Initialize(&szModifyCount);
																											OpcUa_String_AttachCopy(&szModifyCount, "ModifyCount");
																											// Value
																											OpcUa_Variant_Clear(&varVal);
																											OpcUa_Variant_Initialize(&varVal);
																											varVal.Datatype = OpcUaType_UInt32;
																											varVal.Value.UInt32 = pSubscriptionDiagnosticsDataType->GetModifyCount();
																											uStatus = AddUAVariable(ModifyCountId, szModifyCount, typeDefinition, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
																											if (uStatus == OpcUa_Good)
																											{
																												uStatus = AddUAReference(aNodeId, ModifyCountId, aNodeIdHasComponent);
																												if (uStatus == OpcUa_Good)
																												{
																													// Adjust timeStamp and statuscode
																													InitTimeStampAndStatusCode(ModifyCountId);
																													// RepublishMessageCount
																													OpcUa_String szRepublishMessageCount;
																													OpcUa_String_Initialize(&szRepublishMessageCount);
																													OpcUa_NodeId RepublishMessageCountId;
																													OpcUa_NodeId_Initialize(&RepublishMessageCountId);
																													ZeroMemory(szNodeId, 50);
																													sprintf(szNodeId, "RepublishMessageCount-%u", pSubscriptionDiagnosticsDataType->GetSubscriptionId());
																													OpcUa_String_AttachCopy(&szRepublishMessageCount, szNodeId);
																													RepublishMessageCountId.IdentifierType = OpcUa_IdentifierType_String;
																													RepublishMessageCountId.NamespaceIndex = 1;
																													OpcUa_String_CopyTo(&szRepublishMessageCount, &RepublishMessageCountId.Identifier.String);
																													// BrowseName
																													OpcUa_String_Clear(&szRepublishMessageCount);
																													OpcUa_String_Initialize(&szRepublishMessageCount);
																													OpcUa_String_AttachCopy(&szRepublishMessageCount, "RepublishMessageCount");
																													// Value
																													OpcUa_Variant_Clear(&varVal);
																													OpcUa_Variant_Initialize(&varVal);
																													varVal.Datatype = OpcUaType_UInt32;
																													varVal.Value.UInt32 = pSubscriptionDiagnosticsDataType->GetRepublishMessageCount();
																													uStatus = AddUAVariable(RepublishMessageCountId, szRepublishMessageCount, typeDefinition, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
																													if (uStatus == OpcUa_Good)
																													{
																														uStatus = AddUAReference(aNodeId, RepublishMessageCountId, aNodeIdHasComponent);
																														if (uStatus == OpcUa_Good)
																														{
																															// Adjust timeStamp and statuscode
																															InitTimeStampAndStatusCode(RepublishMessageCountId);
																															// DisabledMonitoredItemCount
																															OpcUa_String szDisabledMonitoredItemCount;
																															OpcUa_String_Initialize(&szDisabledMonitoredItemCount);
																															OpcUa_NodeId DisabledMonitoredItemCountId;
																															OpcUa_NodeId_Initialize(&DisabledMonitoredItemCountId);
																															ZeroMemory(szNodeId, 50);
																															sprintf(szNodeId, "DisabledMonitoredItemCount-%u", pSubscriptionDiagnosticsDataType->GetSubscriptionId());
																															OpcUa_String_AttachCopy(&szDisabledMonitoredItemCount, szNodeId);
																															DisabledMonitoredItemCountId.IdentifierType = OpcUa_IdentifierType_String;
																															DisabledMonitoredItemCountId.NamespaceIndex = 1;
																															OpcUa_String_CopyTo(&szDisabledMonitoredItemCount, &DisabledMonitoredItemCountId.Identifier.String);
																															// BrowseName
																															OpcUa_String_Clear(&szDisabledMonitoredItemCount);
																															OpcUa_String_Initialize(&szDisabledMonitoredItemCount);
																															OpcUa_String_AttachCopy(&szDisabledMonitoredItemCount, "DisabledMonitoredItemCount");
																															// Value
																															OpcUa_Variant_Clear(&varVal);
																															OpcUa_Variant_Initialize(&varVal);
																															varVal.Datatype = OpcUaType_UInt32;
																															varVal.Value.UInt32 = pSubscriptionDiagnosticsDataType->GetDisabledMonitoredItemCount();
																															uStatus = AddUAVariable(DisabledMonitoredItemCountId, szDisabledMonitoredItemCount, typeDefinition, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
																															if (uStatus == OpcUa_Good)
																															{
																																uStatus = AddUAReference(aNodeId, DisabledMonitoredItemCountId, aNodeIdHasComponent);
																																if (uStatus == OpcUa_Good)
																																{
																																	// Adjust timeStamp and statuscode
																																	InitTimeStampAndStatusCode(DisabledMonitoredItemCountId);
																																	// MonitoringQueueOverflowCount
																																	OpcUa_String szMonitoringQueueOverflowCount;
																																	OpcUa_String_Initialize(&szMonitoringQueueOverflowCount);
																																	OpcUa_NodeId MonitoringQueueOverflowCountId;
																																	OpcUa_NodeId_Initialize(&MonitoringQueueOverflowCountId);
																																	ZeroMemory(szNodeId, 50);
																																	sprintf(szNodeId, "MonitoringQueueOverflowCount-%u", pSubscriptionDiagnosticsDataType->GetSubscriptionId());
																																	OpcUa_String_AttachCopy(&szMonitoringQueueOverflowCount, szNodeId);
																																	MonitoringQueueOverflowCountId.IdentifierType = OpcUa_IdentifierType_String;
																																	MonitoringQueueOverflowCountId.NamespaceIndex = 1;
																																	OpcUa_String_CopyTo(&szMonitoringQueueOverflowCount, &MonitoringQueueOverflowCountId.Identifier.String);
																																	// BrowseName
																																	OpcUa_String_Clear(&szMonitoringQueueOverflowCount);
																																	OpcUa_String_Initialize(&szMonitoringQueueOverflowCount);
																																	OpcUa_String_AttachCopy(&szMonitoringQueueOverflowCount, "MonitoringQueueOverflowCount");
																																	// Value
																																	OpcUa_Variant_Clear(&varVal);
																																	OpcUa_Variant_Initialize(&varVal);
																																	varVal.Datatype = OpcUaType_UInt32;
																																	varVal.Value.UInt32 = pSubscriptionDiagnosticsDataType->GetMonitoringQueueOverflowCount();
																																	uStatus = AddUAVariable(MonitoringQueueOverflowCountId, szMonitoringQueueOverflowCount, typeDefinition, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
																																	if (uStatus == OpcUa_Good)
																																	{
																																		uStatus = AddUAReference(aNodeId, MonitoringQueueOverflowCountId, aNodeIdHasComponent);
																																		if (uStatus == OpcUa_Good)
																																		{
																																			// Adjust timeStamp and statuscode
																																			InitTimeStampAndStatusCode(MonitoringQueueOverflowCountId);
																																			// etc. TODO 

																																		}
																																	}
																																	else
																																	{
																																		char* szNodeId = OpcUa_Null;
																																		Utils::NodeId2String(&aNodeIdSessionId, &szNodeId);
																																		if (szNodeId)
																																		{
																																			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "AddUAVariable: %s failed : 0x%05x.\n", szNodeId, uStatus);
																																			free(szNodeId);
																																		}
																																	}
																																	OpcUa_String_Clear(&szMonitoringQueueOverflowCount);
																																	OpcUa_NodeId_Clear(&MonitoringQueueOverflowCountId);
																																}
																															}
																															else
																															{
																																char* szNodeId = OpcUa_Null;
																																Utils::NodeId2String(&aNodeIdSessionId, &szNodeId);
																																if (szNodeId)
																																{
																																	OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "AddUAVariable: %s failed : 0x%05x.\n", szNodeId, uStatus);
																																	free(szNodeId);
																																}
																															}
																															OpcUa_String_Clear(&szDisabledMonitoredItemCount);
																															OpcUa_NodeId_Clear(&DisabledMonitoredItemCountId);
																														}
																													}
																													else
																													{
																														char* szNodeId = OpcUa_Null;
																														Utils::NodeId2String(&aNodeIdSessionId, &szNodeId);
																														if (szNodeId)
																														{
																															OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "AddUAVariable: %s failed : 0x%05x.\n", szNodeId, uStatus);
																															free(szNodeId);
																														}
																													}
																													OpcUa_String_Clear(&szRepublishMessageCount);
																													OpcUa_NodeId_Clear(&RepublishMessageCountId);
																												}
																											}
																											else
																											{
																												char* szNodeId = OpcUa_Null;
																												Utils::NodeId2String(&aNodeIdSessionId, &szNodeId);
																												if (szNodeId)
																												{
																													OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "AddUAVariable: %s failed : 0x%05x.\n", szNodeId, uStatus);
																													free(szNodeId);
																												}
																											}
																											OpcUa_String_Clear(&szModifyCount);
																											OpcUa_NodeId_Clear(&ModifyCountId);
																										}
																									}
																									else
																									{
																										char* szNodeId = OpcUa_Null;
																										Utils::NodeId2String(&aNodeIdSessionId, &szNodeId);
																										if (szNodeId)
																										{
																											OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "AddUAVariable: %s failed : 0x%05x.\n", szNodeId, uStatus);
																											free(szNodeId);
																										}
																									}
																									OpcUa_String_Clear(&szUnacknowledgedMessageCount);
																									OpcUa_NodeId_Clear(&UnacknowledgedMessageCountId);
																								}
																							}
																							else
																							{
																								char* szNodeId = OpcUa_Null;
																								Utils::NodeId2String(&aNodeIdSessionId, &szNodeId);
																								if (szNodeId)
																								{
																									OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "AddUAVariable: %s failed : 0x%05x.\n", szNodeId, uStatus);
																									free(szNodeId);
																								}
																							}
																							OpcUa_String_Clear(&szNotificationsCount);
																							OpcUa_NodeId_Clear(&NotificationsCountId);
																						}
																					}
																					else
																					{
																						char* szNodeId = OpcUa_Null;
																						Utils::NodeId2String(&aNodeIdSessionId, &szNodeId);
																						if (szNodeId)
																						{
																							OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "AddUAVariable: %s failed : 0x%05x.\n", szNodeId, uStatus);
																							free(szNodeId);
																						}
																					}
																					OpcUa_String_Clear(&szDataChangeNotificationsCount);
																					OpcUa_NodeId_Clear(&DataChangeNotificationsCountId);
																				}
																			}
																			else
																			{
																				char* szNodeId = OpcUa_Null;
																				Utils::NodeId2String(&aNodeIdSessionId, &szNodeId);
																				if (szNodeId)
																				{
																					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "AddUAVariable: %s failed : 0x%05x.\n", szNodeId, uStatus);
																					free(szNodeId);
																				}
																			}
																			OpcUa_String_Clear(&szPublishRequestCount);
																			OpcUa_NodeId_Clear(&PublishRequestCountId);
																		}
																	}
																	else
																	{
																		char* szNodeId = OpcUa_Null;
																		Utils::NodeId2String(&aNodeIdSessionId, &szNodeId);
																		if (szNodeId)
																		{
																			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "AddUAVariable: %s failed : 0x%05x.\n", szNodeId, uStatus);
																			free(szNodeId);
																		}
																	}
																	OpcUa_String_Clear(&szMaxKeepAliveCount);
																	OpcUa_NodeId_Clear(&MaxKeepAliveCountId);
																}
															}
															else
															{
																char* szNodeId = OpcUa_Null;
																Utils::NodeId2String(&aNodeIdSessionId, &szNodeId);
																if (szNodeId)
																{
																	OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "AddUAVariable: %s failed : 0x%05x.\n", szNodeId, uStatus);
																	free(szNodeId);
																}
															}
															OpcUa_String_Clear(&szMaxLifetimeCount);
															OpcUa_NodeId_Clear(&MaxLifetimeCountId);
														}
													}
													else
													{
														char* szNodeId = OpcUa_Null;
														Utils::NodeId2String(&aNodeIdSessionId, &szNodeId);
														if (szNodeId)
														{
															OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "AddUAVariable: %s failed : 0x%05x.\n", szNodeId, uStatus);
															free(szNodeId);
														}
													}
													OpcUa_String_Clear(&szPublishingInterval);
													OpcUa_NodeId_Clear(&PublishingIntervalId);
												}
											}
											else
											{
												char* szNodeId = OpcUa_Null;
												Utils::NodeId2String(&aNodeIdSessionId, &szNodeId);
												if (szNodeId)
												{
													OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "AddUAVariable: %s failed : 0x%05x.\n", szNodeId, uStatus);
													free(szNodeId);
												}
											}
											OpcUa_String_Clear(&szMaxNotificationsPerPublish);
											OpcUa_NodeId_Clear(&MaxNotificationsPerPublishId);
										}
									}
									else
									{
										char* szNodeId = OpcUa_Null;
										Utils::NodeId2String(&aNodeIdSessionId, &szNodeId);
										if (szNodeId)
										{
											OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "AddUAVariable: %s failed : 0x%05x.\n", szNodeId, uStatus);
											free(szNodeId);
										}
									}
									OpcUa_String_Clear(&szPublishingEnabled);
									OpcUa_NodeId_Clear(&PublishingEnabledId);
								}
							}
							else
							{
								char* szNodeId = OpcUa_Null;
								Utils::NodeId2String(&aNodeIdSessionId, &szNodeId);
								if (szNodeId)
								{
									OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "AddUAVariable: %s failed : 0x%05x.\n", szNodeId, uStatus);
									free(szNodeId);
								}
							}
							OpcUa_String_Clear(&szPriority);
							OpcUa_NodeId_Clear(&priorityId);
						}
					}
					else
					{
						char* szNodeId = OpcUa_Null;
						Utils::NodeId2String(&aNodeIdSessionId, &szNodeId);
						if (szNodeId)
						{
							OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "AddUAVariable: %s failed : 0x%05x.\n", szNodeId, uStatus);
							free(szNodeId);
						}
					}
					OpcUa_String_Clear(&szSubscriptionId);
					OpcUa_NodeId_Clear(&subscriptionId);
				}
				else
					OpcUa_Trace(OPCUA_TRACE_SERVER_ERROR, "Diagnostic error>AddUAReference in %s failed\n", OpcUa_String_GetRawString(&aNodeId.Identifier.String));
			}
			else
			{
				OpcUa_NodeId_Clear(varVal.Value.NodeId);
				OpcUa_Free(varVal.Value.NodeId);
				varVal.Value.NodeId = OpcUa_Null;
				char* szNodeId = OpcUa_Null;
				Utils::NodeId2String(&aNodeIdSessionId, &szNodeId);
				if (szNodeId)
				{
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "AddUAVariable: %s failed : 0x%05x.\n", szNodeId, uStatus);
					free(szNodeId);
				}
			}
			OpcUa_NodeId_Clear(&aNodeIdSessionId);
			OpcUa_String_Clear(&SessionId);
			OpcUa_Free(szNodeId);
		}
		else
			OpcUa_Trace(OPCUA_TRACE_SERVER_ERROR, "Diagnostic error>AddUAReference in i=2290 failed\n");
		OpcUa_Variant_Clear(&varVal);
		OpcUa_NodeId_Initialize(&aNodeId2290);
		OpcUa_NodeId_Clear(&aNodeIdHasComponent);
	}
	else
	{
		char* szNodeId = OpcUa_Null;
		Utils::NodeId2String(&aNodeId, &szNodeId);
		if (szNodeId)
		{
			// Attribute
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "AddUAVariable: %s failed : 0x%05x.\n", szNodeId,uStatus);
			free(szNodeId);
		}
	}
	// Adjust timeStamp and statuscode
	InitTimeStampAndStatusCode(aNodeId);
	if (varVal.Value.ExtensionObject)
	{
		OpcUa_ExtensionObject_Clear(varVal.Value.ExtensionObject);
		OpcUa_Free(varVal.Value.ExtensionObject);
		varVal.Value.ExtensionObject = OpcUa_Null;
	}
	OpcUa_Variant_Clear(&varVal);
	OpcUa_NodeId_Clear(&typeDefinition);
	OpcUa_NodeId_Clear(&aNodeId);
	OpcUa_String_Clear(&szSubscriptionId);
	OpcUa_Free(pszSubscriptionId);
	OpcUa_Mutex_Unlock(m_ServerCacheMutex);
	return uStatus;	
}
OpcUa_StatusCode CUAInformationModel::UpdateSubscriptionDiagnosticsValueInAddressSpace(CSubscriptionDiagnosticsDataType* pSubscriptionDiagnosticsDataType)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_Mutex_Lock(m_ServerCacheMutex);
	if (pSubscriptionDiagnosticsDataType)
	{
		OpcUa_CharA* pszNodeId = (OpcUa_CharA*)OpcUa_Alloc(50);
		OpcUa_String szNodeId;
		OpcUa_String_Initialize(&szNodeId);
		OpcUa_NodeId aNodeId;
		// Priority
		OpcUa_NodeId_Initialize(&aNodeId);
		ZeroMemory(pszNodeId, 50);
		sprintf(pszNodeId, "priority-%u", pSubscriptionDiagnosticsDataType->GetSubscriptionId());
		OpcUa_String_AttachCopy(&szNodeId, pszNodeId);
		aNodeId.IdentifierType = OpcUa_IdentifierType_String;
		aNodeId.NamespaceIndex = 1;
		OpcUa_String_CopyTo(&szNodeId, &aNodeId.Identifier.String);

		CUAVariable* pUAVariable = OpcUa_Null;
		uStatus = GetNodeIdFromVariableList(aNodeId, &pUAVariable);
		if (uStatus == OpcUa_Good)
		{
			CDataValue* pDataValue = pUAVariable->GetValue();
			if (pDataValue)
			{
				OpcUa_DataValue* pInternalDataValue = pDataValue->GetInternalDataValue();
				if (pInternalDataValue)
				{
					pInternalDataValue->Value.Value.Byte = pSubscriptionDiagnosticsDataType->GetPriority();
					pInternalDataValue->ServerTimestamp = OpcUa_DateTime_UtcNow();
					pDataValue->SetServerTimestamp(pInternalDataValue->ServerTimestamp);
					pInternalDataValue->SourceTimestamp = OpcUa_DateTime_UtcNow();
					pDataValue->SetSourceTimestamp(pInternalDataValue->SourceTimestamp);
					pInternalDataValue->StatusCode = OpcUa_Good;
					pDataValue->SetStatusCode(pInternalDataValue->StatusCode);
				}
			}
		}
		else
		{
			char* szNodeId = OpcUa_Null;
			Utils::NodeId2String(&aNodeId, &szNodeId);
			if (szNodeId)
			{
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "UpdateSubscriptionDiagnosticsValueInAddressSpace: %s doesn't exist : 0x%05x.\n", szNodeId, uStatus);
				free(szNodeId);
			}
		}
		OpcUa_String_Clear(&szNodeId);
		OpcUa_NodeId_Clear(&aNodeId);
		// PublishRequestCount
		OpcUa_String_Initialize(&szNodeId);
		OpcUa_NodeId_Initialize(&aNodeId);
		ZeroMemory(pszNodeId, 50);
		sprintf(pszNodeId, "PublishRequestCount-%u", pSubscriptionDiagnosticsDataType->GetSubscriptionId());
		OpcUa_String_AttachCopy(&szNodeId, pszNodeId);
		aNodeId.IdentifierType = OpcUa_IdentifierType_String;
		aNodeId.NamespaceIndex = 1;
		OpcUa_String_CopyTo(&szNodeId, &aNodeId.Identifier.String);

		uStatus = GetNodeIdFromVariableList(aNodeId, &pUAVariable);
		if (uStatus == OpcUa_Good)
		{
			CDataValue* pDataValue = pUAVariable->GetValue();
			if (pDataValue)
			{
				OpcUa_DataValue* pInternalDataValue = pDataValue->GetInternalDataValue();
				if (pInternalDataValue)
				{
					pInternalDataValue->Value.Value.UInt32 = pSubscriptionDiagnosticsDataType->GetPublishRequestCount();
					pInternalDataValue->ServerTimestamp = OpcUa_DateTime_UtcNow();
					pDataValue->SetServerTimestamp(pInternalDataValue->ServerTimestamp);
					pInternalDataValue->SourceTimestamp = OpcUa_DateTime_UtcNow();
					pDataValue->SetSourceTimestamp(pInternalDataValue->SourceTimestamp);
					pInternalDataValue->StatusCode = OpcUa_Good;
					pDataValue->SetStatusCode(pInternalDataValue->StatusCode);
				}
			}
		}
		else
		{
			char* szNodeId = OpcUa_Null;
			Utils::NodeId2String(&aNodeId, &szNodeId);
			if (szNodeId)
			{
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "UpdateSubscriptionDiagnosticsValueInAddressSpace: %s doesn't exist : 0x%05x.\n", szNodeId, uStatus);
				free(szNodeId);
			}
		}
		OpcUa_String_Clear(&szNodeId);
		OpcUa_NodeId_Clear(&aNodeId);

		// DataChangeNotificationsCount
		OpcUa_String_Initialize(&szNodeId);
		OpcUa_NodeId_Initialize(&aNodeId);
		ZeroMemory(pszNodeId, 50);
		sprintf(pszNodeId, "DataChangeNotificationsCount-%u", pSubscriptionDiagnosticsDataType->GetSubscriptionId());
		OpcUa_String_AttachCopy(&szNodeId, pszNodeId);
		aNodeId.IdentifierType = OpcUa_IdentifierType_String;
		aNodeId.NamespaceIndex = 1;
		OpcUa_String_CopyTo(&szNodeId, &aNodeId.Identifier.String);

		uStatus = GetNodeIdFromVariableList(aNodeId, &pUAVariable);
		if (uStatus == OpcUa_Good)
		{
			CDataValue* pDataValue = pUAVariable->GetValue();
			if (pDataValue)
			{
				OpcUa_DataValue* pInternalDataValue = pDataValue->GetInternalDataValue();
				if (pInternalDataValue)
				{
					pInternalDataValue->Value.Value.UInt32 = pSubscriptionDiagnosticsDataType->GetDataChangeNotificationsCount();
					pInternalDataValue->ServerTimestamp = OpcUa_DateTime_UtcNow();
					pDataValue->SetServerTimestamp(pInternalDataValue->ServerTimestamp);
					pInternalDataValue->SourceTimestamp = OpcUa_DateTime_UtcNow();
					pDataValue->SetSourceTimestamp(pInternalDataValue->SourceTimestamp);
					pInternalDataValue->StatusCode = OpcUa_Good;
					pDataValue->SetStatusCode(pInternalDataValue->StatusCode);
				}
			}
		}
		else
		{
			char* szNodeId = OpcUa_Null;
			Utils::NodeId2String(&aNodeId, &szNodeId);
			if (szNodeId)
			{
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "UpdateSubscriptionDiagnosticsValueInAddressSpace: %s doesn't exist : 0x%05x.\n", szNodeId, uStatus);
				free(szNodeId);
			}
		}
		OpcUa_String_Clear(&szNodeId);
		OpcUa_NodeId_Clear(&aNodeId);

		// NotificationsCount
		OpcUa_String_Initialize(&szNodeId);
		OpcUa_NodeId_Initialize(&aNodeId);
		ZeroMemory(pszNodeId, 50);
		sprintf(pszNodeId, "NotificationsCount-%u", pSubscriptionDiagnosticsDataType->GetSubscriptionId());
		OpcUa_String_AttachCopy(&szNodeId, pszNodeId);
		aNodeId.IdentifierType = OpcUa_IdentifierType_String;
		aNodeId.NamespaceIndex = 1;
		OpcUa_String_CopyTo(&szNodeId, &aNodeId.Identifier.String);

		uStatus = GetNodeIdFromVariableList(aNodeId, &pUAVariable);
		if (uStatus == OpcUa_Good)
		{
			CDataValue* pDataValue = pUAVariable->GetValue();
			if (pDataValue)
			{
				OpcUa_DataValue* pInternalDataValue = pDataValue->GetInternalDataValue();
				if (pInternalDataValue)
				{
					pInternalDataValue->Value.Value.UInt32 = pSubscriptionDiagnosticsDataType->GetNotificationsCount();
					pInternalDataValue->ServerTimestamp = OpcUa_DateTime_UtcNow();
					pDataValue->SetServerTimestamp(pInternalDataValue->ServerTimestamp);
					pInternalDataValue->SourceTimestamp = OpcUa_DateTime_UtcNow();
					pDataValue->SetSourceTimestamp(pInternalDataValue->SourceTimestamp);
					pInternalDataValue->StatusCode = OpcUa_Good;
					pDataValue->SetStatusCode(pInternalDataValue->StatusCode);
				}
			}
		}
		else
		{
			char* szNodeId = OpcUa_Null;
			Utils::NodeId2String(&aNodeId, &szNodeId);
			if (szNodeId)
			{
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "UpdateSubscriptionDiagnosticsValueInAddressSpace: %s doesn't exist : 0x%05x.\n", szNodeId, uStatus);
				free(szNodeId);
			}
		}
		OpcUa_String_Clear(&szNodeId);
		OpcUa_NodeId_Clear(&aNodeId);

		// UnacknowledgedMessageCount
		OpcUa_String_Initialize(&szNodeId);
		OpcUa_NodeId_Initialize(&aNodeId);
		ZeroMemory(pszNodeId, 50);
		sprintf(pszNodeId, "UnacknowledgedMessageCount-%u", pSubscriptionDiagnosticsDataType->GetSubscriptionId());
		OpcUa_String_AttachCopy(&szNodeId, pszNodeId);
		aNodeId.IdentifierType = OpcUa_IdentifierType_String;
		aNodeId.NamespaceIndex = 1;
		OpcUa_String_CopyTo(&szNodeId, &aNodeId.Identifier.String);

		uStatus = GetNodeIdFromVariableList(aNodeId, &pUAVariable);
		if (uStatus == OpcUa_Good)
		{
			CDataValue* pDataValue = pUAVariable->GetValue();
			if (pDataValue)
			{
				OpcUa_DataValue* pInternalDataValue = pDataValue->GetInternalDataValue();
				if (pInternalDataValue)
				{
					pInternalDataValue->Value.Value.UInt32 = pSubscriptionDiagnosticsDataType->GetUnacknowledgedMessageCount();
					pInternalDataValue->ServerTimestamp = OpcUa_DateTime_UtcNow();
					pDataValue->SetServerTimestamp(pInternalDataValue->ServerTimestamp);
					pInternalDataValue->SourceTimestamp = OpcUa_DateTime_UtcNow();
					pDataValue->SetSourceTimestamp(pInternalDataValue->SourceTimestamp);
					pInternalDataValue->StatusCode = OpcUa_Good;
					pDataValue->SetStatusCode(pInternalDataValue->StatusCode);
				}
			}
		}
		else
		{
			char* szNodeId = OpcUa_Null;
			Utils::NodeId2String(&aNodeId, &szNodeId);
			if (szNodeId)
			{
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "UpdateSubscriptionDiagnosticsValueInAddressSpace: %s doesn't exist : 0x%05x.\n", szNodeId, uStatus);
				free(szNodeId);
			}
		}
		OpcUa_String_Clear(&szNodeId);
		OpcUa_NodeId_Clear(&aNodeId);
		// PublishingEnabled
		OpcUa_String_Initialize(&szNodeId);
		OpcUa_NodeId_Initialize(&aNodeId);
		ZeroMemory(pszNodeId, 50);
		sprintf(pszNodeId, "PublishingEnabled-%u", pSubscriptionDiagnosticsDataType->GetSubscriptionId());
		OpcUa_String_AttachCopy(&szNodeId, pszNodeId);
		aNodeId.IdentifierType = OpcUa_IdentifierType_String;
		aNodeId.NamespaceIndex = 1;
		OpcUa_String_CopyTo(&szNodeId, &aNodeId.Identifier.String);

		uStatus = GetNodeIdFromVariableList(aNodeId, &pUAVariable);
		if (uStatus == OpcUa_Good)
		{
			CDataValue* pDataValue = pUAVariable->GetValue();
			if (pDataValue)
			{
				OpcUa_DataValue* pInternalDataValue = pDataValue->GetInternalDataValue();
				if (pInternalDataValue)
				{
					pInternalDataValue->Value.Value.Boolean = pSubscriptionDiagnosticsDataType->GetPublishingEnabled();
					pInternalDataValue->ServerTimestamp = OpcUa_DateTime_UtcNow();
					pDataValue->SetServerTimestamp(pInternalDataValue->ServerTimestamp);
					pInternalDataValue->SourceTimestamp = OpcUa_DateTime_UtcNow();
					pDataValue->SetSourceTimestamp(pInternalDataValue->SourceTimestamp);
					pInternalDataValue->StatusCode = OpcUa_Good;
					pDataValue->SetStatusCode(pInternalDataValue->StatusCode);
				}
			}
		}
		else
		{
			char* szNodeId = OpcUa_Null;
			Utils::NodeId2String(&aNodeId, &szNodeId);
			if (szNodeId)
			{
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "UpdateSubscriptionDiagnosticsValueInAddressSpace: %s doesn't exist : 0x%05x.\n", szNodeId, uStatus);
				free(szNodeId);
			}
		}
		OpcUa_String_Clear(&szNodeId);
		OpcUa_NodeId_Clear(&aNodeId);
		// ModifyCount
		OpcUa_String_Initialize(&szNodeId);
		OpcUa_NodeId_Initialize(&aNodeId);
		ZeroMemory(pszNodeId, 50);
		sprintf(pszNodeId, "ModifyCount-%u", pSubscriptionDiagnosticsDataType->GetSubscriptionId());
		OpcUa_String_AttachCopy(&szNodeId, pszNodeId);
		aNodeId.IdentifierType = OpcUa_IdentifierType_String;
		aNodeId.NamespaceIndex = 1;
		OpcUa_String_CopyTo(&szNodeId, &aNodeId.Identifier.String);

		uStatus = GetNodeIdFromVariableList(aNodeId, &pUAVariable);
		if (uStatus == OpcUa_Good)
		{
			CDataValue* pDataValue = pUAVariable->GetValue();
			if (pDataValue)
			{
				OpcUa_DataValue* pInternalDataValue = pDataValue->GetInternalDataValue();
				if (pInternalDataValue)
				{
					pInternalDataValue->Value.Value.UInt32 = pSubscriptionDiagnosticsDataType->GetModifyCount();
					pInternalDataValue->ServerTimestamp = OpcUa_DateTime_UtcNow();
					pDataValue->SetServerTimestamp(pInternalDataValue->ServerTimestamp);
					pInternalDataValue->SourceTimestamp = OpcUa_DateTime_UtcNow();
					pDataValue->SetSourceTimestamp(pInternalDataValue->SourceTimestamp);
					pInternalDataValue->StatusCode = OpcUa_Good;
					pDataValue->SetStatusCode(pInternalDataValue->StatusCode);
				}
			}
		}
		else
		{
			char* szNodeId = OpcUa_Null;
			Utils::NodeId2String(&aNodeId, &szNodeId);
			if (szNodeId)
			{
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "UpdateSubscriptionDiagnosticsValueInAddressSpace: %s doesn't exist : 0x%05x.\n", szNodeId, uStatus);
				free(szNodeId);
			}
		}
		OpcUa_String_Clear(&szNodeId);
		OpcUa_NodeId_Clear(&aNodeId);
		// RepublishMessageCount
		OpcUa_String_Initialize(&szNodeId);
		OpcUa_NodeId_Initialize(&aNodeId);
		ZeroMemory(pszNodeId, 50);
		sprintf(pszNodeId, "RepublishMessageCount-%u", pSubscriptionDiagnosticsDataType->GetSubscriptionId());
		OpcUa_String_AttachCopy(&szNodeId, pszNodeId);
		aNodeId.IdentifierType = OpcUa_IdentifierType_String;
		aNodeId.NamespaceIndex = 1;
		OpcUa_String_CopyTo(&szNodeId, &aNodeId.Identifier.String);

		uStatus = GetNodeIdFromVariableList(aNodeId, &pUAVariable);
		if (uStatus == OpcUa_Good)
		{
			CDataValue* pDataValue = pUAVariable->GetValue();
			if (pDataValue)
			{
				OpcUa_DataValue* pInternalDataValue = pDataValue->GetInternalDataValue();
				if (pInternalDataValue)
				{
					pInternalDataValue->Value.Value.UInt32 = pSubscriptionDiagnosticsDataType->GetRepublishMessageCount();
					pInternalDataValue->ServerTimestamp = OpcUa_DateTime_UtcNow();
					pDataValue->SetServerTimestamp(pInternalDataValue->ServerTimestamp);
					pInternalDataValue->SourceTimestamp = OpcUa_DateTime_UtcNow();
					pDataValue->SetSourceTimestamp(pInternalDataValue->SourceTimestamp);
					pInternalDataValue->StatusCode = OpcUa_Good;
					pDataValue->SetStatusCode(pInternalDataValue->StatusCode);
				}
			}
		}
		else
		{
			char* szNodeId = OpcUa_Null;
			Utils::NodeId2String(&aNodeId, &szNodeId);
			if (szNodeId)
			{
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "UpdateSubscriptionDiagnosticsValueInAddressSpace: %s doesn't exist : 0x%05x.\n", szNodeId, uStatus);
				free(szNodeId);
			}
		}
		OpcUa_String_Clear(&szNodeId);
		OpcUa_NodeId_Clear(&aNodeId);
		// DisabledMonitoredItemCount
		OpcUa_String_Initialize(&szNodeId);
		OpcUa_NodeId_Initialize(&aNodeId);
		ZeroMemory(pszNodeId, 50);
		sprintf(pszNodeId, "DisabledMonitoredItemCount-%u", pSubscriptionDiagnosticsDataType->GetSubscriptionId());
		OpcUa_String_AttachCopy(&szNodeId, pszNodeId);
		aNodeId.IdentifierType = OpcUa_IdentifierType_String;
		aNodeId.NamespaceIndex = 1;
		OpcUa_String_CopyTo(&szNodeId, &aNodeId.Identifier.String);

		uStatus = GetNodeIdFromVariableList(aNodeId, &pUAVariable);
		if (uStatus == OpcUa_Good)
		{
			CDataValue* pDataValue = pUAVariable->GetValue();
			if (pDataValue)
			{
				OpcUa_DataValue* pInternalDataValue = pDataValue->GetInternalDataValue();
				if (pInternalDataValue)
				{
					pInternalDataValue->Value.Value.UInt32 = pSubscriptionDiagnosticsDataType->GetDisabledMonitoredItemCount();
					pInternalDataValue->ServerTimestamp = OpcUa_DateTime_UtcNow();
					pDataValue->SetServerTimestamp(pInternalDataValue->ServerTimestamp);
					pInternalDataValue->SourceTimestamp = OpcUa_DateTime_UtcNow();
					pDataValue->SetSourceTimestamp(pInternalDataValue->SourceTimestamp);
					pInternalDataValue->StatusCode = OpcUa_Good;
					pDataValue->SetStatusCode(pInternalDataValue->StatusCode);
				}
			}
		}
		else
		{
			char* szNodeId = OpcUa_Null;
			Utils::NodeId2String(&aNodeId, &szNodeId);
			if (szNodeId)
			{
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "UpdateSubscriptionDiagnosticsValueInAddressSpace: %s doesn't exist : 0x%05x.\n", szNodeId, uStatus);
				free(szNodeId);
			}
		}
		OpcUa_String_Clear(&szNodeId);
		OpcUa_NodeId_Clear(&aNodeId);
		// MonitoringQueueOverflowCount
		OpcUa_String_Initialize(&szNodeId);
		OpcUa_NodeId_Initialize(&aNodeId);
		ZeroMemory(pszNodeId, 50);
		sprintf(pszNodeId, "MonitoringQueueOverflowCount-%u", pSubscriptionDiagnosticsDataType->GetSubscriptionId());
		OpcUa_String_AttachCopy(&szNodeId, pszNodeId);
		aNodeId.IdentifierType = OpcUa_IdentifierType_String;
		aNodeId.NamespaceIndex = 1;
		OpcUa_String_CopyTo(&szNodeId, &aNodeId.Identifier.String);

		uStatus = GetNodeIdFromVariableList(aNodeId, &pUAVariable);
		if (uStatus == OpcUa_Good)
		{
			CDataValue* pDataValue = pUAVariable->GetValue();
			if (pDataValue)
			{
				OpcUa_DataValue* pInternalDataValue = pDataValue->GetInternalDataValue();
				if (pInternalDataValue)
				{
					pInternalDataValue->Value.Value.UInt32 = pSubscriptionDiagnosticsDataType->GetMonitoringQueueOverflowCount();
					pInternalDataValue->ServerTimestamp = OpcUa_DateTime_UtcNow();
					pDataValue->SetServerTimestamp(pInternalDataValue->ServerTimestamp);
					pInternalDataValue->SourceTimestamp = OpcUa_DateTime_UtcNow();
					pDataValue->SetSourceTimestamp(pInternalDataValue->SourceTimestamp);
					pInternalDataValue->StatusCode = OpcUa_Good;
					pDataValue->SetStatusCode(pInternalDataValue->StatusCode);
				}
			}
		}
		else
		{
			char* szNodeId = OpcUa_Null;
			Utils::NodeId2String(&aNodeId, &szNodeId);
			if (szNodeId)
			{
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "UpdateSubscriptionDiagnosticsValueInAddressSpace: %s doesn't exist : 0x%05x.\n", szNodeId, uStatus);
				free(szNodeId);
			}
		}
		OpcUa_String_Clear(&szNodeId);
		OpcUa_NodeId_Clear(&aNodeId);
		//
		OpcUa_Free(pszNodeId);
	}
	OpcUa_Mutex_Unlock(m_ServerCacheMutex);
	return uStatus;
}
///-------------------------------------------------------------------------------------------------
/// <summary>	Initialises the time stamp and status code for the NodeId receive in parameter. </summary>
///
/// <remarks>	Michel, 29/05/2016. </remarks>
///
/// <param name="aNodeId">	Identifier for the node. </param>
///-------------------------------------------------------------------------------------------------

void CUAInformationModel::InitTimeStampAndStatusCode(OpcUa_NodeId aNodeId)
{
	// Adjust timeStamp and statuscode
	CUAVariable* pUAVariable = OpcUa_Null;
	if (GetNodeIdFromVariableList(aNodeId, &pUAVariable) == OpcUa_Good)
	{
		CDataValue* pDataValue = pUAVariable->GetValue();
		if (pDataValue)
		{
			OpcUa_DataValue* pInternalDataValue = pDataValue->GetInternalDataValue();
			if (pInternalDataValue)
			{
				pInternalDataValue->ServerTimestamp = OpcUa_DateTime_UtcNow();
				pDataValue->SetServerTimestamp(pInternalDataValue->ServerTimestamp);
				pInternalDataValue->SourceTimestamp = OpcUa_DateTime_UtcNow();
				pDataValue->SetSourceTimestamp(pInternalDataValue->SourceTimestamp);
				pInternalDataValue->StatusCode = OpcUa_Good;
				pDataValue->SetStatusCode(pInternalDataValue->StatusCode);
			}
		}
	}
}
///-------------------------------------------------------------------------------------------------
/// <summary>
/// 	Removes the subscription in address space described by
/// 	pSubscriptionDiagnosticsDataType.
/// </summary>
///
/// <remarks>	Michel, 29/05/2016. </remarks>
///
/// <param name="pSubscriptionDiagnosticsDataType">	[in,out] If non-null, type of the
/// 												subscription diagnostics data.
/// </param>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode OpenOpcUa::UAAddressSpace::CUAInformationModel::RemoveSubscriptionInAddressSpace(CSubscriptionDiagnosticsDataType*pSubscriptionDiagnosticsDataType)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_Mutex_Lock(m_ServerCacheMutex);
	CUAVariable* pUAVariable=OpcUa_Null;
	if (pSubscriptionDiagnosticsDataType)
	{
		// search for CSessionServer
		OpcUa_NodeId sessionId = pSubscriptionDiagnosticsDataType->GetSessionId();
		CSessionServer* pSession = OpcUa_Null;
		uStatus = g_pTheApplication->FindSessionBySessionId(&sessionId, &pSession);
		if (uStatus == OpcUa_Good)
		{
			OpcUa_Mutex subscriptionListMutex = pSession->GetSubscriptionListMutex();
			OpcUa_Mutex_Lock(subscriptionListMutex);
			// Search for the subscription 
			CSubscriptionServer* pSubscription = OpcUa_Null;
			OpcUa_UInt32 uiSubscriptionId = pSubscriptionDiagnosticsDataType->GetSubscriptionId();
			uStatus = pSession->FindSubscription(uiSubscriptionId, &pSubscription);
			if (uStatus == OpcUa_Good)
			{
				OpcUa_Mutex monitoredItemListMutex = pSubscription->GetMonitoredItemListMutex();
				OpcUa_Mutex_Lock(monitoredItemListMutex);
				// Name 
				OpcUa_CharA* pszSubscriptionId = (OpcUa_CharA*)OpcUa_Alloc(20);
				OpcUa_String szSubscriptionId;
				OpcUa_String_Initialize(&szSubscriptionId);
				ZeroMemory(pszSubscriptionId, 20);
				sprintf(pszSubscriptionId, "%u", uiSubscriptionId);
				OpcUa_String_AttachCopy(&szSubscriptionId, pszSubscriptionId);
				// NodeId
				OpcUa_NodeId aNodeId;
				OpcUa_NodeId_Initialize(&aNodeId);
				OpcUa_String_CopyTo(&szSubscriptionId, &(aNodeId.Identifier.String));
				aNodeId.IdentifierType = OpcUa_IdentifierType_String;
				aNodeId.NamespaceIndex = 1;// 
				OpcUa_Mutex cacheMutex = GetServerCacheMutex();
				OpcUa_Mutex_Lock(cacheMutex);
				CUAVariableList* pUAVariableList = GetVariableList();
				if (pUAVariableList)
				{
					CUAVariableList::iterator it = pUAVariableList->find(&aNodeId);
					if (it != pUAVariableList->end())
					{
						pUAVariable = it->second;
						/*
						CDataValue* pDataValue = pUAVariable->GetValue();
						OpcUa_DataValue* pInternalDataValue = pDataValue->GetInternalDataValue();
						if (pInternalDataValue->Value.Value.ExtensionObject)
						{
							OpcUa_ExtensionObject_Clear(pInternalDataValue->Value.Value.ExtensionObject);
							OpcUa_Free(pInternalDataValue->Value.Value.ExtensionObject);
							pInternalDataValue->Value.Value.ExtensionObject = OpcUa_Null;
						}*/
						pUAVariableList->erase(it);
						CUAReferenceList* pUAReferenceList = pUAVariable->GetReferenceNodeList();
						if (pUAReferenceList)
						{
							CUAReferenceList::iterator itRef;
							for (itRef = pUAReferenceList->begin(); itRef != pUAReferenceList->end(); itRef++)
							{
								CUAReference* pReference = *itRef;
								OpcUa_ExpandedNodeId expandedNodeId = pReference->GetTargetId();
								CUAVariableList::iterator itTarget = pUAVariableList->find(&(expandedNodeId.NodeId));
								if (itTarget != pUAVariableList->end())
								{
									CUAVariable* pUATargetVariable = itTarget->second;
									delete pUATargetVariable;
									pUAVariableList->erase(itTarget);
								}
							}
						}
						delete pUAVariable;
						pUAVariable = OpcUa_Null;
						// We need to remove this reference from 2290
						// 2290
						OpcUa_NodeId aNodeId2290;
						OpcUa_NodeId_Initialize(&aNodeId2290);
						aNodeId2290.Identifier.Numeric = 2290;
						aNodeId2290.IdentifierType = OpcUa_IdentifierType_Numeric;
						aNodeId2290.NamespaceIndex = 0;
						if (GetNodeIdFromVariableList(aNodeId2290, &pUAVariable) == OpcUa_Good)
						{
							CUAReferenceList::iterator itRef;
							CUAReferenceList* pReferenceList=pUAVariable->GetReferenceNodeList();
							for (itRef = pReferenceList->begin(); itRef != pReferenceList->end(); itRef++)
							{
								OpcUa_NodeId TargetId = (*itRef)->GetTargetId().NodeId;
								if (Utils::IsEqual(&TargetId, &aNodeId))
								{
									pReferenceList->erase(itRef);
									OpcUa_Trace(OPCUA_TRACE_SERVER_INFO, "Reference in i=2290 properly removed\n");
									break;
								}
							}
						}
						else
							OpcUa_Trace(OPCUA_TRACE_SERVER_ERROR, "Critical Error>Cannot find i=2290 in the AddressSpace\n");;
						OpcUa_Trace(OPCUA_TRACE_SERVER_ERROR, "Remove UAVariable %s succeed\n", OpcUa_String_GetRawString(&aNodeId.Identifier.String));
					}
					else
					{

						OpcUa_Trace(OPCUA_TRACE_SERVER_ERROR, "Remove UAVariable %s failed\n", OpcUa_String_GetRawString(&aNodeId.Identifier.String));
						uStatus = OpcUa_BadNotFound;
					}
				}
				else
					uStatus = OpcUa_BadInvalidArgument;
				OpcUa_Mutex_Unlock(cacheMutex);
				OpcUa_NodeId_Clear(&aNodeId);
				OpcUa_String_Clear(&szSubscriptionId);
				OpcUa_Free(pszSubscriptionId);
				OpcUa_Mutex_Unlock(monitoredItemListMutex);
			}
			else
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "FindSubscription: %u failed : 0x%05x.\n", uiSubscriptionId, uStatus);
			OpcUa_Mutex_Unlock(subscriptionListMutex);
		}
		else
		{
			char* szNodeId = OpcUa_Null;
			Utils::NodeId2String(&sessionId, &szNodeId);
			if (szNodeId)
			{
				// Attribute
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "FindSessionBySessionId: %s failed : 0x%05x.\n", szNodeId, uStatus);
				free(szNodeId);
			}
		}
		OpcUa_NodeId_Clear(&sessionId);
	}
	OpcUa_Mutex_Unlock(m_ServerCacheMutex);
	return uStatus;
}

///-------------------------------------------------------------------------------------------------
/// <summary>
/// 	Script Autorun. This script must exist in the OpenOpcUaCoreServer.lua file. 
/// </summary>
///
/// <remarks>	Michel, 29/07/2016. </remarks>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode OpenOpcUa::UAAddressSpace::CUAInformationModel::Autorun(void)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_CharA* pszLuaMethodName = (OpcUa_CharA*)OpcUa_Null;
	pszLuaMethodName = (OpcUa_CharA*)malloc(8);
	if (pszLuaMethodName)
	{
		ZeroMemory(pszLuaMethodName, 8);
		OpcUa_MemCpy(pszLuaMethodName, 7, "Autorun", 7);
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_INFO, "The OpenOpcUaCoreServer will run the LUA script %s", pszLuaMethodName);

		CLuaVirtualMachine* pVm = GetLuaVirtualMachine();
		COpenOpcUaScript* pMs = GetOpenOpcUaScript();
		// 
		int iTopS = lua_gettop((lua_State *)pVm->GetLuaState());

		if (pMs->ScriptHasFunction(pszLuaMethodName))
		{
			if (pMs->SelectScriptFunction(pszLuaMethodName))
			{
				if (!pMs->Go(1))
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Cannot execute the script %s\n", pszLuaMethodName);
			}
		}
		free(pszLuaMethodName);
	}
	else
		uStatus = OpcUa_BadOutOfMemory;
	return uStatus;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Is in node hierarchy. </summary>
///
/// <remarks>	Michel, 01/09/2016. </remarks>
///
/// <param name="NodeIdSource">	The node identifier source. </param>
/// <param name="NodeIdTarget">	The node identifier target. </param>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode OpenOpcUa::UAAddressSpace::CUAInformationModel::IsInUABaseHierarchy(OpcUa_NodeId NodeIdSource, OpcUa_NodeId NodeIdTarget)
{
	OpcUa_StatusCode uStatus = OpcUa_BadInvalidArgument;
	vector<OpcUa_NodeId> referenceHierachy; // Liste des references appartenant a cette hirarchie (NodeIdSource et ses enfants)
	// La reference de base est toujours dans la hierarchie
	referenceHierachy.push_back(NodeIdSource);
	// Si la idReferenceSource = 0 On renvoi toujours ok. cad que l'on considère idRefrenceTarget dans la hierarchie
	if (NodeIdSource.Identifier.Numeric == 0)
		uStatus = OpcUa_Good;
	else
	{
		// remplissage de la hierarchy des references 
		OpcUa_UInt32 depth=0;
		uStatus = FillUABaseNodeIdHierarchy(NodeIdSource, &referenceHierachy,&depth);
		if (uStatus == OpcUa_Good)
		{
			uStatus = OpcUa_BadInvalidArgument;
			for (OpcUa_UInt32 ii = 0; ii < referenceHierachy.size(); ii++)
			{
				OpcUa_NodeId aNodeId = referenceHierachy.at(ii);
				if (Utils::IsEqual(&aNodeId, &NodeIdTarget))
				{
					uStatus = OpcUa_Good;
					break;
				}
			}
		}
	}
	return uStatus;	
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Fill UA base node ia hierarchy. </summary>
///
/// <remarks>	Michel, 01/09/2016. </remarks>
///
/// <param name="aNodeId">  	Identifier for the node. </param>
/// <param name="pHierachy">	[in,out] If non-null, the hierachy. </param>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode OpenOpcUa::UAAddressSpace::CUAInformationModel::FillUABaseNodeIdHierarchy(OpcUa_NodeId aNodeId, vector<OpcUa_NodeId>*pHierachy, OpcUa_UInt32* pDepth)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	CUABase* pRelativeRoot = NULL;

	if (pHierachy)
	{
		uStatus = GetNodeIdFromDictionnary(aNodeId, &pRelativeRoot);
		if (uStatus == OpcUa_Good)
		{
			(*pDepth)++;
			if ((*pDepth) < 50)
			{
				CUAReferenceList::iterator myIterator;
				for (myIterator = pRelativeRoot->GetReferenceNodeList()->begin(); myIterator != pRelativeRoot->GetReferenceNodeList()->end(); myIterator++)
				{
					CUAReference* pReferenceNode = *myIterator;
					OpcUa_ExpandedNodeId aExpandedNodeId = pReferenceNode->GetTargetId();
					OpcUa_Boolean bFound = false;
					for (OpcUa_UInt32 ii = 0; ii < pHierachy->size(); ii++)
					{
						OpcUa_NodeId pNodeId = pHierachy->at(ii);
						if (Utils::IsEqual(&pNodeId, &(aExpandedNodeId.NodeId)))
						{
							bFound = true;
							break;
						}
					}
					if ((!bFound) && (!(pReferenceNode->IsInverse())))
					{
						pHierachy->push_back(pReferenceNode->GetTargetId().NodeId);
					}
					if (!(pReferenceNode->IsInverse()))
					{
						// workaround for the Odd case where idReference== TargetId
						OpcUa_NodeId aTargetId = pReferenceNode->GetTargetId().NodeId;
						if (OpcUa_NodeId_Compare(&aTargetId, &aNodeId) == 0)
						{
							char* szNodeId = OpcUa_Null;
							Utils::NodeId2String(&aTargetId, &szNodeId);
							if (szNodeId)
							{
								// Attribute
								OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,
									"FillUABaseNodeIaHierarchy>An invalid reference definition was detected. Source and target[%s] node are equal\n", szNodeId);
								free(szNodeId);
							}
						}
						else
							FillUABaseNodeIdHierarchy(aTargetId, pHierachy, pDepth);
					}
				}
			}
		}
	}

	return uStatus;		
}

///-------------------------------------------------------------------------------------------------
/// <summary>
/// 	Search for the definition of limits for the AlarmDefinition available in aNodeId.
/// </summary>
///
/// <remarks>	Michel, 07/09/2016. </remarks>
///
/// <param name="pUAAlarmDefiniton">	[in,out] Identifier for the node. </param>
/// <param name="pLimits">				[in,out] If non-null, the limits. </param>
///
/// <returns>	The found limits. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode OpenOpcUa::UAAddressSpace::CUAInformationModel::SearchForLimits(CUAObject* pUAAlarmDefiniton, UALimitsAlarm* pLimits)
{
	OpcUa_StatusCode uStatus = OpcUa_BadNotFound;
	// Low
	OpcUa_StatusCode uStatusLow = OpcUa_BadNotFound;
	OpcUa_String szLowLimit;
	OpcUa_String_Initialize(&szLowLimit);
	OpcUa_String_AttachCopy(&szLowLimit, "LowLimit");
	// LowLow
	OpcUa_StatusCode uStatusLowLow = OpcUa_BadNotFound;
	OpcUa_String szLowLowLimit;
	OpcUa_String_Initialize(&szLowLowLimit);
	OpcUa_String_AttachCopy(&szLowLowLimit, "LowLowLimit");
	// High
	OpcUa_StatusCode uStatusHigh = OpcUa_BadNotFound;
	OpcUa_String szHighLimit;
	OpcUa_String_Initialize(&szHighLimit);
	OpcUa_String_AttachCopy(&szHighLimit, "HighLimit");
	// HighHigh
	OpcUa_StatusCode uStatusHighHigh = OpcUa_BadNotFound;
	OpcUa_String szHighHighLimit;
	OpcUa_String_Initialize(&szHighHighLimit);
	OpcUa_String_AttachCopy(&szHighHighLimit, "HighHighLimit");
	if (pUAAlarmDefiniton)
	{
		CUAReferenceList* pRefList = pUAAlarmDefiniton->GetReferenceNodeList();
		OpcUa_Int32 iNoOfRef = pUAAlarmDefiniton->GetNoOfReferences();
		for (OpcUa_Int32 ii = 0; ii < iNoOfRef; ii++)
		{
			CUAReference* pReference = pRefList->at(ii);
			if (pReference)
			{
				// Prepare the TargetId
				OpcUa_ExpandedNodeId aReferenceTargetId = pReference->GetTargetId();
				// Compare the BrowseName
				CUAVariable* pUAVariable = OpcUa_Null;
				if (GetNodeIdFromVariableList(aReferenceTargetId.NodeId, &pUAVariable) == OpcUa_Good)
				{
					OpcUa_QualifiedName* pBrowseName = pUAVariable->GetBrowseName();
					if (OpcUa_String_Compare(&(pBrowseName->Name), &szLowLimit) == 0)
					{
						pLimits->pUAVariableLowLimit = pUAVariable;
						uStatusLow = OpcUa_Good;
					}
					if (OpcUa_String_Compare(&(pBrowseName->Name), &szLowLowLimit) == 0)
					{
						pLimits->pUAVariableLowLowLimit = pUAVariable;
						uStatusLowLow = OpcUa_Good;
					}
					if (OpcUa_String_Compare(&(pBrowseName->Name), &szHighLimit) == 0)
					{
						pLimits->pUAVariableHighLimit = pUAVariable;
						uStatusHigh = OpcUa_Good;
					}
					if (OpcUa_String_Compare(&(pBrowseName->Name), &szHighHighLimit) == 0)
					{
						pLimits->pUAVariableHighHighLimit = pUAVariable;
						uStatusHighHigh = OpcUa_Good;
					}
				}
			}
		}
	}
	if ((uStatusLow == OpcUa_Good) 
		&& (uStatusLowLow == OpcUa_Good) 
		&& (uStatusHigh == OpcUa_Good) 
		&& (uStatusHighHigh == OpcUa_Good))
		uStatus = OpcUa_Good;
	OpcUa_String_Clear(&szLowLimit);
	OpcUa_String_Clear(&szLowLowLimit);
	OpcUa_String_Clear(&szHighLimit);
	OpcUa_String_Clear(&szHighHighLimit);
	return uStatus;	
}

///-------------------------------------------------------------------------------------------------
/// <summary>	This method will instanciate a new pCondition (CUAObject*) according to its 
/// 			ConditionType (CUAObjectType) and the pCondition passed in parameter CUAObject.  
/// 			Note that the CUAObjectType is extract from the CUAObject pCondition</summary>
///
/// <remarks>	Michel, 23/09/2016. </remarks>
///
/// <param name="pConditionType">	[in,out] If non-null, type of the condition. </param>
/// <param name="pCondition">	 	[in,out] If non-null, the condition. </param>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode OpenOpcUa::UAAddressSpace::CUAInformationModel::InstanciateConditionType(CUAVariable* pSourceNode,CUAObject* pCondition, CUAObject** ppNewCondition)
{
	OpcUa_StatusCode uStatus = OpcUa_BadNotFound;
	//
	OpcUa_NodeId DiscreteAlarmType;
	OpcUa_NodeId_Initialize(&DiscreteAlarmType);
	DiscreteAlarmType.Identifier.Numeric = 10523; // DiscreteAlarmType
	OpcUa_NodeId OffNormalAlarmType;
	OpcUa_NodeId_Initialize(&OffNormalAlarmType);
	OffNormalAlarmType.Identifier.Numeric = 10637; // OffNormalAlarmType
	OpcUa_NodeId LimitAlarmType;
	OpcUa_NodeId_Initialize(&LimitAlarmType);
	LimitAlarmType.Identifier.Numeric = 2955; // LimitAlarmType
	OpcUa_NodeId hasTypeDefinition;
	OpcUa_NodeId_Initialize(&hasTypeDefinition);
	hasTypeDefinition.Identifier.Numeric = 40;// HasTypeDefinition
	OpcUa_NodeId HasCondition;
	OpcUa_NodeId_Initialize(&HasCondition);
	HasCondition.Identifier.Numeric = 9006;// HasCondition
	//
	if (pSourceNode)
	{
		if (pCondition)
		{
			// Get ConditionType (CUAObjectType) for the Input parameter CUAObject
			// EventType from pCondition
			OpcUa_NodeId conditionTypeNodeId = pCondition->GetTypeDefinition();
			CUAObjectType* pConditionType = OpcUa_Null;
			if (GetNodeIdFromObjectTypeList(conditionTypeNodeId, &pConditionType) == OpcUa_Good)
			{
				if (pConditionType->GetNodeClass() == OpcUa_NodeClass_ObjectType)
				{
					CUAReferenceList* pReferences = pConditionType->GetReferenceNodeList();
					OpcUa_NodeId* pEventTypeNodeId = pConditionType->GetNodeId();
					if (OpcUa_NodeId_Compare(&LimitAlarmType, pEventTypeNodeId) == 0)
					{
						// Create the new instance
						(*ppNewCondition) = new CUAObject();
						// Create a new reference for HasTypeDefintiion --> LimitAlarmType
						CUAReference* pNewReference = new CUAReference();
						CUAReferenceList* pNewReferences = (*ppNewCondition)->GetReferenceNodeList();
						pNewReference->SetReferenceTypeId(&hasTypeDefinition);
						pNewReference->SetInverse(OpcUa_False);
						pNewReference->SetTargetId(&LimitAlarmType);
						// Save It
						pNewReferences->push_back(pNewReference);
						// Create the commom EventAttribute (references)
						if (CreateBaseEventTypeAttributeAndReferences(pSourceNode, pCondition, ppNewCondition) == OpcUa_Good)
						{
							(*ppNewCondition)->SetTypeDefinition(&LimitAlarmType);
							uStatus = CreateLimitAlarmConditionTypeReferences(ppNewCondition);
							uStatus = OpcUa_Good;
						}
					}
					else
					{
						if (OpcUa_NodeId_Compare(&OffNormalAlarmType, pEventTypeNodeId) == 0)
						{
							// Create the new instance
							(*ppNewCondition) = new CUAObject();
							// Create a new reference for HasTypeDefintiion --> OffNormalAlarmType
							CUAReference* pNewReference = new CUAReference();
							CUAReferenceList* pNewReferences = (*ppNewCondition)->GetReferenceNodeList();
							pNewReference->SetReferenceTypeId(&hasTypeDefinition);
							pNewReference->SetInverse(OpcUa_False);
							pNewReference->SetTargetId(&OffNormalAlarmType);
							pNewReferences->push_back(pNewReference);
							if (CreateBaseEventTypeAttributeAndReferences(pSourceNode, pCondition, ppNewCondition) == OpcUa_Good)
							{
								(*ppNewCondition)->SetTypeDefinition(&OffNormalAlarmType);
								uStatus = CreateOffNormalAlarmTypeReferences(ppNewCondition);
								uStatus = OpcUa_Good;
							}

						}
						else
						{
							if (OpcUa_NodeId_Compare(&DiscreteAlarmType, pEventTypeNodeId) == 0)
							{
								// Create the new instance
								(*ppNewCondition) = new CUAObject();
								// Create a new reference for HasTypeDefintiion --> DiscreteAlarmType
								CUAReference* pNewReference = new CUAReference();
								CUAReferenceList* pNewReferences = (*ppNewCondition)->GetReferenceNodeList();
								pNewReference->SetReferenceTypeId(&hasTypeDefinition);
								pNewReference->SetInverse(OpcUa_False);
								pNewReference->SetTargetId(&DiscreteAlarmType);
								pNewReferences->push_back(pNewReference);
								if (CreateBaseEventTypeAttributeAndReferences(pSourceNode, pCondition, ppNewCondition) == OpcUa_Good)
								{
									(*ppNewCondition)->SetTypeDefinition(&DiscreteAlarmType);
									uStatus = CreateDiscreteAlarmTypeReferences(ppNewCondition);
									uStatus = OpcUa_Good;
								}

							}
							else
								uStatus = OpcUa_BadNotSupported;
						}
					}
				}
				else
					uStatus = OpcUa_BadInvalidArgument;
			}
			else
				uStatus = OpcUa_BadInvalidArgument;
		}
	}
	else
		uStatus = OpcUa_BadInvalidArgument;

	OpcUa_NodeId_Clear(&hasTypeDefinition);
	OpcUa_NodeId_Clear(&OffNormalAlarmType);
	OpcUa_NodeId_Clear(&LimitAlarmType);
	return uStatus;		
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Creates default attribute for the new condition definition and basic references. 
/// 			</summary>
///
/// <remarks>	Michel, 23/09/2016. </remarks>
///
/// <param name="pCondition">	[in,out] If non-null, the condition. </param>
///
/// <returns>	The new defaul eventt attribute. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode OpenOpcUa::UAAddressSpace::CUAInformationModel::CreateBaseEventTypeAttributeAndReferences(CUAVariable* pSourceNode, CUAObject* pCondition, CUAObject** ppNewCondition)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_NodeId HasCondition;
	OpcUa_NodeId_Initialize(&HasCondition);
	HasCondition.Identifier.Numeric = 9006;// HasCondition
	OpcUa_NodeId HasComponent;
	OpcUa_NodeId_Initialize(&HasComponent);
	HasComponent.Identifier.Numeric = 47;// HasComponent
	OpcUa_NodeId PropertyType;
	OpcUa_NodeId_Initialize(&PropertyType);
	PropertyType.Identifier.Numeric = 68;// PropertyType
	// Search for an available nodeId
	
	OpcUa_NodeId TwoStateVariableType;
	OpcUa_NodeId_Initialize(&TwoStateVariableType);
	TwoStateVariableType.Identifier.Numeric = 8995;// "i=8995" BrowseName="TwoStateVariableType
	// Create a new nodeId
	OpcUa_NodeId NewCondition;
	OpcUa_NodeId_Initialize(&NewCondition);
	OpcUa_UInt16 namespaceIndex = pSourceNode->GetNodeId()->NamespaceIndex;
	OpcUa_UInt16 IdentifierType = OpcUa_IdentifierType_Numeric;
	uStatus = CreateNodeId(IdentifierType, namespaceIndex,1, &NewCondition);
	if (uStatus == OpcUa_Good)
	{
		// Set the NodeId of the new Node
		(*ppNewCondition)->SetNodeId(NewCondition);
		// NodeClass
		(*ppNewCondition)->SetNodeClass(OpcUa_NodeClass_Object);
		// BrowseName
		(*ppNewCondition)->SetBrowseName(pSourceNode->GetBrowseName());
		// DisplayName
		OpcUa_LocalizedText localizedText = pSourceNode->GetDisplayName();
		(*ppNewCondition)->SetDisplayName(&localizedText);
		// Description
		localizedText = pSourceNode->GetDescription();
		(*ppNewCondition)->SetDescription(localizedText);
		// WriteMask
		(*ppNewCondition)->SetWriteMask(OpcUa_AccessLevels_CurrentRead);
		// UserWriteMask
		(*ppNewCondition)->SetUserWriteMask(OpcUa_AccessLevels_CurrentRead);
		CUAReferenceList* pNewReferences = (*ppNewCondition)->GetReferenceNodeList();
		// Remove the references coming from the nodeSet Description
		CUAReferenceList* pSourceNodeReferences =pSourceNode->GetReferenceNodeList();
		// between pSourceNode and pCondition
		OpcUa_NodeId* pConditionNodeId=pCondition->GetNodeId();
		CUAReferenceList::iterator it;
		for (it = pSourceNodeReferences->begin(); it != pSourceNodeReferences->end(); it++)
		{
			CUAReference* pUAReference=*it;
			OpcUa_ExpandedNodeId aExpandedNodeId= pUAReference->GetTargetId();
			if (OpcUa_NodeId_Compare(pConditionNodeId, &aExpandedNodeId.NodeId) == 0)
			{
				delete pUAReference;
				pSourceNodeReferences->erase(it);
				pUAReference = OpcUa_Null;
				break;
			}
		}
		// Make it visible in the addressSpace
		// The new condition should be a child of the SourceNode
		CUAReference* pNewReference = new CUAReference();
		pNewReference->SetReferenceTypeId(&HasCondition);
		pNewReference->SetInverse(OpcUa_False);//pSourceNode->GetNodeId()
		pNewReference->SetTargetId(&NewCondition);
		pSourceNodeReferences->push_back(pNewReference);
		// 
		pNewReference = new CUAReference();
		pNewReference->SetReferenceTypeId(&HasComponent);
		pNewReference->SetInverse(OpcUa_False);//pSourceNode->GetNodeId()
		pNewReference->SetTargetId(&NewCondition);
		pSourceNodeReferences->push_back(pNewReference);
		
		//
		// Message  from pCondition
		CUAVariable* pMessage = OpcUa_Null;
		uStatus = SearchForEventMessage(pCondition, &pMessage);
		if (uStatus == OpcUa_Good)
		{
			// Create the reference for the message on the same target as define in Condition
			pNewReference = new CUAReference();
			pNewReference->SetReferenceTypeId(&HasComponent);
			pNewReference->SetInverse(OpcUa_False);
			pNewReference->SetTargetId(pMessage->GetNodeId());
			pNewReferences->push_back(pNewReference);
			PopulateInformationModel((*ppNewCondition));
			// Severity  from pCondition
			CUAVariable* pSeverity=OpcUa_Null;
			uStatus = SearchForEventSeverity(pCondition, &pSeverity);
			if (uStatus == OpcUa_Good)
			{
				pNewReference = new CUAReference();
				pNewReference->SetReferenceTypeId(&HasComponent);
				pNewReference->SetInverse(OpcUa_False);
				pNewReference->SetTargetId(pSeverity->GetNodeId());
				pNewReferences->push_back(pNewReference);
				/*
				// Now we will create ActiveState -  -
				OpcUa_NodeId aNewNodeId;
				CreateNodeId(OpcUa_IdentifierType_Numeric, namespaceIndex, NewCondition.Identifier.Numeric+1, &aNewNodeId);
				OpcUa_Variant varVal;
				OpcUa_Variant_Initialize(&varVal);
				varVal.Datatype = OpcUaType_LocalizedText;
				OpcUa_String szBrowseName;
				OpcUa_String_AttachCopy(&szBrowseName, "ActiveState");
				uStatus=AddUAVariable(aNewNodeId, szBrowseName, TwoStateVariableType, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
				if (uStatus == OpcUa_Good)
				{
				uStatus = AddUAReference(NewCondition, aNewNodeId, HasComponent);
				if (uStatus == OpcUa_Good)
				{
				// Add ActiveStateId
				OpcUa_NodeId ActiveStateId;
				CreateNodeId(OpcUa_IdentifierType_Numeric, namespaceIndex, NewCondition.Identifier.Numeric+1, &ActiveStateId);
				OpcUa_Variant_Initialize(&varVal);
				varVal.Datatype = OpcUaType_Boolean;
				OpcUa_String szBrowseName;
				OpcUa_String_AttachCopy(&szBrowseName, "Id");
				uStatus = AddUAVariable(ActiveStateId, szBrowseName, PropertyType, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
				if (uStatus == OpcUa_Good)
				{
				uStatus = AddUAReference(aNewNodeId, ActiveStateId, HasComponent);
				}
				}
				else
				{
				// Report Error

				}
				}
				else
				{
				// Report Error
				}
				*/
			}
			//
		}
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	OpcUa_NodeId_Initialize(&HasCondition);
	return uStatus;			
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Searches for the first event message. </summary>
///
/// <remarks>	Michel, 23/09/2016. </remarks>
///
/// <param name="pCondition">  	[in,out] If non-null, the condition. </param>
/// <param name="ppUAVariable">	[in,out] If non-null, the UA variable. </param>
///
/// <returns>	The found event message. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode OpenOpcUa::UAAddressSpace::CUAInformationModel::SearchForEventMessage(CUAObject* pCondition, CUAVariable** ppUAVariable)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_QualifiedName szMessage;
	OpcUa_QualifiedName_Initialize(&szMessage);
	OpcUa_String_AttachCopy(&(szMessage.Name), "Message");
	uStatus = SearchForState(pCondition, szMessage, ppUAVariable);
	OpcUa_QualifiedName_Clear(&szMessage);
	return uStatus;	
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Searches for the first event severity. </summary>
///
/// <remarks>	Michel, 24/09/2016. </remarks>
///
/// <param name="pCondition">  	[in,out] If non-null, the condition. </param>
/// <param name="ppUAVariable">	[in,out] If non-null, the UA variable. </param>
///
/// <returns>	The found event severity. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode OpenOpcUa::UAAddressSpace::CUAInformationModel::SearchForEventSeverity(CUAObject*pCondition, CUAVariable** ppUAVariable)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_QualifiedName szMessage;
	OpcUa_QualifiedName_Initialize(&szMessage);
	OpcUa_String_AttachCopy(&(szMessage.Name), "Severity");
	uStatus = SearchForState(pCondition, szMessage, ppUAVariable);
	OpcUa_QualifiedName_Clear(&szMessage);
	return uStatus;		
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Creates condition type references. </summary>
///
/// <remarks>	Michel, 26/09/2016. </remarks>
///
/// <param name="ppNewCondition">	[in,out] If non-null, the new condition. </param>
///
/// <returns>	The new condition type references. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode OpenOpcUa::UAAddressSpace::CUAInformationModel::CreateConditionTypeReferences(CUAObject** ppNewCondition)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_NodeId HasComponent;
	OpcUa_NodeId_Initialize(&HasComponent);
	HasComponent.Identifier.Numeric = 47;// HasComponent

	OpcUa_NodeId PropertyType;
	OpcUa_NodeId_Initialize(&PropertyType);
	PropertyType.Identifier.Numeric = 68;// PropertyType

	OpcUa_NodeId HasProperty;
	OpcUa_NodeId_Initialize(&HasProperty);
	HasProperty.Identifier.Numeric = 46; // HasProperty

	OpcUa_NodeId HasTypeDefinition;
	OpcUa_NodeId_Initialize(&HasTypeDefinition);
	HasTypeDefinition.Identifier.Numeric = 40; // HasTypeDefinition

	OpcUa_NodeId TwoStateVariableType;
	OpcUa_NodeId_Initialize(&TwoStateVariableType);
	TwoStateVariableType.Identifier.Numeric = 8995;// "i=8995" BrowseName=TwoStateVariableType

	OpcUa_NodeId ConditionVariableType;
	OpcUa_NodeId_Initialize(&ConditionVariableType);
	ConditionVariableType.Identifier.Numeric = 9002;// "i=9002" BrowseName=ConditionVariableType

	OpcUa_NodeId UtcTime;
	OpcUa_NodeId_Initialize(&UtcTime);
	UtcTime.Identifier.Numeric = 294;// "i=UtcTime" BrowseName=UtcTime

	// Now we will create ConditionName -  -
	OpcUa_NodeId* pConditionNodeId=(*ppNewCondition)->GetNodeId();
	if (pConditionNodeId)
	{
		// ConditionName
		OpcUa_UInt16 namespaceIndex = pConditionNodeId->NamespaceIndex;
		OpcUa_NodeId aConditionNameNodeId;
		CreateNodeId(OpcUa_IdentifierType_Numeric, namespaceIndex, pConditionNodeId->Identifier.Numeric+10, &aConditionNameNodeId);
		OpcUa_Variant varVal;
		OpcUa_Variant_Initialize(&varVal);
		varVal.Datatype = OpcUaType_String;
		OpcUa_String szBrowseName;
		OpcUa_String_Initialize(&szBrowseName);
		OpcUa_String_AttachCopy(&szBrowseName, "ConditionName");
		uStatus = AddUAVariable(aConditionNameNodeId, szBrowseName, PropertyType, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
		if (uStatus == OpcUa_Good)
		{
			// Make it visible behind the Condition definition
			uStatus = AddUAReference(*pConditionNodeId, aConditionNameNodeId, HasProperty);
			// Add TypeDefintion
			uStatus = AddUAReference(aConditionNameNodeId, PropertyType, HasTypeDefinition);

			if (uStatus == OpcUa_Good)
			{
				// ConditionClassName
				OpcUa_NodeId aConditionClassNameNodeId;
				CreateNodeId(OpcUa_IdentifierType_Numeric, namespaceIndex, pConditionNodeId->Identifier.Numeric + 11, &aConditionClassNameNodeId);
				OpcUa_Variant varVal;
				OpcUa_Variant_Initialize(&varVal);
				varVal.Datatype = OpcUaType_LocalizedText;
				OpcUa_String_Clear(&szBrowseName);
				OpcUa_String_Initialize(&szBrowseName);

				OpcUa_String_AttachCopy(&szBrowseName, "ConditionClassName");
				uStatus = AddUAVariable(aConditionClassNameNodeId, szBrowseName, PropertyType, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
				if (uStatus == OpcUa_Good)
				{
					// Make it visible behind the Condition definition
					uStatus = AddUAReference(*pConditionNodeId, aConditionClassNameNodeId, HasProperty);
					// Add TypeDefintion
					uStatus = AddUAReference(aConditionClassNameNodeId, PropertyType, HasTypeDefinition);
					if (uStatus == OpcUa_Good)
					{
						// ConditionClassId
						OpcUa_NodeId aConditionClassIdNodeId;
						CreateNodeId(OpcUa_IdentifierType_Numeric, namespaceIndex, pConditionNodeId->Identifier.Numeric + 12, &aConditionClassIdNodeId);
						OpcUa_Variant varVal;
						OpcUa_Variant_Initialize(&varVal);
						varVal.Datatype = OpcUaType_NodeId;
						OpcUa_String_Clear(&szBrowseName);
						OpcUa_String_Initialize(&szBrowseName);

						OpcUa_String_AttachCopy(&szBrowseName, "ConditionClassId");
						uStatus = AddUAVariable(aConditionClassIdNodeId, szBrowseName, PropertyType, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
						if (uStatus == OpcUa_Good)
						{
							// Make it visible behind the Condition definition
							uStatus = AddUAReference(*pConditionNodeId, aConditionClassIdNodeId, HasProperty);
							// Add TypeDefintion
							uStatus = AddUAReference(aConditionClassIdNodeId, PropertyType, HasTypeDefinition);
							// BranchId
							OpcUa_NodeId aBranchIdNodeId;
							CreateNodeId(OpcUa_IdentifierType_Numeric, namespaceIndex, pConditionNodeId->Identifier.Numeric + 13, &aBranchIdNodeId);
							OpcUa_Variant varVal;
							OpcUa_Variant_Initialize(&varVal);
							varVal.Datatype = OpcUaType_NodeId;
							OpcUa_String_Clear(&szBrowseName);
							OpcUa_String_Initialize(&szBrowseName);

							OpcUa_String_AttachCopy(&szBrowseName, "BranchId");
							uStatus = AddUAVariable(aBranchIdNodeId, szBrowseName, PropertyType, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
							if (uStatus == OpcUa_Good)
							{
								// Make it visible behind the Condition definition
								uStatus = AddUAReference(*pConditionNodeId, aBranchIdNodeId, HasProperty);
								// Add TypeDefintion
								uStatus = AddUAReference(aBranchIdNodeId, PropertyType, HasTypeDefinition);
								// Retain
								OpcUa_NodeId aRetainNodeId;
								CreateNodeId(OpcUa_IdentifierType_Numeric, namespaceIndex, pConditionNodeId->Identifier.Numeric + 14, &aRetainNodeId);
								OpcUa_Variant varVal;
								OpcUa_Variant_Initialize(&varVal);
								varVal.Datatype = OpcUaType_Boolean;
								OpcUa_String_Clear(&szBrowseName);
								OpcUa_String_Initialize(&szBrowseName);

								OpcUa_String_AttachCopy(&szBrowseName, "Retain");
								uStatus = AddUAVariable(aRetainNodeId, szBrowseName, PropertyType, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
								if (uStatus == OpcUa_Good)
								{
									// Make it visible behind the Condition definition
									uStatus = AddUAReference(*pConditionNodeId, aRetainNodeId, HasProperty);
									// Add TypeDefintion
									uStatus = AddUAReference(aRetainNodeId, PropertyType, HasTypeDefinition);

									// EnabledState
									OpcUa_NodeId aEnabledStateNodeId;
									CreateNodeId(OpcUa_IdentifierType_Numeric, namespaceIndex, pConditionNodeId->Identifier.Numeric + 15, &aEnabledStateNodeId);
									OpcUa_Variant varVal;
									OpcUa_Variant_Initialize(&varVal);
									varVal.Datatype = OpcUaType_LocalizedText;
									OpcUa_String_Clear(&szBrowseName);
									OpcUa_String_Initialize(&szBrowseName);
									OpcUa_String_AttachCopy(&szBrowseName, "EnabledState");
									uStatus = AddUAVariable(aEnabledStateNodeId, szBrowseName, TwoStateVariableType, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
									if (uStatus == OpcUa_Good)
									{
										// Make it visible behind the Condition definition
										uStatus = AddUAReference(*pConditionNodeId, aEnabledStateNodeId, HasProperty);
										// Add TypeDefintion
										uStatus = AddUAReference(aEnabledStateNodeId, TwoStateVariableType, HasTypeDefinition);
										// Add Id child of aEnabledStateNodeId
										// EnabledState - Id
										OpcUa_NodeId aEnabledStateIdNodeId;
										CreateNodeId(OpcUa_IdentifierType_Numeric, namespaceIndex, pConditionNodeId->Identifier.Numeric + 16, &aEnabledStateIdNodeId);
										OpcUa_Variant varVal;
										OpcUa_Variant_Initialize(&varVal);
										varVal.Datatype = OpcUaType_Boolean;
										OpcUa_String_Clear(&szBrowseName);
										OpcUa_String_Initialize(&szBrowseName);

										OpcUa_String_AttachCopy(&szBrowseName, "Id");
										uStatus = AddUAVariable(aEnabledStateIdNodeId, szBrowseName, PropertyType, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
										if (uStatus == OpcUa_Good)
										{
											// Make it visible behind the Condition definition
											uStatus = AddUAReference(aEnabledStateNodeId, aEnabledStateIdNodeId, HasProperty);
											// Add TypeDefintion
											uStatus = AddUAReference(aEnabledStateIdNodeId, PropertyType, HasTypeDefinition);
											if (uStatus == OpcUa_Good)
											{
												// Inherit from StateVariableType
												// Name (Optionnal from StateVariableType)
												// Number (Optionnal from StateVariableType)
												// 
												// EffectiveDisplayName(Optionnal from StateVariableType)
												// TransitionTime (Optionnal from TwoStateVariableType)
												// EffectiveTransitionTime (Optionnal from TwoStateVariableType)
												// TrueState (Optionnal from TwoStateVariableType)
												// FalseState (Optionnal from TwoStateVariableType)
											}
										}
									}
									//
									// Quality
									OpcUa_NodeId aQualityNodeId;
									CreateNodeId(OpcUa_IdentifierType_Numeric, namespaceIndex, pConditionNodeId->Identifier.Numeric + 17, &aQualityNodeId);
									OpcUa_Variant_Clear(&varVal);
									OpcUa_Variant_Initialize(&varVal);
									varVal.Datatype = OpcUaType_StatusCode;

									OpcUa_String_Clear(&szBrowseName);
									OpcUa_String_Initialize(&szBrowseName);
									OpcUa_String_AttachCopy(&szBrowseName, "Quality");
									uStatus = AddUAVariable(aQualityNodeId, szBrowseName, ConditionVariableType, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
									if (uStatus == OpcUa_Good)
									{
										// Make it visible behind the Condition definition
										uStatus = AddUAReference(*pConditionNodeId, aQualityNodeId, HasProperty);
										// Add TypeDefintion
										uStatus = AddUAReference(aQualityNodeId, ConditionVariableType, HasTypeDefinition);
										// Add SourceTimestamp child of Quality
										// Quality - SourceTimestamp
										OpcUa_NodeId aSourceTimestampNodeId;
										CreateNodeId(OpcUa_IdentifierType_Numeric, namespaceIndex, pConditionNodeId->Identifier.Numeric + 18, &aSourceTimestampNodeId);
										OpcUa_Variant varVal;
										OpcUa_Variant_Initialize(&varVal);
										varVal.Datatype = OpcUaType_DateTime;
										OpcUa_String_Clear(&szBrowseName);
										OpcUa_String_Initialize(&szBrowseName);

										OpcUa_String_AttachCopy(&szBrowseName, "SourceTimestamp");
										uStatus = AddUAVariable(aSourceTimestampNodeId, szBrowseName, UtcTime, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
										if (uStatus == OpcUa_Good)
										{
											// Make it visible behind the Condition definition
											uStatus = AddUAReference(aQualityNodeId, aSourceTimestampNodeId, HasProperty);
											// Add TypeDefintion
											uStatus = AddUAReference(aSourceTimestampNodeId, PropertyType, HasTypeDefinition);
										}
									}

									// LastSeverity
									OpcUa_NodeId aLastSeverityNodeId;
									CreateNodeId(OpcUa_IdentifierType_Numeric, namespaceIndex, pConditionNodeId->Identifier.Numeric + 19, &aLastSeverityNodeId);
									OpcUa_Variant_Clear(&varVal);
									OpcUa_Variant_Initialize(&varVal);
									varVal.Datatype = OpcUaType_UInt16;

									OpcUa_String_Clear(&szBrowseName);
									OpcUa_String_Initialize(&szBrowseName);
									OpcUa_String_AttachCopy(&szBrowseName, "LastSeverity");
									uStatus = AddUAVariable(aLastSeverityNodeId, szBrowseName, ConditionVariableType, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
									if (uStatus == OpcUa_Good)
									{
										// Make it visible behind the Condition definition
										uStatus = AddUAReference(*pConditionNodeId, aLastSeverityNodeId, HasProperty);
										// Add TypeDefintion
										uStatus = AddUAReference(aLastSeverityNodeId, ConditionVariableType, HasTypeDefinition);
										// Add SourceTimestamp child of Quality
										// Quality - SourceTimestamp
										OpcUa_NodeId aSourceTimestampNodeId;
										CreateNodeId(OpcUa_IdentifierType_Numeric, namespaceIndex, pConditionNodeId->Identifier.Numeric + 20, &aSourceTimestampNodeId);
										OpcUa_Variant varVal;
										OpcUa_Variant_Initialize(&varVal);
										varVal.Datatype = OpcUaType_DateTime;
										OpcUa_String szBrowseName;

										OpcUa_String_AttachCopy(&szBrowseName, "SourceTimestamp");
										uStatus = AddUAVariable(aSourceTimestampNodeId, szBrowseName, UtcTime, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
										if (uStatus == OpcUa_Good)
										{
											// Make it visible behind the Condition definition
											uStatus = AddUAReference(aLastSeverityNodeId, aSourceTimestampNodeId, HasProperty);
											// Add TypeDefintion
											uStatus = AddUAReference(aSourceTimestampNodeId, PropertyType, HasTypeDefinition);
										}
										OpcUa_Variant_Clear(&varVal);
										OpcUa_NodeId_Clear(&aSourceTimestampNodeId);
										OpcUa_String_Clear(&szBrowseName);
									}

									// Comment
									OpcUa_NodeId aCommentNodeId;
									CreateNodeId(OpcUa_IdentifierType_Numeric, namespaceIndex, pConditionNodeId->Identifier.Numeric + 21, &aCommentNodeId);
									OpcUa_Variant_Clear(&varVal);
									OpcUa_Variant_Initialize(&varVal);
									varVal.Datatype = OpcUaType_LocalizedText;

									OpcUa_String_Clear(&szBrowseName);
									OpcUa_String_Initialize(&szBrowseName);
									OpcUa_String_AttachCopy(&szBrowseName, "Comment");
									uStatus = AddUAVariable(aCommentNodeId, szBrowseName, ConditionVariableType, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
									if (uStatus == OpcUa_Good)
									{
										// Make it visible behind the Condition definition
										uStatus = AddUAReference(*pConditionNodeId, aCommentNodeId, HasProperty);
										// Add TypeDefintion
										uStatus = AddUAReference(aCommentNodeId, ConditionVariableType, HasTypeDefinition);
										// Add SourceTimestamp child of Quality
										// Quality - SourceTimestamp
										OpcUa_NodeId aSourceTimestampNodeId;
										CreateNodeId(OpcUa_IdentifierType_Numeric, namespaceIndex, pConditionNodeId->Identifier.Numeric + 22, &aSourceTimestampNodeId);
										OpcUa_Variant varVal;
										OpcUa_Variant_Initialize(&varVal);
										varVal.Datatype = OpcUaType_DateTime;
										OpcUa_String szBrowseName;

										OpcUa_String_AttachCopy(&szBrowseName, "SourceTimestamp");
										uStatus = AddUAVariable(aSourceTimestampNodeId, szBrowseName, UtcTime, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
										if (uStatus == OpcUa_Good)
										{
											// Make it visible behind the Condition definition
											uStatus = AddUAReference(aCommentNodeId, aSourceTimestampNodeId, HasProperty);
											// Add TypeDefintion
											uStatus = AddUAReference(aSourceTimestampNodeId, PropertyType, HasTypeDefinition);
										}
										OpcUa_Variant_Clear(&varVal);
										OpcUa_NodeId_Clear(&aSourceTimestampNodeId);
										OpcUa_String_Clear(&szBrowseName);
									}

									// ClientUserId
									OpcUa_NodeId aClientUserIdNodeId;
									CreateNodeId(OpcUa_IdentifierType_Numeric, namespaceIndex, pConditionNodeId->Identifier.Numeric + 23, &aClientUserIdNodeId);
									OpcUa_Variant_Clear(&varVal);
									OpcUa_Variant_Initialize(&varVal);
									varVal.Datatype = OpcUaType_String;

									OpcUa_String_Clear(&szBrowseName);
									OpcUa_String_Initialize(&szBrowseName);
									OpcUa_String_AttachCopy(&szBrowseName, "ClientUserId");
									uStatus = AddUAVariable(aClientUserIdNodeId, szBrowseName, PropertyType, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
									if (uStatus == OpcUa_Good)
									{
										// Make it visible behind the Condition definition
										uStatus = AddUAReference(*pConditionNodeId, aClientUserIdNodeId, HasProperty);
										// Add TypeDefintion
										uStatus = AddUAReference(aClientUserIdNodeId, PropertyType, HasTypeDefinition);
									}
								}
							}
						}
					}
				}
			}

		}
		OpcUa_String_Clear(&szBrowseName);
	}
	OpcUa_NodeId_Clear(&HasComponent);
	OpcUa_NodeId_Clear(&PropertyType);
	OpcUa_NodeId_Clear(&HasProperty);
	OpcUa_NodeId_Clear(&HasTypeDefinition);
	OpcUa_NodeId_Clear(&TwoStateVariableType);
	OpcUa_NodeId_Clear(&ConditionVariableType);
	OpcUa_NodeId_Clear(&UtcTime);
	return uStatus;		
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Creates acknowledgeable condition type references. </summary>
///
/// <remarks>	Michel, 26/09/2016. </remarks>
///
/// <param name="ppNewCondition">	[in,out] If non-null, the new condition. </param>
///
/// <returns>	The new acknowledgeable condition type references. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode OpenOpcUa::UAAddressSpace::CUAInformationModel::CreateAcknowledgeableConditionTypeReferences(CUAObject** ppNewCondition)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_NodeId HasComponent;
	OpcUa_NodeId_Initialize(&HasComponent);
	HasComponent.Identifier.Numeric = 47;// HasComponent

	OpcUa_NodeId PropertyType;
	OpcUa_NodeId_Initialize(&PropertyType);
	PropertyType.Identifier.Numeric = 68;// PropertyType

	OpcUa_NodeId HasProperty;
	OpcUa_NodeId_Initialize(&HasProperty);
	HasProperty.Identifier.Numeric = 46; // HasProperty

	OpcUa_NodeId HasTypeDefinition;
	OpcUa_NodeId_Initialize(&HasTypeDefinition);
	HasTypeDefinition.Identifier.Numeric = 40; // HasTypeDefinition

	OpcUa_NodeId TwoStateVariableType;
	OpcUa_NodeId_Initialize(&TwoStateVariableType);
	TwoStateVariableType.Identifier.Numeric = 8995;// "i=8995" BrowseName=TwoStateVariableType

	OpcUa_NodeId ConditionVariableType;
	OpcUa_NodeId_Initialize(&ConditionVariableType);
	ConditionVariableType.Identifier.Numeric = 9002;// "i=9002" BrowseName=ConditionVariableType

	OpcUa_NodeId UtcTime;
	OpcUa_NodeId_Initialize(&UtcTime);
	UtcTime.Identifier.Numeric = 294;// "i=UtcTime" BrowseName=UtcTime

	uStatus = CreateConditionTypeReferences(ppNewCondition);
	if (uStatus==OpcUa_Good)
	{

		// Now we will create ConditionName -  -
		OpcUa_NodeId* pConditionNodeId = (*ppNewCondition)->GetNodeId();
		if (pConditionNodeId)
		{
			// AckedState 
			OpcUa_UInt16 namespaceIndex = pConditionNodeId->NamespaceIndex;
			OpcUa_NodeId aAckedStateNodeId;
			CreateNodeId(OpcUa_IdentifierType_Numeric, namespaceIndex, pConditionNodeId->Identifier.Numeric + 10, &aAckedStateNodeId);
			OpcUa_Variant varVal;
			OpcUa_Variant_Initialize(&varVal);
			varVal.Datatype = OpcUaType_LocalizedText;
			OpcUa_String szBrowseName;
			OpcUa_String_Initialize(&szBrowseName);
			OpcUa_String_AttachCopy(&szBrowseName, "AckedState");
			uStatus = AddUAVariable(aAckedStateNodeId, szBrowseName, HasProperty, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
			if (uStatus == OpcUa_Good)
			{
				// Make it visible behind the Condition definition
				uStatus = AddUAReference(*pConditionNodeId, aAckedStateNodeId, HasProperty);
				// Add TypeDefintion
				uStatus = AddUAReference(aAckedStateNodeId, PropertyType, HasTypeDefinition);
				if (uStatus == OpcUa_Good)
				{
					OpcUa_UInt16 namespaceIndex = pConditionNodeId->NamespaceIndex;
					OpcUa_NodeId aAckedStateIdNodeId;
					CreateNodeId(OpcUa_IdentifierType_Numeric, namespaceIndex, pConditionNodeId->Identifier.Numeric + 10, &aAckedStateIdNodeId);
					OpcUa_Variant varVal;
					OpcUa_Variant_Initialize(&varVal);
					varVal.Datatype = OpcUaType_Boolean;
					OpcUa_String_Clear(&szBrowseName);
					OpcUa_String_Initialize(&szBrowseName);

					OpcUa_String_AttachCopy(&szBrowseName, "Id");
					uStatus = AddUAVariable(aAckedStateIdNodeId, szBrowseName, PropertyType, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
					if (uStatus == OpcUa_Good)
					{
						// Make it visible behind the Condition definition
						uStatus = AddUAReference(aAckedStateNodeId, aAckedStateIdNodeId, HasProperty);
						// Add TypeDefintion
						uStatus = AddUAReference(aAckedStateIdNodeId, PropertyType, HasTypeDefinition);
					}
				}
				if (uStatus == OpcUa_Good)
				{
					// ConfirmedState
					OpcUa_UInt16 namespaceIndex = pConditionNodeId->NamespaceIndex;
					OpcUa_NodeId aConfirmedStateNodeId;
					CreateNodeId(OpcUa_IdentifierType_Numeric, namespaceIndex, pConditionNodeId->Identifier.Numeric + 10, &aConfirmedStateNodeId);
					OpcUa_Variant varVal;
					OpcUa_Variant_Initialize(&varVal);
					varVal.Datatype = OpcUaType_LocalizedText;
					OpcUa_String_Clear(&szBrowseName);
					OpcUa_String_Initialize(&szBrowseName);

					OpcUa_String_AttachCopy(&szBrowseName, "ConfirmedState");
					uStatus = AddUAVariable(aConfirmedStateNodeId, szBrowseName, HasProperty, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
					if (uStatus == OpcUa_Good)
					{
						// Make it visible behind the Condition definition
						uStatus = AddUAReference(*pConditionNodeId, aConfirmedStateNodeId, HasProperty);
						// Add TypeDefintion
						uStatus = AddUAReference(aConfirmedStateNodeId, PropertyType, HasTypeDefinition);
						if (uStatus == OpcUa_Good)
						{
							OpcUa_UInt16 namespaceIndex = pConditionNodeId->NamespaceIndex;
							OpcUa_NodeId aConfirmedStateIdNodeId;
							CreateNodeId(OpcUa_IdentifierType_Numeric, namespaceIndex, pConditionNodeId->Identifier.Numeric + 10, &aConfirmedStateIdNodeId);
							OpcUa_Variant varVal;
							OpcUa_Variant_Initialize(&varVal);
							varVal.Datatype = OpcUaType_Boolean;
							OpcUa_String_Clear(&szBrowseName);
							OpcUa_String_Initialize(&szBrowseName);

							OpcUa_String_AttachCopy(&szBrowseName, "Id");
							uStatus = AddUAVariable(aConfirmedStateIdNodeId, szBrowseName, PropertyType, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
							if (uStatus == OpcUa_Good)
							{
								// Make it visible behind the Condition definition
								uStatus = AddUAReference(aConfirmedStateNodeId, aConfirmedStateIdNodeId, HasProperty);
								// Add TypeDefintion
								uStatus = AddUAReference(aConfirmedStateIdNodeId, PropertyType, HasTypeDefinition);
							}
						}
					}
				}
			}
			OpcUa_String_Clear(&szBrowseName);
		}

	}
	OpcUa_NodeId_Clear(&HasComponent);
	OpcUa_NodeId_Clear(&PropertyType);
	OpcUa_NodeId_Clear(&HasProperty);
	OpcUa_NodeId_Clear(&HasTypeDefinition);
	OpcUa_NodeId_Clear(&TwoStateVariableType);
	OpcUa_NodeId_Clear(&ConditionVariableType);
	OpcUa_NodeId_Clear(&UtcTime);
	return uStatus;		
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Creates alarm references. </summary>
///
/// <remarks>	Michel, 26/09/2016. </remarks>
///
/// <param name="ppNewCondition">	[in,out] If non-null, the new condition. </param>
///
/// <returns>	The new alarm references. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode OpenOpcUa::UAAddressSpace::CUAInformationModel::CreateAlarmConditionTypeReferences(CUAObject** ppNewCondition)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_NodeId HasComponent;
	OpcUa_NodeId_Initialize(&HasComponent);
	HasComponent.Identifier.Numeric = 47;// HasComponent

	OpcUa_NodeId PropertyType;
	OpcUa_NodeId_Initialize(&PropertyType);
	PropertyType.Identifier.Numeric = 68;// PropertyType

	OpcUa_NodeId HasProperty;
	OpcUa_NodeId_Initialize(&HasProperty);
	HasProperty.Identifier.Numeric = 46; // HasProperty

	OpcUa_NodeId HasTypeDefinition;
	OpcUa_NodeId_Initialize(&HasTypeDefinition);
	HasTypeDefinition.Identifier.Numeric = 40; // HasTypeDefinition

	OpcUa_NodeId TwoStateVariableType;
	OpcUa_NodeId_Initialize(&TwoStateVariableType);
	TwoStateVariableType.Identifier.Numeric = 8995;// "i=8995" BrowseName=TwoStateVariableType

	OpcUa_NodeId ConditionVariableType;
	OpcUa_NodeId_Initialize(&ConditionVariableType);
	ConditionVariableType.Identifier.Numeric = 9002;// "i=9002" BrowseName=ConditionVariableType

	OpcUa_NodeId ShelvedStateMachineType;
	OpcUa_NodeId_Initialize(&ShelvedStateMachineType);
	ShelvedStateMachineType.Identifier.Numeric = 2929;// "i=2929" BrowseName=ShelvedStateMachineType

	OpcUa_NodeId UtcTime;
	OpcUa_NodeId_Initialize(&UtcTime);
	UtcTime.Identifier.Numeric = 294;// "i=UtcTime" BrowseName=UtcTime
	uStatus = CreateAcknowledgeableConditionTypeReferences(ppNewCondition);
	if (uStatus == OpcUa_Good)
	{
		OpcUa_NodeId* pConditionNodeId = (*ppNewCondition)->GetNodeId();
		if (pConditionNodeId)
		{
			// ActiveState
			OpcUa_UInt16 namespaceIndex = pConditionNodeId->NamespaceIndex;
			OpcUa_NodeId aActiveStateNodeId;
			CreateNodeId(OpcUa_IdentifierType_Numeric, namespaceIndex, pConditionNodeId->Identifier.Numeric + 10, &aActiveStateNodeId);
			OpcUa_Variant varVal;
			OpcUa_Variant_Initialize(&varVal);
			varVal.Datatype = OpcUaType_LocalizedText;
			OpcUa_String szBrowseName;
			OpcUa_String_Initialize(&szBrowseName);
			OpcUa_String_AttachCopy(&szBrowseName, "ActiveState");
			uStatus = AddUAVariable(aActiveStateNodeId, szBrowseName, TwoStateVariableType, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
			if (uStatus == OpcUa_Good)
			{
				// Make it visible behind the Condition definition
				uStatus = AddUAReference(*pConditionNodeId, aActiveStateNodeId, HasProperty);
				// Add TypeDefintion
				uStatus = AddUAReference(aActiveStateNodeId, TwoStateVariableType, HasTypeDefinition);

				if (uStatus == OpcUa_Good)
				{
					// ActiveState - Id
					OpcUa_UInt16 namespaceIndex = pConditionNodeId->NamespaceIndex;
					OpcUa_NodeId aActiveStateIdNodeId;
					CreateNodeId(OpcUa_IdentifierType_Numeric, namespaceIndex, pConditionNodeId->Identifier.Numeric + 10, &aActiveStateIdNodeId);
					OpcUa_Variant varVal;
					OpcUa_Variant_Initialize(&varVal);
					varVal.Datatype = OpcUaType_Boolean;
					OpcUa_String_Clear(&szBrowseName);
					OpcUa_String_Initialize(&szBrowseName);

					OpcUa_String_AttachCopy(&szBrowseName, "Id");
					uStatus = AddUAVariable(aActiveStateIdNodeId, szBrowseName, PropertyType, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
					if (uStatus == OpcUa_Good)
					{
						// Make it visible behind the Condition definition
						uStatus = AddUAReference(aActiveStateNodeId, aActiveStateIdNodeId, HasProperty);
						// Add TypeDefintion
						uStatus = AddUAReference(aActiveStateIdNodeId, PropertyType, HasTypeDefinition);
						// ActiveState -  EffectiveDisplayName
						OpcUa_UInt16 namespaceIndex = pConditionNodeId->NamespaceIndex;
						OpcUa_NodeId aActiveStateEffectiveDisplayNameNodeId;
						CreateNodeId(OpcUa_IdentifierType_Numeric, namespaceIndex, pConditionNodeId->Identifier.Numeric + 10, &aActiveStateEffectiveDisplayNameNodeId);
						OpcUa_Variant varVal;
						OpcUa_Variant_Initialize(&varVal);
						varVal.Datatype = OpcUaType_LocalizedText;
						OpcUa_String_Clear(&szBrowseName);
						OpcUa_String_Initialize(&szBrowseName);

						OpcUa_String_AttachCopy(&szBrowseName, "EffectiveDisplayName");
						uStatus = AddUAVariable(aActiveStateEffectiveDisplayNameNodeId, szBrowseName, PropertyType, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
						if (uStatus == OpcUa_Good)
						{
							// Make it visible behind the Condition definition
							uStatus = AddUAReference(aActiveStateNodeId, aActiveStateEffectiveDisplayNameNodeId, HasProperty);
							// Add TypeDefintion
							uStatus = AddUAReference(aActiveStateEffectiveDisplayNameNodeId, PropertyType, HasTypeDefinition);
							if (uStatus == OpcUa_Good)
							{
								// ActiveState - TransitionTime
								OpcUa_UInt16 namespaceIndex = pConditionNodeId->NamespaceIndex;
								OpcUa_NodeId aActiveStateTransitionTimeNodeId;
								CreateNodeId(OpcUa_IdentifierType_Numeric, namespaceIndex, pConditionNodeId->Identifier.Numeric + 10, &aActiveStateTransitionTimeNodeId);
								OpcUa_Variant varVal;
								OpcUa_Variant_Initialize(&varVal);
								varVal.Datatype = OpcUaType_DateTime;
								OpcUa_String_Clear(&szBrowseName);
								OpcUa_String_Initialize(&szBrowseName);

								OpcUa_String_AttachCopy(&szBrowseName, "TransitionTime");
								uStatus = AddUAVariable(aActiveStateTransitionTimeNodeId, szBrowseName, PropertyType, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
								if (uStatus == OpcUa_Good)
								{
									// Make it visible behind the Condition definition
									uStatus = AddUAReference(aActiveStateNodeId, aActiveStateTransitionTimeNodeId, HasProperty);
									// Add TypeDefintion
									uStatus = AddUAReference(aActiveStateTransitionTimeNodeId, PropertyType, HasTypeDefinition);
									if (uStatus == OpcUa_Good)
									{
										// ActiveState - EffectiveTransitionTime
										//OpcUa_UInt16 namespaceIndex = pConditionNodeId->NamespaceIndex;
										//OpcUa_NodeId aActiveStateEffectiveTransitionTimeNodeId;
										//CreateNodeId(OpcUa_IdentifierType_Numeric, namespaceIndex, pConditionNodeId->Identifier.Numeric + 10, &aActiveStateEffectiveTransitionTimeNodeId);
										//OpcUa_Variant varVal;
										//OpcUa_Variant_Initialize(&varVal);
										//varVal.Datatype = OpcUaType_String;
										//OpcUa_String szBrowseName;

										//OpcUa_String_AttachCopy(&szBrowseName, "EffectiveTransitionTime");
										//uStatus = AddUAVariable(aActiveStateEffectiveTransitionTimeNodeId, szBrowseName, HasProperty, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
										//if (uStatus == OpcUa_Good)
										//{
										//	// Make it visible behind the Condition definition
										//	uStatus = AddUAReference(aActiveStateNodeId, aActiveStateEffectiveTransitionTimeNodeId, HasProperty);
										//	// Add TypeDefintion
										//	uStatus = AddUAReference(aActiveStateEffectiveTransitionTimeNodeId, PropertyType, HasTypeDefinition);
										//}
									}
								}
							}
						}
					}
				}
				// InputNode
				OpcUa_UInt16 namespaceIndex = pConditionNodeId->NamespaceIndex;
				OpcUa_NodeId aInputNodeNodeId;
				CreateNodeId(OpcUa_IdentifierType_Numeric, namespaceIndex, pConditionNodeId->Identifier.Numeric + 10, &aInputNodeNodeId);
				OpcUa_Variant varVal;
				OpcUa_Variant_Initialize(&varVal);
				varVal.Datatype = OpcUaType_String;
				OpcUa_String_Clear(&szBrowseName);
				OpcUa_String_Initialize(&szBrowseName);

				OpcUa_String_AttachCopy(&szBrowseName, "InputNode");
				uStatus = AddUAVariable(aInputNodeNodeId, szBrowseName, PropertyType, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
				if (uStatus == OpcUa_Good)
				{
					// Make it visible behind the Condition definition
					uStatus = AddUAReference(*pConditionNodeId, aInputNodeNodeId, HasProperty);
					// Add TypeDefintion
					uStatus = AddUAReference(aInputNodeNodeId, PropertyType, HasTypeDefinition);
					if (uStatus == OpcUa_Good)
					{
						// SuppressedState
						OpcUa_UInt16 namespaceIndex = pConditionNodeId->NamespaceIndex;
						OpcUa_NodeId aSuppressedStateNodeId;
						CreateNodeId(OpcUa_IdentifierType_Numeric, namespaceIndex, pConditionNodeId->Identifier.Numeric + 10, &aSuppressedStateNodeId);
						OpcUa_Variant varVal;
						OpcUa_Variant_Initialize(&varVal);
						varVal.Datatype = OpcUaType_String;
						OpcUa_String_Clear(&szBrowseName);
						OpcUa_String_Initialize(&szBrowseName);

						OpcUa_String_AttachCopy(&szBrowseName, "SuppressedState");
						uStatus = AddUAVariable(aSuppressedStateNodeId, szBrowseName, TwoStateVariableType, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
						if (uStatus == OpcUa_Good)
						{
							// Make it visible behind the Condition definition
							uStatus = AddUAReference(*pConditionNodeId, aSuppressedStateNodeId, HasProperty);
							// Add TypeDefintion
							uStatus = AddUAReference(aSuppressedStateNodeId, TwoStateVariableType, HasTypeDefinition);
							if (uStatus == OpcUa_Good)
							{
								// SuppressedState - Id
								OpcUa_UInt16 namespaceIndex = pConditionNodeId->NamespaceIndex;
								OpcUa_NodeId aSuppressedStateIdNodeId;
								CreateNodeId(OpcUa_IdentifierType_Numeric, namespaceIndex, pConditionNodeId->Identifier.Numeric + 10, &aSuppressedStateIdNodeId);
								OpcUa_Variant varVal;
								OpcUa_Variant_Initialize(&varVal);
								varVal.Datatype = OpcUaType_String;
								OpcUa_String_Clear(&szBrowseName);
								OpcUa_String_Initialize(&szBrowseName);

								OpcUa_String_AttachCopy(&szBrowseName, "Id");
								uStatus = AddUAVariable(aSuppressedStateIdNodeId, szBrowseName, HasProperty, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
								if (uStatus == OpcUa_Good)
								{
									// Make it visible behind the Condition definition
									uStatus = AddUAReference(aSuppressedStateNodeId, aSuppressedStateIdNodeId, HasProperty);
									// Add TypeDefintion
									uStatus = AddUAReference(aSuppressedStateIdNodeId, PropertyType, HasTypeDefinition);
									if (uStatus == OpcUa_Good)
									{
										// SuppressedState - TransitionTime
										OpcUa_UInt16 namespaceIndex = pConditionNodeId->NamespaceIndex;
										OpcUa_NodeId aSuppressedStateTransitionTimeNodeId;
										CreateNodeId(OpcUa_IdentifierType_Numeric, namespaceIndex, pConditionNodeId->Identifier.Numeric + 10, &aSuppressedStateTransitionTimeNodeId);
										OpcUa_Variant varVal;
										OpcUa_Variant_Initialize(&varVal);
										varVal.Datatype = OpcUaType_String;
										OpcUa_String_Clear(&szBrowseName);
										OpcUa_String_Initialize(&szBrowseName);

										OpcUa_String_AttachCopy(&szBrowseName, "TransitionTime");
										uStatus = AddUAVariable(aSuppressedStateTransitionTimeNodeId, szBrowseName, HasProperty, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
										if (uStatus == OpcUa_Good)
										{
											// Make it visible behind the Condition definition
											uStatus = AddUAReference(aSuppressedStateNodeId, aSuppressedStateTransitionTimeNodeId, HasProperty);
											// Add TypeDefintion
											uStatus = AddUAReference(aSuppressedStateTransitionTimeNodeId, PropertyType, HasTypeDefinition);
										}
									}
								}
							}
							// ShelvingState 
							OpcUa_UInt16 namespaceIndex = pConditionNodeId->NamespaceIndex;
							OpcUa_NodeId aShelvingStateNodeId;
							CreateNodeId(OpcUa_IdentifierType_Numeric, namespaceIndex, pConditionNodeId->Identifier.Numeric + 10, &aShelvingStateNodeId);
							OpcUa_Variant varVal;
							OpcUa_Variant_Initialize(&varVal);
							varVal.Datatype = OpcUaType_String;
							OpcUa_String_Clear(&szBrowseName);
							OpcUa_String_Initialize(&szBrowseName);
							OpcUa_String_AttachCopy(&szBrowseName, "ShelvingState");
							uStatus = AddUAVariable(aShelvingStateNodeId, szBrowseName, ShelvedStateMachineType, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
							if (uStatus == OpcUa_Good)
							{
								// Make it visible behind the Condition definition
								uStatus = AddUAReference(*pConditionNodeId, aShelvingStateNodeId, HasProperty);
								// Add TypeDefintion
								uStatus = AddUAReference(aShelvingStateNodeId, PropertyType, HasTypeDefinition);
								if (uStatus == OpcUa_Good)
								{
									// ShelvingState - CurrentState
									OpcUa_UInt16 namespaceIndex = pConditionNodeId->NamespaceIndex;
									OpcUa_NodeId aShelvingStateCurrentStateNodeId;
									CreateNodeId(OpcUa_IdentifierType_Numeric, namespaceIndex, pConditionNodeId->Identifier.Numeric + 10, &aShelvingStateCurrentStateNodeId);
									OpcUa_Variant varVal;
									OpcUa_Variant_Initialize(&varVal);
									varVal.Datatype = OpcUaType_String;
									OpcUa_String_Clear(&szBrowseName);
									OpcUa_String_Initialize(&szBrowseName);
									OpcUa_String_AttachCopy(&szBrowseName, "CurrentState");
									uStatus = AddUAVariable(aShelvingStateCurrentStateNodeId, szBrowseName, HasProperty, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
									if (uStatus == OpcUa_Good)
									{
										// Make it visible behind the Condition definition
										uStatus = AddUAReference(aShelvingStateNodeId, aShelvingStateCurrentStateNodeId, HasProperty);
										// Add TypeDefintion
										uStatus = AddUAReference(aShelvingStateCurrentStateNodeId, PropertyType, HasTypeDefinition);
										if (uStatus == OpcUa_Good)
										{
											// ShelvingState - LastTransition
											OpcUa_UInt16 namespaceIndex = pConditionNodeId->NamespaceIndex;
											OpcUa_NodeId aShelvingStateLastTransitionNodeId;
											CreateNodeId(OpcUa_IdentifierType_Numeric, namespaceIndex, pConditionNodeId->Identifier.Numeric + 10, &aShelvingStateLastTransitionNodeId);
											OpcUa_Variant varVal;
											OpcUa_Variant_Initialize(&varVal);
											varVal.Datatype = OpcUaType_String;
											OpcUa_String_Clear(&szBrowseName);
											OpcUa_String_Initialize(&szBrowseName);
											OpcUa_String_AttachCopy(&szBrowseName, "LastTransition");
											uStatus = AddUAVariable(aShelvingStateLastTransitionNodeId, szBrowseName, HasProperty, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
											if (uStatus == OpcUa_Good)
											{
												// Make it visible behind the Condition definition
												uStatus = AddUAReference(aShelvingStateNodeId, aShelvingStateLastTransitionNodeId, HasProperty);
												// Add TypeDefintion
												uStatus = AddUAReference(aShelvingStateLastTransitionNodeId, PropertyType, HasTypeDefinition);
												if (uStatus == OpcUa_Good)
												{
													// ShelvingState - UnshelveTime	
													OpcUa_UInt16 namespaceIndex = pConditionNodeId->NamespaceIndex;
													OpcUa_NodeId aShelvingStateUnshelveTimeNodeId;
													CreateNodeId(OpcUa_IdentifierType_Numeric, namespaceIndex, pConditionNodeId->Identifier.Numeric + 10, &aShelvingStateUnshelveTimeNodeId);
													OpcUa_Variant varVal;
													OpcUa_Variant_Initialize(&varVal);
													varVal.Datatype = OpcUaType_String;
													OpcUa_String_Clear(&szBrowseName);
													OpcUa_String_Initialize(&szBrowseName);
													OpcUa_String_AttachCopy(&szBrowseName, "UnshelveTime");
													uStatus = AddUAVariable(aShelvingStateUnshelveTimeNodeId, szBrowseName, HasProperty, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
													if (uStatus == OpcUa_Good)
													{
														// Make it visible behind the Condition definition
														uStatus = AddUAReference(aShelvingStateNodeId, aShelvingStateUnshelveTimeNodeId, HasProperty);
														// Add TypeDefintion
														uStatus = AddUAReference(aShelvingStateUnshelveTimeNodeId, PropertyType, HasTypeDefinition);
													}
												}
											}
										}
									}
								}
								// SuppressedOrShelved
								OpcUa_UInt16 namespaceIndex = pConditionNodeId->NamespaceIndex;
								OpcUa_NodeId aSuppressedOrShelvedNodeId;
								CreateNodeId(OpcUa_IdentifierType_Numeric, namespaceIndex, pConditionNodeId->Identifier.Numeric + 10, &aSuppressedOrShelvedNodeId);
								OpcUa_Variant varVal;
								OpcUa_Variant_Initialize(&varVal);
								varVal.Datatype = OpcUaType_String;
								OpcUa_String_Clear(&szBrowseName);
								OpcUa_String_Initialize(&szBrowseName);
								OpcUa_String_AttachCopy(&szBrowseName, "SuppressedOrShelved");
								uStatus = AddUAVariable(aSuppressedOrShelvedNodeId, szBrowseName, PropertyType, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
								if (uStatus == OpcUa_Good)
								{
									// Make it visible behind the Condition definition
									uStatus = AddUAReference(*pConditionNodeId, aSuppressedOrShelvedNodeId, HasProperty);
									// Add TypeDefintion
									uStatus = AddUAReference(aSuppressedOrShelvedNodeId, PropertyType, HasTypeDefinition);
									if (uStatus == OpcUa_Good)
									{
										// MaxTimeShelved
										OpcUa_UInt16 namespaceIndex = pConditionNodeId->NamespaceIndex;
										OpcUa_NodeId aMaxTimeShelvedNodeId;
										CreateNodeId(OpcUa_IdentifierType_Numeric, namespaceIndex, pConditionNodeId->Identifier.Numeric + 10, &aMaxTimeShelvedNodeId);
										OpcUa_Variant varVal;
										OpcUa_Variant_Initialize(&varVal);
										varVal.Datatype = OpcUaType_String;
										OpcUa_String_Clear(&szBrowseName);
										OpcUa_String_Initialize(&szBrowseName);
										OpcUa_String_AttachCopy(&szBrowseName, "MaxTimeShelved");
										uStatus = AddUAVariable(aMaxTimeShelvedNodeId, szBrowseName, PropertyType, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
										if (uStatus == OpcUa_Good)
										{
											// Make it visible behind the Condition definition
											uStatus = AddUAReference(*pConditionNodeId, aMaxTimeShelvedNodeId, HasProperty);
											// Add TypeDefintion
											uStatus = AddUAReference(aMaxTimeShelvedNodeId, PropertyType, HasTypeDefinition);
										}
									}
								}
							}
						}
					}
				}
			}
			OpcUa_String_Clear(&szBrowseName);
		}
	}
	OpcUa_NodeId_Clear(&HasComponent);
	OpcUa_NodeId_Clear(&PropertyType);
	OpcUa_NodeId_Clear(&HasProperty);
	OpcUa_NodeId_Clear(&HasTypeDefinition);
	OpcUa_NodeId_Clear(&TwoStateVariableType);
	OpcUa_NodeId_Clear(&ConditionVariableType);
	OpcUa_NodeId_Clear(&UtcTime);
	return uStatus;		
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Creates limit alarm condition type references. </summary>
///
/// <remarks>	Michel, 26/09/2016. </remarks>
///
/// <param name="ppNewCondition">	[in,out] If non-null, the new condition. </param>
///
/// <returns>	The new limit alarm condition type references. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode OpenOpcUa::UAAddressSpace::CUAInformationModel::CreateLimitAlarmConditionTypeReferences(CUAObject** ppNewCondition)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_NodeId HasComponent;
	OpcUa_NodeId_Initialize(&HasComponent);
	HasComponent.Identifier.Numeric = 47;// HasComponent

	OpcUa_NodeId PropertyType;
	OpcUa_NodeId_Initialize(&PropertyType);
	PropertyType.Identifier.Numeric = 68;// PropertyType

	OpcUa_NodeId HasProperty;
	OpcUa_NodeId_Initialize(&HasProperty);
	HasProperty.Identifier.Numeric = 46; // HasProperty

	OpcUa_NodeId HasTypeDefinition;
	OpcUa_NodeId_Initialize(&HasTypeDefinition);
	HasTypeDefinition.Identifier.Numeric = 40; // HasTypeDefinition

	OpcUa_NodeId TwoStateVariableType;
	OpcUa_NodeId_Initialize(&TwoStateVariableType);
	TwoStateVariableType.Identifier.Numeric = 8995;// "i=8995" BrowseName=TwoStateVariableType

	OpcUa_NodeId ConditionVariableType;
	OpcUa_NodeId_Initialize(&ConditionVariableType);
	ConditionVariableType.Identifier.Numeric = 9002;// "i=9002" BrowseName=ConditionVariableType

	OpcUa_NodeId ShelvedStateMachineType;
	OpcUa_NodeId_Initialize(&ShelvedStateMachineType);
	ShelvedStateMachineType.Identifier.Numeric = 2929;// "i=2929" BrowseName=ShelvedStateMachineType

	OpcUa_NodeId UtcTime;
	OpcUa_NodeId_Initialize(&UtcTime);
	UtcTime.Identifier.Numeric = 294;// "i=UtcTime" BrowseName=UtcTime
	uStatus = CreateAlarmConditionTypeReferences(ppNewCondition);
	if (uStatus == OpcUa_Good)
	{
		OpcUa_NodeId* pConditionNodeId = (*ppNewCondition)->GetNodeId();
		if (pConditionNodeId)
		{
			// HighHighAlarm
			OpcUa_UInt16 namespaceIndex = pConditionNodeId->NamespaceIndex;
			OpcUa_NodeId aHighHighAlarmNodeId;
			CreateNodeId(OpcUa_IdentifierType_Numeric, namespaceIndex, pConditionNodeId->Identifier.Numeric + 10, &aHighHighAlarmNodeId);
			OpcUa_Variant varVal;
			OpcUa_Variant_Initialize(&varVal);
			varVal.Datatype = OpcUaType_Double;
			OpcUa_String szBrowseName;
			OpcUa_String_Initialize(&szBrowseName);
			OpcUa_String_AttachCopy(&szBrowseName, "HighHighLimit");
			uStatus = AddUAVariable(aHighHighAlarmNodeId, szBrowseName, PropertyType, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
			if (uStatus == OpcUa_Good)
			{
				// Make it visible behind the Condition definition
				uStatus = AddUAReference(*pConditionNodeId, aHighHighAlarmNodeId, HasProperty);
				// Add TypeDefintion
				uStatus = AddUAReference(aHighHighAlarmNodeId, PropertyType, HasTypeDefinition);
				if (uStatus == OpcUa_Good)
				{
					// HighAlarm
					OpcUa_UInt16 namespaceIndex = pConditionNodeId->NamespaceIndex;
					OpcUa_NodeId aHighAlarmNodeId;
					CreateNodeId(OpcUa_IdentifierType_Numeric, namespaceIndex, pConditionNodeId->Identifier.Numeric + 10, &aHighAlarmNodeId);
					OpcUa_Variant varVal;
					OpcUa_Variant_Initialize(&varVal);
					varVal.Datatype = OpcUaType_Double;
					OpcUa_String_Clear(&szBrowseName);
					OpcUa_String_Initialize(&szBrowseName);

					OpcUa_String_AttachCopy(&szBrowseName, "HighLimit");
					uStatus = AddUAVariable(aHighAlarmNodeId, szBrowseName, PropertyType, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
					if (uStatus == OpcUa_Good)
					{
						// Make it visible behind the Condition definition
						uStatus = AddUAReference(*pConditionNodeId, aHighAlarmNodeId, HasProperty);
						// Add TypeDefintion
						uStatus = AddUAReference(aHighAlarmNodeId, PropertyType, HasTypeDefinition);
						if (uStatus == OpcUa_Good)
						{
							// LowAlarm
							OpcUa_UInt16 namespaceIndex = pConditionNodeId->NamespaceIndex;
							OpcUa_NodeId aLowAlarmNodeId;
							CreateNodeId(OpcUa_IdentifierType_Numeric, namespaceIndex, pConditionNodeId->Identifier.Numeric + 10, &aLowAlarmNodeId);
							OpcUa_Variant varVal;
							OpcUa_Variant_Initialize(&varVal);
							varVal.Datatype = OpcUaType_Double;
							OpcUa_String_Clear(&szBrowseName);
							OpcUa_String_Initialize(&szBrowseName);
							OpcUa_String_AttachCopy(&szBrowseName, "LowLimit");
							uStatus = AddUAVariable(aLowAlarmNodeId, szBrowseName, PropertyType, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
							if (uStatus == OpcUa_Good)
							{
								// Make it visible behind the Condition definition
								uStatus = AddUAReference(*pConditionNodeId, aLowAlarmNodeId, HasProperty);
								// Add TypeDefintion
								uStatus = AddUAReference(aLowAlarmNodeId, PropertyType, HasTypeDefinition);
								if (uStatus == OpcUa_Good)
								{
									// LowLowAlarm
									OpcUa_UInt16 namespaceIndex = pConditionNodeId->NamespaceIndex;
									OpcUa_NodeId aLowLowAlarmNodeId;
									CreateNodeId(OpcUa_IdentifierType_Numeric, namespaceIndex, pConditionNodeId->Identifier.Numeric + 10, &aLowLowAlarmNodeId);
									OpcUa_Variant varVal;
									OpcUa_Variant_Initialize(&varVal);
									varVal.Datatype = OpcUaType_Double;
									OpcUa_String_Clear(&szBrowseName);
									OpcUa_String_Initialize(&szBrowseName);
									OpcUa_String_AttachCopy(&szBrowseName, "LowLowLimit");
									uStatus = AddUAVariable(aLowLowAlarmNodeId, szBrowseName, PropertyType, varVal, OpcUa_AccessLevels_CurrentRead, OpcUa_False);
									if (uStatus == OpcUa_Good)
									{
										// Make it visible behind the Condition definition
										uStatus = AddUAReference(*pConditionNodeId, aLowLowAlarmNodeId, HasProperty);
										// Add TypeDefintion
										uStatus = AddUAReference(aLowLowAlarmNodeId, PropertyType, HasTypeDefinition);
									}
								}
							}
						}
					}
				}
			}
			OpcUa_String_Clear(&szBrowseName);
		}
	}
	return uStatus;		
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Creates discrete alarm type references. </summary>
///
/// <remarks>	Michel, 26/09/2016. </remarks>
///
/// <param name="ppNewCondition">	[in,out] If non-null, the new condition. </param>
///
/// <returns>	The new discrete alarm type references. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode OpenOpcUa::UAAddressSpace::CUAInformationModel::CreateDiscreteAlarmTypeReferences(CUAObject**ppNewCondition)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	uStatus = CreateAlarmConditionTypeReferences(ppNewCondition);
	if (uStatus == OpcUa_Good)
	{
	}
	return uStatus;		
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Creates off normal alarm type references. </summary>
///
/// <remarks>	Michel, 26/09/2016. </remarks>
///
/// <param name="ppNewCondition">	[in,out] If non-null, the new condition. </param>
///
/// <returns>	The new off normal alarm type references. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode OpenOpcUa::UAAddressSpace::CUAInformationModel::CreateOffNormalAlarmTypeReferences(CUAObject**ppNewCondition)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	uStatus = CreateDiscreteAlarmTypeReferences(ppNewCondition);
	if (uStatus == OpcUa_Good)
	{
	}
	return uStatus;		
}

