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
//查询角钢内圆弧半径
double CLibraryQuery::FindAngleInnerR(double wide)
{
	return FindJg_r(wide);
}
JGZJ CLibraryQuery::GetAngleZJ(double wide)		//查询角钢准距
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
		if(cSubType!='T'&&cSubType!='D'&&cSubType!='X'&&cSubType!='L')
			cSubType='L';	//防止出现错误数据
		JG_PARA_TYPE *pJgPara=NULL;
		if(wing_wide_y<=0||wing_wide_y==wing_wide)
			pJgPara=FindJgType(wing_wide,wing_thick,cSubType);
		if(pJgPara)
			return pJgPara->area*100;
		else	//如库内设定，只能计算近似值
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
		else	//如库内设定，只能计算近似值
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
		else	//如库内设定，只能计算近似值
			return 0;
	}
	else if(idPartClsType==CLS_LINETUBE)
	{
		double wing_wide=size_para1;
		double wing_thick=size_para2;
		TUBE_PARA_TYPE *pTubePara=FindTubeType(wing_wide,wing_thick);
		if(pTubePara)
			return pTubePara->I*10000;
		else	//如库内设定，只能计算近似值
			return 0;
	}
	else if(idPartClsType==CLS_LINEFLAT)
	{
		return 0;
	}
	else
		return 0;
}
//返回杆件指定（角钢时para=0平行轴或para=1最小轴）抗弯模量(mm3)
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
		else	//如库内设定，只能计算近似值
			return 0;
	}
	else if(idPartClsType==CLS_LINETUBE)
	{
		double wing_wide=size_para1;
		double wing_thick=size_para2;
		TUBE_PARA_TYPE *pTubePara=FindTubeType(wing_wide,wing_thick);
		if(pTubePara)
			return pTubePara->W*1000;
		else	//如库内设定，只能计算近似值
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
		else	//如库内设定，只能计算近似值
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
		else	//如库内设定，只能计算近似值
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
//返回系统参数火曲间隙
UINT CLibraryQuery::GetHuoQuClearance()
{
#if defined(__ANGLE_PART_INC_)&&defined(__LDS_CONTEXT_)//defined(__TMA_)||defined(__LMA_)||defined(__LDS_)
	return g_sysPara.HuoQuDist;
#else
	return 10;
#endif
}

//获得系统参数顶点间隙
UINT CLibraryQuery::GetVertexDist()
{
#if defined(__ANGLE_PART_INC_)&&defined(__LDS_CONTEXT_)//defined(__TMA_)||defined(__LMA_)||defined(__LDS_)
	return g_sysPara.VertexDist;
#else
	return 5;
#endif
}

//获得系统参数应力扩散角 
double CLibraryQuery::GetStressDiffuseAngle()
{
#if defined(__ANGLE_PART_INC_)&&defined(__LDS_CONTEXT_)//defined(__TMA_)||defined(__LMA_)||defined(__LDS_)
	return g_sysPara.fStressDiffuseAngle;
#else
	return 30;
#endif
}

#if !defined(__TSA_)&&!defined(__TSA_FILE_)
//根据钢管直径、厚度、级别和端板类型等查询标准件
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
//得到标准构件模型
bool CLibraryQuery::GetStdPartModel(char *sPartNo,int std_part_type, void *pStdPartModelObj,double t/*=0*/)
{
	CLDSStdPartModel *pStdPartModel=(CLDSStdPartModel*)pStdPartModelObj;
	pStdPartModel->m_iStdPartType=std_part_type;
	if(std_part_type==TYPE_FLD)
	{	//对焊法兰
		for(int i=0;i<fld_param_N;i++)
		{
			if(fld_param_table[i].IsSameCode(sPartNo))	//对焊法兰实际编号中可能会含有主管壁厚分组信息
			{
				pStdPartModel->param.fl_param=fld_param_table[i];
				pStdPartModel->param.fl_param.t=t;
				pStdPartModel->param.fl_param.UpdateCodeName();
				return TRUE;
			}
		}
	}
	else if(std_part_type==TYPE_FLP)
	{	//平焊法兰
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
	{	//刚性法兰
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
	{	//柔性法兰
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
	{	//槽型插板
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
	{	//U型插板
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
	{	//十字插板
		for(int i=0;i<xend_param_N;i++)
		{	//十字插板代号与基准钢板编号不相同,使用构件编号查找十字插板参数时应将编号与基板编号对比 wht 10-01-24
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

//得到垫圈厚度 wht 10-12-24
int CLibraryQuery::GetBoltPadThick(int bolt_d,char ciGetFirst0Max1Min2/*=0*/)
{
	BYTE* pxarrThicks = NULL;
#if defined(__LDS_CONTEXT_)||defined(__LDS_GROUP_CONTEXT_)
	if(bolt_d==12)
		pxarrThicks = &g_sysPara.BoltPadThick.xarrM12Thicks[0];
	else if(bolt_d==16)
		pxarrThicks = &g_sysPara.BoltPadThick.xarrM16Thicks[0];
	else if(bolt_d==20)
		pxarrThicks = &g_sysPara.BoltPadThick.xarrM20Thicks[0];
	else if(bolt_d==24)
		pxarrThicks = &g_sysPara.BoltPadThick.xarrM24Thicks[0];
#endif
	if(pxarrThicks==NULL)
		return 0;
	BYTE cnInitThick=pxarrThicks[0];
	if(ciGetFirst0Max1Min2!=1&&ciGetFirst0Max1Min2!=2)
		return cnInitThick;
	for(int i=1;i<4;i++)
	{
		if(ciGetFirst0Max1Min2==1&&pxarrThicks[i]>cnInitThick)
			cnInitThick=pxarrThicks[i];
		else if(ciGetFirst0Max1Min2==2&&pxarrThicks[i]<cnInitThick)
			cnInitThick=pxarrThicks[i];
	}
	return cnInitThick;
}
int CLibraryQuery::GetWasherThicks(int niMd,BYTE* parrThicks,int nMaxThickCount/*=4*/)
{
#if defined(__LDS_CONTEXT_)||defined(__LDS_GROUP_CONTEXT_)
	if(niMd==12)
		memcpy(parrThicks,g_sysPara.BoltPadThick.xarrM12Thicks,min(4,nMaxThickCount));
	else if(niMd==16)
		memcpy(parrThicks,g_sysPara.BoltPadThick.xarrM16Thicks,min(4,nMaxThickCount));
	else if(niMd==20)
		memcpy(parrThicks,g_sysPara.BoltPadThick.xarrM20Thicks,min(4,nMaxThickCount));
	else if(niMd==24)
		memcpy(parrThicks,g_sysPara.BoltPadThick.xarrM24Thicks,min(4,nMaxThickCount));
	else 
#endif
		return 0;
	for(int i=0;i<4;i++)
	{
		if(parrThicks[i]==0)
			return i;
	}
	return i;
}
short CLibraryQuery::GetBestMatchWasherThick(int niMd,double distance)
{
	BYTE thicks[4]={0};
	int i,count=GetWasherThicks(niMd,thicks);
	if(count==0||fabs(distance)<EPS2)
		return 0;
	short siMaxThick=0;
	for(i=0;i<count;i++)
		siMaxThick=max(siMaxThick,thicks[i]);
	if(distance<(siMaxThick*3+1.2))	//dist>nBoltPadThick&&
	{	//大于1个垫圈厚度小于3个垫圈厚度
		short siMatchThick=siMaxThick;
		short siWasherN=ftoi(distance/siMaxThick);
		double dfMinClearance=fabs(distance-siWasherN*siMaxThick);
		for(i=0;i<count;i++)
		{
			double ncoef=distance/thicks[i];
			double clearance=fabs(ncoef-ftoi(ncoef))*thicks[i];
			if(clearance<dfMinClearance-0.5)	//-0.5是为了避免3x4与4x3差别 wjh-2019.9.2
			{
				dfMinClearance=clearance;
				siMatchThick=thicks[i];
			}
			if(thicks[i]==0)
				break;
		}
		return siMatchThick;
	}
	else
		return siMaxThick;	//返回零的话与最原始被替代的函数GetBoltPadThick()逻辑不一致，可能导致外围代码异常 wjh-2019.5.19
}
//查找匹配的螺栓规格型号
LS_XING_HAO* CLibraryQuery::FindLsGuiGe(int d, double L0,long hFamily/*=0*/)
{
	return CLsLibrary::FindLsGuiGe(d,L0,hFamily);
}
//查找匹配的螺栓规格型号
LS_XING_HAO* CLibraryQuery::FindLsGuiGeByValidLen(int d, double L,long hFamily/*=0*/)
{
	return CLsLibrary::FindLsGuiGeByValidLen(d,L,hFamily);
}
	//根据螺栓直径及有效长查找匹配螺栓的单颗重量,kg
double CLibraryQuery::GetLsWeight(int d, double L,long hFamily/*=0*/)
{
	LS_XING_HAO *pXingHao=CLsLibrary::FindLsGuiGeByValidLen(d,L,hFamily);
	if(pXingHao)
		return pXingHao->weight;
	else
		return 0;
}
#endif
//角钢开合角阈值 wht 11-05-06
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
//TAP中需要用到该参数，暂时打开将来需统筹考虑　wjh-2014.9.1
//#if defined(__ANGLE_PART_INC_)&&defined(__LDS_CONTEXT_)
	FindLsD_H(d,D,H,H2);
//#endif
}

#if defined(__TSA_)||defined(__LDS_)
#include "WireLoadDatabase.h"
void GetSysPath(char* App_Path,const char *src_path=NULL,char *file_name=NULL,char *extension=NULL);//获得系统运行路径，App_Path应在函数外开辟好内存
//线缆专用查询函数
double CLibraryQuery::GetWireUnitWeightPerKM(char* wireType,char* wireSpec)	//单位长重量,kg/km
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
double CLibraryQuery::GetWireArea(char* wireType,char* wireSpec)		//线缆截面积,mm2
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
double CLibraryQuery::GetWireForcePermit(char* wireType,char* wireSpec)		//额定抗拉力,N
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
double CLibraryQuery::GetWireElasticCoef(char* wireType,char* wireSpec)		//弹性系数(模量),MPa
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
double CLibraryQuery::GetWireExpansionCoef(char* wireType,char* wireSpec)	//温度线膨胀系数,
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

//itemType--辅助参数 -1表示不启用该参数 cMaterial--材质
COLORREF CLibraryQuery::GetColor(int idClassType,long itemType,BYTE cMaterial)
{
#if defined(__ANGLE_PART_INC_)&&(defined(__LDS_CONTEXT_)||defined(__LDS_GROUP_CONTEXT_))//defined(__TMA_)||defined(__LMA_)||defined(__TSA_)||defined(__LDS_)||defined(__TMAGROUP_)||defined(__LMAGROUP_)||defined(__TSAGROUP_)||defined(__LDSGROUP_)
	if(idClassType==CLS_BOLT)
	{	//螺栓颜色不受颜色模式限制
		if (12 == itemType)
			return g_sysPara.crPartMode.crBolt12;
		else if (16 == itemType)
			return g_sysPara.crPartMode.crBolt16;
		else if (20 == itemType)
			return g_sysPara.crPartMode.crBolt20;
		else if (24 == itemType)
			return g_sysPara.crPartMode.crBolt24;
		else if (27 == itemType)
			return g_sysPara.crPartMode.crBolt27;
		else if (30 == itemType)
			return g_sysPara.crPartMode.crBolt30;
		else if (36 == itemType)
			return g_sysPara.crPartMode.crBolt36;
		else if (39 == itemType)
			return g_sysPara.crPartMode.crBolt39;
		else if (42 == itemType)
			return g_sysPara.crPartMode.crBolt42;
		else if (48 == itemType)
			return g_sysPara.crPartMode.crBolt48;
		else if(-1==itemType)	//防盗螺母
			return g_sysPara.crPartMode.crAntiTheftNut;
		else if(-2==itemType)	//防松螺母
			return g_sysPara.crPartMode.crAntiLoosenNut;
		else 
			return g_sysPara.crPartMode.crOtherBolt;
	}
	else if(g_sysPara.iColorSchema==1)
	{	//TSA模式
		if(itemType==0)
			return g_sysPara.crTsaMode.crBackground;
		else if(itemType==1)
			return g_sysPara.crTsaMode.crPrimaryLine;
		else if(itemType==2)
			return g_sysPara.crTsaMode.crNode;
		else if(itemType==3)
			return g_sysPara.crTsaMode.crPole;
		else if(itemType==4)
			return g_sysPara.crTsaMode.crLoadArrow;
		else if(itemType==5)
			return g_sysPara.crTsaMode.crDeformedPoleLine;
		else if(itemType==6)
			return g_sysPara.crTsaMode.crAuxLine;
		else if(itemType==7)
			return g_sysPara.crTsaMode.crZeroStressElem;
		else if(itemType==8)
			return g_sysPara.crTsaMode.crOverflowElem;
		//else
		//	return RGB(0,160,180);
	}
	if(g_sysPara.iColorSchema==2)
	{	//材质分类模式
		if(itemType==0)
			return g_sysPara.crMaterialMode.crBackground;
		else if(itemType==1)
			return g_sysPara.crMaterialMode.crLine;
		else if(itemType==2)
			return g_sysPara.crMaterialMode.crNode;
		else if(itemType==3)
			return g_sysPara.crMaterialMode.crPole;
		else 
		{
			if(cMaterial == CSteelMatLibrary::Q235BriefMark())//'S')
				return g_sysPara.crMaterialMode.crQ235;
			else if (cMaterial == CSteelMatLibrary::Q345BriefMark())//'H')
				return g_sysPara.crMaterialMode.crQ345;
			else if (cMaterial == CSteelMatLibrary::Q390BriefMark())//'G')
				return g_sysPara.crMaterialMode.crQ390;
			else if (cMaterial == CSteelMatLibrary::Q420BriefMark())//'T')
				return g_sysPara.crMaterialMode.crQ420;
			else if (cMaterial == CSteelMatLibrary::Q460BriefMark())//'U')
				return g_sysPara.crMaterialMode.crQ460;
			else
				return g_sysPara.crMaterialMode.crOther;
		}
	}
	else //if(g_sysPara.iColorSchema==3)
	{	//构件分类模式
		if(itemType==0)
			return g_sysPara.crPartMode.crBackground;
		else if(itemType==1)
			return g_sysPara.crPartMode.crLine;
		else if(itemType==2||idClassType==CLS_NODE)
			return g_sysPara.crPartMode.crNode;
		else if(itemType==3)
			return g_sysPara.crPartMode.crPole;
		else if(idClassType==CLS_LINEANGLE||idClassType==CLS_GROUPLINEANGLE|| idClassType == CLS_LINESLOT || idClassType==CLS_ARCANGLE)
		{
			if(itemType=='E')
				return g_sysPara.crPartMode.crAngleEdge;
			else if(itemType=='X')
				return g_sysPara.crPartMode.crAngleWingX;
			else if(itemType=='Y')
				return g_sysPara.crPartMode.crAngleWingY;
			else if(itemType=='e')	//1号子角钢肢边颜色	wht 11-05-20
				return g_sysPara.crPartMode.crSonAngle1Edge;
			else if(itemType=='x')	//1号子角钢X肢颜色
				return g_sysPara.crPartMode.crSonAngle1WingX;
			else if(itemType=='y')	//1号子角钢Y肢颜色
				return g_sysPara.crPartMode.crSonAngle1WingY;
			else if(itemType=='S')
				return g_sysPara.crPartMode.crShortAngle;
			else
				return g_sysPara.crPartMode.crAngleWingX;
		}
		else if(idClassType==CLS_PLATE || idClassType==CLS_PARAMPLATE)
		{	//不同材质连接板的配色方案			-----wxc 2014.5.16
			
			if(itemType=='S' && idClassType==CLS_PARAMPLATE)
				return g_sysPara.crPartMode.crStdPart;
			else
				return g_sysPara.plateColRef.GetPlateColor(cMaterial);
		}
		else if(idClassType==CLS_POLYJOINT)
			return g_sysPara.crPartMode.crCutFaceOrPolyJoint;
		else
			return RGB(0,160,180);
	}
#else
	if(idClassType==CLS_BOLT)
	{	//螺栓颜色不受颜色模式限制
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
