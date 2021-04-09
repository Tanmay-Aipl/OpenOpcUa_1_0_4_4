#ifndef _VPIOPERATINGSYSTEM_H
#define _VPIOPERATINGSYSTEM_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "SourceObject.h"


namespace UASubSystem 
{
	class CVpiInternalData;
	class CVpiNull
	{
	public:

		CVpiNull();
		~CVpiNull();
		void SetVpiInternalData(CVpiInternalData* pInternalData) {m_pInternalData=pInternalData;}
		Vpi_Handle GetVpiHandle() {return m_pVpiHandle;}
		void SetVpiHandle(Vpi_Handle hVal) {m_pVpiHandle=hVal;}


		Vpi_String GetSubSystemName() {return m_szSubSystemName;}
		void SetSubSystemName(Vpi_String szVal) 
		{
			(void)szVal;
		}
		Vpi_NodeId GetSubsystemId() {return m_SubsystemId;}
		void SetSubsystemId(Vpi_NodeId id) {m_SubsystemId=id;}
	private:
		CVpiInternalData* m_pInternalData;
		Vpi_Handle m_pVpiHandle;
		Vpi_String m_szSubSystemName;
		Vpi_NodeId m_SubsystemId;
	};
}
#endif // _VPIOPERATINGSYSTEM_H
