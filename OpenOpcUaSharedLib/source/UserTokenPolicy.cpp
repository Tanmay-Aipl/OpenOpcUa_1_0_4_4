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
#include "UserTokenPolicy.h"
using namespace OpenOpcUa;
using namespace UASharedLib;
// Creates an empty policy.
CUserTokenPolicy::CUserTokenPolicy(void)
{ 
	m_eTokenType=OpcUa_UserTokenType_Anonymous;
	m_securityPolicyUri = OpcUa_Null;
}

// Initializes the object from a OpcUa_UserTokenPolicy.
CUserTokenPolicy::CUserTokenPolicy(OpcUa_UserTokenPolicy* pPolicy)
{
	if (pPolicy)
	{
		/*pPolicy->IssuedTokenType;
		pPolicy->IssuerEndpointUrl;
		pPolicy->PolicyId;*/
		m_securityPolicyUri = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
		if (m_securityPolicyUri)
		{
			OpcUa_String_Initialize(m_securityPolicyUri);
			OpcUa_String_CopyTo(&(pPolicy->SecurityPolicyUri), m_securityPolicyUri);
		}
		m_eTokenType = pPolicy->TokenType;
	}
}
// Releases all resources used by the object.
CUserTokenPolicy::~CUserTokenPolicy(void)
{
	if (m_securityPolicyUri)
	{
		OpcUa_String_Clear(m_securityPolicyUri);
		m_securityPolicyUri = OpcUa_Null;
	}
}
// Returns the id assigned by the server.
std::string CUserTokenPolicy::GetPolicyId()
{
	return m_policyId;
}

// Returns the token type.
OpcUa_UserTokenType CUserTokenPolicy::GetTokenType()
{
	return m_eTokenType;
}

// Returns the security policy used by the token.
OpcUa_String* CUserTokenPolicy::GetSecurityPolicyUri()
{
	return m_securityPolicyUri;
}