#pragma once
#include "ModCore.h"
#include "TidCplus.h"
#include "I3DS.h"
#include "XhCharString.h"
#include "LogFile.h"
#ifdef _HANDOVER_TO_CLIENT_
#include "AtomList.h"
#else
#include "f_ent_list.h"	//主要用ATOM_LIST类
#endif

extern CLogFile xMyErrLog;
//////////////////////////////////////////////////////////////////////////
//CTowerModle塔型(多呼高多接腿)
class CTowerManager;
class CTowerModel
{
	CTowerManager* m_pTaManager;
public:
	CXhChar100 m_sFilePath;
	CXhChar50 m_sTowerPlaceCode;	//塔位号
	CXhChar50 m_sTowerDot;	//塔位点
	CXhChar50 m_sTowerType;	//塔型
	CXhChar50 m_sHuGao;		//胡高
	double m_fLocateH;		//定位高
	double m_fBodyHeight;	//接身高
	double m_fMaxLegHeight;	//最长腿高
	BOOL m_bTurnLeft;	//是否左转，需调整塔型显示坐标系及配腿
	UINT m_uiHeightSerial;
	UINT m_uiLegSerialArr[4];
	BYTE m_ciModelFlag;	//0.TID|1.MOD
	//
	static const int TYPE_TID	=0;
	static const int TYPE_MOD	=1;	
	//
	BYTE m_ciErrorType;
	static const BYTE ERROR_NO_FILE			= 1;	//找不到文件
	static const BYTE ERROR_READ_FILE		= 2;	//文件读取失败
	static const BYTE ERROR_FIND_HUGAO		= 3;	//找不到呼高
	static const BYTE ERROR_FIND_LEG		= 4;	//找不到配腿
	static const BYTE ERROR_FIND_INSTANCE	= 5;	//找不到塔例
public:
	CTowerModel();
	~CTowerModel();
	//
	void SetManager(CTowerManager* pManager){m_pTaManager=pManager;}
	CTowerManager* BelongManager(){return m_pTaManager;}
	void* ExtractActiveTaInstance();
	BOOL ExtractActiveTaInstance(char* sHuGao,double fLegA,double fLegB,double fLegC,double fLegD);
	CXhChar100 GetFileName(void* pInstance);
	GECS BuildTransCS(void* pInstance=NULL);
	GECS BuildRotateCS();
	//
	void CreateXmlFile(ITidTowerInstance* pInstance,const char* sFileName);
	void CreateXmlFile(IModTowerInstance* pInstance,const char* sFileName);
	void CreateModFile(ITidTowerInstance* pInstance,const char* sFileName);
	void CreateModFile(IModTowerInstance* pInstance,const char* sFileName);
	void Create3dsFile(ITidTowerInstance* pInstance,const char* sFileName);
	void Create3dsFile(IModTowerInstance* pInstance,const char* sFileName);
	void CreateDxfFile(ITidTowerInstance* pInstance,const char* sFileName);
	void CreateDxfFile(IModTowerInstance* pInstance,const char* sFileName);
};
//////////////////////////////////////////////////////////////////////////
//CTowerManager
class CTowerManager
{
private:
	ATOM_LIST<CTowerModel> m_xTaModelList;
public:
	CTowerModel* m_pActiveTa;
	ITidModel* m_pTidModel;
	IModModel* m_pModModel;
	BYTE m_ciInstanceLevel;	//0.LOD0单线|1.LOD1杆件|2.LOD2杆件+板|3.LOD3放样模型
	BOOL m_bUseUnitM;		//是否启用长度单位米（默认为毫米）
public:
	CTowerManager();
	~CTowerManager();
	void Empty();
	//
	CTowerModel* EnumFirstTaModel(){return m_xTaModelList.GetFirst();}
	CTowerModel* EnumNextTaModel(){return m_xTaModelList.GetNext();}
	int GetTaModelNum(){return m_xTaModelList.GetNodeNum();}
	//
	BOOL ReadTidFile(const char* sFileName);
	BOOL ReadModFile(const char* sFileName);
	BOOL ReadExcleFile(const char* sFileName);
	void CreateXmlFiles(const char* sPath);
	void CreateModFiles(const char* sPath);
	void Create3dsFiles(const char* sPath);
};
extern CTowerManager TaManager;