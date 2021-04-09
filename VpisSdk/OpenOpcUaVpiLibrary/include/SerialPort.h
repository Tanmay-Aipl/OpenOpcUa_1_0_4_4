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
#pragma once
typedef enum PARITY
{
	NO_PARITY,
	ODD_PARITY,
	EVEN_PARITY
} _PARITY;
typedef enum STOP
{
	STOP10BIT, // 1 stop bit
	STOP15BITS, // 1.5 stop bit
	STOP20BITS // 2 stop bits
} _STOP;
typedef enum FLOW 
{ 
	HARDWARE, 
	XON_XOFF, 
	NONE 
} _FLOW;
typedef enum SERIAL_TYPE
{
	SERIAL_NATIVE,
	SERIAL_USB
} _SERIAL_TYPE;
#ifdef WIN32 
	#define	VPI_EXPORT __declspec(dllexport)
#endif
#ifdef _GNUC_
	#define VPI_EXPORT
#endif

/// <summary>
/// Class to handle serial port
/// </summary>
class VPI_EXPORT CSerialPort
{
public:
	CSerialPort(void);
	~CSerialPort(void);
	Vpi_String*			GetPortName();        // used as file name to open port
	void					SetPortName(Vpi_String* strVal);
	Vpi_Boolean			IsEnable();
	void					SetEnable(Vpi_Boolean bVal);
	Vpi_UInt32			GetBaud();       // the value, eg 9600
	void					SetBaud(Vpi_UInt32 dwVal);
	Vpi_UInt16			GetDataBits();  // 7 or 8
	void					SetDataBits(Vpi_UInt16 wVal);
	Vpi_UInt16			GetParity();     // EVENPARITY, etc.
	void					SetParity(Vpi_UInt16 ui16Val); // ui16Val can be 0 = none, 1 = odd, 2 = even
	FLOW					GetFlow();
	void					SetFlow(FLOW flow);
	Vpi_UInt16			GetStop();        // defined constants
	void					SetStop(Vpi_UInt16 wVal);
	Vpi_Mutex			GetPortMutex();
	void					SetHPort(Vpi_Handle hPort);
	Vpi_Handle			GetHPort();        // file handle for port
	Vpi_Boolean			IsReInitialize(); // if settings have changed
	void					SetReInitialize(Vpi_Boolean bVal);
	SERIAL_TYPE				GetSerialType();
	void					SetSerialType(SERIAL_TYPE eSerialType);
#ifdef WIN32
	COMMTIMEOUTS			GetCts();
#endif
	Vpi_Boolean			Initialize(); // ouvre le port serie
	Vpi_StatusCode	Close(); // ferme le port serie
private:
	Vpi_String*			m_pPortName;        // used as file name to open port
	Vpi_Mutex			m_PortMutex;
	Vpi_Boolean			m_bEnable;
	Vpi_UInt32			m_uiBaud;        // the value, eg 9600
	Vpi_UInt16			m_uiDataBits;    // 7 or 8
	Vpi_UInt16			m_uiParity;      // EVEN, ODD, NONE PARITY.
	FLOW					m_flow;			 // CTS/RTS
	Vpi_UInt16			m_uiStop;        // defined constants
	Vpi_Handle			m_hPort;        // file handle for port
	Vpi_Boolean			m_bReInitialize; // if settings have changed
	SERIAL_TYPE				m_SerialType; // indique le type de port série : Native (100% serie), Emulate (USB via FDTI)
#ifdef WIN32
	COMMTIMEOUTS			m_cts;
#endif
};
