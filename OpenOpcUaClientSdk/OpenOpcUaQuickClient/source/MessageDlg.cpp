// MessageDlg.cpp : fichier d'implémentation
//

#include "stdafx.h"
#include "MessageDlg.h"
#include "afxdialogex.h"


// Boîte de dialogue CMessageDlg
enum _MESSAGE_COLUMN {
	TIMESPAMP_COLUMN,
	TEXT_COLUMN
};

// Gestionnaires de messages de CMessageDlg
BEGIN_MESSAGE_MAP(CMessageDlg, CDialog)
	ON_WM_CLOSE()
END_MESSAGE_MAP()
IMPLEMENT_DYNAMIC(CMessageDlg, CDialog)

CMessageDlg::CMessageDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMessageDlg::IDD, pParent)
{
	m_iCptMessage = 0;
}

CMessageDlg::~CMessageDlg()
{
}

void CMessageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MESSAGE_LIST, m_ListCtrlMessage);
}


void CMessageDlg::OnClose()
{
	// TODO: ajoutez ici le code de votre gestionnaire de messages et/ou les paramètres par défaut des appels

	CDialog::OnClose();
}

BOOL CMessageDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_ListCtrlMessage.InsertColumn(TIMESPAMP_COLUMN, L"Timestamp",LVCFMT_CENTER, 80, 0);
	m_ListCtrlMessage.InsertColumn(TEXT_COLUMN, L"Message", LVCFMT_CENTER, 250, 0);


	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION : les pages de propriétés OCX devraient retourner FALSE
}

OpcUa_StatusCode CMessageDlg::AddLoggerMessage(OpcUa_String Message, DWORD Level)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	if (OpcUa_String_StrLen(&Message)==0)
		return S_FALSE;
	FILETIME ft;
	OpcUa_DateTime dt;
	//SYSTEMTIME sysTime;

	CoFileTimeNow(&ft); 
	dt.dwHighDateTime=ft.dwHighDateTime; 
	dt.dwLowDateTime = ft.dwLowDateTime;

	OpcUa_String* strTime = OpcUa_Null;
	OpenOpcUa_DateTimeToString(dt, &strTime);
	/*---------------------------------------------------*/

	OpcUa_Trace(OPCUA_TRACE_CLIENT_ERROR, "%s\n", OpcUa_String_GetRawString(&Message));
	//ft.dwHighDateTime = dt.dwHighDateTime;
	//ft.dwLowDateTime = dt.dwLowDateTime;
	//if (FileTimeToSystemTime(&ft, &sysTime))
	{
		//char* buffer = (char*)malloc(25);
		//if (buffer)
		//{
		//	memset(buffer, 0, 25);
		//	sprintf(buffer, "%02u/%02u/%04u %02u:%02u:%02u.%03u\n",
		//		sysTime.wDay, sysTime.wMonth, sysTime.wYear, sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds);
		//	OpcUa_Free(buffer);
		//	uStatus = OpcUa_Good;
		//}
		/*---------------------------------------------------*/
		//GetLocalTime(&sysTime);
		//CString strTime;
		//strTime.Format(L"%02u-%02u-%04u %02u:%02u:%02u.%03u", sysTime.wDay, sysTime.wMonth, sysTime.wYear,
		//	sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds);

		if (m_ListCtrlMessage.m_hWnd != NULL)
		{
			CSize size(0, 1000);
			//int nbItem=m_ListCtrlMessage.GetItemCount();
			int nbItem = m_iCptMessage++;
			CString strVal(OpcUa_String_GetRawString(strTime));
			int index = m_ListCtrlMessage.InsertItem(nbItem, strVal.GetBuffer(), Level);
			CString strMessage(OpcUa_String_GetRawString(&Message));
			m_ListCtrlMessage.SetItem(index, 1, LVIF_TEXT, strMessage.GetBuffer(), 0, 0, 0, 0);
			m_ListCtrlMessage.SetColumnWidth(0, LVSCW_AUTOSIZE);
			m_ListCtrlMessage.SetColumnWidth(1, LVSCW_AUTOSIZE);
			m_ListCtrlMessage.SetHotItem(index);
			m_ListCtrlMessage.SetItemState(index, LVIS_SELECTED, LVIS_SELECTED);
			m_ListCtrlMessage.Scroll(size);
			//if (pDoc)
			//{
			//	if (nbItem>(int)(pDoc->m_NbEltInLogView))  // Maximum pDoc->m_NbEltInLogView messages par fenêtre
			//	{
			//		ClearAllLoggerMessage();
			//		nbItem = m_iCptMessage++;
			//	}
			//}
		}
		else
			uStatus=OpcUa_Bad;
	}
	//else
	//{
	//	uStatus = OpcUa_Bad;
	//}
	return uStatus;
}

