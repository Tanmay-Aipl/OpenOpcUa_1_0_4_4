// CSVFile.cpp: implementation of the CCSVFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CSVFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
UCHAR CrLf[2]={0x0D,0x0A};
CLineReferenceDescription::CLineReferenceDescription()
{
	
}
CLineReferenceDescription::CLineReferenceDescription(OpcUa_ReferenceDescription* pReferenceDescription,HANDLE  hApplication,HANDLE hSession)
{
	OpcUa_StatusCode uStatus;
	OpcUa_DataValue* pResults=OpcUa_Null;
	OpcUa_ReadValueId* pNodesToRead=OpcUa_Null;

	m_hApplication= hApplication;
	m_hSession= hSession;
	//m_BrowseName
	OpcUa_String_StrnCpy(&(m_BrowseName.Name), &(pReferenceDescription->BrowseName.Name),OpcUa_String_StrLen(&(pReferenceDescription->BrowseName.Name)));
	m_BrowseName.NamespaceIndex=pReferenceDescription->BrowseName.NamespaceIndex;
	// m_DisplayName
	OpcUa_String_StrnCpy(&(m_DisplayName.Locale), &(pReferenceDescription->DisplayName.Locale),OpcUa_String_StrLen(&(pReferenceDescription->DisplayName.Locale)));
	OpcUa_String_StrnCpy(&(m_DisplayName.Text), &(pReferenceDescription->DisplayName.Text),OpcUa_String_StrLen(&(pReferenceDescription->DisplayName.Text)));
	// m_NodeId
	OpcUa_String_StrnCpy(&(m_NodeId.NamespaceUri),&(pReferenceDescription->NodeId.NamespaceUri),OpcUa_String_StrLen(&(pReferenceDescription->NodeId.NamespaceUri)));
	m_NodeId.NodeId=pReferenceDescription->NodeId.NodeId;
	m_NodeId.ServerIndex=pReferenceDescription->NodeId.ServerIndex;
	// m_NodeClass
	m_NodeClass=pReferenceDescription->NodeClass;
	if (m_NodeClass==OpcUa_NodeClass_Variable)
	{
		OpcUa_Int32 iNodNodesToRead=1;
		pNodesToRead=(OpcUa_ReadValueId*)OpcUa_Alloc(iNodNodesToRead*sizeof(OpcUa_ReadValueId));
		OpcUa_ReadValueId_Initialize(&pNodesToRead[0]);
		pNodesToRead[0].AttributeId=OpcUa_Attributes_DataType;
		pNodesToRead[0].NodeId=m_NodeId.NodeId;
		uStatus=OpenOpcUa_ReadAttributes(m_hApplication,m_hSession,OpcUa_TimestampsToReturn_Both,iNodNodesToRead,pNodesToRead,&pResults);
		if (uStatus==OpcUa_Good)
		{
			// dans le cas du DataType le type est dans un NodeId
			if (pResults[0].StatusCode==OpcUa_Good)
				m_DataType=pResults[0].Value.Value.NodeId->Identifier.Numeric;
		}
		OpcUa_Free(pNodesToRead);
	}
	// m_ReferenceTypeId
	switch (pReferenceDescription->ReferenceTypeId.IdentifierType)
	{
	case OpcUa_IdentifierType_Numeric:
		m_ReferenceTypeId.Identifier.Numeric=pReferenceDescription->ReferenceTypeId.Identifier.Numeric;
		break;
	case OpcUa_IdentifierType_String:
		OpcUa_String_StrnCpy(&(m_ReferenceTypeId.Identifier.String),&(pReferenceDescription->ReferenceTypeId.Identifier.String),OpcUa_String_StrLen(&(pReferenceDescription->ReferenceTypeId.Identifier.String)));
		break;
	default:
		ASSERT(0);
	}
	m_ReferenceTypeId.NamespaceIndex=pReferenceDescription->ReferenceTypeId.NamespaceIndex;
	m_ReferenceTypeId.IdentifierType=pReferenceDescription->ReferenceTypeId.IdentifierType;
	// TypeDefinition
	OpcUa_String_StrnCpy(&(m_TypeDefinition.NamespaceUri),&(pReferenceDescription->TypeDefinition.NamespaceUri),OpcUa_String_StrLen(&(pReferenceDescription->TypeDefinition.NamespaceUri)));
	m_TypeDefinition.NodeId=pReferenceDescription->TypeDefinition.NodeId;
	m_TypeDefinition.ServerIndex=pReferenceDescription->TypeDefinition.ServerIndex;
	
}
CLineReferenceDescription::~CLineReferenceDescription()
{
}

HRESULT CLineReferenceDescription::UpdateDataType(OpcUa_NodeId aNodeId)
{
	HRESULT hr=S_OK;
	OpcUa_StatusCode uStatus=OpcUa_Bad;
	OpcUa_Int32 iNoOfNodesToRead=1;
	OpcUa_ReadValueId* pNodesToRead=OpcUa_Null;
	OpcUa_DataValue* pResults=OpcUa_Null;

	pNodesToRead=(OpcUa_ReadValueId*)OpcUa_Alloc(sizeof(OpcUa_ReadValueId));
	OpcUa_ReadValueId_Initialize(pNodesToRead);
	pNodesToRead->AttributeId=OpcUa_Attributes_DataType;
	pNodesToRead->NodeId=aNodeId;
	
	uStatus=OpenOpcUa_ReadAttributes(m_hApplication,
		m_hSession,
		OpcUa_TimestampsToReturn_Both,
		iNoOfNodesToRead,
		pNodesToRead,
		&pResults);

	return hr;
}
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCSVFile::CCSVFile()
{
	m_hParent=NULL;
	m_iExportedCounter=0;
}

CCSVFile::CCSVFile(LPCTSTR /*lpszFileName*/)
{
	m_hParent=NULL;
	m_NodeClassMask=OpcUa_NodeClass_Object|OpcUa_NodeClass_Variable;
	m_iExportedCounter=0;
}
CCSVFile::~CCSVFile()
{
	m_NodeClassMask=OpcUa_NodeClass_Object|OpcUa_NodeClass_Variable;
}
CString CCSVFile::GetFileName()
{	
	return m_strCSVFileName;
}
HRESULT CCSVFile::SetFileName()
{
	HRESULT hr=S_FALSE;
	LPCTSTR szFilter = _T("CSV. files (*.csv)|*.csv||/All Files (*.*)|*.*||");
	DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	CString CSVFileName;
	CString message;
	CFileDialog* pfDlg=new CFileDialog( FALSE, 
		_T("csv"), _T(".csv"), dwFlags, szFilter, m_hParent);	
	if (pfDlg->DoModal()==IDOK)
	{
		m_strCSVFileName=pfDlg->GetPathName();
		m_strCSVFileName=pfDlg->m_ofn.lpstrFile;
		hr=S_OK;
	}
	return hr;
}
HRESULT CCSVFile::ExportCSVFile(OpcUa_NodeId aRootNode)
{
	HRESULT hr=S_FALSE;
	if(m_strCSVFileName.IsEmpty())
		hr=SetFileName();
	//
	// On commence par ouvrir le fichier
	//CFileException e;
	//if (Open(m_strCSVFileName,CFile::modeCreate | CFile::modeWrite,&e))
	{

		OpcUa_StatusCode uStatus;
		OpcUa_Int32 iNoOfReferenceDescription=0;
		OpcUa_ReferenceDescription* pRefenceList=(OpcUa_ReferenceDescription*)OpcUa_Alloc(sizeof(OpcUa_ReferenceDescription));
		if ( m_hApplication && m_hSession)
		{
			// init Browse Description
			OpcUa_BrowseDescription* pNodesToBrowse=OpcUa_Null;
			OpcUa_Int32 a_nNoOfNodesToBrowse=1;
			pNodesToBrowse=(OpcUa_BrowseDescription*)OpcUa_Alloc((sizeof(OpcUa_BrowseDescription)*a_nNoOfNodesToBrowse));
			OpcUa_BrowseDescription_Initialize(&pNodesToBrowse[0]);
			pNodesToBrowse[0].BrowseDirection=(OpcUa_BrowseDirection)m_iBrowseDirection; 
			pNodesToBrowse[0].IncludeSubtypes=TRUE;
			pNodesToBrowse[0].NodeClassMask=m_NodeClassMask; //
			//OpcUa_UInt32 aTmpiVal=OpcUa_NodeClass_Object|OpcUa_NodeClass_Variable;
			pNodesToBrowse[0].ResultMask=OpcUa_BrowseResultMask_All;
			// On browse a partir de aRootNode
			if (aRootNode.IdentifierType==OpcUa_IdentifierType_Numeric)
				pNodesToBrowse[0].NodeId.Identifier.Numeric=aRootNode.Identifier.Numeric;// OpcUaId_ObjectsFolder;
			else
			{
				if (aRootNode.IdentifierType==OpcUa_IdentifierType_String)
					OpcUa_String_StrnCpy(&(pNodesToBrowse[0].NodeId.Identifier.String),
										 &(aRootNode.Identifier.String),
										 OpcUa_String_StrLen(&(aRootNode.Identifier.String)) );
			}
			pNodesToBrowse[0].NodeId.IdentifierType=aRootNode.IdentifierType;//OpcUa_IdentifierType_Numeric;
			pNodesToBrowse[0].NodeId.NamespaceIndex=aRootNode.NamespaceIndex;// 0;

			OpcUa_NodeId_Initialize(&(pNodesToBrowse[0].ReferenceTypeId));
			pNodesToBrowse[0].ReferenceTypeId.IdentifierType=OpcUa_IdentifierType_Numeric;
			pNodesToBrowse[0].ReferenceTypeId.NamespaceIndex=0;
			pNodesToBrowse[0].ReferenceTypeId.Identifier.Numeric=OpcUaId_References;
			uStatus=OpenOpcUa_Browse(m_hApplication,
									m_hSession,
									a_nNoOfNodesToBrowse,
									pNodesToBrowse,
									&iNoOfReferenceDescription,
				&pRefenceList);
			if(uStatus==OpcUa_Good)
			{
				CString message;
				message.Format(L"OpenOpcUa_Browse succeed");			
				if (iNoOfReferenceDescription)
				{
					for (OpcUa_Int32 ii=0;ii<iNoOfReferenceDescription;ii++)
					{
						CLineReferenceDescription* pLineReferenceDescription=new CLineReferenceDescription(&pRefenceList[ii],m_hApplication,m_hSession);
						if (pLineReferenceDescription)
						{
							pLineReferenceDescription->SethApplication(m_hApplication);
							pLineReferenceDescription->SethSession(m_hSession);
							pRefenceList[ii].TypeDefinition.NodeId;
							WriteLineReferenceDescription(pLineReferenceDescription);
							// appel recusif
							if ( (pRefenceList[ii].IsForward) 
								&& ( (pRefenceList[ii].NodeClass==OpcUa_NodeClass_Variable) || (pRefenceList[ii].NodeClass==OpcUa_NodeClass_Object)) )
								ExportCSVFile(pRefenceList[ii].NodeId.NodeId);
							else
								TRACE("Très bizarre\n");
						}
					}
				}
			}
		}
		// fermeture du fichier d'export
		//Close();
	}

	return hr;
}

HRESULT CCSVFile::WriteLineReferenceDescription(CLineReferenceDescription* pLineReferenceDescription)
{
	HRESULT hr=S_OK;
	CString strOutput;
	OpcUa_String strNodeId;
	OpcUa_String_Initialize(&strNodeId);
	// Mise à jour du dataType:
	pLineReferenceDescription->UpdateDataType(pLineReferenceDescription->m_NodeId.NodeId);
	OpcUa_StatusCode uStatus=OpenOpcUa_NodeIdToString(pLineReferenceDescription->m_NodeId.NodeId,&strNodeId);
	if(uStatus==OpcUa_Good)
	{
		if( pLineReferenceDescription->m_NodeClass==OpcUa_NodeClass_Variable)
			strOutput.Format(L"%s,%s,%s",
				OpcUa_String_GetRawString(&strNodeId),
				OpcUa_String_GetRawString(&(pLineReferenceDescription->m_DisplayName.Text)),
				OpcUa_String_GetRawString(pLineReferenceDescription->FromDataType()));
		else
			strOutput.Format(L"%s,%s",
				OpcUa_String_GetRawString(&strNodeId),
				OpcUa_String_GetRawString(&(pLineReferenceDescription->m_DisplayName.Text)));
	}
	// On est pret pour ecrire
	//strOutput.Format(L"%u,%s,%s,%u:%u,%u, %u,%u,%u,%u,%u,%u,%u,%s,%u,%u;",
	//	pLineTag->m_wDeviceNum,
	//	pLineTag->m_strTagFullName,
	//	pLineTag->m_strTagDescription,
	//	pLineTag->m_dwTagPLCLocation,
	//	pLineTag->m_dwTagPLCBitnumber,
	//	pLineTag->m_wTagPLCType,
	//	pLineTag->m_wTagDataType,
	//	pLineTag->m_bTagScaling,
	//	pLineTag->m_dwTagRawMin,
	//	pLineTag->m_dwTagRawMax,
	//	pLineTag->m_dwTagEngMin,
	//	pLineTag->m_dwTagEngMax,
	//	pLineTag->m_bTagConversion,
	//	pLineTag->m_strTagUnit,
	//	pLineTag->m_wTagDeadband,
	//	pLineTag->m_wTagLogband);
	// Ecriture dans le fichier CVS
	TRACE("%s\n",strOutput);
	int len=strOutput.GetLength();
	try
	{
		Write(strOutput.GetBuffer(len),len);
		Write(CrLf,2);
		m_iExportedCounter++;
	}
	catch(CFileException* e)
	{
		CString Message;
		if (e)
			Message.Format(L"Export failed>WriteTagLine fail %s",e->m_cause);
		else
			Message.Format(L"Export failed>WriteTagLine fail");		
		
		Message.ReleaseBuffer();Message.Empty();
		if (e)
			hr=e->m_lOsError;
		else
			hr=E_FAIL;
	}
	return hr;
}
//HRESULT CCSVFile::ReadOneField(CString* strField)
//{
//	UINT rCount=1;
//	UCHAR uBuf;
//
//	while (rCount)
//	{
//		rCount=Read(&uBuf,1);
//		if (rCount)
//		{
//			if ( (uBuf!=0x2C) && (uBuf!=0x3B) )// ',' virgule ou ";" point virgule (fin ligne)
//			{
//				CString str;
//				str.Format("%c",uBuf);
//				*strField+=str;
//			}
//			else
//			{
//				if (uBuf==0x3B)
//					ReadCrLf();
//				return S_OK;
//			}
//		}
//	}
//	return S_FALSE;
//}

//HRESULT CCSVFile::ReadCrLf()
//{
//	UINT rCount=1;
//	UCHAR uBuf[2];
//	HRESULT hr;
//	rCount=Read(&uBuf[0],2);
//	if (rCount==2)
//	{
//		hr=S_OK;
//	}
//	else
//		hr=S_FALSE;
//	return hr;
//}

// Conversion du type de donnée en type variant
OpcUa_String* CLineReferenceDescription::FromDataType(void)
{
	OpcUa_String* vtStr=OpcUa_Null;
	vtStr=(OpcUa_String*)OpcUa_Alloc(sizeof(OpcUa_String));
	OpcUa_String_Initialize(vtStr);
	switch (m_DataType)
	{
	case OpcUaType_Boolean:
		OpcUa_String_AttachCopy(vtStr,"Boolean");
		break;
	case OpcUaType_Byte:
		OpcUa_String_AttachCopy(vtStr,"Byte");
		break;
	case OpcUaType_DateTime:
		OpcUa_String_AttachCopy(vtStr,"DateTime");
		break;
	case OpcUaType_Double:
		OpcUa_String_AttachCopy(vtStr,"Double");
		break;
	case OpcUaType_Float:
		OpcUa_String_AttachCopy(vtStr,"Float");
		break;
	case OpcUaType_Int16:
		OpcUa_String_AttachCopy(vtStr,"Int16");
		break;
	case OpcUaType_Int32:
		OpcUa_String_AttachCopy(vtStr,"Int32");
		break;
	case OpcUaType_Int64:
		OpcUa_String_AttachCopy(vtStr,"Int64");
		break;
	case OpcUaType_UInt16:
		OpcUa_String_AttachCopy(vtStr,"UInt16");
		break;
	case OpcUaType_UInt32:
		OpcUa_String_AttachCopy(vtStr,"UInt32");
		break;
	case OpcUaType_UInt64:
		OpcUa_String_AttachCopy(vtStr,"UInt64");
		break;
	case OpcUaType_String:
		OpcUa_String_AttachCopy(vtStr,"String");
		break;
	case OpcUaType_SByte:
		OpcUa_String_AttachCopy(vtStr,"SByte");
		break;
	case 14:
		OpcUa_String_AttachCopy(vtStr,"Guid");
		break;
	case 15:
		OpcUa_String_AttachCopy(vtStr,"ByteString");
		break;
	case 17:
		OpcUa_String_AttachCopy(vtStr,"NodeId");
		break;
	case 18:
		OpcUa_String_AttachCopy(vtStr,"ExpandedNodeId");
		break;
	case 21:
		OpcUa_String_AttachCopy(vtStr,"LocalizedText");
		break;
	case 294:
		OpcUa_String_AttachCopy(vtStr,"UtcTime");
		break;
	case 884:
		OpcUa_String_AttachCopy(vtStr,"Range");
		break;
	default:
		OpcUa_String_AttachCopy(vtStr,"unknown");
		break;


	}
	return vtStr;
}


