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
	namespace UACoreServer
	{
		// class permettant d'instancier les binding supportés par ce serveur
		class CUABinding
		{
		public:
			CUABinding();
			~CUABinding();
			OpcUa_String GetProtocol();
			void SetProtocol(OpcUa_String szVal);
			OpcUa_String GetPort();
			void SetPort(OpcUa_String szVal);
			OpcUa_Endpoint_SerializerType GetEncoding();
			void SetEncoding(OpcUa_Endpoint_SerializerType eVal);
			OpcUa_String AsString();
			OpcUa_String GetTransportProfileUri();
		protected:
			void UpdateBinding();
		protected:
			OpcUa_String								m_Protocol; // protocol supporté par ce serveur HTTP et/ou TCP et/ou HTTPS
			OpcUa_String								m_Port; // port d'écoute
			OpcUa_Endpoint_SerializerType				m_Encoding; // Encodage XML ou Binary
			OpcUa_String								m_szBinding; // contains the fullstring ie: opc.tcp://localhost:16664/4CEUAServer
			OpcUa_String								m_TransportProfileUri; //TransportProfileUri associate with the binding
			// Binding can be one of the following :
			//OpcUa_TransportProfile_UaTcp          "http://opcfoundation.org/UA-Profile/Transport/uatcp-uasc-uabinary"
			//OpcUa_TransportProfile_SoapHttpBinary "http://opcfoundation.org/UA-Profile/Transport/http-uasc-uabinary"
			//OpcUa_TransportProfile_SoapHttpsUaXml "http://opcfoundation.org/UA-Profile/Transport/soaphttps-uaxml"
			//OpcUa_TransportProfile_HttpsBinary    "http://opcfoundation.org/UA-Profile/Transport/https-uabinary"
		};
		typedef std::vector<CUABinding*> CUABindings;
	}
}