//**************************************************************************
//
//  Copyright (c) 4CE Industry 1999-2014, All Rights Reserved
//
//**************************************************************************
//
//  Filename   :  SerialPort.cpp
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
#include "SerialPort.h"
#ifdef _GNUC_
#include <termios.h>
#endif
using namespace VpiBuiltinType;
CSerialPort::CSerialPort(void)
{
	Vpi_Mutex_Create( &m_PortMutex );
	m_bEnable = TRUE;
	m_bReInitialize = TRUE;
	m_uiBaud = 9600;
	m_uiDataBits = 8;
	m_uiParity = NO_PARITY;
	m_flow = HARDWARE;
	m_uiStop = STOP10BIT;
	m_hPort = Vpi_Null;
	m_uiParity  = 0;
	m_SerialType=SERIAL_NATIVE;
#ifdef WIN32
	m_cts.ReadIntervalTimeout = 0;
	m_cts.ReadTotalTimeoutMultiplier = 1;
	m_cts.ReadTotalTimeoutConstant = 500;
	m_cts.WriteTotalTimeoutMultiplier = 1;
	m_cts.WriteTotalTimeoutConstant = 500;
#endif
	m_pPortName=(Vpi_String*)malloc(sizeof(Vpi_String));
	Vpi_String_Initialize(m_pPortName);
}

CSerialPort::~CSerialPort(void)
{
	Vpi_Mutex_Delete( &m_PortMutex );
	if (m_hPort)
		Close();
	Vpi_String_Clear(m_pPortName);
	free(m_pPortName);
}
Vpi_StatusCode CSerialPort::Close()
{
	Vpi_StatusCode uStatus=Vpi_Good;
#ifdef WIN32
	if (CloseHandle(m_hPort))
		m_hPort=NULL;
	else
		uStatus=Vpi_Bad;
#endif
#ifdef _GNUC_
	if (close((int)m_hPort)==-1)
		uStatus=Vpi_Bad;
#endif
	return uStatus;

}
//*******************************************************************
Vpi_Boolean CSerialPort::Initialize()
{
	Vpi_Boolean bResult=Vpi_False;
	Vpi_Mutex_Lock(m_PortMutex ); 
#ifdef WIN32
	// Is first init ?
	if( m_hPort == Vpi_Null )
	{
		wchar_t* szPortName=Vpi_Null;
		szPortName=(wchar_t*)malloc(m_pPortName->uLength+1);
		if (szPortName)
		{
			ZeroMemory(szPortName, m_pPortName->uLength + 1);
			Vpi_CharA* pPortName = Vpi_String_GetRawString(m_pPortName);
			if (pPortName)
			{
				m_hPort = CreateFileA(pPortName,
					GENERIC_READ | GENERIC_WRITE,
					0,
					NULL,   // security
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL,
					NULL);
				if (m_hPort == INVALID_HANDLE_VALUE)
				{
					DWORD dwError=GetLastError();
					m_hPort = Vpi_Null;
					bResult = Vpi_False;
				}
				else
					m_bReInitialize = Vpi_True;
			}
		}
	}
	if( m_hPort != Vpi_Null )
	{
		if( m_bReInitialize == Vpi_False )
		  bResult = Vpi_True;
		else
		{
			DCB dcb;
			SecureZeroMemory(&dcb, sizeof(DCB));
			dcb.DCBlength = sizeof(DCB);

			if (!GetCommState(m_hPort, &dcb)) 
			{
				DWORD dwError = GetLastError();
				m_hPort = NULL;
				bResult = Vpi_False;
			}
			else
			{
				dcb.BaudRate = m_uiBaud;
				//dcb.fBinary = TRUE;
				//dcb.fParity = NOPARITY. //(m_wParity==NOPARITY ? FALSE : TRUE);
				//dcb.fNull = FALSE;
				dcb.ByteSize = (BYTE)m_uiDataBits;
				dcb.Parity = NOPARITY; //(BYTE)m_wParity;
				dcb.StopBits = ONESTOPBIT; // (BYTE)m_wStop;
				//dcb.fOutxCtsFlow = (m_flow==HARDWARE ? TRUE : FALSE);
				//dcb.fRtsControl = (m_flow==HARDWARE ? RTS_CONTROL_HANDSHAKE : RTS_CONTROL_DISABLE);

				if( !SetCommState( m_hPort, &dcb ) )
				{
					//dwError=GetLastError();
					bResult = Vpi_False;
				}
				else
				{
					m_cts.ReadTotalTimeoutConstant = 500;
					m_cts.WriteTotalTimeoutConstant = 500;
					if( !SetCommTimeouts( m_hPort, &m_cts ) )
					{
						//DWORD dwError=GetLastError();
						bResult = Vpi_False;
					}
					else
						m_bReInitialize = FALSE;
				}
			}
		}
	}
#endif
#ifdef _GNUC_
		// Is first init ?
	if( m_hPort == Vpi_Null )
	{
		struct termios tty;
		wchar_t* szPortName=Vpi_Null;
		szPortName=(wchar_t*)malloc(m_pPortName->uLength+1);
		ZeroMemory(szPortName,m_pPortName->uLength+1);
		Vpi_CharA* pPortName=Vpi_String_GetRawString(m_pPortName);
		m_hPort=(Vpi_Handle)open(pPortName,O_RDWR | O_NOCTTY );
		if (m_hPort>0)
		{
			// Get the current termios state
			if (tcgetattr((int)m_hPort,&tty)==0)
			{
				// Fill in the tty structure with the requested parameters
				switch (m_uiBaud)
				{
				case 9600:
					tty.c_cflag|=B9600;
					break;
				case 19200:
					tty.c_cflag|=B19200;
					break;
				case 38400:
					tty.c_cflag|=B38400;
					break;
				case 57600:
					tty.c_cflag|=B57600;
					break;
				case 115200:
					tty.c_cflag|=B115200;
					break;
				default:
					tty.c_cflag|=B38400;
					break;
				}
				// N,8,1 
				if (m_uiDataBits==8)
					tty.c_cflag|=CS8;
				if (m_uiDataBits==7)
					tty.c_cflag|=CS7;
				//m_uiParity;
				if (m_uiStop==STOP20BITS)
					tty.c_cflag|=CSTOPB ;
				// Set the port params
				if (tcsetattr((int)m_hPort,TCSANOW, &tty)==0)
				{
					m_bReInitialize = Vpi_True;
					bResult = Vpi_True;
				}
			}
		}
	}
#endif
	Vpi_Mutex_Unlock(m_PortMutex );
	return bResult;
}
Vpi_String* CSerialPort::GetPortName()	
{
	return m_pPortName;
}  
void CSerialPort::SetPortName(Vpi_String* strVal) 
{
	if (strVal)
		Vpi_String_StrnCpy(m_pPortName,strVal,Vpi_String_StrLen(strVal));
}
Vpi_UInt16 CSerialPort::GetParity()	
{
	return m_uiParity;
}      // EVENPARITY, etc.
// Set the parity
// ui16Val can be 0 = none, 1 = odd, 2 = even
void CSerialPort::SetParity(Vpi_UInt16 ui16Val) 
{
	switch (ui16Val)
	{
	case 0:
	case 1:
	case 2:
		m_uiParity=ui16Val;
		break;
	default:
		m_uiParity=0; // none
		break;
	}
}
// RTS, CTS. Linux value are
// CCTS_OFLOW CRTS_IFLOW 
//	HARDWARE, 
//	XON_XOFF, 
//	NONE 
FLOW CSerialPort::GetFlow()	
{
	return m_flow;
}
void CSerialPort::SetFlow(FLOW flow) 
{
	switch (flow)
	{
	case HARDWARE:
	case NONE:
	case XON_XOFF:
		m_flow=flow;
		break;
	default:
		m_flow=NONE;
		break;
	}
	
}
Vpi_UInt16 CSerialPort::GetStop()	
{
	return m_uiStop;
}        // defined constants
// set the bit stop value
// possible value are 10 (1 stop),15 (1.5 stops),20 (2 stops)
void CSerialPort::SetStop(Vpi_UInt16 uiVal) 
{
	switch (uiVal)
	{
	case 10:
		m_uiStop=STOP10BIT;
		break;
	case 15:
		m_uiStop=STOP15BITS;
		break;
	case 20:
		m_uiStop=STOP20BITS;
		break;
	default:
		m_uiStop=STOP10BIT;
		break;
	}
}
// 7 or 8
Vpi_UInt16 CSerialPort::GetDataBits()	
{
	return m_uiDataBits;  
}  
void CSerialPort::SetDataBits(Vpi_UInt16 uiVal) 
{
	switch (uiVal)
	{
	case 7:
	case 8:
		m_uiDataBits=uiVal;
	default:
		m_uiDataBits=8;
		break;
	}
}
Vpi_Mutex CSerialPort::GetPortMutex()	
{
	return m_PortMutex;
}
void CSerialPort::SetHPort(Vpi_Handle hPort) 
{
	m_hPort=hPort;
}
// fill handle for port
Vpi_Handle CSerialPort::GetHPort()	
{
	return m_hPort;
}    
// if settings have changed    
Vpi_Boolean CSerialPort::IsReInitialize()	
{
	return m_bReInitialize;
} 
void CSerialPort::SetReInitialize(Vpi_Boolean bVal) 
{
	m_bReInitialize=bVal;
}
SERIAL_TYPE	CSerialPort::GetSerialType() 
{
	return m_SerialType;
}
void CSerialPort::SetSerialType(SERIAL_TYPE eSerialType) 
{
	m_SerialType=eSerialType;
}
Vpi_Boolean CSerialPort::IsEnable()		
{
	return m_bEnable;
}
void CSerialPort::SetEnable(Vpi_Boolean bVal) 
{
	m_bEnable=bVal;
}
Vpi_UInt32 CSerialPort::GetBaud()	
{
	return m_uiBaud;
}        
// the value, eg 9600
void CSerialPort::SetBaud(Vpi_UInt32 uiVal) 
{
	switch (uiVal)
	{
	case 9600:
	case 19200:
	case 38400:
	case 57600:
	case 115200:
	case 307200:
		m_uiBaud=uiVal;
		break;
	default:
		m_uiBaud=9600;
		break;
	}
}
#ifdef WIN32
COMMTIMEOUTS CSerialPort::GetCts()	
{
	return m_cts;
}
#endif