/*****************************************************************************
	  Author
		©. Michel Condemine, 4CE Industry (2010-2012)
	  
	  Contributors


	This software is a computer program whose purpose is to 
			implement behavior describe in the OPC UA specification.
		see wwww.opcfoundation.org for more details about OPC.
	This software is governed by the CeCILL-C license under French law and
	abiding by the rules of distribution of free software.  You can  use, 
	modify and/ or redistribute the software under the terms of the CeCILL-C
	license as circulated by CEA, CNRS and INRIA at the following URL
	"http://www.cecill.info". 

	As a counterpart to the access to the source code and  rights to copy,
	modify and redistribute granted by the license, users are provided only
	with a limited warranty  and the software's author,  the holder of the
	economic rights,  and the successive licensors  have only  limited
	liability. 

	In this respect, the user's attention is drawn to the risks associated
	with loading,  using,  modifying and/or developing or reproducing the
	software by the user in light of its specific status of free software,
	that may mean  that it is complicated to manipulate,  and  that  also
	therefore means  that it is reserved for developers  and  experienced
	professionals having in-depth computer knowledge. Users are therefore
	encouraged to load and test the software's suitability as regards their
	requirements in conditions enabling the security of their systems and/or 
	data to be ensured and,  more generally, to use and operate it in the 
	same conditions as regards security. 

	The fact that you are presently reading this means that you have had
		knowledge of the CeCILL-C license and that you accept its terms.

*****************************************************************************/

#include <opcua.h>
#include <opcua_p_os.h>
#include <opcua_string.h>

#include <opcua_guid.h>

/* Format string used in conversion from GUID to string. */
#if OPCUA_GUID_STRING_USE_CURLYBRACE
#define OPCUA_GUID_FORMATSTRING_OUT "{%08lx-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}"
#else /* OPCUA_GUID_STRING_USE_CURLYBRACE */
#define OPCUA_GUID_FORMATSTRING_OUT "%08lx-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x"
#endif /* OPCUA_GUID_STRING_USE_CURLYBRACE */

/* Format string used in conversion from string to GUID. */
#define OPCUA_GUID_FORMATSTRING_IN  "%08lx-%04hx-%04hx-%02hx%02hx-%02hx%02hx%02hx%02hx%02hx%02hx"
OpcUa_Guid OpcUa_Guid_Null = { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };
/*
#if !OPCUA_USE_STATIC_PLATFORM_INTERFACE
#define OpcUa_P_Guid_Create OpcUa_ProxyStub_g_PlatformLayerCalltable->GuidCreate
#endif
*/
/*============================================================================
 * CreateGuid
 *===========================================================================*/
OpcUa_Guid* OpcUa_Guid_Create(OpcUa_Guid* a_pGuid)
{
	if(a_pGuid == OpcUa_Null)
	{
		return OpcUa_Null;
	}

	//a_pGuid = OpcUa_P_Guid_Create(a_pGuid);


#ifndef _GUID_CREATE_NOT_AVAILABLE
	if (UuidCreate((UUID*)a_pGuid) != RPC_S_OK)
	{
		/* Error */
		a_pGuid = OpcUa_Null;
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

/*============================================================================
 * OpcUa_Guid_ToString
 *===========================================================================*/
OpcUa_CharA* OpcUa_Guid_ToStringA(  OpcUa_Guid*     a_pGuid, 
									OpcUa_CharA*    a_pchGuid)
{
	if((a_pGuid == OpcUa_Null) || (a_pchGuid == OpcUa_Null))
	{
		return OpcUa_Null;
	}

#if 1
	OpcUa_SPrintfA( a_pchGuid,
#if OPCUA_USE_SAFE_FUNCTIONS
					OPCUA_GUID_LEXICAL_LENGTH,
#endif /* OPCUA_USE_SAFE_FUNCTIONS */
					"{%08lx-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}", 
					a_pGuid->Data1,
					a_pGuid->Data2,
					a_pGuid->Data3,
					a_pGuid->Data4[0], a_pGuid->Data4[1],
					a_pGuid->Data4[2], a_pGuid->Data4[3], a_pGuid->Data4[4],
					a_pGuid->Data4[5], a_pGuid->Data4[6], a_pGuid->Data4[7]);
#endif
	return a_pchGuid;
}

/*============================================================================
 * GetStringFromGuid
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Guid_ToString(   OpcUa_Guid*     a_pGuid, 
										OpcUa_String**  a_ppString)
{
	OpcUa_CharA pRawString[OPCUA_GUID_LEXICAL_LENGTH + 1] = {0};

	OpcUa_DeclareErrorTraceModule(OpcUa_Module_Guid);

	OpcUa_ReturnErrorIfArgumentNull(a_ppString);
	OpcUa_ReturnErrorIfArgumentNull(a_pGuid);
	
	*a_ppString = OpcUa_Null;

#if 1
	OpcUa_SPrintfA( pRawString,
#if OPCUA_USE_SAFE_FUNCTIONS
					OPCUA_GUID_LEXICAL_LENGTH,
#endif /* OPCUA_USE_SAFE_FUNCTIONS */
					"{%08lx-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}", 
					a_pGuid->Data1,
					a_pGuid->Data2,
					a_pGuid->Data3,
					a_pGuid->Data4[0], a_pGuid->Data4[1],
					a_pGuid->Data4[2], a_pGuid->Data4[3], a_pGuid->Data4[4],
					a_pGuid->Data4[5], a_pGuid->Data4[6], a_pGuid->Data4[7]);
#endif
	
	return OpcUa_String_CreateNewString(pRawString,
										OPCUA_GUID_LEXICAL_LENGTH,
										0, 
										OpcUa_True,
										OpcUa_True,
										a_ppString);
}

/*============================================================================
 * GetGuidFromString
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Guid_FromString( OpcUa_CharA*    a_pText, 
										OpcUa_Guid*     a_pGuid)
{
	OpcUa_Int32     nScanned    = 0;
	OpcUa_UInt16    Byte0       = 0;
	OpcUa_UInt16    Byte1       = 0;
	OpcUa_UInt16    Byte2       = 0;
	OpcUa_UInt16    Byte3       = 0;
	OpcUa_UInt16    Byte4       = 0;
	OpcUa_UInt16    Byte5       = 0;
	OpcUa_UInt16    Byte6       = 0;
	OpcUa_UInt16    Byte7       = 0;

	if(     (a_pText == OpcUa_Null)
		||  (a_pGuid == OpcUa_Null))
	{
		return OpcUa_BadInvalidArgument;
	}

	if(a_pText[0] == '\0')
	{
		/* empty string */
		return OpcUa_BadInvalidArgument;
	}
	else
	{
		/* test for and skip optional brace */
		if(a_pText[0] == '{')
		{
			a_pText++;
		}
	}

	nScanned = OpcUa_SScanfA(   a_pText,
								OPCUA_GUID_FORMATSTRING_IN,
								&a_pGuid->Data1,
								&a_pGuid->Data2,
								&a_pGuid->Data3,
								&Byte0, &Byte1,
								&Byte2, &Byte3, 
								&Byte4, &Byte5, 
								&Byte6, &Byte7);

	if(nScanned != 11)
	{
		return OpcUa_BadInvalidArgument;
	}

	a_pGuid->Data4[0] = (OpcUa_UCharA)Byte0;
	a_pGuid->Data4[1] = (OpcUa_UCharA)Byte1;
	a_pGuid->Data4[2] = (OpcUa_UCharA)Byte2;
	a_pGuid->Data4[3] = (OpcUa_UCharA)Byte3;
	a_pGuid->Data4[4] = (OpcUa_UCharA)Byte4;
	a_pGuid->Data4[5] = (OpcUa_UCharA)Byte5;
	a_pGuid->Data4[6] = (OpcUa_UCharA)Byte6;
	a_pGuid->Data4[7] = (OpcUa_UCharA)Byte7;

	return OpcUa_Good;
}

/*============================================================================
 * OpcUa_Guid_IsEqual
 *===========================================================================*/
OpcUa_Boolean OpcUa_Guid_IsEqual(   OpcUa_Guid* a_pGuid1, 
									OpcUa_Guid* a_pGuid2)
{
	if(a_pGuid1 == a_pGuid2)
	{
		return OpcUa_True;
	}

	if(a_pGuid1 == OpcUa_Null || a_pGuid2 == OpcUa_Null)
	{
		return OpcUa_False;
	}

	if(OpcUa_MemCmp(a_pGuid1, a_pGuid2, sizeof(OpcUa_Guid)) == 0)
	{
		return OpcUa_True;
	}

	return OpcUa_False;
}

/*============================================================================
 * IsNullGuid
 *===========================================================================*/
OpcUa_Boolean OpcUa_Guid_IsNull(OpcUa_Guid* a_pGuid)
{
	if(a_pGuid == OpcUa_Null)
	{
		return OpcUa_False;
	}

	if(OpcUa_MemCmp(a_pGuid, &OpcUa_Guid_Null, sizeof(OpcUa_Guid)) == 0)
	{
		return OpcUa_True;
	}

	return OpcUa_False;
}

/*============================================================================
 * OpcUa_Guid_Copy
 *===========================================================================*/
OpcUa_StatusCode OpcUa_Guid_Copy(   OpcUa_Guid*  a_pSource,
									OpcUa_Guid** a_ppDestination)
{
	OpcUa_ReturnErrorIfArgumentNull(a_ppDestination);
	OpcUa_ReturnErrorIfArgumentNull(a_pSource);

	*a_ppDestination = (OpcUa_Guid*)OpcUa_Alloc(sizeof(OpcUa_Guid));
	OpcUa_ReturnErrorIfAllocFailed((*a_ppDestination));

	OpcUa_MemCpy(   *a_ppDestination,
					sizeof(OpcUa_Guid),
					a_pSource,
					sizeof(OpcUa_Guid));

	return OpcUa_Good;
}
