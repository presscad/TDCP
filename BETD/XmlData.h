#pragma once
#include "f_ent.h"
#include "ArrayList.h"
#include "Markup.h"
#include "XhCharString.h"
#include "HashTable.h"
//�ҵ���Ϣ
struct HANG_POINT
{
	char m_ciWireType;		//������:'C'����|'E'����|'J'����
	char m_ciHangingStyle;	//�Ҵ����ͣ�0.����|'S'˫��|'V'V��
	BYTE m_ciTensionType;		//-1.δ��ֵ;0.����|1.����
	char m_ciHangDirect;
	char m_sHangOrder[16];
	char m_sHangName[50];	//�ҵ�����
	GEPOINT m_xHangPos;		//λ��
	HANG_POINT(){
		m_ciWireType='C';
		m_ciHangingStyle=0;
		m_ciTensionType=-1;
		strcpy(m_sHangOrder,"");
		strcpy(m_sHangName,"");
	}
	void CopyMember(HANG_POINT* pSrcHangPt,BOOL bReverse=FALSE)
	{
		m_ciWireType = pSrcHangPt->m_ciWireType;
		m_ciHangingStyle = pSrcHangPt->m_ciHangingStyle;
		m_ciTensionType = pSrcHangPt->m_ciTensionType;
		if (bReverse)
		{
			if (pSrcHangPt->m_ciHangDirect == 'Q')
				m_ciHangDirect = 'H';
			else if(pSrcHangPt->m_ciHangDirect == 'H')
				m_ciHangDirect = 'Q';
		}
		else
			m_ciHangDirect = pSrcHangPt->m_ciHangDirect;
		m_xHangPos = pSrcHangPt->m_xHangPos;
	}
};
class CHangPhase
{
public:
	DWORD m_dwHangPhase;		//�����
	ARRAY_LIST<HANG_POINT> m_xBackHangPtArr;	//���ҵ�
	ARRAY_LIST<HANG_POINT> m_xFrontHangPtArr;	//ǰ��ҵ�
	ARRAY_LIST<HANG_POINT> m_xWireJHangPtArr;	//���߹ҵ�
public:
	CHangPhase();
	//
	void SetKey(DWORD key){m_dwHangPhase=key;}
	BOOL GetBKVirtualHangPt(HANG_POINT& hang_pt_v);
	BOOL GetFTVirtualHangPt(HANG_POINT& hang_pt_v);
	BOOL GetWJVirtualHangPt(HANG_POINT& hang_pt_v);
};
//��������·��Ϣ
class CTowerLoop
{
public:
	DWORD m_dwLoop;		//
	CXhChar50 m_sCurrent;	//
	CHashListEx<CHangPhase> m_hashHangPhase;	//�ർ��
public:
	void SetKey(DWORD key){m_dwLoop=key;}
};
//
class CEarthLoop
{
public:
	DWORD m_dwLoop;		//
	ARRAY_LIST<HANG_POINT> m_xBackHangPtList;	//���߹ҵ�
	ARRAY_LIST<HANG_POINT> m_xFrontHangPtList;	//���߹ҵ�
public:
	void SetKey(DWORD key){m_dwLoop=key;}
	BOOL GetBKVirtualHangPt(HANG_POINT& hang_pt_v);
	BOOL GetFTVirtualHangPt(HANG_POINT& hang_pt_v);
};
//����
struct FUNDATION
{
	char m_sFeetName[50];
	GEPOINT m_xFeetPos;
	FUNDATION(){strcpy(m_sFeetName,"");}
};
//
class CXmlModel
{
public:
	BOOL m_bTransPtMMtoM;
	BYTE m_ciModel;		//
	BYTE get_ciHangingType();		//0.����|1.����
	__declspec(property(get=get_ciHangingType)) BYTE ciHangingType;	//����������������ԣ�0.����|1.����
	CXhChar50 m_sCategory;	//���
	CXhChar50 m_sTowerFile;
	CXhChar50 m_sTowerName;
	CXhChar50  m_sPropName;		//��������
	CXhChar100 m_sPropValue;	//����ֵ
	FUNDATION m_xFundation[4];
	CHashListEx<CTowerLoop> m_hashTowerLoop;	//���߻�·
	CHashListEx<CEarthLoop> m_hashEarthLoop;	//���߻�·
protected:
	void AddEarthLoop(CMarkup& xml,CEarthLoop* pEarthLoop);
	void AddWireLoop(CMarkup& xml,CTowerLoop* pWireLoop);
	void AddHangPoint(CMarkup& xml,HANG_POINT* pHangPt,ARRAY_LIST<HANG_POINT>* pHangPtArr=NULL);
public:
	CXmlModel(BOOL bUseUnitM=FALSE);
	~CXmlModel();
	//
	void AmendHangInfo(BOOL bTurnLeft=FALSE);
	void CreateOldXmlFile(const char* sFilePath);
};