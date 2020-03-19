#if !defined(AFX_CMDLINEDLG_H__C98B7E8D_74F1_498A_BB22_875F080B3C06__INCLUDED_)
#define AFX_CMDLINEDLG_H__C98B7E8D_74F1_498A_BB22_875F080B3C06__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CmdLineDlg.h : header file
//
#include "afxwin.h"
#include "resource.h"
#include "LinkLabel.h"
#include "CmdLineMiniPanel.h"
#include "PropListItem.h"
#include "ArrayList.h"
#include "I_DrawSolid.h"
/////////////////////////////////////////////////////////////////////////////
// CCmdLineDlg dialog

class CCmdLineDlg : public CDialog
{
	DECLARE_DYNCREATE(CCmdLineDlg)

	CCmdLineMiniPanel* m_pMiniPanel;
// Construction
public:
	CString m_sPreCmd;
	BOOL IsHasFocus();
	void RelayoutWnd();
	HWND m_hMsgProcessWnd;
	void FinishCmdLine(const char* inputCmdStr=NULL);
	void CancelCmdLine();
	void FillCmdLine(CString cmdPromptStr,CString cmdStr);
	void PromptStrInCmdLine(const char* cmdPromptStr);
	BOOL AttachMiniCmdOptionPanel(CCmdLineMiniPanel* pMiniPanel,int cx);
	void DetachMiniCmdOptionPanel();
	HWND SetCmdLineFocus();
	int m_nCmdLineHeight;
	int m_nCmdPromptWndWidth;
	BOOL m_bGettingStrFromCmdLine;
	BOOL m_bForceExitCommand;	//强行退出当前执行的命令
	CCmdLineDlg(CWnd* pParent = NULL);   // standard constructor
	static const DWORD KEYWORD_AS_RETURN = 0x00000001;
	static const DWORD LBUTTONDOWN_AS_RETURN = 0x00000002;	//鼠标左键按下
	static const DWORD LBUTTONUP_AS_RETURN	 = 0x00000004;	//鼠标左键抬起
	BOOL GetStrFromCmdLine(CString& cmdStr,DWORD dwFlag=0,const char* keywords=NULL);
	BOOL PeekStrFromCmdLine(CString& cmdStr);

	
// Dialog Data
	//{{AFX_DATA(CCmdLineDlg)
	enum { IDD = IDD_CMD_LINE_DLG };
	CColorLabel m_ctrlCmdLabel;
	CRichEditCtrl	m_richCmdHistory;
	CString	m_sCmdPrompt;
	CString	m_sCmdHistory;
	CString	m_sUserInputCmd;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCmdLineDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCmdLineDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnReturnEUserInputParam(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnPaint();
	afx_msg void OnSelchangeCmdHistory(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
class PEEKCMDLINE{
	ARRAY_LIST<CXhChar16> m_arrKeywords;
	CCmdLineDlg* m_pCmdLine;
public:
	PEEKCMDLINE(const char* keywords,CCmdLineDlg* pCmdLine=NULL);
	CCmdLineDlg* GetCmdLineWnd(){return m_pCmdLine;}
	bool InitKeywords(const char* keywords);
	bool MatchKeyword();
	static int MatchKeyword(ISolidSnap::SNAPMSGLOOP_CONTEXT* pContext);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CMDLINEDLG_H__C98B7E8D_74F1_498A_BB22_875F080B3C06__INCLUDED_)
