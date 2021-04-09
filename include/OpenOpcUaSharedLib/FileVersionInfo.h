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

#if !defined(AFX_FILEVERSION_H__F828004C_7680_40FE_A08D_7BB4FF05B4CC__INCLUDED_)
#define AFX_FILEVERSION_H__F828004C_7680_40FE_A08D_7BB4FF05B4CC__INCLUDED_

#ifdef WIN32
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
namespace OpenOpcUa
{
//#include <winver.h>
	namespace UASharedLib 
	{
		class SHAREDLIB_EXPORT CFileVersionInfo
		{
		// construction/destruction
		public:
			CFileVersionInfo();
			virtual ~CFileVersionInfo();

		// operations
		public:
			OpcUa_Boolean Create(HMODULE hModule = NULL);
			OpcUa_Boolean Create(char* lpszFileName);

		// attribute operations
		public:
			WORD GetFileVersion(int nIndex) const;
			WORD GetProductVersion(int nIndex) const;
			DWORD GetFileFlagsMask() const;
			DWORD GetFileFlags() const;
			DWORD GetFileOs() const;
			DWORD GetFileType() const;
			DWORD GetFileSubtype() const;
			OpcUa_DateTime GetFileDate() const;

			OpcUa_String GetCompanyName() const;
			OpcUa_String GetFileDescription() const;
			OpcUa_String GetFileVersion() const;
			OpcUa_String GetInternalName() const;
			OpcUa_String GetLegalCopyright() const;
			OpcUa_String GetOriginalFileName() const;
			OpcUa_String GetProductName() const;
			OpcUa_String GetProductVersion() const;
			OpcUa_String GetComments() const;
			OpcUa_String GetLegalTrademarks() const;
			OpcUa_String GetPrivateBuild() const;
			OpcUa_String GetSpecialBuild() const;

		// implementation helpers
		protected:
			virtual void Reset();
			BOOL GetTranslationId(LPVOID lpData, UINT unBlockSize, WORD wLangId, DWORD &dwId, BOOL bPrimaryEnough = FALSE);

		// attributes
		private:
			VS_FIXEDFILEINFO m_FileInfo;

			OpcUa_String m_strCompanyName;
			OpcUa_String m_strFileDescription;
			OpcUa_String m_strFileVersion;
			OpcUa_String m_strInternalName;
			OpcUa_String m_strLegalCopyright;
			OpcUa_String m_strOriginalFileName;
			OpcUa_String m_strProductName;
			OpcUa_String m_strProductVersion;
			OpcUa_String m_strComments;
			OpcUa_String m_strLegalTrademarks;
			OpcUa_String m_strPrivateBuild;
			OpcUa_String m_strSpecialBuild;
		};

	}
}
#endif
#endif // !defined(AFX_FILEVERSION_H__F828004C_7680_40FE_A08D_7BB4FF05B4CC__INCLUDED_)