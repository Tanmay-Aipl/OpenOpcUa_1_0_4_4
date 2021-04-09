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
#include "stdafx.h"
#include "BuildInfo.h"
using namespace OpenOpcUa;
using namespace UASharedLib;
CBuildInfo::CBuildInfo(void)
{
	m_pInternalBuildInfo=(OpcUa_BuildInfo*)OpcUa_Alloc(sizeof(OpcUa_BuildInfo));
	if (m_pInternalBuildInfo)
	{
		OpcUa_BuildInfo_Initialize(m_pInternalBuildInfo);
		// initialisation des chaines encapsulé dans le m_pInternalBuildInfo
		OpcUa_DateTime_Initialize(&(m_pInternalBuildInfo->BuildDate));
		OpcUa_String* pszSeparatorUrl1 = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
		if (pszSeparatorUrl1)
		{
			OpcUa_String_Initialize(pszSeparatorUrl1);
			OpcUa_String_AttachCopy(pszSeparatorUrl1, "-");
		}
		OpcUa_String_Initialize(&(m_pInternalBuildInfo->BuildNumber));
		OpcUa_String_StrnCpy(&(m_pInternalBuildInfo->BuildNumber), pszSeparatorUrl1, 1);
		//
		OpcUa_String_Initialize(&(m_pInternalBuildInfo->ManufacturerName));
		OpcUa_String_StrnCpy(&(m_pInternalBuildInfo->ManufacturerName), pszSeparatorUrl1, 1);
		//
		OpcUa_String_Initialize(&(m_pInternalBuildInfo->ProductName));
		OpcUa_String_StrnCpy(&(m_pInternalBuildInfo->ProductName), pszSeparatorUrl1, 1);
		//
		OpcUa_String_Initialize(&(m_pInternalBuildInfo->ProductUri));
		OpcUa_String_StrnCpy(&(m_pInternalBuildInfo->ProductUri), pszSeparatorUrl1, 1);
		//
		OpcUa_String_Initialize(&(m_pInternalBuildInfo->SoftwareVersion));
		OpcUa_String_StrnCpy(&(m_pInternalBuildInfo->SoftwareVersion), pszSeparatorUrl1, 1);
		OpcUa_Free(pszSeparatorUrl1);
	}
}

CBuildInfo::CBuildInfo(OpcUa_BuildInfo* pBuildInfo)
{
	if (pBuildInfo)
		m_pInternalBuildInfo = pBuildInfo;
}
CBuildInfo::~CBuildInfo(void)
{
	if (m_pInternalBuildInfo)
		OpcUa_Free(m_pInternalBuildInfo);
}
// ProductUri
OpcUa_String   CBuildInfo::GetProductUri()
{
	return m_pInternalBuildInfo->ProductUri;
}
void CBuildInfo::SetProductUri(OpcUa_String* pString)
{
	OpcUa_Int32 iSize=OpcUa_String_StrLen(pString);
	OpcUa_String_StrnCpy(&(m_pInternalBuildInfo->ProductUri),pString,iSize);
}
// ManufacturerName
OpcUa_String   CBuildInfo::GetManufacturerName()
{
	return m_pInternalBuildInfo->ManufacturerName;
}
void CBuildInfo::SetManufacturerName(OpcUa_String* pString)
{
	OpcUa_Int32 iSize=OpcUa_String_StrLen(pString);
	OpcUa_String_StrnCpy(&(m_pInternalBuildInfo->ManufacturerName),pString,iSize);
}
// ProductName
OpcUa_String   CBuildInfo::GetProductName()
{
	return m_pInternalBuildInfo->ProductName;
}
void CBuildInfo::SetProductName(OpcUa_String* pString)
{
	OpcUa_Int32 iSize=OpcUa_String_StrLen(pString);
	OpcUa_String_StrnCpy(&(m_pInternalBuildInfo->ProductName),pString,iSize);
}
// SoftwareVersion
OpcUa_String   CBuildInfo::GetSoftwareVersion()
{
	return m_pInternalBuildInfo->SoftwareVersion;
}
void CBuildInfo::SetSoftwareVersion(OpcUa_String* pString)
{
	OpcUa_Int32 iSize=OpcUa_String_StrLen(pString);
	OpcUa_String_StrnCpy(&(m_pInternalBuildInfo->SoftwareVersion),pString,iSize);
}
// BuildNumber
OpcUa_String   CBuildInfo::GetBuildNumber()
{
	return m_pInternalBuildInfo->BuildNumber;
}
void CBuildInfo::SetBuildNumber(OpcUa_String* pString)
{
	OpcUa_String* aString=(OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(aString);
	OpcUa_Int32 iSize=OpcUa_String_StrLen(pString);
	OpcUa_String_CreateNewString(OpcUa_String_GetRawString(pString),
								iSize,
								iSize,
								true,
								false,
								&aString);
	OpcUa_String_StrnCpy(&(m_pInternalBuildInfo->BuildNumber),aString,iSize);
}
// BuildDate
OpcUa_DateTime CBuildInfo::GetBuildDate()
{
	if (m_pInternalBuildInfo)
	 return m_pInternalBuildInfo->BuildDate;
	else
	{
		OpcUa_DateTime aNullDateTime;
		OpcUa_DateTime_Initialize(&aNullDateTime);
		return aNullDateTime;
	}
}
void CBuildInfo::SetBuildDate(OpcUa_DateTime dtVal)
{
	if (m_pInternalBuildInfo)
		m_pInternalBuildInfo->BuildDate=dtVal;
	else
		throw std::exception();
}
OpcUa_BuildInfo* CBuildInfo::GetInternalBuildInfo() 
{ 
	return m_pInternalBuildInfo; 
}
void CBuildInfo::SetInternalBuildInfo(OpcUa_BuildInfo* pInternalBuildInfo)
{
	if (pInternalBuildInfo)
	{
		if (m_pInternalBuildInfo)
			OpcUa_MemCpy(m_pInternalBuildInfo, sizeof(pInternalBuildInfo), pInternalBuildInfo, sizeof(pInternalBuildInfo));;
	}
}