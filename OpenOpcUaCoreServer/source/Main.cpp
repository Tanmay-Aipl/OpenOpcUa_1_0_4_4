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
// below header file for the new keyboard control mode (q or Q without hiting 'Enter')
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
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
#include "VpiWriteObject.h"
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

//#include "MonitoredItemServer.h"
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
#include "ServiceModule.h" // Implement the server as a Windows Service. So this class/header is a Windows Only class/header
#ifdef WIN32
#include <conio.h>
//#include "resource.h"
#endif
using namespace OpenOpcUa;
using namespace UASimulation;
using namespace UASubSystem;
using namespace UAAddressSpace;
using namespace UACoreServer;


CUAInformationModel*	CServerApplication::m_pTheAddressSpace=NULL;
OpcUa_Semaphore g_ShutdownServerSem;
OpcUa_Boolean g_bRun;
//CServerApplication m_theApplication;
CServerApplication* g_pTheApplication = OpcUa_Null;
OpcUa_Thread hInvertingReferenceThread;
#ifdef WIN32
//CServiceModule _Module;
#endif
// 
/// <summary>
/// Shutdowns the server.
/// Sequence of de-initialization for a proper stop of the server
/// </summary>
void ShutdownServer(OpcUa_Int32 iSecondsTillShutdown,OpcUa_CharA* ShutdownReason)
{
	CUAInformationModel* pInformationModel=CServerApplication::m_pTheAddressSpace;
	if (pInformationModel)
	{
		// First we stop the thread in charge of updated the mandatory nodes
		pInformationModel->StopUpdateMandatoryNodesThread();
		// Now let's update the server status
		CServerStatus* pServerStatus = pInformationModel->GetInternalServerStatus();
		if (pServerStatus)
		{
			pServerStatus->SetServerState(OpcUa_ServerState_Shutdown);
			OpcUa_NodeId aTmpNodeId00;
			OpcUa_NodeId_Initialize(&aTmpNodeId00);
			CUAVariable* pUATmpVariable00 = OpcUa_Null;
			aTmpNodeId00.IdentifierType = OpcUa_IdentifierType_Numeric;
			OpcUa_Variant varVal00;
			OpcUa_Variant_Initialize(&varVal00);

			varVal00.Datatype = OpcUaType_Int32;
			aTmpNodeId00.Identifier.Numeric = 2259; // State
			OpcUa_StatusCode uStatus = pInformationModel->GetNodeIdFromVariableList(aTmpNodeId00, &pUATmpVariable00);
			if (uStatus == OpcUa_Good)
			{
				varVal00.Datatype = OpcUaType_UInt32;
				varVal00.Value.Int32 = pServerStatus->GetServerState();
				CDataValue* pDataValue = pUATmpVariable00->GetValue();
				if (pDataValue)
					pDataValue->SetValue(varVal00);
			}
			// SecondsTillShutdown
			pServerStatus->SetSecondsTillShutdown(iSecondsTillShutdown);
			aTmpNodeId00.Identifier.Numeric = 2992; // SecondsTillShutdown
			uStatus = pInformationModel->GetNodeIdFromVariableList(aTmpNodeId00, &pUATmpVariable00);
			if (uStatus == OpcUa_Good)
			{
				varVal00.Datatype = OpcUaType_UInt32;
				varVal00.Value.Int32 = pServerStatus->GetSecondsTillShutdown();
				CDataValue* pDataValue = pUATmpVariable00->GetValue();
				if (pDataValue)
					pDataValue->SetValue(varVal00);
			}
			// ShutdownReason
			OpcUa_LocalizedText aShutdownReason;
			OpcUa_LocalizedText_Initialize(&aShutdownReason);
			OpcUa_String_AttachCopy(&(aShutdownReason.Locale), "en-US");
			OpcUa_String_AttachCopy(&(aShutdownReason.Text), ShutdownReason);

			pServerStatus->SetShutdownReason(aShutdownReason);
			OpcUa_LocalizedText_Clear(&aShutdownReason);
			OpcUa_NodeId_Clear(&aTmpNodeId00);
			///////////////////////////////////////////
			//
			// ServerStatus
			//
			//////////////////////////////////////////
			CUAVariable* pUAVariable = NULL;
			OpcUa_NodeId aNodeId;
			OpcUa_NodeId_Initialize(&aNodeId);
			aNodeId.Identifier.Numeric = 2256; // ServerStatus UpdateMandatoryNodesThread
			aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
			aNodeId.NamespaceIndex = 0; // opc foundation namespace

			OpcUa_Mutex_Lock(pInformationModel->GetServerCacheMutex());
			uStatus = pInformationModel->GetNodeIdFromVariableList(aNodeId, &pUAVariable);
			if (uStatus == OpcUa_Good)
			{
				///////////////////////////////////////////////////////////////////////
				CDataValue* pDataValue = pUAVariable->GetValue();
				OpcUa_Variant aVariant = pDataValue->GetValue();
				if (aVariant.Value.ExtensionObject)
				{
					OpcUa_ServerStatusDataType* pInternalServerStatus = (OpcUa_ServerStatusDataType*)aVariant.Value.ExtensionObject->Body.EncodeableObject.Object;
					pInternalServerStatus->CurrentTime = pServerStatus->GetInternalCurrentTime();

					if (pDataValue)
					{
						if (pDataValue->GetValue().Value.ExtensionObject)
						{
							{
								// 
								// mise a jour des nodes encapsulées 2258,2259,2992, 2993
								OpcUa_NodeId aTmpNodeId;
								OpcUa_NodeId_Initialize(&aTmpNodeId);
								CUAVariable* pUATmpVariable = OpcUa_Null;
								aTmpNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
								OpcUa_Variant varVal;
								OpcUa_Variant_Initialize(&varVal);
								aTmpNodeId.Identifier.Numeric = 2258; // CurrentTime
								uStatus = pInformationModel->GetNodeIdFromVariableList(aTmpNodeId, &pUATmpVariable);
								if (uStatus == OpcUa_Good)
								{
									varVal.Datatype = OpcUaType_DateTime;
									varVal.Value.DateTime = pServerStatus->GetInternalCurrentTime();
									CDataValue* pDataValue00 = pUATmpVariable->GetValue();
									if (pDataValue00)
									{
										pDataValue00->SetValue(varVal);
										pDataValue00->SetStatusCode(OpcUa_Good);
										pDataValue00->SetServerTimestamp(OpcUa_DateTime_UtcNow());
									}
								}

								OpcUa_Variant_Initialize(&varVal);
								aTmpNodeId.Identifier.Numeric = 2259; // State
								uStatus = pInformationModel->GetNodeIdFromVariableList(aTmpNodeId, &pUATmpVariable);
								if (uStatus == OpcUa_Good)
								{
									varVal.Datatype = OpcUaType_Int32;
									varVal.Value.Int32 = pServerStatus->GetServerState();
									CDataValue* pDataValue00 = pUATmpVariable->GetValue();
									if (pDataValue00)
									{
										pDataValue00->SetValue(varVal);
										pDataValue00->SetStatusCode(OpcUa_Good);
										pDataValue00->SetServerTimestamp(OpcUa_DateTime_UtcNow());
									}
								}
								OpcUa_Variant_Clear(&varVal);
								// SecondsTillShutdown
								OpcUa_Variant_Initialize(&varVal);
								aTmpNodeId.Identifier.Numeric = 2992; // SecondsTillShutdown
								uStatus = pInformationModel->GetNodeIdFromVariableList(aTmpNodeId, &pUATmpVariable);
								if (uStatus == OpcUa_Good)
								{
									varVal.Datatype = OpcUaType_Int32;
									varVal.Value.Int32 = pServerStatus->GetSecondsTillShutdown();
									CDataValue* pDataValue00 = pUATmpVariable->GetValue();
									if (pDataValue00)
									{
										pDataValue00->SetValue(varVal);
										pDataValue00->SetStatusCode(OpcUa_Good);
										pDataValue00->SetServerTimestamp(OpcUa_DateTime_UtcNow());
									}
								}
								OpcUa_Variant_Clear(&varVal);
								// ShutdownReason
								OpcUa_Variant_Initialize(&varVal);
								aTmpNodeId.Identifier.Numeric = 2993; // ShutdownReason
								uStatus = pInformationModel->GetNodeIdFromVariableList(aTmpNodeId, &pUATmpVariable);
								if (uStatus == OpcUa_Good)
								{
									varVal.Value.LocalizedText = (OpcUa_LocalizedText*)OpcUa_Alloc(sizeof(OpcUa_LocalizedText));
									if (varVal.Value.LocalizedText)
									{
										OpcUa_LocalizedText_Initialize(varVal.Value.LocalizedText);
										varVal.Datatype = OpcUaType_LocalizedText;
										OpcUa_LocalizedText ltVal = pServerStatus->GetShutdownReason();
										OpcUa_LocalizedText_CopyTo(&ltVal, varVal.Value.LocalizedText);
										CDataValue* pDataValue00 = pUATmpVariable->GetValue();
										if (pDataValue00)
										{
											pDataValue00->SetValue(varVal);
											pDataValue00->SetStatusCode(OpcUa_Good);
											pDataValue00->SetServerTimestamp(OpcUa_DateTime_UtcNow());
										}
									}
								}
								OpcUa_Variant_Clear(&varVal);
								OpcUa_NodeId_Clear(&aTmpNodeId);
							}
						}
					}
				}
			}
			OpcUa_Mutex_Unlock(pInformationModel->GetServerCacheMutex());
		}

	}
	// fin informations de Shutdown
}
// Play a small animation while stoping the server
OpcUa_StatusCode StopAnimation(OpcUa_Int32 iSecond)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	CUAInformationModel* pInformationModel = CServerApplication::m_pTheAddressSpace;
	if (pInformationModel)
	{
		CServerStatus* pServerStatus = pInformationModel->GetInternalServerStatus();
		OpcUa_Variant varVal;
		OpcUa_NodeId aTmpNodeId;
		OpcUa_NodeId_Initialize(&aTmpNodeId);
		aTmpNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		aTmpNodeId.Identifier.Numeric = 2992; // SecondsTillShutdown
		CUAVariable* pUATmpVariable = OpcUa_Null;
		uStatus = pInformationModel->GetNodeIdFromVariableList(aTmpNodeId, &pUATmpVariable);
		if (uStatus == OpcUa_Good)
		{
			for (OpcUa_Int32 ii = 0; ii < iSecond*10; ii++)
			{
				if ((ii % 10) == 0)
				{
					printf(".");
					// SecondsTillShutdown
					OpcUa_Variant_Initialize(&varVal);
					CDataValue* pDataValue = pUATmpVariable->GetValue();
					varVal.Datatype = OpcUaType_Int32;
					varVal.Value.Int32 = pServerStatus->GetSecondsTillShutdown() - 1;
					pServerStatus->SetSecondsTillShutdown(varVal.Value.Int32);
					pDataValue->SetValue(varVal);
				}
				OpcUa_Thread_Sleep(100);
			}
		}
		printf("\n");
	}
	return uStatus;
}

OpcUa_Boolean IsService(char* szCommand)
{
	OpcUa_Boolean bResult = OpcUa_False;
#ifdef _GNUC_
	bResult = OpcUa_False;
#endif
#ifdef WIN32
	if (strcmp(szCommand, "Install") == 0)
		bResult = OpcUa_True;

	if (strcmp(szCommand, "Uninstall") == 0)
		bResult = OpcUa_True;


	if (strcmp(szCommand, "Service") == 0)
		bResult = OpcUa_True;

#endif
	return bResult;
}
OpcUa_StatusCode RunAsApplication(STARTUP_COMMAND eCommand)
{
	////////////////////////////////////////////////
	//
	// Run as a Shell or Linux server
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_CharA* path = g_pTheApplication->GetConfigPath();
	OpcUa_CharA* fileName = g_pTheApplication->GetConfigFileName();
	if ((path) && (fileName) && (eCommand==RUN_SERVER) )
	{
		CServerApplication::m_pTheAddressSpace = new CUAInformationModel();
		///////////////////////////////////////////////////
		//
		// Sempahore to remotly stop the server
		OpcUa_Semaphore_Create(&g_ShutdownServerSem, 0, 0x100);

		// Parsing du fichier de configuration du serveur
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Loading %s %s\n", path, fileName);
		uStatus = g_pTheApplication->LoadUaServerConfiguration(path, fileName);
		if (uStatus == OpcUa_Good)
		{
			// Fin chargement du fichier de configuration
			// Mise en place de L'application Description et des informations sur les ports d'écoute
			CApplicationDescription* pApplicationDescription = g_pTheApplication->GetApplicationDescription();
			uStatus = SetServerDescriptionTransportPortListener(&pApplicationDescription);
			if (uStatus == OpcUa_Good)
			{
				g_pTheApplication->SetApplicationDescription(pApplicationDescription);
				//////////////////////////////////////////
				//
				// Definition du niveau de trace
				//
				//////////////////////////////////////////
				// OPCUA_TRACE_OUTPUT_LEVEL_DEBUG
				// OPCUA_TRACE_OUTPUT_LEVEL_INFO
				// OPCUA_TRACE_OUTPUT_LEVEL_WARNING
				// OPCUA_TRACE_OUTPUT_LEVEL_NONE
				// OPCUA_TRACE_OUTPUT_LEVEL_SYSTEM
				// OPCUA_TRACE_OUTPUT_LEVEL_ERROR
				OpcUa_UInt32 uiTraceLevel = g_pTheApplication->GetTraceLevel();
				OpcUa_Trace_ChangeTraceLevel(uiTraceLevel);
				OpcUa_UInt32 iOutput = g_pTheApplication->GetTraceOutput();
				if (iOutput == OPCUA_TRACE_OUTPUT_FILE)
				{
					// Mise en place du nom du fichier de sortie
					OpcUa_String strFileName;
					OpcUa_String_Initialize(&strFileName);
					OpcUa_String strBakFileName;
					OpcUa_String_Initialize(&strBakFileName);
					// extension du fichier
					OpcUa_String strExtension;
					OpcUa_String_Initialize(&strExtension);
					OpcUa_String_AttachCopy(&strExtension, ".log");
					// extension de la sauvegarde
					OpcUa_String strBakExtension;
					OpcUa_String_Initialize(&strBakExtension);
					OpcUa_String_AttachCopy(&strBakExtension, ".bak");
					// create the output filename base on the name of the server EndPoint
					pApplicationDescription = g_pTheApplication->GetApplicationDescription();
					if (pApplicationDescription)
					{
						// On ne cherche que le nom de l'application. Donc Le premier suffira ...
						OpcUa_LocalizedText aLocalizedText = pApplicationDescription->GetApplicationName();
						OpcUa_String_CopyTo(&(aLocalizedText.Text), &strFileName);
						OpcUa_String_CopyTo(&(aLocalizedText.Text), &strBakFileName);

						// creation du nom du fichier de log
						OpcUa_String_StrnCat(&strFileName, &strExtension, OpcUa_String_StrLen(&strExtension));
						// creation du nom du fichier de sauvegarde
						OpcUa_String_StrnCat(&strBakFileName, &strBakExtension, OpcUa_String_StrLen(&strBakExtension));
						OpcUa_Trace_SetTraceFile(strFileName);
						// On sauvegarde une eventuelle trace existante
						FILE* hFile = OpcUa_Null;
						hFile = fopen(OpcUa_String_GetRawString(&strFileName), "r");
						if (hFile)
						{
							// Ouverture du fichier de sauvegarde
							FILE* hFileBak = OpcUa_Null;
							OpcUa_CharA* szBakFileName = OpcUa_String_GetRawString(&strBakFileName);
							hFileBak = fopen(szBakFileName, "w");
							char bakBuffer;
							// On lit l'ensemble du fichier dans le buffer
							// On copie dans la sauvegarde
							while (fread(&bakBuffer, sizeof(char), 1, hFile) == 1)
								fwrite(&bakBuffer, sizeof(char), 1, hFileBak);
							// On ferme l'ancien fichier
							fclose(hFile);
							fflush(hFileBak);
							if (fclose(hFileBak)==EOF)
								OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Cannot close the bak log file\n");
							// On supprime l'ancien fichier 
							hFile = fopen(OpcUa_String_GetRawString(&strFileName), "w");
							if (hFile)
								fclose(hFile);
						}
					}
					OpcUa_String_Clear(&strExtension);
					OpcUa_String_Clear(&strFileName);
					OpcUa_String_Clear(&strBakExtension);
					OpcUa_String_Clear(&strBakFileName);
				}

				// Load nodeset files to create the server AddressSpace
				uStatus = LoadNodeSetFiles();
				if (uStatus == OpcUa_Good)
				{
					uStatus = LoadSimulationFiles();
					if (uStatus == OpcUa_Good)
					{
						// 
						// Load subsystem configuration file
						uStatus = LoadSubSystemFiles();
						if (uStatus == OpcUa_Good)
						{
							CEventsEngine* pEventsEngine = g_pTheApplication->GetEventsEngine();
							pEventsEngine = new CEventsEngine();
							if (pEventsEngine)
								g_pTheApplication->SetEventsEngine(pEventsEngine);
							/////////////////////////////////////////////////////////
							// PostProcessing
							//
							uStatus = PostProcessing();
							if (uStatus != OpcUa_Good)
								OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Post Processing failed 0x%05x\n", uStatus);
							else
							{
								CUAInformationModel* pInformationModel = CServerApplication::m_pTheAddressSpace;
								// Déblocage de la thread de mise à jour des MandatoryNodes
								OpcUa_Semaphore_Post(pInformationModel->m_SemMandatoryEvent, 1);
								// démarrage du mecanisme de dialogue avec les sous-systèmes
								//g_pTheApplication->WakeupAllVpi();

								// Si besoin demarrage du moteur d'archivage
								CHaEngine* pHaEngine = g_pTheApplication->GetHaEngine();
								if (pHaEngine)
								{
									if (pHaEngine->LoadVfi() == OpcUa_Good)
									{
										OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Your Vfi was correctly loaded\n");
										pHaEngine->InitHaEngine();	// Call GlobalStart
										OpcUa_Vfi_Handle hVfiHandle = OpcUa_Null;
										OpcUa_String szArchiveId = g_pTheApplication->GetArchiveId();
										OpcUa_NodeId aNodeId;
										OpcUa_NodeId_Initialize(&aNodeId);
										if (ParseNodeId(OpcUa_String_GetRawString(&szArchiveId), &aNodeId) == OpcUa_Good)
										{
											// Warning the Vfi is suppose to copy the aNodeId
											uStatus = pHaEngine->VfiGlobalStart(g_pTheApplication->GetArchiveName(),
												aNodeId,
												&hVfiHandle);
											if (uStatus == OpcUa_Good)
											{
												pHaEngine->SetVfiHandle(hVfiHandle);
												uStatus = pHaEngine->VfiColdStart(hVfiHandle);
												if (uStatus != OpcUa_Good)
													OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "VfiColdStart failed 0x%05x\n", uStatus);
											}
											else
												OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "VfiGlobalStart failed 0x%05x\n", uStatus);
										}
										else
											OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "ParseNodeId failed when preparing the call to VfiGlobalStart\n");
										OpcUa_NodeId_Clear(&aNodeId);
									}
								}
								// initialize security by specifying the client certificate and the location of the trusted certificates.

								OpcUa_String aCertificateStore;
								OpcUa_String_Initialize(&aCertificateStore);
								OpcUa_String_AttachCopy(&aCertificateStore, "CertificateStore");
								g_pTheApplication->SetCertificateStorePath(aCertificateStore);
								uStatus = g_pTheApplication->LoadPFXCertificate();
								if (uStatus == OpcUa_Good)
								{
									// on va tenter de lire le fichier DER afin de vérifier qu'il s'agit du bon DER
									// a savoir : 
									//	1- il existe
									//	2- sa clé correpsond acelle du pfx
									uStatus = g_pTheApplication->LoadDERCertificate();
									if (uStatus != OpcUa_Good)
									{
										OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Your certificate in DER file not fit the private key. Will create a new one\n");
										uStatus = g_pTheApplication->CreateCertificate();
									}
								}
								else
									uStatus = g_pTheApplication->CreateCertificate();

								// on va vérifier que le certificat du serveur est encore valide
								OpcUa_CharA* sThumbprint = OpcUa_Null;
								OpcUa_CharA* lApplicationUri = OpcUa_Null;
								OpcUa_CharA* psCommonName = OpcUa_Null;
								OpcUa_ByteString* pServerCertificate = g_pTheApplication->GetCertificate();
								if (pServerCertificate)
								{
									uStatus = OpcUa_Certificate_GetInfo(
										(OpcUa_ByteString*)pServerCertificate,
										OpcUa_Null,
										OpcUa_Null,
										&psCommonName,
										&sThumbprint,
										&lApplicationUri,
										OpcUa_Null,
										OpcUa_Null);
									if (lApplicationUri)
									{
										OpcUa_Free(lApplicationUri);
										lApplicationUri = OpcUa_Null;
									}
									if (sThumbprint)
									{
										OpcUa_Free(sThumbprint);
										sThumbprint = OpcUa_Null;
									}
									if (psCommonName)
									{
										OpcUa_Free(psCommonName);
										psCommonName = OpcUa_Null;
									}
									OpcUa_DateTime ValidFrom, ValidTo;
									OpcUa_DateTime_Initialize(&ValidFrom);
									OpcUa_DateTime_Initialize(&ValidTo);
									uStatus = OpcUa_Certificate_GetDateBound(
										(OpcUa_ByteString*)pServerCertificate,
										&ValidFrom, &ValidTo);
									if (uStatus == OpcUa_Good)
									{
										// Verification de la validitée du certificat;
										OpcUa_String* strFrom = OpcUa_Null; // (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
										OpcUa_String* strTo = OpcUa_Null;  //OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
										Utils::OpcUaDateTimeToString(ValidFrom, &strFrom);
										Utils::OpcUaDateTimeToString(ValidTo, &strTo);
										OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Your certificate validate your server \nfrom: %s \nto: %s\n",
											OpcUa_String_GetRawString(strFrom),
											OpcUa_String_GetRawString(strTo));
										if (strTo)
										{
											OpcUa_String_Clear(strTo);
											OpcUa_Free(strTo);
										}
										if (strFrom)
										{
											OpcUa_String_Clear(strFrom);
											OpcUa_Free(strFrom);
										}
										// Est ce que la date de fin est dépassée
										OpcUa_DateTime utcNow = OpcUa_DateTime_UtcNow();
										OpcUa_UInt32 uDiff = 0;
										uStatus = OpcUa_P_GetDateTimeDiffInSeconds32(utcNow, ValidTo, &uDiff);
										if ((uStatus != OpcUa_BadOutOfRange) && (uStatus != OpcUa_BadInvalidArgument))
										{
											// Il n'est peut etre pas encore valide
											uStatus = OpcUa_P_GetDateTimeDiffInSeconds32(ValidFrom, utcNow, &uDiff);
											if ((uStatus != OpcUa_BadOutOfRange) && (uStatus != OpcUa_BadInvalidArgument))
											{
												// Il est peut être mal formé
												uStatus = OpcUa_P_GetDateTimeDiffInSeconds32(ValidFrom, ValidTo, &uDiff);
												if ((uStatus == OpcUa_BadOutOfRange) || (uStatus == OpcUa_BadInvalidArgument))
												{
													OpcUa_Trace(OPCUA_TRACE_LEVEL_ALWAYS, "Your certificate is corrupted. Server is stopping\n");
													ShutdownServer(5, (OpcUa_CharA*)"Server shutdown: Your certificate is corrupted");
													uStatus = OpcUa_BadCertificateInvalid;
												}
											}
											else
											{
												OpcUa_Trace(OPCUA_TRACE_LEVEL_ALWAYS, "Your certificate is not yet valid. Server is stopping\n");
												ShutdownServer(5, (OpcUa_CharA*)"Server shutdown: Your certificate is not yet valid.");
												uStatus = OpcUa_BadCertificateInvalid;
											}
										}
										else
										{
											OpcUa_Trace(OPCUA_TRACE_LEVEL_ALWAYS, "Your certificate is expired. Server is stopping\n");
											ShutdownServer(5, (OpcUa_CharA*)"Server shutdown: Your certificate is expired.");
											uStatus = OpcUa_BadCertificateInvalid;
										}
									}
									if (uStatus == OpcUa_Good)
									{
										uStatus = g_pTheApplication->InitializeSecurity();
										if (uStatus != OpcUa_Good)
										{
											OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "InitializeSecurity failed StatusCode=0x%05x\n", uStatus);
										}
										else
										{
											// make sure the discovery server knows about the server.
											// note: it may take time for the discovery server to recognize a new certificate.
											//g_pTheApplication->AddServerToDiscoveryServerTrustList(OpcUa_False);
											// start the server.
											g_pTheApplication->StartLDSRegistrationThread();

											CApplicationDescription* pAppDescription = g_pTheApplication->GetApplicationDescription(); // pApplicationDescriptionList->at(ii);
											if (pAppDescription)
											{
												/////////////////////////////////////////////////////////////////////////////////////////////
												OpcUa_String* pListener = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
												OpcUa_String_Initialize(pListener);
												if (pAppDescription->GetDiscoveryUrls())
												{
													OpcUa_String* pUrl = pAppDescription->GetDiscoveryUrls();
													if (pUrl)
														OpcUa_String_CopyTo(pUrl, pListener);
													OpcUa_LocalizedText aAppName = pAppDescription->GetApplicationName();
													OpcUa_String_StrnCat(pListener, &(aAppName.Text), OpcUa_String_StrLen(&(aAppName.Text)));
												}
												else
												{
													OpcUa_String* pUri = pAppDescription->GetApplicationUri();
													if (pUri)
														OpcUa_String_CopyTo(pUri, pListener);
												}
												if (pListener)
												{
													OpcUa_Trace(OPCUA_TRACE_LEVEL_ALWAYS, "Server listening at :\n\t%s.\r\n", OpcUa_String_GetRawString(pListener));
													OpcUa_String_Clear(pListener);
													OpcUa_Free(pListener);
												}
												///////////////////////////////////////////////////////////////////////////////////////////////
												//OpcUa_String* aString=pAppDescription->GetApplicationUri();
												//OpcUa_Trace(OPCUA_TRACE_LEVEL_ALWAYS,"Server listening at %s.\r\n", OpcUa_String_GetRawString(aString));
												// démarrage de la thread de surveillance des timeout de l'ensemble des sessions
												g_pTheApplication->StartSessionsTimeoutThread();
											}
											else
												OpcUa_Trace(OPCUA_TRACE_LEVEL_ALWAYS, "Critical error. OpenOpcUaServer cannot start.\r\n");

										}
										// show the summary of nodes
										g_pTheApplication->m_pTheAddressSpace->TraceAddressSpace();

										// Let's start/unlock the EventsEngine thread if needed
										if (pEventsEngine)
										{
											if (pEventsEngine->GetEventDefinitionListSize() > 0)
											{
												OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Will start the EventEngine...\n");
												OpcUa_Semaphore_Post(pEventsEngine->m_EventsThreadSem, 1);
											}
										}
										// Let's start/unlock the simulation thread if needed
										OpcUa_Semaphore_Post(pInformationModel->GetSimulationEvent(), 1);
										// Let's start/unlock the HA Engine in case it's require
										if (pHaEngine)
										{
											OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Will start the HA engine...\n");
											OpcUa_Semaphore_Post(pHaEngine->m_StorageThreadSem, 1);
										}
										OpcUa_Trace(OPCUA_TRACE_LEVEL_ALWAYS, "Press Q or q to exit.\r\n");
										getQChar();
										// stop the server.	
										// We have to enter the full stop process mecanism 
										ShutdownServer(15, (OpcUa_CharA*)"Server shutdown by operator");
										// Will now play an basic animation and update the value of SecondsTillShutdown
										StopAnimation(15);

									}
									else
									{
										// Let's unlock the simulation thread
										OpcUa_Semaphore_Post(pInformationModel->GetSimulationEvent(), 1);
										OpcUa_String certificateStore = g_pTheApplication->GetCertificateStorePath();
										OpcUa_Trace(OPCUA_TRACE_LEVEL_ALWAYS, "Cleanup your certificate Store \n in: .\\%s  \n and restart the server\n", OpcUa_String_GetRawString(&certificateStore));
										OpcUa_Trace(OPCUA_TRACE_LEVEL_ALWAYS, "Press Q or q to exit.\r\n");
										getQChar();
										// stop the server.	
										// We have to enter the full stop process mecanism 
										if (uStatus != OpcUa_BadCertificateInvalid)
											ShutdownServer(10, (OpcUa_CharA*)"Server shutdown Configuration error");
										// Will now play an basic animation and update the value of SecondsTillShutdown
										StopAnimation(10);
									}
								}
								else
									OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critical error> Server certificate is not correct\n");
								OpcUa_String_Clear(&aCertificateStore);
							}
						}
						else
							OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critical error> Error in SubSystem Files\n");
					}
					else
						OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critical error> Error in Simulation Files\n");
				}
				else
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critical error> Error in fileNodeSet\n");
			}
			else
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critical error> Information in server Configuration file are not consistant\n");
				if (pApplicationDescription)
					delete pApplicationDescription;
		}
		else
		{
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critical error : Cannot load ServerConfiguration file. Probably a parsing error\n");
			for (OpcUa_Int32 ii = 0; ii < 100; ii++)
			{
				if ((ii % 10) == 0)
					printf(".");
				OpcUa_Thread_Sleep(100);
			}
			printf("\n");
		}
		if (g_pTheApplication)
		{
			g_pTheApplication->RemoveAllVpiDevice();
		}
		// Cleanup the InvertingThread. Here this thread is suppose to be finish a long time ago
		OpcUa_Thread_Delete(hInvertingReferenceThread);
		// 
		if (CServerApplication::m_pTheAddressSpace)
		{
			delete CServerApplication::m_pTheAddressSpace;
			CServerApplication::m_pTheAddressSpace = OpcUa_Null;
		}
		/*
		if (fileName)
			OpcUa_Free(fileName);
		if (path)
			OpcUa_Free(path);
			*/
		if (g_pTheApplication)
		{
			delete g_pTheApplication;
			g_pTheApplication = OpcUa_Null;
		}
		///////////////////////////////////////////////////
		//
		// Sempahore to remotly stop the server
		OpcUa_Semaphore_Delete(&g_ShutdownServerSem);
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "OpenOpcUaCore server is stopped.\n");
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}
OpcUa_StatusCode RunAsWindowsService(STARTUP_COMMAND eCommand) // char* szCommand, char* path, char* fileName)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
#ifdef WIN32
	HRESULT hr = S_OK;

	OpcUa_CharA* path = g_pTheApplication->GetConfigPath();
	OpcUa_CharA* fileName = g_pTheApplication->GetConfigFileName();

	if (eCommand==STARTUP_COMMAND::INSTALL_SERVICE)
	{
		hr = g_pTheApplication->m_ServiceModule.InstallService(path, fileName);
		if (hr == S_OK)
			uStatus = OpcUa_Good;
		else
			uStatus = OpcUa_BadInternalError;
		hr = S_FALSE; // force to false in order to stop the server startup
	}
	else
	{
		if (eCommand == STARTUP_COMMAND::UNINSTALL_SERVICE)
		{
			hr = g_pTheApplication->m_ServiceModule.UninstallService();
			if (hr == S_OK)
			{
				uStatus = OpcUa_Good;
				hr = S_FALSE; // Force to false in order to stop the server startup
			}
			else
				uStatus = OpcUa_BadInternalError;
		}
	}
	if (eCommand == STARTUP_COMMAND::RUN_SERVER)
	{
		uStatus = OpcUa_BadInvalidArgument;
		OpcUa_Trace(OPCUA_TRACE_SERVER_ERROR, "You cannot run the server as an application when it was installed as a service.\n");
	}
	else
	{
		HKEY hk;
		char szSubKey[256];
		ZeroMemory(szSubKey, 256);
		strcpy(&szSubKey[0], "AppID\\{CB85A975-BAD5-4A1E-A142-FF98330EB221}");
		if (RegOpenKey(HKEY_CLASSES_ROOT,
			szSubKey, &hk))
		{
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "cannot open the registry key.\n");
		}
		RegCloseKey(hk);

		CServerApplication::m_pTheAddressSpace = new CUAInformationModel();
		// Extract configuration parameter from the registry
		g_pTheApplication->m_ServiceModule.ExtractConfigurationParams(&path, &fileName);
		///////////////////////////////////////////////////////////////
		//
		// 
		///////////////////////////////////////////////////
		//
		// Sempahore to remotly stop the server
		OpcUa_Semaphore_Create(&g_ShutdownServerSem, 0, 0x100);

		// Parsing du fichier de configuration du serveur
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Loading %s %s\n", path, fileName);
		uStatus = g_pTheApplication->LoadUaServerConfiguration(path, fileName);
		if (uStatus == OpcUa_Good)
		{
			// Fin chargement du fichier de configuration
			// Mise en place de L'application Description et des informations sur les ports d'écoute
			CApplicationDescription* pApplicationDescription = g_pTheApplication->GetApplicationDescription();
			uStatus = SetServerDescriptionTransportPortListener(&pApplicationDescription);
			if (uStatus == OpcUa_Good)
			{
				g_pTheApplication->SetApplicationDescription(pApplicationDescription);
				//////////////////////////////////////////
				//
				// Definition du niveau de trace
				//
				//////////////////////////////////////////
				// OPCUA_TRACE_OUTPUT_LEVEL_DEBUG
				// OPCUA_TRACE_OUTPUT_LEVEL_INFO
				// OPCUA_TRACE_OUTPUT_LEVEL_WARNING
				// OPCUA_TRACE_OUTPUT_LEVEL_NONE
				// OPCUA_TRACE_OUTPUT_LEVEL_SYSTEM
				// OPCUA_TRACE_OUTPUT_LEVEL_ERROR
				OpcUa_UInt32 uiTraceLevel = g_pTheApplication->GetTraceLevel();
				OpcUa_Trace_ChangeTraceLevel(uiTraceLevel);
				OpcUa_UInt32 iOutput = g_pTheApplication->GetTraceOutput();
				if (iOutput == OPCUA_TRACE_OUTPUT_FILE)
				{
					// Mise en place du nom du fichier de sortie
					OpcUa_String strFileName;
					OpcUa_String_Initialize(&strFileName);
					OpcUa_String strBakFileName;
					OpcUa_String_Initialize(&strBakFileName);
					// extension du fichier
					OpcUa_String strExtension;
					OpcUa_String_Initialize(&strExtension);
					OpcUa_String_AttachCopy(&strExtension, ".log");
					// extension de la sauvegarde
					OpcUa_String strBakExtension;
					OpcUa_String_Initialize(&strBakExtension);
					OpcUa_String_AttachCopy(&strBakExtension, ".bak");
					// create the output filename base on the name of the server EndPoint
					pApplicationDescription = g_pTheApplication->GetApplicationDescription();
					if (pApplicationDescription)
					{
						// On ne cherche que le nom de l'application. Donc Le premier suffira ...
						OpcUa_LocalizedText aLocalizedText = pApplicationDescription->GetApplicationName();
						OpcUa_String_CopyTo(&(aLocalizedText.Text), &strFileName);
						OpcUa_String_CopyTo(&(aLocalizedText.Text), &strBakFileName);

						// creation du nom du fichier de log
						OpcUa_String_StrnCat(&strFileName, &strExtension, OpcUa_String_StrLen(&strExtension));
						// creation du nom du fichier de sauvegarde
						OpcUa_String_StrnCat(&strBakFileName, &strBakExtension, OpcUa_String_StrLen(&strBakExtension));
						OpcUa_Trace_SetTraceFile(strFileName);
						// On sauvegarde une eventuelle trace existante
						FILE* hFile = OpcUa_Null;
						hFile = fopen(OpcUa_String_GetRawString(&strFileName), "r");
						if (hFile)
						{
							// Ouverture du fichier de sauvegarde
							FILE* hFileBak = OpcUa_Null;
							hFileBak = fopen(OpcUa_String_GetRawString(&strBakFileName), "w");
							char bakBuffer;
							// On lit l'ensemble du fichier dans le buffer
							// On copie dans la sauvegarde
							while (fread(&bakBuffer, sizeof(char), 1, hFile) == 1)
								fwrite(&bakBuffer, sizeof(char), 1, hFileBak);
							fclose(hFileBak);
							// On supprime l'ancien fichier 
							hFile = fopen(OpcUa_String_GetRawString(&strFileName), "w");
							if (hFile)
								fclose(hFile);
						}
					}
					OpcUa_String_Clear(&strExtension);
					OpcUa_String_Clear(&strFileName);
					OpcUa_String_Clear(&strBakExtension);
					OpcUa_String_Clear(&strBakFileName);
				}

				// Chargement des fichiers NodeSet qui constitueront l'espace d'adressage
				uStatus = LoadNodeSetFiles();
				if (uStatus == OpcUa_Good)
				{
					uStatus = LoadSimulationFiles();
					if (uStatus == OpcUa_Good)
					{
						// Chargement de la configuration des subsystems
						uStatus = LoadSubSystemFiles();
						if (uStatus == OpcUa_Good)
						{
							CEventsEngine* pEventsEngine = g_pTheApplication->GetEventsEngine();
							pEventsEngine = new CEventsEngine();
							if (pEventsEngine)
								g_pTheApplication->SetEventsEngine(pEventsEngine);
							/////////////////////////////////////////////////////////
							// PostProcessing
							//
							uStatus = PostProcessing();
							if (uStatus != OpcUa_Good)
								OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Post Processing failed 0x%05x\n", uStatus);
							else
							{
								CUAInformationModel* pInformationModel = CServerApplication::m_pTheAddressSpace;
								// Déblocage de la thread de mise à jour des MandatoryNodes
								OpcUa_Semaphore_Post(pInformationModel->m_SemMandatoryEvent, 1);
								// démarrage du mecanisme de dialogue avec les sous-systèmes
								//g_pTheApplication->WakeupAllVpi();

								// Si besoin demarrage du moteur d'archivage
								CHaEngine* pHaEngine = g_pTheApplication->GetHaEngine();
								if (pHaEngine)
								{
									if (pHaEngine->LoadVfi() == OpcUa_Good)
									{
										OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Your Vfi was correctly loaded\n");
										pHaEngine->InitHaEngine();	// Call GlobalStart
										OpcUa_Vfi_Handle hVfiHandle = OpcUa_Null;
										OpcUa_String szArchiveId = g_pTheApplication->GetArchiveId();
										OpcUa_NodeId aNodeId;
										OpcUa_NodeId_Initialize(&aNodeId);
										if (ParseNodeId(OpcUa_String_GetRawString(&szArchiveId), &aNodeId) == OpcUa_Good)
										{
											// Warning the Vfi is suppose to copy the aNodeId
											uStatus = pHaEngine->VfiGlobalStart(g_pTheApplication->GetArchiveName(),
												aNodeId,
												&hVfiHandle);
											if (uStatus == OpcUa_Good)
											{
												pHaEngine->SetVfiHandle(hVfiHandle);
												uStatus = pHaEngine->VfiColdStart(hVfiHandle);
												if (uStatus != OpcUa_Good)
													OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "VfiColdStart failed 0x%05x\n", uStatus);
											}
											else
												OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "VfiGlobalStart failed 0x%05x\n", uStatus);
										}
										else
											OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "ParseNodeId failed when preparing the call to VfiGlobalStart\n");
										OpcUa_NodeId_Clear(&aNodeId);
									}
								}
								// initialize security by specifying the client certificate and the location of the trusted certificates.

								OpcUa_String aCertificateStore;
								OpcUa_String_Initialize(&aCertificateStore);
								OpcUa_String_AttachCopy(&aCertificateStore, "CertificateStore");
								g_pTheApplication->SetCertificateStorePath(aCertificateStore);
								uStatus = g_pTheApplication->LoadPFXCertificate();
								if (uStatus == OpcUa_Good)
								{
									// on va tenter de lire le fichier DER afin de vérifier qu'il s'agit du bon DER
									// a savoir : 
									//	1- il existe
									//	2- sa clé correpsond acelle du pfx
									uStatus = g_pTheApplication->LoadDERCertificate();
									if (uStatus != OpcUa_Good)
									{
										OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Your certificate in DER file not fit the private key. Will create a new one\n");
										uStatus = g_pTheApplication->CreateCertificate();
									}
								}
								else
									uStatus = g_pTheApplication->CreateCertificate();

								// on va vérifier que le certificat du serveur est encore valide
								OpcUa_CharA* sThumbprint = OpcUa_Null;
								OpcUa_CharA* lApplicationUri = OpcUa_Null;
								OpcUa_CharA* psCommonName = OpcUa_Null;
								OpcUa_ByteString* pServerCertificate = g_pTheApplication->GetCertificate();
								if (pServerCertificate)
								{
									uStatus = OpcUa_Certificate_GetInfo(
										(OpcUa_ByteString*)pServerCertificate,
										OpcUa_Null,
										OpcUa_Null,
										&psCommonName,
										&sThumbprint,
										&lApplicationUri,
										OpcUa_Null,
										OpcUa_Null);
									OpcUa_DateTime ValidFrom, ValidTo;
									OpcUa_DateTime_Initialize(&ValidFrom);
									OpcUa_DateTime_Initialize(&ValidTo);

									uStatus = OpcUa_Certificate_GetDateBound(
										(OpcUa_ByteString*)pServerCertificate,
										&ValidFrom, &ValidTo);
									if (uStatus == OpcUa_Good)
									{
										// Verification de la validitée du certificat;
										OpcUa_String* strFrom = OpcUa_Null;// (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
										OpcUa_String* strTo = OpcUa_Null; // (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
										Utils::OpcUaDateTimeToString(ValidFrom, &strFrom);
										Utils::OpcUaDateTimeToString(ValidTo, &strTo);
										OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Your certificate validate your server \nfrom: %s \nto: %s\n",
											OpcUa_String_GetRawString(strFrom),
											OpcUa_String_GetRawString(strTo));
										if (strTo)
										{
											OpcUa_String_Clear(strTo);
											OpcUa_Free(strTo);
										}
										if (strFrom)
										{
											OpcUa_String_Clear(strFrom);
											OpcUa_Free(strFrom);
										}
										// Est ce que la date de fin est dépassée
										OpcUa_DateTime utcNow = OpcUa_DateTime_UtcNow();
										OpcUa_UInt32 uDiff = 0;
										uStatus = OpcUa_P_GetDateTimeDiffInSeconds32(utcNow, ValidTo, &uDiff);
										if ((uStatus != OpcUa_BadOutOfRange) && (uStatus != OpcUa_BadInvalidArgument))
										{
											// Il n'est peut etre pas encore valide
											uStatus = OpcUa_P_GetDateTimeDiffInSeconds32(ValidFrom, utcNow, &uDiff);
											if ((uStatus != OpcUa_BadOutOfRange) && (uStatus != OpcUa_BadInvalidArgument))
											{
												// Il est peut être mal formé
												uStatus = OpcUa_P_GetDateTimeDiffInSeconds32(ValidFrom, ValidTo, &uDiff);
												if ((uStatus == OpcUa_BadOutOfRange) || (uStatus == OpcUa_BadInvalidArgument))
												{
													OpcUa_Trace(OPCUA_TRACE_LEVEL_ALWAYS, "Your certificate is corrupted. Server is stopping\n");
													ShutdownServer(5, (OpcUa_CharA*)"Server shutdown: Your certificate is corrupted");
													uStatus = OpcUa_BadCertificateInvalid;
												}
											}
											else
											{
												OpcUa_Trace(OPCUA_TRACE_LEVEL_ALWAYS, "Your certificate is not yet valid. Server is stopping\n");
												ShutdownServer(5, (OpcUa_CharA*)"Server shutdown: Your certificate is not yet valid.");
												uStatus = OpcUa_BadCertificateInvalid;
											}
										}
										else
										{
											OpcUa_Trace(OPCUA_TRACE_LEVEL_ALWAYS, "Your certificate is expired. Server is stopping\n");
											ShutdownServer(5, (OpcUa_CharA*)"Server shutdown: Your certificate is expired.");
											uStatus = OpcUa_BadCertificateInvalid;
										}
									}
									if (uStatus == OpcUa_Good)
									{
										uStatus = g_pTheApplication->InitializeSecurity();
										if (uStatus != OpcUa_Good)
										{
											OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "InitializeSecurity failed StatusCode=0x%05x\n", uStatus);
										}
										else
										{
											// make sure the discovery server knows about the server.
											// note: it may take time for the discovery server to recognize a new certificate.
											//g_pTheApplication->AddServerToDiscoveryServerTrustList(OpcUa_False);
											// start the server.
											g_pTheApplication->StartLDSRegistrationThread();

											CApplicationDescription* pAppDescription = g_pTheApplication->GetApplicationDescription(); // pApplicationDescriptionList->at(ii);
											if (pAppDescription)
											{
												/////////////////////////////////////////////////////////////////////////////////////////////
												OpcUa_String* pListener = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
												OpcUa_String_Initialize(pListener);
												if (pAppDescription->GetDiscoveryUrls())
												{
													OpcUa_String* pUrl = pAppDescription->GetDiscoveryUrls();
													if (pUrl)
														OpcUa_String_CopyTo(pUrl, pListener);
													OpcUa_LocalizedText aAppName = pAppDescription->GetApplicationName();
													OpcUa_String_StrnCat(pListener, &(aAppName.Text), OpcUa_String_StrLen(&(aAppName.Text)));
												}
												else
												{
													OpcUa_String* pUri = pAppDescription->GetApplicationUri();
													if (pUri)
														OpcUa_String_CopyTo(pUri, pListener);
												}
												if (pListener)
												{
													OpcUa_Trace(OPCUA_TRACE_LEVEL_ALWAYS, "Server listening at :\n\t%s.\r\n", OpcUa_String_GetRawString(pListener));
													OpcUa_String_Clear(pListener);
													OpcUa_Free(pListener);
												}
												///////////////////////////////////////////////////////////////////////////////////////////////
												//OpcUa_String* aString=pAppDescription->GetApplicationUri();
												//OpcUa_Trace(OPCUA_TRACE_LEVEL_ALWAYS,"Server listening at %s.\r\n", OpcUa_String_GetRawString(aString));
												// démarrage de la thread de surveillance des timeout de l'ensemble des sessions
												g_pTheApplication->StartSessionsTimeoutThread();
											}
											else
												OpcUa_Trace(OPCUA_TRACE_LEVEL_ALWAYS, "Critical error. OpenOpcUaServer cannot start.\r\n");

										}
										g_pTheApplication->m_pTheAddressSpace->TraceAddressSpace();

										// Let's start/unlock the EventsEngine thread if needed
										if (pEventsEngine)
										{
											if (pEventsEngine->GetEventDefinitionListSize() > 0)
											{
												OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Will start the EventEngine...\n");
												OpcUa_Semaphore_Post(pEventsEngine->m_EventsThreadSem, 1);
											}
										}
										// Let's start/unlock the simulation thread if needed
										OpcUa_Semaphore_Post(pInformationModel->GetSimulationEvent(), 1);
										// Let's start/unlock the HA Engine in case it's require
										if (pHaEngine)
										{
											OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Will start the HA engine...\n");
											OpcUa_Semaphore_Post(pHaEngine->m_StorageThreadSem, 1);
										}
										g_bRun = OpcUa_True;
										g_pTheApplication->m_ServiceModule.Start();
										g_bRun = OpcUa_False;
										// When we get here, the service has been stopped
										//
										// stop the server.	
										// We have to enter the full stop process mecanism 
										ShutdownServer(15, (OpcUa_CharA*)"Server shutdown by operator");
										// Will now play an basic animation and update the value of SecondsTillShutdown
										StopAnimation(15);
									}
									else
									{
										// Let's unlock the simulation thread
										OpcUa_Semaphore_Post(pInformationModel->GetSimulationEvent(), 1);
										OpcUa_String certificateStore = g_pTheApplication->GetCertificateStorePath();
										OpcUa_Trace(OPCUA_TRACE_LEVEL_ALWAYS, "Cleanup your certificate Store \n in: .\\%s  \n and restart the server\n", OpcUa_String_GetRawString(&certificateStore));
										OpcUa_Trace(OPCUA_TRACE_LEVEL_ALWAYS, "Press Q or q to exit.\r\n");
										getQChar();
										// stop the server.	
										// We have to enter the full stop process mecanism 
										if (uStatus != OpcUa_BadCertificateInvalid)
											ShutdownServer(10, (OpcUa_CharA*)"Server shutdown Configuration error");
										// Will now play an basic animation and update the value of SecondsTillShutdown
										StopAnimation(10);
									}
								}
								else
									OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critical error> Server certificate is not correct\n");
								OpcUa_String_Clear(&aCertificateStore);
							}
						}
						else
							OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critical error> Error in SubSystem Files\n");
					}
					else
						OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critical error> Error in Simulation Files\n");
				}
				else
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critical error> Error in fileNodeSet\n");
			}
			else
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critical error> Information in server Configuration file are not consistant\n");
		}
		else
		{
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critical error : Cannot load ServerConfiguration file. Probably a parsing error\n");
			for (OpcUa_Int32 ii = 0; ii < 100; ii++)
			{
				if ((ii % 10) == 0)
					printf(".");
				OpcUa_Thread_Sleep(100);
			}
			printf("\n");
		}
		if (g_pTheApplication)
		{
			g_pTheApplication->RemoveAllVpiDevice();
		}
		// Cleanup the InvertingThread. Here this thread is suppose to be finish a long time ago
		OpcUa_Thread_Delete(hInvertingReferenceThread);
		// 
		if (CServerApplication::m_pTheAddressSpace)
		{
			delete CServerApplication::m_pTheAddressSpace;
			CServerApplication::m_pTheAddressSpace = OpcUa_Null;
		}
		if (fileName)
			OpcUa_Free(fileName);
		if (path)
			OpcUa_Free(path);
		if (g_pTheApplication)
		{
#ifdef WIN32
			SERVICE_STATUS uServiceStatus = g_pTheApplication->m_ServiceModule.GetServiceStatus();
			uStatus = uServiceStatus.dwWin32ExitCode;
#endif
			delete g_pTheApplication;
			g_pTheApplication = OpcUa_Null;
		}
		///////////////////////////////////////////////////
		//
		// Sempahore to remotly stop the server
		OpcUa_Semaphore_Delete(&g_ShutdownServerSem);
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "OpenOpcUaCore server is stopped.\n");
	}	
		
#endif
	return uStatus;
}
// This application needs to have the uastack and OpenSSL DLLs in its path to run.
//
// Open the Project Properties | Debugging page
// Add this string to the Environment setting:
//
// PATH=%PATH%;
//
// Le fichier de configuration sera passé en ligne de commande
// 
// 
#ifdef WIN32
int _tmain(int argc, _TCHAR* argv[])
{
#if defined(WIN32) || !defined(_WIN32_WCE)
	#ifdef _DEBUG
		_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG | _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	#endif
#endif

#endif
#ifdef _GNUC_
int main(int argc, char* argv[])
{
#endif

	OpcUa_ReferenceParameter(argc);
	OpcUa_ReferenceParameter(argv);
	
	OpcUa_String aString;
	OpcUa_String_Initialize(&aString);
	try
	{
		//Sleep(7000); // Sleep to debug the service version
		//////////////////////////////////////////////////////////
		// 
		// initialize the application and the stack.
		g_pTheApplication = new CServerApplication();
		if (g_pTheApplication)
			g_pTheApplication->Initialize();
		STARTUP_COMMAND eCommand = COMMAND_UNKNOWN;
		switch (argc)
		{
		case 1: // no parameter is an incorrect call
			printf("The server cannot start. Please use the correct syntax\n");
			printf("The syntax is : OpenOpcUa_CoreServer <fullpath\\configfile.xml>\n");
			break;
		case 2: // One parameter is the correct syntax to start the server for both as app or as a service
			// Parse the file parameter or Service name if started as a service
			g_pTheApplication->ParseFirstParameter(argv[1],&eCommand);
			if (eCommand == COMMAND_UNKNOWN)
			{
				printf("The server cannot start. Please use the correct syntax\n");
				printf("The syntax is : OpenOpcUa_CoreServer <fullpath\\configfile.xml>\n");
			}
			break;
		case 3: // Two parameters is correct for server registration as a Windows service
#ifdef _GNUC_
			printf("The server cannot start. Please use the correct syntax\n");
			printf("The syntax is : OpenOpcUa_CoreServer <fullpath\\configfile.xml>\n");
#endif
#ifdef WIN32
			// Parse the file parameter 
			g_pTheApplication->ParseFirstParameter(argv[1], &eCommand);
			// Parse the service registration/unregistration info
			g_pTheApplication->ParseSecondParameter(argv[2],&eCommand);
#endif
			break;
		default:
			printf("The server cannot start. Please use the correct syntax\n");
			printf("The syntax is : OpenOpcUa_CoreServer <fullpath\\configfile.xml>\n");
			break;
		}


#ifdef WIN32
		if (!g_pTheApplication->m_ServiceModule.IsInstalled())
		{
			{
				if (eCommand == STARTUP_COMMAND::INSTALL_SERVICE)
				{
					RunAsWindowsService(eCommand);
				}
				else
				{
					if (eCommand == STARTUP_COMMAND::RUN_SERVER)
						RunAsApplication(eCommand);
					else
					{
						printf("The server cannot start. Please use the correct syntax\n");
						printf("The syntax is : OpenOpcUa_CoreServer <fullpath\\configfile.xml>\n");
					}
				}
			}
		}
		else
		{
			// If the server is installed as a service. and no parameter was on the command line let's start as a service
			if ((g_pTheApplication->m_ServiceModule.IsInstalled()) && (g_pTheApplication->m_ServiceModule.IsService()))
				RunAsWindowsService(eCommand);
			else
				RunAsApplication(eCommand);
		}
#endif
#ifdef _GNUC_
		if (eCommand == RUN_SERVER)	
			RunAsApplication(eCommand);
		else
		{
			printf("The server cannot start. Please use the correct syntax\n");
			printf("The syntax is : OpenOpcUa_CoreServer <fullpath\\configfile.xml>\n");
		}

#endif
	}
	catch (CStatusCodeException* e)
	{
		if (e)
		{
			OpcUa_String aString=e->GetMessage();
			OpcUa_CharA* pMessage=OpcUa_String_GetRawString(&aString); 
			if (pMessage)
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"ERROR [0x%08X]: %s\r\n", e->GetCode(), *pMessage);
			else
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"ERROR [0x%08X]: \r\n", e->GetCode());
		}
		else
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Critical error> in Main.cpp\n");
		return 0;
	}
	catch (std::exception e)
	{
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"ERROR> %s\n",e.what());
		return 0;
	}

	return 0;
}
#ifdef _GNUC_
// strips off all characters entered on a line except for the first one.

int _kbhit(void)
{
	struct termios oldt, newt;
	int ch;  int oldf;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
	ch = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);
	if (ch != EOF)
	{
		ungetc(ch, stdin);
		return 1;
	}
	return 0;
}

#endif

// strips off all characters entered on a line except for the first one.

int getQChar()
{
	int Choice=0;
	g_bRun = OpcUa_True;
	while (g_bRun)
	{
		if (_kbhit())
		{
#ifdef _GNUC_
			Choice = getchar();
#endif
#ifdef WIN32
			Choice = _getch();
#endif
			switch (Choice)
			{
			case 0x51:
			case 0x71:
				g_bRun = OpcUa_False;
				break;
			default:
				break;
			}
		}
		OpcUa_Semaphore_TimedWait(g_ShutdownServerSem, 1);
	}
	return Choice;
}

OpcUa_StatusCode AddLoggerMessage(wchar_t* szMessage, DWORD dwLevel)
{
	OpcUa_StatusCode hr = OpcUa_Good;
	OpcUa_ReferenceParameter(dwLevel);
	OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"%ls\n",szMessage);
	return hr;
}

/// <summary>
/// Finds the type definition for the aNodeId passed in.
/// </summary>
/// <param name="aNodeId">A node identifier.</param>
/// <param name="pDefinition">The p definition.</param>
/// <returns>OpcUa_StatusCode OpcUa_Good if CDefintion properly found</returns>
OpcUa_StatusCode FindTypeDefinition(OpcUa_NodeId aNodeId, CDefinition** pDefinition)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	CUADataType* pUADataType=NULL;
	CUAInformationModel* pInformationModel=CServerApplication::m_pTheAddressSpace;
	uStatus = pInformationModel->GetNodeIdFromDataTypeList(aNodeId, &pUADataType);
	if (uStatus==OpcUa_Good)
		*pDefinition=pUADataType->GetDefinition();
	return uStatus;
}

// Function name   : CUAInformationModel::FindBuiltinType
// Description     : This function is Looking for the builtin type for a type specify in a NodeId
// Return type     : OpcUa_StatusCode OpcUa_Good if the function succeed, OpcUa_BadNotFound is not
// Argument        : OpcUa_NodeId aNodeId = DataType declare in the NodeId
// Argument        : OpcUa_Byte* pBuiltInType [out] related BuiltInType 
OpcUa_StatusCode FindBuiltinType(OpcUa_NodeId aNodeId, OpcUa_Byte* pBuiltInType)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	CUADataType* pUADataType=NULL;
	CUAInformationModel* pInformationModel=CServerApplication::m_pTheAddressSpace;
	// Afin de prendre en compte la désorganisation du fichier Opc.Ua.NodeSet2.xml
	// Je vais traiter quelques types particuliers manuellement
	if ( (aNodeId.NamespaceIndex==0) && (aNodeId.Identifier.Numeric==0)) // Probablement une erreur dans le fichier XML. Je vais considérer que le type par défaut est BaseDataType ns=0;i=24
		*pBuiltInType=24; //BaseDataType
	else
	{
		if ( (aNodeId.NamespaceIndex==0) && (aNodeId.Identifier.Numeric==29)) //Enumeration
			*pBuiltInType=6; // une enumeration sera traité comme un Int32
		else
		{
			// 338 = BuildInfo, 862 = ServerStatusDataType, 296 = Argument, 859 = ServerDiagnosticsSummaryDataType, 874 = SubscriptionDiagnosticsDataType
			// 871 = ServiceCounterDataType, 865 = SessionDiagnosticsDataType, 868 = SessionSecurityDiagnosticsDataType, 853 = RedundantServerDataType, 
			// 856 = SamplingIntervalDiagnosticsDataType, 877 = ModelChangeStructureDataType, 884 = Range, 887 = EUInformation, 897 = SemanticChangeStructureDataType
			// 890 = ExceptionDeviationFormat, 891 = Annotation, 11944 = NetworkGroupDataType, 12079 = AxisInformation, 8912 = TimeZoneDataType
			// This default answer accelerate the look up and fix issues in the OPC FOundation Part 5 nodeset sequential declaration
			if ((
				(aNodeId.Identifier.Numeric == 338) || 
				(aNodeId.Identifier.Numeric == 853) ||
				(aNodeId.Identifier.Numeric == 856) ||
				(aNodeId.Identifier.Numeric == 859) ||
				(aNodeId.Identifier.Numeric == 865) ||
				(aNodeId.Identifier.Numeric == 868) ||
				(aNodeId.Identifier.Numeric == 871) ||
				(aNodeId.Identifier.Numeric == 874) ||
				(aNodeId.Identifier.Numeric == 877) ||
				(aNodeId.Identifier.Numeric == 884) ||
				(aNodeId.Identifier.Numeric == 887) ||
				(aNodeId.Identifier.Numeric == 890) ||
				(aNodeId.Identifier.Numeric == 891) ||
				(aNodeId.Identifier.Numeric == 897) ||
				(aNodeId.Identifier.Numeric == 338) || 
				(aNodeId.Identifier.Numeric == 862) ||
				(aNodeId.Identifier.Numeric == 8912) ||
				(aNodeId.Identifier.Numeric == 11944) ||
				(aNodeId.Identifier.Numeric == 12079) ||
				(aNodeId.Identifier.Numeric == 296) ) && 
				(aNodeId.NamespaceIndex == 0) )
				*pBuiltInType=22;
			else
			{
				// 852 = ServerState, 851 = RedundancySupport, 12077 = AxisScaleEnumeration, 
				if ((
					(aNodeId.Identifier.Numeric == 851) || 
					(aNodeId.Identifier.Numeric == 852) || 
					(aNodeId.Identifier.Numeric == 12077)) && 
					(aNodeId.NamespaceIndex == 0) )
				{ 
					*pBuiltInType=29; // Enumeration
				}
				else
				{
					// check the consistency of param
					// Si on recoit un builtInType on retourne directement ce type
					if ((aNodeId.NamespaceIndex == 0) && (aNodeId.Identifier.Numeric <= 25) && (aNodeId.Identifier.Numeric > 0))
						*pBuiltInType = (OpcUa_Byte)aNodeId.Identifier.Numeric; // cast to byte
					else
					{
						uStatus = pInformationModel->GetNodeIdFromDataTypeList(aNodeId, &pUADataType);
						if (uStatus == OpcUa_Good)
						{
							// on recupère la reference HasSubType
							OpcUa_NodeId NodeIdHasSubType;
							OpcUa_NodeId_Initialize(&NodeIdHasSubType);
							NodeIdHasSubType.Identifier.Numeric = 45;
							NodeIdHasSubType.NamespaceIndex = 0;
							NodeIdHasSubType.IdentifierType = OpcUa_IdentifierType_Numeric;
							//OpcUa_ReferenceNodeList* pRefNodeList= pUADataType->GetReferenceNodeList();
							CUAReferenceList* pRefNodeList = pUADataType->GetReferenceNodeList();
							for (OpcUa_UInt32 ii = 0; ii < pRefNodeList->size(); ii++)
							{
								//OpcUa_ReferenceNode* pRefNode=pRefNodeList->at(ii);
								CUAReference* pRefNode = pRefNodeList->at(ii);
								OpcUa_NodeId aRefNodeId = pRefNode->GetReferenceTypeId();
								if (Utils::IsEqual(&aRefNodeId, &NodeIdHasSubType))
								{
									OpcUa_NodeId aTargetId = pRefNode->GetTargetId().NodeId;
									uStatus = FindBuiltinType(aTargetId, pBuiltInType);
									break;
								}
							}
						}
						else
							uStatus = OpcUa_BadNotFound;
					}
				}
			}
		}
	}
	return uStatus;
}
// analyse un nodeid afin de déterminer s'il est syntaxiquement correct ou nom
//un node in correcte syntaxiquement est "ns=2;s="
// ns>3
OpcUa_StatusCode IsNodeIdValid(const OpcUa_NodeId  aNodeId)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	switch (aNodeId.IdentifierType)
	{
	case OpcUa_IdentifierType_Numeric:
		break;
	case OpcUa_IdentifierType_String:
		//if (OpcUa_String_StrLen(&(aNodeId.Identifier.String))==0)
		//	uStatus=OpcUa_BadNodeIdInvalid;
		break;
	case OpcUa_IdentifierType_Guid:
		break;
	case OpcUa_IdentifierType_Opaque:
		//if (aNodeId.Identifier.ByteString.Data==OpcUa_Null)
		//	uStatus=OpcUa_BadNodeIdInvalid;
		break;
	default:
		uStatus=OpcUa_BadNodeIdInvalid;
		break;
	}
	return uStatus;
}
/// <summary>
/// Parses the node identifier The parsed char* should be in for define by the specification ie: ns=1;i=256 or ns=1;s=myNode .
/// </summary>
/// <param name="atts">The input string to parse.</param>
/// <param name="pNodeId">The pNode identifier this outpout parameter should be allocated and release by the caller.</param>
/// <returns></returns>
OpcUa_StatusCode ParseNodeId(const char* atts,OpcUa_NodeId* pNodeId)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	if (atts)
	{
		//OpcUa_StatusCode uStatus=OpcUa_Bad;
		OpcUa_UInt32 iNamespace;
		OpcUa_UInt32 iNodeId;
		if (!pNodeId)
			uStatus = OpcUa_BadInvalidArgument;
		else
		{
			//CUAInformationModel* pInformationModel=CServerApplication::m_pTheAddressSpace;
			// Format isbg
			// i = numeric
			// s = string
			// b = opaque
			// g = guid
			// On verifie la cohérence syntaxique de atts
			int iRes=-1;
			std::string str(atts);
			std::string szprefix=str.substr(0,3);
			if (szprefix=="ns=")
			{
				OpcUa_UInt32 iPos=str.find(";");
				if (iPos!=string::npos)
				{
					switch (str.at(iPos+1))
					{
					case 105: // i
						iRes = sscanf_s(atts, "ns=%u;i=%u", (unsigned int*)&iNamespace, (unsigned int*)&iNodeId);
						if (iRes == 2)
						{
							//// Recherche du namespace réel
							CUAInformationModel* pInformationModel = CServerApplication::m_pTheAddressSpace;
							// il faut recherche le CNamespaceUri ou m_indexReference==iNamespace
							OpcUa_Int32 iIndexNS = -1; //							
							uStatus = pInformationModel->OnLoadNamespaceUriToAbsoluteNamespaceUri(iNamespace, &iIndexNS);
							if (uStatus == OpcUa_Good)
							{
								CNamespaceUri* pNamespaceUri = pInformationModel->GetNamespaceUri(iIndexNS);
								if (!pNamespaceUri)
								{
									OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critical error this namespace was not define. Check your NodeSet file\n");
									uStatus = OpcUa_BadInternalError;
								}
								else
								{
									// affection des informations dans le nodeid qui sera renvoyé
									pNodeId->IdentifierType = OpcUa_IdentifierType_Numeric;
									pNodeId->NamespaceIndex = (OpcUa_UInt16)pNamespaceUri->GetAbsoluteIndex();
									pNodeId->Identifier.Numeric = iNodeId;
								}
							}
							else
								OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critical error. Cannot retrieve the Absolute namespace Uri Index. Check your NodeSet file\n");
						}
						else
							uStatus = OpcUa_BadInvalidArgument;
						break;
					case 115: //s
					{
						OpcUa_CharA* strId = (OpcUa_CharA*)OpcUa_Alloc(256);
						if (strId)
						{
							ZeroMemory(strId, 256);
							int iRes115 = sscanf(atts, "ns=%u;s=%s", (unsigned int*)&iNamespace, strId);
							if (iRes115 == 2)
							{
								std::string szIdentifier = str.substr(iPos + 3, str.size() - (iPos + 3));
								//// Recherche du namespace réel
								CUAInformationModel* pInformationModel = CServerApplication::m_pTheAddressSpace;
								// il faut recherche le CNamespaceUri ou m_indexReference==iNamespace
								OpcUa_Int32 iIndexNS = -1;
								uStatus = pInformationModel->OnLoadNamespaceUriToAbsoluteNamespaceUri(iNamespace, &iIndexNS);
								if (uStatus == OpcUa_Good)
								{
									CNamespaceUri* pNamespaceUri = pInformationModel->GetNamespaceUri(iIndexNS);
									if (!pNamespaceUri)
									{
										OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critical error this namespace was not define. Check your NodeSet file\n");
										uStatus = OpcUa_BadInvalidArgument;
									}
									else
									{
										// affection des informations dans le nodeid qui sera renvoyé
										pNodeId->IdentifierType = OpcUa_IdentifierType_String;
										pNodeId->NamespaceIndex = (OpcUa_UInt16)pNamespaceUri->GetAbsoluteIndex();
										OpcUa_String_AttachCopy(&(pNodeId->Identifier.String), (OpcUa_CharA*)szIdentifier.c_str());
									}
								}
								else
									OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critical error. Cannot retrieve the Absolute namespace Uri Index. Check your NodeSet file\n");
							}
							else
								uStatus = OpcUa_BadInvalidArgument;
							OpcUa_Free(strId);
						}
					}
						break;
					case 98: //b
						uStatus=OpcUa_BadNotImplemented;
						break;
					case 103: //g
						uStatus=OpcUa_BadNotImplemented;
						break;
					default:
						uStatus=OpcUa_BadNotSupported;
						break;
					}
				}
			}
			else
			{
				switch (str.at(0))
				{
				case 105: // i
					iRes=sscanf_s(atts,"i=%u",(unsigned int*)&iNodeId);
					if (iRes==1)
					{
						pNodeId->IdentifierType=OpcUa_IdentifierType_Numeric;
						pNodeId->NamespaceIndex=0;
						pNodeId->Identifier.Numeric=iNodeId;
					}
					else
						uStatus=OpcUa_BadInvalidArgument;
					break;
				case 115: //s
					uStatus=OpcUa_BadNotImplemented;
					break;
				case 98: //b
					uStatus=OpcUa_BadNotImplemented;
					break;
				case 103: //g
					uStatus=OpcUa_BadNotImplemented;
					break;
				default:
					uStatus=OpcUa_BadInvalidArgument;
					break;
				}
			}
			// traitement des alias
			if (uStatus !=OpcUa_Good)
			{
				CUAInformationModel* pInformationModel=CServerApplication::m_pTheAddressSpace;
				CAliasList* pAliasList=pInformationModel->GetAliasList();
				if (pAliasList)
				{
					for (OpcUa_UInt16 ii=0;ii<pAliasList->size();ii++)
					{
						CAlias* pAlias=pAliasList->at(ii);
						OpcUa_String aString=pAlias->GetAliasName();
						if (OpcUa_StrCmpA(atts,OpcUa_String_GetRawString(&aString))==0)
						{
							*pNodeId=pAlias->GetNodeId();
							uStatus=OpcUa_Good;
							break;
						}
					}
				}
			}
		}
	}
	else
		uStatus=OpcUa_BadInvalidArgument;
	return uStatus;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Creates node identifier. </summary>
///
/// <remarks>	Michel, 23/09/2016. </remarks>
///
/// <param name="IdentifierType">	Type of the identifier. </param>
/// <param name="namespaceIndex">	Zero-based index of the namespace. </param>
/// <param name="pNodeId">		 	[in,out] If non-null, identifier for the node. </param>
///
/// <returns>	The new node identifier. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode CreateNodeId(OpcUa_UInt16 IdentifierType, OpcUa_UInt16 namespaceIndex, OpcUa_UInt32 InitialValue, OpcUa_NodeId* pNodeId)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	CUAInformationModel* pInformationModel = CServerApplication::m_pTheAddressSpace;
	if (pNodeId)
	{
		OpcUa_NodeId_Initialize(pNodeId);
		pNodeId->IdentifierType = IdentifierType;
		pNodeId->NamespaceIndex = namespaceIndex;
		pNodeId->Identifier.Numeric = InitialValue;
		CUABase* pUABase = OpcUa_Null;
		while (pInformationModel->GetNodeIdFromDictionnary(*pNodeId, &pUABase) == OpcUa_Good)
		{
			pNodeId->Identifier.Numeric++;
		}
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}
// Il s'agit de préparer le code d'etat en fonction de attente de la specification UA
// Ce OpcUa_StatusCode servira de base pour la selection du code a retourner lors des lectures
// Plus loin dans le code le resultat sera traité de cette manière
//		if (uStatusPreChecked==OpcUa_BadIndexRangeInvalid)
//			pResponse->Results[ii].StatusCode=uStatusPreChecked;	
//		else
//			pResponse->Results[ii].StatusCode=OpcUa_Good;
OpcUa_StatusCode PreCheckedStatusCode(OpcUa_ReadValueId* pNodeToRead, CUABase* pUABase)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	if (OpcUa_String_StrLen(&(pNodeToRead->IndexRange))>0)
	{
		uStatus = OpcUa_BadOutOfRange;
		if ( (pNodeToRead->AttributeId == OpcUa_Attributes_Value) 
			&& ((pUABase->GetNodeClass()==OpcUa_NodeClass_Variable) || ( pUABase->GetNodeClass()==OpcUa_NodeClass_VariableType) ) 
			&& ( ( ((CUAVariable*)pUABase)->GetBuiltInType()==OpcUaType_Variant) || ( ((CUAVariable*)pUABase)->GetBuiltInType()==OpcUaType_ByteString) || (((CUAVariable*)pUABase)->GetBuiltInType()==OpcUaType_String) ) )
			uStatus=OpcUa_Good;
	}
	else
	{
		uStatus=OpcUa_BadAttributeIdInvalid;
	}
	return uStatus;
}
// Pour un nodeId donnée recherche le nodeId inverse associé
OpcUa_StatusCode FindConsistentInverseNodeId(OpcUa_NodeId aReferenceNodeId,OpcUa_NodeId* pConsistentInverseNodeId)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	if (pConsistentInverseNodeId)
	{
		OpcUa_NodeId_Initialize(pConsistentInverseNodeId);
		pConsistentInverseNodeId->IdentifierType=OpcUa_IdentifierType_Numeric;
		if (aReferenceNodeId.IdentifierType==OpcUa_IdentifierType_Numeric)
		{
			if (aReferenceNodeId.Identifier.Numeric==45) // HasSubtype
				pConsistentInverseNodeId->Identifier.Numeric=40; // HasTypeDefinition
			if (aReferenceNodeId.Identifier.Numeric==40) // HasTypeDefinition
				pConsistentInverseNodeId->Identifier.Numeric=45; // HasSubtype
		}
	}
	else
		uStatus=OpcUa_BadInvalidArgument;
	return uStatus;
}

void InvertingReferenceThread(void* /*arg*/)
{
	CUAInformationModel* pInformationModel = CServerApplication::m_pTheAddressSpace;
	if (!pInformationModel)
	{
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critical Error>CUAInformationModel null. Contact Michel Condemine\n");
		return;
	}
	OpcUa_StatusCode uStatus = pInformationModel->InvertInverseReferences(); // UpdateAllForwardReferences();
	if ((uStatus != OpcUa_Good) && (uStatus != OpcUa_BadNothingToDo))
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Configuration inconsistency. Error during inverting of inverse references. Please check you XMLs files\n");
	// We will add an iverse reference for all forward reference declared in the nodeset
	// Those references are require for theinverse browsing of the AddressSpace
	// il s'agit des references indispensable pour le browsing inverse de l'espace d'addressage
	uStatus = pInformationModel->InvertForwardReferences();;
	if ((uStatus != OpcUa_Good) && (uStatus != OpcUa_BadNothingToDo))
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Configuration inconsistency. Error during inverting of forware references.. Please check you XMLs files\n");
	// We tell pending thread that part of the postParsing threatment is finished
	//OpcUa_Semaphore_Post(pInformationModel->m_PostThreatmentSem, 1);
	pInformationModel->SearchEventsDefinition();
	//
	CEventsEngine* pEventsEngine = g_pTheApplication->GetEventsEngine();
	if (pEventsEngine)
	{
		if (pEventsEngine->GetEventDefinitionListSize() > 0)
		{
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Will start the EventEngine...\n");
			OpcUa_Semaphore_Post(pEventsEngine->m_EventsThreadSem, 1);
		}
	}
	pInformationModel->Autorun();
	OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "End Post-Parsing initialization\n");
	//OpcUa_Thread_Delete(hInvertingReferenceThread);
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Posts the processing. </summary>
///
/// <remarks>	Michel, 29/07/2016. </remarks>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode PostProcessing()
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	//////////////////////////////////////////
	//
	// Begining Post Treatment
	//
	//////////////////////////////////////////
	OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Start Post-Parsing initialization\n");
	// This all the action made on the server before it runs
	// Some action are made synchronously others asynchronously

	CUAInformationModel* pInformationModel=CServerApplication::m_pTheAddressSpace;
	if (!pInformationModel)
	{
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Critical Error>CUAInformationModel null. Contact Michel Condemine\n");
		return 3;
	}
	uStatus=pInformationModel->UpdateNamespaceArray();

	// Add System nodes.Those nodes  are not loaded through nodeset files.
	uStatus = pInformationModel->AddinternalOpenOpcUaSystemNodes();
	// 
	// Let's update builtIn types for all UAVariables
	OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Start Updating UAVariablesBuiltinType\n");
	uStatus=pInformationModel->UpdateUAVariablesBuiltinType();
	if (uStatus!=OpcUa_Good)
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Configuration inconsistency.Error during Built-In type update Please check you XMLs files\n");
	// Mise à jour des EncodeableType de chaque extensionObject
	OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Start Updating UAVariablesEncodeableObject\n");
	uStatus=pInformationModel->UpdateUAVariablesEncodeableObject();
	if (uStatus!=OpcUa_Good)
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Update of EncodeableType failed. Please contact OpenOpcUa team\n");
	// Mise à jour des DataTypes pour chaque UAVariableType.
	// A l'initialisation le dataType de chaque UAVariable doit être ns=0;i=0
	// les dataType initialisé au travers du fichier XML ne seront pas traité
	uStatus = pInformationModel->UpdateUAVariableTypeDataType();

	// mise jour de tous les types de données pending <=> celle qui n'ont pas été initialisé lors du chargement du fichier XML
	uStatus=pInformationModel->UpdatePendingVariableDatatype();

	// Update fastAccessList (m_UAInformationModelFastAccessList) for all nodeid declare in namespace index 0   
	uStatus=pInformationModel->UpdateInformationModelFastAccessList();


	// We will forward all the inverse reference detected during the xml file parsing
	// This means that we will create a new forward reference
	OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Start Updating inverse references\n");

	// Create the thread for inversing reference
	uStatus = OpcUa_Thread_Create(&hInvertingReferenceThread, InvertingReferenceThread, OpcUa_Null);
	if (uStatus == OpcUa_Good)
		OpcUa_Thread_Start(hInvertingReferenceThread);
	
	OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "End Post-Parsing. Threads running...\n");
	//////////////////////////////////////////
	//
	// End Post Treatment
	//
	//////////////////////////////////////////
	return uStatus;
}
///////////////////////////////////////////////////////////////////////////////////////
//
// Chargement de tous les fichiers nodeset declarés dasn le fichier XML de configuration
// 
OpcUa_StatusCode LoadNodeSetFiles()
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_CharA* localPath=OpcUa_Null;
	OpcUa_CharA* fileName=OpcUa_Null;
	// Chargement des fichiers NodeSet qui constitueront l'espace d'adressage
	for (OpcUa_UInt32 ii=0;ii<g_pTheApplication->m_FilesNodeSet.size();ii++)
	{
		OpcUa_String* pFullFileName=(OpcUa_String*)g_pTheApplication->m_FilesNodeSet.at(ii);
		if (pFullFileName)
		{
			OpcUa_CharA* chFileName = OpcUa_String_GetRawString(pFullFileName);
			// séparation du path et du nom de fichier

			basic_string<char> myString(chFileName);
			basic_string<char>::size_type index = 0;
#ifdef _GNUC_
			index=myString.rfind("/");
#else
#ifdef WIN32
			index = myString.rfind("\\");
#endif
#endif

			if (index != basic_string<char>::npos)
			{
				// path
				basic_string<char> tmpStr = myString.substr(0, index + 1);

				localPath = (char*)OpcUa_Alloc(tmpStr.size() + 1);
				if (localPath)
				{
					ZeroMemory(localPath, tmpStr.size() + 1);
					memcpy(localPath, tmpStr.c_str(), tmpStr.size());
					// fileName
					tmpStr = myString.substr(index + 1, myString.size() - index);

					fileName = (char*)OpcUa_Alloc(tmpStr.size() + 1);
					if (fileName)
					{
						ZeroMemory(fileName, tmpStr.size() + 1);
						memcpy(fileName, tmpStr.c_str(), tmpStr.size());

						if ((localPath) && (fileName))
						{
							// Parsing des fichiers de configuration de l'espace d'adressage
							uStatus = g_pTheApplication->LoadUaServerNodeSet(localPath, fileName);
							if (uStatus != OpcUa_Good)
								OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critical error>Cannot load NodeSet file\n");
						}
						else
							OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critical error>Memory error,NodeSetName corrupted\n");
						if (localPath)
							OpcUa_Free(localPath);
						if (fileName)
							OpcUa_Free(fileName);
					}
				}
			}
			else
			{
				uStatus = OpcUa_BadFileNotFound;
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critical error>Full NodeSet filename is corrupted\n");
			}
		}
		else
		{
			uStatus = OpcUa_BadInvalidArgument;
			break;
		}
	}

	// fin Chargement des fichiers NodeSet qui constitueront l'espace d'adressage

	return uStatus;
}
// Mise en place de L'application Description et des informations sur les ports d'écoute
OpcUa_StatusCode SetServerDescriptionTransportPortListener(CApplicationDescription** pAppDescription)
{
	OpcUa_StatusCode uStatus=OpcUa_BadInvalidArgument;
	//CApplicationDescriptionList* pAppDescriptionList= m_theApplication.GetApplicationDescriptions();
	
	CUABindings aUABindings=g_pTheApplication->GetServerBindings();
	for (OpcUa_UInt16 iii=0;iii<aUABindings.size();iii++)
	{
		CUABinding* pBinding=aUABindings.at(iii);
		if (pBinding)
		{
			uStatus=OpcUa_Good;
			OpcUa_ApplicationDescription* pUaApplicationDescription=(OpcUa_ApplicationDescription*)OpcUa_Alloc(sizeof(OpcUa_ApplicationDescription));
			OpcUa_ApplicationDescription_Initialize(pUaApplicationDescription);
			OpcUa_LocalizedText* pAppName= g_pTheApplication->GetApplicationName();
			if (pAppName)
			{
				OpcUa_LocalizedText_Initialize(&(pUaApplicationDescription->ApplicationName));
				OpcUa_LocalizedText_CopyTo(pAppName,&(pUaApplicationDescription->ApplicationName));
			}
			pUaApplicationDescription->ApplicationType=OpcUa_ApplicationType_Server;
			// ApplicationUri
			// creation de l'applicationURI a partir du nom de l'application
			std::string applicationUri;
			if (pAppName)
				uStatus=OpcUa_CreateApplication_Uri(OpcUa_String_GetRawString(&(pAppName->Text)),&applicationUri);
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_WARNING, "ApplicationUri=%s\n", applicationUri.c_str());
			OpcUa_String_AttachCopy(&(pUaApplicationDescription->ApplicationUri),applicationUri.c_str());
			// ProductUri
			OpcUa_String strBinding=pBinding->AsString();
			OpcUa_String_AttachCopy(&(pUaApplicationDescription->ProductUri),applicationUri.c_str());

			pUaApplicationDescription->NoOfDiscoveryUrls=1;
			pUaApplicationDescription->DiscoveryUrls=(OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
			OpcUa_String_Initialize(&(pUaApplicationDescription->DiscoveryUrls[0]));
			OpcUa_String_StrnCpy(&(pUaApplicationDescription->DiscoveryUrls[0]),
				&strBinding,
				OpcUa_String_StrLen(&strBinding));
			OpcUa_String_Initialize(&(pUaApplicationDescription->DiscoveryProfileUri));
			OpcUa_String_Initialize(&(pUaApplicationDescription->GatewayServerUri));

			(*pAppDescription)=new CApplicationDescription(pUaApplicationDescription);
			OpcUa_ApplicationDescription_Clear(pUaApplicationDescription);
			OpcUa_Free(pUaApplicationDescription);
		}
	}
	return uStatus;
}
OpcUa_StatusCode LoadSimulationFiles()
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_CharA* localPath=OpcUa_Null;
	OpcUa_CharA* fileName=OpcUa_Null;
	// Chargement des paramètres de simulation
	for (OpcUa_UInt16 iii=0;iii<g_pTheApplication->m_FilesSimulation.size();iii++)
	{
		OpcUa_String* pFullFileName=g_pTheApplication->m_FilesSimulation.at(iii);
		OpcUa_CharA* chFileName=OpcUa_String_GetRawString(pFullFileName);
		// séparation du path et du nom de fichier
		
		basic_string<char> myString(chFileName);
		basic_string<char>::size_type index=0;
	#ifdef _GNUC_
		index=myString.rfind("/");
	#else
		#ifdef WIN32
			index=myString.rfind("\\");
		#endif
	#endif
		if (index!=basic_string<char>::npos)
		{
			// path
			basic_string<char> tmpStr=myString.substr(0,index+1);

			localPath=(char*)OpcUa_Alloc(tmpStr.size()+1);
			if (localPath)
			{
				ZeroMemory(localPath, tmpStr.size() + 1);
				memcpy(localPath, tmpStr.c_str(), tmpStr.size());
				// fileName
				tmpStr = myString.substr(index + 1, myString.size() - index);
				fileName = (char*)OpcUa_Alloc(tmpStr.size() + 1);
				if (fileName)
				{
					ZeroMemory(fileName, tmpStr.size() + 1);
					memcpy(fileName, tmpStr.c_str(), tmpStr.size());

					// Parsing des fichiers de configuration de l'espace d'adressage
					uStatus = g_pTheApplication->LoadUaServerSimulation(localPath, fileName);
					if (uStatus != OpcUa_Good)
						OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critical error>Cannot load Server Simulation file %s %s\n", localPath, fileName);
					if (localPath)
						OpcUa_Free(localPath);
					if (fileName)
						OpcUa_Free(fileName);
				}
			}
		}
		else
		{
			uStatus=OpcUa_BadFileNotFound;
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Critical error>Simulation filename is corrupted\n");
		}
	}

	// fin Chargement des paramètres de simulation
	return uStatus;
}
void LoadSubsystemsThread(LPVOID arg)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_CharA* localPath = OpcUa_Null;
	OpcUa_CharA* fileName = OpcUa_Null;
	OpcUa_String* pFullFileName =(OpcUa_String*)arg;
	OpcUa_CharA* chFileName = OpcUa_String_GetRawString(pFullFileName);
	// séparation du path et du nom de fichier

	basic_string<char> myString(chFileName);
	basic_string<char>::size_type index = 0;
#ifdef _GNUC_
	index = myString.rfind("/");
#else
#ifdef WIN32
	index = myString.rfind("\\");
#endif
#endif
	if (index != basic_string<char>::npos)
	{
		// path
		basic_string<char> tmpStr = myString.substr(0, index + 1);
		
		OpcUa_Mutex_Lock(g_pTheApplication->m_pTheAddressSpace->m_ServerCacheMutex);
		localPath = (char*)OpcUa_Alloc(tmpStr.size() + 1);
		if (localPath)
		{
			ZeroMemory(localPath, tmpStr.size() + 1);
			memcpy(localPath, tmpStr.c_str(), tmpStr.size());
			// fileName
			tmpStr = myString.substr(index + 1, myString.size() - index);
			fileName = (char*)OpcUa_Alloc(tmpStr.size() + 1);
			if (fileName)
			{
				ZeroMemory(fileName, tmpStr.size() + 1);
				memcpy(fileName, tmpStr.c_str(), tmpStr.size());

				// Parsing des fichiers de configuration de l'espace d'adressage
				uStatus = g_pTheApplication->LoadUaServerSubsystems(localPath, fileName);
				if (uStatus != OpcUa_Good)
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critical error>Cannot load Server Subsystem file%s %s\n", localPath, fileName);
				if (localPath)
					OpcUa_Free(localPath);
				if (fileName)
					OpcUa_Free(fileName);
				// démarrage du mecanisme de dialogue avec les sous-systèmes
				g_pTheApplication->WakeupAllVpi();
			}
			else
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critical Error. Not enough memory");
		}
		else
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critical Error. Not enough memory");
		OpcUa_Mutex_Unlock(g_pTheApplication->m_pTheAddressSpace->m_ServerCacheMutex);
	}
	else
	{
		uStatus = OpcUa_BadFileNotFound;
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critical error>Subsystem filename is corrupted\n");
	}
}
OpcUa_StatusCode LoadSubSystemFiles()
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	//OpcUa_CharA* localPath=OpcUa_Null;
	//OpcUa_CharA* fileName=OpcUa_Null;
	for (OpcUa_UInt16 iii=0;iii<g_pTheApplication->m_FilesSubsystem.size();iii++)
	{
		OpcUa_String* pFullFileName=g_pTheApplication->m_FilesSubsystem.at(iii);
		if (pFullFileName)
		{
			OpcUa_Thread hThread;
			if (OpcUa_Thread_Create(&hThread, LoadSubsystemsThread, pFullFileName) == OpcUa_Good)
				OpcUa_Thread_Start(hThread);
			else
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critical error>Cannot load Server Subsystem file: %s %s\n", OpcUa_String_GetRawString(pFullFileName));
		}
		/*
		OpcUa_CharA* chFileName=OpcUa_String_GetRawString(pFullFileName);
		// séparation du path et du nom de fichier
		
		basic_string<char> myString(chFileName);
		basic_string<char>::size_type index=0;
#ifdef _GNUC_
		index=myString.rfind("/");
#else
#ifdef WIN32
		index=myString.rfind("\\");
#endif
#endif
		if (index!=basic_string<char>::npos)
		{
			// path
			basic_string<char> tmpStr=myString.substr(0,index+1);

			localPath=(char*)OpcUa_Alloc(tmpStr.size()+1);
			if (localPath)
			{
				ZeroMemory(localPath, tmpStr.size() + 1);
				memcpy(localPath, tmpStr.c_str(), tmpStr.size());
				// fileName
				tmpStr = myString.substr(index + 1, myString.size() - index);
				fileName = (char*)OpcUa_Alloc(tmpStr.size() + 1);
				if (fileName)
				{
					ZeroMemory(fileName, tmpStr.size() + 1);
					memcpy(fileName, tmpStr.c_str(), tmpStr.size());

					// Parsing des fichiers de configuration de l'espace d'adressage
					uStatus = g_pTheApplication->LoadUaServerSubsystems(localPath, fileName);
					if (uStatus != OpcUa_Good)
						OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critical error>Cannot load Server Subsystem file%s %s\n", localPath, fileName);
					if (localPath)
						OpcUa_Free(localPath);
					if (fileName)
						OpcUa_Free(fileName);
				}
				else
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critical Error. Not enough memory");
			}
			else
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critical Error. Not enough memory");
		}
		else
		{
			uStatus=OpcUa_BadFileNotFound;
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Critical error>Subsystem filename is corrupted\n");
		}
		*/
	}
	// Fin Chargement de la configuration des subsystems
	return uStatus;
}


// Extract the OpcData_Value from pDataValue1 and send in the pDataValue2 based on the pRange 
OpcUa_StatusCode AdjustArrayToRange(OpcUa_DataValue* pDataValue1, CNumericRange* pNumericRange, OpcUa_DataValue** pDataValue2)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	if (pDataValue1)
	{
		if (pDataValue1->Value.ArrayType==OpcUa_VariantArrayType_Array)
		{
			if (pNumericRange)
			{
				// Allocate output
				(*pDataValue2)=(OpcUa_DataValue*)OpcUa_Alloc(sizeof(OpcUa_DataValue));
				if ((*pDataValue2))
					OpcUa_DataValue_Initialize((*pDataValue2));
				//OpcUa_DataValue* pValue1=pDataValue1;
				if (pNumericRange->IsMultiDimensional())
				{
					OpcUa_UInt32 iNoOfSubRanges =pNumericRange->GetNoOfSubRanges() ;
					if (iNoOfSubRanges<=2)
					{
						if (iNoOfSubRanges==2)
						{
							CDataValue* pTmpDataValue2=new CDataValue();
							if (pTmpDataValue2)
							{
								// it can be only ByteString and String
								// SubRange pour la première dimension
								CNumericRange* pSubRange00=pNumericRange->GetRangeAt(0);
								// SubRange pour la deuxième dimension
								CNumericRange* pSubRange01=pNumericRange->GetRangeAt(1);
								if (pSubRange00&&pSubRange01)
								{
									OpcUa_Int32 iFirstDimLen=pSubRange00->GetEndIndex()-pSubRange00->GetBeginIndex()+1;
									uStatus=pTmpDataValue2->InitializeArray(pDataValue1->Value.Datatype,iFirstDimLen);
									if (uStatus==OpcUa_Good)
									{
										OpcUa_DataValue* pValue2=pTmpDataValue2->GetInternalDataValue();
										switch (pDataValue1->Value.Datatype)
										{
										case OpcUaType_ByteString:	
											{
												OpcUa_UInt32 j=0;
												// Extraction pour la première dimension (Ligne)
												for (OpcUa_Int32 i=pSubRange00->GetBeginIndex();i<=pSubRange00->GetEndIndex();i++)
												{
													// On extrait le byteString pour la première dimension
													OpcUa_ByteString aByteString=pDataValue1->Value.Value.Array.Value.ByteStringArray[i];
													if (aByteString.Data)
													{
														// On applique les second subRange sur le byteString Extrait 
														OpcUa_ByteString aSubByteString;
														OpcUa_ByteString_Initialize(&aSubByteString);
														OpcUa_Int32 iOffset = pSubRange01->GetEndIndex() - pSubRange01->GetBeginIndex() + 1;
														aSubByteString.Data = (OpcUa_Byte*)OpcUa_Alloc(sizeof(OpcUa_Byte)*iOffset);
														// Mise en form d'un byteString Resultat
														OpcUa_Int32 ii = 0;
														for (OpcUa_Int32 iSubIndex = pSubRange01->GetBeginIndex(); iSubIndex <= pSubRange01->GetEndIndex(); iSubIndex++)
															aSubByteString.Data[ii++] = aByteString.Data[iSubIndex];
														aSubByteString.Length = iOffset;
														// Transfert du resultat dans la byteString
														pValue2->Value.ArrayType = OpcUa_VariantArrayType_Array;
														pValue2->Value.Datatype = OpcUaType_ByteString;
														OpcUa_ByteString_CopyTo(&aSubByteString, &(pValue2->Value.Value.Array.Value.ByteStringArray[j++]));
														OpcUa_ByteString_Clear(&aSubByteString);
													}
												}
												OpcUa_DataValue_CopyTo(pValue2, (*pDataValue2));
												delete pTmpDataValue2;
												pTmpDataValue2 = OpcUa_Null;
												//(*pDataValue2)=pValue2;
											}
											break;
										case OpcUaType_String:	
											{
												OpcUa_UInt32 j=0;
												// Extraction pour la première dimension (Ligne)
												for (OpcUa_Int32 i=pSubRange00->GetBeginIndex();i<=pSubRange00->GetEndIndex();i++)
												{
													// On extrait le byteString pour la première dimension
													OpcUa_String aString=pDataValue1->Value.Value.Array.Value.StringArray[i];
													// On applique les second subRange sur le byteString Extrait 
													OpcUa_String aSubString;
													OpcUa_String_Initialize(&aSubString);
													OpcUa_Int32 iOffset=pSubRange01->GetEndIndex()-pSubRange01->GetBeginIndex()+1;
													OpcUa_CharA* pBuf=(OpcUa_CharA*)OpcUa_Alloc(sizeof(OpcUa_Byte)*iOffset+1);
													OpcUa_CharA* aRawString=OpcUa_String_GetRawString(&aString);
													ZeroMemory(pBuf,iOffset+1);
													// Mise en form d'un byteString Resultat
													OpcUa_Int32 ii=0;
													for (OpcUa_Int32 iSubIndex=pSubRange01->GetBeginIndex();iSubIndex<=pSubRange01->GetEndIndex();iSubIndex++)
														pBuf[ii++]=aRawString[iSubIndex];
													// Transfert du resultat dans la byteString
													pValue2->Value.ArrayType=OpcUa_VariantArrayType_Array;
													pValue2->Value.Datatype=OpcUaType_String;
													OpcUa_String_AttachCopy(&aSubString,pBuf);
													OpcUa_String_CopyTo(&aSubString,&(pValue2->Value.Value.Array.Value.StringArray[j++]));
													OpcUa_Free(pBuf);
													OpcUa_String_Clear(&aSubString);
												}
												OpcUa_DataValue_CopyTo(pValue2, (*pDataValue2));										
												delete pTmpDataValue2;
												pTmpDataValue2 = OpcUa_Null;
												//(*pDataValue2)=pValue2;
											}
											break;
										default:
											uStatus = OpcUa_BadIndexRangeNoData; //  OpcUa_BadInvalidArgument; 1.0.4.0
											delete pTmpDataValue2;
											pTmpDataValue2 = OpcUa_Null;
											break;
										}
									}
								}
								else
								{
									delete pTmpDataValue2;
									pTmpDataValue2 = OpcUa_Null;
									uStatus = OpcUa_BadInvalidArgument;
								}
							}
							else
								uStatus=OpcUa_BadOutOfMemory;

						}
						else
							uStatus=OpcUa_BadInvalidArgument;
					}
					else
						uStatus=OpcUa_BadNotSupported;
				}
				else
				{
					if (pNumericRange->IsUnique())
					{
						CDataValue* pTmpDataValue2=new CDataValue();
						if (pTmpDataValue2)
						{
							uStatus=pTmpDataValue2->InitializeArray(pDataValue1->Value.Datatype,1);
							
							if (uStatus==OpcUa_Good)
							{
								OpcUa_UInt32 uniqueIndex = pNumericRange->GetEndIndex();
								OpcUa_DataValue* pValue2=pTmpDataValue2->GetInternalDataValue();
								switch (pDataValue1->Value.Datatype)
								{
								case OpcUaType_Boolean:
									pValue2->Value.Value.Array.Value.BooleanArray[0] = pDataValue1->Value.Value.Array.Value.BooleanArray[uniqueIndex];
									OpcUa_DataValue_CopyTo(pValue2, (*pDataValue2));
									break;
								case OpcUaType_Byte:
									pValue2->Value.Value.Array.Value.ByteArray[0] = pDataValue1->Value.Value.Array.Value.ByteArray[uniqueIndex];
									OpcUa_DataValue_CopyTo(pValue2, (*pDataValue2));
									break;
								case OpcUaType_SByte:
									pValue2->Value.Value.Array.Value.SByteArray[0] = pDataValue1->Value.Value.Array.Value.SByteArray[uniqueIndex];
									OpcUa_DataValue_CopyTo(pValue2, (*pDataValue2));
									break;
								case OpcUaType_DateTime:
									pValue2->Value.Value.Array.Value.DateTimeArray[0] = pDataValue1->Value.Value.Array.Value.DateTimeArray[uniqueIndex];
									OpcUa_DataValue_CopyTo(pValue2, (*pDataValue2));
									break;
								case OpcUaType_Double:
									pValue2->Value.Value.Array.Value.DoubleArray[0] = pDataValue1->Value.Value.Array.Value.DoubleArray[uniqueIndex];
									OpcUa_DataValue_CopyTo(pValue2, (*pDataValue2));
									break;
								case OpcUaType_Float:
									pValue2->Value.Value.Array.Value.FloatArray[0] = pDataValue1->Value.Value.Array.Value.FloatArray[uniqueIndex];
									OpcUa_DataValue_CopyTo(pValue2, (*pDataValue2));
									break;
								case OpcUaType_Int16:
									pValue2->Value.Value.Array.Value.Int16Array[0] = pDataValue1->Value.Value.Array.Value.Int16Array[uniqueIndex];
									OpcUa_DataValue_CopyTo(pValue2, (*pDataValue2));
									break;
								case OpcUaType_Int32:
									pValue2->Value.Value.Array.Value.Int32Array[0] = pDataValue1->Value.Value.Array.Value.Int32Array[uniqueIndex];
									OpcUa_DataValue_CopyTo(pValue2, (*pDataValue2));
									break;
								case OpcUaType_Int64:
									pValue2->Value.Value.Array.Value.Int64Array[0] = pDataValue1->Value.Value.Array.Value.Int64Array[uniqueIndex];
									OpcUa_DataValue_CopyTo(pValue2, (*pDataValue2));
									break;
								case OpcUaType_UInt16:
									pValue2->Value.Value.Array.Value.UInt16Array[0] = pDataValue1->Value.Value.Array.Value.UInt16Array[uniqueIndex];
									OpcUa_DataValue_CopyTo(pValue2, (*pDataValue2));
									break;
								case OpcUaType_UInt32:
									pValue2->Value.Value.Array.Value.UInt32Array[0] = pDataValue1->Value.Value.Array.Value.UInt32Array[uniqueIndex];
									OpcUa_DataValue_CopyTo(pValue2, (*pDataValue2));
									break;
								case OpcUaType_UInt64:
									pValue2->Value.Value.Array.Value.UInt64Array[0] = pDataValue1->Value.Value.Array.Value.UInt64Array[uniqueIndex];
									OpcUa_DataValue_CopyTo(pValue2, (*pDataValue2));
									break;
								case OpcUaType_StatusCode:
									pValue2->Value.Value.Array.Value.StatusCodeArray[0] = pDataValue1->Value.Value.Array.Value.StatusCodeArray[uniqueIndex];
									OpcUa_DataValue_CopyTo(pValue2, (*pDataValue2));
									break;
								case OpcUaType_Guid:
									OpcUa_Guid_CopyTo(&(pDataValue1->Value.Value.Array.Value.GuidArray[uniqueIndex]), &(pValue2->Value.Value.Array.Value.GuidArray[0]));
									OpcUa_DataValue_CopyTo(pValue2, (*pDataValue2));
									break;
								case OpcUaType_QualifiedName:
									OpcUa_QualifiedName_CopyTo(&(pDataValue1->Value.Value.Array.Value.QualifiedNameArray[uniqueIndex]), &(pValue2->Value.Value.Array.Value.QualifiedNameArray[0]));
									OpcUa_DataValue_CopyTo(pValue2, (*pDataValue2));
									break;
								case OpcUaType_LocalizedText:
									OpcUa_LocalizedText_CopyTo(&(pDataValue1->Value.Value.Array.Value.LocalizedTextArray[uniqueIndex]), &(pValue2->Value.Value.Array.Value.LocalizedTextArray[0]));
									OpcUa_DataValue_CopyTo(pValue2, (*pDataValue2));
									break;
								case OpcUaType_String:
									OpcUa_String_CopyTo(&(pDataValue1->Value.Value.Array.Value.StringArray[uniqueIndex]), &(pValue2->Value.Value.Array.Value.StringArray[0]));
									OpcUa_DataValue_CopyTo(pValue2, (*pDataValue2));
									break;
								default:
									uStatus = OpcUa_BadNotSupported;
									if (*pDataValue2)
									{
										OpcUa_DataValue_Initialize((*pDataValue2));
										OpcUa_Free((*pDataValue2));
										(*pDataValue2) = OpcUa_Null;
									}
									break;
								}
							}
							else
							{
								if (*pDataValue2)
								{
									OpcUa_DataValue_Initialize((*pDataValue2));
									OpcUa_Free((*pDataValue2));
									(*pDataValue2) = OpcUa_Null;
								}
							}
							// release resources
							delete pTmpDataValue2;
							pTmpDataValue2 = OpcUa_Null;
						}
						else
						{
							if (*pDataValue2)
							{
								OpcUa_DataValue_Initialize((*pDataValue2));
								OpcUa_Free((*pDataValue2));
								(*pDataValue2) = OpcUa_Null;
							}
						}
					}
					else
					{
						OpcUa_Int32 iArrayLen=(pNumericRange->GetEndIndex()-pNumericRange->GetBeginIndex())+1;
						CDataValue* pTmpDataValue2=new CDataValue();
						if (pTmpDataValue2)
						{
							uStatus=pTmpDataValue2->InitializeArray(pDataValue1->Value.Datatype,iArrayLen);
							if (uStatus==OpcUa_Good)
							{
								OpcUa_DataValue* pValue1=pDataValue1;
								OpcUa_DataValue* pValue2=pTmpDataValue2->GetInternalDataValue();
								OpcUa_Int32 ii=0;
								for (OpcUa_Int32 i=pNumericRange->GetBeginIndex();i<=pNumericRange->GetEndIndex();i++)
								{
									if (pNumericRange->GetBeginIndex()<=pValue1->Value.Value.Array.Length)
									{
										switch (pValue1->Value.Datatype)
										{
										case OpcUaType_Boolean:
											pValue2->Value.Value.Array.Value.BooleanArray[ii] = pValue1->Value.Value.Array.Value.BooleanArray[i];
											break;
										case OpcUaType_Byte:
											pValue2->Value.Value.Array.Value.ByteArray[ii] = pValue1->Value.Value.Array.Value.ByteArray[i];
											break;
										case OpcUaType_ByteString:	
											OpcUa_ByteString_Initialize(&(pValue2->Value.Value.Array.Value.ByteStringArray[ii]));
											OpcUa_ByteString_CopyTo(&(pValue1->Value.Value.Array.Value.ByteStringArray[i]),&(pValue2->Value.Value.Array.Value.ByteStringArray[ii]));
											break;
										case OpcUaType_DateTime:
											pValue2->Value.Value.Array.Value.DateTimeArray[ii].dwHighDateTime =pValue1->Value.Value.Array.Value.DateTimeArray[i].dwHighDateTime;
											pValue2->Value.Value.Array.Value.DateTimeArray[ii].dwLowDateTime =pValue1->Value.Value.Array.Value.DateTimeArray[i].dwLowDateTime;
											break;
										case OpcUaType_Double:
											pValue2->Value.Value.Array.Value.DoubleArray[ii] = pValue1->Value.Value.Array.Value.DoubleArray[i];
											break;	
										case OpcUaType_Float:
											pValue2->Value.Value.Array.Value.FloatArray[ii] = pValue1->Value.Value.Array.Value.FloatArray[i];
											break;
										case OpcUaType_Guid:												
											OpcUa_Guid_CopyTo(&(pValue1->Value.Value.Array.Value.GuidArray[i]),&(pValue2->Value.Value.Array.Value.GuidArray[ii]));
											break;
										case OpcUaType_Int16:
											pValue2->Value.Value.Array.Value.Int16Array[ii] = pValue1->Value.Value.Array.Value.Int16Array[i];
											break;
										case OpcUaType_Int32:
											pValue2->Value.Value.Array.Value.Int32Array[ii] = pValue1->Value.Value.Array.Value.Int32Array[i];
											break;
										case OpcUaType_Int64:
											pValue2->Value.Value.Array.Value.Int64Array[ii] = pValue1->Value.Value.Array.Value.Int64Array[i];
											break;
										case OpcUaType_LocalizedText:
											OpcUa_LocalizedText_CopyTo(&(pValue1->Value.Value.Array.Value.LocalizedTextArray[i]),&(pValue2->Value.Value.Array.Value.LocalizedTextArray[ii]));
											break;
										case OpcUaType_NodeId:
											OpcUa_NodeId_CopyTo(&(pValue1->Value.Value.Array.Value.NodeIdArray[i]),&(pValue2->Value.Value.Array.Value.NodeIdArray[ii]));
											break;
										case OpcUaType_QualifiedName:
											OpcUa_QualifiedName_CopyTo(&(pValue1->Value.Value.Array.Value.QualifiedNameArray[i]),&(pValue2->Value.Value.Array.Value.QualifiedNameArray[ii]));
											break;
										case OpcUaType_SByte:
											pValue2->Value.Value.Array.Value.SByteArray[ii] = pValue1->Value.Value.Array.Value.SByteArray[i];
											break;
										case OpcUaType_StatusCode:
											pValue2->Value.Value.Array.Value.StatusCodeArray[ii] = pValue1->Value.Value.Array.Value.StatusCodeArray[i];
											break;
										case OpcUaType_String:
											OpcUa_String_Initialize(&(pValue2->Value.Value.Array.Value.StringArray[ii]));
											OpcUa_String_CopyTo(&(pValue1->Value.Value.Array.Value.StringArray[i]),&(pValue2->Value.Value.Array.Value.StringArray[ii]));
											break;
										case OpcUaType_UInt16:
											pValue2->Value.Value.Array.Value.UInt16Array[ii] = pValue1->Value.Value.Array.Value.UInt16Array[i];
											break;
										case OpcUaType_UInt32:
											pValue2->Value.Value.Array.Value.UInt32Array[ii] = pValue1->Value.Value.Array.Value.UInt32Array[i];
											break;
										case OpcUaType_UInt64:
											pValue2->Value.Value.Array.Value.UInt64Array[ii] = pValue1->Value.Value.Array.Value.UInt64Array[i];
											break;
										default:
											break;
										}
									}
									ii++;
								}
								OpcUa_DataValue_CopyTo(pValue2, (*pDataValue2));
							}
							else
							{
								if (*pDataValue2)
								{
									OpcUa_DataValue_Initialize((*pDataValue2));
									OpcUa_Free((*pDataValue2));
									(*pDataValue2) = OpcUa_Null;
								}
							}
							// release resources
							delete pTmpDataValue2;
							pTmpDataValue2 = OpcUa_Null;
						}
						else
						{
							if (*pDataValue2)
							{
								OpcUa_DataValue_Initialize((*pDataValue2));
								OpcUa_Free((*pDataValue2));
								(*pDataValue2) = OpcUa_Null;
							}
						}
					}
				}
			}
			else
				uStatus=OpcUa_BadInvalidArgument;
		}
		else
			uStatus=OpcUa_BadInvalidArgument;
	}
	else
		uStatus=OpcUa_BadInvalidArgument;
	return uStatus;
}
// Generic Extension object copy function
OpcUa_StatusCode Copy(OpcUa_ExtensionObject** pTargetExtensionObject,OpcUa_ExtensionObject* pSourceExtensionObject)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	if (pSourceExtensionObject)
	{
		switch (pSourceExtensionObject->Encoding)
		{
		case OpcUa_ExtensionObjectEncoding_None:
			break;
		case OpcUa_ExtensionObjectEncoding_Binary:
		case OpcUa_ExtensionObjectEncoding_Xml:
			OpcUa_ExtensionObject_Initialize(*pTargetExtensionObject);
			OpcUa_ExtensionObject_CopyTo(pSourceExtensionObject,*pTargetExtensionObject);
			break;
		case OpcUa_ExtensionObjectEncoding_EncodeableObject:
			{
				CUAInformationModel* pInformationModel=CServerApplication::m_pTheAddressSpace;
				OpcUa_Int32 iNamespaceIndex=0;
				OpcUa_NodeId aNodeId;
				OpcUa_NodeId_Initialize(&aNodeId);
				aNodeId.IdentifierType=OpcUa_IdentifierType_Numeric;
				OpcUa_String aString;
				OpcUa_String_Initialize(&aString);
				if (pSourceExtensionObject)
				{
					if (pSourceExtensionObject->Body.EncodeableObject.Type)
					{
						if (pSourceExtensionObject->Body.EncodeableObject.Type->NamespaceUri)
						{
							OpcUa_String_AttachCopy(&aString, pSourceExtensionObject->Body.EncodeableObject.Type->NamespaceUri);
							if (pInformationModel->GetNamespaceUri(aString, &iNamespaceIndex) != OpcUa_Good)
								iNamespaceIndex = 0;
						}
						aNodeId.NamespaceIndex = (OpcUa_UInt16)iNamespaceIndex;
						aNodeId.Identifier.Numeric = pSourceExtensionObject->Body.EncodeableObject.Type->TypeId;
						CUADataType* pUADataType=OpcUa_Null;
						// Look for the dataType
						uStatus=pInformationModel->GetNodeIdFromDataTypeList(aNodeId,&pUADataType);
						if (uStatus==OpcUa_Good)
						{
							// Get the definition for this DataType
							CDefinition* pDefinition=pUADataType->GetDefinition();
							if (pDefinition)
							{
								OpcUa_Int32 iInstanceComputedSize = -1;
								uStatus = pDefinition->GetInstanceSize(&iInstanceComputedSize);
								uStatus = pDefinition->DuplicateExtensionObject(pSourceExtensionObject, pTargetExtensionObject);
							}
						}
					}
					else
						uStatus = OpcUa_BadInvalidArgument;
				}
				else
					uStatus = OpcUa_BadInvalidArgument;
			}
			break;
		}
	}
	else
		uStatus=OpcUa_BadInvalidArgument;
	return uStatus;
}

/// <summary>
/// Copy from pVoidBufSource to pVoidBufTarget considering that pVoidBufSource contains a "builtInType" with a size of "ifieldSize".
/// ifieldSize is used only for "NormalType". "SpecialType" are type that contains a sting (something dynamic)
/// </summary>
/// <param name="ifieldSize">Size of the ifield.</param>
/// <param name="builtInType">Type of the built in.</param>
/// <param name="pVoidBufSource">The p void buf source.</param>
/// <param name="pVoidBufTarget">The p void buf target.</param>
/// <returns>OpcUa_StatusCode OpcUa_Good when no error are detected</returns>
OpcUa_StatusCode CopyBuiltInType(const OpcUa_Int32 ifieldSize, const OpcUa_Byte builtInType, void** pVoidBufSource, void** pVoidBufTarget)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	switch (builtInType)
	{
	case OpcUaType_ExtensionObject:
	{
	}
		break;
	case OpcUaType_ByteString:
	{
		// Initialize the byteString
		OpcUa_ByteString* pByteString = (OpcUa_ByteString*)OpcUa_Alloc(sizeof(OpcUa_ByteString));
		OpcUa_ByteString_Initialize(pByteString);
		// 
		// Let's get the size of the byteString
		OpcUa_MemCpy(&(pByteString->Length), 4, *pVoidBufSource, 4);
		((OpcUa_Byte*&)*pVoidBufTarget) += 4;
		((OpcUa_Byte*&)*pVoidBufSource) += 4;
		// Let's now trnsfert the content
		pByteString->Data = (OpcUa_Byte*)OpcUa_Alloc(pByteString->Length);
		ZeroMemory(pByteString->Data, pByteString->Length);
		OpcUa_MemCpy(&(pByteString->Data), pByteString->Length, *pVoidBufSource, pByteString->Length);
		// Ajust pointers
		((OpcUa_Byte*&)*pVoidBufTarget) += pByteString->Length; //
		((OpcUa_Byte*&)*pVoidBufSource) += pByteString->Length;
	}
		break;
	case OpcUaType_DataValue:
		break;
	case OpcUaType_LocalizedText:
	{
		CopyBuiltInType(0, OpcUaType_String, pVoidBufSource, pVoidBufTarget);
		CopyBuiltInType(0, OpcUaType_String, pVoidBufSource, pVoidBufTarget);
		//		OpcUa_LocalizedText* pLocalizedText = (OpcUa_LocalizedText*)OpcUa_Alloc(sizeof(OpcUa_LocalizedText));
		//		OpcUa_LocalizedText_Initialize(pLocalizedText);
		//#ifdef _DEBUG						
		//		///////////////////////////////////////////////	
		//		// Copy Locale pLocalizedText->Locale
		//		// copy flag
		//		OpcUa_MemCpy(pVoidBufTarget, 4, *pVoidBufSource, 4);
		//		((OpcUa_Byte*&)pVoidBufTarget) += 4; // 
		//		OpcUa_MemCpy(&(pLocalizedText->Locale.flags), 2, *pVoidBufSource, 2);
		//		((OpcUa_Byte*&)*pVoidBufSource) += 4; // The real size is 4 because a OpcUa_String is in fact an OpcUa_StringInternal
		//		// copy Length
		//		OpcUa_MemCpy(pVoidBufTarget, 4, *pVoidBufSource, 4);
		//		((OpcUa_Byte*&)pVoidBufTarget) += 4; //
		//		OpcUa_MemCpy(&(pLocalizedText->Locale.uLength), 4, *pVoidBufSource, 4);
		//		pLocalizedText->Locale.strContent = (OpcUa_CharA*)OpcUa_Alloc(pLocalizedText->Locale.uLength + 1);
		//		ZeroMemory(pLocalizedText->Locale.strContent, pLocalizedText->Locale.uLength + 1);
		//		// Copy the content
		//		((OpcUa_Byte*&)*pVoidBufSource) += 4;
		//		void* apVoid = OpcUa_Alloc(4);
		//		memcpy(&apVoid, *pVoidBufSource, 4);
		//		OpcUa_MemCpy((pLocalizedText->Locale.strContent), pLocalizedText->Locale.uLength, ((void*)(apVoid)), pLocalizedText->Locale.uLength);
		//		OpcUa_MemCpy(pVoidBufTarget, 4, &(pLocalizedText->Locale.strContent), 4);
		//		// Update the pointer
		//		((OpcUa_Byte*&)pVoidBufTarget) += 4; //
		//		((OpcUa_Byte*&)*pVoidBufSource) += 4;
		//		///////////////////////////////////////////////
		//		// Copy Locale pLocalizedText->Text
		//		// copy flag
		//		OpcUa_MemCpy(pVoidBufTarget, 4, *pVoidBufSource, 4);
		//		((OpcUa_Byte*&)pVoidBufTarget) += 4; // 
		//		OpcUa_MemCpy(&(pLocalizedText->Text.flags), 2, *pVoidBufSource, 2);
		//		((OpcUa_Byte*&)*pVoidBufSource) += 4; // The real size is 4 because a OpcUa_String is in fact an OpcUa_StringInternal
		//		// copy Length
		//		OpcUa_MemCpy(pVoidBufTarget, 4, *pVoidBufSource, 4);
		//		((OpcUa_Byte*&)pVoidBufTarget) += 4; //
		//		OpcUa_MemCpy(&(pLocalizedText->Text.uLength), 4, *pVoidBufSource, 4);
		//		pLocalizedText->Text.strContent = (OpcUa_CharA*)OpcUa_Alloc(pLocalizedText->Text.uLength + 1);
		//		ZeroMemory(pLocalizedText->Text.strContent, pLocalizedText->Text.uLength + 1);
		//		// Copy the content
		//		((OpcUa_Byte*&)*pVoidBufSource) += 4;
		//		void* apVoid1 = OpcUa_Alloc(4);
		//		memcpy(&apVoid1, *pVoidBufSource, 4);
		//		OpcUa_MemCpy((pLocalizedText->Text.strContent), pLocalizedText->Text.uLength, ((void*)(apVoid1)), pLocalizedText->Text.uLength);
		//		OpcUa_MemCpy(pVoidBufTarget, 4, &(pLocalizedText->Text.strContent), 4);
		//		// Update the pointer
		//		((OpcUa_Byte*&)pVoidBufTarget) += 4; //
		//		((OpcUa_Byte*&)*pVoidBufSource) += 4;
		//#endif
	}
		break;
	case OpcUaType_QualifiedName:
	{
		CopyBuiltInType(2, OpcUaType_UInt16, pVoidBufSource, pVoidBufTarget);
		CopyBuiltInType(2, OpcUaType_UInt16, pVoidBufSource, pVoidBufTarget);
		CopyBuiltInType(0, OpcUaType_String, pVoidBufSource, pVoidBufTarget);
		//iSize = 12; // string
		//iSize += 6; // uLength + flag
		//iSize += 4; // NamespaceIndex + Reserved
	}
		break;
	case OpcUaType_String:
	{
#ifdef _DEBUG
		OpcUa_String* pString = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
		OpcUa_String_Initialize(pString);
		// copy flag
		OpcUa_MemCpy(*pVoidBufTarget, 4, *pVoidBufSource, 4);
		((OpcUa_Byte*&)*pVoidBufTarget) += 4; // 
		OpcUa_MemCpy(&(pString->flags), 2, *pVoidBufSource, 2);
		((OpcUa_Byte*&)*pVoidBufSource) += 4; // The real size is 4 because a OpcUa_String is in fact an OpcUa_StringInternal
		// copy Length
		OpcUa_MemCpy(*pVoidBufTarget, 4, *pVoidBufSource, 4);
		((OpcUa_Byte*&)*pVoidBufTarget) += 4; //
		OpcUa_MemCpy(&(pString->uLength), 4, *pVoidBufSource, 4);
		pString->strContent = (OpcUa_CharA*)OpcUa_Alloc(pString->uLength + 1);
		ZeroMemory(pString->strContent, pString->uLength + 1);
		((OpcUa_Byte*&)*pVoidBufSource) += 4;
		// Copy the content
		void* apVoid = OpcUa_Alloc(4);
		memcpy(&apVoid, *pVoidBufSource, 4);
		OpcUa_MemCpy((pString->strContent), pString->uLength, ((void*)(apVoid)), pString->uLength);
		OpcUa_MemCpy(*pVoidBufTarget, 4, &(pString->strContent), 4);
		// Update the pointer
		((OpcUa_Byte*&)*pVoidBufTarget) += 4; //
		((OpcUa_Byte*&)*pVoidBufSource) += 4;
#else
		OpcUa_String* pString = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
		OpcUa_String_Initialize(pString);
		// copy flag
		OpcUa_MemCpy(*pVoidBufTarget, 4, *pVoidBufSource, 4);
		((OpcUa_Byte*&)*pVoidBufTarget) += 4; // 
		OpcUa_MemCpy(&(pString->uReserved1), 2, *pVoidBufSource, 2);
		((OpcUa_Byte*&)*pVoidBufSource) += 4; // The real size is 4 because a OpcUa_String is in fact an OpcUa_StringInternal
		// copy Length
		OpcUa_MemCpy(*pVoidBufTarget, 4, *pVoidBufSource, 4);
		((OpcUa_Byte*&)*pVoidBufTarget) += 4; //
		OpcUa_MemCpy(&(pString->uReserved2), 4, *pVoidBufSource, 4);
		pString->uReserved4 = (OpcUa_CharA*)OpcUa_Alloc(pString->uReserved2 + 1);
		ZeroMemory(pString->uReserved4, pString->uReserved2 + 1);
		((OpcUa_Byte*&)*pVoidBufSource) += 4;
		// Copy the content
		void* apVoid = OpcUa_Alloc(4);
		memcpy(&apVoid, *pVoidBufSource, 4);
		OpcUa_MemCpy((pString->uReserved4), pString->uReserved2, ((void*)(apVoid)), pString->uReserved2);
		OpcUa_MemCpy(*pVoidBufTarget, 4, &(pString->uReserved4), 4);
		// Update the pointer
		((OpcUa_Byte*&)*pVoidBufTarget) += 4; //
		((OpcUa_Byte*&)*pVoidBufSource) += 4;
#endif
	}
		break;
	case OpcUaType_Variant:
		printf("OpcUaType_Variant... arrggghhh Encapsulated Variant special case\n");
		break;
	case OpcUaType_NodeId:
	{
		OpcUa_NodeId aNodeId;
		
		// IdentifierType OpcUa_UInt16
		OpcUa_MemCpy(*pVoidBufTarget, 2, *pVoidBufSource, 2);
		OpcUa_MemCpy(&(aNodeId.IdentifierType), 2, *pVoidBufSource, 2);
		((OpcUa_Byte*&)*pVoidBufTarget) += 2; // 
		((OpcUa_Byte*&)*pVoidBufSource) += 2; // 	
		// NamespaceIndex OpcUa_UInt16
		OpcUa_MemCpy(*pVoidBufTarget, 2, *pVoidBufSource, 2);
		OpcUa_MemCpy(&(aNodeId.NamespaceIndex), 2, *pVoidBufSource, 2);
		((OpcUa_Byte*&)*pVoidBufTarget) += 2; // 
		((OpcUa_Byte*&)*pVoidBufSource) += 2; // 
		// Now the content of the NodeId
		switch (aNodeId.IdentifierType)
		{
		case OpcUa_IdentifierType_Numeric:
		{
			OpcUa_MemCpy(*pVoidBufTarget, 4, *pVoidBufSource, 4);
			OpcUa_MemCpy(&(aNodeId.Identifier.Numeric), 4, *pVoidBufSource, 4);
			((OpcUa_Byte*&)*pVoidBufTarget) += 4; // 
			((OpcUa_Byte*&)*pVoidBufSource) += 4; // 
		}
			break;
		case OpcUa_IdentifierType_String:
		{
#ifdef _DEBUG
			OpcUa_String_Initialize(&(aNodeId.Identifier.String));
			// copy flag
			OpcUa_MemCpy(*pVoidBufTarget, 4, *pVoidBufSource, 4);
			((OpcUa_Byte*&)*pVoidBufTarget) += 4; // 
			OpcUa_MemCpy(&(aNodeId.Identifier.String.flags), 2, *pVoidBufSource, 2);
			((OpcUa_Byte*&)*pVoidBufSource) += 4; // The real size is 4 because a OpcUa_String is in fact an OpcUa_StringInternal
			// copy Length
			OpcUa_MemCpy(*pVoidBufTarget, 4, *pVoidBufSource, 4);
			((OpcUa_Byte*&)*pVoidBufTarget) += 4; //
			OpcUa_MemCpy(&(aNodeId.Identifier.String.uLength), 4, *pVoidBufSource, 4);
			aNodeId.Identifier.String.strContent = (OpcUa_CharA*)OpcUa_Alloc(aNodeId.Identifier.String.uLength + 1);
			ZeroMemory(aNodeId.Identifier.String.strContent, aNodeId.Identifier.String.uLength + 1);
			// Copy the content
			((OpcUa_Byte*&)*pVoidBufSource) += 4;
			void* apVoid = OpcUa_Alloc(4);
			memcpy(&apVoid, *pVoidBufSource, 4);
			OpcUa_MemCpy((aNodeId.Identifier.String.strContent), aNodeId.Identifier.String.uLength, ((void*)(apVoid)), aNodeId.Identifier.String.uLength);
			OpcUa_MemCpy(*pVoidBufTarget, 4, &(aNodeId.Identifier.String.strContent), 4);
			// Update the pointer
			((OpcUa_Byte*&)*pVoidBufTarget) += 4; //
			((OpcUa_Byte*&)*pVoidBufSource) += 4;
#else
			OpcUa_String* pString = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
			OpcUa_String_Initialize(pString);
			// copy flag
			OpcUa_MemCpy(*pVoidBufTarget, 4, *pVoidBufSource, 4);
			((OpcUa_Byte*&)*pVoidBufTarget) += 4; // 
			OpcUa_MemCpy(&(pString->uReserved1), 2, *pVoidBufSource, 2);
			((OpcUa_Byte*&)*pVoidBufSource) += 4; // The real size is 4 because a OpcUa_String is in fact an OpcUa_StringInternal
			// copy Length
			OpcUa_MemCpy(*pVoidBufTarget, 4, *pVoidBufSource, 4);
			((OpcUa_Byte*&)*pVoidBufTarget) += 4; //
			OpcUa_MemCpy(&(pString->uReserved2), 4, *pVoidBufSource, 4);
			pString->uReserved4 = (OpcUa_CharA*)OpcUa_Alloc(pString->uReserved2 + 1);
			ZeroMemory(pString->uReserved4, pString->uReserved2 + 1);
			// Copy the content
			((OpcUa_Byte*&)*pVoidBufSource) += 4;
			void* apVoid = OpcUa_Alloc(4);
			memcpy(&apVoid, *pVoidBufSource, 4);
			OpcUa_MemCpy((pString->uReserved4), pString->uReserved2, ((void*)(apVoid)), pString->uReserved2);
			OpcUa_MemCpy(*pVoidBufTarget, 4, &(pString->uReserved4), 4);
			// Update the pointer
			((OpcUa_Byte*&)*pVoidBufTarget) += 4; //
			((OpcUa_Byte*&)*pVoidBufSource) += 4;
#endif
		}
			break;
		case OpcUa_IdentifierType_Guid:
		{
			; // TODO
		}
			break;
		case OpcUa_IdentifierType_Opaque:
		{
			; // TODO
		}
			break;
		default:
			uStatus = OpcUa_BadNodeIdInvalid;
			break;
		}
	}
		break;
	default:
		OpcUa_MemCpy(*pVoidBufTarget, ifieldSize, *pVoidBufSource, ifieldSize);
		// move the calculation pointer 
		((OpcUa_Byte*&)*pVoidBufSource) += ifieldSize;
		// move the result pointer
		((OpcUa_Byte*&)*pVoidBufTarget) += ifieldSize;
		break;
	}
	return uStatus;
}