#pragma once
#include "afxole.h"
class CInterfaceDropTarget :
	public COleDropTarget
{
public:
	CInterfaceDropTarget();
	~CInterfaceDropTarget();
protected:
	BOOL CInterfaceDropTarget::OnDrop(CWnd *pDrop,
		COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
	DROPEFFECT CInterfaceDropTarget::OnDragOver(CWnd *pDrop,
		COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	BOOL IsDropable(CPoint point);
};

