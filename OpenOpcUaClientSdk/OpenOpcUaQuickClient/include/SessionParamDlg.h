#pragma once


// Boîte de dialogue CSessionParamDlg

class CSessionParamDlg : public CDialog
{
	DECLARE_DYNAMIC(CSessionParamDlg)

public:
	CSessionParamDlg(CWnd* pParent = NULL);   // constructeur standard
	virtual ~CSessionParamDlg();

// Données de boîte de dialogue
	enum { IDD = IDD_SESSION_PARAMS_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // Prise en charge de DDX/DDV

	DECLARE_MESSAGE_MAP()
public:
	double m_dblSessionTimeout;
	UINT m_uiMaxMsgSize;
	BOOL m_bActive;
};
