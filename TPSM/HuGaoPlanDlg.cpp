// HuGaoPlanDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "TPSM.h"
#include "HuGaoPlanDlg.h"
#include "afxdialogex.h"


// CHuGaoPlanDlg 对话框

IMPLEMENT_DYNAMIC(CHuGaoPlanDlg, CDialogEx)

CHuGaoPlanDlg::CHuGaoPlanDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CHuGaoPlanDlg::IDD, pParent)
{
	m_nMinHeight=30;
	m_nMaxHeight=54;
	m_nHeightGroup=9;
	m_nMaxLegCoun=8;
}

CHuGaoPlanDlg::~CHuGaoPlanDlg()
{
}

void CHuGaoPlanDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_E_MINHEIGHT, m_nMinHeight);
	DDX_Text(pDX, IDC_E_MAXHEIGHT, m_nMaxHeight);
	DDX_Text(pDX, IDC_E_HEIGHTGROUP, m_nHeightGroup);
	DDX_Text(pDX, IDC_E_MAXLEGCOUNT, m_nMaxLegCoun);
}


BEGIN_MESSAGE_MAP(CHuGaoPlanDlg, CDialogEx)
END_MESSAGE_MAP()


// CHuGaoPlanDlg 消息处理程序
void CHuGaoPlanDlg::OnOK()
{
	UpdateData();
	UINT uiVarHeight=m_nMaxHeight-m_nMinHeight;
	if(uiVarHeight%(m_nHeightGroup-1)!=0)
	{
		AfxMessageBox("规划呼高分组数与呼高规划范围不一致");
		return;
	}
	return CDialog::OnOK();
}