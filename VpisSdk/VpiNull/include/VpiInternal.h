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
			m_SourceObjects.push_back(pSourceObject);
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
		Vpi_StatusCode GetFileParameter(char*	p_Item, Vpi_String*	p_Value);
		CVpiNull* GetVPINull() {return m_pVPINull;}
		Vpi_StatusCode GetInternalStatus() {return m_InternalStatus;}
		void SetInternalStatus(Vpi_StatusCode uStatus) {m_InternalStatus=uStatus;}
	private:
		CSourceObjectList		m_SourceObjects; // contientles association NodeId, Address, value. Il s'agit de la cache du Vpi
		Vpi_Handle		m_pVpiHandle;
		Vpi_String			m_szSubSystemName;
		Vpi_NodeId			m_SubsystemId;
		CVpiNull*				m_pVPINull;
		Vpi_StatusCode	m_InternalStatus; // permet d'indiquer l'etat interne du VPI
	};
	typedef std::vector<CVpiInternalData*> CVpiInternalDataList;
}
