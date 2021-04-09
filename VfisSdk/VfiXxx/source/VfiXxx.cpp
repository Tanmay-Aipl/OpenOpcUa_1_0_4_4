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
// VfiCsv.cpp : définit les fonctions exportées pour l'application DLL.
//

#include "stdafx.h"
#include "InternalVfiRecord.h"
#include "VfiInternal.h"
using namespace UAHistoricalAccess;
OpcUa_Vfi_StatusCode VfiGlobalStart(OpcUa_String szVfiName, OpcUa_NodeId VfiId,OpcUa_Vfi_Handle* hVfi)
{
	OpcUa_Vfi_StatusCode uStatus=OpcUa_Vfi_Good;

	return uStatus;
}
OpcUa_Vfi_StatusCode VfiColdStart(OpcUa_Vfi_Handle hVfi)
{
	OpcUa_Vfi_StatusCode uStatus=OpcUa_Vfi_Good;

	return uStatus;
}
OpcUa_Vfi_StatusCode VfiWarmStart(OpcUa_Vfi_Handle hVfi)
{
	OpcUa_Vfi_StatusCode uStatus=OpcUa_Vfi_Good;

	return uStatus;
}

/// <summary>
/// Vfis the history read.
/// </summary>
/// <param name="hVfi">The h vfi.</param>
/// <param name="Id">The identifier.</param>
/// <param name="dtFrom">The dt from.</param>
/// <param name="dtTo">The dt to.</param>
/// <param name="pUiNbOfValueRead">The p UI nb of value read.</param>
/// <param name="pValue">The p value.</param>
/// <returns></returns>
OpcUa_Vfi_StatusCode VfiHistoryRead(OpcUa_Vfi_Handle hVfi,
										 OpcUa_NodeId Id, 
										 OpcUa_DateTime dtFrom, 
										 OpcUa_DateTime dtTo,
										 OpcUa_UInt32* pUiNbOfValueRead,
										 OpcUa_DataValue** pValue)
{
	OpcUa_Vfi_StatusCode uStatus=OpcUa_Vfi_Good;
	//return uStatus;
	CVfiInternalData* pVfiInternal=(CVfiInternalData*)hVfi;
	if (pVfiInternal)
	{
		CVfiXxx* pVfiXxx=pVfiInternal->GetVfiXxx();
		if (pVfiXxx)
		{
		}
		else
			uStatus=OpcUa_Vfi_BadInternalError;
	}
	else
		uStatus=OpcUa_Vfi_BadInvalidArgument;
	return uStatus;
}
/// <summary>
/// Vfis the history write.
/// </summary>
/// <param name="hVfi">The h vfi.</param>
/// <param name="Id">The identifier.</param>
/// <param name="uiNbOfValueToWrite">The UI nb of value to write.</param>
/// <param name="pValue">The p value.</param>
/// <returns></returns>
OpcUa_Vfi_StatusCode VfiHistoryWrite(OpcUa_Vfi_Handle hVfi, 
									 OpcUa_NodeId Id, 
									 OpcUa_UInt32 uiNbOfValueToWrite, 
									 OpcUa_DataValue* pValue)
{
	OpcUa_Vfi_StatusCode uStatus=OpcUa_Vfi_Good;
	//return uStatus;
	CVfiInternalData* pVfiInternal=(CVfiInternalData*)hVfi;
	if (pVfiInternal)
	{
		CVfiXxx* pVfiXxx=pVfiInternal->GetVfiXxx();
		if (pVfiXxx)
		{

		}
		else
			uStatus=OpcUa_Vfi_BadInternalError;
	}
	else
		uStatus=OpcUa_Vfi_BadInvalidArgument;
	return uStatus;
}
OpcUa_Vfi_StatusCode VfiHistoryUpdateValue(OpcUa_Vfi_Handle hVfi, 
										   OpcUa_NodeId Id, 
										   OpcUa_DataValue* pValue)
{
	OpcUa_Vfi_StatusCode uStatus=OpcUa_Vfi_Good;
	return uStatus;
}
OpcUa_Vfi_StatusCode VfiParseAddId(OpcUa_Vfi_Handle hVfi,
								   OpcUa_NodeId Id, 
								   OpcUa_Byte Datatype, 
								   OpcUa_UInt32 iNbElt,
								   OpcUa_Byte AccessRight, 
								   OpcUa_UInt16 uiHistoryMode, 
								   OpcUa_UInt32 uiRate )
{
	OpcUa_Vfi_StatusCode uStatus=OpcUa_Vfi_Good;
	return uStatus;
}

OpcUa_Vfi_StatusCode VfiParseRemoveId(OpcUa_Vfi_Handle hVpi,OpcUa_NodeId Id)
{
	OpcUa_Vfi_StatusCode uStatus=OpcUa_Vfi_Good;
	return uStatus;
}
OpcUa_Vfi_StatusCode VfiSetNotifyCallback(OpcUa_Vfi_Handle hVfi,PFUNCNOTIFY_HISTORYCALDATA_CALLBACK lpHistorycalDataCallbackNotify)
{
	OpcUa_Vfi_StatusCode uStatus=OpcUa_Vfi_Good;
	return uStatus;
}

///////////////////////////////////////////////////////////////////////////////////////////
//
//

CVfiXxx::CVfiXxx()
{
	m_pInternalData=OpcUa_Null;
}
CVfiXxx::~CVfiXxx()
{
}
void CVfiXxx::SetVfiInternalData(CVfiInternalData* pInternalData)
{
	m_pInternalData=pInternalData;
}
void CVfiXxx::SetUserName(std::string userName)
{
}
void CVfiXxx::SetPwd(std::string userName)
{
}