#pragma once
#include "afxwin.h"
#include "..\resource.h"

// Boîte de dialogue CDiscoveryDlg

class CDiscoveryDlg : public CDialog
{
	DECLARE_DYNAMIC(CDiscoveryDlg)

public:
	CDiscoveryDlg(CWnd* pParent = NULL,OpcUa_Handle hApplication=OpcUa_Null, OpcUa_Handle hSession=OpcUa_Null);   // constructeur standard
	virtual ~CDiscoveryDlg();

// Données de boîte de dialogue
	enum { IDD = IDD_DISCOVERY_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // Prise en charge de DDX/DDV

	DECLARE_MESSAGE_MAP()
private:
	OpcUa_Handle m_hSession;
	OpcUa_Handle m_hApplication;
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnCbnSelchangeComboAppName();
	afx_msg void OnCbnDropdownComboAppName();
	afx_msg void OnDeltaposSpinEndpointDescription(NMHDR *pNMHDR, LRESULT *pResult);
	void ViewEndpointDescription();
	OpcUa_EndpointDescription* GetSelectedEndpointDescription() { return m_pSelectedEndpointDescription; }
	CComboBox m_ComboAppName;
	CString m_hostName;
	// AppDescription
	OpcUa_Int32 m_NbOfAppDescription;
	OpcUa_ApplicationDescription* m_pAppDescription;
	// EndpointDescription
	OpcUa_UInt32 m_NbOfEndpointDescription;
	OpcUa_UInt32 m_uiCurrentEndpointDescription;
	OpcUa_EndpointDescription* m_pEndpointDescription; // Endpoints read from the server
	CString m_EndpointUrl;
	CString m_szCurrentEndpointDescription;
	bool m_bSecurityModeNone;
	bool m_bSecurityModeSign;
	bool m_bSecurityModeSignEncrypt;
	int  m_RadioSecurityVal;
	CString m_SecurityPolicyUri;
	CToolTipCtrl		m_Tooltip;
private:
	OpcUa_EndpointDescription* m_pSelectedEndpointDescription; // selected Endpoint. This endpoint will be used for server connection

public:
	afx_msg void OnBnClickedSelectButtom();
	afx_msg void OnBnClickedRefreshButton();
	afx_msg void OnBnClickedRadioNone();
	afx_msg void OnBnClickedRadioSign();
	afx_msg void OnBnClickedRadioSignEncrypt();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
