#pragma once
// CRememberLocationDlg
#include "HashTable.h"

class CRememberLocationDlg : public CDialogEx
{
public:
	CRememberLocationDlg(UINT nIDTemplate,CWnd* pParent = NULL);

protected:
	long m_nIDTemplate;
	bool m_bEnableWindowMoveListen;
	static CHashList<CPoint> g_hashLocationPosByDlgID;
	//
	void MoveWndToLocation();
	afx_msg void OnMove(int x, int y);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	DECLARE_MESSAGE_MAP()
};


