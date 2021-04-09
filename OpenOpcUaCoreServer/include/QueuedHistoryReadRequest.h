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
//#include "QueueRequest.h"
namespace OpenOpcUa
{
	namespace UACoreServer 
	{
		class CQueuedHistoryReadMessage :
			public CQueueRequest 
		{
		public: 
			CQueuedHistoryReadMessage();
			CQueuedHistoryReadMessage(OpcUa_HistoryReadRequest* pReadRequest, OpcUa_Endpoint hEndpoint, OpcUa_Handle hContext, OpcUa_EncodeableType* pRequestType);
			~CQueuedHistoryReadMessage();
			OpcUa_HistoryReadRequest* GetInternalReadRequest() {return m_pInternalReadRequest;}
			OpcUa_HistoryReadResponse* GetInternalReadResponse() {return m_pInternalReadResponse;}
			//OpcUa_TimestampsToReturn_Source  = 0,
			//OpcUa_TimestampsToReturn_Server  = 1,
			//OpcUa_TimestampsToReturn_Both    = 2,
			//OpcUa_TimestampsToReturn_Neither = 3
			OpcUa_Boolean IsSourceTimestampRequested() 
			{
				if( (GetInternalReadRequest()->TimestampsToReturn==OpcUa_TimestampsToReturn_Source) 
					|| (GetInternalReadRequest()->TimestampsToReturn==OpcUa_TimestampsToReturn_Both))
					return OpcUa_True;
				else
					return OpcUa_False;
			}
			OpcUa_Boolean IsServerTimestampRequested() 
			{
				if( (GetInternalReadRequest()->TimestampsToReturn==OpcUa_TimestampsToReturn_Server) 
					|| (GetInternalReadRequest()->TimestampsToReturn==OpcUa_TimestampsToReturn_Both))
					return OpcUa_True;
				else
					return OpcUa_False;
			}
			OpcUa_UInt32	GetRequestHandle() {return GetInternalReadRequest()->RequestHeader.RequestHandle;}
			OpcUa_Int32 GetNoOfNodesToRead() {return GetInternalReadRequest()->NoOfNodesToRead;}
			OpcUa_HistoryReadValueId* GetReadValueId(OpcUa_Int32 ii) 
			{
				return &(GetInternalReadRequest()->NodesToRead[ii]);
			}
			//OpcUa_Double	GetMaxAge() {return GetInternalReadRequest()->MaxAge;} // get the max age requested by the client for the read request
			OpcUa_TimestampsToReturn GetTimestampsToReturn();
			OpcUa_StatusCode FillResponseHeader();
			OpcUa_StatusCode CancelSendResponse();
			OpcUa_StatusCode BeginSendResponse();
			OpcUa_StatusCode EndSendResponse();
			OpcUa_StatusCode EncodeableObjectDelete();
		private:
			OpcUa_HistoryReadRequest*	m_pInternalReadRequest;// Requete transmise par le client au serveur
			OpcUa_HistoryReadResponse*	m_pInternalReadResponse;// Reponse transmise par le serveur au client
			OpcUa_Boolean				m_bEncodeableObjectDeleted;
		};
		typedef std::vector<CQueuedHistoryReadMessage*> CQueuedHistoryReadMessages;
	}
}