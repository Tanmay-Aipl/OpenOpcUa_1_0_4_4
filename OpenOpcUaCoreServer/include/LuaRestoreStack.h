// ---------------------------------------------------------------------------
// FILE NAME            : LuaRestoreStack.h
// ---------------------------------------------------------------------------
// DESCRIPTION :
//
// Restores the Lua stack to the way we found it :)
// 
// ---------------------------------------------------------------------------
// VERSION              : 1.00
// DATE                 : 1-Sep-2005
// AUTHOR               : Richard Shephard
// ---------------------------------------------------------------------------
// LIBRARY INCLUDE FILES
#ifndef __LUA_RESTORE_STACK_H__
#define __LUA_RESTORE_STACK_H__

#include "luainc.h"
namespace OpenOpcUa
{
	namespace UAScript
	{
		class CLuaRestoreStack
		{
		public:
			CLuaRestoreStack(CLuaVirtualMachine* vm);

			virtual ~CLuaRestoreStack(void);

		protected:
			lua_State *m_pState;
			int m_iTop;
		};
	}
}
#endif // __LUA_RESTORE_STACK_H__