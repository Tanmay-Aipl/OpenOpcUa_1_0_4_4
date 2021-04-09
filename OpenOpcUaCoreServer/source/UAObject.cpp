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

using namespace OpenOpcUa;
using namespace UAAddressSpace;
CUAObject::CUAObject(void)
{
	m_bEventNotifier=OpcUa_False;
}
CUAObject::CUAObject(OpcUa_NodeClass aNodeClass, const char **atts) :CUABase(aNodeClass, atts)
{
	m_bEventNotifier = OpcUa_False;
	int ii = 0;
	while (atts[ii])
	{
		if (OpcUa_StrCmpA(atts[ii], "EventNotifier") == 0)
		{
			if (OpcUa_StrCmpA(atts[ii + 1], "true") == 0)
				m_bEventNotifier = true;
			else
			{
				if (OpcUa_StrCmpA(atts[ii + 1], "false") == 0)
					m_bEventNotifier = false;
				else
					m_bEventNotifier = (OpcUa_Byte)atoi(atts[ii + 1]);
			}
		}
		ii += 2;
	}
}
CUAObject::~CUAObject(void)
{
}
/////////////////////////////////////////////////////
// Implementation pour CUAObjects
//
OpcUa_StatusCode CUAObject::Write(OpcUa_UInt32 AttributeId, OpcUa_String szIndexRange, OpcUa_DataValue Value) 
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	uStatus = CUABase::Write(AttributeId, szIndexRange, Value);
	if (uStatus == OpcUa_Good)
	{
		if (Utils::IsWritable(AttributeId, GetWriteMask()))
		{
			if (AttributeId == OpcUa_Attributes_EventNotifier)
			{
				if (Value.Value.Datatype == OpcUaType_Byte)
					SetEventNotifier(Value.Value.Value.Byte);
				else
					uStatus = OpcUa_BadInvalidArgument;
			}
		}
	}
	return uStatus;
}

OpcUa_Byte CUAObject::GetEventNotifier()
{
	return m_bEventNotifier;
}
void CUAObject::SetEventNotifier(OpcUa_Byte aValue)
{
	m_bEventNotifier = aValue;
}
CUAObject* CUAObject::operator=(CUAObject* pUAObject)
{
	m_bEventNotifier = pUAObject->m_bEventNotifier;
	return this;
}