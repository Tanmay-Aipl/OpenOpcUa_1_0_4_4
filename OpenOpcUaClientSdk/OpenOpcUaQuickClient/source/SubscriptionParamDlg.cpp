// SubscriptionParamDlg.cpp : fichier d'implémentation
//

#include "stdafx.h"
#include "UAQuickClient.h"
#include "SubscriptionParamDlg.h"


// Boîte de dialogue CSubscriptionParamDlg

IMPLEMENT_DYNAMIC(CSubscriptionParamDlg, CDialog)

CSubscriptionParamDlg::CSubscriptionParamDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSubscriptionParamDlg::IDD, pParent)
{
	m_dblPublishingInterval=1000.0;
	m_uiLifeTimeCount=0;
	m_uiKeepAliveCount=0;
	m_uiMaxNotificationPerPublish=0;
	m_Priority=0;
}

CSubscriptionParamDlg::~CSubscriptionParamDlg()
{
}

void CSubscriptionParamDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_PUBLISHING_INTERVAL, m_dblPublishingInterval);
	DDX_Text(pDX, IDC_LIFETIME_COUNT, m_uiLifeTimeCount);
	DDX_Text(pDX, IDC_KEEPALIVE_COUNT, m_uiKeepAliveCount);
	DDX_Text(pDX, IDC_MAX_NOTIFICATION_PER_PUBLISH, m_uiMaxNotificationPerPublish);
	DDX_Text(pDX, IDC_PRIORITY, m_Priority);
	DDX_Check(pDX, IDC_CHECK_PUBLISHING_ENABLED, m_bPublishingEnabled);
}


BEGIN_MESSAGE_MAP(CSubscriptionParamDlg, CDialog)
END_MESSAGE_MAP()


// Gestionnaires de messages de CSubscriptionParamDlg
