
// TPSMView.cpp : CTPSMView 类的实现
//

#include "stdafx.h"
#include "TPSM.h"
#include "TPSMDoc.h"
#include "TPSMView.h"
#include "TPSMModel.h"
#include "ProcBarDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

::CEvent LButtonDown,RButtonDown,HaltProc;
//////////////////////////////////////////////////////////////////////////
//回调函数
CHashList<CSolidBody> hashListBody;
void DrawModTower(CTPSMView* pView,IModModel* pModModel)
{
	if(pView==NULL||pModModel==NULL)
		return;
	IModHeightGroup* pHeightGroup=pModModel->GetHeightGroup(1);
	if(pHeightGroup==NULL)
		return;
	IModTowerInstance* pInstance=pHeightGroup->GetTowerInstance(1,1,1,1);
	if(pInstance==NULL)
		return;
	UCS_STRU object_ucs;
	object_ucs.origin.Set();
	object_ucs.axis_x.Set(1,0,0);
	object_ucs.axis_y.Set(0,0,1);
	object_ucs.axis_z.Set(0,-1,0);
	pView->SolidSet()->SetObjectUcs(object_ucs);
	DISPLAY_TYPE disp_type;
	pView->SolidSet()->GetDisplayType(&disp_type);
	int serial=1;
	int nRod=pInstance->GetModRodNum();
	if(disp_type==DISP_SOLID)
	{
		DisplayProcess(0,"生成实体模型....");
		for(IModRod* pRod=pInstance->EnumModRodFir();pRod;pRod=pInstance->EnumModRodNext(),serial++)
		{
			DisplayProcess(ftoi(serial*100/nRod),"生成实体模型...");
			//
			CSolidPart xSolidPart;
			xSolidPart.m_hPart=pRod->GetId();
			xSolidPart.pBody=hashListBody.Add(serial);
			pRod->Create3dSolidModel(xSolidPart.pBody);
			pView->SolidDraw()->NewSolidPart(xSolidPart);
		}
		DisplayProcess(100,"生成实体模型....");
	}
	else
	{
		DisplayProcess(0,"生成单线模型...");
		for(IModRod* pRod=pInstance->EnumModRodFir();pRod;pRod=pInstance->EnumModRodNext(),serial++)
		{
			DisplayProcess(ftoi(serial*100/nRod),"生成单线模型...");
			//
			MOD_LINE mod_line=pRod->GetBaseLineToLdsModel();
			f3dLine line;
			line.startPt.Set(mod_line.startPt.x,mod_line.startPt.y,mod_line.startPt.z);
			line.endPt.Set(mod_line.endPt.x,mod_line.endPt.y,mod_line.endPt.z);
			line.pen.crColor=RGB(220,220,220);
			line.ID=pRod->GetId();
			pView->SolidDraw()->NewLine(line);
		}
		DisplayProcess(100,"生成单线模型...");
	}
	pView->SolidOper()->ZoomAll(0.95);
}
UCS_STRU CastToUcs( TID_CS& tcs)
{
	UCS_STRU cs;
	cs.origin=f3dPoint(tcs.origin);
	cs.axis_x=f3dPoint(tcs.axisX);
	cs.axis_y=f3dPoint(tcs.axisY);
	cs.axis_z=f3dPoint(tcs.axisZ);
	return cs;
}
void DrawTidTower(CTPSMView* pView,ITidModel* pTidModel)
{
	if(pView==NULL||pTidModel==NULL||pTidModel->HeightGroupCount()==0)
		return;
	ITidHeightGroup* pHeightGroup=pTidModel->GetHeightGroupAt(0);
	if(pHeightGroup==NULL)
		return;
	ITidTowerInstance* pTowerInstance=pHeightGroup->GetTowerInstance(1,1,1,1);
	if(pTowerInstance==NULL)
		return;
	UCS_STRU cs=CastToUcs(pTidModel->ModelCoordSystem());
	pView->SolidSet()->SetObjectUcs(cs);
	DISPLAY_TYPE disp_type;
	pView->SolidSet()->GetDisplayType(&disp_type);
	//根据配基信息获取一桩塔例
	int nPart=pTowerInstance->GetAssemblePartNum();
	int nBolt=pTowerInstance->GetAssembleBoltNum();
	int nAnchorBolt=pTowerInstance->GetAssembleAnchorBoltNum();
	int nSum=nPart+nBolt+nAnchorBolt;
	if(disp_type==DISP_SOLID)
	{	//绘制实体
		DisplayProcess(0,"生成实体模型....");
		DWORD serial=1;
		for(ITidAssemblePart* pAssmPart=pTowerInstance->EnumAssemblePartFirst();pAssmPart;pAssmPart=pTowerInstance->EnumAssemblePartNext(),serial++)
		{
			CSolidPart solidpart;
			solidpart.m_hPart=pAssmPart->GetId();
			solidpart.pBody=hashListBody.Add(serial);
			
			if(pAssmPart->IsHasBriefRodLine())
			{
				TID_COORD3D ptS=pAssmPart->BriefLineStart();
				TID_COORD3D ptE=pAssmPart->BriefLineEnd();
				solidpart.line.startPt.Set(ptS.x,ptS.y,ptS.z);
				solidpart.line.endPt.Set(ptE.x,ptE.y,ptE.z);
				solidpart.m_iLineType=1;
				solidpart.m_bDisplaySingle=true;
			}
			ITidSolidBody* pSolidBody=pAssmPart->GetSolidPart();
			solidpart.pBody->CopyBuffer(pSolidBody->SolidBufferPtr(),pSolidBody->SolidBufferLength());
			pView->SolidDraw()->NewSolidPart(solidpart);
			pSolidBody->ReleaseTemporaryBuffer();
			DisplayProcess(ftoi(100*serial/nSum),"生成杆件实体模型....");
		}
		for(ITidAssembleBolt* pAssmBolt=pTowerInstance->EnumAssembleBoltFirst();pAssmBolt;pAssmBolt=pTowerInstance->EnumAssembleBoltNext(),serial++)
		{
			DisplayProcess(ftoi(100*serial/nSum),"生成螺栓实体模型....");
			ITidSolidBody* pSolidBody=pAssmBolt->GetBoltSolid();
			ITidSolidBody* pNutSolid=pAssmBolt->GetNutSolid();
			if(pSolidBody==NULL||pNutSolid==NULL)
				continue;
			CSolidPart solidpart;
			solidpart.m_hPart=pAssmBolt->GetId();
			solidpart.pBody=hashListBody.Add(serial);
			solidpart.pBody->CopyBuffer(pSolidBody->SolidBufferPtr(),pSolidBody->SolidBufferLength());
			CSolidBody nutsolid;
			nutsolid.CopyBuffer(pNutSolid->SolidBufferPtr(),pNutSolid->SolidBufferLength());
			solidpart.pBody->MergeBodyBuffer(nutsolid.BufferPtr(),nutsolid.BufferLength());
			pView->SolidDraw()->NewSolidPart(solidpart);
			//
			pSolidBody->ReleaseTemporaryBuffer();
			pNutSolid->ReleaseTemporaryBuffer();
		}
		DisplayProcess(100,"生成实体模型....");
	}
	else if(disp_type==DISP_LINE)
	{	//绘制单线
		f3dLine line;
		line.pen.crColor=RGB(150,150,255);
		line.pen.width=1;
		line.pen.style=PS_SOLID;
		DisplayProcess(0,"生成单线实体...");
		int index=0;
		for(ITidAssemblePart* pAssmPart=pTowerInstance->EnumAssemblePartFirst();pAssmPart;pAssmPart=pTowerInstance->EnumAssemblePartNext(),index++)
		{
			DisplayProcess(ftoi(index*100/nPart),"生成单线实体...");
			if(!pAssmPart->IsHasBriefRodLine())
				continue;
			ITidNode* pNodeS=pTowerInstance->FindNode(pAssmPart->GetStartNodeId());
			ITidNode* pNodeE=pTowerInstance->FindNode(pAssmPart->GetEndNodeId());
			if(pNodeS==NULL||pNodeE==NULL)
				continue;	//短角钢
			TID_COORD3D ptS=pNodeS->GetPos();//pAssmPart->BriefLineStart();
			TID_COORD3D ptE=pNodeE->GetPos();//pAssmPart->BriefLineEnd();
			line.startPt.Set(ptS.x,ptS.y,ptS.z);
			line.endPt.Set(ptE.x,ptE.y,ptE.z);
			line.ID=pAssmPart->GetId();
			pView->SolidDraw()->NewLine(line);
		}
		DisplayProcess(100,"生成单线实体...");
	}
	pView->SolidOper()->ZoomAll(0.95);
}
void DrawTower(void* pContext)
{
	CTPSMView* pView=(CTPSMView*)pContext;
	if(pView==NULL)
		return;
	CTPSMDoc* pDoc=(CTPSMDoc*)pView->GetDocument();
	if(pDoc==NULL)
		return;
	if(pDoc->m_ciDocType==CTPSMDoc::DOC_MOD&&pDoc->m_pModModel)
		return DrawModTower(pView,pDoc->m_pModModel);
	else if(pDoc->m_ciDocType==CTPSMDoc::DOC_TID&&pDoc->m_pTidModel)
		return DrawTidTower(pView,pDoc->m_pTidModel);
	//
	UCS_STRU object_ucs;
	object_ucs.origin.Set();
	object_ucs.axis_x.Set(1,0,0);
	object_ucs.axis_y.Set(0,0,1);
	object_ucs.axis_z.Set(0,-1,0);
	pView->SolidSet()->SetObjectUcs(object_ucs);
	DISPLAY_TYPE disp_type;
	pView->SolidSet()->GetDisplayType(&disp_type);
	int serial=0;
	CTower* pTower=NULL;
	CUniWireModel* pWireModel = NULL;
	CFGLEG::SetSchema(CFGLEG::MULTILEG_DEFAULT);
	if(pDoc->m_pModel)
	{
		pTower=&(pDoc->m_pModel->m_xTower);
		pWireModel = &(pDoc->m_pModel->m_xWirePointModel);
		if(pDoc->m_pModel->m_nMaxLegs==8)
			CFGLEG::SetSchema(CFGLEG::MULTILEG_MAX08);
		else if(pDoc->m_pModel->m_nMaxLegs==16)
			CFGLEG::SetSchema(CFGLEG::MULTILEG_MAX16);
		else if(pDoc->m_pModel->m_nMaxLegs==24)
			CFGLEG::SetSchema(CFGLEG::MULTILEG_MAX24);
	}
	else if (pDoc->m_ciDocType == CTPSMDoc::DOC_TPI)
	{
		pTower = &(pDoc->Tpi.m_xTower);
		pWireModel = &(pDoc->Tpi.m_xWirePointModel);
	}
	else if (pDoc->m_ciDocType == CTPSMDoc::DOC_LDS)
	{
		pTower = &(pDoc->Ta);
		pWireModel = &gxWirePointModel;
	}
	CLDSModule *pModule=pTower->GetActiveModule();
	if(pModule==NULL)
		return;
	if(disp_type==DISP_LINE)
	{	//普通单线模式
		int nPart=pTower->GetObjNum(CLS_LINEPART);
		if(nPart<=0)
			return;
		for(CLDSLinePart* pRod=pTower->EnumRodFirst();pRod;pRod=pTower->EnumRodNext(),serial++)
		{
			if(!pModule->IsSonPart(pRod))
				continue;
			CSolidBody *pSolidBody=pRod->pSolidBody;
			pRod->pSolidBody=NULL;	//加快单线图显示速度（设为NULL则暂不进行实体显示）
			pView->SolidDraw()->NewSolidPart(pRod->GetSolidPartObject());
			pRod->pSolidBody=pSolidBody;
		}
	}
	else if(disp_type==DISP_SOLID)
	{	//实体显示模式
		int nPart=pTower->Parts.GetNodeNum();
		if(nPart<=0)
			return;
		DisplayProcess(0,"生成实体模型....");
		for(CLDSPart* pPart=pTower->EnumPartFirst();pPart;pPart=pTower->EnumPartNext(),serial++)
		{
			DisplayProcess(ftoi(100*serial/nPart),"生成杆件实体模型....");
			if(!pModule->IsSonPart(pPart))
				continue;
			pPart->Create3dSolidModel();
			pView->SolidDraw()->NewSolidPart(pPart->GetSolidPartObject());
		}
		DisplayProcess(100,"生成实体模型....");
	}
	//显示挂点
	if (pWireModel)
	{
		CXhWirePoint* pWirePoint = NULL;
		for (pWirePoint = pWireModel->m_hashWirePoints.GetFirst(); pWirePoint; pWirePoint = pWireModel->m_hashWirePoints.GetNext())
		{
			GEPOINT pos(pWirePoint->xPosition), vec;
			BYTE ciPosSymbol = 0;
			if (pWirePoint->m_xWirePlace.ciWireType != 'J' && pWirePoint->m_xWirePlace.ciTensionType >= 2)
			{
				if (pWirePoint->m_xWirePlace.ciWireDirection == 'Y')
					ciPosSymbol = (pos.y < -EPS2) ? 'Q' : 'H';
				else //if(m_xWirePlace.ciWireDirection=='X')
					ciPosSymbol = (pos.x < -EPS2) ? 'Q' : 'H';
			}
			if (ciPosSymbol == 0 || pWirePoint->m_xWirePlace.ciTensionType == 1)
				vec.Set(0, 0, 1);	//悬垂挂线默认铅垂向下
			else if (ciPosSymbol == 'Q')
			{
				if (pWirePoint->m_xWirePlace.ciWireDirection == 'X')
					vec.Set(-1, 0, 0);
				else
					vec.Set(0, -1, 0);
			}
			else if (ciPosSymbol == 'H')
			{
				if (pWirePoint->m_xWirePlace.ciWireDirection == 'X')
					vec.Set(1, 0, 0);
				else
					vec.Set(0, 1, 0);
			}
			//绘制箭头
			pView->SolidSet()->SetPen(RGB(0, 255, 255), PS_SOLID, 2);
			double dfArrowLen = 2000;
			CSolidPart solidpart;
			solidpart.m_bDisplaySingle = true;
			solidpart.m_iLineType = 3;
			solidpart.dfSphereRadius = 0;
			solidpart.line.startPt = pos;
			solidpart.line.endPt = pos + vec * dfArrowLen;
			pView->SolidDraw()->NewSolidPart(solidpart);
			//绘制文字
			pView->SolidSet()->SetPen(RGB(255, 255, 0), PS_SOLID, 2);
			f3dPoint dimpos = pos + vec * dfArrowLen*0.5, align_vec(vec);
			pView->SolidDraw()->NewText(pWirePoint->m_sGimName, dimpos, 160, align_vec, f3dPoint(0, 0, 0), ISolidDraw::TEXT_ALIGN_BTMCENTER, 2);
		}
	}
}
//////////////////////////////////////////////////////////////////////////
// CTPSMView
IMPLEMENT_DYNCREATE(CTPSMView, CView)

BEGIN_MESSAGE_MAP(CTPSMView, CView)
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

// CTPSMView 构造/析构

CTPSMView::CTPSMView()
{
	m_pDrawSolid=NULL;
	m_pSolidDraw=NULL;
	m_pSolidSet=NULL;
	m_pSolidSnap=NULL;
	m_pSolidOper=NULL;
	m_pen.crColor=RGB(220,220,220);
	m_pen.width = 1;
	m_pen.style = 0;
}

CTPSMView::~CTPSMView()
{
	if(m_pDrawSolid)
	{
		CDrawSolidFactory::Destroy(m_pDrawSolid->GetSerial());
		m_pSolidDraw=NULL;
		m_pSolidOper=NULL;
		m_pSolidSet=NULL;
		m_pSolidSnap=NULL;
		m_pDrawSolid=NULL;
	}
}

BOOL CTPSMView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CView::PreCreateWindow(cs);
}

// CTPSMView 绘制
void CTPSMView::OnDraw(CDC* /*pDC*/)
{
	CTPSMDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;
	m_pSolidDraw->Draw();
}
// CTPSMView 诊断

#ifdef _DEBUG
void CTPSMView::AssertValid() const
{
	CView::AssertValid();
}

void CTPSMView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CTPSMDoc* CTPSMView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CTPSMDoc)));
	return (CTPSMDoc*)m_pDocument;
}
#endif //_DEBUG

void CTPSMView::Refresh(BOOL bZoomAll/*=TRUE*/)
{
	Invalidate();
	m_pSolidDraw->BuildDisplayList(this);
	if(bZoomAll)
		m_pSolidOper->ZoomAll(0.9);
}
//////////////////////////////////////////////////////////////////////////
// CTPSMView 消息处理程序
int CTPSMView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	m_pDrawSolid=CDrawSolidFactory::CreateDrawEngine();
	m_pSolidDraw=m_pDrawSolid->SolidDraw();
	m_pSolidSet=m_pDrawSolid->SolidSet();
	m_pSolidSnap=m_pDrawSolid->SolidSnap();
	m_pSolidOper=m_pDrawSolid->SolidOper();
	m_pSolidSet->Init(m_hWnd);
	//设置字体
	char sFontFile[MAX_PATH],sBigFontFile[MAX_PATH];
	sprintf(sFontFile,"%s\\sys\\simplex.shx",theApp.execute_path);
	m_pSolidSet->SetShxFontFile(sFontFile);
	sprintf(sBigFontFile,"%s\\sys\\GBHZFS.shx",theApp.execute_path);
	m_pSolidSet->SetBigFontFile(sBigFontFile);
	return 0;
}

void CTPSMView::OnInitialUpdate()
{
	CView::OnInitialUpdate();
	//
	UCS_STRU ucs;								//设置坐标系
	LoadDefaultUCS(&ucs);
	m_pSolidSet->SetObjectUcs(ucs);
	m_pSolidSet->SetPen(m_pen);
	m_pSolidSet->SetBkColor(RGB(0,64,160));	//RGB(192,192,192)	
	m_pSolidSet->SetDisplayType(DISP_LINE);	
	m_pSolidSet->SetDisplayFunc(DrawTower);
	m_pSolidSet->SetSolidAndLineStatus(TRUE);
	m_pSolidSet->SetArcSamplingLength(5);
	m_pSolidSet->SetSmoothness(36);
	m_pSolidSet->SetDisplayLineVecMark(FALSE);
	m_pSolidSet->SetDispObjectUcs(TRUE);
	m_pSolidSet->SetRotOrg(f3dPoint(0,0,0));
	m_pSolidSet->SetZoomStyle(ROT_CENTER);
	m_pSolidDraw->InitialUpdate();
	Refresh();
}

void CTPSMView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	if(GetParentFrame()!=NULL)
	{	//刷新工作显示区
		m_pSolidDraw->BatchClearCS(ISolidDraw::MODEL_CS);
		m_pSolidDraw->BuildDisplayList();
		m_pSolidOper->ZoomAll(0.95);
		m_pSolidOper->ReSetView();
		m_pSolidSnap->SetSnapType(SNAP_ALL);
		m_pSolidSet->SetOperType(OPER_OTHER);
	}
}

void CTPSMView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	if(m_pSolidOper)
	{
		m_pSolidOper->ReSize();
		m_pSolidOper->ZoomAll(0.9);
	}
}

void CTPSMView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if(nChar==VK_ESCAPE)
	{
		m_pSolidSnap->SetSnapType(SNAP_ALL);
		m_pSolidSet->SetOperType(OPER_OTHER);
	}
	m_pSolidOper->KeyDown(nChar);
	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CTPSMView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	BOOL bAltKeyDown=GetKeyState(VK_MENU)&0x8000;
	if(nFlags&MK_SHIFT)
	{
		m_pSolidDraw->ReleaseSnapStatus();
		m_pSolidDraw->BatchClearCS(4);
	}
	SetCapture();	//锁定光标在当前窗口
	m_pSolidOper->LMouseDown(point);
	CView::OnLButtonDown(nFlags, point);
}

void CTPSMView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	m_pSolidSet->SetOperType(OPER_ROTATE);	//为便捷操作，双击时激活旋转操作
	BOOL bAltKeyDown=GetKeyState(VK_MENU)&0x8000;
	if((nFlags&MK_CONTROL)==0)	//只有未按下Control键时的双击才会激发双击更改旋转中心操作
		m_pSolidOper->LMouseDoubleClick(point);
	CView::OnLButtonDblClk(nFlags, point);
}

void CTPSMView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	m_pSolidOper->LMouseUp(point);
	if(GetCapture()==this)
		ReleaseCapture();
	CView::OnLButtonUp(nFlags, point);
}

void CTPSMView::OnMouseMove(UINT nFlags, CPoint point) 
{
	m_pSolidOper->MouseMove(point,nFlags);
	//
	f3dPoint user_pt;
	f2dPoint port_pt(point.x,point.y);
	CString sMousePos;
	if(!m_pSolidOper->IsHasHighlightPoint(&user_pt))
		m_pSolidOper->ScreenToUser(&user_pt,port_pt);
	UCS_STRU object_ucs;
	m_pSolidSet->GetObjectUcs(object_ucs);
	coord_trans(user_pt,object_ucs,FALSE);
	sMousePos.Format("X=%.2f,Y=%.2f,Z=%.2f",user_pt.x,user_pt.y,user_pt.z);

	CView::OnMouseMove(nFlags, point);
}
BOOL CTPSMView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	ZOOM_CENTER_STYLE zoom_style;
	m_pSolidSet->GetZoomStyle(&zoom_style);
	m_pSolidSet->SetZoomStyle(MOUSE_CENTER);
	if(zDelta>0)
		m_pSolidOper->Scale(1.6);
	else
		m_pSolidOper->Scale(1/1.6);
	m_pSolidOper->RefreshScope();
	m_pSolidSet->SetZoomStyle(zoom_style);

	return CView::OnMouseWheel(nFlags, zDelta, pt);
}
void CTPSMView::OnRButtonDown(UINT nFlags, CPoint point)
{
	m_pSolidOper->RMouseDown(point);
	CView::OnRButtonDown(nFlags, point);
}

void CTPSMView::OnRButtonUp(UINT nFlags, CPoint point)
{
	m_pSolidOper->RMouseUp(point);
	CView::OnRButtonUp(nFlags, point);
}
