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
/* New file from RCL project*/
/* core */
#include <opcua.h>

#include <opcua_https_secureconnection.h>

//OpcUa_StatusCode OpcUa_HttpsSecureConnection_Create(OpcUa_Connection*			pTransportConnect, 
//													OpcUa_Encoder*				pEncoder,
//													OpcUa_Decoder*				pDecoder,
//													OpcUa_StringTable*			aNamespaceUris,
//													OpcUa_EncodeableTypeTable*	aEncodeableTypeTable,
//													OpcUa_Connection**			ppSecureConnection)
//
//{
//	OpcUa_StatusCode uStatus;
//	OpcUa_Connection*       pConnection;
//    OpcUa_SecureConnection* pSecureConnection ;
//
//    OpcUa_ReturnErrorIfArgumentNull(pTransportConnect);
//    OpcUa_ReturnErrorIfArgumentNull(ppSecureConnection);
//    OpcUa_ReturnErrorIfArgumentNull(pEncoder);
//    OpcUa_ReturnErrorIfArgumentNull(pDecoder);
//    OpcUa_ReturnErrorIfArgumentNull(aNamespaceUris);
//    OpcUa_ReturnErrorIfArgumentNull(aEncodeableTypeTable);
//
//    /* allocate connection object */
//    pConnection = (OpcUa_Connection*)OpcUa_Alloc(sizeof(OpcUa_Connection));
//    if (!pConnection)
//		uStatus=OpcUa_BadOutOfMemory;
//	else
//	{
//		OpcUa_MemSet(pConnection, 0, sizeof(OpcUa_Connection));
//
//		/* allocate connection handle */
//		pSecureConnection = (OpcUa_SecureConnection*)OpcUa_Alloc(sizeof(OpcUa_SecureConnection));
//		if (!pSecureConnection)
//			uStatus=OpcUa_BadOutOfMemory;
//		else
//		{
//			OpcUa_MemSet(pSecureConnection, 0, sizeof(OpcUa_SecureConnection));
//
//			/* initialize connection handle */
//			pSecureConnection->SanityCheck          = OpcUa_SecureConnection_SanityCheck;
//			pSecureConnection->TransportConnection  = pTransportConnect;
//			pSecureConnection->State                = OpcUa_SecureConnectionState_Disconnected;
//			pSecureConnection->Encoder              = pEncoder;
//			pSecureConnection->Decoder              = pDecoder;
//			pSecureConnection->NamespaceUris        = aNamespaceUris;
//			pSecureConnection->KnownTypes           = aEncodeableTypeTable;
//			pSecureConnection->uRequestId           = 1; /* we start with 1 since zero is error prone */
//
//			/* initialize connection object */
//			pConnection->Handle           = pSecureConnection;
//			pConnection->Connect          = OpcUa_SecureConnection_Connect;
//			pConnection->Disconnect       = OpcUa_SecureConnection_Disconnect;
//			pConnection->BeginSendRequest = OpcUa_SecureConnection_BeginSendRequest;
//			pConnection->EndSendRequest   = OpcUa_SecureConnection_EndSendRequest;
//			pConnection->AbortSendRequest = OpcUa_SecureConnection_AbortSendRequest;
//			pConnection->Delete           = OpcUa_SecureConnection_Delete;
//
//			/* create mutex */
//			uStatus = OpcUa_Mutex_Create(&pSecureConnection->RequestMutex);
//			if (uStatus==OpcUa_Good)
//			{
//				uStatus = OpcUa_Mutex_Create(&pSecureConnection->ResponseMutex);
//				if (uStatus==OpcUa_Good)
//				{
//					*ppSecureConnection = pConnection;
//
//					/* create list for pending requests */
//					uStatus = OpcUa_List_Create(&(pSecureConnection->PendingRequests));
//					if (uStatus==OpcUa_Good)
//					{
//						/* create watchdog timer for outstanding responses. */
//						uStatus = OpcUa_Timer_Create(   &(pSecureConnection->hWatchdogTimer),
//														OPCUA_SECURECONNECTION_TIMEOUTINTERVAL,
//														OpcUa_SecureConnection_WatchdogTimerCallback,
//														OpcUa_SecureConnection_WatchdogTimerKillCallback,
//														(OpcUa_Void*)(*ppSecureConnection));
//					}		
//				}
//			}
//		}
//	}
//	return uStatus;
//}
