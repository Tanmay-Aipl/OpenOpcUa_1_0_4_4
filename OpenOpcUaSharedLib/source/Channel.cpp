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

#include "stdafx.h"
#include "Channel.h"
using namespace OpenOpcUa;
using namespace UASharedLib;
CChannel::CChannel() 
{
	OpcUa_Mutex_Create(&m_ChannelMutex);
	m_pApplication=OpcUa_Null;
	//m_pInternalChannel = OpcUa_Null;
	m_hChannel = 0;
	//m_pSecurityToken = (OpcUa_Channel_SecurityToken*)OpcUa_Alloc(sizeof(OpcUa_Channel_SecurityToken));
	//if (m_pSecurityToken)
	//{
	//	m_pSecurityToken->eTokenType = OpcUa_Channel_SecurityTokenType_OpcSecureConversation;
	//	m_pSecurityToken->SecurityToken.pOpcChannelSecurityToken = (OpcUa_ChannelSecurityToken*)OpcUa_Alloc(sizeof(OpcUa_ChannelSecurityToken));
	//	if (m_pSecurityToken->SecurityToken.pOpcChannelSecurityToken)
	//		OpcUa_ChannelSecurityToken_Initialize(m_pSecurityToken->SecurityToken.pOpcChannelSecurityToken);

	//	// m_pSecurityToken->SecurityToken is a union. . So because we support UASC only we are not initialize the following one
	//	// OpcUa_ByteString_Initialize(&(m_pSecurityToken->SecurityToken.HttpsSecurityToken.bsServerCertificate));
	//	// OpcUa_String_Initialize(&(m_pSecurityToken->SecurityToken.HttpsSecurityToken.sUsedCipher));
	//}
	//m_eSecurityMode = OpcUa_MessageSecurityMode_None;
	//m_sSecurityPolicy=(OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	//if (m_sSecurityPolicy)
	//	OpcUa_String_Initialize(m_sSecurityPolicy);
	OpcUa_ByteString_Initialize(&m_tServerCertificate);	
	//m_endpointUrl=(OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	//if (m_endpointUrl)
	//	OpcUa_String_Initialize(m_endpointUrl);
}
//============================================================================
// Channel::Constructor
//============================================================================
CChannel::CChannel(CApplication* pApplication)
{
	OpcUa_Mutex_Create(&m_ChannelMutex);
	m_pApplication = pApplication;
	//m_pInternalChannel = OpcUa_Null;
	m_hChannel = 0;
	//m_pSecurityToken = (OpcUa_Channel_SecurityToken*)OpcUa_Alloc(sizeof(OpcUa_Channel_SecurityToken));
	//if (m_pSecurityToken)
	//{
	//	m_pSecurityToken->eTokenType = OpcUa_Channel_SecurityTokenType_OpcSecureConversation;
	//	m_pSecurityToken->SecurityToken.pOpcChannelSecurityToken = (OpcUa_ChannelSecurityToken*)OpcUa_Alloc(sizeof(OpcUa_ChannelSecurityToken));
	//	if (m_pSecurityToken->SecurityToken.pOpcChannelSecurityToken)
	//		OpcUa_ChannelSecurityToken_Initialize(m_pSecurityToken->SecurityToken.pOpcChannelSecurityToken);
	//	
	//	// m_pSecurityToken->SecurityToken is a union. . So because we support UASC only we are not initialize the following one
	//	// OpcUa_ByteString_Initialize(&(m_pSecurityToken->SecurityToken.HttpsSecurityToken.bsServerCertificate));
	//	// OpcUa_String_Initialize(&(m_pSecurityToken->SecurityToken.HttpsSecurityToken.sUsedCipher));
	//}
	//m_eSecurityMode = OpcUa_MessageSecurityMode_None;
	//m_sSecurityPolicy=(OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	//if (m_sSecurityPolicy)
	//	OpcUa_String_Initialize(m_sSecurityPolicy);
	OpcUa_ByteString_Initialize(&m_tServerCertificate);	
	//m_endpointUrl=(OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	//if (m_endpointUrl)
	//	OpcUa_String_Initialize(m_endpointUrl);
}

//============================================================================
// Channel::Destructor
//============================================================================
CChannel::~CChannel(void)
{
	Disconnect();
	//if (m_sSecurityPolicy)
	//{
	//	OpcUa_String_Clear(m_sSecurityPolicy);
	//	OpcUa_Free(m_sSecurityPolicy);
	//}
	OpcUa_ByteString_Clear(&m_tServerCertificate);	

	//if (m_pSecurityToken)
	//{	
	//	// So because we support UASC only we are not initialize the following one
	//	if (m_pSecurityToken->eTokenType == OpcUa_Channel_SecurityTokenType_OpcSecureConversation)
	//	{
	//		OpcUa_ChannelSecurityToken_Clear(m_pSecurityToken->SecurityToken.pOpcChannelSecurityToken); 
	//		OpcUa_Free(m_pSecurityToken->SecurityToken.pOpcChannelSecurityToken);
	//	}
	//	
	//	OpcUa_Free(m_pSecurityToken);
	//}
	//if (m_endpointUrl)
	//{
	//	OpcUa_String_Clear(m_endpointUrl);
	//	OpcUa_Free(m_endpointUrl);
	//}
	//OpcUa_String_Clear(m_endpointUrl);
	//OpcUa_Free(&m_endpointUrl);
	OpcUa_Mutex_Delete(&m_ChannelMutex);
	m_pApplication=OpcUa_Null;
}

//============================================================================
// Channel::Connect(endpointUrl)
//============================================================================
OpcUa_StatusCode CChannel::Connect(const OpcUa_String& endpointUrl, OpcUa_MessageSecurityMode eSecurityMode,  OpcUa_String szSecurityPolicy)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;

	if (OpcUa_String_StrLen(&szSecurityPolicy) == 0)
	{
		OpcUa_String_Initialize(&szSecurityPolicy);
		OpcUa_String_AttachCopy(&szSecurityPolicy, OpcUa_SecurityPolicy_None);
	}
	if (eSecurityMode==OpcUa_MessageSecurityMode_Invalid)
		eSecurityMode = OpcUa_MessageSecurityMode_None;


	OpcUa_ByteString_Initialize(&m_tServerCertificate);
	uStatus = InternalConnect(endpointUrl,eSecurityMode,szSecurityPolicy);
	OpcUa_String_Clear(&szSecurityPolicy);

	return uStatus;
}

//============================================================================
// Channel::Connect(endpoint)
//============================================================================
OpcUa_StatusCode CChannel::Connect(CEndpointDescription* pEndpoint)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	if (pEndpoint)
	{
		OpcUa_String* pSecurityPolicy=pEndpoint->GetSecurityPolicyUri();
		if(pSecurityPolicy)
		{
			OpcUa_ByteString* pTmpByteString=pEndpoint->GetServerCertificate();
			if (m_tServerCertificate.Length>0)
				OpcUa_ByteString_Clear(&m_tServerCertificate);
			OpcUa_ByteString_Initialize(&m_tServerCertificate);
			OpcUa_ByteString_CopyTo(pTmpByteString,&m_tServerCertificate);
			///////////////////////////////////////////////////////////////////////
			// extract the information from the certificate.
			OpcUa_CharA* sThumbprint = NULL;
			OpcUa_CharA* lApplicationUri = NULL;
			if (m_tServerCertificate.Data)
			{
				uStatus = OpcUa_Certificate_GetInfo(
					&m_tServerCertificate,
					NULL,
					NULL,
					NULL,
					&sThumbprint,
					&lApplicationUri,
					NULL,
					NULL);
			}
			
			if (uStatus!=OpcUa_Good)
				OpcUa_Trace(OPCUA_TRACE_EXTRA_LEVEL_ERROR,"OpcUa_Certificate_GetInfo failed 0x%05x\n",uStatus);
			else
			{
				///////////////////////////////////////////////////////////////////////
				OpcUa_MessageSecurityMode eSecurityMode = pEndpoint->GetSecurityMode();

				try
				{
					OpcUa_String* pszUrl=pEndpoint->GetEndpointUrl();
					uStatus = InternalConnect(*pszUrl, eSecurityMode, *pSecurityPolicy);
					if (uStatus!=OpcUa_Good)
						OpcUa_Trace(OPCUA_TRACE_EXTRA_LEVEL_ERROR,"Could not connect to local discovery server 0x%05x\n",uStatus);
					if (sThumbprint)
						OpcUa_Free(sThumbprint);
					if (lApplicationUri)
						OpcUa_Free(lApplicationUri);
				}
				catch (...)
				{
					OpcUa_Trace(OPCUA_TRACE_EXTRA_LEVEL_ERROR, "Method CChannel::Connect failed. Could not connect to local discovery server at %s.\r\n", OpcUa_String_GetRawString(pEndpoint->GetEndpointUrl() ));
				}
			}
		}
		else
			uStatus=OpcUa_BadInvalidArgument;
	}
	else
		uStatus=OpcUa_BadInvalidArgument;
	return uStatus;
}

//============================================================================
// Channel::InternalConnect
//============================================================================
OpcUa_StatusCode CChannel::InternalConnect(const OpcUa_String& endpointUrl, OpcUa_MessageSecurityMode eSecurityMode,OpcUa_String szSecurityPolicy)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_ByteString* PrivateKey_Key=(OpcUa_ByteString*)OpcUa_Alloc(sizeof(OpcUa_ByteString));
	OpcUa_ByteString_Initialize(PrivateKey_Key);
	OpcUa_Key* pPrivateKey=OpcUa_Null;//(OpcUa_Key*)OpcUa_Alloc(sizeof(OpcUa_Key));
	//OpcUa_Key_Initialize(pPrivateKey);
	if (eSecurityMode!=OpcUa_MessageSecurityMode_None) 
	{
		// recherche du certificat contenant la clé privée pour réaliser une connexion sur le LDS
		OpcUa_Certificate_FindContext* pContext = OpcUa_Null;
		OpcUa_ByteString tCertificate;
		OpcUa_ByteString_Initialize(&tCertificate);
		OpcUa_LocalizedText* sApplicationName=m_pApplication->GetApplicationName();
		if (sApplicationName)
		{
			if (OpcUa_String_StrLen(&(sApplicationName->Text))>0)
			{
				OpcUa_String* aFullFileName=OpcUa_Null;
				OpcUa_String certificateStorePath=m_pApplication->GetCertificateStorePath();
				OpcUa_String aTmpString;
				OpcUa_String_Initialize(&aTmpString);
				OpcUa_String_CopyTo(&(sApplicationName->Text),&aTmpString);
				OpcUa_StringA pString=OpcUa_String_GetRawString(&aTmpString);
				//
				pPrivateKey=m_pApplication->GetPrivateKey();
				//
				if (pPrivateKey==OpcUa_Null)
				{
					uStatus = OpcUa_Certificate_FindCertificateInStore(
						&pContext,
						OpcUa_String_GetRawString(&certificateStorePath),
						OpcUa_True, // will use privateKey 
						NULL,
						pString,
						NULL,
						&tCertificate,
						pPrivateKey);
					if (uStatus!=OpcUa_Good)
					{
						OpcUa_Trace(OPCUA_TRACE_EXTRA_LEVEL_ERROR,"OpcUa_Certificate_FindCertificateInStore failed. 0x%05x\n",uStatus);
					}
				}
				OpcUa_String_Clear(&aTmpString);
				if (aFullFileName)
				{
					OpcUa_String_Clear(aFullFileName);
					OpcUa_Free(aFullFileName);
					aFullFileName=OpcUa_Null;
				}
				OpcUa_Free(pContext);
			}
		}
		OpcUa_ByteString_Clear(&tCertificate);
	}
	// create the channel. only binary encoding supported at this time.
	OpcUa_Mutex_Lock(m_ChannelMutex);
	uStatus = OpcUa_Channel_Create(&m_hChannel, OpcUa_Channel_SerializerType_Binary);
	if (uStatus==OpcUa_Good)
	{    	
		OpcUa_String aStringTransportUri;
		OpcUa_String_Initialize(&aStringTransportUri);
		OpcUa_String_AttachCopy(&aStringTransportUri,OpcUa_TransportProfile_UaTcp);
		pPrivateKey=m_pApplication->GetPrivateKey();
		if (pPrivateKey)
			OpcUa_ByteString_CopyTo(&(pPrivateKey->Key),PrivateKey_Key);
		// connect to the server.
		OpcUa_Channel_SecurityToken* pSecurityToken = OpcUa_Null;
		
		uStatus = OpcUa_Channel_Connect(   
			m_hChannel,
			OpcUa_String_GetRawString(&endpointUrl),
			OpcUa_String_GetRawString(&aStringTransportUri),
			OpcUa_Null, // Pointer sur une fonction callback ou indiquer que l'etat de la connection a changé
			OpcUa_Null, // données associé à la fonction callback
			m_pApplication->GetCertificate(),
			PrivateKey_Key,
			&m_tServerCertificate,
			m_pApplication->GetPkiConfig(),
			&szSecurityPolicy,
			OPCUA_SECURITYTOKEN_LIFETIME_MAX,
			eSecurityMode,
			&pSecurityToken,
			UTILS_DEFAULT_TIMEOUT); 

		if (uStatus!=OpcUa_Good)
		{
			//if (m_pSecurityToken)
			//{
			//	if (m_pSecurityToken->SecurityToken.pOpcChannelSecurityToken)
			//		OpcUa_ChannelSecurityToken_Clear(m_pSecurityToken->SecurityToken.pOpcChannelSecurityToken);
			//}
			OpcUa_Trace(OPCUA_TRACE_EXTRA_LEVEL_ERROR,"Could not connect to server.\n");
		}
		OpcUa_String_Clear(&aStringTransportUri);
		//OpcUa_ByteString_Clear(PrivateKey_Key);
	}
	else
		OpcUa_Trace(OPCUA_TRACE_EXTRA_LEVEL_ERROR,"Could not create new channel\n");
	//
	//OpcUa_Key_Clear(pPrivateKey);
	//OpcUa_Free(pPrivateKey);
	//
	OpcUa_ByteString_Clear(PrivateKey_Key);
	OpcUa_Free(PrivateKey_Key);
	OpcUa_Mutex_Unlock(m_ChannelMutex);
	return uStatus;
}

//============================================================================
// Channel::Disconnect
//============================================================================
void CChannel::Disconnect()
{
	OpcUa_Mutex_Lock(m_ChannelMutex);
	if (m_hChannel)
	{
		OpcUa_Channel_Disconnect(m_hChannel);
		OpcUa_Channel_Delete(&m_hChannel);
		m_hChannel=OpcUa_Null;
	}
	
	OpcUa_Mutex_Unlock(m_ChannelMutex);
}
//void	CChannel::SetEndpointUrl(OpcUa_String* pString) 
//{
//	if (pString)
//	{
//		if (OpcUa_String_StrLen(pString)>0)
//		{
//			if (m_pInternalChannel)
//			{
//				OpcUa_String szUrl=m_pInternalChannel->Url;
//				OpcUa_Mutex_Lock(m_pInternalChannel->Mutex);
//				if (OpcUa_String_StrLen(&(szUrl)) > 0)
//					OpcUa_String_Clear(&(szUrl));
//				OpcUa_String_CopyTo(pString, &(szUrl));
//				OpcUa_Mutex_Unlock(m_pInternalChannel->Mutex);
//			}
//		}
//	}
//}
// Returns the stack assigned handled for the channel.
OpcUa_Channel CChannel::GetInternalHandle()
{
	return m_hChannel;
}
//OpcUa_String* CChannel::GetEndpointUrl() 
//{ 
//	if (m_pInternalChannel)
//		return &(m_pInternalChannel->Url);
//	else
//		return OpcUa_Null;
//}