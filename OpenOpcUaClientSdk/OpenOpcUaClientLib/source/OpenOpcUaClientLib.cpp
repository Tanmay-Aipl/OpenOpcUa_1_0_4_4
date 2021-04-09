/*****************************************************************************
	  Author
		©. Michel Condemine, 4CE Industry (2010-2013)
	  
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
#include "Application.h"
#include "OpenOpcUaClientLib.h"

#include "MonitoredItemClient.h"
#include "MonitoredItemsNotification.h"
#include "SubscriptionClient.h"
#include "ClientSession.h"
#include "LoggerMessage.h"
#include "ClientApplication.h"
#include "ClientAttribute.h" // Attribute structure used for lookup
#include "UABuiltInType.h" // BuiltIn type structure used for lookup

using namespace OpenOpcUa;
using namespace UACoreClient;
using namespace UASharedLib;
CClientApplicationList	g_pUaClientApplicationList;
OpcUa_Boolean g_bAbstractionLayerInitialized=OpcUa_False;
OpcUa_UInt32 g_ServiceCallTimeout = CLIENTLIB_DEFAULT_TIMEOUT;
OpcUa_UInt32 CClientApplication::m_uiRequestHandle = 1;
// Initialisation de la bibliothèque 
OpcUa_StatusCode OpenOpcUa_InitializeAbstractionLayer(OpcUa_CharA* szApplicationName, OpcUa_Handle* hApplication, OpcUa_UInt32 uiDefaultTimeout)
{
	OpcUa_StatusCode uStatus=OpcUa_BadInternalError;
	if (!szApplicationName)
		uStatus=OpcUa_BadInvalidArgument;
	else
	{
		if (!g_bAbstractionLayerInitialized)
		{
			CClientApplication* g_pUaClientApplication=new CClientApplication();
			g_pUaClientApplicationList.push_back(g_pUaClientApplication);
			*hApplication=(OpcUa_Handle)g_pUaClientApplication;
			OpcUa_LocalizedText aAppName;
			OpcUa_LocalizedText_Initialize(&aAppName);
			OpcUa_String_AttachCopy(&(aAppName.Text),szApplicationName);
			OpcUa_String_AttachCopy(&(aAppName.Locale),(OpcUa_CharA*)"en-US");
			g_pUaClientApplication->SetApplicationName(&aAppName);
			uStatus=OpcUa_Good;
			g_bAbstractionLayerInitialized=OpcUa_True;
			// Save the function timeout
			g_ServiceCallTimeout = uiDefaultTimeout;
			// Il restera a initialiser l'ApplicationDescription du client
			// Elle sera mise en place lors de l'appel à OpenOpcUa_InitializeSecurity

			// On place le niveau de trace
			// Mise en place du nom du fichier de sortie
			OpcUa_LocalizedText* sApplicationName=OpcUa_Null;
			OpcUa_String strFileName;
			OpcUa_String_Initialize(&strFileName);
			OpcUa_String strBakFileName;
			OpcUa_String_Initialize(&strBakFileName);
			// extension du fichier
			OpcUa_String strExtension;
			OpcUa_String_Initialize(&strExtension);
			OpcUa_String_AttachCopy(&strExtension,".log");
			// extension de la sauvegarde
			OpcUa_String strBakExtension;
			OpcUa_String_Initialize(&strBakExtension);
			OpcUa_String_AttachCopy(&strBakExtension,".bak");
			sApplicationName=g_pUaClientApplication->GetApplicationName();
			if (sApplicationName)
			{
				OpcUa_String_CopyTo(&(sApplicationName->Text),&strFileName);
				OpcUa_String_CopyTo(&(sApplicationName->Text),&strBakFileName);

				// creation du nom du fichier de log
				OpcUa_String_StrnCat(&strFileName,&strExtension,OpcUa_String_StrLen(&strExtension));
				// creation du nom du fichier de sauvegarde
				OpcUa_String_StrnCat(&strBakFileName,&strBakExtension,OpcUa_String_StrLen(&strBakExtension));
				OpcUa_Trace_SetTraceFile(strFileName);
				//
				// On sauvegarde une eventuelle trace existante
				FILE* hFile=OpcUa_Null;
				OpcUa_CharA* szFileName = OpcUa_String_GetRawString(&strFileName);
				hFile=fopen(szFileName, "r");
				if (hFile)
				{
					// Ouverture du fichier de sauvegarde
					FILE* hFileBak=OpcUa_Null;
					OpcUa_CharA* szBakFileName = OpcUa_String_GetRawString(&strBakFileName);
					hFileBak=fopen(szBakFileName, "w");
					char bakBuffer;
					// On lit l'ensemble du fichier dans le buffer
					// On copie dans la sauvegarde
					while (fread(&bakBuffer,sizeof(char),1,hFile)==1)
						fwrite(&bakBuffer,sizeof(char),1,hFileBak);
					fclose(hFileBak);
					hFileBak = OpcUa_Null;
					fclose(hFile);
					// On supprime l'ancien fichier 
					hFile = OpcUa_Null;
					hFile = fopen(szFileName, "w");
					if (hFile)
						fclose(hFile);
				}
				g_pUaClientApplication->SetTraceLevel(OPCUA_TRACE_CLIENT_LEVEL_ERROR);
				g_pUaClientApplication->SetTraceOutput(OPCUA_TRACE_OUTPUT_FILE);
				OpcUa_Trace_SetTraceFile(strFileName);
			}
			OpcUa_String_Clear(&strExtension);
			OpcUa_String_Clear(&strFileName);
			OpcUa_String_Clear(&strBakExtension);
			OpcUa_String_Clear(&strBakFileName);
			OpcUa_LocalizedText_Clear(&aAppName);
		}
	}
	return uStatus;
}
OpcUa_StatusCode OpenOpcUa_ClearAbstractionLayer(OpcUa_Handle hApplication)
{
	OpcUa_StatusCode uStatus=OpcUa_BadInternalError;
	OpcUa_LocalizedText* sApplicationName=OpcUa_Null;
	if (g_bAbstractionLayerInitialized)
	{
		OpcUa_LocalizedText_Initialize(sApplicationName);
		CClientApplication* pUaClientApplication=(CClientApplication*)hApplication;
		for (CClientApplicationList::iterator it=g_pUaClientApplicationList.begin();it!=g_pUaClientApplicationList.end();it++)
		{
			if (*it==pUaClientApplication)
			{
				delete pUaClientApplication;
				pUaClientApplication = OpcUa_Null;
				g_pUaClientApplicationList.erase(it);
				break;
			}
		}
		uStatus=OpcUa_Good;
	}
	return uStatus;
}
OpcUa_StatusCode OpenOpcUa_InitializeSecurity(OpcUa_Handle hApplication,OpcUa_String szCertificateStore)
{
	OpcUa_StatusCode uStatus=OpcUa_BadInternalError;
	OpcUa_String sApplicationUri;
	OpcUa_LocalizedText* pApplicationName=OpcUa_Null;
	if (g_bAbstractionLayerInitialized)
	{
		OpcUa_LocalizedText_Initialize(pApplicationName);
		CClientApplication* pUaClientApplication=(CClientApplication*)hApplication;
		pApplicationName=pUaClientApplication->GetApplicationName();
		// L'application URI est fabriquée a partifir de l'applicationName
		OpcUa_String_Initialize(&sApplicationUri);
		OpcUa_String_AttachCopy(
			&(sApplicationUri),(OpcUa_CharA*)"http://www.OpenOpcUa.org/UAClientLib/");
		OpcUa_String_StrnCat(&(sApplicationUri), &(pApplicationName->Text), OpcUa_String_StrLen(&(pApplicationName->Text)));
		//
		//uStatus=pUaClientApplication->InitializeSecurity(
		//	szCertificateStore, 
		//	&sApplicationUri, 
		//	sApplicationName,
		//	OpcUa_False); // will use the DER
		pUaClientApplication->SetCertificateStorePath(szCertificateStore);
		uStatus=pUaClientApplication->LoadPFXCertificate();
		if (uStatus==OpcUa_Good)
		{
			// on va tenter de lire le fichier DER afin de vérifier qu'il s'agit du bon DER
			// a savoir : 
			//	1- il existe
			//	2- sa clé correpsond acelle du pfx
			uStatus=pUaClientApplication->LoadDERCertificate();
			if (uStatus!=OpcUa_Good)
				uStatus=pUaClientApplication->CreateCertificate();
		}
		else
		{
			uStatus=pUaClientApplication->CreateCertificate();
		}
		uStatus = pUaClientApplication->InitializeSecurity(&sApplicationUri, pApplicationName);
		if (uStatus==OpcUa_Good)
		{
			// Maintenant que tout est pret on va remplir l'ApplicationDescription de ce client
			// Creation et remplissage d'un OpcUa_ApplicationDescription.
			// Il contient l'ensemble des informations relative a cette application UA (client)
			OpcUa_ApplicationDescription* pAppDescription=(OpcUa_ApplicationDescription*)OpcUa_Alloc(sizeof(OpcUa_ApplicationDescription));
			OpcUa_ApplicationDescription_Initialize(pAppDescription);
			pAppDescription->ApplicationType=OpcUa_ApplicationType_Client;
			// ApplicationUri
			OpcUa_String_CopyTo(&sApplicationUri, &(pAppDescription->ApplicationUri));
			// ApplicationName
			OpcUa_LocalizedText_CopyTo(pApplicationName, &(pAppDescription->ApplicationName));
			// ProductUri
			OpcUa_String_CopyTo(&sApplicationUri, &(pAppDescription->ProductUri));
			pUaClientApplication->SetApplicationDescription(pAppDescription);
			OpcUa_ApplicationDescription_Clear(pAppDescription);
			OpcUa_Free(pAppDescription);
		}
	}
	OpcUa_String_Clear(&sApplicationUri);
	//if (pApplicationName)
	//{
	//	OpcUa_LocalizedText_Clear(pApplicationName);
	//	OpcUa_Free(pApplicationName);
	//}
	return uStatus;
}
// FindServers
// L'usage classique de cette fonction consiste a demander au LDS ou au server ses ApplicationDescription
OpcUa_StatusCode OpenOpcUa_FindServers(OpcUa_Handle hApplication,OpcUa_String* hostName,OpcUa_Int32* uiNoOfServer,OpcUa_ApplicationDescription** pServers)
{
	OpcUa_StatusCode uStatus=OpcUa_BadInternalError;
	if (g_bAbstractionLayerInitialized)
	{
		OpcUa_RequestHeader tRequestHeader;
		OpcUa_String sDiscoveryUrl;
		OpcUa_String sLocaleIds;
		OpcUa_String sServerUris;
		OpcUa_ResponseHeader tResponseHeader;
		
		OpcUa_RequestHeader_Initialize(&tRequestHeader);
		OpcUa_String_Initialize(&sDiscoveryUrl);
		OpcUa_String_Initialize(&sLocaleIds);
		OpcUa_String_Initialize(&sServerUris);
		OpcUa_ResponseHeader_Initialize(&tResponseHeader);
		CClientApplication* pUaClientApplication=(CClientApplication*)hApplication;
		if (pUaClientApplication)
		{
			// Construction de la discoveryUrl
			std::string tmpDiscoveryUrl;
			// construct the discovery url from the host name.	
			tmpDiscoveryUrl.append("opc.tcp://");
			if (OpcUa_String_StrLen(hostName)>0)
				tmpDiscoveryUrl.append(OpcUa_String_GetRawString(hostName));
			else
				tmpDiscoveryUrl.append("localhost");
			tmpDiscoveryUrl.append(":4840");
			uStatus=OpcUa_String_AttachCopy(&sDiscoveryUrl,(OpcUa_CharA*)tmpDiscoveryUrl.c_str());
			// 
			tmpDiscoveryUrl.clear();
			CChannel channel(pUaClientApplication);
			OpcUa_MessageSecurityMode eSecurityMode = OpcUa_MessageSecurityMode_None;
			OpcUa_String szSecurityPolicy;
			OpcUa_String_Initialize(&szSecurityPolicy);
			uStatus = channel.Connect(sDiscoveryUrl,eSecurityMode,szSecurityPolicy);
			if (uStatus==OpcUa_Good)
			{
				// find the servers.
				tRequestHeader.TimeoutHint = g_ServiceCallTimeout;// UTILS_DEFAULT_TIMEOUT;
				tRequestHeader.Timestamp   = OpcUa_DateTime_UtcNow();
				tRequestHeader.RequestHandle = CClientApplication::m_uiRequestHandle++;
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
					uiNoOfServer,
					pServers);
			}
			tmpDiscoveryUrl.clear();
		}
		OpcUa_String_Clear(&sDiscoveryUrl);
		OpcUa_String_Clear(&sLocaleIds);
		OpcUa_String_Clear(&sServerUris);
	}
	return uStatus;
}

// GetEndpoints
OpcUa_StatusCode OpenOpcUa_GetEndpoints(OpcUa_Handle hApplication,OpcUa_String* discoveryUrl,OpcUa_UInt32* uiNoOfEndpointDescription,OpcUa_EndpointDescription** ppEndpointDescription)
{
	OpcUa_StatusCode uStatus=OpcUa_BadInternalError;
	if (g_bAbstractionLayerInitialized)
	{
		CClientApplication* pUaClientApplication=(CClientApplication*)hApplication;
		if (pUaClientApplication)
		{
			CEndpointDescriptionList aEndpoints;
			aEndpoints.clear();
			uStatus=pUaClientApplication->DiscoverEndpoints(*discoveryUrl,&aEndpoints);
			if (uStatus==OpcUa_Good)
			{
				*uiNoOfEndpointDescription=aEndpoints.size();

				OpcUa_EndpointDescription* pTmpEndpointDescription=(OpcUa_EndpointDescription*)OpcUa_Alloc((*uiNoOfEndpointDescription)*sizeof(OpcUa_EndpointDescription));
				*ppEndpointDescription = pTmpEndpointDescription;

				for (unsigned int ii = 0; ii < (*uiNoOfEndpointDescription); ii++)
				{
					CEndpointDescription* pEndpointDescription=aEndpoints.at(ii);
					if (pEndpointDescription)
					{						
						OpcUa_EndpointDescription_Initialize(&pTmpEndpointDescription[ii]);
						OpcUa_EndpointDescription* pInternalEndpointDescription=pEndpointDescription->GetInternalEndPointDescription();
						if (pInternalEndpointDescription)
						{
							///////////////////////////////////////////////////////////////////////////////////////
							// Copy the EndpointUrl
							if ( OpcUa_String_Compare(discoveryUrl,&(pInternalEndpointDescription->EndpointUrl))==0) 
								OpcUa_String_CopyTo(&(pInternalEndpointDescription->EndpointUrl), &(pTmpEndpointDescription[ii].EndpointUrl));
							else
								OpcUa_String_CopyTo(discoveryUrl, &(pTmpEndpointDescription[ii].EndpointUrl));
							// Copy the UserIdentity Tokens
							pTmpEndpointDescription[ii].NoOfUserIdentityTokens = pInternalEndpointDescription->NoOfUserIdentityTokens;
							OpcUa_UInt32 iLen=pTmpEndpointDescription[ii].NoOfUserIdentityTokens;
							if (iLen)
							{
								pTmpEndpointDescription[ii].UserIdentityTokens=(OpcUa_UserTokenPolicy*)OpcUa_Alloc(iLen*sizeof(OpcUa_UserTokenPolicy));
								ZeroMemory(pTmpEndpointDescription[ii].UserIdentityTokens, iLen*sizeof(OpcUa_UserTokenPolicy));
								for (unsigned int iii = 0; iii < iLen; iii++)
								{
									OpcUa_String_CopyTo(&(pInternalEndpointDescription->UserIdentityTokens[iii].IssuedTokenType),
										&(pTmpEndpointDescription[ii].UserIdentityTokens[iii].IssuedTokenType));
									OpcUa_String_CopyTo(&(pInternalEndpointDescription->UserIdentityTokens[iii].IssuerEndpointUrl),
										&(pTmpEndpointDescription[ii].UserIdentityTokens[iii].IssuerEndpointUrl));;
									OpcUa_String_CopyTo(&(pInternalEndpointDescription->UserIdentityTokens[iii].PolicyId),
										&(pTmpEndpointDescription[ii].UserIdentityTokens[iii].PolicyId));
									OpcUa_String_CopyTo(&(pInternalEndpointDescription->UserIdentityTokens[iii].SecurityPolicyUri),
										&(pTmpEndpointDescription[ii].UserIdentityTokens[iii].SecurityPolicyUri));
									pTmpEndpointDescription[ii].UserIdentityTokens[iii].TokenType = pInternalEndpointDescription->UserIdentityTokens[iii].TokenType;
								}
							}
							// Copy SecurityLevel and SecurityMode
							pTmpEndpointDescription[ii].SecurityLevel = pInternalEndpointDescription->SecurityLevel;
							pTmpEndpointDescription[ii].SecurityMode = pInternalEndpointDescription->SecurityMode;
							// Copy SecurityPolicyUri
							OpcUa_String aString=pInternalEndpointDescription->SecurityPolicyUri;
							OpcUa_String_CopyTo(&aString, &(pTmpEndpointDescription[ii].SecurityPolicyUri));
							//Copy ApplicationDescription
							//OpcUa_ApplicationDescription* pApplicationDescription=Utils::Copy(&(pInternalEndpointDescription->Server));	
							// fill the ApplicationDescription
							// ApplicationName
							OpcUa_LocalizedText_CopyTo(&(pInternalEndpointDescription->Server.ApplicationName), 
								&(pTmpEndpointDescription[ii].Server.ApplicationName));
							// ApplicationType
							pTmpEndpointDescription[ii].Server.ApplicationType=pInternalEndpointDescription->Server.ApplicationType;
							// ApplicationUri
							OpcUa_String_CopyTo(&(pInternalEndpointDescription->Server.ApplicationUri),
								&(pTmpEndpointDescription[ii].Server.ApplicationUri));
							// DiscoveryProfileUri
							OpcUa_String_CopyTo(&(pInternalEndpointDescription->Server.DiscoveryProfileUri),
								&(pTmpEndpointDescription[ii].Server.DiscoveryProfileUri));
							// DiscoveryUrls
							/*for (OpcUa_UInt32 u = 0; u < pInternalEndpointDescription->Server.NoOfDiscoveryUrls; u++)
							{
								OpcUa_String_CopyTo(&(pInternalEndpointDescription->Server.DiscoveryUrls[u]), 
									&(pTmpEndpointDescription[ii].Server.DiscoveryUrls[u]));
							}*/
							// GatewayServerUri
							OpcUa_String_CopyTo(&(pInternalEndpointDescription->Server.GatewayServerUri),
								&(pTmpEndpointDescription[ii].Server.GatewayServerUri));
							// ProductUri
							OpcUa_String_CopyTo(&(pInternalEndpointDescription->Server.ProductUri),
								&(pTmpEndpointDescription[ii].Server.ProductUri));

							// Copy ServerCertificate
							OpcUa_ByteString_CopyTo(&(pInternalEndpointDescription->ServerCertificate), &(pTmpEndpointDescription[ii].ServerCertificate));
							// Copy TransportProfileUri
							OpcUa_String_CopyTo(&(pInternalEndpointDescription->TransportProfileUri), &(pTmpEndpointDescription[ii].TransportProfileUri));
						}
					}
				}

			}
			// clear and release pEndPoints discovered
			CEndpointDescriptionList::iterator it;
			while (!aEndpoints.empty())
			{
				it=aEndpoints.begin();
				CEndpointDescription* pEndpointDescription=*it;
				delete pEndpointDescription;
				aEndpoints.erase(it);
			}
		}
	}
	return uStatus;
}
// recupère une ApplicationDescription en fonction de sont Handle
OpcUa_StatusCode OpenOpcUa_GetApplicationDescription(OpcUa_Handle hApplication, OpcUa_ApplicationDescription* pAppDescription)
{
	OpcUa_StatusCode uStatus=OpcUa_BadInternalError;
	if (g_bAbstractionLayerInitialized)
	{
		CClientApplication* pUaClientApplication=(CClientApplication*)hApplication;
		if (pUaClientApplication)
		{
			pAppDescription=pUaClientApplication->GetApplicationDescription()->GetInternalApplicationDescription();
			uStatus=OpcUa_Good;
		}
	}
	return uStatus;
}// recupère une EndpointDescription en fonction de sont Handle
// Il s'agit des recupérer les informations sur le serveur sur lequel le client est connecté actuellement
OpcUa_StatusCode OpenOpcUa_GetEndpointDescription(OpcUa_Handle hApplication, OpcUa_Handle hSession, OpcUa_EndpointDescription** pEndpointDescription)
{
	OpcUa_StatusCode uStatus=OpcUa_BadInternalError;
	if (g_bAbstractionLayerInitialized)
	{
		CClientApplication* pUaClientApplication=(CClientApplication*)hApplication;
		if (pUaClientApplication)
		{
			CSessionClient* pSession=(CSessionClient*)hSession;
			if (pSession)
			{
				CEndpointDescription* pUAEndpointDescription = pSession->GetEndpointDescription();
				if (pUAEndpointDescription)
				{
					*pEndpointDescription = pUAEndpointDescription->GetInternalEndPointDescription();
					uStatus = OpcUa_Good;
				}
			}
		}
	}
	return uStatus;
}
// Activate Session
/// <summary>
/// Activate the previously created hSession.
/// </summary>
/// <param name="hApplication">Handle of your Application.</param>
/// <param name="hSession">Hadnle of the Session to activcate.</param>
/// <returns></returns>
OpcUa_StatusCode OpenOpcUa_ActivateSession( OpcUa_Handle hApplication, OpcUa_Handle hSession, OpcUa_EndpointDescription* pEndpointDescription,
											const char * AuthenticationMode, const char * sUserName, const char * sPassword)
{
	OpcUa_StatusCode uStatus=OpcUa_BadInternalError;
	if (g_bAbstractionLayerInitialized)
	{
		CClientApplication* pUaClientApplication=(CClientApplication*)hApplication;
		if (pUaClientApplication)
		{
			CSessionClient* pSession=(CSessionClient*)hSession;
			if (pSession)
			{
				uStatus = pSession->Activate(pEndpointDescription, AuthenticationMode, sUserName, sPassword);
			}
		}
		else
			uStatus=OpcUa_BadInvalidArgument;
	}
	return uStatus;
}
// Create Session
OpcUa_StatusCode OpenOpcUa_CreateSession(OpcUa_Handle hApplication,
										 OpcUa_EndpointDescription* pEndpointDescription,
										 OpcUa_Double nRequestedClientSessionTimeout,
										 OpcUa_String aSessionName, 
										 OpcUa_Handle* hSession)
{
	OpcUa_StatusCode uStatus=OpcUa_BadInternalError;
	if (g_bAbstractionLayerInitialized)
	{
		CClientApplication* pUaClientApplication=(CClientApplication*)hApplication;
		if (pUaClientApplication)
		{
			if (pEndpointDescription->SecurityMode != OpcUa_MessageSecurityMode_None)
				pUaClientApplication->TrustCertificate(&(pEndpointDescription->ServerCertificate));

			CSessionClient*  pSession=new CSessionClient(pUaClientApplication);
			if (pSession)
			{
				CEndpointDescription* pEndPtDesc = new CEndpointDescription(pEndpointDescription);
				uStatus = pSession->Create(pEndPtDesc, &aSessionName, nRequestedClientSessionTimeout);
				if (uStatus == OpcUa_Good)
				{
					pUaClientApplication->AddSessionClient(pSession);
					*hSession = (OpcUa_Handle)pSession;
					// creation de la thread de surveillance du serveur
					pSession->StartWatchingThread();
					// creation de la thread de  publication
					pSession->StartPublishingThread();
				}
				else
					delete pSession;
				delete pEndPtDesc;
			}
			else
				uStatus = OpcUa_BadOutOfMemory;
		}
		else
			uStatus=OpcUa_BadInvalidArgument;
	}
	return uStatus;
}
// Close Session
OpcUa_StatusCode OpenOpcUa_CloseSession(OpcUa_Handle hApplication,OpcUa_Handle hSession)
{
	OpcUa_StatusCode uStatus=OpcUa_BadInternalError;

	if (g_bAbstractionLayerInitialized)
	{
		CClientApplication* pUaClientApplication=(CClientApplication*)hApplication;
		if (pUaClientApplication)
		{
			CSessionClient* pSession = OpcUa_Null;// (CSessionClient*)hSession;
			uStatus = pUaClientApplication->GetSession(hSession, &pSession);
			if ( (uStatus==OpcUa_Good) && (pSession) )
			{
				if (!pSession->IsWatchingThreadDetectError())
				{
					// First of all we need to stop the watching thread to avoid side effets					
					pSession->StopWatchingThread();
					// 
					uStatus = pSession->DeleteAllSubscriptions();
					if (uStatus != OpcUa_Good)
						OpcUa_Trace(OPCUA_TRACE_CLIENT_LEVEL_ERROR, "DeleteAllSubscriptions failed 0x%05x", uStatus);
					else
					{
						//CChannel* pChannel = pSession->GetChannel();
						//if (pChannel)
						//	pChannel->Disconnect();
						uStatus = pUaClientApplication->RemoveSessionClient(pSession);
					}
				}
				else
				{
					OpcUa_Trace(OPCUA_TRACE_CLIENT_LEVEL_ERROR, "The Watching Thread DetectError detected an error. The session has been closed that way\n");
					uStatus = OpcUa_Good;
				}
			}
		}
		else
			uStatus=OpcUa_BadInvalidArgument;
	}
	return uStatus;
}
// Create Subscription
OpcUa_StatusCode OpenOpcUa_CreateSubscription(OpcUa_Handle hApplication,OpcUa_Handle hSession,
											  OpcUa_Double* dblPublishingInterval, // In/Out param
											  OpcUa_UInt32* uiLifetimeCount, // In/Out param
											  OpcUa_UInt32* uiMaxKeepAliveCount,// In/Out param
											  OpcUa_UInt32 uiMaxNotificationsPerPublish,
											  OpcUa_Boolean bPublishingEnabled,
											  OpcUa_Byte aPriority,
																	OpcUa_Handle* hSubscription)
{
	OpcUa_StatusCode uStatus=OpcUa_BadInternalError;
	if (g_bAbstractionLayerInitialized)
	{
		CClientApplication* pUaClientApplication=(CClientApplication*)hApplication;
		if (pUaClientApplication)
		{
			CSessionClient* pSession=(CSessionClient*)hSession;
			if (pSession)
			{
				uStatus=pSession->CreateSubscription(dblPublishingInterval,uiLifetimeCount,uiMaxKeepAliveCount,uiMaxNotificationsPerPublish,bPublishingEnabled,aPriority,hSubscription);
			}
		}
	}
	return uStatus;
}
// Delete Subscription
OpcUa_StatusCode OpenOpcUa_DeleteSubscription(OpcUa_Handle hApplication,
											  OpcUa_Handle hSession,
											  OpcUa_Handle hSubscription)
{
	OpcUa_StatusCode uStatus=OpcUa_BadInternalError;
	if (g_bAbstractionLayerInitialized)
	{
		CClientApplication* pUaClientApplication=(CClientApplication*)hApplication;
		if (pUaClientApplication)
		{
			CSessionClient* pSession=(CSessionClient*)hSession;
			if (pSession)
			{
				CSubscriptionClient* pSubscriptionClient=(CSubscriptionClient*)hSubscription;
				if (pSubscriptionClient)
				{
					uStatus=pSession->DeleteSubscription(pSubscriptionClient);
				}
				else
					uStatus=OpcUa_BadInvalidArgument;
			}
			else
				uStatus=OpcUa_BadInvalidArgument;
		}
		else
			uStatus=OpcUa_BadInvalidArgument;
	}
	return uStatus;
}
// Modify Subscription
OpcUa_StatusCode OpenOpcUa_ModifySubscription(OpcUa_Handle hApplication,
											  OpcUa_Handle hSession,
											  OpcUa_Handle hSubscription,
											  OpcUa_Double* dblPublishingInterval, // In/Out param
											  OpcUa_UInt32* uiLifetimeCount, // In/Out param
											  OpcUa_UInt32* uiMaxKeepAliveCount,// In/Out param
											  OpcUa_UInt32 uiMaxNotificationsPerPublish,
											  OpcUa_Byte aPriority)
{
	OpcUa_StatusCode uStatus=OpcUa_BadInternalError;
	if (g_bAbstractionLayerInitialized)
	{
		CClientApplication* pUaClientApplication=(CClientApplication*)hApplication;
		if (pUaClientApplication)
		{
			CSessionClient* pSession=(CSessionClient*)hSession;
			if (pSession)
			{
				CSubscriptionClient* pSubscriptionClient=(CSubscriptionClient*)hSubscription;
				if (pSubscriptionClient)
				{
					uStatus=pSubscriptionClient->ModifySubscription(*dblPublishingInterval,*uiLifetimeCount,*uiMaxKeepAliveCount,uiMaxNotificationsPerPublish,aPriority);
				}
				else
					uStatus=OpcUa_BadInvalidArgument;
			}
			else
				uStatus=OpcUa_BadInvalidArgument;
		}
		else
			uStatus=OpcUa_BadInvalidArgument;
	}
	return uStatus;
}
// retrieve the subscription attached to this monitoredItem
OpcUa_StatusCode OpenOpcUa_GetSubscriptionOfMonitoredItem(OpcUa_Handle hApplication, OpcUa_Handle hSession, OpcUa_Handle hMonitoredItem, OpcUa_Handle* hSubscription)
{
	OpcUa_StatusCode uStatus = OpcUa_BadNotFound;
	if (g_bAbstractionLayerInitialized)
	{
		CClientApplication* pUaClientApplication = (CClientApplication*)hApplication;
		if (pUaClientApplication)
		{
			if (!hSession)
			{
				// will scan all sessions
				CSubscriptionClient* pSubscriptionClient = OpcUa_Null;
				uStatus=pUaClientApplication->GetSubscriptionOfMonitoredItem(hMonitoredItem,&pSubscriptionClient);
				if (uStatus==OpcUa_Good)
				{
					(*hSubscription) = (OpcUa_Handle)pSubscriptionClient;
				}
			}
			else
			{
				// scan just hSession
				CSessionClient* pSession = OpcUa_Null;
				uStatus = pUaClientApplication->GetSession(hSession, &pSession);
				if (uStatus == OpcUa_Good)
				{
					CSubscriptionClient* pSubscriptionClient = OpcUa_Null;
					uStatus=pSession->GetSubscriptionOfMonitoredItem(hMonitoredItem,&pSubscriptionClient);
					if (uStatus==OpcUa_Good)
					{
						(*hSubscription) = (OpcUa_Handle)pSubscriptionClient;
					}
				}
			}		
		}
	}
	return uStatus;
}
// Retieve the subscription params of the given subscription (hSubscription)
OpcUa_StatusCode OpenOpcUa_GetSubscriptionParams(OpcUa_Handle hApplication,
													OpcUa_Handle hSession,
													OpcUa_Handle hSubscription,
													OpenOpcUa_SubscriptionParam* pSubscriptionParam)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	if (pSubscriptionParam)
	{
		if (g_bAbstractionLayerInitialized)
		{
			CClientApplication* pUaClientApplication = (CClientApplication*)hApplication;
			if (pUaClientApplication)
			{
				CSessionClient* pSession = (CSessionClient*)hSession;
				if (pSession)
				{
					CSubscriptionClient* pSubscriptionClient = (CSubscriptionClient*)hSubscription;
					if (pSubscriptionClient)
					{
						pSubscriptionParam->uiSubscriptionId = pSubscriptionClient->GetSubscriptionId();
						pSubscriptionParam->bPublishingEnabled = pSubscriptionClient->GetPublishingEnabled();
						pSubscriptionParam->dblPublishingInterval = pSubscriptionClient->GetPublishingInterval();
						pSubscriptionParam->uiLifetimeCount = pSubscriptionClient->GetLifetimeCount();
						pSubscriptionParam->uiMaxKeepAliveCount = pSubscriptionClient->GetMaxKeepAliveCount();
						pSubscriptionParam->aPriority = pSubscriptionClient->GetPriority();
						pSubscriptionParam->uiMaxNotificationsPerPublish = pSubscriptionClient->GetMaxNotificationsPerPublish();
					}
					else
						uStatus = OpcUa_BadInvalidArgument;
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
	return uStatus;
}

OpcUa_StatusCode OpenOpcUa_SetPublishingMode(OpcUa_Handle hApplication,
											  OpcUa_Handle hSession,
											  OpcUa_Handle hSubscription,
											  OpcUa_Boolean bPublishMode)
{
	OpcUa_StatusCode uStatus=OpcUa_BadInternalError;
	if (g_bAbstractionLayerInitialized)
	{
		CClientApplication* pUaClientApplication=(CClientApplication*)hApplication;
		if (pUaClientApplication)
		{
			CSessionClient* pSession=(CSessionClient*)hSession;
			if (pSession)
			{
				CSubscriptionClient* pSubscriptionClient=(CSubscriptionClient*)hSubscription;
				if (pSubscriptionClient)
				{
					uStatus=pSubscriptionClient->SetPublishingMode(bPublishMode);
				}
				else
					uStatus=OpcUa_BadInvalidArgument;
			}
			else
				uStatus=OpcUa_BadInvalidArgument;
		}
		else
			uStatus=OpcUa_BadInvalidArgument;
	}
	return uStatus;
}
// Create MonitoredItems
OpcUa_StatusCode OpenOpcUa_CreateMonitoredItemsEx(	OpcUa_Handle					hApplication,// in
													OpcUa_Handle					hSession, // in
													OpcUa_Handle					hSubscription,// in
													OpcUa_UInt32					NoOfItemsToCreate, // in
													OpcUa_MonitoredItemToCreate*	pItemsToCreate, // in
													OpcUa_MonitoredItemCreated**	ppItemsCreated) // out

{
	OpcUa_StatusCode uStatus = OpcUa_BadInternalError;

	if (g_bAbstractionLayerInitialized)
	{
		CClientApplication* pUaClientApplication = (CClientApplication*)hApplication;
		if (pUaClientApplication)
		{
			CSessionClient* pSession = (CSessionClient*)hSession;
			if (pSession)
			{
				CSubscriptionClient* pSubscriptionClient = (CSubscriptionClient*)hSubscription;
				if (pSubscriptionClient)
				{
					OpcUa_MonitoredItemCreateRequest* pItemsToCreateRequest = (OpcUa_MonitoredItemCreateRequest*)OpcUa_Alloc(sizeof(OpcUa_MonitoredItemCreateRequest)*NoOfItemsToCreate);
					ZeroMemory(pItemsToCreateRequest, sizeof(OpcUa_MonitoredItemCreateRequest)*NoOfItemsToCreate);
					for (OpcUa_UInt32 i = 0; i < NoOfItemsToCreate; i++)
					{
						pItemsToCreateRequest[i].ItemToMonitor.AttributeId = pItemsToCreate[i].m_AttributeId;
						//OpcUa_QualifiedName_CopyTo(pItemsToCreate[i], pItemsToCreateRequest->ItemToMonitor.DataEncoding);
						OpcUa_String_CopyTo(&(pItemsToCreate[i].m_IndexRange), &(pItemsToCreateRequest[i].ItemToMonitor.IndexRange));
						OpcUa_NodeId_CopyTo(&(pItemsToCreate[i].m_NodeId), &(pItemsToCreateRequest[i].ItemToMonitor.NodeId));
						pItemsToCreateRequest[i].MonitoringMode = pItemsToCreate[i].m_MonitoringMode;
						pItemsToCreateRequest[i].RequestedParameters.ClientHandle = pItemsToCreate[i].m_ClientHandle;
						pItemsToCreateRequest[i].RequestedParameters.DiscardOldest = pItemsToCreate[i].m_DiscardOldest;
						// Filter;
						OpcUa_ExtensionObject_Initialize(&(pItemsToCreateRequest[i].RequestedParameters.Filter));
						pItemsToCreateRequest[i].RequestedParameters.Filter.TypeId.NodeId.Identifier.Numeric = OpcUaId_DataChangeFilter_Encoding_DefaultBinary;
						pItemsToCreateRequest[i].RequestedParameters.Filter.Encoding = OpcUa_ExtensionObjectEncoding_EncodeableObject;
						pItemsToCreateRequest[i].RequestedParameters.Filter.Body.EncodeableObject.Type = (OpcUa_EncodeableType*)OpcUa_Alloc(sizeof(OpcUa_EncodeableType));
						pItemsToCreateRequest[i].RequestedParameters.Filter.Body.EncodeableObject.Type = &OpcUa_DataChangeFilter_EncodeableType;
						//DataChangeFilter.Body.EncodeableObject.Type->TypeId = OpcUaId_DataChangeFilter;
						OpcUa_DataChangeFilter* pDataChangeFilter = (OpcUa_DataChangeFilter*)OpcUa_Alloc(sizeof(OpcUa_DataChangeFilter));
						if (pDataChangeFilter)
						{
							OpcUa_DataChangeFilter_Initialize(pDataChangeFilter);
							pDataChangeFilter->DeadbandType = pItemsToCreate[i].m_DeadbandType;
							pDataChangeFilter->DeadbandValue = pItemsToCreate[i].m_DeadbandValue;
							pDataChangeFilter->Trigger = (OpcUa_DataChangeTrigger)pItemsToCreate[i].m_DatachangeTrigger;
							pItemsToCreateRequest[i].RequestedParameters.Filter.Body.EncodeableObject.Object = (void*)pDataChangeFilter;
						}
						pItemsToCreateRequest[i].RequestedParameters.QueueSize = pItemsToCreate[i].m_QueueSize;
						pItemsToCreateRequest[i].RequestedParameters.SamplingInterval = pItemsToCreate[i].m_SamplingInterval;
					}
					OpcUa_TimestampsToReturn sharedTimestampToReturn = pItemsToCreate[0].m_TimestampsToReturn;
					OpcUa_MonitoredItemCreateResult* pResult = OpcUa_Null;
					OpcUa_Handle* hMonitoredItems=OpcUa_Null;
					// We will subscribe to the requested nodes
					uStatus = pSubscriptionClient->CreateMonitoredItems(
						(OpcUa_TimestampsToReturn)sharedTimestampToReturn,
													NoOfItemsToCreate,
													pItemsToCreateRequest,
													&pResult,
													&hMonitoredItems);
					if (uStatus == OpcUa_Good)
					{
						OpcUa_Int32 iNodNodesToRead = 0;
						OpcUa_ReadValueId* pNodesToRead = OpcUa_Null;
						OpcUa_DataValue* pResults = OpcUa_Null;
						iNodNodesToRead = NoOfItemsToCreate;
						pNodesToRead = (OpcUa_ReadValueId*)OpcUa_Alloc(iNodNodesToRead*sizeof(OpcUa_ReadValueId));
						//////////////
						// Fill the result of the creation
						(*ppItemsCreated) = (OpcUa_MonitoredItemCreated*)OpcUa_Alloc(sizeof(OpcUa_MonitoredItemCreated)*NoOfItemsToCreate);
						ZeroMemory((*ppItemsCreated), sizeof(OpcUa_MonitoredItemCreated)*NoOfItemsToCreate);
						for (OpcUa_UInt32 ii = 0; ii < NoOfItemsToCreate; ii++)
						{
							(*ppItemsCreated)[ii].m_hMonitoredItem = hMonitoredItems[ii];
							(*ppItemsCreated)[ii].m_MonitoredItemId = pResult[ii].MonitoredItemId;
							(*ppItemsCreated)[ii].m_pRevisedQueueSize = pResult[ii].RevisedQueueSize;
							(*ppItemsCreated)[ii].m_Result = pResult[ii].StatusCode;
							(*ppItemsCreated)[ii].m_RevisedSamplingInterval = pResult[ii].RevisedSamplingInterval;
						}
						//////////////

						// Read BrowseName for all Added Nodes
						/*
						for (OpcUa_UInt32 ii = 0; ii<NoOfItemsToCreate; ii++)
						{
							OpcUa_ReadValueId_Initialize(&pNodesToRead[ii]);
							pNodesToRead[ii].AttributeId = OpcUa_Attributes_BrowseName;
							OpcUa_NodeId_CopyTo(&(pItemsToCreate[ii].m_NodeId), &(pNodesToRead[ii].NodeId));
						}
						uStatus = OpenOpcUa_ReadAttributes(hApplication, hSession, OpcUa_TimestampsToReturn_Both, iNodNodesToRead, pNodesToRead, &pResults);
						if (uStatus == OpcUa_Good)
						{
							for (OpcUa_UInt32 ii = 0; ii<NoOfItemsToCreate; ii++)
							{
								CMonitoredItemClient* pMonitoredItem = (CMonitoredItemClient*)(hMonitoredItems)[ii];
								if (pMonitoredItem)
								{
									if (pResults[ii].Value.Value.QualifiedName)
									{
										pMonitoredItem->SetMonitoredItemName(pResults[ii].Value.Value.QualifiedName->Name);
									}
								}
								OpcUa_DataValue_Clear(&pResults[ii]);
							}
						}
						if (pResults)
						{
							OpcUa_Free(pResults);
							pResults = OpcUa_Null;
						}
						*/
						// Read Value for all Added Nodes
						/*
						for (OpcUa_UInt32 ii = 0; ii<NoOfItemsToCreate; ii++)
						{
							OpcUa_ReadValueId_Initialize(&pNodesToRead[ii]);
							pNodesToRead[ii].AttributeId = OpcUa_Attributes_Value;
							OpcUa_NodeId_CopyTo(&(pItemsToCreate[ii].m_NodeId), &(pNodesToRead[ii].NodeId));
						}
						uStatus = OpenOpcUa_ReadAttributes(hApplication, hSession, OpcUa_TimestampsToReturn_Both, iNodNodesToRead, pNodesToRead, &pResults);
						if (uStatus == OpcUa_Good)
						{
							for (OpcUa_UInt32 ii = 0; ii<NoOfItemsToCreate; ii++)
							{
								CMonitoredItemClient* pMonitoredItem = (CMonitoredItemClient*)(hMonitoredItems)[ii];
								if (pMonitoredItem)
								{
									//if (pResults[ii].Value.Value.QualifiedName)
									{
										pMonitoredItem->SetValue(&pResults[ii]);
									}
								}
								OpcUa_ReadValueId_Clear(pNodesToRead);
								OpcUa_DataValue_Clear(&pResults[ii]);
							}
						}
						if (pResults)
						{
							OpcUa_Free(pResults);
							pResults = OpcUa_Null;
						}
						*/
						OpcUa_Free(pNodesToRead);
					}
				}
			}
		}
	}

	return uStatus;
}
// Create MonitoredItems
OpcUa_StatusCode OpenOpcUa_CreateMonitoredItems(OpcUa_Handle hApplication,
												OpcUa_Handle hSession,
												OpcUa_Handle hSubscription,
												OpcUa_Byte aTimestampsToReturn,
												OpcUa_UInt32 NoOfItemsToCreate,
												OpcUa_MonitoredItemCreateRequest* pItemsToCreate,
												OpcUa_MonitoredItemCreateResult** ppResult,
												OpcUa_Handle** hMonitoredItems)
{
	OpcUa_StatusCode uStatus=OpcUa_BadInternalError;
	if (g_bAbstractionLayerInitialized)
	{
		CClientApplication* pUaClientApplication=(CClientApplication*)hApplication;
		if (pUaClientApplication)
		{
			CSessionClient* pSession=(CSessionClient*)hSession;
			if (pSession)
			{
				CSubscriptionClient* pSubscriptionClient=(CSubscriptionClient*)hSubscription;
				if (pSubscriptionClient)
				{
					// We will subscribe to the requested nodes
					uStatus=pSubscriptionClient->CreateMonitoredItems(
						(OpcUa_TimestampsToReturn)aTimestampsToReturn,
						NoOfItemsToCreate,
						pItemsToCreate,
						ppResult,
						hMonitoredItems);
					if (uStatus==OpcUa_Good)
					{
						OpcUa_Int32 iNodNodesToRead=0;
						OpcUa_ReadValueId* pNodesToRead=OpcUa_Null;
						OpcUa_DataValue* pResults=OpcUa_Null;
						iNodNodesToRead=NoOfItemsToCreate;
						pNodesToRead=(OpcUa_ReadValueId*)OpcUa_Alloc(iNodNodesToRead*sizeof(OpcUa_ReadValueId));
						// Read BrowseName for all Added Nodes
						for (OpcUa_UInt32 ii=0;ii<NoOfItemsToCreate;ii++)
						{
							OpcUa_ReadValueId_Initialize(&pNodesToRead[ii]);
							pNodesToRead[ii].AttributeId=OpcUa_Attributes_BrowseName;
							OpcUa_NodeId_CopyTo(&(pItemsToCreate[ii].ItemToMonitor.NodeId), &(pNodesToRead[ii].NodeId));
						}
						uStatus=OpenOpcUa_ReadAttributes(hApplication,hSession,OpcUa_TimestampsToReturn_Both,iNodNodesToRead,pNodesToRead,&pResults);
						if (uStatus==OpcUa_Good)
						{
							for (OpcUa_UInt32 ii=0;ii<NoOfItemsToCreate;ii++)
							{
								CMonitoredItemClient* pMonitoredItem=(CMonitoredItemClient*)(*hMonitoredItems)[ii];
								if (pMonitoredItem)
								{
									if (pResults[ii].Value.Value.QualifiedName)
									{
										pMonitoredItem->SetMonitoredItemName(pResults[ii].Value.Value.QualifiedName->Name);
										OpcUa_QualifiedName_Clear(pResults[ii].Value.Value.QualifiedName);
									}
								}
								OpcUa_DataValue_Clear(&pResults[ii]);
							}
						}
						for (OpcUa_UInt32 ii = 0; ii < NoOfItemsToCreate; ii++)
						{
							OpcUa_NodeId_Clear(&(pNodesToRead[ii].NodeId));
						}
						if (pResults)
						{
							OpcUa_Free(pResults);
							pResults = OpcUa_Null;
						}
						// Read Value for all Added Nodes
						for (OpcUa_UInt32 ii=0;ii<NoOfItemsToCreate;ii++)
						{
							OpcUa_ReadValueId_Initialize(&pNodesToRead[ii]);
							pNodesToRead[ii].AttributeId=OpcUa_Attributes_Value;							
							OpcUa_NodeId_CopyTo(&(pItemsToCreate[ii].ItemToMonitor.NodeId),&(pNodesToRead[ii].NodeId));
						}
						uStatus=OpenOpcUa_ReadAttributes(hApplication,hSession,OpcUa_TimestampsToReturn_Both,iNodNodesToRead,pNodesToRead,&pResults);
						if (uStatus==OpcUa_Good)
						{
							for (OpcUa_UInt32 ii=0;ii<NoOfItemsToCreate;ii++)
							{
								CMonitoredItemClient* pMonitoredItem=(CMonitoredItemClient*)(*hMonitoredItems)[ii];
								if (pMonitoredItem)
								{
									//if (pResults[ii].Value.Value.QualifiedName)
									{
										pMonitoredItem->SetValue(&pResults[ii]);
									}
								}
								OpcUa_ReadValueId_Clear(pNodesToRead);
								OpcUa_DataValue_Clear(&pResults[ii]);
							}
						}
						if (pResults)
						{
							OpcUa_Free(pResults);
							pResults = OpcUa_Null;
						}
						for (OpcUa_UInt32 ii = 0; ii < NoOfItemsToCreate; ii++)
						{
							OpcUa_NodeId_Clear(&(pNodesToRead[ii].NodeId));
						}
						if (pNodesToRead)
						{
							OpcUa_Free(pNodesToRead);
							pNodesToRead = OpcUa_Null;
						}
					}
				}
			}
		}
	}
	return uStatus;
}
// Delete MonitoredItems
OpcUa_StatusCode OpenOpcUa_DeleteMonitoredItems(OpcUa_Handle hApplication,
												OpcUa_Handle hSession,
												OpcUa_Handle hSubscription,
												OpcUa_Int32 iNoOfMonitoredItem,
												OpcUa_Handle* hMonitoredItems,
												OpcUa_StatusCode** ppStatusCode)
{
	OpcUa_StatusCode uStatus=OpcUa_BadInternalError;
	if (g_bAbstractionLayerInitialized)
	{
		CClientApplication* pUaClientApplication = (CClientApplication*)hApplication;
		if (pUaClientApplication)
		{
			CSessionClient* pSession = (CSessionClient*)hSession;
			if (pSession)
			{
				CSubscriptionClient* pSubscriptionClient = (CSubscriptionClient*)hSubscription;
				if (pSubscriptionClient)
				{
						// let's prepare the list of monitoredItem to delete
						OpcUa_Mutex monitoredItemListMutex = pSubscriptionClient->GetMonitoredItemListMutex();
						OpcUa_Mutex_Lock(monitoredItemListMutex);
						CMonitoredItemClientList* pMonitoredItemList = new CMonitoredItemClientList();
						for (OpcUa_Int32 i = 0; i < iNoOfMonitoredItem; i++)
						{
							CMonitoredItemClient* pMonitoredItem = pSubscriptionClient->FindMonitoredItemByHandle((OpcUa_UInt32)(uintptr_t)hMonitoredItems[i]);
							if (pMonitoredItem)
								pMonitoredItemList->push_back(pMonitoredItem);
						}
						OpcUa_Mutex_Unlock(monitoredItemListMutex);
						// Now lets inform the server that we want to delete it
						if (iNoOfMonitoredItem > 0)
							uStatus=pSubscriptionClient->DeleteMonitoredItems(pMonitoredItemList);
						else
							OpcUa_BadInvalidArgument;
						delete pMonitoredItemList;
				}
			}
			else
				uStatus = OpcUa_BadInvalidArgument;
		}
		else
			uStatus = OpcUa_BadInvalidArgument;
	}
	return uStatus;
}
// Modify MonitoredItems All parameter are input parameters
OpcUa_StatusCode OpenOpcUa_ModifyMonitoredItems(OpcUa_Handle	hApplication,
												OpcUa_Handle	hSession,
												OpcUa_Handle	hSubscription,
												OpcUa_Handle	hMonitoredItem,
												OpcUa_Byte		aTimestampsToReturn,
												OpcUa_Boolean	bDiscardOldest,
												OpcUa_UInt32	uiQueueSize,
												OpcUa_Double	dblSamplingInterval,
												OpcUa_UInt32	DeadbandType,
												OpcUa_Double	DeadbandValue,
												OpcUa_Byte		DatachangeTrigger)
{
	OpcUa_StatusCode uStatus = OpcUa_BadInternalError;
	if (g_bAbstractionLayerInitialized)
	{
		CClientApplication* pUaClientApplication = (CClientApplication*)hApplication;
		if (pUaClientApplication)
		{
			CSessionClient* pSession = (CSessionClient*)hSession;
			if (pSession)
			{
				CSubscriptionClient* pSubscriptionClient = (CSubscriptionClient*)hSubscription;
				if (pSubscriptionClient)
				{
					uStatus = pSubscriptionClient->ModifyMonitoredItems(hMonitoredItem,aTimestampsToReturn,bDiscardOldest,uiQueueSize,dblSamplingInterval,DeadbandType,DeadbandValue,DatachangeTrigger);
				}
			}
		}
	}
	return uStatus;
}
// SetMonitoringMode
OpcUa_StatusCode OpenOpcUa_SetMonitoringMode(OpcUa_Handle hApplication, OpcUa_Handle hSession, OpcUa_Handle hSubscription,
	OpcUa_Int32			iNoOfMonitoredItem,
	OpcUa_Handle*			hMonitoredItems,
	OpcUa_MonitoringMode	eMonitoringMode)
{
	OpcUa_StatusCode uStatus=OpcUa_BadInternalError;
	if (hMonitoredItems)
	{
		if (g_bAbstractionLayerInitialized)
		{
			CClientApplication* pUaClientApplication = (CClientApplication*)hApplication;
			if (pUaClientApplication)
			{
				CSessionClient* pSession = (CSessionClient*)hSession;
				if (pSession)
				{
					CSubscriptionClient* pSubscriptionClient = (CSubscriptionClient*)hSubscription;
					if (pSubscriptionClient)
					{
						pSubscriptionClient->SetDefaultMonitoringMode(eMonitoringMode);
						if (iNoOfMonitoredItem > 0)
							uStatus = pSubscriptionClient->SetMonitoringMode(eMonitoringMode, iNoOfMonitoredItem, (OpcUa_UInt32*)hMonitoredItems);
						else
							uStatus = OpcUa_Good;
					}
				}
			}
		}
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}
// Read Attributes
OpcUa_StatusCode OpenOpcUa_ReadAttributes(  OpcUa_Handle hApplication,
											OpcUa_Handle hSession,
											OpcUa_TimestampsToReturn eTimestampsToReturn,
											OpcUa_Int32 iNoOfNodesToRead, 
											OpcUa_ReadValueId* pNodesToRead,
											OpcUa_DataValue** ppResults)
{
	OpcUa_StatusCode uStatus=OpcUa_BadInternalError;
	if (g_bAbstractionLayerInitialized)
	{
		CClientApplication* pUaClientApplication=(CClientApplication*)hApplication;
		if (pUaClientApplication)
		{
			CSessionClient* pSession=(CSessionClient*)hSession;
			if (pSession)
			{
				OpcUa_ReadValueIdList	aReadValueIdList;
				OpcUa_DataValueList Results;
				OpcUa_DataValueList::iterator iteratorResult;
				// on demande les attributs que l'on souhaite lire
				for (int ii = 0;ii<iNoOfNodesToRead;ii++)
				{
					aReadValueIdList.push_back(&pNodesToRead[ii]);
				}
				uStatus=pSession->Read(aReadValueIdList,eTimestampsToReturn,&Results);
				if (uStatus==OpcUa_Good)
				{
					OpcUa_UInt32 iii=0;
					(*ppResults)=(OpcUa_DataValue*)OpcUa_Alloc(Results.size()*sizeof(OpcUa_DataValue));
					for (OpcUa_UInt32 ij = 0; ij < Results.size();ij++)
					//for (OpcUa_DataValueList::iterator it=Results.begin();it!=Results.end();it++)
					{
						OpcUa_DataValue_Initialize(&((*ppResults)[iii]));
						OpcUa_DataValue* pDataValue = Results.at(ij);
						(*ppResults)[iii].ServerPicoseconds=pDataValue->ServerPicoseconds;
						(*ppResults)[iii].ServerTimestamp=pDataValue->ServerTimestamp;
						(*ppResults)[iii].SourcePicoseconds=pDataValue->SourcePicoseconds;
						(*ppResults)[iii].SourceTimestamp=pDataValue->SourceTimestamp;
						(*ppResults)[iii].StatusCode=pDataValue->StatusCode;
						if (pDataValue->StatusCode==OpcUa_Good)
							OpcUa_Variant_CopyTo(&(pDataValue->Value),&((*ppResults)[iii].Value));
						else
						{
							(*ppResults)[iii].Value.ArrayType = pDataValue->Value.ArrayType;
							(*ppResults)[iii].Value.Datatype = pDataValue->Value.Datatype;
						}
						iii++;
						OpcUa_DataValue_Clear(pDataValue);
						OpcUa_Free(pDataValue);
						pDataValue = OpcUa_Null;
					}
					Results.clear();
				}
				aReadValueIdList.clear();
			}
		}
	}
	return uStatus;
}
// Write Attributes
OpcUa_StatusCode OpenOpcUa_WriteAttributes(OpcUa_Handle hApplication,
										   OpcUa_Handle hSession,
										   OpcUa_Int32 iNoOfNodesToWrite, 
										   OpcUa_WriteValue* pNodesToWrite,
										   OpcUa_StatusCode** ppStatusCode)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	if (g_bAbstractionLayerInitialized)
	{
		CClientApplication* pUaClientApplication=(CClientApplication*)hApplication;
		if (pUaClientApplication)
		{
			CSessionClient* pSession=(CSessionClient*)hSession;
			if (pSession)
			{
				uStatus = pSession->Write(iNoOfNodesToWrite, pNodesToWrite, ppStatusCode);
				/* *ppStatusCode=(OpcUa_StatusCode*)OpcUa_Alloc(sizeof(OpcUa_StatusCode)*iNoOfNodesToWrite);
				for (int ii=0;ii<iNoOfNodesToWrite;ii++)
				{
					(*ppStatusCode)[ii]=pSession->Write(&pNodesToWrite[ii]);
					if ((*ppStatusCode)[ii]!=OpcUa_Good)
						uStatus=(*ppStatusCode)[ii];
				}
				*/
			}
		}
	}
	return uStatus;
}

OpcUa_StatusCode HistoryReadCallback(
	OpcUa_Channel         hChannel,
	OpcUa_Void*           pResponse,
	OpcUa_EncodeableType* pResponseType,
	OpcUa_Void*           pCallbackData,
	OpcUa_StatusCode      StatusCode)
{
	OpcUa_StatusCode uStatus=OpcUa_BadInternalError;

	return uStatus;
}
OpcUa_StatusCode OpenOpcUa_HistoryReadRaw(OpcUa_Handle					hApplication,
										  OpcUa_Handle					hSession,
										  OpcUa_Boolean					bModified,
										  OpcUa_TimestampsToReturn      eTimestampsToReturn,
										  OpcUa_Int32                   iNoOfNodesToRead,
										  OpenOpcUa_HistoryReadValueId* pNodesToRead,
										  OpcUa_Int32*                  ipNoOfResults,
										  OpenOpcUa_HistoryReadResult** ppHistoryResults)
{
	OpcUa_StatusCode uStatus=OpcUa_BadInternalError;
	OpcUa_RequestHeader			tRequestHeader;

	if (g_bAbstractionLayerInitialized)
	{
		CClientApplication* pUaClientApplication = (CClientApplication*)hApplication;
		if (pUaClientApplication)
		{
			CSessionClient* pSession = (CSessionClient*)hSession;
			if (pSession)
			{
				OpcUa_RequestHeader_Initialize(&tRequestHeader);
				tRequestHeader.TimeoutHint = g_ServiceCallTimeout;// UTILS_DEFAULT_TIMEOUT;
				tRequestHeader.Timestamp = OpcUa_DateTime_UtcNow();
				tRequestHeader.AuthenticationToken.IdentifierType = OpcUa_IdentifierType_Numeric;
				tRequestHeader.AuthenticationToken.NamespaceIndex = 0;
				OpcUa_NodeId_CopyTo(pSession->GetAuthenticationToken(), &(tRequestHeader.AuthenticationToken));
				tRequestHeader.RequestHandle = CClientApplication::m_uiRequestHandle++;
				// Let's make the asynchronous call
				CChannel* pChannel = pSession->GetChannel();
				OpcUa_Channel aChannel = pChannel->GetInternalHandle();
				OpcUa_ExtensionObject* pHistoryReadDetails=OpcUa_Null;
				OpcUa_Boolean bReleaseContinuationPoints = OpcUa_False;
				OpcUa_HistoryReadValueId* pHistoryReadValueId;
				void* pCallbackData = OpcUa_Null;
				if (iNoOfNodesToRead > 0)
				{
					pHistoryReadValueId = (OpcUa_HistoryReadValueId*)OpcUa_Alloc(sizeof(OpcUa_HistoryReadValueId)*iNoOfNodesToRead);
					if (pHistoryReadValueId)
					{
						uStatus=OpcUa_ClientApi_BeginHistoryRead(aChannel,
							&tRequestHeader,
							pHistoryReadDetails,
							eTimestampsToReturn,
							bReleaseContinuationPoints,
							iNoOfNodesToRead,
							pHistoryReadValueId,
							(OpcUa_Channel_PfnRequestComplete*)&HistoryReadCallback,
							pCallbackData);
					}
				}
				else
					uStatus = OpcUa_BadInvalidArgument;
			}
		}
	}
	return uStatus;
}

// Call
OpcUa_StatusCode OpenOpcUa_Call(OpcUa_Handle hApplication,
								OpcUa_Handle hSession,
								OpcUa_Int32 iNoOfMethodsToCall,
								OpcUa_CallMethodRequest* pMethodsToCall,
								OpcUa_CallMethodResult** ppResults)
{
	OpcUa_StatusCode uStatus=OpcUa_BadInternalError;
	OpcUa_RequestHeader	tRequestHeader;
	OpcUa_ResponseHeader* pResponseHeader = OpcUa_Null;
	OpcUa_Int32             nNoOfDiagnosticInfos = 0;
	OpcUa_DiagnosticInfo*   pDiagnosticInfos = 0;
	if (g_bAbstractionLayerInitialized)
	{
		CClientApplication* pUaClientApplication = (CClientApplication*)hApplication;
		if (pUaClientApplication)
		{
			CSessionClient* pSession = (CSessionClient*)hSession;
			if (pSession)
			{
				OpcUa_Int32 NoOfResult;
				OpcUa_RequestHeader_Initialize(&tRequestHeader);
				tRequestHeader.TimeoutHint = g_ServiceCallTimeout;// UTILS_DEFAULT_TIMEOUT;
				tRequestHeader.Timestamp = OpcUa_DateTime_UtcNow();
				tRequestHeader.AuthenticationToken.IdentifierType = OpcUa_IdentifierType_Numeric;
				tRequestHeader.AuthenticationToken.NamespaceIndex = 0;
				OpcUa_NodeId_CopyTo(pSession->GetAuthenticationToken(), &(tRequestHeader.AuthenticationToken));
				tRequestHeader.RequestHandle = CClientApplication::m_uiRequestHandle++;
				// Let's make the asynchronous call
				CChannel* pChannel = pSession->GetChannel();
				if (pChannel)
				{
					OpcUa_Channel aChannel = pChannel->GetInternalHandle();
					uStatus = OpcUa_ClientApi_Call(
						aChannel, 
						&tRequestHeader, 
						iNoOfMethodsToCall, 
						pMethodsToCall, 
						pResponseHeader, 
						&NoOfResult, 
						ppResults, 
						&nNoOfDiagnosticInfos, 
						&pDiagnosticInfos);
				}
			}
		}
	}
	return uStatus;
}
// Recupère la session a laquelle appartient cette souscription
OpcUa_StatusCode OpenOpcUa_GetSessionOfSubscription(OpcUa_Handle hSubscription,OpcUa_Handle* hSession)
{
	OpcUa_StatusCode uStatus=OpcUa_BadInternalError;
	OpenOpcUa_HandleType aHandleType;
	uStatus=OpenOpcUa_WhatIsIt(hSubscription,&aHandleType);
	if (aHandleType==OPENOPCUA_SUBSCRIPTION)
	{
		CSubscriptionClient* pSubscriptionClient=(CSubscriptionClient*)hSubscription;
		CSessionClient *pSession=pSubscriptionClient->GetSession();
		if (pSession)
				  *hSession=(OpcUa_Handle)pSession;
	}
	else
	{
		*hSession=OpcUa_Null;
		uStatus=OpcUa_BadInvalidArgument;
	}
	return uStatus;
}
///////////////////////////////////////////////
// Callback functions
OpcUa_StatusCode OpenOpcUa_SetPublishCallback(OpcUa_Handle hApplication,
																	OpcUa_Handle hSession,
																	OpcUa_Handle hSubscription,
											  PFUNC lpCallback,
											  void* pCallbackData)
{
	OpcUa_StatusCode uStatus=OpcUa_BadInternalError;
	if (g_bAbstractionLayerInitialized)
	{
		CClientApplication* pUaClientApplication=(CClientApplication*)hApplication;
		if (pUaClientApplication)
		{
			CSessionClient* pSession=(CSessionClient*)hSession;
			if (pSession)
			{
				CSubscriptionClient* pSubscriptionClient=(CSubscriptionClient*)hSubscription;
				if (pSubscriptionClient)
				{
					pSubscriptionClient->SetNotificationCallback(lpCallback);
					pSubscriptionClient->SetNotificationCallbackData(pCallbackData);
					uStatus=OpcUa_Good;
				}
			}
		}
	}
	return uStatus;
}
OpcUa_StatusCode OpenOpcUa_SetShutdownCallback(OpcUa_Handle hApplication,
												OpcUa_Handle hSession,
												PFUNCSHUTDOWN lpCallback, void* pCallbackData)
{
	OpcUa_StatusCode uStatus=OpcUa_BadInternalError;
	if (g_bAbstractionLayerInitialized)
	{
		CClientApplication* pUaClientApplication=(CClientApplication*)hApplication;
		if (pUaClientApplication)
		{
			CSessionClient* pSession=(CSessionClient*)hSession;
			if (pSession)
			{
				pSession->SetShutdownCallback(lpCallback);
				pSession->SetShutdownCallbackData(pCallbackData);
				uStatus=OpcUa_Good;
			}
		}
	}
	return uStatus;
}
OpcUa_StatusCode OpenOpcUa_ReleaseInternalNode(OpenOpcUa_InternalNode* pInternalNode)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	if (pInternalNode)
	{
		OpcUa_NodeId_Clear(&(pInternalNode->m_NodeId));
		OpcUa_String_Clear(&(pInternalNode->m_BrowseName));
		OpcUa_DataValue_Clear(&(pInternalNode->m_DataValue));
		OpcUa_DataValue_Clear(&(pInternalNode->m_DataValue));
		pInternalNode->m_hMonitoredItem=OpcUa_Null;
		if (pInternalNode->m_pMonitoredItemParam)
			OpcUa_Free(pInternalNode->m_pMonitoredItemParam);
		OpcUa_Free(pInternalNode);
	}
	else
		uStatus=OpcUa_BadInvalidArgument;
	return uStatus;
}
OpcUa_StatusCode OpenOpcUa_GetInternalNodeByMonitoredItemId(OpcUa_Handle hApplication,
	OpcUa_Handle hSession,
	OpcUa_Handle hSubscription,
	OpcUa_UInt32 MonitoredItemId,
	OpenOpcUa_InternalNode** pInternalNode)
{
	OpcUa_StatusCode uStatus = OpcUa_BadInternalError;
	if (g_bAbstractionLayerInitialized)
	{
		CClientApplication* pUaClientApplication = (CClientApplication*)hApplication;
		if (pUaClientApplication)
		{
			CSessionClient* pSession = (CSessionClient*)hSession;
			if (pSession)
			{
				// pSession->FindSubscription(hSubscription,&pSubscriptionClient); This call can also works
				CSubscriptionClient* pSubscriptionClient = (CSubscriptionClient*)hSubscription;
				if (pSubscriptionClient)
				{
					CMonitoredItemClient* pMonitoredItemClient =pSubscriptionClient->FindMonitoredItemById(MonitoredItemId);
					if (pMonitoredItemClient)
					{
						*pInternalNode = (OpenOpcUa_InternalNode*)OpcUa_Alloc(sizeof(OpenOpcUa_InternalNode));
						if (*pInternalNode)
						{
							// NodeId
							OpcUa_NodeId_Initialize(&((*pInternalNode)->m_NodeId));
							OpcUa_NodeId aNodeId = pMonitoredItemClient->GetNodeId();
							OpcUa_NodeId_CopyTo(&aNodeId, &((*pInternalNode)->m_NodeId));

							// BrowseName
							OpcUa_String_Initialize(&((*pInternalNode)->m_BrowseName));
							OpcUa_String* aString = pMonitoredItemClient->GetMonitoredItemName();
							if (OpcUa_String_StrLen(aString)>0)
							{
								OpcUa_String_StrnCpy(&((*pInternalNode)->m_BrowseName), aString, OpcUa_String_StrLen(aString));
							}
							// DataValue
							OpcUa_DataValue_Initialize(&((*pInternalNode)->m_DataValue));
							OpcUa_DataValue* pDataValue = pMonitoredItemClient->GetValue();
							(*pInternalNode)->m_DataValue = *Utils::Copy(pDataValue);
							// OpcUa_Handle
							(*pInternalNode)->m_hMonitoredItem = (OpcUa_Handle)pMonitoredItemClient->GetClientHandle();
							// Let's fill the OpenOpcUa_MonitoredItemParam* but first we need to allocate it
							(*pInternalNode)->m_pMonitoredItemParam = (OpenOpcUa_MonitoredItemParam*)OpcUa_Alloc(sizeof(OpenOpcUa_MonitoredItemParam));
							if ((*pInternalNode)->m_pMonitoredItemParam)
							{
								// TimestampsToReturn
								(*pInternalNode)->m_pMonitoredItemParam->m_aTimestampsToReturn = (OpcUa_Byte)pMonitoredItemClient->GetTimestampsToReturn();
								// DiscardOldest
								(*pInternalNode)->m_pMonitoredItemParam->m_bDiscardOldest = pMonitoredItemClient->IsDiscardOldest();
								// SamplingInterval
								(*pInternalNode)->m_pMonitoredItemParam->m_dblSamplingInterval = pMonitoredItemClient->GetSamplingInterval();
								// Deadband
								OpcUa_ExtensionObject* pExtensionObject = pMonitoredItemClient->GetFilterToUse();
								if (pExtensionObject)
								{
									// Deadband Default value. 
									(*pInternalNode)->m_pMonitoredItemParam->m_DeadbandType = OpcUa_DeadbandType_None;
									(*pInternalNode)->m_pMonitoredItemParam->m_dblDeadbandValue = 0;
									(*pInternalNode)->m_pMonitoredItemParam->m_DataChangeTrigger = OpcUa_DataChangeTrigger_StatusValueTimestamp;
									if (pExtensionObject->Body.EncodeableObject.Type)
									{
										if (pExtensionObject->Body.EncodeableObject.Type->TypeId == OpcUaId_DataChangeFilter)
										{
											OpcUa_DataChangeFilter* pDataChangeFilter =
												(OpcUa_DataChangeFilter*)pExtensionObject->Body.EncodeableObject.Object;
											if (pDataChangeFilter)
											{
												(*pInternalNode)->m_pMonitoredItemParam->m_DeadbandType = pDataChangeFilter->DeadbandType;
												(*pInternalNode)->m_pMonitoredItemParam->m_dblDeadbandValue = pDataChangeFilter->DeadbandValue;
												(*pInternalNode)->m_pMonitoredItemParam->m_DataChangeTrigger = pDataChangeFilter->Trigger;
											}
										}
									}
								}
								// QueueSize
								(*pInternalNode)->m_pMonitoredItemParam->m_uiQueueSize = pMonitoredItemClient->GetQueueSize();
								uStatus = OpcUa_Good;
							}
							else
								uStatus = OpcUa_BadOutOfMemory;
						}
						else
							uStatus = OpcUa_BadOutOfMemory;
					}
				}
			}
		}
	}
	return uStatus;
}
OpcUa_StatusCode OpenOpcUa_GetInternalNodeByClientHandle(OpcUa_Handle hApplication,
	OpcUa_Handle hSession,
	OpcUa_Handle hSubscription,
	OpcUa_UInt32 ClientHandle,
	OpenOpcUa_InternalNode** pInternalNode)
{
	OpcUa_StatusCode uStatus = OpcUa_BadInternalError;
	if (g_bAbstractionLayerInitialized)
	{
		CClientApplication* pUaClientApplication = (CClientApplication*)hApplication;
		if (pUaClientApplication)
		{
			CSessionClient* pSession = (CSessionClient*)hSession;
			if (pSession)
			{
				// pSession->FindSubscription(hSubscription,&pSubscriptionClient); This call can also works
				CSubscriptionClient* pSubscriptionClient = (CSubscriptionClient*)hSubscription;
				if (pSubscriptionClient)
				{
					CMonitoredItemClient* pMonitoredItemClient = pSubscriptionClient->FindMonitoredItemByHandle(ClientHandle);
					if (pMonitoredItemClient)
					{
						*pInternalNode = (OpenOpcUa_InternalNode*)OpcUa_Alloc(sizeof(OpenOpcUa_InternalNode));
						if (*pInternalNode)
						{
							// NodeId
							OpcUa_NodeId_Initialize(&((*pInternalNode)->m_NodeId));
							OpcUa_NodeId aNodeId = pMonitoredItemClient->GetNodeId();
							OpcUa_NodeId_CopyTo(&aNodeId, &((*pInternalNode)->m_NodeId));

							// BrowseName
							OpcUa_String_Initialize(&((*pInternalNode)->m_BrowseName));
							OpcUa_String* aString = pMonitoredItemClient->GetMonitoredItemName();
							if (OpcUa_String_StrLen(aString)>0)
							{
								OpcUa_String_StrnCpy(&((*pInternalNode)->m_BrowseName), aString, OpcUa_String_StrLen(aString));
							}
							// DataValue
							OpcUa_DataValue_Initialize(&((*pInternalNode)->m_DataValue));
							OpcUa_DataValue* pDataValue = pMonitoredItemClient->GetValue();
							(*pInternalNode)->m_DataValue = *Utils::Copy(pDataValue);
							// OpcUa_Handle
							(*pInternalNode)->m_hMonitoredItem = (OpcUa_Handle)pMonitoredItemClient->GetClientHandle();
							// Let's fill the OpenOpcUa_MonitoredItemParam* but first we need to allocate it
							(*pInternalNode)->m_pMonitoredItemParam = (OpenOpcUa_MonitoredItemParam*)OpcUa_Alloc(sizeof(OpenOpcUa_MonitoredItemParam));
							if ((*pInternalNode)->m_pMonitoredItemParam)
							{
								// TimestampsToReturn
								(*pInternalNode)->m_pMonitoredItemParam->m_aTimestampsToReturn = (OpcUa_Byte)pMonitoredItemClient->GetTimestampsToReturn();
								// DiscardOldest
								(*pInternalNode)->m_pMonitoredItemParam->m_bDiscardOldest = pMonitoredItemClient->IsDiscardOldest();
								// SamplingInterval
								(*pInternalNode)->m_pMonitoredItemParam->m_dblSamplingInterval = pMonitoredItemClient->GetSamplingInterval();
								// Deadband
								OpcUa_ExtensionObject* pExtensionObject = pMonitoredItemClient->GetFilterToUse();
								if (pExtensionObject)
								{
									// Deadband Default value. 
									(*pInternalNode)->m_pMonitoredItemParam->m_DeadbandType = OpcUa_DeadbandType_None;
									(*pInternalNode)->m_pMonitoredItemParam->m_dblDeadbandValue = 0;
									(*pInternalNode)->m_pMonitoredItemParam->m_DataChangeTrigger = OpcUa_DataChangeTrigger_StatusValueTimestamp;
									if (pExtensionObject->Body.EncodeableObject.Type)
									{
										if (pExtensionObject->Body.EncodeableObject.Type->TypeId == OpcUaId_DataChangeFilter)
										{
											OpcUa_DataChangeFilter* pDataChangeFilter =
												(OpcUa_DataChangeFilter*)pExtensionObject->Body.EncodeableObject.Object;
											if (pDataChangeFilter)
											{
												(*pInternalNode)->m_pMonitoredItemParam->m_DeadbandType = pDataChangeFilter->DeadbandType;
												(*pInternalNode)->m_pMonitoredItemParam->m_dblDeadbandValue = pDataChangeFilter->DeadbandValue;
												(*pInternalNode)->m_pMonitoredItemParam->m_DataChangeTrigger = pDataChangeFilter->Trigger;
											}
										}
									}
								}
								// QueueSize
								(*pInternalNode)->m_pMonitoredItemParam->m_uiQueueSize = pMonitoredItemClient->GetQueueSize();
								uStatus = OpcUa_Good;
							}
							else
								uStatus = OpcUa_BadOutOfMemory;
						}
						else
							uStatus = OpcUa_BadOutOfMemory;
					}
				}
			}
		}
	}
	return uStatus;
}
/// <summary>
/// Opens the opc ua_ get internal node.
/// </summary>
/// <param name="hApplication">The h application.</param>
/// <param name="hSession">The h session.</param>
/// <param name="hSubscription">The h subscription.</param>
/// <param name="hMonitoredItem">The h monitored item.</param>
/// <param name="pInternalNode">The p internal node.</param>
/// <returns></returns>
OpcUa_StatusCode OpenOpcUa_GetInternalNode(OpcUa_Handle hApplication,
										   OpcUa_Handle hSession,
										   OpcUa_Handle hSubscription,
										   OpcUa_Handle hMonitoredItem,
										   OpenOpcUa_InternalNode** pInternalNode)
{
	OpcUa_StatusCode uStatus=OpcUa_BadInternalError;
	if (g_bAbstractionLayerInitialized)
	{
		CClientApplication* pUaClientApplication=(CClientApplication*)hApplication;
		if (pUaClientApplication)
		{
			CSessionClient* pSession=(CSessionClient*)hSession;
			if (pSession)
			{
				// pSession->FindSubscription(hSubscription,&pSubscriptionClient); This call can also works
				CSubscriptionClient* pSubscriptionClient = (CSubscriptionClient*)hSubscription;
				if (pSubscriptionClient)
				{
					CMonitoredItemClient* pMonitoredItemClient=(CMonitoredItemClient*)hMonitoredItem;
					if (pMonitoredItemClient)
					{
						*pInternalNode=( OpenOpcUa_InternalNode*)OpcUa_Alloc(sizeof(OpenOpcUa_InternalNode));
						if (*pInternalNode)
						{
							// NodeId
							OpcUa_NodeId_Initialize(&((*pInternalNode)->m_NodeId));
							OpcUa_NodeId aNodeId = pMonitoredItemClient->GetNodeId();
							OpcUa_NodeId_CopyTo(&aNodeId,&((*pInternalNode)->m_NodeId));
							
							// BrowseName
							OpcUa_String_Initialize(&((*pInternalNode)->m_BrowseName));
							OpcUa_String* pString=pMonitoredItemClient->GetMonitoredItemName();
							if (OpcUa_String_StrLen(pString)>0)
							{
								OpcUa_String_StrnCpy(&((*pInternalNode)->m_BrowseName),	pString,OpcUa_String_StrLen(pString));							
							}
							// DataValue
							OpcUa_DataValue_Initialize(&((*pInternalNode)->m_DataValue));
							OpcUa_DataValue* pDataValue=pMonitoredItemClient->GetValue();
							//(*pInternalNode)->m_DataValue=*Utils::Copy(pDataValue);
							OpcUa_DataValue_CopyTo(pDataValue, &((*pInternalNode)->m_DataValue));
							// OpcUa_Handle
							(*pInternalNode)->m_hMonitoredItem = (OpcUa_Handle)pMonitoredItemClient->GetClientHandle();
							// Let's fill the OpenOpcUa_MonitoredItemParam* but first we need to allocate it
							(*pInternalNode)->m_pMonitoredItemParam=(OpenOpcUa_MonitoredItemParam*)OpcUa_Alloc(sizeof(OpenOpcUa_MonitoredItemParam));
							if ((*pInternalNode)->m_pMonitoredItemParam)
							{
								// TimestampsToReturn
								(*pInternalNode)->m_pMonitoredItemParam->m_aTimestampsToReturn = (OpcUa_Byte)pMonitoredItemClient->GetTimestampsToReturn();
								// DiscardOldest
								(*pInternalNode)->m_pMonitoredItemParam->m_bDiscardOldest = pMonitoredItemClient->IsDiscardOldest();
								// SamplingInterval
								(*pInternalNode)->m_pMonitoredItemParam->m_dblSamplingInterval = pMonitoredItemClient->GetSamplingInterval();
								// Deadband
								OpcUa_ExtensionObject* pExtensionObject = pMonitoredItemClient->GetFilterToUse();
								{
									// Deadband Default value. 
									(*pInternalNode)->m_pMonitoredItemParam->m_DeadbandType = OpcUa_DeadbandType_None;
									(*pInternalNode)->m_pMonitoredItemParam->m_dblDeadbandValue = 0;
									(*pInternalNode)->m_pMonitoredItemParam->m_DataChangeTrigger = OpcUa_DataChangeTrigger_StatusValueTimestamp;
									if (pExtensionObject->Body.EncodeableObject.Type)
									{
										if (pExtensionObject->Body.EncodeableObject.Type->TypeId == OpcUaId_DataChangeFilter)
										{
											OpcUa_DataChangeFilter* pDataChangeFilter =
												(OpcUa_DataChangeFilter*)pExtensionObject->Body.EncodeableObject.Object;
											if (pDataChangeFilter)
											{
												(*pInternalNode)->m_pMonitoredItemParam->m_DeadbandType = pDataChangeFilter->DeadbandType;
												(*pInternalNode)->m_pMonitoredItemParam->m_dblDeadbandValue = pDataChangeFilter->DeadbandValue;
												(*pInternalNode)->m_pMonitoredItemParam->m_DataChangeTrigger = pDataChangeFilter->Trigger;
											}
										}
									}
								}
								// QueueSize
								(*pInternalNode)->m_pMonitoredItemParam->m_uiQueueSize = pMonitoredItemClient->GetQueueSize();
								uStatus = OpcUa_Good;
							}
							else
								uStatus=OpcUa_BadOutOfMemory;
						}
						else
							uStatus=OpcUa_BadOutOfMemory;
					}
				}
			}
		}
	}
	return uStatus;
}
OpcUa_StatusCode OpenOpcUa_WhatIsIt(OpcUa_Handle hHandle, OpenOpcUa_HandleType* aHandleType)
{
	OpcUa_StatusCode uStatus=OpcUa_BadInvalidArgument;
	*aHandleType=OPENOPCUA_UNKNOWN;
	if (hHandle)
	{
		// Check s'il s'agit d'une Application
		for (OpcUa_UInt32 ii=0;ii<g_pUaClientApplicationList.size();ii++)
		{
			CClientApplication* pApplication = g_pUaClientApplicationList.at(ii);
			if (pApplication==(CClientApplication*)hHandle)
			{
				*aHandleType=OPENOPCUA_APPLICATION;
				uStatus=OpcUa_Good;
				break;
			}
		}
		if (uStatus!=OpcUa_Good)
		{
			// check s'il s'agit d'une session
			for (OpcUa_UInt32 ii=0;ii<g_pUaClientApplicationList.size();ii++)
			{
				CClientApplication* pApplication = g_pUaClientApplicationList.at(ii);
				CSessionClient* pSession=OpcUa_Null;
				uStatus=pApplication->GetSession(hHandle,&pSession);
				if (uStatus==OpcUa_Good)
				{
					*aHandleType=OPENOPCUA_SESSION;
					break;
				}
			}
			if (uStatus!=OpcUa_Good)
			{
				// check s'il s'agit d'une souscription
				for (OpcUa_UInt32 ii=0;ii<g_pUaClientApplicationList.size();ii++)
				{
					CClientApplication* pApplication = g_pUaClientApplicationList.at(ii);
					CSubscriptionClient* pSubscription=OpcUa_Null;
					uStatus=pApplication->GetSubscription(hHandle,&pSubscription);
					if (uStatus==OpcUa_Good)
					{
						*aHandleType=OPENOPCUA_SUBSCRIPTION;
						break;
					}
				}
			}			
		}
	
	}
	return uStatus;
}
///////////////////////////////////////////////
// Helper function
OpcUa_StatusCode OpenOpcUa_VariantToString(OpcUa_Variant& Var,OpcUa_String** strValue)
{
	OpcUa_StatusCode uStatus=OpcUa_BadInternalError;
	if (strValue)
	{
		uStatus=Utils::OpcUaVariantToString(Var,strValue);
	}
	else
		uStatus=OpcUa_BadInvalidArgument;

	return uStatus;
}
OpcUa_StatusCode OpenOpcUa_DateTimeToString(OpcUa_DateTime aDateTime,OpcUa_String** strTime)
{
	OpcUa_StatusCode uStatus=OpcUa_BadInternalError;
	uStatus=Utils::OpcUaDateTimeToString(aDateTime,strTime);

	return uStatus;
}
// Transforme un chaine en nodeId.
// pour que la transformation fonctionne la chaine doit avoir la forme ns=x;i=yyy ou ns=x;s=zzz
OpcUa_StatusCode OpenOpcUa_StringToNodeId(OpcUa_String strNodeId,OpcUa_NodeId* pNodeId)
{	
	OpcUa_StatusCode uStatus=OpcUa_BadInternalError;
	if (!pNodeId)
		uStatus=OpcUa_BadInvalidArgument;
	else
	{
		OpcUa_NodeId_Initialize(pNodeId);
		OpcUa_UInt32 iNs=0,iId=0;
		char* pBuff=OpcUa_String_GetRawString(&strNodeId);
		int iRes=sscanf(pBuff,"ns=%u;i=%u",(unsigned int*)&iNs,(unsigned int*)&iId);
		if (iRes==2)
		{
			pNodeId->IdentifierType=OpcUa_IdentifierType_Numeric;
			pNodeId->NamespaceIndex=(OpcUa_UInt16)iNs;
			pNodeId->Identifier.Numeric=iId;
			uStatus=OpcUa_Good;
		}
		else
		{
			if (iRes==1)
			{
				// probably another format
				OpcUa_CharA* strId=(OpcUa_CharA*)OpcUa_Alloc(256);
				if (strId)
				{
					char* ptr = strchr(pBuff,';');
					char* subStr=strchr(ptr,'=');
					ZeroMemory(strId,256);
					int iRes=sscanf(pBuff,"ns=%u;s=%255s",(unsigned int*)&iNs,strId);
					if (iRes==2)
					{
						pNodeId->IdentifierType=OpcUa_IdentifierType_String;
						pNodeId->NamespaceIndex=(OpcUa_UInt16)iNs;
						uStatus=OpcUa_String_AttachCopy(&(pNodeId->Identifier.String),++subStr);
					}
					else
						uStatus=OpcUa_BadInvalidArgument;
					OpcUa_Free(strId);
				}
			}
		}
	}
	return uStatus;
}

// Transforme un NodeId en chaine
OpcUa_StatusCode OpenOpcUa_NodeIdToString(OpcUa_NodeId aNodeId,OpcUa_String* strNodeId)
{
	OpcUa_StatusCode uStatus=OpcUa_BadInternalError;
	if (strNodeId)
	{
		switch (aNodeId.IdentifierType)
		{
			case OpcUa_IdentifierType_Numeric: // Numeric
			{
				char* buffer=(char*)malloc(20);
				if (buffer)
				{
					memset(buffer, 0, 20);
					sprintf(buffer, "ns=%u;i=%u", aNodeId.NamespaceIndex, (unsigned int)aNodeId.Identifier.Numeric);
					OpcUa_String_AttachCopy(strNodeId, buffer);
					uStatus = OpcUa_Good;
					memset(buffer, 0, 20);
					free(buffer);
				}
				else
					uStatus = OpcUa_BadOutOfMemory;
			}
			break;
			case OpcUa_IdentifierType_String: // string
			{
				char* buffer=(char*)malloc(512);
				if (buffer)
				{
					ZeroMemory(buffer, 512);
					sprintf(buffer, "ns=%u;s=%s", aNodeId.NamespaceIndex, OpcUa_String_GetRawString(&(aNodeId.Identifier.String)));
					OpcUa_String_AttachCopy(strNodeId, buffer);
					uStatus = OpcUa_Good;
					ZeroMemory(buffer, 512);
					free(buffer);
				}
				else
					uStatus = OpcUa_BadOutOfMemory;
			}
			break;
			case OpcUa_IdentifierType_Guid: // guid
			{
				uStatus=OpcUa_BadNotSupported;
			}
			break;
			case OpcUa_IdentifierType_Opaque: // opaque (ByteString)
			{
				uStatus=OpcUa_BadNotSupported;
			}
			break;
			default:
				uStatus=OpcUa_BadInvalidArgument;
				break;
		}
	}
	else
		uStatus=OpcUa_BadInvalidArgument;
	return uStatus;
}

OpcUa_StatusCode OpenOpcUa_Browse(OpcUa_Handle hApplication,
								  OpcUa_Handle hSession,
								  OpcUa_Int32 iNoOfNodesToBrowse,
								  const OpcUa_BrowseDescription* pNodesToBrowse,
								  OpcUa_Int32* iNoOfReferenceDescription,
								  OpcUa_ReferenceDescription** pReferenceList)
{
	OpcUa_StatusCode uStatus=OpcUa_BadInternalError;
	if (g_bAbstractionLayerInitialized)
	{
		CClientApplication* pUaClientApplication=(CClientApplication*)hApplication;
		if (pUaClientApplication)
		{
			CSessionClient* pSession=(CSessionClient*)hSession;
			if (pSession)
			{
				OpcUa_ReferenceDescriptionList aReferences;
				uStatus=pSession->Browse(iNoOfNodesToBrowse,pNodesToBrowse,&aReferences);
				if (uStatus==OpcUa_Good)
				{
					*iNoOfReferenceDescription=aReferences.size();
					if (aReferences.size() > 0)
					{
						*pReferenceList = (OpcUa_ReferenceDescription*)OpcUa_Alloc(aReferences.size()*sizeof(OpcUa_ReferenceDescription));
						for (OpcUa_UInt32 ii = 0; ii < aReferences.size(); ii++)
						{
							OpcUa_ReferenceDescription* pReferenceDescription = aReferences.at(ii);
							if (pReferenceDescription)
							{
								OpcUa_ReferenceDescription_Initialize(&((*pReferenceList)[ii]));
								OpcUa_QualifiedName_CopyTo(&(pReferenceDescription->BrowseName), &((*pReferenceList)[ii].BrowseName));
								OpcUa_LocalizedText_CopyTo(&(pReferenceDescription->DisplayName), &((*pReferenceList)[ii].DisplayName));
								(*pReferenceList)[ii].IsForward = pReferenceDescription->IsForward;
								(*pReferenceList)[ii].NodeClass = pReferenceDescription->NodeClass;
								OpcUa_ExpandedNodeId_CopyTo(&(pReferenceDescription->NodeId), &((*pReferenceList)[ii].NodeId));
								OpcUa_NodeId_CopyTo(&(pReferenceDescription->ReferenceTypeId), &((*pReferenceList)[ii].ReferenceTypeId));
								OpcUa_ExpandedNodeId_CopyTo(&(pReferenceDescription->TypeDefinition), &((*pReferenceList)[ii].TypeDefinition));
							}
						}
					}
					OpcUa_ReferenceDescriptionList::iterator it;
					while (!aReferences.empty())
					{
						it=aReferences.begin();
						OpcUa_ReferenceDescription* pReferenceDescription=*it;
						OpcUa_ReferenceDescription_Clear(pReferenceDescription);
						delete pReferenceDescription;
						aReferences.erase(it);
					}
				}
			}
		}
	}
	return uStatus;
}
//#define OPCUA_TRACE_LEVEL_ERROR         0x00000020 /* in-system errors, which require bugfixing        */
//#define OPCUA_TRACE_LEVEL_WARNING       0x00000010 /* in-system warnings and extern errors             */
//#define OPCUA_TRACE_LEVEL_SYSTEM        0x00000008 /* rare system messages (start, stop, connect)      */
//#define OPCUA_TRACE_LEVEL_INFO          0x00000004 /* more detailed information about system events    */
//#define OPCUA_TRACE_LEVEL_DEBUG         0x00000002 /* information needed for debug reasons             */
//#define OPCUA_TRACE_LEVEL_CONTENT       0x00000001 /* all message content */
OpcUa_StatusCode OpenOpcUa_Trace(OpcUa_Handle hApplication, OpcUa_UInt32 a_uTraceLevel, const OpcUa_CharA* format,...)
{
	OpcUa_StatusCode uStatus=OpcUa_BadInternalError;
	char* outBuffer=(char*)malloc(1024);
	ZeroMemory(outBuffer, 1024);	
	char* outTmpBuffer = (char*)malloc(1024);
	ZeroMemory(outTmpBuffer, 1024);
	if (g_bAbstractionLayerInitialized)
	{
		CClientApplication* pUaClientApplication=(CClientApplication*)hApplication;
		if (pUaClientApplication)
		{
			va_list args;
			va_start(args, format);
			//char* tmpStr=va_arg( args, char* );
			
			CLoggerMessage* pLoggerMessage=new CLoggerMessage();
			pLoggerMessage->m_pString=(OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
			// recherche des caractères  % dans format afin de déterminer ce qui vient de nous être passé
			if (format)
			{
				int iSize=strlen(format);
				// get first position of %
				int iFirstPos=-1;
				//save the first pos
				memcpy(outBuffer, format, iSize);
				
				for (int ii=0;ii<iSize;ii++)
				{
					if (format[ii]==0x25)
					{
						if (iFirstPos==-1)
							iFirstPos=ii;
						// analyse du contenu de l'octet suivant
						switch (format[ii+1])
						{
							case 'u':
							{
								unsigned int uiVal=va_arg(args,unsigned int);
								// Concatenation dans l'outBuffer
								sprintf(outTmpBuffer, "%s %u", outBuffer, uiVal);
							}
							break;
							case 'i':
							case 'd':
							{
								int uiVal=va_arg(args,int);
								// Concatenation dans l'outBuffer
								sprintf(outTmpBuffer, "%s %u", outBuffer, uiVal);
							}
							break;
							case 'e':
							case 'f':
							{
								double fltVal=va_arg(args,double);
								sprintf(outTmpBuffer, "%s %f", outBuffer, fltVal);
							}
							break;
							case 's':
							{
								char* cVal=va_arg(args,char*);
								// Concatenation dans l'outBuffer
								sprintf(outTmpBuffer, "%s %s", outBuffer, cVal);
							}
							break;
							case 'X':
							case 'x':
							{
								unsigned int uiVal=va_arg(args,unsigned int);
								// Concatenation dans l'outBuffer
								sprintf(outTmpBuffer, "%s %x", outBuffer, uiVal);
							}
							break;
							default:
								break;
						}
						strcpy(outBuffer, outTmpBuffer);
					}
				}
			}			
			OpcUa_String_AttachCopy(pLoggerMessage->m_pString,outBuffer);
			pLoggerMessage->m_uiLevel=a_uTraceLevel;
			pUaClientApplication->AddLoggerMessage(pLoggerMessage);
			va_end(args);
		}
	}
	free(outBuffer);
	free(outTmpBuffer);
	return uStatus;
}
// this function will provide the details for an attributes. This is not related to hApplication or hSession 
// This function is just an helper
OpcUa_StatusCode OpenOpcUa_GetAttributeDetails(OpcUa_Int32 iAttributeId, OpcUa_String** szAttributeName, OpcUa_String** szAttributeDescription)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	if (szAttributeName && szAttributeDescription)
	{
		if ( (iAttributeId>0) && (iAttributeId<23) )
		{
			// AttributeName
			*szAttributeName=(OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
			OpcUa_String_Initialize(*szAttributeName);
			OpcUa_String_AttachCopy(*szAttributeName, OpcUa_Attributes[iAttributeId-1].szAttribute);
			// Attribute description
			*szAttributeDescription=(OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
			OpcUa_String_Initialize(*szAttributeDescription);
			OpcUa_String_AttachCopy(*szAttributeDescription, OpcUa_Attributes[iAttributeId-1].szAttributeText);
		}
		else
			uStatus=OpcUa_BadInvalidArgument;
	}
	else
		uStatus=OpcUa_BadInvalidArgument;
	return uStatus;
}

// Function name   : OpenOpcUa_GetBuiltInTypeDetails
// Description     : 
// This function will provide detail for BuiltInTypes. This is just an helper function
// Application have to release allocated ressources, szBuiltInTypeName, szBuiltInTypeDescription
// Return type     : OpcUa_StatusCode 
// Argument        : OpcUa_Int32 iBuiltInTypeId
// Argument        : OpcUa_String** szBuiltInTypeName output param
// Argument        : OpcUa_String** szBuiltInTypeDescription output param

OpcUa_StatusCode OpenOpcUa_GetBuiltInTypeDetails(OpcUa_Int32 iBuiltInTypeId, OpcUa_String** szBuiltInTypeName, OpcUa_String** szBuiltInTypeDescription)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	if (szBuiltInTypeName && szBuiltInTypeDescription)
	{
		if ( (iBuiltInTypeId>=0) && (iBuiltInTypeId<26) )
		{
			// BuilInTypeName
			*szBuiltInTypeName=(OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
			OpcUa_String_Initialize(*szBuiltInTypeName);
			OpcUa_String_AttachCopy(*szBuiltInTypeName, OpcUa_BuiltInTypes[iBuiltInTypeId].szBuilInType);
			// Attribute description
			*szBuiltInTypeDescription=(OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
			OpcUa_String_Initialize(*szBuiltInTypeDescription);
			OpcUa_String_AttachCopy(*szBuiltInTypeDescription, OpcUa_BuiltInTypes[iBuiltInTypeId].szBuilInTypeText);
		}
		else
			uStatus=OpcUa_BadInvalidArgument;
	}
	else
		uStatus=OpcUa_BadInvalidArgument;
	return uStatus;
}
// Get the NodeClassName based ont the NodeClassId
OpcUa_StatusCode OpenOpcUa_GetNodeClassDetails(OpcUa_Int32 iNodeClass, OpcUa_String** szNodeClassName)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	*szNodeClassName=(OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(*szNodeClassName);
	switch (iNodeClass)
	{
	case 0:
		OpcUa_String_AttachCopy(*szNodeClassName,"Unspecified");
		break;
	case 1:
		OpcUa_String_AttachCopy(*szNodeClassName,"Object");
		break;
	case 2:
		OpcUa_String_AttachCopy(*szNodeClassName,"Variable");
		break;
	case 4:
		OpcUa_String_AttachCopy(*szNodeClassName,"Method");
		break;
	case 8:
		OpcUa_String_AttachCopy(*szNodeClassName,"ObjectType");
		break;
	case 16:
		OpcUa_String_AttachCopy(*szNodeClassName,"VariableType");
		break;
	case 32:
		OpcUa_String_AttachCopy(*szNodeClassName,"ReferenceType");
		break;
	case 64:
		OpcUa_String_AttachCopy(*szNodeClassName,"DataType");
		break;
	case 128:
		OpcUa_String_AttachCopy(*szNodeClassName,"View");
		break;
	default:
		uStatus=OpcUa_BadInvalidArgument;
		OpcUa_String_Clear(*szNodeClassName);
		break;
	}
	return uStatus;
}
OpcUa_StatusCode OpenOpcUa_GetAccessLevel(OpcUa_Byte AccessLevel, OpcUa_String** szAccessLevel)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	*szAccessLevel = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	if (*szAccessLevel)
	{
		OpcUa_String StreamAccess, HistoryAccess;
		OpcUa_String_Initialize(&StreamAccess);
		OpcUa_String_Initialize(&HistoryAccess);
		OpcUa_String_Initialize((*szAccessLevel));
		if (AccessLevel == OpcUa_AccessLevels_None)
			OpcUa_String_AttachCopy(&StreamAccess, "None");
		if ((AccessLevel&OpcUa_AccessLevels_CurrentRead) == OpcUa_AccessLevels_CurrentRead)
			OpcUa_String_AttachCopy(&StreamAccess, "Read");
		if ((AccessLevel&OpcUa_AccessLevels_CurrentWrite) == OpcUa_AccessLevels_CurrentWrite)
			OpcUa_String_AttachCopy(&StreamAccess, "Write");
		if ((AccessLevel&OpcUa_AccessLevels_CurrentReadOrWrite) == OpcUa_AccessLevels_CurrentReadOrWrite)
			OpcUa_String_AttachCopy(&StreamAccess, "Read or Write");

		if ((AccessLevel&OpcUa_AccessLevels_HistoryRead) == OpcUa_AccessLevels_HistoryRead)
			OpcUa_String_AttachCopy(&HistoryAccess, "History Read");
		if ((AccessLevel&OpcUa_AccessLevels_HistoryWrite) == OpcUa_AccessLevels_HistoryWrite)
			OpcUa_String_AttachCopy(&HistoryAccess, "History Write");
		if ((AccessLevel&OpcUa_AccessLevels_HistoryReadOrWrite) == OpcUa_AccessLevels_HistoryReadOrWrite)
			OpcUa_String_AttachCopy(&HistoryAccess, "History Read and Write");
		OpcUa_String_StrnCpy((*szAccessLevel), &StreamAccess, OpcUa_String_StrLen(&StreamAccess));
		if (OpcUa_String_StrLen(&HistoryAccess) > 0)
			OpcUa_String_StrnCat((*szAccessLevel), &HistoryAccess, OpcUa_String_StrLen(&HistoryAccess));
		OpcUa_String_Clear(&StreamAccess);
		OpcUa_String_Clear(&HistoryAccess);

	}
	return uStatus;
}
OpcUa_StatusCode OpenOpcUa_GetUserAccessLevel(OpcUa_Byte AccessLevel, OpcUa_String** szUserAccessLevel)
{
	return OpenOpcUa_GetAccessLevel(AccessLevel, szUserAccessLevel);
}
// Transform an OpcUa_String to OpcUa_Variant according to the requested DataType
OpcUa_StatusCode OpenOpcUa_StringToVariant(OpcUa_String* pString, OpcUa_Int32 iDataType,OpcUa_Variant** pVariant)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	if (pString)
	{
		if (OpcUa_String_StrLen(pString) > 0)
		{
			*pVariant = (OpcUa_Variant*)OpcUa_Alloc(sizeof(OpcUa_Variant));
			OpcUa_Variant_Initialize(*pVariant);
			switch (iDataType)
			{
			case OpcUaType_Boolean:
				(*pVariant)->Datatype = OpcUaType_Boolean;
				(*pVariant)->Value.Boolean = (OpcUa_Boolean)atoi(OpcUa_String_GetRawString(pString));
				break;
			case OpcUaType_Byte:
				(*pVariant)->Datatype = OpcUaType_Byte;
				(*pVariant)->Value.Byte = (OpcUa_Byte)atoi(OpcUa_String_GetRawString(pString));
				break;
			case OpcUaType_ByteString:
				break;
			case OpcUaType_DataValue:
				break;
			case OpcUaType_DateTime:
				break;
			case OpcUaType_DiagnosticInfo:
				uStatus = OpcUa_BadNotSupported;
				break;
			case OpcUaType_Double:
			{
				(*pVariant)->Datatype = OpcUaType_Double;
				OpcUa_Double dblVal = 0.0;
				if (sscanf(OpcUa_String_GetRawString(pString), "%lf", &dblVal) == 1)
					(*pVariant)->Value.Double = dblVal;
				else
					uStatus = OpcUa_BadInvalidArgument;
			}
				break;
			case OpcUaType_ExpandedNodeId:
				uStatus = OpcUa_BadNotSupported;
				break;
			case OpcUaType_ExtensionObject:
				uStatus = OpcUa_BadNotSupported;
				break;
			case OpcUaType_Float:
			{
				(*pVariant)->Datatype = OpcUaType_Float;
				OpcUa_Float fltVal = 0.0;
				if (sscanf(OpcUa_String_GetRawString(pString), "%f", &fltVal) == 1)
					(*pVariant)->Value.Float = fltVal;
				else
					uStatus = OpcUa_BadInvalidArgument;
			}
				break;
			case OpcUaType_Guid:
				uStatus = OpcUa_BadNotSupported;
				break;
			case OpcUaType_Int16:
				(*pVariant)->Datatype = OpcUaType_Int16;
				(*pVariant)->Value.Int16 = (OpcUa_Int16)atoi(OpcUa_String_GetRawString(pString));
				break;
			case OpcUaType_Int32:
				(*pVariant)->Datatype = OpcUaType_Int32;
				(*pVariant)->Value.Int32 = (OpcUa_Int32)atoi(OpcUa_String_GetRawString(pString));
				break;
			case OpcUaType_Int64:
				break;
			case OpcUaType_LocalizedText:
				break;
			case OpcUaType_NodeId:
				uStatus = OpcUa_BadNotSupported;
				break;
			case OpcUaType_Null:
				uStatus = OpcUa_BadNotSupported;
				break;
			case OpcUaType_QualifiedName:
				break;
			case OpcUaType_SByte:
				break;
			case OpcUaType_StatusCode:
				break;
			case OpcUaType_String:
				(*pVariant)->Datatype = OpcUaType_String;
				OpcUa_String_StrnCpy(&((*pVariant)->Value.String), pString, OpcUa_String_StrLen(pString));
				break;
			case OpcUaType_UInt16:
				(*pVariant)->Datatype = OpcUaType_UInt16;
				(*pVariant)->Value.UInt16 = (OpcUa_UInt16)atoi(OpcUa_String_GetRawString(pString));
				break;
			case OpcUaType_UInt32:
				(*pVariant)->Datatype = OpcUaType_UInt32;
				(*pVariant)->Value.UInt32 = (OpcUa_UInt32)atoi(OpcUa_String_GetRawString(pString));
				break;
			case OpcUaType_UInt64:
				break;
			case OpcUaType_Variant:
				uStatus = OpcUa_BadNotSupported;
				break;
			case OpcUaType_XmlElement:
				uStatus = OpcUa_BadNotSupported;
				break;
			default:
				uStatus = OpcUa_BadDataTypeIdUnknown;
			}
			if (uStatus != OpcUa_Good)
				OpcUa_Variant_Clear(*pVariant);
		}
		else
			uStatus = OpcUa_BadInvalidArgument;
	}
	else
		uStatus=OpcUa_BadInvalidArgument;
	return uStatus;
}

// Function for configuration file manipulation. Configuration file are conform to OpenOpcUaClientConfig.xsd
OpcUa_StatusCode OpenOpcUa_LoadConfig(OpcUa_Handle hApplication, OpcUa_String szConfigFileName)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	if (g_bAbstractionLayerInitialized)
	{
		CClientApplication* pUaClientApplication = (CClientApplication*)hApplication;
		if (pUaClientApplication)
		{
			OpcUa_CharA* localPath = OpcUa_Null;
			OpcUa_CharA* fileName = OpcUa_Null;
			OpcUa_CharA* chFileName = OpcUa_String_GetRawString(&szConfigFileName);
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
							uStatus = pUaClientApplication->LoadUaClientConfiguration(localPath, fileName);
							if (uStatus != OpcUa_Good)
								OpcUa_Trace(OPCUA_TRACE_CLIENT_LEVEL_ERROR, "Critical error>Cannot load ClientConfiguration file\n");
						}
						else
							OpcUa_Trace(OPCUA_TRACE_CLIENT_LEVEL_ERROR, "Critical error>Memory error, ClientConfiguration corrupted\n");
						if (fileName)
							OpcUa_Free(fileName);
					}
					if (localPath)
						OpcUa_Free(localPath);
				}
				else
				{
					OpcUa_Trace(OPCUA_TRACE_CLIENT_ERROR, "OpenOpcUa_LoadConfig failed. Not enough memory\n");
					uStatus = OpcUa_BadOutOfMemory;
				}
			}
			else
			{
				uStatus = OpcUa_BadFileNotFound;
				OpcUa_Trace(OPCUA_TRACE_CLIENT_LEVEL_ERROR, "Critical error>Full ClientConfiguration filename is corrupted\n");
			}
		}
		else
		{
			OpcUa_Trace(OPCUA_TRACE_CLIENT_ERROR, "Your Application handle is invalid\n");
			uStatus = OpcUa_BadInvalidArgument;
		}
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}
OpcUa_StatusCode OpenOpcUa_SaveConfig(OpcUa_Handle hApplication, OpcUa_String szConfigFileName)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	if (g_bAbstractionLayerInitialized)
	{
		CClientApplication* pUaClientApplication = (CClientApplication*)hApplication;
		if (pUaClientApplication)
		{
			OpcUa_CharA* localPath = OpcUa_Null;
			OpcUa_CharA* fileName = OpcUa_Null;
			OpcUa_CharA* chFileName = OpcUa_String_GetRawString(&szConfigFileName);
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
							uStatus=pUaClientApplication->SaveUaClientConfiguration(localPath, fileName);

						}
					}
				}
			}
		}
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}
// Retrive sessions for a hApplication
OpcUa_StatusCode OpenOpcUa_GetSessions(OpcUa_Handle hApplication,
	OpcUa_UInt32* uiNoOfSessions, // out
	OpcUa_Handle** hSessions) // out
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	if (g_bAbstractionLayerInitialized)
	{
		CClientApplication* pUaClientApplication = (CClientApplication*)hApplication;
		if (pUaClientApplication)
		{
			uStatus = pUaClientApplication->GetSessions(uiNoOfSessions,hSessions);
		}
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}
OpcUa_StatusCode OpenOpcUa_GetSubscriptions(OpcUa_Handle hApplication,
	OpcUa_Handle hSession,
	OpcUa_UInt32* uiNoOfSubscription, // out
	OpcUa_Handle** hSubscription) // out
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	if (g_bAbstractionLayerInitialized)
	{
		CClientApplication* pUaClientApplication = (CClientApplication*)hApplication;
		if (pUaClientApplication)
		{
			CSessionClient* pSession = (CSessionClient*)hSession;
			if (pSession)
			{
				uStatus=pSession->GetSubscriptions(uiNoOfSubscription,hSubscription);
			}
		}
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}
// retrieve the hMonitoredItems for a hSubscription
/// <summary>
/// Opens the opc ua_ get monitored items.
/// </summary>
/// <param name="hApplication">The h application.</param>
/// <param name="hSession">The h session.</param>
/// <param name="hSubscription">The h subscription.</param>
/// <param name="uiNoOfMonitoredItems">The UI no of monitored items.</param>
/// <param name="hMonitoredItems">hMonitoredItems is an input parameter it should be null in the call. It will be release by the caller</param>
/// <returns></returns>
OpcUa_StatusCode OpenOpcUa_GetMonitoredItems(OpcUa_Handle hApplication,
	OpcUa_Handle hSession,
	OpcUa_Handle hSubscription,
	OpcUa_UInt32* uiNoOfMonitoredItems, // out
	OpcUa_Handle** hMonitoredItems) // out
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	if (hMonitoredItems)
	{
		if (g_bAbstractionLayerInitialized)
		{
			CClientApplication* pUaClientApplication = (CClientApplication*)hApplication;
			if (pUaClientApplication)
			{
				CSessionClient* pSession = (CSessionClient*)hSession;
				if (pSession)
				{
					CSubscriptionClient* pSubscriptionClient = (CSubscriptionClient*)hSubscription;
					if (pSubscriptionClient)
					{

						uStatus = pSubscriptionClient->GetMonitoredItems(uiNoOfMonitoredItems, hMonitoredItems);
					}
				}
			}
		}
		else
			uStatus = OpcUa_BadInvalidArgument;
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}

// Retrieve the dataType of the UAVariable (aNodeId)  
OpcUa_StatusCode OpenOpcUa_GetUAVariableDatatype(OpcUa_Handle hApplication, OpcUa_Handle hSession, OpcUa_NodeId aNodeId, OpcUa_NodeId* pDataType)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_DataValue* pResults = OpcUa_Null;
	OpcUa_ReadValueId* pNodesToRead = OpcUa_Null;
	if (g_bAbstractionLayerInitialized)
	{
		CClientApplication* pUaClientApplication = (CClientApplication*)hApplication;
		if (pUaClientApplication)
		{
			OpcUa_Int32 iNodNodesToRead = 1;
			pNodesToRead = (OpcUa_ReadValueId*)OpcUa_Alloc(iNodNodesToRead*sizeof(OpcUa_ReadValueId));
			OpcUa_ReadValueId_Initialize(&pNodesToRead[0]);
			pNodesToRead[0].AttributeId = OpcUa_Attributes_DataType;
			pNodesToRead[0].NodeId = aNodeId;
			OpcUa_NodeId_CopyTo(&aNodeId, &(pNodesToRead[0].NodeId));
			uStatus = OpenOpcUa_ReadAttributes(hApplication, hSession, OpcUa_TimestampsToReturn_Both, iNodNodesToRead, pNodesToRead, &pResults);
			if (uStatus == OpcUa_Good)
			{
				// dans le cas du DataType le type est dans un NodeId
				if (pResults[0].StatusCode == OpcUa_Good)
				{
					if (!pDataType)
						pDataType=(OpcUa_NodeId*)OpcUa_Alloc(sizeof(OpcUa_NodeId));
					OpcUa_NodeId_Initialize(pDataType);
					OpcUa_NodeId_CopyTo(pResults[0].Value.Value.NodeId, pDataType);
				}
				if (pResults)
				{
					OpcUa_DataValue_Clear(pResults);
					OpcUa_Free(pResults);
				}
			}
			OpcUa_ReadValueId_Clear(pNodesToRead);
			OpcUa_Free(pNodesToRead);
		}
	}
	return uStatus;
}
// Caller must clear and release the extensionObject
OpcUa_StatusCode OpenOpcUa_CreateFilterObject(OpcUa_UInt32    DeadbandType,
	OpcUa_Double DeadbandValue,
	OpcUa_Byte   DatachangeTrigger,
	OpcUa_ExtensionObject** pExtensionObject)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	if (!(*pExtensionObject))
	{
		(*pExtensionObject) = (OpcUa_ExtensionObject*)OpcUa_Alloc(sizeof(OpcUa_ExtensionObject));
		OpcUa_ExtensionObject_Initialize((*pExtensionObject));
		(*pExtensionObject)->TypeId.NodeId.Identifier.Numeric = OpcUaId_DataChangeFilter_Encoding_DefaultBinary;
		(*pExtensionObject)->Encoding = OpcUa_ExtensionObjectEncoding_EncodeableObject;
		(*pExtensionObject)->Body.EncodeableObject.Type = (OpcUa_EncodeableType*)OpcUa_Alloc(sizeof(OpcUa_EncodeableType));
		(*pExtensionObject)->Body.EncodeableObject.Type = &OpcUa_DataChangeFilter_EncodeableType;
		//DataChangeFilter.Body.EncodeableObject.Type->TypeId = OpcUaId_DataChangeFilter;
		OpcUa_DataChangeFilter* pDataChangeFilter = (OpcUa_DataChangeFilter*)OpcUa_Alloc(sizeof(OpcUa_DataChangeFilter));
		pDataChangeFilter->DeadbandType = DeadbandType;
		pDataChangeFilter->DeadbandValue = DeadbandValue;
		pDataChangeFilter->Trigger = (OpcUa_DataChangeTrigger)DatachangeTrigger;
		(*pExtensionObject)->Body.EncodeableObject.Object = (void*)pDataChangeFilter;
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}
// Give a string representation of an StatusCode
OpcUa_StatusCode OpenOpcUa_StatusCodeToString(OpcUa_StatusCode inStatus, OpcUa_String** pszStatusCode)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	if (!(*pszStatusCode))
	{
		(*pszStatusCode) = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
		if ((*pszStatusCode))
		{
			OpcUa_String_Initialize((*pszStatusCode));
			switch (inStatus)
			{
				case OpcUa_GoodCallAgain:
					OpcUa_String_AttachCopy((*pszStatusCode), "GoodCallAgain");
					break;
				case OpcUa_GoodClamped:
					OpcUa_String_AttachCopy((*pszStatusCode), "GoodClamped");
					break;
				case OpcUa_GoodCommunicationEvent:
					OpcUa_String_AttachCopy((*pszStatusCode), "GoodCommunicationEvent");
					break;
				case OpcUa_GoodCompletesAsynchronously:
					OpcUa_String_AttachCopy((*pszStatusCode), "GoodCompletesAsynchronously");
					break;
				case OpcUa_GoodDataIgnored:
					OpcUa_String_AttachCopy((*pszStatusCode), "GoodDataIgnored");
					break;
				case OpcUa_GoodEntryInserted:
					OpcUa_String_AttachCopy((*pszStatusCode), "GoodEntryInserted");
					break;
				case OpcUa_GoodEntryReplaced:
					OpcUa_String_AttachCopy((*pszStatusCode), "GoodEntryReplaced");
					break;
				case OpcUa_GoodLocalOverride:
					OpcUa_String_AttachCopy((*pszStatusCode), "GoodLocalOverride");
					break;
				case OpcUa_GoodMoreData:
					OpcUa_String_AttachCopy((*pszStatusCode), "GoodMoreData");
					break;
				case OpcUa_GoodNoData:
					OpcUa_String_AttachCopy((*pszStatusCode), "GoodNoData");
					break;
				case OpcUa_GoodNonCriticalTimeout:
					OpcUa_String_AttachCopy((*pszStatusCode), "GoodNonCriticalTimeout");
					break;
				case OpcUa_GoodOverload:
					OpcUa_String_AttachCopy((*pszStatusCode), "GoodOverload");
					break;
				case OpcUa_GoodResultsMayBeIncomplete:
					OpcUa_String_AttachCopy((*pszStatusCode), "GoodResultsMayBeIncomplete");
					break;
				case OpcUa_GoodShutdownEvent:
					OpcUa_String_AttachCopy((*pszStatusCode), "GoodShutdownEvent");
					break;
				case OpcUa_GoodSubscriptionTransferred:
					OpcUa_String_AttachCopy((*pszStatusCode), "GoodSubscriptionTransferred");
					break;
				case OpcUa_GoodTimeout:
					OpcUa_String_AttachCopy((*pszStatusCode), "GoodTimeout");
					break;
				case OpcUa_Good:
					OpcUa_String_AttachCopy((*pszStatusCode), "Good");
					break;
				case OpcUa_Uncertain:
					OpcUa_String_AttachCopy((*pszStatusCode), "Uncertain");
					break;
				case OpcUa_BadTimeout:
					OpcUa_String_AttachCopy((*pszStatusCode), "BadTimeout");
					break;
				case OpcUa_BadCommunicationError:
					OpcUa_String_AttachCopy((*pszStatusCode), "BadCommunicationError");
					break;
				case OpcUa_BadConnectionClosed:
					OpcUa_String_AttachCopy((*pszStatusCode), "BadCertificateIssuerRevocationUnknown");
					break;
				case OpcUa_BadCertificateInvalid:
					OpcUa_String_AttachCopy((*pszStatusCode), "BadCertificateInvalid");
					break;
				case OpcUa_BadCertificateTimeInvalid:
					OpcUa_String_AttachCopy((*pszStatusCode), "BadCertificateTimeInvalid");
					break;
				case OpcUa_BadCertificateRevoked:
					OpcUa_String_AttachCopy((*pszStatusCode), "BadCertificateRevoked");
					break;
				case OpcUa_BadCertificateUntrusted:
					OpcUa_String_AttachCopy((*pszStatusCode), "BadCertificateUntrusted");
					break;
				case OpcUa_BadCertificateIssuerRevocationUnknown:
					OpcUa_String_AttachCopy((*pszStatusCode), "BadCertificateIssuerRevocationUnknown");
					break;
				case OpcUa_BadConnectionRejected:
					OpcUa_String_AttachCopy((*pszStatusCode), "BadConnectionRejected");
					break;
				default:
					{
						OpcUa_CharA* buf=(OpcUa_CharA*)OpcUa_Alloc(15);
						ZeroMemory(buf, 15);
						sprintf(buf, "0x%05x", (unsigned int)inStatus);
						OpcUa_String_AttachCopy((*pszStatusCode), buf);
						free(buf);
					}
					break;
			}
		}
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}