// LibraryQuery.cpp: implementation of the CLibraryQuery class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LibraryQuery.h"
#if defined(__LDS_CONTEXT_)||defined(__LDS_GROUP_CONTEXT_)
#include "env_def.h"
#endif
#include "Tower.h"
#include "LogFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLibraryQuery globalLibrary;
CLibraryQuery::CLibraryQuery()
{
	nAngleType = nTubeType = nFlatType = 0; 
	this->InitColor();
}
int CLibraryQuery::FindAngleTypeIndex(double wing_wide,double wing_thick,char cType)
{
	for(int i=0;i<nAngleType;i++)
	{
		if(jgguige_table[i].wing_wide==wing_wide&&jgguige_table[i].wing_thick==wing_thick&&jgguige_table[i].cType==cType)
			return i;
	}
	return -1;
}
int CLibraryQuery::FindTubeTypeIndex(double d,double t)
{
	for(int i=0;i<nTubeType;i++)
	{
		if(tubeguige_table[i].D==d&&tubeguige_table[i].thick==t)
			return i;
	}
	return -1;
}
int CLibraryQuery::FindFlatTypeIndex(double wide,double thick)
{
	for(int i=0;i<nTubeType;i++)
	{
		if(flatguige_table[i].wide==wide&&flatguige_table[i].thick==thick)
			return i;
	}
	return -1;
}
//��ѯ�Ǹ���Բ���뾶
double CLibraryQuery::FindAngleInnerR(double wide)
{
	return FindJg_r(wide);
}
JGZJ CLibraryQuery::GetAngleZJ(double wide)		//��ѯ�Ǹ�׼��
{
	JGZJ jgzj;
	jgzj.g=jgzj.g1=jgzj.g2=jgzj.g3=0;
	if(!getjgzj(jgzj,wide))
		jgzj.g=ftoi(wide/2);
	return jgzj;
}
double CLibraryQuery::GetPartArea(double size_para1,double size_para2,double size_para3,int idPartClsType,char cSubType/*='L'*/)
{
	if(idPartClsType==CLS_LINEANGLE||idPartClsType==CLS_GROUPLINEANGLE)
	{
		double wing_wide=size_para1;
		double wing_thick=size_para2;
		double wing_wide_y=size_para3;
		if(cSubType!='D'&&cSubType!='X'&&cSubType!='L')
			cSubType='L';	//��ֹ���ִ�������
		JG_PARA_TYPE *pJgPara=NULL;
		if(wing_wide_y<=0||wing_wide_y==wing_wide)
			pJgPara=FindJgType(wing_wide,wing_thick,cSubType);
		if(pJgPara)
			return pJgPara->area*100;
		else	//������趨��ֻ�ܼ������ֵ
		{
			if(wing_wide_y>0)
				return (wing_wide+wing_wide_y)*wing_thick-wing_thick*wing_thick;
			else
				return wing_wide*wing_thick*2-wing_thick*wing_thick;
		}
	}
	else if(idPartClsType==CLS_LINETUBE)
	{
		double d=size_para1;
		double t=size_para2;
		return Pi*(d*t-t*t);
	}
	else if(idPartClsType==CLS_LINEFLAT)
	{
		double w=size_para1,t=size_para2;
		return w*t;
	}
	else
		return 0;
}
double CLibraryQuery::GetWeightOf1Meter(double size_para1,double size_para2,double size_para3,int idPartClsType,char cSubType/*='L'*/)
{
	if(idPartClsType==CLS_LINEANGLE||idPartClsType==CLS_GROUPLINEANGLE)
	{
		double wing_wide=size_para1;
		double wing_thick=size_para2;
		double wing_wide_y=size_para3;
		JG_PARA_TYPE *pJgPara=NULL;
		if(wing_wide_y<=0||wing_wide_y==wing_wide)
			pJgPara=FindJgType(wing_wide,wing_thick,cSubType);
		if(pJgPara)
			return pJgPara->theroy_weight;
		else	//������趨��ֻ�ܼ������ֵ
		{
			double area=GetPartArea(size_para1,size_para2,size_para3,idPartClsType,cSubType);
			return area*7.85e-3;
		}
	}
	else if(idPartClsType==CLS_LINETUBE)
	{
		double area=GetPartArea(size_para1,size_para2,size_para3,idPartClsType,cSubType);
		return area*7.85e-3;
	}
	else if(idPartClsType==CLS_LINEFLAT)
	{
		double area=GetPartArea(size_para1,size_para2,size_para3,idPartClsType,cSubType);
		return area*7.85e-3;
	}
	else
		return 0;
}
double CLibraryQuery::GetWorkI(double size_para1,double size_para2,double size_para3,int axis,int idPartClsType,char cSubType/*='L'*/)
{
	if(idPartClsType==CLS_LINEANGLE||idPartClsType==CLS_GROUPLINEANGLE)
	{
		double wing_wide=size_para1;
		double wing_thick=size_para2;
		JG_PARA_TYPE *pJgPara=FindJgType(wing_wide,wing_thick,cSubType);
		if(pJgPara)
		{
			if(axis==1)
				return pJgPara->Iy0*10000;
			else
				return pJgPara->Ix*10000;
		}
		else	//������趨��ֻ�ܼ������ֵ
			return 0;
	}
	else if(idPartClsType==CLS_LINETUBE)
	{
		double wing_wide=size_para1;
		double wing_thick=size_para2;
		TUBE_PARA_TYPE *pTubePara=FindTubeType(wing_wide,wing_thick);
		if(pTubePara)
			return pTubePara->I*10000;
		else	//������趨��ֻ�ܼ������ֵ
			return 0;
	}
	else if(idPartClsType==CLS_LINEFLAT)
	{
		return 0;
	}
	else
		return 0;
}
//���ظ˼�ָ�����Ǹ�ʱpara=0ƽ�����para=1��С�ᣩ����ģ��(mm3)
double CLibraryQuery::GetWorkW(double size_para1,double size_para2,double size_para3,int axis,int idPartClsType,char cSubType/*='L'*/)
{
	if(idPartClsType==CLS_LINEANGLE||idPartClsType==CLS_GROUPLINEANGLE)
	{
		double wing_wide=size_para1;
		double wing_thick=size_para2;
		JG_PARA_TYPE *pJgPara=FindJgType(wing_wide,wing_thick,cSubType);
		if(pJgPara)
		{
			if(axis==1)
				return pJgPara->Wy0*1000;
			else
				return pJgPara->Wx*1000;
		}
		else	//������趨��ֻ�ܼ������ֵ
			return 0;
	}
	else if(idPartClsType==CLS_LINETUBE)
	{
		double wing_wide=size_para1;
		double wing_thick=size_para2;
		TUBE_PARA_TYPE *pTubePara=FindTubeType(wing_wide,wing_thick);
		if(pTubePara)
			return pTubePara->W*1000;
		else	//������趨��ֻ�ܼ������ֵ
			return 0;
	}
	else if(idPartClsType==CLS_LINEFLAT)
	{
		return 0;
	}
	else
		return 0;
}
double CLibraryQuery::GetWorkR(double size_para1,double size_para2,double size_para3,int axis,int idPartClsType,char cSubType/*='L'*/)
{
	if(idPartClsType==CLS_LINEANGLE||idPartClsType==CLS_GROUPLINEANGLE)
	{
		double wing_wide=size_para1;
		double wing_thick=size_para2;
		JG_PARA_TYPE *pJgPara=FindJgType(wing_wide,wing_thick,cSubType);
		if(pJgPara)
		{
			if(axis==1)
				return pJgPara->Ry0*10;
			else
				return pJgPara->Rx*10;
		}
		else	//������趨��ֻ�ܼ������ֵ
			return 0;
	}
	else if(idPartClsType==CLS_LINETUBE)
	{
		return 0;
	}
	else if(idPartClsType==CLS_LINEFLAT)
	{
		return 0;
	}
	else
		return 0;
}
double CLibraryQuery::GetW(double size_para1,double size_para2,double size_para3,int axis,int idPartClsType,char cSubType/*='L'*/)
{
	if(idPartClsType==CLS_LINEANGLE||idPartClsType==CLS_GROUPLINEANGLE)
	{
		double wing_wide=size_para1;
		double wing_thick=size_para2;
		JG_PARA_TYPE *pJgPara=FindJgType(wing_wide,wing_thick,cSubType);
		if(pJgPara)
		{
			if(axis==1)
				return pJgPara->Wy0*1000;
			else
				return pJgPara->Wx*1000;
		}
		else	//������趨��ֻ�ܼ������ֵ
			return 0;
	}
	else if(idPartClsType==CLS_LINETUBE)
	{
		return 0;
	}
	else if(idPartClsType==CLS_LINEFLAT)
	{
		return 0;
	}
	else
		return 0;
}
double CLibraryQuery::GetElasticityModulus(char cMaterial)
{
	STEELMAT *pSteel=QuerySteelMatByBriefMark(toupper(cMaterial));
	if(pSteel)
		return pSteel->E;
	else
		return 0;
}
STEELMAT* CLibraryQuery::QuerySteelMatByBriefMark(char briefmark)
{
	STEELMAT *pSteel=::QuerySteelMatByBriefMark(toupper(briefmark));
	return pSteel;
}
//����ϵͳ����������϶
UINT CLibraryQuery::GetHuoQuClearance()
{
#if defined(__ANGLE_PART_INC_)&&defined(__LDS_CONTEXT_)//defined(__TMA_)||defined(__LMA_)||defined(__LDS_)
	return g_sysPara.HuoQuDist;
#else
	return 10;
#endif
}

//���ϵͳ���������϶
UINT CLibraryQuery::GetVertexDist()
{
#if defined(__ANGLE_PART_INC_)&&defined(__LDS_CONTEXT_)//defined(__TMA_)||defined(__LMA_)||defined(__LDS_)
	return g_sysPara.VertexDist;
#else
	return 5;
#endif
}

//���ϵͳ����Ӧ����ɢ�� 
double CLibraryQuery::GetStressDiffuseAngle()
{
#if defined(__ANGLE_PART_INC_)&&defined(__LDS_CONTEXT_)//defined(__TMA_)||defined(__LMA_)||defined(__LDS_)
	return g_sysPara.fStressDiffuseAngle;
#else
	return 30;
#endif
}

#if !defined(__TSA_)&&!defined(__TSA_FILE_)
//���ݸֹ�ֱ������ȡ�����Ͷ˰����͵Ȳ�ѯ��׼��
bool CLibraryQuery::GetStdPartModel(double d,double t,char cMaterial,BYTE level,int std_part_type, void *pStdPartModelObj)
{
	CLDSStdPartModel *pStdPartModel=(CLDSStdPartModel*)pStdPartModelObj;
	if(std_part_type==TYPE_ROLLEND)
	{
		INSERT_PLATE_PARAM *pInsertPlateParam=FindRollEndParam(d,t,cMaterial,level);
		if(pInsertPlateParam==NULL)
			return false;
		pStdPartModel->m_iStdPartType=std_part_type;
		pStdPartModel->param.insert_plate_param=*pInsertPlateParam;
	}
	else if(std_part_type==TYPE_UEND)
	{
		INSERT_PLATE_PARAM *pInsertPlateParam=FindUEndParam(d,t,cMaterial,level);
		if(pInsertPlateParam==NULL)
			return false;
		pStdPartModel->m_iStdPartType=std_part_type;
		pStdPartModel->param.insert_plate_param=*pInsertPlateParam;
	}
	else if(std_part_type==TYPE_XEND)
	{
		INSERT_PLATE_PARAM *pInsertPlateParam=FindXEndParam(d,t,cMaterial,level);
		if(pInsertPlateParam==NULL)
			return false;
		pStdPartModel->m_iStdPartType=std_part_type;
		pStdPartModel->param.insert_plate_param=*pInsertPlateParam;
	}
	else if(std_part_type==TYPE_FLD||std_part_type==TYPE_FLP)
	{
		FL_COUPLE couple;
		if(std_part_type==TYPE_FLD)
		{
			if(!QueryFlCouple(d,d,'D',&couple))
				return false;
		}
		else //if(std_part_type==TYPE_FLP)
		{
			if(!QueryFlCouple(d,d,'P',&couple))
				return false;
		}
		FL_PARAM* pFLDPara=FindFlDParam(couple.mainFlName);
		if(pFLDPara==NULL)
			return false;
		pStdPartModel->m_iStdPartType=std_part_type;
		pStdPartModel->param.fl_param=*pFLDPara;
		pStdPartModel->param.fl_param.t=t;
		if(std_part_type==TYPE_FLD)
			pStdPartModel->param.fl_param.UpdateCodeName();
	}
	else if(std_part_type==TYPE_FLG)
	{
		FL_PARAM2 *pFlParam=FindFlGParam(d,t,level);
		if(pFlParam==NULL)
			return false;
		pStdPartModel->m_iStdPartType=std_part_type;
		pStdPartModel->param.fl_param2=*pFlParam;
	}
	else if(std_part_type==TYPE_FLR)
	{
		FL_PARAM2 *pFlParam=FindFlRParam(d,t,level);
		if(pFlParam==NULL)
			return false;
		pStdPartModel->m_iStdPartType=std_part_type;
		pStdPartModel->param.fl_param2=*pFlParam;
	}
	else
		return false;
	return true;
}
//�õ���׼����ģ��
bool CLibraryQuery::GetStdPartModel(char *sPartNo,int std_part_type, void *pStdPartModelObj,double t/*=0*/)
{
	CLDSStdPartModel *pStdPartModel=(CLDSStdPartModel*)pStdPartModelObj;
	pStdPartModel->m_iStdPartType=std_part_type;
	if(std_part_type==TYPE_FLD)
	{	//�Ժ�����
		for(int i=0;i<fld_param_N;i++)
		{
			if(fld_param_table[i].IsSameCode(sPartNo))	//�Ժ�����ʵ�ʱ���п��ܻẬ�����ܱں������Ϣ
			{
				pStdPartModel->param.fl_param=fld_param_table[i];
				pStdPartModel->param.fl_param.t=t;
				pStdPartModel->param.fl_param.UpdateCodeName();
				return TRUE;
			}
		}
	}
	else if(std_part_type==TYPE_FLP)
	{	//ƽ������
		for(int i=0;i<flp_param_N;i++)
		{
			if(stricmp(flp_param_table[i].codeName,sPartNo)==0)
			{	
				pStdPartModel->param.fl_param=flp_param_table[i];
				return TRUE;
			}
		}
	}
	else if(std_part_type==TYPE_FLG)
	{	//���Է���
		for(int i=0;i<flg_param_N;i++)
		{
			if(stricmp(flg_param_tbl[i].codeName,sPartNo)==0)
			{	
				pStdPartModel->param.fl_param2=flg_param_tbl[i];
				return TRUE;
			}
		}
	}
	else if(std_part_type==TYPE_FLR)
	{	//���Է���
		for(int i=0;i<flr_param_N;i++)
		{
			if(stricmp(flr_param_tbl[i].codeName,sPartNo)==0)
			{	
				pStdPartModel->param.fl_param2=flr_param_tbl[i];
				return TRUE;
			}
		}
	}
	else if(std_part_type==TYPE_ROLLEND)
	{	//���Ͳ��
		for(int i=0;i<rollend_param_N;i++)
		{
			if(stricmp(rollend_param_table[i].codeName,sPartNo)==0)
			{
				pStdPartModel->param.insert_plate_param=rollend_param_table[i];
				return TRUE;
			}
			
		}
	}
	else if(std_part_type==TYPE_UEND)
	{	//U�Ͳ��
		for(int i=0;i<uend_param_N;i++)
		{
			if(stricmp(uend_param_table[i].codeName,sPartNo)==0)
			{
				pStdPartModel->param.insert_plate_param=uend_param_table[i];
				return TRUE;
			}
		}
	}
	else if(std_part_type==TYPE_XEND)
	{	//ʮ�ֲ��
		for(int i=0;i<xend_param_N;i++)
		{	//ʮ�ֲ��������׼�ְ��Ų���ͬ,ʹ�ù�����Ų���ʮ�ֲ�����ʱӦ�����������ŶԱ� wht 10-01-24
			//if(strcmp(xend_param_table[i].codeName,sPartNo)==0)
			if(stricmp(xend_param_table[i].datumPlatePartNo,sPartNo)==0
				||strcmp(xend_param_table[i].codeName,sPartNo)==0)
			{
				pStdPartModel->param.insert_plate_param=xend_param_table[i];
				return TRUE;
			}
		}
	}
	return false;
}

//�õ���Ȧ��� wht 10-12-24
int CLibraryQuery::GetBoltPadThick(int bolt_d)
{
#if defined(__LDS_CONTEXT_)||defined(__LDS_GROUP_CONTEXT_)
	if(bolt_d==12)
		return g_sysPara.BoltPadThick.m_nM12;
	else if(bolt_d==16)
		return g_sysPara.BoltPadThick.m_nM16;
	else if(bolt_d==20)
		return g_sysPara.BoltPadThick.m_nM20;
	else if(bolt_d==24)
		return g_sysPara.BoltPadThick.m_nM24;
	else 
#endif
		return 0;
}
//����ƥ�����˨����ͺ�
LS_XING_HAO* CLibraryQuery::FindLsGuiGe(int d, double L0,long hFamily/*=0*/)
{
	return CLsLibrary::FindLsGuiGe(d,L0,hFamily);
}
	//������˨ֱ������Ч������ƥ����˨�ĵ�������,kg
double CLibraryQuery::GetLsWeight(int d, double L,long hFamily/*=0*/)
{
	LS_XING_HAO *pXingHao=CLsLibrary::FindLsGuiGeByValidLen(d,L,hFamily);
	if(pXingHao)
		return pXingHao->weight;
	else
		return 0;
}
#endif
//�Ǹֿ��Ͻ���ֵ wht 11-05-06
double CLibraryQuery::GetJgKaiHeJiaoThreshold()
{
#if defined(__ANGLE_PART_INC_)&&defined(__LDS_CONTEXT_)//defined(__TMA_)||defined(__LMA_)||defined(__LDS_)||defined(__TMAGROUP_)||defined(__LMAGROUP_)||defined(__LDSGROUP_)
	return g_sysPara.m_fKaiHeJiaoThreshold;
#endif
	return 2;
}
double CLibraryQuery::GetJgKaiHeJiaoMinDeita()
{
#if defined(__ANGLE_PART_INC_)&&defined(__LDS_CONTEXT_)
	return g_sysPara.m_fKaiHeJiaoMinDeita;
#endif
	return 20;
}
double CLibraryQuery::GetMaxKaiHeJiaoClearance()
{
#if defined(__ANGLE_PART_INC_)&&defined(__LDS_CONTEXT_)
	return g_sysPara.m_fMaxKaiHeJiaoClearance;
#endif
	return (CLDSPart::CustomerSerial==6)?3:2;
}
double CLibraryQuery::GetMinCutAngleLen()
{
#if defined(__ANGLE_PART_INC_)&&defined(__LDS_CONTEXT_)
	return g_sysPara.m_fMinCutAngleLen;
#endif
	return 5;
}
double CLibraryQuery::GetMaxIgnoreCutAngleLen()
{
#if defined(__ANGLE_PART_INC_)&&defined(__LDS_CONTEXT_)
	return g_sysPara.m_fMaxIgnoreCutAngleLen;
#endif
	return 2;
}
double CLibraryQuery::GetCutAngleCoefR()
{
#if defined(__ANGLE_PART_INC_)&&defined(__LDS_CONTEXT_)
	return g_sysPara.m_fCutAngleCoefR;
#endif
	return 2.0/3.0;
}
int CLibraryQuery::GetProfileAlgorithm()
{
#if defined(__ANGLE_PART_INC_)&&defined(__LDS_CONTEXT_)
	return CLDSPlate::m_siProfileAlgor;
#endif
	return 0;
}
void CLibraryQuery::FindLsBoltDH(int d, double *D,double *H,double *H2)
{
//TAP����Ҫ�õ��ò�������ʱ�򿪽�����ͳ�￼�ǡ�wjh-2014.9.1
//#if defined(__ANGLE_PART_INC_)&&defined(__LDS_CONTEXT_)
	FindLsD_H(d,D,H,H2);
//#endif
}

#if defined(__TSA_)||defined(__LDS_)
#include "WireLoadDatabase.h"
void GetSysPath(char* App_Path,const char *src_path=NULL,char *file_name=NULL,char *extension=NULL);//���ϵͳ����·����App_PathӦ�ں����⿪�ٺ��ڴ�
//����ר�ò�ѯ����
double CLibraryQuery::GetWireUnitWeightPerKM(char* wireType,char* wireSpec)	//��λ������,kg/km
{
	CWireLoadDatabase db;
	char file_path[MAX_PATH];
	GetSysPath(file_path,NULL,NULL,NULL);
	strcat(file_path,"wireload.d");
	if(db.OpenDbFile(file_path))
	{
		DIANXIAN_STRU wire;
		if(db.QueryWire(wireType,wireSpec,&wire))
			return wire.m_fUnitWeight;
	}
	return 0;
}
double CLibraryQuery::GetWireArea(char* wireType,char* wireSpec)		//���½����,mm2
{
	CWireLoadDatabase db;
	char file_path[MAX_PATH];
	GetSysPath(file_path,NULL,NULL,NULL);
	strcat(file_path,"wireload.d");
	if(db.OpenDbFile(file_path))
	{
		DIANXIAN_STRU wire;
		if(db.QueryWire(wireType,wireSpec,&wire))
			return wire.m_fArea;
	}
	return 0;
}
double CLibraryQuery::GetWireForcePermit(char* wireType,char* wireSpec)		//�������,N
{
	CWireLoadDatabase db;
	char file_path[MAX_PATH];
	GetSysPath(file_path,NULL,NULL,NULL);
	strcat(file_path,"wireload.d");
	if(db.OpenDbFile(file_path))
	{
		DIANXIAN_STRU wire;
		if(db.QueryWire(wireType,wireSpec,&wire))
			return wire.m_fForcePermit;
	}
	return 0;
}
double CLibraryQuery::GetWireElasticCoef(char* wireType,char* wireSpec)		//����ϵ��(ģ��),MPa
{
	CWireLoadDatabase db;
	char file_path[MAX_PATH];
	GetSysPath(file_path,NULL,NULL,NULL);
	strcat(file_path,"wireload.d");
	if(db.OpenDbFile(file_path))
	{
		DIANXIAN_STRU wire;
		if(db.QueryWire(wireType,wireSpec,&wire))
			return wire.m_fElasticCoef;
	}
	return 0;
}
double CLibraryQuery::GetWireExpansionCoef(char* wireType,char* wireSpec)	//�¶�������ϵ��,
{
	CWireLoadDatabase db;
	char file_path[MAX_PATH];
	GetSysPath(file_path,NULL,NULL,NULL);
	strcat(file_path,"wireload.d");
	if(db.OpenDbFile(file_path))
	{
		DIANXIAN_STRU wire;
		if(db.QueryWire(wireType,wireSpec,&wire))
			return wire.m_fExpansionCoef;
	}
	return 0;
}
#endif

//itemType--�������� -1��ʾ�����øò��� cMaterial--����
void CLibraryQuery::InitColor()
{
	//������ɫ����
	crPartMode.crNode = RGB(255, 255, 255);
	crPartMode.crControlNode = RGB(255, 255, 255);		//���ƽڵ�
	crPartMode.crDivideScaleNode = RGB(255, 255, 0);	//�ȷֽڵ�
	crPartMode.crOffsetNode = RGB(128, 128, 0);		//ƫ�ƽڵ�
	crPartMode.crAxisValFixNode = RGB(255, 80, 0);	//ָ����������ڵ�
	crPartMode.crPole = RGB(192, 192, 192);		//(150,150,255);���µ���Ϊ����ɫ���ÿ� wjh-2018.6.30
	crPartMode.crLine = RGB(255, 255, 255);	//RGB(192,192,192);		//(150,150,255);
	crPartMode.crAngleEdge = RGB(0, 100, 255);
	crPartMode.crAngleWingX = RGB(150, 150, 255);//RGB(220,220,220);//RGB(192,192,192);	//
	crPartMode.crAngleWingY = RGB(150, 150, 255);//RGB(220,220,220);//RGB(192,192,192);	////RGB(0,128,255);
	crPartMode.crSonAngle1Edge = RGB(0, 100, 255);
	crPartMode.crSonAngle1WingX = RGB(0, 128, 128);
	crPartMode.crSonAngle1WingY = RGB(0, 128, 128);
	crPartMode.crBolt12 = RGB(255, 0, 255);
	crPartMode.crBolt16 = RGB(128, 0, 64);
	crPartMode.crBolt20 = RGB(128, 0, 255);
	crPartMode.crBolt24 = RGB(46, 0, 91);
	crPartMode.crBolt27 = RGB(200, 160, 88);
	crPartMode.crBolt30 = RGB(130, 20, 130);
	crPartMode.crBolt36 = RGB(233, 13, 133);
	crPartMode.crBolt39 = RGB(45, 195, 195);
	crPartMode.crBolt42 = RGB(255, 160, 0);
	crPartMode.crBolt48 = RGB(180, 180, 60);
	crPartMode.crOtherBolt = RGB(128, 0, 255);
	crPartMode.crAntiTheftNut = RGB(255, 0, 0);
	crPartMode.crAntiLoosenNut = RGB(150, 150, 255);
	crPartMode.crPlate = RGB(159, 120, 50);//RGB(0,255,128);
	crPartMode.crCutFaceOrPolyJoint = RGB(255, 128, 128);
	crPartMode.crPartBackground = RGB(150, 150, 255);
	crPartMode.crShortAngle = RGB(90, 90, 90);
	crPartMode.crStdPart = RGB(100, 100, 100);
	crPartMode.crBackground = RGB(0, 64, 160);
}
COLORREF CLibraryQuery::GetColor(int idClassType,long itemType,BYTE cMaterial)
{
#if defined(__ANGLE_PART_INC_)&&(defined(__LDS_CONTEXT_)||defined(__LDS_GROUP_CONTEXT_))//defined(__TMA_)||defined(__LMA_)||defined(__TSA_)||defined(__LDS_)||defined(__TMAGROUP_)||defined(__LMAGROUP_)||defined(__TSAGROUP_)||defined(__LDSGROUP_)
	if(idClassType==CLS_BOLT)
	{	//��˨��ɫ������ɫģʽ����
		if (12 == itemType)
			return crPartMode.crBolt12;
		else if (16 == itemType)
			return crPartMode.crBolt16;
		else if (20 == itemType)
			return crPartMode.crBolt20;
		else if (24 == itemType)
			return crPartMode.crBolt24;
		else if (27 == itemType)
			return crPartMode.crBolt27;
		else if (30 == itemType)
			return crPartMode.crBolt30;
		else if (36 == itemType)
			return crPartMode.crBolt36;
		else if (39 == itemType)
			return crPartMode.crBolt39;
		else if (42 == itemType)
			return crPartMode.crBolt42;
		else if (48 == itemType)
			return crPartMode.crBolt48;
		else 
			return crPartMode.crOtherBolt;
	}
	else //if(g_sysPara.iColorSchema==3)
	{	//��������ģʽ
		if(itemType==0)
			return crPartMode.crBackground;
		else if(itemType==1)
			return crPartMode.crLine;
		else if(itemType==2||idClassType==CLS_NODE)
			return crPartMode.crNode;
		else if(itemType==3)
			return crPartMode.crPole;
		else if(idClassType==CLS_LINEANGLE||idClassType==CLS_GROUPLINEANGLE||idClassType==CLS_ARCANGLE)
		{
			if(itemType=='E')
				return crPartMode.crAngleEdge;
			else if(itemType=='X')
				return crPartMode.crAngleWingX;
			else if(itemType=='Y')
				return crPartMode.crAngleWingY;
			else if(itemType=='e')	//1���ӽǸ�֫����ɫ	wht 11-05-20
				return crPartMode.crSonAngle1Edge;
			else if(itemType=='x')	//1���ӽǸ�X֫��ɫ
				return crPartMode.crSonAngle1WingX;
			else if(itemType=='y')	//1���ӽǸ�Y֫��ɫ
				return crPartMode.crSonAngle1WingY;
			else if(itemType=='S')
				return crPartMode.crShortAngle;
			else
				return crPartMode.crAngleWingX;
		}
		else if(idClassType==CLS_PLATE || idClassType==CLS_PARAMPLATE)
		{	//��ͬ�������Ӱ����ɫ����			-----wxc 2014.5.16
			if (itemType == 'S' && idClassType == CLS_PARAMPLATE)
				return crPartMode.crStdPart;
			else
				return crPartMode.crPlate;//g_sysPara.plateColRef.GetPlateColor(cMaterial);
		}
		else if(idClassType==CLS_POLYJOINT)
			return crPartMode.crCutFaceOrPolyJoint;
		else
			return RGB(0,160,180);
	}
#else
	if(idClassType==CLS_BOLT)
	{	//��˨��ɫ������ɫģʽ����
		int boltDArr[10]={12,16,20,24,27,30,36,39,42,48};
		COLORREF clrArr[11]={ RGB(255,0,255),RGB(128,0,64),RGB(128,0,255),RGB( 46,0,91),
							  RGB(200,160,88),RGB(130,20,130),RGB(233,13,133),RGB(45,195,195),
							  RGB(255,160,0),RGB(180,180,60),RGB(128,0,255)};
		for(int i=0;i<10;i++)
		{
			if(boltDArr[i]==itemType)
				return clrArr[i];
		}
		return clrArr[10];
	}
	else if(idClassType==CLS_LINEANGLE||idClassType==CLS_GROUPLINEANGLE||idClassType==CLS_ARCANGLE)
	{
		char typeArr[7]={'E','X','Y','e','x','y','S'};
		COLORREF clrArr[8]={ RGB(0,100,255),RGB(150,150,255),RGB(150,150,255),RGB(0,100,255),
							  RGB(0,128,128),RGB(0,128,128),RGB(90,90,90),RGB(150,150,255)};
		for(int i=0;i<7;i++)
		{
			if(typeArr[i]==itemType)
				return clrArr[i];
		}
		return clrArr[7];
	}
	else if(idClassType==CLS_PLATE || idClassType==CLS_PARAMPLATE)
		return RGB(0,255,128);
	else if(idClassType==CLS_POLYJOINT)
		return RGB(255,128,128);
	else
		return RGB(0,160,180);
#endif
}
