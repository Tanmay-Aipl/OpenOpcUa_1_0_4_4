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
#include <list>
using namespace std;
namespace OpenOpcUa
{
	namespace UASharedLib 
	{
		class CMonitoredItemBase;
		class SHAREDLIB_EXPORT CSubscription : public COpenOpcUa
		{
		public:
			CSubscription(void);
			virtual ~CSubscription(void);
			OpcUa_UInt32 GetSubscriptionId();
			void SetSubscriptionId(OpcUa_UInt32 val);
			// PublishingInterval
			OpcUa_Double GetPublishingInterval();
			void SetPublishingInterval(OpcUa_Double val);
			// LifetimeCount
			OpcUa_UInt32 GetLifetimeCount();
			void SetLifetimeCount(OpcUa_UInt32 val);
			// MaxKeepAliveCount
			OpcUa_UInt32 GetMaxKeepAliveCount();
			void SetMaxKeepAliveCount(OpcUa_UInt32 val);
			// Priority
			OpcUa_Byte GetPriority();
			void SetPriority(OpcUa_Byte bVal);
			// MaxNotificationsPerPublish;
			OpcUa_UInt32 GetMaxNotificationsPerPublish();
			void SetMaxNotificationsPerPublish(OpcUa_UInt32 uiVal);
			//
			void SetChannel(CChannel* pChannel);
			CChannel* GetChannel();
			OpcUa_Handle GetHandle();
			void SetHandle(OpcUa_Handle hVal);
			OpcUa_Boolean GetPublishingEnabled();
			void SetPublishingEnabled(OpcUa_Boolean bPublishingEnabled);
			OpcUa_Int32 GetDefaultMonitoringMode();
			void SetDefaultMonitoringMode(OpcUa_Int32);
		protected:
			OpcUa_UInt32						m_pSubscriptionId;
			OpcUa_Double						m_pRevisedPublishingInterval;
			OpcUa_UInt32						m_pRevisedLifetimeCount; // quand  (m_pRevisedLifetimeCount * m_pRevisedPublishingInterval) atteint sans publish ==> destruction de la souscritpion
			OpcUa_UInt32						m_pRevisedMaxKeepAliveCount; //quand (m_pRevisedMaxKeepAliveCount*m_pRevisedPublishingInterval) ==> envoi d'une notification keepalive
			OpcUa_UInt32						m_uiMaxNotificationsPerPublish; // The number of notifications per publish is the sum of monitoredItems in the DataChangeNotification
			OpcUa_Byte							m_Priority; 
			CChannel*							m_pChannel;
			OpcUa_Handle						m_hSubscription;
			OpcUa_MonitoringMode				m_iMonitoringMode;
			OpcUa_Boolean						m_bPublishingEnabled;
		};
	}// namespace 
}