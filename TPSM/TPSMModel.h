#pragma once
#include "Tower.h"
#include "ElectricTower.h"
#include "PropListItem.h"
#include "XhCharString.h"

//铁塔设计参数
struct TOWER_DESIGN_PARA
{	
	BYTE m_ciTowerType;
	int m_nVoltGrade;			//电压等级（KV）
	BYTE m_ciShapeType;			//铁塔外形类型（鼓型|猫头型）
	BYTE m_ciMainRodType;		//主材类型：0.角钢|1.钢管
	int m_nCircuit;				//回路数
	double m_fAaltitude;		//海拔高度(m)
	double m_fWindSpeed;		//10米设计风速(m/s)
	double m_fNiceThick;		//设计冰厚(mm)
	double m_fCheckNiceThick;	//验算冰厚(mm)
	double m_fWindSpan;			//水平档距(m)
	double m_fWeightSpan;		//垂直档距(m)
	double m_fRulingSpan;		//代表档距(m)
	char m_sAngleRange[100];	//转角度数范围
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
	TOWER_DESIGN_PARA m_xDesPara;		//设计参数
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
	//文件导出
	void CreateModFile(const char* sFileName);
	void CreateDxfFile(const char* sFileName);
	void Create3dsFile(const char* sFileName);
	void CreateLDSFile(const char* sFileName);
	//文件存储
	BOOL SaveTpiFile();
	BOOL OpenTpiFile();
	void FromBuffer(CBuffer& buffer);
	void ToBuffer(CBuffer& buffer);
	//属性栏显示
	DECLARE_PROP_FUNC(CTPSMModel);
	static KEY4C GetPropKey(long id);
	int GetPropValueStr(long id,char* valueStr,UINT nMaxStrBufLen=100);	//根据属性ID得到属性值
};

//////////////////////////////////////////////////////////////////////////
//杆塔规划工程
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
	//数据操作
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
//工程方案管理器
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
	//数据操作
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
