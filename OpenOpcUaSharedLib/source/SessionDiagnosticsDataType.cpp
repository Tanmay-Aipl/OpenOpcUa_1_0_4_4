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
#include "SessionDiagnosticsDataType.h"
using namespace OpenOpcUa;
using namespace UASharedLib;
CSessionDiagnosticsDataType::CSessionDiagnosticsDataType(void)
{
	m_pInternalSessionDiagnosticsDataType=(OpcUa_SessionDiagnosticsDataType*)OpcUa_Alloc(sizeof(OpcUa_SessionDiagnosticsDataType));
	OpcUa_SessionDiagnosticsDataType_Initialize(m_pInternalSessionDiagnosticsDataType);
}
CSessionDiagnosticsDataType::CSessionDiagnosticsDataType(OpcUa_SessionDiagnosticsDataType* pSessionDiagnosticsDataType)
{
	m_pInternalSessionDiagnosticsDataType = OpcUa_Null;
	if (pSessionDiagnosticsDataType)
		m_pInternalSessionDiagnosticsDataType=Utils::Copy(pSessionDiagnosticsDataType);
}


CSessionDiagnosticsDataType::~CSessionDiagnosticsDataType(void)
{
	if (m_pInternalSessionDiagnosticsDataType)
	{
		OpcUa_SessionDiagnosticsDataType_Clear(m_pInternalSessionDiagnosticsDataType);
		OpcUa_Free(m_pInternalSessionDiagnosticsDataType);
		m_pInternalSessionDiagnosticsDataType=OpcUa_Null;
	}
}
// synchronisation du m_pInternalSessionDiagnosticsDataType en fonction des variables de classe
void CSessionDiagnosticsDataType::UpdateInternalSessionDiagnosticsDataType()
{
}
// synchronisation des variables de classe en fonction du m_pInternalSessionSecurityDiagnosticsDataType
void CSessionDiagnosticsDataType::UpdateVariables()
{
}
void CSessionDiagnosticsDataType::UpdateInstanceSize()
{
	//OpcUa_UInt16 iSize;			
}

OpcUa_NodeId CSessionDiagnosticsDataType::GetSessionId()
{
	OpcUa_NodeId nullNodeId;
	OpcUa_NodeId_Initialize(&nullNodeId);
	if (m_pInternalSessionDiagnosticsDataType)
		return m_pInternalSessionDiagnosticsDataType->SessionId;
	else
		return nullNodeId;
}
void CSessionDiagnosticsDataType::SetSessionId(OpcUa_NodeId* aNodeId)
{
	OpcUa_NodeId_CopyTo(aNodeId, &(m_pInternalSessionDiagnosticsDataType->SessionId));
}

OpcUa_String CSessionDiagnosticsDataType::GetSessionName() 
{ 
	return m_pInternalSessionDiagnosticsDataType->SessionName;
}
void CSessionDiagnosticsDataType::SetSessionName(OpcUa_String strVal)
{ 
	OpcUa_String_CopyTo(&strVal, &(m_pInternalSessionDiagnosticsDataType->SessionName));
}
OpcUa_String CSessionDiagnosticsDataType::GetServerUri()
{ 
	return m_pInternalSessionDiagnosticsDataType->ServerUri;
}
void CSessionDiagnosticsDataType::SetServerUri(OpcUa_String strVal)
{ 
	OpcUa_String_CopyTo(&strVal, &(m_pInternalSessionDiagnosticsDataType->ServerUri));
}
OpcUa_ApplicationDescription CSessionDiagnosticsDataType::GetClientDescription()
{ 
	return m_pInternalSessionDiagnosticsDataType->ClientDescription;
}
void CSessionDiagnosticsDataType::SetClientDescription(const OpcUa_ApplicationDescription* pClientDescription)
{ 
	if (pClientDescription)
	{
		if (m_pInternalSessionDiagnosticsDataType)
		{
			OpcUa_ApplicationDescription_Clear(&(m_pInternalSessionDiagnosticsDataType->ClientDescription));
			// ApplicationName
			OpcUa_LocalizedText_Initialize(&(m_pInternalSessionDiagnosticsDataType->ClientDescription.ApplicationName));
			OpcUa_LocalizedText_CopyTo(&(pClientDescription->ApplicationName), &(m_pInternalSessionDiagnosticsDataType->ClientDescription.ApplicationName));
			// ApplicationType
			m_pInternalSessionDiagnosticsDataType->ClientDescription.ApplicationType = pClientDescription->ApplicationType;
		}
		// ApplicationUri
		if (OpcUa_String_StrLen(&(pClientDescription->ApplicationUri)))
		{
			if (m_pInternalSessionDiagnosticsDataType)
			{
				// ApplicationUri
				OpcUa_String_Initialize(&(m_pInternalSessionDiagnosticsDataType->ClientDescription.ApplicationUri));
				OpcUa_String_CopyTo(&(pClientDescription->ApplicationUri), &(m_pInternalSessionDiagnosticsDataType->ClientDescription.ApplicationUri));
				// ServerUri (Extra affectation because available here
				OpcUa_String_Initialize(&(m_pInternalSessionDiagnosticsDataType->ServerUri));
				OpcUa_String_CopyTo(&(pClientDescription->ApplicationUri), &(m_pInternalSessionDiagnosticsDataType->ServerUri));
			}
		}
		// ProductUri
		if (OpcUa_String_StrLen(&(pClientDescription->ProductUri)))
		{
			if (m_pInternalSessionDiagnosticsDataType)
			{
				OpcUa_String_Initialize(&(m_pInternalSessionDiagnosticsDataType->ClientDescription.ProductUri));
				OpcUa_String_StrnCpy(&(m_pInternalSessionDiagnosticsDataType->ClientDescription.ProductUri),
					&(pClientDescription->ProductUri),
					OpcUa_String_StrLen(&(pClientDescription->ProductUri)));
			}
		}
		// GatewayServerUri
		if (OpcUa_String_StrLen(&(pClientDescription->GatewayServerUri)))
		{
			if (m_pInternalSessionDiagnosticsDataType)
			{
				OpcUa_String_Initialize(&(m_pInternalSessionDiagnosticsDataType->ClientDescription.GatewayServerUri));
				OpcUa_String_StrnCpy(&(m_pInternalSessionDiagnosticsDataType->ClientDescription.GatewayServerUri),
					&(pClientDescription->GatewayServerUri),
					OpcUa_String_StrLen(&(pClientDescription->GatewayServerUri)));
			}
		}
		// DiscoveryProfileUri
		if (OpcUa_String_StrLen(&(pClientDescription->DiscoveryProfileUri)))
		{
			if (m_pInternalSessionDiagnosticsDataType)
			{
				OpcUa_String_Initialize(&(m_pInternalSessionDiagnosticsDataType->ClientDescription.DiscoveryProfileUri));
				OpcUa_String_StrnCpy(&(m_pInternalSessionDiagnosticsDataType->ClientDescription.DiscoveryProfileUri),
					&(pClientDescription->DiscoveryProfileUri),
					OpcUa_String_StrLen(&(pClientDescription->DiscoveryProfileUri)));
			}
		}
		if (m_pInternalSessionDiagnosticsDataType)
		{
			// NoOfDiscoveryUrls
			m_pInternalSessionDiagnosticsDataType->ClientDescription.NoOfDiscoveryUrls = pClientDescription->NoOfDiscoveryUrls;
			// DiscoveryUrls
			if (pClientDescription->NoOfDiscoveryUrls)
			{
				m_pInternalSessionDiagnosticsDataType->ClientDescription.DiscoveryUrls = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String)*pClientDescription->NoOfDiscoveryUrls);
				for (int ii = 0; ii < pClientDescription->NoOfDiscoveryUrls; ii++)
				{
					OpcUa_String_Initialize(&(m_pInternalSessionDiagnosticsDataType->ClientDescription.DiscoveryUrls[ii]));
					if (OpcUa_String_StrLen(&(pClientDescription->DiscoveryUrls[ii])) > 0)
					{
						OpcUa_String_StrnCpy(&(m_pInternalSessionDiagnosticsDataType->ClientDescription.DiscoveryUrls[ii]),
							&(pClientDescription->DiscoveryUrls[ii]),
							OpcUa_String_StrLen(&(pClientDescription->DiscoveryUrls[ii])));
					}
				}
			}
			else
				m_pInternalSessionDiagnosticsDataType->ClientDescription.DiscoveryUrls = OpcUa_Null;
		}
	}
}
OpcUa_SessionDiagnosticsDataType* CSessionDiagnosticsDataType::GetInternalPtr() 
{ 
	return m_pInternalSessionDiagnosticsDataType; 
}

OpcUa_String CSessionDiagnosticsDataType::GetEndpointUrl()
{
	return m_pInternalSessionDiagnosticsDataType->EndpointUrl;
}
void CSessionDiagnosticsDataType::SetEndpointUrl(OpcUa_String* strVal)
{
	OpcUa_String_CopyTo(strVal,&(m_pInternalSessionDiagnosticsDataType->EndpointUrl));
}
