// TowerTreeDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "TMDH.h"
#include "TowerPage.h"
#include "afxdialogex.h"
#include "XhCharString.h"
#include "ProcBarDlg.h"
#include "InstanceDlg.h"
#include "GlobalFunc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////////
// CTowerTreeDlg 对话框
IMPLEMENT_DYNCREATE(CTowerPageDlg, CDialogEx)

CTowerPageDlg::CTowerPageDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CTowerPageDlg::IDD, pParent)
{

}

CTowerPageDlg::~CTowerPageDlg()
{
}

void CTowerPageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE_CTRL, m_treeCtrl);
}


BEGIN_MESSAGE_MAP(CTowerPageDlg, CDialogEx)
	ON_WM_SIZE()
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_CTRL, OnTvnSelchangedTreeCtrl)
	ON_NOTIFY(TVN_KEYDOWN, IDC_TREE_CTRL, OnTvnKeydownTreeCtrl)
	ON_NOTIFY(NM_RCLICK, IDC_TREE_CTRL, OnRclickTreeCtrl)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE_CTRL, OnNMDblclkTreeCtrl)
	ON_COMMAND(ID_ADD_ITEM,OnAddItem)
	ON_COMMAND(ID_DEL_ITEM,OnDelItem)
	ON_COMMAND(ID_UPDATE_ITEM,OnUpdateItem)
	ON_COMMAND(ID_EXPORT_TID,OnExportTidFiles)
	ON_COMMAND(ID_EXPORT_MOD,OnExportModFiles)
	ON_COMMAND(ID_EXPORT_3DS,OnExport3dsFiles)
	ON_COMMAND(ID_EXPORT_STL,OnExportStlFiles)
END_MESSAGE_MAP()


// CTowerTreeDlg 消息处理程序
BOOL CTowerPageDlg::OnInitDialog() 
{
	CDialogEx::OnInitDialog();
	//
	m_images.Create(IDB_IL_PROJECT, 16, 1, RGB(0,255,0));
	m_treeCtrl.SetImageList(&m_images,TVSIL_NORMAL);
	m_treeCtrl.ModifyStyle(0,TVS_HASLINES|TVS_HASBUTTONS|TVS_SHOWSELALWAYS|TVS_FULLROWSELECT);
	RefreshTreeCtrl();
	return TRUE;  
}
void CTowerPageDlg::OnOK() 
{
}

void CTowerPageDlg::OnCancel() 
{
}

void CTowerPageDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialogEx::OnSize(nType, cx, cy);
	//
	if(m_treeCtrl.GetSafeHwnd())
		m_treeCtrl.MoveWindow(0,0,cx,cy);
}
void CTowerPageDlg::RefreshTreeCtrl()
{
	m_xItemInfoList.Empty();
	m_treeCtrl.DeleteAllItems();
	TREEITEM_INFO *pItemInfo=NULL;
	//杆塔数据层级
	m_hTaSetItem=m_treeCtrl.InsertItem("杆塔数据",IMG_TOWER_ROOT,IMG_TOWER_ROOT,TVI_ROOT);
	pItemInfo=m_xItemInfoList.append(TREEITEM_INFO(TREEITEM_INFO::INFO_TA_SET,NULL));
	m_treeCtrl.SetItemData(m_hTaSetItem,(DWORD)pItemInfo);
	m_treeCtrl.Expand(m_hTaSetItem,TVE_EXPAND);
}
void CTowerPageDlg::RefreshTowerItem(HTREEITEM hTowerItem)
{
	TREEITEM_INFO* pItemInfo=(TREEITEM_INFO*)m_treeCtrl.GetItemData(hTowerItem);
	if(pItemInfo==NULL||pItemInfo->itemType!=TREEITEM_INFO::INFO_TOWER||pItemInfo->dwRefData==0)
		return;
	CTMDHTower* pTower=(CTMDHTower*)pItemInfo->dwRefData;
	CLDSModule* pActiveModule=pTower->m_xTower.GetActiveModule();
	if(pActiveModule==NULL)
		return;
	m_treeCtrl.DeleteAllSonItems(hTowerItem);
	//呼高
	CString ss,ss_leg[4];
	ss.Format("当前呼高:%s",pActiveModule->description);
	HTREEITEM hItem=m_treeCtrl.InsertItem(ss,IMG_TOWER_ITEM,IMG_TOWER_ITEM,hTowerItem);
	pItemInfo=m_xItemInfoList.append(TREEITEM_INFO(TREEITEM_INFO::INFO_HUGAO,(DWORD)pActiveModule));
	m_treeCtrl.SetItemData(hItem,(DWORD)pItemInfo);
	//配腿
	for(int i=1;i<=192;i++)
	{
		if(!pActiveModule->m_dwLegCfgWord.IsHasNo(i))
			continue;
		if(pActiveModule->m_arrActiveQuadLegNo[0]==i)
			ss_leg[3].Format("%C",(i-1)%pTower->m_nMaxLegs+'A');
		if(pActiveModule->m_arrActiveQuadLegNo[1]==i)
			ss_leg[0].Format("%C",(i-1)%pTower->m_nMaxLegs+'A');
		if(pActiveModule->m_arrActiveQuadLegNo[2]==i)
			ss_leg[2].Format("%C",(i-1)%pTower->m_nMaxLegs+'A');
		if(pActiveModule->m_arrActiveQuadLegNo[3]==i)
			ss_leg[1].Format("%C",(i-1)%pTower->m_nMaxLegs+'A');
	}
	for(int iQuad=0;iQuad<4;iQuad++)
	{
		ss.Format("%d号接腿:%s",iQuad+1,ss_leg[iQuad]);
		hItem=m_treeCtrl.InsertItem(ss,IMG_LEG_ITEM,IMG_LEG_ITEM,hTowerItem);
		if(iQuad==0)
			pItemInfo=m_xItemInfoList.append(TREEITEM_INFO(TREEITEM_INFO::INFO_LEG1,0));
		else if(iQuad==1)
			pItemInfo=m_xItemInfoList.append(TREEITEM_INFO(TREEITEM_INFO::INFO_LEG2,0));
		else if(iQuad==2)
			pItemInfo=m_xItemInfoList.append(TREEITEM_INFO(TREEITEM_INFO::INFO_LEG3,0));
		else if(iQuad==3)
			pItemInfo=m_xItemInfoList.append(TREEITEM_INFO(TREEITEM_INFO::INFO_LEG4,0));
		m_treeCtrl.SetItemData(hItem,(DWORD)pItemInfo);
	}
	m_treeCtrl.Expand(hTowerItem,TVE_EXPAND);
}
TREEITEM_INFO *CTowerPageDlg::GetSelItemInfo(HTREEITEM* pItem/*=NULL*/)
{
	HTREEITEM hSelItem=m_treeCtrl.GetSelectedItem();
	if(hSelItem==NULL)
		return NULL;
	TREEITEM_INFO *pItemInfo=(TREEITEM_INFO*)m_treeCtrl.GetItemData(hSelItem);
	if(pItem)
		*pItem=hSelItem;
	return pItemInfo;
}

void CTowerPageDlg::ContextMenu(CWnd *pWnd, CPoint point)
{
	TREEITEM_INFO *pItemInfo=GetSelItemInfo();
	if(pItemInfo==NULL)
		return;
	CPoint scr_point = point;
	m_treeCtrl.ClientToScreen(&scr_point);
	CMenu menu;
	menu.CreatePopupMenu();
	if(pItemInfo->itemType==TREEITEM_INFO::INFO_TA_SET)
	{
		menu.AppendMenu(MF_STRING,ID_ADD_ITEM,"添加塔型");
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING,ID_EXPORT_TID,"生成TID文件");
		menu.AppendMenu(MF_STRING,ID_EXPORT_MOD,"生成MOD文件");
		menu.AppendMenu(MF_STRING,ID_EXPORT_3DS,"生成3DS文件");
		menu.AppendMenu(MF_STRING,ID_EXPORT_STL,"生成STL文件");
	}
	else if(pItemInfo->itemType==TREEITEM_INFO::INFO_TOWER)
	{
		menu.AppendMenu(MF_STRING,ID_UPDATE_ITEM,"配基属性");
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING,ID_DEL_ITEM,"删除塔型");
	}
	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, scr_point.x, scr_point.y, this, NULL);
	menu.DestroyMenu();
}

void CTowerPageDlg::AppendLdsFile(const char* sFilePath)
{
	CXhChar16 file_name;
	_splitpath(sFilePath,NULL,NULL,file_name,NULL);
	//
	CTMDHTower* pTa=Manager.AppendTA();
	pTa->m_sPath.Copy(sFilePath);
	pTa->m_sTower.Copy(file_name);
	pTa->m_xTower.DisplayProcess=DisplayProcess;
	pTa->ReadLdsFile(sFilePath);
	pTa->m_nMaxLegs=CFGLEG::MaxLegs();
	//插入新的树节点
	m_treeCtrl.SetItemText(m_hTaSetItem,CXhChar50("杆塔数据(%d)",Manager.TowerNum()));
	HTREEITEM hItem=m_treeCtrl.InsertItem(pTa->m_sTower,IMG_TOWER_ITEM,IMG_TOWER_ITEM,m_hTaSetItem);
	TREEITEM_INFO *pSonItemInfo=m_xItemInfoList.append(TREEITEM_INFO(TREEITEM_INFO::INFO_TOWER,(DWORD)pTa));
	m_treeCtrl.SetItemData(hItem,(DWORD)pSonItemInfo);
	m_treeCtrl.Expand(m_hTaSetItem,TVE_EXPAND);
	m_treeCtrl.SelectItem(hItem);
	RefreshTowerItem(hItem);
	//刷新塔型实体
	theApp.SetActiveDoc(pTa);
}
//////////////////////////////////////////////////////////////////////////
void CTowerPageDlg::OnRclickTreeCtrl(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TVHITTESTINFO HitTestInfo;
	GetCursorPos(&HitTestInfo.pt);
	CTreeCtrl *pTreeCtrl = GetTreeCtrl();
	pTreeCtrl->ScreenToClient(&HitTestInfo.pt);
	pTreeCtrl->HitTest(&HitTestInfo);
	pTreeCtrl->Select(HitTestInfo.hItem,TVGN_CARET);
	//
	ContextMenu(this,HitTestInfo.pt);
	*pResult = 0;
}

void CTowerPageDlg::OnNMDblclkTreeCtrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	TREEITEM_INFO *pInfo=GetSelItemInfo();
	if(pInfo!=NULL && pInfo->itemType==TREEITEM_INFO::INFO_TOWER)
	{
		CTMDHTower* pTower=(CTMDHTower*)pInfo->dwRefData;
		theApp.SetActiveDoc(pTower);
	}
	*pResult = 0;
}

void CTowerPageDlg::OnTvnSelchangedTreeCtrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	HTREEITEM hItem = m_treeCtrl.GetSelectedItem();
	TREEITEM_INFO *pInfo=(TREEITEM_INFO*)m_treeCtrl.GetItemData(hItem);
	
	*pResult = 0;
}

void CTowerPageDlg::OnTvnKeydownTreeCtrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTVKEYDOWN pTVKeyDown = reinterpret_cast<LPNMTVKEYDOWN>(pNMHDR);
	if(pTVKeyDown->wVKey==VK_DELETE)
		OnDelItem();
	*pResult = 0;
}

void CTowerPageDlg::OnAddItem()
{
	HTREEITEM hSelItem=NULL;
	TREEITEM_INFO *pItemInfo=GetSelItemInfo(&hSelItem);
	if(pItemInfo==NULL)
		return;
	if(pItemInfo->itemType==TREEITEM_INFO::INFO_TA_SET)
	{	//杆塔集合
		CXhChar500 filter("塔型文件(*.lds)|*.lds");
		filter.Append("|所有文件(*.*)|*.*||");
		CFileDialog dlg(TRUE,"塔型文件","塔型文件",OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,filter);
		if(dlg.DoModal()!=IDOK)
			return;
		CTMDHTower* pTa=Manager.AppendTA();
		pTa->m_sPath.Copy(dlg.GetPathName().GetString());
		pTa->m_sTower.Copy(dlg.GetFileTitle());
		pTa->m_xTower.DisplayProcess=DisplayProcess;
		pTa->ReadLdsFile(dlg.GetPathName());
		pTa->m_nMaxLegs=CFGLEG::MaxLegs();
		//插入新的树节点
		m_treeCtrl.SetItemText(hSelItem,CXhChar50("杆塔数据(%d)",Manager.TowerNum()));
		HTREEITEM hItem=m_treeCtrl.InsertItem(pTa->m_sTower,IMG_TOWER_ITEM,IMG_TOWER_ITEM,hSelItem);
		TREEITEM_INFO *pSonItemInfo=m_xItemInfoList.append(TREEITEM_INFO(TREEITEM_INFO::INFO_TOWER,(DWORD)pTa));
		m_treeCtrl.SetItemData(hItem,(DWORD)pSonItemInfo);
		m_treeCtrl.Expand(hSelItem,TVE_EXPAND);
		m_treeCtrl.SelectItem(hItem);
		//
		RefreshTowerItem(hItem);
		//刷新塔型实体
		theApp.SetActiveDoc(pTa);
	}
}

void CTowerPageDlg::OnUpdateItem()
{
	HTREEITEM hSelItem=NULL;
	TREEITEM_INFO *pItemInfo=GetSelItemInfo(&hSelItem);
	if(pItemInfo==NULL)
		return;
	if(pItemInfo->itemType==TREEITEM_INFO::INFO_TOWER)
	{
		CTMDHTower* pTower=(CTMDHTower*)pItemInfo->dwRefData;
		CInstanceDlg dlg;
		dlg.m_ciModelFlag=0;
		dlg.m_pModel=pTower;
		if(dlg.DoModal()==IDOK)
			RefreshTowerItem(hSelItem);
		CTMDHView* pView=theApp.GetActiveView();
		if(pView)
			pView->Refresh();
	}
}

void CTowerPageDlg::OnDelItem()
{
	HTREEITEM hSelItem=NULL;
	TREEITEM_INFO *pItemInfo=GetSelItemInfo(&hSelItem);
	if(pItemInfo==NULL)
		return;
	if(pItemInfo->itemType==TREEITEM_INFO::INFO_TOWER)
	{
		CTMDHTower* pTower=(CTMDHTower*)pItemInfo->dwRefData;
		if(pTower)
			Manager.DeleteTa(pTower);
		m_treeCtrl.DeleteItem(hSelItem);
		//
		theApp.DeleteDoc(pTower);
	}
}
//导出指定文件
void CTowerPageDlg::OnExportTidFiles()
{
	Manager.ExportTowerTidFiles();
}
void CTowerPageDlg::OnExportModFiles()
{
	Manager.ExportTowerModFiles();
}

void CTowerPageDlg::OnExport3dsFiles()
{
	Manager.ExportTower3dsFiles();
}

void CTowerPageDlg::OnExportStlFiles()
{
	Manager.ExportTowerStlFiles();
}
