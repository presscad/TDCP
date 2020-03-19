#pragma once


// TreePage 对话框
#include "XhTreeCtrl.h"
#include "TreeItemInfo.h"

class CGimPageDlg : public CDialogEx
{
	DECLARE_DYNCREATE(CGimPageDlg)
	CImageList m_images;
	ATOM_LIST<TREEITEM_INFO> m_xItemInfoList;
public:
	CString m_sOutPath;
	HTREEITEM m_hModSetItem,m_hGimSetItem;
public:
	CGimPageDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CGimPageDlg();
	//
	TREEITEM_INFO *GetSelItemInfo(HTREEITEM* pItem=NULL);
	void RefreshTreeCtrl();
	void ContextMenu(CWnd *pWnd, CPoint point);
	void AppendModFile(const char* sFilePath);
	void AppendGimFile(const char* sFilePath);
// 对话框数据
	enum { IDD = IDD_GIM_PAGE_DLG };
	CXhTreeCtrl m_treeCtrl;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTvnSelchangedTreeCtrl(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnKeydownTreeCtrl(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnRclickTreeCtrl(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMDblclkTreeCtrl(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnAddItem();
	afx_msg void OnDelItem();
	afx_msg void OnUpdateItem();
	afx_msg void OnPackData();
	afx_msg void OnTransData();
	afx_msg void OnParseData();
	afx_msg void OnUnpackData();
};
