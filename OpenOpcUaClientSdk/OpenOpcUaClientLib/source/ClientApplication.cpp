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


#include "MonitoredItemClient.h"
#include "MonitoredItemsNotification.h"
#include "SubscriptionClient.h"
#include "ClientSession.h"
#include "LoggerMessage.h"
#include "ClientApplication.h"
#include "StatusCodeException.h"
#include "Channel.h"

using namespace OpenOpcUa;
using namespace UASharedLib;
using namespace UACoreClient;
typedef struct _TMonitoredItem
{
	char*						szNodeId;
	OpcUa_Int32					iAttributeId;
	char*						szIndexRange;
	OpcUa_Double				dblSamplingInterval;
	OpcUa_Int32					iQueueSize;
	OpcUa_Int32					iMonitoringMode;
	OpcUa_UInt32				uiMaxNotificationperPublish;
	OpcUa_Boolean				bDiscardOldest;
} TMonitoredItem;
// structure userData associé au parsing aux fichiers xml de simulation
typedef struct handler_ClientConfig_Data
{
	XML_ParserStruct*			XML_Parser;
	void*						userData;
	CClientApplication*			pClientApplication;
	// Session
	char*						szHostName;
	char*						szEndpointUrl;
	OpcUa_MessageSecurityMode	eSecurityMode;
	char*						szSecurityPolicy;
	char*						szSessionName;
	OpcUa_Double				dblSessionTimeout;
	OpcUa_Handle				hCurrentSession;
	// Subscription
	char*						szSubscriptionName;
	OpcUa_Double				dblPublishInterval;
	OpcUa_UInt32				uiLifetimeCount;
	OpcUa_UInt32				uiMaxKeepaliveCount;
	OpcUa_Boolean				bPublishEnable;
	OpcUa_Handle				hCurrentSubscription;
	OpcUa_Byte					Priority;
	OpcUa_UInt32				uiMaxNotificationperPublish;
	OpcUa_Int32					eMonitoringMode;
	// MonitoredItem
	vector<TMonitoredItem*>		monitedItemList;
	/*
	char*						szNodeId;
	OpcUa_Int32					iAttributeId;
	char*						szIndexRange;
	OpcUa_Double				dblSamplingInterval;
	OpcUa_Int32					iQueueSize;
	OpcUa_Int32					iMonitoringMode;
	OpcUa_Boolean				bDiscardOldest;
	*/
	OpcUa_StatusCode			uStatus;

} HANDLER_CLIENTCONFIG_DATA;
////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
// Fonction délegué pour la prise en charge du chargement des fichiers de configuration de la connection client/server
/// <summary>
/// XMLs the client configuration start element handler.
/// </summary>
/// <param name="userData">The user data.</param>
/// <param name="name">The name.</param>
/// <param name="atts">The atts.</param>
void xmlClientConfigStartElementHandler(
	void *userData,     /* user defined data */
	const char *name,   /* Element name */
	const char **atts)  /* Element Attribute list, provided in name value pairs */
	/* i.e. atts[0] contains name, atts[1] contains value for */
	/* atts[0], and so on...*/
{
	OpcUa_StatusCode uStatus;
	HANDLER_CLIENTCONFIG_DATA* pMyData = (HANDLER_CLIENTCONFIG_DATA*)userData;
	if (name)
	{
		//if (OpcUa_StrCmpA(name, "ClientConfig") == 0)
		//{
		//	pMyData->pClientApplication;
		//}
		if (OpcUa_StrCmpA(name, "Session") == 0)
		{
			int ii = 0;
			while (atts[ii])
			{
				// EndpointUrl
				if (OpcUa_StrCmpA(atts[ii], "EndpointUrl") == 0)
				{
					OpcUa_UInt32 iLen = strlen(atts[ii + 1]) + 1;
					pMyData->szEndpointUrl = (char*)OpcUa_Alloc(iLen);
					ZeroMemory(pMyData->szEndpointUrl, iLen);
					strcpy(pMyData->szEndpointUrl, atts[ii + 1]);
				}
				// SecurityMode
				if (OpcUa_StrCmpA(atts[ii], "SecurityMode") == 0)
				{
					pMyData->eSecurityMode = (OpcUa_MessageSecurityMode)atoi(atts[ii + 1]);
				}
				// SecurityPolicy
				if (OpcUa_StrCmpA(atts[ii], "SecurityPolicy") == 0)
				{
					OpcUa_UInt32 iLen = strlen(atts[ii + 1]) + 1;
					pMyData->szSecurityPolicy = (char*)OpcUa_Alloc(iLen);
					ZeroMemory(pMyData->szSecurityPolicy, iLen);
					strcpy(pMyData->szSecurityPolicy, atts[ii + 1]);
				}
				// Name (Session name)
				if (OpcUa_StrCmpA(atts[ii], "Name") == 0)
				{
					OpcUa_UInt32 iLen = strlen(atts[ii + 1]) + 1;
					pMyData->szSessionName = (char*)OpcUa_Alloc(iLen);
					ZeroMemory(pMyData->szSessionName, iLen);
					strcpy(pMyData->szSessionName, atts[ii + 1]);
				}
				// Client Timeout
				if (OpcUa_StrCmpA(atts[ii], "Timeout") == 0)
				{
					pMyData->dblSessionTimeout = (OpcUa_Double)atof(atts[ii + 1]);
				}
				ii += 2;
			}
			// Create the session and activate it
			OpcUa_String* pszEndpointUrl = OpcUa_Null;
			pszEndpointUrl = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
			if (pszEndpointUrl)
			{
				// retrieve the Endpoints  using OpenOpcUa_GetEndpoints based on :
				// pMyData->szHostName and pMyData->pClientApplication
				OpcUa_UInt32 uiNbOfEndpointDescription = 0;
				OpcUa_EndpointDescription* pEndpointDescription = OpcUa_Null;
				OpcUa_String_AttachCopy(pszEndpointUrl, pMyData->szEndpointUrl);
				OpcUa_Handle hApplication = (OpcUa_Handle)pMyData->pClientApplication;
				uStatus = OpenOpcUa_GetEndpoints(hApplication,
					pszEndpointUrl,
					&uiNbOfEndpointDescription,
					&pEndpointDescription);
				// pick the proper security mode and policy
				if (uStatus == OpcUa_Good)
				{
					OpcUa_String SecurityPolicy;
					OpcUa_String_Initialize(&SecurityPolicy);
					OpcUa_String_AttachCopy(&SecurityPolicy, pMyData->szSecurityPolicy);

					for (OpcUa_UInt32 i = 0; i < uiNbOfEndpointDescription; i++)
					{
						if (pEndpointDescription[i].SecurityMode == pMyData->eSecurityMode)
						{
							if (OpcUa_String_Compare(&SecurityPolicy, &(pEndpointDescription[i].SecurityPolicyUri)) == 0)
							{
								// Let's connect
								OpcUa_String SessionName;
								OpcUa_String_Initialize(&SessionName);
								OpcUa_String_AttachCopy(&SessionName, pMyData->szSessionName);

								uStatus = OpenOpcUa_CreateSession(hApplication,
									&pEndpointDescription[i],
									pMyData->dblSessionTimeout,
									SessionName,
									&(pMyData->hCurrentSession));
								if (uStatus == OpcUa_Good)
								{
									// Now we can Activate the session
									uStatus = OpenOpcUa_ActivateSession(hApplication, pMyData->hCurrentSession, pEndpointDescription,"anonymous","","");
									if (uStatus != OpcUa_Good)
									{
										OpenOpcUa_Trace(hApplication, OPCUA_TRACE_CLIENT_LEVEL_ERROR,
											"OpenOpcUa_ActivateSession failed error 0x%x. Verify your configuration file\n",
											uStatus);
										pMyData->uStatus = uStatus;
									}
								}
								else
								{
									OpenOpcUa_Trace(hApplication, OPCUA_TRACE_CLIENT_LEVEL_ERROR,
										"OpenOpcUa_CreateSession failed  error 0x%x. Verify your configuration file\n",
										uStatus);
									pMyData->uStatus = uStatus;
								}
								OpcUa_String_Clear(&SessionName);
								
								break;
							}

						}
						OpcUa_EndpointDescription_Clear(&pEndpointDescription[i]);
					}
					for (OpcUa_UInt32 i = 0; i < uiNbOfEndpointDescription; i++)
						OpcUa_EndpointDescription_Clear(&pEndpointDescription[i]);
					if (pEndpointDescription)
						OpcUa_Free(pEndpointDescription);
					OpcUa_String_Clear(&SecurityPolicy);
				}
				else
				{
					// Notify error to the Application
					OpenOpcUa_Trace(hApplication, OPCUA_TRACE_CLIENT_LEVEL_ERROR, "LoadConfiguration failed:OpenOpcUa_GetEndpoints return 0x%x\n", uStatus);
					pMyData->uStatus = uStatus;
				}
				OpcUa_String_Clear(pszEndpointUrl);
				OpcUa_Free(pszEndpointUrl);
			}
		}
		if (OpcUa_StrCmpA(name, "Subscription") == 0)
		{
			int ii = 0;
			while (atts[ii])
			{
				// Name
				if (OpcUa_StrCmpA(atts[ii], "Name") == 0)
				{
					OpcUa_UInt32 iLen = strlen(atts[ii + 1]) + 1;
					pMyData->szSubscriptionName = (char*)OpcUa_Alloc(iLen);
					ZeroMemory(pMyData->szSubscriptionName, iLen);
					strcpy(pMyData->szSubscriptionName, atts[ii + 1]);
				}
				// PublishInterval
				if (OpcUa_StrCmpA(atts[ii], "PublishInterval") == 0)
				{
					pMyData->dblPublishInterval = (OpcUa_Double)atof(atts[ii + 1]);
				}
				// LifetimeCount
				if (OpcUa_StrCmpA(atts[ii], "LifetimeCount") == 0)
				{
					pMyData->uiLifetimeCount = (OpcUa_UInt32)atoi(atts[ii + 1]);
				}
				// MaxKeepaliveCount
				if (OpcUa_StrCmpA(atts[ii], "MaxKeepaliveCount") == 0)
				{
					pMyData->uiMaxKeepaliveCount = (OpcUa_UInt32)atoi(atts[ii + 1]);
				}
				// PublishEnable
				if (OpcUa_StrCmpA(atts[ii], "PublishEnable") == 0)
				{
					if (strcmp(atts[ii + 1], "true")==0)
						pMyData->bPublishEnable = (OpcUa_Boolean)OpcUa_True;
					else
						pMyData->bPublishEnable = (OpcUa_Boolean)OpcUa_False;
				}
				// MaxNotificationperPublish
				if (OpcUa_StrCmpA(atts[ii], "MaxNotificationperPublish") == 0)//MaxNotificationsPerPublish
				{
					pMyData->uiMaxNotificationperPublish = (OpcUa_UInt32)atoi(atts[ii + 1]);
				}
				// Priority
				if (OpcUa_StrCmpA(atts[ii], "Priority") == 0)
				{
					pMyData->Priority = (OpcUa_Byte)atoi(atts[ii + 1]);
				}
				// MonitoringMode
				if (OpcUa_StrCmpA(atts[ii], "MonitoringMode") == 0)
				{
					pMyData->eMonitoringMode = (OpcUa_Byte)atoi(atts[ii + 1]);
				}
				ii += 2;
			}
			//
			OpcUa_Handle hApplication = (OpcUa_Handle)pMyData->pClientApplication;
			OpcUa_Handle hSession = (OpcUa_Handle)pMyData->hCurrentSession;
			if (hSession)
			{
				uStatus = OpenOpcUa_CreateSubscription(
					hApplication, hSession,
					&(pMyData->dblPublishInterval),
					&(pMyData->uiLifetimeCount), &(pMyData->uiMaxKeepaliveCount),
					pMyData->uiMaxNotificationperPublish,OpcUa_False, // pMyData->bPublishEnable
					pMyData->Priority,
					&(pMyData->hCurrentSubscription));
				if (uStatus != OpcUa_Good)
				{
					OpenOpcUa_Trace(hApplication, OPCUA_TRACE_CLIENT_LEVEL_ERROR,
						"OpenOpcUa_CreateSubscription failed error 0x%x. Verify your configuration file\n",
						uStatus);
					pMyData->uStatus = uStatus;
				}
				else
				{
					// Restaure the default MonitoringMode
					OpenOpcUa_SetMonitoringMode(hApplication, hSession, pMyData->hCurrentSubscription, 0, OpcUa_Null, (OpcUa_MonitoringMode)pMyData->eMonitoringMode);
					// Restore the default MaxNotificationsPerPublish
				}
			}
		}
		if (OpcUa_StrCmpA(name, "MonitoredItem") == 0)
		{
			TMonitoredItem* pNewMonitoredItem = (TMonitoredItem*)OpcUa_Alloc(sizeof(TMonitoredItem));
			if (pNewMonitoredItem)
			{
				ZeroMemory(pNewMonitoredItem, sizeof(TMonitoredItem));
				int ii = 0;
				while (atts[ii])
				{
					// NodeId
					if (OpcUa_StrCmpA(atts[ii], "NodeId") == 0)
					{
						OpcUa_UInt32 iLen = strlen(atts[ii + 1]) + 1;
						pNewMonitoredItem->szNodeId = (char*)OpcUa_Alloc(iLen);
						ZeroMemory(pNewMonitoredItem->szNodeId, iLen);
						strcpy(pNewMonitoredItem->szNodeId, atts[ii + 1]);
					}
					// AttributeId
					if (OpcUa_StrCmpA(atts[ii], "AttributeId") == 0)
					{
						pNewMonitoredItem->iAttributeId = (OpcUa_Int32)atoi(atts[ii + 1]);
					}
					// IndexRange
					if (OpcUa_StrCmpA(atts[ii], "IndexRange") == 0)
					{
						OpcUa_UInt32 iLen = strlen(atts[ii + 1]) + 1;
						pNewMonitoredItem->szIndexRange = (char*)OpcUa_Alloc(iLen);
						ZeroMemory(pNewMonitoredItem->szIndexRange, iLen);
						strcpy(pNewMonitoredItem->szIndexRange, atts[ii + 1]);
					}
					// SamplingInterval
					if (OpcUa_StrCmpA(atts[ii], "SamplingInterval") == 0)
					{
						pNewMonitoredItem->dblSamplingInterval = (OpcUa_Double)atof(atts[ii + 1]);
					}
					// MonitoringMode
					if (OpcUa_StrCmpA(atts[ii], "MonitoringMode") == 0)
					{
						pNewMonitoredItem->iMonitoringMode = (OpcUa_Int32)atoi(atts[ii + 1]);
					}
					// QueueSize
					if (OpcUa_StrCmpA(atts[ii], "QueueSize") == 0)
					{
						pNewMonitoredItem->iQueueSize = (OpcUa_Int32)atoi(atts[ii + 1]);
					}
					// DiscardOldest
					if (OpcUa_StrCmpA(atts[ii], "DiscardOldest") == 0)
					{
						pNewMonitoredItem->bDiscardOldest = (OpcUa_Boolean)atoi(atts[ii + 1]);
					}
					ii += 2;
				}
				// 
				pMyData->monitedItemList.push_back(pNewMonitoredItem);
			}
		}
	}
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Handler, called when the XML client configuration end element. </summary>
///
/// <remarks>	Michel, 14/02/2016. </remarks>
///
/// <param name="userData">	[in,out] The user data. </param>
/// <param name="name">	   	The name. </param>
///-------------------------------------------------------------------------------------------------

void xmlClientConfigEndElementHandler(
	void *userData,     /* user defined data */
	const char *name)   /* Element name */
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	HANDLER_CLIENTCONFIG_DATA* pMyData = (HANDLER_CLIENTCONFIG_DATA*)userData;
	if (name)
	{
		if (OpcUa_StrCmpA(name, "ClientConfig") == 0)
		{

		}
		if (OpcUa_StrCmpA(name, "Session") == 0)
		{
			pMyData->hCurrentSession = OpcUa_Null;
		}
		if (OpcUa_StrCmpA(name, "Subscription") == 0)
		{
			OpcUa_Handle hApplication = (OpcUa_Handle)pMyData->pClientApplication;
			OpcUa_Handle hSession = pMyData->hCurrentSession;
			OpcUa_Handle hSubscription = pMyData->hCurrentSubscription;
			// Here is the place to restore the subscription
			// This means to create all monitoredItems for the subscription
			OpcUa_UInt32 iNoOfItemsToCreate = pMyData->monitedItemList.size();
			if (iNoOfItemsToCreate > 0)
			{
				OpcUa_MonitoredItemCreateRequest* pItemsToCreate = (OpcUa_MonitoredItemCreateRequest*)OpcUa_Alloc(sizeof(OpcUa_MonitoredItemCreateRequest)*iNoOfItemsToCreate);
				if (pItemsToCreate)
				{
					for (OpcUa_UInt32 i = 0; i < pMyData->monitedItemList.size(); i++)
					{
						TMonitoredItem* pNewMonitoredItem = pMyData->monitedItemList.at(i);
						OpcUa_NodeId aNodeId;
						OpcUa_NodeId_Initialize(&aNodeId);
						OpcUa_String szNodeId;
						OpcUa_String_Initialize(&szNodeId);
						OpcUa_String_AttachCopy(&szNodeId, pNewMonitoredItem->szNodeId);
						if (OpenOpcUa_StringToNodeId(szNodeId, &aNodeId) == OpcUa_Good)
						{
							OpcUa_MonitoredItemCreateRequest_Initialize(&pItemsToCreate[i]);
							OpcUa_NodeId_CopyTo(&aNodeId, &(pItemsToCreate[i].ItemToMonitor.NodeId));
							pItemsToCreate[i].ItemToMonitor.AttributeId = pNewMonitoredItem->iAttributeId;
							pItemsToCreate[i].MonitoringMode = (OpcUa_MonitoringMode)pNewMonitoredItem->iMonitoringMode;
							pItemsToCreate[i].RequestedParameters.SamplingInterval = pNewMonitoredItem->dblSamplingInterval;
							pItemsToCreate[i].RequestedParameters.QueueSize = pNewMonitoredItem->iQueueSize;
							pItemsToCreate[i].RequestedParameters.ClientHandle = (OpcUa_UInt32)(uintptr_t)pNewMonitoredItem;
							pItemsToCreate[i].RequestedParameters.DiscardOldest = pNewMonitoredItem->bDiscardOldest;
						}
						OpcUa_NodeId_Clear(&aNodeId);
						OpcUa_String_Clear(&szNodeId);
					}

					////
					OpcUa_MonitoredItemCreateResult* ppResult = OpcUa_Null;

					OpcUa_Handle* hMonitoredItems = OpcUa_Null;
					uStatus = OpenOpcUa_CreateMonitoredItems(
						hApplication, hSession, hSubscription,
						OpcUa_TimestampsToReturn_Both,
						iNoOfItemsToCreate, pItemsToCreate,
						&ppResult, &hMonitoredItems);
					if (uStatus != OpcUa_Good)
					{
						OpenOpcUa_Trace(hApplication, OPCUA_TRACE_CLIENT_LEVEL_ERROR,
							"OpenOpcUa_CreateSubscription failed error 0x%x. Verify your configuration file\n",
							uStatus);
						pMyData->uStatus = uStatus;
					}
					else
					{
						for (OpcUa_UInt32 i = 0; i < iNoOfItemsToCreate; i++)
							OpcUa_MonitoredItemCreateResult_Clear(&ppResult[i]);
						OpcUa_Free(ppResult);
					}
					OpcUa_Free(hMonitoredItems); 
					hMonitoredItems = OpcUa_Null;
					for (OpcUa_UInt32 i = 0; i < pMyData->monitedItemList.size(); i++)
					{
						// Release resource for OpcUa_MonitoredItemCreateRequest
						OpcUa_NodeId_Clear(&(pItemsToCreate[i].ItemToMonitor.NodeId));
						// Release ressouce for TMonitoredItem
						TMonitoredItem* pNewMonitoredItem = pMyData->monitedItemList.at(i);
						OpcUa_Free(pNewMonitoredItem->szIndexRange);
						OpcUa_Free(pNewMonitoredItem->szNodeId);
						OpcUa_Free(pNewMonitoredItem);
						pNewMonitoredItem = OpcUa_Null;
					}
					// Release resource
					OpcUa_Free(pItemsToCreate);
					pItemsToCreate = OpcUa_Null;
					pMyData->monitedItemList.clear();
				}
			}
			uStatus = OpenOpcUa_SetPublishingMode(hApplication, hSession, hSubscription, pMyData->bPublishEnable);
			pMyData->hCurrentSubscription = OpcUa_Null;
		}
	}
}
CClientApplication::CClientApplication(void)
{
	// initialize the application and the stack.
	InitializeAbstractionLayer();
	// Need to put the TraceFilename before calling this function
	CApplication::InitializeTrace();

	m_pApplicationDescription=OpcUa_Null;
	m_pSessionClientList=new CSessionClientList();
	OpcUa_Mutex_Create(&m_hSessionsMutex);
	OpcUa_Mutex_Create(&m_hMessageLoggerMutex);
	OpcUa_Semaphore_Create(&m_InternalMessageSem,0,0x100);
	m_hInternalMessageThread=OpcUa_Null;

	StartInternalMessageThread();
	// initialisation du chemin
	m_LogFileName=OpcUa_Null;
	m_LogPathName=OpcUa_Null;
	OpcUa_String strPath;
	OpcUa_String_Initialize(&strPath);
	OpcUa_String_AttachCopy(&strPath,".");
	SetLogPathName(&strPath);
	OpcUa_String_Clear(&strPath);
}
OpcUa_StatusCode CClientApplication::SetApplicationDescription(OpcUa_ApplicationDescription* pAppDescription)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	if (!m_pApplicationDescription)
	{
		m_pApplicationDescription=new CApplicationDescription(pAppDescription);
		// il faut mettre a jour les valeur de classe de la classe parent afin de maintenir la cohérence de l'héritage
		OpcUa_LocalizedText pLocalizedText=m_pApplicationDescription->GetApplicationName();
		SetApplicationName(&(pLocalizedText));
		// definition du nom du fichier de log
		OpcUa_String strExtension;
		OpcUa_String_AttachCopy(&strExtension,".log");
		OpcUa_String FileName;
		OpcUa_String_StrnCpy(
			&FileName,
			&(pLocalizedText.Text),
			OpcUa_String_StrLen(&(pLocalizedText.Text)));
		OpcUa_String_StrnCat(
			&FileName,
			&strExtension,
			OpcUa_String_StrLen(&strExtension));
		SetLogFileName(&FileName);
		OpcUa_String_Clear(&strExtension);
		OpcUa_String_Clear(&FileName);
	}
	else
		uStatus=OpcUa_BadInternalError;
	return uStatus;
}

CClientApplication::~CClientApplication(void)
{
	// delete all the session for theis application
	DeleteAllSessions();
	// stop the message thread
	StopInternalMessageThread();
	if (m_LogFileName)
	{
		OpcUa_String_Clear(m_LogFileName);
		OpcUa_Free(m_LogFileName);
	}
	if (m_LogPathName)
	{
		OpcUa_String_Clear(m_LogPathName);
		OpcUa_Free(m_LogPathName);
	}
	while (!m_LoggerMessageList.empty())
	{
		m_LoggerMessageList.erase(m_LoggerMessageList.begin());
	}
	if(m_pApplicationDescription)
		delete m_pApplicationDescription;
	OpcUa_Mutex_Lock(m_hSessionsMutex);
	if (m_pSessionClientList)
		delete m_pSessionClientList;
	OpcUa_Mutex_Unlock(m_hSessionsMutex);
	OpcUa_Mutex_Delete(&m_hSessionsMutex);
	OpcUa_Mutex_Delete(&m_hMessageLoggerMutex);
	OpcUa_Semaphore_Delete(&m_InternalMessageSem);
}

OpcUa_StatusCode CClientApplication::FindServers(std::string hostName, CApplicationDescriptionList** pServerDescriptionList)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	CChannel channel(this);
	(*pServerDescriptionList)->clear();

	OpcUa_RequestHeader tRequestHeader;
	OpcUa_String sDiscoveryUrl;
	OpcUa_String sLocaleIds;
	OpcUa_String sServerUris;
	OpcUa_ResponseHeader tResponseHeader;
	OpcUa_Int32 nNoOfServers = 0;
	OpcUa_ApplicationDescription* pServers = OpcUa_Null;

	
	OpcUa_RequestHeader_Initialize(&tRequestHeader);
	OpcUa_String_Initialize(&sDiscoveryUrl);
	OpcUa_String_Initialize(&sLocaleIds);
	OpcUa_String_Initialize(&sServerUris);
	OpcUa_ResponseHeader_Initialize(&tResponseHeader);
	std::string tmpDiscoveryUrl;
	// construct the discovery url from the host name.	
	tmpDiscoveryUrl.append("opc.tcp://");

	if (!hostName.empty())
		tmpDiscoveryUrl.append(hostName);
	else
		tmpDiscoveryUrl.append("localhost");

	tmpDiscoveryUrl.append(":4840");
	uStatus=OpcUa_String_AttachCopy(&sDiscoveryUrl,(OpcUa_CharA*)tmpDiscoveryUrl.c_str());
	if (uStatus==OpcUa_Good)
	{
		OpcUa_MessageSecurityMode eSecurityMode = OpcUa_MessageSecurityMode_None;
		OpcUa_String szSecurityPolicy;
		OpcUa_String_Initialize(&szSecurityPolicy);
		uStatus = channel.Connect(sDiscoveryUrl,eSecurityMode,szSecurityPolicy);
		if (uStatus==OpcUa_Good)
		{
			// find the servers.
			tRequestHeader.TimeoutHint = g_ServiceCallTimeout;// UTILS_DEFAULT_TIMEOUT;
			tRequestHeader.Timestamp   = OpcUa_DateTime_UtcNow();

			// find the servers.
			uStatus = OpcUa_ClientApi_FindServers(  
				channel.GetInternalHandle(), 
				&tRequestHeader,
				&sDiscoveryUrl,
				0,
				&sLocaleIds, 
				0,
				&sServerUris, 
				&tResponseHeader,
				&nNoOfServers,
				&pServers);

			if (uStatus==OpcUa_Good)
			{		
				// copy the available servers into a vector.
				if (pServers != OpcUa_Null)
				{
					for (OpcUa_Int32 ii = 0; ii < nNoOfServers; ii++)
					{ 
						// ignore discovery servers.
						if (pServers[ii].ApplicationType != OpcUa_ApplicationType_DiscoveryServer)
						{
							CApplicationDescription* pAppDescription=new CApplicationDescription(&pServers[ii]);
							if (pAppDescription)
								(*pServerDescriptionList)->push_back(pAppDescription);
						}
					}
				}

				// clean up.
				OpcUa_RequestHeader_Clear(&tRequestHeader);
				OpcUa_String_Clear(&sDiscoveryUrl);
				OpcUa_String_Clear(&sLocaleIds);
				OpcUa_String_Clear(&sServerUris);
				OpcUa_ResponseHeader_Clear(&tResponseHeader);
				Utils_ClearArray(pServers, nNoOfServers, OpcUa_ApplicationDescription);

				// disconnect.No need the disconnect will be made in the destructor of CChannel object
				//channel.Disconnect();
			}
		}
		else
			OpcUa_Trace(OPCUA_TRACE_CLIENT_LEVEL_ERROR,"Cannot connect to CChannel uStatus=0x%05x\n",uStatus);
	}
	return uStatus;
}
OpcUa_StatusCode CClientApplication::AddLoggerMessage(OpcUa_String* Message,DWORD Level)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	//char	FileName [64];
	char	s_Lig[512];
	//FILE*	fpT=NULL;

	//ZeroMemory(FileName,64);
	ZeroMemory(s_Lig,512);
	
	if (OpcUa_String_StrLen(Message)<=0 )
		return OpcUa_BadInvalidArgument;
	CApplicationDescription* pAppDescription = GetApplicationDescription();
	OpcUa_LocalizedText aLocalizedText = pAppDescription->GetApplicationName();
	sprintf(s_Lig, "%s> %s",
		OpcUa_String_GetRawString(&(aLocalizedText.Text)),
		OpcUa_String_GetRawString(Message));
	OpcUa_Trace(Level, s_Lig);
	// Will open the archive file
	/*
	OpcUa_String* aFileName=GetLogFileName();
	if (aFileName)
	{
		if (OpcUa_String_StrLen(aFileName)>0)
		{
			OpcUa_String* aPathName=GetLogPathName();
			sprintf( FileName, "%s/%s", OpcUa_String_GetRawString(aPathName),OpcUa_String_GetRawString(aFileName));
			fpT = fopen( FileName, "a+");
			if (fpT == OpcUa_Null) 
			{
				uStatus=OpcUa_BadFileNotFound;
				fprintf( stderr, "Failed to open %s %d\n", FileName, errno);
				fpT = fopen( FileName, "w+");
				if (fpT==NULL)
					fpT = stdout;
			}
			else 
			{
				OpcUa_DateTime utcTime=OpcUa_DateTime_UtcNow();

				OpcUa_String* strTime=OpcUa_Null;
				uStatus=OpenOpcUa_DateTimeToString(utcTime,&strTime);
				if (uStatus==OpcUa_Good)
				{
					CApplicationDescription* pAppDescription=GetApplicationDescription();
					OpcUa_LocalizedText aLocalizedText=pAppDescription->GetApplicationName();
					sprintf(s_Lig,"%s -%s> %s",
						OpcUa_String_GetRawString(strTime),
						OpcUa_String_GetRawString(&(aLocalizedText.Text)), 
						OpcUa_String_GetRawString(Message));
					fprintf(fpT,"%s\n",s_Lig);	
					fclose(fpT);		

				}
				OpcUa_String_Clear(strTime);
				OpcUa_Free(strTime);
			}
		}
		else
			uStatus=OpcUa_BadInvalidArgument;
	}
	else
		uStatus=OpcUa_BadInvalidArgument;*/
	return uStatus;
}
OpcUa_StatusCode CClientApplication::StartInternalMessageThread()
{
	OpcUa_StatusCode uStatus=OpcUa_Good;	
	if (m_hInternalMessageThread==NULL)
	{
		m_bRunInternalMessageThread=TRUE;
		OpcUa_StatusCode uStatus  = OpcUa_Thread_Create(&m_hInternalMessageThread,(CClientApplication::InternalMessageThread),this);

		if(uStatus != OpcUa_Good)
			OpcUa_Trace(OPCUA_TRACE_CLIENT_LEVEL_ERROR,"Start InternalMessageThread Failed");
		else
			OpcUa_Thread_Start(m_hInternalMessageThread);
	}
	return uStatus;
}
OpcUa_StatusCode CClientApplication::StopInternalMessageThread()
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	m_bRunInternalMessageThread=OpcUa_False;
	OpcUa_Semaphore_Post(m_InternalMessageSem,1);
	uStatus = OpcUa_Semaphore_TimedWait( m_InternalMessageSem,15000); // 15 secondes max.
	if (uStatus == OpcUa_GoodNonCriticalTimeout)
	{
		// on force la fin du thread de simulation
		OpcUa_Trace(OPCUA_TRACE_CLIENT_LEVEL_ERROR,"Impossible to stop the InternalMessageThread. Timeout");
		//OpcUa_Thread_Delete(&m_hInternalMessageThread);
		uStatus=OpcUa_Bad;
	}
	else
	{
		OpcUa_Thread_Delete(m_hInternalMessageThread);
		OpcUa_Trace(OPCUA_TRACE_CLIENT_LEVEL_ERROR, "InternalMessageThread stopped properly\n");
	}
	return uStatus;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// thread qui dépile les messages presents dans theMessageList
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void /*__stdcall*/ CClientApplication::InternalMessageThread(LPVOID arg)
{
	CClientApplication* pClientApplication=(CClientApplication*)arg;

	while ( pClientApplication->m_bRunInternalMessageThread)
	{
		OpcUa_Mutex_Lock(pClientApplication->m_hMessageLoggerMutex);
		for (OpcUa_UInt32 ii=0;ii<pClientApplication->m_LoggerMessageList.size();ii++)
		{
			CLoggerMessage* pLoggerMessage=	(CLoggerMessage*)pClientApplication->m_LoggerMessageList.at(ii);
			if (pLoggerMessage)
			{
				try
				{
					OpcUa_Mutex_Unlock(pClientApplication->m_hMessageLoggerMutex);
					//ecriture du message dans le fichier cible
					if (pClientApplication->m_bRunInternalMessageThread)
						pClientApplication->AddLoggerMessage(pLoggerMessage->m_pString, pLoggerMessage->m_uiLevel);
					OpcUa_Mutex_Lock(pClientApplication->m_hMessageLoggerMutex);
					delete pLoggerMessage;
				}
				catch (...)
				{
					OpcUa_Mutex_Unlock(pClientApplication->m_hMessageLoggerMutex);
				}
			}
		}
		// On vide la liste des messages
		pClientApplication->m_LoggerMessageList.clear();
		OpcUa_Mutex_Unlock(pClientApplication->m_hMessageLoggerMutex);
		OpcUa_Semaphore_TimedWait(pClientApplication->m_InternalMessageSem,100);
	}
	OpcUa_Semaphore_Post(pClientApplication->m_InternalMessageSem,1);
	OpcUa_Trace(OPCUA_TRACE_CLIENT_LEVEL_ERROR,"InternalMessageThread Terminated\n");
}
void CClientApplication::DeleteAllSessions()
{
	OpcUa_Mutex_Lock(m_hSessionsMutex);
	CSessionClientList::iterator it=m_pSessionClientList->begin();
	while (it != m_pSessionClientList->end())
	{
		CSessionClient* pSessionClient = *it;
		delete pSessionClient;
		it++;
	}
	m_pSessionClientList->clear();
	OpcUa_Mutex_Unlock(m_hSessionsMutex);
}
void CClientApplication::SetLogPathName(OpcUa_String* aLogPathName)
{
	if (m_LogPathName)
	{
		OpcUa_String_Clear(m_LogPathName);
		OpcUa_Free(m_LogPathName);
	}
	m_LogPathName=(OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(m_LogPathName);
	OpcUa_String_CopyTo(aLogPathName,m_LogPathName);
}
void CClientApplication::SetLogFileName(OpcUa_String* aLogFileName)
{
	if (m_LogFileName)
	{
		OpcUa_String_Clear(m_LogFileName);
		OpcUa_Free(m_LogFileName);
	}
	m_LogFileName=(OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(m_LogFileName);
	OpcUa_String_CopyTo(aLogFileName,m_LogFileName);
}
void CClientApplication::SetConfigPathName(OpcUa_String* aConfigPathName)
{
	if (m_ConfigPathName)
	{
		OpcUa_String_Clear(m_ConfigPathName);
		OpcUa_Free(m_ConfigPathName);
	}
	m_ConfigPathName = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(m_ConfigPathName);
	OpcUa_String_CopyTo(aConfigPathName, m_ConfigPathName);
}
void CClientApplication::SetConfigFileName(OpcUa_String* aConfigFileName)
{
	if (m_ConfigFileName)
	{
		OpcUa_String_Clear(m_ConfigFileName);
		OpcUa_Free(m_ConfigFileName);
	}
	m_ConfigFileName = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(m_ConfigFileName);
	OpcUa_String_CopyTo(aConfigFileName, m_LogFileName);
}
OpcUa_StatusCode CClientApplication::GetSession(OpcUa_Handle hSession, CSessionClient** pSession)
{
	OpcUa_StatusCode uStatus = OpcUa_BadInvalidArgument;
	OpcUa_Mutex_Lock(m_hSessionsMutex);
	for (OpcUa_UInt32 ii = 0; ii<m_pSessionClientList->size(); ii++)
	{
		CSessionClient* pSessionClient = m_pSessionClientList->at(ii);
		if (pSessionClient == (CSessionClient*)hSession)
		{
			*pSession = pSessionClient;
			uStatus = OpcUa_Good;
			break;
		}
	}
	OpcUa_Mutex_Unlock(m_hSessionsMutex);
	return uStatus;
}

void CClientApplication::AddSessionClient(CSessionClient* pSession)
{
	OpcUa_Mutex_Lock(m_hSessionsMutex);
	m_pSessionClientList->push_back(pSession);
	OpcUa_Mutex_Unlock(m_hSessionsMutex);
}
OpcUa_StatusCode CClientApplication::RemoveSessionClient(CSessionClient* pSession)
{
	OpcUa_StatusCode uStatus = OpcUa_BadInvalidArgument;
	if (pSession)
	{
		OpcUa_Mutex_Lock(m_hSessionsMutex);
		for (CSessionClientList::iterator it = m_pSessionClientList->begin(); it != m_pSessionClientList->end(); it++)
		{
			if (*it == pSession)
			{
				// Publishing thread related
				pSession->StopPublishingThread();
				pSession->StopWatchingThread();
				// Check that no pending publish for this session are in the loop. Note: We will use the semaphore to wait m_InternalMessageSem
				OpcUa_UInt16 iCpt = 0;
				while ((pSession->GetPendingPublish() > 0) && (iCpt++<10))
					OpcUa_Semaphore_TimedWait(m_InternalMessageSem, (OpcUa_UInt32)50);
				uStatus = OpcUa_Good;

				delete pSession;
				m_pSessionClientList->erase(it);
				uStatus = OpcUa_Good;
				pSession = OpcUa_Null;
				break;
			}
		}
		OpcUa_Mutex_Unlock(m_hSessionsMutex);
	}
	return uStatus;
}

void CClientApplication::RemoveAllSessionClient()
{
	OpcUa_Mutex_Lock(m_hSessionsMutex);
	for (CSessionClientList::iterator it = m_pSessionClientList->begin(); it != m_pSessionClientList->end(); it++)
	{
		CSessionClient* pSession = *it;
		delete pSession;
	}
	m_pSessionClientList->clear();
	OpcUa_Mutex_Unlock(m_hSessionsMutex);
}

OpcUa_StatusCode CClientApplication::GetSubscription(OpcUa_Handle hSubscription, CSubscriptionClient** pSubscription)
{
	OpcUa_StatusCode uStatus = OpcUa_BadInvalidArgument;
	for (OpcUa_UInt32 ii = 0; ii<m_pSessionClientList->size(); ii++)
	{
		CSessionClient* pSessionClient = m_pSessionClientList->at(ii);
		uStatus = pSessionClient->GetSubscription(hSubscription, pSubscription);
		if (uStatus == OpcUa_Good)
		{
			uStatus = OpcUa_Good;
			break;
		}
	}
	return uStatus;
}

OpcUa_StatusCode CClientApplication::LoadUaClientConfiguration(char* path, char* fileName)
{
	XML_Parser pParser;
	FILE* parseFile;
	OpcUa_StatusCode uStatus = OpcUa_Bad;

	if (!path)
		uStatus = OpcUa_BadInvalidArgument;
	else
	{
		if (!fileName)
			uStatus = OpcUa_BadInvalidArgument;
		else
		{
			uStatus = xml4CE_SAXOpenParser(path, fileName, &parseFile, &pParser);
			if (uStatus == OpcUa_Good)
			{
				uStatus = xml4CE_SAXSetElementHandler(&pParser, xmlClientConfigStartElementHandler, xmlClientConfigEndElementHandler);
				if (uStatus == OpcUa_Good)
				{
					HANDLER_CLIENTCONFIG_DATA xmlClientConfigDataHandler;
					ZeroMemory(&xmlClientConfigDataHandler, sizeof(HANDLER_CLIENTCONFIG_DATA));
					xmlClientConfigDataHandler.XML_Parser = pParser;
					xmlClientConfigDataHandler.pClientApplication = this;
					xmlClientConfigDataHandler.userData = OpcUa_Null;
					xmlClientConfigDataHandler.uStatus = OpcUa_Good;
					xml4CE_SAXSetUserData(&pParser, (void*)&xmlClientConfigDataHandler);
					uStatus = xml4CE_SAXParseBegin(parseFile, &pParser);
					if (uStatus == OpcUa_Good)
						OpcUa_Trace(OPCUA_TRACE_CLIENT_LEVEL_ERROR, "Your XML configuration file : %s%s has been parsed\n", path, fileName);
					if (xmlClientConfigDataHandler.uStatus != OpcUa_Good)
						uStatus = xmlClientConfigDataHandler.uStatus;
					if (xmlClientConfigDataHandler.szEndpointUrl)						
						OpcUa_Free(xmlClientConfigDataHandler.szEndpointUrl);
					if (xmlClientConfigDataHandler.szSecurityPolicy)						
						OpcUa_Free(xmlClientConfigDataHandler.szSecurityPolicy);
					if (xmlClientConfigDataHandler.szSessionName)						
						OpcUa_Free(xmlClientConfigDataHandler.szSessionName);
					if (xmlClientConfigDataHandler.szSubscriptionName)						
						OpcUa_Free(xmlClientConfigDataHandler.szSubscriptionName);
					xmlClientConfigDataHandler.pClientApplication = OpcUa_Null;
				}
				else
					uStatus = OpcUa_BadBoundNotSupported;
				// will close parseFile
				Xml4CE_SAXCloseParser(&pParser);
				fclose(parseFile);
			}
		}
	}
	return uStatus;
}


OpcUa_StatusCode CClientApplication::SaveUaClientConfiguration(char* path, char* fileName)
{
	OpcUa_StatusCode uStatus = OpcUa_BadInvalidArgument;
	XML_Output xmlOutput;
	uStatus = xml4CE_SAXCreateOutput(path, fileName, &xmlOutput);
	if (uStatus == OpcUa_Good)
	{
		xml4CE_SAXCommentWrite(&xmlOutput, "OpenOpcUa client configuration");
		char* ClientConfig; // XML element markup
		char* SessionClient; // XML element markup
		char* Subscription; // XML element markup
		char* MonitoredItem; // XML element markup


		//// Create the ClientConfig XML Element
		ClientConfig = (char*)OpcUa_Alloc(strlen("ClientConfig") + 1);
		ZeroMemory(ClientConfig, strlen("ClientConfig") + 1);
		OpcUa_MemCpy(ClientConfig, strlen("ClientConfig"), (void*)"ClientConfig", strlen("ClientConfig"));
		// Create the SessionClient Config XML Element
		SessionClient = (char*)OpcUa_Alloc(strlen("SessionClient") + 1);
		ZeroMemory(SessionClient, strlen("Session") + 1);
		OpcUa_MemCpy(SessionClient, strlen("Session"), (void*)"Session", strlen("Session"));
		// Create the Subscription Config XML Element
		Subscription = (char*)OpcUa_Alloc(strlen("Subscription") + 1);
		ZeroMemory(Subscription, strlen("Subscription") + 1);
		OpcUa_MemCpy(Subscription, strlen("Subscription"), (void*)"Subscription", strlen("Subscription"));
		// Create the MonitoredItem Config XML Element
		MonitoredItem = (char*)OpcUa_Alloc(strlen("MonitoredItem") + 1);
		ZeroMemory(MonitoredItem, strlen("MonitoredItem") + 1);
		OpcUa_MemCpy(MonitoredItem, strlen("MonitoredItem"), (void*)"MonitoredItem", strlen("MonitoredItem"));

		xml4CE_SAXStartElementWrite(&xmlOutput, ClientConfig, OpcUa_Null);
		// Now let's write the Session part
		for (OpcUa_UInt32 i = 0; i < m_pSessionClientList->size(); i++)
		{
			CSessionClient* pSessionClient = m_pSessionClientList->at(i);
			if (pSessionClient)
			{
				// Get the session as XML attributes
				OpcUa_CharA** attsSessionAtts = OpcUa_Null;
				vector<OpcUa_String*> szAttributes;
				pSessionClient->GetXmlAttributes(&szAttributes);
				OpcUa_Int32 iLen = szAttributes.size();
				if (iLen > 0)
				{
					attsSessionAtts = (OpcUa_CharA**)OpcUa_Alloc((iLen + 1)*sizeof(OpcUa_UInt32));
					ZeroMemory(attsSessionAtts, (iLen + 1)* sizeof(OpcUa_UInt32));
					OpcUa_UInt32 i = 0;
					for (i = 0; i < szAttributes.size(); i++)
					{
						OpcUa_String* pString = szAttributes.at(i);
						if (pString)
						{
							attsSessionAtts[i] = (OpcUa_CharA*)OpcUa_Alloc(OpcUa_String_StrLen(pString) + 1);
							ZeroMemory(attsSessionAtts[i], OpcUa_String_StrLen(pString) + 1);
							OpcUa_MemCpy(attsSessionAtts[i], OpcUa_String_StrLen(pString), OpcUa_String_GetRawString(pString), OpcUa_String_StrLen(pString));
						}
					}
					// Release ressource
					for (i = 0; i < szAttributes.size(); i++)
					{
						OpcUa_String* pString = szAttributes.at(i);
						if (pString)
						{
							OpcUa_String_Clear(pString);
							OpcUa_Free(pString);
						}
					}
					szAttributes.clear();
				}
				// Write to the xml file
				xml4CE_SAXStartElementWrite(&xmlOutput, SessionClient, attsSessionAtts);

				for (OpcUa_UInt32 ii = 0; ii < pSessionClient->m_SubscriptionList.size(); ii++)
				{
					CSubscriptionClient* pSubscription = pSessionClient->m_SubscriptionList.at(ii);
					if (pSubscription)
					{
						OpcUa_CharA** attsSubscription = OpcUa_Null;
						// Now let's write the Subscription part
						// Get the Subscription as XML attributes
						pSubscription->GetXmlAttributes(&szAttributes);
						OpcUa_Int32 iLen = szAttributes.size();
						if (iLen > 0)
						{
							attsSubscription = (OpcUa_CharA**)OpcUa_Alloc((iLen + 1)*sizeof(OpcUa_UInt32));
							ZeroMemory(attsSubscription, (iLen + 1)* sizeof(OpcUa_UInt32));
							OpcUa_UInt32 i = 0;
							for (i = 0; i < szAttributes.size(); i++)
							{
								OpcUa_String* pString = szAttributes.at(i);
								if (pString)
								{
									attsSubscription[i] = (OpcUa_CharA*)OpcUa_Alloc(OpcUa_String_StrLen(pString) + 1);
									ZeroMemory(attsSubscription[i], OpcUa_String_StrLen(pString) + 1);
									OpcUa_MemCpy(attsSubscription[i], OpcUa_String_StrLen(pString), OpcUa_String_GetRawString(pString), OpcUa_String_StrLen(pString));
								}
							}
							// Release ressource
							for (i = 0; i < szAttributes.size(); i++)
							{
								OpcUa_String* pString = szAttributes.at(i);
								if (pString)
								{
									OpcUa_String_Clear(pString);
									OpcUa_Free(pString);
								}
							}
							szAttributes.clear();
						}
						xml4CE_SAXStartElementWrite(&xmlOutput, Subscription, attsSubscription);
						if (attsSubscription)
						{
							for (i = 0; i < szAttributes.size(); i++)
							{
								if (attsSubscription[i])
									OpcUa_Free(attsSubscription[i]);
							}
							OpcUa_Free(attsSubscription);
						}
						// Now let's write the MonitoredItem part
						CMonitoredItemClientList* pMonitoredItemList = pSubscription->GetMonitoredItemList();
						for (OpcUa_UInt32 iii = 0; iii < pMonitoredItemList->size(); iii++)
						{
							CMonitoredItemClient* pMonitoredItem = pMonitoredItemList->at(iii);
							OpcUa_CharA** attsMonitoredItem = OpcUa_Null;
							pMonitoredItem->GetXmlAttributes(&szAttributes);
							OpcUa_Int32 iLen = szAttributes.size();
							if (iLen > 0)
							{
								attsMonitoredItem = (OpcUa_CharA**)OpcUa_Alloc((iLen + 1)*sizeof(OpcUa_UInt32));
								ZeroMemory(attsMonitoredItem, (iLen + 1)* sizeof(OpcUa_UInt32));
								OpcUa_UInt32 i = 0;
								for (i = 0; i < szAttributes.size(); i++)
								{
									OpcUa_String* pString = szAttributes.at(i);
									if (pString)
									{
										attsMonitoredItem[i] = (OpcUa_CharA*)OpcUa_Alloc(OpcUa_String_StrLen(pString) + 1);
										ZeroMemory(attsMonitoredItem[i], OpcUa_String_StrLen(pString) + 1);
										OpcUa_MemCpy(attsMonitoredItem[i], OpcUa_String_StrLen(pString), OpcUa_String_GetRawString(pString), OpcUa_String_StrLen(pString));
									}
								}
								// Release ressource
								for (i = 0; i < szAttributes.size(); i++)
								{
									OpcUa_String* pString = szAttributes.at(i);
									if (pString)
									{
										OpcUa_String_Clear(pString);
										OpcUa_Free(pString);
									}
								}
								szAttributes.clear();
							}
							xml4CE_SAXStartElementWrite(&xmlOutput, MonitoredItem, attsMonitoredItem);
							if (attsMonitoredItem)
							{
								for (i = 0; i < szAttributes.size(); i++)
									OpcUa_Free(attsMonitoredItem[i]);
								OpcUa_Free(attsMonitoredItem);
							}
							xml4CE_SAXEndElementWrite(&xmlOutput, MonitoredItem);
						}
						xml4CE_SAXEndElementWrite(&xmlOutput, Subscription);
					}
				}
				xml4CE_SAXEndElementWrite(&xmlOutput, SessionClient);
			}
		}
		// End the client config XML Element
		xml4CE_SAXEndElementWrite(&xmlOutput, ClientConfig);
		uStatus = xml4CE_SAXCloseOutput(&xmlOutput);
		OpcUa_Free(ClientConfig);
		OpcUa_Free(SessionClient);
		OpcUa_Free(Subscription);
		OpcUa_Free(MonitoredItem);
	}
	return uStatus;
}
OpcUa_StatusCode CClientApplication::GetSessions(OpcUa_UInt32* uiNoOfSessions, OpcUa_Handle** hSessions)
{
	OpcUa_StatusCode uStatus = OpcUa_BadInvalidArgument;
	OpcUa_Mutex_Lock(m_hSessionsMutex);
	*uiNoOfSessions = m_pSessionClientList->size();
	if (*uiNoOfSessions)
	{
		*hSessions = (OpcUa_Handle*)OpcUa_Alloc(m_pSessionClientList->size()*sizeof(OpcUa_Handle));
		ZeroMemory(*hSessions, m_pSessionClientList->size()*sizeof(OpcUa_Handle));
		for (OpcUa_UInt32 ii = 0; ii < m_pSessionClientList->size(); ii++)
		{
			CSessionClient* pSessionClient = m_pSessionClientList->at(ii);
			(*hSessions)[ii] = (OpcUa_Handle)pSessionClient;
		}
		uStatus = OpcUa_Good;
	}
	else
		uStatus = OpcUa_BadNothingToDo;
	OpcUa_Mutex_Unlock(m_hSessionsMutex);
	return uStatus;
}

OpcUa_StatusCode CClientApplication::GetSubscriptionOfMonitoredItem(OpcUa_Handle hMonitoredItem, CSubscriptionClient** pSubscriptionClient)
{

	OpcUa_StatusCode uStatus = OpcUa_BadInvalidArgument;
	OpcUa_Mutex_Lock(m_hSessionsMutex);
	for (OpcUa_UInt32 ii = 0; ii < m_pSessionClientList->size(); ii++)
	{
		CSessionClient* pInternalSessionClient = m_pSessionClientList->at(ii);
		if (pInternalSessionClient)
		{
			uStatus = pInternalSessionClient->GetSubscriptionOfMonitoredItem(hMonitoredItem, pSubscriptionClient);
			if (uStatus == OpcUa_Good)
				break;
		}
	}
	OpcUa_Mutex_Unlock(m_hSessionsMutex);
	return uStatus;
}