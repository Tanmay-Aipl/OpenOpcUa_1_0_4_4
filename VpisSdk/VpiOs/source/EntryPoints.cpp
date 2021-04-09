#include <stdafx.h>

using namespace UASubSystem;
using namespace std;
#include <vector>
#include <string>
CVpiInternalData* pVpiHandle=NULL; // handle du Vpi utilisé pour dialoguer avec le serveur
vector<CVpiInternalData*> gVpiInternalData; // liste de l'ensemble des elements interne pris en charge par cette DLL(VPI). 
											// Chaque appel à global start devra ajouter une instance dans ce vecteur
/*============================================================================
// Fonction non exportée utilisé. 
// Elles sont utilisées en interne pour la réalisation du Vpi
 *===========================================================================*/
/*============================================================================
 * The GetVpiHandle function returns a CVpiInternalData based on a Vpi_Handle
 *===========================================================================*/
Vpi_StatusCode GetVpiHandle(Vpi_Handle* pHandle,CVpiInternalData** pOutInternalData)
{
	Vpi_StatusCode uStatus=Vpi_BadNotFound;

	if (pHandle == NULL)
	{
		return uStatus;
	}
	for (Vpi_UInt32 i = 0; gVpiInternalData.size();i++)
	{
		CVpiInternalData* pInternal = gVpiInternalData.at(i);
		if (pInternal->GetVpiHandle()==pHandle)
		{
			*pOutInternalData=pInternal;
			uStatus=Vpi_Good;
			break;
		}
	}
	return uStatus;
}
/*============================================================================
 * The Vpi_DateTime_UtcNow function (returns the time in Vpi_DateTime format)
 *===========================================================================*/
Vpi_DateTime Vpi_DateTime_UtcNow()
{
	FILETIME ftTime;


#if defined(__GNUC__)

	Vpi_DateTime tmpDateTime;
	struct timeval tv;
	LONGLONG ll = EPOCH_DIFF;

	gettimeofday(&tv,NULL);
	ll += tv.tv_sec;
	ll *= 10000000LL;
	ll += tv.tv_usec * 10;

	tmpDateTime.dwHighDateTime = (DWORD) ll;
	tmpDateTime.dwLowDateTime = ll >>32;
	return tmpDateTime;
#else
	Vpi_DateTime tmpDateTime;

	GetSystemTimeAsFileTime(&ftTime);

	tmpDateTime.dwHighDateTime = (Vpi_UInt32)ftTime.dwHighDateTime;
	tmpDateTime.dwLowDateTime  = (Vpi_UInt32)ftTime.dwLowDateTime;

	return tmpDateTime;
#endif
}
HINSTANCE g_hInstance;
LRESULT CALLBACK MouseTracker(int code, WPARAM wParam, LPARAM lParam);

DWORD    g_dwLastTick = 0;    // tick time of last input event
Vpi_StatusCode VpiGlobalStart(Vpi_String szSubSystemName, Vpi_NodeId SubsystemId,Vpi_Handle* hVpi) 
{
	(void)szSubSystemName;
	(void)SubsystemId;
	Vpi_StatusCode uStatus;
	CVpiInternalData* pInternal=NULL;
	uStatus=GetVpiHandle(hVpi,&pInternal);
	if (uStatus==Vpi_BadNotFound)
	{
		CVpiInternalData* phVpiInternalData=NULL;
		phVpiInternalData=new CVpiInternalData();
		// Save SubsystemName andSubSystemId
		phVpiInternalData->SetSubSystemName(szSubSystemName);
		phVpiInternalData->SetSubsystemId(SubsystemId);
		// Load extra configuration parameter if needed
		phVpiInternalData->LoadConfigurationFile();
		// SetVpiData to the internalData instance
		phVpiInternalData->SetVpiHandle((Vpi_Handle)phVpiInternalData);
		gVpiInternalData.push_back(phVpiInternalData);
		*hVpi  = (Vpi_Handle) phVpiInternalData;
		uStatus=Vpi_Good;
	}
	else
		uStatus=Vpi_Bad;
	return uStatus;
}
Vpi_StatusCode VpiGlobalStop(Vpi_Handle hVpi)
{
	Vpi_StatusCode uStatus = Vpi_Good;
	if (!hVpi)
		uStatus = Vpi_Bad;
	else
	{
		CVpiInternalData* pVpiInternal = (CVpiInternalData*)hVpi;
	}
	return uStatus;
}
Vpi_StatusCode VpiColdStart(Vpi_Handle hVpi) 
{
	Vpi_StatusCode uStatus=Vpi_Good;
	if (!hVpi)
		uStatus=Vpi_Bad;
	else
	{
		CVpiInternalData* pVpi=(CVpiInternalData*)hVpi;
		(void*)pVpi;
	}
	return uStatus;
}

Vpi_StatusCode VpiWarmStart(Vpi_Handle hVpi) 
{
	Vpi_StatusCode uStatus=Vpi_Good;
	if (!hVpi)
		uStatus=Vpi_Bad;
	else
	{
		CVpiInternalData* pVpi=(CVpiInternalData*)hVpi;
		(void*)pVpi;
	}
	return uStatus;
}

Vpi_StatusCode VpiReadValue(Vpi_Handle hVpi, Vpi_UInt32 uiNbOfValueRead, Vpi_NodeId* Ids, Vpi_DataValue** pValue)
{	
	Vpi_StatusCode uStatus=Vpi_Good;
	if (!hVpi)
		uStatus=Vpi_Bad;
	else
	{
		if (uiNbOfValueRead > 0)
		{
			for (Vpi_Int32 i = 0; i < uiNbOfValueRead; i++)
			{
				CVpiInternalData* pVpi = (CVpiInternalData*)hVpi;
				Vpi_DataValue* pDataValue=(*pValue);
				
				CSourceObject* pObject = pVpi->GetSourceObject(Ids[i]);
				if (pObject)
				{
					CVpiDataValue* apValue = pObject->GetValue();
					apValue->SetStatusCode(Vpi_UncertainInitialValue);
					//Vpi_Variant aVariant = pDataValue[i].Value;
					if (_stricmp(pObject->GetAddress().strContent, "MouseX") == 0)
					{
						// Recupération des coordonnées du point
						POINT pt;
						GetCursorPos(&pt);
						pDataValue[i].Value.Value.Int32 = pt.x;
						pDataValue[i].Value.Datatype = VpiType_UInt32;
						pDataValue[i].Value.ArrayType = 0;
						// mise en place de la qualité
						pDataValue[i].StatusCode=Vpi_Good;
					}
					if (_stricmp(pObject->GetAddress().strContent, "MouseY") == 0)
					{
						// Recupération des coordonnées du point
						POINT pt;
						GetCursorPos(&pt);
						pDataValue[i].Value.Value.Int32 = pt.y;
						pDataValue[i].Value.Datatype = VpiType_UInt32;
						pDataValue[i].Value.ArrayType = 0;
						// mise en place de la qualité
						pDataValue[i].StatusCode = Vpi_Good;
					}
					if (_stricmp(pObject->GetAddress().strContent, "OEM-ID") == 0)
					{
						// update pObject
						SYSTEM_INFO siSysInfo;
						// Copy the hardware information to the SYSTEM_INFO structure. 
						GetSystemInfo(&siSysInfo);
						pDataValue[i].Value.Value.UInt32 = siSysInfo.dwOemId;
						pDataValue[i].Value.Datatype = VpiType_UInt32;
						pDataValue[i].Value.ArrayType = 0;
						// mise en place de la qualité
						pDataValue[i].StatusCode = Vpi_Good;
						//apValue->SetValue(aVariant);
						//pObject->SetValue(apValue);
					}
					if (_stricmp(pObject->GetAddress().strContent, "NoProcessor") == 0)
					{
						SYSTEM_INFO siSysInfo;
						// Copy the hardware information to the SYSTEM_INFO structure. 
						GetSystemInfo(&siSysInfo);
						pDataValue[i].Value.Value.UInt32 = siSysInfo.dwNumberOfProcessors;
						pDataValue[i].Value.Datatype = VpiType_UInt32;
						pDataValue[i].Value.ArrayType = 0;
						// mise en place de la qualité
						pDataValue[i].StatusCode = Vpi_Good;
					}
					if (_stricmp(pObject->GetAddress().strContent, "ProcessArch") == 0)
					{
						SYSTEM_INFO siSysInfo;
						// Copy the hardware information to the SYSTEM_INFO structure. 
						GetSystemInfo(&siSysInfo);
						// accès a la valeur
						if (siSysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL)
						{
							pDataValue[i].Value.Value.String.strContent = (char*)malloc(10);
							memset((pDataValue[i].Value.Value.String.strContent), 0, 10);
							pDataValue[i].Value.Value.String.uLength = 9;
							memcpy(pDataValue[i].Value.Value.String.strContent, "INTEL-x86", 9);
						}
						if (siSysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
						{
							pDataValue[i].Value.Value.String.strContent = (char*)malloc(5);
							pDataValue[i].Value.Value.String.uLength = 5;
							memcpy(pDataValue[i].Value.Value.String.strContent, "AMD64", 5);
						}
						if (siSysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
						{
							pDataValue[i].Value.Value.String.strContent = (char*)malloc(13);
							pDataValue[i].Value.Value.String.uLength = 13;
							memcpy(pDataValue[i].Value.Value.String.strContent, "INTEL-Itanium", 13);
						}
						if (siSysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_UNKNOWN)
						{
							pDataValue[i].Value.Value.String.strContent = (char*)malloc(7);
							pDataValue[i].Value.Value.String.uLength = 7;
							memcpy(pDataValue[i].Value.Value.String.strContent, "Unknown", 7);
						}
						// mise en place du source timestamp
						//Vpi_DateTime dateTime = Vpi_DateTime_UtcNow();

						pObject->SetValue(apValue);
					}
					// mise en place du source timestamp
					Vpi_DateTime dateTime = Vpi_DateTime_UtcNow();
					pDataValue[i].SourceTimestamp=dateTime;
					// Transfert back to the VpiCache
					if (pDataValue[i].StatusCode == Vpi_Good)
						apValue->SetValue(&(pDataValue[i].Value));
					apValue->SetStatusCode(pDataValue[i].StatusCode);
					apValue->SetSourceTimestamp(pDataValue[i].SourceTimestamp);
					std::string strTime;
					strTime.empty();
				}
				else
					pDataValue[i].StatusCode = Vpi_BadNotFound;
			}
		}
		else
			uStatus = Vpi_BadNothingToDo;
	}
	return uStatus;
}

Vpi_StatusCode VpiWriteValue(Vpi_Handle hVpi, Vpi_UInt32 UiNbOfValueWrite, Vpi_NodeId* Ids, Vpi_DataValue** ppValue)
{
	(void)Ids;
	Vpi_StatusCode uStatus=Vpi_Good;
	if (!hVpi)
		uStatus=Vpi_Bad;
	else
	{
		CVpiInternalData* pVpi=(CVpiInternalData*)hVpi;
		(void*)pVpi;
	}
	return uStatus;
}

Vpi_StatusCode VpiParseAddId(Vpi_Handle hVpi,Vpi_NodeId Id, Vpi_Byte Datatype, Vpi_UInt32 iNbElt,Vpi_Byte AccessRight, Vpi_String ParsedAddress )
{
	(void*)AccessRight;
	Vpi_StatusCode uStatus=Vpi_ParseError;
	if (!hVpi)
		uStatus=Vpi_Bad;
	else
	{
		CVpiInternalData* pVpi=(CVpiInternalData*)hVpi;
		if (std::string(ParsedAddress.strContent)==std::string("MouseX"))
		{
			CSourceObject* pNewSource=new CSourceObject(Datatype,iNbElt);
			pNewSource->SetAddress(ParsedAddress);
			pNewSource->SetNodeId(Id);
			pVpi->m_SourceObjects.push_back(pNewSource);
			uStatus=Vpi_Good;
		}
		if (std::string(ParsedAddress.strContent)==std::string("MouseY"))
		{
			CSourceObject* pNewSource=new CSourceObject(Datatype,iNbElt);
			pNewSource->SetAddress(ParsedAddress);
			pNewSource->SetNodeId(Id);
			pVpi->m_SourceObjects.push_back(pNewSource);
			uStatus=Vpi_Good;
		}
		if (std::string(ParsedAddress.strContent)==std::string("OEM-ID"))
		{
			CSourceObject* pNewSource=new CSourceObject(Datatype,iNbElt);
			pNewSource->SetAddress(ParsedAddress);
			pNewSource->SetNodeId(Id);
			pVpi->m_SourceObjects.push_back(pNewSource);
			uStatus=Vpi_Good;
		}
		if (std::string(ParsedAddress.strContent)==std::string("NoProcessor"))
		{
			CSourceObject* pNewSource=new CSourceObject(Datatype,iNbElt);
			pNewSource->SetAddress(ParsedAddress);
			pNewSource->SetNodeId(Id);
			pVpi->m_SourceObjects.push_back(pNewSource);
			uStatus=Vpi_Good;
		}
	}
	return uStatus;
}

Vpi_StatusCode VpiParseRemoveId(Vpi_Handle hVpi,Vpi_NodeId Id) 
{
	(void)Id;
	Vpi_StatusCode uStatus=Vpi_Good;
	if (!hVpi)
		uStatus=Vpi_Bad;
	else
	{
		CVpiInternalData* pVpi=(CVpiInternalData*)hVpi;
		(void*)pVpi;
	}
	return uStatus;
}
//typedef Vpi_StatusCode (__stdcall *PFUNCNOTIFYCALLBACK)(Vpi_NodeId Id, CVpiDataValue* pValue);
Vpi_StatusCode VpiSetNotifyCallback(Vpi_Handle hVpi, PFUNCNOTIFYCALLBACK lpCallbackNotify)
{
	Vpi_StatusCode uStatus = Vpi_Good;
	if (!hVpi)
		uStatus = Vpi_Bad;
	else
	{
		CVpiInternalData* pVpiInternal = (CVpiInternalData*)hVpi;
	}
	return uStatus;
}
//============================================================================
// Utils::IsEqual ( pour les Vpi_NodeId
//============================================================================
Vpi_Boolean IsEqual(const Vpi_NodeId* pOne, const Vpi_NodeId* pTwo)
{
	if (pOne == pTwo)
	{
		return true;
	}

	if (pOne->IdentifierType != pTwo->IdentifierType)
	{
		return false;
	}
	
	if (pOne->NamespaceIndex != pTwo->NamespaceIndex)
	{
		return false;
	}

	// check the numeric value.
	if (pOne->IdentifierType == Vpi_IdentifierType_Numeric)
	{
		return pOne->Identifier.Numeric == pTwo->Identifier.Numeric;
	}

	// check the string value.
	if (pOne->IdentifierType == Vpi_IdentifierType_String)
	{
		return  strcmp(pOne->Identifier.String.strContent,pTwo->Identifier.String.strContent)==0;//Vpi_String_StrnCmp((Vpi_String*)&pOne->Identifier.String, (Vpi_String*)&pTwo->Identifier.String, OPCUA_STRING_LENDONTCARE, Vpi_False) == 0;
	}

	// check the guid value.
	if (pOne->IdentifierType == Vpi_IdentifierType_Guid)
	{
		return memcmp(pOne->Identifier.Guid, pTwo->Identifier.Guid, sizeof(Vpi_Guid)) == 0;
	}

	// check the opaque value.
	if (pOne->IdentifierType == Vpi_IdentifierType_Opaque)
	{
		if (pOne->Identifier.ByteString.Length != pTwo->Identifier.ByteString.Length)
		{
			return false;
		}

		return memcmp(pOne->Identifier.ByteString.Data, pTwo->Identifier.ByteString.Data, pOne->Identifier.ByteString.Length) == 0;
	}
		
	return false;
}


