
// ChildFrm.cpp : CChildFrame ���ʵ��
//

#include "stdafx.h"
#include "TPSM.h"
#include "ChildFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CChildFrame

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWndEx)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWndEx)
	ON_WM_MDIACTIVATE()
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
	ON_UPDATE_COMMAND_UI(ID_SOLID_MODE_DISPLAY, OnUpdateSolidModeDisplay)
END_MESSAGE_MAP()

// CChildFrame ����/����

CChildFrame::CChildFrame()
{
	// TODO: �ڴ���ӳ�Ա��ʼ������
}

CChildFrame::~CChildFrame()
{
}


BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: �ڴ˴�ͨ���޸� CREATESTRUCT cs ���޸Ĵ��������ʽ
	if( !CMDIChildWndEx::PreCreateWindow(cs) )
		return FALSE;

	return TRUE;
}

// CChildFrame ���

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

// CChildFrame ��Ϣ�������
//����ͼ�л�ʱ���ݴ���
void CChildFrame::OnMDIActivate(BOOL bActivate,CWnd* pActivateWnd,CWnd* pDeactivateWnd)
{
	if (bActivate)
	{
		CDocument* pDoc = GetActiveDocument();
		if (pDoc != NULL && pDoc->IsKindOf(RUNTIME_CLASS(CTPSMDoc)))
		{
			CTPSMDoc* pActiveDoc = (CTPSMDoc*)pDoc;
			CTPSMView* pActiveView = (CTPSMView*)pActiveDoc->GetView(RUNTIME_CLASS(CTPSMView));
			if (pActiveView)
				pActiveView->Refresh();
			CMainFrame* pMainFrm = (CMainFrame*)AfxGetMainWnd();
			if (pMainFrm && pActiveDoc->m_pModel)
			{
				pMainFrm->UpdatePropertyPage(pActiveDoc->m_pModel);
				pMainFrm->GetWireDataPage()->UpdateWireNodeList(pActiveDoc->m_pModel);
			}
		}
	}
}

void CChildFrame::OnGlAllZoom() 
{	//ȫ��
	CTPSMView* pView=(CTPSMView*)GetActiveView();
	if(pView)
		pView->SolidOper()->ZoomAll(0.95);
}

void CChildFrame::OnGlZoom() 
{	//����
	CTPSMView* pView=(CTPSMView*)GetActiveView();
	if(pView)
		pView->SolidSet()->SetOperType(OPER_ZOOM);
}

void CChildFrame::OnUpdateGlZoom(CCmdUI* pCmdUI) 
{
	CTPSMView* pView=(CTPSMView*)GetActiveView();
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
{	//��ת
	CTPSMView* pView=(CTPSMView*)GetActiveView();
	if(pView)
		pView->SolidSet()->SetOperType(OPER_ROTATE);
}

void CChildFrame::OnUpdateGlRotated(CCmdUI* pCmdUI) 
{
	CTPSMView* pView=(CTPSMView*)GetActiveView();
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
{	//��λ
	CTPSMView* pView=(CTPSMView*)GetActiveView();
	if(pView)
		pView->SolidOper()->ReSetView();
}
void CChildFrame::OnOperOther() 
{
	CTPSMView* pView=(CTPSMView*)GetActiveView();
	if(pView)
	{
		pView->SolidSnap()->SetSnapType(SNAP_ALL);
		pView->SolidSet()->SetOperType(OPER_OTHER);
	}
}
void CChildFrame::OnGlPan() 
{	//ƽ��
	CTPSMView* pView=(CTPSMView*)GetActiveView();
	if(pView)
		pView->SolidSet()->SetOperType(OPER_PAN);	
}

void CChildFrame::OnUpdateGlPan(CCmdUI* pCmdUI) 
{
	CTPSMView* pView=(CTPSMView*)GetActiveView();
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
	CTPSMView* pView=(CTPSMView*)GetActiveView();
	if(pView)
	{
		DISPLAY_TYPE disp;
		pView->SolidSet()->GetDisplayType(&disp);
		//�л���ͼ��ʾ����
		if(disp==DISP_LINE)
			pView->SolidSet()->SetDisplayType(DISP_SOLID);
		else if(disp==DISP_SOLID)
			pView->SolidSet()->SetDisplayType(DISP_LINE);
		//
		pView->SolidDraw()->BuildDisplayList(pView);
		pView->SolidDraw()->Draw();
	}
}

void CChildFrame::OnUpdateSolidModeDisplay(CCmdUI* pCmdUI) 
{
	CTPSMView* pView=(CTPSMView*)GetActiveView();
	if(pView)
	{
		DISPLAY_TYPE type;
		pView->SolidSet()->GetDisplayType(&type);
		pCmdUI->SetCheck(type==DISP_SOLID);
	}
}