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

/* base */
#include <opcua.h>
#if defined(WIN32) || defined(_WIN32_WCE)
#include <windows.h>
#endif
#if OPCUA_MULTITHREADED

/* core */
#include <opcua_mutex.h>
#include <opcua_semaphore.h>

/* self */
#include <opcua_thread.h>
#ifdef _GNUC_
#include <pthread.h>
#endif	/* linux */

/* mappings to platform layer calltable. */
#if !OPCUA_USE_STATIC_PLATFORM_INTERFACE
#define OpcUa_P_Thread_Create               OpcUa_ProxyStub_g_PlatformLayerCalltable->ThreadCreate
#define OpcUa_P_Thread_Delete               OpcUa_ProxyStub_g_PlatformLayerCalltable->ThreadDelete
#define OpcUa_P_Thread_Start                OpcUa_ProxyStub_g_PlatformLayerCalltable->ThreadStart
#define OpcUa_P_Thread_Sleep                OpcUa_ProxyStub_g_PlatformLayerCalltable->ThreadSleep
//#define OpcUa_P_Thread_GetCurrentThreadId   OpcUa_ProxyStub_g_PlatformLayerCalltable->ThreadGetCurrentId
#endif

typedef struct _OpcUa_ThreadInternal OpcUa_ThreadInternal;
struct _OpcUa_ThreadInternal
{
	/** @brief The handle of the platform thread. */
	OpcUa_RawThread      RawThread;
	/** @brief Mutex to synchronize access to the thread object. */
	OpcUa_Mutex          Mutex;
	/** @brief Enables to wait for the termination of the thread. */
	OpcUa_Semaphore      ShutdownEvent;
	/** @brief Tells, if the thread is currently running. */
	OpcUa_Boolean        IsRunning;
	/** @brief A stop flag. */
	/*OpcUa_Boolean        MustStop;*/
	/** @brief The entry function of the thread. */
	OpcUa_PfnThreadMain* ThreadMain;
	/** @brief The data which will be the argument to the thread function. */
	OpcUa_Void*          ThreadData;
};

/*============================================================================
 * The internal main entry function.
 *===========================================================================*/
OpcUa_Void InternalThreadMain(OpcUa_Void* a_Thread)
{
	OpcUa_ThreadInternal* Thread = (OpcUa_ThreadInternal*)a_Thread;

	if(Thread == OpcUa_Null)
	{
		return;
	}

	/* call the user function */
	Thread->ThreadMain(Thread->ThreadData);

	if (Thread->Mutex)
	{
		OpcUa_Mutex_Lock(Thread->Mutex);
		Thread->IsRunning = OpcUa_False;
		if (Thread->ShutdownEvent)
			OpcUa_Semaphore_Post(Thread->ShutdownEvent, 1);
		OpcUa_Mutex_Unlock(Thread->Mutex);
	}
	else
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR,"Critical error with internal Thread Management from opcua_thread.cpp\n");
}

/*============================================================================
 * Create
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Thread_Create(   OpcUa_Thread*           a_pThread,
										OpcUa_PfnThreadMain*    a_pThreadMain,
										OpcUa_Void*             a_pThreadArgument)
{
	OpcUa_StatusCode        uStatus = OpcUa_Good;
	OpcUa_ThreadInternal*   pThread = OpcUa_Null;

	OpcUa_DeclareErrorTraceModule(OpcUa_Module_Thread);

	OpcUa_ReturnErrorIfArgumentNull(a_pThread);
	OpcUa_ReturnErrorIfArgumentNull(a_pThreadMain);

	pThread = (OpcUa_ThreadInternal*)OpcUa_Alloc(sizeof(OpcUa_ThreadInternal));

	OpcUa_ReturnErrorIfAllocFailed(pThread);
	
	OpcUa_MemSet(pThread, 0, sizeof(OpcUa_ThreadInternal));

	pThread->IsRunning      = OpcUa_False;
	pThread->ThreadMain     = a_pThreadMain;
	pThread->ThreadData     = a_pThreadArgument;
	pThread->Mutex          = OpcUa_Null;
	pThread->ShutdownEvent  = OpcUa_Null;

	uStatus = OpcUa_P_Thread_Create(&(pThread->RawThread));
	OpcUa_GotoErrorIfBad(uStatus);

	uStatus = OpcUa_Semaphore_Create( &(pThread->ShutdownEvent), 
										1,  /* the initial value is 1 (signalled, 1 free resource) */
										1); /* the maximum value is 1 */
	OpcUa_GotoErrorIfBad(uStatus);

	uStatus = OpcUa_Mutex_Create(&(pThread->Mutex));
	OpcUa_GotoErrorIfBad(uStatus);

	*a_pThread = pThread;

	return OpcUa_Good;

Error:

	return uStatus;
}


/*============================================================================
 * Delete
 *===========================================================================*/
OpcUa_Void OpcUa_Thread_Delete(OpcUa_Thread a_Thread)
{
	OpcUa_ThreadInternal* pThread = OpcUa_Null;

	if (a_Thread != OpcUa_Null)
	{
		pThread = (OpcUa_ThreadInternal*)a_Thread;

		//if (pThread->IsRunning == OpcUa_False)
		//	return;

		if (pThread->ShutdownEvent)
			OpcUa_Semaphore_Delete(&(pThread->ShutdownEvent));
		// Explicitly stop the thread
		pThread->IsRunning = OpcUa_False;
		if (pThread->RawThread)
			OpcUa_P_Thread_Delete(&(pThread->RawThread));


		if (pThread->Mutex)
			OpcUa_Mutex_Delete(&(pThread->Mutex));

		pThread->ThreadData = OpcUa_Null;
		pThread->ThreadMain = OpcUa_Null;


		//OpcUa_Free(a_Thread);
		OpcUa_Free(pThread);

		a_Thread = OpcUa_Null;
	}
	return;
}


/*============================================================================
 * Start a created thread.
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Thread_Start(OpcUa_Thread a_Thread)
{
	OpcUa_StatusCode        uStatus     = OpcUa_Good;
	OpcUa_ThreadInternal*   pThread     = OpcUa_Null;
	OpcUa_Int32             intThreadId = 0;

	OpcUa_ReturnErrorIfArgumentNull(a_Thread);

	pThread = (OpcUa_ThreadInternal*)a_Thread;

	OpcUa_Mutex_Lock(pThread->Mutex);
	if(pThread->IsRunning != OpcUa_False)
	{
		OpcUa_Mutex_Unlock(pThread->Mutex);
		return OpcUa_Good;
	}

	/* set semaphore to waitable */
	uStatus = OpcUa_Semaphore_Wait((pThread->ShutdownEvent));
	OpcUa_GotoErrorIfBad(uStatus);

	pThread->IsRunning = OpcUa_True;

	intThreadId = OpcUa_P_Thread_Start( pThread->RawThread, 
										InternalThreadMain, 
										(OpcUa_Void*)pThread);

	if(intThreadId != 0)
	{
		pThread->IsRunning = OpcUa_False;

		OpcUa_Mutex_Unlock(pThread->Mutex);
		uStatus = OpcUa_BadInternalError;
		OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_Thread_Start: Error during thread creation!\n");
		goto Error;
	}
	OpcUa_Mutex_Unlock(pThread->Mutex);

	return OpcUa_Good;

Error:

	return uStatus;
}

/*============================================================================
 * Wait for a thread to shutdown.
 *===========================================================================*/
/* conversion is done internally, the parameter must be filled with a value representing milliseconds */
OpcUa_StatusCode OpcUa_Thread_WaitForShutdown(  OpcUa_Thread a_Thread, 
												OpcUa_UInt32 a_msecTimeout)
{
	OpcUa_StatusCode        uStatus = OpcUa_Good;
	OpcUa_ThreadInternal*   Thread  = OpcUa_Null;
	OpcUa_DeclareErrorTraceModule(OpcUa_Module_Thread);

	Thread = (OpcUa_ThreadInternal*)a_Thread;

	OpcUa_ReturnErrorIfArgumentNull(Thread);

	OpcUa_Mutex_Lock(Thread->Mutex);
	if(Thread->IsRunning == OpcUa_False)
	{
		/* printf("wait for shutdown: thread is not running\n");*/
		OpcUa_Mutex_Unlock(Thread->Mutex);
		return OpcUa_Good;
	}
	OpcUa_Mutex_Unlock(Thread->Mutex);

	uStatus = OpcUa_Semaphore_TimedWait(Thread->ShutdownEvent, a_msecTimeout);
	if(OpcUa_IsBad(uStatus))
	{
		return uStatus;
	}

	OpcUa_Mutex_Lock(Thread->Mutex);
	if(Thread->IsRunning == OpcUa_False)
	{
		/* Release the semaphore again to enable other threads waiting on it to get unlocked. */
		uStatus = OpcUa_Semaphore_Post(   Thread->ShutdownEvent,  
											1);
		/*printf("wait for shutdown: thread stopped\n");*/
		OpcUa_Mutex_Unlock(Thread->Mutex);
		return OpcUa_Good;
	}

	OpcUa_Mutex_Unlock(Thread->Mutex);

	/* printf("wait for shutdown: thread is still running\n");*/

	return OpcUa_GoodNonCriticalTimeout;
}

/*============================================================================
 * Let the thread sleep for a certian amount of time
 *===========================================================================*/
OpcUa_Void OpcUa_Thread_Sleep(OpcUa_UInt32 a_msecTimeout)
{
	OpcUa_P_Thread_Sleep(a_msecTimeout);
}

/*============================================================================
 * Retrieve the id of the active thread.
 *===========================================================================*/
OpcUa_UInt32 OpcUa_Thread_GetCurrentThreadId(OpcUa_Void)
{
#ifdef WIN32
	return (OpcUa_UInt32)GetCurrentThreadId();
#endif
#ifdef _GNUC_
	return (OpcUa_UInt32)pthread_self();
#endif
	//return (OpcUa_UInt32)OpcUa_P_Thread_GetCurrentThreadId();
}

/*============================================================================
 * Check if the main function of the given thread object is running.
 *===========================================================================*/
OpcUa_Boolean OpcUa_Thread_IsRunning(OpcUa_Thread a_hThread)
{
	OpcUa_ThreadInternal* pThread = (OpcUa_ThreadInternal*)a_hThread;
	OpcUa_Boolean bTemp = OpcUa_False;
	
	if(OpcUa_Null == pThread)
	{
		return OpcUa_False;
	}

	OpcUa_Mutex_Lock(pThread->Mutex);

	bTemp = pThread->IsRunning;

	OpcUa_Mutex_Unlock(pThread->Mutex);

	return bTemp;
}

#endif /* OPCUA_MULTITHREADED */
