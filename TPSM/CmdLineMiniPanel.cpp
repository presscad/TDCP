// CmdLineMiniPanel.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "CmdLineMiniPanel.h"
#include "CmdLineDlg.h"


// CCmdLineMiniPanel �Ի���

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
