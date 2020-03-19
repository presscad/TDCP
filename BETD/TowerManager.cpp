#include "stdafx.h"
#include "ArrayList.h"
#include "Variant.h"
#include "ExcelOper.h"
#include "XmlData.h"
#include "ProcBarDlg.h"
#include "DxfFile.h"
#include "ProcBarDlg.h"
#include "LogFile.h"
#include "TowerManager.h"
#include "XhMath.h"

CTowerManager TaManager;
CLogFile xMyErrLog("D:\\warning.log");
//////////////////////////////////////////////////////////////////////////
//Exce标题
static const int LOFTING_EXCEL_COL_COUNT  =12;
static const char* T_NO							= "序号";
static const char* T_TOWER_NUM					= "塔位号";
static const char* T_TOWER_DIAN					= "塔位点";
static const char* T_TOWER_TYPE					= "塔型";
static const char* T_LOCATE_HIGH				= "定位高";
static const char* T_WIRE_ANGLE					= "线路转角";
static const char* T_HUGAO_HEIGHT				= "呼称高";
static const char* T_BODY_HEIGHTP				= "接身高";
static const char* T_LEG_LENGTH					= "接腿长度";
CXhChar100 VariantToString(VARIANT value)
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
GEPOINT TransPoint_m_To_mm(GEPOINT srcPt)
{
	GEPOINT dstPt;
	dstPt.x=srcPt.x*1000;
	dstPt.y=srcPt.y*1000;
	dstPt.z=srcPt.z*1000;
	return dstPt;
}
//////////////////////////////////////////////////////////////////////////
//CTowerModle
CTowerModel::CTowerModel()
{
	m_bTurnLeft=FALSE;
	m_ciErrorType=0;
	m_fMaxLegHeight=0;
	m_uiHeightSerial=1;
	for(int i=0;i<4;i++)
		m_uiLegSerialArr[i]=1;
}
CTowerModel::~CTowerModel()
{
}
//杆塔配基中要求左转时，需要将杆塔模型水平旋转180°
//即杆塔模型坐标系中的X轴和Y轴*-1
GECS CTowerModel::BuildRotateCS()
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
GECS CTowerModel::BuildTransCS(void* pInstance/*=NULL*/)
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
/*配腿与象限的对照关系
   象限		  正常模型中配腿	左转模型中配腿
Serial[0]		LegD				LegB
Serial[1]		LegA				LegC
Serial[2]		LegC				LegA
Serial[3]		LegB				LegD
*/
BOOL CTowerModel::ExtractActiveTaInstance(char* sHuGao,double fLegA,double fLegB,double fLegC,double fLegD)
{
	if(m_ciModelFlag==TYPE_TID)
	{
		ITidModel* pTidModel=BelongManager()->m_pTidModel;
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
			m_uiHeightSerial=0;
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
			CXhChar50 szError("接腿配置错误:"),szErrLeg;
			if(iLegASerial==0)
				szErrLeg.Printf("%gm",fLegA);
			if(iLegBSerial==0&&fLegB!=fLegA)
				szErrLeg.Append(CXhChar16("%gm",fLegB),',');
			if(iLegCSerial==0&&fLegC!=fLegB&&fLegC!=fLegA)
				szErrLeg.Append(CXhChar16("%gm",fLegC),',');
			if(iLegDSerial==0&&fLegD!=fLegC&&fLegD!=fLegB&&fLegD!=fLegA)
				szErrLeg.Append(CXhChar16("%gm",fLegD),',');
			logerr.Log("接腿配置错误:%s",(char*)szErrLeg);
			m_uiLegSerialArr[0]=m_uiLegSerialArr[1]=m_uiLegSerialArr[2]=m_uiLegSerialArr[3]=1;
			m_ciErrorType=ERROR_FIND_LEG;
			return FALSE;
		}
		//
		ITidTowerInstance* pInstance=NULL;
		pInstance=pHeightGroup->GetTowerInstance(m_uiLegSerialArr[0],m_uiLegSerialArr[1],m_uiLegSerialArr[2],m_uiLegSerialArr[3]);
		if(pInstance==NULL)
		{
			m_uiLegSerialArr[0]=m_uiLegSerialArr[1]=m_uiLegSerialArr[2]=m_uiLegSerialArr[3]=1;
			m_ciErrorType=ERROR_FIND_INSTANCE;
			return FALSE;
		}
	}
	else if(m_ciModelFlag==TYPE_MOD)
	{
		IModModel* pModModel=BelongManager()->m_pModModel;
		if(pModModel==NULL || !pModModel->ImportModFile(m_sFilePath))
			return FALSE;
		IModHeightGroup* pHeightGroup=pModModel->GetHeightGroup(sHuGao);
		if(pHeightGroup==NULL)
		{
			m_uiHeightSerial=0;
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
			CXhChar50 szError("接腿配置错误:"),szErrLeg;
			if(pLegASerial==0)
				szErrLeg.Printf("%gm",fLegA);
			if(pLegBSerial==0&&fLegB!=fLegA)
				szErrLeg.Append(CXhChar16("%gm",fLegB),',');
			if(pLegCSerial==0&&fLegC!=fLegB&&fLegC!=fLegA)
				szErrLeg.Append(CXhChar16("%gm",fLegC),',');
			if(pLegDSerial==0&&fLegD!=fLegC&&fLegD!=fLegB&&fLegD!=fLegA)
				szErrLeg.Append(CXhChar16("%gm",fLegD),',');
			logerr.Log("接腿配置错误:%s",(char*)szErrLeg);
			m_uiLegSerialArr[0]=m_uiLegSerialArr[1]=m_uiLegSerialArr[2]=m_uiLegSerialArr[3]=1;
			m_ciErrorType=ERROR_FIND_LEG;
			return FALSE;
		}
		IModTowerInstance* pInstance=NULL;
		pInstance=pHeightGroup->GetTowerInstance(m_uiLegSerialArr[0],m_uiLegSerialArr[1],m_uiLegSerialArr[2],m_uiLegSerialArr[3]);
		if(pInstance==NULL)
		{
			m_uiLegSerialArr[0]=m_uiLegSerialArr[1]=m_uiLegSerialArr[2]=m_uiLegSerialArr[3]=1;
			m_ciErrorType=ERROR_FIND_INSTANCE;
			return FALSE;
		}
	}
	return TRUE;
}
void* CTowerModel::ExtractActiveTaInstance()
{
	if(m_ciModelFlag==TYPE_TID)
	{
		ITidModel* pTidModel=BelongManager()->m_pTidModel;
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
		IModModel* pModModel=BelongManager()->m_pModModel;
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
CXhChar100 CTowerModel::GetFileName(void* pInstance)
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
	CXhChar100 sFileName=m_sTowerPlaceCode;
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
		//
		double dfActualHeight=dfNamedHeight+fMaxSubtractLegH;
		sFileName.Append(CXhChar50("%s-%gm-LOD%d",(char*)m_sTowerType,dfActualHeight,BelongManager()->m_ciInstanceLevel),' ');
	}
	else
		sFileName.Append(CXhChar50("%s-error-LOD%d",(char*)m_sTowerType,BelongManager()->m_ciInstanceLevel),' ');
	return sFileName;
}
void CTowerModel::CreateXmlFile(ITidTowerInstance* pInstance,const char* sFileName)
{
	ITidHeightGroup* pHeightGroup=pInstance->BelongHeightGroup();
	if(pHeightGroup==NULL)
		return;
	CXhChar50 sName;
	pHeightGroup->GetName(sName,50);
	GECS rot_cs=BuildRotateCS();
	GECS trans_cs=BuildTransCS(pInstance);
	//初始化XML模型数据
	CXmlModel xXmlModel(BelongManager()->m_bUseUnitM);
	xXmlModel.m_sCategory.Copy("铁塔");
	xXmlModel.m_sTowerName=GetFileName(pInstance);
	if(BelongManager()->m_ciInstanceLevel==0)
		xXmlModel.m_sTowerFile.Printf("%s.dxf",(char*)xXmlModel.m_sTowerName);
	else
		xXmlModel.m_sTowerFile.Printf("%s.3ds",(char*)xXmlModel.m_sTowerName);
	xXmlModel.m_ciModel=0;
	xXmlModel.m_sPropName=sName;
	GEPOINT xBaseLocPt[4], xLocation;	//按照象限获取基础坐标
	for(int i=0;i<4;i++)
	{
		int iHeightGroupSeial=pHeightGroup->GetSerialId();
		BYTE ciLegSerial=(BYTE)pInstance->GetLegSerialIdByQuad(i+1);
		BelongManager()->m_pTidModel->GetSubLegBaseLocation(iHeightGroupSeial,ciLegSerial,xLocation);
		if(i==1||i==3)	 //相对1象限Y轴对称
			xLocation.x*=-1.0;
		if(i==2||i==3)	//相对1象限X轴对称
			xLocation.y*=-1.0;
		if(m_bTurnLeft)
			xLocation=rot_cs.TransPToCS(xLocation);
		xLocation=trans_cs.TransPToCS(xLocation);
		xBaseLocPt[i] = xLocation;
	}
	//将象限顺序坐标调整为配腿顺序坐标
	//A腿基础坐标
	xXmlModel.m_xFundation[0].m_xFeetPos = xBaseLocPt[1];
	if (m_bTurnLeft)
		xXmlModel.m_xFundation[0].m_xFeetPos = xBaseLocPt[2];
	//B腿基础
	xXmlModel.m_xFundation[1].m_xFeetPos = xBaseLocPt[3];
	if (m_bTurnLeft)
		xXmlModel.m_xFundation[1].m_xFeetPos = xBaseLocPt[0];
	//C腿基础
	xXmlModel.m_xFundation[2].m_xFeetPos = xBaseLocPt[2];
	if (m_bTurnLeft)
		xXmlModel.m_xFundation[2].m_xFeetPos = xBaseLocPt[1];
	//D腿基础
	xXmlModel.m_xFundation[3].m_xFeetPos = xBaseLocPt[0];
	if (m_bTurnLeft)
		xXmlModel.m_xFundation[3].m_xFeetPos = xBaseLocPt[3];
	//初始化挂线信息
	int nHangPt=BelongManager()->m_pTidModel->HangPointCount();
	for(int i=0;i<nHangPt;i++)
	{	//导线
		ITidHangPoint* pTidHangPt=BelongManager()->m_pTidModel->GetHangPointAt(i);
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
		if (pTidHangPt->GetWireType() == 'J')
		{	//跳线单独存放
			HANG_POINT* pHangPt = pHangPhase->m_xWireJHangPtArr.append();
			pHangPt->m_ciHangingStyle = pTidHangPt->GetHangingStyle();
			pHangPt->m_ciWireType = 'J';
			pHangPt->m_xHangPos = pos;
		}
		else
		{	//处理导线
			if (ciTensionType == 1)
			{	//悬垂,挂点不分前后,每相线只有一个虚拟挂点
				HANG_POINT* pHangPt = pHangPhase->m_xBackHangPtArr.append();
				pHangPt->m_ciHangingStyle = pTidHangPt->GetHangingStyle();
				pHangPt->m_ciWireType = 'C';
				if (strstr(sDes, "前"))
					pHangPt->m_ciHangDirect = 'Q';
				else
					pHangPt->m_ciHangDirect = 'H';
				pHangPt->m_xHangPos = pos;
				pHangPt->m_ciTensionType = 0;
			}
			else
			{	//耐张，挂点区分前后侧，每相线有两个虚拟挂点
				if (strstr(sDes, "前"))
				{
					HANG_POINT* pHangPt = pHangPhase->m_xFrontHangPtArr.append();
					pHangPt->m_ciHangingStyle = pTidHangPt->GetHangingStyle();
					pHangPt->m_ciWireType = 'C';
					pHangPt->m_ciHangDirect = 'Q';
					pHangPt->m_xHangPos = pos;
					pHangPt->m_ciTensionType = 1;
				}
				else
				{
					HANG_POINT* pHangPt = pHangPhase->m_xBackHangPtArr.append();
					pHangPt->m_ciHangingStyle = pTidHangPt->GetHangingStyle();
					pHangPt->m_ciWireType = 'C';
					pHangPt->m_ciHangDirect = 'H';
					pHangPt->m_xHangPos = pos;
					pHangPt->m_ciTensionType = 1;
				}
			}
		}
	}
	for(int i=0;i<nHangPt;i++)
	{	//地线
		ITidHangPoint* pTidHangPt=BelongManager()->m_pTidModel->GetHangPointAt(i);
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
		if (ciTensionType == 1)
		{
			HANG_POINT* pHangPt = pEarthLoop->m_xBackHangPtList.append();
			pHangPt->m_ciHangingStyle = pTidHangPt->GetHangingStyle();
			pHangPt->m_ciWireType = 'E';
			pHangPt->m_xHangPos = pos;
			pHangPt->m_ciTensionType = (ciTensionType == 1) ? 0 : 1;
			if (strstr(sDes, "前"))
				pHangPt->m_ciHangDirect = 'Q';
			else if (strstr(sDes, "后"))
				pHangPt->m_ciHangDirect = 'H';
		}
		else
		{
			if (strstr(sDes, "前"))
			{
				HANG_POINT* pHangPt = pEarthLoop->m_xFrontHangPtList.append();
				pHangPt->m_ciHangingStyle = pTidHangPt->GetHangingStyle();
				pHangPt->m_ciWireType = 'E';
				pHangPt->m_ciHangDirect = 'Q';
				pHangPt->m_xHangPos = pos;
				pHangPt->m_ciTensionType = 1;
			}
			else
			{
				HANG_POINT* pHangPt = pEarthLoop->m_xBackHangPtList.append();
				pHangPt->m_ciHangingStyle = pTidHangPt->GetHangingStyle();
				pHangPt->m_ciWireType = 'E';
				pHangPt->m_ciHangDirect = 'H';
				pHangPt->m_xHangPos = pos;
				pHangPt->m_ciTensionType = 1;
			}
		}
	}
	//生成XML文件
	xXmlModel.AmendHangInfo(m_bTurnLeft);
	xXmlModel.CreateOldXmlFile(sFileName);
}
void CTowerModel::CreateXmlFile(IModTowerInstance* pInstance,const char* sFileName)
{
	IModHeightGroup* pHeightGroup=pInstance->BelongHuGao();
	if(pHeightGroup==NULL)
		return;
	GECS rot_cs=BuildRotateCS();
	CXmlModel xXmlModel(BelongManager()->m_bUseUnitM);
	xXmlModel.m_sCategory.Copy("铁塔");
	xXmlModel.m_sTowerName=GetFileName(pInstance);
	if(BelongManager()->m_ciInstanceLevel==0)
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
	int nHangPt=BelongManager()->m_pModModel->GetHangNodeNum();
	for(int i=0;i<nHangPt;i++)
	{	//导线
		MOD_HANG_NODE* pModHangPt=BelongManager()->m_pModModel->GetHangNodeById(i);
		if(pModHangPt->m_ciWireType=='E')
			continue;
		if(strstr(pModHangPt->m_sHangName,"前") && ciHangType==0)
			ciHangType=1;
	}
	for(int i=0;i<nHangPt;i++)
	{
		//导线
		MOD_HANG_NODE* pModHangPt=BelongManager()->m_pModModel->GetHangNodeById(i);
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
		if (ciHangType == 0)
		{	//悬垂,挂点不分前后,每相线只有一个虚拟挂点
			HANG_POINT* pHangPt = pHangPhase->m_xBackHangPtArr.append();
			pHangPt->m_ciHangingStyle = pModHangPt->m_ciHangingStyle;
			pHangPt->m_ciWireType = 'C';
			if (strstr(pModHangPt->m_sHangName, "前"))
				pHangPt->m_ciHangDirect = 'Q';
			else
				pHangPt->m_ciHangDirect = 'H';
			pHangPt->m_xHangPos = pos;
			pHangPt->m_ciTensionType = 0;
		}
		else
		{	//耐张，挂点区分前后侧，每相线有两个虚拟挂点
			if (strstr(pModHangPt->m_sHangName, "前"))
			{
				HANG_POINT* pHangPt = pHangPhase->m_xFrontHangPtArr.append();
				pHangPt->m_ciHangingStyle = pModHangPt->m_ciHangingStyle;
				pHangPt->m_ciWireType = 'C';
				pHangPt->m_ciHangDirect = 'Q';
				pHangPt->m_xHangPos = pos;
				pHangPt->m_ciTensionType = ciHangType;
			}
			else
			{
				HANG_POINT* pHangPt = pHangPhase->m_xBackHangPtArr.append();
				pHangPt->m_ciHangingStyle = pModHangPt->m_ciHangingStyle;
				pHangPt->m_ciWireType = 'C';
				pHangPt->m_ciHangDirect = 'H';
				pHangPt->m_xHangPos = pos;
				pHangPt->m_ciTensionType = ciHangType;
			}
		}
	}
	for(int i=0;i<nHangPt;i++)
	{	//地线
		MOD_HANG_NODE* pModHangPt=BelongManager()->m_pModModel->GetHangNodeById(i);
		if(pModHangPt->m_ciWireType!='E')
			continue;
		int iSerial=pModHangPt->m_ciPhaseSerial;
		GEPOINT pos=pModHangPt->m_xHangPos;
		if(m_bTurnLeft)
			pos=rot_cs.TransPToCS(pos);
		//
		CEarthLoop* pEarthLoop=xXmlModel.m_hashEarthLoop.GetValue(iSerial);
		if(pEarthLoop==NULL)
			pEarthLoop=xXmlModel.m_hashEarthLoop.Add(iSerial);
		if (ciHangType == 0)
		{
			HANG_POINT* pHangPt = pEarthLoop->m_xBackHangPtList.append();
			pHangPt->m_ciHangingStyle = pModHangPt->m_ciHangingStyle;
			pHangPt->m_ciWireType = 'E';
			pHangPt->m_xHangPos = pos;
			pHangPt->m_ciTensionType = 0;
			if (strstr(pModHangPt->m_sHangName, "前"))
				pHangPt->m_ciHangDirect = 'Q';
			else //if (strstr(pModHangPt->m_sHangName, "后"))
				pHangPt->m_ciHangDirect = 'H';
		}
		else
		{
			if (strstr(pModHangPt->m_sHangName, "前"))
			{
				HANG_POINT* pHangPt = pEarthLoop->m_xFrontHangPtList.append();
				pHangPt->m_ciHangingStyle = pModHangPt->m_ciHangingStyle;
				pHangPt->m_ciWireType = 'E';
				pHangPt->m_ciHangDirect = 'Q';
				pHangPt->m_xHangPos = pos;
				pHangPt->m_ciTensionType = 1;
			}
			else
			{
				HANG_POINT* pHangPt = pEarthLoop->m_xBackHangPtList.append();
				pHangPt->m_ciHangingStyle = pModHangPt->m_ciHangingStyle;
				pHangPt->m_ciWireType = 'E';
				pHangPt->m_ciHangDirect = 'H';
				pHangPt->m_xHangPos = pos;
				pHangPt->m_ciTensionType = 1;
			}
		}
	}
	//生成XML文件
	xXmlModel.AmendHangInfo(m_bTurnLeft);
	xXmlModel.CreateOldXmlFile(sFileName);
}
void CTowerModel::CreateModFile(ITidTowerInstance* pInstance,const char* sFileName)
{
	pInstance->ExportModFile(sFileName);
}
void CTowerModel::CreateModFile(IModTowerInstance* pInstance,const char* sFileName)
{
	pInstance->ExportModFile(sFileName);
}

void CTowerModel::Create3dsFile(ITidTowerInstance* pInstance,const char* sFileName)
{
	BYTE ciLevel=BelongManager()->m_ciInstanceLevel;
	UCS_STRU rot_cs=BuildRotateCS();
	UCS_STRU trans_cs=BuildTransCS(pInstance);
	//解析三维实体的基本三角面信息
	int nPart=pInstance->GetAssemblePartNum();
	int nBolt=pInstance->GetAssembleBoltNum();
	int nAnchorBolt=pInstance->GetAssembleAnchorBoltNum();
	int nSum=(ciLevel==3)?nPart+nBolt+nAnchorBolt:nPart;
	int serial=1;
	I3DSData* p3dsFile=C3DSFactory::Create3DSInstance();
	DisplayProcess(0,"生成3DS模型....");
	for(ITidAssemblePart* pAssmPart=pInstance->EnumAssemblePartFirst();pAssmPart;pAssmPart=pInstance->EnumAssemblePartNext(),serial++)
	{
		DisplayProcess(int(100*serial/nSum),"生成构件3DS模型....");
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
			p3dsFile->AddSolidPart(&solidBody,serial,CXhChar16("Part%d",serial),BelongManager()->m_bUseUnitM);
		else if(pTidPart->GetPartType()==ITidPart::TYPE_PLATE&&ciLevel>1)
			p3dsFile->AddSolidPart(&solidBody,serial,CXhChar16("Part%d",serial),BelongManager()->m_bUseUnitM);
	}
	if(ciLevel==3)
	{	//放样级别，绘制螺栓
		int iBolt=0;
		for(ITidAssembleBolt* pAssmBolt=pInstance->EnumAssembleBoltFirst();pAssmBolt;pAssmBolt=pInstance->EnumAssembleBoltNext(),serial++,iBolt++)
		{
			DisplayProcess(int(100*serial/nSum),"生成螺栓3DS模型....");
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
			p3dsFile->AddSolidPart(&solidBody,serial,CXhChar16("Bolt%d",iBolt),BelongManager()->m_bUseUnitM);
		}
		/*iBolt=0;
		for(ITidAssembleAnchorBolt* pAssmAnchorBolt=pInstance->EnumFirstAnchorBolt();pAssmAnchorBolt;pAssmAnchorBolt=pInstance->EnumNextAnchorBolt(),serial++,iBolt++)
		{
			DisplayProcess(int(100*serial/nSum),"生成脚钉3DS模型....");
			ITidSolidBody* pBoltSolid=pAssmAnchorBolt->GetBoltSolid();
			ITidSolidBody* pNutSolid=pAssmAnchorBolt->GetNutSolid();
			if(pBoltSolid==NULL||pNutSolid==NULL)
				continue;
			CSolidBody solidBody;
			solidBody.CopyBuffer(pBoltSolid->SolidBufferPtr(),pBoltSolid->SolidBufferLength());
			solidBody.MergeBodyBuffer(pNutSolid->SolidBufferPtr(),pNutSolid->SolidBufferLength());
			if(m_bLeftRot)
				solidBody.TransToACS(rot_cs);
			solidBody.TransToACS(trans_ucs);
			p3dsFile->Init3dsData(&solidBody,serial,CXhChar16("AnchorBolt%d",iBolt),BelongManager()->m_bUseUnitM);
		}*/
	}
	DisplayProcess(100,"生成3DS模型....");
	p3dsFile->Creat3DSFile(sFileName);
	//
	C3DSFactory::Destroy(p3dsFile->GetSerial());
	p3dsFile=NULL;
}
void CTowerModel::Create3dsFile(IModTowerInstance* pInstance,const char* sFileName)
{
	//解析三维实体的基本三角面信息
	UCS_STRU rot_cs=BuildRotateCS();
	I3DSData* p3dsFile=C3DSFactory::Create3DSInstance();
	int nIndex=1,nSum=pInstance->GetModRodNum();
	DisplayProcess(0,"生成3DS文件....");
	for(IModRod* pRod=pInstance->EnumModRodFir();pRod;pRod=pInstance->EnumModRodNext(),nIndex++)
	{
		DisplayProcess(int(100*nIndex/nSum),"生成3DS文件....");
		CSolidBody solidBody;
		pRod->Create3dSolidModel(&solidBody,FALSE);
		if(m_bTurnLeft)
			solidBody.TransToACS(rot_cs);
		p3dsFile->AddSolidPart(&solidBody,nIndex,CXhChar16("Rod%d",nIndex),BelongManager()->m_bUseUnitM);
	}
	DisplayProcess(100,"生成3DS文件....");
	p3dsFile->Creat3DSFile(sFileName);
	//
	C3DSFactory::Destroy(p3dsFile->GetSerial());
	p3dsFile=NULL;
}
void CTowerModel::CreateDxfFile(ITidTowerInstance* pInstance,const char* sFileName)
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
		if(BelongManager()->m_bUseUnitM)
			ptS*=0.001;
		scope.VerifyVertex(ptS);
		f3dPoint ptE(line_E.x,line_E.y,line_E.z);
		if(m_bTurnLeft)
			ptE=rot_cs.TransPToCS(ptE);
		ptE=trans_cs.TransPToCS(ptE);
		if(BelongManager()->m_bUseUnitM)
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
void CTowerModel::CreateDxfFile(IModTowerInstance* pInstance,const char* sFileName)
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
		if(BelongManager()->m_bUseUnitM)
			ptS*=0.001;
		scope.VerifyVertex(ptS);
		f3dPoint ptE=pRod->LinePtE();
		if(m_bTurnLeft)
			ptE=rot_cs.TransPToCS(ptE);
		if(BelongManager()->m_bUseUnitM)
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
//////////////////////////////////////////////////////////////////////////
//CTowerManager
CTowerManager::CTowerManager()
{
	m_pActiveTa=NULL;
	m_pTidModel=NULL;
	m_pModModel=NULL;
	m_ciInstanceLevel=0;
}
CTowerManager::~CTowerManager()
{
	if(m_pTidModel)
		CTidModelFactory::Destroy(m_pTidModel->GetSerialId());
	m_pTidModel=NULL;
	//
	if(m_pModModel)
		CModModelFactory::Destroy(m_pModModel->GetSerialId());
	m_pModModel=NULL;
}
void CTowerManager::Empty()
{
	m_pActiveTa=NULL;
	m_pTidModel=NULL;
	m_pModModel=NULL;
	m_xTaModelList.Empty();
}
BOOL CTowerManager::ReadTidFile(const char* sFileName)
{
	if(m_pTidModel==NULL)
		m_pTidModel=CTidModelFactory::CreateTidModel();
	CXhChar50 sTowerName;
	_splitpath(sFileName,NULL,NULL,sTowerName,NULL);
	m_pActiveTa=m_xTaModelList.append();
	m_pActiveTa->SetManager(this);
	m_pActiveTa->m_sTowerType=sTowerName;
	m_pActiveTa->m_ciModelFlag=CTowerModel::TYPE_TID;
	m_pActiveTa->m_sFilePath.Copy(sFileName);
	if(!m_pTidModel->ReadTidFile(sFileName))
	{
		m_pActiveTa->m_ciErrorType=CTowerModel::ERROR_READ_FILE;
		return FALSE;
	}
	return TRUE;
}
BOOL CTowerManager::ReadModFile(const char* sFileName)
{
	if(m_pModModel==NULL)
		m_pModModel=CModModelFactory::CreateModModel();
	CXhChar50 sTowerName;
	_splitpath(sFileName,NULL,NULL,sTowerName,NULL);
	m_pActiveTa=m_xTaModelList.append();
	m_pActiveTa->SetManager(this);
	m_pActiveTa->m_sTowerType=sTowerName;
	m_pActiveTa->m_ciModelFlag=CTowerModel::TYPE_MOD;
	m_pActiveTa->m_sFilePath.Copy(sFileName);
	if(!m_pModModel->ImportModFile(sFileName))
	{
		m_pActiveTa->m_ciErrorType=CTowerModel::ERROR_READ_FILE;
		return FALSE;
	}
	return TRUE;
}
BOOL CTowerManager::ReadExcleFile(const char* sXlsFileName)
{
	CExcelOperObject xExcelObj;
	if(!xExcelObj.OpenExcelFile(sXlsFileName))
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
	DisplayProcess(0,"读取Exel文件,生成模型进度");
	int nRows=sheet1ContentMap.RowsCount();
	for(int i=1;i<=nRows;i++)
	{	
		DisplayProcess(int(100*i/nRows),"读取Exel文件,生成模型进度");
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
		m_pActiveTa=m_xTaModelList.append();
		m_pActiveTa->SetManager(this);
		m_pActiveTa->m_sTowerPlaceCode.Copy(sTowerNum);
		m_pActiveTa->m_sTowerDot.Copy(sTowerDian);
		m_pActiveTa->m_sTowerType.Copy(sTowerType);
		m_pActiveTa->m_sHuGao.Copy(sHuGao);
		m_pActiveTa->m_fLocateH=atof(sLocateH);	//定位高
		m_pActiveTa->m_fBodyHeight=fBodyHeight;	//接身高
		if(strstr(sWireAngle,"左"))
			m_pActiveTa->m_bTurnLeft=TRUE;
		m_pActiveTa->m_fMaxLegHeight=fHuGao-fBodyHeight;
		CXhChar100* pFullFilePath=hashModelFile.GetValue(sTowerType);
		if(pFullFilePath==NULL)
		{	//找不到塔型对应的文件
			m_pActiveTa->m_ciErrorType=CTowerModel::ERROR_NO_FILE;
			continue;
		}
		CXhChar16 extension;
		m_pActiveTa->m_sFilePath.Copy(*pFullFilePath);
		_splitpath(m_pActiveTa->m_sFilePath,NULL,NULL,NULL,extension);
		if(extension.EqualNoCase(".tid"))
		{
			m_pActiveTa->m_ciModelFlag=CTowerModel::TYPE_TID;
			if(m_pTidModel==NULL)
				m_pTidModel=CTidModelFactory::CreateTidModel();
			if(!m_pTidModel->ReadTidFile(m_pActiveTa->m_sFilePath))
			{
				m_pActiveTa->m_ciErrorType=CTowerModel::ERROR_READ_FILE;
				continue;
			}
		}
		else if(extension.EqualNoCase(".mod"))
		{	
			m_pActiveTa->m_ciModelFlag=CTowerModel::TYPE_MOD;
			if(m_pModModel==NULL)
				m_pModModel=CModModelFactory::CreateModModel();
			if(!m_pModModel->ImportModFile(m_pActiveTa->m_sFilePath))
			{
				m_pActiveTa->m_ciErrorType=CTowerModel::ERROR_READ_FILE;
				continue;
			}
		}
		m_pActiveTa->ExtractActiveTaInstance(sHuGao,fLegA,fLegB,fLegC,fLegD);
	}
	DisplayProcess(100,"读取Exel文件,生成模型进度");
	//显示读取结果信息
	int nSumNum=GetTaModelNum(),nErrNum=0;
	for(CTowerModel* pTower=EnumFirstTaModel();pTower;pTower=EnumNextTaModel())
	{
		if(pTower->m_ciErrorType>0)
			nErrNum++;
	}
	int nRightNum=nSumNum-nErrNum;
	xMyErrLog.Log(CXhChar100("读取塔位共有%d个，提取成功%d个，提取失败%d个",nSumNum,nRightNum,nErrNum));
	//
	for(CTowerModel* pTower=TaManager.EnumFirstTaModel();pTower;pTower=TaManager.EnumNextTaModel())
	{
		if(pTower->m_ciErrorType==0)
			continue;
		if(pTower->m_ciErrorType==CTowerModel::ERROR_NO_FILE)
			xMyErrLog.Log("塔型%s-%s找不到对应的文件!",(char*)pTower->m_sTowerPlaceCode,(char*)pTower->m_sTowerType);
		else if(pTower->m_ciErrorType==CTowerModel::ERROR_READ_FILE)
			xMyErrLog.Log("塔型%s-%s对应的文件读取失败!",(char*)pTower->m_sTowerPlaceCode,(char*)pTower->m_sTowerType);
		else if(pTower->m_ciErrorType==CTowerModel::ERROR_FIND_HUGAO)
			xMyErrLog.Log("塔型%s-%s中不存在%s呼高!",(char*)pTower->m_sTowerPlaceCode,(char*)pTower->m_sTowerType,(char*)pTower->m_sHuGao);
		else if(pTower->m_ciErrorType==CTowerModel::ERROR_FIND_LEG)
			xMyErrLog.Log("塔型%s-%s的%s呼高中配腿信息有误!",(char*)pTower->m_sTowerPlaceCode,(char*)pTower->m_sTowerType,(char*)pTower->m_sHuGao);
		else if(pTower->m_ciErrorType==CTowerModel::ERROR_FIND_INSTANCE)
			xMyErrLog.Log("塔型%s-%s的配基信息有误!",(char*)pTower->m_sTowerPlaceCode,(char*)pTower->m_sTowerType);
	}
	return TRUE;
}
void CTowerManager::CreateXmlFiles(const char* sPath)
{
	DisplayProcess(0,"生成XML文件");
	int i=1,nModel=GetTaModelNum();
	for(CTowerModel* pTower=EnumFirstTaModel();pTower;pTower=EnumNextTaModel(),i++)
	{
		DisplayProcess(int(100*i/nModel),"生成XML文件");
		if(pTower->m_ciErrorType>0)
			continue;
		void* pInstance=pTower->ExtractActiveTaInstance();
		if(pInstance==NULL)
			continue;
		CXhChar200 sFullPath("%s\\%s.xml",sPath,(char*)pTower->GetFileName(pInstance));
		if(pTower->m_ciModelFlag==0)	//TID模型
			pTower->CreateXmlFile((ITidTowerInstance*)pInstance,sFullPath);
		if(pTower->m_ciModelFlag==1)	//MOD模型
			pTower->CreateXmlFile((IModTowerInstance*)pInstance,sFullPath);
	}
	DisplayProcess(100,"生成XML文件");
}
void CTowerManager::CreateModFiles(const char* sPath)
{
	DisplayProcess(0,"生成MOD文件");
	int i=1,nModel=GetTaModelNum();
	for(CTowerModel* pTower=EnumFirstTaModel();pTower;pTower=EnumNextTaModel(),i++)
	{
		DisplayProcess(int(100*i/nModel),"生成MOD文件");
		if(pTower->m_ciErrorType>0)
			continue;
		void* pInstance=pTower->ExtractActiveTaInstance();
		if(pInstance==NULL)
			continue;
		CXhChar200 sFullPath("%s\\%s.mod",sPath,(char*)pTower->GetFileName(pInstance));
		if(pTower->m_ciModelFlag==0)	//TID模型
			pTower->CreateModFile((ITidTowerInstance*)pInstance,sFullPath);
		if(pTower->m_ciModelFlag==1)	//MOD模型
			pTower->CreateModFile((IModTowerInstance*)pInstance,sFullPath);
	}
	DisplayProcess(100,"生成MOD文件");
}
void CTowerManager::Create3dsFiles(const char* sPath)
{
	DisplayProcess(0,"生成3ds文件");
	int i=1,nModel=GetTaModelNum();
	for(CTowerModel* pTower=EnumFirstTaModel();pTower;pTower=EnumNextTaModel(),i++)
	{
		DisplayProcess(int(100*i/nModel),"生成3ds文件");
		if(pTower->m_ciErrorType>0)
			continue;
		void* pInstance=pTower->ExtractActiveTaInstance();
		if(pInstance==NULL)
			continue;
		CXhChar200 sFullPath("%s\\%s.3ds",sPath,(char*)pTower->GetFileName(pInstance));
		if(m_ciInstanceLevel==0)
			sFullPath.Printf("%s\\%s.dxf",sPath,(char*)pTower->GetFileName(pInstance));
		if(m_ciInstanceLevel==0)
		{
			if(pTower->m_ciModelFlag==0)	//TID模型
				pTower->CreateDxfFile((ITidTowerInstance*)pInstance,sFullPath);
			if(pTower->m_ciModelFlag==1)	//MOD模型
				pTower->CreateDxfFile((IModTowerInstance*)pInstance,sFullPath);
		}
		else
		{
			if(pTower->m_ciModelFlag==0)	//TID模型
				pTower->Create3dsFile((ITidTowerInstance*)pInstance,sFullPath);
			if(pTower->m_ciModelFlag==1)	//MOD模型
				pTower->Create3dsFile((IModTowerInstance*)pInstance,sFullPath);
		}
	}
	DisplayProcess(100,"生成3ds文件");
}
