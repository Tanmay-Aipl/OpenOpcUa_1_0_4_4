/*****************************************************************************
	  Author
		©. Michel Condemine, 4CE Industry (2010-2012)
	  
	  Contributors
	© Guillaume Lemarchand, Euriware, Linux and Linux Embedded build (2011)
	© Philippe Buchet, JRC (2011) - Windows CE build


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
/*============================================================================
 * OpcUa_Certificate_FindContext
 *===========================================================================*/
struct OpcUa_Certificate_FindContext
{
	HCERTSTORE Store;
	HANDLE File;
#ifndef _GNUC_
	PCCERT_CONTEXT Context;
#else
	SSL_CTX *Context;
#endif
};
OpcUa_StatusCode OpcUa_WriteFile(
	OpcUa_StringA  a_sFilePath,
	OpcUa_Byte*    a_pBuffer,
	OpcUa_UInt32   a_uBufferLength);

OpcUa_StatusCode OpcUa_ReadFile(OpcUa_StringA a_sFilePath,OpcUa_ByteString* a_pBuffer);
/** 
 * @brief Creates a certificate signed by a certificate authority.
 *
 * @param sStorePath                [in]  The full path to the store to place the certificate in. 
 * @param sApplicationName          [in]  The name of the application.
 * @param uNoOfHostNames            [in]  The number of host names.
 * @param pHostNames                [in]  A list for host names for the machine.
 * @param uKeyType                  [in]  The type of key. 0 chooses default. Only OpcUa_Crypto_Rsa_Id is supported now.
 * @param uKeySize                  [in]  The size of key in bits (1024 or 2048)
 * @param uLifetimeInMonths         [in]  The lifetime of the certificate in months.
 * @param eFileFormat               [in]  The format of the private key file.
 * @param pIssuerPublicKey          [in]  The DER encoded issuer certificate (NULL for self-signed certificates).
 * @param pIssuerPrivateKey         [in]  The issuer's private key (NULL for self-signed certificates).
 * @param sPassword                 [in]  The password used to protect the key file.
 * @param pPublicKey                [out] The DER encoded certificate.
 * @param pPublicKeyFilePath        [out] The full path to the file containing the key.
 * @param pPrivateKey               [out] The private key.
 * @param pPrivateKeyFilePath       [out] The full path to the file containing the key.
 *
 * @return Status code; @see opcua_statuscodes.h
 */
OPCUA_EXPORT OpcUa_StatusCode OpcUa_Certificate_Create(
	OpcUa_StringA      a_sStorePath,
	OpcUa_StringA      a_sApplicationName,
	OpcUa_StringA      a_sApplicationUri,
	OpcUa_StringA      a_sOrganization,
	OpcUa_StringA      a_sSubjectName,
	OpcUa_UInt32       a_uNoOfDomainNames,
	OpcUa_StringA*     a_pDomainNames,
	OpcUa_UInt32       a_uKeyType,
	OpcUa_UInt32       a_uKeySize,
	OpcUa_UInt32       a_uLifetimeInMonths,
	OpcUa_Boolean      a_bIsCA,
	OpcUa_P_FileFormat a_eFileFormat,
	OpcUa_ByteString*  a_pIssuerCertificate,   
	OpcUa_Key*         a_pIssuerPrivateKey,
	OpcUa_StringA      a_sPassword,
	OpcUa_ByteString*  a_pCertificate,   
	OpcUa_StringA*     a_pCertificateFilePath,  
	OpcUa_Key*         a_pPrivateKey,
	OpcUa_StringA*     a_pPrivateKeyFilePath);

/** 
 * @brief Gets the thumbprint for a certificate.
 *
 * @param pCertificate  [in]  The X509 certificate encoded as a DER blob.
 * @param pThumbprint   [out] The SHA1 thumbprint encoded as a hexadecimal string.
 *
 * @return Status code; @see opcua_statuscodes.h
 */
OPCUA_EXPORT
OpcUa_StatusCode OpcUa_Certificate_GetThumbprint(
	OpcUa_ByteString* a_pCertificate,
	OpcUa_StringA*    a_pThumbprint);

/** 
 * @brief Gets the common name for a certificate.
 *
 * @param pCertificate  [in]  The X509 certificate encoded as a DER blob.
 * @param pThumbprint   [out] The common name field from the certificate subject name.
 *
 * @return Status code; @see opcua_statuscodes.h
 */
OPCUA_EXPORT
OpcUa_StatusCode OpcUa_Certificate_GetCommonName(
	OpcUa_ByteString* a_pCertificate,
	OpcUa_StringA*    a_pCommonName);

/** 
 * @brief Saves a certificate in a store.
 *
 * @param sFriendlyName [in]  A human readable name for the certificate.
 * @param pCertificate  [in]  The X509 certificate encoded as a DER blob.
 * @param pFilePath     [out] The full path to the file containing the key.
 *
 * @return Status code; @see opcua_statuscodes.h
 */
OPCUA_EXPORT
OpcUa_StatusCode OpcUa_Certificate_SavePublicKeyInStore(
	OpcUa_StringA     a_sStorePath,
	OpcUa_ByteString* a_pCertificate,
	OpcUa_StringA*    a_pFilePath);

/** 
 * @brief Writes the private key to a file.
 *
 * @param sStorePath      [in]  The full path to the store to place the certificate in. 
 * @param eFileFormat     [in]  The format of the file to save (PEM or PKCK#12).
 * @param sPassword       [in]  The password to use to protect the file.
 * @param pCertificate    [in]  The public key encoded as a DER blob.
 * @param pPrivateKey     [in]  The private key.
 * @param pFilePath       [out] The full path to the file containing the key.
 *
 * @return Status code; @see opcua_statuscodes.h
 */
OPCUA_EXPORT
OpcUa_StatusCode OpcUa_Certificate_SavePrivateKeyInStore(
	OpcUa_StringA      a_sStorePath,
	OpcUa_P_FileFormat a_eFileFormat,
	OpcUa_StringA      a_sPassword,    
	OpcUa_ByteString*  a_pCertificate,   
	OpcUa_Key*         a_pPrivateKey,
	OpcUa_StringA*     a_pFilePath);


/** 
 * @brief Reads the public key file to the Windows certificate store.
 *
 * @param pContext         [in/out] The context used to continue a previous operation.
 * @param bUseMachineStore [in]     If true use the machine store; otherwise use the current user store.
 * @param sStoreName       [in]     The name of Windows certificate store.
 * @param sCommonName      [in]     The common name of the certificate (ignored if null).
 * @param sThumbprint      [in]     The SHA1 thumbprint of the certificate (ignored if null).
 * @param pCertificate     [out]    The certificate encoded as a DER blob.
 *
 * @return Status code; @see opcua_statuscodes.h
 */
OPCUA_EXPORT
OpcUa_StatusCode OpcUa_Certificate_FindCertificateInWindowsStore(
	OpcUa_Handle*     a_pContext,    
	OpcUa_Boolean     a_bUseMachineStore,
	OpcUa_StringA     a_sStoreName,
	OpcUa_StringA     a_sCommonName,
	OpcUa_StringA     a_sThumbprint,
	OpcUa_ByteString* a_pCertificate);

/** 
 * @brief Reads the public key file to the certificate store.
 *
 * @param pContext       [in/out] The context used to continue a previous operation.
 * @param sStorePath,    [in]     The full path to the certificate store.
 * @param bHasPrivateKey [in]     Whether a private key is required.
 * @param sPassword      [in]     The password used to access the private key.
 * @param sCommonName    [in]     The common name of the certificate (ignored if null).
 * @param sThumbprint    [in]     The SHA1 thumbprint of the certificate (ignored if null).
 * @param pCertificate   [out]    The certificate encoded as a DER blob.
 * @param pPrivateKey    [out]    The private key.
 *
 * @return Status code; @see opcua_statuscodes.h
 */
OPCUA_EXPORT
OpcUa_StatusCode OpcUa_Certificate_FindCertificateInStore(
	OpcUa_Certificate_FindContext**     a_pContext,    
	OpcUa_StringA     a_sStorePath,
	OpcUa_Boolean     a_bHasPrivateKey,
	OpcUa_StringA     a_sPassword,
	OpcUa_StringA     a_sCommonName,
	OpcUa_StringA     a_sThumbprint,
	OpcUa_ByteString* a_pCertificate,
	OpcUa_Key*        a_pPrivateKey/*,
	OpcUa_String*     a_FullFileName*/);

/** 
 * @brief Frees the context previous returned.
 *
 * @param pContext [in/out] The context used to continue a find previous operation.
 *
 * @return Status code; @see opcua_statuscodes.h
 */
OPCUA_EXPORT
OpcUa_StatusCode OpcUa_Certificate_FreeFindContext(
	OpcUa_Handle* a_pContext);

/** 
 * @brief Exports the private key from a Windows store to an OpenSSL store.
 *
 * @param bUseMachineStore [in]  If true use the machine store; otherwise use the current user store.
 * @param sStoreName       [in]  The name of Windows certificate store.
 * @param pCertificate     [in]  The certificate to export.
 * @param sPassword        [in]  The password to use to protect the file.
 * @param sTargetStorePath [in]  The full path to the target OpenSSL store.
 * @param pPrivateKey      [out] The private key.
 *
 * @return Status code; @see opcua_statuscodes.h
 */
OPCUA_EXPORT
OpcUa_StatusCode OpcUa_Certificate_ExportPrivateKeyFromWindowsStore(
	OpcUa_Boolean     a_bUseMachineStore,
	OpcUa_StringA     a_sStoreName,
	OpcUa_ByteString* a_pCertificate,
	OpcUa_StringA     a_sPassword,
	OpcUa_StringA     a_sTargetStorePath,
	OpcUa_Key*        a_pPrivateKey);

/** 
 * @brief Imports the certificate to a Windows store.
 *
 * @param pCertificate     [in]  The certificate to import.
 * @param bUseMachineStore [in]  If true use the machine store; otherwise use the current user store.
 * @param sStoreName       [in]  The name of Windows certificate store.
 *
 * @return Status code; @see opcua_statuscodes.h
 */
OPCUA_EXPORT
OpcUa_StatusCode OpcUa_Certificate_ImportToWindowsStore(
	OpcUa_ByteString* a_pCertificate,
	OpcUa_Boolean     a_bUseMachineStore,
	OpcUa_StringA     a_sStoreName);

/** 
 * @brief Imports the private key from an OpenSSL store to a Windows store.
 *
 * @param sSourceStorePath [in]  The full path to the source OpenSSL store.
 * @param pCertificate     [in]  The certificate to import.
 * @param sPassword        [in]  The password to use to protect the file.
 * @param bUseMachineStore [in]  If true use the machine store; otherwise use the current user store.
 * @param sStoreName       [in]  The name of Windows certificate store.
 *
 * @return Status code; @see opcua_statuscodes.h
 */
OPCUA_EXPORT
OpcUa_StatusCode OpcUa_Certificate_ImportPrivateKeyToWindowsStore(
	OpcUa_StringA     a_sSourceStorePath,
	OpcUa_ByteString* a_pCertificate,
	OpcUa_StringA     a_sPassword,
	OpcUa_Boolean     a_bUseMachineStore,
	OpcUa_StringA     a_sStoreName);

/** 
 * @brief Extracts the specified information from the certificate..
 *
 * @param pCertificate      [in]   The certificate to process.
 * @param psNameEntries     [out]  All of the entries in the subject name.
 * @param puNoOfNameEntries [out]  The number of the entries in the subject name.
 * @param psCommonName      [out]  The common name.
 * @param psThumbprint      [out]  The thumbprint.
 * @param psApplicationUri  [out]  The application uri.
 * @param psDomains         [out]  The domains.
 * @param puNoOfDomains     [out]  The number of domains.
 *
 * @return Status code; @see opcua_statuscodes.h
 */
OPCUA_EXPORT
OpcUa_StatusCode OpcUa_Certificate_GetInfo(
	OpcUa_ByteString* a_pCertificate,
	OpcUa_StringA**   a_psNameEntries,
	OpcUa_UInt32*     a_puNoOfNameEntries,
	OpcUa_StringA*    a_psCommonName,
	OpcUa_CharA**     a_psThumbprint, //out
	OpcUa_CharA**     a_psApplicationUri,
	OpcUa_StringA**   a_psDomains,
	OpcUa_UInt32*     a_puNoOfDomains);

OPCUA_EXPORT OpcUa_StatusCode OpcUa_Certificate_GetDateBound(OpcUa_ByteString* a_pCertificate,OpcUa_DateTime* ValidFrom,OpcUa_DateTime* ValidTo);
//
// Create an application URI for the local machine
//
OPCUA_EXPORT OpcUa_StatusCode OpcUa_CreateApplication_Uri(
														OpcUa_StringA a_sApplicationName,
														std::string* ApplicationUri);
/** 
 * @brief Looks up the domain names for the IP address.
 *
 * @param sAddress    [in]  The IP address
 * @param pDomainName [out] The domain name.
 *
 * @return Status code; @see opcua_statuscodes.h
 */
OPCUA_EXPORT
OpcUa_StatusCode OpcUa_LookupDomainName(
	OpcUa_StringA  a_sAddress,
	OpcUa_StringA* a_pDomainName);

/** 
 * @brief Looks up the names and IP address for the localhost.
 *
 * @param pHostNames     [out] The hostnames.
 * @param pNoOfHostNames [out] The number of hostnames.
 *
 * @return Status code; @see opcua_statuscodes.h
 */
OPCUA_EXPORT
OpcUa_StatusCode OpcUa_Certificate_LookupLocalhostNames(
	OpcUa_StringA** a_pHostNames,
	OpcUa_UInt32*   a_pNoOfHostNames);


/** 
 * @brief Loads a private key from a file.
 *
 * @return Status code; @see opcua_statuscodes.h
 */
OpcUa_StatusCode OpcUa_Certificate_LoadPrivateKeyFromFile(
	OpcUa_StringA      a_sFilePath,
	OpcUa_P_FileFormat a_eFileFormat,
	OpcUa_StringA      a_sPassword,    
	OpcUa_ByteString*  a_pCertificate,   
	OpcUa_Key*         a_pPrivateKey);

OPCUA_EXPORT
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_LoadLibrary(const OpcUa_String* LibraryName, void** hInst);
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_FreeLibrary(void* hInst);

/**
* @brief Create a Pki. This mean initialize the tree folder that contains the certificates.
*
* @param szRootStoreLocation	 [in] The Root of the certificate store. the 4 next char* will be appended to it
* @param szTrustedLocation		 [in] name of the Trusted Location will be appended to szRootStoreLocation (This is the folder for Trusted certificate)
* @param szRevokedLocation		 [in] name of the Revoked Location will be appended to szRootStoreLocation (This is the folder for revoked certificate)
* @param szIssuerLocation		 [in] name of the Issuer Location will be appended to szRootStoreLocation (this is the folder for certification authority)
* @param szRevokedIssuerLocation [in] name of the RevokedIssuer Location will be appended to szRootStoreLocation (this is the folder for revoked certification authority)
* @param pPkiProviders			 [out] The newly created PkiProvider. 
*
* @return Status code; @see opcua_statuscodes.h
*/
OPCUA_EXPORT
OpcUa_StatusCode OpcUa_Certificate_CreatePkiProvider(OpcUa_CharA* szRootStoreLocation,
													OpcUa_CharA* szTrustedLocation, 
													OpcUa_CharA* szRevokedLocation, 
													OpcUa_CharA* szIssuerLocation, 
													OpcUa_CharA* szRevokedIssuerLocation, 
													OpcUa_CertificateStoreConfiguration** ppPkiConfig,
													OpcUa_PKIProvider** pPkiProvide);