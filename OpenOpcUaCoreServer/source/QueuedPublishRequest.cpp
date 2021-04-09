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
#include "Field.h"
#include "Definition.h"
#include "UAVariableType.h"
#include "UAMethod.h"
#include "UAView.h"
#include "UADataType.h"
#include "UAObjectType.h"
#include "UAObject.h"
#include "UAMonitoredItemNotification.h"
#include "UADataChangeNotification.h"
#include "EventDefinition.h"
using namespace UAEvents;
#include "MonitoredItemServer.h"
#include "UAEventNotificationList.h"
#include "UAStatusChangeNotification.h"
#include "QueuedPublishRequest.h"
using namespace OpenOpcUa;
using namespace UACoreServer;

//OpcUa_UInt32 PublishCounter = 0;
//OpcUa_UInt32 PublishRequestCounter = 0;
//OpcUa_UInt32 PublishResponseCounter = 0;

CQueuedPublishMessage::CQueuedPublishMessage()
{
	m_pInternalPublishRequest=OpcUa_Null;
	m_pInternalPublishResponse=OpcUa_Null;
	m_pUInt32s=new CUInt32Collection();
	m_ServiceResult=OpcUa_Good;
	m_bKeepAlive=OpcUa_False;
	m_bEncodeableObjectDeleted = OpcUa_False;
	m_uiNotificationCounter = 0;
}
CQueuedPublishMessage::CQueuedPublishMessage(OpcUa_PublishRequest* pPublishRequest,OpcUa_Endpoint hEndpoint,OpcUa_Handle hContext,OpcUa_EncodeableType* pRequestType)
					:CQueueRequest(hEndpoint,hContext,pRequestType)
{
	m_bEncodeableObjectDeleted = OpcUa_False;
	m_bKeepAlive=OpcUa_False;
	m_pUInt32s=OpcUa_Null;
	m_pInternalPublishResponse=OpcUa_Null;
	m_ServiceResult=OpcUa_Good;
	m_uiNotificationCounter = 0;
	m_pResponseType = OpcUa_Null;
	if (pPublishRequest)
	{
		OpcUa_EndpointContext* pContext = (OpcUa_EndpointContext*)GetContext();	
		if (pContext)
		{
			// Allocation du CUInt32Collection*		
			m_pUInt32s=new CUInt32Collection();
			m_pResponseType = pContext->ServiceType.ResponseType;
		}
		else
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Critical error CQueuedPublishMessage\n");
		if (pPublishRequest->NoOfSubscriptionAcknowledgements==0)
			pPublishRequest->SubscriptionAcknowledgements=OpcUa_Null;
		m_pInternalPublishRequest=pPublishRequest;
	}
	else
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Critical error CQueuedPublishMessage\n");
}
CQueuedPublishMessage::~CQueuedPublishMessage()
{
	if (m_pInternalPublishRequest)
	{
		OpcUa_PublishRequest_Clear(m_pInternalPublishRequest);
		OpcUa_EncodeableObject_Delete(GetRequestType(), (OpcUa_Void**)&m_pInternalPublishRequest);
		m_pInternalPublishRequest = OpcUa_Null;
	}
	
	if (m_pInternalPublishResponse)
	{
		OpcUa_PublishResponse_Clear(m_pInternalPublishResponse);
		OpcUa_EncodeableObject_Delete(m_pResponseType, (OpcUa_Void**)&m_pInternalPublishResponse);
		m_pInternalPublishResponse = OpcUa_Null;
	}
	if (m_pUInt32s)
	{
		m_pUInt32s->clear();
		delete m_pUInt32s;
		m_pUInt32s = OpcUa_Null;
	}
	std::vector<OpcUa_StatusCode>::iterator it;
	while (!m_StatusCodes.empty())
	{
		it=m_StatusCodes.begin();
		m_StatusCodes.erase(it);
	}
	m_StatusCodes.clear();
}
OpcUa_StatusCode CQueuedPublishMessage::BeginSendResponse()
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	m_pResponseType = OpcUa_Null;
	// Create a context to use for sending a response.
	uStatus = OpcUa_Endpoint_BeginSendResponse(GetEndpoint(), GetContext(), (OpcUa_Void**)&m_pInternalPublishResponse, &m_pResponseType);
	return uStatus;
}
OpcUa_StatusCode CQueuedPublishMessage::EndSendResponse()
{
	OpcUa_StatusCode uStatus=OpcUa_BadInvalidArgument;
	if (m_pResponseType)
	{		
		OpcUa_Handle pContextHandle=GetContext();
		if (m_pInternalPublishResponse)
		{
			if (m_pInternalPublishResponse->SubscriptionId == 0)
			{
				///////////////////////////////////////////////////////
				m_pInternalPublishResponse->ResponseHeader.RequestHandle = m_pInternalPublishRequest->RequestHeader.RequestHandle;
				m_pInternalPublishResponse->ResponseHeader.Timestamp = OpcUa_DateTime_UtcNow();
				//////////////////////////////////////////////////////
				m_pInternalPublishResponse->ResponseHeader.ServiceResult = OpcUa_BadNoSubscription ;// ; // OpcUa_Good;
			}
			uStatus = OpcUa_Endpoint_EndSendResponse(   
				GetEndpoint(),
				&pContextHandle,
				OpcUa_Good,
				m_pInternalPublishResponse,
				m_pResponseType);
		}
	}
	else
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_WARNING, "CQueuedPublishMessage::EndSendResponse>m_pResponseType is Null. Something goes wrong during the initialisation\n");
	return uStatus;
}
OpcUa_StatusCode CQueuedPublishMessage::CancelSendResponse()
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_Handle pContextHandle=GetContext();
	OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"CancelSendResponse pContextHandle=0x%x\n",pContextHandle);
	SetDeleted(OpcUa_True); // First we mark the Request as deleted
	uStatus=OpcUa_Endpoint_CancelSendResponse( 
		GetEndpoint(),
		uStatus,
		OpcUa_Null,
		&pContextHandle);
	if (uStatus==OpcUa_Good)
		m_hContext = OpcUa_Null;
	return uStatus;
}

// Function name   : CQueuedPublishMessage::FillResponseHeader
// Description     : Mise en forme de l'entête qui sera retourné dans le message de réponse au client
// Return type     : OpcUa_StatusCode 

OpcUa_StatusCode CQueuedPublishMessage::FillResponseHeader()
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	m_pInternalPublishResponse->ResponseHeader.Timestamp= OpcUa_DateTime_UtcNow();
	m_pInternalPublishResponse->ResponseHeader.RequestHandle = m_pInternalPublishRequest->RequestHeader.RequestHandle;
	m_pInternalPublishResponse->ResponseHeader.ServiceResult = m_ServiceResult; 
	return uStatus;
}
OpcUa_StatusCode CQueuedPublishMessage::FillStatusCodes()
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_UInt32 uiNbStatusCode=m_StatusCodes.size();
	if (m_pInternalPublishResponse->Results==OpcUa_Null)
	{
		if (uiNbStatusCode)
		{
			m_pInternalPublishResponse->Results = (OpcUa_StatusCode*)OpcUa_Alloc(uiNbStatusCode*sizeof(OpcUa_StatusCode));
			ZeroMemory(m_pInternalPublishResponse->Results, (uiNbStatusCode*sizeof(OpcUa_StatusCode)));
		}
	}
	m_pInternalPublishResponse->NoOfResults=uiNbStatusCode;
	for (OpcUa_UInt32 ii=0;ii<uiNbStatusCode;ii++)
		m_pInternalPublishResponse->Results[ii]=m_StatusCodes.at(ii);
	return uStatus;
}
//
// Function name   : CQueuedPublishMessage::FillAvailableSequenceNumbers
// Description     :  fabrique la liste des AvailableSequenceNumbers.
// Cette liste sera mis en forme dans un OpcUa_Int32* puis transmise au client 
// Cette liste est composé des n° de sequence transmise et non acquité par le client
// Return type     : OpcUa_StatusCode 
OpcUa_StatusCode CQueuedPublishMessage::FillAvailableSequenceNumbers()
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_UInt32 nbOfSequenceNumbers=0;
	if (m_pUInt32s)
	{
		OpcUa_UInt32* uiSequenceNumbers=OpcUa_Null;
		if (m_pUInt32s->empty())
		{
			nbOfSequenceNumbers=0;
			uiSequenceNumbers=OpcUa_Null;
		}
		else
		{
			// transformation de la collection en OpcUa_UInt32*
			nbOfSequenceNumbers=m_pUInt32s->size();
			uiSequenceNumbers=(OpcUa_UInt32*)OpcUa_Alloc(sizeof(OpcUa_UInt32)*nbOfSequenceNumbers);
			if (uiSequenceNumbers)
			{
				for (OpcUa_UInt32 ii = 0; ii < nbOfSequenceNumbers; ii++)
					uiSequenceNumbers[ii] = m_pUInt32s->at(ii);

				m_pInternalPublishResponse->AvailableSequenceNumbers = (OpcUa_UInt32*)OpcUa_Alloc(sizeof(OpcUa_UInt32)*nbOfSequenceNumbers);
				OpcUa_MemCpy(
					m_pInternalPublishResponse->AvailableSequenceNumbers,
					sizeof(OpcUa_UInt32)*nbOfSequenceNumbers,
					uiSequenceNumbers,
					sizeof(OpcUa_UInt32)*nbOfSequenceNumbers);
				OpcUa_Free(uiSequenceNumbers);
			}
		}
		m_pInternalPublishResponse->NoOfAvailableSequenceNumbers=nbOfSequenceNumbers;
	}
	else
		uStatus=OpcUa_BadInvalidArgument;

	return uStatus;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Prepare the notification message . This involve the following 4 actions
/// 			Set the NotificationMessage sequence number
/// 			Fill the response header
/// 			Fill the available sequence number
/// 			Fill the statuscode </summary>
///
/// <remarks>	Michel, 29/01/2016. </remarks>
///
/// <param name="uiCurrentSequenceNumber">	The current sequence number. </param>
///
/// <returns>	An OpcUa_StatusCode : OpcUa_Good if the method work properly. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode CQueuedPublishMessage::FillNotificationMessage(OpcUa_UInt32 uiCurrentSequenceNumber)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	if (m_pInternalPublishResponse)
	{
		//if (!m_bKeepAlive)
		//{
		//	if (m_pUInt32s->empty()) // Just in case it was not filled in the calling function
		//	{
		//		AddSequenceNumber(uiCurrentSequenceNumber);
		//	}
		//}
		//if (uiCurrentSequenceNumber==0)		
		//	uiCurrentSequenceNumber=1;
		if (m_pInternalPublishResponse->NotificationMessage.NoOfNotificationData==0)
			m_pInternalPublishResponse->NotificationMessage.SequenceNumber = uiCurrentSequenceNumber+1;
		else
			m_pInternalPublishResponse->NotificationMessage.SequenceNumber=uiCurrentSequenceNumber; 
		uStatus=FillResponseHeader();
		// liste des sequences
		uStatus=FillAvailableSequenceNumbers();
		// result code
		uStatus=FillStatusCodes();

		// DiagnosticInfos
		m_pInternalPublishResponse->NoOfDiagnosticInfos=0;
		m_pInternalPublishResponse->DiagnosticInfos=OpcUa_Null;
	}
	else
		uStatus=OpcUa_BadInvalidArgument;
	return uStatus;
}
OpcUa_StatusCode CQueuedPublishMessage::FillEventNotificationMessage(OpcUa_UInt32 uiSubscriptionId,
																	 OpcUa_Boolean bPublishingEnabled,
																	 OpcUa_UInt32 uiPos, // position courante dans le pDataChangeNotificationList
																	 CUAEventNotificationListList* pEventNotificationListList,
																	 OpcUa_UInt32 uiMaxNotificationsPerPublish,
																	 OpcUa_Boolean* pbKeepAlive) // out
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_UInt32 iNoOfEventNotificationToSend = 0;
	OpcUa_DateTime PublishTime;
	OpcUa_DateTime_Initialize(&PublishTime);
	//OpcUa_Trace(OPCUA_TRACE_LEVEL_WARNING, "CQueuedPublishMessage::FillEventNotificationMessage> MaxNotificationsPerPublish=%u\n", uiMaxNotificationsPerPublish);
	if (m_pInternalPublishResponse)
	{
		m_pInternalPublishResponse->SubscriptionId = uiSubscriptionId;
		m_pInternalPublishResponse->ResponseHeader.RequestHandle = m_pInternalPublishRequest->RequestHeader.RequestHandle;
		// Check if we have something to do
		if (bPublishingEnabled)
		{
			// We have to determine the number of CUAEventNotificationList to send for this subscription (iNoOfEventNotificationToSend)
			OpcUa_UInt32 ii = 0;
			for (ii = uiPos; ii < pEventNotificationListList->size(); ii++)
			{
				CUAEventNotificationList* pEventNotificationList = pEventNotificationListList->at(ii);
				if (!pEventNotificationList->IsKeepAlive() /*&& (!pEventNotificationList->IsTransactionAcked()) */&& (pEventNotificationList->IsEventListNotifiable())) // s'il ne s'agit pas d'un KeepAlive
				{
					(*pbKeepAlive) = OpcUa_False;
					// We have to determine the number of CUAEventNotificationList to send for this subscription (iNoOfEventNotificationToSend)
					iNoOfEventNotificationToSend++;
				}
			}
		}
		if (iNoOfEventNotificationToSend > 0)
		{
			// Allocation de l'extensionObject. 
			// d'après la spécification UA 1.01 part 4 table 151. un seul extensionObject par type de Notification à transmettre. 
			// On alloue 1 ExtensionObject pour l'ensemble des Notifications
			// ATTENTION pour le support de AC nous utiliserons une autre extensionObject
			if (m_pInternalPublishResponse->NotificationMessage.NotificationData == OpcUa_Null)
				m_pInternalPublishResponse->NotificationMessage.NotificationData =
				(OpcUa_ExtensionObject*)OpcUa_Alloc(sizeof(OpcUa_ExtensionObject));
			else
			{
				OpcUa_ExtensionObject* pNewExtensionObject = OpcUa_Null;
				OpcUa_UInt32 iNewSize = m_pInternalPublishResponse->NotificationMessage.NoOfNotificationData + 1;
				pNewExtensionObject = (OpcUa_ExtensionObject*)OpcUa_ReAlloc(m_pInternalPublishResponse->NotificationMessage.NotificationData, sizeof(OpcUa_ExtensionObject)*iNewSize);
				if (pNewExtensionObject)
					m_pInternalPublishResponse->NotificationMessage.NotificationData = pNewExtensionObject;
			}

			if (!(*pbKeepAlive) && (iNoOfEventNotificationToSend > 0))
			{
				// préparation d'un OpcUa_EventNotificationList. Ils seront placés dans un NotificationData (OpcUa_ExtensionObject)
				OpcUa_EventNotificationList* pNotification =
					(OpcUa_EventNotificationList*)OpcUa_Alloc(sizeof(OpcUa_EventNotificationList));
				// On positionnera l'extensionObject contenant les event dans NotificationData[0], NotificationData[1], NotificationData[2]
				// en fonction du nombre d'evenement déjà transmis lors de l'appel à 
				//	FillDataChangeNotificationMessage, 
				//	FillEventNotificationMessage, 
				// 	FillStatusChangeNotificationMessage
				// Question ClientHandle ? base on the part 4 table 149 :Client-supplied handle for the MonitoredItem. The IntegerId type is defined in 7.13.
				// NoOfEventFields ? EventFields ?
				pNotification->Events = (OpcUa_EventFieldList*)OpcUa_Alloc(sizeof(OpcUa_EventFieldList)*iNoOfEventNotificationToSend);
				pNotification->NoOfEvents = iNoOfEventNotificationToSend;
				OpcUa_Int32 iii = 0;
				for (OpcUa_UInt32 ii = uiPos; ii < pEventNotificationListList->size(); ii++)
				{
					CUAEventNotificationList* pEventNotificationList = pEventNotificationListList->at(ii);
					if (pEventNotificationList)
					{
						if (!pEventNotificationList->IsKeepAlive() /*&& (!pEventNotificationList->IsTransactionAcked())*/ && (pEventNotificationList->IsEventListNotifiable()))
						{
							FastEventFieldIndex aFastEventFieldIndex= pEventNotificationList->GetFastEventFieldIndex();
							OpcUa_EventFieldList_Initialize(&(pNotification->Events[iii]));
							//
							CEventDefinition* pEventDefinition=pEventNotificationList->GetEventDefinition();
							//
							CMonitoredItemServer* pMonitoredItemServer = pEventNotificationList->GetMonitoredItem();
							if (pMonitoredItemServer)
							{
								OpcUa_EventNotificationList* pInternalEventNotificationList = pEventNotificationList->GetInternalEventNotificationList();
								if (pInternalEventNotificationList)
								{
									for (OpcUa_Int32 iEvt = 0; iEvt < pInternalEventNotificationList->NoOfEvents; iEvt++)
									{
										pNotification->Events[iii].ClientHandle = pInternalEventNotificationList->Events[iEvt].ClientHandle;
										pNotification->Events[iii].NoOfEventFields = pInternalEventNotificationList->Events[iEvt].NoOfEventFields;
										pNotification->Events[iii].EventFields = (OpcUa_Variant*)OpcUa_Alloc(sizeof(OpcUa_Variant)*pNotification->Events[iii].NoOfEventFields);
										for (OpcUa_Int32 iEvtField = 0; iEvtField < pNotification->Events[iii].NoOfEventFields; iEvtField++)
										{
											if (aFastEventFieldIndex.iMessage == iEvtField)
												pEventNotificationList->UpdateInternalMessage();
											OpcUa_Variant_Initialize(&(pNotification->Events[iii].EventFields[iEvtField]));
											OpcUa_Variant_CopyTo(&(pInternalEventNotificationList->Events[iEvt].EventFields[iEvtField]), &(pNotification->Events[iii].EventFields[iEvtField]));
										}
									}
									// Inc the pNotification->Events counter
									iii++;
									pEventNotificationList->SetTransactionStatus(OPCUA_EVENTSTATUS_SENDTOCLIENT);
									pEventDefinition->SetTransactionStatus(OPCUA_EVENTSTATUS_SENDTOCLIENT);
								}
							}
							//
							if (pEventNotificationList->GetLastNotifiedActiveState() != pEventDefinition->GetActiveState())
							{
								pEventDefinition->OnStage(OpcUa_False);
								pEventNotificationList->SetLastNotifiedActiveState(pEventDefinition->GetActiveState());
							}
							// 
							if (pEventNotificationList->IsAlarmAcked() && (pEventNotificationList->GetLastNotifiedAlarmAcked() != pEventNotificationList->IsAlarmAcked()))
							{
								pEventNotificationList->SetLastNotifiedAlarmAcked(OpcUa_True);
								if (pEventDefinition->GetActiveState() == OpcUa_False)
									pEventNotificationList->Terminate();
							}
							// 
							if (pEventNotificationList->IsConfirmed() && (pEventNotificationList->GetLastNotifiedConfirmed() != pEventNotificationList->IsConfirmed()))
								pEventNotificationList->SetLastNotifiedConfirmed(OpcUa_True);
						}
					}
				}
				pNotification->NoOfEvents = iii;
				m_pInternalPublishResponse->NotificationMessage.PublishTime = PublishTime; // We have to handle the publishTime in the same way we did in DataChangeNotification
				////////////////////////////////////////////////////////////////////////////////////////////
				// Mise en forme du résultat
				OpcUa_Int32 iIndex = m_pInternalPublishResponse->NotificationMessage.NoOfNotificationData;
				OpcUa_ExtensionObject_Initialize(&(m_pInternalPublishResponse->NotificationMessage.NotificationData[iIndex]));
				//NodeId
				m_pInternalPublishResponse->NotificationMessage.NotificationData[iIndex].TypeId.NodeId.Identifier.Numeric = OpcUaId_EventNotificationList_Encoding_DefaultBinary;
				m_pInternalPublishResponse->NotificationMessage.NotificationData[iIndex].TypeId.NodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
				// Encoding
				m_pInternalPublishResponse->NotificationMessage.NotificationData[iIndex].Encoding = OpcUa_ExtensionObjectEncoding_EncodeableObject;
				m_pInternalPublishResponse->NotificationMessage.NotificationData[iIndex].Body.EncodeableObject.Type = &OpcUa_EventNotificationList_EncodeableType;
				// Embedded object
				m_pInternalPublishResponse->NotificationMessage.NotificationData[iIndex].Body.EncodeableObject.Object = pNotification;
				m_pInternalPublishResponse->NotificationMessage.NoOfNotificationData++; // 			
			}
		}
	}
	return uStatus;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	 Préparation du message de notification pour les Messages relatifs aux CUADataChangeNotification
///				Ces CUADataChangeNotification seront placés dans la variable de classe 
///				m_pInternalPublishResponse->NotificationMessage.NotificationData
///				C'est le m_pInternalPublishResponse qui sera transmis au client.</summary>
///
/// <remarks>	Michel, 05/02/2016. </remarks>
///
/// <param name="uiSubscriptionId">			   	Identifier for the subscription. </param>
/// <param name="bPublishingEnabled">		   	The publishing enabled. </param>
/// <param name="uiPos">					   	The position. </param>
/// <param name="pAvailableSequenceNumbers">   	[in,out] List of CUADataChangeNotification to put in the notification message. </param>
/// <param name="uiMaxNotificationsPerPublish">	Max notification per publish on the subscription. </param>
/// <param name="pbKeepAlive">				   	[in,out] if one of the DataChangeNotification in pDataChangeNotificationList
///												 is not a keepAlive the will be set to OpcUa_False. </param>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode CQueuedPublishMessage::FillDataChangeNotificationMessageEx2(OpcUa_UInt32 uiSubscriptionId,
															OpcUa_Boolean bPublishingEnabled,
															OpcUa_UInt32 uiPos, // current position in the pDataChangeNotificationList
															CAvailableSequenceNumbers* pAvailableSequenceNumbers,
															OpcUa_UInt32 uiMaxNotificationsPerPublish,
															OpcUa_Boolean* pbKeepAlive) // out
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_Boolean bMoreNotification = OpcUa_False;
	OpcUa_DateTime PublishTime;
	OpcUa_UInt32 uiLocalMaxNotificationsPerPublish = 0;
	// Prepare the OpcUa_DataChangeNotification. It will be place in the NotificationData (OpcUa_ExtensionObject)

	// Hold the queueSize for each MonitoredItem
	// We use a map where the key is the ClientHandle
	map<OpcUa_UInt32, OpcUa_UInt32> uiQueueSizes;
	if ((pAvailableSequenceNumbers->size() > 0) || (pbKeepAlive))
	{
		OpcUa_DateTime_Initialize(&PublishTime);
		if (m_pInternalPublishResponse)
		{
			OpcUa_DataChangeNotification* pNotification = (OpcUa_DataChangeNotification*)OpcUa_Alloc(sizeof(OpcUa_DataChangeNotification));
			if (pNotification)
			{
				if (uiMaxNotificationsPerPublish == 0)
					uiLocalMaxNotificationsPerPublish = OpcUa_UInt32_Max;
				else
					uiLocalMaxNotificationsPerPublish = uiMaxNotificationsPerPublish;
				SetPublishResponseSubscriptionId(uiSubscriptionId);
				m_pInternalPublishResponse->ResponseHeader.RequestHandle = m_pInternalPublishRequest->RequestHeader.RequestHandle;
				//
				OpcUa_UInt32 iIndexNotification = 0; // Index for NotificationData ie: m_pInternalPublishResponse->NotificationMessage.NotificationData[]
				OpcUa_UInt32 iIndexMonitoredItem = 0; // Index for MonitoredItem ie: pNotification->MonitoredItems[]
				std::vector<OpcUa_MonitoredItemNotification*> preparedMonitoredItemNotificationList; // List of OpcUa_MonitoredItemNotification prepare before transmission

				// Allocation of an extensionObject. 
				// according to the spécification UA 1.01 part 4 table 151. un seul extensionObject par type de Notification à transmettre. 
				// On alloue 1 ExtensionObject pour l'ensemble des Notifications
				// ATTENTION pour le support de AC nous utiliserons une autre extensionObject
				if (m_pInternalPublishResponse->NotificationMessage.NotificationData == OpcUa_Null)
					m_pInternalPublishResponse->NotificationMessage.NotificationData =
										(OpcUa_ExtensionObject*)OpcUa_Alloc(sizeof(OpcUa_ExtensionObject));
				else
				{
					OpcUa_ExtensionObject* pNewExtensionObject = OpcUa_Null;
					OpcUa_UInt32 iNewSize = m_pInternalPublishResponse->NotificationMessage.NoOfNotificationData + 1;
					pNewExtensionObject = (OpcUa_ExtensionObject*)OpcUa_ReAlloc(m_pInternalPublishResponse->NotificationMessage.NotificationData, sizeof(OpcUa_ExtensionObject)*iNewSize);
					if (pNewExtensionObject)
						m_pInternalPublishResponse->NotificationMessage.NotificationData = pNewExtensionObject;
				}
				/////////////////////////////////////////////////////////////////////////////// 
				// First step
				// in order to prepare the preparedMonitoredItemNotificationList
				// we go through DataChangeNotificationListand MonitoredItemNotificationList
				// to retrieve the MonitoredItemNotification to send to the client in the context 
				// of the current subscription
				OpcUa_Boolean bPreKeepAlive = OpcUa_True;
				CAvailableSequenceNumbers::iterator iteratorAvailableSequenceNumber;
				for (iteratorAvailableSequenceNumber = pAvailableSequenceNumbers->begin(); iteratorAvailableSequenceNumber != pAvailableSequenceNumbers->end(); iteratorAvailableSequenceNumber++)
				{
					CUADataChangeNotification* pDataChangeNotification = iteratorAvailableSequenceNumber->second;
					if (pDataChangeNotification)
					{
						if (pDataChangeNotification->IsSomethingToNotify())
						{
							bPreKeepAlive = OpcUa_False;
							if (!bMoreNotification)
							{
								CUAMonitoredItemNotificationList aUAMonitoredItemNotificationList = pDataChangeNotification->GetUAMonitoredItemNotificationList();
								CUAMonitoredItemNotificationList::iterator it = aUAMonitoredItemNotificationList.begin();
								for (OpcUa_UInt32 i = 0; i < aUAMonitoredItemNotificationList.size(); i++)
								{
									(*pbKeepAlive) = OpcUa_False;
									if (!bMoreNotification)
									{
										CUAMonitoredItemNotification* pUAMonitoredItemNotification = aUAMonitoredItemNotificationList.at(i);
										if (pUAMonitoredItemNotification)
										{
											//OpcUa_UInt32 uiMaxQueueSize = pUAMonitoredItemNotification->GetQueueSize();
											// 
											if (!pUAMonitoredItemNotification->IsMonitoredNotificationSent()) // perf purpose only
											{
												OpcUa_MonitoredItemNotification* pMonitoredItemNotification = pUAMonitoredItemNotification->GetMonitoredItemNotification();
												if (bPublishingEnabled) // Le cas normal ne fonctionne que si la publication est activée... Testcase 014.js Subscription Services - subscription Basic
												{
													if (uiLocalMaxNotificationsPerPublish > preparedMonitoredItemNotificationList.size())
													{
														OpcUa_MonitoredItemNotification* pNewMonitoredItemNotification = (OpcUa_MonitoredItemNotification*)OpcUa_Alloc(sizeof(OpcUa_MonitoredItemNotification));
														OpcUa_MonitoredItemNotification_Initialize(pNewMonitoredItemNotification);
														// Transfert dans pNotification->MonitoredItems[]
														// ClientHandle
														pNewMonitoredItemNotification->ClientHandle = pMonitoredItemNotification->ClientHandle;
														// Value
														switch (pMonitoredItemNotification->Value.Value.Datatype)
														{
														case OpcUaType_ExtensionObject:
														{
															switch (pMonitoredItemNotification->Value.Value.ArrayType)
															{
															case OpcUa_VariantArrayType_Scalar:
															{
																if (pMonitoredItemNotification->Value.Value.Value.ExtensionObject)
																{
																	if (pMonitoredItemNotification->Value.Value.Value.ExtensionObject->Encoding == OpcUa_ExtensionObjectEncoding_EncodeableObject)
																	{
																		OpcUa_EncodeableType* pEncodeableType = pMonitoredItemNotification->Value.Value.Value.ExtensionObject->Body.EncodeableObject.Type;
																		//OpcUa_DataValue_Initialize(&(pNotification->MonitoredItems[iIndexMonitoredItem].Value));
																		if (pEncodeableType)
																		{
																			if (pNewMonitoredItemNotification->Value.Value.Value.ExtensionObject == OpcUa_Null)
																				pNewMonitoredItemNotification->Value.Value.Value.ExtensionObject = (OpcUa_ExtensionObject*)OpcUa_Alloc(sizeof(OpcUa_ExtensionObject));
																			else
																				OpcUa_ExtensionObject_Clear(pNewMonitoredItemNotification->Value.Value.Value.ExtensionObject);

																			OpcUa_ExtensionObject_Initialize(pNewMonitoredItemNotification->Value.Value.Value.ExtensionObject);

																			pNewMonitoredItemNotification->Value.Value.Datatype = OpcUaType_ExtensionObject;

																			OpcUa_ExtensionObject_CopyTo(pMonitoredItemNotification->Value.Value.Value.ExtensionObject, pNewMonitoredItemNotification->Value.Value.Value.ExtensionObject);
																			/*
																			// suppression TEMPORAIRE du code ci-dessous. Cette fonction de copie générique des extensionObject fuit...
																			// Commentaire novembre 2015
																			OpcUa_ExtensionObject* pExtensionObject = OpcUa_Null;
																			Copy(&pExtensionObject,
																			pMonitoredItemNotification->Value.Value.Value.ExtensionObject);
																			if (uStatus == OpcUa_Good)
																			{
																			if (pExtensionObject)
																			{
																			OpcUa_ExtensionObject_CopyTo(pExtensionObject, pNotification->MonitoredItems[iIndex].Value.Value.Value.ExtensionObject);
																			OpcUa_ExtensionObject_Clear(pExtensionObject);
																			OpcUa_Free(pExtensionObject);
																			}
																			}*/
																			//pNotification->MonitoredItems[iIndex].Value.Value.Value.ExtensionObject->Body.EncodeableObject.Type = pEncodeableType;
																			OpcUa_DateTime_CopyTo(&(pMonitoredItemNotification->Value.ServerTimestamp), &(pNewMonitoredItemNotification->Value.ServerTimestamp));
																			pNewMonitoredItemNotification->Value.ServerPicoseconds = pMonitoredItemNotification->Value.ServerPicoseconds;
																			OpcUa_DateTime_CopyTo(&(pMonitoredItemNotification->Value.SourceTimestamp), &(pNewMonitoredItemNotification->Value.SourceTimestamp));
																			pNewMonitoredItemNotification->Value.SourcePicoseconds = pMonitoredItemNotification->Value.SourcePicoseconds;
																			//iIndexMonitoredItem++;
																			preparedMonitoredItemNotificationList.push_back(pNewMonitoredItemNotification);
																		}
																	}
																}
															}
															break;
															case OpcUa_VariantArrayType_Array:
															{
																OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_INFO, "FillDataChangeNotificationMessage with an ExtensionObject of OpcUa_VariantArrayType_Array\n");
															}
															break;
															default:
																uStatus = OpcUa_BadNotSupported;
																break;
															}
														}
														break;
														default:
															OpcUa_DataValue_CopyTo(&(pMonitoredItemNotification->Value), &(pNewMonitoredItemNotification->Value));
															preparedMonitoredItemNotificationList.push_back(pNewMonitoredItemNotification);
															break;
														}
														// We mark the MonitoredItemNotification as sent
														pUAMonitoredItemNotification->MonitoredNotificationSent(OpcUa_True);

														if ((PublishTime.dwHighDateTime == 0) && (PublishTime.dwLowDateTime == 0))
														{
															pDataChangeNotification->SetPublishTime(OpcUa_DateTime_UtcNow()); // Set the publish Time
															PublishTime = pDataChangeNotification->GetPublishTime();
														}

													}
													else
													{

														if ((PublishTime.dwHighDateTime == 0) && (PublishTime.dwLowDateTime == 0))
															pDataChangeNotification->SetPublishTime(OpcUa_DateTime_UtcNow()); // Set the publish Time
														bMoreNotification = IsMoreToNotify(pAvailableSequenceNumbers, iteratorAvailableSequenceNumber, i);
														if (bMoreNotification)
															break;
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
				/////////////////////////////////////////////////////
				// Last step
				// Do we have something to notify to the client ?
				iIndexMonitoredItem = preparedMonitoredItemNotificationList.size();
				if (iIndexMonitoredItem > 0)
				{
					pNotification->MonitoredItems =
						(OpcUa_MonitoredItemNotification*)OpcUa_Alloc(iIndexMonitoredItem*sizeof(OpcUa_MonitoredItemNotification));
					if (pNotification->MonitoredItems)
					{
						for (OpcUa_UInt32 ii = 0; ii < iIndexMonitoredItem; ii++)
						{
							OpcUa_MonitoredItemNotification_Initialize(&(pNotification->MonitoredItems[ii]));
							OpcUa_MonitoredItemNotification* pMonitoredItemNotification = preparedMonitoredItemNotificationList.at(ii);
							pNotification->MonitoredItems[ii].ClientHandle = pMonitoredItemNotification->ClientHandle;
							OpcUa_DataValue_CopyTo(&(pMonitoredItemNotification->Value), &(pNotification->MonitoredItems[ii].Value));
							// Cleanup
							OpcUa_MonitoredItemNotification_Clear(pMonitoredItemNotification);
							OpcUa_Free(pMonitoredItemNotification);
						}

						preparedMonitoredItemNotificationList.clear();
						// NoOfMonitoredItems
						pNotification->NoOfMonitoredItems = iIndexMonitoredItem;
						// DiagnosticInfos
						pNotification->NoOfDiagnosticInfos = 0;
						pNotification->DiagnosticInfos = OpcUa_Null;
						m_pInternalPublishResponse->NotificationMessage.PublishTime = PublishTime;
						////////////////////////////////////////////////////////////////////////////////////////////
						// Mise en forme du résultat
						iIndexNotification = m_pInternalPublishResponse->NotificationMessage.NoOfNotificationData;
						OpcUa_ExtensionObject_Initialize(&(m_pInternalPublishResponse->NotificationMessage.NotificationData[iIndexNotification]));
						//NodeId
						m_pInternalPublishResponse->NotificationMessage.NotificationData[iIndexNotification].TypeId.NodeId.Identifier.Numeric = OpcUaId_DataChangeNotification_Encoding_DefaultBinary;
						m_pInternalPublishResponse->NotificationMessage.NotificationData[iIndexNotification].TypeId.NodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
						// Encoding
						m_pInternalPublishResponse->NotificationMessage.NotificationData[iIndexNotification].Encoding = OpcUa_ExtensionObjectEncoding_EncodeableObject;
						m_pInternalPublishResponse->NotificationMessage.NotificationData[iIndexNotification].Body.EncodeableObject.Type = &OpcUa_DataChangeNotification_EncodeableType;
						// Embedded object
						m_pInternalPublishResponse->NotificationMessage.NotificationData[iIndexNotification].Body.EncodeableObject.Object = pNotification;
						m_pInternalPublishResponse->NotificationMessage.NoOfNotificationData++; // 
					}
					else
					{
						if (pNotification)
						{
							OpcUa_Free(pNotification);
							pNotification = OpcUa_Null;
						}
						uStatus = OpcUa_BadOutOfMemory;
					}
				}
				else
				{
					if (pNotification)
					{
						OpcUa_Free(pNotification);
						pNotification = OpcUa_Null;
					}
					uStatus = OpcUa_BadNothingToDo;
				}
				if ((*pbKeepAlive) == OpcUa_True)
				{
					////////////////////////////////////////////////////////////////////////////////////////////
					OpcUa_Free(m_pInternalPublishResponse->NotificationMessage.NotificationData);
					if (pNotification)
					{
						OpcUa_Free(pNotification);
						pNotification = OpcUa_Null;
					}
					m_pInternalPublishResponse->NotificationMessage.NotificationData = OpcUa_Null;
					m_pInternalPublishResponse->NotificationMessage.NoOfNotificationData = 0;
					m_pInternalPublishResponse->NotificationMessage.PublishTime = OpcUa_DateTime_UtcNow();
					uStatus = OpcUa_Good;
				}
				m_pInternalPublishResponse->MoreNotifications = bMoreNotification;
			}
			else
				uStatus = OpcUa_BadOutOfMemory;
		}
	}
	uiQueueSizes.clear();
	return uStatus;
}



///-------------------------------------------------------------------------------------------------
/// <summary>	Check if there is more notification to send for inCUADataChangeNotificationList starting from
///					indexUAMonitoredItemNotificationList for the current UAMonitoredItemNotificationList and from
///					indexDataChangeNotification for the next one.
/// <remarks>	Michel, 25/01/2016. </remarks>
///
/// <param name="pDataChangeNotification">			   	[in,out] If non-null, the data change
/// 													notification.
/// </param>
/// <param name="indexDataChangeNotification">		   	The index data change notification.
/// </param>
/// <param name="indexUAMonitoredItemNotificationList">	List of index UA monitored item
/// 													notifications.
/// </param>
///
/// <returns>	An OpcUa_Boolean. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_Boolean CQueuedPublishMessage::IsMoreToNotify(CAvailableSequenceNumbers* pAvailableSequenceNumbers, CAvailableSequenceNumbers::iterator iteratorAvailableSequenceNumber, OpcUa_UInt32 indexUAMonitoredItemNotificationList)
{
	OpcUa_Boolean bResult=OpcUa_False;
	CUADataChangeNotification* pUADataChangeNotification = iteratorAvailableSequenceNumber->second;
	if (!pUADataChangeNotification->IsKeepAlive())
	{
		CUAMonitoredItemNotificationList aUAMonitoredItemNotificationList = pUADataChangeNotification->GetUAMonitoredItemNotificationList();
		for (OpcUa_UInt32 ii = indexUAMonitoredItemNotificationList; ii < aUAMonitoredItemNotificationList.size(); ii++)
		{
			CUAMonitoredItemNotification* pUAMonitoredItemNotification = aUAMonitoredItemNotificationList.at(ii);
			if (!pUAMonitoredItemNotification->IsMonitoredNotificationSent())
			{
				bResult = OpcUa_True;
				break;
			}
		}
	}
	if (!bResult)
	{
		iteratorAvailableSequenceNumber++;
		// Deals with the next one
		for (iteratorAvailableSequenceNumber = pAvailableSequenceNumbers->begin(); iteratorAvailableSequenceNumber != pAvailableSequenceNumbers->end(); iteratorAvailableSequenceNumber++)
		{
			CUADataChangeNotification* pUADataChangeNotification = iteratorAvailableSequenceNumber->second;
			if (!pUADataChangeNotification->IsKeepAlive())
			{
				if (!bResult)
				{
					CUAMonitoredItemNotificationList aUAMonitoredItemNotificationList = pUADataChangeNotification->GetUAMonitoredItemNotificationList();
					for (OpcUa_UInt32 ii = 0; ii < aUAMonitoredItemNotificationList.size(); ii++)
					{
						CUAMonitoredItemNotification* pUAMonitoredItemNotification = aUAMonitoredItemNotificationList.at(ii);
						if (!pUAMonitoredItemNotification->IsMonitoredNotificationSent())
						{
							bResult = OpcUa_True;
							break;
						}
					}
				}
			}
		}
	}
	return bResult;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Fill status change notification message. </summary>
///
/// <remarks>	Michel, 05/02/2016. </remarks>
///
/// <param name="pNotification">	[in,out] If non-null, the notification. </param>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode CQueuedPublishMessage::FillStatusChangeNotificationMessage(OpcUa_StatusChangeNotification* pNotification)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	if (pNotification)
	{
		if (m_pInternalPublishResponse->NotificationMessage.NotificationData==OpcUa_Null)
			m_pInternalPublishResponse->NotificationMessage.NotificationData=
				(OpcUa_ExtensionObject*)OpcUa_Alloc(sizeof(OpcUa_ExtensionObject));
		// Mise en forme du résultat
		OpcUa_Int32 iIndex= m_pInternalPublishResponse->NotificationMessage.NoOfNotificationData;
		OpcUa_ExtensionObject_Initialize(&(m_pInternalPublishResponse->NotificationMessage.NotificationData[iIndex]));
		//m_pInternalPublishResponse->NotificationMessage.NotificationData[iIndex].Body.EncodeableObject.Object=(OpcUa_StatusChangeNotification*)OpcUa_Alloc(sizeof(OpcUa_StatusChangeNotification));
		//NodeId
		m_pInternalPublishResponse->NotificationMessage.NotificationData[iIndex].TypeId.NodeId.Identifier.Numeric=OpcUaId_StatusChangeNotification_Encoding_DefaultBinary;
		m_pInternalPublishResponse->NotificationMessage.NotificationData[iIndex].TypeId.NodeId.IdentifierType=OpcUa_IdentifierType_Numeric;
		m_pInternalPublishResponse->NotificationMessage.NotificationData[iIndex].TypeId.NodeId.NamespaceIndex=0;
		// Encoding
		m_pInternalPublishResponse->NotificationMessage.NotificationData[iIndex].Encoding = OpcUa_ExtensionObjectEncoding_EncodeableObject;
		m_pInternalPublishResponse->NotificationMessage.NotificationData[iIndex].Body.EncodeableObject.Type=&OpcUa_StatusChangeNotification_EncodeableType;
		// Embedded object
		m_pInternalPublishResponse->NotificationMessage.NotificationData[iIndex].Body.EncodeableObject.Object=pNotification;
		m_pInternalPublishResponse->NotificationMessage.NoOfNotificationData++;
	}
	else
		uStatus=OpcUa_BadInvalidArgument;
	return uStatus;
}
// Send all StatusChangeNotification in the CUAStatusChangeNotificationList to the client
OpcUa_StatusCode CQueuedPublishMessage::FillStatusChangeNotificationMessage(OpcUa_UInt32 uiSubscriptionId,
																OpcUa_Boolean bPublishingEnabled,
																OpcUa_UInt32 uiPos, // position courante dans le pDataChangeNotificationList
																CUAStatusChangeNotificationList* pStatusChangeNotificationList,
																OpcUa_UInt32 uiMaxNotificationsPerPublish,
																OpcUa_Boolean* pbKeepAlive)
{
	(void)uiMaxNotificationsPerPublish;
	OpcUa_StatusCode uStatus=OpcUa_Good;
	(void)bPublishingEnabled;
	if (pStatusChangeNotificationList->size() > 0) // 1.0.4.0 test
	{
		if (m_pInternalPublishResponse)
		{
			m_pInternalPublishResponse->SubscriptionId = uiSubscriptionId;
			m_pInternalPublishResponse->NotificationMessage.PublishTime = OpcUa_DateTime_UtcNow();
			m_pInternalPublishResponse->ResponseHeader.RequestHandle = m_pInternalPublishRequest->RequestHeader.RequestHandle;

			// Allocation de l'extensionObject qui transportera le StatusChangeNotification. 
			if (m_pInternalPublishResponse->NotificationMessage.NotificationData == OpcUa_Null)
				m_pInternalPublishResponse->NotificationMessage.NotificationData =
				(OpcUa_ExtensionObject*)OpcUa_Alloc(sizeof(OpcUa_ExtensionObject));
			else
			{

				OpcUa_ExtensionObject* pNewExtensionObject = OpcUa_Null;
				OpcUa_UInt32 iNewSize = m_pInternalPublishResponse->NotificationMessage.NoOfNotificationData + 1;
				pNewExtensionObject = (OpcUa_ExtensionObject*)OpcUa_ReAlloc(m_pInternalPublishResponse->NotificationMessage.NotificationData, sizeof(OpcUa_ExtensionObject)*iNewSize);
				if (pNewExtensionObject)
					m_pInternalPublishResponse->NotificationMessage.NotificationData = pNewExtensionObject;
			}
			//
			// préparation d'un OpcUa_StatusChangeNotification. Ils seront placés dans un NotificationData (OpcUa_ExtensionObject)
			OpcUa_StatusChangeNotification* pNotification =
				(OpcUa_StatusChangeNotification*)OpcUa_Alloc(sizeof(OpcUa_StatusChangeNotification));
			OpcUa_UInt32 ii = 0;
			for (ii = uiPos; ii < pStatusChangeNotificationList->size(); ii++)
			{
				CUAStatusChangeNotification* pStatusChangeNotification = pStatusChangeNotificationList->at(ii);
				if (!pStatusChangeNotification->IsKeepAlive() && (!pStatusChangeNotification->IsAcked())) // s'il ne s'agit pas d'un KeepAlive
					(*pbKeepAlive) = OpcUa_False;
				if (!pStatusChangeNotification->IsAcked())
				{
					pNotification->DiagnosticInfo = pStatusChangeNotification->GetInternalStatusChangeNotification()->DiagnosticInfo;
					pNotification->Status = pStatusChangeNotification->GetInternalStatusChangeNotification()->Status;
				}
			}
			if (!(*pbKeepAlive))
			{
				if (pStatusChangeNotificationList->size() > 0)
				{
					////////////////////////////////////////////////////////////////////////////////////////////
					// Mise en forme du résultat
					OpcUa_Int32 iIndex = m_pInternalPublishResponse->NotificationMessage.NoOfNotificationData;
					OpcUa_ExtensionObject_Initialize(&(m_pInternalPublishResponse->NotificationMessage.NotificationData[iIndex]));
					m_pInternalPublishResponse->NotificationMessage.NotificationData[iIndex].Body.EncodeableObject.Object = (OpcUa_StatusChangeNotification*)OpcUa_Alloc(sizeof(OpcUa_StatusChangeNotification));
					//NodeId
					m_pInternalPublishResponse->NotificationMessage.NotificationData[iIndex].TypeId.NodeId.Identifier.Numeric = OpcUaId_StatusChangeNotification_Encoding_DefaultBinary;
					m_pInternalPublishResponse->NotificationMessage.NotificationData[iIndex].TypeId.NodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
					m_pInternalPublishResponse->NotificationMessage.NotificationData[iIndex].TypeId.NodeId.NamespaceIndex = 0;
					// Encoding
					m_pInternalPublishResponse->NotificationMessage.NotificationData[iIndex].Encoding = OpcUa_ExtensionObjectEncoding_EncodeableObject;
					m_pInternalPublishResponse->NotificationMessage.NotificationData[iIndex].Body.EncodeableObject.Type = &OpcUa_StatusChangeNotification_EncodeableType;
					// Embedded object
					m_pInternalPublishResponse->NotificationMessage.NotificationData[iIndex].Body.EncodeableObject.Object = pNotification;
					m_pInternalPublishResponse->NotificationMessage.NoOfNotificationData++;
				}
				else
					OpcUa_Free(pNotification);
			}
			//else
			//{
			//	////////////////////////////////////////////////////////////////////////////////////////////
			//	OpcUa_Free(m_pInternalPublishResponse->NotificationMessage.NotificationData);
			//	OpcUa_Free(pNotification);
			//	m_pInternalPublishResponse->NotificationMessage.NotificationData=OpcUa_Null;
			//	m_pInternalPublishResponse->NotificationMessage.NoOfNotificationData=0;
			//}
		}
	}
	return uStatus;
}
void CQueuedPublishMessage::AddSequenceNumber(OpcUa_UInt32 uiVal) 
{
	// Avant d'ajouter on va verifier que ce n° de sequence n'ai pas déja dedans
	if (m_pUInt32s)
	{
		CUInt32Collection::iterator it;
		for (it=m_pUInt32s->begin();it<m_pUInt32s->end();it++)
		{
			OpcUa_UInt32 uiVal1=*it;
			if (uiVal1==uiVal)
			 return;
		}
		m_pUInt32s->push_back(uiVal);
	}
}
OpcUa_StatusCode CQueuedPublishMessage::ResetPublishResponse()
{

	OpcUa_StatusCode uStatus = OpcUa_BadInvalidArgument;
	// delete de response
	if (m_pResponseType)
	{
		OpcUa_EncodeableObject_Delete(m_pResponseType, (OpcUa_Void**)&m_pInternalPublishResponse);
		m_pInternalPublishResponse = OpcUa_Null;
		uStatus = OpcUa_Good;
	}
	else
		uStatus = OpcUa_BadInternalError;
	return uStatus;
}
OpcUa_StatusCode CQueuedPublishMessage::EncodeableObjectDelete()
{
	OpcUa_StatusCode uStatus = OpcUa_BadInvalidArgument;
	// delete the request
	OpcUa_EncodeableType* pRequestType = m_pRequestType;
	if (pRequestType)
	{
		OpcUa_EncodeableObject_Delete(pRequestType, (OpcUa_Void**)&m_pInternalPublishRequest);
		m_pInternalPublishRequest = OpcUa_Null;
		uStatus = OpcUa_Good;
	}
	// delete de response
	if (m_pResponseType)
	{
		OpcUa_EncodeableObject_Delete(m_pResponseType, (OpcUa_Void**)&m_pInternalPublishResponse);
		m_pInternalPublishResponse = OpcUa_Null;
		uStatus = OpcUa_Good;
	}
	else
		uStatus = OpcUa_BadInternalError;
	m_bEncodeableObjectDeleted = OpcUa_True;
	return uStatus;
}

OpcUa_Boolean CQueuedPublishMessage::IsTimeouted()
{
	OpcUa_Boolean bResult = OpcUa_False;
	OpcUa_DateTime utcNow = OpcUa_DateTime_UtcNow();
	if (m_pInternalPublishRequest)
	{
		OpcUa_UInt64 HeaderTimestamp = m_pInternalPublishRequest->RequestHeader.Timestamp.dwHighDateTime;
		HeaderTimestamp <<= 32;
		HeaderTimestamp += m_pInternalPublishRequest->RequestHeader.Timestamp.dwLowDateTime;
		OpcUa_UInt64 utcNow64 = utcNow.dwHighDateTime;
		utcNow64 <<= 32;
		utcNow64 += utcNow.dwLowDateTime;

		// Let's add a 7500 msec tolerance in the future for the serveur time
		utcNow64 += OPC_TIMEOUT * 10000;
		OpcUa_UInt64 ullTimeout = utcNow64 - HeaderTimestamp;

		if (m_pInternalPublishRequest->RequestHeader.TimeoutHint * 10000 < ullTimeout)
		{
			SetDeleted(OpcUa_True);
			bResult = OpcUa_True;
		}
	}
	else
		bResult = OpcUa_True;
	return bResult;
}

void CQueuedPublishMessage::SetPublishResponseSubscriptionId(OpcUa_UInt32 uiSubscriptionId)
{
	m_pInternalPublishResponse->SubscriptionId = uiSubscriptionId;
}