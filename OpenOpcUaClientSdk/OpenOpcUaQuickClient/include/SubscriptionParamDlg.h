#pragma once


// Boîte de dialogue CSubscriptionParamDlg

class CSubscriptionParamDlg : public CDialog
{
	DECLARE_DYNAMIC(CSubscriptionParamDlg)

public:
	CSubscriptionParamDlg(CWnd* pParent = NULL);   // constructeur standard
	virtual ~CSubscriptionParamDlg();

// Données de boîte de dialogue
	enum { IDD = IDD_SUBSCRIPTION_PARAM_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // Prise en charge de DDX/DDV

	DECLARE_MESSAGE_MAP()
public:
	OpcUa_Double	m_dblPublishingInterval;
	OpcUa_UInt32	m_uiLifeTimeCount;
	OpcUa_UInt32	m_uiKeepAliveCount;
	OpcUa_UInt32	m_uiMaxNotificationPerPublish;
	OpcUa_Byte		m_Priority;
	BOOL m_bPublishingEnabled;
};
