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

#pragma once
#include "ServiceModule.h"
typedef enum STARTUP_COMMAND
{
	RUN_SERVER,
	RUN_SERVICE,
	INSTALL_SERVICE,
	UNINSTALL_SERVICE,
	COMMAND_UNKNOWN
} _STARTUP_COMMAND;
// Ajoute un element de type ##name## dans aVariant.Value.Array.Value##name##Array
// l'element ajoué devra se trouver dans une variable appelée aNewVal
// aVariant
#define OpcUa_Array_AddElt(name)\
{\
	/* structure de stockage temporaire*/\
	std::vector<OpcUa_##name*> aTmpArray;\
	int iCurrentElt=aVariant.Value.Array.Length;\
	/* Taille total en octet*/ \
	for (OpcUa_Int32 ii=0;ii<iCurrentElt;ii++)\
	{\
		OpcUa_##name* pTmpElt=(OpcUa_##name*)OpcUa_Alloc(sizeof(OpcUa_##name));\
		OpcUa_##name##_Initialize(pTmpElt);\
		OpcUa_##name##_CopyTo(&(aVariant.Value.Array.Value.name##Array[ii]), pTmpElt); \
		aTmpArray.push_back(pTmpElt);\
		OpcUa_##name##_Clear(&(aVariant.Value.Array.Value.name##Array[ii]));\
	}\
	/*Ajout du nouveau element*/ \
	aTmpArray.push_back(pNewVal); \
	if (aVariant.Value.Array.Value.name##Array)\
	{\
		OpcUa_Free(aVariant.Value.Array.Value.name##Array);\
		aVariant.Value.Array.Value.name##Array=OpcUa_Null;\
	}\
	/*Allocation de la nouvelle taille*/ \
	aVariant.Value.Array.Value.name##Array=(OpcUa_##name*)OpcUa_Alloc((aTmpArray.size())*sizeof(OpcUa_##name)); \
	for (OpcUa_UInt16 iii=0;iii<aTmpArray.size();iii++)\
	{\
		OpcUa_##name* pVal1=aTmpArray.at(iii);\
		OpcUa_##name##_Initialize(&aVariant.Value.Array.Value.name##Array[iii]);\
		OpcUa_##name##_CopyTo(pVal1,&aVariant.Value.Array.Value.name##Array[iii]);\
		OpcUa_##name##_Clear(pVal1);\
		OpcUa_Free(pVal1);\
		pVal1=OpcUa_Null;\
	}\
	aVariant.Value.Array.Length=aTmpArray.size();\
	aTmpArray.clear();	\
}

#define OpcUa_ExtensionObject_Copy(name) \
{\
	OpcUa_ExtensionObject* pExtensionObj=pUAVariable->GetValue()->GetValue().Value.ExtensionObject; \
	OpcUa_##name* pSrc=(OpcUa_##name*)pExtensionObj->Body.EncodeableObject.Object; \
	OpcUa_##name* pTarget=Utils::Copy(pSrc); \
	 \
	pResponse->Results[ii].Value.Value.ExtensionObject=(OpcUa_ExtensionObject*)OpcUa_Alloc(sizeof(OpcUa_ExtensionObject)); \
	pResponse->Results[ii].Value.Datatype=OpcUaType_ExtensionObject; \
	pResponse->Results[ii].Value.Value.ExtensionObject->BodySize=0; \
	pResponse->Results[ii].Value.Value.ExtensionObject->Encoding=OpcUa_ExtensionObjectEncoding_EncodeableObject; \
	OpcUa_ExpandedNodeId_Initialize(&(pResponse->Results[ii].Value.Value.ExtensionObject->TypeId)); \
	pResponse->Results[ii].Value.Value.ExtensionObject->TypeId.ServerIndex=0; \
	OpcUa_String_Initialize(&(pResponse->Results[ii].Value.Value.ExtensionObject->TypeId.NamespaceUri)); \
	pResponse->Results[ii].Value.Value.ExtensionObject->TypeId.NodeId.Identifier.Numeric=pEncodeableType->BinaryEncodingTypeId; \
	pResponse->Results[ii].Value.Value.ExtensionObject->TypeId.NodeId.IdentifierType=OpcUa_IdentifierType_Numeric; \
	pResponse->Results[ii].Value.Value.ExtensionObject->TypeId.NodeId.NamespaceIndex=0; \
	pResponse->Results[ii].Value.Value.ExtensionObject->Body.EncodeableObject.Type=pEncodeableType; \
	pResponse->Results[ii].Value.Value.ExtensionObject->Body.EncodeableObject.Object=pTarget; \
}
#define OpcUa_ExtensionObject_Array_Copy(name) \
{\
	OpcUa_ExtensionObject* pExtensionObj=&(pUAVariable->GetValue()->GetValue().Value.Array.Value.ExtensionObjectArray[iii]); \
	OpcUa_##name* pSrc=(OpcUa_##name*)pExtensionObj->Body.EncodeableObject.Object; \
	OpcUa_##name* pTarget=Utils::Copy(pSrc); \
	pResponse->Results[ii].Value.Datatype=OpcUaType_ExtensionObject; \
	pResponse->Results[ii].Value.Value.Array.Value.ExtensionObjectArray[jj].BodySize=0; \
	pResponse->Results[ii].Value.Value.Array.Value.ExtensionObjectArray[jj].Encoding=OpcUa_ExtensionObjectEncoding_EncodeableObject; \
	OpcUa_ExpandedNodeId_Initialize(&(pResponse->Results[ii].Value.Value.Array.Value.ExtensionObjectArray[iii].TypeId)); \
	pResponse->Results[ii].Value.Value.Array.Value.ExtensionObjectArray[jj].TypeId.ServerIndex=0; \
	OpcUa_String_Initialize(&(pResponse->Results[ii].Value.Value.Array.Value.ExtensionObjectArray[jj].TypeId.NamespaceUri)); \
	pResponse->Results[ii].Value.Value.Array.Value.ExtensionObjectArray[jj].TypeId.NodeId.Identifier.Numeric=pEncodeableType->BinaryEncodingTypeId; \
	pResponse->Results[ii].Value.Value.Array.Value.ExtensionObjectArray[jj].TypeId.NodeId.IdentifierType=OpcUa_IdentifierType_Numeric; \
	pResponse->Results[ii].Value.Value.Array.Value.ExtensionObjectArray[jj].TypeId.NodeId.NamespaceIndex=0; \
	pResponse->Results[ii].Value.Value.Array.Value.ExtensionObjectArray[jj].Body.EncodeableObject.Type=pEncodeableType; \
	pResponse->Results[ii].Value.Value.Array.Value.ExtensionObjectArray[jj].Body.EncodeableObject.Object=(OpcUa_BuildInfo*)OpcUa_Alloc(pEncodeableType->AllocationSize); \
	pResponse->Results[ii].Value.Value.Array.Value.ExtensionObjectArray[jj].Body.EncodeableObject.Object=pTarget; 	\
}
///////////////////////////////////////////////////////////////////////////////////////
// structure userData associé au parsing aux fichiers xml de l'addressSpace (NodeSet)
typedef struct handler_Data 
{
	XML_ParserStruct*		XML_Parser;      
	void*					userData;
	OpcUa_StatusCode		uStatus; // Status code updated during the asynchronous load of the XML File
	CUAObject*				pUAObject;
	CUAObjectType*			pUAObjectType;
	CUAReferenceType*		pUAReferenceType;
	CUAVariableType*		pUAVariableType;
	CUAVariable*			pUAVariable; // All Variable: DataVariable and Property
	CUADataType*			pUADataType;
	CAlias*					pAlias;
	CUAMethod*				pMethod;
	CUAView*				pView;
	CUAReference*			pReferenceNode;
	OpcUa_NodeClass			NodeClass;
	CDataValue*				pDataValue; // Contiendra la valeur a charger. Cette valeur est issue du parsing <Value></Value>
	OpcUa_Boolean			bDisplayName; // Bool qui indique de l'on recevra un DisplayName pour la classe en cours
	OpcUa_Boolean			bDescription; // Bool qui indique de l'on recevra uns Description pour la classe en cours
	OpcUa_Boolean			bInverseName; // Bool qui indique de l'on recevra un InverseName pour la classe (CUAReferenceType) en cours
	OpcUa_Boolean			bValue; // Bool qui indique de l'on recevra un InverseName pour la classe (CUAVariable) ou (CUAVariableType) en cours
	OpcUa_Boolean			bNamespaceUri; // Bool qui indique que l'on est en train de traiter un NamespaceUri
	// Boolean commun pour l'initialisation des LocalizedText
	OpcUa_Boolean			bLocale;
	OpcUa_Boolean			bText;
	// Name of the current Xml Element handled by the parser
	char*					szCurrentXmlElement;
	// Prise en charge des balises element pour l'initialisation des extensionObjet

	// Partie commune a tous les ExtensionObject
	OpcUa_Boolean			bExtensionObject;
	OpcUa_Boolean			bExtensionObjectTypeId;
	OpcUa_Boolean			bExtensionObjectBody;
	// Partie specifique pour les extensions objects contenant un OpcUa_EnumValueType
	OpcUa_EnumValueType*	pEnumValueType;
	OpcUa_Boolean			bExtensionObjectEnumValueType;
	OpcUa_Boolean			bExtensionObjectEnumValueTypeValue;
	OpcUa_Boolean			bExtensionObjectEnumValueTypeDisplayName;
	OpcUa_Boolean			bExtensionObjectEnumValueTypeDescription;
	// Partie specifique pour lesextensions objects contenant un OpcUa_Argument
	OpcUa_Argument*			pArgument;
	OpcUa_Boolean			bExtensionObjectArgument;
	OpcUa_Boolean			bExtensionObjectName;
	OpcUa_Boolean			bExtensionObjectDataType;
	OpcUa_Boolean			bExtensionObjectIdentifier;
	OpcUa_Boolean			bExtensionObjectValueRank;
	OpcUa_Boolean			bExtensionObjectArrayDimensions;
	OpcUa_Boolean			bExtensionObjectDescription;
	OpcUa_Int32				uiArrayCurrentElt; // Element en cours de remplissage dans le cas ou l'on traite un vecteur (Array)
	// prise en charge de la définition de types spéciaux et des UADataType en général
	CField*					pField;
	CDefinition*			pDefinition;
	// Prise en charge pour l'initialisation des LocalizedText et autre chaine specifique
	OpcUa_String			tmpString;
	OpcUa_LocalizedText*	pLocalizedText;	
} HANDLER_DATA; 

// Collection that store the UserNameIdentityToken
typedef std::vector<OpcUa_UserNameIdentityToken*> CUserNameIdentityTokenList;
typedef std::vector<OpcUa_X509IdentityToken*> CX509IdentityTokenList;

// Cette fonction renvoie le nodeId de l'object en cours de parsing dans le HANDLER_DATA
OpcUa_StatusCode GetNodeIdForCurrentParsedObject(HANDLER_DATA* pHandleData, OpcUa_NodeId** pNodeId);
namespace OpenOpcUa
{
	namespace UACoreServer 
	{	
		// Manages an instance of a UA server application.
		class CServerApplication : public CApplication
		{
		public:
			CServerApplication(void);
			~CServerApplication(void);
			void EnableServerDiagnostics(OpcUa_Boolean bVal);
			OpcUa_StatusCode InitializeEndpointDescription(
				OpcUa_EndpointDescription* pEndpoint,
				OpcUa_String endpointUrl,
				OpcUa_String applicationUri,
				OpcUa_String productUri,
				OpcUa_LocalizedText* applicationName,
				OpcUa_ByteString* pCertificate,
				OpcUa_MessageSecurityMode securityMode,
				OpcUa_String securityPolicyUri,
				OpcUa_Byte securityLevel);
			OpcUa_StatusCode GetServerUris(OpcUa_String* pServerUris);
			// LDS
			OpcUa_UInt32 GetLDSRegistrationInterval();
			void SetLDSRegistrationInterval(OpcUa_UInt32 dwVal);
			OpcUa_Boolean IsLDSRegistrationActive();
			void LDSRegistrationActive(OpcUa_Boolean bVal);
			// Security None accepted or not
			void SecurityNoneAccepted(OpcUa_Boolean bVal);
			OpcUa_Boolean IsSecurityNoneAccepted();
			// //
			OpcUa_StatusCode LookupEncodeableType(OpcUa_UInt32 iTypeId,OpcUa_EncodeableType** pEncodeableType);
			// Initializes the stack and application.
			virtual void Initialize(void);

			// Frees all resources used by the stack and application.
			virtual void Uninitialize(void);

			// Initializes security and loads the application instance certificate.
			virtual OpcUa_StatusCode InitializeSecurity(/*OpcUa_String certificateStorePath*/);

			// Adds the server to the local discovery server's trust list.
			//void AddServerToDiscoveryServerTrustList(bool useWindowsTrustList);

			// Starts listening at the specified URL.
			void Start();
			
			// Stops the server.
			void Stop(void);

			// Called by the stack when an event, such as a change to a secure channel, occurs.
			OpcUa_StatusCode OnEndpointCallback(
										OpcUa_Endpoint_Event		a_eEvent,
										OpcUa_StatusCode			a_uStatus,
										OpcUa_UInt32				a_uSecureChannelId,
										OpcUa_Handle*				a_phRawRequestContext,
										OpcUa_ByteString*			a_pbsClientCertificate,
										OpcUa_String*				a_pSecurityPolicy,
										OpcUa_MessageSecurityMode   a_uSecurityMode,
										OpcUa_UInt32				a_uRequestedLifetime);

			CSecureChannelList*	GetSecureChannelList() {return m_pSecureChannels;}
			// Returns the endpoints supported by the server.
			OpcUa_StatusCode GetEndpoints(
										OpcUa_UInt32                uSecureChannelId,
										const OpcUa_RequestHeader*  pRequestHeader,
										const OpcUa_String*         pEndpointUrl,
										OpcUa_Int32                 nNoOfLocaleIds,
										const OpcUa_String*         pLocaleIds,
										OpcUa_Int32                 nNoOfProfileUris,
										const OpcUa_String*         pProfileUris,
										OpcUa_ResponseHeader*       pResponseHeader,
										OpcUa_Int32*                pNoOfEndpoints,
										OpcUa_EndpointDescription** pEndpoints);
			
			// Creates a session with the server.
			OpcUa_StatusCode CreateSession(
				OpcUa_UInt32                        uSecureChannelId,
				OpcUa_MessageSecurityMode           eSecurityMode,
				OpcUa_String                        securityPolicyUri,
				const OpcUa_RequestHeader*          pRequestHeader,
				const OpcUa_ApplicationDescription* pClientDescription,
				const OpcUa_String*                 pServerUri,
				const OpcUa_String*                 pEndpointUrl,
				const OpcUa_String*                 pSessionName,
				const OpcUa_ByteString*             pClientNonce,
				const OpcUa_ByteString*             pClientCertificate,
				OpcUa_Double                        nRequestedSessionTimeout,
				OpcUa_UInt32                        nMaxResponseMessageSize,
				OpcUa_ResponseHeader*               pResponseHeader,
				OpcUa_NodeId*                       pSessionId,
				OpcUa_NodeId*                       pAuthenticationToken,
				OpcUa_Double*                       pRevisedSessionTimeout,
				OpcUa_ByteString*                   pServerNonce,
				OpcUa_ByteString*                   pServerCertificate,
				OpcUa_Int32*                        pNoOfServerEndpoints,
				OpcUa_EndpointDescription**         pServerEndpoints,
				OpcUa_SignatureData*                pServerSignature,
				OpcUa_UInt32*                       pMaxRequestMessageSize);
			
			// Activates a session with the server.
			OpcUa_StatusCode ActivateSession(
				OpcUa_UInt32							uSecureChannelId,
				OpcUa_MessageSecurityMode				eSecurityMode,
				OpcUa_String*							securityPolicyUri,
				OpcUa_Int32								nNoOfClientSoftwareCertificates,
				const OpcUa_SignedSoftwareCertificate*	pClientSoftwareCertificates,
				const OpcUa_ExtensionObject*            pUserIdentityToken,
				const OpcUa_RequestHeader*				pRequestHeader,
				const OpcUa_SignatureData*				pClientSignature,
				OpcUa_ResponseHeader*					pResponseHeader,
				OpcUa_ByteString*						pServerNonce);

			// Closes a session with the server.
			OpcUa_StatusCode CloseSession(
				OpcUa_UInt32               uSecureChannelId,
				const OpcUa_RequestHeader* pRequestHeader,
				OpcUa_Boolean              bDeleteSubscriptions,
				OpcUa_ResponseHeader*      pResponseHeader);

			// Browse the server mesh
			OpcUa_StatusCode Browse(OpcUa_UInt32 uSecureChannelId,
				const OpcUa_RequestHeader* pRequestHeader,
				const OpcUa_ViewDescription*   pView,
				OpcUa_UInt32                   uiRequestedMaxReferencesPerNode,
				OpcUa_UInt32                    iNoOfNodesToBrowse,
				const OpcUa_BrowseDescription* pNodesToBrowse,
				OpcUa_ResponseHeader*          pResponseHeader,
				OpcUa_Int32*                   pNoOfResults,
				OpcUa_BrowseResult**           ppResults,
				OpcUa_Int32*                   pNoOfDiagnosticInfos,
				OpcUa_DiagnosticInfo**         ppDiagnosticInfos);
			// BrowseNext in the server
			OpcUa_StatusCode     BrowseNext(OpcUa_UInt32 uSecureChannelId,
				const OpcUa_RequestHeader* a_pRequestHeader,
				OpcUa_Boolean              a_bReleaseContinuationPoints,
				OpcUa_Int32                a_nNoOfContinuationPoints,
				const OpcUa_ByteString*    a_pContinuationPoints,
				OpcUa_ResponseHeader*      a_pResponseHeader,
				OpcUa_Int32*               a_pNoOfResults,
				OpcUa_BrowseResult**       a_pResults,
				OpcUa_Int32*               a_pNoOfDiagnosticInfos,
				OpcUa_DiagnosticInfo**     a_pDiagnosticInfos);

			OpcUa_StatusCode RegisterNodes( OpcUa_UInt32 uSecureChannelId,
											const OpcUa_RequestHeader* a_pRequestHeader,
											OpcUa_Int32                a_nNoOfNodesToRegister,
											const OpcUa_NodeId*        a_pNodesToRegister,
											OpcUa_ResponseHeader*      a_pResponseHeader,
											OpcUa_Int32*               a_pNoOfRegisteredNodeIds,
											OpcUa_NodeId**             a_pRegisteredNodeIds);

			OpcUa_StatusCode UnregisterNodes(	OpcUa_UInt32 uSecureChannelId,
												const OpcUa_RequestHeader* a_pRequestHeader,
												OpcUa_Int32                a_nNoOfNodesToUnregister,
												const OpcUa_NodeId*        a_pNodesToUnregister,
												OpcUa_ResponseHeader*      a_pResponseHeader);
			// Finds the session identified by the SecureChannel.
			CSessionServer* FindSession(OpcUa_UInt32 uSecureChannelId);
			// Finds the session identified by its SecureChannel and its authentication token.
			OpcUa_StatusCode FindSession(const OpcUa_UInt32 uSecureChannelId,const OpcUa_NodeId* pAuthenticationToken, CSessionServer** pSession);
			// Find a session based on its authentication token. So it could return the first one if multiple session use the same authentication token.
			CSessionServer* FindSession(const OpcUa_NodeId* pAuthenticationToken);
			OpcUa_StatusCode FindSessionBySessionId(OpcUa_NodeId* pSessionID, CSessionServer** pSession);
			//
			OpcUa_StatusCode AddBinding(CUABinding* pBinding);
			CUABindings GetServerBindings() {return m_ServerBindingList;}
			// Thread en charge de l'inscription auprès du LDS
			void StartLDSRegistrationThread();
			OpcUa_StatusCode StopLDSRegistrationThread();
			static void  LDSRegistrationThread(LPVOID arg);	
			OpcUa_StatusCode LoadUaServerConfiguration(char* path,char* fileName); // Chargement du fichier de configuration
			OpcUa_StatusCode LoadUaServerNodeSet(char* path,char* fileName); // chargement des fichiers qui composent l'adresseSpace
			OpcUa_StatusCode LoadUaServerSimulation(char* path,char* fileName); // chargement des fichiers de simulation
			OpcUa_StatusCode LoadUaServerSubsystems(char* path,char* fileName); // chargement des fichiers de subsystem

			CSecureChannel* FindSecureChannel(OpcUa_UInt32 uSecureChannelId);
			OpcUa_UInt32 GetCurrentSessionCount() {return m_sessions.size();}

			// This method will ack and comment all instances of this CEventDefiniton. The boolean bAck indicate if Ack is require of just comment
			// The idea is to be able to ack for all client connected to the server for EventDefinition associated with the EventId passed in parameter
			OpcUa_StatusCode AckCommentConfirmApplicationEvents(CEventDefinition* pEventDefinition, const OpcUa_LocalizedText comment, METHOD_NAME eBehavior);
			void WakeupAllVpi(); // débloque la seaphore de tous les Vpis
			void AddVpiDevice(CVpiDevice* pSubsystem);
			OpcUa_StatusCode RemoveVpiDevice(CVpiDevice* pSubsystem);
			void RemoveAllVpiDevice();
		private:
			OpcUa_StatusCode OpenSecureChannel(	OpcUa_UInt32				uSecureChannelId,
												OpcUa_ByteString*			pbsClientCertificate,
												OpcUa_String*				pSecurityPolicy,
												OpcUa_MessageSecurityMode	uSecurityMode);
			OpcUa_StatusCode AddSecureChannel(CSecureChannel* pSecureChannel);
			OpcUa_StatusCode CloseSecureChannel(CSecureChannel* pChannel);
			OpcUa_StatusCode RemoveSecureChannel(CSecureChannel* pChannel);
			OpcUa_StatusCode RemoveAllSecureChannel();
			// Registers the server with the local discovery server.
			OpcUa_StatusCode RegisterServer(OpcUa_CharA* szEndPointType,const OpcUa_String& discoveryUrl, OpcUa_Boolean bOnline);

			// Registers the server with the local discovery server at the specified endpoint.
			OpcUa_StatusCode RegisterServer(CEndpointDescription* pEndpoint, OpcUa_Boolean bOnline);

			// Returns a copy of the endpoints for the server.
			OpcUa_StatusCode GetServerEndpoints(
				OpcUa_Int32                 nNoOfProfileUris, // in
				const OpcUa_String*         pProfileUris,
				OpcUa_Int32*                pNoOfEndpoints,
				OpcUa_EndpointDescription** pEndpoints);

			// Sessions handling
			void AddSessionServer(CSessionServer* pSession);
			void RemoveAllSessionServer();
			OpcUa_StatusCode RemoveSessionServer(CSessionServer* pSession);
			// UsersIdentityToken
			OpcUa_StatusCode InitializeUsersIdentityToken();
			OpcUa_StatusCode RemoveUsersIdentityToken();
			// X509IdentityToken
			OpcUa_StatusCode InitializeX509IdentityTokenList();
			OpcUa_StatusCode RemoveX509IdentityTokenList();
			
		public:
			OpcUa_StatusCode WakeupAllSubscription();
			OpcUa_Boolean IsClientNonceExist(const OpcUa_UInt32 uSecureChannelId,const OpcUa_ByteString* pClientNonce); // verification de l'existence d'un clientNonce
			void StartSessionsTimeoutThread();
			// Returns the CApplication Description, Name, Type, etc..
			OpenOpcUa::UASharedLib::CApplicationDescription* GetApplicationDescription()
			{
				return m_pApplicationDescription;
			}	
			void SetApplicationDescription(CApplicationDescription* pApplicationDescription)
			{
				m_pApplicationDescription=pApplicationDescription;
			}
			CHaEngine*	GetHaEngine() {return m_pHaEngine;}
			void SetHaEngine(CHaEngine* pVal) {m_pHaEngine=pVal;}
			void SetArchiveId(OpcUa_String ArchiveId)
			{
				OpcUa_String_CopyTo(&ArchiveId,&m_ArchiveId);
			}
			OpcUa_String GetArchiveId() {return m_ArchiveId;}
			void SetArchiveName(OpcUa_String ArchiveName)
			{
				OpcUa_String_CopyTo(&ArchiveName,&m_ArchiveName);
			}
			OpcUa_String GetArchiveName() {return m_ArchiveName;}
			CEventsEngine*	GetEventsEngine();
			void SetEventsEngine(CEventsEngine* pEventsEngine);
			CVpiDeviceList*  GetVpiDeviceList() { return m_pVpiDevices; } // Access to the Vpi List
			OpcUa_StatusCode GetVpiDeviceById(OpcUa_NodeId deviceNodeId, CVpiDevice** ppVpiDevice);
			void ParseCommandLine(OpcUa_CharA* szRawCommand, STARTUP_COMMAND* pStartupCommand);
			void ParseFirstParameter(char* pszParam,STARTUP_COMMAND* pStartupCommand);
			void ParseSecondParameter(char* pszParam, STARTUP_COMMAND* pStartupCommand);
			// Accessor for config file and path
			OpcUa_CharA* GetConfigPath();
			void SetConfigPath(OpcUa_CharA* pszVal);
			OpcUa_CharA* GetConfigFileName();
			void SetConfigFileName(OpcUa_CharA* pszVal);
		private:
			void StopSessionsTimeoutThread();
			static void SessionsTimeoutThread(LPVOID arg);
			OpcUa_UInt32 UpdateTimeoutInterval();
		public:
			static OpcUa_Double							m_dblMiniSamplingInterval;
			static OpcUa_Double							m_dblMaxSamplingInterval;
#ifdef WIN32
			CServiceModule								m_ServiceModule;
#endif
		private:
			OpcUa_CharA*								m_ConfigPath;
			OpcUa_CharA*								m_ConfigFileName;
			OpcUa_Mutex									m_hSecureChannelsMutex;
			CSecureChannelList*							m_pSecureChannels; // SecureChannel creé pour cette instance du serveur
			// variables for the LDSRegistration thread
			OpcUa_Thread								m_hLDSRegistrationThread;
			OpcUa_Boolean								m_bRunLDSRegistrationThread;
			OpcUa_Semaphore								m_hStopLDSRegistrationSem;
			OpcUa_Semaphore								m_hLDSRegistrationRequest; // Event to notify the LDS Registration thread
			OpcUa_UInt32								m_uiLDSRegistrationInterval; // interval between two registration to the LDS
			OpcUa_Mutex									m_hMutex;  // Syncro object
			OpcUa_Endpoint								m_hEndpoint; // UA EndPoint Object
			OpcUa_Int32									m_nNoOfSecurityPolicies;// size of m_pSecurityPolicies
			OpcUa_Endpoint_SecurityPolicyConfiguration* m_pSecurityPolicies;
			OpcUa_Mutex									m_hSessionsMutex; // Mutex de protection de la list de session m_sessions
			CSessionServerList							m_sessions; // List des sessions prise en charge par le serveur
			OpcUa_Int32									m_lastSessionId;
			CUABindings									m_ServerBindingList; // Contient la liste des Binding pour ce serveur (Transport, Port, Encoding)
			CApplicationDescription*					m_pApplicationDescription;																			 
			OpcUa_Thread								m_hSessionsTimeoutThread;
			OpcUa_Boolean								m_bRunSessionsTimeoutThread;
			OpcUa_Semaphore								m_SessionsTimeoutSem;
			CHaEngine*									m_pHaEngine; // Historical Access Engine class
			OpcUa_String								m_ArchiveId;
			OpcUa_String								m_ArchiveName;
			OpcUa_Boolean								m_bLDSRegistrationActive; // Boolean use to indicate if the LDS registeration is active for this server instance
			CEventsEngine*								m_pEventsEngine; // Events Engine 
			OpcUa_Mutex									m_VpiDevicesMutex;
			CVpiDeviceList*								m_pVpiDevices; // Liste des équipements relié au serveur
			OpcUa_Boolean								m_bSecurityNoneAccepted;
		protected:
			static OpcUa_Boolean						m_bIsExist;
		public:
			// Attribut to handle UserNameIdentityToken
			CUserNameIdentityTokenList					m_UserNameIdentityTokenList;
			OpcUa_Mutex									m_UserNameIdentityTokenListMutex;
			// Attribute to handle 
			CX509IdentityTokenList						m_X509IdentityTokenList;
			OpcUa_Mutex									m_X509IdentityTokenListMutex;
			// 
			static CUAInformationModel*					m_pTheAddressSpace;	// AddressSpace of this server
			std::vector<OpcUa_String*>					m_FilesNodeSet; // Collection contenant le nom des fichiers de configuration du serveur
			std::vector<OpcUa_String*>					m_FilesSimulation; // Collection contenant le nom des fichiers de simulation utilisé par le serveur
			std::vector<OpcUa_String*>					m_FilesSubsystem; // Collection contenant le nom des fichiers de définition des subsystems qui seront utilisé
		};
	} 
}