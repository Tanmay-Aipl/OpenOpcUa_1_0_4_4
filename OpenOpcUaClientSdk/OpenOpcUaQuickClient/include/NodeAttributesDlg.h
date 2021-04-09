#pragma once
#include "afxcmn.h"


// Boîte de dialogue CNodeAttributesDlg

class CNodeAttributesDlg : public CDialog
{
	DECLARE_DYNAMIC(CNodeAttributesDlg)

public:
	CNodeAttributesDlg(CWnd* pParent = NULL,HANDLE hSession=OpcUa_Null);   // constructeur standard
	virtual ~CNodeAttributesDlg();
	void DeleteAllAttributes();
// Données de boîte de dialogue
	enum { IDD = IDD_ATTRIBUTES_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // Prise en charge de DDX/DDV

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_NodeAttributesCtrl;
	OpcUa_NodeId m_NodeId;
	HANDLE m_hApplication;
	HANDLE m_hSession;
public:
	virtual BOOL OnInitDialog();
	void AddAttributesColumn(void);
	HRESULT DisplayAttributes(OpcUa_NodeId* sourceNodeId);
	afx_msg void OnClose();
};
