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

	for (unsigned short ii=0;ii<gVpiInternalData.size();ii++)
	{
		CVpiInternalData* pInternal=gVpiInternalData.at(ii); 
		if (pInternal->GetVpiHandle()==pHandle)
		{
			*pOutInternalData=pInternal;
			uStatus=Vpi_Good;
			break;
		}
	}
	return uStatus;
}
//typedef Vpi_StatusCode (__stdcall *PFUNCNOTIFYCALLBACK)(Vpi_NodeId Id, CVpiDataValue* pValue);
Vpi_StatusCode VpiSetNotifyCallback(Vpi_Handle hVpi,
										  PFUNCNOTIFYCALLBACK lpCallbackNotify)
{
	Vpi_StatusCode uStatus=Vpi_Good;
	if (!hVpi)
		uStatus=Vpi_Bad;
	else
	{
		CVpiInternalData* pVpiInternal=(CVpiInternalData*)hVpi;
	}
	return uStatus;
}
Vpi_StatusCode VpiGlobalStop(Vpi_Handle hVpi) 
{
	Vpi_StatusCode uStatus=Vpi_Good;
	if (!hVpi)
		uStatus=Vpi_Bad;
	else
	{
	}
	return uStatus;
}

DWORD    g_dwLastTick = 0;    // tick time of last input event
Vpi_StatusCode VpiGlobalStart(Vpi_String szSubSystemName, Vpi_NodeId SubsystemId,Vpi_Handle* hVpi) 
{
	Vpi_StatusCode uStatus;
	CVpiInternalData* pInternal=NULL;
	(void)SubsystemId;
	(void)szSubSystemName;
	uStatus=GetVpiHandle(hVpi,&pInternal);
	if (uStatus==Vpi_BadNotFound)
	{
		CVpiInternalData* phVpiInternalData=NULL;
		phVpiInternalData=new CVpiInternalData();
		// SetVpiData to the internalData instance
		phVpiInternalData->SetVpiHandle((Vpi_Handle*)phVpiInternalData);
		gVpiInternalData.push_back(phVpiInternalData);
		*hVpi  = (Vpi_Handle*) phVpiInternalData;
		uStatus=Vpi_Good;
	}
	else
		uStatus=Vpi_Bad;
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
		(void)pVpi;
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
		(void)pVpi;
	}
	return uStatus;
}

Vpi_StatusCode VpiReadValue(Vpi_Handle hVpi, 
								  Vpi_UInt32 uiNbOfValueRead, 
								  Vpi_NodeId* Ids, Vpi_DataValue** pValue)
{	
	Vpi_StatusCode uStatus = Vpi_Good;
	if (!hVpi)
		uStatus = Vpi_Bad;
	else
	{
		if (uiNbOfValueRead > 0)
		{
			for (Vpi_Int32 i = 0; i < uiNbOfValueRead; i++)
			{
				CVpiInternalData* pVpi = (CVpiInternalData*)hVpi;
				CSourceObject* pObject = pVpi->GetSourceObject(Ids[i]);
				if (pObject)
				{
					CVpiDataValue* apValue = pObject->GetValue();
					apValue->SetStatusCode(Vpi_UncertainInitialValue);
				}
				else
					uStatus = Vpi_BadNotFound;
			}
		}
	}
	return uStatus;
}

Vpi_StatusCode VpiWriteValue(Vpi_Handle hVpi, 
								   Vpi_UInt32 UiNbOfValueWrite, 
								   Vpi_NodeId* Ids, 
								   Vpi_DataValue** ppValue)
{
	Vpi_StatusCode uStatus=Vpi_Good;
	(void)Ids;
	if (!hVpi)
		uStatus=Vpi_Bad;
	else
	{
		CVpiInternalData* pVpi=(CVpiInternalData*)hVpi;
		(void)pVpi;
	}
	return uStatus;
}

Vpi_StatusCode VpiParseAddId(Vpi_Handle hVpi,
								   Vpi_NodeId Id, 
								   Vpi_Byte Datatype, 
								   Vpi_UInt32 iNbElt,
								   Vpi_Byte AccessRight, 
								   Vpi_String ParsedAddress )
{
	Vpi_StatusCode uStatus=Vpi_Good;
	(void)AccessRight;
	if (!hVpi)
		uStatus=Vpi_Bad;
	else
	{

	}
	return uStatus;
}

Vpi_StatusCode VpiParseRemoveId(Vpi_Handle hVpi,Vpi_NodeId Id) 
{
	Vpi_StatusCode uStatus=Vpi_Good;
	(void)Id;
	if (!hVpi)
		uStatus=Vpi_Bad;
	else
	{
		CVpiInternalData* pVpi=(CVpiInternalData*)hVpi;
		(void)pVpi;
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
		return  strcmp(pOne->Identifier.String.strContent,pTwo->Identifier.String.strContent)==0;//Vpi_String_StrnCmp((Vpi_String*)&pOne->Identifier.String, (Vpi_String*)&pTwo->Identifier.String, Vpi_STRING_LENDONTCARE, Vpi_False) == 0;
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
