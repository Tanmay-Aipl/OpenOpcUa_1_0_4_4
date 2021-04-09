/*****************************************************************************
	  Author
		©. Michel Condemine, 4CE Industry (2010-2012)
	  
	  Contributors


	This software is a computer program whose purpose is to 
			implement behavior describe in the OPC UA specification.
		see wwww.opcfoundation.org for more details about OPC.
	This software is governed by the CeCILL-C license under French law and
	abiding by the rules of distribution of free software.  You can  use, 
	modify and/ or redistribute the software under the terms of the CeCILL-C
	license as circulated by CEA, CNRS and INRIA at the following URL
	"http://www.cecill.info". 

	As a counterpart to the access to the source code and  rights to copy,
	modify and redistribute granted by the license, users are provided only
	with a limited warranty  and the software's author,  the holder of the
	economic rights,  and the successive licensors  have only  limited
	liability. 

	In this respect, the user's attention is drawn to the risks associated
	with loading,  using,  modifying and/or developing or reproducing the
	software by the user in light of its specific status of free software,
	that may mean  that it is complicated to manipulate,  and  that  also
	therefore means  that it is reserved for developers  and  experienced
	professionals having in-depth computer knowledge. Users are therefore
	encouraged to load and test the software's suitability as regards their
	requirements in conditions enabling the security of their systems and/or 
	data to be ensured and,  more generally, to use and operate it in the 
	same conditions as regards security. 

	The fact that you are presently reading this means that you have had
		knowledge of the CeCILL-C license and that you accept its terms.

*****************************************************************************/

#pragma once

//#include "Application.h"
//#include "ServerDescription.h"
//using namespace OpenOpcUa;
//using namespace UASharedLib;
namespace OpenOpcUa
{
	namespace UACoreClient 
	{
		// Manages an instance of a UA client application.
		class CClientApplication : public OpenOpcUa::UASharedLib::CApplication
		{
			public:
				
				// Creates an empty description.
				CClientApplication(void);
				// Releases all resources used by the object.
				~CClientApplication(void);
				// Start the parsing of XML file that contains the client/server configuration
				OpcUa_StatusCode LoadUaClientConfiguration(char* path, char* fileName);
				OpcUa_StatusCode SaveUaClientConfiguration(char* path, char* fileName);
				// recherche une session dans ce CClientApplication à partir de son Handle
				OpcUa_StatusCode GetSession(OpcUa_Handle hSession, CSessionClient** pSession);
				OpcUa_StatusCode GetSubscription(OpcUa_Handle hSubscription, CSubscriptionClient** pSubscription);
				OpcUa_StatusCode SetApplicationDescription(OpcUa_ApplicationDescription* pAppDescription);
				// Finds the servers that are available on the specified host.
				OpcUa_StatusCode FindServers(std::string hostName, OpenOpcUa::UASharedLib::CApplicationDescriptionList** pServerDescriptionList);
				// Returns the CApplication Description, Name, Type, etc..
				OpenOpcUa::UASharedLib::CApplicationDescription* GetApplicationDescription()
				{
					return m_pApplicationDescription;
				}	
				// Sessions handling
				void AddSessionClient(CSessionClient* pSession);
				OpcUa_StatusCode RemoveSessionClient(CSessionClient* pSession);
				void RemoveAllSessionClient();
				// Fonction pour la gestion des messages de trace
				static void /*__stdcall*/ InternalMessageThread(LPVOID arg); 
				// ecriture du message dans le fichier de sortie
				OpcUa_StatusCode AddLoggerMessage(OpcUa_String* Message,DWORD Level);
				// sauvegarde du message dans la liste des messages
				void AddLoggerMessage(CLoggerMessage* pLoggerMessage) 
				{
					m_LoggerMessageList.push_back(pLoggerMessage);
				}
				OpcUa_StatusCode	StopInternalMessageThread();
				OpcUa_StatusCode	StartInternalMessageThread();
				OpcUa_String*		GetLogFileName() {return m_LogFileName;}
				void				SetLogFileName(OpcUa_String* aLogFileName);
				OpcUa_String*		GetLogPathName() {return m_LogPathName;}
				void				SetLogPathName(OpcUa_String* aLogPathName);
				void				DeleteAllSessions();
				OpcUa_Mutex			GetSessionsMutex() { return m_hSessionsMutex; }
				// ConfigFile
				OpcUa_String*		GetConfigFileName() { return m_ConfigFileName; }
				void				SetConfigFileName(OpcUa_String* aConfigFileName);
				OpcUa_String*		GetConfigPathName() { return m_ConfigPathName; }
				void				SetConfigPathName(OpcUa_String* aConfigPathName);
				OpcUa_StatusCode	GetSessions(OpcUa_UInt32* uiNoOfSessions,OpcUa_Handle** hSessions);
				OpcUa_StatusCode	GetSubscriptionOfMonitoredItem(OpcUa_Handle hMonitoredItem, CSubscriptionClient** pSubscriptionClient);
			private:
				OpenOpcUa::UASharedLib::CApplicationDescription*	m_pApplicationDescription;
				OpcUa_Mutex											m_hSessionsMutex; // Mutex de protection de la list de session m_pSessionClientList
				CSessionClientList*									m_pSessionClientList;
				// Variables pour la gestion des messages de trace
				OpcUa_Thread										m_hInternalMessageThread;
				OpcUa_Semaphore										m_InternalMessageSem;
				OpcUa_Mutex											m_hMessageLoggerMutex;
				OpcUa_Boolean										m_bRunInternalMessageThread;
				std::vector<OpenOpcUa::CLoggerMessage*>				m_LoggerMessageList; // liste des messages de trace
				OpcUa_String*										m_LogFileName;
				OpcUa_String*										m_LogPathName;
				OpcUa_String*										m_ConfigFileName;
				OpcUa_String*										m_ConfigPathName;
			public:
				static OpcUa_UInt32									m_uiRequestHandle;
		};
		typedef std::vector<CClientApplication*> CClientApplicationList;
	} // namespace UACoreClient
} // namespace OpenOpcUa
