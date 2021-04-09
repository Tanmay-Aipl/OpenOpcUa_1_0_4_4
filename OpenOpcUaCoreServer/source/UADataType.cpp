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
#include "Definition.h"
#include "UADataType.h"
using namespace OpenOpcUa;
using namespace UAAddressSpace;
CUADataType::CUADataType(void)
{
	m_bAbstract = OpcUa_False;
	m_pDefinition=NULL;
	m_iSize=0;
	OpcUa_NodeId_Initialize(&m_ParentType);
	m_AncestorType = 0;
}
CUADataType::CUADataType(OpcUa_NodeClass aNodeClass, const char **atts) :CUABase(aNodeClass, atts)
{
	m_bAbstract = OpcUa_False;
	m_pDefinition = OpcUa_Null;
	OpcUa_NodeId_Initialize(&m_ParentType);
	m_AncestorType = 0;
	m_iSize = 0;
	int ii = 0;
	while (atts[ii])
	{
		if (OpcUa_StrCmpA(atts[ii], "IsAbstract") == 0)
		{
			if (OpcUa_StrCmpA(atts[ii + 1], "true") == 0)
				m_bAbstract = true;
			else
			{
				if (OpcUa_StrCmpA(atts[ii + 1], "false") == 0)
					m_bAbstract = false;
				else
					m_bAbstract = (OpcUa_Boolean)atoi(atts[ii + 1]);
			}
		}
		ii += 2;
	}
}
CUADataType::~CUADataType(void)
{
	if (m_pDefinition)
		delete m_pDefinition;
	OpcUa_NodeId_Clear(&m_ParentType);
}

// Function name   : CUADataType::UpdateParentType
// Description     : This method is looking for the parent of the current UADataType
//					 The parentType is detected by the reference HasSubtype between child and parent 
// Return type     : OpcUa_StatusCode OpcUa_BadDataTypeIdUnknown if not fouund and OpcUa_Good otherwise

/// <summary>
/// Updates the type of the parent.
/// Search for the HasSubtype reference in the declared reference of the UADataType
/// Once found update the m_ParentType nodeId with the reference Target
/// </summary>
/// <returns>OpcUa_StatusCode OpcUa_Good if it succeed. OpcUa_BadDataTypeIdUnknown if it failed</returns>
OpcUa_StatusCode CUADataType::UpdateParentType()
{
	OpcUa_StatusCode uStatus=OpcUa_BadDataTypeIdUnknown;
	OpcUa_UInt32 iSize=m_pReferences->size();
	for (OpcUa_UInt32 ii=0;ii<iSize;ii++)
	{
		CUAReference* pReference=m_pReferences->at(ii);
		OpcUa_NodeId aSubTypeNodeId;
		aSubTypeNodeId.NamespaceIndex=0;
		aSubTypeNodeId.IdentifierType=OpcUa_IdentifierType_Numeric;
		aSubTypeNodeId.Identifier.Numeric=45; // HasSubtype
		OpcUa_NodeId aReferenceNodeId=pReference->GetReferenceTypeId();
		// Is this UADataType contains an Inverse HasSubtype
		if ((Utils::IsEqual(&aReferenceNodeId,&aSubTypeNodeId )) && (pReference->IsInverse()) )
		{
			uStatus=OpcUa_Good;
			OpcUa_ExpandedNodeId aTargetExpandedNodeId= pReference->GetTargetId();
			OpcUa_NodeId_CopyTo(&(aTargetExpandedNodeId.NodeId),&m_ParentType);
			break;
		}
	}
	return uStatus;
}

OpcUa_Boolean CUADataType::IsAbstract()
{
	return m_bAbstract;
}
void CUADataType::Abstract(OpcUa_Boolean aValue)
{
	m_bAbstract = aValue;
}
OpcUa_Int32	CUADataType::GetSize()
{
	if (m_pDefinition)
		return m_pDefinition->GetSize();
	else
		return m_iSize;
}
CDefinition* CUADataType::GetDefinition() 
{ 
	return	m_pDefinition;
}
void	CUADataType::SetDefinition(CDefinition* pDefinition) 
{ 
	m_pDefinition = pDefinition;
}

OpcUa_NodeId	CUADataType::GetParentType() 
{ 
	return m_ParentType; 
}
OpcUa_Byte		CUADataType::GetAncestorType() 
{ 
	return m_AncestorType; 
}
void CUADataType::SetAncestorType(OpcUa_Byte bVal) 
{ 
	m_AncestorType = bVal; 
}