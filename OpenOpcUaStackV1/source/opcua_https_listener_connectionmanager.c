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

#ifdef OPCUA_HAVE_SERVERAPI

#include <opcua_mutex.h>
#include <opcua_datetime.h>
#include <opcua_socket.h>
#include <opcua_statuscodes.h>
#include <opcua_guid.h>
#include <opcua_list.h>
#include <opcua_timer.h>
#include <opcua_utilities.h>

#include <opcua_httpsstream.h>

#include <opcua_https_listener.h>
#include <opcua_https_listener_connectionmanager.h>

#define OPCUA_HTTPSLISTENER_CONNECTIONMANAGER_TRACEREFCOUNT OPCUA_CONFIG_NO

#if OPCUA_HTTPSLISTENER_CONNECTIONMANAGER_TRACEREFCOUNT
# define OPCUA_HLCM_TRACEREF    OpcUa_Trace
#else /* OPCUA_HTTPSLISTENER_CONNECTIONMANAGER_TRACEREFCOUNT */
# define OPCUA_HLCM_TRACEREF(...)
#endif /* OPCUA_HTTPSLISTENER_CONNECTIONMANAGER_TRACEREFCOUNT */

/** @brief Clear an HttpListener_Connection */
OpcUa_Void              OpcUa_HttpsListener_Connection_Clear(               OpcUa_HttpsListener_Connection*    pValue);

/** @brief Initialize an HttpListener_Connection */
OpcUa_StatusCode        OpcUa_HttpsListener_Connection_Initialize(          OpcUa_HttpsListener_Connection*    pValue);

#if OPCUA_HTTPSLISTENER_CONNECTIONMANAGER_ENABLEWATCHDOG

/*============================================================================
 * HTTPS connection watchdog timer handler.
 *===========================================================================*/
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_HttpsListener_ConnectionManager_WatchdogTimerCallback(
    OpcUa_Void*     a_pvCallbackData,
    OpcUa_Timer     a_hTimer,
    OpcUa_UInt32    a_msecElapsed)
{
    OpcUa_HttpsListener_Connection*         pConnection         = OpcUa_Null;
    OpcUa_HttpsListener_ConnectionManager*  pConnectionManager  = (OpcUa_HttpsListener_ConnectionManager*)a_pvCallbackData;
    OpcUa_UInt32                            uCurrentTime        = 0;
    OpcUa_UInt32                            uTimeDiff           = 0;
    OpcUa_UInt32                            uConnectionCount    = 0;

OpcUa_InitializeStatus(OpcUa_Module_HttpListener, "ConnectionManager_WatchdogTimerCallback");

    OpcUa_ReferenceParameter(a_msecElapsed);
    OpcUa_ReferenceParameter(a_hTimer);

    OpcUa_ReturnErrorIfArgumentNull(pConnectionManager);

    OpcUa_List_Enter(pConnectionManager->Connections);

    OpcUa_List_GetNumberOfElements( pConnectionManager->Connections,
                                   &uConnectionCount);

    if(uConnectionCount > 0)
    {
        OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_HttpsListener_ConnectionManager_WatchdogTimerCallback: Checking %u connections of listener 0x%08X!\n", uConnectionCount, pConnectionManager->Listener);
    }
    else
    {
        OpcUa_List_Leave(pConnectionManager->Connections);
        OpcUa_ReturnStatusCode;
    }

    uCurrentTime = OpcUa_GetTickCount();

    OpcUa_List_ResetCurrent(pConnectionManager->Connections);
    pConnection = (OpcUa_HttpsListener_Connection*)OpcUa_List_GetCurrentElement(pConnectionManager->Connections);

    while(pConnection != OpcUa_Null)
    {
        OpcUa_Mutex_Lock(pConnection->Mutex);

        /* compare timestamps */
        if(uCurrentTime < pConnection->uLastReceiveTime)
        {
            /* overflow */
            uTimeDiff = (OpcUa_UInt32_Max - pConnection->uLastReceiveTime) + uCurrentTime;
        }
        else
        {
            uTimeDiff = uCurrentTime - pConnection->uLastReceiveTime;
        }

        if(     uTimeDiff >= OPCUA_HTTPSLISTENER_CONNECTIONMANAGER_TIMEOUT
            &&  pConnection->pOutputStream == OpcUa_Null)
        {
            /* timeout */
            OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_HttpsListener_ConnectionManager_WatchdogTimerCallback: Connection 0x%08X timeout!\n", pConnection);

            uStatus = OpcUa_HttpsListener_ConnectionManager_ReleaseConnection(  pConnectionManager,
                                                                                &pConnection);

            if(OpcUa_IsEqual(OpcUa_GoodCallAgain))
            {
                pConnection = (OpcUa_HttpsListener_Connection *)OpcUa_List_GetNextElement(pConnectionManager->Connections);
            }
            else
            {
                pConnection = (OpcUa_HttpsListener_Connection*)OpcUa_List_GetCurrentElement(pConnectionManager->Connections);
            }
        }
        else
        {
            OpcUa_Mutex_Unlock(pConnection->Mutex);
            pConnection = (OpcUa_HttpsListener_Connection *)OpcUa_List_GetNextElement(pConnectionManager->Connections);
        }
    }

    OpcUa_List_Leave(pConnectionManager->Connections);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

#endif /* OPCUA_HTTPSLISTENER_CONNECTIONMANAGER_ENABLEWATCHDOG */

/*============================================================================
 * HTTPS connection watchdog kill handler.
 *===========================================================================*/
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_HttpsListener_ConnectionManager_WatchdogTimerKillCallback(
    OpcUa_Void*     a_pvCallbackData,
    OpcUa_Timer     a_hTimer,
    OpcUa_UInt32    a_msecElapsed)
{
OpcUa_InitializeStatus(OpcUa_Module_HttpListener, "ConnectionManager_WatchdogTimerKillCallback");

    OpcUa_ReferenceParameter(a_pvCallbackData);
    OpcUa_ReferenceParameter(a_hTimer);
    OpcUa_ReferenceParameter(a_msecElapsed);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * Connection Manager Create
 *===========================================================================*/
/**
 * Create a new connection manager
 * @param a_ppConnectionManager Description
 * @return StatusCode
 */
OpcUa_StatusCode OpcUa_HttpsListener_ConnectionManager_Create(
    OpcUa_HttpsListener_ConnectionDeleteCB      a_pfConnectionRemovedCallback,
    OpcUa_HttpsListener_ConnectionManager**     a_ppConnectionManager)
{
    OpcUa_HttpsListener_ConnectionManager *pConnMngr  = OpcUa_Null;
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_HttpListener);

    OpcUa_ReturnErrorIfArgumentNull(a_ppConnectionManager);

    *a_ppConnectionManager = OpcUa_Null;

    pConnMngr = (OpcUa_HttpsListener_ConnectionManager*)OpcUa_Alloc(sizeof(OpcUa_HttpsListener_ConnectionManager));
    OpcUa_ReturnErrorIfAllocFailed(pConnMngr);

    OpcUa_HttpsListener_ConnectionManager_Initialize(pConnMngr, a_pfConnectionRemovedCallback);

    if(pConnMngr->Connections == OpcUa_Null)
    {
        OpcUa_HttpsListener_ConnectionManager_Delete(&pConnMngr);
    }

    *a_ppConnectionManager = pConnMngr;
    return OpcUa_Good;
}


/*============================================================================
 * Connection Manager Initialize
 *===========================================================================*/
/**
 * @brief Initialize a allocated connection manager.
 */
OpcUa_StatusCode OpcUa_HttpsListener_ConnectionManager_Initialize(
    OpcUa_HttpsListener_ConnectionManager*  a_pConnectionManager,
    OpcUa_HttpsListener_ConnectionDeleteCB  a_pfConnectionRemovedCallback)
{
    OpcUa_StatusCode uStatus    = OpcUa_Good;
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_HttpListener);

    if (a_pConnectionManager == OpcUa_Null)
    {
        return OpcUa_BadInvalidArgument;
    }

    OpcUa_MemSet(a_pConnectionManager, 0, sizeof(OpcUa_HttpsListener_ConnectionManager));

    uStatus = OpcUa_List_Create(    &(a_pConnectionManager->Connections));
    OpcUa_ReturnErrorIfBad(uStatus);

    a_pConnectionManager->pfConnectionDeleteCB = a_pfConnectionRemovedCallback;

#if OPCUA_HTTPSLISTENER_CONNECTIONMANAGER_ENABLEWATCHDOG
    /* create watchdog timer for outstanding responses. */
    uStatus = OpcUa_Timer_Create(   &(a_pConnectionManager->hWatchdogTimer),
                                    OPCUA_HTTPSLISTENER_CONNECTIONMANAGER_WATCHDOGINTERVAL,
                                    OpcUa_HttpsListener_ConnectionManager_WatchdogTimerCallback,
                                    OpcUa_HttpsListener_ConnectionManager_WatchdogTimerKillCallback,
                                    (OpcUa_Void*)(a_pConnectionManager));
    OpcUa_ReturnErrorIfBad(uStatus);
#endif

    return uStatus;
}

/*============================================================================
 * Connection Manager Clear
 *===========================================================================*/
/**
 * @brief Clear a connection manager.
 */
OpcUa_Void OpcUa_HttpsListener_ConnectionManager_Clear(
    OpcUa_HttpsListener_ConnectionManager* a_pConnectionManager)
{
    if(a_pConnectionManager == OpcUa_Null)
    {
        return;
    }

#if OPCUA_HTTPSLISTENER_CONNECTIONMANAGER_ENABLEWATCHDOG
    /* delete the timer for the watchdog */
    if(a_pConnectionManager->hWatchdogTimer != OpcUa_Null)
    {
        OpcUa_Timer_Delete(&(a_pConnectionManager->hWatchdogTimer));
    }
#endif

    /* be sure to delete all connections before! */
    OpcUa_List_Delete(&(a_pConnectionManager->Connections));
}

/*============================================================================
 * Connection Manager Delete
 *===========================================================================*/
/**
 * @brief Delete an connection manager.
 */
OpcUa_Void OpcUa_HttpsListener_ConnectionManager_Delete(
    OpcUa_HttpsListener_ConnectionManager** a_ppConnectionManager)
{
    if (a_ppConnectionManager != OpcUa_Null)
    {
        OpcUa_HttpsListener_ConnectionManager_Clear(*a_ppConnectionManager);
        OpcUa_Free(*a_ppConnectionManager);
        *a_ppConnectionManager = OpcUa_Null;
        return;
    }
}

/*==============================================================================*/
/* Get Connection                                                               */
/*==============================================================================*/
/**
 * @brief Retrieves a started stream for the connection identified by the socket.
 *
 * @return StatusCode
 */
OpcUa_StatusCode OpcUa_HttpsListener_ConnectionManager_GetConnectionBySocket(
    OpcUa_HttpsListener_ConnectionManager*    a_pConnectionManager,
    OpcUa_Socket                              a_pSocket,
    OpcUa_HttpsListener_Connection**          a_ppConnection)
{
    OpcUa_StatusCode                  uStatus         = OpcUa_Good;
    OpcUa_HttpsListener_Connection*   tmpConnection   = OpcUa_Null;

    OpcUa_ReturnErrorIfArgumentNull(a_pConnectionManager);
    OpcUa_ReturnErrorIfArgumentNull(a_pConnectionManager->Connections);
    OpcUa_ReturnErrorIfArgumentNull(a_pSocket);
    OpcUa_ReturnErrorIfArgumentNull(a_ppConnection);

    *a_ppConnection = OpcUa_Null;

    OpcUa_List_Enter(a_pConnectionManager->Connections);

    uStatus = OpcUa_BadNotFound;

    OpcUa_List_ResetCurrent(a_pConnectionManager->Connections);
    tmpConnection = (OpcUa_HttpsListener_Connection*)OpcUa_List_GetCurrentElement(a_pConnectionManager->Connections);

    while(tmpConnection != OpcUa_Null)
    {
        if(a_pSocket == tmpConnection->Socket)
        {
            *a_ppConnection = tmpConnection;

            uStatus = OpcUa_Good;
            break;
        }
        tmpConnection = (OpcUa_HttpsListener_Connection *)OpcUa_List_GetNextElement(a_pConnectionManager->Connections);
    }

    if(tmpConnection != OpcUa_Null)
    {
        tmpConnection->iReferenceCount++;
        OPCUA_HLCM_TRACEREF(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_HttpsListener_ConnectionManager_GetConnectionBySocket: 0x%08X increasing RefCount %u\n", tmpConnection, tmpConnection->iReferenceCount);
    }

    OpcUa_List_Leave(a_pConnectionManager->Connections);

    return uStatus;
}

/*==============================================================================*/
/*                                                                              */
/*==============================================================================*/
/**
* @brief Remove a connection identified by the connection object itself (if no id was assigned ie. pre validation)
*
* @return: Status Code;
*/
OpcUa_StatusCode OpcUa_HttpsListener_ConnectionManager_RemoveConnection(
    OpcUa_HttpsListener_ConnectionManager*    a_pConnectionManager,
    OpcUa_HttpsListener_Connection*           a_pConnection)
{
    OpcUa_StatusCode uStatus = OpcUa_Good;

    OpcUa_List_Enter(a_pConnectionManager->Connections);
    uStatus = OpcUa_List_DeleteElement(a_pConnectionManager->Connections, a_pConnection);
    OpcUa_List_Leave(a_pConnectionManager->Connections);

    return uStatus;
}

/*==============================================================================*/
/*                                                                              */
/*==============================================================================*/
/**
* @brief Release reference to given connection
*
* @return: Status Code;
*/
OpcUa_StatusCode OpcUa_HttpsListener_ConnectionManager_ReleaseConnection(
    OpcUa_HttpsListener_ConnectionManager*    a_pConnectionManager,
    OpcUa_HttpsListener_Connection**          a_ppConnection)
{
    /* OpcUa_GoodCallAgain indicates that connection still exists. */
    OpcUa_StatusCode uStatus = OpcUa_GoodCallAgain;

    OpcUa_List_Enter(a_pConnectionManager->Connections);

    (*a_ppConnection)->iReferenceCount--;

    if((*a_ppConnection)->iReferenceCount <= 0)
    {
        OPCUA_HLCM_TRACEREF(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_HttpsListener_ConnectionManager_ReleaseConnection: Deleting 0x%08X -- RefCount %u\n", (*a_ppConnection), (*a_ppConnection)->iReferenceCount);

        OpcUa_List_DeleteElement(a_pConnectionManager->Connections, (*a_ppConnection));

        if(a_pConnectionManager->pfConnectionDeleteCB != OpcUa_Null)
        {
            a_pConnectionManager->pfConnectionDeleteCB( a_pConnectionManager->Listener,
                                                        (*a_ppConnection));
        }

        OpcUa_HttpsListener_Connection_Delete(a_ppConnection);

        uStatus = OpcUa_Good;
    }
    else
    {
        OPCUA_HLCM_TRACEREF(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_HttpsListener_ConnectionManager_ReleaseConnection: 0x%08X -- RefCount %u\n", (*a_ppConnection), (*a_ppConnection)->iReferenceCount);
    }

    OpcUa_List_Leave(a_pConnectionManager->Connections);

    return uStatus;
}

/*==============================================================================*/
/*                                                                              */
/*==============================================================================*/
/**
* @brief Builds a connection record for the given parameters and returns it.
*
* @return: Status Code;
*/
OpcUa_StatusCode OpcUa_HttpsListener_ConnectionManager_AddConnection(
    OpcUa_HttpsListener_ConnectionManager*    a_pConnectionManager,
    OpcUa_HttpsListener_Connection*           a_pConnection)
{
    OpcUa_StatusCode    uStatus = OpcUa_Good;

    OpcUa_GotoErrorIfArgumentNull(a_pConnection);
    OpcUa_GotoErrorIfArgumentNull(a_pConnectionManager);
    OpcUa_GotoErrorIfArgumentNull(a_pConnectionManager->Connections);

    a_pConnection->uConnectTime     = OpcUa_GetTickCount(); /* expiration of connection would be DisconnectTime+Lifetime */

    OpcUa_List_Enter(a_pConnectionManager->Connections);
    OpcUa_List_AddElement(a_pConnectionManager->Connections, a_pConnection);
    OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_HttpsListener_ConnectionManager_AddConnection: Connection added!\n");
    OpcUa_List_Leave(a_pConnectionManager->Connections);


    return OpcUa_Good;

Error:
    return OpcUa_Bad;
}

/*==============================================================================*/
/* Delete all connections                                                       */
/*==============================================================================*/
/* Iterates over all connections, closes and deletes each. The given callback is called everytime. */
OpcUa_StatusCode OpcUa_HttpsListener_ConnectionManager_RemoveConnections(
    OpcUa_HttpsListener_ConnectionManager* a_pConnectionManager)
{
    OpcUa_StatusCode                  uStatus         = OpcUa_Good;
    OpcUa_HttpsListener_Connection*   httpConnection   = OpcUa_Null;

    OpcUa_DeclareErrorTraceModule(OpcUa_Module_HttpListener);

    OpcUa_ReturnErrorIfArgumentNull(a_pConnectionManager);
    /*OpcUa_ReturnErrorIfArgumentNull(a_fConnectionDeleteCB);*/

    /* obtain lock on the list */
    OpcUa_List_Enter(a_pConnectionManager->Connections);

    /* prepare cursor for iteration over all elements */
    OpcUa_List_ResetCurrent(a_pConnectionManager->Connections);
    httpConnection = (OpcUa_HttpsListener_Connection*)OpcUa_List_GetCurrentElement(a_pConnectionManager->Connections);

    /* check every Connection for deletion */
    while(httpConnection != OpcUa_Null)
    {
        if(a_pConnectionManager->pfConnectionDeleteCB != OpcUa_Null)
        {
            a_pConnectionManager->pfConnectionDeleteCB( a_pConnectionManager->Listener,
                                                        httpConnection);
        }

        OpcUa_HttpsListener_Connection_Delete((&httpConnection));
        OpcUa_List_DeleteCurrentElement(a_pConnectionManager->Connections);
        httpConnection = (OpcUa_HttpsListener_Connection*)OpcUa_List_GetCurrentElement(a_pConnectionManager->Connections);
    }

    /* list must be empty here */

    /* leave it */
    OpcUa_List_Leave(a_pConnectionManager->Connections);

    return uStatus;
}

/***  HttpListener_Connection Class  ***/


/*===========================================================================*/
/* Create                                                                    */
/*===========================================================================*/
/**
* @brief Allocate and initialize a new OpcUa_HttpsListener_Connection object.
*
* @return Bad status on fail; Good status on success;
*/
OpcUa_StatusCode OpcUa_HttpsListener_Connection_Create(OpcUa_HttpsListener_Connection** a_ppConnection)
{
    OpcUa_HttpsListener_Connection*   pConnection = OpcUa_Null;

    OpcUa_DeclareErrorTraceModule(OpcUa_Module_HttpListener);

    pConnection = (OpcUa_HttpsListener_Connection*) OpcUa_Alloc(sizeof(OpcUa_HttpsListener_Connection));
    OpcUa_ReturnErrorIfAllocFailed(pConnection);

    OpcUa_HttpsListener_Connection_Initialize(pConnection);

    pConnection->iReferenceCount    = 1;
    OPCUA_HLCM_TRACEREF(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_HttpsListener_Connection_Create: 0x%08X initial refcount %u\n", pConnection, pConnection->iReferenceCount);
    pConnection->pOutputStream      = OpcUa_Null;

    *a_ppConnection = pConnection;

    return OpcUa_Good;
}

/*==============================================================================*/
/* Initialize                                                                   */
/*==============================================================================*/
/**
* @brief Initializes the given parameter and allocates embedded objects.
* If embedded objects cannot be allocated, the given pointer is set to OpcUa_Null.
*
* @return Bad status on fail; Good status on success;
*/
OpcUa_StatusCode OpcUa_HttpsListener_Connection_Initialize(OpcUa_HttpsListener_Connection* a_pConnection)
{
    OpcUa_StatusCode uStatus    = OpcUa_Good;
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_HttpListener);

    OpcUa_ReturnErrorIfArgumentNull(a_pConnection);

    OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_HttpsListener_Connection_Initialize: 0x%08X!\n", a_pConnection);

    a_pConnection->uConnectTime             = 0;
    a_pConnection->uDisconnectTime          = 0;
    a_pConnection->uLastReceiveTime         = 0;
    a_pConnection->iReferenceCount          = 0;

    a_pConnection->Socket                   = OpcUa_Null;
    a_pConnection->pListenerHandle          = OpcUa_Null;
    a_pConnection->pInputStream             = OpcUa_Null;
    a_pConnection->uNoOfRequestsTotal       = 0;
    a_pConnection->bConnected               = OpcUa_False;

    OpcUa_String_Initialize(&a_pConnection->sSecurityPolicy);

    uStatus = OpcUa_Mutex_Create(&(a_pConnection->Mutex));
    OpcUa_ReturnErrorIfBad(uStatus);

    return OpcUa_Good;
}

/*==============================================================================*/
/* Clear                                                                        */
/*==============================================================================*/
/**
* @brief Clean up the given OpcUa_HttpsListener_Connection. May be used for static memory.
*/
OpcUa_Void OpcUa_HttpsListener_Connection_Clear(OpcUa_HttpsListener_Connection* a_pConnection)
{
    if(a_pConnection == OpcUa_Null)
    {
        return;
    }

    OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_HttpsListener_Connection_Clear: 0x%08X with socket 0x%08X!\n", a_pConnection, a_pConnection->Socket);

    a_pConnection->iReferenceCount          = 0;
    a_pConnection->uConnectTime             = 0;
    a_pConnection->uDisconnectTime          = 0;
    a_pConnection->uLastReceiveTime         = 0;

    a_pConnection->Socket                   = OpcUa_Null; /* Socket has to be freed externally. */
    a_pConnection->pListenerHandle          = OpcUa_Null;
    a_pConnection->uNoOfRequestsTotal       = 0;

    if(a_pConnection->pInputStream != OpcUa_Null)
    {
        OpcUa_Stream_Delete((OpcUa_Stream**)&a_pConnection->pInputStream);
    }

    OpcUa_String_Clear(&a_pConnection->sSecurityPolicy);

    if(a_pConnection->Mutex)
    {
        OpcUa_Mutex_Delete(&(a_pConnection->Mutex));
    }

    OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_HttpsListener_Connection_Clear: Done!\n");
}

/*==============================================================================*/
/* Delete                                                                       */
/*==============================================================================*/
/**
* @brief Clean up and free the given OpcUa_HttpsListener_Connection.
*/
OpcUa_Void OpcUa_HttpsListener_Connection_Delete(OpcUa_HttpsListener_Connection** a_ppConnection)
{
    if(a_ppConnection == OpcUa_Null || *a_ppConnection == OpcUa_Null)
    {
        return;
    }

    /* clean up internal resources */
    OpcUa_HttpsListener_Connection_Clear(*a_ppConnection);

    /* free instance memory */
    OpcUa_Free(*a_ppConnection);
    a_ppConnection = OpcUa_Null;
}

/*==============================================================================*/
/* Delete                                                                       */
/*==============================================================================*/
/**
* @brief .
*/
OpcUa_StatusCode OpcUa_HttpsListener_ConnectionManager_GetConnectionCount(
    OpcUa_HttpsListener_ConnectionManager*    a_pConnectionManager,
    OpcUa_UInt32*                             a_pNoOfConnections)
{
OpcUa_InitializeStatus(OpcUa_Module_HttpListener, "GetConnectionCout");

    OpcUa_ReturnErrorIfArgumentNull(a_pConnectionManager);
    OpcUa_ReturnErrorIfArgumentNull(a_pNoOfConnections);

    OpcUa_List_GetNumberOfElements(a_pConnectionManager->Connections, a_pNoOfConnections);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    /* nothing */

OpcUa_FinishErrorHandling;
}

#endif /* OPCUA_HAVE_SERVERAPI */

/*==============================================================================*/
/* End Of File                                                                  */
/*==============================================================================*/
