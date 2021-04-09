// UAWriteDlg.cpp : fichier d'implémentation
//

#include "stdafx.h"
#include "UAQuickClient.h"
#include "UAWriteDlg.h"


// Boîte de dialogue CUAWriteDlg

IMPLEMENT_DYNAMIC(CUAWriteDlg, CDialog)

CUAWriteDlg::CUAWriteDlg(CWnd* pParent /*=NULL*/,OpcUa_NodeId* pNodeId,HANDLE hApplication, HANDLE hSession)
	: CDialog(CUAWriteDlg::IDD, pParent),m_hApplication(hApplication),m_hSession(hSession)
	, m_strNodeId(_T(""))
	, m_NodeValueAsString(_T(""))
	, m_NodeValueNumeric(0)
	, m_iAttributeId(0)
	, m_iDatatype(0)
{
	// Init the structure that contains the value to write
	m_pValueToWrite = (OpcUa_WriteValue*)OpcUa_Alloc(sizeof(OpcUa_WriteValue));
	OpcUa_WriteValue_Initialize(m_pValueToWrite);

	OpcUa_NodeId_Initialize(&m_DataTypeNodeId);
	if (pNodeId)
	{
		OpcUa_NodeId_Initialize(&(m_pValueToWrite->NodeId));
		OpcUa_NodeId_CopyTo(pNodeId, &(m_pValueToWrite->NodeId));
	}
}

CUAWriteDlg::~CUAWriteDlg()
{
}

void CUAWriteDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_ComboAttributeId);
	DDX_Control(pDX, IDC_COMBO_DATATYPE, m_ComboDatatype);
	DDX_Text(pDX, IDC_NODEID, m_strNodeId);
	DDX_Control(pDX, IDC_LIST_ARRAY, m_NodeArrayValue);
	DDX_Text(pDX, IDC_ATTRIBUTE_VAL_STR, m_NodeValueAsString);
	DDX_Text(pDX, IDC_ATTRIBUTE_VAL, m_NodeValueNumeric);
	DDX_Control(pDX, IDC_ATTRIBUTE_VAL_STR, m_NodeValueAsStringCtrl);
	DDX_Control(pDX, IDC_ATTRIBUTE_VAL, m_NodeValueNumericCtrl);
	DDX_CBIndex(pDX, IDC_COMBO_ATTRIBUTEID, m_iAttributeId);
	DDX_CBIndex(pDX, IDC_COMBO_DATATYPE, m_iDatatype);
}


BEGIN_MESSAGE_MAP(CUAWriteDlg, CDialog)
	ON_CBN_SELCHANGE(IDC_COMBO_ATTRIBUTEID, &CUAWriteDlg::OnCbnSelchangeComboAttributeid)
	ON_CBN_SELCHANGE(IDC_COMBO_DATATYPE, &CUAWriteDlg::OnCbnSelchangeComboDatatype)
END_MESSAGE_MAP()


// Gestionnaires de messages de CUAWriteDlg

void CUAWriteDlg::InitComboAttributes(void)
{
	m_ComboAttributeId.AddString(L"NodeId");
	m_ComboAttributeId.AddString(L"NodeClass");
	m_ComboAttributeId.AddString(L"BrowseName");
	m_ComboAttributeId.AddString(L"DisplayName");
	m_ComboAttributeId.AddString(L"Description");
	m_ComboAttributeId.AddString(L"WriteMask");
	m_ComboAttributeId.AddString(L"UserWriteMask");
	m_ComboAttributeId.AddString(L"IsAbstract");
	m_ComboAttributeId.AddString(L"Symmetric");
	m_ComboAttributeId.AddString(L"InverseName");
	m_ComboAttributeId.AddString(L"ContainsNoLoops");
	m_ComboAttributeId.AddString(L"EventNotifier");
	m_ComboAttributeId.AddString(L"Value");
	m_ComboAttributeId.AddString(L"DataType");
	m_ComboAttributeId.AddString(L"ValueRank");
	m_ComboAttributeId.AddString(L"ArrayDimensions");
	m_ComboAttributeId.AddString(L"AccessLevel");
	m_ComboAttributeId.AddString(L"UserAccessLevel");
	m_ComboAttributeId.AddString(L"MinimumSamplingInterval");
	m_ComboAttributeId.AddString(L"Historizing");
	m_ComboAttributeId.AddString(L"Executable");
	m_ComboAttributeId.AddString(L"UserExecutable");
}

BOOL CUAWriteDlg::OnInitDialog()
{
	OpcUa_ReadValueId* pNodesToRead=OpcUa_Null;
	OpcUa_DataValue* pResults=OpcUa_Null; // To read attribute
	CDialog::OnInitDialog();
	// Remplissage de la boite de dialogue avec les AttributesId OPC UA
	InitComboAttributes();
	// Remplissage de la boite de dialgue avec les types de données
	InitDataType();
	// Initialisation de l'IHM en fonction du nodeId qui est demandé
	OpcUa_String strNodeId;
	OpcUa_String_Initialize(&strNodeId);
	if (OpenOpcUa_NodeIdToString(m_pValueToWrite->NodeId, &strNodeId) == OpcUa_Good)
	{
		m_strNodeId=OpcUa_String_GetRawString(&strNodeId);
		OpcUa_String_Clear(&strNodeId);
	}
	// Adjust attribute and related control
	// DataType
	pNodesToRead=(OpcUa_ReadValueId*)OpcUa_Alloc(sizeof(OpcUa_ReadValueId));
	if (pNodesToRead)
	{
		OpcUa_ReadValueId_Initialize(&pNodesToRead[0]);
		pNodesToRead[0].AttributeId = OpcUa_Attributes_DataType;
		pNodesToRead[0].NodeId = m_pValueToWrite->NodeId;
		if (OpenOpcUa_ReadAttributes(m_hApplication, m_hSession, OpcUa_TimestampsToReturn_Both, 1, pNodesToRead, &pResults) == OpcUa_Good)
		{
			OpcUa_NodeId_CopyTo(pResults[0].Value.Value.NodeId, &m_DataTypeNodeId);
			
			if (pResults[0].Value.ArrayType == OpcUa_VariantArrayType_Scalar)
			{
				if (pResults[0].Value.Value.NodeId->Identifier.Numeric == OpcUaType_String)
				{
					m_NodeValueNumericCtrl.EnableWindow(FALSE);
					m_NodeArrayValue.EnableWindow(FALSE);
					m_NodeValueAsStringCtrl.EnableWindow(TRUE);
				}
				else
				{
					m_NodeValueNumericCtrl.EnableWindow(TRUE);
					m_NodeArrayValue.EnableWindow(FALSE);
					m_NodeValueAsStringCtrl.EnableWindow(FALSE);
				}
			}
			else
			{
				if (pResults[0].Value.ArrayType == OpcUa_VariantArrayType_Array)
				{
					m_NodeValueNumericCtrl.EnableWindow(FALSE);
					m_NodeArrayValue.EnableWindow(TRUE);
					m_NodeValueAsStringCtrl.EnableWindow(FALSE);
				}
			}
			// Set the combo value based on the current DataType in the server
			if (pResults[0].Value.Value.NodeId->Identifier.Numeric)
				m_ComboDatatype.SetCurSel(pResults[0].Value.Value.NodeId->Identifier.Numeric - 1);
			// release resources 
			OpcUa_DataValue_Clear(pResults);
			OpcUa_Free(pResults);
		}
		OpcUa_ReadValueId_Clear(&pNodesToRead[0]);
		OpcUa_Free(pNodesToRead);
	}
	// initialize Attribute to Value
	m_pValueToWrite->AttributeId = OpcUa_Attributes_Value;
	m_iAttributeId = 12;
	// AttributeId. Default will be OpcUa_Attributes_Value
	m_ComboAttributeId.SetCurSel(OpcUa_Attributes_Value-1);
	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
}

void CUAWriteDlg::InitDataType(void)
{
	m_ComboDatatype.AddString(L"Boolean");//1
	m_ComboDatatype.AddString(L"SByte"); //2
	m_ComboDatatype.AddString(L"Byte"); // 3
	m_ComboDatatype.AddString(L"Int16"); //4
	m_ComboDatatype.AddString(L"UInt16");//5
	m_ComboDatatype.AddString(L"Int32");//6
	m_ComboDatatype.AddString(L"UInt32");//7
	m_ComboDatatype.AddString(L"Int64");//8
	m_ComboDatatype.AddString(L"UInt64");// 9
	m_ComboDatatype.AddString(L"Float");//10
	m_ComboDatatype.AddString(L"Double");//11
	m_ComboDatatype.AddString(L"String");//12
	m_ComboDatatype.AddString(L"DateTime");//13
	m_ComboDatatype.AddString(L"Guid");//14
	m_ComboDatatype.AddString(L"ByteString");//15
	m_ComboDatatype.AddString(L"XmlElement");//16
	m_ComboDatatype.AddString(L"NodeId");//17
	m_ComboDatatype.AddString(L"ExpandedNodeId");//18
	m_ComboDatatype.AddString(L"StatusCode");//19
	m_ComboDatatype.AddString(L"QualifiedName"); //20
	m_ComboDatatype.AddString(L"LocalizedText");//21
}
 OpcUa_StatusCode CUAWriteDlg::GetValueToWrite(OpcUa_WriteValue** ppWriteValue)
{
	 OpcUa_StatusCode uStatus = OpcUa_Good;
	 USES_CONVERSION;
	// Mise en forme de m_pValueToWrite en fonction de m_DataTypeNodeId
	m_pValueToWrite->NodeId;
	if (m_DataTypeNodeId.IdentifierType == OpcUa_IdentifierType_Numeric)
	{
		OpcUa_WriteValue_Initialize((*ppWriteValue));
		(*ppWriteValue)->AttributeId = m_pValueToWrite->AttributeId;
		OpcUa_String_StrnCpy(&((*ppWriteValue)->IndexRange), &(m_pValueToWrite->IndexRange), OpcUa_String_StrLen(&(m_pValueToWrite->IndexRange)));
		OpcUa_NodeId_CopyTo(&(m_pValueToWrite->NodeId), &((*ppWriteValue)->NodeId));
		// m_pValueToWrite->Value;
		(*ppWriteValue)->Value.Value.Datatype = (OpcUa_Byte)m_DataTypeNodeId.Identifier.Numeric;
		switch (m_DataTypeNodeId.Identifier.Numeric)
		{
		case OpcUaType_Boolean:
			(*ppWriteValue)->Value.Value.Value.Boolean = (OpcUa_Boolean)m_NodeValueNumeric;
			break;
		case OpcUaType_SByte:
			(*ppWriteValue)->Value.Value.Value.SByte = (OpcUa_SByte)m_NodeValueNumeric;
			break;
		case OpcUaType_Byte:
			(*ppWriteValue)->Value.Value.Value.Byte = (OpcUa_Byte)m_NodeValueNumeric;
			break;
		case OpcUaType_Int16:
			(*ppWriteValue)->Value.Value.Value.Int16 = (OpcUa_Int16)m_NodeValueNumeric;
			break;
		case OpcUaType_UInt16:
			(*ppWriteValue)->Value.Value.Value.UInt16 = (OpcUa_UInt16)m_NodeValueNumeric;
			break;
		case OpcUaType_Int32:
			(*ppWriteValue)->Value.Value.Value.Int32 = (OpcUa_Int32)m_NodeValueNumeric;
			break;
		case OpcUaType_UInt32:
			(*ppWriteValue)->Value.Value.Value.UInt32 = (OpcUa_UInt32)m_NodeValueNumeric;
			break;
		case OpcUaType_Int64:
			(*ppWriteValue)->Value.Value.Value.Int64 = (OpcUa_Int64)m_NodeValueNumeric;
			break;
		case OpcUaType_UInt64:
			(*ppWriteValue)->Value.Value.Value.UInt64 = (OpcUa_UInt64)m_NodeValueNumeric;
			break;
		case OpcUaType_Float:
			(*ppWriteValue)->Value.Value.Value.Float = (OpcUa_Float)m_NodeValueNumeric;
			break;
		case OpcUaType_Double:
			(*ppWriteValue)->Value.Value.Value.Double = (OpcUa_Double)m_NodeValueNumeric;
			break;
		case OpcUaType_String:			
			char* pszValue=W2A(m_NodeValueAsString.GetBuffer());
			OpcUa_String_AttachCopy(&((*ppWriteValue)->Value.Value.Value.String), pszValue);
			break;
		}
	}
	else
		uStatus=OpcUa_Bad;
	return uStatus;
 }

 void CUAWriteDlg::OnCbnSelchangeComboAttributeid()
 {
	 UpdateData(TRUE);
	 m_pValueToWrite->AttributeId = m_iAttributeId+1;
 }

 void CUAWriteDlg::OnCbnSelchangeComboDatatype()
 {
	 UpdateData(TRUE);
 }
