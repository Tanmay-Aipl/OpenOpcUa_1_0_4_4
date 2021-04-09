/************************************
VpiInternal.cpp
VpiInternallHelpers initial implemantation of Helper fonction in VPis
No exported.
Revision By: Michel Condemine
Revised on 22/11/2011 20:15:00
Comments: ...
************************************/ 
#include "stdafx.h"
using namespace VpiBuiltinType;
using namespace UASubSystem;
#include "VpiInternal.h"
#include "opcuavpimutsem.h"
#ifdef _GNUC_
#include <pthread.h>
#include <semaphore.h>
#include <sys/timeb.h>
#endif
CVpiInternalData::CVpiInternalData() 
{
	m_InternalStatus = Vpi_UnInitialized;
	Vpi_Mutex_Create(&m_OpcUaSourceObjectMutex);
	m_pVPINullEx = new CVPINullEx();
	m_pVPINullEx->SetVpiInternalData(this);
}
CVpiInternalData::~CVpiInternalData() 
{
	Vpi_String_Clear(&m_ConfigFileName);
	Vpi_String_Clear(&m_szSubSystemName);
	Vpi_Mutex_Lock(m_OpcUaSourceObjectMutex);
	CSourceObjectList::iterator it;
	while (!m_SourceObjects.empty())
	{
		it = m_SourceObjects.begin();
		CSourceObject* pObject = *it;
		delete pObject;
		m_SourceObjects.erase(it);
	}
	Vpi_Mutex_Unlock(m_OpcUaSourceObjectMutex);
	Vpi_Mutex_Delete(&m_OpcUaSourceObjectMutex);
	delete m_pVPINullEx;
}
Vpi_StatusCode CVpiInternalData::LoadConfigurationFile()
{
	Vpi_StatusCode uStatus=Vpi_Good;
	// Here we are suppose to use the Method GetFileParameter
	return uStatus;
}

// Function name   : CVpiInternalData::GetFileParameter
// Description     : 
// Return type     : Vpi_StatusCode 
// Argument        : char* pItem
// Argument        : Vpi_String* p_Value

Vpi_StatusCode CVpiInternalData::GetFileParameter(char* pItem, Vpi_String* p_Value)
{
	char	Buffer [128];
	char	s_Value [128];
	char	Item [64];
	FILE	* fp ;
	int		nb;



	if ((fp = fopen( m_ConfigFileName.strContent, "r")) != NULL) 
	{

		while (fgets( Buffer, sizeof(Buffer), fp) != NULL) 
		{
			nb = sscanf( Buffer, "%s %s", Item, s_Value);
			if (nb == 2 && !strcmp( Item, pItem)) 
			{
				strcpy(p_Value->strContent, s_Value);
				fclose( fp);
				return Vpi_Good ;
			}
		}
		fclose( fp);
	} 

	return Vpi_BadNotFound ;

}
