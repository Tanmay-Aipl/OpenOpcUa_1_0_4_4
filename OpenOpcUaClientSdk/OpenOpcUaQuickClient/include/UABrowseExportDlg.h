#pragma once


// Boîte de dialogue CUABrowseExportDlg

class CUABrowseExportDlg : public CDialog
{
	DECLARE_DYNAMIC(CUABrowseExportDlg)

public:
	CUABrowseExportDlg(CWnd* pParent = NULL);   // constructeur standard
	virtual ~CUABrowseExportDlg();

// Données de boîte de dialogue
	enum { IDD = IDD_EXPORT_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // Prise en charge de DDX/DDV

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedSelectFileButton();
	CString m_Filename;
	CCSVFile* m_pCSVFile;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedExportButton();
	HANDLE m_hApplication;
	HANDLE m_hSession;
	OpcUa_NodeId m_FromNodeId; // point de départ de l'export

	afx_msg void OnBnClickedCheckMethodNodeclass();
	afx_msg void OnBnClickedCheckUnspecifiedNodeclass();
	afx_msg void OnBnClickedCheckReferencetypeNodeclass();
	afx_msg void OnBnClickedCheckObjectNodeclass();
	afx_msg void OnBnClickedCheckViewNodeclass();
	afx_msg void OnBnClickedCheckObjecttypeNodeclass();
	afx_msg void OnBnClickedCheckVariableNodeclass();
	afx_msg void OnBnClickedCheckDatatypeNodeclass();
	afx_msg void OnBnClickedCheckVariabletypeNodeclass();
	BOOL			m_bUnspecified;
	BOOL			m_bDataType;
	BOOL			m_bMethod;
	BOOL			m_bView;
	BOOL			m_bVariable;
	BOOL			m_bReferenceType;
	BOOL			m_bObjectType;
	BOOL			m_bVariableType;
	BOOL			m_bObject;
	OpcUa_UInt32	m_NodeClassMask; // contient la classe à browser
	OpcUa_Int32		m_iBrowseDirection; // contient la valeur de l'enum OpcUa_BrowseDirection
	CString m_strExportResult;
};
