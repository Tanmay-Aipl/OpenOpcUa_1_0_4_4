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
#include <openssl/x509.h>
#include "Utils.h"
#include "opcua_encoder.h"
#include "opcua_decoder.h"
#include <typeinfo>
#include "NodeIdLookupTable.h"
using namespace OpenOpcUa;
using namespace UASharedLib;

#define GUID_SIZE 128
//============================================================================
// Utils::StrDup(std::string)
//============================================================================
//char* Utils::StrDup(std::string src)
//{
//	char* pDst = 0;
//
//	if (src.empty())
//	{
//		return pDst;
//	}
//
//	pDst = (char*)OpcUa_Alloc(src.length()+1);
//	memcpy(pDst, src.c_str(), src.length()+1);
//	return pDst;
//}

//============================================================================
// Utils::StrDup(OpcUa_ByteString)
//============================================================================
OpcUa_ByteString Utils::StrDup(const OpcUa_ByteString* pSrc)
{
	OpcUa_ByteString tDst;

	if (pSrc == 0)
	{
		OpcUa_ByteString_Initialize(&tDst);
		return tDst;
	}

	tDst.Length = pSrc->Length;
	tDst.Data = (OpcUa_Byte*)OpcUa_Alloc(tDst.Length);
	OpcUa_MemCpy(tDst.Data, tDst.Length, pSrc->Data, pSrc->Length);
	return tDst;
}

//============================================================================
// Utils::Copy(OpcUa_NodeId)
//============================================================================
OpcUa_NodeId* Utils::Copy(OpcUa_NodeId* pSrc)
{
	OpcUa_NodeId* pTarget=OpcUa_Null;
	if (pSrc)
	{
		pTarget=(OpcUa_NodeId*)OpcUa_Alloc(sizeof(OpcUa_NodeId));
		if (pTarget)
		{
			OpcUa_NodeId_Initialize(pTarget);
			if (!OpcUa_NodeId_IsNull(pSrc))
			{			
				pTarget->IdentifierType=pSrc->IdentifierType;
				pTarget->NamespaceIndex=pSrc->NamespaceIndex;
				// return the numeric value.
				if (pSrc->IdentifierType == OpcUa_IdentifierType_Numeric)
				{
					pTarget->Identifier.Numeric=pSrc->Identifier.Numeric;
				}
				else
				{
					// copy the string value.
					if (pSrc->IdentifierType == OpcUa_IdentifierType_String)
					{
						OpcUa_String* pString=Utils::Copy(&(pSrc->Identifier.String));
						if (pString)
							pTarget->Identifier.String=*pString;
					}
					else
					{
						// copy the guid value.
						if (pSrc->IdentifierType == OpcUa_IdentifierType_Guid)
						{
							pTarget->Identifier.Guid=Utils::Copy(pSrc->Identifier.Guid);
						}
						else
						{
							// copy the opaque value.
							if (pSrc->IdentifierType == OpcUa_IdentifierType_Opaque)
							{
								OpcUa_ByteString_Initialize(&(pTarget->Identifier.ByteString));
								pTarget->Identifier.ByteString.Length = pSrc->Identifier.ByteString.Length;
								pTarget->Identifier.ByteString.Data = (OpcUa_Byte*)OpcUa_Alloc(pSrc->Identifier.ByteString.Length);
								OpcUa_MemCpy(
									pTarget->Identifier.ByteString.Data, 
									pTarget->Identifier.ByteString.Length, 
									pSrc->Identifier.ByteString.Data, 
									pSrc->Identifier.ByteString.Length);
							}
						}
					}
				}
				// other id types not supported yet.
			}
		}
	}
	return pTarget;
}
OpcUa_Boolean Utils::IsEqual(OpcUa_QualifiedName* pOne,OpcUa_QualifiedName * pTwo)
{
	OpcUa_Boolean bResult=OpcUa_False;
	if (pOne)
	{
		if (Utils::IsEqual(&(pOne->Name), &(pTwo->Name)))
		{
			if (pOne->NamespaceIndex == pTwo->NamespaceIndex)
				bResult = OpcUa_True;
		}
	}
	return bResult;
}
OpcUa_Boolean Utils::IsEqual(OpcUa_String* pOne,OpcUa_String * pTwo)
{
	OpcUa_Boolean bResult=OpcUa_False;
	OpcUa_UInt32 uiLen1,uiLen2;
	if (pOne!=OpcUa_Null)
	{
		if (pTwo!=OpcUa_Null)
		{
			uiLen1 = OpcUa_String_StrLen(pOne);
			uiLen2 = OpcUa_String_StrLen(pTwo);
			if (uiLen1 == uiLen2)
			{
				if (uiLen1 > 0)
				{
					if (OpcUa_String_StrnCmp(pOne, pTwo, uiLen1, OpcUa_False) == 0)
						bResult = OpcUa_True;
				}
				else
					bResult = OpcUa_True; // deux chaine qui ne contiennent rien seront considéré egale
			}
		}
	}
	return bResult;
}
OpcUa_Boolean Utils::IsEqual(OpcUa_BuildInfo* pOneEx,OpcUa_BuildInfo * pTwoEx)
{
	OpcUa_Boolean bResult=OpcUa_False;
	if (( pOneEx->BuildDate.dwHighDateTime==pTwoEx->BuildDate.dwHighDateTime)
		&& (pOneEx->BuildDate.dwLowDateTime==pTwoEx->BuildDate.dwLowDateTime) )
	{
		if (Utils::IsEqual(&(pOneEx->BuildNumber),&(pTwoEx->BuildNumber)))
		{
			if (Utils::IsEqual(&(pOneEx->ManufacturerName),&(pTwoEx->ManufacturerName)))
			{
				if (Utils::IsEqual(&(pOneEx->ProductName),&(pTwoEx->ProductName)))
				{
					if (Utils::IsEqual(&(pOneEx->ProductUri),&(pTwoEx->ProductUri)))
					{
						if (Utils::IsEqual(&(pOneEx->ProductUri),&(pTwoEx->ProductUri)))
							bResult=OpcUa_True;
					}
				}
			}
		}
	}
	return bResult;
}
OpcUa_Boolean Utils::IsEqual(OpcUa_ServerStatusDataType* pOneEx,OpcUa_ServerStatusDataType * pTwoEx)
{
	OpcUa_Boolean bResult=OpcUa_False;
	if (Utils::IsEqual(&(pOneEx->BuildInfo),&(pTwoEx->BuildInfo)) )
	{
		if ( (pOneEx->CurrentTime.dwHighDateTime==pTwoEx->CurrentTime.dwHighDateTime) && (pOneEx->CurrentTime.dwLowDateTime==pTwoEx->CurrentTime.dwLowDateTime) )
		{
			if ( (pOneEx->StartTime.dwHighDateTime==pTwoEx->StartTime.dwHighDateTime) && (pOneEx->StartTime.dwLowDateTime==pTwoEx->StartTime.dwLowDateTime) )
			{
				if (pOneEx->SecondsTillShutdown==pTwoEx->SecondsTillShutdown)
				{
					if (pOneEx->State==pTwoEx->State)
					{
						if (Utils::IsEqual(&(pOneEx->ShutdownReason.Text),&(pTwoEx->ShutdownReason.Text)) )
							bResult=OpcUa_True;
					}
				}
			}
		}
	}
	return bResult;
}
OpcUa_Boolean Utils::IsEqual(OpcUa_ByteString* pOne,OpcUa_ByteString * pTwo)
{
	OpcUa_Boolean bResult=OpcUa_False;
	if (pOne)
	{
		if (pTwo)
		{
			if (pOne == pTwo)
				bResult = OpcUa_True;
			else
			{
				if (pOne->Length == pTwo->Length)
				{
					if (OpcUa_MemCmp(pOne->Data, pTwo->Data, pOne->Length) == 0)
						bResult = OpcUa_True;
				}
			}
		}
	}
	return bResult;
}
//============================================================================
// Utils::IsEqual ( pour les OpcUa_Variant
//============================================================================
OpcUa_Boolean Utils::IsEqual(OpcUa_Variant* pOne,OpcUa_Variant * pTwo)
{
	OpcUa_Boolean bResult=OpcUa_False;
	if (pOne && pTwo)
	{
		// cas des deux pointeur égaux
		if (pOne == pTwo)
			bResult= OpcUa_True;
		else
		{
			// comparaison du contenu
			if (pOne->Datatype!=pTwo->Datatype)
				bResult=OpcUa_False;
			else
			{
				if (pOne->ArrayType!=pTwo->ArrayType)
					bResult=OpcUa_False;
				else
				{
					if (pOne->ArrayType==OpcUa_VariantArrayType_Scalar)
					{
						// compare Scalar
						switch (pOne->Datatype)
						{
						case OpcUaType_Boolean:
							bResult=(pOne->Value.Boolean==pTwo->Value.Boolean);
							break;
						case OpcUaType_Byte:
							bResult=pOne->Value.Byte==pTwo->Value.Byte;
							break;
						case OpcUaType_ByteString:
							{
								int iLen=pOne->Value.ByteString.Length;
								if ((pOne->Value.ByteString.Data) && (pTwo->Value.ByteString.Data))
								{
									int iRes = OpcUa_MemCmp(pOne->Value.ByteString.Data, pTwo->Value.ByteString.Data, iLen);
									if (!iRes)
										bResult = OpcUa_True;
								}																	
							}
							break;
						case OpcUaType_DataValue:
							break;
						case OpcUaType_DateTime:
							{
								if (OpcUa_DateTime_Compare(&pOne->Value.DateTime,&pTwo->Value.DateTime)==0)
									bResult = OpcUa_True;
							//if ((pOne->Value.DateTime.dwHighDateTime == pTwo->Value.DateTime.dwHighDateTime) && (pOne->Value.DateTime.dwLowDateTime == pTwo->Value.DateTime.dwLowDateTime))
							//	bResult = OpcUa_True;
							}
							break;
						case OpcUaType_DiagnosticInfo:			
							break;
						case OpcUaType_Double:
							bResult=(pOne->Value.Double==pTwo->Value.Double);
							break;
						case OpcUaType_ExpandedNodeId:
							break;
						case OpcUaType_ExtensionObject:
							if ((pOne->Value.ExtensionObject) && (pTwo->Value.ExtensionObject) )
							{
								// il faut faire un traitement specifique en fonction du type de données contenu dans l'extensionObject
								// le type contenu est codé dans 
								OpcUa_EncodeableType* pEncodeableType=pTwo->Value.ExtensionObject->Body.EncodeableObject.Type;
								if (pEncodeableType)
								{
									if(pTwo->Value.ExtensionObject->Body.EncodeableObject.Object && pOne->Value.ExtensionObject->Body.EncodeableObject.Object)
									{
										// BuildInfo
										if (OpcUa_StrCmpA(pEncodeableType->TypeName,"BuildInfo")==0)
										{
											OpcUa_BuildInfo* pOneEx=(OpcUa_BuildInfo*)pOne->Value.ExtensionObject->Body.EncodeableObject.Object;
											OpcUa_BuildInfo* pTwoEx=(OpcUa_BuildInfo*)pTwo->Value.ExtensionObject->Body.EncodeableObject.Object;
											bResult=Utils::IsEqual(pOneEx,pTwoEx);
										}
										// ServerStatusDataType
										if (OpcUa_StrCmpA(pEncodeableType->TypeName,"ServerStatusDataType")==0)
										{
											OpcUa_ServerStatusDataType* pOneEx=(OpcUa_ServerStatusDataType*)pOne->Value.ExtensionObject->Body.EncodeableObject.Object;
											OpcUa_ServerStatusDataType* pTwoEx=(OpcUa_ServerStatusDataType*)pTwo->Value.ExtensionObject->Body.EncodeableObject.Object;
											bResult=Utils::IsEqual(pOneEx,pTwoEx);
										}
										// TODO autre EncodeableObject connu
									}
								}
							}
							break;
						case OpcUaType_Float:
							bResult=(pOne->Value.Float==pTwo->Value.Float);
							break;
						case OpcUaType_Guid:
							if (OpcUa_Guid_Compare(pOne->Value.Guid, pTwo->Value.Guid) == 0)
								bResult = OpcUa_True;
							break;
						case OpcUaType_Int16:
							bResult=(pOne->Value.Int16==pTwo->Value.Int16);
							break;
						case OpcUaType_Int32:
							bResult=(pOne->Value.Int32==pTwo->Value.Int32);
							break;
						case OpcUaType_Int64:
							bResult=(pOne->Value.Int64==pTwo->Value.Int64);
							break;
						case OpcUaType_LocalizedText:
							{
								if (OpcUa_LocalizedText_Compare(pOne->Value.LocalizedText, pTwo->Value.LocalizedText) == 0)
									bResult = OpcUa_True;
								//bResult=Utils::IsEqual(&(pOne->Value.LocalizedText->Locale),&(pTwo->Value.LocalizedText->Locale));
								//if (bResult)
								//	bResult=Utils::IsEqual(&(pOne->Value.LocalizedText->Text),&(pTwo->Value.LocalizedText->Text));
							}
							break;
						case OpcUaType_NodeId:
							{
								bResult=IsEqual(pOne->Value.NodeId,pTwo->Value.NodeId);
							}
							break;
						case OpcUaType_Null:
							break;
						case OpcUaType_QualifiedName:
							if (OpcUa_QualifiedName_Compare(pOne->Value.QualifiedName,pTwo->Value.QualifiedName)==0)
								bResult = OpcUa_True;
							//bResult=Utils::IsEqual(&(pOne->Value.QualifiedName->Name),&(pTwo->Value.QualifiedName->Name));
							break;
						case OpcUaType_SByte:
							bResult=(pOne->Value.SByte==pTwo->Value.SByte);
							break;
						case OpcUaType_StatusCode:
							bResult=(pOne->Value.StatusCode==pTwo->Value.StatusCode);
							break;
						case OpcUaType_String:
							{
								bResult=Utils::IsEqual(&(pOne->Value.String),&(pTwo->Value.String));
							}
							break;
						case OpcUaType_UInt16:
							bResult=(pOne->Value.UInt16==pTwo->Value.UInt16);
							break;
						case OpcUaType_UInt32:
							bResult=(pOne->Value.UInt32==pTwo->Value.UInt32);
							break;
						case OpcUaType_UInt64:
							bResult=(pOne->Value.UInt64==pTwo->Value.UInt64);
							break;
						case OpcUaType_Variant:
							break;
						case OpcUaType_XmlElement:
							break;
						default:
							break;
						}
					}
					else
					{
						if (pOne->ArrayType==OpcUa_VariantArrayType_Array)
						{
							// compare Array
							pOne->Value.Array.Length=pTwo->Value.Array.Length;
							int iLen=pTwo->Value.Array.Length;
							// On va tester que l'un des indice est different
							// un seul different suffit a faire en sorte que tous le tableau soit different
							for (int ii=0;ii<iLen;ii++)
							{
								switch (pOne->Datatype)
								{
								case OpcUaType_Boolean:
									bResult=(pOne->Value.Array.Value.BooleanArray[ii]==pTwo->Value.Array.Value.BooleanArray[ii]);
									break;
								case OpcUaType_Byte:
									bResult=(pOne->Value.Array.Value.ByteArray[ii]==pTwo->Value.Array.Value.ByteArray[ii]);
									break;
								case OpcUaType_ByteString:
									{
										int iLen2=pTwo->Value.Array.Value.ByteStringArray[ii].Length;
										if (iLen2)
										{
											if (pOne->Value.Array.Value.ByteStringArray[ii].Data)
											{
												if (pTwo->Value.Array.Value.ByteStringArray[ii].Data)
												{
													int iRes=OpcUa_MemCmp(pOne->Value.Array.Value.ByteStringArray[ii].Data,
																		  pTwo->Value.Array.Value.ByteStringArray[ii].Data,
																		  iLen2);
													if (!iRes)
														bResult=OpcUa_True;
												}
											}
										}
									}
									break;
								case OpcUaType_DataValue:
									break;
								case OpcUaType_DateTime:
									if ((pOne->Value.Array.Value.DateTimeArray[ii].dwHighDateTime==pTwo->Value.Array.Value.DateTimeArray[ii].dwHighDateTime) && (pOne->Value.Array.Value.DateTimeArray[ii].dwLowDateTime==pTwo->Value.Array.Value.DateTimeArray[ii].dwLowDateTime) )
										bResult=OpcUa_True;
									break;
								case OpcUaType_DiagnosticInfo:			
									break;
								case OpcUaType_Double:
									bResult=(pOne->Value.Array.Value.DoubleArray[ii]==pTwo->Value.Array.Value.DoubleArray[ii]);
									break;
								case OpcUaType_ExpandedNodeId:
									break;
								case OpcUaType_ExtensionObject:
									break;
								case OpcUaType_Float:
									bResult=(pOne->Value.Array.Value.FloatArray[ii]==pTwo->Value.Array.Value.FloatArray[ii]);
									break;
								case OpcUaType_Guid:
									break;
								case OpcUaType_Int16:
									bResult=(pOne->Value.Array.Value.Int16Array[ii]==pTwo->Value.Array.Value.Int16Array[ii]);
									break;
								case OpcUaType_Int32:
									bResult=(pOne->Value.Array.Value.Int32Array[ii]==pTwo->Value.Array.Value.Int32Array[ii]);
									break;
								case OpcUaType_Int64:
									bResult=(pOne->Value.Array.Value.Int64Array[ii]==pTwo->Value.Array.Value.Int64Array[ii]);
									break;
								case OpcUaType_LocalizedText:
									{
										bResult=Utils::IsEqual(&(pOne->Value.Array.Value.LocalizedTextArray[ii].Text),&(pTwo->Value.Array.Value.LocalizedTextArray[ii].Text));
									}
									break;
								case OpcUaType_NodeId:
									{
										bResult=IsEqual(&(pOne->Value.Array.Value.NodeIdArray[ii]),&(pTwo->Value.Array.Value.NodeIdArray[ii]));
									}
									break;
								case OpcUaType_Null:
									break;
								case OpcUaType_QualifiedName:
									{
										bResult=Utils::IsEqual(&(pOne->Value.Array.Value.QualifiedNameArray[ii].Name),&(pTwo->Value.Array.Value.QualifiedNameArray[ii].Name));
									}
									break;
								case OpcUaType_SByte:
									bResult=(pOne->Value.Array.Value.SByteArray[ii]==pTwo->Value.Array.Value.SByteArray[ii]);
									break;
								case OpcUaType_StatusCode:
									bResult=(pOne->Value.Array.Value.StatusCodeArray[ii]==pTwo->Value.Array.Value.StatusCodeArray[ii]);
									break;
								case OpcUaType_String:
									{
										bResult=Utils::IsEqual(&(pOne->Value.Array.Value.StringArray[ii]),&(pTwo->Value.Array.Value.StringArray[ii]));
									}
									break;
								case OpcUaType_UInt16:
									bResult=(pOne->Value.Array.Value.UInt16Array[ii]==pTwo->Value.Array.Value.UInt16Array[ii]);
									break;
								case OpcUaType_UInt32:
									bResult=(pOne->Value.Array.Value.UInt32Array[ii]==pTwo->Value.Array.Value.UInt32Array[ii]);
									break;
								case OpcUaType_UInt64:
									bResult=(pOne->Value.Array.Value.UInt64Array[ii]==pTwo->Value.Array.Value.UInt64Array[ii]);
									break;
								case OpcUaType_Variant:
									break;
								case OpcUaType_XmlElement:
									break;
								default:
									break;
								}
								if (!bResult)
									break;
							}
						}
						else
						{
							if (pOne->ArrayType==OpcUa_VariantArrayType_Matrix)
							{
								// compare Matrix. Not supported
								;
							}
							else
							{
								// error VariantType unknown
								OpcUa_Trace(OPCUA_TRACE_EXTRA_LEVEL_ERROR,"Utils::IsEqual>error VariantType unknown %u\n",pOne->ArrayType);
							}
						}
					}
				}
			}
		}
	}

	return bResult;
}
//============================================================================
// comparaison de deux OpcUa_ExpandedNodeId
//============================================================================
OpcUa_Boolean Utils::IsEqual(const OpcUa_ExpandedNodeId* pOne, const OpcUa_ExpandedNodeId* pTwo)
{
	OpcUa_Boolean bResult=OpcUa_False;
	if (pOne == pTwo)
		bResult= OpcUa_True;
	else
	{
		bResult=Utils::IsEqual(&(pOne->NodeId),&(pTwo->NodeId));
		if (bResult)
		{
			if (pOne->ServerIndex==pTwo->ServerIndex)
			{
				OpcUa_String aString1=pOne->NamespaceUri;
				OpcUa_String aString2=pTwo->NamespaceUri;
				bResult=Utils::IsEqual(&aString1,&aString2);
				if (bResult)
					bResult=OpcUa_True;
			}
		}
	}
	return bResult;
}
//============================================================================
// comparaison de deux OpcUa_NodeId
//============================================================================
OpcUa_Boolean Utils::IsEqual(const OpcUa_NodeId* pOne, const OpcUa_NodeId* pTwo)
{
	if (pOne == pTwo)
	{
		return true;
	}

	if (pOne->IdentifierType != pTwo->IdentifierType)
	{
		return false;
	}
	
	if (pOne->NamespaceIndex != pTwo->NamespaceIndex)
	{
		return false;
	}

	// check the numeric value.
	if (pOne->IdentifierType == OpcUa_IdentifierType_Numeric)
	{
		return pOne->Identifier.Numeric == pTwo->Identifier.Numeric;
	}

	// check the string value.
	if (pOne->IdentifierType == OpcUa_IdentifierType_String)
	{
		return  OpcUa_String_StrnCmp((OpcUa_String*)&pOne->Identifier.String, (OpcUa_String*)&pTwo->Identifier.String, OPCUA_STRING_LENDONTCARE, OpcUa_False) == 0;
	}

	// check the guid value.
	if (pOne->IdentifierType == OpcUa_IdentifierType_Guid)
	{
		return OpcUa_MemCmp(pOne->Identifier.Guid, pTwo->Identifier.Guid, sizeof(OpcUa_Guid)) == 0;
	}

	// check the opaque value.
	if (pOne->IdentifierType == OpcUa_IdentifierType_Opaque)
	{
		if (pOne->Identifier.ByteString.Length != pTwo->Identifier.ByteString.Length)
		{
			return false;
		}

		return OpcUa_MemCmp(pOne->Identifier.ByteString.Data, pTwo->Identifier.ByteString.Data, pOne->Identifier.ByteString.Length) == 0;
	}
		
	return false;
}
OpcUa_Boolean Utils::IsBufferEmpty(OpcUa_CharA* buffer, OpcUa_UInt32 uiLen)
{
	OpcUa_Boolean bRes=OpcUa_True;
	for (OpcUa_UInt32 ii=0;ii<uiLen;++ii)
	{
		if (buffer[ii]!=0x20)
		{
			bRes=OpcUa_False;
			break;
		}
	}
	return bRes;
}
//============================================================================
// Utils::Copy
//============================================================================
OpcUa_String* Utils::Copy(std::string src)
{
	OpcUa_String* pTarget=OpcUa_Null;
	if (!src.empty())
	{
		pTarget=(OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
		OpcUa_String_Initialize(pTarget);
		OpcUa_CharA* pChar=(OpcUa_CharA*)src.c_str();
		OpcUa_String_AttachCopy(pTarget,pChar);
	}

	return pTarget;
}
OpcUa_BrowseResult* Utils::Copy(OpcUa_BrowseResult* pSrc)
{
	OpcUa_BrowseResult* pTarget=OpcUa_Null;
	if (pSrc)
	{
		pTarget=(OpcUa_BrowseResult*)OpcUa_Alloc(sizeof(OpcUa_BrowseResult));
		OpcUa_BrowseResult_Initialize(pTarget);
		if (pSrc->ContinuationPoint.Length>0)
			pTarget->ContinuationPoint=*Utils::Copy(&(pSrc->ContinuationPoint));
		pTarget->NoOfReferences=pSrc->NoOfReferences;
		pTarget->References=(OpcUa_ReferenceDescription*)OpcUa_Alloc((pSrc->NoOfReferences)*sizeof(OpcUa_ReferenceDescription));
		for (OpcUa_Int32 ii=0;ii<pSrc->NoOfReferences;ii++)
			pTarget->References[ii]=*Utils::Copy(&(pSrc->References[ii]));
		pTarget->StatusCode=pSrc->StatusCode;
	}
	return pTarget;
}
OpcUa_ReferenceDescription* Utils::Copy(OpcUa_ReferenceDescription* pSrc)
{
	OpcUa_ReferenceDescription* pTarget=OpcUa_Null;
	if (pSrc)
	{
		pTarget=(OpcUa_ReferenceDescription*)OpcUa_Alloc(sizeof(OpcUa_ReferenceDescription));
		OpcUa_ReferenceDescription_Initialize(pTarget);
		pTarget->BrowseName=*Utils::Copy(&(pSrc->BrowseName));
		pTarget->DisplayName=*Utils::Copy(&(pSrc->DisplayName));
		pTarget->IsForward=pSrc->IsForward;
		pTarget->NodeClass=pSrc->NodeClass;
		OpcUa_ExpandedNodeId_CopyTo(&(pSrc->NodeId), &(pTarget->NodeId));
		pTarget->ReferenceTypeId=*Utils::Copy(&(pSrc->ReferenceTypeId));
		OpcUa_ExpandedNodeId_CopyTo(&(pSrc->TypeDefinition), &(pTarget->TypeDefinition));
	}
	return pTarget;
}
OpcUa_BuildInfo* Utils::Copy(OpcUa_BuildInfo* pSrcBuildInfo)
{
	OpcUa_BuildInfo* pBuildInfo=OpcUa_Null;
	if (pSrcBuildInfo)
	{
		pBuildInfo=(OpcUa_BuildInfo*)OpcUa_Alloc(sizeof(OpcUa_BuildInfo));
		OpcUa_BuildInfo_Initialize(pBuildInfo);
		OpcUa_String_StrnCpy(
			&(pBuildInfo->ProductUri),
			&pSrcBuildInfo->ProductUri,
			OpcUa_String_StrLen(&(pSrcBuildInfo->ProductUri)) );
		OpcUa_String_StrnCpy(
			&(pBuildInfo->ProductName),
			&pSrcBuildInfo->ProductName,
			OpcUa_String_StrLen(&(pSrcBuildInfo->ProductName)) );
		OpcUa_String_StrnCpy(
			&(pBuildInfo->ManufacturerName),
			&pSrcBuildInfo->ManufacturerName,
			OpcUa_String_StrLen(&(pSrcBuildInfo->ManufacturerName)) );
		OpcUa_String_StrnCpy(
			&(pBuildInfo->BuildNumber),
			&pSrcBuildInfo->BuildNumber,
			OpcUa_String_StrLen(&(pSrcBuildInfo->BuildNumber)) );
		OpcUa_String_StrnCpy(
			&(pBuildInfo->SoftwareVersion),
			&pSrcBuildInfo->SoftwareVersion,
			OpcUa_String_StrLen(&(pSrcBuildInfo->SoftwareVersion)) );
		pBuildInfo->BuildDate.dwHighDateTime=pSrcBuildInfo->BuildDate.dwHighDateTime;
		pBuildInfo->BuildDate.dwLowDateTime=pSrcBuildInfo->BuildDate.dwLowDateTime;
	}
	return pBuildInfo;
}
OpcUa_ServerStatusDataType* Utils::Copy(OpcUa_ServerStatusDataType* pSrcServerStatus)
{
	OpcUa_ServerStatusDataType* pServerStatus=OpcUa_Null;
	if (pSrcServerStatus)
	{
		pServerStatus=(OpcUa_ServerStatusDataType*)OpcUa_Alloc(sizeof(OpcUa_ServerStatusDataType));
		if(pServerStatus)
		{
			OpcUa_ServerStatusDataType_Initialize(pServerStatus);
			// StartTime
			pServerStatus->StartTime=pSrcServerStatus->StartTime;
			// CurrentTime
			pServerStatus->CurrentTime=pSrcServerStatus->CurrentTime;
			// State
			pServerStatus->State= pSrcServerStatus->State;
			// SecondsTillShutdown
			pServerStatus->SecondsTillShutdown=pSrcServerStatus->SecondsTillShutdown;
			// ShutdownReason
			OpcUa_String_StrnCpy(
				&(pServerStatus->ShutdownReason.Locale),
				&pSrcServerStatus->ShutdownReason.Locale,
				OpcUa_String_StrLen(&(pSrcServerStatus->ShutdownReason.Locale)) );
			OpcUa_String_StrnCpy(
				&(pServerStatus->ShutdownReason.Text),
				&pSrcServerStatus->ShutdownReason.Text,
				OpcUa_String_StrLen(&(pSrcServerStatus->ShutdownReason.Text)) );
			// BuildInfo
			OpcUa_BuildInfo* pBuildInfo=Utils::Copy(&(pSrcServerStatus->BuildInfo));
			if (pBuildInfo)
			{
				// BuildDate
				pServerStatus->BuildInfo.BuildDate=pBuildInfo->BuildDate;
				// BuildNumber
				OpcUa_String_StrnCpy(&(pServerStatus->BuildInfo.BuildNumber),
					&(pBuildInfo->BuildNumber),
					OpcUa_String_StrLen(&(pBuildInfo->BuildNumber)));
				// ManufacturerName
				OpcUa_String_StrnCpy(&(pServerStatus->BuildInfo.ManufacturerName),
					&(pBuildInfo->ManufacturerName),
					OpcUa_String_StrLen(&(pBuildInfo->ManufacturerName)));
				// ProductName
				OpcUa_String_StrnCpy(&(pServerStatus->BuildInfo.ProductName),
					&(pBuildInfo->ProductName),
					OpcUa_String_StrLen(&(pBuildInfo->ProductName))); 
				// ProductUri
				OpcUa_String_StrnCpy(&(pServerStatus->BuildInfo.ProductUri),
					&(pBuildInfo->ProductUri),
					OpcUa_String_StrLen(&(pBuildInfo->ProductUri)));
				// SoftwareVersion
				OpcUa_String_StrnCpy(&(pServerStatus->BuildInfo.SoftwareVersion),
					&(pBuildInfo->SoftwareVersion),
					OpcUa_String_StrLen(&(pBuildInfo->SoftwareVersion)));
				OpcUa_BuildInfo_Clear(pBuildInfo);
				OpcUa_Free(pBuildInfo); 
			}
		}
	}
	return pServerStatus;
}
OpcUa_StatusResult* Utils::Copy(OpcUa_StatusResult* pSrc)
{
	OpcUa_StatusResult* pTarget=(OpcUa_StatusResult*)OpcUa_Alloc(sizeof(OpcUa_StatusResult));
	OpcUa_StatusResult_Initialize(pTarget);
	// StatusCode
	pTarget->StatusCode=pSrc->StatusCode;
	// DiagnosticInfo
	OpcUa_DiagnosticInfo* pDiagnosticInfo=NULL;
	pDiagnosticInfo=Utils::Copy(&(pSrc->DiagnosticInfo));
	if (pDiagnosticInfo)
		pTarget->DiagnosticInfo=*pDiagnosticInfo;

	return pTarget;
}
OpcUa_DiagnosticInfo* Utils::Copy(OpcUa_DiagnosticInfo* pSrc)
{
	OpcUa_DiagnosticInfo* pTarget=(OpcUa_DiagnosticInfo*)OpcUa_Alloc(sizeof(OpcUa_DiagnosticInfo));
	OpcUa_DiagnosticInfo_Initialize(pTarget);
	// SymbolicId
	pTarget->SymbolicId=pSrc->SymbolicId;
	// NamespaceUri
	pTarget->NamespaceUri=pSrc->NamespaceUri;
	// Locale
	pTarget->Locale=pSrc->Locale;
	// LocalizedText
	pTarget->LocalizedText=pSrc->LocalizedText;
	// AdditionalInfo
	if (OpcUa_String_StrLen(&(pSrc->AdditionalInfo))>0)
		OpcUa_String_StrnCpy(
			&(pTarget->AdditionalInfo),
			&pSrc->AdditionalInfo,
			OpcUa_String_StrLen(&(pSrc->AdditionalInfo)) );
	// InnerStatusCode
	pTarget->InnerStatusCode=pSrc->InnerStatusCode;
	// InnerDiagnosticInfo
	if (pSrc->InnerDiagnosticInfo)
	{
		//pTarget->InnerDiagnosticInfo=(_OpcUa_DiagnosticInfo*)OpcUa_Alloc(sizeof(*(pSrc->InnerStatusCode)));
		OpcUa_MemCpy(pTarget->InnerDiagnosticInfo,
			sizeof(*(pSrc->InnerDiagnosticInfo)),
			pSrc->InnerDiagnosticInfo,
			sizeof(*(pSrc->InnerDiagnosticInfo)));
	}
	return pTarget;
}
OpcUa_Argument* Utils::Copy(OpcUa_Argument* pSrcArgument)
{
	OpcUa_Argument* pTarget=OpcUa_Null;
	if (pSrcArgument)
	{
		pTarget=(OpcUa_Argument*)OpcUa_Alloc(sizeof(OpcUa_Argument));
		if (pTarget)
		{
			OpcUa_Argument_Initialize(pTarget);
			if (OpcUa_String_StrLen(&pSrcArgument->Name)>0)
				OpcUa_String_CopyTo(&(pSrcArgument->Name), &(pTarget->Name));
			OpcUa_NodeId_CopyTo(&(pSrcArgument->DataType), &(pTarget->DataType));
			pTarget->ValueRank=pSrcArgument->ValueRank;
			pTarget->NoOfArrayDimensions=pSrcArgument->NoOfArrayDimensions;
			if (pTarget->NoOfArrayDimensions > 0)
			{
				pTarget->ArrayDimensions = (OpcUa_UInt32*)OpcUa_Alloc(sizeof(OpcUa_UInt32)*(pSrcArgument->NoOfArrayDimensions));
				for (OpcUa_Int32 i = 0; i < pTarget->NoOfArrayDimensions; i++)
					pTarget->ArrayDimensions[i] = pSrcArgument->ArrayDimensions[i];
			}
			// Description
			OpcUa_LocalizedText_CopyTo(&pSrcArgument->Description, &pTarget->Description);
		}
	}
	return pTarget;
}
OpcUa_SessionSecurityDiagnosticsDataType* Utils::Copy(OpcUa_SessionSecurityDiagnosticsDataType* pSrc)
{
	OpcUa_SessionSecurityDiagnosticsDataType* pTarget=OpcUa_Null;
	if (pSrc)
	{
		pTarget=(OpcUa_SessionSecurityDiagnosticsDataType*)OpcUa_Alloc(sizeof(OpcUa_SessionSecurityDiagnosticsDataType));
		OpcUa_SessionSecurityDiagnosticsDataType_Initialize(pTarget);
		// SessionId
		OpcUa_NodeId_CopyTo(&(pSrc->SessionId),&(pTarget->SessionId));
		// ClientUserIdOfSession
		OpcUa_String_Initialize(&(pTarget->ClientUserIdOfSession));
		OpcUa_String_StrnCpy(&(pTarget->ClientUserIdOfSession),
			&(pSrc->ClientUserIdOfSession),
			OpcUa_String_StrLen(&(pSrc->ClientUserIdOfSession)) );
		// NoOfClientUserIdHistory
		pTarget->NoOfClientUserIdHistory=pSrc->NoOfClientUserIdHistory;
		if (pTarget->NoOfClientUserIdHistory)
			pTarget->ClientUserIdHistory=(OpcUa_String*)OpcUa_Alloc(pSrc->NoOfClientUserIdHistory*sizeof(OpcUa_String));
		for (int ii=0;ii<pSrc->NoOfClientUserIdHistory;++ii)
		{
			OpcUa_String_Initialize(&(pTarget->ClientUserIdHistory[ii]));
			OpcUa_String_CopyTo(&(pSrc->ClientUserIdHistory[ii]), &(pTarget->ClientUserIdHistory[ii]));
		}
		// AuthenticationMechanism
		OpcUa_String_Initialize(&(pTarget->AuthenticationMechanism));
		OpcUa_String_CopyTo(&(pSrc->AuthenticationMechanism),&(pTarget->AuthenticationMechanism));

		// Encoding
		OpcUa_String_Initialize(&(pTarget->Encoding));
		OpcUa_String_StrnCpy(&(pTarget->Encoding),
			&(pSrc->Encoding),
			OpcUa_String_StrLen(&(pSrc->Encoding)) );
		// TransportProtocol
		OpcUa_String_Initialize(&(pTarget->TransportProtocol));
		OpcUa_String_StrnCpy(&(pTarget->TransportProtocol),
			&(pSrc->TransportProtocol),
			OpcUa_String_StrLen(&(pSrc->TransportProtocol)) );
		// SecurityMode
		pTarget->SecurityMode=pSrc->SecurityMode;
		// SecurityPolicyUri
		OpcUa_String_Initialize(&(pTarget->SecurityPolicyUri));
		OpcUa_String_StrnCpy(&(pTarget->SecurityPolicyUri),
			&(pSrc->SecurityPolicyUri),
			OpcUa_String_StrLen(&(pSrc->SecurityPolicyUri)) );
		// ClientCertificate
		OpcUa_ByteString_Initialize(&(pTarget->ClientCertificate));
		OpcUa_Int32 iVal=pSrc->ClientCertificate.Length;
		if (iVal>0)
		{
			OpcUa_ByteString_CopyTo(&(pSrc->ClientCertificate), &(pTarget->ClientCertificate));
			//pTarget->ClientCertificate.Data=(OpcUa_Byte*)OpcUa_Alloc(pSrc->ClientCertificate.Length);
			//pTarget->ClientCertificate.Length=iVal;
			//OpcUa_MemCpy(
			//	pTarget->ClientCertificate.Data,
			//	pSrc->ClientCertificate.Length,
			//	pSrc->ClientCertificate.Data,
			//	pSrc->ClientCertificate.Length);
		}
	}
	return pTarget;
}
OpcUa_SamplingIntervalDiagnosticsDataType* Utils::Copy(OpcUa_SamplingIntervalDiagnosticsDataType* pSrc)
{
	OpcUa_SamplingIntervalDiagnosticsDataType* pTarget=(OpcUa_SamplingIntervalDiagnosticsDataType*)OpcUa_Alloc(sizeof(OpcUa_SamplingIntervalDiagnosticsDataType));
	pTarget->DisabledMonitoredItemCount=pSrc->DisabledMonitoredItemCount;
	pTarget->MaxMonitoredItemCount = pSrc->MaxMonitoredItemCount ;
	pTarget->MonitoredItemCount = pSrc->MonitoredItemCount ;
	pTarget->SamplingInterval = pSrc->SamplingInterval ;
	return pTarget;
}
OpcUa_ServiceCounterDataType* Utils::Copy(OpcUa_ServiceCounterDataType* pSrc)
{
	OpcUa_ServiceCounterDataType* pTarget=OpcUa_Null;
	if (pSrc)
	{
		pTarget=(OpcUa_ServiceCounterDataType*)OpcUa_Alloc(sizeof(OpcUa_ServiceCounterDataType));
		if (pTarget)
		{
			pTarget->ErrorCount=pSrc->ErrorCount;
			pTarget->TotalCount=pSrc->TotalCount;
		}
	}
	return pTarget;
}
OpcUa_SoftwareCertificate* Utils::Copy(OpcUa_SoftwareCertificate* pSrc)
{
	OpcUa_SoftwareCertificate* pTarget=(OpcUa_SoftwareCertificate*)OpcUa_Alloc(sizeof(OpcUa_SoftwareCertificate));
	OpcUa_SoftwareCertificate_Initialize(pTarget);
	// ProductName
	OpcUa_String_Initialize(&(pTarget->ProductName));
	OpcUa_String_StrnCpy(&(pTarget->ProductName),
		&(pSrc->ProductName),
		OpcUa_String_StrLen(&(pSrc->ProductName)) );
	// ProductUri
	OpcUa_String_Initialize(&(pTarget->ProductUri));
	OpcUa_String_StrnCpy(&(pTarget->ProductUri),
		&(pSrc->ProductUri),
		OpcUa_String_StrLen(&(pSrc->ProductUri)) );
	// VendorName
	OpcUa_String_Initialize(&(pTarget->VendorName));
	OpcUa_String_StrnCpy(&(pTarget->VendorName),
		&(pSrc->VendorName),
		OpcUa_String_StrLen(&(pSrc->VendorName)) );
	// VendorProductCertificate
	OpcUa_ByteString_Initialize(&(pTarget->VendorProductCertificate));
	pTarget->VendorProductCertificate.Data=(OpcUa_Byte*)OpcUa_Alloc(pSrc->VendorProductCertificate.Length);
	pTarget->VendorProductCertificate.Length=pSrc->VendorProductCertificate.Length;
	OpcUa_MemCpy(
		pTarget->VendorProductCertificate.Data,
		pSrc->VendorProductCertificate.Length,
		pSrc->VendorProductCertificate.Data,
		pSrc->VendorProductCertificate.Length);
	// SoftwareVersion
	OpcUa_String_Initialize(&(pTarget->SoftwareVersion));
	OpcUa_String_StrnCpy(&(pTarget->SoftwareVersion),
		&(pSrc->SoftwareVersion),
		OpcUa_String_StrLen(&(pSrc->SoftwareVersion)) );
	// BuildNumber
	OpcUa_String_Initialize(&(pTarget->BuildNumber));
	OpcUa_String_StrnCpy(&(pTarget->BuildNumber),
		&(pSrc->BuildNumber),
		OpcUa_String_StrLen(&(pSrc->BuildNumber)) );
	// BuildDate
	pTarget->BuildDate=pSrc->BuildDate;	
	// IssuedBy
	OpcUa_String_Initialize(&(pTarget->IssuedBy));
	OpcUa_String_StrnCpy(&(pTarget->IssuedBy),
		&(pSrc->IssuedBy),
		OpcUa_String_StrLen(&(pSrc->IssuedBy)) );
	// IssueDate
	pTarget->IssueDate=pSrc->BuildDate;	
	// NoOfSupportedProfiles
	pTarget->NoOfSupportedProfiles=pSrc->NoOfSupportedProfiles;
	// SupportedProfiles
	pTarget->SupportedProfiles=(OpcUa_SupportedProfile*)OpcUa_Alloc(sizeof(OpcUa_SupportedProfile)*pTarget->NoOfSupportedProfiles);
	for (int ii=0;ii<pTarget->NoOfSupportedProfiles;ii++)
		pTarget->SupportedProfiles[ii]=*(Utils::Copy(&(pSrc->SupportedProfiles[ii])));
	return pTarget;
}
OpcUa_SignedSoftwareCertificate* Utils::Copy(OpcUa_SignedSoftwareCertificate* pSrc)
{
	OpcUa_SignedSoftwareCertificate* pTarget=(OpcUa_SignedSoftwareCertificate*)OpcUa_Alloc(sizeof(OpcUa_SignedSoftwareCertificate));
	OpcUa_SignedSoftwareCertificate_Initialize(pTarget);
	// CertificateData
	OpcUa_ByteString_Initialize(&(pTarget->CertificateData));
	pTarget->CertificateData.Data=(OpcUa_Byte*)OpcUa_Alloc(pSrc->CertificateData.Length);
	pTarget->CertificateData.Length=pSrc->CertificateData.Length;
	OpcUa_MemCpy(
		pTarget->CertificateData.Data,
		pSrc->CertificateData.Length,
		pSrc->CertificateData.Data,
		pSrc->CertificateData.Length);
	// Signature
	OpcUa_ByteString_Initialize(&(pTarget->Signature));
	pTarget->Signature.Data=(OpcUa_Byte*)OpcUa_Alloc(pSrc->Signature.Length);
	pTarget->Signature.Length=pSrc->Signature.Length;
	OpcUa_MemCpy(
		pTarget->Signature.Data,
		pSrc->Signature.Length,
		pSrc->Signature.Data,
		pSrc->Signature.Length);
	return pTarget;
}
OpcUa_SupportedProfile* Utils::Copy(OpcUa_SupportedProfile* pSrc)
{
	OpcUa_SupportedProfile* pTarget=(OpcUa_SupportedProfile*)OpcUa_Alloc(sizeof(OpcUa_SupportedProfile));
	OpcUa_SupportedProfile_Initialize(pTarget);
	// OrganizationUri
	OpcUa_String_Initialize(&(pTarget->OrganizationUri));
	OpcUa_String_StrnCpy(&(pTarget->OrganizationUri),
		&(pSrc->OrganizationUri),
		OpcUa_String_StrLen(&(pSrc->OrganizationUri)) );
	// ProfileId
	OpcUa_String_Initialize(&(pTarget->ProfileId));
	OpcUa_String_StrnCpy(&(pTarget->ProfileId),
		&(pSrc->ProfileId),
		OpcUa_String_StrLen(&(pSrc->ProfileId)) );
	// ComplianceTool
	OpcUa_String_Initialize(&(pTarget->ComplianceTool));
	OpcUa_String_StrnCpy(&(pTarget->ComplianceTool),
		&(pSrc->ComplianceTool),
		OpcUa_String_StrLen(&(pSrc->ComplianceTool)) );
	// ComplianceDate
	pTarget->ComplianceDate=pSrc->ComplianceDate;
	// ComplianceLevel
	pTarget->ComplianceLevel=pSrc->ComplianceLevel;
	// NoOfUnsupportedUnitIds
	pTarget->NoOfUnsupportedUnitIds=pSrc->NoOfUnsupportedUnitIds;
	// UnsupportedUnitIds
	for (int ii=0;ii<pTarget->NoOfUnsupportedUnitIds;ii++)
	{
		OpcUa_String_Initialize(&(pTarget->UnsupportedUnitIds[ii]));
		OpcUa_String_StrnCpy(&(pTarget->UnsupportedUnitIds[ii]),
			&(pSrc->UnsupportedUnitIds[ii]),
			OpcUa_String_StrLen(&(pSrc->UnsupportedUnitIds[ii])) );
	}
	return pTarget;
}
OpcUa_RedundantServerDataType* Utils::Copy(OpcUa_RedundantServerDataType* pSrc)
{
	OpcUa_RedundantServerDataType* pTarget=(OpcUa_RedundantServerDataType*)OpcUa_Alloc(sizeof(OpcUa_RedundantServerDataType));
	OpcUa_RedundantServerDataType_Initialize(pTarget);
	// ServerId
	OpcUa_String_Initialize(&(pTarget->ServerId));
	OpcUa_String_StrnCpy(&(pTarget->ServerId),
		&(pSrc->ServerId),
		OpcUa_String_StrLen(&(pSrc->ServerId)) );
	// ServiceLevel
	pTarget->ServiceLevel=pSrc->ServiceLevel;
	// ServerState
	pTarget->ServerState=pSrc->ServerState;
	return pTarget;
}
OpcUa_ServerDiagnosticsSummaryDataType* Utils::Copy(OpcUa_ServerDiagnosticsSummaryDataType* pSrc)
{
	OpcUa_ServerDiagnosticsSummaryDataType* pTarget=(OpcUa_ServerDiagnosticsSummaryDataType*)OpcUa_Alloc(sizeof(OpcUa_ServerDiagnosticsSummaryDataType));
	OpcUa_ServerDiagnosticsSummaryDataType_Initialize(pTarget);
	pTarget->CumulatedSessionCount=pSrc->CumulatedSessionCount;
	pTarget->CumulatedSubscriptionCount=pSrc->CumulatedSubscriptionCount;
	pTarget->CurrentSessionCount=pSrc->CurrentSessionCount;
	pTarget->CurrentSubscriptionCount=pSrc->CurrentSubscriptionCount;
	pTarget->PublishingIntervalCount=pSrc->PublishingIntervalCount;
	pTarget->RejectedRequestsCount=pSrc->RejectedRequestsCount;
	pTarget->RejectedSessionCount=pSrc->RejectedSessionCount;
	pTarget->SecurityRejectedRequestsCount=pSrc->SecurityRejectedRequestsCount;
	pTarget->SecurityRejectedSessionCount=pSrc->SecurityRejectedSessionCount;
	pTarget->ServerViewCount=pSrc->ServerViewCount;
	pTarget->SessionAbortCount=pSrc->SessionAbortCount;
	pTarget->SessionTimeoutCount=pSrc->SessionTimeoutCount;
	return pTarget;
}
OpcUa_SubscriptionDiagnosticsDataType* Utils::Copy(OpcUa_SubscriptionDiagnosticsDataType* pSrc)
{
	OpcUa_SubscriptionDiagnosticsDataType* pTarget=OpcUa_Null;
	if (pSrc)
	{
		pTarget=(OpcUa_SubscriptionDiagnosticsDataType*)OpcUa_Alloc(sizeof(OpcUa_SubscriptionDiagnosticsDataType));
		OpcUa_SubscriptionDiagnosticsDataType_Initialize(pTarget);
		// SessionId
		OpcUa_NodeId_CopyTo(&(pSrc->SessionId),&(pTarget->SessionId));
		// SubscriptionId
		pTarget->SubscriptionId=pSrc->SubscriptionId;
		pTarget->Priority=pSrc->Priority;
		pTarget->PublishingInterval=pSrc->PublishingInterval;
		pTarget->MaxKeepAliveCount=pSrc->MaxKeepAliveCount;
		pTarget->MaxLifetimeCount=pSrc->MaxLifetimeCount;
		pTarget->MaxNotificationsPerPublish=pSrc->MaxNotificationsPerPublish;
		pTarget->PublishingEnabled=pSrc->PublishingEnabled;
		pTarget->ModifyCount=pSrc->ModifyCount;
		pTarget->EnableCount=pSrc->EnableCount;
		pTarget->DisableCount=pSrc->DisableCount;
		pTarget->RepublishRequestCount=pSrc->RepublishRequestCount;
		pTarget->RepublishMessageRequestCount=pSrc->RepublishMessageRequestCount;
		pTarget->RepublishMessageCount=pSrc->RepublishMessageCount;
		pTarget->TransferRequestCount=pSrc->TransferRequestCount;
		pTarget->TransferredToAltClientCount=pSrc->TransferredToAltClientCount;
		pTarget->TransferredToSameClientCount=pSrc->TransferredToSameClientCount;
		pTarget->PublishRequestCount=pSrc->PublishRequestCount;
		pTarget->DataChangeNotificationsCount=pSrc->DataChangeNotificationsCount;
		pTarget->EventNotificationsCount=pSrc->EventNotificationsCount;
		pTarget->NotificationsCount=pSrc->NotificationsCount;
		pTarget->LatePublishRequestCount=pSrc->LatePublishRequestCount;
		pTarget->CurrentKeepAliveCount=pSrc->CurrentLifetimeCount;
		pTarget->CurrentLifetimeCount=pSrc->CurrentLifetimeCount;
		pTarget->UnacknowledgedMessageCount=pSrc->UnacknowledgedMessageCount;
		pTarget->DiscardedMessageCount=pSrc->DiscardedMessageCount;
		pTarget->MonitoredItemCount=pSrc->MonitoredItemCount;
		pTarget->DisabledMonitoredItemCount=pSrc->DisabledMonitoredItemCount;
		pTarget->MonitoringQueueOverflowCount=pSrc->MonitoringQueueOverflowCount;
		pTarget->NextSequenceNumber=pSrc->NextSequenceNumber;
		pTarget->EventQueueOverFlowCount=pSrc->EventQueueOverFlowCount;       
	}
	return pTarget;
}
OpcUa_LocalizedText* Utils::Copy(OpcUa_LocalizedText* pSrc)
{
	OpcUa_LocalizedText* pTarget=OpcUa_Null;
	if (pSrc)
	{
		pTarget=(OpcUa_LocalizedText*)OpcUa_Alloc(sizeof(OpcUa_LocalizedText));
		OpcUa_LocalizedText_Initialize(pTarget);
		//Locale
		//pTarget->Locale=*Utils::Copy(&(pSrc->Locale));
		//if (pString)
		//{
		//	//pTarget->Locale.strContent=(OpcUa_CharA*)OpcUa_Alloc(OpcUa_String_StrLen(pString));
			//OpcUa_String_AttachCopy(&(pTarget->Locale),OpcUa_String_GetRawString(&(pSrc->Locale)));
			OpcUa_UInt32 iSize=OpcUa_String_StrLen(&(pSrc->Text));
			if (iSize)
				OpcUa_String_AttachCopy(&(pTarget->Locale),OpcUa_String_GetRawString(&(pSrc->Locale)));

		//}
		//Text
		//pTarget->Text=*Utils::Copy(&(pSrc->Text));
		//if (pString)
		//{
		//	//pTarget->Text.strContent=(OpcUa_CharA*)OpcUa_Alloc(OpcUa_String_StrLen(pString));
			iSize=OpcUa_String_StrLen(&(pSrc->Text));
			if (iSize)
				OpcUa_String_AttachCopy(&(pTarget->Text),OpcUa_String_GetRawString(&(pSrc->Text)));
		//}
	}
	return pTarget;
}
OpcUa_ModelChangeStructureDataType* Utils::Copy(OpcUa_ModelChangeStructureDataType* pSrc)
{
	OpcUa_ModelChangeStructureDataType* pTarget=(OpcUa_ModelChangeStructureDataType*)OpcUa_Alloc(sizeof(OpcUa_ModelChangeStructureDataType));
	OpcUa_ModelChangeStructureDataType_Initialize(pTarget);
	// Affected
	OpcUa_NodeId* pNodeId=Utils::Copy(&(pSrc->Affected));
	if (pNodeId)
		pTarget->Affected=*pNodeId;
	// AffectedType
	pNodeId=Utils::Copy(&(pSrc->AffectedType));
	if (pNodeId)
		pTarget->AffectedType=*pNodeId;
	// Verb
	pTarget->Verb=pSrc->Verb;
	return pTarget;
}
OpcUa_SemanticChangeStructureDataType* Utils::Copy(OpcUa_SemanticChangeStructureDataType* pSrc)
{
	OpcUa_SemanticChangeStructureDataType* pTarget=(OpcUa_SemanticChangeStructureDataType*)OpcUa_Alloc(sizeof(OpcUa_SemanticChangeStructureDataType));
	OpcUa_SemanticChangeStructureDataType_Initialize(pTarget);
	// Affected
	OpcUa_NodeId* pNodeId=Utils::Copy(&(pSrc->Affected));
	if (pNodeId)
		pTarget->Affected=*pNodeId;
	// AffectedType
	pNodeId=Utils::Copy(&(pSrc->AffectedType));
	if (pNodeId)
		pTarget->AffectedType=*pNodeId;
	return pTarget;
}
OpcUa_LiteralOperand* Utils::Copy(OpcUa_LiteralOperand* pSrc)
{
	OpcUa_LiteralOperand* pTarget=OpcUa_Null;
	if (pSrc)
	{
		pTarget=(OpcUa_LiteralOperand*)OpcUa_Alloc(sizeof(OpcUa_LiteralOperand));
		OpcUa_LiteralOperand_Initialize(pTarget);
		OpcUa_Variant_CopyTo(&(pSrc->Value),&(pTarget->Value));
		//pTarget->Value=*Utils::Copy(&(pSrc->Value));
	}
	return pTarget;
}
OpcUa_Range* Utils::Copy(OpcUa_Range* pSrc)
{
	OpcUa_Range* pTarget=(OpcUa_Range*)OpcUa_Alloc(sizeof(OpcUa_Range));
	OpcUa_Range_Initialize(pTarget);
	pTarget->High=pSrc->High;
	pTarget->Low=pSrc->Low;
	return pTarget;
}
OpcUa_EncodeableType* Utils::Copy(OpcUa_EncodeableType* pSrc)
{
	if (!pSrc)
		return OpcUa_Null;
	OpcUa_EncodeableType* pTarget=(OpcUa_EncodeableType*)OpcUa_Alloc(sizeof(OpcUa_EncodeableType));
	if (pTarget)
	{
		pTarget->AllocationSize = pSrc->AllocationSize;
		pTarget->BinaryEncodingTypeId = pSrc->BinaryEncodingTypeId;
		pTarget->Clear = pSrc->Clear;
		pTarget->Decode = pSrc->Decode;
		pTarget->Encode = pSrc->Encode;
		pTarget->GetSize = pSrc->GetSize;
		pTarget->Initialize = pSrc->Initialize;
		pTarget->NamespaceUri = OpcUa_Null;
		if (pSrc->NamespaceUri)
		{
			OpcUa_UInt32 strSize = strlen(pSrc->NamespaceUri);
			if (strSize)
			{
				pTarget->NamespaceUri = (OpcUa_StringA)OpcUa_Alloc(strSize + 1);
				ZeroMemory(pTarget->NamespaceUri, strSize + 1);
				OpcUa_StrnCpyA(pTarget->NamespaceUri, strSize, pSrc->NamespaceUri, strSize); // StringA
			}
		}

		pTarget->TypeId = pSrc->TypeId;
		pTarget->TypeName = OpcUa_Null;
		if (pSrc->TypeName)
		{
			OpcUa_UInt32 strSize = strlen(pSrc->TypeName);
			if (strSize)
			{
				pTarget->TypeName = (OpcUa_StringA)OpcUa_Alloc(strSize + 1);
				ZeroMemory(pTarget->TypeName, strSize + 1);
				OpcUa_StrnCpyA(pTarget->TypeName, strSize, pSrc->TypeName, strSize); // StringA
			}
		}
		pTarget->XmlEncodingTypeId = pSrc->XmlEncodingTypeId;
	}
	return pTarget;
}
OpcUa_EndpointDescription* Utils::Copy(OpcUa_EndpointDescription* pSrc)
{
	if (!pSrc)
		return OpcUa_Null;
	OpcUa_EndpointDescription* pTarget=(OpcUa_EndpointDescription*)OpcUa_Alloc(sizeof(OpcUa_EndpointDescription));
	OpcUa_EndpointDescription_Initialize(pTarget);
	OpcUa_String_CopyTo(&(pSrc->EndpointUrl),&(pTarget->EndpointUrl));	
	pTarget->NoOfUserIdentityTokens=pSrc->NoOfUserIdentityTokens;
	if (pSrc->NoOfUserIdentityTokens)
	{
		pTarget->UserIdentityTokens=(OpcUa_UserTokenPolicy*)OpcUa_Alloc(sizeof(OpcUa_UserTokenPolicy)*(pSrc->NoOfUserIdentityTokens));
		for (OpcUa_Int32 ii = 0; ii < pSrc->NoOfUserIdentityTokens; ++ii)
		{
			OpcUa_UserTokenPolicy_Initialize(&(pTarget->UserIdentityTokens[ii]));
			OpcUa_String_CopyTo(&(pSrc->UserIdentityTokens[ii].IssuedTokenType), &(pTarget->UserIdentityTokens[ii].IssuedTokenType));
			OpcUa_String_CopyTo(&(pSrc->UserIdentityTokens[ii].IssuerEndpointUrl), &(pTarget->UserIdentityTokens[ii].IssuerEndpointUrl));
			OpcUa_String_CopyTo(&(pSrc->UserIdentityTokens[ii].PolicyId), &(pTarget->UserIdentityTokens[ii].PolicyId));
			OpcUa_String_CopyTo(&(pSrc->UserIdentityTokens[ii].SecurityPolicyUri), &(pTarget->UserIdentityTokens[ii].SecurityPolicyUri));
			pTarget->UserIdentityTokens[ii].TokenType=pSrc->UserIdentityTokens[ii].TokenType;
		}
	}
	pTarget->SecurityLevel=pSrc->SecurityLevel;
	pTarget->SecurityMode = pSrc->SecurityMode ;
	// SecurityPolicyUri
	OpcUa_String_Initialize(&(pTarget->SecurityPolicyUri));
	OpcUa_String_CopyTo(&(pSrc->SecurityPolicyUri),&(pTarget->SecurityPolicyUri));

	// ApplicationDescription	
	OpcUa_LocalizedText_CopyTo(&(pSrc->Server.ApplicationName), &(pTarget->Server.ApplicationName));
	pTarget->Server.ApplicationType=pSrc->Server.ApplicationType;
	OpcUa_String_CopyTo(&(pSrc->Server.ApplicationUri), &pTarget->Server.ApplicationUri);
	OpcUa_String_CopyTo(&(pSrc->Server.DiscoveryProfileUri), &(pTarget->Server.DiscoveryProfileUri));
	pTarget->Server.NoOfDiscoveryUrls = pSrc->Server.NoOfDiscoveryUrls;
	/*if (pSrc->Server.NoOfDiscoveryUrls > 0)
	{
		for (OpcUa_Int32 ii = 0; ii < pSrc->Server.NoOfDiscoveryUrls; ++ii)
			OpcUa_String_CopyTo(&(pSrc->Server.DiscoveryUrls[ii]), &(pTarget->Server.DiscoveryUrls[ii]));
	}*/
	OpcUa_String_CopyTo(&(pSrc->Server.GatewayServerUri), &(pTarget->Server.GatewayServerUri));
	OpcUa_String_CopyTo(&(pSrc->Server.ProductUri),&(pTarget->Server.ProductUri));

	// ServerCertificate
	OpcUa_ByteString_Initialize(&(pTarget->ServerCertificate));
	if (pSrc->ServerCertificate.Data)
		OpcUa_ByteString_CopyTo(&(pSrc->ServerCertificate),&(pTarget->ServerCertificate));
	// TransportProfileUri
	OpcUa_String_Initialize(&(pTarget->TransportProfileUri));
	OpcUa_String_CopyTo(&(pSrc->TransportProfileUri),&(pTarget->TransportProfileUri));

	
	return pTarget;
}
OpcUa_UserTokenPolicy* Utils::Copy(OpcUa_UserTokenPolicy* pSrc)
{
	OpcUa_UserTokenPolicy* pTarget=(OpcUa_UserTokenPolicy*)OpcUa_Alloc(sizeof(OpcUa_UserTokenPolicy));
	OpcUa_UserTokenPolicy_Initialize(pTarget);
	if (OpcUa_String_StrLen(&(pSrc->IssuedTokenType))>0)
		OpcUa_String_CopyTo(&(pSrc->IssuedTokenType),&(pTarget->IssuedTokenType));
		
	if (OpcUa_String_StrLen(&(pSrc->IssuerEndpointUrl))>0)
		OpcUa_String_CopyTo(&(pSrc->IssuerEndpointUrl),&(pTarget->IssuerEndpointUrl));

	if (OpcUa_String_StrLen(&(pSrc->PolicyId))>0)
		OpcUa_String_CopyTo(&(pSrc->PolicyId),&(pTarget->PolicyId));
		
	if (OpcUa_String_StrLen(&(pSrc->SecurityPolicyUri))>0)
		OpcUa_String_CopyTo(&(pSrc->SecurityPolicyUri),&(pTarget->SecurityPolicyUri));
	pTarget->TokenType=pSrc->TokenType;
	return pTarget;
}
OpcUa_EnumValueType* Utils::Copy(OpcUa_EnumValueType* pSrc)
{
	OpcUa_EnumValueType* pTarget=(OpcUa_EnumValueType*)OpcUa_Alloc(sizeof(OpcUa_EnumValueType));
	if (pTarget)
	{
		OpcUa_EnumValueType_Initialize(pTarget);
		// Value
		pTarget->Value=pSrc->Value;
		// Description
		OpcUa_LocalizedText_CopyTo(&(pSrc->Description), &(pTarget->Description));
		// DisplayName
		OpcUa_LocalizedText_CopyTo(&(pSrc->DisplayName),&(pTarget->DisplayName));
	}
	return pTarget;
}
OpcUa_EUInformation* Utils::Copy(OpcUa_EUInformation* pSrc)
{
	OpcUa_EUInformation* pTarget=(OpcUa_EUInformation*)OpcUa_Alloc(sizeof(OpcUa_EUInformation));
	OpcUa_EUInformation_Initialize(pTarget);
	// NamespaceUri
	OpcUa_String_Initialize(&(pTarget->NamespaceUri));
	OpcUa_String_StrnCpy(&(pTarget->NamespaceUri),
		&(pSrc->NamespaceUri),
		OpcUa_String_StrLen(&(pSrc->NamespaceUri)) );
	// UnitId
	pTarget->UnitId=pSrc->UnitId;
	// DisplayName
	OpcUa_LocalizedText_Initialize(&(pTarget->DisplayName));
	OpcUa_String_StrnCpy(&(pTarget->DisplayName.Locale),
		&(pSrc->DisplayName.Locale),
		OpcUa_String_StrLen(&(pSrc->DisplayName.Locale)) );
	OpcUa_String_StrnCpy(&(pTarget->DisplayName.Text),
		&(pSrc->DisplayName.Text),
		OpcUa_String_StrLen(&(pSrc->DisplayName.Text)) );
	// Description
	OpcUa_LocalizedText_Initialize(&(pTarget->Description));
	OpcUa_String_StrnCpy(&(pTarget->Description.Locale),
		&(pSrc->Description.Locale),
		OpcUa_String_StrLen(&(pSrc->Description.Locale)) );
	OpcUa_String_StrnCpy(&(pTarget->Description.Text),
		&(pSrc->Description.Text),
		OpcUa_String_StrLen(&(pSrc->Description.Text)) );
	return pTarget;
}
OpcUa_Annotation* Utils::Copy(OpcUa_Annotation* pSrc)
{
	OpcUa_Annotation* pTarget=(OpcUa_Annotation*)OpcUa_Alloc(sizeof(OpcUa_Annotation));
	OpcUa_Annotation_Initialize(pTarget);
	pTarget->AnnotationTime=pSrc->AnnotationTime;
	// Message
	OpcUa_String_Initialize(&(pTarget->Message));
	OpcUa_String_StrnCpy(&(pTarget->Message),
		&(pSrc->Message),
		OpcUa_String_StrLen(&(pSrc->Message)) );
	// UserName
	OpcUa_String_Initialize(&(pTarget->UserName));
	OpcUa_String_StrnCpy(&(pTarget->UserName),
		&(pSrc->UserName),
		OpcUa_String_StrLen(&(pSrc->UserName)) );
	return pTarget;
}
OpcUa_ProgramDiagnosticDataType* Utils::Copy(OpcUa_ProgramDiagnosticDataType* pSrc)
{
	OpcUa_ProgramDiagnosticDataType* pTarget=(OpcUa_ProgramDiagnosticDataType*)OpcUa_Alloc(sizeof(OpcUa_ProgramDiagnosticDataType));
	OpcUa_ProgramDiagnosticDataType_Initialize(pTarget);
	// CreateSessionId
	OpcUa_NodeId* pNodeId=Utils::Copy(&(pSrc->CreateSessionId));
	if (pNodeId)
		pTarget->CreateSessionId=*pNodeId;
	// CreateClientName
	OpcUa_String_Initialize(&(pTarget->CreateClientName));
	OpcUa_String_StrnCpy(&(pTarget->CreateClientName),
		&(pSrc->CreateClientName),
		OpcUa_String_StrLen(&(pSrc->CreateClientName)) );
	// InvocationCreationTime
	pTarget->InvocationCreationTime=pSrc->InvocationCreationTime;
	// LastTransitionTime
	pTarget->LastTransitionTime=pSrc->LastTransitionTime;
	// LastMethodCall
	OpcUa_String_Initialize(&(pTarget->LastMethodCall));
	OpcUa_String_StrnCpy(&(pTarget->LastMethodCall),
		&(pSrc->LastMethodCall),
		OpcUa_String_StrLen(&(pSrc->LastMethodCall)) );
	// LastMethodSessionId
	pNodeId=Utils::Copy(&(pSrc->LastMethodSessionId));
	if (pNodeId)
		pTarget->LastMethodSessionId=*pNodeId;
	// NoOfLastMethodInputArguments
	pTarget->NoOfLastMethodInputArguments=pSrc->NoOfLastMethodInputArguments;
	// LastMethodInputArguments
	for (int ii=0;ii<pTarget->NoOfLastMethodInputArguments;ii++)
		pTarget->LastMethodInputArguments[ii]=*(Utils::Copy(&(pSrc->LastMethodInputArguments[ii])));
	// NoOfLastMethodOutputArguments
	pTarget->NoOfLastMethodOutputArguments=pSrc->NoOfLastMethodOutputArguments;
	// LastMethodInputArguments
	for (int ii=0;ii<pTarget->NoOfLastMethodOutputArguments;ii++)
		pTarget->LastMethodOutputArguments[ii]=*(Utils::Copy(&(pSrc->LastMethodOutputArguments[ii])));
	//LastMethodCallTime
	pTarget->LastMethodCallTime=pSrc->LastMethodCallTime;
	// LastMethodReturnStatus
	OpcUa_StatusResult* pStatusResult=Utils::Copy(&(pSrc->LastMethodReturnStatus));
	if (pStatusResult)
		pTarget->LastMethodReturnStatus=*pStatusResult;
	return pTarget;
}
OpcUa_SessionDiagnosticsDataType* Utils::Copy(OpcUa_SessionDiagnosticsDataType* pSrc)
{
	OpcUa_SessionDiagnosticsDataType* pTarget = OpcUa_Null;
	if (pSrc)
	{
		pTarget = (OpcUa_SessionDiagnosticsDataType*)OpcUa_Alloc(sizeof(OpcUa_SessionDiagnosticsDataType));
		if (pTarget)
		{
			OpcUa_SessionDiagnosticsDataType_Initialize(pTarget);
			// SessionId
			OpcUa_NodeId_Initialize(&(pTarget->SessionId));
			OpcUa_NodeId_CopyTo(&(pSrc->SessionId), &(pTarget->SessionId));
			// SessionName
			OpcUa_String_Initialize(&(pTarget->SessionName));
			OpcUa_String_CopyTo(&(pSrc->SessionName), &(pTarget->SessionName));
			// ClientDescription
			OpcUa_LocalizedText_CopyTo(&(pSrc->ClientDescription.ApplicationName), &(pTarget->ClientDescription.ApplicationName));
			pSrc->ClientDescription.ApplicationType = pTarget->ClientDescription.ApplicationType;
			OpcUa_String_CopyTo( &(pSrc->ClientDescription.ApplicationUri), &(pTarget->ClientDescription.ApplicationUri));
			OpcUa_String_CopyTo( &(pSrc->ClientDescription.DiscoveryProfileUri), &(pTarget->ClientDescription.DiscoveryProfileUri));
			pTarget->ClientDescription.NoOfDiscoveryUrls = pSrc->ClientDescription.NoOfDiscoveryUrls;
			if (pTarget->ClientDescription.NoOfDiscoveryUrls> 0)
			{
				pTarget->ClientDescription.DiscoveryUrls = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String)*pTarget->ClientDescription.NoOfDiscoveryUrls);
				for (OpcUa_UInt32 i = 0; i < pTarget->ClientDescription.NoOfDiscoveryUrls; i++)
				{
					OpcUa_String_Initialize(&(pTarget->ClientDescription.DiscoveryUrls[i]));
					OpcUa_String_CopyTo(&(pSrc->ClientDescription.DiscoveryUrls[i]), &(pTarget->ClientDescription.DiscoveryUrls[i]));
				}
			}
			OpcUa_String_CopyTo(&(pSrc->ClientDescription.GatewayServerUri), &(pTarget->ClientDescription.GatewayServerUri));
			OpcUa_String_CopyTo(&(pSrc->ClientDescription.ProductUri), &(pTarget->ClientDescription.ProductUri));

			// ServerUri
			OpcUa_String_Initialize(&(pTarget->ServerUri));
			OpcUa_String_CopyTo(&(pSrc->ServerUri), &(pTarget->ServerUri));
			// EndpointUrl
			OpcUa_String_Initialize(&(pTarget->EndpointUrl));
			OpcUa_String_CopyTo(&(pSrc->EndpointUrl), &(pTarget->EndpointUrl));

			// NoOfLocaleIds
			pTarget->NoOfLocaleIds = pSrc->NoOfLocaleIds;
			if (pTarget->NoOfLocaleIds>0)
			{
				pTarget->LocaleIds = (OpcUa_String*)OpcUa_Alloc(pSrc->NoOfLocaleIds*sizeof(OpcUa_String));
				for (OpcUa_Int32 ii = 0; ii < pSrc->NoOfLocaleIds; ii++)
				{
					OpcUa_String_Initialize(&(pTarget->LocaleIds[ii]));
					OpcUa_String_CopyTo(&(pSrc->LocaleIds[ii]), &(pTarget->LocaleIds[ii]));
				}
			}
			// ActualSessionTimeout
			pTarget->ActualSessionTimeout = pSrc->ActualSessionTimeout;
			// MaxResponseMessageSize
			pTarget->MaxResponseMessageSize = pSrc->MaxResponseMessageSize;
			// ClientConnectionTime
			pTarget->ClientConnectionTime = pSrc->ClientConnectionTime;
			// ClientLastContactTime
			pTarget->ClientLastContactTime = pSrc->ClientLastContactTime;
			// CurrentSubscriptionsCount
			pTarget->CurrentSubscriptionsCount = pSrc->CurrentSubscriptionsCount;
			// CurrentMonitoredItemsCount
			pTarget->CurrentMonitoredItemsCount = pSrc->CurrentMonitoredItemsCount;
			// CurrentPublishRequestsInQueue
			pTarget->CurrentPublishRequestsInQueue = pSrc->CurrentPublishRequestsInQueue;
			// TotalRequestCount
			pTarget->TotalRequestCount.ErrorCount = pSrc->TotalRequestCount.ErrorCount;
			pTarget->TotalRequestCount.TotalCount = pSrc->TotalRequestCount.TotalCount;
			// CurrentMonitoredItemsCount
			pTarget->CurrentMonitoredItemsCount = pSrc->CurrentMonitoredItemsCount;
			pTarget->CurrentMonitoredItemsCount = pSrc->CurrentMonitoredItemsCount;
			// ReadCount
			pTarget->ReadCount.ErrorCount = pSrc->ReadCount.ErrorCount;
			pTarget->ReadCount.TotalCount = pSrc->ReadCount.TotalCount;
			// HistoryReadCount
			pTarget->HistoryReadCount.ErrorCount = pSrc->HistoryReadCount.ErrorCount;
			pTarget->HistoryReadCount.TotalCount = pSrc->HistoryReadCount.TotalCount;
			// WriteCount
			pTarget->WriteCount.ErrorCount = pSrc->WriteCount.ErrorCount;
			pTarget->WriteCount.TotalCount = pSrc->WriteCount.TotalCount;
			// HistoryUpdateCount
			pTarget->HistoryUpdateCount.ErrorCount = pSrc->HistoryUpdateCount.ErrorCount;
			pTarget->HistoryUpdateCount.TotalCount = pSrc->HistoryUpdateCount.TotalCount;
			// CallCount
			pTarget->CallCount.ErrorCount = pSrc->CallCount.ErrorCount;
			pTarget->CallCount.TotalCount = pSrc->CallCount.TotalCount;
			// CreateMonitoredItemsCount
			pTarget->CreateMonitoredItemsCount.ErrorCount = pSrc->CreateMonitoredItemsCount.ErrorCount;
			pTarget->CreateMonitoredItemsCount.TotalCount = pSrc->CreateMonitoredItemsCount.TotalCount;
			// ModifyMonitoredItemsCount
			pTarget->ModifyMonitoredItemsCount.ErrorCount = pSrc->ModifyMonitoredItemsCount.ErrorCount;
			pTarget->ModifyMonitoredItemsCount.TotalCount = pSrc->ModifyMonitoredItemsCount.TotalCount;
			// SetMonitoringModeCount
			pTarget->SetMonitoringModeCount.ErrorCount = pSrc->SetMonitoringModeCount.ErrorCount;
			pTarget->SetMonitoringModeCount.TotalCount = pSrc->SetMonitoringModeCount.TotalCount;
			// SetTriggeringCount
			pTarget->SetTriggeringCount.ErrorCount = pSrc->SetTriggeringCount.ErrorCount;
			pTarget->SetTriggeringCount.TotalCount = pSrc->SetTriggeringCount.TotalCount;
			// DeleteMonitoredItemsCount
			pTarget->DeleteMonitoredItemsCount.ErrorCount = pSrc->DeleteMonitoredItemsCount.ErrorCount;
			pTarget->DeleteMonitoredItemsCount.TotalCount = pSrc->DeleteMonitoredItemsCount.TotalCount;
			// CreateSubscriptionCount
			pTarget->CreateSubscriptionCount.ErrorCount = pSrc->CreateSubscriptionCount.ErrorCount;
			pTarget->CreateSubscriptionCount.TotalCount = pSrc->CreateSubscriptionCount.TotalCount;
			// ModifySubscriptionCount
			pTarget->ModifySubscriptionCount.ErrorCount = pSrc->ModifySubscriptionCount.ErrorCount;
			pTarget->ModifySubscriptionCount.TotalCount = pSrc->ModifySubscriptionCount.TotalCount;
			// SetPublishingModeCount
			pTarget->SetPublishingModeCount.ErrorCount = pSrc->SetPublishingModeCount.ErrorCount;
			pTarget->SetPublishingModeCount.TotalCount = pSrc->SetPublishingModeCount.TotalCount;
			// PublishCount
			pTarget->PublishCount.ErrorCount = pSrc->PublishCount.ErrorCount;
			pTarget->PublishCount.TotalCount = pSrc->PublishCount.TotalCount;
			// RepublishCount
			pTarget->RepublishCount.ErrorCount = pSrc->RepublishCount.ErrorCount;
			pTarget->RepublishCount.TotalCount = pSrc->RepublishCount.TotalCount;
			// TransferSubscriptionsCount
			pTarget->TransferSubscriptionsCount.ErrorCount = pSrc->TransferSubscriptionsCount.ErrorCount;
			pTarget->TransferSubscriptionsCount.TotalCount = pSrc->TransferSubscriptionsCount.TotalCount;
			// DeleteSubscriptionsCount
			pTarget->DeleteSubscriptionsCount.ErrorCount = pSrc->DeleteSubscriptionsCount.ErrorCount;
			pTarget->DeleteSubscriptionsCount.TotalCount = pSrc->DeleteSubscriptionsCount.TotalCount;
			// AddNodesCount
			pTarget->AddNodesCount.ErrorCount = pSrc->AddNodesCount.ErrorCount;
			pTarget->AddNodesCount.TotalCount = pSrc->AddNodesCount.TotalCount;
			// AddReferencesCount
			pTarget->TotalRequestCount.ErrorCount = pSrc->AddReferencesCount.ErrorCount;
			pTarget->TotalRequestCount = pSrc->AddReferencesCount;
			// DeleteNodesCount
			pTarget->DeleteNodesCount.ErrorCount = pSrc->DeleteNodesCount.ErrorCount;
			pTarget->DeleteNodesCount.TotalCount = pSrc->DeleteNodesCount.TotalCount;
			// DeleteReferencesCount
			pTarget->DeleteReferencesCount.ErrorCount = pSrc->DeleteReferencesCount.ErrorCount;
			pTarget->DeleteReferencesCount.TotalCount = pSrc->DeleteReferencesCount.TotalCount;
			// BrowseCount
			pTarget->BrowseCount.ErrorCount = pSrc->BrowseCount.ErrorCount;
			pTarget->BrowseCount.TotalCount = pSrc->BrowseCount.TotalCount;
			// BrowseNextCount
			pTarget->BrowseNextCount.ErrorCount = pSrc->BrowseNextCount.ErrorCount;
			pTarget->BrowseNextCount.TotalCount = pSrc->BrowseNextCount.TotalCount;
			// TranslateBrowsePathsToNodeIdsCount
			pTarget->TranslateBrowsePathsToNodeIdsCount.ErrorCount = pSrc->TranslateBrowsePathsToNodeIdsCount.ErrorCount;
			pTarget->TranslateBrowsePathsToNodeIdsCount.TotalCount = pSrc->TranslateBrowsePathsToNodeIdsCount.TotalCount;
			// QueryFirstCount
			pTarget->QueryFirstCount.ErrorCount = pSrc->QueryFirstCount.ErrorCount;
			pTarget->QueryFirstCount.TotalCount = pSrc->QueryFirstCount.TotalCount;
			// QueryNextCount
			pTarget->QueryNextCount.ErrorCount = pSrc->QueryNextCount.ErrorCount;
			pTarget->QueryNextCount.TotalCount = pSrc->QueryNextCount.TotalCount;
			// RegisterNodesCount
			pTarget->RegisterNodesCount.ErrorCount = pSrc->RegisterNodesCount.ErrorCount;
			pTarget->RegisterNodesCount.TotalCount = pSrc->RegisterNodesCount.TotalCount;
			// UnregisterNodesCount
			pTarget->UnregisterNodesCount.ErrorCount = pSrc->UnregisterNodesCount.ErrorCount;
			pTarget->UnregisterNodesCount.TotalCount = pSrc->UnregisterNodesCount.TotalCount;

		}
	}
	return pTarget;
}
OpcUa_ApplicationDescription* Utils::Copy(OpcUa_ApplicationDescription* pSrc)
{
	OpcUa_ApplicationDescription* pTarget=OpcUa_Null;
	if (pSrc)
	{
		pTarget=(OpcUa_ApplicationDescription*)OpcUa_Alloc(sizeof(OpcUa_ApplicationDescription));
		if (pTarget)
		{
			OpcUa_ApplicationDescription_Initialize(pTarget);
			// ApplicationName
			OpcUa_LocalizedText_Initialize(&(pTarget->ApplicationName));
			OpcUa_LocalizedText_CopyTo(&(pSrc->ApplicationName), &(pTarget->ApplicationName));

			// ProductUri
			OpcUa_String_Initialize(&(pTarget->ProductUri));
			if (OpcUa_String_StrLen(&(pSrc->ProductUri)))
			{
				OpcUa_String_StrnCpy(&(pTarget->ProductUri),
					&(pSrc->ProductUri),
					OpcUa_String_StrLen(&(pSrc->ProductUri)));
			}
			// ApplicationUri
			OpcUa_String_Initialize(&(pTarget->ApplicationUri));
			if (OpcUa_String_StrLen(&(pSrc->ApplicationUri)))
				OpcUa_String_StrnCpy(&(pTarget->ApplicationUri),
				&(pSrc->ApplicationUri),
				OpcUa_String_StrLen(&(pSrc->ApplicationUri)));
			// ApplicationType
			pTarget->ApplicationType = pSrc->ApplicationType;
			// GatewayServerUri
			OpcUa_String_Initialize(&(pTarget->GatewayServerUri));
			if (OpcUa_String_StrLen(&(pSrc->GatewayServerUri)))
				OpcUa_String_StrnCpy(&(pTarget->GatewayServerUri),
				&(pSrc->GatewayServerUri),
				OpcUa_String_StrLen(&(pSrc->GatewayServerUri)));
			// DiscoveryProfileUri
			OpcUa_String_Initialize(&(pTarget->DiscoveryProfileUri));
			if (OpcUa_String_StrLen(&(pSrc->DiscoveryProfileUri)))
				OpcUa_String_StrnCpy(&(pTarget->DiscoveryProfileUri),
				&(pSrc->DiscoveryProfileUri),
				OpcUa_String_StrLen(&(pSrc->DiscoveryProfileUri)));
			// NoOfDiscoveryUrls
			pTarget->NoOfDiscoveryUrls = pSrc->NoOfDiscoveryUrls;
			// DiscoveryUrls
			if (pSrc->NoOfDiscoveryUrls)
			{
				pTarget->DiscoveryUrls = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String)*pSrc->NoOfDiscoveryUrls);
				for (int ii = 0; ii < pSrc->NoOfDiscoveryUrls; ++ii)
				{
					OpcUa_String_Initialize(&(pTarget->DiscoveryUrls[ii]));
					if (OpcUa_String_StrLen(&(pSrc->DiscoveryUrls[ii]))>0)
					{
						OpcUa_String_StrnCpy(&(pTarget->DiscoveryUrls[ii]),
							&(pSrc->DiscoveryUrls[ii]),
							OpcUa_String_StrLen(&(pSrc->DiscoveryUrls[ii])));
					}
				}
			}
			else
				pTarget->DiscoveryUrls = OpcUa_Null;
		}
	}
	return pTarget;
}
OpcUa_TimeZoneDataType* Utils::Copy(OpcUa_TimeZoneDataType* pSrc)
{
	OpcUa_TimeZoneDataType* pTarget=(OpcUa_TimeZoneDataType*)OpcUa_Alloc(sizeof(OpcUa_TimeZoneDataType));
	OpcUa_TimeZoneDataType_Initialize(pTarget);
	// Offset
	pTarget->Offset=pSrc->Offset;
	// DaylightSavingInOffset
	pTarget->DaylightSavingInOffset=pSrc->DaylightSavingInOffset;
	return pTarget;
}
OpcUa_UserIdentityToken* Utils::Copy(OpcUa_UserIdentityToken* pSrc)
{
	OpcUa_UserIdentityToken* pTarget=(OpcUa_UserIdentityToken*)OpcUa_Alloc(sizeof(OpcUa_UserIdentityToken));
	OpcUa_UserIdentityToken_Initialize(pTarget);
	// PolicyId
	OpcUa_String_Initialize(&(pTarget->PolicyId));
	if (OpcUa_String_StrLen(&(pSrc->PolicyId)))
		OpcUa_String_StrnCpy(&(pTarget->PolicyId),
			&(pSrc->PolicyId),
			OpcUa_String_StrLen(&(pSrc->PolicyId)) );
	return pTarget;
}
OpcUa_Guid* Utils::Copy(OpcUa_Guid* pSrc)
{
	OpcUa_Guid* pTarget=(OpcUa_Guid*)OpcUa_Alloc(sizeof(OpcUa_Guid));
	OpcUa_Guid_Initialize(pTarget);
	if (pSrc)
	{
		pTarget->Data1=pSrc->Data1;
		pTarget->Data2=pSrc->Data2;
		pTarget->Data3=pSrc->Data3;
		OpcUa_MemCpy(pTarget->Data4,8,pSrc->Data4,8);
	}
	return pTarget;
}
OpcUa_HistoryEventFieldList* Utils::Copy(OpcUa_HistoryEventFieldList* pSrc)
{
	OpcUa_HistoryEventFieldList* pTarget=(OpcUa_HistoryEventFieldList*)OpcUa_Alloc(sizeof(OpcUa_HistoryEventFieldList));
	OpcUa_HistoryEventFieldList_Initialize(pTarget);
	pTarget->NoOfEventFields=pSrc->NoOfEventFields;
	for (int ii=0;ii<pTarget->NoOfEventFields;ii++)
	{
		OpcUa_Variant_Initialize(&(pTarget->EventFields[ii]));
		OpcUa_Variant_CopyTo(&(pSrc->EventFields[ii]),&(pTarget->EventFields[ii]));
		//OpcUa_Variant* pVariant=Utils::Copy(pSrc->EventFields);
		//if (pVariant)
		//	pTarget->EventFields[ii]=*pVariant;
	}
	return pTarget;
}
OpcUa_DataChangeFilter* Utils::Copy(OpcUa_DataChangeFilter* pSrc)
{
	OpcUa_DataChangeFilter* pTarget=OpcUa_Null;
	if (pSrc)
	{
		pTarget=(OpcUa_DataChangeFilter*)OpcUa_Alloc(sizeof(OpcUa_DataChangeFilter));
		OpcUa_DataChangeFilter_Initialize(pTarget);
		pTarget->DeadbandType=pSrc->DeadbandType;
		pTarget->DeadbandValue=pSrc->DeadbandValue;
		pTarget->Trigger=pSrc->Trigger;
	}
	return pTarget;
}
OpcUa_DataValue* Utils::Copy(OpcUa_DataValue* pSrc)
{
	OpcUa_DataValue* pTarget=OpcUa_Null;
	if (pSrc)
	{
		pTarget=(OpcUa_DataValue*)OpcUa_Alloc(sizeof(OpcUa_DataValue));
		if (pTarget)
		{
			// OpcUa_DataValue_CopyTo(pSrc,pTarget);
			OpcUa_DataValue_Initialize(pTarget);
			pTarget->ServerPicoseconds=pSrc->ServerPicoseconds;
			OpcUa_DateTime_CopyTo(&(pSrc->ServerTimestamp), &(pTarget->ServerTimestamp));
			//pTarget->ServerTimestamp=*Utils::Copy(&(pSrc->ServerTimestamp));
			pTarget->SourcePicoseconds=pSrc->SourcePicoseconds;
			//pTarget->SourceTimestamp=*Utils::Copy(&(pSrc->SourceTimestamp));
			OpcUa_DateTime_CopyTo(&(pSrc->SourceTimestamp), &(pTarget->SourceTimestamp));
			pTarget->StatusCode=pSrc->StatusCode;
			OpcUa_Variant_CopyTo(&(pSrc->Value),&(pTarget->Value));
			//pTarget->Value=*Utils::Copy(&(pSrc->Value));
		}
	}
	return pTarget;
}
//OpcUa_Variant* Utils::Copy(OpcUa_Variant* pSrc)
//{
//	OpcUa_Variant* pTarget=(OpcUa_Variant*)OpcUa_Alloc(sizeof(OpcUa_Variant));
//	OpcUa_Variant_Initialize(pTarget);
//	// ArrayType
//	pTarget->ArrayType=pSrc->ArrayType;
//	// Datatype
//	pTarget->Datatype=pSrc->Datatype;
//	// Reserved
//	pTarget->Reserved=pSrc->Reserved;
//	// Value
//	if (pTarget->ArrayType==OpcUa_VariantArrayType_Scalar)
//	{
//		// Copie Scalar
//		switch (pTarget->Datatype)
//		{
//		case OpcUaType_Boolean:
//			pTarget->Value.Boolean=pSrc->Value.Boolean;
//			break;
//		case OpcUaType_Byte:
//			pTarget->Value.Byte=pSrc->Value.Byte;
//			break;
//		case OpcUaType_ByteString:
//			{
//				int iLen=pSrc->Value.ByteString.Length;
//				pTarget->Value.ByteString.Data=(OpcUa_Byte*)OpcUa_Alloc(iLen);
//				OpcUa_MemCpy(pTarget->Value.ByteString.Data,iLen,pSrc->Value.ByteString.Data,iLen);
//			}
//			break;
//		case OpcUaType_DataValue:
//			break;
//		case OpcUaType_DateTime:
//			pTarget->Value.DateTime=pSrc->Value.DateTime;
//			break;
//		case OpcUaType_DiagnosticInfo:			
//			break;
//		case OpcUaType_Double:
//			pTarget->Value.Double=pSrc->Value.Double;
//			break;
//		case OpcUaType_ExpandedNodeId:
//			break;
//		case OpcUaType_ExtensionObject:
//			pTarget->Value.ExtensionObject=Utils::Copy(pSrc->Value.ExtensionObject);
//			break;
//		case OpcUaType_Float:
//			pTarget->Value.Float=pSrc->Value.Float;
//			break;
//		case OpcUaType_Guid:
//			break;
//		case OpcUaType_Int16:
//			pTarget->Value.Int16=pSrc->Value.Int16;
//			break;
//		case OpcUaType_Int32:
//			pTarget->Value.Int32=pSrc->Value.Int32;
//			break;
//		case OpcUaType_Int64:
//			pTarget->Value.Int64=pSrc->Value.Int64;
//			break;
//		case OpcUaType_LocalizedText:
//			{
//				pTarget->Value.LocalizedText=Utils::Copy(pSrc->Value.LocalizedText);
//			}
//			break;
//		case OpcUaType_NodeId:
//			{
//				pTarget->Value.NodeId=Utils::Copy(pSrc->Value.NodeId);
//			}
//			break;
//		case OpcUaType_Null:
//			break;
//		case OpcUaType_QualifiedName:
//			pTarget->Value.QualifiedName=Utils::Copy(pSrc->Value.QualifiedName);
//			break;
//		case OpcUaType_SByte:
//			pTarget->Value.SByte=pSrc->Value.SByte;
//			break;
//		case OpcUaType_StatusCode:
//			pTarget->Value.StatusCode=pSrc->Value.StatusCode;
//			break;
//		case OpcUaType_String:
//			{
//				OpcUa_String* pString=Utils::Copy(&(pSrc->Value.String));
//				if (pString)
//					pTarget->Value.String=*pString;
//			}
//			break;
//		case OpcUaType_UInt16:
//			pTarget->Value.UInt16=pSrc->Value.UInt16;
//			break;
//		case OpcUaType_UInt32:
//			pTarget->Value.UInt32=pSrc->Value.UInt32;
//			break;
//		case OpcUaType_UInt64:
//			pTarget->Value.UInt64=pSrc->Value.UInt64;
//			break;
//		case OpcUaType_Variant:
//			break;
//		case OpcUaType_XmlElement:
//			break;
//		default:
//			break;
//		}
//	}
//	else
//	{
//		if (pTarget->ArrayType==OpcUa_VariantArrayType_Array)
//		{
//			// Copie Array
//			pTarget->Value.Array.Length=pSrc->Value.Array.Length;
//			int iLen=pSrc->Value.Array.Length;
//			for (int ii=0;ii<iLen;ii++)
//			{
//				//pTarget->Value.Array.Value.[ii]=pSrc->Value.Array.Value.[ii];
//				switch (pTarget->Datatype)
//				{
//				case OpcUaType_Boolean:
//					{						
//						if (pTarget->Value.Array.Value.BooleanArray==OpcUa_Null)
//							pTarget->Value.Array.Value.BooleanArray=(OpcUa_Boolean*)OpcUa_Alloc(sizeof(OpcUa_Boolean)*(pSrc->Value.Array.Length));
//						pTarget->Value.Array.Value.BooleanArray[ii]=pSrc->Value.Array.Value.BooleanArray[ii];
//					}
//					break;
//				case OpcUaType_Byte:
//					{						
//						if (pTarget->Value.Array.Value.ByteArray==OpcUa_Null)
//							pTarget->Value.Array.Value.ByteArray=(OpcUa_Byte*)OpcUa_Alloc(sizeof(OpcUa_Byte)*(pSrc->Value.Array.Length));
//						pTarget->Value.Array.Value.ByteArray[ii]=pSrc->Value.Array.Value.ByteArray[ii];
//					}
//					break;
//				case OpcUaType_ByteString:
//					{
//						if (pTarget->Value.Array.Value.ByteStringArray==OpcUa_Null)
//							pTarget->Value.Array.Value.ByteStringArray=(OpcUa_ByteString*)OpcUa_Alloc(sizeof(OpcUa_ByteString)*(pSrc->Value.Array.Length));
//						OpcUa_ByteString* pByteString=Utils::Copy(&(pSrc->Value.Array.Value.ByteStringArray[ii]));
//						if (pByteString)
//							pTarget->Value.Array.Value.ByteStringArray[ii]=*pByteString;
//					}
//					break;
//				case OpcUaType_DataValue:
//					break;
//				case OpcUaType_DateTime:
//					{						
//						if (pTarget->Value.Array.Value.DateTimeArray==OpcUa_Null)
//							pTarget->Value.Array.Value.DateTimeArray=(OpcUa_DateTime*)OpcUa_Alloc(sizeof(OpcUa_DateTime)*(pSrc->Value.Array.Length));
//						pTarget->Value.Array.Value.DateTimeArray[ii]=pSrc->Value.Array.Value.DateTimeArray[ii];
//					}
//					break;
//				case OpcUaType_DiagnosticInfo:			
//					break;
//				case OpcUaType_Double:
//					{						
//						if (pTarget->Value.Array.Value.DoubleArray==OpcUa_Null)
//							pTarget->Value.Array.Value.DoubleArray=(OpcUa_Double*)OpcUa_Alloc(sizeof(OpcUa_Double)*(pSrc->Value.Array.Length));
//						pTarget->Value.Array.Value.DoubleArray[ii]=pSrc->Value.Array.Value.DoubleArray[ii];
//					}
//					break;
//				case OpcUaType_ExpandedNodeId:
//					break;
//				case OpcUaType_ExtensionObject:
//					break;
//				case OpcUaType_Float:
//					{						
//						if (pTarget->Value.Array.Value.FloatArray==OpcUa_Null)
//							pTarget->Value.Array.Value.FloatArray=(OpcUa_Float*)OpcUa_Alloc(sizeof(OpcUa_Float)*(pSrc->Value.Array.Length));
//						pTarget->Value.Array.Value.FloatArray[ii]=pSrc->Value.Array.Value.FloatArray[ii];
//					}
//					break;
//				case OpcUaType_Guid:
//					{
//						if (pTarget->Value.Array.Value.GuidArray==OpcUa_Null)
//							pTarget->Value.Array.Value.GuidArray=(OpcUa_Guid*)OpcUa_Alloc(sizeof(OpcUa_Guid)*(pSrc->Value.Array.Length));
//						OpcUa_Guid* pGuid=Utils::Copy(&(pSrc->Value.Array.Value.GuidArray[ii]));
//						if (pGuid)
//							pTarget->Value.Array.Value.GuidArray[ii]=*pGuid;
//					}
//					break;
//				case OpcUaType_Int16:
//					{						
//						if (pTarget->Value.Array.Value.Int16Array==OpcUa_Null)
//							pTarget->Value.Array.Value.Int16Array=(OpcUa_Int16*)OpcUa_Alloc(sizeof(OpcUa_Int16)*(pSrc->Value.Array.Length));
//						pTarget->Value.Array.Value.Int16Array[ii]=pSrc->Value.Array.Value.Int16Array[ii];
//					}
//					break;
//				case OpcUaType_Int32:
//					{						
//						if (pTarget->Value.Array.Value.Int32Array==OpcUa_Null)
//							pTarget->Value.Array.Value.Int32Array=(OpcUa_Int32*)OpcUa_Alloc(sizeof(OpcUa_Int32)*(pSrc->Value.Array.Length));
//						pTarget->Value.Array.Value.Int32Array[ii]=pSrc->Value.Array.Value.Int32Array[ii];
//					}
//					break;
//				case OpcUaType_Int64:
//					{						
//						if (pTarget->Value.Array.Value.Int64Array==OpcUa_Null)
//							pTarget->Value.Array.Value.Int64Array=(OpcUa_Int64*)OpcUa_Alloc(sizeof(OpcUa_Int64)*(pSrc->Value.Array.Length));
//						pTarget->Value.Array.Value.Int64Array[ii]=pSrc->Value.Array.Value.Int64Array[ii];
//					}
//					break;
//				case OpcUaType_LocalizedText:
//					{
//						OpcUa_LocalizedText* pLocalText=Utils::Copy(&(pSrc->Value.Array.Value.LocalizedTextArray[ii]));
//						if (pLocalText)
//							pTarget->Value.Array.Value.LocalizedTextArray[ii]=*pLocalText;
//					}
//					break;
//				case OpcUaType_NodeId:
//					{
//						OpcUa_NodeId* pNodeId=Utils::Copy(&(pSrc->Value.Array.Value.NodeIdArray[ii]));
//						if (pNodeId)
//							pTarget->Value.Array.Value.NodeIdArray[ii]=*pNodeId;
//					}
//					break;
//				case OpcUaType_Null:
//					break;
//				case OpcUaType_QualifiedName:
//					{
//					OpcUa_QualifiedName* pQualifiedName=Utils::Copy(&(pSrc->Value.Array.Value.QualifiedNameArray[ii]));
//					if (pQualifiedName)
//						pTarget->Value.Array.Value.QualifiedNameArray[ii]=*pQualifiedName;
//					}
//					break;
//				case OpcUaType_SByte:
//					pTarget->Value.Array.Value.SByteArray[ii]=pSrc->Value.Array.Value.SByteArray[ii];
//					break;
//				case OpcUaType_StatusCode:
//					pTarget->Value.Array.Value.StatusCodeArray[ii]=pSrc->Value.Array.Value.StatusCodeArray[ii];
//					break;
//				case OpcUaType_String:
//					{
//						if (pTarget->Value.Array.Value.StringArray==OpcUa_Null)
//							pTarget->Value.Array.Value.StringArray=(OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String)*(pSrc->Value.Array.Length));
//						OpcUa_String* pString=Utils::Copy(&(pSrc->Value.Array.Value.StringArray[ii]));
//						if (pString)
//							pTarget->Value.Array.Value.StringArray[ii]=*pString;
//					}
//					break;
//				case OpcUaType_UInt16:
//					pTarget->Value.Array.Value.UInt16Array[ii]=pSrc->Value.Array.Value.UInt16Array[ii];
//					break;
//				case OpcUaType_UInt32:
//					pTarget->Value.Array.Value.UInt32Array[ii]=pSrc->Value.Array.Value.UInt32Array[ii];
//					break;
//				case OpcUaType_UInt64:
//					pTarget->Value.Array.Value.UInt64Array[ii]=pSrc->Value.Array.Value.UInt64Array[ii];
//					break;
//				case OpcUaType_Variant:
//					break;
//				case OpcUaType_XmlElement:
//					break;
//				default:
//					break;
//				}
//			}
//		}
//		else
//		{
//			if (pTarget->ArrayType==OpcUa_VariantArrayType_Matrix)
//			{
//				// Copie Matrix. Not supported
//				;
//			}
//			else
//			{
//				// error VariantType unknown
//				OpcUa_Trace(OPCUA_TRACE_EXTRA_LEVEL_ERROR,"Utils::Copy OpcUa_Variant>error VariantType unknown %u\n",pTarget->ArrayType);
//			}
//		}
//	}
//
//
//	return pTarget;
//}

OpcUa_AddReferencesItem* Utils::Copy(OpcUa_AddReferencesItem* pSrc)
{
	OpcUa_AddReferencesItem* pTarget=(OpcUa_AddReferencesItem*)OpcUa_Alloc(sizeof(OpcUa_AddReferencesItem));
	OpcUa_AddReferencesItem_Initialize(pTarget);
	// SourceNodeId
	OpcUa_NodeId* pNodeId=Utils::Copy(&(pSrc->SourceNodeId));
	if (pNodeId)
		pTarget->SourceNodeId=*pNodeId;
	// ReferenceTypeId
	pNodeId=Utils::Copy(&(pSrc->ReferenceTypeId));
	if (pNodeId)
	pTarget->ReferenceTypeId=*pNodeId;
	// IsForward
	pTarget->IsForward=pSrc->IsForward;
	// PolicyId
	OpcUa_String_Initialize(&(pTarget->TargetServerUri));
	if (OpcUa_String_StrLen(&(pSrc->TargetServerUri)))
		OpcUa_String_StrnCpy(&(pTarget->TargetServerUri),
			&(pSrc->TargetServerUri),
			OpcUa_String_StrLen(&(pSrc->TargetServerUri)) );
	// TargetNodeId
	OpcUa_ExpandedNodeId_CopyTo(&(pSrc->TargetNodeId), &(pTarget->TargetNodeId));	
	// TargetNodeClass
	pTarget->TargetNodeClass=pSrc->TargetNodeClass;
	return pTarget;
}
OpcUa_DeleteReferencesItem* Utils::Copy(OpcUa_DeleteReferencesItem* pSrc)
{
	OpcUa_DeleteReferencesItem* pTarget=(OpcUa_DeleteReferencesItem*)OpcUa_Alloc(sizeof(OpcUa_DeleteReferencesItem));
	OpcUa_DeleteReferencesItem_Initialize(pTarget);
	// SourceNodeId
	OpcUa_NodeId* pNodeId=Utils::Copy(&(pSrc->SourceNodeId));
	if (pNodeId)
		pTarget->SourceNodeId=*pNodeId;
	// ReferenceTypeId
	pNodeId=Utils::Copy(&(pSrc->ReferenceTypeId));
	if (pNodeId)
		pTarget->ReferenceTypeId=*pNodeId;
	// IsForward
	pTarget->IsForward=pSrc->IsForward;
	// TargetNodeId
	OpcUa_ExpandedNodeId_CopyTo(&(pSrc->TargetNodeId), &(pTarget->TargetNodeId));
	// DeleteBidirectional
	pTarget->DeleteBidirectional=pSrc->DeleteBidirectional;
	return pTarget;
}
OpcUa_DateTime* Utils::Copy(OpcUa_DateTime* pSrc)
{
	OpcUa_DateTime* pTarget=(OpcUa_DateTime*)OpcUa_Alloc(sizeof(OpcUa_DateTime));
	if (pTarget)
	{
		OpcUa_DateTime_Initialize(pTarget);
		// dwHighDateTime
		pTarget->dwHighDateTime=pSrc->dwHighDateTime;
		// dwLowDateTime
		pTarget->dwLowDateTime=pSrc->dwLowDateTime;
	}
	return pTarget;
}
OpcUa_DeleteNodesItem* Utils::Copy(OpcUa_DeleteNodesItem* pSrc)
{
	OpcUa_DeleteNodesItem* pTarget=(OpcUa_DeleteNodesItem*)OpcUa_Alloc(sizeof(OpcUa_DeleteNodesItem));
	OpcUa_DeleteNodesItem_Initialize(pTarget);
	// NodeId
	OpcUa_NodeId* pNodeId=Utils::Copy(&(pSrc->NodeId));
	if (pNodeId)
		pTarget->NodeId=*pNodeId;
	// DeleteTargetReferences
	pTarget->DeleteTargetReferences=pSrc->DeleteTargetReferences;
	return pTarget;
}
//OpcUa_AddNodesItem* Utils::Copy(OpcUa_AddNodesItem* pSrc)
//{
//	OpcUa_AddNodesItem* pTarget=(OpcUa_AddNodesItem*)OpcUa_Alloc(sizeof(OpcUa_AddNodesItem));
//	OpcUa_AddNodesItem_Initialize(pTarget);
//	// ParentNodeId
//	pTarget->ParentNodeId=Utils::Copy(&(pSrc->ParentNodeId));
//	// ReferenceTypeId
//	OpcUa_NodeId* pNodeId=Utils::Copy(&(pSrc->ReferenceTypeId));
//	if (pNodeId)
//		pTarget->ReferenceTypeId=*pNodeId;
//	// RequestedNewNodeId
//	pTarget->RequestedNewNodeId=Utils::Copy(&(pSrc->RequestedNewNodeId));
//	// BrowseName
//
//	// NodeClass
//	pTarget->NodeClass=pSrc->NodeClass;
//	// NodeAttributes
//	pTarget->NodeAttributes=*Utils::Copy(&(pSrc->NodeAttributes));
//	// TypeDefinition
//	pTarget->TypeDefinition=Utils::Copy(&(pSrc->TypeDefinition));
//	return pTarget;
//}
OpcUa_EventFilterResult* Utils::Copy(OpcUa_EventFilterResult* pSrc)
{
	OpcUa_EventFilterResult* pTarget=(OpcUa_EventFilterResult*)OpcUa_Alloc(sizeof(OpcUa_EventFilterResult));
	OpcUa_EventFilterResult_Initialize(pTarget);
	// NoOfSelectClauseResults
	pTarget->NoOfSelectClauseResults=pSrc->NoOfSelectClauseResults;
	//SelectClauseResults
	for (int ii=0;ii<pTarget->NoOfSelectClauseResults;ii++)
		pTarget->SelectClauseResults[ii]=pSrc->SelectClauseResults[ii];
	// NoOfSelectClauseDiagnosticInfos
	pTarget->NoOfSelectClauseDiagnosticInfos=pSrc->NoOfSelectClauseDiagnosticInfos;
	// SelectClauseDiagnosticInfos
	for (int ii=0;ii<pTarget->NoOfSelectClauseDiagnosticInfos;ii++)
	{
		OpcUa_DiagnosticInfo* pDiag=Utils::Copy(&(pTarget->SelectClauseDiagnosticInfos[ii]));
		if (pDiag)
			pTarget->SelectClauseDiagnosticInfos[ii]=*pDiag;
	}
	// WhereClauseResult
	OpcUa_ContentFilterResult* pContentFilterResult=Utils::Copy(&(pSrc->WhereClauseResult));
	if (pContentFilterResult)
		pTarget->WhereClauseResult=*pContentFilterResult;
	return pTarget;
}

//OpcUa_EventFilter* Utils::Copy(OpcUa_EventFilter* pSrc)
//{
//	OpcUa_EventFilter* pTarget=(OpcUa_EventFilter*)OpcUa_Alloc(sizeof(OpcUa_EventFilter));
//	OpcUa_EventFilter_Initialize(pTarget);
//	// NoOfSelectClauses
//	pTarget->NoOfSelectClauses=pSrc->NoOfSelectClauses;
//	// SelectClauses
//	pTarget->SelectClauses=(OpcUa_SimpleAttributeOperand*)OpcUa_Alloc(sizeof(OpcUa_SimpleAttributeOperand)*(pTarget->NoOfSelectClauses));
//	for (int ii=0;ii<pTarget->NoOfSelectClauses;ii++)
//	{
//		pTarget->SelectClauses[ii]=*Utils::Copy(&(pSrc->SelectClauses[ii]));
//	}
//	// WhereClause
//	OpcUa_ContentFilter* pContentFilter=Utils::Copy(&(pSrc->WhereClause));
//	if (pContentFilter)
//	{
//		pTarget->WhereClause=*pContentFilter;
//		OpcUa_Free(pContentFilter);
//	}
//	return pTarget;
//}
OpcUa_SimpleAttributeOperand* Utils::Copy(OpcUa_SimpleAttributeOperand* pSrc)
{
	OpcUa_SimpleAttributeOperand* pTarget=(OpcUa_SimpleAttributeOperand*)OpcUa_Alloc(sizeof(OpcUa_SimpleAttributeOperand));
	OpcUa_SimpleAttributeOperand_Initialize(pTarget);
	// TypeDefinitionId
	OpcUa_NodeId* pNodeId=Utils::Copy(&(pSrc->TypeDefinitionId));
	if (pNodeId)
		pTarget->TypeDefinitionId=*pNodeId;
	// NoOfBrowsePath
	pTarget->NoOfBrowsePath=pSrc->NoOfBrowsePath;
	// BrowsePath
	pTarget->BrowsePath=(OpcUa_QualifiedName*)OpcUa_Alloc(sizeof(OpcUa_QualifiedName)*(pTarget->NoOfBrowsePath));
	for (int ii=0;ii<pTarget->NoOfBrowsePath;ii++)
	{
		pTarget->BrowsePath[ii]=*Utils::Copy(&(pSrc->BrowsePath[ii]));
	}
	pTarget->AttributeId=pSrc->AttributeId;
	// IndexRange
	if (OpcUa_String_StrLen(&(pSrc->IndexRange)))
		OpcUa_String_StrnCpy(&(pTarget->IndexRange),
			&(pSrc->IndexRange),
			OpcUa_String_StrLen(&(pSrc->IndexRange)) );
	return pTarget;
}
OpcUa_QualifiedName* Utils::Copy(OpcUa_QualifiedName* pSrc)
{
	OpcUa_QualifiedName* pTarget=(OpcUa_QualifiedName*)OpcUa_Alloc(sizeof(OpcUa_QualifiedName));
	OpcUa_QualifiedName_Initialize(pTarget);
	// Name
	if (OpcUa_String_StrLen(&(pSrc->Name)))
		OpcUa_String_StrnCpy(&(pTarget->Name),
			&(pSrc->Name),
			OpcUa_String_StrLen(&(pSrc->Name)) );
	// NamespaceIndex
	pTarget->NamespaceIndex=pSrc->NamespaceIndex;
	// Reserved
	pTarget->Reserved=pSrc->Reserved;
	return pTarget;
}
//OpcUa_ContentFilter* Utils::Copy(OpcUa_ContentFilter* pSrc)
//{
//	OpcUa_ContentFilter* pTarget=(OpcUa_ContentFilter*)OpcUa_Alloc(sizeof(OpcUa_ContentFilter));
//	OpcUa_ContentFilter_Initialize(pTarget);
//	// NoOfElements
//	pTarget->NoOfElements=pSrc->NoOfElements;
//	// Elements
//	pTarget->Elements=(OpcUa_ContentFilterElement*)OpcUa_Alloc(sizeof(OpcUa_ContentFilterElement)*(pTarget->NoOfElements));
//	for (int ii=0;ii<pTarget->NoOfElements;ii++)
//	{
//		pTarget->Elements[ii]=*Utils::Copy(&(pSrc->Elements[ii]));
//	}
//
//	return pTarget;
//}
//OpcUa_ContentFilterElement* Utils::Copy(OpcUa_ContentFilterElement* pSrc)
//{
//	OpcUa_ContentFilterElement* pTarget=(OpcUa_ContentFilterElement*)OpcUa_Alloc(sizeof(OpcUa_ContentFilterElement));
//	OpcUa_ContentFilterElement_Initialize(pTarget);
//	//FilterOperator
//	pTarget->FilterOperator=pSrc->FilterOperator;
//	//NoOfFilterOperands
//	pTarget->NoOfFilterOperands=pSrc->NoOfFilterOperands;
//	//FilterOperands
//	pTarget->FilterOperands=(OpcUa_ExtensionObject*)OpcUa_Alloc(sizeof(OpcUa_ExtensionObject)*(pTarget->NoOfFilterOperands));
//	for (int ii=0;ii<pTarget->NoOfFilterOperands;ii++)
//		pTarget->FilterOperands[ii]=*Utils::Copy(&(pSrc->FilterOperands[ii]));
//
//	return pTarget;
//}
OpcUa_ContentFilterElementResult* Utils::Copy(OpcUa_ContentFilterElementResult* pSrc)
{
	OpcUa_ContentFilterElementResult* pTarget=(OpcUa_ContentFilterElementResult*)OpcUa_Alloc(sizeof(OpcUa_ContentFilterElementResult));
	OpcUa_ContentFilterElementResult_Initialize(pTarget);
	// StatusCode
	pTarget->StatusCode=pSrc->StatusCode;
	// NoOfOperandStatusCodes
	pTarget->NoOfOperandStatusCodes=pSrc->NoOfOperandStatusCodes;
	// OperandStatusCodes
	for (int ii=0;ii<pSrc->NoOfOperandStatusCodes;ii++)
		pTarget->OperandStatusCodes[ii]=pSrc->OperandStatusCodes[ii];
	// NoOfOperandDiagnosticInfos
	pTarget->NoOfOperandDiagnosticInfos=pSrc->NoOfOperandDiagnosticInfos;
	// OperandDiagnosticInfos
	for (int ii=0;ii<pTarget->NoOfOperandDiagnosticInfos;ii++)
	{
		OpcUa_DiagnosticInfo* pDiag=Utils::Copy(&(pSrc->OperandDiagnosticInfos[ii]));
		if (pDiag)
			pTarget->OperandDiagnosticInfos[ii]=*pDiag;
	}

	return pTarget;
}
OpcUa_ContentFilterResult* Utils::Copy(OpcUa_ContentFilterResult* pSrc)
{
	OpcUa_ContentFilterResult* pTarget=(OpcUa_ContentFilterResult*)OpcUa_Alloc(sizeof(OpcUa_ContentFilterResult));
	OpcUa_ContentFilterResult_Initialize(pTarget);
	// NoOfElementResults
	pTarget->NoOfElementResults=pSrc->NoOfElementResults;
	// ElementResults
	for (int ii=0;ii<pTarget->NoOfElementResults;ii++)
	{
		OpcUa_ContentFilterElementResult* pContentFilterElementResult=Utils::Copy(&(pSrc->ElementResults[ii]));
		if (pContentFilterElementResult)
			pTarget->ElementResults[ii]=*pContentFilterElementResult;
	}
	// NoOfElementDiagnosticInfos
	pTarget->NoOfElementDiagnosticInfos=pSrc->NoOfElementDiagnosticInfos;
	// ElementDiagnosticInfos
	for (int ii=0;ii<pTarget->NoOfElementDiagnosticInfos;ii++)
	{
		OpcUa_DiagnosticInfo* pDiag=Utils::Copy(&(pSrc->ElementDiagnosticInfos[ii]));
		if (pDiag)
			pTarget->ElementDiagnosticInfos[ii]=*pDiag;
	}
	return pTarget;
}
//OpcUa_ExpandedNodeId Utils::Copy(OpcUa_ExpandedNodeId* pSrc)
//{
//	OpcUa_ExpandedNodeId* pTarget=(OpcUa_ExpandedNodeId*)OpcUa_Alloc(sizeof(OpcUa_ExpandedNodeId));
//	if(pTarget)
//	{
//		OpcUa_ExpandedNodeId_Initialize(pTarget);
//		// NodeId
//		OpcUa_NodeId* pNodeId=Utils::Copy(&(pSrc->NodeId));
//		if (pNodeId)
//			pTarget->NodeId=*pNodeId;
//		// ServerIndex
//		pTarget->ServerIndex=pSrc->ServerIndex;
//		// NamespaceUri
//		OpcUa_String_Initialize(&(pTarget->NamespaceUri));
//		if (OpcUa_String_StrLen(&(pSrc->NamespaceUri)))
//			OpcUa_String_StrnCpy(&(pTarget->NamespaceUri),
//				&(pSrc->NamespaceUri),
//				OpcUa_String_StrLen(&(pSrc->NamespaceUri)) );
//	}
//	else
//		OpcUa_Trace(OPCUA_TRACE_EXTRA_LEVEL_ERROR,"Memory allocation error\n");
//	return *pTarget;
//}
//============================================================================
// Utils::Copy(std::vector<unsigned char>)
//============================================================================
OpcUa_ByteString Utils::Copy(std::vector<unsigned char> src)
{
	OpcUa_ByteString tDst;
	OpcUa_ByteString_Initialize(&tDst);

	if (src.empty())
	{
		return tDst;
	}

	tDst.Length = src.size();
	tDst.Data = (OpcUa_Byte*)OpcUa_Alloc(tDst.Length);

	if (tDst.Data == 0)
	{
		OpcUa_ByteString_Initialize(&tDst);
		return tDst;
	}

	for (int ii = 0; ii < tDst.Length; ii++)
	{
		tDst.Data[ii] = src[ii];
	}

	return tDst;
}

//============================================================================
// Utils::Copy(OpcUa_String)
//============================================================================
//std::string Utils::Copy(OpcUa_String* pSrc)
//{
//	std::string dst;
//
//	if (pSrc == 0)
//	{
//		return dst;
//	}
//
//	OpcUa_StringA pData = OpcUa_String_GetRawString((OpcUa_String*)pSrc);
//	OpcUa_UInt32 nLength = OpcUa_String_StrLen((OpcUa_String*)pSrc);
//
//	dst.assign(pData, nLength);
//
//	return dst;
//}
OpcUa_String* Utils::Copy(OpcUa_String* pSrc)
{
	OpcUa_String* pTarget=OpcUa_Null;
	if (pSrc)
	{
		if (OpcUa_String_StrLen(pSrc)>0)
		{
			pTarget=(OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
			if (pTarget)
			{
				OpcUa_String_Initialize(pTarget);
				OpcUa_String_StrnCpy(pTarget,
						pSrc,
						OpcUa_String_StrLen(pSrc) );
			}
		}
	}
	return pTarget;
}
OpcUa_ByteString* Utils::Copy(OpcUa_ByteString* pSrc)
{ 
	OpcUa_ByteString* pTarget=OpcUa_Null;
	if (pSrc)
	{
		pTarget=(OpcUa_ByteString*)OpcUa_Alloc(sizeof(OpcUa_ByteString));
		if (pTarget)
		{
			if (pSrc->Length>0)
			{
				pTarget->Data=(OpcUa_Byte*)OpcUa_Alloc(pSrc->Length);
				if (pTarget->Data)
				{
					ZeroMemory(pTarget->Data,pSrc->Length);
					pTarget->Length= pSrc->Length;
					OpcUa_MemCpy(pTarget->Data, 
						pSrc->Length,
						pSrc->Data,
						pSrc->Length);
				}
				else
				{
					OpcUa_Free(pTarget);
					pTarget=OpcUa_Null;
				}
			}
			else
			{
				OpcUa_Free(pTarget);
				pTarget=OpcUa_Null;
			}
		}
	}
	return pTarget;
}
//============================================================================
// Utils::Copy(OpcUa_ByteString)
//============================================================================
std::vector<unsigned char> Utils::Copy(const OpcUa_ByteString* pSrc)
{
	std::vector<unsigned char> dst;

	if (pSrc == 0 || pSrc->Length <= 0)
	{
		return dst;
	}

	dst.reserve(pSrc->Length);

	for (int ii = 0; ii < pSrc->Length; ii++)
	{
		dst.push_back(pSrc->Data[ii]);
	}

	return dst;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Convert a OpcUa__DateTime in a string simple representation. </summary>
///
/// <remarks>	Michel, 15/09/2016. </remarks>
///
/// <param name="dt">	  	The dt. </param>
/// <param name="strTime">	[in,out] If non-null, the time. </param>
///
/// <returns>	An OpcUa_StatusCode. </returns>
///-------------------------------------------------------------------------------------------------

OpcUa_StatusCode Utils::OpcUaDateTimeToString(OpcUa_DateTime dt, OpcUa_String** strTime)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	if (*strTime)
		uStatus=OpcUa_BadInvalidArgument;
	else
	{
		*strTime=(OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
		if (*strTime)
		{
			OpcUa_String_Initialize(*strTime);
			FILETIME ft;
			SYSTEMTIME sysTime;
			ft.dwHighDateTime=dt.dwHighDateTime;
			ft.dwLowDateTime=dt.dwLowDateTime;
			if (FileTimeToSystemTime(&ft, &sysTime))
			{
				char* buffer = (char*)malloc(25);
				if (buffer)
				{
					memset(buffer, 0, 25);
					OpcUa_SPrintfA(buffer, "%02u/%02u/%04u %02u:%02u:%02u.%03u\n",
						sysTime.wDay, sysTime.wMonth, sysTime.wYear, sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds);
					OpcUa_String_AttachCopy(*strTime, (OpcUa_CharA*)buffer);
					OpcUa_Free(buffer);
					uStatus = OpcUa_Good;
				}
			}
			else
			{
				OpcUa_String_Clear(*strTime);
				OpcUa_Free(*strTime);
				*strTime = OpcUa_Null;
				uStatus=OpcUa_BadInvalidArgument;
			}
		}
		else
			uStatus=OpcUa_BadOutOfMemory;
	}
	return uStatus;
}
//============================================================================
// Utils::StatusToString
//============================================================================
std::string Utils::StatusToString(OpcUa_StatusCode uStatus)
{
	std::string sError;

	switch(uStatus)
	{
		case OpcUa_BadTimeout:
		{
			sError = "OpcUa_BadTimeout";
			break;
		}

		case OpcUa_BadCommunicationError:
		{
			sError = "OpcUa_BadCommunicationError";
			break;
		}

		case OpcUa_BadConnectionClosed:
		{
			sError = "OpcUa_BadConnectionClosed";
			break;
		}

		case OpcUa_BadCertificateInvalid:
		{
			sError = "OpcUa_BadCertificateInvalid";
			break;
		}

		case OpcUa_BadCertificateTimeInvalid:
		{
			sError = "OpcUa_BadCertificateTimeInvalid";
			break;
		}

		case OpcUa_BadCertificateRevoked:
		{
			sError = "OpcUa_BadCertificateRevoked";
			break;
		}

		case OpcUa_BadCertificateUntrusted:
		{
			sError = "OpcUa_BadCertificateUntrusted";
			break;
		}

		case OpcUa_BadCertificateIssuerRevocationUnknown:
		{
			sError = "OpcUa_BadCertificateIssuerRevocationUnknown";
			break;
		}

		case OpcUa_BadConnectionRejected:
		{
			sError = "OpcUa_BadConnectionRejected";
			break;
		}

		default:
		{
			sError = "Unknown Error";
		}
	}

	return sError;
}

OpcUa_StatusCode Utils::OpcUaVariantArrayToString(const OpcUa_Variant& Var, OpcUa_String** strValue)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_CharA cStart[2]="[";
	OpcUa_CharA cEnd[2]="]";

	char* wcsLocal = (char*)malloc(4096 * sizeof(char));
	if (wcsLocal)
	{
		ZeroMemory(wcsLocal, 4096);

		// taille du tableau
		for (int ii = 0; ii < Var.Value.Array.Length; ii++)
		{
			char* wcsTmp = (char*)malloc(1024 * sizeof(char));
			if (wcsTmp)
			{
				ZeroMemory(wcsTmp, 1024);
				switch (Var.Datatype)
				{
				case OpcUaType_Null:// 0,
					OpcUa_SPrintfA(wcsTmp, "OpcUa_VariantNull,");
					break;
				case OpcUaType_Boolean: // 1,
					OpcUa_SPrintfA(wcsTmp, "%hu", Var.Value.Array.Value.BooleanArray[ii]);
					break;
				case OpcUaType_SByte: // 2,
					OpcUa_SPrintfA(wcsTmp, "%hd", Var.Value.Array.Value.SByteArray[ii]);

					break;
				case OpcUaType_Byte: // 3,
					OpcUa_SPrintfA(wcsTmp, "%hu", Var.Value.Array.Value.ByteArray[ii]);
					break;
				case OpcUaType_Int16: // 4,
					OpcUa_SPrintfA(wcsTmp, "%hd", Var.Value.Array.Value.Int16Array[ii]);
					break;
				case OpcUaType_UInt16: // 5,
					OpcUa_SPrintfA(wcsTmp, "%hu", Var.Value.Array.Value.UInt16Array[ii]);
					break;
				case OpcUaType_Int32: // 6,
					OpcUa_SPrintfA(wcsTmp, "%ld", Var.Value.Array.Value.Int32Array[ii]);
					break;
				case OpcUaType_UInt32: // 7,
					OpcUa_SPrintfA(wcsTmp, "%lu", Var.Value.Array.Value.UInt32Array[ii]);
					break;
				case OpcUaType_Int64: // 8,
					OpcUa_SPrintfA(wcsTmp, "%lld", Var.Value.Array.Value.Int64Array[ii]);
					break;
				case OpcUaType_UInt64: // 9,
					OpcUa_SPrintfA(wcsTmp, "%llu", Var.Value.Array.Value.UInt64Array[ii]);
					break;
				case OpcUaType_Float: // 10,
					OpcUa_SPrintfA(wcsTmp, "%f", Var.Value.Array.Value.FloatArray[ii]);
					break;
				case OpcUaType_Double: // 11,
					OpcUa_SPrintfA(wcsTmp, "%lf", Var.Value.Array.Value.DoubleArray[ii]);
					break;
				case OpcUaType_String: // 12,		
					//wcsTmp = OpcUa_String_GetRawString(&(Var.Value.Array.Value.StringArray[ii]));
					{
						OpcUa_UInt32 uiLen = OpcUa_String_StrLen(&(Var.Value.Array.Value.StringArray[ii]));
						OpcUa_MemCpy(wcsTmp, uiLen, OpcUa_String_GetRawString(&(Var.Value.Array.Value.StringArray[ii])), uiLen);
					}
					break;
				case OpcUaType_DateTime: // 13,
				{
					OpcUa_String* strTime = OpcUa_Null; // (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
					uStatus = OpcUaDateTimeToString(Var.Value.Array.Value.DateTimeArray[ii], &strTime);
					if (uStatus == OpcUa_Good)
					{
						OpcUa_UInt32 uiLen = OpcUa_String_StrLen(strTime);
						OpcUa_MemCpy(wcsTmp, uiLen, OpcUa_String_GetRawString(strTime), uiLen);
					}
					if (strTime)
					{
						OpcUa_String_Clear(strTime);
						OpcUa_Free(strTime);
					}
				}
					break;
				case OpcUaType_Guid: // 14,
					break;
				case OpcUaType_ByteString: // 15,
					uStatus = OpcUa_BadInvalidArgument;
					break;
				case OpcUaType_XmlElement: // 16,
					uStatus = OpcUa_BadInvalidArgument;
					break;
				case OpcUaType_NodeId: // 17,
					if (Var.Value.Array.Value.NodeIdArray[ii].IdentifierType == OpcUa_IdentifierType_Numeric)
					{
						OpcUa_SPrintfA(wcsTmp, "ns=%u;i=%lu",
							Var.Value.Array.Value.NodeIdArray[ii].NamespaceIndex,
							Var.Value.Array.Value.NodeIdArray[ii].Identifier.Numeric);
					}
					else
					{
						if (Var.Value.Array.Value.NodeIdArray[ii].IdentifierType == OpcUa_IdentifierType_String)
						{

							OpcUa_SPrintfA(wcsTmp, "ns=%u",
								Var.Value.Array.Value.NodeIdArray[ii].NamespaceIndex);
							OpcUa_StrnCatA(wcsTmp, 1024,
								OpcUa_String_GetRawString(&(Var.Value.Array.Value.NodeIdArray[ii].Identifier.String)),
								OpcUa_String_StrLen(&(Var.Value.Array.Value.NodeIdArray[ii].Identifier.String)));
						}
					}
					break;
				case OpcUaType_ExpandedNodeId: // 18,
					uStatus = OpcUa_BadInvalidArgument;
					break;
				case OpcUaType_StatusCode: // 19,
					uStatus = OpcUa_BadInvalidArgument;
					break;
				case OpcUaType_QualifiedName: // 20,
					wcsTmp = OpcUa_String_GetRawString(&(Var.Value.Array.Value.QualifiedNameArray[ii].Name));
					break;
				case OpcUaType_LocalizedText: // 21,
				{
					//wcsTmp = OpcUa_String_GetRawString(&(Var.Value.Array.Value.LocalizedTextArray[ii].Text));
					OpcUa_UInt32 uiLen = OpcUa_String_StrLen(&(Var.Value.Array.Value.LocalizedTextArray[ii].Text));
					OpcUa_MemCpy(wcsTmp, uiLen, OpcUa_String_GetRawString(&(Var.Value.Array.Value.LocalizedTextArray[ii].Text)), uiLen);
				}
				break;
				case OpcUaType_ExtensionObject: // 22,
				{
					OpcUa_String* pString = OpcUa_Null;
					uStatus = OpcUaExtentionObjectToString((Var.Value.Array.Value.ExtensionObjectArray[ii]), &pString);
					if (uStatus != OpcUa_Good)
						throw CStatusCodeException(OpcUa_Bad, (OpcUa_CharA*)"OpcUaType_ExtensionObject unknown");
					else
						wcsTmp = OpcUa_String_GetRawString(pString);
				}
					break;
				case OpcUaType_DataValue: // 23,
					uStatus = OpcUa_BadInvalidArgument;
					break;
				case OpcUaType_Variant: // 24,
				case OpcUaType_DiagnosticInfo: // 25
					uStatus = OpcUa_BadInvalidArgument;
					break;
				default:
					uStatus = OpcUa_BadInvalidArgument;
					break;
				}
				// On transfert le contenu dans le buffer et on place les delimiteurs "[]"
				OpcUa_UInt32 iLen = OpcUa_StrLenA(wcsLocal) + OpcUa_StrLenA(wcsTmp);
				if (iLen < 4000)
				{
					OpcUa_StrnCatA(wcsLocal, 4096, cStart, 1);
					OpcUa_StrnCatA(wcsLocal, 4096, wcsTmp, OpcUa_StrLenA(wcsTmp));
					OpcUa_StrnCatA(wcsLocal, 4096, cEnd, 1);
				}
				else
				{
					OpcUa_Free(wcsTmp);
					break;
				}
				OpcUa_Free(wcsTmp);
			}
		}
		if (*strValue == OpcUa_Null)
		{
			*strValue = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
			OpcUa_String_Initialize(*strValue);
		}
		else
			OpcUa_String_Clear(*strValue);
		uStatus=OpcUa_String_AttachCopy(*strValue, wcsLocal);
		ZeroMemory(wcsLocal, 4096);
		OpcUa_Free(wcsLocal);
	}
	else
		uStatus = OpcUa_BadOutOfMemory;
	return uStatus;
}

OpcUa_StatusCode Utils::OpcUaVariantToString(const OpcUa_Variant& Var, OpcUa_String** strValue)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
		
	if (Var.ArrayType>0)
	{
		// Il s'agit d'un tableau
		uStatus = OpcUaVariantArrayToString(Var, strValue);
	}
	else
	{
		char* wcsLocal=(char*)OpcUa_Alloc(1024*sizeof(char));
		if (wcsLocal)
		{
			ZeroMemory(wcsLocal,1024);
			switch (Var.Datatype)
			{
			case OpcUaType_Null:// 0,
				OpcUa_SPrintfA(wcsLocal, "OpcUa_VariantNull");
				break;
			case OpcUaType_Boolean: // 1,
				OpcUa_SPrintfA(wcsLocal, "%hu", Var.Value.Boolean);
				break;
			case OpcUaType_SByte: // 2,
				OpcUa_SPrintfA(wcsLocal, "%hd", Var.Value.SByte);
				break;
			case OpcUaType_Byte: // 3,
				OpcUa_SPrintfA(wcsLocal, "%hu", Var.Value.Byte);
				break;
			case OpcUaType_Int16: // 4,
				OpcUa_SPrintfA(wcsLocal, "%hd", Var.Value.Int16);
				break;
			case OpcUaType_UInt16: // 5,
				OpcUa_SPrintfA(wcsLocal, "%hu", Var.Value.UInt16);
				break;
			case OpcUaType_Int32: // 6,
				OpcUa_SPrintfA(wcsLocal, "%ld", Var.Value.Int32);
				break;
			case OpcUaType_UInt32: // 7,
				OpcUa_SPrintfA(wcsLocal, "%lu", Var.Value.UInt32);
				break;
			case OpcUaType_Int64: // 8,
				OpcUa_SPrintfA(wcsLocal, "%lld", Var.Value.Int64);
				break;
			case OpcUaType_UInt64: // 9,
				OpcUa_SPrintfA(wcsLocal, "%llu", Var.Value.UInt64);
				break;
			case OpcUaType_Float: // 10,
				OpcUa_SPrintfA(wcsLocal, "%f", Var.Value.Float);
				break;
			case OpcUaType_Double: // 11,
				OpcUa_SPrintfA(wcsLocal, "%lf", Var.Value.Double);
				break;
			case OpcUaType_String: // 12,
				{
					OpcUa_UInt32 uiLen = OpcUa_String_StrLen(&(Var.Value.String));
					OpcUa_MemCpy(wcsLocal, uiLen, OpcUa_String_GetRawString(&(Var.Value.String)), uiLen);
				}
				break;
			case OpcUaType_DateTime: // 13,
			{
				OpcUa_String* strTime = OpcUa_Null;
				uStatus = OpcUaDateTimeToString(Var.Value.DateTime, &strTime);
				if (uStatus == OpcUa_Good)
				{
					//wcsLocal = OpcUa_String_GetRawString(strTime);
					OpcUa_UInt32 uiLen = OpcUa_String_StrLen(strTime);
					OpcUa_MemCpy(wcsLocal, uiLen, OpcUa_String_GetRawString(strTime), uiLen);
					if (strTime)
					{
						OpcUa_String_Clear(strTime);
						OpcUa_Free(strTime);
					}
				}
			}
				break;
			case OpcUaType_Guid: // 14,
				break;
			case OpcUaType_ByteString: // 15,
				uStatus = OpcUa_BadInvalidArgument;
				break;
			case OpcUaType_XmlElement: // 16,
				uStatus = OpcUa_BadInvalidArgument;
				break;
			case OpcUaType_NodeId: // 17,
			{
				if (Var.Value.NodeId->IdentifierType == OpcUa_IdentifierType_Numeric)
				{
					OpcUa_SPrintfA(wcsLocal, "ns=%u;i=%lu",
						Var.Value.NodeId->NamespaceIndex,
						Var.Value.NodeId->Identifier.Numeric);
				}
				else
				{
					if (Var.Value.NodeId->IdentifierType == OpcUa_IdentifierType_String)
					{
						OpcUa_SPrintfA(wcsLocal, "ns=%u;",
							Var.Value.NodeId->NamespaceIndex);
						strcat(wcsLocal, OpcUa_String_GetRawString(&(Var.Value.NodeId->Identifier.String)));
					}
				}
			}
				break;
			case OpcUaType_ExpandedNodeId: // 18,
				uStatus = OpcUa_BadInvalidArgument;
				break;
			case OpcUaType_StatusCode: // 19,
				uStatus = OpcUa_BadInvalidArgument;
				break;
			case OpcUaType_QualifiedName: // 20,
				if (Var.Value.QualifiedName)
				{
					OpcUa_SPrintfA(wcsLocal, "[ns=%u]-%s",
						Var.Value.QualifiedName->NamespaceIndex,
						OpcUa_String_GetRawString(&(Var.Value.QualifiedName->Name)));
				}
				break;
			case OpcUaType_LocalizedText: // 21,	
				if (Var.Value.LocalizedText)
				{
					OpcUa_SPrintfA(wcsLocal, "%s - %s",
						OpcUa_String_GetRawString(&(Var.Value.LocalizedText->Locale)),
						OpcUa_String_GetRawString(&(Var.Value.LocalizedText->Text)));
				}
				break;
			case OpcUaType_ExtensionObject: // 22,
				uStatus = OpcUaExtentionObjectToString(*(Var.Value.ExtensionObject), strValue);
				if (uStatus != OpcUa_Good)
					OpcUa_Trace(OPCUA_TRACE_EXTRA_LEVEL_ERROR, "OpcUaType_ExtensionObject unknown\n");
				break;
			case OpcUaType_DataValue: // 23,
				break;
			case OpcUaType_Variant: // 24,
			case OpcUaType_DiagnosticInfo: // 25
				uStatus = OpcUa_BadInvalidArgument;
				break;
			default:
				uStatus = OpcUa_BadInvalidArgument;
				break;
			}
			
			if (*strValue==OpcUa_Null)
			{
				*strValue=(OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));	
				OpcUa_String_Initialize(*strValue);		
			}
			else
				OpcUa_String_Clear(*strValue);	

			uStatus=OpcUa_String_AttachCopy(*strValue,wcsLocal);
			OpcUa_Free(wcsLocal);
		}
	}
	return uStatus;
}

OpcUa_StatusCode Utils::OpcUaExtentionObjectToString(OpcUa_ExtensionObject& extObject, OpcUa_String** strValue)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	std::string myString=typeid(extObject).name();
	//// Les délimiteus sont utilisé pour mettre en forme certain ExtensionObject
	(*strValue)=(OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize((*strValue));
	OpcUa_String start;
	OpcUa_String end;
	OpcUa_String separator;
	OpcUa_String_Initialize(&separator);
	OpcUa_String_Initialize(&start);
	OpcUa_String_Initialize(&end);
	OpcUa_String_AttachCopy(&separator," - ");
	OpcUa_String_AttachCopy(&start,"[ ");
	OpcUa_String_AttachCopy(&end,"] ");
	// 
	if (myString==std::string("struct _OpcUa_ExtensionObject"))
	{
		if (extObject.Body.EncodeableObject.Type)
		{
			char* wcsLocal=(char*)malloc(1024*sizeof(char));
			if (wcsLocal)
			{
				ZeroMemory(wcsLocal, 1024);
				switch (extObject.Body.EncodeableObject.Type->TypeId)
				{
				case OpcUaId_ServerStatusDataType:
				{
					OpcUa_ServerStatusDataType* pServerStatus =
						(OpcUa_ServerStatusDataType*)extObject.Body.EncodeableObject.Object;
					/////////////////////////////////////////////////////////////////////////
					// StartTime
					OpcUa_String* strVal = OpcUa_Null;
					OpcUa_Variant aVar;
					OpcUa_Variant_Initialize(&aVar);
					aVar.Datatype = OpcUaType_DateTime;
					aVar.Value.DateTime = pServerStatus->StartTime;
					uStatus = Utils::OpcUaVariantToString(aVar, &strVal);
					if (uStatus == OpcUa_Good)
					{
						OpcUa_String_StrnCat(strVal, &separator, OpcUa_String_StrLen(&separator));
						OpcUa_String_StrnCat(*strValue, strVal, OpcUa_String_StrLen(strVal));
					}
					if (strVal)
					{
						OpcUa_String_Clear(strVal);
						OpcUa_Free(strVal);
						strVal = OpcUa_Null;
					}
					/////////////////////////////////////////////////////////////////////////
					// CurrentTime
					OpcUa_String_Clear(strVal);
					OpcUa_Free(strVal);
					OpcUa_Variant_Initialize(&aVar);
					aVar.Datatype = OpcUaType_DateTime;
					aVar.Value.DateTime = pServerStatus->CurrentTime;
					uStatus = Utils::OpcUaVariantToString(aVar, &strVal);
					if (uStatus == OpcUa_Good)
					{
						OpcUa_String_StrnCat(strVal, &separator, OpcUa_String_StrLen(&separator));
						OpcUa_String_StrnCat(*strValue, strVal, OpcUa_String_StrLen(strVal));
					}
					// ServerState
					if (strVal)
					{
						OpcUa_String_Clear(strVal);
						OpcUa_Free(strVal);
						strVal = OpcUa_Null;
					}
					strVal = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
					OpcUa_String_Initialize(strVal);
					switch (pServerStatus->State) // OpcUa_ServerState
					{
					case OpcUa_ServerState_Running:
						OpcUa_String_AttachCopy(strVal, "Running");
						break;
					case OpcUa_ServerState_Failed:
						OpcUa_String_AttachCopy(strVal, "Failed");
						break;
					case OpcUa_ServerState_NoConfiguration:
						OpcUa_String_AttachCopy(strVal, "NoConfiguration");
						break;
					case OpcUa_ServerState_Suspended:
						OpcUa_String_AttachCopy(strVal, "Suspended");
						break;
					case OpcUa_ServerState_Shutdown:
						OpcUa_String_AttachCopy(strVal, "Shutdown");
						break;
					case OpcUa_ServerState_Test:
						OpcUa_String_AttachCopy(strVal, "Test");
						break;
					case OpcUa_ServerState_CommunicationFault:
						OpcUa_String_AttachCopy(strVal, "CommunicationFault");
						break;
					case OpcUa_ServerState_Unknown:
						OpcUa_String_AttachCopy(strVal, "Unknown");
						break;
					default:
						break;
					}
					OpcUa_String_StrnCat(*strValue, strVal, OpcUa_String_StrLen(strVal));
					if (strVal)
					{
						OpcUa_String_Clear(strVal);
						OpcUa_Free(strVal);
						strVal = OpcUa_Null;
					}
					// BuildInfo				
					OpcUa_String_StrnCat(*strValue, &(pServerStatus->BuildInfo.ProductUri), OpcUa_String_StrLen(&(pServerStatus->BuildInfo.ProductUri)));
					OpcUa_String_StrnCat(*strValue, &separator, OpcUa_String_StrLen(&separator));

					OpcUa_String_StrnCat(*strValue, &(pServerStatus->BuildInfo.ManufacturerName), OpcUa_String_StrLen(&(pServerStatus->BuildInfo.ManufacturerName)));
					OpcUa_String_StrnCat(*strValue, &separator, OpcUa_String_StrLen(&separator));

					OpcUa_String_StrnCat(*strValue, &(pServerStatus->BuildInfo.ProductName), OpcUa_String_StrLen(&(pServerStatus->BuildInfo.ProductName)));
					OpcUa_String_StrnCat(*strValue, &separator, OpcUa_String_StrLen(&separator));

					OpcUa_String_StrnCat(*strValue, &(pServerStatus->BuildInfo.SoftwareVersion), OpcUa_String_StrLen(&(pServerStatus->BuildInfo.SoftwareVersion)));
					OpcUa_String_StrnCat(*strValue, &separator, OpcUa_String_StrLen(&separator));
					if (strVal)
					{
						OpcUa_String_Clear(strVal);
						OpcUa_Free(strVal);
						strVal = OpcUa_Null;
					}
					OpcUa_Variant_Clear(&aVar);
					OpcUaDateTimeToString(pServerStatus->BuildInfo.BuildDate, &strVal);

					OpcUa_String_StrnCat(strVal, &separator, OpcUa_String_StrLen(&separator));
					OpcUa_String_StrnCat(*strValue, strVal, OpcUa_String_StrLen(strVal));
					if (strVal)
					{
						OpcUa_String_Clear(strVal);
						OpcUa_Free(strVal);
						strVal = OpcUa_Null;
					}
					// SecondsTillShutdown
					OpcUa_Variant_Initialize(&aVar);
					aVar.Datatype = OpcUaType_UInt32;
					aVar.Value.UInt32 = pServerStatus->SecondsTillShutdown;
					uStatus = Utils::OpcUaVariantToString(aVar, &strVal);
					if (uStatus == OpcUa_Good)
					{
						OpcUa_String_StrnCat(strVal, &separator, OpcUa_String_StrLen(&separator));
						OpcUa_String_StrnCat(*strValue, strVal, OpcUa_String_StrLen(strVal));
					}
					// ShutdownReason
					OpcUa_String_StrnCat(*strValue, &(pServerStatus->ShutdownReason.Text), OpcUa_String_StrLen(&(pServerStatus->ShutdownReason.Text)));
					if (strVal)
					{
						OpcUa_String_Clear(strVal);
						OpcUa_Free(strVal);
						strVal = OpcUa_Null;
					}
				}
					break;
				case OpcUaId_ServerDiagnosticsSummaryDataType:
				{
					OpcUa_ServerDiagnosticsSummaryDataType* pServerDiagnosticsSummaryDataType =
						(OpcUa_ServerDiagnosticsSummaryDataType*)extObject.Body.EncodeableObject.Object;
					OpcUa_SPrintfA(wcsLocal,
						"ServerViewCount:%lu CurrentSessionCount:%lu CumulatedSessionCount:%lu SecurityRejectedSessionCount:%lu RejectedSessionCount:%lu SessionTimeoutCount:%lu SessionAbortCount%lu CurrentSubscriptionCount:%lu CumulatedSubscriptionCount:%lu PublishingIntervalCount:%lu SecurityRejectedRequestsCount%lu RejectedRequestsCount:%lu",
						pServerDiagnosticsSummaryDataType->ServerViewCount,
						pServerDiagnosticsSummaryDataType->CurrentSessionCount,
						pServerDiagnosticsSummaryDataType->CumulatedSessionCount,
						pServerDiagnosticsSummaryDataType->SecurityRejectedSessionCount,
						pServerDiagnosticsSummaryDataType->RejectedSessionCount,
						pServerDiagnosticsSummaryDataType->SessionTimeoutCount,
						pServerDiagnosticsSummaryDataType->SessionAbortCount,
						pServerDiagnosticsSummaryDataType->CurrentSubscriptionCount,
						pServerDiagnosticsSummaryDataType->CumulatedSubscriptionCount,
						pServerDiagnosticsSummaryDataType->PublishingIntervalCount,
						pServerDiagnosticsSummaryDataType->SecurityRejectedRequestsCount,
						pServerDiagnosticsSummaryDataType->RejectedRequestsCount);
					OpcUa_String_AttachCopy(*strValue, wcsLocal);
				}
					break;
				case OpcUaId_BuildInfo:
				{
					OpcUa_String* strVal = OpcUa_Null;
					OpcUa_BuildInfo* pBuildInfo =
						(OpcUa_BuildInfo*)extObject.Body.EncodeableObject.Object;
					OpcUa_String_StrnCpy(*strValue, &start, OpcUa_String_StrLen(&start));
					OpcUa_String_StrnCat(*strValue, &(pBuildInfo->ProductUri), OpcUa_String_StrLen(&(pBuildInfo->ProductUri)));
					OpcUa_String_StrnCat(*strValue, &end, OpcUa_String_StrLen(&end));

					OpcUa_String_StrnCat(*strValue, &start, OpcUa_String_StrLen(&start));
					OpcUa_String_StrnCat(*strValue, &(pBuildInfo->ManufacturerName), OpcUa_String_StrLen(&(pBuildInfo->ManufacturerName)));
					OpcUa_String_StrnCat(*strValue, &end, OpcUa_String_StrLen(&end));

					OpcUa_String_StrnCat(*strValue, &start, OpcUa_String_StrLen(&start));
					OpcUa_String_StrnCat(*strValue, &(pBuildInfo->ProductName), OpcUa_String_StrLen(&(pBuildInfo->ProductName)));
					OpcUa_String_StrnCat(*strValue, &end, OpcUa_String_StrLen(&end));

					OpcUa_String_StrnCat(*strValue, &start, OpcUa_String_StrLen(&start));
					OpcUa_String_StrnCat(*strValue, &(pBuildInfo->SoftwareVersion), OpcUa_String_StrLen(&(pBuildInfo->SoftwareVersion)));
					OpcUa_String_StrnCat(*strValue, &end, OpcUa_String_StrLen(&end));

					OpcUa_String_StrnCat(*strValue, &start, OpcUa_String_StrLen(&start));
					OpcUa_String_StrnCat(*strValue, &(pBuildInfo->BuildNumber), OpcUa_String_StrLen(&(pBuildInfo->BuildNumber)));
					OpcUa_String_StrnCat(*strValue, &end, OpcUa_String_StrLen(&end));

					OpcUa_Variant aVar;
					OpcUa_Variant_Initialize(&aVar);
					aVar.Datatype = OpcUaType_DateTime;
					aVar.Value.DateTime = pBuildInfo->BuildDate;
					uStatus = Utils::OpcUaVariantToString(aVar, &strVal);
					if (uStatus == OpcUa_Good)
					{
						OpcUa_String_StrnCat(*strValue, &start, OpcUa_String_StrLen(&start));
						OpcUa_String_StrnCat(*strValue, strVal, OpcUa_String_StrLen(strVal));
						OpcUa_String_StrnCat(*strValue, &end, OpcUa_String_StrLen(&end));
					}
				}
					break;

				case OpcUaId_ObjectTypeAttributes:
				{
					OpcUa_String* strVal = OpcUa_Null;
					OpcUa_ObjectTypeAttributes* pObjectAttributes =
						(OpcUa_ObjectTypeAttributes*)extObject.Body.EncodeableObject.Object;
					// SpecifiedAttributes;
					OpcUa_Variant aVar;
					OpcUa_Variant_Initialize(&aVar);
					aVar.Datatype = OpcUaType_UInt32;
					aVar.Value.UInt32 = pObjectAttributes->SpecifiedAttributes;
					uStatus = Utils::OpcUaVariantToString(aVar, &strVal);
					OpcUa_String_StrnCpy(*strValue, strVal, OpcUa_String_StrLen(strVal));
					OpcUa_String_StrnCat(*strValue, &separator, OpcUa_String_StrLen(&separator));

					// DisplayName;
					OpcUa_String_StrnCat(*strValue,
						&(pObjectAttributes->DisplayName.Text),
						OpcUa_String_StrLen(&(pObjectAttributes->DisplayName.Text)));

					// Description;
					OpcUa_String_StrnCat(*strValue,
						&(pObjectAttributes->Description.Text),
						OpcUa_String_StrLen(&(pObjectAttributes->Description.Text)));

					// WriteMask;
					OpcUa_Variant_Initialize(&aVar);
					aVar.Datatype = OpcUaType_UInt32;
					aVar.Value.UInt32 = pObjectAttributes->WriteMask;
					uStatus = Utils::OpcUaVariantToString(aVar, &strVal);
					OpcUa_String_StrnCat(*strValue, strVal, OpcUa_String_StrLen(strVal));
					OpcUa_String_StrnCat(*strValue, &separator, OpcUa_String_StrLen(&separator));

					// UserWriteMask;
					OpcUa_Variant_Initialize(&aVar);
					aVar.Datatype = OpcUaType_UInt32;
					aVar.Value.UInt32 = pObjectAttributes->UserWriteMask;
					uStatus = Utils::OpcUaVariantToString(aVar, &strVal);
					OpcUa_String_StrnCat(*strValue, strVal, OpcUa_String_StrLen(strVal));
					OpcUa_String_StrnCat(*strValue, &separator, OpcUa_String_StrLen(&separator));

					// IsAbstract;
					OpcUa_Variant_Initialize(&aVar);
					aVar.Datatype = OpcUaType_Boolean;
					aVar.Value.UInt32 = pObjectAttributes->IsAbstract;
					uStatus = Utils::OpcUaVariantToString(aVar, &strVal);
					OpcUa_String_StrnCat(*strValue, strVal, OpcUa_String_StrLen(strVal));
					OpcUa_String_StrnCat(*strValue, &separator, OpcUa_String_StrLen(&separator));

				}
					break;
				case OpcUaId_SessionDiagnosticsDataType:
				{
					//OpcUa_SessionDiagnosticsDataType* pSessionDiagnosticsDataType=
					//	(OpcUa_SessionDiagnosticsDataType*)extObject.Body.EncodeableObject.Object;
				}
					break;
				case OpcUaId_SubscriptionDiagnosticsDataType:
				{
					//OpcUa_SubscriptionDiagnosticsDataType* pSubscriptionDiagnosticsDataType=
					//	(OpcUa_SubscriptionDiagnosticsDataType*)extObject.Body.EncodeableObject.Object;
				}
					break;
				case OpcUaId_SessionSecurityDiagnosticsDataType:
				{
					//OpcUa_SessionSecurityDiagnosticsDataType* pSessionSecurityDiagnosticsDataType=
					//	(OpcUa_SessionSecurityDiagnosticsDataType*)extObject.Body.EncodeableObject.Object;
				}
					break;
				case OpcUaId_ApplicationDescription:
				{
					//OpcUa_ApplicationDescription* pApplicationDescription=
					//	(OpcUa_ApplicationDescription*)extObject.Body.EncodeableObject.Object;
				}
					break;
				case OpcUaId_ServiceCounterDataType:
				{
					OpcUa_String* strVal = OpcUa_Null;
					OpcUa_ServiceCounterDataType* pServiceCounterDataType =
						(OpcUa_ServiceCounterDataType*)extObject.Body.EncodeableObject.Object;
					OpcUa_Variant aVar;
					OpcUa_Variant_Initialize(&aVar);
					aVar.Datatype = OpcUaType_UInt32;
					aVar.Value.UInt32 = pServiceCounterDataType->TotalCount;
					uStatus = Utils::OpcUaVariantToString(aVar, &strVal);
					OpcUa_String_StrnCpy(*strValue, strVal, OpcUa_String_StrLen(strVal));
					OpcUa_String_StrnCat(*strValue, &separator, OpcUa_String_StrLen(&separator));

					aVar.Value.UInt32 = pServiceCounterDataType->ErrorCount;
					OpcUa_String_Clear(strVal);
					uStatus = Utils::OpcUaVariantToString(aVar, &strVal);
					OpcUa_String_StrnCat(*strValue, strVal, OpcUa_String_StrLen(strVal));

				}
					break;
				case OpcUaId_Range:
				{
					OpcUa_String* strVal = OpcUa_Null;
					OpcUa_Range* pRange =
						(OpcUa_Range*)extObject.Body.EncodeableObject.Object;
					OpcUa_Variant aVar;
					OpcUa_Variant_Initialize(&aVar);
					aVar.Datatype = OpcUaType_Double;
					aVar.Value.Double = pRange->Low;
					uStatus = Utils::OpcUaVariantToString(aVar, &strVal);
					OpcUa_String_StrnCpy(*strValue, strVal, OpcUa_String_StrLen(strVal));
					OpcUa_String_StrnCat(*strValue, &separator, OpcUa_String_StrLen(&separator));

					OpcUa_Variant_Initialize(&aVar);
					aVar.Datatype = OpcUaType_Double;
					aVar.Value.Double = pRange->High;
					OpcUa_String_Clear(strVal);
					uStatus = Utils::OpcUaVariantToString(aVar, &strVal);
					OpcUa_String_StrnCat(*strValue, strVal, OpcUa_String_StrLen(strVal));
				}
					break;
				case OpcUaId_EUInformation:
				{
					OpcUa_String* strVal = OpcUa_Null;
					OpcUa_EUInformation* pEUInformation =
						(OpcUa_EUInformation*)extObject.Body.EncodeableObject.Object;
					OpcUa_Variant aVar;
					OpcUa_Variant_Initialize(&aVar);
					// NamespaceUri

					OpcUa_String_StrnCpy(*strValue, &(pEUInformation->NamespaceUri), OpcUa_String_StrLen(&(pEUInformation->NamespaceUri)));
					OpcUa_String_StrnCat(*strValue, &separator, OpcUa_String_StrLen(&separator));

					// UnitId
					aVar.Datatype = OpcUaType_Int32;
					aVar.Value.Int32 = pEUInformation->UnitId;
					uStatus = Utils::OpcUaVariantToString(aVar, &strVal);
					OpcUa_String_StrnCat(*strValue, strVal, OpcUa_String_StrLen(strVal));
					OpcUa_String_StrnCat(*strValue, &separator, OpcUa_String_StrLen(&separator));
					// DisplayName
					OpcUa_String_StrnCat(*strValue, &(pEUInformation->DisplayName.Text), OpcUa_String_StrLen(&(pEUInformation->DisplayName.Text)));
					OpcUa_String_StrnCat(*strValue, &separator, OpcUa_String_StrLen(&separator));

					// Description;
					OpcUa_String_StrnCat(*strValue, &(pEUInformation->Description.Text), OpcUa_String_StrLen(&(pEUInformation->Description.Text)));
				}
					break;
				case OpcUaId_Argument:
				{
					OpcUa_Argument* pArgument=(OpcUa_Argument*)extObject.Body.EncodeableObject.Object;
					OpcUa_String szNodeId;
					OpcUa_String_Initialize(&szNodeId);
					// Name
					OpcUa_String_StrnCat(*strValue, &pArgument->Name, OpcUa_String_StrLen(&pArgument->Name));
					OpcUa_String_StrnCat(*strValue, &separator, OpcUa_String_StrLen(&separator));
					// DataType
					NodeId2String(pArgument->DataType, &szNodeId);
					OpcUa_String_StrnCat(*strValue, &szNodeId, OpcUa_String_StrLen(&szNodeId));
					OpcUa_String_StrnCat(*strValue, &separator, OpcUa_String_StrLen(&separator));
					// Description
					OpcUa_String_StrnCat(*strValue, &pArgument->Description.Text, OpcUa_String_StrLen(&pArgument->Description.Text));
					OpcUa_String_StrnCat(*strValue, &separator, OpcUa_String_StrLen(&separator));

					//pArgument->NoOfArrayDimensions;
					//pArgument->ArrayDimensions;
					//pArgument->ValueRank;
				}
					break;
				default:
				{
					switch (extObject.Encoding)
					{
					case OpcUa_ExtensionObjectEncoding_Binary:
						break;
					case OpcUa_ExtensionObjectEncoding_Xml:
						break;
					case OpcUa_ExtensionObjectEncoding_EncodeableObject:
						{
							OpcUa_Decoder* a_pDecoder = (OpcUa_Decoder*)OpcUa_Alloc(sizeof(OpcUa_Decoder));
							if (a_pDecoder)
							{
								ZeroMemory(a_pDecoder, sizeof(OpcUa_Decoder));
								extObject.Body.EncodeableObject.Type->Decode((void*)extObject.Body.EncodeableObject.Object, a_pDecoder);
								OpcUa_Free(a_pDecoder);
							}
						}
						break;
					default:
						break;
					}
					uStatus = OpcUa_Bad;
				}
					break;
				}
				OpcUa_Free(wcsLocal);
			}
		}
		else
			uStatus = OpcUa_BadInvalidArgument;
	}
	else
		uStatus = OpcUa_BadInvalidArgument;

	OpcUa_String_Clear(&separator);
	OpcUa_String_Clear(&start);
	OpcUa_String_Clear(&end);
	return uStatus;
}

OpcUa_StatusCode Utils::StringToNodeClass(std::string strNodeClass, OpcUa_NodeClass* iNodeClass)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;

	if (strNodeClass== "Unspecified")
		*iNodeClass=OpcUa_NodeClass_Unspecified;

	if (strNodeClass== "Object")
		*iNodeClass=OpcUa_NodeClass_Object;

	if (strNodeClass== "Variable")
		*iNodeClass=OpcUa_NodeClass_Variable;

	if (strNodeClass== "Method")
		*iNodeClass=OpcUa_NodeClass_Method;

	if (strNodeClass== "ObjectType")
		*iNodeClass=OpcUa_NodeClass_ObjectType;	

	if (strNodeClass== "VariableType")
		*iNodeClass=OpcUa_NodeClass_VariableType;		

	if (strNodeClass== "ReferenceType")
		*iNodeClass=OpcUa_NodeClass_ReferenceType;	

	if (strNodeClass== "DataType")
		*iNodeClass=OpcUa_NodeClass_DataType;

	if (strNodeClass== "View")
		*iNodeClass=OpcUa_NodeClass_View;

	return uStatus;
}
OpcUa_StatusCode Utils::NodeClassToString(OpcUa_Int32 iVal, std::string* NodeClass)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	switch (iVal)
	{
	case 0:
		NodeClass->append("Unspecified");
		break;
	case 1:
		NodeClass->append("Object");
		break;
	case 2:
		NodeClass->append("Variable");
		break;
	case 4:
		NodeClass->append("Method");
		break;
	case 8:
		NodeClass->append("ObjectType");
		break;
	case 16:
		NodeClass->append("VariableType");
		break;
	case 32:
		NodeClass->append("ReferenceType");
		break;
	case 64:
		NodeClass->append("DataType");
		break;
	case 128:
		NodeClass->append("View");
		break;
	default:
		uStatus = OpcUa_Bad;
		break;
	}
	return uStatus;
}
OpcUa_StatusCode ServerStateToString(unsigned int eServerState, wchar_t** strServerState)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	switch (eServerState)
	{
	case OpcUa_ServerState_Running:
		{
			*strServerState=(wchar_t*)malloc(1024*sizeof(wchar_t));
			if (*strServerState)
			{
				ZeroMemory(*strServerState, 1024);
				OpcUa_SWPrintf(*strServerState, 7, L"Running");
			}
			else
				uStatus = OpcUa_BadOutOfMemory;
		}
		break;
	case OpcUa_ServerState_Failed:
		{
			*strServerState=(wchar_t*)malloc(1024*sizeof(wchar_t));
			if (*strServerState)
			{
				ZeroMemory(*strServerState, 1024);
				OpcUa_SWPrintf(*strServerState, 6, L"Failed");
			}
			else
				uStatus = OpcUa_BadOutOfMemory;
		}
		break;
	case OpcUa_ServerState_NoConfiguration:
		{
			*strServerState=(wchar_t*)malloc(1024*sizeof(wchar_t));
			if (*strServerState)
			{
				ZeroMemory(*strServerState, 1024);
				OpcUa_SWPrintf(*strServerState, 15, L"NoConfiguration");
			}
			else
				uStatus = OpcUa_BadOutOfMemory;
		}
		break;
	case OpcUa_ServerState_Suspended:
		{
			*strServerState=(wchar_t*)malloc(1024*sizeof(wchar_t));
			if (*strServerState)
			{
				ZeroMemory(*strServerState, 1024);
				OpcUa_SWPrintf(*strServerState, 9, L"Suspended");
			}
			else
				uStatus = OpcUa_BadOutOfMemory;
		}
		break;
	case OpcUa_ServerState_Shutdown:
		{
			*strServerState=(wchar_t*)malloc(1024*sizeof(wchar_t));
			if (*strServerState)
			{
				ZeroMemory(*strServerState, 1024);
				OpcUa_SWPrintf(*strServerState, 8, L"Shutdown");
			}
			else
				uStatus = OpcUa_BadOutOfMemory;
		}
		break;
	case OpcUa_ServerState_Test:
		{
			*strServerState=(wchar_t*)malloc(1024*sizeof(wchar_t));
			if (*strServerState)
			{
				ZeroMemory(*strServerState, 1024);
				OpcUa_SWPrintf(*strServerState, 4, L"Test");
			}
			else
				uStatus = OpcUa_BadOutOfMemory;
		}
		break;
	case OpcUa_ServerState_CommunicationFault:
		{
			*strServerState=(wchar_t*)malloc(1024*sizeof(wchar_t));
			if (*strServerState)
			{
				ZeroMemory(*strServerState, 1024);
				OpcUa_SWPrintf(*strServerState, 18, L"CommunicationFault");
			}
			else
				uStatus = OpcUa_BadOutOfMemory;
		}
		break;
	case OpcUa_ServerState_Unknown:
		{
			*strServerState=(wchar_t*)malloc(1024*sizeof(wchar_t));
			if (*strServerState)
			{
				ZeroMemory(*strServerState, 1024);
				OpcUa_SWPrintf(*strServerState, 7, L"Unknown");
			}
			else
				uStatus = OpcUa_BadOutOfMemory;
		}
		break;
	default:
		break;
	}
	return uStatus;
}
/// <summary>
/// Convert a NodeId in a char*.
/// </summary>
/// <param name="pNodeId">The nodeId to convert.</param>
/// <param name="strNodeId">The char** with the converted nodeId.</param>
/// <returns></returns>
OpcUa_StatusCode Utils::NodeId2String(OpcUa_NodeId* pNodeId, char** strNodeId)
{
	OpcUa_StatusCode uStatus = OpcUa_BadInvalidArgument;
	OpcUa_Int32 iRes=0;
	if (pNodeId)
	{
		if (strNodeId)
		{
			if (*strNodeId)
				;
			else
			{
				switch (pNodeId->IdentifierType)
				{
				case 0: // Numeric
				{
					char* cLocal = (char*)malloc(1024 * sizeof(char));
					if (cLocal)
					{
						ZeroMemory(cLocal, 1024);
						if (pNodeId->NamespaceIndex == 0)
						{
							iRes=OpcUa_SPrintfA(cLocal, "i=%lu",
								pNodeId->Identifier.Numeric);
						}
						else
						{
							iRes=OpcUa_SPrintfA(cLocal, "ns=%u;i=%lu",
								pNodeId->NamespaceIndex,
								pNodeId->Identifier.Numeric);
						}
						if (iRes > 0)
						{
							uStatus = OpcUa_Good;
							OpcUa_Int32 iLen = strlen(cLocal);
							(*strNodeId) = (char*)malloc(iLen + 1);
							if (*strNodeId)
							{
								ZeroMemory((*strNodeId), iLen + 1);
								memcpy((*strNodeId), cLocal, iLen);
							}
							else
								uStatus = OpcUa_BadOutOfMemory;
						}
						free(cLocal);
					}
				}
				break;
				case 1: // string
				{
					// Attention la chaine devra être nettoyer par l'appelant (free)
					char* cLocal = (char*)malloc(1024 * sizeof(char));
					if (cLocal)
					{
						ZeroMemory(cLocal, 1024);
						if (OpcUa_SPrintfA(cLocal, "ns=%u;s=%s",
							pNodeId->NamespaceIndex,
							OpcUa_String_GetRawString(&(pNodeId->Identifier.String))) != -1)
						{
							uStatus = OpcUa_Good;
							OpcUa_Int32 iLen = strlen(cLocal);
							(*strNodeId) = (char*)malloc(iLen + 1);
							if (*strNodeId)
							{
								ZeroMemory((*strNodeId), iLen + 1);
								memcpy((*strNodeId), cLocal, iLen);
							}
							else
								uStatus = OpcUa_BadOutOfMemory;
						}
						free(cLocal);
					}
				}
				break;
				case 2: // guid
				{
					(*strNodeId) = (char*)malloc(8);
					if (*strNodeId)
					{
						ZeroMemory(*strNodeId, 8);
						memcpy(*strNodeId, (char*)pNodeId->Identifier.Guid->Data4, 8);
						uStatus = OpcUa_Good;
					}
					else
						uStatus = OpcUa_BadOutOfMemory;
				}
				break;
				case 3: // opaque (ByteString)
				{
					// Attention la chaine devra être nettoyer par l'appelant (free)
					if (pNodeId->Identifier.ByteString.Data)
					{
						OpcUa_Int32 iSize = strlen((char*)pNodeId->Identifier.ByteString.Data);
						(*strNodeId) = (char*)malloc(iSize);
						if (*strNodeId)
						{
							ZeroMemory(*strNodeId, iSize);
							memcpy(*strNodeId, (char*)pNodeId->Identifier.ByteString.Data, iSize);
							uStatus = OpcUa_Good;
						}
						else
							uStatus = OpcUa_BadOutOfMemory;
					}
				}
				break;
				default:
					break;
				}
			}
		}
	}
	return uStatus;
}
// Function name   : Utils::NodeId2String
// Description     : Converti un NodeId en chaine
//					 Attention la chaine devra être nettoyer par l'appelant (free)
// Return type     : OpcUa_StatusCode S_OK ou E_INVALIDARG
// Argument        : OpcUa_NodeId nodeId
// Argument        : wchar_t** strNodeId

OpcUa_StatusCode Utils::NodeId2String(OpcUa_NodeId nodeId, OpcUa_String* strNodeId)
{
	OpcUa_StatusCode uStatus = OpcUa_BadInternalError;
	if (strNodeId)
	{
		switch (nodeId.IdentifierType)
		{
		case OpcUa_IdentifierType_Numeric: // Numeric
		{
			char* buffer = (char*)malloc(20);
			if (buffer)
			{
				ZeroMemory(buffer, 20);

				if (sprintf(buffer, "ns=%u;i=%u", nodeId.NamespaceIndex, (unsigned int)nodeId.Identifier.Numeric) == 2)
				{
					OpcUa_String_AttachCopy(strNodeId, buffer);
					uStatus = OpcUa_Good;
				}
				else
				{
					if (printf(buffer, "i=%u", nodeId.NamespaceIndex, (unsigned int)nodeId.Identifier.Numeric) == 1)
					{

						OpcUa_String_AttachCopy(strNodeId, buffer);
						uStatus = OpcUa_Good;
					}
				}
				ZeroMemory(buffer, 20);
				free(buffer);
			}
			else
				uStatus = OpcUa_BadOutOfMemory;
		}
		break;
		case OpcUa_IdentifierType_String: // string
		{
			char* buffer = (char*)malloc(512);
			if (buffer)
			{
				ZeroMemory(buffer, 512);
				sprintf(buffer, "ns=%u;s=%s", nodeId.NamespaceIndex, OpcUa_String_GetRawString(&(nodeId.Identifier.String)));
				OpcUa_String_AttachCopy(strNodeId, buffer);
				uStatus = OpcUa_Good;
				ZeroMemory(buffer, 512);
				free(buffer);
			}
			else
				uStatus = OpcUa_BadOutOfMemory;
		}
		break;
		case OpcUa_IdentifierType_Guid: // guid
		{
			uStatus = OpcUa_BadNotSupported;
		}
		break;
		case OpcUa_IdentifierType_Opaque: // opaque (ByteString)
		{
			uStatus = OpcUa_BadNotSupported;
		}
		break;
		default:
			uStatus = OpcUa_BadInvalidArgument;
			break;
		}
	}
	else
		uStatus = OpcUa_BadInvalidArgument;
	return uStatus;
}


// Transforme un chaine en nodeId.
// pour que la transformation fonctionne la chaine doit avoir la forme ns=x;i=yyy ou ns=x;s=zzz
OpcUa_StatusCode Utils::String2NodeId(OpcUa_String strNodeId, OpcUa_NodeId* pNodeId)
{
	OpcUa_StatusCode uStatus = OpcUa_BadInternalError;
	if (!pNodeId)
		uStatus = OpcUa_BadInvalidArgument;
	else
	{
		if (OpcUa_String_StrLen(&strNodeId) > 0)
		{
			OpcUa_NodeId_Initialize(pNodeId);
			OpcUa_UInt32 iNs = 0, iId = 0;
			char* pBuff = OpcUa_String_GetRawString(&strNodeId);
			int iRes = sscanf(pBuff, "ns=%u;i=%u", (unsigned int*)&iNs, (unsigned int*)&iId);
			if (iRes == 2)
			{
				pNodeId->IdentifierType = OpcUa_IdentifierType_Numeric;
				pNodeId->NamespaceIndex = (OpcUa_UInt16)iNs;
				pNodeId->Identifier.Numeric = iId;
				uStatus = OpcUa_Good;
			}
			else
			{
				if (iRes == 1)
				{
					// probably another format
					OpcUa_CharA* strId = (OpcUa_CharA*)OpcUa_Alloc(256);
					if (strId)
					{
						char* ptr = strchr(pBuff, ';');
						char* subStr = strchr(ptr, '=');
						ZeroMemory(strId, 256);
						iRes = sscanf(pBuff, "ns=%u;s=%255s", (unsigned int*)&iNs, strId);
						if (iRes == 2)
						{
							pNodeId->IdentifierType = OpcUa_IdentifierType_String;
							pNodeId->NamespaceIndex = (OpcUa_UInt16)iNs;
							uStatus = OpcUa_String_AttachCopy(&(pNodeId->Identifier.String), ++subStr);
						}
						else
							uStatus = OpcUa_BadInvalidArgument;
						OpcUa_Free(strId);
					}
				}
				if (iRes == 0)
				{
					iRes = sscanf(pBuff, "i=%u", (unsigned int*)&iId);
					if (iRes==1)
					{
						pNodeId->IdentifierType = OpcUa_IdentifierType_Numeric;
						pNodeId->NamespaceIndex = (OpcUa_UInt16)0;
						pNodeId->Identifier.Numeric = iId;
						uStatus = OpcUa_Good;
					}
				}
			}
		}
		else
			uStatus = OpcUa_BadInvalidArgument;

	}
	return uStatus;
}

// Function name   : LookupNodeId
// Description     : 
// Return type     : OpcUa_StatusCode 
// Argument        : DWORD dwNodeId = NodeId  to lookup
// Argument        : wchar_t**  strText = Message related to the nodeId
// Argument        : DWORD* pdwFamily = Family of the nodeId 0 DataType, 1 method, 2 object 3 objectType 4 referenceType 5 variable 6 variableType

OpcUa_StatusCode Utils::LookupNodeId(OpcUa_NodeId NodeId, wchar_t**  strText, DWORD* pdwFamily)
{
	OpcUa_StatusCode uStatus = OpcUa_BadInvalidArgument;
	if (!pdwFamily)
		return uStatus;
	if (*strText)
	{
		int iSize=wcsnlen(*strText,256);
		ZeroMemory(*strText,iSize);
	}
	else
	{
		int iSize=sizeof(wchar_t)*1024;
		*strText=(wchar_t*)malloc(iSize);
		ZeroMemory(*strText,iSize);
	}
	// will now lookup the code
	// 1 in OpcUa_DataType_NodeIds
	int ii=0;
	switch (NodeId.IdentifierType)
	{
		case 0: // Numeric
			{
				int iNodeId=NodeId.Identifier.Numeric;
				while (OpcUa_DataType_NodeIds[ii].dwNodeId!=-1)
				{
					if (OpcUa_DataType_NodeIds[ii].dwNodeId==iNodeId)
					{
						OpcUa_String_AtoW(OpcUa_DataType_NodeIds[ii].szNodeIdText,(OpcUa_CharW**)strText);
						uStatus = OpcUa_Good;
						*pdwFamily=0;
						break;
					}
					ii++;
				}
				// 2 in OpcUa_Method_NodeIds
				if (uStatus != OpcUa_Good)
				{
					ii=0;
					while (OpcUa_Method_NodeIds[ii].dwNodeId!=-1)
					{
						if (OpcUa_Method_NodeIds[ii].dwNodeId==iNodeId)
						{
							OpcUa_String_AtoW(OpcUa_Method_NodeIds[ii].szNodeIdText,(OpcUa_CharW**)strText);
							uStatus = OpcUa_Good;
							*pdwFamily=1;
							break;
						}
						ii++;
					}
				}
				else 
					return uStatus;
				// 3 in OpcUa_Object_NodeIds
				if (uStatus != OpcUa_Good)
				{
					ii=0;
					while (OpcUa_Object_NodeIds[ii].dwNodeId!=-1)
					{
						if (OpcUa_Object_NodeIds[ii].dwNodeId==iNodeId)
						{
							OpcUa_String_AtoW(OpcUa_Object_NodeIds[ii].szNodeIdText,(OpcUa_CharW**)strText);							
							uStatus = OpcUa_Good;
							*pdwFamily=2;
							break;
						}
						ii++;
					}
				}
				else 
					return uStatus;
				// 4 in OpcUa_ObjectType_NodeIds
				if (uStatus != OpcUa_Good)
				{
					ii=0;
					while (OpcUa_ObjectType_NodeIds[ii].dwNodeId!=-1)
					{
						if (OpcUa_ObjectType_NodeIds[ii].dwNodeId==iNodeId)
						{
							OpcUa_String_AtoW(OpcUa_ObjectType_NodeIds[ii].szNodeIdText,(OpcUa_CharW**)strText);
							uStatus = OpcUa_Good;
							*pdwFamily=3;
							break;
						}
						ii++;
					}
				}
				else 
					return uStatus;
				// 5 in OpcUa_ReferenceType_NodeIds
				if (uStatus != OpcUa_Good)
				{
					ii=0;
					while (OpcUa_ReferenceType_NodeIds[ii].dwNodeId!=-1)
					{
						if (OpcUa_ReferenceType_NodeIds[ii].dwNodeId==iNodeId)
						{
							OpcUa_String_AtoW(OpcUa_ReferenceType_NodeIds[ii].szNodeIdText,(OpcUa_CharW**)strText);
							uStatus = OpcUa_Good;
							*pdwFamily=4;
							break;
						}
						ii++;
					}
				}
				else 
					return uStatus;
				// 6 in OpcUa_Variable_NodeIds
				if (uStatus != OpcUa_Good)
				{
					ii=0;
					while (OpcUa_Variable_NodeIds[ii].dwNodeId!=-1)
					{
						if (OpcUa_Variable_NodeIds[ii].dwNodeId==iNodeId)
						{							
							OpcUa_String_AtoW(OpcUa_Variable_NodeIds[ii].szNodeIdText,(OpcUa_CharW**)strText);
							uStatus = OpcUa_Good;
							*pdwFamily=5;
							break;
						}
						ii++;
					}
						
				}
				else 
					return uStatus;
				// 7 in OpcUa_VariableType_NodeIds
				if (uStatus != OpcUa_Good)
				{
					ii=0;
					while (OpcUa_VariableType_NodeIds[ii].dwNodeId!=-1)
					{
						if (OpcUa_VariableType_NodeIds[ii].dwNodeId==iNodeId)
						{
							OpcUa_String_AtoW(OpcUa_VariableType_NodeIds[ii].szNodeIdText,(OpcUa_CharW**)strText);
							uStatus = OpcUa_Good;
							*pdwFamily=6;
							break;
						}
						ii++;
					}
						
				}
				else 
					return uStatus;
			}
			break;
		case 1: // string
		case 2: // guid
		case 3: // opaque (ByteString)
		default:
			uStatus = OpcUa_Bad;
			break;
	}
	return uStatus;
}
OpcUa_Boolean Utils::IsNodeIdNull(OpcUa_NodeId aNodeId)
{
	OpcUa_Boolean bVal=OpcUa_False;
	if (aNodeId.IdentifierType==OpcUa_IdentifierType_Numeric)
	{
		if ( (aNodeId.Identifier.Numeric==0) && (aNodeId.NamespaceIndex==0))
			bVal=OpcUa_True;
	}
	else
	{
		if (aNodeId.IdentifierType==OpcUa_IdentifierType_String)
		{
			if (OpcUa_String_StrLen(&(aNodeId.Identifier.String))==0)
				bVal=OpcUa_True;
		}
		else
		{
			if (aNodeId.IdentifierType==OpcUa_IdentifierType_Guid)
			{
				if (!aNodeId.Identifier.Guid)
					bVal=OpcUa_True;
				else
				{
					if ((aNodeId.Identifier.Guid->Data1==0) 
						&& (aNodeId.Identifier.Guid->Data2==0) 
						&& (aNodeId.Identifier.Guid->Data3==0)
						&& (aNodeId.Identifier.Guid->Data4[0]==0) )
							bVal=OpcUa_True;
				}
			}
			else
			{
				if (aNodeId.IdentifierType==OpcUa_IdentifierType_Opaque)
				{
					if ( (aNodeId.Identifier.ByteString.Data==OpcUa_Null) || (aNodeId.Identifier.ByteString.Length<=0) )
						bVal=OpcUa_True;
				}
			}
		}
	}
	return bVal;
}
OpcUa_Boolean Utils::IsWritable(OpcUa_UInt32 AttributeId,OpcUa_UInt32 WriteMask)
{
	OpcUa_Boolean bWritable=false;
	if (AttributeId==OpcUa_Attributes_AccessLevel)
		return (OpcUa_Boolean)(WriteMask&0x1);
	if (AttributeId==OpcUa_Attributes_ArrayDimensions)
		return (OpcUa_Boolean)(WriteMask&0x2);
	if (AttributeId==OpcUa_Attributes_BrowseName)
		return (OpcUa_Boolean)(WriteMask&0x4);
	if (AttributeId==OpcUa_Attributes_ContainsNoLoops)
		return (OpcUa_Boolean)(WriteMask&0x8);
	if (AttributeId==OpcUa_Attributes_DataType)
		return (OpcUa_Boolean)(WriteMask&0x10);
	if (AttributeId==OpcUa_Attributes_Description)
		return (OpcUa_Boolean)(WriteMask&0x20);
	if (AttributeId==OpcUa_Attributes_DisplayName)
		return (OpcUa_Boolean)(WriteMask&0x40);
	if (AttributeId==OpcUa_Attributes_EventNotifier)
		return (OpcUa_Boolean)(WriteMask&0x80);
	if (AttributeId==OpcUa_Attributes_Executable)
		return (OpcUa_Boolean)(WriteMask&0x100);
	if (AttributeId==OpcUa_Attributes_Historizing)
		return (OpcUa_Boolean)(WriteMask&0x200);
	if (AttributeId==OpcUa_Attributes_InverseName)
		return (OpcUa_Boolean)(WriteMask&0x400);
	if (AttributeId==OpcUa_Attributes_IsAbstract)
		return (OpcUa_Boolean)(WriteMask&0x800);
	if (AttributeId==OpcUa_Attributes_MinimumSamplingInterval)
		return (OpcUa_Boolean)(WriteMask&0x1000);
	if (AttributeId==OpcUa_Attributes_NodeClass)
		return (OpcUa_Boolean)(WriteMask&0x2000);
	if (AttributeId==OpcUa_Attributes_NodeId)
		return (OpcUa_Boolean)(WriteMask&0x4000);
	if (AttributeId==OpcUa_Attributes_Symmetric)
		return (OpcUa_Boolean)(WriteMask&0x8000);
	if (AttributeId==OpcUa_Attributes_UserAccessLevel)
		return (OpcUa_Boolean)(WriteMask&0x10000);
	if (AttributeId==OpcUa_Attributes_UserExecutable)
		return (OpcUa_Boolean)(WriteMask&0x20000);
	if (AttributeId==OpcUa_Attributes_UserWriteMask)
		return (OpcUa_Boolean)(WriteMask&0x40000);
	if (AttributeId==OpcUa_Attributes_ValueRank)
		return (OpcUa_Boolean)(WriteMask&0x80000);
	if (AttributeId==OpcUa_Attributes_WriteMask)
		return (OpcUa_Boolean)(WriteMask&0x100000);
	// the attribute below is define in the Part3 table 3 but not in the AnsiC Stack
	//if (AttributeId==OpcUa_Attributes_ValueForVariableType)
	//	return (OpcUa_Bool)(WriteMask&0x200000);


	return bWritable;
}

// Function to check that 2 Datatypes are compliant
// TODO add all datatype
OpcUa_Boolean Utils::IsDataTypeCompliant(OpcUa_Byte uaType1,OpcUa_Byte uaType2)
{
	OpcUa_Boolean bResult = OpcUa_False;
	if (uaType1 == uaType2)
		bResult = OpcUa_True;
	else
	{
		if (uaType1 == OpcUaType_Boolean)
		{
			switch (uaType2)
			{
			case OpcUaType_Boolean: // Just to please the compiler
				bResult = OpcUa_False;
				break;
			default:
				bResult = OpcUa_False;
				break;
			}
		}
		else
		{
			if (uaType1 == OpcUaType_Int16)
			{
				switch (uaType2)
				{
				case OpcUaType_Int16: // Just to please the compiler
					bResult = OpcUa_False;
					break;
				default:
					bResult = OpcUa_False;
					break;
				}
			}
			else
			{
				if (uaType1 == OpcUaType_UInt16)
				{
					switch (uaType2)
					{
					case OpcUaType_Boolean:
					case OpcUaType_Byte:
					case OpcUaType_UInt32:
						bResult = OpcUa_True;
						break;
					default:
						bResult = OpcUa_False;
						break;
					}
				}
				else
				{
					// attention cas speciale de conversion de bytestring vers tableau de byte
					if ((uaType1 == OpcUaType_Byte) && (uaType2 == OpcUaType_ByteString))
						bResult = OpcUa_True;
					else
					{
						if (uaType1 == OpcUaType_SByte)
						{
							switch (uaType2)
							{
							case OpcUaType_Boolean:
							case OpcUaType_Byte:
							case OpcUaType_Int16:
								//case OpcUaType_Int32:
								//case OpcUaType_Int64:
								bResult = OpcUa_True;
								break;
							default:
								bResult = OpcUa_False;
								break;
							}
						}
					}
				}
			}
		}
	}
	return bResult;
}
OpcUa_StatusCode Utils::GetDataTypesize(OpcUa_Byte uaType,OpcUa_Int16* iSize)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	switch (uaType)
	{
	case OpcUaType_Boolean: //boolean
		*iSize=sizeof(OpcUa_Boolean);
		break;
	case OpcUaType_SByte:
		*iSize=sizeof(OpcUa_Boolean);
		break;
	case OpcUaType_Byte:
		*iSize=sizeof(OpcUa_SByte);
		break;
	case OpcUaType_Int16:
		*iSize=sizeof(OpcUa_Int16);
		break;
	case OpcUaType_UInt16:
		*iSize=sizeof(OpcUa_UInt16);
		break;
	case OpcUaType_Int32:
		*iSize=sizeof(OpcUa_Int32);
		break;
	case OpcUaType_UInt32:
		*iSize=sizeof(OpcUa_UInt32);
		break;
	case OpcUaType_Int64:
		*iSize=sizeof(OpcUa_Int64);
		break;
	case OpcUaType_UInt64:
		*iSize=sizeof(OpcUa_UInt64);
		break;
	case OpcUaType_Float:
		*iSize=sizeof(OpcUa_Float);
		break;
	case OpcUaType_Double:
		*iSize=sizeof(OpcUa_Double);
		break;
	case OpcUaType_String:
		*iSize=sizeof(OpcUa_String);
		break;
	case OpcUaType_DateTime:
		*iSize=sizeof(OpcUa_DateTime);
		break;
	case OpcUaType_Guid:
		*iSize=sizeof(OpcUa_Guid);
		break;
	case OpcUaType_ByteString:
		*iSize=sizeof(OpcUa_ByteString);
		break;
	case OpcUaType_XmlElement:
		*iSize=sizeof(OpcUa_XmlElement);
		break;
	case OpcUaType_NodeId:
		*iSize=sizeof(OpcUa_NodeId );
		break;
	case OpcUaType_ExpandedNodeId:
		*iSize=sizeof(OpcUa_ExpandedNodeId);
		break;
	case OpcUaType_StatusCode:
		*iSize=sizeof(OpcUa_StatusCode);
		break;
	case OpcUaType_QualifiedName:
		*iSize=sizeof(OpcUa_QualifiedName);
		break;
	case OpcUaType_LocalizedText:
		*iSize=sizeof(OpcUa_LocalizedText);
		break;
	case OpcUaType_ExtensionObject:
		*iSize=sizeof(OpcUa_ExtensionObject);
		break;
	case OpcUaType_DataValue:
		*iSize=sizeof(OpcUa_DataValue);
		break;
	case OpcUaType_Variant:
		*iSize=sizeof(OpcUa_Variant);
		break;
	case OpcUaType_DiagnosticInfo:
		*iSize=sizeof(OpcUa_DiagnosticInfo);
		break;
	default:
		uStatus=OpcUa_Bad;
		break;
	}
	return uStatus;
}
OpcUa_StatusCode Utils::GetNodeSize(OpcUa_NodeId aNodeId,OpcUa_UInt16* iSize)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_UInt16 uiTmpSize=0;
	if (iSize)
	{
		uiTmpSize=sizeof(OpcUa_UInt16); // IdentifierType
		uiTmpSize+=sizeof(OpcUa_UInt16); // NamespaceIndex
		switch (aNodeId.IdentifierType)
		{
			case OpcUa_IdentifierType_Numeric:
				uiTmpSize+=sizeof(OpcUa_UInt32);		
				break;
			case OpcUa_IdentifierType_String:
				uiTmpSize+=(OpcUa_UInt16)OpcUa_String_StrLen(&(aNodeId.Identifier.String));
				break;
			case OpcUa_IdentifierType_Guid:
				uiTmpSize+=sizeof(aNodeId.Identifier.Guid);
				break;
			case OpcUa_IdentifierType_Opaque:
				uiTmpSize+=(OpcUa_UInt16)aNodeId.Identifier.ByteString.Length;
				break;
			default:
				uStatus=OpcUa_BadNodeIdInvalid;
				break;
		}
		if (uStatus==OpcUa_Good)
			*iSize=uiTmpSize;
	}
	else
		uStatus=OpcUa_BadInvalidArgument;
	return uStatus;
}

