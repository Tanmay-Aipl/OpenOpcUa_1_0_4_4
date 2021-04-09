#include "stdafx.h"
#include "SourceObject.h"

CSourceObject::CSourceObject(void)
{
	m_pValue=Vpi_Null;
}
CSourceObject::CSourceObject(Vpi_Byte Datatype, Vpi_UInt32 iNbElt)
{
	m_pValue=new CVpiDataValue(Datatype,iNbElt);
	if (iNbElt)
		m_pValue->SetArraySize(iNbElt);
	else
		m_pValue->SetArrayType(Vpi_VariantArrayType_Scalar);
}

CSourceObject::~CSourceObject(void)
{
	if (m_pValue)
		delete m_pValue;
}
