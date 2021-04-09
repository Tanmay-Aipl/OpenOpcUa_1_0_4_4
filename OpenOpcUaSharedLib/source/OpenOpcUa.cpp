#include "stdafx.h"
#include "OpenOpcUa.h"
using namespace OpenOpcUa;
using namespace UASharedLib;
COpenOpcUa::COpenOpcUa(void)
{
	m_ClassName=std::string("UASharedLib::CSessionBase");
}

COpenOpcUa::~COpenOpcUa(void)
{
	m_ClassName.empty();
	m_ClassName.clear();
}
