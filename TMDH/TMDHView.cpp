
// TMDHView.cpp : CTMDHView 类的实现
//

#include "stdafx.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "TMDH.h"
#endif

#include "TMDHDoc.h"
#include "TMDHView.h"
#include "HashTable.h"
#include "SolidBody.h"
#include "ProcBarDlg.h"
#include "ElectricTower.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////////
//回调函数
CHashList<CSolidBody> hashListBody;
extern GECS TransToUcs(MOD_CS modCs);
void DrawModTower(CTMDHView* pView,IModTowerInstance* pInstance)
{
	if(pView==NULL||pInstance==NULL)
		return;
	hashListBody.Empty();
	GECS rot_cs;
	CTMDHModel* pModel=NULL;
	CTMDHDoc* pDoc=(CTMDHDoc*)pView->GetDocument();
	if(pDoc->m_ciDataFlag==CTMDHDoc::TMD_DATA)
	{
		pModel=(CTMDHModel*)pDoc->m_pData;
		rot_cs=pModel->BuildRotateCS();
	}
	DISPLAY_TYPE disp_type;
	disp_type=(Manager.m_ciInstanceLevel==0)?DISP_LINE:DISP_SOLID;
	pView->SolidSet()->SetDisplayType(disp_type);
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
			if(pModel && pModel->m_bTurnLeft)
				xSolidPart.pBody->TransToACS(rot_cs);
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
			line.ID=pRod->GetId();
			line.pen.crColor=RGB(220,220,220);
			line.startPt.Set(mod_line.startPt.x,mod_line.startPt.y,mod_line.startPt.z);
			line.endPt.Set(mod_line.endPt.x,mod_line.endPt.y,mod_line.endPt.z);
			if(pModel && pModel->m_bTurnLeft)
			{
				line.startPt=rot_cs.TransPToCS(line.startPt);
				line.endPt=rot_cs.TransPToCS(line.endPt);
			}
			pView->SolidDraw()->NewLine(line);
		}
		DisplayProcess(100,"生成单线模型...");
	}
	//显示挂点信息
	GECS ucs=TransToUcs(Manager.m_pModModel->BuildUcsByModCS());
	int nHangPt=Manager.m_pModModel->GetHangNodeNum();
	for(int i=0;i<nHangPt;i++)
	{
		MOD_HANG_NODE* pModHangPt=Manager.m_pModModel->GetHangNodeById(i);
		GEPOINT pos=pModHangPt->m_xHangPos;
		pos=ucs.TransPToCS(pos);
		if(pModel && pModel->m_bTurnLeft)
			pos=rot_cs.TransPToCS(pos);
		double dfArrowLen=2000;
		f3dPoint vStdArrowVec;
		if(strstr(pModHangPt->m_sHangName,"前")||strstr(pModHangPt->m_sHangName,"后"))
		{
			if(pos.y<0)
				vStdArrowVec.Set(0,-1,0);
			else
				vStdArrowVec.Set(0, 1,0);
		}
		else
			vStdArrowVec.Set(0,0,1);	//悬垂挂线默认铅垂向下
		pView->SolidSet()->SetPen(RGB(0,255,255),PS_SOLID,2);
		CSolidPart solidpart;
		solidpart.m_bDisplaySingle=true;
		solidpart.m_iLineType = 3;	//箭头
		solidpart.dfSphereRadius=30;
		solidpart.line.startPt=pos;
		solidpart.line.endPt=pos+vStdArrowVec*dfArrowLen;
		pView->SolidDraw()->NewSolidPart(solidpart);
		//
		pView->SolidSet()->SetPen(RGB(255,255,0),PS_SOLID,2);
		f3dPoint dimpos=pos+vStdArrowVec*(dfArrowLen*0.5);
		pView->SolidDraw()->NewText(pModHangPt->m_sHangName,dimpos,160,vStdArrowVec,f3dPoint(0,0,0),ISolidDraw::TEXT_ALIGN_BTMCENTER,2);
	}
	pView->SolidOper()->ZoomAll(0.95);
}
void DrawTidTower(CTMDHView* pView,ITidTowerInstance* pInstance)
{
	if(pView==NULL||pInstance==NULL)
		return;
	hashListBody.Empty();
	CTMDHDoc* pDoc=(CTMDHDoc*)pView->GetDocument();
	GECS rot_cs;
	CTMDHModel* pModel=NULL;
	if(pDoc->m_ciDataFlag==CTMDHDoc::TMD_DATA)
	{
		pModel=(CTMDHModel*)pDoc->m_pData;
		rot_cs=pModel->BuildRotateCS();
	}
	DISPLAY_TYPE disp_type;
	disp_type=(Manager.m_ciInstanceLevel==0)?DISP_LINE:DISP_SOLID;
	pView->SolidSet()->SetDisplayType(disp_type);
	//根据配基信息获取一桩塔例
	int nPart=pInstance->GetAssemblePartNum();
	int nBolt=pInstance->GetAssembleBoltNum();
	int nAnchorBolt=pInstance->GetAssembleAnchorBoltNum();
	int nSum=nPart+nBolt+nAnchorBolt;
	if(disp_type==DISP_SOLID)
	{	//绘制实体
		DisplayProcess(0,"生成实体模型....");
		DWORD serial=1;
		for(ITidAssemblePart* pAssmPart=pInstance->EnumAssemblePartFirst();pAssmPart;pAssmPart=pInstance->EnumAssemblePartNext(),serial++)
		{
			DisplayProcess(ftoi(100*serial/nSum),"生成杆件实体模型....");
			if(Manager.m_ciInstanceLevel==1&&!pAssmPart->IsHasBriefRodLine())
				continue;	//只显示杆件实体
			ITidSolidBody* pSolidBody=pAssmPart->GetSolidPart();
			if(pSolidBody==NULL)
				continue;
			CSolidPart solidpart;
			solidpart.m_hPart=pAssmPart->GetId();
			solidpart.pBody=hashListBody.Add(serial);
			solidpart.pBody->CopyBuffer(pSolidBody->SolidBufferPtr(),pSolidBody->SolidBufferLength());
			if(pModel && pModel->m_bTurnLeft)
				solidpart.pBody->TransToACS(rot_cs);
			pView->SolidDraw()->NewSolidPart(solidpart);
			pSolidBody->ReleaseTemporaryBuffer();
		}
		if(Manager.m_ciInstanceLevel==3)
		{	//放样实体级别，显示螺栓
			for(ITidAssembleBolt* pAssmBolt=pInstance->EnumAssembleBoltFirst();pAssmBolt;pAssmBolt=pInstance->EnumAssembleBoltNext(),serial++)
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
				if(pModel && pModel->m_bTurnLeft)
					solidpart.pBody->TransToACS(rot_cs);
				pView->SolidDraw()->NewSolidPart(solidpart);
				//
				pSolidBody->ReleaseTemporaryBuffer();
				pNutSolid->ReleaseTemporaryBuffer();
			}
		}
		DisplayProcess(100,"生成实体模型....");
	}
	else if(disp_type==DISP_LINE)
	{	//绘制单线
		f3dLine line;
		line.pen.crColor=RGB(220,220,220);
		line.pen.width=1;
		line.pen.style=PS_SOLID;
		DisplayProcess(0,"生成单线实体...");
		int index=0;
		for(ITidAssemblePart* pAssmPart=pInstance->EnumAssemblePartFirst();pAssmPart;pAssmPart=pInstance->EnumAssemblePartNext(),index++)
		{
			DisplayProcess(ftoi(index*100/nPart),"生成单线实体...");
			if(!pAssmPart->IsHasBriefRodLine())
				continue;
			ITidNode* pNodeS=pInstance->FindNode(pAssmPart->GetStartNodeId());
			ITidNode* pNodeE=pInstance->FindNode(pAssmPart->GetEndNodeId());
			if(pNodeS==NULL||pNodeE==NULL)
				continue;	//短角钢
			line.ID=pAssmPart->GetId();
			TID_COORD3D ptS=pNodeS->GetPos();//pAssmPart->BriefLineStart();
			TID_COORD3D ptE=pNodeE->GetPos();//pAssmPart->BriefLineEnd();
			line.startPt.Set(ptS.x,ptS.y,ptS.z);
			line.endPt.Set(ptE.x,ptE.y,ptE.z);
			if(pModel && pModel->m_bTurnLeft)
			{
				line.startPt=rot_cs.TransPToCS(line.startPt);
				line.endPt=rot_cs.TransPToCS(line.endPt);
			}
			pView->SolidDraw()->NewLine(line);
		}
		DisplayProcess(100,"生成单线实体...");
	}
	//显示挂点信息
	int nHangPt=Manager.m_pTidModel->HangPointCount();
	for(int i=0;i<nHangPt;i++)
	{	
		ITidHangPoint* pTidHangPt=Manager.m_pTidModel->GetHangPointAt(i);
		GEPOINT pos(pTidHangPt->GetPos());
		if(pModel && pModel->m_bTurnLeft)
			pos=rot_cs.TransPToCS(pos);
		double dfArrowLen=2000;
		f3dPoint vStdArrowVec;
		if(pTidHangPt->GetTensionType()==1)
			vStdArrowVec.Set(0,0,1);	//悬垂挂线默认铅垂向下
		else if(pTidHangPt->GetWireDirection()=='X')
		{
			if(pos.x<0)
				vStdArrowVec.Set(-1,0,0);
			else
				vStdArrowVec.Set( 1,0,0);
		}
		else
		{
			if(pos.y<0)
				vStdArrowVec.Set(0,-1,0);
			else
				vStdArrowVec.Set(0, 1,0);
		}
		pView->SolidSet()->SetPen(RGB(0,255,255),PS_SOLID,2);
		CSolidPart solidpart;
		solidpart.m_bDisplaySingle=true;
		solidpart.m_iLineType = 3;	//箭头
		solidpart.dfSphereRadius=30;
		solidpart.line.startPt=pos;
		solidpart.line.endPt=pos+vStdArrowVec*dfArrowLen;
		pView->SolidDraw()->NewSolidPart(solidpart);
		//
		pView->SolidSet()->SetPen(RGB(255,255,0),PS_SOLID,2);
		f3dPoint dimpos=pos+vStdArrowVec*(dfArrowLen*0.5);
		CXhChar50 szWireName;
		pTidHangPt->GetWireDescription(szWireName);
		pView->SolidDraw()->NewText(szWireName,dimpos,160,vStdArrowVec,f3dPoint(0,0,0),ISolidDraw::TEXT_ALIGN_BTMCENTER,2);
	}
	pView->SolidOper()->ZoomAll(0.95);
}
void DrawTower(void* pContext)
{
	CTMDHView* pView=(CTMDHView*)pContext;
	if(pView==NULL)
		return;
	CTMDHDoc* pDoc=(CTMDHDoc*)pView->GetDocument();
	if(pDoc==NULL||pDoc->m_pData==NULL)
		return;
	if(pDoc->m_ciDataFlag==CTMDHDoc::GIM_DATA)
	{
		CTMDHGim* pGim=(CTMDHGim*)pDoc->m_pData;
		IModTowerInstance* pInstance=(IModTowerInstance*)pGim->ExtractActiveTaInstance();
		if(pInstance==NULL)
			return;
		return DrawModTower(pView,pInstance);
	}
	else if(pDoc->m_ciDataFlag==CTMDHDoc::TMD_DATA)
	{
		CTMDHModel* pModel=(CTMDHModel*)pDoc->m_pData;
		void* pInstance=pModel->ExtractActiveTaInstance();
		if(pModel->m_ciModelFlag==CTMDHModel::TYPE_TID)
			return DrawTidTower(pView,(ITidTowerInstance*)pInstance);
		else
			return DrawModTower(pView,(IModTowerInstance*)pInstance);
	}
	//
	CTMDHTower* pLdsModel=(CTMDHTower*)pDoc->m_pData;
	CTower* pTower=&(pLdsModel->m_xTower);
	if(pLdsModel->m_nMaxLegs==8)
		CFGLEG::SetSchema(CFGLEG::MULTILEG_MAX08);
	else if(pLdsModel->m_nMaxLegs==16)
		CFGLEG::SetSchema(CFGLEG::MULTILEG_MAX16);
	else if(pLdsModel->m_nMaxLegs==24)
		CFGLEG::SetSchema(CFGLEG::MULTILEG_MAX24);
	CLDSModule *pModule=pTower->GetActiveModule();
	if(pModule==NULL)
		return;
	UCS_STRU stdcs;
	LoadDefaultUCS(&stdcs);
	DISPLAY_TYPE disp_type;
	disp_type=(Manager.m_ciInstanceLevel==0)?DISP_LINE:DISP_SOLID;
	pView->SolidSet()->SetDisplayType(disp_type);
	//显示挂点
	for(CXhWirePoint* pWirePoint=gxWirePointModel.m_hashWirePoints.GetFirst();pWirePoint;pWirePoint=gxWirePointModel.m_hashWirePoints.GetNext())
	{
		if(pWirePoint->acs.axis_z.IsZero())
		{	//设定默认挂线方向
			if(pWirePoint->m_xWirePlace.ciTensionType==1)
				pWirePoint->acs.axis_z.Set(0,0,1);	//悬垂挂线默认铅垂向下
			else if(pWirePoint->m_xWirePlace.ciWireDirection=='X')
			{
				if(pWirePoint->xPosition.x<0)
					pWirePoint->acs.axis_z.Set(-1,0,0);
				else
					pWirePoint->acs.axis_z.Set( 1,0,0);
			}
			else //if(pWirePoint->m_xWirePlace.ciWireDirection=='Y')
			{
				if(pWirePoint->xPosition.y<0)
					pWirePoint->acs.axis_z.Set(0,-1,0);
				else
					pWirePoint->acs.axis_z.Set(0, 1,0);
			}
		}
		double dfArrowLen=2000;
		f3dPoint vStdArrowVec=pWirePoint->vWirePull;
		pView->SolidSet()->SetPen(RGB(0,255,255),PS_SOLID,2);
		pWirePoint->Create3dSolidModel();
		CSolidPart solidpart=pWirePoint->GetSolidPartObject(dfArrowLen);
		pView->SolidDraw()->NewSolidPart(solidpart);
		f3dPoint align_vec=vStdArrowVec;
		f3dPoint dimpos=pWirePoint->xPosition+align_vec*(dfArrowLen*0.5);
		normalize(align_vec);
		pView->SolidSet()->SetPen(RGB(255,255,0),PS_SOLID,2);
		CXhChar50 szWireName="导线";
		if(pWirePoint->m_xWirePlace.ciWireType=='J')
			szWireName.Copy("跳线");
		else if(pWirePoint->m_xWirePlace.ciWireType=='E')
			szWireName.Copy("地线");
		if(pWirePoint->m_sGimName.Length==0)
			gxWirePointModel.UpdateWirePointGimName(pWirePoint);
		if(pWirePoint->m_sGimName.Length>0)
			szWireName=pWirePoint->m_sGimName;
		pView->SolidDraw()->NewText(szWireName,dimpos,160,vStdArrowVec,f3dPoint(0,0,0),ISolidDraw::TEXT_ALIGN_BTMCENTER,2);
	}
	if(disp_type==DISP_LINE)
	{	//普通单线模式
		int nPart=pTower->GetObjNum(CLS_LINEPART);
		if(nPart<=0)
			return;
		pView->SolidSet()->SetPen(RGB(220,220,220),PS_SOLID,2);
		for(CLDSLinePart* pRod=pTower->EnumRodFirst();pRod;pRod=pTower->EnumRodNext())
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
		int serial=1;
		hashListBody.Empty();
		DisplayProcess(0,"生成构件实体模型....");
		for(CLDSPart* pPart=pTower->EnumPartFirst();pPart;pPart=pTower->EnumPartNext(),serial++)
		{
			DisplayProcess(ftoi(100*serial/nPart),"生成构件实体模型....");
			if(!pModule->IsSonPart(pPart))
				continue;
			if(Manager.m_ciInstanceLevel==1&&!pPart->IsLinePart())
				continue;	//级别1只显示杆件
			if(Manager.m_ciInstanceLevel!=3&&(pPart->GetClassTypeId()==CLS_BOLT||pPart->GetClassTypeId()==CLS_ANCHORBOLT))
				continue;	//非级别3不显示螺栓
			if(pPart->pSolidBody==NULL)
			{
				pPart->SetModified();
				pPart->Create3dSolidModel();
			}
			if(pPart->pSolidBody)
			{
				CSolidPart solidpart;
				solidpart.m_hPart=serial;
				solidpart.pBody=hashListBody.Add(serial);
				solidpart.pBody->CopyBuffer(pPart->pSolidBody->BufferPtr(),pPart->pSolidBody->BufferLength());
				pView->SolidDraw()->NewSolidPart(solidpart);
			}
		}
		DisplayProcess(100,"生成构件实体模型....");
		//生成部件实体
		int nBlckRef=pTower->BlockRef.GetNodeNum();
		if(nBlckRef<=0)
			return;
		DisplayProcess(0,"生成部件实体模型....");
		for(CBlockReference *pBlockRef=pTower->BlockRef.GetFirst();pBlockRef;pBlockRef=pTower->BlockRef.GetNext())
		{
			CBlockModel *pBlock=pTower->Block.FromHandle(pBlockRef->m_hBlock);
			if(pBlock==NULL)
				continue;
			UCS_STRU acs=pBlockRef->GetACS();
			UCS_STRU cs=pBlock->TransToACS(acs);
			PARTSET partSet;
			if(pBlock->IsEmbeded())
			{	//嵌入式部件
				for(CLDSPart *pPart=pTower->EnumPartFirst();pPart;pPart=pTower->EnumPartNext())
				{
					if(pPart->m_hBlock==pBlock->handle)
						partSet.append(pPart);
				}
			}
			else 
			{	//非嵌入式部件
				for(CLDSPart* pPart=pBlock->Parts.GetFirst();pPart;pPart=pBlock->Parts.GetNext())
					partSet.append(pPart);
			}
			for(pPart=partSet.GetFirst();pPart;pPart=partSet.GetNext(),serial++)
			{
				if(Manager.m_ciInstanceLevel==1&&!pPart->IsLinePart())
					continue;	//级别1只显示杆件
				if(Manager.m_ciInstanceLevel!=3&&(pPart->GetClassTypeId()==CLS_BOLT||pPart->GetClassTypeId()==CLS_ANCHORBOLT))
					continue;	//非级别3不显示螺栓
				if(pPart->pSolidBody==NULL)
				{
					pPart->SetModified();
					pPart->Create3dSolidModel(FALSE);
				}
				if(pPart->pSolidBody)
				{
					CSolidPart solidpart;
					solidpart.m_hPart=serial;
					solidpart.pBody=hashListBody.Add(serial);
					solidpart.pBody->CopyBuffer(pPart->pSolidBody->BufferPtr(),pPart->pSolidBody->BufferLength());
					solidpart.pBody->TransACS(stdcs,cs);
					pView->SolidDraw()->NewSolidPart(solidpart);
				}
			}
		}
		DisplayProcess(100,"生成部件实体模型....");
	}
}
//////////////////////////////////////////////////////////////////////////
// CTMDHView

IMPLEMENT_DYNCREATE(CTMDHView, CView)

BEGIN_MESSAGE_MAP(CTMDHView, CView)
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

// CTMDHView 构造/析构

CTMDHView::CTMDHView()
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

CTMDHView::~CTMDHView()
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

BOOL CTMDHView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CView::PreCreateWindow(cs);
}

int CTMDHView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	m_pDrawSolid=CDrawSolidFactory::CreateDrawEngine();
	m_pSolidDraw=m_pDrawSolid->SolidDraw();
	m_pSolidSet=m_pDrawSolid->SolidSet();
	m_pSolidSnap=m_pDrawSolid->SolidSnap();
	m_pSolidOper=m_pDrawSolid->SolidOper();
	m_pSolidSet->Init(m_hWnd);
	//字体设置
	I2dDrawing* pDraw=m_pDrawSolid->Drawing2d();
	IDrawingAssembly* pDrawAssembly=pDraw->InitDrawingAssembly();
	IDrawing* pDrawing=pDrawAssembly->AppendDrawing(1,"SolidCouple");
	char asciifontfile[MAX_PATH],hzfontfile[MAX_PATH];
	strcpy(asciifontfile,theApp.execute_path);
	strcat(asciifontfile,"sys\\simplex.shx");
	strcpy(hzfontfile,theApp.execute_path);
	strcat(hzfontfile,"sys\\GBHZFS.shx");
	m_pSolidSet->SetShxFontFile(asciifontfile);	//负责g_pSolidDraw->NewText英文
	m_pSolidSet->SetBigFontFile(hzfontfile);	//负责g_pSolidDraw->NewText汉字
	pDrawing->Database()->AddTextStyle("standard",asciifontfile,hzfontfile);//负责I2dDrawing->NewText
	m_pSolidSet->SetSolidBuddyDrawing(pDrawing->GetId());
	return 0;
}

void CTMDHView::OnInitialUpdate()
{
	CView::OnInitialUpdate();
	//
	UCS_STRU object_ucs;
	object_ucs.origin.Set();
	object_ucs.axis_x.Set(1,0,0);
	object_ucs.axis_y.Set(0,0,1);
	object_ucs.axis_z.Set(0,-1,0);
	m_pSolidSet->SetObjectUcs(object_ucs);
	m_pSolidSet->SetPen(m_pen);
	m_pSolidSet->SetBkColor(RGB(0,64,160));	//RGB(192,192,192)	
	m_pSolidSet->SetDisplayType(DISP_LINE);	
	m_pSolidSet->SetDisplayFunc(DrawTower);
	m_pSolidSet->SetSolidAndLineStatus(FALSE);
	m_pSolidSet->SetArcSamplingLength(5);
	m_pSolidSet->SetSmoothness(36);
	m_pSolidSet->SetDisplayLineVecMark(FALSE);
	m_pSolidSet->SetDispObjectUcs(TRUE);
	m_pSolidSet->SetRotOrg(f3dPoint(0,0,0));
	m_pSolidSet->SetZoomStyle(ROT_CENTER);
	m_pSolidDraw->InitialUpdate();
	Refresh();
}

void CTMDHView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
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

void CTMDHView::Refresh(BOOL bZoomAll/*=TRUE*/)
{
	Invalidate();
	m_pSolidDraw->BuildDisplayList(this);
	if(bZoomAll)
		m_pSolidOper->ZoomAll(0.9);
}
// CTMDHView 绘制
void CTMDHView::OnDraw(CDC* /*pDC*/)
{
	CTMDHDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;
	m_pSolidDraw->Draw();
}

// CTMDHView 诊断

#ifdef _DEBUG
void CTMDHView::AssertValid() const
{
	CView::AssertValid();
}

void CTMDHView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CTMDHDoc* CTMDHView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CTMDHDoc)));
	return (CTMDHDoc*)m_pDocument;
}
#endif //_DEBUG


// CTMDHView 消息处理程序
void CTMDHView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	if(m_pSolidOper)
	{
		m_pSolidOper->ReSize();
		m_pSolidOper->ZoomAll(0.9);
	}
}
void CTMDHView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if(nChar==VK_ESCAPE)
	{
		m_pSolidSnap->SetSnapType(SNAP_ALL);
		m_pSolidSet->SetOperType(OPER_OTHER);
	}
	m_pSolidOper->KeyDown(nChar);
	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CTMDHView::OnLButtonDown(UINT nFlags, CPoint point) 
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

void CTMDHView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	m_pSolidSet->SetOperType(OPER_ROTATE);	//为便捷操作，双击时激活旋转操作
	BOOL bAltKeyDown=GetKeyState(VK_MENU)&0x8000;
	if((nFlags&MK_CONTROL)==0)	//只有未按下Control键时的双击才会激发双击更改旋转中心操作
		m_pSolidOper->LMouseDoubleClick(point);
	CView::OnLButtonDblClk(nFlags, point);
}

void CTMDHView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	m_pSolidOper->LMouseUp(point);
	if(GetCapture()==this)
		ReleaseCapture();
	CView::OnLButtonUp(nFlags, point);
}

void CTMDHView::OnMouseMove(UINT nFlags, CPoint point) 
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
BOOL CTMDHView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
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
void CTMDHView::OnRButtonDown(UINT nFlags, CPoint point)
{
	m_pSolidOper->RMouseDown(point);
	CView::OnRButtonDown(nFlags, point);
}

void CTMDHView::OnRButtonUp(UINT nFlags, CPoint point)
{
	m_pSolidOper->RMouseUp(point);
	CView::OnRButtonUp(nFlags, point);
}
