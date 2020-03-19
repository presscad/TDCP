#if !defined(AFX_CALLBACKDIALOG_H__F07AFCF4_8860_4C20_A03A_E723AB08E4B6__INCLUDED_)
#define AFX_CALLBACKDIALOG_H__F07AFCF4_8860_4C20_A03A_E723AB08E4B6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CallBackDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCallBackDialog dialog

class CCallBackDialog : public CDialog
{
	UINT   m_message;	//ExecuteCommand传入的参数，临时保存用于调用FireCallBackFunc
	WPARAM m_wParam;	//ExecuteCommand传入的参数，临时保存用于调用FireCallBackFunc
	CWnd* m_pCallWnd;	//ExecuteCommand传入的参数，临时保存用于调用FireCallBackFunc
// Construction
	BOOL m_bModelessDlg;	//用来表示当前对话框的类型,隐藏对话框时用来判断调用OnOK()还是ShowWindow(SW_HIDE)
	void PickObject();
	BOOL (*FireCallBackFunc)(CWnd* pCallWnd,UINT message,WPARAM wParam);
public:
	CCallBackDialog(UINT nIDTemplate,CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCallBackDialog)
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCallBackDialog)
	public:
	virtual int DoModal();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCallBackDialog)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	long m_iBreakType;			//中断类型 0.拾取构件 1.执行命令
	long m_iPickObjType;		//构件类型 1.添加碰撞参照杆件 2.重新选择碰撞参照杆件
	long m_hPickObj;
	BOOL m_bFireListItemChange;	//是否响应ListCtrl中ItemChange事件,一般在OnInitDialog中由于Insert触发的该事件不响应
	BOOL m_bInernalStart;		//内部重新启动对话框
	BOOL m_bPauseBreakExit;		//内部拾取对象时临时中断退出对话框
	DWORD m_dwSelectPartsType;	//拾取对象时的拾对象类型范围
	CString m_sCmdPickPrompt;	//拾取对象时命令行的提示字符串
	long m_idEventProp;			//记录触发选择事件的属性ID,恢复对话框时选中该项 wht 11-04-12
public:
	CBlockModel *m_pBlock;
	BOOL m_bOpenWndSel;			//开窗选择构件 wht 11-05-16
	void SelectObject(int cls_id=0,int idEventProp=0);	//启动选择对象操作
	void SetEventPropId(long idProp){m_idEventProp=idProp;}
	long GetEventPropId(){return m_idEventProp;}
	void SetPickObjTypeId(int cls_id) {m_iPickObjType=cls_id;}
	void SetCmdPickPromptStr(CString sPromptStr) {m_sCmdPickPrompt=sPromptStr;}
	BOOL ShowModelessDlg();
	virtual void FinishSelectObjOper(){;}
	void SetCallBackFunc(BOOL (*func)(CWnd* pCallWnd,UINT message,WPARAM wParam)){FireCallBackFunc = func;}
	void ExecuteCommand(CWnd* pCallWnd,UINT message=0,WPARAM wParam=0);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CALLBACKDIALOG_H__F07AFCF4_8860_4C20_A03A_E723AB08E4B6__INCLUDED_)
