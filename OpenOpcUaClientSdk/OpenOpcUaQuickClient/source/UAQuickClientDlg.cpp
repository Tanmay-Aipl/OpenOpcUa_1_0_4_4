
// UAQuickClientDlg.cpp : fichier d'implémentation
//

#include "stdafx.h"
#include "MessageDlg.h"
#include "UAQuickClient.h"
#include "UAQuickClientDlg.h"
#include "UABrowserDlg.h"
#include "SessionParamDlg.h"
#include "SubscriptionParamDlg.h"
#include "EndpointUrlDlg.h"
#include "NodeAttributesDlg.h"
#include "UAWriteDlg.h"
#include "DiscoveryDlg.h"
#include "MonitoredItemParams.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// boîte de dialogue CUAQuickClientDlg


enum MONITOREDITEM_COLUMN {MONITOREDITEM_NAME,MONITOREDITEM_VALUE,MONITOREDITEM_TIMESTAMP};

CUAQuickClientDlg::CUAQuickClientDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CUAQuickClientDlg::IDD, pParent)
	, m_szEndpointDescription(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON_OPEN_OPC_UA);
	// 
	m_pEndpointDescription=OpcUa_Null;
	m_bConnected=OpcUa_False;
	m_bBrowseOrExport=OpcUa_False;
	m_pNodeAttributesDlg=OpcUa_Null;
	m_pMessageDlg=OpcUa_Null;
	m_pUABrowserDlg = OpcUa_Null;
	CString format = AfxGetApp()->GetProfileString(L"DragDrop", L"Clipformat", L"Common");
	if (format == "Private")
		m_DragDropFormat = ::RegisterClipboardFormat(L"OpenOpcUAQuickClientFormat");
	else
	{
		if (format != "Common")
			AfxMessageBox(L"Please specify a valid clipformat!", MB_ICONSTOP);
		m_DragDropFormat = CF_TEXT;
	}
}

void CUAQuickClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MONITORED_ITEMCTRL, m_MonitoredElts);
	DDX_Control(pDX, IDC_TREE_SESSION_SUBSCRIPTION, m_TreeSessionSubscription);
	DDX_Control(pDX, IDC_BUTTON_CONNECT, m_btConnectDisconnect);
	DDX_Text(pDX, IDC_EDITENDPOINT_DESCRIPTION, m_szEndpointDescription);
	DDX_Control(pDX, IDC_BUTTON_BROWSE, m_btBrowse);
	DDX_Control(pDX, IDC_BUTTON_ATTRIBUTE, m_btAttribute);
	DDX_Control(pDX, IDC_BUTTON_MESSAGE, m_btMessage);
	DDX_Control(pDX, IDC_BUTTON_EXIT, m_btExit);
}

BEGIN_MESSAGE_MAP(CUAQuickClientDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	//ON_CBN_DROPDOWN(IDC_COMBO_APP_NAME, &CUAQuickClientDlg::OnCbnDropdownComboAppName)
	//ON_CBN_SELCHANGE(IDC_COMBO_APP_NAME, &CUAQuickClientDlg::OnCbnSelchangeComboAppName)
	ON_BN_CLICKED(IDC_BUTTON_CONNECT, &CUAQuickClientDlg::OnBnClickedButtonConnect)
	ON_COMMAND(IDM_CLOSE_SESSION, &CUAQuickClientDlg::OnCloseSession)
	ON_COMMAND(IDM_BROWSE, &CUAQuickClientDlg::OnBrowse)
	ON_WM_CONTEXTMENU()
	//ON_CBN_SELCHANGE(IDC_COMBO_ENDPOINT_DESCRIPTION, &CUAQuickClientDlg::OnCbnSelchangeComboEndpointDescription)
	ON_COMMAND(IDM_CREATE_SUBSCRIPTION, &CUAQuickClientDlg::OnCreateSubscription)
	ON_COMMAND(IDM_ADD_MONITOREDITEMS, &CUAQuickClientDlg::OnAddMonitoreditems)
	ON_COMMAND(ID__CHANGEPARAMS, &CUAQuickClientDlg::OnSubscriptionParamChange)
	ON_COMMAND(IDM_ATTRIBUTES, &CUAQuickClientDlg::OnAttributes)
	ON_COMMAND(IDM_WRITE, &CUAQuickClientDlg::OnWrite)
	ON_COMMAND(IDM_DELETE, &CUAQuickClientDlg::OnDelete)
	ON_COMMAND(IDM_READ_ATTRIBUTES, &CUAQuickClientDlg::OnReadAttributes)
	ON_COMMAND(IDM_WRITE_ATTRIBUTES, &CUAQuickClientDlg::OnWriteAttributes)
	ON_WM_MOVE()
	ON_NOTIFY(NM_CLICK, IDC_MONITORED_ITEMCTRL, &CUAQuickClientDlg::OnNMClickMonitoredItemctrl)
	ON_NOTIFY(NM_CLICK, IDC_TREE_SESSION_SUBSCRIPTION, &CUAQuickClientDlg::OnNMClickTreeSessionSubscription)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_SESSION_SUBSCRIPTION, &CUAQuickClientDlg::OnTvnSelchangedTreeSessionSubscription)
	ON_COMMAND(ID__REMOVESUBSCRIPTION, &CUAQuickClientDlg::OnRemoveSubscription)
	ON_COMMAND(ID_FILE_EXIT, &CUAQuickClientDlg::OnFileExit)
	ON_COMMAND(ID_FILE_LOAD, &CUAQuickClientDlg::OnFileLoad)
	ON_COMMAND(ID_FILE_SAVE, &CUAQuickClientDlg::OnFileSave)
	ON_COMMAND(ID_VIEW_MESSAGE, &CUAQuickClientDlg::OnViewMessage)
	ON_COMMAND(ID_VIEW_ATTRIBUTE, &CUAQuickClientDlg::OnViewAttribute)
	ON_COMMAND(IDM_DISCOVER_SERVER, &CUAQuickClientDlg::OnDiscoverServer)
	ON_NOTIFY(TVN_BEGINRDRAG, IDC_TREE_SESSION_SUBSCRIPTION, &CUAQuickClientDlg::OnTvnBeginrdragTreeSessionSubscription)
	ON_NOTIFY(HDN_ENDDRAG, 0, &CUAQuickClientDlg::OnHdnEnddragMonitoredItemctrl)
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_COMMAND(ID_VIEW_BROWSER, &CUAQuickClientDlg::OnViewBrowser)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE, &CUAQuickClientDlg::OnBnClickedButtonBrowse)
	ON_BN_CLICKED(IDC_BUTTON_ATTRIBUTE, &CUAQuickClientDlg::OnBnClickedButtonAttribute)
	ON_BN_CLICKED(IDC_BUTTON_MESSAGE, &CUAQuickClientDlg::OnBnClickedButtonMessage)
	ON_BN_CLICKED(IDC_BUTTON_EXIT, &CUAQuickClientDlg::OnBnClickedButtonExit)
	ON_COMMAND(IDM_MODIFY, &CUAQuickClientDlg::OnModifyMonitoredItem)
END_MESSAGE_MAP()


// gestionnaires de messages pour CUAQuickClientDlg

BOOL CUAQuickClientDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_DT.Register(this);
	m_hApplication = OpcUa_Null;
	OpcUa_StatusCode uStatus=OpenOpcUa_InitializeAbstractionLayer("OpenOpcUaQuickClient",&m_hApplication);
	if (uStatus==OpcUa_Good)
	{
		OpcUa_String szCertificateStore;
		OpcUa_String_Initialize(&szCertificateStore);
		OpcUa_String_AttachCopy(&szCertificateStore,(OpcUa_CharA*)"CertificateStore");
		uStatus=OpenOpcUa_InitializeSecurity(m_hApplication,szCertificateStore);
	}
	// ToolTips
	m_Tooltip.Create(this);
	m_Tooltip.Activate(TRUE);
	m_Tooltip.AddTool(GetDlgItem(IDC_BUTTON_CONNECT),L"Connect to UA Server");
	//
	OpenOpcUa_Trace(m_hApplication,OPCUA_TRACE_CLIENT_ERROR,"Welcome in the very simple MFC Based OpenOpcUa's Client\n");
	// Définir l'icône de cette boîte de dialogue. L'infrastructure effectue cela automatiquement
	//  lorsque la fenêtre principale de l'application n'est pas une boîte de dialogue
	SetIcon(m_hIcon, TRUE);			// Définir une grande icône
	SetIcon(m_hIcon, FALSE);		// Définir une petite icône
	// Association des images avec le CTreeCtrl m_TreeSessionSubscription
	m_ImageList.Create(16,16,ILC_COLOR32,1,1);
	m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_SESSION_INACTIVE ) ); //0
	m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_SESSION_ACTIVE  ) );  //1
	m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_SUBSCRIPTION_INACTIVE  ) );  //2
	m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_SUBSCRIPTION_ACTIVE  ) );  //3
	m_TreeSessionSubscription.SetImageList(&m_ImageList,TVSIL_NORMAL);
	// Chargement de l'imageList qui contiendra la images pour les NodeClasses
	m_NodeClassesImageList.Create(16,16,ILC_COLOR32,1,1);
	m_NodeClassesImageList.Add(AfxGetApp()->LoadIcon( IDI_UNSPECIFIED ) ); //0
	m_NodeClassesImageList.Add(AfxGetApp()->LoadIcon( IDI_OBJECT ) ); //1
	m_NodeClassesImageList.Add(AfxGetApp()->LoadIcon( IDI_VARIABLE ) ); //2
	m_NodeClassesImageList.Add(AfxGetApp()->LoadIcon( IDI_METHOD ) ); //3
	m_NodeClassesImageList.Add(AfxGetApp()->LoadIcon( IDI_OBJECT_TYPE ) ); //4
	m_NodeClassesImageList.Add(AfxGetApp()->LoadIcon( IDI_VARIABLE_TYPE ) ); //5
	m_NodeClassesImageList.Add(AfxGetApp()->LoadIcon( IDI_REFERENCE_TYPE ) ); //6
	m_NodeClassesImageList.Add(AfxGetApp()->LoadIcon( IDI_DATA_TYPE ) ); //7
	m_NodeClassesImageList.Add(AfxGetApp()->LoadIcon( IDI_VIEW ) ); //8
	m_MonitoredElts.SetImageList(&m_NodeClassesImageList,TVSIL_NORMAL);
	// Association des images avec le listCtrl qui contient les items monitoré
	// Ajout des colonnes pour les items monitorés
	InsertMonitoredItemColumns();
	m_MonitoredElts.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	// Connect/disconnect button
	HICON  hIcon=AfxGetApp()->LoadIcon(IDI_CONNECT);
	m_btConnectDisconnect.SetIcon(hIcon);
	m_btConnectDisconnect.EnableWindow(TRUE);
	// Browse button
	HICON  hIconBrowse = AfxGetApp()->LoadIcon(IDI_BROWSE);
	m_btBrowse.SetIcon(hIconBrowse);
	m_btBrowse.EnableWindow(TRUE);
	m_Tooltip.AddTool(GetDlgItem(IDC_BUTTON_BROWSE), L"Show browser");
	// Attribute button
	HICON  hIconAttributes = AfxGetApp()->LoadIcon(IDI_ATTRIBUTES);
	m_btAttribute.SetIcon(hIconAttributes);
	m_btAttribute.EnableWindow(TRUE);
	m_Tooltip.AddTool(GetDlgItem(IDC_BUTTON_ATTRIBUTE), L"Show MonitoredItem attributes");
	// Message button
	HICON  hIconMessages = AfxGetApp()->LoadIcon(IDI_MESSAGE);
	m_btMessage.SetIcon(hIconMessages);
	m_Tooltip.AddTool(GetDlgItem(IDC_BUTTON_MESSAGE), L"Show application messages");
	// Exit button
	HICON  hIconMExit = AfxGetApp()->LoadIcon(IDI_EXIT);
	m_btExit.SetIcon(hIconMExit);
	m_Tooltip.AddTool(GetDlgItem(IDC_BUTTON_EXIT), L"Quit the application");
	return TRUE;  // retourne TRUE, sauf si vous avez défini le focus sur un contrôle
}

// Si vous ajoutez un bouton Réduire à votre boîte de dialogue, vous devez utiliser le code ci-dessous
//  pour dessiner l'icône. Pour les applications MFC utilisant le modèle Document/Vue,
//  cela est fait automatiquement par l'infrastructure.

void CUAQuickClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // contexte de périphérique pour la peinture

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Centrer l'icône dans le rectangle client
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Dessiner l'icône
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// Le système appelle cette fonction pour obtenir le curseur à afficher lorsque l'utilisateur fait glisser
//  la fenêtre réduite.
HCURSOR CUAQuickClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


HRESULT CUAQuickClientDlg::InsertMonitoredItemColumns(void)
{
	HRESULT hr=S_OK;
	hr=m_MonitoredElts.InsertColumn(MONITOREDITEM_NAME,L"Item Name",LVCFMT_LEFT | LVCFMT_CENTER,80,0);
	hr=m_MonitoredElts.InsertColumn(MONITOREDITEM_VALUE,L"Value",LVCFMT_LEFT | LVCFMT_CENTER,80,0);
	hr=m_MonitoredElts.InsertColumn(MONITOREDITEM_TIMESTAMP,L"Timestamp",LVCFMT_LEFT | LVCFMT_CENTER,80,0);
	return hr;
}
HRESULT CUAQuickClientDlg::RemoveMonitoredItemColumns(void)
{
	BOOL bSucceed=TRUE;
	while  (bSucceed)
		bSucceed=m_MonitoredElts.DeleteColumn(0);
	return S_OK;
}
 /// <summary>
/// Inserts the monitored item in list control.
/// </summary>
/// <param name="pInternalNode">The p internal node.</param>
/// <returns></returns>
OpcUa_StatusCode CUAQuickClientDlg::InsertMonitoredItemInListCtrl(OpenOpcUa_InternalNode* pInternalNode)
{
	HRESULT hr=S_OK;	
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_DataValue* pResults=OpcUa_Null;
	OpcUa_ReadValueId* pNodesToRead=OpcUa_Null;
	if (pInternalNode)
	{
		CString strTmp;
		
		std::basic_string<wchar_t>* strVal=new std::basic_string<wchar_t>();
		int index=m_MonitoredElts.GetItemCount();

		// Read the NodeClass to show the appropriate icon in the dialog
		
		strTmp.Empty();strTmp.ReleaseBuffer();
		// First get the current selected Handle, Session or subscription
		// Based on the will get the hSession
		HTREEITEM hCurrentSession=m_TreeSessionSubscription.GetSelectedItem();
		OpenOpcUa_HandleType aHandleType;
		HANDLE hSession = OpcUa_Null;
		HANDLE hLocalSubscription=(HANDLE)m_TreeSessionSubscription.GetItemData(hCurrentSession);
		uStatus=OpenOpcUa_WhatIsIt(hLocalSubscription,&aHandleType);
		if (aHandleType==OPENOPCUA_SUBSCRIPTION)
			uStatus=OpenOpcUa_GetSessionOfSubscription(hLocalSubscription,&hSession);
		else
			hSession = hLocalSubscription;
		if ((hSession) && (uStatus==OpcUa_Good))
		{
			// Let select the most appropriate icon for the inserted object		
			// We will do this by reading the NodeClass of the inserted Node
			int iNewItem = -1;
			OpcUa_UInt32 iIcone=0;			
			pNodesToRead=(OpcUa_ReadValueId*)OpcUa_Alloc(sizeof(OpcUa_ReadValueId));
			OpcUa_ReadValueId_Initialize(&pNodesToRead[0]);
			pNodesToRead[0].AttributeId=OpcUa_Attributes_NodeClass;
			pNodesToRead[0].NodeId=pInternalNode->m_NodeId;
			uStatus=OpenOpcUa_ReadAttributes(m_hApplication,hSession,OpcUa_TimestampsToReturn_Both,1,pNodesToRead,&pResults);
			if (uStatus == OpcUa_Good)
			{
				switch (pResults->Value.Value.Int32)
				{
				case OpcUa_NodeClass_DataType:
					iIcone = 7;
					break;
				case OpcUa_NodeClass_Method:
					iIcone = 3;
					break;
				case OpcUa_NodeClass_Object:
					iIcone = 1;
					break;
				case OpcUa_NodeClass_ObjectType:
					iIcone = 4;
					break;
				case OpcUa_NodeClass_ReferenceType:
					iIcone = 6;
					break;
				case OpcUa_NodeClass_Unspecified:
					iIcone = 0;
					break;
				case OpcUa_NodeClass_Variable:
					iIcone = 2;
					break;
				case OpcUa_NodeClass_VariableType:
					iIcone = 5;
					break;
				case OpcUa_NodeClass_View:
					iIcone = 8;
					break;
				default:
					iIcone = 0;
					break;
				}
			
				// Insert the item in the control
				// We need to ask the displayName to the server

				pNodesToRead[0].AttributeId = OpcUa_Attributes_DisplayName;
				OpcUa_String* pszDisplayName = OpcUa_Null;
				pszDisplayName = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
				OpcUa_String_Initialize(pszDisplayName);
				uStatus = OpenOpcUa_ReadAttributes(m_hApplication, hSession, OpcUa_TimestampsToReturn_Both, 1, pNodesToRead, &pResults);
				if (uStatus == OpcUa_Good)
				{
					// dans le cas du DataType le type est dans un NodeId
					if (pResults[0].StatusCode == OpcUa_Good)
					{
						OpcUa_String_CopyTo(&(pResults[0].Value.Value.LocalizedText->Text), pszDisplayName);
						iNewItem = m_MonitoredElts.InsertItem(index, CString(OpcUa_String_GetRawString(pszDisplayName)), iIcone);
						pInternalNode->m_UserData=(void*)iNewItem;
						m_MonitoredElts.SetItemData(iNewItem, (DWORD_PTR)pInternalNode);
					}
				}
			}
			OpcUa_Free(pNodesToRead);
			//
			strTmp.Format(L"**");
			// Value
			OpcUa_String* strValue=OpcUa_Null; //(OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
			uStatus=OpenOpcUa_VariantToString(pInternalNode->m_DataValue.Value,&strValue);
			if (uStatus==OpcUa_Good)
			{
				if (strValue)
				{
					CString aTmpString(OpcUa_String_GetRawString(strValue));
					strTmp.Format(L"%ls",aTmpString);
				}			
				m_MonitoredElts.SetItem(iNewItem,MONITOREDITEM_VALUE,LVIF_TEXT|LVIF_STATE,(LPTSTR)(LPCTSTR)strTmp,iIcone,0,0,0);
				strTmp.ReleaseBuffer();strTmp.Empty();
				OpcUa_String_Clear(strValue);
				OpcUa_Free(strValue);
			}
			// Timestamp
			OpcUa_String* strTime=OpcUa_Null;
			uStatus=OpenOpcUa_DateTimeToString(pInternalNode->m_DataValue.ServerTimestamp,&strTime);
			if (uStatus==OpcUa_Good)
			{
				strTmp=CString(OpcUa_String_GetRawString(strTime));
			}
			else
			{
				strTmp.Format(L"01/01/1601 00:00:00.0000");
			}
			m_MonitoredElts.SetItem(iNewItem,MONITOREDITEM_TIMESTAMP,LVIF_TEXT,(LPTSTR)(LPCTSTR)strTmp,0,0,0,0);
			strTmp.ReleaseBuffer();strTmp.Empty();
			m_MonitoredElts.SetItemData(iNewItem,(DWORD_PTR)pInternalNode);
			delete strVal;
		}
	}
	return hr;
}
HRESULT CUAQuickClientDlg::UpdateMonitoredItemValueInListCtrl(HANDLE hSubscription,void* hHandle,OpcUa_DataValue aDataValue)
{
	HRESULT hr=S_OK;
	OpcUa_StatusCode uStatus=OpcUa_Good;
	int index=-1;

	HTREEITEM hItem=m_TreeSessionSubscription.GetSelectedItem();
	if (hItem)
	{
		OpenOpcUa_HandleType aHandleType;
		HANDLE hLocalSubscription = (HANDLE)m_TreeSessionSubscription.GetItemData(hItem);
		uStatus = OpenOpcUa_WhatIsIt(hLocalSubscription, &aHandleType);
		if (aHandleType == OPENOPCUA_SUBSCRIPTION)
		{
			if (hSubscription != hLocalSubscription)
				return hr;
			BOOL bFound = FALSE;
			int iCount = m_MonitoredElts.GetItemCount();
			for (int ii = 0; ii < iCount; ii++)
			{
				OpenOpcUa_InternalNode* pTmpInternalNode = (OpenOpcUa_InternalNode*)m_MonitoredElts.GetItemData(ii);
				if (pTmpInternalNode)
				{
					if (pTmpInternalNode->m_hMonitoredItem == (HANDLE)hHandle)
					{
						bFound = TRUE;
						index = ii;
						break;
					}
				}
			}
			if (bFound)
			{
				CString strTmp;
				// Will extract the associated index
				OpcUa_String* strValue = OpcUa_Null;
				uStatus = OpenOpcUa_VariantToString(aDataValue.Value, &strValue);
				if (uStatus == OpcUa_Good)
				{
					CString aTmpString(OpcUa_String_GetRawString(strValue));
					m_MonitoredElts.SetItem(index, MONITOREDITEM_VALUE, LVIF_TEXT, (LPTSTR)(LPCTSTR)aTmpString, 0, 0, 0, 0);
					aTmpString.ReleaseBuffer(); aTmpString.Empty();
				}
				if (strValue)
				{
					OpcUa_String_Clear(strValue);
					OpcUa_Free(strValue);
				}
				OpcUa_String* strTime = OpcUa_Null;
				uStatus = OpenOpcUa_DateTimeToString(aDataValue.ServerTimestamp, &strTime);
				if (uStatus == OpcUa_Good)
				{
					if (strTime)
					{
						CString aTmpString(OpcUa_String_GetRawString(strTime));
						strTmp.Format(L"%s", aTmpString.GetBuffer());
						aTmpString.ReleaseBuffer(); aTmpString.Empty();
					}
				}
				else
				{
					strTmp.Format(L"01/01/1601 00:00:00.0000");
				}
				if (strTime)
				{
					OpcUa_String_Clear(strTime);
					OpcUa_Free(strTime);
				}
				m_MonitoredElts.SetItem(index, MONITOREDITEM_TIMESTAMP, LVIF_TEXT, (LPTSTR)(LPCTSTR)strTmp, 0, 0, 0, 0);
				strTmp.ReleaseBuffer(); strTmp.Empty();
			}


		}
		else
		{
			if (aHandleType == OPENOPCUA_SESSION)
			{
				BOOL bFound = FALSE;
				int iCount = m_MonitoredElts.GetItemCount();
				for (int ii = 0; ii < iCount; ii++)
				{
					OpenOpcUa_InternalNode* pTmpInternalNode = (OpenOpcUa_InternalNode*)m_MonitoredElts.GetItemData(ii);
					if (pTmpInternalNode)
					{
						if (pTmpInternalNode->m_hMonitoredItem == (HANDLE)hHandle)
						{
							bFound = TRUE;
							index = ii;
							break;
						}
					}
				}
				if (bFound)
				{

					CString strTmp;
					// Will extract the associated index
					OpcUa_String* strValue = OpcUa_Null;
					uStatus = OpenOpcUa_VariantToString(aDataValue.Value, &strValue);
					if (uStatus == OpcUa_Good)
					{
						CString aTmpString(OpcUa_String_GetRawString(strValue));
						m_MonitoredElts.SetItem(index, MONITOREDITEM_VALUE, LVIF_TEXT, (LPTSTR)(LPCTSTR)aTmpString, 0, 0, 0, 0);
						aTmpString.ReleaseBuffer(); aTmpString.Empty();
					}
					if (strValue)
					{
						OpcUa_String_Clear(strValue);
						OpcUa_Free(strValue);
					}
					OpcUa_String* strTime = OpcUa_Null;
					uStatus = OpenOpcUa_DateTimeToString(aDataValue.ServerTimestamp, &strTime);
					if (uStatus == OpcUa_Good)
					{
						if (strTime)
						{
							CString aTmpString(OpcUa_String_GetRawString(strTime));
							strTmp.Format(L"%s", aTmpString.GetBuffer());
						}
					}
					else
					{
						strTmp.Format(L"01/01/1601 00:00:00.0000");
					}
					if (strTime)
					{
						OpcUa_String_Clear(strTime);
						OpcUa_Free(strTime);
					}
					m_MonitoredElts.SetItem(index, MONITOREDITEM_TIMESTAMP, LVIF_TEXT, (LPTSTR)(LPCTSTR)strTmp, 0, 0, 0, 0);
					strTmp.ReleaseBuffer(); strTmp.Empty();
				}
			}
		}
	}
	return hr;
}

// Création de la session sur le serveur slectionnée selon le contexte de securité selectionné
void CUAQuickClientDlg::OnBnClickedButtonConnect()
{
	OpcUa_StatusCode uStatus;
	if (!m_bConnected)
	{
		// recupération de EndpointUrl selectionnée
		if (m_pEndpointDescription)
		{
			AddLoggerMessage("--- CreateSession ---", 1);
			uStatus = CreateSession(m_pEndpointDescription);
		}
		else
		{
			// in case nothing was selected in the endpoint description will show a dialogBox
			// this dialogBox allows user to enter EndpointUrl directly			
			CEndpointUrlDlg dlg;
			if (dlg.DoModal()==IDOK)
			{
				OpcUa_UInt32 uiNbOfEndpointDescription=0;
				OpcUa_EndpointDescription* pEndpointDescription=OpcUa_Null;
				OpcUa_String* pString=OpcUa_Null;
				pString=(OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
				if (pString)
				{
					OpcUa_String_Initialize(pString);
					USES_CONVERSION;
					char* chEndpointUrl = W2A(dlg.m_EndpointUrl.GetBuffer());
					OpcUa_String_AttachCopy(pString, chEndpointUrl);
					uStatus = OpenOpcUa_GetEndpoints(m_hApplication,
						pString,
						&uiNbOfEndpointDescription,
						&pEndpointDescription);
					if (uStatus == OpcUa_Good)
					{
						uStatus = CreateSession(pEndpointDescription);
						if (uStatus != OpcUa_Good)
							AddLoggerMessage("CreateSession failed", 0);
						else
						{
							AddLoggerMessage("--- CreateSession ---", 1);
							m_szEndpointDescription = CString(OpcUa_String_GetRawString(&(pEndpointDescription->EndpointUrl)));
							UpdateData(FALSE);
						}
					}
					else
						AddLoggerMessage("OpenOpcUa_GetEndpoints failed",0);
				}
				else
					AddLoggerMessage("Not enough momory to Create a Session", 0);
			}
		}
	}
	else
	{
		AddLoggerMessage("Your are already connected. The connection will be shutdown", 0);
		if (m_pUABrowserDlg != OpcUa_Null)
		{
			m_pUABrowserDlg->OnClose();
			delete m_pUABrowserDlg;
			m_pUABrowserDlg = OpcUa_Null;
		}
		HANDLE hSession=OpcUa_Null;
		HTREEITEM hItem=m_TreeSessionSubscription.GetSelectedItem();
		if (hItem)
		{
			hSession=(HANDLE)m_TreeSessionSubscription.GetItemData(hItem);
			if (hSession)
			{
				OpcUa_StatusCode uStatus=OpenOpcUa_CloseSession(m_hApplication,hSession);
				if (uStatus == OpcUa_Good)
				{
					m_btConnectDisconnect.SetWindowText(L"Connect");
					// change the tooltips message for the connect/disconenct button
					m_Tooltip.DelTool(GetDlgItem(IDC_BUTTON_CONNECT));
					m_Tooltip.AddTool(GetDlgItem(IDC_BUTTON_CONNECT), L"Connect to UAServer");
					HICON  hIcon = AfxGetApp()->LoadIcon(IDI_CONNECT);
					m_btConnectDisconnect.SetIcon(hIcon);
					m_TreeSessionSubscription.DeleteItem(hItem);
					m_bConnected = OpcUa_False;
					// clear the monitored elementList
					m_MonitoredElts.DeleteAllItems();
					// clear the NodeAttributeDlg
					if (m_pNodeAttributesDlg)
					{
						m_pNodeAttributesDlg->DeleteAllAttributes();
					}
					m_szEndpointDescription.Empty();
					UpdateData(FALSE);
					delete m_pEndpointDescription;
					m_pEndpointDescription = OpcUa_Null;
					AddLoggerMessage("--- SessionClosed by the user ---", 1);
				}
				else
					OpenOpcUa_Trace(m_hApplication, OPCUA_TRACE_CLIENT_LEVEL_ERROR, "OpenOpcUa_CloseSession failed uStatus\n");
			}
		}
	}
}

void CUAQuickClientDlg::OnCloseSession()
{
	OnBnClickedButtonConnect();
}

void CUAQuickClientDlg::OnBrowse()
{
	HANDLE hSession=OpcUa_Null;
	OpcUa_StatusCode uStatus;
	m_bBrowseOrExport=OpcUa_True;
	HTREEITEM hItem=m_TreeSessionSubscription.GetSelectedItem();
	if (hItem)
	{
		hSession=(HANDLE)m_TreeSessionSubscription.GetItemData(hItem);
		if (hSession)
		{
			OpenOpcUa_HandleType aHandleType;
			uStatus=OpenOpcUa_WhatIsIt(hSession,&aHandleType);
			if (aHandleType==OPENOPCUA_SESSION)
			{
				CUABrowserDlg*pDlg=new CUABrowserDlg(this,hSession);
				pDlg->SetCallReason(OPCUA_BROWSE);
				pDlg->m_hApplication=m_hApplication;
				pDlg->m_hSession=hSession;
				if (pDlg->DoModal()==IDOK)
				{
					;//TRACE(L"Browse done\n");
				}
				delete pDlg;
			}		
		}
		else
		{
			CString str;
			str.Format(L"Critical error please contact 4CE Industry (Code:B0001)");
			//AddString(str);
		}
	}
	else
	{
		CString str;
		str.Format(L"Select the session to browse to");
		//AddString(str);
	}
	m_bBrowseOrExport=OpcUa_False;
}

void CUAQuickClientDlg::OnContextMenu(CWnd* pWnd, CPoint point)
{
	OpcUa_StatusCode uStatus=OpcUa_Bad;
	CMenu menu;
	if (!pWnd)
		return;
	if (m_TreeSessionSubscription.m_hWnd==pWnd->m_hWnd)
	{
		HTREEITEM hItem=m_TreeSessionSubscription.GetSelectedItem();
		if (hItem)
		{
			HANDLE hSession=(HANDLE)m_TreeSessionSubscription.GetItemData(hItem);

			OpenOpcUa_HandleType aHandleType;
			uStatus=OpenOpcUa_WhatIsIt(hSession,&aHandleType);
			if (aHandleType==OPENOPCUA_SESSION)
			{
				VERIFY(menu.LoadMenu(IDR_TREECTRL_SESSION_CONTEXT_MENU));
				CMenu* pPopup = menu.GetSubMenu(0);
				if (pPopup != NULL)
				{
					CWnd* pWndPopupOwner = this;

					while (pWndPopupOwner->GetStyle() & WS_CHILD)
						pWndPopupOwner = pWndPopupOwner->GetParent();

					pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y,
						pWndPopupOwner);
				}
			}
			else
			{
				if (aHandleType==OPENOPCUA_SUBSCRIPTION)
				{
					//VERIFY(menu.LoadMenu(IDR_TREECTRL_SESSION_CONTEXT_MENU));
					VERIFY(menu.LoadMenu(IDR_TREECTRL_SUBSCRIPTION_CONTEXT_MENU));
					CMenu* pPopup = menu.GetSubMenu(0);
					if (pPopup != NULL)
					{
						CWnd* pWndPopupOwner = this;

						while (pWndPopupOwner->GetStyle() & WS_CHILD)
							pWndPopupOwner = pWndPopupOwner->GetParent();

						pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y,
							pWndPopupOwner);
					}
				}
			}
		}
	}
	else
	{
		if (m_MonitoredElts.m_hWnd==pWnd->m_hWnd)
		{
			if (m_bConnected)
			{
				VERIFY(menu.LoadMenu(IDR_MONITORED_ITEM_CONTEXT_MENU));
				CMenu* pPopup = menu.GetSubMenu(0);
				if (pPopup != NULL)
				{
					CWnd* pWndPopupOwner = this;

					while (pWndPopupOwner->GetStyle() & WS_CHILD)
						pWndPopupOwner = pWndPopupOwner->GetParent();

					pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y,
						pWndPopupOwner);
				}
			}
		}
	}
}

void CUAQuickClientDlg::OnCbnSelchangeComboEndpointDescription()
{
	m_btConnectDisconnect.EnableWindow(TRUE);
}

void CUAQuickClientDlg::OnCreateSubscription()
{
	HANDLE hSession=NULL;
	// On commence par récupérer le hSession
	HTREEITEM hItem=m_TreeSessionSubscription.GetSelectedItem();
	if (hItem)
	{
		hSession=(HANDLE)m_TreeSessionSubscription.GetItemData(hItem);
	}
	if (hSession)
	{
		CSubscriptionParamDlg dlg;
		if (dlg.DoModal()==IDOK)
		{
			OpcUa_StatusCode uStatus;
			HANDLE hSubscription;
			OpcUa_Double dblPublishingInterval=dlg.m_dblPublishingInterval;
			OpcUa_UInt32 uiLifeTimeCount=dlg.m_uiLifeTimeCount;
			OpcUa_UInt32 uiKeepAliveCount=dlg.m_uiKeepAliveCount;
			uStatus=OpenOpcUa_CreateSubscription(
					m_hApplication,hSession,
					&dblPublishingInterval,
					&uiLifeTimeCount,
					&uiKeepAliveCount,
					dlg.m_uiMaxNotificationPerPublish,
					dlg.m_bPublishingEnabled,
					dlg.m_Priority,
					&hSubscription);
			if (uStatus == OpcUa_Good)
			{
				CString strTmp;
				strTmp.Format(L"Subscription-%05lf", dblPublishingInterval);
				HTREEITEM hNewItem = m_TreeSessionSubscription.InsertItem(strTmp, hItem);
				if (dlg.m_bPublishingEnabled)
					m_TreeSessionSubscription.SetItemImage(hNewItem, 3, 3); // Active publish
				else
					m_TreeSessionSubscription.SetItemImage(hNewItem, 2, 2); // Inactive publish
				m_TreeSessionSubscription.SetItemData(hNewItem, (DWORD_PTR)hSubscription);
				// we want to expand the subscription
				m_TreeSessionSubscription.Expand(hItem, TVE_EXPAND);
				m_TreeSessionSubscription.SelectItem(hNewItem);
				OpenOpcUa_SetPublishCallback(m_hApplication, hSession, hSubscription, (PFUNC)CUAQuickClientDlg::OnNotificationMessage, (void*)this);
			}
			else
				OpenOpcUa_Trace(m_hApplication ,OPCUA_TRACE_CLIENT_LEVEL_DEBUG, "OpenOpcUa_CreateSubscription failed: 0x%05x\n",uStatus);
		}
	}
}


OpcUa_StatusCode __stdcall CUAQuickClientDlg::OnNotificationMessage(OpcUa_Handle hSubscription,
															 OpcUa_Int32 NoOfMonitoredItems,
															 OpcUa_MonitoredItemNotification* MonitoredItems,
															 void* pParam)
{
	CUAQuickClientDlg* pUAQuickClientDlg=(CUAQuickClientDlg*)pParam;
	OpcUa_StatusCode uStatus=OpcUa_Good;
	SYSTEMTIME sysTime;
	
	GetSystemTime(&sysTime);
	int iTotal= sysTime.wHour*3600000 + sysTime.wMinute*60000 + sysTime.wMilliseconds+sysTime.wSecond*1000;
	
	// pas de mise a jour pendant le browse ou l'export
	if (pUAQuickClientDlg->m_bBrowseOrExport==OpcUa_True)
		return uStatus;
	if (NoOfMonitoredItems)
	{
		for (int ii=0;ii<NoOfMonitoredItems;ii++)
		{
			HANDLE hMonitoredItem=(HANDLE)MonitoredItems[ii].ClientHandle;
			OpenOpcUa_InternalNode* pInternalNode=OpcUa_Null;

			OpcUa_DataValue aDataValue=MonitoredItems[ii].Value;
			pUAQuickClientDlg->UpdateMonitoredItemValueInListCtrl(hSubscription,hMonitoredItem,aDataValue);
		}
	}
	return uStatus;
}
void CUAQuickClientDlg::GetCurrentSessionandSubscription(OpcUa_Handle* hSession, OpcUa_Handle* hSubscription)
{
	OpcUa_Handle hTmpSubscription;
	OpcUa_StatusCode uStatus;
	HTREEITEM hItem = m_TreeSessionSubscription.GetSelectedItem();
	if (hItem)
		hTmpSubscription = (HANDLE)m_TreeSessionSubscription.GetItemData(hItem);
	else
		return;
	if (hTmpSubscription)
	{
		OpenOpcUa_HandleType aHandleType;
		uStatus = OpenOpcUa_WhatIsIt(hTmpSubscription, &aHandleType);
		if (aHandleType == OPENOPCUA_SUBSCRIPTION)
		{
			uStatus = OpenOpcUa_GetSessionOfSubscription(hTmpSubscription, hSession);
			if (uStatus == OpcUa_Good)
				*hSubscription = hTmpSubscription;
		}
		else
		{
			if (aHandleType == OPENOPCUA_SESSION)
			{
				*hSession = hTmpSubscription;
				*hSubscription = OpcUa_Null; // No Subscription selected
			}
		}
	}
}
void CUAQuickClientDlg::OnAddMonitoreditems()
{
	HANDLE hSession=OpcUa_Null;
	HANDLE hSubscription=OpcUa_Null;
	OpcUa_StatusCode uStatus;
	HTREEITEM hItem=m_TreeSessionSubscription.GetSelectedItem();
	if (hItem)
		hSubscription=(HANDLE)m_TreeSessionSubscription.GetItemData(hItem);
	else
		return;
	if (hSubscription)
	{
		OpenOpcUa_HandleType aHandleType;
		uStatus=OpenOpcUa_WhatIsIt(hSubscription,&aHandleType);
		if (aHandleType==OPENOPCUA_SUBSCRIPTION)
		{
			uStatus=OpenOpcUa_GetSessionOfSubscription(hSubscription,&hSession);
			if (uStatus==OpcUa_Good)
			{
				if (m_pUABrowserDlg==OpcUa_Null)
				{
					m_pUABrowserDlg=new CUABrowserDlg(this,hSession);
					m_pUABrowserDlg->SetCallReason(OPCUA_ADD_MONITORED_ITEM);
					m_pUABrowserDlg->m_hApplication = m_hApplication;
					m_pUABrowserDlg->m_hSession = hSession;
					m_pUABrowserDlg->Create(IDD_UA_BROWSER_DIALOG, this);
					m_pUABrowserDlg->ShowWindow(SW_SHOW);
					SendMessage(WM_MOVE, 0, 0);
				}		
			}
		}
		else
		{
			AddLoggerMessage("Critical error please contact 4CE Industry (Code:B0001)", 0);
		}
	}
	else
	{
		AddLoggerMessage("Select the session to browse to",0);
	}
}
/// <summary>
/// Called when by the DLL when the connection client/server is facing a problem.
/// </summary>
/// <param name="hApplication">The h application.</param>
/// <param name="hSession">The h session.</param>
/// <param name="strShutdownMessage">The string shutdown message.</param>
/// <param name="extraParam">The extra parameter.</param>
/// <returns></returns>
OpcUa_StatusCode __stdcall CUAQuickClientDlg::OnShutdownMessage(HANDLE hApplication,HANDLE hSession,
																	OpcUa_String strShutdownMessage,
																	void* extraParam)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	//CString strMessage(OpcUa_String_GetRawString(&strShutdownMessage));
	//AfxMessageBox(strMessage);
	OpcUa_StatusCode uClientLibStatus;
	char Message[1024];
	ZeroMemory(Message, 1024);
	char* pszStatusCode = OpcUa_String_GetRawString(&strShutdownMessage);
	sscanf(pszStatusCode, "0x%05x", &uClientLibStatus);
	//OpcUa_StatusCode uClientLibStatus=(OpcUa_StatusCode)(extraParam);
	sprintf(Message, "%s 0x%05x\n", OpcUa_String_GetRawString(&strShutdownMessage), uClientLibStatus);
	CUAQuickClientDlg* pUAQuickClient = (CUAQuickClientDlg*)extraParam;
	pUAQuickClient->AddLoggerMessage(Message, 0);
	switch (uClientLibStatus)
	{
	case OpcUa_BadTimeout: // Read is not working any more
		// Based on the ShutdownMessage we got. It could be usefull to clean the ressource and update HMI
		uStatus = OpenOpcUa_CloseSession(pUAQuickClient->m_hApplication, hSession);
		if (uStatus == OpcUa_Good)
			::PostMessage(pUAQuickClient->m_hWnd, OPENOPCUA_SHUTDOWN, 0, 0);
		break;
	case OpcUa_BadShutdown: // Publish is not working any more
		// Based on the ShutdownMessage we got. It could be usefull to clean the ressource and update HMI
		uStatus = OpenOpcUa_CloseSession(pUAQuickClient->m_hApplication, hSession);
		if (uStatus == OpcUa_Good)
			::PostMessage(pUAQuickClient->m_hWnd, OPENOPCUA_SHUTDOWN, 0, 0);
		break;
	case OpcUa_BadInvalidState: // Publish is not working any more
		// Based on the ShutdownMessage we got. It could be usefull to clean the ressource and update HMI
		//uStatus = OpenOpcUa_CloseSession(pUAQuickClient->m_hApplication, hSession); // cannot be called from here
		//if (uStatus == OpcUa_Good)
			::PostMessage(pUAQuickClient->m_hWnd, OPENOPCUA_SHUTDOWN, 0, 0);
		break;
	}
	return uStatus;
}
void CUAQuickClientDlg::OnSubscriptionParamChange()
{
	OpcUa_Handle hSession=OpcUa_Null;
	OpcUa_Handle hSubscription = OpcUa_Null;
	OpcUa_StatusCode uStatus;
	HTREEITEM hItem=m_TreeSessionSubscription.GetSelectedItem();
	if (hItem)
		hSubscription = (OpcUa_Handle)m_TreeSessionSubscription.GetItemData(hItem);
	else
		return;
	if (hSubscription)
	{
		OpenOpcUa_HandleType aHandleType;
		uStatus=OpenOpcUa_WhatIsIt(hSubscription,&aHandleType);
		if (aHandleType==OPENOPCUA_SUBSCRIPTION)
		{
			uStatus=OpenOpcUa_GetSessionOfSubscription(hSubscription,&hSession);
			if (uStatus==OpcUa_Good)
			{
				// Get the current params of the subscription
				OpenOpcUa_SubscriptionParam subscriptionParams;
				uStatus = OpenOpcUa_GetSubscriptionParams(
					m_hApplication,
					hSession,
					hSubscription, &subscriptionParams);
				if (uStatus == OpcUa_Good)
				{
					CSubscriptionParamDlg dlg;
					dlg.m_bPublishingEnabled = subscriptionParams.bPublishingEnabled;
					dlg.m_dblPublishingInterval = subscriptionParams.dblPublishingInterval;
					dlg.m_uiLifeTimeCount = subscriptionParams.uiLifetimeCount;
					dlg.m_uiMaxNotificationPerPublish = subscriptionParams.uiMaxKeepAliveCount;
					dlg.m_Priority = subscriptionParams.aPriority;
					if (dlg.DoModal() == IDOK)
					{
						OpcUa_StatusCode uStatus = OpenOpcUa_ModifySubscription(
							m_hApplication,
							hSession,
							hSubscription,
							&(dlg.m_dblPublishingInterval),
							&(dlg.m_uiLifeTimeCount),
							&(dlg.m_uiKeepAliveCount),
							dlg.m_uiMaxNotificationPerPublish,
							dlg.m_Priority);
						if (uStatus == OpcUa_Good)
						{
							uStatus = OpenOpcUa_SetPublishingMode(
								m_hApplication,
								hSession,
								hSubscription,
								dlg.m_bPublishingEnabled);
							if (uStatus == OpcUa_Good)
							{
								if (dlg.m_bPublishingEnabled)
									m_TreeSessionSubscription.SetItemImage(hItem, 3, 3); // Active publish
								else
									m_TreeSessionSubscription.SetItemImage(hItem, 2, 2); // Inactive publish
							}
						}
					}
				}
			}
		}
	}
}

/// <summary>
/// Called when [attributes].
/// </summary>
void CUAQuickClientDlg::OnAttributes()
{
	HANDLE hSession=OpcUa_Null;
	HANDLE hSubscription=OpcUa_Null;
	OpcUa_StatusCode uStatus;
	OpenOpcUa_InternalNode* pInternalNode = OpcUa_Null;

	HTREEITEM hItem=m_TreeSessionSubscription.GetSelectedItem();
	if (hItem)
	{
		hSession = (HANDLE)m_TreeSessionSubscription.GetItemData(hItem);

		OpenOpcUa_HandleType aHandleType;
		uStatus = OpenOpcUa_WhatIsIt(hSession, &aHandleType);
		if (aHandleType == OPENOPCUA_SESSION)
		{
		}
		else
		{
			if (aHandleType == OPENOPCUA_SUBSCRIPTION)
			{
				hSubscription = hSession;
				hSession = OpcUa_Null;
				uStatus = OpenOpcUa_GetSessionOfSubscription(hSubscription, &hSession);

			}
		}
	}
	m_pNodeAttributesDlg=new CNodeAttributesDlg(this,hSession);
	m_pNodeAttributesDlg->m_hApplication=m_hApplication;
	m_pNodeAttributesDlg->m_hSession=hSession;
	m_pNodeAttributesDlg->Create(IDD_ATTRIBUTES_DIALOG,this);
	m_pNodeAttributesDlg->ShowWindow(SW_SHOW);
	// Get the current selected item in the list
	POSITION pos = m_MonitoredElts.GetFirstSelectedItemPosition();
	if (pos)
	{
		OpcUa_Int32 index = m_MonitoredElts.GetNextSelectedItem(pos);
		// Get the encapsulate OpenOpcUa_InternalNode
		pInternalNode = (OpenOpcUa_InternalNode*)m_MonitoredElts.GetItemData(index);
	}
	if (pInternalNode)
		m_pNodeAttributesDlg->DisplayAttributes(&(pInternalNode->m_NodeId));
}



/// <summary>
/// Called when [write].
// Function name   : CUAQuickClientDlg::OnWrite
// Description     : Write the current selected item using a dedicate dialogBox
// Return type     : void 
/// </summary>
void CUAQuickClientDlg::OnWrite()
{
	HANDLE hSession=OpcUa_Null;
	HANDLE hSubscription=OpcUa_Null;
	OpenOpcUa_HandleType aHandleType;
	OpcUa_StatusCode uStatus;
	OpenOpcUa_InternalNode* pInternalNode=OpcUa_Null;
	CUAWriteDlg* pWriteDlg=OpcUa_Null;
	OpcUa_ReadValueId* pNodesToRead=OpcUa_Null;
	OpcUa_DataValue* pResults=OpcUa_Null; // To read attribute
	OpcUa_WriteValue* pWriteValue=OpcUa_Null;
	OpcUa_StatusCode* pWriteResults=OpcUa_Null;

	// Get the current selected item in the list
	POSITION pos=m_MonitoredElts.GetFirstSelectedItemPosition();
	if (pos)
	{
		OpcUa_Int32 index=m_MonitoredElts.GetNextSelectedItem(pos);
		// Get the encapsulate OpenOpcUa_InternalNode
		pInternalNode=(OpenOpcUa_InternalNode*)m_MonitoredElts.GetItemData(index);
	}
	if (pInternalNode)
	{	
		// Check if we currently select a session or a subscription
		HTREEITEM hCurrentSession=m_TreeSessionSubscription.GetSelectedItem();
		if (hCurrentSession)
		{
			HANDLE hLocalSubscription=(HANDLE)m_TreeSessionSubscription.GetItemData(hCurrentSession);
			// Will start by checking if the nodeId is writeable
			// To do so will read Attribute OpcUa_Attributes_AccessLevel
			// First we need to retrieve the hSession of the selected object (session or subscription)
			uStatus=OpenOpcUa_WhatIsIt(hLocalSubscription,&aHandleType);
			if (aHandleType==OPENOPCUA_SUBSCRIPTION)
				uStatus=OpenOpcUa_GetSessionOfSubscription(hLocalSubscription,&hSession);
			else
				hSession = hLocalSubscription; 
			if (hSession)
			{
				if (uStatus==OpcUa_Good)
				{
					pNodesToRead=(OpcUa_ReadValueId*)OpcUa_Alloc(sizeof(OpcUa_ReadValueId));
					OpcUa_ReadValueId_Initialize(&pNodesToRead[0]);
					pNodesToRead[0].AttributeId=OpcUa_Attributes_AccessLevel;
					pNodesToRead[0].NodeId=pInternalNode->m_NodeId;
					uStatus = OpenOpcUa_ReadAttributes(m_hApplication, hSession, OpcUa_TimestampsToReturn_Both, 1, pNodesToRead, &pResults);
					if (uStatus == OpcUa_Good)
					{
						// access level is in a byte
						if ((pResults[0].Value.Value.Byte & 2) == 2)
						{
							pWriteDlg = new CUAWriteDlg(this, &(pInternalNode->m_NodeId), m_hApplication, hSession);
							if (pWriteDlg->DoModal() == IDOK)
							{
								// will processed the write
								pWriteValue = (OpcUa_WriteValue*)OpcUa_Alloc(sizeof(OpcUa_WriteValue));
								OpcUa_WriteValue_Initialize(pWriteValue);
								pWriteValue->AttributeId = OpcUa_Attributes_Value;
								pWriteValue->IndexRange;
								uStatus = pWriteDlg->GetValueToWrite(&pWriteValue);

								OpcUa_Int32 iNoOfNodesToWrite = 1;
								{
									// will now write the OpcUa_Variant to the OpcUa Server
									uStatus = OpenOpcUa_WriteAttributes(m_hApplication, hSession, iNoOfNodesToWrite, pWriteValue, &pWriteResults);
									if (uStatus != OpcUa_Good)
									{
										OpenOpcUa_Trace(m_hApplication, OPCUA_TRACE_CLIENT_ERROR, "Write failed 0x%05x.", uStatus);
										AddLoggerMessage("Write failed",0);
									}
								}
							}
						}
						else
							AddLoggerMessage("This node is ReadOnly",0);
					}
					else
					{
						char Message[128];
						ZeroMemory(Message, 128);
						sprintf(Message, "OpenOpcUa_ReadAttributes failed 0x%05x", uStatus);
						AddLoggerMessage(Message, 0);
					}
				}
			}
		}
	}

	if (pNodesToRead)
	{
		OpcUa_ReadValueId_Clear(pNodesToRead);
		OpcUa_Free(pNodesToRead);
	}
}

void CUAQuickClientDlg::OnDelete()
{
	HTREEITEM hCurrentSession = m_TreeSessionSubscription.GetSelectedItem();
	OpenOpcUa_HandleType aHandleType;
	HANDLE hSession = OpcUa_Null;
	OpenOpcUa_InternalNode* pInternalNode = OpcUa_Null;
	OpcUa_Int32 index = 0;
	OpcUa_StatusCode uStatus = OpcUa_Good;
	// Get the current selected item in the list
	POSITION pos = m_MonitoredElts.GetFirstSelectedItemPosition();
	if (pos)
	{
		index = m_MonitoredElts.GetNextSelectedItem(pos);
		// Get the encapsulate OpenOpcUa_InternalNode
		pInternalNode = (OpenOpcUa_InternalNode*)m_MonitoredElts.GetItemData(index);
	}
	if (pInternalNode)
	{
		HANDLE hLocalSubscription = (HANDLE)m_TreeSessionSubscription.GetItemData(hCurrentSession);
		uStatus = OpenOpcUa_WhatIsIt(hLocalSubscription, &aHandleType);
		if (aHandleType == OPENOPCUA_SUBSCRIPTION)
		{
			uStatus = OpenOpcUa_GetSessionOfSubscription(hLocalSubscription, &hSession);
			OpcUa_StatusCode* pStatusCode = OpcUa_Null;
			uStatus = OpenOpcUa_DeleteMonitoredItems(m_hApplication, hSession, hLocalSubscription, 1, &(pInternalNode->m_hMonitoredItem), &pStatusCode);
			if (uStatus == OpcUa_Good)
			{
				OpenOpcUa_InternalNode* pInternalNode=(OpenOpcUa_InternalNode*)m_MonitoredElts.GetItemData(index);
				if (pInternalNode)
					OpcUa_Free(pInternalNode);
				m_MonitoredElts.DeleteItem(index);
			}
			else
				AddLoggerMessage("OpenOpcUa_DeleteMonitoredItems failed",0);
		}
		else
		{
			hSession = hLocalSubscription;
			hLocalSubscription = OpcUa_Null;
			uStatus = OpenOpcUa_GetSubscriptionOfMonitoredItem(m_hApplication, hSession, pInternalNode->m_hMonitoredItem, &hLocalSubscription);
			if (uStatus == OpcUa_Good)
			{

				OpcUa_StatusCode* pStatusCode = OpcUa_Null;
				uStatus = OpenOpcUa_DeleteMonitoredItems(m_hApplication, hSession, hLocalSubscription, 1, &(pInternalNode->m_hMonitoredItem), &pStatusCode);
				if (uStatus == OpcUa_Good)
				{
					OpenOpcUa_InternalNode* pInternalNode = (OpenOpcUa_InternalNode*)m_MonitoredElts.GetItemData(index);
					if (pInternalNode)
						OpcUa_Free(pInternalNode);
					m_MonitoredElts.DeleteItem(index);
				}
				else
					AddLoggerMessage("OpenOpcUa_DeleteMonitoredItems failed", 0);
			}
		}
	}
}

/// <summary>
/// Creates the session.
/// </summary>
/// <param name="pEndpointDescription">The p endpoint description.</param>
/// <returns></returns>
OpcUa_StatusCode CUAQuickClientDlg::CreateSession(OpcUa_EndpointDescription* pEndpointDescription)
{
	OpcUa_StatusCode uStatus=OpcUa_Bad;
	if (pEndpointDescription)
	{
		OpcUa_String aSessionName;
		OpcUa_String_Initialize(&aSessionName);
		OpcUa_String_AttachCopy(&aSessionName,(OpcUa_CharA*)"UAQuickClient");
		HANDLE hSession;
		CSessionParamDlg dlg;
		if (dlg.DoModal()==IDOK)
		{
			uStatus=OpenOpcUa_CreateSession(m_hApplication,pEndpointDescription,dlg.m_dblSessionTimeout,aSessionName,&hSession);
			if (uStatus==OpcUa_Good)
			{
				uStatus=OpenOpcUa_GetEndpointDescription(m_hApplication,hSession,&pEndpointDescription);
				USES_CONVERSION;
				CString strTmp;
				strTmp.Format(L"%ls",A2W(OpcUa_String_GetRawString(&(pEndpointDescription->EndpointUrl))));
				HTREEITEM hItem=m_TreeSessionSubscription.InsertItem(strTmp,TVI_ROOT);
				m_TreeSessionSubscription.SetItemImage(hItem,0,0);
				m_TreeSessionSubscription.SetItemData(hItem,(DWORD_PTR)hSession);
				// Mise a jour des boutons
				HICON  hIcon=AfxGetApp()->LoadIcon(IDI_DISCONNECT);
				m_btConnectDisconnect.SetIcon(hIcon);
				m_btConnectDisconnect.SetWindowText(L"Disconnect");
				// change the tooltips message for the connect/disconenct button
				m_Tooltip.DelTool(GetDlgItem(IDC_BUTTON_CONNECT));
				m_Tooltip.AddTool(GetDlgItem(IDC_BUTTON_CONNECT),L"Disonnect from UAServer");
				m_bConnected=OpcUa_True;
				// On va activer la session
				if (dlg.m_bActive)
				{
					uStatus = OpenOpcUa_ActivateSession(m_hApplication, hSession, pEndpointDescription,"","","");
					if (uStatus==OpcUa_Good)
						m_TreeSessionSubscription.SetItemImage(hItem,1,1);
				}
				// Connection de la fonction callback en charge du shutdown
				OpenOpcUa_SetShutdownCallback(m_hApplication,hSession,(PFUNCSHUTDOWN)CUAQuickClientDlg::OnShutdownMessage,this);
				m_szEndpointDescription = CString(OpcUa_String_GetRawString(&(pEndpointDescription->EndpointUrl)));
				UpdateData(FALSE);
			}
			else
			{
				AddLoggerMessage("OpenOpcUa_CreateSession failed", 0);
			}
		}
	}
	return uStatus;
}

/// <summary>
/// Called when [read attributes].
/// </summary>
void CUAQuickClientDlg::OnReadAttributes()
{
	HANDLE hSession=OpcUa_Null;
	OpcUa_StatusCode uStatus;
	m_bBrowseOrExport=OpcUa_True;
	HTREEITEM hItem=m_TreeSessionSubscription.GetSelectedItem();
	if (hItem)
	{
		hSession=(HANDLE)m_TreeSessionSubscription.GetItemData(hItem);
		if (hSession)
		{
			OpenOpcUa_HandleType aHandleType;
			uStatus=OpenOpcUa_WhatIsIt(hSession,&aHandleType);
			if (aHandleType==OPENOPCUA_SESSION)
			{
				CUABrowserDlg*pDlg=new CUABrowserDlg(this,hSession);
				pDlg->SetCallReason(OPCUA_READ);
				pDlg->m_hApplication=m_hApplication;
				pDlg->m_hSession=hSession;
				if (pDlg->DoModal()==IDOK)
				{
					;//TRACE(L"Browse done\n");
				}
				delete pDlg;
			}		
		}
		else
		{
			CString str;
			str.Format(L"Critical error please contact 4CE Industry (Code:B0001)");
		}
	}
	else
	{
		CString str;
		str.Format(L"Select the session to read on to");
	}
	m_bBrowseOrExport=OpcUa_False;
}



/// <summary>
/// Called when [write attributes].
// Function name   : CUAQuickClientDlg::OnWriteAttributes
// Description     : Write through the UABrowser dialog
// Return type     : void 
/// </summary>
void CUAQuickClientDlg::OnWriteAttributes()
{
	HANDLE hSession=OpcUa_Null;
	OpcUa_StatusCode uStatus;
	m_bBrowseOrExport=OpcUa_True;
	HTREEITEM hItem=m_TreeSessionSubscription.GetSelectedItem();
	if (hItem)
	{
		hSession=(HANDLE)m_TreeSessionSubscription.GetItemData(hItem);
		if (hSession)
		{
			OpenOpcUa_HandleType aHandleType;
			uStatus=OpenOpcUa_WhatIsIt(hSession,&aHandleType);
			if (aHandleType==OPENOPCUA_SESSION)
			{
				CUABrowserDlg*pDlg=new CUABrowserDlg(this,hSession);
				pDlg->SetCallReason(OPCUA_WRITE);
				pDlg->m_hApplication=m_hApplication;
				pDlg->m_hSession=hSession;
				pDlg->DoModal();
				delete pDlg;
			}		
		}
		else
		{
			CString str;
			str.Format(L"Critical error please contact 4CE Industry (Code:B0001)");
		}
	}
	else
	{
		CString str;
		str.Format(L"Select the session to read on to");
	}
	m_bBrowseOrExport=OpcUa_False;
}
BOOL CUAQuickClientDlg::PreTranslateMessage(MSG* pMsg)
{
	m_Tooltip.RelayEvent(pMsg);

	return CDialog::PreTranslateMessage(pMsg);
}
void CUAQuickClientDlg::OnMove(int x, int y)
{
	CDialog::OnMove(x, y);

	CRect rect;
	GetWindowRect(&rect);
	OpcUa_Int16 iWindowsNumber = 0;
	// Ajust the position of the Attribute Dlg
	if ( m_pNodeAttributesDlg  )
	{
		m_pNodeAttributesDlg->SetWindowPos(
			&CWnd::wndTop,
			rect.right, rect.top, 0, 0,
			SWP_NOSIZE | SWP_SHOWWINDOW);
		iWindowsNumber++;
	}
	if (m_pUABrowserDlg)
	{
		if (iWindowsNumber == 0)
		{
			m_pUABrowserDlg->SetWindowPos(
				&CWnd::wndTop,
				rect.right,rect.top,0,0,
				SWP_NOSIZE|SWP_SHOWWINDOW );
		}
		if (iWindowsNumber == 1)
		{
			CRect rectNodeAttributesDlg;
			m_pNodeAttributesDlg->GetWindowRect(&rectNodeAttributesDlg);
			m_pUABrowserDlg->SetWindowPos(
				&CWnd::wndTop,
				rect.right, rectNodeAttributesDlg.bottom, rect.right, 0,
				SWP_NOSIZE|SWP_SHOWWINDOW );
		}
		iWindowsNumber++;
	}
	if (m_pMessageDlg)
	{
		m_pMessageDlg->SetWindowPos(
			&CWnd::wndTop,
			rect.left,rect.bottom,0,0,
			SWP_NOSIZE|SWP_SHOWWINDOW );
		iWindowsNumber++;
	}
	UpdateWindow( );
}

/// <summary>
/// Called when [nm click monitored itemctrl].
/// </summary>
/// <param name="pNMHDR">The p NMHDR.</param>
/// <param name="pResult">The p result.</param>
void CUAQuickClientDlg::OnNMClickMonitoredItemctrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	if (m_pNodeAttributesDlg)
	{
		OpenOpcUa_InternalNode* pInternalNode = OpcUa_Null;
		if (pNMItemActivate->iItem > 0)
		{
			pInternalNode = (OpenOpcUa_InternalNode*)m_MonitoredElts.GetItemData(pNMItemActivate->iItem);
			if (pInternalNode)
			{
				OpcUa_NodeId sourceNodeId = pInternalNode->m_NodeId;
				m_pNodeAttributesDlg->DisplayAttributes(&sourceNodeId);

				SendMessage(WM_MOVE, 0, 0);
			}
		}
	}
	*pResult = 0;
}

/// <summary>
/// Called when [nm click tree session subscription].
/// </summary>
/// <param name="pNMHDR">The p NMHDR.</param>
/// <param name="pResult">The p result.</param>
void CUAQuickClientDlg::OnNMClickTreeSessionSubscription(NMHDR *pNMHDR, LRESULT *pResult)
{

	*pResult = 0;
}

/// <summary>
/// Called when [TVN selchanged tree session subscription].
/// </summary>
/// <param name="pNMHDR">The p NMHDR.</param>
/// <param name="pResult">The p result.</param>
void CUAQuickClientDlg::OnTvnSelchangedTreeSessionSubscription(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	HTREEITEM hCurrentSession=m_TreeSessionSubscription.GetSelectedItem();
	OpenOpcUa_HandleType aHandleType=OPENOPCUA_UNKNOWN;
	HANDLE hSession = OpcUa_Null;
	if (hCurrentSession)
	{
		HANDLE hLocalSubscription=(HANDLE)m_TreeSessionSubscription.GetItemData(hCurrentSession);
		OpcUa_StatusCode uStatus=OpenOpcUa_WhatIsIt(hLocalSubscription,&aHandleType);
		if (aHandleType==OPENOPCUA_SUBSCRIPTION)
		{
			m_btConnectDisconnect.EnableWindow(FALSE);
			// delete all current viewed monitoredItems
			m_MonitoredElts.DeleteAllItems();
			// First retrieve the session of this subscription 
			OpcUa_Handle hSessionOfSubscription;
			if (OpenOpcUa_GetSessionOfSubscription(hLocalSubscription, &hSessionOfSubscription) == OpcUa_Good)
			{
				if (m_pUABrowserDlg)
					m_pUABrowserDlg->SetSession(hSessionOfSubscription);
				// Retrieve the monitoredItem for hLocalSubscription
				OpcUa_UInt32 uiNoOfMonitoredItems = 0;
				OpcUa_Handle* hMonitoredItems = OpcUa_Null;

				uStatus = OpenOpcUa_GetMonitoredItems(m_hApplication, hSessionOfSubscription, hLocalSubscription, &uiNoOfMonitoredItems, &hMonitoredItems);
				if (uStatus == OpcUa_Good)
				{
					// Show the MonitoredItems in the ListCtrl
					for (OpcUa_UInt32 iii = 0; iii < uiNoOfMonitoredItems; iii++)
					{
						OpenOpcUa_InternalNode* pInternalNode = OpcUa_Null;
						uStatus = OpenOpcUa_GetInternalNode(m_hApplication, hSessionOfSubscription, hLocalSubscription, (OpcUa_Handle)hMonitoredItems[iii], &pInternalNode);
						if (uStatus == OpcUa_Good)
						{
							InsertMonitoredItemInListCtrl(pInternalNode);
						}
						else
						{
							USES_CONVERSION;
							CString message;
							message.Format(L"OnFileLoad>OpenOpcUa_GetInternalNode failed uStatus=0x%05x", uStatus);
							AddLoggerMessage(W2A(message.GetBuffer()), 0);
							message.Empty(); message.ReleaseBuffer();
						}
					}
				}
				if (hMonitoredItems)
					OpcUa_Free(hMonitoredItems);
			}

		}
		if (aHandleType==OPENOPCUA_SESSION)
		{
			m_btConnectDisconnect.EnableWindow(TRUE);
			hSession = hLocalSubscription;
			if (m_pUABrowserDlg)
				m_pUABrowserDlg->SetSession(hSession);
		}
	}
	*pResult = 0;
}
/// <summary>
/// Called when [remove subscription].
/// </summary>
void CUAQuickClientDlg::OnRemoveSubscription()
{
	HTREEITEM hCurrentSession=m_TreeSessionSubscription.GetSelectedItem();
	OpenOpcUa_HandleType aHandleType=OPENOPCUA_UNKNOWN;
	HANDLE hSession = OpcUa_Null;
	if (hCurrentSession)
	{
		HANDLE hLocalSubscription=(HANDLE)m_TreeSessionSubscription.GetItemData(hCurrentSession);
		OpcUa_StatusCode uStatus=OpenOpcUa_WhatIsIt(hLocalSubscription,&aHandleType);
		if (aHandleType==OPENOPCUA_SUBSCRIPTION)
		{
			uStatus=OpenOpcUa_GetSessionOfSubscription(hLocalSubscription,&hSession);
			if (uStatus==OpcUa_Good)
			{
				uStatus=OpenOpcUa_DeleteSubscription(m_hApplication,hSession,hLocalSubscription);
				if (uStatus==OpcUa_Good)
				{
					m_TreeSessionSubscription.DeleteItem(hCurrentSession);
					m_MonitoredElts.DeleteAllItems();
				}
			}
		}
	}
}
/// <summary>
/// Called when [file exit].
/// </summary>
void CUAQuickClientDlg::OnFileExit()
{
	CDialog::OnOK();
}
/// <summary>
/// Called when [file load].
/// </summary>
void CUAQuickClientDlg::OnFileLoad()
{
	HRESULT hr = S_FALSE;
	OpcUa_StatusCode uStatus = OpcUa_Good;
	OpcUa_UInt32 uiNoOfSessions = 0;
	OpcUa_Handle* hSessions = OpcUa_Null;//  (OpcUa_Handle*)OpcUa_Alloc(sizeof(OpcUa_Handle));
	LPCTSTR szFilter = _T("XML. files (*.xml)|*.xml||/All Files (*.*)|*.*||");
	DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	CString XMLFileName;
	CString message;
	CFileDialog* pfDlg = new CFileDialog(TRUE,
		_T("xml"), _T(".xml"), dwFlags, szFilter, this);
	USES_CONVERSION;
	if (pfDlg->DoModal() == IDOK)
	{
		m_strXMLFileName = pfDlg->GetPathName();
		m_strXMLFileName = pfDlg->m_ofn.lpstrFile;
		hr = S_OK;
	}
	OpcUa_String ConfigFileName;
	OpcUa_String_Initialize(&ConfigFileName);
	OpcUa_String_AttachCopy(&ConfigFileName, W2A(m_strXMLFileName.GetBuffer()));
	uStatus = OpenOpcUa_LoadConfig(m_hApplication, ConfigFileName);
	if (uStatus == OpcUa_Good)
	{
		// retrieve the sessions for the application
		uStatus = OpenOpcUa_GetSessions(m_hApplication, &uiNoOfSessions, &hSessions);
		if (uStatus == OpcUa_Good)
		{
			// Show the session in the treeCtrl
			for (OpcUa_UInt32 i = 0; i < uiNoOfSessions; i++)
			{
				OpcUa_EndpointDescription* pEndpointDescription = OpcUa_Null;
				uStatus = OpenOpcUa_GetEndpointDescription(m_hApplication, hSessions[i], &pEndpointDescription);
				USES_CONVERSION;
				CString strTmp;
				strTmp.Format(L"%ls", A2W(OpcUa_String_GetRawString(&(pEndpointDescription->EndpointUrl))));
				m_szEndpointDescription = CString(OpcUa_String_GetRawString(&(pEndpointDescription->EndpointUrl)));
				UpdateData(FALSE);
				HTREEITEM hItem = m_TreeSessionSubscription.InsertItem(strTmp, TVI_ROOT);
				m_TreeSessionSubscription.SetItemImage(hItem, 1, 1); // Session active
				m_TreeSessionSubscription.SetItemData(hItem, (DWORD_PTR)hSessions[i]);
				// Mise a jour des boutons
				HICON  hIcon = AfxGetApp()->LoadIcon(IDI_DISCONNECT);
				m_btConnectDisconnect.SetIcon(hIcon);
				m_btConnectDisconnect.SetWindowText(L"Disconnect");
				// change the tooltips message for the connect/disconenct button
				m_Tooltip.DelTool(GetDlgItem(IDC_BUTTON_CONNECT));
				m_Tooltip.AddTool(GetDlgItem(IDC_BUTTON_CONNECT), L"Disonnect from UAServer");
				m_bConnected = OpcUa_True;
				// Retrieve the subscription for each session
				OpcUa_UInt32 uiNoOfSubscriptions = 0;
				OpcUa_Handle* hSubscriptions = OpcUa_Null;// (OpcUa_Handle*)OpcUa_Alloc(sizeof(OpcUa_Handle));
				uStatus = OpenOpcUa_GetSubscriptions(m_hApplication, hSessions[i], &uiNoOfSubscriptions, &hSubscriptions);
				if (uStatus == OpcUa_Good)
				{
					// Show the subscription in the treeCtrl
					for (OpcUa_UInt32 ii = 0; ii < uiNoOfSubscriptions; ii++)
					{
						// Get the current params of the subscription
						OpenOpcUa_SubscriptionParam subscriptionParams;
						uStatus = OpenOpcUa_GetSubscriptionParams(
							m_hApplication,
							hSessions[i],
							hSubscriptions[ii], &subscriptionParams);
						if (uStatus == OpcUa_Good)
						{
							CString strTmp;
							strTmp.Format(L"Subscription-%05lf", subscriptionParams.dblPublishingInterval);
							HTREEITEM hNewItem = m_TreeSessionSubscription.InsertItem(strTmp, hItem);
							if (subscriptionParams.bPublishingEnabled)
								m_TreeSessionSubscription.SetItemImage(hNewItem, 3, 3); // Active publish
							else
								m_TreeSessionSubscription.SetItemImage(hNewItem, 2, 2); // Inactive publish

							m_TreeSessionSubscription.SetItemData(hNewItem, (DWORD_PTR)hSubscriptions[ii]);
							// we want to expand the subscription
							m_TreeSessionSubscription.Expand(hItem, TVE_EXPAND);
							m_TreeSessionSubscription.SelectItem(hNewItem);
							OpenOpcUa_SetPublishCallback(m_hApplication, hSessions[i], hSubscriptions[ii], (PFUNC)CUAQuickClientDlg::OnNotificationMessage, (void*)this);
							// Retrieve the monitoredItem for each subscription
							OpcUa_UInt32 uiNoOfMonitoredItems = 0;
							OpcUa_Handle* hMonitoredItems = OpcUa_Null;

							uStatus = OpenOpcUa_GetMonitoredItems(m_hApplication, hSessions[i], hSubscriptions[ii], &uiNoOfMonitoredItems, &hMonitoredItems);
							if (uStatus == OpcUa_Good)
							{

								// Connection de la fonction callback en charge du shutdown
								OpenOpcUa_SetShutdownCallback(m_hApplication, hSessions[i], (PFUNCSHUTDOWN)CUAQuickClientDlg::OnShutdownMessage, this);
							}
							if (hMonitoredItems)
								OpcUa_Free(hMonitoredItems);
						}
					}
				}
				if (hSubscriptions)
					OpcUa_Free(hSubscriptions);
			}
		}
		if (hSessions)
			OpcUa_Free(hSessions);
		SetWindowText(CString("OpenOpcUa QuickClient:[") + m_strXMLFileName + CString("]"));
	}
	else
		AddLoggerMessage("OpenOpcUa_LoadConfig failed. look at you log file for more detail", 1);
}

/// <summary>
/// Called when [file save].
/// </summary>
void CUAQuickClientDlg::OnFileSave()
{
	HRESULT hr = S_FALSE;
	LPCTSTR szFilter = _T("XML. files (*.xml)|*.xml||/All Files (*.*)|*.*||");
	DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	CString XMLFileName;
	CString message;
	CFileDialog* pfDlg = new CFileDialog(FALSE,
		_T("xml"), _T(".xml"), dwFlags, szFilter, this);
	USES_CONVERSION;
	if (pfDlg->DoModal() == IDOK)
	{
		m_strXMLFileName = pfDlg->GetPathName();
		m_strXMLFileName = pfDlg->m_ofn.lpstrFile;
		hr = S_OK;
		OpcUa_String ConfigFileName;
		OpcUa_String_Initialize(&ConfigFileName);
		OpcUa_String_AttachCopy(&ConfigFileName,W2A(m_strXMLFileName.GetBuffer()));
		OpenOpcUa_SaveConfig(m_hApplication,ConfigFileName);
		SetWindowText(CString("OpenOpcUa QuickClient:[")+m_strXMLFileName+CString("]"));
	}
}

/// <summary>
/// Called when [view message].
/// </summary>
void CUAQuickClientDlg::OnViewMessage()
{
	CMenu* pMenu = GetMenu();
	if (pMenu)
	{
		CMenu* pSubMenu = pMenu->GetSubMenu(1);
		if (m_pMessageDlg == OpcUa_Null)
		{
			pSubMenu->CheckMenuItem(ID_VIEW_MESSAGE, MF_CHECKED);

			m_pMessageDlg = new CMessageDlg(this);
			m_pMessageDlg->Create(IDD_MESSAGE_DLG, this);
			m_pMessageDlg->ShowWindow(SW_SHOW);
			AddLoggerMessage("Logger Message activated",2);
		}
		else
		{
			pSubMenu->CheckMenuItem(ID_VIEW_MESSAGE, MF_UNCHECKED);
			m_pMessageDlg->OnClose();
			delete m_pMessageDlg;
			m_pMessageDlg = OpcUa_Null;
		}

	}
	SendMessage(WM_MOVE, 0, 0);
}

/// <summary>
/// Called when [view attribute].
/// </summary>
void CUAQuickClientDlg::OnViewAttribute()
{
	// Get the current selected item in the list
	OpenOpcUa_InternalNode* pInternalNode = OpcUa_Null;
	POSITION pos = m_MonitoredElts.GetFirstSelectedItemPosition();
	if (pos)
	{
		OpcUa_Int32 index = m_MonitoredElts.GetNextSelectedItem(pos);
		// Get the encapsulate OpenOpcUa_InternalNode
		pInternalNode = (OpenOpcUa_InternalNode*)m_MonitoredElts.GetItemData(index);
	}
	CMenu* pMenu = GetMenu();
	if (pMenu)
	{
		CMenu* pSubMenu = pMenu->GetSubMenu(1);
		if (m_pNodeAttributesDlg == OpcUa_Null)
		{
			pSubMenu->CheckMenuItem(ID_VIEW_ATTRIBUTE, MF_CHECKED);
			OnAttributes();
		}
		else
		{
			pSubMenu->CheckMenuItem(ID_VIEW_ATTRIBUTE, MF_UNCHECKED);
			m_pNodeAttributesDlg->OnClose();
			delete m_pNodeAttributesDlg;
			m_pNodeAttributesDlg = OpcUa_Null;
		}
	}
	SendMessage(WM_MOVE, 0, 0);

}
/// <summary>
/// Adds the logger message.
/// </summary>
/// <param name="message">The message.</param>
/// <param name="uiLevel">The level of the message 0 for the most critical. (always printed at least in the file)</param>
void CUAQuickClientDlg::AddLoggerMessage(char* message, OpcUa_UInt16 uiLevel)
{
	OpcUa_String Message;
	OpcUa_String_Initialize(&Message);
	OpcUa_String_AttachCopy(&Message, message);
	if (m_pMessageDlg)
		m_pMessageDlg->AddLoggerMessage(Message, uiLevel);
	else
	{
		if (uiLevel==0) // 0 most critical
			OpcUa_Trace(OPCUA_TRACE_LEVEL_ERROR, "%s\n", message);
	}
	OpcUa_String_Clear(&Message);
}


void CUAQuickClientDlg::OnDiscoverServer()
{
	CDiscoveryDlg dlg(this,m_hApplication);
	dlg.DoModal();
	OpcUa_EndpointDescription* pEndpointDescription = dlg.GetSelectedEndpointDescription();
	if (pEndpointDescription)
	{
		m_pEndpointDescription = pEndpointDescription;
		m_szEndpointDescription = CString(OpcUa_String_GetRawString(&(pEndpointDescription->EndpointUrl)));
		// MessageSecurityMode
		CString MessageSecurityMode;
		if (pEndpointDescription->SecurityMode == OpcUa_MessageSecurityMode_None)
			MessageSecurityMode = CString("[None - ");
		if (pEndpointDescription->SecurityMode == OpcUa_MessageSecurityMode_Sign)
			MessageSecurityMode = CString("[Sign - ");
		if (pEndpointDescription->SecurityMode == OpcUa_MessageSecurityMode_SignAndEncrypt)
			MessageSecurityMode = CString("[SignAndEncrypt - ");
		m_szEndpointDescription.Append(MessageSecurityMode);
		// SecurityPolicyUri
		m_szEndpointDescription.Append(CString(OpcUa_String_GetRawString(&(pEndpointDescription->SecurityPolicyUri))));
		m_szEndpointDescription.Append(CString("]"));
	}
	UpdateData(FALSE);
}

void CUAQuickClientDlg::OnHdnEnddragMonitoredItemctrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	// TODO: ajoutez ici le code de votre gestionnaire de notification de contrôle
	*pResult = 0;
}
void CUAQuickClientDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: ajoutez ici le code de votre gestionnaire de messages et/ou les paramètres par défaut des appels

	CDialog::OnLButtonUp(nFlags, point);
}


void CUAQuickClientDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	UINT uFlags;
	int hitem = m_MonitoredElts.HitTest(point, &uFlags);

	if ((hitem != -1) && (TVHT_ONITEM & uFlags))
	{
		//m_TreeCtrl.SelectItem(hItem);
	}


	CDialog::OnMouseMove(nFlags, point);
}

void CUAQuickClientDlg::OnViewBrowser()
{
	OpcUa_StatusCode uStatus;
	OpcUa_Handle hSubscription = OpcUa_Null;
	OpcUa_Handle hSession = OpcUa_Null;
	CMenu* pMenu = GetMenu();
	if (pMenu)
	{
		CMenu* pSubMenu = pMenu->GetSubMenu(1);
		if (m_pUABrowserDlg == OpcUa_Null)
		{
			HTREEITEM hItem = m_TreeSessionSubscription.GetSelectedItem();
			if (hItem)
				hSubscription = (HANDLE)m_TreeSessionSubscription.GetItemData(hItem);

			if (hSubscription)
			{
				OpenOpcUa_HandleType aHandleType;
				uStatus = OpenOpcUa_WhatIsIt(hSubscription, &aHandleType);
				if (aHandleType == OPENOPCUA_SUBSCRIPTION)
					uStatus = OpenOpcUa_GetSessionOfSubscription(hSubscription, &hSession);
				if (aHandleType == OPENOPCUA_SESSION)
					hSession = hSubscription;
			}
			pSubMenu->CheckMenuItem(ID_VIEW_BROWSER, MF_CHECKED);
			m_pUABrowserDlg = new CUABrowserDlg(this, hSession);
			m_pUABrowserDlg->m_hApplication = m_hApplication;
			m_pUABrowserDlg->SetCallReason(OPCUA_ADD_MONITORED_ITEM);
			m_pUABrowserDlg->Create(IDD_UA_BROWSER_DIALOG, this);
			m_pUABrowserDlg->ShowWindow(SW_SHOW);
		}
		else
		{
			pSubMenu->CheckMenuItem(ID_VIEW_BROWSER, MF_UNCHECKED);
			m_pUABrowserDlg->OnClose();
			delete m_pUABrowserDlg;
			m_pUABrowserDlg = OpcUa_Null;
		}
	}
	SendMessage(WM_MOVE, 0, 0);
}


LRESULT CUAQuickClientDlg::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case OPENOPCUA_SHUTDOWN:
		{
			m_btConnectDisconnect.SetWindowText(L"Connect");
			m_btConnectDisconnect.EnableWindow(TRUE);
			// change the tooltips message for the connect/disconenct button
			m_Tooltip.DelTool(GetDlgItem(IDC_BUTTON_CONNECT));
			m_Tooltip.AddTool(GetDlgItem(IDC_BUTTON_CONNECT), L"Connect to UAServer");
			HICON  hIcon = AfxGetApp()->LoadIcon(IDI_CONNECT);
			m_btConnectDisconnect.SetIcon(hIcon);

			m_TreeSessionSubscription.DeleteAllItems();
			m_bConnected = OpcUa_False;
			//// clear the monitored elementList
			m_MonitoredElts.DeleteAllItems();
			//// clear the NodeAttributeDlg
			if (m_pNodeAttributesDlg)
			{
				m_pNodeAttributesDlg->DeleteAllAttributes();
			}
			if (m_pUABrowserDlg)
			{
				m_pUABrowserDlg->m_ReferencesTreeCtrl.DeleteAllItems();
				m_pUABrowserDlg->m_NodeAttributesCtrl.DeleteAllItems();
			}
			m_szEndpointDescription.Empty();
			delete m_pEndpointDescription;
			m_pEndpointDescription = OpcUa_Null;
			UpdateData(FALSE);
		}
		break;
	default:
		break;
	}

	return CDialog::DefWindowProc(message, wParam, lParam);
}

void CUAQuickClientDlg::OnBnClickedButtonBrowse()
{
	OnViewBrowser();
}


void CUAQuickClientDlg::OnBnClickedButtonAttribute()
{
	OnViewAttribute();
}


void CUAQuickClientDlg::OnBnClickedButtonMessage()
{
	OnViewMessage();
}


void CUAQuickClientDlg::OnBnClickedButtonExit()
{
	OnFileExit();
}

void CUAQuickClientDlg::OnModifyMonitoredItem()
{
	HTREEITEM hCurrentSession = m_TreeSessionSubscription.GetSelectedItem();
	OpenOpcUa_HandleType aHandleType;
	HANDLE hSession = OpcUa_Null;
	OpenOpcUa_InternalNode* pInternalNode = OpcUa_Null;
	OpcUa_Int32 index = 0;
	OpcUa_StatusCode uStatus = OpcUa_Good;
	// Get the current selected item in the list
	POSITION pos = m_MonitoredElts.GetFirstSelectedItemPosition();
	if (pos)
	{
		index = m_MonitoredElts.GetNextSelectedItem(pos);
		// Get the encapsulate OpenOpcUa_InternalNode
		pInternalNode = (OpenOpcUa_InternalNode*)m_MonitoredElts.GetItemData(index);
	}
	if (pInternalNode)
	{
		HANDLE hLocalSubscription = (HANDLE)m_TreeSessionSubscription.GetItemData(hCurrentSession);
		uStatus = OpenOpcUa_WhatIsIt(hLocalSubscription, &aHandleType);
		if (aHandleType == OPENOPCUA_SUBSCRIPTION)
		{
			uStatus = OpenOpcUa_GetSessionOfSubscription(hLocalSubscription, &hSession);
			OpcUa_StatusCode* pStatusCode = OpcUa_Null;
			OpcUa_MonitoringMode MonitoringMode;
			CMonitoredItemParamsDlg dlg;
			// Init value in the DialogBox
			dlg.m_NodeId;
			OpcUa_String szNodeId;
			OpcUa_String_Initialize(&szNodeId);
			if (OpenOpcUa_NodeIdToString(pInternalNode->m_NodeId, &szNodeId) == OpcUa_Good)
			{
				dlg.m_NodeId=CString(OpcUa_String_GetRawString(&szNodeId));
				if (pInternalNode->m_pMonitoredItemParam)
				{
					// DiscardOldest
					dlg.m_bDiscardOldest = pInternalNode->m_pMonitoredItemParam->m_bDiscardOldest;
					// DeadbandType
					switch (pInternalNode->m_pMonitoredItemParam->m_DeadbandType)
					{
					case OpcUa_DeadbandType_None:
						dlg.m_szDeadbandType = CString("None");
						break;
					case OpcUa_DeadbandType_Absolute:
						dlg.m_szDeadbandType = CString("Absolute");
						break;
					case OpcUa_DeadbandType_Percent:
						dlg.m_szDeadbandType = CString("Percent");
						break;
					default:
						break;
					}
					// DeadbandValue
					dlg.m_dblDeadbandValue = pInternalNode->m_pMonitoredItemParam->m_dblDeadbandValue;
					// DataChangeTrigger
					switch (pInternalNode->m_pMonitoredItemParam->m_DataChangeTrigger)
					{
					case OpcUa_DataChangeTrigger_Status:
						dlg.m_bTriggerStatusCode = OpcUa_True;
						break;
					case OpcUa_DataChangeTrigger_StatusValueTimestamp:
						dlg.m_bTriggerStatusCode = OpcUa_True;
						dlg.m_bTriggerTimestamp = OpcUa_True;
						dlg.m_bTriggerValue = OpcUa_True;
						break;
					case OpcUa_DataChangeTrigger_StatusValue:
						dlg.m_bTriggerStatusCode = OpcUa_True;
						dlg.m_bTriggerValue = OpcUa_True;
						break;
					default:
						break;
					}
					// QueueSize
					dlg.m_uiQueueSize = pInternalNode->m_pMonitoredItemParam->m_uiQueueSize;
					// TimestampsToReturn
					switch (pInternalNode->m_pMonitoredItemParam->m_aTimestampsToReturn)
					{
					case OpcUa_TimestampsToReturn_Source:
						dlg.m_szTimestampToReturn = CString("Source");
						break;
					case OpcUa_TimestampsToReturn_Server:
						dlg.m_szTimestampToReturn = CString("Server");
						break;
					case OpcUa_TimestampsToReturn_Both:
						dlg.m_szTimestampToReturn = CString("Both");
						break;
					case OpcUa_TimestampsToReturn_Neither:
						dlg.m_szTimestampToReturn = CString("Neither");
						break;
					default:
						break;
					}
					// SamplingInterval
					dlg.m_dblSamplingInterval = pInternalNode->m_pMonitoredItemParam->m_dblSamplingInterval;
				}
				if (dlg.DoModal() == IDOK)
				{
					OpcUa_Byte aTimestampsToReturn;
					if (dlg.m_szTimestampToReturn == CString("Both"))
						aTimestampsToReturn = OpcUa_TimestampsToReturn_Both;
					else
					{
						if (dlg.m_szTimestampToReturn == CString("Server"))
							aTimestampsToReturn = OpcUa_TimestampsToReturn_Server;
						else
						{
							if (dlg.m_szTimestampToReturn == CString("Both"))
								aTimestampsToReturn = OpcUa_TimestampsToReturn_Source;
						}
					}
					OpcUa_Boolean	bDiscardOldest = dlg.m_bDiscardOldest;
					OpcUa_UInt32	uiQueueSize = dlg.m_uiQueueSize;
					OpcUa_Double	dblSamplingInterval = dlg.m_dblSamplingInterval;
					OpcUa_UInt32	DeadbandType=OpcUa_DeadbandType_None;
					if (dlg.m_szDeadbandType==CString("None"))
						DeadbandType = OpcUa_DeadbandType_None;
					else
					{

						if (dlg.m_szDeadbandType == CString("Percent"))
							DeadbandType = DeadbandType == OpcUa_DeadbandType_Percent;
						else
						{
							if (dlg.m_szDeadbandType == CString("Absolute"))
								DeadbandType = OpcUa_DeadbandType_Absolute;
						}
					}
					OpcUa_Double	DeadbandValue = dlg.m_dblDeadbandValue;
					OpcUa_Byte DataChangeTrigger=0;
					if (dlg.m_bTriggerStatusCode)
						DataChangeTrigger = OpcUa_DataChangeTrigger_Status;
					if (dlg.m_bTriggerTimestamp)
						DataChangeTrigger = OpcUa_DataChangeTrigger_StatusValueTimestamp;
					if (dlg.m_bTriggerValue)
						DataChangeTrigger = OpcUa_DataChangeTrigger_StatusValue;
					uStatus = OpenOpcUa_ModifyMonitoredItems(m_hApplication, hSession, hLocalSubscription,
															pInternalNode->m_hMonitoredItem, aTimestampsToReturn,
															bDiscardOldest, uiQueueSize, dblSamplingInterval,
															DeadbandType, DeadbandValue,DataChangeTrigger);
					if (uStatus == OpcUa_Good)
					{
						OpcUa_UInt32 hMonitoredItem = (OpcUa_UInt32)pInternalNode->m_hMonitoredItem;
						OpenOpcUa_InternalNode* pNewInternalNode=OpcUa_Null;
						if (OpenOpcUa_GetInternalNodeByClientHandle(m_hApplication, hSession, hLocalSubscription,
							hMonitoredItem, &pNewInternalNode) == OpcUa_Good)
						{
							OpenOpcUa_ReleaseInternalNode(pInternalNode);
							m_MonitoredElts.SetItemData(index, (DWORD_PTR)pNewInternalNode);
						}
					}
				}
			}
			OpcUa_String_Clear(&szNodeId);
		}
	}
}
