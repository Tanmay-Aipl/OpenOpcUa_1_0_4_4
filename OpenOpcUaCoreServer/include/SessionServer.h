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
using namespace UASharedLib;
namespace OpenOpcUa
{
	namespace UACoreServer 
	{
		class CServerApplication;
		class CSessionServer: public CSessionBase
		{
		public:
			CSessionServer(void);
			CSessionServer(	
				UACoreServer::CServerApplication* pApplication,
				OpcUa_UInt32                  uSecureChannelId,
				OpcUa_UInt16			      securityMode,
				OpcUa_String				  securityPolicyUri,
				const OpcUa_ApplicationDescription* pClientDescription,
				const OpcUa_String*           pSessionName,
				const OpcUa_ByteString*       pClientNonce,
				const OpcUa_ByteString*       pClientCertificate,
				OpcUa_Double                  nRequestedSessionTimeout,
				OpcUa_UInt32                  nMaxResponseMessageSize,
				OpcUa_NodeId*                 pSessionId,
				OpcUa_NodeId*                 pAuthenticationToken,
				OpcUa_Double*                 pRevisedSessionTimeout,
				OpcUa_ByteString*             pServerNonce,
				OpcUa_SignatureData*          pServerSignature,
				OpcUa_UInt32*                 pMaxRequestMessageSize);
			~CSessionServer(void);
			OpcUa_StatusCode FindSubscription(OpcUa_UInt32 aSubscriptionId, CSubscriptionServer** ppSubscription);
			//
			OpcUa_StatusCode InitSessionDiagnosticsDataType();
			OpcUa_StatusCode InitSessionSecurityDiagnosticsDataType();
			OpcUa_StatusCode InitSubscriptionDiagnosticsDataType(CSubscriptionServer* pSubscription);
			OpcUa_StatusCode Reconnect(OpcUa_UInt32 uSecureChannelId);

			OpcUa_StatusCode Activate(
				OpcUa_UInt32							uSecureChannelId,
				OpcUa_UInt16							securityMode,
				OpcUa_String*							securityPolicyUri,
				OpcUa_Int32								nNoOfClientSoftwareCertificates, // in
				const OpcUa_SignedSoftwareCertificate*	pClientSoftwareCertificates, //in 
				const OpcUa_SignatureData*				pClientSignature,
				const OpcUa_ExtensionObject*            a_pUserIdentityToken,
				OpcUa_ByteString*						pServerNonce);

			//void Delete();

			OpcUa_Boolean IsAuthenticationToken(const OpcUa_NodeId* pAuthenticationToken);
			//create d'une souscription 
			OpcUa_StatusCode CreateSubscription(OpcUa_UInt32 uSecureChannelId,
												OpcUa_Double dblRequestedPublishingInterval,
												OpcUa_UInt32 uiRequestedLifetimeCount,
												OpcUa_UInt32 uiRequestedMaxKeepAliveCount,
												OpcUa_UInt32 uiMaxNotificationsPerPublish,
												OpcUa_Boolean bPublishingEnabled,
												OpcUa_Byte cPriority,
												CSubscriptionServer** pSubscription);
			
			OpcUa_StatusCode FindNextSubscriptionToNotify(CSubscriptionServer** ppSubscription);
			OpcUa_UInt32 GetMaxResponseMessageSize(){return m_nMaxResponseMessageSize;}
			void SetMaxResponseMessageSize(OpcUa_UInt32 uiMaxResponseMessageSize) {m_nMaxResponseMessageSize=uiMaxResponseMessageSize;}
			void StartAsyncRequestThread();
			OpcUa_StatusCode StopAsyncRequestThread();
			static void AsyncRequestThread(LPVOID arg);				
			DWORD GetAsyncThreadInterval();
			void SetAsyncThreadInterval(DWORD dwVal);
			void UpdateAsyncThreadInterval();
			// Thread to send notification data to the client and related functions
			static void NotificationDataThread(LPVOID arg);
			void UpdateNotificationDataThreadInterval();
			DWORD GetNotificationDataInterval();
			void WakeupNotificationDataThread();
			void StartNotificationDataThread();
			OpcUa_StatusCode StopNotificationDataThread();
			// Queues a HistoryRead Request for a later processing.
			OpcUa_StatusCode QueueHistoryReadMessage(CQueuedHistoryReadMessage* pRequest);
			// Process a Queued HistoryReadMessage
			void ProcessQueuedHistoryReadMessage(CQueuedHistoryReadMessage* pRequest, OpcUa_Boolean abort);
			// Process a history read message
			OpcUa_StatusCode ProcessHistoryReadMessage(CQueuedHistoryReadMessage* pHistoryReadMessage);
			// Queues a Read Request for later processing.
			OpcUa_StatusCode QueueReadMessage(CQueuedReadMessage* pRequest);
			// Processes a queued Read Message.
			void ProcessQueuedReadRequest(CQueuedReadMessage* pRequest, OpcUa_Boolean bAbort);
			// Processes a read request.		
			OpcUa_StatusCode ProcessReadRequest(CQueuedReadMessage* pReadMessage);
			// Queue Call Message
			OpcUa_StatusCode QueueCallMessage(CQueuedCallMessage* pCallMessage);
			// Processes a queued call request.		
			OpcUa_StatusCode ProcessQueuedCallRequest(CQueuedCallMessage* pCallMessage, OpcUa_Boolean bAbort);
			// Processes a Call request.		
			OpcUa_StatusCode ProcessCallRequest(CQueuedCallMessage* pCallMessage);
			// Suppression de toutes les appel de methode déjà traité (m_bDeleted=OpcUa_True)
			OpcUa_StatusCode RemoveAllCallRequestDeleted();
			// abort all call requests
			OpcUa_StatusCode RemoveAllCallRequest();
			// Queue the request receive in the BeginQueryFirst
			OpcUa_StatusCode QueueQueryFirstMessage(CQueuedQueryFirstMessage* pQueryFirstMessage);
			// Suppression de toutes les appel de QueryFirst déjà traité (m_bDeleted=OpcUa_True)
			OpcUa_StatusCode RemoveAllQueryFirstRequestDeleted();
			// abort all QueryFirst requests
			OpcUa_StatusCode RemoveAllQueryFirstRequest();
			// Process the QueryFirst Request (do the job)
			OpcUa_StatusCode ProcessQueuedQueryFirstRequest(CQueuedQueryFirstMessage* pQueryFirstMessage, OpcUa_Boolean bAbort);
			// Queue the request receive in the BeginQueryFirst
			OpcUa_StatusCode QueueQueryNextMessage(CQueuedQueryNextMessage* pQuestNextMessage);
			// Suppression de toutes les appel de QueryNext déjà traité (m_bDeleted=OpcUa_True)
			OpcUa_StatusCode RemoveAllQueryNextRequestDeleted();
			// abort all QueryNext requests
			OpcUa_StatusCode RemoveAllQueryNextRequest();
			// Process the QueryNext Request (do the job)
			OpcUa_StatusCode ProcessQueuedQueryNextRequest(CQueuedQueryNextMessage* pQueryNextMessage, OpcUa_Boolean bAbort);
			// abort all Read Requests.
			OpcUa_StatusCode RemoveAllReadRequest();
			// Suppression de toutes les requêtes de lecture déjà traité (m_bDeleted=OpcUa_True)
			OpcUa_StatusCode RemoveAllReadRequestDeleted();
			// Abort All HistoryRead request
			OpcUa_StatusCode RemoveAllHistoryReadRequest();
			// Suppression de toutes les requêtes de lecture d'historique déjà traité (m_bDeleted=OpcUa_True)
			OpcUa_StatusCode RemoveAllHistoryReadRequestDeleted();	
			// Remove a ReadRequest
			OpcUa_StatusCode RemoveReadRequest(CQueuedReadMessage* pReadRequest);
			// Processes publish requests verify, ack and queued .
			OpcUa_StatusCode ProcessPublishRequest(OpcUa_PublishRequest* pPublishRequest, OpcUa_Endpoint hEndpoint, OpcUa_Handle hContext, OpcUa_EncodeableType* pRequestType);
			OpcUa_StatusCode NotifyWhenNoSubscription(CQueuedPublishMessage* pPublishMessage);
			OpcUa_StatusCode AckPublishRequest(CQueuedPublishMessage* pPublishMessage);
			OpcUa_StatusCode QueuePublishRequest(CQueuedPublishMessage* pPublishMessage);
			// Get The first PublishRequest
			CQueuedPublishMessage* GetFirstPublishRequest();
			// Clean up the PublishRequest timeouted
			void CleanupTimeoutedPublishRequest();
			// Get The last PublishRequest
			CQueuedPublishMessage* GetLastPublishRequest();
			// lastTrasmissionTime manipulation
			void SetLastTransmissionTime(OpcUa_DateTime dtVal);
			OpcUa_DateTime GetLastTransmissionTime();
			// supprime toute les demande de publication
			void RemoveAllPublishRequest();
			// supprime les demandes de puiblication qui ont déjàt été traitée
			OpcUa_StatusCode RemoveAllPublishRequestDeleted();
			// accesseur for the sessionName pass by the client
			OpcUa_String GetSessionName();
			void SetSessionName(OpcUa_String*  pStrVal);
			OpcUa_StatusCode SetPublishingMode( OpcUa_Boolean              a_bPublishingEnabled,
												OpcUa_Int32                a_nNoOfSubscriptionIds,
												const OpcUa_UInt32*		   a_pSubscriptionIds,
												OpcUa_Int32*               a_pNoOfResults,
												OpcUa_StatusCode**         a_pResults);
			CSessionDiagnosticsDataType*	GetSessionDiagnosticsDataType(); // Return the OpcUa_SessionDiagnosticsDataType associate to this session
			CSessionSecurityDiagnosticsDataType*GetSessionSecurityDiagnosticsDataType();
			OpcUa_Double GetSessionTimeout();
			void SetSessionTimeout(OpcUa_Double val);
			// Fonction de gestion de souscription
			OpcUa_StatusCode AddSubscription(CSubscriptionServer* pSubscription);
			OpcUa_StatusCode RemoveSubscriptions();
			OpcUa_StatusCode RemoveSubscription(CSubscriptionServer* pSubscription);
			OpcUa_StatusCode RemoveSubscriptionBySubscriptionId(OpcUa_UInt32 SubscriptionId);
			OpcUa_StatusCode ModifySubscription(
										OpcUa_UInt32               a_nSubscriptionId,
										OpcUa_Double               a_nRequestedPublishingInterval,
										OpcUa_UInt32               a_nRequestedLifetimeCount,
										OpcUa_UInt32               a_nRequestedMaxKeepAliveCount,
										OpcUa_UInt32               a_nMaxNotificationsPerPublish,
										OpcUa_Byte                 a_nPriority,
										OpcUa_Double*              a_pRevisedPublishingInterval,
										OpcUa_UInt32*              a_pRevisedLifetimeCount,
										OpcUa_UInt32*              a_pRevisedMaxKeepAliveCount);
			CSecureChannel*	GetSecureChannel();
			void SetSecureChannel(CSecureChannel* pSecureChannel);
			OpcUa_Mutex GetPublishMutex();
			OpcUa_Semaphore GetPublishSem();
			OpcUa_UInt32 GetPublishRequestsSize();
			OpcUa_StatusCode Republish( OpcUa_UInt32 a_nSubscriptionId,
										OpcUa_UInt32 a_nRetransmitSequenceNumber,
										OpcUa_NotificationMessage** a_pNotificationMessage);
			//Register node in the server
			OpcUa_StatusCode RegisterNodes(OpcUa_Int32 a_nNoOfNodesToRegister,
											   const OpcUa_NodeId* a_pNodesToRegister,
											   OpcUa_Int32* a_pNoOfRegisteredNodeIds,
											   OpcUa_NodeId** a_pRegisteredNodeIds);
			// unregister node
			OpcUa_StatusCode UnregisterNodes(OpcUa_Int32 a_nNoOfNodesToUnregister,
											 const OpcUa_NodeId* a_pNodesToUnregister);
			OpcUa_Boolean IsSubscription(); // indique s'il y a des subscription dans cette session

			OpcUa_Boolean Is101Session();
			OpcUa_StatusCode ReadScalarExtensionObject(CUAVariable* pUAVariable,OpcUa_Int32 ii,OpcUa_ReadResponse* pResponse);
			OpcUa_Boolean IsActive(); // indique si cette session a été activée
			void StartSessionTimeoutThread();
			void StopSessionTimeoutThread();
			static void SessionTimeoutThread(LPVOID arg);
			OpcUa_Boolean IsTimeouted();// indique si la session est en timeout
			CServerApplication*	GetServerApplication();
			OpcUa_ByteString* GetServerNonce();
			OpcUa_ByteString* GetClientNonce();
			OpcUa_UInt32 GetSubscriptionListSize();
			OpcUa_Mutex GetSubscriptionListMutex();
			OpcUa_Double GetFastestSubscriptionPublishingInterval(); // Trouve la souscription qui a le plus rapide interval de publication pour cette session
			OpcUa_Boolean IsServerDiagnosticsEnabled(); // correspond au contenu du NodeId i=2294
			void EnableServerDiagnostics(OpcUa_Boolean bVal);// correspond au contenu du NodeId i=2294
			
			OpcUa_Semaphore GetStopSubscriptionsLifeTimeCountSem();
			OpcUa_Semaphore GetSubscriptionsLifeTimeCountSem();
			OpcUa_StatusCode AckCommentConfirmSessionEvents(CEventDefinition* pEventDefinition, const OpcUa_LocalizedText comment, METHOD_NAME eBehavior);
			OpcUa_StatusCode WakeupAllSubscription();
			void WakeupAsyncRequestThread();
			void SortSubscription();
			void SortSubscriptionsByPriority(OpcUa_Byte currentLowerPriority, CSubscriptionList* pSortedSubscriptionList);
		protected:
			OpcUa_StatusCode CallLuaMethod(CUAMethod* pUAMethodCalled, OpcUa_CallMethodRequest* pCallMethodRequest, OpcUa_CallResponse** ppResponse);
			// Ensemble de methode en charge de la surveillance des LifeTimeCount pour toutes les subscriptions de cette session
			void				StartSubscriptionsLifeTimeCountThread();
			OpcUa_StatusCode	StopSubscriptionsLifeTimeCount();
			static void			SubscriptionsLifeTimeCountThread(LPVOID arg);
		private:
			OpcUa_Boolean								m_bServerDiagnosticsEnabled;// correspond au contenu du NodeId i=2294
			// Gestion du timeout de la session
			// comme indiqué dans la spec sans activité depuis le client dans ce timeout le serveur doit détruire la session du client 
			OpcUa_Double								m_SessionTimeout;
			OpcUa_Thread								m_hSessionTimeoutThread;
			OpcUa_Boolean								m_bRunSessionTimeoutThread;
		public:
			OpcUa_Semaphore								m_SessionTimeoutSem; // This semaphore is post each time a service is called on the server. This allows to detect when the session is in timeout
			OpcUa_Semaphore								m_StopSessionTimeoutSem; // This semaphore is used to complete the stop of the SessionTimeoutThread
			// pour la prise en charge du ContinuationPoint
			CContinuationPointList*						m_pContinuationPointList; // ContinuationPoint associés a cette session
			std::vector<OpcUa_StatusCode>				m_StatusCodes;  // Attribute that hold the statusdChange to notify to the client. This is mainly use when serverStatus change and no subscription are alive.
																		// for instance when a lifetimecount is reach
		private:
			//  variable for all Async Request
			OpcUa_Thread								m_hAsyncRequestThread;
			OpcUa_Boolean								m_bRunAsyncRequestThread;
			DWORD										m_dwAsyncRequestThreadId;
			OpcUa_Semaphore								m_hStopAsyncRequestThreadSem;
			// Type experimental pour la prise en compte de 
			CSessionSecurityDiagnosticsDataType*		m_pSessionSecurityDiagnostics;
			CSessionDiagnosticsDataType*				m_pSessionDiagnostics;

			//
			OpcUa_Semaphore								m_hAsyncRequest; // Semaphore for the AsyncRequest thread
			//
			OpcUa_Semaphore								m_hNotificationDataSem;
			OpcUa_Semaphore								m_hStopNotificationDataThreadSem;
			DWORD										m_dwNotificationDataThreadInterval;
			OpcUa_Thread								m_hNotificationDataThread;
			OpcUa_Boolean								m_bRunNotificationDataThread;
			// 
			CServerApplication*							m_pApplication;  // serveur associé a cette session server
			CSecureChannel*								m_pSecureChannel; // SecureChannel associé à cette instance du serveur
			OpcUa_UInt32								m_nMaxResponseMessageSize; // Taille max de tous les messages de réponse que peut retourner le serveur.
																				   // Si la taille de la réponse dépasse cette limite le serveur devra retourner OpcUa_BadResponseTooLarge
			OpcUa_ByteString*							m_pServerNonce;
			OpcUa_ByteString*							m_pClientNonce; // Il s'agit du nonce passé par le client lors de la création de la session. Il ne sert que pour le CTT afin de s'assurer le l'unicité de cette demande.
			OpcUa_CryptoProvider						m_tCryptoProvider;
			OpcUa_Mutex									m_hSubscriptionListMutex;
			CSubscriptionList							m_SubscriptionList; // list of subscription handle by this Session
			DWORD										m_dwAsyncThreadInterval; // Rate of the Asyncthread Interval
			OpcUa_String								m_SessionName; // SessionName pass by the client
			// HistoryRead releated
			OpcUa_Mutex									m_hHistoryReadMutex;  // Object to protect the m_pHistoryReadMessages queue
			CQueuedHistoryReadMessages*					m_pHistoryReadMessages;
			// Read Releated
			OpcUa_Mutex									m_hReadMutex;  // Object to protect the m_pReadMessages queue
			CQueuedReadMessages*						m_pReadMessages; // AsyncRead Message Queue
			// Method call Related
			OpcUa_Mutex									m_hCallMutex;  // Object to protect the m_pCallMessages queue
			CQueuedCallMessages*						m_pCallMessages;
			// Publish related
			OpcUa_Semaphore								m_hPublishSem; // semaphore to synchronize access to the m_PublishRequests
			OpcUa_Mutex									m_hPublishMutex;  // Syncro object to protect m_PublishRequests
			CQueuedPublishMessages						m_PublishRequests; // Publish Request Queue
			// QueryFirst related
			OpcUa_Mutex									m_hQueryFirstMutex;  // Object to protect the m_pQueryFirstMessages queue
			CQueuedQueryFirstMessages*					m_pQueryFirstMessages;

			// QueryNext related
			OpcUa_Mutex									m_hQueryNextMutex;  // Object to protect the m_pQueryNextMessages queue
			CQueuedQueryNextMessages*					m_pQueryNextMessages;

			// Other variable
			OpcUa_Boolean								m_bSessionActive;
			OpcUa_Boolean								m_bGlobalInLate; // indique que l'une des souscriptions est en retard
			OpcUa_Thread								m_hSubscriptionsLifeTimeCountThread; // Thread en charge de la surveillance de Subscription.
																					  // Il s'agit de surveiller que le LifeTimeCount des subscriptions n'est pas atteint
																					  // s'il est atteint on arrête la souscription
			OpcUa_Boolean								m_bSubscriptionLifeTimeCountThread; 
			OpcUa_Semaphore								m_SubscriptionsLifeTimeCountSem; // permet de cadencer le fonctionnement de SubscriptionsLifeTimeCountThread
			OpcUa_Semaphore								m_hStopSubscriptionsLifeTimeCountThreadSem; // permet de controler l'arret de la SubscriptionsLifeTimeCountThread
			std::vector<OpcUa_NodeId>					m_RegistredNodes; // this is a vector that contains the list of nodeRegister by the client for this session.
																		  // Server are suppose to have best performance in dataSource Access for those nodes
			OpcUa_DateTime								m_LastTransmissionTime; // Last time we sent sometime to the client. It could be a keepAlive of a NotificationMessage
		};
		typedef vector<CSessionServer*> CSessionServerList;
	} // namespace AnsiCStackQuickstart

}