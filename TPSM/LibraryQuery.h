// LibraryQuery.h: interface for the CLibraryQuery class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LIBRARYQUERY_H__A53880D0_95CA_4974_9E11_58DE9C2176E9__INCLUDED_)
#define AFX_LIBRARYQUERY_H__A53880D0_95CA_4974_9E11_58DE9C2176E9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IPartLibrary.h"

class CLibraryQuery : public IPartLibrary  
{
	//建立各类临时构件规格库，用于选材时用
public:
	int nAngleType,nTubeType,nFlatType;
	JG_PARA_TYPE jgguige_table[100];
	TUBE_PARA_TYPE tubeguige_table[100];
	BIAN_PARA_TYPE flatguige_table[100];
	CLibraryQuery(){nAngleType=nTubeType=nFlatType=0;}
	int FindAngleTypeIndex(double wing_wide,double wing_thick,char cType);
	int FindTubeTypeIndex(double d,double t);
	int FindFlatTypeIndex(double wide,double thick);
	//虚函数区，用于查询构件库用
public:
	virtual double FindAngleInnerR(double wide);	//查询角钢内圆弧半径
	virtual JGZJ   GetAngleZJ(double wide);			//查询角钢准距
	//单位mm2
	virtual double GetPartArea(double size_para1,double size_para2,double size_para3,int idPartClsType,char cSubType='L');
	//单位kg
	virtual double GetWeightOf1Meter(double size_para1,double size_para2,double size_para3,int idPartClsType,char cSubType='L');
	//返回杆件指定（角钢时para=0平行轴或para=1最小轴）惯性矩(mm4)
	virtual double GetWorkI(double size_para1,double size_para2,double size_para3,int axis,int idPartClsType,char cSubType='L');
	//返回杆件指定（角钢时para=0平行轴或para=1最小轴）抗弯模量(mm3)
	virtual double GetWorkW(double size_para1,double size_para2,double size_para3,int axis,int idPartClsType,char cSubType='L');
	//返回杆件指定（角钢时para=0平行轴或para=1最小轴）回转半径(mm)
	virtual double GetWorkR(double size_para1,double size_para2,double size_para3,int axis,int idPartClsType,char cSubType='L');
	virtual double GetW(double size_para1,double size_para2,double size_para3,int axis,int idPartClsType,char cSubType='L');
	virtual double GetElasticityModulus(char cMaterial);//弹性模量(MPa)
	virtual STEELMAT* QuerySteelMatByBriefMark(char briefmark);
	//获得系统参数应力扩散角 暂时与查询构件库的函数放在一起，以后再将IPartLibrary进行扩展 wht 10-02-23 
	virtual double GetStressDiffuseAngle();
	//返回系统参数顶点间隙
	virtual UINT GetVertexDist();
	//返回系统参数火曲间隙
	virtual UINT GetHuoQuClearance();
	//获取角钢开合角阈值 wht 11-05-06
	virtual double GetJgKaiHeJiaoThreshold();
	virtual double GetJgKaiHeJiaoMinDeita();
	virtual double GetMaxKaiHeJiaoClearance();
	//获取最小切角宽度
	virtual double GetMinCutAngleLen();
	virtual double GetMaxIgnoreCutAngleLen();
	//获取切角间隙内圆弧半径占比系数
	virtual double GetCutAngleCoefR();
	//获取钢板外形设计算法
	virtual int GetProfileAlgorithm();
	virtual void FindLsBoltDH(int d, double *D,double *H,double *H2);
#if defined(__TSA_)||defined(__LDS_)
	//线缆专用查询函数
	virtual double GetWireArea(char* wireType,char* wireSpec);				//线缆截面积,mm2
	virtual double GetWireUnitWeightPerKM(char* wireType,char* wireSpec);	//单位长重量,kg/km
	virtual double GetWireForcePermit(char* wireType,char* wireSpec);		//额定抗拉力,N
	virtual double GetWireElasticCoef(char* wireType,char* wireSpec);		//弹性系数(模量),MPa
	virtual double GetWireExpansionCoef(char* wireType,char* wireSpec);		//温度线膨胀系数,
#endif
#if !defined(__TSA_)&&!defined(__TSA_FILE_)
	//得到标准构件模型
	virtual bool GetStdPartModel(char *sPartNo,int std_part_type, void *pStdPartModel,double t=0);
	virtual bool GetStdPartModel(double d,double t,char cMaterial,BYTE level,int std_part_type, void *pStdPartModel);
	//得到垫圈厚度 wht 10-12-24
	//ciGetFirst0Max1=0,返回第一条适配垫圈厚度值；
	//ciGetFirst0Max1=1,返回最大的适配垫圈厚度值；=2,返回最小的适配厚度值 wjh-2019.7.24
	virtual int GetBoltPadThick(int bolt_d,char ciGetFirst0Max1Min2=0);
	virtual int GetWasherThicks(int niMd,BYTE* parrThicks,int nMaxThickCount=4);
	virtual short GetBestMatchWasherThick(int niMd,double distance);	//返回最恰当的垫圈厚度
	//查找匹配的螺栓规格型号
	virtual LS_XING_HAO *FindLsGuiGe(int d, double L0,long hFamily=0);
	//根据直径及有效长度查找螺栓规格型号
	LS_XING_HAO* FindLsGuiGeByValidLen(int d, double L,long hFamily=0);
	//根据螺栓直径及有效长查找匹配螺栓的单颗重量,kg
	virtual double GetLsWeight(int d, double L,long hFamily=0);
#endif
	//itemType--辅助参数 -1表示不启用该参数 cMaterial--材质
	virtual COLORREF GetColor(int idClassType,long itemType,BYTE cMaterial);
};
extern CLibraryQuery globalLibrary;

#endif // !defined(AFX_LIBRARYQUERY_H__A53880D0_95CA_4974_9E11_58DE9C2176E9__INCLUDED_)
