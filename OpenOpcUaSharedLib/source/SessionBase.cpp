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
using namespace OpenOpcUa;
using namespace UASharedLib;

CSessionBase::CSessionBase(void)
{
	m_pEndpoint=OpcUa_Null;
	m_pAuthenticationToken=OpcUa_Null;
	OpcUa_NodeId_Initialize(&m_tSessionId);
	m_pSessionName = OpcUa_Null;
	m_ClassName=std::string("UASharedLib::CSessionBase");
}

CSessionBase::~CSessionBase(void)
{
	OpcUa_NodeId_Clear(&m_tSessionId);
	if (m_pAuthenticationToken)
	{
		OpcUa_NodeId_Clear(m_pAuthenticationToken);
		OpcUa_Free(m_pAuthenticationToken);
		m_pAuthenticationToken=OpcUa_Null;
	}
	if (m_pEndpoint)
		delete m_pEndpoint;
	if (m_pSessionName)
	{
		OpcUa_String_Clear(m_pSessionName);
		OpcUa_Free(m_pSessionName);
	}
}
void CSessionBase::SetAuthenticationToken(OpcUa_NodeId* AuthenticationToken) 
{
	if (AuthenticationToken)
	{
		if (m_pAuthenticationToken)
		{
			OpcUa_NodeId_Clear(m_pAuthenticationToken);
			OpcUa_Free(m_pAuthenticationToken);
		}
		m_pAuthenticationToken=(OpcUa_NodeId*)OpcUa_Alloc(sizeof(OpcUa_NodeId));
		OpcUa_NodeId_Initialize(m_pAuthenticationToken);
		OpcUa_NodeId_CopyTo(AuthenticationToken, m_pAuthenticationToken);
	}
}

void CSessionBase::SetSessionName(OpcUa_String* strVal)
{
	if (strVal)
	{
		if (m_pSessionName)
			OpcUa_String_Clear(m_pSessionName);
		else
			m_pSessionName = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
		OpcUa_String_Initialize(m_pSessionName);
		OpcUa_String_CopyTo(strVal, m_pSessionName);
	}
}
OpcUa_NodeId CSessionBase::GetSessionId() 
{ 
	return m_tSessionId; 
}
void CSessionBase::SetSessionId(OpcUa_NodeId* pValId) 
{ 
	OpcUa_NodeId_CopyTo(pValId,&m_tSessionId); 
}

CEndpointDescription* CSessionBase::GetEndpointDescription() 
{ 
	return m_pEndpoint; 
}
void CSessionBase::SetEndpointDescription(CEndpointDescription* pEndPointDescription) 
{ 
	m_pEndpoint = pEndPointDescription; 
}
OpcUa_NodeId* CSessionBase::GetAuthenticationToken() 
{ 
	return m_pAuthenticationToken; 
}
OpcUa_String*	CSessionBase::GetSessionName() 
{ 
	return m_pSessionName; 
}