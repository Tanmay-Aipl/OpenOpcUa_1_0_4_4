#pragma once


// Boîte de dialogue CMessageDlg
#include "..\resource.h"
#include "afxcmn.h"
class CMessageDlg : public CDialog
{
	DECLARE_DYNAMIC(CMessageDlg)

public:
	CMessageDlg(CWnd* pParent = NULL);   // constructeur standard
	virtual ~CMessageDlg();

// Données de boîte de dialogue
	enum { IDD = IDD_MESSAGE_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // Prise en charge de DDX/DDV

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	CListCtrl m_ListCtrlMessage;
	OpcUa_StatusCode AddLoggerMessage(OpcUa_String Message, DWORD Level = 0);

	int m_iCptMessage;
};
