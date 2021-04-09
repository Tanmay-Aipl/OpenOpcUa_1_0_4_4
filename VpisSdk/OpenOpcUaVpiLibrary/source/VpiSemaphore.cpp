//**************************************************************************
//
//  Copyright (c) 4CE Industry 1999-2014, All Rights Reserved
//
//**************************************************************************
//
//  Filename   :  VpiSemaphore.cpp
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
#include "VpiSemaphore.h"
#include <sys/timeb.h> 
Vpi_StatusCode VPI_DLLCALL Vpi_Semaphore_Create(Vpi_Semaphore*    a_Semaphore, 
															Vpi_UInt32        a_uInitalValue,
															Vpi_UInt32        a_uMaxRange)
{
	Vpi_StatusCode uStatus=Vpi_Good;
	if (a_Semaphore)
	{
	#ifdef WIN32
		HANDLE InternalSemaphore = NULL;

		*a_Semaphore = Vpi_Null;

		if((a_uMaxRange == 0) || (a_uMaxRange < a_uInitalValue))
			uStatus= Vpi_BadInvalidArgument;
		else
		{
			InternalSemaphore = CreateSemaphore(    NULL,
					a_uInitalValue,
					a_uMaxRange,
					(LPCTSTR)NULL);

			if(InternalSemaphore == NULL)
				uStatus= Vpi_BadInternalError;
			else
				*a_Semaphore = (Vpi_Semaphore)InternalSemaphore;
		}
	#endif
	#ifdef _GNUC_
		sem_private token;
		int result;
		if (a_uMaxRange==0)
			uStatus=Vpi_BadInvalidArgument;
		else
		{
			if (a_uMaxRange<a_uInitalValue)
				uStatus=Vpi_BadInvalidArgument;
			else
			{
				token = (sem_private)malloc(sizeof(sem_private_struct));

				pthread_mutexattr_t	att;

				/* initialize and set mutex attribute object */
				result = pthread_mutexattr_init(&att);
				if(result != 0)
				{
					free(token);
					uStatus= Vpi_BadInternalError;
				}
				else
				{
					if( pthread_mutex_init(&(token->mutex),&att) )
					{
						free(token);
						uStatus = Vpi_Bad;
					}
					else
					{
						/* delete the temporary attribute object */
						result = pthread_mutexattr_destroy(&att); /* code */
						if(result != 0)
							uStatus = Vpi_Bad;
						else
						{
							if( pthread_cond_init(&(token->condition),NULL) )
							{
								pthread_mutex_destroy(&(token->mutex));
								free(token);
								uStatus = Vpi_Bad;
							}
							else
							{
								token->semCount = a_uInitalValue;
								token->semMaxRange=a_uMaxRange;
								*a_Semaphore = (Vpi_Semaphore)token;
							}
						}
					}
				}
			}
		}
	#endif
	}
	else
		uStatus=Vpi_BadInvalidArgument;
	return uStatus;
}

Vpi_StatusCode VPI_DLLCALL Vpi_Semaphore_Delete(Vpi_Semaphore* pRawSemaphore)
{
	Vpi_StatusCode uStatus=Vpi_Good;
#ifdef WIN32
	if(pRawSemaphore == Vpi_Null || *pRawSemaphore == Vpi_Null)
		uStatus = Vpi_BadInvalidArgument;
	else
	{
		if(CloseHandle((HANDLE)*pRawSemaphore) != 0)
			/* CloseHandle succeeded if it returned non-zero */
			*pRawSemaphore = Vpi_Null;
		else
			/* CloseHandle failed if it returned zero */
			uStatus=Vpi_BadInternalError;
	}
#endif
#ifdef _GNUC_
	sem_private token = (sem_private)*pRawSemaphore;

	if (pthread_mutex_lock(&(token->mutex)))
		uStatus=Vpi_Bad;
	else
	{
		if (pthread_cond_destroy(&(token->condition)))
			uStatus=Vpi_Bad;
		else
		{
			if (pthread_mutex_unlock(&(token->mutex)))
				uStatus=Vpi_Bad;
			else
			{
				if (pthread_mutex_destroy(&(token->mutex)))
					uStatus=Vpi_Bad;
				else
				{
					free(token);
					*pRawSemaphore = Vpi_Null;
					uStatus=Vpi_Good;
				}
			}
		}
	}
#endif
	return uStatus;
	//return Vpi_ProxyStub_g_PlatformLayerCalltable->SemaphoreDelete(phSemaphore);
}

Vpi_StatusCode VPI_DLLCALL Vpi_Semaphore_Wait(Vpi_Semaphore RawSemaphore)
{
	Vpi_StatusCode uStatus=Vpi_Good;
#ifdef WIN32
	HANDLE  InternalSemaphore = (HANDLE)RawSemaphore;
	DWORD   dwResult;

	dwResult = WaitForSingleObject(InternalSemaphore, INFINITE);
	if(dwResult == WAIT_TIMEOUT)
	{
		uStatus = Vpi_GoodNonCriticalTimeout;
	}
	else 
	{
		if(dwResult == WAIT_OBJECT_0)
			uStatus = Vpi_Good;
		else /*dwResult == WAIT_FAILED*/
			uStatus = Vpi_BadInternalError;
	}
#endif
#ifdef _GNUC_
	sem_private token = (sem_private)RawSemaphore;
	int rc;

	if ( pthread_mutex_lock(&(token->mutex)) )
		uStatus = Vpi_Bad;
	else
	{
		while (token->semCount <= 0)
		{
			rc = pthread_cond_wait(&(token->condition), &(token->mutex));
			if (rc &&errno != EINTR ){
				break;
			}
		}
		token->semCount--;
		if ( pthread_mutex_unlock(&(token->mutex)))
			uStatus= Vpi_Bad;
	}
#endif
	return uStatus;
}

Vpi_StatusCode VPI_DLLCALL Vpi_Semaphore_TimedWait(   Vpi_Semaphore     a_RawSemaphore, 
															Vpi_UInt32        msecTimeout)
{
	Vpi_StatusCode uStatus=Vpi_Good;
#ifdef WIN32
	HANDLE  InternalSemaphore = (HANDLE)a_RawSemaphore;
	DWORD   dwResult;

	dwResult = WaitForSingleObject(InternalSemaphore, msecTimeout);

	if(dwResult == WAIT_TIMEOUT)
		uStatus = Vpi_GoodNonCriticalTimeout;
	else
	{
		if(dwResult == WAIT_OBJECT_0)
			uStatus = Vpi_Good;
		else /*dwResult == WAIT_FAILED*/
			uStatus = Vpi_BadInternalError;
	}
#endif
//
#ifdef _GNUC_
	sem_private token = (sem_private)a_RawSemaphore;
	struct timespec	ts;
	struct timeb tp;
	long sec, millisec;
	int rc=0;

	if ( pthread_mutex_lock(&(token->mutex)) )
		uStatus = Vpi_BadInternalError;
	else
	{
		sec = msecTimeout / 1000;
		millisec = msecTimeout % 1000;

		ftime( &tp );
		tp.time += sec;
		tp.millitm += millisec;
		if( tp.millitm > 999 )

		{
			tp.millitm -= 1000;
			tp.time++;
		}
		ts.tv_sec = tp.time;
		ts.tv_nsec = tp.millitm * 1000000 ;

		while (token->semCount <= 0)
		{
			rc = pthread_cond_timedwait(&(token->condition), &(token->mutex), &ts);
			if (rc && (errno != EINTR) )
				break;		
		}
		if (rc)
		{
			if ( pthread_mutex_unlock(&(token->mutex)) )
				uStatus = Vpi_BadInternalError;
			else
			{
				if ( rc == ETIMEDOUT )
					uStatus = Vpi_GoodNonCriticalTimeout;/* we have a time out */
				else
					uStatus = Vpi_BadInternalError;
			}
		}
		else
			token->semCount--;
		if ( pthread_mutex_unlock(&(token->mutex)) )
			uStatus = Vpi_BadInternalError;
	}
#endif
	return uStatus;
}

Vpi_StatusCode VPI_DLLCALL Vpi_Semaphore_Post(        Vpi_Semaphore     RawSemaphore,
															Vpi_UInt32        uReleaseCount)
{
	Vpi_StatusCode uStatus=Vpi_Good;
#ifdef WIN32
	HANDLE InternalSemaphore = (HANDLE)RawSemaphore;

	if(uReleaseCount < 1)
		uStatus = Vpi_BadInvalidArgument;
	else
	{
		Vpi_Int32        uPrevCount=0;
		if(ReleaseSemaphore(InternalSemaphore, uReleaseCount, &uPrevCount))
			uStatus = Vpi_Good;
		else
		{
#ifdef WIN32
			DWORD dwLastError = GetLastError();

			switch(dwLastError)
			{
				case ERROR_TOO_MANY_POSTS:
					uStatus = Vpi_BadTooManyPosts;
					break;
				default:
					uStatus = Vpi_BadInternalError;
					break;
			}
#endif
		}
	}
#endif
#ifdef _GNUC_
	sem_private token = (sem_private)RawSemaphore;
	Vpi_UInt32 ii=0;
	if(uReleaseCount < 1)
		uStatus=Vpi_BadInvalidArgument;
	else
	{
		for (ii=0;ii<uReleaseCount;ii++)
		{
			if ( pthread_mutex_lock(&(token->mutex)) )
				uStatus= Vpi_Bad;
			else
			{
				token->semCount ++;
				if ( pthread_mutex_unlock(&(token->mutex)) )
					uStatus= Vpi_Bad;
				else
				{
					if ( pthread_cond_signal(&(token->condition)) )
						uStatus= Vpi_Bad;
				}
			}
		}
	}
#endif
	return uStatus;
}