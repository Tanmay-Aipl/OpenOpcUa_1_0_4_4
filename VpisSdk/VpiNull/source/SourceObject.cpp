#include "stdafx.h"
#include "SourceObject.h"

CSourceObject::CSourceObject(void)
{
	m_pValue=Vpi_Null;
}


CSourceObject::~CSourceObject(void)
{
	if (m_pValue)
		delete m_pValue;
}
