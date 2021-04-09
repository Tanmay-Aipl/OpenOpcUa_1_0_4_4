#pragma once
#include "afxwin.h"


// Boîte de dialogue CUABrowseOption

class CUABrowseOption : public CDialog
{
	DECLARE_DYNAMIC(CUABrowseOption)

public:
	CUABrowseOption(CWnd* pParent = NULL);   // constructeur standard
	virtual ~CUABrowseOption();

// Données de boîte de dialogue
	enum { IDD = IDD_BROWSE_OPTION_DIALOG };
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
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // Prise en charge de DDX/DDV

	DECLARE_MESSAGE_MAP()
public:
	OpcUa_UInt32	m_NodeClassMask; // contient la classe à browser
	OpcUa_Int32		m_iBrowseDirection; // contient la valeur de l'enum OpcUa_BrowseDirection
public:
	afx_msg void OnCbnSelchangeComboBrowseDirection();
	CComboBox m_BrowseDirectionCombo;
	virtual BOOL OnInitDialog();
};
