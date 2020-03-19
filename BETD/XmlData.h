#pragma once
#include "f_ent.h"
#include "ArrayList.h"
#include "Markup.h"
#include "XhCharString.h"
#include "HashTable.h"
//挂点信息
struct HANG_POINT
{
	char m_ciWireType;		//线类型:'C'导线|'E'地线|'J'跳线
	char m_ciHangingStyle;	//挂串类型：0.单挂|'S'双挂|'V'V挂
	BYTE m_ciTensionType;		//-1.未赋值;0.悬垂|1.耐张
	char m_ciHangDirect;
	char m_sHangOrder[16];
	char m_sHangName[50];	//挂点名称
	GEPOINT m_xHangPos;		//位置
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
	DWORD m_dwHangPhase;		//相序号
	ARRAY_LIST<HANG_POINT> m_xBackHangPtArr;	//后侧挂点
	ARRAY_LIST<HANG_POINT> m_xFrontHangPtArr;	//前侧挂点
	ARRAY_LIST<HANG_POINT> m_xWireJHangPtArr;	//跳线挂点
public:
	CHangPhase();
	//
	void SetKey(DWORD key){m_dwHangPhase=key;}
	BOOL GetBKVirtualHangPt(HANG_POINT& hang_pt_v);
	BOOL GetFTVirtualHangPt(HANG_POINT& hang_pt_v);
	BOOL GetWJVirtualHangPt(HANG_POINT& hang_pt_v);
};
//铁塔输电回路信息
class CTowerLoop
{
public:
	DWORD m_dwLoop;		//
	CXhChar50 m_sCurrent;	//
	CHashListEx<CHangPhase> m_hashHangPhase;	//相导线
public:
	void SetKey(DWORD key){m_dwLoop=key;}
};
//
class CEarthLoop
{
public:
	DWORD m_dwLoop;		//
	ARRAY_LIST<HANG_POINT> m_xBackHangPtList;	//地线挂点
	ARRAY_LIST<HANG_POINT> m_xFrontHangPtList;	//地线挂点
public:
	void SetKey(DWORD key){m_dwLoop=key;}
	BOOL GetBKVirtualHangPt(HANG_POINT& hang_pt_v);
	BOOL GetFTVirtualHangPt(HANG_POINT& hang_pt_v);
};
//基础
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
	BYTE get_ciHangingType();		//0.悬垂|1.耐张
	__declspec(property(get=get_ciHangingType)) BYTE ciHangingType;	//杆塔的整体挂线属性，0.悬垂|1.耐张
	CXhChar50 m_sCategory;	//类别
	CXhChar50 m_sTowerFile;
	CXhChar50 m_sTowerName;
	CXhChar50  m_sPropName;		//属性名称
	CXhChar100 m_sPropValue;	//属性值
	FUNDATION m_xFundation[4];
	CHashListEx<CTowerLoop> m_hashTowerLoop;	//导线回路
	CHashListEx<CEarthLoop> m_hashEarthLoop;	//地线回路
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