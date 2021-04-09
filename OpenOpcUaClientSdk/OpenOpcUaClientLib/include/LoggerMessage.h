#pragma once
namespace OpenOpcUa
{
	class CLoggerMessage
	{
	public:
		CLoggerMessage(void);
		~CLoggerMessage(void);

		OpcUa_UInt32 m_uiLevel;
		OpcUa_String* m_pString;
	};
}