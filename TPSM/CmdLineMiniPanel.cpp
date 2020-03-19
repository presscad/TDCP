// CmdLineMiniPanel.cpp : 实现文件
//

#include "stdafx.h"
#include "CmdLineMiniPanel.h"
#include "CmdLineDlg.h"


// CCmdLineMiniPanel 对话框

CCmdLineMiniPanel::CCmdLineMiniPanel(UINT nIDTemplate,CWnd* pParent /*=NULL*/)
	: CDialogEx(nIDTemplate, pParent)
{
	m_pCmdLineDlg=NULL;
}
CCmdLineMiniPanel::~CCmdLineMiniPanel()
{
	if(m_pCmdLineDlg)
		m_pCmdLineDlg->DetachMiniCmdOptionPanel();
	m_pCmdLineDlg=NULL;
}
