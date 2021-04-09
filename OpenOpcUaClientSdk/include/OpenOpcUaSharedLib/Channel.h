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
#include "opcua_encoder.h"
#include "opcua_decoder.h"
#include "opcua_channel_internal.h"
namespace OpenOpcUa
{
	namespace UASharedLib 
	{

		//class CApplication;

		// Manages a communication channel with a server.
		class SHAREDLIB_EXPORT CChannel
		{
		public:


			// default constructor
			CChannel();
			// Constructs a channel that can be used by the specified application.
			CChannel(CApplication* pApplication);

			// Releases all resources used by the channel.
			~CChannel(void);

			// Connects to the endpoint with no security.
			OpcUa_StatusCode Connect(const OpcUa_String& endpointUrl, OpcUa_MessageSecurityMode eSecurityMode ,OpcUa_String szSecurityPolicy);

			// Connects to the endpoint with the security specified in the EndpointDescription.
			OpcUa_StatusCode Connect(CEndpointDescription* pEndpoint);
		
			// Disconnects from the server.
			void Disconnect(void);
			// EndpointUrl
			//OpcUa_String* GetEndpointUrl();
			//void	SetEndpointUrl(OpcUa_String* pString);
			// Returns the stack assigned handled for the channel.
			OpcUa_Channel GetInternalHandle();
		private:

			// Connects to the server.
			OpcUa_StatusCode InternalConnect(const OpcUa_String& endpointUrl, OpcUa_MessageSecurityMode eSecurityMode, OpcUa_String szSecurityPolicy);
			OpcUa_Mutex						m_ChannelMutex;
			CApplication*					m_pApplication;
			//OpcUa_String*					m_endpointUrl;
			OpcUa_Channel					m_hChannel;
			//OpcUa_String*					m_sSecurityPolicy;
			//OpcUa_MessageSecurityMode		m_eSecurityMode;
			OpcUa_ByteString				m_tServerCertificate;
			//OpcUa_InternalChannel*			m_pInternalChannel;
			//OpcUa_Channel_SecurityToken*	m_pSecurityToken; // new version for the stack 1.0.2.3 based on the 334.5 from the OPC Foundation. It contains HTTPS and SSL encoding support

		};

	} // namespace UAQuickClient

}