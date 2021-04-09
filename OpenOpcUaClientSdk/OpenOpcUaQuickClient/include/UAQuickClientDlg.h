
// UAQuickClientDlg.h : fichier d'en-tête
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "NodeAttributesDlg.h"
#include "MessageDlg.h"
#include "UABrowserDlg.h"
#include "InterfaceDropTarget.h"

// boîte de dialogue CUAQuickClientDlg
class CUAQuickClientDlg : public CDialog
{
// Construction
public:
	CUAQuickClientDlg(CWnd* pParent = NULL);	// constructeur standard

// Données de boîte de dialogue
	enum { IDD = IDD_UAQUICKCLIENT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// Prise en charge de DDX/DDV


// Implémentation
protected:
	HICON			m_hIcon;
	CToolTipCtrl	m_Tooltip;
	CUABrowserDlg*	m_pUABrowserDlg;
	// Fonctions générées de la table des messages
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	HRESULT InsertMonitoredItemColumns(void);
	HRESULT RemoveMonitoredItemColumns(void);
	HRESULT UpdateMonitoredItemValueInListCtrl(HANDLE hSubscription,void* hHandle,OpcUa_DataValue aDataValue);
	DECLARE_MESSAGE_MAP()
	static OpcUa_StatusCode __stdcall OnNotificationMessage(OpcUa_Handle hSubscription,
								OpcUa_Int32 NoOfMonitoredItems,
								OpcUa_MonitoredItemNotification* MonitoredItems,
								void* pParam);
public:
	CListCtrl m_MonitoredElts;
	//CString m_hostName;
	HANDLE m_hApplication;
	OpcUa_Boolean m_bConnected;
	// AppDescription
	//OpcUa_Int32 m_NbOfAppDescription;
	OpcUa_ApplicationDescription m_AppDescription;
	// EndpointDescription
	//OpcUa_UInt32 m_NbOfEndpointDescription;
	OpcUa_EndpointDescription* m_pEndpointDescription;
	OpcUa_Boolean m_bBrowseOrExport;
	CNodeAttributesDlg* m_pNodeAttributesDlg;
	CImageList			m_ImageList; // ImageList for the treeView where appears Sessions and Subscription
	CImageList			m_NodeClassesImageList;
	CMessageDlg*		m_pMessageDlg;
	//
	//afx_msg void OnCbnDropdownComboAppName();
	//CComboBox m_ComboAppName;
	//afx_msg void OnCbnSelchangeComboAppName();
	CComboBox m_ComboSecurityPolicy;
	afx_msg void OnBnClickedButtonConnect();
	//CComboBox m_ComboEndPointDescription;
	CTreeCtrl m_TreeSessionSubscription;
	CButton m_btConnectDisconnect;
	// EndpointDescription as a String
	CString m_szEndpointDescription;
	UINT m_DragDropFormat;
	CInterfaceDropTarget m_DT;
	CString m_strXMLFileName;
public:
	afx_msg void OnCloseSession();
	afx_msg void OnBrowse();
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnCbnSelchangeComboEndpointDescription();
	afx_msg void OnCreateSubscription();
	afx_msg void OnAddMonitoreditems();
	static OpcUa_StatusCode __stdcall OnShutdownMessage(HANDLE hApplication,HANDLE hSession,
														OpcUa_String strShutdownMessage,void* extraParam);
	afx_msg void OnSubscriptionParamChange();
	afx_msg void OnAttributes();
	afx_msg void OnWrite();
	afx_msg void OnDelete();
	OpcUa_StatusCode CreateSession(OpcUa_EndpointDescription* pEndpointDescription);
	afx_msg void OnReadAttributes();
	afx_msg void OnWriteAttributes();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnMove(int x, int y);
	afx_msg void OnNMClickMonitoredItemctrl(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickTreeSessionSubscription(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnSelchangedTreeSessionSubscription(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnRemoveSubscription();	
	afx_msg void OnFileExit();
	afx_msg void OnFileLoad();
	afx_msg void OnFileSave();
	afx_msg void OnViewMessage();
	afx_msg void OnViewAttribute();
public:
	void AddLoggerMessage(char* message, OpcUa_UInt16 uiLevel);
	afx_msg void OnDiscoverServer();
	afx_msg void OnTvnBeginrdragTreeSessionSubscription(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnHdnEnddragMonitoredItemctrl(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	void GetCurrentSessionandSubscription(OpcUa_Handle* hSession, OpcUa_Handle* hSubscription);
	OpcUa_StatusCode InsertMonitoredItemInListCtrl(OpenOpcUa_InternalNode* pInternalNode);
	afx_msg void OnViewBrowser();
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	CButton m_btBrowse;
	CButton m_btAttribute;
	CButton m_btMessage;
	CButton m_btExit;
	afx_msg void OnBnClickedButtonBrowse();
	afx_msg void OnBnClickedButtonAttribute();
	afx_msg void OnBnClickedButtonMessage();
	afx_msg void OnBnClickedButtonExit();
	afx_msg void OnModifyMonitoredItem();
};
