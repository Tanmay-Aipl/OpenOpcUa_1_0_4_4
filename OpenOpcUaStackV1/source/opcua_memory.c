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

#include <opcua_mutex.h>
#include <opcua_trace.h>

#include <opcua_memory.h>

#if !OPCUA_USE_STATIC_PLATFORM_INTERFACE
#define OPCUA_P_MEMORY_ALLOC    OpcUa_ProxyStub_g_PlatformLayerCalltable->MemAlloc
#define OPCUA_P_MEMORY_REALLOC  OpcUa_ProxyStub_g_PlatformLayerCalltable->MemReAlloc
#define OPCUA_P_MEMORY_FREE     OpcUa_ProxyStub_g_PlatformLayerCalltable->MemFree
#define OPCUA_P_MEMORY_MEMCPY   OpcUa_ProxyStub_g_PlatformLayerCalltable->MemCpy
#endif
/*============================================================================
 * OpcUa_Memory_Alloc
 *===========================================================================*/
OpcUa_Void* OPCUA_DLLCALL OpcUa_Memory_Alloc(OpcUa_UInt32 nSize)
{
	if(OpcUa_ProxyStub_g_PlatformLayerCalltable == NULL)
	{
		//return (OpcUa_Void*)malloc(nSize);
		return (OpcUa_Void*)calloc(1, nSize);
	}
	else
	{
		return OPCUA_P_MEMORY_ALLOC(nSize);
	}
}

/*============================================================================
 * OpcUa_Memory_ReAlloc
 *===========================================================================*/
OpcUa_Void* OPCUA_DLLCALL OpcUa_Memory_ReAlloc(   OpcUa_Void*     a_pBuffer, 
												  OpcUa_UInt32    a_nSize)
{
	return OPCUA_P_MEMORY_REALLOC(  a_pBuffer, 
									a_nSize);
}

/*============================================================================
 * OpcUa_Memory_Free
 *===========================================================================*/
OpcUa_Void OPCUA_DLLCALL OpcUa_Memory_Free(OpcUa_Void* a_pBuffer)
{
	if (OpcUa_ProxyStub_g_PlatformLayerCalltable==OpcUa_Null)
	{
		if (a_pBuffer != OpcUa_Null)
			free(a_pBuffer);
	}
	else
	{
		if (a_pBuffer != OpcUa_Null)
		{
			OPCUA_P_MEMORY_FREE(a_pBuffer);
		}
	}
}

/*============================================================================
 * OpcUa_Memory_MemCpy
 *===========================================================================*/
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_Memory_MemCpy(   OpcUa_Void*     a_pBuffer, 
										OpcUa_UInt32    a_nSizeInBytes, 
										OpcUa_Void*     a_pSource, 
										OpcUa_UInt32    a_nCount)
{
	return OPCUA_P_MEMORY_MEMCPY(   a_pBuffer, 
									a_nSizeInBytes, 
									a_pSource, 
									a_nCount);
}
