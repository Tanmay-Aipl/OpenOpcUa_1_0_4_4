// MonitoredItemParams.cpp : fichier d'implémentation
//

#include "stdafx.h"
#include "MonitoredItemParams.h"
#include "afxdialogex.h"


// Boîte de dialogue CMonitoredItemParams

IMPLEMENT_DYNAMIC(CMonitoredItemParamsDlg, CDialog)

CMonitoredItemParamsDlg::CMonitoredItemParamsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMonitoredItemParamsDlg::IDD, pParent)
	, m_NodeId(_T(""))
	, m_bDiscardOldest(FALSE)
	, m_uiQueueSize(0)
	, m_dblSamplingInterval(0)
	, m_dblDeadbandValue(0)
	, m_szTimestampToReturn(_T("Both"))
	, m_szDeadbandType(_T("None"))
	, m_bTriggerValue(FALSE)
	, m_bTriggerStatusCode(FALSE)
	, m_bTriggerTimestamp(FALSE)
{

}

CMonitoredItemParamsDlg::~CMonitoredItemParamsDlg()
{
}

void CMonitoredItemParamsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_NODEID, m_NodeId);
	DDX_Check(pDX, IDC_CHECK_DISCARD_OLDEST, m_bDiscardOldest);
	DDX_Control(pDX, IDC_COMBO_TIMESTAMP_TO_RETURN, m_TimeStampToReturnCtrlCombo);
	DDX_Text(pDX, IDC_EDIT_QUEUE_SIZE, m_uiQueueSize);
	DDX_Text(pDX, IDC_EDIT_SAMPLING_INTERVAL, m_dblSamplingInterval);
	DDX_Control(pDX, IDC_COMBO_DEADBAND_TYPE, m_DeadbandCombo);
	DDX_Text(pDX, IDC_EDIT_DEADBAND_VALUE, m_dblDeadbandValue);
	DDX_CBString(pDX, IDC_COMBO_TIMESTAMP_TO_RETURN, m_szTimestampToReturn);
	DDX_CBString(pDX, IDC_COMBO_DEADBAND_TYPE, m_szDeadbandType);
	DDX_Check(pDX, IDC_CHECK_VALUE, m_bTriggerValue);
	DDX_Check(pDX, IDC_CHECK_STATUSCODE, m_bTriggerStatusCode);
	DDX_Check(pDX, IDC_CHECK_TIMESTAMP, m_bTriggerTimestamp);
}


BEGIN_MESSAGE_MAP(CMonitoredItemParamsDlg, CDialog)
END_MESSAGE_MAP()


// Gestionnaires de messages de CMonitoredItemParams
BOOL CMonitoredItemParamsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	// Deadband
	m_DeadbandCombo.AddString(L"Absolute");
	m_DeadbandCombo.AddString(L"None");
	m_DeadbandCombo.AddString(L"Percent");
	
	// DeadbandValue
	m_dblDeadbandValue = 0;
	// DiscardOldest
	m_bDiscardOldest = OpcUa_False;
	// SamplingInterval
	m_dblSamplingInterval = 1000;
	// TimeStampToReturn
	m_TimeStampToReturnCtrlCombo.AddString(L"Both");
	m_TimeStampToReturnCtrlCombo.AddString(L"Server");
	m_TimeStampToReturnCtrlCombo.AddString(L"Source");
	// QueueSize
	m_uiQueueSize = 0;
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION : les pages de propriétés OCX devraient retourner FALSE
}