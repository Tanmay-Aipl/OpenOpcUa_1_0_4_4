#include "stdafx.h"
#include "InterfaceDropTarget.h"
#include "..\resource.h"
#include "UAQuickClientDlg.h"

CInterfaceDropTarget::CInterfaceDropTarget()
{
}


CInterfaceDropTarget::~CInterfaceDropTarget()
{
}
BOOL CInterfaceDropTarget::IsDropable(CPoint point)
{
	CUAQuickClientDlg* pUAQuickClient = (CUAQuickClientDlg*)AfxGetApp()->m_pMainWnd;
	BOOL bResult = FALSE;

	CRect MonitoredEltsrect;
	pUAQuickClient->m_MonitoredElts.GetWindowRect(&MonitoredEltsrect);
	pUAQuickClient->ClientToScreen(&point);
	UINT uFlags;
	int iHitTest = pUAQuickClient->m_MonitoredElts.HitTest(point, &uFlags);
	if ((MonitoredEltsrect.top<point.y) && (MonitoredEltsrect.bottom>point.y)
		&& (MonitoredEltsrect.left<point.x) && (MonitoredEltsrect.right>point.x))
		bResult = TRUE;
	return bResult;
}
DROPEFFECT CInterfaceDropTarget::OnDragOver(CWnd *pDrop,
	COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	UINT DragDropFormat;
	CString format = AfxGetApp()->GetProfileString(L"DragDrop", L"Clipformat", L"Common");
	if (format == "Private")
		DragDropFormat = ::RegisterClipboardFormat(L"OpenOpcUAQuickClientFormat");
	else
		DragDropFormat = CF_TEXT;
	if (IsDropable(point))
		return DROPEFFECT_COPY; // data fits
	else
		return DROPEFFECT_NONE; // data won't fit
}

BOOL CInterfaceDropTarget::OnDrop(CWnd *pDrop,
	COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point)
{
	OpcUa_StatusCode uStatus = OpcUa_Good;
	UINT DragDropFormat;
	CString format = AfxGetApp()->GetProfileString(L"DragDrop", L"Clipformat", L"Common");
	if (format == "Private")
		DragDropFormat = ::RegisterClipboardFormat(L"OpenOpcUAQuickClientFormat");
	else
		DragDropFormat = CF_TEXT;

	CFile *pFile = pDataObject->GetFileData(DragDropFormat);
	if (pFile != NULL)
	{
		OpcUa_NodeId aNodeId;
		OpcUa_NodeId_Initialize(&aNodeId);
		CString Data;
		//perhaps some point checking first?
		TRY
		{
			CArchive ar(pFile, CArchive::load);
			TRY
			{
				USES_CONVERSION;
				ar >> Data;
				OpcUa_String szNodeId;
				OpcUa_String_Initialize(&szNodeId);
				OpcUa_String_AttachCopy(&szNodeId, W2A(Data.GetBuffer()));
				if (OpenOpcUa_StringToNodeId(szNodeId, &aNodeId) == OpcUa_Good)
				{
					OpcUa_UInt32 iNoOfItemsToCreate = 1;
					OpcUa_MonitoredItemCreateRequest* pItemsToCreate = (OpcUa_MonitoredItemCreateRequest*)OpcUa_Alloc(sizeof(OpcUa_MonitoredItemCreateRequest)*iNoOfItemsToCreate);
					if (pItemsToCreate)
					{
						OpcUa_MonitoredItemCreateRequest_Initialize(&pItemsToCreate[0]);

						pItemsToCreate[0].ItemToMonitor.NodeId = aNodeId;
						pItemsToCreate[0].ItemToMonitor.AttributeId = OpcUa_Attributes_Value;
						pItemsToCreate[0].MonitoringMode = OpcUa_MonitoringMode_Reporting;
						OpcUa_ExtensionObject* pExtensionObject=OpcUa_Null;
						uStatus = OpenOpcUa_CreateFilterObject(OpcUa_DeadbandType_None, 0, OpcUa_DataChangeTrigger_StatusValueTimestamp, &pExtensionObject); 
						OpcUa_ExtensionObject_CopyTo(pExtensionObject, &(pItemsToCreate[0].RequestedParameters.Filter));
						OpcUa_MonitoredItemCreateResult* ppResult = OpcUa_Null;
						HANDLE* hMonitoredItems = OpcUa_Null;
						HANDLE hSession = OpcUa_Null;
						HANDLE hSubscription = OpcUa_Null;
						CUAQuickClientDlg* pUAQuickClient = (CUAQuickClientDlg*)AfxGetApp()->m_pMainWnd;
						pUAQuickClient->GetCurrentSessionandSubscription(&hSession, &hSubscription);
						if (!hSubscription)
						{
							// Suggest to create a new subscription
							pUAQuickClient->OnCreateSubscription();
							// Retrieve handles of the new created subscription
							pUAQuickClient->GetCurrentSessionandSubscription(&hSession, &hSubscription);
						}
						if ((hSession != OpcUa_Null) && (hSubscription != OpcUa_Null))
						{
							uStatus = OpenOpcUa_CreateMonitoredItems(
								pUAQuickClient->m_hApplication,
								hSession,
								hSubscription,
								OpcUa_TimestampsToReturn_Both,
								iNoOfItemsToCreate,
								pItemsToCreate,
								&ppResult,
								&hMonitoredItems);
							if (uStatus == OpcUa_Good)
							{
								OpenOpcUa_InternalNode* pInternalNode = OpcUa_Null;
								ppResult[0].MonitoredItemId;
								uStatus = OpenOpcUa_GetInternalNode(pUAQuickClient->m_hApplication, hSession, hSubscription, hMonitoredItems[0], &pInternalNode);
								if (uStatus == OpcUa_Good)
								{
									pUAQuickClient->InsertMonitoredItemInListCtrl(pInternalNode);
								}
								else
								{
									USES_CONVERSION;
									CString message;
									message.Format(L"OnAddmonitoreditem>OpenOpcUa_GetInternalNode failed uStatus=0x%05x", uStatus);
									pUAQuickClient->AddLoggerMessage(W2A(message.GetBuffer()), 0);
									message.Empty(); message.ReleaseBuffer();
								}
							}
						}
						OpcUa_ExtensionObject_Clear(pExtensionObject);
						OpcUa_Free(pExtensionObject);
						OpcUa_MonitoredItemCreateRequest_Clear(&pItemsToCreate[0]);
						OpcUa_Free(pItemsToCreate);
					}
				}
				OpcUa_String_Clear(&szNodeId);
				ar.Close();
			}
				CATCH_ALL(eInner)
			{
				// exception while reading
				// from or closing the archive
				ASSERT(FALSE);
			}
			END_CATCH_ALL;
		}
		CATCH_ALL(eOuter)
		{
			// exception in the destructor of ar
			ASSERT(FALSE);
		}
		END_CATCH_ALL;
		//OpcUa_NodeId_Clear(&aNodeId);
		return TRUE;
	}
	return COleDropTarget::OnDrop(pDrop,
		pDataObject, dropEffect, point);
}