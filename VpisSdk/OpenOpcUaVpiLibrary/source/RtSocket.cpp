//**************************************************************************
//
//  Copyright (c) 4CE Industry 1999-2014, All Rights Reserved
//
//**************************************************************************
//
//  Filename   :  RTSocket.cpp
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
// RtSocket.cpp: implementation of the CRtSocket class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifndef NT_TARGET
	#undef _WIN32_WINNT
#endif
#include <errno.h>
#include <assert.h>
#include "RtSocket.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRtSocket::CRtSocket()
{ 
	m_hSocket		= (Vpi_RawSocket)INVALID_SOCKET;
	m_nAddressFamily = AF_INET;
	m_nSocketType = SOCK_STREAM;
	m_uiProtocolType = IPPROTO_TCP;
	m_nTimeout = 0;
	m_pThreadTimeout = NULL;
	m_pAcceptInternalSocket=Vpi_Null;
	m_pListenInternalSocket=Vpi_Null;
	
	//m_LastPeerSockAddr = { 0 };
	//m_LastPeerSockAddr.sin_addr.s_addr=0;
	//m_LastPeerSockAddr.sin_family=0;
	//m_LastPeerSockAddr.sin_port=0;
	//ZeroMemory(m_LastPeerSockAddr.sin_zero,8);

	//memset(&m_LastPeerSockAddr, 0, sizeof(SOCKADDR_IN));
}

CRtSocket::~CRtSocket()
{
	if (((SOCKET)m_hSocket) != INVALID_SOCKET)
	{
		Close();
	}
	if (m_pAcceptInternalSocket)
		free(m_pAcceptInternalSocket);
	if (m_pListenInternalSocket)
		free(m_pListenInternalSocket);
	
	//if (m_pThreadTimeout != NULL)
	//{
	//	m_pThreadTimeout->Quit();
	//}
}



void CRtSocket::Close()
{
	if (((SOCKET)m_hSocket) != INVALID_SOCKET)
	{
		Vpi_RawSocket_Close(m_hSocket);
		m_hSocket = (Vpi_RawSocket)INVALID_SOCKET;
	}
}

Vpi_StatusCode CRtSocket::CreateServer()
{
	Vpi_StatusCode uStatus = Vpi_Good;
	
	m_pAcceptInternalSocket = (Vpi_InternalSocket*)malloc(sizeof(Vpi_InternalSocket));
	if (m_pAcceptInternalSocket)
	{
		ZeroMemory(m_pAcceptInternalSocket, sizeof(Vpi_InternalSocket));
		m_pAcceptInternalSocket->rawSocket = (Vpi_RawSocket)INVALID_SOCKET;
		if (!m_pListenInternalSocket)
		{
			m_pListenInternalSocket = (Vpi_InternalSocket*)malloc(sizeof(Vpi_InternalSocket));
			// Check that something was send by the application
			if ((m_uiProtocolType != IPPROTO_TCP) && (m_uiProtocolType != IPPROTO_HOPOPTS))
				m_uiProtocolType = IPPROTO_TCP;
			if (Socket(AF_INET, m_nSocketType, m_uiProtocolType))// IPPROTO_HOPOPTS for UDP, IPPROTO_TCP for TCP
			{
#ifdef WIN32
				DWORD dwError;
				dwError = WSAGetLastError();
#endif
				int one = 1;
				setsockopt((SOCKET)m_hSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof(one));
				if (!Bind(m_uiPort, Vpi_String_GetRawString(&m_szIpAddress)))
				{
					uStatus = Vpi_BadNotConnected;
					Close();
				}
#ifdef WIN32
				dwError = WSAGetLastError();
#endif
			}
		}
	}
	return uStatus;
}

Vpi_StatusCode CRtSocket::CreateServer(UINT nSocketPort, LPCSTR lpszSocketAddress)
{
	Vpi_StatusCode uStatus = Vpi_Good;
	Vpi_String aString;
	if (lpszSocketAddress)
	{

		Vpi_String_Initialize(&aString);
		Vpi_String_AttachCopy(&aString, lpszSocketAddress);
		SetIpAddress(aString);
		m_uiPort = nSocketPort;
		uStatus=CreateServer();
		if (uStatus != Vpi_Good)
		{
			free(m_pListenInternalSocket);
			m_pListenInternalSocket = Vpi_Null;
		}
#ifdef WIN32
		DWORD dwError;
		dwError = WSAGetLastError();
#endif
	}
	else
		uStatus=Vpi_BadInvalidArgument;
	return uStatus;
}

/// <summary>
/// Creates the client side communication.
/// </summary>
/// <returns>Vpi_StatusCode</returns>
Vpi_StatusCode CRtSocket::CreateClient()
{
	Vpi_StatusCode uStatus=Vpi_Bad;

	//m_nSocketType=iProtoType;
	if (Socket(AF_INET, m_nSocketType, m_uiProtocolType))//IPPROTO_TCP
	{
		uStatus=Connect(m_uiPort, Vpi_String_GetRawString(&m_szIpAddress));
	}
	return uStatus;
}


Vpi_StatusCode CRtSocket::CreateClient (UINT nSocketPort, LPCSTR lpszSocketAddress)
{
	Vpi_StatusCode uStatus = Vpi_Good;
	m_uiPort = nSocketPort;
	Vpi_String aString;
	if (lpszSocketAddress)
	{

		Vpi_String_Initialize(&aString);
		Vpi_String_AttachCopy(&aString, lpszSocketAddress);
		SetIpAddress(aString);
		uStatus=CreateClient();
	}
	return uStatus;
}

Vpi_StatusCode CRtSocket::Connect()
{
	SOCKADDR_IN sockAddr;
	Vpi_StatusCode uStatus=Vpi_Good;
	SOCKET aSocket=(SOCKET)m_hSocket;
	if (m_nTimeout > 0)
	{
		int err;
#ifdef _GNUC_
		timeval aTimeVal;
		if (m_nTimeout > 1000)
		{
			aTimeVal.tv_sec = m_nTimeout / 1000;
			aTimeVal.tv_usec = 0;
		}
		else
			aTimeVal.tv_usec = m_nTimeout ; //* 1000;
		err = setsockopt(aSocket, SOL_SOCKET, SO_RCVTIMEO, (void *)&aTimeVal, sizeof(aTimeVal));
#endif
#ifdef WIN32
		err = setsockopt(aSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&m_nTimeout, sizeof(m_nTimeout));
#endif
		if (err != NO_ERROR) 
		{
		#ifdef WIN32
			int iWsaError = WSAGetLastError();
			uStatus=Vpi_Bad;
		#endif
		#ifdef _GNUC_
			switch(errno)
			{
			case EINVAL:
			case EBADF:
			case ENOPROTOOPT:
			case ENOTSOCK:
				uStatus=Vpi_BadInvalidArgument;
				break;
			case EFAULT:
				uStatus=Vpi_BadInternalError;
				break;
			default:
				uStatus=Vpi_Bad;
				break;
			}
			Vpi_Trace(Vpi_Null,VPI_TRACE_EXTRA_LEVEL_ERROR,"CRtSocket::Connect error on setsockopt 1>[%u]-%s\n",errno,strerror(errno));
		#endif
			Close();
			return uStatus;
		}
#ifdef _GNUC_
		err = setsockopt( aSocket, SOL_SOCKET, SO_SNDTIMEO, (void *)&aTimeVal, sizeof(aTimeVal));
#endif
#ifdef WIN32
		err = setsockopt(aSocket, SOL_SOCKET, SO_SNDTIMEO, (char *)&m_nTimeout, sizeof(m_nTimeout));
#endif
		if (err != NO_ERROR) 
		{
		#ifdef WIN32
			uStatus=Vpi_Bad;
		#endif
		#ifdef _GNUC_
			switch(errno)
			{
			case EINVAL:
			case EBADF:
			case ENOPROTOOPT:
			case ENOTSOCK:
				uStatus=Vpi_BadInvalidArgument;
				break;
			case EFAULT:
				uStatus=Vpi_BadInternalError;
				break;
			default:
				uStatus=Vpi_Bad;
				break;
			}
			Vpi_Trace(Vpi_Null,VPI_TRACE_EXTRA_LEVEL_ERROR,"CRtSocket::Connect error on setsockopt 2>[%u]-%s\n",errno,strerror(errno));
		#endif
			Close();
			return uStatus;
		}
	}


	sockAddr.sin_family		= m_nAddressFamily;
	sockAddr.sin_addr.s_addr   = inet_addr(m_szIpAddress.strContent);
	sockAddr.sin_port			= htons((u_short) m_uiPort);

	if (connect(aSocket,(SOCKADDR *)&sockAddr,sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
#ifdef _GNUC_
		Vpi_Trace(Vpi_Null,VPI_TRACE_EXTRA_LEVEL_ERROR, "CRtSocket::Connect connect error[%u]-%s\n", errno, strerror(errno));
#endif
#ifdef WIN32
		DWORD dwError=WSAGetLastError();
		Vpi_Trace(Vpi_Null, VPI_TRACE_EXTRA_LEVEL_ERROR, "CRtSocket::Connect connect error[%u]\n", dwError);
#endif
		Close();
		uStatus= Vpi_Bad;
	}
	else
		m_hSocket = (Vpi_RawSocket)aSocket;

	return uStatus;
}

Vpi_StatusCode CRtSocket::Connect(UINT nSocketPort, LPCSTR lpszSocketAddress, int nAddressFamily)
{
	m_uiPort = nSocketPort;
	m_nAddressFamily = nAddressFamily;

	return Connect();
}

Vpi_Boolean CRtSocket::Listen(int nConnectionBacklog)
{
	SOCKET aSocket=(SOCKET)m_hSocket;
	if (listen(aSocket,nConnectionBacklog) == SOCKET_ERROR)
	{
		Close();
		//DWORD dwLastError = GetLastError();
		return FALSE;
	}
	return TRUE;
}



Vpi_Boolean CRtSocket::Bind(UINT nSocketPort, LPCSTR lpszSocketAddress)
{
	SOCKET aSocket=(SOCKET)m_hSocket;
	SOCKADDR_IN sockAddr;
	memset(&sockAddr,0,sizeof(sockAddr));

	LPSTR lpszAscii = (LPSTR)lpszSocketAddress;
	sockAddr.sin_family = AF_INET;
	//sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (lpszAscii == NULL)
		sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	else
	{
		DWORD lResult = inet_addr(lpszAscii);
		if (lResult == INADDR_NONE)
		{
#ifdef WIN32
			WSASetLastError(WSAEINVAL);
#endif
			return FALSE;
		}
		sockAddr.sin_addr.s_addr = lResult;
	}

	sockAddr.sin_port = htons((u_short)nSocketPort);

	if(bind(aSocket,(SOCKADDR*)&sockAddr, sizeof(sockAddr)) == SOCKET_ERROR)
	{
#ifdef WIN32
		DWORD dwLastError;
		dwLastError = WSAGetLastError();
#endif
		Close();
		return FALSE;
	}
	return TRUE;
	
}

Vpi_Boolean CRtSocket::Socket(int nAddressFormat, int nSocketType ,int nProtocolType)
{
	Vpi_Boolean bRes=Vpi_True;
	SOCKET aSocket=(SOCKET)m_hSocket;
	if (aSocket == INVALID_SOCKET)
	{//"Your socket is already initialized"

		aSocket = socket(nAddressFormat, nSocketType, nProtocolType);
		//TRACE_LOG("Handle Socket : %ld",m_hSocket);
		if (aSocket == INVALID_SOCKET)
		{
			//TRACE_LOG("Unable to Create Socket : %ld", GetLastError());
			aSocket = INVALID_SOCKET;
			bRes = FALSE;
		}
		else
			m_hSocket = (Vpi_RawSocket)aSocket;
	}
	else
		m_hSocket = (Vpi_RawSocket)aSocket;
	return bRes;
}

Vpi_RawSocket CRtSocket::Accept(struct sockaddr* lpSockAddr, int* lpSockAddrLen)
{
	Vpi_StatusCode uStatus=Vpi_Good;
	Vpi_UInt32*			pAddress=0;
	Vpi_RawSocket			AcceptedRawSocket = (Vpi_RawSocket)VPI_P_SOCKET_INVALID;
	//Vpi_InternalSocket*   pListenInternalSocket = (Vpi_InternalSocket*)a_pListenSocket;
	Vpi_Boolean			bNagleOff=Vpi_True;
	Vpi_Boolean			bKeepAliveOn=Vpi_False;
	SOCKET aSocket = (SOCKET)m_hSocket;
	AcceptedRawSocket = Vpi_RawSocket_Accept(m_hSocket, &m_uiPort, pAddress, bNagleOff, bKeepAliveOn);
	if(AcceptedRawSocket == (Vpi_RawSocket)VPI_P_SOCKET_INVALID)
		uStatus=Vpi_BadCommunicationError;
	else
	{
		m_pAcceptInternalSocket->rawSocket=AcceptedRawSocket;
		/* inherit from parent (listen) socket */
		m_pAcceptInternalSocket->pfnEventCallback       = m_pListenInternalSocket->pfnEventCallback;
		m_pAcceptInternalSocket->pvUserData             = m_pListenInternalSocket->pvUserData;
		VPI_SOCKET_SETVALID(m_pAcceptInternalSocket);
		m_pAcceptInternalSocket->Flags.bSocketIsInUse   = Vpi_True;
		m_pAcceptInternalSocket->Flags.bIsListenSocket  = Vpi_False;
		m_pAcceptInternalSocket->Flags.bIsShutDown      = Vpi_False;
		m_pAcceptInternalSocket->Flags.bOwnThread       = Vpi_False;
		m_pAcceptInternalSocket->Flags.EventMask        =   VPI_SOCKET_READ_EVENT
														| VPI_SOCKET_EXCEPT_EVENT
														| VPI_SOCKET_ACCEPT_EVENT
														| VPI_SOCKET_CLOSE_EVENT
														| VPI_SOCKET_TIMEOUT_EVENT;
		m_pAcceptInternalSocket->pSocketManager         = m_pListenInternalSocket->pSocketManager;
		m_pAcceptInternalSocket->usPort                 = m_pListenInternalSocket->usPort;
		m_pAcceptInternalSocket->Flags.bSSL             = m_pListenInternalSocket->Flags.bSSL;
	}
	return m_pAcceptInternalSocket->rawSocket;
}

Vpi_StatusCode	CRtSocket::AttachRawSocket(Vpi_RawSocket hSocket)
{
	Vpi_StatusCode uStatus = Vpi_Good;
	if (m_hSocket != (Vpi_RawSocket)INVALID_SOCKET)
		uStatus = Vpi_AlreadyInitialized;
	else
	{
		if ((SOCKET)hSocket != INVALID_SOCKET)
			m_hSocket = hSocket;
		else
			uStatus = Vpi_BadInvalidArgument;
	}
	return uStatus;
}

int CRtSocket::ReceiveLine(char* lpBuf, int nBufLen)
{
	int nIndex = 0;

	if (m_nTimeout > 0)
	{
		m_pThreadTimeout = new CRtSocketTimeout();
		m_pThreadTimeout->SetTimeOut(m_nTimeout);
		m_pThreadTimeout->SetSocket(m_hSocket);
		m_pThreadTimeout->Run();
	}

	do
	{
		int nNumBytesRead = Receive(&lpBuf[nIndex], 1);
		if (nNumBytesRead == -1 || nNumBytesRead == 0)
		{
			//DWORD dwLastError = GetLastError();
			return nNumBytesRead;
		}
	}while(lpBuf[nIndex] != 10 && nIndex++ < nBufLen); //Carriage return;

	if (m_nTimeout > 0)
	{
		m_pThreadTimeout->Quit();
	}

	return nIndex+1; // Exit with Carriage return not increment the index
}

int CRtSocket::Receive(void* lpBuf, int nBufLen, int nFlags)
{
	int iWsaError;
	SOCKET aSocket=(SOCKET)m_hSocket;
	iWsaError = recv(aSocket, (LPSTR)lpBuf, nBufLen, nFlags);
//	if (iWsaError == SOCKET_ERROR)
//	{
//#ifdef WIN32
//		iWsaError = WSAGetLastError();
//#endif
//		iWsaError = errno;
//		switch (iWsaError)
//		{
//		case ECONNRESET:
//			iWsaError = Vpi_BadCommunicationError;
//			break;
//		case EINVAL:
//		case EBADF:
//		case EDESTADDRREQ:
//		case ENOTSOCK:
//		case EFAULT:
//			iWsaError = Vpi_BadInvalidArgument;
//			break;
//		case ENOBUFS:
//		case EMSGSIZE:
//			iWsaError = Vpi_BadInternalError;
//			break;
//		default:
//			iWsaError = Vpi_Bad;
//			break;
//		}
//	}
//	else
//		iWsaError = Vpi_Good;
	return iWsaError;
}

int CRtSocket::ReceiveFrom(void* lpBuf, int* nBufLen, int nFlags)
{
	int nRet;
	SOCKET aSocket=(SOCKET)m_hSocket;
	SOCKADDR_IN sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons((u_short)m_uiPort);
#ifdef WIN32
	int sockAddrInLen = sizeof(struct sockaddr_in);
	nRet = recvfrom(aSocket, (LPSTR)lpBuf, *nBufLen, 0, (SOCKADDR*)&sockAddr, &sockAddrInLen);
#endif
#ifdef _GNUC_

	socklen_t sockAddrInLen = sizeof(struct sockaddr_in);
	nRet = recvfrom(aSocket, (LPSTR)lpBuf, *nBufLen, 0, (SOCKADDR*)&sockAddr, &sockAddrInLen);
#endif
	if (nRet!=-1)
		m_LastPeerSockAddr = sockAddr;
	return nRet;
}
int CRtSocket::SendTo(const void *buf, int len, int flags)
{
	int nRet;
	SOCKET aSocket=(SOCKET)m_hSocket;
	int sockAddrInLen = sizeof(struct sockaddr_in);

	//memset(&sockAddr, 0, sizeof(sockAddr));
	nRet = sendto(aSocket, (char*)buf, len, flags, (SOCKADDR*)&m_LastPeerSockAddr, sockAddrInLen);

	return nRet;
}
int CRtSocket::Send(const void* lpBuf, int nBufLen, int nFlags)
{
	SOCKET aSocket=(SOCKET)m_hSocket;
	int iWsaError = 0;
	errno = 0;
	iWsaError=send(aSocket, (LPSTR)lpBuf, nBufLen, nFlags);
	if (iWsaError == SOCKET_ERROR)
	{
#ifdef WIN32
		iWsaError =WSAGetLastError();
#endif
		iWsaError = errno;
		switch (iWsaError)
		{
		case ECONNRESET:
			iWsaError = Vpi_BadCommunicationError;
			break;
		case EINVAL:
		case EBADF:
		case EDESTADDRREQ:
		case ENOTSOCK:
		case EFAULT:
			iWsaError = Vpi_BadInvalidArgument;
			break;
		case ENOBUFS:
		case EMSGSIZE:
			iWsaError = Vpi_BadInternalError;
			break;
		default:
			iWsaError = Vpi_Bad;
			break;
		}
	}
	else
		iWsaError = Vpi_Good;
	return iWsaError;
}

Vpi_StatusCode CRtSocket::IOCtl( long lCommand, DWORD* lpArgument )
{
	Vpi_StatusCode uStatus=Vpi_Good;
	SOCKET aSocket=(SOCKET)m_hSocket;
#ifdef WIN32
	if (ioctlsocket(aSocket,lCommand,lpArgument)!=0)
		uStatus=Vpi_BadCommunicationError;
#endif
#ifdef _GNUC_
	int              flags       = 0;
	flags = fcntl(aSocket,lCommand,lpArgument);
	if(flags == -1)
		uStatus= Vpi_BadCommunicationError;
#endif
	return uStatus;
}
Vpi_UInt16 CRtSocket::GetPort()
{
	return m_uiPort;
}
void	CRtSocket::SetPort(Vpi_UInt16 dwPort)
{
	m_uiPort = dwPort;
}
void	CRtSocket::SetAddressFamily(Vpi_Int32 nAddressFamily)
{ 
	m_nAddressFamily  = nAddressFamily;
}
Vpi_Int32 CRtSocket::GetSocketType()
{
	return m_nSocketType;
}
// Set the SocketType could be SOCK_STREAM, SOCK_DGRAM, etc.
void	CRtSocket::SetSocketType(Vpi_Int32 nSocketType)
{ 
	m_nSocketType = nSocketType;
}
void	CRtSocket::SetTimeout(Vpi_Int32 nTimeout){ m_nTimeout = nTimeout; }

Vpi_String	CRtSocket::GetIPAddress()
{
	return m_szIpAddress;
}
void	CRtSocket::SetIpAddress(Vpi_String szIpAddress) 
{ 
	Vpi_String_Initialize(&m_szIpAddress);
	Vpi_String_CopyTo(&szIpAddress,&m_szIpAddress);
}
Vpi_RawSocket CRtSocket::GetRawSocket()
{
	return m_hSocket;
}
Vpi_UInt32 CRtSocket::GetProtocolType()
{
	return m_uiProtocolType;
}
// Set the ProtocolType Could be IPPROTO_TCP, IPPROTO_UDP, etc
void CRtSocket::SetProtocolType(Vpi_UInt32 uiProtocolType)
{
	m_uiProtocolType = uiProtocolType;
}

Vpi_Boolean CRtSocket::IsSocketOk()
{
	return (((SOCKET)m_hSocket) != INVALID_SOCKET);
}
Vpi_Boolean CRtSocket::IsSet(Vpi_P_Socket_Array* pFdSet)
{
	return Vpi_RawSocket_FD_Isset(m_hSocket, pFdSet);
}
CRtSocketTimeout::CRtSocketTimeout() : CRtThread()
{
	m_hSocket = NULL;

	m_bWaitEnd = TRUE;
	Vpi_String_AttachCopy(&m_szName,"Socket Timeout");
	m_bAutoDelete = TRUE;
}

CRtSocketTimeout::~CRtSocketTimeout()
{

}

BOOL CRtSocketTimeout::ExitInstance()
{
	BOOL bStatus = TRUE;

	bStatus = CRtThread::ExitInstance();

	return bStatus;
}

DWORD CRtSocketTimeout::Main(void *pParam)
{
	Vpi_StatusCode uStatus= Vpi_Semaphore_TimedWait(m_hEventRunSem,m_dwTimeOut);
	
	if (uStatus == Vpi_GoodNonCriticalTimeout)
	{
		Vpi_RawSocket_Close(m_hSocket);
	}

	return 0;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Sets tcpno delay. </summary>
///
/// <remarks>	Michel, 22/06/2016. </remarks>
///
/// <param name="bNoDelay">	The no delay. </param>
///-------------------------------------------------------------------------------------------------

void CRtSocket::SetTcpnoDelay(Vpi_Boolean bNoDelay)
{
	SOCKET aSocket = (SOCKET)m_hSocket;
	if (aSocket != INVALID_SOCKET)
		setsockopt(aSocket, IPPROTO_TCP, TCP_NODELAY, (char *)&bNoDelay, sizeof(int));
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Gets sock option. </summary>
///
/// <remarks>	Michel, 30/06/2016. </remarks>
///
/// <param name="s">	  	The SOCKET to process. </param>
/// <param name="level">  	The level. </param>
/// <param name="optname">	The optname. </param>
/// <param name="optval"> 	[in,out] If non-null, the optval. </param>
/// <param name="optlen"> 	[in,out] If non-null, the optlen. </param>
///
/// <returns>	The sock option. </returns>
///-------------------------------------------------------------------------------------------------

int CRtSocket::GetSockOpt(Vpi_RawSocket rawSocket, int level, int optname, int *optval, int *optlen)
{
	SOCKET s = (SOCKET)rawSocket;
	int optVal;
#ifdef WIN32
	int optLenTmp = sizeof(int);
	int iRes = getsockopt(s, level, optname, (char*)&optVal, &optLenTmp);
#endif 

#ifdef _GNUC_
	socklen_t optLenTmp=sizeof(int);
	int iRes = getsockopt(s, level, optname, (char*)&optVal, &optLenTmp);
#endif 
	if (iRes == SOCKET_ERROR)
	{
#ifdef WIN32
		int wsaError = WSAGetLastError();
		Vpi_Trace(Vpi_Null, VPI_TRACE_EXTRA_LEVEL_ERROR, "WSAGetLastError=%u\n", wsaError);
#endif 
	}
	else
		*optval = optVal;
	return iRes;
}

