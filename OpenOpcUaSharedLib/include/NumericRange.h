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
#pragma once
namespace OpenOpcUa
{
	namespace UASharedLib 
	{
	// just for CNumericRange definition
		class CNumericRange;
		typedef std::vector<CNumericRange*> CNumericRanges;
		class SHAREDLIB_EXPORT CNumericRange
		{
		public:
			CNumericRange(void);
			CNumericRange(OpcUa_String* pszRange);
			~CNumericRange(void);
			OpcUa_Int32 GetBeginIndex() {return m_iBegin;}
			OpcUa_Int32 GetEndIndex() {return m_iEnd;}
			CNumericRange* GetRangeAt(int iPos) 
			{
				if (m_subRanges)
					return (*m_subRanges)[iPos];
				else
					return OpcUa_Null;
			}
			OpcUa_Boolean IsInRange(int iIndex)
			{
				if (m_bUniqueRange)
				{
					// Cas ou le CNumericRange est défini comme un indice unique
					if ( (m_iBegin<iIndex) && (m_iEnd>=iIndex))
						return OpcUa_True;
					else
						return OpcUa_False;
				}
				else
				{
					// Cas ou le CNumericRange est défini comme un veritable Range
					if ( (m_iBegin<=iIndex) && (m_iEnd>=iIndex))
						return OpcUa_True;
					else
						return OpcUa_False;
				}
			}
			wchar_t* ToString()
			{
				if (m_subRanges)
				{
					wchar_t wcsLocal[30];
					ZeroMemory(wcsLocal,30);
					OpcUa_SWPrintf(wcsLocal,30,L"Begin:%i-End:%li",GetBeginIndex(),GetEndIndex());
					
					wcsncat(m_pStrRepresentation,wcsLocal,wcslen(wcsLocal));
					for(CNumericRanges::iterator it = m_subRanges->begin(); it != m_subRanges->end(); it++)
					{
						CNumericRange* pRange = *it;
						ZeroMemory(wcsLocal,30);
						OpcUa_SWPrintf(wcsLocal,30,L"Begin:%i-End:%li",pRange->GetBeginIndex(),pRange->GetEndIndex());
						wcsncat(m_pStrRepresentation,wcsLocal,wcslen(wcsLocal));
					}
				}
				return m_pStrRepresentation;
			}
			OpcUa_StatusCode GetStatusCode() {return m_StatusCode;}
			OpcUa_Boolean IsUnique() {return m_bUniqueRange;}
			OpcUa_Boolean IsMultiDimensional() 
			{ 
				if (m_subRanges) 
				{
					if (m_subRanges->size()>0)
						return OpcUa_True;
				}
				return OpcUa_False;
			}
			OpcUa_UInt32 GetNoOfSubRanges() {return m_subRanges->size();}
		private:
			int					m_iBegin;
			int					m_iEnd;
			CNumericRanges*		m_subRanges;
			wchar_t*			m_pStrRepresentation;
			OpcUa_StatusCode	m_StatusCode;
			OpcUa_Boolean		m_bUniqueRange; // indique que l'on et en train de traiter un indice seul
		};
	}
}