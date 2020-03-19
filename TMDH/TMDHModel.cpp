#include "stdafx.h"
#include "TMDHModel.h"
#include "CryptBuffer.h"
#include "ExcelOper.h"
#include <direct.h>
#include "GlobalFunc.h"
#include "ExcelColumn.h"
#include "XmlData.h"
#include "3DS.h"
#include "DxfFile.h"
#include "ISTL.h"
#include "ProductDef.h"
#include "ElectricTower.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CTMDHManager Manager;
CLogFile xMyErrLog("D:\\warning.log");
//////////////////////////////////////////////////////////////////////////
//
static int comparefun(const NODE_PTR& item1,const NODE_PTR& item2)
{
	return compare_long(item1->pointI,item2->pointI);
}
static BOOL IsHasValidPointI(CHashSet<NODE_PTR>& hashLinkNode)
{
	CHashList<BOOL> hashByPointI;
	for(CLDSNode* pNode=hashLinkNode.GetFirst();pNode;pNode=hashLinkNode.GetNext())
	{
		if(pNode->point_i<=0)
			return FALSE;
		if(hashByPointI.GetValue(pNode->point_i))
			return FALSE;
		hashByPointI.SetValue(pNode->point_i,TRUE);
	}
	return TRUE;
}
GECS TransToUcs(MOD_CS modCs)
{
	GECS cs;
	cs.origin.Set(modCs.origin.x,modCs.origin.y,modCs.origin.z);
	cs.axis_x.Set(modCs.axisX.x,modCs.axisX.y,modCs.axisX.z);
	cs.axis_y.Set(modCs.axisY.x,modCs.axisY.y,modCs.axisY.z);
	cs.axis_z.Set(modCs.axisZ.x,modCs.axisZ.y,modCs.axisZ.z);
	return cs;
}
static CXhChar100 VariantToString(VARIANT value)
{
	CXhChar100 sValue;
	if(value.vt==VT_BSTR)
		return sValue.Copy(CString(value.bstrVal));
	else if(value.vt==VT_R8)
		return sValue.Copy(CXhChar100(value.dblVal));
	else if(value.vt==VT_R4)
		return sValue.Copy(CXhChar100(value.fltVal));
	else if(value.vt==VT_INT)
		return sValue.Copy(CXhChar100(value.intVal));
	else 
		return sValue;
}
//////////////////////////////////////////////////////////////////////////
//CTMDHModel
CTMDHModel::CTMDHModel()
{
	m_ciErrorType=0;
	m_ciModelFlag=0;
	m_uiHeightSerial=1;
	for(int i=0;i<4;i++)
		m_uiLegSerialArr[i]=1;
	m_fLocateH=0;
	m_fBodyHeight=0;
	m_fMaxLegHeight=0;
	m_bTurnLeft=FALSE;
}
CTMDHModel::~CTMDHModel()
{
}
//杆塔配基中要求左转时，需要将杆塔模型水平旋转180°
//即杆塔模型坐标系中的X轴和Y轴*-1
GECS CTMDHModel::BuildRotateCS()
{
	GECS cs;
	cs.origin.Set(0,0,0);
	cs.axis_x.Set(-1,0,0);
	cs.axis_y.Set(0,-1,0);
	cs.axis_z.Set(0,0,1);
	return cs;
}
/*建立转换坐标系，三维点坐标从LDS模型坐标系转换到GIM坐标系中
//LDS模型坐标：			GIM模型坐标：		 从LDS-->GIM的转换坐标系
//	AxisX(1,0,0)		  AxisX(1,0,0)			AxisX(1,0,0)
//	AxisY(0,0,1)		  AxisY(0,0,-1)			AxisY(0,-1,0)
//	AxisZ(0,-1,0)		  AxisZ(0,1,0)			AxisZ(0,0,-1)
//  原点在顶部			 原点在底部
*/
GECS CTMDHModel::BuildTransCS(void* pInstance/*=NULL*/)
{
	GECS cs;
	if(m_ciModelFlag==TYPE_TID)
	{	//TID模型坐标系与LDS模型坐标系一致
		if(pInstance==NULL)
			pInstance=ExtractActiveTaInstance();
		double fHeight=((ITidTowerInstance*)pInstance)->GetInstanceHeight();
		cs.origin.Set(0,0,fHeight);
		cs.axis_x.Set(1,0,0);
		cs.axis_y.Set(0,-1,0);
		cs.axis_z.Set(0,0,-1);	
	}
	else if(m_ciModelFlag==TYPE_MOD)
	{	//MOD模型坐标系与GIM模型坐标系一致
		cs.origin.Set(0,0,0);
		cs.axis_x.Set(1,0,0);
		cs.axis_y.Set(0,1,0);
		cs.axis_z.Set(0,0,1);
	}
	return cs;
}
CXhChar100 CTMDHModel::GetFileName(void* pInstance)
{
	double dfNamedHeight=0;	//呼高(V1.4增加属性,旧版返回0)，单位m  wjh-2019.4.25
	double fLegA=0,fLegB=0,fLegC=0,fLegD=0;
	if(m_ciModelFlag==TYPE_TID)
	{
		ITidTowerInstance* pTidInstance=(ITidTowerInstance*)pInstance;
		dfNamedHeight=pTidInstance->BelongHeightGroup()->GetNamedHeight()*0.001;
		fLegA=pTidInstance->BelongHeightGroup()->GetLegHeightDifference(m_uiLegSerialArr[0]);
		fLegB=pTidInstance->BelongHeightGroup()->GetLegHeightDifference(m_uiLegSerialArr[1]);
		fLegC=pTidInstance->BelongHeightGroup()->GetLegHeightDifference(m_uiLegSerialArr[2]);
		fLegD=pTidInstance->BelongHeightGroup()->GetLegHeightDifference(m_uiLegSerialArr[3]);
	}
	else if(m_ciModelFlag==TYPE_MOD)
	{
		IModTowerInstance* pModInstance=(IModTowerInstance*)pInstance;
		dfNamedHeight=pModInstance->BelongHuGao()->GetNamedHeight()*0.001;
		fLegA=pModInstance->BelongHuGao()->GetLegDiffDist(m_uiLegSerialArr[0]);
		fLegB=pModInstance->BelongHuGao()->GetLegDiffDist(m_uiLegSerialArr[1]);
		fLegC=pModInstance->BelongHuGao()->GetLegDiffDist(m_uiLegSerialArr[2]);
		fLegD=pModInstance->BelongHuGao()->GetLegDiffDist(m_uiLegSerialArr[3]);
	}
	CMaxDouble maxValue;
	maxValue.Update(fLegA);
	maxValue.Update(fLegB);
	maxValue.Update(fLegC);
	maxValue.Update(fLegD);
	CXhChar100 sFileName=m_sTowerNum;
	if(maxValue.IsInited())
	{
		double fMaxSubtractLegH=maxValue.number;
		if(dfNamedHeight==0)
		{
			CXhChar50 sHuGao(m_sHuGao);
			sHuGao.Remove('m');
			sHuGao.Remove('M');
			dfNamedHeight=atof(sHuGao);
		}
		double dfActualHeight=dfNamedHeight+fMaxSubtractLegH;
		sFileName.Append(CXhChar50("%s-%gm-LOD%d",(char*)m_sTowerType,dfActualHeight,Manager.m_ciInstanceLevel),' ');
	}
	else
		sFileName.Append(CXhChar50("%s-error-LOD%d",(char*)m_sTowerType,Manager.m_ciInstanceLevel),' ');
	return sFileName;
}
/*配腿与象限的对照关系
   象限		  正常模型中配腿	左转模型中配腿
Serial[0]		LegD				LegB
Serial[1]		LegA				LegC
Serial[2]		LegC				LegA
Serial[3]		LegB				LegD
*/
BOOL CTMDHModel::ExtractActiveTaInstance(char* sHuGao,double fLegA,double fLegB,double fLegC,double fLegD)
{
	if(m_ciModelFlag==TYPE_TID)
	{
		ITidModel* pTidModel=Manager.m_pTidModel;
		if(pTidModel==NULL || !pTidModel->ReadTidFile(m_sFilePath))
		{
			m_ciErrorType=ERROR_READ_FILE;
			return FALSE;
		}
		ITidHeightGroup* pHeightGroup=NULL;
		for(int i=0;i<pTidModel->HeightGroupCount();i++)
		{
			CXhChar50 sName;
			ITidHeightGroup* pTemHuGao=pTidModel->GetHeightGroupAt(i);
			if(pTemHuGao)
				pTemHuGao->GetName(sName,50);
			if(sName.EqualNoCase(sHuGao))
			{
				pHeightGroup=pTemHuGao;
				break;
			}
		}
		if(pHeightGroup==NULL)
		{
			m_ciErrorType=ERROR_FIND_HUGAO;
			return FALSE;
		}
		if(fLegA>0 && fLegB>0 && fLegC>0 && fLegD>0)
		{	//加腿模式，需要转化为减腿数据
			fLegA-=m_fMaxLegHeight;
			fLegB-=m_fMaxLegHeight;
			fLegC-=m_fMaxLegHeight;
			fLegD-=m_fMaxLegHeight;
		}
		m_uiHeightSerial=pHeightGroup->GetSerialId();
		int iLegASerial=pHeightGroup->GetLegSerial(fLegA);
		int iLegBSerial=pHeightGroup->GetLegSerial(fLegB);
		int iLegCSerial=pHeightGroup->GetLegSerial(fLegC);
		int iLegDSerial=pHeightGroup->GetLegSerial(fLegD);
		if(iLegASerial>0&&iLegBSerial>0&&iLegCSerial>0&&iLegDSerial>0)
		{
			if(m_bTurnLeft)
			{
				m_uiLegSerialArr[1]=iLegCSerial;
				m_uiLegSerialArr[3]=iLegDSerial;
				m_uiLegSerialArr[2]=iLegASerial;
				m_uiLegSerialArr[0]=iLegBSerial;
			}
			else
			{
				m_uiLegSerialArr[1]=iLegASerial;
				m_uiLegSerialArr[3]=iLegBSerial;
				m_uiLegSerialArr[2]=iLegCSerial;
				m_uiLegSerialArr[0]=iLegDSerial;
			}
		}
		else
		{
			m_ciErrorType=ERROR_FIND_LEG;
			return FALSE;
		}
		ITidTowerInstance* pInstance=pHeightGroup->GetTowerInstance(m_uiLegSerialArr[0],m_uiLegSerialArr[1],m_uiLegSerialArr[2],m_uiLegSerialArr[3]);
		if(pInstance==NULL)
		{
			m_ciErrorType=ERROR_FIND_INSTANCE;
			return FALSE;
		}
	}
	else if(m_ciModelFlag==TYPE_MOD)
	{
		IModModel* pModModel=Manager.m_pModModel;
		if(pModModel==NULL || !pModModel->ImportModFile(m_sFilePath))
			return FALSE;
		IModHeightGroup* pHeightGroup=pModModel->GetHeightGroup(sHuGao);
		if(pHeightGroup==NULL)
		{
			m_ciErrorType=ERROR_FIND_HUGAO;
			return FALSE;
		}
		if(fLegA>0 && fLegB>0 && fLegC>0 && fLegD>0)
		{	//加腿模式，需要转化为减腿数据
			fLegA-=m_fMaxLegHeight;
			fLegB-=m_fMaxLegHeight;
			fLegC-=m_fMaxLegHeight;
			fLegD-=m_fMaxLegHeight;
		}
		m_uiHeightSerial=pHeightGroup->GetNo();
		int *pLegASerial=pHeightGroup->GetLegSerialId(CXhChar50(fLegA));
		int *pLegBSerial=pHeightGroup->GetLegSerialId(CXhChar50(fLegB));
		int *pLegCSerial=pHeightGroup->GetLegSerialId(CXhChar50(fLegC));
		int *pLegDSerial=pHeightGroup->GetLegSerialId(CXhChar50(fLegD));
		if(pLegASerial&&pLegBSerial&&pLegCSerial&&pLegDSerial)
		{
			if(m_bTurnLeft)
			{
				m_uiLegSerialArr[1]=*pLegCSerial;
				m_uiLegSerialArr[3]=*pLegDSerial;
				m_uiLegSerialArr[2]=*pLegASerial;
				m_uiLegSerialArr[0]=*pLegBSerial;
			}
			else
			{
				m_uiLegSerialArr[1]=*pLegASerial;
				m_uiLegSerialArr[3]=*pLegBSerial;
				m_uiLegSerialArr[2]=*pLegCSerial;
				m_uiLegSerialArr[0]=*pLegDSerial;
			}
		}
		else
		{
			m_ciErrorType=ERROR_FIND_LEG;
			return FALSE;
		}
		IModTowerInstance* pInstance=pHeightGroup->GetTowerInstance(m_uiLegSerialArr[0],m_uiLegSerialArr[1],m_uiLegSerialArr[2],m_uiLegSerialArr[3]);
		if(pInstance==NULL)
		{
			m_ciErrorType=ERROR_FIND_INSTANCE;
			return FALSE;
		}
	}
	return TRUE;
}
void* CTMDHModel::ExtractActiveTaInstance()
{
	if(m_ciModelFlag==TYPE_TID)
	{
		ITidModel* pTidModel=Manager.m_pTidModel;
		if(pTidModel==NULL || !pTidModel->ReadTidFile(m_sFilePath))
		{
			m_ciErrorType=ERROR_READ_FILE;
			return NULL;
		}
		ITidHeightGroup* pHeightGroup=pTidModel->GetHeightGroupAt(m_uiHeightSerial-1);
		if(pHeightGroup==NULL)
		{
			m_ciErrorType=ERROR_FIND_HUGAO;
			return NULL;
		}
		pHeightGroup->GetName(m_sHuGao,50);
		ITidTowerInstance* pInstance=NULL;
		pInstance=pHeightGroup->GetTowerInstance(m_uiLegSerialArr[0],m_uiLegSerialArr[1],m_uiLegSerialArr[2],m_uiLegSerialArr[3]);
		if(pInstance==NULL)
		{
			m_ciErrorType=ERROR_FIND_INSTANCE;
			return NULL;
		}
		return pInstance;
	}
	else if(m_ciModelFlag==TYPE_MOD)
	{
		IModModel* pModModel=Manager.m_pModModel;
		if(pModModel==NULL || !pModModel->ImportModFile(m_sFilePath))
		{
			m_ciErrorType=ERROR_READ_FILE;
			return NULL;
		}
		IModHeightGroup* pHeightGroup=pModModel->GetHeightGroup(m_uiHeightSerial);
		if(pHeightGroup==NULL)
		{
			m_ciErrorType=ERROR_FIND_HUGAO;
			return NULL;
		}
		pHeightGroup->GetName(m_sHuGao);
		IModTowerInstance* pInstance=NULL;
		pInstance=pHeightGroup->GetTowerInstance(m_uiLegSerialArr[0],m_uiLegSerialArr[1],m_uiLegSerialArr[2],m_uiLegSerialArr[3]);
		if(pInstance==NULL)
		{
			m_ciErrorType=ERROR_FIND_INSTANCE;
			return NULL;
		}
		return pInstance;
	}
	return NULL;
}
void CTMDHModel::CreateXmlFile(ITidTowerInstance* pInstance,const char* sFileName)
{
	ITidHeightGroup* pHeightGroup=pInstance->BelongHeightGroup();
	if(pHeightGroup==NULL)
		return;
	CXhChar50 sName;
	pHeightGroup->GetName(sName,50);
	GECS rot_cs=BuildRotateCS();
	GECS trans_cs=BuildTransCS(pInstance);
	//初始化XML模型数据
	CXmlModel xXmlModel(Manager.m_bUseUnitM);
	xXmlModel.m_sCategory.Copy("铁塔");
	xXmlModel.m_sTowerName=GetFileName(pInstance);
	if(Manager.m_ciInstanceLevel==0)
		xXmlModel.m_sTowerFile.Printf("%s.dxf",(char*)xXmlModel.m_sTowerName);
	else
		xXmlModel.m_sTowerFile.Printf("%s.3ds",(char*)xXmlModel.m_sTowerName);
	xXmlModel.m_ciModel=0;
	xXmlModel.m_sPropName=sName;
	GEPOINT xLocation;
	for(int i=0;i<4;i++)
	{
		int iHeightGroupSeial=pHeightGroup->GetSerialId();
		BYTE ciLegSerial=(BYTE)pInstance->GetLegSerialIdByQuad(i+1);
		Manager.m_pTidModel->GetSubLegBaseLocation(iHeightGroupSeial,ciLegSerial,xLocation);
		if(i==1||i==3)	 //相对1象限Y轴对称
			xLocation.x*=-1.0;
		if(i==2||i==3)	//相对1象限X轴对称
			xLocation.y*=-1.0;
		if(m_bTurnLeft)
			xLocation=rot_cs.TransPToCS(xLocation);
		xLocation=trans_cs.TransPToCS(xLocation);
		xXmlModel.m_xFundation[i].m_xFeetPos=xLocation;
	}
	//初始化挂线信息
	int nHangPt=Manager.m_pTidModel->HangPointCount();
	for(int i=0;i<nHangPt;i++)
	{	//导线
		ITidHangPoint* pTidHangPt=Manager.m_pTidModel->GetHangPointAt(i);
		if(pTidHangPt->GetWireType()=='E')
			continue;
		CXhChar50 sDes;
		pTidHangPt->GetWireDescription(sDes);
		TID_COORD3D tid_pos=pTidHangPt->GetPos();
		GEPOINT pos(tid_pos);
		if(m_bTurnLeft)
			pos=rot_cs.TransPToCS(pos);
		pos=trans_cs.TransPToCS(pos);
		int iLoop=pTidHangPt->GetCircuitSerial();
		int iPhase=pTidHangPt->GetPhaseSerial();
		BYTE ciTensionType=pTidHangPt->GetTensionType();
		//
		CTowerLoop* pTowerLoop=xXmlModel.m_hashTowerLoop.GetValue(iLoop);
		if(pTowerLoop==NULL)
			pTowerLoop=xXmlModel.m_hashTowerLoop.Add(iLoop);
		CHangPhase* pHangPhase=pTowerLoop->m_hashHangPhase.GetValue(iPhase);
		if(pHangPhase==NULL)
			pHangPhase=pTowerLoop->m_hashHangPhase.Add(iPhase);
		if(pTidHangPt->GetWireType()=='J')
		{
			HANG_POINT* pHangPt=pHangPhase->m_xWireJHangPtArr.append();
			pHangPt->m_ciHangingStyle=pTidHangPt->GetHangingStyle();
			pHangPt->m_ciWireType='J';
			pHangPt->m_xHangPos=pos;
		}
		else if(strstr(sDes,"前"))
		{
			HANG_POINT* pHangPt=pHangPhase->m_xFrontHangPtArr.append();
			pHangPt->m_ciHangingStyle=pTidHangPt->GetHangingStyle();
			pHangPt->m_ciWireType='C';
			pHangPt->m_ciHangDirect='Q';
			pHangPt->m_xHangPos=pos;
			pHangPt->m_ciTensionType=(ciTensionType==1)?0:1;
		}
		else
		{
			HANG_POINT* pHangPt=pHangPhase->m_xBackHangPtArr.append();
			pHangPt->m_ciHangingStyle=pTidHangPt->GetHangingStyle();
			pHangPt->m_ciWireType='C';
			pHangPt->m_ciHangDirect='H';
			pHangPt->m_xHangPos=pos;
			pHangPt->m_ciTensionType=(ciTensionType==1)?0:1;
		}
	}
	for(int i=0;i<nHangPt;i++)
	{	//地线
		ITidHangPoint* pTidHangPt=Manager.m_pTidModel->GetHangPointAt(i);
		if(pTidHangPt->GetWireType()!='E')
			continue;
		CXhChar50 sDes;
		pTidHangPt->GetWireDescription(sDes);
		TID_COORD3D tid_pos=pTidHangPt->GetPos();
		GEPOINT pos(tid_pos);
		if(m_bTurnLeft)
			pos=rot_cs.TransPToCS(pos);
		pos=trans_cs.TransPToCS(pos);
		int iSerial=pTidHangPt->GetPhaseSerial();
		BYTE ciTensionType=pTidHangPt->GetTensionType();
		//
		CEarthLoop* pEarthLoop=xXmlModel.m_hashEarthLoop.GetValue(iSerial);
		if(pEarthLoop==NULL)
			pEarthLoop=xXmlModel.m_hashEarthLoop.Add(iSerial);
		HANG_POINT* pHangPt=pEarthLoop->m_xHangPtList.append();
		pHangPt->m_ciHangingStyle=pTidHangPt->GetHangingStyle();
		pHangPt->m_ciWireType='E';
		pHangPt->m_xHangPos=pos;
		pHangPt->m_ciTensionType=(ciTensionType==1)?0:1;
		if(strstr(sDes,"前"))
			pHangPt->m_ciHangDirect='Q';
		else if(strstr(sDes,"后"))
			pHangPt->m_ciHangDirect='H';
	}
	//生成XML文件
	xXmlModel.AmendHangInfo();
	xXmlModel.CreateOldXmlFile(sFileName);
}
void CTMDHModel::CreateXmlFile(IModTowerInstance* pInstance,const char* sFileName)
{
	IModHeightGroup* pHeightGroup=pInstance->BelongHuGao();
	if(pHeightGroup==NULL)
		return;
	GECS rot_cs=BuildRotateCS();
	CXmlModel xXmlModel(Manager.m_bUseUnitM);
	xXmlModel.m_sCategory.Copy("铁塔");
	xXmlModel.m_sTowerName=GetFileName(pInstance);
	if(Manager.m_ciInstanceLevel==0)
		xXmlModel.m_sTowerFile.Printf("%s.dxf",(char*)xXmlModel.m_sTowerName);
	else
		xXmlModel.m_sTowerFile.Printf("%s.3ds",(char*)xXmlModel.m_sTowerName);
	xXmlModel.m_ciModel=0;
	pHeightGroup->GetName(xXmlModel.m_sPropName);
	GEPOINT xLocation;
	for(int i=0;i<4;i++)
	{
		MOD_POINT base_pt=pInstance->GetBaseLocation(i+1);
		GEPOINT xLocation(base_pt.x,base_pt.y,base_pt.z);
		if(m_bTurnLeft)
			xLocation=rot_cs.TransPToCS(xLocation);
		xXmlModel.m_xFundation[i].m_xFeetPos=xLocation;
	}
	//初始化挂点信息
	BYTE ciHangType=0;
	int nHangPt=Manager.m_pModModel->GetHangNodeNum();
	for(int i=0;i<nHangPt;i++)
	{	
		MOD_HANG_NODE* pModHangPt=Manager.m_pModModel->GetHangNodeById(i);
		if(pModHangPt->m_ciWireType=='E')
			continue;
		if(strstr(pModHangPt->m_sHangName,"前") && ciHangType==0)
			ciHangType=1;
	}
	for(int i=0;i<nHangPt;i++)
	{	//导线
		MOD_HANG_NODE* pModHangPt=Manager.m_pModModel->GetHangNodeById(i);
		if(pModHangPt->m_ciWireType=='E')
			continue;
		GEPOINT pos=pModHangPt->m_xHangPos;
		if(m_bTurnLeft)
			pos=rot_cs.TransPToCS(pos);
		int iLoop=pModHangPt->m_ciLoopSerial;
		int iPhase=pModHangPt->m_ciPhaseSerial;
		//
		CTowerLoop* pTowerLoop=xXmlModel.m_hashTowerLoop.GetValue(iLoop);
		if(pTowerLoop==NULL)
			pTowerLoop=xXmlModel.m_hashTowerLoop.Add(iLoop);
		CHangPhase* pHangPhase=pTowerLoop->m_hashHangPhase.GetValue(iPhase);
		if(pHangPhase==NULL)
			pHangPhase=pTowerLoop->m_hashHangPhase.Add(iPhase);
		if(pModHangPt->m_ciWireType=='J')
		{
			HANG_POINT* pHangPt=pHangPhase->m_xWireJHangPtArr.append();
			pHangPt->m_ciHangingStyle=pModHangPt->m_ciHangingStyle;
			pHangPt->m_ciWireType='J';
			pHangPt->m_xHangPos=pos;
		}
		else if(strstr(pModHangPt->m_sHangName,"前"))
		{
			HANG_POINT* pHangPt=pHangPhase->m_xFrontHangPtArr.append();
			pHangPt->m_ciHangingStyle=pModHangPt->m_ciHangingStyle;
			pHangPt->m_ciWireType='C';
			pHangPt->m_ciHangDirect='Q';
			pHangPt->m_xHangPos=pos;
			pHangPt->m_ciTensionType=ciHangType;
		}
		else
		{
			HANG_POINT* pHangPt=pHangPhase->m_xBackHangPtArr.append();
			pHangPt->m_ciHangingStyle=pModHangPt->m_ciHangingStyle;
			pHangPt->m_ciWireType='C';
			pHangPt->m_ciHangDirect='H';
			pHangPt->m_xHangPos=pos;
			pHangPt->m_ciTensionType=ciHangType;
		}
	}
	for(int i=0;i<nHangPt;i++)
	{	//地线
		MOD_HANG_NODE* pModHangPt=Manager.m_pModModel->GetHangNodeById(i);
		if(pModHangPt->m_ciWireType!='E')
			continue;
		GEPOINT pos=pModHangPt->m_xHangPos;
		if(m_bTurnLeft)
			pos=rot_cs.TransPToCS(pos);
		int iSerial=pModHangPt->m_ciPhaseSerial;
		//
		CEarthLoop* pEarthLoop=xXmlModel.m_hashEarthLoop.GetValue(iSerial);
		if(pEarthLoop==NULL)
			pEarthLoop=xXmlModel.m_hashEarthLoop.Add(iSerial);
		HANG_POINT* pHangPt=pEarthLoop->m_xHangPtList.append();
		pHangPt->m_ciHangingStyle=pModHangPt->m_ciHangingStyle;
		pHangPt->m_ciWireType='E';
		pHangPt->m_xHangPos=pos;
		pHangPt->m_ciTensionType=ciHangType;
		if(strstr(pModHangPt->m_sHangName,"前"))
			pHangPt->m_ciHangDirect='Q';
		else if(strstr(pModHangPt->m_sHangName,"后"))
			pHangPt->m_ciHangDirect='H';
	}
	//生成XML文件
	xXmlModel.AmendHangInfo();
	xXmlModel.CreateOldXmlFile(sFileName);
}
void CTMDHModel::CreateModFile(ITidTowerInstance* pInstance,const char* sFileName)
{
	pInstance->ExportModFile(sFileName);
}
void CTMDHModel::CreateModFile(IModTowerInstance* pInstance,const char* sFileName)
{
	pInstance->ExportModFile(sFileName);
}
void CTMDHModel::Create3dsFile(ITidTowerInstance* pInstance,const char* sFileName)
{
	BYTE ciLevel=Manager.m_ciInstanceLevel;
	UCS_STRU rot_cs=BuildRotateCS();
	UCS_STRU trans_cs=BuildTransCS(pInstance);
	//解析三维实体的基本三角面信息
	int nPart=pInstance->GetAssemblePartNum();
	int nBolt=pInstance->GetAssembleBoltNum();
	int nAnchorBolt=pInstance->GetAssembleAnchorBoltNum();
	int nSum=(ciLevel==3)?nPart+nBolt+nAnchorBolt:nPart;
	int serial=1;
	I3DSData* p3dsFile=C3DSFactory::Create3DSInstance();
	Manager.DisplayProcess(0,"生成3DS模型....");
	for(ITidAssemblePart* pAssmPart=pInstance->EnumAssemblePartFirst();pAssmPart;pAssmPart=pInstance->EnumAssemblePartNext(),serial++)
	{
		Manager.DisplayProcess(ftoi(100*serial/nSum),"生成构件3DS模型....");
		ITidPart* pTidPart=pAssmPart->GetPart();
		ITidSolidBody* pSolidBody=pAssmPart->GetSolidPart();
		if(pSolidBody==NULL||pTidPart==NULL)
			continue;
		CSolidBody solidBody;
		solidBody.CopyBuffer(pSolidBody->SolidBufferPtr(),pSolidBody->SolidBufferLength());
		if(m_bTurnLeft)
			solidBody.TransToACS(rot_cs);
		solidBody.TransToACS(trans_cs);
		if(pAssmPart->IsHasBriefRodLine())
			p3dsFile->AddSolidPart(&solidBody,serial,CXhChar16("Part%d",serial),Manager.m_bUseUnitM);
		else if(pTidPart->GetPartType()==ITidPart::TYPE_PLATE&&ciLevel>1)
			p3dsFile->AddSolidPart(&solidBody,serial,CXhChar16("Part%d",serial),Manager.m_bUseUnitM);
	}
	if(ciLevel==3)
	{	//放样级别，绘制螺栓
		int iBolt=0;
		for(ITidAssembleBolt* pAssmBolt=pInstance->EnumAssembleBoltFirst();pAssmBolt;pAssmBolt=pInstance->EnumAssembleBoltNext(),serial++,iBolt++)
		{
			Manager.DisplayProcess(ftoi(100*serial/nSum),"生成螺栓3DS模型....");
			ITidSolidBody* pBoltSolid=pAssmBolt->GetBoltSolid();
			ITidSolidBody* pNutSolid=pAssmBolt->GetNutSolid();
			if(pBoltSolid==NULL||pNutSolid==NULL)
				continue;
			CSolidBody solidBody;
			solidBody.CopyBuffer(pBoltSolid->SolidBufferPtr(),pBoltSolid->SolidBufferLength());
			solidBody.MergeBodyBuffer(pNutSolid->SolidBufferPtr(),pNutSolid->SolidBufferLength());
			if(m_bTurnLeft)
				solidBody.TransToACS(rot_cs);
			solidBody.TransToACS(trans_cs);
			p3dsFile->AddSolidPart(&solidBody,serial,CXhChar16("Bolt%d",iBolt),Manager.m_bUseUnitM);
		}
		/*iBolt=0;
		for(ITidAssembleAnchorBolt* pAssmAnchorBolt=pInstance->EnumFirstAnchorBolt();pAssmAnchorBolt;pAssmAnchorBolt=pInstance->EnumNextAnchorBolt(),serial++,iBolt++)
		{
			DisplayProcess(ftoi(100*serial/nSum),"生成脚钉3DS模型....");
			ITidSolidBody* pBoltSolid=pAssmAnchorBolt->GetBoltSolid();
			ITidSolidBody* pNutSolid=pAssmAnchorBolt->GetNutSolid();
			if(pBoltSolid==NULL||pNutSolid==NULL)
				continue;
			CSolidBody solidBody;
			solidBody.CopyBuffer(pBoltSolid->SolidBufferPtr(),pBoltSolid->SolidBufferLength());
			solidBody.MergeBodyBuffer(pNutSolid->SolidBufferPtr(),pNutSolid->SolidBufferLength());
			if(m_bLeftRot)
			solidBody.TransToACS(rot_cs);
			solidBody.TransToACS(trans_cs);
			p3dsFile->Init3dsData(&solidBody,serial,CXhChar16("AnchorBolt%d",iBolt),BelongManager()->m_bUseUnitM);
		}*/
	}
	Manager.DisplayProcess(100,"生成3DS模型....");
	p3dsFile->Creat3DSFile(sFileName);
	//
	C3DSFactory::Destroy(p3dsFile->GetSerial());
	p3dsFile=NULL;
}
void CTMDHModel::Create3dsFile(IModTowerInstance* pInstance,const char* sFileName)
{
	//解析三维实体的基本三角面信息
	UCS_STRU rot_cs=BuildRotateCS();
	I3DSData* p3dsFile=C3DSFactory::Create3DSInstance();
	int nIndex=1,nSum=pInstance->GetModRodNum();
	Manager.DisplayProcess(0,"生成3DS文件....");
	for(IModRod* pRod=pInstance->EnumModRodFir();pRod;pRod=pInstance->EnumModRodNext(),nIndex++)
	{
		Manager.DisplayProcess(ftoi(100*nIndex/nSum),"生成3DS文件....");
		CSolidBody solidBody;
		pRod->Create3dSolidModel(&solidBody,FALSE);
		if(m_bTurnLeft)
			solidBody.TransToACS(rot_cs);
		p3dsFile->AddSolidPart(&solidBody,nIndex,CXhChar16("Rod%d",nIndex),Manager.m_bUseUnitM);
	}
	Manager.DisplayProcess(100,"生成3DS文件....");
	p3dsFile->Creat3DSFile(sFileName);
	//
	C3DSFactory::Destroy(p3dsFile->GetSerial());
	p3dsFile=NULL;
}
void CTMDHModel::CreateDxfFile(ITidTowerInstance* pInstance,const char* sFileName)
{
	GECS rot_cs=BuildRotateCS();
	GECS trans_cs=BuildTransCS(pInstance);
	//提取直线集合，并初始化区域
	SCOPE_STRU scope;
	ARRAY_LIST<f3dLine> line_arr;
	for(ITidAssemblePart* pAssmPart=pInstance->EnumAssemblePartFirst();pAssmPart;pAssmPart=pInstance->EnumAssemblePartNext())
	{
		if(!pAssmPart->IsHasBriefRodLine())
			continue;
		TID_COORD3D line_S=pAssmPart->BriefLineStart();
		TID_COORD3D line_E=pAssmPart->BriefLineEnd();
		f3dPoint ptS(line_S.x,line_S.y,line_S.z);
		if(m_bTurnLeft)
			ptS=rot_cs.TransPToCS(ptS);
		ptS=trans_cs.TransPToCS(ptS);
		if(Manager.m_bUseUnitM)
			ptS*=0.001;
		scope.VerifyVertex(ptS);
		f3dPoint ptE(line_E.x,line_E.y,line_E.z);
		if(m_bTurnLeft)
			ptE=rot_cs.TransPToCS(ptE);
		ptE=trans_cs.TransPToCS(ptE);
		if(Manager.m_bUseUnitM)
			ptE*=0.001;
		scope.VerifyVertex(ptE);
		//
		line_arr.append(f3dLine(ptS,ptE));
	}
	//生成DXF文件
	CDxfFile dxf;
	dxf.extmin.Set(scope.fMinX,scope.fMinY,scope.fMinZ);
	dxf.extmax.Set(scope.fMaxX,scope.fMaxY,scope.fMaxZ);
	dxf.OpenFile(sFileName);
	for(int i=0;i<line_arr.GetSize();i++)
		dxf.New3dLine(line_arr[i].startPt,line_arr[i].endPt);
	dxf.CloseFile();
}
void CTMDHModel::CreateDxfFile(IModTowerInstance* pInstance,const char* sFileName)
{
	GECS rot_cs=BuildRotateCS();
	//提取直线集合，并初始化区域
	SCOPE_STRU scope;
	ARRAY_LIST<f3dLine> line_arr;
	for(IModRod* pRod=pInstance->EnumModRodFir();pRod;pRod=pInstance->EnumModRodNext())
	{
		f3dPoint ptS=pRod->LinePtS();
		if(m_bTurnLeft)
			ptS=rot_cs.TransPToCS(ptS);
		if(Manager.m_bUseUnitM)
			ptS*=0.001;
		scope.VerifyVertex(ptS);
		f3dPoint ptE=pRod->LinePtE();
		if(m_bTurnLeft)
			ptE=rot_cs.TransPToCS(ptE);
		if(Manager.m_bUseUnitM)
			ptE*=0.001;
		scope.VerifyVertex(ptE);
		//
		line_arr.append(f3dLine(ptS,ptE));
	}
	//生成DXF文件
	CDxfFile dxf;
	dxf.extmin.Set(scope.fMinX,scope.fMinY,scope.fMinZ);
	dxf.extmax.Set(scope.fMaxX,scope.fMaxY,scope.fMaxZ);
	dxf.OpenFile(sFileName);
	for(int i=0;i<line_arr.GetSize();i++)
		dxf.New3dLine(line_arr[i].startPt,line_arr[i].endPt);
	dxf.CloseFile();
}
void CTMDHModel::CreateStlFile(ITidTowerInstance* pInstance,const char* sFileName)
{
	BYTE ciLevel=Manager.m_ciInstanceLevel;
	//初始化GIM坐标系
	GECS rot_cs=BuildRotateCS();
	GECS trans_cs=BuildTransCS(pInstance);
	//解析三维实体的基本三角面信息
	int nPart=pInstance->GetAssemblePartNum();
	int nBolt=pInstance->GetAssembleBoltNum();
	int nAnchorBolt=pInstance->GetAssembleAnchorBoltNum();
	int nSum=(ciLevel==3)?nPart+nBolt+nAnchorBolt:nPart;
	int serial=1;
	IStlData* pStlFile=CStlFactory::CreateStl();
	Manager.DisplayProcess(0,"生成STL模型....");
	for(ITidAssemblePart* pAssmPart=pInstance->EnumAssemblePartFirst();pAssmPart;pAssmPart=pInstance->EnumAssemblePartNext(),serial++)
	{
		Manager.DisplayProcess(ftoi(100*serial/nSum),"生成构件STL模型....");
		ITidPart* pTidPart=pAssmPart->GetPart();
		ITidSolidBody* pSolidBody=pAssmPart->GetSolidPart();
		if(pSolidBody==NULL||pTidPart==NULL)
			continue;
		CSolidBody solidBody;
		solidBody.CopyBuffer(pSolidBody->SolidBufferPtr(),pSolidBody->SolidBufferLength());
		if(m_bTurnLeft)
			solidBody.TransToACS(rot_cs);
		solidBody.TransToACS(trans_cs);
		if(pAssmPart->IsHasBriefRodLine())
			pStlFile->AddSolidBody(solidBody.BufferPtr(),solidBody.BufferLength());
		else if(pTidPart->GetPartType()==ITidPart::TYPE_PLATE&&ciLevel>1)
			pStlFile->AddSolidBody(solidBody.BufferPtr(),solidBody.BufferLength());
	}
	if(ciLevel==3)
	{	//放样级别，绘制螺栓
		int iBolt=0;
		for(ITidAssembleBolt* pAssmBolt=pInstance->EnumAssembleBoltFirst();pAssmBolt;pAssmBolt=pInstance->EnumAssembleBoltNext(),serial++,iBolt++)
		{
			Manager.DisplayProcess(ftoi(100*serial/nSum),"生成螺栓STL模型....");
			ITidSolidBody* pBoltSolid=pAssmBolt->GetBoltSolid();
			ITidSolidBody* pNutSolid=pAssmBolt->GetNutSolid();
			if(pBoltSolid==NULL||pNutSolid==NULL)
				continue;
			CSolidBody solidBody;
			solidBody.CopyBuffer(pBoltSolid->SolidBufferPtr(),pBoltSolid->SolidBufferLength());
			solidBody.MergeBodyBuffer(pNutSolid->SolidBufferPtr(),pNutSolid->SolidBufferLength());
			if(m_bTurnLeft)
				solidBody.TransToACS(rot_cs);
			solidBody.TransToACS(trans_cs);
			pStlFile->AddSolidBody(solidBody.BufferPtr(),solidBody.BufferLength());
		}
	}
	Manager.DisplayProcess(100,"生成STL模型....");
	pStlFile->SaveFile((char*)sFileName,1);
	//
	CStlFactory::Destroy(pStlFile->GetSerial());
	pStlFile=NULL;
}
void CTMDHModel::CreateStlFile(IModTowerInstance* pInstance,const char* sFileName)
{
	//解析三维实体的基本三角面信息
	UCS_STRU rot_cs=BuildRotateCS();
	IStlData* pStlFile=CStlFactory::CreateStl();
	int nIndex=1,nSum=pInstance->GetModRodNum();
	Manager.DisplayProcess(0,"生成STL文件....");
	for(IModRod* pRod=pInstance->EnumModRodFir();pRod;pRod=pInstance->EnumModRodNext(),nIndex++)
	{
		Manager.DisplayProcess(ftoi(100*nIndex/nSum),"生成STL文件....");
		CSolidBody solidBody;
		pRod->Create3dSolidModel(&solidBody,FALSE);
		if(m_bTurnLeft)
			solidBody.TransToACS(rot_cs);
		pStlFile->AddSolidBody(solidBody.BufferPtr(),solidBody.BufferLength());
	}
	Manager.DisplayProcess(100,"生成STL文件....");
	pStlFile->SaveFile(sFileName,1);
	//
	CStlFactory::Destroy(pStlFile->GetSerial());
	pStlFile=NULL;
}
//////////////////////////////////////////////////////////////////////////
//CTMDHTower
CTMDHTower::CTMDHTower()
{

}
CTMDHTower::~CTMDHTower()
{

}
bool CTMDHTower::ReadLdsFile(const char* sFileName)
{
	m_xTower.Empty();
	m_xTower.InitTower();
	//
	FILE* fp=fopen(sFileName,"rb");
	if(fp==NULL)
	{
		logerr.Log(CXhChar100("%s文件打开失败!",sFileName));
		return false;
	}
	fseek(fp,0,SEEK_END);
	long buf_size=ftell(fp);
	fseek(fp,0,SEEK_SET);
	CBuffer buffer;
	buffer.Write(NULL,buf_size);
	fread(buffer.GetBufferPtr(),buffer.GetLength(),1,fp);
	fclose(fp);
	//
	DWORD buffer_len=0,cursor_pipeline_no=0;
	CXhChar100 sDocTypeName,sFileVersion;
	buffer.SeekToBegin();
	buffer.ReadString(sDocTypeName);
	buffer.ReadString(sFileVersion);
	buffer.ReadDword(&m_xTower.user_pipeline_no);
	buffer.ReadDword(&cursor_pipeline_no);
	buffer.ReadDword(&buffer_len);
	CBuffer ta_buf;
	ta_buf.Write(NULL,buffer_len);
	buffer.Read(ta_buf.GetBufferPtr(),buffer_len);
	_snprintf(m_xTower.version,19,"%s",(char*)sFileVersion);
	char bEncryptByAES=false;
	if(compareVersion(m_xTower.version,"1.3.8.0")>=0)
		bEncryptByAES=2;
	else if(compareVersion(m_xTower.version,"1.3.0.0")>=0)
		bEncryptByAES=true;
	DecryptBuffer(ta_buf,bEncryptByAES,cursor_pipeline_no);
	m_xTower.FromBuffer(ta_buf,PRODUCT_LDS);
	//读取附加信息
	CBuffer* pUniWireModel=m_xTower.AttachBuffer.GetValue(CTower::UNI_WIREPOINT_MODEL);
	if(pUniWireModel)
		gxWirePointModel.FromBuffer(*pUniWireModel);
	else
		gxWirePointModel.m_bGimWireModelInherited=false;
	if(!gxWirePointModel.m_bGimWireModelInherited)
		m_xTower.RetrieveWireModelFromNodes(&gxWirePointModel);
	return true;
}
void CTMDHTower::CreateTidFile(const char* sFileName)
{
	m_xTower.ExportTowerSolidDataExchangeFile((char*)sFileName);
}
void CTMDHTower::CreateModFile(const char* sFileName)
{
	IModModel* pModModel=CModModelFactory::CreateModModel();
	if(m_xTower.Parts.GetNodeNum()<=0 || pModModel==NULL )
		return;
	SetModCfgwordSchema(CFGLEG::Schema());
	//提取MOD呼高信息，建立MOD坐标系
	double fTowerHeight=0;
	for(CLDSModule *pModule=m_xTower.Module.GetFirst();pModule;pModule=m_xTower.Module.GetNext())
	{
		double lowest_module_z=0;
		pModule->GetModuleScopeZ(NULL,&lowest_module_z);
		if(lowest_module_z>fTowerHeight)
			fTowerHeight=lowest_module_z;
		//
		IModHeightGroup* pHeightGroup=pModModel->AppendHeightGroup(pModule->GetBodyNo());
		pHeightGroup->SetBelongModel(pModModel);
		pHeightGroup->SetLowestZ(pModule->LowestZ());
		pHeightGroup->SetLegCfg(pModule->m_dwLegCfgWord.flag.bytes);
		pHeightGroup->SetNameHeight(pModule->NamedHeight);
	}
	pModModel->SetTowerHeight(fTowerHeight);
	GECS ucs=TransToUcs(pModModel->BuildUcsByModCS());
	//提取挂点信息
	for(CLDSNode* pNode=m_xTower.EnumNodeFirst();pNode;pNode=m_xTower.EnumNodeNext())
	{
		if(pNode->m_cHangWireType!='C'&&pNode->m_cHangWireType!='E'&&pNode->m_cHangWireType!='J')
			continue;
		MOD_HANG_NODE* pGuaInfo=pModModel->AppendHangNode();
		pGuaInfo->m_xHangPos=ucs.TransPFromCS(pNode->xOriginalPos);
		if(pNode->m_cHangWireType=='C')			//导线
			pGuaInfo->m_ciWireType='C';
		else if(pNode->m_cHangWireType=='E')	//地线
			pGuaInfo->m_ciWireType='G';
		else //if(pNode->m_cHangWireType=='J')	//跳线
			pGuaInfo->m_ciWireType='T';
		if(strlen(pNode->m_sHangWireDesc)>0)
			strcpy(pGuaInfo->m_sHangName,pNode->m_sHangWireDesc);
	}
	//提取有效节点，进行编号处理
	CHashSet<NODE_PTR> hashLinkNode;
	ARRAY_LIST<NODE_PTR> nodeArr;
	for(CLDSLinePart* pRod=m_xTower.EnumRodFirst();pRod;pRod=m_xTower.EnumRodNext())
	{
		CLDSNode* pNodeS=pRod->pStart;
		if(pNodeS && hashLinkNode.GetValue(pNodeS->handle)==NULL)
		{
			hashLinkNode.SetValue(pNodeS->handle,pNodeS);
			nodeArr.append(pNodeS);
		}
		//
		CLDSNode* pNodeE=pRod->pEnd;
		if(pNodeE && hashLinkNode.GetValue(pNodeE->handle)==NULL)
		{
			hashLinkNode.SetValue(pNodeE->handle,pNodeE);
			nodeArr.append(pNodeE);
		}
	}
	BOOL bHasPtI=IsHasValidPointI(hashLinkNode);
	if(bHasPtI)
	{
		CQuickSort<NODE_PTR>::QuickSort(nodeArr.m_pData,nodeArr.GetSize(),comparefun);
		hashLinkNode.Empty();
		for(int i=0;i<nodeArr.GetSize();i++)
			hashLinkNode.SetValue(nodeArr[i]->handle,nodeArr[i]);
	}
	int index=1;
	CHashList<int> hashNodePointI;
	for(CLDSNode* pNode=hashLinkNode.GetFirst();pNode;pNode=hashLinkNode.GetNext())
	{
		int iNode=bHasPtI?pNode->point_i:index++;
		hashNodePointI.SetValue(pNode->handle,iNode);
	}
	//初始化MOD模型的节点与杆件
	index=0;
	for(CLDSNode* pNode=hashLinkNode.GetFirst();pNode;pNode=hashLinkNode.GetNext())
	{
		int *pIndex=hashNodePointI.GetValue(pNode->handle);
		if(pIndex==NULL)
			continue;
		GEPOINT org_pt=ucs.TransPFromCS(pNode->xOriginalPos);
		IModNode* pModNode=pModModel->AppendNode(*pIndex);
		pModNode->SetBelongModel(pModModel);
		pModNode->SetCfgword(pNode->cfgword.flag.bytes);
		pModNode->SetLdsOrg(MOD_POINT(pNode->xOriginalPos));
		pModNode->SetOrg(MOD_POINT(org_pt));
		if(pNode->IsLegObj())
			pModNode->SetLayer('L');	//腿部Leg
		else
			pModNode->SetLayer('B');	//塔身Body
	}
	index=0;
	for(CLDSLinePart* pRod=m_xTower.EnumRodFirst();pRod;pRod=m_xTower.EnumRodNext())
	{
		if(pRod==NULL||pRod->pStart==NULL||pRod->pEnd==NULL)
			continue;
		if(pRod->GetClassTypeId()==CLS_GROUPLINEANGLE)
			continue;
		int *iNodeIdS=hashNodePointI.GetValue(pRod->pStart->handle);
		int *iNodeIdE=hashNodePointI.GetValue(pRod->pEnd->handle);
		IModNode* pModNodeS=pModModel->FindNode(*iNodeIdS);
		IModNode* pModNodeE=pModModel->FindNode(*iNodeIdE);
		if(pModNodeS==NULL || pModNodeE==NULL)
			continue;
		IModRod* pModRod=pModModel->AppendRod(index++);
		pModRod->SetBelongModel(pModModel);
		pModRod->SetCfgword(pRod->cfgword.flag.bytes);
		pModRod->SetNodeS(pModNodeS);
		pModRod->SetNodeE(pModNodeE);
		pModRod->SetMaterial(pRod->cMaterial);
		pModRod->SetWidth(pRod->GetWidth());
		pModRod->SetThick(pRod->GetThick());
		if(pRod->IsLegObj())
			pModRod->SetLayer('L');	//腿部Leg
		else
			pModRod->SetLayer('B');	//塔身Body
		if(pRod->GetClassTypeId()==CLS_LINEANGLE)
		{
			CLDSLineAngle* pJg=(CLDSLineAngle*)pRod;
			GEPOINT wing_vec_x=ucs.TransVToCS(pJg->GetWingVecX());
			GEPOINT wing_vec_y=ucs.TransVToCS(pJg->GetWingVecY());
			pModRod->SetWingXVec(MOD_POINT(wing_vec_x));
			pModRod->SetWingYVec(MOD_POINT(wing_vec_y));
			pModRod->SetRodType(1);
		}
		else
			pModRod->SetRodType(2);
	}
	//初始化多胡高塔型的MOD结构
	pModModel->InitMultiModData();
	//生成Mod文件
	FILE *fp=fopen(sFileName,"wt,ccs=UTF-8");
	if(fp==NULL)
	{
		logerr.Log("%s文件打开失败!",sFileName);
		return;
	}
	pModModel->WriteModFileByUtf8(fp);
}
void CTMDHTower::Create3dsFile(const char* sFileName)
{
	CLDSModule *pModule=m_xTower.GetActiveModule();
	if(pModule==NULL)
		return;
	pModule->GetModuleScopeZ();
	//初始化GIM坐标系
	UCS_STRU stdcs;
	LoadDefaultUCS(&stdcs);
	UCS_STRU draw_ucs;
	draw_ucs.origin.Set(0,0,pModule->LowestZ());
	draw_ucs.axis_x.Set(1,0,0);
	draw_ucs.axis_y.Set(0,-1,0);
	draw_ucs.axis_z.Set(0,0,-1);
	//解析三维实体的基本三角面信息
	int nPart=m_xTower.Parts.GetNodeNum();
	int serial=1;
	I3DSData* p3dsFile=C3DSFactory::Create3DSInstance();
	Manager.DisplayProcess(0,"生成3DS模型....");
	for(CLDSPart* pPart=m_xTower.EnumPartFirst();pPart;pPart=m_xTower.EnumPartNext(),serial++)
	{
		Manager.DisplayProcess(ftoi(100*serial/nPart),"生成构件3DS模型....");
		if(!pModule->IsSonPart(pPart))
			continue;
		if(pPart->pSolidBody==NULL)
			pPart->Create3dSolidModel();
		if(pPart->pSolidBody)
		{
			CSolidBody solidBody;
			solidBody.CopyBuffer(pPart->pSolidBody->BufferPtr(),pPart->pSolidBody->BufferLength());
			solidBody.TransToACS(draw_ucs);
			p3dsFile->AddSolidPart(&solidBody,serial,CXhChar16("Part%d",serial));
		}
	}
	Manager.DisplayProcess(0,"生成3DS模型....");
	for(CBlockReference *pBlockRef=m_xTower.BlockRef.GetFirst();pBlockRef;pBlockRef=m_xTower.BlockRef.GetNext())
	{
		CBlockModel *pBlock=m_xTower.Block.FromHandle(pBlockRef->m_hBlock);
		if(pBlock==NULL)
			continue;
		UCS_STRU acs=pBlockRef->GetACS();
		UCS_STRU cs=pBlock->TransToACS(acs);
		PARTSET partSet;
		if(pBlock->IsEmbeded())
		{	//嵌入式部件
			for(CLDSPart *pPart=m_xTower.EnumPartFirst();pPart;pPart=m_xTower.EnumPartNext())
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
		//
		for(pPart=partSet.GetFirst();pPart;pPart=partSet.GetNext(),serial++)
		{
			if(pPart->pSolidBody==NULL)
				pPart->Create3dSolidModel(FALSE);
			if(pPart->pSolidBody)
			{
				CSolidBody solidBody;
				solidBody.CopyBuffer(pPart->pSolidBody->BufferPtr(),pPart->pSolidBody->BufferLength());
				solidBody.TransACS(stdcs,cs);	//
				solidBody.TransToACS(draw_ucs);	//
				p3dsFile->AddSolidPart(&solidBody,serial,CXhChar16("Part%d",serial));
			}
		}
	}
	Manager.DisplayProcess(100,"生成3DS模型....");
	p3dsFile->Creat3DSFile(sFileName);
	//
	C3DSFactory::Destroy(p3dsFile->GetSerial());
	p3dsFile=NULL;
}
void CTMDHTower::CreateStlFile(const char* sFileName)
{
	CLDSModule *pModule=m_xTower.GetActiveModule();
	if(pModule==NULL)
		return;
	pModule->GetModuleScopeZ();
	//初始化GIM坐标系
	UCS_STRU draw_ucs;
	draw_ucs.origin.Set(0,0,pModule->LowestZ());
	draw_ucs.axis_x.Set(1,0,0);
	draw_ucs.axis_y.Set(0,-1,0);
	draw_ucs.axis_z.Set(0,0,-1);
	//解析三维实体的基本三角面信息
	int nPart=m_xTower.Parts.GetNodeNum();
	int serial=1;
	IStlData* pStlFile=CStlFactory::CreateStl();
	Manager.DisplayProcess(0,"生成STL模型....");
	for(CLDSPart* pPart=m_xTower.EnumPartFirst();pPart;pPart=m_xTower.EnumPartNext(),serial++)
	{
		Manager.DisplayProcess(ftoi(100*serial/nPart),"生成构件STL模型....");
		if(!pModule->IsSonPart(pPart))
			continue;
		if(pPart->pSolidBody==NULL)
			pPart->Create3dSolidModel();
		if(pPart->pSolidBody)
		{
			CSolidBody solidBody;
			solidBody.CopyBuffer(pPart->pSolidBody->BufferPtr(),pPart->pSolidBody->BufferLength());
			solidBody.TransToACS(draw_ucs);
			pStlFile->AddSolidBody(solidBody.BufferPtr(),solidBody.BufferLength());
		}
	}
	Manager.DisplayProcess(100,"生成STL模型....");
	pStlFile->SaveFile(sFileName,1);
	//
	CStlFactory::Destroy(pStlFile->GetSerial());
	pStlFile=NULL;
}
//////////////////////////////////////////////////////////////////////////
//CTMDHGim
//CModFileError
void CModFileError::SetError(int nType,int nItemNoOrRow,int nSubLegNo)
{
	switch(nType)
	{
	case ERROR_MOD_BODY_H:
	case ERROR_MOD_HBODY_MISS: 
		logerr.Log("本体%d的高度缺失或错误",nItemNoOrRow);
		break;
	case ERROR_MOD_LEG_H:
	case ERROR_MOD_HLEG_MISS:
		logerr.Log("接腿%d的高度缺失或错误",nItemNoOrRow);
		break;
	case ERROR_MOD_HSUBLEG_MISS:
	case ERROR_MOD_SUBLEG_H:
		logerr.Log("接腿%d子腿%d的高度错误或数据缺失",nItemNoOrRow,nSubLegNo);
		break;
	case ERROR_MOD_H_MISS:
	case ERROR_MOD_LAYOUT_H:
		logerr.Log("第%d行塔例格式或数据错误",nItemNoOrRow);
		break;
	case ERROR_MOD_LAYOUT_R:
		logerr.Log("第%d行未知杆件，格式错误",nItemNoOrRow);
		break;
	case ERROR_MOD_P_MISS:
	case ERROR_MOD_LAYOUT_P:
		logerr.Log("第%d行点格式或数据错误",nItemNoOrRow);
		break;
	case ERROR_MOD_G_MISS:
	case ERROR_MOD_LAYOUT_G:
		logerr.Log("第%d行挂点格式或数据错误",nItemNoOrRow);
		break;
	case ERROR_MOD_LAYOUT_R_L: 
	case ERROR_MOD_R_L_MISS:
		logerr.Log("第%d行角钢格式或数据错误",nItemNoOrRow);
		break;
	case ERROR_MOD_LAYOUT_R_T:
	case ERROR_MOD_R_T_MISS:
		logerr.Log("第%d行钢管格式或数据错误",nItemNoOrRow);
		break;
	case ERROR_MOD_LAYOUT_SUNLEG_H:
		logerr.Log("接腿%d子腿%d的高度数据行缺失",nItemNoOrRow,nSubLegNo);
		break;
	case ERROR_MOD_LAYOUT_LEG_H:
		logerr.Log("接腿%d的高度数据行缺失",nItemNoOrRow);
		break;
	case ERROR_MOD_LAYOUT_BODY_H:
		logerr.Log("本体%d的高度数据行缺失",nItemNoOrRow);
		break;
	case ERROR_MOD_LAYOUT_SUNLEG:
		logerr.Log("第%d行前缺失子腿标记声明行",nItemNoOrRow,nSubLegNo);
		break;
	case ERROR_MOD_LAYOUT_LEG:
		logerr.Log("第%d行前缺失接腿标记声明行",nItemNoOrRow);
		break;
	case ERROR_MOD_LAYOUT_BODY:
		logerr.Log("第%d行前缺失本体标记声明行",nItemNoOrRow);
		break;
	}
}
void CModFileError::SetError(int nType,const char* sError)
{

}
void CModFileError::SetError(int nType)
{

}
//
CTMDHGim::CTMDHGim()
{
	m_ciCodingType=UTF8_CODING;
	m_uiHeightSerial=1;
	for(int i=0;i<4;i++)
		m_uiLegSerialArr[i]=1;
}
CTMDHGim::~CTMDHGim()
{
}
void CTMDHGim::WriteModFile(const char* sModFilePath)
{
	CXhChar200 mod_file("%s\\%s.mod",(char*)m_sModPath,(char*)m_sModName);
	FILE* fp=fopen(mod_file,"rb");
	if(fp)
	{
		fseek(fp,0,SEEK_END);
		long buf_size=ftell(fp);
		fseek(fp,0,SEEK_SET);
		CBuffer buffer;
		buffer.Write(NULL,buf_size);
		fread(buffer.GetBufferPtr(),buffer.GetLength(),1,fp);
		fclose(fp);
		//
		fp=fopen(sModFilePath,"wb");
		if(fp==NULL)
			return;
		fwrite(buffer.GetBufferPtr(),buffer.GetLength(),1,fp);
		fclose(fp);
	}
}
void CTMDHGim::WritePhmFile(const char* sPhmFile,char* sModName)
{
	FILE *fp=NULL;
	if(m_ciCodingType==ANSI_CODING)
		fp=fopen(sPhmFile,"wt");
	else
		fp=fopen(sPhmFile,"wt,ccs=UTF-8");
	if(fp==NULL)
		return;
	if(m_ciCodingType==ANSI_CODING)
	{	//ANSI
		fprintf(fp,"SOLIDMODELS.NUM=1\n");
		fprintf(fp,"SOLIDMODEL0=%s.mod\n",sModName);
		fprintf(fp,"TRANSFORMMATRIX0=1.000000,0.000000,0.000000,0.000000,0.000000,1.000000,0.000000,0.000000,0.000000,0.000000,1.000000,0.000000,0.000000,0.000000,0.000000,1.000000\n");
		fprintf(fp,"COLOR0=255,255,255\n");
	}
	else
	{	//UTF-8
		wchar_t sWValue[MAX_PATH];
		ANSIToUnicode(sModName,sWValue);
		fwprintf(fp,L"SOLIDMODELS.NUM=1\n");
		fwprintf(fp,L"SOLIDMODEL0=%s.mod\n",sWValue);
		fwprintf(fp,L"TRANSFORMMATRIX0=1.000000,0.000000,0.000000,0.000000,0.000000,1.000000,0.000000,0.000000,0.000000,0.000000,1.000000,0.000000,0.000000,0.000000,0.000000,1.000000\n");
		fwprintf(fp,L"COLOR0=255,255,255\n");
	}
	fclose(fp);
}
void CTMDHGim::WriteDevFile(const char* sDevFile)
{
	FILE *fp=NULL;
	if(m_ciCodingType==ANSI_CODING)
		fp=fopen(sDevFile,"wt");
	else
		fp=fopen(sDevFile,"wt,ccs=UTF-8");
	if(fp==NULL)
		return;
	if(m_ciCodingType==ANSI_CODING)
	{	//ANSI
		fprintf(fp,"DEVICETYPE = TOWER\n");
		fprintf(fp,"SYMBOLNAME = TOWER\n");
		fprintf(fp,"BASEFAMILYPOINTER=%s.fam\n",(char*)m_sFamGuid);
		fprintf(fp,"SOLIDMODELS.NUM = 1\n");
		fprintf(fp,"SOLIDMODEL0 = %s.phm\n",(char*)m_sPhmGuid);
		fprintf(fp,"TRANSFORMMATRIX0=1.000000,0.000000,0.000000,0.000000,0.000000,1.000000,0.000000,0.000000,0.000000,0.000000,1.000000,0.000000,0.000000,0.000000,0.000000,1.000000\n");
	}
	else
	{	//UTF-8
		wchar_t sWValue[MAX_PATH];
		fwprintf(fp,L"DEVICETYPE = TOWER\n");
		fwprintf(fp,L"SYMBOLNAME = TOWER\n");
		ANSIToUnicode(m_sFamGuid,sWValue);
		fwprintf(fp,L"BASEFAMILYPOINTER=%s.fam\n",sWValue);
		fwprintf(fp,L"SOLIDMODELS.NUM = 1\n");
		ANSIToUnicode(m_sPhmGuid,sWValue);
		fwprintf(fp,L"SOLIDMODEL0 = %s.phm\n",sWValue);
		fwprintf(fp,L"TRANSFORMMATRIX0=1.000000,0.000000,0.000000,0.000000,0.000000,1.000000,0.000000,0.000000,0.000000,0.000000,1.000000,0.000000,0.000000,0.000000,0.000000,1.000000\n");
	}
	fclose(fp);
}
void CTMDHGim::WriteFamFile(const char* sFamFile)
{
	FILE *fp=NULL;
	if(m_ciCodingType==ANSI_CODING)
		fp=fopen(sFamFile,"wt");
	else
		fp=fopen(sFamFile,"wt,ccs=UTF-8");
	if(fp==NULL)
		return;
	CXhChar50 ss;
	if(m_ciCodingType==ANSI_CODING)
	{
		//电压等级
		fprintf(fp,"VOLTAGE = 电压等级 = %s\n",(char*)m_xTowerProp.m_sVoltGrade);
		//塔型
		fprintf(fp,"TYPE = 塔型 = %s\n",(char*)m_xTowerProp.m_sType);
		//杆塔材质
		fprintf(fp,"TEXTURE = 杆塔材质 = %s\n",(char*)m_xTowerProp.m_sTexture);
		//固定方式
		fprintf(fp,"FIXEDTYPE = 固定方式 = %s\n",(char*)m_xTowerProp.m_sFixedType);
		//杆塔类型
		fprintf(fp,"TOWERTYPE = 杆塔类型 = %s\n",(char*)m_xTowerProp.m_sTaType);
		//回路数
		fprintf(fp,"CIRCUIT = 回路数 = %d\n",m_xTowerProp.m_nCircuit);
		//导线型号
		fprintf(fp,"CONDUCTOR = 导线型号 = %s\n",(char*)m_xTowerProp.m_sCWireSpec);
		//地线型号
		fprintf(fp,"GROUNDWIRE = 地线型号 = %s\n",(char*)m_xTowerProp.m_sEWireSpec);
		//基本风速
		ss.Printf("%f",m_xTowerProp.m_fWindSpeed);
		SimplifiedNumString(ss);
		fprintf(fp,"REFERENCEWINDSPEED = 设计基本风速 = %s\n",(char*)ss);
		//覆冰厚度
		ss.Printf("%f",m_xTowerProp.m_fNiceThick);
		SimplifiedNumString(ss);
		fprintf(fp,"DESIGNICETHICKNESS = 设计覆冰厚度 = %s\n",(char*)ss);
		//水平档距
		fprintf(fp,"DESIGNWINDSPAN = 设计水平档距 = %s\n",(char*)m_xTowerProp.m_sWindSpan);
		//垂直档距
		fprintf(fp,"DESIGNWEIGHTSPAN = 设计垂直档距 = %s\n",(char*)m_xTowerProp.m_sWeightSpan);
		//前侧代表档距
		ss.Printf("%f",m_xTowerProp.m_fFrontRulingSpan);
		SimplifiedNumString(ss);
		fprintf(fp,"DESIGNFRONTRULINGSPAN = 设计前侧代表档距 = %s\n",(char*)ss);
		//后侧代表档距
		ss.Printf("%f",m_xTowerProp.m_fBackRulingSpan);
		SimplifiedNumString(ss);
		fprintf(fp,"DESIGNBACKRULINGSPAN=设计后侧代表档距=%s\n",(char*)ss);
		//最大档距
		ss.Printf("%f",m_xTowerProp.m_fMaxSpan);
		SimplifiedNumString(ss);
		fprintf(fp,"DESIGNMAXSPAN = 设计最大档距 =%s\n",(char*)ss);
		//转角范围
		fprintf(fp,"ANGLERANGE = 设计转角范围 =%s\n",(char*)m_xTowerProp.m_sAngleRange);
		//Kv
		ss.Printf("%f",m_xTowerProp.m_fDesignKV);
		SimplifiedNumString(ss);
		fprintf(fp,"DESIGNKV = 设计Kv值 = %s\n",(char*)ss);
		//计算呼高
		fprintf(fp,"RATEDNOMINALHEIGHT = 计算呼高 = %s\n",(char*)m_xTowerProp.m_sRatedHeight);
		//呼高范围
		fprintf(fp,"NOMINALHEIGHTRANGE = 呼高范围 = %s\n",(char*)m_xTowerProp.m_sHeightRange);
		//塔重
		fprintf(fp,"TOWERWEIGHT = 塔重 = %s\n",(char*)m_xTowerProp.m_sTowerWeight);
		//设计工频摇摆角度
		ss.Printf("%f",m_xTowerProp.m_fFrequencyRockAngle);
		SimplifiedNumString(ss);
		fprintf(fp,"ROCKANGLEOFDESIGNPOWERFREQUENCY = 设计工频摇摆角度 = %s\n",(char*)ss);
		//设计雷电摇摆角度
		ss.Printf("%f",m_xTowerProp.m_fLightningRockAngle);
		SimplifiedNumString(ss);
		fprintf(fp,"ROCKANGLEOFDESIGNLIGHTNING = 设计雷电摇摆角度 = %s\n",(char*)ss);
		//设计操作摇摆角度
		ss.Printf("%f",m_xTowerProp.m_fSwitchingRockAngle);
		SimplifiedNumString(ss);
		fprintf(fp,"ROCKANGLEOFDESIGNSWITCHING = 设计操作摇摆角度 = %s\n",(char*)ss);
		//设计带电作业摇摆角度
		ss.Printf("%f",m_xTowerProp.m_fWorkingRockAngle);
		SimplifiedNumString(ss);
		fprintf(fp,"ROCKANGLEOFDESIGNLIVEWORKING = 设计带电作业摇摆角度 = %s\n",(char*)ss);
		//生产厂家
		fprintf(fp,"MANUFACTURER = 生产厂家 = %s\n",(char*)m_xTowerProp.m_sManuFacturer);
		//物资编码
		fprintf(fp,"MATERIALCODE = 物资编码 = %s\n",(char*)m_xTowerProp.m_sMaterialCode);
		//装配模型编号
		fprintf(fp,"PROCESSMODELCODE = 装配模型编号 = %s\n",(char*)m_xTowerProp.m_sProModelCode);
	}
	else
	{
		CXhChar500 sValue;
		wchar_t sWValue[MAX_PATH];
		//电压等级
		sValue.Printf("VOLTAGE = 电压等级 = %s",(char*)m_xTowerProp.m_sVoltGrade);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//塔型
		sValue.Printf("TYPE = 塔型 = %s",(char*)m_xTowerProp.m_sType);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//杆塔材质
		sValue.Printf("TEXTURE = 杆塔材质 = %s",(char*)m_xTowerProp.m_sTexture);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//固定方式
		sValue.Printf("FIXEDTYPE = 固定方式 = %s",(char*)m_xTowerProp.m_sFixedType);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//杆塔类型
		sValue.Printf("TOWERTYPE = 杆塔类型 = %s",(char*)m_xTowerProp.m_sTaType);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//回路数
		sValue.Printf("CIRCUIT = 回路数 = %d",m_xTowerProp.m_nCircuit);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//导线型号
		sValue.Printf("CONDUCTOR = 导线型号 = %s",(char*)m_xTowerProp.m_sCWireSpec);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//地线型号
		sValue.Printf("GROUNDWIRE = 地线型号 = %s",(char*)m_xTowerProp.m_sEWireSpec);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//设计基本风速
		ss.Printf("%f",m_xTowerProp.m_fWindSpeed);
		SimplifiedNumString(ss);
		sValue.Printf("REFERENCEWINDSPEED = 设计基本风速 = %s",(char*)ss);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//设计覆冰厚度
		ss.Printf("%f",m_xTowerProp.m_fNiceThick);
		SimplifiedNumString(ss);
		sValue.Printf("DESIGNICETHICKNESS = 设计覆冰厚度 = %s",(char*)ss);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//设计水平档距
		sValue.Printf("DESIGNWINDSPAN = 设计水平档距 = %s",(char*)m_xTowerProp.m_sWindSpan);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//设计垂直档距
		sValue.Printf("DESIGNWEIGHTSPAN = 设计垂直档距 = %s",(char*)m_xTowerProp.m_sWeightSpan);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//设计前侧代表档距
		ss.Printf("%f",m_xTowerProp.m_fFrontRulingSpan);
		SimplifiedNumString(ss);
		sValue.Printf("DESIGNFRONTRULINGSPAN = 设计前侧代表档距 = %s",(char*)ss);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//设计后侧代表档距
		ss.Printf("%f",m_xTowerProp.m_fBackRulingSpan);
		SimplifiedNumString(ss);
		sValue.Printf("DESIGNBACKRULINGSPAN = 设计后侧代表档距 = %s",(char*)ss);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//设计最大档距
		ss.Printf("%f",m_xTowerProp.m_fMaxSpan);
		SimplifiedNumString(ss);
		sValue.Printf("DESIGNMAXSPAN = 设计最大档距 = %s",(char*)ss);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//设计转角范围
		sValue.Printf("ANGLERANGE = 设计转角范围 = %s",(char*)m_xTowerProp.m_sAngleRange);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//Kv
		ss.Printf("%f",m_xTowerProp.m_fDesignKV);
		SimplifiedNumString(ss);
		sValue.Printf("DESIGNKV = 设计Kv值 = %s",(char*)ss);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//计算呼高
		sValue.Printf("RATEDNOMINALHEIGHT = 计算呼高 = %s",(char*)m_xTowerProp.m_sRatedHeight);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//呼高范围
		sValue.Printf("NOMINALHEIGHTRANGE = 呼高范围 = %s",(char*)m_xTowerProp.m_sHeightRange);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//塔重
		sValue.Printf("TOWERWEIGHT = 塔重 = %s",(char*)m_xTowerProp.m_sTowerWeight);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//设计工频摇摆角度
		ss.Printf("%f",m_xTowerProp.m_fFrequencyRockAngle);
		SimplifiedNumString(ss);
		sValue.Printf("ROCKANGLEOFDESIGNPOWERFREQUENCY = 设计工频摇摆角度 = %s",(char*)ss);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//设计雷电摇摆角度
		ss.Printf("%f",m_xTowerProp.m_fLightningRockAngle);
		SimplifiedNumString(ss);
		sValue.Printf("ROCKANGLEOFDESIGNLIGHTNING = 设计雷电摇摆角度 = %s",(char*)ss);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//设计操作摇摆角度
		ss.Printf("%f",m_xTowerProp.m_fSwitchingRockAngle);
		SimplifiedNumString(ss);
		sValue.Printf("ROCKANGLEOFDESIGNSWITCHING = 设计操作摇摆角度 = %s",(char*)ss);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//设计带电作业摇摆角度
		ss.Printf("%f",m_xTowerProp.m_fWorkingRockAngle);
		SimplifiedNumString(ss);
		sValue.Printf("ROCKANGLEOFDESIGNLIVEWORKING = 设计带电作业摇摆角度 = %s",(char*)ss);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//生产厂家
		sValue.Printf("MANUFACTURER = 生产厂家 = %s",(char*)m_xTowerProp.m_sManuFacturer);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//物资编码
		sValue.Printf("MATERIALCODE = 物资编码 = %s",(char*)m_xTowerProp.m_sMaterialCode);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//装配模型编号
		sValue.Printf("PROCESSMODELCODE = 装配模型编号 = %s",(char*)m_xTowerProp.m_sProModelCode);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
	}
	fclose(fp);
}
void CTMDHGim::WriteCbmFile(const char* sCbmFile)
{
	FILE *fp=NULL;
	if(m_ciCodingType==ANSI_CODING)
		fp=fopen(sCbmFile,"wt");
	else
		fp=fopen(sCbmFile,"wt,ccs=UTF-8");
	if(fp==NULL)
		return;
	if(m_ciCodingType==ANSI_CODING)
	{
		fprintf(fp,"ENTITYNAME = Device\n");
		fprintf(fp,"OBJECTMODELPOINTER = %s.dev\n",(char*)m_sDevGuid);
		fprintf(fp,"BASEFAMILY = \n");
		fprintf(fp,"TRANSFORMMATRIX = 0.000000,0.000000,-0.000000,0.000000,-0.000000,0.000000,-0.000000,0.000000,-0.000000,0.000000,1.000000,0.000000,-0.000000,0.000000,-0.000000,1.000000\n");
	}
	else
	{
		wchar_t sWValue[MAX_PATH];
		ANSIToUnicode(m_sDevGuid,sWValue);
		fwprintf(fp,L"ENTITYNAME = Device\n");
		fwprintf(fp,L"OBJECTMODELPOINTER = %s.dev\n",sWValue);
		fwprintf(fp,L"BASEFAMILY = \n");
		fwprintf(fp,L"TRANSFORMMATRIX = 0.000000,0.000000,-0.000000,0.000000,-0.000000,0.000000,-0.000000,0.000000,-0.000000,0.000000,1.000000,0.000000,-0.000000,0.000000,-0.000000,1.000000\n");
	}
	fclose(fp);
}
void CTMDHGim::ReadFamFile(const char* sFamFile)
{
	FILE *fp=fopen(sFamFile,"rt");
	if(fp==NULL)
	{
		logerr.Log("%s打开失败!",sFamFile);
		return;
	}
	char sLine1[MAX_PATH]="",sLine2[MAX_PATH]="";
	while(!feof(fp))
	{
		if(fgets(sLine1,MAX_PATH,fp)==NULL)
			continue;
		if(m_ciCodingType==UTF8_CODING)
			UTF8ToANSI(sLine1,sLine2);
		else
			strcpy(sLine2,sLine1);
		CXhChar100 sText(sLine2);
		sText.Replace('\t',' ');
		sText.Replace('\n',' ');
		sText.Remove(' ');
		char *skey1=strtok(sText,"=");
		char *skey2=strtok(NULL,"=");
		char *skey=strtok(NULL,"=");
		if(skey==NULL||strlen(skey)<=0||skey[0]==10)
			continue;
		if(strstr(skey1,"VOLTAGE")||stricmp(skey2,"电压等级")==0)
			m_xTowerProp.m_sVoltGrade.Copy(skey);
		else if(stricmp(skey1,"TYPE")==0||stricmp(skey2,"塔型")==0)
			m_xTowerProp.m_sType.Copy(skey);
		else if(stricmp(skey1,"TEXTURE")==0||stricmp(skey2,"杆塔材质")==0)
			m_xTowerProp.m_sTexture.Copy(skey);
		else if(stricmp(skey1,"FIXEDTYPE")==0||stricmp(skey2,"固定方式")==0)
			m_xTowerProp.m_sFixedType.Copy(skey);
		else if(stricmp(skey1,"TOWERTYPE")==0||stricmp(skey2,"杆塔类型")==0)
			m_xTowerProp.m_sTaType.Copy(skey);
		else if(stricmp(skey1,"CIRCUIT")==0||stricmp(skey2,"回路数")==0)
			m_xTowerProp.m_nCircuit=atoi(skey);
		else if(stricmp(skey1,"CONDUCTOR")==0||stricmp(skey2,"导线型号")==0)
			m_xTowerProp.m_sCWireSpec.Copy(skey);
		else if(stricmp(skey1,"GROUNDWIRE")==0||stricmp(skey2,"地线型号")==0)
			m_xTowerProp.m_sEWireSpec.Copy(skey);
		else if(stricmp(skey1,"REFERENCEWINDSPEED")==0||stricmp(skey2,"设计基本风速")==0)
			m_xTowerProp.m_fWindSpeed=atof(skey);
		else if(stricmp(skey1,"DESIGNICETHICKNESS")==0||stricmp(skey2,"设计覆冰厚度")==0)
			m_xTowerProp.m_fNiceThick=atof(skey);
		else if(stricmp(skey1,"DESIGNWINDSPAN")==0||stricmp(skey2,"设计水平档距")==0)
			m_xTowerProp.m_sWindSpan.Copy(skey);
		else if(stricmp(skey1,"DESIGNWEIGHTSPAN")==0||stricmp(skey2,"设计垂直档距")==0)
			m_xTowerProp.m_sWeightSpan.Copy(skey);
		else if(stricmp(skey1,"DESIGNFRONTRULINGSPAN")==0||stricmp(skey2,"设计前侧代表档距")==0)
			m_xTowerProp.m_fFrontRulingSpan=atof(skey);
		else if(stricmp(skey1,"DESIGNBACKRULINGSPAN")==0||stricmp(skey2,"设计后侧代表档距")==0)
			m_xTowerProp.m_fBackRulingSpan=atof(skey);
		else if(stricmp(skey1,"DESIGNMAXSPAN")==0||stricmp(skey2,"设计最大档距")==0)
			m_xTowerProp.m_fMaxSpan=atof(skey);
		else if(stricmp(skey1,"ANGLERANGE")==0||stricmp(skey2,"设计转角范围")==0)
			m_xTowerProp.m_sAngleRange.Copy(skey);
		else if(stricmp(skey1,"DESIGNKV")==0||stricmp(skey2,"设计Kv值")==0)
			m_xTowerProp.m_fDesignKV=atof(skey);
		else if(stricmp(skey1,"RATEDNOMINALHEIGHT")==0||stricmp(skey2,"计算呼高")==0)
			m_xTowerProp.m_sRatedHeight.Copy(skey);
		else if(stricmp(skey1,"NOMINALHEIGHTRANGE")==0||stricmp(skey2,"呼高范围")==0)
			m_xTowerProp.m_sHeightRange.Copy(skey);
		else if(stricmp(skey1,"TOWERWEIGHT")==0||stricmp(skey2,"塔重")==0)
			m_xTowerProp.m_sTowerWeight.Copy(skey);
		else if(stricmp(skey1,"ROCKANGLEOFDESIGNPOWERFREQUENCY")==0||stricmp(skey2,"设计工频摇摆角度")==0)
			m_xTowerProp.m_fFrequencyRockAngle=atof(skey);
		else if(stricmp(skey1,"ROCKANGLEOFDESIGNLIGHTNING")==0||stricmp(skey2,"设计雷电摇摆角度")==0)
			m_xTowerProp.m_fLightningRockAngle=atof(skey);
		else if(stricmp(skey1,"ROCKANGLEOFDESIGNSWITCHING")==0||stricmp(skey2,"设计操作摇摆角度")==0)
			m_xTowerProp.m_fSwitchingRockAngle=atof(skey);
		else if(stricmp(skey1,"ROCKANGLEOFDESIGNLIVEWORKING")==0||stricmp(skey2,"设计带电作业摇摆角度")==0)
			m_xTowerProp.m_fWorkingRockAngle=atof(skey);
		else if(stricmp(skey1,"MANUFACTURER")==0||stricmp(skey2,"生产厂家")==0)
			m_xTowerProp.m_sManuFacturer.Copy(skey);
		else if(stricmp(skey1,"MATERIALCODE")==0||stricmp(skey2,"物资编码")==0)
			m_xTowerProp.m_sMaterialCode.Copy(skey);
		else if(stricmp(skey1,"PROCESSMODELCODE")==0||stricmp(skey2,"装配模型编号")==0)
			m_xTowerProp.m_sProModelCode.Copy(skey);
	}
	fclose(fp);
}
void CTMDHGim::ReadDevFile(const char* sDevFile)
{
	FILE *fp=fopen(sDevFile,"rt");
	if(fp==NULL)
	{
		logerr.Log("%s打开失败!",sDevFile);
		return;
	}
	CXhChar50 sPhmName;
	char sLine1[MAX_PATH]="",sLine2[MAX_PATH]="";
	while(!feof(fp))
	{
		if(fgets(sLine1,MAX_PATH,fp)==NULL)
			continue;
		if(m_ciCodingType==UTF8_CODING)
			UTF8ToANSI(sLine1,sLine2);
		else
			strcpy(sLine2,sLine1);
		CXhChar100 sText(sLine2);
		sText.Replace('\t',' ');
		sText.Remove(' ');
		char *skey=strtok(sText,"=");
		if(stricmp(skey,"SOLIDMODEL0")==0)
		{
			skey=strtok(NULL,"=");
			sPhmName.Copy(skey);
		}
	}
	fclose(fp);
	//获取FAM&PHM文件
	_splitpath(sDevFile,NULL,NULL,m_sDevGuid,NULL);
	m_sFamGuid=m_sDevGuid;
	if(sPhmName.GetLength()>0)
		_splitpath(sPhmName,NULL,NULL,m_sPhmGuid,NULL);
}
void CTMDHGim::ReadPhmFile(const char* sPhmFile)
{
	FILE *fp=fopen(sPhmFile,"rt");
	if(fp==NULL)
	{
		logerr.Log("%s打开失败!",sPhmFile);
		return;
	}
	CXhChar50 sModName;
	char sLine1[MAX_PATH]="",sLine2[MAX_PATH]="";
	while(!feof(fp))
	{
		if(fgets(sLine1,MAX_PATH,fp)==NULL)
			continue;
		if(m_ciCodingType==UTF8_CODING)
			UTF8ToANSI(sLine1,sLine2);
		else
			strcpy(sLine2,sLine1);
		CXhChar100 sText(sLine2);
		sText.Replace('\t',' ');
		sText.Remove(' ');
		char *skey=strtok(sText,"=");
		if(stricmp(skey,"SOLIDMODEL0")==0)
		{
			skey=strtok(NULL,"=");
			sModName.Copy(skey);
		}
	}
	fclose(fp);
	//获取MOD文件
	if(sModName.GetLength()>0)
		_splitpath(sModName,NULL,NULL,m_sModName,NULL);
}
void CTMDHGim::ReadCbmFile(const char* sCbmFile)
{
	FILE *fp=fopen(sCbmFile,"rt");
	if(fp==NULL)
	{
		logerr.Log("%s打开失败!",sCbmFile);
		return;
	}
	
	fclose(fp);
}
void CTMDHGim::CreateGuidFile()
{
	//在输出目录下生成该塔文件夹
	CXhChar100 sOutPath=m_sOutputPath;
	sOutPath.Append("\\");
	sOutPath.Append(m_sGimName);
	_mkdir(sOutPath);
	//在输出目录下生成MOD文件
	CXhChar100 sFilePath=sOutPath;
	sFilePath.Append("\\MOD");
	_mkdir(sFilePath);
	CXhChar200 sModFilePath("%s\\%s.mod",(char*)sFilePath,(char*)m_sModName);
	WriteModFile(sModFilePath);
	//生成引用MOD文件的PHM文件
	sFilePath=sOutPath;
	sFilePath.Append("\\PHM");
	_mkdir(sFilePath);
	m_sPhmGuid=CreateGuidStr();
	CXhChar200 sPhmFilePath("%s\\%s.phm",(char*)sFilePath,(char*)m_sPhmGuid);
	WritePhmFile(sPhmFilePath,m_sModName);
	//生成引用PHM文件的DEV文件及相应属性文件
	sFilePath=sOutPath;
	sFilePath.Append("\\DEV");
	_mkdir(sFilePath);
	m_sFamGuid=m_sDevGuid=CreateGuidStr();
	CXhChar200 sFamFilePath("%s\\%s.fam",(char*)sFilePath,(char*)m_sFamGuid);
	WriteFamFile(sFamFilePath);
	CXhChar200 sDevFilePath("%s\\%s.dev",(char*)sFilePath,(char*)m_sDevGuid);
	WriteDevFile(sDevFilePath);
	//生成引用DEV文件的设备工程文件CBM
	sFilePath=sOutPath;
	sFilePath.Append("\\CBM");
	_mkdir(sFilePath);
	m_sCbmGuid=CreateGuidStr();
	CXhChar200 sCbmFilePath("%s\\%s.cbm",(char*)sFilePath,(char*)m_sCbmGuid);
	WriteCbmFile(sCbmFilePath);
}
void CTMDHGim::DeleteGuidFile()
{
	CXhChar100 sOutPath=m_sOutputPath;
	sOutPath.Append("\\");
	sOutPath.Append(m_sGimName);
	CXhChar100 sFilePath=sOutPath;
	sFilePath.Append("\\MOD");
	DeleteDirectory(sFilePath);
	sFilePath=sOutPath;
	sFilePath.Append("\\PHM");
	DeleteDirectory(sFilePath);
	sFilePath=sOutPath;
	sFilePath.Append("\\DEV");
	DeleteDirectory(sFilePath);
	sFilePath=sOutPath;
	sFilePath.Append("\\CBM");
	DeleteDirectory(sFilePath);
	DeleteDirectory(sOutPath);
}
void CTMDHGim::ToBuffer(CBuffer &buffer,CBuffer &zipBuffer)
{
	//获取当前时间，并转换为数字型
	if(strlen(m_xGimHeadProp.m_sTime)<=0)
	{
		time_t tt = time(0);
		char sTime[32]={0};
		strftime(sTime, sizeof(sTime),"%Y-%m-%d %H:%M", localtime(&tt));
		memcpy(m_xGimHeadProp.m_sTime,sTime,16);
	}
	CXhString szBuffSize(m_xGimHeadProp.m_sBufSize,8);
	szBuffSize.Printf("%d",zipBuffer.GetLength());
	//写入指定头部分
	buffer.Write(m_xGimHeadProp.m_sFileTag,16);			//文件标识
	buffer.Write(m_xGimHeadProp.m_sFileName,256);		//文件名称
	buffer.Write(m_xGimHeadProp.m_sDesigner,64);		//设计者
	buffer.Write(m_xGimHeadProp.m_sUnit,256);			//组织单位
	buffer.Write(m_xGimHeadProp.m_sSoftName,128);		//软件名称
	buffer.Write(m_xGimHeadProp.m_sTime,16);			//创建时间		
	buffer.Write(m_xGimHeadProp.m_sSoftMajorVer,8);		//软件主版本号
	buffer.Write(m_xGimHeadProp.m_sSoftMinorVer,8);		//软件次版本号
	buffer.Write(m_xGimHeadProp.m_sMajorVersion,8);		//标准主版本号
	buffer.Write(m_xGimHeadProp.m_sMinorVersion,8);		//标准次版本号
	buffer.Write(m_xGimHeadProp.m_sBufSize,8);			//存储区域大小
	//写入7Z部分
	buffer.Write(zipBuffer.GetBufferPtr(),zipBuffer.GetLength());
}
void CTMDHGim::FromBuffer(CBuffer &buffer,CBuffer &zipBuffer)
{
	buffer.SeekToBegin();
	//读取头部分
	buffer.Read(m_xGimHeadProp.m_sFileTag,16);		//文件标识
	buffer.Read(m_xGimHeadProp.m_sFileName,256);	//文件名称
	buffer.Read(m_xGimHeadProp.m_sDesigner,64);		//设计者
	buffer.Read(m_xGimHeadProp.m_sUnit,256);		//组织单位
	buffer.Read(m_xGimHeadProp.m_sSoftName,128);	//软件名称
	buffer.Read(m_xGimHeadProp.m_sTime,16);			//创建时间		
	buffer.Read(m_xGimHeadProp.m_sSoftMajorVer,8);	//软件主版本号
	buffer.Read(m_xGimHeadProp.m_sSoftMinorVer,8);	//软件次版本号
	buffer.Read(m_xGimHeadProp.m_sMajorVersion,8);	//标准主版本号
	buffer.Read(m_xGimHeadProp.m_sMinorVersion,8);	//标准次版本号
	buffer.Read(m_xGimHeadProp.m_sBufSize,8);		//存储区域大小
	//读取7Z部分存储部分
	int buf_size=atoi(m_xGimHeadProp.m_sBufSize);
	zipBuffer.Write(NULL,buf_size);
	buffer.Read(zipBuffer.GetBufferPtr(),buf_size);		//存储区域大小
}
BOOL CTMDHGim::UpdateGimHeadInfo()
{
	//读取7z压缩包默认的字节
	CXhChar100 gim_path("%s\\%s.gim",(char*)m_sOutputPath,(char*)m_sGimName);
	FILE* fp=fopen(gim_path,"rb");
	if(fp==NULL)
	{
		logerr.Log(CXhChar100("%s更新内容失败(读)!",(char*)gim_path));
		return FALSE;
	}
	fseek(fp,0,SEEK_END);
	long buf_size=ftell(fp);
	fseek(fp,0,SEEK_SET);
	CBuffer zipBuf;
	zipBuf.Write(NULL,buf_size);
	fread(zipBuf.GetBufferPtr(),zipBuf.GetLength(),1,fp);
	fclose(fp);
	//在GIM文件头部添加特定内容
	CBuffer gimBuf(10000000);
	ToBuffer(gimBuf,zipBuf);
	fp=fopen(gim_path,"wb");
	if(fp==NULL)
	{
		logerr.Log(CXhChar100("%s更新内容失败(写)!",(char*)gim_path));
		return FALSE;
	}
	fwrite(gimBuf.GetBufferPtr(),gimBuf.GetLength(),1,fp);
	fclose(fp);
	return TRUE;
}
BOOL CTMDHGim::UpdateGimTowerPro()
{
	//重写FAM文件
	CXhChar100 sFilePath=m_sOutputPath;
	sFilePath.Append("\\");
	sFilePath.Append(m_sGimName);
	sFilePath.Append("\\DEV");
	CXhChar200 sFamFilePath("%s\\%s.fam",(char*)sFilePath,(char*)m_sFamGuid);
	WriteFamFile(sFamFilePath);
	//更新GIM包数据
	CXhChar100 gim_path("%s\\%s.gim",(char*)m_sOutputPath,(char*)m_sGimName);
	CXhChar200 cmd_str("7z.exe u %s %s\\",(char*)gim_path,(char*)sFilePath);
	if(!CTMDHManager::Run7zCmd(cmd_str))
		return FALSE;
	return TRUE;
}
//进行压缩打包处理
BOOL CTMDHGim::PackGimFile()
{
	//默认压缩为7Z格式包
	CXhChar100 sOutPath=m_sOutputPath;
	sOutPath.Append("\\");
	sOutPath.Append(m_sGimName);
	CXhChar100 gim_path("%s\\%s.gim",(char*)m_sOutputPath,(char*)m_sGimName);
	//7z.exe V18.5支持压缩后自动删除文件夹
	//CXhChar200 cmd_str("7z.exe a %s %s\\* -sdel",(char*)gim_path,(char*)sOutPath);
	//7z.exe V4.65不支持自动删除文件夹
	CXhChar200 cmd_str("7z.exe a %s %s\\*",(char*)gim_path,(char*)sOutPath);
	if(!CTMDHManager::Run7zCmd(cmd_str))
		return FALSE;
	//删除临时生成的文件夹
	DeleteGuidFile();
	return TRUE;
}
//
void CTMDHGim::CheckModData()
{
	CXhChar200 sModFile("%s\\%s.mod",(char*)m_sModPath,(char*)m_sModName);
	BOOL bUtf8=IsUTF8File(sModFile);
	FILE *fp=fopen(sModFile,"rt");
	if(fp==NULL)
		return;
	int nRow=0;
	BYTE ciReadType=0;	//0:本体|1.接腿|2.子腿
	CXhChar100 line_txt,sText,key_word;
	char sLine1[MAX_PATH]="",sLine2[MAX_PATH]="";
	while(!feof(fp))
	{
		if(fgets(sLine1,MAX_PATH,fp)==NULL)
			break;
		nRow+=1;
		if(bUtf8)
			UTF8ToANSI(sLine1,sLine2);
		else
			strcpy(sLine2,sLine1);
		line_txt.Copy(sLine2);
		line_txt.Replace('\t',' ');
		line_txt.Remove(' ');
		strcpy(sText,line_txt);
		char *skey=strtok(sText,",");
		strncpy(key_word,skey,100);
		if(strstr(key_word,"HNum"))
		{
			if((skey=strtok(NULL,","))==NULL)
				CModFileError::SetError(CModFileError::ERROR_MOD_HNUM_MISS,nRow);
			continue;
		}
		if(strstr(key_word,"HBody"))
		{
			if((skey=strtok(NULL,","))==NULL)
				CModFileError::SetError(CModFileError::ERROR_MOD_HBODY_MISS,nRow);
			continue;
		}
		if(strstr(key_word,"HLeg"))
		{
			skey=strtok(NULL,",");
			double fItemHeight=(skey!=NULL)?atof(skey):0;
			if((skey=strtok(NULL,","))==NULL)
				CModFileError::SetError(CModFileError::ERROR_MOD_HLEG_MISS,nRow);
			continue;
		}
		if(strstr(key_word,"HSubLeg"))
		{
			if((skey=strtok(NULL,","))==NULL)
				CModFileError::SetError(CModFileError::ERROR_MOD_HSUBLEG_MISS,nRow);
			continue;
		}
		if(stricmp(key_word,"H")==0)
		{	//
			if((skey=strtok(NULL,","))==NULL)
				CModFileError::SetError(CModFileError::ERROR_MOD_BODY_H,nRow);
			continue;
		}
		else if(strstr(key_word,"Body")&&key_word[0]=='B')
		{	//本体
			ciReadType=0;
			continue;
		}
		else if(strstr(key_word,"Leg")&&key_word[0]=='L')
		{	//接腿
			ciReadType=1;
			continue;
		}
		else if(strstr(key_word,"SubLeg")&&key_word[0]=='S')
		{	//子腿长=共有部分最低点-腿部最低点
			ciReadType=2;
			continue;
		}
		else
		{
			if(stricmp(key_word,"G")!=0)
				line_txt.Replace(',',' ');
			key_word.Remove(' ');
			if(stricmp(key_word,"P")==0)
			{	//节点
				int i=0;
				for(skey=strtok(NULL,",");skey;skey=strtok(NULL,","))
					i++;
				if(i!=4)
				{
					CModFileError::SetError(CModFileError::ERROR_MOD_P_MISS,nRow);
					return;
				}
			}
			else if(stricmp(key_word,"R")==0)
			{	//杆件
				int indexS=0,indexE=0;
				double fWidth=0,fThick=0;
				char sMat[16]="",sSpec[16]="";
				if(strstr(line_txt,"L"))
				{	//角钢
					int i=0;
					for(skey=strtok(NULL,",");skey;skey=strtok(NULL,","))
						i++;
					if(i!=10)
					{
						CModFileError::SetError(CModFileError::ERROR_MOD_R_L_MISS,nRow);
						return;
					}
				}
				else if(strstr(line_txt,"Φ")||strstr(line_txt,"φ"))
				{	//钢管
					int i=0;
					for(skey=strtok(NULL,",");skey;skey=strtok(NULL,","))
						i++;
					if(i!=3)
					{
						CModFileError::SetError(CModFileError::ERROR_MOD_R_T_MISS,nRow);
						return;
					}
				}
				else
				{	//类型不存在
					logerr.Log("Mod第%d行未知杆件!",nRow);
					return;
				}
			}
		}
	}
	fclose(fp);
}
void* CTMDHGim::ExtractActiveTaInstance()
{
	IModModel* pModModel=Manager.m_pModModel;
	if(pModModel==NULL || !pModModel->ImportModFile(m_sModFile))
		return NULL;
	IModHeightGroup* pHeightGroup=pModModel->GetHeightGroup(m_uiHeightSerial);
	if(pHeightGroup==NULL)
		return NULL;
	pHeightGroup->GetName(m_sHuGao);
	IModTowerInstance* pInstance=NULL;
	pInstance=pHeightGroup->GetTowerInstance(m_uiLegSerialArr[0],m_uiLegSerialArr[1],m_uiLegSerialArr[2],m_uiLegSerialArr[3]);
	if(pInstance==NULL)
		return NULL;
	return pInstance;
}
//属性栏
const DWORD HASHTABLESIZE = 500;
IMPLEMENT_PROP_FUNC(CTMDHGim);
void CTMDHGim::InitPropHashtable()
{
	int id = 1;
	propHashtable.SetHashTableGrowSize(HASHTABLESIZE);
	propStatusHashtable.CreateHashTable(50);
	//基本信息
	AddPropItem("BasicInfo",PROPLIST_ITEM(id++,"基本信息","基本信息"));
	AddPropItem("Designer",PROPLIST_ITEM(id++,"设计者","设计者"));
	AddPropItem("Unit",PROPLIST_ITEM(id++,"组织单位","组织单位"));
	AddPropItem("MajorVer",PROPLIST_ITEM(id++,"主版本号","主版本号"));
	AddPropItem("MinorVer",PROPLIST_ITEM(id++,"次版本号","次版本号"));
	//设计信息
	AddPropItem("TowerInfo",PROPLIST_ITEM(id++,"设计信息","设计信息"));
	AddPropItem("VoltGrade",PROPLIST_ITEM(id++,"电压等级","电压等级"));
	AddPropItem("Type",PROPLIST_ITEM(id++,"塔型","塔型"));
	AddPropItem("Texture",PROPLIST_ITEM(id++,"杆塔材质","杆塔材质","角钢塔|钢管塔|钢管杆|水泥杆|铁杆"));
	AddPropItem("FixedType",PROPLIST_ITEM(id++,"固定方式","固定方式","自立|拉线"));
	AddPropItem("TaType",PROPLIST_ITEM(id++,"杆塔类型","杆塔类型","悬垂塔|耐张塔|换位塔"));
	AddPropItem("Circuit",PROPLIST_ITEM(id++,"回路数","回路数"));
	AddPropItem("CWireSpec",PROPLIST_ITEM(id++,"导线型号","导线型号"));
	AddPropItem("EWireSpec",PROPLIST_ITEM(id++,"地线型号","地线型号"));
	AddPropItem("WindSpeed",PROPLIST_ITEM(id++,"设计基本风速","设计基本风速"));
	AddPropItem("NiceThick",PROPLIST_ITEM(id++,"设计覆冰厚度","设计覆冰厚度"));
	AddPropItem("WindSpan",PROPLIST_ITEM(id++,"设计水平档距","设计水平档距"));
	AddPropItem("WeightSpan",PROPLIST_ITEM(id++,"设计垂直档距","设计垂直档距"));
	AddPropItem("MaxSpan",PROPLIST_ITEM(id++,"设计最大档距","最大档距"));
	AddPropItem("FrontRulingSpan",PROPLIST_ITEM(id++,"设计前侧代表档距","前侧代表档距"));
	AddPropItem("BackRulingSpan",PROPLIST_ITEM(id++,"设计后侧代表档距","后侧代表档距"));
	AddPropItem("AngleRange",PROPLIST_ITEM(id++,"设计转角范围","转角范围"));
	AddPropItem("DesignKV",PROPLIST_ITEM(id++,"设计Kv值","设计Kv值"));
	AddPropItem("RatedHeight",PROPLIST_ITEM(id++,"计算呼高","计算呼高"));
	AddPropItem("HeightRange",PROPLIST_ITEM(id++,"呼高范围","呼高范围"));
	AddPropItem("TowerWeight",PROPLIST_ITEM(id++,"塔重","塔重"));
	AddPropItem("FrequencyRockAngle",PROPLIST_ITEM(id++,"设计工频摇摆角度","设计工频摇摆角度"));
	AddPropItem("LightningRockAngle",PROPLIST_ITEM(id++,"设计雷电摇摆角度","设计雷电摇摆角度"));
	AddPropItem("SwitchingRockAngle",PROPLIST_ITEM(id++,"设计操作摇摆角度","设计操作摇摆角度"));
	AddPropItem("WorkingRockAngle",PROPLIST_ITEM(id++,"设计带电作业角度","设计带电作业摇摆角度"));
	AddPropItem("ManuFacturer",PROPLIST_ITEM(id++,"生产厂家","生产厂家"));
	AddPropItem("MaterialCode",PROPLIST_ITEM(id++,"物资编码","物资编码"));
}
int CTMDHGim::GetPropValueStr(long id,char* valueStr,UINT nMaxStrBufLen/*=100*/)	//根据属性ID得到属性值字符串
{
	char sText[101]="";
	if(GetPropID("Designer")==id)
		strncpy(sText,m_xGimHeadProp.m_sDesigner,100);
	else if(GetPropID("Unit")==id)
		strncpy(sText,m_xGimHeadProp.m_sUnit,100);
	else if(GetPropID("MajorVer")==id)
		strcpy(sText,m_xGimHeadProp.m_sMajorVersion);
	else if(GetPropID("MinorVer")==id)
		strcpy(sText,m_xGimHeadProp.m_sMinorVersion);
	else if(GetPropID("VoltGrade")==id)
		strcpy(sText,m_xTowerProp.m_sVoltGrade);
	else if(GetPropID("Texture")==id)
		strcpy(sText,m_xTowerProp.m_sTexture);
	else if(GetPropID("FixedType")==id)
		strcpy(sText,m_xTowerProp.m_sFixedType);
	else if(GetPropID("Type")==id)
		strcpy(sText,m_xTowerProp.m_sType);
	else if(GetPropID("TaType")==id)
		strcpy(sText,m_xTowerProp.m_sTaType);
	else if(GetPropID("Circuit")==id)
		sprintf(sText,"%d",m_xTowerProp.m_nCircuit);
	else if(GetPropID("CWireSpec")==id)
		strcpy(sText,m_xTowerProp.m_sCWireSpec);
	else if(GetPropID("EWireSpec")==id)
		strcpy(sText,m_xTowerProp.m_sEWireSpec);
	else if(GetPropID("WindSpeed")==id)
	{
		sprintf(sText,"%f",m_xTowerProp.m_fWindSpeed);
		SimplifiedNumString(sText);
	}
	else if(GetPropID("NiceThick")==id)
	{
		sprintf(sText,"%f",m_xTowerProp.m_fNiceThick);
		SimplifiedNumString(sText);
	}
	else if(GetPropID("WindSpan")==id)
		strcpy(sText,m_xTowerProp.m_sWindSpan);
	else if(GetPropID("WeightSpan")==id)
		strcpy(sText,m_xTowerProp.m_sWeightSpan);
	else if(GetPropID("FrontRulingSpan")==id)
	{
		sprintf(sText,"%f",m_xTowerProp.m_fFrontRulingSpan);
		SimplifiedNumString(sText);
	}
	else if(GetPropID("BackRulingSpan")==id)
	{
		sprintf(sText,"%f",m_xTowerProp.m_fBackRulingSpan);
		SimplifiedNumString(sText);
	}
	else if(GetPropID("MaxSpan")==id)
	{
		sprintf(sText,"%f",m_xTowerProp.m_fMaxSpan);
		SimplifiedNumString(sText);
	}
	else if(GetPropID("AngleRange")==id)
		strcpy(sText,m_xTowerProp.m_sAngleRange);
	else if(GetPropID("DesignKV")==id)
	{
		sprintf(sText,"%f",m_xTowerProp.m_fDesignKV);
		SimplifiedNumString(sText);
	}
	else if(GetPropID("RatedHeight")==id)
		strcpy(sText,m_xTowerProp.m_sRatedHeight);
	else if(GetPropID("HeightRange")==id)
		strcpy(sText,m_xTowerProp.m_sHeightRange);
	else if(GetPropID("TowerWeight")==id)
		strcpy(sText,m_xTowerProp.m_sTowerWeight);
	else if(GetPropID("FrequencyRockAngle")==id)
	{
		sprintf(sText,"%f",m_xTowerProp.m_fFrequencyRockAngle);
		SimplifiedNumString(sText);
	}
	else if(GetPropID("LightningRockAngle")==id)
	{
		sprintf(sText,"%f",m_xTowerProp.m_fLightningRockAngle);
		SimplifiedNumString(sText);
	}
	else if(GetPropID("SwitchingRockAngle")==id)
	{
		sprintf(sText,"%f",m_xTowerProp.m_fSwitchingRockAngle);
		SimplifiedNumString(sText);
	}
	else if(GetPropID("WorkingRockAngle")==id)
	{
		sprintf(sText,"%f",m_xTowerProp.m_fWorkingRockAngle);
		SimplifiedNumString(sText);
	}
	else if(GetPropID("ManuFacturer")==id)
		strcpy(sText,m_xTowerProp.m_sManuFacturer);
	else if(GetPropID("MaterialCode")==id)
		strcpy(sText,m_xTowerProp.m_sMaterialCode);
	if(valueStr)
		StrCopy(valueStr,sText,nMaxStrBufLen);
	return strlen(sText);
}
//////////////////////////////////////////////////////////////////////////
//CTMDHPrjGim
CTMDHPrjGim::CTMDHPrjGim()
{
	m_xDevList.Empty();
}
CTMDHPrjGim::~CTMDHPrjGim()
{

}
void CTMDHPrjGim::ReadGimHeadInfo(CBuffer &buffer)
{
	buffer.SeekToBegin();
	//读取头部分
	buffer.Read(m_xGimHeadProp.m_sFileTag,16);		//文件标识
	buffer.Read(m_xGimHeadProp.m_sFileName,256);	//文件名称
	buffer.Read(m_xGimHeadProp.m_sDesigner,64);		//设计者
	buffer.Read(m_xGimHeadProp.m_sUnit,256);		//组织单位
	buffer.Read(m_xGimHeadProp.m_sSoftName,128);	//软件名称
	buffer.Read(m_xGimHeadProp.m_sTime,16);			//创建时间		
	buffer.Read(m_xGimHeadProp.m_sSoftMajorVer,8);	//软件主版本号
	buffer.Read(m_xGimHeadProp.m_sSoftMinorVer,8);	//软件次版本号
	buffer.Read(m_xGimHeadProp.m_sMajorVersion,8);	//标准主版本号
	buffer.Read(m_xGimHeadProp.m_sMinorVersion,8);	//标准次版本号
	buffer.Read(m_xGimHeadProp.m_sBufSize,8);		//存储区域大小
}
void CTMDHPrjGim::DeleteGimFolder(const char* sFolder)
{
	CXhChar100 sOutPath(sFolder);
	CXhChar100 sFilePath=sOutPath;
	sFilePath.Append("\\MOD");
	DeleteDirectory(sFilePath);
	sFilePath=sOutPath;
	sFilePath.Append("\\PHM");
	DeleteDirectory(sFilePath);
	sFilePath=sOutPath;
	sFilePath.Append("\\DEV");
	DeleteDirectory(sFilePath);
	sFilePath=sOutPath;
	sFilePath.Append("\\CBM");
	DeleteDirectory(sFilePath);
	DeleteDirectory(sOutPath);
}
void CTMDHPrjGim::SearchFile(ATOM_LIST<CString>& xFileList,CXhChar200 sFilePath,BYTE ciFileType/*=0*/)
{
	CFileFind file_find;
	BOOL bFind=file_find.FindFile(sFilePath);
	while(bFind)
	{
		bFind=file_find.FindNextFile();
		if(file_find.IsDots()||file_find.IsHidden()||file_find.IsReadOnly()||
			file_find.IsSystem()||file_find.IsTemporary())
			continue;
		CString file_path=file_find.GetFilePath();
		if(file_find.IsDirectory())
		{
			CString file_name=file_find.GetFileName();
			if(ciFileType==0 ||
				(ciFileType==CBM_FILE && file_name=="CBM")||
				(ciFileType==DEV_FILE && file_name=="DEV")||
				(ciFileType==FAM_FILE && file_name=="DEV")||
				(ciFileType==PHM_FILE && file_name=="PHM")||
				(ciFileType==MOD_FILE && file_name=="MOD"))
				SearchFile(xFileList,CXhChar200("%s\\*",file_path));
		}
		else
			xFileList.append(file_path);
	}
	file_find.Close();
}
BOOL CTMDHPrjGim::IsTowerDevFile(const char* sDevFile)
{
	FILE *fp=fopen(sDevFile,"rt");
	if(fp==NULL)
		return FALSE;
	BOOL bTowerDevFile=FALSE;
	char sLine1[MAX_PATH]="",sLine2[MAX_PATH]="";
	while(!feof(fp))
	{
		if(fgets(sLine1,MAX_PATH,fp)==NULL)
			continue;
		UTF8ToANSI(sLine1,sLine2);
		CXhChar100 sText(sLine2);
		sText.Replace('\t',' ');
		sText.Remove(' ');
		char *skey=strtok(sText,"=");
		skey=strtok(NULL,"=");
		if(skey==NULL)
			continue;
		if(strstr(skey,"TOWER"))
		{
			bTowerDevFile=TRUE;
			break;
		}
	}
	fclose(fp);
	return bTowerDevFile;
}
BOOL CTMDHPrjGim::ParseGimFile(const char* gim_file)
{
	//获取头部信息
	FILE* fp=fopen(gim_file,"rb");
	if(fp==NULL)
	{
		logerr.Log("GIM文件读取失败!");
		return FALSE;
	}
	fseek(fp,0,SEEK_END);
	long buf_size=ftell(fp);
	fseek(fp,0,SEEK_SET);
	CBuffer gimBuf;
	gimBuf.Write(NULL,buf_size);
	fread(gimBuf.GetBufferPtr(),gimBuf.GetLength(),1,fp);
	fclose(fp);
	ReadGimHeadInfo(gimBuf);
	//解压并解析DEV
	CString sCmd;
	CXhChar100 sOutPath("%s\\%s",(char*)m_sGimPath,(char*)m_sGimName);
	Manager.DisplayProcess(0,"解析DEV文件....");
	sCmd.Format("7z.exe x %s DEV\\*.dev -o%s -aoa",gim_file,(char*)sOutPath);
	if(!CTMDHManager::Run7zCmd(sCmd.GetBuffer()))
		return FALSE;
	m_xDevList.Empty();
	ATOM_LIST<CString> xDevFileList;
	CXhChar200 sFilePath("%s\\*",(char*)sOutPath);
	SearchFile(xDevFileList,sFilePath);
	int index=0,nFileNum=xDevFileList.GetNodeNum();
	for(index=0;index<nFileNum;index++)
	{
		Manager.DisplayProcess(ftoi(100*index/nFileNum),"解析DEV文件....");
		CString fdPath=xDevFileList[index];
		CString str_ext=fdPath.Right(4);	//取后缀名
		str_ext.MakeLower();
		if(str_ext.CompareNoCase(".dev")!=0)
			continue;
		if(!IsTowerDevFile(fdPath))
			continue;
		CTMDHGim* pGim=m_xDevList.append();
		pGim->m_sGimName=m_sGimName;
		pGim->m_sOutputPath=m_sGimPath;
		pGim->ReadDevFile(fdPath);
	}
	Manager.DisplayProcess(100,"解析DEV文件....");
	//解压并解析FAM文件
	CString file_str;
	nFileNum=m_xDevList.GetNodeNum();
	if(nFileNum<=0)
		return FALSE;
	Manager.DisplayProcess(0,"解析FAM文件....");
	for(CTMDHGim* pGim=m_xDevList.GetFirst();pGim;pGim=m_xDevList.GetNext())
		file_str.Append(CXhChar50("DEV\\%s.fam ",(char*)pGim->m_sFamGuid));
	sCmd.Format("7z.exe x %s %s -o%s -aoa",gim_file,file_str.GetBuffer(),(char*)sOutPath);
	if(!CTMDHManager::Run7zCmd(sCmd.GetBuffer()))
		return FALSE;
	index=1;
	for(CTMDHGim* pGim=m_xDevList.GetFirst();pGim;pGim=m_xDevList.GetNext(),index++)
	{
		Manager.DisplayProcess(ftoi(100*index/nFileNum),"解析FAM文件....");
		sFilePath=sOutPath;
		sFilePath.Append("\\DEV");
		CXhChar200 sFamFile("%s\\%s.fam",(char*)sFilePath,(char*)pGim->m_sFamGuid);
		pGim->ReadFamFile(sFamFile);
	}
	Manager.DisplayProcess(100,"解析FAM文件....");
	//解压并解析PHM文件
	file_str.Empty();
	Manager.DisplayProcess(0,"解析PHM文件....");
	for(CTMDHGim* pGim=m_xDevList.GetFirst();pGim;pGim=m_xDevList.GetNext())
		file_str.Append(CXhChar50("PHM\\%s.phm ",(char*)pGim->m_sPhmGuid));
	sCmd.Format("7z.exe x %s %s -o%s -aoa",gim_file,file_str.GetBuffer(),(char*)sOutPath);
	if(!CTMDHManager::Run7zCmd(sCmd.GetBuffer()))
		return FALSE;
	index=1;
	for(CTMDHGim* pGim=m_xDevList.GetFirst();pGim;pGim=m_xDevList.GetNext(),index++)
	{
		Manager.DisplayProcess(ftoi(100*index/nFileNum),"解析PHM文件....");
		sFilePath=sOutPath;
		sFilePath.Append("\\PHM");
		CXhChar200 sPhmFile("%s\\%s.phm",(char*)sFilePath,(char*)pGim->m_sPhmGuid);
		pGim->ReadPhmFile(sPhmFile);
	}
	Manager.DisplayProcess(100,"解析PHM文件....");
	//解压并解析MOD文件
	file_str.Empty();
	for(CTMDHGim* pGim=m_xDevList.GetFirst();pGim;pGim=m_xDevList.GetNext())
		file_str.Append(CXhChar50("MOD\\%s.mod ",(char*)pGim->m_sModName));
	sCmd.Format("7z.exe x %s %s -o%s -aoa",gim_file,file_str.GetBuffer(),(char*)sOutPath);
	if(!CTMDHManager::Run7zCmd(sCmd.GetBuffer()))
		return FALSE;
	index=1;
	for(CTMDHGim* pGim=m_xDevList.GetFirst();pGim;pGim=m_xDevList.GetNext(),index++)
	{
		Manager.DisplayProcess(ftoi(100*index/nFileNum),"解析MOD文件....");
		pGim->m_sModFile.Printf("%s\\MOD\\%s.mod",(char*)sOutPath,(char*)pGim->m_sModName);
		if (Manager.m_pModModel == NULL)
			Manager.m_pModModel = CModModelFactory::CreateModModel();
	}
	Manager.DisplayProcess(100,"解析MOD文件....");
	//删除临时文件夹
	//DeleteGimFolder(sOutPath);
	return TRUE;
}
void CTMDHPrjGim::UnpackGimFile(const char* sFolder)
{
	CXhChar200 sGimFile("%s\\%s.gim",(char*)m_sGimPath,(char*)m_sGimName);
	CXhChar200 sOutPath("%s\\%s",sFolder,(char*)m_sGimName);
	CXhChar200 sCmd("7z.exe x %s -o%s -aoa",(char*)sGimFile,(char*)sOutPath);
	CTMDHManager::Run7zCmd(sCmd);
}
BOOL CTMDHPrjGim::UpdateGimTowerPro()
{
	//重写FAM文件
	CXhChar100 sFilePath=m_sGimPath;
	sFilePath.Append("\\");
	sFilePath.Append(m_sGimName);
	sFilePath.Append("\\DEV");
	for(CTMDHGim* pGim=m_xDevList.GetFirst();pGim;pGim=m_xDevList.GetNext())
	{
		CXhChar200 sFamFilePath("%s\\%s.fam",(char*)sFilePath,(char*)pGim->m_sFamGuid);
		pGim->WriteFamFile(sFamFilePath);
	}
	//更新GIM包数据
	CXhChar100 gim_path("%s\\%s.gim",(char*)m_sGimPath,(char*)m_sGimName);
	CXhChar200 cmd_str("7z.exe u %s %s\\",(char*)gim_path,(char*)sFilePath);
	if(!CTMDHManager::Run7zCmd(cmd_str))
		return FALSE;
	return TRUE;
}
//////////////////////////////////////////////////////////////////////////
//CTMDHManager
CTMDHManager::CTMDHManager()
{
	m_pActiveTa=NULL;
	DisplayProcess=NULL;
	m_xTowerDataList.Empty();
	m_xModelDataList.Empty();
	m_xGimDataList.Empty();
	m_xPrjGimList.Empty();
	m_pTidModel=NULL;
	m_pModModel=NULL;
}
CTMDHManager::~CTMDHManager()
{
	if(m_pTidModel)
		CTidModelFactory::Destroy(m_pTidModel->GetSerialId());
	m_pTidModel=NULL;
	//
	if(m_pModModel)
		CModModelFactory::Destroy(m_pModModel->GetSerialId());
	m_pModModel=NULL;
}
BOOL CTMDHManager::Run7zCmd(char* sCmd)
{
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(pi));
	if (!CreateProcess(NULL,sCmd,NULL,NULL,FALSE,CREATE_NO_WINDOW,NULL, NULL, &si,&pi))
	{
		logerr.Log("进程启动失败");
		return FALSE;
	}
	//等待进程结束
	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return TRUE;
}
void CTMDHManager::DeleteTa(CTMDHTower* pTower)
{
	if(pTower==NULL)
		return;
	for(CTMDHTower* pData=m_xTowerDataList.GetFirst();pData;pData=m_xTowerDataList.GetNext())
	{
		if(pData==pTower)
		{
			m_xTowerDataList.DeleteCursor();
			break;
		}	
	}
	m_xTowerDataList.Clean();
}
void CTMDHManager::DeleteModel(CTMDHModel* pModel)
{
	if(pModel==NULL)
		return;
	for(CTMDHModel* pData=m_xModelDataList.GetFirst();pData;pData=m_xModelDataList.GetNext())
	{
		if(pData==pModel)
		{
			m_xModelDataList.DeleteCursor();
			break;
		}
	}
	m_xModelDataList.Clean();
}
void CTMDHManager::DeleteGim(CTMDHGim* pGim)
{
	if(pGim==NULL)
		return;
	for(CTMDHGim* pData=m_xGimDataList.GetFirst();pData;pData=m_xGimDataList.GetNext())
	{
		if(pData==pGim)
		{
			m_xGimDataList.DeleteCursor();
			break;
		}
	}
	m_xGimDataList.Clean();
}
void CTMDHManager::DeletePrjGim(CTMDHPrjGim* pPrjGimGim)
{
	if(pPrjGimGim==NULL)
		return;
	for(CTMDHPrjGim* pData=m_xPrjGimList.GetFirst();pData;pData=m_xPrjGimList.GetNext())
	{
		if(pData==pPrjGimGim)
		{
			m_xPrjGimList.DeleteCursor();
			break;
		}
	}
	m_xPrjGimList.Clean();
}
BOOL CTMDHManager::ReadBatchInstanceFile(const char* sFilePath)
{
	CExcelOperObject xExcelObj;
	if(!xExcelObj.OpenExcelFile(sFilePath))
		return FALSE;
	LPDISPATCH pWorksheets=xExcelObj.GetWorksheets();
	ASSERT(pWorksheets != NULL);
	Sheets       excel_sheets;	//工作表集合
	excel_sheets.AttachDispatch(pWorksheets);
	int nSheetNum=excel_sheets.GetCount();
	if(nSheetNum!=2)
	{
		excel_sheets.ReleaseDispatch();
		return FALSE;
	}
	CVariant2dArray sheet1ContentMap(1,1),sheet2ContentMap(1,1);
	for(int iSheet=1;iSheet<=nSheetNum;iSheet++)
	{
		LPDISPATCH pWorksheet=excel_sheets.GetItem(COleVariant((short) iSheet));
		_Worksheet  excel_sheet;
		excel_sheet.AttachDispatch(pWorksheet);
		excel_sheet.Select();
		Range excel_usedRange,excel_range;
		excel_usedRange.AttachDispatch(excel_sheet.GetUsedRange());
		excel_range.AttachDispatch(excel_usedRange.GetRows());
		long nRowNum = excel_range.GetCount();
		excel_range.AttachDispatch(excel_usedRange.GetColumns());
		long nColNum=excel_range.GetCount();
		CXhChar16 sCell("%C%d",'A'+nColNum,nRowNum+2);
		LPDISPATCH pRange=excel_sheet.GetRange(COleVariant("A1"),COleVariant(sCell));
		excel_range.AttachDispatch(pRange,FALSE);
		if(iSheet==1)
			sheet1ContentMap.var=excel_range.GetValue();
		else
			sheet2ContentMap.var=excel_range.GetValue();
		excel_usedRange.ReleaseDispatch();
		excel_range.ReleaseDispatch();
		excel_sheet.ReleaseDispatch();
	}
	excel_sheets.ReleaseDispatch();
	//解析塔型文件表
	VARIANT value;
	CHashStrList<CXhChar100> hashModelFile;
	for(int i=1;i<sheet2ContentMap.RowsCount();i++)
	{
		//塔型
		sheet2ContentMap.GetValueAt(i,0,value);
		if(value.vt==VT_EMPTY)
			continue;
		CXhChar100 sTowerType=VariantToString(value);
		//文件名称
		sheet2ContentMap.GetValueAt(i,1,value);
		CXhChar100 sFileName=VariantToString(value);
		//文件路程
		sheet2ContentMap.GetValueAt(i,2,value);
		CXhChar100 sPath=VariantToString(value);
		//
		hashModelFile.SetValue(sTowerType,CXhChar100("%s\\%s",(char*)sPath,(char*)sFileName));
	}
	//解析塔型配基表
	CHashStrList<DWORD> hashColIndexByColTitle;
	for(int i=0;i<LOFTING_EXCEL_COL_COUNT;i++)
	{
		sheet1ContentMap.GetValueAt(0,i,value);
		CString itemstr(value.bstrVal);
		if(itemstr.CompareNoCase(T_NO)==0) 
			hashColIndexByColTitle.SetValue(T_NO,i);
		else if(itemstr.CompareNoCase(T_TOWER_NUM)==0)
			hashColIndexByColTitle.SetValue(T_TOWER_NUM,i);
		else if(itemstr.CompareNoCase(T_TOWER_DIAN)==0)
			hashColIndexByColTitle.SetValue(T_TOWER_DIAN,i);
		else if(itemstr.CompareNoCase(T_TOWER_TYPE)==0)
			hashColIndexByColTitle.SetValue(T_TOWER_TYPE,i);
		else if(itemstr.CompareNoCase(T_WIRE_ANGLE)==0)
			hashColIndexByColTitle.SetValue(T_WIRE_ANGLE,i);
		else if(strstr(itemstr,T_LOCATE_HIGH))
			hashColIndexByColTitle.SetValue(T_LOCATE_HIGH,i);
		else if(strstr(itemstr,T_HUGAO_HEIGHT))
			hashColIndexByColTitle.SetValue(T_HUGAO_HEIGHT,i);
		else if(strstr(itemstr,T_BODY_HEIGHTP))
			hashColIndexByColTitle.SetValue(T_BODY_HEIGHTP,i);
		else if(strstr(itemstr,T_LEG_LENGTH))
			hashColIndexByColTitle.SetValue(T_LEG_LENGTH,i);
	}
	if (hashColIndexByColTitle.GetValue(T_NO)==NULL||
		hashColIndexByColTitle.GetValue(T_TOWER_NUM)==NULL||
		hashColIndexByColTitle.GetValue(T_TOWER_DIAN)==NULL||
		hashColIndexByColTitle.GetValue(T_TOWER_TYPE)==NULL||
		hashColIndexByColTitle.GetValue(T_HUGAO_HEIGHT)==NULL||
		hashColIndexByColTitle.GetValue(T_BODY_HEIGHTP)==NULL||
		hashColIndexByColTitle.GetValue(T_LOCATE_HIGH)==NULL||
		hashColIndexByColTitle.GetValue(T_LEG_LENGTH)==NULL)
	{
		xMyErrLog.Log("文件格式不对");
		return FALSE;
	}
	//读取文件，提取塔例
	DisplayProcess(0,"读取批处理配基Exel文件,生成模型进度");
	int nRows=sheet1ContentMap.RowsCount();
	for(int i=1;i<=nRows;i++)
	{	
		DisplayProcess(ftoi(100*i/nRows),"读取批处理配基Exel文件,生成模型进度");
		//序号
		DWORD *pColIndex=hashColIndexByColTitle.GetValue(T_NO);
		sheet1ContentMap.GetValueAt(i,*pColIndex,value);
		if(value.vt==VT_EMPTY)
			continue;
		//塔位号
		pColIndex=hashColIndexByColTitle.GetValue(T_TOWER_NUM);
		sheet1ContentMap.GetValueAt(i,*pColIndex,value);
		CXhChar16 sTowerNum=VariantToString(value);
		//塔位点
		pColIndex=hashColIndexByColTitle.GetValue(T_TOWER_DIAN);
		sheet1ContentMap.GetValueAt(i,*pColIndex,value);
		CXhChar16 sTowerDian=VariantToString(value);
		//塔型
		pColIndex=hashColIndexByColTitle.GetValue(T_TOWER_TYPE);
		sheet1ContentMap.GetValueAt(i,*pColIndex,value);
		CXhChar50 sTowerType=VariantToString(value);
		//定位高
		pColIndex=hashColIndexByColTitle.GetValue(T_LOCATE_HIGH);
		sheet1ContentMap.GetValueAt(i,*pColIndex,value);
		CXhChar16 sLocateH=VariantToString(value);
		//线路转角
		pColIndex=hashColIndexByColTitle.GetValue(T_WIRE_ANGLE);
		sheet1ContentMap.GetValueAt(i,*pColIndex,value);
		CXhChar50 sWireAngle=VariantToString(value);
		//呼称高
		pColIndex=hashColIndexByColTitle.GetValue(T_HUGAO_HEIGHT);
		sheet1ContentMap.GetValueAt(i,*pColIndex,value);
		double fHuGao=atof(VariantToString(value));
		CXhChar50 sHuGao=VariantToString(value);
		sHuGao.Append("m");
		//接身高
		pColIndex=hashColIndexByColTitle.GetValue(T_BODY_HEIGHTP);
		sheet1ContentMap.GetValueAt(i,*pColIndex,value);
		double fBodyHeight=atof(VariantToString(value));
		//接腿长度
		double fLegA=0,fLegB=0,fLegC=0,fLegD=0;
		pColIndex=hashColIndexByColTitle.GetValue(T_LEG_LENGTH);
		DWORD dwLegIndex=*pColIndex;
		for(int j=0;j<4;j++)
		{
			sheet1ContentMap.GetValueAt(i,dwLegIndex,value);
			if(j==0)
				fLegA=atof(VariantToString(value));
			else if(j==1)
				fLegB=atof(VariantToString(value));
			else if(j==2)
				fLegC=atof(VariantToString(value));
			else if(j==3)
				fLegD=atof(VariantToString(value));
			dwLegIndex++;
		}
		//读取模型文件，并提取塔例
		CTMDHModel* pModel=AppendModel();
		strcpy(pModel->m_sTowerNum,sTowerNum);
		strcpy(pModel->m_sTowerDot,sTowerDian);
		strcpy(pModel->m_sTowerType,sTowerType);
		strcpy(pModel->m_sHuGao,sHuGao);
		pModel->m_fLocateH=atof(sLocateH);	//定位高
		pModel->m_fBodyHeight=fBodyHeight;	//接身高
		if(strstr(sWireAngle,"左"))
			pModel->m_bTurnLeft=TRUE;
		pModel->m_fMaxLegHeight=fHuGao-fBodyHeight;
		CXhChar100* pFullFilePath=hashModelFile.GetValue(sTowerType);
		if(pFullFilePath==NULL)
		{
			pModel->m_ciErrorType=CTMDHModel::ERROR_NO_FILE;
			continue;
		}
		CXhChar16 extension;
		pModel->m_sFilePath.Copy(*pFullFilePath);
		_splitpath(pModel->m_sFilePath,NULL,NULL,NULL,extension);
		if(extension.EqualNoCase(".tid"))
		{
			pModel->m_ciModelFlag=CTMDHModel::TYPE_TID;
			if(m_pTidModel==NULL)
				m_pTidModel=CTidModelFactory::CreateTidModel();
			if(!m_pTidModel->ReadTidFile(pModel->m_sFilePath))
			{
				pModel->m_ciErrorType=CTMDHModel::ERROR_READ_FILE;
				continue;
			}
		}
		else if(extension.EqualNoCase(".mod"))
		{	
			pModel->m_ciModelFlag=CTMDHModel::TYPE_MOD;
			if(m_pModModel==NULL)
				m_pModModel=CModModelFactory::CreateModModel();
			if(!m_pModModel->ImportModFile(pModel->m_sFilePath))
			{
				pModel->m_ciErrorType=CTMDHModel::ERROR_READ_FILE;
				continue;
			}
		}
		pModel->ExtractActiveTaInstance(sHuGao,fLegA,fLegB,fLegC,fLegD);
	}
	DisplayProcess(100,"读取批处理配基Exel文件,生成模型进度");
	//显示读取结果信息
	int nSumNum=ModelNum(),nErrNum=0;
	for(CTMDHModel* pModel=EnumFirstModel();pModel;pModel=EnumNextModel())
	{
		if(pModel->m_ciErrorType>0)
			nErrNum++;
	}
	int nRightNum=nSumNum-nErrNum;
	xMyErrLog.Log(CXhChar100("读取塔位共有%d个，提取成功%d个，提取失败%d个",nSumNum,nRightNum,nErrNum));
	//
	for(CTMDHModel* pModel=EnumFirstModel();pModel;pModel=EnumNextModel())
	{
		if(pModel->m_ciErrorType==0)
			continue;
		if(pModel->m_ciErrorType==CTMDHModel::ERROR_NO_FILE)
			xMyErrLog.Log("塔型%s-%s找不到对应的文件!",(char*)pModel->m_sTowerNum,(char*)pModel->m_sTowerType);
		else if(pModel->m_ciErrorType==CTMDHModel::ERROR_READ_FILE)
			xMyErrLog.Log("塔型%s-%s对应的文件读取失败!",(char*)pModel->m_sTowerNum,(char*)pModel->m_sTowerType);
		else if(pModel->m_ciErrorType==CTMDHModel::ERROR_FIND_HUGAO)
			xMyErrLog.Log("塔型%s-%s中不存在%s呼高!",(char*)pModel->m_sTowerNum,(char*)pModel->m_sTowerType,(char*)pModel->m_sHuGao);
		else if(pModel->m_ciErrorType==CTMDHModel::ERROR_FIND_LEG)
			xMyErrLog.Log("塔型%s-%s的%s呼高中配腿信息有误!",(char*)pModel->m_sTowerNum,(char*)pModel->m_sTowerType,(char*)pModel->m_sHuGao);
		else if(pModel->m_ciErrorType==CTMDHModel::ERROR_FIND_INSTANCE)
			xMyErrLog.Log("塔型%s-%s的配基信息有误!",(char*)pModel->m_sTowerNum,(char*)pModel->m_sTowerType);
	}
	return TRUE;
}
BOOL CTMDHManager::ParseTaModSheetContent(CVariant2dArray &sheetContentMap)
{
	if(sheetContentMap.RowsCount()<1)
		return FALSE;
	m_xGimDataList.Empty();
	//第一、二行记录基本信息
	GIM_HEAD_PROP_ITEM head_info;
	char sModPath[MAX_PATH]={0},sOutPath[MAX_PATH]={0};
	VARIANT value;
	sheetContentMap.GetValueAt(1,0,value);
	strcpy(sModPath,VariantToString(value));	//MOD路径
	sheetContentMap.GetValueAt(1,1,value);
	strcpy(sOutPath,VariantToString(value));	//输出路径
	sheetContentMap.GetValueAt(1,2,value);
	strcpy(head_info.m_sDesigner,VariantToString(value));//设计者
	sheetContentMap.GetValueAt(1,3,value);
	strcpy(head_info.m_sUnit,VariantToString(value));	//组织单位
	sheetContentMap.GetValueAt(1,4,value);
	strcpy(head_info.m_sSoftMajorVer,VariantToString(value));	//软件主版本
	sheetContentMap.GetValueAt(1,5,value);
	strcpy(head_info.m_sSoftMinorVer,VariantToString(value));	//软件次版本
	sheetContentMap.GetValueAt(1,6,value);
	strcpy(head_info.m_sMajorVersion,VariantToString(value));	//标准主版本
	sheetContentMap.GetValueAt(1,7,value);
	strcpy(head_info.m_sMinorVersion,VariantToString(value));	//标准次版本
	if(strlen(sModPath)<=0 || strlen(sOutPath)<=0)
		return FALSE;
	//第三行记录属性列，根据列名记录列序号
	CHashStrList<DWORD> hashColIndexByColTitle;
	for(int i=0;i<TA_MOD_EXCEL_COL_COUNT;i++)
	{
		VARIANT value;
		sheetContentMap.GetValueAt(2,i,value);
		if(CString(value.bstrVal).CompareNoCase(T_MOD_NAME1)==0) 
			hashColIndexByColTitle.SetValue(T_MOD_NAME1,i);
		else if(CString(value.bstrVal).CompareNoCase(T_MOD_NAME2)==0) 
			hashColIndexByColTitle.SetValue(T_MOD_NAME2,i);
		else if(CString(value.bstrVal).CompareNoCase(T_OUT_PATH)==0)
			hashColIndexByColTitle.SetValue(T_OUT_PATH,i);
		else if(CString(value.bstrVal).CompareNoCase(T_DESIGNER)==0)
			hashColIndexByColTitle.SetValue(T_DESIGNER,i);
		else if(CString(value.bstrVal).CompareNoCase(T_UNIT)==0)
			hashColIndexByColTitle.SetValue(T_UNIT,i);
		else if(CString(value.bstrVal).CompareNoCase(T_SOFTMAJORVER)==0)
			hashColIndexByColTitle.SetValue(T_SOFTMAJORVER,i);
		else if(CString(value.bstrVal).CompareNoCase(T_SOFTMINORVER)==0)
			hashColIndexByColTitle.SetValue(T_SOFTMINORVER,i);
		else if(CString(value.bstrVal).CompareNoCase(T_MAJORVERSION)==0)
			hashColIndexByColTitle.SetValue(T_MAJORVERSION,i);
		else if(CString(value.bstrVal).CompareNoCase(T_MINORVERSION)==0)
			hashColIndexByColTitle.SetValue(T_MINORVERSION,i);
		else if(CString(value.bstrVal).CompareNoCase(T_VOLTGRADE)==0)
			hashColIndexByColTitle.SetValue(T_VOLTGRADE,i);
		else if(CString(value.bstrVal).CompareNoCase(T_TYPE)==0)
			hashColIndexByColTitle.SetValue(T_TYPE,i);
		else if(CString(value.bstrVal).CompareNoCase(T_TEXTURE)==0)
			hashColIndexByColTitle.SetValue(T_TEXTURE,i);
		else if(CString(value.bstrVal).CompareNoCase(T_FIXEDTYPE)==0)
			hashColIndexByColTitle.SetValue(T_FIXEDTYPE,i);
		else if(CString(value.bstrVal).CompareNoCase(T_TATYPE)==0)
			hashColIndexByColTitle.SetValue(T_TATYPE,i);
		else if(CString(value.bstrVal).CompareNoCase(T_CIRCUIT)==0)
			hashColIndexByColTitle.SetValue(T_CIRCUIT,i);
		else if(CString(value.bstrVal).CompareNoCase(T_CWIRESPEC)==0)
			hashColIndexByColTitle.SetValue(T_CWIRESPEC,i);
		else if(CString(value.bstrVal).CompareNoCase(T_EWIRESPEC)==0)
			hashColIndexByColTitle.SetValue(T_EWIRESPEC,i);
		else if(CString(value.bstrVal).CompareNoCase(T_WINDSPEED)==0)
			hashColIndexByColTitle.SetValue(T_WINDSPEED,i);
		else if(CString(value.bstrVal).CompareNoCase(T_NICETHICK)==0)
			hashColIndexByColTitle.SetValue(T_NICETHICK,i);
		else if(CString(value.bstrVal).CompareNoCase(T_WINDSPAN)==0)
			hashColIndexByColTitle.SetValue(T_WINDSPAN,i);
		else if(CString(value.bstrVal).CompareNoCase(T_WEIGHTSPAN)==0)
			hashColIndexByColTitle.SetValue(T_WEIGHTSPAN,i);
		else if(CString(value.bstrVal).CompareNoCase(T_FRONTRULINGSPAN)==0)
			hashColIndexByColTitle.SetValue(T_FRONTRULINGSPAN,i);
		else if(CString(value.bstrVal).CompareNoCase(T_BACKRULINGSPAN)==0)
			hashColIndexByColTitle.SetValue(T_BACKRULINGSPAN,i);
		else if(CString(value.bstrVal).CompareNoCase(T_MAXSPAN)==0)
			hashColIndexByColTitle.SetValue(T_MAXSPAN,i);
		else if(CString(value.bstrVal).CompareNoCase(T_ANGLERANGE)==0)
			hashColIndexByColTitle.SetValue(T_ANGLERANGE,i);
		else if(CString(value.bstrVal).CompareNoCase(T_DESIGNKV)==0)
			hashColIndexByColTitle.SetValue(T_DESIGNKV,i);
		else if(CString(value.bstrVal).CompareNoCase(T_RATEDHEIGHT)==0)
			hashColIndexByColTitle.SetValue(T_RATEDHEIGHT,i);
		else if(CString(value.bstrVal).CompareNoCase(T_HEIGHTRANGE)==0)
			hashColIndexByColTitle.SetValue(T_HEIGHTRANGE,i);
		else if(CString(value.bstrVal).CompareNoCase(T_TOWERWEIGHT)==0)
			hashColIndexByColTitle.SetValue(T_TOWERWEIGHT,i);
		else if(CString(value.bstrVal).CompareNoCase(T_FREQUENCYROCKANGLE)==0)
			hashColIndexByColTitle.SetValue(T_FREQUENCYROCKANGLE,i);
		else if(CString(value.bstrVal).CompareNoCase(T_LIGHTNINGROCKANGLE)==0)
			hashColIndexByColTitle.SetValue(T_LIGHTNINGROCKANGLE,i);
		else if(CString(value.bstrVal).CompareNoCase(T_SWITCHINGROCKANGLE)==0)
			hashColIndexByColTitle.SetValue(T_SWITCHINGROCKANGLE,i);
		else if(CString(value.bstrVal).CompareNoCase(T_WORKINGROCKANGLE)==0)
			hashColIndexByColTitle.SetValue(T_WORKINGROCKANGLE,i);
		else if(CString(value.bstrVal).CompareNoCase(T_MANUFACTURER)==0)
			hashColIndexByColTitle.SetValue(T_MANUFACTURER,i);
		else if(CString(value.bstrVal).CompareNoCase(T_MATERIALCODE)==0)
			hashColIndexByColTitle.SetValue(T_MATERIALCODE,i);
		else if(CString(value.bstrVal).CompareNoCase(T_PROMODELCODE)==0)
			hashColIndexByColTitle.SetValue(T_PROMODELCODE,i);
	}
	//
	DWORD *pColIndex=NULL;
	CTMDHGim* pGimInfo=NULL;
	DisplayProcess(0,"读取批量打包Exel文件,生成模型进度");
	int nRows=sheetContentMap.RowsCount();
	for(int i=3;i<=nRows;i++)
	{
		DisplayProcess(ftoi(100*i/nRows),"读取批量打包Exel文件,生成模型进度");
		VARIANT value;
		//MOD文件名称
		pColIndex=hashColIndexByColTitle.GetValue(T_MOD_NAME1);
		if(pColIndex==NULL)
			pColIndex=hashColIndexByColTitle.GetValue(T_MOD_NAME2);
		if(pColIndex==NULL)
		{
			logerr.Log("读取的Excle文件与模板文件不一致!");
			return FALSE;
		}
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		if(value.vt==VT_EMPTY)
			continue;
		CXhChar50 sModeName;
		_splitpath(VariantToString(value),NULL,NULL,sModeName,NULL);
		//
		pGimInfo=m_xGimDataList.append();
		pGimInfo->m_sModName=sModeName;
		pGimInfo->m_sGimName=sModeName;
		pGimInfo->m_sModPath.Copy(sModPath);
		pGimInfo->m_sOutputPath.Copy(sOutPath);
		pGimInfo->m_sModFile.Printf("%s\\%s.mod",(char*)sModPath,(char*)sModeName);
		if(IsUTF8File(pGimInfo->m_sModFile))
			pGimInfo->m_ciCodingType=CTMDHGim::UTF8_CODING;
		else
			pGimInfo->m_ciCodingType=CTMDHGim::ANSI_CODING;
		if(m_pModModel==NULL)
			m_pModModel=CModModelFactory::CreateModModel();
		//GIM头信息
		strcpy(pGimInfo->m_xGimHeadProp.m_sFileName,pGimInfo->m_sModName);
		strcpy(pGimInfo->m_xGimHeadProp.m_sDesigner,head_info.m_sDesigner);
		strcpy(pGimInfo->m_xGimHeadProp.m_sUnit,head_info.m_sUnit);
		strcpy(pGimInfo->m_xGimHeadProp.m_sSoftMajorVer,head_info.m_sSoftMajorVer);
		strcpy(pGimInfo->m_xGimHeadProp.m_sSoftMinorVer,head_info.m_sSoftMinorVer);
		strcpy(pGimInfo->m_xGimHeadProp.m_sMajorVersion,head_info.m_sMajorVersion);
		strcpy(pGimInfo->m_xGimHeadProp.m_sMinorVersion,head_info.m_sMinorVersion);
		//电压等级
		pColIndex=hashColIndexByColTitle.GetValue(T_VOLTGRADE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_sVoltGrade=VariantToString(value);
		//型号
		pColIndex=hashColIndexByColTitle.GetValue(T_TYPE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_sType=VariantToString(value);
		//杆塔材质
		pColIndex=hashColIndexByColTitle.GetValue(T_TEXTURE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_sTexture=VariantToString(value);
		//固定方式
		pColIndex=hashColIndexByColTitle.GetValue(T_FIXEDTYPE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_sFixedType=VariantToString(value);
		//杆塔类型
		pColIndex=hashColIndexByColTitle.GetValue(T_TATYPE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_sTaType=VariantToString(value);
		//回路数
		pColIndex=hashColIndexByColTitle.GetValue(T_CIRCUIT);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_nCircuit=atoi(VariantToString(value));
		//导线型号
		pColIndex=hashColIndexByColTitle.GetValue(T_CWIRESPEC);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_sCWireSpec=VariantToString(value);
		//地线型号
		pColIndex=hashColIndexByColTitle.GetValue(T_EWIRESPEC);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_sEWireSpec=VariantToString(value);
		//设计基本风速
		pColIndex=hashColIndexByColTitle.GetValue(T_WINDSPEED);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_fWindSpeed=atof(VariantToString(value));
		//设计覆冰厚度
		pColIndex=hashColIndexByColTitle.GetValue(T_NICETHICK);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_fNiceThick=atof(VariantToString(value));
		//设计水平档距
		pColIndex=hashColIndexByColTitle.GetValue(T_WINDSPAN);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_sWindSpan=VariantToString(value);
		//设计垂直档距
		pColIndex=hashColIndexByColTitle.GetValue(T_WEIGHTSPAN);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_sWeightSpan=VariantToString(value);
		//设计前侧代表档距
		pColIndex=hashColIndexByColTitle.GetValue(T_FRONTRULINGSPAN);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_fFrontRulingSpan=atof(VariantToString(value));
		//设计后侧代表档距
		pColIndex=hashColIndexByColTitle.GetValue(T_BACKRULINGSPAN);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_fBackRulingSpan=atof(VariantToString(value));
		//设计最大档距
		pColIndex=hashColIndexByColTitle.GetValue(T_MAXSPAN);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_fMaxSpan=atof(VariantToString(value));
		//设计转角范围
		pColIndex=hashColIndexByColTitle.GetValue(T_ANGLERANGE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_sAngleRange=VariantToString(value);
		//设计Kv值
		pColIndex=hashColIndexByColTitle.GetValue(T_DESIGNKV);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_fDesignKV=atof(VariantToString(value));
		//计算呼高
		pColIndex=hashColIndexByColTitle.GetValue(T_RATEDHEIGHT);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_sRatedHeight=VariantToString(value);
		//呼高范围
		pColIndex=hashColIndexByColTitle.GetValue(T_HEIGHTRANGE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_sHeightRange=VariantToString(value);
		//塔重
		pColIndex=hashColIndexByColTitle.GetValue(T_TOWERWEIGHT);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_sTowerWeight=VariantToString(value);
		//设计工频摇摆角度
		pColIndex=hashColIndexByColTitle.GetValue(T_FREQUENCYROCKANGLE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_fFrequencyRockAngle=atof(VariantToString(value));
		//设计雷电摇摆角度
		pColIndex=hashColIndexByColTitle.GetValue(T_LIGHTNINGROCKANGLE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_fLightningRockAngle=atof(VariantToString(value));
		//设计操作摇摆角度
		pColIndex=hashColIndexByColTitle.GetValue(T_SWITCHINGROCKANGLE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_fSwitchingRockAngle=atof(VariantToString(value));
		//设计带电作业摇摆角度
		pColIndex=hashColIndexByColTitle.GetValue(T_WORKINGROCKANGLE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_fWorkingRockAngle=atof(VariantToString(value));
		//生产厂家
		pColIndex=hashColIndexByColTitle.GetValue(T_MANUFACTURER);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_sManuFacturer=VariantToString(value);
		//物资编码
		pColIndex=hashColIndexByColTitle.GetValue(T_MATERIALCODE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_sMaterialCode=VariantToString(value);
		//装配模型编号
		pColIndex=hashColIndexByColTitle.GetValue(T_PROMODELCODE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_sProModelCode=VariantToString(value);
	}
	DisplayProcess(100,"读取批量打包Exel文件,生成模型进度");
	return TRUE;
}
BOOL CTMDHManager::ReadTaModBatchGzipFile(const char* sFilePath)
{
	CExcelOperObject xExcelObj;
	if(!xExcelObj.OpenExcelFile(sFilePath))
		return FALSE;
	LPDISPATCH pWorksheets=xExcelObj.GetWorksheets();
	ASSERT(pWorksheets != NULL);
	Sheets       excel_sheets;	//工作表集合
	excel_sheets.AttachDispatch(pWorksheets);
	int nSheetNum=excel_sheets.GetCount();
	if(nSheetNum<1)
	{
		excel_sheets.ReleaseDispatch();
		return FALSE;
	}
	LPDISPATCH pWorksheet=excel_sheets.GetItem(COleVariant((short) 1));
	_Worksheet  excel_sheet;
	excel_sheet.AttachDispatch(pWorksheet);
	excel_sheet.Select();
	//1.获取Excel指定Sheet内容存储至sheetContentMap
	Range excel_usedRange,excel_range;
	excel_usedRange.AttachDispatch(excel_sheet.GetUsedRange());
	excel_range.AttachDispatch(excel_usedRange.GetRows());
	long nRowNum = excel_range.GetCount();
	excel_range.AttachDispatch(excel_usedRange.GetColumns());
	long nColNum = excel_range.GetCount();
	if(nColNum>=TA_MOD_EXCEL_COL_COUNT)
	{
		CXhChar50 sCellS("A1"),sCellE=CExcelOper::GetCellPos(nColNum,nRowNum);
		LPDISPATCH pRange = excel_sheet.GetRange(COleVariant(sCellS),COleVariant(sCellE));
		excel_range.AttachDispatch(pRange,FALSE);
		CVariant2dArray sheetContentMap(1,1);
		sheetContentMap.var=excel_range.GetValue();
		excel_range.ReleaseDispatch();
		//2、解析数据
		if(!ParseTaModSheetContent(sheetContentMap))
		{
			excel_sheet.ReleaseDispatch();
			return FALSE;
		}
		excel_sheet.ReleaseDispatch();
	}
	excel_sheets.ReleaseDispatch();
	return TRUE;
}
