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
#include "QueueRequest.h"
using namespace OpenOpcUa;
using namespace UACoreServer;
CQueueRequest::CQueueRequest()
{
	m_pResponseType = OpcUa_Null;
	m_pRequestType=OpcUa_Null;
	m_hContext=OpcUa_Null;
	m_hEndpoint=OpcUa_Null;
}
CQueueRequest::CQueueRequest(OpcUa_Endpoint hEndpoint,OpcUa_Handle hContext,OpcUa_EncodeableType* pRequestType): 
		m_hEndpoint(hEndpoint),m_hContext(hContext),m_pRequestType(pRequestType)
{
	m_bDeleted=OpcUa_False;
	m_pResponseType = OpcUa_Null;
}
CQueueRequest::~CQueueRequest()
{
	m_hContext=OpcUa_Null;
	m_hEndpoint=OpcUa_Null;
	m_pRequestType = OpcUa_Null;
	m_pResponseType = OpcUa_Null;
}
//void CQueueRequest::SendErrorResponse(OpcUa_StatusCode uStatus)
//{
//	OpcUa_Endpoint_SendErrorResponse(m_hEndpoint, m_hContext, uStatus);
//}
OpcUa_Endpoint CQueueRequest::GetEndpoint() 
{ 
	return m_hEndpoint; 
}
OpcUa_Handle CQueueRequest::GetContext() 
{ 
	return m_hContext; 
}
OpcUa_EncodeableType* CQueueRequest::GetRequestType() 
{ 
	return m_pRequestType; 
}
OpcUa_Boolean	CQueueRequest::IsDeleted() 
{ 
	return m_bDeleted; 
}
void CQueueRequest::SetDeleted(OpcUa_Boolean bVal) 
{ 
	m_bDeleted = bVal; 
}
