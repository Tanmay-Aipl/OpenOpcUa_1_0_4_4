#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// Boîte de dialogue CUAWriteDlg

class CUAWriteDlg : public CDialog
{
	DECLARE_DYNAMIC(CUAWriteDlg)

public:
	CUAWriteDlg(CWnd* pParent = NULL,OpcUa_NodeId* pNodeId=OpcUa_Null,HANDLE hApplication=OpcUa_Null, HANDLE hSession=OpcUa_Null);   // constructeur standard
	virtual ~CUAWriteDlg();

// Données de boîte de dialogue
	enum { IDD = IDD_WRITE_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // Prise en charge de DDX/DDV

	DECLARE_MESSAGE_MAP()
public:
	void InitComboAttributes(void);
	CComboBox m_ComboAttributeId;
	virtual BOOL OnInitDialog();
	void InitDataType(void);
	OpcUa_NodeId GetDataType() {return m_DataTypeNodeId;}
	OpcUa_StatusCode GetValueToWrite(OpcUa_WriteValue** ppWriteValue);
	CComboBox m_ComboDatatype;
private:
	//OpcUa_NodeId m_NodeId;
	OpcUa_NodeId m_DataTypeNodeId; // DataType of the Node To Write. This DataType is Initialize in the OnInitDialog method.
public:
	// NodeId en cours de traitement danc ce control
	CString m_strNodeId;
	HANDLE m_hApplication;
	HANDLE m_hSession;
	// This a control use to select values when the Node is an array
	CListCtrl m_NodeArrayValue;
	// Used when the Node contains a string
	CString m_NodeValueAsString;
	// Used when the Node contains a numerical value. We always go through a DOUBLE
	double m_NodeValueNumeric;
	CEdit m_NodeValueAsStringCtrl;
	CEdit m_NodeValueNumericCtrl;
	OpcUa_WriteValue* m_pValueToWrite;

	int m_iAttributeId;
	afx_msg void OnCbnSelchangeComboAttributeid();
	int m_iDatatype;
	afx_msg void OnCbnSelchangeComboDatatype();
};
