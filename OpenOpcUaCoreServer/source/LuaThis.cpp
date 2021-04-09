#include "stdafx.h"
#include "luainc.h"
#include "LuaDebugger.h"
#include "LuaVirtualMachine.h"
#include "LuaThis.h"
using namespace OpenOpcUa;
using namespace UAScript;
CLuaThis::CLuaThis(CLuaVirtualMachine* vm, int iRef) : m_iOldRef(0), m_pVm(vm)
{
	//m_pVm = new CLuaVirtualMachine();
	lua_State *state = (lua_State *)m_pVm->GetLuaState();
	if (m_pVm->Ok())
	{
		// Save the old "this" table
		lua_getglobal(state, "this");
		m_iOldRef = luaL_ref(state, LUA_REGISTRYINDEX);

		// replace it with our new one
		lua_rawgeti(state, LUA_REGISTRYINDEX, iRef);
		lua_setglobal(state, "this");
	}
}

CLuaThis::~CLuaThis(void)
{
	if (m_pVm)
	{
		lua_State *state = (lua_State *)m_pVm->GetLuaState();
		if (m_iOldRef > 0 && m_pVm->Ok())
		{
			// Replace the old "this" table
			lua_rawgeti(state, LUA_REGISTRYINDEX, m_iOldRef);
			lua_setglobal(state, "this");
			luaL_unref(state, LUA_REGISTRYINDEX, m_iOldRef);
		}
		//delete m_pVm;
		//m_pVm = OpcUa_Null;
	}
}