
// MainFrm.h : CMainFrame ��Ľӿ�
//

#pragma once
#include "DialogPanel.h"
#include "ProjectTreeDlg.h"
#include "PropertyDlg.h"
#include "WireDataDlg.h"
#include "CmdLineDlg.h"
#include "PromptDlg.h"

class CMainFrame : public CMDIFrameWndEx
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	//��ʾ��
	static CSysPromptObject prompt;
	class PROMPT_LIFE{
		bool m_bDisableAutoCloseOnUse;
		bool m_bAutoClose;
	public:
		PROMPT_LIFE(bool disableAutoCloseOnUse=false);
		~PROMPT_LIFE();
		void Prompt(const char* promptstr);
		void SetPromptPicture(long bitmapId);
		void SetPromptPicture(const char* bmpfile);
		void DisableAutoClose(){m_bAutoClose=false;}
	};
public:
	CMainFrame();
	virtual ~CMainFrame();
	// ����
	BOOL CreateDockingWindows();
	//��������ʾ
	void UpdatePropertyPage(CTPSMModel* pActiveModel =NULL);
	void DisplaySysProperty();
	void DisplayTowerParaProperty();
	// ��д
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = NULL, CCreateContext* pContext = NULL);
	virtual void OnUpdateFrameTitle(BOOL bAddToTitle);
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // �ؼ���Ƕ���Ա
	CMFCMenuBar       m_wndMenuBar;
	CMFCToolBar       m_wndToolBar;
	CMFCStatusBar     m_wndStatusBar;
	CMFCToolBarImages m_UserImages;
	CMFCToolBar		  m_wndGLBar;
	CDialogPanel	  m_PanelProperty;
	CDialogPanel	  m_PanelWireData;
	CDialogPanel	  m_PanelProjectTree;
	CDialogPanel	  m_PanelCmdLine;
// ���ɵ���Ϣӳ�亯��
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnWindowManager();
	afx_msg void OnViewCustomize();
	afx_msg LRESULT OnToolbarCreateNew(WPARAM wp, LPARAM lp);
	afx_msg void OnApplicationLook(UINT id);
	afx_msg void OnUpdateApplicationLook(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
public:
	CPropertyDlg* GetPropertyPage(){return (CPropertyDlg*)m_PanelProperty.GetDlgPtr();}
	CWireDataDlg* GetWireDataPage(){return (CWireDataDlg*)m_PanelWireData.GetDlgPtr();}
	CProjectTreeDlg* GetTowerTreePage(){return (CProjectTreeDlg*)m_PanelProjectTree.GetDlgPtr();}
	CCmdLineDlg* GetCmdLinePage(){return (CCmdLineDlg*)m_PanelCmdLine.GetDlgPtr();}
};


