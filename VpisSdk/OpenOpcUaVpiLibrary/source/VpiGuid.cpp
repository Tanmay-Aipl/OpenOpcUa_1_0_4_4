//**************************************************************************
//
//  Copyright (c) 4CE Industry 1999-2014, All Rights Reserved
//
//**************************************************************************
//
//  Filename   :  VpiGuid.cpp
//  $Author    :  Michel Condemine
//
//  Description:  This file is part of the OpenOpcUaLibrary. 
//                This compoment and all related file are not Open Source
//				  This must be use with the autorisation of Michel Condemine
//**************************************************************************
/*
 *
 * Permission is hereby granted, for a commerciale use of this 
 * software and associated documentation files (the "Software")
 *
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 * ======================================================================*/
#include "stdafx.h"
#include "VpiGuid.h"
#include "VpiOs.h"
#ifndef _GNUC_
#include "Rpc.h"
#endif
using namespace VpiBuiltinType;
/*============================================================================
* Vpi_Guid_Copy
*===========================================================================*/
Vpi_StatusCode Vpi_Guid_CopyTo(Vpi_Guid*  a_pSource,
	Vpi_Guid** a_ppDestination)
{
	Vpi_StatusCode uStatus = Vpi_Good;
	if (a_ppDestination)
	{
		if (a_pSource)
		{
			*a_ppDestination = (Vpi_Guid*)Vpi_Alloc(sizeof(Vpi_Guid));
			if ((*a_ppDestination))
			{
				Vpi_MemCpy(*a_ppDestination,
					sizeof(Vpi_Guid),
					a_pSource,
					sizeof(Vpi_Guid));
			}
			else
				uStatus = Vpi_BadOutOfMemory;
		}
		else
			uStatus = Vpi_BadInvalidArgument;
	}
	else
		uStatus = Vpi_BadInvalidArgument;
	return Vpi_Good;
}

/*============================================================================
* CreateGuid
*===========================================================================*/
Vpi_Guid* Vpi_Guid_Create(Vpi_Guid* a_pGuid)
{
	if (a_pGuid == Vpi_Null)
	{
		return Vpi_Null;
	}

	//a_pGuid = OpcUa_P_Guid_Create(a_pGuid);


#ifndef _GUID_CREATE_NOT_AVAILABLE
	if (UuidCreate((UUID*)a_pGuid) != RPC_S_OK)
	{
		/* Error */
		a_pGuid =Vpi_Null;
		return a_pGuid;
	}

	/* Good */
	return a_pGuid;
#else
	unsigned int *data = (unsigned int*)a_pGuid;
	int chunks = 16 / sizeof(unsigned int);
	static const int intbits = sizeof(int) * 8;
	static int randbits = 0;
	if (!randbits)
	{
		int max = RAND_MAX;
		do
		{
			++randbits;
		} while ((max = max >> 1));
		srand(GetTickCount());
		rand(); /* Skip first */
	}

	while (chunks--)
	{
		unsigned int randNumber = 0;
		int filled;
		for (filled = 0; filled < intbits; filled += randbits)
			randNumber |= rand() << filled;
		*(data + chunks) = randNumber;
	}

	a_pGuid->Data4[0] = (a_pGuid->Data4[0] & 0x3F) | 0x80; /* UV_DCE */
	a_pGuid->Data3 = (a_pGuid->Data3 & 0x0FFF) | 0x4000;   /* UV_Random */

	return a_pGuid;
#endif

}