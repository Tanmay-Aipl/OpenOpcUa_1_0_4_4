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
namespace OpenOpcUa
{
	namespace UASharedLib 
	{
		// classe wrapper for OpcUa_SessionSecurityDiagnosticsDataType voir detail part 5 Table 135
		class SHAREDLIB_EXPORT CSessionSecurityDiagnosticsDataType
		{
		public:
			CSessionSecurityDiagnosticsDataType(void);
			CSessionSecurityDiagnosticsDataType(OpcUa_SessionSecurityDiagnosticsDataType* pSessionSecurityDiagnosticsDataType);
			//CSessionSecurityDiagnosticsDataType(OpcUa_NodeId SessionId, OpcUa_String ClientUserIdOfSession,
			//									OpcUa_Int32 NoOfClientUserIdHistory, OpcUa_String* pClientUserIdHistory,
			//									OpcUa_String AuthenticationMechanism, OpcUa_String Encoding,
			//									OpcUa_String TransportProtocol, OpcUa_MessageSecurityMode SecurityMode,
			//									OpcUa_String SecurityPolicyUri,OpcUa_ByteString ClientCertificate);
			~CSessionSecurityDiagnosticsDataType(void);
			//void UpdateInternalSessionSecurityDiagnosticsDataType();
			//void UpdateVariables();
			OpcUa_NodeId GetSessionId();
			void SetSessionId(OpcUa_NodeId* aNodeId);
			OpcUa_ByteString GetClientCertificate();
			void SetClientCertificate(OpcUa_ByteString* ClientCertificate);
			OpcUa_MessageSecurityMode GetSecurityMode();
			void SetSecurityMode(OpcUa_MessageSecurityMode Mode);
			OpcUa_String GetSecurityPolicyUri();
			void SetSecurityPolicyUri(OpcUa_String* strVal);
			OpcUa_SessionSecurityDiagnosticsDataType* GetInternalPtr();
			void SetTransportProtocol(OpcUa_String* pszTransportProtocol);
			OpcUa_String GetTransportProtocol();
			void SetEncoding(OpcUa_String* pszEncoding);
			OpcUa_String GetEncoding();
			void SetAuthenticationMechanism(OpcUa_String* pszAuthenticationMechanism);
			OpcUa_String GetAuthenticationMechanism();
		private:
			/*
			OpcUa_UInt32								m_InstanceSize; // size of the current instance of this class
			OpcUa_NodeId								m_SessionId; // Server-assigned identifier of the session.
			OpcUa_String								m_ClientUserIdOfSession; // Name of authenticated user when creating the session
			OpcUa_Int32									m_NoOfClientUserIdHistory;
			OpcUa_String*								m_pClientUserIdHistory;
			OpcUa_String								m_AuthenticationMechanism;// Type of authentication (user name and password, X.509,Kerberos).
			OpcUa_String								m_Encoding; //Which encoding is used on the wire, e.g. XML or UA Binary.
			OpcUa_String								m_TransportProtocol; // Which transport protocol is used, e.g. TCP or HTTP.
			OpcUa_MessageSecurityMode					m_SecurityMode; // The message security mode used for the session.
			OpcUa_String								m_SecurityPolicyUri; // The name of the security policy used for the session.
			OpcUa_ByteString							m_ClientCertificate; // The application instance certificate provided by the client in the CreateSession request.  
			*/
		protected:
			OpcUa_SessionSecurityDiagnosticsDataType*	m_pInternalSessionSecurityDiagnosticsDataType;
		};
		typedef std::vector<CSessionSecurityDiagnosticsDataType*> CSessionSecurityDiagnosticsDataTypeList;
	}
}