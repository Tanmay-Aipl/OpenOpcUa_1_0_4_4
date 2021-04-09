
#include "stdafx.h"
#ifdef WIN32
#include "ServiceModule.h"
#include "ServiceMessages.h"
#include "resource.h"
//#include "atlbase.h"
using namespace OpenOpcUa;
using namespace UACoreServer;
//extern CServiceModule _Module;
CServiceModule*	CServiceModule::m_pServiceModule;
#define MAX_ERROR_STRING 1024
/*--
********************************************************************************
Nom        : ConsoleHandler
Role       : Console pour mode debug. Permet d'effectuer une sortie propre
de l'application lors de l'arret de l'application.
********************************************************************************
--*/
BOOL WINAPI ConsoleHandler(DWORD dwCtrlType)
{
	DWORD i = 0;
	switch (dwCtrlType) {

	case CTRL_BREAK_EVENT:  /*FreeConsole();*/	return true;
	case CTRL_CLOSE_EVENT:
	case CTRL_C_EVENT:
		// PostThreadMessage(_Module.m_dwThreadId, WM_QUIT, 0, 0); // Trouver une implémentation compatible
		while (i++<500) Sleep(10);
		return TRUE;
	}
	return FALSE;
}


CServiceModule::CServiceModule()
{
	m_pAppId = (OpcUa_Guid*)OpcUa_Alloc(sizeof(OpcUa_Guid));
	m_hEventSource = OpcUa_Null;
	m_bService = OpcUa_True;
	ZeroMemory(m_szServiceName, 256);
	ZeroMemory(m_szServiceDisplayName, 256);
	OpcUa_Semaphore_Create(&m_hServerReadySem, 0, 0x100);
	m_dwThreadId=0;
	m_hWndParent = OpcUa_Null;
	m_hWnd = OpcUa_Null;
	m_pServiceModule = OpcUa_Null;
}


CServiceModule::~CServiceModule()
{
	if (m_pAppId)
	{
		OpcUa_Free(m_pAppId);
		m_pAppId = OpcUa_Null;
	}
	/*if (!m_bService)
	{
		m_tnd.cbSize = sizeof(NOTIFYICONDATA);
		m_tnd.hWnd = m_hWnd;
		m_tnd.uID = IDI_TRAY;
		Shell_NotifyIcon(NIM_DELETE, &m_tnd);
	}*/
	OpcUa_Semaphore_Delete(&m_hServerReadySem);
	// Here we need to release server ressources
}

// Michel Condemine - 4CE Industry 
// Function name	: AddEventSource
// Description	    :  Le journal d'evenement a besoin de connaitre l'endroit ou trouve le texte qui correspond aux messages ID.
//								Pour ce faire il faut ajouter une clé dans le registre HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Eventlog\NOMSERVICE
// Return type		: void 
// Argument         : voud
void CServiceModule::AddEventSource()
{
	HKEY hk;
	DWORD dwData;
	TCHAR szFilePath[_MAX_PATH];
	char szSubKey[256];
	DWORD dwLen = _MAX_PATH;

	memset(szSubKey, 0, 256);
	strcpy(&szSubKey[0], "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\OpenOpcUaCoreServer");
	//USES_CONVERSION;

	// Add your source name as a subkey under the Application 
	// key in the EventLog registry key. 

	if (RegCreateKey(HKEY_LOCAL_MACHINE,
		szSubKey, &hk))
	{
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Could not create the registry key.\n");
		return;
	}

	// Set the name of the message file. 
	// Get the executable file path
	::GetModuleFileName(NULL, szFilePath, _MAX_PATH);

	// Add the name to the EventMessageFile subkey. 

	if (RegSetValueEx(hk,             // subkey handle 
		"EventMessageFile",       // value name 
		0,                        // must be zero 
		REG_EXPAND_SZ,            // value type 
		(LPBYTE)szFilePath,           // pointer to value data 
		dwLen))       // length of value data 
	{
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Could not set the event message file.\n");
		return;
	}
	// Set the supported event types in the TypesSupported subkey. 

	dwData = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE |
		EVENTLOG_INFORMATION_TYPE;

	if (RegSetValueEx(hk,      // subkey handle 
		"TypesSupported",  // value name 
		0,                 // must be zero 
		REG_DWORD,         // value type 
		(LPBYTE)&dwData,  // pointer to value data 
		sizeof(DWORD)))    // length of value data 
	{
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Could not set the supported types.\n");
		return;
	}
	RegCloseKey(hk);
}
// Michel Condemine - 4CE Industry 
// Function name	: RemoveEventSource
// Description	    :  Supprime les clé de registre relative à l'observateur d'evenement.//								
// Return type		: void 
// Argument         : voud
void CServiceModule::RemoveEventSource()
{
	HKEY hk;
	char szSubKey[256];
	memset(szSubKey, 0, 256);
	strcpy(&szSubKey[0], "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\OpenOpcUaCoreServer");
	//USES_CONVERSION;

	// first check if the key is existing

	if (RegOpenKey(HKEY_LOCAL_MACHINE,
		szSubKey, &hk))
	{
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"cannot open the registry key.\n");
		return;
	}
	RegCloseKey(hk);

	// The key is existing, we can remove
	LONG lRes = RegDeleteKey(HKEY_LOCAL_MACHINE, szSubKey);
	if (lRes != ERROR_SUCCESS)
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Cannot delete the requested key\n");
	else
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Registry key remove\n");
}

// Handler
void CServiceModule::ServiceHandler(DWORD dwOpcode)
{
	//USES_CONVERSION;
	CServiceModule *pServiceModule=CServiceModule::m_pServiceModule;
	switch (dwOpcode)
	{
	case SERVICE_CONTROL_STOP:
		pServiceModule->SetServiceStatus(SERVICE_STOP_PENDING);
		// Stop the server
		PostThreadMessage(pServiceModule->m_dwThreadId, WM_QUIT, 0, 0);
		break;
	case SERVICE_CONTROL_PAUSE:
		// Pause the server. What does it mean ?
		// This means changing the server status to paused
		// and not allowing the server to accept new session or secure channel creation
		break;
	case SERVICE_CONTROL_CONTINUE:
		// Resuming the server from a previous pause
		break;
	case SERVICE_CONTROL_INTERROGATE:
		break;
	case SERVICE_CONTROL_SHUTDOWN:
		// What is the difference with Stop ?
		break;
	default:
		pServiceModule->LogEvent(EVENTLOG_ERROR_TYPE, EVMSG_BADREQUEST, pServiceModule->m_szServiceName);
	}
}

// Init
void CServiceModule::Init(HINSTANCE h, UINT nServiceNameID)
{
	//////////////////////////////////////////////
	m_bService = TRUE;
	LoadString(h, nServiceNameID, m_szServiceName, sizeof(m_szServiceName) / sizeof(TCHAR));
	DWORD dwLastError=GetLastError();
	LoadString(h, IDS_SERVICE_DISPLAYNAME, m_szServiceDisplayName, sizeof(m_szServiceDisplayName) / sizeof(TCHAR));
	dwLastError=GetLastError();
	// set up the initial service status 
	m_hServiceStatus = NULL;
	m_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	m_status.dwCurrentState = SERVICE_STOPPED;
	m_status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	m_status.dwWin32ExitCode = 0;
	m_status.dwServiceSpecificExitCode = 0;
	m_status.dwCheckPoint = 0;
	m_status.dwWaitHint = 0;
}
// SetServiceStatus
void CServiceModule::SetServiceStatus(DWORD dwState)
{
	m_status.dwCurrentState = dwState;
	if (m_hServiceStatus)
		::SetServiceStatus(m_hServiceStatus, &m_status);

}

// This function makes an entry into the application event log
void CServiceModule::LogEvent(WORD wType, DWORD dwID,
	const char* pszS1,
	const char* pszS2,
	const char* pszS3)
{
	const char* ps[3];

	ps[0] = pszS1;
	ps[1] = pszS2;
	ps[2] = pszS3;
	LPCTSTR  lpszStrings[1];

	//DWORD size=0;
	//USES_CONVERSION;
	WORD iStr = 0;
	for (int i = 0; i < 3; i++)
	{
		if (ps[i] != NULL)
		{
			lpszStrings[0] = ps[i];
			iStr++;
		}
	}

	// Check the event source has been registered and if
	// not then register it now
	if (!m_hEventSource) {
		m_hEventSource = ::RegisterEventSource(NULL,  // local machine
			m_szServiceName); // source name m_szServiceName
	}

	if (m_hEventSource)
	{
		BOOL bErr = ::ReportEvent(m_hEventSource,
			wType,
			(WORD)0,
			dwID,
			NULL, // sid
			iStr,
			0,
			&lpszStrings[0],
			NULL);
		if (!bErr)
		{
#ifdef DEBUG
			DWORD dwError = GetLastError();
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Last error for OS=0x%0x\n", dwError);
#endif			
		}
	}
}
// Install
BOOL CServiceModule::Install()
{
	DWORD dwInfoLevel = SERVICE_CONFIG_DESCRIPTION;
	SERVICE_DESCRIPTION lServiceDesciption;
	if (IsInstalled())
		return TRUE;
	OpcUa_CharA* szServiceName= GetServiceName();
	lServiceDesciption.lpDescription = (LPSTR)malloc(strlen(szServiceName)+25);
	ZeroMemory(lServiceDesciption.lpDescription, strlen(szServiceName)+25);
	memcpy(lServiceDesciption.lpDescription, szServiceName, strlen(szServiceName)); //_T("OpenOpcUaCoreServer");
	strcat(lServiceDesciption.lpDescription, " from OpenOpcUa.org");
	
	SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hSCM == NULL)
	{
		MessageBox(NULL, _T("Couldn't open service manager"), m_szServiceName, MB_OK);
		return FALSE;
	}

	// Get the executable file path
	TCHAR szFilePath[_MAX_PATH];
	::GetModuleFileName(NULL, szFilePath, _MAX_PATH);
	// Append the AppId
	OpcUa_String* szAppId = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(szAppId);
	OpcUa_Guid_ToString(m_pAppId, &szAppId);
	strcat(szFilePath," AppId=");
	strcat(szFilePath, OpcUa_String_GetRawString(szAppId));
	SC_HANDLE hService = ::CreateService(
		hSCM, m_szServiceName, m_szServiceDisplayName,
		SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
		SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL,
		szFilePath, NULL, NULL, NULL, NULL, NULL);//_T("RPCSS\0")

	if (hService == NULL)
	{
		::CloseServiceHandle(hSCM);
		MessageBox(NULL, _T("Couldn't create service"), m_szServiceName, MB_OK);
		return FALSE;
	}
	// Ajout de la description associé à RapidService. 
	// Cette descirption sera visible dans le gestionnaire de service
	BOOL  bRes = ChangeServiceConfig2(hService, dwInfoLevel, &lServiceDesciption);
	if (!bRes)
	{
		::CloseServiceHandle(hSCM);
		MessageBox(NULL, _T("Couldn't set service desciption"), m_szServiceName, MB_OK);
		return FALSE;
	}
	::CloseServiceHandle(hService);
	::CloseServiceHandle(hSCM);
	return TRUE;
}
// IsInstalled
BOOL CServiceModule::IsInstalled()
{
	BOOL bResult = FALSE;

	SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	OpcUa_CharA	szServiceName[256];
	ZeroMemory(szServiceName, 256);
	memcpy(szServiceName, m_szServiceName, strlen(m_szServiceName));
	//strcat(szServiceName, " from OpenOpcUa.org");
	if (hSCM != NULL)
	{
		SC_HANDLE hService = ::OpenService(hSCM, szServiceName, SERVICE_QUERY_CONFIG);
		if (hService != NULL)
		{
			bResult = TRUE;
			::CloseServiceHandle(hService);
		}
		::CloseServiceHandle(hSCM);
	}
	return bResult;
}

// Uninstall
BOOL CServiceModule::Uninstall()
{
	if (!IsInstalled())
		return TRUE;

	SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (hSCM == NULL)
	{
		MessageBox(NULL, _T("Couldn't open service manager"), m_szServiceName, MB_OK);
		return FALSE;
	}
	OpcUa_CharA	szServiceName[256];
	ZeroMemory(szServiceName, 256);
	memcpy(szServiceName, m_szServiceName, strlen(m_szServiceName));
	//strcat(szServiceName, " from OpenOpcUa.org");
	SC_HANDLE hService = ::OpenService(hSCM, szServiceName, SERVICE_STOP | DELETE);

	if (hService == NULL)
	{
		::CloseServiceHandle(hSCM);
		MessageBox(NULL, _T("Couldn't open service"), m_szServiceName, MB_OK);
		return FALSE;
	}
	SERVICE_STATUS status;
	::ControlService(hService, SERVICE_CONTROL_STOP, &status);

	BOOL bDelete = ::DeleteService(hService);
	::CloseServiceHandle(hService);
	::CloseServiceHandle(hSCM);

	// Wait until the service is actually removed from the SCM database.
	// Not waiting could cause problems if the service is immediately re-registered.
	// The loop waits a maximumn of 10 seconds in case of a fatal problem.
	for (int ii = 0; IsInstalled() && ii < 100; ii++)
	{
		Sleep(100);
	}

	if (!bDelete)
	{
		DWORD dwError = GetLastError();
		OpcUa_CharA szMessage[256];
		ZeroMemory(szMessage, 256);
		sprintf(szMessage, "Service could not be deleted LastError give : %u", dwError);
		MessageBox(NULL, szMessage, m_szServiceName, MB_OK);
	}
	return bDelete;
}

// Run
void CServiceModule::Run()
{
	//USES_CONVERSION;
	if (m_pServiceModule)
	{
		m_pServiceModule->m_dwThreadId = GetCurrentThreadId();
		// The addressSpace was built on the server startup ?
		// Do we need a different behavior ?
		/*hr = BuildOPCAddressSpace();
		if (hr != S_OK)
		{
		char message[512];
		memset(message, 0, 512);
		sprintf(message, "Cannot build addressSpace... please restart the service hr=0x%05x LastError=0x%05x", hr, GetLastError());
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, message);
		if (hr == S_FALSE)
		LogEvent(EVENTLOG_INFORMATION_TYPE, EVMSG_OPCCFG_ERR_LVL01);
		if (hr == E_FAIL)
		LogEvent(EVENTLOG_INFORMATION_TYPE, EVMSG_OPCCFG_ERR_LVL02);
		if (hr == E_INVALIDARG)
		LogEvent(EVENTLOG_INFORMATION_TYPE, EVMSG_OPCCFG_ERR_LVL03);
		LogEvent(EVENTLOG_INFORMATION_TYPE, EVMSG_OPCCFG_FAILED);
		}*/
		// Do something like create server address space or start the server itself
		// 

		// Maybe we can tell other thread that the server is ready to use
		OpcUa_Semaphore_Post(m_hServerReadySem, 1);
		// fermeture du handle et ré-initialisation
		OpcUa_Semaphore_Delete(&m_hServerReadySem);
		m_hServerReadySem = OpcUa_Null;

		//LogEvent(_T("Service started"));
		LogEvent(EVENTLOG_INFORMATION_TYPE, EVMSG_SUCCEED);
		SetServiceStatus(SERVICE_RUNNING);
		/////////////////////////////////////////////////
		/*
		hr = CreateInvisibleWnd();
		m_hWnd = GetWindow();
		{
		// Icon status SysTray

		m_tnd.hIcon = LoadIcon(IDI_TRAY);//IDI_ICONDA_DISCONNECT
		m_tnd.cbSize = sizeof(NOTIFYICONDATA);
		m_tnd.hWnd = m_hWnd;
		m_tnd.uID = IDI_TRAY;
		m_tnd.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
		m_tnd.uCallbackMessage = WM_TRAYNOTIFY;
		}
		lstrcpyn(m_tnd.szTip, _T("4CE Industry RapidService"), sizeof(m_tnd.szTip));
		if (!m_bService)
		{
		BOOL rb = Shell_NotifyIcon(NIM_ADD, &m_tnd);
		if (!rb)
		Beep(1500, 200);
		}
		*/
		/////////////////////////////////////////////////
		/////////////////////////////////////////////////
		MSG msg;
		while (GetMessage(&msg, 0, 0, 0))
			DispatchMessage(&msg);
		/*
		if (!m_bService)
		{
		m_tnd.cbSize = sizeof(NOTIFYICONDATA);
		m_tnd.hWnd = m_hWnd;
		m_tnd.uID = IDI_TRAY;
		Shell_NotifyIcon(NIM_DELETE, &m_tnd);
		}*/
	}
}

// Start
void CServiceModule::Start()
{
	SERVICE_TABLE_ENTRY st[] =
	{
		{ m_szServiceName, _ServiceMain },
		{ NULL, NULL }
	};
	
	//HANDLE hProcess=GetCurrentProcess();
	//SetPriorityClass(hProcess,REALTIME_PRIORITY_CLASS);
	//USES_CONVERSION;
	BOOL bSrvDisp = ::StartServiceCtrlDispatcher(st);
	if (m_bService && !bSrvDisp)
	{
		DWORD dwError = GetLastError();
		TCHAR tsMsg[MAX_ERROR_STRING + 1];
		LCID lcID = GetSystemDefaultLCID();
		memset(tsMsg, 0, sizeof(tsMsg));
		/////////////////////////////////////////////////////

		FormatMessage(
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dwError,
			LANGIDFROMLCID(lcID),
			tsMsg,
			MAX_ERROR_STRING,
			NULL);
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"%ls", tsMsg);
		LogEvent(EVENTLOG_ERROR_TYPE, EVMSG_FAILED, m_szServiceName);
		m_bService = FALSE;
	}

	if (m_bService == FALSE)
	{
		SetConsoleCtrlHandler(ConsoleHandler, TRUE);
		Run();
	}
	/*
	if (!m_bService)
	{
		m_tnd.cbSize = sizeof(NOTIFYICONDATA);
		m_tnd.hWnd = m_hWnd;
		m_tnd.uID = IDI_TRAY;
		Shell_NotifyIcon(NIM_DELETE, &m_tnd);
	}*/
}
// _ServiceMain
void WINAPI CServiceModule::_ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
	(void*)dwArgc; 
	(void*)lpszArgv;
	ServiceMain(m_pServiceModule);
}
// ServiceMainvoid CServiceModule::ServiceMain(DWORD /* dwArgc */, LPTSTR* /* lpszArgv */)
void CServiceModule::ServiceMain(LPVOID arg)
{
	//USES_CONVERSION;
	// Register the control request handler
	CServiceModule* pServiceModule = (CServiceModule*)arg;
	if (pServiceModule)
	{
		pServiceModule->m_status.dwCurrentState = SERVICE_START_PENDING;
		pServiceModule->m_hServiceStatus = RegisterServiceCtrlHandler(pServiceModule->m_szServiceName, (LPHANDLER_FUNCTION)CServiceModule::ServiceHandler);
		if (pServiceModule->m_hServiceStatus == NULL)
		{
			//LogEvent(_T("Handler not installed"));
			pServiceModule->LogEvent(EVENTLOG_ERROR_TYPE, EVMSG_CTRLHANDLERNOTINSTALLED, pServiceModule->m_szServiceName);
			return;
		}
		pServiceModule->SetServiceStatus(SERVICE_START_PENDING);

		pServiceModule->m_status.dwWin32ExitCode = S_OK;
		pServiceModule->m_status.dwCheckPoint = 0;
		pServiceModule->m_status.dwWaitHint = 0;

		// When the Run function returns, the service has stopped.
		pServiceModule->Run();

		pServiceModule->SetServiceStatus(SERVICE_STOPPED);
		//LogEvent(_T("Service stopped"));
		pServiceModule->LogEvent(EVENTLOG_INFORMATION_TYPE, EVMSG_STOPPED);
	}
}
void CServiceModule::Serviced(OpcUa_Boolean bVal)
{
	m_bService = bVal;
}
// InstallService
HRESULT CServiceModule::InstallService(char* path, char* fileName)
{
	HRESULT hr = S_OK;
	HKEY hAppIdKey;
	char szSubKey[256];
	memset(szSubKey, 0, 256);
	strcpy(&szSubKey[0], "AppID\\");
	OpcUa_CharA* szAppId = (OpcUa_CharA*)OpcUa_Alloc(1024);
	ZeroMemory(szAppId, 1024);
	OpcUa_Guid* pAppId=GetApplicationId();
	OpcUa_Guid_ToStringA(pAppId, szAppId);
	strcat(&szSubKey[0], szAppId);

	// first check if the key is existing
	hr = RegOpenKeyEx(HKEY_CLASSES_ROOT, szSubKey, 0, KEY_ALL_ACCESS, &hAppIdKey);
	if (hr==ERROR_SUCCESS)
	{
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "OpenOpcUaCoreServer is already registred as a Windows Service\n");
		hr = E_INVALIDARG;
	}
	else
	{
		// Let's register it
		DWORD dwDisposition;
		if (RegCreateKeyEx(HKEY_CLASSES_ROOT, szSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hAppIdKey, &dwDisposition) != ERROR_SUCCESS)
		{
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Could not create the registry key.hr=0x%05x\n", hr);
		}
		else
		{
			// Now add he other entry in the registry for the server configuration
			// Add LocalService with Value of the ServiceName
			OpcUa_CharA* szServiceName=GetServiceName();
			hr=RegSetValueEx(hAppIdKey, "LocalService", 0, REG_SZ, (BYTE*)szServiceName, (DWORD)strlen(szServiceName));
			if (hr == ERROR_SUCCESS)
			{
				// Add ServiceParameters with Value of -Service
				hr=RegSetValueEx(hAppIdKey, "ServiceParameters", 0, REG_SZ, (BYTE*)"-Service", (DWORD)8);
				if (hr == ERROR_SUCCESS)
				{
					// Add ConfigPathName with value in path
					hr = RegSetValueEx(hAppIdKey, "ConfigPathName", 0, REG_SZ, (BYTE*)path, (DWORD)strlen(path));
					if (hr == ERROR_SUCCESS)
					{
						// Add ConfigFileName with value in path
						hr = RegSetValueEx(hAppIdKey, "ConfigFileName", 0, REG_SZ, (BYTE*)fileName, (DWORD)strlen(fileName));
						if (hr != ERROR_SUCCESS)
							OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Could not create the registry key value ConfigFileName. hr=0x%05x\n", hr);
						else
						{
							if (!Install())
								hr = E_FAIL;
						}
					}
					else
					{
						OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Could not create the registry key value ConfigPathName. hr=0x%05x\n", hr);
					}
				}
				else
				{
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Could not create the registry key value ServiceParameters. hr=0x%05x\n", hr);
				}
				RegCloseKey(hAppIdKey);
 			}
			else
			{
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Could not create the registry key value LocalService. hr=0x%05x\n",hr);
			}
		}
	}

	// Adjust the AppID for Local Server or Service
	/*
	ATL::CRegKey keyAppID;
	hr = keyAppID.Open(HKEY_CLASSES_ROOT, _T("AppID"));
	if (hr == ERROR_SUCCESS)
	{
		OpcUa_String* pszAppId = OpcUa_Null;
		OpcUa_Guid_ToString(m_pAppId, &pszAppId);
		HKEY hKeyAppId;
		ATL::CRegKey key;
		hr = key.Open(keyAppID, _T("{CB85A975-BAD5-4A1E-A142-FF98330EB221}")); // GUID of the OpenOpcUaCoreServer when it runs as a WindowsService
		if (hr == ERROR_SUCCESS)
		{			
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "OpenOpcUACoreServer is already registred as a Windows Service");
			hr = E_INVALIDARG;
		}
		else
		{
			hr = key.Create(keyAppID, _T("{CB85A975-BAD5-4A1E-A142-FF98330EB221}"));
			if (hr == ERROR_SUCCESS)
			{
				{
					if (m_bService)
					{
						hr = key.SetStringValue(_T("LocalService"), m_szServiceName);// only for C++.Net (ATL/MFC7)
						if (hr != ERROR_SUCCESS)
							OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "SetStringValue failed:LocalService %s", m_szServiceName);
						else
						{
							hr = key.SetStringValue(_T("ServiceParameters"), _T("-Service"));// only for C++.Net (ATL/MFC7)
							if (hr != ERROR_SUCCESS)
								OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "SetStringValue failed:ServiceParameters -Service");
							// Here in the name for the serverConfig.xsd compliant file used by the server
							hr = key.SetStringValue(_T("ConfigPathName"), path);
							if (hr != ERROR_SUCCESS)
								OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "SetStringValue failed:ServiceParameters OpenOpcUaCoreServerConfig.cml");
							hr = key.SetStringValue(_T("ConfigFileName"), fileName);
							if (hr != ERROR_SUCCESS)
								OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "SetStringValue failed:ServiceParameters OpenOpcUaCoreServerConfig.cml");
							// Create service
							if (!Install())
								hr = E_FAIL;
						}
					}
				}
			}
			else
			{
				if (hr==ERROR_ACCESS_DENIED)
					MessageBox(NULL, _T("Couldn't Access Registry"), m_szServiceName, MB_OK);
				else
					MessageBox(NULL, _T("General error with Registry"), m_szServiceName, MB_OK);
			}
			key.Close();
		}
		keyAppID.Close();
		if (pszAppId)
			OpcUa_String_Clear(pszAppId);
		
	}*/
	return hr;
}
//installService
HRESULT CServiceModule::UninstallService()
{
	HRESULT hr = S_OK;
	HKEY hk;
	char szSubKey[256];
	memset(szSubKey, 0, 256);
	strcpy(&szSubKey[0], "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\OpenOpcUaCoreServer");
	// Remove service
	if (!Uninstall())
		hr = S_FALSE;
	else
	{
		// first check if the key is existing
		hr = RegOpenKey(HKEY_LOCAL_MACHINE, szSubKey, &hk);
		if (hr!=S_OK)
		{
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "cannot open the registry key.\n");
		}
		else
		{
			RegCloseKey(hk);

			// The key is existing, we can remove
			LONG lRes = RegDeleteKey(HKEY_LOCAL_MACHINE, szSubKey);
			if (lRes != ERROR_SUCCESS)
			{
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Cannot delete the requested key\n");
				hr = E_FAIL;
			}
			else
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Registry key remove\n");
		}
	}
	HKEY hAppIdKey;
	//char szSubKey[256];
	memset(szSubKey, 0, 256);
	strcpy(&szSubKey[0], "AppID\\");
	OpcUa_CharA* szAppId = (OpcUa_CharA*)OpcUa_Alloc(1024);
	ZeroMemory(szAppId, 1024);
	OpcUa_Guid* pAppId = GetApplicationId();
	OpcUa_Guid_ToStringA(pAppId, szAppId);
	strcat(&szSubKey[0], szAppId);

	// first check if the key is existing
	hr = RegOpenKeyEx(HKEY_CLASSES_ROOT, szSubKey, 0, KEY_ALL_ACCESS, &hAppIdKey);
	if (hr == ERROR_SUCCESS)
	{
		RegCloseKey(hAppIdKey);
		hr = RegDeleteKeyEx(HKEY_CLASSES_ROOT, szSubKey, KEY_WOW64_32KEY, 0);
		if (hr == ERROR_SUCCESS)
		{

		}
	}
	//
	
	// Delete the registry key for the server configuration in AppId
	/*
	ATL::CRegKey keyAppID;
	hr = keyAppID.Open(HKEY_CLASSES_ROOT, _T("AppID"));
	if (hr == ERROR_SUCCESS)
	{
		ATL::CRegKey key;
		hr = key.Open(keyAppID, _T("{CB85A975-BAD5-4A1E-A142-FF98330EB221}")); // GUID of the OpenOpcUaCoreServer when it runs as a WindowsService
		if (hr != ERROR_SUCCESS)
		{
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "OpenOpcUACoreServer is not registred as a Windows Service. 0x%05x",hr);
		}
		else
		{
			key.Close();
			hr =keyAppID.DeleteSubKey(_T("{CB85A975-BAD5-4A1E-A142-FF98330EB221}"));
			if (hr == ERROR_SUCCESS)
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "AppID\\CB85A975-BAD5-4A1E-A142-FF98330EB221 was properly removed from the registry");
			else
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Errro>AppID\\CB85A975-BAD5-4A1E-A142-FF98330EB221 cannot be removed from the registry. 0x%05x", hr);
		}
	}
	*/
	return hr;
}
SERVICE_STATUS CServiceModule::GetServiceStatus()
{
	return m_status;
}
OpcUa_Boolean CServiceModule::IsService()
{
	return m_bService;
}
OpcUa_StatusCode CServiceModule::ExtractConfigurationParams(char** pPath, char** pFileName)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	if (pPath && pFileName)
	{
		OpcUa_Guid* pAppId= GetApplicationId();
		if (pAppId)
		{
			HRESULT hr = S_OK;
			HKEY hAppIdKey;
			char szSubKey[256];
			memset(szSubKey, 0, 256);
			strcpy(&szSubKey[0], "AppID\\");
			OpcUa_CharA* szAppId = (OpcUa_CharA*)OpcUa_Alloc(1024);
			ZeroMemory(szAppId, 1024);
			OpcUa_Guid_ToStringA(pAppId, szAppId);
			strcat(&szSubKey[0], szAppId);

			// first check if the key is existing
			hr = RegOpenKeyEx(HKEY_CLASSES_ROOT, szSubKey, 0, KEY_ALL_ACCESS, &hAppIdKey);
			if (hr == ERROR_SUCCESS)
			{
				DWORD lenData = 512;
				OpcUa_CharA* szConfigFileName = (OpcUa_CharA*)OpcUa_Alloc(512 * sizeof(OpcUa_CharW));
				ZeroMemory(szConfigFileName, 512);
				hr = RegQueryValueEx(hAppIdKey, "ConfigFileName", NULL, NULL, (LPBYTE)szConfigFileName, &lenData);
				if (hr == ERROR_SUCCESS)
				{
					(*pFileName) = (char*)OpcUa_Alloc(strlen(szConfigFileName)+1);
					ZeroMemory((*pFileName), strlen(szConfigFileName) + 1);
					OpcUa_MemCpy((*pFileName), lenData, szConfigFileName, lenData);
					// Path
					lenData = 512;
					OpcUa_CharA* szPath = (OpcUa_CharA*)OpcUa_Alloc(512 * sizeof(OpcUa_CharW));
					ZeroMemory(szPath, 512);
					hr = RegQueryValueEx(hAppIdKey, "ConfigPathName", NULL, NULL, (LPBYTE)szPath, &lenData);
					if (hr == ERROR_SUCCESS)
					{
						(*pPath) = (char*)OpcUa_Alloc(strlen(szPath)+1);
						ZeroMemory((*pPath), strlen(szPath) + 1);
						OpcUa_MemCpy((*pPath), lenData, szPath, lenData);
					}
				}
			}
		}
		/*
		ATL::CRegKey keyAppID;
		uStatus = keyAppID.Open(HKEY_CLASSES_ROOT, _T("AppID"));
		if (uStatus == ERROR_SUCCESS)
		{
			ATL::CRegKey key;
			uStatus = key.Open(keyAppID, _T("{CB85A975-BAD5-4A1E-A142-FF98330EB221}")); // GUID of the OpenOpcUaCoreServer when it runs as a WindowsService
			if (uStatus == ERROR_SUCCESS)
			{
				OpcUa_CharW* szConfigFileName = (OpcUa_CharW*)OpcUa_Alloc(512 * sizeof(OpcUa_CharW));
				ZeroMemory(szConfigFileName, 512);
				OpcUa_CharW* szPath = (OpcUa_CharW*)OpcUa_Alloc(512 * sizeof(OpcUa_CharW));
				ZeroMemory(szPath, 512);
				ULONG ulChars = 512 * sizeof(OpcUa_CharW);
				// Extract the ConfigFileName
				uStatus = key.QueryStringValue(_T("ConfigFileName"), (LPTSTR)szConfigFileName, &ulChars);
				if (uStatus != ERROR_SUCCESS)
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critical error>Cannot retrieve configuration filename from the registry");
				else
				{
					(*pFileName)=(char*)OpcUa_Alloc(ulChars);
					OpcUa_MemCpy(*pFileName, ulChars, szConfigFileName, ulChars);
				}
				// Extract the path
				ulChars = 512 * sizeof(OpcUa_CharW);
				uStatus = key.QueryStringValue(_T("ConfigPathName"), (LPTSTR)szPath, &ulChars);
				if (uStatus != ERROR_SUCCESS)
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critical error>Cannot retrieve configuration filename from the registry");
				else
				{
					(*pPath) = (char*)OpcUa_Alloc(ulChars);
					OpcUa_MemCpy(*pPath, ulChars, szPath, ulChars);
				}
			}
		}
		*/
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}
void CServiceModule::SetInternalServiceModule(CServiceModule* pServiceModule)
{
	m_pServiceModule = pServiceModule;
}
///-------------------------------------------------------------------------------------------------
/// <summary>	Sets application unique identifier. </summary>
///
/// <remarks>	Michel, 07/07/2016. </remarks>
///
/// <param name="pAppId">	[in,out] If non-null, identifier for the application. </param>
///-------------------------------------------------------------------------------------------------

void OpenOpcUa::UACoreServer::CServiceModule::SetApplicationId(OpcUa_Guid* pAppId)
{
	if (!m_pAppId)
	{
		m_pAppId = (OpcUa_Guid*)OpcUa_Alloc(sizeof(OpcUa_Guid));
		OpcUa_Guid_Initialize(m_pAppId);
	}
	OpcUa_Guid_CopyTo(pAppId, m_pAppId);
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Gets application identifier. </summary>
///
/// <remarks>	Michel, 07/07/2016. </remarks>
///
/// <returns>	null if it fails, else the application identifier. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_Guid* OpenOpcUa::UACoreServer::CServiceModule::GetApplicationId(void)
{
	return m_pAppId;
}
void CServiceModule::SetServiceName(OpcUa_CharA* szServiceName)
{
	if (szServiceName)
	{
		ZeroMemory(m_szServiceName, 256);
		memcpy(m_szServiceName, szServiceName, strlen(szServiceName));
	}
}
OpcUa_CharA* CServiceModule::GetServiceName()
{
	return m_szServiceName;
}
#endif