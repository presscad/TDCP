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
	//����������ʱ�������⣬����ѡ��ʱ��
public:
	int nAngleType,nTubeType,nFlatType;
	JG_PARA_TYPE jgguige_table[100];
	TUBE_PARA_TYPE tubeguige_table[100];
	BIAN_PARA_TYPE flatguige_table[100];
	CLibraryQuery(){nAngleType=nTubeType=nFlatType=0;}
	int FindAngleTypeIndex(double wing_wide,double wing_thick,char cType);
	int FindTubeTypeIndex(double d,double t);
	int FindFlatTypeIndex(double wide,double thick);
	//�麯���������ڲ�ѯ��������
public:
	virtual double FindAngleInnerR(double wide);	//��ѯ�Ǹ���Բ���뾶
	virtual JGZJ   GetAngleZJ(double wide);			//��ѯ�Ǹ�׼��
	//��λmm2
	virtual double GetPartArea(double size_para1,double size_para2,double size_para3,int idPartClsType,char cSubType='L');
	//��λkg
	virtual double GetWeightOf1Meter(double size_para1,double size_para2,double size_para3,int idPartClsType,char cSubType='L');
	//���ظ˼�ָ�����Ǹ�ʱpara=0ƽ�����para=1��С�ᣩ���Ծ�(mm4)
	virtual double GetWorkI(double size_para1,double size_para2,double size_para3,int axis,int idPartClsType,char cSubType='L');
	//���ظ˼�ָ�����Ǹ�ʱpara=0ƽ�����para=1��С�ᣩ����ģ��(mm3)
	virtual double GetWorkW(double size_para1,double size_para2,double size_para3,int axis,int idPartClsType,char cSubType='L');
	//���ظ˼�ָ�����Ǹ�ʱpara=0ƽ�����para=1��С�ᣩ��ת�뾶(mm)
	virtual double GetWorkR(double size_para1,double size_para2,double size_para3,int axis,int idPartClsType,char cSubType='L');
	virtual double GetW(double size_para1,double size_para2,double size_para3,int axis,int idPartClsType,char cSubType='L');
	virtual double GetElasticityModulus(char cMaterial);//����ģ��(MPa)
	virtual STEELMAT* QuerySteelMatByBriefMark(char briefmark);
	//���ϵͳ����Ӧ����ɢ�� ��ʱ���ѯ������ĺ�������һ���Ժ��ٽ�IPartLibrary������չ wht 10-02-23 
	virtual double GetStressDiffuseAngle();
	//����ϵͳ���������϶
	virtual UINT GetVertexDist();
	//����ϵͳ����������϶
	virtual UINT GetHuoQuClearance();
	//��ȡ�Ǹֿ��Ͻ���ֵ wht 11-05-06
	virtual double GetJgKaiHeJiaoThreshold();
	virtual double GetJgKaiHeJiaoMinDeita();
	virtual double GetMaxKaiHeJiaoClearance();
	//��ȡ��С�нǿ��
	virtual double GetMinCutAngleLen();
	virtual double GetMaxIgnoreCutAngleLen();
	//��ȡ�нǼ�϶��Բ���뾶ռ��ϵ��
	virtual double GetCutAngleCoefR();
	//��ȡ�ְ���������㷨
	virtual int GetProfileAlgorithm();
	virtual void FindLsBoltDH(int d, double *D,double *H,double *H2);
#if defined(__TSA_)||defined(__LDS_)
	//����ר�ò�ѯ����
	virtual double GetWireArea(char* wireType,char* wireSpec);				//���½����,mm2
	virtual double GetWireUnitWeightPerKM(char* wireType,char* wireSpec);	//��λ������,kg/km
	virtual double GetWireForcePermit(char* wireType,char* wireSpec);		//�������,N
	virtual double GetWireElasticCoef(char* wireType,char* wireSpec);		//����ϵ��(ģ��),MPa
	virtual double GetWireExpansionCoef(char* wireType,char* wireSpec);		//�¶�������ϵ��,
#endif
#if !defined(__TSA_)&&!defined(__TSA_FILE_)
	//�õ���׼����ģ��
	virtual bool GetStdPartModel(char *sPartNo,int std_part_type, void *pStdPartModel,double t=0);
	virtual bool GetStdPartModel(double d,double t,char cMaterial,BYTE level,int std_part_type, void *pStdPartModel);
	//�õ���Ȧ��� wht 10-12-24
	//ciGetFirst0Max1=0,���ص�һ�������Ȧ���ֵ��
	//ciGetFirst0Max1=1,�������������Ȧ���ֵ��=2,������С��������ֵ wjh-2019.7.24
	virtual int GetBoltPadThick(int bolt_d,char ciGetFirst0Max1Min2=0);
	virtual int GetWasherThicks(int niMd,BYTE* parrThicks,int nMaxThickCount=4);
	virtual short GetBestMatchWasherThick(int niMd,double distance);	//������ǡ���ĵ�Ȧ���
	//����ƥ�����˨����ͺ�
	virtual LS_XING_HAO *FindLsGuiGe(int d, double L0,long hFamily=0);
	//����ֱ������Ч���Ȳ�����˨����ͺ�
	LS_XING_HAO* FindLsGuiGeByValidLen(int d, double L,long hFamily=0);
	//������˨ֱ������Ч������ƥ����˨�ĵ�������,kg
	virtual double GetLsWeight(int d, double L,long hFamily=0);
#endif
	//itemType--�������� -1��ʾ�����øò��� cMaterial--����
	virtual COLORREF GetColor(int idClassType,long itemType,BYTE cMaterial);
};
extern CLibraryQuery globalLibrary;

#endif // !defined(AFX_LIBRARYQUERY_H__A53880D0_95CA_4974_9E11_58DE9C2176E9__INCLUDED_)
