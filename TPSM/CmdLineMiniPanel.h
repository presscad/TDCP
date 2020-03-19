#pragma once


// CCmdLineMiniPanel ¶Ô»°¿ò

#include "afxdialogex.h"
class CCmdLineDlg;
class CCmdLineMiniPanel : public CDialogEx
{
	CCmdLineDlg* m_pCmdLineDlg;
	friend CCmdLineDlg;
	virtual UINT DlgResourceID()=0;
	virtual void SetParentCmdLineDlg(CCmdLineDlg* pParentCmdLine){
		m_pCmdLineDlg=pParentCmdLine;
	}
public:
	CCmdLineMiniPanel(UINT nIDTemplate,CWnd* pParent = NULL);
	virtual ~CCmdLineMiniPanel();
};
