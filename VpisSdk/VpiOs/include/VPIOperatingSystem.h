#ifndef _VPIOPERATINGSYSTEM_H
#define _VPIOPERATINGSYSTEM_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "SourceObject.h"


namespace UASubSystem 
{
	class CVpiInternalData
	{
	public:

		CVpiInternalData(){}
		~CVpiInternalData() {}
		Vpi_Handle GetVpiHandle() {return m_pVpiHandle;}
		void SetVpiHandle(Vpi_Handle hVal) { m_pVpiHandle = hVal; }
		CSourceObjectList* GetVpiObjectList() {return &m_VpiObjectList;}
		void AddSourceObject(CSourceObject* pSourceObject) 
		{
			m_VpiObjectList.push_back(pSourceObject);
		}
		CSourceObject* GetSourceObject(Vpi_NodeId aNodeId)
		{
			for (Vpi_UInt32 i = 0; i < m_SourceObjects.size();i++)
			{
				CSourceObject* pObject = m_SourceObjects.at(i);
				if (IsEqual(&(pObject->GetNodeId()) , &aNodeId) )
					return pObject;
			}
			return NULL;
		}
		CSourceObjectList		m_SourceObjects; // contientles association NodeId, Address, value. Il s'agit de la cache du Vpi
		Vpi_String GetSubSystemName() {return m_szSubSystemName;}
		void SetSubSystemName(Vpi_String szVal) 
		{
			(void)szVal;
			// TODO Allouer et affecter
			m_szSubSystemName;
		}
		Vpi_NodeId GetSubsystemId() {m_SubsystemId;}
		void SetSubsystemId(Vpi_NodeId id) { m_SubsystemId = id; }
		Vpi_StatusCode LoadConfigurationFile();
	private:
		Vpi_Handle m_pVpiHandle;
		CSourceObjectList m_VpiObjectList;
		Vpi_String m_szSubSystemName;
		Vpi_NodeId m_SubsystemId;
	};
}
#endif // _VPIOPERATINGSYSTEM_H