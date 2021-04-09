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
#ifndef _ATTRIBUTE_STRUCTURE
#define _ATTRIBUTE_STRUCTURE
typedef struct _UA_ATTRIBUTE_STRUCTURE_
{
	const OpcUa_CharA*		szAttribute;
	OpcUa_Int32			iAttributeId;
	const OpcUa_CharA*		szAttributeText;

} UA_ATTRIBUTE_STRUCTURE;

// The attribute collection
static UA_ATTRIBUTE_STRUCTURE OpcUa_Attributes[]=
{
	{"NodeId",OpcUa_Attributes_NodeId,"The canonical identifier for the node."},
	{"NodeClass",OpcUa_Attributes_NodeClass,"The class of the node."},
	{"BrowseName",OpcUa_Attributes_BrowseName,"A non-localized, human readable name for the node."},
	{"DisplayName",OpcUa_Attributes_DisplayName,"A localized, human readable name for the node."},
	{"Description",OpcUa_Attributes_Description,"A localized description for the node."},
	{"WriteMask",OpcUa_Attributes_WriteMask,"Indicates which attributes are writeable."},
	{"UserWriteMask",OpcUa_Attributes_UserWriteMask,"Indicates which attributes are writeable by the current user."},
	{"IsAbstract",OpcUa_Attributes_IsAbstract,"Indicates that a type node may not be instantiated."},
	{"Symmetric",OpcUa_Attributes_Symmetric,"Indicates that forward and inverse references have the same meaning."},
	{"InverseName",OpcUa_Attributes_InverseName,"The browse name for an inverse reference."},
	{"ContainsNoLoops",OpcUa_Attributes_ContainsNoLoops,"Indicates that following forward references within a view will not cause a loop."},
	{"EventNotifier",OpcUa_Attributes_EventNotifier,"Indicates that the node can be used to subscribe to events."},
	{"Value",OpcUa_Attributes_Value,"The value of a variable"},
	{"DataType",OpcUa_Attributes_DataType,"The node id of the data type for the variable value."},
	{"ValueRank",OpcUa_Attributes_ValueRank,"The number of dimensions in the value."},
	{"ArrayDimensions",OpcUa_Attributes_ArrayDimensions,"The length for each dimension of an array value."},
	{"AccessLevel",OpcUa_Attributes_AccessLevel,"How a variable may be accessed."},
	{"UserAccessLevel",OpcUa_Attributes_UserAccessLevel,"How a variable may be accessed after taking the user's access rights into account."},
	{"MinimumSamplingInterval",OpcUa_Attributes_MinimumSamplingInterval,"Specifies (in milliseconds) how fast the server can reasonably sample the value for changes."},
	{"Historizing",OpcUa_Attributes_Historizing,"Specifies whether the server is actively collecting historical data for the variable."},
	{"Executable",OpcUa_Attributes_Executable,"Whether the method can be called."},
	{"UserExecutable",OpcUa_Attributes_UserExecutable,"Whether the method can be called by the current user."},
};

#endif