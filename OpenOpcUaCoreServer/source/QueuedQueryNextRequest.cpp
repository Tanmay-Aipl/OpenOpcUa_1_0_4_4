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
//#include "QueueRequest.h"
#include "QueuedQueryNextRequest.h"
using namespace OpenOpcUa;
using namespace UACoreServer;
using namespace UASharedLib;
CQueuedQueryNextMessage::CQueuedQueryNextMessage()
{
	m_pInternalQueryNextRequest=OpcUa_Null;
	m_pInternalQueryNextResponse=OpcUa_Null;
	m_bEncodeableObjectDeleted=OpcUa_False;
}
CQueuedQueryNextMessage::CQueuedQueryNextMessage(OpcUa_QueryNextRequest* pReadRequest, OpcUa_Endpoint hEndpoint, OpcUa_Handle hContext, OpcUa_EncodeableType* pRequestType)
			:CQueueRequest(hEndpoint,hContext,pRequestType)
{
	m_bEncodeableObjectDeleted=OpcUa_False;
	m_pResponseType=pRequestType;
	m_pInternalQueryNextRequest=pReadRequest;
	m_pInternalQueryNextResponse = OpcUa_Null;
}
CQueuedQueryNextMessage::~CQueuedQueryNextMessage()
{
	if (!m_bEncodeableObjectDeleted)
		OpcUa_EncodeableObject_Delete(GetRequestType(), (OpcUa_Void**)&m_pInternalQueryNextRequest);;
}
OpcUa_StatusCode CQueuedQueryNextMessage::FillResponseHeader()
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	m_pInternalQueryNextResponse->ResponseHeader.Timestamp = OpcUa_DateTime_UtcNow();
	m_pInternalQueryNextResponse->ResponseHeader.RequestHandle = m_pInternalQueryNextRequest->RequestHeader.RequestHandle;
	m_pInternalQueryNextResponse->ResponseHeader.ServiceResult = OpcUa_Good;
	return uStatus;
}
OpcUa_StatusCode CQueuedQueryNextMessage::CancelSendResponse()
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_Handle pContextHandle=GetContext();
	uStatus=OpcUa_Endpoint_CancelSendResponse( 
		GetEndpoint(),
		uStatus,
		OpcUa_Null,
		&pContextHandle);
	return uStatus;
}

OpcUa_StatusCode CQueuedQueryNextMessage::BeginSendResponse()
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	m_pResponseType = OpcUa_Null;
	// Create a context to use for sending a response.
	uStatus = OpcUa_Endpoint_BeginSendResponse(GetEndpoint(), GetContext(), (OpcUa_Void**)&m_pInternalQueryNextResponse, &m_pResponseType);
	if (uStatus==OpcUa_Good)
	{
		// DiagnosticInfo
		if (m_pInternalQueryNextRequest->RequestHeader.ReturnDiagnostics)
		{
			//
			OpcUa_String_AttachCopy(&(m_pInternalQueryNextResponse->ResponseHeader.ServiceDiagnostics.AdditionalInfo), "OpenOpcUa");
			m_pInternalQueryNextResponse->ResponseHeader.ServiceDiagnostics.InnerStatusCode = OpcUa_Good;
			m_pInternalQueryNextResponse->ResponseHeader.ServiceDiagnostics.Locale = 1;
			m_pInternalQueryNextResponse->ResponseHeader.ServiceDiagnostics.LocalizedText = 1;
			m_pInternalQueryNextResponse->ResponseHeader.ServiceDiagnostics.NamespaceUri = 1;
			m_pInternalQueryNextResponse->ResponseHeader.ServiceDiagnostics.SymbolicId = 1;
			
		}
	}
	return uStatus;
}
OpcUa_StatusCode CQueuedQueryNextMessage::EndSendResponse()
{
	OpcUa_StatusCode uStatus=OpcUa_BadInvalidArgument;
	if (m_pResponseType)
	{
		OpcUa_Handle pContextHandle=GetContext();
		uStatus = OpcUa_Endpoint_EndSendResponse(   
			GetEndpoint(),
			&pContextHandle,
			OpcUa_Good,
			m_pInternalQueryNextResponse,
			m_pResponseType);
	}
	return uStatus;
}
OpcUa_StatusCode CQueuedQueryNextMessage::EncodeableObjectDelete()
{
	OpcUa_StatusCode uStatus=OpcUa_BadInvalidArgument;
	OpcUa_EncodeableObject_Delete(m_pResponseType, (OpcUa_Void**)&m_pInternalQueryNextResponse);
	m_bEncodeableObjectDeleted=OpcUa_True;
	return uStatus;
}