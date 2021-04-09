// CSVFile.h: interface for the CCSVFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CSVFILE_H__2028AC34_01D1_48CB_80E8_08ED383F235F__INCLUDED_)
#define AFX_CSVFILE_H__2028AC34_01D1_48CB_80E8_08ED383F235F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CLineReferenceDescription
{
public:
	CLineReferenceDescription();
	CLineReferenceDescription(OpcUa_ReferenceDescription* pReferenceDescription,HANDLE  hApplication,HANDLE hSession);
	~CLineReferenceDescription();
	void SethApplication(HANDLE hApp) {m_hApplication=hApp;}
	void SethSession(HANDLE hSession) {m_hSession=hSession;}
	HRESULT UpdateDataType(OpcUa_NodeId aNodeId);
private:
public:
	OpcUa_NodeId         m_ReferenceTypeId;
	OpcUa_Boolean        m_IsForward;
	OpcUa_ExpandedNodeId m_NodeId;
	OpcUa_QualifiedName  m_BrowseName;
	OpcUa_LocalizedText  m_DisplayName;
	OpcUa_NodeClass      m_NodeClass;
	OpcUa_ExpandedNodeId m_TypeDefinition;
	OpcUa_UInt32		 m_DataType;
	HANDLE m_hApplication;
	HANDLE m_hSession;
	// Conversion du type de donnée en type variant
	OpcUa_String* FromDataType(void);
};
class CCSVFile : public CFile  
{
public:
	HRESULT ExportCSVFile(OpcUa_NodeId aRootNode);
	HRESULT SetFileName();
	CString GetFileName();
	CCSVFile();
	CCSVFile(LPCTSTR lpszFileName);
	virtual ~CCSVFile();
	CLineReferenceDescription* m_pCurrentTagLine;
	CWnd* m_hParent; // handle sur le parent
	void SethApplication(HANDLE hApp) {m_hApplication=hApp;}
	void SethSession(HANDLE hSession) {m_hSession=hSession;}
	OpcUa_UInt32	m_NodeClassMask; // contient la classe à browser
	OpcUa_Int32		m_iBrowseDirection; // contient la valeur de l'enum OpcUa_BrowseDirection
	OpcUa_Int32		m_iExportedCounter;
private:
	CString m_strCSVFileName;
	HANDLE m_hApplication;
	HANDLE m_hSession;
	// Write function
	HRESULT WriteLineReferenceDescription(CLineReferenceDescription* pLineReferenceDescription);	
};

#endif // !defined(AFX_CSVFILE_H__2028AC34_01D1_48CB_80E8_08ED383F235F__INCLUDED_)
