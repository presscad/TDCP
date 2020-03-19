#pragma once


// CTowerTreeDlg 对话框
#include "XhTreeCtrl.h"

enum PROJECT_IMAGE{
	IMG_PRJ_ROOT,
	IMG_TOWERMODEL_SET,
	IMG_TAINSTANCE_SET,
	IMG_TOWER_ITEM,
};
struct TREEITEM_INFO{
	DWORD dwRefData;
	int itemType;
	//
	static const int INFO_PROJECT_SET		= 1;
	static const int INFO_TOWERMODEL_SET	= 2;
	static const int INFO_TAINSTANCE_SET	= 3;
	static const int INFO_TOWER_ITEM		= 4;
public:
	TREEITEM_INFO(){;}
	TREEITEM_INFO(int type,DWORD dw){itemType=type;dwRefData=dw;}
};

class CProjectTreeDlg : public CDialogEx
{
	DECLARE_DYNCREATE(CProjectTreeDlg)
	HTREEITEM m_hPrjTowerModel,m_hPrjInstance;
	CImageList m_images;
	ATOM_LIST<TREEITEM_INFO> m_xItemInfoList;
public:
	CProjectTreeDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CProjectTreeDlg();
	//
	CTreeCtrl *GetTreeCtrl(){return &m_treeCtrl;}
	TREEITEM_INFO *GetSelItemInfo(HTREEITEM* pItem=NULL);
// 对话框数据
	enum { IDD = IDD_PROJECT_TREE_DLG };
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
	afx_msg void OnNewItem();
	afx_msg void OnDelItem();
	afx_msg void OnUpdateItem();
	afx_msg void OnExportTidFiles();
	afx_msg void OnExportModFiles();
	afx_msg void OnExportDxfFiles();
	afx_msg void OnExport3dsFiles();
	afx_msg void OnExportLdsFiles();
	afx_msg void OnPlanTrunkSegment();
public:
	void InitTreeView();
	void RefreshPrjTree(HTREEITEM hPrjItem);
	void ContextMenu(CWnd *pWnd, CPoint point);
	//
	void NewTowerModel(HTREEITEM hItem);
};
