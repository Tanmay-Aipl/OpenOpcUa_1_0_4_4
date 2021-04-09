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
using namespace UABuiltinType;
#define OpcUa_Vfi_UnInitialized 0x87040000
namespace UAHistoricalAccess
{
	class CVfiDataValue
	{
	public:
		CVfiDataValue()
		{
		}
		~CVfiDataValue()
		{
		}
		OpcUa_DataValue* GetValue() {return m_pValue;}

		// source
		void SetSourceTimestamp(OpcUa_DateTime dateTime) 
		{
			if (m_pValue)
				m_pValue->SourceTimestamp = dateTime; 
		}
		void SetSourcePicosecond(OpcUa_UInt16 uiVal) 
		{ 
			if (m_pValue)
				m_pValue->SourcePicoseconds = uiVal; 
		}
		OpcUa_DateTime   GetSourceTimestamp() 
		{ 
			if (m_pValue)
				return m_pValue->SourceTimestamp; 
		}
		OpcUa_UInt16 GetSourcePicosecond() 
		{ 
			if (m_pValue)
				return m_pValue->SourcePicoseconds; 
		}
		// Server
		OpcUa_DateTime   GetServerTimestamp() 
		{
			if (m_pValue)
				return m_pValue->ServerTimestamp; 
		}
		OpcUa_UInt16 GetServerPicosecond() 
		{
			if (m_pValue)
				return m_pValue->ServerPicoseconds; 
		}
		void SetServerTimestamp(OpcUa_DateTime dateTime) 
		{
			if (m_pValue)
				m_pValue->ServerTimestamp = dateTime; 
		}
		void SetServerPicosecond(OpcUa_UInt16 uiVal) 
		{
			if (m_pValue)
				m_pValue->ServerPicoseconds = uiVal; 
		}
		OpcUa_StatusCode GetStatusCode() 
		{
			if (m_pValue)
				return m_pValue->StatusCode; 
		}
		void SetStatusCode(OpcUa_StatusCode status) 
		{
			if (m_pValue)
				m_pValue->StatusCode=status;
		}
	protected:
		OpcUa_DataValue*   m_pValue;
	};
}