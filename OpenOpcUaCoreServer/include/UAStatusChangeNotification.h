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
	namespace UACoreServer 
	{
		class CUAStatusChangeNotification
		{
		public:
			CUAStatusChangeNotification(void);
			CUAStatusChangeNotification(OpcUa_UInt32 uiSequenceNumber);
			~CUAStatusChangeNotification(void);
			OpcUa_UInt32 GetSequenceNumber() {return m_uiSequenceNumber;}
			OpcUa_Boolean IsAcked() {return m_bAcked;}
			void Acked() 
			{
				m_bAcked=OpcUa_True;
			}
			OpcUa_Boolean IsKeepAlive()
			{ 
				if (m_DataChangeNotificationType == NOTIFICATION_MESSAGE_KEEPALIVE)
					return OpcUa_True;
				else
					return OpcUa_False;
			}
			void SetDiagnosticInfo(OpcUa_DiagnosticInfo aDiagInfo) 
			{ 
				if (m_pInternalStatusChangeNotification) 
					m_pInternalStatusChangeNotification->DiagnosticInfo=aDiagInfo;
			}
			void SetStatusCode(OpcUa_StatusCode uStatus) 
			{
				if (m_pInternalStatusChangeNotification) 
					m_pInternalStatusChangeNotification->Status=uStatus;
			}
			OpcUa_StatusChangeNotification*	GetInternalStatusChangeNotification() {return	m_pInternalStatusChangeNotification; }
		private:
			OpcUa_StatusChangeNotification*		m_pInternalStatusChangeNotification; 
			OpcUa_UInt32						m_uiSequenceNumber; // n° de séquence de ce StatusChangeNotification
			OpcUa_Boolean						m_bAcked; // indique si le n° de séquence associé à cette notification à été acquité
			NotificationMessageType				m_DataChangeNotificationType;
		};
		typedef vector<CUAStatusChangeNotification*> CUAStatusChangeNotificationList;
	}
}
