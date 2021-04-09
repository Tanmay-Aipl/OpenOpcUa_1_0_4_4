#pragma once


// Boîte de dialogue CEndpointUrl

class CEndpointUrlDlg : public CDialog
{
	DECLARE_DYNAMIC(CEndpointUrlDlg)

public:
	CEndpointUrlDlg(CWnd* pParent = NULL);   // constructeur standard
	virtual ~CEndpointUrlDlg();

// Données de boîte de dialogue
	enum { IDD = IDD_DIALOG_ENDPOINT_URL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // Prise en charge de DDX/DDV

	DECLARE_MESSAGE_MAP()
public:
	CString			m_EndpointUrl;
protected:
	CToolTipCtrl	m_Tooltip;
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
