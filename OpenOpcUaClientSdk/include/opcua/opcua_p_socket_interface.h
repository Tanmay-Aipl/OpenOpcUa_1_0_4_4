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

#ifndef _OpcUa_Socket_Interface_H_
#define _OpcUa_Socket_Interface_H_ 1

OPCUA_BEGIN_EXTERN_C

/*============================================================================
 * Create a new socket manager or initialize the global one (OpcUa_Null first).
 *===========================================================================*/
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_P_SocketManager_Create(        OpcUa_SocketManager*    pSocketManager,
                                                                    OpcUa_UInt32            nSockets,
                                                                    OpcUa_UInt32            nFlags);

/*============================================================================
 *
 *===========================================================================*/
OpcUa_Void OPCUA_DLLCALL OpcUa_P_SocketManager_Delete(              OpcUa_SocketManager*    pSocketManager);

/*============================================================================
 * Create a server socket
 *===========================================================================*/
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_P_SocketManager_CreateServer(  OpcUa_SocketManager         pSocketManager,
                                                                    OpcUa_StringA               sAddress,
                                                                    OpcUa_ByteString*           pServerCertificate,
                                                                    OpcUa_Key*                  pServerPrivateKey,
                                                                    OpcUa_Void*                 pPKIConfig,
                                                                    OpcUa_Socket_EventCallback  pfnSocketCallBack,
                                                                    OpcUa_Void*                 pCallbackData,
                                                                    OpcUa_Socket*               pSocket);

/*============================================================================
 * Create a client socket
 *===========================================================================*/
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_P_SocketManager_CreateClient(  OpcUa_SocketManager                 a_hSocketManager,
                                                                    OpcUa_StringA                       sRemoteAddress,
                                                                    OpcUa_UInt16                        uLocalPort,
                                                                    OpcUa_ByteString*                   pClientCertificate,
                                                                    OpcUa_Key*                          pClientPrivateKey,
                                                                    OpcUa_Void*                         pPKIConfig,
                                                                    OpcUa_Socket_EventCallback          pfnSocketCallBack,
                                                                    OpcUa_Socket_CertificateCallback    pfnCertificateCallback,
                                                                    OpcUa_Void*                         pCallbackData,
                                                                    OpcUa_Socket*                       pSocket);

/*============================================================================
 * Signal a certain event on a socket.
 *===========================================================================*/
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_P_SocketManager_SignalEvent(   OpcUa_SocketManager pSocketManager,
                                                                    OpcUa_UInt32        uEvent,
                                                                    OpcUa_Boolean       bAllManagers);

/*============================================================================
 *
 *===========================================================================*/
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_P_SocketManager_ServeLoop(     OpcUa_SocketManager     pSocketManager,
                                                                    OpcUa_UInt32            msecTimeout,
                                                                    OpcUa_Boolean           bRunOnce);

/*============================================================================
 * Break server loop(s) and issue event(s).
 *===========================================================================*/
#if 1
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_P_SocketManager_InterruptLoop( OpcUa_SocketManager pSocketManager,
                                                                    OpcUa_UInt32        uEvent,
                                                                    OpcUa_Boolean       bAllManagers);
#endif

/*============================================================================
 * Get last socket error
 *===========================================================================*/
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_Socket_GetLastError(         OpcUa_Socket    pSocket);

/*============================================================================
 * Read Socket.
 *===========================================================================*/
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_P_Socket_Read(                 OpcUa_Socket    pSocket,
                                                                    OpcUa_Byte*     pBuffer,
                                                                    OpcUa_UInt32    nBufferSize,
                                                                    OpcUa_UInt32*   pBytesRead);

/*============================================================================
 * Write Socket.
 *===========================================================================*/
OpcUa_Int32 OPCUA_DLLCALL OpcUa_P_Socket_Write(                     OpcUa_Socket    pSocket,
                                                                    OpcUa_Byte*     pBuffer,
                                                                    OpcUa_UInt32    uBufferSize,
                                                                    OpcUa_Boolean   bBlock);
/*============================================================================
 * Close Socket and release all ressources.
 *===========================================================================*/
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_P_Socket_Delete(               OpcUa_Socket pSocket);

/*============================================================================
 * Shutdown Socket.
 *===========================================================================*/
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_P_Socket_Shutdown(             OpcUa_Socket pSocket);

/*============================================================================
 * Close Socket.
 *===========================================================================*/
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_P_Socket_Close(                OpcUa_Socket pSocket);

#if OPCUA_P_SOCKETGETPEERINFO_V2
/*============================================================================
 * Get IP Address and Port Number of the Peer
 *===========================================================================*/
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_P_Socket_GetPeerInfo(          OpcUa_Socket RawSocket,
                                                                    OpcUa_CharA* achPeerInfoBuffer,
                                                                    OpcUa_UInt32 uiPeerInfoBufferSize);

#else /* OPCUA_P_SOCKETGETPEERINFO_V2 */
/*============================================================================
 * Get IP Address and Port Number of the Peer
 *===========================================================================*/
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_P_Socket_GetPeerInfo(          OpcUa_Socket  pSocket,
                                                                    OpcUa_UInt32* pIP,
                                                                    OpcUa_UInt16* pPort);
#endif /* OPCUA_P_SOCKETGETPEERINFO_V2 */

/*============================================================================
 * Initialize the platform network interface
 *===========================================================================*/
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_Socket_InitializeNetwork(void);

/*============================================================================
 * Clean the platform network interface up.
 *===========================================================================*/
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_Socket_CleanupNetwork(void);

/*============================================================================
 * Convert a text encoded internet address to its binary representation.
 *===========================================================================*/
OpcUa_UInt32 OPCUA_DLLCALL OpcUa_P_Socket_InetAddr(                 OpcUa_StringA sRemoteAddress);

/*============================================================================
 * Get the name of the local host.
 *===========================================================================*/
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_P_Socket_GetHostName(          OpcUa_CharA* pBuffer,
                                                                    OpcUa_UInt32 uiBufferLength);

OPCUA_END_EXTERN_C

#endif /* _OpcUa_Socket_Interface_H_ */
