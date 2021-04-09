// SubEdit.cpp : fichier d'implémentation
//

#include "stdafx.h"
#include "SubEdit.h"


// CSubEdit

IMPLEMENT_DYNAMIC(CSubEdit, CEdit)

CSubEdit::CSubEdit()
{
}

CSubEdit::~CSubEdit()
{
}


BEGIN_MESSAGE_MAP(CSubEdit, CEdit)
	ON_WM_WINDOWPOSCHANGING()
END_MESSAGE_MAP()



// Gestionnaires de messages CSubEdit


void CSubEdit::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	lpwndpos->x=m_x;
	CEdit::OnWindowPosChanging(lpwndpos);
}
