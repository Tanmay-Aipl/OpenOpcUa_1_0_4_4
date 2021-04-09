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
#include "SessionSecurityDiagnosticsDataType.h"
#include "SessionDiagnosticsDataType.h"
using namespace OpenOpcUa;
using namespace UASharedLib;
CSessionSecurityDiagnosticsDataType::CSessionSecurityDiagnosticsDataType(void)
{
	m_pInternalSessionSecurityDiagnosticsDataType=(OpcUa_SessionSecurityDiagnosticsDataType*)OpcUa_Alloc(sizeof(OpcUa_SessionSecurityDiagnosticsDataType));
	if (m_pInternalSessionSecurityDiagnosticsDataType)
		OpcUa_SessionSecurityDiagnosticsDataType_Initialize(m_pInternalSessionSecurityDiagnosticsDataType);
	else
		m_pInternalSessionSecurityDiagnosticsDataType = OpcUa_Null;
}
CSessionSecurityDiagnosticsDataType::CSessionSecurityDiagnosticsDataType(OpcUa_SessionSecurityDiagnosticsDataType* pSessionSecurityDiagnosticsDataType)
{
	if (pSessionSecurityDiagnosticsDataType)
		m_pInternalSessionSecurityDiagnosticsDataType = Utils::Copy(pSessionSecurityDiagnosticsDataType);
	else
		m_pInternalSessionSecurityDiagnosticsDataType = OpcUa_Null;
}


CSessionSecurityDiagnosticsDataType::~CSessionSecurityDiagnosticsDataType(void)
{
	if (m_pInternalSessionSecurityDiagnosticsDataType)
	{
		OpcUa_SessionSecurityDiagnosticsDataType_Clear(m_pInternalSessionSecurityDiagnosticsDataType);
		OpcUa_Free(m_pInternalSessionSecurityDiagnosticsDataType);
		m_pInternalSessionSecurityDiagnosticsDataType=OpcUa_Null;
	}
}
// synchronisation du m_pInternalSessionSecurityDiagnosticsDataType en fonction des variables de classe


// synchronisation des variables de classe en fonction du m_pInternalSessionSecurityDiagnosticsDataType
/*
void CSessionSecurityDiagnosticsDataType::UpdateVariables()
{
	if (m_pInternalSessionSecurityDiagnosticsDataType)
	{
		m_AuthenticationMechanism	=m_pInternalSessionSecurityDiagnosticsDataType->AuthenticationMechanism;
		m_ClientCertificate			=m_pInternalSessionSecurityDiagnosticsDataType->ClientCertificate;
		m_pClientUserIdHistory		=m_pInternalSessionSecurityDiagnosticsDataType->ClientUserIdHistory;
		m_ClientUserIdOfSession		=m_pInternalSessionSecurityDiagnosticsDataType->ClientUserIdOfSession;
		m_Encoding					=m_pInternalSessionSecurityDiagnosticsDataType->Encoding;
		m_NoOfClientUserIdHistory	=m_pInternalSessionSecurityDiagnosticsDataType->NoOfClientUserIdHistory;
		m_SecurityMode				=m_pInternalSessionSecurityDiagnosticsDataType->SecurityMode;
		m_SecurityPolicyUri			=m_pInternalSessionSecurityDiagnosticsDataType->SecurityPolicyUri;
		m_SessionId					=m_pInternalSessionSecurityDiagnosticsDataType->SessionId;
		m_TransportProtocol			=m_pInternalSessionSecurityDiagnosticsDataType->TransportProtocol;
		UpdateInstanceSize();
	}
}
*/
/*
void CSessionSecurityDiagnosticsDataType::UpdateInstanceSize()
{
	OpcUa_UInt16 iSize;
	m_InstanceSize=OpcUa_String_StrLen(&m_AuthenticationMechanism);
	m_InstanceSize+=m_ClientCertificate.Length;
	m_InstanceSize+=sizeof(OpcUa_Int32); //	m_NoOfClientUserIdHistory
	for (OpcUa_Int32 ii=0;ii<m_NoOfClientUserIdHistory;++ii)
		m_InstanceSize+=OpcUa_String_StrLen(m_pClientUserIdHistory);		
	m_InstanceSize+=OpcUa_String_StrLen(&m_ClientUserIdOfSession);		
	m_InstanceSize+=OpcUa_String_StrLen(&m_Encoding);			
	m_InstanceSize+=sizeof(OpcUa_Int32); //m_NoOfClientUserIdHistory	
	m_InstanceSize+=sizeof(OpcUa_MessageSecurityMode); //m_SecurityMode				
	m_InstanceSize+=OpcUa_String_StrLen(&m_SecurityPolicyUri);			
	Utils::GetNodeSize(m_SessionId,&iSize);
	m_InstanceSize+=iSize;
	m_InstanceSize+=OpcUa_String_StrLen(&m_TransportProtocol);			
}
*/
OpcUa_NodeId CSessionSecurityDiagnosticsDataType::GetSessionId() 
{ 
	return m_pInternalSessionSecurityDiagnosticsDataType->SessionId;
}
void CSessionSecurityDiagnosticsDataType::SetSessionId(OpcUa_NodeId* aNodeId)
{
	OpcUa_NodeId_CopyTo(aNodeId, &(m_pInternalSessionSecurityDiagnosticsDataType->SessionId));
}
OpcUa_ByteString CSessionSecurityDiagnosticsDataType::GetClientCertificate() 
{ 
	return m_pInternalSessionSecurityDiagnosticsDataType->ClientCertificate;
}
void CSessionSecurityDiagnosticsDataType::SetClientCertificate(OpcUa_ByteString* ClientCertificate)
{
	 OpcUa_ByteString_CopyTo(ClientCertificate,&(m_pInternalSessionSecurityDiagnosticsDataType->ClientCertificate));
}
OpcUa_MessageSecurityMode CSessionSecurityDiagnosticsDataType::GetSecurityMode() 
{ 
	return m_pInternalSessionSecurityDiagnosticsDataType->SecurityMode;
}
void CSessionSecurityDiagnosticsDataType::SetSecurityMode(OpcUa_MessageSecurityMode Mode)
{
	m_pInternalSessionSecurityDiagnosticsDataType->SecurityMode = Mode;
}
OpcUa_String CSessionSecurityDiagnosticsDataType::GetSecurityPolicyUri() 
{ 
	return m_pInternalSessionSecurityDiagnosticsDataType->SecurityPolicyUri;
}
void CSessionSecurityDiagnosticsDataType::SetSecurityPolicyUri(OpcUa_String* strVal)
{
	OpcUa_String_CopyTo(strVal, &(m_pInternalSessionSecurityDiagnosticsDataType->SecurityPolicyUri));
}
OpcUa_SessionSecurityDiagnosticsDataType* CSessionSecurityDiagnosticsDataType::GetInternalPtr() 
{ 
	return m_pInternalSessionSecurityDiagnosticsDataType; 
}

void CSessionSecurityDiagnosticsDataType::SetTransportProtocol(OpcUa_String* pszTransportProtocol)
{
	OpcUa_String_CopyTo(pszTransportProtocol, &(m_pInternalSessionSecurityDiagnosticsDataType->TransportProtocol));
}
OpcUa_String CSessionSecurityDiagnosticsDataType::GetTransportProtocol()
{
	return m_pInternalSessionSecurityDiagnosticsDataType->TransportProtocol;
}
OpcUa_String CSessionSecurityDiagnosticsDataType::GetEncoding()
{
	return m_pInternalSessionSecurityDiagnosticsDataType->Encoding;
}
void CSessionSecurityDiagnosticsDataType::SetEncoding(OpcUa_String* pszEncoding)
{
	OpcUa_String_CopyTo(pszEncoding, &(m_pInternalSessionSecurityDiagnosticsDataType->Encoding));
}
OpcUa_String CSessionSecurityDiagnosticsDataType::GetAuthenticationMechanism()
{
	return m_pInternalSessionSecurityDiagnosticsDataType->AuthenticationMechanism;
}
void CSessionSecurityDiagnosticsDataType::SetAuthenticationMechanism(OpcUa_String* pszEncoding)
{
	OpcUa_String_CopyTo(pszEncoding, &(m_pInternalSessionSecurityDiagnosticsDataType->AuthenticationMechanism));
}