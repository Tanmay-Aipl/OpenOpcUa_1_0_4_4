/* ========================================================================
 * Copyright (c) 2005-2009 The OPC Foundation, Inc. All rights reserved.
 *
 * OPC Foundation MIT License 1.00
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
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
 *
 * The complete license agreement can be found here:
 * http://opcfoundation.org/License/MIT/1.00/
 * ======================================================================*/

#ifndef _OpcUa_Trace_H_
#define _OpcUa_Trace_H_ 1

OPCUA_BEGIN_EXTERN_C

/*============================================================================
 * Trace Levels
 *===========================================================================*/
/* Output for the trace */
#define OPCUA_TRACE_OUTPUT_FILE			0x00010000
#define OPCUA_TRACE_OUTPUT_CONSOLE		0x00010001
#define OPCUA_TRACE_OUTPUT_NONE			0x00010002
/* predefined trace levels */
#define OPCUA_TRACE_LEVEL_ALWAYS        0x00002000 /* Always code. permet de toujours afficher un message 2^13 (8192)*/
// Trace message from the stack
// 
#define OPCUA_TRACE_STACK_NONE			0x00000004 // 0
#define OPCUA_TRACE_STACK_INFO			0x00000002 // 2^1
#define OPCUA_TRACE_STACK_WARNING		0x00000004 // 2^2
#define OPCUA_TRACE_STACK_ERROR			0x00000008 // 2^3
// 
// Trace message from the server 
// 
#define OPCUA_TRACE_SERVER_NONE			0x00000000 // 0
#define OPCUA_TRACE_SERVER_INFO			0x00000010 // 2^4
#define OPCUA_TRACE_SERVER_WARNING		0x00000020 // 2^5
#define OPCUA_TRACE_SERVER_ERROR		0x00000040 // 2^6
// 
// Trace message from the client
// 
#define OPCUA_TRACE_CLIENT_NONE			0x00000000 // 0
#define OPCUA_TRACE_CLIENT_INFO			0x00000080 // 2^7
#define OPCUA_TRACE_CLIENT_WARNING		0x00000100 // 2^8
#define OPCUA_TRACE_CLIENT_ERROR		0x00000200 // 2^9 
// 
// Trace message from the extra componenet. This include Lua, Xml and SharedLibserver
// 
#define OPCUA_TRACE_EXTRA_NONE			0x00000000 // 0
#define OPCUA_TRACE_EXTRA_INFO			0x00000400 // 2^10
#define OPCUA_TRACE_EXTRA_WARNING		0x00000800 // 2^11
#define OPCUA_TRACE_EXTRA_ERROR			0x00001000 // 2^12 
// 
/* trace level packages */
// 
// trace used in the function OPCUA_TRACE
#define OPCUA_TRACE_STACK_LEVEL_DEBUG   (OPCUA_TRACE_STACK_WARNING | OPCUA_TRACE_STACK_INFO | OPCUA_TRACE_STACK_ERROR )
#define OPCUA_TRACE_STACK_LEVEL_ERROR   (OPCUA_TRACE_STACK_ERROR)
#define OPCUA_TRACE_STACK_LEVEL_WARNING (OPCUA_TRACE_STACK_WARNING)
#define OPCUA_TRACE_STACK_LEVEL_INFO    (OPCUA_TRACE_STACK_INFO )

#define OPCUA_TRACE_SERVER_LEVEL_DEBUG   (OPCUA_TRACE_SERVER_WARNING | OPCUA_TRACE_SERVER_INFO | OPCUA_TRACE_SERVER_ERROR )
#define OPCUA_TRACE_SERVER_LEVEL_ERROR   (OPCUA_TRACE_SERVER_ERROR)
#define OPCUA_TRACE_SERVER_LEVEL_WARNING (OPCUA_TRACE_SERVER_WARNING)
#define OPCUA_TRACE_SERVER_LEVEL_INFO    (OPCUA_TRACE_SERVER_INFO )

#define OPCUA_TRACE_CLIENT_LEVEL_DEBUG   (OPCUA_TRACE_CLIENT_WARNING | OPCUA_TRACE_CLIENT_INFO | OPCUA_TRACE_CLIENT_ERROR )
#define OPCUA_TRACE_CLIENT_LEVEL_ERROR   (OPCUA_TRACE_CLIENT_ERROR)
#define OPCUA_TRACE_CLIENT_LEVEL_WARNING (OPCUA_TRACE_CLIENT_WARNING)
#define OPCUA_TRACE_CLIENT_LEVEL_INFO    (OPCUA_TRACE_CLIENT_INFO )

#define OPCUA_TRACE_EXTRA_LEVEL_DEBUG   (OPCUA_TRACE_EXTRA_WARNING | OPCUA_TRACE_EXTRA_INFO | OPCUA_TRACE_EXTRA_ERROR )
#define OPCUA_TRACE_EXTRA_LEVEL_ERROR   (OPCUA_TRACE_EXTRA_ERROR)
#define OPCUA_TRACE_EXTRA_LEVEL_WARNING (OPCUA_TRACE_EXTRA_WARNING)
#define OPCUA_TRACE_EXTRA_LEVEL_INFO    (OPCUA_TRACE_EXTRA_INFO )
////////////////////////////////////////////////////////////////////
// Use to setup the trace itself
#define OPCUA_TRACE_OUTPUT_ALL_ERROR		(OPCUA_TRACE_STACK_LEVEL_DEBUG |  OPCUA_TRACE_SERVER_LEVEL_DEBUG | OPCUA_TRACE_CLIENT_LEVEL_DEBUG | OPCUA_TRACE_EXTRA_LEVEL_DEBUG)
#define OPCUA_TRACE_OUTPUT_STACK_ERROR		OPCUA_TRACE_STACK_LEVEL_ERROR
#define OPCUA_TRACE_OUTPUT_SERVER_ERROR		OPCUA_TRACE_SERVER_LEVEL_ERROR
#define OPCUA_TRACE_OUTPUT_CLIENT_ERROR		OPCUA_TRACE_CLIENT_ERROR
#define OPCUA_TRACE_OUTPUT_EXTRA_ERROR		OPCUA_TRACE_EXTRA_LEVEL_ERROR

#define OPCUA_TRACE_OUTPUT_ALL_DEBUG		(OPCUA_TRACE_STACK_LEVEL_DEBUG |  OPCUA_TRACE_SERVER_LEVEL_DEBUG | OPCUA_TRACE_EXTRA_LEVEL_DEBUG)
#define OPCUA_TRACE_OUTPUT_STACK_DEBUG		OPCUA_TRACE_STACK_LEVEL_DEBUG
#define OPCUA_TRACE_OUTPUT_SERVER_DEBUG		OPCUA_TRACE_SERVER_LEVEL_DEBUG
#define OPCUA_TRACE_OUTPUT_EXTRA_DEBUG		OPCUA_TRACE_EXTRA_LEVEL_DEBUG

#define OPCUA_TRACE_OUTPUT_ALL_WARNING		(OPCUA_TRACE_STACK_LEVEL_WARNING |  OPCUA_TRACE_SERVER_LEVEL_WARNING | OPCUA_TRACE_EXTRA_LEVEL_WARNING)
#define OPCUA_TRACE_OUTPUT_STACK_WARNING	OPCUA_TRACE_STACK_LEVEL_WARNING
#define OPCUA_TRACE_OUTPUT_SERVER_WARNING	OPCUA_TRACE_SERVER_LEVEL_WARNING
#define OPCUA_TRACE_OUTPUT_EXTRA_WARNING	OPCUA_TRACE_EXTRA_LEVEL_WARNING

#define OPCUA_TRACE_OUTPUT_ALL_INFO			(OPCUA_TRACE_STACK_LEVEL_INFO | OPCUA_TRACE_SERVER_LEVEL_INFO | OPCUA_TRACE_EXTRA_LEVEL_INFO)
#define OPCUA_TRACE_OUTPUT_STACK_INFO		OPCUA_TRACE_STACK_LEVEL_INFO
#define OPCUA_TRACE_OUTPUT_SERVER_INFO		OPCUA_TRACE_SERVER_LEVEL_INFO
#define OPCUA_TRACE_OUTPUT_EXTRA_INFO		OPCUA_TRACE_EXTRA_LEVEL_INFO

#define OPCUA_TRACE_OUTPUT_LEVEL_ALL     (0xFFFFFFFF)
#define OPCUA_TRACE_OUTPUT_LEVEL_NONE    (0x00000000)
/*============================================================================
 * Trace Initialize
 *===========================================================================*/
/**
* Initialize all resources needed for tracing.
*/
// OPCUA_EXPORT OpcUa_StatusCode OPCUA_DLLCALL OpcUa_Trace_Initialize();
OPCUA_EXPORT OpcUa_StatusCode OPCUA_DLLCALL OpcUa_Trace_Initialize(OpcUa_Int32 iTraceLevel, OpcUa_Int32 iTraceOutput,OpcUa_String TraceFileName,FILE** hTraceFile );

/*============================================================================
 * Trace Initialize
 *===========================================================================*/
/**
* Clear all resources needed for tracing.
*/
OPCUA_EXPORT OpcUa_Void OPCUA_DLLCALL OpcUa_Trace_Clear();

/*============================================================================
 * Change Trace Level
 *===========================================================================*/
/**
 * Activate or deactivate trace output during runtime.
 */
OPCUA_EXPORT OpcUa_Void OPCUA_DLLCALL OpcUa_Trace_ChangeTraceLevel(OpcUa_UInt32 a_uNewTraceLevel);

/*============================================================================
 * Activate/Deactivate Trace 
 *===========================================================================*/
/**
 * Activate or deactivate trace output during runtime.
 */
//OPCUA_EXPORT OpcUa_Void OPCUA_DLLCALL OpcUa_Trace_Toggle(OpcUa_Boolean a_bActive);
/*============================================================================
 * Change the TraceFileName during runtime.
 */
OPCUA_EXPORT OpcUa_Void OPCUA_DLLCALL OpcUa_Trace_SetTraceFile(OpcUa_String strFileName);

/*============================================================================
 * Get the current TraceFileName during runtime.
 */
OPCUA_EXPORT OpcUa_Void OPCUA_DLLCALL OpcUa_Trace_GetTraceFile(OpcUa_String* strFileName);

/*============================================================================
 * Handle the default traceLevel
 */
OPCUA_EXPORT OpcUa_UInt32 OPCUA_DLLCALL OpcUa_Trace_GetTraceLevel();
OPCUA_EXPORT OpcUa_Void OPCUA_DLLCALL OpcUa_Trace_SetTraceLevel(OpcUa_UInt32 iTraceLevel);

/*============================================================================
 * Handle the default traceOutput
 */
OPCUA_EXPORT OpcUa_UInt32 OPCUA_DLLCALL OpcUa_Trace_GetTraceOutput();
OPCUA_EXPORT OpcUa_Void OPCUA_DLLCALL OpcUa_Trace_SetTraceOutput(OpcUa_UInt32 iTraceOutput);
/*============================================================================
 * Tracefunction
 *===========================================================================*/
/**
* @brief Writes the given string and the parameters to the trace device, if the given 
* trace level is activated in the header file.
*
* @see OpcUa_P_Trace
*
* @return The number of bytes written to the trace device.
*/

OPCUA_EXPORT OpcUa_Boolean OPCUA_DLLCALL OpcUa_Trace( 
    OpcUa_UInt32 uTraceLevel, 
    const OpcUa_CharA* sFormat,
    ...);

OPCUA_END_EXTERN_C
OpcUa_Void OPCUA_DLLCALL OpcUa_Trace_Internal(OpcUa_CharA* a_sMessage);
#endif /* _OpcUa_Trace_H_ */
