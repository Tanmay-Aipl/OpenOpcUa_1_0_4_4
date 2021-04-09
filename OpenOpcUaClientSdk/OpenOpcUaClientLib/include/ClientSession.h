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


#include <vector>
#include <list>
using namespace std;
using namespace OpenOpcUa;
using namespace UASharedLib;
namespace OpenOpcUa
{
	namespace UACoreClient 
	{
		class CPendingPublish 
		{
		public:
			CPendingPublish(OpcUa_UInt64 TimeoutHint);
			~CPendingPublish();
			OpcUa_Boolean IsTimeouted();
			void SetDeleted(OpcUa_Boolean bVal);
		private:
			OpcUa_Handle	m_hPublish;
			OpcUa_DateTime	m_publishTime;
			OpcUa_UInt64	m_TimeoutHint;
			OpcUa_Boolean	m_bDeleted;
		};
		typedef std::vector<CPendingPublish*> CPendingPublishList;

		class CClientApplication; 
		//
		enum SESSION_STATE
		{
			SESSION_STATE_UNDETERMINED,
			SESSION_STATE_ON_ACTIVE,
			SESSION_STATE_ON_INACTIVE,
			SESSION_STATE_SUBSCRIBED
		} ;
		typedef std::list<OpcUa_NodeId*> OpcUa_NodeIdList;
		typedef std::vector<OpcUa_ReferenceDescription*> OpcUa_ReferenceDescriptionList;
		typedef std::list<OpcUa_BrowseDescription*> OpcUa_BrowseDescriptionList;
		typedef std::list<OpcUa_ByteString*> OpcUa_ByteStringList;
		typedef std::vector<OpcUa_ReadValueId*> OpcUa_ReadValueIdList;
		typedef std::vector<OpcUa_DataValue*> OpcUa_DataValueList;
		class CSessionClient;
		// A function used to received notifications when an asynchronous read completes.
		typedef OpcUa_StatusCode (PfnSessionClient_OnReadValue)(  
			CSessionClient*   pSession,
			OpcUa_Void*      pCallbackData,
			OpcUa_StatusCode uStatus,
			OpcUa_Variant*   pValue);

		//function signature to receive the notification when a new Publish callback is received
		typedef OpcUa_StatusCode (PfnSessionClient_OnPublish)(
			OpcUa_Channel           a_hChannel,
			OpcUa_Void*             a_pResponse,
			OpcUa_EncodeableType*   a_pResponseType,
			OpcUa_Void*             a_pCallbackData,
			OpcUa_StatusCode        a_uStatus);

		// Manages a session with a UA server for a client.
		class CSessionClient : public CSessionBase
		{
			//DECLARE_CHILD_DYNCLASS(Session,Client)
		public:
			// Creates a new session for the specified client.
			CSessionClient();
			CSessionClient(CClientApplication* pApplication);

			// Releases all resources owned by the object.
			~CSessionClient(void);
			
			// Creates the session.
			OpcUa_StatusCode Create(CEndpointDescription* pEndpoint, OpcUa_String* sessionName,OpcUa_Double nRequestedClientSessionTimeout = 0);

			// Activates the session.
			OpcUa_StatusCode Activate(OpcUa_EndpointDescription* pEndpointDescription,const char* AuthenticationMode,const char* sUserName,const char* sPassword);
			OpcUa_StatusCode Activate();
			// Adjust the number of publish sent to the server
			void AdjustPusblish(OpcUa_Int32 iVal);
			// Closes the session.
			OpcUa_StatusCode Close();
			// Write
			OpcUa_StatusCode Write(OpcUa_Int32 iNoNodeToWrite, OpcUa_WriteValue* pValue, OpcUa_StatusCode** pResults);
			// Begins an asynchronous read the value of the specified Node.
			OpcUa_StatusCode BeginReadValue(OpcUa_NodeId nodeId, PfnSessionClient_OnReadValue* pCallback, OpcUa_Void* pCallbackData);
			// Synchronous Read for any attribute
			OpcUa_StatusCode Read(OpcUa_ReadValueIdList ReadValues,OpcUa_TimestampsToReturn	eTimestampsToReturn,OpcUa_DataValueList* Results);
			// create the subscription
			OpcUa_StatusCode CreateSubscription(OpcUa_Double* dblPublishingInterval, // In/Out param
													  OpcUa_UInt32* uiLifetimeCount, // In/Out param
													  OpcUa_UInt32* uiMaxKeepAliveCount,// In/Out param
													  OpcUa_UInt32 uiMaxNotificationsPerPublish,
													  OpcUa_Boolean bPublishingEnabled,
													  OpcUa_Byte aPriority,
													  OpcUa_Handle* hSubscription);
			// delete the subscription
			OpcUa_StatusCode DeleteSubscription(CSubscriptionClient* pSubscription);
			// Get the subscription based on its Id
			CSubscription* GetSubscriptionById(OpcUa_UInt32 aSubscriptionId); 
			// Get the subscription based on its hSubscription (OpcUa_Handle)
			OpcUa_StatusCode GetSubscription(OpcUa_Handle hSubscription,CSubscriptionClient** pSubscription);
			// Browse 
			OpcUa_StatusCode Browse(OpcUa_Int32 a_nNoOfNodesToBrowse,
									const OpcUa_BrowseDescription*  pNodesToBrowse,
									OpcUa_ReferenceDescriptionList* pReferenceList);
			// récupère l'état de la session client side
			WORD GetSessionState();

			//OpcUa_NodeId GetAuthenticationToken() {return m_tAuthenticationToken;}
			OpcUa_StatusCode FindSubscription(OpcUa_UInt32 aSubscriptionId, CSubscriptionClient** ppSubscription);
			OpcUa_StatusCode FindSubscription(OpcUa_Handle hSubscription, CSubscriptionClient** ppSubscription);
			OpcUa_StatusCode RemoveSubscription(CSubscriptionClient* pSubscription);	
			// Methode en charge de la notification du shutdwn de la session
			PFUNCSHUTDOWN GetShutdownCallback();
			void SetShutdownCallback(PFUNCSHUTDOWN pShutdownCallback);
			void* GetShutdownCallbackData();
			void SetShutdownCallbackData(void* pCallbackData);
			OpcUa_StatusCode StartWatchingThread();
			OpcUa_StatusCode StopWatchingThread();
			static void WatchingThread(LPVOID arg); 
			OpcUa_StatusCode NotifyStatusChange(OpcUa_StatusCode uLastNotifiedStatus);
			CChannel* GetChannel();
			void SetChannel(CChannel* pChannel);
			OpcUa_Handle GetApplicationHandle();
			OpcUa_StatusCode DeleteAllSubscriptions();
			///////////////////////////////////////////////////
			//
			// Pris en compte du mecansime de publication
			OpcUa_StatusCode StopPublishingThread();
			void StartPublishingThread();
			static void PublishingThread(LPVOID arg);
			
			// Etat interne du serveur
			  OpcUa_StatusCode GetInternalServerStatus();
			void SetInternalServerStatus(OpcUa_StatusCode uStatus);
			// retrouve l'interval de publication le plus rapide pour les souscrioption prise en compte pour cette session
			OpcUa_StatusCode GetFastestPublishInterval(OpcUa_Double* dblFastestInterval);
			OpcUa_UInt32 GetSequenceNumber();
			void SetSequenceNumber(OpcUa_UInt32 uiVal);
			CClientApplication* GetClientApplication();
			// Pending publish v1.0.2.0>
			void IncPendingPublish();
			void DecPendingPublish();
			OpcUa_UInt32 GetPendingPublish();
			// PendingPublish management v1.0.4.0
			void AddPendingPublish(CPendingPublish* pPendingPublish);
			void RemovePendingPublish(CPendingPublish* pPendingPublish);
			void RemoveAllPendingPublish();
			OpcUa_UInt32 GetPendingPublishSize();
			void CleanupPendingPublish();
			//
			OpcUa_Boolean IsWatchingThreadDetectError();
			OpcUa_StatusCode GetXmlAttributes(vector<OpcUa_String*>* szAttributes);
			OpcUa_StatusCode GetSubscriptions(OpcUa_UInt32* uiNoOfSessions, OpcUa_Handle** hSubscriptions);
			OpcUa_StatusCode  GetSubscriptionOfMonitoredItem(OpcUa_Handle hMonitoredItem, CSubscriptionClient** pSubscriptionClient);
			// 
		private:
			OpcUa_Thread			m_hInternalWatchingThread;
			OpcUa_Semaphore			m_WatchingSem;
			OpcUa_Mutex				m_hWatchingMutex;
			OpcUa_Boolean			m_bRunWatchingThread;
			OpcUa_Boolean			m_bFromWatchingThread;
			OpcUa_StatusCode		m_InternalServerStatus; // contient l'etat du serveur reception par le mécanisme de notification (publish/callback)
			OpcUa_Int32				m_AdjustPublishVal;
		public:
			OpcUa_Mutex				m_SubscriptionListMutex;
			CSubscriptionList		m_SubscriptionList; // list of subscription handle by this Session for the client application
			OpcUa_StatusCode		m_internalPublishStatus;
		private:
			// Deletes the session.
			void Delete();
			WORD					m_wSessionState; // etat de la session géré localement. Cette varible est une assistant.
									// Elle prend les valeurs suivantes
									// 0 undeterminé
									// 1 En session Active(connecté)
									// 2 En session Inactive (connecté)
									// 3 Subscribed. Indique, qu'une ou plusieurs souscriptions sont active
			CClientApplication*		m_pApplication;
			OpcUa_Double			m_nSessionTimeout;
			OpcUa_ByteString		m_tServerNonce;
			OpcUa_ByteString		m_tServerCertificate;
			OpcUa_CryptoProvider	m_tCryptoProvider;
			PFUNCSHUTDOWN			m_pShutdownCallback;
			void*					m_pShutdownCallbackData;
			OpcUa_Mutex				m_ChannelMutex;
			CChannel*				m_pChannel;
			// variable for the publishing thread
			OpcUa_Thread			m_hPublishingThread;
			BOOL					m_bRunPublishingThread;
			DWORD					m_dwPublishingThreadId;
			OpcUa_Semaphore			m_hStopPublishingThread;
			OpcUa_UInt32			m_uiSequenceNumber; // Default sequence number to send to the server i publish. This is use in case there not subscription
			OpcUa_UInt32			m_uiPendingPublish; // pending publish for this session. This is used to deleted properly the session, all its subscriptions and monitoredItems
			OpcUa_Mutex				m_PendingPublishListMutex;
			CPendingPublishList		m_PendingPublishList;
		};
		typedef std::vector<CSessionClient*> CSessionClientList;

	} // namespace UACoreClient

}
