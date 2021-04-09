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

/* core */
#include <opcua.h>

#ifdef OPCUA_HAVE_CLIENTAPI

#include <opcua_semaphore.h>
#include <opcua_mutex.h>
#include <opcua_utilities.h>

/* types */
#include <opcua_builtintypes.h>
#include <opcua_encodeableobject.h>
#include <opcua_types.h>

/* communication */
#include <opcua_connection.h>

/* client api */
#include <opcua_channel.h>

/* self */
#include <opcua_asynccallstate.h>

/*============================================================================
 * OpcUa_AsyncCallState_Create
 *===========================================================================*/
OpcUa_StatusCode OpcUa_AsyncCallState_Create(
    OpcUa_Void*             a_hChannel,
    OpcUa_Void*             a_pRequestData,
    OpcUa_EncodeableType*   a_pRequestType,
    OpcUa_AsyncCallState**  a_ppAsyncState)
{
    OpcUa_AsyncCallState* pAsyncState = OpcUa_Null;

OpcUa_InitializeStatus(OpcUa_Module_AsyncCallState, "Create");
    OpcUa_ReturnErrorIfArgumentNull(a_hChannel);
    OpcUa_ReturnErrorIfArgumentNull(a_ppAsyncState);

    *a_ppAsyncState = OpcUa_Null;

    pAsyncState = (OpcUa_AsyncCallState*)OpcUa_Alloc(sizeof(OpcUa_AsyncCallState));
    OpcUa_GotoErrorIfAllocFailed(pAsyncState);
    OpcUa_MemSet(pAsyncState, 0, sizeof(OpcUa_AsyncCallState));

    pAsyncState->Channel       = a_hChannel;
    pAsyncState->RequestData   = a_pRequestData;
    pAsyncState->RequestType   = a_pRequestType;
    pAsyncState->Status        = OpcUa_BadWaitingForResponse;

#if OPCUA_USE_SYNCHRONISATION
    pAsyncState->WaitMutex     = OpcUa_Null;
    pAsyncState->WaitCondition = OpcUa_Null;

    uStatus = OpcUa_Mutex_Create(&(pAsyncState->WaitMutex));
    OpcUa_GotoErrorIfBad(uStatus);

    /* create binary semaphore */
    uStatus = OpcUa_Semaphore_Create(&pAsyncState->WaitCondition, 0, 1);
    OpcUa_GotoErrorIfBad(uStatus);
#endif /* OPCUA_USE_SYNCHRONISATION */
    *a_ppAsyncState = pAsyncState;

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    OpcUa_AsyncCallState_Delete(&pAsyncState);

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_AsyncCallState_Delete
 *===========================================================================*/
OpcUa_Void OpcUa_AsyncCallState_Delete(OpcUa_AsyncCallState** a_ppAsyncState)
{
    if (a_ppAsyncState != OpcUa_Null && *a_ppAsyncState != OpcUa_Null)
    {
        OpcUa_AsyncCallState* pAsyncState = *a_ppAsyncState;

#if OPCUA_USE_SYNCHRONISATION
        OpcUa_Mutex_Delete(&(pAsyncState->WaitMutex));
        OpcUa_Semaphore_Delete(&(pAsyncState->WaitCondition));
#endif /* OPCUA_USE_SYNCHRONISATION */

        OpcUa_Free(pAsyncState);

        *a_ppAsyncState = OpcUa_Null;
    }
}

/*============================================================================
 * OpcUa_AsyncCallState_WaitForCompletion
 *===========================================================================*/
#if OPCUA_USE_SYNCHRONISATION
OpcUa_StatusCode OpcUa_AsyncCallState_WaitForCompletion(
    OpcUa_AsyncCallState* a_pAsyncState,
    OpcUa_UInt32          a_uTimeout)
{
    OpcUa_UInt32    uElapsedTime        = 0;
    OpcUa_Boolean   bTimeoutOccurred    = OpcUa_False;

OpcUa_InitializeStatus(OpcUa_Module_AsyncCallState, "WaitForCompletion");


    OpcUa_ReturnErrorIfArgumentNull(a_pAsyncState);

    /* lock request mutex */
    OpcUa_Mutex_Lock(a_pAsyncState->WaitMutex);

    /* block until call completes */
    while (a_pAsyncState->Status == OpcUa_BadWaitingForResponse)
    {
        OpcUa_UInt32 uActualTimeout = OPCUA_INFINITE;
        OpcUa_UInt32 uStartCount = OpcUa_GetTickCount();
        OpcUa_UInt32 uEndCount = uStartCount;

        /* calculate the time left in the requested timeout period */
        if (a_uTimeout > 0)
        {
            if (a_uTimeout < uElapsedTime)
            {
                bTimeoutOccurred = OpcUa_True;
                a_pAsyncState->Status = OpcUa_BadOperationAbandoned;
                break;
            }
                
            uActualTimeout = a_uTimeout - uElapsedTime;
        }

        /* block until timeout or response arrives */
        if (uActualTimeout > 0 || (uActualTimeout == 0 && a_uTimeout == 0))
        {
            OpcUa_Mutex_Unlock(a_pAsyncState->WaitMutex);
            uStatus = OpcUa_Semaphore_TimedWait(a_pAsyncState->WaitCondition, uActualTimeout); 


            if(OpcUa_IsEqual(OpcUa_GoodNonCriticalTimeout))
            {
                bTimeoutOccurred = OpcUa_True;
            }
            else if(OpcUa_IsBad(uStatus))
            {
                break;
            }
            OpcUa_Mutex_Lock(a_pAsyncState->WaitMutex);
        }

        /* abandon request if timeout expired */
        if (bTimeoutOccurred)
        {
            a_pAsyncState->Status = OpcUa_BadOperationAbandoned;
            break;
        }

        /* calculate the elapsed time */
        uEndCount = OpcUa_GetTickCount();

        if (uEndCount < uStartCount)
        {
            uElapsedTime += (uEndCount + (OpcUa_UInt32_Max - uStartCount));
        }
        else
        {
            uElapsedTime += (uEndCount - uStartCount);
        }
    }
    

    if(OpcUa_IsBad(uStatus))
    {
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_AsyncCallState_WaitForCompletion: OpcUa_Semaphore_TimedWait returned error 0x%08X!\n", uStatus);
    }
    else
    {
        /* release request mutex */
        OpcUa_Mutex_Unlock(a_pAsyncState->WaitMutex);

        uStatus = a_pAsyncState->Status;

        /* do not free async state object if timeout occurred (callback will happen eventually). */
        if(OpcUa_False != bTimeoutOccurred)
        {
            uStatus = OpcUa_BadTimeout;
        }
    }

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_AsyncCallState_SignalCompletion
 *===========================================================================*/
OpcUa_StatusCode OpcUa_AsyncCallState_SignalCompletion(
    OpcUa_AsyncCallState* a_pAsyncState,
    OpcUa_StatusCode      a_uOperationStatus)
{   
OpcUa_InitializeStatus(OpcUa_Module_AsyncCallState, "SignalCompletion");

    OpcUa_ReturnErrorIfArgumentNull(a_pAsyncState);

    /* lock request mutex */
    OpcUa_Mutex_Lock(a_pAsyncState->WaitMutex);

    /* set the return value for the waiter on this status */
    a_pAsyncState->Status = a_uOperationStatus;

    /* signal that the request has completed */
    uStatus = OpcUa_Semaphore_Post(a_pAsyncState->WaitCondition, 1);
    OpcUa_GotoErrorIfBad(uStatus);
    
    /* release the wait mutex */
    OpcUa_Mutex_Unlock(a_pAsyncState->WaitMutex);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    /* release the wait mutex */
    OpcUa_Mutex_Unlock(a_pAsyncState->WaitMutex);

OpcUa_FinishErrorHandling;
}
#endif /* OPCUA_USE_SYNCHRONISATION */

#endif /* OPCUA_HAVE_CLIENTAPI */
