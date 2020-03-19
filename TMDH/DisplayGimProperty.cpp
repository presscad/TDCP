#include "stdafx.h"
#include "TMDH.h"
#include "PropertyListOper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BOOL ModifyProperty(CPropertyList *pPropList,CPropTreeItem* pItem, CString &valueStr)
{
	CTMDHGim* pGim=(CTMDHGim*)pPropList->m_pObj;
	if(pGim==NULL)
		return FALSE;
	CPropertyListOper<CTMDHGim> oper(pPropList,pGim);
	BOOL bUpdateWireInfo=FALSE;
	if(CTMDHGim::GetPropID("Designer")==pItem->m_idProp)
		strcpy(pGim->m_xGimHeadProp.m_sDesigner,valueStr);
	else if(CTMDHGim::GetPropID("Unit")==pItem->m_idProp)
		strcpy(pGim->m_xGimHeadProp.m_sUnit,valueStr);
	else if(CTMDHGim::GetPropID("MajorVer")==pItem->m_idProp)
		strcpy(pGim->m_xGimHeadProp.m_sMajorVersion,valueStr);
	else if(CTMDHGim::GetPropID("MinorVer")==pItem->m_idProp)
		strcpy(pGim->m_xGimHeadProp.m_sMinorVersion,valueStr);
	else if(CTMDHGim::GetPropID("VoltGrade")==pItem->m_idProp)
		strcpy(pGim->m_xTowerProp.m_sVoltGrade,valueStr);
	else if(CTMDHGim::GetPropID("Type")==pItem->m_idProp)
		strcpy(pGim->m_xTowerProp.m_sType,valueStr);
	else if(CTMDHGim::GetPropID("Texture")==pItem->m_idProp)
		strcpy(pGim->m_xTowerProp.m_sTexture,valueStr);
	else if(CTMDHGim::GetPropID("FixedType")==pItem->m_idProp)
		strcpy(pGim->m_xTowerProp.m_sFixedType,valueStr);
	else if(CTMDHGim::GetPropID("TaType")==pItem->m_idProp)
		strcpy(pGim->m_xTowerProp.m_sTaType,valueStr);
	else if(CTMDHGim::GetPropID("Circuit")==pItem->m_idProp)
		pGim->m_xTowerProp.m_nCircuit=atoi(valueStr);
	else if(CTMDHGim::GetPropID("CWireSpec")==pItem->m_idProp)
		strcpy(pGim->m_xTowerProp.m_sCWireSpec,valueStr);
	else if(CTMDHGim::GetPropID("EWireSpec")==pItem->m_idProp)
		strcpy(pGim->m_xTowerProp.m_sEWireSpec,valueStr);
	else if(CTMDHGim::GetPropID("WindSpeed")==pItem->m_idProp)
		pGim->m_xTowerProp.m_fWindSpeed=atof(valueStr);
	else if(CTMDHGim::GetPropID("NiceThick")==pItem->m_idProp)
		pGim->m_xTowerProp.m_fNiceThick=atof(valueStr);
	else if(CTMDHGim::GetPropID("WindSpan")==pItem->m_idProp)
		strcpy(pGim->m_xTowerProp.m_sWindSpan,valueStr);
	else if(CTMDHGim::GetPropID("WeightSpan")==pItem->m_idProp)
		strcpy(pGim->m_xTowerProp.m_sWeightSpan,valueStr);
	else if(CTMDHGim::GetPropID("FrontRulingSpan")==pItem->m_idProp)
		pGim->m_xTowerProp.m_fFrontRulingSpan=atof(valueStr);
	else if(CTMDHGim::GetPropID("BackRulingSpan")==pItem->m_idProp)
		pGim->m_xTowerProp.m_fBackRulingSpan=atof(valueStr);
	else if(CTMDHGim::GetPropID("MaxSpan")==pItem->m_idProp)
		pGim->m_xTowerProp.m_fMaxSpan=atof(valueStr);
	else if(CTMDHGim::GetPropID("AngleRange")==pItem->m_idProp)
		strcpy(pGim->m_xTowerProp.m_sAngleRange,valueStr);
	else if(CTMDHGim::GetPropID("DesignKV")==pItem->m_idProp)
		pGim->m_xTowerProp.m_fDesignKV=atof(valueStr);
	else if(CTMDHGim::GetPropID("RatedHeight")==pItem->m_idProp)
		strcpy(pGim->m_xTowerProp.m_sRatedHeight,valueStr);
	else if(CTMDHGim::GetPropID("HeightRange")==pItem->m_idProp)
		strcpy(pGim->m_xTowerProp.m_sHeightRange,valueStr);
	else if(CTMDHGim::GetPropID("TowerWeight")==pItem->m_idProp)
		strcpy(pGim->m_xTowerProp.m_sTowerWeight,valueStr);
	else if(CTMDHGim::GetPropID("FrequencyRockAngle")==pItem->m_idProp)
		pGim->m_xTowerProp.m_fFrequencyRockAngle=atof(valueStr);
	else if(CTMDHGim::GetPropID("LightningRockAngle")==pItem->m_idProp)
		pGim->m_xTowerProp.m_fLightningRockAngle=atof(valueStr);
	else if(CTMDHGim::GetPropID("SwitchingRockAngle")==pItem->m_idProp)
		pGim->m_xTowerProp.m_fSwitchingRockAngle=atof(valueStr);
	else if(CTMDHGim::GetPropID("WorkingRockAngle")==pItem->m_idProp)
		pGim->m_xTowerProp.m_fWorkingRockAngle=atof(valueStr);
	else if(CTMDHGim::GetPropID("ManuFacturer")==pItem->m_idProp)
		strcpy(pGim->m_xTowerProp.m_sManuFacturer,valueStr);
	else if(CTMDHGim::GetPropID("MaterialCode")==pItem->m_idProp)
		strcpy(pGim->m_xTowerProp.m_sMaterialCode,valueStr);
	else
		return FALSE;
	return TRUE;
}
//
void CMainFrame::DisplayGimProperty(CTMDHGim* pGim)
{
	CPropertyDlg *pPropDlg=GetPropPage();
	if(pPropDlg==NULL)
		return;
	CPropertyList *pPropList=pPropDlg->GetPropertyList();
	pPropList->CleanCallBackFunc();
	pPropList->CleanTree();
	pPropList->m_nObjClassTypeId = 0;
	pPropList->m_pObj=pGim;
	pPropList->SetModifyValueFunc(ModifyProperty);
	//
	CPropertyListOper<CTMDHGim> oper(pPropList,pGim);
	CPropTreeItem* pRoot=pPropList->GetRootItem();
	CPropTreeItem *pGroupItem=NULL,*pPropItem=NULL;
	//基本参数	
	pGroupItem=oper.InsertPropItem(pRoot,"BasicInfo");
	oper.InsertEditPropItem(pGroupItem,"Designer");
	oper.InsertEditPropItem(pGroupItem,"Unit");
	oper.InsertEditPropItem(pGroupItem,"MajorVer");
	oper.InsertEditPropItem(pGroupItem,"MinorVer");
	//挂线参数
	pGroupItem=oper.InsertPropItem(pRoot,"TowerInfo");
	oper.InsertEditPropItem(pGroupItem,"VoltGrade");
	oper.InsertEditPropItem(pGroupItem,"Type");
	oper.InsertCmbListPropItem(pGroupItem,"Texture");
	oper.InsertCmbListPropItem(pGroupItem,"FixedType");
	oper.InsertCmbListPropItem(pGroupItem,"TaType");
	oper.InsertEditPropItem(pGroupItem,"Circuit");
	oper.InsertEditPropItem(pGroupItem,"CWireSpec");
	oper.InsertEditPropItem(pGroupItem,"EWireSpec");
	oper.InsertEditPropItem(pGroupItem,"WindSpeed");
	oper.InsertEditPropItem(pGroupItem,"NiceThick");
	oper.InsertEditPropItem(pGroupItem,"WindSpan");
	oper.InsertEditPropItem(pGroupItem,"WeightSpan");
	oper.InsertEditPropItem(pGroupItem,"FrontRulingSpan");
	oper.InsertEditPropItem(pGroupItem,"BackRulingSpan");
	oper.InsertEditPropItem(pGroupItem,"MaxSpan");
	oper.InsertEditPropItem(pGroupItem,"AngleRange");
	oper.InsertEditPropItem(pGroupItem,"DesignKV");
	oper.InsertEditPropItem(pGroupItem,"RatedHeight");
	oper.InsertEditPropItem(pGroupItem,"HeightRange");
	oper.InsertEditPropItem(pGroupItem,"TowerWeight");
	oper.InsertEditPropItem(pGroupItem,"FrequencyRockAngle");
	oper.InsertEditPropItem(pGroupItem,"LightningRockAngle");
	oper.InsertEditPropItem(pGroupItem,"SwitchingRockAngle");
	oper.InsertEditPropItem(pGroupItem,"WorkingRockAngle");
	oper.InsertEditPropItem(pGroupItem,"ManuFacturer");
	oper.InsertEditPropItem(pGroupItem,"MaterialCode");
	//
	pPropList->Redraw();
}