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
#include "UAReferenceType.h"
using namespace OpenOpcUa;
using namespace UAAddressSpace;
CUAReferenceType::CUAReferenceType(void)
{
	m_pInverseName = (OpcUa_LocalizedText*)OpcUa_Alloc(sizeof(OpcUa_LocalizedText));
	if (m_pInverseName)
		OpcUa_LocalizedText_Initialize(m_pInverseName);
}

CUAReferenceType::~CUAReferenceType(void)
{
	if (m_pInverseName)
	{
		OpcUa_LocalizedText_Clear(m_pInverseName);
		OpcUa_Free(m_pInverseName);
	}
}
CUAReferenceType::CUAReferenceType(OpcUa_NodeClass aNodeClass, const char **atts) :CUABase(aNodeClass, atts)
{
	m_pInverseName = (OpcUa_LocalizedText*)OpcUa_Alloc(sizeof(OpcUa_LocalizedText));
	if (m_pInverseName)
	{
		OpcUa_LocalizedText_Initialize(m_pInverseName);
		OpcUa_String_AttachCopy(&(m_pInverseName->Locale), "eu-us");
		OpcUa_String_AttachCopy(&(m_pInverseName->Text), "Default InverseName");

	}
	// Il reste a récupérer les autres attributs conformement au schéma
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
		if (OpcUa_StrCmpA(atts[ii], "Symetric") == 0)
		{
			if (OpcUa_StrCmpA(atts[ii + 1], "true") == 0)
				m_bSymetric = true;
			else
			{
				if (OpcUa_StrCmpA(atts[ii + 1], "false") == 0)
					m_bSymetric = false;
				else
					m_bSymetric = (OpcUa_Boolean)atoi(atts[ii + 1]);
			}
		}
		if (OpcUa_StrCmpA(atts[ii], "InverseName") == 0)
		{
			OpcUa_LocalizedText lName;
			OpcUa_LocalizedText_Initialize(&lName);
			OpcUa_String_AttachCopy(&(lName.Locale), "en-us");
			OpcUa_String_AttachCopy(&(lName.Text), (OpcUa_StringA)atts[ii + 1]);
			SetInverseName(lName);
			OpcUa_LocalizedText_Clear(&lName);
		}
		ii += 2;
	}
}