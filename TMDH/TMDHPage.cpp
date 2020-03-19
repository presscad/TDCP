// ModelPage.cpp : 实现文件
//

#include "stdafx.h"
#include "TMDH.h"
#include "TMDHPage.h"
#include "afxdialogex.h"
#include "ProcBarDlg.h"
#include "InstanceDlg.h"
#include "GlobalFunc.h"
#include "folder_dialog.h"
//////////////////////////////////////////////////////////////////////////
static DWORD TrackLiveComboMenu(CWnd* pParentWnd, CPoint point)
{
	CTMDHPageDlg *pDlg=(CTMDHPageDlg*)pParentWnd;
	if(pDlg==NULL)
		return FALSE;
	pDlg->GetTreeCtrl()->ScreenToClient(&point);
	pDlg->ContextMenu(pDlg->GetTreeCtrl(),point);
	return TRUE;
}
//////////////////////////////////////////////////////////////////////////
// CModelPage 对话框
IMPLEMENT_DYNCREATE(CTMDHPageDlg, CDialogEx)

CTMDHPageDlg::CTMDHPageDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CTMDHPageDlg::IDD, pParent)
{
}

CTMDHPageDlg::~CTMDHPageDlg()
{
}

void CTMDHPageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE_CTRL, m_treeCtrl);
	DDX_Control(pDX, IDC_MENU_BUTTON, m_xMenuBtn);
}


BEGIN_MESSAGE_MAP(CTMDHPageDlg, CDialogEx)
	ON_WM_SIZE()
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_CTRL, OnTvnSelchangedTreeCtrl)
	ON_NOTIFY(TVN_KEYDOWN, IDC_TREE_CTRL, OnTvnKeydownTreeCtrl)
	ON_NOTIFY(NM_RCLICK, IDC_TREE_CTRL, OnRclickTreeCtrl)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE_CTRL, OnNMDblclkTreeCtrl)
	ON_COMMAND(ID_ADD_ITEM,OnAddItem)
	ON_COMMAND(ID_DEL_ITEM,OnDelItem)
	ON_COMMAND(ID_UPDATE_ITEM,OnUpdateItem)
	ON_COMMAND(ID_EXPORT_TID,OnExportTidFiles)
	ON_COMMAND(ID_EXPORT_3DS,OnExport3dsFiles)
	ON_COMMAND(ID_EXPORT_MOD,OnExportModFiles)
	ON_COMMAND(ID_EXPORT_STL,OnExportStlFiles)
	ON_COMMAND(ID_EXPORT_XML,OnExportXmlFiles)
	ON_COMMAND(ID_TRANSFORM_DATA,OnTransData)
	ON_COMMAND(ID_PARSE_DATA,OnParseData)
	ON_COMMAND(ID_PACK_DATA,OnPackData)
	ON_COMMAND(ID_UNPACK_DATA,OnUnpackData)
END_MESSAGE_MAP()


// CModelPage 消息处理程序
BOOL CTMDHPageDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	//
	m_images.Create(IDB_IL_PROJECT, 16, 1, RGB(0,255,0));
	m_treeCtrl.SetImageList(&m_images,TVSIL_NORMAL);
	m_treeCtrl.ModifyStyle(0,TVS_HASLINES|TVS_HASBUTTONS|TVS_SHOWSELALWAYS|TVS_FULLROWSELECT);
	RefreshTreeCtrl();
	m_xMenuBtn.FireClickComboMenu=TrackLiveComboMenu;
	//
	UpdateData(FALSE);
	return TRUE;
}
void CTMDHPageDlg::OnOK()
{

}
void CTMDHPageDlg::OnCancel()
{

}

void CTMDHPageDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialogEx::OnSize(nType, cx, cy);
	//
	m_rcClient.bottom = cy;
	m_rcClient.right = cx;
	RelayoutDlg();
}

void CTMDHPageDlg::RelayoutDlg()
{
	RECT rcMenu,rcBtm;
	CWnd* pMenuWnd=GetDlgItem(IDC_MENU_BUTTON);
	if(pMenuWnd)
		pMenuWnd->GetWindowRect(&rcMenu);
	ScreenToClient(&rcMenu);
	int nMenuH=rcMenu.bottom-rcMenu.top;
	int nMenuL=rcMenu.right-rcMenu.left;
	rcMenu.left=5;
	rcMenu.right=rcMenu.left+nMenuL;
	rcMenu.top=2;
	rcMenu.bottom=rcMenu.top+nMenuH;
	rcBtm.top=rcMenu.bottom+5;
	rcBtm.bottom=m_rcClient.bottom;
	rcBtm.left=0;
	rcBtm.right=m_rcClient.right;
	if(pMenuWnd->GetSafeHwnd())
		pMenuWnd->MoveWindow(&rcMenu);
	if(m_treeCtrl.GetSafeHwnd())
		m_treeCtrl.MoveWindow(&rcBtm);
}

void CTMDHPageDlg::RefreshTreeCtrl()
{
	m_xItemInfoList.Empty();
	m_treeCtrl.DeleteAllItems();
	TREEITEM_INFO *pItemInfo=NULL;
	//
	HTREEITEM xRootItem=m_treeCtrl.InsertItem("操作模型",IMG_TOWER_ROOT,IMG_TOWER_ROOT,TVI_ROOT);
	pItemInfo=m_xItemInfoList.append(TREEITEM_INFO(TREEITEM_INFO::INFO_ROOT,NULL));
	m_treeCtrl.SetItemData(xRootItem,(DWORD)pItemInfo);
	//LDS数据层级
	m_hTaSetItem=m_treeCtrl.InsertItem("LDS杆塔数据",IMG_TOWER_ITEM,IMG_TOWER_ITEM,xRootItem);
	pItemInfo=m_xItemInfoList.append(TREEITEM_INFO(TREEITEM_INFO::INFO_TA_SET,NULL));
	m_treeCtrl.SetItemData(m_hTaSetItem,(DWORD)pItemInfo);
	//MOD数据层级
	m_hModSetItem=m_treeCtrl.InsertItem("MOD打包数据",IMG_MOD_ITEM,IMG_MOD_ITEM,xRootItem);
	pItemInfo=m_xItemInfoList.append(TREEITEM_INFO(TREEITEM_INFO::INFO_MOD_SET,NULL));
	m_treeCtrl.SetItemData(m_hModSetItem,(DWORD)pItemInfo);
	//GIM数据层级
	m_hGimSetItem=m_treeCtrl.InsertItem("GIM移交数据",IMG_GIM_ITEM,IMG_GIM_ITEM,xRootItem);
	pItemInfo=m_xItemInfoList.append(TREEITEM_INFO(TREEITEM_INFO::INFO_GIM_SET,NULL));
	m_treeCtrl.SetItemData(m_hGimSetItem,(DWORD)pItemInfo);
	//批量处理数据层级
	m_hModelSetItem=m_treeCtrl.InsertItem("批量处理数据",IMG_MODEL_ITEM,IMG_MODEL_ITEM,xRootItem);
	pItemInfo=m_xItemInfoList.append(TREEITEM_INFO(TREEITEM_INFO::INFO_MODEL_SET,NULL));
	m_treeCtrl.SetItemData(m_hModelSetItem,(DWORD)pItemInfo);
	//
	m_treeCtrl.Expand(xRootItem,TVE_EXPAND);
}

void CTMDHPageDlg::RefreshTowerItem(HTREEITEM hTowerItem)
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

TREEITEM_INFO *CTMDHPageDlg::GetSelItemInfo(HTREEITEM* pItem/*=NULL*/)
{
	HTREEITEM hSelItem=m_treeCtrl.GetSelectedItem();
	if(hSelItem==NULL)
		return NULL;
	TREEITEM_INFO *pItemInfo=(TREEITEM_INFO*)m_treeCtrl.GetItemData(hSelItem);
	if(pItem)
		*pItem=hSelItem;
	return pItemInfo;
}

void CTMDHPageDlg::ContextMenu(CWnd *pWnd, CPoint point)
{
	TREEITEM_INFO *pItemInfo=GetSelItemInfo();
	if(pItemInfo==NULL)
		return;
	CMenu menu;
	menu.CreatePopupMenu();
	if(pItemInfo->itemType==TREEITEM_INFO::INFO_TA_SET)
	{
		menu.AppendMenu(MF_STRING,ID_ADD_ITEM,"添加塔型数据");
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
		menu.AppendMenu(MF_STRING,ID_DEL_ITEM,"删除");
	}
	else if(pItemInfo->itemType==TREEITEM_INFO::INFO_MODEL_SET)
	{
		menu.AppendMenu(MF_STRING,ID_ADD_ITEM,"添加批量数据");
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING,ID_EXPORT_3DS,"生成3DS文件");
		menu.AppendMenu(MF_STRING,ID_EXPORT_MOD,"生成MOD文件");
		menu.AppendMenu(MF_STRING,ID_EXPORT_XML,"生成XML文件");
	}
	else if(pItemInfo->itemType==TREEITEM_INFO::INFO_MODEL)
	{
		menu.AppendMenu(MF_STRING,ID_UPDATE_ITEM,"配基属性");
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING,ID_DEL_ITEM,"删除");
	}
	else if(pItemInfo->itemType==TREEITEM_INFO::INFO_MOD_SET)
	{
		menu.AppendMenu(MF_STRING,ID_ADD_ITEM,"添加MOD数据");
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING,ID_UPDATE_ITEM,"数据检测");
		//menu.AppendMenu(MF_STRING,ID_TRANSFORM_DATA,"数据转换");
		menu.AppendMenu(MF_STRING,ID_PACK_DATA,"批量打包");
	}
	else if(pItemInfo->itemType==TREEITEM_INFO::INFO_MOD)
	{
		menu.AppendMenu(MF_STRING,ID_PACK_DATA,"打包");
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING,ID_DEL_ITEM,"删除");
	}
	else if(pItemInfo->itemType==TREEITEM_INFO::INFO_GIM_SET)
	{
		menu.AppendMenu(MF_STRING,ID_ADD_ITEM,"添加GIM数据");
	}
	else if(pItemInfo->itemType==TREEITEM_INFO::INFO_PRJ_GIM)
	{
		menu.AppendMenu(MF_STRING,ID_UNPACK_DATA,"解压到..");
		menu.AppendMenu(MF_STRING,ID_PARSE_DATA,"数据解析");
		//menu.AppendMenu(MF_STRING,ID_UPDATE_ITEM,"更新GIM包");
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING,ID_DEL_ITEM,"删除");
	}
	else if(pItemInfo->itemType==TREEITEM_INFO::INFO_DEV_GIM)
	{
		menu.AppendMenu(MF_STRING,ID_UPDATE_ITEM,"更新GIM包");
	}
	CPoint scr_point = point;
	m_treeCtrl.ClientToScreen(&scr_point);
	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, scr_point.x, scr_point.y, this, NULL);
	menu.DestroyMenu();
}
void CTMDHPageDlg::AppendLdsFile(const char* sFilePath)
{
	CXhChar200 file_name;
	_splitpath(sFilePath,NULL,NULL,file_name,NULL);
	//
	CTMDHTower* pTa=Manager.AppendTA();
	pTa->m_sPath.Copy(sFilePath);
	pTa->m_sTower.Copy(file_name);
	pTa->m_xTower.DisplayProcess=DisplayProcess;
	pTa->ReadLdsFile(sFilePath);
	pTa->m_nMaxLegs=CFGLEG::MaxLegs();
	//插入新的树节点
	m_treeCtrl.SetItemText(m_hTaSetItem,CXhChar50("杆塔模型数据(%d)",Manager.TowerNum()));
	HTREEITEM hItem=m_treeCtrl.InsertItem(pTa->m_sTower,IMG_TOWER_ITEM,IMG_TOWER_ITEM,m_hTaSetItem);
	TREEITEM_INFO *pSonItemInfo=m_xItemInfoList.append(TREEITEM_INFO(TREEITEM_INFO::INFO_TOWER,(DWORD)pTa));
	m_treeCtrl.SetItemData(hItem,(DWORD)pSonItemInfo);
	m_treeCtrl.Expand(m_hTaSetItem,TVE_EXPAND);
	m_treeCtrl.SelectItem(hItem);
	RefreshTowerItem(hItem);
	//刷新塔型实体
	theApp.SetActiveDoc(pTa);
}
void CTMDHPageDlg::AppendTidFile(const char* sFilePath)
{
	CXhChar200 file_name;
	_splitpath(sFilePath,NULL,NULL,file_name,NULL);
	//
	CTMDHModel* pModel=Manager.AppendModel();
	pModel->m_sTowerType=file_name;
	pModel->m_ciModelFlag=CTMDHModel::TYPE_TID;
	pModel->m_sFilePath.Copy(sFilePath);
	if(Manager.m_pTidModel==NULL)
		Manager.m_pTidModel=CTidModelFactory::CreateTidModel();
	//插入新的树节点
	HTREEITEM hItem=m_treeCtrl.InsertItem(pModel->m_sTowerType,IMG_MODEL_ITEM,IMG_MODEL_ITEM,m_hModelSetItem);
	TREEITEM_INFO *pSonItemInfo=m_xItemInfoList.append(TREEITEM_INFO(TREEITEM_INFO::INFO_MODEL,(DWORD)pModel));
	m_treeCtrl.SetItemData(hItem,(DWORD)pSonItemInfo);
	m_treeCtrl.SetItemText(m_hModelSetItem,CXhChar50("批量处理数据(%d)",Manager.ModelNum()));
	m_treeCtrl.Expand(m_hModelSetItem,TVE_EXPAND);
	m_treeCtrl.SelectItem(hItem);
	//
	theApp.SetActiveDoc(pModel,CTMDHDoc::TMD_DATA);
}
void CTMDHPageDlg::AppendModFile(const char* sFilePath)
{
	char fname[MAX_PATH];
	_splitpath(sFilePath,NULL,NULL,fname,NULL);
	CString folder(sFilePath);
	int index = folder.ReverseFind('\\');//反向查找'\\'
	folder = folder.Left(index);		//移除文件名
	//
	CTMDHGim* pGim=Manager.AppendGim();
	pGim->m_sModName.Copy(fname);
	pGim->m_sGimName.Copy(fname);
	pGim->m_sModPath.Copy(folder);
	pGim->m_sOutputPath.Copy(folder);
	pGim->m_sModFile.Copy(sFilePath);
	if(IsUTF8File(sFilePath))
		pGim->m_ciCodingType=CTMDHGim::UTF8_CODING;
	else
		pGim->m_ciCodingType=CTMDHGim::ANSI_CODING;
	if(Manager.m_pModModel==NULL)
		Manager.m_pModModel=CModModelFactory::CreateModModel();
	//插入新的树节点
	HTREEITEM hItem=m_treeCtrl.InsertItem(pGim->m_sModName,IMG_MOD_ITEM,IMG_MOD_ITEM,m_hModSetItem);
	TREEITEM_INFO *pSonItemInfo=m_xItemInfoList.append(TREEITEM_INFO(TREEITEM_INFO::INFO_MOD,(DWORD)pGim));
	m_treeCtrl.SetItemData(hItem,(DWORD)pSonItemInfo);
	m_treeCtrl.SetItemText(m_hModSetItem,CXhChar50("MOD打包数据(%d)",Manager.GimNum()));
	m_treeCtrl.Expand(m_hModSetItem,TVE_EXPAND);
	m_treeCtrl.SelectItem(hItem);
	//
	theApp.SetActiveDoc(pGim,CTMDHDoc::GIM_DATA);
}
void CTMDHPageDlg::AppendGimFile(const char* sFilePath)
{
	char fname[MAX_PATH];
	_splitpath(sFilePath,NULL,NULL,fname,NULL);
	CString folder(sFilePath);
	int index = folder.ReverseFind('\\');//反向查找'\\'
	folder = folder.Left(index);		//移除文件名
	//
	CTMDHPrjGim* pPrjGim=Manager.AppendPrjGim();
	pPrjGim->m_sGimName=fname;
	pPrjGim->m_sGimPath=folder;
	//插入新的树节点
	HTREEITEM hItem=m_treeCtrl.InsertItem(pPrjGim->m_sGimName,IMG_GIM_ITEM,IMG_GIM_ITEM,m_hGimSetItem);
	TREEITEM_INFO *pSonItemInfo=m_xItemInfoList.append(TREEITEM_INFO(TREEITEM_INFO::INFO_PRJ_GIM,(DWORD)pPrjGim));
	m_treeCtrl.SetItemData(hItem,(DWORD)pSonItemInfo);
	m_treeCtrl.SetItemText(m_hGimSetItem,CXhChar50("GIM移交数据(%d)",Manager.PrjGimNum()));
	m_treeCtrl.Expand(m_hGimSetItem,TVE_EXPAND);
	m_treeCtrl.SelectItem(hItem);
}
//////////////////////////////////////////////////////////////////////////
void CTMDHPageDlg::OnRclickTreeCtrl(NMHDR* pNMHDR, LRESULT* pResult) 
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

void CTMDHPageDlg::OnNMDblclkTreeCtrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	TREEITEM_INFO *pInfo=GetSelItemInfo();
	if(pInfo==NULL)
		return ;
	if(pInfo->itemType==TREEITEM_INFO::INFO_TOWER)
	{
		CTMDHTower* pTower=(CTMDHTower*)pInfo->dwRefData;
		theApp.SetActiveDoc(pTower);
	}
	else if(pInfo->itemType==TREEITEM_INFO::INFO_MODEL)
	{
		CTMDHModel* pModel=(CTMDHModel*)pInfo->dwRefData;
		theApp.SetActiveDoc(pModel,CTMDHDoc::TMD_DATA);
	}
	else if(pInfo->itemType==TREEITEM_INFO::INFO_MOD||
		pInfo->itemType==TREEITEM_INFO::INFO_DEV_GIM)
	{
		CTMDHGim* pGim=(CTMDHGim*)pInfo->dwRefData;
		theApp.SetActiveDoc(pGim,CTMDHDoc::GIM_DATA);
	}
	*pResult = 0;
}

void CTMDHPageDlg::OnTvnSelchangedTreeCtrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	HTREEITEM hItem = m_treeCtrl.GetSelectedItem();
	TREEITEM_INFO *pInfo=(TREEITEM_INFO*)m_treeCtrl.GetItemData(hItem);
	if(pInfo->itemType==TREEITEM_INFO::INFO_TA_SET)
		m_xMenuBtn.NotifyComboMenuClicked(ID_ADD_ITEM,"添加塔型数据");
	else if(pInfo->itemType==TREEITEM_INFO::INFO_TOWER)
		m_xMenuBtn.NotifyComboMenuClicked(ID_UPDATE_ITEM,"配基属性");
	else if(pInfo->itemType==TREEITEM_INFO::INFO_MODEL_SET)
		m_xMenuBtn.NotifyComboMenuClicked(ID_ADD_ITEM,"添加批量数据");
	else if(pInfo->itemType==TREEITEM_INFO::INFO_MODEL)
		m_xMenuBtn.NotifyComboMenuClicked(ID_UPDATE_ITEM,"配基属性");
	else if(pInfo->itemType==TREEITEM_INFO::INFO_MOD_SET)
		m_xMenuBtn.NotifyComboMenuClicked(ID_ADD_ITEM,"添加MOD数据");
	else if(pInfo->itemType==TREEITEM_INFO::INFO_MOD)
		m_xMenuBtn.NotifyComboMenuClicked(ID_PACK_DATA,"打包");
	else if(pInfo->itemType==TREEITEM_INFO::INFO_GIM_SET)
		m_xMenuBtn.NotifyComboMenuClicked(ID_ADD_ITEM,"添加GIM数据");
	else if(pInfo->itemType==TREEITEM_INFO::INFO_PRJ_GIM)
		m_xMenuBtn.NotifyComboMenuClicked(ID_PARSE_DATA,"数据解析");
	else if(pInfo->itemType==TREEITEM_INFO::INFO_DEV_GIM)
		m_xMenuBtn.NotifyComboMenuClicked(ID_UPDATE_ITEM,"更新GIM包");
	//
	if(pInfo->itemType==TREEITEM_INFO::INFO_MOD||
		pInfo->itemType==TREEITEM_INFO::INFO_DEV_GIM)
	{
		CTMDHGim* pGim=(CTMDHGim*)pInfo->dwRefData;
		//更新属性栏显示
		CMainFrame* pMainFrm=(CMainFrame*)AfxGetMainWnd();
		if(pMainFrm)
			pMainFrm->DisplayGimProperty(pGim);
	}
	*pResult = 0;
}

void CTMDHPageDlg::OnTvnKeydownTreeCtrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTVKEYDOWN pTVKeyDown = reinterpret_cast<LPNMTVKEYDOWN>(pNMHDR);
	if(pTVKeyDown->wVKey==VK_DELETE)
		OnDelItem();
	*pResult = 0;
}

void CTMDHPageDlg::OnAddItem()
{
	HTREEITEM hSelItem=NULL;
	TREEITEM_INFO *pItemInfo=GetSelItemInfo(&hSelItem);
	if(pItemInfo==NULL)
		return;
	CLogErrorLife logErrLife(&xMyErrLog);
	if(pItemInfo->itemType==TREEITEM_INFO::INFO_TA_SET)
	{	//杆塔集合
		CXhChar500 filter("铁塔设计绘图一体化软件(*.lds)|*.lds");
		filter.Append("|所有文件(*.*)|*.*||");
		CFileDialog dlg(TRUE,"塔型文件","塔型文件",OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,filter);
		if(dlg.DoModal()!=IDOK)
			return;
		AppendLdsFile(dlg.GetPathName());
	}
	else if(pItemInfo->itemType==TREEITEM_INFO::INFO_MODEL_SET)
	{
		CXhChar500 filter("铁塔模型信息展示文件(*.tid)|*.tid");
		filter.Append("|国网移交几何模型文件(*.mod)|*.mod");
		filter.Append("|批量配基文件(*.xls)|*.xls|批量配基文件(*.xlsx)|*.xlsx");
		filter.Append("|所有文件(*.*)|*.*||");
		CFileDialog file_dlg(TRUE,"模型文件","模型文件",OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,filter);
		if(file_dlg.DoModal()!=IDOK)
			return;
		CString extension=file_dlg.GetFileExt();
		if(extension.CompareNoCase("tid")==0||extension.CompareNoCase("TID")==0)
		{	//单个加载TID文件
			CTMDHModel* pModel=Manager.AppendModel();
			pModel->m_sTowerType=file_dlg.GetFileTitle();
			pModel->m_ciModelFlag=CTMDHModel::TYPE_TID;
			pModel->m_sFilePath.Copy(file_dlg.GetPathName());
			if(Manager.m_pTidModel==NULL)
				Manager.m_pTidModel=CTidModelFactory::CreateTidModel();
			//插入新的树节点
			HTREEITEM hItem=m_treeCtrl.InsertItem(pModel->m_sTowerType,IMG_MODEL_ITEM,IMG_MODEL_ITEM,hSelItem);
			TREEITEM_INFO *pSonItemInfo=m_xItemInfoList.append(TREEITEM_INFO(TREEITEM_INFO::INFO_MODEL,(DWORD)pModel));
			m_treeCtrl.SetItemData(hItem,(DWORD)pSonItemInfo);
			m_treeCtrl.SetItemText(hSelItem,CXhChar50("批量处理数据(%d)",Manager.ModelNum()));
			m_treeCtrl.Expand(hSelItem,TVE_EXPAND);
			m_treeCtrl.SelectItem(hItem);
			//
			theApp.SetActiveDoc(pModel,CTMDHDoc::TMD_DATA);
		}
		else if(extension.CompareNoCase("mod")==0||extension.CompareNoCase("MOD")==0)
		{	//单个加载MOD文件
			CTMDHModel* pModel=Manager.AppendModel();
			pModel->m_sTowerType=file_dlg.GetFileTitle();
			pModel->m_ciModelFlag=CTMDHModel::TYPE_MOD;
			pModel->m_sFilePath.Copy(file_dlg.GetPathName());
			if(Manager.m_pModModel==NULL)
				Manager.m_pModModel=CModModelFactory::CreateModModel();
			//插入新的树节点
			HTREEITEM hItem=m_treeCtrl.InsertItem(pModel->m_sTowerType,IMG_MODEL_ITEM,IMG_MODEL_ITEM,hSelItem);
			TREEITEM_INFO *pSonItemInfo=m_xItemInfoList.append(TREEITEM_INFO(TREEITEM_INFO::INFO_MODEL,(DWORD)pModel));
			m_treeCtrl.SetItemData(hItem,(DWORD)pSonItemInfo);
			m_treeCtrl.SetItemText(hSelItem,CXhChar50("批量处理数据(%d)",Manager.ModelNum()));
			m_treeCtrl.Expand(hSelItem,TVE_EXPAND);
			m_treeCtrl.SelectItem(hItem);
			//
			theApp.SetActiveDoc(pModel,CTMDHDoc::TMD_DATA);
		}
		else if(extension.CompareNoCase("xls")==0||extension.CompareNoCase("xlsx")==0)
		{	//批量加载塔例文件
			Manager.ReadBatchInstanceFile(file_dlg.GetPathName());
			//插入新的树节点
			m_treeCtrl.DeleteAllSonItems(hSelItem);
			for(CTMDHModel* pModel=Manager.EnumFirstModel();pModel;pModel=Manager.EnumNextModel())
			{
				if(pModel->m_ciErrorType>0)
					continue;
				CXhChar100 sName("%s-%s",(char*)pModel->m_sTowerNum,(char*)pModel->m_sTowerType);
				HTREEITEM hItem=m_treeCtrl.InsertItem(sName,IMG_MODEL_ITEM,IMG_MODEL_ITEM,hSelItem);
				TREEITEM_INFO *pSonItemInfo=m_xItemInfoList.append(TREEITEM_INFO(TREEITEM_INFO::INFO_MODEL,(DWORD)pModel));
				m_treeCtrl.SetItemData(hItem,(DWORD)pSonItemInfo);
			}
			m_treeCtrl.SetItemText(hSelItem,CXhChar50("批量处理数据(%d)",Manager.ModelNum()));
			m_treeCtrl.Expand(hSelItem,TVE_EXPAND);
		}
	}
	else if(pItemInfo->itemType==TREEITEM_INFO::INFO_MOD_SET)
	{
		CXhChar500 filter("国网移交几何模型文件(*.mod)|*.mod|");
		filter.Append("批打包文件(*.xls)|*.xls|批打包文件(*.xlsx)|*.xlsx|");
		filter.Append("所有文件(*.*)|*.*||");
		CFileDialog dlg(TRUE,"塔型文件","塔型文件",OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,filter);
		if(dlg.DoModal()!=IDOK)
			return;
		CString sFileExt=dlg.GetFileExt();
		if(sFileExt.CompareNoCase("mod")==0||sFileExt.CompareNoCase("MOD")==0)
		{	//单个加载
			AppendModFile(dlg.GetPathName());
		}
		else if(sFileExt.CompareNoCase("xls")==0||sFileExt.CompareNoCase("xlsx")==0)
		{	//批量加载
			Manager.ReadTaModBatchGzipFile(dlg.GetPathName());
			m_treeCtrl.DeleteAllSonItems(hSelItem);
			for(CTMDHGim* pGim=Manager.EnumFirstGim();pGim;pGim=Manager.EnumNextGim())
			{	//插入新的树节点
				HTREEITEM hItem=m_treeCtrl.InsertItem(pGim->m_sModName,IMG_MOD_ITEM,IMG_MOD_ITEM,hSelItem);
				TREEITEM_INFO *pSonItemInfo=m_xItemInfoList.append(TREEITEM_INFO(TREEITEM_INFO::INFO_MOD,(DWORD)pGim));
				m_treeCtrl.SetItemData(hItem,(DWORD)pSonItemInfo);	
			}
			m_treeCtrl.SetItemText(hSelItem,CXhChar50("MOD打包数据(%d)",Manager.GimNum()));
			m_treeCtrl.Expand(hSelItem,TVE_EXPAND);
		}
	}
	else if(pItemInfo->itemType==TREEITEM_INFO::INFO_GIM_SET)
	{
		CFileDialog dlg(TRUE,"GIM包","GIM包",OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
			"GIM文件(*.gim)|*.gim|所有文件(*.*)|*.*||");
		if(dlg.DoModal()!=IDOK)
			return;
		AppendGimFile(dlg.GetPathName());
	}
}

void CTMDHPageDlg::OnDelItem()
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
		m_treeCtrl.SetItemText(m_hTaSetItem,CXhChar50("杆塔模型数据(%d)",Manager.TowerNum()));
		//
		theApp.DeleteDoc(pTower);
	}
	else if(pItemInfo->itemType==TREEITEM_INFO::INFO_MODEL)
	{
		CTMDHModel* pModel=(CTMDHModel*)pItemInfo->dwRefData;
		if(pModel)
			Manager.DeleteModel(pModel);
		m_treeCtrl.DeleteItem(hSelItem);
		m_treeCtrl.SetItemText(m_hModelSetItem,CXhChar50("杆塔处理数据(%d)",Manager.ModelNum()));
		//
		theApp.DeleteDoc(pModel);
	}
	else if(pItemInfo->itemType==TREEITEM_INFO::INFO_MOD)
	{
		CTMDHGim* pGim=(CTMDHGim*)pItemInfo->dwRefData;
		if(pGim)
			Manager.DeleteGim(pGim);
		m_treeCtrl.DeleteItem(hSelItem);
		m_treeCtrl.SetItemText(m_hModSetItem,CXhChar50("MOD打包数据(%d)",Manager.GimNum()));
		//
		theApp.DeleteDoc(pGim);
	}
	else if(pItemInfo->itemType==TREEITEM_INFO::INFO_PRJ_GIM)
	{
		CTMDHPrjGim* pPrjGim=(CTMDHPrjGim*)pItemInfo->dwRefData;
		if(pPrjGim)
			Manager.DeletePrjGim(pPrjGim);
		m_treeCtrl.DeleteItem(hSelItem);
		m_treeCtrl.SetItemText(m_hGimSetItem,CXhChar50("GIM移交数据(%d)",Manager.PrjGimNum()));
	}
}

void CTMDHPageDlg::OnUpdateItem()
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
	else if(pItemInfo->itemType==TREEITEM_INFO::INFO_MODEL)
	{
		CTMDHModel* pModel=(CTMDHModel*)pItemInfo->dwRefData;
		CInstanceDlg dlg;
		dlg.m_ciModelFlag=1;
		dlg.m_pModel=pModel;
		if(dlg.DoModal()!=IDOK)
			return;
		CTMDHView* pView=theApp.GetActiveView();
		if(pView)
			pView->Refresh();
	}
	else if(pItemInfo->itemType==TREEITEM_INFO::INFO_MOD_SET)
	{
		for(CTMDHGim* pGim=Manager.EnumFirstGim();pGim;pGim=Manager.EnumNextGim())
			pGim->CheckModData();
		AfxMessageBox("检测完成");
	}
	else if(pItemInfo->itemType==TREEITEM_INFO::INFO_PRJ_GIM)
	{
		CWaitCursor waitCursor;
		CTMDHPrjGim* pPrjGim=(CTMDHPrjGim*)pItemInfo->dwRefData;
		if(pPrjGim->UpdateGimTowerPro())
		{
			if(AfxMessageBox("GIM更新完成！是否打开输出路径？",MB_YESNO)==IDYES)
				WinExec(CXhChar500("explorer.exe %s",(char*)pPrjGim->m_sGimPath),SW_SHOW);
		}
	}
	else if(pItemInfo->itemType==TREEITEM_INFO::INFO_DEV_GIM)
	{
		CWaitCursor waitCursor;
		CTMDHGim* pGim=(CTMDHGim*)pItemInfo->dwRefData;
		if(pGim->UpdateGimTowerPro())
		{
			if(AfxMessageBox("GIM更新完成！是否打开输出路径？",MB_YESNO)==IDYES)
				WinExec(CXhChar500("explorer.exe %s",(char*)pGim->m_sOutputPath),SW_SHOW);
		}
	}
}
//GIM包解析
void CTMDHPageDlg::OnParseData()
{
	HTREEITEM hSelItem=NULL;
	TREEITEM_INFO *pItemInfo=GetSelItemInfo(&hSelItem);
	if(pItemInfo==NULL||pItemInfo->itemType!=TREEITEM_INFO::INFO_PRJ_GIM)
		return;
	CTMDHPrjGim* pPrjGim=(CTMDHPrjGim*)pItemInfo->dwRefData;
	CXhChar200 sGimFile("%s\\%s.gim",(char*)pPrjGim->m_sGimPath,(char*)pPrjGim->m_sGimName);
	if(!pPrjGim->ParseGimFile(sGimFile))
	{
		logerr.Log(CXhChar100("GIM文件解析失败!"));
		return;
	}
	//刷新树节点
	m_treeCtrl.DeleteAllSonItems(hSelItem);
	int index=1;
	for(CTMDHGim* pGim=pPrjGim->EnumFirstGim();pGim;pGim=pPrjGim->EnumNextGim(),index++)
	{
		CXhChar100 ss("杆塔模型%d(%s)",index,(char*)pGim->m_sModName);
		HTREEITEM hSonItem=m_treeCtrl.InsertItem(ss,IMG_DATA_ITEM,IMG_DATA_ITEM,hSelItem);
		TREEITEM_INFO* pSonItemInfo=m_xItemInfoList.append(TREEITEM_INFO(TREEITEM_INFO::INFO_DEV_GIM,(DWORD)pGim));
		m_treeCtrl.SetItemData(hSonItem,(DWORD)pSonItemInfo);
	}
	m_treeCtrl.Expand(hSelItem,TVE_EXPAND);
}
//进行数据转换生成
void CTMDHPageDlg::OnTransData()
{
	HTREEITEM hSelItem=NULL;
	TREEITEM_INFO *pItemInfo=GetSelItemInfo(&hSelItem);
	if(pItemInfo==NULL||pItemInfo->itemType!=TREEITEM_INFO::INFO_MOD_SET)
		return;
	CTMDHGim* pFirGim=Manager.EnumFirstGim();
	CString sPath=pFirGim->m_sOutputPath;
	if(sPath.GetLength()<=0 && InvokeFolderPickerDlg(sPath))
	{
		for(CTMDHGim* pGim=Manager.EnumFirstGim();pGim;pGim=Manager.EnumNextGim())
			pGim->m_sOutputPath.Copy(sPath);
	}
	for(CTMDHGim* pGim=Manager.EnumFirstGim();pGim;pGim=Manager.EnumNextGim())
		pGim->CreateGuidFile();
	if(AfxMessageBox("数据转换完成！是否打开输出路径？",MB_YESNO)==IDYES)
		WinExec(CXhChar500("explorer.exe %s",sPath),SW_SHOW);
}
//进行压缩打包
void CTMDHPageDlg::OnPackData()
{
	HTREEITEM hSelItem=NULL;
	TREEITEM_INFO *pItemInfo=GetSelItemInfo(&hSelItem);
	if(pItemInfo==NULL)
		return;
	if(pItemInfo->itemType==TREEITEM_INFO::INFO_MOD_SET)
	{	//批量打包
		CTMDHGim* pFirGim=Manager.EnumFirstGim();
		CString sPath=pFirGim->m_sOutputPath;
		if(sPath.GetLength()<=0&&InvokeFolderPickerDlg(sPath))
		{
			for(CTMDHGim* pGim=Manager.EnumFirstGim();pGim;pGim=Manager.EnumNextGim())
				pGim->m_sOutputPath.Copy(sPath);
		}
		for(CTMDHGim* pGim=Manager.EnumFirstGim();pGim;pGim=Manager.EnumNextGim())
			pGim->DeleteGuidFile();
		for(CTMDHGim* pGim=Manager.EnumFirstGim();pGim;pGim=Manager.EnumNextGim())
			pGim->CreateGuidFile();
		for(CTMDHGim* pGim=Manager.EnumFirstGim();pGim;pGim=Manager.EnumNextGim())
			pGim->PackGimFile();
		for(CTMDHGim* pGim=Manager.EnumFirstGim();pGim;pGim=Manager.EnumNextGim())
			pGim->UpdateGimHeadInfo();
		if(AfxMessageBox("GIM打包完成！是否打开输出路径？",MB_YESNO)==IDYES)
			WinExec(CXhChar500("explorer.exe %s",sPath),SW_SHOW);
	}
	else if(pItemInfo->itemType==TREEITEM_INFO::INFO_MOD)
	{	//单个打包
		CTMDHGim* pGim=(CTMDHGim*)pItemInfo->dwRefData;
		if(pGim==NULL)
			return;
		pGim->CreateGuidFile();
		pGim->PackGimFile();
		pGim->UpdateGimHeadInfo();
		if(AfxMessageBox("GIM打包完成！是否打开输出路径？",MB_YESNO)==IDYES)
			WinExec(CXhChar500("explorer.exe %s",(char*)pGim->m_sOutputPath),SW_SHOW);
	}
	
}
//解压GIM包
void CTMDHPageDlg::OnUnpackData()
{
	HTREEITEM hSelItem=NULL;
	TREEITEM_INFO *pItemInfo=GetSelItemInfo(&hSelItem);
	if(pItemInfo==NULL||pItemInfo->itemType!=TREEITEM_INFO::INFO_PRJ_GIM)
		return;
	CWaitCursor waitCursor;
	CTMDHPrjGim* pPrjGim=(CTMDHPrjGim*)pItemInfo->dwRefData;
	CString sFolder=pPrjGim->m_sGimPath;
	if(sFolder.GetLength()<=0 && !InvokeFolderPickerDlg(sFolder))
		return;
	pPrjGim->UnpackGimFile(sFolder);
	if(AfxMessageBox("GIM解压完成！是否打开输出路径？",MB_YESNO)==IDYES)
		WinExec(CXhChar500("explorer.exe %s",sFolder),SW_SHOW);
}
void CTMDHPageDlg::OnExportTidFiles()
{
	TREEITEM_INFO *pItemInfo=GetSelItemInfo();
	if(pItemInfo==NULL||pItemInfo->itemType!=TREEITEM_INFO::INFO_TA_SET)
		return;
	if(Manager.TowerNum()<=0)
	{
		AfxMessageBox("杆塔模型数据为空!");
		return;
	}
	CString sFolder;
	if(!InvokeFolderPickerDlg(sFolder))
		return;
	sFolder.Append("\\TID");
	MakeDirectory(sFolder);
	for(CTMDHTower* pTower=Manager.EnumFirstTA();pTower;pTower=Manager.EnumNextTA())
	{
		if(pTower->m_xTower.Parts.GetNodeNum()<=0)
			continue;
		CXhChar200 sFilename("%s\\%s.tid",sFolder.GetBuffer(),(char*)pTower->m_sTower);
		pTower->CreateTidFile(sFilename);
	}
	//
	WinExec(CXhChar500("explorer.exe %s",sFolder.GetBuffer()),SW_SHOW);
}

void CTMDHPageDlg::OnExportModFiles()
{
	TREEITEM_INFO *pItemInfo=GetSelItemInfo();
	if(pItemInfo==NULL)
		return;
	if(pItemInfo->itemType==TREEITEM_INFO::INFO_TA_SET)
	{
		if(Manager.TowerNum()<=0)
		{
			AfxMessageBox("杆塔模型数据为空!");
			return;
		}
		CString sFolder;
		if(!InvokeFolderPickerDlg(sFolder))
			return;
		sFolder.Append("\\MOD");
		MakeDirectory(sFolder);
		//
		int nNum=Manager.TowerNum(),index=0;
		DisplayProcess(0,"导出国网移交几何模型文件.....");
		for(CTMDHTower* pTower=Manager.EnumFirstTA();pTower;pTower=Manager.EnumNextTA(),index++)
		{
			DisplayProcess(ftoi(100*index/nNum),"导出国网移交几何模型文件.....");
			if(pTower->m_xTower.Parts.GetNodeNum()<=0)
			{
				logerr.Log("%s塔型数据为空!",(char*)pTower->m_sTower);
				continue;
			}
			CXhChar200 sFilename("%s\\%s.mod",sFolder.GetBuffer(),(char*)pTower->m_sTower);
			pTower->CreateModFile(sFilename);
		}
		DisplayProcess(100,"导出国网移交几何模型文件.....");
		//
		WinExec(CXhChar500("explorer.exe %s",sFolder.GetBuffer()),SW_SHOW);
	}
	else if(pItemInfo->itemType==TREEITEM_INFO::INFO_MODEL_SET)
	{
		if(Manager.ModelNum()<=0)
		{
			AfxMessageBox("批量处理数据为空!");
			return;
		}
		CString sFolder=Manager.EnumFirstModel()->m_sFilePath;
		int index=sFolder.ReverseFind('\\');
		sFolder=sFolder.Left(index);
		sFolder.Append("\\MOD");
		MakeDirectory(sFolder);
		//
		DisplayProcess(0,"生成MOD文件");
		int i=1,nModel=Manager.ModelNum();
		for(CTMDHModel* pModel=Manager.EnumFirstModel();pModel;pModel=Manager.EnumNextModel(),i++)
		{
			DisplayProcess(int(100*i/nModel),"生成MOD文件");
			if(pModel->m_ciErrorType>0)
				continue;
			void* pInstance=pModel->ExtractActiveTaInstance();
			if(pInstance==NULL)
				continue;
			CXhChar200 sFullPath("%s\\%s.mod",sFolder.GetBuffer(),(char*)pModel->GetFileName(pInstance));
			if(pModel->m_ciModelFlag==CTMDHModel::TYPE_TID)	//TID模型
				pModel->CreateModFile((ITidTowerInstance*)pInstance,sFullPath);
			if(pModel->m_ciModelFlag==CTMDHModel::TYPE_MOD)	//MOD模型
				pModel->CreateModFile((IModTowerInstance*)pInstance,sFullPath);
		}
		DisplayProcess(100,"生成MOD文件");
		//
		WinExec(CXhChar500("explorer.exe %s",sFolder.GetBuffer()),SW_SHOW);
	}
}
void CTMDHPageDlg::OnExport3dsFiles()
{
	TREEITEM_INFO *pItemInfo=GetSelItemInfo();
	if(pItemInfo==NULL)
		return;
	if(pItemInfo->itemType==TREEITEM_INFO::INFO_TA_SET)
	{
		if(Manager.TowerNum()<=0)
		{
			AfxMessageBox("杆塔模型数据为空!");
			return;
		}
		CString sFolder;
		if(!InvokeFolderPickerDlg(sFolder))
			return;
		sFolder.Append("\\3DS");
		MakeDirectory(sFolder);
		//
		int nNum=Manager.TowerNum(),index=0;
		DisplayProcess(0,"导出3ds文件.....");
		for(CTMDHTower* pTower=Manager.EnumFirstTA();pTower;pTower=Manager.EnumNextTA(),index++)
		{
			DisplayProcess(ftoi(100*index/nNum),"导出3ds文件.....");
			if(pTower->m_xTower.Parts.GetNodeNum()<=0)
			{
				logerr.Log("%s塔型数据为空!",(char*)pTower->m_sTower);
				continue;
			}
			CXhChar200 sFilename("%s\\%s.3ds",sFolder.GetBuffer(),(char*)pTower->m_sTower);
			pTower->Create3dsFile(sFilename);
		}
		DisplayProcess(100,"导出3ds文件.....");
		//
		WinExec(CXhChar500("explorer.exe %s",sFolder.GetBuffer()),SW_SHOW);
	}
	else if(pItemInfo->itemType==TREEITEM_INFO::INFO_MODEL_SET)
	{
		if(Manager.ModelNum()<=0)
		{
			AfxMessageBox("批量处理数据为空!");
			return;
		}
		CString sFolder=Manager.EnumFirstModel()->m_sFilePath;
		int index=sFolder.ReverseFind('\\');
		sFolder=sFolder.Left(index);
		sFolder.Append("\\3DS");
		MakeDirectory(sFolder);
		//
		int i=1,nModel=Manager.ModelNum();
		DisplayProcess(0,"生成3DSL文件");
		for(CTMDHModel* pModel=Manager.EnumFirstModel();pModel;pModel=Manager.EnumNextModel(),i++)
		{
			DisplayProcess(int(100*i/nModel),"生成3DS文件");
			if(pModel->m_ciErrorType>0)
				continue;
			void* pInstance=pModel->ExtractActiveTaInstance();
			if(pInstance==NULL)
				continue;
			CXhChar200 sFullPath("%s\\%s.3ds",sFolder.GetBuffer(),(char*)pModel->GetFileName(pInstance));
			if(Manager.m_ciInstanceLevel==0)
				sFullPath.Printf("%s\\%s.dxf",sFolder.GetBuffer(),(char*)pModel->GetFileName(pInstance));
			if(Manager.m_ciInstanceLevel==0)
			{
				if(pModel->m_ciModelFlag==CTMDHModel::TYPE_TID)	//TID模型
					pModel->CreateDxfFile((ITidTowerInstance*)pInstance,sFullPath);
				if(pModel->m_ciModelFlag==CTMDHModel::TYPE_MOD)	//MOD模型
					pModel->CreateDxfFile((IModTowerInstance*)pInstance,sFullPath);
			}
			else
			{
				if(pModel->m_ciModelFlag==CTMDHModel::TYPE_TID)	//TID模型
					pModel->Create3dsFile((ITidTowerInstance*)pInstance,sFullPath);
				if(pModel->m_ciModelFlag==CTMDHModel::TYPE_MOD)	//MOD模型
					pModel->Create3dsFile((IModTowerInstance*)pInstance,sFullPath);
			}
		}
		DisplayProcess(100,"生成3DS文件");
		//
		WinExec(CXhChar500("explorer.exe %s",sFolder.GetBuffer()),SW_SHOW);
	}
}
void CTMDHPageDlg::OnExportStlFiles()
{
	TREEITEM_INFO *pItemInfo=GetSelItemInfo();
	if(pItemInfo==NULL)
		return;
	if(pItemInfo->itemType==TREEITEM_INFO::INFO_TA_SET)
	{
		if(Manager.TowerNum()<=0)
		{
			AfxMessageBox("杆塔模型数据为空!");
			return;
		}
		CString sFolder;
		if(!InvokeFolderPickerDlg(sFolder))
			return;
		sFolder.Append("\\STL");
		MakeDirectory(sFolder);
		//
		int nNum=Manager.TowerNum(),index=0;
		DisplayProcess(0,"生成STL文件.....");
		for(CTMDHTower* pTower=Manager.EnumFirstTA();pTower;pTower=Manager.EnumNextTA(),index++)
		{
			DisplayProcess(ftoi(100*index/nNum),"生成STL文件.....");
			if(pTower->m_xTower.Parts.GetNodeNum()<=0)
			{
				logerr.Log("%s塔型数据为空!",(char*)pTower->m_sTower);
				continue;
			}
			CXhChar200 sFilename("%s\\%s.stl",sFolder.GetBuffer(),(char*)pTower->m_sTower);
			pTower->CreateStlFile(sFilename);
		}
		DisplayProcess(100,"生成STL文件.....");
		//
		WinExec(CXhChar500("explorer.exe %s",sFolder.GetBuffer()),SW_SHOW);
	}
	else if(pItemInfo->itemType==TREEITEM_INFO::INFO_MODEL_SET)
	{
		if(Manager.ModelNum()<=0)
		{
			AfxMessageBox("批量处理数据为空!");
			return;
		}
		CString sFolder=Manager.EnumFirstModel()->m_sFilePath;
		int index=sFolder.ReverseFind('\\');
		sFolder=sFolder.Left(index);
		sFolder.Append("\\STL");
		MakeDirectory(sFolder);
		//
		int i=1,nModel=Manager.ModelNum();
		DisplayProcess(0,"生成STL文件.....");
		for(CTMDHModel* pModel=Manager.EnumFirstModel();pModel;pModel=Manager.EnumNextModel(),i++)
		{
			DisplayProcess(int(100*i/nModel),"生成STL文件.....");
			if(pModel->m_ciErrorType>0)
				continue;
			void* pInstance=pModel->ExtractActiveTaInstance();
			if(pInstance==NULL)
				continue;
			CXhChar200 sFullPath("%s\\%s.stl",sFolder.GetBuffer(),(char*)pModel->GetFileName(pInstance));
			if(pModel->m_ciModelFlag==CTMDHModel::TYPE_TID)	//TID模型
				pModel->CreateStlFile((ITidTowerInstance*)pInstance,sFullPath);
			if(pModel->m_ciModelFlag==CTMDHModel::TYPE_MOD)	//MOD模型
				pModel->CreateStlFile((IModTowerInstance*)pInstance,sFullPath);
		}
		DisplayProcess(100,"生成STL文件.....");
		//
		WinExec(CXhChar500("explorer.exe %s",sFolder.GetBuffer()),SW_SHOW);
	}
}
void CTMDHPageDlg::OnExportXmlFiles()
{
	TREEITEM_INFO *pItemInfo=GetSelItemInfo();
	if(pItemInfo==NULL||pItemInfo->itemType!=TREEITEM_INFO::INFO_MODEL_SET)
		return;
	if(Manager.ModelNum()<=0)
	{
		AfxMessageBox("批量处理数据为空!");
		return;
	}
	CString sFolder=Manager.EnumFirstModel()->m_sFilePath;
	int index=sFolder.ReverseFind('\\');
	sFolder=sFolder.Left(index);
	sFolder.Append("\\XML");
	MakeDirectory(sFolder);
	//
	int i=1,nModel=Manager.ModelNum();
	DisplayProcess(0,"生成XML文件");
	for(CTMDHModel* pModel=Manager.EnumFirstModel();pModel;pModel=Manager.EnumNextModel(),i++)
	{
		DisplayProcess(int(100*i/nModel),"生成XML文件");
		if(pModel->m_ciErrorType>0)
			continue;
		void* pInstance=pModel->ExtractActiveTaInstance();
		if(pInstance==NULL)
			continue;
		CXhChar200 sFullPath("%s\\%s.xml",sFolder.GetBuffer(),(char*)pModel->GetFileName(pInstance));
		if(pModel->m_ciModelFlag==CTMDHModel::TYPE_TID)	//TID模型
			pModel->CreateXmlFile((ITidTowerInstance*)pInstance,sFullPath);
		if(pModel->m_ciModelFlag==CTMDHModel::TYPE_MOD)	//MOD模型
			pModel->CreateXmlFile((IModTowerInstance*)pInstance,sFullPath);
	}
	DisplayProcess(100,"生成XML文件");
	//
	WinExec(CXhChar500("explorer.exe %s",sFolder.GetBuffer()),SW_SHOW);
}