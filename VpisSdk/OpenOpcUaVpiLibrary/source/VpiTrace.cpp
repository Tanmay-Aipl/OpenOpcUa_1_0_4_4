//**************************************************************************
//
//  Copyright (c) 4CE Industry 1999-2014, All Rights Reserved
//
//**************************************************************************
//
//  Filename   :  VpiTrace.cpp
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

#include "VpiTrace.h"
using namespace VpiBuiltinType;
#if !VPI_USE_STATIC_PLATFORM_INTERFACE
#define VPI_P_TRACE_CLEAR         Vpi_ProxyStub_g_PlatformLayerCalltable->TraceClear
#endif
Vpi_ProxyStubConfiguration*        Vpi_ProxyStub_g_Configuration=Vpi_Null;

Vpi_UInt32 g_iDefaultTraceLevel=0;
Vpi_UInt32 g_iDefaultTraceOutput=0;
/*============================================================================
 * Trace Lock
 *===========================================================================*/
/**
* Global Trace Buffer.
*/
Vpi_CharA Vpi_Trace_g_aTraceBuffer[VPI_TRACE_MAXLENGTH];

/*============================================================================
 * Trace Lock
 *===========================================================================*/
/**
* Global Mutex to synchronize access to the trace device.
*/
#if VPI_USE_SYNCHRONISATION
Vpi_Mutex Vpi_Trace_s_pLock = Vpi_Null;
#endif /* VPI_USE_SYNCHRONISATION */


/*============================================================================
 * Trace Initialize
 *===========================================================================*/
/**
* Initialize all ressources needed for tracing.
*/
//Vpi_StatusCode VPI_DLLCALL Vpi_Trace_Initialize(Vpi_Void)
Vpi_StatusCode VPI_DLLCALL Vpi_Trace_Initialize(void* pProxyStubConfiguration,Vpi_Int32 iTraceLevel, Vpi_Int32 iTraceOutput, Vpi_String TraceFileName, FILE** hTraceFile)
{
	Vpi_StatusCode    uStatus = Vpi_Good;
#if VPI_USE_SYNCHRONISATION
	uStatus = Vpi_Mutex_Create(&Vpi_Trace_s_pLock);
	Vpi_ReturnErrorIfBad(uStatus);
#endif /* VPI_USE_SYNCHRONISATION */
	if (pProxyStubConfiguration)
	{
		Vpi_ProxyStubConfiguration* pProxyStubConfiguration = (Vpi_ProxyStubConfiguration*)pProxyStubConfiguration;
		if (strlen(TraceFileName.strContent)==0)
		{
			uStatus=Vpi_BadInvalidArgument;
		}
		else
		{
			Vpi_Trace_SetTraceFile(pProxyStubConfiguration, TraceFileName);
			// save the default value for the trace
			Vpi_Trace_SetTraceLevel(pProxyStubConfiguration, iTraceLevel);
			Vpi_Trace_SetTraceLevel(pProxyStubConfiguration, iTraceOutput);
		}
	}
	#ifdef _GNUC_
	#ifdef _USE_SYSLOG_

	#endif
	#endif
	return uStatus;
}
Vpi_StatusCode VPI_DLLCALL Vpi_Trace_InitializePtr(void** ppProxyStubConfiguration)
{
	Vpi_StatusCode uStatus = Vpi_Good;
	if (!(*ppProxyStubConfiguration))
	{
		(*ppProxyStubConfiguration) = (Vpi_ProxyStubConfiguration*)Vpi_Alloc(sizeof(Vpi_ProxyStubConfiguration));
		ZeroMemory((*ppProxyStubConfiguration), sizeof(Vpi_ProxyStubConfiguration));
	}
	else
		uStatus = Vpi_BadNothingToDo;
	return uStatus;
}
/*============================================================================
 * Trace Clear
 *===========================================================================*/
/**
* Clear all ressources needed for tracing.
*/
Vpi_Void VPI_DLLCALL Vpi_Trace_Clear(void* pProxyStubConfiguration)
{
	Vpi_ProxyStubConfiguration* pLocalProxyStubConfiguration = (Vpi_ProxyStubConfiguration*)pProxyStubConfiguration;
	if (pLocalProxyStubConfiguration)
	{
		free(pLocalProxyStubConfiguration);
		pLocalProxyStubConfiguration = Vpi_Null;
#if VPI_USE_SYNCHRONISATION
		Vpi_Mutex_Delete(&Vpi_Trace_s_pLock);
#endif /* VPI_USE_SYNCHRONISATION */
	}
}



/*============================================================================
 * Change Trace Level
 *===========================================================================*/
/** 
 * Activate or deactivate trace output during runtime.
 * @param a_uNewTraceLevel Description
 */
Vpi_Void VPI_DLLCALL Vpi_Trace_ChangeTraceLevel(void* pProxyStubConfiguration, Vpi_UInt32 a_uNewTraceLevel)
{
	Vpi_ProxyStubConfiguration* pLocalProxyStubConfiguration = (Vpi_ProxyStubConfiguration*)pProxyStubConfiguration;
	if (pLocalProxyStubConfiguration)
	{
#if VPI_USE_SYNCHRONISATION
		if(Vpi_Trace_s_pLock == Vpi_Null)
		{
			return;
		}
		Vpi_Mutex_Lock(Vpi_Trace_s_pLock);
#endif /* VPI_USE_SYNCHRONISATION */
		Vpi_ProxyStub_g_Configuration = pLocalProxyStubConfiguration;
		if (!Vpi_ProxyStub_g_Configuration)
		{
			Vpi_ProxyStub_g_Configuration = (Vpi_ProxyStubConfiguration*)Vpi_Alloc(sizeof(Vpi_ProxyStubConfiguration));
			ZeroMemory(Vpi_ProxyStub_g_Configuration, sizeof(Vpi_ProxyStubConfiguration));
			Vpi_ProxyStub_g_Configuration->uProxyStub_Trace_Level = a_uNewTraceLevel;
		}
#if VPI_USE_SYNCHRONISATION
		Vpi_Mutex_Unlock(Vpi_Trace_s_pLock);
#endif /* VPI_USE_SYNCHRONISATION */
	}
	return;
}
// initialize the TraceFileName. Set strFileName in the globalTraceFileName g_StrTraceFileName
Vpi_Void VPI_DLLCALL Vpi_Trace_SetTraceFile(void* pProxyStubConfiguration, Vpi_String strFileName)
{	
	Vpi_ProxyStubConfiguration* pLocalProxyStubConfiguration = (Vpi_ProxyStubConfiguration*)pProxyStubConfiguration;
	if (pLocalProxyStubConfiguration)
	{
		Vpi_ProxyStub_g_Configuration = pLocalProxyStubConfiguration;
		if (Vpi_String_StrLen(&strFileName) > 0)
		{
			if (Vpi_String_StrLen(&(Vpi_ProxyStub_g_Configuration->StrProxyStub_Trace_FileName)) > 0)
				Vpi_String_Clear(&(Vpi_ProxyStub_g_Configuration->StrProxyStub_Trace_FileName));
			Vpi_String_Initialize(&(Vpi_ProxyStub_g_Configuration->StrProxyStub_Trace_FileName));
			Vpi_String_CopyTo(&strFileName, &(Vpi_ProxyStub_g_Configuration->StrProxyStub_Trace_FileName));
			Vpi_ProxyStub_g_Configuration->uProxyStub_Trace_Output = VPI_TRACE_OUTPUT_FILE;
		}
	}
	return;
}
// Retrieve the TraceFileName and sent it back in strFileName
Vpi_Void VPI_DLLCALL Vpi_Trace_GetTraceFile(void* pProxyStubConfiguration, Vpi_String* strFileName)
{
	Vpi_ProxyStubConfiguration* pLocalProxyStubConfiguration = (Vpi_ProxyStubConfiguration*)pProxyStubConfiguration;
	if (pLocalProxyStubConfiguration)
	{
		Vpi_ProxyStub_g_Configuration = pLocalProxyStubConfiguration;
		if (Vpi_String_StrLen(&(Vpi_ProxyStub_g_Configuration->StrProxyStub_Trace_FileName)) > 0)
			Vpi_String_CopyTo(&(Vpi_ProxyStub_g_Configuration->StrProxyStub_Trace_FileName), strFileName);
	}
	return;
}
Vpi_UInt32 VPI_DLLCALL Vpi_Trace_GetTraceLevel(void* pProxyStubConfiguration)
{
	Vpi_ProxyStubConfiguration* pLocalProxyStubConfiguration = (Vpi_ProxyStubConfiguration*)pProxyStubConfiguration;
	if (pLocalProxyStubConfiguration)
		return g_iDefaultTraceLevel;
	else
		; // default
}
Vpi_Void VPI_DLLCALL Vpi_Trace_SetTraceLevel(void* pProxyStubConfiguration, Vpi_UInt32 iTraceLevel)
{
	Vpi_ProxyStubConfiguration* pLocalProxyStubConfiguration = (Vpi_ProxyStubConfiguration*)pProxyStubConfiguration;
	if (pLocalProxyStubConfiguration)
	{
		Vpi_ProxyStub_g_Configuration = pLocalProxyStubConfiguration;
#if VPI_USE_SYNCHRONISATION
		if(Vpi_Trace_s_pLock == Vpi_Null)
			return;
		Vpi_Mutex_Lock(Vpi_Trace_s_pLock);
#endif /* VPI_USE_SYNCHRONISATION */
		if (!Vpi_ProxyStub_g_Configuration)
		{
			Vpi_ProxyStub_g_Configuration = (Vpi_ProxyStubConfiguration*)Vpi_Alloc(sizeof(Vpi_ProxyStubConfiguration));
			ZeroMemory(Vpi_ProxyStub_g_Configuration, sizeof(Vpi_ProxyStubConfiguration));
		}
		Vpi_ProxyStub_g_Configuration->uProxyStub_Trace_Level = iTraceLevel;
		g_iDefaultTraceLevel = iTraceLevel;

#if VPI_USE_SYNCHRONISATION
		Vpi_Mutex_Unlock(Vpi_Trace_s_pLock);
#endif /* VPI_USE_SYNCHRONISATION */
	}
}
Vpi_UInt32 VPI_DLLCALL Vpi_Trace_GetTraceOutput(void* pProxyStubConfiguration)
{
	return g_iDefaultTraceOutput;
}
Vpi_Void VPI_DLLCALL Vpi_Trace_SetTraceOutput(void* pProxyStubConfiguration, Vpi_UInt32 iTraceOutput)
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
// Vpi_UInt32     a_uTraceLevel contient le code demandé par l'appelant
// Vpi_CharA*		a_sFormat chaine de caractère a formater
Vpi_Boolean VPI_DLLCALL Vpi_Trace(void* pProxyStubConfiguration, 
										Vpi_UInt32       a_uTraceLevel,
										const Vpi_CharA* a_sFormat,
										...)
{
	Vpi_Boolean bTraced = Vpi_False;
	Vpi_ProxyStubConfiguration* pLocalProxyStubConfiguration = (Vpi_ProxyStubConfiguration*)pProxyStubConfiguration;
	if (pLocalProxyStubConfiguration)
	{
#if VPI_USE_SYNCHRONISATION
		if(Vpi_Trace_s_pLock == Vpi_Null)
		{
			return Vpi_False;
		}
		Vpi_Mutex_Lock(Vpi_Trace_s_pLock);
#endif /* VPI_USE_SYNCHRONISATION */
		Vpi_ProxyStub_g_Configuration = pLocalProxyStubConfiguration;
		if (Vpi_ProxyStub_g_Configuration)
		{
			/* check if app wants trace output */
			if (Vpi_ProxyStub_g_Configuration->uProxyStub_Trace_Output == VPI_TRACE_OUTPUT_NONE)
			{
#if VPI_USE_SYNCHRONISATION
				Vpi_Mutex_Unlock(Vpi_Trace_s_pLock);
#endif /* VPI_USE_SYNCHRONISATION */
				return Vpi_False;
			}
			// Vpi_ProxyStub_g_Configuration->uProxyStub_Trace_Level contient le code 
			// demandé par défaut dans le fichier de configuration
			if ((a_uTraceLevel & Vpi_ProxyStub_g_Configuration->uProxyStub_Trace_Level) || (a_uTraceLevel == VPI_TRACE_LEVEL_ALWAYS))
			{
				Vpi_P_VA_List argumentList;

				VPI_P_VA_START(argumentList, a_sFormat);

				/* write trace buffer */
#ifdef _WIN32_WCE
				_vsnprintf( Vpi_Trace_g_aTraceBuffer,/*VPI_P_STRINGA_VSNPRINTF*/
					VPI_TRACE_MAXLENGTH - 1,
					(const Vpi_StringA)a_sFormat,
					argumentList);
#else
				vsnprintf(Vpi_Trace_g_aTraceBuffer,/*VPI_P_STRINGA_VSNPRINTF*/
					VPI_TRACE_MAXLENGTH - 1,
					(const Vpi_StringA)a_sFormat,
					argumentList);
#endif
				if ((a_uTraceLevel == VPI_TRACE_LEVEL_ALWAYS) && (Vpi_ProxyStub_g_Configuration->uProxyStub_Trace_Output != VPI_TRACE_OUTPUT_CONSOLE))
					printf("OpenOpcUa:%u> %s", (Vpi_UInt)Vpi_Thread_GetCurrentThreadId(), Vpi_Trace_g_aTraceBuffer);
				//else
				Vpi_Trace_g_aTraceBuffer[VPI_TRACE_MAXLENGTH - 1] = '\0';

				/* send trace buffer to platform trace device */
				Vpi_Trace_Internal(pProxyStubConfiguration,Vpi_Trace_g_aTraceBuffer);

				bTraced = Vpi_True;

				VPI_P_VA_END(argumentList);
			}

		}
#if VPI_USE_SYNCHRONISATION
		Vpi_Mutex_Unlock(Vpi_Trace_s_pLock);
#endif /* VPI_USE_SYNCHRONISATION */
	}
	return bTraced;
}
/*============================================================================
 * Tracefunction
 *===========================================================================*/
/**
 * Writes the given string to the trace device, if the given trace level is
 * activated in the header file.
 */
Vpi_Void VPI_DLLCALL Vpi_Trace_Internal(void* pProxyStubConfiguration, Vpi_CharA* a_sMessage)
{
#ifdef _WIN32_WCE
	wchar_t *pwString = NULL;
#endif

	char dtbuffer[26] = {'\0'};
	Vpi_ProxyStubConfiguration* pLocalProxyStubConfiguration = (Vpi_ProxyStubConfiguration*)pProxyStubConfiguration;
	if (!pLocalProxyStubConfiguration)
		return;
	Vpi_DateTime_GetStringFromDateTime(Vpi_DateTime_UtcNow(), dtbuffer, 25);
	dtbuffer[25] = '\0';
	if (Vpi_ProxyStub_g_Configuration)
	{
		if (Vpi_ProxyStub_g_Configuration->uProxyStub_Trace_Output == VPI_TRACE_OUTPUT_FILE)
		{
			if (Vpi_ProxyStub_g_Configuration->hProxyStub_OutFile == Vpi_Null)
				Vpi_ProxyStub_g_Configuration->hProxyStub_OutFile = fopen(Vpi_String_GetRawString(&(Vpi_ProxyStub_g_Configuration->StrProxyStub_Trace_FileName)), "a");
			//
			if (Vpi_ProxyStub_g_Configuration->hProxyStub_OutFile != Vpi_Null)
			{
				fprintf(Vpi_ProxyStub_g_Configuration->hProxyStub_OutFile, "|%u| %s %s", (Vpi_UInt)Vpi_Thread_GetCurrentThreadId(), &dtbuffer[11], a_sMessage);
#if VPI_P_TRACE_FFLUSH_IMMEDIATELY
				fflush(Vpi_ProxyStub_g_Configuration->hProxyStub_OutFile);
#endif
				Vpi_ProxyStub_g_Configuration->hOutFileNoOfEntries++;
				/* delete backup store and rename current file and create new one */
				//fflush(Vpi_ProxyStub_g_Configuration->hProxyStub_OutFile);
				if (Vpi_ProxyStub_g_Configuration->hProxyStub_OutFile)
					fclose(Vpi_ProxyStub_g_Configuration->hProxyStub_OutFile);
			}
			Vpi_ProxyStub_g_Configuration->hProxyStub_OutFile = Vpi_Null;
			if (Vpi_ProxyStub_g_Configuration->hOutFileNoOfEntries >= Vpi_ProxyStub_g_Configuration->hOutFileNoOfEntriesMax)
				Vpi_ProxyStub_g_Configuration->hOutFileNoOfEntries = 0;
		}
		if ((Vpi_ProxyStub_g_Configuration->uProxyStub_Trace_Output == VPI_TRACE_OUTPUT_CONSOLE))
		{
			printf("OpenOpcUa:%u> %s", (Vpi_UInt)Vpi_Thread_GetCurrentThreadId(), a_sMessage);
		}
	}
}