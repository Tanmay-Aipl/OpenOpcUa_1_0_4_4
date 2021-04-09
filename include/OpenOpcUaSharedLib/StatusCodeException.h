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
		// Stores a status code and message that can be thrown as an exception.
		class CStatusCodeException
		{
		public:

			// Creates a default exception.
			CStatusCodeException()
			{
				m_code = OpcUa_Bad;
				OpcUa_String_Initialize(&m_message);
			}

			// Creates a new instance of the exception.
			CStatusCodeException(OpcUa_StatusCode code, OpcUa_String message)
			{
				m_code = code;
				OpcUa_String_Initialize(&m_message);
				OpcUa_String_CopyTo(&message,&m_message);
			}
			// Creates a new instance of the exception.
			CStatusCodeException(OpcUa_StatusCode code, OpcUa_CharA* message)
			{
				m_code = code;
				OpcUa_String_Initialize(&m_message);
				if (message)
					OpcUa_String_AttachCopy(&m_message,message);
				else
					OpcUa_String_AttachCopy(&m_message,"Empty message");
			}
		    	
			// Frees all resources used by the exception.
			~CStatusCodeException(void)
			{
				OpcUa_String_Clear(&m_message);
			}

			// Returns the status code associated with the exception.
			OpcUa_StatusCode GetCode(void)
			{
				return m_code;
			}

			// Returns the message associated with the exception.
			OpcUa_String GetMessage(void)
			{
				return m_message;
			}

		private:

			OpcUa_StatusCode m_code;
			OpcUa_String m_message;
		};
	}
}
#define ThrowIfBad(xStatus,xMessage) if (OpcUa_IsBad(xStatus)) throw CStatusCodeException(xStatus,xMessage); 
#define ThrowIfAllocFailed(xBuffer) if (xBuffer == NULL) throw CStatusCodeException(OpcUa_BadOutOfMemory,"Memory allocation failed."); 
#define ThrowIfCallFailed(xStatus,xFunction) if (OpcUa_IsBad(xStatus)) throw CStatusCodeException(xStatus,#xFunction " call failed."); 