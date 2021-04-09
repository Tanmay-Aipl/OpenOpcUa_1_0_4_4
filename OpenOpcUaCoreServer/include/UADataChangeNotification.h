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
#define OpcUa_ExtensionObject_EncodeableObject_Copy(SourceVariant,TargetVariant,name) \
{\
	OpcUa_ExtensionObject* pExtensionObj=SourceVariant.Value.ExtensionObject; \
	OpcUa_##name* pSrc=(OpcUa_##name*)pExtensionObj->Body.EncodeableObject.Object; \
	OpcUa_##name* pTarget=Utils::Copy(pSrc); \
	 \
	TargetVariant.Value.ExtensionObject=(OpcUa_ExtensionObject*)OpcUa_Alloc(sizeof(OpcUa_ExtensionObject)); \
	TargetVariant.Datatype=OpcUaType_ExtensionObject; \
	TargetVariant.Value.ExtensionObject->BodySize=0; \
	TargetVariant.Value.ExtensionObject->Encoding=OpcUa_ExtensionObjectEncoding_EncodeableObject; \
	OpcUa_ExpandedNodeId_Initialize(&(TargetVariant.Value.ExtensionObject->TypeId)); \
	TargetVariant.Value.ExtensionObject->TypeId.ServerIndex=0; \
	OpcUa_String_Initialize(&(TargetVariant.Value.ExtensionObject->TypeId.NamespaceUri)); \
	TargetVariant.Value.ExtensionObject->TypeId.NodeId.Identifier.Numeric=pEncodeableType->BinaryEncodingTypeId; \
	TargetVariant.Value.ExtensionObject->TypeId.NodeId.IdentifierType=OpcUa_IdentifierType_Numeric; \
	TargetVariant.Value.ExtensionObject->TypeId.NodeId.NamespaceIndex=0; \
	TargetVariant.Value.ExtensionObject->Body.EncodeableObject.Type=pEncodeableType; \
	TargetVariant.Value.ExtensionObject->Body.EncodeableObject.Object=OpcUa_Alloc(pEncodeableType->AllocationSize); \
	TargetVariant.Value.ExtensionObject->Body.EncodeableObject.Object=pTarget; \
}

namespace OpenOpcUa
{
	namespace UACoreServer 
	{
		///-------------------------------------------------------------------------------------------------
		/// <summary>	class wrapper for the OpcUa_DataChangeNotification structure. </summary>
		/// Each instance contains only one clientHandle in the m_UAMonitoredItemNotificationList
		/// This is just to please the CTT
		/// <remarks>	Michel, 25/01/2016. </remarks>
		///-------------------------------------------------------------------------------------------------

		class CUADataChangeNotification
		{
		public:
			CUADataChangeNotification(void);
			CUADataChangeNotification(OpcUa_UInt32 uiSequenceNumber);
			//CUADataChangeNotification(OpcUa_DataChangeNotification* pDataChangeNotification);
			~CUADataChangeNotification(void);
			OpcUa_StatusCode AddMonitoredItemNotification(OpcUa_MonitoredItemNotification* pVal, OpcUa_UInt32	uiQueueSize, OpcUa_Boolean bDiscardOldest); // Ajoute un OpcUa_MonitoredItemNotification dans m_pInternalDataChangeNotification
																		   //met aussi a jour le NoOfMonitoredItems
			OpcUa_StatusCode RemoveMonitoredItemNotification(OpcUa_UInt32 clientHandle);

			OpcUa_Mutex GetInternalDataChangeNotificationMutex() 
			{
				return	m_hInternalDataChangeNotificationMutex;
			}

			OpcUa_UInt32	GetNoOfMonitoredItemNotification() 
			{
				return	m_UAMonitoredItemNotificationList.size();
			}

			OpcUa_UInt32 GetSequenceNumber();
			void SetSequenceNumber(OpcUa_UInt32 uiSequenceNumber);
			OpcUa_UInt32 GetClientHandle(); 
			OpcUa_Boolean IsAcked();
			void Acked();
			void UnAcked();
			OpcUa_Boolean IsKeepAlive();
			//OpcUa_Boolean IsItFitQueueSize(OpcUa_UInt32 clientHandle); // Determin if the use of this handle in this DataChangeNotification is allowed according to the queueSize 
			OpcUa_Boolean IsSomethingToNotify();
			NotificationMessageType GetDataChangeNotificationType() { return m_DataChangeNotificationType; }
			void SetDataChangeNotificationType(NotificationMessageType aDataChangeNotificationType) { m_DataChangeNotificationType = aDataChangeNotificationType; }

			CUAMonitoredItemNotificationList GetUAMonitoredItemNotificationList() {return 	m_UAMonitoredItemNotificationList;}
			OpcUa_Boolean IsAlreadyInTheTransmissionList(OpcUa_MonitoredItemNotification* pVal);
			void SetMaxQueueSize(OpcUa_UInt32 uiVal);
			OpcUa_UInt32 GetMaxQueueSize() {return m_uiMaxQueueSize;}
			OpcUa_Int32 GetMaxQueueSize(OpcUa_UInt32 ClientHandle);
			OpcUa_DateTime	GetPublishTime() { return m_PublishTime; }
			void SetPublishTime(OpcUa_DateTime dtVal) { m_PublishTime = dtVal; }
		//private:
			OpcUa_Boolean RemoveSentMonitoredItemNotification();
		private:
			OpcUa_Mutex							m_hInternalDataChangeNotificationMutex;
			CUAMonitoredItemNotificationList	m_UAMonitoredItemNotificationList; // 1.0.5.0 note :replace with std::map<ClientHandle,CUAMonitoredItemNotification*> The new map will replace both m_UAMonitoredItemNotificationList and m_uiQueueSizesMap
			map<OpcUa_UInt32, OpcUa_UInt32>		m_uiQueueSizesMap; // map for this queue made of <clientHandle, currentQueueSize>
			OpcUa_UInt32						m_uiSequenceNumber; // n° de séquence de cette DataChangeNotification
			OpcUa_Boolean						m_bAcked; // indique si le n° de séquence associé à cette notification à été acquité
			NotificationMessageType				m_DataChangeNotificationType;
			OpcUa_UInt32						m_uiMaxQueueSize; // Considerons qu'il s'agit de la MaxQueueSize des MonitoredItem dans cette DataChangeNotification
			OpcUa_DateTime						m_PublishTime;
		};
		typedef std::vector<CUADataChangeNotification*> CUADataChangeNotificationList;

		typedef std::map<OpcUa_UInt32,CUADataChangeNotification*> CAvailableSequenceNumbers;
	}
}