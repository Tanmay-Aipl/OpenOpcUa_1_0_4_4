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
#include "UAVariableType.h"
using namespace OpenOpcUa;
using namespace UAAddressSpace;
CUAVariableType::CUAVariableType(void)
{
}

CUAVariableType::~CUAVariableType(void)
{
}
CUAVariableType::CUAVariableType(OpcUa_NodeClass aNodeClass,const char **atts):CUABase(aNodeClass,atts) 
{
	OpcUa_NodeId_Initialize(&m_DataType);
	m_ValueRank=0;
	int ii=0;
	while (atts[ii])
	{
		if (OpcUa_StrCmpA(atts[ii],"IsAbstract")==0)
		{
			if (OpcUa_StrCmpA(atts[ii+1],"true")==0)
				m_bAbstract=true;
			else
			{
				if (OpcUa_StrCmpA(atts[ii+1],"false")==0)
					m_bAbstract=false;
				else
					m_bAbstract=(OpcUa_Boolean)atoi(atts[ii+1]);
			}
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
				OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"An incorrect ValueRank was used\n");
			}
			else			
				m_ValueRank=iVal;
		}
		//DataType
		if (OpcUa_StrCmpA(atts[ii],"DataType")==0)
		{
			OpcUa_NodeId aNodeId;
			OpcUa_NodeId_Initialize(&aNodeId);	
			{
				if (ParseNodeId(atts[ii+1],&aNodeId)==OpcUa_Good)
				{
					SetDataType(aNodeId);
				}
				else
				{
					aNodeId.Identifier.Numeric=1; // boolean by default
					aNodeId.IdentifierType=OpcUa_IdentifierType_Numeric;
					aNodeId.NamespaceIndex=0;
					SetDataType(aNodeId);
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR,"CUAVariableType::CUAVariableType>Critical error ParseNodeId failed. You are using an undefined DataType\n");
				}
			}
			OpcUa_NodeId_Clear(&aNodeId);
		}
		ii+=2;
	}
}