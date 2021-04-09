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
#include "Field.h"
//#include "EventDefinition.h"
#include "Definition.h"
//////////////////////////////////////////////////////////////////////////

#include "UAReferenceType.h"
#include "UAObjectType.h"
#include "UADataType.h"
#include "UAVariableType.h"
#include "UAMethod.h"
#include "UAView.h"
#include "UAObject.h"
#include "Alias.h"
#include "SimulatedNode.h"
#include "SimulatedGroup.h"
#include "NamespaceUri.h"
#include "BuildInfo.h"
#include "ServerStatus.h"
#include "VpiWriteObject.h"
using namespace OpenOpcUa;
using namespace UASubSystem;
#include "VpiFuncCaller.h"
#include "VpiTag.h"
#include "VpiDevice.h"
#include "VPIScheduler.h"

#include "ContinuationPoint.h"
#include "SessionSecurityDiagnosticsDataType.h"
#include "SessionDiagnosticsDataType.h"
#include "SubscriptionDiagnosticsDataType.h"
#include "luainc.h"
#include "LuaVirtualMachine.h"
#include "LuaScript.h"
#include "OpenOpcUaScript.h"
using namespace UAScript;

#include "QueuedCallRequest.h"
#include "QueuedReadRequest.h"
#include "UAMonitoredItemNotification.h"
#include "UADataChangeNotification.h"
#include "UAStatusChangeNotification.h"
#include "EventDefinition.h"
#include "EventsEngine.h"
using namespace UAEvents;
#include "UAInformationModel.h"
#include "MonitoredItemServer.h"
#include "UAEventNotificationList.h"
#include "QueuedPublishRequest.h"
#include "SubscriptionServer.h"
#include "QueuedHistoryReadRequest.h"
#include "QueuedQueryFirstRequest.h"
#include "QueuedQueryNextRequest.h"
#include "SessionServer.h"
#include "UABinding.h"
#include "UAHistorianVariable.h"
#include "VfiDataValue.h"
#include "HaEngine.h"
using namespace UAHistoricalAccess;

#include "ServerApplication.h"
extern CServerApplication* g_pTheApplication;
//////////////////////////////////////////////////////////////////////////
using namespace OpenOpcUa;
using namespace UAAddressSpace;
CDefinition::CDefinition(void)
{
	m_FieldList.clear();

	OpcUa_String_Initialize(&m_SymbolicName);
	OpcUa_QualifiedName_Initialize(&m_BaseType);
	OpcUa_QualifiedName_Initialize(&m_Name);
}
CDefinition::CDefinition(const char **atts)
{
	m_FieldList.clear();

	OpcUa_String_Initialize(&m_SymbolicName);
	OpcUa_QualifiedName_Initialize(&m_BaseType);
	OpcUa_QualifiedName_Initialize(&m_Name);
	int ii=0;
	while (atts[ii])
	{
		if (OpcUa_StrCmpA(atts[ii],"Name")==0)
		{
			OpcUa_QualifiedName* qName=(OpcUa_QualifiedName*)OpcUa_Alloc(sizeof(OpcUa_QualifiedName));
			OpcUa_QualifiedName_Initialize(qName);	
			//int iSize=strlen(atts[ii+1])+1;
			if (strlen(atts[ii+1])>0)
				OpcUa_String_AttachCopy(&(qName->Name),(OpcUa_StringA)atts[ii+1]);
			else
			{
				//qName->Name.strContent=(OpcUa_CharA*)OpcUa_Alloc(1);
				OpcUa_String_AttachCopy(&(qName->Name),OpcUa_StringA(" "));
			}
			SetName(qName);
			OpcUa_QualifiedName_Clear(qName);
			OpcUa_Free(qName);
		}
		if (OpcUa_StrCmpA(atts[ii],"BaseType")==0)
		{
			OpcUa_QualifiedName* qName=(OpcUa_QualifiedName*)OpcUa_Alloc(sizeof(OpcUa_QualifiedName));
			OpcUa_QualifiedName_Initialize(qName);	
			OpcUa_String_AttachCopy(&(qName->Name),atts[ii+1]);
			SetBaseType(qName);
			OpcUa_QualifiedName_Clear(qName);
			OpcUa_Free(qName);
		}
		if (OpcUa_StrCmpA(atts[ii],"SymbolicName")==0)
		{
			OpcUa_String aString;
			OpcUa_String_Initialize(&aString);
			//int iSize=strlen(atts[ii+1])+1;
			if (strlen(atts[ii+1])>0)
			{
				OpcUa_String_AttachCopy(&aString,(OpcUa_StringA)atts[ii+1]);
			}
			else
				OpcUa_String_AttachCopy(&aString,OpcUa_StringA(" "));
			SetSymbolicName(&aString);
			OpcUa_String_Clear(&aString);
		}
		ii=ii+2;
	}
}
CDefinition::~CDefinition(void)
{
	std::vector<CField*>::iterator it;
	while (!m_FieldList.empty())	
	{
		it=m_FieldList.begin();
		delete *it;
		m_FieldList.erase(it);
	}
	OpcUa_String_Clear(&m_SymbolicName);
	OpcUa_QualifiedName_Clear(&m_BaseType);
	OpcUa_QualifiedName_Clear(&m_Name);
}
OpcUa_StatusCode CDefinition::GetInstanceSize(OpcUa_Int32 *pSize)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_Int32 iSize=0;

	for (OpcUa_UInt16 ii=0;ii<m_FieldList.size();ii++)
	{
		CField* pField=m_FieldList.at(ii);
		pField->GetDataType();
		OpcUa_Byte BuiltInType=0;
		uStatus=FindBuiltinType(pField->GetDataType(),&BuiltInType);
		if (uStatus==OpcUa_Good)
		{
			switch (BuiltInType)
			{
			case OpcUaType_ExtensionObject:
				break;
			case OpcUaType_ByteString:
				iSize+=sizeof(OpcUa_Byte*);
				iSize+=4; //Length
				break;
			case OpcUaType_DataValue:
				break;
			case OpcUaType_LocalizedText:
				iSize+=12;//OpcUa_String_StrLen(&(aVariant.Value.LocalizedText->Locale));
				iSize+=6; // uLength + flag
				iSize+=12; //OpcUa_String_StrLen(&(aVariant.Value.LocalizedText->Text));
				iSize+=6; // uLength + flag
				break;
			case OpcUaType_QualifiedName:
				iSize+=12;
				iSize+=6; // uLength + flag
				iSize+=4; // NamespaceIndex + Reserved
				break;
			case OpcUaType_String:
				iSize+=12; // uLength + flag + strContent
				break;
			case OpcUaType_Variant:
				break;
			default:
				iSize+=pField->GetFieldSize();
				break;
			}
		}
	}
	if (uStatus==OpcUa_Good)
		*pSize=iSize;

	return uStatus;
}
/// <summary>
/// Duplicates the extension object based on its DataType definition.
/// </summary>
/// <param name="pExtensionObject">input OpcUa_ExtensionObject* pExtensionObject</param>
/// <param name="ppNewExtensionObject">Output OpcUa_ExtensionObject** ppNewExtensionObject</param>
/// <returns></returns>
OpcUa_StatusCode CDefinition::DuplicateExtensionObject(OpcUa_ExtensionObject* pExtensionObject, OpcUa_ExtensionObject** ppNewExtensionObject)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_Int32 iSize = 0;
	CUAInformationModel* pInformationModel = CServerApplication::m_pTheAddressSpace;
	if (pExtensionObject)
	{
		OpcUa_EncodeableType* pEncodeableType = pExtensionObject->Body.EncodeableObject.Type;
		if (pEncodeableType)
		{
			// create a "calculation void"
			void* pVoidBuf = OpcUa_Alloc(pEncodeableType->AllocationSize);
			ZeroMemory(pVoidBuf, pEncodeableType->AllocationSize);

			// create a save buffer for freeing memory
			void* pVoidBufSave = pVoidBuf; 

			// create a  "result void"
			void* pVoidResult = OpcUa_Alloc(pEncodeableType->AllocationSize); // here is the result
			ZeroMemory(pVoidResult, pEncodeableType->AllocationSize);

			// create a "save result void"
			void* pVoidResultSave = pVoidResult;

			// Fill the "calculation void" with the content of the EncodableObject.Object
			OpcUa_MemCpy(
				pVoidBuf,
				pEncodeableType->AllocationSize,
				pExtensionObject->Body.EncodeableObject.Object,
				pEncodeableType->AllocationSize);
			// iterate through FiedlList to fill the "result void"
			for (OpcUa_UInt16 ii = 0; ii < m_FieldList.size(); ii++)
			{
				CField* pField = m_FieldList.at(ii);
				if (pField)
				{
					OpcUa_UInt32 iuNoOf = 0; // used only in case of arrayOfAttribute. Will got a iuNoOfxxxxx follow by xxxxxxAttribute ptr
					//OpcUa_UInt32 iFielSize = pField->GetFieldSize();
					pField->GetDataType();
					pField->GetValueRank(); // The case of array need to be implemented ValueRank=1
					OpcUa_Byte BuiltInType = 0;
					uStatus = FindBuiltinType(pField->GetDataType(), &BuiltInType);
					if (uStatus == OpcUa_Good)
					{
						switch (BuiltInType)
						{
						case OpcUaType_ExtensionObject:
						{

							if (pField->GetValueRank() == 1)
							{
								OpcUa_MemCpy(&iuNoOf, 4, pVoidBuf, 4);
							}
							else
							{
								OpcUa_UInt32 iType = pField->GetDataType().Identifier.Numeric;
								OpcUa_EncodeableType* pEmbeddedEncodeableType = OpcUa_Null;// (OpcUa_EncodeableType*)OpcUa_Alloc(sizeof(OpcUa_EncodeableType));
								// Encapsulated ExtensionObject
								OpcUa_ExtensionObject* pEncapsulatedExtensionObject = (OpcUa_ExtensionObject*)OpcUa_Alloc(sizeof(OpcUa_ExtensionObject));
								if (pEncapsulatedExtensionObject)
								{
									OpcUa_ExtensionObject_Initialize(pEncapsulatedExtensionObject);
									if (g_pTheApplication->LookupEncodeableType(iType, &pEmbeddedEncodeableType) == OpcUa_Good)
									{
										OpcUa_ExtensionObject* pTargetExtensionObject = OpcUa_Null;
										// We need the size of this extensionObject
										OpcUa_NodeId aDatatype = pField->GetDataType();
										//CNamespaceUri* pNameSpaceUri = pInformationModel->GetNamespaceUri(aDatatype.NamespaceIndex);
										//OpcUa_String* pString = pNameSpaceUri->GetUriName();
										//if (pString)
										//	OpcUa_String_CopyTo(pString, &(pEncapsulatedExtensionObject->TypeId.NamespaceUri));
										pEncapsulatedExtensionObject->TypeId.ServerIndex = aDatatype.NamespaceIndex;
										pEncapsulatedExtensionObject->TypeId.NodeId = aDatatype;
										pEncapsulatedExtensionObject->Encoding = OpcUa_ExtensionObjectEncoding_EncodeableObject;
										pEncapsulatedExtensionObject->Body.EncodeableObject.Type = pEmbeddedEncodeableType; //  Utils::Copy(pEmbeddedEncodeableType); modif mai 2015
										pEncapsulatedExtensionObject->Body.EncodeableObject.Object = (void*)pVoidBuf;
										// 
										CUADataType* pEncapsulatedDataType = OpcUa_Null;
										if (pInformationModel->GetNodeIdFromDataTypeList(aDatatype, &pEncapsulatedDataType) == OpcUa_Good)
										{
											CDefinition* pDefinition = pEncapsulatedDataType->GetDefinition();
											if (pDefinition)
											{
												if (pDefinition->DuplicateExtensionObject(pEncapsulatedExtensionObject, &pTargetExtensionObject) == OpcUa_Good)
												{
													OpcUa_UInt32 iEncodeableObjectSize = pEmbeddedEncodeableType->AllocationSize;

													OpcUa_MemCpy(pVoidResult, iEncodeableObjectSize, pTargetExtensionObject->Body.EncodeableObject.Object, iEncodeableObjectSize);
													((OpcUa_Byte*&)pVoidBuf) += iEncodeableObjectSize;
													((OpcUa_Byte*&)pVoidResult) += iEncodeableObjectSize; //
												}
											}
										}
										if (pTargetExtensionObject)
										{
											//OpcUa_ExtensionObject_Clear(pTargetExtensionObject);  // No need to free the pTargetExtensionObject->Body.EncodeableObject.Object. This will be done at the end of the run
											OpcUa_Free(pTargetExtensionObject);
											pTargetExtensionObject = OpcUa_Null;
										}
										//OpcUa_ExtensionObject_Clear(pEncapsulatedExtensionObject);
										OpcUa_Free(pEncapsulatedExtensionObject);
										pEncapsulatedExtensionObject = OpcUa_Null;
									}
									else
									{
										OpcUa_ExtensionObject_Clear(pEncapsulatedExtensionObject); // modif mai 2015
										OpcUa_Free(pEncapsulatedExtensionObject);
										pEncapsulatedExtensionObject = OpcUa_Null;
									}
								}
							}
						}
							break;
						case OpcUaType_NodeId:
						{
							if (pField->GetValueRank() == 1)
							{
								OpcUa_MemCpy(&iuNoOf, 4, pVoidBuf, 4);
								for (OpcUa_UInt32 iii = 0; iii < iuNoOf; iii++)
									DuplicateNodeId(&pVoidBuf, &pVoidResult);
							}
							else
								DuplicateNodeId(&pVoidBuf, &pVoidResult);
						}
							break;
						case OpcUaType_ByteString:
						{
							if (pField->GetValueRank() == 1)
							{
								OpcUa_MemCpy(&iuNoOf, 4, pVoidBuf, 4);
							}
							else
							{
								OpcUa_ByteString* pByteString = (OpcUa_ByteString*)OpcUa_Alloc(sizeof(OpcUa_ByteString));
								if (pByteString)
								{
									OpcUa_ByteString_Initialize(pByteString);
									// copy Length
									OpcUa_MemCpy(&(pByteString->Length), 4, pVoidBuf, 4);
									OpcUa_MemCpy(pVoidResult, 4, pVoidBuf, 4);
									((OpcUa_Byte*&)pVoidResult) += 4; // 
									// Copy the Data
									((OpcUa_Byte*&)pVoidBuf) += 4;
									void* apVoid = OpcUa_Alloc(4);
									memcpy(&apVoid, pVoidBuf, 4);
									if (pByteString->Length > 0)
									{
										pByteString->Data = (OpcUa_Byte*)OpcUa_Alloc(pByteString->Length);
										if (pByteString->Data)
										{
											ZeroMemory(pByteString->Data, pByteString->Length);
											OpcUa_MemCpy((pByteString->Data), pByteString->Length, ((void*)(apVoid)), pByteString->Length);
											OpcUa_MemCpy(pVoidResult, 4, &(pByteString->Data), 4);
											((OpcUa_Byte*&)pVoidBuf) += 4; //
											((OpcUa_Byte*&)pVoidResult) += 4; // 
										}
										else
											OpcUa_Free(pByteString);
									}
									else
										OpcUa_Free(pByteString);
								}
							}
						}
							break;
						case OpcUaType_DataValue:
							break;
						case OpcUaType_LocalizedText:
						{
							if (pField->GetValueRank() == 1)
							{
								OpcUa_MemCpy(&iuNoOf, 4, pVoidBuf, 4);
							}
							else
							{
								// Duplicate a LocalizedText
								// Duplicate Locale
								DuplicateString(&pVoidBuf, &pVoidResult);
								// Duplicate Text
								DuplicateString(&pVoidBuf, &pVoidResult);
							}
						}
							break;
						case OpcUaType_QualifiedName:
						{
							if (pField->GetValueRank() == 1)
							{
								OpcUa_MemCpy(&iuNoOf, 4, pVoidBuf, 4);
							}
							else
							{
								// pQualifiedName->NamespaceIndex;
								OpcUa_MemCpy(pVoidResult, 2, pVoidBuf, 2);
								((OpcUa_Byte*&)pVoidResult) += 2;
								((OpcUa_Byte*&)pVoidBuf) += 2;
								// pQualifiedName->Reserved;
								OpcUa_MemCpy(pVoidResult, 2, pVoidBuf, 2);
								((OpcUa_Byte*&)pVoidResult) += 2;
								((OpcUa_Byte*&)pVoidBuf) += 2;
								// pQualifiedName->Name;
								DuplicateString(&pVoidBuf, &pVoidResult);
							}
						}
							break;
						case OpcUaType_String:
						{
							if (pField->GetValueRank() == 1)
							{
								OpcUa_MemCpy(&iuNoOf, 4, pVoidBuf, 4);
								if (iuNoOf == 0)
								{
									//	// jump 4 bytes, sizeof string. Only 4 are enough because we already jump for in advance
									ZeroMemory(pVoidResult, 4);
									((OpcUa_Byte*&)pVoidResult) += 8;
									((OpcUa_Byte*&)pVoidBuf) += 8;
								}
								else
								{
									// Copy the length
									OpcUa_MemCpy(pVoidResult, 4, pVoidBuf, 4);
									((OpcUa_Byte*&)pVoidResult) += 4;
									((OpcUa_Byte*&)pVoidBuf) += 4;
									// Now the content of the array
									void* apVoid = OpcUa_Alloc(4);
									memcpy(&apVoid, pVoidBuf, 4);
									OpcUa_String* pString = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String)*iuNoOf);
									if (pString)
									{
										for (OpcUa_UInt32 iii = 0; iii < iuNoOf; iii++)
											OpcUa_String_CopyTo(&(((OpcUa_String*)apVoid)[iii]), &pString[iii]);
										// Transfert the new string to the result buffer
										OpcUa_MemCpy(pVoidResult, 4, (void*)&pString, 4);
									}
									// move the pointer to continue the decoding of the extension object
									((OpcUa_Byte*&)pVoidBuf) += 4;
									((OpcUa_Byte*&)pVoidResult) += 4;
								}
							}
							else
								DuplicateString(&pVoidBuf, &pVoidResult);
						}
							break;
						case OpcUaType_Variant:
							break;
						case 29: // Enumeration
						{
							OpcUa_MemCpy(pVoidResult, 4, pVoidBuf, 4);
							// move the calculation pointer 
							((OpcUa_Byte*&)pVoidBuf) += 4;
							// move the result pointer
							((OpcUa_Byte*&)pVoidResult) += 4;
						}
							break;
						default:
						{
							iSize = pField->GetFieldSize();
							if (iSize > 0)
							{
								if (pField->GetValueRank() == 1)
								{
									OpcUa_MemCpy(&iuNoOf, 4, pVoidBuf, 4);
									// move the calculation pointer 
									((OpcUa_Byte*&)pVoidBuf) += 4;
									// move the result pointer
									((OpcUa_Byte*&)pVoidResult) += 4;
									if (iuNoOf > 0)
									{
										for (OpcUa_UInt32 iii = 0; iii < iuNoOf; iii++)
										{
											OpcUa_MemCpy(pVoidResult, iSize, pVoidBuf, iSize);
											// move the calculation pointer 
											((OpcUa_Byte*&)pVoidBuf) += iSize;
											// move the result pointer
											((OpcUa_Byte*&)pVoidResult) += iSize;
										}
									}
									else
									{
										// move the calculation pointer 
										((OpcUa_Byte*&)pVoidBuf) += iSize;
										// move the result pointer
										((OpcUa_Byte*&)pVoidResult) += iSize;
									}
								}
								else
								{
									OpcUa_MemCpy(pVoidResult, iSize, pVoidBuf, iSize);
									// move the calculation pointer 
									((OpcUa_Byte*&)pVoidBuf) += iSize;
									// move the result pointer
									((OpcUa_Byte*&)pVoidResult) += iSize;
								}
							}
						}
							break;
						}
					}
				}
				else
					uStatus = OpcUa_BadInternalError;
			}
			// If everything goes right during the duplication steps then uStatus==OpcUa_Good
			if (uStatus == OpcUa_Good)
			{
				if (!(*ppNewExtensionObject))
					(*ppNewExtensionObject) = (OpcUa_ExtensionObject*)OpcUa_Alloc(sizeof(OpcUa_ExtensionObject));


				if ((*ppNewExtensionObject))
				{
					OpcUa_ExtensionObject_Initialize((*ppNewExtensionObject));

					(*ppNewExtensionObject)->BodySize = 0;
					(*ppNewExtensionObject)->Encoding = OpcUa_ExtensionObjectEncoding_EncodeableObject;
					OpcUa_ExpandedNodeId_Initialize(&((*ppNewExtensionObject)->TypeId));
					(*ppNewExtensionObject)->TypeId.ServerIndex = 0;
					OpcUa_String_Initialize(&((*ppNewExtensionObject)->TypeId.NamespaceUri));
					(*ppNewExtensionObject)->TypeId.NodeId.Identifier.Numeric = pEncodeableType->BinaryEncodingTypeId; //
					(*ppNewExtensionObject)->TypeId.NodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
					(*ppNewExtensionObject)->TypeId.NodeId.NamespaceIndex = 0;
					(*ppNewExtensionObject)->Body.EncodeableObject.Type = pEncodeableType;// Utils::Copy(pEncodeableType); // modif mai 2015
					(*ppNewExtensionObject)->Body.EncodeableObject.Object = pVoidResultSave;
				}
				else
				{
					OpcUa_Free(pVoidBufSave);
					pVoidBufSave = OpcUa_Null;
					OpcUa_Free(pVoidResultSave);
					pVoidResultSave = OpcUa_Null;
					uStatus = OpcUa_BadOutOfMemory;
				}
			}
			else
			{
				OpcUa_Free(pVoidResultSave);
				pVoidResultSave = OpcUa_Null;
			}
			// modif mai 2015
			//if (uStatus != OpcUa_Good)
			{
				OpcUa_Free(pVoidBufSave);
				pVoidBufSave = OpcUa_Null;
			}
			// Fin modif mai 2015
		}
		else 
			uStatus = OpcUa_BadInvalidArgument;
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}

OpcUa_StatusCode CDefinition::DuplicateString(void** pVoidBuf, void** pVoidResult)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	// Duplicate a string
	OpcUa_String* pString = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(pString);
#ifdef _DEBUG
	// copy flag
	OpcUa_MemCpy(*pVoidResult, 4, *pVoidBuf, 4);
	((OpcUa_Byte*&)*pVoidResult) += 4; // 
	OpcUa_MemCpy(&(pString->flags), 2, *pVoidBuf, 2);
	((OpcUa_Byte*&)*pVoidBuf) += 4; // The real size is 4 because a OpcUa_String is in fact an OpcUa_StringInternal
	// copy Length
	OpcUa_MemCpy(*pVoidResult, 4, *pVoidBuf, 4);
	((OpcUa_Byte*&)*pVoidResult) += 4; //
	OpcUa_MemCpy(&(pString->uLength), 4, *pVoidBuf, 4);
	if (pString->uLength > 0)
	{
		pString->strContent = (OpcUa_CharA*)OpcUa_Alloc(pString->uLength + 1);
		ZeroMemory(pString->strContent, pString->uLength + 1);
	}
	// Copy the content
	((OpcUa_Byte*&)*pVoidBuf) += 4;
	void* apVoid = OpcUa_Null; // OpcUa_Alloc(4);
	memcpy(&apVoid, *pVoidBuf, 4);
	if (pString->uLength > 0)
	{
		if (apVoid)
			OpcUa_MemCpy((pString->strContent), pString->uLength, ((void*)(apVoid)), pString->uLength);
		OpcUa_MemCpy(*pVoidResult, 4, &(pString->strContent), 4);
	}
	// ici pString->strContent contient la valeur de la chaine. Les autres pointeurs ne servent qu'a transporter
#else
	// copy flag
	OpcUa_MemCpy(*pVoidResult, 4, *pVoidBuf, 4);
	((OpcUa_Byte*&)*pVoidResult) += 4; // 
	OpcUa_MemCpy(&(pString->uReserved1), 2, *pVoidBuf, 2);
	((OpcUa_Byte*&)*pVoidBuf) += 4; // The real size is 4 because a OpcUa_String is in fact an OpcUa_StringInternal
	// copy Length
	OpcUa_MemCpy(*pVoidResult, 4, *pVoidBuf, 4);
	((OpcUa_Byte*&)*pVoidResult) += 4; //
	OpcUa_MemCpy(&(pString->uReserved2), 4, *pVoidBuf, 4);
	if (OpcUa_String_StrLen(pString) > 0)
	{
		pString->uReserved4 = (OpcUa_CharA*)OpcUa_Alloc(pString->uReserved2 + 1);
		ZeroMemory(pString->uReserved4, pString->uReserved2 + 1);
	}
	// Copy the content
	((OpcUa_Byte*&)*pVoidBuf) += 4;
	void* apVoid;// = OpcUa_Alloc(4);
	memcpy(&apVoid, *pVoidBuf, 4);
	if (OpcUa_String_StrLen(pString) > 0)
	{
		OpcUa_MemCpy((pString->uReserved4), pString->uReserved2, ((void*)(apVoid)), pString->uReserved2);
		OpcUa_MemCpy(*pVoidResult, 4, &(pString->uReserved4), 4);
	}
#endif
	// Update the pointer
	((OpcUa_Byte*&)(*pVoidResult)) += 4; //
	((OpcUa_Byte*&)(*pVoidBuf)) += 4;
	//OpcUa_String_Clear(pString);
	OpcUa_Free(pString);
	return uStatus;
}
OpcUa_StatusCode CDefinition::DuplicateNodeId(void** pVoidBuf, void** pVoidResult)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_UInt32 iFieldOnlineSize = 0;
	OpcUa_UInt32 iFielSize = sizeof(OpcUa_NodeId);
	OpcUa_NodeId* pNodeId = (OpcUa_NodeId*)OpcUa_Alloc(sizeof(OpcUa_NodeId));
	OpcUa_NodeId_Initialize(pNodeId);
	// IdentifierType
	OpcUa_MemCpy(&(pNodeId->IdentifierType), 2, *pVoidBuf, 2);
	OpcUa_MemCpy(*pVoidResult, 2, *pVoidBuf, 2);
	((OpcUa_Byte*&)*pVoidResult) += 2;
	((OpcUa_Byte*&)*pVoidBuf) += 2;
	iFieldOnlineSize += 2;
	// NamespaceIndex
	OpcUa_MemCpy(&(pNodeId->NamespaceIndex), 2, *pVoidBuf, 2);
	OpcUa_MemCpy(*pVoidResult, 2, *pVoidBuf, 2);
	((OpcUa_Byte*&)*pVoidResult) += 2;
	((OpcUa_Byte*&)*pVoidBuf) += 2;
	iFieldOnlineSize += 2;
	// Identifier
	switch (pNodeId->IdentifierType)
	{
	case OpcUa_IdentifierType_Numeric:
	{
		OpcUa_MemCpy(&(pNodeId->Identifier.Numeric), 4, *pVoidBuf, 4);
		OpcUa_MemCpy(*pVoidResult, 4, *pVoidBuf, 4);
		((OpcUa_Byte*&)*pVoidResult) += 4;
		((OpcUa_Byte*&)*pVoidBuf) += 4;
		iFieldOnlineSize += 4;
	}
		break;
	case OpcUa_IdentifierType_String:
	{
		DuplicateString(pVoidBuf, pVoidResult);
		iFieldOnlineSize += 8; // String size is 8
	}
		break;
	case OpcUa_IdentifierType_Guid:
	{
		// not implemented we need AddressSpace to test it
		iFieldOnlineSize += sizeof(OpcUa_Guid);
	}
		break;
	case OpcUa_IdentifierType_Opaque:
	{
		// not implemented we need AddressSpace to test it
		iFieldOnlineSize += sizeof(OpcUa_ByteString);
	}
		break;
	default:
		break;
	}
	((OpcUa_Byte*&)(*pVoidBuf)) += (iFielSize - iFieldOnlineSize);
	((OpcUa_Byte*&)(*pVoidResult)) += (iFielSize - iFieldOnlineSize);
	return uStatus;
}

OpcUa_Int32 CDefinition::GetSize()
{
	OpcUa_Int32 iSize=0;
	for (OpcUa_UInt16 ii=0;ii<m_FieldList.size();ii++)
	{
		CField* pField=m_FieldList.at(ii);
		iSize+=pField->GetFieldSize();
	}
	return iSize;
}