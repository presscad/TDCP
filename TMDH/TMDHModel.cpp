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
//���������Ҫ����תʱ����Ҫ������ģ��ˮƽ��ת180��
//������ģ������ϵ�е�X���Y��*-1
GECS CTMDHModel::BuildRotateCS()
{
	GECS cs;
	cs.origin.Set(0,0,0);
	cs.axis_x.Set(-1,0,0);
	cs.axis_y.Set(0,-1,0);
	cs.axis_z.Set(0,0,1);
	return cs;
}
/*����ת������ϵ����ά�������LDSģ������ϵת����GIM����ϵ��
//LDSģ�����꣺			GIMģ�����꣺		 ��LDS-->GIM��ת������ϵ
//	AxisX(1,0,0)		  AxisX(1,0,0)			AxisX(1,0,0)
//	AxisY(0,0,1)		  AxisY(0,0,-1)			AxisY(0,-1,0)
//	AxisZ(0,-1,0)		  AxisZ(0,1,0)			AxisZ(0,0,-1)
//  ԭ���ڶ���			 ԭ���ڵײ�
*/
GECS CTMDHModel::BuildTransCS(void* pInstance/*=NULL*/)
{
	GECS cs;
	if(m_ciModelFlag==TYPE_TID)
	{	//TIDģ������ϵ��LDSģ������ϵһ��
		if(pInstance==NULL)
			pInstance=ExtractActiveTaInstance();
		double fHeight=((ITidTowerInstance*)pInstance)->GetInstanceHeight();
		cs.origin.Set(0,0,fHeight);
		cs.axis_x.Set(1,0,0);
		cs.axis_y.Set(0,-1,0);
		cs.axis_z.Set(0,0,-1);	
	}
	else if(m_ciModelFlag==TYPE_MOD)
	{	//MODģ������ϵ��GIMģ������ϵһ��
		cs.origin.Set(0,0,0);
		cs.axis_x.Set(1,0,0);
		cs.axis_y.Set(0,1,0);
		cs.axis_z.Set(0,0,1);
	}
	return cs;
}
CXhChar100 CTMDHModel::GetFileName(void* pInstance)
{
	double dfNamedHeight=0;	//����(V1.4��������,�ɰ淵��0)����λm  wjh-2019.4.25
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
/*���������޵Ķ��չ�ϵ
   ����		  ����ģ��������	��תģ��������
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
		{	//����ģʽ����Ҫת��Ϊ��������
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
		{	//����ģʽ����Ҫת��Ϊ��������
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
	//��ʼ��XMLģ������
	CXmlModel xXmlModel(Manager.m_bUseUnitM);
	xXmlModel.m_sCategory.Copy("����");
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
		if(i==1||i==3)	 //���1����Y��Գ�
			xLocation.x*=-1.0;
		if(i==2||i==3)	//���1����X��Գ�
			xLocation.y*=-1.0;
		if(m_bTurnLeft)
			xLocation=rot_cs.TransPToCS(xLocation);
		xLocation=trans_cs.TransPToCS(xLocation);
		xXmlModel.m_xFundation[i].m_xFeetPos=xLocation;
	}
	//��ʼ��������Ϣ
	int nHangPt=Manager.m_pTidModel->HangPointCount();
	for(int i=0;i<nHangPt;i++)
	{	//����
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
		else if(strstr(sDes,"ǰ"))
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
	{	//����
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
		if(strstr(sDes,"ǰ"))
			pHangPt->m_ciHangDirect='Q';
		else if(strstr(sDes,"��"))
			pHangPt->m_ciHangDirect='H';
	}
	//����XML�ļ�
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
	xXmlModel.m_sCategory.Copy("����");
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
	//��ʼ���ҵ���Ϣ
	BYTE ciHangType=0;
	int nHangPt=Manager.m_pModModel->GetHangNodeNum();
	for(int i=0;i<nHangPt;i++)
	{	
		MOD_HANG_NODE* pModHangPt=Manager.m_pModModel->GetHangNodeById(i);
		if(pModHangPt->m_ciWireType=='E')
			continue;
		if(strstr(pModHangPt->m_sHangName,"ǰ") && ciHangType==0)
			ciHangType=1;
	}
	for(int i=0;i<nHangPt;i++)
	{	//����
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
		else if(strstr(pModHangPt->m_sHangName,"ǰ"))
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
	{	//����
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
		if(strstr(pModHangPt->m_sHangName,"ǰ"))
			pHangPt->m_ciHangDirect='Q';
		else if(strstr(pModHangPt->m_sHangName,"��"))
			pHangPt->m_ciHangDirect='H';
	}
	//����XML�ļ�
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
	//������άʵ��Ļ�����������Ϣ
	int nPart=pInstance->GetAssemblePartNum();
	int nBolt=pInstance->GetAssembleBoltNum();
	int nAnchorBolt=pInstance->GetAssembleAnchorBoltNum();
	int nSum=(ciLevel==3)?nPart+nBolt+nAnchorBolt:nPart;
	int serial=1;
	I3DSData* p3dsFile=C3DSFactory::Create3DSInstance();
	Manager.DisplayProcess(0,"����3DSģ��....");
	for(ITidAssemblePart* pAssmPart=pInstance->EnumAssemblePartFirst();pAssmPart;pAssmPart=pInstance->EnumAssemblePartNext(),serial++)
	{
		Manager.DisplayProcess(ftoi(100*serial/nSum),"���ɹ���3DSģ��....");
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
	{	//�������𣬻�����˨
		int iBolt=0;
		for(ITidAssembleBolt* pAssmBolt=pInstance->EnumAssembleBoltFirst();pAssmBolt;pAssmBolt=pInstance->EnumAssembleBoltNext(),serial++,iBolt++)
		{
			Manager.DisplayProcess(ftoi(100*serial/nSum),"������˨3DSģ��....");
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
			DisplayProcess(ftoi(100*serial/nSum),"���ɽŶ�3DSģ��....");
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
	Manager.DisplayProcess(100,"����3DSģ��....");
	p3dsFile->Creat3DSFile(sFileName);
	//
	C3DSFactory::Destroy(p3dsFile->GetSerial());
	p3dsFile=NULL;
}
void CTMDHModel::Create3dsFile(IModTowerInstance* pInstance,const char* sFileName)
{
	//������άʵ��Ļ�����������Ϣ
	UCS_STRU rot_cs=BuildRotateCS();
	I3DSData* p3dsFile=C3DSFactory::Create3DSInstance();
	int nIndex=1,nSum=pInstance->GetModRodNum();
	Manager.DisplayProcess(0,"����3DS�ļ�....");
	for(IModRod* pRod=pInstance->EnumModRodFir();pRod;pRod=pInstance->EnumModRodNext(),nIndex++)
	{
		Manager.DisplayProcess(ftoi(100*nIndex/nSum),"����3DS�ļ�....");
		CSolidBody solidBody;
		pRod->Create3dSolidModel(&solidBody,FALSE);
		if(m_bTurnLeft)
			solidBody.TransToACS(rot_cs);
		p3dsFile->AddSolidPart(&solidBody,nIndex,CXhChar16("Rod%d",nIndex),Manager.m_bUseUnitM);
	}
	Manager.DisplayProcess(100,"����3DS�ļ�....");
	p3dsFile->Creat3DSFile(sFileName);
	//
	C3DSFactory::Destroy(p3dsFile->GetSerial());
	p3dsFile=NULL;
}
void CTMDHModel::CreateDxfFile(ITidTowerInstance* pInstance,const char* sFileName)
{
	GECS rot_cs=BuildRotateCS();
	GECS trans_cs=BuildTransCS(pInstance);
	//��ȡֱ�߼��ϣ�����ʼ������
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
	//����DXF�ļ�
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
	//��ȡֱ�߼��ϣ�����ʼ������
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
	//����DXF�ļ�
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
	//��ʼ��GIM����ϵ
	GECS rot_cs=BuildRotateCS();
	GECS trans_cs=BuildTransCS(pInstance);
	//������άʵ��Ļ�����������Ϣ
	int nPart=pInstance->GetAssemblePartNum();
	int nBolt=pInstance->GetAssembleBoltNum();
	int nAnchorBolt=pInstance->GetAssembleAnchorBoltNum();
	int nSum=(ciLevel==3)?nPart+nBolt+nAnchorBolt:nPart;
	int serial=1;
	IStlData* pStlFile=CStlFactory::CreateStl();
	Manager.DisplayProcess(0,"����STLģ��....");
	for(ITidAssemblePart* pAssmPart=pInstance->EnumAssemblePartFirst();pAssmPart;pAssmPart=pInstance->EnumAssemblePartNext(),serial++)
	{
		Manager.DisplayProcess(ftoi(100*serial/nSum),"���ɹ���STLģ��....");
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
	{	//�������𣬻�����˨
		int iBolt=0;
		for(ITidAssembleBolt* pAssmBolt=pInstance->EnumAssembleBoltFirst();pAssmBolt;pAssmBolt=pInstance->EnumAssembleBoltNext(),serial++,iBolt++)
		{
			Manager.DisplayProcess(ftoi(100*serial/nSum),"������˨STLģ��....");
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
	Manager.DisplayProcess(100,"����STLģ��....");
	pStlFile->SaveFile((char*)sFileName,1);
	//
	CStlFactory::Destroy(pStlFile->GetSerial());
	pStlFile=NULL;
}
void CTMDHModel::CreateStlFile(IModTowerInstance* pInstance,const char* sFileName)
{
	//������άʵ��Ļ�����������Ϣ
	UCS_STRU rot_cs=BuildRotateCS();
	IStlData* pStlFile=CStlFactory::CreateStl();
	int nIndex=1,nSum=pInstance->GetModRodNum();
	Manager.DisplayProcess(0,"����STL�ļ�....");
	for(IModRod* pRod=pInstance->EnumModRodFir();pRod;pRod=pInstance->EnumModRodNext(),nIndex++)
	{
		Manager.DisplayProcess(ftoi(100*nIndex/nSum),"����STL�ļ�....");
		CSolidBody solidBody;
		pRod->Create3dSolidModel(&solidBody,FALSE);
		if(m_bTurnLeft)
			solidBody.TransToACS(rot_cs);
		pStlFile->AddSolidBody(solidBody.BufferPtr(),solidBody.BufferLength());
	}
	Manager.DisplayProcess(100,"����STL�ļ�....");
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
		logerr.Log(CXhChar100("%s�ļ���ʧ��!",sFileName));
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
	//��ȡ������Ϣ
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
	//��ȡMOD������Ϣ������MOD����ϵ
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
	//��ȡ�ҵ���Ϣ
	for(CLDSNode* pNode=m_xTower.EnumNodeFirst();pNode;pNode=m_xTower.EnumNodeNext())
	{
		if(pNode->m_cHangWireType!='C'&&pNode->m_cHangWireType!='E'&&pNode->m_cHangWireType!='J')
			continue;
		MOD_HANG_NODE* pGuaInfo=pModModel->AppendHangNode();
		pGuaInfo->m_xHangPos=ucs.TransPFromCS(pNode->xOriginalPos);
		if(pNode->m_cHangWireType=='C')			//����
			pGuaInfo->m_ciWireType='C';
		else if(pNode->m_cHangWireType=='E')	//����
			pGuaInfo->m_ciWireType='G';
		else //if(pNode->m_cHangWireType=='J')	//����
			pGuaInfo->m_ciWireType='T';
		if(strlen(pNode->m_sHangWireDesc)>0)
			strcpy(pGuaInfo->m_sHangName,pNode->m_sHangWireDesc);
	}
	//��ȡ��Ч�ڵ㣬���б�Ŵ���
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
	//��ʼ��MODģ�͵Ľڵ���˼�
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
			pModNode->SetLayer('L');	//�Ȳ�Leg
		else
			pModNode->SetLayer('B');	//����Body
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
			pModRod->SetLayer('L');	//�Ȳ�Leg
		else
			pModRod->SetLayer('B');	//����Body
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
	//��ʼ����������͵�MOD�ṹ
	pModModel->InitMultiModData();
	//����Mod�ļ�
	FILE *fp=fopen(sFileName,"wt,ccs=UTF-8");
	if(fp==NULL)
	{
		logerr.Log("%s�ļ���ʧ��!",sFileName);
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
	//��ʼ��GIM����ϵ
	UCS_STRU stdcs;
	LoadDefaultUCS(&stdcs);
	UCS_STRU draw_ucs;
	draw_ucs.origin.Set(0,0,pModule->LowestZ());
	draw_ucs.axis_x.Set(1,0,0);
	draw_ucs.axis_y.Set(0,-1,0);
	draw_ucs.axis_z.Set(0,0,-1);
	//������άʵ��Ļ�����������Ϣ
	int nPart=m_xTower.Parts.GetNodeNum();
	int serial=1;
	I3DSData* p3dsFile=C3DSFactory::Create3DSInstance();
	Manager.DisplayProcess(0,"����3DSģ��....");
	for(CLDSPart* pPart=m_xTower.EnumPartFirst();pPart;pPart=m_xTower.EnumPartNext(),serial++)
	{
		Manager.DisplayProcess(ftoi(100*serial/nPart),"���ɹ���3DSģ��....");
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
	Manager.DisplayProcess(0,"����3DSģ��....");
	for(CBlockReference *pBlockRef=m_xTower.BlockRef.GetFirst();pBlockRef;pBlockRef=m_xTower.BlockRef.GetNext())
	{
		CBlockModel *pBlock=m_xTower.Block.FromHandle(pBlockRef->m_hBlock);
		if(pBlock==NULL)
			continue;
		UCS_STRU acs=pBlockRef->GetACS();
		UCS_STRU cs=pBlock->TransToACS(acs);
		PARTSET partSet;
		if(pBlock->IsEmbeded())
		{	//Ƕ��ʽ����
			for(CLDSPart *pPart=m_xTower.EnumPartFirst();pPart;pPart=m_xTower.EnumPartNext())
			{
				if(pPart->m_hBlock==pBlock->handle)
					partSet.append(pPart);
			}
		}
		else 
		{	//��Ƕ��ʽ����
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
	Manager.DisplayProcess(100,"����3DSģ��....");
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
	//��ʼ��GIM����ϵ
	UCS_STRU draw_ucs;
	draw_ucs.origin.Set(0,0,pModule->LowestZ());
	draw_ucs.axis_x.Set(1,0,0);
	draw_ucs.axis_y.Set(0,-1,0);
	draw_ucs.axis_z.Set(0,0,-1);
	//������άʵ��Ļ�����������Ϣ
	int nPart=m_xTower.Parts.GetNodeNum();
	int serial=1;
	IStlData* pStlFile=CStlFactory::CreateStl();
	Manager.DisplayProcess(0,"����STLģ��....");
	for(CLDSPart* pPart=m_xTower.EnumPartFirst();pPart;pPart=m_xTower.EnumPartNext(),serial++)
	{
		Manager.DisplayProcess(ftoi(100*serial/nPart),"���ɹ���STLģ��....");
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
	Manager.DisplayProcess(100,"����STLģ��....");
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
		logerr.Log("����%d�ĸ߶�ȱʧ�����",nItemNoOrRow);
		break;
	case ERROR_MOD_LEG_H:
	case ERROR_MOD_HLEG_MISS:
		logerr.Log("����%d�ĸ߶�ȱʧ�����",nItemNoOrRow);
		break;
	case ERROR_MOD_HSUBLEG_MISS:
	case ERROR_MOD_SUBLEG_H:
		logerr.Log("����%d����%d�ĸ߶ȴ��������ȱʧ",nItemNoOrRow,nSubLegNo);
		break;
	case ERROR_MOD_H_MISS:
	case ERROR_MOD_LAYOUT_H:
		logerr.Log("��%d��������ʽ�����ݴ���",nItemNoOrRow);
		break;
	case ERROR_MOD_LAYOUT_R:
		logerr.Log("��%d��δ֪�˼�����ʽ����",nItemNoOrRow);
		break;
	case ERROR_MOD_P_MISS:
	case ERROR_MOD_LAYOUT_P:
		logerr.Log("��%d�е��ʽ�����ݴ���",nItemNoOrRow);
		break;
	case ERROR_MOD_G_MISS:
	case ERROR_MOD_LAYOUT_G:
		logerr.Log("��%d�йҵ��ʽ�����ݴ���",nItemNoOrRow);
		break;
	case ERROR_MOD_LAYOUT_R_L: 
	case ERROR_MOD_R_L_MISS:
		logerr.Log("��%d�нǸָ�ʽ�����ݴ���",nItemNoOrRow);
		break;
	case ERROR_MOD_LAYOUT_R_T:
	case ERROR_MOD_R_T_MISS:
		logerr.Log("��%d�иֹܸ�ʽ�����ݴ���",nItemNoOrRow);
		break;
	case ERROR_MOD_LAYOUT_SUNLEG_H:
		logerr.Log("����%d����%d�ĸ߶�������ȱʧ",nItemNoOrRow,nSubLegNo);
		break;
	case ERROR_MOD_LAYOUT_LEG_H:
		logerr.Log("����%d�ĸ߶�������ȱʧ",nItemNoOrRow);
		break;
	case ERROR_MOD_LAYOUT_BODY_H:
		logerr.Log("����%d�ĸ߶�������ȱʧ",nItemNoOrRow);
		break;
	case ERROR_MOD_LAYOUT_SUNLEG:
		logerr.Log("��%d��ǰȱʧ���ȱ��������",nItemNoOrRow,nSubLegNo);
		break;
	case ERROR_MOD_LAYOUT_LEG:
		logerr.Log("��%d��ǰȱʧ���ȱ��������",nItemNoOrRow);
		break;
	case ERROR_MOD_LAYOUT_BODY:
		logerr.Log("��%d��ǰȱʧ������������",nItemNoOrRow);
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
		//��ѹ�ȼ�
		fprintf(fp,"VOLTAGE = ��ѹ�ȼ� = %s\n",(char*)m_xTowerProp.m_sVoltGrade);
		//����
		fprintf(fp,"TYPE = ���� = %s\n",(char*)m_xTowerProp.m_sType);
		//��������
		fprintf(fp,"TEXTURE = �������� = %s\n",(char*)m_xTowerProp.m_sTexture);
		//�̶���ʽ
		fprintf(fp,"FIXEDTYPE = �̶���ʽ = %s\n",(char*)m_xTowerProp.m_sFixedType);
		//��������
		fprintf(fp,"TOWERTYPE = �������� = %s\n",(char*)m_xTowerProp.m_sTaType);
		//��·��
		fprintf(fp,"CIRCUIT = ��·�� = %d\n",m_xTowerProp.m_nCircuit);
		//�����ͺ�
		fprintf(fp,"CONDUCTOR = �����ͺ� = %s\n",(char*)m_xTowerProp.m_sCWireSpec);
		//�����ͺ�
		fprintf(fp,"GROUNDWIRE = �����ͺ� = %s\n",(char*)m_xTowerProp.m_sEWireSpec);
		//��������
		ss.Printf("%f",m_xTowerProp.m_fWindSpeed);
		SimplifiedNumString(ss);
		fprintf(fp,"REFERENCEWINDSPEED = ��ƻ������� = %s\n",(char*)ss);
		//�������
		ss.Printf("%f",m_xTowerProp.m_fNiceThick);
		SimplifiedNumString(ss);
		fprintf(fp,"DESIGNICETHICKNESS = ��Ƹ������ = %s\n",(char*)ss);
		//ˮƽ����
		fprintf(fp,"DESIGNWINDSPAN = ���ˮƽ���� = %s\n",(char*)m_xTowerProp.m_sWindSpan);
		//��ֱ����
		fprintf(fp,"DESIGNWEIGHTSPAN = ��ƴ�ֱ���� = %s\n",(char*)m_xTowerProp.m_sWeightSpan);
		//ǰ�������
		ss.Printf("%f",m_xTowerProp.m_fFrontRulingSpan);
		SimplifiedNumString(ss);
		fprintf(fp,"DESIGNFRONTRULINGSPAN = ���ǰ������� = %s\n",(char*)ss);
		//��������
		ss.Printf("%f",m_xTowerProp.m_fBackRulingSpan);
		SimplifiedNumString(ss);
		fprintf(fp,"DESIGNBACKRULINGSPAN=��ƺ�������=%s\n",(char*)ss);
		//��󵵾�
		ss.Printf("%f",m_xTowerProp.m_fMaxSpan);
		SimplifiedNumString(ss);
		fprintf(fp,"DESIGNMAXSPAN = �����󵵾� =%s\n",(char*)ss);
		//ת�Ƿ�Χ
		fprintf(fp,"ANGLERANGE = ���ת�Ƿ�Χ =%s\n",(char*)m_xTowerProp.m_sAngleRange);
		//Kv
		ss.Printf("%f",m_xTowerProp.m_fDesignKV);
		SimplifiedNumString(ss);
		fprintf(fp,"DESIGNKV = ���Kvֵ = %s\n",(char*)ss);
		//�������
		fprintf(fp,"RATEDNOMINALHEIGHT = ������� = %s\n",(char*)m_xTowerProp.m_sRatedHeight);
		//���߷�Χ
		fprintf(fp,"NOMINALHEIGHTRANGE = ���߷�Χ = %s\n",(char*)m_xTowerProp.m_sHeightRange);
		//����
		fprintf(fp,"TOWERWEIGHT = ���� = %s\n",(char*)m_xTowerProp.m_sTowerWeight);
		//��ƹ�Ƶҡ�ڽǶ�
		ss.Printf("%f",m_xTowerProp.m_fFrequencyRockAngle);
		SimplifiedNumString(ss);
		fprintf(fp,"ROCKANGLEOFDESIGNPOWERFREQUENCY = ��ƹ�Ƶҡ�ڽǶ� = %s\n",(char*)ss);
		//����׵�ҡ�ڽǶ�
		ss.Printf("%f",m_xTowerProp.m_fLightningRockAngle);
		SimplifiedNumString(ss);
		fprintf(fp,"ROCKANGLEOFDESIGNLIGHTNING = ����׵�ҡ�ڽǶ� = %s\n",(char*)ss);
		//��Ʋ���ҡ�ڽǶ�
		ss.Printf("%f",m_xTowerProp.m_fSwitchingRockAngle);
		SimplifiedNumString(ss);
		fprintf(fp,"ROCKANGLEOFDESIGNSWITCHING = ��Ʋ���ҡ�ڽǶ� = %s\n",(char*)ss);
		//��ƴ�����ҵҡ�ڽǶ�
		ss.Printf("%f",m_xTowerProp.m_fWorkingRockAngle);
		SimplifiedNumString(ss);
		fprintf(fp,"ROCKANGLEOFDESIGNLIVEWORKING = ��ƴ�����ҵҡ�ڽǶ� = %s\n",(char*)ss);
		//��������
		fprintf(fp,"MANUFACTURER = �������� = %s\n",(char*)m_xTowerProp.m_sManuFacturer);
		//���ʱ���
		fprintf(fp,"MATERIALCODE = ���ʱ��� = %s\n",(char*)m_xTowerProp.m_sMaterialCode);
		//װ��ģ�ͱ��
		fprintf(fp,"PROCESSMODELCODE = װ��ģ�ͱ�� = %s\n",(char*)m_xTowerProp.m_sProModelCode);
	}
	else
	{
		CXhChar500 sValue;
		wchar_t sWValue[MAX_PATH];
		//��ѹ�ȼ�
		sValue.Printf("VOLTAGE = ��ѹ�ȼ� = %s",(char*)m_xTowerProp.m_sVoltGrade);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//����
		sValue.Printf("TYPE = ���� = %s",(char*)m_xTowerProp.m_sType);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//��������
		sValue.Printf("TEXTURE = �������� = %s",(char*)m_xTowerProp.m_sTexture);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//�̶���ʽ
		sValue.Printf("FIXEDTYPE = �̶���ʽ = %s",(char*)m_xTowerProp.m_sFixedType);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//��������
		sValue.Printf("TOWERTYPE = �������� = %s",(char*)m_xTowerProp.m_sTaType);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//��·��
		sValue.Printf("CIRCUIT = ��·�� = %d",m_xTowerProp.m_nCircuit);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//�����ͺ�
		sValue.Printf("CONDUCTOR = �����ͺ� = %s",(char*)m_xTowerProp.m_sCWireSpec);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//�����ͺ�
		sValue.Printf("GROUNDWIRE = �����ͺ� = %s",(char*)m_xTowerProp.m_sEWireSpec);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//��ƻ�������
		ss.Printf("%f",m_xTowerProp.m_fWindSpeed);
		SimplifiedNumString(ss);
		sValue.Printf("REFERENCEWINDSPEED = ��ƻ������� = %s",(char*)ss);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//��Ƹ������
		ss.Printf("%f",m_xTowerProp.m_fNiceThick);
		SimplifiedNumString(ss);
		sValue.Printf("DESIGNICETHICKNESS = ��Ƹ������ = %s",(char*)ss);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//���ˮƽ����
		sValue.Printf("DESIGNWINDSPAN = ���ˮƽ���� = %s",(char*)m_xTowerProp.m_sWindSpan);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//��ƴ�ֱ����
		sValue.Printf("DESIGNWEIGHTSPAN = ��ƴ�ֱ���� = %s",(char*)m_xTowerProp.m_sWeightSpan);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//���ǰ�������
		ss.Printf("%f",m_xTowerProp.m_fFrontRulingSpan);
		SimplifiedNumString(ss);
		sValue.Printf("DESIGNFRONTRULINGSPAN = ���ǰ������� = %s",(char*)ss);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//��ƺ�������
		ss.Printf("%f",m_xTowerProp.m_fBackRulingSpan);
		SimplifiedNumString(ss);
		sValue.Printf("DESIGNBACKRULINGSPAN = ��ƺ������� = %s",(char*)ss);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//�����󵵾�
		ss.Printf("%f",m_xTowerProp.m_fMaxSpan);
		SimplifiedNumString(ss);
		sValue.Printf("DESIGNMAXSPAN = �����󵵾� = %s",(char*)ss);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//���ת�Ƿ�Χ
		sValue.Printf("ANGLERANGE = ���ת�Ƿ�Χ = %s",(char*)m_xTowerProp.m_sAngleRange);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//Kv
		ss.Printf("%f",m_xTowerProp.m_fDesignKV);
		SimplifiedNumString(ss);
		sValue.Printf("DESIGNKV = ���Kvֵ = %s",(char*)ss);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//�������
		sValue.Printf("RATEDNOMINALHEIGHT = ������� = %s",(char*)m_xTowerProp.m_sRatedHeight);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//���߷�Χ
		sValue.Printf("NOMINALHEIGHTRANGE = ���߷�Χ = %s",(char*)m_xTowerProp.m_sHeightRange);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//����
		sValue.Printf("TOWERWEIGHT = ���� = %s",(char*)m_xTowerProp.m_sTowerWeight);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//��ƹ�Ƶҡ�ڽǶ�
		ss.Printf("%f",m_xTowerProp.m_fFrequencyRockAngle);
		SimplifiedNumString(ss);
		sValue.Printf("ROCKANGLEOFDESIGNPOWERFREQUENCY = ��ƹ�Ƶҡ�ڽǶ� = %s",(char*)ss);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//����׵�ҡ�ڽǶ�
		ss.Printf("%f",m_xTowerProp.m_fLightningRockAngle);
		SimplifiedNumString(ss);
		sValue.Printf("ROCKANGLEOFDESIGNLIGHTNING = ����׵�ҡ�ڽǶ� = %s",(char*)ss);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//��Ʋ���ҡ�ڽǶ�
		ss.Printf("%f",m_xTowerProp.m_fSwitchingRockAngle);
		SimplifiedNumString(ss);
		sValue.Printf("ROCKANGLEOFDESIGNSWITCHING = ��Ʋ���ҡ�ڽǶ� = %s",(char*)ss);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//��ƴ�����ҵҡ�ڽǶ�
		ss.Printf("%f",m_xTowerProp.m_fWorkingRockAngle);
		SimplifiedNumString(ss);
		sValue.Printf("ROCKANGLEOFDESIGNLIVEWORKING = ��ƴ�����ҵҡ�ڽǶ� = %s",(char*)ss);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//��������
		sValue.Printf("MANUFACTURER = �������� = %s",(char*)m_xTowerProp.m_sManuFacturer);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//���ʱ���
		sValue.Printf("MATERIALCODE = ���ʱ��� = %s",(char*)m_xTowerProp.m_sMaterialCode);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//װ��ģ�ͱ��
		sValue.Printf("PROCESSMODELCODE = װ��ģ�ͱ�� = %s",(char*)m_xTowerProp.m_sProModelCode);
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
		logerr.Log("%s��ʧ��!",sFamFile);
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
		if(strstr(skey1,"VOLTAGE")||stricmp(skey2,"��ѹ�ȼ�")==0)
			m_xTowerProp.m_sVoltGrade.Copy(skey);
		else if(stricmp(skey1,"TYPE")==0||stricmp(skey2,"����")==0)
			m_xTowerProp.m_sType.Copy(skey);
		else if(stricmp(skey1,"TEXTURE")==0||stricmp(skey2,"��������")==0)
			m_xTowerProp.m_sTexture.Copy(skey);
		else if(stricmp(skey1,"FIXEDTYPE")==0||stricmp(skey2,"�̶���ʽ")==0)
			m_xTowerProp.m_sFixedType.Copy(skey);
		else if(stricmp(skey1,"TOWERTYPE")==0||stricmp(skey2,"��������")==0)
			m_xTowerProp.m_sTaType.Copy(skey);
		else if(stricmp(skey1,"CIRCUIT")==0||stricmp(skey2,"��·��")==0)
			m_xTowerProp.m_nCircuit=atoi(skey);
		else if(stricmp(skey1,"CONDUCTOR")==0||stricmp(skey2,"�����ͺ�")==0)
			m_xTowerProp.m_sCWireSpec.Copy(skey);
		else if(stricmp(skey1,"GROUNDWIRE")==0||stricmp(skey2,"�����ͺ�")==0)
			m_xTowerProp.m_sEWireSpec.Copy(skey);
		else if(stricmp(skey1,"REFERENCEWINDSPEED")==0||stricmp(skey2,"��ƻ�������")==0)
			m_xTowerProp.m_fWindSpeed=atof(skey);
		else if(stricmp(skey1,"DESIGNICETHICKNESS")==0||stricmp(skey2,"��Ƹ������")==0)
			m_xTowerProp.m_fNiceThick=atof(skey);
		else if(stricmp(skey1,"DESIGNWINDSPAN")==0||stricmp(skey2,"���ˮƽ����")==0)
			m_xTowerProp.m_sWindSpan.Copy(skey);
		else if(stricmp(skey1,"DESIGNWEIGHTSPAN")==0||stricmp(skey2,"��ƴ�ֱ����")==0)
			m_xTowerProp.m_sWeightSpan.Copy(skey);
		else if(stricmp(skey1,"DESIGNFRONTRULINGSPAN")==0||stricmp(skey2,"���ǰ�������")==0)
			m_xTowerProp.m_fFrontRulingSpan=atof(skey);
		else if(stricmp(skey1,"DESIGNBACKRULINGSPAN")==0||stricmp(skey2,"��ƺ�������")==0)
			m_xTowerProp.m_fBackRulingSpan=atof(skey);
		else if(stricmp(skey1,"DESIGNMAXSPAN")==0||stricmp(skey2,"�����󵵾�")==0)
			m_xTowerProp.m_fMaxSpan=atof(skey);
		else if(stricmp(skey1,"ANGLERANGE")==0||stricmp(skey2,"���ת�Ƿ�Χ")==0)
			m_xTowerProp.m_sAngleRange.Copy(skey);
		else if(stricmp(skey1,"DESIGNKV")==0||stricmp(skey2,"���Kvֵ")==0)
			m_xTowerProp.m_fDesignKV=atof(skey);
		else if(stricmp(skey1,"RATEDNOMINALHEIGHT")==0||stricmp(skey2,"�������")==0)
			m_xTowerProp.m_sRatedHeight.Copy(skey);
		else if(stricmp(skey1,"NOMINALHEIGHTRANGE")==0||stricmp(skey2,"���߷�Χ")==0)
			m_xTowerProp.m_sHeightRange.Copy(skey);
		else if(stricmp(skey1,"TOWERWEIGHT")==0||stricmp(skey2,"����")==0)
			m_xTowerProp.m_sTowerWeight.Copy(skey);
		else if(stricmp(skey1,"ROCKANGLEOFDESIGNPOWERFREQUENCY")==0||stricmp(skey2,"��ƹ�Ƶҡ�ڽǶ�")==0)
			m_xTowerProp.m_fFrequencyRockAngle=atof(skey);
		else if(stricmp(skey1,"ROCKANGLEOFDESIGNLIGHTNING")==0||stricmp(skey2,"����׵�ҡ�ڽǶ�")==0)
			m_xTowerProp.m_fLightningRockAngle=atof(skey);
		else if(stricmp(skey1,"ROCKANGLEOFDESIGNSWITCHING")==0||stricmp(skey2,"��Ʋ���ҡ�ڽǶ�")==0)
			m_xTowerProp.m_fSwitchingRockAngle=atof(skey);
		else if(stricmp(skey1,"ROCKANGLEOFDESIGNLIVEWORKING")==0||stricmp(skey2,"��ƴ�����ҵҡ�ڽǶ�")==0)
			m_xTowerProp.m_fWorkingRockAngle=atof(skey);
		else if(stricmp(skey1,"MANUFACTURER")==0||stricmp(skey2,"��������")==0)
			m_xTowerProp.m_sManuFacturer.Copy(skey);
		else if(stricmp(skey1,"MATERIALCODE")==0||stricmp(skey2,"���ʱ���")==0)
			m_xTowerProp.m_sMaterialCode.Copy(skey);
		else if(stricmp(skey1,"PROCESSMODELCODE")==0||stricmp(skey2,"װ��ģ�ͱ��")==0)
			m_xTowerProp.m_sProModelCode.Copy(skey);
	}
	fclose(fp);
}
void CTMDHGim::ReadDevFile(const char* sDevFile)
{
	FILE *fp=fopen(sDevFile,"rt");
	if(fp==NULL)
	{
		logerr.Log("%s��ʧ��!",sDevFile);
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
	//��ȡFAM&PHM�ļ�
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
		logerr.Log("%s��ʧ��!",sPhmFile);
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
	//��ȡMOD�ļ�
	if(sModName.GetLength()>0)
		_splitpath(sModName,NULL,NULL,m_sModName,NULL);
}
void CTMDHGim::ReadCbmFile(const char* sCbmFile)
{
	FILE *fp=fopen(sCbmFile,"rt");
	if(fp==NULL)
	{
		logerr.Log("%s��ʧ��!",sCbmFile);
		return;
	}
	
	fclose(fp);
}
void CTMDHGim::CreateGuidFile()
{
	//�����Ŀ¼�����ɸ����ļ���
	CXhChar100 sOutPath=m_sOutputPath;
	sOutPath.Append("\\");
	sOutPath.Append(m_sGimName);
	_mkdir(sOutPath);
	//�����Ŀ¼������MOD�ļ�
	CXhChar100 sFilePath=sOutPath;
	sFilePath.Append("\\MOD");
	_mkdir(sFilePath);
	CXhChar200 sModFilePath("%s\\%s.mod",(char*)sFilePath,(char*)m_sModName);
	WriteModFile(sModFilePath);
	//��������MOD�ļ���PHM�ļ�
	sFilePath=sOutPath;
	sFilePath.Append("\\PHM");
	_mkdir(sFilePath);
	m_sPhmGuid=CreateGuidStr();
	CXhChar200 sPhmFilePath("%s\\%s.phm",(char*)sFilePath,(char*)m_sPhmGuid);
	WritePhmFile(sPhmFilePath,m_sModName);
	//��������PHM�ļ���DEV�ļ�����Ӧ�����ļ�
	sFilePath=sOutPath;
	sFilePath.Append("\\DEV");
	_mkdir(sFilePath);
	m_sFamGuid=m_sDevGuid=CreateGuidStr();
	CXhChar200 sFamFilePath("%s\\%s.fam",(char*)sFilePath,(char*)m_sFamGuid);
	WriteFamFile(sFamFilePath);
	CXhChar200 sDevFilePath("%s\\%s.dev",(char*)sFilePath,(char*)m_sDevGuid);
	WriteDevFile(sDevFilePath);
	//��������DEV�ļ����豸�����ļ�CBM
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
	//��ȡ��ǰʱ�䣬��ת��Ϊ������
	if(strlen(m_xGimHeadProp.m_sTime)<=0)
	{
		time_t tt = time(0);
		char sTime[32]={0};
		strftime(sTime, sizeof(sTime),"%Y-%m-%d %H:%M", localtime(&tt));
		memcpy(m_xGimHeadProp.m_sTime,sTime,16);
	}
	CXhString szBuffSize(m_xGimHeadProp.m_sBufSize,8);
	szBuffSize.Printf("%d",zipBuffer.GetLength());
	//д��ָ��ͷ����
	buffer.Write(m_xGimHeadProp.m_sFileTag,16);			//�ļ���ʶ
	buffer.Write(m_xGimHeadProp.m_sFileName,256);		//�ļ�����
	buffer.Write(m_xGimHeadProp.m_sDesigner,64);		//�����
	buffer.Write(m_xGimHeadProp.m_sUnit,256);			//��֯��λ
	buffer.Write(m_xGimHeadProp.m_sSoftName,128);		//�������
	buffer.Write(m_xGimHeadProp.m_sTime,16);			//����ʱ��		
	buffer.Write(m_xGimHeadProp.m_sSoftMajorVer,8);		//������汾��
	buffer.Write(m_xGimHeadProp.m_sSoftMinorVer,8);		//����ΰ汾��
	buffer.Write(m_xGimHeadProp.m_sMajorVersion,8);		//��׼���汾��
	buffer.Write(m_xGimHeadProp.m_sMinorVersion,8);		//��׼�ΰ汾��
	buffer.Write(m_xGimHeadProp.m_sBufSize,8);			//�洢�����С
	//д��7Z����
	buffer.Write(zipBuffer.GetBufferPtr(),zipBuffer.GetLength());
}
void CTMDHGim::FromBuffer(CBuffer &buffer,CBuffer &zipBuffer)
{
	buffer.SeekToBegin();
	//��ȡͷ����
	buffer.Read(m_xGimHeadProp.m_sFileTag,16);		//�ļ���ʶ
	buffer.Read(m_xGimHeadProp.m_sFileName,256);	//�ļ�����
	buffer.Read(m_xGimHeadProp.m_sDesigner,64);		//�����
	buffer.Read(m_xGimHeadProp.m_sUnit,256);		//��֯��λ
	buffer.Read(m_xGimHeadProp.m_sSoftName,128);	//�������
	buffer.Read(m_xGimHeadProp.m_sTime,16);			//����ʱ��		
	buffer.Read(m_xGimHeadProp.m_sSoftMajorVer,8);	//������汾��
	buffer.Read(m_xGimHeadProp.m_sSoftMinorVer,8);	//����ΰ汾��
	buffer.Read(m_xGimHeadProp.m_sMajorVersion,8);	//��׼���汾��
	buffer.Read(m_xGimHeadProp.m_sMinorVersion,8);	//��׼�ΰ汾��
	buffer.Read(m_xGimHeadProp.m_sBufSize,8);		//�洢�����С
	//��ȡ7Z���ִ洢����
	int buf_size=atoi(m_xGimHeadProp.m_sBufSize);
	zipBuffer.Write(NULL,buf_size);
	buffer.Read(zipBuffer.GetBufferPtr(),buf_size);		//�洢�����С
}
BOOL CTMDHGim::UpdateGimHeadInfo()
{
	//��ȡ7zѹ����Ĭ�ϵ��ֽ�
	CXhChar100 gim_path("%s\\%s.gim",(char*)m_sOutputPath,(char*)m_sGimName);
	FILE* fp=fopen(gim_path,"rb");
	if(fp==NULL)
	{
		logerr.Log(CXhChar100("%s��������ʧ��(��)!",(char*)gim_path));
		return FALSE;
	}
	fseek(fp,0,SEEK_END);
	long buf_size=ftell(fp);
	fseek(fp,0,SEEK_SET);
	CBuffer zipBuf;
	zipBuf.Write(NULL,buf_size);
	fread(zipBuf.GetBufferPtr(),zipBuf.GetLength(),1,fp);
	fclose(fp);
	//��GIM�ļ�ͷ������ض�����
	CBuffer gimBuf(10000000);
	ToBuffer(gimBuf,zipBuf);
	fp=fopen(gim_path,"wb");
	if(fp==NULL)
	{
		logerr.Log(CXhChar100("%s��������ʧ��(д)!",(char*)gim_path));
		return FALSE;
	}
	fwrite(gimBuf.GetBufferPtr(),gimBuf.GetLength(),1,fp);
	fclose(fp);
	return TRUE;
}
BOOL CTMDHGim::UpdateGimTowerPro()
{
	//��дFAM�ļ�
	CXhChar100 sFilePath=m_sOutputPath;
	sFilePath.Append("\\");
	sFilePath.Append(m_sGimName);
	sFilePath.Append("\\DEV");
	CXhChar200 sFamFilePath("%s\\%s.fam",(char*)sFilePath,(char*)m_sFamGuid);
	WriteFamFile(sFamFilePath);
	//����GIM������
	CXhChar100 gim_path("%s\\%s.gim",(char*)m_sOutputPath,(char*)m_sGimName);
	CXhChar200 cmd_str("7z.exe u %s %s\\",(char*)gim_path,(char*)sFilePath);
	if(!CTMDHManager::Run7zCmd(cmd_str))
		return FALSE;
	return TRUE;
}
//����ѹ���������
BOOL CTMDHGim::PackGimFile()
{
	//Ĭ��ѹ��Ϊ7Z��ʽ��
	CXhChar100 sOutPath=m_sOutputPath;
	sOutPath.Append("\\");
	sOutPath.Append(m_sGimName);
	CXhChar100 gim_path("%s\\%s.gim",(char*)m_sOutputPath,(char*)m_sGimName);
	//7z.exe V18.5֧��ѹ�����Զ�ɾ���ļ���
	//CXhChar200 cmd_str("7z.exe a %s %s\\* -sdel",(char*)gim_path,(char*)sOutPath);
	//7z.exe V4.65��֧���Զ�ɾ���ļ���
	CXhChar200 cmd_str("7z.exe a %s %s\\*",(char*)gim_path,(char*)sOutPath);
	if(!CTMDHManager::Run7zCmd(cmd_str))
		return FALSE;
	//ɾ����ʱ���ɵ��ļ���
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
	BYTE ciReadType=0;	//0:����|1.����|2.����
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
		{	//����
			ciReadType=0;
			continue;
		}
		else if(strstr(key_word,"Leg")&&key_word[0]=='L')
		{	//����
			ciReadType=1;
			continue;
		}
		else if(strstr(key_word,"SubLeg")&&key_word[0]=='S')
		{	//���ȳ�=���в�����͵�-�Ȳ���͵�
			ciReadType=2;
			continue;
		}
		else
		{
			if(stricmp(key_word,"G")!=0)
				line_txt.Replace(',',' ');
			key_word.Remove(' ');
			if(stricmp(key_word,"P")==0)
			{	//�ڵ�
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
			{	//�˼�
				int indexS=0,indexE=0;
				double fWidth=0,fThick=0;
				char sMat[16]="",sSpec[16]="";
				if(strstr(line_txt,"L"))
				{	//�Ǹ�
					int i=0;
					for(skey=strtok(NULL,",");skey;skey=strtok(NULL,","))
						i++;
					if(i!=10)
					{
						CModFileError::SetError(CModFileError::ERROR_MOD_R_L_MISS,nRow);
						return;
					}
				}
				else if(strstr(line_txt,"��")||strstr(line_txt,"��"))
				{	//�ֹ�
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
				{	//���Ͳ�����
					logerr.Log("Mod��%d��δ֪�˼�!",nRow);
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
//������
const DWORD HASHTABLESIZE = 500;
IMPLEMENT_PROP_FUNC(CTMDHGim);
void CTMDHGim::InitPropHashtable()
{
	int id = 1;
	propHashtable.SetHashTableGrowSize(HASHTABLESIZE);
	propStatusHashtable.CreateHashTable(50);
	//������Ϣ
	AddPropItem("BasicInfo",PROPLIST_ITEM(id++,"������Ϣ","������Ϣ"));
	AddPropItem("Designer",PROPLIST_ITEM(id++,"�����","�����"));
	AddPropItem("Unit",PROPLIST_ITEM(id++,"��֯��λ","��֯��λ"));
	AddPropItem("MajorVer",PROPLIST_ITEM(id++,"���汾��","���汾��"));
	AddPropItem("MinorVer",PROPLIST_ITEM(id++,"�ΰ汾��","�ΰ汾��"));
	//�����Ϣ
	AddPropItem("TowerInfo",PROPLIST_ITEM(id++,"�����Ϣ","�����Ϣ"));
	AddPropItem("VoltGrade",PROPLIST_ITEM(id++,"��ѹ�ȼ�","��ѹ�ȼ�"));
	AddPropItem("Type",PROPLIST_ITEM(id++,"����","����"));
	AddPropItem("Texture",PROPLIST_ITEM(id++,"��������","��������","�Ǹ���|�ֹ���|�ֹܸ�|ˮ���|����"));
	AddPropItem("FixedType",PROPLIST_ITEM(id++,"�̶���ʽ","�̶���ʽ","����|����"));
	AddPropItem("TaType",PROPLIST_ITEM(id++,"��������","��������","������|������|��λ��"));
	AddPropItem("Circuit",PROPLIST_ITEM(id++,"��·��","��·��"));
	AddPropItem("CWireSpec",PROPLIST_ITEM(id++,"�����ͺ�","�����ͺ�"));
	AddPropItem("EWireSpec",PROPLIST_ITEM(id++,"�����ͺ�","�����ͺ�"));
	AddPropItem("WindSpeed",PROPLIST_ITEM(id++,"��ƻ�������","��ƻ�������"));
	AddPropItem("NiceThick",PROPLIST_ITEM(id++,"��Ƹ������","��Ƹ������"));
	AddPropItem("WindSpan",PROPLIST_ITEM(id++,"���ˮƽ����","���ˮƽ����"));
	AddPropItem("WeightSpan",PROPLIST_ITEM(id++,"��ƴ�ֱ����","��ƴ�ֱ����"));
	AddPropItem("MaxSpan",PROPLIST_ITEM(id++,"�����󵵾�","��󵵾�"));
	AddPropItem("FrontRulingSpan",PROPLIST_ITEM(id++,"���ǰ�������","ǰ�������"));
	AddPropItem("BackRulingSpan",PROPLIST_ITEM(id++,"��ƺ�������","��������"));
	AddPropItem("AngleRange",PROPLIST_ITEM(id++,"���ת�Ƿ�Χ","ת�Ƿ�Χ"));
	AddPropItem("DesignKV",PROPLIST_ITEM(id++,"���Kvֵ","���Kvֵ"));
	AddPropItem("RatedHeight",PROPLIST_ITEM(id++,"�������","�������"));
	AddPropItem("HeightRange",PROPLIST_ITEM(id++,"���߷�Χ","���߷�Χ"));
	AddPropItem("TowerWeight",PROPLIST_ITEM(id++,"����","����"));
	AddPropItem("FrequencyRockAngle",PROPLIST_ITEM(id++,"��ƹ�Ƶҡ�ڽǶ�","��ƹ�Ƶҡ�ڽǶ�"));
	AddPropItem("LightningRockAngle",PROPLIST_ITEM(id++,"����׵�ҡ�ڽǶ�","����׵�ҡ�ڽǶ�"));
	AddPropItem("SwitchingRockAngle",PROPLIST_ITEM(id++,"��Ʋ���ҡ�ڽǶ�","��Ʋ���ҡ�ڽǶ�"));
	AddPropItem("WorkingRockAngle",PROPLIST_ITEM(id++,"��ƴ�����ҵ�Ƕ�","��ƴ�����ҵҡ�ڽǶ�"));
	AddPropItem("ManuFacturer",PROPLIST_ITEM(id++,"��������","��������"));
	AddPropItem("MaterialCode",PROPLIST_ITEM(id++,"���ʱ���","���ʱ���"));
}
int CTMDHGim::GetPropValueStr(long id,char* valueStr,UINT nMaxStrBufLen/*=100*/)	//��������ID�õ�����ֵ�ַ���
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
	//��ȡͷ����
	buffer.Read(m_xGimHeadProp.m_sFileTag,16);		//�ļ���ʶ
	buffer.Read(m_xGimHeadProp.m_sFileName,256);	//�ļ�����
	buffer.Read(m_xGimHeadProp.m_sDesigner,64);		//�����
	buffer.Read(m_xGimHeadProp.m_sUnit,256);		//��֯��λ
	buffer.Read(m_xGimHeadProp.m_sSoftName,128);	//�������
	buffer.Read(m_xGimHeadProp.m_sTime,16);			//����ʱ��		
	buffer.Read(m_xGimHeadProp.m_sSoftMajorVer,8);	//������汾��
	buffer.Read(m_xGimHeadProp.m_sSoftMinorVer,8);	//����ΰ汾��
	buffer.Read(m_xGimHeadProp.m_sMajorVersion,8);	//��׼���汾��
	buffer.Read(m_xGimHeadProp.m_sMinorVersion,8);	//��׼�ΰ汾��
	buffer.Read(m_xGimHeadProp.m_sBufSize,8);		//�洢�����С
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
	//��ȡͷ����Ϣ
	FILE* fp=fopen(gim_file,"rb");
	if(fp==NULL)
	{
		logerr.Log("GIM�ļ���ȡʧ��!");
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
	//��ѹ������DEV
	CString sCmd;
	CXhChar100 sOutPath("%s\\%s",(char*)m_sGimPath,(char*)m_sGimName);
	Manager.DisplayProcess(0,"����DEV�ļ�....");
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
		Manager.DisplayProcess(ftoi(100*index/nFileNum),"����DEV�ļ�....");
		CString fdPath=xDevFileList[index];
		CString str_ext=fdPath.Right(4);	//ȡ��׺��
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
	Manager.DisplayProcess(100,"����DEV�ļ�....");
	//��ѹ������FAM�ļ�
	CString file_str;
	nFileNum=m_xDevList.GetNodeNum();
	if(nFileNum<=0)
		return FALSE;
	Manager.DisplayProcess(0,"����FAM�ļ�....");
	for(CTMDHGim* pGim=m_xDevList.GetFirst();pGim;pGim=m_xDevList.GetNext())
		file_str.Append(CXhChar50("DEV\\%s.fam ",(char*)pGim->m_sFamGuid));
	sCmd.Format("7z.exe x %s %s -o%s -aoa",gim_file,file_str.GetBuffer(),(char*)sOutPath);
	if(!CTMDHManager::Run7zCmd(sCmd.GetBuffer()))
		return FALSE;
	index=1;
	for(CTMDHGim* pGim=m_xDevList.GetFirst();pGim;pGim=m_xDevList.GetNext(),index++)
	{
		Manager.DisplayProcess(ftoi(100*index/nFileNum),"����FAM�ļ�....");
		sFilePath=sOutPath;
		sFilePath.Append("\\DEV");
		CXhChar200 sFamFile("%s\\%s.fam",(char*)sFilePath,(char*)pGim->m_sFamGuid);
		pGim->ReadFamFile(sFamFile);
	}
	Manager.DisplayProcess(100,"����FAM�ļ�....");
	//��ѹ������PHM�ļ�
	file_str.Empty();
	Manager.DisplayProcess(0,"����PHM�ļ�....");
	for(CTMDHGim* pGim=m_xDevList.GetFirst();pGim;pGim=m_xDevList.GetNext())
		file_str.Append(CXhChar50("PHM\\%s.phm ",(char*)pGim->m_sPhmGuid));
	sCmd.Format("7z.exe x %s %s -o%s -aoa",gim_file,file_str.GetBuffer(),(char*)sOutPath);
	if(!CTMDHManager::Run7zCmd(sCmd.GetBuffer()))
		return FALSE;
	index=1;
	for(CTMDHGim* pGim=m_xDevList.GetFirst();pGim;pGim=m_xDevList.GetNext(),index++)
	{
		Manager.DisplayProcess(ftoi(100*index/nFileNum),"����PHM�ļ�....");
		sFilePath=sOutPath;
		sFilePath.Append("\\PHM");
		CXhChar200 sPhmFile("%s\\%s.phm",(char*)sFilePath,(char*)pGim->m_sPhmGuid);
		pGim->ReadPhmFile(sPhmFile);
	}
	Manager.DisplayProcess(100,"����PHM�ļ�....");
	//��ѹ������MOD�ļ�
	file_str.Empty();
	for(CTMDHGim* pGim=m_xDevList.GetFirst();pGim;pGim=m_xDevList.GetNext())
		file_str.Append(CXhChar50("MOD\\%s.mod ",(char*)pGim->m_sModName));
	sCmd.Format("7z.exe x %s %s -o%s -aoa",gim_file,file_str.GetBuffer(),(char*)sOutPath);
	if(!CTMDHManager::Run7zCmd(sCmd.GetBuffer()))
		return FALSE;
	index=1;
	for(CTMDHGim* pGim=m_xDevList.GetFirst();pGim;pGim=m_xDevList.GetNext(),index++)
	{
		Manager.DisplayProcess(ftoi(100*index/nFileNum),"����MOD�ļ�....");
		pGim->m_sModFile.Printf("%s\\MOD\\%s.mod",(char*)sOutPath,(char*)pGim->m_sModName);
		if (Manager.m_pModModel == NULL)
			Manager.m_pModModel = CModModelFactory::CreateModModel();
	}
	Manager.DisplayProcess(100,"����MOD�ļ�....");
	//ɾ����ʱ�ļ���
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
	//��дFAM�ļ�
	CXhChar100 sFilePath=m_sGimPath;
	sFilePath.Append("\\");
	sFilePath.Append(m_sGimName);
	sFilePath.Append("\\DEV");
	for(CTMDHGim* pGim=m_xDevList.GetFirst();pGim;pGim=m_xDevList.GetNext())
	{
		CXhChar200 sFamFilePath("%s\\%s.fam",(char*)sFilePath,(char*)pGim->m_sFamGuid);
		pGim->WriteFamFile(sFamFilePath);
	}
	//����GIM������
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
		logerr.Log("��������ʧ��");
		return FALSE;
	}
	//�ȴ����̽���
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
	Sheets       excel_sheets;	//��������
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
	//���������ļ���
	VARIANT value;
	CHashStrList<CXhChar100> hashModelFile;
	for(int i=1;i<sheet2ContentMap.RowsCount();i++)
	{
		//����
		sheet2ContentMap.GetValueAt(i,0,value);
		if(value.vt==VT_EMPTY)
			continue;
		CXhChar100 sTowerType=VariantToString(value);
		//�ļ�����
		sheet2ContentMap.GetValueAt(i,1,value);
		CXhChar100 sFileName=VariantToString(value);
		//�ļ�·��
		sheet2ContentMap.GetValueAt(i,2,value);
		CXhChar100 sPath=VariantToString(value);
		//
		hashModelFile.SetValue(sTowerType,CXhChar100("%s\\%s",(char*)sPath,(char*)sFileName));
	}
	//�������������
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
		xMyErrLog.Log("�ļ���ʽ����");
		return FALSE;
	}
	//��ȡ�ļ�����ȡ����
	DisplayProcess(0,"��ȡ���������Exel�ļ�,����ģ�ͽ���");
	int nRows=sheet1ContentMap.RowsCount();
	for(int i=1;i<=nRows;i++)
	{	
		DisplayProcess(ftoi(100*i/nRows),"��ȡ���������Exel�ļ�,����ģ�ͽ���");
		//���
		DWORD *pColIndex=hashColIndexByColTitle.GetValue(T_NO);
		sheet1ContentMap.GetValueAt(i,*pColIndex,value);
		if(value.vt==VT_EMPTY)
			continue;
		//��λ��
		pColIndex=hashColIndexByColTitle.GetValue(T_TOWER_NUM);
		sheet1ContentMap.GetValueAt(i,*pColIndex,value);
		CXhChar16 sTowerNum=VariantToString(value);
		//��λ��
		pColIndex=hashColIndexByColTitle.GetValue(T_TOWER_DIAN);
		sheet1ContentMap.GetValueAt(i,*pColIndex,value);
		CXhChar16 sTowerDian=VariantToString(value);
		//����
		pColIndex=hashColIndexByColTitle.GetValue(T_TOWER_TYPE);
		sheet1ContentMap.GetValueAt(i,*pColIndex,value);
		CXhChar50 sTowerType=VariantToString(value);
		//��λ��
		pColIndex=hashColIndexByColTitle.GetValue(T_LOCATE_HIGH);
		sheet1ContentMap.GetValueAt(i,*pColIndex,value);
		CXhChar16 sLocateH=VariantToString(value);
		//��·ת��
		pColIndex=hashColIndexByColTitle.GetValue(T_WIRE_ANGLE);
		sheet1ContentMap.GetValueAt(i,*pColIndex,value);
		CXhChar50 sWireAngle=VariantToString(value);
		//���Ƹ�
		pColIndex=hashColIndexByColTitle.GetValue(T_HUGAO_HEIGHT);
		sheet1ContentMap.GetValueAt(i,*pColIndex,value);
		double fHuGao=atof(VariantToString(value));
		CXhChar50 sHuGao=VariantToString(value);
		sHuGao.Append("m");
		//�����
		pColIndex=hashColIndexByColTitle.GetValue(T_BODY_HEIGHTP);
		sheet1ContentMap.GetValueAt(i,*pColIndex,value);
		double fBodyHeight=atof(VariantToString(value));
		//���ȳ���
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
		//��ȡģ���ļ�������ȡ����
		CTMDHModel* pModel=AppendModel();
		strcpy(pModel->m_sTowerNum,sTowerNum);
		strcpy(pModel->m_sTowerDot,sTowerDian);
		strcpy(pModel->m_sTowerType,sTowerType);
		strcpy(pModel->m_sHuGao,sHuGao);
		pModel->m_fLocateH=atof(sLocateH);	//��λ��
		pModel->m_fBodyHeight=fBodyHeight;	//�����
		if(strstr(sWireAngle,"��"))
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
	DisplayProcess(100,"��ȡ���������Exel�ļ�,����ģ�ͽ���");
	//��ʾ��ȡ�����Ϣ
	int nSumNum=ModelNum(),nErrNum=0;
	for(CTMDHModel* pModel=EnumFirstModel();pModel;pModel=EnumNextModel())
	{
		if(pModel->m_ciErrorType>0)
			nErrNum++;
	}
	int nRightNum=nSumNum-nErrNum;
	xMyErrLog.Log(CXhChar100("��ȡ��λ����%d������ȡ�ɹ�%d������ȡʧ��%d��",nSumNum,nRightNum,nErrNum));
	//
	for(CTMDHModel* pModel=EnumFirstModel();pModel;pModel=EnumNextModel())
	{
		if(pModel->m_ciErrorType==0)
			continue;
		if(pModel->m_ciErrorType==CTMDHModel::ERROR_NO_FILE)
			xMyErrLog.Log("����%s-%s�Ҳ�����Ӧ���ļ�!",(char*)pModel->m_sTowerNum,(char*)pModel->m_sTowerType);
		else if(pModel->m_ciErrorType==CTMDHModel::ERROR_READ_FILE)
			xMyErrLog.Log("����%s-%s��Ӧ���ļ���ȡʧ��!",(char*)pModel->m_sTowerNum,(char*)pModel->m_sTowerType);
		else if(pModel->m_ciErrorType==CTMDHModel::ERROR_FIND_HUGAO)
			xMyErrLog.Log("����%s-%s�в�����%s����!",(char*)pModel->m_sTowerNum,(char*)pModel->m_sTowerType,(char*)pModel->m_sHuGao);
		else if(pModel->m_ciErrorType==CTMDHModel::ERROR_FIND_LEG)
			xMyErrLog.Log("����%s-%s��%s������������Ϣ����!",(char*)pModel->m_sTowerNum,(char*)pModel->m_sTowerType,(char*)pModel->m_sHuGao);
		else if(pModel->m_ciErrorType==CTMDHModel::ERROR_FIND_INSTANCE)
			xMyErrLog.Log("����%s-%s�������Ϣ����!",(char*)pModel->m_sTowerNum,(char*)pModel->m_sTowerType);
	}
	return TRUE;
}
BOOL CTMDHManager::ParseTaModSheetContent(CVariant2dArray &sheetContentMap)
{
	if(sheetContentMap.RowsCount()<1)
		return FALSE;
	m_xGimDataList.Empty();
	//��һ�����м�¼������Ϣ
	GIM_HEAD_PROP_ITEM head_info;
	char sModPath[MAX_PATH]={0},sOutPath[MAX_PATH]={0};
	VARIANT value;
	sheetContentMap.GetValueAt(1,0,value);
	strcpy(sModPath,VariantToString(value));	//MOD·��
	sheetContentMap.GetValueAt(1,1,value);
	strcpy(sOutPath,VariantToString(value));	//���·��
	sheetContentMap.GetValueAt(1,2,value);
	strcpy(head_info.m_sDesigner,VariantToString(value));//�����
	sheetContentMap.GetValueAt(1,3,value);
	strcpy(head_info.m_sUnit,VariantToString(value));	//��֯��λ
	sheetContentMap.GetValueAt(1,4,value);
	strcpy(head_info.m_sSoftMajorVer,VariantToString(value));	//������汾
	sheetContentMap.GetValueAt(1,5,value);
	strcpy(head_info.m_sSoftMinorVer,VariantToString(value));	//����ΰ汾
	sheetContentMap.GetValueAt(1,6,value);
	strcpy(head_info.m_sMajorVersion,VariantToString(value));	//��׼���汾
	sheetContentMap.GetValueAt(1,7,value);
	strcpy(head_info.m_sMinorVersion,VariantToString(value));	//��׼�ΰ汾
	if(strlen(sModPath)<=0 || strlen(sOutPath)<=0)
		return FALSE;
	//�����м�¼�����У�����������¼�����
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
	DisplayProcess(0,"��ȡ�������Exel�ļ�,����ģ�ͽ���");
	int nRows=sheetContentMap.RowsCount();
	for(int i=3;i<=nRows;i++)
	{
		DisplayProcess(ftoi(100*i/nRows),"��ȡ�������Exel�ļ�,����ģ�ͽ���");
		VARIANT value;
		//MOD�ļ�����
		pColIndex=hashColIndexByColTitle.GetValue(T_MOD_NAME1);
		if(pColIndex==NULL)
			pColIndex=hashColIndexByColTitle.GetValue(T_MOD_NAME2);
		if(pColIndex==NULL)
		{
			logerr.Log("��ȡ��Excle�ļ���ģ���ļ���һ��!");
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
		//GIMͷ��Ϣ
		strcpy(pGimInfo->m_xGimHeadProp.m_sFileName,pGimInfo->m_sModName);
		strcpy(pGimInfo->m_xGimHeadProp.m_sDesigner,head_info.m_sDesigner);
		strcpy(pGimInfo->m_xGimHeadProp.m_sUnit,head_info.m_sUnit);
		strcpy(pGimInfo->m_xGimHeadProp.m_sSoftMajorVer,head_info.m_sSoftMajorVer);
		strcpy(pGimInfo->m_xGimHeadProp.m_sSoftMinorVer,head_info.m_sSoftMinorVer);
		strcpy(pGimInfo->m_xGimHeadProp.m_sMajorVersion,head_info.m_sMajorVersion);
		strcpy(pGimInfo->m_xGimHeadProp.m_sMinorVersion,head_info.m_sMinorVersion);
		//��ѹ�ȼ�
		pColIndex=hashColIndexByColTitle.GetValue(T_VOLTGRADE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_sVoltGrade=VariantToString(value);
		//�ͺ�
		pColIndex=hashColIndexByColTitle.GetValue(T_TYPE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_sType=VariantToString(value);
		//��������
		pColIndex=hashColIndexByColTitle.GetValue(T_TEXTURE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_sTexture=VariantToString(value);
		//�̶���ʽ
		pColIndex=hashColIndexByColTitle.GetValue(T_FIXEDTYPE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_sFixedType=VariantToString(value);
		//��������
		pColIndex=hashColIndexByColTitle.GetValue(T_TATYPE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_sTaType=VariantToString(value);
		//��·��
		pColIndex=hashColIndexByColTitle.GetValue(T_CIRCUIT);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_nCircuit=atoi(VariantToString(value));
		//�����ͺ�
		pColIndex=hashColIndexByColTitle.GetValue(T_CWIRESPEC);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_sCWireSpec=VariantToString(value);
		//�����ͺ�
		pColIndex=hashColIndexByColTitle.GetValue(T_EWIRESPEC);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_sEWireSpec=VariantToString(value);
		//��ƻ�������
		pColIndex=hashColIndexByColTitle.GetValue(T_WINDSPEED);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_fWindSpeed=atof(VariantToString(value));
		//��Ƹ������
		pColIndex=hashColIndexByColTitle.GetValue(T_NICETHICK);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_fNiceThick=atof(VariantToString(value));
		//���ˮƽ����
		pColIndex=hashColIndexByColTitle.GetValue(T_WINDSPAN);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_sWindSpan=VariantToString(value);
		//��ƴ�ֱ����
		pColIndex=hashColIndexByColTitle.GetValue(T_WEIGHTSPAN);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_sWeightSpan=VariantToString(value);
		//���ǰ�������
		pColIndex=hashColIndexByColTitle.GetValue(T_FRONTRULINGSPAN);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_fFrontRulingSpan=atof(VariantToString(value));
		//��ƺ�������
		pColIndex=hashColIndexByColTitle.GetValue(T_BACKRULINGSPAN);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_fBackRulingSpan=atof(VariantToString(value));
		//�����󵵾�
		pColIndex=hashColIndexByColTitle.GetValue(T_MAXSPAN);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_fMaxSpan=atof(VariantToString(value));
		//���ת�Ƿ�Χ
		pColIndex=hashColIndexByColTitle.GetValue(T_ANGLERANGE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_sAngleRange=VariantToString(value);
		//���Kvֵ
		pColIndex=hashColIndexByColTitle.GetValue(T_DESIGNKV);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_fDesignKV=atof(VariantToString(value));
		//�������
		pColIndex=hashColIndexByColTitle.GetValue(T_RATEDHEIGHT);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_sRatedHeight=VariantToString(value);
		//���߷�Χ
		pColIndex=hashColIndexByColTitle.GetValue(T_HEIGHTRANGE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_sHeightRange=VariantToString(value);
		//����
		pColIndex=hashColIndexByColTitle.GetValue(T_TOWERWEIGHT);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_sTowerWeight=VariantToString(value);
		//��ƹ�Ƶҡ�ڽǶ�
		pColIndex=hashColIndexByColTitle.GetValue(T_FREQUENCYROCKANGLE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_fFrequencyRockAngle=atof(VariantToString(value));
		//����׵�ҡ�ڽǶ�
		pColIndex=hashColIndexByColTitle.GetValue(T_LIGHTNINGROCKANGLE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_fLightningRockAngle=atof(VariantToString(value));
		//��Ʋ���ҡ�ڽǶ�
		pColIndex=hashColIndexByColTitle.GetValue(T_SWITCHINGROCKANGLE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_fSwitchingRockAngle=atof(VariantToString(value));
		//��ƴ�����ҵҡ�ڽǶ�
		pColIndex=hashColIndexByColTitle.GetValue(T_WORKINGROCKANGLE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_fWorkingRockAngle=atof(VariantToString(value));
		//��������
		pColIndex=hashColIndexByColTitle.GetValue(T_MANUFACTURER);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_sManuFacturer=VariantToString(value);
		//���ʱ���
		pColIndex=hashColIndexByColTitle.GetValue(T_MATERIALCODE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_sMaterialCode=VariantToString(value);
		//װ��ģ�ͱ��
		pColIndex=hashColIndexByColTitle.GetValue(T_PROMODELCODE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pGimInfo->m_xTowerProp.m_sProModelCode=VariantToString(value);
	}
	DisplayProcess(100,"��ȡ�������Exel�ļ�,����ģ�ͽ���");
	return TRUE;
}
BOOL CTMDHManager::ReadTaModBatchGzipFile(const char* sFilePath)
{
	CExcelOperObject xExcelObj;
	if(!xExcelObj.OpenExcelFile(sFilePath))
		return FALSE;
	LPDISPATCH pWorksheets=xExcelObj.GetWorksheets();
	ASSERT(pWorksheets != NULL);
	Sheets       excel_sheets;	//��������
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
	//1.��ȡExcelָ��Sheet���ݴ洢��sheetContentMap
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
		//2����������
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
