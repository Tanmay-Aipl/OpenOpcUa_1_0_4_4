//**************************************************************************
//
//  Copyright (c) 4CE Industry 1999-2014, All Rights Reserved
//
//**************************************************************************
//
//  Filename   :  VpiInternalThread.h
//  $Author    :  Michel Condemine
//
//  Description:  This file is part of the OpenOpcUaLibrary. 
//                This compoment and all related file are not Open Source
//				  This must be use with the autorisation of Michel Condemine
//**************************************************************************

#ifndef _Vpi_Thread_H_
#define _Vpi_Thread_H_ 1
typedef Vpi_Void* Vpi_RawThread;

typedef struct _Vpi_ThreadArg
{
	Vpi_Handle                 hThread;
	Vpi_PfnInternalThreadMain* pfnInternalThreadMain;
	Vpi_Void*                  ThreadArgs;
} Vpi_ThreadArg;
VPI_BEGIN_EXTERN_C

/*============================================================================
 * Type Definition
 *===========================================================================*/

/**
 * @brief Thread main entry function definition.
 */
 typedef Vpi_Void (Vpi_PfnThreadMain)(Vpi_Void* pArgument);

/**
 * @brief Describes a thread handle.
 */

/*============================================================================
 * Type Management
 *===========================================================================*/

/**
 * @brief Create a thread.
 *
 * @param ppThread [in/out] Pointer to the thread handle. Contains the created thread or Vpi_Null.
 *
 * @return An error code for the operation.
 */
VPILIBRARY_EXPORT 
Vpi_StatusCode    Vpi_Thread_Create(Vpi_Thread*        pThread,
									  Vpi_PfnThreadMain* pThreadMain,
									  Vpi_Void*          pThreadArgument);

/**
 * @brief Delete a thread.
 *
 * @param ppThread [in] Pointer to the thread handle.
 *
 * @return
 */
VPILIBRARY_EXPORT 
Vpi_Void	Vpi_Thread_Delete(Vpi_Thread pThread);


/*============================================================================
 * Type Operations
 *===========================================================================*/

/**
 * @brief Start a Thread.
 *
 * @param Thread [in] The thread handle.
 *
 * @return An error code for the operation.
 */
VPILIBRARY_EXPORT 
Vpi_StatusCode Vpi_Thread_Start(            Vpi_Thread   Thread);
// Internal start thread function
Vpi_StatusCode Vpi_Internal_Thread_Start(Vpi_RawThread             pThread,
										   Vpi_PfnInternalThreadMain pfnStartFunction,
										   Vpi_Void*                 pArguments);
/**
 * @brief Wait For Thread Shutdown.
 *
 * @param Thread        [in] The thread handle.
 * @param msecTimeout   [in] The maximum time to wait for shutdown.
 *
 * @return An error code for the operation.
 */
VPILIBRARY_EXPORT 
Vpi_StatusCode Vpi_Thread_WaitForShutdown(  Vpi_Thread   Thread, 
												Vpi_UInt32    msecTimeout);


/**
 * @brief Lets the thread sleep for a certain amount of time.
 *
 * @param msecTimeout [in] The time in milliseconds to suspend the calling thread.
 */
VPILIBRARY_EXPORT 
Vpi_Void Vpi_Thread_Sleep(Vpi_UInt32    msecTimeout);

/**
 * @brief Get the ID of the current thread.
 *
 * @return The thread ID.
 */
VPILIBRARY_EXPORT Vpi_UInt32 Vpi_Thread_GetCurrentThreadId();

/**
 * @brief Check if the main function of the given thread object is running.
 *        State may have already changed when function returns!
 *
 * @return Vpi_True if running, Vpi_False else.
 */
VPILIBRARY_EXPORT
Vpi_Boolean Vpi_Thread_IsRunning(Vpi_Thread    hThread);

VPI_END_EXTERN_C

#endif /* _Vpi_Thread_H_ */
