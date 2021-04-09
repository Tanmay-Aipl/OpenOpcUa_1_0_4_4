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

//#include <opcua_mutex.h>
#include <opcua_thread.h>

#include <opcua_trace.h>
#include <opcua_core.h>
#if !OPCUA_USE_STATIC_PLATFORM_INTERFACE
#define OPCUA_P_TRACE_CLEAR         OpcUa_ProxyStub_g_PlatformLayerCalltable->TraceClear
#endif
//#define OPCUA_P_STRINGA_VSNPRINTF   OpcUa_ProxyStub_g_PlatformLayerCalltable->StrVsnPrintf

OpcUa_UInt32 g_iDefaultTraceLevel=0;
OpcUa_UInt32 g_iDefaultTraceOutput=0;
/*============================================================================
 * Trace buffer
 *===========================================================================*/
/**
* Global Trace Buffer.
*/
OpcUa_CharA OpcUa_Trace_g_aTraceBuffer[OPCUA_TRACE_MAXLENGTH];

/*============================================================================
 * Trace Lock
 *===========================================================================*/
/**
* Global Mutex to synchronize access to the trace device.
*/
#if OPCUA_USE_SYNCHRONISATION
OpcUa_Mutex OpcUa_Trace_s_pLock = OpcUa_Null;
#endif /* OPCUA_USE_SYNCHRONISATION */


/*============================================================================
 * Trace Initialize
 *===========================================================================*/
/**
* Initialize all ressources needed for tracing.
*/
//OpcUa_StatusCode OPCUA_DLLCALL OpcUa_Trace_Initialize(OpcUa_Void)
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_Trace_Initialize(OpcUa_Int32 iTraceLevel, OpcUa_Int32 iTraceOutput,OpcUa_String TraceFileName, FILE** hTraceFile )
{
    OpcUa_StatusCode    uStatus = OpcUa_Good;
#if OPCUA_USE_SYNCHRONISATION
	uStatus = OpcUa_Mutex_Create(&OpcUa_Trace_s_pLock);
	OpcUa_ReturnErrorIfBad(uStatus);
#endif /* OPCUA_USE_SYNCHRONISATION */
	if (OpcUa_String_StrLen(&TraceFileName)==0)
	{
		uStatus=OpcUa_BadInvalidArgument;
	}
	else
	{
		OpcUa_Trace_SetTraceFile(TraceFileName);

		//uStatus = OPCUA_P_TRACE_INITIALIZE();
		// save the default value for the trace
		OpcUa_Trace_SetTraceLevel(iTraceLevel);
		OpcUa_Trace_SetTraceLevel(iTraceOutput);
		if (iTraceOutput==OPCUA_TRACE_OUTPUT_FILE)
		{
			*hTraceFile = fopen(OpcUa_String_GetRawString(&TraceFileName), "a");
		}
	//#if OPCUA_P_TRACE_TO_FILE
	//    OpcUa_P_Trace_g_hOutFile = fopen(g_StrTraceFileName, "w");
	//#endif /* OPCUA_P_TRACE_TO_FILE */


	#ifdef _GNUC_
	#ifdef _USE_SYSLOG_

	#endif
	#endif
	}
    return uStatus;
}

/*============================================================================
 * Trace Clear
 *===========================================================================*/
/**
* Clear all ressources needed for tracing.
*/
OpcUa_Void OPCUA_DLLCALL OpcUa_Trace_Clear(OpcUa_Void)
{
	if (OpcUa_String_StrLen(&(OpcUa_ProxyStub_g_Configuration.StrProxyStub_Trace_FileName))>0)
		OpcUa_String_Clear(&(OpcUa_ProxyStub_g_Configuration.StrProxyStub_Trace_FileName));
#if OPCUA_USE_SYNCHRONISATION
    OpcUa_Mutex_Delete(&OpcUa_Trace_s_pLock);
#endif /* OPCUA_USE_SYNCHRONISATION */
}

/*============================================================================
 * Change Trace Level
 *===========================================================================*/
/** 
 * Activate or deactivate trace output during runtime.
 * @param a_uNewTraceLevel Description
 */
OpcUa_Void OPCUA_DLLCALL OpcUa_Trace_ChangeTraceLevel(OpcUa_UInt32 a_uNewTraceLevel)
{
#if OPCUA_USE_SYNCHRONISATION
    if(OpcUa_Trace_s_pLock == OpcUa_Null)
    {
        return;
    }
    OpcUa_Mutex_Lock(OpcUa_Trace_s_pLock);
#endif /* OPCUA_USE_SYNCHRONISATION */

    OpcUa_ProxyStub_g_Configuration.uProxyStub_Trace_Level = a_uNewTraceLevel;

#if OPCUA_USE_SYNCHRONISATION
    OpcUa_Mutex_Unlock(OpcUa_Trace_s_pLock);
#endif /* OPCUA_USE_SYNCHRONISATION */

    return;
}
// initialize the TraceFileName. Set strFileName in the globalTraceFileName g_StrTraceFileName
OpcUa_Void OPCUA_DLLCALL OpcUa_Trace_SetTraceFile(OpcUa_String strFileName)
{	
	if (OpcUa_String_StrLen(&strFileName)>0)
	{
		if (OpcUa_String_StrLen(&(OpcUa_ProxyStub_g_Configuration.StrProxyStub_Trace_FileName))>0)
			OpcUa_String_Clear(&(OpcUa_ProxyStub_g_Configuration.StrProxyStub_Trace_FileName));
		OpcUa_String_CopyTo(&strFileName,&(OpcUa_ProxyStub_g_Configuration.StrProxyStub_Trace_FileName));
		OpcUa_ProxyStub_g_Configuration.uProxyStub_Trace_Output=OPCUA_TRACE_OUTPUT_FILE;
	}
	return;
}
// Retrieve the TraceFileName and sent it back in strFileName
OpcUa_Void OPCUA_DLLCALL OpcUa_Trace_GetTraceFile(OpcUa_String* strFileName)
{
	if (OpcUa_String_StrLen(&(OpcUa_ProxyStub_g_Configuration.StrProxyStub_Trace_FileName))>0)
		OpcUa_String_CopyTo(&(OpcUa_ProxyStub_g_Configuration.StrProxyStub_Trace_FileName), strFileName);
	return;
}
OpcUa_UInt32 OPCUA_DLLCALL OpcUa_Trace_GetTraceLevel()
{
	return g_iDefaultTraceLevel;
}
OpcUa_Void OPCUA_DLLCALL OpcUa_Trace_SetTraceLevel(OpcUa_UInt32 iTraceLevel)
{
#if OPCUA_USE_SYNCHRONISATION
    if(OpcUa_Trace_s_pLock == OpcUa_Null)
            return;
    OpcUa_Mutex_Lock(OpcUa_Trace_s_pLock);
#endif /* OPCUA_USE_SYNCHRONISATION */
	
	OpcUa_ProxyStub_g_Configuration.uProxyStub_Trace_Level = iTraceLevel;
	g_iDefaultTraceLevel=iTraceLevel;

#if OPCUA_USE_SYNCHRONISATION
    OpcUa_Mutex_Unlock(OpcUa_Trace_s_pLock);
#endif /* OPCUA_USE_SYNCHRONISATION */
}
OpcUa_UInt32 OPCUA_DLLCALL OpcUa_Trace_GetTraceOutput()
{
	return g_iDefaultTraceOutput;
}
OpcUa_Void OPCUA_DLLCALL OpcUa_Trace_SetTraceOutput(OpcUa_UInt32 iTraceOutput)
{
	g_iDefaultTraceOutput=iTraceOutput;
}
/*============================================================================
 * Tracefunction
 *===========================================================================*/
/**
* Writes the given string and the parameters to the trace device, if the given 
* trace level is activated.
*/
// OpcUa_UInt32     a_uTraceLevel contient le code demandé par l'appelant
// OpcUa_CharA*		a_sFormat chaine de caractère a formater
OpcUa_Boolean OPCUA_DLLCALL OpcUa_Trace(OpcUa_UInt32       a_uTraceLevel,
                                            const OpcUa_CharA* a_sFormat,
                                            ...)
{
    OpcUa_Boolean bTraced = OpcUa_False;

#if OPCUA_USE_SYNCHRONISATION
    if(OpcUa_Trace_s_pLock == OpcUa_Null)
    {
        return OpcUa_False;
    }
    OpcUa_Mutex_Lock(OpcUa_Trace_s_pLock);
#endif /* OPCUA_USE_SYNCHRONISATION */

    /* check if app wants trace output */
    if(OpcUa_ProxyStub_g_Configuration.uProxyStub_Trace_Output == OPCUA_TRACE_OUTPUT_NONE)
    {
#if OPCUA_USE_SYNCHRONISATION
        OpcUa_Mutex_Unlock(OpcUa_Trace_s_pLock);
#endif /* OPCUA_USE_SYNCHRONISATION */
        return OpcUa_False;
    }
	// OpcUa_ProxyStub_g_Configuration.uProxyStub_Trace_Level contient le code 
	// demandé par défaut dans le fichier de configuration
    if ( (a_uTraceLevel & OpcUa_ProxyStub_g_Configuration.uProxyStub_Trace_Level) || (a_uTraceLevel==OPCUA_TRACE_LEVEL_ALWAYS))
    {
        OpcUa_P_VA_List argumentList;

        OPCUA_P_VA_START(argumentList, a_sFormat);

        /* write trace buffer */
#ifdef _WIN32_WCE
        _vsnprintf( OpcUa_Trace_g_aTraceBuffer,/*OPCUA_P_STRINGA_VSNPRINTF*/
                   OPCUA_TRACE_MAXLENGTH - 1,
                   (const OpcUa_StringA)a_sFormat,
                   argumentList);
#else
        vsnprintf( OpcUa_Trace_g_aTraceBuffer,/*OPCUA_P_STRINGA_VSNPRINTF*/
                   OPCUA_TRACE_MAXLENGTH - 1,
                   (const OpcUa_StringA)a_sFormat,
                   argumentList);
#endif
		if ( (a_uTraceLevel==OPCUA_TRACE_LEVEL_ALWAYS)  && (OpcUa_ProxyStub_g_Configuration.uProxyStub_Trace_Output!=OPCUA_TRACE_OUTPUT_CONSOLE) )
			printf("OpenOpcUa:%u> %s", (OpcUa_UInt)OpcUa_Thread_GetCurrentThreadId(), OpcUa_Trace_g_aTraceBuffer);
		//else
			OpcUa_Trace_g_aTraceBuffer[OPCUA_TRACE_MAXLENGTH - 1] = '\0';		
		
			/* send trace buffer to platform trace device */
		OpcUa_Trace_Internal(OpcUa_Trace_g_aTraceBuffer);
		
        bTraced = OpcUa_True;

        OPCUA_P_VA_END(argumentList);
    }

#if OPCUA_USE_SYNCHRONISATION
        OpcUa_Mutex_Unlock(OpcUa_Trace_s_pLock);
#endif /* OPCUA_USE_SYNCHRONISATION */

    return bTraced;
}
/*============================================================================
 * Tracefunction
 *===========================================================================*/
/**
 * Writes the given string to the trace device, if the given trace level is
 * activated in the header file.
 */
OpcUa_Void OPCUA_DLLCALL OpcUa_Trace_Internal(OpcUa_CharA* a_sMessage)
{
#ifdef _WIN32_WCE
	wchar_t *pwString = NULL;
#endif

    char dtbuffer[26] = {'\0'};

    OpcUa_DateTime_GetStringFromDateTime(OpcUa_DateTime_UtcNow(), dtbuffer, 25);
    dtbuffer[25] = '\0';

	if (OpcUa_ProxyStub_g_Configuration.uProxyStub_Trace_Output==OPCUA_TRACE_OUTPUT_FILE)
	{
		if(OpcUa_ProxyStub_g_Configuration.hProxyStub_OutFile == OpcUa_Null)
			OpcUa_ProxyStub_g_Configuration.hProxyStub_OutFile = fopen(OpcUa_String_GetRawString(&(OpcUa_ProxyStub_g_Configuration.StrProxyStub_Trace_FileName)), "a");
		//
		if(OpcUa_ProxyStub_g_Configuration.hProxyStub_OutFile != OpcUa_Null)
		{
			fprintf(OpcUa_ProxyStub_g_Configuration.hProxyStub_OutFile, "|%u| %s %s", (OpcUa_UInt)OpcUa_Thread_GetCurrentThreadId(), &dtbuffer[0], a_sMessage);
	#if OPCUA_P_TRACE_FFLUSH_IMMEDIATELY
			fflush(OpcUa_ProxyStub_g_Configuration.hProxyStub_OutFile);
	#endif
			OpcUa_ProxyStub_g_Configuration.hOutFileNoOfEntries++;
			/* delete backup store and rename current file and create new one */
			fflush(OpcUa_ProxyStub_g_Configuration.hProxyStub_OutFile);
			fclose(OpcUa_ProxyStub_g_Configuration.hProxyStub_OutFile);	
			OpcUa_ProxyStub_g_Configuration.hProxyStub_OutFile = OpcUa_Null;
			if(OpcUa_ProxyStub_g_Configuration.hOutFileNoOfEntries >= OpcUa_ProxyStub_g_Configuration.hOutFileNoOfEntriesMax)
				OpcUa_ProxyStub_g_Configuration.hOutFileNoOfEntries = 0;
		}
	}
	if ( (OpcUa_ProxyStub_g_Configuration.uProxyStub_Trace_Output==OPCUA_TRACE_OUTPUT_CONSOLE)  )
	{
		printf("OpenOpcUa:%u> %s", (OpcUa_UInt)OpcUa_Thread_GetCurrentThreadId(), a_sMessage);
	}
}