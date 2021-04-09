#pragma once
#ifdef WIN32
namespace OpenOpcUa
{
	namespace UACoreServer
	{
		class CServiceModule /*:
			public CComModule*/
		{
		public:
			CServiceModule();
			~CServiceModule();
			void Start();
			static void ServiceMain(LPVOID arg);
			static void ServiceHandler(DWORD dwOpcode);
			void Run();
			BOOL IsInstalled();
			BOOL Install();
			HRESULT InstallService(char* path, char* fileName); // Install the service and create registryEntry user by the server to find its configuration
			HRESULT UninstallService();
			OpcUa_Boolean IsService();
			void AddEventSource(); // enregistre le service comme source de message pour l'observateur d'event
			void RemoveEventSource(); // supprime la clé du registre
			BOOL Uninstall();
			void Serviced(OpcUa_Boolean bVal);
			void Init(HINSTANCE h, UINT nServiceNameID);
			SERVICE_STATUS GetServiceStatus();
			OpcUa_StatusCode ExtractConfigurationParams(char** pPath, char** pFileName);
			void SetInternalServiceModule(CServiceModule* pServiceModule);
			void SetApplicationId(OpcUa_Guid* pAppId);
			OpcUa_Guid* GetApplicationId();
			void SetServiceName(OpcUa_CharA* szServiceName);
			OpcUa_CharA* GetServiceName();
		private:
			static void WINAPI _ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv);
			void SetServiceStatus(DWORD dwState);
			void LogEvent(WORD wType, DWORD dwID,
				const char* pszS1 = NULL,
				const char* pszS2 = NULL,
				const char* pszS3 = NULL);
			// Variable
		private:
			OpcUa_Boolean			m_bService;
			SERVICE_STATUS_HANDLE	m_hServiceStatus;
			SERVICE_STATUS			m_status;
			OpcUa_CharA				m_szServiceName[256];
			OpcUa_CharA				m_szServiceDisplayName[256];
			OpcUa_Handle			m_hEventSource;
			OpcUa_Semaphore			m_hServerReadySem;
			DWORD					m_dwThreadId;
			HWND					m_hWnd;
			HWND					m_hWndParent;
			static CServiceModule*	m_pServiceModule;
			OpcUa_Guid*				m_pAppId; // Application Guid used by the server instance when it run on Windows as a service
		};
	}
}
#endif