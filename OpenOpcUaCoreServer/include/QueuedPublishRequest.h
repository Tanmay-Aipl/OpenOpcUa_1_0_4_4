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


namespace OpenOpcUa
{
	namespace UACoreServer 
	{	
		class CQueuedPublishMessage :
			public CQueueRequest 
		{
		public:
			CQueuedPublishMessage();
			CQueuedPublishMessage(OpcUa_PublishRequest* pPublishRequest,OpcUa_Endpoint hEndpoint,OpcUa_Handle hContext,OpcUa_EncodeableType* pRequestType);
			~CQueuedPublishMessage();
			OpcUa_PublishRequest* GetInternalPublisRequest() {return m_pInternalPublishRequest;}

			OpcUa_StatusCode FillNotificationMessage(OpcUa_UInt32 uiCurrentSequenceNumber);


			OpcUa_StatusCode FillDataChangeNotificationMessageEx2(OpcUa_UInt32 uiSubscriptionId,
																OpcUa_Boolean bPublishingEnabled,
																OpcUa_UInt32 uiPos, // position courante dans le pDataChangeNotificationList
																CAvailableSequenceNumbers* pAvailableSequenceNumbers,
																OpcUa_UInt32 uiMaxNotificationsPerPublish,
																OpcUa_Boolean* pbKeepAlive); // I/O

			OpcUa_Boolean IsMoreToNotify(CAvailableSequenceNumbers* pAvailableSequenceNumbers,
										 CAvailableSequenceNumbers::iterator iteratorAvailableSequenceNumber,
										 OpcUa_UInt32 UAMonitoredItemNotificationList);

			OpcUa_StatusCode FillEventNotificationMessage(OpcUa_UInt32 uiSubscriptionId,
														  OpcUa_Boolean bPublishingEnabled,
														  OpcUa_UInt32 uiPos, // position courante dans le pEventNotificationListList
														  CUAEventNotificationListList* pEventNotificationListList,
														  OpcUa_UInt32 uiMaxNotificationsPerPublish,
														  OpcUa_Boolean* pbKeepAlive); // I/O
			OpcUa_StatusCode FillStatusChangeNotificationMessage(OpcUa_UInt32 uiSubscriptionId,
																OpcUa_Boolean bPublishingEnabled,
																OpcUa_UInt32 uiPos, // position courante dans le pDataChangeNotificationList
																CUAStatusChangeNotificationList* pStatusChangeNotificationList,
																OpcUa_UInt32 uiMaxNotificationsPerPublish,
																OpcUa_Boolean* pbKeepAlive); // I/O
			OpcUa_StatusCode FillStatusChangeNotificationMessage(OpcUa_StatusChangeNotification* pNotification); // Call from session to notify "Session Level Status Change"
			void AddSequenceNumber(OpcUa_UInt32 uiVal);
			OpcUa_PublishResponse* GetInternalPublishResponse() {return m_pInternalPublishResponse;}
			OpcUa_StatusCode FillResponseHeader();
			OpcUa_StatusCode FillAvailableSequenceNumbers();
			OpcUa_StatusCode FillStatusCodes();
			void SetPublishResponseSubscriptionId(OpcUa_UInt32 uiSubscriptionId);
			OpcUa_StatusCode CancelSendResponse();
			OpcUa_StatusCode BeginSendResponse();
			OpcUa_StatusCode EndSendResponse();
			OpcUa_StatusCode ResetPublishResponse();
			OpcUa_StatusCode EncodeableObjectDelete();
			void AddStatusCode(OpcUa_StatusCode uStatus)
			{
				m_StatusCodes.push_back(uStatus);
			}
			OpcUa_StatusCode GetServiceResult() {return m_ServiceResult;}
			void SetServiceResult(OpcUa_StatusCode aServiceResult) {m_ServiceResult= aServiceResult;}
			void SetKeepAlive(OpcUa_Boolean bVal) {m_bKeepAlive=bVal; }
			OpcUa_Boolean IsKeepAlive() { return m_bKeepAlive; }
			OpcUa_Boolean IsTimeouted();
		private:
			OpcUa_PublishRequest*			m_pInternalPublishRequest; // Requete transmise par le client
			OpcUa_PublishResponse*			m_pInternalPublishResponse; // Réponse transmis au client
			CUInt32Collection*				m_pUInt32s; // Liste de n° de séquence  transmettre
			std::vector<OpcUa_StatusCode>	m_StatusCodes; // contient les StatusCode qui seront transmis au client. Il seront mis en forme dans CQueuedPublishMessage::FillStatusCodes()
			OpcUa_Boolean					m_bKeepAlive; // Bool indiquant que on envoie un KeepAlive message pour cette souscirpion (usage interne pour n° des sequences)
			OpcUa_StatusCode				m_ServiceResult;
			OpcUa_Boolean					m_bEncodeableObjectDeleted;
			OpcUa_UInt32					m_uiNotificationCounter; // The number of notifications for this Publish.
																	 // This is the sum of monitoredItems in the DataChangeNotification and events in the EventNotificationList
																	 // See OPC UA 1.03 Part 4 Table 86
		};
		typedef std::vector<CQueuedPublishMessage*>	 CQueuedPublishMessages;
	}
}