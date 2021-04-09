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
#include "UABinding.h"
using namespace OpenOpcUa;
using namespace UACoreServer;
CUABinding::CUABinding()
{
	OpcUa_String_Initialize(&m_Port);
	OpcUa_String_Initialize(&m_Protocol);
	OpcUa_String_Initialize(&m_szBinding);
	OpcUa_String_Initialize(&m_TransportProfileUri);
	m_Encoding = OpcUa_Endpoint_SerializerType_Invalid;
}
CUABinding::~CUABinding()
{
	OpcUa_String_Clear(&m_Port);
	OpcUa_String_Clear(&m_Protocol);
	OpcUa_String_Clear(&m_szBinding);
	OpcUa_String_Clear(&m_TransportProfileUri);
}
OpcUa_String CUABinding::AsString()
{
	return m_szBinding;
}
OpcUa_String CUABinding::GetTransportProfileUri()
{
	return m_TransportProfileUri;
}
void CUABinding::UpdateBinding()
{
	if (OpcUa_String_StrLen(&m_szBinding) > 0)
		OpcUa_String_Clear(&m_szBinding);
	OpcUa_String_Initialize(&m_szBinding);
	// Create the tcp signature
	OpcUa_String* pszUaTcp = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszUaTcp);
	OpcUa_String_AttachCopy(pszUaTcp, "opc.tcp");

	// Create the https signature
	OpcUa_String* pszUaHttps = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszUaHttps);
	OpcUa_String_AttachCopy(pszUaHttps, "https");


	// Create the http signature
	OpcUa_String* pszUaHttp = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszUaHttp);
	OpcUa_String_AttachCopy(pszUaHttp, "http");


	// Create the protocol separator
	OpcUa_String* pszSeparatorUrl = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszSeparatorUrl);
	OpcUa_String_AttachCopy(pszSeparatorUrl, "://");

	// create the simple separator
	OpcUa_String* pszSeparatorUrl1 = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszSeparatorUrl1);
	OpcUa_String_AttachCopy(pszSeparatorUrl1, "/");

	// create the port separator
	OpcUa_String* pszPortSeparator = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pszPortSeparator);
	OpcUa_String_AttachCopy(pszPortSeparator, ":");


	// Add the Protocol
	OpcUa_String szProtocol = GetProtocol();
	if (OpcUa_String_StrLen(&szProtocol) > 0)
	{

		if (OpcUa_StrCmpA(OpcUa_String_GetRawString(&szProtocol), "TCP") == 0)
		{
			OpcUa_String_StrnCat(&m_szBinding,
				pszUaTcp,
				OpcUa_String_StrLen(pszUaTcp));
		}
		else
		{
			if (OpcUa_StrCmpA(OpcUa_String_GetRawString(&szProtocol), "HTTPS") == 0)
				OpcUa_String_StrnCat(&m_szBinding,
				pszUaHttps,
				OpcUa_String_StrLen(pszUaHttps));
			else
				OpcUa_String_StrnCat(&m_szBinding,
				pszUaHttps,
				OpcUa_String_StrLen(pszUaHttp));

		}
	}
	// Add the ://
	OpcUa_String_StrnCat(&m_szBinding,
		pszSeparatorUrl,
		OpcUa_String_StrLen(pszSeparatorUrl));

	// Add the HostName
	OpcUa_StringA pDomainName = OpcUa_Null;
	OpcUa_StatusCode uStatus = OpcUa_LookupDomainName((OpcUa_StringA)"127.0.0.1", &pDomainName);
	if (uStatus == OpcUa_Good)
	{
		if (pDomainName)
		{
			OpcUa_String* HostName = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
			OpcUa_String_Initialize(HostName);
			OpcUa_String_AttachCopy(HostName, pDomainName);

			OpcUa_String_StrnCat(&m_szBinding,
				HostName,
				OpcUa_String_StrLen(HostName));
			OpcUa_String_Clear(HostName);
			OpcUa_Free(HostName);
			OpcUa_Free(pDomainName);
			pDomainName = OpcUa_Null;
		}
	}
	// Add

	// Add the Port Separator
	OpcUa_String_StrnCat(&m_szBinding,
		pszPortSeparator,
		OpcUa_String_StrLen(pszPortSeparator));
	// Add the Port 
	OpcUa_String szPort = GetPort();
	if (OpcUa_String_StrLen(&szPort) > 0)
	{
		OpcUa_String_StrnCat(&m_szBinding, &szPort,
			OpcUa_String_StrLen(&szPort));
	}
	// Add the /
	OpcUa_String_StrnCat(&m_szBinding,
		pszSeparatorUrl1,
		OpcUa_String_StrLen(pszSeparatorUrl1));
	// will now update the TransportProfileUri
	// Choisir parmi 
	//OpcUa_TransportProfile_UaTcp          "http://opcfoundation.org/UA-Profile/Transport/uatcp-uasc-uabinary"
	//OpcUa_TransportProfile_SoapHttpBinary "http://opcfoundation.org/UA-Profile/Transport/http-uasc-uabinary"
	//OpcUa_TransportProfile_SoapHttpsUaXml "http://opcfoundation.org/UA-Profile/Transport/soaphttps-uaxml"
	//OpcUa_TransportProfile_HttpsBinary    "http://opcfoundation.org/UA-Profile/Transport/https-uabinary"
	if (OpcUa_StrCmpA(OpcUa_String_GetRawString(&m_Protocol), "HTTPS") == 0)
	{
		if (GetEncoding() == OpcUa_Endpoint_SerializerType_Binary)
			OpcUa_String_AttachCopy(&m_TransportProfileUri, OpcUa_TransportProfile_HttpsBinary);
		else
		{
			if (GetEncoding() == OpcUa_Endpoint_SerializerType_Xml)
				OpcUa_String_AttachCopy(&m_TransportProfileUri, OpcUa_TransportProfile_HttpsSoapUaXml);
			else
				OpcUa_String_Clear(&m_TransportProfileUri);
		}
	}
	else
	{
		if (OpcUa_StrCmpA(OpcUa_String_GetRawString(&m_Protocol), "TCP") == 0)
		{
			if (GetEncoding() == OpcUa_Endpoint_SerializerType_Binary)
				OpcUa_String_AttachCopy(&m_TransportProfileUri, OpcUa_TransportProfile_UaTcp);
			else
			{
				OpcUa_String_Clear(&m_TransportProfileUri);
			}
		}
	}
	// release ressources
	OpcUa_String_Clear(pszUaTcp);
	OpcUa_String_Clear(pszUaHttps);
	OpcUa_String_Clear(pszUaHttp);
	OpcUa_String_Clear(pszSeparatorUrl);
	OpcUa_String_Clear(pszSeparatorUrl1);
	OpcUa_String_Clear(pszPortSeparator);
	OpcUa_Free(pszUaTcp);
	OpcUa_Free(pszUaHttps);
	OpcUa_Free(pszUaHttp);
	OpcUa_Free(pszSeparatorUrl);
	OpcUa_Free(pszSeparatorUrl1);
	OpcUa_Free(pszPortSeparator);
}
OpcUa_String CUABinding::GetProtocol()
{
	return m_Protocol;
}
void CUABinding::SetProtocol(OpcUa_String szVal)
{
	m_Protocol = szVal;
	UpdateBinding();
}
OpcUa_String CUABinding::GetPort()
{
	return m_Port;
}
void CUABinding::SetPort(OpcUa_String szVal)
{
	OpcUa_String_CopyTo(&szVal, &m_Port);
	UpdateBinding();
}
OpcUa_Endpoint_SerializerType CUABinding::GetEncoding()
{
	return m_Encoding;
}
void CUABinding::SetEncoding(OpcUa_Endpoint_SerializerType eVal)
{
	m_Encoding = eVal;
	UpdateBinding();
}