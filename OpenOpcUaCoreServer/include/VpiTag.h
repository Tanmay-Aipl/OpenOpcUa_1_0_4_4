/*****************************************************************************
	  Author
		©. Philippe Buchet, JRC (2011)
	  
	  Contributors
	©. Michel Condemine, 4CE Industry (2010-2014)

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

#if !defined(AFX_ACQCTRLSIGNAL_H__2A708F13_BCD6_11D1_AF01_006097758E14__INCLUDED_)
#define AFX_ACQCTRLSIGNAL_H__2A708F13_BCD6_11D1_AF01_006097758E14__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

using namespace OpenOpcUa;
using namespace UAAddressSpace;
namespace OpenOpcUa
{
	namespace UASubSystem 
	{
		class CVpiDevice;
		class CVpiTag  
		{
		public:
			CVpiTag(CVpiDevice* pDevice, CUAVariable* pUAVariable, OpcUa_String szAddress);
			CVpiTag(const char **atts,CVpiDevice* pDevice);
			virtual ~CVpiTag();
			// Method

		public:
			CVpiDevice*		GetDevice();
			void			SetName(OpcUa_String szName);
			OpcUa_String	GetName();


			void			SetDescription(OpcUa_String szDesc);

			void			SetAddress(OpcUa_String szAddr);
			OpcUa_String	GetAddress();

			void			SetModified(OpcUa_Boolean bModified=TRUE);

			void			SetNbElement(OpcUa_Int32 wNbElement);
			OpcUa_Int32		GetNbElement();
			void			SetDataType(OpcUa_NodeId nDataType);
			OpcUa_NodeId	GetDataType();
			void			SetAccessRight(OpcUa_Byte uiVal);
			OpcUa_Byte		GetAccessRight();
			CUAVariable*	GetUAVariable();
			void			SetUAVariable(CUAVariable* pVariable);
			// NodeId
			OpcUa_NodeId GetNodeId();
			void SetNodeId(OpcUa_NodeId aNodeId);
			void SyncUAVariable();
			// Properties
			// Access the Dimension
			const OpcUa_UInt16& GetDimension(void) const		{ return(m_wDimension);		}
			void SetDimension(const OpcUa_UInt16& dimension)	{ m_wDimension = dimension;	}

		protected:
			OpcUa_String	m_szName;
			OpcUa_String	m_szDescription;
			OpcUa_String	m_szAddress;
			OpcUa_UInt16	m_wDimension;
			OpcUa_Int32		m_iNbElement;
			OpcUa_NodeId	m_nDataType;
			OpcUa_Boolean	m_bModified;
			OpcUa_Byte		m_uiAccessRight; // 1=Input; 2=Output; 3=Input_Output
			CVpiDevice*		m_pDevice;
			CNumericRange*	m_pInstrumentRange; // Instrument Range. It must be a single range with a min and a max
			CNumericRange*	m_pEURange; // Engineering Range. It must be a single range with a min and a max
		private:
			OpcUa_NodeId	m_NodeId;
			CUAVariable*	m_pUAVariable;
		};
		typedef vector<CVpiTag*> CVpiTagList;
	}
}
#endif // !defined(AFX_ACQCTRLSIGNAL_H__2A708F13_BCD6_11D1_AF01_006097758E14__INCLUDED_)
