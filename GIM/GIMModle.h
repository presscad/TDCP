#pragma once

#include "XhCharString.h"
#include "ArrayList.h"
#include "Variant.h"
#include "Buffer.h"
#include "LogFile.h"

struct GIM_HEAD_PROP_ITEM
{
	char m_sFileTag[16];		//�ļ���ʶ
	char m_sFileName[256];		//�ļ�����
	char m_sDesigner[64];		//�����
	char m_sUnit[256];			//��֯��λ
	char m_sSoftName[128];		//�������
	char m_sTime[16];			//����ʱ��
	char m_sSoftMajorVer[8];	//������汾��
	char m_sSoftMinorVer[8];	//����ΰ汾��
	char m_sMajorVersion[8];	//��׼���汾��
	char m_sMinorVersion[8];	//��׼�ΰ汾��
	char m_sBufSize[8];			//�洢�����С
	GIM_HEAD_PROP_ITEM()
	{
		memset(this,0,sizeof(GIM_HEAD_PROP_ITEM));
		strcpy(m_sFileTag,"GIMPKGT");
		strcpy(m_sSoftName,"�ź������ƻ�ͼһ�廯���");
		//strcpy(m_sSoftName,"������ά���ϵͳ");
	}
};
struct TOWER_PRPERTY_ITEM
{
	CXhChar100 m_sVoltGrade;		//��ѹ�ȼ�(��λKV)
	CXhChar100 m_sType;				//�ͺ�
	CXhChar100 m_sTexture;			//�������ʣ��Ǹ���|�ֹ���
	CXhChar100 m_sFixedType;		//�̶���ʽ������
	CXhChar100 m_sTaType;			//��������
	int m_nCircuit;					//��·��
	CXhChar100 m_sCWireSpec;		//�����ͺ�
	CXhChar100 m_sEWireSpec;		//�����ͺ�
	double m_fWindSpeed;			//��������
	double m_fNiceThick;			//�������
	CXhChar100 m_sWindSpan;			//ˮƽ����
	CXhChar100 m_sWeightSpan;		//��ֱ����
	double m_fFrontRulingSpan;		//ǰ�������
	double m_fBackRulingSpan;		//��������
	double m_fMaxSpan;				//��󵵾�
	CXhChar100 m_sAngleRange;		//ת�Ƿ�Χ
	double m_fDesignKV;				//���Kvֵ
	CXhChar100 m_sRatedHeight;		//�������
	CXhChar500 m_sHeightRange;		//���߷�Χ
	CXhChar500 m_sTowerWeight;		//����
	double m_fFrequencyRockAngle;	//��Ƶҡ�ڽǶ�
	double m_fLightningRockAngle;	//�׵�ҡ�ڽǶ�
	double m_fSwitchingRockAngle;	//����ҡ�ڽǶ�
	double m_fWorkingRockAngle;		//������ҵҡ�ڽǶ�
	CXhChar100 m_sManuFacturer;		//��������
	CXhChar100 m_sMaterialCode;		//���ʱ���
	CXhChar100 m_sProModelCode;		//װ��ģ�ͱ��
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
	BYTE m_ciCodingType;	//�����ʽ:0.ANSI|1.UTF-8
	GIM_HEAD_PROP_ITEM m_xGimHeadProp;
	TOWER_PRPERTY_ITEM m_xTowerProp;
	CXhChar50 m_sModName;
	CXhChar100 m_sModPath;
	CXhChar100 m_sOutputPath;
	CXhChar100 m_sPhmGuid;		//�������ģ���ļ�.phm
	CXhChar100 m_sDevGuid;		//��������ģ���ļ�.dev
	CXhChar100 m_sFamGuid;		//���������ļ�.fam
	CXhChar100 m_sCbmGuid;		//�����豸�����ļ�.cbm
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