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
#include "UAReference.h"

using namespace OpenOpcUa;
using namespace UAAddressSpace;

CUAReference::CUAReference(const char **atts)
{
	m_pInternalReference=(OpcUa_ReferenceNode*)OpcUa_Alloc(sizeof(OpcUa_ReferenceNode));
	if (m_pInternalReference)
	{
		OpcUa_ReferenceNode_Initialize(m_pInternalReference);
		OpcUa_ExpandedNodeId_Initialize(&(m_pInternalReference->TargetId));
		OpcUa_NodeId_Initialize(&(m_pInternalReference->ReferenceTypeId));
		OpcUa_Int32 ii=0;
		while (atts[ii])
		{
			if (OpcUa_StrCmpA(atts[ii], "ReferenceType") == 0)
			{
				OpcUa_String aString;
				OpcUa_String_Initialize(&aString);
				OpcUa_String_AttachCopy(&aString, atts[ii + 1]);

				OpcUa_NodeId* pNodeId = (OpcUa_NodeId*)OpcUa_Alloc(sizeof(OpcUa_NodeId));
				if (pNodeId)
				{
					OpcUa_NodeId_Initialize(pNodeId);
					if (ParseNodeId((char*)OpcUa_String_GetRawString(&aString), pNodeId) == OpcUa_Good)
						SetReferenceTypeId(pNodeId);
					else
						OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, "XmlStartElementHandler cannot parse this alias or this nodeId! %s. Your configuration file is corruped\n", OpcUa_String_GetRawString(&aString));
					OpcUa_NodeId_Clear(pNodeId);
					OpcUa_Free(pNodeId);
				}
				OpcUa_String_Clear(&aString);
			}
			if (OpcUa_StrCmpA(atts[ii], "IsForward") == 0)
			{
				if (OpcUa_StrCmpA(atts[ii + 1], "true") == 0)
					SetInverse(OpcUa_False);
				else
				{
					if (OpcUa_StrCmpA(atts[ii + 1], "false") == 0)
						SetInverse(OpcUa_True);
					else
						SetInverse((OpcUa_Boolean)atoi(atts[ii + 1]));
				}
				// si pNewReferenceNode->IsInverse == true
				// Il faut réaliser un traitement spécifique.
			}
			ii += 2;
		}
	}
}
CUAReference::CUAReference(void)
{
	m_pInternalReference=(OpcUa_ReferenceNode*)OpcUa_Alloc(sizeof(OpcUa_ReferenceNode));
	if (m_pInternalReference)
	{
		OpcUa_ReferenceNode_Initialize(m_pInternalReference);
		OpcUa_ExpandedNodeId_Initialize(&(m_pInternalReference->TargetId));
		OpcUa_NodeId_Initialize(&(m_pInternalReference->ReferenceTypeId));
	}
}

CUAReference::~CUAReference(void)
{
	if (m_pInternalReference)
	{
		OpcUa_ReferenceNode_Clear(m_pInternalReference);
		OpcUa_Free(m_pInternalReference);
	}
}
// Permet de determiner si cette reference est de type HasTypeDefinition
// renvoie OpcUa_True si oui et OPCUa_False sinon
OpcUa_Boolean CUAReference::IsTypeDefinition()
{
	OpcUa_Boolean bResult=OpcUa_False;
	OpcUa_NodeId aNodeId=GetReferenceTypeId();
	switch (aNodeId.IdentifierType)
	{
	case OpcUa_IdentifierType_Numeric:
		if ( (aNodeId.Identifier.Numeric==OpcUaId_HasTypeDefinition) && (aNodeId.NamespaceIndex==0) )
			bResult=OpcUa_True;
		break;
	case OpcUa_IdentifierType_String: // 
		if ( (OpcUa_StrCmpA(OpcUa_String_GetRawString(&(aNodeId.Identifier.String)),"HasTypeDefinition")==0) && (aNodeId.NamespaceIndex==0) )
			bResult=OpcUa_True;
		break;
	default:
		break;
	}
	return bResult;
}
// Permet de determiner si cette reference est de type HasHistoricalConfiguration
// renvoie OpcUa_True si oui et OPCUa_False sinon
OpcUa_Boolean CUAReference::IsHistoricalConfiguration()
{
	OpcUa_Boolean bResult=OpcUa_False;
	OpcUa_NodeId aNodeId=GetReferenceTypeId();
	switch (aNodeId.IdentifierType)
	{
	case OpcUa_IdentifierType_Numeric:
		if ( (aNodeId.Identifier.Numeric==OpcUaId_HasHistoricalConfiguration) && (aNodeId.NamespaceIndex==0) )
			bResult=OpcUa_True;
		break;
	case OpcUa_IdentifierType_String: // 
		if ( (OpcUa_StrCmpA(OpcUa_String_GetRawString(&(aNodeId.Identifier.String)),"HasHistoricalConfiguration")==0) && (aNodeId.NamespaceIndex==0) )
			bResult=OpcUa_True;
		break;
	default:
		break;
	}
	return bResult;
}

OpcUa_NodeId CUAReference::GetReferenceTypeId() 
{ 
	return m_pInternalReference->ReferenceTypeId; 
}
void CUAReference::SetReferenceTypeId(OpcUa_NodeId* pVal)
{
	if (m_pInternalReference)
	{
		OpcUa_NodeId_Initialize(&(m_pInternalReference->ReferenceTypeId));
		OpcUa_NodeId_CopyTo(pVal, &(m_pInternalReference->ReferenceTypeId));
	}
}
OpcUa_Boolean CUAReference::IsInverse() 
{ 
	OpcUa_Boolean bResult=OpcUa_False;
	if (m_pInternalReference)
		bResult = m_pInternalReference->IsInverse;
	return bResult; 
}
void CUAReference::SetInverse(OpcUa_Boolean bVal) 
{ 
	if (m_pInternalReference)
		m_pInternalReference->IsInverse = bVal; 
}
OpcUa_ExpandedNodeId CUAReference::GetTargetId() 
{ 
	if (m_pInternalReference)
		return m_pInternalReference->TargetId;
	else
		throw std::exception();
}
void CUAReference::SetTargetId(OpcUa_ExpandedNodeId* pVal)
{	
	if (m_pInternalReference)
		OpcUa_ExpandedNodeId_CopyTo(pVal, &(m_pInternalReference->TargetId));
}
void CUAReference::SetTargetId(OpcUa_NodeId* pVal)
{
	if (m_pInternalReference)
	{
		if (pVal)
		{
			OpcUa_NodeId_Initialize(&(m_pInternalReference->TargetId.NodeId));
			OpcUa_NodeId_CopyTo(pVal, &(m_pInternalReference->TargetId.NodeId));
			m_pInternalReference->TargetId.ServerIndex = 0;
			OpcUa_String_Initialize(&(m_pInternalReference->TargetId.NamespaceUri));
		}
	}
}