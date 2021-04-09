#include "stdafx.h"
#include "LoggerMessage.h"
using namespace OpenOpcUa;
CLoggerMessage::CLoggerMessage(void)
{
	m_pString=OpcUa_Null;
	m_uiLevel=0;
}

CLoggerMessage::~CLoggerMessage(void)
{
	if (m_pString)
	{
		OpcUa_String_Clear(m_pString);
		OpcUa_Free(m_pString);
	}
}
