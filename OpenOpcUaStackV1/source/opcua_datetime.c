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
#include <opcua_datetime.h>
#if !OPCUA_USE_STATIC_PLATFORM_INTERFACE
#define OpcUa_P_DateTime_UtcNow                 OpcUa_ProxyStub_g_PlatformLayerCalltable->UtcNow
#define OpcUa_P_DateTime_GetTimeOfDay           OpcUa_ProxyStub_g_PlatformLayerCalltable->GetTimeOfDay
#define OpcUa_P_DateTime_GetDateTimeFromString  OpcUa_ProxyStub_g_PlatformLayerCalltable->GetDateTimeFromString
#define OpcUa_P_DateTime_GetStringFromDateTime  OpcUa_ProxyStub_g_PlatformLayerCalltable->GetStringFromDateTime
#endif
/*============================================================================*/
OpcUa_StatusCode OpcUa_DateTime_GetTimeOfDay(OpcUa_TimeVal* a_pValue)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_DateTime);
    OpcUa_ReturnErrorIfArgumentNull(a_pValue);

    OpcUa_P_DateTime_GetTimeOfDay(a_pValue);
    
    return OpcUa_Good;
}

/*============================================================================*/
OpcUa_StatusCode OpcUa_DateTime_GetDateTimeFromString(  OpcUa_StringA   a_pchDateTimeString, 
                                                        OpcUa_DateTime* a_pDateTime)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_DateTime);
    OpcUa_ReturnErrorIfArgumentNull(a_pchDateTimeString);
    OpcUa_ReturnErrorIfArgumentNull(a_pDateTime);

    return OpcUa_P_DateTime_GetDateTimeFromString(a_pchDateTimeString, a_pDateTime);
}

/*============================================================================*/
OpcUa_StatusCode OpcUa_DateTime_GetStringFromDateTime(  OpcUa_DateTime  a_dateTime, 
                                                        OpcUa_StringA   a_pBuffer, 
                                                        OpcUa_UInt32    a_uLength)
{
    OpcUa_DeclareErrorTraceModule(OpcUa_Module_DateTime);
    OpcUa_ReturnErrorIfArgumentNull(a_pBuffer);

    if(a_uLength < 25)
    {
        return OpcUa_BadInvalidArgument;
    }

    return OpcUa_P_DateTime_GetStringFromDateTime(a_dateTime, a_pBuffer, a_uLength);
}
