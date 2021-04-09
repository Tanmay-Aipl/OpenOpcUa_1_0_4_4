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
#include "SecureChannel.h"
#include "opcua_securechannel_types.h"
#include "Utils.h"
using namespace OpenOpcUa;
using namespace UACoreServer;
using namespace UASharedLib;
//============================================================================
// SecureChannel::Constructor
//============================================================================
CSecureChannel::CSecureChannel(void)
{
	m_uSecureChannelId = 0;
	m_pSecurityPolicy=OpcUa_Null;
	m_uSecurityMode = (OpcUa_MessageSecurityMode)OPCUA_SECURECHANNEL_MESSAGESECURITYMODE_NONE; //OpcUa_MessageSecurityMode_None;
	m_pClientCertificate=OpcUa_Null;
	//OpcUa_ByteString_Initialize(&m_clientCertificate);
}
//
CSecureChannel::CSecureChannel(
	OpcUa_UInt32      uSecureChannelId,
	OpcUa_ByteString* pbsClientCertificate,
	OpcUa_String*     pSecurityPolicy,
	OpcUa_MessageSecurityMode      uSecurityMode):m_uSecureChannelId(uSecureChannelId),m_uSecurityMode(uSecurityMode)
{
	//m_uSecureChannelId = 0;
	m_pSecurityPolicy=OpcUa_Null;
	m_pClientCertificate=OpcUa_Null;
	//
	if (pbsClientCertificate)
	{
		if	(pbsClientCertificate->Length>0) //"http://opcfoundation.org/UA/SecurityPolicy#None"
		{
			SetClientCertificate(pbsClientCertificate);// m_pClientCertificate=Utils::Copy(pbsClientCertificate);
			if (!m_pClientCertificate)
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Critical error on ByteString copy. probably a memory allocation error or a NULL certificate was receive from the client\n");
		}
	}
	else
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"NULL ClientCertificate receive to create the secureChannel\n");
	if (pSecurityPolicy)
		m_pSecurityPolicy = Utils::Copy(pSecurityPolicy);
	else
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"NULL SecurityPolicy receive to create the secureChannel\n");
}

//============================================================================
// SecureChannel::Destructor
//============================================================================
CSecureChannel::~CSecureChannel(void)
{
	if (m_pSecurityPolicy)
	{
		OpcUa_String_Clear(m_pSecurityPolicy);
		OpcUa_Free(m_pSecurityPolicy);
		m_pSecurityPolicy=OpcUa_Null;
	}
	if (m_pClientCertificate)
	{
		OpcUa_ByteString_Clear(m_pClientCertificate);
		OpcUa_Free(m_pClientCertificate);
		m_pClientCertificate=OpcUa_Null;
	}
	m_uSecureChannelId=0;
	m_uSecurityMode=(OpcUa_MessageSecurityMode)OPCUA_SECURECHANNEL_MESSAGESECURITYMODE_INVALID;// OpcUa_MessageSecurityMode_Invalid;
}
OpcUa_ByteString* CSecureChannel::GetClientCertificate() 
{ 
	return m_pClientCertificate; 
}
void CSecureChannel::SetClientCertificate(const OpcUa_ByteString* pClientCertificate)
{
	if (pClientCertificate)
	{
		if (m_pClientCertificate)
		{
			OpcUa_ByteString_Clear(m_pClientCertificate);
			OpcUa_Free(m_pClientCertificate);
		}
		m_pClientCertificate = (OpcUa_ByteString*)OpcUa_Alloc(sizeof(OpcUa_ByteString));
		OpcUa_ByteString_Initialize(m_pClientCertificate);
		OpcUa_ByteString_CopyTo(pClientCertificate, m_pClientCertificate);
	}
}
OpcUa_UInt32 CSecureChannel::GetSecureChannelId()
{
	return m_uSecureChannelId;
}
void CSecureChannel::SetSecureChannelId(OpcUa_UInt32 uiVal)
{
	m_uSecureChannelId = uiVal;
}

OpcUa_MessageSecurityMode CSecureChannel::GetSecurityMode() 
{ 
	return m_uSecurityMode; 
}
void CSecureChannel::SetSecurityMode(OpcUa_MessageSecurityMode Mode) 
{ 
	m_uSecurityMode = Mode;
}

OpcUa_String* CSecureChannel::GetSecurityPolicy() 
{ 
	return m_pSecurityPolicy; 
}
void CSecureChannel::SetSecurityPolicy(OpcUa_String* str)
{
	if (str)
	{
		if (m_pSecurityPolicy)
		{
			OpcUa_String_Clear(m_pSecurityPolicy);
			OpcUa_Free(m_pSecurityPolicy);
		}
		m_pSecurityPolicy=(OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
		OpcUa_String_Initialize(m_pSecurityPolicy);

		OpcUa_String_CopyTo(str, m_pSecurityPolicy);
	}
}