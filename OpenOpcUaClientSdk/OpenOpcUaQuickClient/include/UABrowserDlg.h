#pragma once
#include "afxcmn.h"
using namespace std;
#include <vector>
#include "afxwin.h"
#include "SubEdit.h"
// Boîte de dialogue CUABrowserDlg
enum CALL_BROWSEDIALOGBOX_REASON
{
	OPCUA_UNKNOWN,
	OPCUA_BROWSE,
	OPCUA_ADD_MONITORED_ITEM,
	OPCUA_READ,
	OPCUA_WRITE
};
class CUABrowserDlg : public CDialog
{
	DECLARE_DYNAMIC(CUABrowserDlg)

public:
	CUABrowserDlg(CWnd* pParent = NULL,HANDLE hSession = NULL);   // constructeur standard
	virtual ~CUABrowserDlg();
	void SetSession(OpcUa_Handle hSession);
// Données de boîte de dialogue
	enum { IDD = IDD_UA_BROWSER_DIALOG };
	HANDLE		m_hApplication;
	HANDLE		m_hSession;
	CSubEdit	m_editWnd;
private:
	CToolTipCtrl		m_Tooltip;
	CImageList			m_ImageList;
	OpcUa_UInt32		m_CallReason; //  Tell the dialogBox the reason why it was loaded, Browse, AddMonitoredItem, Read or Write
									//OPCUA_UNKNOWN,
									//OPCUA_BROWSE,
									//OPCUA_ADD_MONITORED_ITEM,
									//OPCUA_READ,
									//OPCUA_WRITE
	OpcUa_UInt32		m_iCurrentDataType;
protected:	
	// Browse 
	OpcUa_StatusCode Browse(OpcUa_NodeId aFromNodeId);
	void AddAttributesColumn(void);
	HRESULT DisplayAttributes(OpcUa_NodeId* sourceNodeId);
	virtual void DoDataExchange(CDataExchange* pDX);    // Prise en charge de DDX/DDV
	HRESULT GetNodeClassIcon(OpcUa_NodeClass aNodeId, short* sImage);
	HRESULT GetDatatypeIcon(OpcUa_NodeId aNodeClass, short* sImage);
	DECLARE_MESSAGE_MAP()
public:
	void SetCallReason(OpcUa_UInt32 iReason) {m_CallReason=iReason;}
	virtual BOOL OnInitDialog();
	CListCtrl m_NodeAttributesCtrl;
	CTreeCtrl m_ReferencesTreeCtrl;
private:
	UINT			m_DragDropFormat;
	OpcUa_UInt32	m_NodeClassMask; // Contains the Mask for the class to browse
	OpcUa_Int32		m_iBrowseDirection; // Contains the value for the browse direction requested. Values are define in Enum OpcUa_BrowseDirection
public:
	afx_msg void OnTvnSelchangedNodeidsTree(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnBrowseOption();
	afx_msg void OnExport();
	std::vector<OpcUa_NodeId*> m_NodeIds; // liste des node a ajouter dans la souscription
	afx_msg void OnMonitor();
	// Button to refresh attributes from the server
	CButton m_btRefreshAttributes;
	afx_msg void OnBnClickedButtonRefreshAttributes();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnLvnBeginlabeleditNodeAttributesList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnEndlabeleditNodeAttributesList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnBegindragNodeidsTree(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnClose();
};
