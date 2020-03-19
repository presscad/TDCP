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
//Exce����
static const int LOFTING_EXCEL_COL_COUNT  =12;
static const char* T_NO							= "���";
static const char* T_TOWER_NUM					= "��λ��";
static const char* T_TOWER_DIAN					= "��λ��";
static const char* T_TOWER_TYPE					= "����";
static const char* T_LOCATE_HIGH				= "��λ��";
static const char* T_WIRE_ANGLE					= "��·ת��";
static const char* T_HUGAO_HEIGHT				= "���Ƹ�";
static const char* T_BODY_HEIGHTP				= "�����";
static const char* T_LEG_LENGTH					= "���ȳ���";
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
//���������Ҫ����תʱ����Ҫ������ģ��ˮƽ��ת180��
//������ģ������ϵ�е�X���Y��*-1
GECS CTowerModel::BuildRotateCS()
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
GECS CTowerModel::BuildTransCS(void* pInstance/*=NULL*/)
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
/*���������޵Ķ��չ�ϵ
   ����		  ����ģ��������	��תģ��������
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
			CXhChar50 szError("�������ô���:"),szErrLeg;
			if(iLegASerial==0)
				szErrLeg.Printf("%gm",fLegA);
			if(iLegBSerial==0&&fLegB!=fLegA)
				szErrLeg.Append(CXhChar16("%gm",fLegB),',');
			if(iLegCSerial==0&&fLegC!=fLegB&&fLegC!=fLegA)
				szErrLeg.Append(CXhChar16("%gm",fLegC),',');
			if(iLegDSerial==0&&fLegD!=fLegC&&fLegD!=fLegB&&fLegD!=fLegA)
				szErrLeg.Append(CXhChar16("%gm",fLegD),',');
			logerr.Log("�������ô���:%s",(char*)szErrLeg);
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
			CXhChar50 szError("�������ô���:"),szErrLeg;
			if(pLegASerial==0)
				szErrLeg.Printf("%gm",fLegA);
			if(pLegBSerial==0&&fLegB!=fLegA)
				szErrLeg.Append(CXhChar16("%gm",fLegB),',');
			if(pLegCSerial==0&&fLegC!=fLegB&&fLegC!=fLegA)
				szErrLeg.Append(CXhChar16("%gm",fLegC),',');
			if(pLegDSerial==0&&fLegD!=fLegC&&fLegD!=fLegB&&fLegD!=fLegA)
				szErrLeg.Append(CXhChar16("%gm",fLegD),',');
			logerr.Log("�������ô���:%s",(char*)szErrLeg);
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
	//��ʼ��XMLģ������
	CXmlModel xXmlModel(BelongManager()->m_bUseUnitM);
	xXmlModel.m_sCategory.Copy("����");
	xXmlModel.m_sTowerName=GetFileName(pInstance);
	if(BelongManager()->m_ciInstanceLevel==0)
		xXmlModel.m_sTowerFile.Printf("%s.dxf",(char*)xXmlModel.m_sTowerName);
	else
		xXmlModel.m_sTowerFile.Printf("%s.3ds",(char*)xXmlModel.m_sTowerName);
	xXmlModel.m_ciModel=0;
	xXmlModel.m_sPropName=sName;
	GEPOINT xBaseLocPt[4], xLocation;	//�������޻�ȡ��������
	for(int i=0;i<4;i++)
	{
		int iHeightGroupSeial=pHeightGroup->GetSerialId();
		BYTE ciLegSerial=(BYTE)pInstance->GetLegSerialIdByQuad(i+1);
		BelongManager()->m_pTidModel->GetSubLegBaseLocation(iHeightGroupSeial,ciLegSerial,xLocation);
		if(i==1||i==3)	 //���1����Y��Գ�
			xLocation.x*=-1.0;
		if(i==2||i==3)	//���1����X��Գ�
			xLocation.y*=-1.0;
		if(m_bTurnLeft)
			xLocation=rot_cs.TransPToCS(xLocation);
		xLocation=trans_cs.TransPToCS(xLocation);
		xBaseLocPt[i] = xLocation;
	}
	//������˳���������Ϊ����˳������
	//A�Ȼ�������
	xXmlModel.m_xFundation[0].m_xFeetPos = xBaseLocPt[1];
	if (m_bTurnLeft)
		xXmlModel.m_xFundation[0].m_xFeetPos = xBaseLocPt[2];
	//B�Ȼ���
	xXmlModel.m_xFundation[1].m_xFeetPos = xBaseLocPt[3];
	if (m_bTurnLeft)
		xXmlModel.m_xFundation[1].m_xFeetPos = xBaseLocPt[0];
	//C�Ȼ���
	xXmlModel.m_xFundation[2].m_xFeetPos = xBaseLocPt[2];
	if (m_bTurnLeft)
		xXmlModel.m_xFundation[2].m_xFeetPos = xBaseLocPt[1];
	//D�Ȼ���
	xXmlModel.m_xFundation[3].m_xFeetPos = xBaseLocPt[0];
	if (m_bTurnLeft)
		xXmlModel.m_xFundation[3].m_xFeetPos = xBaseLocPt[3];
	//��ʼ��������Ϣ
	int nHangPt=BelongManager()->m_pTidModel->HangPointCount();
	for(int i=0;i<nHangPt;i++)
	{	//����
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
		{	//���ߵ������
			HANG_POINT* pHangPt = pHangPhase->m_xWireJHangPtArr.append();
			pHangPt->m_ciHangingStyle = pTidHangPt->GetHangingStyle();
			pHangPt->m_ciWireType = 'J';
			pHangPt->m_xHangPos = pos;
		}
		else
		{	//������
			if (ciTensionType == 1)
			{	//����,�ҵ㲻��ǰ��,ÿ����ֻ��һ������ҵ�
				HANG_POINT* pHangPt = pHangPhase->m_xBackHangPtArr.append();
				pHangPt->m_ciHangingStyle = pTidHangPt->GetHangingStyle();
				pHangPt->m_ciWireType = 'C';
				if (strstr(sDes, "ǰ"))
					pHangPt->m_ciHangDirect = 'Q';
				else
					pHangPt->m_ciHangDirect = 'H';
				pHangPt->m_xHangPos = pos;
				pHangPt->m_ciTensionType = 0;
			}
			else
			{	//���ţ��ҵ�����ǰ��࣬ÿ��������������ҵ�
				if (strstr(sDes, "ǰ"))
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
	{	//����
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
			if (strstr(sDes, "ǰ"))
				pHangPt->m_ciHangDirect = 'Q';
			else if (strstr(sDes, "��"))
				pHangPt->m_ciHangDirect = 'H';
		}
		else
		{
			if (strstr(sDes, "ǰ"))
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
	//����XML�ļ�
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
	xXmlModel.m_sCategory.Copy("����");
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
	//��ʼ���ҵ���Ϣ
	BYTE ciHangType=0;
	int nHangPt=BelongManager()->m_pModModel->GetHangNodeNum();
	for(int i=0;i<nHangPt;i++)
	{	//����
		MOD_HANG_NODE* pModHangPt=BelongManager()->m_pModModel->GetHangNodeById(i);
		if(pModHangPt->m_ciWireType=='E')
			continue;
		if(strstr(pModHangPt->m_sHangName,"ǰ") && ciHangType==0)
			ciHangType=1;
	}
	for(int i=0;i<nHangPt;i++)
	{
		//����
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
		{	//����,�ҵ㲻��ǰ��,ÿ����ֻ��һ������ҵ�
			HANG_POINT* pHangPt = pHangPhase->m_xBackHangPtArr.append();
			pHangPt->m_ciHangingStyle = pModHangPt->m_ciHangingStyle;
			pHangPt->m_ciWireType = 'C';
			if (strstr(pModHangPt->m_sHangName, "ǰ"))
				pHangPt->m_ciHangDirect = 'Q';
			else
				pHangPt->m_ciHangDirect = 'H';
			pHangPt->m_xHangPos = pos;
			pHangPt->m_ciTensionType = 0;
		}
		else
		{	//���ţ��ҵ�����ǰ��࣬ÿ��������������ҵ�
			if (strstr(pModHangPt->m_sHangName, "ǰ"))
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
	{	//����
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
			if (strstr(pModHangPt->m_sHangName, "ǰ"))
				pHangPt->m_ciHangDirect = 'Q';
			else //if (strstr(pModHangPt->m_sHangName, "��"))
				pHangPt->m_ciHangDirect = 'H';
		}
		else
		{
			if (strstr(pModHangPt->m_sHangName, "ǰ"))
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
	//����XML�ļ�
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
	//������άʵ��Ļ�����������Ϣ
	int nPart=pInstance->GetAssemblePartNum();
	int nBolt=pInstance->GetAssembleBoltNum();
	int nAnchorBolt=pInstance->GetAssembleAnchorBoltNum();
	int nSum=(ciLevel==3)?nPart+nBolt+nAnchorBolt:nPart;
	int serial=1;
	I3DSData* p3dsFile=C3DSFactory::Create3DSInstance();
	DisplayProcess(0,"����3DSģ��....");
	for(ITidAssemblePart* pAssmPart=pInstance->EnumAssemblePartFirst();pAssmPart;pAssmPart=pInstance->EnumAssemblePartNext(),serial++)
	{
		DisplayProcess(int(100*serial/nSum),"���ɹ���3DSģ��....");
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
	{	//�������𣬻�����˨
		int iBolt=0;
		for(ITidAssembleBolt* pAssmBolt=pInstance->EnumAssembleBoltFirst();pAssmBolt;pAssmBolt=pInstance->EnumAssembleBoltNext(),serial++,iBolt++)
		{
			DisplayProcess(int(100*serial/nSum),"������˨3DSģ��....");
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
			DisplayProcess(int(100*serial/nSum),"���ɽŶ�3DSģ��....");
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
	DisplayProcess(100,"����3DSģ��....");
	p3dsFile->Creat3DSFile(sFileName);
	//
	C3DSFactory::Destroy(p3dsFile->GetSerial());
	p3dsFile=NULL;
}
void CTowerModel::Create3dsFile(IModTowerInstance* pInstance,const char* sFileName)
{
	//������άʵ��Ļ�����������Ϣ
	UCS_STRU rot_cs=BuildRotateCS();
	I3DSData* p3dsFile=C3DSFactory::Create3DSInstance();
	int nIndex=1,nSum=pInstance->GetModRodNum();
	DisplayProcess(0,"����3DS�ļ�....");
	for(IModRod* pRod=pInstance->EnumModRodFir();pRod;pRod=pInstance->EnumModRodNext(),nIndex++)
	{
		DisplayProcess(int(100*nIndex/nSum),"����3DS�ļ�....");
		CSolidBody solidBody;
		pRod->Create3dSolidModel(&solidBody,FALSE);
		if(m_bTurnLeft)
			solidBody.TransToACS(rot_cs);
		p3dsFile->AddSolidPart(&solidBody,nIndex,CXhChar16("Rod%d",nIndex),BelongManager()->m_bUseUnitM);
	}
	DisplayProcess(100,"����3DS�ļ�....");
	p3dsFile->Creat3DSFile(sFileName);
	//
	C3DSFactory::Destroy(p3dsFile->GetSerial());
	p3dsFile=NULL;
}
void CTowerModel::CreateDxfFile(ITidTowerInstance* pInstance,const char* sFileName)
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
	//����DXF�ļ�
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
	//��ȡֱ�߼��ϣ�����ʼ������
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
	//����DXF�ļ�
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
	DisplayProcess(0,"��ȡExel�ļ�,����ģ�ͽ���");
	int nRows=sheet1ContentMap.RowsCount();
	for(int i=1;i<=nRows;i++)
	{	
		DisplayProcess(int(100*i/nRows),"��ȡExel�ļ�,����ģ�ͽ���");
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
		m_pActiveTa=m_xTaModelList.append();
		m_pActiveTa->SetManager(this);
		m_pActiveTa->m_sTowerPlaceCode.Copy(sTowerNum);
		m_pActiveTa->m_sTowerDot.Copy(sTowerDian);
		m_pActiveTa->m_sTowerType.Copy(sTowerType);
		m_pActiveTa->m_sHuGao.Copy(sHuGao);
		m_pActiveTa->m_fLocateH=atof(sLocateH);	//��λ��
		m_pActiveTa->m_fBodyHeight=fBodyHeight;	//�����
		if(strstr(sWireAngle,"��"))
			m_pActiveTa->m_bTurnLeft=TRUE;
		m_pActiveTa->m_fMaxLegHeight=fHuGao-fBodyHeight;
		CXhChar100* pFullFilePath=hashModelFile.GetValue(sTowerType);
		if(pFullFilePath==NULL)
		{	//�Ҳ������Ͷ�Ӧ���ļ�
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
	DisplayProcess(100,"��ȡExel�ļ�,����ģ�ͽ���");
	//��ʾ��ȡ�����Ϣ
	int nSumNum=GetTaModelNum(),nErrNum=0;
	for(CTowerModel* pTower=EnumFirstTaModel();pTower;pTower=EnumNextTaModel())
	{
		if(pTower->m_ciErrorType>0)
			nErrNum++;
	}
	int nRightNum=nSumNum-nErrNum;
	xMyErrLog.Log(CXhChar100("��ȡ��λ����%d������ȡ�ɹ�%d������ȡʧ��%d��",nSumNum,nRightNum,nErrNum));
	//
	for(CTowerModel* pTower=TaManager.EnumFirstTaModel();pTower;pTower=TaManager.EnumNextTaModel())
	{
		if(pTower->m_ciErrorType==0)
			continue;
		if(pTower->m_ciErrorType==CTowerModel::ERROR_NO_FILE)
			xMyErrLog.Log("����%s-%s�Ҳ�����Ӧ���ļ�!",(char*)pTower->m_sTowerPlaceCode,(char*)pTower->m_sTowerType);
		else if(pTower->m_ciErrorType==CTowerModel::ERROR_READ_FILE)
			xMyErrLog.Log("����%s-%s��Ӧ���ļ���ȡʧ��!",(char*)pTower->m_sTowerPlaceCode,(char*)pTower->m_sTowerType);
		else if(pTower->m_ciErrorType==CTowerModel::ERROR_FIND_HUGAO)
			xMyErrLog.Log("����%s-%s�в�����%s����!",(char*)pTower->m_sTowerPlaceCode,(char*)pTower->m_sTowerType,(char*)pTower->m_sHuGao);
		else if(pTower->m_ciErrorType==CTowerModel::ERROR_FIND_LEG)
			xMyErrLog.Log("����%s-%s��%s������������Ϣ����!",(char*)pTower->m_sTowerPlaceCode,(char*)pTower->m_sTowerType,(char*)pTower->m_sHuGao);
		else if(pTower->m_ciErrorType==CTowerModel::ERROR_FIND_INSTANCE)
			xMyErrLog.Log("����%s-%s�������Ϣ����!",(char*)pTower->m_sTowerPlaceCode,(char*)pTower->m_sTowerType);
	}
	return TRUE;
}
void CTowerManager::CreateXmlFiles(const char* sPath)
{
	DisplayProcess(0,"����XML�ļ�");
	int i=1,nModel=GetTaModelNum();
	for(CTowerModel* pTower=EnumFirstTaModel();pTower;pTower=EnumNextTaModel(),i++)
	{
		DisplayProcess(int(100*i/nModel),"����XML�ļ�");
		if(pTower->m_ciErrorType>0)
			continue;
		void* pInstance=pTower->ExtractActiveTaInstance();
		if(pInstance==NULL)
			continue;
		CXhChar200 sFullPath("%s\\%s.xml",sPath,(char*)pTower->GetFileName(pInstance));
		if(pTower->m_ciModelFlag==0)	//TIDģ��
			pTower->CreateXmlFile((ITidTowerInstance*)pInstance,sFullPath);
		if(pTower->m_ciModelFlag==1)	//MODģ��
			pTower->CreateXmlFile((IModTowerInstance*)pInstance,sFullPath);
	}
	DisplayProcess(100,"����XML�ļ�");
}
void CTowerManager::CreateModFiles(const char* sPath)
{
	DisplayProcess(0,"����MOD�ļ�");
	int i=1,nModel=GetTaModelNum();
	for(CTowerModel* pTower=EnumFirstTaModel();pTower;pTower=EnumNextTaModel(),i++)
	{
		DisplayProcess(int(100*i/nModel),"����MOD�ļ�");
		if(pTower->m_ciErrorType>0)
			continue;
		void* pInstance=pTower->ExtractActiveTaInstance();
		if(pInstance==NULL)
			continue;
		CXhChar200 sFullPath("%s\\%s.mod",sPath,(char*)pTower->GetFileName(pInstance));
		if(pTower->m_ciModelFlag==0)	//TIDģ��
			pTower->CreateModFile((ITidTowerInstance*)pInstance,sFullPath);
		if(pTower->m_ciModelFlag==1)	//MODģ��
			pTower->CreateModFile((IModTowerInstance*)pInstance,sFullPath);
	}
	DisplayProcess(100,"����MOD�ļ�");
}
void CTowerManager::Create3dsFiles(const char* sPath)
{
	DisplayProcess(0,"����3ds�ļ�");
	int i=1,nModel=GetTaModelNum();
	for(CTowerModel* pTower=EnumFirstTaModel();pTower;pTower=EnumNextTaModel(),i++)
	{
		DisplayProcess(int(100*i/nModel),"����3ds�ļ�");
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
			if(pTower->m_ciModelFlag==0)	//TIDģ��
				pTower->CreateDxfFile((ITidTowerInstance*)pInstance,sFullPath);
			if(pTower->m_ciModelFlag==1)	//MODģ��
				pTower->CreateDxfFile((IModTowerInstance*)pInstance,sFullPath);
		}
		else
		{
			if(pTower->m_ciModelFlag==0)	//TIDģ��
				pTower->Create3dsFile((ITidTowerInstance*)pInstance,sFullPath);
			if(pTower->m_ciModelFlag==1)	//MODģ��
				pTower->Create3dsFile((IModTowerInstance*)pInstance,sFullPath);
		}
	}
	DisplayProcess(100,"����3ds�ļ�");
}
