// TowerTreeDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "TMDH.h"
#include "GimPage.h"
#include "afxdialogex.h"
#include "XhCharString.h"
#include "folder_dialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////////
// CTowerTreeDlg 对话框
IMPLEMENT_DYNCREATE(CGimPageDlg, CDialogEx)

CGimPageDlg::CGimPageDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CGimPageDlg::IDD, pParent)
{

}

CGimPageDlg::~CGimPageDlg()
{
}

void CGimPageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE_CTRL, m_treeCtrl);
}


BEGIN_MESSAGE_MAP(CGimPageDlg, CDialogEx)
	ON_WM_SIZE()
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_CTRL, OnTvnSelchangedTreeCtrl)
	ON_NOTIFY(TVN_KEYDOWN, IDC_TREE_CTRL, OnTvnKeydownTreeCtrl)
	ON_NOTIFY(NM_RCLICK, IDC_TREE_CTRL, OnRclickTreeCtrl)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE_CTRL, OnNMDblclkTreeCtrl)
	ON_COMMAND(ID_ADD_ITEM,OnAddItem)
	ON_COMMAND(ID_DEL_ITEM,OnDelItem)
	ON_COMMAND(ID_UPDATE_ITEM,OnUpdateItem)
	ON_COMMAND(ID_TRANSFORM_DATA,OnTransData)
	ON_COMMAND(ID_PARSE_DATA,OnParseData)
	ON_COMMAND(ID_PACK_DATA,OnPackData)
	ON_COMMAND(ID_UNPACK_DATA,OnUnpackData)
END_MESSAGE_MAP()


// CTowerTreeDlg 消息处理程序
BOOL CGimPageDlg::OnInitDialog() 
{
	CDialogEx::OnInitDialog();
	//
	m_images.Create(IDB_IL_PROJECT, 16, 1, RGB(0,255,0));
	m_treeCtrl.SetImageList(&m_images,TVSIL_NORMAL);
	m_treeCtrl.ModifyStyle(0,TVS_HASLINES|TVS_HASBUTTONS|TVS_SHOWSELALWAYS|TVS_FULLROWSELECT);
	RefreshTreeCtrl();
	return TRUE;  
}
void CGimPageDlg::OnOK() 
{
}

void CGimPageDlg::OnCancel() 
{
}

void CGimPageDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialogEx::OnSize(nType, cx, cy);
	//
	if(m_treeCtrl.GetSafeHwnd())
		m_treeCtrl.MoveWindow(0,0,cx,cy);
}
void CGimPageDlg::RefreshTreeCtrl()
{
	m_xItemInfoList.Empty();
	TREEITEM_INFO *pItemInfo=NULL;
	m_treeCtrl.DeleteAllItems();
	m_hModSetItem=m_treeCtrl.InsertItem("MOD数据",IMG_MOD_ROOT,IMG_MOD_ROOT,TVI_ROOT);
	pItemInfo=m_xItemInfoList.append(TREEITEM_INFO(TREEITEM_INFO::INFO_MOD_SET,NULL));
	m_treeCtrl.SetItemData(m_hModSetItem,(DWORD)pItemInfo);
	m_treeCtrl.Expand(m_hModSetItem,TVE_EXPAND);
	//GIM数据
	m_hGimSetItem=m_treeCtrl.InsertItem("GIM数据",IMG_GIM_ROOT,IMG_GIM_ROOT,TVI_ROOT);
	pItemInfo=m_xItemInfoList.append(TREEITEM_INFO(TREEITEM_INFO::INFO_GIM_SET,NULL));
	m_treeCtrl.SetItemData(m_hGimSetItem,(DWORD)pItemInfo);
	m_treeCtrl.Expand(m_hGimSetItem,TVE_EXPAND);
}

TREEITEM_INFO *CGimPageDlg::GetSelItemInfo(HTREEITEM* pItem/*=NULL*/)
{
	HTREEITEM hSelItem=m_treeCtrl.GetSelectedItem();
	if(hSelItem==NULL)
		return NULL;
	TREEITEM_INFO *pItemInfo=(TREEITEM_INFO*)m_treeCtrl.GetItemData(hSelItem);
	if(pItem)
		*pItem=hSelItem;
	return pItemInfo;
}

void CGimPageDlg::ContextMenu(CWnd *pWnd, CPoint point)
{
	TREEITEM_INFO *pItemInfo=GetSelItemInfo();
	if(pItemInfo==NULL)
		return;
	CPoint scr_point = point;
	m_treeCtrl.ClientToScreen(&scr_point);
	CMenu menu;
	menu.CreatePopupMenu();
	if(pItemInfo->itemType==TREEITEM_INFO::INFO_MOD_SET)
	{
		menu.AppendMenu(MF_STRING,ID_ADD_ITEM,"添加MOD数据");
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING,ID_UPDATE_ITEM,"数据检测");
		menu.AppendMenu(MF_STRING,ID_TRANSFORM_DATA,"数据转换");
		menu.AppendMenu(MF_STRING,ID_PACK_DATA,"压缩打包");
	}
	else if(pItemInfo->itemType==TREEITEM_INFO::INFO_MOD)
	{
		menu.AppendMenu(MF_STRING,ID_DEL_ITEM,"删除");
	}
	else if(pItemInfo->itemType==TREEITEM_INFO::INFO_GIM_SET)
	{
		menu.AppendMenu(MF_STRING,ID_ADD_ITEM,"添加GIM包");
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
	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, scr_point.x, scr_point.y, this, NULL);
	menu.DestroyMenu();
}

void CGimPageDlg::AppendModFile(const char* sFilePath)
{
	CXhChar16 file_name;
	_splitpath(sFilePath,NULL,NULL,file_name,NULL);
	//
	CTMDHGim* pGim=Manager.AppendGim();
	pGim->m_sModName=file_name;
	pGim->m_sGimName=file_name;
	pGim->m_sModPath=sFilePath;
	pGim->m_sModFile=sFilePath;
	if(IsUTF8File(sFilePath))
		pGim->m_ciCodingType=CTMDHGim::UTF8_CODING;
	else
		pGim->m_ciCodingType=CTMDHGim::ANSI_CODING;
	//插入新的树节点
	HTREEITEM hItem=m_treeCtrl.InsertItem(pGim->m_sModName,IMG_MOD_ITEM,IMG_MOD_ITEM,m_hModSetItem);
	TREEITEM_INFO *pSonItemInfo=m_xItemInfoList.append(TREEITEM_INFO(TREEITEM_INFO::INFO_MOD,(DWORD)pGim));
	m_treeCtrl.SetItemData(hItem,(DWORD)pSonItemInfo);
	m_treeCtrl.SetItemText(m_hModSetItem,CXhChar50("MOD数据(%d)",Manager.GimNum()));
	m_treeCtrl.Expand(m_hModSetItem,TVE_EXPAND);
	//
	theApp.SetActiveDoc(pGim,CTMDHDoc::GIM_DATA);
}

void CGimPageDlg::AppendGimFile(const char* sFilePath)
{
	char drive[4];
	char dir[MAX_PATH],fname[MAX_PATH],folder[MAX_PATH];
	_splitpath(sFilePath,drive,dir,fname,NULL);
	strcpy(folder,drive);
	strcat(folder,dir);
	//
	CTMDHPrjGim* pPrjGim=Manager.AppendPrjGim();
	pPrjGim->m_sGimName=fname;
	pPrjGim->m_sGimPath=folder;
	m_sOutPath=folder;
	//插入新的树节点
	HTREEITEM hItem=m_treeCtrl.InsertItem(pPrjGim->m_sGimName,IMG_GIM_ITEM,IMG_GIM_ITEM,m_hGimSetItem);
	TREEITEM_INFO *pSonItemInfo=m_xItemInfoList.append(TREEITEM_INFO(TREEITEM_INFO::INFO_PRJ_GIM,(DWORD)pPrjGim));
	m_treeCtrl.SetItemData(hItem,(DWORD)pSonItemInfo);
	m_treeCtrl.SetItemText(m_hGimSetItem,CXhChar50("GIM数据(%d)",Manager.PrjGimNum()));
	m_treeCtrl.Expand(m_hGimSetItem,TVE_EXPAND);
}
//////////////////////////////////////////////////////////////////////////
void CGimPageDlg::OnRclickTreeCtrl(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TVHITTESTINFO HitTestInfo;
	GetCursorPos(&HitTestInfo.pt);
	m_treeCtrl.ScreenToClient(&HitTestInfo.pt);
	m_treeCtrl.HitTest(&HitTestInfo);
	m_treeCtrl.Select(HitTestInfo.hItem,TVGN_CARET);

	ContextMenu(this,HitTestInfo.pt);
	*pResult = 0;
}

void CGimPageDlg::OnNMDblclkTreeCtrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	HTREEITEM hSelItem;
	TREEITEM_INFO *pInfo=GetSelItemInfo(&hSelItem);
	if(pInfo==NULL)
		return;
	if(pInfo->itemType==TREEITEM_INFO::INFO_MOD||
		pInfo->itemType==TREEITEM_INFO::INFO_DEV_GIM)
	{
		CTMDHGim* pGim=(CTMDHGim*)pInfo->dwRefData;
		theApp.SetActiveDoc(pGim,CTMDHDoc::GIM_DATA);
	}
	*pResult = 0;
}

void CGimPageDlg::OnTvnSelchangedTreeCtrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	HTREEITEM hItem = m_treeCtrl.GetSelectedItem();
	TREEITEM_INFO *pInfo=(TREEITEM_INFO*)m_treeCtrl.GetItemData(hItem);
	if(pInfo==NULL)
		return;
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

void CGimPageDlg::OnTvnKeydownTreeCtrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTVKEYDOWN pTVKeyDown = reinterpret_cast<LPNMTVKEYDOWN>(pNMHDR);
	if(pTVKeyDown->wVKey==VK_DELETE)
		OnDelItem();
	*pResult = 0;
}

void CGimPageDlg::OnAddItem()
{
	HTREEITEM hSelItem=NULL;
	TREEITEM_INFO *pItemInfo=GetSelItemInfo(&hSelItem);
	if(pItemInfo==NULL)
		return;
	CLogErrorLife logErrLife(&xMyErrLog);
	if(pItemInfo->itemType==TREEITEM_INFO::INFO_MOD_SET)
	{
		CXhChar500 filter("批处理文件(*.xls)|*.xls");
		filter.Append("|批处理文件(*.xlsx)|*.xlsx");
		filter.Append("|模型文件(*.mod)|*.mod");
		filter.Append("|所有文件(*.*)|*.*||");
		CFileDialog dlg(TRUE,"塔型文件","塔型文件",OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,filter);
		if(dlg.DoModal()!=IDOK)
			return;
		CString sFileExt=dlg.GetFileExt();
		if(sFileExt.CompareNoCase("mod")==0||sFileExt.CompareNoCase("MOD")==0)
		{	//单个加载
			CTMDHGim* pGim=Manager.AppendGim();
			pGim->m_sModName=dlg.GetFileTitle();
			pGim->m_sGimName=pGim->m_sModName;
			pGim->m_sModPath=dlg.GetFolderPath();
			pGim->m_sModFile=dlg.GetPathName();
			if(IsUTF8File(dlg.GetPathName()))
				pGim->m_ciCodingType=CTMDHGim::UTF8_CODING;
			else
				pGim->m_ciCodingType=CTMDHGim::ANSI_CODING;
			//插入新的树节点
			HTREEITEM hItem=m_treeCtrl.InsertItem(pGim->m_sModName,IMG_MOD_ITEM,IMG_MOD_ITEM,hSelItem);
			TREEITEM_INFO *pSonItemInfo=m_xItemInfoList.append(TREEITEM_INFO(TREEITEM_INFO::INFO_MOD,(DWORD)pGim));
			m_treeCtrl.SetItemData(hItem,(DWORD)pSonItemInfo);	
		}
		else if(sFileExt.CompareNoCase("xls")==0||sFileExt.CompareNoCase("xlsx")==0)
		{	//批量加载
			if(Manager.ReadTaModBatchGzipFile(dlg.GetPathName()))
				m_treeCtrl.DeleteAllSonItems(hSelItem);
			for(CTMDHGim* pGim=Manager.EnumFirstGim();pGim;pGim=Manager.EnumNextGim())
			{
				if(m_sOutPath.GetLength()<=0)
					m_sOutPath=pGim->m_sOutputPath;
				//插入新的树节点
				HTREEITEM hItem=m_treeCtrl.InsertItem(pGim->m_sModName,IMG_MOD_ITEM,IMG_MOD_ITEM,hSelItem);
				TREEITEM_INFO *pSonItemInfo=m_xItemInfoList.append(TREEITEM_INFO(TREEITEM_INFO::INFO_MOD,(DWORD)pGim));
				m_treeCtrl.SetItemData(hItem,(DWORD)pSonItemInfo);	
			}
			m_treeCtrl.SetItemText(hSelItem,CXhChar50("MOD数据(%d)",Manager.GimNum()));
			m_treeCtrl.Expand(hSelItem,TVE_EXPAND);
		}
	}
	else if(pItemInfo->itemType==TREEITEM_INFO::INFO_GIM_SET)
	{
		CFileDialog dlg(TRUE,"GIM包","GIM包",OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
						"GIM文件(*.gim)|*.gim|所有文件(*.*)|*.*||");
		if(dlg.DoModal()!=IDOK)
			return;
		CTMDHPrjGim* pPrjGim=Manager.AppendPrjGim();
		pPrjGim->m_sGimName=dlg.GetFileTitle();
		pPrjGim->m_sGimPath=dlg.GetFolderPath();
		m_sOutPath=pPrjGim->m_sGimPath;
		//插入新的树节点
		HTREEITEM hItem=m_treeCtrl.InsertItem(pPrjGim->m_sGimName,IMG_GIM_ITEM,IMG_GIM_ITEM,hSelItem);
		TREEITEM_INFO *pSonItemInfo=m_xItemInfoList.append(TREEITEM_INFO(TREEITEM_INFO::INFO_PRJ_GIM,(DWORD)pPrjGim));
		m_treeCtrl.SetItemData(hItem,(DWORD)pSonItemInfo);
		m_treeCtrl.SetItemText(hSelItem,CXhChar50("GIM数据(%d)",Manager.PrjGimNum()));
		m_treeCtrl.Expand(hSelItem,TVE_EXPAND);
	}
}

void CGimPageDlg::OnDelItem()
{
	HTREEITEM hSelItem=NULL;
	TREEITEM_INFO *pItemInfo=GetSelItemInfo(&hSelItem);
	if(pItemInfo==NULL)
		return;
	if(pItemInfo->itemType==TREEITEM_INFO::INFO_MOD)
	{
		CTMDHGim* pGim=(CTMDHGim*)pItemInfo->dwRefData;
		if(pGim)
			Manager.DeleteGim(pGim);
		m_treeCtrl.DeleteItem(hSelItem);
		//
		theApp.DeleteDoc(pGim);
	}
	else if(pItemInfo->itemType==TREEITEM_INFO::INFO_PRJ_GIM)
	{
		CTMDHPrjGim* pPrjGim=(CTMDHPrjGim*)pItemInfo->dwRefData;
		if(pPrjGim)
			Manager.DeletePrjGim(pPrjGim);
		m_treeCtrl.DeleteItem(hSelItem);
	}
}
//MOD数据检测
void CGimPageDlg::OnUpdateItem()
{
	HTREEITEM hSelItem=NULL;
	TREEITEM_INFO *pItemInfo=GetSelItemInfo(&hSelItem);
	if(pItemInfo==NULL)
		return;
	if(pItemInfo->itemType==TREEITEM_INFO::INFO_MOD_SET)
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
				WinExec(CXhChar500("explorer.exe %s",m_sOutPath),SW_SHOW);
		}
	}
	else if(pItemInfo->itemType==TREEITEM_INFO::INFO_DEV_GIM)
	{
		CWaitCursor waitCursor;
		CTMDHGim* pGim=(CTMDHGim*)pItemInfo->dwRefData;
		if(pGim->UpdateGimTowerPro())
		{
			if(AfxMessageBox("GIM更新完成！是否打开输出路径？",MB_YESNO)==IDYES)
				WinExec(CXhChar500("explorer.exe %s",m_sOutPath),SW_SHOW);
		}
	}
}
//GIM包解析
void CGimPageDlg::OnParseData()
{
	HTREEITEM hSelItem=NULL;
	TREEITEM_INFO *pItemInfo=GetSelItemInfo(&hSelItem);
	if(pItemInfo==NULL)
		return;
	if(pItemInfo->itemType==TREEITEM_INFO::INFO_PRJ_GIM)
	{
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
}
//进行数据转换生成
void CGimPageDlg::OnTransData()
{
	HTREEITEM hSelItem=NULL;
	TREEITEM_INFO *pItemInfo=GetSelItemInfo(&hSelItem);
	if(pItemInfo==NULL||pItemInfo->itemType!=TREEITEM_INFO::INFO_MOD_SET)
		return;
	if(m_sOutPath.GetLength()<=0&&InvokeFolderPickerDlg(m_sOutPath))
	{
		for(CTMDHGim* pGim=Manager.EnumFirstGim();pGim;pGim=Manager.EnumNextGim())
			pGim->m_sOutputPath.Copy(m_sOutPath);
	}
	if(m_sOutPath.GetLength()<=0)
		return;
	for(CTMDHGim* pGim=Manager.EnumFirstGim();pGim;pGim=Manager.EnumNextGim())
		pGim->CreateGuidFile();
	if(AfxMessageBox("数据转换完成！是否打开输出路径？",MB_YESNO)==IDYES)
		WinExec(CXhChar500("explorer.exe %s",m_sOutPath),SW_SHOW);
}
//进行压缩打包
void CGimPageDlg::OnPackData()
{
	HTREEITEM hSelItem=NULL;
	TREEITEM_INFO *pItemInfo=GetSelItemInfo(&hSelItem);
	if(pItemInfo==NULL||pItemInfo->itemType!=TREEITEM_INFO::INFO_MOD_SET)
		return;
	if(m_sOutPath.GetLength()<=0&&InvokeFolderPickerDlg(m_sOutPath))
	{
		for(CTMDHGim* pGim=Manager.EnumFirstGim();pGim;pGim=Manager.EnumNextGim())
			pGim->m_sOutputPath.Copy(m_sOutPath);
	}
	if(m_sOutPath.GetLength()<=0)
		return;
	for(CTMDHGim* pGim=Manager.EnumFirstGim();pGim;pGim=Manager.EnumNextGim())
		pGim->DeleteGuidFile();
	for(CTMDHGim* pGim=Manager.EnumFirstGim();pGim;pGim=Manager.EnumNextGim())
		pGim->CreateGuidFile();
	for(CTMDHGim* pGim=Manager.EnumFirstGim();pGim;pGim=Manager.EnumNextGim())
		pGim->PackGimFile();
	for(CTMDHGim* pGim=Manager.EnumFirstGim();pGim;pGim=Manager.EnumNextGim())
		pGim->UpdateGimHeadInfo();
	if(AfxMessageBox("GIM打包完成！是否打开输出路径？",MB_YESNO)==IDYES)
		WinExec(CXhChar500("explorer.exe %s",m_sOutPath),SW_SHOW);
}
//
void CGimPageDlg::OnUnpackData()
{
	HTREEITEM hSelItem=NULL;
	TREEITEM_INFO *pItemInfo=GetSelItemInfo(&hSelItem);
	if(pItemInfo==NULL)
		return;
	CString sFolder=m_sOutPath;
	if(!InvokeFolderPickerDlg(sFolder))
		return;
	if(pItemInfo->itemType==TREEITEM_INFO::INFO_PRJ_GIM)
	{
		CWaitCursor waitCursor;
		CTMDHPrjGim* pPrjGim=(CTMDHPrjGim*)pItemInfo->dwRefData;
		pPrjGim->UnpackGimFile(sFolder);
	}
	if(AfxMessageBox("GIM解压完成！是否打开输出路径？",MB_YESNO)==IDYES)
		WinExec(CXhChar500("explorer.exe %s",sFolder),SW_SHOW);
}
