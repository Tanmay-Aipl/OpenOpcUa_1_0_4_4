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
//#include "UABase.h"
#include <map>
using namespace std;
namespace OpenOpcUa
{
	namespace UAAddressSpace 
	{
		class CUAReferenceType :
			public CUABase
		{
		public:
			CUAReferenceType(void);
			CUAReferenceType(OpcUa_NodeClass aNodeClass, const char **atts);
			~CUAReferenceType(void);

			OpcUa_Boolean IsAbstract()
			{
				return m_bAbstract; 
			}
			void Abstract(OpcUa_Boolean aValue)
			{
				m_bAbstract=aValue;
			}
			OpcUa_Boolean IsSymetric()
			{
				return m_bSymetric; 
			}
			void Symetric(OpcUa_Boolean aValue)
			{
				m_bSymetric=aValue;
			}
			OpcUa_LocalizedText* GetInverseName()
			{
				return m_pInverseName;
			}
			void SetInverseName(OpcUa_LocalizedText aValue)
			{
				if (m_pInverseName)
					OpcUa_LocalizedText_Clear(m_pInverseName);
				OpcUa_LocalizedText_CopyTo(&aValue, m_pInverseName);
			}
		private:
			OpcUa_Boolean			m_bAbstract;  
			OpcUa_Boolean			m_bSymetric;	
			OpcUa_LocalizedText*	m_pInverseName;
		};
		typedef std::vector<CUAReferenceType*> CUAReferenceTypeList;
	}
}