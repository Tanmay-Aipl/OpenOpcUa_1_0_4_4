// UABrowseOption.cpp : fichier d'implémentation
//

#include "stdafx.h"
#include "UAQuickClient.h"
#include "UABrowseOption.h"


// Boîte de dialogue CUABrowseOption

IMPLEMENT_DYNAMIC(CUABrowseOption, CDialog)

CUABrowseOption::CUABrowseOption(CWnd* pParent /*=NULL*/)
	: CDialog(CUABrowseOption::IDD, pParent)
{
	m_iBrowseDirection=OpcUa_BrowseDirection_Forward;
}

CUABrowseOption::~CUABrowseOption()
{
}

void CUABrowseOption::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_UNSPECIFIED_NODECLASS, m_bUnspecified);
	DDX_Check(pDX, IDC_CHECK_DATATYPE_NODECLASS, m_bDataType);
	DDX_Check(pDX, IDC_CHECK_METHOD_NODECLASS, m_bMethod);
	DDX_Check(pDX, IDC_CHECK_VIEW_NODECLASS, m_bView);
	DDX_Check(pDX, IDC_CHECK_VARIABLE_NODECLASS, m_bVariable);
	DDX_Check(pDX, IDC_CHECK_REFERENCETYPE_NODECLASS, m_bReferenceType);
	DDX_Check(pDX, IDC_CHECK_OBJECTTYPE_NODECLASS, m_bObjectType);
	DDX_Check(pDX, IDC_CHECK_VARIABLETYPE_NODECLASS, m_bVariableType);
	DDX_Check(pDX, IDC_CHECK_OBJECT_NODECLASS, m_bObject);
	DDX_Control(pDX, IDC_COMBO_BROWSE_DIRECTION, m_BrowseDirectionCombo);
}


BEGIN_MESSAGE_MAP(CUABrowseOption, CDialog)
	ON_BN_CLICKED(IDC_CHECK_METHOD_NODECLASS, &CUABrowseOption::OnBnClickedCheckMethodNodeclass)
	ON_BN_CLICKED(IDC_CHECK_UNSPECIFIED_NODECLASS, &CUABrowseOption::OnBnClickedCheckUnspecifiedNodeclass)
	ON_BN_CLICKED(IDC_CHECK_REFERENCETYPE_NODECLASS, &CUABrowseOption::OnBnClickedCheckReferencetypeNodeclass)
	ON_BN_CLICKED(IDC_CHECK_OBJECT_NODECLASS, &CUABrowseOption::OnBnClickedCheckObjectNodeclass)
	ON_BN_CLICKED(IDC_CHECK_VIEW_NODECLASS, &CUABrowseOption::OnBnClickedCheckViewNodeclass)
	ON_BN_CLICKED(IDC_CHECK_OBJECTTYPE_NODECLASS, &CUABrowseOption::OnBnClickedCheckObjecttypeNodeclass)
	ON_BN_CLICKED(IDC_CHECK_VARIABLE_NODECLASS, &CUABrowseOption::OnBnClickedCheckVariableNodeclass)
	ON_BN_CLICKED(IDC_CHECK_DATATYPE_NODECLASS, &CUABrowseOption::OnBnClickedCheckDatatypeNodeclass)
	ON_BN_CLICKED(IDC_CHECK_VARIABLETYPE_NODECLASS, &CUABrowseOption::OnBnClickedCheckVariabletypeNodeclass)
	ON_CBN_SELCHANGE(IDC_COMBO_BROWSE_DIRECTION, &CUABrowseOption::OnCbnSelchangeComboBrowseDirection)
END_MESSAGE_MAP()

void CUABrowseOption::OnBnClickedCheckMethodNodeclass()
{
	UpdateData(TRUE);
	if ((m_NodeClassMask&OpcUa_NodeClass_Method)==OpcUa_NodeClass_Method)
		m_NodeClassMask^=OpcUa_NodeClass_Method;
	else
		m_NodeClassMask|= OpcUa_NodeClass_Method;
}

void CUABrowseOption::OnBnClickedCheckUnspecifiedNodeclass()
{
	UpdateData(TRUE);
	if ((m_NodeClassMask&OpcUa_NodeClass_Unspecified)==OpcUa_NodeClass_Unspecified)
		m_NodeClassMask^=OpcUa_NodeClass_Unspecified;
	else
		m_NodeClassMask|= OpcUa_NodeClass_Unspecified;
}

void CUABrowseOption::OnBnClickedCheckReferencetypeNodeclass()
{
	UpdateData(TRUE);
	if ((m_NodeClassMask&OpcUa_NodeClass_ReferenceType)==OpcUa_NodeClass_ReferenceType)
		m_NodeClassMask^=OpcUa_NodeClass_ReferenceType;
	else
		m_NodeClassMask|= OpcUa_NodeClass_ReferenceType;
}

void CUABrowseOption::OnBnClickedCheckObjectNodeclass()
{
	UpdateData(TRUE);
	if ((m_NodeClassMask&OpcUa_NodeClass_Method)==OpcUa_NodeClass_Method)
		m_NodeClassMask^=OpcUa_NodeClass_Method;
	else
		m_NodeClassMask|= OpcUa_NodeClass_Object;
}

void CUABrowseOption::OnBnClickedCheckViewNodeclass()
{
	UpdateData(TRUE);
	if ((m_NodeClassMask&OpcUa_NodeClass_View)==OpcUa_NodeClass_View)
		m_NodeClassMask^=OpcUa_NodeClass_View;
	else
		m_NodeClassMask|= OpcUa_NodeClass_View;
}

void CUABrowseOption::OnBnClickedCheckObjecttypeNodeclass()
{
	UpdateData(TRUE);
	if ((m_NodeClassMask&OpcUa_NodeClass_ObjectType)==OpcUa_NodeClass_ObjectType)
		m_NodeClassMask^=OpcUa_NodeClass_ObjectType;
	else
		m_NodeClassMask|= OpcUa_NodeClass_ObjectType;
}

void CUABrowseOption::OnBnClickedCheckVariableNodeclass()
{
	UpdateData(TRUE);
	if ((m_NodeClassMask&OpcUa_NodeClass_VariableType)==OpcUa_NodeClass_VariableType)
		m_NodeClassMask^=OpcUa_NodeClass_VariableType;
	else
		m_NodeClassMask|= OpcUa_NodeClass_VariableType;
}

void CUABrowseOption::OnBnClickedCheckDatatypeNodeclass()
{
	UpdateData(TRUE);
	if ((m_NodeClassMask&OpcUa_NodeClass_DataType)==OpcUa_NodeClass_DataType)
		m_NodeClassMask^=OpcUa_NodeClass_DataType;
	else
		m_NodeClassMask|= OpcUa_NodeClass_DataType;
}

void CUABrowseOption::OnBnClickedCheckVariabletypeNodeclass()
{
	UpdateData(TRUE);
	if ((m_NodeClassMask&OpcUa_NodeClass_VariableType)==OpcUa_NodeClass_VariableType)
		m_NodeClassMask^=OpcUa_NodeClass_VariableType;
	else
		m_NodeClassMask|= OpcUa_NodeClass_VariableType;
}
// Gestionnaires de messages de CUABrowseOption

void CUABrowseOption::OnCbnSelchangeComboBrowseDirection()
{
	CString strDirection;
	UpdateData(TRUE);
	int iIndex=m_BrowseDirectionCombo.GetCurSel();
	if( iIndex!=CB_ERR)
	{
		int n=m_BrowseDirectionCombo.GetLBTextLen(iIndex);
		m_BrowseDirectionCombo.GetLBText(iIndex,strDirection.GetBuffer(n));
		if (strDirection.Compare(L"Both")==0)
			m_iBrowseDirection=OpcUa_BrowseDirection_Both;
		if (strDirection.Compare(L"Forward")==0)
			m_iBrowseDirection=OpcUa_BrowseDirection_Forward;
		if (strDirection.Compare(L"Inverse")==0)
			m_iBrowseDirection=OpcUa_BrowseDirection_Inverse;
	}
}

BOOL CUABrowseOption::OnInitDialog()
{
	CDialog::OnInitDialog();

	// restitution des option precedement selectionnée
	m_BrowseDirectionCombo.SetCurSel(m_iBrowseDirection);
	//if (m_iBrowseDirection==OpcUa_BrowseDirection_Both)
	//	m_BrowseDirectionCombo.SetCurSel(m_iBrowseDirection+1);
	//if (m_iBrowseDirection==OpcUa_BrowseDirection_Forward)
	//	;
	//if (m_iBrowseDirection==OpcUa_BrowseDirection_Inverse)
	//	;
	if ((m_NodeClassMask&OpcUa_NodeClass_DataType)==OpcUa_NodeClass_DataType)
		m_bDataType=OpcUa_True;
	if ((m_NodeClassMask&OpcUa_NodeClass_Method)==OpcUa_NodeClass_Method)
		m_bMethod=OpcUa_True;
	if ((m_NodeClassMask&OpcUa_NodeClass_Object)==OpcUa_NodeClass_Object)
		m_bObject=OpcUa_True;
	if ((m_NodeClassMask&OpcUa_NodeClass_ObjectType)==OpcUa_NodeClass_ObjectType)
		m_bObjectType=OpcUa_True;
	if ((m_NodeClassMask&OpcUa_NodeClass_ReferenceType)==OpcUa_NodeClass_ReferenceType)
		m_bReferenceType=OpcUa_True;
	if ((m_NodeClassMask&OpcUa_NodeClass_Unspecified)==OpcUa_NodeClass_Unspecified)
		m_bUnspecified=OpcUa_True;
	if ((m_NodeClassMask&OpcUa_NodeClass_Variable)==OpcUa_NodeClass_Variable)
		m_bVariable=OpcUa_True;
	if ((m_NodeClassMask&OpcUa_NodeClass_VariableType)==OpcUa_NodeClass_VariableType)
		m_bVariableType=OpcUa_True;
	if ((m_NodeClassMask&OpcUa_NodeClass_View)==OpcUa_NodeClass_View)
		m_bView=OpcUa_True;
	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION : les pages de propriétés OCX devraient retourner FALSE
}
