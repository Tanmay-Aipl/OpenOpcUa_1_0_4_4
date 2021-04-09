// NodeAttributesDlg.cpp : fichier d'implémentation
//

#include "stdafx.h"
#include "UAQuickClient.h"
#include "NodeAttributesDlg.h"


// Boîte de dialogue CNodeAttributesDlg

IMPLEMENT_DYNAMIC(CNodeAttributesDlg, CDialog)

CNodeAttributesDlg::CNodeAttributesDlg(CWnd* pParent /*=NULL*/,HANDLE hSession)
	: CDialog(CNodeAttributesDlg::IDD, pParent),m_hSession(hSession)
{

}

CNodeAttributesDlg::~CNodeAttributesDlg()
{
}

void CNodeAttributesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_NODE_ATTRIBUTES_LIST, m_NodeAttributesCtrl);
}
void CNodeAttributesDlg::DeleteAllAttributes()
{
	m_NodeAttributesCtrl.DeleteAllItems();
}

BEGIN_MESSAGE_MAP(CNodeAttributesDlg, CDialog)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// Gestionnaires de messages de CNodeAttributesDlg

BOOL CNodeAttributesDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	AddAttributesColumn();
	m_NodeAttributesCtrl.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	DisplayAttributes(&m_NodeId);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION : les pages de propriétés OCX devraient retourner FALSE
}
enum _ATTRIBUTES_COLUMN {ATTRIBUTE_NAME_COLUMN, ATTRIBUTE_TYPE_COLUMN,ATTRIBUTE_VALUE_COLUMN};
void CNodeAttributesDlg::AddAttributesColumn(void)
{
	m_NodeAttributesCtrl.InsertColumn(ATTRIBUTE_NAME_COLUMN,L"Attribute Name",LVCFMT_CENTER,80,0);
	m_NodeAttributesCtrl.InsertColumn(ATTRIBUTE_TYPE_COLUMN,L"Attribute Type",LVCFMT_CENTER,80,0);
	m_NodeAttributesCtrl.InsertColumn(ATTRIBUTE_VALUE_COLUMN,L"Attribute Value",LVCFMT_CENTER,80,0);
}

// Gestionnaires de messages de CMonitoredItemDetailDlg

HRESULT CNodeAttributesDlg::DisplayAttributes(OpcUa_NodeId* sourceNodeId)
{
	HRESULT hr=S_OK;
	OpcUa_StatusCode		uStatus;
	OpcUa_DataValue* pResults=OpcUa_Null;
	OpcUa_ReadValueId* pNodesToRead=OpcUa_Null;
	OpcUa_Int32 iNodNodesToRead=0;
	//int iCurrentAttributeId;
	OpcUa_NodeId aNodeId;
	OpcUa_NodeId_Initialize(&aNodeId);
	// we clear all first
	m_NodeAttributesCtrl.DeleteAllItems();
	// compute the nuùmber of attribute to read on the server
	iNodNodesToRead=(OpcUa_Attributes_UserExecutable-OpcUa_Attributes_NodeId)+1;
	// Allocate and prepare the attribute to read
	pNodesToRead=(OpcUa_ReadValueId*)OpcUa_Alloc((OpcUa_Attributes_UserExecutable+1)*sizeof(OpcUa_ReadValueId));
	for (OpcUa_Int32 ii = 0;ii<=OpcUa_Attributes_UserExecutable-1;ii++)
	{
		OpcUa_ReadValueId_Initialize(&pNodesToRead[ii]);
		pNodesToRead[ii].AttributeId=ii+1;
		pNodesToRead[ii].NodeId=*sourceNodeId;
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
					if (pNodesToRead[iii].AttributeId!=OpcUa_Attributes_NodeClass)
					{
						if (pNodesToRead[iii].AttributeId==OpcUa_Attributes_AccessLevel)
						{
							OpcUa_String* szAccessLevel=OpcUa_Null; 
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
						OpcUa_String* szNodeClassName=OpcUa_Null;
						uStatus=OpenOpcUa_GetNodeClassDetails(pResults[iii].Value.Value.Int32,&szNodeClassName);
						if (uStatus==OpcUa_Good)
						{
							m_NodeAttributesCtrl.SetItem(iItem,ATTRIBUTE_VALUE_COLUMN,LVIF_TEXT,CString(OpcUa_String_GetRawString(szNodeClassName)),0,0,0,0); 
							OpcUa_String_Clear(szNodeClassName);
							OpcUa_Free(szNodeClassName);
						}
					}
					m_NodeAttributesCtrl.SetColumnWidth(ATTRIBUTE_VALUE_COLUMN,LVSCW_AUTOSIZE);
					// release allocated ressources
					OpcUa_String_Clear(szAttributeName);
					OpcUa_String_Clear(szAttributeDescription);
					OpcUa_String_Clear(szBuiltInTypeName);
					OpcUa_String_Clear(szBuiltInTypeDescription);
				}
			}
		}
	}
	return hr;
}
void CNodeAttributesDlg::OnClose()
{
	// TODO: ajoutez ici le code de votre gestionnaire de messages et/ou les paramètres par défaut des appels

	CDialog::OnClose();
}