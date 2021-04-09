// DiscoveryDlg.cpp : fichier d'implémentation
//

#include "stdafx.h"
#include "DiscoveryDlg.h"
#include "afxdialogex.h"


// Boîte de dialogue CDiscoveryDlg

IMPLEMENT_DYNAMIC(CDiscoveryDlg, CDialog)

CDiscoveryDlg::CDiscoveryDlg(CWnd* pParent /*=NULL*/, OpcUa_Handle hApplication, OpcUa_Handle hSession)
: CDialog(CDiscoveryDlg::IDD, pParent), m_hApplication(hApplication), m_hSession(hSession)
, m_hostName(_T("Localhost"))
, m_EndpointUrl(_T(""))
, m_szCurrentEndpointDescription(_T(""))
, m_bSecurityModeNone(false)
, m_bSecurityModeSign(false)
, m_bSecurityModeSignEncrypt(false)
, m_SecurityPolicyUri(_T(""))
{
	m_uiCurrentEndpointDescription = 0;
	m_pSelectedEndpointDescription = OpcUa_Null;
	m_pEndpointDescription = OpcUa_Null;
	m_RadioSecurityVal = 0;
}

CDiscoveryDlg::~CDiscoveryDlg()
{
}

void CDiscoveryDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_APP_NAME, m_ComboAppName);
	DDX_Text(pDX, IDC_EDIT_HOSTNAME, m_hostName);
	DDX_Text(pDX, IDC_EDIT_ENDPOINTURL, m_EndpointUrl);
	DDX_Text(pDX, IDC_CURRENT_ENDPOINT_DESCRIPTION, m_szCurrentEndpointDescription);
	DDX_Text(pDX, IDC_EDIT_SECURITY_POLICY_URI, m_SecurityPolicyUri);
	DDX_Radio(pDX, IDC_RADIO_NONE, m_RadioSecurityVal);
}


BEGIN_MESSAGE_MAP(CDiscoveryDlg, CDialog)
	ON_CBN_SELCHANGE(IDC_COMBO_APP_NAME, &CDiscoveryDlg::OnCbnSelchangeComboAppName)
	ON_CBN_DROPDOWN(IDC_COMBO_APP_NAME, &CDiscoveryDlg::OnCbnDropdownComboAppName)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_ENDPOINT_DESCRIPTION, &CDiscoveryDlg::OnDeltaposSpinEndpointDescription)
	ON_BN_CLICKED(IDC_SELECT_BUTTOM, &CDiscoveryDlg::OnBnClickedSelectButtom)
	ON_BN_CLICKED(IDC_REFRESH_BUTTON, &CDiscoveryDlg::OnBnClickedRefreshButton)
	ON_BN_CLICKED(IDC_RADIO_NONE, &CDiscoveryDlg::OnBnClickedRadioNone)
	ON_BN_CLICKED(IDC_RADIO_SIGN, &CDiscoveryDlg::OnBnClickedRadioSign)
	ON_BN_CLICKED(IDC_RADIO_SIGN_ENCRYPT, &CDiscoveryDlg::OnBnClickedRadioSignEncrypt)
END_MESSAGE_MAP()


// Gestionnaires de messages de CDiscoveryDlg


BOOL CDiscoveryDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_Tooltip.Create(this);
	m_Tooltip.Activate(TRUE);
	m_Tooltip.AddTool(GetDlgItem(IDC_SPIN_ENDPOINT_DESCRIPTION), L"Click to move through discoverd Endpoints");
	m_Tooltip.AddTool(GetDlgItem(IDC_EDIT_HOSTNAME), L"Enter the hostname to discover");
	m_Tooltip.AddTool(GetDlgItem(IDC_COMBO_APP_NAME), L"Application Name");
	m_Tooltip.AddTool(GetDlgItem(IDC_EDIT_ENDPOINTURL), L"Endpoint URL");
	m_Tooltip.AddTool(GetDlgItem(IDC_EDIT_SECURITY_POLICY_URI), L"Security policy URI");
	m_Tooltip.AddTool(GetDlgItem(IDC_REFRESH_BUTTON), L"Refresh Endpoint URL");
	m_Tooltip.AddTool(GetDlgItem(IDC_SELECT_BUTTOM), L"Select the current Endpoint");
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION : les pages de propriétés OCX devraient retourner FALSE
}


void CDiscoveryDlg::OnCbnSelchangeComboAppName()
{
	OpcUa_StatusCode uStatus;
	OpcUa_String discoveryUrl;
	UpdateData(TRUE);
	USES_CONVERSION;
	CString strDiscoveryUrl;
	int iIndex = m_ComboAppName.GetCurSel();
	if (iIndex != CB_ERR)
	{
		m_ComboAppName.GetLBText(iIndex, strDiscoveryUrl);
		// 
		OpcUa_String_Initialize(&discoveryUrl);
		OpcUa_String_AttachCopy(&discoveryUrl, W2A(strDiscoveryUrl.GetBuffer()));
		// lookup ApplicationName et DiscoveryUrl
		for (int i = 0; i<m_NbOfAppDescription; i++)
		{
			if (OpcUa_String_StrnCmp(&(m_pAppDescription[i].ApplicationName.Text), &discoveryUrl, OpcUa_String_StrLen(&discoveryUrl), OpcUa_False) == 0)
			{
				iIndex = i;
				break;
			}

		}
		// recupération des EndpointDescription
		uStatus = OpenOpcUa_GetEndpoints(m_hApplication,
			m_pAppDescription[iIndex].DiscoveryUrls,
			&m_NbOfEndpointDescription,
			&m_pEndpointDescription);
		if (uStatus == OpcUa_Good)
		{
			m_uiCurrentEndpointDescription = 0;
			ViewEndpointDescription();
		}
		else
		{
			char Message[50];
			ZeroMemory(Message, 50);
			sprintf(Message, "OpenOpcUa_GetEndpoints failed 0x%05x\n", uStatus);
		}
	}
	UpdateData(FALSE);
}

void CDiscoveryDlg::ViewEndpointDescription()
{
	if (m_pEndpointDescription)
	{
		m_szCurrentEndpointDescription.Format(L"%u/%u", m_uiCurrentEndpointDescription+1, m_NbOfEndpointDescription);
		// ApplicationName
		while (m_ComboAppName.DeleteString(0) != CB_ERR);
		m_ComboAppName.AddString(CString(OpcUa_String_GetRawString(&(m_pEndpointDescription[m_uiCurrentEndpointDescription].Server.ApplicationName.Text))));
		m_ComboAppName.SelectString(0, CString(OpcUa_String_GetRawString(&(m_pEndpointDescription[m_uiCurrentEndpointDescription].Server.ApplicationName.Text))));
		// EndpointUrl
		m_EndpointUrl = CString(OpcUa_String_GetRawString(&(m_pEndpointDescription[m_uiCurrentEndpointDescription].EndpointUrl)));
		// SecurityMode
		if (m_pEndpointDescription[m_uiCurrentEndpointDescription].SecurityMode == OpcUa_MessageSecurityMode_None)
			 m_RadioSecurityVal = 0;
		if (m_pEndpointDescription[m_uiCurrentEndpointDescription].SecurityMode == OpcUa_MessageSecurityMode_Sign)
			m_RadioSecurityVal = 1;
		if (m_pEndpointDescription[m_uiCurrentEndpointDescription].SecurityMode == OpcUa_MessageSecurityMode_SignAndEncrypt)
			m_RadioSecurityVal = 2;
		// SecurityPolicyUri
		m_SecurityPolicyUri = CString(OpcUa_String_GetRawString(&(m_pEndpointDescription[m_uiCurrentEndpointDescription].SecurityPolicyUri)));
	}
	UpdateData(FALSE);
}
/// <summary>
/// Called when [CBN dropdown combo application name].
/// it will call OpenOpcUa_FindServers to retrieve application description available on this host
/// This call will go to the LDS
/// </summary>
void CDiscoveryDlg::OnCbnDropdownComboAppName()
{
	OpcUa_StatusCode uStatus;
	UpdateData(TRUE);
	USES_CONVERSION;
	
	OpcUa_String hostName;
	OpcUa_String_Initialize(&hostName);
	LPTSTR pBuffer = m_hostName.GetBuffer();
	OpcUa_String_AttachCopy(&hostName, (OpcUa_CharA*)W2A(pBuffer));
	uStatus = OpenOpcUa_FindServers(m_hApplication, &hostName, &m_NbOfAppDescription, &m_pAppDescription);
	if (uStatus == OpcUa_Good)
	{
		// Vide la combo
		while (m_ComboAppName.DeleteString(0)>0);
		for (int ii = 0; ii<m_NbOfAppDescription; ii++)
		{
			if (m_pAppDescription[ii].ApplicationType == OpcUa_ApplicationType_Server)
			{
				CString strTmp(OpcUa_String_GetRawString(&(m_pAppDescription[ii].ApplicationName.Text)));
				m_ComboAppName.AddString(strTmp);
			}
		}
	}
	OpcUa_String_Clear(&hostName);
}
void CDiscoveryDlg::OnDeltaposSpinEndpointDescription(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	OpcUa_UInt32 iDelta=pNMUpDown->iDelta;
	if ((iDelta == 1) && (m_uiCurrentEndpointDescription>0))
		m_uiCurrentEndpointDescription--;
	else
	{
		if ((iDelta == 0xffffffff) && (m_uiCurrentEndpointDescription<m_NbOfEndpointDescription - 1))
			m_uiCurrentEndpointDescription++;
	}
	ViewEndpointDescription();
	*pResult = 0;
}
// 
/// <summary>
/// Called SelectButtom is clicke to transfert 
/// m_pEndpointDescription[m_uiCurrentEndpointDescription]  in m_pSelectedEndpointDescription
/// </summary>
void CDiscoveryDlg::OnBnClickedSelectButtom()
{
	if (m_pEndpointDescription)
	{
		m_pSelectedEndpointDescription=(OpcUa_EndpointDescription*)OpcUa_Alloc(sizeof(OpcUa_EndpointDescription));
		OpcUa_EndpointDescription_Initialize(m_pSelectedEndpointDescription);
		// EndpointUrl
		OpcUa_String_StrnCpy(
			&(m_pSelectedEndpointDescription->EndpointUrl),
			&(m_pEndpointDescription[m_uiCurrentEndpointDescription].EndpointUrl),
			OpcUa_String_StrLen(&(m_pEndpointDescription[m_uiCurrentEndpointDescription].EndpointUrl)));
		// ApplpicationDescription (Server)
		// ApplicationUri
		OpcUa_String_StrnCpy(
			&(m_pSelectedEndpointDescription->Server.ApplicationUri),
			&(m_pEndpointDescription[m_uiCurrentEndpointDescription].Server.ApplicationUri),
			OpcUa_String_StrLen(&(m_pEndpointDescription[m_uiCurrentEndpointDescription].Server.ApplicationUri)));
		// ProductUri
		OpcUa_String_StrnCpy(
			&(m_pSelectedEndpointDescription->Server.ProductUri),
			&(m_pEndpointDescription[m_uiCurrentEndpointDescription].Server.ProductUri),
			OpcUa_String_StrLen(&(m_pEndpointDescription[m_uiCurrentEndpointDescription].Server.ProductUri)));
		// ApplicationName
		OpcUa_String_StrnCpy(
			&(m_pSelectedEndpointDescription->Server.ApplicationName.Locale),
			&(m_pEndpointDescription[m_uiCurrentEndpointDescription].Server.ApplicationName.Locale),
			OpcUa_String_StrLen(&(m_pEndpointDescription[m_uiCurrentEndpointDescription].Server.ApplicationName.Locale)));

		OpcUa_String_StrnCpy(
			&(m_pSelectedEndpointDescription->Server.ApplicationName.Text),
			&(m_pEndpointDescription[m_uiCurrentEndpointDescription].Server.ApplicationName.Text),
			OpcUa_String_StrLen(&(m_pEndpointDescription[m_uiCurrentEndpointDescription].Server.ApplicationName.Text)));
		// ApplicationType
		m_pSelectedEndpointDescription->Server.ApplicationType = m_pEndpointDescription[m_uiCurrentEndpointDescription].Server.ApplicationType;
		// GatewayServerUri
		OpcUa_String_StrnCpy(
			&(m_pSelectedEndpointDescription->Server.GatewayServerUri),
			&(m_pEndpointDescription[m_uiCurrentEndpointDescription].Server.GatewayServerUri),
			OpcUa_String_StrLen(&(m_pEndpointDescription[m_uiCurrentEndpointDescription].Server.GatewayServerUri)));
		// DiscoveryProfileUri
		OpcUa_String_StrnCpy(
			&(m_pSelectedEndpointDescription->Server.DiscoveryProfileUri),
			&(m_pEndpointDescription[m_uiCurrentEndpointDescription].Server.DiscoveryProfileUri),
			OpcUa_String_StrLen(&(m_pEndpointDescription[m_uiCurrentEndpointDescription].Server.DiscoveryProfileUri)));
		// DiscoveryUrls
		m_pSelectedEndpointDescription->Server.NoOfDiscoveryUrls = m_pEndpointDescription[m_uiCurrentEndpointDescription].Server.NoOfDiscoveryUrls;
		if (m_pSelectedEndpointDescription->Server.NoOfDiscoveryUrls > 0)
		{
			m_pSelectedEndpointDescription->Server.DiscoveryUrls = (OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String) * m_pSelectedEndpointDescription->Server.NoOfDiscoveryUrls);
			for (OpcUa_UInt32 i = 0; i < m_pSelectedEndpointDescription->Server.NoOfDiscoveryUrls; i++)
			{
				OpcUa_String_Initialize(&(m_pSelectedEndpointDescription->Server.DiscoveryUrls[i]));
				OpcUa_String_StrnCpy(
					&(m_pSelectedEndpointDescription->Server.DiscoveryUrls[i]),
					&(m_pEndpointDescription[m_uiCurrentEndpointDescription].Server.DiscoveryUrls[i]),
					OpcUa_String_StrLen(&(m_pEndpointDescription[m_uiCurrentEndpointDescription].Server.DiscoveryUrls[i])));
			}
		}
		// ServerCertificate
		if (m_pEndpointDescription[m_uiCurrentEndpointDescription].ServerCertificate.Length > 0)
		{
			m_pSelectedEndpointDescription->ServerCertificate.Length = m_pEndpointDescription[m_uiCurrentEndpointDescription].ServerCertificate.Length;
			m_pSelectedEndpointDescription->ServerCertificate.Data = (OpcUa_Byte*)OpcUa_Alloc(m_pSelectedEndpointDescription->ServerCertificate.Length);
			ZeroMemory(m_pSelectedEndpointDescription->ServerCertificate.Data, m_pSelectedEndpointDescription->ServerCertificate.Length);
			memcpy(
				m_pSelectedEndpointDescription->ServerCertificate.Data,				
				m_pEndpointDescription[m_uiCurrentEndpointDescription].ServerCertificate.Data,
				m_pSelectedEndpointDescription->ServerCertificate.Length);
		}
		// SecurityMode
		m_pSelectedEndpointDescription->SecurityMode = m_pEndpointDescription[m_uiCurrentEndpointDescription].SecurityMode;
		// SecurityPolicyUri
		OpcUa_String_StrnCpy(
			&(m_pSelectedEndpointDescription->SecurityPolicyUri),
			&(m_pEndpointDescription[m_uiCurrentEndpointDescription].SecurityPolicyUri),
			OpcUa_String_StrLen(&(m_pEndpointDescription[m_uiCurrentEndpointDescription].SecurityPolicyUri)));
		// UserIdentityTokens
		m_pSelectedEndpointDescription->NoOfUserIdentityTokens=m_pEndpointDescription[m_uiCurrentEndpointDescription].NoOfUserIdentityTokens;
		if (m_pSelectedEndpointDescription->NoOfUserIdentityTokens)
		{
			m_pSelectedEndpointDescription->UserIdentityTokens = (OpcUa_UserTokenPolicy*)OpcUa_Alloc(sizeof(OpcUa_UserTokenPolicy)*m_pSelectedEndpointDescription->NoOfUserIdentityTokens);
			for (OpcUa_UInt32 ii = 0; ii < m_pEndpointDescription[m_uiCurrentEndpointDescription].NoOfUserIdentityTokens; ii++)
			{	
				// PolicyId
				OpcUa_String_StrnCpy(&(m_pSelectedEndpointDescription->UserIdentityTokens[ii].PolicyId),
					&(m_pEndpointDescription[m_uiCurrentEndpointDescription].UserIdentityTokens[ii].PolicyId),
					OpcUa_String_StrLen(&(m_pEndpointDescription[m_uiCurrentEndpointDescription].UserIdentityTokens[ii].PolicyId)));
				// IssuedTokenType
				m_pSelectedEndpointDescription->UserIdentityTokens[ii].IssuedTokenType = m_pEndpointDescription[m_uiCurrentEndpointDescription].UserIdentityTokens[ii].IssuedTokenType;
				// IssuedTokenType
				OpcUa_String_StrnCpy(&(m_pSelectedEndpointDescription->UserIdentityTokens[ii].IssuedTokenType),
					&(m_pEndpointDescription[m_uiCurrentEndpointDescription].UserIdentityTokens[ii].IssuedTokenType),
					OpcUa_String_StrLen(&(m_pEndpointDescription[m_uiCurrentEndpointDescription].UserIdentityTokens[ii].IssuedTokenType)));
				// IssuerEndpointUrl
				OpcUa_String_StrnCpy(&(m_pSelectedEndpointDescription->UserIdentityTokens[ii].IssuerEndpointUrl),
					&(m_pEndpointDescription[m_uiCurrentEndpointDescription].UserIdentityTokens[ii].IssuerEndpointUrl),
					OpcUa_String_StrLen(&(m_pEndpointDescription[m_uiCurrentEndpointDescription].UserIdentityTokens[ii].IssuerEndpointUrl)));
				// SecurityPolicyUri
				OpcUa_String_StrnCpy(&(m_pSelectedEndpointDescription->UserIdentityTokens[ii].SecurityPolicyUri),
					&(m_pEndpointDescription[m_uiCurrentEndpointDescription].UserIdentityTokens[ii].SecurityPolicyUri),
					OpcUa_String_StrLen(&(m_pEndpointDescription[m_uiCurrentEndpointDescription].UserIdentityTokens[ii].SecurityPolicyUri)));
			}
		}
		// TransportProfileUri
		OpcUa_String_StrnCpy(&(m_pSelectedEndpointDescription->TransportProfileUri),
			&(m_pEndpointDescription[m_uiCurrentEndpointDescription].TransportProfileUri),
			OpcUa_String_StrLen(&(m_pEndpointDescription[m_uiCurrentEndpointDescription].TransportProfileUri)));
		// SecurityLevel
		m_pSelectedEndpointDescription->SecurityLevel = m_pEndpointDescription[m_uiCurrentEndpointDescription].SecurityLevel;
	}
	CDiscoveryDlg::OnOK();
}
void CDiscoveryDlg::OnBnClickedRefreshButton()
{
	OpcUa_StatusCode uStatus;
	UpdateData(TRUE);
	USES_CONVERSION;
	OpcUa_String discoveryUrl;
	OpcUa_String_Initialize(&discoveryUrl);
	OpcUa_String_AttachCopy(&discoveryUrl, W2A(m_EndpointUrl.GetBuffer()));
	// recupération des EndpointDescription
	uStatus = OpenOpcUa_GetEndpoints(m_hApplication, &discoveryUrl,
									 &m_NbOfEndpointDescription,	&m_pEndpointDescription);
	if (uStatus == OpcUa_Good)
	{
		m_uiCurrentEndpointDescription = 1;
		ViewEndpointDescription();
	}
	UpdateData(FALSE);
}

void CDiscoveryDlg::OnBnClickedRadioNone()
{
	m_RadioSecurityVal = 0;
	ViewEndpointDescription();
}
void CDiscoveryDlg::OnBnClickedRadioSign()
{
	ViewEndpointDescription();
}


void CDiscoveryDlg::OnBnClickedRadioSignEncrypt()
{
	ViewEndpointDescription();
}
BOOL CDiscoveryDlg::PreTranslateMessage(MSG* pMsg)
{
	m_Tooltip.RelayEvent(pMsg);

	return CDialog::PreTranslateMessage(pMsg);
}