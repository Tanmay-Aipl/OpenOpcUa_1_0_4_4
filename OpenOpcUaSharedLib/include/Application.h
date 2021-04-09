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
#include "opcua_types.h"
namespace OpenOpcUa
{
	namespace UASharedLib 
	{
		class SHAREDLIB_EXPORT CApplicationDescription
		{
		public:
			CApplicationDescription()
			{
				m_pInternalApplicationDescription=(OpcUa_ApplicationDescription*)OpcUa_Alloc(sizeof(OpcUa_ApplicationDescription));
				if (m_pInternalApplicationDescription)
					OpcUa_ApplicationDescription_Initialize(m_pInternalApplicationDescription);
			}
			CApplicationDescription(OpcUa_ApplicationDescription* pApplicationDescription)
			{				
				//m_pInternalApplicationDescription=(OpcUa_ApplicationDescription*)OpcUa_Alloc(sizeof(OpcUa_ApplicationDescription));
				//if (m_pInternalApplicationDescription)
				//	OpcUa_ApplicationDescription_Initialize(m_pInternalApplicationDescription);

				m_pInternalApplicationDescription=Utils::Copy(pApplicationDescription);
			}
			~CApplicationDescription()
			{
				if (m_pInternalApplicationDescription)
				{
					OpcUa_ApplicationDescription_Clear(m_pInternalApplicationDescription);
					OpcUa_Free(m_pInternalApplicationDescription);
				}
			}
			OpcUa_ApplicationDescription* GetInternalApplicationDescription() {return m_pInternalApplicationDescription;}
			OpcUa_LocalizedText	GetApplicationName() {return m_pInternalApplicationDescription->ApplicationName;}

			OpcUa_String*	GetApplicationUri() 
			{
				if (m_pInternalApplicationDescription)
					return  &(m_pInternalApplicationDescription->ApplicationUri);
				else
					return OpcUa_Null;
			}
			OpcUa_String*	GetProductUri() 
			{
				if (m_pInternalApplicationDescription)
					return &(m_pInternalApplicationDescription->ProductUri); 
				else
					return OpcUa_Null;
			} 
			OpcUa_ApplicationType GetApplicationType() 
			{
				if (m_pInternalApplicationDescription)
					return m_pInternalApplicationDescription->ApplicationType;
				else
					return OpcUa_ApplicationType_Server; // server by default
			}
			OpcUa_Int32    GetNoOfDiscoveryUrls() 
			{
				if (m_pInternalApplicationDescription)
					return m_pInternalApplicationDescription->NoOfDiscoveryUrls;
				else
					return -1;
			}
			OpcUa_String* GetDiscoveryUrls() 
			{
				if (m_pInternalApplicationDescription)
					return m_pInternalApplicationDescription->DiscoveryUrls;
				else
					return OpcUa_Null;
			}
		protected:

			OpcUa_ApplicationDescription* m_pInternalApplicationDescription;
		};
		typedef std::vector<CApplicationDescription*> CApplicationDescriptionList;
		// Stores information associated with a UA CApplication instance.

		class SHAREDLIB_EXPORT CApplication
		{
		public:
			CApplication(void);
			~CApplication(void);
			// Initializes the stack and CApplication.
			virtual void InitializeAbstractionLayer(void);
			OpcUa_StatusCode InitializeTrace();
			// Frees all resources used by the stack and CApplication.
			virtual void Uninitialize(void);
			// Initializes security and loads the CApplication instance certificate from an OpenSSL certificate store.
			virtual OpcUa_StatusCode InitializeSecurity(
														OpcUa_String* sApplicationUri,
														OpcUa_LocalizedText* sApplicationName);
			OpcUa_StatusCode TrustCertificate(OpcUa_ByteString* pCertificate);// Adds the certificate to the trusted peer store for the CApplication.				
			OpcUa_String GetCertificateStorePath();// Returns the certificate store path.
			void SetCertificateStorePath(OpcUa_String certificateStorePath);// Sets the certificate store path.
			OpcUa_StatusCode CreateCertificate();			
			OpcUa_ByteString* GetCertificate();	// Returns the CApplication instance certificate.	
			void  SetCertificate(OpcUa_ByteString* pCertificate); // Set the instance certificate
			OpcUa_Key* GetPrivateKey();	// Returns the CApplication instance certificate's private key.	
			void SetPrivateKey(OpcUa_Key aKey);
			OpcUa_CertificateStoreConfiguration* GetPkiConfig();// Returns the configuration for the PKI provider used by the CApplication.					
			OpcUa_PKIProvider* GetPkiProvider();// Returns the PKI provider used by the CApplication.				
			OpcUa_PKIProvider* GetX509UserPkiProvider(); // Returns the PKI provider used by the CApplication.					
			OpcUa_EncodeableTypeTable* GetTypeTable();// Returns the type table used by the CApplication.
			void SetApplicationName(OpcUa_LocalizedText* Val);
			OpcUa_LocalizedText* GetApplicationName() ;			
			OpcUa_StatusCode DiscoverEndpoints(const OpcUa_String& discoveryUrl,CEndpointDescriptionList* pEndpoints);// Fetches the endpoint descriptions from a discovery endpoint.
			OpcUa_StatusCode LoadPFXCertificate(); // will load PFX
			OpcUa_StatusCode LoadDERCertificate(); // will load DER
			OpcUa_StatusCode RejectCertificate(OpcUa_ByteString*pCertificate);
			OpcUa_Boolean IsPfxDerValide();
			// Trace
			OpcUa_UInt32 GetTraceLevel();
			void SetTraceLevel(OpcUa_UInt32 iTraceLevel);
			OpcUa_UInt32 GetTraceOutput();
			void SetTraceOutput(OpcUa_UInt32 iTraceOutput);
		private:
			// Frees memory allocated by the object.
			OpcUa_LocalizedText*					m_pApplicationName; // il s'agit du nom de l'application. Pour un serveur il s'agira du nom du serveur
																		// ce nom doit être unique et permettra de créer le certificat de l'application
			OpcUa_ByteString*						m_pCertificate;
			OpcUa_Key*								m_pPrivateKey;
			OpcUa_String							m_certificateStorePath; // Chemin complet sur le repertoire qui stocke le magasin de certificate
			OpcUa_String*							m_pThumbprint;
			// platform layer related
			OpcUa_Handle							m_hPlatformLayer;
			OpcUa_ProxyStubConfiguration			m_tConfiguration;
			OpcUa_EncodeableTypeTable				m_tTypeTable;
			// pki related
			OpcUa_PKIProvider*						m_pPkiProvider;
			OpcUa_CertificateStoreConfiguration*	m_pPkiConfig;
			OpcUa_PKIProvider*						m_pUserX509PkiProvider;
			OpcUa_CertificateStoreConfiguration*	m_pUserX509PkiConfig;
			OpcUa_Boolean							m_bPfxDerValide; // permet d'indiquer que le certificat dans les fichiers DER et PFX correspond
		};
	} // namespace 

}