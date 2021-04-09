// ---------------------------------------------------------------------------
// FILE NAME            : LuaScript.h
// ---------------------------------------------------------------------------
// DESCRIPTION :
//
// Scripting base class
// 
// ---------------------------------------------------------------------------
// VERSION              : 1.00
// DATE                 : 1-Sep-2005
// AUTHOR               : Richard Shephard
// ---------------------------------------------------------------------------
// LIBRARY INCLUDE FILES

#ifndef __LUA_SCRIPT_BASE_H__
#define __LUA_SCRIPT_BASE_H__

#include "luainc.h"
#include "LuaVirtualMachine.h"
namespace OpenOpcUa
{
	namespace UAScript
	{
		class CLuaScript
		{
		public:
			CLuaScript(CLuaVirtualMachine* vm);
			virtual ~CLuaScript(void);

			// Compile script into Virtual Machine
			bool CompileFile(const char *strFilename);
			bool CompileBuffer(unsigned char *pbBuffer, size_t szLen);

			// Register function with Lua
			int RegisterFunction(const char *strFuncName);

			// Selects a Lua Script function to call
			bool SelectScriptFunction(const char *strFuncName);
			//void AddParam(int iInt);
			void AddParam(lua_Number luaNumber);
			void AddParam(char *string);

			// Runs the loaded script
			bool Go(int nReturns = 0);

			// Checks on Virtual Machine script
			bool ScriptHasFunction(const char *strScriptName);

			// Method indexing check
			int methods(void) { return m_nMethods; }


			// When the script calls a class method, this is called
			virtual OpcUa_Int32 ScriptCalling(CLuaVirtualMachine* vm, int iFunctionNumber) = 0;

			// When the script function has returns
			virtual void HandleReturns(CLuaVirtualMachine* vm, const char *strFunc) = 0;

			CLuaVirtualMachine* vm(void) { return m_pVm; }

		protected:
			int m_nMethods;
			CLuaVirtualMachine* m_pVm;
			int m_iThisRef;
			int m_nArgs;
			const char *m_strFunctionName;
		};
	}
}

#endif // __LUA_SCRIPT_BASE_H__