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

//============================================================================
// EndpointDescription::Constructor
//============================================================================
CEndpointDescription::CEndpointDescription(void)
{
	//m_endpointUrl=OpcUa_Null;
	//m_eSecurityMode = OpcUa_MessageSecurityMode_Invalid;
	m_pInternalEndPointDescription=OpcUa_Null;
	//m_pServerCertificate=OpcUa_Null;
	//m_securityPolicyUri=OpcUa_Null;
	//m_serverUri =OpcUa_Null;
}

//============================================================================
// CEndpointDescription::Constructor(OpcUa_EndpointDescription)
//============================================================================
CEndpointDescription::CEndpointDescription(OpcUa_EndpointDescription* pDescription)
{
	m_pInternalEndPointDescription=OpcUa_Null;

	if (pDescription)
		m_pInternalEndPointDescription=Utils::Copy(pDescription);	
}
CEndpointDescription::CEndpointDescription(CEndpointDescription* pEndPointDescription)
{
	m_pInternalEndPointDescription=OpcUa_Null;
	if (pEndPointDescription)		
		m_pInternalEndPointDescription=Utils::Copy(pEndPointDescription->m_pInternalEndPointDescription);
}

//============================================================================
// CEndpointDescription::Destructor
//============================================================================
CEndpointDescription::~CEndpointDescription(void)
{
	if (m_pInternalEndPointDescription)
	{
		OpcUa_EndpointDescription_Clear(m_pInternalEndPointDescription);
		OpcUa_Free(m_pInternalEndPointDescription);
	}
}

// Returns the URL of the server endpoint.
OpcUa_String* CEndpointDescription::GetEndpointUrl()
{
	if (m_pInternalEndPointDescription)
		return &(m_pInternalEndPointDescription->EndpointUrl);
	else
		return OpcUa_Null;
}

// Returns the security mode used by the endpoint.
OpcUa_MessageSecurityMode CEndpointDescription::GetSecurityMode()
{
	if (m_pInternalEndPointDescription)
		return m_pInternalEndPointDescription->SecurityMode;
	else
		return OpcUa_MessageSecurityMode_Invalid;
}
OpcUa_StatusCode CEndpointDescription::GetSecurityModeAsString(OpcUa_String* pSecurityMode)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	if (pSecurityMode)
	{
		if (m_pInternalEndPointDescription)
		{
			OpcUa_String_Initialize(pSecurityMode);
			switch (m_pInternalEndPointDescription->SecurityMode)
			{
			case OpcUa_MessageSecurityMode_Invalid:
				OpcUa_String_AttachCopy(pSecurityMode, "Invalid");
				break;
			case OpcUa_MessageSecurityMode_None:
				OpcUa_String_AttachCopy(pSecurityMode, "None");
				break;
			case OpcUa_MessageSecurityMode_Sign:
				OpcUa_String_AttachCopy(pSecurityMode, "Sign");
				break;
			case OpcUa_MessageSecurityMode_SignAndEncrypt:
				OpcUa_String_AttachCopy(pSecurityMode, "Sign&Encrypt");
				break;
			default:
				OpcUa_String_AttachCopy(pSecurityMode, "SecurityMode Unknown");
				break;
			}
		}
		else
			return OpcUa_BadInvalidArgument;
	}
	else
		return OpcUa_BadInvalidArgument;
	return uStatus;
}

// Returns the security policy used by the endpoint.
OpcUa_String* CEndpointDescription::GetSecurityPolicyUri()
{
	if (m_pInternalEndPointDescription)
		return &(m_pInternalEndPointDescription->SecurityPolicyUri);
	else
		return OpcUa_Null;
}
OpcUa_StatusCode CEndpointDescription::GetShortSecurityPolicyUri(OpcUa_String* pPolicyUri)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	if (pPolicyUri)
	{
		if (m_pInternalEndPointDescription)
		{
			char cs = 0x23; //#
			char* cPos = OpcUa_Null;
			char* fullPolicyUri = OpcUa_String_GetRawString(&m_pInternalEndPointDescription->SecurityPolicyUri);
			// recherche # dans SecurityPolicyUri
			cPos =OpcUa_StrrChrA(fullPolicyUri, cs);
			if (cPos)
				OpcUa_String_AttachCopy(pPolicyUri, cPos);
		}
		else
			return OpcUa_BadInvalidArgument;
	}
	else
		return OpcUa_BadInvalidArgument;
	return uStatus;
}
OpcUa_ByteString* CEndpointDescription::GetServerCertificate()
{
	if (m_pInternalEndPointDescription)
		return &(m_pInternalEndPointDescription->ServerCertificate);
	else
		return OpcUa_Null;
}
OpcUa_EndpointDescription*	CEndpointDescription::GetInternalEndPointDescription() 
{
	return m_pInternalEndPointDescription;
}
OpcUa_ApplicationDescription* CEndpointDescription::GetApplicationDescription()
{
	if (m_pInternalEndPointDescription)
		return &(m_pInternalEndPointDescription->Server);
	else
		return OpcUa_Null;
}
OpcUa_String CEndpointDescription::GetTransportProfileUri()
{
	OpcUa_String NullString;
	OpcUa_String_Initialize(&NullString);
	if (m_pInternalEndPointDescription)
	{
		return m_pInternalEndPointDescription->TransportProfileUri;

	}
	else
		return NullString;
}