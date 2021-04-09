//**************************************************************************
//
//  Copyright (c) 4CE Industry 1999-2014, All Rights Reserved
//
//**************************************************************************
//
//  Filename   :  VpiSocket.cpp
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
/* System Headers */
#include <VpiOs.h>
/* UA platform definitions */
#include <VpiInternal.h>
/* own headers */
#include <VpiSocket.h>
#include <VpiTrace.h>
using namespace VpiBuiltinType;
#ifdef _MSC_VER
/* this pragma is for win32 */
#pragma warning(disable:4127) /* suppress "conditional expression is constant" in fdset macros */
#pragma warning(disable:4748) /* suppress /GS can not protect parameters and local variables from local buffer overrun because optimizations are disabled in function */
#endif /* _MSC_VER */
#if defined(WIN32) || defined (WIN32_WCE)
#undef max
#endif
#define max(a,b) \
		({ __typeof__ (a) _a = (a); \
		__typeof__ (b) _b = (b); \
		_a > _b ? _a : _b; })

#ifdef WIN32
/*============================================================================
 * Initialize the platform network interface
 *===========================================================================*/
Vpi_StatusCode VPI_DLLCALL Vpi_RawSocket_InitializeNetwork(Vpi_Void)
{
#ifdef _linux
	return Vpi_Good;
#endif
#ifdef WIN32
	WSADATA wsaData;
	int     apiResult   = 0;
	Vpi_StatusCode uStatus = Vpi_Good;
	/* The return value is zero if the operation was successful.
	   Otherwise, the value SOCKET_ERROR is returned, and a specific
	   error number can be retrieved by calling WSAGetLastError. */
	apiResult = WSAStartup(0x202, &wsaData);

	if(apiResult == 0)
		uStatus= Vpi_Good;
	else
	{
		switch (apiResult)
		{
		case WSASYSNOTREADY:
			uStatus = Vpi_BadInternalError;
			break;
		case WSAVERNOTSUPPORTED:
			uStatus = Vpi_BadInternalError;
			break;
		case WSAEINPROGRESS:
			uStatus = Vpi_BadInvalidState;
			break;
		case WSAEPROCLIM:
			uStatus = Vpi_BadOutOfMemory;
			break;
		case WSAEFAULT:
			uStatus = Vpi_BadInvalidArgument;
			break;
		default:
			uStatus = Vpi_BadInternalError;
			break;
		}
	}
	return uStatus;
#endif
}

/*============================================================================
 * Clean the platform network interface up.
 *===========================================================================*/
Vpi_StatusCode Vpi_RawSocket_CleanupNetwork(Vpi_Void)
{
#ifdef WIN32
	WSACleanup();
#endif
	return Vpi_Good;
}
/*============================================================================
 * Shutdown Socket.
 *===========================================================================*/
Vpi_StatusCode Vpi_RawSocket_Shutdown(Vpi_RawSocket a_RawSocket,Vpi_Int       a_iHow)
{
	SOCKET  rawSocket   = (SOCKET)VPI_P_SOCKET_INVALID;
	int     iRetVal     = 0;
	Vpi_StatusCode uStatus=Vpi_Good;
//Vpi_InitializeStatus(Vpi_Module_Socket, "P_Shutdown");

	rawSocket = (SOCKET)a_RawSocket;

	/* shutdown socket */
	iRetVal = shutdown(rawSocket, a_iHow); /* SD_RECEIVE=0, SD_SEND=1, SD_BOTH=2 */

	/* check uStatus */
	if(iRetVal == SOCKET_ERROR)
	{
#ifdef WIN32
		int iLastError = WSAGetLastError();
		switch(iLastError)
		{
			case WSAECONNABORTED:
			case WSAECONNRESET:
				uStatus=(Vpi_BadNotConnected);
				break;
			case WSAEINPROGRESS:
				/*A blocking Windows Sockets 1.1 call is in progress, or the
				service provider is still processing a callback function.*/
				uStatus=(Vpi_GoodCallAgain);
				break;
			case WSAENOTSOCK:
			case WSAEINVAL:
				uStatus=(Vpi_BadInvalidArgument);
				break;
			case WSAENETDOWN:
			case WSAENOTCONN:
			case WSANOTINITIALISED:
				uStatus=(Vpi_BadInvalidState);
				break;
			default:
				uStatus = Vpi_BadUnexpectedError;
				break;
		}
#endif
#ifdef _GNUC_
		switch (errno)
		{
			case EBADF:
				uStatus=(Vpi_BadInvalidArgument);
				break;
			case EINVAL:
				uStatus=(Vpi_BadInvalidArgument);
				break;
			case ENOTCONN:
				uStatus=(Vpi_BadInvalidState);
				break;
			case ENOTSOCK:
				uStatus=(Vpi_BadInvalidState);
				break;
			case ENOBUFS:
				uStatus=(Vpi_BadOutOfMemory);
				break;
			default:
				uStatus = Vpi_BadUnexpectedError;
				break;
		}
#endif
	}

	return uStatus;
}
/*============================================================================
 * Shutdown and Close Socket.
 *===========================================================================*/
Vpi_StatusCode Vpi_RawSocket_Close(Vpi_RawSocket a_RawSocket)
{
	SOCKET winSocket = (SOCKET)VPI_P_SOCKET_INVALID;

	Vpi_StatusCode uStatus=Vpi_Good;

	winSocket = (SOCKET)a_RawSocket;

	/* close socket */
	uStatus=Vpi_RawSocket_Shutdown((Vpi_RawSocket)winSocket, VPI_SOCKET_SD_BOTH);
	if (uStatus != Vpi_Good)
		Vpi_Trace(Vpi_Null, VPI_TRACE_LEVEL_ERROR, "Vpi_RawSocket_Shutdown failed uStatus=0x%05x\n", uStatus);

	uStatus = closesocket(winSocket);

	/* check uStatus */
	if(uStatus == VPI_P_SOCKET_SOCKETERROR)
	{
		uStatus = Vpi_BadCommunicationError;
	}

	return uStatus;
}

/*============================================================================
 * Create Socket.
 *===========================================================================*/
Vpi_StatusCode Vpi_RawSocket_Create(Vpi_RawSocket*    a_pRawSocket,
										Vpi_Boolean       a_bNagleOff,
										Vpi_Boolean       a_bKeepAliveOn,
										Vpi_Boolean*      a_pbIPv6)
{
	Vpi_StatusCode    uStatus     = Vpi_Good;
	int                 iFlag       = 1;
	Vpi_Int           apiResult   = 0;
	SOCKET              rawSocket   = (SOCKET)VPI_P_SOCKET_INVALID;

#if VPI_P_SOCKET_SETTCPRCVBUFFERSIZE || VPI_P_SOCKET_SETTCPSNDBUFFERSIZE
	Vpi_Int           iBufferSize = VPI_P_TCPRCVBUFFERSIZE;
#endif /* VPI_P_SOCKET_SETTCPRCVBUFFERSIZE || VPI_P_SOCKET_SETTCPSNDBUFFERSIZE */

	if (a_pRawSocket)
	{
		if (a_pbIPv6)
		{
			/* create socket through platform API */
			do
			{
				rawSocket = socket(((*a_pbIPv6)?AF_INET6:AF_INET), SOCK_STREAM, 0);
				apiResult = Vpi_RawSocket_GetLastError((Vpi_RawSocket)rawSocket);

				/* check if socket creation was successful */
				if(     rawSocket == VPI_P_SOCKET_INVALID
					||  apiResult != 0)
				{
					uStatus=(Vpi_BadCommunicationError);
					apiResult=0;
				}
				else
				{
					/* try to activate dual stack support (IPv4 and IPv6) by disabling IPV6_V6ONLY */
					if(*a_pbIPv6)
					{
						int no = 0;
						apiResult = setsockopt(rawSocket, IPPROTO_IPV6, IPV6_V6ONLY, (const char*)&no, sizeof(int));
						if(VPI_P_SOCKET_SOCKETERROR == apiResult)
						{
							Vpi_Trace(Vpi_Null, VPI_TRACE_LEVEL_WARNING, "Could not enable dual protocol stack. IPv4 only.\n");
							closesocket(rawSocket);
							*a_pbIPv6 = Vpi_False;
						}
					}
				}
			} while(apiResult);

			/* set socketoptions */
			if ((a_bNagleOff) && (uStatus==Vpi_Good))
			{
				if(VPI_P_SOCKET_SOCKETERROR == setsockopt(rawSocket, IPPROTO_TCP, TCP_NODELAY, (const char*)&iFlag, sizeof(int)))
				{
					Vpi_Trace(Vpi_Null, VPI_TRACE_LEVEL_WARNING, "Could not set nodelay option.\n");
					uStatus=(Vpi_BadCommunicationError);
					if(rawSocket != VPI_P_SOCKET_INVALID)
					{
						Vpi_RawSocket_Close((Vpi_RawSocket)rawSocket);
						*a_pRawSocket = (Vpi_RawSocket)VPI_P_SOCKET_INVALID;
					}
				}
			}
			if (uStatus!=Vpi_BadCommunicationError)
			{
				if(a_bKeepAliveOn)
				{
					/* set socket options */
					if(VPI_P_SOCKET_SOCKETERROR == setsockopt(rawSocket, IPPROTO_TCP,  SO_KEEPALIVE, (const char*)&iFlag, sizeof(int)))
					{
						Vpi_Trace(Vpi_Null, VPI_TRACE_LEVEL_WARNING, "Could not set keepalive option.\n");
						uStatus=(Vpi_BadCommunicationError);
						if(rawSocket != VPI_P_SOCKET_INVALID)
						{
							Vpi_RawSocket_Close((Vpi_RawSocket)rawSocket);
							*a_pRawSocket = (Vpi_RawSocket)VPI_P_SOCKET_INVALID;
						}
					}
				}


			#if VPI_P_SOCKET_SETTCPRCVBUFFERSIZE
				iBufferSize = VPI_P_TCPRCVBUFFERSIZE;
				if(VPI_P_SOCKET_SOCKETERROR == setsockopt(rawSocket, SOL_SOCKET,  SO_RCVBUF, (const char*)&iBufferSize, sizeof(int)))
				{
					uStatus=(Vpi_BadCommunicationError);
					if(rawSocket != VPI_P_SOCKET_INVALID)
					{
						Vpi_RawSocket_Close((Vpi_RawSocket)rawSocket);
						*a_pRawSocket = (Vpi_RawSocket)VPI_P_SOCKET_INVALID;
					}
				}
			#endif /* VPI_P_SOCKET_SETTCPRCVBUFFERSIZE */

			#if VPI_P_SOCKET_SETTCPSNDBUFFERSIZE
				iBufferSize = VPI_P_TCPSNDBUFFERSIZE;
				if(VPI_P_SOCKET_SOCKETERROR == setsockopt(rawSocket, SOL_SOCKET,  SO_SNDBUF, (const char*)&iBufferSize, sizeof(int)))
				{
					uStatus=(Vpi_BadCommunicationError);
					if(rawSocket != VPI_P_SOCKET_INVALID)
					{
						Vpi_RawSocket_Close((Vpi_RawSocket)rawSocket);
						*a_pRawSocket = (Vpi_RawSocket)VPI_P_SOCKET_INVALID;
					}
				}
			#endif /* VPI_P_SOCKET_SETTCPSNDBUFFERSIZE */
				if(rawSocket != VPI_P_SOCKET_INVALID)
					*a_pRawSocket = (Vpi_RawSocket)rawSocket;
			}
		}
		else
			uStatus=Vpi_BadInvalidArgument;
	}
	else
		uStatus=Vpi_BadInvalidArgument;
	return uStatus;

Error:

	if(rawSocket != VPI_P_SOCKET_INVALID)
	{
		Vpi_RawSocket_Close((Vpi_RawSocket)rawSocket);
		*a_pRawSocket = (Vpi_RawSocket)VPI_P_SOCKET_INVALID;
	}

}

/*============================================================================
 * Connect Socket for Client.
 *===========================================================================*/
Vpi_StatusCode Vpi_RawSocket_Connect( Vpi_RawSocket a_RawSocket,
		Vpi_Int16     a_nPort,
		Vpi_StringA   a_sHost)
{
	Vpi_StatusCode uStatus=Vpi_Good;
	int					 intSize   = 0;
	SOCKET               winSocket = (SOCKET)VPI_P_SOCKET_INVALID;
	struct sockaddr*	 pName;
	struct sockaddr_in   srv;
	char*                localhost = "127.0.0.1";

	//Vpi_InitializeStatus(Vpi_Module_Socket, "P_Connect");

	if (a_RawSocket)
	{
		winSocket = (SOCKET)a_RawSocket;

		intSize = sizeof(struct sockaddr_in);
		Vpi_MemSet(&srv, 0, intSize);

		if(!strcmp("localhost", a_sHost))
		{
			a_sHost = localhost;
		}

		srv.sin_addr.s_addr = inet_addr(a_sHost);

		if(srv.sin_addr.s_addr == INADDR_NONE)
			uStatus = Vpi_BadInvalidArgument;
		else
		{
			srv.sin_port   = htons(a_nPort);
			srv.sin_family = AF_INET;

			pName = (struct sockaddr *) &srv;

			if(connect(winSocket, pName, intSize) == VPI_P_SOCKET_SOCKETERROR)
			{
				int result = Vpi_RawSocket_GetLastError((Vpi_RawSocket)winSocket);

				/* a connect takes some time and this "error" is common with nonblocking sockets */
				if(result == WSAEWOULDBLOCK || result == WSAEINPROGRESS)
				{
					uStatus = Vpi_BadWouldBlock;
				}
				else
				{
					uStatus = Vpi_BadCommunicationError;
				}
			}

		}
	}
	else
		uStatus=Vpi_BadInvalidArgument;
	return uStatus;
}

/*============================================================================
 * Bind to Socket
 *===========================================================================*/
Vpi_StatusCode Vpi_RawSocket_Bind(Vpi_RawSocket a_RawSocket,Vpi_Int16 a_nPort,Vpi_Boolean a_bIPv6)
{
	Vpi_StatusCode uStatus=Vpi_Good;
	Vpi_Int32         intSize    = 0;
	SOCKET              winSocket  = (SOCKET)VPI_P_SOCKET_INVALID;
	struct sockaddr_in  srv;
	struct sockaddr_in6 srv6;
	struct sockaddr     *pName;

	//Vpi_InitializeStatus(Vpi_Module_Socket, "P_Bind");

	if (a_RawSocket)
	{
		winSocket = (SOCKET)a_RawSocket;
		if (!a_bIPv6)
		{
			// ipv4
			intSize = sizeof(struct sockaddr_in);
			Vpi_MemSet(&srv, 0, intSize);
			srv.sin_addr.s_addr = INADDR_ANY;
			srv.sin_port        = htons(a_nPort);
			srv.sin_family      = AF_INET;
			pName               = (struct sockaddr*)&srv;
		}
		else
		{
			// ipv6
			intSize = sizeof(struct sockaddr_in6);
			Vpi_MemSet(&srv6, 0, intSize);
			memset(&srv6.sin6_addr.u, 0, sizeof(srv6.sin6_addr.u));
			srv6.sin6_port       = htons(a_nPort);
			srv6.sin6_family     = AF_INET6;
			pName = (struct sockaddr*)&srv6;
		}
		if(bind(winSocket, pName, intSize) == VPI_P_SOCKET_SOCKETERROR)
		{
			uStatus = Vpi_BadCommunicationError;
		}
	}
	else
		uStatus=Vpi_BadInvalidArgument;
	return uStatus;
}


/*============================================================================
 * Bind to Socket and set to listen for Server.
 *===========================================================================*/
Vpi_StatusCode Vpi_RawSocket_Listen(Vpi_RawSocket a_RawSocket)
{
	Vpi_StatusCode uStatus=Vpi_Good;
	SOCKET winSocket  = (SOCKET)VPI_P_SOCKET_INVALID;

	//Vpi_InitializeStatus(Vpi_Module_Socket, "P_Listen");

	winSocket = (SOCKET)a_RawSocket;

	if(listen(winSocket, SOMAXCONN) == VPI_P_SOCKET_SOCKETERROR)
	{
		uStatus = Vpi_BadCommunicationError;		
	}

	return uStatus;
}

/*============================================================================
 * Accept Socket connection from Client.
 *===========================================================================*/
/// <summary>
/// This function make an Accept on a raw socket
/// </summary>
/// <param name="a_RawSocket">The a_ raw socket.</param>
/// <param name="a_pPort">The a_p port.</param>
/// <param name="a_pAddress">The a_p address.</param>
/// <param name="a_bNagleOff">The a_b nagle off.</param>
/// <param name="a_bKeepAliveOn">The a_b keep alive on.</param>
/// <returns>Vpi_RawSocket </returns>
Vpi_RawSocket Vpi_RawSocket_Accept(Vpi_RawSocket a_RawSocket,
									   Vpi_UInt16*   a_pPort,
									   Vpi_UInt32*   a_pAddress,
									   Vpi_Boolean   a_bNagleOff,
									   Vpi_Boolean   a_bKeepAliveOn)
{
	int                 cli_size        = 0;
	int                 iFlag           = 1;
	SOCKET              winSocketServer = (SOCKET)VPI_P_SOCKET_INVALID;
	SOCKET              winSocketClient = (SOCKET)VPI_P_SOCKET_INVALID;
	struct sockaddr_in  cli;

#if VPI_P_SOCKET_SETTCPRCVBUFFERSIZE || VPI_P_SOCKET_SETTCPSNDBUFFERSIZE
	Vpi_Int           iBufferSize = VPI_P_TCPRCVBUFFERSIZE;
#endif /* VPI_P_SOCKET_SETTCPRCVBUFFERSIZE || VPI_P_SOCKET_SETTCPSNDBUFFERSIZE */

	if(a_RawSocket == (Vpi_RawSocket)VPI_P_SOCKET_INVALID)
	{
		return Vpi_Null;
	}

	winSocketServer = (SOCKET)a_RawSocket;

	cli_size = sizeof(cli);

	Vpi_MemSet(&cli, 0, cli_size);

	winSocketClient = accept(winSocketServer,(struct sockaddr*) &cli, &cli_size);

	if(winSocketClient == VPI_P_SOCKET_INVALID)
	{
		/* accept failed */
		goto Error;
	}

	if(a_pPort != Vpi_Null)
	{
		*a_pPort = ntohs((Vpi_UInt16)((struct sockaddr_in*)(&cli))->sin_port);
	}

	if(a_pAddress != Vpi_Null)
	{
		*a_pAddress = ((struct sockaddr_in*)(&cli))->sin_addr.s_addr;
	}

	if(a_bNagleOff)
	{
		/* set socket options */
		if(VPI_P_SOCKET_SOCKETERROR == setsockopt(winSocketClient, IPPROTO_TCP, TCP_NODELAY, (const char*)&iFlag, sizeof(int)))
		{
			goto Error;
		}
	}

	if(a_bKeepAliveOn)
	{
		/* set socket options */
		if(VPI_P_SOCKET_SOCKETERROR == setsockopt( winSocketClient, IPPROTO_TCP, SO_KEEPALIVE, (const char*)&iFlag, sizeof(int)))
		{
			goto Error;
		}
	}

#if VPI_P_SOCKET_SETTCPRCVBUFFERSIZE
	iBufferSize = VPI_P_TCPRCVBUFFERSIZE;
	if(VPI_P_SOCKET_SOCKETERROR == setsockopt(winSocketClient, SOL_SOCKET,  SO_RCVBUF, (const char*)&iBufferSize, sizeof(int)))
	{
		/*int result = Vpi_RawSocket_GetLastError((Vpi_RawSocket)winSocketClient);*/
		goto Error;
	}
#endif /* VPI_P_SOCKET_SETTCPRCVBUFFERSIZE */

#if VPI_P_SOCKET_SETTCPSNDBUFFERSIZE
	iBufferSize = VPI_P_TCPSNDBUFFERSIZE;
	if(VPI_P_SOCKET_SOCKETERROR == setsockopt(winSocketClient, SOL_SOCKET,  SO_SNDBUF, (const char*)&iBufferSize, sizeof(int)))
	{
		/*int result = Vpi_RawSocket_GetLastError((Vpi_RawSocket)winSocketClient);*/
		goto Error;
	}
#endif /* VPI_P_SOCKET_SETTCPSNDBUFFERSIZE */

	return (Vpi_RawSocket)winSocketClient;

Error:

	if(winSocketClient == VPI_P_SOCKET_INVALID)
	{
		Vpi_RawSocket_Close((Vpi_RawSocket)winSocketClient);
	}

	return (Vpi_RawSocket)VPI_P_SOCKET_INVALID;
}

/*============================================================================
 * Read Socket.
 *===========================================================================*/
Vpi_Int32 VPI_DLLCALL Vpi_RawSocket_Read( Vpi_RawSocket a_RawSocket,
		Vpi_Byte*     a_pBuffer,
		Vpi_UInt32    a_nBufferSize,
		int flags)
{
	int     intBytesReceived    = 0;
	SOCKET  winSocket           = (SOCKET)VPI_P_SOCKET_INVALID;

	if(a_RawSocket == (Vpi_RawSocket)VPI_P_SOCKET_INVALID)
		intBytesReceived = - 1;
	else
	{

		winSocket = (SOCKET)a_RawSocket;
		intBytesReceived = recv(winSocket, (char*)a_pBuffer, (int)a_nBufferSize, flags);
	}
	return intBytesReceived;
}

/*============================================================================
 * Write Socket.
 *===========================================================================*/
Vpi_Int32 VPI_DLLCALL Vpi_RawSocket_Write(    Vpi_RawSocket a_RawSocket,
		Vpi_Byte*     a_pBuffer,
		Vpi_UInt32    a_uBufferSize)
{
	Vpi_Int32     intBytesSend    = 0;
	SOCKET  winSocket       = (SOCKET)VPI_P_SOCKET_INVALID;

	if(a_RawSocket == (Vpi_RawSocket)VPI_P_SOCKET_INVALID)
	{
		return 0;
	}

	winSocket = (SOCKET)a_RawSocket;

	intBytesSend = send(winSocket, (char*)a_pBuffer, a_uBufferSize, 0);

	return intBytesSend;
}


Vpi_StatusCode VPI_DLLCALL Vpi_RawSocket_SetTimeout(Vpi_RawSocket a_RawSocket, Vpi_Int32 nTimeOut)
{
	Vpi_StatusCode uStatus = Vpi_Good;
#ifdef WIN32
	int              apiResult   = 0;
	SOCKET           winSocket   = (SOCKET)VPI_P_SOCKET_INVALID;

	if(a_RawSocket == (Vpi_RawSocket)VPI_P_SOCKET_INVALID)
	{
		return 0;
	}			
	int err = setsockopt((SOCKET)a_RawSocket, SOL_SOCKET, SO_SNDTIMEO, (char *)&nTimeOut, sizeof(nTimeOut));
	if (err != NO_ERROR)
		uStatus=Vpi_BadCommunicationError;
#endif
	return uStatus;
}
/*============================================================================
 * Set socket to nonblocking mode
 *===========================================================================*/
Vpi_StatusCode VPI_DLLCALL Vpi_RawSocket_SetBlockMode(    Vpi_RawSocket a_RawSocket,
		Vpi_Boolean   a_bBlocking)
{
#ifdef WIN32
	int              apiResult   = 0;
	SOCKET           winSocket   = (SOCKET)VPI_P_SOCKET_INVALID;
	Vpi_StatusCode uStatus     = Vpi_Good;
	u_long           uNonBlocking= (a_bBlocking==Vpi_False)?1:0;

	if(a_RawSocket == (Vpi_RawSocket)VPI_P_SOCKET_INVALID)
	{
		return 0;
	}

	winSocket = (SOCKET)a_RawSocket;

	apiResult = ioctlsocket(winSocket, FIONBIO, &uNonBlocking);

	if(apiResult != 0)
	{
		uStatus = Vpi_BadCommunicationError;
	}
#endif
#ifdef _linux
SOCKET           winSocket   = (SOCKET)VPI_P_SOCKET_INVALID;
Vpi_StatusCode uStatus     = Vpi_Good;
int              flags       = 0;

if(a_RawSocket == (Vpi_RawSocket)VPI_P_SOCKET_INVALID)
{
	return 0;
}

winSocket = (SOCKET)a_RawSocket;

flags = fcntl(winSocket,F_GETFL,0);

if(flags == -1)
	return Vpi_BadCommunicationError;

if(a_bBlocking == Vpi_False)
	flags |= O_NONBLOCK;
else
	flags &= ~O_NONBLOCK;

if(fcntl(winSocket, F_SETFL, flags | O_NONBLOCK) == -1)
	uStatus = Vpi_BadCommunicationError;

#endif
return uStatus;
}


/*============================================================================
 * Network Byte Order Conversion Helper Functions
 *===========================================================================*/
Vpi_UInt32 Vpi_RawSocket_NToHL(Vpi_UInt32 netLong)
{
	Vpi_UInt32 retval = ntohl((unsigned long)netLong);
	return retval;
}

Vpi_UInt16 Vpi_RawSocket_NToHS(Vpi_UInt16 netShort)
{
	Vpi_UInt16 retval = ntohs((unsigned short)netShort);
	return retval;
}

Vpi_UInt32 Vpi_RawSocket_HToNL(Vpi_UInt32 hstLong)
{
	Vpi_UInt32 retval = htonl((unsigned long)hstLong);
	return retval;
}

Vpi_UInt16 Vpi_RawSocket_HToNS(Vpi_UInt16 hstShort)
{
	Vpi_UInt16 retval = htons((unsigned short)hstShort);
	return retval;
}

#if VPI_P_SOCKETGETPEERINFO_V2
/*============================================================================
 * Get address information about the peer
 *===========================================================================*/
Vpi_StatusCode Vpi_RawSocket_GetPeerInfo( Vpi_Socket    a_RawSocket,
		Vpi_CharA*    a_achPeerInfoBuffer,
		Vpi_UInt32    a_uiPeerInfoBufferSize)
{
	int                 apiResult       = 0;
	struct sockaddr_in  sockAddrIn;
	size_t              TempLen         = sizeof(struct sockaddr_in);
	SOCKET              winSocket       = (SOCKET)VPI_P_SOCKET_INVALID;
	char*               pchAddrBuf      = Vpi_Null;
	Vpi_UInt16        usPort          = 0;

	//Vpi_InitializeStatus(Vpi_Module_Socket, "GetPeerInfo");

	/* initial parameter check */
	Vpi_ReturnErrorIfTrue((a_RawSocket == (Vpi_RawSocket)VPI_P_SOCKET_INVALID), Vpi_BadInvalidArgument);
	Vpi_ReturnErrorIfTrue((a_uiPeerInfoBufferSize < VPI_P_PEERINFO_MIN_SIZE), Vpi_BadInvalidArgument);
	Vpi_ReturnErrorIfArgumentNull(a_achPeerInfoBuffer);

	winSocket = (SOCKET)a_RawSocket;
	/* Montpellier Workshop */
	apiResult = getpeername(winSocket, (struct sockaddr*)&sockAddrIn, (socklen_t *)&TempLen);

	Vpi_ReturnErrorIfTrue((apiResult != 0), Vpi_BadInternalError);

	/* IP */
	pchAddrBuf = inet_ntoa(sockAddrIn.sin_addr);
	Vpi_GotoErrorIfTrue(pchAddrBuf == Vpi_Null, Vpi_BadInternalError);

	/* Port */
	usPort = Vpi_RawSocket_NToHS((Vpi_UInt16)sockAddrIn.sin_port);

	/* build result string */
	TempLen = strlen(pchAddrBuf);

#if VPI_USE_SAFE_FUNCTIONS
	Vpi_GotoErrorIfTrue((strncpy_s(a_achPeerInfoBuffer, a_uiPeerInfoBufferSize + 1, pchAddrBuf, TempLen) != 0), Vpi_Bad);
	a_achPeerInfoBuffer[TempLen] = ':';
	TempLen++;
	sprintf_s(&a_achPeerInfoBuffer[TempLen], a_uiPeerInfoBufferSize - TempLen, "%u", usPort);
#else /* VPI_USE_SAFE_FUNCTIONS */
	Vpi_GotoErrorIfTrue((strncpy(a_achPeerInfoBuffer, pchAddrBuf, TempLen) != a_achPeerInfoBuffer), Vpi_Bad);
	a_achPeerInfoBuffer[TempLen] = ':';
	sprintf(&a_achPeerInfoBuffer[TempLen + 1], "%u", usPort);
#endif /* VPI_USE_SAFE_FUNCTIONS */

	Vpi_ReturnStatusCode;
	Vpi_BeginErrorHandling;
	Vpi_FinishErrorHandling;
}
#else /* VPI_P_SOCKETGETPEERINFO_V2 */
/*============================================================================
 * Get IP Address and Port Number of the Peer
 *===========================================================================*/
Vpi_StatusCode Vpi_RawSocket_GetPeerInfo( Vpi_RawSocket a_RawSocket,
		Vpi_UInt32*   a_pIP,
		Vpi_UInt16*   a_pPort)
{
	Vpi_StatusCode uStatus=Vpi_Good;
	int                 apiResult       = 0;
	struct sockaddr_in  sockAddrIn;
	size_t              sockAddrInLen   = sizeof(struct sockaddr_in);
	SOCKET              winSocket       = (SOCKET)VPI_P_SOCKET_INVALID;

	//Vpi_InitializeStatus(Vpi_Module_Socket, "GetPeerInfo");

	if(a_RawSocket == (Vpi_RawSocket)VPI_P_SOCKET_INVALID)
	{
		uStatus=Vpi_BadInvalidArgument;
	}
	else
	{
		winSocket = (SOCKET)a_RawSocket;

		apiResult = getpeername(winSocket, (struct sockaddr*)&sockAddrIn, (int*)&sockAddrInLen);

		if(apiResult != 0)
		{
			uStatus = Vpi_BadCommunicationError;
		}
		else
		{
			if(a_pIP != Vpi_Null)
				*a_pIP   = Vpi_RawSocket_NToHL((Vpi_UInt32)sockAddrIn.sin_addr.s_addr);
			if(a_pPort != Vpi_Null)
				*a_pPort = Vpi_RawSocket_NToHS((Vpi_UInt16)sockAddrIn.sin_port);
		}
	}
	return uStatus;
}
#endif

/*============================================================================
 * Get IP Address and Port Number of the local connection
 *===========================================================================*/
Vpi_StatusCode Vpi_RawSocket_GetLocalInfo(Vpi_RawSocket a_RawSocket,
											  Vpi_UInt32*   a_pIP,
											  Vpi_UInt16*   a_pPort)
{
	Vpi_StatusCode uStatus=Vpi_Good;
	int                 apiResult     = 0;
	struct sockaddr_in  sockAddrIn;
	size_t              sockAddrInLen = sizeof(struct sockaddr_in);
	SOCKET              winSocket     = (SOCKET)VPI_P_SOCKET_INVALID;

	//Vpi_InitializeStatus(Vpi_Module_Socket, "GetLocalInfo");

	if(a_RawSocket == (Vpi_RawSocket)VPI_P_SOCKET_INVALID)
		uStatus=Vpi_BadInvalidArgument;
	else
	{
		winSocket = (SOCKET)a_RawSocket;
		apiResult = getsockname(winSocket, (struct sockaddr*)&sockAddrIn, (socklen_t *)&sockAddrInLen);

		if(apiResult != 0)
		{
			apiResult = Vpi_RawSocket_GetLastError(a_RawSocket);
			uStatus = Vpi_BadCommunicationError;			
		}
		else
		{
			if(a_pIP != Vpi_Null)
				*a_pIP   = Vpi_RawSocket_NToHL((Vpi_UInt32)sockAddrIn.sin_addr.s_addr);

			if(a_pPort != Vpi_Null)
				*a_pPort = Vpi_RawSocket_NToHS((Vpi_UInt16)sockAddrIn.sin_port);
		}
	}
	return uStatus;
}

/*============================================================================
 * Select usable socket. (maxfds ignored in win32)
 *===========================================================================*/
Vpi_StatusCode Vpi_RawSocket_Select(  Vpi_RawSocket         a_MaxFds,
		Vpi_P_Socket_Array*   a_pFdSetRead,
		Vpi_P_Socket_Array*   a_pFdSetWrite,
		Vpi_P_Socket_Array*   a_pFdSetException,
		Vpi_TimeVal*          a_pTimeout)
{
	Vpi_StatusCode uStatus=Vpi_Good;
	int                 apiResult  = 0;
	struct timeval      timeout;

	//Vpi_InitializeStatus(Vpi_Module_Socket, "P_Select");

	//Vpi_ReferenceParameter(a_MaxFds);

	//if (a_pFdSetRead)
	{
		//if (a_pFdSetWrite)
		{
			//if (a_pFdSetException)
			{
				timeout.tv_sec  = a_pTimeout->uintSeconds;
				timeout.tv_usec = a_pTimeout->uintMicroSeconds;

				apiResult = select( 0,
						(fd_set*)a_pFdSetRead,
						(fd_set*)a_pFdSetWrite,
						(fd_set*)a_pFdSetException,
						&timeout);

				if(apiResult == VPI_P_SOCKET_SOCKETERROR)
				{
			#ifndef _linux
					apiResult = WSAGetLastError();
					switch(apiResult)
					{
						case WSAENOTSOCK:
						case WSAEINVAL:
							uStatus = Vpi_BadInvalidArgument;
							break;
						default:
							uStatus = Vpi_BadCommunicationError;
							break;
					}
			#else
					uStatus   = Vpi_BadCommunicationError;
					apiResult = Vpi_RawSocket_GetLastError(Vpi_Null);
			#endif
				}
			}
			//else
			//	uStatus=Vpi_BadInvalidArgument;
		}
		//else
		//	uStatus=Vpi_BadInvalidArgument;
	}
	//else
	//	uStatus=Vpi_BadInvalidArgument;
	return uStatus;
}

/*============================================================================
 * Get last socket error.
 *===========================================================================*/
Vpi_Int32 Vpi_RawSocket_GetLastError( Vpi_RawSocket a_RawSocket)
{
	int lastError = 0;
	//Vpi_ReferenceParameter(a_RawSocket); /* Not needed in this implementation. */

	lastError = WSAGetLastError();

	return (Vpi_Int32)lastError;
}

/*============================================================================
 * Vpi_RawSocket_FD_Isset
 *===========================================================================*/
Vpi_Boolean Vpi_RawSocket_FD_Isset(   Vpi_RawSocket         a_RawSocket,
		Vpi_P_Socket_Array*   a_pFdSet)
{
	SOCKET WinSocket = (SOCKET)a_RawSocket;

	if(FD_ISSET(WinSocket, (fd_set*)a_pFdSet) == TRUE)
	{
		return Vpi_True;
	}
	else
	{
		return Vpi_False;
	}
}

/*============================================================================
 * Initialize the platform network interface
 *===========================================================================*/
Vpi_UInt32 Vpi_RawSocket_InetAddr(Vpi_StringA sRemoteAddress)
{
	if(sRemoteAddress != Vpi_Null)
	{
		return (Vpi_UInt32)inet_addr(sRemoteAddress);
	}

	return 0;
}
#endif /* WIN32 */

#ifdef _GNUC_
/*============================================================================
* Set the time for a specified socket
*===========================================================================*/
Vpi_StatusCode Vpi_RawSocket_SetTimeout(Vpi_RawSocket a_RawSocket, Vpi_Int32 nTimeOut)
{
	Vpi_StatusCode uStatus=Vpi_Good;
	SOCKET           rawSocket   = (SOCKET)VPI_P_SOCKET_INVALID;
	rawSocket = (SOCKET)a_RawSocket;
	if((int)rawSocket == VPI_P_SOCKET_INVALID)
	{
		return 0;
	}			
	int err = setsockopt(rawSocket, SOL_SOCKET, SO_SNDTIMEO, (char *)&nTimeOut, sizeof(nTimeOut));
	if (err != NO_ERROR)
		uStatus=Vpi_BadCommunicationError;
	return uStatus;
}
/*============================================================================
 * Initialize the platform network interface
 *===========================================================================*/
Vpi_StatusCode Vpi_RawSocket_InitializeNetwork()
{
	return Vpi_Good;
}
/*============================================================================
 * Clean the platform network interface up.
 *===========================================================================*/
Vpi_StatusCode Vpi_RawSocket_CleanupNetwork()
{
	return Vpi_Good;
}
/*============================================================================
 * Shutdown Socket.
 *===========================================================================*/
Vpi_StatusCode Vpi_RawSocket_Shutdown(Vpi_RawSocket a_RawSocket,Vpi_Int       a_iHow)
{
	SOCKET  rawSocket   = (SOCKET)VPI_P_SOCKET_INVALID;
	int     iRetVal     = 0;

	Vpi_StatusCode uStatus=Vpi_Good;

	rawSocket = (SOCKET)a_RawSocket;

	/* shutdown socket */
	iRetVal = shutdown(rawSocket, a_iHow); /* SD_RECEIVE=0, SD_SEND=1, SD_BOTH=2 */

	/* check uStatus */
	if(iRetVal == SOCKET_ERROR)
	{
		switch (errno)
		{
			case EBADF:
				uStatus=(Vpi_BadInvalidArgument);
				break;
			case EINVAL:
				uStatus=(Vpi_BadInvalidArgument);
				break;
			case ENOTCONN:
				uStatus=(Vpi_BadInvalidState);
				break;
			case ENOTSOCK:
				uStatus=(Vpi_BadInvalidState);
				break;
			case ENOBUFS:
				uStatus=(Vpi_BadOutOfMemory);
				break;
			default:
				uStatus = Vpi_BadUnexpectedError;
				break;
		}
	}

	return uStatus;
}
/*============================================================================
 * Close Socket.
 *===========================================================================*/
Vpi_StatusCode Vpi_RawSocket_Close(Vpi_RawSocket a_RawSocket)
{
	Vpi_StatusCode uStatus;
	int gnuSocket = (int)VPI_P_SOCKET_INVALID;

	gnuSocket = (int)a_RawSocket;
	uStatus = closesocket(gnuSocket);

	/* check uStatus */
	if(uStatus == (Vpi_StatusCode)-1)
	{
		uStatus = Vpi_BadCommunicationError;
	}
	return uStatus;
}

/*============================================================================
 * Create Socket.
 *===========================================================================*/
Vpi_StatusCode Vpi_RawSocket_Create(Vpi_RawSocket*    a_pRawSocket,
										Vpi_Boolean       a_bNagleOff,
										Vpi_Boolean       a_bKeepAliveOn,
										Vpi_Boolean*      a_pbIPv6)
{
	Vpi_StatusCode    uStatus     = Vpi_Good;
	int                 iFlag       = 1;
	Vpi_Int           apiResult   = 0;
	SOCKET              rawSocket   = (SOCKET)VPI_P_SOCKET_INVALID;

#if VPI_P_SOCKET_SETTCPRCVBUFFERSIZE || VPI_P_SOCKET_SETTCPSNDBUFFERSIZE
	Vpi_Int           iBufferSize = VPI_P_TCPRCVBUFFERSIZE;
#endif /* VPI_P_SOCKET_SETTCPRCVBUFFERSIZE || VPI_P_SOCKET_SETTCPSNDBUFFERSIZE */

	if (a_pRawSocket)
	{
		if (a_pbIPv6)
		{
			/* create socket through platform API */
			do
			{
				rawSocket = socket(((*a_pbIPv6)?AF_INET6:AF_INET), SOCK_STREAM, 0);
				apiResult = Vpi_RawSocket_GetLastError((Vpi_RawSocket)rawSocket);

				/* check if socket creation was successful */
				if(     rawSocket == VPI_P_SOCKET_INVALID
					||  apiResult != 0)
				{
					uStatus=(Vpi_BadCommunicationError);
					apiResult=0;
				}
				else
				{
					/* try to activate dual stack support (IPv4 and IPv6) by disabling IPV6_V6ONLY */
					if(*a_pbIPv6)
					{
						int no = 0;
						apiResult = setsockopt(rawSocket, IPPROTO_IPV6, IPV6_V6ONLY, (const char*)&no, sizeof(int));
						if(VPI_P_SOCKET_SOCKETERROR == apiResult)
						{
							Vpi_Trace(Vpi_Null,VPI_TRACE_LEVEL_WARNING, "Could not enable dual protocol stack. IPv4 only.\n");
							closesocket(rawSocket);
							*a_pbIPv6 = Vpi_False;
						}
					}
				}
			} while(apiResult);

			/* set socketoptions */
			if ((a_bNagleOff) && (uStatus==Vpi_Good))
			{
				if(VPI_P_SOCKET_SOCKETERROR == setsockopt(rawSocket, IPPROTO_TCP, TCP_NODELAY, (const char*)&iFlag, sizeof(int)))
				{
					Vpi_Trace(Vpi_Null,VPI_TRACE_LEVEL_WARNING, "Could not set nodelay option.\n");
					uStatus=(Vpi_BadCommunicationError);
					if(rawSocket != VPI_P_SOCKET_INVALID)
					{
						Vpi_RawSocket_Close((Vpi_RawSocket)rawSocket);
						*a_pRawSocket = (Vpi_RawSocket)VPI_P_SOCKET_INVALID;
					}
				}
			}
			if (uStatus!=Vpi_BadCommunicationError)
			{
				if(a_bKeepAliveOn)
				{
					/* set socket options */
					if(VPI_P_SOCKET_SOCKETERROR == setsockopt(rawSocket, IPPROTO_TCP,  SO_KEEPALIVE, (const char*)&iFlag, sizeof(int)))
					{
						Vpi_Trace(Vpi_Null,VPI_TRACE_LEVEL_WARNING, "Could not set keepalive option.\n");
						uStatus=(Vpi_BadCommunicationError);
						if(rawSocket != VPI_P_SOCKET_INVALID)
						{
							Vpi_RawSocket_Close((Vpi_RawSocket)rawSocket);
							*a_pRawSocket = (Vpi_RawSocket)VPI_P_SOCKET_INVALID;
						}
					}
				}


			#if VPI_P_SOCKET_SETTCPRCVBUFFERSIZE
				iBufferSize = VPI_P_TCPRCVBUFFERSIZE;
				if(VPI_P_SOCKET_SOCKETERROR == setsockopt(rawSocket, SOL_SOCKET,  SO_RCVBUF, (const char*)&iBufferSize, sizeof(int)))
				{
					uStatus=(Vpi_BadCommunicationError);
					if(rawSocket != VPI_P_SOCKET_INVALID)
					{
						Vpi_RawSocket_Close((Vpi_RawSocket)rawSocket);
						*a_pRawSocket = (Vpi_RawSocket)VPI_P_SOCKET_INVALID;
					}
				}
			#endif /* VPI_P_SOCKET_SETTCPRCVBUFFERSIZE */

			#if VPI_P_SOCKET_SETTCPSNDBUFFERSIZE
				iBufferSize = VPI_P_TCPSNDBUFFERSIZE;
				if(VPI_P_SOCKET_SOCKETERROR == setsockopt(rawSocket, SOL_SOCKET,  SO_SNDBUF, (const char*)&iBufferSize, sizeof(int)))
				{
					uStatus=(Vpi_BadCommunicationError);
					if(rawSocket != VPI_P_SOCKET_INVALID)
					{
						Vpi_RawSocket_Close((Vpi_RawSocket)rawSocket);
						*a_pRawSocket = (Vpi_RawSocket)VPI_P_SOCKET_INVALID;
					}
				}
			#endif /* VPI_P_SOCKET_SETTCPSNDBUFFERSIZE */
				if(rawSocket != VPI_P_SOCKET_INVALID)
					*a_pRawSocket = (Vpi_RawSocket)rawSocket;
			}
		}
		else
			uStatus=Vpi_BadInvalidArgument;
	}
	else
		uStatus=Vpi_BadInvalidArgument;
	return uStatus;

//Error:
//
//    if(rawSocket != VPI_P_SOCKET_INVALID)
//    {
//        Vpi_RawSocket_Close((Vpi_RawSocket)rawSocket);
//        *a_pRawSocket = (Vpi_RawSocket)VPI_P_SOCKET_INVALID;
//		uStatus=Vpi_BadInvalidArgument;
//    }
}
/*============================================================================
 * Connect Socket for Client.
 *===========================================================================*/
Vpi_StatusCode Vpi_RawSocket_Connect(Vpi_RawSocket a_RawSocket,
		Vpi_Int16     a_nPort,
		Vpi_StringA   a_sHost)
{
	char*              localhost = (char*)"127.0.0.1";
	struct sockaddr_in gnuAddr;
	int gnuSocket = (int)a_RawSocket;
	Vpi_StatusCode uStatus=Vpi_Good;

	if(strcmp("localhost", a_sHost) == 0)
	{
		a_sHost = localhost;
	}
	memset(&gnuAddr, 0, sizeof(gnuAddr));
	gnuAddr.sin_family = AF_INET;
	gnuAddr.sin_port = htons(a_nPort);
	inet_pton(AF_INET, a_sHost, &gnuAddr.sin_addr);
	int rc = connect(gnuSocket, (struct sockaddr *)&gnuAddr, sizeof(gnuAddr));
	if(rc != 0)

	{
		if(errno == EINPROGRESS)
		{
			uStatus = Vpi_BadWouldBlock;
		}
		else
		{
			uStatus = Vpi_BadCommunicationError;
		}
	}
	uStatus = Vpi_Good;
	return uStatus;
}
/*============================================================================
 * Bind to Socket
 *===========================================================================*/
Vpi_StatusCode Vpi_RawSocket_Bind(Vpi_RawSocket a_RawSocket,Vpi_Int16 a_nPort,Vpi_Boolean a_bIPv6)
{
	Vpi_StatusCode uStatus=Vpi_Good;
	Vpi_Int32         intSize    = 0;

	struct sockaddr_in  srv;
	struct sockaddr_in6 srv6;
	struct sockaddr     *pName;

	//Vpi_InitializeStatus(Vpi_Module_Socket, "P_Bind");
	if (a_RawSocket)
	{
		int gnuSocket = (int)a_RawSocket;

		if (!a_bIPv6)
		{
			// ipv4
			intSize = sizeof(struct sockaddr_in);
			Vpi_MemSet(&srv, 0, intSize);
			srv.sin_addr.s_addr = INADDR_ANY;
			srv.sin_port        = htons(a_nPort);
			srv.sin_family      = AF_INET;
			pName               = (struct sockaddr*)&srv;
		}
		else
		{
			// ipv6
			intSize = sizeof(struct sockaddr_in6);
			Vpi_MemSet(&srv6, 0, intSize);
			memset(&srv6.sin6_addr.s6_addr, 0, sizeof(srv6.sin6_addr.s6_addr));
			srv6.sin6_port       = htons(a_nPort);
			srv6.sin6_family     = AF_INET6;
			pName = (struct sockaddr*)&srv6;
		}
		if(bind (gnuSocket, pName, intSize) == SOCKET_ERROR)
		{
			uStatus = Vpi_BadCommunicationError;
		}
	}
	else
		uStatus=Vpi_BadInvalidArgument;
	return uStatus;
}
/*============================================================================
 * Bind to Socket and set to listen for Server.
 *===========================================================================*/
Vpi_StatusCode Vpi_RawSocket_Listen(Vpi_RawSocket a_RawSocket)
{
	int gnuSocket  = (int)VPI_P_SOCKET_INVALID;
	Vpi_StatusCode uStatus=Vpi_Good;

	//Vpi_InitializeStatus(Vpi_Module_Socket, "P_Listen");
	gnuSocket = (int)a_RawSocket;

	if(listen(gnuSocket, SOMAXCONN) == VPI_P_SOCKET_SOCKETERROR)
	{
		uStatus = Vpi_BadCommunicationError;
	}


	return uStatus;
}
/*============================================================================
 * Accept Socket connection from Client.
 *===========================================================================*/
Vpi_RawSocket Vpi_RawSocket_Accept(Vpi_RawSocket a_RawSocket,
		Vpi_UInt16*   a_pPort,
		Vpi_UInt32*   a_pAddress,
		Vpi_Boolean   a_bNagleOff,
		Vpi_Boolean   a_bKeepAliveOn)
{
	int                 iFlag           = 1;
	SOCKET              winSocketServer = (SOCKET)VPI_P_SOCKET_INVALID;
	SOCKET              winSocketClient = (SOCKET)VPI_P_SOCKET_INVALID;
	struct sockaddr_in  cli;
	socklen_t			cli_size = sizeof(cli);
#if VPI_P_SOCKET_SETTCPRCVBUFFERSIZE || VPI_P_SOCKET_SETTCPSNDBUFFERSIZE
	Vpi_Int           iBufferSize = VPI_P_TCPRCVBUFFERSIZE;
#endif /* VPI_P_SOCKET_SETTCPRCVBUFFERSIZE || VPI_P_SOCKET_SETTCPSNDBUFFERSIZE */
	if(a_RawSocket == (Vpi_RawSocket)VPI_P_SOCKET_INVALID)
	{
		return Vpi_Null;
	}
	winSocketServer = (SOCKET)a_RawSocket;
	cli_size = sizeof(cli);
	Vpi_MemSet(&cli, 0, cli_size);
	winSocketClient = accept(winSocketServer,(struct sockaddr*) &cli, &cli_size);
	if(winSocketClient == VPI_P_SOCKET_INVALID)
	{
		/* accept failed */
		goto Error;
	}
	if(a_pPort != Vpi_Null)
	{
		*a_pPort = ntohs((Vpi_UInt16)((struct sockaddr_in*)(&cli))->sin_port);
	}
	if(a_pAddress != Vpi_Null)
	{
		*a_pAddress = ((struct sockaddr_in*)(&cli))->sin_addr.s_addr;
	}
	if(a_bNagleOff)
	{
		/* set socket options */
		if(VPI_P_SOCKET_SOCKETERROR == setsockopt(winSocketClient, IPPROTO_TCP, TCP_NODELAY, (const char*)&iFlag, sizeof(int)))
		{
			goto Error;
		}
	}
	if(a_bKeepAliveOn)
	{
		/* set socket options */
		if(VPI_P_SOCKET_SOCKETERROR == setsockopt( winSocketClient, IPPROTO_TCP, SO_KEEPALIVE, (const char*)&iFlag, sizeof(int)))
		{
			goto Error;
		}
	}
#if VPI_P_SOCKET_SETTCPRCVBUFFERSIZE
	iBufferSize = VPI_P_TCPRCVBUFFERSIZE;
	if(VPI_P_SOCKET_SOCKETERROR == setsockopt(winSocketClient, SOL_SOCKET,  SO_RCVBUF, (const char*)&iBufferSize, sizeof(int)))
	{
		/*int result = Vpi_RawSocket_GetLastError((Vpi_RawSocket)winSocketClient);*/
		goto Error;
	}
#endif /* VPI_P_SOCKET_SETTCPRCVBUFFERSIZE */
#if VPI_P_SOCKET_SETTCPSNDBUFFERSIZE
	iBufferSize = VPI_P_TCPSNDBUFFERSIZE;

	if(VPI_P_SOCKET_SOCKETERROR == setsockopt(winSocketClient, SOL_SOCKET,  SO_SNDBUF, (const char*)&iBufferSize, sizeof(int)))
	{
		/*int result = Vpi_RawSocket_GetLastError((Vpi_RawSocket)winSocketClient);*/
		goto Error;
	}
#endif /* VPI_P_SOCKET_SETTCPSNDBUFFERSIZE */
	return (Vpi_RawSocket)winSocketClient;
	Error:
	if(winSocketClient == VPI_P_SOCKET_INVALID)
	{
		Vpi_RawSocket_Close((Vpi_RawSocket)winSocketClient);
	}

	return (Vpi_RawSocket)VPI_P_SOCKET_INVALID;
}
/*============================================================================
 * Read Socket.
 *===========================================================================*/
Vpi_Int32 Vpi_RawSocket_Read(Vpi_RawSocket a_RawSocket,
		Vpi_Byte*     a_pBuffer,
		Vpi_UInt32    a_nBufferSize,
		int flags)
{
	ssize_t intBytesReceived = 0;

	int gnuSocket = (int)VPI_P_SOCKET_INVALID;
	if(a_RawSocket == (Vpi_RawSocket)VPI_P_SOCKET_INVALID)
	{
		return 0;
	}

	gnuSocket = (int)a_RawSocket;
	intBytesReceived = recv(gnuSocket, (char*)a_pBuffer, a_nBufferSize, flags);

	if(intBytesReceived == -1){
		/* Error -> use errno */
	}

	return intBytesReceived;
}
/*============================================================================
 * Write Socket.
 *===========================================================================*/
Vpi_UInt32 Vpi_RawSocket_Write(Vpi_RawSocket a_RawSocket,
		Vpi_Byte*     a_pBuffer,
		Vpi_UInt32    a_uBufferSize)
{
	ssize_t   intBytesSend    = 0;

	int  gnuSocket = (int)VPI_P_SOCKET_INVALID;

	if(a_RawSocket == (Vpi_RawSocket)VPI_P_SOCKET_INVALID)
	{
		return 0;
	}

	gnuSocket = (int)a_RawSocket;

	DWORD timer = GetTickCount();

	intBytesSend = send(gnuSocket, (char*)a_pBuffer, a_uBufferSize,0);

	timer = GetTickCount() - timer;

	if(intBytesSend == -1)
	{
		// TODO traitement des erreurs via errno

		return 0;
	}

	return intBytesSend;
}
/*============================================================================
 * Set socket to nonblocking mode
 *===========================================================================*/
Vpi_StatusCode Vpi_RawSocket_SetBlockMode(Vpi_RawSocket a_RawSocket,
		Vpi_Boolean   a_bBlocking)
{
	int  gnuSocket   = (int)VPI_P_SOCKET_INVALID;
	Vpi_StatusCode 	uStatus     = Vpi_Good;
	int              	flags       = 0;
	int 				apiResult 	= -1;
	if(a_RawSocket == (Vpi_RawSocket)VPI_P_SOCKET_INVALID)
	{
		return 0;
	}

	gnuSocket = (int)a_RawSocket;
	flags = fcntl (gnuSocket, F_GETFL);
	if(a_bBlocking != Vpi_True)
	{
		apiResult = fcntl (gnuSocket, F_SETFL, flags & ~O_NONBLOCK);
	}else
	{
		apiResult = fcntl (gnuSocket, F_SETFL, flags | O_NONBLOCK);
	}
	if(apiResult == -1){
		uStatus = Vpi_BadCommunicationError;
	}
	return uStatus;
}
/*============================================================================
 * Network Byte Order Conversion Helper Functions
 *===========================================================================*/
Vpi_UInt32 Vpi_RawSocket_NToHL(Vpi_UInt32 netLong)
{
	Vpi_UInt32 retval = ntohl((unsigned long)netLong);
	return retval;
}
Vpi_UInt16 Vpi_RawSocket_NToHS(Vpi_UInt16 netShort)
{
	Vpi_UInt16 retval = ntohs((unsigned short)netShort);
	return retval;
}
Vpi_UInt32 Vpi_RawSocket_HToNL(Vpi_UInt32 hstLong)
{
	Vpi_UInt32 retval = htonl((unsigned long)hstLong);
	return retval;
}
Vpi_UInt16 Vpi_RawSocket_HToNS(Vpi_UInt16 hstShort)
{
	Vpi_UInt16 retval = htons((unsigned short)hstShort);
	return retval;
}
#if VPI_P_SOCKETGETPEERINFO_V2
/*============================================================================
 * Get address information about the peer
 *===========================================================================*/
Vpi_StatusCode Vpi_RawSocket_GetPeerInfo(Vpi_Socket    a_RawSocket,
		Vpi_CharA*    a_achPeerInfoBuffer,
		Vpi_UInt32    a_uiPeerInfoBufferSize)
{
	int                 apiResult       = 0;
	struct sockaddr_in  sockAddrIn;
	size_t              TempLen         = sizeof(struct sockaddr_in);
	SOCKET              winSocket       = (SOCKET)VPI_P_SOCKET_INVALID;
	char*               pchAddrBuf      = Vpi_Null;
	Vpi_UInt16        usPort          = 0;
	//Vpi_InitializeStatus(Vpi_Module_Socket, "GetPeerInfo");
	/* initial parameter check */
	Vpi_ReturnErrorIfTrue((a_RawSocket == (Vpi_RawSocket)VPI_P_SOCKET_INVALID), Vpi_BadInvalidArgument);
	Vpi_ReturnErrorIfTrue((a_uiPeerInfoBufferSize < VPI_P_PEERINFO_MIN_SIZE), Vpi_BadInvalidArgument);
	Vpi_ReturnErrorIfArgumentNull(a_achPeerInfoBuffer);
	winSocket = (SOCKET)a_RawSocket;

	/* Montpellier Workshop */
	apiResult = getpeername(winSocket, (struct sockaddr*)&sockAddrIn, (socklen_t *)&TempLen);
	Vpi_ReturnErrorIfTrue((apiResult != 0), Vpi_BadInternalError);
	/* IP */
	pchAddrBuf = inet_ntoa(sockAddrIn.sin_addr);
	Vpi_GotoErrorIfTrue(pchAddrBuf == Vpi_Null, Vpi_BadInternalError);
	/* Port */
	usPort = Vpi_RawSocket_NToHS((Vpi_UInt16)sockAddrIn.sin_port);

	/* build result string */
	TempLen = strlen(pchAddrBuf);

#ifdef WIN32
#if VPI_USE_SAFE_FUNCTIONS
	Vpi_GotoErrorIfTrue((strncpy_s(a_achPeerInfoBuffer, a_uiPeerInfoBufferSize + 1, pchAddrBuf, TempLen) != 0), Vpi_Bad);
	a_achPeerInfoBuffer[TempLen] = ':';
	TempLen++;
	sprintf_s(&a_achPeerInfoBuffer[TempLen], a_uiPeerInfoBufferSize - TempLen, "%u", usPort);

#else /* VPI_USE_SAFE_FUNCTIONS */
	Vpi_GotoErrorIfTrue((strncpy(a_achPeerInfoBuffer, pchAddrBuf, TempLen) != a_achPeerInfoBuffer), Vpi_Bad);
	a_achPeerInfoBuffer[TempLen] = ':';
	sprintf(&a_achPeerInfoBuffer[TempLen + 1], "%u", usPort);
#endif /* VPI_USE_SAFE_FUNCTIONS */
#endif

#ifdef _GNUC_
	Vpi_GotoErrorIfTrue((strncpy(a_achPeerInfoBuffer, pchAddrBuf, TempLen) != a_achPeerInfoBuffer), Vpi_Bad);
	a_achPeerInfoBuffer[TempLen] = ':';
	sprintf(&a_achPeerInfoBuffer[TempLen + 1], "%u", usPort);	
#endif

	Vpi_ReturnStatusCode;
	Vpi_BeginErrorHandling;
	Vpi_FinishErrorHandling;
}
#else /* VPI_P_SOCKETGETPEERINFO_V2 */
/*============================================================================
 * Get IP Address and Port Number of the Peer
 *===========================================================================*/
Vpi_StatusCode Vpi_RawSocket_GetPeerInfo(Vpi_RawSocket a_RawSocket,
		Vpi_UInt32*   a_pIP,
		Vpi_UInt16*   a_pPort)
{
	Vpi_StatusCode uStatus=Vpi_Good;
	int                 apiResult       = 0;
	struct sockaddr_in  sockAddrIn;
	size_t              sockAddrInLen   = sizeof(struct sockaddr_in);
	SOCKET              winSocket       = (SOCKET)VPI_P_SOCKET_INVALID;
	//Vpi_InitializeStatus(Vpi_Module_Socket, "GetPeerInfo");
	if(a_RawSocket == (Vpi_RawSocket)VPI_P_SOCKET_INVALID)
	{
		return 0;
	}
	winSocket = (SOCKET)a_RawSocket;
	apiResult = getpeername(winSocket, (struct sockaddr*)&sockAddrIn, (socklen_t*)&sockAddrInLen);
	if(apiResult != 0)
	{
		uStatus = Vpi_BadCommunicationError;
	}
	else
	{
		if(a_pIP != Vpi_Null)
		{
			*a_pIP   = Vpi_RawSocket_NToHL((Vpi_UInt32)sockAddrIn.sin_addr.s_addr);
		}
		if(a_pPort != Vpi_Null)
		{
			*a_pPort = Vpi_RawSocket_NToHS((Vpi_UInt16)sockAddrIn.sin_port);
		}
	}
	return uStatus;
}
#endif
/*============================================================================
 * Get IP Address and Port Number of the local connection
 *===========================================================================*/
Vpi_StatusCode Vpi_RawSocket_GetLocalInfo(Vpi_RawSocket a_RawSocket,
		Vpi_UInt32*   a_pIP,
		Vpi_UInt16*   a_pPort)
{
	Vpi_StatusCode uStatus=Vpi_Good;
	int                 apiResult     = 0;
	struct sockaddr_in  sockAddrIn;
	size_t              sockAddrInLen = sizeof(struct sockaddr_in);
	SOCKET              winSocket     = (SOCKET)VPI_P_SOCKET_INVALID;
	//Vpi_InitializeStatus(Vpi_Module_Socket, "GetLocalInfo");

	if(a_RawSocket == (Vpi_RawSocket)VPI_P_SOCKET_INVALID)
	{
		return 0;
	}
	winSocket = (SOCKET)a_RawSocket;
	/* Montpellier Workshop */
	apiResult = getsockname(winSocket, (struct sockaddr*)&sockAddrIn, (socklen_t *)&sockAddrInLen);
	if(apiResult != 0)
	{
		apiResult = Vpi_RawSocket_GetLastError(a_RawSocket);
		uStatus = Vpi_BadCommunicationError;
	}
	else
	{
		if(a_pIP != Vpi_Null)
		{
			*a_pIP   = Vpi_RawSocket_NToHL((Vpi_UInt32)sockAddrIn.sin_addr.s_addr);
		}
		if(a_pPort != Vpi_Null)
		{
			*a_pPort = Vpi_RawSocket_NToHS((Vpi_UInt16)sockAddrIn.sin_port);
		}
	}
	return uStatus;
}

/*============================================================================
 * Select usable socket. (maxfds ignored in win32)
 *===========================================================================*/
Vpi_StatusCode Vpi_RawSocket_Select( Vpi_RawSocket         a_MaxFds,
		Vpi_P_Socket_Array*   a_pFdSetRead,
		Vpi_P_Socket_Array*   a_pFdSetWrite,
		Vpi_P_Socket_Array*   a_pFdSetException,
		Vpi_TimeVal*          a_pTimeout)
{
	Vpi_StatusCode uStatus=Vpi_Good;
	int                 apiResult  = 0;
	struct timeval      timeout;

	//Vpi_InitializeStatus(Vpi_Module_Socket, "P_Select");
	if (a_pFdSetRead)
	{
		if (a_pFdSetWrite)
		{
			if (a_pFdSetException)
			{
				timeout.tv_sec  = a_pTimeout->uintSeconds;
				timeout.tv_usec = a_pTimeout->uintMicroSeconds;


				do
				{

					apiResult = select(
							(int)a_MaxFds + 1,
							(fd_set*)&(a_pFdSetRead->SocketArray),
							(fd_set*)&(a_pFdSetWrite->SocketArray),
							(fd_set*)&(a_pFdSetException->SocketArray),
							&timeout);

					if((apiResult == VPI_P_SOCKET_SOCKETERROR) && (errno == EINTR))
					{
						Vpi_Trace(Vpi_Null,VPI_TRACE_LEVEL_ERROR,"Signal while Vpi_RawSocket_Select (errno is %d) - Try again\n",errno);
					}

				}while( (apiResult == VPI_P_SOCKET_SOCKETERROR) && (errno == EINTR) );


				if(apiResult == VPI_P_SOCKET_SOCKETERROR)
				{

						uStatus   = Vpi_BadCommunicationError;
						apiResult = Vpi_RawSocket_GetLastError(Vpi_Null);

						Vpi_Trace(Vpi_Null,VPI_TRACE_LEVEL_ERROR,"Error while Vpi_RawSocket_Select: (API result is %d, errno is %d\n",apiResult,errno);
				}
			}
		}
	}

	uStatus = apiResult;
	return uStatus;
}
/*============================================================================
 * Get last socket error.
 *===========================================================================*/
Vpi_Int32 Vpi_RawSocket_GetLastError( Vpi_RawSocket a_RawSocket)
{
	int lastError = 0;
	//Vpi_ReferenceParameter(a_RawSocket); /* Not needed in this implementation. */
	return (Vpi_Int32)lastError;
}

/*============================================================================
 * Vpi_RawSocket_FD_Isset
 *===========================================================================*/
Vpi_Boolean Vpi_RawSocket_FD_Isset(Vpi_RawSocket         a_RawSocket,
		Vpi_P_Socket_Array*   a_pFdSet)
{
	int gnuSocket = (int)a_RawSocket;

	if(FD_ISSET(gnuSocket, (fd_set*)&(a_pFdSet->SocketArray)))
	{
		return Vpi_True;
	}
	else
	{
		return Vpi_False;
	}
}

/*============================================================================
 * Initialize the platform network interface
 *===========================================================================*/
Vpi_UInt32 Vpi_RawSocket_InetAddr(Vpi_StringA sRemoteAddress)
{
	if(sRemoteAddress != Vpi_Null)
	{
		return (Vpi_UInt32)inet_addr(sRemoteAddress);
	}
	return 0;
}
#endif
