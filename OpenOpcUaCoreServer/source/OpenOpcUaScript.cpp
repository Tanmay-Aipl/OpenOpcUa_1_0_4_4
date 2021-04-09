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
#include <stdexcept>
#endif
#include "VpiFuncCaller.h"
#include "VpiTag.h"
#include "VpiWriteObject.h"
#include "VpiDevice.h"
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
#include "VPIScheduler.h"
#include "NamespaceUri.h"
#include "StatusCodeException.h"
#include "ContinuationPoint.h"
#include "SessionSecurityDiagnosticsDataType.h"
#include "SessionDiagnosticsDataType.h"

#include "luainc.h"
#include "LuaDebugger.h"
#include "LuaVirtualMachine.h"
#include "LuaScript.h"
#include "OpenOpcUaScript.h"
using namespace OpenOpcUa;
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
COpenOpcUaScript::COpenOpcUaScript(CLuaVirtualMachine* vm) : CLuaScript(vm)
{
	m_iMethodBase = RegisterFunction("AddUAObject"); // 0
	RegisterFunction("GetUAValue"); // 1
	RegisterFunction("SetUAValue"); // 2
	RegisterFunction("AddUAVariable"); // 3
	RegisterFunction("AddUAReference"); // 4
	RegisterFunction("AddTag"); // 5
	OpcUa_Variant_Initialize(&m_variantFromLuaFunction);
}
COpenOpcUaScript::~COpenOpcUaScript()
{
	OpcUa_Variant_Clear(&m_variantFromLuaFunction);
}

///-------------------------------------------------------------------------------------------------
/// <summary>	This method is a callback. 
/// 			It receives the returns from LUA function invoke by the client. </summary>
///
/// <remarks>	Michel, 02/02/2016. </remarks>
///
/// <param name="vm">	  	[in,out] If non-null, the virtual memory. </param>
/// <param name="strFunc">	The LUA function name. 
/// 						This function is in the OpenOpcUaCoreServerScript.lua </param>
///-------------------------------------------------------------------------------------------------

void COpenOpcUaScript::HandleReturns(CLuaVirtualMachine* vm, const char *strFunc)
{
	if (strcmp(strFunc, m_strFunctionName) == 0) // m_strFunctionName contains the name of the method called by this OPC client
	{
	//	// frames returns an answer of the stack
		lua_State *state = (lua_State *)vm->GetLuaState();
		int itop = lua_gettop(state);
		int iType = lua_type(state, -1);
		const char *s = lua_typename(state, iType);
		if (strcmp(s, "number") == 0)
		{
			m_variantFromLuaFunction.Datatype = OpcUaType_Double;
			m_variantFromLuaFunction.Value.Double = lua_tonumber(state, -1);
		}
	}
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Script calling. </summary>
///
/// <remarks>	Michel, 02/02/2016. </remarks>
///
/// <param name="vm">			  	[in,out] If non-null, the virtual memory. </param>
/// <param name="iFunctionNumber">	Zero-based index of the function number. </param>
///
/// <returns>	An OpcUa_Int32. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_Int32 COpenOpcUaScript::ScriptCalling(CLuaVirtualMachine* vm, int iFunctionNumber)
{
	OpcUa_Int32 iResult= 0;
	switch (iFunctionNumber - m_iMethodBase)
	{
	case 0:
		iResult = AddUAObject(vm);
		break;
	case 1:
		iResult = GetUAValue(vm);
		break;
	case 2:
		iResult = SetUAValue(vm);
		break;
	case 3:
		iResult = AddUAVariable(vm);
		break;
	case 4:
		iResult = AddUAReference(vm);
		break;
	case 5:
		iResult = AddTag(vm);
		break;
	default:
		break;
	}
	return iResult;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Adds a new UAobject. 
/// 			Syntax from LUA is 
/// 			AddUAObject(char* aNodeId, char* szBrowseName, char* typeDefinition)</summary>
///
/// <remarks>	Michel, 03/02/2016. </remarks>
///
/// <param name="vm">	[in,out] If non-null, the virtual memory. </param>
///
/// <returns>	An OpcUa_Int32. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_Int32 COpenOpcUaScript::AddUAObject(CLuaVirtualMachine* vm)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_Int32 iResult = 1;
	OpcUa_String strBrowseName;

	OpcUa_NodeId typeDefinition;
	OpcUa_NodeId nodeId;

	OpcUa_NodeId_Initialize(&typeDefinition);
	OpcUa_String_Initialize(&strBrowseName);
	OpcUa_NodeId_Initialize(&nodeId);
	CUAInformationModel* pInformationModel = CServerApplication::m_pTheAddressSpace;
	lua_State *state = (lua_State *)vm->GetLuaState();
	int itop = lua_gettop(state);
	if (itop == 3)
	{
		CUAObject* pUAObject = OpcUa_Null;
		CUAObjectType* pUAObjectType = OpcUa_Null;
		// Extract nodeId
		int iType = lua_type(state, 1);
		const char *s = lua_typename(state, iType);
		if (strcmp(s, "string") == 0)
		{
			OpcUa_String szNodeId;
			OpcUa_String_Initialize(&szNodeId);
			const char* szVal = lua_tostring(state, 1);
			OpcUa_String_AttachCopy(&szNodeId, szVal);
			Utils::String2NodeId(szNodeId, &nodeId);

			uStatus = pInformationModel->GetNodeIdFromObjectList(nodeId, &pUAObject);
			if (uStatus == OpcUa_Good) // must fail
				iResult = 2;
			else
			{
				iType = lua_type(state, 2);
				s = lua_typename(state, iType);
				if (strcmp(s, "string") == 0)
				{
					const char* szBrowseName = lua_tostring(state, 2);
					OpcUa_String_AttachCopy(&strBrowseName, szBrowseName);
					// typeDefinition
					iType = lua_type(state, 3);
					s = lua_typename(state, iType);
					if (strcmp(s, "string") == 0)
					{
						OpcUa_String_Initialize(&szNodeId);
						const char* sztypeDefinition = lua_tostring(state, 3);
						OpcUa_String_AttachCopy(&szNodeId, sztypeDefinition);
						Utils::String2NodeId(szNodeId, &typeDefinition);

						uStatus = pInformationModel->GetNodeIdFromObjectTypeList(typeDefinition, &pUAObjectType);
						if (uStatus != OpcUa_Good)
							iResult = 6;
					}
					else
						iResult = 5;
				}
				else
					iResult = 4;
			}
		}
		else
			iResult = 3;
		// create the new UAObject
		if (iResult == 1)
			uStatus = pInformationModel->AddUAObject(nodeId, strBrowseName, typeDefinition);
	}
	return iResult;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Adds a new UAVariable. 
/// 			Syntax is
/// 			AddUAVariable(char* aNodeId, char* szBrowseName, char* typeDefinition, int builtinType, int arraySize)
/// 			</summary>
///
/// <remarks>	Michel, 03/02/2016. </remarks>
///
/// <param name="vm">	[in,out] If non-null, the virtual memory. </param>
///
/// <returns>	An OpcUa_Int32. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_Int32 COpenOpcUaScript::AddUAVariable(CLuaVirtualMachine* vm)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_Int32 iResult = 1;
	OpcUa_String strBrowseName;
	int iDatatype = 0;
	int iArraysize = 0;
	int iAccessLevel = 3; // default read-write
	OpcUa_NodeId typeDefinition;
	OpcUa_NodeId nodeId;
	OpcUa_Boolean bHistorizing = OpcUa_False;

	OpcUa_String_Initialize(&strBrowseName);
	OpcUa_NodeId_Initialize(&typeDefinition);
	OpcUa_NodeId_Initialize(&nodeId);
	CUAInformationModel* pInformationModel = CServerApplication::m_pTheAddressSpace;
	lua_State *state = (lua_State *)vm->GetLuaState();
	int itop = lua_gettop(state); // contains the number of parameters
	if (itop == 6)
	{
		CUAVariable* pUAVariable = OpcUa_Null;
		CUAVariableType* pUAVariableType = OpcUa_Null;
		// Extract nodeId
		int iType = lua_type(state, 1);
		const char *s = lua_typename(state, iType);
		if (strcmp(s, "string") == 0)
		{
			OpcUa_String szNodeId;
			OpcUa_String_Initialize(&szNodeId);
			const char* szVal = lua_tostring(state, 1); // param 1 NodeId. This is a string containing a nodeId
			OpcUa_String_AttachCopy(&szNodeId, szVal);
			Utils::String2NodeId(szNodeId, &nodeId);

			uStatus = pInformationModel->GetNodeIdFromVariableList(nodeId, &pUAVariable);
			if (uStatus == OpcUa_Good) // must fail
				iResult = 2;
			else
			{
				iType = lua_type(state, 2); // param 2 szBrowseName
				s = lua_typename(state, iType);
				if (strcmp(s, "string") == 0)
				{
					const char* szBrowseName = lua_tostring(state, 2);
					OpcUa_String_AttachCopy(&strBrowseName, szBrowseName);
					// typeDefinition
					iType = lua_type(state, 3); // param 3 typeDefiniton. This is a string containing a nodeId
					s = lua_typename(state, iType);
					if (strcmp(s, "string") == 0)
					{
						OpcUa_NodeId_Initialize(&typeDefinition);
						OpcUa_String_Initialize(&szNodeId);
						const char* sztypeDefinition = lua_tostring(state, 3);
						OpcUa_String_AttachCopy(&szNodeId, sztypeDefinition);
						Utils::String2NodeId(szNodeId, &typeDefinition);

						uStatus = pInformationModel->GetNodeIdFromVariableTypeList(typeDefinition, &pUAVariableType);
						if (uStatus != OpcUa_Good)
							iResult = 6;
						else
						{
							// extract dataType
							iDatatype = (int)lua_tonumber(state, 4); // param 4 builtInType this is a numerical value according to UA builtIn type
																	 // verify that we are in the scope of the supported datatype (1..15)
							if ((iDatatype > 0) && (iDatatype < 16))
							{
								iArraysize = (int)lua_tonumber(state, 5); // Param 5 arraySize. This is a numerical value
								// Access Level 
								iAccessLevel = (int)lua_tonumber(state, 6);// Param 6 accessLevel. This is a numerical value OpcUa_AccessLevels_CurrentReadOrWrite
																			// Valid value are 1 read, 2 write, 3 read-write, 4 History Read, 5 History Write, 12 History Read-Write
																			// those value can be ored
							}
							else
								iResult = 7;
						}
					}
					else
						iResult = 5;
				}
				else
					iResult = 4;
			}
		}
		else
			iResult = 3;
		if (iResult == 1)
		{
			OpcUa_Variant internalValue;
			OpcUa_Variant_Initialize(&internalValue);
			internalValue.Datatype = (OpcUa_Byte)iDatatype;
			if (iArraysize > 0)
				internalValue.ArrayType = OpcUa_VariantArrayType_Array;
			else
				internalValue.ArrayType = OpcUa_VariantArrayType_Scalar;
			if ((iAccessLevel&OpcUa_AccessLevels_HistoryReadOrWrite) == OpcUa_AccessLevels_HistoryReadOrWrite)
				bHistorizing = OpcUa_True;
			if ((iAccessLevel&OpcUa_AccessLevels_HistoryRead) == OpcUa_AccessLevels_HistoryRead)
				bHistorizing = OpcUa_True;
			if ((iAccessLevel&OpcUa_AccessLevels_HistoryWrite) == OpcUa_AccessLevels_HistoryWrite)
				bHistorizing = OpcUa_True;
			// create the new UAVariable
			if (iResult == 1)
				uStatus = pInformationModel->AddUAVariable(nodeId, strBrowseName, typeDefinition, internalValue, iAccessLevel, bHistorizing);
			OpcUa_Variant_Clear(&internalValue);
		}
	}
	return iResult;
}
OpcUa_Int32 COpenOpcUaScript::AddUAReference(CLuaVirtualMachine* vm)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_Int32 iResult = 1;
	OpcUa_NodeId sourceNodeId;
	OpcUa_NodeId targetNodeid;
	OpcUa_NodeId referenceTypeId;
	
	OpcUa_NodeId_Initialize(&sourceNodeId);
	OpcUa_NodeId_Initialize(&targetNodeid);
	OpcUa_NodeId_Initialize(&referenceTypeId);
	CUAInformationModel* pInformationModel = CServerApplication::m_pTheAddressSpace;
	lua_State *state = (lua_State *)vm->GetLuaState();
	int itop = lua_gettop(state);
	if (itop == 3)
	{
		CUABase* pSourceNode = OpcUa_Null;
		CUABase* pTargetNode = OpcUa_Null;
		CUABase* pReferenceNode = OpcUa_Null;
		// Extract sourceNodeId
		int iType = lua_type(state, 1);
		const char *s = lua_typename(state, iType);
		if (strcmp(s, "string") == 0)
		{
			OpcUa_String szNodeId;
			OpcUa_String_Initialize(&szNodeId);
			const char* szVal = lua_tostring(state, 1);
			OpcUa_String_AttachCopy(&szNodeId, szVal);
			Utils::String2NodeId(szNodeId, &sourceNodeId);

			uStatus = pInformationModel->GetNodeIdFromDictionnary(sourceNodeId, &pSourceNode);
			if (uStatus == OpcUa_Good)
			{
				// Extract targetNodeid
				OpcUa_String_Initialize(&szNodeId);
				const char* szVal = lua_tostring(state, 2);
				OpcUa_String_AttachCopy(&szNodeId, szVal);
				Utils::String2NodeId(szNodeId, &targetNodeid);

				uStatus = pInformationModel->GetNodeIdFromDictionnary(targetNodeid, &pTargetNode);// 
				if (uStatus == OpcUa_Good)
				{
					// Extract referenceTypeId
					OpcUa_String_Initialize(&szNodeId);
					const char* szVal = lua_tostring(state, 3);
					OpcUa_String_AttachCopy(&szNodeId, szVal);
					Utils::String2NodeId(szNodeId, &referenceTypeId);

					uStatus = pInformationModel->GetNodeIdFromDictionnary(referenceTypeId, &pReferenceNode);// 
					if (uStatus != OpcUa_Good)
						iResult = 5;
				}
				else
					iResult = 4;
			}
			else
				iResult = 3;
		}
		else
			iResult = 2;

	}
	if (iResult == 1)
	{
		uStatus = pInformationModel->AddUAReference(sourceNodeId, targetNodeid, referenceTypeId);
		if (uStatus != OpcUa_Good)
			iResult = 7;
	}
	return iResult;
}
///-------------------------------------------------------------------------------------------------
/// <summary>	Set the Value in the server addressSpace. 
/// 			The node to set must be a UAVariable and the value on can be set. 
/// 			The method wil receive 2 parameters from LUA
/// 			The nodeid on index 1, and the value to set on index 2</summary>
///
/// <remarks>	Michel, 02/02/2016. </remarks>
///
/// <param name="vm">	[in,out] If non-null, the virtual memory. </param>
///
/// <returns>	An OpcUa_Int32 : 1 if the method run properly
/// 			2 if the recevied nodeId is incorrect  
/// 			3 if not enough param was sent. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_Int32 COpenOpcUaScript::SetUAValue(CLuaVirtualMachine* vm)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_Int32 iResult= 1;
	CUAInformationModel* pInformationModel = CServerApplication::m_pTheAddressSpace;
	lua_State *state = (lua_State *)vm->GetLuaState();
	int itop = lua_gettop(state);
	if (itop == 2)
	{
		OpcUa_DataValue* pDataValue = OpcUa_Null;
		CUAVariable* pUAVariable = OpcUa_Null;
		// Extract nodeId
		int iType = lua_type(state, 1);
		const char *s = lua_typename(state, iType);
		if (strcmp(s, "string") == 0)
		{
			OpcUa_String szNodeId;
			OpcUa_NodeId nodeId;
			OpcUa_NodeId_Initialize(&nodeId);
			OpcUa_String_Initialize(&szNodeId);
			const char* szVal = lua_tostring(state, 1);
			OpcUa_String_AttachCopy(&szNodeId, szVal);
			Utils::String2NodeId(szNodeId, &nodeId);
			
			uStatus = pInformationModel->GetNodeIdFromVariableList(nodeId, &pUAVariable);
			if (uStatus == OpcUa_Good)
			{
				pDataValue = pUAVariable->GetValue()->GetInternalDataValue();
			}
		}
		// Extract value
		if (pDataValue)
		{
			uStatus = OpcUa_BadInternalError;
			iType = lua_type(state, 2);
			s = lua_typename(state, iType);
			if (strcmp(s, "number") == 0)
			{
				uStatus = OpcUa_Good;
				switch (pDataValue->Value.Datatype)
				{
				case OpcUaType_Boolean:
					pDataValue->Value.Value.Boolean = (OpcUa_Boolean)lua_toboolean(state, 2);
					break;
				case OpcUaType_Byte:
					pDataValue->Value.Value.Byte = (OpcUa_Byte)lua_tonumber(state, 2);
					break;
				case OpcUaType_UInt16:
					pDataValue->Value.Value.UInt16 = (OpcUa_UInt16)lua_tonumber(state, 2);
					break;
				case OpcUaType_UInt32:
					pDataValue->Value.Value.UInt32 = (OpcUa_UInt32)lua_tonumber(state, 2);
					break;
				case OpcUaType_UInt64:
					pDataValue->Value.Value.UInt64 = (OpcUa_UInt64)lua_tonumber(state, 2);
					break;
				case OpcUaType_SByte:
					pDataValue->Value.Value.SByte = (OpcUa_SByte)lua_tonumber(state, 2);
					break;
				case OpcUaType_Int16:
					pDataValue->Value.Value.UInt16 = (OpcUa_Int16)lua_tonumber(state, 2);
					break;
				case OpcUaType_Int32:
					pDataValue->Value.Value.UInt32 = (OpcUa_Int32)lua_tonumber(state, 2);
					break;
				case OpcUaType_Int64:
					pDataValue->Value.Value.UInt64 = (OpcUa_Int64)lua_tonumber(state, 2);
					break;
				case OpcUaType_Float:
					pDataValue->Value.Value.Float = (OpcUa_Float)lua_tonumber(state, 2);
					break;
				case OpcUaType_Double:
				default:
					pDataValue->Value.Value.Double = lua_tonumber(state, 2);
					break;
				}
			}
			else
			{
				if (strcmp(s, "string") == 0)
				{
					if (pDataValue->Value.Datatype == OpcUaType_String)
					{
						uStatus = OpcUa_Good;
						const char* szVal = lua_tostring(state, 2);
						OpcUa_String_AttachCopy(&(pDataValue->Value.Value.String), szVal);
					}
				}
			}

			pDataValue->StatusCode = uStatus;
			pDataValue->ServerTimestamp = OpcUa_DateTime_UtcNow();
			pDataValue->SourceTimestamp = OpcUa_DateTime_UtcNow();
		}
		else
			iResult = 2;

	}
	else
		iResult = 3;
	return iResult;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Gets UA value. </summary>
///
/// <remarks>	Michel, 02/02/2016. </remarks>
///
/// <param name="vm">	[in,out] If non-null, the virtual memory. </param>
///
/// <returns>	The UA value. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_Int32 COpenOpcUaScript::GetUAValue(CLuaVirtualMachine*vm)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_Int32 iResult= 0;
	lua_State *state = (lua_State *)vm->GetLuaState();
	CUAInformationModel* pInformationModel = CServerApplication::m_pTheAddressSpace;
	int itop = lua_gettop(state);
	if (itop == 2)
	{
		OpcUa_Int32 iRequestAttribute=0;
		OpcUa_DataValue* pDataValue = OpcUa_Null;
		CUAVariable* pUAVariable = OpcUa_Null;
		// Extract nodeId
		int iType = lua_type(state, 1);
		const char *s = lua_typename(state, iType);
		if (strcmp(s, "string") == 0)
		{
			OpcUa_String szNodeId;
			OpcUa_NodeId nodeId;
			OpcUa_NodeId_Initialize(&nodeId);
			OpcUa_String_Initialize(&szNodeId);
			const char* szVal = lua_tostring(state, 1);
			OpcUa_String_AttachCopy(&szNodeId, szVal);
			Utils::String2NodeId(szNodeId, &nodeId);

			uStatus = pInformationModel->GetNodeIdFromVariableList(nodeId, &pUAVariable);
			if (uStatus == OpcUa_Good)
			{
				pDataValue = pUAVariable->GetValue()->GetInternalDataValue();
			}
		}
		// Extract Attribute (can be from 1 to 22)
		iType = lua_type(state, 2);
		s = lua_typename(state, iType);
		if (strcmp(s, "number") == 0)
		{
			iRequestAttribute = (OpcUa_Int32)lua_tonumber(state, 2);
		}
		// Extract output parameter (Result)
		
		switch (iRequestAttribute)
		{
		case OpcUa_Attributes_Value:
		{
			if (pDataValue)
			{
				switch (pDataValue->Value.Datatype)
				{
				case OpcUaType_Boolean:
				{
					OpcUa_Int32 iUAResult = pDataValue->Value.Value.Boolean;
					lua_pushboolean(state, iUAResult);
					iResult = 1;
				}
				break;
				case OpcUaType_Byte:
				{
					OpcUa_Int32 iUAResult = pDataValue->Value.Value.Byte;
					lua_pushnumber(state, iUAResult);
					iResult = 1;
				}
				break;
				case OpcUaType_UInt16:
				{
					OpcUa_Int32 iUAResult = pDataValue->Value.Value.UInt16;
					lua_pushnumber(state, iUAResult);
					iResult = 1;
				}
				break;
				case OpcUaType_UInt32:
				{
					OpcUa_UInt32 iUAResult = pDataValue->Value.Value.UInt32;
					lua_pushnumber(state, iUAResult);
					iResult = 1;
				}
				break;
				case OpcUaType_UInt64:
				{
					OpcUa_UInt64 iUAResult = pDataValue->Value.Value.UInt64;
					lua_pushnumber(state, (lua_Number)iUAResult);
					iResult = 1;
				}
				break;
				case OpcUaType_SByte:
				{
					OpcUa_Int32 iUAResult = pDataValue->Value.Value.SByte;
					lua_pushnumber(state, iUAResult);
					iResult = 1;
				}
				break;
				case OpcUaType_Int16:
				{
					OpcUa_Int32 iUAResult = pDataValue->Value.Value.Int16;
					lua_pushnumber(state, iUAResult);
					iResult = 1;
				}
				break;
				case OpcUaType_Int32:
				{
					OpcUa_Int32 iUAResult = pDataValue->Value.Value.Int32;
					lua_pushnumber(state, iUAResult);
					iResult = 1;
				}
				break;
				case OpcUaType_Int64:
				{
					OpcUa_Int64 iUAResult = pDataValue->Value.Value.Int64;
					lua_pushnumber(state, (lua_Number)iUAResult);
					iResult = 1;
				}
				break;
				case OpcUaType_Float:
				{
					OpcUa_Float iUAResult = pDataValue->Value.Value.Float;
					lua_pushnumber(state, iUAResult);
					iResult = 1;
				}
				break;
				case OpcUaType_Double:
				{
					OpcUa_Double dblUAResult = pDataValue->Value.Value.Double;
					lua_pushnumber(state, dblUAResult);
					iResult = 1;
				}
				break;
				case OpcUaType_String:
				{
					OpcUa_CharA* pszResult = OpcUa_String_GetRawString(&(pDataValue->Value.Value.String));
					lua_pushstring(state, pszResult);
					iResult = 1;
				}
				break;
				default:
					lua_pushnil(state);
					break;
				}
			}
		}
		break;
		default:
			break;
		}
	}
	return iResult;	
}
///-------------------------------------------------------------------------------------------------
/// <summary>	Adds a tag. </summary>
///
/// <remarks>	Michel, 03/02/2016. </remarks>
///
/// <param name="vm">	[in,out] If non-null, the virtual memory. </param>
///
/// <returns>	An OpcUa_Int32. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_Int32 OpenOpcUa::UAScript::COpenOpcUaScript::AddTag(CLuaVirtualMachine*vm)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_Int32 iResult = 1;
	CUAVariable* pUAVariable = OpcUa_Null;
	lua_State *state = (lua_State *)vm->GetLuaState();
	OpcUa_NodeId nodeId;
	OpcUa_NodeId_Initialize(&nodeId);
	OpcUa_NodeId deviceNodeId;
	OpcUa_NodeId_Initialize(&deviceNodeId);
	OpcUa_String szAddress;
	OpcUa_String_Initialize(&szAddress);

	CUAInformationModel* pInformationModel = CServerApplication::m_pTheAddressSpace;
	int itop = lua_gettop(state);
	if (itop == 3)
	{
		// Extract nodeId
		int iType = lua_type(state, 1);
		const char *s = lua_typename(state, iType);
		if (strcmp(s, "string") == 0)
		{
			// existin nodeId . It must be on a UAVariable
			OpcUa_String szNodeId;
			OpcUa_String_Initialize(&szNodeId);
			const char* szVal = lua_tostring(state, 1);
			OpcUa_String_AttachCopy(&szNodeId, szVal);
			Utils::String2NodeId(szNodeId, &nodeId);

			uStatus = pInformationModel->GetNodeIdFromVariableList(nodeId, &pUAVariable);
			if (uStatus == OpcUa_Good)
			{
				// Device Id (NodeId)
				iType = lua_type(state, 2);
				s = lua_typename(state, iType);
				OpcUa_String_Initialize(&szNodeId);
				const char* szVal = lua_tostring(state, 2);
				OpcUa_String_AttachCopy(&szNodeId, szVal);
				Utils::String2NodeId(szNodeId, &deviceNodeId);
				// Address
				iType = lua_type(state, 3);
				s = lua_typename(state, iType);
				if (strcmp(s, "string") == 0)
				{
					szVal = lua_tostring(state, 3);
					OpcUa_String_AttachCopy(&szAddress, szVal);
				}
				else
					iResult = 5;
			}
			else
				iResult = 4;
		}
		else
			iResult = 3;
	}
	else
		iResult = 2;
	if (iResult == 1)
	{
		uStatus = pInformationModel->AddVpiUATag(nodeId, deviceNodeId, szAddress);
		if (uStatus != OpcUa_Good)
			iResult = 7;
	}
	return iResult;	
}
const OpcUa_Variant& COpenOpcUaScript::GetVariantFromLuaFunction(void)
{ 
	return(m_variantFromLuaFunction); 
}
void COpenOpcUaScript::SetVariantFromLuaFunction(const OpcUa_Variant& variantFromLuaFunction)
{ 
	m_variantFromLuaFunction = variantFromLuaFunction; 
}



