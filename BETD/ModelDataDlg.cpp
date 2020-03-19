// Main.cpp : 实现文件
//
#include "stdafx.h"
#include <direct.h>
#include "HashTable.h"
#include "ModelDataDlg.h"
#include "TowerManager.h"
#include "ProcBarDlg.h"
#include "LogFile.h"
#include "ArrayList.h"
//////////////////////////////////////////////////////////////////////////
//
CHashList<CSolidBody> hashListBody;
static BOOL DblclkListCtrl(CXhListCtrl* pListCtrl,int iItem,int iSubItem)
{
	CModelDataDlg* pDlg=(CModelDataDlg*)pListCtrl->GetParent();
	if(pDlg==NULL)
		return FALSE;
	TaManager.m_pActiveTa=(CTowerModel*)pListCtrl->GetItemData(iItem);
	pDlg->RedrawTowerSketch();
	return TRUE;
}
void MakeDirectory(const char *path)
{
	char bak_path[MAX_PATH],drive[MAX_PATH];
	strcpy(bak_path,path);
	char *dir = strtok(bak_path,"/\\");
	if(strlen(dir)==2&&dir[1]==':')
	{
		strcpy(drive,dir);
		strcat(drive,"\\");
		_chdir(drive);
		dir = strtok(NULL,"/\\");
	}
	while(dir)
	{
		_mkdir(dir);
		_chdir(dir);
		dir = strtok(NULL,"/\\");
	}
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
#ifndef _HANDOVER_TO_CLIENT_
void DrawSolidTower(void* pContext)
{
	CModelDataDlg* pView=(CModelDataDlg*)pContext;
	if(pView==NULL||TaManager.m_pActiveTa==NULL)
		return;
	CLogErrorLife logErrLife;
	hashListBody.Empty();
	pView->SolidDraw()->EmptyDisplayBuffer();
	DISPLAY_TYPE disp_type=(TaManager.m_ciInstanceLevel==0)?DISP_LINE:DISP_SOLID;
	pView->SolidSet()->SetDisplayType(disp_type);
	GECS rot_cs=TaManager.m_pActiveTa->BuildRotateCS();
	if(TaManager.m_pActiveTa->m_ciModelFlag==0)
	{	//TID模型
		ITidTowerInstance* pTowerInstance=(ITidTowerInstance*)TaManager.m_pActiveTa->ExtractActiveTaInstance();
		if(pTowerInstance==NULL)
		{
			AfxMessageBox(CXhChar100("%s配基信息有误!",(char*)TaManager.m_pActiveTa->m_sTowerType));
			return;
		}
		int nPart=pTowerInstance->GetAssemblePartNum();
		int nBolt=pTowerInstance->GetAssembleBoltNum();
		int nAnchorBolt=pTowerInstance->GetAssembleAnchorBoltNum();
		int nSum=nPart+nBolt+nAnchorBolt;
		int serial=1;
		if(disp_type==DISP_SOLID)
		{	//绘制实体
			DisplayProcess(0,"生成实体模型....");
			for(ITidAssemblePart* pAssmPart=pTowerInstance->EnumAssemblePartFirst();pAssmPart;pAssmPart=pTowerInstance->EnumAssemblePartNext(),serial++)
			{
				DisplayProcess((int)(100*serial/nSum+0.5),"生成杆件实体模型....");
				if(TaManager.m_ciInstanceLevel==1&&!pAssmPart->IsHasBriefRodLine())
					continue;	//只显示杆件实体
				ITidSolidBody* pSolidBody=pAssmPart->GetSolidPart();
				if(pSolidBody==NULL)
					continue;
				CSolidPart xSolidPart;
				xSolidPart.m_hPart=pAssmPart->GetId();
				xSolidPart.pBody=hashListBody.Add(serial);
				xSolidPart.pBody->CopyBuffer(pSolidBody->SolidBufferPtr(),pSolidBody->SolidBufferLength());
				if(TaManager.m_pActiveTa->m_bTurnLeft)
					xSolidPart.pBody->TransToACS(rot_cs);
				pView->SolidDraw()->NewSolidPart(xSolidPart);
			}
			if(TaManager.m_ciInstanceLevel==3)
			{	//放样实体级别，显示螺栓
				for(ITidAssembleBolt* pAssmBolt=pTowerInstance->EnumAssembleBoltFirst();pAssmBolt;pAssmBolt=pTowerInstance->EnumAssembleBoltNext(),serial++)
				{
					DisplayProcess((int)(100*serial/nSum+0.5),"生成螺栓实体模型....");
					ITidSolidBody* pBoltSolid=pAssmBolt->GetBoltSolid();
					ITidSolidBody* pNutSolid=pAssmBolt->GetNutSolid();
					if(pBoltSolid==NULL||pNutSolid==NULL)
						continue;
					CSolidPart xSolidPart;
					xSolidPart.m_hPart=pAssmBolt->GetId();
					xSolidPart.pBody=hashListBody.Add(serial);
					xSolidPart.pBody->CopyBuffer(pBoltSolid->SolidBufferPtr(),pBoltSolid->SolidBufferLength());
					xSolidPart.pBody->MergeBodyBuffer(pNutSolid->SolidBufferPtr(),pNutSolid->SolidBufferLength());
					if(TaManager.m_pActiveTa->m_bTurnLeft)
						xSolidPart.pBody->TransToACS(rot_cs);
					pView->SolidDraw()->NewSolidPart(xSolidPart);
				}
				for(ITidAssembleAnchorBolt* pAssmAnchorBolt=pTowerInstance->EnumFirstAnchorBolt();pAssmAnchorBolt;pAssmAnchorBolt=pTowerInstance->EnumNextAnchorBolt(),serial++)
				{
					DisplayProcess((int)(100*serial/nSum+0.5),"生成地脚螺栓实体模型....");
					ITidSolidBody* pBoltSolid=pAssmAnchorBolt->GetBoltSolid();
					ITidSolidBody* pNutSolid=pAssmAnchorBolt->GetNutSolid();
					if(pBoltSolid==NULL||pNutSolid==NULL)
						continue;
					CSolidPart xSolidPart;
					xSolidPart.m_hPart=pAssmAnchorBolt->GetId();
					xSolidPart.pBody=hashListBody.Add(serial);
					xSolidPart.pBody->CopyBuffer(pBoltSolid->SolidBufferPtr(),pBoltSolid->SolidBufferLength());
					xSolidPart.pBody->MergeBodyBuffer(pNutSolid->SolidBufferPtr(),pNutSolid->SolidBufferLength());
					if(TaManager.m_pActiveTa->m_bTurnLeft)
						xSolidPart.pBody->TransToACS(rot_cs);
					pView->SolidDraw()->NewSolidPart(xSolidPart);
				}
			}
			DisplayProcess(100,"生成实体模型....");
		}
		else
		{
			DisplayProcess(0,"生成单线模型...");
			for(ITidAssemblePart* pAssmPart=pTowerInstance->EnumAssemblePartFirst();pAssmPart;pAssmPart=pTowerInstance->EnumAssemblePartNext(),serial++)
			{
				DisplayProcess((int)(serial*100/nPart+0.5),"生成单线模型...");
				if(!pAssmPart->IsHasBriefRodLine())
					continue;
				UINT iNodeIdS=pAssmPart->GetStartNodeId();
				UINT iNodeIdE=pAssmPart->GetEndNodeId();
				ITidNode* pTidNodeS=pTowerInstance->FindNode(iNodeIdS);
				ITidNode* pTidNodeE=pTowerInstance->FindNode(iNodeIdE);
				if(pTidNodeS==NULL||pTidNodeE==NULL)
				{
					//logerr.Log("S:%d-E:%d",iNodeIdS,iNodeIdE);
					continue;
				}
				TID_COORD3D ptS=pAssmPart->BriefLineStart();
				TID_COORD3D ptE=pAssmPart->BriefLineEnd();
				f3dLine line;
				line.ID=pAssmPart->GetId();
				line.pen.crColor=RGB(220,220,220);
				line.startPt.Set(ptS.x,ptS.y,ptS.z);
				line.endPt.Set(ptE.x,ptE.y,ptE.z);
				if(TaManager.m_pActiveTa->m_bTurnLeft)
				{
					line.startPt=rot_cs.TransPToCS(line.startPt);
					line.endPt=rot_cs.TransPToCS(line.endPt);
				}
				pView->SolidDraw()->NewLine(line);
			}
			DisplayProcess(100,"生成单线模型...");
		}
	}
	else if(TaManager.m_pActiveTa->m_ciModelFlag==1)
	{	//MOD模型
		IModTowerInstance* pTowerInstance=(IModTowerInstance*)TaManager.m_pActiveTa->ExtractActiveTaInstance();
		if(pTowerInstance==NULL)
		{
			AfxMessageBox(CXhChar100("%s配基信息有误!",(char*)TaManager.m_pActiveTa->m_sTowerType));
			return;
		}
		DWORD serial=1;
		int nRod=pTowerInstance->GetModRodNum();
		if(disp_type==DISP_SOLID)
		{
			DisplayProcess(0,"生成实体模型....");
			for(IModRod* pRod=pTowerInstance->EnumModRodFir();pRod;pRod=pTowerInstance->EnumModRodNext(),serial++)
			{
				DisplayProcess((int)(serial*100/nRod+0.5),"生成实体模型...");
				//
				CSolidPart xSolidPart;
				xSolidPart.m_hPart=pRod->GetId();
				xSolidPart.pBody=hashListBody.Add(serial);
				pRod->Create3dSolidModel(xSolidPart.pBody);
				if(TaManager.m_pActiveTa->m_bTurnLeft)
					xSolidPart.pBody->TransToACS(rot_cs);
				pView->SolidDraw()->NewSolidPart(xSolidPart);
			}
			DisplayProcess(100,"生成实体模型....");
		}
		else
		{
			DisplayProcess(0,"生成单线模型...");
			for(IModRod* pRod=pTowerInstance->EnumModRodFir();pRod;pRod=pTowerInstance->EnumModRodNext(),serial++)
			{
				DisplayProcess((int)(serial*100/nRod+0.5),"生成单线模型...");
				//
				MOD_LINE mod_line=pRod->GetBaseLineToLdsModel();
				f3dLine line;
				line.ID=pRod->GetId();
				line.pen.crColor=RGB(220,220,220);
				line.startPt.Set(mod_line.startPt.x,mod_line.startPt.y,mod_line.startPt.z);
				line.endPt.Set(mod_line.endPt.x,mod_line.endPt.y,mod_line.endPt.z);
				if(TaManager.m_pActiveTa->m_bTurnLeft)
				{
					line.startPt=rot_cs.TransPToCS(line.startPt);
					line.endPt=rot_cs.TransPToCS(line.endPt);
				}
				pView->SolidDraw()->NewLine(line);
			}
			DisplayProcess(100,"生成单线模型...");
		}
	}
	pView->SolidOper()->ZoomAll(0.95);
}
#else
#include "MonoImage.h"
void CModelDataDlg::RedrawTowerSketchImage()
{
	CMonoImage image;
	//TODO:根据杆塔数据模型生成二维单线预览缩略图，并显示到窗口绘图Panel中 wjh-2019.4.10
	if(TaManager.m_pActiveTa==NULL)
		return;
	/*CLogErrorLife logErrLife;
	hashListBody.Empty();
	GECS dcs;
	if(TaManager.m_pActiveTa->m_ciModelFlag==0)
	{	//TID模型
		dcs=CastToUcs(TaManager.m_pActiveTa->m_pTidModel->ModelCoordSystem());
		//pView->SolidSet()->SetObjectUcs(dcs);
		ITidTowerInstance* pTowerInstance=(ITidTowerInstance*)TaManager.m_pActiveTa->ExtractActiveTaInstance();
		if(pTowerInstance==NULL)
		{
			AfxMessageBox(CXhChar100("%s配基信息有误!",(char*)TaManager.m_pActiveTa->m_sTowerType));
			return;
		}
		int nPart=pTowerInstance->GetAssemblePartNum();
		int nBolt=pTowerInstance->GetAssembleBoltNum();
		int nAnchorBolt=pTowerInstance->GetAssembleAnchorBoltNum();
		int nSum=nPart+nBolt+nAnchorBolt;
		int serial=1;
		DisplayProcess(0,"生成单线模型...");
		for(ITidAssemblePart* pAssmPart=pTowerInstance->EnumAssemblePartFirst();pAssmPart;pAssmPart=pTowerInstance->EnumAssemblePartNext(),serial++)
		{
			DisplayProcess((int)(serial*100/nPart+0.5),"生成单线模型...");
			if(!pAssmPart->IsHasBriefRodLine())
				continue;
			UINT iNodeIdS=pAssmPart->GetStartNodeId();
			UINT iNodeIdE=pAssmPart->GetEndNodeId();
			ITidNode* pTidNodeS=pTowerInstance->FindNode(iNodeIdS);
			ITidNode* pTidNodeE=pTowerInstance->FindNode(iNodeIdE);
			if(pTidNodeS==NULL||pTidNodeE==NULL)
			{
				//logerr.Log("S:%d-E:%d",iNodeIdS,iNodeIdE);
				continue;
			}
			TID_COORD3D ptS=pAssmPart->BriefLineStart();
			TID_COORD3D ptE=pAssmPart->BriefLineEnd();
			f3dLine line;
			line.startPt.Set(ptS.x,ptS.y,ptS.z);
			line.endPt.Set(ptE.x,ptE.y,ptE.z);
			line.pen.crColor=RGB(220,220,220);
			line.ID=pAssmPart->GetId();
			image.DrawLine(line.startPt,line.endPt);
			//pView->SolidDraw()->NewLine(line);
		}
		DisplayProcess(100,"生成单线模型...");
	}
	else if(TaManager.m_pActiveTa->m_ciModelFlag==1)
	{	//MOD模型
		IModTowerInstance* pTowerInstance=(IModTowerInstance*)TaManager.m_pActiveTa->ExtractActiveTaInstance();
		if(pTowerInstance==NULL)
		{
			AfxMessageBox(CXhChar100("%s配基信息有误!",(char*)TaManager.m_pActiveTa->m_sTowerType));
			return;
		}
		dcs.axis_x.Set(1,0,0);
		dcs.axis_y.Set(0,0,1);
		dcs.axis_z.Set(0,-1,0);
		//pView->SolidSet()->SetObjectUcs(dcs);
		DWORD serial=1;
		int nRod=pTowerInstance->GetModRodNum();
		DisplayProcess(0,"生成单线模型...");
		for(IModRod* pRod=pTowerInstance->EnumModRodFir();pRod;pRod=pTowerInstance->EnumModRodNext(),serial++)
		{
			DisplayProcess((int)(serial*100/nRod+0.5),"生成单线模型...");
			//
			MOD_LINE mod_line=pRod->GetBaseLineToLdsModel();
			f3dLine line;
			line.startPt.Set(mod_line.startPt.x,mod_line.startPt.y,mod_line.startPt.z);
			line.endPt.Set(mod_line.endPt.x,mod_line.endPt.y,mod_line.endPt.z);
			line.pen.crColor=RGB(220,220,220);
			line.ID=pRod->GetId();
			image.DrawLine(line.startPt,line.endPt);
			//pView->SolidDraw()->NewLine(line);
		}
		DisplayProcess(100,"生成单线模型...");
	}*/
}
#endif
void CModelDataDlg::RedrawTowerSketch()
{
#ifdef _HANDOVER_TO_CLIENT_
	RedrawTowerSketchImage();
#else
	m_pSolidDraw->BuildDisplayList(this);
	m_pSolidDraw->Draw();
#endif
}

//////////////////////////////////////////////////////////////////////////
// CMainDlg 对话框
IMPLEMENT_DYNAMIC(CModelDataDlg, CDialog)
CModelDataDlg::CModelDataDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CModelDataDlg::IDD, pParent)
	, m_sHeightName(_T(""))
	, m_sLegASerial(_T("1"))
	, m_sLegBSerial(_T("1"))
	, m_sLegCSerial(_T("1"))
	, m_sLegDSerial(_T("1"))
	, m_iLengthUnit(0)
{
#ifndef _HANDOVER_TO_CLIENT_
	m_pDrawSolid=NULL;
	m_pSolidDraw=NULL;
	m_pSolidOper=NULL;
	m_pSolidSet=NULL;
	m_pSolidSnap=NULL;
#endif
	m_iModelLevel=0;
	m_nDistR=0;
	m_nDistB=0;
	m_nBtnW=0;
	m_bExport3ds=TRUE;
	m_bExportXml=FALSE;
	m_bExportMod=FALSE;
	m_bBatchMode=FALSE;
}

CModelDataDlg::~CModelDataDlg()
{
#ifndef _HANDOVER_TO_CLIENT_
	if(m_pDrawSolid)
		CDrawSolidFactory::Destroy(m_pDrawSolid->GetSerial());
	m_pSolidDraw=NULL;
	m_pSolidOper=NULL;
	m_pSolidSet=NULL;
	m_pSolidSnap=NULL;
#endif
}

void CModelDataDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_CBString(pDX, IDC_CMB_HUGAO, m_sHeightName);
	DDX_CBString(pDX, IDC_CMB_LEG_A, m_sLegASerial);
	DDX_CBString(pDX, IDC_CMB_LEG_B, m_sLegBSerial);
	DDX_CBString(pDX, IDC_CMB_LEG_C, m_sLegCSerial);
	DDX_CBString(pDX, IDC_CMB_LEG_D, m_sLegDSerial);
	DDX_CBIndex(pDX, IDC_CMB_MODEL_LEVEL, m_iModelLevel);
	DDX_CBIndex(pDX, IDC_CMB_LENGTH_UNIT, m_iLengthUnit);
	DDX_Check(pDX, IDC_CHK_DWG, m_bExport3ds);
	DDX_Check(pDX, IDC_CHK_XML, m_bExportXml);
	DDX_Check(pDX, IDC_CHK_MOD, m_bExportMod);
	DDX_Control(pDX, IDC_LIST_CTRL, m_xListCtrl);
}


BEGIN_MESSAGE_MAP(CModelDataDlg, CDialog)
	ON_BN_CLICKED(IDC_BTN_SELECT_FILE,OnBnClickedSelectFile)
	ON_BN_CLICKED(IDC_BTN_PREVIEW, OnBnClickedPreview)
	ON_BN_CLICKED(IDC_BTN_EXPORT_FILE, OnBnClickedExportFile)
	ON_CBN_SELCHANGE(IDC_CMB_MODEL_LEVEL, &CModelDataDlg::OnCbnSelchangeCmbModelLevel)
	ON_CBN_SELCHANGE(IDC_CMB_LENGTH_UNIT, &CModelDataDlg::OnCbnSelchangeCmbLengthUnit)
	ON_CBN_SELCHANGE(IDC_CMB_HUGAO, &CModelDataDlg::OnCbnSelchangeCmbHugao)
	ON_CBN_SELCHANGE(IDC_CMB_LEG_A, &CModelDataDlg::OnCbnSelchangeCmbLegA)
	ON_CBN_SELCHANGE(IDC_CMB_LEG_B, &CModelDataDlg::OnCbnSelchangeCmbLegB)
	ON_CBN_SELCHANGE(IDC_CMB_LEG_C, &CModelDataDlg::OnCbnSelchangeCmbLegC)
	ON_CBN_SELCHANGE(IDC_CMB_LEG_D, &CModelDataDlg::OnCbnSelchangeCmbLegD)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_PAINT()
	ON_WM_CLOSE()
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CMainDlg 消息处理程序
BOOL CModelDataDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
#ifndef _HANDOVER_TO_CLIENT_
	//初始化绘图引擎
	m_pDrawSolid=CDrawSolidFactory::CreateDrawEngine();
	m_pSolidDraw=m_pDrawSolid->SolidDraw();
	m_pSolidSet=m_pDrawSolid->SolidSet();
	m_pSolidSnap=m_pDrawSolid->SolidSnap();
	m_pSolidOper=m_pDrawSolid->SolidOper();
	InitDrawSolidInfo();
	if(m_iModelLevel==0)
		m_pSolidSet->SetDisplayType(DISP_LINE);
	else
		m_pSolidSet->SetDisplayType(DISP_SOLID);
	m_pSolidDraw->BuildDisplayList(this);
#endif
	m_iModelLevel=TaManager.m_ciInstanceLevel;
	//
	CComboBox* pCmbLevel=(CComboBox*)GetDlgItem(IDC_CMB_LENGTH_UNIT);
	pCmbLevel->ResetContent();
	pCmbLevel->AddString("毫米(mm)");
	pCmbLevel->AddString("米(m)");
	pCmbLevel->SetCurSel(0);
	//
	CRect rect,clientRect;
	GetClientRect(&clientRect);
	GetDlgItem(IDC_WORK_PANEL)->GetWindowRect(&rect);
	ScreenToClient(&rect);
	m_nDistR=clientRect.right-rect.right;
	m_nDistB=clientRect.bottom-rect.bottom;
	GetDlgItem(IDC_BTN_SELECT_FILE)->GetWindowRect(&rect);
	m_nBtnW=rect.Width();
	GetDlgItem(IDC_WORK_PANDLE2)->GetWindowRect(&rect);
	ScreenToClient(&rect);
	m_xListCtrl.MoveWindow(&rect);
	m_xListCtrl.AddColumnHeader("序号",40);
	m_xListCtrl.AddColumnHeader("塔位",100);
	m_xListCtrl.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP);
	m_xListCtrl.InitListCtrl();
	m_xListCtrl.SetDblclkFunc(DblclkListCtrl);
	//
	RereshContrl();
	UpdateData(FALSE);
	return TRUE;
}
void CModelDataDlg::InitDrawSolidInfo()
{
	UCS_STRU ucs;
	ucs.axis_x.Set(1,0,0);
	ucs.axis_y.Set(0,0,1);
	ucs.axis_z.Set(0,-1,0);
	CWnd *pWnd=GetDlgItem(IDC_WORK_PANEL);
#ifndef _HANDOVER_TO_CLIENT_
	m_pSolidSet->Init(pWnd->GetSafeHwnd(),GetSafeHwnd());
	m_pSolidSet->SetBkColor(RGB(0,64,160));
	m_pSolidSet->SetDisplayFunc(DrawSolidTower);
	m_pSolidSet->SetSolidAndLineStatus(FALSE);
	m_pSolidSet->SetArcSamplingLength(5);
	m_pSolidSet->SetSmoothness(36);
	m_pSolidSet->SetDisplayLineVecMark(FALSE);
	m_pSolidSet->SetRotOrg(f3dPoint(0,0,0));
	m_pSolidSet->SetObjectUcs(ucs);
	m_pSolidSet->SetZoomStyle(ROT_CENTER);
	m_pSolidOper->ZoomAll(0.95);		//需进行缩放，否则界面显示过小而看不清
	m_pSolidDraw->InitialUpdate();
#endif
}
void CModelDataDlg::OnOK()
{

}
void CModelDataDlg::OnCancel()
{
#ifndef _HANDOVER_TO_CLIENT_
	m_pSolidSet->SetOperType(OPER_OTHER);
	m_pSolidDraw->ReleaseSnapStatus();	
#endif
}
void CModelDataDlg::OnPaint()
{
	CPaintDC dc(this);				//
#ifndef _HANDOVER_TO_CLIENT_
	m_pSolidDraw->Draw();
#endif
}
void CModelDataDlg::OnClose()
{
	CDialog::OnCancel();
}
void CModelDataDlg::RereshContrl()
{
	if(m_bBatchMode)
	{
		GetDlgItem(IDC_CMB_HUGAO)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_CMB_LEG_A)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_CMB_LEG_B)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_CMB_LEG_C)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_CMB_LEG_D)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_S_HUGAO)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_S_LEGA)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_S_LEGA)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_S_LEGA)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_S_INSTANCE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_LIST_CTRL)->ShowWindow(SW_SHOW);
	}
	else
	{
		GetDlgItem(IDC_CMB_HUGAO)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_CMB_LEG_A)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_CMB_LEG_B)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_CMB_LEG_C)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_CMB_LEG_D)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_S_HUGAO)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_S_LEGA)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_S_LEGA)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_S_LEGA)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_S_INSTANCE)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_LIST_CTRL)->ShowWindow(SW_HIDE);
	}
}
void CModelDataDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
#ifndef _HANDOVER_TO_CLIENT_
	m_pSolidSet->SetOperType(OPER_ROTATE);	//为便捷操作，双击时激活旋转操作
	if((nFlags&MK_CONTROL)==0)	//只有未按下Control键时的双击才会激发双击更改旋转中心操作
		m_pSolidOper->LMouseDoubleClick(point);
#endif
	CDialog::OnLButtonDblClk(nFlags, point);
}
void CModelDataDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	CRect rect;
	CWnd *pWnd=GetDlgItem(IDC_WORK_PANEL);
	pWnd->GetWindowRect(&rect);
	ScreenToClient(&rect);
	if(!rect.PtInRect(point))
		return;
	pWnd->SetFocus();	//否则无法方便获取屏幕焦点，进而无法通过滚轮进行缩放wjh-2014.12.04
	SetCapture();
	BOOL bAltKeyDown=GetKeyState(VK_MENU)&0x8000;
#ifndef _HANDOVER_TO_CLIENT_
	if(nFlags&MK_SHIFT)
	{
		m_pSolidDraw->ReleaseSnapStatus();
		m_pSolidDraw->BatchClearCS(4);
	}
	m_pSolidOper->LMouseDown(point);
#endif
	CDialog::OnLButtonDown(nFlags, point);
}

void CModelDataDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
#ifndef _HANDOVER_TO_CLIENT_
	m_pSolidOper->LMouseUp(point);
#endif
	if(GetCapture()==this)
		ReleaseCapture();
	CDialog::OnLButtonUp(nFlags, point);
}

void CModelDataDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	CRect rect;
	CWnd *pWnd=GetDlgItem(IDC_WORK_PANEL);
	pWnd->GetWindowRect(&rect);
	ScreenToClient(&rect);
	if(!rect.PtInRect(point))
		return;
#ifndef _HANDOVER_TO_CLIENT_
	m_pSolidOper->MouseMove(point,nFlags);
#endif
	CDialog::OnMouseMove(nFlags, point);
}

BOOL CModelDataDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
#ifndef _HANDOVER_TO_CLIENT_
	ZOOM_CENTER_STYLE zoom_style;
	m_pSolidSet->GetZoomStyle(&zoom_style);
	m_pSolidSet->SetZoomStyle(MOUSE_CENTER);
	if(zDelta>0)
		m_pSolidOper->Scale(1.4);
	else
		m_pSolidOper->Scale(1/1.4);
	m_pSolidOper->RefreshScope();
	m_pSolidSet->SetZoomStyle(zoom_style);
#endif
	return CDialog::OnMouseWheel(nFlags, zDelta, pt);
}

void CModelDataDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
#ifndef _HANDOVER_TO_CLIENT_
	m_pSolidOper->RMouseDown(point);
	m_pSolidSet->SetOperType(OPER_OTHER);
	m_pSolidDraw->ReleaseSnapStatus();
#endif
	CDialog::OnRButtonDown(nFlags, point);
}

void CModelDataDlg::OnRButtonUp(UINT nFlags, CPoint point)
{
	CDialog::OnRButtonUp(nFlags, point);
}

void CModelDataDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
#ifndef _HANDOVER_TO_CLIENT_
	if(nChar==VK_ESCAPE)
	{
		m_pSolidSnap->SetSnapType(SNAP_ALL);
		m_pSolidSet->SetOperType(OPER_OTHER);
	}
	//在nChar==VK_DELETE时返回实际删除的构件数量,用以处理删除当前未显示构件的情况
	m_pSolidOper->KeyDown(nChar);
#endif
	CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CModelDataDlg::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CDialog::OnKeyUp(nChar, nRepCnt, nFlags);
}

void CModelDataDlg::OnBnClickedSelectFile()
{
	CLogErrorLife logErrLife(&xMyErrLog);
	CXhChar500 filter("铁塔三维数据模型文件(*.tid)|*.tid");
	filter.Append("|国网移交几何模型文件(*.mod)|*.mod");
	filter.Append("|批量生成文件(*.xls)|*.xls|批量生成文件(*.xlsx)|*.xlsx");
	filter.Append("|所有文件(*.*)|*.*||");
	CFileDialog file_dlg(TRUE,"tower","Tower",OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,filter);
	if(file_dlg.DoModal()!=IDOK)
		return;
	m_sFileName=file_dlg.GetPathName();
	m_bBatchMode=FALSE;
	CXhChar16 extension;
	_splitpath(m_sFileName,NULL,NULL,NULL,extension);
	TaManager.Empty();
	if(extension.EqualNoCase(".tid"))
		TaManager.ReadTidFile(m_sFileName);
	else if(extension.EqualNoCase(".mod"))
		TaManager.ReadModFile(m_sFileName);
	else if(extension.EqualNoCase(".xls")||extension.EqualNoCase(".xlsx"))
	{	
		TaManager.ReadExcleFile(m_sFileName);
		m_bBatchMode=TRUE;
	}
	//更新精度等级
	CComboBox* pCmbLevel=(CComboBox*)GetDlgItem(IDC_CMB_MODEL_LEVEL);
	pCmbLevel->ResetContent();
	pCmbLevel->AddString("LOD0");
	pCmbLevel->AddString("LOD1");
	if(!extension.EqualNoCase(".mod"))
	{
		pCmbLevel->AddString("LOD2");
		pCmbLevel->AddString("LOD3");
	}
	pCmbLevel->SetCurSel(0);
	//更新配基控件内容及状态
	CEdit* pEdit=(CEdit*)GetDlgItem(IDC_E_SELE_FILE);
	pEdit->SetWindowText(file_dlg.GetPathName());
	RereshContrl();
	if(m_bBatchMode)
	{	//
		m_xListCtrl.DeleteAllItems();
		int iNo=1;
		for(CTowerModel* pTower=TaManager.EnumFirstTaModel();pTower;pTower=TaManager.EnumNextTaModel(),iNo++)
		{
			if(pTower->m_ciErrorType>0)
				continue;	//不显示错误塔例
			CStringArray str_arr;
			str_arr.SetSize(2);
			str_arr[0].Format("%d",iNo);
			str_arr[1].Format("%s",(char*)pTower->m_sTowerPlaceCode);
			int iCur=m_xListCtrl.InsertItemRecord(-1,str_arr);
			m_xListCtrl.SetItemData(iCur,(DWORD)pTower);
		}
	}
	else if(TaManager.m_pActiveTa)
	{
		CComboBox* pCmbHugao=(CComboBox*)GetDlgItem(IDC_CMB_HUGAO);
		pCmbHugao->ResetContent();
		if(TaManager.m_pActiveTa->m_ciModelFlag==0)
		{
			ITidModel* pModel=TaManager.m_pTidModel;
			for(int i=0;i<pModel->HeightGroupCount();i++)
			{
				CXhChar50 sName;
				ITidHeightGroup* pHuGao=pModel->GetHeightGroupAt(i);
				pHuGao->GetName(sName,50);
				pCmbHugao->AddString(sName);
			}
		}
		if(TaManager.m_pActiveTa->m_ciModelFlag==1)
		{
			IModModel* pModel=TaManager.m_pModModel;
			for(IModHeightGroup* pHuGao=pModel->EnumFirstHGroup();pHuGao;pHuGao=pModel->EnumNextHGroup())
			{
				CXhChar50 sName;
				pHuGao->GetName(sName);
				pCmbHugao->AddString(sName);
			}
		}
		pCmbHugao->SetCurSel(0);
		OnCbnSelchangeCmbHugao();
	}
	UpdateData(FALSE);
}
void CModelDataDlg::OnBnClickedPreview()
{
	if(m_bBatchMode)
	{
		int iSelItem=m_xListCtrl.GetSelectedItem();
		if(iSelItem<0)
			return;
		TaManager.m_pActiveTa=(CTowerModel*)m_xListCtrl.GetItemData(iSelItem);
	}
#ifndef _HANDOVER_TO_CLIENT_
	m_pSolidDraw->BuildDisplayList(this);
	m_pSolidDraw->Draw();
#endif
}
void CModelDataDlg::OnBnClickedExportFile()
{
	UpdateData(TRUE);
	if(strlen(m_sFileName)<=0)
		return;
	int index=m_sFileName.ReverseFind('\\');	//反向查找'\\'
	CString sFilePath=m_sFileName.Left(index);	//移除文件名
	CXhChar100 sWorkDir;
	if(m_bExport3ds)
	{
		sWorkDir.Printf("%s\\3DS",sFilePath);
		MakeDirectory(sWorkDir);
		TaManager.Create3dsFiles(sWorkDir);
	}
	if(m_bExportXml)
	{
		sWorkDir.Printf("%s\\XML",sFilePath);
		MakeDirectory(sWorkDir);
		TaManager.CreateXmlFiles(sWorkDir);
	}
	if(m_bExportMod)
	{
		sWorkDir.Printf("%s\\MOD",sFilePath);
		MakeDirectory(sWorkDir);
		TaManager.CreateModFiles(sWorkDir);
	}
	if(sWorkDir.GetLength()>0)
		WinExec(CXhChar500("explorer.exe %s",(char*)sWorkDir),SW_SHOW);
}
void CModelDataDlg::OnCbnSelchangeCmbModelLevel()
{
	UpdateData(TRUE);
	TaManager.m_ciInstanceLevel=m_iModelLevel;
#ifndef _HANDOVER_TO_CLIENT_
	if(m_iModelLevel==0)
		m_pSolidSet->SetDisplayType(DISP_LINE);
	else
		m_pSolidSet->SetDisplayType(DISP_SOLID);
#endif
	UpdateData(FALSE);
}

void CModelDataDlg::OnCbnSelchangeCmbLengthUnit()
{
	UpdateData(TRUE);
	if(m_iLengthUnit==0)	//毫米
		TaManager.m_bUseUnitM=FALSE;
	else					//米
		TaManager.m_bUseUnitM=TRUE;
	UpdateData(FALSE);
}

void CModelDataDlg::OnCbnSelchangeCmbHugao()
{
	UpdateData(TRUE);
	m_hashSubLeg.Empty();
	if(TaManager.m_pActiveTa->m_ciModelFlag==0)
	{
		ITidHeightGroup* pHuGao=NULL;
		for(int i=0;i<TaManager.m_pTidModel->HeightGroupCount();i++)
		{
			CXhChar50 sName;
			ITidHeightGroup* pTemHuGao=TaManager.m_pTidModel->GetHeightGroupAt(i);
			if(pTemHuGao)
				pTemHuGao->GetName(sName,50);
			if(sName.EqualNoCase(m_sHeightName))
			{
				pHuGao=pTemHuGao;
				break;
			}
		}
		if(pHuGao)
		{
			ARRAY_LIST<int> legSerialArr;
			TaManager.m_pActiveTa->m_uiHeightSerial=pHuGao->GetSerialId();
			int nLeg=pHuGao->GetLegSerialArr(NULL);
			legSerialArr.SetSize(nLeg);
			pHuGao->GetLegSerialArr(legSerialArr.m_pData);
			for(int i=0;i<nLeg;i++)
			{
				double fValue=pHuGao->GetLegHeightDifference(legSerialArr[i]);
				m_hashSubLeg.SetValue(CXhChar16(fValue),legSerialArr[i]);
			}
		}
	}
	if(TaManager.m_pActiveTa->m_ciModelFlag==1)
	{
		IModHeightGroup* pHuGao=TaManager.m_pModModel->GetHeightGroup(m_sHeightName);
		if(pHuGao)
		{
			TaManager.m_pActiveTa->m_uiHeightSerial=pHuGao->GetNo();
			for(int *pLegI=pHuGao->EnumFirstLegSerial();pLegI;pLegI=pHuGao->EnumNextLegSerial())
			{
				double fValue=pHuGao->GetLegDiffDist(*pLegI);
				m_hashSubLeg.SetValue(CXhChar16(fValue),*pLegI);
			}
		}
	}
	//更新配腿信息
	int nLegSize=m_hashSubLeg.GetNodeNum();
	if(nLegSize>0)
	{	
		CString sText;
		//A腿
		CComboBox* pCmbLegA=(CComboBox*)GetDlgItem(IDC_CMB_LEG_A);
		pCmbLegA->ResetContent();
		for(int *pLegI=m_hashSubLeg.GetFirst();pLegI;pLegI=m_hashSubLeg.GetNext())
		{
			sText.Format("%s",m_hashSubLeg.GetCursorKey());
			pCmbLegA->AddString(sText);
		}
		pCmbLegA->GetLBText(0,m_sLegASerial);
		pCmbLegA->SetCurSel(0);
		//B腿
		CComboBox* pCmbLegB=(CComboBox*)GetDlgItem(IDC_CMB_LEG_B);
		pCmbLegB->ResetContent();
		for(int *pLegI=m_hashSubLeg.GetFirst();pLegI;pLegI=m_hashSubLeg.GetNext())
		{
			sText.Format("%s",m_hashSubLeg.GetCursorKey());
			pCmbLegB->AddString(sText);
		}
		pCmbLegB->GetLBText(0,m_sLegBSerial);
		pCmbLegB->SetCurSel(0);
		//C腿
		CComboBox* pCmbLegC=(CComboBox*)GetDlgItem(IDC_CMB_LEG_C);
		pCmbLegC->ResetContent();
		for(int *pLegI=m_hashSubLeg.GetFirst();pLegI;pLegI=m_hashSubLeg.GetNext())
		{
			sText.Format("%s",m_hashSubLeg.GetCursorKey());
			pCmbLegC->AddString(sText);
		}
		pCmbLegC->GetLBText(0,m_sLegCSerial);
		pCmbLegC->SetCurSel(0);
		//D腿
		CComboBox* pCmbLegD=(CComboBox*)GetDlgItem(IDC_CMB_LEG_D);
		pCmbLegD->ResetContent();
		for(int *pLegI=m_hashSubLeg.GetFirst();pLegI;pLegI=m_hashSubLeg.GetNext())
		{
			sText.Format("%s",m_hashSubLeg.GetCursorKey());
			pCmbLegD->AddString(sText);
		}
		pCmbLegD->GetLBText(0,m_sLegDSerial);
		pCmbLegD->SetCurSel(0);
	}
	UpdateData(FALSE);
}
//A腿对应第二象限
void CModelDataDlg::OnCbnSelchangeCmbLegA()
{
	UpdateData(TRUE);
	if(TaManager.m_pActiveTa)
		TaManager.m_pActiveTa->m_uiLegSerialArr[1]=*m_hashSubLeg.GetValue(m_sLegASerial);
	UpdateData(FALSE);
}
//B腿对应第四象限
void CModelDataDlg::OnCbnSelchangeCmbLegB()
{
	UpdateData(TRUE);
	if(TaManager.m_pActiveTa)
		TaManager.m_pActiveTa->m_uiLegSerialArr[3]=*m_hashSubLeg.GetValue(m_sLegBSerial);
	UpdateData(FALSE);
}
//C腿对应第三象限
void CModelDataDlg::OnCbnSelchangeCmbLegC()
{
	UpdateData(TRUE);
	if(TaManager.m_pActiveTa)
		TaManager.m_pActiveTa->m_uiLegSerialArr[2]=*m_hashSubLeg.GetValue(m_sLegCSerial);
	UpdateData(FALSE);
}
//D腿对应第一象限
void CModelDataDlg::OnCbnSelchangeCmbLegD()
{
	UpdateData(TRUE);
	if(TaManager.m_pActiveTa)
		TaManager.m_pActiveTa->m_uiLegSerialArr[0]=*m_hashSubLeg.GetValue(m_sLegDSerial);
	UpdateData(FALSE);
}
void CModelDataDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	RECT rect;
	CWnd* pWnd=GetDlgItem(IDC_WORK_PANEL);
	if(pWnd->GetSafeHwnd()!=NULL)
	{
		pWnd->GetWindowRect(&rect);
		ScreenToClient(&rect);
		rect.right=cx-m_nDistR;
		rect.bottom=cy-m_nDistB;
		pWnd->MoveWindow(&rect);
	}
	pWnd=GetDlgItem(IDC_BTN_SELECT_FILE);
	if(pWnd->GetSafeHwnd()!=NULL)
	{
		pWnd->GetWindowRect(&rect);
		ScreenToClient(&rect);
		rect.right=cx-m_nDistR;
		rect.left=rect.right-m_nBtnW;
		pWnd->MoveWindow(&rect);
	}
	pWnd=GetDlgItem(IDC_E_SELE_FILE);
	if(pWnd->GetSafeHwnd()!=NULL)
	{
		pWnd->GetWindowRect(&rect);
		ScreenToClient(&rect);
		rect.right=cx-m_nDistR-m_nBtnW-1;
		pWnd->MoveWindow(&rect);
	}
#ifndef _HANDOVER_TO_CLIENT_
	if(m_pSolidOper)
	{	//调整绘图区域
		m_pSolidOper->ReSize();
		m_pSolidOper->ZoomAll(0.95);
	}
#endif
}
