#pragma once
#include "..\resource.h"
#include "afxwin.h"
// Boîte de dialogue CMonitoredItemParams

class CMonitoredItemParamsDlg : public CDialog
{
	DECLARE_DYNAMIC(CMonitoredItemParamsDlg)

public:
	CMonitoredItemParamsDlg(CWnd* pParent = NULL);   // constructeur standard
	virtual ~CMonitoredItemParamsDlg();

// Données de boîte de dialogue
	enum { IDD = IDD_MONITOREDITEM_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // Prise en charge de DDX/DDV

	DECLARE_MESSAGE_MAP()
public:
	CString m_NodeId;
	BOOL m_bDiscardOldest;
	CComboBox m_TimeStampToReturnCtrlCombo;
	UINT m_uiQueueSize;
	double m_dblSamplingInterval;
	CComboBox m_DeadbandCombo;
	double m_dblDeadbandValue;
	virtual BOOL OnInitDialog();
	CString m_szTimestampToReturn;
	CString m_szDeadbandType;
	BOOL m_bTriggerValue;
	BOOL m_bTriggerStatusCode;
	BOOL m_bTriggerTimestamp;
};
