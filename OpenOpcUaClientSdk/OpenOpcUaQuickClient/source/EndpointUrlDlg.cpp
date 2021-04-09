// EndpointUrl.cpp : fichier d'implémentation
//

#include "stdafx.h"
#include "UAQuickClient.h"
#include "EndpointUrlDlg.h"


// Boîte de dialogue CEndpointUrl

IMPLEMENT_DYNAMIC(CEndpointUrlDlg, CDialog)

CEndpointUrlDlg::CEndpointUrlDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEndpointUrlDlg::IDD, pParent)
	, m_EndpointUrl(_T(""))
{

}

CEndpointUrlDlg::~CEndpointUrlDlg()
{
}

void CEndpointUrlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_ENDPOINTURL, m_EndpointUrl);
}


BEGIN_MESSAGE_MAP(CEndpointUrlDlg, CDialog)
END_MESSAGE_MAP()

BOOL CEndpointUrlDlg::PreTranslateMessage(MSG* pMsg)
{
	m_Tooltip.RelayEvent(pMsg);

	return CDialog::PreTranslateMessage(pMsg);
}
// Gestionnaires de messages de CEndpointUrl

BOOL CEndpointUrlDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ToolTips
	m_Tooltip.Create(this);
	m_Tooltip.Activate(TRUE);
	m_Tooltip.AddTool(GetDlgItem(IDC_EDIT_ENDPOINTURL),L"Enter an EndpointUrl. ie: opc.tcp://localhost:16664/4CEUAServer");

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION : les pages de propriétés OCX devraient retourner FALSE
}
