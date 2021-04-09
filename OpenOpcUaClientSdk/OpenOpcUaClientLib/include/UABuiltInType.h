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
#ifndef _UA_BUILTINTYPE_
#define _UA_BUILTINTYPE_
typedef struct _UA_BUILTINTYPE_STRUCTURE_
{
	const OpcUa_CharA*	szBuilInType;
	OpcUa_Int32		iBuilInType;
	const OpcUa_CharA*	szBuilInTypeText;

} UA_BUILTINTYPE_STRUCTURE;
static UA_BUILTINTYPE_STRUCTURE OpcUa_BuiltInTypes[]=
{
	{"OpcUaType_Null",OpcUaType_Null,"Null"}, // 0
	{"OpcUaType_Boolean",OpcUaType_Boolean,"Boolean"},
	{"OpcUaType_SByte",OpcUaType_SByte,"SByte"},
	{"OpcUaType_Byte",OpcUaType_Byte,"Byte"},
	{"OpcUaType_Int16",OpcUaType_Int16,"Int16"},
	{"OpcUaType_UInt16",OpcUaType_UInt16,"UInt16"},
	{"OpcUaType_Int32",OpcUaType_Int32,"Int32"},
	{"OpcUaType_UInt32",OpcUaType_UInt32,"UInt32"},
	{"OpcUaType_Int64",OpcUaType_Int64,"Int64"},
	{"OpcUaType_UInt64",OpcUaType_UInt64,"UInt64"},
	{"OpcUaType_Float",OpcUaType_Float,"Float"},
	{"OpcUaType_Double",OpcUaType_Double,"Double"},
	{"OpcUaType_String",OpcUaType_String,"String"},
	{"OpcUaType_DateTime",OpcUaType_DateTime,"DateTime"},
	{"OpcUaType_Guid",OpcUaType_Guid,"Guid"},
	{"OpcUaType_ByteString",OpcUaType_ByteString,"ByteString"},
	{"OpcUaType_XmlElement",OpcUaType_XmlElement,"XmlElement"},
	{"OpcUaType_NodeId",OpcUaType_NodeId,"NodeId"},
	{"OpcUaType_ExpandedNodeId",OpcUaType_ExpandedNodeId,"ExpandedNodeId"},
	{"OpcUaType_StatusCode",OpcUaType_StatusCode,"StatusCode"},
	{"OpcUaType_QualifiedName",OpcUaType_QualifiedName,"QualifiedName"},
	{"OpcUaType_LocalizedText",OpcUaType_LocalizedText,"LocalizedText"},
	{"OpcUaType_ExtensionObject",OpcUaType_ExtensionObject,"ExtensionObject"},
	{"OpcUaType_DataValue",OpcUaType_DataValue,"DataValue"},
	{"OpcUaType_Variant",OpcUaType_Variant,"Variant"},
	{"OpcUaType_DiagnosticInfo",OpcUaType_DiagnosticInfo,"DiagnosticInfo"}, // 25
};
#endif