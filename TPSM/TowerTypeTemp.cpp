#include "stdafx.h"
#include "TowerTypeTemp.h"

CXhChar50 GetTempLdsFile(BYTE ciTowerType)
{
	CXhChar50 sLdsFileName;
	if (ciTowerType == TYPE_SZC)
		sLdsFileName.Copy("5D1-SZC2.lds");
	else if (ciTowerType == TYPE_JG)
		sLdsFileName.Copy("5B1-JG1.lds");
	else if (ciTowerType == TYPE_SJ)
		sLdsFileName.Copy("SJ452.lds");
	else if (ciTowerType == TYPE_SJB)
		sLdsFileName.Copy("SJB4153.lds");
	else if (ciTowerType == TYPE_SJC)
		sLdsFileName.Copy("5D1-SJC1.lds");
	else if (ciTowerType == TYPE_ZBC)
		sLdsFileName.Copy("5B1-ZBC1.lds");
	else if (ciTowerType == TYPE_ZC)
		sLdsFileName.Copy("ZC30101A.lds");
	else if (ciTowerType == TYPE_ZM)
		sLdsFileName.Copy("5B1-ZM2.lds");
	else if (ciTowerType == TYPE_SZ)
		sLdsFileName.Copy("SZ162.lds");
	else if (ciTowerType == TYPE_JC)
		sLdsFileName.Copy("JC.lds");
	else if (ciTowerType == TYPE_JG2)
		sLdsFileName.Copy("JG2.lds");
	else if (ciTowerType == TYPE_JBB)
		sLdsFileName.Copy("JBB351.lds");
	return sLdsFileName;
}
//////////////////////////////////////////////////////////////////////////
//CTowerTemplateToModel
CTower* CTowerTemplateToModel::m_pTower = NULL;
void CTowerTemplateToModel::InitTowerTempParas(BYTE ciTowerType, CTower* pTower)
{
	if (pTower == NULL)
		return;
	m_pTower = pTower;
	if (ciTowerType == TYPE_SZC)
		InitSZCTowerTmplInfo();
	else if (ciTowerType == TYPE_JG)
		InitJGTowerTmplInfo();
	else if (ciTowerType == TYPE_JG2)
		InitJG2TowerTmplInfo();
	else if (ciTowerType == TYPE_SJ)
		InitSJTowerTmplInfo();
	else if (ciTowerType == TYPE_SJB)
		InitSJBTowerTmplInfo();
	else if (ciTowerType == TYPE_SJC)
		InitSJCTowerTmplInfo();
	else if (ciTowerType == TYPE_ZBC)
		InitZBCTowerTmplInfo();
	else if (ciTowerType == TYPE_ZC)
		InitZCTowerTmplInfo();
	else if (ciTowerType == TYPE_ZM)
		InitZMTowerTmplInfo();
	else if (ciTowerType == TYPE_SZ)
		InitSZTowerTmplInfo();
	else if (ciTowerType == TYPE_JC)
		InitJCTowerTmplInfo();
	else if (ciTowerType == TYPE_JBB)
		InitJBBTowerTmplInfo();
}
//同塔双回路直线塔-SZC塔型
bool CTowerTemplateToModel::InitSZCTowerTmplInfo()
{
	if (m_pTower == NULL)
		return false;
	//模型参数
	CDesParasOper oper(&m_pTower->hashParams);	
	oper.AddParamValue(KEY4C("Z0"), 3500, 0);		//后台隐藏参数，不显示
	oper.AddParamValue(KEY4C("Z"), 600, 1, "顶面高差");
	oper.AddParamValue(KEY4C("Z1"), 2300, 1, "地面支架高");
	oper.AddParamValue(KEY4C("Z2"), 12400, 1, "上中层高");
	oper.AddParamValue(KEY4C("Z3"), 11250, 1, "中下层高");
	oper.AddParamValue(KEY4C("Z4"), 3000, 1, "下层至变坡高");
	oper.AddParamValue(KEY4C("Z5"), 42000, 1, "呼高");
	oper.AddParamValue(KEY4C("K1"), 4000, 1, "变坡口宽");
	oper.AddParamValue(KEY4C("K2"), 1900, 1, "顶面宽");
	oper.AddParamValue(KEY4C("K3"), 11800, 1, "基础正面根开");
	oper.AddParamValue(KEY4C("P1"), 0.04, 1, "变坡上侧坡度");
	oper.AddParamValue(KEY4C("P2"), 0.1, 1, "变坡下侧坡度");
	oper.AddParamValue(KEY4C("L"), 0, 1, "绝缘子串长度");
	oper.AddParamValue(KEY4C("A"), 49.573921, 1, "地线保护角");
	//第一层导地线复合横担TX11
	oper.AddParamValue(KEY4C("XA1"), 10400, 2, "地线挂点中心距");
	oper.AddParamValue(KEY4C("XA2"), 7700, 2, "上导挂点中心距");
	oper.AddParamValue(KEY4C("XA3"), 800, 2, "地线架悬臂长");
	oper.AddParamValue(KEY4C("YA1"), 600, 2, "地线架纵向宽");
	oper.AddParamValue(KEY4C("YA2"), 1200, 2, "上导横担纵向宽");
	oper.AddParamValue(KEY4C("ZA1"), 2200, 2, "复合横担根部高");
	oper.AddParamValue(KEY4C("ZA2"), 600, 2, "地线架悬臂高");
	oper.AddParamValue(KEY4C("ZA3"), 600, 2, "上导横担根部高");
	//第二层导线横担HX2
	oper.AddParamValue(KEY4C("XB"), 10100, 3, "中导挂点中心距");
	oper.AddParamValue(KEY4C("YB"), 1200, 3, "中导横担纵向宽");
	oper.AddParamValue(KEY4C("ZB"), 2600, 3, "中导横担根部高");
	//第三层导线横担HX2
	oper.AddParamValue(KEY4C("XC"), 8350, 4, "下导挂点中心距");
	oper.AddParamValue(KEY4C("YC"), 1400, 4, "下导横担纵向宽");
	oper.AddParamValue(KEY4C("ZC"), 2400, 4, "下导横担根部高");
	//节点依赖关系
	//塔身关联节点
	SLAVE_NODE* pSlaveNode = NULL;
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x36);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "K2*0.5", 20);
	StrCopy(pSlaveNode->posy_expr, "K2*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x6E);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "Z0+Z2-Z", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x7E);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "Z0+Z2+Z3-Z", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x3A);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "K1*0.5", 20);
	StrCopy(pSlaveNode->posy_expr, "K1*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z2+Z3+Z4-Z", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xC2);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "K3*0.5", 20);
	StrCopy(pSlaveNode->posy_expr, "K3*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z2+Z3+Z4+Z5-Z", 20);
	//第一导导地线横担
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x42);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "Z0+ZA1", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x2A);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "XA1", 20);
	StrCopy(pSlaveNode->posy_expr, "YA1*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0-Z-Z1", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x32);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "Z0-Z-Z1", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x2E);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "XA1-XA3", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x3E);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "XA1-XA3", 20);
	StrCopy(pSlaveNode->posy_expr, "YA1*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0-Z-Z1-ZA2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x52);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "XA2", 20);
	StrCopy(pSlaveNode->posy_expr, "YA2*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0-Z", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x46);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "Z0-Z", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x4e);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "Z0-Z-ZA3", 20);
	//第二导线横担
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x6A);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "Z0+Z2-Z-ZB", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x56);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "XB", 20);
	StrCopy(pSlaveNode->posy_expr, "YB*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z2-Z", 20);
	//第三导线横担
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x7A);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "Z0+Z2+Z3-Z-ZC", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x5A);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "XC", 20);
	StrCopy(pSlaveNode->posy_expr, "YC*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z2+Z3-Z", 20);
	//
	return true;
}
//干字型耐张塔-JC塔型
bool CTowerTemplateToModel::InitJGTowerTmplInfo()
{
	if (m_pTower == NULL)
		return false;
	//模型参数
	CDesParasOper oper(&m_pTower->hashParams);
	oper.AddParamValue(KEY4C("Z0"), 0, 0);		//后台隐藏参数，不显示
	oper.AddParamValue(KEY4C("Z1"), 7000, 1, "地线支架高");
	oper.AddParamValue(KEY4C("Z2"), 7000, 1, "上中层高");
	oper.AddParamValue(KEY4C("Z3"), 4000, 1, "下层至变坡高");
	oper.AddParamValue(KEY4C("Z4"), 36000, 1, "呼高");
	oper.AddParamValue(KEY4C("K1"), 3860, 1, "变坡口宽");
	oper.AddParamValue(KEY4C("K2"), 1700, 1, "顶面宽");
	oper.AddParamValue(KEY4C("K3"), 14740, 1, "基础正面根开");
	oper.AddParamValue(KEY4C("P1"), 0.06, 1, "变坡上侧坡度");
	oper.AddParamValue(KEY4C("P2"), 0.17, 1, "变坡下侧坡度");
	//左上横担HX1
	oper.AddParamValue(KEY4C("XA"), 5500, 2, "地线挂点中心距");
	oper.AddParamValue(KEY4C("YA"), 1700, 2, "地线支架纵向宽");
	oper.AddParamValue(KEY4C("ZAB"), 2000, 2, "地线支架根部高");
	//右上横担HX12
	oper.AddParamValue(KEY4C("XB1"), 5500, 3, "地线挂点中心距");
	oper.AddParamValue(KEY4C("XB2"), 2500, 3, "跳线架控制尺寸");
	oper.AddParamValue(KEY4C("XB3"), 1000, 3, "跳线架控制尺寸");
	oper.AddParamValue(KEY4C("YB"), 1700, 3, "跳线架控制尺寸");
	oper.AddParamValue(KEY4C("YB1"), 4000, 3, "跳线架纵向宽度");
	oper.AddParamValue(KEY4C("YB2"), 4000, 3, "跳线架纵向宽度");
	oper.AddParamValue(KEY4C("ZB"), 1000, 3, "跳线架高度");
	//下层横担
	oper.AddParamValue(KEY4C("ZCD"), 2500, 4, "下导横担根部高");
	//左下横担HX11
	oper.AddParamValue(KEY4C("XC1"), 7500, 4, "左导挂点中心距");
	oper.AddParamValue(KEY4C("XC2"), 2000, 4, "左导跳线架距离");
	oper.AddParamValue(KEY4C("XC3"), 560, 4, "左导挂点间距");
	oper.AddParamValue(KEY4C("YC"), 3380, 4, "左导横担纵向宽");
	oper.AddParamValue(KEY4C("ZC"), 800, 4, "跳线架高度");
	//右下横担HX9
	oper.AddParamValue(KEY4C("XD1"), 7500, 5, "右导挂点中心距");
	oper.AddParamValue(KEY4C("XD2"), 560, 5, "右导挂点间距");
	oper.AddParamValue(KEY4C("YD"), 3380, 5, "右导横担纵向宽");
	oper.AddParamValue(KEY4C("ZD"), 800, 5, "跳线架高度");
	//节点依赖关系
	//塔身主控节点
	SLAVE_NODE* pSlaveNode = NULL;
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x29);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "K2*0.5", 20);
	StrCopy(pSlaveNode->posy_expr, "K2*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x71);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "Z1", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x37);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "Z1+Z2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x33);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "K1*0.5", 20);
	StrCopy(pSlaveNode->posy_expr, "K1*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z1+Z2+Z3", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x7D);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "K3*0.5", 20);
	StrCopy(pSlaveNode->posy_expr, "K3*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z1+Z2+Z4", 20);
	//第一层横担
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x49);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "ZAB", 20);
	//左上横担
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x31);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XA", 20);
	StrCopy(pSlaveNode->posy_expr, "YA*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0", 20);
	//右上横担
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x45);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XB1+XB2", 20);
	StrCopy(pSlaveNode->posy_expr, "YB*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x51);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XB1+XB2", 20);
	StrCopy(pSlaveNode->posz_expr, "ZB", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x41);
	pSlaveNode->cFlag = 0x02;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posy_expr, "YB1", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x47);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XB1+XB2-XB3", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x53);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XB1+XB2-XB3", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x43);
	pSlaveNode->cFlag = 0x02;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posy_expr, "YB1", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x2f);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XB1", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x55);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XB1", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x4d);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posz_expr, "ZB", 20);
	//第二层横担
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x57);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "Z1+Z2-ZCD", 20);
	//左下横担
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x3f);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XC1-XC2", 20);
	StrCopy(pSlaveNode->posy_expr, "YC*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z1+Z2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x67);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XC1-XC2", 20);
	StrCopy(pSlaveNode->posy_expr, "YC*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z1+Z2-ZC", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x3d);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XC1-XC3*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x65);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XC1-XC3*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x5d);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XC1+XC3*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x61);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posz_expr, "Z1+Z2-ZC", 20);
	//右下横担
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x3b);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XD1+XD2*0.5", 20);
	StrCopy(pSlaveNode->posy_expr, "YD*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z1+Z2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x63);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XD1+XD2*0.5", 20);
	StrCopy(pSlaveNode->posy_expr, "YD*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z1+Z2-ZD", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x5b);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XD1-XD2*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x5f);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posz_expr, "Z1+Z2-ZD", 20);
	//
	return true;
}
//
bool CTowerTemplateToModel::InitJG2TowerTmplInfo()
{
	if (m_pTower == NULL)
		return false;
	//模型参数
	CDesParasOper oper(&m_pTower->hashParams);
	oper.AddParamValue(KEY4C("Z0"), 0, 0);		//后台隐藏参数，不显示
	oper.AddParamValue(KEY4C("Z1"), 7000, 1, "上中层高");
	oper.AddParamValue(KEY4C("Z2"), 7000, 1, "中下层高");
	oper.AddParamValue(KEY4C("Z3"), 4000, 1, "下层至变坡高");
	oper.AddParamValue(KEY4C("Z4"), 36000, 1, "呼高");
	oper.AddParamValue(KEY4C("K1"), 3860, 1, "变坡口宽");
	oper.AddParamValue(KEY4C("K2"), 1700, 1, "顶面宽");
	oper.AddParamValue(KEY4C("K3"), 14740, 1, "基础正面根开");
	oper.AddParamValue(KEY4C("P1"), 0.06, 1, "变坡上侧坡度");
	oper.AddParamValue(KEY4C("P2"), 0.17, 1, "变坡下侧坡度");
	//左上横担HX1
	oper.AddParamValue(KEY4C("XA"), 5500, 2, "地线挂点中心距");
	oper.AddParamValue(KEY4C("YA"), 1700, 2, "地线支架纵向宽");
	oper.AddParamValue(KEY4C("ZAB"), 2000, 2, "地线支架根部高");
	//右上横担HX12
	oper.AddParamValue(KEY4C("XB1"), 5500, 3, "地线挂点中心距");
	oper.AddParamValue(KEY4C("XB2"), 2500, 3, "跳线架控制尺寸");
	oper.AddParamValue(KEY4C("XB3"), 1000, 3, "跳线架控制尺寸");
	oper.AddParamValue(KEY4C("YB"), 1700, 3, "跳线架控制尺寸");
	oper.AddParamValue(KEY4C("YB1"), 4000, 3, "跳线架纵向宽度");
	oper.AddParamValue(KEY4C("YB2"), 4000, 3, "跳线架纵向宽度");
	oper.AddParamValue(KEY4C("ZB"), 1000, 3, "跳线架高度");
	//中层横担HX2
	oper.AddParamValue(KEY4C("XC"), 3000, 4, "中导挂点中心距");
	oper.AddParamValue(KEY4C("YC"), 2400, 4, "中导横担纵向宽");
	oper.AddParamValue(KEY4C("ZC"), 1800, 4, "中导横担根部高");
	//下层横担
	oper.AddParamValue(KEY4C("ZDE"), 2500, 5, "下导横担根部高");
	//左下横担HX11
	oper.AddParamValue(KEY4C("XD1"), 7500, 5, "左导挂点中心距");
	oper.AddParamValue(KEY4C("XD2"), 2000, 5, "左导跳线架距离");
	oper.AddParamValue(KEY4C("XD3"), 560, 5, "左导挂点间距");
	oper.AddParamValue(KEY4C("YD"), 3380, 5, "左导横担纵向宽");
	oper.AddParamValue(KEY4C("ZD"), 800, 5, "跳线架高度");
	//右下横担HX9
	oper.AddParamValue(KEY4C("XE1"), 7500, 6, "右导挂点中心距");
	oper.AddParamValue(KEY4C("XE2"), 560, 6, "右导挂点间距");
	oper.AddParamValue(KEY4C("YE"), 3380, 6, "右导横担纵向宽");
	oper.AddParamValue(KEY4C("ZE"), 800, 6, "跳线架高度");
	//节点依赖关系
	//塔身主控节点
	SLAVE_NODE* pSlaveNode = NULL;
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x29);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "K2*0.5", 20);
	StrCopy(pSlaveNode->posy_expr, "K2*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x71);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "Z1", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x37);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "Z1+Z2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x33);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "K1*0.5", 20);
	StrCopy(pSlaveNode->posy_expr, "K1*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z1+Z2+Z3", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x7D);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "K3*0.5", 20);
	StrCopy(pSlaveNode->posy_expr, "K3*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z1+Z2+Z4", 20);
	//第一层横担
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x49);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "ZAB", 20);
	//左上横担
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x31);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XA", 20);
	StrCopy(pSlaveNode->posy_expr, "YA*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0", 20);
	//右上横担
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x45);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XB1+XB2", 20);
	StrCopy(pSlaveNode->posy_expr, "YB*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x51);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XB1+XB2", 20);
	StrCopy(pSlaveNode->posz_expr, "ZB", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x41);
	pSlaveNode->cFlag = 0x02;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posy_expr, "YB1", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x47);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XB1+XB2-XB3", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x53);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XB1+XB2-XB3", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x43);
	pSlaveNode->cFlag = 0x02;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posy_expr, "YB1", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x2f);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XB1", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x55);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XB1", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x4d);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posz_expr, "ZB", 20);
	//中层横担
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x6D);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "Z1-ZC", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x2382);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "XC", 20);
	StrCopy(pSlaveNode->posy_expr, "YC*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z1", 20);
	//下层横担
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x57);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "Z1+Z2-ZDE", 20);
	//左下横担
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x3f);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XD1-XD2", 20);
	StrCopy(pSlaveNode->posy_expr, "YD*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z1+Z2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x67);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XD1-XD2", 20);
	StrCopy(pSlaveNode->posy_expr, "YD*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z1+Z2-ZD", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x3d);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XD1-XD3*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x65);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XD1-XD3*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x5d);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XD1+XD3*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x61);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posz_expr, "Z1+Z2-ZD", 20);
	//右下横担
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x3b);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XE1+XE2*0.5", 20);
	StrCopy(pSlaveNode->posy_expr, "YE*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z1+Z2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x63);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XE1+XE2*0.5", 20);
	StrCopy(pSlaveNode->posy_expr, "YE*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z1+Z2-ZE", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x5b);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XE1-XE2*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x5f);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posz_expr, "Z1+Z2-ZE", 20);
	return true;
}
//同塔双回路耐张塔-SJ塔型
bool CTowerTemplateToModel::InitSJTowerTmplInfo()
{
	if (m_pTower == NULL)
		return false;
	//模型参数
	CDesParasOper oper(&m_pTower->hashParams);
	oper.AddParamValue(KEY4C("Z0"), 0, 0);		//后台隐藏参数，不显示
	oper.AddParamValue(KEY4C("Z1"), 7800, 1, "地线支架高");
	oper.AddParamValue(KEY4C("Z2"), 15600, 1, "上中层高");
	oper.AddParamValue(KEY4C("Z3"), 14600, 1, "中下层高");
	oper.AddParamValue(KEY4C("Z4"), 3200, 1, "下层至变坡高");
	oper.AddParamValue(KEY4C("Z5"), 39000, 1, "呼高");
	oper.AddParamValue(KEY4C("K1"), 7000, 1, "变坡口宽");
	oper.AddParamValue(KEY4C("K2"), 2200, 1, "顶面宽");
	oper.AddParamValue(KEY4C("K3"), 18456, 1, "基础正面根开");
	oper.AddParamValue(KEY4C("P1"), 0.058, 1, "变坡上侧坡度");
	oper.AddParamValue(KEY4C("P2"), 0.32, 1, "变坡下侧坡度");
	//第一层
	oper.AddParamValue(KEY4C("ZAB"), 3300, 2, "地线支架根部高");
	//左一横担HX1
	oper.AddParamValue(KEY4C("XA"), 13570, 2, "地线挂点中心距");
	oper.AddParamValue(KEY4C("YA"), 1200, 2, "地线支架纵向宽");
	//右一横担HX1
	oper.AddParamValue(KEY4C("XB"), 12170, 3, "地线支架中心距");
	oper.AddParamValue(KEY4C("YB"), 1200, 3, "地线支架纵向宽");
	//第二层
	oper.AddParamValue(KEY4C("ZCD"), 2800, 4, "上导横担根部高");
	//左二横担HX8
	oper.AddParamValue(KEY4C("XC1"), 10400, 4, "上导挂点中心距");
	oper.AddParamValue(KEY4C("XC2"), 2000, 4, "上导跳线架距离");
	oper.AddParamValue(KEY4C("XC3"), 560, 4, "上导挂点间距");
	oper.AddParamValue(KEY4C("XC4"), 2060, 4, "跳线架上平面宽");
	oper.AddParamValue(KEY4C("YC"), 1800, 4, "上导横担纵向宽");
	oper.AddParamValue(KEY4C("ZC2"), 1000, 4, "跳线架高度");
	//右二横担HX8
	oper.AddParamValue(KEY4C("XD1"), 9000, 5, "上导挂点中心距");
	oper.AddParamValue(KEY4C("XD2"), 1000, 5, "上导跳线架距离");
	oper.AddParamValue(KEY4C("XD3"), 560, 5, "上导挂点间距");
	oper.AddParamValue(KEY4C("XD4"), 2060, 5, "跳线架上平面宽");
	oper.AddParamValue(KEY4C("YD"), 1800, 5, "上导横担纵向宽");
	oper.AddParamValue(KEY4C("ZD2"), 1000, 5, "跳线架高度");
	//第三层
	oper.AddParamValue(KEY4C("ZEF"), 3600, 6, "中导横担根部高");
	//左三横担HX8
	oper.AddParamValue(KEY4C("XE1"), 13570, 6, "中导挂点中心距");
	oper.AddParamValue(KEY4C("XE2"), 2000, 6, "中导跳线架距离");
	oper.AddParamValue(KEY4C("XE3"), 560, 6, "中导挂点间距");
	oper.AddParamValue(KEY4C("XE4"), 2060, 6, "跳线架上平面宽");
	oper.AddParamValue(KEY4C("YE"), 2400, 6, "中导横担纵向宽");
	oper.AddParamValue(KEY4C("ZE2"), 1000, 6, "跳线架高度");
	//右三横担HX8
	oper.AddParamValue(KEY4C("XF1"), 12170, 7, "中导挂点中心距");
	oper.AddParamValue(KEY4C("XF2"), 1000, 7, "中导跳线架距离");
	oper.AddParamValue(KEY4C("XF3"), 260, 7, "中导挂点间距");
	oper.AddParamValue(KEY4C("XF4"), 2060, 7, "跳线架上平面宽");
	oper.AddParamValue(KEY4C("YF"), 2400, 7, "中导横担纵向宽");
	oper.AddParamValue(KEY4C("ZF2"), 1000, 7, "跳线架高度");
	//第四层
	oper.AddParamValue(KEY4C("ZGH"), 3000, 8, "下导横担根部高");
	//左四横担HX8
	oper.AddParamValue(KEY4C("XG1"), 11700, 8, "下导挂点中心距");
	oper.AddParamValue(KEY4C("XG2"), 2000, 8, "下导跳线架距离");
	oper.AddParamValue(KEY4C("XG3"), 560, 8, "下导挂点间距");
	oper.AddParamValue(KEY4C("XG4"), 2060, 8, "跳线架上平面宽");
	oper.AddParamValue(KEY4C("YG"), 3000, 8, "下导横担纵向宽");
	oper.AddParamValue(KEY4C("ZG2"), 1000, 8, "跳线架高度");
	//右四横担HX8
	oper.AddParamValue(KEY4C("XH1"), 10300, 9, "下导挂点中心距");
	oper.AddParamValue(KEY4C("XH2"), 1000, 9, "下导跳线架距离");
	oper.AddParamValue(KEY4C("XH3"), 560, 9, "下导挂点间距");
	oper.AddParamValue(KEY4C("XH4"), 2060, 9, "跳线架上平面宽");
	oper.AddParamValue(KEY4C("YH"), 3000, 9, "下导横担纵向宽");
	oper.AddParamValue(KEY4C("ZH2"), 1000, 9, "跳线架高度");
	//节点依赖关系
	//塔身主控节点
	SLAVE_NODE* pSlaveNode = NULL;
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x2E);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "K2*0.5", 20);
	StrCopy(pSlaveNode->posy_expr, "K2*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x3E);
	pSlaveNode->cFlag = 0x03;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "Z1", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x52);
	pSlaveNode->cFlag = 0x03;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "Z1+Z2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x62);
	pSlaveNode->cFlag = 0x03;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "Z1+Z2+Z3", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x32);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "K1*0.5", 20);
	StrCopy(pSlaveNode->posy_expr, "K1*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z1+Z2+Z3+Z4", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xDA);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "K3*0.5", 20);
	StrCopy(pSlaveNode->posy_expr, "K3*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z1+Z2+Z3+Z5", 20);
	//第一层横担
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x36);
	pSlaveNode->cFlag = 0x03;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "ZAB", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x2C);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XA", 20);
	StrCopy(pSlaveNode->posy_expr, "YA*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x2A);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XB", 20);
	StrCopy(pSlaveNode->posy_expr, "YB*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0", 20);
	//第二层横担
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x3A);
	pSlaveNode->cFlag = 0x03;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "Z1-ZCD", 20);
	//左横担
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x7C);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XC1-XC3*0.5+XC4", 20);
	StrCopy(pSlaveNode->posy_expr, "YC*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z1", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x70);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XC1-XC2", 20);
	StrCopy(pSlaveNode->posy_expr, "YC*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z1", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x78);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XC1+XC3*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x74);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XC1-XC3*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x68);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XC1-XC3*0.5+XC4", 20);
	StrCopy(pSlaveNode->posz_expr, "Z1-ZC2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x6C);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XC1-XC3*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z1-ZC2", 20);
	//右横担
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x7A);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XD1+XD3*0.5-XD4", 20);
	StrCopy(pSlaveNode->posy_expr, "YD*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z1", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x6E);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XD1+XD2", 20);
	StrCopy(pSlaveNode->posy_expr, "YD*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z1", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x76);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XD1-XD3*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x72);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XD1+XD3*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x66);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XD1+XD3*0.5-XD4", 20);
	StrCopy(pSlaveNode->posz_expr, "Z1-ZD2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x6A);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XD1+XD3*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z1-ZD2", 20);
	//第三层横担
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x4E);
	pSlaveNode->cFlag = 0x03;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "Z1+Z2-ZEF", 20);
	//左横担
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x94);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XE1-XE3*0.5+XE4", 20);
	StrCopy(pSlaveNode->posy_expr, "YE*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z1+Z2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x88);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XE1-XE2", 20);
	StrCopy(pSlaveNode->posy_expr, "YE*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z1+Z2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x90);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XE1+XE3*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x8C);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XE1-XE3*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x80);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XE1-XE3*0.5+XE4", 20);
	StrCopy(pSlaveNode->posz_expr, "Z1+Z2-ZE2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x84);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XE1-XE3*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z1+Z2-ZE2", 20);
	//右横担
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x92);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XF1+XF3*0.5-XF4", 20);
	StrCopy(pSlaveNode->posy_expr, "YF*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z1+Z2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x86);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XF1+XF2", 20);
	StrCopy(pSlaveNode->posy_expr, "YF*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z1+Z2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x8E);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XF1-XF3*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x8A);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XF1+XF3*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x7E);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XF1+XF3*0.5-XF4", 20);
	StrCopy(pSlaveNode->posz_expr, "Z1+Z2-ZF2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x82);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XF1+XF3*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z1+Z2-ZF2", 20);
	//第四层横担
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x5E);
	pSlaveNode->cFlag = 0x03;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "Z1+Z2+Z3-ZGH", 20);
	//左横担
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xAC);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XG1-XG3*0.5+XG4", 20);
	StrCopy(pSlaveNode->posy_expr, "YG*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z1+Z2+Z3", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xA0);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XG1-XG2", 20);
	StrCopy(pSlaveNode->posy_expr, "YG*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z1+Z2+Z3", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xA8);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XG1+XG3*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xA4);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XG1-XG3*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x98);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XG1-XG3*0.5+XG4", 20);
	StrCopy(pSlaveNode->posz_expr, "Z1+Z2+Z3-ZG2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x9C);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XG1-XG3*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z1+Z2+Z3-ZG2", 20);
	//右横担
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xAA);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XH1+XH3*0.5-XH4", 20);
	StrCopy(pSlaveNode->posy_expr, "YH*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z1+Z2+Z3", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x9E);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XH1+XH2", 20);
	StrCopy(pSlaveNode->posy_expr, "YH*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z1+Z2+Z3", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xA6);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XH1-XH3*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xA2);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XH1+XH3*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x96);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XH1+XH3*0.5-XH4", 20);
	StrCopy(pSlaveNode->posz_expr, "Z1+Z2+Z3-ZH2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x9A);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XH1+XH3*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z1+Z2+Z3-ZH2", 20);
	return true;
}
//同塔双回路耐张塔-SJB塔型
bool CTowerTemplateToModel::InitSJBTowerTmplInfo()
{
	if (m_pTower == NULL)
		return false;
	//模型参数
	CDesParasOper oper(&m_pTower->hashParams);
	oper.AddParamValue(KEY4C("Z0"), 6300, 0);		//后台隐藏参数，不显示
	oper.AddParamValue(KEY4C("Z1"), 3200, 1, "上导横担根部高");
	oper.AddParamValue(KEY4C("Z2"), 16100, 1, "上中层高");
	oper.AddParamValue(KEY4C("Z3"), 15100, 1, "中下层高");
	oper.AddParamValue(KEY4C("Z4"), 3200, 1, "下层至变坡高");
	oper.AddParamValue(KEY4C("Z5"), 42000, 1, "呼高");
	oper.AddParamValue(KEY4C("K1"), 7200, 1, "变坡口宽");
	oper.AddParamValue(KEY4C("K2"), 3000, 1, "顶面宽");
	oper.AddParamValue(KEY4C("K3"), 19600, 1, "基础正面根开");
	oper.AddParamValue(KEY4C("P1"), 0.05, 1, "变坡上侧坡度");
	oper.AddParamValue(KEY4C("P2"), 0.16, 1, "变坡下侧坡度");
	//第一层横担
	//左侧复合横担TX4
	oper.AddParamValue(KEY4C("XA1"), 15900, 2, "地线挂点中心距");
	oper.AddParamValue(KEY4C("XA2"), 11900, 2, "上导挂点中心距");
	oper.AddParamValue(KEY4C("XA3"), 2000, 2, "上导跳线架距离");
	oper.AddParamValue(KEY4C("XA4"), 640, 2, "挂点间距");
	oper.AddParamValue(KEY4C("XA5"), 4800, 2, "地线支架折段长度");
	oper.AddParamValue(KEY4C("XA6"), 700, 2, "地线支架折段宽");
	oper.AddParamValue(KEY4C("YA1"), 3200, 2, "导线横担纵向宽");
	oper.AddParamValue(KEY4C("YA2"), 1000, 2, "地线支架纵向宽");
	oper.AddParamValue(KEY4C("ZA1"), 9500, 2, "地线支架高");
	oper.AddParamValue(KEY4C("ZA2"), 1000, 2, "跳线架高度");
	oper.AddParamValue(KEY4C("ZA3"), 1500, 2, "地线支架折段高");
	//右侧复合横担TX3
	oper.AddParamValue(KEY4C("XB1"), 13600, 3, "地线挂点中心距");
	oper.AddParamValue(KEY4C("XB2"), 9500, 3, "上导挂点中心距");
	oper.AddParamValue(KEY4C("XB3"), 640, 3, "挂点间距");
	oper.AddParamValue(KEY4C("XB4"), 4000, 3, "地线支架折段长度");
	oper.AddParamValue(KEY4C("XB5"), 400, 3, "地线支架折段宽");
	oper.AddParamValue(KEY4C("YB1"), 3200, 3, "导线横担纵向宽");
	oper.AddParamValue(KEY4C("YB2"), 1000, 3, "地线支架纵向宽");
	oper.AddParamValue(KEY4C("ZB1"), 9500, 3, "地线支架高");
	oper.AddParamValue(KEY4C("ZB2"), 1000, 3, "跳线架高度");
	oper.AddParamValue(KEY4C("ZB3"), 1500, 3, "地线支架折段高");
	//第二层
	oper.AddParamValue(KEY4C("ZCD"), 4000, 4, "中导横担根部高");
	//左侧横担HX14
	oper.AddParamValue(KEY4C("XC1"), 15600, 4, "中导挂点中心距");
	oper.AddParamValue(KEY4C("XC2"), 2000, 4, "中导跳线架距离");
	oper.AddParamValue(KEY4C("XC3"), 640, 4, "中导挂点间距");
	oper.AddParamValue(KEY4C("XC4"), 3820, 4, "跳线架上平面宽");
	oper.AddParamValue(KEY4C("YC"), 2800, 4, "中导横担纵向宽");
	oper.AddParamValue(KEY4C("ZC2"), 1000, 4, "跳线架高度");
	//右侧横担HX13
	oper.AddParamValue(KEY4C("XD1"), 13300, 5, "中导挂点中心距");
	oper.AddParamValue(KEY4C("XD2"), 640, 5, "中导挂点间距");
	oper.AddParamValue(KEY4C("XD3"), 2140, 5, "跳线架上平面宽");
	oper.AddParamValue(KEY4C("YD"), 2800, 5, "中导横担纵向宽");
	oper.AddParamValue(KEY4C("ZD2"), 1000, 5, "跳线架高度");
	//第三层
	oper.AddParamValue(KEY4C("ZEF"), 4000, 6, "下导横担根部高");
	//左侧横担HX14
	oper.AddParamValue(KEY4C("XE1"), 13600, 6, "下导挂点中心距");
	oper.AddParamValue(KEY4C("XE2"), 2000, 6, "下导跳线架距离");
	oper.AddParamValue(KEY4C("XE3"), 640, 6, "下导挂点间距");
	oper.AddParamValue(KEY4C("XE4"), 3820, 6, "跳线架上平面宽");
	oper.AddParamValue(KEY4C("YE"), 4200, 6, "下导横担纵向宽");
	oper.AddParamValue(KEY4C("ZE2"), 1000, 6, "跳线架高度");
	//右侧横担HX13
	oper.AddParamValue(KEY4C("XF1"), 11300, 7, "下导挂点中心距");
	oper.AddParamValue(KEY4C("XF2"), 640, 7, "下导挂点间距");
	oper.AddParamValue(KEY4C("XF3"), 2140, 7, "跳线架上平面宽");
	oper.AddParamValue(KEY4C("YF"), 3000, 7, "下导横担纵向宽");
	oper.AddParamValue(KEY4C("ZF2"), 1000, 7, "跳线架高度");

	//节点依赖关系
	//塔身主控节点
	SLAVE_NODE* pSlaveNode = NULL;
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x2A);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "K2*0.5", 20);
	StrCopy(pSlaveNode->posy_expr, "K2*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x3A);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x52);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x66);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2+Z3", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x2E);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "K1*0.5", 20);
	StrCopy(pSlaveNode->posy_expr, "K1*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2+Z3+Z4", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xFA);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "K3*0.5", 20);
	StrCopy(pSlaveNode->posy_expr, "K3*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2+Z3+Z5", 20);
	//第一层横担
	//左侧
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x96);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XA1", 20);
	StrCopy(pSlaveNode->posy_expr, "YA2*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1-ZA1", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x98);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XA1+XA5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1-ZA1", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x94);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XA1+XA5-XA6", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1-ZA1+ZA3", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xA2);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XA2-XA3", 20);
	StrCopy(pSlaveNode->posy_expr, "YA1*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xA0);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XA2-XA4*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x9E);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XA2+XA4*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xCC);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XA2-XA3", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1-ZA2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xCA);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XA2-XA4*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xC8);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1-ZA2", 20);
	//右侧
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x90);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XB1", 20);
	StrCopy(pSlaveNode->posy_expr, "YB2*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1-ZB1", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x92);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XB1-XB4", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1-ZB1", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x8E);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XB1-XB4+XB5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1-ZB1+ZB3", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x9C);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XB2+XB3*0.5", 20);
	StrCopy(pSlaveNode->posy_expr, "YB1*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x9A);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XB2-XB3*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xC6);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XB2+XB3*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1-ZB2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xC4);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1-ZB2", 20);
	//第二层横担
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x4A);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2-ZCD", 20);
	//左侧
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xA6);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XC1-XC2", 20);
	StrCopy(pSlaveNode->posy_expr, "YC*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xAE);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XC1-XC3*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xB0);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XC1+XC3*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xB2);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XC1-XC2+XC4", 20);
	StrCopy(pSlaveNode->posy_expr, "YC*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xDA);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XC1-XC2", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2-ZC2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xD8);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XC1-XC3*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xD6);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XC1-XC2+XC4", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2-ZC2", 20);
	//右侧
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xA8);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XD1+XD2*0.5", 20);
	StrCopy(pSlaveNode->posy_expr, "YD*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xAA);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XD1-XD2*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xAC);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XD1+XD2*0.5-XD3", 20);
	StrCopy(pSlaveNode->posy_expr, "YD*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xD4);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XD1+XD2*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2-ZD2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xD2);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XD1+XD2*0.5-XD3", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2-ZD2", 20);
	//第三层横担
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x5E);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2+Z3-ZEF", 20);
	//左侧
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xB6);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XE1-XE2", 20);
	StrCopy(pSlaveNode->posy_expr, "YE*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2+Z3", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xBE);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XE1-XE3*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xC0);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XE1+XE3*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xC2);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XE1-XE2+XE4", 20);
	StrCopy(pSlaveNode->posy_expr, "YE*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2+Z3", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xE8);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XE1-XE2", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2+Z3-ZE2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xE6);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XE1-XE3*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xE4);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XE1-XE2+XE4", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2+Z3-ZE2", 20);
	//右侧
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xB8);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XF1+XF2*0.5", 20);
	StrCopy(pSlaveNode->posy_expr, "YF*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2+Z3", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xBA);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XF1-XF2*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xBC);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XF1+XF2*0.5-XF3", 20);
	StrCopy(pSlaveNode->posy_expr, "YF*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2+Z3", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xE0);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XF1+XF2*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2+Z3-ZF2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xDE);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XF1+XF2*0.5-XF3", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2+Z3-ZF2", 20);
	return true;
}
//同塔双回路耐张塔-SJC塔型
bool CTowerTemplateToModel::InitSJCTowerTmplInfo()
{
	if (m_pTower == NULL)
		return false;
	//模型参数
	CDesParasOper oper(&m_pTower->hashParams);
	oper.AddParamValue(KEY4C("Z0"), 4300, 0);		//后台隐藏参数，不显示
	oper.AddParamValue(KEY4C("Z1"), 2700, 1, "上导横担根部高");
	oper.AddParamValue(KEY4C("Z2"), 12500, 1, "上中层高");
	oper.AddParamValue(KEY4C("Z3"), 11500, 1, "中下层高");
	oper.AddParamValue(KEY4C("Z4"), 3000, 1, "下层至变坡高");
	oper.AddParamValue(KEY4C("Z5"), 36000, 1, "呼高");
	oper.AddParamValue(KEY4C("K1"), 6200, 1, "变坡口宽");
	oper.AddParamValue(KEY4C("K2"), 2636, 1, "顶面宽");
	oper.AddParamValue(KEY4C("K3"), 14780, 1, "基础正面根开");
	oper.AddParamValue(KEY4C("P1"), 0.06, 1, "变坡上侧坡度");
	oper.AddParamValue(KEY4C("P2"), 0.13, 1, "变坡下侧坡度");
	//第一层横担
	//左侧复合横担TX6
	oper.AddParamValue(KEY4C("XA1"), 13250, 2, "地线挂点中心距");
	oper.AddParamValue(KEY4C("XA2"), 7500, 2, "上导挂点中心距");
	oper.AddParamValue(KEY4C("XA3"), 2000, 2, "上导跳线架距离");
	oper.AddParamValue(KEY4C("XA4"), 560, 2, "挂点间距");
	oper.AddParamValue(KEY4C("XA5"), 3280, 2, "跳线架上平面宽");
	oper.AddParamValue(KEY4C("XA6"), 400, 2, "地线挂点宽度");
	oper.AddParamValue(KEY4C("YA1"), 2800, 2, "导线横担纵向宽");
	oper.AddParamValue(KEY4C("YA2"), 600, 2, "地线支架纵向宽");
	oper.AddParamValue(KEY4C("ZA1"), 7000, 2, "地线支架高");
	oper.AddParamValue(KEY4C("ZA2"), 1000, 2, "跳线架高度");
	//右侧复合横担TX5
	oper.AddParamValue(KEY4C("XB1"), 11950, 3, "地线挂点中心距");
	oper.AddParamValue(KEY4C("XB2"), 7500, 3, "上导挂点中心距");
	oper.AddParamValue(KEY4C("XB3"), 560, 3, "挂点间距");
	oper.AddParamValue(KEY4C("XB4"), 1560, 3, "跳线架上平面宽");
	oper.AddParamValue(KEY4C("XB5"), 400, 3, "地线挂点宽度");
	oper.AddParamValue(KEY4C("YB1"), 2800, 3, "导线横担纵向宽");
	oper.AddParamValue(KEY4C("YB2"), 600, 3, "地线支架纵向宽");
	oper.AddParamValue(KEY4C("ZB1"), 7000, 3, "地线支架高");
	oper.AddParamValue(KEY4C("ZB2"), 1000, 3, "跳线架高度");
	//第二层横担
	oper.AddParamValue(KEY4C("ZCD"), 3600, 4, "中导横担根部高");
	//左侧横担HX14
	oper.AddParamValue(KEY4C("XC1"), 10750, 4, "中导挂点中心距");
	oper.AddParamValue(KEY4C("XC2"), 2000, 4, "中导跳线架距离");
	oper.AddParamValue(KEY4C("XC3"), 560, 4, "中导挂点间距");
	oper.AddParamValue(KEY4C("XC4"), 3780, 4, "跳线架上平面宽");
	oper.AddParamValue(KEY4C("YC"), 3000, 4, "中导横担纵向宽");
	oper.AddParamValue(KEY4C("ZC2"), 1000, 4, "跳线架高度");
	//右侧横担HX13
	oper.AddParamValue(KEY4C("XD1"), 10750, 5, "中导挂点中心距");
	oper.AddParamValue(KEY4C("XD2"), 560, 5, "中导挂点间距");
	oper.AddParamValue(KEY4C("XD3"), 2060, 5, "跳线架上平面宽");
	oper.AddParamValue(KEY4C("YD"), 3000, 5, "中导横担纵向宽");
	oper.AddParamValue(KEY4C("ZD2"), 1000, 5, "跳线架高度");
	//第三层横担
	oper.AddParamValue(KEY4C("ZEF"), 3000, 6, "下导横担根部高");
	//左侧横担HX14
	oper.AddParamValue(KEY4C("XE1"), 9000, 6, "下导挂点中心距");
	oper.AddParamValue(KEY4C("XE2"), 2000, 6, "下导跳线架距离");
	oper.AddParamValue(KEY4C("XE3"), 560, 6, "下导挂点间距");
	oper.AddParamValue(KEY4C("XE4"), 3180, 6, "跳线架上平面宽");
	oper.AddParamValue(KEY4C("YE"), 3000, 6, "下导横担纵向宽");
	oper.AddParamValue(KEY4C("ZE2"), 1000, 6, "跳线架高度");
	//右侧横担HX13
	oper.AddParamValue(KEY4C("XF1"), 9000, 7, "下导挂点中心距");
	oper.AddParamValue(KEY4C("XF2"), 560, 7, "下导挂点间距");
	oper.AddParamValue(KEY4C("XF3"), 2060, 7, "跳线架上平面宽");
	oper.AddParamValue(KEY4C("YF"), 3000, 7, "下导横担纵向宽");
	oper.AddParamValue(KEY4C("ZF2"), 1000, 7, "跳线架高度");
	
	//节点依赖关系
	//塔身主控节点
	SLAVE_NODE* pSlaveNode = NULL;
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x29);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "K2*0.5", 20);
	StrCopy(pSlaveNode->posy_expr, "K2*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x35);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x45);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x51);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2+Z3", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x2D);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "K1*0.5", 20);
	StrCopy(pSlaveNode->posy_expr, "K1*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2+Z3+Z4", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x31);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "K3*0.5", 20);
	StrCopy(pSlaveNode->posy_expr, "K3*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2+Z3+Z5", 20);
	//第一层横担
	//左侧
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xC5);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XA1", 20);
	StrCopy(pSlaveNode->posy_expr, "YA2*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1-ZA1", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xC7);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XA1+XA6", 20);
	StrCopy(pSlaveNode->posy_expr, "YA2*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1-ZA1", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xCF);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XA2-XA3", 20);
	StrCopy(pSlaveNode->posy_expr, "YA1*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xD1);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XA2-XA4*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xD3);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XA2+XA4*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xFB);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XA2-XA3", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1-ZA2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xF9);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XA2-XA4*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xF7);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XA2-XA3+XA5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1-ZA2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x1FBC);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XA2-XA3+XA5", 20);
	//右侧
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xC1);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XB1", 20);
	StrCopy(pSlaveNode->posy_expr, "YB2*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1-ZB1", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xC3);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XB1-XB5", 20);
	StrCopy(pSlaveNode->posy_expr, "YB2*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1-ZB1", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xC9);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XB2+XB3*0.5", 20);
	StrCopy(pSlaveNode->posy_expr, "YB1*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xCB);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XB2-XB3*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xF3);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XB2+XB3*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1-ZB2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xF5);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XB2+XB3*0.5-XB4", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1-ZB2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x1FBE);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XB2+XB3*0.5-XB4", 20);
	//第二层横担
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x41);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2-ZCD", 20);
	//左侧
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xDD);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XC1-XC2", 20);
	StrCopy(pSlaveNode->posy_expr, "YC*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xDF);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XC1-XC3*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xE1);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XC1+XC3*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xE3);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XC1-XC2+XC4", 20);
	StrCopy(pSlaveNode->posy_expr, "YC*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x105);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XC1-XC2", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2-ZC2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x103);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XC1-XC3*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xFF);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XC1-XC2+XC4", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2-ZC2", 20);
	//右侧
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xD7);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XD1+XD2*0.5", 20);
	StrCopy(pSlaveNode->posy_expr, "YD*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xD9);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XD1-XD2*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xDB);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XD1+XD2*0.5-XD3", 20);
	StrCopy(pSlaveNode->posy_expr, "YD*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x101);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XD1+XD2*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2-ZD2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xFD);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XD1+XD2*0.5-XD3", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2-ZD2", 20);
	//第三层横担
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x4D);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2+Z3-ZEF", 20);
	//左侧
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xEB);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XE1-XE2", 20);
	StrCopy(pSlaveNode->posy_expr, "YE*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2+Z3", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xED);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XE1-XE3*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xEF);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XE1+XE3*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xF1);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XE1-XE2+XE4", 20);
	StrCopy(pSlaveNode->posy_expr, "YE*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2+Z3", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x10F);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XE1-XE2", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2+Z3-ZE2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x10D);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XE1-XE3*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x109);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XE1-XE2+XE4", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2+Z3-ZE2", 20);
	//右侧
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xE5);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XF1+XF2*0.5", 20);
	StrCopy(pSlaveNode->posy_expr, "YF*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2+Z3", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xE7);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XF1-XF2*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xE9);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XF1+XF2*0.5-XF3", 20);
	StrCopy(pSlaveNode->posy_expr, "YF*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2+Z3", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x10B);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XF1+XF2*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2+Z3-ZF2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x107);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XF1+XF2*0.5-XF3", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2+Z3-ZF2", 20);
	return true;
}
//酒杯型直线塔-ZBC塔型
bool CTowerTemplateToModel::InitZBCTowerTmplInfo()
{
	if (m_pTower == NULL)
		return false;
	//模型参数
	CDesParasOper oper(&m_pTower->hashParams);
	oper.AddParamValue(KEY4C("Z0"), 20400, 0);	//后台隐藏参数，不显示
	oper.AddParamValue(KEY4C("Z"), 42000, 1, "呼高");
	oper.AddParamValue(KEY4C("K1"), 3400, 1, "变坡口宽");
	oper.AddParamValue(KEY4C("K2"), 9384, 1, "塔腿正面根开");
	oper.AddParamValue(KEY4C("P"), 0.11, 1, "坡度");
	//酒杯塔头
	oper.AddParamValue(KEY4C("XA1"), 8970, 2, "地线挂点中心距");
	oper.AddParamValue(KEY4C("XA2"), 500, 2, "地线挂点悬臂长度");
	oper.AddParamValue(KEY4C("XA3"), 4850, 2, "边导线悬臂长度");
	oper.AddParamValue(KEY4C("XA4"), 1800, 2, "上曲臂根部宽度");
	oper.AddParamValue(KEY4C("XA5"), 4070, 2, "中横担跨度");
	oper.AddParamValue(KEY4C("XA6"), 5000, 2, "曲臂交点中心距");
	oper.AddParamValue(KEY4C("YA1"), 500, 2, "地线挂点纵向宽");
	oper.AddParamValue(KEY4C("YA2"), 1200, 2, "横担根部纵向宽");
	oper.AddParamValue(KEY4C("YA3"), 1200, 2, "边导挂点纵向宽");
	oper.AddParamValue(KEY4C("ZA1"), 9100, 2, "下曲臂高度");
	oper.AddParamValue(KEY4C("ZA2"), 5700, 2, "上曲臂高度");
	oper.AddParamValue(KEY4C("ZA3"), 5600, 2, "地线支架高度");
	oper.AddParamValue(KEY4C("ZA4"), 1000, 2, "中横担隔面高度");
	oper.AddParamValue(KEY4C("ZA5"), 1600, 2, "边横担根部高度");
	oper.AddParamValue(KEY4C("ZA6"), 600, 2, "地线支架根部高度");

	//节点依赖关系
	//塔身依赖节点
	SLAVE_NODE* pSlaveNode = NULL;
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x60);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "K1*0.5", 20);
	StrCopy(pSlaveNode->posy_expr, "K1*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x66);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "K2*0.5", 20);
	StrCopy(pSlaveNode->posy_expr, "K2*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0-ZA1-ZA2+Z", 20);
	//酒杯塔头
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x2C);	//N10节点
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "XA1", 20);
	StrCopy(pSlaveNode->posy_expr, "YA1*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0-ZA1-ZA2-ZA3", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x38);	//N20节点
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "XA1-XA2", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0-ZA1-ZA2-ZA3-ZA6", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x34);	//N30
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "XA1-XA2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x30);	//N40
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "Z0-ZA1-ZA2-ZA3", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x3c);	//N50
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "XA4+XA5", 20);
	StrCopy(pSlaveNode->posy_expr, "YA2*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0-ZA1-ZA2-ZA5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x40);	//N60
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "XA5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0-ZA1-ZA2-ZA5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x4c);	//N70
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "XA3+XA4+XA5", 20);
	StrCopy(pSlaveNode->posy_expr, "YA3*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0-ZA1-ZA2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x44);	//N80
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "XA4+XA5", 20);
	StrCopy(pSlaveNode->posy_expr, "YA2*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0-ZA1-ZA2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x48);	//N90
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "XA5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0-ZA1-ZA2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x50);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x04;
	StrCopy(pSlaveNode->posz_expr, "Z0-ZA1-ZA2-ZA5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x52);	//N100
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x04;
	StrCopy(pSlaveNode->posz_expr, "Z0-ZA1-ZA2-ZA5+ZA4", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x5C);	//N110
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "XA6", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0-ZA1", 20);
	return true;
}
//直流直线塔-ZC塔型
bool CTowerTemplateToModel::InitZCTowerTmplInfo()
{
	if (m_pTower == NULL)
		return false;
	//模型参数
	CDesParasOper oper(&m_pTower->hashParams);
	oper.AddParamValue(KEY4C("Z0"), 3600, 0);		//后台隐藏参数，不显示
	oper.AddParamValue(KEY4C("Z1"), 3200, 1, "横担根部高度");
	oper.AddParamValue(KEY4C("Z2"), 9000, 1, "塔身直段高度");
	oper.AddParamValue(KEY4C("Z3"), 15000, 1, "塔身第一坡段高度");
	oper.AddParamValue(KEY4C("Z4"), 66000, 1, "呼高");
	oper.AddParamValue(KEY4C("K1"), 5500, 1, "二次变坡口宽");
	oper.AddParamValue(KEY4C("K2"), 4000, 1, "一次变坡口宽");
	oper.AddParamValue(KEY4C("K3"), 13900, 1, "塔腿正面根开");
	oper.AddParamValue(KEY4C("P1"), 0.05, 1, "变坡上侧坡度");
	oper.AddParamValue(KEY4C("P2"), 0.1, 1, "变坡下侧坡度");
	//右侧复合横担TX12
	oper.AddParamValue(KEY4C("XA1"), 13650, 2, "地线挂点中心距");
	oper.AddParamValue(KEY4C("XA2"), 800, 2, "地线挂点悬臂长度");
	oper.AddParamValue(KEY4C("XA3"), 18580, 2, "横担长度");
	oper.AddParamValue(KEY4C("XA4"), 7880, 2, "横担变坡外侧长度");
	oper.AddParamValue(KEY4C("XA5"), 2533, 2, "横担挂架长度");
	oper.AddParamValue(KEY4C("XA6"), 2800, 2, "地线支架根部宽度");
	oper.AddParamValue(KEY4C("YA1"), 800, 2, "地线挂点纵向宽度");
	oper.AddParamValue(KEY4C("YA2"), 1600, 2, "横担纵向宽度");
	oper.AddParamValue(KEY4C("ZA1"), 6000, 2, "地线支架高度");
	oper.AddParamValue(KEY4C("ZA2"), 800, 2, "地线挂点悬臂高度");
	oper.AddParamValue(KEY4C("ZA3"), 2800, 2, "横担变坡外侧高度");
	oper.AddParamValue(KEY4C("ZA4"), 900, 2, "横担挂架高度");
	//塔身主控节点
	SLAVE_NODE* pSlaveNode = NULL;
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x2E);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "K2*0.5", 20);
	StrCopy(pSlaveNode->posy_expr, "K2*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x6A);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x32);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "K2*0.5", 20);
	StrCopy(pSlaveNode->posy_expr, "K2*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x36);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "K1*0.5", 20);
	StrCopy(pSlaveNode->posy_expr, "K1*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2+Z3", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x3A);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "K3*0.5", 20);
	StrCopy(pSlaveNode->posy_expr, "K3*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z4", 20);
	//横担
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x3E);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "XA1", 20);
	StrCopy(pSlaveNode->posy_expr, "YA1*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1-ZA1", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x4E);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "XA1-XA2", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1-ZA1-ZA2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x46);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "XA1-XA2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x52);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1-ZA1", 20);
	
	//节点依赖关系
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x56);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "XA3", 20);
	StrCopy(pSlaveNode->posy_expr, "YA2*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x7A);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "XA3-XA5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x76);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "XA3-XA4", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x7E);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "XA3", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1-ZA4", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x6E);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1-ZA4", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x66);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "XA3-XA4", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1-ZA3", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x62);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "XA3-XA4-XA6", 20);
	return true;
}
//猫头塔-ZM塔型
bool CTowerTemplateToModel::InitZMTowerTmplInfo()
{
	if (m_pTower == NULL)
		return false;
	//模型参数
	CDesParasOper oper(&m_pTower->hashParams);
	oper.AddParamValue(KEY4C("Z0"), 12600, 0);	//后台隐藏参数，不显示
	oper.AddParamValue(KEY4C("Z"), 45000, 1, "呼高");
	oper.AddParamValue(KEY4C("K1"), 3200, 1, "变坡口宽");
	oper.AddParamValue(KEY4C("K2"), 11580, 1, "塔腿正面根开");
	oper.AddParamValue(KEY4C("P"), 0.1, 1, "坡度");
	//猫头塔头
	oper.AddParamValue(KEY4C("X1"), 7700, 2, "边导挂点中心距");
	oper.AddParamValue(KEY4C("X2"), 5200, 2, "X2");
	oper.AddParamValue(KEY4C("X3"), 1850, 2, "X3");
	oper.AddParamValue(KEY4C("X4"), 3300, 2, "X4");
	oper.AddParamValue(KEY4C("X5"), 2067, 2, "X5");
	oper.AddParamValue(KEY4C("X6"), 5100, 2, "X6");
	oper.AddParamValue(KEY4C("X7"), 5950, 2, "地线挂点中心距");
	oper.AddParamValue(KEY4C("X8"), 500, 2, "地线架悬臂高");
	oper.AddParamValue(KEY4C("Y1"), 1200, 2, "边横担纵向宽度");
	oper.AddParamValue(KEY4C("Y2"), 1300, 2, "中横担纵向宽度");
	oper.AddParamValue(KEY4C("Y3"), 700, 2, "地线挂点纵向宽");
	oper.AddParamValue(KEY4C("Z1"), 3100, 2, "下导线挂点距变坡高度");
	oper.AddParamValue(KEY4C("Z2"), 6500, 2, "下曲臂高度");
	oper.AddParamValue(KEY4C("Z3"), 3200, 2, "上曲臂高度");
	oper.AddParamValue(KEY4C("Z4"), 2500, 2, "地线支架高度");
	oper.AddParamValue(KEY4C("Z5"), 1600, 2, "Z5");
	oper.AddParamValue(KEY4C("Z6"), 1500, 2, "Z6");
	oper.AddParamValue(KEY4C("Z7"), 1300, 2, "Z7");
	oper.AddParamValue(KEY4C("Z8"), 1100, 2, "中横担隔面高度");
	oper.AddParamValue(KEY4C("Z9"), 600, 2, "地线支架悬臂高度");

	//节点依赖关系
	//塔身主控点
	SLAVE_NODE* pSlaveNode = NULL;
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x69);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "K1*0.5", 20);
	StrCopy(pSlaveNode->posy_expr, "K1*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xBD);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "K2*0.5", 20);
	StrCopy(pSlaveNode->posy_expr, "K2*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0-Z1+Z", 20);
	//猫头
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x2F);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "X7", 20);
	StrCopy(pSlaveNode->posy_expr, "Y3*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0-Z1-Z2-Z3-Z4", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x37);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "X7-X8", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0-Z1-Z2-Z3-Z4-Z9", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x33);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "Z0-Z1-Z2-Z3-Z4", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x2B);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "X7-X8", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x3B);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "X6", 20);
	StrCopy(pSlaveNode->posy_expr, "Y2*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0-Z1-Z2-Z3-Z7", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x3F);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "X4", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x4B);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "X4+X5", 20);
	StrCopy(pSlaveNode->posy_expr, "Y2*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0-Z1-Z2-Z3", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x47);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "X4", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0-Z1-Z2-Z3", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x45);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x04;
	StrCopy(pSlaveNode->posz_expr, "Z0-Z1-Z2-Z3-Z7+Z8", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x4F);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "Z0-Z1-Z2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x57);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "X2+X3", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0-Z1-Z6", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x63);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "X2", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0-Z1-Z6", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x53);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "X1", 20);
	StrCopy(pSlaveNode->posy_expr, "Y1*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0-Z1", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x5F);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "Z0-Z1", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x67);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "Z0-Z5", 20);
	return true;
}
//同塔双回路直线塔-SZ塔型
bool CTowerTemplateToModel::InitSZTowerTmplInfo()
{
	if (m_pTower == NULL)
		return false;
	//模型参数
	CDesParasOper oper(&m_pTower->hashParams);
	oper.AddParamValue(KEY4C("Z0"), 0, 0);	//后台隐藏参数，不显示
	oper.AddParamValue(KEY4C("Z1"), 2000, 1, "横担根部高");
	oper.AddParamValue(KEY4C("Z2"), 11700, 1, "上中层高");
	oper.AddParamValue(KEY4C("Z3"), 12900, 1, "中下层高");
	oper.AddParamValue(KEY4C("Z4"), 3000, 1, "下层至变坡高");
	oper.AddParamValue(KEY4C("Z5"), 48000, 1, "呼高");
	oper.AddParamValue(KEY4C("K1"), 4400, 1, "变坡口宽");
	oper.AddParamValue(KEY4C("K2"), 2032, 1, "顶面宽");
	oper.AddParamValue(KEY4C("K3"), 13400, 1, "基础正面根开");
	oper.AddParamValue(KEY4C("P1"), 0.04, 1, "变坡上侧坡度");
	oper.AddParamValue(KEY4C("P2"), 0.1, 1, "变坡下侧坡度");
	oper.AddParamValue(KEY4C("L"), 0, 1, "绝缘子串长度");
	oper.AddParamValue(KEY4C("A"), 49.573921, 1, "地线保护角");
	//第一层导地线复合横担TX13
	oper.AddParamValue(KEY4C("XA1"), 9750, 2, "地线挂点中心距");
	oper.AddParamValue(KEY4C("XA2"), 800, 2, "地线架悬臂长");
	oper.AddParamValue(KEY4C("XA3"), 11250, 2, "上导挂点中心距");
	oper.AddParamValue(KEY4C("XA4"), 6650, 2, "地线架根部中心距");
	oper.AddParamValue(KEY4C("XA5"), 1280, 2, "地线支架根部宽度");
	oper.AddParamValue(KEY4C("YA1"), 600, 2, "地线架纵向宽");
	oper.AddParamValue(KEY4C("YA2"), 700, 2, "上导横担纵向宽");
	oper.AddParamValue(KEY4C("ZA1"), 2800, 2, "地面支架高");
	oper.AddParamValue(KEY4C("ZA2"), 600, 2, "地线架悬臂高");
	oper.AddParamValue(KEY4C("ZA3"), 1700, 2, "上导横担根部高");
	//第二层导线横担HX2
	oper.AddParamValue(KEY4C("XB"), 9450, 3, "中导挂点中心距");
	oper.AddParamValue(KEY4C("YB"), 1200, 3, "中导横担纵向宽");
	oper.AddParamValue(KEY4C("ZB"), 2400, 3, "中导横担根部高");
	//第三层导线横担HX2
	oper.AddParamValue(KEY4C("XC"), 12300, 4, "下导挂点中心距");
	oper.AddParamValue(KEY4C("YC"), 700, 4, "下导横担纵向宽");
	oper.AddParamValue(KEY4C("ZC"), 3000, 4, "下导横担根部高");
	//节点依赖关系
	//塔身关联节点
	SLAVE_NODE* pSlaveNode = NULL;
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x59);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "K2*0.5", 20);
	StrCopy(pSlaveNode->posy_expr, "K2*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x61);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x79);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x89);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2+Z3", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x5D);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "K1*0.5", 20);
	StrCopy(pSlaveNode->posy_expr, "K1*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2+Z3+Z4", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x155);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "K3*0.5", 20);
	StrCopy(pSlaveNode->posy_expr, "K3*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+ZA1+Z2+Z3+Z5", 20);
	//第一导导地线横担
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x2D);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "XA1", 20);
	StrCopy(pSlaveNode->posy_expr, "YA1*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1-ZA1+ZA2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x31);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "XA1-XA2", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1-ZA1", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x35);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "XA1-XA2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x39);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1-ZA1+ZA2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x45);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "XA3", 20);
	StrCopy(pSlaveNode->posy_expr, "YA2*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x4D);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "XA4", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x3D);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "XA4", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1-ZA3", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x41);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "XA4+XA5", 20);
	//第二导线横担
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x75);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2-ZB", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x8D);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "XB", 20);
	StrCopy(pSlaveNode->posy_expr, "YB*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2", 20);
	//第三导线横担
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x85);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2+Z3-ZC", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x99);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "XC", 20);
	StrCopy(pSlaveNode->posy_expr, "YC*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2+Z3", 20);
	return true;
}
//直流单回路耐张塔-JC塔型
bool CTowerTemplateToModel::InitJCTowerTmplInfo()
{
	if (m_pTower == NULL)
		return false;
	//模型参数
	CDesParasOper oper(&m_pTower->hashParams);
	oper.AddParamValue(KEY4C("Z0"), 1000, 0);		//后台隐藏参数，不显示
	oper.AddParamValue(KEY4C("Z1"), 15000, 1, "导地线层高");
	oper.AddParamValue(KEY4C("Z2"), 4500, 1, "横担下平面至变坡处高");
	oper.AddParamValue(KEY4C("Z3"), 51000, 1, "呼高");
	oper.AddParamValue(KEY4C("K1"), 4800, 1, "变坡口宽");
	oper.AddParamValue(KEY4C("K2"), 2460, 1, "顶面宽");
	oper.AddParamValue(KEY4C("K3"), 17040, 1, "基础正面根开");
	oper.AddParamValue(KEY4C("P1"), 0.06, 1, "变坡上侧坡度");
	oper.AddParamValue(KEY4C("P2"), 0.12, 1, "变坡下侧坡度");
	//地线支架-HX1
	oper.AddParamValue(KEY4C("ZAB"), 3600, 2, "地线支架根部高");
	//左侧地线支架
	oper.AddParamValue(KEY4C("XA"), 13000, 2, "地线挂点中心距");
	oper.AddParamValue(KEY4C("YA"), 2460, 2, "地线支架纵向宽");
	//右侧地线支架
	oper.AddParamValue(KEY4C("XB"), 13000, 3, "地线挂点中心距");
	oper.AddParamValue(KEY4C("YB"), 2460, 3, "地线支架纵向宽");
	//横担-HX15
	oper.AddParamValue(KEY4C("ZCD"), 4200, 4, "横担根部高");
	//左侧横担
	oper.AddParamValue(KEY4C("XC1"), 9100, 4, "导线挂点中心距");
	oper.AddParamValue(KEY4C("XC2"), 11250, 4, "跳线架长度");
	oper.AddParamValue(KEY4C("XC3"), 1380, 4, "挂点间距");
	oper.AddParamValue(KEY4C("XC4"), 2640, 4, "X4");
	oper.AddParamValue(KEY4C("YC"), 4260, 4, "横担纵向宽");
	oper.AddParamValue(KEY4C("ZC"), 2100, 4, "跳线架高度");
	//右侧横担
	oper.AddParamValue(KEY4C("XD1"), 9100, 5, "导线挂点中心距");
	oper.AddParamValue(KEY4C("XD2"), 11250, 5, "跳线架长度");
	oper.AddParamValue(KEY4C("XD3"), 1380, 5, "挂点间距");
	oper.AddParamValue(KEY4C("XD4"), 2640, 5, "X4");
	oper.AddParamValue(KEY4C("YD"), 4260, 5, "横担纵向宽");
	oper.AddParamValue(KEY4C("ZD"), 2100, 5, "跳线架高度");
	//节点依赖关系
	//塔身主控节点
	SLAVE_NODE* pSlaveNode = NULL;
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x2C);//200节点
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "K2*0.5", 20);
	StrCopy(pSlaveNode->posy_expr, "K2*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x4C); //800节点
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x30); //1000节点
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "K1*0.5", 20);
	StrCopy(pSlaveNode->posy_expr, "K1*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z2 ", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0xE0); //3900节点
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "K3*0.5", 20);
	StrCopy(pSlaveNode->posy_expr, "K3*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1+Z3 ", 20);
	//地线支架
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x3C);//250节点
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "Z0+ZAB", 20);
	//左地线
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x56); //10节点
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XA", 20);
	StrCopy(pSlaveNode->posy_expr, "YA*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0", 20);
	//右地线
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x54); //10节点
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XB", 20);
	StrCopy(pSlaveNode->posy_expr, "YB*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0", 20);
	//导线横担
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x48); //450节点
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1-ZCD", 20);
	//左导线横担
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x6E); //321节点
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XC1-XC2", 20);
	StrCopy(pSlaveNode->posy_expr, "YC*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1 ", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x5E); //521节点
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XC1-XC3*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x5A); //551节点
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XC1+XC3*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x6A); //311节点
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, " -XC1-XC2+XC4 ", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1-ZC", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x66); //481节点
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-XC1-XC3*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x62); //601节点
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1-ZC", 20);
	//右导横担
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x6C); //320节点
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XD1+XD2", 20);
	StrCopy(pSlaveNode->posy_expr, "YD*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1 ", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x5C); //520节点
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XD1+XD3*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x58); //550节点
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XD1-XD3*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x68); //310节点
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XD1+XD2-XD4", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1-ZD", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x64); //480节点
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "XD1+XD3*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x60); //600节点
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posz_expr, "Z0+Z1-ZD ", 20);
	return true;
}
//直流单回路耐张塔 - JBB塔型
bool CTowerTemplateToModel::InitJBBTowerTmplInfo()
{
	if (m_pTower == NULL)
		return false;
	//模型参数
	CDesParasOper oper(&m_pTower->hashParams);
	oper.AddParamValue(KEY4C("Z0"), 31000, 0);	//后台隐藏参数，不显示
	oper.AddParamValue(KEY4C("Z"), 39000, 1, "呼高");
	oper.AddParamValue(KEY4C("K1"), 9000, 1, "变坡口宽");
	oper.AddParamValue(KEY4C("K2"), 17550, 1, "塔腿正面根开");
	oper.AddParamValue(KEY4C("P"), 0.19, 1, "坡度");
	//酒杯塔头
	oper.AddParamValue(KEY4C("X1"), 11500, 2, "左地线挂点中心距");
	oper.AddParamValue(KEY4C("X2"), 10300, 2, "右地线挂点中心距");
	oper.AddParamValue(KEY4C("X3"), 600, 2, "中相移距");
	oper.AddParamValue(KEY4C("X4"), 5800, 2, "中横担跨度");
	oper.AddParamValue(KEY4C("X5"), 3200, 2, "上曲臂宽度");
	oper.AddParamValue(KEY4C("X6"), 6000, 2, "左导线挂点中心距");
	oper.AddParamValue(KEY4C("X7"), 2000, 2, "左跳线支教长度");
	oper.AddParamValue(KEY4C("X8"), 4800, 2, "右导线挂点中心距");
	oper.AddParamValue(KEY4C("X9"), 1000, 2, "右跳线支教长度");
	oper.AddParamValue(KEY4C("W1"), 400, 2, "左地线支架宽度");
	oper.AddParamValue(KEY4C("W2"), 400, 2, "右地线支架宽度");
	oper.AddParamValue(KEY4C("W3"), 570, 2, "左导线挂点宽度");
	oper.AddParamValue(KEY4C("W4"), 570, 2, "中导线挂点宽度");
	oper.AddParamValue(KEY4C("W5"), 570, 2, "右导线挂点宽度");
	oper.AddParamValue(KEY4C("Y1"), 300, 2, "地线挂点纵向宽");
	oper.AddParamValue(KEY4C("Y2"), 3600, 2, "横担根部纵向宽");
	oper.AddParamValue(KEY4C("Y3"), 3600, 2, "边导挂点纵向宽");
	oper.AddParamValue(KEY4C("Z1"), 9500, 2, "下曲臂高度");
	oper.AddParamValue(KEY4C("Z2"), 7000, 2, "上曲臂高度");
	oper.AddParamValue(KEY4C("Z3"), 14500, 2, "地线支架高度");
	oper.AddParamValue(KEY4C("Z4"), 3000, 2, "中横担隔面高度");
	oper.AddParamValue(KEY4C("Z5"), 1200, 2, "边横担根部高度");
	//节点依赖关系
	//塔身依赖节点
	SLAVE_NODE* pSlaveNode = NULL;
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x71);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "K1*0.5", 20);
	StrCopy(pSlaveNode->posy_expr, "K1*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x79);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "K2*0.5", 20);
	StrCopy(pSlaveNode->posy_expr, "K2*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0+Z-Z1-Z2", 20);
	//酒杯塔头
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x75);
	pSlaveNode->cFlag = 0x04;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posz_expr, "Z0-Z1", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x3B);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "X4+X5", 20);
	StrCopy(pSlaveNode->posy_expr, "Y2*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0-Z1-Z2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x4B);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "X4", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x5F);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "X4+X5+X8+X9", 20);
	StrCopy(pSlaveNode->posy_expr, "Y3*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0-Z1-Z2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x61);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "X4+X5+X8+W5*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x63);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "X4+X5+X8-W5*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x5B);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "X4+X5+X8+W5*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0-Z1-Z2-Z5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x6B);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-X4-X5-X6-X7", 20);
	StrCopy(pSlaveNode->posy_expr, "Y3*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0-Z1-Z2", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x6D);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-X4-X5-X6-W3*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x6F);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-X4-X5-X6+W3*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x5D);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-X4-X5-X6-X7", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0-Z1-Z2-Z5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x65);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-X4-X5-X6-W3*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0-Z1-Z2-Z5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x59);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-X3+W4*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x57);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-X3-W4*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x55);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-X3+W4*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x53);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-X3-W4*0.5", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x33);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "X4+X5", 20);
	StrCopy(pSlaveNode->posy_expr, "Y2*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0-Z1-Z2-Z4", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x37);
	pSlaveNode->cFlag = 0x01;
	pSlaveNode->cbMirFlag = 0x07;
	StrCopy(pSlaveNode->posx_expr, "X4", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x2B);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "X2+W2*0.5", 20);
	StrCopy(pSlaveNode->posy_expr, "Y1*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0-Z1-Z2-Z3", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x2D);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "X2-W2*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0-Z1-Z2-Z3", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x2F);
	pSlaveNode->cFlag = 0x07;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-X1-W1*0.5", 20);
	StrCopy(pSlaveNode->posy_expr, "Y1*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0-Z1-Z2-Z3", 20);
	pSlaveNode = m_pTower->hashSlaveNodes.Append(0x31);
	pSlaveNode->cFlag = 0x05;
	pSlaveNode->cbMirFlag = 0x01;
	StrCopy(pSlaveNode->posx_expr, "-X1+W1*0.5", 20);
	StrCopy(pSlaveNode->posz_expr, "Z0-Z1-Z2-Z3", 20);
	return true;
}
//初始化塔型模板的尺寸标注关联项
void CTowerTemplateToModel::InitTempParaSketchs(BYTE ciTowerType, CHashList<PARA_SKETCH_ITEM>& hashParaSketchs)
{
	CParaSketchOper oper(&hashParaSketchs);
	if (ciTowerType == TYPE_SZC)
	{
		oper.AddItemValue(KEY4C("Z"), 3, 0X54, 0X38);
		oper.AddItemValue(KEY4C("Z1"), 3, 0X2C, 0X54);
		oper.AddItemValue(KEY4C("Z2"), 3, 0X54, 0X58);
		oper.AddItemValue(KEY4C("Z3"), 3, 0X58, 0X5C);
		oper.AddItemValue(KEY4C("Z4"), 3, 0X5C, 0X3C);
		oper.AddItemValue(KEY4C("Z5"), 3, 0X5C, 0XC4);
		oper.AddItemValue(KEY4C("K1"), 1, 0X3C, 0X3A);
		oper.AddItemValue(KEY4C("K2"), 1, 0X38, 0X36);
		oper.AddItemValue(KEY4C("K3"), 1, 0XC4, 0XC2);
		//
		oper.AddItemValue(KEY4C("XA1"), 1, 0X2A);//,0XE6);
		oper.AddItemValue(KEY4C("XA2"), 1, 0X52);//,0XE6);
		oper.AddItemValue(KEY4C("XA3"), 1, 0X2A, 0X2E);
		oper.AddItemValue(KEY4C("ZA1"), 3, 0X42, 0X36);
		oper.AddItemValue(KEY4C("ZA2"), 3, 0X2A, 0X3E);
		oper.AddItemValue(KEY4C("ZA3"), 3, 0X52, 0X4E);
		oper.AddItemValue(KEY4C("YA1"), 2, 0X2A, 0X2B);
		oper.AddItemValue(KEY4C("YA2"), 2, 0X52, 0X53);

		oper.AddItemValue(KEY4C("XB"), 1, 0X56);//,0X6E);
		oper.AddItemValue(KEY4C("YB"), 2, 0X56, 0X57);
		oper.AddItemValue(KEY4C("ZB"), 3, 0X56, 0X6A);

		oper.AddItemValue(KEY4C("XC"), 1, 0X5A);//,0X7E);
		oper.AddItemValue(KEY4C("YC"), 2, 0X5A, 0X5B);
		oper.AddItemValue(KEY4C("ZC"), 3, 0X5A, 0X7A);
	}
	else if (ciTowerType == TYPE_JG)
	{
		oper.AddItemValue(KEY4C("Z1"), 3, 0X31, 0X10D);
		oper.AddItemValue(KEY4C("Z2"), 3, 0X10D, 0X3F);
		oper.AddItemValue(KEY4C("Z3"), 3, 0X3F, 0X35);
		oper.AddItemValue(KEY4C("Z4"), 3, 0X3F, 0X7F);
		oper.AddItemValue(KEY4C("K1"), 1, 0X35, 0X33);
		oper.AddItemValue(KEY4C("K2"), 1, 0X2B, 0X29);
		oper.AddItemValue(KEY4C("K3"), 1, 0X7F, 0X7D);

		oper.AddItemValue(KEY4C("XA"), 1, 0X31);
		oper.AddItemValue(KEY4C("YA"), 2, 0X31, 0X32);
		oper.AddItemValue(KEY4C("ZAB"), 3, 0X31, 0X4B);

		oper.AddItemValue(KEY4C("XB1"), 1, 0X2F);
		oper.AddItemValue(KEY4C("XB2"), 1, 0X2F, 0X45);
		oper.AddItemValue(KEY4C("XB3"), 1, 0X43, 0X41);
		oper.AddItemValue(KEY4C("YB"), 2, 0X46, 0X45);
		oper.AddItemValue(KEY4C("YB1"), 2, 0XFE2, 0X42);
		oper.AddItemValue(KEY4C("YB2"), 2, 0XFE2, 0X41);
		oper.AddItemValue(KEY4C("ZB"), 3, 0X45, 0X51);

		oper.AddItemValue(KEY4C("XC1"), 1, 0X237F);
		oper.AddItemValue(KEY4C("XC2"), 1, 0X3F, 0X237F);
		oper.AddItemValue(KEY4C("XC3"), 1, 0X5D, 0X65);
		oper.AddItemValue(KEY4C("YC"), 2, 0X3F, 0X40);
		oper.AddItemValue(KEY4C("ZCD"), 3, 0X3F, 0X59);
		oper.AddItemValue(KEY4C("ZC"), 3, 0X3F, 0X67);

		oper.AddItemValue(KEY4C("XD1"), 1, 0X237D);//未明是哪个节点
		oper.AddItemValue(KEY4C("XD2"), 1, 0X5B, 0X3B);
		oper.AddItemValue(KEY4C("YD"), 2, 0X3B, 0X3C);
		oper.AddItemValue(KEY4C("ZD"), 3, 0X3B, 0X63);
	}
	else if (ciTowerType == TYPE_JG2)
	{
		oper.AddItemValue(KEY4C("Z1"), 3, 0X31, 0X10D);
		oper.AddItemValue(KEY4C("Z2"), 3, 0X10D, 0X3F);
		oper.AddItemValue(KEY4C("Z3"), 3, 0X3F, 0X35);
		oper.AddItemValue(KEY4C("Z4"), 3, 0X3F, 0X7F);
		oper.AddItemValue(KEY4C("K1"), 1, 0X35, 0X33);
		oper.AddItemValue(KEY4C("K2"), 1, 0X2B, 0X29);
		oper.AddItemValue(KEY4C("K3"), 1, 0X7F, 0X7D);

		oper.AddItemValue(KEY4C("XA"), 1, 0X31);
		oper.AddItemValue(KEY4C("YA"), 2, 0X31, 0X32);
		oper.AddItemValue(KEY4C("ZAB"), 3, 0X31, 0X4B);

		oper.AddItemValue(KEY4C("XB1"), 1, 0X2F);
		oper.AddItemValue(KEY4C("XB2"), 1, 0X2F, 0X45);
		oper.AddItemValue(KEY4C("XB3"), 1, 0X43, 0X41);
		oper.AddItemValue(KEY4C("YB"), 2, 0X46, 0X45);
		oper.AddItemValue(KEY4C("YB1"), 2, 0XFE2, 0X42);
		oper.AddItemValue(KEY4C("YB2"), 2, 0XFE2, 0X41);
		oper.AddItemValue(KEY4C("ZB"), 3, 0X45, 0X51);

		oper.AddItemValue(KEY4C("XC"), 1, 0X2382);
		oper.AddItemValue(KEY4C("YC"), 2, 0X2382, 0X2383);
		oper.AddItemValue(KEY4C("ZC"), 3, 0X6D, 0X2382);

		oper.AddItemValue(KEY4C("ZDE"), 3, 0X3F, 0X59);
		oper.AddItemValue(KEY4C("XD1"), 1, 0X237F);
		oper.AddItemValue(KEY4C("XD2"), 1, 0X3F, 0X237F);
		oper.AddItemValue(KEY4C("XD3"), 1, 0X5D, 0X65);
		oper.AddItemValue(KEY4C("YD"), 2, 0X3F, 0X40);
		oper.AddItemValue(KEY4C("ZD"), 3, 0X3F, 0X67);

		oper.AddItemValue(KEY4C("XE1"), 1, 0X237D);//未明是哪个节点
		oper.AddItemValue(KEY4C("XE2"), 1, 0X5B, 0X3B);
		oper.AddItemValue(KEY4C("YD"), 2, 0X3B, 0X3C);
		oper.AddItemValue(KEY4C("ZE"), 3, 0X3B, 0X63);
	}
	else if (ciTowerType == TYPE_SJ)
	{
		oper.AddItemValue(KEY4C("Z1"), 3, 0X2C, 0X70);
		oper.AddItemValue(KEY4C("Z2"), 3, 0X70, 0X88);
		oper.AddItemValue(KEY4C("Z3"), 3, 0X88, 0XA0);
		oper.AddItemValue(KEY4C("Z4"), 3, 0XA0, 0X34);
		oper.AddItemValue(KEY4C("Z5"), 3, 0XA0, 0XDC);
		oper.AddItemValue(KEY4C("K1"), 1, 0X34, 0X32);
		oper.AddItemValue(KEY4C("K2"), 1, 0X30, 0X2E);
		oper.AddItemValue(KEY4C("K3"), 1, 0XDC, 0XDA);

		oper.AddItemValue(KEY4C("ZAB"), 3, 0X2C, 0X38);
		oper.AddItemValue(KEY4C("XA"), 1, 0X2C);
		oper.AddItemValue(KEY4C("YA"), 2, 0X2C, 0X2D);
		oper.AddItemValue(KEY4C("XB"), 1, 0X2A);
		oper.AddItemValue(KEY4C("YB"), 2, 0X2A, 0X2B);

		oper.AddItemValue(KEY4C("ZCD"), 3, 0X3C, 0X70);
		oper.AddItemValue(KEY4C("XC1"), 1, 0X81F);
		oper.AddItemValue(KEY4C("XC2"), 1, 0X81F, 0X70);
		oper.AddItemValue(KEY4C("XC3"), 1, 0X74, 0X78);
		oper.AddItemValue(KEY4C("XC4"), 1, 0X68, 0X6C);
		oper.AddItemValue(KEY4C("YC"), 2, 0X70, 0X71);
		oper.AddItemValue(KEY4C("ZC2"), 3, 0X70, 0X6C);
		oper.AddItemValue(KEY4C("XD1"), 1, 0X821);
		oper.AddItemValue(KEY4C("XD2"), 1, 0X821, 0X6E);
		oper.AddItemValue(KEY4C("XD3"), 1, 0X72, 0X76);
		oper.AddItemValue(KEY4C("XD4"), 1, 0X66, 0X6A);
		oper.AddItemValue(KEY4C("YD"), 2, 0X6E, 0X6F);
		oper.AddItemValue(KEY4C("ZD2"), 3, 0X6E, 0X6A);

		oper.AddItemValue(KEY4C("ZEF"), 3, 0X50, 0X88);
		oper.AddItemValue(KEY4C("XE1"), 1, 0X829);
		oper.AddItemValue(KEY4C("XE2"), 1, 0X829, 0X88);
		oper.AddItemValue(KEY4C("XE3"), 1, 0X90, 0X8C);
		oper.AddItemValue(KEY4C("XE4"), 1, 0X84, 0X80);
		oper.AddItemValue(KEY4C("YE"), 2, 0X88, 0X89);
		oper.AddItemValue(KEY4C("ZE2"), 3, 0X88, 0X84);
		oper.AddItemValue(KEY4C("XF1"), 1, 0X823);
		oper.AddItemValue(KEY4C("XF2"), 1, 0X823, 0X86);
		oper.AddItemValue(KEY4C("XF3"), 1, 0X8E, 0X8A);
		oper.AddItemValue(KEY4C("XF4"), 1, 0X7E, 0X82);
		oper.AddItemValue(KEY4C("YF"), 2, 0X86, 0X87);
		oper.AddItemValue(KEY4C("ZF2"), 3, 0X86, 0X82);

		oper.AddItemValue(KEY4C("ZGH"), 3, 0X60, 0XA0);
		oper.AddItemValue(KEY4C("XG1"), 1, 0X827);
		oper.AddItemValue(KEY4C("XG2"), 1, 0X827, 0XA0);
		oper.AddItemValue(KEY4C("XG3"), 1, 0XA4, 0XA8);
		oper.AddItemValue(KEY4C("XG4"), 1, 0X9C, 0X98);
		oper.AddItemValue(KEY4C("YG"), 2, 0XA0, 0XA1);
		oper.AddItemValue(KEY4C("ZG2"), 3, 0XA0, 0X9C);
		oper.AddItemValue(KEY4C("XH1"), 1, 0X825);
		oper.AddItemValue(KEY4C("XH2"), 1, 0X825, 0X9E);
		oper.AddItemValue(KEY4C("XH3"), 1, 0XA6, 0XA2);
		oper.AddItemValue(KEY4C("XH4"), 1, 0X96, 0X9A);
		oper.AddItemValue(KEY4C("YH"), 2, 0X9E, 0X9F);
		oper.AddItemValue(KEY4C("ZH2"), 3, 0X9E, 0X9A);
	}
	else if (ciTowerType == TYPE_SJB)
	{
		oper.AddItemValue(KEY4C("Z1"), 3, 0X2C, 0XA2);
		oper.AddItemValue(KEY4C("Z2"), 3, 0XA2, 0XA6);
		oper.AddItemValue(KEY4C("Z3"), 3, 0XA6, 0XB6);
		oper.AddItemValue(KEY4C("Z4"), 3, 0XB6, 0X30);
		oper.AddItemValue(KEY4C("Z5"), 3, 0XB6, 0XFC);
		oper.AddItemValue(KEY4C("K1"), 1, 0X30, 0X2E);
		oper.AddItemValue(KEY4C("K2"), 1, 0X2C, 0X2A);
		oper.AddItemValue(KEY4C("K3"), 1, 0XFC, 0XFA);

		oper.AddItemValue(KEY4C("XA1"), 1, 0X96);
		oper.AddItemValue(KEY4C("XA2"), 1, 0X836);
		oper.AddItemValue(KEY4C("XA3"), 1, 0X836, 0XA2);
		oper.AddItemValue(KEY4C("XA4"), 1, 0XA0, 0X9E);
		oper.AddItemValue(KEY4C("XA5"), 1, 0X96, 0X98);
		oper.AddItemValue(KEY4C("XA6"), 1, 0X98, 0X94);
		oper.AddItemValue(KEY4C("YA1"), 2, 0XA2, 0XA3);
		oper.AddItemValue(KEY4C("YA2"), 2, 0X96, 0X97);
		oper.AddItemValue(KEY4C("ZA1"), 3, 0XA2, 0X96);
		oper.AddItemValue(KEY4C("ZA2"), 3, 0XA2, 0XCC);
		oper.AddItemValue(KEY4C("ZA3"), 3, 0X96, 0X94);
		//右侧复合横担TX3
		oper.AddItemValue(KEY4C("XB1"), 1, 0X90);
		oper.AddItemValue(KEY4C("XB2"), 1, 0X838);
		oper.AddItemValue(KEY4C("XB3"), 1, 0X9A, 0X9C);
		oper.AddItemValue(KEY4C("XB4"), 1, 0X90, 0X92);
		oper.AddItemValue(KEY4C("XB5"), 1, 0X92, 0X8E);
		oper.AddItemValue(KEY4C("YB1"), 2, 0X9C, 0X9D);
		oper.AddItemValue(KEY4C("YB2"), 2, 0X90, 0X91);
		oper.AddItemValue(KEY4C("ZB1"), 3, 0X9C, 0X90);
		oper.AddItemValue(KEY4C("ZB2"), 3, 0XC6, 0X9C);
		oper.AddItemValue(KEY4C("ZB3"), 3, 0X90, 0X8E);
		//第二层						
		oper.AddItemValue(KEY4C("ZCD"), 3, 0XA6, 0X4C);
		//左侧横担HX14
		oper.AddItemValue(KEY4C("XC1"), 1, 0X83A);
		oper.AddItemValue(KEY4C("XC2"), 1, 0X83A, 0XA6);
		oper.AddItemValue(KEY4C("XC3"), 1, 0XAE, 0XB0);
		oper.AddItemValue(KEY4C("XC4"), 1, 0XDA, 0XD6);
		oper.AddItemValue(KEY4C("YC"), 2, 0XA6, 0XA7);
		oper.AddItemValue(KEY4C("ZC2"), 3, 0XA6, 0XDA);
		//右侧横担HX13
		oper.AddItemValue(KEY4C("XD1"), 1, 0X83C);
		oper.AddItemValue(KEY4C("XD2"), 1, 0XAA, 0XA8);
		oper.AddItemValue(KEY4C("XD3"), 1, 0XD2, 0XD4);
		oper.AddItemValue(KEY4C("YD"), 2, 0XA8, 0XA9);
		oper.AddItemValue(KEY4C("ZD2"), 3, 0XA8, 0XD4);
		//第三层
		oper.AddItemValue(KEY4C("ZEF"), 3, 0X60, 0XB6);
		//左侧横担HX14
		oper.AddItemValue(KEY4C("XE1"), 1, 0X840);
		oper.AddItemValue(KEY4C("XE2"), 1, 0X840, 0XB6);
		oper.AddItemValue(KEY4C("XE3"), 1, 0XBE, 0XC0);
		oper.AddItemValue(KEY4C("XE4"), 1, 0XE8, 0XE4);
		oper.AddItemValue(KEY4C("YE"), 2, 0XB6, 0XB7);
		oper.AddItemValue(KEY4C("ZE2"), 3, 0XB6, 0XE8);
		//右侧横担HX13
		oper.AddItemValue(KEY4C("XF1"), 1, 0X83E);
		oper.AddItemValue(KEY4C("XF2"), 1, 0XBA, 0XB8);
		oper.AddItemValue(KEY4C("XF3"), 1, 0XDE, 0XE0);
		oper.AddItemValue(KEY4C("YF"), 2, 0XB8, 0XB9);
		oper.AddItemValue(KEY4C("ZF2"), 3, 0XB8, 0XE0);
	}
	else if (ciTowerType == TYPE_SJC)
	{
		oper.AddItemValue(KEY4C("Z1"), 3, 0X2B, 0XCF);
		oper.AddItemValue(KEY4C("Z2"), 3, 0XCF, 0XDD);
		oper.AddItemValue(KEY4C("Z3"), 3, 0XDD, 0XEB);
		oper.AddItemValue(KEY4C("Z4"), 3, 0XEB, 0X2F);
		oper.AddItemValue(KEY4C("Z5"), 3, 0XEB, 0X33);
		oper.AddItemValue(KEY4C("K1"), 1, 0X2F, 0X2D);
		oper.AddItemValue(KEY4C("K2"), 1, 0X29, 0X2B);
		oper.AddItemValue(KEY4C("K3"), 1, 0X33, 0X31);

		oper.AddItemValue(KEY4C("XA1"), 1, 0XC5);
		oper.AddItemValue(KEY4C("XA2"), 1, 0X1FC0);
		oper.AddItemValue(KEY4C("XA3"), 1, 0X1FC0, 0XCF);
		oper.AddItemValue(KEY4C("XA4"), 1, 0XD1, 0XD3);
		oper.AddItemValue(KEY4C("XA5"), 1, 0XFB, 0XF7);
		oper.AddItemValue(KEY4C("XA6"), 1, 0XC5, 0XC7);
		oper.AddItemValue(KEY4C("YA1"), 2, 0XCF, 0XD0);
		oper.AddItemValue(KEY4C("YA2"), 2, 0XC5, 0XC6);
		oper.AddItemValue(KEY4C("ZA1"), 3, 0XC5, 0XCF);
		oper.AddItemValue(KEY4C("ZA2"), 3, 0XCF, 0XFB);
		//右侧复合横担TX3
		oper.AddItemValue(KEY4C("XB1"), 1, 0XC1);
		oper.AddItemValue(KEY4C("XB2"), 1, 0X1FC2);
		oper.AddItemValue(KEY4C("XB3"), 1, 0XCB, 0XC9);
		oper.AddItemValue(KEY4C("XB4"), 1, 0XF3, 0XF5);
		oper.AddItemValue(KEY4C("XB5"), 1, 0XC3, 0XC1);
		oper.AddItemValue(KEY4C("YB1"), 2, 0XC9, 0XCA);
		oper.AddItemValue(KEY4C("YB2"), 2, 0XC1, 0XC2);
		oper.AddItemValue(KEY4C("ZB1"), 3, 0XC1, 0XC9);
		oper.AddItemValue(KEY4C("ZB2"), 3, 0XC9, 0XF3);
		//第二层						
		oper.AddItemValue(KEY4C("ZCD"), 3, 0X43, 0XDD);
		//左侧横担HX14
		oper.AddItemValue(KEY4C("XC1"), 1, 0X1FCA);
		oper.AddItemValue(KEY4C("XC2"), 1, 0X1FCA, 0XDD);
		oper.AddItemValue(KEY4C("XC3"), 1, 0XDF, 0XE1);
		oper.AddItemValue(KEY4C("XC4"), 1, 0X105, 0XFF);
		oper.AddItemValue(KEY4C("YC"), 2, 0XDD, 0XDE);
		oper.AddItemValue(KEY4C("ZC2"), 3, 0XDD, 0X105);
		//右侧横担HX13
		oper.AddItemValue(KEY4C("XD1"), 1, 0X1FC4);
		oper.AddItemValue(KEY4C("XD2"), 1, 0XD9, 0XD7);
		oper.AddItemValue(KEY4C("XD3"), 1, 0XFD, 0X101);
		oper.AddItemValue(KEY4C("YD"), 2, 0XD7, 0XD8);
		oper.AddItemValue(KEY4C("ZD2"), 3, 0XD7, 0X101);
		//第三层
		oper.AddItemValue(KEY4C("ZEF"), 3, 0X4F, 0XEB);
		//左侧横担HX14
		oper.AddItemValue(KEY4C("XE1"), 1, 0X1FC8);
		oper.AddItemValue(KEY4C("XE2"), 1, 0X1FC8, 0XEB);
		oper.AddItemValue(KEY4C("XE3"), 1, 0XED, 0XEF);
		oper.AddItemValue(KEY4C("XE4"), 1, 0X10F, 0X109);
		oper.AddItemValue(KEY4C("YE"), 2, 0XEB, 0XEC);
		oper.AddItemValue(KEY4C("ZE2"), 3, 0XEB, 0X10F);
		//右侧横担HX13
		oper.AddItemValue(KEY4C("XF1"), 1, 0X1FC6);
		oper.AddItemValue(KEY4C("XF2"), 1, 0XE5, 0XE7);
		oper.AddItemValue(KEY4C("XF3"), 1, 0X107, 0X10B);
		oper.AddItemValue(KEY4C("YF"), 2, 0XE5, 0XE6);
		oper.AddItemValue(KEY4C("ZF2"), 3, 0XE5, 0X10B);
	}
	else if (ciTowerType == TYPE_ZC)
	{
		oper.AddItemValue(KEY4C("Z1"), 3, 0X30, 0X58);
		oper.AddItemValue(KEY4C("Z2"), 3, 0X58, 0X34);
		oper.AddItemValue(KEY4C("Z3"), 3, 0X34, 0X38);
		oper.AddItemValue(KEY4C("Z4"), 3, 0X58, 0X3C);
		oper.AddItemValue(KEY4C("K1"), 1, 0X38, 0X36);
		oper.AddItemValue(KEY4C("K2"), 1, 0X34, 0X32);
		oper.AddItemValue(KEY4C("K3"), 1, 0X3C, 0X3A);
		//右侧复合横担TX12			
		oper.AddItemValue(KEY4C("XA1"), 1, 0X40);
		oper.AddItemValue(KEY4C("XA2"), 1, 0X40, 0X48);
		oper.AddItemValue(KEY4C("XA3"), 1, 0X58);
		oper.AddItemValue(KEY4C("XA4"), 1, 0X58, 0X78);
		oper.AddItemValue(KEY4C("XA5"), 1, 0X58, 0X7C);
		oper.AddItemValue(KEY4C("XA6"), 1, 0X64, 0X68);
		oper.AddItemValue(KEY4C("YA1"), 2, 0X40, 0X41);
		oper.AddItemValue(KEY4C("YA2"), 2, 0X58, 0X59);
		oper.AddItemValue(KEY4C("ZA1"), 3, 0X40, 0X58);
		oper.AddItemValue(KEY4C("ZA2"), 3, 0X40, 0X50);
		oper.AddItemValue(KEY4C("ZA3"), 3, 0X58, 0X68);
		oper.AddItemValue(KEY4C("ZA4"), 3, 0X58, 0X80);
	}
	else if (ciTowerType == TYPE_ZBC)
	{	//ZBC
		oper.AddItemValue(KEY4C("Z"), 3, 0X68, 0X4E);
		oper.AddItemValue(KEY4C("K1"), 1, 0X62, 0X60);
		oper.AddItemValue(KEY4C("K2"), 1, 0X68, 0X66);
		//酒杯塔头
		oper.AddItemValue(KEY4C("XA1"), 1, 0X2E);
		oper.AddItemValue(KEY4C("XA2"), 1, 0X36, 0X2E);
		oper.AddItemValue(KEY4C("XA3"), 1, 0X4A);
		oper.AddItemValue(KEY4C("XA4"), 1, 0X46, 0X4A);
		oper.AddItemValue(KEY4C("XA5"), 1, 0X46, 0X4E);
		oper.AddItemValue(KEY4C("XA6"), 1, 0X5E);
		oper.AddItemValue(KEY4C("YA1"), 2, 0X2C, 0X2D);
		oper.AddItemValue(KEY4C("YA2"), 2, 0X3C, 0X3D);
		oper.AddItemValue(KEY4C("YA3"), 2, 0X4C, 0X4D);
		oper.AddItemValue(KEY4C("ZA1"), 3, 0X62, 0X5E);
		oper.AddItemValue(KEY4C("ZA2"), 3, 0X5E, 0X4E);
		oper.AddItemValue(KEY4C("ZA3"), 3, 0X4E, 0X2E);
		oper.AddItemValue(KEY4C("ZA4"), 3, 0X50, 0X52);
		oper.AddItemValue(KEY4C("ZA5"), 3, 0X3E, 0X4E);
		oper.AddItemValue(KEY4C("ZA6"), 3, 0X3A, 0X2E);
	}
	else if (ciTowerType == TYPE_ZM)
	{	//ZM
		oper.AddItemValue(KEY4C("Z"), 3, 0X55, 0XBF);
		oper.AddItemValue(KEY4C("K1"), 1, 0X6B, 0X69);
		oper.AddItemValue(KEY4C("K2"), 1, 0XBF, 0XBD);
		//猫头塔头
		oper.AddItemValue(KEY4C("X1"), 1, 0X55);
		oper.AddItemValue(KEY4C("X2"), 1, 0X65);
		oper.AddItemValue(KEY4C("X3"), 1, 0X65, 0X59);
		oper.AddItemValue(KEY4C("X4"), 1, 0X49);
		oper.AddItemValue(KEY4C("X5"), 1, 0X49, 0X4D);
		oper.AddItemValue(KEY4C("X6"), 1, 0X3D);
		oper.AddItemValue(KEY4C("X7"), 1, 0X31);
		oper.AddItemValue(KEY4C("X8"), 1, 0X2D, 0X31);
		oper.AddItemValue(KEY4C("Y1"), 2, 0X53, 0X54);
		oper.AddItemValue(KEY4C("Y2"), 2, 0X3B, 0X3C);
		oper.AddItemValue(KEY4C("Y3"), 2, 0X2F, 0X30);
		oper.AddItemValue(KEY4C("Z1"), 3, 0X6B, 0X55);
		oper.AddItemValue(KEY4C("Z2"), 3, 0X55, 0X51);
		oper.AddItemValue(KEY4C("Z3"), 3, 0X51, 0X4D);
		oper.AddItemValue(KEY4C("Z4"), 3, 0X4D, 0X31);
		oper.AddItemValue(KEY4C("Z5"), 3, 0X6B, 0X67);
		oper.AddItemValue(KEY4C("Z6"), 3, 0X55, 0X59);
		oper.AddItemValue(KEY4C("Z7"), 3, 0X3D, 0X4D);
		oper.AddItemValue(KEY4C("Z8"), 3, 0X43, 0X45);
		oper.AddItemValue(KEY4C("Z9"), 3, 0X39, 0X31);
	}
	else if (ciTowerType == TYPE_SZ)
	{
		oper.AddItemValue(KEY4C("Z1"), 3, 0X5B, 0X47);
		oper.AddItemValue(KEY4C("Z2"), 3, 0X47, 0X8F);
		oper.AddItemValue(KEY4C("Z3"), 3, 0X8F, 0X9B);
		oper.AddItemValue(KEY4C("Z4"), 3, 0X9B, 0X5F);
		oper.AddItemValue(KEY4C("Z5"), 3, 0X9B, 0X157);
		oper.AddItemValue(KEY4C("K1"), 1, 0X5D, 0X5F);
		oper.AddItemValue(KEY4C("K2"), 1, 0X59, 0X5B);
		oper.AddItemValue(KEY4C("K3"), 1, 0X155, 0X157);
		//第一层导地线复合横担TX11
		oper.AddItemValue(KEY4C("XA1"), 1, 0X2D);
		oper.AddItemValue(KEY4C("XA2"), 1, 0X2D, 0X35);
		oper.AddItemValue(KEY4C("XA3"), 1, 0X45);
		oper.AddItemValue(KEY4C("XA4"), 1, 0X4D);
		oper.AddItemValue(KEY4C("XA5"), 1, 0X3D, 0X41);
		oper.AddItemValue(KEY4C("YA1"), 2, 0X2D, 0X2E);
		oper.AddItemValue(KEY4C("YA2"), 2, 0X45, 0X46);
		oper.AddItemValue(KEY4C("ZA1"), 3, 0X45, 0X31);
		oper.AddItemValue(KEY4C("ZA2"), 3, 0X2D, 0X31);
		oper.AddItemValue(KEY4C("ZA3"), 3, 0X3D, 0X4D);
		//第二层导线横担HX2
		oper.AddItemValue(KEY4C("XB"), 1, 0X8D);
		oper.AddItemValue(KEY4C("YB"), 2, 0X8D, 0X8E);
		oper.AddItemValue(KEY4C("ZB"), 3, 0X75, 0X8D);
		//第三层导线横担HX2
		oper.AddItemValue(KEY4C("XC"), 1, 0X99);
		oper.AddItemValue(KEY4C("YC"), 2, 0X99, 0X9A);
		oper.AddItemValue(KEY4C("ZC"), 3, 0X85, 0X99);
	}
	else if (ciTowerType == TYPE_JC)
	{
		oper.AddItemValue(KEY4C("Z1"), 3, 0X56, 0X6E);
		oper.AddItemValue(KEY4C("Z2"), 3, 0X6E, 0X32);
		oper.AddItemValue(KEY4C("Z3"), 3, 0X6E, 0XE2);
		oper.AddItemValue(KEY4C("K1"), 1, 0X30, 0X32);
		oper.AddItemValue(KEY4C("K2"), 1, 0X2C, 0X2E);
		oper.AddItemValue(KEY4C("K3"), 1, 0XE0, 0XE2);
		//
		oper.AddItemValue(KEY4C("ZAB"), 3, 0X2E, 0X3E);
		//左侧地线支架
		oper.AddItemValue(KEY4C("XA"), 1, 0X56);
		oper.AddItemValue(KEY4C("YA"), 2, 0X56, 0X57);
		//右侧地线支架
		oper.AddItemValue(KEY4C("XB"), 1, 0X54);
		oper.AddItemValue(KEY4C("YB"), 2, 0X54, 0X55);
		//
		oper.AddItemValue(KEY4C("ZCD"), 3, 0X4A, 0X4E);
		//左侧横担
		oper.AddItemValue(KEY4C("XC1"), 1, 0X1AA);
		oper.AddItemValue(KEY4C("XC2"), 1, 0X6E, 0X1AA);
		oper.AddItemValue(KEY4C("XC3"), 1, 0X5E, 0X5A);
		oper.AddItemValue(KEY4C("XC4"), 1, 0X6A, 0X6E);
		oper.AddItemValue(KEY4C("YC"), 2, 0X6E, 0X6F);
		oper.AddItemValue(KEY4C("ZC"), 3, 0X66, 0X5E);
		//右侧横担
		oper.AddItemValue(KEY4C("XD1"), 1, 0X1A8);
		oper.AddItemValue(KEY4C("XD2"), 1, 0X1A8, 0X6C);
		oper.AddItemValue(KEY4C("XD3"), 1, 0X5C, 0X58);
		oper.AddItemValue(KEY4C("XD4"), 1, 0X68, 0X6C);
		oper.AddItemValue(KEY4C("YD"), 2, 0X6C, 0X6D);
		oper.AddItemValue(KEY4C("ZD"), 3, 0X64, 0X5C);
	}
	else if (ciTowerType == TYPE_JBB)
	{
		oper.AddItemValue(KEY4C("Z"), 3, 0X6B, 0X7B);
		oper.AddItemValue(KEY4C("K1"), 1, 0X73, 0X71);
		oper.AddItemValue(KEY4C("K2"), 1, 0X7B, 0X79);
		//酒杯塔头
		oper.AddItemValue(KEY4C("X1"), 1, 0X7A3);
		oper.AddItemValue(KEY4C("X2"), 1, 0X7A5);
		oper.AddItemValue(KEY4C("X3"), 1, 0X7AD);
		oper.AddItemValue(KEY4C("X4"), 1, 0X4D);
		oper.AddItemValue(KEY4C("X5"), 1, 0X4D, 0X3D);
		oper.AddItemValue(KEY4C("X6"), 1, 0X3D, 0X7A7);
		oper.AddItemValue(KEY4C("X7"), 1, 0X7A7, 0X6B);
		oper.AddItemValue(KEY4C("X8"), 1, 0X3B, 0X7A9);
		oper.AddItemValue(KEY4C("X9"), 1, 0X7A9, 0X5F);
		oper.AddItemValue(KEY4C("W1"), 1, 0X2F, 0X31);
		oper.AddItemValue(KEY4C("W2"), 1, 0X2B, 0X2D);
		oper.AddItemValue(KEY4C("W3"), 1, 0X57, 0X59);
		oper.AddItemValue(KEY4C("W4"), 1, 0X6D, 0X6F);
		oper.AddItemValue(KEY4C("W5"), 1, 0X61, 0X63);

		oper.AddItemValue(KEY4C("Y1"), 2, 0X2F, 0X30);
		oper.AddItemValue(KEY4C("Y2"), 2, 0X35, 0X36);
		oper.AddItemValue(KEY4C("Y3"), 2, 0X6B, 0X6C);

		oper.AddItemValue(KEY4C("Z1"), 3, 0X73, 0X77);
		oper.AddItemValue(KEY4C("Z2"), 3, 0X77, 0X6B);
		oper.AddItemValue(KEY4C("Z3"), 3, 0X6B, 0X2F);
		oper.AddItemValue(KEY4C("Z4"), 3, 0X3D, 0X35);
		oper.AddItemValue(KEY4C("Z5"), 3, 0X6B, 0X5D);
	}
}
bool CTowerTemplateToModel::UpdateTowerSlopeDesPara(BYTE ciTowerType, CTower* pTower)
{
	if (pTower == NULL)
		return false;
	double fK2 = 0, fK3 = 0, fA = 0, fD1 = 0, fD2 = 0, fD3 = 0;
	DESIGN_PARAM_ITEM* pParam = NULL;
	if (ciTowerType == TYPE_SZC)
	{
		pParam = pTower->hashParams.GetValue(KEY4C("Z"));
		double fZ = pParam ? pParam->value.fValue : 0;
		pParam = pTower->hashParams.GetValue(KEY4C("Z2"));
		double fZ2 = pParam ? pParam->value.fValue : 0;
		pParam = pTower->hashParams.GetValue(KEY4C("Z3"));
		double fZ3 = pParam ? pParam->value.fValue : 0;
		pParam = pTower->hashParams.GetValue(KEY4C("Z4"));
		double fZ4 = pParam ? pParam->value.fValue : 0;
		pParam = pTower->hashParams.GetValue(KEY4C("Z5"));
		double fZ5 = pParam ? pParam->value.fValue : 0;
		pParam = pTower->hashParams.GetValue(KEY4C("K1"));
		double fK1 = pParam ? pParam->value.fValue : 0;
		pParam = pTower->hashParams.GetValue(KEY4C("P1"));
		double fP1 = pParam ? pParam->value.fValue : 0;
		pParam = pTower->hashParams.GetValue(KEY4C("P2"));
		double fP2 = pParam ? pParam->value.fValue : 0;
		fK2 = fK1 - 2 * (fZ2 + fZ3 + fZ4 - fZ)*fP1;
		fK3 = fK1 + 2 * (fZ5 - fZ4)*fP2;

		pParam = pTower->hashParams.GetValue(KEY4C("L"));
		double fL = pParam ? pParam->value.fValue : 0;
		pParam = pTower->hashParams.GetValue(KEY4C("Z1"));
		double fZ1 = pParam ? pParam->value.fValue : 0;
		pParam = pTower->hashParams.GetValue(KEY4C("XA1"));
		double fXA1 = pParam ? pParam->value.fValue : 0;
		pParam = pTower->hashParams.GetValue(KEY4C("XA2"));
		double fXA2 = pParam ? pParam->value.fValue : 0;
		pParam = pTower->hashParams.GetValue(KEY4C("XB"));
		double fXB = pParam ? pParam->value.fValue : 0;
		pParam = pTower->hashParams.GetValue(KEY4C("XC"));
		double fXC = pParam ? pParam->value.fValue : 0;
		fD1 = abs(fXA1 - fXA2);
		fD2 = abs(fXA2 - fXB);
		fD3 = abs(fXB - fXC);
		fA = atan(fD1 / (fZ1 + fL))*DEGTORAD_COEF;//角度
		pParam = pTower->hashParams.GetValue(KEY4C("A"));
		pParam->value.fValue = fA;
	}
	else if (ciTowerType == TYPE_JG || ciTowerType == TYPE_JG2)
	{
		pParam = pTower->hashParams.GetValue(KEY4C("Z1"));
		double fZ1 = pParam ? pParam->value.fValue : 0;
		pParam = pTower->hashParams.GetValue(KEY4C("Z2"));
		double fZ2 = pParam ? pParam->value.fValue : 0;
		pParam = pTower->hashParams.GetValue(KEY4C("Z3"));
		double fZ3 = pParam ? pParam->value.fValue : 0;
		pParam = pTower->hashParams.GetValue(KEY4C("Z4"));
		double fZ4 = pParam ? pParam->value.fValue : 0;
		pParam = pTower->hashParams.GetValue(KEY4C("K1"));
		double fK1 = pParam ? pParam->value.fValue : 0;
		pParam = pTower->hashParams.GetValue(KEY4C("P1"));
		double fP1 = pParam ? pParam->value.fValue : 0;
		pParam = pTower->hashParams.GetValue(KEY4C("P2"));
		double fP2 = pParam ? pParam->value.fValue : 0;
		fK2 = fK1 - 2 * (fZ1 + fZ2 + fZ3)*fP1;
		fK3 = fK1 + 2 * (fZ4 - fZ3)*fP2;
	}
	else if (ciTowerType == TYPE_SJ || ciTowerType == TYPE_SJB || ciTowerType == TYPE_SJC || ciTowerType == TYPE_SZ)
	{
		pParam = pTower->hashParams.GetValue(KEY4C("Z1"));
		double fZ1 = pParam ? pParam->value.fValue : 0;
		pParam = pTower->hashParams.GetValue(KEY4C("Z2"));
		double fZ2 = pParam ? pParam->value.fValue : 0;
		pParam = pTower->hashParams.GetValue(KEY4C("Z3"));
		double fZ3 = pParam ? pParam->value.fValue : 0;
		pParam = pTower->hashParams.GetValue(KEY4C("Z4"));
		double fZ4 = pParam ? pParam->value.fValue : 0;
		pParam = pTower->hashParams.GetValue(KEY4C("Z5"));
		double fZ5 = pParam ? pParam->value.fValue : 0;
		pParam = pTower->hashParams.GetValue(KEY4C("K1"));
		double fK1 = pParam ? pParam->value.fValue : 0;
		pParam = pTower->hashParams.GetValue(KEY4C("P1"));
		double fP1 = pParam ? pParam->value.fValue : 0;
		pParam = pTower->hashParams.GetValue(KEY4C("P2"));
		double fP2 = pParam ? pParam->value.fValue : 0;
		fK2 = fK1 - 2 * (fZ1 + fZ2 + fZ3 + fZ4)*fP1;
		fK3 = fK1 + 2 * (fZ5 - fZ4)*fP2;
	}
	else if (ciTowerType == TYPE_ZC)
	{
		pParam = pTower->hashParams.GetValue(KEY4C("Z1"));
		double fZ1 = pParam ? pParam->value.fValue : 0;
		pParam = pTower->hashParams.GetValue(KEY4C("Z2"));
		double fZ2 = pParam ? pParam->value.fValue : 0;
		pParam = pTower->hashParams.GetValue(KEY4C("Z3"));
		double fZ3 = pParam ? pParam->value.fValue : 0;
		pParam = pTower->hashParams.GetValue(KEY4C("Z4"));
		double fZ4 = pParam ? pParam->value.fValue : 0;
		pParam = pTower->hashParams.GetValue(KEY4C("K1"));
		double fK1 = pParam ? pParam->value.fValue : 0;
		pParam = pTower->hashParams.GetValue(KEY4C("P1"));
		double fP1 = pParam ? pParam->value.fValue : 0;
		pParam = pTower->hashParams.GetValue(KEY4C("P2"));
		double fP2 = pParam ? pParam->value.fValue : 0;
		fK2 = fK1 - 2 * fZ3*fP1;
		fK3 = fK1 + 2 * (fZ4 - fZ3 - fZ2)*fP2;
	}
	else if (ciTowerType == TYPE_ZBC)
	{
		pParam = pTower->hashParams.GetValue(KEY4C("Z"));
		double fZ = pParam ? pParam->value.fValue : 0;
		pParam = pTower->hashParams.GetValue(KEY4C("ZA1"));
		double fZ1 = pParam ? pParam->value.fValue : 0;
		pParam = pTower->hashParams.GetValue(KEY4C("ZA2"));
		double fZ2 = pParam ? pParam->value.fValue : 0;
		pParam = pTower->hashParams.GetValue(KEY4C("K1"));
		double fK1 = pParam ? pParam->value.fValue : 0;
		pParam = pTower->hashParams.GetValue(KEY4C("P"));
		double fP = pParam ? pParam->value.fValue : 0;
		fK2 = fK1 + 2 * (fZ - fZ1 - fZ2)*fP;
	}
	else if (ciTowerType == TYPE_JBB)
	{
		pParam = pTower->hashParams.GetValue(KEY4C("Z"));
		double fZ = pParam ? pParam->value.fValue : 0;
		pParam = pTower->hashParams.GetValue(KEY4C("Z1"));
		double fZ1 = pParam ? pParam->value.fValue : 0;
		pParam = pTower->hashParams.GetValue(KEY4C("Z2"));
		double fZ2 = pParam ? pParam->value.fValue : 0;
		pParam = pTower->hashParams.GetValue(KEY4C("K1"));
		double fK1 = pParam ? pParam->value.fValue : 0;
		pParam = pTower->hashParams.GetValue(KEY4C("P"));
		double fP = pParam ? pParam->value.fValue : 0;
		fK2 = fK1 + 2 * (fZ - fZ1 - fZ2)*fP;
	}
	else if (ciTowerType == TYPE_ZM)
	{
		pParam = pTower->hashParams.GetValue(KEY4C("Z"));
		double fZ = pParam ? pParam->value.fValue : 0;
		pParam = pTower->hashParams.GetValue(KEY4C("Z1"));
		double fZ1 = pParam ? pParam->value.fValue : 0;
		pParam = pTower->hashParams.GetValue(KEY4C("K1"));
		double fK1 = pParam ? pParam->value.fValue : 0;
		pParam = pTower->hashParams.GetValue(KEY4C("P"));
		double fP = pParam ? pParam->value.fValue : 0;
		fK2 = fK1 + 2 * (fZ - fZ1)*fP;
	}
	else if (ciTowerType == TYPE_JC)
	{
		pParam = pTower->hashParams.GetValue(KEY4C("Z1"));
		double fZ1 = pParam ? pParam->value.fValue : 0;
		pParam = pTower->hashParams.GetValue(KEY4C("Z2"));
		double fZ2 = pParam ? pParam->value.fValue : 0;
		pParam = pTower->hashParams.GetValue(KEY4C("Z3"));
		double fZ3 = pParam ? pParam->value.fValue : 0;
		pParam = pTower->hashParams.GetValue(KEY4C("K1"));
		double fK1 = pParam ? pParam->value.fValue : 0;
		pParam = pTower->hashParams.GetValue(KEY4C("P1"));
		double fP1 = pParam ? pParam->value.fValue : 0;
		pParam = pTower->hashParams.GetValue(KEY4C("P2"));
		double fP2 = pParam ? pParam->value.fValue : 0;
		fK2 = fK1 - 2 * (fZ1 + fZ2)*fP1;
		fK3 = fK1 + 2 * (fZ3 - fZ2)*fP2;
	}
	//变坡开口宽度
	pParam = pTower->hashParams.GetValue(KEY4C("K2"));
	if (pParam && fK2 > 0)
		pParam->value.fValue = fK2;
	else
		return false;
	pParam = pTower->hashParams.GetValue(KEY4C("K3"));
	if (pParam && fK2 > 0)
		pParam->value.fValue = fK3;
	else
		return false;
	return true;
}