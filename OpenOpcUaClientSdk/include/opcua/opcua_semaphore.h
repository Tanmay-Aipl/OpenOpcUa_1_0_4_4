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
OPCUA_BEGIN_EXTERN_C
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_Semaphore_Create(OpcUa_Semaphore*    phNewSemaphore, 
                                                       OpcUa_UInt32        uInitalValue,
                                                       OpcUa_UInt32        uMaxRange);
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_Semaphore_Delete(OpcUa_Semaphore*    phSemaphore);
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_Semaphore_Wait (OpcUa_Semaphore     hSemaphore);
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_Semaphore_TimedWait(OpcUa_Semaphore     hSemaphore, 
                                                         OpcUa_UInt32        msecTimeout);
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_Semaphore_Post(OpcUa_Semaphore     hSemaphore,
                                                    OpcUa_UInt32        uReleaseCount);
OPCUA_END_EXTERN_C
