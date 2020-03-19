#pragma once
#include "HashTable.h"
#include "DesignParamItem.h"
#include "Tower.h"

enum MODEL_PARAM_TYPE {
	TYPE_SZC = 1,
	TYPE_JG = 2,
	TYPE_SJ = 3,
	TYPE_SJB = 4,
	TYPE_SJC = 5,
	TYPE_ZBC = 6,
	TYPE_ZC = 7,
	TYPE_ZM = 8,
	TYPE_SZ = 9,
	TYPE_JC = 10,
	TYPE_JG2 = 11,
	TYPE_JBB = 12,
};
class CDesParasOper
{
	CHashList<DESIGN_PARAM_ITEM>* m_pHashDesParams;
public:
	CDesParasOper(CHashList<DESIGN_PARAM_ITEM>* pHashDesParams)
	{
		m_pHashDesParams = pHashDesParams;
		m_pHashDesParams->Empty();
	}
	~CDesParasOper(void) { ; }
	void AddParamValue(KEY4C key, double fValue, UINT id, char* sLabel = NULL)
	{
		DESIGN_PARAM_ITEM* pParam = m_pHashDesParams->Add(key);
		pParam->key = key;
		pParam->value.fValue = fValue;
		pParam->uiCategoryId = id;
		if (sLabel)
			pParam->sLabel.Copy(sLabel);
	}
};
class CParaSketchOper
{
	CHashList<PARA_SKETCH_ITEM>* m_pHashParaSchemas;
public:
	CParaSketchOper(CHashList<PARA_SKETCH_ITEM>* pHashParaSchemas)
	{
		m_pHashParaSchemas = pHashParaSchemas;
		m_pHashParaSchemas->Empty();
	}
	~CParaSketchOper(void) { ; }
	void AddItemValue(KEY4C key, BYTE ciVect, long hNode1, long hNode2 = 0)
	{
		PARA_SKETCH_ITEM* pItem = m_pHashParaSchemas->Add(key);
		pItem->key = key;
		pItem->m_ciSketchVec = ciVect;
		pItem->m_hNode1 = hNode1;
		pItem->m_hNode2 = hNode2;
	}
};
class CTowerTemplateToModel
{
	static CTower* m_pTower;
	static bool InitSZCTowerTmplInfo();
	static bool InitJGTowerTmplInfo();
	static bool InitJG2TowerTmplInfo();
	static bool InitSJTowerTmplInfo();
	static bool InitSJBTowerTmplInfo();
	static bool InitSJCTowerTmplInfo();
	static bool InitZBCTowerTmplInfo();
	static bool InitZCTowerTmplInfo();
	static bool InitZMTowerTmplInfo();
	static bool InitSZTowerTmplInfo();
	static bool InitJCTowerTmplInfo();
	static bool InitJBBTowerTmplInfo();
public:
	static void InitTowerTempParas(BYTE ciTowerType, CTower* pTower);
	static bool UpdateTowerSlopeDesPara(BYTE ciTowerType, CTower* pTower);
	static void InitTempParaSketchs(BYTE ciTowerType, CHashList<PARA_SKETCH_ITEM>& hashParaSchemas);
};
CXhChar50 GetTempLdsFile(BYTE ciTowerType);
