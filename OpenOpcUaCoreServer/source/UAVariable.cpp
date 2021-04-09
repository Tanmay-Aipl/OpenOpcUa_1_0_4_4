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

using namespace OpenOpcUa;
using namespace UAAddressSpace;

extern CServerApplication* g_pTheApplication;
extern OpcUa_Semaphore g_ShutdownServerSem;
extern OpcUa_Boolean g_bRun;

CUAVariable::CUAVariable(void)
{
	m_pDataValue=OpcUa_Null;
	OpcUa_Mutex_Create(&m_DataValueMutex);
		
	m_BuiltInType=0;
	// default AccessLevel
	m_bAccessLevel=1;
	// default UserAccessLevel
	m_UserAccessLevel=1;
	// default DataType
	OpcUa_NodeId_Initialize(&m_DataType);
	m_DataType.IdentifierType=OpcUa_IdentifierType_Numeric;
	m_DataType.Identifier.Numeric=OpcUaType_Null;
	m_DataType.NamespaceIndex=0;
	// default ValueRank
	m_iValueRank=-1;
	// default Historizing
	m_bHistorizing=false;
	// default MinimumSamplingInterval
	m_dblMinimumSamplingInterval=0.0;
	m_ArrayDimensions.empty();
	m_pData=OpcUa_Null;
}

CUAVariable::~CUAVariable(void)
{
	//if ((GetNodeId()->Identifier.Numeric == OpcUaId_Server_ServerRedundancy_RedundantServerArray) && (GetNodeId()->NamespaceIndex == 0))
	//{
	//	printf("debug destructorOpcUaId_Server_ServerRedundancy_RedundantServerArray\n");
	//	OpcUa_DataValue* pInternalDataValue=m_pDataValue->GetInternalDataValue();
	//	pInternalDataValue->Value.Value.Array.Value.ExtensionObjectArray[0].Body.EncodeableObject.Type;
	//}
	OpcUa_Mutex_Lock(m_DataValueMutex);
	if (m_pDataValue)
		delete m_pDataValue;
	OpcUa_NodeId_Clear(&m_DataType);
	OpcUa_Mutex_Unlock(m_DataValueMutex);
	OpcUa_Mutex_Delete(&m_DataValueMutex);
	m_ArrayDimensions.clear();
	// Clear the internalEventDefinition
	m_InternalEventDefinitionList.clear();
}
CUAVariable::CUAVariable(OpcUa_NodeClass aNodeClass,const char **atts):CUABase(aNodeClass,atts)
{
	m_pData=OpcUa_Null;
	m_BuiltInType=0;
	// called to get init value
	m_pDataValue=OpcUa_Null;
	OpcUa_Mutex_Create(&m_DataValueMutex);
	//if ((GetNodeId().Identifier.Numeric == 2256) && (GetNodeId().NamespaceIndex == 0))
	//	printf("debug destructor ns=0;i=2256\n");
	////////////////////////////////////////////////////
	// Begin init the default values
	// default AccessLevel
	m_bAccessLevel=1;
	// default UserAccessLevel
	m_UserAccessLevel=1;
	// default DataType = BaseDataType i=24 according to the nodeset
	OpcUa_NodeId_Initialize(&m_DataType);
	m_DataType.Identifier.Numeric = 24;
	// default ValueRank
	m_iValueRank=-1;
	// default Historizing
	m_bHistorizing=false;
	// default MinimumSamplingInterval
	m_dblMinimumSamplingInterval=0.0;
	m_ArrayDimensions.empty();	
	// End Init the default values
	////////////////////////////////////////////////////
	int ii=0;
	while (atts[ii])
	{
		if (OpcUa_StrCmpA(atts[ii],"AccessLevel")==0)
		{
			m_bAccessLevel=(OpcUa_Byte)atoi(atts[ii+1]);
		}
		if (OpcUa_StrCmpA(atts[ii],"UserAccessLevel")==0)
		{
			m_UserAccessLevel=(OpcUa_Byte)atoi(atts[ii+1]);
		}
		// si ValueRank=n 
		// n>1 => Tableau à n dimension, 
		// n=0 => Tableau d'une ou plusieurs dimension, 
		// n=-1 Il ne s'agit pas d'un tableau
		// n=-2 Il peut s'agir d'un scalaire ou d'un tableau a n dimension
		// n=-3 Il peut s'agir d'un scalaire ou d'un tableau a 1 dimension
		if (OpcUa_StrCmpA(atts[ii],"ValueRank")==0)
		{
			OpcUa_Int32 iVal=0;
			int iRes=sscanf_s(atts[ii+1],"%i",(int*)&iVal);
			if (!iRes)
			{
				// erreur de conversion
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"an incorrect ValueRank was used\n");
			}
			else			
				m_iValueRank=iVal;
		}
		// Cette valeur est lié a ValueRank et indique le nombre de valeur par dimension
		// il s'agit d'un tableau de UInt32. UInt32[]
		if (OpcUa_StrCmpA(atts[ii],"ArrayDimensions")==0)
		{
			// On va compter les dimensions du tableau
			// le nombre de dimension correspond au nombre de (,)+1
			// si 0 virgule et ArrayDimension =0 => dimension variable de longueur
			basic_string<char>::size_type index=0,firstPos=0;
			basic_string<char> myString(atts[ii+1]);
			while(index!=basic_string<char>::npos)
			{
				index=myString.find(",",index);			
				//lastPos=index;
				basic_string<char> tmpStr=myString.substr(firstPos,index-firstPos);			 
				m_ArrayDimensions.push_back(atoi(tmpStr.c_str()));
				firstPos=index;
			}
		}
		if (OpcUa_StrCmpA(atts[ii],"DataType")==0)
		{
			OpcUa_NodeId aNodeId;
			OpcUa_NodeId_Initialize(&aNodeId);					
			if (ParseNodeId(atts[ii+1],&aNodeId)==OpcUa_Good)
			{
				// ici on peu initialiser le BuiltInType de cette UAVariable directement à partir du DataType
				if ( (aNodeId.IdentifierType==OpcUa_IdentifierType_Numeric) && (aNodeId.Identifier.Numeric<=29))
					SetBuiltInType((OpcUa_Byte)aNodeId.Identifier.Numeric);
				// DataType
				SetDataType(aNodeId);
			}
			else
			{		
				aNodeId.Identifier.Numeric=1; // boolean by default
				aNodeId.IdentifierType=OpcUa_IdentifierType_Numeric;
				aNodeId.NamespaceIndex=0;
				SetDataType(aNodeId);
				SetBuiltInType(OpcUaType_Boolean);
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"CUAVariable::CUAVariable>Critical error ParseNodeId failed. You are using an undefined DataType\n");
			}
			OpcUa_NodeId_Clear(&aNodeId);
		}
		if (OpcUa_StrCmpA(atts[ii],"Historizing")==0)
		{	
			if (OpcUa_StrCmpA(atts[ii+1],"true")==0)
				m_bHistorizing=true;
			else
			{
				if (OpcUa_StrCmpA(atts[ii+1],"false")==0)
					m_bHistorizing=false;
				else
					m_bHistorizing=(OpcUa_Boolean)atoi(atts[ii+1]);
			}
		}
		if (OpcUa_StrCmpA(atts[ii],"MinimumSamplingInterval")==0)
		{
			m_dblMinimumSamplingInterval=atof(atts[ii+1]);
		}
		ii+=2;
	}	
}
OpcUa_StatusCode CUAVariable::Write(OpcUa_UInt32 AttributeId, OpcUa_String szIndexRange, OpcUa_DataValue Value)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	if (!IsWritable())
		uStatus=OpcUa_BadNotWritable;
	else
	{	
		switch (AttributeId)
		{
			case OpcUa_Attributes_Value:
			{
				Lock();
				CNumericRange* pNumericRange=OpcUa_Null;
				if (OpcUa_String_StrLen(&szIndexRange)>0)
					pNumericRange=new CNumericRange(&szIndexRange);

				// check for dataType consistency
				OpcUa_Byte bBuiltInType=GetBuiltInType();
				// Verify for 3 special case in BaseObjectType (Number, Integrer and UInteger)
				if (bBuiltInType == 24) // BaseObjectType
				{
					// special case Number
					if ((GetDataType().IdentifierType == OpcUa_IdentifierType_Numeric)
						&& (GetDataType().Identifier.Numeric == 26)) // Number
					{
						// Create the NodeId of the written OpcUa_Variant
						OpcUa_NodeId aNewDataTypeNodeId;
						OpcUa_NodeId_Initialize(&aNewDataTypeNodeId);
						aNewDataTypeNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
						aNewDataTypeNodeId.Identifier.Numeric = Value.Value.Datatype;
						switch (Value.Value.Datatype)
						{
						case OpcUaType_SByte:
						case OpcUaType_Int16:
						case OpcUaType_Int32:
						case OpcUaType_Int64:
							bBuiltInType = Value.Value.Datatype;
							break;
						default:
							break;
						}
					}
					// special case integer
					if ((GetDataType().IdentifierType == OpcUa_IdentifierType_Numeric)
						&& (GetDataType().Identifier.Numeric == 27)) // integer
					{
						// Create the NodeId of the written OpcUa_Variant
						OpcUa_NodeId aNewDataTypeNodeId;
						OpcUa_NodeId_Initialize(&aNewDataTypeNodeId);
						aNewDataTypeNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
						aNewDataTypeNodeId.Identifier.Numeric = Value.Value.Datatype;
						switch (Value.Value.Datatype)
						{
						case OpcUaType_SByte:
						case OpcUaType_Int16:
						case OpcUaType_Int32:
						case OpcUaType_Int64:
							bBuiltInType = Value.Value.Datatype;
							break;
						default:
							break;
						}
					}
					// special case UInteger
					if ((GetDataType().IdentifierType == OpcUa_IdentifierType_Numeric)
						&& (GetDataType().Identifier.Numeric == 28)) // UInteger
					{
						// Create the NodeId of the written OpcUa_Variant
						OpcUa_NodeId aNewDataTypeNodeId;
						OpcUa_NodeId_Initialize(&aNewDataTypeNodeId);
						aNewDataTypeNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
						aNewDataTypeNodeId.Identifier.Numeric = Value.Value.Datatype;
						switch (Value.Value.Datatype)
						{
						case OpcUaType_Byte:
						case OpcUaType_UInt16:
						case OpcUaType_UInt32:
						case OpcUaType_UInt64:
							bBuiltInType = Value.Value.Datatype;
							break;
						default:
							break;
						}
					}
				}
				if (Utils::IsDataTypeCompliant(bBuiltInType,Value.Value.Datatype))
				{
					// ecriture de la valeur AttributeId:13
					if (Value.Value.ArrayType==OpcUa_VariantArrayType_Array)
					{
						// Est ce que l'on a a faire a un range ?
						if (pNumericRange)
						{
							OpcUa_Variant aVariant=m_pDataValue->GetValue();
							// Est ce que ce range est valide ?
							if (pNumericRange->GetStatusCode()==OpcUa_Good) 
							{
								OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_WARNING, "Write> Range :%ls\n", pNumericRange->ToString());
								if ( (pNumericRange->GetBeginIndex()>0) ||(pNumericRange->GetEndIndex()>0) )
								{
									if (pNumericRange->GetEndIndex()>=pNumericRange->GetBeginIndex())
									{	
										// On va verifier que la taille demandé n'est pas supérieure a la taille actuelle du tableau
										//aVariant= GetValue()->GetValue();
										OpcUa_Int32 iRangeLen=(pNumericRange->GetEndIndex()-pNumericRange->GetBeginIndex())+1;
										if (iRangeLen>0)
										{
											if (pNumericRange->GetBeginIndex()<=iRangeLen)
											{
												if (aVariant.Value.Array.Length >= iRangeLen)
												{
													// ajustement pour prendre en compte les cas ou l'index de début est égale a 0
													OpcUa_Int32 iBeginIndex = 0;
													OpcUa_Int32 iEndIndex = 0;
													if (!pNumericRange->IsUnique())
														iEndIndex = pNumericRange->GetEndIndex() + 1;
													else
														iEndIndex = pNumericRange->GetEndIndex();
													switch (Value.Value.Datatype)
													{
													case OpcUaType_Boolean:
														for (OpcUa_Int32 ii = pNumericRange->GetBeginIndex(); ii < iEndIndex; ii++)
															aVariant.Value.Array.Value.BooleanArray[ii] = Value.Value.Value.Array.Value.BooleanArray[iBeginIndex++];
														break;
													case OpcUaType_Byte:
														for (OpcUa_Int32 ii = pNumericRange->GetBeginIndex(); ii < iEndIndex; ii++)
															aVariant.Value.Array.Value.ByteArray[ii] = Value.Value.Value.Array.Value.ByteArray[iBeginIndex++];
														break;
													case OpcUaType_SByte:
														for (OpcUa_Int32 ii = pNumericRange->GetBeginIndex(); ii < iEndIndex; ii++)
															aVariant.Value.Array.Value.SByteArray[ii] = Value.Value.Value.Array.Value.SByteArray[iBeginIndex++];
														break;
													case OpcUaType_Float:
														for (OpcUa_Int32 ii = pNumericRange->GetBeginIndex(); ii < iEndIndex; ii++)
															aVariant.Value.Array.Value.FloatArray[ii] = Value.Value.Value.Array.Value.FloatArray[iBeginIndex++];
														break;
													case OpcUaType_Double:
														for (OpcUa_Int32 ii = pNumericRange->GetBeginIndex(); ii < iEndIndex; ii++)
															aVariant.Value.Array.Value.DoubleArray[ii] = Value.Value.Value.Array.Value.DoubleArray[iBeginIndex++];
														break;
													case OpcUaType_DateTime:
														for (OpcUa_Int32 ii = pNumericRange->GetBeginIndex(); ii < iEndIndex; ii++)
															aVariant.Value.Array.Value.DateTimeArray[ii] = Value.Value.Value.Array.Value.DateTimeArray[iBeginIndex++];
														break;
													case OpcUaType_Guid:
														for (OpcUa_Int32 ii = pNumericRange->GetBeginIndex(); ii < iEndIndex; ii++)
														{
															aVariant.Value.Array.Value.GuidArray[ii].Data1 = Value.Value.Value.Array.Value.GuidArray[iBeginIndex].Data1;
															aVariant.Value.Array.Value.GuidArray[ii].Data2 = Value.Value.Value.Array.Value.GuidArray[iBeginIndex].Data2;
															aVariant.Value.Array.Value.GuidArray[ii].Data3 = Value.Value.Value.Array.Value.GuidArray[iBeginIndex].Data3;

															OpcUa_MemCpy(aVariant.Value.Array.Value.GuidArray[ii].Data4,
																8,
																Value.Value.Value.Array.Value.GuidArray[iBeginIndex].Data4,
																8);
															iBeginIndex++;
														}
														break;
													case OpcUaType_ByteString:
													{
														// On ne remplace pas la valeur en cours mais seul les indices concerné (valeur de range)
														// aVariant contient la valeur courante
														{
															for (OpcUa_Int32 ii = pNumericRange->GetBeginIndex(); ii < iEndIndex; ii++)
															{
																OpcUa_ByteString_Clear(&(aVariant.Value.Array.Value.ByteStringArray[ii]));
																OpcUa_ByteString_CopyTo(&(Value.Value.Value.Array.Value.ByteStringArray[iBeginIndex]), &(aVariant.Value.Array.Value.ByteStringArray[ii]));
															}
														}
													}
													break;
													case OpcUaType_String:
														for (OpcUa_Int32 ii = pNumericRange->GetBeginIndex(); ii < iEndIndex; ii++)
														{
															OpcUa_UInt32 iLen = OpcUa_String_StrLen(&Value.Value.Value.Array.Value.StringArray[iBeginIndex]);
															if (iLen > 0)
															{
																OpcUa_String_StrnCpy(
																	&(aVariant.Value.Array.Value.StringArray[ii]),
																	&(Value.Value.Value.Array.Value.StringArray[iBeginIndex]),
																	iLen);
																iBeginIndex++;
															}
														}
														break;
													default:
														break;
													}
												}
												else
													uStatus = OpcUa_BadOutOfRange;// OpcUa_BadIndexRangeInvalid;
											}
											else
												uStatus=OpcUa_BadIndexRangeNoData; //;OpcUa_BadWriteNotSupported
										}
										else
											uStatus = OpcUa_BadOutOfRange;// OpcUa_BadIndexRangeInvalid;
									}
									else
										uStatus = OpcUa_BadOutOfRange; // OpcUa_BadIndexRangeInvalid;
								}
								else
									uStatus = OpcUa_BadOutOfRange;//  OpcUa_BadIndexRangeInvalid;
							}
							else
								uStatus=pNumericRange->GetStatusCode();
						}
						else
						{
							OpcUa_Variant aVariant = m_pDataValue->GetValue();
							// Is it un uninitialize Array. I mean a aVariant.Value.Array.Length==0
							if (aVariant.Value.Array.Length == 0)
							{
								if (Value.Value.Value.Array.Length > 0)
								{
									m_pDataValue->InitializeArray(Value.Value.Datatype, Value.Value.Value.Array.Length);
									aVariant = m_pDataValue->GetValue();
								}
							}
							OpcUa_Int32 iEndIndex = aVariant.Value.Array.Length;
							switch (Value.Value.Datatype)
							{
							case OpcUaType_Boolean:
								for (OpcUa_Int32 ii = 0; ii<iEndIndex; ii++)
									aVariant.Value.Array.Value.BooleanArray[ii] = Value.Value.Value.Array.Value.BooleanArray[ii];
								break;
							case OpcUaType_Byte:
								for (OpcUa_Int32 ii = 0; ii<iEndIndex; ii++)
									aVariant.Value.Array.Value.ByteArray[ii] = Value.Value.Value.Array.Value.ByteArray[ii];
								break;
							case OpcUaType_SByte:
								for (OpcUa_Int32 ii = 0; ii<iEndIndex; ii++)
									aVariant.Value.Array.Value.SByteArray[ii] = Value.Value.Value.Array.Value.SByteArray[ii];
								break;
							case OpcUaType_Int16:
								for (OpcUa_Int32 ii = 0; ii<iEndIndex; ii++)
									aVariant.Value.Array.Value.Int16Array[ii] = Value.Value.Value.Array.Value.Int16Array[ii];
								break;
							case OpcUaType_Int32:
								for (OpcUa_Int32 ii = 0; ii<iEndIndex; ii++)
									aVariant.Value.Array.Value.Int32Array[ii] = Value.Value.Value.Array.Value.Int32Array[ii];
								break;
							case OpcUaType_UInt16:
								for (OpcUa_Int32 ii = 0; ii<iEndIndex; ii++)
									aVariant.Value.Array.Value.UInt16Array[ii] = Value.Value.Value.Array.Value.UInt16Array[ii];
								break;
							case OpcUaType_UInt32:
								for (OpcUa_Int32 ii = 0; ii<iEndIndex; ii++)
									aVariant.Value.Array.Value.UInt32Array[ii] = Value.Value.Value.Array.Value.UInt32Array[ii];
								break;
							case OpcUaType_Float:
								for (OpcUa_Int32 ii = 0; ii<iEndIndex; ii++)
									aVariant.Value.Array.Value.FloatArray[ii] = Value.Value.Value.Array.Value.FloatArray[ii];
								break;
							case OpcUaType_Double:
								for (OpcUa_Int32 ii = 0; ii<iEndIndex; ii++)
									aVariant.Value.Array.Value.DoubleArray[ii] = Value.Value.Value.Array.Value.DoubleArray[ii];
								break;
							case OpcUaType_DateTime:
								for (OpcUa_Int32 ii = 0; ii<iEndIndex; ii++)
									aVariant.Value.Array.Value.DateTimeArray[ii] = Value.Value.Value.Array.Value.DateTimeArray[ii];
								break;
							case OpcUaType_Guid:
								for (OpcUa_Int32 ii = 0; ii<iEndIndex; ii++)
								{
									aVariant.Value.Array.Value.GuidArray[ii].Data1 = Value.Value.Value.Array.Value.GuidArray[ii].Data1;
									aVariant.Value.Array.Value.GuidArray[ii].Data2 = Value.Value.Value.Array.Value.GuidArray[ii].Data2;
									aVariant.Value.Array.Value.GuidArray[ii].Data3 = Value.Value.Value.Array.Value.GuidArray[ii].Data3;

									OpcUa_MemCpy(aVariant.Value.Array.Value.GuidArray[ii].Data4,
										8,
										Value.Value.Value.Array.Value.GuidArray[ii].Data4,
										8);
								}
								break;
							case OpcUaType_ByteString:
								{
									// On ne remplace pas la valeur en cours mais seul les indices concerné (valeur de range)
									// aVariant contient la valeur courante
									{
										for (OpcUa_Int32 ii = 0; ii<iEndIndex; ii++)
										{
											// 
											OpcUa_ByteString_Clear(&aVariant.Value.Array.Value.ByteStringArray[ii]);
											OpcUa_ByteString_CopyTo(&(Value.Value.Value.Array.Value.ByteStringArray[ii]), &(aVariant.Value.Array.Value.ByteStringArray[ii]));
										}
									}
								}
								break;
							case OpcUaType_LocalizedText:
								for (OpcUa_Int32 ii = 0; ii<iEndIndex; ii++)
								{
									OpcUa_UInt32 iLen = OpcUa_String_StrLen(&Value.Value.Value.Array.Value.LocalizedTextArray[ii].Text);
									if (iLen > 0)
									{
										OpcUa_LocalizedText_CopyTo(
											&Value.Value.Value.Array.Value.LocalizedTextArray[ii],
											&aVariant.Value.Array.Value.LocalizedTextArray[ii]);
									}
									else
										OpcUa_String_Initialize(&(aVariant.Value.Array.Value.StringArray[ii]));
								}
								break;
							case OpcUaType_String:
								for (OpcUa_Int32 ii = 0; ii<iEndIndex; ii++)
								{
									OpcUa_UInt32 iLen = OpcUa_String_StrLen(&Value.Value.Value.Array.Value.StringArray[ii]);
									if (iLen > 0)
									{
										OpcUa_String_Clear(&(aVariant.Value.Array.Value.StringArray[ii]));
										OpcUa_String_Initialize(&(aVariant.Value.Array.Value.StringArray[ii]));
										OpcUa_String_StrnCpy(
											&(aVariant.Value.Array.Value.StringArray[ii]),
											&(Value.Value.Value.Array.Value.StringArray[ii]),
											iLen);
									}
									else
										OpcUa_String_Initialize(&(aVariant.Value.Array.Value.StringArray[ii]));
								}
								break;
							default:
								break;
							}
						
							uStatus = m_pDataValue->UpdateValue(aVariant); // on inclu tous les elements
						}
					}
					else
					{
						if (Value.Value.ArrayType == OpcUa_VariantArrayType_Scalar)
						{
							OpcUa_Variant aVariant = m_pDataValue->GetValue();
							OpcUa_Variant_CopyTo(&(Value.Value), &aVariant);
							uStatus = m_pDataValue->UpdateValue(aVariant);
							if (uStatus == OpcUa_Good)
								EvaluateEventDefinitionActiveState();
							// EnabledFlag ServerDiagnostic activate i=2294
							if ((GetNodeId()->Identifier.Numeric == 2294) && (GetNodeId()->NamespaceIndex==0))
							{
								// Update CSessionServer::EnableServerDiagnostics for all session
								g_pTheApplication->EnableServerDiagnostics(Value.Value.Value.Boolean);
							}
							// if ns=1;i=8 ==> Shutdown
							if ((GetNodeId()->Identifier.Numeric == 8) && (GetNodeId()->NamespaceIndex==1))
							{
								if (aVariant.Value.Boolean == (OpcUa_Boolean)OpcUa_True)
								{
									g_bRun = OpcUa_False;
									// Post a message to stop the server
									OpcUa_Semaphore_Post(g_ShutdownServerSem,1);
								}
							}
							// Modif mai 2015
							OpcUa_Variant_Clear(&aVariant);
						}
					}
					// Write Timestamp
					if ( (Value.ServerTimestamp.dwHighDateTime==0) && (Value.ServerTimestamp.dwLowDateTime==0) )
						m_pDataValue->SetServerTimestamp(CUAInformationModel::GetInternalServerStatus()->GetInternalCurrentTime());
					else
						m_pDataValue->SetServerTimestamp(Value.ServerTimestamp);
					//
					if ( (Value.SourceTimestamp.dwHighDateTime==0) && (Value.SourceTimestamp.dwLowDateTime==0) )
						m_pDataValue->SetServerTimestamp(CUAInformationModel::GetInternalServerStatus()->GetInternalCurrentTime());
					else
						m_pDataValue->SetSourceTimestamp(Value.SourceTimestamp);
					// Write StatusCode
					m_pDataValue->SetStatusCode(Value.StatusCode);
				}
				else
					uStatus=OpcUa_BadTypeMismatch;
				if (pNumericRange)
					delete pNumericRange;
				UnLock();
			}
			break;
///////////////////////////////////////////////////////////////
			case OpcUa_Attributes_NodeId:
			case OpcUa_Attributes_NodeClass:
			case OpcUa_Attributes_BrowseName:
			case OpcUa_Attributes_DisplayName:
			case OpcUa_Attributes_Description:
			case OpcUa_Attributes_WriteMask:
			case OpcUa_Attributes_UserWriteMask:
			case OpcUa_Attributes_IsAbstract:
				uStatus=CUABase::Write(AttributeId,szIndexRange,Value);
			break;
			case OpcUa_Attributes_DataType:
			{
				// changement de DataType
				// first check dataType itself. This mean the dataType of the Datatype is suppose to be a NodeId
				if (Value.Value.Datatype == OpcUaType_NodeId)
				{
					SetDataType(*(Value.Value.Value.NodeId));
				}
				else
					uStatus = OpcUa_BadInvalidArgument;
			}
			break;
			case OpcUa_Attributes_ValueRank:
			{
				// changement de type
			}
			break;
			case OpcUa_Attributes_ArrayDimensions:
			{
				// changement de type
			}
			break;
			case OpcUa_Attributes_AccessLevel:
			{
				// changement de AccessLevel
				// first check dataType itself. This mean the dataType of the AccessLevel is suppose to be a Byte
				if (Value.Value.Datatype == OpcUaType_Byte)
				{
					SetAccessLevel(Value.Value.Value.Byte);
				}
				else
					uStatus = OpcUa_BadInvalidArgument;
			}
			break;
			case OpcUa_Attributes_MinimumSamplingInterval:
			{
				// changement de MinimumSamplingInterval
				// first check dataType itself. This mean the dataType of the MinimumSamplingInterval is suppose to be a Double
				if (Value.Value.Datatype == OpcUaType_Double)
				{
					SetMinimumSamplingInterval(Value.Value.Value.Double);
				}
				else
					uStatus = OpcUa_BadInvalidArgument;
			}
			break;
			case OpcUa_Attributes_UserAccessLevel:
			{
				// changement de UserAccessLevel
				// first check dataType itself. This mean the dataType of the UserAccessLevel is suppose to be a Byte
				if (Value.Value.Datatype == OpcUaType_Byte)
				{
					SetUserAccessLevel(Value.Value.Value.Byte);
				}
				else
					uStatus = OpcUa_BadInvalidArgument;
			}
			break;
			case OpcUa_Attributes_Historizing:
			{
				// changement de Historizing
				// first check dataType itself. This mean the dataType of the Historizing is suppose to be a Boolean
				if (Value.Value.Datatype == OpcUaType_Boolean)
				{
					SetHistorizing(Value.Value.Value.Byte);
				}
				else
					uStatus = OpcUa_BadInvalidArgument;
			}
			break;
			default:
				uStatus = OpcUa_BadAttributeIdInvalid;
			break;
		}
		// wake up all subscription thread where this UAVariable is monitored
		//if (uStatus==OpcUa_Good)
		//	g_pTheApplication->WakeupAllSubscription(); // This fix some CTT script and corrupt others like Monitor Basic 004.js and 006.js
	}
	return uStatus;
}

OpcUa_StatusCode CUAVariable::InitializeDataValue()
{
	OpcUa_StatusCode uStatus=OpcUa_Good;

	if (!m_pDataValue)
	{
		try
		{
		m_pDataValue = new CDataValue();
		}
		catch (...)
		{
			OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"A critical error happen. Please contact Michel Condemine\n");
		}
		if (m_pDataValue)
		{
			if (m_BuiltInType==0)
			{
				uStatus= FindBuiltinType(GetDataType(), &m_BuiltInType);
				if (uStatus!=OpcUa_Good)
				{
					char* szNodeId = OpcUa_Null;
					// NodeId
					OpcUa_NodeId nodeId = GetDataType();
					Utils::NodeId2String(&nodeId,&szNodeId);
					if (szNodeId)
					{
						// Attribute
						OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Warning FindBuiltinType %s failed 0x%05x\n",szNodeId,uStatus);
						free(szNodeId);
					}
					return uStatus;
				}
			}
			uStatus=m_pDataValue->Initialize(
				GetBuiltInType(),
				GetValueRank(),
				GetArrayDimensions());
			m_pDataValue->SetStatusCode(OpcUa_UncertainInitialValue);
			m_pDataValue->SetServerPicoseconds(0);
			m_pDataValue->SetSourcePicoseconds(0);
		}
		else
			uStatus=OpcUa_BadOutOfMemory;
	}
	else
		uStatus=OpcUa_BadNothingToDo;
	return uStatus;
}
// extract the binary content of the encapsulated m_pDataValue
// This method will be used to fill the EcodeableObject Object attribute
OpcUa_Void* CUAVariable::Dump()
{
	OpcUa_Void* pVoid=OpcUa_Null;
	OpcUa_Byte	bType=GetBuiltInType();
	switch (bType)
	{
	case OpcUaType_Boolean:
		break;
	case OpcUaType_ExtensionObject:
		{
			// Is it a well known or a user define dataType ?
		//	OpcUa_EncodeableType* pEncodeableType=OpcUa_Null;
		//	CDataValue* pDataValue=GetValue();
		//	if (pDataValue)
		//	{
		//		OpcUa_Variant aVariant=pDataValue->GetValue();
		//		if (aVariant.ArrayType==OpcUa_VariantArrayType_Array)
		//		{
		//			for (OpcUa_UInt32 ii=0;ii<aVariant.Value.Array.Length;ii++)
		//			{
		//				aVariant.Value.Array.Value.ExtensionObjectArray[ii].Body.EncodeableObject.Object;
		//			}
		//		}
		//		else
		//		{
		//			if (aVariant.ArrayType==OpcUa_VariantArrayType_Scalar)
		//			{
		//				;
		//			}
		//			else
		//				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"Warning MAtrix not supported\n");
		//		}
		//	}
		}
		break;
	default:
		break;
	}
	return pVoid;
}

CDataValue* CUAVariable::GetValue()
{
	//OpcUa_Mutex_Lock(m_DataValueMutex);
	return m_pDataValue;
	//OpcUa_Mutex_Unlock(m_DataValueMutex);
}
// Transfert pDataValue in pInternalDataValue both OpcUa_DataValue
// Also update the class variable
// After the call pDataValue can be release
void CUAVariable::SetValue(OpcUa_DataValue* pDataValue)
{
	if (pDataValue)
	{
		OpcUa_DataValue* pInternalDataValue = OpcUa_Null;
		if (m_pDataValue)
		{
			pInternalDataValue = m_pDataValue->GetInternalDataValue();
			if (pInternalDataValue)
				OpcUa_DataValue_Clear(pInternalDataValue);
			OpcUa_DataValue_Initialize(pInternalDataValue);
			OpcUa_DataValue_CopyTo(pDataValue, pInternalDataValue);
		}
		else
			m_pDataValue=new CDataValue(pDataValue);
		// timestamp
		m_pDataValue->SetServerTimestamp(pDataValue->ServerTimestamp);
		m_pDataValue->SetServerPicoseconds(pDataValue->ServerPicoseconds);
		m_pDataValue->SetSourceTimestamp(pDataValue->SourceTimestamp);
		m_pDataValue->SetSourcePicoseconds(pDataValue->SourcePicoseconds);
		// StatusCode
		m_pDataValue->SetStatusCode(pDataValue->StatusCode);
		// Event ?
		EvaluateEventDefinitionActiveState();
	}
}
void CUAVariable::SetValue(CDataValue* pValue)
{
	OpcUa_Mutex_Lock(m_DataValueMutex);
	// embedded Variant
	if (pValue != OpcUa_Null)
	{
		if (!m_pDataValue)
			InitializeDataValue();
		OpcUa_Variant aVariant = pValue->GetValue();
		if (pValue->GetStatusCode() == OpcUa_Good)
		{
			m_pDataValue->SetValue(aVariant);
		}
		// timestamp
		m_pDataValue->SetServerTimestamp(pValue->GetServerTimestamp());
		m_pDataValue->SetServerPicoseconds(pValue->GetServerPicoseconds());
		m_pDataValue->SetSourceTimestamp(pValue->GetSourceTimeStamp());
		m_pDataValue->SetSourcePicoseconds(pValue->GetSourcePicoseconds());
		// StatusCode
		m_pDataValue->SetStatusCode(pValue->GetStatusCode());
		// Event ?
		EvaluateEventDefinitionActiveState();
	}
	OpcUa_Mutex_Unlock(m_DataValueMutex);
}

OpcUa_StatusCode CUAVariable::SetValue(OpcUa_Variant aVariantValue)
{
	OpcUa_Mutex_Lock(m_DataValueMutex);
	OpcUa_StatusCode  uStatus = OpcUa_Good;
	if (m_pDataValue)
		m_pDataValue->SetValue(aVariantValue);
	else
		uStatus = OpcUa_BadInvalidArgument;
	OpcUa_Mutex_Unlock(m_DataValueMutex);
	// Event ?
	EvaluateEventDefinitionActiveState();
	return uStatus;
}

// DataType
OpcUa_NodeId CUAVariable::GetDataType()
{
	return m_DataType;
}
void CUAVariable::SetDataType(OpcUa_NodeId aValue)
{
	OpcUa_NodeId_Initialize(&m_DataType);
	OpcUa_NodeId_CopyTo(&aValue, &m_DataType);
}
// ValueRank
OpcUa_Int32 CUAVariable::GetValueRank()
{
	return m_iValueRank;
}
void CUAVariable::SetValueRank(OpcUa_Int32 aValue)
{
	m_iValueRank = aValue;
}
std::vector<OpcUa_UInt32>* CUAVariable::GetArrayDimensions()
{
	return &m_ArrayDimensions;
}
// NoOfArrayDimensions
OpcUa_Int32 CUAVariable::GetNoOfArrayDimensions()
{
	return m_ArrayDimensions.size();
}
OpcUa_UInt32 CUAVariable::operator [](int index)
{
	return (OpcUa_UInt32)m_ArrayDimensions[index];
}

OpcUa_Boolean CUAVariable::IsWritable()
{
	if (m_bAccessLevel&OpcUa_AccessLevels_CurrentWrite)
		return true;
	else
		return false;
}
OpcUa_Boolean CUAVariable::IsReadable()
{
	if (m_bAccessLevel&OpcUa_AccessLevels_CurrentRead)
		return true;
	else
		return false;
}
OpcUa_Boolean CUAVariable::IsHistoryWritable()
{
	if (m_bAccessLevel&OpcUa_AccessLevels_HistoryWrite) // was 4 (need to be checked)
		return true;
	else
		return false;
}
OpcUa_Boolean CUAVariable::IsHistoryReadable()
{
	if (m_bAccessLevel&OpcUa_AccessLevels_HistoryRead) // was 8 (need to be checked)
		return true;
	else
		return false;
}

OpcUa_Byte CUAVariable::GetAccessLevel() 
{ 
	return m_bAccessLevel;
}
void  CUAVariable::SetAccessLevel(OpcUa_Byte bAccessLevel) 
{ 
	m_bAccessLevel = bAccessLevel; 
}
OpcUa_Byte CUAVariable::GetUserAccessLevel() 
{ 
	return m_UserAccessLevel; 
}
void  CUAVariable::SetUserAccessLevel(OpcUa_Byte bUserAccessLevel) 
{ 
	m_UserAccessLevel = bUserAccessLevel; 
}
OpcUa_Double  CUAVariable::GetMinimumSamplingInterval() 
{ 
	return m_dblMinimumSamplingInterval; 
}
void CUAVariable::SetMinimumSamplingInterval(OpcUa_Double dblVal) 
{ 
	m_dblMinimumSamplingInterval = dblVal; 
}
OpcUa_Boolean CUAVariable::GetHistorizing() 
{ 
	return m_bHistorizing; 
}
void  CUAVariable::SetHistorizing(OpcUa_Boolean bHistorizing) 
{ 
	m_bHistorizing = bHistorizing; 
}
void*	CUAVariable::GetPData()	
{ 
	return  m_pData; 
}
void CUAVariable::SetPData(void* pData) 
{ 
	m_pData = pData; 
}
// Accesseur BuiltInType
OpcUa_Byte	CUAVariable::GetBuiltInType() 
{ 
	return  m_BuiltInType; 
}
void CUAVariable::SetBuiltInType(OpcUa_Byte bVal) 
{ 
	m_BuiltInType = bVal; 
}
void CUAVariable::Lock() 
{ 
	OpcUa_Mutex_Lock(m_DataValueMutex); 
}
void CUAVariable::UnLock() 
{ 
	OpcUa_Mutex_Unlock(m_DataValueMutex); 
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Adds an internal event definition. </summary>
///
/// <remarks>	Michel, 30/08/2016. </remarks>
///
/// <param name="pInternalEventDefinition">	[in,out] If non-null, the internal event definition.
/// </param>
///-------------------------------------------------------------------------------------------------

void OpenOpcUa::UAAddressSpace::CUAVariable::AddInternalEventDefinition(void* pInternalEventDefinition)
{
	m_InternalEventDefinitionList.push_back(pInternalEventDefinition);
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Evaluate event definition active state. </summary>
///
/// <remarks>	Michel, 30/08/2016. </remarks>
///-------------------------------------------------------------------------------------------------

void OpenOpcUa::UAAddressSpace::CUAVariable::EvaluateEventDefinitionActiveState(void)
{
	for (OpcUa_UInt32 i = 0; i < m_InternalEventDefinitionList.size(); i++)
	{
		CEventDefinition* pEventDefinition = (CEventDefinition*)m_InternalEventDefinitionList.at(i);
		pEventDefinition->EvaluateStateMachine();
	}
}

