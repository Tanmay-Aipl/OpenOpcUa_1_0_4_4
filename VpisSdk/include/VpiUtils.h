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
namespace UASubSystem 
{	
	class VpiUtils
	{
		public:
			VpiUtils(void){;}
			~VpiUtils(void){;}

			/*============================================================================
			* Check if string is null.
			*===========================================================================*/
			static Vpi_Boolean StringIsNull(const Vpi_String* a_pString)
			{
				Vpi_Boolean bResult=Vpi_False;
				if(a_pString == Vpi_Null)
					bResult= Vpi_True;
				else
				{
					if(a_pString->strContent[0] == 0x00)
					{
						if(a_pString->strContent == Vpi_Null)
							bResult=Vpi_True;
					}
				}
				return bResult;
			}

			/*============================================================================
			 * Vpi_NodeId_IsNull
			 *===========================================================================*/
			static Vpi_Boolean NodeIdIsNull(Vpi_NodeId* a_pValue)
			{
				if (a_pValue == Vpi_Null)
				{
					return Vpi_True;
				}

				if (a_pValue->NamespaceIndex != 0)
				{
					return Vpi_False;
				}

				switch (a_pValue->IdentifierType)
				{
					case Vpi_IdentifierType_Numeric:
					{
						if (a_pValue->Identifier.Numeric != 0)
						{
							return Vpi_False;
						}

						break;
					}

					case Vpi_IdentifierType_String:
					{
						if (!StringIsNull(&(a_pValue->Identifier.String)) && strlen(a_pValue->Identifier.String.strContent) > 0)
						{
							return Vpi_False;
						}

						break;
					}
			        
					case Vpi_IdentifierType_Guid:
					{
						if (a_pValue->Identifier.Guid != Vpi_Null)
						{
							return Vpi_False;
						}

						break;
					}
			        
					case Vpi_IdentifierType_Opaque:
					{
						if (a_pValue->Identifier.ByteString.Length > 0)
						{
							return Vpi_False;
						}

						break;
					}
				}
			        
				return Vpi_True;
			}
			//============================================================================
			// comparaison de deux Vpi_NodeId
			//============================================================================
			static Vpi_Boolean IsEqual(const Vpi_NodeId* pOne, const Vpi_NodeId* pTwo)
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
				if (pOne->IdentifierType == Vpi_IdentifierType_Numeric)
				{
					return pOne->Identifier.Numeric == pTwo->Identifier.Numeric;
				}

				// check the string value.
				if (pOne->IdentifierType == Vpi_IdentifierType_String)
				{
					return  strncmp(pOne->Identifier.String.strContent, pTwo->Identifier.String.strContent, Vpi_False) == 0;
				}

				// check the guid value.
				if (pOne->IdentifierType == Vpi_IdentifierType_Guid)
				{
					return memcmp(pOne->Identifier.Guid, pTwo->Identifier.Guid, sizeof(Vpi_Guid)) == 0;
				}

				// check the opaque value.
				if (pOne->IdentifierType == Vpi_IdentifierType_Opaque)
				{
					if (pOne->Identifier.ByteString.Length != pTwo->Identifier.ByteString.Length)
					{
						return false;
					}

					return memcmp(pOne->Identifier.ByteString.Data, pTwo->Identifier.ByteString.Data, pOne->Identifier.ByteString.Length) == 0;
				}
					
				return false;
			}
			//============================================================================
			// VpiUtils::Copy(Vpi_ByteString)
			//============================================================================
			static Vpi_ByteString* Copy(Vpi_ByteString* pSrc)
			{ 
				Vpi_ByteString* pTarget=Vpi_Null;
				if (pSrc)
				{
					pTarget=(Vpi_ByteString*)malloc(sizeof(Vpi_ByteString));
					if (pSrc->Length>0)
					{
						pTarget->Data=(Vpi_Byte*)malloc(pSrc->Length);
						pTarget->Length= pSrc->Length;
						memcpy(pTarget->Data, 
							pSrc->Data,
							pSrc->Length);
					}
					else
					{
						free(pTarget);
						pTarget=Vpi_Null;
					}
				}
				return pTarget;
			}
			//============================================================================
			// VpiUtils::Copy(Vpi_Guid)
			//============================================================================
			static Vpi_Guid* Copy(Vpi_Guid* pSrc)
			{
				Vpi_Guid* pTarget=(Vpi_Guid*)malloc(sizeof(Vpi_Guid));
				ZeroMemory(pTarget,sizeof(Vpi_Guid));
				if (pSrc)
				{
					pTarget->Data1=pSrc->Data1;
					pTarget->Data2=pSrc->Data2;
					pTarget->Data3=pSrc->Data3;
					memcpy(pTarget->Data4,pSrc->Data4,8);
				}
				return pTarget;
			}
			//============================================================================
			// VpiUtils::Copy(Vpi_String)
			//============================================================================
			static Vpi_String* Copy(Vpi_String* pSrc)
			{
				Vpi_String* pTarget=Vpi_Null;
				if (pSrc)
				{
					if (strlen(pSrc->strContent)>0)
					{
						pTarget=(Vpi_String*)malloc(sizeof(Vpi_String));
						if (pTarget)
						{
							ZeroMemory(pTarget,sizeof(Vpi_String));
							pTarget->strContent=(Vpi_CharA*)malloc(strlen(pSrc->strContent));
							pTarget->uLength=strlen(pSrc->strContent);
							strncpy(pTarget->strContent,
									pSrc->strContent,
									strlen(pSrc->strContent) );
						}
					}
				}
				return pTarget;
			}
			//============================================================================
			// VpiUtils::Copy(Vpi_NodeId)
			//============================================================================
			static Vpi_NodeId* Copy(Vpi_NodeId* pSrc)
			{
				Vpi_NodeId* pTarget=(Vpi_NodeId*)malloc(sizeof(Vpi_NodeId));
				ZeroMemory(pTarget,sizeof(Vpi_NodeId));
				if (NodeIdIsNull(pSrc))
				{
					// will return a nullNodeId
					return pTarget;
				}
				
				pTarget->IdentifierType=pSrc->IdentifierType;
				pTarget->NamespaceIndex=pSrc->NamespaceIndex;
				// return the numeric value.
				if (pSrc->IdentifierType == Vpi_IdentifierType_Numeric)
				{
					pTarget->Identifier.Numeric=pSrc->Identifier.Numeric;
				}
				else
				{
					// copy the string value.
					if (pSrc->IdentifierType == Vpi_IdentifierType_String)
					{
						Vpi_String* pString=VpiUtils::Copy(&(pSrc->Identifier.String));
						if (pString)
							pTarget->Identifier.String=*pString;
					}
					else
					{
						// copy the guid value.
						if (pSrc->IdentifierType == Vpi_IdentifierType_Guid)
						{
							pTarget->Identifier.Guid=VpiUtils::Copy(pSrc->Identifier.Guid);
						}
						else
						{
							// copy the opaque value.
							if (pSrc->IdentifierType == Vpi_IdentifierType_Opaque)
							{
								ZeroMemory(&(pTarget->Identifier.ByteString),sizeof(Vpi_ByteString));
								pTarget->Identifier.ByteString.Length = pSrc->Identifier.ByteString.Length;
								pTarget->Identifier.ByteString.Data = (Vpi_Byte*)malloc(pSrc->Identifier.ByteString.Length);
								ZeroMemory(pTarget->Identifier.ByteString.Data,pSrc->Identifier.ByteString.Length);
								memcpy(
									pTarget->Identifier.ByteString.Data, 
									pSrc->Identifier.ByteString.Data, 
									pSrc->Identifier.ByteString.Length);
							}
						}
					}
				}
				// other id types not supported yet.
				return pTarget;
			}
			static Vpi_Variant* Copy(Vpi_Variant* pSrc)
			{
				Vpi_Variant* pTarget=(Vpi_Variant*)malloc(sizeof(Vpi_Variant));
				ZeroMemory(pTarget,sizeof(Vpi_Variant));
				// ArrayType
				pTarget->ArrayType=pSrc->ArrayType;
				// Datatype
				pTarget->Datatype=pSrc->Datatype;
				// Reserved
				pTarget->Reserved=pSrc->Reserved;
				// Value
				if (pTarget->ArrayType==Vpi_VariantArrayType_Scalar)
				{
					// Copie Scalar
					switch (pTarget->Datatype)
					{
					case VpiType_Boolean:
						pTarget->Value.Boolean=pSrc->Value.Boolean;
						break;
					case VpiType_Byte:
						pTarget->Value.Byte=pSrc->Value.Byte;
						break;
					case VpiType_ByteString:
						{
							int iLen=pSrc->Value.ByteString.Length;
							pTarget->Value.ByteString.Data=(Vpi_Byte*)malloc(iLen);
							memcpy(pTarget->Value.ByteString.Data,pSrc->Value.ByteString.Data,iLen);
						}
						break;
					case VpiType_DataValue:
						break;
					case VpiType_DateTime:
						pTarget->Value.DateTime=pSrc->Value.DateTime;
						break;
					case VpiType_DiagnosticInfo:			
						break;
					case VpiType_Double:
						pTarget->Value.Double=pSrc->Value.Double;
						break;
					case VpiType_ExpandedNodeId:
						break;
					case VpiType_ExtensionObject:
						//pTarget->Value.ExtensionObject=VpiUtils::Copy(pSrc->Value.ExtensionObject);
						break;
					case VpiType_Float:
						pTarget->Value.Float=pSrc->Value.Float;
						break;
					case VpiType_Guid:
						break;
					case VpiType_Int16:
						pTarget->Value.Int16=pSrc->Value.Int16;
						break;
					case VpiType_Int32:
						pTarget->Value.Int32=pSrc->Value.Int32;
						break;
					case VpiType_Int64:
						pTarget->Value.Int64=pSrc->Value.Int64;
						break;
					case VpiType_LocalizedText:
						{
							//pTarget->Value.LocalizedText=VpiUtils::Copy(pSrc->Value.LocalizedText);
						}
						break;
					case VpiType_NodeId:
						{
							pTarget->Value.NodeId=VpiUtils::Copy(pSrc->Value.NodeId);
						}
						break;
					case VpiType_Null:
						break;
					case VpiType_QualifiedName:
						//pTarget->Value.QualifiedName=VpiUtils::Copy(pSrc->Value.QualifiedName);
						break;
					case VpiType_SByte:
						pTarget->Value.SByte=pSrc->Value.SByte;
						break;
					case VpiType_StatusCode:
						pTarget->Value.StatusCode=pSrc->Value.StatusCode;
						break;
					case VpiType_String:
						{
							Vpi_String* pString=VpiUtils::Copy(&(pSrc->Value.String));
							if (pString)
								pTarget->Value.String=*pString;
						}
						break;
					case VpiType_UInt16:
						pTarget->Value.UInt16=pSrc->Value.UInt16;
						break;
					case VpiType_UInt32:
						pTarget->Value.UInt32=pSrc->Value.UInt32;
						break;
					case VpiType_UInt64:
						pTarget->Value.UInt64=pSrc->Value.UInt64;
						break;
					case VpiType_Variant:
						break;
					case VpiType_XmlElement:
						break;
					default:
						break;
					}
				}
				else
				{
					if (pTarget->ArrayType==Vpi_VariantArrayType_Array)
					{
						// Copie Array
						pTarget->Value.Array.Length=pSrc->Value.Array.Length;
						int iLen=pSrc->Value.Array.Length;
						for (int ii=0;ii<iLen;ii++)
						{
							//pTarget->Value.Array.Value.[ii]=pSrc->Value.Array.Value.[ii];
							switch (pTarget->Datatype)
							{
							case VpiType_Boolean:
								pTarget->Value.Array.Value.BooleanArray[ii]=pSrc->Value.Array.Value.BooleanArray[ii];
								break;
							case VpiType_Byte:
								pTarget->Value.Array.Value.ByteArray[ii]=pSrc->Value.Array.Value.ByteArray[ii];
								break;
							case VpiType_ByteString:
								{
									if (pTarget->Value.Array.Value.ByteStringArray==Vpi_Null)
										pTarget->Value.Array.Value.ByteStringArray=(Vpi_ByteString*)malloc(sizeof(Vpi_ByteString)*(pSrc->Value.Array.Length));
									Vpi_ByteString* pByteString=VpiUtils::Copy(&(pSrc->Value.Array.Value.ByteStringArray[ii]));
									if (pByteString)
										pTarget->Value.Array.Value.ByteStringArray[ii]=*pByteString;
								}
								break;
							case VpiType_DataValue:
								break;
							case VpiType_DateTime:
								pTarget->Value.Array.Value.DateTimeArray[ii]=pSrc->Value.Array.Value.DateTimeArray[ii];
								break;
							case VpiType_DiagnosticInfo:			
								break;
							case VpiType_Double:
								pTarget->Value.Array.Value.DoubleArray[ii]=pSrc->Value.Array.Value.DoubleArray[ii];
								break;
							case VpiType_ExpandedNodeId:
								break;
							case VpiType_ExtensionObject:
								break;
							case VpiType_Float:
								pTarget->Value.Array.Value.FloatArray[ii]=pSrc->Value.Array.Value.FloatArray[ii];
								break;
							case VpiType_Guid:
								break;
							case VpiType_Int16:
								pTarget->Value.Array.Value.Int16Array[ii]=pSrc->Value.Array.Value.Int16Array[ii];
								break;
							case VpiType_Int32:
								pTarget->Value.Array.Value.Int32Array[ii]=pSrc->Value.Array.Value.Int32Array[ii];
								break;
							case VpiType_Int64:
								pTarget->Value.Array.Value.Int64Array[ii]=pSrc->Value.Array.Value.Int64Array[ii];
								break;
							case VpiType_LocalizedText:
								{
									//Vpi_LocalizedText* pLocalText=VpiUtils::Copy(&(pSrc->Value.Array.Value.LocalizedTextArray[ii]));
									//if (pLocalText)
									//	pTarget->Value.Array.Value.LocalizedTextArray[ii]=*pLocalText;
								}
								break;
							case VpiType_NodeId:
								{
									Vpi_NodeId* pNodeId=VpiUtils::Copy(&(pSrc->Value.Array.Value.NodeIdArray[ii]));
									if (pNodeId)
										pTarget->Value.Array.Value.NodeIdArray[ii]=*pNodeId;
								}
								break;
							case VpiType_Null:
								break;
							case VpiType_QualifiedName:
								{
								//Vpi_QualifiedName* pQualifiedName=VpiUtils::Copy(&(pSrc->Value.Array.Value.QualifiedNameArray[ii]));
								//if (pQualifiedName)
								//	pTarget->Value.Array.Value.QualifiedNameArray[ii]=*pQualifiedName;
								}
								break;
							case VpiType_SByte:
								pTarget->Value.Array.Value.SByteArray[ii]=pSrc->Value.Array.Value.SByteArray[ii];
								break;
							case VpiType_StatusCode:
								pTarget->Value.Array.Value.StatusCodeArray[ii]=pSrc->Value.Array.Value.StatusCodeArray[ii];
								break;
							case VpiType_String:
								{
									Vpi_String* pString=VpiUtils::Copy(&(pSrc->Value.Array.Value.StringArray[ii]));
									if (pString)
										pTarget->Value.Array.Value.StringArray[ii]=*pString;
								}
								break;
							case VpiType_UInt16:
								pTarget->Value.Array.Value.UInt16Array[ii]=pSrc->Value.Array.Value.UInt16Array[ii];
								break;
							case VpiType_UInt32:
								pTarget->Value.Array.Value.UInt32Array[ii]=pSrc->Value.Array.Value.UInt32Array[ii];
								break;
							case VpiType_UInt64:
								pTarget->Value.Array.Value.UInt64Array[ii]=pSrc->Value.Array.Value.UInt64Array[ii];
								break;
							case VpiType_Variant:
								break;
							case VpiType_XmlElement:
								break;
							default:
								break;
							}
						}
					}
					else
					{
						if (pTarget->ArrayType==Vpi_VariantArrayType_Matrix)
						{
							// Copie Matrix. Not supported
							;
						}
						else
						{
							// error VariantType unknown
							//Vpi_Trace(OPCUA_TRACE_LEVEL_ERROR,"Utils::Copy OpocUa_Variant>error VariantType unknown %u\n",pTarget->ArrayType);
						}
					}
				}
				return pTarget;
			}
	};
}