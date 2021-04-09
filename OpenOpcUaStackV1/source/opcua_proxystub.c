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

#include <opcua_platformdefs.h> 
#include <opcua_trace.h>
#include <opcua_socket.h>
#include <opcua_timer.h>
#include <opcua_memory.h>
#include <opcua_mutex.h>
#include <opcua_proxystub.h>
#include <opcua_stringtable.h>

#ifndef OPCUA_PROXYSTUB_VERSIONSTRING
# define OPCUA_PROXYSTUB_VERSIONSTRING "UaStackVersion not set"
#endif /* OPCUA_PROXYSTUB_VERSIONSTRING */

#ifndef OPCUA_PROXYSTUB_STATICCONFIGSTRING
# define OPCUA_PROXYSTUB_STATICCONFIGSTRING "UaStackStaticConfiguration not set"
#endif /* OPCUA_PROXYSTUB_STATICCONFIGSTRING */

#define OPCUA_CONFIG_STRING_SIZE    800

OpcUa_Port_CallTable*               OpcUa_ProxyStub_g_PlatformLayerCalltable;
OpcUa_ProxyStubConfiguration        OpcUa_ProxyStub_g_Configuration;
OpcUa_EncodeableTypeTable           OpcUa_ProxyStub_g_EncodeableTypes;
OpcUa_StringTable                   OpcUa_ProxyStub_g_NamespaceUris;
OpcUa_StringA                       OpcUa_ProxyStub_g_pConfigString;
OpcUa_StringA                       OpcUa_ProxyStub_g_VersionString         = OPCUA_PROXYSTUB_VERSIONSTRING;
OpcUa_StringA                       OpcUa_ProxyStub_g_StaticConfigString    = OPCUA_PROXYSTUB_STATICCONFIGSTRING;

#if OPCUA_USE_SYNCHRONISATION
static OpcUa_Mutex                  OpcUa_ProxyStub_g_hGlobalsMutex         = OpcUa_Null;
#endif /* OPCUA_USE_SYNCHRONISATION */

static OpcUa_UInt32                 OpcUa_ProxyStub_g_uNoOfChannels         = 0;
static OpcUa_UInt32                 OpcUa_ProxyStub_g_uNoOfEndpoints        = 0;
static OpcUa_UInt32                 OpcUa_ProxyStub_g_uNoOfInits            = 0;

/*============================================================================
 * OpcUa_StandardNamespaceUris
 *===========================================================================*/
OpcUa_StringA            OpcUa_ProxyStub_StandardNamespaceUris[] =
{
    "http://opcfoundation.org/UA/",
    OpcUa_Null
};

/*============================================================================
 * OpcUa_ProxyStub_UpdateConfigString
 *===========================================================================*/
static OpcUa_StatusCode OpcUa_ProxyStub_UpdateConfigString()
{
    OpcUa_Int32  iRes  = 0;
    OpcUa_Int32  iPos  = 0;

OpcUa_InitializeStatus(OpcUa_Module_ProxyStub, "UpdateConfigString");

#if OPCUA_USE_SYNCHRONISATION
    OpcUa_Mutex_Lock(OpcUa_ProxyStub_g_hGlobalsMutex);
#endif /* OPCUA_USE_SYNCHRONISATION */

    if(OpcUa_ProxyStub_g_pConfigString == OpcUa_Null)
    {
        OpcUa_ProxyStub_g_pConfigString = (OpcUa_StringA)OpcUa_Alloc(OPCUA_CONFIG_STRING_SIZE + 1);
        OpcUa_GotoErrorIfAllocFailed(OpcUa_ProxyStub_g_pConfigString);
        OpcUa_MemSet(OpcUa_ProxyStub_g_pConfigString, 0, OPCUA_CONFIG_STRING_SIZE + 1);
    }

#if OPCUA_USE_SAFE_FUNCTIONS

    iRes = OpcUa_SnPrintfA(&OpcUa_ProxyStub_g_pConfigString[iPos], OPCUA_CONFIG_STRING_SIZE - iPos, OPCUA_CONFIG_STRING_SIZE - iPos, "%s:%u\\", "TraceEnabled", OpcUa_ProxyStub_g_Configuration.uProxyStub_Trace_Output);
    if(iRes > 0){iPos += iRes;}else{OpcUa_GotoErrorWithStatus(OpcUa_BadInternalError);}
    iRes = OpcUa_SnPrintfA(&OpcUa_ProxyStub_g_pConfigString[iPos], OPCUA_CONFIG_STRING_SIZE - iPos, OPCUA_CONFIG_STRING_SIZE - iPos, "%s:%u\\", "TraceLevel", OpcUa_ProxyStub_g_Configuration.uProxyStub_Trace_Level);
    if(iRes > 0){iPos += iRes;}else{OpcUa_GotoErrorWithStatus(OpcUa_BadOutOfMemory);}
    iRes = OpcUa_SnPrintfA(&OpcUa_ProxyStub_g_pConfigString[iPos], OPCUA_CONFIG_STRING_SIZE - iPos, OPCUA_CONFIG_STRING_SIZE - iPos, "%s:%i\\", "iSerializer_MaxAlloc", OpcUa_ProxyStub_g_Configuration.iSerializer_MaxAlloc);
    if(iRes > 0){iPos += iRes;}else{OpcUa_GotoErrorWithStatus(OpcUa_BadOutOfMemory);}
    iRes = OpcUa_SnPrintfA(&OpcUa_ProxyStub_g_pConfigString[iPos], OPCUA_CONFIG_STRING_SIZE - iPos, OPCUA_CONFIG_STRING_SIZE - iPos, "%s:%i\\", "iSerializer_MaxStringLength", OpcUa_ProxyStub_g_Configuration.iSerializer_MaxStringLength);
    if(iRes > 0){iPos += iRes;}else{OpcUa_GotoErrorWithStatus(OpcUa_BadOutOfMemory);}
    iRes = OpcUa_SnPrintfA(&OpcUa_ProxyStub_g_pConfigString[iPos], OPCUA_CONFIG_STRING_SIZE - iPos, OPCUA_CONFIG_STRING_SIZE - iPos, "%s:%i\\", "iSerializer_MaxByteStringLength", OpcUa_ProxyStub_g_Configuration.iSerializer_MaxByteStringLength);
    if(iRes > 0){iPos += iRes;}else{OpcUa_GotoErrorWithStatus(OpcUa_BadOutOfMemory);}
    iRes = OpcUa_SnPrintfA(&OpcUa_ProxyStub_g_pConfigString[iPos], OPCUA_CONFIG_STRING_SIZE - iPos, OPCUA_CONFIG_STRING_SIZE - iPos, "%s:%i\\", "iSerializer_MaxArrayLength", OpcUa_ProxyStub_g_Configuration.iSerializer_MaxArrayLength);
    if(iRes > 0){iPos += iRes;}else{OpcUa_GotoErrorWithStatus(OpcUa_BadOutOfMemory);}
    iRes = OpcUa_SnPrintfA(&OpcUa_ProxyStub_g_pConfigString[iPos], OPCUA_CONFIG_STRING_SIZE - iPos, OPCUA_CONFIG_STRING_SIZE - iPos, "%s:%i\\", "iSerializer_MaxMessageSize", OpcUa_ProxyStub_g_Configuration.iSerializer_MaxMessageSize);
    if(iRes > 0){iPos += iRes;}else{OpcUa_GotoErrorWithStatus(OpcUa_BadOutOfMemory);}
    iRes = OpcUa_SnPrintfA(&OpcUa_ProxyStub_g_pConfigString[iPos], OPCUA_CONFIG_STRING_SIZE - iPos, OPCUA_CONFIG_STRING_SIZE - iPos, "%s:%u\\", "bSecureListener_ThreadPool_Enabled", (OpcUa_ProxyStub_g_Configuration.bSecureListener_ThreadPool_Enabled != 0)?1:0);
    if(iRes > 0){iPos += iRes;}else{OpcUa_GotoErrorWithStatus(OpcUa_BadOutOfMemory);}
    iRes = OpcUa_SnPrintfA(&OpcUa_ProxyStub_g_pConfigString[iPos], OPCUA_CONFIG_STRING_SIZE - iPos, OPCUA_CONFIG_STRING_SIZE - iPos, "%s:%i\\", "iSecureListener_ThreadPool_MinThreads", OpcUa_ProxyStub_g_Configuration.iSecureListener_ThreadPool_MinThreads);
    if(iRes > 0){iPos += iRes;}else{OpcUa_GotoErrorWithStatus(OpcUa_BadOutOfMemory);}
    iRes = OpcUa_SnPrintfA(&OpcUa_ProxyStub_g_pConfigString[iPos], OPCUA_CONFIG_STRING_SIZE - iPos, OPCUA_CONFIG_STRING_SIZE - iPos, "%s:%i\\", "iSecureListener_ThreadPool_MaxThreads", OpcUa_ProxyStub_g_Configuration.iSecureListener_ThreadPool_MaxThreads);
    if(iRes > 0){iPos += iRes;}else{OpcUa_GotoErrorWithStatus(OpcUa_BadOutOfMemory);}
    iRes = OpcUa_SnPrintfA(&OpcUa_ProxyStub_g_pConfigString[iPos], OPCUA_CONFIG_STRING_SIZE - iPos, OPCUA_CONFIG_STRING_SIZE - iPos, "%s:%i\\", "iSecureListener_ThreadPool_MaxJobs", OpcUa_ProxyStub_g_Configuration.iSecureListener_ThreadPool_MaxJobs);
    if(iRes > 0){iPos += iRes;}else{OpcUa_GotoErrorWithStatus(OpcUa_BadOutOfMemory);}
    iRes = OpcUa_SnPrintfA(&OpcUa_ProxyStub_g_pConfigString[iPos], OPCUA_CONFIG_STRING_SIZE - iPos, OPCUA_CONFIG_STRING_SIZE - iPos, "%s:%u\\", "bSecureListener_ThreadPool_BlockOnAdd", (OpcUa_ProxyStub_g_Configuration.bSecureListener_ThreadPool_BlockOnAdd != 0)?1:0);
    if(iRes > 0){iPos += iRes;}else{OpcUa_GotoErrorWithStatus(OpcUa_BadOutOfMemory);}
    iRes = OpcUa_SnPrintfA(&OpcUa_ProxyStub_g_pConfigString[iPos], OPCUA_CONFIG_STRING_SIZE - iPos, OPCUA_CONFIG_STRING_SIZE - iPos, "%s:%u\\", "uSecureListener_ThreadPool_Timeout", OpcUa_ProxyStub_g_Configuration.uSecureListener_ThreadPool_Timeout);
    if(iRes > 0){iPos += iRes;}else{OpcUa_GotoErrorWithStatus(OpcUa_BadOutOfMemory);}
    iRes = OpcUa_SnPrintfA(&OpcUa_ProxyStub_g_pConfigString[iPos], OPCUA_CONFIG_STRING_SIZE - iPos, OPCUA_CONFIG_STRING_SIZE - iPos, "%s:%u\\", "bTcpListener_ClientThreadsEnabled", (OpcUa_ProxyStub_g_Configuration.bTcpListener_ClientThreadsEnabled != 0)?1:0);
    if(iRes > 0){iPos += iRes;}else{OpcUa_GotoErrorWithStatus(OpcUa_BadOutOfMemory);}
    iRes = OpcUa_SnPrintfA(&OpcUa_ProxyStub_g_pConfigString[iPos], OPCUA_CONFIG_STRING_SIZE - iPos, OPCUA_CONFIG_STRING_SIZE - iPos, "%s:%i\\", "iTcpListener_DefaultChunkSize", OpcUa_ProxyStub_g_Configuration.iTcpListener_DefaultChunkSize);
    if(iRes > 0){iPos += iRes;}else{OpcUa_GotoErrorWithStatus(OpcUa_BadOutOfMemory);}
    iRes = OpcUa_SnPrintfA(&OpcUa_ProxyStub_g_pConfigString[iPos], OPCUA_CONFIG_STRING_SIZE - iPos, OPCUA_CONFIG_STRING_SIZE - iPos, "%s:%i\\", "iTcpConnection_DefaultChunkSize", OpcUa_ProxyStub_g_Configuration.iTcpConnection_DefaultChunkSize);
    if(iRes > 0){iPos += iRes;}else{OpcUa_GotoErrorWithStatus(OpcUa_BadOutOfMemory);}
    iRes = OpcUa_SnPrintfA(&OpcUa_ProxyStub_g_pConfigString[iPos], OPCUA_CONFIG_STRING_SIZE - iPos, OPCUA_CONFIG_STRING_SIZE - iPos, "%s:%i\\", "iTcpTransport_MaxMessageLength", OpcUa_ProxyStub_g_Configuration.iTcpTransport_MaxMessageLength);
    if(iRes > 0){iPos += iRes;}else{OpcUa_GotoErrorWithStatus(OpcUa_BadOutOfMemory);}
    iRes = OpcUa_SnPrintfA(&OpcUa_ProxyStub_g_pConfigString[iPos], OPCUA_CONFIG_STRING_SIZE - iPos, OPCUA_CONFIG_STRING_SIZE - iPos, "%s:%i\\", "iTcpTransport_MaxChunkCount", OpcUa_ProxyStub_g_Configuration.iTcpTransport_MaxChunkCount);
    if(iRes > 0){iPos += iRes;}else{OpcUa_GotoErrorWithStatus(OpcUa_BadOutOfMemory);}
    iRes = OpcUa_SnPrintfA(&OpcUa_ProxyStub_g_pConfigString[iPos], OPCUA_CONFIG_STRING_SIZE - iPos, OPCUA_CONFIG_STRING_SIZE - iPos, "%s:%u\\", "bTcpStream_ExpectWriteToBlock", (OpcUa_ProxyStub_g_Configuration.bTcpStream_ExpectWriteToBlock != 0)?1:0);
    if(iRes > 0){iPos += iRes;}else{OpcUa_GotoErrorWithStatus(OpcUa_BadOutOfMemory);}

#else /* OPCUA_USE_SAFE_FUNCTIONS */

    iRes = OpcUa_SnPrintfA(&OpcUa_ProxyStub_g_pConfigString[iPos], OPCUA_CONFIG_STRING_SIZE - iPos, "%s:%u\\", "TraceEnabled", (OpcUa_UInt)OpcUa_ProxyStub_g_Configuration.uProxyStub_Trace_Output);
    if(iRes > 0){iPos += iRes;}else{OpcUa_GotoErrorWithStatus(OpcUa_BadInternalError);}
    iRes = OpcUa_SnPrintfA(&OpcUa_ProxyStub_g_pConfigString[iPos], OPCUA_CONFIG_STRING_SIZE - iPos, "%s:%u\\", "TraceLevel", (OpcUa_UInt)OpcUa_ProxyStub_g_Configuration.uProxyStub_Trace_Level);
    if(iRes > 0){iPos += iRes;}else{OpcUa_GotoErrorWithStatus(OpcUa_BadOutOfMemory);}
    iRes = OpcUa_SnPrintfA(&OpcUa_ProxyStub_g_pConfigString[iPos], OPCUA_CONFIG_STRING_SIZE - iPos, "%s:%i\\", "iSerializer_MaxAlloc", (OpcUa_Int)OpcUa_ProxyStub_g_Configuration.iSerializer_MaxAlloc);
    if(iRes > 0){iPos += iRes;}else{OpcUa_GotoErrorWithStatus(OpcUa_BadOutOfMemory);}
    iRes = OpcUa_SnPrintfA(&OpcUa_ProxyStub_g_pConfigString[iPos], OPCUA_CONFIG_STRING_SIZE - iPos, "%s:%i\\", "iSerializer_MaxStringLength", (OpcUa_Int)OpcUa_ProxyStub_g_Configuration.iSerializer_MaxStringLength);
    if(iRes > 0){iPos += iRes;}else{OpcUa_GotoErrorWithStatus(OpcUa_BadOutOfMemory);}
    iRes = OpcUa_SnPrintfA(&OpcUa_ProxyStub_g_pConfigString[iPos], OPCUA_CONFIG_STRING_SIZE - iPos, "%s:%i\\", "iSerializer_MaxByteStringLength", (OpcUa_Int)OpcUa_ProxyStub_g_Configuration.iSerializer_MaxByteStringLength);
    if(iRes > 0){iPos += iRes;}else{OpcUa_GotoErrorWithStatus(OpcUa_BadOutOfMemory);}
    iRes = OpcUa_SnPrintfA(&OpcUa_ProxyStub_g_pConfigString[iPos], OPCUA_CONFIG_STRING_SIZE - iPos, "%s:%i\\", "iSerializer_MaxArrayLength", (OpcUa_Int)OpcUa_ProxyStub_g_Configuration.iSerializer_MaxArrayLength);
    if(iRes > 0){iPos += iRes;}else{OpcUa_GotoErrorWithStatus(OpcUa_BadOutOfMemory);}
    iRes = OpcUa_SnPrintfA(&OpcUa_ProxyStub_g_pConfigString[iPos], OPCUA_CONFIG_STRING_SIZE - iPos, "%s:%i\\", "iSerializer_MaxMessageSize", (OpcUa_Int)OpcUa_ProxyStub_g_Configuration.iSerializer_MaxMessageSize);
    if(iRes > 0){iPos += iRes;}else{OpcUa_GotoErrorWithStatus(OpcUa_BadOutOfMemory);}
    iRes = OpcUa_SnPrintfA(&OpcUa_ProxyStub_g_pConfigString[iPos], OPCUA_CONFIG_STRING_SIZE - iPos, "%s:%u\\", "bSecureListener_ThreadPool_Enabled", (OpcUa_ProxyStub_g_Configuration.bSecureListener_ThreadPool_Enabled != 0)?1:0);
    if(iRes > 0){iPos += iRes;}else{OpcUa_GotoErrorWithStatus(OpcUa_BadOutOfMemory);}
    iRes = OpcUa_SnPrintfA(&OpcUa_ProxyStub_g_pConfigString[iPos], OPCUA_CONFIG_STRING_SIZE - iPos, "%s:%i\\", "iSecureListener_ThreadPool_MinThreads", (OpcUa_Int)OpcUa_ProxyStub_g_Configuration.iSecureListener_ThreadPool_MinThreads);
    if(iRes > 0){iPos += iRes;}else{OpcUa_GotoErrorWithStatus(OpcUa_BadOutOfMemory);}
    iRes = OpcUa_SnPrintfA(&OpcUa_ProxyStub_g_pConfigString[iPos], OPCUA_CONFIG_STRING_SIZE - iPos, "%s:%i\\", "iSecureListener_ThreadPool_MaxThreads", (OpcUa_Int)OpcUa_ProxyStub_g_Configuration.iSecureListener_ThreadPool_MaxThreads);
    if(iRes > 0){iPos += iRes;}else{OpcUa_GotoErrorWithStatus(OpcUa_BadOutOfMemory);}
    iRes = OpcUa_SnPrintfA(&OpcUa_ProxyStub_g_pConfigString[iPos], OPCUA_CONFIG_STRING_SIZE - iPos, "%s:%i\\", "iSecureListener_ThreadPool_MaxJobs", (int)OpcUa_ProxyStub_g_Configuration.iSecureListener_ThreadPool_MaxJobs);
    if(iRes > 0){iPos += iRes;}else{OpcUa_GotoErrorWithStatus(OpcUa_BadOutOfMemory);}
    iRes = OpcUa_SnPrintfA(&OpcUa_ProxyStub_g_pConfigString[iPos], OPCUA_CONFIG_STRING_SIZE - iPos, "%s:%u\\", "bSecureListener_ThreadPool_BlockOnAdd", (OpcUa_ProxyStub_g_Configuration.bSecureListener_ThreadPool_BlockOnAdd != 0)?1:0);
    if(iRes > 0){iPos += iRes;}else{OpcUa_GotoErrorWithStatus(OpcUa_BadOutOfMemory);}
    iRes = OpcUa_SnPrintfA(&OpcUa_ProxyStub_g_pConfigString[iPos], OPCUA_CONFIG_STRING_SIZE - iPos, "%s:%u\\", "uSecureListener_ThreadPool_Timeout", (OpcUa_Int)OpcUa_ProxyStub_g_Configuration.uSecureListener_ThreadPool_Timeout);
    if(iRes > 0){iPos += iRes;}else{OpcUa_GotoErrorWithStatus(OpcUa_BadOutOfMemory);}
    iRes = OpcUa_SnPrintfA(&OpcUa_ProxyStub_g_pConfigString[iPos], OPCUA_CONFIG_STRING_SIZE - iPos, "%s:%u\\", "bTcpListener_ClientThreadsEnabled", (OpcUa_ProxyStub_g_Configuration.bTcpListener_ClientThreadsEnabled != 0)?1:0);
    if(iRes > 0){iPos += iRes;}else{OpcUa_GotoErrorWithStatus(OpcUa_BadOutOfMemory);}
    iRes = OpcUa_SnPrintfA(&OpcUa_ProxyStub_g_pConfigString[iPos], OPCUA_CONFIG_STRING_SIZE - iPos, "%s:%i\\", "iTcpListener_DefaultChunkSize", (OpcUa_Int)OpcUa_ProxyStub_g_Configuration.iTcpListener_DefaultChunkSize);
    if(iRes > 0){iPos += iRes;}else{OpcUa_GotoErrorWithStatus(OpcUa_BadOutOfMemory);}
    iRes = OpcUa_SnPrintfA(&OpcUa_ProxyStub_g_pConfigString[iPos], OPCUA_CONFIG_STRING_SIZE - iPos, "%s:%i\\", "iTcpConnection_DefaultChunkSize", (OpcUa_Int)OpcUa_ProxyStub_g_Configuration.iTcpConnection_DefaultChunkSize);
    if(iRes > 0){iPos += iRes;}else{OpcUa_GotoErrorWithStatus(OpcUa_BadOutOfMemory);}
    iRes = OpcUa_SnPrintfA(&OpcUa_ProxyStub_g_pConfigString[iPos], OPCUA_CONFIG_STRING_SIZE - iPos, "%s:%i\\", "iTcpTransport_MaxMessageLength", (OpcUa_Int)OpcUa_ProxyStub_g_Configuration.iTcpTransport_MaxMessageLength);
    if(iRes > 0){iPos += iRes;}else{OpcUa_GotoErrorWithStatus(OpcUa_BadOutOfMemory);}
    iRes = OpcUa_SnPrintfA(&OpcUa_ProxyStub_g_pConfigString[iPos], OPCUA_CONFIG_STRING_SIZE - iPos, "%s:%i\\", "iTcpTransport_MaxChunkCount", (OpcUa_Int)OpcUa_ProxyStub_g_Configuration.iTcpTransport_MaxChunkCount);
    if(iRes > 0){iPos += iRes;}else{OpcUa_GotoErrorWithStatus(OpcUa_BadOutOfMemory);}
    iRes = OpcUa_SnPrintfA(&OpcUa_ProxyStub_g_pConfigString[iPos], OPCUA_CONFIG_STRING_SIZE - iPos, "%s:%u\\", "bTcpStream_ExpectWriteToBlock", (OpcUa_ProxyStub_g_Configuration.bTcpStream_ExpectWriteToBlock != 0)?1:0);
    if(iRes > 0){iPos += iRes;}else{OpcUa_GotoErrorWithStatus(OpcUa_BadOutOfMemory);}

#endif /* OPCUA_USE_SAFE_FUNCTIONS */

#if OPCUA_USE_SYNCHRONISATION
    OpcUa_Mutex_Unlock(OpcUa_ProxyStub_g_hGlobalsMutex);
#endif /* OPCUA_USE_SYNCHRONISATION */

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_ProxyStub_ReInitialize
 *===========================================================================*/
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_ProxyStub_ReInitialize(OpcUa_ProxyStubConfiguration* a_pProxyStubConfiguration)
{
OpcUa_InitializeStatus(OpcUa_Module_ProxyStub, "ReInitialize");

#if OPCUA_USE_SYNCHRONISATION
        OpcUa_Mutex_Lock(OpcUa_ProxyStub_g_hGlobalsMutex);
#endif /* OPCUA_USE_SYNCHRONISATION */

    OpcUa_GotoErrorIfArgumentNull(a_pProxyStubConfiguration);

    if((OpcUa_ProxyStub_g_uNoOfChannels != 0) || (OpcUa_ProxyStub_g_uNoOfEndpoints != 0))
    {
        OpcUa_GotoErrorWithStatus(OpcUa_BadInvalidState);
    }

    /* set global configuration object */
    OpcUa_ProxyStub_g_Configuration = *a_pProxyStubConfiguration;

    /* check for negative values and fall back to default values in opcua_config.h */
    if(OpcUa_ProxyStub_g_Configuration.iSerializer_MaxAlloc == -1)
    {
        OpcUa_ProxyStub_g_Configuration.iSerializer_MaxAlloc                     = OPCUA_SERIALIZER_MAXALLOC; /* currently unused */
    }
    if(OpcUa_ProxyStub_g_Configuration.iSerializer_MaxStringLength == -1)
    {
        OpcUa_ProxyStub_g_Configuration.iSerializer_MaxStringLength              = OPCUA_ENCODER_MAXSTRINGLENGTH;
    }
    if(OpcUa_ProxyStub_g_Configuration.iSerializer_MaxByteStringLength == -1)
    {
        OpcUa_ProxyStub_g_Configuration.iSerializer_MaxByteStringLength          = OPCUA_ENCODER_MAXBYTESTRINGLENGTH;
    }
    if(OpcUa_ProxyStub_g_Configuration.iSerializer_MaxArrayLength == -1)
    {
        OpcUa_ProxyStub_g_Configuration.iSerializer_MaxArrayLength               = OPCUA_ENCODER_MAXARRAYLENGTH;
    }
    if(OpcUa_ProxyStub_g_Configuration.iSerializer_MaxMessageSize == -1)
    {
        OpcUa_ProxyStub_g_Configuration.iSerializer_MaxMessageSize               = OPCUA_ENCODER_MAXMESSAGELENGTH;
    }
    if(OpcUa_ProxyStub_g_Configuration.iSecureListener_ThreadPool_MinThreads == -1)
    {
        OpcUa_ProxyStub_g_Configuration.iSecureListener_ThreadPool_MinThreads    = OPCUA_SECURELISTENER_THREADPOOL_MINTHREADS;
    }
    if(OpcUa_ProxyStub_g_Configuration.iSecureListener_ThreadPool_MaxThreads == -1)
    {
        OpcUa_ProxyStub_g_Configuration.iSecureListener_ThreadPool_MaxThreads    = OPCUA_SECURELISTENER_THREADPOOL_MAXTHREADS;
    }
    if(OpcUa_ProxyStub_g_Configuration.iSecureListener_ThreadPool_MaxJobs == -1)
    {
        OpcUa_ProxyStub_g_Configuration.iSecureListener_ThreadPool_MaxJobs       = OPCUA_SECURELISTENER_THREADPOOL_MAXJOBS;
    }
    if(OpcUa_ProxyStub_g_Configuration.iTcpListener_DefaultChunkSize == -1)
    {
        OpcUa_ProxyStub_g_Configuration.iTcpListener_DefaultChunkSize            = OPCUA_TCPLISTENER_DEFAULTCHUNKSIZE;
    }
    if(OpcUa_ProxyStub_g_Configuration.iTcpConnection_DefaultChunkSize == -1)
    {
        OpcUa_ProxyStub_g_Configuration.iTcpConnection_DefaultChunkSize          = OPCUA_TCPCONNECTION_DEFAULTCHUNKSIZE;
    }
    if(OpcUa_ProxyStub_g_Configuration.iTcpTransport_MaxChunkCount == -1)
    {
        OpcUa_ProxyStub_g_Configuration.iTcpTransport_MaxChunkCount              = 0;
    }
    if(OpcUa_ProxyStub_g_Configuration.iTcpTransport_MaxMessageLength == -1)
    {
        OpcUa_ProxyStub_g_Configuration.iTcpTransport_MaxMessageLength           = OPCUA_ENCODER_MAXMESSAGELENGTH;
    }

#if OPCUA_USE_SYNCHRONISATION
        OpcUa_Mutex_Unlock(OpcUa_ProxyStub_g_hGlobalsMutex);
#endif /* OPCUA_USE_SYNCHRONISATION */

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

#if OPCUA_USE_SYNCHRONISATION
    OpcUa_Mutex_Unlock(OpcUa_ProxyStub_g_hGlobalsMutex);
#endif /* OPCUA_USE_SYNCHRONISATION */

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_ProxyStub_Initialize
 *===========================================================================*/
#if !OPCUA_USE_STATIC_PLATFORM_INTERFACE
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_ProxyStub_Initialize(OpcUa_Handle                  a_pPlatformLayerCalltable,
                                                          OpcUa_ProxyStubConfiguration* a_pProxyStubConfiguration)
#else /* !OPCUA_USE_STATIC_PLATFORM_INTERFACE */
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_ProxyStub_Initialize(OpcUa_ProxyStubConfiguration* a_pProxyStubConfiguration)
#endif /* !OPCUA_USE_STATIC_PLATFORM_INTERFACE */
{
    OpcUa_Boolean bSkip = OpcUa_False;
	OpcUa_Int32 iCount=0;
OpcUa_InitializeStatus(OpcUa_Module_ProxyStub, "Initialize");

    OpcUa_ReturnErrorIfArgumentNull(a_pProxyStubConfiguration)
    OpcUa_ReturnErrorIfArgumentNull(a_pPlatformLayerCalltable);

    /* set global platform layer handle */
    OpcUa_ProxyStub_g_PlatformLayerCalltable = (OpcUa_Port_CallTable *)a_pPlatformLayerCalltable;

#if OPCUA_USE_SYNCHRONISATION
    if(OpcUa_ProxyStub_g_hGlobalsMutex == OpcUa_Null)
    {
        uStatus = OpcUa_Mutex_Create(&OpcUa_ProxyStub_g_hGlobalsMutex);
        OpcUa_GotoErrorIfBad(uStatus);
    }
#endif /* OPCUA_USE_SYNCHRONISATION */

#if OPCUA_USE_SYNCHRONISATION
    OpcUa_Mutex_Lock(OpcUa_ProxyStub_g_hGlobalsMutex);
#endif /* OPCUA_USE_SYNCHRONISATION */

    OpcUa_ProxyStub_g_uNoOfInits++;

    if(OpcUa_ProxyStub_g_uNoOfInits > 1)
    {
        bSkip = OpcUa_True;
    }

#if OPCUA_USE_SYNCHRONISATION
    OpcUa_Mutex_Unlock(OpcUa_ProxyStub_g_hGlobalsMutex);
#endif /* OPCUA_USE_SYNCHRONISATION */

    if(bSkip == OpcUa_False)
    {
        /* set global configuration object */
        uStatus = OpcUa_ProxyStub_ReInitialize(a_pProxyStubConfiguration);
        OpcUa_GotoErrorIfBad(uStatus);

        /* initialize networking. */
        OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_ProxyStub_Initialize: Network Module...\n");
		uStatus =OpcUa_Socket_InitializeNetwork();
        //uStatus = OPCUA_P_INITIALIZENETWORK();
        OpcUa_GotoErrorIfBad(uStatus);
        OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_ProxyStub_Initialize: Network Module done!\n");
		
        uStatus = OpcUa_EncodeableTypeTable_Create(&OpcUa_ProxyStub_g_EncodeableTypes);
        OpcUa_GotoErrorIfBad(uStatus);

        uStatus = OpcUa_EncodeableTypeTable_AddTypes(&OpcUa_ProxyStub_g_EncodeableTypes, iCount, OpcUa_KnownEncodeableTypes);
        OpcUa_GotoErrorIfBad(uStatus);

        OpcUa_StringTable_Initialize(&OpcUa_ProxyStub_g_NamespaceUris);
        uStatus = OpcUa_StringTable_AddStringList(&OpcUa_ProxyStub_g_NamespaceUris, OpcUa_ProxyStub_StandardNamespaceUris,OpcUa_False);
        OpcUa_GotoErrorIfBad(uStatus);
		
    }

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;

    OpcUa_ProxyStub_Clear();

OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_ProxyStub_Clear
 *===========================================================================*/
OpcUa_Void OPCUA_DLLCALL OpcUa_ProxyStub_Clear()
{
    OpcUa_Boolean bSkip = OpcUa_False;

    if(OpcUa_ProxyStub_g_PlatformLayerCalltable == OpcUa_Null)
    {
        /* error */
        return;
    }
    else
    {
#if OPCUA_USE_SYNCHRONISATION
        OpcUa_Mutex_Lock(OpcUa_ProxyStub_g_hGlobalsMutex);
#endif /* OPCUA_USE_SYNCHRONISATION */

        OpcUa_ProxyStub_g_uNoOfInits--;

        if(OpcUa_ProxyStub_g_uNoOfInits > 0)
        {
            bSkip = OpcUa_True;
        }

#if OPCUA_USE_SYNCHRONISATION
        OpcUa_Mutex_Unlock(OpcUa_ProxyStub_g_hGlobalsMutex);
#endif /* OPCUA_USE_SYNCHRONISATION */

        if(bSkip == OpcUa_False)
        {
            if(OpcUa_ProxyStub_g_pConfigString != OpcUa_Null)
            {
                OpcUa_Free(OpcUa_ProxyStub_g_pConfigString);
                OpcUa_ProxyStub_g_pConfigString = OpcUa_Null;
            }

            /* P-Layer resource */
            OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_ProxyStub_Clear: Network Module...\n");
            OPCUA_P_CLEANUPNETWORK();
            OPCUA_P_CLEANUPTIMERS(); /* Forces a stop of all timers not yet deleted. Leads to callbacks! */
#if OPCUA_USE_SYNCHRONISATION
            OpcUa_Mutex_Delete(&OpcUa_ProxyStub_g_hGlobalsMutex);
#endif /* OPCUA_USE_SYNCHRONISATION */
            OpcUa_Trace(OPCUA_TRACE_STACK_LEVEL_ERROR, "OpcUa_ProxyStub_Clear: Network Module done!\n");

#if OPCUA_TRACE_ENABLE 
            /* internal resource */
            OpcUa_Trace_Clear();
#endif /* OPCUA_TRACE_ENABLE */

            OpcUa_EncodeableTypeTable_Delete(&OpcUa_ProxyStub_g_EncodeableTypes);
            OpcUa_StringTable_Clear(&OpcUa_ProxyStub_g_NamespaceUris);

            OpcUa_ProxyStub_g_PlatformLayerCalltable = OpcUa_Null;
        }
    }
}

/*============================================================================
 * OpcUa_ProxyStub_RegisterChannel
 *===========================================================================*/
OpcUa_Void OpcUa_ProxyStub_RegisterChannel(OpcUa_Void)
{
#if OPCUA_USE_SYNCHRONISATION
        OpcUa_Mutex_Lock(OpcUa_ProxyStub_g_hGlobalsMutex);
#endif /* OPCUA_USE_SYNCHRONISATION */

    ++OpcUa_ProxyStub_g_uNoOfChannels;    

#if OPCUA_USE_SYNCHRONISATION
        OpcUa_Mutex_Unlock(OpcUa_ProxyStub_g_hGlobalsMutex);
#endif /* OPCUA_USE_SYNCHRONISATION */
}

/*============================================================================
 * OpcUa_ProxyStub_RegisterEndpoint
 *===========================================================================*/
OpcUa_Void OpcUa_ProxyStub_RegisterEndpoint(OpcUa_Void)
{
#if OPCUA_USE_SYNCHRONISATION
        OpcUa_Mutex_Lock(OpcUa_ProxyStub_g_hGlobalsMutex);
#endif /* OPCUA_USE_SYNCHRONISATION */

    ++OpcUa_ProxyStub_g_uNoOfEndpoints;

#if OPCUA_USE_SYNCHRONISATION
        OpcUa_Mutex_Unlock(OpcUa_ProxyStub_g_hGlobalsMutex);
#endif /* OPCUA_USE_SYNCHRONISATION */
}

/*============================================================================
 * OpcUa_ProxyStub_DeRegisterChannel
 *===========================================================================*/
OpcUa_Void OpcUa_ProxyStub_DeRegisterChannel(OpcUa_Void)
{
#if OPCUA_USE_SYNCHRONISATION
        OpcUa_Mutex_Lock(OpcUa_ProxyStub_g_hGlobalsMutex);
#endif /* OPCUA_USE_SYNCHRONISATION */

    --OpcUa_ProxyStub_g_uNoOfChannels;    

#if OPCUA_USE_SYNCHRONISATION
        OpcUa_Mutex_Unlock(OpcUa_ProxyStub_g_hGlobalsMutex);
#endif /* OPCUA_USE_SYNCHRONISATION */
}

/*============================================================================
 * OpcUa_ProxyStub_DeRegisterEndpoint
 *===========================================================================*/
OpcUa_Void OpcUa_ProxyStub_DeRegisterEndpoint(OpcUa_Void)
{
#if OPCUA_USE_SYNCHRONISATION
        OpcUa_Mutex_Lock(OpcUa_ProxyStub_g_hGlobalsMutex);
#endif /* OPCUA_USE_SYNCHRONISATION */

    --OpcUa_ProxyStub_g_uNoOfEndpoints;

#if OPCUA_USE_SYNCHRONISATION
        OpcUa_Mutex_Unlock(OpcUa_ProxyStub_g_hGlobalsMutex);
#endif /* OPCUA_USE_SYNCHRONISATION */
}

/*============================================================================
 * OpcUa_ProxyStub_AddTypes
 *===========================================================================*/
OpcUa_StatusCode OpcUa_ProxyStub_AddTypes(OpcUa_EncodeableType** a_ppTypes)
{
	OpcUa_Int32 iCount=0;
OpcUa_InitializeStatus(OpcUa_Module_ProxyStub, "AddTypes");
        
    uStatus = OpcUa_EncodeableTypeTable_AddTypes(   &OpcUa_ProxyStub_g_EncodeableTypes, 
													iCount,
                                                    a_ppTypes);
    OpcUa_GotoErrorIfBad(uStatus);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_ProxyStub_SetNamespaceUris
 *===========================================================================*/
OpcUa_StatusCode OpcUa_ProxyStub_SetNamespaceUris(OpcUa_StringA* a_psNamespaceUris,
                                                  OpcUa_Boolean  a_bMakeCopy)
{
OpcUa_InitializeStatus(OpcUa_Module_ProxyStub, "SetNamespaceUris");
    
    /* discard existing strings */
    OpcUa_StringTable_Clear(&OpcUa_ProxyStub_g_NamespaceUris);
    
    /* update table */
    uStatus = OpcUa_StringTable_AddStringList(  &OpcUa_ProxyStub_g_NamespaceUris, 
                                                a_psNamespaceUris,
												a_bMakeCopy);
    OpcUa_GotoErrorIfBad(uStatus);

OpcUa_ReturnStatusCode;
OpcUa_BeginErrorHandling;
OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_ProxyStub_GetVersion
 *===========================================================================*/
OpcUa_StringA OPCUA_DLLCALL OpcUa_ProxyStub_GetVersion()
{
    return OpcUa_ProxyStub_g_VersionString;
}

/*============================================================================
 * OpcUa_ProxyStub_GetConfigString
 *===========================================================================*/
OpcUa_StringA OPCUA_DLLCALL OpcUa_ProxyStub_GetConfigString()
{
    if(OpcUa_ProxyStub_g_uNoOfInits == 0)
    {
        return (OpcUa_StringA)"ProxyStub not initialized!";
    }
    else
    {
        if(OpcUa_IsBad(OpcUa_ProxyStub_UpdateConfigString()))
        {
            return (OpcUa_StringA)"Could not update ConfigString!";
        }
        else
        {
            return OpcUa_ProxyStub_g_pConfigString;
        }
    }
}

/*============================================================================
 * OpcUa_ProxyStub_GetStaticConfigString
 *===========================================================================*/
OpcUa_StringA OPCUA_DLLCALL OpcUa_ProxyStub_GetStaticConfigString()
{
    return OpcUa_ProxyStub_g_StaticConfigString;
}
