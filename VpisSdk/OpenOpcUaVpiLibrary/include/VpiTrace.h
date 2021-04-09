//**************************************************************************
//
//  Copyright (c) 4CE Industry 1999-2014, All Rights Reserved
//
//**************************************************************************
//
//  Filename   :  AddItemDlg.cpp
//  $Author    :  Michel Condemine
//
//  Description:  This file is part of the OpenOpcUaLibrary. 
//                This compoment and all related file are not Open Source
//				  This must be use with the autorisation of Michel Condemine
//**************************************************************************

#ifndef _Vpi_Trace_H_
#define _Vpi_Trace_H_ 1

VPI_BEGIN_EXTERN_C
/*============================================================================
 * va_list definitions
 *===========================================================================*/
typedef va_list Vpi_P_VA_List;

#define VPI_P_VA_START(ap,v)  va_start(ap,v)
#define VPI_P_VA_END(ap)      va_end(ap)

/*============================================================================
* Trace Levels
*===========================================================================*/
/* Output for the trace */
#define VPI_TRACE_OUTPUT_FILE			0x00010000
#define VPI_TRACE_OUTPUT_CONSOLE		0x00010001
#define VPI_TRACE_OUTPUT_NONE			0x00010002
/* predefined trace levels */
#define VPI_TRACE_LEVEL_ALWAYS        0x00002000 /* Always code. permet de toujours afficher un message 2^13 (8192)*/
// Trace message from the stack
// 
#define VPI_TRACE_STACK_NONE			0x00000004 // 0
#define VPI_TRACE_STACK_INFO			0x00000002 // 2^1
#define VPI_TRACE_STACK_WARNING		0x00000004 // 2^2
#define VPI_TRACE_STACK_ERROR			0x00000008 // 2^3
// 
// Trace message from the server 
// 
#define VPI_TRACE_SERVER_NONE			0x00000000 // 0
#define VPI_TRACE_SERVER_INFO			0x00000010 // 2^4
#define VPI_TRACE_SERVER_WARNING		0x00000020 // 2^5
#define VPI_TRACE_SERVER_ERROR		0x00000040 // 2^6
// 
// Trace message from the client
// 
#define VPI_TRACE_CLIENT_NONE			0x00000000 // 0
#define VPI_TRACE_CLIENT_INFO			0x00000080 // 2^7
#define VPI_TRACE_CLIENT_WARNING		0x00000100 // 2^8
#define VPI_TRACE_CLIENT_ERROR		0x00000200 // 2^9 
// 
// Trace message from the extra componenet. This include Lua, Xml and SharedLibserver
// 
#define VPI_TRACE_EXTRA_NONE			0x00000000 // 0
#define VPI_TRACE_EXTRA_INFO			0x00000400 // 2^10
#define VPI_TRACE_EXTRA_WARNING		0x00000800 // 2^11
#define VPI_TRACE_EXTRA_ERROR			0x00001000 // 2^12 
// 
/* trace level packages */
// 
// trace used in the function VPI_TRACE
#define VPI_TRACE_STACK_LEVEL_DEBUG   (VPI_TRACE_STACK_WARNING | VPI_TRACE_STACK_INFO | VPI_TRACE_STACK_ERROR )
#define VPI_TRACE_STACK_LEVEL_ERROR   (VPI_TRACE_STACK_ERROR)
#define VPI_TRACE_STACK_LEVEL_WARNING (VPI_TRACE_STACK_WARNING)
#define VPI_TRACE_STACK_LEVEL_INFO    (VPI_TRACE_STACK_INFO )

#define VPI_TRACE_SERVER_LEVEL_DEBUG   (VPI_TRACE_SERVER_WARNING | VPI_TRACE_SERVER_INFO | VPI_TRACE_SERVER_ERROR )
#define VPI_TRACE_SERVER_LEVEL_ERROR   (VPI_TRACE_SERVER_ERROR)
#define VPI_TRACE_SERVER_LEVEL_WARNING (VPI_TRACE_SERVER_WARNING)
#define VPI_TRACE_SERVER_LEVEL_INFO    (VPI_TRACE_SERVER_INFO )

#define VPI_TRACE_CLIENT_LEVEL_DEBUG   (VPI_TRACE_CLIENT_WARNING | VPI_TRACE_CLIENT_INFO | VPI_TRACE_CLIENT_ERROR )
#define VPI_TRACE_CLIENT_LEVEL_ERROR   (VPI_TRACE_CLIENT_ERROR)
#define VPI_TRACE_CLIENT_LEVEL_WARNING (VPI_TRACE_CLIENT_WARNING)
#define VPI_TRACE_CLIENT_LEVEL_INFO    (VPI_TRACE_CLIENT_INFO )

#define VPI_TRACE_EXTRA_LEVEL_DEBUG   (VPI_TRACE_EXTRA_WARNING | VPI_TRACE_EXTRA_INFO | VPI_TRACE_EXTRA_ERROR )
#define VPI_TRACE_EXTRA_LEVEL_ERROR   (VPI_TRACE_EXTRA_ERROR)
#define VPI_TRACE_EXTRA_LEVEL_WARNING (VPI_TRACE_EXTRA_WARNING)
#define VPI_TRACE_EXTRA_LEVEL_INFO    (VPI_TRACE_EXTRA_INFO )
////////////////////////////////////////////////////////////////////
// Use to setup the trace itself
#define VPI_TRACE_OUTPUT_ALL_ERROR		(VPI_TRACE_STACK_LEVEL_DEBUG |  VPI_TRACE_SERVER_LEVEL_DEBUG | VPI_TRACE_CLIENT_LEVEL_DEBUG | VPI_TRACE_EXTRA_LEVEL_DEBUG)
#define VPI_TRACE_OUTPUT_STACK_ERROR		VPI_TRACE_STACK_LEVEL_ERROR
#define VPI_TRACE_OUTPUT_SERVER_ERROR		VPI_TRACE_SERVER_LEVEL_ERROR
#define VPI_TRACE_OUTPUT_CLIENT_ERROR		VPI_TRACE_CLIENT_ERROR
#define VPI_TRACE_OUTPUT_EXTRA_ERROR		VPI_TRACE_EXTRA_LEVEL_ERROR

#define VPI_TRACE_OUTPUT_ALL_DEBUG		(VPI_TRACE_STACK_LEVEL_DEBUG |  VPI_TRACE_SERVER_LEVEL_DEBUG | VPI_TRACE_EXTRA_LEVEL_DEBUG)
#define VPI_TRACE_OUTPUT_STACK_DEBUG		VPI_TRACE_STACK_LEVEL_DEBUG
#define VPI_TRACE_OUTPUT_SERVER_DEBUG		VPI_TRACE_SERVER_LEVEL_DEBUG
#define VPI_TRACE_OUTPUT_EXTRA_DEBUG		VPI_TRACE_EXTRA_LEVEL_DEBUG

#define VPI_TRACE_OUTPUT_ALL_WARNING		(VPI_TRACE_STACK_LEVEL_WARNING |  VPI_TRACE_SERVER_LEVEL_WARNING | VPI_TRACE_EXTRA_LEVEL_WARNING)
#define VPI_TRACE_OUTPUT_STACK_WARNING	VPI_TRACE_STACK_LEVEL_WARNING
#define VPI_TRACE_OUTPUT_SERVER_WARNING	VPI_TRACE_SERVER_LEVEL_WARNING
#define VPI_TRACE_OUTPUT_EXTRA_WARNING	VPI_TRACE_EXTRA_LEVEL_WARNING

#define VPI_TRACE_OUTPUT_ALL_INFO			(VPI_TRACE_STACK_LEVEL_INFO | VPI_TRACE_SERVER_LEVEL_INFO | VPI_TRACE_EXTRA_LEVEL_INFO)
#define VPI_TRACE_OUTPUT_STACK_INFO		VPI_TRACE_STACK_LEVEL_INFO
#define VPI_TRACE_OUTPUT_SERVER_INFO		VPI_TRACE_SERVER_LEVEL_INFO
#define VPI_TRACE_OUTPUT_EXTRA_INFO		VPI_TRACE_EXTRA_LEVEL_INFO

#define VPI_TRACE_OUTPUT_LEVEL_ALL     (0xFFFFFFFF)
#define VPI_TRACE_OUTPUT_LEVEL_NONE    (0x00000000)
/*============================================================================
 * Trace Initialize
 *===========================================================================*/
/**
* Initialize all resources needed for tracing.
*/
VPILIBRARY_EXPORT Vpi_StatusCode VPI_DLLCALL Vpi_Trace_InitializePtr(void** ppProxyStubConfiguration);
VPILIBRARY_EXPORT Vpi_StatusCode VPI_DLLCALL Vpi_Trace_Initialize(void* pProxyStubConfiguration,Vpi_Int32 iTraceLevel, Vpi_Int32 iTraceOutput, Vpi_String TraceFileName, FILE** hTraceFile);

/*============================================================================
 * Trace Initialize
 *===========================================================================*/
/**
* Clear all resources needed for tracing.
*/
VPILIBRARY_EXPORT Vpi_Void VPI_DLLCALL Vpi_Trace_Clear(void* pProxyStubConfiguration);

/*============================================================================
 * Change Trace Level
 *===========================================================================*/
/**
 * Activate or deactivate trace output during runtime.
 */
VPILIBRARY_EXPORT Vpi_Void VPI_DLLCALL Vpi_Trace_ChangeTraceLevel(void* pProxyStubConfiguration, Vpi_UInt32 a_uNewTraceLevel);

/*============================================================================
 * Activate/Deactivate Trace 
 *===========================================================================*/
/**
 * Activate or deactivate trace output during runtime.
 */
//VPILIBRARY_EXPORT Vpi_Void VPI_DLLCALL Vpi_Trace_Toggle(Vpi_Boolean a_bActive);
/*============================================================================
 * Change the TraceFileName during runtime.
 */
VPILIBRARY_EXPORT Vpi_Void VPI_DLLCALL Vpi_Trace_SetTraceFile(void* pProxyStubConfiguration, Vpi_String strFileName);

/*============================================================================
 * Get the current TraceFileName during runtime.
 */
VPILIBRARY_EXPORT Vpi_Void VPI_DLLCALL Vpi_Trace_GetTraceFile(void* pProxyStubConfiguration, Vpi_String* strFileName);

/*============================================================================
 * Handle the default traceLevel
 */
VPILIBRARY_EXPORT Vpi_UInt32 VPI_DLLCALL Vpi_Trace_GetTraceLevel(void* pProxyStubConfiguration );
VPILIBRARY_EXPORT Vpi_Void VPI_DLLCALL Vpi_Trace_SetTraceLevel(void* pProxyStubConfiguration, Vpi_UInt32 iTraceLevel);

/*============================================================================
 * Handle the default traceOutput
 */
VPILIBRARY_EXPORT Vpi_UInt32 VPI_DLLCALL Vpi_Trace_GetTraceOutput(void* pProxyStubConfiguration);
VPILIBRARY_EXPORT Vpi_Void VPI_DLLCALL Vpi_Trace_SetTraceOutput(void* pProxyStubConfiguration, Vpi_UInt32 iTraceOutput);
/*============================================================================
 * Tracefunction
 *===========================================================================*/
/**
* @brief Writes the given string and the parameters to the trace device, if the given 
* trace level is activated in the header file.
*
* @see Vpi_P_Trace
*
* @return The number of bytes written to the trace device.
*/

VPILIBRARY_EXPORT Vpi_Boolean VPI_DLLCALL Vpi_Trace(void* pProxyStubConfiguration,
    Vpi_UInt32 uTraceLevel, 
    const Vpi_CharA* sFormat,
    ...);

VPI_END_EXTERN_C
Vpi_Void VPI_DLLCALL Vpi_Trace_Internal(void* pProxyStubConfiguration, Vpi_CharA* a_sMessage);
#endif /* _Vpi_Trace_H_ */
