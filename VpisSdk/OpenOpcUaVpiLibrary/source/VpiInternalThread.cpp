//**************************************************************************
//
//  Copyright (c) 4CE Industry 1999-2014, All Rights Reserved
//
//**************************************************************************
//
//  Filename   :  VpiInternalThread.cpp
//  $Author    :  Michel Condemine
//
//  Description:  This file is part of the OpenOpcUaLibrary. 
//                This compoment and all related file are not Open Source
//				  This must be use with the autorisation of Michel Condemine
//**************************************************************************
/*
 *
 * Permission is hereby granted, for a commerciale use of this 
 * software and associated documentation files (the "Software")
 *
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 * ======================================================================*/
#include "stdafx.h"
/* base */
//#include <opcua.h>
#if defined(WIN32) || defined(_WIN32_WCE)
#include <windows.h>
#endif
#if VPI_MULTITHREADED
/* core */
#include <VpiMutex.h>
#include <VpiSemaphore.h>

/* self */
#include <VpiThread.h>
#ifdef _GNUC_
#include <pthread.h>
#endif	/* linux */

/* mappings to platform layer calltable. */
#if !VPI_USE_STATIC_PLATFORM_INTERFACE
#define Vpi_P_Thread_Create               Vpi_ProxyStub_g_PlatformLayerCalltable->ThreadCreate
#define Vpi_P_Thread_Delete               Vpi_ProxyStub_g_PlatformLayerCalltable->ThreadDelete
#define Vpi_P_Thread_Start                Vpi_ProxyStub_g_PlatformLayerCalltable->ThreadStart
#define Vpi_P_Thread_Sleep                Vpi_ProxyStub_g_PlatformLayerCalltable->ThreadSleep
#endif

typedef struct _Vpi_ThreadInternal Vpi_ThreadInternal;
struct _Vpi_ThreadInternal
{
	/** @brief The handle of the platform thread. */
	Vpi_RawThread      RawThread;
	/** @brief Mutex to synchronize access to the thread object. */
	Vpi_Mutex          Mutex;
	/** @brief Enables to wait for the termination of the thread. */
	Vpi_Semaphore      ShutdownSem;
	/** @brief Tells, if the thread is currently running. */
	Vpi_Boolean        IsRunning;
	/** @brief A stop flag. */
	/*Vpi_Boolean        MustStop;*/
	/** @brief The entry function of the thread. */
	Vpi_PfnThreadMain* ThreadMain;
	/** @brief The data which will be the argument to the thread function. */
	Vpi_Void*          ThreadData;
};
void* pthread_start(void* args)
{
#ifdef WIN32
	Vpi_ThreadArg*  pThreadArgs         = Vpi_Null;
	Vpi_Void*         pArguments          = Vpi_Null;

	if(args != Vpi_Null)
	{    
		pThreadArgs = (Vpi_ThreadArg*)args;

		pArguments  = pThreadArgs->ThreadArgs;

		/* run stack thread! */
		pThreadArgs->pfnInternalThreadMain(pArguments);

		ExitThread(0);

	}
	else
		return Vpi_Null;
#endif
#ifdef _GNUC_
	Vpi_ThreadArg*    p_ThreadArgs      = Vpi_Null;
	Vpi_Thread       pThread             = Vpi_Null; // Attention confusion ptr a verifier
	pthread_t*          pInternalThread     = Vpi_Null;
	int                 apiResult           = 0;


	if(args == Vpi_Null)
	{
		return Vpi_Null;
	}

	p_ThreadArgs = (Vpi_ThreadArg*)args;

	pInternalThread = (pthread_t*)(p_ThreadArgs->hThread);
	pThread         = p_ThreadArgs->ThreadArgs;

	if(pInternalThread == Vpi_Null)
	{
		return Vpi_Null;
	}

	if(apiResult != 0)
	{
		return Vpi_Null;
	}

	/* run stack thread! */
	p_ThreadArgs->pfnInternalThreadMain(pThread);

	free(p_ThreadArgs);

	pthread_exit(NULL);

	return Vpi_Null;
#endif
}
/*============================================================================
 * The internal main entry function.
 *===========================================================================*/
Vpi_Void InternalThreadMain(Vpi_Void* a_Thread)
{
	Vpi_ThreadInternal* Thread = (Vpi_ThreadInternal*)a_Thread;

	if(Thread == Vpi_Null)
	{
		return;
	}

	/* call the user function */
	Thread->ThreadMain(Thread->ThreadData);

	Vpi_Mutex_Lock(Thread->Mutex);
	Thread->IsRunning = Vpi_False;  
	Vpi_Semaphore_Post(Thread->ShutdownSem, 1);
	Vpi_Mutex_Unlock(Thread->Mutex);
}

/*============================================================================
 * Create
 *===========================================================================*/
/// <summary>
/// Opcs the ua_ vpi_ thread_ create.
/// </summary>
/// <param name="a_pThread">The internal pointer to the thread .</param>
/// <param name="a_pThreadMain">The entry function of the thread.</param>
/// <param name="a_pThreadArgument">The data which will be the argument to the thread function.</param>
/// <returns></returns>
Vpi_StatusCode Vpi_Thread_Create(Vpi_Thread*        a_pThread,
									  Vpi_PfnThreadMain*    a_pThreadMain,
									  Vpi_Void*             a_pThreadArgument)
{
	Vpi_StatusCode        uStatus = Vpi_Good;
	Vpi_ThreadInternal*   pThreadInternal = Vpi_Null;

	if (a_pThread)
	{
		if (a_pThreadMain)
		{
			pThreadInternal = (Vpi_ThreadInternal*)malloc(sizeof(Vpi_ThreadInternal));

			if (pThreadInternal)
			{		    
				Vpi_MemSet(pThreadInternal, 0, sizeof(Vpi_ThreadInternal));

				pThreadInternal->IsRunning = Vpi_False;
				pThreadInternal->ThreadMain = a_pThreadMain;
				pThreadInternal->ThreadData = a_pThreadArgument;
				pThreadInternal->Mutex = Vpi_Null;
				pThreadInternal->ShutdownSem = Vpi_Null;
				// This is the argument of the thread object
				Vpi_ThreadArg* pThreadArgs = Vpi_Null;
				pThreadArgs = (Vpi_ThreadArg*)malloc(sizeof(Vpi_ThreadArg));
				if (pThreadArgs)
				{
					pThreadArgs->hThread                = (Vpi_Handle)INVALID_HANDLE_VALUE;
					pThreadArgs->pfnInternalThreadMain  = Vpi_Null;
					pThreadArgs->ThreadArgs             = Vpi_Null;
					
					uStatus = Vpi_Semaphore_Create(&(pThreadInternal->ShutdownSem),
														1,  /* the initial value is 1 (signalled, 1 free resource) */
														1); /* the maximum value is 1 */
					if (uStatus==Vpi_Good)
						uStatus = Vpi_Mutex_Create(&(pThreadInternal->Mutex));
					pThreadInternal->RawThread = (Vpi_RawThread*)pThreadArgs;
					*a_pThread = pThreadInternal;
				}
				else
					uStatus=Vpi_BadOutOfMemory;
			}
			else
				uStatus=Vpi_BadOutOfMemory;
		}
		else
			uStatus=Vpi_BadInvalidArgument;
	}
	else
		uStatus=Vpi_BadInvalidArgument;
	return uStatus;
}


/*============================================================================
 * Delete
 *===========================================================================*/
Vpi_Void Vpi_Thread_Delete(Vpi_Thread a_Thread)
{
	Vpi_ThreadInternal*   pThread = Vpi_Null;

	if (a_Thread)
	{
		pThread = (Vpi_ThreadInternal*)a_Thread;
		if (pThread->ShutdownSem)
			Vpi_Semaphore_Delete(&(pThread->ShutdownSem));
		if (pThread->IsRunning == Vpi_False)
		{
			if (pThread->RawThread)
			{
				Vpi_ThreadArg* pThreadArgs = (Vpi_ThreadArg*)pThread->RawThread;
				if (((Vpi_Handle)INVALID_HANDLE_VALUE) != pThreadArgs->hThread)
				{
#ifdef WIN32
					CloseHandle(pThreadArgs->hThread);
#endif
					pThreadArgs->hThread = (void*)INVALID_HANDLE_VALUE;
				}
				free(pThreadArgs);
				pThreadArgs = (Vpi_ThreadArg *)Vpi_Null;
			}

			if (pThread->Mutex)
				Vpi_Mutex_Delete(&(pThread->Mutex));
			
			free(pThread);
			a_Thread = Vpi_Null;
			return;
		}
		pThread->IsRunning = Vpi_False;
		if (pThread->RawThread)
		{
#ifdef WIN32
			Vpi_ThreadArg* pThreadArgs = (Vpi_ThreadArg*)pThread->RawThread;

			if (INVALID_HANDLE_VALUE != pThreadArgs->hThread)
			{
				WaitForSingleObject(pThreadArgs->hThread, INFINITE);
				CloseHandle(pThreadArgs->hThread);
				pThreadArgs->hThread = INVALID_HANDLE_VALUE;
			}

			pThreadArgs->pfnInternalThreadMain = Vpi_Null;
			pThreadArgs->ThreadArgs = Vpi_Null;

			free(pThreadArgs);
			pThreadArgs = Vpi_Null;
#endif
#ifdef _GNUC_
			Vpi_MemSet((Vpi_Void*)(a_Thread), 0, sizeof(pthread_t));
#endif
		}

		if (pThread->Mutex)
			Vpi_Mutex_Delete(&(pThread->Mutex));

		pThread->ThreadData = Vpi_Null;
		pThread->ThreadMain = Vpi_Null;


		free(a_Thread);

		a_Thread = Vpi_Null;
	}
	return;
}


/*============================================================================
 * Start a created thread.
 *===========================================================================*/
Vpi_StatusCode Vpi_Thread_Start(Vpi_Thread a_Thread)
{
	Vpi_StatusCode        uStatus     = Vpi_Good;
	Vpi_ThreadInternal*   pThread     = Vpi_Null;

	if (a_Thread)
	{
		pThread = (Vpi_ThreadInternal*)a_Thread;

		Vpi_Mutex_Lock(pThread->Mutex);
		if(pThread->IsRunning != Vpi_False)
			uStatus = Vpi_Good;
		else
		{
			/* set semaphore to waitable */
			uStatus = Vpi_Semaphore_Wait((pThread->ShutdownSem));
			if (uStatus==Vpi_Good)
			{
				pThread->IsRunning = Vpi_True;

				uStatus = Vpi_Internal_Thread_Start(pThread->RawThread,
													InternalThreadMain, 
													(Vpi_Void*)pThread);

				if (uStatus != Vpi_Good)
				{
					pThread->IsRunning = Vpi_False;
					uStatus = Vpi_BadInternalError;
					Vpi_Trace(Vpi_Null, VPI_TRACE_EXTRA_LEVEL_ERROR, "Vpi_Thread_Start: Error during thread creation!\n");
				}
			}
		}
		Vpi_Mutex_Unlock(pThread->Mutex);

	}
	else
		uStatus=Vpi_BadInvalidArgument;
	

	return uStatus;
}
/*============================================================================
 * start the internal thread .
 *===========================================================================*/
Vpi_StatusCode Vpi_Internal_Thread_Start(Vpi_RawThread             pThread,
										   Vpi_PfnInternalThreadMain pfnStartFunction,
										   Vpi_Void*                 pArguments)
{
	Vpi_StatusCode uStatus=Vpi_Good;
#ifdef WIN32
	HANDLE threadHandle = 0;

	if(pThread == Vpi_Null)
	{
		return Vpi_BadInvalidArgument;
	}

	((Vpi_ThreadArg*)pThread)->pfnInternalThreadMain    = pfnStartFunction;
	((Vpi_ThreadArg*)pThread)->ThreadArgs               = pArguments;

	threadHandle = CreateThread(    NULL,
									0,
									(LPTHREAD_START_ROUTINE)pthread_start,
									pThread,
									0,
									NULL);

	if(threadHandle == NULL)
	{
		return Vpi_BadResourceUnavailable;
	}
	else
	{
		((Vpi_ThreadArg*)pThread)->hThread = threadHandle;
	}
#endif
#ifdef _GNUC_
	Vpi_Int32         apiResult = 0;
	Vpi_ThreadArg*  pThreadArguments = Vpi_Null;

	if(pThread == Vpi_Null)
	{
		return Vpi_BadInvalidArgument;
	}

	pthread_attr_t threadAttr;

	// initialize the thread attribute
	pthread_attr_init(&threadAttr);

	// Set the stack size of the thread
	pthread_attr_setstacksize(&threadAttr, 120*1024);

	// Set thread to detached state. No need for pthread_join
	pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_DETACHED);

	if(pThread == Vpi_Null)
	{
		return Vpi_BadInvalidArgument;
	}

	pThreadArguments = (Vpi_ThreadArg*)malloc(sizeof(Vpi_ThreadArg));
	memset(pThreadArguments, 0, sizeof(Vpi_ThreadArg));

	pThreadArguments->hThread               = pThread;
	pThreadArguments->pfnInternalThreadMain = pfnStartFunction;
	pThreadArguments->ThreadArgs            = pArguments;

	apiResult = pthread_create((pthread_t*)(pThread), &threadAttr, pthread_start, pThreadArguments);

	// Destroy the thread attributes
	pthread_attr_destroy(&threadAttr);

	switch(apiResult)
	{
		case EAGAIN:
		{
			return Vpi_BadResourceUnavailable;
		}
		case 0:
		{
			return Vpi_Good;
		}
		default:
		{
			return Vpi_BadInternalError;
		}
	}
#endif
	return uStatus;
}

/*============================================================================
 * Wait for a thread to shutdown.
 *===========================================================================*/
/* conversion is done internally, the parameter must be filled with a value representing milliseconds */
Vpi_StatusCode Vpi_Thread_WaitForShutdown(  Vpi_Thread a_Thread, 
												Vpi_UInt32 a_msecTimeout)
{
	Vpi_StatusCode        uStatus = Vpi_Good;
	Vpi_ThreadInternal*   Thread  = Vpi_Null;
	
	Thread = (Vpi_ThreadInternal*)a_Thread;

	if (Thread)
	{
		Vpi_Mutex_Lock(Thread->Mutex);
		if(Thread->IsRunning == Vpi_False)
		{
			/* printf("wait for shutdown: thread is not running\n");*/
			Vpi_Mutex_Unlock(Thread->Mutex);
			return Vpi_Good;
		}
		Vpi_Mutex_Unlock(Thread->Mutex);

		uStatus = Vpi_Semaphore_TimedWait(Thread->ShutdownSem, a_msecTimeout);
		if(uStatus==Vpi_Bad)
		{
			return uStatus;
		}

		Vpi_Mutex_Lock(Thread->Mutex);
		if(Thread->IsRunning == Vpi_False)
		{
			/* Release the semaphore again to enable other threads waiting on it to get unlocked. */
			uStatus = Vpi_Semaphore_Post(   Thread->ShutdownSem,  
												1);
			/*printf("wait for shutdown: thread stopped\n");*/
			Vpi_Mutex_Unlock(Thread->Mutex);
			return Vpi_Good;
		}

		Vpi_Mutex_Unlock(Thread->Mutex);

		/* printf("wait for shutdown: thread is still running\n");*/
		uStatus=Vpi_GoodNonCriticalTimeout;
	}
	else
		uStatus=Vpi_BadInvalidArgument;
	return uStatus;;
}

/*============================================================================
 * Let the thread sleep for a certian amount of time
 *===========================================================================*/
Vpi_Void Vpi_Thread_Sleep(Vpi_UInt32 a_msecTimeout)
{
#ifdef WIN32
	Sleep(a_msecTimeout);
#endif
#ifdef _GNUC_
	usleep(a_msecTimeout*1000);
#endif
}

/*============================================================================
 * Retrieve the id of the active thread.
 *===========================================================================*/
Vpi_UInt32 Vpi_Thread_GetCurrentThreadId()
{
#ifdef WIN32
	return (Vpi_UInt32)GetCurrentThreadId();
#endif
#ifdef _GNUC_
	return (Vpi_UInt32)pthread_self();
#endif
	//return (Vpi_UInt32)Vpi_P_Thread_GetCurrentThreadId();
}

/*============================================================================
 * Check if the main function of the given thread object is running.
 *===========================================================================*/
Vpi_Boolean Vpi_Thread_IsRunning(Vpi_Thread a_hThread)
{
	Vpi_ThreadInternal* pThread = (Vpi_ThreadInternal*)a_hThread;
	Vpi_Boolean bTemp = Vpi_False;
	
	if(Vpi_Null == pThread)
	{
		return Vpi_False;
	}

	Vpi_Mutex_Lock(pThread->Mutex);

	bTemp = pThread->IsRunning;

	Vpi_Mutex_Unlock(pThread->Mutex);

	return bTemp;
}

#endif /* VPI_MULTITHREADED */
