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
#pragma once
#include "VfiTypes.h"
#include "VfiDataTypes.h"
#include "VfiDataValue.h"
using namespace UAHistoricalAccess;
using namespace UABuiltinType;

typedef OpcUa_UInt32 OpcUa_Vfi_StatusCode;

#if defined(_OPCUA_VFIEXPORT)
  #define OPCUA_VFIEXPORT __declspec(dllexport)
#else
  #define OPCUA_VFIEXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

	typedef OpcUa_Vfi_StatusCode(__stdcall *PFUNCNOTIFY_HISTORYCALDATA_CALLBACK)(OpcUa_UInt32 uiNoOfNotifiedObject, OpcUa_NodeId* Id, OpcUa_DataValue* pValue);
// Exported function
OpcUa_Vfi_StatusCode VfiGlobalStart(OpcUa_String szVfiName, OpcUa_NodeId VfiId,OpcUa_Vfi_Handle** hVfi);
OpcUa_Vfi_StatusCode VfiColdStart(OpcUa_Vfi_Handle* hVfi);
OpcUa_Vfi_StatusCode VfiWarmStart(OpcUa_Vfi_Handle* hVfi);

OpcUa_Vfi_StatusCode VfiHistoryRead(OpcUa_Vfi_Handle* hVfi,
									OpcUa_NodeId Id, 
									OpcUa_DateTime dtFrom, 
									OpcUa_DateTime dtTo,
									OpcUa_UInt32* pUiNbOfValueToRead,
									OpcUa_DataValue** pValue);

OpcUa_Vfi_StatusCode VfiHistoryWrite(OpcUa_Vfi_Handle* hVfi,
									 OpcUa_NodeId Id, 
									 OpcUa_UInt32 uiNbOfValueToWrite,
									 UAHistoricalAccess::CVfiDataValue* pValue);
OpcUa_Vfi_StatusCode VfiHistoryUpdateValue(OpcUa_Vfi_Handle* hVfi,OpcUa_NodeId Id, UAHistoricalAccess::CVfiDataValue* pValue);
OpcUa_Vfi_StatusCode VfiParseAddId(OpcUa_Vfi_Handle* hVfi,OpcUa_NodeId Id, OpcUa_Byte Datatype, OpcUa_UInt32 iNbElt,OpcUa_Byte AccessRight, OpcUa_UInt16 uiHistoryMode, OpcUa_UInt32 uiRate );
OpcUa_Vfi_StatusCode VfiParseRemoveId(OpcUa_Vfi_Handle* hVpi,OpcUa_NodeId Id);
OpcUa_Vfi_StatusCode VfiSetNotifyCallback(OpcUa_Vfi_Handle* hVfi,PFUNCNOTIFY_HISTORYCALDATA_CALLBACK lpHistorycalDataCallbackNotify);
#ifdef __cplusplus
}
#endif
#define OpcUa_Vfi_Good 0x00000000 // doit correspondre a OpcUa_Vpi_Good
#define OpcUa_Vfi_BadOutOfRange 0x803C0000
#define OpcUa_Vfi_BadNotSupported 0x803D0000
#define OpcUa_Vfi_BadNotImplemented 0x80400000
#define OpcUa_Vfi_BadInvalidArgument 0x80AB0000
#define OpcUa_Vfi_Bad 0x83000000
#define OpcUa_Vfi_BadNotFound 0x83010000
#define OpcUa_Vfi_BadOutOfMemory 0x83090000 
#define OpcUa_Vfi_GoodNonCriticalTimeout 0x830B0000
#define OpcUa_Vfi_BadInternalError 0x830C0000
#define OpcUa_Vfi_BadTooManyPosts 0x830D0000