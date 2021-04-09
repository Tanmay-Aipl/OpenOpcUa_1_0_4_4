#pragma once
namespace UASubSystem 
{
	class CVpiInternalData
	{
	public:
		CVpiInternalData();
		~CVpiInternalData();
		Vpi_Handle GetVpiHandle() {return m_pVpiHandle;}
		void SetVpiHandle(Vpi_Handle hVal) {m_pVpiHandle=hVal;}
		CSourceObjectList* GeSourceObjectList() { return &m_SourceObjects; }
		void AddSourceObject(CSourceObject* pSourceObject)
		{
			Vpi_Mutex_Lock(m_OpcUaSourceObjectMutex);
			m_SourceObjects.push_back(pSourceObject);
			Vpi_Mutex_Unlock(m_OpcUaSourceObjectMutex);
		}
		CSourceObject* GetSourceObject(Vpi_NodeId aNodeId)
		{
			for (Vpi_UInt32 i = 0; i < m_SourceObjects.size(); i++)
			{
				CSourceObject* pObject = m_SourceObjects.at(i);
				if (IsEqual(&(pObject->GetNodeId()), &aNodeId))
					return pObject;
			}
			return Vpi_Null;
		}
		CSourceObject* GetSourceObject(Vpi_String szAddress)
		{
			CSourceObjectList* pSourceObjects = GeSourceObjectList();
			for (Vpi_UInt16 ii = 0; ii<pSourceObjects->size(); ii++)
			{
				CSourceObject* pObject = pSourceObjects->at(ii);
				Vpi_String szLocalAddress = pObject->GetAddress();
				if (Vpi_String_Compare(&szAddress, &szLocalAddress) == 0)
					return pObject;
			}
			return Vpi_Null;
		}
		Vpi_String GetSubSystemName() {return m_szSubSystemName;}
		void SetSubSystemName(Vpi_String szVal) 
		{
			m_szSubSystemName.strContent=(char*)malloc(std::string(szVal.strContent).length()+1);
			m_szSubSystemName.uLength=std::string(szVal.strContent).length();
			memset(m_szSubSystemName.strContent,0,m_szSubSystemName.uLength+1);
			strcpy(m_szSubSystemName.strContent,szVal.strContent);
		}
		Vpi_NodeId GetSubsystemId() {return m_SubsystemId;}
		void SetSubsystemId(Vpi_NodeId id) {m_SubsystemId=id;}
		Vpi_StatusCode LoadConfigurationFile();
		Vpi_StatusCode GetFileParameter(char*	p_Item, Vpi_String*	p_Value);
		void SetConfigFileName(Vpi_String ConfigFileName) 
		{
			int iLen=strlen(ConfigFileName.strContent)+1;
			m_ConfigFileName.strContent=(Vpi_CharA*)malloc(iLen);
			memset(m_ConfigFileName.strContent,0,iLen);
			m_ConfigFileName.uLength=strlen(ConfigFileName.strContent);
			strcpy(m_ConfigFileName.strContent,ConfigFileName.strContent);
		}
		Vpi_String GetConfigFileName() {return m_ConfigFileName;}
		CVPINullEx* GetVPINullEx() { return m_pVPINullEx; }
		Vpi_StatusCode GetInternalStatus() {return m_InternalStatus;}
		void SetInternalStatus(Vpi_StatusCode uStatus) {m_InternalStatus=uStatus;}
	public:
		Vpi_Mutex			m_OpcUaSourceObjectMutex;
	private:
		CSourceObjectList	m_SourceObjects; // contientles association NodeId, Address, value. Il s'agit de la cache du Vpi
		Vpi_Handle			m_pVpiHandle;
		Vpi_String			m_szSubSystemName;
		Vpi_NodeId			m_SubsystemId;
		Vpi_String			m_ConfigFileName;
		CVPINullEx*			m_pVPINullEx;
		Vpi_StatusCode		m_InternalStatus; // permet d'indiquer l'etat interne du VPI
		void*				m_ProxyStubConfiguration; // for trace 
	};
	typedef std::vector<CVpiInternalData*> CVpiInternalDataList;
}
