
// MainFrm.h : CMainFrame 类的接口
//

#pragma once

#include "DialogPanel.h"
#include "PropertyDlg.h"
#include "EditPage.h"
#include "TMDHPage.h"

class CMainFrame : public CMDIFrameWndEx
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();

// 特性
public:
	//属性栏显示
	void DisplayGimProperty(CTMDHGim* pGim);
// 操作
public:
	BOOL CreateDockingWindows();
// 重写
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = NULL, CCreateContext* pContext = NULL);
	virtual void OnUpdateFrameTitle(BOOL bAddToTitle);
// 实现
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // 控件条嵌入成员
	CMFCMenuBar       m_wndMenuBar;
	CMFCToolBar       m_wndToolBar;
	CMFCStatusBar     m_wndStatusBar;
	CMFCToolBarImages m_UserImages;
	CMFCToolBar		  m_wndGLBar;
	CMFCToolBar		  m_wndSLBar;
	CDialogPanel	  m_PanelTMDH;
	CDialogPanel	  m_PanelProp;
	CDialogPanel	  m_PanelEdit;
	CMFCToolBarComboBoxButton *m_cmbButton;
// 生成的消息映射函数
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnWindowManager();
	afx_msg void OnViewCustomize();
	afx_msg LRESULT OnToolbarCreateNew(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnToolbarReset(WPARAM,LPARAM);
	afx_msg void OnApplicationLook(UINT id);
	afx_msg void OnUpdateApplicationLook(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
public:
	CPropertyDlg* GetPropPage(){return (CPropertyDlg*)m_PanelProp.GetDlgPtr();}
	CEditPageDlg* GetEditPage(){return (CEditPageDlg*)m_PanelEdit.GetDlgPtr();}
	CTMDHPageDlg* GetTMDHPage(){return (CTMDHPageDlg*)m_PanelTMDH.GetDlgPtr();}
};


