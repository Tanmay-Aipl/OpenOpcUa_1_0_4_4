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
#include "UAVariable.h"
#ifdef _GNUC_
#include <dlfcn.h>
#endif
#include "VpiFuncCaller.h"
#include "VpiTag.h"
#include "VpiWriteObject.h"
#include "VpiDevice.h"
#include "VPIScheduler.h"
#include "UAReferenceType.h"
#include "UAObjectType.h"
#include "Field.h"
#include "Definition.h"
#include "UADataType.h"
#include "UAVariableType.h"
#include "UAMethod.h"
#include "UAView.h"
#include "UAObject.h"
#include "Alias.h"
#include "SimulatedNode.h"
#include "SimulatedGroup.h"
#include "BuildInfo.h"
#include "ServerStatus.h"
#include "VpiWriteObject.h"
#include "VPIScheduler.h"
#include "NamespaceUri.h"
#include "StatusCodeException.h"
#include "ContinuationPoint.h"
#include "SessionSecurityDiagnosticsDataType.h"
#include "SessionDiagnosticsDataType.h"
#include "luainc.h"
#include "LuaVirtualMachine.h"
#include "LuaScript.h"
#include "OpenOpcUaScript.h"
using namespace UAScript;

#include "UAMonitoredItemNotification.h"
#include "UADataChangeNotification.h"
#include "EventDefinition.h"
#include "EventsEngine.h"
#include "UAInformationModel.h"
using namespace UAEvents;
#include "MonitoredItemServer.h"
#include "UAEventNotificationList.h"
#include "QueuedPublishRequest.h"
#include "UAStatusChangeNotification.h"
//#include "MonitoredItemServer.h"
#include "SubscriptionServer.h"
#include "QueuedCallRequest.h"
#include "QueuedReadRequest.h"
#include "QueuedHistoryReadRequest.h"
#include "QueuedQueryFirstRequest.h"
#include "QueuedQueryNextRequest.h"
#include "SessionServer.h"
#include "UABinding.h"
#include "SecureChannel.h"
#include "VfiDataValue.h"
#include "UAHistorianVariable.h"
#include "HaEngine.h"
using namespace UAHistoricalAccess;
#include "ServerApplication.h"
using namespace OpenOpcUa;
using namespace UACoreServer;
using namespace UAAddressSpace;
using namespace UASubSystem;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CVpiTag::CVpiTag(CVpiDevice* pDevice, CUAVariable* pUAVariable, OpcUa_String szAddress) :m_pDevice(pDevice), m_pUAVariable(pUAVariable)
{
	m_pInstrumentRange = OpcUa_Null;
	m_pEURange = OpcUa_Null;
	OpcUa_String szName = pUAVariable->GetBrowseName()->Name;
	OpcUa_String_Initialize(&m_szName);
	SetName(szName);
	OpcUa_String szDescription = pUAVariable->GetDescription().Text;
	OpcUa_String_Initialize(&m_szDescription);
	SetDescription(szDescription);
	OpcUa_NodeId dataType = pUAVariable->GetDataType();
	SetDataType(dataType);
	SetAccessRight(pUAVariable->GetAccessLevel());
	OpcUa_NodeId_Initialize(&m_NodeId);
	OpcUa_String_Initialize(&m_szAddress);
	SetAddress(szAddress);
}
CVpiTag::CVpiTag(const char **atts, CVpiDevice* pDevice) :m_pDevice(pDevice)
{
	m_pInstrumentRange = OpcUa_Null;
	m_pEURange = OpcUa_Null;
	OpcUa_String_Initialize(&m_szName);
	OpcUa_String_Initialize(&m_szDescription);
	OpcUa_String_Initialize(&m_szAddress);
	OpcUa_NodeId_Initialize(&m_NodeId);

	m_bModified = OpcUa_False;
	OpcUa_NodeId_Initialize(&m_nDataType);
	m_nDataType.IdentifierType	= OpcUa_IdentifierType_Numeric;
	m_nDataType.Identifier.Numeric= 11;// OpcUaType_Double;

	OpcUa_NodeId_Initialize(&m_NodeId);
	m_uiAccessRight		= 1; // Input


	m_iNbElement = 0;

	m_pUAVariable=NULL;
	OpcUa_Int16 ii=0;
	while (atts[ii])
	{
		if (OpcUa_StrCmpA(atts[ii],"Id")==0)
		{
			//int iSize=strlen(atts[ii+1]);
			//OpcUa_String aString;
			//OpcUa_String_Initialize(&aString);
			//OpcUa_String_AttachToString((OpcUa_StringA)atts[ii+1],iSize,iSize,true,true,&aString);
			const char* szNodeId = atts[ii + 1];
			OpcUa_NodeId aNodeId;
			OpcUa_NodeId_Initialize(&aNodeId);
			// parsing du NodeId
			if (ParseNodeId((char*)szNodeId, &aNodeId) == OpcUa_Good)
			{
				SetNodeId(aNodeId);
				// recherche de l'UAVariable associée
				CUAInformationModel* pInformationModel=CServerApplication::m_pTheAddressSpace;
				CUAVariable* pUAVariable=OpcUa_Null;
				if (pInformationModel->GetNodeIdFromVariableList(aNodeId, &pUAVariable) == OpcUa_Good)
					SetUAVariable(pUAVariable);
				else
				{
					char* szNodeId = OpcUa_Null;
					Utils::NodeId2String(&aNodeId, &szNodeId);
					if (szNodeId)
					{
						// NodeId
						OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Warning discordance detected on %s between nodeset and SubSystem files.\n", szNodeId);
						free(szNodeId);
					}
				}
			}
			else
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"CVpiTag cannot ParseNodeId. Please check your configuration file\n");
			OpcUa_NodeId_Clear(&aNodeId);
		}
		if (OpcUa_StrCmpA(atts[ii],"Name")==0)
		{
			OpcUa_String szName;
			OpcUa_String_Initialize(&szName);
			OpcUa_String_AttachCopy(&szName, atts[ii+1]);
			SetName(szName);
			OpcUa_String_Clear(&szName);
		}
		if (OpcUa_StrCmpA(atts[ii],"Description")==0)
		{
			OpcUa_String szDescription;
			OpcUa_String_Initialize(&szDescription);
			OpcUa_String_AttachCopy(&szDescription, atts[ii+1]);
			SetDescription(szDescription);
			OpcUa_String_Clear(&szDescription);
		}
		if (OpcUa_StrCmpA(atts[ii],"Dimension")==0)
		{
			m_wDimension=(OpcUa_UInt16)atoi(atts[ii+1]);
		}
		if (OpcUa_StrCmpA(atts[ii],"NbElement")==0)
		{
			m_iNbElement=(OpcUa_UInt16)atoi(atts[ii+1]);
		}
		if (OpcUa_StrCmpA(atts[ii],"Type")==0)
		{
			OpcUa_NodeId aNodeId; // contient le type du signal
			OpcUa_NodeId_Initialize(&aNodeId);
			aNodeId.IdentifierType=OpcUa_IdentifierType_Numeric;
			OpcUa_Boolean bOk=OpcUa_False;
			if (OpcUa_StrCmpA(atts[ii+1],"Boolean")==0)
			{
				aNodeId.Identifier.Numeric=OpcUaType_Boolean;
				bOk=OpcUa_True;
			}
			if (OpcUa_StrCmpA(atts[ii+1],"SByte")==0)
			{
				aNodeId.Identifier.Numeric=OpcUaType_SByte;
				bOk=OpcUa_True;
			}
			if (OpcUa_StrCmpA(atts[ii+1],"Byte")==0)
			{
				aNodeId.Identifier.Numeric=OpcUaType_Byte;
				bOk=OpcUa_True;
			}
			if (OpcUa_StrCmpA(atts[ii+1],"Int16")==0)
			{
				aNodeId.Identifier.Numeric=OpcUaType_Int16;
				bOk=OpcUa_True;
			}
			if (OpcUa_StrCmpA(atts[ii+1],"UInt16")==0)
			{
				aNodeId.Identifier.Numeric=OpcUaType_UInt16;
				bOk=OpcUa_True;
			}
			if (OpcUa_StrCmpA(atts[ii+1],"Int32")==0)
			{
				aNodeId.Identifier.Numeric=OpcUaType_Int32;
				bOk=OpcUa_True;
			}
			if (OpcUa_StrCmpA(atts[ii+1],"UInt32")==0)
			{
				aNodeId.Identifier.Numeric=OpcUaType_UInt32;
				bOk=OpcUa_True;
			}
			if (OpcUa_StrCmpA(atts[ii+1],"Int64")==0)
			{
				aNodeId.Identifier.Numeric=OpcUaType_Int64;
				bOk=OpcUa_True;
			}
			if (OpcUa_StrCmpA(atts[ii+1],"UInt64")==0)
			{
				aNodeId.Identifier.Numeric=OpcUaType_UInt64;
				bOk=OpcUa_True;
			}
			if (OpcUa_StrCmpA(atts[ii+1],"Float")==0)
			{
				aNodeId.Identifier.Numeric=OpcUaType_Float;
				bOk=OpcUa_True;
			}
			if (OpcUa_StrCmpA(atts[ii+1],"Double")==0)
			{
				aNodeId.Identifier.Numeric=OpcUaType_Double;
				bOk=OpcUa_True;
			}
			if (OpcUa_StrCmpA(atts[ii+1],"DateTime")==0)
			{
				aNodeId.Identifier.Numeric=OpcUaType_DateTime;
				bOk=OpcUa_True;
			}
			if (OpcUa_StrCmpA(atts[ii+1],"String")==0)
			{
				aNodeId.Identifier.Numeric=OpcUaType_String;
				bOk=OpcUa_True;
			}
			if (bOk==(OpcUa_Boolean)OpcUa_True)
				SetDataType(aNodeId);
		}
		if (OpcUa_StrCmpA(atts[ii],"AccessRight")==0)
		{
			if (OpcUa_StrCmpA(atts[ii+1],"Input")==0)
				SetAccessRight(1);
			if (OpcUa_StrCmpA(atts[ii+1],"Output")==0)
				SetAccessRight(2);
			if (OpcUa_StrCmpA(atts[ii+1],"Input_Output")==0)
				SetAccessRight(3);
		}
		if (OpcUa_StrCmpA(atts[ii],"Address")==0)
		{
			OpcUa_String szAddress;
			OpcUa_String_Initialize(&szAddress);
			OpcUa_String_AttachCopy(&szAddress, atts[ii+1]);
			SetAddress(szAddress);
			OpcUa_String_Clear(&szAddress);
		}
		ii+=2;
	}
}

CVpiTag::~CVpiTag()
{
	if (m_pInstrumentRange)
		delete m_pInstrumentRange;
	if (m_pEURange)
		delete m_pEURange;
	OpcUa_NodeId_Clear(&m_nDataType);
	OpcUa_String_Clear(&m_szName);
	OpcUa_String_Clear(&m_szDescription);
	OpcUa_NodeId_Clear(&m_NodeId);
	OpcUa_String_Clear(&m_szAddress);
}
CVpiDevice* CVpiTag::GetDevice() 
{ 
	return m_pDevice; 
}
void CVpiTag::SetModified (OpcUa_Boolean bModified)
{
	m_bModified = bModified;
}

void CVpiTag::SyncUAVariable()
{
	// recherche de l'UAVariable associée
	CUAInformationModel* pInformationModel=CServerApplication::m_pTheAddressSpace;
	CUAVariable* pUAVariable=OpcUa_Null;
	OpcUa_NodeId aNodeId=GetNodeId();
	OpcUa_StatusCode hr = pInformationModel->GetNodeIdFromVariableList(aNodeId, &pUAVariable);
	if (hr==OpcUa_Good)
	{
		SetUAVariable(pUAVariable);
	}
}

void CVpiTag::SetName(OpcUa_String szName) 
{ 
	OpcUa_String_CopyTo(&szName, &m_szName);  
}
OpcUa_String CVpiTag::GetName()
{ 
	return m_szName; 
}


void CVpiTag::SetDescription(OpcUa_String szDesc) 
{ 
	OpcUa_String_CopyTo(&szDesc, &m_szDescription);
}

void CVpiTag::SetAddress(OpcUa_String szAddr) 
{ 
	OpcUa_String_CopyTo(&szAddr, &m_szAddress);
}
OpcUa_String CVpiTag::GetAddress()
{ 
	return m_szAddress; 
}

void CVpiTag::SetNbElement(OpcUa_Int32 wNbElement) 
{ 
	m_iNbElement = wNbElement; 
}
OpcUa_Int32 CVpiTag::GetNbElement() 
{ 
	return m_iNbElement; 
}
void CVpiTag::SetDataType(OpcUa_NodeId nDataType)
{
	m_nDataType = nDataType;
	if (m_pUAVariable)
	{
		if ((m_pUAVariable->GetBuiltInType() == OpcUaType_String) && (GetNbElement() > 0) && (m_nDataType.Identifier.Numeric == OpcUaType_Byte))
			; // nothing to do this is a special case where a string is in the serveur AddressSpace and Array of Byte in the Vpi
		else
			m_pUAVariable->SetDataType(nDataType);
	}
	else
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critical Error>Your NodeSet file not fit your SubSystem file\n");
}
OpcUa_NodeId   CVpiTag::GetDataType()
{ 
	return m_nDataType; 
}
void CVpiTag::SetAccessRight(OpcUa_Byte uiVal)
{ 
	m_uiAccessRight = uiVal; 
}
OpcUa_Byte CVpiTag::GetAccessRight()
{ 
	return m_uiAccessRight; 
}
CUAVariable* CVpiTag::GetUAVariable() 
{ 
	return m_pUAVariable; 
}
void CVpiTag::SetUAVariable(CUAVariable* pVariable)
{
	m_pUAVariable = pVariable;
}

OpcUa_NodeId  CVpiTag::GetNodeId()
{
	return m_NodeId;
}
void  CVpiTag::SetNodeId(OpcUa_NodeId aNodeId)
{
	OpcUa_NodeId_CopyTo(&aNodeId, &m_NodeId);
}