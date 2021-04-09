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
#include <stdexcept>
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
#include "LuaVirtualMachine.h"
#include "LuaScript.h"
#include "OpenOpcUaScript.h"
using namespace UAScript;

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
#include "SecureChannel.h"
#include "VfiDataValue.h"
#include "UAHistorianVariable.h"
#include "HaEngine.h"
using namespace UAHistoricalAccess;

#include "ServerApplication.h"
#include "Utils.h"
#include "Channel.h"
#include "StackCallbacks.h"
#include "ServiceModule.h" // To support the server as a Windows Service
#include <vector>
#ifdef WIN32
#include "resource.h"
#endif

using namespace std;
using namespace OpenOpcUa;
using namespace UACoreServer;
//BOOL	CServerApplication::m_bRunLDSRegistrationThread;
//OpcUa_Semaphore	CServerApplication::m_hStopLDSRegistrationThread;
extern CServerApplication* g_pTheApplication;
OpcUa_Double CServerApplication::m_dblMiniSamplingInterval;
OpcUa_Double CServerApplication::m_dblMaxSamplingInterval;
OpcUa_Boolean CServerApplication::m_bIsExist;

// Cette fonction renvoie le nodeId de l'object en cours de parsing dans le HANDLER_DATA
OpcUa_StatusCode GetNodeIdForCurrentParsedObject(HANDLER_DATA* pHandleData, OpcUa_NodeId** pNodeId)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	if (pNodeId)
	{
		if (*pNodeId == OpcUa_Null)
		{
			*pNodeId = (OpcUa_NodeId*)OpcUa_Alloc(sizeof(OpcUa_NodeId));
			OpcUa_NodeId_Initialize(*pNodeId);
			if (pHandleData)
			{
				if (pHandleData->pUADataType)
				{
					OpcUa_NodeId* pTmpNodeId = pHandleData->pUADataType->GetNodeId();
					OpcUa_NodeId_CopyTo(pTmpNodeId, *pNodeId);
				}
				else
				{
					if (pHandleData->pUAObject)
					{
						OpcUa_NodeId* pTmpNodeId = pHandleData->pUAObject->GetNodeId();
						OpcUa_NodeId_CopyTo(pTmpNodeId, *pNodeId);
					}
					else
					{
						if (pHandleData->pUAObjectType)
						{
							OpcUa_NodeId* pTmpNodeId = pHandleData->pUAObjectType->GetNodeId();
							OpcUa_NodeId_CopyTo(pTmpNodeId, *pNodeId);
						}
						else
						{
							if (pHandleData->pUAReferenceType)
							{
								OpcUa_NodeId* pTmpNodeId = pHandleData->pUAReferenceType->GetNodeId();
								OpcUa_NodeId_CopyTo(pTmpNodeId, *pNodeId);
							}
							else
							{
								if (pHandleData->pUAVariable)
								{
									OpcUa_NodeId* pTmpNodeId = pHandleData->pUAVariable->GetNodeId();
									OpcUa_NodeId_CopyTo(pTmpNodeId, *pNodeId);
								}
								else
								{
									if (pHandleData->pUAVariableType)
									{
										OpcUa_NodeId* pTmpNodeId = pHandleData->pUAVariableType->GetNodeId();
										OpcUa_NodeId_CopyTo(pTmpNodeId, *pNodeId);
									}
									else
									{
										if (pHandleData->pView)
										{
											OpcUa_NodeId* pTmpNodeId = pHandleData->pView->GetNodeId();
											OpcUa_NodeId_CopyTo(pTmpNodeId, *pNodeId);
										}
										else
										{
											if (pHandleData->pMethod)
											{
												OpcUa_NodeId* pTmpNodeId = pHandleData->pMethod->GetNodeId();
												OpcUa_NodeId_CopyTo(pTmpNodeId, *pNodeId);
											}
											else
											{
												OpcUa_Free(*pNodeId);
												*pNodeId = OpcUa_Null;
												uStatus = OpcUa_BadNotFound;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
		else
			uStatus = OpcUa_BadInvalidArgument;
	}
	return uStatus;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Fonction de notification appelé par les VPI
// il s'agit d'une fonction unsollicited qui permet au VPI de transferer des données pour les protoles Maitre/Maitre ou client/serveur
// l'utilisation de cette fonction implique que le Vpi Implémente la fonction VpiSetNotifyCallback
// Cette fonction permet au serveur d'indiquer au le client pourra se brancher
// elle utilise la signature suivante :
// OpcUa_Vpi_StatusCode VpiSetNotifyCallback(OpcUa_Handle hVpi,PFUNCNOTIFYCALLBACK lpCallbackNotify)
// Contactez Michel Condemine pour plus d'explication
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef WIN32
OpcUa_Vpi_StatusCode _stdcall VpiNotifyCallback(OpcUa_UInt32 uiNoOfNotifiedObject, OpcUa_NodeId* Id, OpcUa_DataValue* pDataValue)
#endif
#ifdef _GNUC_
OpcUa_Vpi_StatusCode  VpiNotifyCallback(OpcUa_UInt32 uiNoOfNotifiedObject, OpcUa_NodeId* Id, OpcUa_DataValue* pDataValue)
#endif
{
	OpcUa_Vpi_StatusCode uStatus=0;
	CUAInformationModel* pInformationModel=CServerApplication::m_pTheAddressSpace;
	CUAVariable* pUAVariable=OpcUa_Null;
	// Request from the Vpi.
	OpcUa_Boolean bResquestFromVpi = OpcUa_False;

	for (OpcUa_UInt32 ii=0;ii<uiNoOfNotifiedObject;ii++)
	{
		OpcUa_Mutex_Lock(pInformationModel->GetServerCacheMutex());
		OpcUa_StatusCode hr = pInformationModel->GetNodeIdFromVariableList(Id[ii], &pUAVariable);
		if (hr==OpcUa_Good)
		{
			if (pDataValue[ii].Value.Datatype==0)
				bResquestFromVpi = OpcUa_True;
			// The Vpi wants the value of the requested NodeId
			if (bResquestFromVpi)
			{
				OpcUa_DataValue_Initialize(&pDataValue[ii]);
				CDataValue* pUADataValue = pUAVariable->GetValue();
				OpcUa_DataValue* pInternalDataValue = pUADataValue->GetInternalDataValue();
				OpcUa_DataValue_CopyTo(pInternalDataValue, &pDataValue[ii]);
			}
			else
			{
				if (pDataValue[ii].StatusCode == 0x83050000) // OpcUa_Vpi_WarmStartNeed
				{
					// Call the WarmStart
					CVpiDevice* pDevice = OpcUa_Null;
					CVpiTag* pSignal = (CVpiTag*)(pUAVariable->GetPData());
					if (pSignal)
					{
						pDevice = pSignal->GetDevice();
						if (pDevice)
						{
							pDevice->SetInternalStatus(0x83050000); 
						}
						else
							OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "WarmStart requested but internal corruption detected on Device\n");
					}
					else
						OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "WarmStart requested but internal corruption detected Tag\n");
				}

				// Qualité
				OpcUa_StatusCode uStatusCode = pDataValue[ii].StatusCode;
				// if the internal CUAVariable::m_pDataValue is null we create an instance
				CDataValue* pUADataValue = pUAVariable->GetValue();
				if (!pUADataValue)
				{
					// initialisation du CDataValue
					pUADataValue = new CDataValue();
					pUADataValue->Initialize(pUAVariable->GetBuiltInType(), pUAVariable->GetValueRank(), pUAVariable->GetArrayDimensions());
					pUAVariable->SetValue(pUADataValue);
				}

				pUADataValue->SetStatusCode(uStatusCode);
				// Special case if the UAVariableType is a string and the VpiType a array of byte
				if ((pUAVariable->GetBuiltInType() == OpcUaType_String)
					&& (pDataValue[ii].Value.Datatype == OpcUaType_Byte)
					&& (pDataValue[ii].Value.ArrayType == OpcUa_VariantArrayType_Array))
				{
					OpcUa_Variant aVariant = pUADataValue->GetValue();
					OpcUa_String_Initialize(&aVariant.Value.String);
					OpcUa_CharA* szData = (OpcUa_CharA*)OpcUa_Alloc(pDataValue[ii].Value.Value.Array.Length+1);
					ZeroMemory(szData, pDataValue[ii].Value.Value.Array.Length+1);
					OpcUa_MemCpy(szData, 
						pDataValue[ii].Value.Value.Array.Length, 
						(OpcUa_CharA*)(pDataValue[ii].Value.Value.Array.Value.ByteArray), 
						pDataValue[ii].Value.Value.Array.Length);
					OpcUa_String_AttachCopy(&aVariant.Value.String, szData);
					pUADataValue->SetValue(aVariant);
					OpcUa_Free(szData);
				}
				else
				// End Special case
					pUADataValue->SetValue(pDataValue[ii].Value);

				if ((pDataValue[ii].SourceTimestamp.dwLowDateTime == 0) && (pDataValue[ii].SourceTimestamp.dwHighDateTime == 0))
				{
					// Create timestamp from host timestamp
					pUADataValue->SetServerTimestamp(OpcUa_DateTime_UtcNow());
				}
				else
				{
					pUADataValue->SetServerTimestamp(OpcUa_DateTime_UtcNow());
					// Use timestamp sent by the Vpi
					OpcUa_DateTime aDateTime = pDataValue[ii].SourceTimestamp;
					pUADataValue->SetSourceTimestamp(aDateTime);
				}
			}

		}
		else
			uStatus=OpcUa_BadNodeIdInvalid;
		OpcUa_Mutex_Unlock(pInformationModel->GetServerCacheMutex());
	}
	
	return uStatus;
}

// Parsing du fichier XML
///////////////////////////////////////////////////////////////////////////////////////
// structure userData associé au parsing du fichier xml de configuration
typedef struct handler_Config_Data 
{
	XML_ParserStruct*		XML_Parser;    
	void*					userData; 
	CServerApplication*		pServerApplication;
} HANDLER_CONFIG_DATA;
/// <summary>
/// Gets the cleanup XML elt.
/// </summary>
/// <param name="name">The name.</param>
/// <param name="cleanedName">Name of the cleaned.</param>
/// <param name="xmlNamespace">The XML namespace.</param>
void GetCleanupXmlElt(const char* name, char** cleanedName, char** xmlNamespace)
{
	OpcUa_Int32 iSOHpos = -1;
	OpcUa_UInt32 u = 0;
	
	while (strlen(name) != u)
	{
		if (name[u] == 1) 	// SOH
			iSOHpos = u;
		u++;

	}
	if (iSOHpos != -1)
	{
		(*cleanedName) = (char*)malloc(strlen(name) - iSOHpos);
		if ((*cleanedName))
		{
			ZeroMemory((*cleanedName), strlen(name) - iSOHpos);
			memcpy((*cleanedName), &name[iSOHpos + 1], strlen(name) - iSOHpos);
		}
		(*xmlNamespace) = (char*)malloc(iSOHpos+1);
		if ((*xmlNamespace))
		{
			ZeroMemory((*xmlNamespace), iSOHpos + 1);
			memcpy((*xmlNamespace), &name[0], iSOHpos);
		}
	}
	else
	{
		(*cleanedName) = (char*)malloc(strlen(name));
		if ((*cleanedName))
		{
			ZeroMemory((*cleanedName), strlen(name));
			memcpy((*cleanedName), &name[iSOHpos + 1], strlen(name));
		}
	}
	return ;
}
///////////////////////////////////////////////////////////////////////////////////////
// Fonction délegué pour la prise en charge du chargement des fichiers de configuration
void xmlConfigStartElementHandler(
	void *userData,     /* user defined data */
	const char *name,   /* Element name */
	const char **atts)  /* Element Attribute list, provided in name value pairs */
						/* i.e. atts[0] contains name, atts[1] contains value for */
						/* atts[0], and so on...*/
{
	HANDLER_CONFIG_DATA* pMyData=(HANDLER_CONFIG_DATA*)userData; 
	char* cleanedName = OpcUa_Null;
	char* xmlNamespace = OpcUa_Null;
	GetCleanupXmlElt(name, &cleanedName, &xmlNamespace);

	if (OpcUa_StrCmpA(cleanedName, "ServerConfig") == 0)
	{
		int ii=0;
		while (atts[ii])
		{
			if (OpcUa_StrCmpA(atts[ii],"ServerName")==0)
			{
				OpcUa_LocalizedText aApplicationName;
				OpcUa_LocalizedText_Initialize(&aApplicationName);
				OpcUa_String_AttachCopy(&(aApplicationName.Text),OpcUa_StringA(atts[ii+1]));	
				OpcUa_String_AttachCopy(&(aApplicationName.Locale),"en-EN");				
				pMyData->pServerApplication->SetApplicationName(&aApplicationName);
				OpcUa_LocalizedText_Clear(&aApplicationName);
			}
			if (OpcUa_StrCmpA(atts[ii],"SecurityNone")==0)
			{				
				if ((OpcUa_StrCmpA(atts[ii + 1], "True") == 0)
					|| (OpcUa_StrCmpA(atts[ii + 1], "true") == 0)
					|| (OpcUa_StrCmpA(atts[ii + 1], "TRUE") == 0))
					pMyData->pServerApplication->SecurityNoneAccepted(OpcUa_True);
				else
					pMyData->pServerApplication->SecurityNoneAccepted(OpcUa_False);
			}
			if (OpcUa_StrCmpA(atts[ii], "AppId") == 0)
			{
				OpcUa_Guid aApplicationGuid;
				OpcUa_Guid_Initialize(&aApplicationGuid);
				OpcUa_String szGuid;
				OpcUa_String_Initialize(&szGuid);
				OpcUa_String_AttachCopy(&(szGuid),OpcUa_StringA(atts[ii+1]));
				OpcUa_CharA* pszGuid = OpcUa_String_GetRawString(&szGuid);
				OpcUa_Guid_FromString(pszGuid, &aApplicationGuid);
#ifdef WIN32
				pMyData->pServerApplication->m_ServiceModule.SetApplicationId(&aApplicationGuid);
#endif
				OpcUa_String_Clear(&szGuid);
				OpcUa_Guid_Clear(&aApplicationGuid);
			}
			ii+=2;
		}
	}
	if (OpcUa_StrCmpA(cleanedName, "LDSRegistration") == 0)
	{
		int ii=0;
		while (atts[ii])
		{
			if (OpcUa_StrCmpA(atts[ii], "Active") == 0)
			{
				if ( (OpcUa_StrCmpA(atts[ii + 1], "True") == 0) 
					|| (OpcUa_StrCmpA(atts[ii + 1], "true") == 0)
					|| (OpcUa_StrCmpA(atts[ii + 1], "TRUE") == 0) )
					pMyData->pServerApplication->LDSRegistrationActive(OpcUa_True);
				else
					pMyData->pServerApplication->LDSRegistrationActive(OpcUa_False);
			}
			if (OpcUa_StrCmpA(atts[ii], "Interval") == 0)
			{
				OpcUa_UInt32 uiVal = atoi(atts[ii + 1]); // The extracted value is in second so let change it in milli-second
				pMyData->pServerApplication->SetLDSRegistrationInterval(uiVal*1000);
			}
			ii+=2;
		}
	}
	if (OpcUa_StrCmpA(cleanedName, "Trace") == 0)
	{
		int ii=0;
		while (atts[ii])
		{
			if (OpcUa_StrCmpA(atts[ii],"Output")==0)
			{
				OpcUa_UInt32 iTraceOutput=OPCUA_TRACE_OUTPUT_NONE;
				if (OpcUa_StrCmpA(atts[ii+1],"CONSOLE")==0)
					iTraceOutput=OPCUA_TRACE_OUTPUT_CONSOLE;
				else
				{
					if (OpcUa_StrCmpA(atts[ii+1],"FILE")==0)
						iTraceOutput=OPCUA_TRACE_OUTPUT_FILE;
				}
				pMyData->pServerApplication->SetTraceOutput(iTraceOutput); // sauvegarde la sortie selectionnée
				OpcUa_Trace_SetTraceOutput(iTraceOutput);
			}
			if (OpcUa_StrCmpA(atts[ii],"Level")==0)
			{
				OpcUa_UInt32 iTraceLevel=OPCUA_TRACE_OUTPUT_LEVEL_NONE;
				if (OpcUa_StrCmpA(atts[ii+1],"STACK_DEBUG")==0)	// contient le niveau de trace selectionné dans le fichier de configuration
					iTraceLevel = OPCUA_TRACE_OUTPUT_STACK_DEBUG;
				else
				{
					if (OpcUa_StrCmpA(atts[ii+1],"STACK_ERROR")==0)
						iTraceLevel = OPCUA_TRACE_OUTPUT_STACK_ERROR;
					else
					{
						if (OpcUa_StrCmpA(atts[ii+1],"STACK_WARNING")==0)
							iTraceLevel = OPCUA_TRACE_OUTPUT_STACK_WARNING;
						else
						{
							if (OpcUa_StrCmpA(atts[ii+1],"STACK_INFO")==0)
								iTraceLevel = OPCUA_TRACE_OUTPUT_STACK_INFO;
							else
							{
								if (OpcUa_StrCmpA(atts[ii + 1], "SERVER_DEBUG") == 0)	// contient le niveau de trace selectionné dans le fichier de configuration
									iTraceLevel = OPCUA_TRACE_SERVER_LEVEL_DEBUG; //   OPCUA_TRACE_OUTPUT_SERVER_DEBUG
								else
								{
									if (OpcUa_StrCmpA(atts[ii + 1], "SERVER_ERROR") == 0)
										iTraceLevel = OPCUA_TRACE_OUTPUT_SERVER_ERROR;
									else
									{
										if (OpcUa_StrCmpA(atts[ii + 1], "SERVER_WARNING") == 0)
											iTraceLevel = OPCUA_TRACE_OUTPUT_SERVER_WARNING;
										else
										{
											if (OpcUa_StrCmpA(atts[ii + 1], "SERVER_INFO") == 0)
												iTraceLevel = OPCUA_TRACE_OUTPUT_SERVER_INFO;
											else
											{
												if (OpcUa_StrCmpA(atts[ii + 1], "EXTRA_DEBUG") == 0)	// contient le niveau de trace selectionné dans le fichier de configuration
													iTraceLevel = OPCUA_TRACE_OUTPUT_EXTRA_DEBUG;
												else
												{
													if (OpcUa_StrCmpA(atts[ii + 1], "EXTRA_ERROR") == 0)
														iTraceLevel = OPCUA_TRACE_OUTPUT_EXTRA_ERROR;
													else
													{
														if (OpcUa_StrCmpA(atts[ii + 1], "EXTRA_WARNING") == 0)
															iTraceLevel = OPCUA_TRACE_OUTPUT_EXTRA_WARNING;
														else
														{
															if (OpcUa_StrCmpA(atts[ii + 1], "EXTRA_INFO") == 0)
																iTraceLevel = OPCUA_TRACE_OUTPUT_EXTRA_INFO;
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
				pMyData->pServerApplication->SetTraceLevel(iTraceLevel); // sauvegarde le niveau de trace selectionnée
				OpcUa_Trace_SetTraceLevel(iTraceLevel);
			}
			ii+=2;
		}
	}
	if (OpcUa_StrCmpA(cleanedName, "FileNodeSet") == 0)
	{
		int ii=0;
		while (atts[ii])
		{
			if (OpcUa_StrCmpA(atts[ii],"FullFileName")==0)
			{
				OpcUa_String* pString=(OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
				OpcUa_String_Initialize(pString);
				OpcUa_String_AttachCopy(pString,OpcUa_StringA(atts[ii+1]));
				pMyData->pServerApplication->m_FilesNodeSet.push_back(pString);
			}
			ii+=2;
		}
	}
	if (OpcUa_StrCmpA(cleanedName, "FileSimulation") == 0)
	{
		int ii=0;
		while (atts[ii])
		{
			if (OpcUa_StrCmpA(atts[ii],"FullFileName")==0)
			{
				OpcUa_String* pString = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
				OpcUa_String_Initialize(pString);
				OpcUa_String_AttachCopy(pString, OpcUa_StringA(atts[ii + 1]));
				pMyData->pServerApplication->m_FilesSimulation.push_back(pString);
			}
			ii+=2;
		}
	}

	if (OpcUa_StrCmpA(cleanedName, "FileSubsystem") == 0)
	{
		int ii=0;
		while (atts[ii])
		{
			if (OpcUa_StrCmpA(atts[ii],"FullFileName")==0)
			{
				OpcUa_String* pString = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
				OpcUa_String_Initialize(pString);
				OpcUa_String_AttachCopy(pString, OpcUa_StringA(atts[ii + 1]));
				pMyData->pServerApplication->m_FilesSubsystem.push_back(pString);
			}
			ii+=2;
		}
	}
	if (OpcUa_StrCmpA(cleanedName, "Binding") == 0)
	{
		CUABinding* pBinding=new CUABinding();
		int ii=0;
		while (atts[ii])
		{
			if (OpcUa_StrCmpA(atts[ii],"Protocol")==0)
			{
				OpcUa_String aString;
				OpcUa_String_Initialize(&aString);
				OpcUa_String_AttachCopy(&aString,(OpcUa_StringA)atts[ii+1]);
				pBinding->SetProtocol(aString);
			}
			if (OpcUa_StrCmpA(atts[ii],"Port")==0)
			{
				OpcUa_String aString;
				OpcUa_String_Initialize(&aString);
				OpcUa_String_AttachCopy(&aString,(OpcUa_StringA)atts[ii+1]);
				pBinding->SetPort(aString);
				OpcUa_String_Clear(&aString);
			}
			if (OpcUa_StrCmpA(atts[ii],"Encoding")==0)
			{
				if (OpcUa_StrCmpA(atts[ii+1],"Binary")==0)
					pBinding->SetEncoding(OpcUa_Endpoint_SerializerType_Binary);
				else
				{
					if (OpcUa_StrCmpA(atts[ii+1],"XML")==0)
						pBinding->SetEncoding(OpcUa_Endpoint_SerializerType_Xml);
					else
						pBinding->SetEncoding(OpcUa_Endpoint_SerializerType_Invalid);
				}
			}
			ii+=2;
		}
		pMyData->pServerApplication->AddBinding(pBinding);
	}
	if (OpcUa_StrCmpA(cleanedName, "HistoricalAccess") == 0)
	{
		int ii=0;
		while (atts[ii])
		{
			CHaEngine* pHaEngine=pMyData->pServerApplication->GetHaEngine();
			if (OpcUa_StrCmpA(atts[ii],"VfiName")==0)
			{
				OpcUa_String aString;
				OpcUa_String_Initialize(&aString);
				OpcUa_String_AttachCopy(&aString,OpcUa_StringA(atts[ii+1]));
				if (!pHaEngine)
				{
					pHaEngine=new CHaEngine();
					pMyData->pServerApplication->SetHaEngine(pHaEngine);
				}
				pHaEngine->SetLibraryName(aString);
			}
			if (OpcUa_StrCmpA(atts[ii],"ArchiveId")==0)
			{
				OpcUa_String aNodeId;
				OpcUa_String_Initialize(&aNodeId);				
				if (OpcUa_String_AttachCopy(&aNodeId,atts[ii+1])==OpcUa_Good)
					pMyData->pServerApplication->SetArchiveId(aNodeId);
				OpcUa_String_Initialize(&aNodeId);
			}
			if (OpcUa_StrCmpA(atts[ii],"ArchiveName")==0)
			{
				OpcUa_String aString;
				OpcUa_String_Initialize(&aString);
				if (OpcUa_String_AttachCopy(&aString,atts[ii+1])==OpcUa_Good)
					pMyData->pServerApplication->SetArchiveName(aString);
				OpcUa_String_Clear(&aString);
			}
			if (OpcUa_StrCmpA(atts[ii], "EngineFreq") == 0) // Archive engine minimum frequency
			{
				OpcUa_UInt32 uiVal = atoi(atts[ii + 1]);
				if (pHaEngine)
					pHaEngine->SetArchiveEngineFrequency(uiVal);
			}
			ii+=2;
		}
	}

	if (cleanedName)
		free(cleanedName);
	if (xmlNamespace)
		free(xmlNamespace);
}
void xmlConfigEndElementHandler( 
	void *userData,     /* user defined data */
	const char *name)/* Element name */
{
	OpcUa_ReferenceParameter(userData);
	if (OpcUa_StrCmpA(name,"ServerConfig")==0)
	{

	}
	if (OpcUa_StrCmpA(name,"FileNodeSet")==0)
	{
	}	
}


// Function name   : GetCurrentHandledClass
// Description     : Recupère la classe en cours de manipulation par 
//                    le parser au sein du Handler contextuel.
// Return type     : CUABase* classe en cours de manipulation, NULL si aucune n'est en cours de paramétrage
// Argument        : HANDLER_DATA* pMyData. Handler a tester

CUABase* GetCurrentHandledClass(HANDLER_DATA* pMyData)
{
	CUABase* pBase=NULL;
	if (pMyData->pUAVariable) 
		return (CUABase*)pMyData->pUAVariable;
	if (pMyData->pUAVariableType)
		return (CUABase*)pMyData->pUAVariableType;
	if (pMyData->pDataValue) 
		return (CUABase*)pMyData->pDataValue;
	if (pMyData->pMethod)
		return (CUABase*)pMyData->pMethod; 
	if (pMyData->pUADataType) 
		return (CUABase*)pMyData->pUADataType; 
	if (pMyData->pUAObject) 
		return (CUABase*)pMyData->pUAObject;  
	if (pMyData->pUAObjectType)
		return (CUABase*)pMyData->pUAObjectType;
	if (pMyData->pUAReferenceType)
		return (CUABase*)pMyData->pUAReferenceType;
	if (pMyData->pView)
		return (CUABase*)pMyData->pView;
	return pBase;
}

// Fonction speciale pour l'init des extensions object contenant des OpcUa_EnumValueType a partir du fichier XML
// quand on arrive dans cette fonction les balises element XML suivante on été lu :
// <Value> && (<ListOfExtensionObject> || <ExtensionObject>) && <TypeId> && <Body> && <EnumValueType>
// 
// Il reste a venir <Value> <DisplayName> <Locale> <Text> <Description><Locale> <Text>
OpcUa_StatusCode xmlInitEnumValueTypeExtensionObject(HANDLER_DATA* pMyData,const char* s,int len)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	char* buff=(char*)malloc(len+1);
	if (buff)
	{

		ZeroMemory(buff, len + 1);
		memcpy(buff, s, len);
		/////////////////////////////////////////////////
		//workaround
		// Si le champ commence par un espace on l'ignore
		if ((buff[0] == 0x20) || (buff[0] == 0x0a))
		{
			memset(buff, 0, len + 1);
			free(buff);
			return uStatus;
		}
		// fin workaround
		if (pMyData->bExtensionObjectEnumValueTypeValue)
		{
			OpcUa_Int32 iVal = 0;
			int iRes = sscanf_s(buff, "%i", (int*)&iVal);
			if (!iRes)
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "an incorrect pEnumValueType->Value was used\n");
			else
				pMyData->pEnumValueType->Value = iVal;
		}
		else
		{
			if (pMyData->bExtensionObjectEnumValueTypeDisplayName)
			{
				if (pMyData->bLocale)
					OpcUa_String_AttachCopy(&(pMyData->pEnumValueType->DisplayName.Locale), buff);
				if (pMyData->bText)
					OpcUa_String_AttachCopy(&(pMyData->pEnumValueType->DisplayName.Text), buff);
			}
			else
			{
				if (pMyData->bExtensionObjectEnumValueTypeDescription)
				{
					if (pMyData->bLocale)
						OpcUa_String_AttachCopy(&(pMyData->pEnumValueType->Description.Locale), buff);
					if (pMyData->bText)
						OpcUa_String_AttachCopy(&(pMyData->pEnumValueType->Description.Text), buff);
				}
			}
		}
		free(buff);
	}
	return uStatus;
}
// Fonction speciale pour l'init des extensions object contenant des OpcUa_EUInformation a partir du fichier XML
// quand on arrive dans cette fonction les balises element XML suivante ont été lu :
// <Value> && (<ListOfExtensionObject> || <ExtensionObject>) && <TypeId> && <Body> && <EUInformation>
// rappel : Les arguments sont les paramètres des méthodes OPC UA
// Il reste a venir <NamespaceUri> (String) <UnitId> (Int32)  <DisplayName> (LocalizedText) <Description> ((LocalizedText)
// Notons que <DisplayName> et  <Description> contiennent un sous element <Text>
OpcUa_StatusCode xmlInit887ExtensionObject(HANDLER_DATA* pMyData, const char* s, int len)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	char* buff = (char*)OpcUa_Alloc(len + 1);
	ZeroMemory(buff, len + 1);
	memcpy(buff, s, len);
	/////////////////////////////////////////////////
	//workaround
	// Si le champ commence par un espace on l'ignore
	if ((buff[0] == 0x20) || (buff[0] == 0x0a))
	{
		OpcUa_Free(buff);
		return uStatus;
	}
	// fin workaround
	// 
	OpcUa_Variant aVariant = pMyData->pDataValue->GetValue();
	// Let's implement 887 decoding for initialization
	OpcUa_EUInformation* pEUInformation = (OpcUa_EUInformation*)aVariant.Value.ExtensionObject->Body.EncodeableObject.Object;
	if (!pEUInformation)
	{
		pEUInformation = (OpcUa_EUInformation*)OpcUa_Alloc(sizeof(OpcUa_EUInformation));
		if (pEUInformation)
			OpcUa_EUInformation_Initialize(pEUInformation);
		else
			uStatus = OpcUa_BadOutOfMemory;
	}
	if (uStatus == OpcUa_Good)
	{
		if (OpcUa_StrCmpA(pMyData->szCurrentXmlElement, "NamespaceUri") == 0)
		{
			OpcUa_String_AttachCopy(&(pEUInformation->NamespaceUri),buff );
		}

		if (OpcUa_StrCmpA(pMyData->szCurrentXmlElement, "UnitId") == 0)
		{
			pEUInformation->UnitId = (OpcUa_Int32)atoi(buff);
		}

		if (pMyData->bDisplayName == 1)
		{
			OpcUa_String_AttachCopy(&(pEUInformation->DisplayName.Locale), "en-us");
			OpcUa_String_AttachCopy(&(pEUInformation->DisplayName.Text), buff);
		}

		if (pMyData->bExtensionObjectDescription == 1)
		{
			OpcUa_String_AttachCopy(&(pEUInformation->Description.Locale), "en-us");
			OpcUa_String_AttachCopy(&(pEUInformation->Description.Text), buff);
		}
		aVariant.Value.ExtensionObject->Body.EncodeableObject.Object = (void*)pEUInformation;
	}
	//Update the variant
	OpcUa_Free(buff);
	return uStatus;
}
// Fonction speciale pour l'init des extensions object contenant des OpcUa_TimeZoneDataType a partir du fichier XML
// quand on arrive dans cette fonction les balises element XML suivante ont été lu :
// <Value> && (<ListOfExtensionObject> || <ExtensionObject>) && <TypeId> && <Body> && <Argument>
// rappel : Les arguments sont les paramètres des méthodes OPC UA
// Il reste a venir <Offset> <DaylightSavingInOffset>
OpcUa_StatusCode xmlInit8912ExtensionObject(HANDLER_DATA* pMyData, const char* s, int len)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	char* buff=(char*)OpcUa_Alloc(len+1);
	ZeroMemory(buff,len+1);
	memcpy(buff,s,len);
	/////////////////////////////////////////////////
	//workaround
	// Si le champ commence par un espace on l'ignore
	if ( (buff[0]==0x20) || (buff[0]==0x0a) )
	{
		OpcUa_Free(buff);
		return uStatus; 
	}
	// fin workaround
	// 
	OpcUa_Variant aVariant=pMyData->pDataValue->GetValue();
	//We need to validate that the ExtensionObject contains the correct DataType (TimeZoneDataType)
	OpcUa_TimeZoneDataType* pTimeZoneDataType=(OpcUa_TimeZoneDataType*)aVariant.Value.ExtensionObject->Body.EncodeableObject.Object;
	if (!pTimeZoneDataType)
	{
		pTimeZoneDataType=(OpcUa_TimeZoneDataType*)OpcUa_Alloc(sizeof(OpcUa_TimeZoneDataType));
		if (pTimeZoneDataType)
			OpcUa_TimeZoneDataType_Initialize(pTimeZoneDataType);
		else
			uStatus = OpcUa_BadOutOfMemory;
	}
	if (uStatus == OpcUa_Good)
	{
		if (OpcUa_StrCmpA(pMyData->szCurrentXmlElement, "Offset") == 0)
		{
			pTimeZoneDataType->Offset = (OpcUa_UInt16)atoi(buff);
		}

		if (OpcUa_StrCmpA(pMyData->szCurrentXmlElement, "DaylightSavingInOffset") == 0)
		{
			if (OpcUa_StrCmpA(buff, "true") == 0)
				pTimeZoneDataType->DaylightSavingInOffset = true;
			else
			{
				if (OpcUa_StrCmpA(buff, "false") == 0)
					pTimeZoneDataType->DaylightSavingInOffset = false;
				else
					pTimeZoneDataType->DaylightSavingInOffset = (OpcUa_Byte)atoi(buff);
			}
		}
		aVariant.Value.ExtensionObject->Body.EncodeableObject.Object = (void*)pTimeZoneDataType;
	}
	//Update the variant
	OpcUa_Free(buff);
	return uStatus;
}
// Fonction speciale pour l'init des extensions object contenant des OpcUa_Argument a partir du fichier XML
// quand on arrive dans cette fonction les balises element XML suivante ont été lu :
// <Value> && (<ListOfExtensionObject> || <ExtensionObject>) && <TypeId> && <Body> && <Argument>
// rappel : Les arguments sont les paramètres des méthodes OPC UA
// Il reste a venir <Name> <DataType> <Identifier> <ValueRank> <ArrayDimensions> <Description>
OpcUa_StatusCode xmlInitArgumentExtensionObject(HANDLER_DATA* pMyData,const char* s,int len)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	char* buff=(char*)OpcUa_Alloc(len+1);
	ZeroMemory(buff,len+1);
	memcpy(buff,s,len);
	/////////////////////////////////////////////////
	//workaround
	// Si le champ commence par un espace on l'ignore
	if ( (buff[0]==0x20) || (buff[0]==0x0a) )
	{
		memset(buff,0,len+1);
		OpcUa_Free(buff);
		return uStatus; 
	}
	// fin workaround
	
	if (pMyData->bExtensionObjectName)
	{
		if (pMyData->pArgument)
		{
			if (OpcUa_String_StrLen(&(pMyData->pArgument->Name)) > 0)
				OpcUa_String_Clear(&(pMyData->pArgument->Name));
			OpcUa_String_Initialize(&(pMyData->pArgument->Name));
			OpcUa_String_AttachCopy(&(pMyData->pArgument->Name), buff);
		}
	}
	else
	{
		if (pMyData->bExtensionObjectDataType)
		{
			if (pMyData->bExtensionObjectIdentifier)
			{	
				OpcUa_NodeId aNodeId;
				OpcUa_NodeId_Initialize(&aNodeId);
				if (ParseNodeId(buff, &aNodeId) == OpcUa_Good)
					OpcUa_NodeId_CopyTo(&aNodeId,&(pMyData->pArgument->DataType));
				else
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Cannot Parse buffer to retrieve the dataType. Check you configuration file\n");
				OpcUa_NodeId_Clear(&aNodeId);
			}
		}
		else
		{
			if (pMyData->bExtensionObjectValueRank)
			{
				OpcUa_Int32 iVal=0;
				int iRes=sscanf_s(buff,"%i",(int*)&iVal);
				if (!iRes)
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"an incorrect ValueRank was used\n");
				else			
					pMyData->pArgument->ValueRank=iVal;
			}
			else
			{
				if (pMyData->bExtensionObjectArrayDimensions)
				{
					// On va compter les dimensions du tableau
					// le nombre de dimension correspond au nombre de (,)+1
					// si 0 virgule et ArrayDimension =0 => dimension  de longueur variable
					basic_string<char>::size_type index=0,firstPos=0;
					basic_string<char> myString(buff);
					std::vector<OpcUa_UInt32>  ArrayDimensions;
					while(index!=basic_string<char>::npos)
					{
						index=myString.find(",",index);		
						basic_string<char> tmpStr=myString.substr(firstPos,index-firstPos);			 
						ArrayDimensions.push_back(atoi(tmpStr.c_str()));
						firstPos=index;
					}
					// Transfert dans l'OpcUa_Argument
					OpcUa_UInt16 uiArrayDimension = ArrayDimensions.size();
					if (uiArrayDimension > 0)
					{
						pMyData->pArgument->ArrayDimensions = (OpcUa_UInt32*)OpcUa_Alloc(uiArrayDimension*sizeof(OpcUa_UInt32));
						OpcUa_MemSet(pMyData->pArgument->ArrayDimensions, 0, uiArrayDimension*sizeof(OpcUa_UInt32));
						int ii = 0;
						for (OpcUa_UInt16 iii = 0; iii < uiArrayDimension; iii++)
						{
							OpcUa_UInt32 uiVal = ArrayDimensions.at(iii);
							pMyData->pArgument->ArrayDimensions[ii++] = uiVal;
						}
						pMyData->pArgument->NoOfArrayDimensions = ii;
					}
				}
				else
				{
					if (pMyData->bExtensionObjectDescription)
					{
						if (OpcUa_String_StrLen(&(pMyData->pArgument->Description.Locale))>0)
							OpcUa_String_Clear(&(pMyData->pArgument->Description.Locale));
						if (OpcUa_String_StrLen(&(pMyData->pArgument->Description.Text))>0)
							OpcUa_String_Clear(&(pMyData->pArgument->Description.Text));
						OpcUa_LocalizedText_Initialize(&(pMyData->pArgument->Description));
						OpcUa_String_AttachCopy(&(pMyData->pArgument->Description.Locale), buff);
						OpcUa_String_AttachCopy(&(pMyData->pArgument->Description.Text), buff);
					}
				}
			}
		}
	}
	OpcUa_Free(buff);
	return uStatus;
}

/// <summary>
/// XMLs the node set character data handler.
/// Fonction délegué pour la prise en charge du chargement des fichiers constituant l'addressSpace
/// </summary>
/// <param name="userData">The user data.</param>
/// <param name="s">The s.</param>
/// <param name="len">The length.</param>
void xmlNodeSetCharacterDataHandler	(void *userData,     /* user defined data */
									 const char *s,  /* non-null terminated character string */
									 int len)
{
	CUAInformationModel* pInformationModel=CServerApplication::m_pTheAddressSpace;
	HANDLER_DATA* pMyData=(HANDLER_DATA*)userData; 
	OpcUa_StatusCode uStatus=OpcUa_Bad;
	if (len==0)
		return;
	if (*s == 0xA) // newline
	{
		// Special case workaround when an empty <Locale></Locale> is provided
		/*
		if ( (pMyData->pUAVariable)
				&& (pMyData->bValue)
				&& (pMyData->bLocale)
				&& (!pMyData->bText))
		{
			OpcUa_DataValue* pDataValue = pMyData->pDataValue->GetInternalDataValue();
			OpcUa_Variant aVariant = pMyData->pDataValue->GetValue();
			if ((aVariant.Datatype == OpcUaType_LocalizedText) && (aVariant.ArrayType == OpcUa_VariantArrayType_Array))
			{
				OpcUa_LocalizedText aNewVal;
				OpcUa_LocalizedText_Initialize(&(aNewVal));
				OpcUa_String_AttachCopy(&(aNewVal.Locale), "en-us");
				OpcUa_Array_AddElt(LocalizedText);				
			}
		}
		*/
		return;
	}
	if (*s==0x9) // TAB
		return;

	if (*s == 0x3c) // <
		return;
	if (*s == 0x3e) // >
		return;
	OpcUa_CharA* buffTmp = (OpcUa_CharA*)OpcUa_Alloc(len + 1);
	ZeroMemory(buffTmp, len + 1);
	memcpy(buffTmp, s, len);
	if (Utils::IsBufferEmpty(buffTmp, len))
	{
		OpcUa_Free(buffTmp);
		return ;
	}
	else
		free(buffTmp);
	// traitement des Uris
	if (pMyData->bNamespaceUri)
	{
		char* buff = (char*)OpcUa_Alloc(len + 1);
		if (buff)
		{
			ZeroMemory(buff, len + 1);
			memcpy(buff, s, len);
			if (!Utils::IsBufferEmpty(buff, len))
			{
				OpcUa_String aNamespaceUri;
				OpcUa_String_Initialize(&aNamespaceUri);
				OpcUa_String_AttachCopy(&aNamespaceUri, buff);
				uStatus = pInformationModel->OnLoadNamespaceUriVerifyUri(aNamespaceUri);
				OpcUa_String_Clear(&aNamespaceUri);
			}
			ZeroMemory(buff, len + 1);
			free(buff);
		}
		return;
	}
	// Traitement de valeur d'initialisation
	if ((pMyData->pUAVariable)
		&& (pMyData->pDataValue)
		&& (pMyData->bValue)
		&& (pMyData->bExtensionObjectBody)
		&& (!(pMyData->bExtensionObjectEnumValueType))
		&& (!(pMyData->bExtensionObjectArgument)))
	{
		// OpcUa_ExtensionObjectEncoding_EncodeableObject
		OpcUa_DataValue* pDataValue = pMyData->pDataValue->GetInternalDataValue();
		if (pDataValue)
		{
			if (pDataValue->Value.ArrayType == OpcUa_VariantArrayType_Scalar)
			{
				OpcUa_ExtensionObject* pExtObject = pDataValue->Value.Value.ExtensionObject;
				if (pExtObject)
				{
					if (pExtObject->Encoding == OpcUa_ExtensionObjectEncoding_EncodeableObject)
					{
						if (pExtObject->Body.EncodeableObject.Type->TypeId == OpcUaId_TimeZoneDataType)
						{
							xmlInit8912ExtensionObject(pMyData, s, len);
							return;
						}
						if (pExtObject->Body.EncodeableObject.Type->TypeId == OpcUaId_EUInformation)
						{
							xmlInit887ExtensionObject(pMyData, s, len);
							return;
						}
					}
				}
			}
		}
	}
	// il s'agit d'un extensionObject contenant un OpcUa_Argument
	if ( (pMyData->pUAVariable) 
		&& (pMyData->pDataValue)
		&& (pMyData->bValue)
		&& (pMyData->bExtensionObjectBody)
		&& (pMyData->bExtensionObjectArgument) )
	{
		xmlInitArgumentExtensionObject(pMyData,s,len);
		return ;
	}
	else
	{
		// il s'agit d'un extensionObject contenant un OpcUa_EnumValueType
		if ( (pMyData->pUAVariable) 
			&& (pMyData->pDataValue)
			&& (pMyData->bValue) 
			&& (pMyData->bExtensionObjectBody)
			&& (pMyData->bExtensionObjectEnumValueType) )
		{
			xmlInitEnumValueTypeExtensionObject(pMyData,s,len);
			return;
		}
		else
		{
			// Il s'agit d'autre chose.
			// Cette autre chose peut être un type simple, un tableau de type simple 
			// ou un extensionObject qui contient autre chose qu'un OpcUa_Argument ou un OpcUa_EnumValueType
			if ( (pMyData->pUAVariable) && (pMyData->pDataValue) && (pMyData->bValue) && (!pMyData->pReferenceNode) )
			{
				char* buff=(char*)OpcUa_Alloc(len+1);
				if (buff)
				{
					ZeroMemory(buff, len + 1);
					memcpy(buff, s, len);
					uStatus = OpcUa_Good;
				}
				/////////////////////////////////////////////////
				//OpcUa_DataValue* pDataValue = pMyData->pDataValue->GetInternalDataValue();
				OpcUa_Variant aVariant=pMyData->pDataValue->GetValue();
				switch (aVariant.Datatype)
				{
				case OpcUaType_Boolean:
					if (!Utils::IsBufferEmpty(buff,len))
					{
						if (aVariant.ArrayType==OpcUa_VariantArrayType_Array)
						{
							OpcUa_Boolean* pNewVal=(OpcUa_Boolean*)OpcUa_Alloc(sizeof(OpcUa_Boolean));
							if (OpcUa_StriCmpA(buff,"true")==0)
								*pNewVal=(OpcUa_Boolean)OpcUa_True;
							else
							{
								if (OpcUa_StriCmpA(buff,"false")==0)
									*pNewVal=(OpcUa_Boolean)OpcUa_False;
								else
									*pNewVal=(OpcUa_Boolean)atoi(buff);
							}
							OpcUa_Array_AddElt(Boolean);
						}
						else
						{
							if (aVariant.ArrayType==OpcUa_VariantArrayType_Scalar)
							{
								if (OpcUa_StriCmpA(buff,"true")==0)
									aVariant.Value.Boolean=true;
								else
								{
									if (OpcUa_StriCmpA(buff,"false")==0)
										aVariant.Value.Boolean=false;
									else
										aVariant.Value.Boolean=(OpcUa_Boolean)atoi(buff);
								}
							}
						}
						pMyData->pDataValue->SetValue(aVariant);
						pMyData->pDataValue->SetStatusCode(OpcUa_Good);
					}
					break;
				case OpcUaType_Byte:
					if (!Utils::IsBufferEmpty(buff,len))
					{
						if (aVariant.ArrayType==OpcUa_VariantArrayType_Array)
						{	
							OpcUa_Byte* pNewVal = (OpcUa_Byte*)OpcUa_Alloc(sizeof(OpcUa_Byte));
							*pNewVal=(OpcUa_Byte)atoi(buff);
							OpcUa_Array_AddElt(Byte);
						}
						else
						{
							if (aVariant.ArrayType==OpcUa_VariantArrayType_Scalar)
								aVariant.Value.Byte=(OpcUa_Byte)atoi(buff);
						}
						pMyData->pDataValue->SetValue(aVariant);
						pMyData->pDataValue->SetStatusCode(OpcUa_Good);
					}
					break;
				case OpcUaType_SByte:
					if (!Utils::IsBufferEmpty(buff,len))
					{
						if (aVariant.ArrayType==OpcUa_VariantArrayType_Array)
						{	
							OpcUa_SByte* pNewVal = (OpcUa_SByte*)OpcUa_Alloc(sizeof(OpcUa_SByte));
							*pNewVal=(OpcUa_SByte)atoi(buff);
							OpcUa_Array_AddElt(SByte);
						}
						else
						{
							if (aVariant.ArrayType==OpcUa_VariantArrayType_Scalar)
								aVariant.Value.Byte=(OpcUa_Byte)atoi(buff);
						}
						pMyData->pDataValue->SetValue(aVariant);
						pMyData->pDataValue->SetStatusCode(OpcUa_Good);
					}
					break;
				case OpcUaType_Int16:
					if (!Utils::IsBufferEmpty(buff,len))
					{
						if (aVariant.ArrayType==OpcUa_VariantArrayType_Array)
						{	
							OpcUa_Int16* pNewVal = (OpcUa_Int16*)OpcUa_Alloc(sizeof(OpcUa_Int16));
							*pNewVal=(OpcUa_Int16)atoi(buff);
							OpcUa_Array_AddElt(Int16);
						}
						else
							aVariant.Value.Int16=(OpcUa_Int16)atoi(buff);
						pMyData->pDataValue->SetValue(aVariant);
						pMyData->pDataValue->SetStatusCode(OpcUa_Good);
					}
					break;
				case OpcUaType_UInt16:
					if (!Utils::IsBufferEmpty(buff,len))
					{
						if (aVariant.ArrayType==OpcUa_VariantArrayType_Array)
						{	
							OpcUa_UInt16* pNewVal = (OpcUa_UInt16*)OpcUa_Alloc(sizeof(OpcUa_UInt16));
							*pNewVal = (OpcUa_UInt16)atoi(buff);
							OpcUa_Array_AddElt(UInt16);
						}
						else
							aVariant.Value.UInt16=(OpcUa_Int16)atoi(buff);
						pMyData->pDataValue->SetValue(aVariant);
						pMyData->pDataValue->SetStatusCode(OpcUa_Good);
					}
					break;
				case OpcUaType_Int32:
					if (!Utils::IsBufferEmpty(buff,len))
					{
						if (aVariant.ArrayType==OpcUa_VariantArrayType_Array)
						{	
							OpcUa_Int32* pNewVal = (OpcUa_Int32*)OpcUa_Alloc(sizeof(OpcUa_Int32));
							*pNewVal = (OpcUa_Int32)atoi(buff);
							OpcUa_Array_AddElt(Int32);
						}
						else
							aVariant.Value.Int32=(OpcUa_Int32)atol(buff);
						pMyData->pDataValue->SetValue(aVariant);
						pMyData->pDataValue->SetStatusCode(OpcUa_Good);
					}
					break;
				case OpcUaType_UInt32:
					if (!Utils::IsBufferEmpty(buff,len))
					{
						if (aVariant.ArrayType==OpcUa_VariantArrayType_Array)
						{	
							OpcUa_UInt32* pNewVal = (OpcUa_UInt32*)OpcUa_Alloc(sizeof(OpcUa_UInt32));
							*pNewVal = (OpcUa_UInt32)atoi(buff);
							OpcUa_Array_AddElt(UInt32);
						}
						else
							aVariant.Value.UInt32=(OpcUa_UInt32)atol(buff);
						pMyData->pDataValue->SetValue(aVariant);
						pMyData->pDataValue->SetStatusCode(OpcUa_Good);
					}
					break;
				case OpcUaType_Int64:
					if (!Utils::IsBufferEmpty(buff,len))
					{
						if (aVariant.ArrayType==OpcUa_VariantArrayType_Array)
						{	
							OpcUa_Int64* pNewVal = (OpcUa_Int64*)OpcUa_Alloc(sizeof(OpcUa_Int64));
							*pNewVal = (OpcUa_Int64)atol(buff);
							OpcUa_Array_AddElt(Int64);
						}
						else
							aVariant.Value.Int64=(OpcUa_Int64)atol(buff);
						pMyData->pDataValue->SetValue(aVariant);
						pMyData->pDataValue->SetStatusCode(OpcUa_Good);
					}
					break;
				case OpcUaType_UInt64:
					if (!Utils::IsBufferEmpty(buff,len))
					{

						if (aVariant.ArrayType==OpcUa_VariantArrayType_Array)
						{
							// Mise en forme du nouveau element

							OpcUa_UInt64* pNewVal = (OpcUa_UInt64*)OpcUa_Alloc(sizeof(OpcUa_UInt64));
							OpcUa_UInt64_Initialize(pNewVal);
							*pNewVal = (OpcUa_UInt64)atol(buff);
							OpcUa_Array_AddElt(UInt64);
						}
						else
							aVariant.Value.UInt64=(OpcUa_UInt64)atol(buff);
						pMyData->pDataValue->SetValue(aVariant);
						pMyData->pDataValue->SetStatusCode(OpcUa_Good);
					}
					break;
				case OpcUaType_Float:
					if (!Utils::IsBufferEmpty(buff,len))
					{
						if (aVariant.ArrayType==OpcUa_VariantArrayType_Array)
						{
							// Mise en forme du nouveau element
							OpcUa_Float* pNewVal = (OpcUa_Float*)OpcUa_Alloc(sizeof(OpcUa_Float));
							OpcUa_Float_Initialize(pNewVal);
							*pNewVal=(OpcUa_Float)atof(buff);
							OpcUa_Array_AddElt(Float);
						}
						else
						{
							if (aVariant.ArrayType==OpcUa_VariantArrayType_Scalar)
								aVariant.Value.Float=(OpcUa_Float)atof(buff);
						}
						pMyData->pDataValue->SetValue(aVariant);
						pMyData->pDataValue->SetStatusCode(OpcUa_Good);
					}
					break;
				case OpcUaType_Double:
					if (!Utils::IsBufferEmpty(buff,len))
					{
						if (aVariant.ArrayType==OpcUa_VariantArrayType_Array)
						{
							OpcUa_Double* pNewVal = (OpcUa_Double*)OpcUa_Alloc(sizeof(OpcUa_Double));
							OpcUa_Double_Initialize(pNewVal);
							if (buff)
							{
								if (sscanf(buff, "%lf", pNewVal) == 1)
									OpcUa_Array_AddElt(Double);
							}
							else
								OpcUa_Array_AddElt(Double);
						}
						else
						{
							if (aVariant.ArrayType==OpcUa_VariantArrayType_Scalar)
							{
								OpcUa_Double aNewVal=0.0;
								if (buff)
								{
									if (sscanf(buff, "%lf", &aNewVal)==1)
										aVariant.Value.Double = aNewVal;
								}
								else
									aVariant.Value.Double=aNewVal;
							}
						}
						pMyData->pDataValue->SetValue(aVariant);
						pMyData->pDataValue->SetStatusCode(OpcUa_Good);
					}			
					break;
				case OpcUaType_Guid:
					if (!Utils::IsBufferEmpty(buff,len))
					{
						if (aVariant.ArrayType==OpcUa_VariantArrayType_Array)
						{
							// Mise en forme du nouveau element
							OpcUa_Guid* pNewVal = (OpcUa_Guid*)OpcUa_Alloc(sizeof(OpcUa_Guid));
							OpcUa_Guid_Initialize(pNewVal);
							OpcUa_Guid_FromString(buff,pNewVal);	
							OpcUa_Array_AddElt(Guid);
							pMyData->pDataValue->SetValue(aVariant);
							// Verify release below ask for verification on nov 2015
							//OpcUa_Guid_Clear(aVariant.Value.Guid);
							//OpcUa_Free(aVariant.Value.Guid);
							//aVariant.Value.Guid = OpcUa_Null;
						}
						else
						{
							if (aVariant.ArrayType == OpcUa_VariantArrayType_Scalar)
							{
								OpcUa_Guid_Initialize(aVariant.Value.Guid);
								OpcUa_Guid_FromString(buff, aVariant.Value.Guid);
								pMyData->pDataValue->SetValue(aVariant);
								OpcUa_Guid_Clear(aVariant.Value.Guid);
								OpcUa_Free(aVariant.Value.Guid);
								aVariant.Value.Guid = OpcUa_Null;
							}
						}
						pMyData->pDataValue->SetStatusCode(OpcUa_Good);
					}
					break;
				case OpcUaType_QualifiedName:
					if (!Utils::IsBufferEmpty(buff, len))
					{
						if (aVariant.ArrayType == OpcUa_VariantArrayType_Array)
						{
							// Mise en forme du nouveau element
							OpcUa_QualifiedName* pNewVal = (OpcUa_QualifiedName*)OpcUa_Alloc(sizeof(OpcUa_QualifiedName));
							OpcUa_QualifiedName_Initialize(pNewVal);

							OpcUa_String_AttachCopy(&(pNewVal->Name), buff);
							OpcUa_Array_AddElt(QualifiedName);
							pMyData->pDataValue->SetValue(aVariant);
						}
						else
						{
							if (aVariant.ArrayType == OpcUa_VariantArrayType_Scalar)
							{
								OpcUa_QualifiedName_Initialize(aVariant.Value.QualifiedName);
								OpcUa_String_AttachCopy(&(aVariant.Value.QualifiedName->Name), buff);
								pMyData->pDataValue->SetValue(aVariant);
								OpcUa_QualifiedName_Clear(aVariant.Value.QualifiedName);
							}
						}
						pMyData->pDataValue->SetStatusCode(OpcUa_Good);
					}
				break;
				case OpcUaType_LocalizedText:
					if (!Utils::IsBufferEmpty(buff, len))
					{
						if (aVariant.ArrayType == OpcUa_VariantArrayType_Array)
						{
							// Mise en forme du nouveau element
							OpcUa_LocalizedText* pNewVal = (OpcUa_LocalizedText*)OpcUa_Alloc(sizeof(OpcUa_LocalizedText));
							OpcUa_LocalizedText_Initialize(pNewVal);
							// Data correspondant à <Locale> uniquement
							if ((pMyData->bLocale) && (pMyData->bText == OpcUa_False))
							{
								if (Utils::IsBufferEmpty(buff, len))
									OpcUa_String_AttachCopy(&(pNewVal->Locale), "en-us");
								else
									OpcUa_String_AttachCopy(&(pNewVal->Locale), buff);
								OpcUa_Array_AddElt(LocalizedText);
								// Special case handling
								pMyData->pDataValue->SetValue(aVariant);
							}
							else
							{
								if ((!pMyData->bLocale) && (pMyData->bText))
								{
									if (!Utils::IsBufferEmpty(buff, len))
									{
										OpcUa_String_AttachCopy(&(pNewVal->Text), buff);
										OpcUa_Int32  iLength = aVariant.Value.Array.Length;
										OpcUa_String strLocale;
										OpcUa_String_Initialize(&strLocale);
										if (iLength > 0)
											OpcUa_String_CopyTo(&(aVariant.Value.Array.Value.LocalizedTextArray[iLength - 1].Locale), &strLocale);
										if (pMyData->uiArrayCurrentElt ==iLength)
										{
											OpcUa_String_AttachCopy(&(pNewVal->Locale), "en-us");
											OpcUa_Array_AddElt(LocalizedText);
											pMyData->pDataValue->SetValue(aVariant);
										}
										else
										{
											// here there are no need to call pMyData->pDataValue->SetValue(aVariant);
											// because it was already added during the management of Locale
											OpcUa_String_CopyTo(&(pNewVal->Text), &(aVariant.Value.Array.Value.LocalizedTextArray[iLength - 1].Text));
											OpcUa_LocalizedText_Clear(pNewVal);
											OpcUa_Free(pNewVal);
										}
										OpcUa_String_Clear(&strLocale);
									}
								}
							}
						}
						else
						{
							if (aVariant.ArrayType == OpcUa_VariantArrayType_Scalar)
							{
								//OpcUa_LocalizedText_Initialize(aVariant.Value.LocalizedText);
								if (pMyData->bText)
									OpcUa_String_AttachCopy(&(aVariant.Value.LocalizedText->Text), buff);
								if (pMyData->bLocale)
									OpcUa_String_AttachCopy(&(aVariant.Value.LocalizedText->Locale), buff);
							}
						}
						//pMyData->pDataValue->SetValue(aVariant);
						pMyData->pDataValue->SetStatusCode(OpcUa_Good);
					}
					break;
				case OpcUaType_DateTime:
					if (!Utils::IsBufferEmpty(buff,len))
					{
						if (aVariant.ArrayType==OpcUa_VariantArrayType_Array)
						{
							// Mise en forme du nouveau DateTime
							OpcUa_DateTime* pNewVal = (OpcUa_DateTime*)OpcUa_Alloc(sizeof(OpcUa_DateTime));
							OpcUa_DateTime_Initialize(pNewVal);
							unsigned int uiYear,uiMonth,uiDay,uiHour,uiMin,uiSec;
							if (sscanf(buff,"%04u-%02u-%02uT%02u:%02u:%02u",&uiYear,&uiMonth,&uiDay,&uiHour,&uiMin,&uiSec)!=EOF)
								OpcUa_DateTime_GetDateTimeFromString(buff,pNewVal);
							OpcUa_Array_AddElt(DateTime);
							pMyData->pDataValue->SetValue(aVariant);
						}
						else
						{
							unsigned int uiYear,uiMonth,uiDay,uiHour,uiMin,uiSec;
							if (sscanf(buff,"%04u-%02u-%02uT%02u:%02u:%02u",&uiYear,&uiMonth,&uiDay,&uiHour,&uiMin,&uiSec)!=EOF)
							{
								OpcUa_DateTime_Initialize(&(aVariant.Value.DateTime));
								OpcUa_DateTime_GetDateTimeFromString(buff,&(aVariant.Value.DateTime));
								pMyData->pDataValue->SetValue(aVariant);
							}
							else
								uStatus = OpcUa_Bad;
						}
						pMyData->pDataValue->SetStatusCode(OpcUa_Good);
					}
					break;
				case OpcUaType_String:
					if (!Utils::IsBufferEmpty(buff,len))
					{
						if (aVariant.ArrayType==OpcUa_VariantArrayType_Array)
						{
							// Mise en forme du nouveau element
							OpcUa_String* pNewVal = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
							OpcUa_String_Initialize(pNewVal);
							OpcUa_String_AttachCopy(pNewVal,buff);
							OpcUa_Array_AddElt(String);
							pMyData->pDataValue->SetValue(aVariant);
							OpcUa_String_Clear(pNewVal);
						}
						else
						{
							if (aVariant.ArrayType==OpcUa_VariantArrayType_Scalar)
							{
								OpcUa_String_Initialize(&(aVariant.Value.String));
								OpcUa_String_AttachCopy(&(aVariant.Value.String),buff);
								pMyData->pDataValue->SetValue(aVariant);
								OpcUa_String_Clear(&(aVariant.Value.String));
							}
						}
						pMyData->pDataValue->SetStatusCode(OpcUa_Good);
					}
					break;
				case OpcUaType_NodeId:
					if (!Utils::IsBufferEmpty(buff, len))
					{
						// supprimer " 0x22 au debut et à la fin de buff
						// ATTENTION >0x22 ne concerne que les NodeId qui seront des chaines
						size_t iSize = strlen(buff);
						char* szNodeId = OpcUa_Null;
						if (buff[0]==0x22)
							szNodeId = (char*)OpcUa_Alloc(iSize - 1);
						else
							szNodeId = (char*)OpcUa_Alloc(iSize+1);
						if (szNodeId)
						{
							if (buff[0] == 0x22)
							{
								ZeroMemory(szNodeId, iSize - 1);
								OpcUa_MemCpy(szNodeId, iSize - 2, &buff[1], iSize - 2);
							}
							else
							{
								ZeroMemory(szNodeId, iSize+1);
								OpcUa_MemCpy(szNodeId, iSize, &buff[0], iSize);
							}
							if (aVariant.ArrayType == OpcUa_VariantArrayType_Array)
							{
								// Mise en forme du nouveau element
								OpcUa_NodeId* pNewVal = (OpcUa_NodeId*)OpcUa_Alloc(sizeof(OpcUa_NodeId));
								OpcUa_NodeId_Initialize(pNewVal);
								uStatus = ParseNodeId(szNodeId, pNewVal);
								if (uStatus == OpcUa_Good)
								{
									OpcUa_Array_AddElt(NodeId);
								}
								else
									OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "ParseNodeId failed on a NodeId initialization from the NodeSetFile\n");
								OpcUa_NodeId_Clear(pNewVal);
							}
							else
							{
								if (aVariant.ArrayType == OpcUa_VariantArrayType_Scalar)
								{

									OpcUa_NodeId aNewVal;
									OpcUa_NodeId_Initialize(&aNewVal);
									uStatus = ParseNodeId(szNodeId, &aNewVal);
									if (uStatus == OpcUa_Good)
									{
										OpcUa_NodeId_Initialize(aVariant.Value.NodeId);
										OpcUa_NodeId_CopyTo(&aNewVal, aVariant.Value.NodeId);
									}
									else
										OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "ParseNodeId failed on a NodeId initialization from the NodeSetFile\n");
									OpcUa_NodeId_Clear(&(aNewVal));
								}
							}
							pMyData->pDataValue->SetValue(aVariant);
							pMyData->pDataValue->SetStatusCode(OpcUa_Good);
							OpcUa_Free(szNodeId);
						}
						else
							uStatus = OpcUa_BadOutOfMemory;
					}
					break;
				case OpcUaType_ByteString:
					if (!Utils::IsBufferEmpty(buff,len))
					{
						OpcUa_DataValue* pTmpDataValue=pMyData->pDataValue->GetInternalDataValue();
						if (pTmpDataValue->Value.ArrayType == OpcUa_VariantArrayType_Array)
						{
							// structure de stockage temporaire
							//OpcUa_String_Initialize(&(pMyData->tmpString));
							//OpcUa_String_AttachCopy(&(pMyData->tmpString),buff);
							OpcUa_ByteString* pNewVal = (OpcUa_ByteString*)OpcUa_Alloc(sizeof(OpcUa_ByteString));
							OpcUa_ByteString_Initialize(pNewVal);
							if (buff)
							{
								pNewVal->Data=(OpcUa_Byte*)OpcUa_Alloc(len);
								pNewVal->Length=len;
								OpcUa_MemCpy(pNewVal->Data,len,buff,len);
							}
							OpcUa_Array_AddElt(ByteString);
							pTmpDataValue->Value.Value.Array.Value.ByteStringArray=aVariant.Value.Array.Value.ByteStringArray;
							pTmpDataValue->Value.Value.Array.Length=aVariant.Value.Array.Length;
						}
						else
						{
							// Is it a ByteString split in a multiple line ?
							// A multipleline ByteString can be detected by a already initialize aVariant
							if ((pTmpDataValue->Value.Value.ByteString.Length == 0) || (pTmpDataValue->Value.Value.ByteString.Length == -1))
							{
								OpcUa_ByteString_Initialize(&(pTmpDataValue->Value.Value.ByteString));
								pTmpDataValue->Value.Value.ByteString.Data = (OpcUa_Byte*)OpcUa_Alloc(len);
								pTmpDataValue->Value.Value.ByteString.Length = len;
								ZeroMemory(pTmpDataValue->Value.Value.ByteString.Data, len);
								OpcUa_MemCpy(pTmpDataValue->Value.Value.ByteString.Data, len, buff, len);
							}
							else
							{
								OpcUa_UInt32 uiOldLen = aVariant.Value.ByteString.Length;
								OpcUa_UInt32 uiLen = aVariant.Value.ByteString.Length + len;
								OpcUa_Byte* pTmpData = (OpcUa_Byte*)OpcUa_ReAlloc(pTmpDataValue->Value.Value.ByteString.Data, uiLen);
								if (pTmpData)
								{
									pTmpDataValue->Value.Value.ByteString.Data = pTmpData;
									pTmpDataValue->Value.Value.ByteString.Length = uiLen;
								}
								else
								{
									OpcUa_Free(pTmpDataValue->Value.Value.ByteString.Data);
									pTmpDataValue->Value.Value.ByteString.Data = OpcUa_Null;
								}

								OpcUa_MemCpy(&pTmpDataValue->Value.Value.ByteString.Data[uiOldLen], len, buff, len);

							}
							pMyData->pDataValue->SetStatusCode(OpcUa_Good);
						}
					}
					break;
				case OpcUaType_ExtensionObject:
					if (!Utils::IsBufferEmpty(buff,len))
					{
						if (pMyData->bExtensionObjectTypeId)
						{
							OpcUa_NodeId aNodeId;
							OpcUa_NodeId_Initialize(&aNodeId);
							if (ParseNodeId(buff,&aNodeId)==OpcUa_Good) // the buffer we received from the parser is suppose to contains a nodeId (typeId)
							{
								// initialisation du typeId de l'extensionObject a partir de l'identifier lu dans le fichier XML						
								OpcUa_EncodeableType* pEncodeableType;
								uStatus=g_pTheApplication->LookupEncodeableType(aNodeId.Identifier.Numeric,&pEncodeableType);
								if (uStatus==OpcUa_Good)
								{
									if (pEncodeableType)
									{
										aNodeId.Identifier.Numeric=pEncodeableType->TypeId;
										// Est ce que l'on a faire à un tableau, un scalaire ou une matrice ?
										if (aVariant.ArrayType==OpcUa_VariantArrayType_Array)
										{		
											// This will add pNewVal in aVariant
											if (aVariant.Value.Array.Value.ExtensionObjectArray == OpcUa_Null)
											{
												OpcUa_UInt32 uiVal = 1;
												pMyData->pDataValue->InitializeArray(OpcUaType_ExtensionObject,1);
												OpcUa_Variant aVariant = pMyData->pDataValue->GetValue();
											}											
											
											if (pMyData->pDataValue)
											{
												OpcUa_ExtensionObject* pNewVal = (OpcUa_ExtensionObject*)OpcUa_Alloc(sizeof(OpcUa_ExtensionObject));
												OpcUa_ExtensionObject_Initialize(pNewVal);
												pNewVal->BodySize = 0;
												pNewVal->Encoding = OpcUa_ExtensionObjectEncoding_EncodeableObject;
												OpcUa_NodeId_CopyTo(&aNodeId, &(pNewVal->TypeId.NodeId));

												pNewVal->TypeId.NodeId.Identifier.Numeric = pEncodeableType->BinaryEncodingTypeId;;
												pNewVal->TypeId.NodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
												pNewVal->TypeId.ServerIndex = 0;
												pNewVal->Body.EncodeableObject.Type = pEncodeableType;
												pNewVal->Body.EncodeableObject.Object = OpcUa_Alloc(pEncodeableType->AllocationSize);
												ZeroMemory(pNewVal->Body.EncodeableObject.Object, pEncodeableType->AllocationSize);
												//OpcUa_Free(pEncodeableType); // Modif mai 2015
												OpcUa_DataValue* pTmpDataValue = pMyData->pDataValue->GetInternalDataValue();
												OpcUa_Array_AddElt(ExtensionObject);
												pTmpDataValue->Value.Value.Array.Value.ExtensionObjectArray = aVariant.Value.Array.Value.ExtensionObjectArray;
												pTmpDataValue->Value.Value.Array.Length = aVariant.Value.Array.Length;
											}
											else
											{
												OpcUa_NodeId* pNodeId = OpcUa_Null;
												if (GetNodeIdForCurrentParsedObject(pMyData, &pNodeId) == OpcUa_Good)
												{
													char* szNodeId = OpcUa_Null;
													Utils::NodeId2String(pNodeId, &szNodeId);
													if (szNodeId)
													{
														OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_DEBUG, "%s contains an ExtensionObject that is not initialize properly\n", szNodeId);
														OpcUa_Free(szNodeId);
													}
													OpcUa_Free(pNodeId);
												}
											}

											///////////////////////////////////////////////////////////////////////////////////
										}
										else
										{
											if (aVariant.ArrayType==OpcUa_VariantArrayType_Scalar)
											{
												if (!aVariant.Value.ExtensionObject)
												{
													aVariant.Value.ExtensionObject = (OpcUa_ExtensionObject*)OpcUa_Alloc(sizeof(OpcUa_ExtensionObject));
													OpcUa_ExtensionObject_Initialize(aVariant.Value.ExtensionObject);
												}
												OpcUa_NodeId_CopyTo(&aNodeId, &(aVariant.Value.ExtensionObject->TypeId.NodeId));
												aVariant.Value.ExtensionObject->BodySize=0;
												aVariant.Value.ExtensionObject->Encoding=OpcUa_ExtensionObjectEncoding_EncodeableObject;
												aVariant.Value.ExtensionObject->TypeId.ServerIndex=0;
												OpcUa_String_Initialize(&(aVariant.Value.ExtensionObject->TypeId.NamespaceUri));
												aVariant.Value.ExtensionObject->Body.EncodeableObject.Type = pEncodeableType; //  Utils::Copy(pEncodeableType); Modif mai 2015
												aVariant.Value.ExtensionObject->Body.EncodeableObject.Object=OpcUa_Alloc(pEncodeableType->AllocationSize);
											}
											//else
											//	OpcUa_Free(pEncodeableType); // modif mai 2015. Hypthesis a Variant with an unknown ArrayType is receive the will leak so... free it
										}
										
										pMyData->pDataValue->SetStatusCode(OpcUa_Good);
										// OpcUa_Free(pEncodeableType); // modif mai 2015 we keep because we just assign instead of copying
									}
								}
								else
								{
									OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "The TypeId %s for this extensionObject is unknown in the EncodeableTypeTable. Please check you NodeSet file\n",buff);
									uStatus = OpcUa_Bad;
								}
							}
							OpcUa_NodeId_Clear(&aNodeId);
						}								
					}
					break;
				default:
					uStatus = OpcUa_Bad;
					break;
				}
				if (uStatus == OpcUa_Good)
				{
					// will set up the timesptamp
					CDataValue* pDataValue00=pMyData->pUAVariable->GetValue();
					if (pDataValue00)
					{
						pDataValue00->SetServerTimestamp(OpcUa_DateTime_UtcNow());
						pDataValue00->SetSourceTimestamp(OpcUa_DateTime_UtcNow());
					}
				}
				OpcUa_Free(buff);
				return;
			}
		}
	}
	// Traitement de la description
	if (pMyData->bDescription)
	{
		// Description
		OpcUa_LocalizedText lName;
		OpcUa_LocalizedText_Initialize(&lName);
		char* buff00 = (char*)malloc(len + 1);
		if (buff00)
		{
			ZeroMemory(buff00, len + 1);
			memcpy(buff00, s, len);
			if (strlen(buff00) > 0)
			{
				OpcUa_String_AttachCopy(&(lName.Locale), (OpcUa_StringA)"en-us");
				OpcUa_String_AttachCopy(&(lName.Text), (OpcUa_StringA)buff00);
				if (pMyData->pField)
					pMyData->pField->SetDescription(lName);
				else
				{
					CUABase* pUABase = GetCurrentHandledClass(pMyData);
					if (pUABase)
					{
						pUABase->SetDescription(lName);
						ZeroMemory(buff00, len + 1);
					}
				}
			}
			free(buff00);
		}
		OpcUa_LocalizedText_Clear(&lName);
		return;
	}
	// Traitement du DisplayName
	if (pMyData->bDisplayName)
	{
		// DisplayName
		OpcUa_LocalizedText lName;
		OpcUa_LocalizedText_Initialize(&lName);
		CUABase* pUABase=GetCurrentHandledClass(pMyData);
		if (pUABase)
		{
			char* buff00 = (char*)malloc(len + 1);
			if (buff00)
			{
				ZeroMemory(buff00, len + 1);
				memcpy(buff00, s, len);
				if (strlen(buff00) > 0)
				{
					OpcUa_String_AttachCopy(&(lName.Locale), (OpcUa_StringA)"en-us");
					OpcUa_String_AttachCopy(&(lName.Text), (OpcUa_StringA)buff00);
					pUABase->SetDisplayName(&lName);
				}
				ZeroMemory(buff00, len + 1);
				free(buff00);
			}
		}
		OpcUa_LocalizedText_Clear(&lName);
		return;
	}
	// Traitement de l'InverseName
	if (pMyData->bInverseName)
	{
		// InverseName
		OpcUa_LocalizedText lName;
		OpcUa_LocalizedText_Initialize(&lName);
		CUABase* pUABase=GetCurrentHandledClass(pMyData);
		if (pUABase)
		{
			char* buff00=(char*)malloc(len+1);
			if (buff00)
			{
				ZeroMemory(buff00, len + 1);
				memcpy(buff00, s, len);
				if (strlen(buff00) > 0)
				{
					OpcUa_String_AttachCopy(&(lName.Locale), (OpcUa_StringA)"en-us");
					OpcUa_String_AttachCopy(&(lName.Text), (OpcUa_StringA)buff00);
					((CUAReferenceType*)pUABase)->SetInverseName(lName);
				}
				ZeroMemory(buff00, len + 1);
				free(buff00);
			}
		}
		OpcUa_LocalizedText_Clear(&lName);
		return;
	}
	// Traitement des Alias
	if (pMyData->pAlias)
	{
		char* buff00=(char*)malloc(len+1);
		if (buff00)
		{
			ZeroMemory(buff00, len + 1);
			memcpy(buff00, s, len);
			OpcUa_NodeId* pNodeId = (OpcUa_NodeId*)OpcUa_Alloc(sizeof(OpcUa_NodeId));
			if (pNodeId)
			{
				OpcUa_NodeId_Initialize(pNodeId);
				if (ParseNodeId(buff00, pNodeId) == OpcUa_Good)
				{
					// Affectation du resultat en fonction du type parsé
					pMyData->pAlias->SetNodeId(pNodeId);
				}
				OpcUa_NodeId_Clear(pNodeId);
				OpcUa_Free(pNodeId);
			}
			free(buff00);
		}
		return ;
	}
	// Traitement des references
	if ( ( 
		( (pMyData->pUAVariable) 
		|| (pMyData->pDataValue) 
		|| (pMyData->pAlias) 
		|| (pMyData->pMethod) 
		|| (pMyData->pReferenceNode) 
		|| (pMyData->pUADataType)  
		|| (pMyData->pUAObject)  
		|| (pMyData->pUAObjectType) 
		|| (pMyData->pUAReferenceType) 
		|| (pMyData->pView) ) 
		&& (pMyData->pReferenceNode) ))
	{
		char* buff00=(char*)malloc(len+1);
		if (buff00)
		{
			ZeroMemory(buff00, len + 1);
			memcpy(buff00, s, len);
			OpcUa_NodeId* pNodeId = (OpcUa_NodeId*)OpcUa_Alloc(sizeof(OpcUa_NodeId));
			OpcUa_NodeId_Initialize(pNodeId);
			/////////////////////////////////////////////////
			//workaround
			// on ignore les espace, la tabulation horizontale et le tabulation verticale.
			if ((buff00[0] == 0x20) || (buff00[0] == 0x0a) || (buff00[0] == 0x09))
			{
				memset(buff00, 0, len + 1);
				free(buff00);
				return; // Si le champ commence par un espace on l'ignore
			}
			// fin workaround
			/////////////////////////////////////////////////
			if (ParseNodeId(buff00, pNodeId) == OpcUa_Good)
			{
				pMyData->pReferenceNode->SetTargetId(pNodeId);
				//pMyData->pReferenceNode->TargetId.ServerIndex=0; // aNodeId.NamespaceIndex; On suppose que toute les node font réference au server local
				// On va essayer d'affecter la definition du type du noeud
				// S'il s'agit de la definition du type du noeud (HasTypeDefinition ns=0;i=40)
				if (pMyData->pReferenceNode->IsTypeDefinition())
				{
					switch (pMyData->NodeClass)
					{
					case OpcUa_NodeClass_View:
						pMyData->pView->SetTypeDefinition(pNodeId);
						break;
					case OpcUa_NodeClass_ObjectType:
						pMyData->pUAObjectType->SetTypeDefinition(pNodeId);
						break;
					case OpcUa_NodeClass_Object:
						pMyData->pUAObject->SetTypeDefinition(pNodeId);
						break;
					case OpcUa_NodeClass_ReferenceType:
						pMyData->pUAReferenceType->SetTypeDefinition(pNodeId);
						break;
					case OpcUa_NodeClass_VariableType:
						pMyData->pUAVariableType->SetTypeDefinition(pNodeId);
						break;
					case OpcUa_NodeClass_Variable:
						pMyData->pUAVariable->SetTypeDefinition(pNodeId);
						break;
					case OpcUa_NodeClass_DataType:
						pMyData->pUADataType->SetTypeDefinition(pNodeId);
						break;
					case OpcUa_NodeClass_Method:
						pMyData->pMethod->SetTypeDefinition(pNodeId);
						break;
					default:
						uStatus = OpcUa_Bad;
						break;
					}
				}
				if (Utils::IsNodeIdNull(pMyData->pReferenceNode->GetTargetId().NodeId))
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "xmlNodeSetCharacterDataHandler>Handlinf references a Null NodeId detected\n");

			}
			else
			{
				// NodeId
				if (pNodeId)
				{
					char* szNodeId = OpcUa_Null;
					Utils::NodeId2String(pNodeId, &szNodeId);
					if (szNodeId)
					{
						// Attribute
						OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "ParseNodeId: %s failed. Check that your XML file is not corrupted\n", szNodeId);
						free(szNodeId);
					}
				}
			}
			OpcUa_NodeId_Clear(pNodeId);
			OpcUa_Free(pNodeId);
			free(buff00);
		}
		return ;
	}
	// 
	pMyData->uStatus = OpcUa_BadInternalError; // let mark an internal error
	OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"xmlNodeSetCharacterDataHandler: unexpected situation. Please contact OpenOpcUa Team leader\n");
}

// Parsing du fichier XML
///////////////////////////////////////////////////////////////////////////////////////
// structure userData associé au parsing du fichier xml de configuration des sous-systèmes
typedef struct handler_Subsystem_Data 
{
	XML_ParserStruct*		XML_Parser;    
	void*					userData; 
	OpcUa_StatusCode		uStatus; // Status code updated during the asynchronous load of the XML File
	CServerApplication*		pServerApplication;
	CVpiDevice*				pDevice;
	CVpiTag*				pSignal;
	OpcUa_Boolean			bNamespaceUri;
} HANDLER_SUBSYSTEM_DATA;

void xmlSubSystemStartElementHandler(
	void *userData,     /* user defined data */
	const char *name,   /* Element name */
	const char **atts)  /* Element Attribute list, provided in name value pairs */
						/* i.e. atts[0] contains name, atts[1] contains value for */
						/* atts[0], and so on...*/
{
	//CUAInformationModel* pInformationModel=CServerApplication::m_pTheAddressSpace;
	HANDLER_SUBSYSTEM_DATA* pMyData=(HANDLER_SUBSYSTEM_DATA*)userData; 

	char* cleanedName = OpcUa_Null;
	char* xmlNamespace = OpcUa_Null;
	GetCleanupXmlElt(name, &cleanedName, &xmlNamespace);
	if (OpcUa_StrCmpA(cleanedName,"SubSystems")==0)
	{
	}
	if (OpcUa_StrCmpA(cleanedName, "Uri") == 0)
	{
		pMyData->bNamespaceUri = OpcUa_True;
	}
	if (OpcUa_StrCmpA(cleanedName,"SubSystem")==0)
	{ 
		{
			CVpiDevice* pSubSystem = new CVpiDevice(atts, new CVpiFuncCaller());
			if (pSubSystem)
			{
				g_pTheApplication->AddVpiDevice(pSubSystem);
				pMyData->pDevice = pSubSystem;
			}
		}
	}
	if (OpcUa_StrCmpA(cleanedName,"Tag")==0)
	{
		CVpiTag* pSignal=new CVpiTag(atts,pMyData->pDevice);
		pMyData->pDevice->AddTag(pSignal);
		// maintenant on va chercher le Node associé a ce tag
		// Subsystem
		CUAInformationModel* pInformationModel=CServerApplication::m_pTheAddressSpace;
		CUAVariable* pUAVariable=NULL;
		OpcUa_StatusCode uStatus=pInformationModel->GetNodeIdFromVariableList(pSignal->GetNodeId(),&pUAVariable);
		if (uStatus == OpcUa_Good)
		{
			// Si le Tag contient un tableau il faudra synchroniser l'UAVariable
			// Ici on considère que pour le type de la variable ce sont le caractéristiques définit pour le Tag qui font loi
			// c'est a dire la definition dans le fichier Subsystem
			// On commence par verifier que les deux types sont identiques
			OpcUa_NodeId SignalDataType=pSignal->GetDataType();
			OpcUa_NodeId VariableDataType=pUAVariable->GetDataType();
			OpcUa_Byte builtInType = pUAVariable->GetBuiltInType();
			// Is it the special case where a Vpi contains an array of byte and the Variable a String
			if ((builtInType == OpcUaType_String) && (SignalDataType.Identifier.Numeric == OpcUaType_Byte) && (pSignal->GetNbElement()>0))
			{
				// force the Type of the UAVariable to string
				VariableDataType.IdentifierType = 0;
				VariableDataType.Identifier.Numeric = OpcUaType_String;
				pUAVariable->SetDataType(VariableDataType);
			}
			if (Utils::IsEqual(&SignalDataType,&VariableDataType )==OpcUa_False )
			{
				// Synchronisation des types
				char* szNodeId = OpcUa_Null;
				OpcUa_NodeId aNodeId = pSignal->GetNodeId();
				Utils::NodeId2String(&aNodeId, &szNodeId);
				if (szNodeId)
				{
					// NodeId
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Warning discordance detected on %s in your configuration files. Will synchronized the Node/Tag definition\n", szNodeId);
					free(szNodeId);
				}
			}
			// vérification de la taille du tableau
			if (pSignal->GetNbElement()>0)
			{
				uStatus=OpcUa_Good;
				OpcUa_Byte bBuiltInType=(OpcUa_Byte)pUAVariable->GetDataType().Identifier.Numeric; // cast to byte
				
				if (pUAVariable->GetNoOfArrayDimensions()>1)
				{
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Critical error in configuration file. Only vector are supported. Check you configuration\n");
					uStatus = OpcUa_BadInvalidArgument;
				}
				else
				{
					CDataValue* pDataValue=pUAVariable->GetValue();
					if (!pDataValue)
					{
						pUAVariable->InitializeDataValue();
						pDataValue=pUAVariable->GetValue();
					}
					if (pUAVariable->GetValueRank()!=1)
					{
						// On doit manipuler un vecteur sauf pour les String dans l'AddressSpace and Array of Byte dans le Vpi
						if ((builtInType == OpcUaType_String) && (SignalDataType.Identifier.Numeric == OpcUaType_Byte) && (pSignal->GetNbElement() > 0))
							;
						else
							pUAVariable->SetValueRank(1);						
						uStatus=pDataValue->Initialize(
							bBuiltInType,
							pUAVariable->GetValueRank(), // Must be 1
							pUAVariable->GetArrayDimensions());
						pUAVariable->SetValue(pDataValue);
					}
					else
					{
						if (pUAVariable->GetValueRank()==1)
						{
							// Verifions que le nombre d'element correspond
							std::vector<OpcUa_UInt32>* pArrayDim=pUAVariable->GetArrayDimensions();
							if (pArrayDim->size()==1)
							{
								// Check nombre element
								OpcUa_Int32 iNbElt=(OpcUa_UInt32)pArrayDim->at(0);
								if (iNbElt==pSignal->GetNbElement())
									OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Your configuration is ok\n");
								else
								{
									pArrayDim->clear();
									pArrayDim->push_back(pSignal->GetNbElement());
									uStatus=pDataValue->Initialize(
										bBuiltInType,
										pUAVariable->GetValueRank(), // Must be 1
										pArrayDim);
									pUAVariable->SetValue(pDataValue);
								}
							}
							else
							{
								// dicordance. Il faut ajouter le CDataValue
								pArrayDim->clear();
								pArrayDim->push_back(pSignal->GetNbElement());
								uStatus=pDataValue->Initialize(
									bBuiltInType,
									pUAVariable->GetValueRank(), // Must be 1
									pArrayDim);
								pUAVariable->SetValue(pDataValue);
							}
						}
					}
				}
			}
			// Il faut transferer la structure pData dans l'UAVariable
			pUAVariable->SetPData((void*)pSignal);
		}
		else
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Critical configuration error. A subsystem tag is not define in the UA AddressSpace\n");
	}
	if (cleanedName)
		free(cleanedName);
	if (xmlNamespace)
		free(xmlNamespace);
}
// Callback function for the XML file Parsing
// We end of element on a SubSystem file for Vpi definition
void xmlSubSystemEndElementHandler(
	void *userData,     /* user defined data */
	const char *name)
{
	HANDLER_SUBSYSTEM_DATA* pMyData=(HANDLER_SUBSYSTEM_DATA*)userData; 
	CUAInformationModel* pInformationModel = CServerApplication::m_pTheAddressSpace;
	char* cleanedName = OpcUa_Null;
	char* xmlNamespace = OpcUa_Null;
	GetCleanupXmlElt(name, &cleanedName, &xmlNamespace);
	if (OpcUa_StrCmpA(cleanedName, "Uri") == 0)
	{
		pMyData->bNamespaceUri = OpcUa_False;
	}
	if (OpcUa_StrCmpA(cleanedName,"SubSystem")==0)
	{
		if (pMyData)
		{
			if ( pMyData->pDevice)
			{
				CVpiDevice* pVpiDevice=pMyData->pDevice;
				if (pVpiDevice)
				{
					CVpiFuncCaller* pFuncCaller = pVpiDevice->GetVpiFuncCaller();
					if (pFuncCaller)
					{
						// will load the function pointer for the Vpi

						OpcUa_StatusCode uStatus = pFuncCaller->LoadVpi(pVpiDevice->GetAccessDataMode());
						if (uStatus == OpcUa_Good)
						{
							// GlobaStart
							OpcUa_Vpi_Handle hVpi = NULL;
							OpcUa_String vpiName = pVpiDevice->GetName();
							if (OpcUa_String_StrLen(&vpiName) > 0)
							{
								OpcUa_NodeId vpiId = pVpiDevice->GetSubSystemId();
								if (!Utils::IsNodeIdNull(vpiId))
								{
									uStatus = pFuncCaller->GlobalStart(vpiName, vpiId, &hVpi);
									if (uStatus == OpcUa_Good)//OpcUa_Vpi_Good
									{
										pFuncCaller->m_hVpi = hVpi;
										//  connect the NotifyCallback 	XMLSAXParser.dll!XML_Parse(XML_ParserStruct * parser, const char * s, int len, int isFinal) Ligne 1618	C

										if (pFuncCaller->SetNotifyCallback)
										{
											//Callback is ok. We can connect it
											uStatus = pFuncCaller->SetNotifyCallback(hVpi, (PFUNCNOTIFYCALLBACK)(VpiNotifyCallback));
										}

									}
									OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Vpi %s loaded sucessfully\n", OpcUa_String_GetRawString(pFuncCaller->GetLibraryName()));
									// Tag initialisation : ParseAddId
									CVpiTagList* pTags = pVpiDevice->GetTags();
									for (OpcUa_UInt32 iii = 0; iii < pTags->size(); iii++)
									{
										CVpiTag* pSignal = pTags->at(iii);
										hVpi = pFuncCaller->m_hVpi;
										if (hVpi)
										{
											// Check that the Tag AccessRight conform with the UAVariable AccessLevel
											CUAVariable* pUAVariable = OpcUa_Null;
											OpcUa_NodeId aNodeId = pSignal->GetNodeId();
											if (pInformationModel->GetNodeIdFromVariableList(aNodeId, &pUAVariable) == OpcUa_Good)
											{
												// 
												if ((pUAVariable->GetAccessLevel() & 0x3) == pSignal->GetAccessRight()) // 0x3 == ReadWrite we are only interest by the 2 first bits
												{
													OpcUa_String SignalAddress = pSignal->GetAddress();
													uStatus = pFuncCaller->ParseAddId(hVpi,
														pSignal->GetNodeId(),
														(OpcUa_Byte)pSignal->GetDataType().Identifier.Numeric,
														pSignal->GetNbElement(),
														(OpcUa_Byte)pSignal->GetAccessRight(),
														SignalAddress);
													if (uStatus != 0)// OpcUa_Vpi_ParseError
													{
														OpcUa_String aString = pSignal->GetAddress();
														OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Vpi %s Parse failed. Check the definition of your Tag %s uStatus=0x%05x\n",
															OpcUa_String_GetRawString(pFuncCaller->GetLibraryName()),
															OpcUa_String_GetRawString(&aString),
															uStatus);
														//OpcUa_String_Clear(&aString);
													}
												}
												else
												{
													const OpcUa_String aString = pSignal->GetAddress();
													char* szNodeId = OpcUa_Null;
													Utils::NodeId2String(&aNodeId, &szNodeId);
													if (szNodeId)
													{
														// Attribute
														OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critical error in the SubSystem configuration.NodeId %s AccessRights are not conform to the Tag %s declaration\n",
															szNodeId,
															OpcUa_String_GetRawString(&aString));
														free(szNodeId);
													}
												}
											}
											else
											{
												OpcUa_String aString = pSignal->GetAddress();
												char* szNodeId = OpcUa_Null;
												Utils::NodeId2String(&aNodeId, &szNodeId);
												if (szNodeId)
												{
													// Attribute
													OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critical error in the SubSystem configuration.NodeId %s not found TagAddress %s\n",
														szNodeId,
														OpcUa_String_GetRawString(&aString));
													free(szNodeId);
												}
												//OpcUa_String_Clear(&aString);
											}
										}
									}
									// Coldstart
									if (pFuncCaller->ColdStart(pFuncCaller->m_hVpi) == OpcUa_Good)//OpcUa_Vpi_Good
									{
										pFuncCaller->ColdStarted(OpcUa_True);
										pVpiDevice->SetInternalStatus(OpcUa_Good);
									}
								}
								else
									OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Vpi %s cannot be loaded VpiId is Null\n", OpcUa_String_GetRawString(pFuncCaller->GetLibraryName()));
							}
							else
								OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Vpi %s cannot be loaded vpiName is empty\n", OpcUa_String_GetRawString(pFuncCaller->GetLibraryName()));
						}
						else
							OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Vpi %s cannot be loaded.\n", pFuncCaller->GetLibraryName());
					}
					pMyData->pDevice = NULL;
				}
				else
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "VpiDevice is Null. Check your Vpi or its configuration\n");
			}
			else
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Critical error in the server initialization CVpiDevice corrupted\n");
		}
		else
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Critical error in the server initialization HANDLER_SUBSYSTEM_DATA corrupted\n");
	}
	if (OpcUa_StrCmpA(cleanedName,"Tag")==0)
	{
		if (pMyData)
			pMyData->pSignal=NULL;
	}
	if (cleanedName)
		free(cleanedName);
	if (xmlNamespace)
		free(xmlNamespace);
}

void xmlSubSystemCharacterDataHandler(void *userData,     /* user defined data */
	const char *s,  /* non-null terminated character string */
	int len)
{
	CUAInformationModel* pInformationModel = CServerApplication::m_pTheAddressSpace;
	HANDLER_SUBSYSTEM_DATA* pMyData = (HANDLER_SUBSYSTEM_DATA*)userData;
	OpcUa_StatusCode uStatus = OpcUa_Bad;
	if (len == 0)
		return;
	if (*s == 0xA) // newline
		return;
	if (*s == 0x9) // TAB
		return;

	if (*s == 0x3c) // <
		return;
	if (*s == 0x3e) // >
		return;
	OpcUa_CharA* buffTmp = (OpcUa_CharA*)OpcUa_Alloc(len + 1);
	ZeroMemory(buffTmp, len + 1);
	memcpy(buffTmp, s, len);
	if (Utils::IsBufferEmpty(buffTmp, len))
	{
		OpcUa_Free(buffTmp);
		return;
	}
	else
		free(buffTmp);
	// traitement des Uris
	if (pMyData->bNamespaceUri)
	{
		char* buff = (char*)OpcUa_Alloc(len + 1);
		if (buff)
		{
			ZeroMemory(buff, len + 1);
			memcpy(buff, s, len);
			if (!Utils::IsBufferEmpty(buff, len))
			{
				OpcUa_String aNamespaceUri;
				OpcUa_String_Initialize(&aNamespaceUri);
				OpcUa_String_AttachCopy(&aNamespaceUri, buff);
				uStatus = pInformationModel->OnLoadNamespaceUriVerifyUri(aNamespaceUri);
				OpcUa_String_Clear(&aNamespaceUri);
			}
			ZeroMemory(buff, len + 1);
			free(buff);
		}
		return;
	}
}
// Parsing du fichier XML
void xmlNodeSetStartElementHandler(
	void *userData,     /* user defined data */
	const char *name,   /* Element name */
	const char **atts)  /* Element Attribute list, provided in name value pairs */
						/* i.e. atts[0] contains name, atts[1] contains value for */
						/* atts[0], and so on...*/
{
	CUAInformationModel* pInformationModel=CServerApplication::m_pTheAddressSpace;
	HANDLER_DATA* pMyData=(HANDLER_DATA*)userData; 
	OpcUa_StatusCode uStatus=OpcUa_Good;
	char* cleanedName = OpcUa_Null;
	char* xmlNamespace = OpcUa_Null;
	GetCleanupXmlElt(name, &cleanedName, &xmlNamespace);
	//
	//if (pMyData->pUAVariable)
	//{
	//	if ((pMyData->pUAVariable->GetNodeId()->Identifier.Numeric == 10) && (pMyData->pUAVariable->GetNodeId()->NamespaceIndex == 2))
	//		printf("Debug purpose>ns=1;i=10(GUID)\n");
	//}
	//if (pMyData->pUAVariableType)
	//{
	//	if ( (pMyData->pUAVariableType->GetNodeId()->Identifier.Numeric==65) && (pMyData->pUAVariableType->GetNodeId()->NamespaceIndex==0) )
	//		printf("Debug purpose>i=62 \n");
	//}
	if (cleanedName)
	{
		size_t iNameLen = strlen(cleanedName);
		if (iNameLen)
		{
			if (pMyData->szCurrentXmlElement)
				OpcUa_Free(pMyData->szCurrentXmlElement);
			pMyData->szCurrentXmlElement = (char*)OpcUa_Alloc(iNameLen+1);
			ZeroMemory(pMyData->szCurrentXmlElement, iNameLen+1);
			memcpy(pMyData->szCurrentXmlElement, cleanedName, iNameLen);
		}
		if (OpcUa_StrCmpA(cleanedName,"UANodeSet")==0)
		{
		}
		if (OpcUa_StrCmpA(cleanedName,"NamespaceUris")==0)
		{
		}
		if (OpcUa_StrCmpA(cleanedName,"Uri")==0)
		{
			pMyData->bNamespaceUri=OpcUa_True;
		}
		if (OpcUa_StrCmpA(cleanedName,"ServerUris")==0)
		{
		}
		if (OpcUa_StrCmpA(cleanedName,"Aliases")==0)
		{
		}
		if (OpcUa_StrCmpA(cleanedName,"Alias")==0)
		{
			CAlias* pAlias=new CAlias(atts);
			pMyData->pAlias=pAlias;
			CAliasList* pAliasList=pInformationModel->GetAliasList();
			pAliasList->push_back(pAlias);
		}
		// On vient de trouver un element <Object>
		if (OpcUa_StrCmpA(cleanedName,"UAObject")==0)
		{
			CUAObject* pUAObject = new CUAObject(OpcUa_NodeClass_Object, atts);
			{
				//CUAObjectList* pObjectList=pInformationModel->GetObjectList();
				pInformationModel->PopulateInformationModel(pUAObject);
				pMyData->pUAObject = pUAObject;
				pMyData->NodeClass=OpcUa_NodeClass_Object;
			}
		}
		if (OpcUa_StrCmpA(cleanedName,"UAObjectType")==0)
		{
			CUAObjectType* pNewNode=new CUAObjectType(OpcUa_NodeClass_ObjectType, atts);
			{
				CUAObjectTypeList* pObjectTypeList=pInformationModel->GetObjectTypeList();
				pObjectTypeList->push_back(pNewNode);
				pMyData->pUAObjectType=pNewNode;
				pMyData->NodeClass=OpcUa_NodeClass_ObjectType;
			}
		}
		if (OpcUa_StrCmpA(cleanedName,"UADataType")==0)
		{
			CUADataType* pNewNode=new CUADataType(OpcUa_NodeClass_DataType, atts);
			{
				CUADataTypeList* pDataTypeList=pInformationModel->GetDataTypeList();
				pDataTypeList->push_back((CUADataType*)pNewNode);
				pMyData->pUADataType=pNewNode;
				pMyData->NodeClass=OpcUa_NodeClass_DataType;
			}
		}
		// Autres balise element associé aux UADataTypes
		if (OpcUa_StrCmpA(cleanedName,"Definition")==0)
		{
			if (pMyData->pUADataType)
			{
				CDefinition* pDefinition=new CDefinition(atts);
				pMyData->pDefinition=pDefinition;
				pMyData->pUADataType->SetDefinition(pDefinition);
			}
			else
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Your XML file is corrupted. on a <Definition> element\n");
		}
		if (OpcUa_StrCmpA(cleanedName,"Field")==0)
		{
			if ( (pMyData->pUADataType) && (pMyData->pDefinition) )
			{
				CField* pField=new CField(atts);
				pMyData->pDefinition->AddField(pField);
				pMyData->pField=pField;
			}
			else
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Your XML file is corrupted. on a <Field> element\n");
		}

		if (OpcUa_StrCmpA(cleanedName,"UAVariableType")==0)
		{
			CUAVariableType* pNewVariableType=new CUAVariableType(OpcUa_NodeClass_VariableType, atts);
			{
				CUAVariableTypeList* pVariableTypeList=pInformationModel->GetVariableTypeList();
				pVariableTypeList->push_back(pNewVariableType);
				pMyData->pUAVariableType=pNewVariableType;
				pMyData->NodeClass=OpcUa_NodeClass_VariableType;
			}
		}
		// On vient de trouver un element <Variable>
		if ( (OpcUa_StrCmpA(cleanedName,"UAVariable")==0) || (OpcUa_StrCmpA(cleanedName,"Property")==0) || (OpcUa_StrCmpA(cleanedName,"DataVariable")==0) )
		{ 
			pMyData->uiArrayCurrentElt=0; // initialisation de l'indice des vecteurs (Array)
			CUAVariable* pNewNode=new CUAVariable(OpcUa_NodeClass_Variable, atts);
			//CUAVariableList* pVariableList=pInformationModel->GetVariableList();
			uStatus = pInformationModel->PopulateInformationModel(pNewNode);
			if (uStatus != OpcUa_Good)
				delete pNewNode;
			else
			{
				pMyData->pUAVariable = pNewNode;
				pMyData->NodeClass = OpcUa_NodeClass_Variable;
			}
		}
		
		if (OpcUa_StrCmpA(cleanedName,"UAMethod")==0)
		{
			CUAMethod* pNewNode=new CUAMethod(OpcUa_NodeClass_Method,atts);
			CUAMethodList* pMethodList=pInformationModel->GetMethodList();
			pMethodList->push_back(pNewNode);
			pMyData->pMethod=pNewNode;
			pMyData->NodeClass=OpcUa_NodeClass_Method;
		}
		if (OpcUa_StrCmpA(cleanedName,"UAView")==0)
		{
			CUAView* pNewNode=new CUAView(OpcUa_NodeClass_View,atts);
			CUAViewList* pViewList=pInformationModel->GetViewList();
			pViewList->push_back(pNewNode);
			pMyData->pView=pNewNode;
			pMyData->NodeClass=OpcUa_NodeClass_View;
		}
		if (OpcUa_StrCmpA(cleanedName,"References")==0)
		{
		}
		//OpcUa_ReferenceNode;
		if (OpcUa_StrCmpA(cleanedName,"Reference")==0)
		{
			CUAReference* pNewReferenceNode=new CUAReference(atts);
			pMyData->pReferenceNode=pNewReferenceNode;
			// Il faut déterminer a qui appartient cette reference
			CUAObjectList* pObjectList=pInformationModel->GetObjectList();
			//int iSize=pObjectList->size();
			if (pMyData->NodeClass==OpcUa_NodeClass_Unspecified)
			{
				OpcUa_NodeId rootNodeId;
				OpcUa_NodeId_Initialize(&rootNodeId);
				rootNodeId.Identifier.Numeric = OpcUaId_RootFolder;// We will consider that the default node is Root
				CUAObjectList::iterator itOL= pObjectList->find(&rootNodeId);
				if (itOL != pObjectList->end())
				{
					CUAObject* pOPCUABase = itOL->second;
					pOPCUABase->GetReferenceNodeList()->push_back(pNewReferenceNode);
				}
				OpcUa_NodeId_Clear(&rootNodeId);
			}
			if (pMyData->NodeClass==OpcUa_NodeClass_ObjectType)
			{
				CUAObjectType* pOPCUAObjectType=pMyData->pUAObjectType;
				pOPCUAObjectType->GetReferenceNodeList()->push_back(pNewReferenceNode);
			}
			if (pMyData->NodeClass==OpcUa_NodeClass_Object)
			{
				CUAObject* pOPCUAObject=pMyData->pUAObject;
				if (!pOPCUAObject)
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Your UANodeSet file is not consistent\n");
				pOPCUAObject->GetReferenceNodeList()->push_back(pNewReferenceNode);
			}
			if (pMyData->NodeClass==OpcUa_NodeClass_ReferenceType)
			{
				CUAReferenceType* pOPCUAReferenceType=pMyData->pUAReferenceType;
				pOPCUAReferenceType->GetReferenceNodeList()->push_back(pNewReferenceNode);
			}
			if (pMyData->NodeClass==OpcUa_NodeClass_VariableType)
			{
				CUAVariableType* pOPCUAVariableType=pMyData->pUAVariableType;
				pOPCUAVariableType->GetReferenceNodeList()->push_back(pNewReferenceNode);
			}
			if (pMyData->NodeClass==OpcUa_NodeClass_Variable)
			{
				CUAVariable* pOPCUAVariable=pMyData->pUAVariable;
				pOPCUAVariable->GetReferenceNodeList()->push_back(pNewReferenceNode);
			}
			if (pMyData->NodeClass==OpcUa_NodeClass_DataType)
			{
				CUADataType* pOPCUADataType=pMyData->pUADataType;;
				pOPCUADataType->GetReferenceNodeList()->push_back(pNewReferenceNode);
			}
			if (pMyData->NodeClass==OpcUa_NodeClass_Method)
			{
				CUAMethod* pOPCUAMethod=pMyData->pMethod;
				pOPCUAMethod->GetReferenceNodeList()->push_back(pNewReferenceNode);
			}
			if (pMyData->NodeClass==OpcUa_NodeClass_View)
			{
				CUAView* pOPCUAView=pMyData->pView;
				pOPCUAView->GetReferenceNodeList()->push_back(pNewReferenceNode);
			}
		}
		// On vient de trouver un element <ReferenceType>
		if (OpcUa_StrCmpA(cleanedName,"UAReferenceType")==0)
		{
			CUAReferenceType* pNewReferenceType=new CUAReferenceType(OpcUa_NodeClass_ReferenceType, atts);
			int ii=0;
			while (atts[ii])
			{
				if (OpcUa_StrCmpA(atts[ii],"NodeId")==0)
				{
				}
				if (OpcUa_StrCmpA(atts[ii],"TargetId")==0)
				{
				}
				ii+=2;
			}
			CUAReferenceTypeList* pReferenceTypeList=CServerApplication::m_pTheAddressSpace->GetReferenceTypeList();
			pReferenceTypeList->push_back(pNewReferenceType);
			pMyData->pUAReferenceType=pNewReferenceType;
			pMyData->NodeClass=OpcUa_NodeClass_ReferenceType;
		}
		// On vient de trouver un element <Value> qui correspond à une sequence d'init et pas à un EnumValueType
		if (OpcUa_StrCmpA(cleanedName,"Value")==0)
		{
			if (pMyData->NodeClass==OpcUa_NodeClass_Variable)
			{
				if (pMyData->bExtensionObjectEnumValueType)
				{
					// ok il s'agit de la valeur d'un EnumValueType
					pMyData->bExtensionObjectEnumValueTypeValue=OpcUa_True;
				}
				else
				{
					//if (pMyData->pUAVariable)
					//{
					//	if ( (pMyData->pUAVariable->GetNodeId().Identifier.Numeric==12169) && (pMyData->pUAVariable->GetNodeId().NamespaceIndex==0) )
					//		printf("Debug purpose on ns=0;i=12169\n");
					//}
					// autre voie d'initialisation... On a trouvé une balise <Value> dans le fichier XML
					// Attention il faut prendre en cas l'init depuis le fichier XML de type non encore initialisé. fucking Randy !!!
					CUADataType* pUADataType=OpcUa_Null;
					OpcUa_Byte builtInType=pMyData->pUAVariable->GetBuiltInType();
					if (builtInType==0)
					{
						OpcUa_NodeId aNodeId=pMyData->pUAVariable->GetDataType();
						uStatus=pInformationModel->GetNodeIdFromDataTypeList(aNodeId,&pUADataType);
						if (uStatus==OpcUa_Good)
						{
							// We are looking for the SuperType of the DataType. (pUADataType)
							// This should be as inverse reference HasSubtype Target Id will contains the supertype
							//OpcUa_NodeId aNodeIdParent=pUADataType->GetParentType();
							// Warning. We have to check the the Parent is in the field 1 to 25
							builtInType=(OpcUa_Byte)pUADataType->GetAncestorType();
						}
						else
						{
							char* szNodeId = OpcUa_Null;
							Utils::NodeId2String(&aNodeId,&szNodeId);
							pMyData->uStatus = OpcUa_BadBrowseNameInvalid;
							if (szNodeId)
							{
								OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "xmlNodeSetStartElementHandler>Critical error. Cannot find %s in DatatypeList\n", szNodeId);
								OpcUa_Free(szNodeId);
							}
						}
						OpcUa_NodeId aNodeId2;
						//Argument
						aNodeId2.IdentifierType=OpcUa_IdentifierType_Numeric;
						aNodeId2.NamespaceIndex=0;
						aNodeId2.Identifier.Numeric=296; 
						if (Utils::IsEqual(&aNodeId,&aNodeId2))
							builtInType=22; // Structure
						aNodeId2.Identifier.Numeric=7594; 
						if (Utils::IsEqual(&aNodeId,&aNodeId2))
							builtInType=22; // Structure
						aNodeId2.Identifier.Numeric=120; 
						if (Utils::IsEqual(&aNodeId,&aNodeId2))
							builtInType=29; // Enumeration
						aNodeId2.Identifier.Numeric=290; // Duration
						if (Utils::IsEqual(&aNodeId,&aNodeId2))
							builtInType=11; // double
						aNodeId2.Identifier.Numeric=292; // Time
						if (Utils::IsEqual(&aNodeId,&aNodeId2))
							builtInType=12; // String
						aNodeId2.Identifier.Numeric=294; // UtcTime
						if (Utils::IsEqual(&aNodeId,&aNodeId2))
							builtInType=13; // DateTime
						aNodeId2.Identifier.Numeric=295; // LocaleId
						if (Utils::IsEqual(&aNodeId,&aNodeId2))
							builtInType=12; // String
					}
					if (builtInType==0)
					{
						OpcUa_NodeId aNodeId=pMyData->pUAVariable->GetDataType();
						pUADataType = OpcUa_Null;
						uStatus=pInformationModel->GetNodeIdFromDataTypeList(aNodeId,&pUADataType);
						if (uStatus==OpcUa_Good)
						{
							// We are looking for the SuperType of the DataType. (pUADataType)
							// This should be as inverse reference HasSubtype Target Id will contains the supertype
						}
						
						char* szNodeId = OpcUa_Null;
						Utils::NodeId2String(&aNodeId,&szNodeId);
						if (szNodeId)
						{
							OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"xmlNodeSetStartElementHandler>Critical builtInType Error. DataType=%s.\n",szNodeId);
							if ((aNodeId.IdentifierType == OpcUa_IdentifierType_Numeric)
								&& (aNodeId.Identifier.Numeric == 24)
								&& (aNodeId.NamespaceIndex == 0))
								OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critical error. BaseDataType is an Abstract Datatype. You cannot instanciate it. Fix your NodeSet file\n");
							free(szNodeId);
						}
					}
					else
					{
						//if (pMyData->pUAVariable)
						//{
						//	if ( (pMyData->pUAVariable->GetNodeId().Identifier.Numeric==12169) && (pMyData->pUAVariable->GetNodeId().NamespaceIndex==0) )
						//		printf("Debug purpose on ns=0;i=12169\n");
						//}
						pMyData->pDataValue=new CDataValue();
						pMyData->pUAVariable->SetBuiltInType(builtInType);
						pMyData->pDataValue->Initialize(builtInType,
											   pMyData->pUAVariable->GetValueRank(),
											   pMyData->pUAVariable->GetArrayDimensions());
						
						pMyData->bValue=OpcUa_True;
					}
				}
			}
			else
			{
				pMyData->uStatus = OpcUa_BadInvalidArgument;
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Please respect the UANodeSet.xsd schema\n");
			}
		}
		if (OpcUa_StrCmpA(cleanedName,"Description")==0)
		{
			if (pMyData->bExtensionObjectBody)
			{
				if (pMyData->bExtensionObjectEnumValueType)
					pMyData->bExtensionObjectEnumValueTypeDescription=OpcUa_True;
				else
					pMyData->bExtensionObjectDescription=OpcUa_True;
			}
			else
				pMyData->bDescription=OpcUa_True;
		}
		if (OpcUa_StrCmpA(cleanedName,"DisplayName")==0)
		{
			if (pMyData->bExtensionObjectEnumValueType)
				pMyData->bExtensionObjectEnumValueTypeDisplayName=OpcUa_True;
			else
				pMyData->bDisplayName=OpcUa_True;
		}
		if (OpcUa_StrCmpA(cleanedName,"InverseName")==0)
		{
			pMyData->bInverseName=OpcUa_True;
		}
		////////////////////////////////////////////////////////////////////
		// Type de données pouvant être encaspulé dans un UAVariable
		// il s'agit ici du processus d'initialisation à partir du fichier XML
		if (pMyData->bValue)
		{
			if (OpcUa_StrCmpA(cleanedName,"Boolean")==0)
				pMyData->pDataValue->SetBuiltInDataType(OpcUaType_Boolean);
			if (OpcUa_StrCmpA(cleanedName,"ListOfBoolean")==0)
			{
				pMyData->pDataValue->SetArrayType(OpcUa_VariantArrayType_Array);
				pMyData->pDataValue->SetBuiltInDataType(OpcUaType_Boolean);
			}
			if (OpcUa_StrCmpA(cleanedName,"Byte")==0)
				pMyData->pDataValue->SetBuiltInDataType(OpcUaType_Byte);
			if (OpcUa_StrCmpA(cleanedName,"ListOfByte")==0)
			{
				pMyData->pDataValue->SetArrayType(OpcUa_VariantArrayType_Array);
				pMyData->pDataValue->SetBuiltInDataType(OpcUaType_Byte);
			}
			if (OpcUa_StrCmpA(cleanedName,"SByte")==0)
				pMyData->pDataValue->SetBuiltInDataType(OpcUaType_SByte);
			if (OpcUa_StrCmpA(cleanedName,"ListOfSByte")==0)
			{
				pMyData->pDataValue->SetArrayType(OpcUa_VariantArrayType_Array);
				pMyData->pDataValue->SetBuiltInDataType(OpcUaType_SByte);
			}
			if (OpcUa_StrCmpA(cleanedName,"Int16")==0)
				pMyData->pDataValue->SetBuiltInDataType(OpcUaType_Int16);
			if (OpcUa_StrCmpA(cleanedName,"ListOfInt16")==0)
			{
				pMyData->pDataValue->SetArrayType(OpcUa_VariantArrayType_Array);
				pMyData->pDataValue->SetBuiltInDataType(OpcUaType_Int16);
			}
			if (OpcUa_StrCmpA(cleanedName,"UInt16")==0)
				pMyData->pDataValue->SetBuiltInDataType(OpcUaType_UInt16);
			if (OpcUa_StrCmpA(cleanedName,"ListOfUInt16")==0)
			{
				pMyData->pDataValue->SetArrayType(OpcUa_VariantArrayType_Array);
				pMyData->pDataValue->SetBuiltInDataType(OpcUaType_UInt16);
			}
			if (OpcUa_StrCmpA(cleanedName,"Int32")==0)
				pMyData->pDataValue->SetBuiltInDataType(OpcUaType_Int32);
			if (OpcUa_StrCmpA(cleanedName,"ListOfInt32")==0)
			{
				pMyData->pDataValue->SetArrayType(OpcUa_VariantArrayType_Array);
				pMyData->pDataValue->SetBuiltInDataType(OpcUaType_Int32);
			}
			if (OpcUa_StrCmpA(cleanedName,"UInt32")==0)
				pMyData->pDataValue->SetBuiltInDataType(OpcUaType_UInt32);
			if (OpcUa_StrCmpA(cleanedName,"ListOfUInt32")==0)
			{
				pMyData->pDataValue->SetArrayType(OpcUa_VariantArrayType_Array);
				pMyData->pDataValue->SetBuiltInDataType(OpcUaType_UInt32);
			}
			if (OpcUa_StrCmpA(cleanedName, "Integer") == 0)
				pMyData->pDataValue->SetBuiltInDataType(OpcUaType_Int32);
			if (OpcUa_StrCmpA(cleanedName, "ListOfInteger") == 0)
			{
				pMyData->pDataValue->SetArrayType(OpcUa_VariantArrayType_Array);
				pMyData->pDataValue->SetBuiltInDataType(OpcUaType_Int32);
			}
			if (OpcUa_StrCmpA(cleanedName, "UInteger") == 0)
				pMyData->pDataValue->SetBuiltInDataType(OpcUaType_UInt32);
			if (OpcUa_StrCmpA(cleanedName, "ListOfUInteger") == 0)
			{
				pMyData->pDataValue->SetArrayType(OpcUa_VariantArrayType_Array);
				pMyData->pDataValue->SetBuiltInDataType(OpcUaType_UInt32);
			}
			if (OpcUa_StrCmpA(cleanedName,"Int64")==0)
				pMyData->pDataValue->SetBuiltInDataType(OpcUaType_Int64);
			if (OpcUa_StrCmpA(cleanedName,"ListOfInt64")==0)
			{
				pMyData->pDataValue->SetArrayType(OpcUa_VariantArrayType_Array);
				pMyData->pDataValue->SetBuiltInDataType(OpcUaType_Int64);
			}
			if (OpcUa_StrCmpA(cleanedName,"UInt64")==0)
				pMyData->pDataValue->SetBuiltInDataType(OpcUaType_UInt64);
			if (OpcUa_StrCmpA(cleanedName,"ListOfUInt64")==0)
			{
				pMyData->pDataValue->SetArrayType(OpcUa_VariantArrayType_Array);
				pMyData->pDataValue->SetBuiltInDataType(OpcUaType_UInt64);
			}
			if (OpcUa_StrCmpA(cleanedName, "Float") == 0)
				pMyData->pDataValue->SetBuiltInDataType(OpcUaType_Float);
			if (OpcUa_StrCmpA(cleanedName, "ListOfFloat") == 0)
			{
				pMyData->pDataValue->SetArrayType(OpcUa_VariantArrayType_Array);
				pMyData->pDataValue->SetBuiltInDataType(OpcUaType_Float);
			}
			if (OpcUa_StrCmpA(cleanedName, "Double") == 0)
				pMyData->pDataValue->SetBuiltInDataType(OpcUaType_Double);
			if (OpcUa_StrCmpA(cleanedName, "ListOfDouble") == 0)
			{
				pMyData->pDataValue->SetArrayType(OpcUa_VariantArrayType_Array);
				pMyData->pDataValue->SetBuiltInDataType(OpcUaType_Double);
			}
			if ( (OpcUa_StrCmpA(cleanedName,"DateTime")==0) || (OpcUa_StrCmpA(cleanedName,"UtcTime")==0) )
			{
				pMyData->pDataValue->SetBuiltInDataType(OpcUaType_DateTime);
			}		
			if (OpcUa_StrCmpA(cleanedName,"ListOfDateTime")==0)
			{
				pMyData->pDataValue->SetArrayType(OpcUa_VariantArrayType_Array);
				pMyData->pDataValue->SetBuiltInDataType(OpcUaType_DateTime);
			}
			if ( (OpcUa_StrCmpA(cleanedName,"String")==0)|| (OpcUa_StrCmpA(cleanedName,"Time")==0) ) // Le Type Time est une chaine normal au format HH:MM:SS.SSS
			{
				pMyData->pDataValue->SetBuiltInDataType(OpcUaType_String);
			}
			if (OpcUa_StrCmpA(cleanedName,"ListOfString")==0)
			{
				pMyData->pDataValue->SetArrayType(OpcUa_VariantArrayType_Array);
				pMyData->pDataValue->SetBuiltInDataType(OpcUaType_String);
			}
			if ((OpcUa_StrCmpA(cleanedName, "NodeId") == 0) ) // Le Type Time est une chaine normal au format HH:MM:SS.SSS
			{
				pMyData->pDataValue->SetBuiltInDataType(OpcUaType_NodeId);
			}
			if (OpcUa_StrCmpA(cleanedName, "ListOfNodeId") == 0)
			{
				pMyData->pDataValue->SetArrayType(OpcUa_VariantArrayType_Array);
				pMyData->pDataValue->SetBuiltInDataType(OpcUaType_NodeId);
			}
			if (OpcUa_StrCmpA(cleanedName,"ListOfLocalizedText")==0)
			{
				pMyData->pDataValue->SetArrayType(OpcUa_VariantArrayType_Array);
				pMyData->pDataValue->SetBuiltInDataType(OpcUaType_LocalizedText);
			}
			if (OpcUa_StrCmpA(cleanedName,"LocalizedText")==0)
				pMyData->pDataValue->SetBuiltInDataType(OpcUaType_LocalizedText);

			if (OpcUa_StrCmpA(cleanedName,"ByteString")==0)
			{
				pMyData->pDataValue->SetBuiltInDataType(OpcUaType_ByteString);

			}
			if (OpcUa_StrCmpA(cleanedName,"ListOfByteString")==0)
			{
				pMyData->pDataValue->SetArrayType(OpcUa_VariantArrayType_Array);
				pMyData->pDataValue->SetBuiltInDataType(OpcUaType_ByteString);
			}
			if (OpcUa_StrCmpA(cleanedName,"Guid")==0)
			{
				pMyData->pDataValue->SetBuiltInDataType(OpcUaType_Guid);
			}
			if (OpcUa_StrCmpA(cleanedName,"ListOfGuid")==0)
			{
				pMyData->pDataValue->SetArrayType(OpcUa_VariantArrayType_Array);
				pMyData->pDataValue->SetBuiltInDataType(OpcUaType_Guid);
			}
			if (OpcUa_StrCmpA(cleanedName,"Text")==0)
			{
				pMyData->bText=OpcUa_True;
			}
			if (OpcUa_StrCmpA(cleanedName,"Locale")==0)
			{
				pMyData->bLocale=OpcUa_True;
			}
			/////////////////////////////////////////////////////////////////////////////////////
			// Prise en charge de l'ExtensionObject
			if ((OpcUa_StrCmpA(cleanedName, "uax:ExtensionObject") == 0) || (OpcUa_StrCmpA(cleanedName, "ExtensionObject") == 0))
			{
				pMyData->bExtensionObject=OpcUa_True;
				pMyData->pDataValue->SetBuiltInDataType(OpcUaType_ExtensionObject);

			}
			if ((OpcUa_StrCmpA(cleanedName, "uax:ListOfExtensionObject") == 0) || (OpcUa_StrCmpA(cleanedName, "ListOfExtensionObject") == 0) )
			{
				pMyData->pDataValue->SetArrayType(OpcUa_VariantArrayType_Array);
				pMyData->pDataValue->SetBuiltInDataType(OpcUaType_ExtensionObject);
			}
		}
		////////////////////////////////////////////////////////////////////////////
		// Prise en charge de balise element dédié a l'ExtensionObject
		if ( (OpcUa_StrCmpA(cleanedName,"uax:TypeId")==0) || (OpcUa_StrCmpA(cleanedName,"TypeId")==0) ) //OpcUa_ExpandedNodeId TypeId;
		{
			if ( (pMyData->pUAVariable) && (pMyData->pDataValue) && (pMyData->bValue) )
			{
				pMyData->bExtensionObjectTypeId=OpcUa_True;
			}
			else
			{
				pMyData->uStatus = OpcUa_BadSyntaxError;
				if (pMyData->pUAVariable)
				{
					char* szNodeId = OpcUa_Null;
					// NodeId
					OpcUa_NodeId* pNodeId = pMyData->pUAVariable->GetNodeId();
					Utils::NodeId2String(pNodeId, &szNodeId);
					if (szNodeId)
					{
						// Attribute
						OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "xmlNodeSetStartElementHandler> XML file corrupted. the Element TypeId: %s is define outside of UAVariable.\n", szNodeId);					
						free(szNodeId);
					}
				}
				else
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"xmlNodeSetStartElementHandler> XML file corrupted. An Element TypeId is define outside of UAVariable.\n");
			}			
		}
		if ( (OpcUa_StrCmpA(cleanedName,"uax:Body")==0) || (OpcUa_StrCmpA(cleanedName,"Body")==0))//L'element Body peut contenir n'importe quel type défini dans Opc.Ua.Types.xsd
		{
			pMyData->bExtensionObjectBody=OpcUa_True;
		}
		if ( (OpcUa_StrCmpA(cleanedName,"uax:EnumValueType")==0) || (OpcUa_StrCmpA(cleanedName,"EnumValueType")==0) )//L'element Body peut contenir n'importe quel type défini dans Opc.Ua.Types.xsd
		{
			pMyData->bExtensionObjectEnumValueType=OpcUa_True;
			pMyData->pEnumValueType=(OpcUa_EnumValueType*)OpcUa_Alloc(sizeof(OpcUa_EnumValueType));
			OpcUa_EnumValueType_Initialize(pMyData->pEnumValueType);
		}
		if ( (OpcUa_StrCmpA(cleanedName,"uax:Argument")==0) || (OpcUa_StrCmpA(cleanedName,"Argument")==0))//L'element Body peut contenir n'importe quel type défini dans Opc.Ua.Types.xsd
		{
			// L'extensionObject contient 
			pMyData->bExtensionObjectArgument=OpcUa_True;
			pMyData->pArgument=(OpcUa_Argument*)OpcUa_Alloc(sizeof(OpcUa_Argument));
			OpcUa_Argument_Initialize(pMyData->pArgument);
			// On va mettre un paramètre dans Description une valeur par défaut
			OpcUa_String_AttachCopy(&(pMyData->pArgument->Description.Text),"Description missing");
			OpcUa_String_AttachCopy(&(pMyData->pArgument->Description.Locale),"en-us");
		}
		if ( (OpcUa_StrCmpA(cleanedName,"uax:Name")==0) || (OpcUa_StrCmpA(cleanedName,"Name")==0) )//L'element Body peut contenir n'importe quel type défini dans Opc.Ua.Types.xsd
		{
			if (pMyData->pArgument)
				pMyData->bExtensionObjectName=OpcUa_True;
		}
		if ( (OpcUa_StrCmpA(cleanedName,"uax:DataType")==0) || (OpcUa_StrCmpA(cleanedName,"DataType")==0))//L'element Body peut contenir n'importe quel type défini dans Opc.Ua.Types.xsd
		{
			pMyData->bExtensionObjectDataType=OpcUa_True;
		}
		if ((OpcUa_StrCmpA(cleanedName,"uax:Identifier")==0) || (OpcUa_StrCmpA(cleanedName,"Identifier")==0))//L'element Body peut contenir n'importe quel type défini dans Opc.Ua.Types.xsd
		{
			pMyData->bExtensionObjectIdentifier=OpcUa_True;
		}
		if ((OpcUa_StrCmpA(cleanedName,"uax:ValueRank")==0) ||(OpcUa_StrCmpA(cleanedName,"ValueRank")==0))//L'element Body peut contenir n'importe quel type défini dans Opc.Ua.Types.xsd
		{
			pMyData->bExtensionObjectValueRank=OpcUa_True;
		}
		if ( (OpcUa_StrCmpA(cleanedName,"uax:ArrayDimensions")==0) || (OpcUa_StrCmpA(cleanedName,"ArrayDimensions")==0))//L'element Body peut contenir n'importe quel type défini dans Opc.Ua.Types.xsd
		{
			pMyData->bExtensionObjectArrayDimensions=OpcUa_True;
		}

		//if (OpcUa_StrCmpA(cleanedName,"Description")==0)//L'element Body peut contenir n'importe quel type défini dans Opc.Ua.Types.xsd
		//{
		//	pMyData->bExtensionObjectDescription=OpcUa_True;
		//}

		// fin de la prise en charge de balise element dédié a l'ExtensionObject
		/////////////////////////////////////////////////////////////////////////////////////
	}
	else
	{
		pMyData->uStatus = OpcUa_BadSyntaxError;
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Null StartElement\n");
	}
	if (cleanedName)
		free(cleanedName);
	if (xmlNamespace)
		free(xmlNamespace);
}

// Parsing du fichier XML
void xmlNodeSetEndElementHandler( 
	void *userData,     /* user defined data */
	const char *name)/* Element name */
{
	HANDLER_DATA* pMyData=(HANDLER_DATA*)userData; 
	CUAInformationModel* pInformationModel=CServerApplication::m_pTheAddressSpace;
	char* cleanedName = OpcUa_Null;
	char* xmlNamespace = OpcUa_Null;
	GetCleanupXmlElt(name, &cleanedName, &xmlNamespace);
	//if (pMyData->pUAVariable)
	//{
	//	if ( (pMyData->pUAVariable->GetNodeId().Identifier.Numeric==11433) && (pMyData->pUAVariable->GetNodeId().NamespaceIndex==0) )
	//		printf("Debug purpose>11433\n");
	//}
	if (pMyData->szCurrentXmlElement)
	{
		OpcUa_Free(pMyData->szCurrentXmlElement);
		pMyData->szCurrentXmlElement = OpcUa_Null;
	}
	if (cleanedName)
	{
		//	OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"xmlNodeSetEndElementHandler>%s\n",cleanedName);
		if (OpcUa_StrCmpA(cleanedName,"UANodeSet")==0)
		{
		}
		if (OpcUa_StrCmpA(cleanedName,"NamespaceUris")==0)
		{
		}
		if (OpcUa_StrCmpA(cleanedName,"Uri")==0)
		{
			pMyData->bNamespaceUri=OpcUa_False;
		}
		if (OpcUa_StrCmpA(cleanedName,"ServerUris")==0)
		{
		}
		if (OpcUa_StrCmpA(cleanedName,"Aliases")==0)
		{
		}
		if (OpcUa_StrCmpA(cleanedName,"Alias")==0)
		{
			pMyData->pAlias=NULL;
		}
		if (OpcUa_StrCmpA(cleanedName,"UAObjectType")==0)
		{
			pMyData->pUAObjectType=OpcUa_Null;
			pMyData->NodeClass = OpcUa_NodeClass_Unspecified;
		}
		if (OpcUa_StrCmpA(cleanedName,"UAObject")==0)
		{
			pMyData->pUAObject=NULL;
			pMyData->NodeClass = OpcUa_NodeClass_Unspecified;
		}
		if (OpcUa_StrCmpA(cleanedName,"UADataType")==0)
		{
			// update The ParentDataType
			OpcUa_StatusCode uStatus=pMyData->pUADataType->UpdateParentType();
			if (uStatus!=OpcUa_Good)
			{
				OpcUa_NodeId* pNodeId=pMyData->pUADataType->GetNodeId();
				if ((pNodeId->IdentifierType==OpcUa_IdentifierType_Numeric)&&(pNodeId->Identifier.Numeric<=29) )
				{
					// BuiltInType
				}
				else
				{
					char* szNodeId = OpcUa_Null;
					Utils::NodeId2String(pNodeId,&szNodeId);
					if (szNodeId)
					{
						OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "xmlNodeSetEndElementHandler CriticalError>The declaration of your new UADataType %s is incorrect. HasSubType Reference is missing or orphan declaration.\n", szNodeId);
						pMyData->uStatus = OpcUa_BadSyntaxError;
						free(szNodeId);
					}
				}
			}
			else
			{
				// Will check that the parent allows to initialize the AncestorType.
				CUADataType* pUADataTypeAncestor=pMyData->pUADataType;
				if (pUADataTypeAncestor)
				{
					OpcUa_Byte bBuiltInType=pUADataTypeAncestor->GetAncestorType();
					uStatus=OpcUa_Good;
					while ( (!bBuiltInType) && (uStatus==OpcUa_Good) )
					{
						OpcUa_UInt32 iType=pUADataTypeAncestor->GetParentType().Identifier.Numeric;
						if ( (pUADataTypeAncestor->GetParentType().IdentifierType==OpcUa_IdentifierType_Numeric) && (iType>0) && (iType<=29) )
						{
							pMyData->pUADataType->SetAncestorType((OpcUa_Byte)iType);
							bBuiltInType=(OpcUa_Byte)iType;
						}
						else
						{
							// Look for the AncestorType
							OpcUa_NodeId aNodeId=pUADataTypeAncestor->GetParentType();
							uStatus=pInformationModel->GetNodeIdFromDataTypeList(aNodeId,&pUADataTypeAncestor);
						}
					}
					if (bBuiltInType==0)
					{
						char* szNodeId = OpcUa_Null;
						OpcUa_NodeId* pNodeId = pMyData->pUADataType->GetNodeId();
						Utils::NodeId2String(pNodeId,&szNodeId);
						if (szNodeId)
						{
							pMyData->uStatus = OpcUa_BadReferenceTypeIdInvalid;
							OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_WARNING, "xmlNodeSetEndElementHandler>Warning NodeSet inconsistancy cannot retrieve the AncestorType (BuiltIn) for your new UADataType %s.\n", szNodeId);
							free(szNodeId);
						}
					}
				}
				else
				{
					pMyData->uStatus = OpcUa_BadSyntaxError;
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "xmlNodeSetEndElementHandler>Critical  NodeSet error there no AncestorType defines for the currentType\n");
				}
			}
			pMyData->pUADataType=OpcUa_Null;		
			pMyData->NodeClass = OpcUa_NodeClass_Unspecified;
		}
		if (OpcUa_StrCmpA(cleanedName,"Definition")==0)
			pMyData->pDefinition=NULL;
		if (OpcUa_StrCmpA(cleanedName,"Field")==0)
			pMyData->pField=NULL;
		if ( (OpcUa_StrCmpA(cleanedName,"UAVariable") == 0) || (OpcUa_StrCmpA(cleanedName, "Property") == 0) || (OpcUa_StrCmpA(cleanedName, "DataVariable") == 0) )
		{
			// premier cas 		
			// On a à faire à un UAVariable qui n'a pas été initialisé dans le fichier XML		
			// il faut procéder à une initialisation minimum pour que les clients puissent lire cette UAVariable
			if ( (pMyData->pUAVariable) && (pMyData->pDataValue==OpcUa_Null ))
			{
				OpcUa_Byte BuiltInType = pMyData->pUAVariable->GetBuiltInType();
				switch (BuiltInType)
				{
				case OpcUaType_Boolean:
				case OpcUaType_Byte:
				case OpcUaType_LocalizedText:
				case OpcUaType_QualifiedName:
				case OpcUaType_UInt16:
				case OpcUaType_UInt32:
				case OpcUaType_UInt64:
				case OpcUaType_Int16:
				case OpcUaType_Int32:
				case OpcUaType_Int64:
				case OpcUaType_Double:
				case OpcUaType_Float:
				case OpcUaType_NodeId:
				case OpcUaType_ByteString:
				case OpcUaType_StatusCode:
				case OpcUaType_DataValue:
				case OpcUaType_DateTime:
				case OpcUaType_String:
				case OpcUaType_Guid:
					{
						CDataValue* pDataValue = OpcUa_Null;
						if (pMyData->pUAVariable->GetValueRank()==0)
							pDataValue = new CDataValue(BuiltInType);
						else
						{
							OpcUa_UInt32 iRank = pMyData->pUAVariable->GetValueRank();
							std::vector<OpcUa_UInt32>* pArrayDim=pMyData->pUAVariable->GetArrayDimensions();
							pDataValue = new CDataValue();
							pDataValue->Initialize(BuiltInType, iRank,pArrayDim);
						}
						pMyData->pUAVariable->SetValue(pDataValue);
						pMyData->uStatus = OpcUa_Good; //OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Odd case\n");
						delete pDataValue; // modif mai 2015
					}
					break;
				case  OpcUaType_Null:
					{
						OpcUa_NodeId aTypeId = pMyData->pUAVariable->GetDataType();
						if (FindBuiltinType(aTypeId, &BuiltInType) == OpcUa_Good)
						{
							CDataValue* pDataValue = OpcUa_Null;
							if (pMyData->pUAVariable->GetValueRank() == 0)
								pDataValue = new CDataValue(BuiltInType);
							else
							{
								OpcUa_UInt32 iRank = pMyData->pUAVariable->GetValueRank();
								std::vector<OpcUa_UInt32>* pArrayDim = pMyData->pUAVariable->GetArrayDimensions();
								pDataValue = new CDataValue();
								pDataValue->Initialize(BuiltInType, iRank, pArrayDim);
							}
							pMyData->pUAVariable->SetBuiltInType(BuiltInType);
							pMyData->pUAVariable->SetValue(pDataValue);
							pMyData->uStatus = OpcUa_Good;
							delete pDataValue;
						}
						else
						{
							char* szNodeId = OpcUa_Null;
							OpcUa_NodeId* pNodeId = pMyData->pUAVariable->GetNodeId();
							Utils::NodeId2String(pNodeId, &szNodeId);
							if (szNodeId)
							{
								char* szDataType = OpcUa_Null;
								OpcUa_NodeId aNodeId = pMyData->pUAVariable->GetDataType();
								Utils::NodeId2String(&aNodeId, &szDataType);
								if (szDataType)
								{
									// NodeId
									OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "xmlNodeSetEndElementHandler detect an error in your nodeset file. Declare your DataType %s before using it. BuiltInType=%u NodeId=%s.\n", szDataType, BuiltInType, szNodeId);									
									free(szDataType);
								}
								free(szNodeId);
							}
						}
					}
					break;
				default:
					CDataValue* pDataValue = new CDataValue(BuiltInType);
					pMyData->pUAVariable->SetValue(pDataValue);
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "xmlNodeSetEndElementHandler Odd case workarounded BuiltInType=%u.\n", BuiltInType);
					pMyData->uStatus = OpcUa_Good; //OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Odd case\n");
					break;
				}
			}
			// deuxième cas 
			// on a à faire à une UAVaribale qui a été initialisé pendant la lecture du fichier XML
			// il reste à finaliser cette initialisation
			else
			{
				// traitement EnabledFlag
				if ( (pMyData->pUAVariable->GetNodeId()->Identifier.Numeric==2294) && (pMyData->pUAVariable->GetNodeId()->NamespaceIndex==0) )
				{
					// i=2294=EnabledFlag
					OpcUa_Variant valVal=pMyData->pDataValue->GetValue();
					pInformationModel->EnableServerDiagnosticsDefaultValue(valVal.Value.Boolean);

				}
				//if ( (pMyData->pUAVariable->GetNodeId()->Identifier.Numeric==109) && (pMyData->pUAVariable->GetNodeId()->NamespaceIndex==2) )
				//	printf("CTT debug purpose ns=2;i=109\n");
				// Initialisation de quelques nodes speciales
				if ( (pMyData->pUAVariable) && (pMyData->pDataValue) )
				{
					pMyData->pDataValue->SetStatusCode(OpcUa_Good);
					OpcUa_Boolean bInitValid=OpcUa_False;
					OpcUa_NodeId aDataType=pMyData->pUAVariable->GetDataType();
					CUADataType* pUADataType=OpcUa_Null;
					OpcUa_StatusCode uStatus=pInformationModel->GetNodeIdFromDataTypeList(aDataType,&pUADataType);
					if (uStatus!=OpcUa_Good)
					{
						if (pUADataType)
						{
							if (!pUADataType->IsAbstract())
								bInitValid=OpcUa_True;
						}
						else
						{
							char* szNodeId = OpcUa_Null;
							Utils::NodeId2String(&aDataType,&szNodeId);
							if (szNodeId)
							{
								pMyData->uStatus = OpcUa_BadInternalError;
								OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "xmlNodeSetEndElementHandler>GetNodeIdFromDataTypeList failed on NodeId %s\n", szNodeId);
								free(szNodeId);
							}
						}
					}
					if (pMyData->pDataValue->GetBuiltInDataType() == pMyData->pUAVariable->GetBuiltInType())
					{
						if ((pMyData->pUAVariable->GetValueRank() == -1) && (pMyData->pDataValue->GetArrayType() != 0))
						{
							OpcUa_NodeId* pNodeId = OpcUa_Null;
							if (GetNodeIdForCurrentParsedObject(pMyData, &pNodeId) == OpcUa_Good)
							{
								char* szNodeId = OpcUa_Null;
								Utils::NodeId2String(pNodeId, &szNodeId);
								if (szNodeId)
								{
									pMyData->uStatus = OpcUa_BadInternalError;
									OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "The ValueRank in the NodeSet in node compatiple with the Value declaration %s\n",szNodeId);
									free(szNodeId);
								}
								OpcUa_Free(pNodeId);
							}
							else
							{
								pMyData->uStatus = OpcUa_BadInternalError;
								OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "The ValueRank in the NodeSet in node compatiple with the Value declaration\n");
							}		
							pMyData->pDataValue->SetArrayType(OpcUa_VariantArrayType_Scalar);
						}
						else
						{
							bInitValid = OpcUa_True;
							if (pMyData->pDataValue->GetBuiltInDataType() == OpcUaType_ExtensionObject)
							{
								if (pMyData->pDataValue->GetArrayType() == OpcUa_VariantArrayType_Array)
								{
									for (OpcUa_UInt16 ii = 0; ii < pMyData->pDataValue->GetArraySize(); ii++)
									{
										OpcUa_ExtensionObject* pExtensionObject = pMyData->pDataValue->GetInternalExtensionObjectArray(ii);
										if (pExtensionObject)
										{
											OpcUa_NodeId aNodeId = pMyData->pUAVariable->GetDataType();
											if (Utils::IsNodeIdNull(pExtensionObject->TypeId.NodeId))
												OpcUa_NodeId_CopyTo(&aNodeId, &(pExtensionObject->TypeId.NodeId));
										}
										else
										{
											char* szNodeId = OpcUa_Null;
											OpcUa_NodeId* pNodeId = pMyData->pUAVariable->GetNodeId();
											Utils::NodeId2String(pNodeId, &szNodeId);
											if (szNodeId)
											{
												// Attribute
												pMyData->uStatus = OpcUa_BadInternalError;
												OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "xmlNodeSetEndElementHandler>Critical error during initialization pExtensionObject is NULL. NodeId=%s\n", szNodeId);
												free(szNodeId);
											}
										}
									}
								}
								//else
								//{
								//	OpcUa_ExtensionObject* pExtensionObject = pMyData->pDataValue->GetInternalExtensionObject();
								//}
							}
						}
					}
					else
					{
						// Exception Number, Integer and UInteger, Enumeration
						switch (pMyData->pUAVariable->GetBuiltInType())
						{
						case 26:
						case 27:
						case 28:
						case 29: // Enumeration
							bInitValid = OpcUa_True;
							break;
						default:
							break;
						}
					}
					if (bInitValid)
					{
						OpcUa_DataValue* pInternalDataValue=pMyData->pDataValue->GetInternalDataValue();
						pMyData->pUAVariable->SetValue(pInternalDataValue);
						delete pMyData->pDataValue;
						pMyData->pDataValue = OpcUa_Null;
					}
					else
					{
						delete pMyData->pDataValue;
						pMyData->pDataValue = OpcUa_Null;
					}
				}
				else
				{
					OpcUa_NodeId* pNodeId = OpcUa_Null;
					if (GetNodeIdForCurrentParsedObject(pMyData, &pNodeId) == OpcUa_Good)
					{
						char* szNodeId = OpcUa_Null;
						Utils::NodeId2String(pNodeId, &szNodeId);
						if (szNodeId)
						{
							pMyData->uStatus = OpcUa_BadInternalError;
							OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "xmlNodeSetEndElementHandler>Critical error during initialization %s\n", szNodeId);
							free(szNodeId);
						}
						OpcUa_Free(pNodeId);
					}
					else
					{
						pMyData->uStatus = OpcUa_BadInternalError;
						OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "xmlNodeSetEndElementHandler>Critical error during initialization\n");
					}
				}
			}
			// reinit des handle_data
			pMyData->pUAVariable=OpcUa_Null;
			if (pMyData->pDataValue)
				delete pMyData->pDataValue;
			pMyData->pDataValue=OpcUa_Null;
			pMyData->NodeClass = OpcUa_NodeClass_Unspecified;
			OpcUa_String_Clear(&(pMyData->tmpString));
		}
		if (OpcUa_StrCmpA(cleanedName,"UAVariableType")==0)
		{
			pMyData->pUAVariableType=OpcUa_Null;
			pMyData->NodeClass = OpcUa_NodeClass_Unspecified;
		}
		if (OpcUa_StrCmpA(cleanedName,"UAMethod")==0)
		{
			pMyData->pMethod=NULL;
			pMyData->NodeClass = OpcUa_NodeClass_Unspecified;
		}
		if (OpcUa_StrCmpA(cleanedName,"UAView")==0)
		{
			pMyData->pView=NULL;
			pMyData->NodeClass = OpcUa_NodeClass_Unspecified;
		}
		if (OpcUa_StrCmpA(cleanedName,"UAReferenceType")==0)
		{
			pMyData->pUAReferenceType=OpcUa_Null;
			pMyData->NodeClass = OpcUa_NodeClass_Unspecified;
		}
		if (OpcUa_StrCmpA(cleanedName,"References")==0)
		{
		}
		if (OpcUa_StrCmpA(cleanedName,"Reference")==0)
		{
			if (pMyData->pReferenceNode)
			{
				if (Utils::IsNodeIdNull(pMyData->pReferenceNode->GetTargetId().NodeId))
				{
					pMyData->uStatus = OpcUa_BadSyntaxError;
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "xmlNodeSetEndElementHandler. You define an empty Target for a Reference\n");
				}
			}
			pMyData->pReferenceNode=OpcUa_Null;
		}
		if (OpcUa_StrCmpA(cleanedName,"Value")==0)
		{
			if (pMyData->bExtensionObjectEnumValueTypeValue)
				pMyData->bExtensionObjectEnumValueTypeValue=OpcUa_False;
			else
				pMyData->bValue=OpcUa_False;
		}
		if (OpcUa_StrCmpA(cleanedName,"Description")==0)
		{
			if (pMyData->bExtensionObjectBody)
			{
				if (pMyData->bExtensionObjectEnumValueType)
					pMyData->bExtensionObjectEnumValueTypeDescription=OpcUa_False;
				else
					pMyData->bExtensionObjectDescription=OpcUa_False;
			}
			else
				pMyData->bDescription=OpcUa_False;
		}
		if (OpcUa_StrCmpA(cleanedName,"DisplayName")==0)
		{
			if (pMyData->bExtensionObjectEnumValueType)
				pMyData->bExtensionObjectEnumValueTypeDisplayName=OpcUa_False;
			else
				pMyData->bDisplayName=OpcUa_False;
		}
		if (OpcUa_StrCmpA(cleanedName,"InverseName")==0)
		{
			pMyData->bInverseName=FALSE;
		}
		////////////////////////////////////////////////////////////////////
		// Type de données pouvant être encaspulé dans un UAVariable
		if (OpcUa_StrCmpA(cleanedName,"Boolean")==0)
		{
			pMyData->uiArrayCurrentElt++;
		}
		if (OpcUa_StrCmpA(cleanedName,"ListOfBoolean")==0)
		{
		}
		if (OpcUa_StrCmpA(cleanedName,"Byte")==0)
		{
			pMyData->uiArrayCurrentElt++;
		}
		if (OpcUa_StrCmpA(cleanedName,"ListOfByte")==0)
		{

		}
		if (OpcUa_StrCmpA(cleanedName,"SByte")==0)
		{
			pMyData->uiArrayCurrentElt++;
		}
		if (OpcUa_StrCmpA(cleanedName,"ListOfSByte")==0)
		{

		}
		if (OpcUa_StrCmpA(cleanedName,"Int16")==0)
		{
			pMyData->uiArrayCurrentElt++;
		}
		if (OpcUa_StrCmpA(cleanedName,"UInt16")==0)
		{
			pMyData->uiArrayCurrentElt++;
		}
		if (OpcUa_StrCmpA(cleanedName,"Int32")==0)
		{
			pMyData->uiArrayCurrentElt++;
		}
		if (OpcUa_StrCmpA(cleanedName,"UInt32")==0)
		{
			pMyData->uiArrayCurrentElt++;
		}
		if (OpcUa_StrCmpA(cleanedName, "Integer") == 0)
		{
			pMyData->uiArrayCurrentElt++;
		}
		if (OpcUa_StrCmpA(cleanedName, "UInteger") == 0)
		{
			pMyData->uiArrayCurrentElt++;
		}
		if (OpcUa_StrCmpA(cleanedName,"Int64")==0)
		{
			pMyData->uiArrayCurrentElt++;
		}
		if (OpcUa_StrCmpA(cleanedName,"UInt64")==0)
		{
			pMyData->uiArrayCurrentElt++;
		}
		if (OpcUa_StrCmpA(cleanedName, "Float") == 0)
		{
			pMyData->uiArrayCurrentElt++;
		}
		if (OpcUa_StrCmpA(cleanedName, "ListOfFloat") == 0)
		{
		}
		if (OpcUa_StrCmpA(cleanedName, "Double") == 0)
			pMyData->uiArrayCurrentElt++;
		if (OpcUa_StrCmpA(cleanedName, "ListOfDouble") == 0)
		{
		}
		if ( (OpcUa_StrCmpA(cleanedName,"uax:ListOfExtensionObject")==0) || (OpcUa_StrCmpA(cleanedName,"ListOfExtensionObject")==0))
		{		

		}
		if ( (OpcUa_StrCmpA(cleanedName,"uax:ExtensionObject")==0) || (OpcUa_StrCmpA(cleanedName,"ExtensionObject")==0))
		{
			pMyData->uiArrayCurrentElt++;
			pMyData->bExtensionObject=OpcUa_False;
		}
		if (OpcUa_StrCmpA(cleanedName,"DateTime")==0)
		{
			pMyData->uiArrayCurrentElt++;
		}	
		if (OpcUa_StrCmpA(cleanedName,"ListOfDateTime")==0)
		{	

		}
		if (OpcUa_StrCmpA(cleanedName,"String")==0)
		{
			pMyData->uiArrayCurrentElt++;
		}
		if (OpcUa_StrCmpA(cleanedName,"ListOfString")==0)
		{	

		}
		if (OpcUa_StrCmpA(cleanedName,"ListOfQualifiedName")==0)
		{
			if (pMyData->pDataValue)
			{
				OpcUa_Variant aVariant = pMyData->pDataValue->GetValue();
				aVariant.Value.Array.Length=pMyData->uiArrayCurrentElt;			
			}
		}
		if (OpcUa_StrCmpA(cleanedName, "QualifiedName") == 0)
		{
			pMyData->uiArrayCurrentElt++;
		}
		if (OpcUa_StrCmpA(cleanedName,"ListOfLocalizedText")==0)
		{
			if (pMyData->pDataValue)
			{
				OpcUa_Variant aVariant = pMyData->pDataValue->GetValue();
				aVariant.Value.Array.Length=pMyData->uiArrayCurrentElt;			
			}
		}
		if (OpcUa_StrCmpA(cleanedName,"LocalizedText")==0)
		{
			pMyData->uiArrayCurrentElt++;
		}
		if (OpcUa_StrCmpA(cleanedName,"Text")==0)
		{
			pMyData->bText=OpcUa_False;
			//pMyData->bLocale = OpcUa_False;
		}
		if (OpcUa_StrCmpA(cleanedName,"Locale")==0)
		{
			pMyData->bLocale=OpcUa_False;
		}
		if (OpcUa_StrCmpA(cleanedName,"ByteString")==0)
		{
			pMyData->uiArrayCurrentElt++;
		}
		if (OpcUa_StrCmpA(cleanedName,"ListOfByteString")==0)
		{		
			OpcUa_Variant aVariant = pMyData->pDataValue->GetValue();
			aVariant.Value.Array.Length=pMyData->uiArrayCurrentElt;		
		}
		if (OpcUa_StrCmpA(cleanedName,"Guid")==0)
		{
			pMyData->uiArrayCurrentElt++;
		}

		// Prise en charge de balise element dédié a l'ExtensionObject
		if ( (OpcUa_StrCmpA(cleanedName,"uax:TypeId")==0) || (OpcUa_StrCmpA(cleanedName,"TypeId")==0))//OpcUa_ExpandedNodeId TypeId;
		{
			if ( (pMyData->pUAVariable) && (pMyData->pDataValue) && (pMyData->bValue) )
			{
				pMyData->bExtensionObjectTypeId=OpcUa_False;
			}
			else
			{
				if (pMyData->pUAVariable)
				{
					char* szNodeId = OpcUa_Null;
					// NodeId
					OpcUa_NodeId* pNodeId = pMyData->pUAVariable->GetNodeId();
					Utils::NodeId2String(pNodeId,&szNodeId);
					if (szNodeId)
					{
						// Attribute
						pMyData->uStatus = OpcUa_BadSyntaxError;
						OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "xmlNodeSetEndElementHandler> XML file corrupted. the Element TypeId: %s is define outside of UAVariable.\n", szNodeId);
						free(szNodeId);
					}
				}
				else
				{
					pMyData->uStatus = OpcUa_BadSyntaxError;
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "xmlNodeSetEndElementHandler> XML file corrupted. An Element TypeId is define outside of UAVariable.\n");
				}
			}
		}
		if ( (OpcUa_StrCmpA(cleanedName,"uax:Body")==0) || (OpcUa_StrCmpA(cleanedName,"Body")==0))//L'element Body peut contenir n'importe quel type défini dans Opc.Ua.Types.xsd
		{
			pMyData->bExtensionObjectBody=OpcUa_False;
		}
		if ( (OpcUa_StrCmpA(cleanedName,"uax:EnumValueType")==0) || (OpcUa_StrCmpA(cleanedName,"EnumValueType")==0))//L'element Body contient dans ce cas un EnumValueType
		{
			if (pMyData->pDataValue)
			{
				OpcUa_Variant aVariant=pMyData->pDataValue->GetValue();
				if (pMyData->pDataValue->GetValue().ArrayType == OpcUa_VariantArrayType_Scalar)
				{
					aVariant.Value.ExtensionObject->Body.EncodeableObject.Object = Utils::Copy(pMyData->pEnumValueType);
				}
				else
				{
					int iCurrentSize = pMyData->uiArrayCurrentElt;
					if (pMyData->pDataValue->GetValue().ArrayType == OpcUa_VariantArrayType_Array)
					{
						OpcUa_DataValue* pInternalDataValue = pMyData->pDataValue->GetInternalDataValue();
						//aVariant.Value.Array.Value.ExtensionObjectArray[iCurrentSize].Body.EncodeableObject.Object = Utils::Copy(pMyData->pEnumValueType);
						if (pInternalDataValue->Value.Value.Array.Value.ExtensionObjectArray)
						{
							OpcUa_EnumValueType* pEnumValueType = (OpcUa_EnumValueType*)pInternalDataValue->Value.Value.Array.Value.ExtensionObjectArray[iCurrentSize].Body.EncodeableObject.Object;
							if (pEnumValueType)
							{
								if (OpcUa_String_StrLen(&pMyData->pEnumValueType->Description.Locale) == 0)
									OpcUa_String_AttachCopy(&pMyData->pEnumValueType->Description.Locale, "en-us");
								if ((OpcUa_String_StrLen(&pMyData->pEnumValueType->Description.Text) > 0))
									OpcUa_LocalizedText_CopyTo(&(pMyData->pEnumValueType->Description), &(pEnumValueType->Description));
								if (OpcUa_String_StrLen(&pMyData->pEnumValueType->DisplayName.Locale) == 0)
									OpcUa_String_AttachCopy(&pMyData->pEnumValueType->DisplayName.Locale, "en-us");
								if ((OpcUa_String_StrLen(&pMyData->pEnumValueType->DisplayName.Text) > 0))
									OpcUa_LocalizedText_CopyTo(&(pMyData->pEnumValueType->DisplayName), &(pEnumValueType->DisplayName));
								pEnumValueType->Value = pMyData->pEnumValueType->Value;
								pInternalDataValue->Value.Value.Array.Length = pMyData->uiArrayCurrentElt + 1;
							}
						}
					}
					else
					{
						if (pMyData->pDataValue->GetValue().ArrayType == OpcUa_VariantArrayType_Matrix)
						{
							pMyData->uStatus = OpcUa_BadInvalidArgument;
							OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "OpcUa_VariantArrayType_Matrix  not supported\n");
						}
					}
				}
			}
			OpcUa_String_Clear(&(pMyData->pEnumValueType->Description.Locale));
			OpcUa_String_Clear(&(pMyData->pEnumValueType->Description.Text));
			OpcUa_EnumValueType_Clear(pMyData->pEnumValueType);
			OpcUa_Free(pMyData->pEnumValueType);
			pMyData->pEnumValueType = OpcUa_Null;
			pMyData->bExtensionObjectEnumValueType=OpcUa_False;
		}
		if ( (OpcUa_StrCmpA(cleanedName,"uax:Argument")==0) || (OpcUa_StrCmpA(cleanedName,"Argument")==0))//L'element Body peut contenir n'importe quel type défini dans Opc.Ua.Types.xsd
		{
			//if (pMyData->pUAVariable->GetNodeId().Identifier.Numeric==11493)
			//	printf("Debug purpose\n");
			OpcUa_Variant aVariant=pMyData->pDataValue->GetValue();
			if (pMyData->pDataValue->GetValue().ArrayType==OpcUa_VariantArrayType_Scalar)
			{
				OpcUa_Argument* pArgument = Utils::Copy(pMyData->pArgument);
				aVariant.Value.ExtensionObject->Body.EncodeableObject.Object = pArgument; // pMyData->pArgument;
			}
			else
			{
				int iCurrentSize=pMyData->uiArrayCurrentElt;
				if (pMyData->pDataValue->GetValue().ArrayType==OpcUa_VariantArrayType_Array)
				{
					// Il faut faire grossir le ExtensionObjectArray d'un element
					if (aVariant.Value.Array.Value.ExtensionObjectArray)
					{
						OpcUa_Argument* pArgument = Utils::Copy(pMyData->pArgument);
						aVariant.Value.Array.Value.ExtensionObjectArray[iCurrentSize].Body.EncodeableObject.Object = pArgument; // pMyData->pArgument;
						aVariant.Value.Array.Length = pMyData->uiArrayCurrentElt+1;
					}
				}
				else
				{
					if (pMyData->pDataValue->GetValue().ArrayType == OpcUa_VariantArrayType_Matrix)
					{
						pMyData->uStatus = OpcUa_BadInvalidArgument;
						OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "OpcUa_VariantArrayType_Matrix  not supported\n");
					}
				}
			}
			OpcUa_Argument_Clear(pMyData->pArgument);
			OpcUa_Free(pMyData->pArgument);
			pMyData->pArgument = OpcUa_Null;// 
			pMyData->bExtensionObjectArgument=OpcUa_False;
		}
		if ((OpcUa_StrCmpA(cleanedName,"uax:Name")==0)|| (OpcUa_StrCmpA(cleanedName,"Name")==0))//L'element Body peut contenir n'importe quel type défini dans Opc.Ua.Types.xsd
		{
			pMyData->bExtensionObjectName=OpcUa_False;
		}
		if ( (OpcUa_StrCmpA(cleanedName,"uax:DataType")==0) || (OpcUa_StrCmpA(cleanedName,"DataType")==0))//L'element Body peut contenir n'importe quel type défini dans Opc.Ua.Types.xsd
		{
			pMyData->bExtensionObjectDataType=OpcUa_False;
		}
		if ( (OpcUa_StrCmpA(cleanedName,"uax:Identifier")==0) || (OpcUa_StrCmpA(cleanedName,"Identifier")==0))//L'element Body peut contenir n'importe quel type défini dans Opc.Ua.Types.xsd
		{
			pMyData->bExtensionObjectIdentifier=OpcUa_False;
		}
		if ( (OpcUa_StrCmpA(cleanedName,"uax:ValueRank")==0) ||( OpcUa_StrCmpA(cleanedName,"ValueRank")==0))//L'element Body peut contenir n'importe quel type défini dans Opc.Ua.Types.xsd
		{
			pMyData->bExtensionObjectValueRank=OpcUa_False;
		}
		if ( (OpcUa_StrCmpA(cleanedName,"uax:ArrayDimensions")==0) || (OpcUa_StrCmpA(cleanedName,"ArrayDimensions")==0))//L'element Body peut contenir n'importe quel type défini dans Opc.Ua.Types.xsd
		{
			pMyData->bExtensionObjectArrayDimensions=OpcUa_False;
		}
		////////////////////////////////////////////////////////////////////////////
	}
	else
		pMyData->uStatus = OpcUa_BadOutOfMemory;
	if (cleanedName)
		free(cleanedName);
	if (xmlNamespace)
		free(xmlNamespace);
}


///////////////////////////////////////////////////////////////////////////////////////
// structure userData associé au parsing aux fichiers xml de simulation
typedef struct handler_Simulation_Data
{
	XML_ParserStruct*		XML_Parser;    
	void*					userData; 
	OpcUa_StatusCode		uStatus; // Status code updated during the asynchronous load of the XML File
	CSimulatedGroup*		pSimulatedGroup;
	OpcUa_Boolean			bNamespaceUri;
} HANDLER_SIMULATION_DATA;
///////////////////////////////////////////////////////////////////////////////////////
// Fonction délegué pour la prise en charge du chargement des fichiers de simulation
void xmlSimulationStartElementHandler(
	void *userData,     /* user defined data */
	const char *name,   /* Element name */
	const char **atts)  /* Element Attribute list, provided in name value pairs */
						/* i.e. atts[0] contains name, atts[1] contains value for */
						/* atts[0], and so on...*/
{
	HANDLER_SIMULATION_DATA* pMyData=(HANDLER_SIMULATION_DATA*)userData; 
	char* cleanedName = OpcUa_Null;
	char* xmlNamespace = OpcUa_Null;
	GetCleanupXmlElt(name, &cleanedName, &xmlNamespace);

	CUAInformationModel* pInformationModel=CServerApplication::m_pTheAddressSpace;
	if (OpcUa_StrCmpA(cleanedName,"Group")==0)
	{
		CSimulatedGroup* pSimulatedGroup=new CSimulatedGroup(atts);
		pMyData->pSimulatedGroup=pSimulatedGroup;
		pInformationModel->AddSimulatedGroup(pSimulatedGroup);
	}
	if (OpcUa_StrCmpA(cleanedName,"Simulated")==0)
	{
		CSimulatedNode* pSimulated=new CSimulatedNode(atts);
		if (pMyData->pSimulatedGroup)
			pMyData->pSimulatedGroup->AddSimulatedNode(pSimulated);
		else
		{
			pMyData->uStatus = OpcUa_BadOutOfMemory;
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Your XML simulation file is corrupted. Check it with the Schema\n");
		}
	}
	if (OpcUa_StrCmpA(cleanedName, "Uri") == 0)
	{
		pMyData->bNamespaceUri = OpcUa_True;
	}
	if (cleanedName)
		free(cleanedName);
	if (xmlNamespace)
		free(xmlNamespace);
}
void xmlSimulationEndElementHandler(
	void *userData,     /* user defined data */
	const char *name)   /* Element name */
{
	HANDLER_SIMULATION_DATA* pMyData=(HANDLER_SIMULATION_DATA*)userData; 
	char* cleanedName = OpcUa_Null;
	char* xmlNamespace = OpcUa_Null;
	GetCleanupXmlElt(name, &cleanedName, &xmlNamespace);
	//CUAInformationModel* pInformationModel=CServerApplication::m_pTheAddressSpace;
	if (OpcUa_StrCmpA(cleanedName, "Group") == 0)
	{
		pMyData->pSimulatedGroup=NULL;
	}
	if (OpcUa_StrCmpA(cleanedName, "Uri") == 0)
	{
		pMyData->bNamespaceUri = OpcUa_False;
	}
	if (cleanedName)
		free(cleanedName);
	if (xmlNamespace)
		free(xmlNamespace);

}

void xmlSimulationCharacterDataHandler(void *userData,     /* user defined data */
	const char *s,  /* non-null terminated character string */
	int len)
{
	CUAInformationModel* pInformationModel = CServerApplication::m_pTheAddressSpace;
	HANDLER_SIMULATION_DATA* pMyData = (HANDLER_SIMULATION_DATA*)userData;
	OpcUa_StatusCode uStatus = OpcUa_Bad;
	if (len == 0)
		return;
	if (*s == 0xA) // newline
		return;
	if (*s == 0x9) // TAB
		return;

	if (*s == 0x3c) // <
		return;
	if (*s == 0x3e) // >
		return;
	OpcUa_CharA* buffTmp = (OpcUa_CharA*)OpcUa_Alloc(len + 1);
	ZeroMemory(buffTmp, len + 1);
	memcpy(buffTmp, s, len);
	if (Utils::IsBufferEmpty(buffTmp, len))
	{
		OpcUa_Free(buffTmp);
		return;
	}
	else
		free(buffTmp);
	// traitement des Uris
	if (pMyData->bNamespaceUri)
	{
		char* buff = (char*)OpcUa_Alloc(len + 1);
		if (buff)
		{
			ZeroMemory(buff, len + 1);
			memcpy(buff, s, len);
			if (!Utils::IsBufferEmpty(buff, len))
			{
				OpcUa_String aNamespaceUri;
				OpcUa_String_Initialize(&aNamespaceUri);
				OpcUa_String_AttachCopy(&aNamespaceUri, buff);
				uStatus = pInformationModel->OnLoadNamespaceUriVerifyUri(aNamespaceUri);
				OpcUa_String_Clear(&aNamespaceUri);
			}
			ZeroMemory(buff, len + 1);
			free(buff);
		}
		return;
	}
}

//============================================================================
// CServerApplication::Constructor
//============================================================================
CServerApplication::CServerApplication(void)
{
	if (!m_bIsExist)
	{
		m_ConfigPath = OpcUa_Null;
		m_ConfigFileName = OpcUa_Null;
		m_hSessionsTimeoutThread = OpcUa_Null;
		// Initialisation de la couche de communication dans le cas d'une implementation pour une Gateway
		m_pVpiDevices = new CVpiDeviceList();
		m_pVpiDevices->clear();
		OpcUa_Mutex_Create(&m_VpiDevicesMutex);
		// Event Engine
		m_pEventsEngine = OpcUa_Null;
		m_bLDSRegistrationActive = OpcUa_True;
		m_bSecurityNoneAccepted = OpcUa_True;
		m_pApplicationDescription = OpcUa_Null;
		m_hEndpoint = 0;
		m_nNoOfSecurityPolicies = 0;
		m_pSecurityPolicies = OpcUa_Null;
		m_lastSessionId = 0;
		m_pSecureChannels = new CSecureChannelList();
		m_hLDSRegistrationThread = OpcUa_Null;
		m_uiLDSRegistrationInterval = 60000; // valeur par défaut 10mn
		m_dblMiniSamplingInterval = DEFAULT_SAMPLING_INTERVAL; // Default value for sampling interval
		// Attention il s'agit d'une valeur arbitraire utilisé dans l'OpenOpcUaCoreServer
		m_dblMaxSamplingInterval = 3600000; // interval d'echantillonnage minimum. Valeur par défaut 3600000 ms // 1 heure
		// Attention il s'agit d'une valeur arbitraire utilisé dans l'OpenOpcUaCoreServer
		m_SessionsTimeoutSem = OpcUa_Null;
		m_pHaEngine = OpcUa_Null;
		OpcUa_String_Initialize(&m_ArchiveName);
		OpcUa_String_Initialize(&m_ArchiveId);
		m_bIsExist = OpcUa_True;
#ifdef WIN32
		// Check if the server is installed as a Windows services
		HANDLE hInstance = (HANDLE)GetModuleHandle(OpcUa_Null);
		m_ServiceModule.SetInternalServiceModule(&m_ServiceModule);
		m_ServiceModule.Init((HINSTANCE)hInstance, IDS_SERVICENAME);
#endif
	}
	else
		throw logic_error("Cannot create more than one CServerApplication");
}

//============================================================================
// CServerApplication::Destructor
//============================================================================
CServerApplication::~CServerApplication(void)
{
	if (m_ConfigPath)
	{
		OpcUa_Free(m_ConfigPath);
		m_ConfigPath = OpcUa_Null;
	}
	if (m_ConfigFileName)
	{
		OpcUa_Free(m_ConfigFileName);
		m_ConfigFileName = OpcUa_Null;
	}
	// Stop the HA engine
	if (m_pHaEngine)
		delete m_pHaEngine;
	// Stop Event engine
	if (m_pEventsEngine)
		delete m_pEventsEngine;
	// Stop the server itself
	Stop();
	// remove Vpis
	RemoveAllVpiDevice();
	if (m_pVpiDevices)
	{
		delete m_pVpiDevices;
		m_pVpiDevices = OpcUa_Null;
	}
	// Remove UsersIdentityToken
	OpcUa_Mutex_Delete(&m_UserNameIdentityTokenListMutex);
	RemoveUsersIdentityToken();
	// Remove X509UserCertificate
	OpcUa_Mutex_Delete(&m_X509IdentityTokenListMutex);
	RemoveX509IdentityTokenList();
	// 
	OpcUa_Mutex_Delete(&m_VpiDevicesMutex);
	//
	if (m_pSecureChannels)
	{
		RemoveAllSecureChannel();
		delete m_pSecureChannels;
		m_pSecureChannels=OpcUa_Null;
	}
	OpcUa_String_Clear(&m_ArchiveName);
	OpcUa_String_Clear(&m_ArchiveId);
	OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_WARNING, "OpenOpcUaCore clearing FileNodeSet list.\n");
	for (OpcUa_UInt32 ii=0;ii<m_FilesNodeSet.size();ii++)
	{
		OpcUa_String* pFullFileName=(OpcUa_String*)g_pTheApplication->m_FilesNodeSet.at(ii);
		OpcUa_String_Clear(pFullFileName);
		OpcUa_Free(pFullFileName);
	}
	for (OpcUa_UInt32 ii=0;ii<m_FilesSimulation.size();ii++)
	{
		OpcUa_String* pFullFileName = (OpcUa_String*)g_pTheApplication->m_FilesSimulation.at(ii);
		OpcUa_String_Clear(pFullFileName);
		OpcUa_Free(pFullFileName);
	}
	for (OpcUa_UInt32 ii=0;ii<m_FilesSubsystem.size();ii++)
	{
		OpcUa_String* pFullFileName = (OpcUa_String*)g_pTheApplication->m_FilesSubsystem.at(ii);
		OpcUa_String_Clear(pFullFileName);
		OpcUa_Free(pFullFileName);
	}
	OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_WARNING, "OpenOpcUaCore server is removing all bindings.\n");
	for (OpcUa_UInt32 iii=0;iii<m_ServerBindingList.size();iii++)
	{
		CUABinding* pBinding=(CUABinding*)m_ServerBindingList.at(iii);
		delete pBinding;
	}
	m_ServerBindingList.clear();


	if (m_pSecurityPolicies)
	{
		for (OpcUa_Int32 i = 0; i < m_nNoOfSecurityPolicies; i++)
		{
			OpcUa_String_Clear(&(m_pSecurityPolicies[i].sSecurityPolicy));
		}
		OpcUa_Free(m_pSecurityPolicies);
		m_pSecurityPolicies = OpcUa_Null;
	}

	OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"OpenOpcUaCore server is deleting pending ressources.\n");
	OpcUa_Mutex_Delete(&m_hMutex);
	OpcUa_Semaphore_Delete(&m_hStopLDSRegistrationSem);
	OpcUa_Semaphore_Delete(&m_hLDSRegistrationRequest);
	OpcUa_Semaphore_Delete(&m_SessionsTimeoutSem);
	OpcUa_Mutex_Delete(&m_hSessionsMutex);
	OpcUa_Mutex_Delete(&m_hSecureChannelsMutex);
	m_bIsExist=OpcUa_False;
}

//============================================================================
// CServerApplication::Initialize
//============================================================================
void CServerApplication::Initialize(void)
{	
	CApplication::InitializeAbstractionLayer();
	CApplication::InitializeTrace();
	OpcUa_Semaphore_Create(&m_hLDSRegistrationRequest,0,0x100);
	OpcUa_Semaphore_Create(&m_hStopLDSRegistrationSem, 0, 0x100);
	OpcUa_Mutex_Create(&m_hMutex);
	OpcUa_Mutex_Create(&m_hSessionsMutex);
	OpcUa_Mutex_Create(&m_hSecureChannelsMutex);
	// Let's initialize the userTokenIdentity and username
	OpcUa_Mutex_Create(&m_UserNameIdentityTokenListMutex);
	InitializeUsersIdentityToken();
	// Let's load the X509User certificate
	OpcUa_Mutex_Create(&m_X509IdentityTokenListMutex);
	InitializeX509IdentityTokenList();
}

//============================================================================
// CServerApplication::Uninitialize
//============================================================================
void CServerApplication::Uninitialize(void)
{
	OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"OpenOpcUaCore server is Uninitializing.\n");
	//Cleanup();
	CApplication::Uninitialize();
}

//============================================================================
// CServerApplication::InitializeSecurity
//============================================================================
OpcUa_StatusCode CServerApplication::InitializeSecurity(/*OpcUa_String certificateStorePath*/)
{
	OpcUa_StatusCode uStatus=OpcUa_Bad;
	OpcUa_Mutex_Lock(m_hMutex);
	// initialize the base.
	OpcUa_String* pApplicationUri=OpcUa_Null;
	OpcUa_LocalizedText* sApplicationName = OpcUa_Null; // (OpcUa_LocalizedText*)OpcUa_Alloc(sizeof(OpcUa_LocalizedText));

	CApplicationDescription* pApplicationDescription=GetApplicationDescription();
	if (pApplicationDescription)
	{
		pApplicationUri = pApplicationDescription->GetApplicationUri();
		sApplicationName = GetApplicationName();
		if (sApplicationName)
		{
			uStatus = CApplication::InitializeSecurity(pApplicationUri,	sApplicationName);
			if (uStatus == OpcUa_Good)
			{
				//
				m_pSecurityPolicies = (OpcUa_Endpoint_SecurityPolicyConfiguration*)OpcUa_Alloc(sizeof(OpcUa_Endpoint_SecurityPolicyConfiguration) * 5);
				if (m_pSecurityPolicies)
				{
					ZeroMemory(m_pSecurityPolicies, sizeof(OpcUa_Endpoint_SecurityPolicyConfiguration) * 5);
					OpcUa_UInt32 i = 0;
					if (IsSecurityNoneAccepted())
					{
						// Add no-security policy.
						OpcUa_String_Initialize(&m_pSecurityPolicies[i].sSecurityPolicy);
						OpcUa_String_AttachCopy(&m_pSecurityPolicies[i].sSecurityPolicy, OpcUa_SecurityPolicy_None);
						m_pSecurityPolicies[i++].uMessageSecurityModes = OPCUA_ENDPOINT_MESSAGESECURITYMODE_NONE;
					}
					// Add Basic128Rsa15 Sign policy.
					OpcUa_String_Initialize(&m_pSecurityPolicies[i].sSecurityPolicy);
					OpcUa_String_AttachCopy(&m_pSecurityPolicies[i].sSecurityPolicy, OpcUa_SecurityPolicy_Basic128Rsa15);
					m_pSecurityPolicies[i++].uMessageSecurityModes = OPCUA_ENDPOINT_MESSAGESECURITYMODE_SIGN;

					// Add Basic128Rsa15 SignAndEncrypt policy.
					OpcUa_String_Initialize(&m_pSecurityPolicies[i].sSecurityPolicy);
					OpcUa_String_AttachCopy(&m_pSecurityPolicies[i].sSecurityPolicy, OpcUa_SecurityPolicy_Basic128Rsa15);
					m_pSecurityPolicies[i++].uMessageSecurityModes = OPCUA_ENDPOINT_MESSAGESECURITYMODE_SIGNANDENCRYPT;

					// Add Basic256 Sign policy.
					OpcUa_String_Initialize(&m_pSecurityPolicies[i].sSecurityPolicy);
					OpcUa_String_AttachCopy(&m_pSecurityPolicies[i].sSecurityPolicy, OpcUa_SecurityPolicy_Basic256);
					m_pSecurityPolicies[i++].uMessageSecurityModes = OPCUA_ENDPOINT_MESSAGESECURITYMODE_SIGN;

					// Add Basic256 SignAndEncrypt policy.
					OpcUa_String_Initialize(&m_pSecurityPolicies[i].sSecurityPolicy);
					OpcUa_String_AttachCopy(&m_pSecurityPolicies[i].sSecurityPolicy, OpcUa_SecurityPolicy_Basic256);
					m_pSecurityPolicies[i++].uMessageSecurityModes = OPCUA_ENDPOINT_MESSAGESECURITYMODE_SIGNANDENCRYPT;

					// Add Basic256Sha256 Sign policy.
					//OpcUa_String_AttachCopy(&m_pSecurityPolicies[i].sSecurityPolicy, OpcUa_SecurityPolicy_Basic256Sha256);
					//m_pSecurityPolicies[i++].uMessageSecurityModes = OPCUA_ENDPOINT_MESSAGESECURITYMODE_SIGN;

					// Add Basic256Sha256 SignAndEncrypt policy.
					//OpcUa_String_AttachCopy(&m_pSecurityPolicies[i].sSecurityPolicy, OpcUa_SecurityPolicy_Basic256Sha256);
					//m_pSecurityPolicies[i++].uMessageSecurityModes = OPCUA_ENDPOINT_MESSAGESECURITYMODE_SIGNANDENCRYPT;

					m_nNoOfSecurityPolicies = i;
				}
			}
			else
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "CApplication::InitializeSecurity failed something is wrong with your certificate and/or certificate store 0x%05x\n", uStatus);
		}
		else
			uStatus = OpcUa_BadInvalidArgument;
	}
	else
		uStatus = OpcUa_BadInternalError;

	OpcUa_Mutex_Unlock(m_hMutex);
	return uStatus;
}

//============================================================================
// CServerApplication::Start
//============================================================================
void CServerApplication::Start()
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_String* pszEndpointUrl=NULL; //(OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));

	try
	{
		// register the server to the Local Discovery Server
		if (m_bLDSRegistrationActive)
		{
			OpcUa_String discoveryUrl;
			OpcUa_String_Initialize(&discoveryUrl);
			OpcUa_String_AttachCopy(&discoveryUrl, "opc.tcp://localhost:4840");
			uStatus = RegisterServer((OpcUa_CharA*)"opc.tcp", discoveryUrl, OpcUa_True);// "opc.tcp://localhost:4840"
			OpcUa_String_Clear(&discoveryUrl);
		}
		for (OpcUa_UInt32 iii=0;iii<m_ServerBindingList.size();iii++)
		{
			CUABinding* pBinding=(CUABinding*)m_ServerBindingList.at(iii);
			// Creation de l'url d'ecoute du serveur a partir des données du binding
			OpcUa_String aString=pBinding->AsString();
			OpcUa_UInt32 iSize=OpcUa_String_StrLen(&aString);
			pszEndpointUrl=(OpcUa_String*)OpcUa_Alloc(iSize);
			OpcUa_String_Initialize(pszEndpointUrl);
			OpcUa_String_StrnCpy(pszEndpointUrl,&aString, iSize);

			// Add the serverName
			OpcUa_String szAppName=GetApplicationName()->Text;
			OpcUa_String_StrnCat(pszEndpointUrl,
				&szAppName,
				OpcUa_String_StrLen(&szAppName) );

			// Acquire lock.
			OpcUa_Mutex_Lock(m_hMutex);
			OpcUa_Endpoint_SerializerType eEncoding= pBinding->GetEncoding();
			if (eEncoding==OpcUa_Endpoint_SerializerType_Binary)
			{
				// Create the endpoint and provide the table of functions that process incoming requests.
				uStatus = OpcUa_Endpoint_Create(   
					&m_hEndpoint,
					OpcUa_Endpoint_SerializerType_Binary,
					g_SupportedServices);

				if (uStatus==OpcUa_Good)
				{
					// Open the endpoint.
					// Setup SecureListener, callback and UserData
					// an endpointUrl is a parameter containing a string which look like  "opc.tcp://localhost:16664/4CEUAServer"
					OpcUa_StringA strAEndpointUrl=(OpcUa_StringA)OpcUa_String_GetRawString(pszEndpointUrl);
					// Configuration du transportProfileUri
					OpcUa_String aTransportProfileUri=pBinding->GetTransportProfileUri();
					OpcUa_StringA strATransportProfileUri=OpcUa_String_GetRawString(&aTransportProfileUri);
					//
					OpcUa_ByteString* pCertificate=GetCertificate();
					OpcUa_Key* pPrivateKey=GetPrivateKey();
					//
					OpcUa_CertificateStoreConfiguration* pStoreConfiguration = GetPkiConfig();
					uStatus = OpcUa_Endpoint_Open(
						m_hEndpoint,                                    
						strAEndpointUrl, 
						strATransportProfileUri,
						Server_EndpointCallback, // Fonction callback                       
						this, // UserData (CServerApplication*) ce pointeur sera utlisé plus tard pour référencé l'application
						pCertificate,              
						pPrivateKey,
						pStoreConfiguration,
						m_nNoOfSecurityPolicies,      
						m_pSecurityPolicies); 
					if (uStatus==OpcUa_Good)
					{				
						OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Server Endpoint open. It's now listening at %s with Binary Serializer\n",strAEndpointUrl);
					}
					else
						OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Critical error>Could not open the Binary endpoint. %s hr=0x%05x \n",strAEndpointUrl,uStatus);
				}
				else
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Error Could not create the endpoint. %s hr=0x%05x \n",pszEndpointUrl,uStatus);
			}
			else
			{
				if (eEncoding==OpcUa_Endpoint_SerializerType_Xml)
				{

					// Create the endpoint and provide the table of functions that process incoming requests.
					uStatus = OpcUa_Endpoint_Create(   
						&m_hEndpoint,
						OpcUa_Endpoint_SerializerType_Xml,
						g_SupportedServices);
					if (uStatus==OpcUa_Good)
					{
						// Open the endpoint.
						// Mise en place des callback et des UserData
						// endpointUrl est un param qui ressemble a "opc.tcp://localhost:16664/4CEUAServer"
						OpcUa_StringA strAEndpointUrl=(OpcUa_StringA)OpcUa_String_GetRawString(pszEndpointUrl);
						OpcUa_String aTransportProfileUri;
						OpcUa_String_Initialize(&aTransportProfileUri);
						OpcUa_String_AttachCopy(&aTransportProfileUri,OpcUa_TransportProfile_HttpsSoapUaXml );
						OpcUa_StringA strATransportProfileUri=OpcUa_String_GetRawString(&aTransportProfileUri);
						uStatus = OpcUa_Endpoint_Open(
							m_hEndpoint,                                    
							strAEndpointUrl,                                   
							strATransportProfileUri,
							Server_EndpointCallback, // Fonction callback                       
							this, // UserData (CServerApplication*) ce pointeur sera utlisé plus tard pour référencé l'application
							GetCertificate(),              
							GetPrivateKey(),
							GetPkiConfig(),                    
							m_nNoOfSecurityPolicies,      
							m_pSecurityPolicies); 
						if (uStatus==OpcUa_Good)
						{		
							OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Server Endpoint open. It's now listening at %s with XML Serializer\n",strAEndpointUrl);
							if (m_bLDSRegistrationActive)
							{
								// register the server to the Local Discovery Server
								OpcUa_String discoveryUrl;
								OpcUa_String_Initialize(&discoveryUrl);
								OpcUa_String_AttachCopy(&discoveryUrl, "opc.tcp://localhost:4840");
								uStatus = RegisterServer((OpcUa_CharA*)"http", discoveryUrl, OpcUa_True);// "opc.tcp://localhost:4840"
								OpcUa_String_Clear(&discoveryUrl);
							}
						}
						else
							OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Critical error>Could not open the XML endpoint. %s hr=0x%05x \n",strAEndpointUrl,uStatus);
					}
					else
						OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Could not create endpoint. %s hr=0x%05x \n",pszEndpointUrl,uStatus);
				}
			}
			OpcUa_Mutex_Unlock(m_hMutex);
		}
		if (pszEndpointUrl)
		{
			OpcUa_String_Clear(pszEndpointUrl);
			OpcUa_Free(pszEndpointUrl);
		}
	}
	catch (...)
	{
		OpcUa_Mutex_Unlock(m_hMutex);
		throw;
	}
}

//============================================================================
// CServerApplication::Stop
//============================================================================
void CServerApplication::Stop()
{	
	StopLDSRegistrationThread();
	// On va indiquer que le serveur est en train de s'arrêter en positionnant les informations de Shutdown
	//CUAInformationModel* pInformationModel=CServerApplication::m_pTheAddressSpace;
	//if (pInformationModel)
	//{
	//	CServerStatus* pServerStatus=pInformationModel->GetInternalServerStatus();
	//	pServerStatus->SetServerState(OpcUa_ServerState_Shutdown);
	//	OpcUa_NodeId aTmpNodeId;
	//	OpcUa_NodeId_Initialize(&aTmpNodeId);
	//	CUAVariable* pUATmpVariable=NULL;
	//	aTmpNodeId.IdentifierType=OpcUa_IdentifierType_Numeric;
	//	OpcUa_Variant varVal;
	//	OpcUa_Variant_Initialize(&varVal);
	//	OpcUa_Variant_Initialize(&varVal);
	//	varVal.Datatype=OpcUaType_Int32;
	//	aTmpNodeId.Identifier.Numeric=2259; // State
	//	OpcUa_StatusCode uStatus=pInformationModel->GetNodeIdFromVariableList(aTmpNodeId,&pUATmpVariable);
	//	if (uStatus==OpcUa_Good)
	//	{
	//		varVal.Datatype=OpcUaType_Int32;
	//		varVal.Value.Int32=pServerStatus->GetServerState();
	//		CDataValue* pDataValue=pUATmpVariable->GetValue();
	//		if (pDataValue)
	//			pDataValue->SetValue(varVal);
	//	}
	//	// 2992
	//	pServerStatus->SetSecondsTillShutdown(10);
	//	aTmpNodeId.Identifier.Numeric=2292; // SecondsTillShutdown
	//	uStatus = pInformationModel->GetNodeIdFromVariableList(aTmpNodeId, &pUATmpVariable);
	//	if (uStatus == OpcUa_Good)
	//	{
	//		varVal.Datatype=OpcUaType_UInt32;
	//		varVal.Value.Int32=pServerStatus->GetSecondsTillShutdown();
	//		CDataValue* pDataValue=pUATmpVariable->GetValue();
	//		if (pDataValue)
	//			pDataValue->SetValue(varVal);
	//	}
	//	// 2993
	//	OpcUa_LocalizedText aShutdownReason;
	//	OpcUa_LocalizedText_Initialize(&aShutdownReason);
	//	OpcUa_String_AttachCopy(&(aShutdownReason.Locale),"en-US");
	//	OpcUa_String_AttachCopy(&(aShutdownReason.Text),"Server shutdown by operator");
	//	pServerStatus->SetShutdownReason(aShutdownReason);
	//	aTmpNodeId.Identifier.Numeric=2293; // ShutdownReason
	//	uStatus=pInformationModel->GetNodeIdFromVariableList(aTmpNodeId,&pUATmpVariable);
	//	if (uStatus == OpcUa_Good)
	//	{
	//		varVal.Datatype=OpcUaType_String;
	//		varVal.Value.LocalizedText=Utils::Copy(&aShutdownReason);
	//		CDataValue* pDataValue=pUATmpVariable->GetValue();
	//		if (pDataValue)
	//			pDataValue->SetValue(varVal);
	//	}
	//}
	// fin informations de Shutdown
	OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"OpenOpcUaCore server is stopping.\n");
	OpcUa_StatusCode uStatus = OpcUa_Good;
	if( m_hEndpoint)
	{
		// Close endpoint.
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"OpenOpcUaCore closing endpoint.\n");
		uStatus = OpcUa_Endpoint_Close(m_hEndpoint);
		if (uStatus!=OpcUa_Good)
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Could not close the endpoint.\n");
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"OpenOpcUaCore deleting endpoint.\n");
		// Cleanup
		if ( m_hEndpoint)
			OpcUa_Endpoint_Delete(&m_hEndpoint);
		m_hEndpoint=0;
	}
	OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"OpenOpcUaCore server is removing all sessions.\n");
	StopSessionsTimeoutThread();
	RemoveAllSessionServer();
	
	if (m_bLDSRegistrationActive)
	{
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"OpenOpcUaCore Will Unregister from the LDS.\n");
		// unregister the server to the Local Discovery Server
		OpcUa_String discoveryUrl;
		OpcUa_String_Initialize(&discoveryUrl);
		OpcUa_String_AttachCopy(&discoveryUrl, "opc.tcp://localhost:4840");
		uStatus = RegisterServer((OpcUa_CharA*)"opc.tcp", discoveryUrl, OpcUa_False);// "opc.tcp://localhost:4840"
		OpcUa_String_Clear(&discoveryUrl);
	}
	//
	// Acquire lock.
	OpcUa_Mutex_Lock(m_hMutex);
	OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"OpenOpcUaCore Uninitializing.\n");
	g_pTheApplication->Uninitialize();


	// Release lock.
	OpcUa_Mutex_Unlock(m_hMutex);
}
OpcUa_StatusCode CServerApplication::OpenSecureChannel(
	OpcUa_UInt32      uSecureChannelId,
	OpcUa_ByteString* pbsClientCertificate,
	OpcUa_String*     pSecurityPolicy,
	OpcUa_MessageSecurityMode      uSecurityMode)
{
	OpcUa_StatusCode uStatus=OpcUa_BadSecureChannelIdInvalid;
	
	if (!FindSecureChannel(uSecureChannelId))
	{
		CSecureChannel* pSecureChannel=new CSecureChannel(
												uSecureChannelId,
												pbsClientCertificate,
												pSecurityPolicy,
												uSecurityMode);
		if (!pSecureChannel)
		{
			uStatus=OpcUa_BadOutOfMemory;
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Critical Error:>Not enough memory to create a CSecureChannel\n");
		}
		else
		{
			uStatus=AddSecureChannel(pSecureChannel);
		}
	}
	else
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Critical Error:SecureChannel already Opened\n");
	return uStatus;
}

///-------------------------------------------------------------------------------------------------
/// <summary>
/// 	============================================================================
/// 	 CServerApplication::OnEndpointCallback
/// 	============================================================================.
/// </summary>
///
/// <remarks>	Michel, 10/02/2016. </remarks>
///
/// <param name="eEvent">					The event. </param>
/// <param name="uStatus">					The status. </param>
/// <param name="uSecureChannelId">			Identifier for the secure channel. </param>
/// <param name="a_phRawRequestContext">	[in,out] If non-null, context for the ph raw request.
/// </param>
/// <param name="pbsClientCertificate"> 	[in,out] If non-null, the pbs client certificate. </param>
/// <param name="pSecurityPolicy">			[in,out] If non-null, the security policy. </param>
/// <param name="uSecurityMode">			The security mode. </param>
/// <param name="a_uRequestedLifetime"> 	The requested lifetime. </param>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode CServerApplication::OnEndpointCallback(
	OpcUa_Endpoint_Event		eEvent,
	OpcUa_StatusCode			uStatus,
	OpcUa_UInt32				uSecureChannelId,
	OpcUa_Handle*				a_phRawRequestContext,
	OpcUa_ByteString*			pbsClientCertificate,
	OpcUa_String*				pSecurityPolicy,
	OpcUa_MessageSecurityMode   uSecurityMode,
	OpcUa_UInt32				a_uRequestedLifetime)
{
	OpcUa_StatusCode uInternalStatus=OpcUa_Good;
	(void)a_phRawRequestContext;
	(void)a_uRequestedLifetime;
	OpcUa_Mutex_Lock(m_hMutex);

	switch(eEvent)
	{
		case eOpcUa_Endpoint_Event_SecureChannelOpened:
		{
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_INFO, "We receive a request to open the SecureChannel %u.\n", uSecureChannelId);
			uInternalStatus=OpenSecureChannel(uSecureChannelId,pbsClientCertificate,pSecurityPolicy,uSecurityMode);
			if (uInternalStatus==OpcUa_Good)
			{
				CSecureChannel* pSecureChannel=FindSecureChannel(uSecureChannelId);
				if (pSecureChannel)
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_INFO, "SecureChannel 0x%05x with SecureChannelId=%u was correctly openned.\r\n", pSecureChannel, uSecureChannelId);
			}
			else
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Critical Error:>OpenSecureChannel failed %05x\n",uInternalStatus);
		}
		break;
		case eOpcUa_Endpoint_Event_SecureChannelClosed:
		{
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_INFO, "Request to close SecureChannel %u \n", uSecureChannelId);
			CSecureChannel* pSecureChannel=FindSecureChannel(uSecureChannelId);
			if (pSecureChannel)
			{
				uInternalStatus=CloseSecureChannel(pSecureChannel);
				if (uInternalStatus!=OpcUa_Good)
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Critical Error:>CloseSecureChannel failed %0x05x\n",uInternalStatus);
				else
				{
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_INFO, "SecureChannel %u was close properly\n", uSecureChannelId);
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_INFO, "RemoveSecureChannel done 0x%05x uStatus=0x%05x\n", pSecureChannel, uInternalStatus);
				}
			}
			else
			{
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Critical Error:>CloseSecureChannel this channel cannot be found\n");
				uInternalStatus=OpcUa_BadSecureChannelIdInvalid;
			}
		}
		break;
		case eOpcUa_Endpoint_Event_SecureChannelOpenVerifyCertificate:
		{
			uInternalStatus=OpcUa_Good;
			if (uStatus!=OpcUa_Good)
			{
				// copy the client certificate in the rejected folder
				RejectCertificate(pbsClientCertificate);
			}
			uStatus=OpcUa_Good;
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_INFO, "SecureChannelOpenVerifyCertificate %u \n", uSecureChannelId);
		}
		break;
		case eOpcUa_Endpoint_Event_SecureChannelRenewed:
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_INFO, "SecureChannel %u renewed\n", uSecureChannelId);
			break;
		case eOpcUa_Endpoint_Event_UnsupportedServiceRequested:
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_INFO, "SecureChannel %u received a request for an unsupported service.\r\n", uSecureChannelId);
			break;
		case eOpcUa_Endpoint_Event_DecoderError:
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_INFO, "SecureChannel %u received a request that could not be decoder.\r\n", uSecureChannelId);
			break;
		case eOpcUa_Endpoint_Event_TransportConnectionClosed:
		{
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "SecureChannel %u received a TransportConnectionClosed. RawRequestContext=0x%x\r\n", uSecureChannelId, a_phRawRequestContext);
			CSecureChannel* pSecureChannel = FindSecureChannel(uSecureChannelId);
			if (pSecureChannel)
			{
				CSessionServer* pSessionServer = FindSession(pSecureChannel->GetSecureChannelId());
				{
					uInternalStatus = RemoveSessionServer(pSessionServer);
					if (uInternalStatus != OpcUa_Good)
						OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critical Error:>RemoveSessionServer failed 0x%05x\n", uInternalStatus);
					else
					{
						uInternalStatus = CloseSecureChannel(pSecureChannel);
						if (uInternalStatus != OpcUa_Good)
							OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critical Error:>CloseSecureChannel failed 0x%05x\n", uInternalStatus);
						else
							OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_INFO, "SecureChannel %u was close properly\n", uSecureChannelId);
					}
				}
			}
		}
		break;
		default:
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_INFO, "Stack reported an unknown event:%u. on SecureChannel: %u\r\n", eEvent, uSecureChannelId);
			uInternalStatus=OpcUa_BadInternalError;
			break;
	}

	OpcUa_Mutex_Unlock(m_hMutex);
	uStatus=uInternalStatus;
	return uInternalStatus;
}



//============================================================================
// CServerApplication::RegisterServer(OpcUa_CharA* szEndPointType, std::string discoveryUrl)
// register the endPoint to the discoveryUrl
//============================================================================
OpcUa_StatusCode CServerApplication::RegisterServer(OpcUa_CharA* szEndPointType, const OpcUa_String& discoveryUrl, OpcUa_Boolean bOnline)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	if (!szEndPointType)
		uStatus=OpcUa_BadInvalidArgument;
	else
	{
		if (strlen(szEndPointType)==0)
			uStatus=OpcUa_BadInvalidArgument;
		else
		{
			try
			{

#ifdef WIN32 // Linux n'a pas de LDS donc seul WIN32 aura cette fonctionnalité
				///////////////////////////////////////////////////////////////////
				// Recherche de l'ensemble des EndPoints du LDS.
				// 
				CEndpointDescriptionList* pEndpoints =new CEndpointDescriptionList();
				if (pEndpoints)
				{
					pEndpoints->clear();
					// L'ensemble des Endpoints du LDS trouvés seront retournés dans pEndpoints
					uStatus= DiscoverEndpoints(discoveryUrl,pEndpoints);
					if (uStatus==OpcUa_Good)
					{
						if (pEndpoints->size() == 0)
						{
							OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"No endpoints available for server registration.\n");
							uStatus=OpcUa_BadNothingToDo;
						}
						else
						{
							// use the first UA TCP LDS endpoint in the list (should not make a difference which one is used).
							// ce EndPoint va être utilisé pour enregistrer notre serveur auprès du LDS
							// Attention il faudrait utiliser un EndPoint non securisé pour l'enregistrement auprès du LDS
							for (OpcUa_UInt32 ii = 0; ii < pEndpoints->size(); ii++)
							{
								CEndpointDescription*  aEndpointDescription=pEndpoints->at(ii);
								OpcUa_String* pString=aEndpointDescription->GetEndpointUrl();
								if (pString)
								{
									OpcUa_CharA* pStringA=OpcUa_String_GetRawString(pString);
									if (pStringA)
									{
										if (OpcUa_StrStrA(pStringA,"opc.tcp"))//"opc.tcp"
										{
											// trust the discovery server certificate.
											// cela consiste a copier le certificat du LDS dans le certificateStore du serveur
											uStatus=TrustCertificate(aEndpointDescription->GetServerCertificate());
											if(uStatus==OpcUa_Good)
											{
												// register the server to the LDS.
												uStatus=RegisterServer(aEndpointDescription, bOnline);
												if (uStatus!=OpcUa_Good)
												{
													OpcUa_String securityMode;
													OpcUa_String_Initialize(&securityMode);
													aEndpointDescription->GetSecurityModeAsString(&securityMode);
													OpcUa_String shortPolicyUri;
													OpcUa_String_Initialize(&shortPolicyUri);
													aEndpointDescription->GetShortSecurityPolicyUri(&shortPolicyUri);
													OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Could not RegisterServer to local discovery server at %s. [%s] [%s]\r\n",
														OpcUa_String_GetRawString(&discoveryUrl),
														OpcUa_String_GetRawString(&securityMode),
														OpcUa_String_GetRawString(&shortPolicyUri));
													//return uStatus;
												}
												else
													break;
											}
											else
												OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "The LDS certificate connot be trusted. Please check your LDS\n");
										}
									}
								}
							}
						}
					}
					else
						OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"DiscoverEndpoints failed uStatus=0x%05x\n",uStatus);
				
					// clear and release pEndPoints discovered
					CEndpointDescriptionList::iterator it;
					while (!pEndpoints->empty())
					{
						it=pEndpoints->begin();
						CEndpointDescription* pEndpointDescription=*it;
						delete pEndpointDescription;
						pEndpoints->erase(it);
					}
					pEndpoints->clear();
					delete pEndpoints;
				}
				else
					uStatus=OpcUa_BadOutOfMemory;
#endif
			}
			catch (std::exception e)
			{
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Could not connect to local discovery server at %s.\r\n", OpcUa_String_GetRawString(&discoveryUrl));
				uStatus=OpcUa_Bad;
				return uStatus;
			}
		}
	}
	return uStatus;
}
void CServerApplication::EnableServerDiagnostics(OpcUa_Boolean bVal)
{
	CUAInformationModel* pInformationModel = CServerApplication::m_pTheAddressSpace;
	OpcUa_Mutex_Lock(m_hSessionsMutex);
	for (OpcUa_UInt32 i = 0; i < m_sessions.size(); i++)
	{
		CSessionServer* pSession=m_sessions.at(i);
		pSession->EnableServerDiagnostics(bVal);
	}
	// in the same time we will activate/deactivate the global indicator
	pInformationModel->EnableServerDiagnosticsDefaultValue(bVal);
	OpcUa_Mutex_Unlock(m_hSessionsMutex);
}
CSecureChannel* CServerApplication::FindSecureChannel(OpcUa_UInt32 uSecureChannelId)
{
	CSecureChannel* pSecureChannel=OpcUa_Null;
	OpcUa_Mutex_Lock(m_hSecureChannelsMutex);
	for (OpcUa_UInt32 ii=0;ii<m_pSecureChannels->size();ii++)
	{
		CSecureChannel* pTmpSecureChannel=m_pSecureChannels->at(ii);
		if (pTmpSecureChannel->GetSecureChannelId() == uSecureChannelId)
		{
			pSecureChannel=pTmpSecureChannel;
			break;
		}
	}
	OpcUa_Mutex_Unlock(m_hSecureChannelsMutex);
	return pSecureChannel;
}
OpcUa_StatusCode CServerApplication::AddSecureChannel(CSecureChannel* pSecureChannel)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_Mutex_Lock(m_hSecureChannelsMutex);
	m_pSecureChannels->push_back(pSecureChannel);
	OpcUa_Mutex_Unlock(m_hSecureChannelsMutex);
	return uStatus;
}
OpcUa_StatusCode CServerApplication::CloseSecureChannel(CSecureChannel* pChannel)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	CSessionServer* pSessionServer=FindSession(pChannel->GetSecureChannelId());
	if (!pSessionServer)
		uStatus=RemoveSecureChannel(pChannel);	
	else
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Error. This SecureChannel is associated with a pending Session\n");
	return uStatus;
}
OpcUa_StatusCode CServerApplication::RemoveAllSecureChannel()
{
	OpcUa_StatusCode uStatus = OpcUa_BadNothingToDo;
	OpcUa_Mutex_Lock(m_hSecureChannelsMutex);
	for (CSecureChannelList::iterator it=m_pSecureChannels->begin();it!=m_pSecureChannels->end();it++)
	{
		uStatus=OpcUa_Good;
		CSecureChannel* pSecureChannel=*it;
		delete pSecureChannel;
	}
	m_pSecureChannels->clear();
	OpcUa_Mutex_Unlock(m_hSecureChannelsMutex);
	return uStatus;
}
OpcUa_StatusCode CServerApplication::RemoveSecureChannel(CSecureChannel* pChannel)
{
	OpcUa_StatusCode uStatus = OpcUa_BadInvalidArgument;
	if (pChannel)
	{
		OpcUa_Mutex_Lock(m_hSecureChannelsMutex);
		for (CSecureChannelList::iterator it = m_pSecureChannels->begin(); it != m_pSecureChannels->end(); it++)
		{
			CSecureChannel* pSecureChannel=*it;
			if (pSecureChannel->GetSecureChannelId() == pChannel->GetSecureChannelId())
			{
				delete pSecureChannel;
				m_pSecureChannels->erase(it);
				pSecureChannel=OpcUa_Null;
				uStatus=OpcUa_Good;
				break;
			}
		}
		OpcUa_Mutex_Unlock(m_hSecureChannelsMutex);
	}
	return uStatus;
}
//============================================================================
// CServerApplication::RegisterServer(EndpointDescription)
//============================================================================
OpcUa_StatusCode CServerApplication::RegisterServer(CEndpointDescription* pEndpoint, OpcUa_Boolean bOnline)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_RequestHeader tRequestHeader;
	OpcUa_RegisteredServer tRegisteredServer;
	OpcUa_ResponseHeader tResponseHeader;


	// initialize parameters.
	OpcUa_RequestHeader_Initialize(&tRequestHeader);
	OpcUa_ResponseHeader_Initialize(&tResponseHeader);
	OpcUa_RegisteredServer_Initialize(&tRegisteredServer);
	CChannel* pChannel=new CChannel(this);
	if (pChannel)
	{
		// connect to the server.
		uStatus=pChannel->Connect(pEndpoint);
		if (uStatus!=OpcUa_Good)
		{
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Could not connect to local discovery server. RegisterServer failed: 0x%05x\n", uStatus);
		}
		else
		{
			// create the header.
			tRequestHeader.TimeoutHint = UTILS_DEFAULT_TIMEOUT;
			tRequestHeader.Timestamp   = OpcUa_DateTime_UtcNow();

			CApplicationDescription* pAppDescription=GetApplicationDescription();
			OpcUa_RegisteredServer_Initialize(&tRegisteredServer);
			// Fill the record used to register the server.  
			// Il s'agit de remplir la structure OpcUa_RegisteredServer (tRegisteredServer)
			// ApplicationUri
			OpcUa_String* pString=pAppDescription->GetApplicationUri();
			OpcUa_String_CopyTo(pString, &(tRegisteredServer.ServerUri));
			// Product Uri
			pString=pAppDescription->GetProductUri();
			OpcUa_String_CopyTo(pString, &(tRegisteredServer.ProductUri));
			// ServerNames to register
			OpcUa_LocalizedText* aLocalizedText=GetApplicationName();
			tRegisteredServer.NoOfServerNames = 1;
			tRegisteredServer.ServerNames = (OpcUa_LocalizedText*)OpcUa_Alloc(sizeof(OpcUa_LocalizedText));
			OpcUa_LocalizedText_Initialize(&tRegisteredServer.ServerNames[0]);
			OpcUa_LocalizedText_CopyTo(aLocalizedText, &(tRegisteredServer.ServerNames[0]));
			// ApplicationType
			tRegisteredServer.ServerType =pAppDescription->GetApplicationType();
			// Application Url
			tRegisteredServer.NoOfDiscoveryUrls = 1;
			OpcUa_String* aDiscoveries = pAppDescription->GetDiscoveryUrls();
			tRegisteredServer.DiscoveryUrls = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));

			OpcUa_String_Initialize(&(tRegisteredServer.DiscoveryUrls[0]));
			OpcUa_String_CopyTo(&aDiscoveries[0], &(tRegisteredServer.DiscoveryUrls[0]));
			// Ajout d'un separateur /
			// create the simple separator
			OpcUa_String* pszSeparatorUrl1=(OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
			OpcUa_String_Initialize(pszSeparatorUrl1);
			OpcUa_String_AttachCopy(pszSeparatorUrl1,(OpcUa_StringA)"/");

			OpcUa_String_StrnCat(&(tRegisteredServer.DiscoveryUrls[0]),
				pszSeparatorUrl1,
				OpcUa_String_StrLen(pszSeparatorUrl1) );
			OpcUa_String_Clear(pszSeparatorUrl1);
			OpcUa_Free(pszSeparatorUrl1);
			// Ajout de discovery
			OpcUa_String* Discovery=(OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
			OpcUa_String_Initialize(Discovery);
			OpcUa_String_AttachCopy(Discovery,"discovery");

			OpcUa_String_StrnCat(&(tRegisteredServer.DiscoveryUrls[0]),
				Discovery,
				OpcUa_String_StrLen(Discovery) );
			OpcUa_String_Clear(Discovery);
			OpcUa_Free(Discovery);
			// The is online flag is used for servers that cannot be auto launched (i.e. someone needs to start the process first).
			// This is the case for all servers implemented with the ANSI C stack.
			// online servers need to periodically re-register with the LDS.
			tRegisteredServer.IsOnline = bOnline;

			// register the server to the LDS.
			uStatus = OpcUa_ClientApi_RegisterServer(  
				pChannel->GetInternalHandle(), 
				&tRequestHeader,
				&tRegisteredServer,
				&tResponseHeader);
			if (uStatus!=OpcUa_Good)
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Could not register server with discovery server. 0x%05x\n",uStatus);
			if (tResponseHeader.ServiceResult!=OpcUa_Good)
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Could not register server with discovery server.0x%05x\n",tResponseHeader.ServiceResult);
			// clean up.
		}
		delete pChannel;
	}
	else
		uStatus=OpcUa_BadOutOfMemory;
	OpcUa_RegisteredServer_Clear(&tRegisteredServer);
	OpcUa_RequestHeader_Clear(&tRequestHeader);
	OpcUa_ResponseHeader_Clear(&tResponseHeader);

	return uStatus;
}

//============================================================================
// InitializeEndpointDescription
//============================================================================
OpcUa_StatusCode CServerApplication::InitializeEndpointDescription(
	OpcUa_EndpointDescription* pEndpoint,
	OpcUa_String endpointUrl,
	OpcUa_String applicationUri,
	OpcUa_String productUri,
	OpcUa_LocalizedText* applicationName,
	OpcUa_ByteString* pCertificate,
	OpcUa_MessageSecurityMode securityMode,
	OpcUa_String securityPolicyUri,
	OpcUa_Byte securityLevel)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_EndpointDescription_Initialize(pEndpoint);
	/* 
	typedef struct _OpcUa_UserTokenPolicy
	{
		OpcUa_String        PolicyId;
		OpcUa_UserTokenType TokenType;
		OpcUa_String        IssuedTokenType;
		OpcUa_String        IssuerEndpointUrl;
		OpcUa_String        SecurityPolicyUri;
	}
	OpcUa_UserTokenPolicy;
	*/

	OpcUa_String_CopyTo(&endpointUrl, &(pEndpoint->EndpointUrl));

	OpcUa_String_CopyTo(&applicationUri, &(pEndpoint->Server.ApplicationUri));
	OpcUa_String_CopyTo(&productUri, &(pEndpoint->Server.ProductUri));
	OpcUa_LocalizedText_CopyTo(applicationName, &(pEndpoint->Server.ApplicationName));


	pEndpoint->Server.ApplicationType = OpcUa_ApplicationType_Server;
	pEndpoint->Server.NoOfDiscoveryUrls = 1;
	pEndpoint->Server.DiscoveryUrls = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pEndpoint->Server.DiscoveryUrls);
	OpcUa_String_CopyTo(&endpointUrl, &(pEndpoint->Server.DiscoveryUrls[0]));
	//
	pEndpoint->SecurityMode = securityMode;

	OpcUa_String_CopyTo(&securityPolicyUri, &(pEndpoint->SecurityPolicyUri));

	// initialization of UserIdentityToken
	pEndpoint->NoOfUserIdentityTokens = 3;
	pEndpoint->UserIdentityTokens=(OpcUa_UserTokenPolicy*)OpcUa_Alloc(sizeof(OpcUa_UserTokenPolicy)*(pEndpoint->NoOfUserIdentityTokens));
	// Anonymous
	OpcUa_UserTokenPolicy_Initialize(&(pEndpoint->UserIdentityTokens[0]));
	pEndpoint->UserIdentityTokens[0].TokenType=OpcUa_UserTokenType_Anonymous;
	OpcUa_String_AttachCopy(&(pEndpoint->UserIdentityTokens[0].PolicyId),"OpenOpcUaAnonymousPolicyId");
	// UserName
	OpcUa_UserTokenPolicy_Initialize(&(pEndpoint->UserIdentityTokens[1]));
	pEndpoint->UserIdentityTokens[1].TokenType = OpcUa_UserTokenType_UserName;
	OpcUa_String_AttachCopy(&(pEndpoint->UserIdentityTokens[1].PolicyId),"OpenOpcUaUserNamePolicyId");
	// X509Identity
	OpcUa_UserTokenPolicy_Initialize(&(pEndpoint->UserIdentityTokens[2]));
	pEndpoint->UserIdentityTokens[2].TokenType = OpcUa_UserTokenType_Certificate;
	OpcUa_String_AttachCopy(&(pEndpoint->UserIdentityTokens[2].PolicyId),"OpenOpcUaCertificatePolicyId");


	pEndpoint->SecurityLevel = securityLevel;
	// Let check the TransportProfile 
	// We will use the endpointUrl is the following way :
	// if it contains https ==> OpcUa_TransportProfile_SoapHttpsUaXml
	// if it contains opc.tcp ==> OpcUa_TransportProfile_UaTcp
	OpcUa_CharA* chTmp=OpcUa_String_GetRawString(&endpointUrl);

	OpcUa_CharA* chOpcTransport=(OpcUa_CharA*)OpcUa_Alloc(8);
	ZeroMemory(chOpcTransport,8);
	OpcUa_MemCpy(chOpcTransport,7,(OpcUa_Void *)"opc.tcp",7);

	OpcUa_CharA* pDest=OpcUa_StrStrA(chTmp,"opc.tcp");// https
	if ((pDest-chTmp)==0)
		OpcUa_String_AttachCopy(&pEndpoint->TransportProfileUri, OpcUa_TransportProfile_UaTcp);
	else
		OpcUa_String_AttachCopy(&pEndpoint->TransportProfileUri, OpcUa_TransportProfile_HttpsSoapUaXml);
	OpcUa_Free(chOpcTransport);
	OpcUa_ByteString_Initialize(&(pEndpoint->ServerCertificate));
	if (pCertificate)
	{
		if (pCertificate->Data) // // 1.02 change for ERRATA
			OpcUa_ByteString_CopyTo(pCertificate,&(pEndpoint->ServerCertificate));
	}

	return uStatus;
}

//============================================================================
// CServerApplication::GetServerEndpoints
//============================================================================
OpcUa_StatusCode CServerApplication::GetServerEndpoints(
	OpcUa_Int32                 nNoOfProfileUris, // in
	const OpcUa_String*         pProfileUris, // in
	OpcUa_Int32*                pNoOfEndpoints,
	OpcUa_EndpointDescription** pEndpoints)	
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	// count the the number of endpoint descriptions specified by the security policies.
	(*pNoOfEndpoints) = 0;
	CApplicationDescription* pAppDescription=GetApplicationDescription();
	// traitement de l'ApplicationUri
	OpcUa_String* sApplicationUri=(OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	if (sApplicationUri)
	{
		OpcUa_String_Initialize(sApplicationUri);
		OpcUa_String* aString = pAppDescription->GetApplicationUri();
		OpcUa_String_CopyTo(aString, sApplicationUri);
		// traitement de l'ApplicationUrl
		OpcUa_String* szApplicationUrl = OpcUa_Null;
		if (pAppDescription->GetNoOfDiscoveryUrls() > 0)
		{
			szApplicationUrl = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
			if (szApplicationUrl)
			{
				OpcUa_String_Initialize(szApplicationUrl);
				OpcUa_String* strDiscoveries = pAppDescription->GetDiscoveryUrls();

				OpcUa_String_StrnCpy(szApplicationUrl, strDiscoveries, OpcUa_String_StrLen(strDiscoveries));
			}
			else
				uStatus = OpcUa_BadOutOfMemory;
		}
		if (uStatus == OpcUa_Good)
		{
			// append the applicationName
			OpcUa_LocalizedText* sApplicationName = OpcUa_Null;//(OpcUa_LocalizedText*)OpcUa_Alloc(sizeof(OpcUa_LocalizedText));
			sApplicationName = GetApplicationName();
			if (sApplicationName)
			{
				if (OpcUa_String_StrLen(&(sApplicationName->Text)) > 0)
					OpcUa_String_StrnCat(szApplicationUrl,
					&(sApplicationName->Text),
					OpcUa_String_StrLen(&(sApplicationName->Text)));
			}
			// traitement du ProductUri
			OpcUa_String* szProductUri = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
			OpcUa_String_Initialize(szProductUri);
			aString = pAppDescription->GetProductUri();
			if (OpcUa_String_StrLen(aString) > 0)
				OpcUa_String_StrnCpy(szProductUri, aString, OpcUa_String_StrLen(aString));
			// Verification des filtres potentiellement passé par le client
			OpcUa_UInt32 iCheckedWithFilter = 0;
			for (OpcUa_UInt32 ii = 0; ii<m_ServerBindingList.size(); ii++)
			{
				CUABinding* pBinding = (CUABinding*)m_ServerBindingList.at(ii);
				OpcUa_String aProfileUri = pBinding->GetTransportProfileUri();
				if (nNoOfProfileUris>0)
				{
					for (OpcUa_Int32 iii = 0; iii < nNoOfProfileUris; iii++)
					{
						if (OpcUa_String_Compare(&aProfileUri, &pProfileUris[iii]) == 0)
						{
							iCheckedWithFilter++;
						}
					}
				}
			}
			if (iCheckedWithFilter)
				(*pNoOfEndpoints) = iCheckedWithFilter*m_nNoOfSecurityPolicies;
			else
			{
				if (nNoOfProfileUris>0)
					(*pNoOfEndpoints) = 0;
				else
					(*pNoOfEndpoints) = m_nNoOfSecurityPolicies*m_ServerBindingList.size();
			}
			// on va créer les OpcUa_EndpointDescription à partir des Binding 
			// qui sont definis dans le fichier de configuration et stocké dans m_ServerBindingList
			// On va avoir m_ServerBindingList.size() * 3 pEndPoints. 
			//	3 car il y a 3 mode de sécurité supporté par le serveur OpenOpcUa
			//		None OPCUA_ENDPOINT_MESSAGESECURITYMODE_NONE
			//		Basic128Rsa15 OPCUA_ENDPOINT_MESSAGESECURITYMODE_SIGN
			//		Basic128Rsa15 OPCUA_ENDPOINT_MESSAGESECURITYMODE_SIGNANDENCRYPT
			if ((*pNoOfEndpoints) > 0)
			{
				(*pEndpoints) = (OpcUa_EndpointDescription*)OpcUa_Alloc((*pNoOfEndpoints)*sizeof(OpcUa_EndpointDescription));
				if (*pEndpoints)
				{
					OpcUa_EndpointDescription* pEndpointDescription = *pEndpoints;
					for (OpcUa_UInt32 ii = 0; ii<m_ServerBindingList.size(); ii++)
					{
						CUABinding* pBinding = (CUABinding*)m_ServerBindingList.at(ii);
						OpcUa_String aProfileUri = pBinding->GetTransportProfileUri();
						OpcUa_EndpointDescription_Initialize(pEndpointDescription);
						if (nNoOfProfileUris>0)
						{
							for (OpcUa_Int32 iii = 0; iii < nNoOfProfileUris; iii++)
							{
								if (OpcUa_String_Compare(&aProfileUri, &pProfileUris[iii]) == 0)
								{
									for (int iiii = 0; iiii < m_nNoOfSecurityPolicies; iiii++)
									{
										OpcUa_Endpoint_SecurityPolicyConfiguration* pSecurityPolicy = &m_pSecurityPolicies[iiii];

										// lowest security level.
										if ((pSecurityPolicy->uMessageSecurityModes & OPCUA_ENDPOINT_MESSAGESECURITYMODE_NONE) != 0)
										{
											InitializeEndpointDescription(
												pEndpointDescription,
												*szApplicationUrl,
												*sApplicationUri,
												*szProductUri,
												sApplicationName,
												GetCertificate(),
												OpcUa_MessageSecurityMode_None,
												pSecurityPolicy->sSecurityPolicy,
												0);
											pEndpointDescription++;
										}

										// medium security level.
										if ((pSecurityPolicy->uMessageSecurityModes & OPCUA_ENDPOINT_MESSAGESECURITYMODE_SIGN) != 0)
										{

											InitializeEndpointDescription(
												pEndpointDescription,
												*szApplicationUrl,
												*sApplicationUri,
												*szProductUri,
												sApplicationName,
												GetCertificate(),
												OpcUa_MessageSecurityMode_Sign,
												pSecurityPolicy->sSecurityPolicy,
												1);
											pEndpointDescription++;
										}

										// highest security level.
										if ((pSecurityPolicy->uMessageSecurityModes & OPCUA_ENDPOINT_MESSAGESECURITYMODE_SIGNANDENCRYPT) != 0)
										{
											InitializeEndpointDescription(
												pEndpointDescription,
												*szApplicationUrl,
												*sApplicationUri,
												*szProductUri,
												sApplicationName,
												GetCertificate(),
												OpcUa_MessageSecurityMode_SignAndEncrypt,
												pSecurityPolicy->sSecurityPolicy,
												2);
											pEndpointDescription++;
										}
									}
								}
								else
								{
									OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "unexpected case\n");
								}
							}
						}
						else
						{
							for (OpcUa_Int32 iii = 0; iii < m_nNoOfSecurityPolicies; iii++)
							{
								OpcUa_Endpoint_SecurityPolicyConfiguration* pSecurityPolicy = &m_pSecurityPolicies[iii];

								// lowest security level.
								if ((pSecurityPolicy->uMessageSecurityModes & OPCUA_ENDPOINT_MESSAGESECURITYMODE_NONE) != 0)
								{
									if (szApplicationUrl)
									{
										InitializeEndpointDescription(
											pEndpointDescription,
											*szApplicationUrl,
											*sApplicationUri,
											*szProductUri,
											sApplicationName,
											GetCertificate(),
											OpcUa_MessageSecurityMode_None,
											pSecurityPolicy->sSecurityPolicy,
											0);
										pEndpointDescription++;
									}
								}

								// medium security level.
								if ((pSecurityPolicy->uMessageSecurityModes & OPCUA_ENDPOINT_MESSAGESECURITYMODE_SIGN) != 0)
								{
									if (szApplicationUrl)
									{
										InitializeEndpointDescription(
											pEndpointDescription,
											*szApplicationUrl,
											*sApplicationUri,
											*szProductUri,
											sApplicationName,
											GetCertificate(),
											OpcUa_MessageSecurityMode_Sign,
											pSecurityPolicy->sSecurityPolicy,
											1);
										pEndpointDescription++;
									}
								}

								// highest security level.
								if ((pSecurityPolicy->uMessageSecurityModes & OPCUA_ENDPOINT_MESSAGESECURITYMODE_SIGNANDENCRYPT) != 0)
								{
									if (szApplicationUrl)
									{
										InitializeEndpointDescription(
											pEndpointDescription,
											*szApplicationUrl,
											*sApplicationUri,
											*szProductUri,
											sApplicationName,
											GetCertificate(),
											OpcUa_MessageSecurityMode_SignAndEncrypt,
											pSecurityPolicy->sSecurityPolicy,
											2);
										pEndpointDescription++;
									}
								}
							}
						}
					}
				}
				else
				{
					*pNoOfEndpoints = 0;
					uStatus = OpcUa_BadOutOfMemory;
				}
			}

			// Free ressouces
			if (sApplicationUri)
			{
				OpcUa_String_Clear(sApplicationUri);
				OpcUa_Free(sApplicationUri);
			}
			if (szApplicationUrl)
			{
				OpcUa_String_Clear(szApplicationUrl);
				OpcUa_Free(szApplicationUrl);
			}
			if (szProductUri)
			{
				OpcUa_String_Clear(szProductUri);
				OpcUa_Free(szProductUri);
			}
		}
		else
			uStatus = OpcUa_BadOutOfMemory;
	}
	else
		uStatus = OpcUa_BadOutOfMemory;
	return uStatus;
}

//============================================================================
// CServerApplication::GetEndpoints
//============================================================================
OpcUa_StatusCode CServerApplication::GetEndpoints(
	OpcUa_UInt32                uSecureChannelId,
	const OpcUa_RequestHeader*  pRequestHeader,
	const OpcUa_String*         pEndpointUrl,
	OpcUa_Int32                 nNoOfLocaleIds,
	const OpcUa_String*         pLocaleIds,
	OpcUa_Int32                 nNoOfProfileUris, // in
	const OpcUa_String*         pProfileUris, // in
	OpcUa_ResponseHeader*       pResponseHeader,
	OpcUa_Int32*                pNoOfEndpoints,
	OpcUa_EndpointDescription** pEndpoints)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	if (pRequestHeader)
	{
		OpcUa_ReferenceParameter(uSecureChannelId);
		OpcUa_ReferenceParameter(pEndpointUrl);
		OpcUa_ReferenceParameter(nNoOfLocaleIds);
		OpcUa_ReferenceParameter(pLocaleIds);
		OpcUa_ReferenceParameter(pResponseHeader);
		OpcUa_ReferenceParameter(nNoOfProfileUris);
		OpcUa_ReferenceParameter(pProfileUris);

		*pNoOfEndpoints = 0;
		*pEndpoints = NULL;

		// Acquire lock.
		OpcUa_Mutex_Lock(m_hMutex);

		// Get if the UA-TCP transport profile requested.
	//    bool requested = nNoOfProfileUris == 0;

	//    if (nNoOfProfileUris > 0)
	//    {
	//        for (int ii = 0; ii < nNoOfProfileUris; ii++)
	//        {
	   //         OpcUa_StringA sProfile = OpcUa_String_GetRawString((OpcUa_String*)&pProfileUris[ii]);
				//// recherche du TransportProfile "http://opcfoundation.org/UA-Profile/Transport/uatcp-uasc-uabinary"
	   //         if (strcmp(OpcUa_TransportProfile_UaTcp, sProfile) == 0)
	   //         {
		  //          requested = true;
		  //          break;
	   //         }
	//        }
	//    }

	//    if (requested)
		uStatus=GetServerEndpoints(nNoOfProfileUris,pProfileUris,pNoOfEndpoints,pEndpoints);

		OpcUa_Mutex_Unlock(m_hMutex);
	}
	else
		uStatus=OpcUa_BadInvalidArgument;
	return uStatus;

}

//============================================================================
// CServerApplication::CreateSession
//============================================================================
OpcUa_StatusCode CServerApplication::CreateSession(
	OpcUa_UInt32                        a_uSecureChannelId,// in
	OpcUa_MessageSecurityMode           a_eSecurityMode,// in
	OpcUa_String                        a_securityPolicyUri,// in
	const OpcUa_RequestHeader*          a_pRequestHeader,// in
	const OpcUa_ApplicationDescription* a_pClientDescription,// in
	const OpcUa_String*                 a_pServerUri,// in
	const OpcUa_String*                 a_pEndpointUrl,// in
	const OpcUa_String*                 a_pSessionName,// in
	const OpcUa_ByteString*             a_pClientNonce,// in
	const OpcUa_ByteString*             a_pClientCertificate,// in
	OpcUa_Double                        a_nRequestedSessionTimeout,// in
	OpcUa_UInt32                        a_nMaxResponseMessageSize,// in
	OpcUa_ResponseHeader*               a_pResponseHeader, // out
	OpcUa_NodeId*                       a_pSessionId,// out
	OpcUa_NodeId*                       a_pAuthenticationToken,// out
	OpcUa_Double*                       a_pRevisedSessionTimeout,// out
	OpcUa_ByteString*                   a_pServerNonce,// out
	OpcUa_ByteString*                   a_pServerCertificate,// out
	OpcUa_Int32*                        a_pNoOfServerEndpoints,// out
	OpcUa_EndpointDescription**         a_pServerEndpoints,// out
	OpcUa_SignatureData*                a_pServerSignature,// out
	OpcUa_UInt32*                       a_pMaxRequestMessageSize)// out
{
	OpcUa_StatusCode uStatus=OpcUa_BadInvalidArgument;
	// first check clientNonce
	if (!IsClientNonceExist(a_uSecureChannelId,a_pClientNonce))
	{
		OpcUa_Guid tGuid;
		OpcUa_Guid_Initialize(&tGuid);
		CSessionServer* pSession = 0;
		if (a_pServerNonce)
		{
			OpcUa_ReferenceParameter(a_pRequestHeader);
			//OpcUa_ReferenceParameter(a_pClientDescription);
			OpcUa_ReferenceParameter(a_pServerUri);
			OpcUa_ReferenceParameter(a_pEndpointUrl);
			OpcUa_ReferenceParameter(a_pResponseHeader);

			OpcUa_Mutex_Lock(m_hMutex);
			/////////////////////////////////////////////////////////////////////////////
			// Creation de l'Id de la session
			// the session id is supposed to be the node id of the session diagnostics.
			a_pSessionId->NamespaceIndex = 1;
			a_pSessionId->IdentifierType = OpcUa_IdentifierType_Numeric;
			// Il faut check que ce n° de session ne soit pas déjà utilisé
			// Il est impossible de boucler forever dans ce code
			uStatus = OpcUa_Good;
			CUAInformationModel* pInformationModel = CServerApplication::m_pTheAddressSpace;
			if (pInformationModel)
			{
				while (uStatus == OpcUa_Good)
				{
					a_pSessionId->Identifier.Numeric = ++m_lastSessionId;
					CUABase* pUABase = NULL;
					uStatus = pInformationModel->GetNodeIdFromDictionnary(*a_pSessionId, &pUABase);
				}
			}
			/////////////////////////////////////////////////////////////////////////////
			// the authentication token must be kept secret because it is
			// used to identify the session in each request.
			// use a Guid to reduce the chance of valid tokens being guessed.
			OpcUa_Guid_Create(&tGuid);
			a_pAuthenticationToken->NamespaceIndex = 1;
			a_pAuthenticationToken->IdentifierType = OpcUa_IdentifierType_Guid;
			a_pAuthenticationToken->Identifier.Guid = Utils::Copy(&tGuid);

			// get the server endpoints.
			uStatus = GetServerEndpoints(0, OpcUa_Null, a_pNoOfServerEndpoints, a_pServerEndpoints);
			if (uStatus != OpcUa_Good)
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "CServerApplication::CreateSession>GetServerEndpoints failed uStatus=0x%05x\n", uStatus);

			// create the sesssion.
			OpcUa_UInt16 uiSecurityMode = (OpcUa_UInt16)a_eSecurityMode;
			pSession = new CSessionServer(this,
				a_uSecureChannelId,
				uiSecurityMode,
				a_securityPolicyUri,
				a_pClientDescription,
				a_pSessionName,
				a_pClientNonce,
				a_pClientCertificate,
				a_nRequestedSessionTimeout,
				a_nMaxResponseMessageSize,
				a_pSessionId,
				a_pAuthenticationToken,
				a_pRevisedSessionTimeout,
				a_pServerNonce,
				a_pServerSignature,
				a_pMaxRequestMessageSize);
			if (pSession)
			{
				// copy the server certificate.
				OpcUa_ByteString_Initialize(a_pServerCertificate);
				if (pSession->Is101Session())
				{
					OpcUa_ByteString* pCertificate = GetCertificate();
					OpcUa_ByteString_CopyTo(pCertificate, a_pServerCertificate);
				}
				// Set the EndPontDescription
				if (a_pServerEndpoints)
				{
					CEndpointDescription* pEndpointDescription = new CEndpointDescription(*a_pServerEndpoints);
					pSession->SetEndpointDescription(pEndpointDescription);
				}
				if (pInformationModel->IsEnabledServerDiagnosticsDefaultValue())
				{
					pSession->InitSessionDiagnosticsDataType();
					pSession->InitSessionSecurityDiagnosticsDataType();
				}
				// save the session.
				if (m_sessions.size() < MAX_SESSION_SUPPORTED)
				{
					AddSessionServer(pSession);
					uStatus = OpcUa_Good;
				}
				else
					uStatus = OpcUa_BadTooManySessions;
			}
			else
				uStatus = OpcUa_BadOutOfMemory;

			OpcUa_Mutex_Unlock(m_hMutex);
		}
		else
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "CServerApplication::CreateSession>a_pServerNonce receive from the stack is NULL\n");
	}
	else
		uStatus=OpcUa_BadNonceInvalid;
	return uStatus;
}
// Recherche de la première session qui est associé a ce uSecureChannelId
CSessionServer* CServerApplication::FindSession(OpcUa_UInt32 uSecureChannelId)
{
	CSessionServer* pSession=OpcUa_Null;
	OpcUa_Mutex_Lock(m_hSessionsMutex);
	for (CSessionServerList::iterator it = m_sessions.begin(); it != m_sessions.end(); ++it)
	{
		if ((*it)->GetSecureChannel()->GetSecureChannelId()==uSecureChannelId)
		{
			pSession=*it;
			break;
		}
	}
	OpcUa_Mutex_Unlock(m_hSessionsMutex);
	return pSession;
}
//============================================================================
// CServerApplication::FindSession based on its SecureChannelId and its AuthenticationToken (NodeId).  
//	 Because each session is associated with a unique AuthenticationToken+SecureChannelId
//============================================================================
OpcUa_StatusCode CServerApplication::FindSession(const OpcUa_UInt32 uSecureChannelId,const OpcUa_NodeId* pAuthenticationToken, CSessionServer** pSession)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	// Il faut faire la difference entre AuthenticationToken inconnu et AuthenticationToken + SecureChannelId inconnu
	OpcUa_Mutex_Lock(m_hSessionsMutex);
	for (CSessionServerList::iterator it = m_sessions.begin(); it != m_sessions.end(); ++it)
	{
		if ( ((*it)->IsAuthenticationToken(pAuthenticationToken)) && ((*it)->GetSecureChannel()->GetSecureChannelId()== uSecureChannelId) )
		{
			(*pSession)=(*it);
			break;
		}
	}
	// en cas d'erreur on va essayer d'affiner la cause
	if (!(*pSession))
	{
		// 
		uStatus=OpcUa_BadSessionIdInvalid;
		for (CSessionServerList::iterator it = m_sessions.begin(); it != m_sessions.end(); ++it)
		{
			if ((*it)->IsAuthenticationToken(pAuthenticationToken))
			{
				// on a trouvé un AuthenticationToken. Comme il y avait eu une erreur precedement il ne peut s'agir que d'un SecureChannelIdInvalid
				uStatus=OpcUa_BadSecureChannelIdInvalid;
			}
		}
	}
	OpcUa_Mutex_Unlock(m_hSessionsMutex);
	return uStatus;
}
OpcUa_StatusCode CServerApplication::FindSessionBySessionId(OpcUa_NodeId* pSessionId, CSessionServer** pSession)
{
	OpcUa_StatusCode uStatus = OpcUa_BadNotFound;
	OpcUa_Mutex_Lock(m_hSessionsMutex);
	for (CSessionServerList::iterator it = m_sessions.begin(); it != m_sessions.end(); ++it)
	{
		OpcUa_NodeId sessionId=(*it)->GetSessionId();
		if ( Utils::IsEqual(&sessionId,pSessionId))
		{
			(*pSession)=(*it);
			uStatus = OpcUa_Good;
			break;
		}
	}
	OpcUa_Mutex_Unlock(m_hSessionsMutex);
	return uStatus;
}
CSessionServer* CServerApplication::FindSession(const OpcUa_NodeId* pAuthenticationToken)
{
	CSessionServer* pSession=OpcUa_Null;
	OpcUa_Mutex_Lock(m_hSessionsMutex);
	for (CSessionServerList::iterator it = m_sessions.begin(); it != m_sessions.end(); ++it)
	{
		if ( ((*it)->IsAuthenticationToken(pAuthenticationToken)))
		{
			pSession=(*it);
			break;
		}
	}
	OpcUa_Mutex_Unlock(m_hSessionsMutex);
	return pSession;
}
//============================================================================
// CServerApplication::FindSession based on its SecureChannelId,
//          Because each session is associated with a unique SecureChannelId
//============================================================================
//CSessionServer* CServerApplication::FindSession(OpcUa_UInt32  uSecureChannelId)
//{
//	CSessionServer* pSession=OpcUa_Null;
//	OpcUa_Mutex_Lock(m_hSessionsMutex);
//	for (CSessionServerList::iterator it = m_sessions.begin(); it != m_sessions.end(); ++it)
//	{
//		if ((*it)->GetSecureChannelId()==uSecureChannelId)
//			return pSession;
//	}
//	OpcUa_Mutex_Unlock(m_hSessionsMutex);
//	return pSession;
//}
//============================================================================
// CServerApplication::ActivateSession
//============================================================================
OpcUa_StatusCode CServerApplication::ActivateSession(
	OpcUa_UInt32							a_uSecureChannelId, // in
	OpcUa_MessageSecurityMode				a_eSecurityMode, // in
	OpcUa_String*							a_securityPolicyUri, // in
	OpcUa_Int32								a_nNoOfClientSoftwareCertificates, // in
	const OpcUa_SignedSoftwareCertificate*	a_pClientSoftwareCertificates, //in 
	const OpcUa_ExtensionObject*            a_pUserIdentityToken,
	const OpcUa_RequestHeader*				a_pRequestHeader, //out
	const OpcUa_SignatureData*				a_pClientSignature, // out
	OpcUa_ResponseHeader*					a_pResponseHeader, // out
	OpcUa_ByteString*						a_pServerNonce) // out
{
	CSessionServer* pSession = 0;
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_StatusCode uServiceResult=OpcUa_Good;
	OpcUa_ReferenceParameter(a_pRequestHeader);
	OpcUa_ReferenceParameter(a_pResponseHeader);

	OpcUa_Mutex_Lock(m_hMutex);

	// find the session.
	pSession = FindSession(&a_pRequestHeader->AuthenticationToken);

	if (pSession)
	{
		// Signal that the session is alive
		OpcUa_Semaphore_Post(pSession->m_SessionTimeoutSem,1);
		if (pSession->GetSecureChannel()->GetSecureChannelId()!=a_uSecureChannelId)
		{
			//OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"CServerApplication::ActivateSession>Secure Channel change>Start\n");
			// Option d'implementation N° 1 on creer une nouvelle session qui utilisera le nouveau canal de communication securi
			// Il faut alors :
			// 1- Détruire l'ancienne session. 
			// 2- En créer une nouvelle qui reprenne l'ensemble des caractéristiques de la nouvelle.
			//		a- m_tAuthenticationToken (a_pRequestHeader->AuthenticationToken)
			//		b- m_tServerNonce
			// 3- Détruire l'ancienne session 
			// 4- La supprimer de la liste
			// 5- Activer la nouvelle session normalement
			//CSessionServer*  pSessionServer=new CSessionServer(a_securityPolicyUri,a_uSecureChannelId, pSession);
			//if (pSessionServer)
			//{
			//	OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"CServerApplication::ActivateSession>Creation du nouveau CSessionServer. ok\n");
			//	pSession->SetSessionValidity(OpcUa_False);
			//	uStatus=RemoveSessionServer(pSession);
			//	if (uStatus==OpcUa_Good)
			//	{
			//		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"CServerApplication::ActivateSession>Suppréssion de l'ancien CSessionServer. ok\n");
			//		pSession=pSessionServer;
			//		// save the session.OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"CServerApplication::ActivateSession>Stockage du noubeau CSessionServer. ok\n");
			//		AddSessionServer(pSessionServer);
			//	}
			//	else
			//		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"CServerApplication::ActivateSession>Suppréssion de l'ancien CSessionServer. echec\n");
			//}
			
			// Option N°2 on va changer le canal de la session existante
			CSecureChannel* pSecureChannel=FindSecureChannel(a_uSecureChannelId);
			if (pSecureChannel)
			{
				pSession->SetSecureChannel(pSecureChannel);
			}
			else
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"CServerApplication::ActivateSession>The SecureChannel %u doesn't exist\n",a_uSecureChannelId);
		}
		if (!pSession->IsTimeouted()) // pour être conforme avec les attentes du CTT script err-004.js Session Base
		{
			// activate the session.
			OpcUa_UInt16 uiSecurityMode = (OpcUa_UInt16)a_eSecurityMode;
			uServiceResult = pSession->Activate(
				a_uSecureChannelId,
				uiSecurityMode,
				a_securityPolicyUri,
				a_nNoOfClientSoftwareCertificates, // in
				a_pClientSoftwareCertificates, //in 
				a_pClientSignature,
				a_pUserIdentityToken, // in
				a_pServerNonce);
		}
		else
			uServiceResult=OpcUa_BadSessionClosed;

	}
	else
		uServiceResult=OpcUa_BadSessionIdInvalid;

	// Update the response header.
	a_pResponseHeader->Timestamp     = OpcUa_DateTime_UtcNow();
	a_pResponseHeader->RequestHandle = a_pRequestHeader->RequestHandle;
	a_pResponseHeader->ServiceResult = uServiceResult;

	OpcUa_Mutex_Unlock(m_hMutex);
	return uStatus;
}
	
//============================================================================
// CServerApplication::CloseSession
//============================================================================
OpcUa_StatusCode CServerApplication::CloseSession(
	OpcUa_UInt32               a_uSecureChannelId,
	const OpcUa_RequestHeader* a_pRequestHeader,
	OpcUa_Boolean              a_bDeleteSubscriptions,
	OpcUa_ResponseHeader*      a_pResponseHeader)
{

	OpcUa_ReferenceParameter(a_bDeleteSubscriptions);
	CSessionServer* pSession = OpcUa_Null;
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_StatusCode uServiceResult = OpcUa_Good;

	OpcUa_Mutex_Lock(m_hMutex);

	// find the session.
	uStatus=FindSession(a_uSecureChannelId,&a_pRequestHeader->AuthenticationToken,&pSession);
	if (uStatus==OpcUa_Good)
	{
		// check secure channel.
		if (pSession->GetSecureChannel()->GetSecureChannelId() != a_uSecureChannelId)
			uStatus=OpcUa_BadSecureChannelIdInvalid;
		else
		{
			// remove the session from the SessionDiagnosticsArray
			/*
			CSessionDiagnosticsDataType* pSessionDiagnosticsDataType=pSession->GetSessionDiagnosticsDataType();
			if (pSessionDiagnosticsDataType)
				m_pTheAddressSpace->RemoveInSessionDiagnosticsArray(pSessionDiagnosticsDataType);
			else
			{
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Warning in this CSessionServer instance pSessionDiagnosticsDataType is Null\n");
				uStatus=OpcUa_Good;
			}
			// remove the session from the SessionSecurityDiagnosticsArray
			CSessionSecurityDiagnosticsDataType* pSessionSecurityDiagnosticsDataType=pSession->GetSessionSecurityDiagnosticsDataType();
			if (pSessionSecurityDiagnosticsDataType)
				m_pTheAddressSpace->RemoveInSessionSecurityDiagnosticsArray(pSessionSecurityDiagnosticsDataType);
			else
			{
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Warning in this CSessionServer instance pSessionSecurityDiagnosticsDataType is Null\n");
				uStatus=OpcUa_Good;
			}
			*/
			// remove the session.
			if (!pSession->IsActive())
				uServiceResult = OpcUa_BadSessionNotActivated;
			uStatus=RemoveSessionServer(pSession);
		}
	}
	OpcUa_Mutex_Unlock(m_hMutex);
	// Update the response header.
	a_pResponseHeader->Timestamp     = OpcUa_DateTime_UtcNow();
	a_pResponseHeader->RequestHandle = a_pRequestHeader->RequestHandle;
	a_pResponseHeader->ServiceResult = uServiceResult;

	return uStatus;
}


OpcUa_StatusCode CServerApplication::Browse(OpcUa_UInt32 uSecureChannelId,
		const OpcUa_RequestHeader*		pRequestHeader, // input
		const OpcUa_ViewDescription*	pView, // input
		OpcUa_UInt32					uiRequestedMaxReferencesPerNode, // intput
		OpcUa_UInt32					iNoOfNodesToBrowse, // intput
		const OpcUa_BrowseDescription*	pNodesToBrowse, // intput Tableau de OpcUa_BrowseDescription de taille iNoOfNodesToBrowse
		OpcUa_ResponseHeader*			pResponseHeader, // output
		OpcUa_Int32*					pNoOfResults, // output
		OpcUa_BrowseResult**			ppResults, // output
		OpcUa_Int32*					pNoOfDiagnosticInfos, // output
		OpcUa_DiagnosticInfo**			ppDiagnosticInfos) // output
{
	CSessionServer* pSession = 0;
	CUAView* pUAView = NULL; // use in case a OpcUa_ViewDescription is specified
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_StatusCode uServiceResult = OpcUa_Good;
	OpcUa_ReferenceParameter(uiRequestedMaxReferencesPerNode);
	OpcUa_ReferenceParameter(pView);
	try
	{
		// 
		CUAInformationModel* pInformationModel = CServerApplication::m_pTheAddressSpace;
		if (pInformationModel)
		{
			if (pView)
			{
				if ((pView->ViewId.IdentifierType == OpcUa_IdentifierType_Numeric) && (pView->ViewId.Identifier.Numeric == 0))
					uStatus = OpcUa_Good;
				else
				{
					uStatus = pInformationModel->GetNodeIdFromViewList(pView->ViewId, &pUAView);
					if (uStatus != OpcUa_Good)
						uStatus = OpcUa_BadViewIdUnknown;
				}		
			}
			// Now will check uStatus in case the pView we got was not conform
			if (uStatus == OpcUa_Good)
			{
				// find the session.
				uStatus = FindSession(uSecureChannelId, &pRequestHeader->AuthenticationToken, &pSession);
				if (uStatus == OpcUa_Good)
				{
					// 
					// Signal that the session is alive
					OpcUa_Semaphore_Post(pSession->m_SessionTimeoutSem, 1);
					// we will browse here
					//veteur que contient la liste des resultats
					std::vector<OpcUa_BrowseResult*> lBrowseResults;
					// Nombre de référence a retouner
					OpcUa_BrowseResult* aNewBrowseResult = NULL;
					/////////////////////////////////////////////////////
					//
					// First step : 
					// Browse of the pNodesToBrowse set by the client
					//
					/////////////////////////////////////////////////////
					OpcUa_UInt32 ii;
					for (ii = 0; ii < iNoOfNodesToBrowse; ii++)
					{
						// creation du continuation point pour ce BrowseResult
						CContinuationPoint* pContinuationPoint = new CContinuationPoint();
						if (pContinuationPoint)
						{
							// Allocation du browseResult qui contiendra le resultat provisoire
							aNewBrowseResult = (OpcUa_BrowseResult*)OpcUa_Alloc(sizeof(OpcUa_BrowseResult));
							if (aNewBrowseResult)
							{
								OpcUa_BrowseResult_Initialize(aNewBrowseResult);
								// We must check that this ReferenceTypeId is a valid one
								CUAReferenceType* pUAReferenceType = OpcUa_Null;
								uStatus = pInformationModel->IsReferenceTypeIdValid(pNodesToBrowse[ii].ReferenceTypeId, &pUAReferenceType);
								if (uStatus == OpcUa_Good)
								{
									uServiceResult = pInformationModel->BrowseOneNode(pNodesToBrowse[ii],
										uiRequestedMaxReferencesPerNode,
										&aNewBrowseResult,
										&pContinuationPoint);
									if ((uServiceResult != OpcUa_Good) || (uiRequestedMaxReferencesPerNode == 0))
									{
										// Ce NodeId est absent du dictionnaire
										aNewBrowseResult->ContinuationPoint.Data = OpcUa_Null;
										aNewBrowseResult->ContinuationPoint.Length = 0;
									}
								}
								else
								{
									// Ce ReferenceTypeId est absent du dictionnaire
									aNewBrowseResult->ContinuationPoint.Data = OpcUa_Null;
									aNewBrowseResult->ContinuationPoint.Length = 0;
									aNewBrowseResult->StatusCode = uStatus;  //OpcUa_BadReferenceTypeIdInvalid;
									uServiceResult = OpcUa_Good;
									uStatus = OpcUa_Good;
								}
								// ContinuationPoint
								if (pContinuationPoint->GetBrowseDescription())
								{
									if ((pSession->m_pContinuationPointList) && (uiRequestedMaxReferencesPerNode > 0))
									{
										if (lBrowseResults.size() >= OPENOPCUA_MAX_BROWSE_CP)
										{
											aNewBrowseResult->StatusCode = OpcUa_BadNoContinuationPoints;
											delete pContinuationPoint;
											aNewBrowseResult->ContinuationPoint.Data = OpcUa_Null;
											aNewBrowseResult->ContinuationPoint.Length = 0;
										}
										else
										{
											pSession->m_pContinuationPointList->push_back(pContinuationPoint);
										}
									}
									else
									{
										aNewBrowseResult->ContinuationPoint.Data = OpcUa_Null;
										aNewBrowseResult->ContinuationPoint.Length = 0;
										delete pContinuationPoint;
									}
								}
								else
								{
									aNewBrowseResult->ContinuationPoint.Data = OpcUa_Null;
									aNewBrowseResult->ContinuationPoint.Length = 0;
									delete pContinuationPoint;
								}
								lBrowseResults.push_back(aNewBrowseResult);
							}
						}
					} // fin for (ii=0;ii<iNoOfNodesToBrowse;ii++)
					// Continuation point

					/////////////////////////////////////////////////////
					//
					// Second step: 
					// Organisation of the response
					//
					/////////////////////////////////////////////////////
					if (lBrowseResults.size() > 0)
					{
						OpcUa_BrowseResult* pTmpResults = (OpcUa_BrowseResult*)OpcUa_Alloc(lBrowseResults.size()*sizeof(OpcUa_BrowseResult));
						*pNoOfResults = lBrowseResults.size();
						for (OpcUa_UInt32 iii = 0; iii < lBrowseResults.size(); iii++)
						{
							OpcUa_BrowseResult_Initialize(&pTmpResults[iii]);
							OpcUa_BrowseResult* pBrowseResult = lBrowseResults.at(iii);
							if ((pNodesToBrowse[iii].ResultMask&OpcUa_BrowseResultMask_BrowseName) != OpcUa_BrowseResultMask_BrowseName)
							{
								// Remove BroweName from the BrowseResult
								for (OpcUa_Int32 i=0;i<pBrowseResult->NoOfReferences;i++)
									OpcUa_QualifiedName_Clear(&(pBrowseResult->References[i].BrowseName));
							}
							if ((pNodesToBrowse[iii].ResultMask&OpcUa_BrowseResultMask_DisplayName) != OpcUa_BrowseResultMask_DisplayName)
							{
								// Remove BroweName from the BrowseResult
								for (OpcUa_Int32 i=0;i<pBrowseResult->NoOfReferences;i++)
									OpcUa_LocalizedText_Clear(&(pBrowseResult->References[i].DisplayName));
							}
							OpcUa_MemCpy(&pTmpResults[iii],sizeof(OpcUa_BrowseResult),pBrowseResult,sizeof(OpcUa_BrowseResult));
							OpcUa_Free(pBrowseResult);
						}
						(*ppResults) = pTmpResults;
					}
				}
				else
					uServiceResult = OpcUa_BadSecureChannelIdInvalid;

			}
		}
		else
			uStatus = OpcUa_BadInternalError;
		// Fin de la mise en forme du resultat
		*pNoOfDiagnosticInfos = 0;
		*ppDiagnosticInfos = OpcUa_Null; // (OpcUa_DiagnosticInfo*)OpcUa_Alloc(sizeof(OpcUa_DiagnosticInfo));
		//ZeroMemory(*ppDiagnosticInfos, sizeof(OpcUa_DiagnosticInfo));
		//
		pResponseHeader->ServiceResult = uServiceResult;
		pResponseHeader->Timestamp = OpcUa_DateTime_UtcNow();
		pResponseHeader->RequestHandle = pRequestHeader->RequestHandle; // return in the answer as specified in the specification part 4 - §7.26
		OpcUa_ReferenceParameter(pRequestHeader);
		OpcUa_ReferenceParameter(pResponseHeader);

	}
	catch (...)
	{
		throw;
	}
	return uStatus;
}


OpcUa_StatusCode CServerApplication::BrowseNext(OpcUa_UInt32 uSecureChannelId, // in
		const OpcUa_RequestHeader* a_pRequestHeader, // in
		OpcUa_Boolean              a_bReleaseContinuationPoints, // in
		OpcUa_Int32                a_nNoOfContinuationPoints, // in
		const OpcUa_ByteString*    a_pContinuationPoints, // in
		OpcUa_ResponseHeader*      a_pResponseHeader,
		OpcUa_Int32*               a_pNoOfResults,
		OpcUa_BrowseResult**       a_pResults,
		OpcUa_Int32*               a_pNoOfDiagnosticInfos,
		OpcUa_DiagnosticInfo**     a_pDiagnosticInfos)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	CSessionServer* pSession = 0;
	(void)a_pResponseHeader;
	CUAInformationModel* pInformationModel=CServerApplication::m_pTheAddressSpace;
	if (a_pRequestHeader)
	{
		(*a_pNoOfResults)=a_nNoOfContinuationPoints;
		(*a_pResults)=(OpcUa_BrowseResult*)OpcUa_Alloc(sizeof(OpcUa_BrowseResult)*a_nNoOfContinuationPoints);

		for (OpcUa_Int32 ii=0;ii<a_nNoOfContinuationPoints;ii++)
		{
			OpcUa_BrowseResult_Initialize(&(*a_pResults)[ii]);
			if (ii>=OPENOPCUA_MAX_BROWSE_CP)
				(*a_pResults)[ii].StatusCode=OpcUa_BadNoContinuationPoints;
			else
			{
				OpcUa_ByteString pContinuationPoint=a_pContinuationPoints[ii];
				if (pContinuationPoint.Length>0)
				{
					{ 
						// find the session.
						uStatus=FindSession(uSecureChannelId,&a_pRequestHeader->AuthenticationToken,&pSession);
						if (uStatus==OpcUa_Good)
						{
							if (pSession->m_pContinuationPointList->size()>0)
							{
								OpcUa_UInt32* iCurrentIndex=(OpcUa_UInt32*)pContinuationPoint.Data;
								if (iCurrentIndex)
								{
									OpcUa_UInt32 iIndex=pSession->m_pContinuationPointList->size();
									if (iIndex>0)
									{
										CContinuationPoint* pClassContinuationPoint = OpcUa_Null; 
										for (OpcUa_UInt32 i = 0; i < iIndex; i++)
										{
											CContinuationPoint* pTmpContinuationPoint = pSession->m_pContinuationPointList->at(i);
											if (pTmpContinuationPoint->GetCurrentIndex() == (*iCurrentIndex))
												pClassContinuationPoint = pTmpContinuationPoint;
										}
										// Verification que le sequence des CP est bien respectée. Si elle ne l'ai pas en renvoi OpcUa_BadContinuationPointInvalid
										if (!pClassContinuationPoint)
										{
											(*a_pResults)[ii].StatusCode=OpcUa_BadContinuationPointInvalid;
										}
										else
										{
											OpcUa_BrowseDescription* pBrowseDescription= pClassContinuationPoint->GetBrowseDescription();
											OpcUa_UInt32 uiRequestedMaxReferencesPerNode=pClassContinuationPoint->GetRequestedMaxReferencesPerNode();
											if (!a_bReleaseContinuationPoints)
											{
												if (pBrowseDescription)
												{
													OpcUa_UInt32 iOldIndex=pClassContinuationPoint->GetCurrentIndex();
													uStatus=pInformationModel->BrowseOneNode(*pBrowseDescription,
																							 uiRequestedMaxReferencesPerNode,
																							 a_pResults, //&a_pTmpResults,
																							 &pClassContinuationPoint);										
													if (pClassContinuationPoint->GetCurrentIndex()==iOldIndex)
													{										
														(*a_pResults)[ii].ContinuationPoint.Data=OpcUa_Null;
														(*a_pResults)[ii].ContinuationPoint.Length=0;
													}
												}
												else
												{
													(*a_pResults)[ii].ContinuationPoint.Data=OpcUa_Null;
													(*a_pResults)[ii].ContinuationPoint.Length=0;
												}

											}
											else
											{
												// Ok, on va maintenant detruire les continuationPoint en cours
												OpcUa_BrowseResult_Initialize(&(*a_pResults)[ii]);
												for (OpcUa_UInt32 kk=0;kk<pSession->m_pContinuationPointList->size();kk++)
												{
													CContinuationPoint* pTmpContinuationPoint=pSession->m_pContinuationPointList->at(kk);
													delete pTmpContinuationPoint;
												}
												pSession->m_pContinuationPointList->clear();
												(*a_pResults)[ii].StatusCode=OpcUa_Good;
												(*a_pResults)[ii].NoOfReferences=0;
												(*a_pResults)[ii].ContinuationPoint.Data=OpcUa_Null;
												(*a_pResults)[ii].ContinuationPoint.Length=0;
												(*a_pResults)[ii].References=OpcUa_Null;
												uStatus=OpcUa_Good; 
											}
										}
									}
									else
										OpcUa_BrowseResult_Initialize(&(*a_pResults)[ii]);
								}
								else
									OpcUa_BrowseResult_Initialize(&(*a_pResults)[ii]);


							}
							else
							{
								OpcUa_BrowseResult_Initialize(&(*a_pResults)[ii]);
								(*a_pResults)[ii].StatusCode=OpcUa_BadContinuationPointInvalid;
								(*a_pResults)[ii].NoOfReferences=0;
								(*a_pResults)[ii].ContinuationPoint.Data=OpcUa_Null;
								(*a_pResults)[ii].ContinuationPoint.Length=0;
								(*a_pResults)[ii].References=OpcUa_Null;
								uStatus=OpcUa_Good; 
							}
						}
						else
							OpcUa_BrowseResult_Initialize(&(*a_pResults)[ii]);
					}
				}
				else
				{
					(*a_pResults)[ii].StatusCode=OpcUa_BadContinuationPointInvalid;
					(*a_pResults)[ii].NoOfReferences=0;
					(*a_pResults)[ii].ContinuationPoint.Data=OpcUa_Null;
					(*a_pResults)[ii].ContinuationPoint.Length=0;
					(*a_pResults)[ii].References=OpcUa_Null;
					uStatus = OpcUa_Good; // OpcUa_BadContinuationPointInvalid;
				}
			}
		}
		// Fin de la mise en forme du resultat
		if (a_pRequestHeader->ReturnDiagnostics > 0)
		{
			*a_pNoOfDiagnosticInfos = 1;
			(*a_pDiagnosticInfos) = (OpcUa_DiagnosticInfo*)OpcUa_Alloc(sizeof(OpcUa_DiagnosticInfo));
			OpcUa_DiagnosticInfo_Initialize(&(*a_pDiagnosticInfos)[0]);
			(*a_pDiagnosticInfos)[0].InnerStatusCode = uStatus;
		}
		else
		{
			*a_pNoOfDiagnosticInfos = 0;
			*a_pDiagnosticInfos = OpcUa_Null;// (OpcUa_DiagnosticInfo*)OpcUa_Alloc(sizeof(OpcUa_DiagnosticInfo));
		}
		//ZeroMemory(*a_pDiagnosticInfos,sizeof(OpcUa_DiagnosticInfo));
	}
	return uStatus;
}

OpcUa_StatusCode CServerApplication::AddBinding(CUABinding* pBinding)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	if (pBinding)
	{
		m_ServerBindingList.push_back(pBinding);
	}
	else
		uStatus=OpcUa_BadInvalidArgument;
	return uStatus;
}

//////////////////////////////////////////////////////////////////////
// 
// Thread assurant le renouvelement de l'inscription auprès du LDS
// 
//////////////////////////////////////////////////////////////////////
void CServerApplication::StartLDSRegistrationThread()
{
	if (m_hLDSRegistrationThread==NULL)
	{
		m_bRunLDSRegistrationThread=OpcUa_True;
		OpcUa_StatusCode uStatus  = OpcUa_Thread_Create(&m_hLDSRegistrationThread,LDSRegistrationThread,this);

		if(uStatus != OpcUa_Good)
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Create AsyncRequestThread Failed\n");
		else
			OpcUa_Thread_Start(m_hLDSRegistrationThread);
	}
}
void CServerApplication::LDSRegistrationThread(LPVOID arg)
{
	OpcUa_Boolean bColdStart=TRUE;
	CServerApplication* pServerApplication=(CServerApplication*)arg;
	OpcUa_UInt32 uiInterval = pServerApplication->GetLDSRegistrationInterval();
	OpcUa_UInt32 uiSleepTime=uiInterval;
#ifdef WIN32 
	OpcUa_StatusCode uStatus=OpcUa_Good;
#endif
	OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_INFO, "LDSRegistrationThread just started\n");
	while (pServerApplication->m_bRunLDSRegistrationThread)
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
		if (bColdStart)
		{
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_INFO, "LDSRegistrationThread>Proceed ServerColdStart.\n");
			pServerApplication->Start();
			bColdStart=FALSE;
		}
#ifdef WIN32 
		else
		{
			if (pServerApplication->m_bLDSRegistrationActive)
			{
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_INFO, "LDSRegistrationThread Register to LDS\n");
				OpcUa_String discoveryUrl;
				OpcUa_String_Initialize(&discoveryUrl);
				OpcUa_String_AttachCopy(&discoveryUrl, "opc.tcp://localhost:4840");
				uStatus = pServerApplication->RegisterServer("opc.tcp", discoveryUrl, OpcUa_True);// "opc.tcp://localhost:4840"
				// register the server to the Local Discovery Server
				if (uStatus != OpcUa_Good)
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Cannot register to LDS...uStatus=0x%05x\n", uStatus);
				else
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_INFO, "LDSRegistrationThread registration succeeded\n");
				OpcUa_String_Clear(&discoveryUrl);
			}
		}
#endif
		//
#if _WIN32_WINNT >= 0x0600
		ULONGLONG dwEnd = GetTickCount64();
		ULONGLONG dwCountedTime = dwEnd - dwStart;
		if (uiInterval>dwCountedTime)
			uiSleepTime = (OpcUa_UInt32)(uiInterval - dwCountedTime);
		else
			uiSleepTime = 0;
#else 
	#if _WIN32_WINNT == 0x0501
		DWORD dwEnd = GetTickCount();
		DWORD dwCountedTime = dwEnd - dwStart;
		if (uiInterval>dwCountedTime)
			uiSleepTime = uiInterval - dwCountedTime;
		else
			uiSleepTime = 0;
	#endif
#endif
#ifdef _GNUC_
		DWORD dwEnd=GetTickCount();
		OpcUa_UInt32 dwCountedTime=dwEnd-dwStart;
		if (uiInterval>dwCountedTime)
			uiSleepTime=uiInterval-dwCountedTime;
		else
			uiSleepTime=0;
		uiInterval=pServerApplication->GetLDSRegistrationInterval();
#endif
		OpcUa_Semaphore_TimedWait( pServerApplication->m_hLDSRegistrationRequest, uiSleepTime );
	}
	OpcUa_Semaphore_Post(pServerApplication->m_hStopLDSRegistrationSem, 1);
	OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"CServerApplication::LDSRegistrationThread stopped\n");
}
OpcUa_StatusCode CServerApplication::StopLDSRegistrationThread()
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	if (m_bRunLDSRegistrationThread)
	{
		m_bRunLDSRegistrationThread=OpcUa_False;
		OpcUa_Semaphore_Post(m_hLDSRegistrationRequest, 1);
		uStatus = OpcUa_Semaphore_TimedWait(m_hStopLDSRegistrationSem, OPC_TIMEOUT * 2); // 15 secondes max.
		if (uStatus == OpcUa_GoodNonCriticalTimeout)
		{
			// on force la fin du thread de simulation
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Impossible to stop the LDSRegistrationThread. Timeout\n");
		}
		else
		{
			OpcUa_Thread_Delete(m_hLDSRegistrationThread);
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "LDSRegistrationThread stopped properly\n");
		}
	}
	return uStatus;
}


OpcUa_StatusCode CServerApplication::LoadUaServerConfiguration(char* path,char* fileName)
{
	XML_Parser pParser;
	FILE* parseFile=OpcUa_Null;
	OpcUa_StatusCode uStatus=OpcUa_Bad;

	if (!path)
		uStatus=OpcUa_BadInvalidArgument;
	else
	{
		if (!fileName)
			uStatus=OpcUa_BadInvalidArgument;
		else
		{
			uStatus = xml4CE_SAXOpenParser(path, fileName, &parseFile, &pParser);
			if (uStatus == OpcUa_Good)
			{
				uStatus = xml4CE_SAXSetElementHandler(&pParser, xmlConfigStartElementHandler, xmlConfigEndElementHandler);
				if (uStatus == OpcUa_Good)
				{
					HANDLER_CONFIG_DATA xmlConfigDataHandler;
					xmlConfigDataHandler.pServerApplication=this;
					xmlConfigDataHandler.userData=NULL;
					xmlConfigDataHandler.XML_Parser=pParser;
					
					xml4CE_SAXSetUserData(&pParser,(void*)&xmlConfigDataHandler);
					if (xml4CE_SAXParseBegin(parseFile,&pParser)==OpcUa_Good)
					{
						OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Your XML configuration file : %s%s has been parsed\n",path,fileName);
						uStatus=OpcUa_Good;
					}
				}
				else
					uStatus=OpcUa_BadBoundNotSupported;
				// will close parseFile and the parser
				Xml4CE_SAXCloseParser(&pParser);
				fclose(parseFile);
			}
		}
	}
	return uStatus;
}
OpcUa_StatusCode CServerApplication::LoadUaServerSubsystems(char* path,char* fileName)
{
	XML_Parser pParser;
	FILE* parseFile;
	OpcUa_StatusCode uStatus=OpcUa_Bad;
	// Erase all the Namesoace Uri loaded for the previous SubSystem File
	CServerApplication::m_pTheAddressSpace->OnLoadNamespaceUriEraseAll();
	uStatus = xml4CE_SAXOpenParser(path, fileName, &parseFile, &pParser);
	if (uStatus == OpcUa_Good)
	{
		uStatus = xml4CE_SAXSetCharacterDataHandler(&pParser, xmlSubSystemCharacterDataHandler);
		if (uStatus == OpcUa_Good)
		{
			uStatus = xml4CE_SAXSetElementHandler(&pParser, xmlSubSystemStartElementHandler, xmlSubSystemEndElementHandler);
			if (uStatus == OpcUa_Good)
			{
				HANDLER_SUBSYSTEM_DATA xmlDataHandler;
				xmlDataHandler.userData = NULL;
				xmlDataHandler.pDevice = NULL;
				xmlDataHandler.pSignal = NULL;
				xmlDataHandler.uStatus = OpcUa_Good;
				xmlDataHandler.bNamespaceUri = OpcUa_False;
				xml4CE_SAXSetUserData(&pParser, (void*)&xmlDataHandler);
				if (xml4CE_SAXParseBegin(parseFile, &pParser) == OpcUa_Good)
				{
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Your XML Subsystem file : %s%s has been parsed. Internal parsing result=0x%05x\n", path, fileName, xmlDataHandler.uStatus);
					uStatus = OpcUa_Good;
				}
			}
		}
		// will close parseFile and parser
		Xml4CE_SAXCloseParser(&pParser);
		fclose(parseFile);
	}
	else
	{
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Cannot load your XML Subsystem file : %s%s \n",path,fileName);
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"The syntax is incorrect in the Main XML Config file\n");
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Press any key to exit.\r\n");
		getQChar();
	}
	return uStatus;
}
OpcUa_StatusCode CServerApplication::LoadUaServerSimulation(char* path,char* fileName)
{
	XML_Parser pParser;
	FILE* parseFile;
	OpcUa_StatusCode uStatus=OpcUa_Bad;
	CServerApplication::m_pTheAddressSpace->OnLoadNamespaceUriEraseAll();
	uStatus = xml4CE_SAXOpenParser(path, fileName, &parseFile, &pParser);
	if (uStatus == OpcUa_Good)
	{
		uStatus = xml4CE_SAXSetCharacterDataHandler(&pParser, xmlSimulationCharacterDataHandler);
		if (uStatus == OpcUa_Good)
		{
			uStatus = xml4CE_SAXSetElementHandler(&pParser, xmlSimulationStartElementHandler, xmlSimulationEndElementHandler);
			if (uStatus == OpcUa_Good)
			{
				HANDLER_SIMULATION_DATA xmlDataHandler;
				xmlDataHandler.userData = NULL;
				xmlDataHandler.pSimulatedGroup = NULL;
				xmlDataHandler.uStatus = OpcUa_Good;
				xml4CE_SAXSetUserData(&pParser, (void*)&xmlDataHandler);
				if (xml4CE_SAXParseBegin(parseFile, &pParser) == OpcUa_Good)
				{
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Your XML Simulation file : %s%s has been parsed. Parsing result=0x%05x\n", path, fileName, xmlDataHandler.uStatus);
					uStatus = OpcUa_Good;
				}
			}
		}
		// will close parseFile and parse file
		Xml4CE_SAXCloseParser(&pParser);
		fclose(parseFile);
	}
	else
	{
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Cannot load your XML Simulation file : %s%s \n",path,fileName);
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"The syntax is incorrect in the Main XML Config file\n");
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Press any key to exit.\r\n");
		getQChar();
	}
	return uStatus;
}
OpcUa_StatusCode CServerApplication::LoadUaServerNodeSet(char* path,char* fileName)
{
	XML_Parser pParser;
	FILE* parseFile=OpcUa_Null;
	OpcUa_StatusCode uStatus=OpcUa_Bad;

	// Erase all  the namespaceUri loaded for the last loaded NodeSet file
	CServerApplication::m_pTheAddressSpace->OnLoadNamespaceUriEraseAll();
	// reset de la table de translation
	// avec la nouvelle implementation on conserve les table de traduction associée a chaque UANodeSet
	OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Now will try to parse: %s%s\n",path,fileName);
	
	uStatus=xml4CE_SAXOpenParser(path,fileName,&parseFile,&pParser);
	if (uStatus==OpcUa_Good)
	{
		uStatus=xml4CE_SAXSetCharacterDataHandler(&pParser,xmlNodeSetCharacterDataHandler);
		if (uStatus==OpcUa_Good)
		{
			uStatus=xml4CE_SAXSetElementHandler(&pParser,xmlNodeSetStartElementHandler,xmlNodeSetEndElementHandler);
			if (uStatus==OpcUa_Good)
			{
				HANDLER_DATA xmlDataHandler;
				xmlDataHandler.XML_Parser=pParser;
				xmlDataHandler.userData=OpcUa_Null;
				xmlDataHandler.NodeClass=OpcUa_NodeClass_Unspecified;
				xmlDataHandler.pUAObject=OpcUa_Null;
				xmlDataHandler.pUAReferenceType=OpcUa_Null;
				xmlDataHandler.pUAObjectType=OpcUa_Null;
				xmlDataHandler.pAlias=OpcUa_Null;
				xmlDataHandler.pMethod=OpcUa_Null;
				xmlDataHandler.pUADataType=OpcUa_Null;
				xmlDataHandler.pUAVariable=OpcUa_Null;
				xmlDataHandler.pUAVariableType=OpcUa_Null;
				xmlDataHandler.pView=OpcUa_Null;
				xmlDataHandler.pDataValue=OpcUa_Null;
				xmlDataHandler.pReferenceNode=OpcUa_Null;
				xmlDataHandler.bDescription=OpcUa_False;
				xmlDataHandler.bDisplayName=OpcUa_False;
				xmlDataHandler.bInverseName=OpcUa_False;
				xmlDataHandler.bValue=OpcUa_False;
				xmlDataHandler.bNamespaceUri=OpcUa_False;
				xmlDataHandler.pField=OpcUa_Null;
				xmlDataHandler.pDefinition=OpcUa_Null;
				xmlDataHandler.bLocale=OpcUa_False;
				xmlDataHandler.bText=OpcUa_False;
				xmlDataHandler.szCurrentXmlElement = OpcUa_Null;
				// initialisation des attributs relatif au extensionObject
				xmlDataHandler.bExtensionObject=OpcUa_False;
				// EnumValueType
				xmlDataHandler.pEnumValueType=OpcUa_Null;
				xmlDataHandler.bExtensionObjectEnumValueType=OpcUa_False;
				xmlDataHandler.bExtensionObjectEnumValueTypeValue=OpcUa_False;
				xmlDataHandler.bExtensionObjectEnumValueTypeDisplayName=OpcUa_False;
				xmlDataHandler.bExtensionObjectEnumValueTypeDescription=OpcUa_False;
				// Argument
				xmlDataHandler.pArgument=OpcUa_Null;
				xmlDataHandler.bExtensionObjectArgument=OpcUa_False;
				xmlDataHandler.bExtensionObjectArrayDimensions=OpcUa_False;
				xmlDataHandler.bExtensionObjectBody=OpcUa_False;
				xmlDataHandler.bExtensionObjectDataType=OpcUa_False;
				xmlDataHandler.bExtensionObjectDescription=OpcUa_False;
				xmlDataHandler.bExtensionObjectIdentifier=OpcUa_False;
				xmlDataHandler.bExtensionObjectName=OpcUa_False;
				xmlDataHandler.bExtensionObjectTypeId=OpcUa_False;
				xmlDataHandler.bExtensionObjectValueRank=OpcUa_False;
				xmlDataHandler.uiArrayCurrentElt=0;
				OpcUa_String_Initialize(&(xmlDataHandler.tmpString));
				xmlDataHandler.pLocalizedText = OpcUa_Null;
				xmlDataHandler.uStatus = OpcUa_Good;
				xml4CE_SAXSetUserData(&pParser,(void*)&xmlDataHandler);
				//
				
				//
				uStatus=xml4CE_SAXParseBegin(parseFile,&pParser);
				if (uStatus==OpcUa_Good)
				{
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Your XML configuration file : %s%s has been parsed. AddressSpace initialization result=0x%05x\n", path, fileName, xmlDataHandler.uStatus);
					// Will look for and try to fix the orphan dataType declaration
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "fixing the orphan dataType declaration\n");
					CUADataTypeList* pUADataTypeList=CServerApplication::m_pTheAddressSpace->GetDataTypeList();
					int iSize=pUADataTypeList->size();
					for (int iii=0;iii<iSize;++iii)
					{
						CUADataType* pUADataType=((*pUADataTypeList)[iii]);
						if (pUADataType)
						{
							if (pUADataType->GetAncestorType()==0)
							{	
								// Found an orphan ;
								if (!pUADataType->IsAbstract())
								{
									OpcUa_NodeId aNodeId = pUADataType->GetParentType();
									CUADataType*  pUAParentDataType = OpcUa_Null;
									if (CServerApplication::m_pTheAddressSpace->GetNodeIdFromDataTypeList(aNodeId, &pUAParentDataType) == OpcUa_Good)
									{
										pUADataType->SetAncestorType(pUAParentDataType->GetAncestorType());
									}
									else
									{
										char* szNodeId = OpcUa_Null;
										Utils::NodeId2String(&aNodeId, &szNodeId);
										if (szNodeId)
										{
											OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "GetNodeIdFromDataTypeList failed on NodeId : %s \n", szNodeId);
											free(szNodeId);
										}
									}
								}
							}
						}
					}	
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Orphan dataType declaration fixed\n");
				}
			}
		}
		// will close parseFile and parser
		Xml4CE_SAXCloseParser(&pParser);
		if (fclose(parseFile) == EOF)
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Cannot close the current NodeSet file\n");
	}
	else
	{
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Cannot load your XML NodeSet file : %s%s \n",path,fileName);
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"The syntax is incorrect in the Main XML Config file\n");
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Press any key to exit.\r\n");		
		getQChar();
	}
	return uStatus;
}



// Function name   : CServerApplication::LookupEncodeableType
// Description     : recherche le OpcUa_EncodeableType correspondant au TypeId (int32) passé en paramètre
//					 La recherche est éffectué dans la stack sur la base des m_tTypeTable pris en charge par la stack AnsiC V1
// Return type     : OpcUa_StatusCode OpcUa_Good si le type a été trouvé
// Argument        : OpcUa_Int32 iTypeId = TypeId a recherche
// Argument        : OpcUa_EncodeableType** pEncodeableType [Out] OpcUa_EncodeableType trouvé

OpcUa_StatusCode CServerApplication::LookupEncodeableType(OpcUa_UInt32 iTypeId,OpcUa_EncodeableType** pEncodeableType)
{
	OpcUa_StatusCode uStatus=OpcUa_BadNotFound;
	// initialisation de l'encodableType
	OpcUa_EncodeableTypeTable* pTypeTable=GetTypeTable();
	if (pTypeTable)
	{
		// Save pointer
		OpcUa_EncodeableType* pOldEntries=pTypeTable->Entries;
		for (OpcUa_Int32 ii=0;ii<pTypeTable->Count;ii++)
		{
			
			if ( (pTypeTable->Entries->TypeId==iTypeId) || (pTypeTable->Entries->XmlEncodingTypeId==iTypeId) )
			{
				*pEncodeableType = pTypeTable->Entries;
				uStatus=OpcUa_Good;
				/* *pEncodeableType=(OpcUa_EncodeableType*)OpcUa_Alloc(sizeof(pTypeTable->Entries[ii]));
				if (*pEncodeableType)
				{
					OpcUa_MemCpy(*pEncodeableType,sizeof(pTypeTable->Entries[ii]),&pTypeTable->Entries[ii],sizeof(pTypeTable->Entries[ii]));
					uStatus=OpcUa_Good;
				}
				else
				{
					uStatus = OpcUa_BadOutOfMemory;
				}*/
				break;
			}
			pTypeTable->Entries++;
		}
		// Restore old pointer
		pTypeTable->Entries = pOldEntries;
	}
	return uStatus;
}
OpcUa_UInt32 CServerApplication::UpdateTimeoutInterval()
{
	OpcUa_UInt32 uiFastest=15000; // 15 sec
	OpcUa_Mutex_Lock(m_hSessionsMutex);
	for (CSessionServerList::iterator it=m_sessions.begin();it!=m_sessions.end();it++)
	{
		CSessionServer* pSession=*it;
		if (((OpcUa_UInt32)pSession->GetSessionTimeout())<uiFastest)
			uiFastest=(OpcUa_UInt32)pSession->GetSessionTimeout();
	}
	OpcUa_Mutex_Unlock(m_hSessionsMutex);
	return uiFastest;
}
void CServerApplication::StartSessionsTimeoutThread()
{
	if (m_hSessionsTimeoutThread == OpcUa_Null)
	{
		OpcUa_Semaphore_Create(&m_SessionsTimeoutSem,0,0x100);
		m_bRunSessionsTimeoutThread=OpcUa_True;
		OpcUa_StatusCode uStatus  = OpcUa_Thread_Create(&m_hSessionsTimeoutThread,(CServerApplication::SessionsTimeoutThread),this);

		if(uStatus != OpcUa_Good)
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Create SessionsTimeoutThread Failed");
		else
			OpcUa_Thread_Start(m_hSessionsTimeoutThread);
	}
}
void CServerApplication::StopSessionsTimeoutThread()
{
	if (m_bRunSessionsTimeoutThread)
	{
		m_bRunSessionsTimeoutThread=FALSE;
		OpcUa_Semaphore_Post(m_SessionsTimeoutSem,1);
		OpcUa_StatusCode uStatus = OpcUa_Semaphore_TimedWait( m_SessionsTimeoutSem,OPC_TIMEOUT*2); // 15 secondes max.
		if (uStatus == OpcUa_GoodNonCriticalTimeout)
		{
			// on force la fin du thread de simulation
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Impossible to stop the CServerApplication::SessionTimeoutThread. Timeout\n");
		}
		else
		{
			OpcUa_Thread_Delete(m_hSessionsTimeoutThread);
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_INFO, "CServerApplication::SessionTimeoutThread stopped properly\n");
		}
	}
	//OpcUa_Semaphore_Delete(&m_SessionsTimeoutSem);
}
/// <summary>
/// This thread watch the session and release ressources for orphans.
/// </summary>
/// <param name="arg">The argument.</param>
void CServerApplication::SessionsTimeoutThread(LPVOID arg)
{
	CServerApplication* pServerApplication=(CServerApplication*)arg;
	OpcUa_StatusCode uStatus=OpcUa_Good;

	OpcUa_UInt32 uiInterval=(DWORD)pServerApplication->UpdateTimeoutInterval();
	DWORD uiSleepTime=100;
	while(pServerApplication->m_bRunSessionsTimeoutThread)
	{
		uiInterval=(DWORD)pServerApplication->UpdateTimeoutInterval();
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
		// calcul du nouvel interval relatif (dwSleepTime)
#if _WIN32_WINNT >= 0x0600
		ULONGLONG dwEnd = GetTickCount64();
		ULONGLONG dwCountedTime = dwEnd - dwStart;
		if (uiInterval>dwCountedTime)
			uiSleepTime = (OpcUa_UInt32)(uiInterval - dwCountedTime);
		else
			uiSleepTime = 0;
#else 
	#if _WIN32_WINNT == 0x0501
		DWORD dwEnd = GetTickCount();
		DWORD dwCountedTime = dwEnd - dwStart;
		if (uiInterval>dwCountedTime)
			uiSleepTime = uiInterval - dwCountedTime;
		else
			uiSleepTime = 0;
	#endif
#endif
#ifdef _GNUC_
		DWORD dwEnd=GetTickCount();
		DWORD dwCountedTime=dwEnd-dwStart;
		if (uiInterval>dwCountedTime)
			uiSleepTime=uiInterval-dwCountedTime;
		else
			uiSleepTime=0;
#endif
		if (uStatus == OpcUa_GoodNonCriticalTimeout)	
		{
			// Verification des session en timeout
			OpcUa_Mutex_Lock(pServerApplication->m_hSessionsMutex);
			CSessionServerList tmpSessionList;
			CSessionServerList::iterator it;
			for (it = pServerApplication->m_sessions.begin(); it != pServerApplication->m_sessions.end(); it++)
			{
				CSessionServer* pSession = *it;
				if (pSession->IsTimeouted())
				{
					delete pSession;
					pSession = OpcUa_Null;
				}
				else
					tmpSessionList.push_back(pSession);
			}
			// Retransfert
			pServerApplication->m_sessions.clear();
			pServerApplication->m_sessions.swap(tmpSessionList);
			tmpSessionList.clear();
			OpcUa_Mutex_Unlock(pServerApplication->m_hSessionsMutex);
		}		
		uStatus=OpcUa_Semaphore_TimedWait( pServerApplication->m_SessionsTimeoutSem, uiSleepTime );  // on attend jusqu'a qu'une demande async soit posé dans la queue
	}
	OpcUa_Semaphore_Post( pServerApplication->m_SessionsTimeoutSem,1);
	OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"CServerApplication::SessionTimeoutThread stopped\n");
}

OpcUa_StatusCode CServerApplication::RegisterNodes( OpcUa_UInt32 uSecureChannelId,
													const OpcUa_RequestHeader* a_pRequestHeader,
													OpcUa_Int32                a_nNoOfNodesToRegister,
													const OpcUa_NodeId*        a_pNodesToRegister,
													OpcUa_ResponseHeader*      a_pResponseHeader,
													OpcUa_Int32*               a_pNoOfRegisteredNodeIds,
													OpcUa_NodeId**             a_pRegisteredNodeIds)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_StatusCode uServiceResult=OpcUa_Good;
	CSessionServer* pSession = 0;
	OpcUa_ReferenceParameter(a_pRequestHeader);
	OpcUa_ReferenceParameter(a_pResponseHeader);

	OpcUa_Mutex_Lock(m_hMutex);

	// find the session.
	uStatus=FindSession(uSecureChannelId,&a_pRequestHeader->AuthenticationToken,&pSession);
	if (uStatus==OpcUa_Good)
	{

		// Signal that the session is alive
		OpcUa_Semaphore_Post(pSession->m_SessionTimeoutSem,1);
		// Register the node in the session.
		uServiceResult=pSession->RegisterNodes(a_nNoOfNodesToRegister,a_pNodesToRegister,a_pNoOfRegisteredNodeIds,a_pRegisteredNodeIds);
		//if (uServiceResult==OpcUa_BadInvalidArgument)
		//{
		//	uServiceResult=OpcUa_BadNodeIdInvalid;
		//	uStatus=OpcUa_Good;
		//}
		//else
		//{
		//	if (uServiceResult==OpcUa_BadNodeIdInvalid)
		//	{
		//		uStatus=OpcUa_BadNodeIdInvalid;
		//		uServiceResult=OpcUa_Good;
		//	}
		//}
	}
	else
		uServiceResult=OpcUa_BadSessionIdInvalid;

	// Update the response header.
	a_pResponseHeader->Timestamp     = OpcUa_DateTime_UtcNow();
	a_pResponseHeader->RequestHandle = a_pRequestHeader->RequestHandle;
	a_pResponseHeader->ServiceResult = uServiceResult;

	OpcUa_Mutex_Unlock(m_hMutex);
	return uStatus;
}

OpcUa_StatusCode CServerApplication::UnregisterNodes(	OpcUa_UInt32 uSecureChannelId,
														const OpcUa_RequestHeader* a_pRequestHeader,
														OpcUa_Int32                a_nNoOfNodesToUnregister,
														const OpcUa_NodeId*        a_pNodesToUnregister,
														OpcUa_ResponseHeader*      a_pResponseHeader)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_StatusCode uServiceResult=OpcUa_Good;
	CSessionServer* pSession = 0;
	OpcUa_ReferenceParameter(a_pRequestHeader);
	OpcUa_ReferenceParameter(a_pResponseHeader);

	OpcUa_Mutex_Lock(m_hMutex);

	// find the session.
	uStatus=FindSession(uSecureChannelId,&a_pRequestHeader->AuthenticationToken,&pSession);
	if (uStatus==OpcUa_Good)
	{
		// Signal that the session is alive
		OpcUa_Semaphore_Post(pSession->m_SessionTimeoutSem,1);
		// Register the node in the session.
		uServiceResult=pSession->UnregisterNodes(a_nNoOfNodesToUnregister,a_pNodesToUnregister);
	}
	else
		uServiceResult=OpcUa_BadSessionIdInvalid;

	// Update the response header.
	a_pResponseHeader->Timestamp     = OpcUa_DateTime_UtcNow();
	a_pResponseHeader->RequestHandle = a_pRequestHeader->RequestHandle;
	a_pResponseHeader->ServiceResult = uServiceResult;

	OpcUa_Mutex_Unlock(m_hMutex);
	return uStatus;
}
OpcUa_Boolean CServerApplication::IsClientNonceExist(const OpcUa_UInt32 uSecureChannelId,const OpcUa_ByteString* pClientNonce)
{
	OpcUa_Boolean bRes=OpcUa_False;
	if (pClientNonce->Data) // 
	{
		OpcUa_Mutex_Lock(m_hSessionsMutex);
		for (OpcUa_UInt32 ii=0;ii<m_sessions.size();ii++)
		{
			CSessionServer* pSessionServer =m_sessions.at(ii);
			if (pSessionServer)
			{
				CSecureChannel* pSecureChannel = pSessionServer->GetSecureChannel();
				if (pSecureChannel->GetSecureChannelId() == uSecureChannelId)
				{
					OpcUa_ByteString* pSessionServerClientNonce = pSessionServer->GetClientNonce();
					if (pSessionServerClientNonce)
					{
						if (pSessionServerClientNonce->Length == pClientNonce->Length)
						{
							if (OpcUa_MemCmp(pSessionServerClientNonce->Data, pClientNonce->Data, pClientNonce->Length) == 0)
								bRes = OpcUa_True;
						}
					}
				}
			}
		}
		OpcUa_Mutex_Unlock(m_hSessionsMutex);
	}
	return bRes;
}

OpcUa_StatusCode CServerApplication::AckCommentConfirmApplicationEvents(CEventDefinition* pEventDefinition, const OpcUa_LocalizedText comment, METHOD_NAME eBehavior)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_Mutex_Lock(m_hSessionsMutex);
	
	// First let 
	for (OpcUa_UInt32 i = 0; i < m_sessions.size(); i++)
	{
		CSessionServer* pSessionServer = m_sessions.at(i);
		if (pSessionServer)
		{
			uStatus = pSessionServer->AckCommentConfirmSessionEvents(pEventDefinition, comment, eBehavior);
		}
	}
	OpcUa_Mutex_Unlock(m_hSessionsMutex);
	return uStatus;
}

OpcUa_UInt32 CServerApplication::GetLDSRegistrationInterval() 
{ 
	return m_uiLDSRegistrationInterval; 
}
void CServerApplication::SetLDSRegistrationInterval(OpcUa_UInt32 dwVal) 
{ 
	m_uiLDSRegistrationInterval = dwVal; 
}

OpcUa_Boolean CServerApplication::IsLDSRegistrationActive() 
{ 
	return m_bLDSRegistrationActive; 
}
void CServerApplication::LDSRegistrationActive(OpcUa_Boolean bVal) 
{ 
	m_bLDSRegistrationActive = bVal; 
}
void CServerApplication::WakeupAllVpi()
{
	OpcUa_Mutex_Lock(m_VpiDevicesMutex);
	if (m_pVpiDevices)
	{
		for (OpcUa_UInt32 i = 0; i < m_pVpiDevices->size(); i++)
		{
			CVpiDevice* pVpiDevice = m_pVpiDevices->at(i);
			pVpiDevice->Wakeup();
		}
	}
	OpcUa_Mutex_Unlock(m_VpiDevicesMutex);
}
void CServerApplication::AddVpiDevice(CVpiDevice* pSubsystem)
{
	OpcUa_Mutex_Lock(m_VpiDevicesMutex);
	if (m_pVpiDevices)
		m_pVpiDevices->push_back(pSubsystem);
	OpcUa_Mutex_Unlock(m_VpiDevicesMutex);
}
OpcUa_StatusCode CServerApplication::RemoveVpiDevice(CVpiDevice* pSubsystem)
{
	OpcUa_StatusCode uStatus = OpcUa_BadNotFound;
	OpcUa_Mutex_Lock(m_VpiDevicesMutex);
	if (m_pVpiDevices)
	{
		CVpiDeviceList::iterator it = m_pVpiDevices->begin();
		for (it = m_pVpiDevices->begin(); it != m_pVpiDevices->end();++it)		
		{
			CVpiDevice* pVpiDevice =*it;
			if (pVpiDevice == pSubsystem)
			{
				delete pVpiDevice;
				m_pVpiDevices->erase(it);
				uStatus = OpcUa_Good;
				break;
			}
		}
	}
	OpcUa_Mutex_Unlock(m_VpiDevicesMutex);
	return uStatus;
}

void CServerApplication::RemoveAllVpiDevice()
{
	OpcUa_Mutex_Lock(m_VpiDevicesMutex);
	if (m_pVpiDevices)
	{
		CVpiDeviceList::iterator it = m_pVpiDevices->begin();
		while (it!=m_pVpiDevices->end())
		{
			delete *it;
			++it;
		}
		m_pVpiDevices->clear();
	}
	OpcUa_Mutex_Unlock(m_VpiDevicesMutex);
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Initializes the user identity token List. </summary>
///
/// <remarks>	Michel, 13/05/2016. </remarks>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode CServerApplication::InitializeUsersIdentityToken()
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_UserNameIdentityToken* pUserNameIdentityToken = (OpcUa_UserNameIdentityToken*)OpcUa_Alloc(sizeof(OpcUa_UserNameIdentityToken));
	if (pUserNameIdentityToken)
	{
		OpcUa_UserNameIdentityToken_Initialize(pUserNameIdentityToken);
		// PolicyId
		OpcUa_String_AttachCopy(&(pUserNameIdentityToken->PolicyId), "OpenOpcUaUserNamePolicyId");
		OpcUa_String_AttachCopy(&(pUserNameIdentityToken->EncryptionAlgorithm),"http://www.w3.org/2001/04/xmlenc#rsa-1_5");
		// UserName
		OpcUa_String_Initialize(&(pUserNameIdentityToken->UserName));
		OpcUa_String_AttachCopy(&(pUserNameIdentityToken->UserName), "OpenOpcUa");
		// Password
		pUserNameIdentityToken->Password.Data = (OpcUa_Byte*)OpcUa_Alloc(10);
		if (pUserNameIdentityToken->Password.Data)
		{
			ZeroMemory(pUserNameIdentityToken->Password.Data, 10);
			OpcUa_MemCpy(pUserNameIdentityToken->Password.Data, 9, (OpcUa_Byte*)"123456789", 9);
			pUserNameIdentityToken->Password.Length = 9;
			// Add to the list of valid passWord
			OpcUa_Mutex_Lock(m_UserNameIdentityTokenListMutex);
			m_UserNameIdentityTokenList.push_back(pUserNameIdentityToken);
			OpcUa_Mutex_Unlock(m_UserNameIdentityTokenListMutex);
		}
		else
		{
			OpcUa_Free(pUserNameIdentityToken);
			uStatus = OpcUa_BadOutOfMemory;
		}
	}
	else
		uStatus = OpcUa_BadOutOfMemory;
	return uStatus;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Remove entries in the users identity token list. </summary>
///
/// <remarks>	Michel, 13/05/2016. </remarks>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode CServerApplication::RemoveUsersIdentityToken()
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	CUserNameIdentityTokenList::iterator it;
	OpcUa_Mutex_Lock(m_UserNameIdentityTokenListMutex);
	for (it = m_UserNameIdentityTokenList.begin(); it != m_UserNameIdentityTokenList.end();it++)
	{
		OpcUa_UserNameIdentityToken* pUserNameIdentityToken = *it;
		OpcUa_UserNameIdentityToken_Clear(pUserNameIdentityToken);
		OpcUa_Free(pUserNameIdentityToken);
	}
	m_UserNameIdentityTokenList.clear();
	OpcUa_Mutex_Unlock(m_UserNameIdentityTokenListMutex);
	return uStatus;
}
void CServerApplication::AddSessionServer(CSessionServer* pSession)
{
	OpcUa_Mutex_Lock(m_hSessionsMutex);
	m_sessions.push_back(pSession);
	OpcUa_Mutex_Unlock(m_hSessionsMutex);
}
void CServerApplication::RemoveAllSessionServer()
{
	OpcUa_Mutex_Lock(m_hSessionsMutex);
	for (CSessionServerList::iterator it = m_sessions.begin(); it != m_sessions.end(); it++)
	{
		CSessionServer* pSession = *it;
		delete pSession;
	}
	m_sessions.clear();
	OpcUa_Mutex_Unlock(m_hSessionsMutex);
}
OpcUa_StatusCode CServerApplication::RemoveSessionServer(CSessionServer* pSession)
{
	OpcUa_StatusCode uStatus = OpcUa_BadNotFound;
	if (pSession)
	{
		OpcUa_Mutex_Lock(m_hSessionsMutex);
		for (CSessionServerList::iterator it = m_sessions.begin(); it != m_sessions.end(); it++)
		{
			if (*it == pSession)
			{
				delete pSession;
				m_sessions.erase(it);
				uStatus = OpcUa_Good;
				break;
			}
		}
		OpcUa_Mutex_Unlock(m_hSessionsMutex);
	}
	return uStatus;
}

OpcUa_StatusCode CServerApplication::WakeupAllSubscription()
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	CSessionServer* pSession = OpcUa_Null;

	OpcUa_Mutex_Lock(m_hSessionsMutex);
	for (CSessionServerList::iterator it = m_sessions.begin(); it != m_sessions.end(); it++)
	{
		pSession = *it;
		uStatus = pSession->WakeupAllSubscription();
	}
	OpcUa_Mutex_Unlock(m_hSessionsMutex);
	return uStatus;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Gets vpi device by identifier. </summary>
///
/// <remarks>	Michel, 03/02/2016. </remarks>
///
/// <param name="deviceNodeId">	Identifier for the device node. </param>
/// <param name="ppVpiDevice"> 	[in,out] If non-null, the vpi device. </param>
///
/// <returns>	The vpi device by identifier. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode OpenOpcUa::UACoreServer::CServerApplication::GetVpiDeviceById(OpcUa_NodeId deviceNodeId, CVpiDevice**ppVpiDevice)
{
	OpcUa_StatusCode uStatus = OpcUa_BadNotFound;
	OpcUa_Mutex_Lock(m_VpiDevicesMutex);
	if (m_pVpiDevices)
	{
		CVpiDeviceList::iterator it = m_pVpiDevices->begin();
		for (OpcUa_UInt16 ii = 0; ii < m_pVpiDevices->size(); ii++)
		{
			CVpiDevice* pVpiDevice = m_pVpiDevices->at(ii);
			OpcUa_NodeId nodeId=pVpiDevice->GetSubSystemId();
			if (OpcUa_NodeId_Compare(&nodeId, &deviceNodeId) == 0)
			{
				*(ppVpiDevice) = pVpiDevice;
				uStatus = OpcUa_Good;
				break;
			}
		}
	}
	OpcUa_Mutex_Unlock(m_VpiDevicesMutex);
	return uStatus;	
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Security None accepted or not.  </summary>
///
/// <remarks>	Michel, 12/05/2016. </remarks>
///
/// <param name="bVal">	The value. </param>
///-------------------------------------------------------------------------------------------------

void OpenOpcUa::UACoreServer::CServerApplication::SecurityNoneAccepted(OpcUa_Boolean bVal)
{
	m_bSecurityNoneAccepted = bVal;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Is security none accepted. </summary>
///
/// <remarks>	Michel, 12/05/2016. </remarks>
///
/// <returns>	An OpcUa_Boolean. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_Boolean OpenOpcUa::UACoreServer::CServerApplication::IsSecurityNoneAccepted(void)
{
	return m_bSecurityNoneAccepted;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	X509IdentityToken.  </summary>
///
/// <remarks>	Michel, 13/05/2016. </remarks>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode OpenOpcUa::UACoreServer::CServerApplication::InitializeX509IdentityTokenList(void)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_X509IdentityToken* pX509IdentityToken = (OpcUa_X509IdentityToken*)OpcUa_Alloc(sizeof(OpcUa_X509IdentityToken));
	if (pX509IdentityToken)
	{
		OpcUa_X509IdentityToken_Initialize(pX509IdentityToken);
		// PolicyId
		OpcUa_String_AttachCopy(&(pX509IdentityToken->PolicyId), "OpenOpcUaCertificatePolicyId");
		// Certificate
		pX509IdentityToken->CertificateData;
		m_X509IdentityTokenList.push_back(pX509IdentityToken);
	}
	else
		uStatus = OpcUa_BadOutOfMemory;
	return uStatus;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Removes the x coordinate 509 identity token list. </summary>
///
/// <remarks>	Michel, 13/05/2016. </remarks>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode OpenOpcUa::UACoreServer::CServerApplication::RemoveX509IdentityTokenList(void)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	CX509IdentityTokenList::iterator it;
	OpcUa_Mutex_Lock(m_X509IdentityTokenListMutex);
	for (it = m_X509IdentityTokenList.begin(); it != m_X509IdentityTokenList.end(); it++)
	{
		OpcUa_X509IdentityToken* pX509IdentityToken = *it;
		OpcUa_X509IdentityToken_Clear(pX509IdentityToken);
		OpcUa_Free(pX509IdentityToken);
	}
	m_X509IdentityTokenList.clear();
	OpcUa_Mutex_Unlock(m_X509IdentityTokenListMutex);
	return uStatus;	
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Parse command line. Valid command line are :
/// 			. </summary>
///
/// <remarks>	Michel, 10/07/2016. </remarks>
///
/// <param name="szRawCommand">	   	[in,out] If non-null, the raw command. </param>
/// <param name="pszParsedCommand">	[in,out] If non-null, the parsed command. </param>
///-------------------------------------------------------------------------------------------------

void OpenOpcUa::UACoreServer::CServerApplication::ParseCommandLine(OpcUa_CharA* szRawCommand, STARTUP_COMMAND* pCommand)
{
	OpcUa_CharA* szCommand1 = OpcUa_Null;
	OpcUa_CharA* szCommandAppId = OpcUa_Null;
	OpcUa_CharA* szCommandServiceName = OpcUa_Null;

	if (szRawCommand)
	{
		OpcUa_Boolean bSoFarSoGood = OpcUa_False;
		for (OpcUa_UInt16 i = 0; i < strlen(szRawCommand); i++)
		{
			if (szRawCommand[i] == 0x3D) // =
			{
				szCommand1 = (OpcUa_CharA*)malloc(i + 1);
				ZeroMemory(szCommand1, i + 1);
				memcpy(szCommand1, &szRawCommand[0], i);
				if (strcmp(szCommand1, "AppId") == 0)
				{
					szCommandAppId = (OpcUa_CharA*)malloc(strlen(szRawCommand) - i);
					ZeroMemory(szCommandAppId, strlen(szRawCommand) - i);
					memcpy(szCommandAppId, &szRawCommand[i + 1], strlen(szRawCommand) - i);
					break;
				}
				else
				{
					// extract AppId
					szCommandAppId = (OpcUa_CharA*)malloc(strlen(szRawCommand) - i);
					ZeroMemory(szCommandAppId, strlen(szRawCommand) - i);
					memcpy(szCommandAppId, &szRawCommand[i + 1], strlen(szRawCommand) - i);
					bSoFarSoGood = OpcUa_True;
				}
			}
			if ((szRawCommand[i] == 0x3B) && bSoFarSoGood) // ;
			{
				szCommandServiceName = (OpcUa_CharA*)malloc(strlen(szRawCommand) - i);
				ZeroMemory(szCommandServiceName, strlen(szRawCommand) - i);
				memcpy(szCommandServiceName, &szRawCommand[i + 1], strlen(szRawCommand) - i);
			}
		}
		// If the format is not Install={xxx-xxx-xxx-xxx-xxx},ServiceName or Uninstall={xxx-xxx-xxx-xxx-xxx},ServiceName
		// let's try the commandline with just Install or Uninstall and the prepare the default OpenOpcUa GUID and ServiceName
		if (!szCommand1)
		{
			szCommand1 = (OpcUa_CharA*)malloc(strlen(szRawCommand) + 1);
			ZeroMemory(szCommand1, strlen(szRawCommand) + 1);
			memcpy(szCommand1, szRawCommand, strlen(szRawCommand));
			// Set the default AppId
			szCommandAppId = (OpcUa_CharA*)malloc(strlen("{CB85A975-BAD5-4A1E-A142-FF98330EB221}") + 1);
			ZeroMemory(szCommandAppId, strlen("{CB85A975-BAD5-4A1E-A142-FF98330EB221}") + 1);
			memcpy(szCommandAppId, "{CB85A975-BAD5-4A1E-A142-FF98330EB221}", strlen("{CB85A975-BAD5-4A1E-A142-FF98330EB221}"));
			// Set the default ServiceName
			szCommandServiceName = (OpcUa_CharA*)malloc(strlen("OpenOpcUaCoreServer") + 1);
			ZeroMemory(szCommandServiceName, strlen("OpenOpcUaCoreServer") + 1);
			memcpy(szCommandServiceName, "OpenOpcUaCoreServer", strlen("OpenOpcUaCoreServer"));
		}
		// AppId
		if (szCommandAppId)
		{
			OpcUa_Guid aApplicationGuid;
			OpcUa_Guid_Initialize(&aApplicationGuid);
			OpcUa_Guid_FromString(szCommandAppId, &aApplicationGuid);
#ifdef WIN32
			m_ServiceModule.SetApplicationId(&aApplicationGuid);
			// ServiceName
			m_ServiceModule.SetServiceName(szCommandServiceName);
#endif
		}
	}
	if (strcmp("Uninstall", szCommand1) == 0)
		(*pCommand) = UNINSTALL_SERVICE;
	if (strcmp("AppId", szCommand1) == 0)
	{
		OpcUa_Guid appId;
		OpcUa_Guid_FromString(szCommandAppId, &appId);
#ifdef WIN32
		m_ServiceModule.SetApplicationId(&appId);
#endif
		(*pCommand) = RUN_SERVICE;
	}
	if (strcmp("Install", szCommand1) == 0)
		(*pCommand) = INSTALL_SERVICE;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Parse first parameter. </summary>
///
/// <remarks>	Michel, 10/07/2016. </remarks>
///
/// <param name="pszParam">	[in,out] If non-null, the parameter. </param>
///-------------------------------------------------------------------------------------------------

void OpenOpcUa::UACoreServer::CServerApplication::ParseFirstParameter(char* pszParam, STARTUP_COMMAND* pStartupCommand)
{
#ifdef _GNUC_
	int fileSep = '/';
#else
#ifdef WIN32
	int fileSep = '\\';
	int equalSep = '=';
#endif
#endif
	char* cPos = OpcUa_Null;
	size_t iSize = 0;
	if (pszParam)
	{
		iSize = strlen(pszParam);
		// let's try to find a / or a \ in the pszParam.
		// If found the fill the config file and path name
		cPos = strrchr(pszParam, fileSep);
		if (cPos)
		{
			int iFileNameLen = strlen(cPos);
			OpcUa_CharA* fileName = (char*)OpcUa_Alloc(iFileNameLen + 1);
			if (fileName)
			{
				OpcUa_MemSet(fileName, 0, iFileNameLen + 1);
				OpcUa_StrnCpyA(fileName, iFileNameLen, ++cPos, iFileNameLen);
				SetConfigFileName(fileName);
				int iPathLen = iSize - strlen(fileName); // Path len
				OpcUa_CharA* path = (OpcUa_CharA*)OpcUa_Alloc(iPathLen + 1);
				OpcUa_MemSet(path, 0, iPathLen + 1);
				memcpy(path, pszParam, iPathLen);
				SetConfigPath(path);
				(*pStartupCommand) = RUN_SERVER;
#ifdef WIN32
				m_ServiceModule.Serviced(OpcUa_False);
#endif
				free(fileName);
				free(path);
			}
		}
#ifdef WIN32 // this case is only valid on windows for the server startup
		else
		{
			m_ServiceModule.Serviced(OpcUa_True);
			ParseCommandLine(pszParam, pStartupCommand);
		}
#endif
	}
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Parse second parameter. </summary>
///
/// <remarks>	Michel, 10/07/2016. </remarks>
///
/// <param name="pszParam">	[in,out] If non-null, the parameter. </param>
///-------------------------------------------------------------------------------------------------

void OpenOpcUa::UACoreServer::CServerApplication::ParseSecondParameter(char* pszParam, STARTUP_COMMAND* pStartupCommand)
{
	if (pszParam)
	{
		ParseCommandLine(pszParam, pStartupCommand);
	}	
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Accessor for config file and path.  </summary>
///
/// <remarks>	Michel, 10/07/2016. </remarks>
///
/// <returns>	null if it fails, else the configuration path. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_CharA* OpenOpcUa::UACoreServer::CServerApplication::GetConfigPath(void)
{
	return m_ConfigPath;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Sets configuration path. </summary>
///
/// <remarks>	Michel, 10/07/2016. </remarks>
///
/// <param name="pszVal">	[in,out] If non-null, the value. </param>
///-------------------------------------------------------------------------------------------------

void OpenOpcUa::UACoreServer::CServerApplication::SetConfigPath(OpcUa_CharA* pszVal)
{
	if (pszVal)
	{
		if (m_ConfigPath)
			OpcUa_Free(m_ConfigPath);
		m_ConfigPath = (OpcUa_CharA*)malloc(strlen(pszVal) + 1);
		ZeroMemory(m_ConfigPath, strlen(pszVal) + 1);
		memcpy(m_ConfigPath, pszVal, strlen(pszVal) + 1);
	}
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Gets configuration file name. </summary>
///
/// <remarks>	Michel, 10/07/2016. </remarks>
///
/// <returns>	null if it fails, else the configuration file name. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_CharA* OpenOpcUa::UACoreServer::CServerApplication::GetConfigFileName(void)
{	
	return m_ConfigFileName;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Sets configuration file name. </summary>
///
/// <remarks>	Michel, 10/07/2016. </remarks>
///
/// <param name="pszVal">	[in,out] If non-null, the value. </param>
///-------------------------------------------------------------------------------------------------

void OpenOpcUa::UACoreServer::CServerApplication::SetConfigFileName(OpcUa_CharA*pszVal)
{
	if (pszVal)
	{
		if (m_ConfigFileName)
			OpcUa_Free(m_ConfigFileName);
		m_ConfigFileName = (OpcUa_CharA*)malloc(strlen(pszVal) + 1);
		ZeroMemory(m_ConfigFileName, strlen(pszVal) + 1);
		memcpy(m_ConfigFileName, pszVal, strlen(pszVal) + 1);
	}
}

CEventsEngine*	CServerApplication::GetEventsEngine() 
{ 
	return m_pEventsEngine; 
}
void CServerApplication::SetEventsEngine(CEventsEngine* pEventsEngine) 
{ 
	m_pEventsEngine = pEventsEngine; 
}