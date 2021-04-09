#pragma once

//#undef  AFX_DATA
//#define AFX_DATA AFX_EXT_CLASS
// CSubEdit

class /*AFX_EXT_CLASS*/ CSubEdit : public CEdit
{
	DECLARE_DYNAMIC(CSubEdit)

public:
	CSubEdit();
	virtual ~CSubEdit();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);
	int m_x;
};
//#undef  AFX_DATA
//#define AFX_DATA

