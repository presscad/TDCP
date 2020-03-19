#pragma once


// CModelPage 对话框
#include "XhTreeCtrl.h"
#include "TreeItemInfo.h"
#include "MenuButton.h"

class CTMDHPageDlg : public CDialogEx
{
	DECLARE_DYNCREATE(CTMDHPageDlg)
	RECT m_rcClient;
	CImageList m_images;
	ATOM_LIST<TREEITEM_INFO> m_xItemInfoList;
	HTREEITEM m_hTaSetItem,m_hModelSetItem,m_hModSetItem,m_hGimSetItem;
public:
	CTMDHPageDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CTMDHPageDlg();
	//
	CTreeCtrl *GetTreeCtrl(){return &m_treeCtrl;}
	TREEITEM_INFO *GetSelItemInfo(HTREEITEM* pItem=NULL);
	void ContextMenu(CWnd *pWnd, CPoint point);
	void RelayoutDlg();
	void RefreshTreeCtrl();
	void RefreshTowerItem(HTREEITEM hTowerItem);
	//
	void AppendLdsFile(const char* sFilePath);
	void AppendTidFile(const char* sFilePath);
	void AppendModFile(const char* sFilePath);
	void AppendGimFile(const char* sFilePath);
// 对话框数据
	enum { IDD = IDD_TMDH_PAGE_DLG };
	CXhTreeCtrl m_treeCtrl;
	CMenuButton m_xMenuBtn;
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
	afx_msg void OnExport3dsFiles();
	afx_msg void OnExportModFiles();
	afx_msg void OnExportStlFiles();
	afx_msg void OnExportXmlFiles();
	afx_msg void OnTransData();
	afx_msg void OnParseData();
	afx_msg void OnPackData();
	afx_msg void OnUnpackData();
};
