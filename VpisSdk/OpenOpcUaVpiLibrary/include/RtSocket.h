//**************************************************************************
//
//  Copyright (c) 4CE Industry 1999-2014, All Rights Reserved
//
//**************************************************************************
//
//  Filename   :  RtSocket.h
//  $Author    :  Michel Condemine
//
//  Description:  This file is part of the OpenOpcUaLibrary. 
//                This compoment and all related file are not Open Source
//				  This must be use with the autorisation of Michel Condemine
//**************************************************************************
// RtSocket.h: interface for the CRtSocket class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RTSOCKET_H__FB691884_C222_11D1_AF04_006097758E14__INCLUDED_)
#define AFX_RTSOCKET_H__FB691884_C222_11D1_AF04_006097758E14__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

//#include "winsock2.h"
#include "VpiThread.h"
#ifdef WIN32 
	#define	VPI_EXPORT __declspec(dllexport)
	#define VPI_IMPORT __declspec(dllimport)
#endif
#ifdef _GNUC_
	#define VPI_EXPORT
	#define VPI_IMPORT
#endif
class CRtSocketTimeout;
/// <summary>
/// Class to handle Sockets. It can be TCP or UDP socket. Client or Server
/// </summary>
class VPI_EXPORT CRtSocket  
{
public:
	CRtSocket();
	~CRtSocket();
	Vpi_Boolean IsSocketOk();
	Vpi_Boolean IsSet(Vpi_P_Socket_Array* pFdSet);
	// Methods
public:
	Vpi_StatusCode	CreateServer();
	Vpi_StatusCode	CreateServer(UINT nSocketPort,LPCSTR lpszSocketAddress);
	Vpi_StatusCode	CreateClient();
	Vpi_StatusCode	CreateClient(UINT nSocketPort, LPCSTR lpszSocketAddress);
	Vpi_StatusCode	AttachRawSocket(Vpi_RawSocket hSocket);
	Vpi_RawSocket			GetRawSocket();
	Vpi_UInt16			GetPort();
	void					SetPort(Vpi_UInt16 dwPort);
	void					SetAddressFamily(Vpi_Int32 nAddressFamily);
	Vpi_Int32				GetSocketType();
	void					SetSocketType(Vpi_Int32 nSocketType);
	void					SetTimeout(Vpi_Int32 nTimeout);
	Vpi_UInt32			GetProtocolType();
	void					SetProtocolType(Vpi_UInt32 uiProtocolType);
	Vpi_String			GetIPAddress();
	void					SetIpAddress(Vpi_String szIpAddress);
	Vpi_Boolean			Listen(int nConnectionBacklog = 5);
	Vpi_Boolean			Socket(int nAddressFormat = AF_INET, int nSocketType=SOCK_STREAM,int nProtocolType = 0);
	int						Receive(void* lpBuf, int nBufLen, int nFlags=0);
	int						ReceiveFrom(void* lpBuf, int* nBufLen, int nFlags = 0);
	int						SendTo(const void *buf, int len, int flags);
	int						ReceiveLine(char* lpBuf, int nBufLen);
	int						Send(const void* lpBuf, int nBufLen, int nFlags=0);
	Vpi_Boolean			Bind(UINT nSocketPort, LPCSTR lpszSocketAddress);
	Vpi_RawSocket			Accept(struct sockaddr* lpSockAddr = NULL, int* lpSockAddrLen = NULL);
	void					Close();
	Vpi_StatusCode	IOCtl( long lCommand, DWORD* lpArgument );
	void					SetTcpnoDelay(Vpi_Boolean bNoDelay);
	//int         SetSockOpt(int nLevel, int nOptName, char *lpOpVal, int nOptLen);
	int						GetSockOpt(Vpi_RawSocket rawSocket,int level,int optname,int *optval,int *optlen);
private:
	Vpi_StatusCode	Connect();
	Vpi_StatusCode	Connect(UINT nSocketPort, LPCSTR lpszSocketAddress, int nAddressFormat = AF_INET);
	//Properties
protected:
	Vpi_InternalSocket*   m_pListenInternalSocket;
	Vpi_InternalSocket*   m_pAcceptInternalSocket;
	Vpi_RawSocket			m_hSocket;
	Vpi_UInt16			m_uiPort;
	Vpi_String			m_szIpAddress;
	Vpi_Int32				m_nAddressFamily;
	Vpi_Int32				m_nSocketType; // SocketType could be SOCK_STREAM, SOCK_DGRAM, etc.
	Vpi_Int32				m_nTimeout;
	Vpi_UInt32			m_uiProtocolType; // ProtocolType Could be IPPROTO_TCP, IPPROTO_UDP, etc
	CRtSocketTimeout*		m_pThreadTimeout;
	SOCKADDR_IN				m_LastPeerSockAddr; // Information about the last peer the connect to the UDP Binded port when 
};


class CRtSocketTimeout : public CRtThread  
{
public:
	CRtSocketTimeout();
	virtual ~CRtSocketTimeout();

protected:
	Vpi_RawSocket	m_hSocket;

public:
	inline void SetSocket(Vpi_RawSocket hSocket) { m_hSocket = hSocket;}

	virtual DWORD Main(void *pParam=NULL);
	virtual BOOL ExitInstance();
};

#endif // !defined(AFX_RTSOCKET_H__FB691884_C222_11D1_AF04_006097758E14__INCLUDED_)
