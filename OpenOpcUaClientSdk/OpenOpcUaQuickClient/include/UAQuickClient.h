
// UAQuickClient.h�: fichier d'en-t�te principal pour l'application PROJECT_NAME
//

#pragma once

#ifndef __AFXWIN_H__
	#error "incluez 'stdafx.h' avant d'inclure ce fichier pour PCH"
#endif

#include "..\resource.h"		// symboles principaux


// CUAQuickClientApp:
// Consultez UAQuickClient.cpp pour l'impl�mentation de cette classe
//

class CUAQuickClientApp : public CWinAppEx
{
public:
	CUAQuickClientApp();

// Substitutions
	public:
	virtual BOOL InitInstance();

// Impl�mentation

	DECLARE_MESSAGE_MAP()
};

extern CUAQuickClientApp theApp;