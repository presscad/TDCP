
// ChildFrm.cpp : CChildFrame 类的实现
//

#include "stdafx.h"
#include "TMDH.h"
#include "ChildFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CChildFrame

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWndEx)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWndEx)
	ON_COMMAND(ID_GL_ALL_ZOOM, OnGlAllZoom)
	ON_COMMAND(ID_GL_ZOOM, OnGlZoom)
	ON_UPDATE_COMMAND_UI(ID_GL_ZOOM, OnUpdateGlZoom)
	ON_COMMAND(ID_GL_ROTATED, OnGlRotated)
	ON_UPDATE_COMMAND_UI(ID_GL_ROTATED, OnUpdateGlRotated)
	ON_COMMAND(ID_RESET_VIEW, OnResetView)
	ON_COMMAND(ID_OPER_OTHER, OnOperOther)
	ON_COMMAND(ID_GL_PAN, OnGlPan)
	ON_UPDATE_COMMAND_UI(ID_GL_PAN, OnUpdateGlPan)
	ON_COMMAND(ID_SOLID_MODE_DISPLAY, OnSolidModeDisplay)
	ON_COMMAND(ID_LEVELTEXT, OnClickLevelCmbBox)
	ON_CBN_SELCHANGE(ID_LEVELTEXT,OnSelChangeClickCmbBox)
END_MESSAGE_MAP()

// CChildFrame 构造/析构

CChildFrame::CChildFrame()
{
	// TODO: 在此添加成员初始化代码
}

CChildFrame::~CChildFrame()
{
}


BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改 CREATESTRUCT cs 来修改窗口类或样式
	if( !CMDIChildWndEx::PreCreateWindow(cs) )
		return FALSE;

	return TRUE;
}

// CChildFrame 诊断

#ifdef _DEBUG
void CChildFrame::AssertValid() const
{
	CMDIChildWndEx::AssertValid();
}

void CChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWndEx::Dump(dc);
}
#endif //_DEBUG

// CChildFrame 消息处理程序
void CChildFrame::OnGlAllZoom() 
{	//全显
	CTMDHView* pView=(CTMDHView*)GetActiveView();
	if(pView)
		pView->SolidOper()->ZoomAll(0.95);
}

void CChildFrame::OnGlZoom() 
{	//缩放
	CTMDHView* pView=(CTMDHView*)GetActiveView();
	if(pView)
		pView->SolidSet()->SetOperType(OPER_ZOOM);
}

void CChildFrame::OnUpdateGlZoom(CCmdUI* pCmdUI) 
{
	CTMDHView* pView=(CTMDHView*)GetActiveView();
	if(pView)
	{
		OPER_TYPE oper_type=pView->SolidSet()->GetOperType();
		if(oper_type==OPER_ZOOM)
			pCmdUI->SetCheck(TRUE);
		else
			pCmdUI->SetCheck(FALSE);
	}
}
void CChildFrame::OnGlRotated() 
{	//旋转
	CTMDHView* pView=(CTMDHView*)GetActiveView();
	if(pView)
		pView->SolidSet()->SetOperType(OPER_ROTATE);
}

void CChildFrame::OnUpdateGlRotated(CCmdUI* pCmdUI) 
{
	CTMDHView* pView=(CTMDHView*)GetActiveView();
	if(pView)
	{
		OPER_TYPE oper_type=pView->SolidSet()->GetOperType();
		if(oper_type==OPER_ROTATE)
			pCmdUI->SetCheck(TRUE);
		else
			pCmdUI->SetCheck(FALSE);
	}
}
void CChildFrame::OnResetView() 
{	//复位
	CTMDHView* pView=(CTMDHView*)GetActiveView();
	if(pView)
		pView->SolidOper()->ReSetView();
}
void CChildFrame::OnOperOther() 
{
	CTMDHView* pView=(CTMDHView*)GetActiveView();
	if(pView)
	{
		pView->SolidSnap()->SetSnapType(SNAP_ALL);
		pView->SolidSet()->SetOperType(OPER_OTHER);
	}
}
void CChildFrame::OnGlPan() 
{	//平移
	CTMDHView* pView=(CTMDHView*)GetActiveView();
	if(pView)
		pView->SolidSet()->SetOperType(OPER_PAN);	
}

void CChildFrame::OnUpdateGlPan(CCmdUI* pCmdUI) 
{
	CTMDHView* pView=(CTMDHView*)GetActiveView();
	if(pView)
	{
		OPER_TYPE oper_type=pView->SolidSet()->GetOperType();
		if(oper_type==OPER_PAN)
			pCmdUI->SetCheck(TRUE);
		else
			pCmdUI->SetCheck(FALSE);
	}	
}

void CChildFrame::OnSolidModeDisplay() 
{
	CTMDHView* pView=(CTMDHView*)GetActiveView();
	if(pView)
	{
		pView->SolidDraw()->BuildDisplayList(pView);
		pView->SolidDraw()->Draw();
	}
}
void CChildFrame::OnClickLevelCmbBox()
{
	
}
void CChildFrame::OnSelChangeClickCmbBox()
{
	CMFCToolBarComboBoxButton* pSrcCmb = CMFCToolBarComboBoxButton::GetByCmd(ID_LEVELTEXT);
	int index=pSrcCmb->GetCurSel();
	CString sText=pSrcCmb->GetItem(index);
	if(sText.CompareNoCase("LOD0:单线")==0)
		Manager.m_ciInstanceLevel=0;
	else if(sText.CompareNoCase("LOD1:杆件")==0)
		Manager.m_ciInstanceLevel=1;
	else if(sText.CompareNoCase("LOD2:杆件+板")==0)
		Manager.m_ciInstanceLevel=2;
	else if(sText.CompareNoCase("LOD3:杆件+板+螺栓")==0)
		Manager.m_ciInstanceLevel=3;
}
