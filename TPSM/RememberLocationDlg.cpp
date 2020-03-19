// RememberLocationDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "RememberLocationDlg.h"

// CRememberLocationDlg
CHashList<CPoint> CRememberLocationDlg::g_hashLocationPosByDlgID;
CRememberLocationDlg::CRememberLocationDlg(UINT nIDTemplate,CWnd* pParent/*=NULL*/)
	: CDialogEx(nIDTemplate, pParent)
{
	m_nIDTemplate=nIDTemplate;
	m_bEnableWindowMoveListen=false;
}

BEGIN_MESSAGE_MAP(CRememberLocationDlg, CDialogEx)
	ON_WM_MOVE()
	ON_WM_CREATE()
END_MESSAGE_MAP()

// CRememberLocationDlg ��Ϣ�������
int CRememberLocationDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	m_bEnableWindowMoveListen=false;
	if (CDialogEx::OnCreate(lpCreateStruct) == -1)
		return -1;
	return 0;
}
void CRememberLocationDlg::MoveWndToLocation()
{	//�ƶ��Ի������Ͻ�
	CPoint *pPoint=g_hashLocationPosByDlgID.GetValue(m_nIDTemplate);
	if(pPoint)
	{
		CRect rect;
		GetWindowRect(rect);
		int width = rect.Width();
		int height=rect.Height();
		rect.left=pPoint->x;
		rect.top=pPoint->y;
		rect.right = rect.left+width;
		rect.bottom = rect.top+height;
		MoveWindow(rect,TRUE);
	}
	m_bEnableWindowMoveListen=true;
}
void CRememberLocationDlg::OnMove(int x, int y)
{
	CDialogEx::OnMove(x, y);

	if(m_bEnableWindowMoveListen)
	{	//�����x,y������ָ�ͻ��������Ͻ�,�����Ǵ��ڵ����Ͻ� wjh-2016.10.22
		CRect rect;
		GetWindowRect(rect);
		CPoint *pPoint=g_hashLocationPosByDlgID.GetValue(m_nIDTemplate);
		if(pPoint==NULL)
			pPoint=g_hashLocationPosByDlgID.Add(m_nIDTemplate);
		if(pPoint)
		{
			pPoint->x=rect.left;//x;
			pPoint->y=rect.top;	//y;
		}
	}
}
