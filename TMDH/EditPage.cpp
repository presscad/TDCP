// EditPage.cpp : 实现文件
//

#include "stdafx.h"
#include "TMDH.h"
#include "EditPage.h"
#include "afxdialogex.h"


// CEditPageDlg 对话框

IMPLEMENT_DYNCREATE(CEditPageDlg, CDialogEx)

CEditPageDlg::CEditPageDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CEditPageDlg::IDD, pParent)
{

}

CEditPageDlg::~CEditPageDlg()
{
}

void CEditPageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_CTRL, m_editCtrl);
}


BEGIN_MESSAGE_MAP(CEditPageDlg, CDialogEx)
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CEditPageDlg 消息处理程序
BOOL CEditPageDlg::OnInitDialog() 
{
	CDialogEx::OnInitDialog();
	//
	
	return TRUE;  
}
void CEditPageDlg::OnOK() 
{
}

void CEditPageDlg::OnCancel() 
{
}

void CEditPageDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialogEx::OnSize(nType, cx, cy);
	//
	if(m_editCtrl.GetSafeHwnd())
		m_editCtrl.MoveWindow(0,0,cx,cy);
}
void CEditPageDlg::ShowText(CBuffer* pBuf,BOOL bUtf8)
{
	if(bUtf8)
	{	//UTF8ToUnicode
		int nLen=MultiByteToWideChar(CP_UTF8,0,pBuf->GetBufferPtr(),pBuf->GetLength(),NULL,0);
		wchar_t* sWText=new wchar_t[nLen+1];
		memset(sWText,0,(nLen+1)*sizeof(wchar_t));
		MultiByteToWideChar(CP_UTF8,0,pBuf->GetBufferPtr(),pBuf->GetLength(),sWText,nLen);
		sWText[nLen]='\0';
		//UnicodeToANSI
		nLen=WideCharToMultiByte(CP_ACP,0,sWText,wcslen(sWText), NULL,0,NULL,NULL);
		char* sText=new char[nLen+1];
		memset(sText,0,sizeof(char)*(nLen+1));
		WideCharToMultiByte(CP_ACP,0,sWText,wcslen(sWText),sText,nLen,NULL,NULL);
		sText[nLen]='\0';
		//
		m_editCtrl.SetWindowText(sText);
		delete sWText;
		delete sText;
	}
	else
	{
		int nLen=pBuf->GetLength();
		char* sText=new char[nLen+1];
		memset(sText,0,sizeof(char)*(nLen+1));
		memcpy(sText,pBuf->GetBufferPtr(),nLen);
		sText[nLen]='\0';
		//
		m_editCtrl.SetWindowText(sText);
		delete sText;
	}
	//
	m_editCtrl.SetModify(FALSE);
	m_editCtrl.SetReadOnly(TRUE);
}