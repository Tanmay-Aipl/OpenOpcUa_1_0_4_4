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
#include "ExtensionObject.h"
using namespace OpenOpcUa;
using namespace UASharedLib;
CExtensionObject::CExtensionObject(void)
{
	m_pInternalExtensionObject=(OpcUa_ExtensionObject*)OpcUa_Alloc(sizeof(OpcUa_ExtensionObject));
	if (m_pInternalExtensionObject)
		OpcUa_ExtensionObject_Initialize( m_pInternalExtensionObject);
}

CExtensionObject::CExtensionObject(const char **atts)
{
	OpcUa_ReferenceParameter(atts);
	m_pInternalExtensionObject=(OpcUa_ExtensionObject*)OpcUa_Alloc(sizeof(OpcUa_ExtensionObject));
	if (m_pInternalExtensionObject)
		OpcUa_ExtensionObject_Initialize( m_pInternalExtensionObject);
}
//CExtensionObject::CExtensionObject(OpcUa_ExtensionObject* pExtensionObject)
//{
//	if (pExtensionObject)
//	{
//		m_pInternalExtensionObject=(OpcUa_ExtensionObject*)OpcUa_Alloc(sizeof(OpcUa_ExtensionObject));
//		if (m_pInternalExtensionObject)
//		{
//			OpcUa_ExtensionObject_Initialize( m_pInternalExtensionObject);
//			m_pInternalExtensionObject=(Utils::Copy(pExtensionObject));
//		}
//	}
//}
void CExtensionObject::SetEncoding(OpcUa_ExtensionObjectEncoding aEncoding)
{
	if (m_pInternalExtensionObject)
		m_pInternalExtensionObject->Encoding=aEncoding;
}
CExtensionObject::~CExtensionObject(void)
{
	if (m_pInternalExtensionObject)
	{
		OpcUa_Free(m_pInternalExtensionObject);
	}
}

OpcUa_ByteString* CExtensionObject::GetBodyBinary()
{
	if ( (m_pInternalExtensionObject->Encoding!=OpcUa_ExtensionObjectEncoding_Binary)  || (m_pInternalExtensionObject->Encoding==OpcUa_ExtensionObjectEncoding_None )  )
		return (OpcUa_ByteString*)NULL;
	else
		return &(m_pInternalExtensionObject->Body.Binary);
}
OpcUa_XmlElement* CExtensionObject::GetBodyXml()
{
	return NULL;
}
void* CExtensionObject::GetBodyObject()
{
	return NULL;
}
