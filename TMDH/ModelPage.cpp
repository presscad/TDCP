// ModelPage.cpp : 实现文件
//

#include "stdafx.h"
#include "TMDH.h"
#include "ModelPage.h"
#include "afxdialogex.h"
#include "ProcBarDlg.h"
#include "InstanceDlg.h"
#include "GlobalFunc.h"
//////////////////////////////////////////////////////////////////////////
// CModelPage 对话框
IMPLEMENT_DYNCREATE(CModelPageDlg, CDialogEx)

CModelPageDlg::CModelPageDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CModelPageDlg::IDD, pParent)
{
	m_iModelLevel=0;
	m_iLengthUnit=0;
}

CModelPageDlg::~CModelPageDlg()
{
}

void CModelPageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE_CTRL, m_treeCtrl);
	DDX_CBIndex(pDX, IDC_CMB_MODEL_LEVEL, m_iModelLevel);
	DDX_CBIndex(pDX, IDC_CMB_LENGTH_UNIT, m_iLengthUnit);
}


BEGIN_MESSAGE_MAP(CModelPageDlg, CDialogEx)
	ON_WM_SIZE()
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_CTRL, OnTvnSelchangedTreeCtrl)
	ON_NOTIFY(TVN_KEYDOWN, IDC_TREE_CTRL, OnTvnKeydownTreeCtrl)
	ON_NOTIFY(NM_RCLICK, IDC_TREE_CTRL, OnRclickTreeCtrl)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE_CTRL, OnNMDblclkTreeCtrl)
	ON_CBN_SELCHANGE(IDC_CMB_MODEL_LEVEL, OnCbnSelchangeCmbModelLevel)
	ON_CBN_SELCHANGE(IDC_CMB_LENGTH_UNIT, OnCbnSelchangeCmbLengthUnit)
	ON_COMMAND(ID_ADD_ITEM,OnAddItem)
	ON_COMMAND(ID_DEL_ITEM,OnDelItem)
	ON_COMMAND(ID_UPDATE_ITEM,OnUpdateItem)
	ON_COMMAND(ID_EXPORT_3DS,OnExport3dsFile)
	ON_COMMAND(ID_EXPORT_XML,OnExportXmlFile)
	ON_COMMAND(ID_EXPORT_MOD,OnExportModFile)
END_MESSAGE_MAP()


// CModelPage 消息处理程序
BOOL CModelPageDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	//
	CComboBox* pCmbUnit=(CComboBox*)GetDlgItem(IDC_CMB_LENGTH_UNIT);
	pCmbUnit->ResetContent();
	pCmbUnit->AddString("毫米(mm)");
	pCmbUnit->AddString("米(m)");
	pCmbUnit->SetCurSel(0);
	//更新精度等级
	CComboBox* pCmbLevel=(CComboBox*)GetDlgItem(IDC_CMB_MODEL_LEVEL);
	pCmbLevel->ResetContent();
	pCmbLevel->AddString("LOD0");
	pCmbLevel->AddString("LOD1");
	pCmbLevel->AddString("LOD2");
	pCmbLevel->AddString("LOD3");
	pCmbLevel->SetCurSel(0);
	//
	m_images.Create(IDB_IL_PROJECT, 16, 1, RGB(0,255,0));
	m_treeCtrl.SetImageList(&m_images,TVSIL_NORMAL);
	m_treeCtrl.ModifyStyle(0,TVS_HASLINES|TVS_HASBUTTONS|TVS_SHOWSELALWAYS|TVS_FULLROWSELECT);
	RefreshTreeCtrl();
	//
	UpdateData(FALSE);
	return TRUE;
}
void CModelPageDlg::OnOK()
{

}
void CModelPageDlg::OnCancel()
{

}

void CModelPageDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialogEx::OnSize(nType, cx, cy);
	//
	m_rcClient.bottom = cy;
	m_rcClient.right = cx;
	RelayoutDlg();
}

void CModelPageDlg::RelayoutDlg()
{
	RECT rcLevelS,rcLevelE,rcUnitS,rcUnitE,rcBtm;
	CWnd* pLevelSWnd=GetDlgItem(IDC_S_LEVEL);
	if(pLevelSWnd)
		pLevelSWnd->GetWindowRect(&rcLevelS);
	ScreenToClient(&rcLevelS);
	CWnd* pLevelEWnd=GetDlgItem(IDC_CMB_MODEL_LEVEL);
	if(pLevelEWnd)
		pLevelEWnd->GetWindowRect(&rcLevelE);
	ScreenToClient(&rcLevelE);
	CWnd* pUnitSWnd=GetDlgItem(IDC_S_UNIT);
	if(pUnitSWnd)
		pUnitSWnd->GetWindowRect(&rcUnitS);
	ScreenToClient(&rcUnitS);
	CWnd* pUnitEWnd=GetDlgItem(IDC_CMB_LENGTH_UNIT);
	if(pUnitEWnd)
		pUnitEWnd->GetWindowRect(&rcUnitE);
	ScreenToClient(&rcUnitE);
	int nStaticH=rcLevelS.bottom-rcLevelS.top;
	int nStaticL=rcLevelS.right-rcLevelS.left;
	int nCmbH=rcLevelE.bottom-rcLevelE.top;
	int nCmbL=rcLevelE.right-rcLevelE.left;
	int nTop=ftoi((nCmbH-nStaticH)*0.5);
	//
	rcLevelS.left=rcUnitS.left=5;
	rcLevelS.right=rcUnitS.right=rcUnitS.left+nStaticL;
	rcLevelS.top=nTop;
	rcLevelS.bottom=nTop+nStaticH;
	rcLevelE.left=rcUnitE.left=rcLevelS.right+4;
	rcLevelE.right=rcUnitE.right=rcLevelE.left+nCmbL;
	rcLevelE.top=0;
	rcLevelE.bottom=nCmbH;
	rcUnitS.top=nCmbH+nTop+4;
	rcUnitS.bottom=rcUnitS.top+nStaticH;
	rcUnitE.top=nCmbH+4;
	rcUnitE.bottom=rcUnitE.top+nCmbH;
	rcBtm.top=rcUnitE.bottom+5;
	rcBtm.bottom=m_rcClient.bottom;
	rcBtm.left=0;
	rcBtm.right=m_rcClient.right;
	if(pLevelSWnd->GetSafeHwnd())
		pLevelSWnd->MoveWindow(&rcLevelS);
	if(pLevelEWnd->GetSafeHwnd())
		pLevelEWnd->MoveWindow(&rcLevelE);
	if(pUnitSWnd->GetSafeHwnd())
		pUnitSWnd->MoveWindow(&rcUnitS);
	if(pUnitEWnd->GetSafeHwnd())
		pUnitEWnd->MoveWindow(&rcUnitE);
	if(m_treeCtrl.GetSafeHwnd())
		m_treeCtrl.MoveWindow(&rcBtm);
}

void CModelPageDlg::RefreshTreeCtrl()
{
	m_xItemInfoList.Empty();
	m_treeCtrl.DeleteAllItems();
	//杆塔数据层级
	HTREEITEM hItem=m_treeCtrl.InsertItem("模型数据",IMG_TOWER_ITEM,IMG_TOWER_ITEM,TVI_ROOT);
	TREEITEM_INFO *pItemInfo=m_xItemInfoList.append(TREEITEM_INFO(TREEITEM_INFO::INFO_MODEL_SET,NULL));
	m_treeCtrl.SetItemData(hItem,(DWORD)pItemInfo);
}

TREEITEM_INFO *CModelPageDlg::GetSelItemInfo(HTREEITEM* pItem/*=NULL*/)
{
	HTREEITEM hSelItem=m_treeCtrl.GetSelectedItem();
	if(hSelItem==NULL)
		return NULL;
	TREEITEM_INFO *pItemInfo=(TREEITEM_INFO*)m_treeCtrl.GetItemData(hSelItem);
	if(pItem)
		*pItem=hSelItem;
	return pItemInfo;
}

void CModelPageDlg::ContextMenu(CWnd *pWnd, CPoint point)
{
	TREEITEM_INFO *pItemInfo=GetSelItemInfo();
	if(pItemInfo==NULL)
		return;
	CPoint scr_point = point;
	m_treeCtrl.ClientToScreen(&scr_point);
	CMenu menu;
	menu.CreatePopupMenu();
	if(pItemInfo->itemType==TREEITEM_INFO::INFO_MODEL_SET)
	{
		menu.AppendMenu(MF_STRING,ID_ADD_ITEM,"添加塔型");
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING,ID_EXPORT_3DS,"生成3DS文件");
		menu.AppendMenu(MF_STRING,ID_EXPORT_MOD,"生成MOD文件");
		menu.AppendMenu(MF_STRING,ID_EXPORT_XML,"生成XML文件");
	}
	else if(pItemInfo->itemType==TREEITEM_INFO::INFO_MODEL)
	{
		menu.AppendMenu(MF_STRING,ID_UPDATE_ITEM,"配基属性");
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING,ID_DEL_ITEM,"删除塔型");
	}
	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, scr_point.x, scr_point.y, this, NULL);
	menu.DestroyMenu();
}

void CModelPageDlg::AppendTidFile(const char* sFilePath)
{
	CXhChar16 file_name;
	_splitpath(sFilePath,NULL,NULL,file_name,NULL);
	//
	m_sFileName=sFilePath;
	CTMDHModel* pModel=Manager.AppendModel();
	pModel->m_sTowerType=file_name;
	pModel->m_ciModelFlag=CTMDHModel::TYPE_TID;
	pModel->m_sFilePath.Copy(sFilePath);
	if(Manager.m_pTidModel==NULL)
		Manager.m_pTidModel=CTidModelFactory::CreateTidModel();
	//插入新的树节点
	HTREEITEM hItem=m_treeCtrl.InsertItem(pModel->m_sTowerType,IMG_TOWER_ITEM,IMG_TOWER_ITEM,m_hRootItem);
	TREEITEM_INFO *pSonItemInfo=m_xItemInfoList.append(TREEITEM_INFO(TREEITEM_INFO::INFO_MODEL,(DWORD)pModel));
	m_treeCtrl.SetItemData(hItem,(DWORD)pSonItemInfo);
	m_treeCtrl.SetItemText(m_hRootItem,CXhChar50("模型数据(%d)",Manager.ModelNum()));
	m_treeCtrl.Expand(m_hRootItem,TVE_EXPAND);
	//
	theApp.SetActiveDoc(pModel,CTMDHDoc::TMD_DATA);
}
//////////////////////////////////////////////////////////////////////////
void CModelPageDlg::OnRclickTreeCtrl(NMHDR* pNMHDR, LRESULT* pResult) 
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

void CModelPageDlg::OnNMDblclkTreeCtrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	TREEITEM_INFO *pInfo=GetSelItemInfo();
	if(pInfo!=NULL && pInfo->itemType==TREEITEM_INFO::INFO_MODEL)
	{
		CTMDHModel* pModel=(CTMDHModel*)pInfo->dwRefData;
		theApp.SetActiveDoc(pModel,CTMDHDoc::TMD_DATA);
	}
	*pResult = 0;
}

void CModelPageDlg::OnTvnSelchangedTreeCtrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	HTREEITEM hItem = m_treeCtrl.GetSelectedItem();
	TREEITEM_INFO *pInfo=(TREEITEM_INFO*)m_treeCtrl.GetItemData(hItem);

	*pResult = 0;
}

void CModelPageDlg::OnTvnKeydownTreeCtrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTVKEYDOWN pTVKeyDown = reinterpret_cast<LPNMTVKEYDOWN>(pNMHDR);
	if(pTVKeyDown->wVKey==VK_DELETE)
		OnDelItem();
	*pResult = 0;
}

void CModelPageDlg::OnCbnSelchangeCmbModelLevel()
{
	UpdateData(TRUE);
	Manager.m_ciInstanceLevel=m_iModelLevel;
	UpdateData(FALSE);
}

void CModelPageDlg::OnCbnSelchangeCmbLengthUnit()
{
	UpdateData(TRUE);
	if(m_iLengthUnit==0)	//毫米
		Manager.m_bUseUnitM=FALSE;
	else					//米
		Manager.m_bUseUnitM=TRUE;
	UpdateData(FALSE);
}

void CModelPageDlg::OnAddItem()
{
	HTREEITEM hSelItem=NULL;
	TREEITEM_INFO *pItemInfo=GetSelItemInfo(&hSelItem);
	if(pItemInfo==NULL)
		return;
	CLogErrorLife logErrLife;
	if(pItemInfo->itemType==TREEITEM_INFO::INFO_MODEL_SET)
	{
		CXhChar500 filter("铁塔三维数据模型文件(*.tid)|*.tid");
		filter.Append("|国网移交几何模型文件(*.mod)|*.mod");
		filter.Append("|批量生成文件(*.xls)|*.xls|批量生成文件(*.xlsx)|*.xlsx");
		filter.Append("|所有文件(*.*)|*.*||");
		CFileDialog file_dlg(TRUE,"模型文件","模型文件",OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,filter);
		if(file_dlg.DoModal()!=IDOK)
			return;
		m_sFileName=file_dlg.GetPathName();
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
			HTREEITEM hItem=m_treeCtrl.InsertItem(pModel->m_sTowerType,IMG_TOWER_ITEM,IMG_TOWER_ITEM,hSelItem);
			TREEITEM_INFO *pSonItemInfo=m_xItemInfoList.append(TREEITEM_INFO(TREEITEM_INFO::INFO_MODEL,(DWORD)pModel));
			m_treeCtrl.SetItemData(hItem,(DWORD)pSonItemInfo);
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
			HTREEITEM hItem=m_treeCtrl.InsertItem(pModel->m_sTowerType,IMG_TOWER_ITEM,IMG_TOWER_ITEM,hSelItem);
			TREEITEM_INFO *pSonItemInfo=m_xItemInfoList.append(TREEITEM_INFO(TREEITEM_INFO::INFO_MODEL,(DWORD)pModel));
			m_treeCtrl.SetItemData(hItem,(DWORD)pSonItemInfo);
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
				HTREEITEM hItem=m_treeCtrl.InsertItem(sName,IMG_TOWER_ITEM,IMG_TOWER_ITEM,hSelItem);
				TREEITEM_INFO *pSonItemInfo=m_xItemInfoList.append(TREEITEM_INFO(TREEITEM_INFO::INFO_MODEL,(DWORD)pModel));
				m_treeCtrl.SetItemData(hItem,(DWORD)pSonItemInfo);
			}
		}
		//
		m_treeCtrl.SetItemText(hSelItem,CXhChar50("模型数据(%d)",Manager.ModelNum()));
		m_treeCtrl.Expand(hSelItem,TVE_EXPAND);
	}
}

void CModelPageDlg::OnDelItem()
{
	HTREEITEM hSelItem=NULL;
	TREEITEM_INFO *pItemInfo=GetSelItemInfo(&hSelItem);
	if(pItemInfo==NULL)
		return;
	if(pItemInfo->itemType==TREEITEM_INFO::INFO_MODEL)
	{
		CTMDHModel* pModel=(CTMDHModel*)pItemInfo->dwRefData;
		if(pModel)
			Manager.DeleteModel(pModel);
		m_treeCtrl.DeleteItem(hSelItem);
		//
		theApp.DeleteDoc(pModel);
	}
}

void CModelPageDlg::OnUpdateItem()
{
	HTREEITEM hSelItem=NULL;
	TREEITEM_INFO *pItemInfo=GetSelItemInfo(&hSelItem);
	if(pItemInfo==NULL)
		return;
	if(pItemInfo->itemType==TREEITEM_INFO::INFO_MODEL)
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
}

void CModelPageDlg::OnExport3dsFile()
{
	int index=m_sFileName.ReverseFind('\\');	//反向查找'\\'
	CString sFilePath=m_sFileName.Left(index);	//移除文件名
	CXhChar100 sWorkDir;
	sWorkDir.Printf("%s\\3DS",sFilePath);
	MakeDirectory(sWorkDir);
	DisplayProcess(0,"生成3DSL文件");
	int i=1,nModel=Manager.ModelNum();
	for(CTMDHModel* pModel=Manager.EnumFirstModel();pModel;pModel=Manager.EnumNextModel(),i++)
	{
		DisplayProcess(int(100*i/nModel),"生成3DS文件");
		if(pModel->m_ciErrorType>0)
			continue;
		void* pInstance=pModel->ExtractActiveTaInstance();
		if(pInstance==NULL)
			continue;
		CXhChar200 sFullPath("%s\\%s.3ds",(char*)sWorkDir,(char*)pModel->GetFileName(pInstance));
		if(Manager.m_ciInstanceLevel==0)
			sFullPath.Printf("%s\\%s.dxf",(char*)sWorkDir,(char*)pModel->GetFileName(pInstance));
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
	if(sWorkDir.GetLength()>0)
		WinExec(CXhChar500("explorer.exe %s",(char*)sWorkDir),SW_SHOW);
}

void CModelPageDlg::OnExportXmlFile()
{
	int index=m_sFileName.ReverseFind('\\');	//反向查找'\\'
	CString sFilePath=m_sFileName.Left(index);	//移除文件名
	CXhChar100 sWorkDir;
	sWorkDir.Printf("%s\\XML",sFilePath);
	MakeDirectory(sWorkDir);
	DisplayProcess(0,"生成XML文件");
	int i=1,nModel=Manager.ModelNum();
	for(CTMDHModel* pModel=Manager.EnumFirstModel();pModel;pModel=Manager.EnumNextModel(),i++)
	{
		DisplayProcess(int(100*i/nModel),"生成XML文件");
		if(pModel->m_ciErrorType>0)
			continue;
		void* pInstance=pModel->ExtractActiveTaInstance();
		if(pInstance==NULL)
			continue;
		CXhChar200 sFullPath("%s\\%s.xml",(char*)sWorkDir,(char*)pModel->GetFileName(pInstance));
		if(pModel->m_ciModelFlag==CTMDHModel::TYPE_TID)	//TID模型
			pModel->CreateXmlFile((ITidTowerInstance*)pInstance,sFullPath);
		if(pModel->m_ciModelFlag==CTMDHModel::TYPE_MOD)	//MOD模型
			pModel->CreateXmlFile((IModTowerInstance*)pInstance,sFullPath);
	}
	DisplayProcess(100,"生成XML文件");
	if(sWorkDir.GetLength()>0)
		WinExec(CXhChar500("explorer.exe %s",(char*)sWorkDir),SW_SHOW);
}

void CModelPageDlg::OnExportModFile()
{
	int index=m_sFileName.ReverseFind('\\');	//反向查找'\\'
	CString sFilePath=m_sFileName.Left(index);	//移除文件名
	CXhChar100 sWorkDir;
	sWorkDir.Printf("%s\\MOD",sFilePath);
	MakeDirectory(sWorkDir);
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
		CXhChar200 sFullPath("%s\\%s.mod",(char*)sWorkDir,(char*)pModel->GetFileName(pInstance));
		if(pModel->m_ciModelFlag==CTMDHModel::TYPE_TID)	//TID模型
			pModel->CreateModFile((ITidTowerInstance*)pInstance,sFullPath);
		if(pModel->m_ciModelFlag==CTMDHModel::TYPE_MOD)	//MOD模型
			pModel->CreateModFile((IModTowerInstance*)pInstance,sFullPath);
	}
	DisplayProcess(100,"生成MOD文件");
	if(sWorkDir.GetLength()>0)
		WinExec(CXhChar500("explorer.exe %s",(char*)sWorkDir),SW_SHOW);
}
