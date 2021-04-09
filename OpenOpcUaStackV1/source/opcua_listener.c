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
#include <opcua_listener.h>

/*============================================================================
 * OpcUa_Listener_Open
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Listener_Open(
    struct _OpcUa_Listener*     a_pListener,
    OpcUa_String*               a_sUrl,
    OpcUa_Listener_PfnOnNotify* a_pCallback,
    OpcUa_Void*                 a_pCallbackData)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_Listener);
    OpcUa_ReturnErrorIfArgumentNull(a_pListener);
    OpcUa_ReturnErrorIfArgumentNull(a_pListener->Open);
    OpcUa_ReturnErrorIfArgumentNull(a_pCallback);

    return a_pListener->Open(a_pListener, a_sUrl, a_pCallback, a_pCallbackData);
}

/*============================================================================
 * OpcUa_Listener_Close
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Listener_Close(OpcUa_Listener* a_pListener)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_Listener);
    OpcUa_ReturnErrorIfArgumentNull(a_pListener);
    OpcUa_ReturnErrorIfArgumentNull(a_pListener->Open);

    return a_pListener->Close(a_pListener);
}

/*============================================================================
 * OpcUa_Listener_BeginSendResponse
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Listener_BeginSendResponse(
    OpcUa_Listener*      a_pListener,
    OpcUa_Handle         a_hConnection,
    OpcUa_InputStream**  a_ppIstrm,
    OpcUa_OutputStream** a_ppOstrm)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_Listener);
    OpcUa_ReturnErrorIfArgumentNull(a_pListener);
    OpcUa_ReturnErrorIfArgumentNull(a_pListener->BeginSendResponse);

    return a_pListener->BeginSendResponse(a_pListener, a_hConnection, a_ppIstrm, a_ppOstrm);
}

/*============================================================================
 * OpcUa_Listener_EndSendResponse
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Listener_EndSendResponse(
    struct _OpcUa_Listener* a_pListener,
    OpcUa_StatusCode        a_uStatus,
    OpcUa_OutputStream**    a_ppOstrm)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_Listener);
    OpcUa_ReturnErrorIfArgumentNull(a_pListener);
    OpcUa_ReturnErrorIfArgumentNull(a_pListener->EndSendResponse);

    return a_pListener->EndSendResponse(a_pListener, a_uStatus, a_ppOstrm);
}

/*============================================================================
 * OpcUa_Listener_AbortSendResponse
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Listener_AbortSendResponse(
    struct _OpcUa_Listener* a_pListener,
    OpcUa_StatusCode        a_uStatus,
    OpcUa_String*           a_psReason,
    OpcUa_OutputStream**    a_ppOstrm)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_Listener);
    OpcUa_ReturnErrorIfArgumentNull(a_pListener);
    OpcUa_ReturnErrorIfArgumentNull(a_pListener->AbortSendResponse);

    return a_pListener->AbortSendResponse(a_pListener, a_uStatus, a_psReason, a_ppOstrm);
}

/*============================================================================
 * OpcUa_Listener_CloseConnection
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Listener_CloseConnection(
    struct _OpcUa_Listener* a_pListener,
    OpcUa_Handle            a_hConnection,
    OpcUa_StatusCode        a_uStatus)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_Listener);
    OpcUa_ReturnErrorIfArgumentNull(a_pListener);
    OpcUa_ReturnErrorIfArgumentNull(a_pListener->AbortSendResponse);

    return a_pListener->CloseConnection(a_pListener, a_hConnection, a_uStatus);
}

/*============================================================================
 * OpcUa_Listener_GetReceiveBufferSize
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_Listener_GetReceiveBufferSize(
    struct _OpcUa_Listener* a_pListener,
    OpcUa_Handle            a_hConnection,
    OpcUa_UInt32*           a_pBufferSize)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_Listener);
    OpcUa_ReturnErrorIfArgumentNull(a_pListener);
    OpcUa_ReturnErrorIfArgumentNull(a_pListener->GetReceiveBufferSize);

    return a_pListener->GetReceiveBufferSize(a_pListener, a_hConnection, a_pBufferSize);
}

/*============================================================================
 * OpcUa_Listener_GetReceiveBufferSize
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_Listener_GetSecurityPolicyConfiguration(
    struct _OpcUa_Listener*                     a_pListener,
    OpcUa_InputStream*                          a_pIstrm,
    OpcUa_Listener_SecurityPolicyConfiguration* a_pSecurityPolicyConfiguration)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_Listener);
    OpcUa_ReturnErrorIfArgumentNull(a_pListener);
    OpcUa_ReturnErrorIfArgumentNull(a_pListener->GetSecurityPolicyConfiguration);

    return a_pListener->GetSecurityPolicyConfiguration(a_pListener, a_pIstrm, a_pSecurityPolicyConfiguration);
}

/*============================================================================
 * OpcUa_Listener_GetPeerInfo
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_Listener_GetPeerInfo(
    struct _OpcUa_Listener* a_pListener,
    OpcUa_Handle            a_hConnection,
    OpcUa_String*           a_pPeerInfo)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_Listener);
    OpcUa_ReturnErrorIfArgumentNull(a_pListener);
    OpcUa_ReturnErrorIfArgumentNull(a_pListener->GetPeerInfo);

    return a_pListener->GetPeerInfo(a_pListener, a_hConnection, a_pPeerInfo);
}

/*============================================================================
 * OpcUa_Listener_Delete
 *===========================================================================*/
OpcUa_Void OpcUa_Listener_Delete(OpcUa_Listener** a_pListener)
{
    if (a_pListener != OpcUa_Null && *a_pListener != OpcUa_Null)
    {
        (*a_pListener)->Delete(a_pListener);
    }
}

/*============================================================================
 * OpcUa_Listener_AddToSendQueue
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_Listener_AddToSendQueue(
    struct _OpcUa_Listener*  a_pListener,
    OpcUa_Handle             a_hConnection,
    OpcUa_BufferListElement* a_pBufferList,
    OpcUa_UInt32             a_uFlags)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_Listener);
    OpcUa_ReturnErrorIfArgumentNull(a_pListener);
    OpcUa_ReturnErrorIfArgumentNull(a_pListener->AddToSendQueue);

    return a_pListener->AddToSendQueue(a_pListener, a_hConnection, a_pBufferList, a_uFlags);
}
