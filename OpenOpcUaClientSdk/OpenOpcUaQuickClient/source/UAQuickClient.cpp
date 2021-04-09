
// UAQuickClient.cpp : D�finit les comportements de classe pour l'application.
//

#include "stdafx.h"
#include "UAQuickClient.h"
#include "UAQuickClientDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CUAQuickClientApp

BEGIN_MESSAGE_MAP(CUAQuickClientApp, CWinAppEx)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// construction CUAQuickClientApp

CUAQuickClientApp::CUAQuickClientApp()
{
	// TODO�: ajoutez ici du code de construction,
	// Placez toutes les initialisations significatives dans InitInstance
}


// Seul et unique objet CUAQuickClientApp

CUAQuickClientApp theApp;


// initialisation de CUAQuickClientApp

BOOL CUAQuickClientApp::InitInstance()
{
	// InitCommonControlsEx() est requis sur Windows�XP si le manifeste de l'application
	// sp�cifie l'utilisation de ComCtl32.dll version�6 ou ult�rieure pour activer les
	// styles visuels.  Dans le cas contraire, la cr�ation de fen�tres �chouera.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// � d�finir pour inclure toutes les classes de contr�les communs � utiliser
	// dans votre application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();

	// Initialisation standard
	// Si vous n'utilisez pas ces fonctionnalit�s et que vous souhaitez r�duire la taille
	// de votre ex�cutable final, vous devez supprimer ci-dessous
	// les routines d'initialisation sp�cifiques dont vous n'avez pas besoin.
	// Changez la cl� de Registre sous laquelle nos param�tres sont enregistr�s
	// TODO : modifiez cette cha�ne avec des informations appropri�es,
	// telles que le nom de votre soci�t� ou organisation
	SetRegistryKey(_T("Applications locales g�n�r�es par AppWizard"));
	AfxOleInit();
	CUAQuickClientDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO : placez ici le code d�finissant le comportement lorsque la bo�te de dialogue est
		//  ferm�e avec OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO : placez ici le code d�finissant le comportement lorsque la bo�te de dialogue est
		//  ferm�e avec Annuler
	}

	// Lorsque la bo�te de dialogue est ferm�e, retourner FALSE afin de quitter
	//  l'application, plut�t que de d�marrer la pompe de messages de l'application.
	return FALSE;
}
void CUAQuickClientDlg::OnTvnBeginrdragTreeSessionSubscription(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: ajoutez ici le code de votre gestionnaire de notification de contr�le
	*pResult = 0;
}