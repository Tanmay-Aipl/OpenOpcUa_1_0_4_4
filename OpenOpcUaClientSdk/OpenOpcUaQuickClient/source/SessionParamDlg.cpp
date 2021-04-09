// SessionParamDlg.cpp : fichier d'implémentation
//

#include "stdafx.h"
#include "UAQuickClient.h"
#include "SessionParamDlg.h"


// Boîte de dialogue CSessionParamDlg

IMPLEMENT_DYNAMIC(CSessionParamDlg, CDialog)

CSessionParamDlg::CSessionParamDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSessionParamDlg::IDD, pParent)
	, m_dblSessionTimeout(600000) // 10 mn
	, m_uiMaxMsgSize(0)
	, m_bActive(TRUE)
{

}

CSessionParamDlg::~CSessionParamDlg()
{
}

void CSessionParamDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_KEEPALIVE_EDIT, m_dblSessionTimeout);
	DDX_Text(pDX, IDC_MAX_MESSAGESIZE_EDIT, m_uiMaxMsgSize);
	DDX_Check(pDX, IDC_CHECK1, m_bActive);
}


BEGIN_MESSAGE_MAP(CSessionParamDlg, CDialog)
END_MESSAGE_MAP()


// Gestionnaires de messages de CSessionParamDlg
