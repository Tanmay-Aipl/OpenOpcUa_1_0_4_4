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
#include <opcua_utilities.h>
#include <opcua_pkifactory.h>
#include <opcua_cryptofactory.h>
#include <opcua_core.h>
#include <sys/timeb.h> 
// Below change for OpenOpcUa Stack V1.5
#include <opcua_p_os.h>
#if OPCUA_MUTEX_ERROR_CHECKING
#define OPCUA_MUTEX_ERROR_CHECKING_PARAMETERS ,__FILE__,__LINE__
#else
#define OPCUA_MUTEX_ERROR_CHECKING_PARAMETERS 
#endif
// Structure declaration to support Event Semaphore on Linux and on POSIX like environment
#ifdef _GNUC_
typedef struct{
	pthread_mutex_t mutex;
	pthread_cond_t	condition;
	int				semCount;
	int				semMaxRange;
}sem_private_struct, *sem_private;
#endif
/*********************************************************************************/
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_Mutex_Create(OpcUa_Mutex* a_phMutex)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
#ifdef WIN32

	LPCRITICAL_SECTION lpCriticalSection = NULL;

	if(a_phMutex)
	{
		*a_phMutex = OpcUa_Null;
		
	#if OPCUA_MUTEX_USE_SPINCOUNT
		BOOL bRet;
		LONG hr;
	#endif

	#if 0
		lpCriticalSection = OpcUa_P_Memory_Alloc(sizeof(CRITICAL_SECTION));
	#else
		lpCriticalSection = (LPCRITICAL_SECTION)malloc(sizeof(CRITICAL_SECTION));
		memset(lpCriticalSection, 0, sizeof(CRITICAL_SECTION));
	#endif

		if(lpCriticalSection)
		{

		#if OPCUA_MUTEX_USE_SPINCOUNT
			bRet = InitializeCriticalSectionAndSpinCount(   lpCriticalSection,
															0x00000400) ;
			if (bRet == 0)
			{
				hr = GetLastError();
				free(lpCriticalSection);
				uStatus= OpcUa_Bad;
			}
			else
			{
		#else
			InitializeCriticalSection(lpCriticalSection);
		#endif

			*a_phMutex = (OpcUa_Mutex)lpCriticalSection;
		#if OPCUA_MUTEX_USE_SPINCOUNT
			}
		#endif
		}
		else
			uStatus=OpcUa_BadOutOfMemory;
	}
	else
		uStatus=OpcUa_BadInvalidArgument;
#endif

#ifdef _GNUC_
	pthread_mutex_t* pPosixMutex    = OpcUa_Null; 
	int result						= 0;
	pthread_mutexattr_t	att;

	if(a_phMutex == OpcUa_Null)
	{
		return OpcUa_BadInvalidArgument;
	}

	pPosixMutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
	OpcUa_ReturnErrorIfAllocFailed(pPosixMutex);

	/* initialize and set mutex attribute object */
	result = pthread_mutexattr_init(&att);
	if(result != 0)
	{
		return OpcUa_Bad;
	}

	result = pthread_mutexattr_settype(&att, PTHREAD_MUTEX_RECURSIVE_NP);
	if(result != 0)
	{
		pthread_mutexattr_destroy(&att);
		return OpcUa_Bad;
	}

	/* initialize mutex with the attribute object */
	result = pthread_mutex_init(pPosixMutex, &att); /* code */
	if(result != 0)
	{
		uStatus = OpcUa_Bad;
	}

	/* delete the temporary attribute object */
	result = pthread_mutexattr_destroy(&att); /* code */
	if(result != 0)
	{
		uStatus = OpcUa_Bad;
	}
	if(OpcUa_IsBad(uStatus))
	{
		OpcUa_Mutex_Delete((OpcUa_Mutex*)&pPosixMutex);
	}
	else
	{
		*a_phMutex = (OpcUa_Mutex)pPosixMutex;
	}
#endif
	return uStatus;
}

OpcUa_Void OPCUA_DLLCALL OpcUa_Mutex_Delete(OpcUa_Mutex* a_phMutex)
{
#ifdef WIN32
	LPCRITICAL_SECTION lpCriticalSection = NULL;

	if(a_phMutex == OpcUa_Null || *a_phMutex == OpcUa_Null)
	{
		return;
	}


	lpCriticalSection = (LPCRITICAL_SECTION)*a_phMutex;


	DeleteCriticalSection(lpCriticalSection);

#if 0
	OpcUa_P_Memory_Free(lpCriticalSection);
#else
	free(lpCriticalSection);
#endif

	*a_phMutex = OpcUa_Null;
#endif
#ifdef _GNUC_
	if( a_phMutex == OpcUa_Null || *a_phMutex == OpcUa_Null)
	{
		return;
	}
	else
	{	
		pthread_mutex_t* pPosixMutex = OpcUa_Null;
		pPosixMutex = (pthread_mutex_t*)(*a_phMutex);

		if(pPosixMutex != OpcUa_Null)
		{

			pthread_mutex_destroy(pPosixMutex);
			free(*a_phMutex);
			*a_phMutex = OpcUa_Null;
		}
	}
#endif
	return;
}

OpcUa_Void OPCUA_DLLCALL OpcUa_Mutex_Lock(OpcUa_Mutex hMutex)
{
#ifdef WIN32
	if(hMutex != OpcUa_Null)
		EnterCriticalSection((CRITICAL_SECTION*)hMutex);
	return;
#endif
#ifdef _GNUC_
	OpcUa_StatusCode    uStatus     = OpcUa_Good;
	int                 apiResult   = 0;
	pthread_mutex_t*    pPosixMutex = (pthread_mutex_t*)hMutex;

	if(hMutex != OpcUa_Null)
	{

		apiResult = pthread_mutex_lock(pPosixMutex);

		if(apiResult != 0)
		{
			/* debug; makes no sense, i know */
			if(apiResult == EINVAL)
				uStatus = OpcUa_BadInternalError;
			else 
			{
				if(apiResult == EDEADLK)
					uStatus = OpcUa_BadInternalError;
				else
					uStatus = OpcUa_BadInternalError;
			}
		}
		else
			uStatus = OpcUa_Good;
		hMutex = (OpcUa_Mutex)pPosixMutex;
	}
#endif
}

OpcUa_Void OPCUA_DLLCALL OpcUa_Mutex_Unlock(                OpcUa_Mutex hMutex)
{
#ifdef WIN32
	if(hMutex != OpcUa_Null)
		LeaveCriticalSection((CRITICAL_SECTION*)hMutex);
#endif
#ifdef _GNUC_
	if(hMutex != OpcUa_Null)
	{
		OpcUa_StatusCode    uStatus     = OpcUa_Good;
		int                 apiResult   = 0;
		pthread_mutex_t*    pPosixMutex = (pthread_mutex_t*)hMutex;

		if(hMutex == OpcUa_Null)    
			getchar();
		else
		{
			apiResult = pthread_mutex_unlock(pPosixMutex);
			if(apiResult != 0)
			{
				/* EPERM = 1 (unlocking unowned mutex) */
				uStatus = OpcUa_BadInternalError;
			}
			hMutex = (OpcUa_Mutex)pPosixMutex;
		}
	}
#endif
	return;
}

/*********************************************************************************/
OpcUa_DateTime OPCUA_DLLCALL OpcUa_DateTime_UtcNow(         OpcUa_Void)
{
	OpcUa_DateTime dtVal;
	OpcUa_DateTime_Initialize(&dtVal);
	if (OpcUa_ProxyStub_g_PlatformLayerCalltable)
		return OpcUa_ProxyStub_g_PlatformLayerCalltable->UtcNow();
	else
		return dtVal;
}

OpcUa_UInt32 OPCUA_DLLCALL OpcUa_Utility_GetTickCount(      OpcUa_Void)
{
	if (OpcUa_ProxyStub_g_PlatformLayerCalltable)
		return OpcUa_ProxyStub_g_PlatformLayerCalltable->UtilGetTickCount();
	else
		return 0;
}

/*********************************************************************************/
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_Semaphore_Create(      OpcUa_Semaphore*    a_Semaphore, 
															OpcUa_UInt32        a_uInitalValue,
															OpcUa_UInt32        a_uMaxRange)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	if (a_Semaphore)
	{
	#ifdef WIN32
		HANDLE InternalSemaphore = NULL;

		*a_Semaphore = OpcUa_Null;

		if((a_uMaxRange == 0) || (a_uMaxRange < a_uInitalValue))
			uStatus= OpcUa_BadInvalidArgument;
		else
		{
			InternalSemaphore = CreateSemaphore(    NULL,
					a_uInitalValue,
					a_uMaxRange,
					(LPCTSTR)NULL);

			if(InternalSemaphore == NULL)
				uStatus= OpcUa_BadInternalError;
			else
				*a_Semaphore = (OpcUa_Semaphore)InternalSemaphore;
		}
	#endif
	#ifdef _GNUC_
		sem_private token;
		int result;
		if (a_uMaxRange==0)
			uStatus=OpcUa_BadInvalidArgument;
		else
		{
			if (a_uMaxRange<a_uInitalValue)
				uStatus=OpcUa_BadInvalidArgument;
			else
			{
				token = (sem_private)malloc(sizeof(sem_private_struct));

				pthread_mutexattr_t	att;

				/* initialize and set mutex attribute object */
				result = pthread_mutexattr_init(&att);
				if(result != 0)
				{
					free(token);
					uStatus= OpcUa_BadInternalError;
				}
				else
				{
					if( pthread_mutex_init(&(token->mutex),&att) )
					{
						free(token);
						uStatus = OpcUa_Bad;
					}
					else
					{
						/* delete the temporary attribute object */
						result = pthread_mutexattr_destroy(&att); /* code */
						if(result != 0)
							uStatus = OpcUa_Bad;
						else
						{
							if( pthread_cond_init(&(token->condition),NULL) )
							{
								pthread_mutex_destroy(&(token->mutex));
								free(token);
								uStatus = OpcUa_Bad;
							}
							else
							{
								token->semCount = a_uInitalValue;
								token->semMaxRange=a_uMaxRange;
								*a_Semaphore = (OpcUa_Semaphore)token;
							}
						}
					}
				}
			}
		}
	#endif
	}
	else
		uStatus=OpcUa_BadInvalidArgument;
	return uStatus;
}

OpcUa_StatusCode OPCUA_DLLCALL OpcUa_Semaphore_Delete(OpcUa_Semaphore* pRawSemaphore)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
#ifdef WIN32
	if(pRawSemaphore == OpcUa_Null || *pRawSemaphore == OpcUa_Null)
		uStatus = OpcUa_BadInvalidArgument;
	else
	{
		HANDLE internalSemaphore = (HANDLE)*pRawSemaphore;
		if (CloseHandle(internalSemaphore) != 0)
			/* CloseHandle succeeded if it returned non-zero */
			*pRawSemaphore = OpcUa_Null;
		else
			/* CloseHandle failed if it returned zero */
			uStatus=OpcUa_BadInternalError;
	}
#endif
#ifdef _GNUC_
	sem_private token = (sem_private)*pRawSemaphore;

	if (pthread_mutex_lock(&(token->mutex)))
		uStatus=OpcUa_Bad;
	else
	{
		if (pthread_cond_destroy(&(token->condition)))
			uStatus=OpcUa_Bad;
		else
		{
			if (pthread_mutex_unlock(&(token->mutex)))
				uStatus=OpcUa_Bad;
			else
			{
				if (pthread_mutex_destroy(&(token->mutex)))
					uStatus=OpcUa_Bad;
				else
				{
					free(token);
					*pRawSemaphore = OpcUa_Null;
					uStatus=OpcUa_Good;
				}
			}
		}
	}
#endif
	return uStatus;
}

OpcUa_StatusCode OPCUA_DLLCALL OpcUa_Semaphore_Wait(OpcUa_Semaphore RawSemaphore)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
#ifdef WIN32
	HANDLE  InternalSemaphore = (HANDLE)RawSemaphore;
	DWORD   dwResult;

	dwResult = WaitForSingleObject(InternalSemaphore, INFINITE);
	if(dwResult == WAIT_TIMEOUT)
	{
		uStatus = OpcUa_GoodNonCriticalTimeout;
	}
	else 
	{
		if(dwResult == WAIT_OBJECT_0)
			uStatus = OpcUa_Good;
		else /*dwResult == WAIT_FAILED*/
			uStatus = OpcUa_BadInternalError;
	}
#endif
#ifdef _GNUC_
	sem_private token = (sem_private)RawSemaphore;
	int rc;

	if ( pthread_mutex_lock(&(token->mutex)) )
		uStatus = OpcUa_Bad;
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
			uStatus= OpcUa_Bad;
	}
#endif
	return uStatus;
}

OpcUa_StatusCode OPCUA_DLLCALL OpcUa_Semaphore_TimedWait(   OpcUa_Semaphore     a_RawSemaphore, 
															OpcUa_UInt32        msecTimeout)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
#ifdef WIN32
	HANDLE  InternalSemaphore = (HANDLE)a_RawSemaphore;
	DWORD   dwResult;

	dwResult = WaitForSingleObject(InternalSemaphore, msecTimeout);

	if(dwResult == WAIT_TIMEOUT)
		uStatus = OpcUa_GoodNonCriticalTimeout;
	else
	{
		if(dwResult == WAIT_OBJECT_0)
			uStatus = OpcUa_Good;
		else /*dwResult == WAIT_FAILED*/
			uStatus = OpcUa_BadInternalError;
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
		uStatus = OpcUa_BadInternalError;
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
			if ( rc == ETIMEDOUT )
				uStatus = OpcUa_GoodNonCriticalTimeout;/* we have a time out */
			else
				uStatus = OpcUa_BadInternalError;
		}
		else
			token->semCount--;
		if ( pthread_mutex_unlock(&(token->mutex)) )
			uStatus = OpcUa_BadInternalError;
	}
#endif
	return uStatus;
}

OpcUa_StatusCode OPCUA_DLLCALL OpcUa_Semaphore_Post(        OpcUa_Semaphore     RawSemaphore,
															OpcUa_UInt32        uReleaseCount)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
#ifdef WIN32
	HANDLE InternalSemaphore = (HANDLE)RawSemaphore;

	if(uReleaseCount < 1)
		uStatus = OpcUa_BadInvalidArgument;
	else
	{
		OpcUa_Int32        uPrevCount=0;
		if(ReleaseSemaphore(InternalSemaphore, uReleaseCount, &uPrevCount))
			uStatus = OpcUa_Good;
		else
		{
			DWORD dwLastError = GetLastError();

			switch(dwLastError)
			{
				case ERROR_TOO_MANY_POSTS:
					uStatus = OpcUa_BadTooManyPosts;
					break;
				default:
					uStatus = OpcUa_BadInternalError;
					break;
			}
		}
	}
#endif
#ifdef _GNUC_
	sem_private token = (sem_private)RawSemaphore;
	OpcUa_UInt32 ii=0;
	if(uReleaseCount < 1)
		uStatus=OpcUa_BadInvalidArgument;
	else
	{
		for (ii=0;ii<uReleaseCount;ii++)
		{
			if ( pthread_mutex_lock(&(token->mutex)) )
				uStatus= OpcUa_Bad;
			else
			{
				token->semCount ++;
				if ( pthread_mutex_unlock(&(token->mutex)) )
					uStatus= OpcUa_Bad;
				else
				{
					if ( pthread_cond_signal(&(token->condition)) )
						uStatus= OpcUa_Bad;
				}
			}
		}
	}
#endif
	return uStatus;
}

/*********************************************************************************/

OpcUa_StatusCode OPCUA_DLLCALL OpcUa_SocketManager_Create(OpcUa_SocketManager*  ppSocketManager, 
														  OpcUa_UInt32          nSockets, 
														  OpcUa_UInt32          uintFlags)
{
	return OpcUa_ProxyStub_g_PlatformLayerCalltable->SocketManagerCreate(ppSocketManager, 
		nSockets, 
		uintFlags);
}


OpcUa_Void       OPCUA_DLLCALL OpcUa_SocketManager_Delete(OpcUa_SocketManager* pSocketManager)
{
	OpcUa_ProxyStub_g_PlatformLayerCalltable->SocketManagerDelete(pSocketManager);
}

OpcUa_StatusCode OPCUA_DLLCALL OpcUa_SocketManager_CreateServer(OpcUa_SocketManager        a_pSocketManager,
																OpcUa_StringA              a_LocalAdress,
																OpcUa_ByteString*          a_pServerCertificate,
																OpcUa_Key*                 a_pServerPrivateKey,
																OpcUa_Void*                a_pPKIConfig,
																OpcUa_Socket_EventCallback a_pfnSocketCallBack,
																OpcUa_Void*                a_pCookie,
																OpcUa_Socket*              a_ppSocket)
{
	return OpcUa_ProxyStub_g_PlatformLayerCalltable->SocketManagerCreateServer(a_pSocketManager,
											  a_LocalAdress,
											  a_pServerCertificate,
											  a_pServerPrivateKey,
											  a_pPKIConfig,
											  a_pfnSocketCallBack,
											  a_pCookie,
											  a_ppSocket);
}

OpcUa_StatusCode OPCUA_DLLCALL OpcUa_SocketManager_CreateClient(OpcUa_SocketManager                 a_pSocketManager,
																OpcUa_StringA                       a_RemoteAdress,
																OpcUa_UInt16                        a_LocalPort,
																OpcUa_ByteString*                   a_pClientCertificate,
																OpcUa_Key*                          a_pClientPrivateKey,
																OpcUa_Void*                         a_pPKIConfig,
																OpcUa_Socket_EventCallback          a_pfnSocketCallBack,
																OpcUa_Socket_CertificateCallback    a_pfnCertificateCallback,
																OpcUa_Void*                         a_pCookie,
																OpcUa_Socket*                       a_ppSocket)
{
	return OpcUa_ProxyStub_g_PlatformLayerCalltable->SocketManagerCreateClient(a_pSocketManager,
											  a_RemoteAdress,
											  a_LocalPort,
											  a_pClientCertificate,
											  a_pClientPrivateKey,
											  a_pPKIConfig,
											  a_pfnSocketCallBack,
											  a_pfnCertificateCallback,
											  a_pCookie,
											  a_ppSocket);
}


OpcUa_StatusCode OPCUA_DLLCALL OpcUa_Socket_Close(  OpcUa_Socket pSocket)
{
	return OpcUa_ProxyStub_g_PlatformLayerCalltable->SocketClose(pSocket);
}

OpcUa_StatusCode OPCUA_DLLCALL OpcUa_Socket_Read(   OpcUa_Socket    pSocket,
													OpcUa_Byte*     pBuffer,
													OpcUa_UInt32    BufferSize,
													OpcUa_UInt32*   puintBytesRead)
{
	return OpcUa_ProxyStub_g_PlatformLayerCalltable->SocketRead(    pSocket,
																	pBuffer,
																	BufferSize,
																	puintBytesRead);
}

OpcUa_Int32      OPCUA_DLLCALL OpcUa_Socket_Write(  OpcUa_Socket    pSocket,
													OpcUa_Byte*     pBuffer,
													OpcUa_UInt32    BufferSize,
													OpcUa_Boolean   bBlock)
{
	return OpcUa_ProxyStub_g_PlatformLayerCalltable->SocketWrite(   pSocket,
																	pBuffer,
																	BufferSize,
																	bBlock);
}

OpcUa_StatusCode OPCUA_DLLCALL OpcUa_SocketManager_Loop(OpcUa_SocketManager pSocketManager, 
														OpcUa_UInt32        msecTimeout,
														OpcUa_Boolean       bRunOnce)
{
	return OpcUa_ProxyStub_g_PlatformLayerCalltable->SocketManagerServeLoop(
		pSocketManager,
		msecTimeout,
		bRunOnce);
}

OpcUa_StatusCode OPCUA_DLLCALL OpcUa_SocketManager_SignalEvent(OpcUa_SocketManager pSocketManager,
															   OpcUa_UInt32        uintEvent,
															   OpcUa_Boolean       bAllLists)
{
	return OpcUa_ProxyStub_g_PlatformLayerCalltable->SocketManagerSignalEvent(  
		pSocketManager,
		uintEvent,
		bAllLists);
}

/*********************************************************************************/

OpcUa_StatusCode OPCUA_DLLCALL OpcUa_PKIProvider_Create(    OpcUa_Void*         a_pCertificateStoreConfig,
															OpcUa_PKIProvider*  a_pProvider)
{
	return OPCUA_P_PKIFACTORY_CREATEPKIPROVIDER(    a_pCertificateStoreConfig, 
													a_pProvider);
}

OpcUa_StatusCode OPCUA_DLLCALL OpcUa_PKIProvider_Delete(    OpcUa_PKIProvider*  a_pProvider)
{
	return OPCUA_P_PKIFACTORY_DELETEPKIPROVIDER(    a_pProvider);
}

/*********************************************************************************/
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_CryptoProvider_Create( OpcUa_StringA           a_psSecurityProfileUri,
															OpcUa_CryptoProvider*   a_pProvider)
{
	return OPCUA_P_CRYPTOFACTORY_CREATECRYPTOPROVIDER(  a_psSecurityProfileUri, 
														a_pProvider);
}

OpcUa_StatusCode OPCUA_DLLCALL OpcUa_CryptoProvider_Delete(OpcUa_CryptoProvider*   a_pProvider)
{
	return OPCUA_P_CRYPTOFACTORY_DELETECRYPTOPROVIDER(  a_pProvider);
}

/*********************************************************************************/
OpcUa_Int32 OPCUA_DLLCALL OpcUa_StringA_vsnprintf(  OpcUa_StringA               a_sDest, 
													OpcUa_UInt32                a_uCount, 
													const OpcUa_CharA*          a_sFormat, 
													OpcUa_P_VA_List             a_argptr)
{
	return OpcUa_ProxyStub_g_PlatformLayerCalltable->StrVsnPrintf(  a_sDest,
																	a_uCount,
																	(const OpcUa_StringA)a_sFormat,
																	a_argptr);
}

/*********************************************************************************/
OpcUa_Int32 OPCUA_DLLCALL OpcUa_StringA_snprintf(   OpcUa_StringA               a_sDest, 
													OpcUa_UInt32                a_uCount, 
													const OpcUa_CharA*          a_sFormat, 
													...)
{
	OpcUa_Int32 ret = 0;
	OpcUa_P_VA_List argumentList;

	if(a_sDest == OpcUa_Null || a_uCount == 0 || a_sFormat == OpcUa_Null)
	{
		return -1;
	}

	OPCUA_P_VA_START(argumentList, a_sFormat);

	ret = OpcUa_ProxyStub_g_PlatformLayerCalltable->StrVsnPrintf(   a_sDest,
																	a_uCount,
																	(const OpcUa_StringA)a_sFormat,
																	argumentList);
	OPCUA_P_VA_END(argumentList);

	return ret;
}
