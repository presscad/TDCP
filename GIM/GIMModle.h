#pragma once

#include "XhCharString.h"
#include "ArrayList.h"
#include "Variant.h"
#include "Buffer.h"
#include "LogFile.h"

struct GIM_HEAD_PROP_ITEM
{
	char m_sFileTag[16];		//文件标识
	char m_sFileName[256];		//文件名称
	char m_sDesigner[64];		//设计者
	char m_sUnit[256];			//组织单位
	char m_sSoftName[128];		//软件名称
	char m_sTime[16];			//创建时间
	char m_sSoftMajorVer[8];	//软件主版本号
	char m_sSoftMinorVer[8];	//软件次版本号
	char m_sMajorVersion[8];	//标准主版本号
	char m_sMinorVersion[8];	//标准次版本号
	char m_sBufSize[8];			//存储区域大小
	GIM_HEAD_PROP_ITEM()
	{
		memset(this,0,sizeof(GIM_HEAD_PROP_ITEM));
		strcpy(m_sFileTag,"GIMPKGT");
		strcpy(m_sSoftName,"信狐天诚设计绘图一体化软件");
		//strcpy(m_sSoftName,"道亨三维设计系统");
	}
};
struct TOWER_PRPERTY_ITEM
{
	CXhChar100 m_sVoltGrade;		//电压等级(单位KV)
	CXhChar100 m_sType;				//型号
	CXhChar100 m_sTexture;			//杆塔材质：角钢塔|钢管塔
	CXhChar100 m_sFixedType;		//固定方式：自立
	CXhChar100 m_sTaType;			//杆塔类型
	int m_nCircuit;					//回路数
	CXhChar100 m_sCWireSpec;		//导线型号
	CXhChar100 m_sEWireSpec;		//地线型号
	double m_fWindSpeed;			//基本风速
	double m_fNiceThick;			//覆冰厚度
	CXhChar100 m_sWindSpan;			//水平档距
	CXhChar100 m_sWeightSpan;		//垂直档距
	double m_fFrontRulingSpan;		//前侧代表档距
	double m_fBackRulingSpan;		//后侧代表档距
	double m_fMaxSpan;				//最大档距
	CXhChar100 m_sAngleRange;		//转角范围
	double m_fDesignKV;				//设计Kv值
	CXhChar100 m_sRatedHeight;		//计算呼高
	CXhChar500 m_sHeightRange;		//呼高范围
	CXhChar500 m_sTowerWeight;		//塔重
	double m_fFrequencyRockAngle;	//工频摇摆角度
	double m_fLightningRockAngle;	//雷电摇摆角度
	double m_fSwitchingRockAngle;	//操作摇摆角度
	double m_fWorkingRockAngle;		//带电作业摇摆角度
	CXhChar100 m_sManuFacturer;		//生产厂家
	CXhChar100 m_sMaterialCode;		//物资编码
	CXhChar100 m_sProModelCode;		//装配模型编号
	TOWER_PRPERTY_ITEM()
	{
		m_nCircuit=0;
		m_fWindSpeed=0;
		m_fNiceThick=0;
		m_fFrontRulingSpan=0;
		m_fBackRulingSpan=0;
		m_fMaxSpan=0;
		m_fDesignKV=0;
		m_fFrequencyRockAngle=0;
		m_fLightningRockAngle=0;
		m_fSwitchingRockAngle=0;
		m_fWorkingRockAngle=0;
	}
};
class CTowerGim
{
	static const BYTE ANSI_CODING	= 0;
	static const BYTE UTF8_CODING	= 1;
private:
	void WriteTowerModFile(const char* sModFilePath);
	void WriteTowerPhmFile(const char* sPhmFile,char* sModName);
	void WriteTowerDevFile(const char* sDevFile);
	void WriteTowerFamFile(const char* sFamFile);
	void WriteTowerCbmFile(const char* sCbmFile);
public:
	BYTE m_ciCodingType;	//编码格式:0.ANSI|1.UTF-8
	GIM_HEAD_PROP_ITEM m_xGimHeadProp;
	TOWER_PRPERTY_ITEM m_xTowerProp;
	CXhChar50 m_sModName;
	CXhChar100 m_sModPath;
	CXhChar100 m_sOutputPath;
	CXhChar100 m_sPhmGuid;		//杆塔组合模型文件.phm
	CXhChar100 m_sDevGuid;		//杆塔物理模型文件.dev
	CXhChar100 m_sFamGuid;		//杆塔属性文件.fam
	CXhChar100 m_sCbmGuid;		//杆塔设备工程文件.cbm
public:
	CTowerGim();
	~CTowerGim();
	//
	void CreateGuidFile();
	void DeleteGuidFile();
	BOOL PackTowerGimFile();
	BOOL UpdateGimFile();
	void ToBuffer(CBuffer &buffer,CBuffer &zipBuffer);
	void FromBuffer(CBuffer &buffer,CBuffer &zipBuffer);
};
//
class CGimModle
{
	ARRAY_LIST<CTowerGim> m_listTowerGim;
	//
	BOOL ParseTaModSheetContent(CVariant2dArray &sheetContentMap);
public:
	CGimModle();
	~CGimModle();
	//
	static BOOL Run7zCmd(char* sCmd);
	//
	BOOL ReadTaModBatchGzipFile(const char* sFilePath);
	int GetCount(){return m_listTowerGim.GetSize();}
	void Empty(){return m_listTowerGim.Empty();}
	CTowerGim* Append(){return m_listTowerGim.append();}
	CTowerGim* EnumFirTowerGim(){return m_listTowerGim.GetFirst();}
	CTowerGim* EnumNextTowerGim(){return m_listTowerGim.GetNext();}
};
extern CGimModle gim_modle;