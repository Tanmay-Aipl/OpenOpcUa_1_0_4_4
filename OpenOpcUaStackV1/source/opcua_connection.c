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

#include <opcua.h>
#include <opcua_mutex.h>
#include <opcua_semaphore.h>
#include <opcua_connection.h>
#include <opcua_statuscodes.h>

/*============================================================================
 * OpcUa_Connection_Connect
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Connection_Connect(
    OpcUa_Connection*               a_pConnection,
    OpcUa_String*                   a_sUrl,
    OpcUa_ClientCredential*         a_pCredentials,
    OpcUa_UInt32                    a_uTimeout,
    OpcUa_Connection_PfnOnNotify*   a_pCallback,
    OpcUa_Void*                     a_pCallbackData)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_Connection);
    OpcUa_ReturnErrorIfArgumentNull(a_pConnection);
    OpcUa_ReturnErrorIfArgumentNull(a_pConnection->Connect);

    return a_pConnection->Connect(a_pConnection, a_sUrl, a_pCredentials, a_uTimeout, a_pCallback, a_pCallbackData);
}

/*============================================================================
 * OpcUa_Connection_Disconnect
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Connection_Disconnect(
    struct _OpcUa_Connection* a_pConnection,
    OpcUa_Boolean             a_bNotifyOnComplete)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_Connection);
    OpcUa_ReturnErrorIfArgumentNull(a_pConnection);
    OpcUa_ReturnErrorIfArgumentNull(a_pConnection->Disconnect);

    return a_pConnection->Disconnect(a_pConnection, a_bNotifyOnComplete);
}

/*============================================================================
 * OpcUa_Connection_BeginSendRequest
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Connection_BeginSendRequest(
    OpcUa_Connection*    a_pConnection,
    OpcUa_OutputStream** a_ppOstrm)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_Connection);
    OpcUa_ReturnErrorIfArgumentNull(a_pConnection);
    OpcUa_ReturnErrorIfArgumentNull(a_pConnection->BeginSendRequest);

    return a_pConnection->BeginSendRequest(a_pConnection, a_ppOstrm);
}

/*============================================================================
 * OpcUa_Connection_EndSendRequest
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Connection_EndSendRequest(
    struct _OpcUa_Connection*       a_pConnection,
    OpcUa_OutputStream**            a_ppOstrm,
    OpcUa_UInt32                    a_uMsecTimeout,
    OpcUa_Connection_PfnOnResponse* a_pCallback,
    OpcUa_Void*                     a_pCallbackData)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_Connection);
    OpcUa_ReturnErrorIfArgumentNull(a_pConnection);
    OpcUa_ReturnErrorIfArgumentNull(a_pConnection->EndSendRequest);

    return a_pConnection->EndSendRequest(a_pConnection, a_ppOstrm, a_uMsecTimeout, a_pCallback, a_pCallbackData);
}

/*============================================================================
 * OpcUa_Connection_AbortSendRequest
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Connection_AbortSendRequest(
    struct _OpcUa_Connection*   a_pConnection,
    OpcUa_StatusCode            a_uStatus,
    OpcUa_String*               a_psReason,
    OpcUa_OutputStream**        a_ppOstrm)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_Connection);
    OpcUa_ReturnErrorIfArgumentNull(a_pConnection);
    OpcUa_ReturnErrorIfArgumentNull(a_pConnection->AbortSendRequest);

    return a_pConnection->AbortSendRequest(a_pConnection, a_uStatus, a_psReason, a_ppOstrm);
}

/*============================================================================
 * OpcUa_Connection_AbortSendRequest
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Connection_GetReceiveBufferSize(
    struct _OpcUa_Connection* a_pConnection,
    OpcUa_UInt32*             a_pBufferSize)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_Connection);
    OpcUa_ReturnErrorIfArgumentNull(a_pConnection);
    OpcUa_ReturnErrorIfArgumentNull(a_pConnection->GetReceiveBufferSize);

    return a_pConnection->GetReceiveBufferSize(a_pConnection, a_pBufferSize);
}

/*============================================================================
 * OpcUa_Connection_Delete
 *===========================================================================*/
OpcUa_Void OpcUa_Connection_Delete(OpcUa_Connection** a_ppConnection)
{
    if (a_ppConnection != OpcUa_Null && *a_ppConnection != OpcUa_Null)
    {
        (*a_ppConnection)->Delete(a_ppConnection);
    }
}
/*============================================================================
 * OpcUa_Connection_AddToSendQueue
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Connection_AddToSendQueue(
    OpcUa_Connection*         a_pConnection,
    OpcUa_BufferListElement*  a_pBufferList,
    OpcUa_UInt32              a_uFlags)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_Connection);
    OpcUa_ReturnErrorIfArgumentNull(a_pConnection);
    OpcUa_ReturnErrorIfArgumentNull(a_pConnection->AddToSendQueue);

    return a_pConnection->AddToSendQueue(a_pConnection, a_pBufferList, a_uFlags);
}