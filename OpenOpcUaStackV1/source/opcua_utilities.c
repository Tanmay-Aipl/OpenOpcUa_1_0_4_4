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
#include <opcua_utilities.h>
#include <opcua_p_os.h>
#if !OPCUA_USE_STATIC_PLATFORM_INTERFACE
#define OPCUA_P_QSORT           OpcUa_ProxyStub_g_PlatformLayerCalltable->qSort
#define OPCUA_P_BSEARCH         OpcUa_ProxyStub_g_PlatformLayerCalltable->bSearch
#define OPCUA_P_GETTICKCOUNT    OpcUa_ProxyStub_g_PlatformLayerCalltable->UtilGetTickCount
#define OPCUA_P_GETLASTERROR    OpcUa_ProxyStub_g_PlatformLayerCalltable->UtilGetLastError
#endif

/*============================================================================
 * Quick Sort
 *===========================================================================*/
OpcUa_StatusCode OpcUa_QSort(   OpcUa_Void*       a_pElements, 
                                OpcUa_UInt32      a_nElementCount, 
                                OpcUa_UInt32      a_nElementSize, 
                                OpcUa_PfnCompare* a_pfnCompare, 
                                OpcUa_Void*       a_pContext)
{
    if(     a_pElements     == OpcUa_Null
        ||  a_pfnCompare    == OpcUa_Null
        ||  a_nElementCount == 0
        ||  a_nElementSize  == 0)
    {
        return OpcUa_BadInvalidArgument;
    }

    OPCUA_P_QSORT(    a_pElements, 
                      a_nElementCount, 
                      a_nElementSize, 
                      a_pfnCompare, 
                      a_pContext);

    return OpcUa_Good;
}

/*============================================================================
 * Binary Search on sorted array
 *===========================================================================*/
OpcUa_Void* OpcUa_BSearch(  OpcUa_Void*       a_pKey, 
                            OpcUa_Void*       a_pElements, 
                            OpcUa_UInt32      a_nElementCount, 
                            OpcUa_UInt32      a_nElementSize, 
                            OpcUa_PfnCompare* a_pfnCompare, 
                            OpcUa_Void*       a_pContext)
{
    if(     a_pElements     == OpcUa_Null
        ||  a_pKey          == OpcUa_Null
        ||  a_pfnCompare    == OpcUa_Null
        ||  a_nElementCount == 0
        ||  a_nElementSize  == 0)
    {
        return OpcUa_Null;
    }

    return OPCUA_P_BSEARCH( a_pKey, 
                            a_pElements, 
                            a_nElementCount, 
                            a_nElementSize, 
                            a_pfnCompare, 
                            a_pContext);
}

/*============================================================================
 * Access to errno
 *===========================================================================*/
OpcUa_UInt32 OpcUa_GetLastError()
{
    return OPCUA_P_GETLASTERROR();
}

/*============================================================================
 * OpcUa_GetTickCount
 *===========================================================================*/
OpcUa_UInt32 OpcUa_GetTickCount()
{
    return OPCUA_P_GETTICKCOUNT();
}
/*============================================================================
 * Calculate DateTime Difference
 *===========================================================================*/
OpcUa_StatusCode OpcUa_P_GetDateTimeDiff( OpcUa_DateTime  a_Value1,
                                                        OpcUa_DateTime  a_Value2,
                                                        OpcUa_DateTime* a_pResult)
{
    unsigned long long   ullValue1 = 0;
    unsigned long long ullValue2 = 0;
    unsigned long long ullResult = 0;

    OpcUa_ReturnErrorIfArgumentNull(a_pResult);

    a_pResult->dwLowDateTime = (OpcUa_UInt32)0;
    a_pResult->dwHighDateTime = (OpcUa_UInt32)0;

    ullValue1 = a_Value1.dwHighDateTime;
    ullValue1 = (ullValue1 << 32) + a_Value1.dwLowDateTime;

    ullValue2 = a_Value2.dwHighDateTime;
    ullValue2 = (ullValue2 << 32) + a_Value2.dwLowDateTime;

    if(ullValue1 > ullValue2)
    {
        return OpcUa_BadInvalidArgument;
    }

    ullResult = ullValue2 - ullValue1;

    a_pResult->dwLowDateTime = (OpcUa_UInt32)(ullResult & 0x00000000FFFFFFFF);
    /* BEGIN MONTPELLIER WORKSHOP */
    a_pResult->dwHighDateTime = (OpcUa_UInt32)((ullResult & 0xFFFFFFFF00000000) >> 32);
    /* END MONTPELLIER WORKSHOP */

    return OpcUa_Good;
}
/*============================================================================
 * Calculate DateTime Difference In Seconds (Rounded)(a_Value2-a_Value1)
 *===========================================================================*/
OpcUa_StatusCode OPCUA_DLLCALL OpcUa_P_GetDateTimeDiffInSeconds32(  OpcUa_DateTime  a_Value1,
                                                                    OpcUa_DateTime  a_Value2,
                                                                    OpcUa_UInt32*   a_puResult)
{

    //unsigned long long ullValue1 = 0;
    //unsigned long long ullValue2 = 0;
	OpcUa_UInt64 ullResult = 0;

    OpcUa_ReturnErrorIfArgumentNull(a_puResult);

    *a_puResult = (OpcUa_UInt32)0;
	OpcUa_UInt64 ullValue1 = a_Value1.dwHighDateTime;
	ullValue1 <<= 32;
	ullValue1 += a_Value1.dwLowDateTime;
    //ullValue1 = a_Value1.dwHighDateTime;
    //ullValue1 = (ullValue1 << 32) + a_Value1.dwLowDateTime;

	OpcUa_UInt64 ullValue2 = a_Value2.dwHighDateTime;
	ullValue2 <<= 32;
	ullValue2 += a_Value2.dwLowDateTime;
    //ullValue2 = a_Value2.dwHighDateTime;
    //ullValue2 = (ullValue2 << 32) + a_Value2.dwLowDateTime;

    if(ullValue1 > ullValue2)
    {
        return OpcUa_BadInvalidArgument;
    }
	ullResult = (ullValue2 - ullValue1) / 10000000;
    //ullResult = (unsigned long long)((ullValue2 - ullValue1 + 5000000) / 10000000);
    // 
	*a_puResult = (OpcUa_UInt32)(ullResult & 0x00000000FFFFFFFF);
    if(ullResult > (unsigned long long)OpcUa_UInt32_Max)
    {
        return OpcUa_BadOutOfRange;
    }

   


    return OpcUa_Good;
}

//// Charge un bibliothèque
//// LibraryName = Nom de la librairie a charger
//// hInst = output handle associé a la librairie chargé
//OpcUa_StatusCode OPCUA_DLLCALL OpcUa_LoadLibrary(const OpcUa_String* LibraryName, void* hInst)
//{
//	OpcUa_StatusCode uStatus=OpcUa_Good;
//#ifdef WIN32
//	LPWSTR pStr=NULL;
//	OpcUa_String_AtoW(OpcUa_String_GetRawString(LibraryName),(OpcUa_CharW**)&pStr);
//	hInst=LoadLibrary(pStr);
//	if (!hInst)
//	{
//		DWORD dwError=GetLastError();
//		uStatus=OpcUa_Bad;
//	}
//	OpcUa_Free(pStr);
//#else
//	#ifdef _GNUC_
//		hInst=dlopen(OpcUa_String_GetRawString(LibraryName),RTLD_NOW);
//	#endif
//#endif
//	return uStatus;
//}