// TowerTemplateDlg.cpp : 实现文件
//
#include "stdafx.h"
#include "TPSM.h"
#include "TowerTemplateDlg.h"
#include "afxdialogex.h"
#include "TowerTypeTemp.h"
#include "PropertyListOper.h"
#include "GlobalFunc.h"
#include "folder_dialog.h"
//////////////////////////////////////////////////////////////////////////
// CTowerTemplateDlg 对话框

IMPLEMENT_DYNAMIC(CTowerTemplateDlg, CRememberLocationDlg)

CTowerTemplateDlg::CTowerTemplateDlg(CWnd* pParent /*=NULL*/)
	: CRememberLocationDlg(CTowerTemplateDlg::IDD, pParent)
	, m_sFilePath(_T(""))
{
	m_ciCurTowerType=1;
	m_nType=1;
	m_dwIconWidth=110;
	m_dwIconHeight=150;
	//
	imageList.Create(m_dwIconWidth,m_dwIconHeight,ILC_COLOR32,2,4);
}

CTowerTemplateDlg::~CTowerTemplateDlg()
{
}

void CTowerTemplateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_CTRL, m_xIconListCtrl);
	DDX_Text(pDX, IDC_E_FILE_PATH, m_sFilePath);
}


BEGIN_MESSAGE_MAP(CTowerTemplateDlg, CRememberLocationDlg)
	ON_WM_PAINT()
	ON_CBN_SELCHANGE(IDC_CMB_TYPE, OnCbnSelchangeCmbType)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_CTRL, OnItemchangedListCtrl)
	ON_BN_CLICKED(IDC_BTN_FILE_PATH, OnBnClickedBtnFilePath)
END_MESSAGE_MAP()


// CTowerTemplateDlg 消息处理程序
BOOL CTowerTemplateDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	//
	if(m_sFilePath.GetLength()<=0)
		m_sFilePath=theApp.execute_path;
	CComboBox* pTypeCmb=(CComboBox*)GetDlgItem(IDC_CMB_TYPE);
	pTypeCmb->AddString("交流双回路");
	pTypeCmb->AddString("交流单回路");
	pTypeCmb->AddString("直流单回路");
	//pTypeCmb->AddString("紧凑型");
	pTypeCmb->SetCurSel(0);
	//初始化ListCtrol
	m_xIconListCtrl.SetExtendedStyle(LVS_ICON);
	m_xIconListCtrl.SetBkColor(RGB(255,255,255));
	m_xIconListCtrl.SetImageList(&imageList,LVSIL_NORMAL);
	m_xIconListCtrl.SetIconSpacing(CSize(m_dwIconWidth+7,m_dwIconHeight+10));	//设置图片间隔
	m_xIconListCtrl.Arrange(LVA_ALIGNLEFT);
	InitImageList();
	RefreshImageLictCtr();
	//移动对话框到左上角
	MoveWndToLocation();
	//
	UpdateData(FALSE);
	return TRUE;
}


void CTowerTemplateDlg::InitImageList()
{
	CXhChar200 sys_path,image_file;
	GetSysPath(sys_path);
	m_xImageRecList.Empty();
	if(m_nType==1)
	{	//交流双回路
		image_file.Printf("%sSketch_SZC.bmp",(char*)sys_path);
		m_xImageRecList.append(CImageRecord(image_file,image_file,"SZC"));
		image_file.Printf("%sSketch_SZ.bmp",(char*)sys_path);
		m_xImageRecList.append(CImageRecord(image_file,image_file,"SZ"));
		image_file.Printf("%sSketch_SJ.bmp",(char*)sys_path);
		m_xImageRecList.append(CImageRecord(image_file,image_file,"SJ"));
		image_file.Printf("%sSketch_SJB.bmp",(char*)sys_path);
		m_xImageRecList.append(CImageRecord(image_file,image_file,"SJB"));
		image_file.Printf("%sSketch_SJC.bmp",(char*)sys_path);
		m_xImageRecList.append(CImageRecord(image_file,image_file,"SJC"));
	}
	else if(m_nType==2)
	{	//交流单回路
		image_file.Printf("%sSketch_ZBC.bmp",(char*)sys_path);
		m_xImageRecList.append(CImageRecord(image_file,image_file,"ZBC"));
		image_file.Printf("%sSketch_ZM.bmp",(char*)sys_path);
		m_xImageRecList.append(CImageRecord(image_file,image_file,"ZM"));
		image_file.Printf("%sSketch_JG.bmp",(char*)sys_path);
		m_xImageRecList.append(CImageRecord(image_file,image_file,"JG1"));
		image_file.Printf("%sSketch_JG2.bmp",(char*)sys_path);
		m_xImageRecList.append(CImageRecord(image_file,image_file,"JG2"));
		image_file.Printf("%sSketch_JBB.bmp", (char*)sys_path);
		m_xImageRecList.append(CImageRecord(image_file, image_file, "JBB"));
	}
	else if(m_nType==3)
	{	//直流单回路
		image_file.Printf("%sSketch_ZC.bmp",(char*)sys_path);
		m_xImageRecList.append(CImageRecord(image_file,image_file,"ZC"));
		image_file.Printf("%sSketch_JC.bmp",(char*)sys_path);
		m_xImageRecList.append(CImageRecord(image_file,image_file,"JC"));
	}
	else if(m_nType==4)
	{	//紧凑型

	}
}

void CTowerTemplateDlg::RefreshImageLictCtr()
{
	while(imageList.GetImageCount()>0)
		imageList.Remove(0);		//清空图标
	m_xIconListCtrl.DeleteAllItems();
	CBitmap bitmap;
	for(CImageRecord* pImageRec=m_xImageRecList.GetFirst();pImageRec;pImageRec=m_xImageRecList.GetNext())
	{
		if(pImageRec->GetSketchBitMap(bitmap)==FALSE)
			continue;
		int index=imageList.Add(&bitmap,RGB(0,0,0));
		m_xIconListCtrl.InsertItem(m_xIconListCtrl.GetItemCount(),pImageRec->GetLabelStr(),index);
	}
	//设置默认选中项
	if(m_xIconListCtrl.GetItemCount()>0)
	{
		m_xIconListCtrl.SetSelectionMark(0);
		m_xIconListCtrl.SetItemState(0,LVIS_SELECTED|LVNI_FOCUSED,LVIS_SELECTED|LVNI_FOCUSED);
	}
}

void CTowerTemplateDlg::OnOK()
{
	CXhChar200 sLdsFile;
	if(!GetLdsPath(sLdsFile))
	{
		AfxMessageBox("");
		return CDialogEx::OnCancel();
	}
	if (m_ciCurTowerType == TYPE_SZC)
		m_sTowerName.Copy("SZC1");
	else if (m_ciCurTowerType == TYPE_JG)
		m_sTowerName.Copy("JG1");
	else if (m_ciCurTowerType == TYPE_JG2)
		m_sTowerName.Copy("JG2");
	else if (m_ciCurTowerType == TYPE_SJ)
		m_sTowerName.Copy("SJ1");
	else if (m_ciCurTowerType == TYPE_SJB)
		m_sTowerName.Copy("SJB1");
	else if (m_ciCurTowerType == TYPE_SJC)
		m_sTowerName.Copy("SJC1");
	else if (m_ciCurTowerType == TYPE_ZBC)
		m_sTowerName.Copy("ZBC1");
	else if (m_ciCurTowerType == TYPE_ZC)
		m_sTowerName.Copy("ZC1");
	else if (m_ciCurTowerType == TYPE_ZM)
		m_sTowerName.Copy("ZM1");
	else if (m_ciCurTowerType == TYPE_SZ)
		m_sTowerName.Copy("SZ1");
	else if (m_ciCurTowerType == TYPE_JC)
		m_sTowerName.Copy("JC1");
	else if (m_ciCurTowerType == TYPE_JBB)
		m_sTowerName.Copy("JBB");
	m_sTemplFile.Printf("%s%s",(char*)sLdsFile,(char*)GetTempLdsFile(m_ciCurTowerType));
	return CDialogEx::OnOK();
}

void CTowerTemplateDlg::OnPaint()
{
	CPaintDC dc(this);
}

void CTowerTemplateDlg::OnItemchangedListCtrl(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	POSITION pos=m_xIconListCtrl.GetFirstSelectedItemPosition();
	int iCurSel=m_xIconListCtrl.GetNextSelectedItem(pos);
	CXhChar50 sLabel;
	m_xIconListCtrl.GetItemText(iCurSel,0,sLabel,50);
	if (sLabel.Equal("SZC"))
		m_ciCurTowerType = TYPE_SZC;
	else if (sLabel.Equal("SZ"))
		m_ciCurTowerType = TYPE_SZ;
	else if (sLabel.Equal("JG1"))
		m_ciCurTowerType = TYPE_JG;
	else if (sLabel.Equal("JG2"))
		m_ciCurTowerType = TYPE_JG2;
	else if (sLabel.Equal("SJ"))
		m_ciCurTowerType = TYPE_SJ;
	else if (sLabel.Equal("SJB"))
		m_ciCurTowerType = TYPE_SJB;
	else if (sLabel.Equal("SJC"))
		m_ciCurTowerType = TYPE_SJC;
	else if (sLabel.Equal("ZBC"))
		m_ciCurTowerType = TYPE_ZBC;
	else if (sLabel.Equal("ZC"))
		m_ciCurTowerType = TYPE_ZC;
	else if (sLabel.Equal("ZM"))
		m_ciCurTowerType = TYPE_ZM;
	else if (sLabel.Equal("SA"))
		m_ciCurTowerType = TYPE_SZ;
	else if (sLabel.Equal("JC"))
		m_ciCurTowerType = TYPE_JC;
	else if (sLabel.Equal("JBB"))
		m_ciCurTowerType = TYPE_JBB;
	//
	*pResult = 0;
}

void CTowerTemplateDlg::OnCbnSelchangeCmbType()
{
	UpdateData();
	CComboBox* pCmb=(CComboBox*)GetDlgItem(IDC_CMB_TYPE);
	int iSelIndex=pCmb->GetCurSel();
	m_nType=iSelIndex+1;
	//
	InitImageList();
	RefreshImageLictCtr();
	UpdateData(FALSE);
}
void CTowerTemplateDlg::OnBnClickedBtnFilePath()
{
	CString sFolder=m_sFilePath;
	if(InvokeFolderPickerDlg(sFolder))
		m_sFilePath=sFolder;
	UpdateData(FALSE);
}
