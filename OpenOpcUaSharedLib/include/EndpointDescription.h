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
		// Stores the information required to connect to a UA server.
		class SHAREDLIB_EXPORT CEndpointDescription
		{
		public:

			// Creates an empty description.
			CEndpointDescription(void);

			// Initializes the object from a OpcUa_CEndpointDescription.
			CEndpointDescription(OpcUa_EndpointDescription* pDescription);
			// Initializes from an existing endpoint
			CEndpointDescription(CEndpointDescription* pEndPointDescription);
			// Releases all resources used by the object.
			~CEndpointDescription(void);
			// Returns the globally unique identifier for the server application.
			//OpcUa_String* GetServerUri();
			// Returns the URL of the server endpoint.
			OpcUa_String* GetEndpointUrl();
			// Returns the security mode used by the endpoint.
			OpcUa_MessageSecurityMode GetSecurityMode();
			// Look up the securityMode of the EndpointDescription to String
			OpcUa_StatusCode GetSecurityModeAsString(OpcUa_String* pSecurityMode);
			// Returns the security policy used by the endpoint.
			OpcUa_String* GetSecurityPolicyUri();
			OpcUa_StatusCode GetShortSecurityPolicyUri(OpcUa_String* pPolicyUri);
			OpcUa_ByteString* GetServerCertificate();
			OpcUa_EndpointDescription*	GetInternalEndPointDescription();
			OpcUa_ApplicationDescription* GetApplicationDescription();
			OpcUa_String GetTransportProfileUri();
		private:
			OpcUa_EndpointDescription*	m_pInternalEndPointDescription;
		};
		typedef std::vector<CEndpointDescription*> CEndpointDescriptionList;
	} // namespace
}