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

#pragma once

namespace OpenOpcUa
{
	namespace UASharedLib
	{

	// Defines utility functions. 
		class SHAREDLIB_EXPORT Utils
		{
		public:
			Utils(void);
			~Utils(void);

			// Converts a std::string to OpcUa_StringA. Returned value must freed with OpcUa_Free.
			//static OpcUa_StringA StrDup(std::string src);

			// Duplicates a OpcUa_ByteString.
			static OpcUa_ByteString StrDup(const OpcUa_ByteString* pSrc);
			// Copie ByteString
			static OpcUa_ByteString* Copy(OpcUa_ByteString* pSrc);
			// copies a OpcUa_ExpandedNodeId
			//static OpcUa_ExpandedNodeId  Copy(const OpcUa_ExpandedNodeId * pSrc);
			// Copie OpcUa_LiteralOperand
			static OpcUa_LiteralOperand* Copy(OpcUa_LiteralOperand* pSrc);
			// Copie OpcUa_EncodeableType
			static OpcUa_EncodeableType* Copy(OpcUa_EncodeableType* pSrc);
			// EndpointDescription
			static OpcUa_EndpointDescription* Copy(OpcUa_EndpointDescription* pSrc);
			// copies ExtensionObject
			//static OpcUa_ExtensionObject* Copy(OpcUa_ExtensionObject* pSrc);
			// Copies a NodeId
			static OpcUa_NodeId* Copy(OpcUa_NodeId* pSrc);
			// Copie OpcUa_BrowseResult
			static OpcUa_BrowseResult* Copy(OpcUa_BrowseResult* pSrc);
			// Copie OpcUa_ReferenceDescription
			static OpcUa_ReferenceDescription* Copy(OpcUa_ReferenceDescription* pSrc);
			// copie OpcUa_BuildInfo
			static OpcUa_BuildInfo* Copy(OpcUa_BuildInfo* pBuildInfo);
			// copie OpcUa_ServerStatusDataType
			static OpcUa_ServerStatusDataType* Copy(OpcUa_ServerStatusDataType* pSrcServerStatus);
			// Copie StatusResult
			static OpcUa_StatusResult* Copy(OpcUa_StatusResult* pSrc);
			// Copie DiagnosticInfo
			static OpcUa_DiagnosticInfo* Copy(OpcUa_DiagnosticInfo* pSrc);
			// Copie OpcUa_Argument
			static OpcUa_Argument* Copy(OpcUa_Argument* pSrcArgument);
			// Copie SessionSecurityDiagnosticsDataType
			static OpcUa_SessionSecurityDiagnosticsDataType* Copy(OpcUa_SessionSecurityDiagnosticsDataType* pSrc);
			// Copie SessionDiagnosticsDataType
			static OpcUa_SessionDiagnosticsDataType* Copy(OpcUa_SessionDiagnosticsDataType* pSrc);
			// Copie ApplicationDescription
			static OpcUa_ApplicationDescription* Copy(OpcUa_ApplicationDescription* pSrc);
			// Copie TimeZoneDataType
			static OpcUa_TimeZoneDataType* Copy(OpcUa_TimeZoneDataType* pSrc);
			// UserTokenPolicy
			static OpcUa_UserTokenPolicy* Copy(OpcUa_UserTokenPolicy* pSrc);
			// Copie UserIdentityToken 
			static OpcUa_UserIdentityToken* Copy(OpcUa_UserIdentityToken* pSrc);
			// Copie OpcUa_DataValue
			static OpcUa_DataValue* Copy(OpcUa_DataValue* pSrc);
			// Copie OpcUa_DataChangeFilter
			static OpcUa_DataChangeFilter* Copy(OpcUa_DataChangeFilter* pSrc);
			// Copie Guid
			static OpcUa_Guid* Copy(OpcUa_Guid* pSrc);
			// Copie HistoryEventFieldList
			static OpcUa_HistoryEventFieldList* Copy(OpcUa_HistoryEventFieldList* pSrc);
			// Copie Variant
			//static OpcUa_Variant* Copy(OpcUa_Variant* pSrc);

			// Copie ExpandedNodeId
			//static OpcUa_ExpandedNodeId Copy(OpcUa_ExpandedNodeId* pSrc);
			// Copie ContentFilterResult
			static OpcUa_ContentFilterResult* Copy(OpcUa_ContentFilterResult* pSrc);
			// ContentFilterElementResult
			static OpcUa_ContentFilterElementResult* Copy(OpcUa_ContentFilterElementResult* pSrc);
			// Copie EventFilter
			//static OpcUa_EventFilter* Copy(OpcUa_EventFilter* pSrc);
			// Copie SimpleAttributeOperand
			static OpcUa_SimpleAttributeOperand* Copy(OpcUa_SimpleAttributeOperand* pSrc);
			// Copie ContentFilter
			//static OpcUa_ContentFilter* Copy(OpcUa_ContentFilter* pSrc);
			// Copie ContentFilterElement
			//static OpcUa_ContentFilterElement* Copy(OpcUa_ContentFilterElement* pSrc);
			// Copie EventFilterResult
			static OpcUa_EventFilterResult* Copy(OpcUa_EventFilterResult* pSrc);
			// Copie AddReferencesItem
			static OpcUa_AddReferencesItem* Copy(OpcUa_AddReferencesItem* pSrc);
			// Copie DeleteReferencesItem
			static OpcUa_DeleteReferencesItem* Copy(OpcUa_DeleteReferencesItem* pSrc);
			// Copie AddNodesItem
			//static OpcUa_AddNodesItem* Copy(OpcUa_AddNodesItem* pSrc);
			// Copie DateTime
			static OpcUa_DateTime* Copy(OpcUa_DateTime* pSrc);
			// Copie DeleteNodesItem
			static OpcUa_DeleteNodesItem* Copy(OpcUa_DeleteNodesItem* pSrc);
			// Copie LocalizedText
			static OpcUa_LocalizedText* Copy(OpcUa_LocalizedText* pSrc);
			// Copie QualifiedName
			static OpcUa_QualifiedName* Copy(OpcUa_QualifiedName* pSrc);
			// copie OpcUa_ServiceCounterDataType* Utils::Copy(OpcUa_ServiceCounterDataType* pSrc)
			static OpcUa_ServiceCounterDataType* Copy(OpcUa_ServiceCounterDataType* pSrc);
			// copie SamplingIntervalDiagnosticsDataType
			static OpcUa_SamplingIntervalDiagnosticsDataType* Copy(OpcUa_SamplingIntervalDiagnosticsDataType* pSrc);
			// copie SoftwareCertificate
			static OpcUa_SoftwareCertificate* Copy(OpcUa_SoftwareCertificate* pSrc);
			// copie SignedSoftwareCertificate
			static OpcUa_SignedSoftwareCertificate* Copy(OpcUa_SignedSoftwareCertificate* pSrc);
			// copie RedundantServerDataType
			static OpcUa_RedundantServerDataType* Copy(OpcUa_RedundantServerDataType* pSrc);
			// copie ServerDiagnosticsSummaryDataType
			static OpcUa_ServerDiagnosticsSummaryDataType* Copy(OpcUa_ServerDiagnosticsSummaryDataType* pSrc);
			// copie SupportedProfile
			static OpcUa_SupportedProfile* Copy(OpcUa_SupportedProfile* pSrc);
			// copie ModelChangeStructureDataType
			static OpcUa_ModelChangeStructureDataType* Copy(OpcUa_ModelChangeStructureDataType* pSrc);
			// Copie SemanticChangeStructureDataType
			static OpcUa_SemanticChangeStructureDataType* Copy(OpcUa_SemanticChangeStructureDataType* pSrc);
			// copie SubscriptionDiagnosticsDataType
			static OpcUa_SubscriptionDiagnosticsDataType* Copy(OpcUa_SubscriptionDiagnosticsDataType* pSrc);
			// Copie String
			static OpcUa_String* Copy(OpcUa_String* pSrc);
			// copie Range
			static OpcUa_Range* Copy(OpcUa_Range* pSrc);
			// copie EUInformation
			static OpcUa_EUInformation* Copy(OpcUa_EUInformation* pSrc);
			// Copie EnumValueType
			static OpcUa_EnumValueType* Copy(OpcUa_EnumValueType* pSrc);
			// copie Annotation
			static OpcUa_Annotation* Copy(OpcUa_Annotation* pSrc);
			// copie ProgramDiagnosticDataType
			static OpcUa_ProgramDiagnosticDataType* Copy(OpcUa_ProgramDiagnosticDataType* pSrc);
			// Compare two OpcUa_Variant
			static OpcUa_Boolean IsEqual(OpcUa_Variant* pOne,OpcUa_Variant * pTwo);
			// compare two OpcUa_ByteString
			static OpcUa_Boolean IsEqual(OpcUa_ByteString* pOne,OpcUa_ByteString * pTwo);
			// compare two OpcUa_string
			static OpcUa_Boolean IsEqual(OpcUa_String* pOne,OpcUa_String * pTwo);
			// compares two OpcUa_QualifiedName
			static OpcUa_Boolean IsEqual(OpcUa_QualifiedName* pOne,OpcUa_QualifiedName * pTwo);
			//  Compares two NodeIds.
			static OpcUa_Boolean IsEqual(const OpcUa_NodeId* pOne, const OpcUa_NodeId* pTwo);
			//  Compares two ExpandedNodeIds.
			static OpcUa_Boolean IsEqual(const OpcUa_ExpandedNodeId* pOne, const OpcUa_ExpandedNodeId* pTwo);
			// compare two OpcUa_BuildInfo
			static OpcUa_Boolean IsEqual(OpcUa_BuildInfo* pOneEx,OpcUa_BuildInfo * pTwoEx);
			// compare two OpcUa_ServerStatusDataType
			static OpcUa_Boolean IsEqual(OpcUa_ServerStatusDataType* pOneEx,OpcUa_ServerStatusDataType * pTwoEx);
			// Check if a NodeId is Null or not. A nodeid is null if ns=0;i=0 or contains an string of len 0
			static OpcUa_Boolean IsNodeIdNull(OpcUa_NodeId aNodeId); 
			// verifie si un buffer de OpcUa_CharA est rempli d'espace 0x20 sur uiLen
			static OpcUa_Boolean IsBufferEmpty(OpcUa_CharA* buffer, OpcUa_UInt32 uiLen);
			// based on the table 3 of the Part3 §5.2.7 this method determine if a attribute is writable or not
			static OpcUa_Boolean IsWritable(OpcUa_UInt32 AttributeId, OpcUa_UInt32 WriteMask);
			// Converts a std::string to OpcUa_String. Returned value must freed with OpcUa_String_Clear.
			static OpcUa_String* Copy(std::string src);

			// Converts a std::vector to OpcUa_ByteString. Returned value must freed with OpcUa_ByteString_Clear.
			static OpcUa_ByteString Copy(std::vector<unsigned char> src);

			// Converts a OpcUa_String to a std::string.
			//static std::string Copy(OpcUa_String* pSrc);

			// Converts a OpcUa_ByteString to a std::vector.
			static std::vector<unsigned char> Copy(const OpcUa_ByteString* pSrc);
			
			// Converts a OpcUa_StatusCode to a std::string.
			static std::string StatusToString(OpcUa_StatusCode uStatus);

			// convert a OpcUa_DateTime tto a std::string
			static OpcUa_StatusCode OpcUaDateTimeToString(OpcUa_DateTime dt, OpcUa_String** strTime);
			// Methode pour verifier que deux type sont compatible
			static OpcUa_Boolean IsDataTypeCompliant(OpcUa_Byte uaType1,OpcUa_Byte uaType2);

			// UA Variant to String
			static OpcUa_StatusCode OpcUaVariantToString(const OpcUa_Variant& Var, OpcUa_String** strValue);
			static OpcUa_StatusCode OpcUaVariantArrayToString(const OpcUa_Variant& Var, OpcUa_String** strValue);
			static OpcUa_StatusCode OpcUaExtentionObjectToString(OpcUa_ExtensionObject& extObject, OpcUa_String** strValue);
			// convert a nodeid to a string (wchar_t)
			static OpcUa_StatusCode NodeId2String(OpcUa_NodeId* pNodeId, char** strNodeId);
			static OpcUa_StatusCode NodeId2String(OpcUa_NodeId nodeId, OpcUa_String* strNodeId);
			// convert a string to a NodeId
			static OpcUa_StatusCode String2NodeId(OpcUa_String strNodeId, OpcUa_NodeId* pNodeId);
			// Convert a class num to a string 
			static OpcUa_StatusCode NodeClassToString(OpcUa_Int32 iVal, std::string* NodeClass);
			//
			static OpcUa_StatusCode StringToNodeClass(std::string strNodeClass, OpcUa_NodeClass* iNodeClass);
			// Lookup nodeId for some well known nodeId
			static OpcUa_StatusCode LookupNodeId(OpcUa_NodeId NodeId, wchar_t**  strText, DWORD* pdwFamily);
			// Retrieve the size of a NodeId
			static OpcUa_StatusCode GetNodeSize(OpcUa_NodeId aNodeId,OpcUa_UInt16* iSize);
			// Retrieve the size of the DataType
			static OpcUa_StatusCode GetDataTypesize(OpcUa_Byte uaType,OpcUa_Int16* iSize);
		};

		// Clears an array of any type with xType_Clear function defined.
		#define Utils_ClearArray(xArray,xCount,xType) \
		if (xArray != OpcUa_Null) \
		{ \
			for (OpcUa_Int32 ii = 0; ii < xCount; ii++) \
			{ \
				xType##_Clear(&(xArray[ii])); \
			} \
		 \
			OpcUa_Free(xArray); \
			xArray = 0; \
		}

		#define UTILS_DEFAULT_TIMEOUT 300000

	} // namespace UASharedLib
}