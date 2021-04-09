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
#include "LuaVirtualMachine.h"
#include "LuaScript.h"
#include "OpenOpcUaScript.h"
using namespace UAScript;

//#include "MonitoredItemServer.h"
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
#include "VfiDataValue.h"
#include "UAHistorianVariable.h"
#include "HaEngine.h"
using namespace UAHistoricalAccess;
#include "ServerApplication.h"
#include <float.h>

using namespace OpenOpcUa;
using namespace UACoreServer;
using namespace UAAddressSpace;

extern CServerApplication* g_pTheApplication;

OpcUa_Boolean CSubscriptionServer::IsMonitoredItemFromUAObjectEqual(CUAObject* /*pUAObject*/, CMonitoredItemServer* /*pMonitoredItemserver*/)
{
	OpcUa_Boolean bResult = OpcUa_False;
	return bResult;
}// Cette méthode verifiera les attributs commun
OpcUa_Boolean CSubscriptionServer::IsMonitoredItemFromUABaseEqual(CUABase* pUABase, CMonitoredItemServer* pMonitoredItemserver)
{
	OpcUa_Boolean bResult = OpcUa_False;
	if ((pUABase) && (pMonitoredItemserver))
	{
		// si le mode de monitoring est Disabled on retourne FALSE afin de ne pas provoquer de notification :)
		if ((pMonitoredItemserver->GetMonitoringMode() == OpcUa_MonitoringMode_Disabled))
		{
			if (pMonitoredItemserver->IsCold())
				pMonitoredItemserver->SetCold(OpcUa_False);
			else
				bResult = OpcUa_True;
		}
		else
		{
			if (!pMonitoredItemserver->IsCold())
			{
				// Analyse de l'attribut auquel le client s'est abonné
				switch (pMonitoredItemserver->GetAttributeId())
				{
				case OpcUa_Attributes_NodeId:
				{
					OpcUa_NodeId aFirstNodeId = pMonitoredItemserver->GetNodeId();
					OpcUa_NodeId* pSecondNodeId = pUABase->GetNodeId();
					bResult = Utils::IsEqual(&aFirstNodeId, pSecondNodeId);
				}
				break;
				case OpcUa_Attributes_NodeClass:
				{
					if (pMonitoredItemserver->GetNodeClass() == pUABase->GetNodeClass())
						bResult = OpcUa_True;
				}
				break;
				case OpcUa_Attributes_BrowseName:
				{
					OpcUa_QualifiedName* aFirstString = pMonitoredItemserver->GetBrowseName();
					OpcUa_QualifiedName* aSecondString = pUABase->GetBrowseName();
					if (OpcUa_String_StrnCmp(&(aFirstString->Name), &(aSecondString->Name), OpcUa_String_StrLen(&(aSecondString->Name)), OpcUa_False) == 0)
						bResult = OpcUa_True;
				}
				break;
				case OpcUa_Attributes_DisplayName:
				{
					OpcUa_LocalizedText aFirstString = pMonitoredItemserver->GetDisplayName();
					OpcUa_LocalizedText aSecondString = pUABase->GetDisplayName();
					if (OpcUa_String_StrnCmp(&(aFirstString.Text), &(aSecondString.Text), OpcUa_String_StrLen(&(aSecondString.Text)), OpcUa_False) == 0)
						bResult = OpcUa_True;
				}
				break;
				case OpcUa_Attributes_Description:
				{
					OpcUa_LocalizedText aFirstString = pMonitoredItemserver->GetDescription();
					OpcUa_LocalizedText aSecondString = pUABase->GetDescription();
					if (OpcUa_String_StrnCmp(&(aFirstString.Text), &(aSecondString.Text), OpcUa_String_StrLen(&(aSecondString.Text)), OpcUa_False) == 0)
						bResult = OpcUa_True;
				}
				break;
				case OpcUa_Attributes_WriteMask:
				{
					if (pMonitoredItemserver->GetWriteMask() == pUABase->GetWriteMask())
						bResult = OpcUa_True;
				}
				break;
				case OpcUa_Attributes_UserWriteMask:
				{
					if (pMonitoredItemserver->GetUserWriteMask() == pUABase->GetUserWriteMask())
						bResult = OpcUa_True;
				}
				break;
				default:
					break;
				}
			}
			else
				pMonitoredItemserver->SetCold(OpcUa_False);
		}
	}
	return bResult;
}
//IsMonitoredItemEqual>IsEqual={bResult} NodeId={pUAVariable->GetNodeId().Identifier.Numeric}
// si on retourne OpcUa_False cela provoquera une notification
OpcUa_Boolean CSubscriptionServer::IsMonitoredItemFromUAVariableEqual(CUAVariable* pUAVariable, CMonitoredItemServer* pMonitoredItemserver)
{
	OpcUa_Boolean bResult = OpcUa_False;

	if ((pUAVariable) && (pMonitoredItemserver))
	{
		pMonitoredItemserver->GetDataChangeFilter()->DeadbandType;
		pMonitoredItemserver->GetDataChangeFilter()->DeadbandValue;
		////////////////////////////////////////////////////////////
		// traitement des filtres de notification
		//OpcUa_DataChangeTrigger_Status               = 0
		//OpcUa_DataChangeTrigger_StatusValue          = 1
		//OpcUa_DataChangeTrigger_StatusValueTimestamp = 2
		OpcUa_Boolean bDataChangeFilterOk = OpcUa_False;
		OpcUa_DataChangeFilter* pDataChangeFilter = pMonitoredItemserver->GetDataChangeFilter();
		if (pDataChangeFilter)
		{
			if ((pDataChangeFilter->Trigger == OpcUa_DataChangeTrigger_StatusValue) || (pDataChangeFilter->Trigger ==OpcUa_DataChangeTrigger_StatusValueTimestamp) )
				bDataChangeFilterOk = OpcUa_True;

		}
		else
			bDataChangeFilterOk = OpcUa_True;
		if (bDataChangeFilterOk)
		{
			// si le mode de monitoring est Disabled on retourne FALSE afin de ne pas provoquer de notification :)
			if ((pMonitoredItemserver->GetMonitoringMode() == OpcUa_MonitoringMode_Disabled))
			{
				bResult = OpcUa_True;
				if (pMonitoredItemserver->IsCold())
					pMonitoredItemserver->SetCold(OpcUa_False);
				else
					bResult = OpcUa_True;
			}
			else
			{
				// Pour le cas sampling on va remonter la valeur si :
				//SamplingInterval>UtcNow-TimeLastNotification
				
				//if ((pMonitoredItemserver->GetMonitoringMode() == OpcUa_MonitoringMode_Sampling))
				//{
				//	if (pMonitoredItemserver->IsCold())
				//		pMonitoredItemserver->SetCold(OpcUa_False);
				//	OpcUa_UInt32 uDiff;
				//	if (OpcUa_P_GetDateTimeDiffInSeconds32(OpcUa_DateTime_UtcNow(), pMonitoredItemserver->GetLastNotificationTime(), &uDiff) == OpcUa_Good)
				//	{
				//		if (uDiff > pMonitoredItemserver->GetSamplingInterval())
				//			bResult = OpcUa_False;
				//		else
				//			bResult = OpcUa_True;
				//	}
				//}
				//else
				{
					// Check the value itself
					//if ((pMonitoredItemserver->GetMonitoringMode() == OpcUa_MonitoringMode_Reporting))
					{
						// Check for the base Attribute
						if (!pMonitoredItemserver->IsCold())
						{
							bResult = OpcUa_True;
							bResult=IsMonitoredItemFromUABaseEqual((CUABase*)pUAVariable, pMonitoredItemserver);
							if (!bResult)
							{
								switch (pMonitoredItemserver->GetAttributeId())
								{
								case OpcUa_Attributes_ArrayDimensions:
								{
									std::vector<OpcUa_UInt32>* pVectOfDim = pUAVariable->GetArrayDimensions();
									if (pVectOfDim)
									{
										if (pMonitoredItemserver->GetValue()->Value.Value.UInt32 == pVectOfDim->size())
											bResult = OpcUa_True;
									}
									else
										bResult = OpcUa_True; // to avoid crash
								}
								break;
								case OpcUa_Attributes_AccessLevel:
								{
									OpcUa_Byte aUAVariableAccessLevel = pUAVariable->GetAccessLevel();
									if (pMonitoredItemserver->GetValue()->Value.Value.Byte == aUAVariableAccessLevel)
										bResult = OpcUa_True;
								}
								break;
								//case OpcUa_Attributes_BrowseName:
								//{
								//	OpcUa_QualifiedName* pQualifiedName = pUAVariable->GetBrowseName();
								//	if (OpcUa_QualifiedName_Compare(pMonitoredItemserver->GetValue()->Value.Value.QualifiedName, pQualifiedName) == 0)
								//		bResult = OpcUa_True;
								//}
								//break;
								case OpcUa_Attributes_ContainsNoLoops:
									break;
								case OpcUa_Attributes_DataType:
								{
									OpcUa_NodeId aUAVariableNodeId = pUAVariable->GetDataType();
									OpcUa_NodeId aMonitoredDataType;
									OpcUa_NodeId_Initialize(&aMonitoredDataType);
									aMonitoredDataType.IdentifierType = OpcUa_IdentifierType_Numeric;
									if (pMonitoredItemserver->GetValue()->Value.Datatype == OpcUaType_NodeId)
										OpcUa_NodeId_CopyTo(pMonitoredItemserver->GetValue()->Value.Value.NodeId, &aMonitoredDataType);
									else
										aMonitoredDataType.Identifier.Numeric = pMonitoredItemserver->GetValue()->Value.Datatype;
									bResult = Utils::IsEqual(&aUAVariableNodeId, &aMonitoredDataType);
									OpcUa_NodeId_Clear(&aMonitoredDataType);
								}
								break;
								//case OpcUa_Attributes_Description:
								//	break;
								//case OpcUa_Attributes_DisplayName:
								//	break;
								case OpcUa_Attributes_Executable:
									break;
								case OpcUa_Attributes_Historizing:
								{
									OpcUa_Byte aUAVariableHistorizing = pUAVariable->GetHistorizing();
									if (pMonitoredItemserver->GetValue()->Value.Value.Boolean == aUAVariableHistorizing)
										bResult = OpcUa_True;
								}
								break;
								case OpcUa_Attributes_InverseName:
									break;
								case OpcUa_Attributes_IsAbstract:
									break;
								case OpcUa_Attributes_MinimumSamplingInterval:
								{
									OpcUa_Double dblVal = pUAVariable->GetMinimumSamplingInterval();
									if (pMonitoredItemserver->GetValue()->Value.Value.Byte == dblVal)
										bResult = OpcUa_True;
								}
									break;
								//case OpcUa_Attributes_NodeClass:
								//break;
								//case OpcUa_Attributes_NodeId:
								//{
								//	OpcUa_NodeId aNodeId = pUAVariable->GetNodeId();
								//	if (OpcUa_NodeId_Compare(pMonitoredItemserver->GetValue()->Value.Value.NodeId, &aNodeId) == 0)
								//		bResult = OpcUa_True;
								//}
								//break;
								case OpcUa_Attributes_Symmetric:
									break;
								case OpcUa_Attributes_UserAccessLevel:
								{
									OpcUa_Byte aUAVariableAccessLevel = pUAVariable->GetUserAccessLevel();
									if (pMonitoredItemserver->GetValue()->Value.Value.Byte == aUAVariableAccessLevel)
										bResult = OpcUa_True;
								}
								break;
								//case OpcUa_Attributes_UserExecutable:
								//	break;
								//case OpcUa_Attributes_UserWriteMask:
								//	break;
								case OpcUa_Attributes_Value:
								{
									CDataValue* pUAVariableValue = pUAVariable->GetValue();
									OpcUa_Variant aUAVariableVariantValue = pUAVariableValue->GetValue();
									OpcUa_DataValue* pMonitoredItemserverValue = pMonitoredItemserver->GetValue();
									// Check the dataType to compare Scalar or Array
									if (aUAVariableVariantValue.ArrayType == OpcUa_VariantArrayType_Array)
									{
										// in case of Array check if the client asked for a Range									
										OpcUa_String* pString = pMonitoredItemserver->GetIndexRange();
										if (OpcUa_String_StrLen(pString) > 0)
										{
											OpcUa_Boolean bRangeOk = OpcUa_True;
											CNumericRange* pNumericRange = new CNumericRange(pString);
											// We have to extract the value to compare based on the pNumericRange
											OpcUa_Int32 iArrayLen = 0;
											if (pNumericRange)
												iArrayLen = (pNumericRange->GetEndIndex() - pNumericRange->GetBeginIndex()) + 1;
											if (pNumericRange->IsMultiDimensional())
												bRangeOk = OpcUa_False;

											if (bRangeOk)
											{
												bResult = OpcUa_True;
												OpcUa_Int32 iBeginRange = pNumericRange->GetBeginIndex();
												OpcUa_UInt32 iii = 0;
												if (pNumericRange->IsUnique())
												{
													if (pNumericRange->GetEndIndex() > 0)
														iBeginRange = pNumericRange->GetEndIndex() - 1;
													else
														iBeginRange = 0;
												}
												for (OpcUa_Int32 ii = iBeginRange; ii < iArrayLen; ii++)
												{
													if (ii < aUAVariableVariantValue.Value.Array.Length)
													{
														switch (aUAVariableVariantValue.Datatype)
														{
														case OpcUaType_Boolean:
															if (pMonitoredItemserverValue->Value.Value.Array.Value.BooleanArray[iii] != aUAVariableVariantValue.Value.Array.Value.BooleanArray[ii])
																bResult = OpcUa_False;
															break;
														case OpcUaType_Byte:
															if (pMonitoredItemserverValue->Value.Value.Array.Value.ByteArray[iii] != aUAVariableVariantValue.Value.Array.Value.ByteArray[ii])
																bResult = OpcUa_False;
															break;
														case OpcUaType_ByteString:
															if (!Utils::IsEqual(&(pMonitoredItemserverValue->Value.Value.Array.Value.ByteStringArray[iii]), &(aUAVariableVariantValue.Value.Array.Value.ByteStringArray[ii])))
																bResult = OpcUa_False;
															break;
														case OpcUaType_DateTime:
															if ((pMonitoredItemserverValue->Value.Value.Array.Value.DateTimeArray[iii].dwHighDateTime != aUAVariableVariantValue.Value.Array.Value.DateTimeArray[ii].dwHighDateTime)
																|| (pMonitoredItemserverValue->Value.Value.Array.Value.DateTimeArray[iii].dwLowDateTime != aUAVariableVariantValue.Value.Array.Value.DateTimeArray[ii].dwLowDateTime))
																bResult = OpcUa_False;
															break;
														case OpcUaType_Double:
															if (pMonitoredItemserverValue->Value.Value.Array.Value.DoubleArray[iii] != aUAVariableVariantValue.Value.Array.Value.DoubleArray[ii])
																bResult = OpcUa_False;
															break;
														case OpcUaType_Float:
															if (pMonitoredItemserverValue->Value.Value.Array.Value.FloatArray[iii] != aUAVariableVariantValue.Value.Array.Value.FloatArray[ii])
																bResult = OpcUa_False;
															break;
														case OpcUaType_Guid:
															if (OpcUa_Guid_Compare(&(pMonitoredItemserverValue->Value.Value.Array.Value.GuidArray[iii]), &(aUAVariableVariantValue.Value.Array.Value.GuidArray[ii])) != 0)
																bResult = OpcUa_False;
															break;
														case OpcUaType_Int16:
															if (pMonitoredItemserverValue->Value.Value.Array.Value.Int16Array[iii] != aUAVariableVariantValue.Value.Array.Value.Int16Array[ii])
																bResult = OpcUa_False;
															break;
														case OpcUaType_Int32:
															if (pMonitoredItemserverValue->Value.Value.Array.Value.Int32Array[iii] != aUAVariableVariantValue.Value.Array.Value.Int32Array[ii])
																bResult = OpcUa_False;
															break;
														case OpcUaType_Int64:
															if (pMonitoredItemserverValue->Value.Value.Array.Value.Int64Array[iii] != aUAVariableVariantValue.Value.Array.Value.Int64Array[ii])
																bResult = OpcUa_False;
															break;
														case OpcUaType_LocalizedText:
															if (OpcUa_LocalizedText_Compare(&(pMonitoredItemserverValue->Value.Value.Array.Value.LocalizedTextArray[iii]), &(aUAVariableVariantValue.Value.Array.Value.LocalizedTextArray[ii])) != 0)
																bResult = OpcUa_False;
															break;
														case OpcUaType_NodeId:
															if (OpcUa_NodeId_Compare(&(pMonitoredItemserverValue->Value.Value.Array.Value.NodeIdArray[iii]), &(aUAVariableVariantValue.Value.Array.Value.NodeIdArray[ii])) != 0)
																bResult = OpcUa_False;
															break;
														case OpcUaType_QualifiedName:
															if (OpcUa_QualifiedName_Compare(&(pMonitoredItemserverValue->Value.Value.Array.Value.QualifiedNameArray[iii]), &(aUAVariableVariantValue.Value.Array.Value.QualifiedNameArray[ii])) != 0)
																bResult = OpcUa_False;
															break;
														case OpcUaType_SByte:
															if (pMonitoredItemserverValue->Value.Value.Array.Value.SByteArray[iii] != aUAVariableVariantValue.Value.Array.Value.SByteArray[ii])
																bResult = OpcUa_False;
															break;
														case OpcUaType_StatusCode:
															if (pMonitoredItemserverValue->Value.Value.Array.Value.StatusCodeArray[iii] != aUAVariableVariantValue.Value.Array.Value.StatusCodeArray[ii])
																bResult = OpcUa_False;
															break;
														case OpcUaType_String:
															if (!Utils::IsEqual(&(pMonitoredItemserverValue->Value.Value.Array.Value.StringArray[iii]), &(aUAVariableVariantValue.Value.Array.Value.StringArray[ii])))
																bResult = OpcUa_False;
															break;
														case OpcUaType_UInt16:
															if (pMonitoredItemserverValue->Value.Value.Array.Value.UInt16Array[iii] != aUAVariableVariantValue.Value.Array.Value.UInt16Array[ii])
																bResult = OpcUa_False;
															break;
														case OpcUaType_UInt32:
															if (pMonitoredItemserverValue->Value.Value.Array.Value.UInt32Array[iii] != aUAVariableVariantValue.Value.Array.Value.UInt32Array[ii])
																bResult = OpcUa_False;
															break;
														case OpcUaType_UInt64:
															if (pMonitoredItemserverValue->Value.Value.Array.Value.UInt64Array[iii] != aUAVariableVariantValue.Value.Array.Value.UInt64Array[ii])
																bResult = OpcUa_False;
															break;
														default:
															break;
														}
														if (!bResult)
															break;
													}
													iii++; // inc the index in the monitoredItem value
												}
											}
											if (pNumericRange)
												delete pNumericRange;
										}
										else
										{
											// Let's check that the UAVariable in the server cache is not Null
											if (aUAVariableVariantValue.Value.Array.Length == 0)
												bResult = OpcUa_True; // So we will not notify en empty array
											else
											{
												// if no range was specified then we will compare the full array
												bResult = Utils::IsEqual(&aUAVariableVariantValue, &(pMonitoredItemserverValue->Value));
												if (bResult) // seulement si true afin de ne pas notifier deux fois
												{
													if (pUAVariableValue->GetStatusCode() != pMonitoredItemserverValue->StatusCode)
														bResult = OpcUa_False;
												}
											}
										}
									}
									else
									{
										if (aUAVariableVariantValue.ArrayType == OpcUa_VariantArrayType_Scalar)	
											bResult = Utils::IsEqual(&aUAVariableVariantValue, &(pMonitoredItemserverValue->Value));
										if (bResult) // seulement si true afin de ne pas notifier deux fois
										{
											if (pUAVariableValue->GetStatusCode() != pMonitoredItemserverValue->StatusCode)
												bResult = OpcUa_False;
										}
									}
								}
								break;
								case OpcUa_Attributes_ValueRank:
									{
										OpcUa_Int32 iValueRank = pUAVariable->GetValueRank();
										if (pMonitoredItemserver->GetValue()->Value.Value.Int32 == iValueRank)
											bResult = OpcUa_True;
									}
									break;
								//case OpcUa_Attributes_WriteMask:
								//	break;
								default:
									OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "MonitoredItemserver requested attribute %u is unsupported\n", pMonitoredItemserver->GetAttributeId());
									bResult = OpcUa_False;
									break;
								}
							}
							/////////////////////////////////////////////////////////////////////////////////////////
							//
							// Check timestamp in case of OpcUa_DataChangeTrigger_StatusValueTimestamp and deadband
							// Verify the deadband
							OpcUa_String* pRange = pMonitoredItemserver->GetIndexRange();
							if (pDataChangeFilter)
							{
								if (pDataChangeFilter->DeadbandType == OpcUa_DeadbandType_Absolute)
								{
									OpcUa_DataValue* pUADataValue = pUAVariable->GetValue()->GetInternalDataValue();
									OpcUa_DataValue* pDataValue = pMonitoredItemserver->GetValue();
									bResult = IsValueEqualWithDataChangeFilter(pUADataValue, pDataValue, pDataChangeFilter, pRange);
								}
							}
							if (bResult)
							{
								if (pDataChangeFilter)
								{
									// Verify the DataChangeTrigger 
									if (pDataChangeFilter->Trigger == OpcUa_DataChangeTrigger_StatusValueTimestamp)
									{
										// Check the Timestamp
										if (pMonitoredItemserver->IsCold())
											pMonitoredItemserver->SetCold(OpcUa_False);
										else
										{
											// Is the StatusCode Change ?
											OpcUa_DateTime ServerTimeStamp1 = pUAVariable->GetValue()->GetServerTimestamp();
											OpcUa_DateTime ServerTimeStamp2 = pMonitoredItemserver->GetValue()->ServerTimestamp;
											if (OpcUa_DateTime_Compare(&ServerTimeStamp1, &ServerTimeStamp2) != 0)
												bResult = OpcUa_False;
											else
											{
												OpcUa_DateTime ServerTimeStamp3 = pUAVariable->GetValue()->GetSourceTimeStamp();
												OpcUa_DateTime ServerTimeStamp4 = pMonitoredItemserver->GetValue()->SourceTimestamp;
												if (OpcUa_DateTime_Compare(&ServerTimeStamp3, &ServerTimeStamp4) != 0)
													bResult = OpcUa_False;
												else
													bResult = OpcUa_True; // Hormis pour le coldStart on considère qu'il n'y a pas de changement			
											}
										}
									}
								}
							}
						}
						else
						{
							CDataValue* pUAVariableValue = pUAVariable->GetValue();
							if (pUAVariableValue)
							{
								OpcUa_Variant aUAVariableVariantValue = pUAVariableValue->GetValue();
								if (aUAVariableVariantValue.ArrayType == OpcUa_VariantArrayType_Scalar)
								{
									OpcUa_String* pString = pMonitoredItemserver->GetIndexRange();
									if (OpcUa_String_StrLen(pString) > 0)
										bResult = OpcUa_True; // In case of a Range we will not notify anything to the client because range are forbiden on Scalar dataType
									// Except for byteString. In this byteString case we are suppose to accept the Range. MonitorValueChange case 023.js
									if ((aUAVariableVariantValue.Datatype == OpcUaType_ByteString) || (aUAVariableVariantValue.Datatype == OpcUaType_String))
									{
										bResult = OpcUa_False;
										m_pSession->WakeupNotificationDataThread();
									}
								}
								// But we are leaving the coldstate
								pMonitoredItemserver->SetCold(OpcUa_False);
							}
							else
								OpcUa_Trace(OPCUA_TRACE_SERVER_ERROR, "Critical error>IsMonitoredItemFromUAVariableEqual failed. Input UAVariable parameter is NULL\n");
						}
					}
				}
			}
		}
		else
		{
			// Traitement du filter de notification sur StatusCode
			if (pDataChangeFilter)
				if (pDataChangeFilter->Trigger == OpcUa_DataChangeTrigger_Status)
					bDataChangeFilterOk = OpcUa_True;
			if (bDataChangeFilterOk)
			{
				{
					if (pMonitoredItemserver->IsCold())
						pMonitoredItemserver->SetCold(OpcUa_False);
					else
					{
						// Is the StatusCode Change ?
						if (pUAVariable->GetValue()->GetStatusCode() != pMonitoredItemserver->GetStatusCode())
							bResult = OpcUa_False;
						else
							bResult = OpcUa_True; // Hormis pour le couldStart on cosidère qu'il n'y a pas de changement			
					}
				}
			}
		}
	}
	else
		bResult = OpcUa_True;
	return bResult;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Is value equal with data change filter. </summary>
///
/// <remarks>	Michel, 16/05/2016. </remarks>
///
/// <param name="pUAVariableValue"> 	[in,out] If non-null, the UA variable value. </param>
/// <param name="pMonitoredValue">  	[in,out] If non-null, the monitored value. </param>
/// <param name="pDataChangeFilter">	[in,out] If non-null, a filter specifying the data change.
/// </param>
/// <param name="pRange">				[in,out] If non-null, the range. </param>
///
/// <returns>	An OpcUa_Boolean. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_Boolean CSubscriptionServer::IsValueEqualWithDataChangeFilter(OpcUa_DataValue* pUAVariableValue, OpcUa_DataValue* pMonitoredValue, OpcUa_DataChangeFilter* pDataChangeFilter,OpcUa_String * pRange)
{
	OpcUa_Boolean bResult = OpcUa_True;
	if ((pUAVariableValue) && (pMonitoredValue))
	{
		if (pDataChangeFilter)
		{
			OpcUa_Boolean bRangeOk = OpcUa_True;
			CNumericRange* pNumericRange = OpcUa_Null;
			OpcUa_Int32 iArrayLen = 0;
			OpcUa_Int32 iBeginRange = 0;
			if (pRange)
			{
				if (OpcUa_String_StrLen(pRange) > 0)
				{
					pNumericRange = new CNumericRange(pRange);
					// We have to extract the value to compare based on the pNumericRange
					if (pNumericRange)
						iArrayLen = (pNumericRange->GetEndIndex() - pNumericRange->GetBeginIndex()) + 2;
					if (pNumericRange->IsMultiDimensional())
						bRangeOk = OpcUa_False;
				}
			}
			if (bRangeOk) // if no range then the range will be ok
			{
				if (pDataChangeFilter->DeadbandType == OpcUa_DeadbandType_Absolute)
				{
					bResult = OpcUa_True;
					if (pUAVariableValue->Value.ArrayType == OpcUa_VariantArrayType_Scalar)
					{
						switch (pUAVariableValue->Value.Datatype)
						{
						case OpcUaType_Byte:
						{
							if (abs(pUAVariableValue->Value.Value.Byte - pMonitoredValue->Value.Value.Byte) > pDataChangeFilter->DeadbandValue)
								bResult = OpcUa_False;
						}
						break;
						case OpcUaType_SByte:
						{
							if (abs(pUAVariableValue->Value.Value.SByte - pMonitoredValue->Value.Value.SByte) > pDataChangeFilter->DeadbandValue)
								bResult = OpcUa_False;
						}
						break;
						case OpcUaType_Int16:
						{
							if (abs(pUAVariableValue->Value.Value.Int16 - pMonitoredValue->Value.Value.Int16) > pDataChangeFilter->DeadbandValue)
								bResult = OpcUa_False;
						}
						break;
						case OpcUaType_UInt16:
						{
							if (abs(pUAVariableValue->Value.Value.UInt16 - pMonitoredValue->Value.Value.UInt16) > pDataChangeFilter->DeadbandValue)
								bResult = OpcUa_False;
						}
						break;
						case OpcUaType_Int32:
						{
							if (abs((int)(pUAVariableValue->Value.Value.Int32 - pMonitoredValue->Value.Value.Int32)) > (OpcUa_Int32)pDataChangeFilter->DeadbandValue)
								bResult = OpcUa_False;
						}
						break;
						case OpcUaType_UInt32:
						{
							if (abs((int)(pUAVariableValue->Value.Value.UInt32 - pMonitoredValue->Value.Value.UInt32)) > pDataChangeFilter->DeadbandValue)
								bResult = OpcUa_False;
						}
						break;
						case OpcUaType_Int64:
						{
							if (abs((int)(pUAVariableValue->Value.Value.Int32 - pMonitoredValue->Value.Value.Int32)) > pDataChangeFilter->DeadbandValue)
								bResult = OpcUa_False;
						}
						break;
						case OpcUaType_UInt64:
						{
							if (abs((int)(pUAVariableValue->Value.Value.UInt64 - pMonitoredValue->Value.Value.UInt64)) > pDataChangeFilter->DeadbandValue)
								bResult = OpcUa_False;
						}
						break;
						case OpcUaType_Float:
						{
							if (abs((pUAVariableValue->Value.Value.Float - pMonitoredValue->Value.Value.Float)) > pDataChangeFilter->DeadbandValue)
								bResult = OpcUa_False;
						}
						break;
						case OpcUaType_Double:
						{
							if (abs((pUAVariableValue->Value.Value.Double - pMonitoredValue->Value.Value.Double)) > pDataChangeFilter->DeadbandValue)
								bResult = OpcUa_False;
						}
						break;
						default:
							break;
						}
					}
					else
					{
						OpcUa_UInt32 uiLen = pUAVariableValue->Value.Value.Array.Length;
						if (pNumericRange)
						{
							/*OpcUa_Int32*/ iBeginRange = pNumericRange->GetBeginIndex();
							if (pNumericRange->IsUnique())
							{
								if (pNumericRange->GetEndIndex() > 0)
									iBeginRange = pNumericRange->GetEndIndex() - 1;
								else
									iBeginRange = 0;
							}
							uiLen = iArrayLen;
						}
						switch (pUAVariableValue->Value.Datatype)
						{
						case OpcUaType_Byte:
						{
							for (OpcUa_UInt32 i = iBeginRange; i<uiLen; i++)
							{
								if (abs(pUAVariableValue->Value.Value.Array.Value.ByteArray[i] - pMonitoredValue->Value.Value.Array.Value.ByteArray[i - iBeginRange]) >= pDataChangeFilter->DeadbandValue)
								{
									bResult = OpcUa_False;
									break;
								}
							}
						}
						break;
						case OpcUaType_SByte:
						{
							for (OpcUa_UInt32 i = iBeginRange; i<uiLen; i++)
							{
								if (abs(pUAVariableValue->Value.Value.Array.Value.SByteArray[i] - pMonitoredValue->Value.Value.Array.Value.SByteArray[i - iBeginRange]) >= pDataChangeFilter->DeadbandValue)
								{
									bResult = OpcUa_False;
									break;
								}
							}
						}
						break;
						case OpcUaType_Int16:
						{
							for (OpcUa_UInt32 i = iBeginRange; i<uiLen; i++)
							{
								if (abs(pUAVariableValue->Value.Value.Array.Value.Int16Array[i] - pMonitoredValue->Value.Value.Array.Value.Int16Array[i - iBeginRange]) >= pDataChangeFilter->DeadbandValue)
								{
									bResult = OpcUa_False;
									break;
								}
							}
						}
						break;
						case OpcUaType_UInt16:
						{
							for (OpcUa_UInt32 i = iBeginRange; i<uiLen; i++)
							{
								if (abs(pUAVariableValue->Value.Value.Array.Value.UInt16Array[i] - pMonitoredValue->Value.Value.Array.Value.UInt16Array[i - iBeginRange]) >= pDataChangeFilter->DeadbandValue)
								{
									bResult = OpcUa_False;
									break;
								}
							}
						}
						break;
						case OpcUaType_Int32:
						{
							for (OpcUa_UInt32 i = iBeginRange; i<uiLen; i++)
							{
								if (abs((int)(pUAVariableValue->Value.Value.Array.Value.Int32Array[i] - pMonitoredValue->Value.Value.Array.Value.Int32Array[i - iBeginRange])) > pDataChangeFilter->DeadbandValue)
								{
									bResult = OpcUa_False;
									break;
								}
							}
						}
						break;
						case OpcUaType_UInt32:
						{
							for (OpcUa_UInt32 i = iBeginRange; i<uiLen; i++)
							{
								if (abs((int)(pUAVariableValue->Value.Value.Array.Value.UInt32Array[i] - pMonitoredValue->Value.Value.Array.Value.UInt32Array[i - iBeginRange])) >= pDataChangeFilter->DeadbandValue)
								{
									bResult = OpcUa_False;
									break;
								}
							}
						}
						break;
						case OpcUaType_Int64:
						{
							for (OpcUa_UInt32 i = iBeginRange; i<uiLen; i++)
							{
								if (abs((int)(pUAVariableValue->Value.Value.Array.Value.Int64Array[i] - pMonitoredValue->Value.Value.Array.Value.Int64Array[i - iBeginRange])) > pDataChangeFilter->DeadbandValue)
								{
									bResult = OpcUa_False;
									break;
								}
							}
						}
						break;
						case OpcUaType_UInt64:
						{
							for (OpcUa_UInt32 i = iBeginRange; i<uiLen; i++)
							{
								if (abs((int)(pUAVariableValue->Value.Value.Array.Value.UInt64Array[i] - pMonitoredValue->Value.Value.Array.Value.UInt64Array[i - iBeginRange])) > pDataChangeFilter->DeadbandValue)
								{
									bResult = OpcUa_False;
									break;
								}
							}
						}
						break;
						case OpcUaType_Float:
						{
							for (OpcUa_UInt32 i = iBeginRange; i<uiLen; i++)
							{
								if (abs(pUAVariableValue->Value.Value.Array.Value.FloatArray[i] - pMonitoredValue->Value.Value.Array.Value.FloatArray[i - iBeginRange]) > pDataChangeFilter->DeadbandValue)
								{
									bResult = OpcUa_False;
									break;
								}
							}
						}
						break;
						case OpcUaType_Double:
						{
							for (OpcUa_UInt32 i = iBeginRange; i<uiLen; i++)
							{
								if (abs(pUAVariableValue->Value.Value.Array.Value.DoubleArray[i] - pMonitoredValue->Value.Value.Array.Value.DoubleArray[i - iBeginRange]) > pDataChangeFilter->DeadbandValue)
								{
									bResult = OpcUa_False;
									break;
								}
							}
						}
						break;
						default:
							break;
						}
					}
				}
			}
			if (pNumericRange)
				delete pNumericRange;
		}
	}
	return bResult;
}
CSubscriptionServer::CSubscriptionServer()
{
	m_uiLifeTimeCountCounter = 0;
	OpcUa_DateTime_Initialize(&m_LastTransmissionTime);
	m_fastestSamplingInterval = 10000.0;
	m_bLifetimeCountReach = OpcUa_False;
	m_bInLate = OpcUa_False;
	m_uiCurrentSequenceNumber = 0;
	OpcUa_Mutex_Create(&m_AvailableSequenceNumbersMutex);
	m_bKeepAlive = OpcUa_False;
	m_bSubscriptionInColdState = OpcUa_True;
	//m_bSubscriptionInColdState = OpcUa_False;
	m_pMonitoredItemList = OpcUa_Null;
	m_LastKnownServerState = OpcUa_ServerState_Unknown;
	//m_pDataChangeNotificationList = OpcUa_Null;
	m_hMonitoredItemListMutex = OpcUa_Null;
	// On doit être certain que le n° de souscription est unique pour cette session
	OpcUa_UInt32 uiRandVal = (OpcUa_UInt32)this;
	SetSubscriptionId(uiRandVal);
	//OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"CSubscriptionServer():%u>\n",GetSubscriptionId());
	CUAInformationModel* pInformationModel = CServerApplication::m_pTheAddressSpace;
	if (pInformationModel->IsEnabledServerDiagnosticsDefaultValue())
	{
		m_pSubscriptionDiagnosticsDataType = new CSubscriptionDiagnosticsDataType();
	}
	else
		m_pSubscriptionDiagnosticsDataType = OpcUa_Null;
	//m_pSubscriptionDiagnosticsDataType = OpcUa_Null;
	//
	OpcUa_StatusCode uStatus = OpcUa_Semaphore_Create(&m_MonitoredItemListSem, 0, 0x100);
	if (uStatus == OpcUa_Good)
	{
		uStatus = OpcUa_Mutex_Create(&m_hMonitoredItemListMutex);
		if (uStatus == OpcUa_Good)
		{
			m_pMonitoredItemList = new CMonitoredItemServerList();
			if (m_pMonitoredItemList)
			{
				OpcUa_Mutex_Lock(m_hMonitoredItemListMutex);
				CMonitoredItemServerList* pMonitoredItemServerList = GetMonitoredItemList(); // Constructor
				pMonitoredItemServerList->clear();
				OpcUa_Mutex_Unlock(m_hMonitoredItemListMutex);
				//
				m_pSession = OpcUa_Null;
				m_bRunUpdateDataSubscriptionThread = OpcUa_True;
				m_hUpdateDataSubscriptionThread = OpcUa_Null;
				m_iLastSentMessage = 0;
				OpcUa_Semaphore_Create(&m_hStopUpdateDataSubscriptionSem, 0, 0x1000);
				OpcUa_Semaphore_Create(&m_hUpdateSubscriptionThreadSafeSem, 0, 0x100);
				// Vecteur contenant les notifications de changement sur les données
				uStatus = OpcUa_Mutex_Create(&m_hDataChangeNotificationListMutex);
				//m_pDataChangeNotificationList = new CUADataChangeNotificationList();
				//if (m_pDataChangeNotificationList)
				//{
					//m_pDataChangeNotificationList->clear();
					m_DataChangeNotificationCurrentPos = 0;
					// Vecteur contenant les notifications de changement sur l'etat interne du serveur
					OpcUa_Mutex_Create(&m_hStatusChangeNotificationListMutex);
					m_pStatusChangeNotificationList = new CUAStatusChangeNotificationList();
					if (m_pStatusChangeNotificationList)
					{
						m_pStatusChangeNotificationList->clear();
						m_StatusChangeNotificationCurrentPos = 0;
						// Vecteur contenant les Events du serveur (CUAEventNotificationListList)
						uStatus = OpcUa_Mutex_Create(&m_hEventNotificationListListMutex);
						m_pEventNotificationListList = new CUAEventNotificationListList();
						if (m_pEventNotificationListList)
						{
							m_pEventNotificationListList->clear();
							m_EventNotificationListCurrentPos = 0;
							// Compteur de KeepAlive et de LifeTimeCount
							//m_uiKeepAliveCounter = 0;
							//m_uiLifeTimeCountCounter = 0;
							// démarrage de la thread en charge de la souscription
							StartUpdateSubscriptionThread();
						}
						else
						{
							uStatus = OpcUa_BadOutOfMemory;
							OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "CSubscriptionServer():%u>m_pEventChangeNotificationList construction failed uStatus=0x%05x\n", GetSubscriptionId(), uStatus);
						}
					}
					else
					{
						uStatus = OpcUa_BadOutOfMemory;
						OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "CSubscriptionServer():%u>m_pStatusChangeNotificationList construction failed uStatus=0x%05x\n", GetSubscriptionId(), uStatus);
					}
				//}
				//else
				//{
				//	uStatus = OpcUa_BadOutOfMemory;
				//	OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "CSubscriptionServer():%u>m_pDataChangeNotificationList construction failed uStatus=0x%05x\n", GetSubscriptionId(), uStatus);
				//}
			}
			else
			{
				uStatus = OpcUa_BadOutOfMemory;
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "CSubscriptionServer():%u>m_pMonitoredItemList construction failed uStatus=0x%05x\n", GetSubscriptionId(), uStatus);
			}
		}
		else
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "CSubscriptionServer():%u>OpcUa_Mutex_Create failed uStatus=0x%05x\n", GetSubscriptionId(), uStatus);
	}
	else
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "CSubscriptionServer():%u>OpcUa_Semaphore_Create failed uStatus=0x%05x\n", GetSubscriptionId(), uStatus);
}
CSubscriptionServer::CSubscriptionServer(CSessionServer* pSession, double dblSubscriptionInterval)
{
	m_uiLifeTimeCountCounter = 0;
	OpcUa_DateTime_Initialize(&m_LastTransmissionTime);
	m_fastestSamplingInterval = 10000.0;
	m_bLifetimeCountReach = OpcUa_False;
	m_bInLate = OpcUa_False;
	m_uiCurrentSequenceNumber = 0;
	OpcUa_Mutex_Create(&m_AvailableSequenceNumbersMutex);
	m_bKeepAlive = OpcUa_False;
	m_bSubscriptionInColdState = OpcUa_True; 
	//m_bSubscriptionInColdState = OpcUa_False;
	m_pMonitoredItemList = OpcUa_Null;
	m_hMonitoredItemListMutex = OpcUa_Null;
	m_LastKnownServerState = OpcUa_ServerState_Unknown;
	// On doit être certain que le n° de souscription est unique pour cette session
	OpcUa_UInt32 uiRandVal = (OpcUa_UInt32)this;
	SetSubscriptionId(uiRandVal);
	CUAInformationModel* pInformationModel = CServerApplication::m_pTheAddressSpace;
	if (pInformationModel->IsEnabledServerDiagnosticsDefaultValue())
	{
		m_pSubscriptionDiagnosticsDataType = new CSubscriptionDiagnosticsDataType();
	}
	else
		m_pSubscriptionDiagnosticsDataType = OpcUa_Null;
	//m_pSubscriptionDiagnosticsDataType = OpcUa_Null;
	//
	OpcUa_Semaphore_Create(&m_MonitoredItemListSem, 0, 0x100);
	OpcUa_StatusCode uStatus = OpcUa_Mutex_Create(&m_hMonitoredItemListMutex);
	if (uStatus == OpcUa_Good)
	{
		OpcUa_Mutex_Lock(m_hMonitoredItemListMutex);
		m_pMonitoredItemList = new CMonitoredItemServerList();
		if (m_pMonitoredItemList)
		{
			m_pMonitoredItemList->clear();
			OpcUa_Mutex_Unlock(m_hMonitoredItemListMutex);
			OpcUa_Semaphore_Post(m_MonitoredItemListSem, 1);
			//
			SetSession(pSession);
			SetPublishingInterval(dblSubscriptionInterval);
			m_bRunUpdateDataSubscriptionThread = OpcUa_True;
			m_hUpdateDataSubscriptionThread = OpcUa_Null;

			m_iLastSentMessage = 0;
			uStatus = OpcUa_Semaphore_Create(&m_hUpdateSubscriptionThreadSafeSem, 0, 0x100);
			uStatus = OpcUa_Semaphore_Create(&m_hStopUpdateDataSubscriptionSem, 0, 0x100);
			if (uStatus == OpcUa_Good)
			{
				// Vecteur contenant les notifications de changement sur les données
				uStatus = OpcUa_Mutex_Create(&m_hDataChangeNotificationListMutex);
				if (uStatus == OpcUa_Good)
				{
					//m_pDataChangeNotificationList = new CUADataChangeNotificationList();
					//if (m_pDataChangeNotificationList)
					//{
					//	m_pDataChangeNotificationList->clear();
						m_DataChangeNotificationCurrentPos = 0;
						// Vecteur contenant les notifications de changement sur l'etat interne du serveur
						m_pStatusChangeNotificationList = new CUAStatusChangeNotificationList();
						if (m_pStatusChangeNotificationList)
						{
							m_pStatusChangeNotificationList->clear();
							m_StatusChangeNotificationCurrentPos = 0;
							uStatus = OpcUa_Mutex_Create(&m_hStatusChangeNotificationListMutex);
							if (uStatus == OpcUa_Good)
							{
								// Vecteur contenant les notifications d'evenement
								uStatus = OpcUa_Mutex_Create(&m_hEventNotificationListListMutex);
								if (uStatus == OpcUa_Good)
								{
									m_pEventNotificationListList = new CUAEventNotificationListList();
									m_pEventNotificationListList->clear();
									m_EventNotificationListCurrentPos = 0;
									//m_uiKeepAliveCounter = 0;
									//m_uiLifeTimeCountCounter = 0;
									// démarrage de la thread en charge de la souscription
									StartUpdateSubscriptionThread();
								}
								else
									OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critcal Error:CSubscriptionServer>Cannot OpcUa_Mutex_Create m_hEventNotificationListListMutex 0x%05x\n", uStatus);
							}
							else
								OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critcal Error:CSubscriptionServer>Cannot OpcUa_Mutex_Create m_hStatusChangeNotificationListMutex 0x%05x\n", uStatus);
						}
					//}
					//else
					//	OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critcal Error:CSubscriptionServer> Probably a memory error\n");
				}
				else
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critcal Error:CSubscriptionServer>Cannot OpcUa_Mutex_Create m_hDataChangeNotificationListMutex 0x%05x\n", uStatus);
			}
			else
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critcal Error:CSubscriptionServer>Cannot OpcUa_Semaphore_Create m_hStopUpdateDataSubscriptionSem 0x%05x\n", uStatus);
		}
		else
		{
			OpcUa_Mutex_Unlock(m_hMonitoredItemListMutex);
			OpcUa_Semaphore_Post(m_MonitoredItemListSem, 1);
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critcal Error:CSubscriptionServer>Probably a memory error with m_pMonitoredItemList\n");
		}
	}
	else
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Critcal Error:CSubscriptionServer>Cannot OpcUa_Mutex_Create m_hMonitoredItemListMutex 0x%05x\n", uStatus);
}

CSubscriptionServer::~CSubscriptionServer()
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	CUAInformationModel* pInformationModel = CServerApplication::m_pTheAddressSpace;
	OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_WARNING, "Destructor>~CSubscriptionServer() for m_pSubscriptionId=%u\n", m_pSubscriptionId);

	StopUpdateSubscriptionThread();
	OpcUa_Semaphore_Delete(&m_hStopUpdateDataSubscriptionSem);
	OpcUa_Semaphore_Delete(&m_hUpdateSubscriptionThreadSafeSem);
	m_hUpdateDataSubscriptionThread = OpcUa_Null;
	// release ressources related to DataChangeNotificationList 
	DeleteAllDataChangeNotfication();
	// delete All the monitoredItems
	uStatus = DeleteMonitoredItems();
	if (uStatus == OpcUa_Good)
	{
		OpcUa_Mutex_Lock(m_hMonitoredItemListMutex);
		if (m_pMonitoredItemList)
		{
			delete m_pMonitoredItemList;
			m_pMonitoredItemList = OpcUa_Null;
		}
		OpcUa_Mutex_Unlock(m_hMonitoredItemListMutex);
		OpcUa_Semaphore_Post(m_MonitoredItemListSem, 1);
	}
	else
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "DeleteMonitoredItems failed uStatus=0x%05x\n", uStatus);
	//
	CSubscriptionDiagnosticsDataType* pSubscriptionDiagnosticsDataType = GetSubscriptionDiagnosticsDataType();
	if (pSubscriptionDiagnosticsDataType)
	{
		if (pInformationModel)
			pInformationModel->RemoveInSubscriptionDiagnosticsArray(pSubscriptionDiagnosticsDataType);

		delete m_pSubscriptionDiagnosticsDataType;
		m_pSubscriptionDiagnosticsDataType = OpcUa_Null;
	}
	OpcUa_Mutex_Lock(m_hDataChangeNotificationListMutex);
	//CUADataChangeNotificationList::iterator it;
	//while (!m_pDataChangeNotificationList->empty())
	//{
	//	it = m_pDataChangeNotificationList->begin();
	//	delete *it;
	//	m_pDataChangeNotificationList->erase(it);
	//}
	//delete m_pDataChangeNotificationList;
	OpcUa_Mutex_Unlock(m_hDataChangeNotificationListMutex);

	OpcUa_Mutex_Delete(&m_hDataChangeNotificationListMutex);
	// release ressources related to StatusChangeNotification
	OpcUa_Mutex_Lock(m_hStatusChangeNotificationListMutex);
	CUAStatusChangeNotificationList::iterator itSC = m_pStatusChangeNotificationList->begin();
	while (itSC != m_pStatusChangeNotificationList->end())
	{
		delete *itSC;
		itSC++;
	}
	m_pStatusChangeNotificationList->clear();
	delete m_pStatusChangeNotificationList;
	m_pStatusChangeNotificationList = OpcUa_Null;
	OpcUa_Mutex_Unlock(m_hStatusChangeNotificationListMutex);
	OpcUa_Mutex_Delete(&m_hStatusChangeNotificationListMutex);
	// release ressources related to EventNotificationList
	OpcUa_Mutex_Lock(m_hEventNotificationListListMutex);
	if (m_pEventNotificationListList)
	{
		CUAEventNotificationListList::iterator itEvent;
		for (itEvent = m_pEventNotificationListList->begin(); itEvent != m_pEventNotificationListList->end(); itEvent++)
		{
			delete *itEvent;
		}
		m_pEventNotificationListList->clear();
		delete m_pEventNotificationListList;
	}
	OpcUa_Mutex_Unlock(m_hEventNotificationListListMutex);
	OpcUa_Mutex_Delete(&m_hEventNotificationListListMutex);

	OpcUa_Semaphore_Delete(&m_MonitoredItemListSem);
	OpcUa_Mutex_Delete(&m_hMonitoredItemListMutex);
	// Empty m_AvailableSequenceNumbers
	OpcUa_Mutex_Lock(m_AvailableSequenceNumbersMutex);
	CAvailableSequenceNumbers::iterator iteratorAvailableSequenceNumber=m_AvailableSequenceNumbers.begin();
	while (iteratorAvailableSequenceNumber!=m_AvailableSequenceNumbers.end())
	{
		CUADataChangeNotification* pDataChangeNotification = iteratorAvailableSequenceNumber->second;
		delete pDataChangeNotification;
		iteratorAvailableSequenceNumber++;
	}
	m_AvailableSequenceNumbers.clear();
	OpcUa_Mutex_Unlock(m_AvailableSequenceNumbersMutex);
	OpcUa_Mutex_Delete(&m_AvailableSequenceNumbersMutex);
}
// Delete all monitoredItems for the Subscription
// Return OpcUa_Good if delete succeed.
// OpcUa_GoodNonCriticalTimeout if it failed
OpcUa_StatusCode CSubscriptionServer::DeleteMonitoredItems()
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	uStatus = OpcUa_Semaphore_TimedWait(m_MonitoredItemListSem, 2000); // OPC_TIMEOUT 7500 - Infinite =OPCUA_INFINITE
	if (uStatus == OpcUa_GoodNonCriticalTimeout)
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "CSubscriptionServer::DeleteMonitoredItems>m_MonitoredItemListSem not receive\n");
	else
	{
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_WARNING, "CSubscriptionServer::DeleteMonitoredItems>m_MonitoredItemListSem properly received\n");
		OpcUa_Mutex_Lock(m_hMonitoredItemListMutex);

		CMonitoredItemServerList::iterator it = m_pMonitoredItemList->begin();
		for (it = m_pMonitoredItemList->begin(); it != m_pMonitoredItemList->end(); it++)
		{
			CMonitoredItemServer* pMonitoredItemServer = *it;
			if (pMonitoredItemServer)
			{
				delete pMonitoredItemServer;
				pMonitoredItemServer = OpcUa_Null;
			}
		}
		OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_WARNING, "DeleteMonitoredItems>m_pSubscriptionId= %u, will clear MonitoredItemList\n", m_pSubscriptionId);
		m_pMonitoredItemList->clear();
		OpcUa_Mutex_Unlock(m_hMonitoredItemListMutex);
		OpcUa_Semaphore_Post(m_MonitoredItemListSem, 1);
	}
	return uStatus;
}

void CSubscriptionServer::StartUpdateSubscriptionThread()
{
	if (m_hUpdateDataSubscriptionThread == OpcUa_Null)
	{
		m_bRunUpdateDataSubscriptionThread = TRUE;
		OpcUa_StatusCode uStatus = OpcUa_Thread_Create(&m_hUpdateDataSubscriptionThread, UpdateDataSubscriptionThread, this);

		if (uStatus != OpcUa_Good)
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Create UpdateDataSubscriptionThread Failed");
		else
		{
			OpcUa_Thread_Start(m_hUpdateDataSubscriptionThread);
		}
	}
}

// Function name   : CSubscriptionServer::UpdateDataSubscriptionThread
// Description     : Cette thread sera chargé de mettre a jour les items qui ont changé depuis la derniere notification
//				   : Ces items seront placé dans m_pDataChangeNotificationList s'il s'agit d'un changement de donnée a notifier, 
//						ou dans m_pEventNotificationListList s'il s'agit d'un event a notifier.
// Return type     : void
// Argument        : LPVOID arg

void CSubscriptionServer::UpdateDataSubscriptionThread(LPVOID arg)
{
	OpcUa_UInt32 uiSleepTime;
	OpcUa_UInt32 uiInterval = 0;
	OpcUa_StatusCode uStatus = OpcUa_Bad;
	CSubscriptionServer* pSubscription = (CSubscriptionServer*)arg;
	pSubscription->ResetLifeTimeCountCounter();
	//CUAInformationModel* pInformationModel = CServerApplication::m_pTheAddressSpace;

	pSubscription->m_uiLastPublishAnswer = 0;
	OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_WARNING, "Started>UpdateDataSubscriptionThread Id=%u run:%u\n", pSubscription->GetSubscriptionId(), pSubscription->m_bRunUpdateDataSubscriptionThread);
	while (pSubscription->m_bRunUpdateDataSubscriptionThread)
	{
		OpcUa_Boolean bPreIncKeepAliveCounter = OpcUa_True;
		uiInterval = ((DWORD)pSubscription->GetPublishingInterval());

		OpcUa_UInt32 uiOldInterval = uiInterval;
#if _WIN32_WINNT >= 0x0600
		ULONGLONG dwStart = GetTickCount64();
#else 
	#if _WIN32_WINNT == 0x0501
		DWORD dwStart = GetTickCount();
	#endif
#endif
#ifdef _GNUC_
		DWORD dwStart=GetTickCount();
#endif
		// Mise a jour de la cache de cette souscription
		// Contient la liste des notifications pour une souscritpion données
		CSessionServer* pSessionServer = pSubscription->GetSession();
		if (pSessionServer)
		{
			// Verrouillage de MonitoredItemList
			// Analyse de tous les items monitoré pour cette souscritpion
			OpcUa_Mutex_Lock(pSubscription->m_hMonitoredItemListMutex);

			CMonitoredItemServerList* pMonitoredItemServerList = pSubscription->GetMonitoredItemList(); // UpdateDataSubscriptionThread
			if (pMonitoredItemServerList)
			{
				// Portion de code pour le CTT 015.js Subscription Services - Subscription basic
				// permet au keepalive de fonctionner même si la souscription est vide
				if (pMonitoredItemServerList->empty())
					bPreIncKeepAliveCounter = OpcUa_True;
				////////////////////////////////////////////////////////////////////////////:
				// First step
				////////////////////////////////////////////////////////////////////////////
				// Let's try to find a CUADataChangeNotification Acked that contains unsent MonitoredNotification 
				// 
				CUADataChangeNotification* pDataChangeNotification = OpcUa_Null;
				OpcUa_Boolean bRecycledDataChangeNotification=OpcUa_False; // tell us that we are using or not a already save pUADataChangeNotification				
				OpcUa_UInt32 uiCurrentSequenceNumber = pSubscription->GetCurrentSequenceNumber();

				OpcUa_Mutex_Lock(pSubscription->m_AvailableSequenceNumbersMutex);
				CAvailableSequenceNumbers AvailableSequenceNumbers = pSubscription->AvailableSequenceNumberGet();
				CAvailableSequenceNumbers::iterator iteratorAvailableSequenceNumber;
				for (iteratorAvailableSequenceNumber = AvailableSequenceNumbers.begin(); iteratorAvailableSequenceNumber != AvailableSequenceNumbers.end(); iteratorAvailableSequenceNumber++)
				{
					CUADataChangeNotification* pTmpDataChangeNotification = iteratorAvailableSequenceNumber->second;//	
					if (pTmpDataChangeNotification) // pTmpDataChangeNotification is null when we recycle a CUADataChangeNotification
					{
						if (pTmpDataChangeNotification->IsAcked())
						{
							CUAMonitoredItemNotificationList aMonitoredItemNotificationList = pTmpDataChangeNotification->GetUAMonitoredItemNotificationList();
							for (OpcUa_UInt32 ii = 0; ii < aMonitoredItemNotificationList.size(); ii++)
							{
								CUAMonitoredItemNotification* pUAMonitoredItemNotification = aMonitoredItemNotificationList.at(ii);
								if (!pUAMonitoredItemNotification->IsMonitoredNotificationSent())
								{
									if (!pDataChangeNotification)
									{
										pDataChangeNotification = pSubscription->GetUnackedDataChangeNotification();
										if (!pDataChangeNotification)
										{
											uiCurrentSequenceNumber = pSubscription->GetCurrentSequenceNumber();
											pDataChangeNotification = new CUADataChangeNotification(uiCurrentSequenceNumber);
											pDataChangeNotification->SetDataChangeNotificationType(NOTIFICATION_MESSAGE_DATACHANGE);
											pDataChangeNotification->SetMaxQueueSize(pTmpDataChangeNotification->GetMaxQueueSize());
											pDataChangeNotification->SetPublishTime(pTmpDataChangeNotification->GetPublishTime());
										}
									}
									if (pDataChangeNotification)
									{
										OpcUa_MonitoredItemNotification* pMonitoredItemNotification = pUAMonitoredItemNotification->GetMonitoredItemNotification();
										OpcUa_MonitoredItemNotification* pNewMonitoredItemNotification = (OpcUa_MonitoredItemNotification*)OpcUa_Alloc(sizeof(OpcUa_MonitoredItemNotification));
										OpcUa_MonitoredItemNotification_Initialize(pNewMonitoredItemNotification);
										// Handle
										pNewMonitoredItemNotification->ClientHandle = pMonitoredItemNotification->ClientHandle;
										// horodate
										pNewMonitoredItemNotification->Value.ServerTimestamp = pMonitoredItemNotification->Value.ServerTimestamp;
										pNewMonitoredItemNotification->Value.ServerPicoseconds = 0;
										pNewMonitoredItemNotification->Value.SourceTimestamp = pMonitoredItemNotification->Value.SourceTimestamp;
										pNewMonitoredItemNotification->Value.SourcePicoseconds = 0;
										// StatusCode
										pNewMonitoredItemNotification->Value.StatusCode = pMonitoredItemNotification->Value.StatusCode;
										OpcUa_Variant_CopyTo(&(pMonitoredItemNotification->Value.Value),
											&(pNewMonitoredItemNotification->Value.Value)); // Warning with ExtensionObject copy
										/*OpcUa_Trace(OPCUA_TRACE_LEVEL_INFO, "UpdateDataSubscriptionThread>Add pending notification QueueSize=%u\n",
											pDataChangeNotification->GetMaxQueueSize());*/
										uStatus = pDataChangeNotification->AddMonitoredItemNotification(pNewMonitoredItemNotification,
											pDataChangeNotification->GetMaxQueueSize(),
											pUAMonitoredItemNotification->IsDiscardOldest());
										if (uStatus==OpcUa_BadNothingToDo)
											pUAMonitoredItemNotification->MonitoredNotificationSent(OpcUa_True);
										if (uStatus == OpcUa_GoodResultsMayBeIncomplete)// Found a UAObject to Notify
										{
											if (pSubscription->m_pSubscriptionDiagnosticsDataType)
											{
												pSubscription->m_pSubscriptionDiagnosticsDataType->SetMonitoringQueueOverflowCount(pSubscription->m_pSubscriptionDiagnosticsDataType->GetMonitoringQueueOverflowCount() + 1);
											}
										}
										OpcUa_MonitoredItemNotification_Clear(pNewMonitoredItemNotification);
										OpcUa_Free(pNewMonitoredItemNotification);
									}
									else
										OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "UpdateDataSubscriptionThread>Critical error: OutofMemory\n");
								}
							}
						}
					}
				}
				/////////////////////////////////////////////////////////////////////////////
				// Second step
				////////////////////////////////////////////////////////////////////////////


				// Il s'agit de mettre a jour la cache de cette souscription et de preparer les messages de notification
				// Chaque message de notification correspond a une instance de CUADataChangeNotification
				// verification de changement d'etat qui on eu lieu dans l'interval
				// scan des MonitoredItems

				for (OpcUa_UInt32 ii = 0; ii < pMonitoredItemServerList->size(); ii++)
				{
					CMonitoredItemServer* pMonitoredItem = pMonitoredItemServerList->at(ii);
					if (pMonitoredItem->GetEventFilter())
					{
						if (pMonitoredItem->GetEventFields().size() > 0)
						{
							// Let's check if the ServerState changed 
							// if yes we will handle a SystemStatusChangeEventType (i=11446). This EventType is define in part 3 §8.71	
							uStatus = pSubscription->HandleSystemStatusChangeEvent(pMonitoredItem);			
							//CServerStatus* pServerStatus = pInformationModel->GetInternalServerStatus();
							//
							// Now let check if there are any Event to notify to the client
							if (g_pTheApplication)
							{
								// Let start by Looking at the current state and their notification
								// is it in confirmState and is it notified
								// is it in ackedState and is it notified
								uStatus = pSubscription->HandleAlreadyNotifiedEvent();
								if (uStatus == OpcUa_BadNothingToDo)
								{
									// Now we will check the Event with ActiveState=true reported by the Event Engine
									// This is the place where we prepare the first step of the events notification to clients.
									// Todo so we will look at the EventDefinitions
									CEventsEngine* pEventsEngine = g_pTheApplication->GetEventsEngine();
									if (pEventsEngine)
									{
										OpcUa_Mutex eventEngineMutex = pEventsEngine->GetEventDefinitionListMutex();
										OpcUa_Mutex_Lock(eventEngineMutex);

										// Let's find all EventDefinition where pMonitoredItem->m_pBase is the sourceNode
										CEventDefinitionList eventDefinitionList;
										uStatus = pEventsEngine->GetEventDefinition(pMonitoredItem->GetNodeId(), &eventDefinitionList); // 2
										if (uStatus == OpcUa_Good) // 2
										{
											for (OpcUa_UInt32 iEventDef = 0; iEventDef < eventDefinitionList.size(); iEventDef++) // 2
												//for (OpcUa_UInt32 iEventDef = 0; iEventDef < pEventsEngine->GetEventDefinitionListSize(); iEventDef++) // 1
											{
												//CEventDefinition* pEventDefinition = OpcUa_Null;  // 1
												CEventDefinition* pEventDefinition = eventDefinitionList.at(iEventDef); // 2
												//if (pEventsEngine->GetEventDefinition(iEventDef, &pEventDefinition) == OpcUa_Good) // 1
												if (!pEventDefinition->IsOnStage())
												{
													if (!(pSubscription->IsOnStage(pEventDefinition)))
													{
														if (pEventDefinition->GetActiveState() == (OpcUa_Boolean)OpcUa_True)
														{
															// check if we notify for an Acknowledge
															pEventDefinition->SetPreviousActiveState(OpcUa_True);
															OpcUa_UInt32 uiSequenceNumber = pSubscription->GetCurrentSequenceNumber();
															CUAEventNotificationList* pUAEventNotificationList = new CUAEventNotificationList(uiSequenceNumber);
															if (pUAEventNotificationList)
															{
																// Set the associated MonitoredItem
																pUAEventNotificationList->SetMonitoredItem(pMonitoredItem);
																// Set the associate EventDefinition
																pUAEventNotificationList->SetEventDefinition(pEventDefinition);
																// Set the NotificationType
																pUAEventNotificationList->SetNotificationType(NOTIFICATION_MESSAGE_EVENT);
																uStatus = pMonitoredItem->InitializeEventsFieldsEx(pEventDefinition, &pUAEventNotificationList);
																if (uStatus == OpcUa_Good)
																{
																	pSubscription->AddEventNotificationList(pUAEventNotificationList);
																	pEventDefinition->OnStage(OpcUa_True);
																}
																else
																	delete pUAEventNotificationList;
															}
														}
													}
												}
											}
										}

										OpcUa_Mutex_Unlock(eventEngineMutex);
									}
								}
							}
						}
					}
					/////////////////////////////////////////////////////////////////////////////
					//
					// Handle Value change. This will be the base for DataChangeNotification
					if (pMonitoredItem->GetMonitoringMode() == OpcUa_MonitoringMode_Reporting)
					{
						CUABase* pUABase = pMonitoredItem->GetUABase();
						if (pUABase)
						{
							switch (pUABase->GetNodeClass())
							{
							case OpcUa_NodeClass_Variable:
							{
								CUAVariable* pUAVariable = (CUAVariable*)pUABase;
								pSubscription->CompareMonitoredItemToUAVariable(&pDataChangeNotification, pUAVariable, pMonitoredItem, &bPreIncKeepAliveCounter, &bRecycledDataChangeNotification);
								uStatus = OpcUa_Good;
							}
							break;
							case OpcUa_NodeClass_Object:
							{
								uStatus = pSubscription->CompareMonitoredItemToUAObject(&pDataChangeNotification, pUABase, pMonitoredItem, &bPreIncKeepAliveCounter);
							}
							break;
							case OpcUa_NodeClass_Method:
							{
								uStatus = OpcUa_BadNothingToDo;
							}
							break;
							case OpcUa_NodeClass_ObjectType:
							{
								uStatus = OpcUa_BadNothingToDo;
							}
							break;
							case OpcUa_NodeClass_VariableType:
							{
								uStatus = OpcUa_BadNothingToDo;
							}
							break;
							case OpcUa_NodeClass_ReferenceType:
							{
							}
							break;
							case OpcUa_NodeClass_DataType:
							{
								uStatus = OpcUa_BadNothingToDo;
							}
							break;
							case OpcUa_NodeClass_View:
							{
								uStatus = OpcUa_BadNothingToDo;
							}
							break;
							default:
								uStatus = OpcUa_BadNothingToDo;
								break;
							}
						}
					}
					else
					{
						if (pMonitoredItem->GetMonitoringMode() == OpcUa_MonitoringMode_Sampling)
						{
							OpcUa_UInt32 uDiff;
							OpcUa_DateTime lastNotification = pMonitoredItem->GetLastNotificationTime();

							OpcUa_StatusCode uStatusTmp = OpcUa_P_GetDateTimeDiffInSeconds32(lastNotification, OpcUa_DateTime_UtcNow(), &uDiff);
							if ((uStatusTmp == OpcUa_Good) || (uStatusTmp == OpcUa_BadOutOfRange))
							{
								if (uDiff > pMonitoredItem->GetSamplingInterval())
								{
									// Need to notify - bResult = OpcUa_False;
									OpcUa_MonitoredItemNotification* pSamplingMonitoredItemNotification = (OpcUa_MonitoredItemNotification*)OpcUa_Alloc(sizeof(OpcUa_MonitoredItemNotification));
									OpcUa_MonitoredItemNotification_Initialize(pSamplingMonitoredItemNotification);
									pSamplingMonitoredItemNotification->ClientHandle = pMonitoredItem->GetClientHandle();
									OpcUa_DataValue_CopyTo(pMonitoredItem->GetValue(), &(pSamplingMonitoredItemNotification->Value));
									if (!pDataChangeNotification)
									{
										uiCurrentSequenceNumber = pSubscription->GetCurrentSequenceNumber();
										pDataChangeNotification = new CUADataChangeNotification(uiCurrentSequenceNumber);
										pDataChangeNotification->SetDataChangeNotificationType(NOTIFICATION_MESSAGE_DATACHANGE);
									}
									pDataChangeNotification->AddMonitoredItemNotification(pSamplingMonitoredItemNotification, 0, OpcUa_False);
									uStatus = OpcUa_Good;
								}
								else
									uStatus = OpcUa_BadNothingToDo;
							}
						}
						else
						{
							uStatus = OpcUa_BadNothingToDo;
						}
					}
				} // fin for (OpcUa_UInt32 ii=0;ii<pMonitoredItemServerList->size();ii++)		


				// Clean the memory in case the pDataChangeNotification is empty
				if (pDataChangeNotification)
				{
					if (pDataChangeNotification->GetNoOfMonitoredItemNotification() == 0)
					{
						delete pDataChangeNotification;
						pDataChangeNotification = OpcUa_Null;
					}/////////////////////////////////////////////////////////////
					if ((pDataChangeNotification) && (!bRecycledDataChangeNotification))
					{
						if ((pDataChangeNotification->GetNoOfMonitoredItemNotification() > 0)
							|| (pDataChangeNotification->GetDataChangeNotificationType() == NOTIFICATION_MESSAGE_KEEPALIVE))
						{
							uiCurrentSequenceNumber = pSubscription->GetCurrentSequenceNumber();
							if (uiCurrentSequenceNumber == 0)
								uiCurrentSequenceNumber = 1;
							pSubscription->AvailableSequenceNumberAdd(uiCurrentSequenceNumber, pDataChangeNotification);
						}
						else
						{
							delete pDataChangeNotification;
							pDataChangeNotification = OpcUa_Null;
						}
					}
				}
				OpcUa_Mutex_Unlock(pSubscription->m_AvailableSequenceNumbersMutex);
				OpcUa_Semaphore publishSem = pSessionServer->GetPublishSem();
				OpcUa_Semaphore_Post(publishSem, 1);
				// Fin de la deuxième étape
				/////////////////////////////////////////////////////////////
			}

			OpcUa_Mutex_Unlock(pSubscription->m_hMonitoredItemListMutex);
		}
		// post un EventSem autorisant la modification sur la m_pMonitoredItemList
		OpcUa_Semaphore_Post(pSubscription->m_MonitoredItemListSem, 1);
		// On calcul le temps de traitement afin de determiner le temps d'attente
#if _WIN32_WINNT >= 0x0600
		ULONGLONG dwEnd = GetTickCount64();
		ULONGLONG dwCountedTime = dwEnd - dwStart;
		if (uiInterval>dwCountedTime)
			uiSleepTime = (OpcUa_UInt32)(uiInterval - dwCountedTime);
		else
			uiSleepTime = 0;
#else 
#if _WIN32_WINNT == 0x0501
		DWORD dwEnd = GetTickCount();
		DWORD dwCountedTime = dwEnd - dwStart;
		if (uiInterval>dwCountedTime)
			uiSleepTime = uiInterval - dwCountedTime;
		else
			uiSleepTime = 0;
#endif
#endif

#ifdef _GNUC_
		DWORD dwEnd = GetTickCount();
		DWORD dwCountedTime = dwEnd - dwStart;
		if (uiInterval>dwCountedTime)
			uiSleepTime = uiInterval - dwCountedTime;
		else
			uiSleepTime = 0;
#endif
		// awake the AsyncRequestThread to force the management of newly detected changes
		// and maybe to send it to the clientpSessionServer->WakeupAsyncRequestThread();
		{
			OpcUa_Semaphore_TimedWait(pSubscription->m_hUpdateSubscriptionThreadSafeSem, uiSleepTime);
			//OpcUa_UInt32 uiOldInterval = uiInterval;
			uiInterval = ((DWORD)pSubscription->GetPublishingInterval());
			if (uiOldInterval<=uiInterval)
				OpcUa_Semaphore_TimedWait(pSubscription->m_hUpdateSubscriptionThreadSafeSem, uiInterval-uiSleepTime);
			uiOldInterval = uiInterval;
		}
		// If the LifeTimeCount is reach will mark the subscription for deletion by the CSessionServer Object
		if (pSubscription->m_uiLifeTimeCountCounter >= pSubscription->GetLifetimeCount())
		{
			pSubscription->m_bLifetimeCountReach = OpcUa_True;
			OpcUa_Semaphore pSemaphore = pSessionServer->GetSubscriptionsLifeTimeCountSem();
			OpcUa_Semaphore_Post(pSemaphore, 1);
			pSubscription->m_uiLifeTimeCountCounter = 0;
		}
	}
	// Au cas ou la thread soit en cours d'arret pendant sa phase de démarrage
	//OpcUa_Semaphore_Post(pSubscription->m_hUpdateSubscriptionThreadSafeSem,1);
	OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_WARNING, "UpdateDataSubscriptionThread stopped Id=%u\n", pSubscription->GetSubscriptionId());
	OpcUa_Semaphore_Post(pSubscription->m_hStopUpdateDataSubscriptionSem, 1);

}
OpcUa_StatusCode CSubscriptionServer::StopUpdateSubscriptionThread()
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	if (m_bRunUpdateDataSubscriptionThread)
	{
		m_bRunUpdateDataSubscriptionThread = OpcUa_False;
		OpcUa_Semaphore_Post(m_hUpdateSubscriptionThreadSafeSem, 1); // StopUpdateSubscriptionThread()
		OpcUa_UInt32 uiSleep = (OpcUa_UInt32)GetPublishingInterval() * 3;
		if (uiSleep > OPC_TIMEOUT * 2)
			uiSleep = OPC_TIMEOUT * 2;
		uStatus = OpcUa_Semaphore_TimedWait(m_hStopUpdateDataSubscriptionSem, uiSleep); // INFINITE - OPC_TIMEOUT*2= 15 secondes max.
		if (uStatus == OpcUa_GoodNonCriticalTimeout)
		{
			// force the end of UpdateDataSubscriptionThread
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Impossible to stop the UpdateDataSubscriptionThread. Timeout Id=%u\n", GetSubscriptionId());
		}
		else
		{
			OpcUa_Thread_Delete(m_hUpdateDataSubscriptionThread);
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_WARNING, "UpdateDataSubscriptionThread stopped properly Id=%u uStatus=0x%05x\n", GetSubscriptionId(), uStatus);
		}
		OpcUa_Semaphore_Post(m_MonitoredItemListSem, 1);

	}
	return uStatus;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	This method check is something need to be notify for this subscription. 
///				It not checks the DataChangeNotification, StatusChangeNotification, EventNotification 
///				and keepAlive</summary>
///
/// <remarks>	Michel, 28/01/2016. </remarks>
///
/// <returns>	An OpcUa_Boolean. True if something to notify otherwise False </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_Boolean  CSubscriptionServer::IsSomethingNotify()
{
	//return OpcUa_True;
	OpcUa_Boolean bResult=OpcUa_False;
	OpcUa_Boolean bDataChangeNotificationAvailable = OpcUa_False;
	OpcUa_Boolean bStatusChangeNotificationAvailable = OpcUa_False;
	OpcUa_Boolean bEventNotificationAvailable = OpcUa_False;
	//CUADataChangeNotificationList* pDataChangeNotificationList = GetDataChangeNotificationList();
	OpcUa_UInt32 ii = 0;
	//OpcUa_Mutex_Lock(m_hDataChangeNotificationListMutex);
	if (IsKeepAliveRequired())
	{
		SetKeepAlive(OpcUa_True);
		bResult = OpcUa_True;
	}
	else
	{
		if (!IsInColdState())
		{
			SetKeepAlive(OpcUa_False);
			//for (ii = 0; ii < pDataChangeNotificationList->size(); ii++)
			//{
			OpcUa_Mutex_Lock(m_AvailableSequenceNumbersMutex);
			CAvailableSequenceNumbers AvailableSequenceNumbers = AvailableSequenceNumberGet();
			CAvailableSequenceNumbers::iterator iteratorAvailableSequenceNumber;
			for (iteratorAvailableSequenceNumber = AvailableSequenceNumbers.begin(); iteratorAvailableSequenceNumber != AvailableSequenceNumbers.end(); iteratorAvailableSequenceNumber++)
			{
				CUADataChangeNotification* pDataChangeNotification = iteratorAvailableSequenceNumber->second;
				if (pDataChangeNotification)
				{
					if (!pDataChangeNotification->IsAcked())
					{
						bDataChangeNotificationAvailable = OpcUa_True;
						break;
					}
				}
			}
			OpcUa_Mutex_Unlock(m_AvailableSequenceNumbersMutex);
			CUAStatusChangeNotificationList* pStatusChangeNotificationList = GetStatusChangeNotificationList();
			for (ii = 0; ii < pStatusChangeNotificationList->size(); ii++)
			{
				CUAStatusChangeNotification* pStatusChangeNotification = pStatusChangeNotificationList->at(ii);
				if (!pStatusChangeNotification->IsAcked())
				{
					bStatusChangeNotificationAvailable = OpcUa_True;
					break;
				}
			}
			CUAEventNotificationListList* pEventNotificationListList = GetEventNotificationListList();
			for (ii = 0; ii < pEventNotificationListList->size(); ii++)
			{
				CUAEventNotificationList* pUAEventNotificationList = pEventNotificationListList->at(ii);
				if (!pUAEventNotificationList->IsTransactionAcked())
				{
					bEventNotificationAvailable = OpcUa_True;
					break;
				}
			}
			if ((bDataChangeNotificationAvailable) || (bStatusChangeNotificationAvailable) || (bEventNotificationAvailable))
				bResult = OpcUa_True;
		}
		else
			bResult = OpcUa_True;
	}
	//OpcUa_Mutex_Unlock(m_hDataChangeNotificationListMutex);
	return bResult;
}

OpcUa_StatusCode CSubscriptionServer::ProcessQueuedPublishRequestEx(CQueuedPublishMessage* pRequest, OpcUa_Boolean bAbort)
{
	OpcUa_StatusCode uStatus = OpcUa_BadInvalidArgument;
	// Check the size to be sure that we have something to do
	OpcUa_Mutex_Lock(m_hMonitoredItemListMutex);
	OpcUa_Mutex_Lock(m_hStatusChangeNotificationListMutex);
	OpcUa_Mutex_Lock(m_hEventNotificationListListMutex);

	OpcUa_Mutex_Lock(m_hDataChangeNotificationListMutex);
	OpcUa_Mutex_Lock(m_AvailableSequenceNumbersMutex);
	if (IsSomethingNotify())
	{
		OpcUa_UInt32 uiCurrentSequenceNumber = GetCurrentSequenceNumber();
		if (GetPublishingEnabled() )
		{
			// Mise en forme de la réponse

			// Dev note from the spec
			// The number of notifications per Publish is the sum of monitoredItems in the DataChangeNotification and events in the EventNotificationList
			CAvailableSequenceNumbers AvailableSequenceNumbers = AvailableSequenceNumberGet();
			OpcUa_Boolean bKeepAlive = m_bKeepAlive;
			uStatus = pRequest->FillDataChangeNotificationMessageEx2(GetSubscriptionId(),
				GetPublishingEnabled(),
				GetDataChangeNotificationCurrentPos(),
				&AvailableSequenceNumbers,
				GetMaxNotificationsPerPublish(),
				&bKeepAlive);
			if (uStatus == OpcUa_Good)
			{
				// Transmission des StatusChangeNotification au besoin
				OpcUa_Boolean bStatusKeepAlive = bKeepAlive;
				CUAStatusChangeNotificationList* pStatusChangeNotificationList = GetStatusChangeNotificationList();
				uStatus = pRequest->FillStatusChangeNotificationMessage(GetSubscriptionId(),
					GetPublishingEnabled(),
					GetStatusChangeNotificationCurrentPos(),
					pStatusChangeNotificationList,
					GetMaxNotificationsPerPublish(),
					&bStatusKeepAlive);
				if (uStatus == OpcUa_Good)
				{
					CUAEventNotificationListList* pEventNotificationListList = GetEventNotificationListList();
					uStatus = pRequest->FillEventNotificationMessage(GetSubscriptionId(),
						GetPublishingEnabled(),
						GetEventNotificationListListCurrentPos(),
						pEventNotificationListList,
						GetMaxNotificationsPerPublish(),
						&bStatusKeepAlive);

					OpcUa_UInt32 uiSequenceNumber=0;
					CAvailableSequenceNumbers::iterator iteratorAvailableSequenceNumber;
					for (iteratorAvailableSequenceNumber = m_AvailableSequenceNumbers.begin(); iteratorAvailableSequenceNumber != m_AvailableSequenceNumbers.end();iteratorAvailableSequenceNumber++)
					{
						uiSequenceNumber=iteratorAvailableSequenceNumber->first;
						CUADataChangeNotification* pDataChangeNotification = iteratorAvailableSequenceNumber->second;
						if (pDataChangeNotification)
						{
							if (!pDataChangeNotification->IsAcked())
							{
								pRequest->AddSequenceNumber(uiSequenceNumber);
							}
						}
						else
							pRequest->AddSequenceNumber(uiSequenceNumber); // pDataChangeNotification is null when it was recycled
					}

					if (uiCurrentSequenceNumber == 0)
						uiCurrentSequenceNumber=1;
					
					uStatus = pRequest->FillNotificationMessage(uiSequenceNumber);
					if (uStatus != OpcUa_Good)
						OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "PQPR>FillNotificationMessage failed. Status 0x%08X\n", uStatus);			
					pRequest->SetKeepAlive(m_bKeepAlive);
				}
				else
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "PQPR>FillStatusChangeNotificationMessage failed. Status 0x%08X\n", uStatus);
			}

		}
		else
		{
			if ( (m_bKeepAlive)|| IsInColdState() )
			{
				pRequest->SetPublishResponseSubscriptionId(m_pSubscriptionId);
				pRequest->SetKeepAlive(OpcUa_True);
				uStatus = pRequest->FillNotificationMessage(uiCurrentSequenceNumber);
				if (uStatus != OpcUa_Good)
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "PQPR>FillNotificationMessage failed. Status 0x%08X\n", uStatus);
				uStatus = OpcUa_Good;
			}
		}
	}
	else
		uStatus = OpcUa_BadNothingToDo;

	OpcUa_Mutex_Unlock(m_AvailableSequenceNumbersMutex);
	OpcUa_Mutex_Unlock(m_hDataChangeNotificationListMutex);
	OpcUa_Mutex_Unlock(m_hStatusChangeNotificationListMutex);
	OpcUa_Mutex_Unlock(m_hEventNotificationListListMutex);
	OpcUa_Mutex_Unlock(m_hMonitoredItemListMutex);
	return uStatus;
}


// Add a new MonitoredItem in the list handled by this subscription
// Return OpcUa_Good or OpcUa_BadInvalidArgument if the input parameter is invalid
OpcUa_StatusCode CSubscriptionServer::AddMonitoredItem(CMonitoredItemServer* pMonitoredItem)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	if (pMonitoredItem)
	{
		OpcUa_Semaphore_TimedWait(m_MonitoredItemListSem, 250); // OPC_TIMEOUT 7500 - Infinite =OPCUA_INFINITE
		OpcUa_Mutex_Lock(m_hMonitoredItemListMutex);
		ResetLifeTimeCountCounter();
		CMonitoredItemServerList* pMonitoredItemList = GetMonitoredItemList(); // AddMonitoredItem
		if (pMonitoredItemList)
		{
			pMonitoredItemList->push_back(pMonitoredItem);
			// CTT workaround for test case 003.js - Subscription Publish Min 02
			SetInColdState(OpcUa_False);
			//WakeupUpdateSubscriptionThread();
		}
		OpcUa_Mutex_Unlock(m_hMonitoredItemListMutex);
		OpcUa_Semaphore_Post(m_MonitoredItemListSem, 1);
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}
/// <summary>
/// Finds the monitored item by identifier (MonitoredItemId).
/// </summary>
/// <param name="Id">The identifier.</param>
/// <param name="pItem">The CMonitoredItemServer** with the  item.</param>
/// <returns>OpcUa_StatusCode</returns>
OpcUa_StatusCode CSubscriptionServer::FindMonitoredItemById(OpcUa_UInt32 Id, CMonitoredItemServer** pItem)
{
	OpcUa_StatusCode uStatus = OpcUa_BadMonitoredItemIdInvalid;
	CMonitoredItemServer* pLocalItem = OpcUa_Null;;
	// /il sera protégé par l'appelant	
	for (OpcUa_UInt32 ii = 0; ii < m_pMonitoredItemList->size(); ii++)
	{
		pLocalItem = m_pMonitoredItemList->at(ii);
		if (pLocalItem->GetMonitoredItemId() == Id)
		{
			*pItem = pLocalItem;
			uStatus = OpcUa_Good;
			break;
		}
	}
	//OpcUa_Semaphore_Post(m_MonitoredItemListSem,1);
	return uStatus;
}
// Supprime toute les OpcUa_DataChangeNotification ou CMonitoredItemServer est présent
// Cette methode est utilisé principalement quand on supprime un CMonitoredItemServer 
// Return OpcUa_BadNoMatch from CUADataChangeNotification::RemoveMonitoredItemNotification if CUADataChangeNotification is not acked
//		  OpcUa_BadNothingToDo if pDataChangeNotificationList is empty
//		  OpcUa_Good if no error was detected and all OpcUa_MonitoredItemNotification related to this clientHandle have been removed
OpcUa_StatusCode CSubscriptionServer::DeleteDataChangeNotfication(OpcUa_UInt32 uiClientHandle)
{
	OpcUa_StatusCode uStatus = OpcUa_BadNothingToDo;
	// We look for pDataChangeNotification in m_AvailableSequenceNumbers with pMonitoredItem->ClientHandle
	// and delete it from the list (pDataChangeNotificationList)
	CAvailableSequenceNumbers tmpAvailableSequenceNumbers;
	OpcUa_Mutex_Lock(m_AvailableSequenceNumbersMutex);
	CAvailableSequenceNumbers::iterator iteratorAvailableSequenceNumber;
	for (iteratorAvailableSequenceNumber = m_AvailableSequenceNumbers.begin(); iteratorAvailableSequenceNumber != m_AvailableSequenceNumbers.end(); iteratorAvailableSequenceNumber++)
	{
		CUADataChangeNotification* pDataChangeNotification = iteratorAvailableSequenceNumber->second;
		if (pDataChangeNotification)
		{
			uStatus = pDataChangeNotification->RemoveMonitoredItemNotification(uiClientHandle);
			if ((pDataChangeNotification->GetNoOfMonitoredItemNotification() == 0))
			{
				if (pDataChangeNotification->IsAcked()) 
					delete pDataChangeNotification;	
				else
					tmpAvailableSequenceNumbers.insert(std::pair<OpcUa_UInt32,CUADataChangeNotification*>(iteratorAvailableSequenceNumber->first, iteratorAvailableSequenceNumber->second));
			}
			else
				tmpAvailableSequenceNumbers.insert(std::pair<OpcUa_UInt32, CUADataChangeNotification*>(iteratorAvailableSequenceNumber->first, iteratorAvailableSequenceNumber->second));
		}
	}
	m_AvailableSequenceNumbers.clear();
	if (tmpAvailableSequenceNumbers.size() > 0)
	{
		m_AvailableSequenceNumbers.swap(tmpAvailableSequenceNumbers);
		tmpAvailableSequenceNumbers.clear();
	}
	OpcUa_Mutex_Unlock(m_AvailableSequenceNumbersMutex);

	return uStatus;
}
OpcUa_StatusCode CSubscriptionServer::DeleteAllDataChangeNotfication()
{
	OpcUa_StatusCode uStatus = OpcUa_BadNothingToDo;
	OpcUa_Mutex_Lock(m_AvailableSequenceNumbersMutex); //m_hDataChangeNotificationListMutex

	CAvailableSequenceNumbers::iterator it = m_AvailableSequenceNumbers.begin();
	for (it = m_AvailableSequenceNumbers.begin(); it != m_AvailableSequenceNumbers.end();it++)	
	{
		CUADataChangeNotification* pDataChangeNotification = it->second;
		if (pDataChangeNotification)
		{
			delete pDataChangeNotification;
			pDataChangeNotification = OpcUa_Null;
		}
	}
	m_AvailableSequenceNumbers.clear();
	OpcUa_Mutex_Unlock(m_AvailableSequenceNumbersMutex);

	return uStatus;
}
OpcUa_StatusCode CSubscriptionServer::DeleteMonitoredItemById(OpcUa_UInt32 Id)
{
	OpcUa_StatusCode uStatus = OpcUa_Bad;
	CMonitoredItemServer* pMonitoredItemServer = OpcUa_Null;
	OpcUa_Mutex_Lock(m_hMonitoredItemListMutex);

	uStatus = FindMonitoredItemById(Id, &pMonitoredItemServer);
	if (uStatus == OpcUa_Good)
	{
		OpcUa_Semaphore_TimedWait(m_MonitoredItemListSem, OPC_TIMEOUT * 2); // OPC_TIMEOUT 7500 - Infinite =OPCUA_INFINITE
		///////////////////////////////////////////////////////////////////////////////////////////////////
		CUABase* pBase = pMonitoredItemServer->GetUABase();
		if (pBase)
		{
			if (pBase->GetNodeClass() == OpcUa_NodeClass_Variable)
			{
				CUAVariable* pUAVariable = (CUAVariable*)pBase;
				CVpiTag* pSignal = (CVpiTag*)(pUAVariable->GetPData());
				if (pSignal)
				{
					CVpiDevice* pDevice = pSignal->GetDevice();
					if (pDevice)
					{
						OpcUa_Double fastestSamplingInterval = GetFastestSamplingInterval();
						pDevice->SetSamplingInterval(fastestSamplingInterval);
					}
				}
			}
		}
		///////////////////////////////////////////////////////////////////////////////////////////////////
		CMonitoredItemServerList* pMonitoredItemServerList = GetMonitoredItemList();
		if (pMonitoredItemServerList)
		{
			CMonitoredItemServerList::iterator it;
			//for (OpcUa_UInt32 i = 0; i < pMonitoredItemServerList->size(); i++)
			for (it = pMonitoredItemServerList->begin(); it != pMonitoredItemServerList->end(); it++)
			{
				CMonitoredItemServer* pTmpMonitoredItemServer =*it;
				if (pTmpMonitoredItemServer->GetMonitoredItemId() == pMonitoredItemServer->GetMonitoredItemId())
				{
					DeleteDataChangeNotfication(pMonitoredItemServer->GetClientHandle());

					delete pTmpMonitoredItemServer;
					pTmpMonitoredItemServer = OpcUa_Null;
					pMonitoredItemServerList->erase(it);//
					uStatus = OpcUa_Good;
					break;
				}
			}
		}
	}

	OpcUa_Mutex_Unlock(m_hMonitoredItemListMutex);
	OpcUa_Semaphore_Post(m_MonitoredItemListSem, 1);
	return uStatus;
}

// This method will Acked DataChangeNotification with the m_uiSequenceNumber = uiSequence
OpcUa_StatusCode CSubscriptionServer::AckDataChangeNotification(OpcUa_UInt32 uiSequence)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_Boolean bFoundSequenceNumber = OpcUa_False;
	if (uiSequence > 0)
	{
		///////////////////////////////////////////////////////////////////////////////////////////////////
		// We ack notification message before stocking them
		OpcUa_Mutex_Lock(m_hDataChangeNotificationListMutex);
		OpcUa_Mutex_Lock(m_AvailableSequenceNumbersMutex);

		CAvailableSequenceNumbers::iterator iteratorAvailableSequenceNumber;
		iteratorAvailableSequenceNumber = m_AvailableSequenceNumbers.find(uiSequence);
		CUADataChangeNotification* pDataChangeNotification = OpcUa_Null;
		if (iteratorAvailableSequenceNumber != m_AvailableSequenceNumbers.end())
		{
			bFoundSequenceNumber = OpcUa_True;
			pDataChangeNotification = iteratorAvailableSequenceNumber->second;
		}
		if (pDataChangeNotification)
		{
			// we need to put in a new CUDataChangeNotification all in unsent OpcUa_MonitoredItemNotification (m_pMonitoredItemNotification)
			if (pDataChangeNotification->RemoveSentMonitoredItemNotification())
			{
				pDataChangeNotification->Acked();
			}
		}
		else
		{
			if (bFoundSequenceNumber)
				m_AvailableSequenceNumbers.erase(iteratorAvailableSequenceNumber);
		}
		//CUADataChangeNotificationList*	pDataChangeNotificationList = GetDataChangeNotificationList();// AckDataChangeNotification
		//if (!pDataChangeNotificationList->empty())
		//{
		//	for (OpcUa_UInt32 iii = 0; iii < pDataChangeNotificationList->size(); iii++)
		//	{				
		//		CUADataChangeNotification* pDataChangeNotification = pDataChangeNotificationList->at(iii);
		//		if (!pDataChangeNotification->IsAcked())
		//		{
		//			if (pDataChangeNotification->GetSequenceNumber() == uiSequence)
		//			{
		//				bFoundSequenceNumber = OpcUa_True;
		//				// we need to put in a new CUDataChangeNotification all in unsent OpcUa_MonitoredItemNotification (m_pMonitoredItemNotification)
		//				if (pDataChangeNotification->RemoveSentMonitoredItemNotification())
		//				{
		//					pDataChangeNotification->Acked();
		//					SetDataChangeNotificationCurrenPos(iii);
		//				}
		//				else
		//				{
		//					OpcUa_UInt32 uiCurrentSequenceNumber = GetCurrentSequenceNumber();
		//					pDataChangeNotification->SetSequenceNumber(uiCurrentSequenceNumber);
		//					//SetDataChangeNotificationCurrenPos(iii);
		//				}
		//			}
		//		}
		//	}
		//}

		OpcUa_Mutex_Unlock(m_AvailableSequenceNumbersMutex);
		OpcUa_Mutex_Unlock(m_hDataChangeNotificationListMutex);
		/////////////////////////////////////////////////////////////////////////////////////////////////
		// !We will do the same with EventNotifications
		OpcUa_Mutex_Lock(m_hEventNotificationListListMutex);
		CUAEventNotificationListList*	pEventNotificationListList = GetEventNotificationListList();// 
		if (!pEventNotificationListList->empty())
		{
			for (OpcUa_UInt32 iii = 0; iii < pEventNotificationListList->size(); iii++)
			{
				CUAEventNotificationList* pEventNotificationList = pEventNotificationListList->at(iii);
				if (!pEventNotificationList->IsTransactionAcked())
				{
					if (pEventNotificationList->GetSequenceNumber() == uiSequence)
					{
						// on acquite celui-ci
						bFoundSequenceNumber = OpcUa_True;
						pEventNotificationList->TransactionAcked(OpcUa_True);
						SetEventNotificationListListCurrentPos(iii);
					}
				}
			}
		}
		OpcUa_Mutex_Unlock(m_hEventNotificationListListMutex);
		// Do the same for StatusChangeNotification

		OpcUa_Mutex_Lock(m_hStatusChangeNotificationListMutex);
		CUAStatusChangeNotificationList* pUAStatusChangeNotificationList = GetStatusChangeNotificationList();
		if (!pUAStatusChangeNotificationList->empty())
		{
			for (OpcUa_UInt32 iii = 0; iii < pUAStatusChangeNotificationList->size(); iii++)
			{
				CUAStatusChangeNotification* pUAStatusChangeNotification = pUAStatusChangeNotificationList->at(iii);
				if (!pUAStatusChangeNotification->IsAcked())
				{
					if (pUAStatusChangeNotification->GetSequenceNumber() == uiSequence)
					{
						// on acquite celui-ci
						bFoundSequenceNumber = OpcUa_True;
						pUAStatusChangeNotification->Acked();
						SetStatusChangeNotificationCurrenPos(iii);
					}
				}
			}
		}
		OpcUa_Mutex_Unlock(m_hStatusChangeNotificationListMutex);
		// Double check that all list are empty (CTT check)
		if (!bFoundSequenceNumber)
		{
			OpcUa_Mutex_Lock(m_AvailableSequenceNumbersMutex);
			OpcUa_Mutex_Lock(m_hEventNotificationListListMutex);
			OpcUa_Mutex_Lock(m_hStatusChangeNotificationListMutex);
			if ((m_AvailableSequenceNumbers.size() == 0) && (pUAStatusChangeNotificationList->empty()) && (pEventNotificationListList->empty()))
				bFoundSequenceNumber = OpcUa_True;
			OpcUa_Mutex_Unlock(m_hStatusChangeNotificationListMutex);
			OpcUa_Mutex_Unlock(m_hEventNotificationListListMutex);
			OpcUa_Mutex_Unlock(m_AvailableSequenceNumbersMutex);
		}
		if (!bFoundSequenceNumber)
		{
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "uiSequence %u not found on SubscriptionId: %u \n", uiSequence,GetSubscriptionId());
			uStatus = OpcUa_BadSequenceNumberUnknown;
		}
		// Si on est en retard on reveil la UpdateNotificationThread immediatement
		//if ((isInLate()) && (!IsInColdState()))
		//{
		//	OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_WARNING, "In Late and not incoldstate>Will wakeup the subscriptionThread for immediate notification\n");
		//	WakeupUpdateSubscriptionThread();
		//}
	}
	else
		uStatus = OpcUa_BadSequenceNumberInvalid;
	return uStatus;
}
OpcUa_StatusCode CSubscriptionServer::UpdateSubscriptionDiagnosticsDataType()
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	CUAInformationModel* pInformationModel = CServerApplication::m_pTheAddressSpace;
	CSessionServer* pSession = GetSession();
	if ((pSession) && (m_pSubscriptionDiagnosticsDataType) )
	{
		if (m_pSubscriptionDiagnosticsDataType->GetInternalSubscriptionDiagnosticsDataType())
		{
			OpcUa_NodeId aNodeId = pSession->GetSessionId();
			m_pSubscriptionDiagnosticsDataType->SetSessionId(aNodeId);
			OpcUa_UInt32 uiSubscriptionId = GetSubscriptionId();
			m_pSubscriptionDiagnosticsDataType->SetSubscriptionId(uiSubscriptionId);
			OpcUa_UInt32 uiMaxKeepAliveCount = GetMaxKeepAliveCount();
			m_pSubscriptionDiagnosticsDataType->SetMaxLifetimeCount(uiMaxKeepAliveCount);
			OpcUa_UInt32 uiLifetimeCount = GetLifetimeCount();
			m_pSubscriptionDiagnosticsDataType->SetMaxLifetimeCount(uiLifetimeCount);
			OpcUa_Byte bPriority = GetPriority();
			m_pSubscriptionDiagnosticsDataType->SetPriority(bPriority);
			OpcUa_Boolean boolVal = GetPublishingEnabled();
			m_pSubscriptionDiagnosticsDataType->SetPublishingEnabled(boolVal);
			OpcUa_Double dblVal = GetPublishingInterval();
			m_pSubscriptionDiagnosticsDataType->SetPublishingInterval(dblVal);
			OpcUa_UInt32 uiMaxNotificationsPerPublish=GetMaxNotificationsPerPublish();
			m_pSubscriptionDiagnosticsDataType->SetMaxNotificationsPerPublish(uiMaxNotificationsPerPublish);
			OpcUa_UInt32 uiPublishRequestsSize=m_pSession->GetPublishRequestsSize();
			m_pSubscriptionDiagnosticsDataType->SetPublishRequestCount(uiPublishRequestsSize);
			if (pInformationModel->UpdateSubscriptionDiagnosticsValueInAddressSpace(m_pSubscriptionDiagnosticsDataType) != OpcUa_Good)
				OpcUa_Trace(OPCUA_TRACE_SERVER_ERROR, "UpdateSubscriptionDiagnosticsDataType failed");
		}
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}

OpcUa_StatusCode CSubscriptionServer::ModifySubscription(
	OpcUa_Double               a_nRequestedPublishingInterval,
	OpcUa_UInt32               a_nRequestedLifetimeCount,
	OpcUa_UInt32               a_nRequestedMaxKeepAliveCount,
	OpcUa_UInt32               a_nMaxNotificationsPerPublish,
	OpcUa_Byte                 a_nPriority,
	OpcUa_Double*              a_pRevisedPublishingInterval,
	OpcUa_UInt32*              a_pRevisedLifetimeCount,
	OpcUa_UInt32*              a_pRevisedMaxKeepAliveCount)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_Mutex_Lock(m_hMonitoredItemListMutex);
	OpcUa_Mutex_Lock(m_hDataChangeNotificationListMutex);
	// PublishingInterval
	// controle de la valeur reçu sur a_nRequestedPublishingInterval
#ifdef WIN32
	if (_isnan(a_nRequestedPublishingInterval))
#endif
#ifdef _GNUC_
		if (isnan(a_nRequestedPublishingInterval))
#endif
			SetPublishingInterval(PUBLISHING_INTERVAL_MINI);
		else
		{
			if (a_nRequestedPublishingInterval == 0) // 
				SetPublishingInterval(PUBLISHING_INTERVAL_MINI);
			else
			{
				// Verification que l'intervalle de publication demandé n'est pas inferieur a PUBLISHING_INTERVAL_MINI
				if (a_nRequestedPublishingInterval < PUBLISHING_INTERVAL_MINI) // 
					SetPublishingInterval(PUBLISHING_INTERVAL_MINI);
				else
				{
					if (a_nRequestedPublishingInterval>0xFFFFFFFF) // 49 jours max
						SetPublishingInterval(0xFFFFFFFF);
					else
						SetPublishingInterval(a_nRequestedPublishingInterval);
				}
			}
		}
	*a_pRevisedPublishingInterval = GetPublishingInterval();
	// MaxKeepAliveCount
	if (a_nRequestedMaxKeepAliveCount == 0)
		SetMaxKeepAliveCount(MAX_KEEPALIVE_COUNT_MINI);
	else
	{
		if (a_nRequestedMaxKeepAliveCount > MAX_KEEPALIVE_COUNT_MAX)
			SetMaxKeepAliveCount(MAX_KEEPALIVE_COUNT_MAX);
		else
			SetMaxKeepAliveCount(a_nRequestedMaxKeepAliveCount);
	}
	*a_pRevisedMaxKeepAliveCount = GetMaxKeepAliveCount();

	// LifetimeCount
	if (a_nRequestedLifetimeCount == 0)
		SetLifetimeCount((*a_pRevisedMaxKeepAliveCount) * 3); // SetLifetimeCount(MAX_KEEPALIVE_COUNT_MINI);
	else
	{
		if (((*a_pRevisedMaxKeepAliveCount) * 3) <= (a_nRequestedLifetimeCount))
			SetLifetimeCount(a_nRequestedLifetimeCount);
		else
		{
			if ((((*a_pRevisedMaxKeepAliveCount) * 3) < a_nRequestedLifetimeCount) && (((*a_pRevisedMaxKeepAliveCount) * 3) < MAX_KEEPALIVE_COUNT_MAX))
				SetLifetimeCount(a_nRequestedLifetimeCount);
			else
				SetLifetimeCount((*a_pRevisedMaxKeepAliveCount) * 3);
		}
	}
	*a_pRevisedLifetimeCount = GetLifetimeCount();
	// MaxNotificationsPerPublish
	SetMaxNotificationsPerPublish(a_nMaxNotificationsPerPublish);
	// Priority
	SetPriority(a_nPriority);
	if (uStatus == OpcUa_Good)
	{
		if (m_pSubscriptionDiagnosticsDataType)
		{
			OpcUa_SubscriptionDiagnosticsDataType* pSubscriptionDiagnosticsDataType = m_pSubscriptionDiagnosticsDataType->GetInternalSubscriptionDiagnosticsDataType();
			if (pSubscriptionDiagnosticsDataType)
			{
				pSubscriptionDiagnosticsDataType->ModifyCount++;
			}
		}
	}
	OpcUa_Mutex_Unlock(m_hDataChangeNotificationListMutex);
	OpcUa_Mutex_Unlock(m_hMonitoredItemListMutex);
	OpcUa_Semaphore_Post(m_MonitoredItemListSem, 1);
	return uStatus;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Republishes. </summary>
///
/// <remarks>	Michel, 01/02/2016. </remarks>
///
/// <param name="a_nRetransmitSequenceNumber">	The retransmit sequence number. </param>
/// <param name="a_pNotificationMessage">	  	[in,out] If non-null, message describing the
/// 											notification.
/// </param>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode CSubscriptionServer::Republish(OpcUa_UInt32 a_nRetransmitSequenceNumber,OpcUa_NotificationMessage** a_pNotificationMessage)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	(*a_pNotificationMessage)->SequenceNumber = a_nRetransmitSequenceNumber;
	OpcUa_DateTime_Initialize(&((*a_pNotificationMessage)->PublishTime));
	OpcUa_Boolean bFoundSequenceNumber = OpcUa_False;
	OpcUa_Mutex_Lock(m_hDataChangeNotificationListMutex);
	OpcUa_Mutex_Lock(m_AvailableSequenceNumbersMutex);
	vector<OpcUa_MonitoredItemNotification*> aMonitoredItemNotification;
	CAvailableSequenceNumbers::iterator iteratorAvailableSequenceNumber;
	iteratorAvailableSequenceNumber = m_AvailableSequenceNumbers.find(a_nRetransmitSequenceNumber);
	CUADataChangeNotification* pDataChangeNotification = OpcUa_Null;
	if (iteratorAvailableSequenceNumber != m_AvailableSequenceNumbers.end())
		pDataChangeNotification = iteratorAvailableSequenceNumber->second;
	if (pDataChangeNotification)
	//CUADataChangeNotificationList* pDataChangeNotificationList = GetDataChangeNotificationList(); // Republish
	//for (OpcUa_UInt32 i = 0; i < pDataChangeNotificationList->size(); i++)
	//{
	//	CUADataChangeNotification* pDataChangeNotification = pDataChangeNotificationList->at(i);
	//	if (pDataChangeNotification->GetSequenceNumber() == a_nRetransmitSequenceNumber)
		{
			if (!pDataChangeNotification->IsAcked())
			{
				bFoundSequenceNumber = OpcUa_True;
				if (((*a_pNotificationMessage)->PublishTime.dwHighDateTime == 0) && ((*a_pNotificationMessage)->PublishTime.dwLowDateTime == 0))
					(*a_pNotificationMessage)->PublishTime = pDataChangeNotification->GetPublishTime();
				CUAMonitoredItemNotificationList aUAMonitoredItemNotificationList = pDataChangeNotification->GetUAMonitoredItemNotificationList();
				for (OpcUa_UInt32 ii = 0; ii < aUAMonitoredItemNotificationList.size(); ii++)
				{
					CUAMonitoredItemNotification* pUAMonitoredItemNotification = aUAMonitoredItemNotificationList.at(ii);
					if (pUAMonitoredItemNotification)
					{
						OpcUa_MonitoredItemNotification* pMonitoredItemNotification = pUAMonitoredItemNotification->GetMonitoredItemNotification();
						if (pMonitoredItemNotification)
							aMonitoredItemNotification.push_back(pMonitoredItemNotification);
						// le code ci-dessous cause un 0x80060000... Il faut activer ce code afin de debugger ce cas qui met le serveur en deadlock
						//for (OpcUa_Int32 iii=0;iii<iNbOfMonitoredItems;iii++)
						//{
						//	aMonitoredItemNotification.push_back(&pMonitoredItemNotification[iii]);
						//}
					}
				}
			}
		}
	//}
	OpcUa_Mutex_Unlock(m_AvailableSequenceNumbersMutex);
	OpcUa_Mutex_Unlock(m_hDataChangeNotificationListMutex);
	if (bFoundSequenceNumber)
	{
		(*a_pNotificationMessage)->NotificationData = (OpcUa_ExtensionObject*)OpcUa_Alloc(sizeof(OpcUa_ExtensionObject));
		OpcUa_DataChangeNotification* pNotification = (OpcUa_DataChangeNotification*)OpcUa_Alloc(sizeof(OpcUa_DataChangeNotification));
		OpcUa_UInt32 uiMaxNotificationsPerPublish = aMonitoredItemNotification.size();
		pNotification->MonitoredItems =
			(OpcUa_MonitoredItemNotification*)OpcUa_Alloc(uiMaxNotificationsPerPublish*sizeof(OpcUa_MonitoredItemNotification));
		// remplissage
		for (OpcUa_UInt32 ii = 0; ii < aMonitoredItemNotification.size(); ii++)
		{
			OpcUa_MonitoredItemNotification_Initialize(&(pNotification->MonitoredItems[ii]));
			OpcUa_MonitoredItemNotification* pMonitoredItemNotification = aMonitoredItemNotification.at(ii);
			OpcUa_MonitoredItemNotification_Initialize(&pNotification->MonitoredItems[ii]);
			pNotification->MonitoredItems[ii].ClientHandle = pMonitoredItemNotification->ClientHandle;
			OpcUa_DataValue_CopyTo(&(pMonitoredItemNotification->Value), &(pNotification->MonitoredItems[ii].Value));
		}
		pNotification->NoOfMonitoredItems = uiMaxNotificationsPerPublish;
		aMonitoredItemNotification.clear();
		// DiagnosticInfos
		pNotification->NoOfDiagnosticInfos = 0;
		pNotification->DiagnosticInfos = OpcUa_Null; // (OpcUa_DiagnosticInfo*)OpcUa_Alloc(sizeof(OpcUa_DiagnosticInfo));
		//ZeroMemory((pNotification->DiagnosticInfos), sizeof(OpcUa_DiagnosticInfo));
		////////////////////////////////////////////////////////////////////////////////////////////
		// Mise en forme du résultat
		OpcUa_ExtensionObject_Initialize(&((*a_pNotificationMessage)->NotificationData[0]));
		//NodeId
		(*a_pNotificationMessage)->NotificationData[0].TypeId.NodeId.Identifier.Numeric = OpcUaId_DataChangeNotification_Encoding_DefaultBinary;
		(*a_pNotificationMessage)->NotificationData[0].TypeId.NodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
		// Encoding
		(*a_pNotificationMessage)->NotificationData[0].Encoding = OpcUa_ExtensionObjectEncoding_EncodeableObject;
		(*a_pNotificationMessage)->NotificationData[0].Body.EncodeableObject.Type = &OpcUa_DataChangeNotification_EncodeableType;
		// Embedded object
		(*a_pNotificationMessage)->NotificationData[0].Body.EncodeableObject.Object = pNotification;
		(*a_pNotificationMessage)->NoOfNotificationData = 1;
	}
	else
		uStatus = OpcUa_BadMessageNotAvailable;
	return uStatus;
}
// Remove All DataChangeNotification Acked on this subscription
// return OpcUa_Good if succeed
OpcUa_StatusCode CSubscriptionServer::RemoveAckedDataChangeNotification()
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_Mutex_Lock(m_hDataChangeNotificationListMutex);
	OpcUa_Mutex_Lock(m_AvailableSequenceNumbersMutex);
	CAvailableSequenceNumbers tmpSequenceNumber;
	CAvailableSequenceNumbers::iterator iteratorAvailableSequenceNumber;
	for (iteratorAvailableSequenceNumber = m_AvailableSequenceNumbers.begin(); iteratorAvailableSequenceNumber != m_AvailableSequenceNumbers.end(); iteratorAvailableSequenceNumber++)
	{
		CUADataChangeNotification* pUADataChangeNotification = iteratorAvailableSequenceNumber->second;
		if (pUADataChangeNotification->IsAcked())
		{
			delete pUADataChangeNotification;
		}
		else
		{
			OpcUa_UInt32 uiSequenceNumber = iteratorAvailableSequenceNumber->first;
			tmpSequenceNumber.insert(std::pair<OpcUa_UInt32, CUADataChangeNotification*>(uiSequenceNumber, pUADataChangeNotification));
		}
	}
	m_AvailableSequenceNumbers.clear();
	if (tmpSequenceNumber.size() > 0)
	{
		m_AvailableSequenceNumbers.swap(tmpSequenceNumber);
		tmpSequenceNumber.clear();
	}
	// Update the current position in the m_pDataChangeNotificationList
	OpcUa_Mutex_Unlock(m_AvailableSequenceNumbersMutex);
	SetDataChangeNotificationCurrenPos(0);
	OpcUa_Mutex_Unlock(m_hDataChangeNotificationListMutex);
	return uStatus;
}
// Remove All DataChangeNotification Acked on this subscription
// return OpcUa_Good if succeed
OpcUa_StatusCode CSubscriptionServer::RemoveAckedStatusChangeNotification()
{

	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_Mutex_Lock(m_hStatusChangeNotificationListMutex);
	if (m_pStatusChangeNotificationList)
	{
		CUAStatusChangeNotificationList::iterator it = m_pStatusChangeNotificationList->begin();
		while (it != m_pStatusChangeNotificationList->end())
		{
			if ((*it)->IsAcked())
			{
				delete *it;
				it = m_pStatusChangeNotificationList->erase(it);
			}
			else
				++it;
		}
	}
	else
		uStatus = OpcUa_BadInternalError;
	// Update the current position in the m_pDataChangeNotificationList
	SetStatusChangeNotificationCurrenPos(0);
	OpcUa_Mutex_Unlock(m_hStatusChangeNotificationListMutex);
	return uStatus;
}
// Remove All CUAEventNotificationList Acked on this subscription. CUAEventNotificationList This are in m_pEventNotificationListList
// return OpcUa_Good if succeed
OpcUa_StatusCode CSubscriptionServer::RemoveAckedEventNotificationList()
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_Mutex_Lock(m_hEventNotificationListListMutex);
	if (m_pEventNotificationListList)
	{	
		CUAEventNotificationListList::iterator it = m_pEventNotificationListList->begin();
		while (it != m_pEventNotificationListList->end())
		{
			if ((*it)->IsTransactionAcked() && ((*it)->IsAlarmAcked()))
			{
				delete *it;
				it = m_pEventNotificationListList->erase(it);
			}
			else
				++it;
		}
	}
	else
		uStatus = OpcUa_BadInternalError;
	// Update the current position in the m_pDataChangeNotificationList
	SetEventNotificationListListCurrentPos(0);
	OpcUa_Mutex_Unlock(m_hEventNotificationListListMutex);
	return uStatus;
}
OpcUa_StatusCode CSubscriptionServer::RemoveStatusChangeNotification(CUAStatusChangeNotification* pStatusChangeNotification)
{
	OpcUa_StatusCode uStatus = OpcUa_BadNotFound;
	OpcUa_Mutex_Lock(m_hStatusChangeNotificationListMutex);
	if (!pStatusChangeNotification)
		uStatus = OpcUa_BadInvalidArgument;
	else
	{
		CUAStatusChangeNotificationList*	pStatusChangeNotificationList = GetStatusChangeNotificationList();
		if (pStatusChangeNotificationList)
		{
			OpcUa_UInt32 ii = 0;
			CUAStatusChangeNotificationList::iterator it = pStatusChangeNotificationList->begin();
			for (ii = 0; ii < pStatusChangeNotificationList->size(); ii++)
			{
				CUAStatusChangeNotification* pTmpStatusChangeNotification = pStatusChangeNotificationList->at(ii);
				if (pTmpStatusChangeNotification == pStatusChangeNotification)
				{
					delete pStatusChangeNotification;
					pStatusChangeNotificationList->erase(it);
					uStatus = OpcUa_Good;
					break;
				}
				it++;
			}
		}
		else
			uStatus = OpcUa_BadInvalidArgument;
	}
	OpcUa_Mutex_Unlock(m_hStatusChangeNotificationListMutex);
	return uStatus;
}
OpcUa_StatusCode CSubscriptionServer::RemoveEventNotificationList(CUAEventNotificationList* pEventNotificationList)
{
	OpcUa_StatusCode uStatus = OpcUa_BadNotFound;
	OpcUa_Mutex_Lock(m_hEventNotificationListListMutex);
	if (!pEventNotificationList)
		uStatus = OpcUa_BadInvalidArgument;
	CUAEventNotificationListList::iterator it;
	for (it = m_pEventNotificationListList->begin(); it != m_pEventNotificationListList->end(); it++)
	{
		CUAEventNotificationList* pTmpEventNotificationList = *it;
		if (pEventNotificationList == pTmpEventNotificationList)
		{
			delete pEventNotificationList;
			m_pEventNotificationListList->erase(it);
			uStatus = OpcUa_Good;
		}
	}
	OpcUa_Mutex_Unlock(m_hEventNotificationListListMutex);
	return uStatus;
}
//	SystemStatusChangeEventType
// We will notify the client subscription of a change in the subscription state
// The new state is in uNewStatus
OpcUa_StatusCode CSubscriptionServer::AddStatusChangeNotificationMessage(OpcUa_DiagnosticInfo aDiagInfo, OpcUa_StatusCode uNewStatus)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	///////////////////////////////////////////////////////////////////////////
	// On va fabriquer un StatusChangeNotification
	OpcUa_UInt32 uiCurrentSequenceNumber = GetCurrentSequenceNumber();
	OpcUa_UInt32 uiMaxNotificationsPerPublish = GetMaxNotificationsPerPublish();
	// On va indiquer au client que l'on est en situation de keepalive
	CUAStatusChangeNotification* pStatusChangeNotification = new CUAStatusChangeNotification(uiCurrentSequenceNumber);

	pStatusChangeNotification->SetDiagnosticInfo(aDiagInfo);
	pStatusChangeNotification->SetStatusCode(uNewStatus);
	OpcUa_Mutex_Lock(m_hStatusChangeNotificationListMutex);
	CUAStatusChangeNotificationList*	pStatusChangeNotificationList = GetStatusChangeNotificationList();
	if (pStatusChangeNotificationList)
	{
		if ((uiMaxNotificationsPerPublish >= pStatusChangeNotificationList->size()) || (uiMaxNotificationsPerPublish == 0))
			pStatusChangeNotificationList->push_back(pStatusChangeNotification);
		else
			delete pStatusChangeNotification;
	}
	else
		delete pStatusChangeNotification;
	OpcUa_Mutex_Unlock(m_hStatusChangeNotificationListMutex);
	///////////////////////////////////////////////////////////////////////////
	return uStatus;
}
// 
/// <summary>
/// Look for the CMonitoredItemServer associated to a ClientHandle
/// </summary>
/// <param name="uiClientHandle">The client handle in a  OpcUa_UInt32.</param>
/// <param name="pItem">Out the item in a CMonitoredItemServer**.</param>
/// <returns>OpcUa_StatusCode</returns>
OpcUa_StatusCode CSubscriptionServer::FindMonitoredItemByClientHandle(OpcUa_UInt32 uiClientHandle, CMonitoredItemServer** pItem)
{
	OpcUa_StatusCode uStatus = OpcUa_BadInvalidArgument;
	OpcUa_Mutex_Lock(m_hMonitoredItemListMutex);
	for (OpcUa_UInt32 ii = 0; ii < m_pMonitoredItemList->size(); ii++)
	{
		CMonitoredItemServer* pLocalItem = m_pMonitoredItemList->at(ii);
		if (pLocalItem->GetClientHandle() == uiClientHandle)
		{
			*pItem = pLocalItem;
			uStatus = OpcUa_Good;
			break;
		}
	}
	OpcUa_Mutex_Unlock(m_hMonitoredItemListMutex);
	OpcUa_Semaphore_Post(m_MonitoredItemListSem, 1);
	return uStatus;
}
// Method used in UpdateDataSubscriptionThread to compare cache and MonitoredItem value for UAObject
// Return : OpcUa_Good is the value are different and could be copy in the DataChangeNotificationList
//          OpcUa_BadNoMatch if the the value are equal and will not be copy in the DataChangeNotificationList
//			OpcUa_BadInvalidArgument if a incorrect attribute was requested to compare
OpcUa_StatusCode CSubscriptionServer::CompareMonitoredItemToUAObject(CUADataChangeNotification** pDataChangeNotification, CUABase* pUABase, CMonitoredItemServer* pMonitoredItem, OpcUa_Boolean* pbPreIncKeepAliveCounter)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_Boolean bReady = OpcUa_False;
	OpcUa_Variant aVariant;
	OpcUa_Variant_Initialize(&aVariant);
	OpcUa_UInt32 uiAttributeId = pMonitoredItem->GetAttributeId();
	switch (uiAttributeId)
	{
	case OpcUa_Attributes_NodeId:
	{
		aVariant.Datatype = OpcUaType_NodeId;
		OpcUa_NodeId* pNodeId1 = pUABase->GetNodeId();
		aVariant.ArrayType = OpcUa_VariantArrayType_Scalar;
		OpcUa_NodeId_CopyTo(pNodeId1, aVariant.Value.NodeId);
		bReady = OpcUa_True;
	}
	break;
	case OpcUa_Attributes_NodeClass:
		aVariant.Datatype = OpcUaType_UInt32;
		aVariant.ArrayType = OpcUa_VariantArrayType_Scalar;
		aVariant.Value.UInt32 = pUABase->GetNodeClass();
		bReady = OpcUa_True;
		break;
	case OpcUa_Attributes_BrowseName:
		aVariant.Datatype = OpcUaType_QualifiedName;
		aVariant.ArrayType = OpcUa_VariantArrayType_Scalar;
		aVariant.Value.QualifiedName = pUABase->GetBrowseName();
		bReady = OpcUa_True;
		break;
	case OpcUa_Attributes_DisplayName:
	{
		aVariant.Datatype = OpcUaType_LocalizedText;
		aVariant.ArrayType = OpcUa_VariantArrayType_Scalar;
		OpcUa_LocalizedText aLocalizedText = pUABase->GetDisplayName();
		if (aVariant.Value.LocalizedText == OpcUa_Null)
			aVariant.Value.LocalizedText = (OpcUa_LocalizedText*)OpcUa_Alloc(sizeof(OpcUa_LocalizedText));
		OpcUa_LocalizedText_Initialize(aVariant.Value.LocalizedText);
		OpcUa_LocalizedText_CopyTo(&aLocalizedText, aVariant.Value.LocalizedText);
		bReady = OpcUa_True;
	}
	break;
	case OpcUa_Attributes_Description:
	{
		aVariant.Datatype = OpcUaType_LocalizedText;
		aVariant.ArrayType = OpcUa_VariantArrayType_Scalar;
		OpcUa_LocalizedText aLocalizedText = pUABase->GetDescription();
		OpcUa_LocalizedText_CopyTo(&aLocalizedText, aVariant.Value.LocalizedText);
		bReady = OpcUa_True;
	}
	break;
	case OpcUa_Attributes_WriteMask:
		aVariant.Datatype = OpcUaType_UInt32;
		aVariant.ArrayType = OpcUa_VariantArrayType_Scalar;
		aVariant.Value.UInt32 = pUABase->GetWriteMask();
		bReady = OpcUa_True;
		break;
	case OpcUa_Attributes_UserWriteMask:
		aVariant.Datatype = OpcUaType_UInt32;
		aVariant.ArrayType = OpcUa_VariantArrayType_Scalar;
		aVariant.Value.UInt32 = pUABase->GetUserWriteMask();
		bReady = OpcUa_True;
		break;
	default:
		break;
	}
	if (bReady)
	{
		if (IsMonitoredItemFromUABaseEqual(pUABase, pMonitoredItem) == OpcUa_False)
		{
			(*pbPreIncKeepAliveCounter) = OpcUa_False;
			uStatus = OpcUa_Good;
			// creation d'un CUADataChangeNotification qui contiendra le OpcUa_MonitoredItemNotification
			if (!(*pDataChangeNotification))
			{
				OpcUa_UInt32 uiCurrentSequenceNumber = GetCurrentSequenceNumber();
				(*pDataChangeNotification) = new CUADataChangeNotification(uiCurrentSequenceNumber);
				(*pDataChangeNotification)->SetDataChangeNotificationType(NOTIFICATION_MESSAGE_DATACHANGE);
				//AvailableSequenceNumberAdd(uiCurrentSequenceNumber, (*pDataChangeNotification));
			}
			// allocation du OpcUa_MonitoredItemNotification
			OpcUa_MonitoredItemNotification* pVal = (OpcUa_MonitoredItemNotification*)OpcUa_Alloc(sizeof(OpcUa_MonitoredItemNotification));
			OpcUa_MonitoredItemNotification_Initialize(pVal);
			// Handle
			pVal->ClientHandle = pMonitoredItem->GetClientHandle();
			// horodate
			pVal->Value.ServerTimestamp = OpcUa_DateTime_UtcNow();
			pVal->Value.ServerPicoseconds = 0;
			pVal->Value.SourceTimestamp = OpcUa_DateTime_UtcNow();
			pVal->Value.SourcePicoseconds = 0;
			// StatusCode
			pVal->Value.StatusCode = OpcUa_Good;
			OpcUa_Variant_CopyTo(&aVariant, &(pVal->Value.Value)); // Warning with ExtensionObject copy
			// Transfert dans le CUADataChangeNotification (pDataChangeNotification)
			if ((*pDataChangeNotification)->AddMonitoredItemNotification(
				pVal,
				pMonitoredItem->GetQueueSize(),
				pMonitoredItem->IsDiscardOldest()) == OpcUa_GoodResultsMayBeIncomplete)// Found a UAObject to Notify
			{
				if (m_pSubscriptionDiagnosticsDataType)
				{
					m_pSubscriptionDiagnosticsDataType->SetMonitoringQueueOverflowCount(m_pSubscriptionDiagnosticsDataType->GetMonitoringQueueOverflowCount() + 1);
				}
			}
			// Modif mai 2015
			OpcUa_MonitoredItemNotification_Clear(pVal);
			OpcUa_Free(pVal);
			// Fin Modif mai 2015
		}
		else
			uStatus = OpcUa_BadNoMatch;
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	OpcUa_Variant_Clear(&aVariant);
	return uStatus;
}
// Method used from UpdateDataSubscriptionThread to compare cache and MonitoredItem value for UAVariable
// in this method if (*pDataChangeNotification) is null will create a new one.
// If we detect a difference between MonitoredItem and UAVariable we create a OpcUa_MonitoredItemNotification and we add it in (*pDataChangeNotification)
// by calling CUADataChangeNotification::AddMonitoredItemNotification
// Return : OpcUa_Good if the value are different and could be copy in the DataChangeNotificationList
//          OpcUa_BadNoMatch if the the value are equal and will not be copy in the DataChangeNotificationList
OpcUa_StatusCode CSubscriptionServer::CompareMonitoredItemToUAVariable(CUADataChangeNotification** pDataChangeNotification, 
	CUAVariable* pUAVariable, 
	CMonitoredItemServer* pMonitoredItem, 
	OpcUa_Boolean* pbPreIncKeepAliveCounter,
	OpcUa_Boolean* pbRecycledDataChangeNotification)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	if ((pUAVariable) && (pMonitoredItem) && (pbPreIncKeepAliveCounter))
	{
		OpcUa_String* pString = pMonitoredItem->GetIndexRange();
		if (OpcUa_String_StrLen(pString) > 0)
		{
			CNumericRange* pNumericRange = new CNumericRange(pString);
			if (pNumericRange)
			{
				CDataValue* pDataValue = pUAVariable->GetValue();
				if ((pDataValue->GetValue().ArrayType != OpcUa_VariantArrayType_Matrix) && (pNumericRange->IsMultiDimensional()))
				{
					pMonitoredItem->SetStatusCode(OpcUa_BadIndexRangeNoData);
				}
				if (pDataValue->GetValue().Value.Array.Length < pNumericRange->GetEndIndex())
				{
					pMonitoredItem->SetStatusCode(OpcUa_BadIndexRangeNoData);
				}
				if (pDataValue->GetValue().Value.Array.Length < pNumericRange->GetBeginIndex())
				{
					pMonitoredItem->SetStatusCode(OpcUa_BadIndexRangeNoData);
				}
				// new portion to check Range with Scalar
				if (pDataValue->GetValue().ArrayType == OpcUa_VariantArrayType_Scalar)
				{
					pMonitoredItem->SetStatusCode(OpcUa_BadIndexRangeNoData);
				}
				delete pNumericRange;
				pNumericRange = OpcUa_Null;
			}
			else
				uStatus = OpcUa_BadOutOfMemory;
		}
		if (uStatus == OpcUa_Good)
		{
			// We will compare the Value in cache witht the last sent value
			if (IsMonitoredItemFromUAVariableEqual(pUAVariable, pMonitoredItem) == OpcUa_False)
			{
				(*pbPreIncKeepAliveCounter) = OpcUa_False;
				uStatus = OpcUa_Good;
				// Check if we have to create a CUADataChangeNotification. It will contains a OpcUa_MonitoredItemNotification
				if (!(*pDataChangeNotification))
				{
					// need to find an existing unack pDataChangeNotification with unsent MonitoredItemNotification
					CAvailableSequenceNumbers::iterator iteratorAvailableSequenceNumber;
					for (iteratorAvailableSequenceNumber = m_AvailableSequenceNumbers.begin(); iteratorAvailableSequenceNumber != m_AvailableSequenceNumbers.end(); iteratorAvailableSequenceNumber++)
					{
						CUADataChangeNotification* pUADataChangeNotification = iteratorAvailableSequenceNumber->second;
						if (pUADataChangeNotification)
						{
							if (!pUADataChangeNotification->IsAcked())
							{
								if (pUADataChangeNotification->IsSomethingToNotify())
								{
									//OpcUa_UInt32 uiCurrentSequenceNumber = GetCurrentSequenceNumber();
									(*pDataChangeNotification) = pUADataChangeNotification;
									(*pbRecycledDataChangeNotification) = OpcUa_True;
									//AvailableSequenceNumberAdd(uiCurrentSequenceNumber, OpcUa_Null);
									break;
								}
							}
						}
					}
				}
				if (!(*pDataChangeNotification))
				{
					OpcUa_UInt32 uiCurrentSequenceNumber = GetCurrentSequenceNumber();
					(*pDataChangeNotification) = new CUADataChangeNotification(uiCurrentSequenceNumber); 
					(*pDataChangeNotification)->SetDataChangeNotificationType(NOTIFICATION_MESSAGE_DATACHANGE);
				}

				// Tag the new CUADataChangeNotification as a NOTIFICATION_MESSAGE_DATACHANGE
				(*pDataChangeNotification)->SetDataChangeNotificationType(NOTIFICATION_MESSAGE_DATACHANGE);

				OpcUa_Mutex pDataChangeNotificationMutex = (*pDataChangeNotification)->GetInternalDataChangeNotificationMutex();
				if (pDataChangeNotificationMutex)
				{
					OpcUa_Mutex_Lock(pDataChangeNotificationMutex);
					// Recupération de la valeur contenu dans la cache du serveur
					CDataValue* pDataValue = pUAVariable->GetValue();
					if (pDataValue)
					{
						// allocation du OpcUa_MonitoredItemNotification
						OpcUa_MonitoredItemNotification* pMonitoredItemNotification = OpcUa_Null;
						pMonitoredItemNotification = (OpcUa_MonitoredItemNotification*)OpcUa_Alloc(sizeof(OpcUa_MonitoredItemNotification));
						if (pMonitoredItemNotification)
						{
							OpcUa_MonitoredItemNotification_Initialize(pMonitoredItemNotification);
							if (pDataValue->GetArrayType() == OpcUa_VariantArrayType_Array)
								pMonitoredItemNotification->Value.StatusCode = OpcUa_BadIndexRangeNoData;
							else
								pMonitoredItemNotification->Value.StatusCode = OpcUa_Good;
							// Handle
							pMonitoredItemNotification->ClientHandle = pMonitoredItem->GetClientHandle();
							// horodate
							if (pMonitoredItem->GetTimestampsToReturn() == OpcUa_TimestampsToReturn_Source)
							{
								ZeroMemory(&(pMonitoredItemNotification->Value.ServerTimestamp), sizeof(OpcUa_DateTime));
								pMonitoredItemNotification->Value.ServerPicoseconds = 0;
								pMonitoredItemNotification->Value.SourceTimestamp = pDataValue->GetSourceTimeStamp();
								pMonitoredItemNotification->Value.SourcePicoseconds = pDataValue->GetSourcePicoseconds();
							}
							else
							{
								if (pMonitoredItem->GetTimestampsToReturn() == OpcUa_TimestampsToReturn_Server)
								{
									pMonitoredItemNotification->Value.ServerTimestamp = pDataValue->GetServerTimestamp();
									pMonitoredItemNotification->Value.ServerPicoseconds = pDataValue->GetServerPicoseconds();
									ZeroMemory(&(pMonitoredItemNotification->Value.SourceTimestamp), sizeof(OpcUa_DateTime));
									pMonitoredItemNotification->Value.SourcePicoseconds = 0;
								}
								else
								{
									if (pMonitoredItem->GetTimestampsToReturn() == OpcUa_TimestampsToReturn_Both)
									{
										pMonitoredItemNotification->Value.ServerTimestamp = pDataValue->GetServerTimestamp();
										pMonitoredItemNotification->Value.ServerPicoseconds = pDataValue->GetServerPicoseconds();
										pMonitoredItemNotification->Value.SourceTimestamp = pDataValue->GetSourceTimeStamp();
										pMonitoredItemNotification->Value.SourcePicoseconds = pDataValue->GetSourcePicoseconds();
									}
									else
									{
										if (pMonitoredItem->GetTimestampsToReturn() == OpcUa_TimestampsToReturn_Neither)
										{
											ZeroMemory(&(pMonitoredItemNotification->Value.ServerTimestamp), sizeof(OpcUa_DateTime));
											pMonitoredItemNotification->Value.ServerPicoseconds = 0;
											ZeroMemory(&(pMonitoredItemNotification->Value.SourceTimestamp), sizeof(OpcUa_DateTime));
											pMonitoredItemNotification->Value.SourcePicoseconds = 0;
										}
										else
											OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Unsupported OpcUa_TimestampsToReturn. Error in the call\n");
									}
								}
							}
							// StatusCode
							pMonitoredItemNotification->Value.StatusCode = pDataValue->GetStatusCode();
							// Valeur
							// Here we will create a OpcUa_Variant to transfert according to the AttributeId
							OpcUa_Variant aVariant;
							OpcUa_Variant_Initialize(&aVariant);
							switch (pMonitoredItem->GetAttributeId())
							{
							case OpcUa_Attributes_DataType:
							{
								aVariant.Datatype = OpcUaType_NodeId;
								//aVariant.Value.NodeId = (OpcUa_NodeId*)OpcUa_Alloc(sizeof(OpcUa_NodeId));
								//OpcUa_NodeId_Initialize(aVariant.Value.NodeId);
								OpcUa_NodeId aNodeId = pUAVariable->GetDataType();
								OpcUa_NodeId_CopyTo(&aNodeId, aVariant.Value.NodeId);
							}
							break;
							case OpcUa_Attributes_NodeId:
							{
								aVariant.Datatype = OpcUaType_NodeId;
								//aVariant.Value.NodeId = (OpcUa_NodeId*)OpcUa_Alloc(sizeof(OpcUa_NodeId));
								//OpcUa_NodeId_Initialize(aVariant.Value.NodeId);
								OpcUa_NodeId* pNodeId = pUAVariable->GetNodeId();
								OpcUa_NodeId_CopyTo(pNodeId, aVariant.Value.NodeId);
							}
							break;
							case OpcUa_Attributes_BrowseName:
							{
								aVariant.Datatype = OpcUaType_QualifiedName;
								if (!aVariant.Value.QualifiedName)
									aVariant.Value.QualifiedName = (OpcUa_QualifiedName*)OpcUa_Alloc(sizeof(OpcUa_QualifiedName));
								else
									OpcUa_QualifiedName_Clear(aVariant.Value.QualifiedName);
								OpcUa_QualifiedName_Initialize(aVariant.Value.QualifiedName);
								OpcUa_QualifiedName* pQualifiedName = pUAVariable->GetBrowseName();
								OpcUa_QualifiedName_CopyTo(pQualifiedName, aVariant.Value.QualifiedName);
							}
							break;
							case OpcUa_Attributes_DisplayName:
							{
								aVariant.Datatype = OpcUaType_LocalizedText;
								//aVariant.Value.LocalizedText = (OpcUa_LocalizedText*)OpcUa_Alloc(sizeof(OpcUa_LocalizedText));
								//OpcUa_LocalizedText_Initialize(aVariant.Value.LocalizedText);
								OpcUa_LocalizedText aLocalizedText = pUAVariable->GetDisplayName();
								OpcUa_LocalizedText_CopyTo(&aLocalizedText, aVariant.Value.LocalizedText);
							}
							break;
							case OpcUa_Attributes_Description:
							{
								aVariant.Datatype = OpcUaType_LocalizedText;
								//aVariant.Value.LocalizedText = (OpcUa_LocalizedText*)OpcUa_Alloc(sizeof(OpcUa_LocalizedText));
								//OpcUa_LocalizedText_Initialize(aVariant.Value.LocalizedText);
								OpcUa_LocalizedText aLocalizedText = pUAVariable->GetDescription();
								OpcUa_LocalizedText_CopyTo(&aLocalizedText, aVariant.Value.LocalizedText);
							}
							break;
							case OpcUa_Attributes_UserAccessLevel:
							{
								aVariant.Datatype = OpcUaType_Byte;
								aVariant.Value.Byte = pUAVariable->GetUserAccessLevel();
							}
							break;
							case OpcUa_Attributes_AccessLevel:
							{
								aVariant.Datatype = OpcUaType_Byte;
								aVariant.Value.Byte = pUAVariable->GetAccessLevel();
							}
							break;
							case OpcUa_Attributes_NodeClass:
							{
								aVariant.Datatype = OpcUaType_Int32;
								aVariant.Value.Int32 = pUAVariable->GetNodeClass();
							}
							break;
							case OpcUa_Attributes_ValueRank:
							{
								aVariant.Datatype = OpcUaType_Int32;
								aVariant.Value.Int32 = pUAVariable->GetValueRank();
							}
							break;
							case OpcUa_Attributes_Historizing:
							{
								aVariant.Datatype = OpcUaType_Boolean;
								aVariant.Value.Boolean = pUAVariable->GetHistorizing();
							}
							break;
							case OpcUa_Attributes_MinimumSamplingInterval:
							{
								aVariant.Datatype = OpcUaType_Double;
								aVariant.Value.Double = pUAVariable->GetMinimumSamplingInterval();
							}
							break;
							case OpcUa_Attributes_ArrayDimensions:
							{
								aVariant.Datatype = OpcUaType_Int32;
								std::vector<OpcUa_UInt32>* pArrayDimensions = pUAVariable->GetArrayDimensions();
								if (pArrayDimensions)
									aVariant.Value.Int32 = pArrayDimensions->size();
							}
							break;
							case OpcUa_Attributes_WriteMask:
								aVariant.Datatype = OpcUaType_UInt32;
								aVariant.ArrayType = OpcUa_VariantArrayType_Scalar;
								aVariant.Value.UInt32 = pUAVariable->GetWriteMask();
								break;
							case OpcUa_Attributes_UserWriteMask:
								aVariant.Datatype = OpcUaType_UInt32;
								aVariant.ArrayType = OpcUa_VariantArrayType_Scalar;
								aVariant.Value.UInt32 = pUAVariable->GetUserWriteMask();
								break;
							case OpcUa_Attributes_Value:
								aVariant = pDataValue->GetValue();
								break;
							}
							// Transfert de la valeur contenu dans la CUAVariable dans un OpcUA_Variant puis copie dans OpcUa_MonitoredItemNotification
							if (aVariant.ArrayType == OpcUa_VariantArrayType_Scalar)
							{
								switch (pUAVariable->GetBuiltInType())
								{
								case OpcUaType_String:
								{
									// check if a Range was require for this String dataType
									pString = pMonitoredItem->GetIndexRange();
									if (OpcUa_String_StrLen(pString) > 0)
									{
										CNumericRange* pNumericRange = new CNumericRange(pString);
										if (pNumericRange)
										{
											OpcUa_String aSubString;
											OpcUa_String_Initialize(&aSubString);
											OpcUa_CharA* aRawString = OpcUa_String_GetRawString(&(aVariant.Value.String));
											if (aRawString)
											{
												OpcUa_Int32 iOffset = (pNumericRange->GetEndIndex() - pNumericRange->GetBeginIndex() + 1);
												OpcUa_CharA* pBuff = OpcUa_Null;
												if (pNumericRange->IsUnique())
												{
													pBuff = (OpcUa_CharA*)OpcUa_Alloc((sizeof(OpcUa_CharA)*iOffset));
													ZeroMemory(pBuff, iOffset);
													for (OpcUa_Int32 iSubIndex = 0; iSubIndex < iOffset - 1; iSubIndex++)
														pBuff[iSubIndex] = aRawString[iSubIndex + (iOffset - 1)];
												}
												else
												{
													pBuff = (OpcUa_CharA*)OpcUa_Alloc((sizeof(OpcUa_CharA)*iOffset) + 1);
													ZeroMemory(pBuff, iOffset + 1);
													for (OpcUa_Int32 iSubIndex = 0; iSubIndex < iOffset; iSubIndex++)
														pBuff[iSubIndex] = aRawString[iSubIndex + pNumericRange->GetBeginIndex()];
												}
												OpcUa_String_AttachCopy(&aSubString, pBuff);
												OpcUa_DataValue_Initialize(&(pMonitoredItemNotification->Value));
												pMonitoredItemNotification->Value.Value.ArrayType = OpcUa_VariantArrayType_Scalar;
												pMonitoredItemNotification->Value.Value.Datatype = OpcUaType_String;
												OpcUa_String_CopyTo(&aSubString, &(pMonitoredItemNotification->Value.Value.Value.String));
												OpcUa_Free(pBuff);
												OpcUa_String_Clear(&aSubString);
											}
											else
											{
												OpcUa_DataValue_Initialize(&(pMonitoredItemNotification->Value));
												pMonitoredItemNotification->Value.Value.ArrayType = OpcUa_VariantArrayType_Scalar;
												pMonitoredItemNotification->Value.Value.Datatype = OpcUaType_String;
												OpcUa_String_CopyTo(&aSubString, &(pMonitoredItemNotification->Value.Value.Value.String));
												OpcUa_String_Clear(&aSubString);
											}
											delete pNumericRange;
										}
										else
											uStatus = OpcUa_BadOutOfMemory;
									}
									else
										OpcUa_Variant_CopyTo(&aVariant, &(pMonitoredItemNotification->Value.Value));
								}
								break;
								case OpcUaType_ByteString:
								{
									// check if a Range was require for this ByteString dataType
									pString = pMonitoredItem->GetIndexRange();
									if (OpcUa_String_StrLen(pString) > 0)
									{
										CNumericRange* pNumericRange = new CNumericRange(pString);
										if (pNumericRange)
										{
											OpcUa_ByteString aSubByteString;
											OpcUa_ByteString_Initialize(&aSubByteString);
											OpcUa_Int32 iOffset = (pNumericRange->GetEndIndex() - pNumericRange->GetBeginIndex()) + 1;
											if (pNumericRange->IsUnique())
											{
												aSubByteString.Data = (OpcUa_Byte*)OpcUa_Alloc(sizeof(OpcUa_Byte)*(iOffset - 1));
												for (OpcUa_Int32 iSubIndex = 0; iSubIndex < iOffset - 1; iSubIndex++)
													aSubByteString.Data[iSubIndex] = aVariant.Value.ByteString.Data[iSubIndex + (iOffset - 1)];
												aSubByteString.Length = iOffset - 1;
											}
											else
											{
												if (aVariant.Value.ByteString.Data)
												{
													aSubByteString.Data = (OpcUa_Byte*)OpcUa_Alloc(sizeof(OpcUa_Byte)*iOffset);
													for (OpcUa_Int32 iSubIndex = 0; iSubIndex < iOffset; iSubIndex++)
														aSubByteString.Data[iSubIndex] = aVariant.Value.ByteString.Data[iSubIndex + pNumericRange->GetBeginIndex()];
													aSubByteString.Length = iOffset;
												}
											}
											pMonitoredItemNotification->Value.Value.ArrayType = OpcUa_VariantArrayType_Scalar;
											pMonitoredItemNotification->Value.Value.Datatype = OpcUaType_ByteString;
											OpcUa_ByteString_CopyTo(&aSubByteString, &(pMonitoredItemNotification->Value.Value.Value.ByteString));
											OpcUa_ByteString_Clear(&aSubByteString);
											delete pNumericRange;
										}
										else
											uStatus = OpcUa_BadOutOfMemory;
									}
									else
										OpcUa_Variant_CopyTo(&aVariant, &(pMonitoredItemNotification->Value.Value));
								}
								break;
								case OpcUaType_NodeId:
								{
									pMonitoredItemNotification->Value.Value.ArrayType = 0;
									pMonitoredItemNotification->Value.Value.Datatype = OpcUaType_NodeId;
									OpcUa_NodeId_CopyTo(pUAVariable->GetValue()->GetValue().Value.NodeId, pMonitoredItemNotification->Value.Value.Value.NodeId);
								}
								break;
								case OpcUaType_Guid:
								{
									pMonitoredItemNotification->Value.Value.ArrayType = 0;
									pMonitoredItemNotification->Value.Value.Datatype = OpcUaType_Guid;
									if (!pMonitoredItemNotification->Value.Value.Value.Guid)
									{
										pMonitoredItemNotification->Value.Value.Value.Guid = (OpcUa_Guid*)OpcUa_Alloc(sizeof(OpcUa_Guid));
										OpcUa_Guid_Initialize(pMonitoredItemNotification->Value.Value.Value.Guid);
									}
									OpcUa_Guid_CopyTo(pUAVariable->GetValue()->GetValue().Value.Guid, pMonitoredItemNotification->Value.Value.Value.Guid);
								}
								break;
								case OpcUaType_QualifiedName:
								{
									pMonitoredItemNotification->Value.Value.ArrayType = 0;
									pMonitoredItemNotification->Value.Value.Datatype = OpcUaType_QualifiedName;
									if (!pMonitoredItemNotification->Value.Value.Value.QualifiedName)
									{
										pMonitoredItemNotification->Value.Value.Value.QualifiedName = (OpcUa_QualifiedName*)OpcUa_Alloc(sizeof(OpcUa_QualifiedName));
										OpcUa_QualifiedName_Initialize(pMonitoredItemNotification->Value.Value.Value.QualifiedName);
									}
									OpcUa_QualifiedName_CopyTo(pUAVariable->GetValue()->GetValue().Value.QualifiedName, pMonitoredItemNotification->Value.Value.Value.QualifiedName);
									/*
									if (aVariant.Value.QualifiedName)
									{
										OpcUa_QualifiedName_Clear(aVariant.Value.QualifiedName);
										OpcUa_Free(aVariant.Value.QualifiedName);
									}*/
								}
								break;
								case OpcUaType_ExtensionObject:
								{
									// alternative
									pMonitoredItemNotification->Value.Value.Value.ExtensionObject = OpcUa_Null; // (OpcUa_ExtensionObject*)OpcUa_Alloc(sizeof(OpcUa_ExtensionObject));
									pMonitoredItemNotification->Value.Value.Datatype = OpcUaType_ExtensionObject;
									//OpcUa_ExtensionObject* pExtensionObject;
									OpcUa_DataValue* pInternalDataValue = pUAVariable->GetValue()->GetInternalDataValue();
									// il faut faire un traitement specifique en fonction du type de données contenu dans l'extensionObject
									// le type contenu est codé dans 
									if (pInternalDataValue->Value.Value.ExtensionObject)
									{
										OpcUa_ExtensionObject* pExtensionObject = OpcUa_Null;
										if (pMonitoredItemNotification->Value.Value.Value.ExtensionObject == OpcUa_Null)
											pMonitoredItemNotification->Value.Value.Value.ExtensionObject = (OpcUa_ExtensionObject*)OpcUa_Alloc(sizeof(OpcUa_ExtensionObject));
										else
											OpcUa_ExtensionObject_Clear(pMonitoredItemNotification->Value.Value.Value.ExtensionObject);
										OpcUa_ExtensionObject_Initialize(pMonitoredItemNotification->Value.Value.Value.ExtensionObject);
										uStatus = OpcUa_ExtensionObject_CopyTo(pInternalDataValue->Value.Value.ExtensionObject, pMonitoredItemNotification->Value.Value.Value.ExtensionObject);
										if (uStatus != OpcUa_Good)
										{
											char* szNodeId = OpcUa_Null;
											OpcUa_NodeId aNodeId = pUAVariable->GetDataType();
											Utils::NodeId2String(&aNodeId, &szNodeId);
											if (szNodeId)
											{
												OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "UpdateDataSubscriptionThread>Unsupported EncodeableType %s\n", szNodeId);
												free(szNodeId);
											}
											pMonitoredItemNotification->Value.StatusCode = OpcUa_BadNotSupported;
											pMonitoredItemNotification->Value.Value.Value.ExtensionObject = (OpcUa_ExtensionObject*)OpcUa_Alloc(sizeof(OpcUa_ExtensionObject));
											OpcUa_ExtensionObject_Initialize(pMonitoredItemNotification->Value.Value.Value.ExtensionObject);
											uStatus = OpcUa_BadEncodingError;
											if (pExtensionObject)
											{
												OpcUa_ExtensionObject_Clear(pExtensionObject);
												OpcUa_Free(pExtensionObject);
											}
										}
										//
										// suppression temporaire du code ci-dessous. Cette fonction de copie générique des extensionObject fuit...
										// Commentaire novembre 2015
										/*uStatus = Copy(&pExtensionObject,
											pInternalDataValue->Value.Value.ExtensionObject);
											if (uStatus != OpcUa_Good)
											{
											char* szNodeId = OpcUa_Null;
											OpcUa_NodeId aNodeId = pUAVariable->GetDataType();
											Utils::NodeId2String(&aNodeId, &szNodeId);
											if (szNodeId)
											{
											OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "UpdateDataSubscriptionThread>Unsupported EncodeableType %s\n", szNodeId);
											free(szNodeId);
											}
											pMonitoredItemNotification->Value.StatusCode = OpcUa_BadNotSupported;
											pMonitoredItemNotification->Value.Value.Value.ExtensionObject = (OpcUa_ExtensionObject*)OpcUa_Alloc(sizeof(OpcUa_ExtensionObject));
											OpcUa_ExtensionObject_Initialize(pMonitoredItemNotification->Value.Value.Value.ExtensionObject);
											uStatus = OpcUa_BadEncodingError;
											if (pExtensionObject)
											{
											OpcUa_ExtensionObject_Clear(pExtensionObject);
											OpcUa_Free(pExtensionObject);
											}
											}
											else
											{
											if (pExtensionObject)
											{
											OpcUa_ExtensionObject_CopyTo(pExtensionObject, pMonitoredItemNotification->Value.Value.Value.ExtensionObject);
											OpcUa_ExtensionObject_Clear(pExtensionObject);
											OpcUa_Free(pExtensionObject);
											}
											}*/
									}
									else
									{
										char* szNodeId = OpcUa_Null;
										OpcUa_NodeId aNodeId = pUAVariable->GetDataType();
										Utils::NodeId2String(&aNodeId, &szNodeId);
										if (szNodeId)
										{
											OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "Uninitialized EncodeableType %s\n", szNodeId);
											free(szNodeId);
										}
										pMonitoredItemNotification->Value.StatusCode = OpcUa_BadWaitingForInitialData;
										pMonitoredItemNotification->Value.Value.Value.ExtensionObject = (OpcUa_ExtensionObject*)OpcUa_Alloc(sizeof(OpcUa_ExtensionObject));
										OpcUa_ExtensionObject_Initialize(pMonitoredItemNotification->Value.Value.Value.ExtensionObject);
										uStatus = OpcUa_BadDataEncodingInvalid;
									}
								}
								break;
								default:
									OpcUa_Variant_CopyTo(&aVariant, &(pMonitoredItemNotification->Value.Value));
									break;
								}
							}
							else
							{
								if (pUAVariable->GetValue()->GetValue().ArrayType == OpcUa_VariantArrayType_Array)
								{
									if (aVariant.Value.Array.Length > 0) // We check that something is in the Array. So we can check any type
									{
										pString = pMonitoredItem->GetIndexRange();
										if (OpcUa_String_StrLen(pString) > 0)
										{
											CNumericRange* pNumericRange = new CNumericRange(pString);
											if (pNumericRange)
											{
												OpcUa_DataValue* pTmpDataValue2 = OpcUa_Null;
												OpcUa_DataValue* pTmpDataValue1 = pDataValue->GetInternalDataValue();
												if (pNumericRange->GetStatusCode() == OpcUa_Good)
												{
													// check pTmpDataValue1 conform pNumericRange
													if (pTmpDataValue1->Value.Value.Array.Length < pNumericRange->GetBeginIndex())
														pMonitoredItemNotification->Value.StatusCode = OpcUa_BadIndexRangeNoData;
													else
													{
														uStatus = AdjustArrayToRange(pTmpDataValue1, pNumericRange, &pTmpDataValue2);
														if (uStatus == OpcUa_Good)
														{
															pMonitoredItemNotification->Value.StatusCode = OpcUa_Variant_CopyTo(&(pTmpDataValue2->Value), &(pMonitoredItemNotification->Value.Value));
															if (pMonitoredItemNotification->Value.StatusCode != OpcUa_Good)
																OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "pMonitoredItemNotification->Value.StatusCode=%u\n", pMonitoredItemNotification->Value.StatusCode);
															// Just for leak tracking 02-2016
															OpcUa_DataValue_Clear(pTmpDataValue2);
															OpcUa_Free(pTmpDataValue2);
															pTmpDataValue2 = OpcUa_Null;
															// end Just for leak tracking 02-2016
														}
														else
														{
															if (pTmpDataValue2)
															{
																OpcUa_DataValue_Clear(pTmpDataValue2);
																OpcUa_Free(pTmpDataValue2);
															}
															if (uStatus == OpcUa_BadIndexRangeNoData)
																uStatus = OpcUa_Good;
														}
													}
												}
												else
													uStatus = pNumericRange->GetStatusCode();
												delete pNumericRange;
											}
											else
												uStatus = OpcUa_BadOutOfMemory;
										}
										else
											uStatus = OpcUa_Variant_CopyTo(&aVariant, &(pMonitoredItemNotification->Value.Value));
									}
									else
										uStatus = OpcUa_BadNothingToDo;
								}
							}
							
							// pDataChangeNotification=SOMME(OpcUa_MonitoredItemNotification) for the specific clientHandle
							//
							if (uStatus == OpcUa_Good)
							{
								if (pMonitoredItem->GetMonitoringMode()!=OpcUa_MonitoringMode_Disabled)
								{
									// Fill the DataChangeNotification with the new MonitoredItemNotification
									if ((*pDataChangeNotification)->AddMonitoredItemNotification(
										pMonitoredItemNotification,
										pMonitoredItem->GetQueueSize(),
										pMonitoredItem->IsDiscardOldest()) != OpcUa_Good)
										OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_WARNING, "Warning:>AddMonitoredItemNotification will not add this MonitoredItemNotification. It is already in the list\n"); // Found a UAVariable to Notify
									else
									{
										// Handle the triggering if any
										if (pMonitoredItem->IsTriggeredItemId())
										{
											vector<OpcUa_MonitoredItemNotification*>::iterator it;
											vector<OpcUa_MonitoredItemNotification*> triggeredMonitoredItemNotificationList = pMonitoredItem->GetTriggeredMonitoredItemNotificationList();
											for (it = triggeredMonitoredItemNotificationList.begin(); it != triggeredMonitoredItemNotificationList.end(); ++it)
											{
												OpcUa_MonitoredItemNotification* pTriggerMonitoredItemNotification = *it;
												if ((*pDataChangeNotification)->AddMonitoredItemNotification(pTriggerMonitoredItemNotification, 0, OpcUa_False) != OpcUa_Good)
												{
													OpcUa_Free(pTriggerMonitoredItemNotification);
													pTriggerMonitoredItemNotification = OpcUa_Null;
												}
											}
											triggeredMonitoredItemNotificationList.clear();
										}
									}
								}
								

								/////////////////////////////////////////////////////////////////////////////////////////////
								pMonitoredItem->SetValue(&(pMonitoredItemNotification->Value));
								pMonitoredItem->SetStatusCode(pMonitoredItemNotification->Value.StatusCode);
								// May 2015 clean up
								OpcUa_MonitoredItemNotification_Clear(pMonitoredItemNotification);
								OpcUa_Free(pMonitoredItemNotification);
								pMonitoredItemNotification = OpcUa_Null;
							}
							else
							{
								/////////////////////////////////////////////////////////////////////////////////////////////
								OpcUa_MonitoredItemNotification_Clear(pMonitoredItemNotification);
								OpcUa_Free(pMonitoredItemNotification);
								pMonitoredItemNotification = OpcUa_Null;
							}
							Changed(TRUE);
						}
						else
							uStatus = OpcUa_BadOutOfMemory;
					}
					else
						uStatus = OpcUa_BadInvalidState;
					OpcUa_Mutex_Unlock(pDataChangeNotificationMutex);
				}
				else
					uStatus = OpcUa_BadInvalidArgument;
			} // fin if (pSubscription->IsMonitoredItemFromUAVariableEqual(pUAVariable,pMonitoredItem)==OpcUa_False ) 
			else
				uStatus = OpcUa_BadNoMatch;
		}
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}
// Cette méthode ajoute simplement un CUAEventNotificationList dans la m_pEventNotificationListList
// Elle vérifie que le paramètre recu est bien formé (non-null et n° de séquence cohérent)
OpcUa_StatusCode CSubscriptionServer::AddEventNotificationList(CUAEventNotificationList* pUAEventNotificationList)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_Mutex_Lock(m_hEventNotificationListListMutex);
	if (pUAEventNotificationList)
	{
		// Verification que le n° de séquence à bien été initialisé lors de la création de l'instance pUAEventNotificationList
		OpcUa_UInt32 uiSeqNum = pUAEventNotificationList->GetSequenceNumber();
		OpcUa_UInt32 uiCurrentSequenceNumber = GetCurrentSequenceNumber();
		// Adjust in case we jsut start for event support
		if (uiCurrentSequenceNumber == 0)
			uiCurrentSequenceNumber = 1;
		if (uiSeqNum == uiCurrentSequenceNumber)
			m_pEventNotificationListList->push_back(pUAEventNotificationList);
		else
			uStatus = OpcUa_BadInvalidArgument;
	}
	OpcUa_Mutex_Unlock(m_hEventNotificationListListMutex);
	return uStatus;
}
OpcUa_StatusCode CSubscriptionServer::GetEventNotificationListFromEventDefinition(CEventDefinition* pEventDefinition, CUAEventNotificationListList* pUAEventNotificationListList)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	if (pEventDefinition)
	{
		for (OpcUa_UInt32 i = 0; i < m_pEventNotificationListList->size(); i++)
		{
			CUAEventNotificationList* pUALocalEventNotificationList = m_pEventNotificationListList->at(i);
			if (!pUALocalEventNotificationList->IsTerminated())
			{
				if (pUALocalEventNotificationList->GetEventDefinition() == pEventDefinition)
					pUAEventNotificationListList->push_back(pUALocalEventNotificationList);
			}
		}
	}
	return uStatus;
}
// GetEventNotificationList based on the EventId
// The EventId is a byteStirng used as a key to unicaly identify Event
OpcUa_StatusCode CSubscriptionServer::GetEventNotificationListFromEventId(OpcUa_ByteString* pByteString, CUAEventNotificationList** pUAEventNotificationList)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	if (pByteString)
	{
		//OpcUa_Mutex_Lock(m_hEventNotificationListListMutex); Already locked by the caller
		for (OpcUa_UInt32 i = 0; i < m_pEventNotificationListList->size(); i++)
		{
			CUAEventNotificationList* pUALocalEventNotificationList = m_pEventNotificationListList->at(i);
			if (!pUALocalEventNotificationList->IsTerminated())
			{
				OpcUa_EventNotificationList* pEventNotificationList = pUALocalEventNotificationList->GetInternalEventNotificationList();
				if (pEventNotificationList)
				{
					if (pEventNotificationList->Events[0].EventFields)
					{
						OpcUa_ByteString EventId = pEventNotificationList->Events[0].EventFields[0].Value.ByteString;
						if (OpcUa_ByteString_Compare(pByteString, &EventId) == 0)
						{
							(*pUAEventNotificationList) = pUALocalEventNotificationList;
							break;
						}
					}
					else
						uStatus = OpcUa_BadInternalError;
				}
			}
		}
		//OpcUa_Mutex_Unlock(m_hEventNotificationListListMutex);
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}
OpcUa_StatusCode  CSubscriptionServer::ConditionRefresh()
{
	OpcUa_StatusCode uStatus = OpcUa_Good;

	// Let start by Looking at the Already notified Event to check if they are acknowledge 
	OpcUa_Mutex_Lock(m_hEventNotificationListListMutex);
	for (OpcUa_UInt32 i = 0; i < m_pEventNotificationListList->size(); i++)
	{
		OpcUa_Boolean bActiveStateId = OpcUa_False;
		OpcUa_Boolean bAckedStateId = OpcUa_False;
		CUAEventNotificationList* pUALocalEventNotificationList = m_pEventNotificationListList->at(i);
		if (pUALocalEventNotificationList)
		{
			FastEventFieldIndex aFastEventFieldIndex = pUALocalEventNotificationList->GetFastEventFieldIndex();
			OpcUa_EventNotificationList* pInternalEventNotificationList = pUALocalEventNotificationList->GetInternalEventNotificationList();
			if (pInternalEventNotificationList)
			{
				if (pInternalEventNotificationList->Events[0].EventFields[aFastEventFieldIndex.iActiveStateId].Datatype == OpcUaType_Boolean)
					bActiveStateId = pInternalEventNotificationList->Events[0].EventFields[aFastEventFieldIndex.iActiveStateId].Value.Boolean;
				if (pInternalEventNotificationList->Events[0].EventFields[aFastEventFieldIndex.iAckedStateId].Datatype == OpcUaType_Boolean)
					bAckedStateId = pInternalEventNotificationList->Events[0].EventFields[aFastEventFieldIndex.iAckedStateId].Value.Boolean;
			}
			if ((bActiveStateId == (OpcUa_Boolean)OpcUa_True) || (bAckedStateId == (OpcUa_Boolean)OpcUa_False))
				pUALocalEventNotificationList->TransactionAcked(OpcUa_False);
		}
	}
	OpcUa_Mutex_Unlock(m_hEventNotificationListListMutex);
	return uStatus;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	This method scan EventNotificationListList (Event cache) to find out pEventDefinition in the list.
///				If pEventDefinition is found and ActiveState=true then it's consider OnStage</summary>
///
/// <remarks>	Michel, 31/08/2016. </remarks>
///
/// <param name="pEventDefinition">	[in] If non-null, the event definition. </param>
///
/// <returns>	An OpcUa_Boolean. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_Boolean CSubscriptionServer::IsOnStage(CEventDefinition* pEventDefinition)
{
	OpcUa_Boolean bResult=OpcUa_False;
	OpcUa_Mutex_Lock(m_hEventNotificationListListMutex);
	for (OpcUa_UInt32 i = 0; i < m_pEventNotificationListList->size(); i++)
	{
		CUAEventNotificationList* pUALocalEventNotificationList = m_pEventNotificationListList->at(i);
		if (pUALocalEventNotificationList)
		{
			//if (!pUALocalEventNotificationList->IsTerminated())
			{
				CEventDefinition* pTmpEventDefinition = pUALocalEventNotificationList->GetEventDefinition();
				if (pEventDefinition == pTmpEventDefinition)
				{
					//if (pEventDefinition->GetActiveState() != pUALocalEventNotificationList->GetLastNotifiedActiveState())
					//{
					//	bResult = OpcUa_False;
					//	break;
					//}
					//else
					{
						//bResult = OpcUa_False;
						//if (pEventDefinition->GetActiveState() && (!pUALocalEventNotificationList->GetLastNotifiedActiveState()))
						if ((pUALocalEventNotificationList->GetLastNotifiedActiveState() == (OpcUa_Boolean)OpcUa_True))
						{
							bResult = OpcUa_True;
							break;
						}
					}
				}
			}
		}
	}
	OpcUa_Mutex_Unlock(m_hEventNotificationListListMutex);
	return bResult;
}

void CSubscriptionServer::UpdateFastestSamplingInterval()
{
	CUAInformationModel* pInformationModel = CServerApplication::m_pTheAddressSpace;
	OpcUa_Mutex_Lock(m_hMonitoredItemListMutex);
	m_fastestSamplingInterval = 10000; // set the fastest to 10 seconds
	OpcUa_NodeId aNodeId;
	OpcUa_NodeId_Initialize(&aNodeId);
	aNodeId.Identifier.Numeric = 2289;
	for (OpcUa_UInt32 ii = 0; ii < m_pMonitoredItemList->size(); ii++)
	{
		CMonitoredItemServer* pLocalItem = m_pMonitoredItemList->at(ii);
		OpcUa_SamplingIntervalDiagnosticsDataType* pSamplingIntervalDiagnostic = OpcUa_Null;
		if (pInformationModel->IsSamplingIntervalDiagnosticsExist(pLocalItem->GetSamplingInterval(), &pSamplingIntervalDiagnostic))
		{
			pSamplingIntervalDiagnostic->MonitoredItemCount--;
			pInformationModel->UpdateSamplingIntervalDiagnosticsArray(aNodeId);
		}
		if (pLocalItem->GetSamplingInterval() < m_fastestSamplingInterval)
			m_fastestSamplingInterval = pLocalItem->GetSamplingInterval();
	}
	OpcUa_Mutex_Unlock(m_hMonitoredItemListMutex);
	OpcUa_NodeId_Clear(&aNodeId);
}

CSubscriptionDiagnosticsDataType* CSubscriptionServer::GetSubscriptionDiagnosticsDataType()
{
	/*
	if ((m_bRunUpdateDataSubscriptionThread) && (m_pSubscriptionDiagnosticsDataType) )
	{
		if (UpdateSubscriptionDiagnosticsDataType() != OpcUa_Good)
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "UpdateSubscriptionDiagnosticsDataType failed\n");
	}*/
	return	m_pSubscriptionDiagnosticsDataType;
}

OpcUa_Boolean CSubscriptionServer::IsInColdState() 
{ 
	return m_bSubscriptionInColdState; 
}
void CSubscriptionServer::SetInColdState(OpcUa_Boolean bVal) 
{ 
	m_bSubscriptionInColdState = bVal; 
}
// Force la reveil de l' UpdateDataSubscriptionThread
void CSubscriptionServer::WakeupUpdateSubscriptionThread()
{
	//if (IsInColdState())
	//	SetKeepAlive(OpcUa_True); 
	if (m_uiLifeTimeCountCounter>1)
		m_uiLifeTimeCountCounter--;
	OpcUa_Semaphore_Post(m_hUpdateSubscriptionThreadSafeSem, 1);// WakeupUpdateSubscriptionThread() 
}

//OpcUa_Semaphore	CSubscriptionServer::GetStopUpdateSubscriptionSem() 
//{ 
//	return m_hStopUpdateDataSubscriptionSem; 
//}
OpcUa_Boolean CSubscriptionServer::isInLate() 
{ 
	return m_bInLate; 
}
void CSubscriptionServer::InLate(OpcUa_Boolean bVal)
{
	m_bInLate = bVal;
}
// indique que (m_pRevisedLifetimeCount * m_pRevisedPublishingInterval) est atteint et que l'on doit arrété cette souscritpion
OpcUa_Boolean CSubscriptionServer::IsLifetimeCountReach() 
{ 
	OpcUa_DateTime dtLastTransmissionTime = GetLastTransmissionTime();
	OpcUa_Double dblPublishingInterval = GetPublishingInterval(); // ms
	OpcUa_UInt32 uiLifeTimeCount=GetLifetimeCount();
	OpcUa_Double dblElaspedValue = dblPublishingInterval*uiLifeTimeCount;
	OpcUa_Int64 i64Val = (OpcUa_Int64)(dblElaspedValue * 10000);
	// Based on the last transmission time calculate the new theorical lifeTime limit
	OpcUa_DateTime dtNow = OpcUa_DateTime_UtcNow();
	OpcUa_Int64 ui64LastTransmissionTime = OpcUa_DateTime_ToInt64(dtLastTransmissionTime);// Last time in a unsigned 64 bit
	// Create a 64 bit representation of now
	OpcUa_Int64 i64Now = OpcUa_DateTime_ToInt64(dtNow);
	// Check that the last transmission time is not 0 (startup)
	if (ui64LastTransmissionTime == 0)
		ui64LastTransmissionTime = i64Now - (i64Val-1);
	OpcUa_UInt64 ui64Next = i64Val + ui64LastTransmissionTime; // last time + interval
	OpcUa_Int64 idtCompared = ui64Next - i64Now;
	if (idtCompared <= 0)
	{
		m_bLifetimeCountReach = OpcUa_True;
		OpcUa_DiagnosticInfo aDiagInfo;
		OpcUa_DiagnosticInfo_Initialize(&aDiagInfo);		
		AddStatusChangeNotificationMessage(aDiagInfo, OpcUa_BadTimeout);
	}
	else
		m_bLifetimeCountReach = OpcUa_False;

	return m_bLifetimeCountReach; 
}

void CSubscriptionServer::ResetLifeTimeCountCounter() 
{ 
	m_uiLifeTimeCountCounter = 0;
}

//void CSubscriptionServer::ResetKeepAliveCounter()
//{ 
//	m_uiKeepAliveCounter = 0;
//}
OpcUa_Double CSubscriptionServer::GetFastestSamplingInterval() 
{ 
	return m_fastestSamplingInterval; 
}
void CSubscriptionServer::SetLastPublishAnswer(OpcUa_UInt32 uiVal)
{
	m_uiLastPublishAnswer = uiVal;
}

OpcUa_MonitoringMode CSubscriptionServer::GetMonitoringMode()
{
	return m_iMonitoringMode;
}
void CSubscriptionServer::SetMonitoringMode(OpcUa_MonitoringMode iMonitoringMode)
{
	m_iMonitoringMode = iMonitoringMode;
}
void CSubscriptionServer::SetKeepAlive(OpcUa_Boolean bVal) 
{ 
	m_bKeepAlive = bVal; 
}

OpcUa_Mutex	CSubscriptionServer::GetDataChangeNotificationListMutex()
{ 
	return m_hDataChangeNotificationListMutex; 
}
//CUADataChangeNotificationList*	CSubscriptionServer::GetDataChangeNotificationList()
//{ 
//	return m_pDataChangeNotificationList; 
//}
// StatusChangeNotificationList
OpcUa_Mutex CSubscriptionServer::GetStatusChangeNotificationListMutex()
{ 
	return m_hStatusChangeNotificationListMutex; 
}
CUAStatusChangeNotificationList* CSubscriptionServer::GetStatusChangeNotificationList()
{ 
	return m_pStatusChangeNotificationList; 
}
// EventNotificationListList
CUAEventNotificationListList* CSubscriptionServer::GetEventNotificationListList()
{ 
	return m_pEventNotificationListList; 
}
OpcUa_Mutex CSubscriptionServer::GetEventNotificationListListMutex()
{ 
	return m_hEventNotificationListListMutex; 
}
OpcUa_UInt32 CSubscriptionServer::GetEventNotificationListListCurrentPos()
{ 
	return m_EventNotificationListCurrentPos; 
}
void CSubscriptionServer::SetEventNotificationListListCurrentPos(OpcUa_UInt32 uiVal)
{ 
	m_EventNotificationListCurrentPos = uiVal; 
}

OpcUa_UInt32 CSubscriptionServer::GetCurrentSequenceNumber()
{
	return m_uiCurrentSequenceNumber;
}
void CSubscriptionServer::SetCurrentSequenceNumber(OpcUa_UInt32 iVal) 
{ 
	m_uiCurrentSequenceNumber = iVal; 
}

OpcUa_Boolean CSubscriptionServer::IsChanged() 
{ 
	return m_bChanged; 
}
void CSubscriptionServer::Changed(OpcUa_Boolean bVal) 
{ 
	m_bChanged = bVal; 
}

OpcUa_UInt32 CSubscriptionServer::GetDataChangeNotificationCurrentPos() 
{ 
	return m_DataChangeNotificationCurrentPos; 
}
void CSubscriptionServer::SetDataChangeNotificationCurrenPos(OpcUa_UInt32 uiVal) 
{ 
	m_DataChangeNotificationCurrentPos = uiVal; 
}
OpcUa_UInt32 CSubscriptionServer::GetStatusChangeNotificationCurrentPos() 
{ 
	return m_StatusChangeNotificationCurrentPos; 
}
void CSubscriptionServer::SetStatusChangeNotificationCurrenPos(OpcUa_UInt32 uiVal) 
{ 
	m_StatusChangeNotificationCurrentPos = uiVal; 
}
CSessionServer* CSubscriptionServer::GetSession() 
{ 
	return m_pSession; 
}
void CSubscriptionServer::SetSession(CSessionServer* pSession)
{
	m_pSession = pSession;
}
CMonitoredItemServerList* CSubscriptionServer::GetMonitoredItemList() 
{ 
	return m_pMonitoredItemList; 
}

//OpcUa_StatusCode CSubscriptionServer::AddDataChangeNotification(CUADataChangeNotification* pUADataChangeNotification)
//{
//	OpcUa_StatusCode uStatus = OpcUa_Good;
//	OpcUa_Mutex_Lock(m_hDataChangeNotificationListMutex);
//	if (pUADataChangeNotification)
//	{
//		m_pDataChangeNotificationList->push_back(pUADataChangeNotification);
//	}
//	else
//		uStatus = OpcUa_BadInvalidArgument;
//	OpcUa_Mutex_Unlock(m_hDataChangeNotificationListMutex);
//	return uStatus;
//}

///-------------------------------------------------------------------------------------------------
/// <summary>	Gets the first unacked CUADataChangeNotification in the m_pDataChangeNotificationList. </summary>
///				Warning the m_pDataChangeNotificationListMutex must be lock and unlock by the caller
/// <remarks>	Michel, 26/01/2016. </remarks>
///
/// <returns>	a CUADataChangeNotification ptr or OpcUa_Null if nothing found. </returns>
///-------------------------------------------------------------------------------------------------

CUADataChangeNotification* CSubscriptionServer::GetUnackedDataChangeNotification(void)
{
	// Already locked by the caller
	CUADataChangeNotification* pUADataChangeNotification = OpcUa_Null;
	CAvailableSequenceNumbers AvailableSequenceNumbers = AvailableSequenceNumberGet();
	CAvailableSequenceNumbers::iterator iteratorAvailableSequenceNumber;
	for (iteratorAvailableSequenceNumber = AvailableSequenceNumbers.begin(); iteratorAvailableSequenceNumber != AvailableSequenceNumbers.end(); iteratorAvailableSequenceNumber++)
	{
		CUADataChangeNotification* pTmpUADataChangeNotification = iteratorAvailableSequenceNumber->second;
		if (!pTmpUADataChangeNotification->IsAcked())
		{
			pUADataChangeNotification = pTmpUADataChangeNotification;
			break;
		}
	}
	return pUADataChangeNotification;
}

///-------------------------------------------------------------------------------------------------
/// <summary>
/// 	Add a new CUADataChangeNotification in the map m_AvailableSequenceNumbers. 
/// 	if the sequence number is not already in there.
/// </summary>
///
/// <remarks>	Michel, 26/01/2016. </remarks>
///
/// <param name="uiSequenceNumber">		  	The sequence number. </param>
/// <param name="pDataChangeNotification">	[in,out] If non-null, the data change notification.
/// </param>
///
/// <returns>	An OpcUa_Boolean. true if the CUADataChangeNotification was properly added   </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_Boolean CSubscriptionServer::AvailableSequenceNumberAdd(OpcUa_UInt32 uiSequenceNumber, CUADataChangeNotification* pDataChangeNotification)
{
	OpcUa_Boolean bResult = OpcUa_True;
	CAvailableSequenceNumbers::iterator iteratorAvailableSequenceNumber;
	OpcUa_Mutex_Lock(m_AvailableSequenceNumbersMutex);
	iteratorAvailableSequenceNumber = m_AvailableSequenceNumbers.find(uiSequenceNumber);
	if (iteratorAvailableSequenceNumber == m_AvailableSequenceNumbers.end())
	{
		//if (pDataChangeNotification->GetDataChangeNotificationType() != NOTIFICATION_MESSAGE_KEEPALIVE)
		{
			m_AvailableSequenceNumbers.insert(std::pair<OpcUa_UInt32, CUADataChangeNotification*>(uiSequenceNumber, pDataChangeNotification));
			SetCurrentSequenceNumber(++uiSequenceNumber);
		}
	}
	else
		bResult = OpcUa_False;
	OpcUa_Mutex_Unlock(m_AvailableSequenceNumbersMutex);
	return bResult;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Available sequence number delete. </summary>
///
/// <remarks>	Michel, 26/01/2016. </remarks>
///
/// <param name="uiSequenceNumber">	The sequence number. </param>
///-------------------------------------------------------------------------------------------------

void CSubscriptionServer::AvailableSequenceNumberDelete(OpcUa_UInt32 uiSequenceNumber)
{
	CUInt32Collection::iterator it;
	OpcUa_Mutex_Lock(m_AvailableSequenceNumbersMutex);
	CAvailableSequenceNumbers::iterator iteratorAvailableSequenceNumber;
	iteratorAvailableSequenceNumber = m_AvailableSequenceNumbers.find(uiSequenceNumber);
	if (iteratorAvailableSequenceNumber != m_AvailableSequenceNumbers.end())
	{
		CUADataChangeNotification* pDataChangeNotification = iteratorAvailableSequenceNumber->second;
		delete pDataChangeNotification;
		m_AvailableSequenceNumbers.erase(iteratorAvailableSequenceNumber);
	}
	OpcUa_Mutex_Unlock(m_AvailableSequenceNumbersMutex);
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Available sequence number get. </summary>
///
/// <remarks>	Michel, 26/01/2016. </remarks>
///
/// <returns>	A CUInt32Collection. </returns>
///-------------------------------------------------------------------------------------------------

CAvailableSequenceNumbers CSubscriptionServer::AvailableSequenceNumberGet(void)
{
	return m_AvailableSequenceNumbers;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Available sequence number get by data change notification. </summary>
///
/// <remarks>	Michel, 27/01/2016. </remarks>
///
/// <param name="pDataChangeNotification">	[in,out] If non-null, the data change notification.
/// </param>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode CSubscriptionServer::AvailableSequenceNumberGetByDataChangeNotification(CUADataChangeNotification* pDataChangeNotification)
{
	OpcUa_StatusCode uStatus = OpcUa_BadNotFound;
	//return uStatus;
	//CAvailableSequenceNumbers::iterator iteratorAvailableSequenceNumber;
	//for (iteratorAvailableSequenceNumber = m_AvailableSequenceNumbers.begin(); iteratorAvailableSequenceNumber != m_AvailableSequenceNumbers.end(); iteratorAvailableSequenceNumber++)
	//{
	//	CUADataChangeNotification* pTmpUADataChangeNotification = iteratorAvailableSequenceNumber->second;
	//	if (pTmpUADataChangeNotification == pDataChangeNotification)
	//	{
	//		uStatus = OpcUa_Good;
	//		break;
	//	}
	//}
	return uStatus;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Is keep alive required. </summary>
///
/// <remarks>	Michel, 27/01/2016. </remarks>
///
/// <returns>	An OpcUa_Boolean. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_Boolean CSubscriptionServer::IsKeepAliveRequired(void)
{
	OpcUa_Boolean bResult=OpcUa_False;
	// Calculate duration between two keepAlive or regular nofication
	OpcUa_UInt32 uiMaxKeepAliveCount = GetMaxKeepAliveCount(); //
	OpcUa_Double dblPublishingInterval = GetPublishingInterval(); // ms
	OpcUa_Double dblElaspedValue = dblPublishingInterval*uiMaxKeepAliveCount;
	OpcUa_Int64 i64Val = (OpcUa_Int64)(dblElaspedValue * 10000);
	// Based on the last transmission time calculate the new theorical transmission time

	//OpcUa_DateTime dtLastTransmissionTime = m_pSession->GetLastTransmissionTime();
	OpcUa_DateTime dtLastTransmissionTime = GetLastTransmissionTime();

	OpcUa_DateTime dtNow = OpcUa_DateTime_UtcNow();
	//OpcUa_CharA* pBufTime= (OpcUa_CharA*)OpcUa_Alloc(50);	
	
	//ZeroMemory(pBufTime, 50); 
	//OpcUa_DateTime_GetStringFromDateTime(dtLastTransmissionTime, pBufTime, 50);

	OpcUa_Int64 ui64LastTransmissionTime = OpcUa_DateTime_ToInt64(dtLastTransmissionTime);// Last time in a unsigned 64 bit
	// Create a 64 bit representation of now
	OpcUa_Int64 i64Now=OpcUa_DateTime_ToInt64(dtNow);
	// Check that the last transmission time is not 0 (startup)
	if (ui64LastTransmissionTime == 0)
		ui64LastTransmissionTime = i64Now - i64Val;
	OpcUa_UInt64 ui64Next = i64Val + ui64LastTransmissionTime; // last time + interval
	union
	{
		OpcUa_UInt64 ui64Val;
		OpcUa_UInt32 ui32Val[2];
	} converterNextKeepAlive;
	converterNextKeepAlive.ui64Val = ui64Next;
	OpcUa_DateTime dtNextKeepAlive;
	dtNextKeepAlive.dwHighDateTime = converterNextKeepAlive.ui32Val[1];// = OpcUa_DateTime_FromInt64(i64Val + ui64LastTransmissionTime);
	dtNextKeepAlive.dwLowDateTime = converterNextKeepAlive.ui32Val[0];


	// Compare with now
	OpcUa_Int64 idtCompared = ui64Next-i64Now  ;
	if (idtCompared<=0)
	{
		//ZeroMemory(pBufTime, 50);
		//OpcUa_DateTime_GetStringFromDateTime(dtNextKeepAlive, pBufTime, 50);
		//ZeroMemory(pBufTime, 50);
		//OpcUa_DateTime_GetStringFromDateTime(dtNow, pBufTime, 50);
		bResult = OpcUa_True;
	}
	//OpcUa_Free(pBufTime);
	return bResult;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Available sequence number get. </summary>
///
/// <remarks>	Michel, 28/01/2016. </remarks>
///
/// <param name="uiSequenceNumber">		  	The sequence number. </param>
/// <param name="pDataChangeNotification">	[in,out] If non-null, the data change notification.
/// </param>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode CSubscriptionServer::AvailableSequenceNumberGet(OpcUa_UInt32 uiSequenceNumber, CUADataChangeNotification*pDataChangeNotification)
{
	OpcUa_StatusCode uStatus = OpcUa_BadNotFound;
	OpcUa_Mutex_Lock(m_AvailableSequenceNumbersMutex);
	CAvailableSequenceNumbers::iterator iteratorAvailableSequenceNumber;
	iteratorAvailableSequenceNumber = m_AvailableSequenceNumbers.find(uiSequenceNumber);	
	if (iteratorAvailableSequenceNumber != m_AvailableSequenceNumbers.end())
	{
		pDataChangeNotification = iteratorAvailableSequenceNumber->second;
		uStatus = OpcUa_Good;
	}
	OpcUa_Mutex_Unlock(m_AvailableSequenceNumbersMutex);
	return uStatus;
}


// lastTrasmissionTime manipulation
void CSubscriptionServer::SetLastTransmissionTime(OpcUa_DateTime dtVal)
{
	OpcUa_DateTime_CopyTo(&dtVal, &m_LastTransmissionTime);
}
OpcUa_DateTime CSubscriptionServer::GetLastTransmissionTime()
{
	return m_LastTransmissionTime;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Sets subscription diagnostics data type. </summary>
///
/// <remarks>	Michel, 26/05/2016. </remarks>
///
/// <param name="pSubscriptionDiagnosticsDataType">	[in,out] If non-null, type of the
/// 												subscription diagnostics data.
/// </param>
///-------------------------------------------------------------------------------------------------

void OpenOpcUa::UACoreServer::CSubscriptionServer::SetSubscriptionDiagnosticsDataType(CSubscriptionDiagnosticsDataType*pSubscriptionDiagnosticsDataType)
{
	m_pSubscriptionDiagnosticsDataType = pSubscriptionDiagnosticsDataType;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Handles the system status change event. </summary>
///
/// <remarks>	Michel, 10/08/2016. </remarks>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode OpenOpcUa::UACoreServer::CSubscriptionServer::HandleSystemStatusChangeEvent(CMonitoredItemServer* pMonitoredItem)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_NodeId SystemStatusChangeEventType;
	OpcUa_NodeId_Initialize(&SystemStatusChangeEventType);
	SystemStatusChangeEventType.IdentifierType = OpcUa_IdentifierType_Numeric;
	SystemStatusChangeEventType.Identifier.Numeric = 11446;
	// Search this EventType in the AddressSpace
	CUAObjectType* pUAObjectType = OpcUa_Null;
	CUAInformationModel* pInformationModel = CServerApplication::m_pTheAddressSpace;
	CServerStatus* pServerStatus = pInformationModel->GetInternalServerStatus();
	if (pServerStatus)
	{
		OpcUa_ServerState currentState = pServerStatus->GetServerState();
		if (m_LastKnownServerState != currentState)
		{
			OpcUa_Boolean bEventable = OpcUa_False;
			CUABase* pUABase = pMonitoredItem->GetUABase();
			if (pUABase)
			{
				switch (pUABase->GetNodeClass())
				{
				case OpcUa_NodeClass_Object:
				case OpcUa_NodeClass_View:
					bEventable = OpcUa_True;
					break;
				default:
					break;
				}
				if (bEventable)
				{
					if (pInformationModel->GetNodeIdFromObjectTypeList(SystemStatusChangeEventType, &pUAObjectType) == OpcUa_Good)
					{
						CEventDefinition* pRefreshStartEventDefinition = new CEventDefinition(OpcUa_Null, OpcUa_Null, OpcUa_Null, pUAObjectType);
						OpcUa_UInt32 uiSequenceNumber = GetCurrentSequenceNumber();
						CUAEventNotificationList* pUAEventNotificationList = new CUAEventNotificationList(uiSequenceNumber);
						if (pUAEventNotificationList)
						{
							// Set the associated MonitoredItem
							pUAEventNotificationList->SetMonitoredItem(pMonitoredItem);
							// Set the associate EventDefinition
							pUAEventNotificationList->SetEventDefinition(pRefreshStartEventDefinition);
							// Set the NotificationType
							pUAEventNotificationList->SetNotificationType(NOTIFICATION_MESSAGE_EVENT);
							uStatus = pMonitoredItem->InitializeEventsFieldsEx(pRefreshStartEventDefinition, &pUAEventNotificationList);
							if (uStatus == OpcUa_Good)
							{
								// Adjust the serverState
								FastEventFieldIndex aFastEventFieldIndex = pUAEventNotificationList->GetFastEventFieldIndex();
								OpcUa_Int32 iStateIndex = aFastEventFieldIndex.iSystemStateIndex;
								if (iStateIndex >= 0)
								{
									OpcUa_EventNotificationList* pEventNotificationList = pUAEventNotificationList->GetInternalEventNotificationList();
									if (pEventNotificationList->Events[0].EventFields)
									{
										pEventNotificationList->Events[0].EventFields[iStateIndex].Value.Int32 = currentState;
										if (AddEventNotificationList(pUAEventNotificationList) == OpcUa_Good)
											m_LastKnownServerState = currentState;
									}
								}
								else
								{
									// in this case the SystemStatusChanegEventType is not requested so it will not be return.
									// to avoid multiple call we copy the current state.
									m_LastKnownServerState = currentState;
									delete pUAEventNotificationList;
								}
							}
						}
					}
				}
			}
		}
	}
	return uStatus;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Handles the already notified event. </summary>
///
/// <remarks>	Michel, 10/08/2016. </remarks>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode OpenOpcUa::UACoreServer::CSubscriptionServer::HandleAlreadyNotifiedEvent(void)
{
	OpcUa_StatusCode uStatus = OpcUa_BadNothingToDo;
	OpcUa_Mutex_Lock(m_hEventNotificationListListMutex);
	for (OpcUa_UInt32 i = 0; i < m_pEventNotificationListList->size(); i++)
	{
		CUAEventNotificationList* pUALocalEventNotificationList = m_pEventNotificationListList->at(i);
		if (pUALocalEventNotificationList)
		{
			CEventDefinition* pEventDefinition = pUALocalEventNotificationList->GetEventDefinition();
			if (pEventDefinition)
			{
				OpcUa_EventNotificationList* pEventNotificationList = pUALocalEventNotificationList->GetInternalEventNotificationList();
				if (pEventNotificationList)
				{
					// evolution for the new notification mecanism (31-8-2016)
					if (pEventDefinition->GetActiveState() && (!pUALocalEventNotificationList->GetLastNotifiedActiveState()))
					{
						// ActiveState need to be notified to the client's subscription but it's not done already
						// Do nothing
						uStatus = OpcUa_BadNothingToDo;
					}
					else
					{
						// Not terminated													
						if (!pUALocalEventNotificationList->IsTerminated())
						{
							OpcUa_UInt32 uiEventStatus = pEventDefinition->GetTransactionStatus();
							switch (uiEventStatus)
							{
							case OPCUA_EVENTSTATUS_UNKNOWN:
							case OPCUA_EVENTSTATUS_TRANSITION:
								pUALocalEventNotificationList->SetTransactionStatus(uiEventStatus);
								uStatus=OpcUa_Good;
								break;
							}
							//if (pEventDefinition->GetTransactionStatus() ==OPCUA_EVENTSTATUS_TRANSITION)
							//{
							//	pUALocalEventNotificationList->SetTransactionStatus(OPCUA_EVENTSTATUS_TRANSITION);
							//	//pUALocalEventNotificationList->SetLastNotifiedActiveState(OpcUa_False);
							//	uStatus=OpcUa_Good;
							//}
							
							// Alarm Confirmed not notified
							if (pUALocalEventNotificationList->IsConfirmed() && (!pUALocalEventNotificationList->GetLastNotifiedConfirmed()))
							{
								// Update of AckedState and l'AckedState Id
								pUALocalEventNotificationList->UpdateInternalConfirmedState(OpcUa_True);
								// mark as not sent
								pUALocalEventNotificationList->SetTransactionStatus(OPCUA_EVENTSTATUS_UNKNOWN);
								uStatus = OpcUa_Good;
							}
							// Alarm Acked not notified
							if (pUALocalEventNotificationList->IsAlarmAcked() && (!pUALocalEventNotificationList->GetLastNotifiedAlarmAcked()))
							{
								pUALocalEventNotificationList->UpdateInternalDateTime(OpcUa_DateTime_UtcNow());
								// Update of AckedState and l'AckedState Id
								pUALocalEventNotificationList->UpdateInternalAckedState(OpcUa_True);
								// mark as not sent	
								pUALocalEventNotificationList->SetTransactionStatus(OPCUA_EVENTSTATUS_UNKNOWN);
								uStatus = OpcUa_Good;
							}
							// ActiveState change from the last notified
							if (pUALocalEventNotificationList->GetLastNotifiedActiveState() != pEventDefinition->GetActiveState())
							{
								if ((pEventDefinition->GetActiveState() == OpcUa_False))
								{
									pUALocalEventNotificationList->UpdateInternalDateTime(OpcUa_DateTime_UtcNow());
									// Update of AckedState and l'AckedState Id													
									pUALocalEventNotificationList->UpdateInternalActiveState(OpcUa_False);
									// mark as not sent
									pUALocalEventNotificationList->SetTransactionStatus(OPCUA_EVENTSTATUS_UNKNOWN);
									uStatus = OpcUa_Good;
								}

							}
						}
					}
				}
			}
		}
	}
	OpcUa_Mutex_Unlock(m_hEventNotificationListListMutex);
	return uStatus;	
}

