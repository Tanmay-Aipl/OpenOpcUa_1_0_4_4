//**************************************************************************
//
//  Copyright (c) 4CE Industry 1999-2014, All Rights Reserved
//
//**************************************************************************
//
//  Filename   :  VpiMutex.cpp
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

Vpi_StatusCode VPI_DLLCALL Vpi_Mutex_Create(Vpi_Mutex* phNewMutex)
{
	Vpi_StatusCode uStatus=Vpi_Good;
#ifdef WIN32

	LPCRITICAL_SECTION lpCriticalSection = NULL;

	if(phNewMutex)
	{        
		if((*phNewMutex) != Vpi_Null)
		{
			*phNewMutex = Vpi_Null;
		}
		
	#if VPI_MUTEX_USE_SPINCOUNT
		BOOL bRet;
		LONG hr;
	#endif

	#if 0
		lpCriticalSection = Vpi_P_Memory_Alloc(sizeof(CRITICAL_SECTION));
	#else
		lpCriticalSection = (LPCRITICAL_SECTION)malloc(sizeof(CRITICAL_SECTION));
		if (lpCriticalSection)
			memset(lpCriticalSection, 0, sizeof(CRITICAL_SECTION));
	#endif

		if(lpCriticalSection)
		{

		#if VPI_MUTEX_USE_SPINCOUNT
			bRet = InitializeCriticalSectionAndSpinCount(   lpCriticalSection,
															0x00000400) ;
			if (bRet == 0)
			{
				//hr = GetLastError();
				free(lpCriticalSection);
				lpCriticalSection = Vpi_Null;
				uStatus= Vpi_Bad;
			}
			else
			{
		#else
			InitializeCriticalSection(lpCriticalSection);
		#endif

			*phNewMutex = (Vpi_Mutex)lpCriticalSection;
		#if VPI_MUTEX_USE_SPINCOUNT
			}
		#endif
		}
		else
			uStatus=Vpi_BadOutOfMemory;
	}
	else
		uStatus=Vpi_BadInvalidArgument;
#endif

#ifdef _GNUC_
	pthread_mutex_t* pPosixMutex    = Vpi_Null; 
	int result						= 0;
	pthread_mutexattr_t	att;

	if(phNewMutex)
	{
		pPosixMutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
		if (pPosixMutex)
		{
			/* initialize and set mutex attribute object */
			result = pthread_mutexattr_init(&att);
			if(result != 0)
				uStatus = Vpi_BadOutOfMemory;
			else
			{
				result = pthread_mutexattr_settype(&att, PTHREAD_MUTEX_RECURSIVE_NP);
				if(result != 0)
				{
					pthread_mutexattr_destroy(&att);
					uStatus = Vpi_Bad;
				}
				else
				{
					/* initialize mutex with the attribute object */
					result = pthread_mutex_init(pPosixMutex, &att); /* code */
					if(result != 0)
						uStatus = Vpi_Bad;
					else
					{
						/* delete the temporary attribute object */
						result = pthread_mutexattr_destroy(&att); /* code */
						if(result != 0)
						{
							uStatus = Vpi_Bad;
							Vpi_Mutex_Delete((Vpi_Mutex*)&pPosixMutex);
						}
						else
						{
							*phNewMutex = (Vpi_Mutex)pPosixMutex;	
						}			
					}
				}
			}
		}
	}
	else
		uStatus= Vpi_BadInvalidArgument;
#endif
	return uStatus;
}
Vpi_Void       VPI_DLLCALL Vpi_Mutex_Delete(Vpi_Mutex* phMutex)
{
#ifdef WIN32
	LPCRITICAL_SECTION lpCriticalSection = NULL;

	if(phMutex == Vpi_Null || *phMutex == Vpi_Null)
	{
		return;
	}
	lpCriticalSection = (LPCRITICAL_SECTION)*phMutex;
	DeleteCriticalSection(lpCriticalSection);
	free(lpCriticalSection);

	*phMutex = Vpi_Null;
#endif
#ifdef _GNUC_
	if( phMutex == Vpi_Null || *phMutex == Vpi_Null)
	{
		return;
	}
	else
	{	
		pthread_mutex_t* pPosixMutex = Vpi_Null;
		pPosixMutex = (pthread_mutex_t*)(*phMutex);

		if(pPosixMutex != Vpi_Null)
		{

			pthread_mutex_destroy(pPosixMutex);
			free(*phMutex);
			*phMutex = Vpi_Null;
		}
	}
#endif
	return;
}
Vpi_Void       VPI_DLLCALL Vpi_Mutex_Lock(Vpi_Mutex  hMutex)
{
#ifdef WIN32
	if(hMutex != Vpi_Null)
		EnterCriticalSection((CRITICAL_SECTION*)hMutex);
	return;
#endif
#ifdef _GNUC_
	Vpi_StatusCode    uStatus     = Vpi_Good;
	int                 apiResult   = 0;
	pthread_mutex_t*    pPosixMutex = (pthread_mutex_t*)hMutex;

	if(hMutex != Vpi_Null)
	{

		apiResult = pthread_mutex_lock(pPosixMutex);

		if(apiResult != 0)
		{
			/* debug; makes no sense, i know */
			if(apiResult == EINVAL)
				uStatus = Vpi_BadInternalError;
			else 
			{
				if(apiResult == EDEADLK)
					uStatus = Vpi_BadInternalError;
				else
					uStatus = Vpi_BadInternalError;
			}
		}
		else
			uStatus = Vpi_Good;
		hMutex = (Vpi_Mutex)pPosixMutex;
	}
#endif
}
Vpi_Void       VPI_DLLCALL Vpi_Mutex_Unlock(Vpi_Mutex  hMutex)
{
#ifdef WIN32
	if(hMutex != Vpi_Null)
		LeaveCriticalSection((CRITICAL_SECTION*)hMutex);
#endif
#ifdef _GNUC_
	if(hMutex != Vpi_Null)
	{
		Vpi_StatusCode    uStatus     = Vpi_Good;
		int                 apiResult   = 0;
		pthread_mutex_t*    pPosixMutex = (pthread_mutex_t*)hMutex;

		if(hMutex == Vpi_Null)    
			getchar();
		else
		{
			apiResult = pthread_mutex_unlock(pPosixMutex);
			if(apiResult != 0)
			{
				/* EPERM = 1 (unlocking unowned mutex) */
				uStatus = Vpi_BadInternalError;
			}
			hMutex = (Vpi_Mutex)pPosixMutex;
		}
	}
#endif
	return;
}

Vpi_Int32 VPI_DLLCALL Vpi_InterlockExchange(Vpi_Int32* volatile pTarget, Vpi_Int32 value)
{
	Vpi_Int32    iValue;
#ifdef WIN32
	iValue=::InterlockedExchange(pTarget, value);
#endif
#ifdef _GNUC_
	iValue=__sync_lock_test_and_set (pTarget, value);
#endif
	return iValue;
}