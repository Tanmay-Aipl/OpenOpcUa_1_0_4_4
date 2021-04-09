// ---------------------------------------------------------------------------
// FILE NAME            : LuaRestoreStack.cpp
// ---------------------------------------------------------------------------
// DESCRIPTION :
//
// 
// 
// ---------------------------------------------------------------------------
// VERSION              : 1.00
// DATE                 : 1-Sep-2005
// AUTHOR               : Richard Shephard
// ---------------------------------------------------------------------------
#include "stdafx.h"
#include "luainc.h"
#include "LuaVirtualMachine.h"
#include "LuaRestoreStack.h"
using namespace OpenOpcUa;
using namespace UAScript;
CLuaRestoreStack::CLuaRestoreStack(CLuaVirtualMachine* vm) : m_pState(NULL)
{
	if (vm)
	{
		m_pState = (lua_State *)vm->GetLuaState();
		if (vm->Ok())
		{
			m_iTop = lua_gettop(m_pState);
		}
	}
}

CLuaRestoreStack::~CLuaRestoreStack(void)
{
	lua_settop(m_pState, m_iTop);
}