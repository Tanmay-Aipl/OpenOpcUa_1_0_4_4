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
//#include "VpiDataValue.h"
#include "UAVariable.h"
#include "Field.h"
#include "Definition.h"
#include "UADataType.h"
#include "UAMethod.h"
#include "UAView.h"
#include "UAVariableType.h"
#include "UAObjectType.h"
#include "UAObject.h"
#include "EventDefinition.h"
using namespace UAAddressSpace;
using namespace OpenOpcUa;
using namespace UASubSystem;
#include "VpiWriteObject.h"
CVpiWriteObject::CVpiWriteObject()
{
	m_bWrite = OpcUa_False;
	OpcUa_NodeId_Initialize(&m_NodeId);
	m_pVpiDataValue = OpcUa_Null;
	m_pFuncCaller = OpcUa_Null;
}
CVpiWriteObject::CVpiWriteObject(OpcUa_NodeId	aNodeId,CVpiDataValue* pVpiDataValue,CVpiFuncCaller* pFuncCaller,CUAVariable* pUAVariable)
{
	m_bWrite = OpcUa_False;
	OpcUa_NodeId_Initialize(&m_NodeId);
	m_pVpiDataValue = OpcUa_Null;
	m_pFuncCaller = OpcUa_Null;
	if (pFuncCaller)
	{
		m_pFuncCaller=pFuncCaller;
		if (pUAVariable)
		{
			m_pUAVariable=pUAVariable;
			if (pVpiDataValue)
			{
				OpcUa_Variant* pVariant=pVpiDataValue->GetValue();
				if (pVariant)
				{
					OpcUa_Byte bBuiltInType=pVpiDataValue->GetDataType();
					OpcUa_UInt32 uiNbElt=pVpiDataValue->GetArraySize();
					OpcUa_Boolean bLatin1 = pVpiDataValue->IsLatin1();
					m_pVpiDataValue = new CVpiDataValue(bBuiltInType, uiNbElt, bLatin1);
					if (m_pVpiDataValue)
					{
						m_pVpiDataValue->SetValue(pVariant);
						OpcUa_NodeId_CopyTo(&aNodeId,&m_NodeId);
					}
				}
			}
		}
	}
}
CVpiWriteObject::~CVpiWriteObject()
{
	if (m_pVpiDataValue)
		delete m_pVpiDataValue;
	OpcUa_NodeId_Clear(&m_NodeId);
	m_pFuncCaller=OpcUa_Null;
}