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
using namespace UAAddressSpace;
#include "UAVariable.h"
#ifdef _GNUC_
#include <dlfcn.h>
#endif
#include "VpiFuncCaller.h"
#include "VpiTag.h"
#include "VpiWriteObject.h"
#include "VpiDevice.h"
#include "Utils.h"
#include "Field.h"
#include "UAReferenceType.h"
#include "UAObjectType.h"
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
#include "VPIScheduler.h"
#include "NamespaceUri.h"
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
using namespace UAEvents;
#include "UAInformationModel.h"
#include "MonitoredItemServer.h"
#include "UAEventNotificationList.h"
#include "QueuedPublishRequest.h"
#include "UAStatusChangeNotification.h"
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

CField::CField(void)
{
	m_pDefinition=OpcUa_Null;
	m_iFieldSize=0;
	m_ValueRank = 0;
	OpcUa_String_Initialize(&m_Name);
	OpcUa_String_Initialize(&m_SymbolicName);
	OpcUa_NodeId_Initialize(&m_DataType);
	OpcUa_LocalizedText_Initialize(&m_Description);
}
CField::CField(const char **atts)
{
	m_pDefinition=OpcUa_Null;
	m_iFieldSize=0;
	m_ValueRank = 0;
	OpcUa_String_Initialize(&m_Name);
	OpcUa_String_Initialize(&m_SymbolicName);
	OpcUa_NodeId_Initialize(&m_DataType);
	OpcUa_LocalizedText_Initialize(&m_Description);
	int ii=0;
	while (atts[ii])
	{
		if (OpcUa_StrCmpA(atts[ii],"Name")==0)
		{
			//OpcUa_String* aString=(OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
			//OpcUa_String_Initialize(aString);
			//int iSize=strlen(atts[ii+1])+1;
			if (strlen(atts[ii+1])>0)
			{
				OpcUa_String_Initialize(&m_Name);
				OpcUa_String_AttachCopy(&m_Name,atts[ii+1]);
			}
			else
				OpcUa_String_AttachCopy(&m_Name,"");
			//SetName(aString);
		}
		if (OpcUa_StrCmpA(atts[ii],"SymbolicName")==0)
		{
			//OpcUa_String* aString=(OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
			//OpcUa_String_Initialize(aString);
			//int iSize=strlen(atts[ii+1])+1;
			if (strlen(atts[ii+1])>0)
			{
				OpcUa_String_Initialize(&m_SymbolicName);
				OpcUa_String_AttachCopy(&m_SymbolicName,(OpcUa_StringA)atts[ii+1]);
			}
			else
				OpcUa_String_AttachCopy(&m_SymbolicName,"");
				
			//SetSymbolicName(aString);
		}
		if (OpcUa_StrCmpA(atts[ii],"DataType")==0)
		{
			// NodeId
			OpcUa_NodeId aNodeId;
			OpcUa_NodeId_Initialize(&aNodeId);
			
			if (ParseNodeId(atts[ii+1],&aNodeId)==OpcUa_Good)
			{
				SetDataType(aNodeId);
			}
			else
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "XML file corrupted. Impossible to parse this DataType:%s\n", atts[ii + 1]);
			OpcUa_NodeId_Clear(&aNodeId);
		}
		if (OpcUa_StrCmpA(atts[ii],"ValueRank")==0)
		{
			OpcUa_Int32 iVal=0;
			int iRes=sscanf_s(atts[ii+1],"%i",(int*)&iVal);
			if (!iRes)
			{
				// erreur de conversion
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "an incorrect ValueRank was used\n");
			}
			else			
				SetValueRank(iVal);
		}
		ii=ii+2;
	}
}
CField::~CField(void)
{
	OpcUa_LocalizedText_Clear(&m_Description);
	OpcUa_String_Clear(&m_Name);
	OpcUa_NodeId_Clear(&m_DataType);
	OpcUa_String_Clear(&m_SymbolicName);
}
// On détermine la taille de ce champ
// ici seul les types elementaire seront pris en compte
OpcUa_StatusCode CField::UpdateFieldSize()
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_Byte BuiltInType=0;
	uStatus=FindBuiltinType(m_DataType,&BuiltInType);
	if (uStatus==OpcUa_Good)
	{
		switch (BuiltInType)
		{
		case OpcUaType_Boolean:
			m_iFieldSize=2;
			break;
		case OpcUaType_SByte:
			break;
		case OpcUaType_Byte:
			break;
		case OpcUaType_Int16:
			m_iFieldSize=2;
			break;
		case OpcUaType_UInt16:
			m_iFieldSize=2;
			break;
		case OpcUaType_Int32:
			m_iFieldSize=4;
			break;
		case OpcUaType_UInt32:
			m_iFieldSize=4;
			break;
		case OpcUaType_Int64:
			m_iFieldSize=8;
			break;
		case OpcUaType_UInt64:
			m_iFieldSize=8;
			break;
		case OpcUaType_Float:
			m_iFieldSize=4;
			break;
		case OpcUaType_Double:
			m_iFieldSize=8;
			break;
		case OpcUaType_String:
			m_iFieldSize=sizeof(OpcUa_String);
			break;
		case OpcUaType_DateTime:
			m_iFieldSize=8;
			break;
		case OpcUaType_Guid:
			break;
		case OpcUaType_ByteString:
			m_iFieldSize=sizeof(OpcUa_ByteString);
			break;
		case OpcUaType_NodeId:
			m_iFieldSize=sizeof(OpcUa_NodeId);
			break;
		case OpcUaType_StatusCode:
			m_iFieldSize=sizeof(OpcUa_StatusCode);
			break;
		case OpcUaType_DiagnosticInfo:
			m_iFieldSize=sizeof(OpcUa_DiagnosticInfo);
			break;
		case OpcUaType_QualifiedName:
			m_iFieldSize=sizeof(OpcUa_QualifiedName);
			break;
		case OpcUaType_LocalizedText:
			m_iFieldSize=sizeof(OpcUa_LocalizedText);
			break;
		case  OpcUaType_XmlElement:
		case  OpcUaType_ExpandedNodeId:
		case  OpcUaType_ExtensionObject:
		case  OpcUaType_DataValue:
			uStatus=OpcUa_BadNotSupported;
			break;
		default:
			uStatus=OpcUa_BadNotSupported;
			break;
		}
	}
	return uStatus;
}
// here will compute the size of the field based on the content of the CUAVariable
OpcUa_StatusCode CField::GetInstanceFieldSize(CUAVariable* pUAVariable)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;

	OpcUa_Byte BuiltInType=0;
	uStatus=FindBuiltinType(m_DataType,&BuiltInType);
	if (uStatus==OpcUa_Good)
	{
		switch (BuiltInType)
		{
		case OpcUaType_Boolean:
			m_iFieldSize=2;
			break;
		case OpcUaType_SByte:
			m_iFieldSize=1;
			break;
		case OpcUaType_Byte:
			m_iFieldSize=1;
			break;
		case OpcUaType_Int16:
			m_iFieldSize=2;
			break;
		case OpcUaType_UInt16:
			m_iFieldSize=2;
			break;
		case OpcUaType_Int32:
			m_iFieldSize=4;
			break;
		case OpcUaType_UInt32:
			m_iFieldSize=4;
			break;
		case OpcUaType_Int64:
			m_iFieldSize=8;
			break;
		case OpcUaType_UInt64:
			m_iFieldSize=8;
			break;
		case OpcUaType_Float:
			m_iFieldSize=4;
			break;
		case OpcUaType_Double:
			m_iFieldSize=8;
			break;
		case OpcUaType_String:
			{
				CDataValue* pValue=pUAVariable->GetValue();
				if (pValue)
				{
					OpcUa_Variant aVariant=pValue->GetValue();
					m_iFieldSize=OpcUa_String_StrLen(&(aVariant.Value.String));
				}
			}
			break;
		case OpcUaType_DateTime:
			m_iFieldSize=8;
			break;
		case OpcUaType_Guid:
			break;
		case OpcUaType_ByteString:
			m_iFieldSize=pUAVariable->GetValue()->GetValue().Value.ByteString.Length;
			break;
		case OpcUaType_NodeId:
			break;
		case OpcUaType_StatusCode:
			break;
		case OpcUaType_DiagnosticInfo:
			break;
		case OpcUaType_QualifiedName:
			m_iFieldSize=OpcUa_String_StrLen(&(pUAVariable->GetValue()->GetValue().Value.QualifiedName->Name));
			break;
		case OpcUaType_LocalizedText:
			m_iFieldSize=OpcUa_String_StrLen(&(pUAVariable->GetValue()->GetValue().Value.LocalizedText->Locale));
			m_iFieldSize+=OpcUa_String_StrLen(&(pUAVariable->GetValue()->GetValue().Value.LocalizedText->Text));
			break;
		case  OpcUaType_XmlElement:
		case  OpcUaType_ExpandedNodeId:
		case  OpcUaType_ExtensionObject:
			{
				OpcUa_NodeId aNodeId;
				CUADataType* pUADataType=OpcUa_Null;
				CUAInformationModel* pInformationModel=CServerApplication::m_pTheAddressSpace;

				OpcUa_NodeId_Initialize(&aNodeId);
				aNodeId.Identifier.Numeric=pUAVariable->GetValue()->GetValue().Value.ExtensionObject->Body.EncodeableObject.Type->TypeId;
				uStatus=pInformationModel->GetNodeIdFromDataTypeList(aNodeId,&pUADataType);
				if (uStatus==OpcUa_Good)
				{
					OpcUa_Int32 iSize=-1;
					CDefinition* pDefinition=pUADataType->GetDefinition();
					uStatus=pDefinition->GetInstanceSize(&iSize);
					if (uStatus==OpcUa_Good)
						m_iFieldSize=iSize;
				}
			}
			break;
		case  OpcUaType_DataValue:
			uStatus=OpcUa_BadNotSupported;
			break;
		default:
			uStatus=OpcUa_BadNotSupported;
			break;
		}
	}
	return uStatus;
}