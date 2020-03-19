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
	UINT   m_message;	//ExecuteCommand����Ĳ�������ʱ�������ڵ���FireCallBackFunc
	WPARAM m_wParam;	//ExecuteCommand����Ĳ�������ʱ�������ڵ���FireCallBackFunc
	CWnd* m_pCallWnd;	//ExecuteCommand����Ĳ�������ʱ�������ڵ���FireCallBackFunc
// Construction
	BOOL m_bModelessDlg;	//������ʾ��ǰ�Ի��������,���ضԻ���ʱ�����жϵ���OnOK()����ShowWindow(SW_HIDE)
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
	long m_iBreakType;			//�ж����� 0.ʰȡ���� 1.ִ������
	long m_iPickObjType;		//�������� 1.�����ײ���ո˼� 2.����ѡ����ײ���ո˼�
	long m_hPickObj;
	BOOL m_bFireListItemChange;	//�Ƿ���ӦListCtrl��ItemChange�¼�,һ����OnInitDialog������Insert�����ĸ��¼�����Ӧ
	BOOL m_bInernalStart;		//�ڲ����������Ի���
	BOOL m_bPauseBreakExit;		//�ڲ�ʰȡ����ʱ��ʱ�ж��˳��Ի���
	DWORD m_dwSelectPartsType;	//ʰȡ����ʱ��ʰ�������ͷ�Χ
	CString m_sCmdPickPrompt;	//ʰȡ����ʱ�����е���ʾ�ַ���
	long m_idEventProp;			//��¼����ѡ���¼�������ID,�ָ��Ի���ʱѡ�и��� wht 11-04-12
public:
	CBlockModel *m_pBlock;
	BOOL m_bOpenWndSel;			//����ѡ�񹹼� wht 11-05-16
	void SelectObject(int cls_id=0,int idEventProp=0);	//����ѡ��������
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
