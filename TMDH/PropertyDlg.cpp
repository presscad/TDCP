// TowerPropertyDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TMDH.h"
#include "PropertyDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPartPropertyDlg dialog
IMPLEMENT_DYNCREATE(CPropertyDlg, CDialogEx)

CPropertyDlg::CPropertyDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CPropertyDlg::IDD, pParent)
{
	m_nSplitterWidth=5;
}


void CPropertyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPartPropertyDlg)
	DDX_Control(pDX, IDC_TAB_GROUP, m_xTabCtrl);
	DDX_Control(pDX, IDC_LIST_BOX, m_xPropList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPropertyDlg, CDialogEx)
	//{{AFX_MSG_MAP(CPartPropertyDlg)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_GROUP, OnSelchangeTabGroup)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPartPropertyDlg message handlers
BOOL CPropertyDlg::OnInitDialog() 
{
	CDialogEx::OnInitDialog();
	//
	GetClientRect(&m_rcClient);
	CWnd *pWnd=GetDlgItem(IDC_E_PROP_HELP_STR);
	m_xPropList.m_hPromptWnd=pWnd->GetSafeHwnd();
	//
	RelayoutDlg();
	RefreshTabCtrl(0);
	m_bTracking = FALSE;
	m_hCursorSize = AfxGetApp()->LoadStandardCursor(IDC_SIZENS);
	m_hCursorArrow = AfxGetApp()->LoadStandardCursor(IDC_ARROW);
	return TRUE; 
}

BOOL CPropertyDlg::PreTranslateMessage(MSG* pMsg) 
{
	return CDialogEx::PreTranslateMessage(pMsg);
}

void CPropertyDlg::OnOK() 
{
}

void CPropertyDlg::OnCancel() 
{
}

void CPropertyDlg::RelayoutDlg()
{
	RECT rcHeader,rcTop,rcBtm;
	CWnd *pBtmWnd = GetDlgItem(IDC_E_PROP_HELP_STR);
	if(pBtmWnd)
		pBtmWnd->GetWindowRect(&rcBtm);
	ScreenToClient(&rcBtm);
	int btmHeight=rcBtm.bottom-rcBtm.top;
	rcHeader.left =rcTop.left =rcBtm.left = 0;
	rcHeader.right=rcTop.right=rcBtm.right=m_rcClient.right;
	rcHeader.top=0;
	rcHeader.bottom=(m_sTabLabelArr.GetSize()<=0)?0:21;
	rcTop.top=rcHeader.bottom;
	rcTop.bottom=m_rcClient.bottom-btmHeight-m_nSplitterWidth;
	rcBtm.top=rcTop.bottom+m_nSplitterWidth;
	rcBtm.bottom=m_rcClient.bottom;
	m_nOldHorzY=rcBtm.top-m_nSplitterWidth/2;
	if(m_xTabCtrl.GetSafeHwnd())
		m_xTabCtrl.MoveWindow(&rcHeader);
	if(m_xPropList.GetSafeHwnd())
		m_xPropList.MoveWindow(&rcTop);
	if(pBtmWnd)
		pBtmWnd->MoveWindow(&rcBtm);
}

void CPropertyDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialogEx::OnSize(nType, cx, cy);
	m_rcClient.bottom = cy;
	m_rcClient.right = cx;
	RelayoutDlg();	
}

void CPropertyDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	RECT rcFull,rcHelpStr,rcSplitter;
	GetClientRect(&rcFull);
	CWnd *pWnd = GetDlgItem(IDC_E_PROP_HELP_STR);
	pWnd->GetWindowRect(&rcHelpStr);
	ScreenToClient(&rcHelpStr);
	rcSplitter.left = rcFull.left;
	int width = 4;
	rcSplitter.top = rcHelpStr.top-width;
	rcSplitter.right = rcFull.right;
	rcSplitter.bottom = rcHelpStr.top-1;
	CPen psPen(PS_SOLID, 1, RGB(120,120,120));
	CPen* pOldPen = dc.SelectObject(&psPen);
	dc.MoveTo(rcSplitter.left,rcSplitter.top);
	dc.LineTo(rcSplitter.right,rcSplitter.top);
	dc.MoveTo(rcSplitter.left,rcSplitter.bottom);
	dc.LineTo(rcSplitter.right,rcSplitter.bottom);
	dc.SelectObject(pOldPen);
	psPen.DeleteObject();
	// TODO: Add your message handler code here
	
	// Do not call CDialogEx::OnPaint() for painting messages
}

void CPropertyDlg::InvertLine(CDC* pDC,CPoint ptFrom,CPoint ptTo)
{
	int nOldMode = pDC->SetROP2(R2_NOT);
	
	pDC->MoveTo(ptFrom);
	pDC->LineTo(ptTo);

	pDC->SetROP2(nOldMode);
}

void CPropertyDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	if ((point.y>=m_nOldHorzY-5) && (point.y<=m_nOldHorzY+5))
	{
		//if mouse clicked on divider line, then start resizing

		::SetCursor(m_hCursorSize);

		CRect windowRect;
		GetWindowRect(windowRect);
		windowRect.left += 10; windowRect.right -= 10;
		//do not let mouse leave the dialog boundary
		::ClipCursor(windowRect);
		m_nOldHorzY = point.y;

		CClientDC dc(this);
		InvertLine(&dc,CPoint(m_rcClient.left,m_nOldHorzY),CPoint(m_rcClient.right,m_nOldHorzY));

		//capture the mouse
		SetCapture();
		m_bTracking = TRUE;
	}
	else
	{
		m_bTracking = FALSE;
		CDialogEx::OnLButtonDown(nFlags, point);
	}
}

void CPropertyDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (m_bTracking)
	{
		m_bTracking = FALSE;
		//if mouse was captured then release it
		if (GetCapture()==this)
			::ReleaseCapture();

		::ClipCursor(NULL);

		CClientDC dc(this);
		InvertLine(&dc,CPoint(m_rcClient.left,point.y),CPoint(m_rcClient.right,point.y));
		//set the divider position to the new value
		m_nOldHorzY = point.y;

		RECT rcTop,rcBottom;
		CWnd *pTopWnd = GetDlgItem(IDC_LIST_BOX);
		CWnd *pBtmWnd = GetDlgItem(IDC_E_PROP_HELP_STR);
		if(pTopWnd)
			pTopWnd->GetWindowRect(&rcTop);
		if(pBtmWnd)
			pBtmWnd->GetWindowRect(&rcBottom);
		ScreenToClient(&rcTop);
		ScreenToClient(&rcBottom);
		rcBottom.top = m_nOldHorzY+m_nSplitterWidth/2;
		rcTop.bottom = rcBottom.top-m_nSplitterWidth-1;
		pTopWnd->MoveWindow(&rcTop);
		pBtmWnd->MoveWindow(&rcBottom);
		//redraw
		Invalidate();
	}
	else
		CDialogEx::OnLButtonUp(nFlags, point);
}

void CPropertyDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_bTracking)
	{
		//move divider line to the mouse pos. if columns are
		//currently being resized
		CClientDC dc(this);
		//remove old divider line
		InvertLine(&dc,CPoint(m_rcClient.left,m_nOldHorzY),CPoint(m_rcClient.right,m_nOldHorzY));
		//draw new divider line
		InvertLine(&dc,CPoint(m_rcClient.left,point.y),CPoint(m_rcClient.right,point.y));
		m_nOldHorzY = point.y;
	}
	else if ((point.y >= m_nOldHorzY-5) && (point.y <= m_nOldHorzY+5))
		//set the cursor to a sizing cursor if the cursor is over the row divider
		::SetCursor(m_hCursorSize);
	else
		CDialogEx::OnMouseMove(nFlags, point);
}

void CPropertyDlg::OnSelchangeTabGroup(NMHDR* pNMHDR, LRESULT* pResult) 
{
	m_xPropList.m_iPropGroup = m_xTabCtrl.GetCurSel();
	m_xPropList.Redraw();
	*pResult = 0;
}

void CPropertyDlg::RefreshTabCtrl(int iCurSel)
{
	m_xTabCtrl.DeleteAllItems();
	for(int i=0;i<m_sTabLabelArr.GetSize();i++)
		m_xTabCtrl.InsertItem(i,m_sTabLabelArr[i]);
	if(m_sTabLabelArr.GetSize()>0)
	{
		m_xTabCtrl.SetCurSel(iCurSel);
		m_xPropList.m_iPropGroup=iCurSel;
	}
	else //不需要分组显示
		m_xPropList.m_iPropGroup=0;
	//根据分组数调整窗口位置
	RelayoutDlg();
}
