// ---------------------------------------------------------------------------
// FILE NAME            : LuaScript.cpp
// ---------------------------------------------------------------------------
// DESCRIPTION :
//
// Simple debugging routines
// 
// ---------------------------------------------------------------------------
// VERSION              : 1.00
// DATE                 : 1-Sep-2005
// AUTHOR               : Richard Shephard
// Windows and OpenOpcUa integration michel Condemine
// ---------------------------------------------------------------------------
#include "stdafx.h"
// LIBRARY INCLUDE FILES
#include <assert.h>
#include "luainc.h"
#include "LuaScript.h"
#include "LuaRestoreStack.h"
#include "LuaThis.h"
// ---------------------------------------------------------------------------

#define BEGIN_LUA_CHECK(vm)   lua_State *state = (lua_State *) m_pVm->GetLuaState(); \
                              if (m_pVm->Ok ()) { 
#define END_LUA_CHECK         }
using namespace OpenOpcUa;
using namespace UAScript;

//============================================================================
// int LuaCallback
//---------------------------------------------------------------------------
// Lua C-API calling that figures out which object to hand over to
//
// Parameter   Dir      Description
// ---------   ---      -----------
// lua         IN       State variable
//
// Return
// ------
// Number of return varaibles on the stack
//
// Comments
// --------
// This is the function lua calls for each C-Function that is
// registered with lua. At the time of registering the function
// with lua, we make lua record the method "number" so we can
// know what method was actually called. The lua stack is the
// following structure:
// 0: 'this' (table)
// 1 - ...: parameters passed in
//
//============================================================================
static int LuaCallback (lua_State *lua)
{
   // Locate the psudo-index for the function number
   int iNumberIdx = lua_upvalueindex (1);
   int nRetsOnStack = 0;

   bool fSuccess = false;

   // Check for the "this" table
   if (lua_istable (lua, 1))
   {
      // Found the "this" table. The object pointer is at the index 0
      lua_rawgeti (lua, 1, 0);
      
      if (lua_islightuserdata (lua, -1))
      {
         // Found the pointer, need to cast it
         CLuaScript *pThis = (CLuaScript *) lua_touserdata (lua, -1);

         // Get the method index
         int iMethodIdx = (int) lua_tonumber (lua, iNumberIdx);

         // Check that the method is correct index
         assert (!(iMethodIdx > pThis->methods ()));

         // Reformat the stack so our parameters are correct
         // Clean up the "this" table
         lua_remove (lua, 1);
         // Clean up the pThis pointer
         lua_remove (lua, -1);

         // Call the class
         nRetsOnStack = pThis->ScriptCalling (pThis->vm (), iMethodIdx);

         fSuccess = true;
      }
   }

   if (fSuccess == false)
   {
      lua_pushstring (lua, "LuaCallback -> Failed to call the class function");
      lua_error (lua);
   }

   // Number of return variables
   return nRetsOnStack;
}


//============================================================================
// CLuaScript::CLuaScript
//---------------------------------------------------------------------------
// Constructor. Sets up the lua stack and the "this" table
//
// Parameter            Dir      Description
// ---------            ---      -----------
// CLuaVirtualMachine   IN       VM to run on
//
// Return
// ------
// None.
//
//============================================================================
CLuaScript::CLuaScript (CLuaVirtualMachine* vm)
	: m_pVm(vm), m_nMethods(0), m_iThisRef(0), m_nArgs(0)
{
	lua_State *state = (lua_State *)m_pVm->GetLuaState(); 
	if (m_pVm->Ok())
	{
		// Create a reference to the "this" table. Each reference is unique
		lua_newtable(state);
		m_iThisRef = luaL_ref(state, LUA_REGISTRYINDEX);

		// Save the "this" table to index 0 of the "this" table
		CLuaRestoreStack rs(vm);
		lua_rawgeti(state, LUA_REGISTRYINDEX, m_iThisRef);
		lua_pushlightuserdata(state, (void *) this);
		lua_rawseti(state, -2, 0);
	}
}

//============================================================================
// CLuaScript::~CLuaScript
//---------------------------------------------------------------------------
// Destructor
//
// Parameter   Dir      Description
// ---------   ---      -----------
// None.
//
// Return
// ------
// None.
//
//============================================================================
CLuaScript::~CLuaScript (void)
{
	CLuaRestoreStack rs(m_pVm);

	BEGIN_LUA_CHECK(m_pVm)
      // Get the reference "this" table
      lua_rawgeti (state, LUA_REGISTRYINDEX, m_iThisRef);

      // Clear index 0
      lua_pushnil (state);
      lua_rawseti (state, -2, 0);
   END_LUA_CHECK
}

//============================================================================
// bool CLuaScript::CompileBuffer
//---------------------------------------------------------------------------
// Compiles a given buffer
//
// Parameter   Dir      Description
// ---------   ---      -----------
// pbBuffer    IN       Data buffer
// szLen       IN       Length of buffer
//
// Return
// ------
// Success
//
//============================================================================
bool CLuaScript::CompileBuffer (unsigned char *pbBuffer, size_t szLen)
{
	bool bResult = false;
	if (pbBuffer)
	{
		if (szLen != 0)
		{
			if (m_pVm->Ok())
			{
				// Make sure we have the correct "this" table
				CLuaThis luaThis(m_pVm, m_iThisRef);
				bResult = m_pVm->RunBuffer(pbBuffer, szLen);
			}
		}
	}
	return bResult;
}

//============================================================================
// bool CLuaScript::CompileBuffer
//---------------------------------------------------------------------------
// Compiles a given file
//
// Parameter   Dir      Description
// ---------   ---      -----------
// strFilename IN       File name to compile
//
// Return
// ------
// Success
//
//============================================================================
bool CLuaScript::CompileFile (const char *strFilename)
{
	bool bResult = false;
	if (strFilename)
	{
		if (m_pVm->Ok())
		{
			// Make sure we have the correct "this" table
			CLuaThis luaThis(m_pVm, m_iThisRef); // 
			bResult = m_pVm->RunFile(strFilename);
		}
	}
	return bResult;
}

//============================================================================
// int CLuaScript::RegisterFunction
//---------------------------------------------------------------------------
// Registers a function with Lua
//
// Parameter   Dir      Description
// ---------   ---      -----------
// strFuncName IN       Function name
//
// Return
// ------
// Success
//
//============================================================================
int CLuaScript::RegisterFunction (const char *strFuncName)
{
	int iMethodIdx = -1;
	if (strFuncName != NULL)
	{
		if (m_pVm->Ok())
		{


			CLuaRestoreStack rs(m_pVm);

			lua_State *state = (lua_State *)m_pVm->GetLuaState();
			if (m_pVm->Ok())
			{
				iMethodIdx = ++m_nMethods;

				// Register a function with the lua script. Added it to the "this" table
				lua_rawgeti(state, LUA_REGISTRYINDEX, m_iThisRef);

				// Push the function and parameters
				lua_pushstring(state, strFuncName);
				lua_pushnumber(state, (lua_Number)iMethodIdx);
				lua_pushcclosure(state, LuaCallback, 1);
				lua_settable(state, -3);

			}
		}
	}
	return iMethodIdx;
}

//============================================================================
// bool CLuaScript::SelectScriptFunction
//---------------------------------------------------------------------------
// Selects a script function to run
//
// Parameter   Dir      Description
// ---------   ---      -----------
// strFuncName IN       Function name
//
// Return
// ------
// Success
//
//============================================================================
bool CLuaScript::SelectScriptFunction (const char *strFuncName)
{
   bool fSuccess = true;
   if (strFuncName != NULL)
   {
	   if (m_pVm->Ok())
	   {
		   BEGIN_LUA_CHECK(m_pVm)
			   // Look up function name
			   lua_rawgeti(state, LUA_REGISTRYINDEX, m_iThisRef);
		   lua_pushstring(state, strFuncName);
		   lua_rawget(state, -2);
		   lua_remove(state, -2);

		   // Put the "this" table back
		   lua_rawgeti(state, LUA_REGISTRYINDEX, m_iThisRef);

		   // Check that we have a valid function
		   if (!lua_isfunction(state, -2))
		   {
			   fSuccess = false;
			   lua_pop(state, 2);
		   }
		   else
		   {
			   m_nArgs = 0;
			   m_strFunctionName = strFuncName;
		   }
		   END_LUA_CHECK
	   }
   }
   else
	   fSuccess = false;
   return fSuccess;
}

//============================================================================
// bool CLuaScript::ScriptHasFunction
//---------------------------------------------------------------------------
// Checks to see if a function exists
//
// Parameter      Dir      Description
// ---------      ---      -----------
// strScriptName  IN       Function name
//
// Return
// ------
// Success
//
//============================================================================
bool CLuaScript::ScriptHasFunction (const char *strScriptName)
{
	bool fFoundFunc = false;
	if (strScriptName) 
	{
		if (m_pVm->Ok())
		{
			assert(strScriptName != NULL && "CLuaScript::ScriptHasFunction -> strScriptName == NULL");
			assert(m_pVm->Ok() && "VM Not OK");

			CLuaRestoreStack rs(m_pVm);

			lua_State *state = (lua_State *)m_pVm->GetLuaState();
			if (m_pVm->Ok())
			{
				lua_rawgeti(state, LUA_REGISTRYINDEX, m_iThisRef);
				lua_pushstring(state, strScriptName);
				lua_rawget(state, -2);
				lua_remove(state, -2);

				if (lua_isfunction(state, -1))
				{
					fFoundFunc = true;
				}
			}
		}
	}
	return fFoundFunc;
}

//============================================================================
// void CLuaScript::AddParam
//---------------------------------------------------------------------------
// Adds a parameter to the parameter list
//
// Parameter   Dir      Description
// ---------   ---      -----------
// string      IN       string param
//
// Return
// ------
// None.
//
//============================================================================
void CLuaScript::AddParam (char *string)
{
	if (string)
	{
		if (m_pVm->Ok())
		{
			lua_State *state = (lua_State *)m_pVm->GetLuaState();
			if (m_pVm->Ok())
			{
				lua_pushstring(state, string);
				++m_nArgs;
			}
		}
	}
}

//============================================================================
// void CLuaScript::AddParam
//---------------------------------------------------------------------------
// Adds a parameter to the parameter list
//
// Parameter   Dir      Description
// ---------   ---      -----------
// fFloat      IN       float param
//
// Return
// ------
// None.
//
//============================================================================
void CLuaScript::AddParam(lua_Number luaNumber)
{
	if (m_pVm->Ok())
	{
		lua_State *state = (lua_State *)m_pVm->GetLuaState();
		if (m_pVm->Ok())
		{
			lua_pushnumber(state, luaNumber);
			++m_nArgs;
		}
	}
}

//============================================================================
// bool CLuaScript::Go
//---------------------------------------------------------------------------
// Runs the selected script function
//
// Parameter   Dir      Description
// ---------   ---      -----------
// nReturns    IN       Number of expected returns
//
// Return
// ------
// None.
//
//============================================================================
bool CLuaScript::Go (int nReturns /* = 0 */)
{
	bool fSuccess=false;
	if (m_pVm->Ok())
	{
		// At this point there should be a parameters and a function on the
		// Lua stack. Each function get a "this" parameter as default and is
		// pushed onto the stack when the method is selected

		fSuccess = m_pVm->CallFunction(m_nArgs + 1, nReturns);

		if (fSuccess == true && nReturns > 0)
		{
			// Check for returns
			HandleReturns(m_pVm, m_strFunctionName);
			lua_pop((lua_State *)m_pVm, nReturns);
		}
	}
	return fSuccess;
}