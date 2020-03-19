#pragma once
#include "ModCore.h"
#include "TidCplus.h"
#include "I3DS.h"
#include "XhCharString.h"
#include "LogFile.h"
#ifdef _HANDOVER_TO_CLIENT_
#include "AtomList.h"
#else
#include "f_ent_list.h"	//��Ҫ��ATOM_LIST��
#endif

extern CLogFile xMyErrLog;
//////////////////////////////////////////////////////////////////////////
//CTowerModle����(����߶����)
class CTowerManager;
class CTowerModel
{
	CTowerManager* m_pTaManager;
public:
	CXhChar100 m_sFilePath;
	CXhChar50 m_sTowerPlaceCode;	//��λ��
	CXhChar50 m_sTowerDot;	//��λ��
	CXhChar50 m_sTowerType;	//����
	CXhChar50 m_sHuGao;		//����
	double m_fLocateH;		//��λ��
	double m_fBodyHeight;	//�����
	double m_fMaxLegHeight;	//��ȸ�
	BOOL m_bTurnLeft;	//�Ƿ���ת�������������ʾ����ϵ������
	UINT m_uiHeightSerial;
	UINT m_uiLegSerialArr[4];
	BYTE m_ciModelFlag;	//0.TID|1.MOD
	//
	static const int TYPE_TID	=0;
	static const int TYPE_MOD	=1;	
	//
	BYTE m_ciErrorType;
	static const BYTE ERROR_NO_FILE			= 1;	//�Ҳ����ļ�
	static const BYTE ERROR_READ_FILE		= 2;	//�ļ���ȡʧ��
	static const BYTE ERROR_FIND_HUGAO		= 3;	//�Ҳ�������
	static const BYTE ERROR_FIND_LEG		= 4;	//�Ҳ�������
	static const BYTE ERROR_FIND_INSTANCE	= 5;	//�Ҳ�������
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
	BYTE m_ciInstanceLevel;	//0.LOD0����|1.LOD1�˼�|2.LOD2�˼�+��|3.LOD3����ģ��
	BOOL m_bUseUnitM;		//�Ƿ����ó��ȵ�λ�ף�Ĭ��Ϊ���ף�
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