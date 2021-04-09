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
#include "NumericRange.h"
using namespace OpenOpcUa;
using namespace UASharedLib;
#include <string>
using namespace std;
CNumericRange::CNumericRange(void)
{
	m_bUniqueRange=OpcUa_False;
	m_subRanges=new CNumericRanges();
	m_iBegin=0;
	m_iEnd=0;
	// chaine pour le debug
	m_pStrRepresentation=(wchar_t*)OpcUa_Alloc(1024*sizeof(wchar_t));
	if (m_pStrRepresentation)
		ZeroMemory(m_pStrRepresentation,1024);
	m_StatusCode = OpcUa_BadIndexRangeInvalid;
}
CNumericRange::CNumericRange(OpcUa_String* pszRange)
{
	m_bUniqueRange=OpcUa_False;
	m_StatusCode = OpcUa_BadIndexRangeInvalid;// OpcUa_BadOutOfRange;
	// selon la specificication OPC UA deux expressions du range sont admissible
	// MIN:MAX pour les tableaux MIN1:MAX1,.....,MINn:MAXn
	// MIN,MAX <==> 1:1,1:1
	int iRes=-1;
	if (OpcUa_String_StrLen(pszRange) <= 0)
	{
		m_iBegin=0;
		m_iEnd=0;
		return;
	}		
	m_iBegin=0;
	m_iEnd=0;
	// chaine pour le debug
	m_pStrRepresentation=(wchar_t*)OpcUa_Alloc(1024*sizeof(wchar_t));
	if (m_pStrRepresentation)
		ZeroMemory(m_pStrRepresentation,1024);
	//
	m_subRanges=new CNumericRanges();
	// On va verifier que le szRange est bien dans un format accepté
	// on considère par défaut que la chaine est mal formée
	// On utilisera un std::string pour l'analyse
	std::string str(OpcUa_String_GetRawString(pszRange));
	// verification du cas particulier MIN,MAX qui correspond à un subRange
	unsigned int iPos=str.find(",");
	if (iPos!=string::npos)
	{
		int iBegin=-1;
		int iEnd=-1;

		iRes=sscanf(str.c_str(),"%u,%u",&iBegin,&iEnd);
		if (iRes==2)
		{
			OpcUa_String aString;
			OpcUa_String_Initialize(&aString);
			OpcUa_CharA* buf=(OpcUa_CharA*)OpcUa_Alloc(15);
			ZeroMemory(buf,15);
			sprintf(buf,"%d:%d",iBegin,iBegin);
			OpcUa_String_AttachCopy(&aString,buf);
			CNumericRange* pSubRange01=new CNumericRange(&aString);
			m_subRanges->push_back(pSubRange01);

			ZeroMemory(buf,15);
			sprintf(buf,"%d:%d",iEnd,iEnd);
			OpcUa_String_AttachCopy(&aString,buf);
			CNumericRange* pSubRange02=new CNumericRange(&aString);
			m_subRanges->push_back(pSubRange02);
			OpcUa_String_Clear(&aString);
			m_StatusCode=OpcUa_Good;
			OpcUa_Free(buf);
			return;
		}
	}
	iPos=str.find("-");
	if (iPos==string::npos)
	{
		iPos=str.find("_");
		if (iPos==string::npos)
			m_StatusCode=OpcUa_Good;
	}
	else
		m_StatusCode=OpcUa_BadIndexRangeInvalid;
	if (m_StatusCode==OpcUa_Good)
	{
		iPos=str.find(","); // Est ce que l'on a à faire a un serie de range
		if (iPos==string::npos)
		{
			iPos=str.find(":");
			if (iPos==string::npos)
			{
				// Il s'agit peut être d'un indice seul
				OpcUa_CharA*  pBuff = OpcUa_String_GetRawString(pszRange);
				if (pBuff)
				{
					iRes=sscanf(pBuff,"%u",&m_iBegin);
					if (iRes!=1)
					{
						OpcUa_Trace(OPCUA_TRACE_EXTRA_LEVEL_WARNING, "The string you specify have an invalid format\n");
					}
					else
					{
						m_bUniqueRange=OpcUa_True;
						m_iEnd=m_iBegin;
						m_iBegin=0;
						m_StatusCode=OpcUa_Good;
					}
				}
			}
			else
			{
				OpcUa_CharA*  pBuff = OpcUa_String_GetRawString(pszRange);
				if (pBuff)
				{
					iRes=sscanf(pBuff,"%u:%u",&m_iBegin,&m_iEnd);
					if (iRes!=2)
					{
						OpcUa_Trace(OPCUA_TRACE_EXTRA_LEVEL_WARNING, "The string you specify have an invalid format\n");
						m_StatusCode=OpcUa_BadIndexRangeInvalid;
					}
					else
					{
						// les bornes ne doivent pas être les mêmes
						if (m_iBegin >= m_iEnd)
							m_StatusCode =  OpcUa_BadIndexRangeInvalid;// OpcUa_BadOutOfRange;
					}
				}
			}
		}
		else
		{
			// , trouvé ==> on a a faire a un tableau
			// extraction des sous-chaines
			OpcUa_UInt32 uiLastKnownPos=0;
			std::string szprefix=str.substr(0,3);
			while (iPos!=string::npos)
			{
				iPos=str.find(",",iPos);
				int iStart=0;
				if (iPos!=string::npos)
				{
					uiLastKnownPos=iPos;
					int iBegin;
					int iEnd;
					std::string szSubRange=str.substr(iStart,iPos);
					//iRes=sscanf_s(szSubRange.c_str(),"%u:%u",&m_iBegin,&m_iEnd);
					iRes=sscanf(szSubRange.c_str(),"%u:%u",&iBegin,&iEnd);
					if (iRes!=2)
					{
						OpcUa_Trace(OPCUA_TRACE_EXTRA_LEVEL_WARNING, "The string you specify have an invalid format\n");
						m_StatusCode=OpcUa_BadIndexRangeInvalid;
					}
					else
					{
						// les bornes ne doivent pas être les mêmes
						if (iBegin >= iEnd)
							m_StatusCode = OpcUa_BadIndexRangeInvalid; // OpcUa_BadOutOfRange; // 
						else
						{
							OpcUa_String subRange;
							OpcUa_String_Initialize(&subRange);
							OpcUa_String_AttachCopy(&subRange,(OpcUa_CharA*)szSubRange.c_str());
							CNumericRange* pSubRange=new CNumericRange(&subRange);
							if (pSubRange->GetStatusCode()==OpcUa_Good)
								m_subRanges->push_back(pSubRange);
							else
								m_StatusCode=pSubRange->GetStatusCode();
							OpcUa_String_Clear(&subRange);
						}
					}
					iPos++;
				}
				else
				{
					// traitement du dernier subRange
					OpcUa_Int32 iBegin;
					OpcUa_Int32 iEnd;
					OpcUa_Int32 iSize=str.size();
					std::string szSubRange=str.substr(uiLastKnownPos+1,iSize-1);
					//iRes=sscanf_s(szSubRange.c_str(),"%u:%u",&m_iBegin,&m_iEnd);
					iRes=sscanf(szSubRange.c_str(),"%li:%li",&iBegin,&iEnd);
					if (iRes==2)
					{
						// les bornes ne doivent pas être les mêmes
						if (iBegin >= iEnd)
							m_StatusCode = OpcUa_BadIndexRangeInvalid; //OpcUa_BadOutOfRange;// 
						else
						{
							OpcUa_String subRange;
							OpcUa_String_Initialize(&subRange);
							OpcUa_String_AttachCopy(&subRange,(OpcUa_CharA*)szSubRange.c_str());
							CNumericRange* pSubRange=new CNumericRange(&subRange);
							if (pSubRange->GetStatusCode()==OpcUa_Good)
								m_subRanges->push_back(pSubRange);
							else
								m_StatusCode=pSubRange->GetStatusCode();
							OpcUa_String_Clear(&subRange);
						}
					}
				}
			}
		}
	}
}
CNumericRange::~CNumericRange(void)
{
	if (m_pStrRepresentation)
		OpcUa_Free(m_pStrRepresentation);
	if (m_subRanges)
	{
		for (OpcUa_UInt32 i = 0; i < m_subRanges->size(); i++)
		{
			CNumericRange* pSubRange = m_subRanges->at(i);
			delete pSubRange;
		}
		m_subRanges->clear();
		delete m_subRanges;
	}
}
