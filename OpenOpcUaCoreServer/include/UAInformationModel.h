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
//
// Classe en charge de la gestion de l'espace d'adressage et/ou du dictionnaire de données

#pragma once

using namespace UASubSystem;
using namespace OpenOpcUa;
using namespace UASimulation;
using namespace UACoreServer;
using namespace UAEvents;
#define UAINFORMATIONMODEL_FASTACCESS_SIZE 25000

OpcUa_StatusCode FindTypeDefinition(OpcUa_NodeId aNodeId, CDefinition** pDefinition);

namespace OpenOpcUa
{
	namespace UAAddressSpace 
	{
		/////////////////////////////////////////////////////////////////////////////
		//
		class CUAInformationModelFastAccess
		{
		public:
			CUAInformationModelFastAccess()
			{
				m_uStatus=OpcUa_Good;
				m_uiNumericNodeId=0;
				m_pUABase=OpcUa_Null;
				m_NodeClass=OpcUa_NodeClass_Unspecified;
			}
			CUAInformationModelFastAccess(CUABase*	pUABase)
			{
				if (pUABase)
				{
					OpcUa_NodeId* pNodeId=pUABase->GetNodeId();
					m_uStatus=OpcUa_Good;
					if ( (pNodeId->IdentifierType == OpcUa_IdentifierType_Numeric) && (pNodeId->NamespaceIndex==0) )
					{
						if (pNodeId->Identifier.Numeric<UAINFORMATIONMODEL_FASTACCESS_SIZE)
							m_uiNumericNodeId=pNodeId->Identifier.Numeric;
						else
							m_uStatus=OpcUa_BadInternalError;
					}
					else
						m_uStatus=OpcUa_BadInvalidArgument;
					m_NodeClass=pUABase->GetNodeClass();
					m_pUABase=pUABase;
				}
			}
			~CUAInformationModelFastAccess()
			{
				m_pUABase=OpcUa_Null;
			}
			OpcUa_UInt32 GetNumericNodeId() {return	m_uiNumericNodeId;}
			void SetNumericNodeId(OpcUa_UInt32 uiVal) {m_uiNumericNodeId=uiVal;}
			CUABase* GetUABase() {return m_pUABase;}
			void SetUABase(CUABase* pUABase) {m_pUABase=pUABase;}
			OpcUa_NodeClass GetNodeClass() {return m_NodeClass;}
			void SetNodeClass(OpcUa_NodeClass val) {m_NodeClass=val;}
			OpcUa_StatusCode GetStatusCode() {return m_uStatus;}
		private:
			OpcUa_UInt32		m_uiNumericNodeId;
			CUABase*			m_pUABase;
			OpcUa_NodeClass		m_NodeClass;
			OpcUa_StatusCode	m_uStatus;
		};
		typedef std::vector<CUAInformationModelFastAccess*> CUAInformationModelFastAccessList;
		/////////////////////////////////////////////////////////////////////////////
		//
		class CUAInformationModel 
		{
		public:
			CUAInformationModel() ;
			CUAInformationModel(const char **atts);
			~CUAInformationModel();
			// Methods to Add a new UANode in the AddressSpace
			OpcUa_StatusCode PopulateInformationModel(CUAVariable* pUAVariable);
			OpcUa_StatusCode PopulateInformationModel(CUAObject* pUAObject);
			// Lookup nodespaceUris en fonction du namespaceindex
			//OpcUa_StatusCode LookupNamespaceUri(OpcUa_String* aNamespaceUri,CUANamespaceTranslationMap* pNamespaceTranslationTable);
			// The addressSpace in made of severail collection. This address is a kind of node Dictionnary
			// Accesseur des collections constituant le dictionnaire
			CUAReferenceTypeList* GetReferenceTypeList() {return m_pReferenceTypeList;}
			CUAObjectList* GetObjectList() {return m_pUAObjectList;}
			CUAObjectTypeList* GetObjectTypeList() {return m_pUAObjectTypeList;}
			CUADataTypeList* GetDataTypeList() {return m_pUADataTypeList;}
			CUAVariableTypeList* GetVariableTypeList() {return m_pUAVariableTypeList;}
			CUAVariableList* GetVariableList() {return m_pUAVariableList;}
			CUAMethodList* GetMethodList() {return m_pUAMethodList;}
			CAliasList* GetAliasList() {return m_pAliasList;}
			CUAViewList* GetViewList() {return m_pUAViewList;}
			//
			// Méthode de recherche dans le dictionnaire
			// recherche sur le nodeId
			OpcUa_StatusCode GetNodeIdFromFastAccessList(const OpcUa_NodeId iNodeId,CUABase** pUABase);
			OpcUa_StatusCode GetNodeIdFromDictionnary(const OpcUa_NodeId iNodeId,CUABase** pUABase);
			OpcUa_StatusCode GetNodeIdFromObjectList(const OpcUa_NodeId iNodeId,CUAObject** pUAObject);
			OpcUa_StatusCode GetNodeIdFromObjectTypeList(const OpcUa_NodeId iNodeId,CUAObjectType** pUAObjectType);
			OpcUa_StatusCode GetNodeIdFromDataTypeList(const OpcUa_NodeId iNodeId,CUADataType** pUADataType);
			OpcUa_StatusCode GetNodeIdFromVariableTypeList(const OpcUa_NodeId iNodeId,CUAVariableType** pUAVariableType);
			OpcUa_StatusCode GetNodeIdFromVariableList(const OpcUa_NodeId iNodeId,CUAVariable** pUAVariable);
			OpcUa_StatusCode GetNodeIdFromReferenceTypeList(const OpcUa_NodeId iNodeId,CUAReferenceType** pUAReferenceType);
			OpcUa_StatusCode GetNodeIdFromMethodList(const OpcUa_NodeId iNodeId,CUAMethod** pUAMethod);
			OpcUa_StatusCode GetNodeIdFromViewList(const OpcUa_NodeId iNodeId,CUAView** pUAView);
			OpcUa_StatusCode GetAliasFromNodeId(const OpcUa_NodeId iNodeId,CAlias** pAlias);
			OpcUa_StatusCode GetAliasFromAliasName(const OpcUa_String szNodeId,CAlias** pAlias);
			// recherche sur le browseName
			OpcUa_StatusCode GetNodeFromDictionnary(OpcUa_QualifiedName szBrowseName,CUABase** pUABase);
			OpcUa_StatusCode GetNodeObjectFromBrowseName(OpcUa_QualifiedName szBrowseName, CUAObject** pUAObject);
			OpcUa_StatusCode GetNodeObjectTypeFromBrowseName(OpcUa_QualifiedName szBrowseName, CUAObjectType** pUAObjectType);
			OpcUa_StatusCode GetNodeMethodFromBrowseName(OpcUa_QualifiedName szBrowseName, CUAMethod** pUAMethod);
			OpcUa_StatusCode GetNodeReferenceTypeFromBrowseName(OpcUa_QualifiedName szBrowseName, CUAReferenceType** pUAReferenceType);
			OpcUa_StatusCode GetNodeVariableFromBrowseName(OpcUa_QualifiedName szBrowseName, CUAVariable** pUAVariable);
			OpcUa_StatusCode GetNodeVariableTypeFromBrowseName(OpcUa_QualifiedName szBrowseName, CUAVariableType** pUAVariableType);
			OpcUa_StatusCode GetNodeViewFromBrowseName(OpcUa_QualifiedName szBrowseName, CUAView** pUAView);
			OpcUa_StatusCode FindNodes(CUABase* pStartingNode,OpcUa_String szTargetNode, OpcUa_NodeId aReferenceTypeId, OpcUa_Boolean bInverse, OpcUa_Boolean bIncludeSubTypes,vector<CUABase*>** pNodes);
			OpcUa_StatusCode IsInUABaseHierarchy(OpcUa_NodeId NodeIdSource, OpcUa_NodeId NodeIdTarget);
			OpcUa_StatusCode IsInEventTypeHierarchy(OpcUa_NodeId idEventSource, OpcUa_NodeId idEventTarget);
			OpcUa_StatusCode IsInReferenceHierarchy(OpcUa_NodeId idReferenceSource, OpcUa_NodeId idReferenceTarget);
			OpcUa_StatusCode UpdateUAVariablesBuiltinType(); // met a jour le BuiltInType de toute les UAVariables
			OpcUa_StatusCode UpdateUAVariablesEncodeableObject();
			OpcUa_StatusCode InitLocaleIdArray();
			OpcUa_StatusCode InitServerStatus(OpcUa_NodeId aNodeId, OpcUa_DateTime dtStartTime); // initialisation de la node 2256 ServerStatus
			OpcUa_StatusCode InitServerArray(OpcUa_CharA* szServerArray); // intialisation de la node 2254 ServerArray
			OpcUa_StatusCode InitBuildInfo(); // intialisation de la node 2260 BuildInfo
			OpcUa_StatusCode InitServerDiagnostics(); // Initialisation de la node 2274 ServerDiagnostics
			OpcUa_StatusCode UpdateServerDiagnostics();
			OpcUa_StatusCode UpdateServerDiagnosticsSummary();
			// Méthodes pour la gestion du SubscriptionDiagnosticsArray
			OpcUa_StatusCode AddInSubscriptionDiagnosticsArray(CSubscriptionDiagnosticsDataType* pSubscriptionDiagnosticsDataType);
			OpcUa_StatusCode RemoveAllSubscriptionDiagnosticsArray();
			OpcUa_StatusCode RemoveInSubscriptionDiagnosticsArray(CSubscriptionDiagnosticsDataType* pSubscriptionDiagnosticsDataType);
			OpcUa_StatusCode UpdateSubscriptionDiagnosticsArray();
			OpcUa_StatusCode AddSubscriptionDiagnosticsInAddressSpace(CSubscriptionDiagnosticsDataType* pSubscriptionDiagnosticsDataType);
			OpcUa_StatusCode RemoveSubscriptionInAddressSpace(CSubscriptionDiagnosticsDataType* pSubscriptionDiagnosticsDataType);
			OpcUa_StatusCode UpdateSubscriptionDiagnosticsValueInAddressSpace(CSubscriptionDiagnosticsDataType* pSubscriptionDiagnosticsDataType);
			// Méthodes pour la gestion du SessionDiagnosticsArray
			OpcUa_StatusCode AddInSessionDiagnosticsArray(CSessionDiagnosticsDataType* pSessionDiagnosticsDataType); // 3707
			OpcUa_StatusCode RemoveInSessionDiagnosticsArray(CSessionDiagnosticsDataType* pSessionDiagnosticsDataType);
			OpcUa_StatusCode AddSessionNameInAddressSpace(OpcUa_String sessionName); // This methode Add the SessionName passed by the client as a node in the addressSpace
			OpcUa_StatusCode RemoveSessionNameInAddressSpace(OpcUa_String sessionName);
			// Méthodes pour la gestion du SessionSecurityDiagnosticsArray
			OpcUa_StatusCode AddInSessionSecurityDiagnosticsArray(CSessionSecurityDiagnosticsDataType* pSessionSecurityDiagnostics); // 3708
			OpcUa_StatusCode RemoveInSessionSecurityDiagnosticsArray(CSessionSecurityDiagnosticsDataType* pSessionSecurityDiagnosticsDataType);
			// SamplingIntervalDiagnostics
			OpcUa_StatusCode InitSamplingIntervalDiagnosticsArray();
			OpcUa_StatusCode AddInSamplingIntervalDiagnosticsArray(OpcUa_SamplingIntervalDiagnosticsDataType* pSamplingIntervalDiagnostic);
			OpcUa_StatusCode RemoveInSamplingIntervalDiagnosticsArray(OpcUa_SamplingIntervalDiagnosticsDataType*pSamplingIntervalDiagnostic);
			OpcUa_Boolean IsSamplingIntervalDiagnosticsExist(OpcUa_Double dblSamplingInterval, OpcUa_SamplingIntervalDiagnosticsDataType** ppSamplingIntervalDiagnostic);
			OpcUa_StatusCode AddSamplingIntervalnTheAddressSpace(OpcUa_SamplingIntervalDiagnosticsDataType* pSamplingIntervalDiagnosticsDataType);
			OpcUa_StatusCode RemoveSamplingIntervalnTheAddressSpace(OpcUa_SamplingIntervalDiagnosticsDataType* pSamplingIntervalDiagnosticsDataType);
			OpcUa_StatusCode UpdateSamplingIntervalDiagnosticsArray(OpcUa_NodeId aNodeId);
			// SimulatedGroup related method
			void AddSimulatedGroup(CSimulatedGroup* pSimulatedGroup);
			// RedundantServerArray
			OpcUa_StatusCode InitRedundantServerArray();
			// 
			OpcUa_StatusCode IsReferenceTypeIdValid(OpcUa_NodeId aRefTypeId, CUAReferenceType** pUAReferenceType);
			//
			// methode permettant de verifier si le client a demandé d'inclure les sous-type
			OpcUa_StatusCode BrowseIsIncludeSubtypes(
													OpcUa_BrowseDescription aNodesToBrowse, // in
													OpcUa_NodeId aReferenceTypeId, //in
													OpcUa_Boolean *bReferenceTypeIdOk); // out

			OpcUa_StatusCode AddReferenceInBrowseResult(const OpcUa_BrowseDescription aNodesToBrowse, // in
														const CUABase* pUABase, // in
														const CUABase* pUABaseTarget, // in
														CUAReference* pReference, // in
														OpcUa_BrowseResult** ppNewBrowseResult, // in/out
														OpcUa_UInt32 *iRef); // in/out
			OpcUa_StatusCode BrowseOneNode(OpcUa_BrowseDescription aNodeToBrowse, 
										   OpcUa_UInt32 uiRequestedMaxReferencesPerNode,
										   OpcUa_BrowseResult** pBrowseResult,
										   CContinuationPoint** pContinuationPoint);
			OpcUa_StatusCode InvertInverseReferences();// Method used to transform inverse references in forward reference for all already define nodes
			OpcUa_StatusCode InvertForwardReferences();// Method used to add inverse refenrece not explicitly define in the NodeSet
			OpcUa_StatusCode UpdateNamespaceArray();
			OpcUa_StatusCode UpdateUAVariableTypeDataType();
			OpcUa_StatusCode UpdatePendingVariableDatatype();
			OpcUa_StatusCode UpdateInformationModelFastAccessList();
			OpcUa_StatusCode InitializeEncodeableObject(CUAVariable* pUAVariable);
			void TraceAddressSpace(); // Show the size of the AddressSpace
			// Ajout d'un nouveau EncodeableObject for the new UADataType define by the user in the NodeSet
			OpcUa_StatusCode AddNewEncodeableObject(CUADataType* pDataType, OpcUa_EncodeableType** pEncodeableType);
			OpcUa_StatusCode GetEncodingNodeId(OpcUa_NodeId inputNodeId, OpcUa_NodeId* pOutEncodingNodeId);
			// Event initialization
			OpcUa_StatusCode SearchEventsDefinition();
			// EventDuplication from its ConditionType
			OpcUa_StatusCode InstanciateConditionType(CUAVariable* pSourceNode, CUAObject* pCondition, CUAObject** ppNewCondition);
			OpcUa_StatusCode CreateBaseEventTypeAttributeAndReferences(CUAVariable* pSourceNode, CUAObject* pCondition, CUAObject** pNewCondition);
			OpcUa_StatusCode CreateConditionTypeReferences(CUAObject** ppNewCondition); // i=2782 (ConditionType) subtype of i=2041 (BaseEventType)
			OpcUa_StatusCode CreateAcknowledgeableConditionTypeReferences(CUAObject** ppNewCondition);
			OpcUa_StatusCode CreateAlarmConditionTypeReferences(CUAObject** ppNewCondition);
			OpcUa_StatusCode CreateLimitAlarmConditionTypeReferences(CUAObject** ppNewCondition);
			OpcUa_StatusCode CreateDiscreteAlarmTypeReferences(CUAObject** ppNewCondition);
			OpcUa_StatusCode CreateOffNormalAlarmTypeReferences(CUAObject** ppNewCondition);
			OpcUa_StatusCode SearchForEventMessage(CUAObject* pCondition, CUAVariable** ppUAVariable);
			OpcUa_StatusCode SearchForEventSeverity(CUAObject* pCondition, CUAVariable** ppUAVariable);
			//
			OpcUa_StatusCode SearchForLimits(CUAObject* pUAAlarmDefiniton, UALimitsAlarm* pLimits);
			OpcUa_StatusCode SearchForEventType(CUAObject* pUAObject);
			OpcUa_StatusCode SearchForSourceNode(CUABase* pUANode, OpcUa_NodeId* pSourceNode);
			OpcUa_StatusCode SearchForHasEventSourceNodeList(CUABase* pUAObject, std::vector<OpcUa_ExpandedNodeId*>* pEventSourceList);
			OpcUa_StatusCode SearchHasCondition(CUABase* pUANode, OpcUa_ExpandedNodeId* pEventConditionId);
			OpcUa_StatusCode SearchForMessage(CUAObject* pUAObject);
			OpcUa_StatusCode SearchForSeverity(CUAObject* pUAObject);
			OpcUa_StatusCode SearchForState(CUABase* pUABaseFound, OpcUa_QualifiedName SearchedState, CUAVariable** pAckedStateUAVariable);
			OpcUa_StatusCode SearchForMethod(CUABase* pUABaseFound, OpcUa_QualifiedName MethodName, CUAMethod** ppMethod);
			OpcUa_StatusCode IsInEventTypeList(OpcUa_NodeId aNodeId, CUAObjectType** pUAObjectType);
			// Add internal OpenOpcUa SystemNodes to the AddressSpace
			OpcUa_StatusCode AddinternalOpenOpcUaSystemNodes();
			CLuaVirtualMachine* GetLuaVirtualMachine();
			// Method script related
			COpenOpcUaScript* GetOpenOpcUaScript();
			OpcUa_StatusCode AddUAObject(OpcUa_NodeId aNodeId, OpcUa_String szBrowseName, OpcUa_NodeId typeDefinition);
			OpcUa_StatusCode AddUAVariable(OpcUa_NodeId aNodeId, OpcUa_String szBrowseName, OpcUa_NodeId typeDefinition, OpcUa_Variant internalValue, OpcUa_Byte bAccessLevel, OpcUa_Boolean bHistorizing);
			OpcUa_StatusCode AddVpiUATag(OpcUa_NodeId nodeId, OpcUa_NodeId deviceNodeId,OpcUa_String szAddress);
			OpcUa_StatusCode AddUAReference(OpcUa_NodeId sourceNodeId, OpcUa_NodeId targetNodeid, OpcUa_NodeId referenceTypeId);
			// 
		private:
			OpcUa_StatusCode UpdateSessionDiagnosticsArray(OpcUa_NodeId aNodeId);
			OpcUa_StatusCode UpdateSessionSecurityDiagnosticsArray(OpcUa_NodeId aNodeId);
			OpcUa_StatusCode FillReferenceNodeIdHierachy(OpcUa_NodeId idReference, vector<OpcUa_NodeId>* pHierachy);
			OpcUa_StatusCode FillEventTypeNodeIdHierachy(OpcUa_NodeId idEvent, vector<OpcUa_NodeId>* pHierachy);
			OpcUa_StatusCode FillUABaseNodeIdHierarchy(OpcUa_NodeId aNodeId, vector<OpcUa_NodeId>* pHierachy, OpcUa_UInt32* pDepth);
			// Init method
			void InitTimeStampAndStatusCode(OpcUa_NodeId aNodeId);
		public:
			OpcUa_Mutex 		GetServerCacheMutex() {return m_ServerCacheMutex;}
			//CSimulatedGroupList* GetSimulatedGroupList() {return m_pSimulatedGroupList;}

			//////////////////////////////////////////////
			//access to the internal server status
			static CServerStatus*	GetInternalServerStatus() 
			{
				return CUAInformationModel::m_pInternalServerStatus;
			}
			//CVPIScheduler* GetVPIScheduler(){return m_pVPIScheduler;}
			OpcUa_Semaphore GetSimulationEvent() {return m_SimulationSem;}
			// methode relative a la manipulation des namespaceUris
			// Recupération du nombre d'Uris dans le m_NamespaceUris
			OpcUa_Int32 GetNumOfNamespaceUris() {return m_NamespaceUris.size();}
			// 
			// ajout d'un Namespaceuri dans la liste des NamespaceUri pris en charge par ce serveur
			void AddNamespaceUri(CNamespaceUri* aNamespaceUri);
			// recherche d'un NamespaceUri sur son index
			CNamespaceUri* GetNamespaceUri(OpcUa_UInt32 index);
			// Recupère l'index d'un NamespaceUri dans la liste a partir de son nom 
			OpcUa_StatusCode GetNamespaceUri(OpcUa_String aNamespaceUri, OpcUa_Int32* pIndex);

			// Managing Uri when the server is loading a NodeSetFile
			OpcUa_StatusCode OnLoadNamespaceUriVerifyUri(OpcUa_String aNamespaceUri);
			void OnLoadNamespaceUriAddUri(OpcUa_String aString, OpcUa_UInt32 index);
			void OnLoadNamespaceUriEraseAll(); // Erase all  the namespaceUri loaded for the last loaded NodeSet file
			OpcUa_UInt32 OnLoadNamespaceUriGetSize();
			OpcUa_StatusCode OnLoadNamespaceUriGetNamespaceIndexbyUri(OpcUa_String aUriName, OpcUa_UInt32* pIndex);
			OpcUa_StatusCode OnLoadNamespaceUriToAbsoluteNamespaceUri(OpcUa_UInt32 iNamespace, OpcUa_Int32* pIndexNS); // convert a OnLoadNamespaceUri to a AbsoluteNamespaceUri
			OpcUa_Boolean OnLoadNamespaceUriContains(OpcUa_String uriName);
			void OnLoadNamespaceUriUpdateIndex(OpcUa_String uriName, OpcUa_UInt32 uiNewIndexVal);
#ifdef WIN32
			CFileVersionInfo* GetFileVersionInfo() {return m_pFileVersionInfo;}
#endif
			OpcUa_Semaphore m_SemMandatoryEvent; // semaphore utilisée pour synchronisze le démarrage de la thread de mise à jour des mandatory Nodes
			OpcUa_Boolean IsEnabledServerDiagnosticsDefaultValue();
			void EnableServerDiagnosticsDefaultValue(OpcUa_Boolean bVal);
			OpcUa_StatusCode StopUpdateMandatoryNodesThread();
			// Access the ServerDiagnosticsSummaryDataType
			OpcUa_ServerDiagnosticsSummaryDataType* GetServerDiagnosticsSummaryDataType(void) const	{ return(m_pServerDiagnosticsSummaryDataType);}
			// Script Autorun. This script must exist in the OpenOpcUaCoreServer.lua file
			OpcUa_StatusCode Autorun();
		protected:			
			OpcUa_StatusCode InitMandatoryNodeId();
			// mise à jour des Nodes Obligatoires
			void StartUpdateMandatoryNodesThread();
			static void UpdateMandatoryNodesThread(LPVOID arg);
		private:
			OpcUa_StatusCode InitAllowNulls(OpcUa_Boolean bVal);
		public:
			static OpcUa_Mutex 						m_ServerCacheMutex; // section critique permettant de protéger la cache du serveur
		protected:
			CLuaVirtualMachine*						m_pLuaVm;
			COpenOpcUaScript*						m_pOpenOpcUaScript;
			OpcUa_Boolean							m_bServerDiagnosticsDefaultValue; // Permet d'indiquer si par défaut les sessions auront le dignositic activé ou non (2294)

			OpcUa_Semaphore							m_SimulationSem; // permet de démarrer la simulation a la fin du chargement du fichier de definition
			OpcUa_Mutex								m_SimulatedGroupMutex;
			CSimulatedGroupList*					m_pSimulatedGroupList; // Liste des groupes de simulation du serveur
			// variable for the publishing thread
			OpcUa_Thread							m_hUpdateMandatoryNodesThread;
			BOOL									m_bRunUpdateMandatoryNodesThread;
			DWORD									m_dwUpdateMandatoryNodesThreadId;
			OpcUa_Semaphore							m_hStopUpdateMandatoryNodesThreadSem;
			// variable pour le support du dictionnaire
			static CServerStatus*					m_pInternalServerStatus; // represent l'état interne du serveur. 
																// Correspond au NodeId i=2256
#ifdef WIN32
			CFileVersionInfo*						m_pFileVersionInfo;
#endif 
			CUAObjectTypeList*						m_pEventTypeList; // This collection is suppose to contains all EventType supported by the server.
																	  // we store all the EventType in a CUAObjectTypeList because EventType are always CUAObjectType
			CUAReferenceTypeList*					m_pReferenceTypeList; // Liste de tous les types de reference pour ce serveur... 
			CUAObjectList*							m_pUAObjectList; // liste de tous les objects de l'espace d'adressage du serveur
			CUAObjectTypeList*						m_pUAObjectTypeList; // Liste de tous les objectType de ce serveur
			CUADataTypeList*						m_pUADataTypeList; // Liste de tous les dataTypê de ce serveur
			CUAVariableTypeList*					m_pUAVariableTypeList; // List de tous les variableType de ce serveur
			CUAVariableList*						m_pUAVariableList; // Liste de toutes les variables, DataVariable et Property de ce serveur
			CUAViewList*							m_pUAViewList; // Liste de toutes les View 
			CUAMethodList*							m_pUAMethodList; // Liste de toutes les méthodes
			CAliasList*								m_pAliasList; // Liste des alias
			// Variable de classe pour la manipulation des NamespaceUris
			OpcUa_Mutex								m_OnLoadNamespaceUrisMutex;
			CUANamespaceUris						m_OnLoadNamespaceUris; // NamespacesUris of the file during the NodeSet loading process			
			// Variable de classe pour la manipulation des NamespaceUris
			std::vector<CNamespaceUri*>				m_NamespaceUris;
			CSessionDiagnosticsDataTypeList			m_SessionDiagnosticList; //Il s'agit d'un vecteur qui contient les OpcUa_SessionDiagnosticsDataType
			OpcUa_Mutex								m_SessionDiagnosticsMutex;
			CSessionSecurityDiagnosticsDataTypeList m_SessionSecurityDiagnosticList; // il s'agit d'un vecteur qui contient des OpcUa_SessionSecurityDiagnosticsDataType
			OpcUa_Mutex								m_SessionSecurityDiagnosticsMutex;
			CSubscriptionDiagnosticsDataTypeList 	m_SubscriptionDiagnosticList;	// on a un vecteur pour toutes les subscriptions prises en charge par le serveur
			OpcUa_Mutex								m_SubscriptionDiagnosticsMutex;

			CUAInformationModelFastAccessList		m_UAInformationModelFastAccessList; // Contient les elements de l'espace d'adressage en accés rapide. UAObject, UADataType, UAVariable, etc...
																						// cet variable de classe ne sert qu'a augmenter les performances du serveur et il ne contient que les nodeId du namespace 0			
			std::vector<OpcUa_SamplingIntervalDiagnosticsDataType*> m_SamplingIntervalDiagnosticsArray; // vector to store the SamplingIntervalDiagnosticsDataType for this server
			OpcUa_Mutex												m_SamplingIntervalDiagnosticsArrayMutex;
			OpcUa_ServerDiagnosticsSummaryDataType*					m_pServerDiagnosticsSummaryDataType;
		public:																				  
			OpcUa_Semaphore							m_PostThreatmentSem;	// Semaphore to synchronise some threatment after the postThreatment. Like Alarm definition
		public:
			OpcUa_StatusCode BuildApplicationUri(OpcUa_CharA** szApplicationUri);
			CUAReference* GetReferenceTarget(CUABase** pStartingNodeBase,OpcUa_NodeId aRefId,OpcUa_QualifiedName aTargetName);
			OpcUa_StatusCode TranslateRelativePathToNodeId(CUABase* pStartingNodeBase,OpcUa_RelativePath aRelativePath,OpcUa_UInt16* uiSucceeded,std::vector<OpcUa_ExpandedNodeId*>* pExpandedNodeIdList);					
		};
	}
}