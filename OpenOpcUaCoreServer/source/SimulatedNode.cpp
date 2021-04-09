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
#include "SimulatedNode.h"
using namespace OpenOpcUa;
using namespace UASimulation;
CSimulatedNode::CSimulatedNode(void)
{
	m_pNodeId=(OpcUa_NodeId*)OpcUa_Alloc(sizeof(OpcUa_NodeId));
	if (m_pNodeId)
		OpcUa_NodeId_Initialize(m_pNodeId);
	m_eSimType = SIMTYPE_NONE;
}

CSimulatedNode::~CSimulatedNode(void)
{
	if (m_pNodeId)
	{
		OpcUa_NodeId_Clear(m_pNodeId);
		OpcUa_Free(m_pNodeId);
		m_pNodeId = OpcUa_Null;
	}
}
CSimulatedNode::CSimulatedNode(const char **atts)
{
	m_pNodeId=(OpcUa_NodeId*)OpcUa_Alloc(sizeof(OpcUa_NodeId));
	if (m_pNodeId)
	{
		OpcUa_NodeId_Initialize(m_pNodeId);
		m_eSimType = SIMTYPE_NONE;
		OpcUa_Int32 ii = 0;
		while (atts[ii])
		{
			if (OpcUa_StrCmpA(atts[ii], "NodeId") == 0)
			{
				// NodeId
				OpcUa_NodeId aNodeId;
				OpcUa_NodeId_Initialize(&aNodeId);

				if (ParseNodeId(atts[ii + 1], &aNodeId) == OpcUa_Good)
					SetNodeId(aNodeId);
				else
					OpcUa_Trace(OPCUA_TRACE_SERVER_LEVEL_ERROR, " XML file corrupted CSimulatedNode\n");
				OpcUa_NodeId_Clear(&aNodeId);

			}
			if (OpcUa_StrCmpA(atts[ii], "SimulationType") == 0)
			{
				if (OpcUa_StrCmpA(atts[ii + 1], "None") == 0)
					m_eSimType = SIMTYPE_NONE;
				if (OpcUa_StrCmpA(atts[ii + 1], "Ramp") == 0)
					m_eSimType = SIMTYPE_RAMP;
				if (OpcUa_StrCmpA(atts[ii + 1], "Sine") == 0)
					m_eSimType = SIMTYPE_SINE;
				if (OpcUa_StrCmpA(atts[ii + 1], "Random") == 0)
					m_eSimType = SIMTYPE_RANDOM;
			}
			if (OpcUa_StrCmpA(atts[ii], "Min") == 0)
			{
				m_dblMin = atof(atts[ii + 1]);
			}
			if (OpcUa_StrCmpA(atts[ii], "Max") == 0)
			{
				m_dblMax = atof(atts[ii + 1]);
			}
			ii += 2;
		}
	}
}
void CSimulatedNode::SetNodeId(OpcUa_NodeId aNodeId)
{
	if (m_pNodeId)
	{
		OpcUa_NodeId_Initialize(m_pNodeId);
		OpcUa_NodeId_CopyTo(&aNodeId, m_pNodeId);
	}
}