// TowerTreeDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "TPSM.h"
#include "ProjectTreeDlg.h"
#include "afxdialogex.h"
#include "NewProjectDlg.h"
#include "LogFile.h"
#include "GlobalFunc.h"
#include "ProcBarDlg.h"
#include "CryptBuffer.h"
#include "TowerTemplateDlg.h"
#include "TrunkSegment.h"
#include "HuGaoPlanDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////////
// CTowerTreeDlg 对话框
IMPLEMENT_DYNCREATE(CProjectTreeDlg, CDialogEx)

CProjectTreeDlg::CProjectTreeDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CProjectTreeDlg::IDD, pParent)
{

}

CProjectTreeDlg::~CProjectTreeDlg()
{
}

void CProjectTreeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE_CTRL, m_treeCtrl);
}


BEGIN_MESSAGE_MAP(CProjectTreeDlg, CDialogEx)
	ON_WM_SIZE()
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_CTRL, OnTvnSelchangedTreeCtrl)
	ON_NOTIFY(TVN_KEYDOWN, IDC_TREE_CTRL, OnTvnKeydownTreeCtrl)
	ON_NOTIFY(NM_RCLICK, IDC_TREE_CTRL, OnRclickTreeCtrl)
	ON_COMMAND(ID_ADD_ITEM,OnAddItem)
	ON_COMMAND(ID_NEW_ITEM,OnNewItem)
	ON_COMMAND(ID_DEL_ITEM,OnDelItem)
	ON_COMMAND(ID_UPDATE_ITEM,OnUpdateItem)
	ON_COMMAND(ID_EXPORT_TID,OnExportTidFiles)
	ON_COMMAND(ID_EXPORT_MOD,OnExportModFiles)
	ON_COMMAND(ID_EXPORT_DXF,OnExportDxfFiles)
	ON_COMMAND(ID_EXPORT_3DS,OnExport3dsFiles)
	ON_COMMAND(ID_EXPORT_LDS,OnExportLdsFiles)
	ON_COMMAND(ID_NEW_TOWER_MODEL,OnPlanTrunkSegment)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE_CTRL, &CProjectTreeDlg::OnNMDblclkTreeCtrl)
END_MESSAGE_MAP()


// CTowerTreeDlg 消息处理程序
BOOL CProjectTreeDlg::OnInitDialog() 
{
	CDialogEx::OnInitDialog();
	//
	m_images.Create(IDB_IL_PROJECT, 16, 1, RGB(0,255,0));
	m_treeCtrl.SetImageList(&m_images,TVSIL_NORMAL);
	m_treeCtrl.ModifyStyle(0,TVS_HASLINES|TVS_HASBUTTONS|TVS_SHOWSELALWAYS|TVS_FULLROWSELECT);
	InitTreeView();
	return TRUE;  
}
void CProjectTreeDlg::OnOK() 
{
}

void CProjectTreeDlg::OnCancel() 
{
}

void CProjectTreeDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialogEx::OnSize(nType, cx, cy);
	//
	if(m_treeCtrl.GetSafeHwnd())
		m_treeCtrl.MoveWindow(0,0,cx,cy);
}
void CProjectTreeDlg::InitTreeView()
{
	m_treeCtrl.DeleteAllItems();
	TREEITEM_INFO *pItemInfo=NULL;
	CTPSMProject* pPrj=NULL;
	//工程塔型节点
	pPrj=Manager.AddProject();
	pPrj->m_sPrjName.Copy("工程塔型");
	m_hPrjTowerModel=m_treeCtrl.InsertItem(pPrj->m_sPrjName,IMG_TOWERMODEL_SET,IMG_TOWERMODEL_SET,TVI_ROOT);
	pItemInfo=m_xItemInfoList.append(TREEITEM_INFO(TREEITEM_INFO::INFO_TOWERMODEL_SET,(DWORD)pPrj));
	m_treeCtrl.SetItemData(m_hPrjTowerModel,(DWORD)pItemInfo);
	//工程塔位节点
	/*pPrj=Manager.AddProject();
	pPrj->m_sPrjName.Copy("工程塔位");
	m_hPrjInstance=m_treeCtrl.InsertItem(pPrj->m_sPrjName,IMG_TAINSTANCE_SET,IMG_TAINSTANCE_SET,TVI_ROOT);
	pItemInfo=m_xItemInfoList.append(TREEITEM_INFO(TREEITEM_INFO::INFO_TAINSTANCE_SET,(DWORD)pPrj));
	m_treeCtrl.SetItemData(m_hPrjInstance,(DWORD)pItemInfo);*/
}

void CProjectTreeDlg::RefreshPrjTree(HTREEITEM hPrjItem)
{
	TREEITEM_INFO *pItemInfo=(TREEITEM_INFO*)m_treeCtrl.GetItemData(hPrjItem);
	if(pItemInfo==NULL)
		return;
	CXhChar100 sText;
	if(hPrjItem==m_hPrjTowerModel||hPrjItem==m_hPrjInstance)
	{
		CTPSMProject* pPrj=(CTPSMProject*)pItemInfo->dwRefData;
		sText.Printf("%s(%d)",(char*)pPrj->m_sPrjName,pPrj->TowerNum());
		m_treeCtrl.SetItemText(hPrjItem,sText);
		m_treeCtrl.DeleteAllSonItems(hPrjItem);
		for(CTPSMModel* pTower=pPrj->EnumFirstModel();pTower;pTower=pPrj->EnumNextModel())
		{
			HTREEITEM hItem=m_treeCtrl.InsertItem(pTower->m_sTower,IMG_TOWER_ITEM,IMG_TOWER_ITEM,hPrjItem);
			TREEITEM_INFO *pSonItemInfo=m_xItemInfoList.append(TREEITEM_INFO(TREEITEM_INFO::INFO_TOWER_ITEM,(DWORD)pTower));
			m_treeCtrl.SetItemData(hItem,(DWORD)pSonItemInfo);
		}
		m_treeCtrl.Expand(hPrjItem,TVE_EXPAND);	
	}
}

TREEITEM_INFO *CProjectTreeDlg::GetSelItemInfo(HTREEITEM* pItem/*=NULL*/)
{
	HTREEITEM hSelItem=m_treeCtrl.GetSelectedItem();
	if(hSelItem==NULL)
		return NULL;
	TREEITEM_INFO *pItemInfo=(TREEITEM_INFO*)m_treeCtrl.GetItemData(hSelItem);
	if(pItem)
		*pItem=hSelItem;
	return pItemInfo;
}

void CProjectTreeDlg::ContextMenu(CWnd *pWnd, CPoint point)
{
	TREEITEM_INFO *pItemInfo=GetSelItemInfo();
	if(pItemInfo==NULL)
		return;
	CPoint scr_point = point;
	m_treeCtrl.ClientToScreen(&scr_point);
	CMenu menu;
	menu.CreatePopupMenu();
	if(pItemInfo->itemType==TREEITEM_INFO::INFO_TOWERMODEL_SET||
		pItemInfo->itemType==TREEITEM_INFO::INFO_TAINSTANCE_SET)
	{	//工程
		menu.AppendMenu(MF_STRING,ID_NEW_ITEM,"新建");
		menu.AppendMenu(MF_STRING,ID_ADD_ITEM,"导入");
		menu.AppendMenu(MF_STRING,ID_UPDATE_ITEM,"保存");
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING,ID_EXPORT_LDS,"导出LDS文件");
		menu.AppendMenu(MF_STRING,ID_EXPORT_TID,"导出TID文件");
		menu.AppendMenu(MF_STRING,ID_EXPORT_MOD,"导出MOD文件");
		menu.AppendMenu(MF_STRING,ID_EXPORT_DXF,"导出DXF文件");
		menu.AppendMenu(MF_STRING,ID_EXPORT_3DS,"导出3DS文件");
	}
	else if(pItemInfo->itemType==TREEITEM_INFO::INFO_TOWER_ITEM)
	{	//塔型
		menu.AppendMenu(MF_STRING,ID_NEW_TOWER_MODEL,"呼高规划");
		menu.AppendMenu(MF_STRING,ID_UPDATE_ITEM,"生成");
		menu.AppendMenu(MF_STRING,ID_DEL_ITEM,"删除");
	}
	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, scr_point.x, scr_point.y, this, NULL);
	menu.DestroyMenu();
}

void CProjectTreeDlg::OnRclickTreeCtrl(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TVHITTESTINFO HitTestInfo;
	GetCursorPos(&HitTestInfo.pt);
	CTreeCtrl *pTreeCtrl = GetTreeCtrl();
	pTreeCtrl->ScreenToClient(&HitTestInfo.pt);
	pTreeCtrl->HitTest(&HitTestInfo);
	pTreeCtrl->Select(HitTestInfo.hItem,TVGN_CARET);

	ContextMenu(this,HitTestInfo.pt);
	*pResult = 0;
}

void CProjectTreeDlg::OnNMDblclkTreeCtrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	TREEITEM_INFO *pInfo=GetSelItemInfo();
	if(pInfo!=NULL && pInfo->itemType==TREEITEM_INFO::INFO_TOWER_ITEM)
	{
		CTPSMModel* pModel=(CTPSMModel*)pInfo->dwRefData;
		theApp.SetActiveDoc(pModel);
	}
	*pResult = 0;
}

void CProjectTreeDlg::OnTvnSelchangedTreeCtrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	HTREEITEM hItem = m_treeCtrl.GetSelectedItem();
	TREEITEM_INFO *pInfo=(TREEITEM_INFO*)m_treeCtrl.GetItemData(hItem);
	if(pInfo!=NULL&&pInfo->itemType!=TREEITEM_INFO::INFO_PROJECT_SET)
	{
		if(pInfo->itemType==TREEITEM_INFO::INFO_TOWERMODEL_SET)
		{
			Manager.m_pActivePrj=(CTPSMProject*)pInfo->dwRefData;
			Manager.m_pActivePrj->m_pActiveModel=Manager.m_pActivePrj->EnumFirstModel();
		}
		else if(pInfo->itemType==TREEITEM_INFO::INFO_TAINSTANCE_SET)
		{
			Manager.m_pActivePrj=(CTPSMProject*)pInfo->dwRefData;
			Manager.m_pActivePrj->m_pActiveModel=Manager.m_pActivePrj->EnumFirstModel();
		}
		else if(pInfo->itemType==TREEITEM_INFO::INFO_TOWER_ITEM)
		{
			CTPSMModel* pModel=(CTPSMModel*)pInfo->dwRefData;
			Manager.m_pActivePrj=pModel->BelongPrj();
			Manager.m_pActivePrj->m_pActiveModel=pModel;
			//更新属性栏显示
			CMainFrame* pMainFrm=(CMainFrame*)AfxGetMainWnd();
			if(pMainFrm)
			{
				pMainFrm->UpdatePropertyPage();
				pMainFrm->GetWireDataPage()->UpdateWireNodeList();
			}
		}
	}
	*pResult = 0;
}

void CProjectTreeDlg::OnTvnKeydownTreeCtrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTVKEYDOWN pTVKeyDown = reinterpret_cast<LPNMTVKEYDOWN>(pNMHDR);
	if(pTVKeyDown->wVKey==VK_DELETE)
		OnDelItem();
	*pResult = 0;
}

void CProjectTreeDlg::OnAddItem()
{
	HTREEITEM hSelItem=NULL;
	TREEITEM_INFO *pItemInfo=GetSelItemInfo(&hSelItem);
	if(pItemInfo==NULL)
		return;
	CLogErrorLife logErrLife;
	if(pItemInfo->itemType==TREEITEM_INFO::INFO_TOWERMODEL_SET||pItemInfo->itemType==TREEITEM_INFO::INFO_TAINSTANCE_SET)
	{	//添加塔型
		CXhChar500 filter("塔型文件(*.tpi)|*.tpi");
		filter.Append("|所有文件(*.*)|*.*||");
		CFileDialog dlg(TRUE,"塔型文件","塔型文件",OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,filter);
		if(dlg.DoModal()!=IDOK)
			return;
		CTPSMProject* pPrj=(CTPSMProject*)pItemInfo->dwRefData;
		CTPSMModel* pAddModel=pPrj->AppendModel();
		pAddModel->SetBelongPrj(pPrj);
		pAddModel->m_sFullPath.Copy(dlg.GetPathName().GetString());
		pAddModel->OpenTpiFile();
		pAddModel->InitParaSketchItems();
		pAddModel->m_nMaxLegs=CFGLEG::MaxLegs();
		//插入新的树节点
		m_treeCtrl.SetItemText(hSelItem,CXhChar50("%s(%d)",(char*)pPrj->m_sPrjName,pPrj->TowerNum()));
		HTREEITEM hItem=m_treeCtrl.InsertItem(pAddModel->m_sTower,IMG_TOWER_ITEM,IMG_TOWER_ITEM,hSelItem);
		TREEITEM_INFO *pSonItemInfo=m_xItemInfoList.append(TREEITEM_INFO(TREEITEM_INFO::INFO_TOWER_ITEM,(DWORD)pAddModel));
		m_treeCtrl.SetItemData(hItem,(DWORD)pSonItemInfo);
		m_treeCtrl.Expand(hSelItem,TVE_EXPAND);
		m_treeCtrl.SelectItem(hItem);
		//刷新塔型实体
		theApp.SetActiveDoc(pAddModel);
	}
}
static CTowerTemplateDlg towerTemlDlg;
void CProjectTreeDlg::NewTowerModel(HTREEITEM hSelItem)
{
	TREEITEM_INFO *pItemInfo=(TREEITEM_INFO*)m_treeCtrl.GetItemData(hSelItem);
	if(pItemInfo==NULL||pItemInfo->itemType!=TREEITEM_INFO::INFO_TOWERMODEL_SET)
		return;
	CTPSMProject* pPrj=(CTPSMProject*)pItemInfo->dwRefData;
	if(towerTemlDlg.DoModal()!=IDOK)
		return;
	CTPSMModel* pNewModel=pPrj->AppendModel();
	pNewModel->SetBelongPrj(pPrj);
	pNewModel->m_xDesPara.m_ciTowerType=towerTemlDlg.m_ciCurTowerType;
	pNewModel->m_sTower=towerTemlDlg.m_sTowerName;
	pNewModel->m_sFullPath.Printf("%s\\%s.tpi",towerTemlDlg.m_sFilePath,(char*)towerTemlDlg.m_sTowerName);
	pNewModel->m_xTower.DisplayProcess=DisplayProcess;
	pNewModel->ReadTowerTempFile(towerTemlDlg.m_sTemplFile,TRUE);
	pNewModel->InitParaSketchItems();
	pNewModel->ReBuildTower();
	pNewModel->SaveTpiFile();
	pNewModel->m_nMaxLegs=CFGLEG::MaxLegs();
	//插入新的树节点
	m_treeCtrl.SetItemText(hSelItem,CXhChar50("%s(%d)",(char*)pPrj->m_sPrjName,pPrj->TowerNum()));
	HTREEITEM hItem=m_treeCtrl.InsertItem(pNewModel->m_sTower,IMG_TOWER_ITEM,IMG_TOWER_ITEM,hSelItem);
	TREEITEM_INFO *pSonItemInfo=m_xItemInfoList.append(TREEITEM_INFO(TREEITEM_INFO::INFO_TOWER_ITEM,(DWORD)pNewModel));
	m_treeCtrl.SetItemData(hItem,(DWORD)pSonItemInfo);
	m_treeCtrl.Expand(hSelItem,TVE_EXPAND);
	m_treeCtrl.SelectItem(hItem);
	//刷新塔型实体
	theApp.SetActiveDoc(pNewModel);
}
void CProjectTreeDlg::OnNewItem()
{
	HTREEITEM hSelItem=NULL;
	TREEITEM_INFO *pItemInfo=GetSelItemInfo(&hSelItem);
	if(pItemInfo==NULL)
		return;
	CLogErrorLife logErrLife;
	if(pItemInfo->itemType==TREEITEM_INFO::INFO_TOWERMODEL_SET)
	{	//新建塔型
		NewTowerModel(hSelItem);
	}
	else if(pItemInfo->itemType==TREEITEM_INFO::INFO_TAINSTANCE_SET)
	{	//新建塔型

	}
}
void CProjectTreeDlg::OnDelItem()
{
	HTREEITEM hSelItem=NULL;
	TREEITEM_INFO *pItemInfo=GetSelItemInfo(&hSelItem);
	if(pItemInfo==NULL)
		return;
	CLogErrorLife logErrLife;
	if(pItemInfo->itemType==TREEITEM_INFO::INFO_TOWER_ITEM)
	{
		CTPSMModel* pModel=(CTPSMModel*)pItemInfo->dwRefData;
		if(pModel->BelongPrj())
			pModel->BelongPrj()->DeleteModel(pModel);
		m_treeCtrl.DeleteItem(hSelItem);
		//
		theApp.DeleteDoc(pModel);
	}
}
void CProjectTreeDlg::OnUpdateItem()
{
	TREEITEM_INFO *pItemInfo=GetSelItemInfo();
	if(pItemInfo==NULL)
		return;
	CLogErrorLife logErrLife;
	if(pItemInfo->itemType==TREEITEM_INFO::INFO_TOWERMODEL_SET||
		pItemInfo->itemType==TREEITEM_INFO::INFO_TAINSTANCE_SET)
	{	//生成工程
		CTPSMProject* pPrj=(CTPSMProject*)pItemInfo->dwRefData;
		for(CTPSMModel* pModel=pPrj->EnumFirstModel();pModel;pModel=pPrj->EnumNextModel())
			pModel->SaveTpiFile();
	}
	else if(pItemInfo->itemType==TREEITEM_INFO::INFO_TOWER_ITEM)
	{	//重新生成塔型模型
		CTPSMModel* pTower=(CTPSMModel*)pItemInfo->dwRefData;
		if(pTower)
			pTower->SaveTpiFile();
	}
}
//进行塔身规划
void CProjectTreeDlg::OnPlanTrunkSegment()
{
	TREEITEM_INFO *pItemInfo=GetSelItemInfo();
	if(pItemInfo==NULL||pItemInfo->itemType!=TREEITEM_INFO::INFO_TOWER_ITEM)
		return;
	CTPSMModel* pModel=(CTPSMModel*)pItemInfo->dwRefData;
	if(pModel)
	{
		CHuGaoPlanDlg dlg;
		if(dlg.DoModal()!=IDOK)
			return;
		CTrunkBodyPlanner trunk;
		//填充当前呼高规划信息
		trunk.uiMinHeight=dlg.m_nMinHeight*1000;
		trunk.uiMaxHeight=dlg.m_nMaxHeight*1000;
		trunk.wnHeightGroup=dlg.m_nHeightGroup;
		trunk.wnMaxLegCount=dlg.m_nMaxLegCoun;
		trunk.wiLegDiffLevel=1000;
		trunk.InitTrunkInfo(pModel->m_xDesPara.m_ciTowerType);
		CHashListEx<HEIGHTGROUP> hashPlanHeights;
		CLogErrorLife life(&logerr);
		if(!trunk.UpdateTowerTrunk(&pModel->m_xTower,&hashPlanHeights))
			return;
		//刷新塔型实体
		CTPSMView* pActiveView=theApp.GetActiveView();
		if(pActiveView)
		{
			pActiveView->Refresh();
			((CMainFrame*)AfxGetMainWnd())->UpdatePropertyPage();
		}
	}
}
//导出指定文件
void CProjectTreeDlg::OnExportTidFiles()
{
	theApp.ExportTidData();
}
void CProjectTreeDlg::OnExportModFiles()
{
	theApp.ExportModData();
}
void CProjectTreeDlg::OnExportDxfFiles()
{
	theApp.ExportDxfData();
}
void CProjectTreeDlg::OnExport3dsFiles()
{
	theApp.Export3dsData();
}
void CProjectTreeDlg::OnExportLdsFiles()
{
	theApp.ExportLdsData();
}
