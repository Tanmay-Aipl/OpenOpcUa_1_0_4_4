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
#include <opcua_stringtable.h>

/*============================================================================
 * OpcUa_StringTable_Initialize
 *===========================================================================*/
OpcUa_Void OpcUa_StringTable_Initialize(OpcUa_StringTable* a_pTable)
{
    if (a_pTable != OpcUa_Null)
    {
        OpcUa_MemSet(a_pTable, 0, sizeof(OpcUa_StringTable));
    }
}

/*============================================================================
 * OpcUa_StringTable_Clear
 *===========================================================================*/
OpcUa_Void OpcUa_StringTable_Clear(OpcUa_StringTable* a_pTable)
{
    if (a_pTable != OpcUa_Null)
    {
        OpcUa_UInt32 ii = 0;
        
        for (ii = 0; ii < a_pTable->Count; ii++)
        {
            OpcUa_String_Clear(&(a_pTable->Values[ii]));
        }

        OpcUa_Free(a_pTable->Values);       
        OpcUa_MemSet(a_pTable, 0, sizeof(OpcUa_StringTable));
    }
}

/*============================================================================
 * OpcUa_StringTable_EnsureCapacity
 *===========================================================================*/
OpcUa_StatusCode OpcUa_StringTable_EnsureCapacity(
    OpcUa_StringTable* a_pTable,
    OpcUa_UInt32       a_nCapacity)
{
    OpcUa_InitializeStatus(OpcUa_Module_String, "OpcUa_StringTable_EnsureCapacity");

    OpcUa_ReturnErrorIfArgumentNull(a_pTable);

    if (a_pTable->Length - a_pTable->Count < a_nCapacity)
    {
        OpcUa_Void* pTmp = OpcUa_Null;
        OpcUa_UInt32 nLength = a_pTable->Count + a_nCapacity;

        pTmp = (OpcUa_String *)OpcUa_ReAlloc(a_pTable->Values, nLength*sizeof(OpcUa_String));
        OpcUa_ReturnErrorIfAllocFailed(pTmp);

        a_pTable->Values = pTmp;
        OpcUa_MemSet(&a_pTable->Values[a_pTable->Count], 0, sizeof(OpcUa_String)*a_nCapacity);
        a_pTable->Length = nLength;
    }

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_StringTable_AddStringList
 *===========================================================================*/
OpcUa_StatusCode OpcUa_StringTable_AddStringList(
    OpcUa_StringTable* a_pTable,
    OpcUa_StringA*     a_pStrings,
    OpcUa_Boolean      a_bMakeCopy)
{
    OpcUa_UInt32 ii = 0;

    OpcUa_InitializeStatus(OpcUa_Module_String, "OpcUa_StringTable_AddStringList");

    OpcUa_ReturnErrorIfArgumentNull(a_pTable);
    OpcUa_ReturnErrorIfArgumentNull(a_pStrings);

    /* count the number of strings */
    for (ii = 0; a_pStrings[ii] != OpcUa_Null; ii++);

    /* allocate space for strings */
    uStatus = OpcUa_StringTable_EnsureCapacity(a_pTable, a_pTable->Count + ii + 10);
    OpcUa_GotoErrorIfBad(uStatus);

    /* add the strings */
    for (ii = 0; a_pStrings[ii] != OpcUa_Null; ii++)
    {
        if(a_bMakeCopy != OpcUa_False)
        {
            uStatus = OpcUa_String_AttachCopy(&(a_pTable->Values[a_pTable->Count+ii]), a_pStrings[ii]);
        }
        else
        {
            uStatus = OpcUa_String_AttachReadOnly(&(a_pTable->Values[a_pTable->Count+ii]), a_pStrings[ii]);
        }
        OpcUa_GotoErrorIfBad(uStatus);
    }

    /* update string count */
    a_pTable->Count += ii;

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* remove any strings that were added */
    for (ii = a_pTable->Count; ii < a_pTable->Length; ii++)
    {
        OpcUa_String_Clear(&(a_pTable->Values[ii]));
    }

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_StringTable_AddStrings
 *===========================================================================*/
OpcUa_StatusCode OpcUa_StringTable_AddStrings(
    OpcUa_StringTable* a_pTable,
    OpcUa_String*      a_pStrings,
    OpcUa_UInt32       a_nNoOfStrings)
{
    OpcUa_UInt32 ii = 0;

    OpcUa_InitializeStatus(OpcUa_Module_String, "OpcUa_StringTable_AddStrings");

    OpcUa_ReturnErrorIfArgumentNull(a_pTable);

    /* nothing to do for empty lists */
    if (a_nNoOfStrings == 0 || a_pStrings == OpcUa_Null)
    {
        OpcUa_ReturnStatusCode;
    }

    /* allocate space for strings */
    uStatus = OpcUa_StringTable_EnsureCapacity(a_pTable, a_pTable->Count + a_nNoOfStrings + 10);
    OpcUa_GotoErrorIfBad(uStatus);

    /* add the strings */
    for (ii = 0; ii < a_nNoOfStrings; ii++)
    {
        OpcUa_String_Initialize(&(a_pTable->Values[a_pTable->Count+ii]));

        uStatus = OpcUa_String_StrnCpy(  
            &(a_pTable->Values[a_pTable->Count+ii]), 
            &(a_pStrings[ii]),
            OpcUa_String_StrSize(&(a_pStrings[ii])));

        OpcUa_GotoErrorIfBad(uStatus);
    }

    /* update string count */
    a_pTable->Count += ii;

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* remove any strings that were added */
    for (ii = a_pTable->Count; ii < a_pTable->Length; ii++)
    {
        OpcUa_String_Clear(&(a_pTable->Values[ii]));
    }

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_StringTable_FindIndex
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_StringTable_FindIndex(
    OpcUa_StringTable* a_pTable,
    OpcUa_String*      a_pString,
    OpcUa_Int32*       a_pIndex)
{
    OpcUa_UInt32 ii = 0;
    OpcUa_UInt32 uLength = 0;
    
    OpcUa_InitializeStatus(OpcUa_Module_String, "OpcUa_StringTable_AddStrings");

    OpcUa_ReturnErrorIfArgumentNull(a_pTable);
    OpcUa_ReturnErrorIfArgumentNull(a_pString);
    OpcUa_ReturnErrorIfArgumentNull(a_pIndex);

    *a_pIndex = -1;

    uLength = OpcUa_String_StrSize(a_pString);

    for (ii = 0; ii < a_pTable->Count; ii++)
    {
        if (OpcUa_String_StrnCmp(&(a_pTable->Values[ii]), a_pString, uLength, OpcUa_False) == 0)
        {
            *a_pIndex = ii;
            break;
        }
    }

    OpcUa_GotoErrorIfTrue(ii == a_pTable->Count, OpcUa_BadNotFound);

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    *a_pIndex = -1;

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_StringTable_FindString
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_StringTable_FindString(
    OpcUa_StringTable* a_pTable,
    OpcUa_Int32        a_nIndex,
    OpcUa_String*      a_pString)
{
    OpcUa_InitializeStatus(OpcUa_Module_String, "OpcUa_StringTable_FindString");

    OpcUa_ReturnErrorIfArgumentNull(a_pTable);
    OpcUa_ReturnErrorIfArgumentNull(a_pString);

    OpcUa_String_Initialize(a_pString);

    /* check if index is in range */
    if (a_nIndex < 0 || a_nIndex >= (OpcUa_Int32)a_pTable->Count)
    {
        uStatus = OpcUa_GoodNoData;
        OpcUa_ReturnStatusCode;
    }

    /* return a readonly reference to the string */    
    uStatus = OpcUa_String_AttachReadOnly(a_pString, OpcUa_String_GetRawString(&(a_pTable->Values[a_nIndex])));
    OpcUa_GotoErrorIfBad(uStatus);

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    OpcUa_String_Clear(a_pString);

    OpcUa_FinishErrorHandling;
}
