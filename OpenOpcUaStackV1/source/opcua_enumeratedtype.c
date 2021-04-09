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
#include <opcua_enumeratedtype.h>

/*============================================================================
 * OpcUa_EnumeratedType_FindName
 *===========================================================================*/
OpcUa_StatusCode OpcUa_EnumeratedType_FindName(
    OpcUa_EnumeratedType* a_pType,
    OpcUa_Int32           a_nValue,
    OpcUa_StringA*        a_pName)
{    
    OpcUa_UInt32 ii = 0;

    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "EnumeratedType_FindName");

    OpcUa_ReturnErrorIfArgumentNull(a_pType);
    OpcUa_ReturnErrorIfArgumentNull(a_pName);

    *a_pName = OpcUa_Null;

    for (ii = 0; a_pType->Values[ii].Name != OpcUa_Null; ii++)
    {
        if (a_pType->Values[ii].Value == a_nValue)
        {
            *a_pName = a_pType->Values[ii].Name;
            break;
        }
    }

    OpcUa_GotoErrorIfTrue(a_pType->Values[ii].Name == OpcUa_Null, OpcUa_BadInvalidArgument);

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}

/*============================================================================
 * OpcUa_EnumeratedType_FindValue
 *===========================================================================*/
OPCUA_EXPORT OpcUa_StatusCode OpcUa_EnumeratedType_FindValue(
    OpcUa_EnumeratedType* a_pType,
    OpcUa_StringA         a_sName,
    OpcUa_Int32*          a_pValue)
{
    OpcUa_UInt32 ii = 0;

    OpcUa_InitializeStatus(OpcUa_Module_Serializer, "EnumeratedType_FindValue");

    OpcUa_ReturnErrorIfArgumentNull(a_pType);
    OpcUa_ReturnErrorIfArgumentNull(a_sName);
    OpcUa_ReturnErrorIfArgumentNull(a_pValue);

    *a_pValue = 0;

    for (ii = 0; a_pType->Values[ii].Name != OpcUa_Null; ii++)
    {
        if (OpcUa_StrCmpA(a_pType->Values[ii].Name, a_sName) == 0)
        {
            *a_pValue = a_pType->Values[ii].Value;
            break;
        }
    }

    OpcUa_GotoErrorIfTrue(a_pType->Values[ii].Name == OpcUa_Null, OpcUa_BadInvalidArgument);

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;

    /* nothing to do */

    OpcUa_FinishErrorHandling;
}
