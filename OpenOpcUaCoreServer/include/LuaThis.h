// ---------------------------------------------------------------------------
// FILE NAME            : LuaThis.h
// ---------------------------------------------------------------------------
// DESCRIPTION :
//
// Controls the "this" table
// 
// ---------------------------------------------------------------------------
// VERSION              : 1.01
// DATE                 : 1-Sep-2005
// AUTHOR               : Richard Shephard
// ---------------------------------------------------------------------------
// LIBRARY INCLUDE FILES
#ifndef __LUA_THIS_H__
#define __LUA_THIS_H__

#include "luainc.h"
#include "LuaVirtualMachine.h"

// Sets the "this" global table that scripts use
namespace OpenOpcUa
{
	namespace UAScript
	{
		class CLuaThis
		{
		public:
			CLuaThis(CLuaVirtualMachine* vm, int iRef);

			virtual ~CLuaThis(void);


		protected:
			int m_iOldRef;
			CLuaVirtualMachine* m_pVm;
		};
	}
}
#endif // __LUA_THIS_H__