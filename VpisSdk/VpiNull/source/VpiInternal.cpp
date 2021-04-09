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
	m_InternalStatus=Vpi_UnInitialized;
	m_pVPINull= new CVpiNull();
	m_pVPINull->SetVpiInternalData(this);
}
CVpiInternalData::~CVpiInternalData() 
{
	delete m_pVPINull;
}

