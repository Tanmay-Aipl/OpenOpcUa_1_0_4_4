// UABrowserDlg.cpp : fichier d'implémentation
//

#include "stdafx.h"
#include "UAQuickClient.h"
#include "SubEdit.h"
#include "UABrowserDlg.h"
#include "UABrowseOption.h"
#include "CSVFile.h"
#include "UABrowseExportDlg.h"

// Boîte de dialogue CUABrowserDlg
enum _ATTRIBUTES_COLUMN {ATTRIBUTE_NAME_COLUMN, ATTRIBUTE_TYPE_COLUMN,ATTRIBUTE_VALUE_COLUMN};
IMPLEMENT_DYNAMIC(CUABrowserDlg, CDialog)

CUABrowserDlg::CUABrowserDlg(CWnd* pParent /*=NULL*/,HANDLE hSession)
	: CDialog(CUABrowserDlg::IDD, pParent),m_hSession(hSession)
{
	m_hApplication=OpcUa_Null;
	//m_hSession=OpcUa_Null;
	m_iBrowseDirection=OpcUa_BrowseDirection_Forward;
	m_NodeClassMask=OpcUa_NodeClass_Object|OpcUa_NodeClass_Variable | OpcUa_NodeClass_Method;
	m_CallReason = OPCUA_UNKNOWN;
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

CUABrowserDlg::~CUABrowserDlg()
{
}

void CUABrowserDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_NODE_ATTRIBUTES_LIST, m_NodeAttributesCtrl);
	DDX_Control(pDX, IDC_NODEIDS_TREE, m_ReferencesTreeCtrl);
	DDX_Control(pDX, IDC_BUTTON_REFRESH_ATTRIBUTES, m_btRefreshAttributes);
}


BEGIN_MESSAGE_MAP(CUABrowserDlg, CDialog)
	ON_NOTIFY(TVN_SELCHANGED, IDC_NODEIDS_TREE, &CUABrowserDlg::OnTvnSelchangedNodeidsTree)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(IDM_BROWSE_OPTION, &CUABrowserDlg::OnBrowseOption)
	ON_COMMAND(IDM_EXPORT, &CUABrowserDlg::OnExport)
	ON_COMMAND(IDM_MONITOR, &CUABrowserDlg::OnMonitor)
	ON_BN_CLICKED(IDC_BUTTON_REFRESH_ATTRIBUTES, &CUABrowserDlg::OnBnClickedButtonRefreshAttributes)
	ON_NOTIFY(LVN_BEGINLABELEDIT, IDC_NODE_ATTRIBUTES_LIST, &CUABrowserDlg::OnLvnBeginlabeleditNodeAttributesList)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_NODE_ATTRIBUTES_LIST, &CUABrowserDlg::OnLvnEndlabeleditNodeAttributesList)
	ON_NOTIFY(TVN_BEGINDRAG, IDC_NODEIDS_TREE, &CUABrowserDlg::OnTvnBegindragNodeidsTree)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// Gestionnaires de messages de CUABrowserDlg

BOOL CUABrowserDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_ImageList.Create(16,16,ILC_COLOR32,1,1);
	m_ImageList.Add(AfxGetApp()->LoadIcon( IDI_UNSPECIFIED ) ); //0
	m_ImageList.Add(AfxGetApp()->LoadIcon( IDI_OBJECT ) ); //1
	m_ImageList.Add(AfxGetApp()->LoadIcon( IDI_VARIABLE ) ); //2
	m_ImageList.Add(AfxGetApp()->LoadIcon( IDI_METHOD ) ); //3
	m_ImageList.Add(AfxGetApp()->LoadIcon( IDI_OBJECT_TYPE ) ); //4
	m_ImageList.Add(AfxGetApp()->LoadIcon( IDI_VARIABLE_TYPE ) ); //5
	m_ImageList.Add(AfxGetApp()->LoadIcon( IDI_REFERENCE_TYPE ) ); //6
	m_ImageList.Add(AfxGetApp()->LoadIcon( IDI_DATA_TYPE ) ); //7
	m_ImageList.Add(AfxGetApp()->LoadIcon( IDI_VIEW ) ); //8
	m_ImageList.Add(AfxGetApp()->LoadIcon( IDI_DATATYPE_LOGIC ) ); //9
	m_ImageList.Add(AfxGetApp()->LoadIcon( IDI_DATATYPE_NUMERIC ) ); //10
	m_ImageList.Add(AfxGetApp()->LoadIcon( IDI_DATATYPE_STRING ) ); //11
	m_ReferencesTreeCtrl.SetImageList(&m_ImageList,TVSIL_NORMAL);
	//
	HICON  hIcon=AfxGetApp()->LoadIcon(IDI_REFRESH);
	m_btRefreshAttributes.SetIcon(hIcon);
	// ToolTips
	m_Tooltip.Create(this);
	m_Tooltip.Activate(TRUE);
	m_Tooltip.AddTool(GetDlgItem(IDOK),L"Click to close the browser");
	m_Tooltip.AddTool(GetDlgItem(IDCANCEL),L"Click to cancel");
	m_Tooltip.AddTool(GetDlgItem(IDC_BUTTON_REFRESH_ATTRIBUTES),L"Click this button to refresh attributes");
	m_Tooltip.AddTool(GetDlgItem(IDC_NODEIDS_TREE),L"Right client to show the Popup Menu");
	// modification du titre de la boite de dialogue en fonction du traitement ne cours de réalisation
	switch (m_CallReason)
	{
	case OPCUA_BROWSE:
		SetWindowText(L"OpenOpcUa Browser");
		break;
	case OPCUA_ADD_MONITORED_ITEM:
		SetWindowText(L"OpenOpcUa AddMonitoredItems");
		break;
	case OPCUA_READ:
		SetWindowText(L"OpenOpcUa ReadAttributes");
		break;
	case OPCUA_WRITE:
		SetWindowText(L"OpenOpcUa Write Value");
		break;
	default:
		break;
	}
	// Column header
	AddAttributesColumn();
	m_NodeAttributesCtrl.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	OpcUa_NodeId aNodeId;
	aNodeId.Identifier.Numeric=OpcUaId_ObjectsFolder;
	aNodeId.IdentifierType=OpcUa_IdentifierType_Numeric;
	aNodeId.NamespaceIndex= 0;
	HTREEITEM  hItem=m_ReferencesTreeCtrl.InsertItem(CString("Objects"));
	m_ReferencesTreeCtrl.SelectItem(hItem);
	m_ReferencesTreeCtrl.SetItemData(hItem,(DWORD_PTR)&aNodeId);
	Browse(aNodeId);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION : les pages de propriétés OCX devraient retourner FALSE
}
//enum _ATTRIBUTES_COLUMN {ATTRIBUTE_NAME_COLUMN, ATTRIBUTE_TYPE_COLUMN,ATTRIBUTE_VALUE_COLUMN};
void CUABrowserDlg::AddAttributesColumn(void)
{
	m_NodeAttributesCtrl.InsertColumn(ATTRIBUTE_NAME_COLUMN,L"Attribute Name",LVCFMT_CENTER,80,0);
	m_NodeAttributesCtrl.InsertColumn(ATTRIBUTE_TYPE_COLUMN,L"Attribute Type",LVCFMT_CENTER,80,0);
	m_NodeAttributesCtrl.InsertColumn(ATTRIBUTE_VALUE_COLUMN,L"Attribute Value",LVCFMT_LEFT,80,0);
}


OpcUa_StatusCode CUABrowserDlg::Browse(OpcUa_NodeId aFromNodeId)
{
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_Int32 iNoOfReferenceDescription=0;
	OpcUa_ReferenceDescription* pReferenceList=OpcUa_Null; //(OpcUa_ReferenceDescription*)OpcUa_Alloc(sizeof(OpcUa_ReferenceDescription));
	// ZeroMemory(pReferenceList,sizeof(OpcUa_ReferenceDescription));
	// On commence par récupérer le hSession
	if (m_hSession)
	{
		// init Browse Description
		OpcUa_BrowseDescription* pNodesToBrowse=OpcUa_Null;
		OpcUa_Int32 a_nNoOfNodesToBrowse=1;
		pNodesToBrowse=(OpcUa_BrowseDescription*)OpcUa_Alloc((sizeof(OpcUa_BrowseDescription)*a_nNoOfNodesToBrowse));
		OpcUa_BrowseDescription_Initialize(&pNodesToBrowse[0]);
		pNodesToBrowse[0].BrowseDirection=(OpcUa_BrowseDirection)m_iBrowseDirection; 
		pNodesToBrowse[0].IncludeSubtypes=TRUE;
		pNodesToBrowse[0].NodeClassMask=m_NodeClassMask; //
		pNodesToBrowse[0].ResultMask=OpcUa_BrowseResultMask_All;
		// On browse a partir de aFromNodeId
		if (aFromNodeId.IdentifierType==OpcUa_IdentifierType_Numeric)
			pNodesToBrowse[0].NodeId.Identifier.Numeric=aFromNodeId.Identifier.Numeric;// OpcUaId_ObjectsFolder;
		else
		{
			if (aFromNodeId.IdentifierType==OpcUa_IdentifierType_String)
				OpcUa_String_StrnCpy(&(pNodesToBrowse[0].NodeId.Identifier.String),
									 &(aFromNodeId.Identifier.String),
									 OpcUa_String_StrLen(&(aFromNodeId.Identifier.String)) );
		}
		pNodesToBrowse[0].NodeId.IdentifierType=aFromNodeId.IdentifierType;//OpcUa_IdentifierType_Numeric;
		pNodesToBrowse[0].NodeId.NamespaceIndex=aFromNodeId.NamespaceIndex;// 0;

		OpcUa_NodeId_Initialize(&(pNodesToBrowse[0].ReferenceTypeId));
		pNodesToBrowse[0].ReferenceTypeId.IdentifierType=OpcUa_IdentifierType_Numeric;
		pNodesToBrowse[0].ReferenceTypeId.NamespaceIndex=0;
		pNodesToBrowse[0].ReferenceTypeId.Identifier.Numeric=OpcUaId_HierarchicalReferences; //OpcUaId_References;
		uStatus=OpenOpcUa_Browse(m_hApplication,m_hSession,a_nNoOfNodesToBrowse,pNodesToBrowse,&iNoOfReferenceDescription,&pReferenceList);
		if(uStatus==OpcUa_Good)
		{
			CString message;
			message.Format(L"OpenOpcUa_Browse succeed");
			
			// Let's clear the selected node
			// Only if we got answers
			if (iNoOfReferenceDescription)
			{
				HTREEITEM hCurrentItem=m_ReferencesTreeCtrl.GetSelectedItem();
				OpcUa_NodeId* aNewNodeId=(OpcUa_NodeId*)m_ReferencesTreeCtrl.GetItemData(hCurrentItem);
				if (aNewNodeId)
				{
					HTREEITEM hChildItem=m_ReferencesTreeCtrl.GetChildItem(hCurrentItem);
					while (hChildItem)
					{
						m_ReferencesTreeCtrl.DeleteItem(hChildItem);
						hChildItem=m_ReferencesTreeCtrl.GetChildItem(hCurrentItem);
					}
				}
				USES_CONVERSION;
				std::vector<OpcUa_ReferenceDescription> aReferenceList;
				for (OpcUa_Int32 ii=0;ii<iNoOfReferenceDescription;ii++)
				{

					OpcUa_Boolean bFound=OpcUa_False;
					/*
					for (OpcUa_UInt32 iii=0;iii<aReferenceList.size();iii++)
					{
						OpcUa_ReferenceDescription aReferenceDescription=aReferenceList.at(iii);
						if ((OpcUa_String_Compare(&(pReferenceList[ii].BrowseName.Name),&(aReferenceDescription.BrowseName.Name)) == 0) )
						{
							bFound=OpcUa_True;
							break;
						}
					}*/
					if (!bFound)
					{
						aReferenceList.push_back(pReferenceList[ii]);
						short sImage=0;
						OpcUa_NodeId pDataType;
						OpcUa_NodeId_Initialize(&pDataType);
						OpenOpcUa_GetUAVariableDatatype(m_hApplication, m_hSession, pReferenceList[ii].NodeId.NodeId, &pDataType);
						if ((pDataType.IdentifierType == OpcUa_IdentifierType_Numeric) && (pDataType.Identifier.Numeric > 0))
							GetDatatypeIcon(pDataType, &sImage);
						else
							GetNodeClassIcon(pReferenceList[ii].NodeClass, &sImage);
						// let use only the NodeClass
						{
							HTREEITEM hNewItem=m_ReferencesTreeCtrl.InsertItem(A2W(OpcUa_String_GetRawString(&pReferenceList[ii].DisplayName.Text)),sImage,sImage,hCurrentItem);
							OpcUa_NodeId* pNodeId=(OpcUa_NodeId*)OpcUa_Alloc(sizeof(OpcUa_NodeId));
							OpcUa_NodeId_Initialize(pNodeId);
							OpcUa_NodeId_CopyTo(&(pReferenceList[ii].NodeId.NodeId),pNodeId);
							m_ReferencesTreeCtrl.SetItemData(hNewItem,(DWORD_PTR)pNodeId);
						}
					}
				}
				// Free ressources OpcUa_ReferenceDescription
				for (OpcUa_UInt32 iii=0;iii<aReferenceList.size();iii++)
				{
					OpcUa_ReferenceDescription aReferenceDescription=aReferenceList.at(iii);
					OpcUa_ReferenceDescription_Clear( &aReferenceDescription);
				}
				OpcUa_Free(pReferenceList);
				pReferenceList=OpcUa_Null;
			}
		}
		OpcUa_Free(pNodesToBrowse);
	}
	if (pReferenceList)
		OpcUa_Free(pReferenceList) ;
	return uStatus;
}
void CUABrowserDlg::OnTvnSelchangedNodeidsTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	HTREEITEM hCurrentItem=m_ReferencesTreeCtrl.GetSelectedItem();
	OpcUa_NodeId* aFromNodeId=(OpcUa_NodeId*)m_ReferencesTreeCtrl.GetItemData(hCurrentItem);
	if (aFromNodeId)
	{
		OpcUa_StatusCode uStatus = Browse(*aFromNodeId);
		if (uStatus==OpcUa_Good)
		{
			// On efface le contenu courant
			m_NodeAttributesCtrl.DeleteAllItems();
			// On affiche le nouveau
			DisplayAttributes(aFromNodeId);
		}
	}
	*pResult = 0;
}

void CUABrowserDlg::OnContextMenu(CWnd* pWnd, CPoint point)
{
	OpcUa_StatusCode uStatus=OpcUa_Bad;
	CMenu menu;
	if (!pWnd)
		return;
	if (m_ReferencesTreeCtrl.m_hWnd==pWnd->m_hWnd)
	{
		HTREEITEM hItem=m_ReferencesTreeCtrl.GetSelectedItem();
		if (hItem)
		{
			VERIFY(menu.LoadMenu(IDR_BROWSE_MENU));
			CMenu* pPopup = menu.GetSubMenu(0);			
			if (pPopup != NULL)
			{
				if (m_CallReason!=OPCUA_ADD_MONITORED_ITEM)
					pPopup->EnableMenuItem(IDM_MONITOR,MF_DISABLED | MF_GRAYED);
				CWnd* pWndPopupOwner = this;

				while (pWndPopupOwner->GetStyle() & WS_CHILD)
					pWndPopupOwner = pWndPopupOwner->GetParent();

				pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y,
					pWndPopupOwner);
			}
		}
	}
}

void CUABrowserDlg::OnBrowseOption()
{
	CUABrowseOption dlg;
	dlg.m_iBrowseDirection=m_iBrowseDirection;
	dlg.m_NodeClassMask=m_NodeClassMask;
	if (dlg.DoModal()==IDOK)
	{
		m_iBrowseDirection=dlg.m_iBrowseDirection;
		m_NodeClassMask=dlg.m_NodeClassMask;
	}
}

void CUABrowserDlg::OnExport()
{
	HTREEITEM hCurrentItem=m_ReferencesTreeCtrl.GetSelectedItem();
	OpcUa_NodeId* aFromNodeId=(OpcUa_NodeId*)m_ReferencesTreeCtrl.GetItemData(hCurrentItem);
	if (aFromNodeId)
	{
		CUABrowseExportDlg dlg;
		dlg.m_FromNodeId=*aFromNodeId;
		dlg.m_hApplication=m_hApplication;
		dlg.m_hSession=m_hSession;
		dlg.DoModal();
	}
}
HRESULT CUABrowserDlg::GetDatatypeIcon(OpcUa_NodeId aNodeId, short* sImage)
{
	HRESULT hr = S_OK;
	if (aNodeId.IdentifierType == OpcUa_IdentifierType_Numeric)
	{
		switch (aNodeId.Identifier.Numeric)
		{
		case OpcUaType_Boolean:
			*sImage = 9;
			break;
		case OpcUaType_Float:
			*sImage = 10;
			break;
		case OpcUaType_String:
			*sImage = 11;
			break;
		default:
			*sImage=10;
		}
	}
	else
		hr = S_FALSE;
	return hr;
}
HRESULT CUABrowserDlg::GetNodeClassIcon(OpcUa_NodeClass aNodeClass, short* sImage)
{
	HRESULT hr=S_OK;
	switch (aNodeClass)
	{
	case OpcUa_NodeClass_Unspecified:
		*sImage= 0;
		break;
	case OpcUa_NodeClass_Object:
		*sImage =1;
		break;
	case OpcUa_NodeClass_Variable :
		*sImage =2;
		break;
	case OpcUa_NodeClass_Method:
		*sImage =3;
		break;
	case OpcUa_NodeClass_ObjectType:
		*sImage =4;
		break;
	case OpcUa_NodeClass_VariableType:
		*sImage =5;
		break;
	case OpcUa_NodeClass_ReferenceType:
		*sImage =6;
		break;
	case OpcUa_NodeClass_DataType:
		*sImage =7;
		break;
	case OpcUa_NodeClass_View:
		*sImage =8;
		break;
	default :
		*sImage =-1;
		hr=E_FAIL;
		break;
	}
	return hr;
}

HRESULT CUABrowserDlg::DisplayAttributes(OpcUa_NodeId* sourceNodeId)
{
	HRESULT hr=S_OK;
	OpcUa_StatusCode		uStatus;
	OpcUa_DataValue* pResults=OpcUa_Null;
	OpcUa_ReadValueId* pNodesToRead=OpcUa_Null;
	OpcUa_Int32 iNodNodesToRead=0;
	//int iCurrentAttributeId;
	OpcUa_NodeId aNodeId;
	OpcUa_NodeId_Initialize(&aNodeId);
	// Reset the Datatype of the current selected sourceNodeId
	m_iCurrentDataType=0;
	// compute the nuùmber of attribute to read on the server
	iNodNodesToRead=(OpcUa_Attributes_UserExecutable-OpcUa_Attributes_NodeId)+1;
	// Allocate and prepare the attribute to read
	pNodesToRead=(OpcUa_ReadValueId*)OpcUa_Alloc((OpcUa_Attributes_UserExecutable+1)*sizeof(OpcUa_ReadValueId));
	for (OpcUa_Int32 ii = 0;ii<=OpcUa_Attributes_UserExecutable-1;ii++)
	{
		OpcUa_ReadValueId_Initialize(&pNodesToRead[ii]);
		pNodesToRead[ii].AttributeId=ii+1;
		OpcUa_NodeId_Initialize(&(pNodesToRead[ii].NodeId));
		OpcUa_NodeId_CopyTo(sourceNodeId,&(pNodesToRead[ii].NodeId));
	}
	// Call the OpenOpcUaClientLib (make the read)
	uStatus=OpenOpcUa_ReadAttributes(m_hApplication,
		m_hSession,
		OpcUa_TimestampsToReturn_Both,
		iNodNodesToRead,
		pNodesToRead,
		&pResults);
	if (uStatus==OpcUa_Good)
	{
		// Show the attributes in the dialogBox
		for (OpcUa_Int32 iii = 0;iii<=OpcUa_Attributes_UserExecutable-1;iii++)
		{
			OpcUa_String* szAttributeName=OpcUa_Null;
			OpcUa_String* szAttributeDescription=OpcUa_Null;
			OpcUa_String* szBuiltInTypeName=OpcUa_Null;
			OpcUa_String* szBuiltInTypeDescription=OpcUa_Null;
			if (pResults[iii].Value.Datatype!=0)
			{
				// will update the datatype of the current sourceNodeId
				if (pNodesToRead[iii].AttributeId==OpcUa_Attributes_DataType)
				{
					// The datatype must be in a NodeId
					if ( (pResults[iii].Value.Value.NodeId->IdentifierType==OpcUa_IdentifierType_Numeric) && (pResults[iii].Value.Datatype==OpcUaType_NodeId))
						m_iCurrentDataType=pResults[iii].Value.Value.NodeId->Identifier.Numeric;
				}
				uStatus=OpenOpcUa_GetAttributeDetails(pNodesToRead[iii].AttributeId,&szAttributeName,&szAttributeDescription);//pNodesToRead[iii].AttributeId
				if (uStatus==OpcUa_Good)
				{
					//
					int iIndex=m_NodeAttributesCtrl.GetItemCount();
					// insertion du nom de l'attribut
					int iItem=m_NodeAttributesCtrl.InsertItem(iIndex,CString(OpcUa_String_GetRawString(szAttributeName)));
					m_NodeAttributesCtrl.SetColumnWidth(ATTRIBUTE_NAME_COLUMN,LVSCW_AUTOSIZE);
					// Attribut Type
					uStatus=OpenOpcUa_GetBuiltInTypeDetails(pResults[iii].Value.Datatype,&szBuiltInTypeName,&szBuiltInTypeDescription);
					if (uStatus==OpcUa_Good)
					{
						m_NodeAttributesCtrl.SetItem(iItem,
							ATTRIBUTE_TYPE_COLUMN,
							LVIF_TEXT,
							CString(OpcUa_String_GetRawString(szBuiltInTypeDescription)),
							0,0,0,0); 
						m_NodeAttributesCtrl.SetColumnWidth(ATTRIBUTE_TYPE_COLUMN,LVSCW_AUTOSIZE);
					}
					// Value
					if (pNodesToRead[iii].AttributeId != OpcUa_Attributes_NodeClass)
					{
						if (pNodesToRead[iii].AttributeId == OpcUa_Attributes_AccessLevel)
						{
							OpcUa_String* szAccessLevel = OpcUa_Null;
							uStatus = OpenOpcUa_GetAccessLevel(pResults[iii].Value.Value.Byte, &szAccessLevel);
							if (uStatus == OpcUa_Good)
							{
								m_NodeAttributesCtrl.SetItem(iItem, ATTRIBUTE_VALUE_COLUMN, LVIF_TEXT, CString(OpcUa_String_GetRawString(szAccessLevel)), 0, 0, 0, 0);
								OpcUa_String_Clear(szAccessLevel);
								OpcUa_Free(szAccessLevel);
							}
						}
						else
						{
							if (pNodesToRead[iii].AttributeId == OpcUa_Attributes_UserAccessLevel)
							{
								OpcUa_String* szUserAccessLevel = OpcUa_Null;
								uStatus = OpenOpcUa_GetUserAccessLevel(pResults[iii].Value.Value.Byte, &szUserAccessLevel);
								if (uStatus == OpcUa_Good)
								{
									m_NodeAttributesCtrl.SetItem(iItem, ATTRIBUTE_VALUE_COLUMN, LVIF_TEXT, CString(OpcUa_String_GetRawString(szUserAccessLevel)), 0, 0, 0, 0);
									OpcUa_String_Clear(szUserAccessLevel);
									OpcUa_Free(szUserAccessLevel);
								}
							}
							else
							{
								OpcUa_String* strValue = OpcUa_Null;// (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
								//OpcUa_String_Initialize(strValue);
								uStatus = OpenOpcUa_VariantToString(pResults[iii].Value, &strValue);
								m_NodeAttributesCtrl.SetItem(iItem, ATTRIBUTE_VALUE_COLUMN, LVIF_TEXT, CString(OpcUa_String_GetRawString(strValue)), 0, 0, 0, 0);
								OpcUa_String_Clear(strValue);
							}
						}
					}
					else
					{
						OpcUa_String* szNodeClassName = OpcUa_Null;
						uStatus = OpenOpcUa_GetNodeClassDetails(pResults[iii].Value.Value.Int32, &szNodeClassName);
						if (uStatus == OpcUa_Good)
						{
							m_NodeAttributesCtrl.SetItem(iItem, ATTRIBUTE_VALUE_COLUMN, LVIF_TEXT, CString(OpcUa_String_GetRawString(szNodeClassName)), 0, 0, 0, 0);
							OpcUa_String_Clear(szNodeClassName);
							OpcUa_Free(szNodeClassName);
						}
					}
					m_NodeAttributesCtrl.SetColumnWidth(ATTRIBUTE_VALUE_COLUMN,LVSCW_AUTOSIZE);
					// release allocated ressources
					OpcUa_String_Clear(szAttributeName);
					OpcUa_Free(szAttributeName);
					OpcUa_String_Clear(szAttributeDescription);
					OpcUa_Free(szAttributeDescription);
					OpcUa_String_Clear(szBuiltInTypeName);
					OpcUa_Free(szBuiltInTypeName);
					OpcUa_String_Clear(szBuiltInTypeDescription);
					OpcUa_Free(szBuiltInTypeDescription);
				}
			}
			
		}
		OpcUa_Free(pResults);
	}
	for (OpcUa_Int32 ii = 0;ii<=OpcUa_Attributes_UserExecutable-1;ii++)
		OpcUa_ReadValueId_Clear(&pNodesToRead[ii]);
	OpcUa_Free(pNodesToRead);
	return hr;
}

void CUABrowserDlg::OnMonitor()
{
	HTREEITEM hCurrentItem=m_ReferencesTreeCtrl.GetSelectedItem();
	OpcUa_NodeId* aNodeId=(OpcUa_NodeId*)m_ReferencesTreeCtrl.GetItemData(hCurrentItem);
	if (aNodeId)
	{
		m_NodeIds.push_back(aNodeId);
	}
}

void CUABrowserDlg::OnBnClickedButtonRefreshAttributes()
{
	HTREEITEM hCurrentItem=m_ReferencesTreeCtrl.GetSelectedItem();
	OpcUa_NodeId* aFromNodeId=(OpcUa_NodeId*)m_ReferencesTreeCtrl.GetItemData(hCurrentItem);
	if (aFromNodeId)
	{
		m_NodeAttributesCtrl.DeleteAllItems();
		DisplayAttributes(aFromNodeId);
	}
}
BOOL CUABrowserDlg::PreTranslateMessage(MSG* pMsg)
{
	m_Tooltip.RelayEvent(pMsg);

	return CDialog::PreTranslateMessage(pMsg);
}

void CUABrowserDlg::OnLvnBeginlabeleditNodeAttributesList(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	CRect  subrect;
	if (m_CallReason==OPCUA_WRITE)
	{
		m_NodeAttributesCtrl.GetSubItemRect( pDispInfo->item.iItem,
										ATTRIBUTE_VALUE_COLUMN,
										LVIR_BOUNDS , subrect );

		//get edit control and subclass
		HWND hWnd=(HWND)m_NodeAttributesCtrl.SendMessage(LVM_GETEDITCONTROL);
		if (hWnd)
		{
			VERIFY(m_editWnd.SubclassWindow(hWnd));

			//move edit control text 1 pixel to the right of org label,
			//as Windows does it...
			m_editWnd.m_x=subrect.left + 6; 
			m_editWnd.SetWindowText(m_NodeAttributesCtrl.GetItemText(pDispInfo->item.iItem,ATTRIBUTE_VALUE_COLUMN) );
		}
	}
	*pResult = 0;
}

void CUABrowserDlg::OnLvnEndlabeleditNodeAttributesList(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	OpcUa_StatusCode uStatus=OpcUa_Good;
	OpcUa_String* pString=OpcUa_Null;
	OpcUa_Int32 iNoOfNodesToWrite=1;
	OpcUa_WriteValue* pWriteValue=OpcUa_Null;
	OpcUa_StatusCode* pWriteResults=OpcUa_Null;
	USES_CONVERSION;
	if (m_CallReason==OPCUA_WRITE)
	{
		// Will start filling the pWriteValue
		HTREEITEM hItem=m_ReferencesTreeCtrl.GetSelectedItem();
		OpcUa_NodeId* pNodeId=(OpcUa_NodeId*)m_ReferencesTreeCtrl.GetItemData(hItem);
		if (pNodeId)
		{
			pWriteValue=(OpcUa_WriteValue*)OpcUa_Alloc(sizeof(OpcUa_WriteValue));
			OpcUa_WriteValue_Initialize(pWriteValue);
			OpcUa_NodeId_CopyTo(pNodeId,&(pWriteValue->NodeId));
			pWriteValue->AttributeId=OpcUa_Attributes_Value;
			pWriteValue->IndexRange;
			pWriteValue->Value.Value;
			// Read the DataType for the NodeTo Write
			OpcUa_DataValue* pResults = OpcUa_Null;
			OpcUa_ReadValueId* pNodesToRead = OpcUa_Null;
			OpcUa_Int32 iNodNodesToRead = 0;

			pNodesToRead = (OpcUa_ReadValueId*)OpcUa_Alloc(sizeof(OpcUa_ReadValueId));
			OpcUa_ReadValueId_Initialize(pNodesToRead);
			pNodesToRead->AttributeId = OpcUa_Attributes_DataType;
			OpcUa_NodeId_Initialize(&(pNodesToRead->NodeId));
			OpcUa_NodeId_CopyTo(pNodeId,&(pNodesToRead->NodeId));

			uStatus = OpenOpcUa_ReadAttributes(m_hApplication, m_hSession,
				OpcUa_TimestampsToReturn_Both,
				1,
				pNodesToRead, &pResults);

			if (pResults)
			{
				pWriteValue->Value.Value.Datatype = pResults[0].Value.Value.NodeId->Identifier.Numeric;
				OpcUa_DataValue_Clear(pResults);
				OpcUa_Free(pResults);
			}
			OpcUa_NodeId_Clear(&(pNodesToRead->NodeId));
			OpcUa_ReadValueId_Clear(pNodesToRead);
			OpcUa_Free(pNodesToRead);
			// 
			pString=(OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
			OpcUa_String_Initialize(pString);
			OpcUa_String_AttachCopy(pString,(OpcUa_CharA*)(W2A(pDispInfo->item.pszText)));
			// will tranform the string enter by the user to a OpcUa_Variant
			OpcUa_Variant* pVariant=OpcUa_Null;
			uStatus=OpenOpcUa_StringToVariant(pString,m_iCurrentDataType,&pVariant);
			if (uStatus==OpcUa_Good)
			{
				OpcUa_Variant_CopyTo(pVariant,&(pWriteValue->Value.Value));
				// will now write the OpcUa_Variant to the OpcUa Server
				uStatus=OpenOpcUa_WriteAttributes(m_hApplication,m_hSession,iNoOfNodesToWrite,pWriteValue,&pWriteResults);
				if (uStatus==OpcUa_Good)
				{
					m_NodeAttributesCtrl.DeleteAllItems();
					// Now will refresh the Attributes
					DisplayAttributes(pNodeId);
				}
				OpcUa_Variant_Clear(pVariant);
			}
		}
	}
	*pResult = 0;
}

void CUABrowserDlg::OnTvnBegindragNodeidsTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// Create the drag&drop source and data objects
	TVITEM tvNewItem = pNMTreeView->itemNew;
	OpcUa_NodeId* pFromNodeId = (OpcUa_NodeId*)m_ReferencesTreeCtrl.GetItemData(tvNewItem.hItem);
	if (pFromNodeId)
	{
		COleDropSource *pDropSource = new COleDropSource;
		COleDataSource *pDataSource = new COleDataSource;
		CString Data;

		{
			CSharedFile file(GMEM_ZEROINIT | GMEM_DDESHARE | GMEM_MOVEABLE);
			TRY
			{
				CArchive ar(&file, CArchive::store);
				TRY
				{
					TRY
					{
						OpcUa_String szNodeId;
						OpcUa_String_Initialize(&szNodeId);
						if (OpenOpcUa_NodeIdToString(*pFromNodeId, &szNodeId)==OpcUa_Good)
							Data=CString(OpcUa_String_GetRawString(&szNodeId));
						ar << Data;
						ar.Close();
						OpcUa_String_Clear(&szNodeId);
					}
					CATCH_ALL(eInner)
					{
						// exception while writing into or closing the archive
						ASSERT(FALSE);
					}
					END_CATCH_ALL;
				}
					CATCH_ALL(eMiddle)
				{
					// exception in the destructor of ar
					ASSERT(FALSE);
				}
				END_CATCH_ALL;

				// put the file object into the data object
				pDataSource->CacheGlobalData(m_DragDropFormat, file.Detach());
				pDataSource->DoDragDrop(DROPEFFECT_MOVE | DROPEFFECT_COPY,
					NULL, pDropSource);
				pDataSource->InternalRelease();
			}
				CATCH_ALL(eOuter)
			{
				// exception while destructing the file
				ASSERT(FALSE);
			}
			END_CATCH_ALL;

			delete pDropSource;
			//delete pDataSource;
		}
	}
	*pResult = 0;
}

void CUABrowserDlg::OnClose()
{
	// TODO: ajoutez ici le code de votre gestionnaire de messages et/ou les paramètres par défaut des appels

	CDialog::OnClose();
}

void CUABrowserDlg::SetSession(OpcUa_Handle hSession)
{
	m_ReferencesTreeCtrl.DeleteAllItems();
	m_NodeAttributesCtrl.DeleteAllItems();
	m_hSession = hSession;
	OpcUa_NodeId aNodeId;
	aNodeId.Identifier.Numeric = OpcUaId_ObjectsFolder;
	aNodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
	aNodeId.NamespaceIndex = 0;
	HTREEITEM  hItem = m_ReferencesTreeCtrl.InsertItem(CString("Objects"));
	m_ReferencesTreeCtrl.SelectItem(hItem);
	m_ReferencesTreeCtrl.SetItemData(hItem, (DWORD_PTR)&aNodeId);
	Browse(aNodeId);
}