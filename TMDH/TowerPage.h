#pragma once


// TreePage 对话框
#include "XhTreeCtrl.h"
#include "TreeItemInfo.h"

class CTowerPageDlg : public CDialogEx
{
	DECLARE_DYNCREATE(CTowerPageDlg)
	HTREEITEM m_hTaSetItem;
	CImageList m_images;
	ATOM_LIST<TREEITEM_INFO> m_xItemInfoList;
public:
	CTowerPageDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CTowerPageDlg();
	//
	CTreeCtrl *GetTreeCtrl(){return &m_treeCtrl;}
	TREEITEM_INFO *GetSelItemInfo(HTREEITEM* pItem=NULL);
	void AppendLdsFile(const char* sFilePath);
// 对话框数据
	enum { IDD = IDD_TOWER_PAGE_DLG };
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
	afx_msg void OnExportTidFiles();
	afx_msg void OnExportModFiles();
	afx_msg void OnExport3dsFiles();
	afx_msg void OnExportStlFiles();
public:
	void RefreshTreeCtrl();
	void RefreshTowerItem(HTREEITEM hTowerItem);
	void ContextMenu(CWnd *pWnd, CPoint point);
};
