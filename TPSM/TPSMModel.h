#pragma once
#include "Tower.h"
#include "ElectricTower.h"
#include "PropListItem.h"
#include "XhCharString.h"

//������Ʋ���
struct TOWER_DESIGN_PARA
{	
	BYTE m_ciTowerType;
	int m_nVoltGrade;			//��ѹ�ȼ���KV��
	BYTE m_ciShapeType;			//�����������ͣ�����|èͷ�ͣ�
	BYTE m_ciMainRodType;		//�������ͣ�0.�Ǹ�|1.�ֹ�
	int m_nCircuit;				//��·��
	double m_fAaltitude;		//���θ߶�(m)
	double m_fWindSpeed;		//10����Ʒ���(m/s)
	double m_fNiceThick;		//��Ʊ���(mm)
	double m_fCheckNiceThick;	//�������(mm)
	double m_fWindSpan;			//ˮƽ����(m)
	double m_fWeightSpan;		//��ֱ����(m)
	double m_fRulingSpan;		//������(m)
	char m_sAngleRange[100];	//ת�Ƕ�����Χ
	//
	TOWER_DESIGN_PARA(){memset(this,0,sizeof(TOWER_DESIGN_PARA));}
};

//////////////////////////////////////////////////////////////////////////
//CTPSMModel
class CTPSMProject;
class CTPSMModel
{
	CTPSMProject* m_pBelongPrj;
public:
	CXhChar50  m_sTower;
	CXhChar100 m_sFullPath;
	TOWER_DESIGN_PARA m_xDesPara;		//��Ʋ���
	CHashList<PARA_SKETCH_ITEM> m_hashParaSketch;
	//
	CTower	m_xTower;
	CUniWireModel m_xWirePointModel;
	int m_nMaxLegs;
	static char lds_file_v[20];
public:
	CTPSMModel();
	~CTPSMModel();
	//
	void SetBelongPrj(CTPSMProject* pProject){m_pBelongPrj=pProject;}
	CTPSMProject* BelongPrj(){return m_pBelongPrj;}
	CTower* GetTa(){return &m_xTower;}
	//
	void ReadTowerTempFile(const char* sLdsFile,BOOL bInitDesPara=FALSE);
	void InitTempDesParaInfo();
	void InitParaSketchItems();
	void InitHangNodeInfo();
	bool UpdateSlopeDesPara();
	void ReBuildTower();
	CString MakeModuleLegStr(CLDSModule *pModule);
	//�ļ�����
	void CreateModFile(const char* sFileName);
	void CreateDxfFile(const char* sFileName);
	void Create3dsFile(const char* sFileName);
	void CreateLDSFile(const char* sFileName);
	//�ļ��洢
	BOOL SaveTpiFile();
	BOOL OpenTpiFile();
	void FromBuffer(CBuffer& buffer);
	void ToBuffer(CBuffer& buffer);
	//��������ʾ
	DECLARE_PROP_FUNC(CTPSMModel);
	static KEY4C GetPropKey(long id);
	int GetPropValueStr(long id,char* valueStr,UINT nMaxStrBufLen=100);	//��������ID�õ�����ֵ
};

//////////////////////////////////////////////////////////////////////////
//�����滮����
class CTPSMProject
{
	ATOM_LIST<CTPSMModel> m_xTpsmModelList;
public:
	CTPSMModel* m_pActiveModel;
	CXhChar50  m_sPrjName;
	CXhChar100 m_sPrjFullPath;
public:
	CTPSMProject();
	~CTPSMProject();
	//���ݲ���
	int TowerNum(){return m_xTpsmModelList.GetNodeNum();}
	void Empty(){m_xTpsmModelList.Empty();}
	void DeleteModel(CTPSMModel* pTpsmModel);
	CTPSMModel* AppendModel(){return m_xTpsmModelList.append();}
	CTPSMModel* EnumFirstModel(){return m_xTpsmModelList.GetFirst();}
	CTPSMModel* EnumNextModel(){return m_xTpsmModelList.GetNext();}
	//
	CTower* GetActiveTa();
	//
	BOOL ReadDesParaExcelFile(const char* sFile);
	BOOL SaveProjectFile();
	BOOL OpenProjectFile();
};
//////////////////////////////////////////////////////////////////////////
//���̷���������
class CProjectManager
{
	ATOM_LIST<CTPSMProject> m_xProjectList;
public:
	CXhChar50  m_sPrjSlnName;
	CXhChar100 m_sPrjSlnFullPath;
	CTPSMProject* m_pActivePrj;
public:
	CProjectManager();
	~CProjectManager();
	//���ݲ���
	int ProjectNum(){return m_xProjectList.GetNodeNum();}
	void Empty(){m_xProjectList.Empty();}
	void DeletePrj(CTPSMProject* pProject);
	CTPSMProject* AddProject(){return m_xProjectList.append();}
	CTPSMProject* EnumFirstPrj(){return m_xProjectList.GetFirst();}
	CTPSMProject* EnumNextPrj(){return m_xProjectList.GetNext();}
	//
	int TowerNum();
	CTower* GetActiveTa();
	//
	void OpenProjSln();
	void SaveProjSln();
};

extern CProjectManager Manager;
