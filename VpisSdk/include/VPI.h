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
#include "VpiTypes.h"
#include "VpiDataValue.h"
using namespace UASubSystem;

using namespace VpiBuiltinType;
//typedef Vpi_UInt32 Vpi_StatusCode;

#if defined(_OPCUA_VPIEXPORT)
  #define OPCUA_VPIEXPORT __declspec(dllexport)
#else
  #define OPCUA_VPIEXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef Vpi_StatusCode(__stdcall *PFUNCNOTIFYCALLBACK)(Vpi_UInt32 uiNoOfNotifiedObject, Vpi_NodeId* Id, Vpi_DataValue* pValue);
// Exported function
Vpi_StatusCode VpiGlobalStart(Vpi_String szSubSystemName, Vpi_NodeId SubsystemId, Vpi_Handle* hVpi);
Vpi_StatusCode VpiGlobalStop(Vpi_Handle hVpi) ;
Vpi_StatusCode VpiColdStart(Vpi_Handle hVpi);
Vpi_StatusCode VpiWarmStart(Vpi_Handle hVpi);
Vpi_StatusCode VpiReadValue(Vpi_Handle hVpi, Vpi_UInt32 UiNbOfValueRead, Vpi_NodeId* Ids, Vpi_DataValue** pValue);

Vpi_StatusCode VpiWriteValue(Vpi_Handle hVpi, Vpi_UInt32 UiNbOfValueWrite, Vpi_NodeId* Ids, Vpi_DataValue** ppValue);
Vpi_StatusCode VpiParseAddId(Vpi_Handle hVpi, Vpi_NodeId Id, Vpi_Byte Datatype, Vpi_UInt32 iNbElt, Vpi_Byte AccessRight, Vpi_String ParsedAddress);
Vpi_StatusCode VpiParseRemoveId(Vpi_Handle hVpi, Vpi_NodeId Id);
Vpi_StatusCode VpiSetNotifyCallback(Vpi_Handle hVpi,PFUNCNOTIFYCALLBACK lpCallbackNotify);
//////////////////////////////////////////////////////////////////////////////
// Helper function
Vpi_Boolean IsEqual(const Vpi_NodeId* pOne, const Vpi_NodeId* pTwo);

#ifdef __cplusplus
}
#endif

#define Vpi_Good 0x00000000 // doit correspondre a Vpi_Good
#define Vpi_GoodCallAgain 0x00A90000
#define Vpi_BadUnexpectedError 0x80010000
#define Vpi_BadOutOfMemory 0x80030000
#define Vpi_BadResourceUnavailable 0x80040000
#define Vpi_BadCommunicationError 0x80050000
#define Vpi_BadNothingToDo 0x800F0000
#define Vpi_BadNotSupported 0x803D0000
#define Vpi_BadInvalidArgument 0x80AB0000
#define Vpi_BadInvalidState 0x80AF0000
#define Vpi_BadWouldBlock 0x80B50000
#define Vpi_Bad 0x83000000
#define Vpi_BadNotFound 0x83010000
#define Vpi_AlreadyInitialized 0x83020000
#define Vpi_ParseError 0x83030000
#define Vpi_UnInitialized 0x83040000
#define Vpi_WarmStartNeed 0x83050000
#define Vpi_CriticalError 0x83060000
#define Vpi_ErrorReadOnly 0x83070000
#define Vpi_ErrorWriteOnly 0x83080000 
#define Vpi_ErrorOutOfMemory 0x83090000 
#define Vpi_GoodNonCriticalTimeout 0x830B0000
#define Vpi_BadInternalError 0x830C0000
#define Vpi_BadTooManyPosts 0x830D0000
#define Vpi_BadNotConnected 0x830F0000
#define Vpi_UncertainInitialValue 0x40920000

#define Vpi_VariantArrayType_Scalar 0x00
#define Vpi_VariantArrayType_Array  0x01
#define Vpi_VariantArrayType_Matrix 0x02

