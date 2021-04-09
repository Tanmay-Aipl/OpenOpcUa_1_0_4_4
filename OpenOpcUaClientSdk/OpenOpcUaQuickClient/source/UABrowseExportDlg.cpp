// UABrowseExportDlg.cpp : fichier d'implémentation
//

#include "stdafx.h"
#include "UAQuickClient.h"
#include "CSVFile.h"
#include "UABrowseExportDlg.h"

// Boîte de dialogue CUABrowseExportDlg

IMPLEMENT_DYNAMIC(CUABrowseExportDlg, CDialog)

CUABrowseExportDlg::CUABrowseExportDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CUABrowseExportDlg::IDD, pParent)
	, m_Filename(_T(""))
	, m_strExportResult(_T(""))
{
	m_hApplication=OpcUa_Null;
	m_hSession=OpcUa_Null;
	m_pCSVFile =new CCSVFile();
	m_iBrowseDirection=OpcUa_BrowseDirection_Forward;
	m_NodeClassMask=OpcUa_NodeClass_Object|OpcUa_NodeClass_Variable;
}

CUABrowseExportDlg::~CUABrowseExportDlg()
{
	if (m_pCSVFile)
		delete m_pCSVFile;
}

void CUABrowseExportDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_FILENAME, m_Filename);
	DDX_Check(pDX, IDC_CHECK_UNSPECIFIED_NODECLASS, m_bUnspecified);
	DDX_Check(pDX, IDC_CHECK_DATATYPE_NODECLASS, m_bDataType);
	DDX_Check(pDX, IDC_CHECK_METHOD_NODECLASS, m_bMethod);
	DDX_Check(pDX, IDC_CHECK_VIEW_NODECLASS, m_bView);
	DDX_Check(pDX, IDC_CHECK_VARIABLE_NODECLASS, m_bVariable);
	DDX_Check(pDX, IDC_CHECK_REFERENCETYPE_NODECLASS, m_bReferenceType);
	DDX_Check(pDX, IDC_CHECK_OBJECTTYPE_NODECLASS, m_bObjectType);
	DDX_Check(pDX, IDC_CHECK_VARIABLETYPE_NODECLASS, m_bVariableType);
	DDX_Check(pDX, IDC_CHECK_OBJECT_NODECLASS, m_bObject);
	DDX_Text(pDX, IDC_EXPORT_RESULT, m_strExportResult);
}


BEGIN_MESSAGE_MAP(CUABrowseExportDlg, CDialog)
	ON_BN_CLICKED(IDC_SELECT_FILE_BUTTON, &CUABrowseExportDlg::OnBnClickedSelectFileButton)
	ON_BN_CLICKED(IDC_EXPORT_BUTTON, &CUABrowseExportDlg::OnBnClickedExportButton)
	ON_BN_CLICKED(IDC_CHECK_METHOD_NODECLASS, &CUABrowseExportDlg::OnBnClickedCheckMethodNodeclass)
	ON_BN_CLICKED(IDC_CHECK_UNSPECIFIED_NODECLASS, &CUABrowseExportDlg::OnBnClickedCheckUnspecifiedNodeclass)
	ON_BN_CLICKED(IDC_CHECK_REFERENCETYPE_NODECLASS, &CUABrowseExportDlg::OnBnClickedCheckReferencetypeNodeclass)
	ON_BN_CLICKED(IDC_CHECK_OBJECT_NODECLASS, &CUABrowseExportDlg::OnBnClickedCheckObjectNodeclass)
	ON_BN_CLICKED(IDC_CHECK_VIEW_NODECLASS, &CUABrowseExportDlg::OnBnClickedCheckViewNodeclass)
	ON_BN_CLICKED(IDC_CHECK_OBJECTTYPE_NODECLASS, &CUABrowseExportDlg::OnBnClickedCheckObjecttypeNodeclass)
	ON_BN_CLICKED(IDC_CHECK_VARIABLE_NODECLASS, &CUABrowseExportDlg::OnBnClickedCheckVariableNodeclass)
	ON_BN_CLICKED(IDC_CHECK_DATATYPE_NODECLASS, &CUABrowseExportDlg::OnBnClickedCheckDatatypeNodeclass)
	ON_BN_CLICKED(IDC_CHECK_VARIABLETYPE_NODECLASS, &CUABrowseExportDlg::OnBnClickedCheckVariabletypeNodeclass)
END_MESSAGE_MAP()


// Gestionnaires de messages de CUABrowseExportDlg

void CUABrowseExportDlg::OnBnClickedSelectFileButton()
{
	if (m_pCSVFile)
	{
		m_pCSVFile->SetFileName();
		m_Filename=m_pCSVFile->GetFileName();
		UpdateData(FALSE);
	}
}

BOOL CUABrowseExportDlg::OnInitDialog()
{
	CDialog::OnInitDialog();


	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION : les pages de propriétés OCX devraient retourner FALSE
}

void CUABrowseExportDlg::OnBnClickedExportButton()
{
	m_pCSVFile->SethApplication(m_hApplication);
	m_pCSVFile->SethSession(m_hSession);
	m_pCSVFile->m_NodeClassMask=m_NodeClassMask;
	m_pCSVFile->m_iBrowseDirection=m_iBrowseDirection;
	// On commence par ouvrir le fichier
	CFileException e;
	CString strCSVFileName=m_pCSVFile->GetFileName();
	if(strCSVFileName.IsEmpty())
	{
		AfxMessageBox(L"Select a filename for export");
		return;
	}
	else
	{
		if (m_pCSVFile->Open(strCSVFileName,CFile::modeCreate | CFile::modeWrite,&e))
		{
			m_pCSVFile->ExportCSVFile(m_FromNodeId);
			m_pCSVFile->Close();
			
			m_strExportResult.Format(L"%u nodes was exported in %s",m_pCSVFile->m_iExportedCounter,strCSVFileName.GetBuffer());
			UpdateData(FALSE);
		}
		else
		{
			CString Message;
			Message.Format(L"Export failed>Cannot open the requested file: %s - %s",strCSVFileName,e.m_cause);
			//InternalAddLoggerMessage(Message,2);
			Message.ReleaseBuffer();Message.Empty();
		}
	}
}
void CUABrowseExportDlg::OnBnClickedCheckMethodNodeclass()
{
	UpdateData(TRUE);
	if ((m_NodeClassMask&OpcUa_NodeClass_Method)==OpcUa_NodeClass_Method)
		m_NodeClassMask^=OpcUa_NodeClass_Method;
	else
		m_NodeClassMask|= OpcUa_NodeClass_Method;
}

void CUABrowseExportDlg::OnBnClickedCheckUnspecifiedNodeclass()
{
	UpdateData(TRUE);
	if ((m_NodeClassMask&OpcUa_NodeClass_Unspecified)==OpcUa_NodeClass_Unspecified)
		m_NodeClassMask^=OpcUa_NodeClass_Unspecified;
	else
		m_NodeClassMask|= OpcUa_NodeClass_Unspecified;
}

void CUABrowseExportDlg::OnBnClickedCheckReferencetypeNodeclass()
{
	UpdateData(TRUE);
	if ((m_NodeClassMask&OpcUa_NodeClass_ReferenceType)==OpcUa_NodeClass_ReferenceType)
		m_NodeClassMask^=OpcUa_NodeClass_ReferenceType;
	else
		m_NodeClassMask|= OpcUa_NodeClass_ReferenceType;
}

void CUABrowseExportDlg::OnBnClickedCheckObjectNodeclass()
{
	UpdateData(TRUE);
	if ((m_NodeClassMask&OpcUa_NodeClass_Method)==OpcUa_NodeClass_Method)
		m_NodeClassMask^=OpcUa_NodeClass_Method;
	else
		m_NodeClassMask|= OpcUa_NodeClass_Object;
}

void CUABrowseExportDlg::OnBnClickedCheckViewNodeclass()
{
	UpdateData(TRUE);
	if ((m_NodeClassMask&OpcUa_NodeClass_View)==OpcUa_NodeClass_View)
		m_NodeClassMask^=OpcUa_NodeClass_View;
	else
		m_NodeClassMask|= OpcUa_NodeClass_View;
}

void CUABrowseExportDlg::OnBnClickedCheckObjecttypeNodeclass()
{
	UpdateData(TRUE);
	if ((m_NodeClassMask&OpcUa_NodeClass_ObjectType)==OpcUa_NodeClass_ObjectType)
		m_NodeClassMask^=OpcUa_NodeClass_ObjectType;
	else
		m_NodeClassMask|= OpcUa_NodeClass_ObjectType;
}

void CUABrowseExportDlg::OnBnClickedCheckVariableNodeclass()
{
	UpdateData(TRUE);
	if ((m_NodeClassMask&OpcUa_NodeClass_VariableType)==OpcUa_NodeClass_VariableType)
		m_NodeClassMask^=OpcUa_NodeClass_VariableType;
	else
		m_NodeClassMask|= OpcUa_NodeClass_VariableType;
}

void CUABrowseExportDlg::OnBnClickedCheckDatatypeNodeclass()
{
	UpdateData(TRUE);
	if ((m_NodeClassMask&OpcUa_NodeClass_DataType)==OpcUa_NodeClass_DataType)
		m_NodeClassMask^=OpcUa_NodeClass_DataType;
	else
		m_NodeClassMask|= OpcUa_NodeClass_DataType;
}

void CUABrowseExportDlg::OnBnClickedCheckVariabletypeNodeclass()
{
	UpdateData(TRUE);
	if ((m_NodeClassMask&OpcUa_NodeClass_VariableType)==OpcUa_NodeClass_VariableType)
		m_NodeClassMask^=OpcUa_NodeClass_VariableType;
	else
		m_NodeClassMask|= OpcUa_NodeClass_VariableType;
}
// Gestionnaires de messages de CUABrowseOption

