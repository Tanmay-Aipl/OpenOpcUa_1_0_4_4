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

#include <opcua_tcpstream.h>

#include <opcua_tcplistener.h>
#include <opcua_tcplistener_connectionmanager.h>
#define OPCUA_TCPLISTENER_CONNECTIONMANAGER_HANDLE_MAKE(xVersion, xId)  ((((OpcUa_UInt32)xVersion)<<16)|(((OpcUa_UInt32)(xId))&0x0000FFFF))
#define OPCUA_TCPLISTENER_CONNECTIONMANAGER_HANDLE_GET_VERSION(xHandle) ((OpcUa_UInt16)(((xHandle)&0xFFFF0000)>>16))
#define OPCUA_TCPLISTENER_CONNECTIONMANAGER_HANDLE_GET_ID(xHandle)      ((OpcUa_UInt16)((xHandle)&0x0000FFFF))
/*============================================================================
 * Connection Manager Create
 *===========================================================================*/
/** 
 * Create a new connection manager
 * @param a_ppConnectionManager Description
 * @return StatusCode
 */
OpcUa_StatusCode OpcUa_TcpListener_ConnectionManager_Create(
    OpcUa_TcpListener_ConnectionManager** a_ppConnectionManager)
{
    OpcUa_TcpListener_ConnectionManager *pConnMngr  = OpcUa_Null;
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_TcpListener);

    OpcUa_ReturnErrorIfArgumentNull(a_ppConnectionManager);

    *a_ppConnectionManager = OpcUa_Null;

    pConnMngr = (OpcUa_TcpListener_ConnectionManager*)OpcUa_Alloc(sizeof(OpcUa_TcpListener_ConnectionManager));
    OpcUa_ReturnErrorIfAllocFailed(pConnMngr);

    OpcUa_TcpListener_ConnectionManager_Initialize(pConnMngr);

    if(pConnMngr->Connections == OpcUa_Null)
    {
        OpcUa_TcpListener_ConnectionManager_Delete(&pConnMngr);
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
OpcUa_StatusCode OpcUa_TcpListener_ConnectionManager_Initialize(
    OpcUa_TcpListener_ConnectionManager* a_pConnectionManager)
{
    OpcUa_StatusCode    uStatus = OpcUa_Good;
    OpcUa_Int           i       = 0;

    OpcUa_DeclareErrorTraceModule(OpcUa_Module_TcpListener);

    if (a_pConnectionManager == OpcUa_Null)
    {
        return OpcUa_BadInvalidArgument;
    }

    OpcUa_MemSet(a_pConnectionManager, 0, sizeof(OpcUa_TcpListener_ConnectionManager));

    uStatus = OpcUa_Mutex_Create(&a_pConnectionManager->pMutex);
    OpcUa_ReturnErrorIfBad(uStatus);

    /* initialize all id parts of the connection handles with their index */
    for(; i < OPCUA_TCPLISTENER_MAXCONNECTIONS; i++)
    {
        a_pConnectionManager->Connections[i].iConnectionHandle = i;
    }

    return uStatus;
}

/*============================================================================
 * Connection Manager Clear
 *===========================================================================*/
/**
 * @brief Clear a connection manager.
 */
OpcUa_Void OpcUa_TcpListener_ConnectionManager_Clear(
    OpcUa_TcpListener_ConnectionManager* a_pConnectionManager)
{
    if (a_pConnectionManager == OpcUa_Null)
    {
        return;
    }

#if OPCUA_USE_SYNCHRONISATION
    if(a_pConnectionManager->pMutex != OpcUa_Null)
    {
        OpcUa_Mutex_Delete(&a_pConnectionManager->pMutex);
    }
#endif /* OPCUA_USE_SYNCHRONISATION */
}

/*============================================================================
 * Connection Manager Delete
 *===========================================================================*/
/**
 * @brief Delete an connection manager.
 */
OpcUa_Void OpcUa_TcpListener_ConnectionManager_Delete(
    OpcUa_TcpListener_ConnectionManager** a_ppConnectionManager)
{
    if (a_ppConnectionManager != OpcUa_Null)
    {
        OpcUa_TcpListener_ConnectionManager_Clear(*a_ppConnectionManager);
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
OpcUa_StatusCode OpcUa_TcpListener_ConnectionManager_GetConnectionBySocket(
    OpcUa_TcpListener_ConnectionManager*    a_pConnectionManager, 
    OpcUa_Socket                            a_pSocket, 
    OpcUa_TcpListener_Connection**          a_ppConnection)
{
    OpcUa_StatusCode                uStatus     = OpcUa_Good;
    OpcUa_Int                       i           = 0;

    OpcUa_ReturnErrorIfArgumentNull(a_pConnectionManager);
    OpcUa_ReturnErrorIfArgumentNull(a_pConnectionManager->Connections);
    OpcUa_ReturnErrorIfArgumentNull(a_pSocket);
    OpcUa_ReturnErrorIfArgumentNull(a_ppConnection);

    *a_ppConnection = OpcUa_Null;
    uStatus = OpcUa_BadNotFound;

    OpcUa_Mutex_Lock(a_pConnectionManager->pMutex);

    /* find connection having given socket. */
    for(; i < OPCUA_TCPLISTENER_MAXCONNECTIONS; i++)
    {
        if(     a_pConnectionManager->Connections[i].bUsed  != OpcUa_False
            &&  a_pConnectionManager->Connections[i].Socket == a_pSocket)
        {
            *a_ppConnection = &a_pConnectionManager->Connections[i];
            uStatus = OpcUa_Good;
            break;
        }
    }

    OpcUa_Mutex_Unlock(a_pConnectionManager->pMutex);

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
//OpcUa_StatusCode OpcUa_TcpListener_ConnectionManager_RemoveConnection(
//    OpcUa_TcpListener_ConnectionManager*    a_pConnectionManager, 
//    OpcUa_TcpListener_Connection*           a_pConnection)
//{
//    OpcUa_StatusCode uStatus = OpcUa_Good;
//
//    OpcUa_List_Enter(a_pConnectionManager->Connections);
//    uStatus = OpcUa_List_DeleteElement(a_pConnectionManager->Connections, a_pConnection);
//    OpcUa_List_Leave(a_pConnectionManager->Connections);
//
//    OpcUa_Trace(OPCUA_TRACE_LEVEL_INFO, "OpcUa_TcpListener_ConnectionManager_RemoveConnection: %p removed with status 0x%08X!\n", a_pConnection, uStatus);
//
//    return uStatus;
//}


/*==============================================================================*/
/*                                                                              */
/*==============================================================================*/
/**
* @brief Builds a connection record for the given parameters and returns it. 
*
* @return: Status Code;
*/
//OpcUa_StatusCode OpcUa_TcpListener_ConnectionManager_AddConnection(
//    OpcUa_TcpListener_ConnectionManager*    a_pConnectionManager, 
//    OpcUa_TcpListener_Connection*           a_pConnection)
//{
//    OpcUa_StatusCode    uStatus = OpcUa_Good;
//    
//    OpcUa_GotoErrorIfArgumentNull(a_pConnection);   
//    OpcUa_GotoErrorIfArgumentNull(a_pConnectionManager);
//    OpcUa_GotoErrorIfArgumentNull(a_pConnectionManager->Connections);
//    
//    a_pConnection->ConnectTime  = OPCUA_P_DATETIME_UTCNOW(); /* expiration of connection would be DisconnectTime+Lifetime */
//    
//    OpcUa_List_Enter(a_pConnectionManager->Connections);
//    OpcUa_List_AddElement(a_pConnectionManager->Connections, a_pConnection);
//    OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_DEBUG, "OpcUa_TcpListener_ConnectionManager_AddConnection: Connection added!\n");
//    OpcUa_List_Leave(a_pConnectionManager->Connections);
//    
//    
//    return OpcUa_Good;
//    
//Error:
//    return OpcUa_Bad;
//}

/*==============================================================================*/
/* Delete all connections                                                       */
/*==============================================================================*/
/* Iterates over all connections, closes and deletes each. The given callback is called everytime. */
OpcUa_StatusCode OpcUa_TcpListener_ConnectionManager_RemoveConnections( 
    OpcUa_TcpListener_ConnectionManager* a_pConnectionManager, 
    OpcUa_TcpListener_ConnectionDeleteCB a_fConnectionDeleteCB)
{
    OpcUa_StatusCode    uStatus = OpcUa_Good;
    OpcUa_Int           i       = 0;

    OpcUa_DeclareErrorTraceModule(OpcUa_Module_TcpListener);

    OpcUa_ReturnErrorIfArgumentNull(a_pConnectionManager);

    /* obtain lock on the list */
    OpcUa_Mutex_Lock(a_pConnectionManager->pMutex);

    /* Find connection haven given handle. */
    for(; i < OPCUA_TCPLISTENER_MAXCONNECTIONS; i++)
    {
        if(a_pConnectionManager->Connections[i].bUsed != OpcUa_False)
        {
            /* notify about deletion */
            if(a_fConnectionDeleteCB != OpcUa_Null)
            {
                a_fConnectionDeleteCB(  a_pConnectionManager->Listener,
                                       &a_pConnectionManager->Connections[i]);
            }

            /* free memory */
            OpcUa_TcpListener_Connection_Clear((&a_pConnectionManager->Connections[i]));
        }
    }

    /* leave it */
    OpcUa_Mutex_Unlock(a_pConnectionManager->pMutex);

    return uStatus;
}
/*===========================================================================*/
/* Create                                                                    */
/*===========================================================================*/
/**
* @brief Allocate and initialize a new OpcUa_TcpListener_Connection object.
*
* @return Bad status on fail; Good status on success;
*/
OpcUa_StatusCode OpcUa_TcpListener_ConnectionManager_CreateConnection(
    OpcUa_TcpListener_ConnectionManager*    a_pConnectionManager, 
    OpcUa_TcpListener_Connection**          a_ppConnection)
{
    OpcUa_TcpListener_Connection*   pConnection = OpcUa_Null;
    OpcUa_Int                       i           = 0;
    OpcUa_StatusCode                uStatus     = OpcUa_Good;

    OpcUa_DeclareErrorTraceModule(OpcUa_Module_TcpListener);

    OpcUa_Mutex_Lock(a_pConnectionManager->pMutex);

    /* TODO: find unused connection object and return it */
    for(; i < OPCUA_TCPLISTENER_MAXCONNECTIONS; i++)
    {
        if(a_pConnectionManager->Connections[i].bUsed == OpcUa_False)
        {
            pConnection = &a_pConnectionManager->Connections[i];
            break;
        }
    }

    if(pConnection)
    {
        OpcUa_TcpListener_Connection_Initialize(pConnection);
        *a_ppConnection = pConnection;
        a_pConnectionManager->uUsedConnections++;
    }
    else
    {
        uStatus = OpcUa_BadNotFound;
        *a_ppConnection = OpcUa_Null;
    }

    OpcUa_Mutex_Unlock(a_pConnectionManager->pMutex);

    return uStatus;
}

/*==============================================================================*/
/* Delete                                                                       */
/*==============================================================================*/
/**
* @brief Clean up and free the given OpcUa_TcpListener_Connection.
*/
OpcUa_Void OpcUa_TcpListener_ConnectionManager_DeleteConnection(
    OpcUa_TcpListener_ConnectionManager*    a_pConnectionManager, 
    OpcUa_TcpListener_Connection**          a_ppConnection)
{   
    if(a_ppConnection == OpcUa_Null || *a_ppConnection == OpcUa_Null)
    {
        return;
    }

    if(((*a_ppConnection)->Socket) != OpcUa_Null)
    {
        OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_TcpListener_Connection_Delete: HANDLE LEAK!\n");
    }

    OpcUa_Mutex_Lock(a_pConnectionManager->pMutex);

    OpcUa_TcpListener_Connection_Clear(*a_ppConnection);
    a_pConnectionManager->uUsedConnections--;

    OpcUa_Mutex_Unlock(a_pConnectionManager->pMutex);
}
/***  TcpListener_Connection Class  ***/


/*===========================================================================*/
/* Create                                                                    */
/*===========================================================================*/
/**
* @brief Allocate and initialize a new OpcUa_TcpListener_Connection object.
*
* @return Bad status on fail; Good status on success;
*/
//OpcUa_StatusCode OpcUa_TcpListener_Connection_Create(OpcUa_TcpListener_Connection** a_ppConnection)
//{
//    OpcUa_TcpListener_Connection*   pConnection = OpcUa_Null;
//
//    OpcUa_DeclareErrorTraceModule(OpcUa_Module_TcpListener);
//
//    pConnection = (OpcUa_TcpListener_Connection*) OpcUa_Alloc(sizeof(OpcUa_TcpListener_Connection));
//    OpcUa_ReturnErrorIfAllocFailed(pConnection);
//    OpcUa_MemSet(pConnection, 0, sizeof(OpcUa_TcpListener_Connection));
//
//    pConnection->uCurrentChunk  = 0;
//    pConnection->pOutputStream  = OpcUa_Null;
//
//    OpcUa_TcpListener_Connection_Initialize(pConnection);
//
//    *a_ppConnection = pConnection;
//    return OpcUa_Good;
//}

/*==============================================================================*/
/* Initialize                                                                   */
/*==============================================================================*/
/**
* @brief Initializes the given parameter and allocates embedded objects.
* If embedded objects cannot be allocated, the given pointer is set to OpcUa_Null.
*
* @return Bad status on fail; Good status on success;
*/
OpcUa_StatusCode OpcUa_TcpListener_Connection_Initialize(OpcUa_TcpListener_Connection* a_pConnection)
{
    OpcUa_StatusCode    uStatus     = OpcUa_Good;
    OpcUa_UInt16        usId        = 0;
    OpcUa_UInt16        usVersion   = 0;

    OpcUa_DeclareErrorTraceModule(OpcUa_Module_TcpListener);

    OpcUa_ReturnErrorIfArgumentNull(a_pConnection);

    /* save handle */
    usId        = OPCUA_TCPLISTENER_CONNECTIONMANAGER_HANDLE_GET_ID((a_pConnection->iConnectionHandle));
    usVersion   = OPCUA_TCPLISTENER_CONNECTIONMANAGER_HANDLE_GET_VERSION((a_pConnection->iConnectionHandle));

    OpcUa_MemSet(&(a_pConnection->ConnectTime), 0, sizeof(OpcUa_DateTime));

    /* restore handle */
    usVersion++;

    /* skip 0 after overflow */
    if(usVersion == 0)
    {
        usVersion++;
    }

    a_pConnection->iConnectionHandle = OPCUA_TCPLISTENER_CONNECTIONMANAGER_HANDLE_MAKE(usVersion, usId);

    OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_DEBUG, "OpcUa_TcpListener_Connection_Initialize: created 0x%p->%u(v%u)!\n", a_pConnection, usId, usVersion);

    a_pConnection->bDelete                  = OpcUa_False;
    a_pConnection->bValid                   = OpcUa_True;

#if OPCUA_P_SOCKETGETPEERINFO_V2
    OpcUa_MemSet(a_pConnection->achPeerInfo, 0, OPCUA_P_PEERINFO_MIN_SIZE);
#else /* OPCUA_P_SOCKETGETPEERINFO_V2 */
    a_pConnection->PeerPort                 = 0;
    a_pConnection->PeerIp                   = 0;
#endif /* OPCUA_P_SOCKETGETPEERINFO_V2 */

    a_pConnection->Socket                   = OpcUa_Null;
    a_pConnection->pListenerHandle          = OpcUa_Null;
    a_pConnection->pInputStream             = OpcUa_Null;
    a_pConnection->uNoOfRequestsTotal       = 0;
    a_pConnection->bConnected               = OpcUa_False;
    a_pConnection->bUsed                    = OpcUa_True;

    OpcUa_String_Initialize(&a_pConnection->sURL);

    uStatus = OpcUa_Mutex_Create(&(a_pConnection->Mutex));
    OpcUa_ReturnErrorIfBad(uStatus);

    a_pConnection->ReceiveBufferSize        = 0;
    a_pConnection->SendBufferSize           = 0;

    return OpcUa_Good;
}

/*==============================================================================*/
/* Clear                                                                        */
/*==============================================================================*/
/**
* @brief Clean up the given OpcUa_TcpListener_Connection. May be used for static memory.
*/
OpcUa_Void OpcUa_TcpListener_Connection_Clear(OpcUa_TcpListener_Connection* a_pConnection)
{
    OpcUa_UInt16 usId      = 0;
    OpcUa_UInt16 usVersion = 0;

    if(a_pConnection == OpcUa_Null)
    {
        return;
    }

#if OPCUA_USE_SYNCHRONISATION
    if(a_pConnection->Mutex != OpcUa_Null)
    {
        OpcUa_Mutex_Lock(a_pConnection->Mutex);
    }
#endif /* OPCUA_USE_SYNCHRONISATION */

    /* save handle */
    usId        = OPCUA_TCPLISTENER_CONNECTIONMANAGER_HANDLE_GET_ID((a_pConnection->iConnectionHandle));
    usVersion   = OPCUA_TCPLISTENER_CONNECTIONMANAGER_HANDLE_GET_VERSION((a_pConnection->iConnectionHandle));

    OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_DEBUG, "OpcUa_TcpListener_Connection_Clear: clearing 0x%p->%u(v%u)!\n", a_pConnection, usId, usVersion);

    OpcUa_String_Clear(&a_pConnection->sURL);

#if OPCUA_USE_SYNCHRONISATION
    if(a_pConnection->Mutex != OpcUa_Null)
    {
        OpcUa_Mutex_Unlock(a_pConnection->Mutex);
        OpcUa_Mutex_Delete(&(a_pConnection->Mutex));
    }
#endif /* OPCUA_USE_SYNCHRONISATION */

    OpcUa_MemSet(a_pConnection, 0, sizeof(OpcUa_TcpListener_Connection));

    a_pConnection->iConnectionHandle = OPCUA_TCPLISTENER_CONNECTIONMANAGER_HANDLE_MAKE(usVersion, usId);
}

/*==============================================================================*/
/* Delete                                                                       */
/*==============================================================================*/
/**
* @brief Clean up and free the given OpcUa_TcpListener_Connection.
*/
//OpcUa_Void OpcUa_TcpListener_Connection_Delete(OpcUa_TcpListener_Connection** a_ppConnection)
//{   
//    if(a_ppConnection == OpcUa_Null || *a_ppConnection == OpcUa_Null)
//    {
//        return;
//    }
//
//	if(((*a_ppConnection)->Socket) != OpcUa_Null)
//	{
//		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_TcpListener_Connection_Delete: HANDLE LEAK!\n");
//	}
//
//    /* clean up internal resources */
//    OpcUa_TcpListener_Connection_Clear(*a_ppConnection);
//
//    /* free instance memory */
//    OpcUa_Free(*a_ppConnection);
//    a_ppConnection = OpcUa_Null;
//}

/*==============================================================================*/
/* Delete                                                                       */
/*==============================================================================*/
/**
* @brief .
*/
OpcUa_StatusCode OpcUa_TcpListener_ConnectionManager_GetConnectionCount(
    OpcUa_TcpListener_ConnectionManager*    a_pConnectionManager, 
    OpcUa_UInt32*                           a_pNoOfConnections)
{
OpcUa_InitializeStatus(OpcUa_Module_TcpListener, "GetConnectionCout");

    OpcUa_ReturnErrorIfArgumentNull(a_pConnectionManager);
    OpcUa_ReturnErrorIfArgumentNull(a_pNoOfConnections);

    OpcUa_Mutex_Lock(a_pConnectionManager->pMutex);
    *a_pNoOfConnections = a_pConnectionManager->uUsedConnections;
    OpcUa_Mutex_Unlock(a_pConnectionManager->pMutex);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    /* nothing */

OpcUa_FinishErrorHandling;
}

/*==============================================================================*/
/* Get Connection By Handle                                                     */
/*==============================================================================*/
/** 
 * @brief Retrieves a started stream for the connection identified by the socket. 
 *
 * @return StatusCode
 */
OpcUa_StatusCode OpcUa_TcpListener_ConnectionManager_GetConnectionByHandle(
    OpcUa_TcpListener_ConnectionManager*    a_pConnectionManager, 
    OpcUa_Handle                            a_hConnection, 
    OpcUa_TcpListener_Connection**          a_ppConnection)
{
    OpcUa_StatusCode                uStatus         = OpcUa_Good;
    OpcUa_Int                       iId             = 0;

    OpcUa_ReturnErrorIfArgumentNull(a_pConnectionManager);
    OpcUa_ReturnErrorIfArgumentNull(a_pConnectionManager->Connections);
    OpcUa_ReturnErrorIfArgumentNull(a_ppConnection);

    *a_ppConnection = OpcUa_Null;
    uStatus = OpcUa_BadNotFound;

    OpcUa_Mutex_Lock(a_pConnectionManager->pMutex);

    iId = (OpcUa_Int)OPCUA_TCPLISTENER_CONNECTIONMANAGER_HANDLE_GET_ID(((OpcUa_Int)a_hConnection));

    if(iId >= OPCUA_TCPLISTENER_MAXCONNECTIONS)
    {
        OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_TcpListener_ConnectionManager_GetConnectionByHandle: FATAL ERROR! Connection ID %i out of bounds!\n", iId);
    }
    else
    {
        OpcUa_UInt16    usVersion       = 0;
        OpcUa_UInt16    usVersionArg    = 0;

        *a_ppConnection = &a_pConnectionManager->Connections[iId];
        if(     (*a_ppConnection)->bUsed    == OpcUa_False
            ||  (*a_ppConnection)->bValid   == OpcUa_False)
        {
            OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_WARNING, "OpcUa_TcpListener_ConnectionManager_GetConnectionByHandle: Connection with id %i not usable!\n", iId);
            *a_ppConnection = OpcUa_Null;
        }
        else
        {
            usVersionArg = OPCUA_TCPLISTENER_CONNECTIONMANAGER_HANDLE_GET_VERSION(((OpcUa_Int)a_hConnection));
            usVersion    = OPCUA_TCPLISTENER_CONNECTIONMANAGER_HANDLE_GET_VERSION(((*a_ppConnection)->iConnectionHandle));
            if(usVersion != usVersionArg)
            {
                OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_WARNING, "OpcUa_TcpListener_ConnectionManager_GetConnectionByHandle: Version %u of connection ID %i does not match argument!\n", usVersion, iId, usVersionArg);
            }
            else
            {
                uStatus = OpcUa_Good;
            }
        }
    }

    OpcUa_Mutex_Unlock(a_pConnectionManager->pMutex);

    return uStatus;
}

#endif /* OPCUA_HAVE_SERVERAPI */

/*==============================================================================*/
/* End Of File                                                                  */
/*==============================================================================*/
