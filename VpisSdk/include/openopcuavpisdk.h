// Header file for the shared classes and function of the OpenOpcUaVpiSdk
#pragma once
#include "OpenOpcUaVpiOs.h"
#include "VpiDataTypes.h"
#include  "VPI.h"
#ifdef _GNUC_
	#define VPI_CDECL
	#define VPI_DLLCALL
	#define VPI_IMPORT
#else
	#define VPI_CDECL __cdecl
	/* call exported functions by stdcall convention */
	#define VPI_DLLCALL __stdcall
	#define VPI_IMPORT __declspec(dllimport)
#endif


#define MAKEWORD_OOUA(a, b)      ((WORD)(((BYTE)(((unsigned long)(a)) & 0xff)) | ((WORD)((BYTE)(((unsigned long)(b)) & 0xff))) << 8))
/* used ie. for unlimited timespans */
#define VPI_INFINITE 0xFFFFFFFF
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

#define VPI_TRACE_EXTRA_LEVEL_DEBUG   (VPI_TRACE_EXTRA_WARNING | VPI_TRACE_EXTRA_INFO | VPI_TRACE_EXTRA_ERROR )
#define VPI_TRACE_EXTRA_LEVEL_ERROR   (VPI_TRACE_EXTRA_ERROR)
#define VPI_TRACE_EXTRA_LEVEL_WARNING (VPI_TRACE_EXTRA_WARNING)
#define VPI_TRACE_EXTRA_LEVEL_INFO    (VPI_TRACE_EXTRA_INFO )
////////////////////////////////////////////////////////////////////
// Use to setup the trace itself
#define VPI_TRACE_OUTPUT_ALL_ERROR		(VPI_TRACE_EXTRA_LEVEL_DEBUG)
#define VPI_TRACE_OUTPUT_EXTRA_ERROR		VPI_TRACE_EXTRA_LEVEL_ERROR

#define VPI_TRACE_OUTPUT_ALL_DEBUG		 VPI_TRACE_EXTRA_LEVEL_DEBUG
#define VPI_TRACE_OUTPUT_EXTRA_DEBUG		VPI_TRACE_EXTRA_LEVEL_DEBUG

#define VPI_TRACE_OUTPUT_ALL_WARNING		VPI_TRACE_EXTRA_LEVEL_WARNING
#define VPI_TRACE_OUTPUT_EXTRA_WARNING	VPI_TRACE_EXTRA_LEVEL_WARNING

#define VPI_TRACE_OUTPUT_ALL_INFO			VPI_TRACE_EXTRA_LEVEL_INFO
#define VPI_TRACE_OUTPUT_EXTRA_INFO		VPI_TRACE_EXTRA_LEVEL_INFO

#define VPI_TRACE_OUTPUT_LEVEL_ALL     (0xFFFFFFFF)
#define VPI_TRACE_OUTPUT_LEVEL_NONE    (0x00000000)

//Writes the given string and the parameters to the trace device, if the given
// trace level is activated.
//
// Vpi_UInt32     a_uTraceLevel contient le code demandé par l'appelant
// Vpi_CharA*		a_sFormat chaine de caractère a formater
extern "C"
{
	Vpi_Boolean VPI_DLLCALL Vpi_Trace(void* pProxyStubConfiguration, Vpi_UInt32 a_uTraceLevel, const Vpi_CharA* a_sFormat, ...);
	Vpi_StatusCode VPI_DLLCALL Vpi_Trace_InitializePtr(void** pProxyStubConfiguration);
	Vpi_StatusCode VPI_DLLCALL Vpi_Trace_Initialize(void* pProxyStubConfiguration, Vpi_Int32 iTraceLevel, Vpi_Int32 iTraceOutput, Vpi_String TraceFileName, FILE** hTraceFile);
	Vpi_UInt32 VPI_DLLCALL Vpi_Trace_GetTraceLevel(void* pProxyStubConfiguration);
	Vpi_Void VPI_DLLCALL Vpi_Trace_SetTraceLevel(void* pProxyStubConfiguration, Vpi_UInt32 iTraceLevel);
	Vpi_UInt32 VPI_DLLCALL Vpi_Trace_GetTraceOutput(void* pProxyStubConfiguration);
	Vpi_Void VPI_DLLCALL Vpi_Trace_SetTraceOutput(void* pProxyStubConfiguration, Vpi_UInt32 iTraceOutput);
	///===========================================================================
	// Change the TraceFileName during runtime.
	//
	Vpi_Void VPI_DLLCALL Vpi_Trace_SetTraceFile(void* pProxyStubConfiguration, Vpi_String strFileName);

	///==========================================================================
	// Get the current TraceFileName during runtime.
	//
	Vpi_Void VPI_DLLCALL Vpi_Trace_GetTraceFile(void* pProxyStubConfiguration, Vpi_String* strFileName);
}

////////////////////////////////////////////////////////////////////////////
// 
// Support for String
//
////////////////////////////////////////////////////////////////////////////
#define VPI_STRINGLENZEROTERMINATED   0xffffffffL
#define VPI_STRING_LENDONTCARE        VPI_STRINGLENZEROTERMINATED
#define Vpi_uiMagic       0
extern "C"
{
	#define Vpi_String_CopyTo(xSource, xDestination) Vpi_String_StrnCpy(xDestination, xSource, VPI_STRING_LENDONTCARE)
	#define Vpi_String_Compare(xValue1, xValue2) Vpi_String_StrnCmp(xValue1, xValue2, VPI_STRING_LENDONTCARE, Vpi_False)
	Vpi_StatusCode VPI_DLLCALL Vpi_String_Initialize(Vpi_String* pString);
	Vpi_StatusCode VPI_DLLCALL Vpi_String_Clear(Vpi_String* a_pString);
	Vpi_StatusCode VPI_DLLCALL Vpi_String_StrnCpy(Vpi_String*       pDestString,
		const Vpi_String* pSrcString,
		Vpi_UInt32        uLength);
	Vpi_StatusCode VPI_DLLCALL Vpi_String_AttachCopy(Vpi_String* a_pDst, const Vpi_CharA* a_pSrc);
	Vpi_UInt32 VPI_DLLCALL Vpi_String_StrLen(const Vpi_String* pString);
	Vpi_CharA* VPI_DLLCALL Vpi_String_GetRawString(const Vpi_String* pString);
	Vpi_Boolean VPI_DLLCALL Vpi_String_IsNull(const Vpi_String* pString);
	Vpi_StatusCode VPI_DLLCALL Vpi_String_WtoA(Vpi_CharW* wStrIn, Vpi_CharA** aStrOut);
	Vpi_StatusCode VPI_DLLCALL Vpi_String_AtoW(const Vpi_CharA* aStrIn, Vpi_CharW** wStrOut);
	Vpi_Int32 VPI_DLLCALL Vpi_String_StrnCmp(const Vpi_String* pString1,
		const Vpi_String* pString2,
		Vpi_UInt32        uLength,
		Vpi_Boolean       bIgnoreCase);
}
////////////////////////////////////////////////////////////////////////////
// 
// Support for ByteString
//
////////////////////////////////////////////////////////////////////////////
extern "C"
{
	Vpi_Void VPI_DLLCALL Vpi_ByteString_Initialize(Vpi_ByteString* a_pValue);
	Vpi_Void VPI_DLLCALL Vpi_ByteString_Clear(Vpi_ByteString* a_pValue);
	Vpi_StatusCode VPI_DLLCALL Vpi_ByteString_CopyTo(const Vpi_ByteString* source, Vpi_ByteString* destination);
}
////////////////////////////////////////////////////////////////////////////
// 
// Support for DateTime
//
////////////////////////////////////////////////////////////////////////////
extern "C"
{
	Vpi_StatusCode VPI_DLLCALL Vpi_DateTime_GetStringFromDateTime(Vpi_DateTime  DateTime,
		Vpi_CharA*    pchBuffer,
		Vpi_UInt32    uLength);
	Vpi_DateTime  VPI_DLLCALL Vpi_DateTime_UtcNow();
}
////////////////////////////////////////////////////////////////////////////
// 
// Support for Variant
//
////////////////////////////////////////////////////////////////////////////
extern "C"
{
	Vpi_StatusCode VPI_DLLCALL Vpi_Variant_Initialize(Vpi_Variant* pVariant);
	Vpi_StatusCode VPI_DLLCALL Vpi_Variant_Clear(Vpi_Variant* pVariant);
	Vpi_StatusCode VPI_DLLCALL Vpi_Variant_CopyTo(const Vpi_Variant* pSource, Vpi_Variant* pDestination);
}

////////////////////////////////////////////////////////////////////////////
// 
// Support for DataValue
//
////////////////////////////////////////////////////////////////////////////
extern "C"
{
	Vpi_StatusCode VPI_DLLCALL Vpi_DataValue_Initialize(Vpi_DataValue* a_pValue);
	Vpi_StatusCode VPI_DLLCALL Vpi_DataValue_Clear(Vpi_DataValue* a_pValue);
	Vpi_StatusCode VPI_DLLCALL Vpi_DataValue_CopyTo(const Vpi_DataValue* pSource, Vpi_DataValue* pDestination);
}
////////////////////////////////////////////////////////////////////////////
// 
// Support for NodeId
//
////////////////////////////////////////////////////////////////////////////
extern "C"
{
	Vpi_Void VPI_DLLCALL Vpi_NodeId_Initialize(Vpi_NodeId* pValue);
	Vpi_Void VPI_DLLCALL Vpi_NodeId_Clear(Vpi_NodeId* pValue);
	Vpi_StatusCode VPI_DLLCALL Vpi_NodeId_CopyTo(const Vpi_NodeId* pSource, Vpi_NodeId* pDestination);
}
extern "C"
{
	Vpi_StatusCode Vpi_Guid_CopyTo(Vpi_Guid*  pSource,Vpi_Guid** ppDestination);
	Vpi_Guid* Vpi_Guid_Create(Vpi_Guid* pGuid);	
}
////////////////////////////////////////////////////////////////////////////
// 
// Support for Socket
//
////////////////////////////////////////////////////////////////////////////
//extern "C"
//{
	typedef struct _Vpi_TimeVal Vpi_TimeVal;
	#define VPI_P_SOCKET_ARRAY_SIZE 60
	typedef Vpi_Void*     Vpi_Socket;
	/* forward definition of the Vpi_Socket structure */
	typedef struct _Vpi_InternalSocket Vpi_InternalSocket;
	typedef Vpi_Void* Vpi_RawSocket; /* pointer to a socket of the platform type */

#if defined(WIN32) || defined(_WIN32_WCE)
	typedef struct _Vpi_P_Socket_Array
	{
		/*! @brief How many sockte entries are _set_. */
		Vpi_UInt uintNbSockets;
		/*! @brief An array of raw (platform) sockets. */
		Vpi_RawSocket SocketArray[VPI_P_SOCKET_ARRAY_SIZE];
	} Vpi_P_Socket_Array;
#endif

#ifdef _GNUC_
#include <sys/select.h>
	typedef struct _Vpi_P_Socket_Array
	{
		/*! @brief How many sockte entries are _set_. */
		Vpi_UInt uintNbSockets;
		/*! @brief An array of raw (platform) sockets. */
		fd_set SocketArray;
		/*! @brief FD maximum on the set */
		Vpi_UInt maxFdInSet;
	} Vpi_P_Socket_Array;
#endif


	Vpi_StatusCode VPI_DLLCALL Vpi_RawSocket_InitializeNetwork();
	Vpi_StatusCode VPI_DLLCALL Vpi_RawSocket_Close(Vpi_RawSocket a_RawSocket);
	Vpi_Int32 VPI_DLLCALL Vpi_RawSocket_Read(Vpi_RawSocket RawSocket,
		Vpi_Byte*     Buffer,
		Vpi_UInt32    BufferSize,
		int flags);
	Vpi_Int32 VPI_DLLCALL Vpi_RawSocket_Write(Vpi_RawSocket RawSocket,
		Vpi_Byte*     Buffer,
		Vpi_UInt32    BufferSize);
	Vpi_StatusCode VPI_DLLCALL Vpi_RawSocket_SetBlockMode(Vpi_RawSocket RawSocket,
		Vpi_Boolean   bBlocking);
	Vpi_StatusCode VPI_DLLCALL Vpi_RawSocket_SetTimeout(Vpi_RawSocket a_RawSocket, Vpi_Int32 nTimeOut);
	Vpi_StatusCode Vpi_RawSocket_GetPeerInfo(Vpi_RawSocket RawSocket,
		Vpi_UInt32*   IP,
		Vpi_UInt16*   Port);

	Vpi_StatusCode Vpi_RawSocket_Select(Vpi_RawSocket         RawSocket,
		Vpi_P_Socket_Array*   FdSetRead,
		Vpi_P_Socket_Array*   FdSetWrite,
		Vpi_P_Socket_Array*   FdSetException,
		Vpi_TimeVal*          Timeout);
//}
////////////////////////////////////////////////////////////////////////////
// 
// Support for Mutex
//
////////////////////////////////////////////////////////////////////////////
extern "C" 
{
	//typedef Vpi_Void*     Vpi_Mutex;
	Vpi_StatusCode VPI_DLLCALL Vpi_Mutex_Create(Vpi_Mutex* phNewMutex);
	Vpi_Void		VPI_DLLCALL	Vpi_Mutex_Delete(Vpi_Mutex* phMutex);
	Vpi_Void		VPI_DLLCALL	Vpi_Mutex_Lock(Vpi_Mutex  hMutex);
	Vpi_Void		VPI_DLLCALL	Vpi_Mutex_Unlock(Vpi_Mutex  hMutex);
	Vpi_Int32		VPI_DLLCALL	Vpi_InterlockExchange(Vpi_Int32* volatile pTarget, Vpi_Int32 value);
}
////////////////////////////////////////////////////////////////////////////
// 
// Support for Semaphore
//
////////////////////////////////////////////////////////////////////////////
extern "C"
{
	//typedef Vpi_Void*     Vpi_Semaphore;
	Vpi_StatusCode VPI_DLLCALL Vpi_Semaphore_Create(Vpi_Semaphore*    phNewSemaphore,
		Vpi_UInt32        uInitalValue,
		Vpi_UInt32        uMaxRange);
	Vpi_StatusCode VPI_DLLCALL Vpi_Semaphore_Delete(Vpi_Semaphore*    phSemaphore);
	Vpi_StatusCode VPI_DLLCALL Vpi_Semaphore_Wait(Vpi_Semaphore     hSemaphore);
	Vpi_StatusCode VPI_DLLCALL Vpi_Semaphore_TimedWait(Vpi_Semaphore     hSemaphore,
		Vpi_UInt32        msecTimeout);
	Vpi_StatusCode VPI_DLLCALL Vpi_Semaphore_Post(Vpi_Semaphore     hSemaphore,
		Vpi_UInt32        uReleaseCount);
}
////////////////////////////////////////////////////////////////////////////
// 
// Support for Thread
//
////////////////////////////////////////////////////////////////////////////
extern "C" 
{
	typedef Vpi_Void*     Vpi_Thread;
	typedef Vpi_Void(Vpi_PfnThreadMain)(Vpi_Void* pArgument);
	Vpi_StatusCode    Vpi_Thread_Create(Vpi_Thread*        pThread,
											  Vpi_PfnThreadMain* pThreadMain,
											  Vpi_Void*          pThreadArgument);
	Vpi_Void          Vpi_Thread_Delete(Vpi_Thread pThread);

	Vpi_StatusCode Vpi_Thread_Start(Vpi_Thread   Thread);

	Vpi_StatusCode Vpi_Thread_WaitForShutdown(Vpi_Thread   Thread, Vpi_UInt32    msecTimeout);

	Vpi_Void Vpi_Thread_Sleep(Vpi_UInt32    msecTimeout);

	Vpi_UInt32 Vpi_Thread_GetCurrentThreadId();

	Vpi_Boolean Vpi_Thread_IsRunning(Vpi_Thread    hThread);
}
////////////////////////////////////////////////////////////////////////////
// 
// Support for CRtSocket
//
////////////////////////////////////////////////////////////////////////////
class CRtSocketTimeout;

class VPI_IMPORT CRtSocket  
{
public:
	CRtSocket();
	~CRtSocket();
	Vpi_Boolean IsSocketOk();
	Vpi_Boolean IsSet(Vpi_P_Socket_Array* pFdSet);
	Vpi_StatusCode	CreateServer();
	Vpi_StatusCode	CreateServer(UINT nSocketPort, LPCSTR lpszSocketAddress);
	Vpi_StatusCode	CreateClient();
	Vpi_StatusCode	CreateClient(UINT nSocketPort, LPCSTR lpszSocketAddress);
	Vpi_StatusCode	AttachRawSocket(Vpi_RawSocket hSocket);
	Vpi_RawSocket			GetRawSocket();
	void					SetTcpnoDelay(Vpi_Boolean bNoDelay);
	Vpi_UInt16			GetPort();
	void					SetPort(Vpi_UInt16 dwPort);
	void					SetAddressFamily(Vpi_Int32 nAddressFamily);
	void					SetSocketType(Vpi_Int32 nSocketType);
	void					SetTimeout(Vpi_Int32 nTimeout);
	Vpi_UInt32			GetProtocolType();
	void					SetProtocolType(Vpi_UInt32 uiProtocolType);
	Vpi_String			GetIPAddress();
	void					SetIpAddress(Vpi_String szIpAddress);
	Vpi_Boolean			Listen(int nConnectionBacklog = 5);
	Vpi_StatusCode	Connect();
	Vpi_StatusCode	Connect(UINT nSocketPort, LPCSTR lpszSocketAddress, int nAddressFormat = AF_INET);
	Vpi_Boolean			Socket(int nAddressFormat = AF_INET, int nSocketType = SOCK_STREAM, int nProtocolType = 0);
	int						Receive(void* lpBuf, int nBufLen, int nFlags=0);
	int						ReceiveFrom(void* lpBuf, int* nBufLen, int nFlags = 0);
	int						ReceiveLine(char* lpBuf, int nBufLen);
	int						SendTo(const void *buf, int len, int flags);
	int						Send(const void* lpBuf, int nBufLen, int nFlags=0);
	Vpi_Boolean			Bind(UINT nSocketPort, LPCSTR lpszSocketAddress);
	Vpi_RawSocket			Accept(struct sockaddr* lpSockAddr = NULL, int* lpSockAddrLen = NULL);
	void					Close();
	Vpi_StatusCode	IOCtl( long lCommand, DWORD* lpArgument );
	//int						SetSockOpt(int nLevel, int nOptName, char *lpOpVal, int nOptLen);
	int						GetSockOpt(Vpi_RawSocket rawSocket, int level, int optname, int *optval, int *optlen);
protected:
	Vpi_InternalSocket*   m_pListenInternalSocket;
	Vpi_InternalSocket*   m_pAcceptInternalSocket;
	Vpi_RawSocket			m_hSocket;
	Vpi_UInt16			m_uiPort;
	Vpi_String			m_szIpAddress;
	int						m_nAddressFamily;
	int						m_nSocketType;
	int						m_nTimeout;
	Vpi_UInt32			m_uiProtocolType;
	CRtSocketTimeout*		m_pThreadTimeout;
};
typedef struct _THREAD_PARAM
{
	void	*pThread;
	void			*pParam;
} THREAD_PARAM;
class   CRtThread  
{
public:
	CRtThread();
	virtual ~CRtThread();
	inline	void			SetName(Vpi_String szName) { Vpi_String_CopyTo(&szName, &m_szName); }
	inline	Vpi_String	GetName(){ return m_szName; }
	inline  DWORD			GetExitThreadStatus(){return m_dwExitThreadStatus;}
	inline  DWORD			GetThreadID(){return m_dwThreadID;}
	inline  void			SetThreadID(DWORD dwThreadID){m_dwThreadID = dwThreadID;}
	inline  HANDLE			GetThreadHandle(){return m_hThread;}
	inline  void			SetAutoDelete(BOOL bAutoDelete=TRUE){m_bAutoDelete=bAutoDelete;}
	inline	BOOL			IsRunning(){return m_bRun;}
	inline	void			SetTimeOut(DWORD dwTimeOut){m_dwTimeOut = dwTimeOut;}
	inline	DWORD			GetTimeOut(){return m_dwTimeOut;}
	virtual	BOOL			Run(void *pParam = NULL);
			BOOL			SetPriority(int nPriority);
			void			SetThreadName( DWORD dwThreadID, LPCSTR szThreadName);
	virtual BOOL			Quit();
	virtual DWORD			Main(void *pParam = NULL);
protected:
	HANDLE					m_hThread;
	DWORD	    			m_dwThreadID;
	DWORD					m_dwExitThreadStatus;
	Vpi_String			m_szName;
	int						m_nPriority;
	HANDLE					m_hEventRun;
	BOOL					m_bWaitEnd;
	BOOL					m_bAutoDelete;
	BOOL					m_bRun;
	DWORD					m_dwTimeOut;
	THREAD_PARAM			m_xThreadParam;
protected:
	virtual BOOL		InitInstance();
	virtual BOOL		ExitInstance();
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

////////////////////////////////////////////////////////////////////////////
//
//			Serial device support
//
typedef enum PARITY
{
	NO_PARITY,
	ODD_PARITY,
	EVEN_PARITY
} _PARITY;
typedef enum STOP
{
	STOP10BIT, // 1 stop bit
	STOP15BIT, // 1.5 stop bit
	STOP20BIT // 2 stop bits
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
class CSerialPort
{
public:
	CSerialPort(void);
	~CSerialPort(void);
	Vpi_String*			GetPortName();        // used as file name to open port
	void					SetPortName(Vpi_String* strVal);
	Vpi_Boolean			IsEnable();
	void					SetEnable(Vpi_Boolean bVal);
	Vpi_UInt32			GetBaud();      // the value, eg 9600
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
	Vpi_Boolean			Initialize(); // Open the serial port
	Vpi_StatusCode	Close(); // Close the serial port
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
	SERIAL_TYPE				m_SerialType;// Contains the serial type : Native (100% serie), Emulate (USB via FDTI)
#ifdef WIN32
	COMMTIMEOUTS			m_cts;
#endif
};