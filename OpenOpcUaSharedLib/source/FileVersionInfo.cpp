/********************************************************************
*
* Copyright (C) 1999-2000 Sven Wiegand
* Copyright (C) 2000-2001 ToolsCenter
* 
* This file is free software; you can redistribute it and/or
* modify, but leave the headers intact and do not remove any 
* copyrights from the source.
*
* If you have further questions, suggestions or bug fixes, visit 
* our homepage
*
*    http://www.ToolsCenter.org
*
********************************************************************/

#include "stdafx.h"
#ifdef WIN32
#include "FileVersionInfo.h"
//#pragma message("automatic link to VERSION.LIB")
//#pragma comment(lib, "version.lib")
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//-------------------------------------------------------------------
// CFileVersionInfo
//-------------------------------------------------------------------
using namespace OpenOpcUa;
using namespace UASharedLib;
CFileVersionInfo::CFileVersionInfo()
{
	ZeroMemory(&m_FileInfo, sizeof(VS_FIXEDFILEINFO));
	OpcUa_String_Initialize(&m_strCompanyName);
	OpcUa_String_Initialize(&m_strFileDescription);
	OpcUa_String_Initialize(&m_strFileVersion);
	OpcUa_String_Initialize(&m_strInternalName);
	OpcUa_String_Initialize(&m_strLegalCopyright);
	OpcUa_String_Initialize(&m_strOriginalFileName);
	OpcUa_String_Initialize(&m_strProductName);
	OpcUa_String_Initialize(&m_strProductVersion);
	OpcUa_String_Initialize(&m_strComments);
	OpcUa_String_Initialize(&m_strLegalTrademarks);
	OpcUa_String_Initialize(&m_strPrivateBuild);
	OpcUa_String_Initialize(&m_strSpecialBuild);
}


CFileVersionInfo::~CFileVersionInfo()
{
	OpcUa_String_Clear(&m_strCompanyName);
	OpcUa_String_Clear(&m_strFileDescription);
	OpcUa_String_Clear(&m_strFileVersion);
	OpcUa_String_Clear(&m_strInternalName);
	OpcUa_String_Clear(&m_strLegalCopyright);
	OpcUa_String_Clear(&m_strOriginalFileName);
	OpcUa_String_Clear(&m_strProductName);
	OpcUa_String_Clear(&m_strProductVersion);
	OpcUa_String_Clear(&m_strComments);
	OpcUa_String_Clear(&m_strLegalTrademarks);
	OpcUa_String_Clear(&m_strPrivateBuild);
	OpcUa_String_Clear(&m_strSpecialBuild);
}


BOOL CFileVersionInfo::GetTranslationId(LPVOID lpData, UINT unBlockSize, WORD wLangId, DWORD &dwId, BOOL bPrimaryEnough/*= FALSE*/)
{
	LPWORD lpwData;
	if (lpData)
	{
		for (lpwData = (LPWORD)lpData; (LPBYTE)lpwData < ((LPBYTE)lpData) + unBlockSize; lpwData += 2)
		{
			if (*lpwData == wLangId)
			{
				dwId = *((DWORD*)lpwData);
				return TRUE;
			}
		}

		if (!bPrimaryEnough)
			return FALSE;

		for (lpwData = (LPWORD)lpData; (LPBYTE)lpwData < ((LPBYTE)lpData) + unBlockSize; lpwData += 2)
		{
			if (((*lpwData) & 0x00FF) == (wLangId & 0x00FF))
			{
				dwId = *((DWORD*)lpwData);
				return TRUE;
			}
		}
	}
	return FALSE;
}


OpcUa_Boolean CFileVersionInfo::Create(HMODULE hModule /*= NULL*/)
{
	OpcUa_Boolean bResult = OpcUa_False;
	char* FileName = (char*)OpcUa_Alloc(sizeof(char)*_MAX_PATH);
	if (FileName)
	{
		if (GetModuleFileName(hModule, FileName, _MAX_PATH))
			bResult = Create(FileName);
		OpcUa_Free(FileName);
		FileName = OpcUa_Null;
	}
	return bResult;
}



// Function name   : CFileVersionInfo::Create
// Description     : Initialise l'ensemble des variables de classe
// Return type     : BOOL 
// Argument        : wchar_t*  lpszFileName

OpcUa_Boolean CFileVersionInfo::Create(char*  lpszFileName)
{
	//Reset();

	DWORD	dwHandle=0;
	DWORD	dwFileVersionInfoSize = GetFileVersionInfoSize((LPCTSTR)lpszFileName, &dwHandle);
	if (!dwFileVersionInfoSize)
	{
		DWORD dwError = GetLastError();
		OpcUa_Trace(OPCUA_TRACE_EXTRA_LEVEL_ERROR,"GetFileVersionInfoSize failed 0x%05x\n", dwError);
		return FALSE;
	}

	void* lpData = malloc(dwFileVersionInfoSize);
	if (!lpData)
		return FALSE;

	try
	{
		if (!GetFileVersionInfo((LPTSTR)lpszFileName, dwHandle, dwFileVersionInfoSize, lpData))
			throw FALSE;

		// catch default information
		LPVOID	lpInfo;
		UINT		unInfoLen;
		if (VerQueryValue(lpData, _T("\\"), &lpInfo, &unInfoLen))
		{
			if (unInfoLen != sizeof(VS_FIXEDFILEINFO))
				throw std::exception("(unInfoLen != sizeof(m_FileInfo)");
			if (unInfoLen == sizeof(VS_FIXEDFILEINFO))
				memcpy(&m_FileInfo, lpInfo, unInfoLen);
		}

		// find best matching language and codepage
		VerQueryValue(lpData, _T("\\VarFileInfo\\Translation"), &lpInfo, &unInfoLen);
		
		DWORD	dwLangCode = 0;
		if (!GetTranslationId(lpInfo, unInfoLen, GetUserDefaultLangID(), dwLangCode, FALSE))
		{
			if (!GetTranslationId(lpInfo, unInfoLen, GetUserDefaultLangID(), dwLangCode, TRUE))
			{
				if (!GetTranslationId(lpInfo, unInfoLen, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), dwLangCode, TRUE))
				{
					if (!GetTranslationId(lpInfo, unInfoLen, MAKELANGID(LANG_ENGLISH, SUBLANG_NEUTRAL), dwLangCode, TRUE))
						// use the first one we can get
						dwLangCode = *((DWORD*)lpInfo);
				}
			}
		}
		
		char buffer[256];
		memset(buffer,0,256);
		OpcUa_SPrintfA(buffer,"\\StringFileInfo\\%04X%04X\\", dwLangCode&0x0000FFFF, (dwLangCode&0xFFFF0000)>>16);
		std::string	strSubBlock(buffer);
		// catch string table
		std::string strTmp;
		strTmp=strSubBlock;
		strTmp.append("CompanyName");
		// conversion en wide char

		// extraction
		if (VerQueryValue(lpData, (OpcUa_CharA*)(strTmp.c_str()), &lpInfo, &unInfoLen))
			OpcUa_String_AttachCopy(&m_strCompanyName, (char*)lpInfo);
		strTmp.clear();
		strTmp=strSubBlock;
		strTmp.append("FileDescription");

		if (VerQueryValue(lpData, (OpcUa_CharA*)(strTmp.c_str()), &lpInfo, &unInfoLen))
			OpcUa_String_AttachCopy(&m_strFileDescription, (char*)lpInfo);
			
		strTmp.clear();
		strTmp=strSubBlock;
		strTmp.append("FileVersion");

		if (VerQueryValue(lpData, (OpcUa_CharA*)(strTmp.c_str()), &lpInfo, &unInfoLen))
			OpcUa_String_AttachCopy(&m_strFileVersion, (char*)lpInfo);
			
		strTmp.clear();
		strTmp=strSubBlock;
		strTmp.append("InternalName");
		if (VerQueryValue(lpData, (OpcUa_CharA*)(strTmp.c_str()), &lpInfo, &unInfoLen))
			OpcUa_String_AttachCopy(&m_strInternalName, (char*)lpInfo);

		strTmp.clear();
		strTmp=strSubBlock;
		strTmp.append("LegalCopyright");
		if (VerQueryValue(lpData, (OpcUa_CharA*)(strTmp.c_str()), &lpInfo, &unInfoLen))
			OpcUa_String_AttachCopy(&m_strLegalCopyright, (char*)lpInfo);

		strTmp.clear();
		strTmp=strSubBlock;
		strTmp.append("OriginalFileName");
		if (VerQueryValue(lpData, (OpcUa_CharA*)(strTmp.c_str()), &lpInfo, &unInfoLen))
			OpcUa_String_AttachCopy(&m_strOriginalFileName, (char*)lpInfo);

		strTmp.clear();
		strTmp=strSubBlock;
		strTmp.append("ProductName");

		if (VerQueryValue(lpData, (OpcUa_CharA*)(strTmp.c_str()), &lpInfo, &unInfoLen))
			OpcUa_String_AttachCopy(&m_strProductName, (char*)lpInfo);
		strTmp.clear();
		strTmp=strSubBlock;
		strTmp.append("ProductVersion");

		if (VerQueryValue(lpData, (OpcUa_CharA*)(strTmp.c_str()), &lpInfo, &unInfoLen))
			OpcUa_String_AttachCopy(&m_strProductVersion, (char*)lpInfo);

		strTmp.clear();
		strTmp=strSubBlock;
		strTmp.append("Comments");

		if (VerQueryValue(lpData, (OpcUa_CharA*)(strTmp.c_str()), &lpInfo, &unInfoLen))
			OpcUa_String_AttachCopy(&m_strComments, (char*)lpInfo);

		strTmp.clear();
		strTmp=strSubBlock;
		strTmp.append("LegalTrademarks");

		if (VerQueryValue(lpData, (OpcUa_CharA*)(strTmp.c_str()), &lpInfo, &unInfoLen))
			OpcUa_String_AttachCopy(&m_strLegalTrademarks, (char*)lpInfo);

		strTmp.clear();
		strTmp=strSubBlock;
		strTmp.append("PrivateBuild");
		if (VerQueryValue(lpData, (OpcUa_CharA*)(strTmp.c_str()), &lpInfo, &unInfoLen))
			OpcUa_String_AttachCopy(&m_strPrivateBuild, (char*)lpInfo);

		strTmp.clear();
		strTmp=strSubBlock;
		strTmp.append("SpecialBuild");
		if (VerQueryValue(lpData, (OpcUa_CharA*)(strTmp.c_str()), &lpInfo, &unInfoLen))
			OpcUa_String_AttachCopy(&m_strSpecialBuild, (char*)lpInfo);

		OpcUa_Free(lpData);
	}
	catch (BOOL)
	{
		free(lpData);
		return FALSE;
	}

	return TRUE;
}


WORD CFileVersionInfo::GetFileVersion(int nIndex) const
{
	if (nIndex == 0)
		return (WORD)(m_FileInfo.dwFileVersionLS & 0x0000FFFF);
	else if (nIndex == 1)
		return (WORD)((m_FileInfo.dwFileVersionLS & 0xFFFF0000) >> 16);
	else if (nIndex == 2)
		return (WORD)(m_FileInfo.dwFileVersionMS & 0x0000FFFF);
	else if (nIndex == 3)
		return (WORD)((m_FileInfo.dwFileVersionMS & 0xFFFF0000) >> 16);
	else
		return 0;
}


WORD CFileVersionInfo::GetProductVersion(int nIndex) const
{
	if (nIndex == 0)
		return (WORD)(m_FileInfo.dwProductVersionLS & 0x0000FFFF);
	else if (nIndex == 1)
		return (WORD)((m_FileInfo.dwProductVersionLS & 0xFFFF0000) >> 16);
	else if (nIndex == 2)
		return (WORD)(m_FileInfo.dwProductVersionMS & 0x0000FFFF);
	else if (nIndex == 3)
		return (WORD)((m_FileInfo.dwProductVersionMS & 0xFFFF0000) >> 16);
	else
		return 0;
}


DWORD CFileVersionInfo::GetFileFlagsMask() const
{
	return m_FileInfo.dwFileFlagsMask;
}


DWORD CFileVersionInfo::GetFileFlags() const
{
	return m_FileInfo.dwFileFlags;
}


DWORD CFileVersionInfo::GetFileOs() const
{
	return m_FileInfo.dwFileOS;
}


DWORD CFileVersionInfo::GetFileType() const
{
	return m_FileInfo.dwFileType;
}


DWORD CFileVersionInfo::GetFileSubtype() const
{
	return m_FileInfo.dwFileSubtype;
}


OpcUa_DateTime CFileVersionInfo::GetFileDate() const
{
	OpcUa_DateTime	ft;
	ft.dwLowDateTime = m_FileInfo.dwFileDateLS;
	ft.dwHighDateTime = m_FileInfo.dwFileDateMS;

	return ft;
}


OpcUa_String CFileVersionInfo::GetCompanyName() const
{
	return m_strCompanyName;
}


OpcUa_String CFileVersionInfo::GetFileDescription() const
{
	return m_strFileDescription;
}


OpcUa_String CFileVersionInfo::GetFileVersion() const
{
	return m_strFileVersion;
}


OpcUa_String CFileVersionInfo::GetInternalName() const
{
	return m_strInternalName;
}


OpcUa_String CFileVersionInfo::GetLegalCopyright() const
{
	return m_strLegalCopyright;
}


OpcUa_String CFileVersionInfo::GetOriginalFileName() const
{
	return m_strOriginalFileName;
}


OpcUa_String CFileVersionInfo::GetProductName() const
{
	return m_strProductName;
}


OpcUa_String CFileVersionInfo::GetProductVersion() const
{
	return m_strProductVersion;
}


OpcUa_String CFileVersionInfo::GetComments() const
{
	return m_strComments;
}


OpcUa_String CFileVersionInfo::GetLegalTrademarks() const
{
	return m_strLegalTrademarks;
}


OpcUa_String CFileVersionInfo::GetPrivateBuild() const
{
	return m_strPrivateBuild;
}


OpcUa_String CFileVersionInfo::GetSpecialBuild() const
{
	return m_strSpecialBuild;
}


void CFileVersionInfo::Reset()
{
	//ZeroMemory(&m_FileInfo, sizeof(m_FileInfo));
	//OpcUa_String_Initialize(&m_strCompanyName);
	//OpcUa_String_Initialize(&m_strFileDescription);
	//OpcUa_String_Initialize(&m_strFileVersion);
	//OpcUa_String_Initialize(&m_strInternalName);
	//OpcUa_String_Initialize(&m_strLegalCopyright);
	//OpcUa_String_Initialize(&m_strOriginalFileName);
	//OpcUa_String_Initialize(&m_strProductName);
	//OpcUa_String_Initialize(&m_strProductVersion);
	//OpcUa_String_Initialize(&m_strComments);
	//OpcUa_String_Initialize(&m_strLegalTrademarks);
	//OpcUa_String_Initialize(&m_strPrivateBuild);
	//OpcUa_String_Initialize(&m_strSpecialBuild);
}

#endif