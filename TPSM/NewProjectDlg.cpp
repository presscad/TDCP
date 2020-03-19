// NewProjectDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "TPSM.h"
#include "NewProjectDlg.h"
#include "afxdialogex.h"
#include "folder_dialog.h"

// CNewProjectDlg 对话框

IMPLEMENT_DYNAMIC(CNewProjectDlg, CDialogEx)

CNewProjectDlg::CNewProjectDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CNewProjectDlg::IDD, pParent)
	, m_sPrjName(_T(""))
	, m_sPrjPath(_T(""))
	, m_sExcelPath(_T(""))
{

}

CNewProjectDlg::~CNewProjectDlg()
{
}

void CNewProjectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_E_PROJECT_NAME, m_sPrjName);
	DDX_Text(pDX, IDC_E_FILE_PATH, m_sPrjPath);
	DDX_Text(pDX, IDC_E_EXCEL_PATH, m_sExcelPath);
}


BEGIN_MESSAGE_MAP(CNewProjectDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_FILE_PATH, &CNewProjectDlg::OnBnClickedBtnFilePath)
	ON_BN_CLICKED(IDC_BTN_EXCEL_PATH, &CNewProjectDlg::OnBnClickedBtnExcelPath)
	ON_EN_CHANGE(IDC_E_PROJECT_NAME, &CNewProjectDlg::OnEnChangeEProjectName)
END_MESSAGE_MAP()


// CNewProjectDlg 消息处理程序
BOOL CNewProjectDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	return TRUE;  
}

extern void GetAppPath(char* startPath);
void CNewProjectDlg::OnBnClickedBtnFilePath()
{
	char ss[MAX_PATH];
	GetAppPath(ss);
	CString sFolder(ss);
	if(InvokeFolderPickerDlg(sFolder))
		m_sPrjPath=sFolder;
	UpdateData(FALSE);
}

void CNewProjectDlg::OnBnClickedBtnExcelPath()
{
	CFileDialog dlg(TRUE,"xls","工程信息表.xls",
		OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT|OFN_ALLOWMULTISELECT,
		"工程信息表(*.xls)|*.xls|工程信息表(*.xlsx)|*.xlsx|所有文件(*.*)|*.*||");
	if(dlg.DoModal()!=IDOK)
		return;
	m_sExcelPath=dlg.GetPathName();
	UpdateData(FALSE);
}


void CNewProjectDlg::OnEnChangeEProjectName()
{
	UpdateData();

	UpdateData(FALSE);
}
