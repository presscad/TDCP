#pragma once

#include "Tower.h"
#include "ModCore.h"
#include "TidCplus.h"
#include "Variant.h"
#include "PropListItem.h"
#include "XhCharString.h"
#include "GimDef.h"
//////////////////////////////////////////////////////////////////////////
//CTMDHModel
class CTMDHModel
{
public:
	CXhChar100 m_sFilePath;
	CXhChar50 m_sTowerNum;	//塔位号
	CXhChar50 m_sTowerDot;	//塔位点
	CXhChar50 m_sTowerType;	//塔型
	CXhChar50 m_sHuGao;		//胡高
	double m_fLocateH;		//定位高
	double m_fBodyHeight;	//接身高
	double m_fMaxLegHeight;	//最长腿高
	BOOL m_bTurnLeft;		//是否左转，需调整塔型显示坐标系及配腿
	//
	BYTE m_ciModelFlag;	//1.TID|2.MOD
	BYTE m_ciErrorType;
	UINT m_uiHeightSerial;
	UINT m_uiLegSerialArr[4];
	//
	static const int TYPE_TID	=1;
	static const int TYPE_MOD	=2;
	static const BYTE ERROR_NO_FILE			= 1;	//找不到文件
	static const BYTE ERROR_READ_FILE		= 2;	//文件读取失败
	static const BYTE ERROR_FIND_HUGAO		= 3;	//找不到呼高
	static const BYTE ERROR_FIND_LEG		= 4;	//找不到配腿
	static const BYTE ERROR_FIND_INSTANCE	= 5;	//找不到塔例
public:
	CTMDHModel();
	~CTMDHModel();
	//
	GECS BuildTransCS(void* pInstance=NULL);
	GECS BuildRotateCS();
	CXhChar100 GetFileName(void* pInstance);
	void* ExtractActiveTaInstance();
	BOOL ExtractActiveTaInstance(char* sHuGao,double fLegA,double fLegB,double fLegC,double fLegD);
	//
	void CreateXmlFile(ITidTowerInstance* pInstance,const char* sFileName);
	void CreateXmlFile(IModTowerInstance* pInstance,const char* sFileName);
	void CreateModFile(ITidTowerInstance* pInstance,const char* sFileName);
	void CreateModFile(IModTowerInstance* pInstance,const char* sFileName);
	void Create3dsFile(ITidTowerInstance* pInstance,const char* sFileName);
	void Create3dsFile(IModTowerInstance* pInstance,const char* sFileName);
	void CreateDxfFile(ITidTowerInstance* pInstance,const char* sFileName);
	void CreateDxfFile(IModTowerInstance* pInstance,const char* sFileName);
	void CreateStlFile(ITidTowerInstance* pInstance,const char* sFileName);
	void CreateStlFile(IModTowerInstance* pInstance,const char* sFileName);
};
//////////////////////////////////////////////////////////////////////////
//
class CTMDHTower{
public:
	CTower m_xTower;
	CXhChar50  m_sTower;
	CXhChar200 m_sPath;
	int m_nMaxLegs;
public:
	CTMDHTower();
	~CTMDHTower();
	//
	bool ReadLdsFile(const char* sFileName);
	void CreateTidFile(const char* sFileName);
	void CreateModFile(const char* sFileName);
	void Create3dsFile(const char* sFileName);
	void CreateStlFile(const char* sFileName);
};
//////////////////////////////////////////////////////////////////////////
//CTMDHGim
class CModFileError
{
public:
	static const int ERROR_MOD_DATA				=1;
	static const int ERROR_MOD_DATA_MISS		=2;
	static const int ERROR_MOD_LAYOUT			=3;
	//
	static const int ERROR_MOD_HNUM				=101;//塔例个数错误
	static const int ERROR_MOD_BODY_H			=102;//本体高度错误
	static const int ERROR_MOD_LEG_H			=103;//接腿高度错误
	static const int ERROR_MOD_SUBLEG_H			=104;//子腿高度错误
	//
	static const int ERROR_MOD_HNUM_MISS		=201;//缺少塔型个数
	static const int ERROR_MOD_HBODY_MISS		=202;//缺少本体高度
	static const int ERROR_MOD_HLEG_MISS		=203;//缺少接腿长
	static const int ERROR_MOD_HSUBLEG_MISS		=204;//缺少子腿长
	static const int ERROR_MOD_H_MISS			=205;//塔例数据缺失
	static const int ERROR_MOD_R_L_MISS			=206;//角钢数据缺失
	static const int ERROR_MOD_R_T_MISS			=207;//钢管数据缺失
	static const int ERROR_MOD_P_MISS			=208;//点数据缺失
	static const int ERROR_MOD_G_MISS			=209;//挂点数据缺失
	//
	static const int ERROR_MOD_LAYOUT_BODY			=301;//body声明缺失
	static const int ERROR_MOD_LAYOUT_LEG			=302;//leg声明缺失
	static const int ERROR_MOD_LAYOUT_SUNLEG		=303;//subleg声明缺失
	static const int ERROR_MOD_LAYOUT_BODY_H		=304;//body高度数据行缺失
	static const int ERROR_MOD_LAYOUT_LEG_H			=305;//leg高度数据行缺失
	static const int ERROR_MOD_LAYOUT_SUNLEG_H		=306;//subleg高度数据行缺失
	static const int ERROR_MOD_LAYOUT_H				=307;//塔例数据格式错误
	static const int ERROR_MOD_LAYOUT_P				=308;//点数据格式错误
	static const int ERROR_MOD_LAYOUT_R				=309;//杆件数据格式错误(未知类型杆件不会读取)
	static const int ERROR_MOD_LAYOUT_R_L			=310;//角钢数据格式错误
	static const int ERROR_MOD_LAYOUT_R_T			=311;//钢管数据格式错误
	static const int ERROR_MOD_LAYOUT_G				=312;//挂点数据格式错误
	//
	static void SetError(int nType);
	static void SetError(int nType,int nItemNoOrRow,int nSubLegNo=0);
	static void SetError(int nType,const char* sError);//参数错误
};
class CTMDHGim
{
private:
	void ToBuffer(CBuffer &buffer,CBuffer &zipBuffer);
	void FromBuffer(CBuffer &buffer,CBuffer &zipBuffer);
public:
	static const BYTE ANSI_CODING	= 0;
	static const BYTE UTF8_CODING	= 1;
	//
	BYTE m_ciCodingType;	//编码格式:0.ANSI|1.UTF-8
	GIM_HEAD_PROP_ITEM m_xGimHeadProp;
	TOWER_PRPERTY_ITEM m_xTowerProp;
	CXhChar100 m_sModPath;	//原始MOD的路径
	CXhChar50 m_sGimName;
	CXhChar100 m_sOutputPath;
	CXhChar100 m_sModName;
	CXhChar100 m_sPhmGuid;		//杆塔组合模型文件.phm
	CXhChar100 m_sDevGuid;		//杆塔物理模型文件.dev
	CXhChar100 m_sFamGuid;		//杆塔属性文件.fam
	CXhChar100 m_sCbmGuid;		//杆塔设备工程文件.cbm
	//配基信息
	CXhChar200 m_sModFile;		//MOD文件完整路径
	CXhChar50 m_sHuGao;
	UINT m_uiHeightSerial;
	UINT m_uiLegSerialArr[4];
public:
	CTMDHGim();
	~CTMDHGim();
	//
	void WriteModFile(const char* sModFilePath);
	void WritePhmFile(const char* sPhmFile,char* sModName);
	void WriteDevFile(const char* sDevFile);
	void WriteFamFile(const char* sFamFile);
	void WriteCbmFile(const char* sCbmFile);
	//
	void ReadDevFile(const char* sDevFile);
	void ReadFamFile(const char* sFamFile);
	void ReadPhmFile(const char* sPhmFile);
	void ReadCbmFile(const char* sCbmFile);
	//
	void CheckModData();
	void CreateGuidFile();
	void DeleteGuidFile();
	BOOL PackGimFile();
	BOOL UpdateGimHeadInfo();
	BOOL UpdateGimTowerPro();
	//
	void* ExtractActiveTaInstance();
	//属性
	DECLARE_PROP_FUNC(CTMDHGim);
	int GetPropValueStr(long id,char* valueStr,UINT nMaxStrBufLen=100);	//根据属性ID得到属性值字符串
};
//////////////////////////////////////////////////////////////////////////
//CTMDHPrjGim
class CTMDHPrjGim
{
	ATOM_LIST<CTMDHGim> m_xDevList;
public:
	const static BYTE ALL_FILE	= 0;
	const static BYTE CBM_FILE	= 1;
	const static BYTE DEV_FILE	= 2;
	const static BYTE PHM_FILE	= 3;
	const static BYTE FAM_FILE	= 4;
	const static BYTE MOD_FILE	= 5;
	//
	CXhChar50 m_sGimName;
	CXhChar100 m_sGimPath;
	GIM_HEAD_PROP_ITEM m_xGimHeadProp;
protected:
	BOOL IsTowerDevFile(const char* sDevFile);
	void ReadGimHeadInfo(CBuffer &buffer);
	void DeleteGimFolder(const char* sFolder);
	void SearchFile(ATOM_LIST<CString>& xFileList,CXhChar200 sFilePath,BYTE ciFileType=0);
public:
	CTMDHPrjGim();
	~CTMDHPrjGim();
	//
	void UnpackGimFile(const char* sFolder);
	BOOL UpdateGimTowerPro();
	//数据操作
	CTMDHGim* EnumFirstGim(){return m_xDevList.GetFirst();}
	CTMDHGim* EnumNextGim(){return m_xDevList.GetNext();}
	int GetGimNum(){return m_xDevList.GetNodeNum();}
	BOOL ParseGimFile(const char* gim_path);
};
//////////////////////////////////////////////////////////////////////////
//CTMDHManager
class CTMDHManager{
	CTower* m_pActiveTa;
	//
	ATOM_LIST<CTMDHTower> m_xTowerDataList;
	ATOM_LIST<CTMDHModel> m_xModelDataList;
	ATOM_LIST<CTMDHGim> m_xGimDataList;
	ATOM_LIST<CTMDHPrjGim> m_xPrjGimList;
public:
	BYTE m_ciInstanceLevel;	//0.LOD0单线|1.LOD1杆件|2.LOD2杆件+板|3.LOD3放样模型
	BOOL m_bUseUnitM;		//是否启用长度单位米（默认为毫米）
	ITidModel* m_pTidModel;
	IModModel* m_pModModel;
protected:
	BOOL ParseTaModSheetContent(CVariant2dArray &sheetContentMap);
public:
	CTMDHManager();
	~CTMDHManager();
	//
	static BOOL Run7zCmd(char* sCmd);
	//进度显示回调函数
	void (*DisplayProcess)(int percent,const char *sTitle);	
	//
	void SetActiveTa(CTower* pTa){m_pActiveTa=pTa;}
	CTower* GetActiveTa(){return m_pActiveTa;}
	//LDS数据操作
	int TowerNum(){return m_xTowerDataList.GetNodeNum();}
	CTMDHTower* AppendTA(){return m_xTowerDataList.append();}
	CTMDHTower* EnumFirstTA(){return m_xTowerDataList.GetFirst();}
	CTMDHTower* EnumNextTA(){return m_xTowerDataList.GetNext();}
	void DeleteTa(CTMDHTower* pTower);
	//MOD|TID数据操作
	BOOL ReadBatchInstanceFile(const char* sFilePath);
	int ModelNum(){return m_xModelDataList.GetNodeNum();}
	CTMDHModel* AppendModel(){return m_xModelDataList.append();}
	CTMDHModel* EnumFirstModel(){return m_xModelDataList.GetFirst();}
	CTMDHModel* EnumNextModel(){return m_xModelDataList.GetNext();}
	void DeleteModel(CTMDHModel* pModel);
	//MOD打包数据操作
	BOOL ReadTaModBatchGzipFile(const char* sFilePath);
	int GimNum(){return m_xGimDataList.GetNodeNum();}
	CTMDHGim* AppendGim(){return m_xGimDataList.append();}
	CTMDHGim* EnumFirstGim(){return m_xGimDataList.GetFirst();}
	CTMDHGim* EnumNextGim(){return m_xGimDataList.GetNext();}
	void DeleteGim(CTMDHGim* pGim);
	//
	int PrjGimNum(){return m_xPrjGimList.GetNodeNum();}
	CTMDHPrjGim* AppendPrjGim(){return m_xPrjGimList.append();}
	void DeletePrjGim(CTMDHPrjGim* pPrjGim);
};
extern CTMDHManager Manager;
extern CLogFile xMyErrLog;