#include "stdafx.h"
#include "TPSM.h"
#include "MainFrm.h"
#include "PropertyListOper.h"
#include "GlobalFunc.h"
#include "TowerTypeTemp.h"
#include "btc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
//////////////////////////////////////////////////////////////////////////
//回调函数处理
void UpdateInstanceProperty(CPropertyList* pPropList,CPropTreeItem* pParentItem);
void DrawParaSketch(PARA_SKETCH_ITEM* pSchemaItem,const char* sLabel)
{
	CTPSMView* pView=theApp.GetActiveView();
	if(pView==NULL)
		return;
	pView->SolidDraw()->DelWorkPlaneSketch(1);
	pView->SolidDraw()->DelWorkPlaneSketch(2);
	pView->SolidDraw()->Draw();
	CTPSMModel*	pModel=Manager.m_pActivePrj->m_pActiveModel;
	if(pModel==NULL||pSchemaItem==NULL)
		return;
	//将当前节点位置放大显示
	GEPOINT pt1,pt2;
	SCOPE_STRU scope;
	CLDSNode* pNode1=pModel->m_xTower.FromNodeHandle(pSchemaItem->m_hNode1);
	if(pNode1)
	{
		pt1=pNode1->xActivePos;
		scope.VerifyVertex(pNode1->xActivePos);
		CLDSNode* pMirXNode1=pNode1->GetMirNode(MIRMSG(1));
		if(pMirXNode1)
			scope.VerifyVertex(pMirXNode1->xActivePos);
	}
	CLDSNode* pNode2=pModel->m_xTower.FromNodeHandle(pSchemaItem->m_hNode2);
	if(pNode2)
	{
		pt2=pNode2->xActivePos;
		scope.VerifyVertex(pNode2->xActivePos);
		CLDSNode* pMirXNode2=pNode2->GetMirNode(MIRMSG(1));
		if(pMirXNode2)
			scope.VerifyVertex(pMirXNode2->xActivePos);
	}
	scope.fMinX-=600;
	scope.fMaxX+=600;
	scope.fMinY-=600;
	scope.fMaxY+=600;
	scope.fMinZ-=600;
	scope.fMaxZ+=600;
	double fFocusZoomCoef=1.0/pView->SolidOper()->GetScaleUserToScreen();
	pView->SolidOper()->FocusTo(scope,fFocusZoomCoef);
	pView->SolidDraw()->Draw();
	//
	if(!pt1.IsZero())
	{
		COLORREF clr=RGB(152,152,152);
		if(pSchemaItem->m_ciSketchVec==1)
		{	//X方向设计参数
			double fZ=(fabs(pt1.z)>fabs(pt2.z))?pt1.z:pt2.z;
			double fX1=(pt1.x>pt2.x)?pt1.x:pt2.x;
			double fX2=(pt1.x>pt2.x)?pt2.x:pt1.x;
			double fEdgeLen=fabs(pt1.x-pt2.x);
			btc::SKETCH_PLANE sketch1;
			sketch1.CreateStdPlane(GEPOINT(fX1,0,fZ),GEPOINT(1,0,0),GEPOINT(1,0,0),(int)fEdgeLen);
			pView->SolidDraw()->NewWorkPlaneSketch(1,clr,sketch1.pVertexArr,sketch1.VertexCount,sketch1.normal,sLabel,50,false,100);
			btc::SKETCH_PLANE sketch2;
			sketch2.CreateStdPlane(GEPOINT(fX2,0,fZ),GEPOINT(-1,0,0),GEPOINT(1,0,0),(int)fEdgeLen);
			pView->SolidDraw()->NewWorkPlaneSketch(2,clr,sketch2.pVertexArr,sketch2.VertexCount,sketch2.normal,"",50,false,100);
		}
		else if(pSchemaItem->m_ciSketchVec==2)
		{	//Y方向设计参数
			GEPOINT norm(0,1,0);
			if(pt1.y<pt2.y)
				norm*=-1;
			btc::SKETCH_PLANE sketch1;
			sketch1.CreateStdPlane(pt1,norm,GEPOINT(1,0,0));
			pView->SolidDraw()->NewWorkPlaneSketch(1,clr,sketch1.pVertexArr,sketch1.VertexCount,sketch1.normal,sLabel,50,false,100);
			btc::SKETCH_PLANE sketch2;
			sketch2.CreateStdPlane(pt2,norm*-1,GEPOINT(1,0,0));
			pView->SolidDraw()->NewWorkPlaneSketch(2,clr,sketch2.pVertexArr,sketch2.VertexCount,sketch2.normal,"",50,false,100);
		}
		else if(pSchemaItem->m_ciSketchVec==3)
		{	//Z方向设计参数
			double fX=(fabs(pt1.x)>fabs(pt2.x))?pt1.x:pt2.x;
			double fZ1=(pt1.z>pt2.z)?pt1.z:pt2.z;
			double fZ2=(pt1.z>pt2.z)?pt2.z:pt1.z;
			double fEdgeLen=fabs(pt1.x-pt2.x);
			btc::SKETCH_PLANE sketch1;
			sketch1.CreateStdPlane(GEPOINT(fX,0,fZ1),GEPOINT(0,0,1),GEPOINT(1,0,0),(int)fEdgeLen);
			pView->SolidDraw()->NewWorkPlaneSketch(1,clr,sketch1.pVertexArr,sketch1.VertexCount,sketch1.normal,sLabel,50,false,100);
			btc::SKETCH_PLANE sketch2;
			sketch2.CreateStdPlane(GEPOINT(fX,0,fZ2),GEPOINT(0,0,-1),GEPOINT(1,0,0),(int)fEdgeLen);
			pView->SolidDraw()->NewWorkPlaneSketch(2,clr,sketch2.pVertexArr,sketch2.VertexCount,sketch2.normal,"",50,false,100);
		}
		pView->SolidDraw()->Draw();
	}
}
BOOL ModifyProperty(CPropertyList* pPropList,CPropTreeItem* pItem,CString& valueStr)
{
	CTPSMModel* pModel=NULL;
	if(Manager.m_pActivePrj)
		pModel=Manager.m_pActivePrj->m_pActiveModel;
	if(pModel==NULL)
		return FALSE;
	CPropertyListOper<CTPSMModel> oper(pPropList,pModel);
	BOOL bRefesh=FALSE;
	if(pItem->m_idProp==CTPSMModel::GetPropID("DesPara.VoltGrade"))
		pModel->m_xDesPara.m_nVoltGrade=atoi(valueStr);
	else if(pItem->m_idProp==CTPSMModel::GetPropID("DesPara.ShapeType"))
		pModel->m_xDesPara.m_ciShapeType=valueStr[0]-'0';
	else if(pItem->m_idProp==CTPSMModel::GetPropID("DesPara.Circuit"))
		pModel->m_xDesPara.m_nCircuit=atoi(valueStr);
	else if(pItem->m_idProp==CTPSMModel::GetPropID("DesPara.Aaltitude"))
		pModel->m_xDesPara.m_fAaltitude=atof(valueStr);
	else if(pItem->m_idProp==CTPSMModel::GetPropID("DesPara.WindSpeed"))
		pModel->m_xDesPara.m_fWindSpeed=atof(valueStr);
	else if(pItem->m_idProp==CTPSMModel::GetPropID("DesPara.NiceThick"))
		pModel->m_xDesPara.m_fNiceThick=atof(valueStr);
	else if(pItem->m_idProp==CTPSMModel::GetPropID("DesPara.CheckNiceThick"))
		pModel->m_xDesPara.m_fCheckNiceThick=atof(valueStr);
	else if(pItem->m_idProp==CTPSMModel::GetPropID("DesPara.WindSpan"))
		pModel->m_xDesPara.m_fWindSpan=atof(valueStr);
	else if(pItem->m_idProp==CTPSMModel::GetPropID("DesPara.WeightSpan"))
		pModel->m_xDesPara.m_fWeightSpan=atof(valueStr);
	else if(pItem->m_idProp==CTPSMModel::GetPropID("DesPara.m_sAngleRange"))
		strcpy(pModel->m_xDesPara.m_sAngleRange,valueStr);
	else if(pItem->m_idProp==CTPSMModel::GetPropID("DesPara.TowerType"))
		pModel->m_xDesPara.m_ciTowerType=valueStr[0]-'0';
	else if(pItem->m_idProp==CTPSMModel::GetPropID("ActiveModule"))
	{
		CLDSModule *pModule=NULL;
		for(pModule=pModel->m_xTower.Module.GetFirst();pModule;pModule=pModel->m_xTower.Module.GetNext())
		{
			if(stricmp(pModule->description,valueStr)==0)
				break;
		}
		bRefesh=pModule?TRUE:FALSE;
		if(pModule)
			pModel->m_xTower.m_hActiveModule=pModule->handle;
		//
		UpdateInstanceProperty(pPropList,pItem);
	}
	else if(pItem->m_idProp==CTPSMModel::GetPropID("ActiveQuadLegNo1"))
	{
		CLDSModule *pModule=pModel->m_xTower.GetActiveModule();
		bRefesh=pModule?TRUE:FALSE;
		if(pModule)
		{
			pModule->m_arrActiveQuadLegNo[0]=valueStr[0]-'@'+(pModule->GetBodyNo()-1)*pModel->m_nMaxLegs;
			pModule->m_arrActiveQuadLegNo[1]=pModule->m_arrActiveQuadLegNo[0];
			pModule->m_arrActiveQuadLegNo[2]=pModule->m_arrActiveQuadLegNo[0];
			pModule->m_arrActiveQuadLegNo[3]=pModule->m_arrActiveQuadLegNo[0];
			//
			oper.UpdatePropItemValue("ActiveQuadLegNo2");
			oper.UpdatePropItemValue("ActiveQuadLegNo3");
			oper.UpdatePropItemValue("ActiveQuadLegNo4");
		}
	}
	else if(pItem->m_idProp==CTPSMModel::GetPropID("ActiveQuadLegNo2"))
	{
		CLDSModule *pModule=pModel->m_xTower.GetActiveModule();
		bRefesh=pModule?TRUE:FALSE;
		if(pModule)
			pModule->m_arrActiveQuadLegNo[1]=valueStr[0]-'@'+(pModule->GetBodyNo()-1)*pModel->m_nMaxLegs;
	}
	else if(pItem->m_idProp==CTPSMModel::GetPropID("ActiveQuadLegNo3"))
	{
		CLDSModule *pModule=pModel->m_xTower.GetActiveModule();
		bRefesh=pModule?TRUE:FALSE;
		if(pModule)
			pModule->m_arrActiveQuadLegNo[2]=valueStr[0]-'@'+(pModule->GetBodyNo()-1)*pModel->m_nMaxLegs;
	}
	else if(pItem->m_idProp==CTPSMModel::GetPropID("ActiveQuadLegNo4"))
	{
		CLDSModule *pModule=pModel->m_xTower.GetActiveModule();
		bRefesh=pModule?TRUE:FALSE;
		if(pModule)
			pModule->m_arrActiveQuadLegNo[3]=valueStr[0]-'@'+(pModule->GetBodyNo()-1)*pModel->m_nMaxLegs;
	}
	else
	{
		KEY4C paramKey=CTPSMModel::GetPropKey(pItem->m_idProp);
		DESIGN_PARAM_ITEM *pDesignItem=pModel->m_xTower.hashParams.GetValue(paramKey);
		if(pDesignItem==NULL)
			return FALSE;
		double fValue=atof(valueStr);
		if(paramKey!=KEY4C("P1")&&paramKey!=KEY4C("P2")&&paramKey!=KEY4C("P")&&paramKey!=KEY4C("A"))
			fValue*=1000;
		pModel->m_xTower.SetDesignItemValue(paramKey,fValue,pDesignItem->dataType);
		if(pModel->UpdateSlopeDesPara())
		{
			CXhChar100 sValue;
			if(pModel->GetPropValueStr(CTPSMModel::GetPropID("K2"),sValue))
				pPropList->SetItemPropValue(CTPSMModel::GetPropID("K2"),sValue);
			if(pModel->GetPropValueStr(CTPSMModel::GetPropID("K3"),sValue))
				pPropList->SetItemPropValue(CTPSMModel::GetPropID("K3"),sValue);
			if(pModel->GetPropValueStr(CTPSMModel::GetPropID("A"),sValue))
				pPropList->SetItemPropValue(CTPSMModel::GetPropID("A"),sValue);
			if(pModel->GetPropValueStr(CTPSMModel::GetPropID("D1"),sValue))
				pPropList->SetItemPropValue(CTPSMModel::GetPropID("D1"),sValue);
			if(pModel->GetPropValueStr(CTPSMModel::GetPropID("D2"),sValue))
				pPropList->SetItemPropValue(CTPSMModel::GetPropID("D2"),sValue);
			if(pModel->GetPropValueStr(CTPSMModel::GetPropID("D3"),sValue))
				pPropList->SetItemPropValue(CTPSMModel::GetPropID("D3"),sValue);
		}				
		//更新塔型节点
		pModel->ReBuildTower();
		CTPSMView* pView=theApp.GetActiveView();
		if(pView)
			pView->Refresh(FALSE);
		CMainFrame* pMainFrm=(CMainFrame*)AfxGetMainWnd();
		if(pMainFrm)
			pMainFrm->GetWireDataPage()->UpdateWireNodeList();
	}
	if(bRefesh)
	{
		CTPSMView* pView=theApp.GetActiveView();
		if(pView)
			pView->Refresh();
	}
	return TRUE;
}
BOOL ButtonClick(CPropertyList* pPropList,CPropTreeItem* pItem)
{
	CTPSMModel* pModel=NULL;
	if(Manager.m_pActivePrj)
		pModel=Manager.m_pActivePrj->m_pActiveModel;
	if(pModel==NULL)
		return FALSE;
	
	return TRUE;
}

BOOL PopMenuChick(CPropertyList* pPropList,CPropTreeItem* pItem,CString sMenuName,int iMenu)
{
	CTPSMModel* pModel=NULL;
	if(Manager.m_pActivePrj)
		pModel=Manager.m_pActivePrj->m_pActiveModel;
	if(pModel==NULL)
		return FALSE;
	
	return FALSE;
}
BOOL FireHelpStr(CPropertyList* pPropList,CPropTreeItem* pItem)
{
	CTPSMModel* pModel=NULL;
	if(Manager.m_pActivePrj)
		pModel=Manager.m_pActivePrj->m_pActiveModel;
	if(pModel==NULL||pItem==NULL)
		return FALSE;
	CMainFrame::PROMPT_LIFE xPromptLife(true);
	KEY4C paramKey=CTPSMModel::GetPropKey(pItem->m_idProp);
	DESIGN_PARAM_ITEM* pDesItem=pModel->m_xTower.hashParams.GetValue(paramKey);
	if(pDesItem==NULL)
		return FALSE;
	PARA_SKETCH_ITEM* pSketchItem=pModel->m_hashParaSketch.GetValue(paramKey);
	DrawParaSketch(pSketchItem,pDesItem->sLabel);
	//显示示意图片
	CXhChar200 sys_path,imagefile;
	GetSysPath(sys_path);
	if(pModel->m_xDesPara.m_ciTowerType==TYPE_SZC)
	{
		if(pDesItem->uiCategoryId==1)		//
			imagefile.Printf("%sImage_SZC.bmp",(char*)sys_path);
		else if(pDesItem->uiCategoryId==2)	//
			imagefile.Printf("%sT_TX11.bmp",(char*)sys_path);
		else if(pDesItem->uiCategoryId==3||pDesItem->uiCategoryId==4)
			imagefile.Printf("%sT_HX2.bmp",(char*)sys_path);
		if(imagefile.GetLength()>0)
			xPromptLife.SetPromptPicture(imagefile);
	}
	else if(pModel->m_xDesPara.m_ciTowerType==TYPE_JG)
	{
		if(pDesItem->uiCategoryId==1)
			imagefile.Printf("%sImage_JG.bmp",(char*)sys_path);
		else if(pDesItem->uiCategoryId==2)
			imagefile.Printf("%sT_HX1.bmp",(char*)sys_path);
		else if(pDesItem->uiCategoryId==3)
			imagefile.Printf("%sT_HX12.bmp",(char*)sys_path);
		else if(pDesItem->uiCategoryId==4)
			imagefile.Printf("%sT_HX11.bmp",(char*)sys_path);
		else if(pDesItem->uiCategoryId==5)
			imagefile.Printf("%sT_HX9.bmp",(char*)sys_path);
		if(imagefile.GetLength()>0)
			xPromptLife.SetPromptPicture(imagefile);
	}
	else if(pModel->m_xDesPara.m_ciTowerType==TYPE_JG2)
	{
		if(pDesItem->uiCategoryId==1)
			imagefile.Printf("%sImage_JG2.bmp",(char*)sys_path);
		else if(pDesItem->uiCategoryId==2)
			imagefile.Printf("%sT_HX1.bmp",(char*)sys_path);
		else if(pDesItem->uiCategoryId==3)
			imagefile.Printf("%sT_HX12.bmp",(char*)sys_path);
		else if(pDesItem->uiCategoryId==4)
			imagefile.Printf("%sT_HX2.bmp",(char*)sys_path);
		else if(pDesItem->uiCategoryId==5)
			imagefile.Printf("%sT_HX11.bmp",(char*)sys_path);
		else if(pDesItem->uiCategoryId==6)
			imagefile.Printf("%sT_HX9.bmp",(char*)sys_path);
		if(imagefile.GetLength()>0)
			xPromptLife.SetPromptPicture(imagefile);
	}
	else if(pModel->m_xDesPara.m_ciTowerType==TYPE_SJ)
	{
		if(pDesItem->uiCategoryId==1)
			imagefile.Printf("%sImage_SJ.bmp",(char*)sys_path);
		else if(pDesItem->uiCategoryId==2||pDesItem->uiCategoryId==3)
			imagefile.Printf("%sT_HX1.bmp",(char*)sys_path);
		else if(pDesItem->uiCategoryId>=4)
			imagefile.Printf("%sT_HX8.bmp",(char*)sys_path);
		if(imagefile.GetLength()>0)
			xPromptLife.SetPromptPicture(imagefile);
	}
	else if(pModel->m_xDesPara.m_ciTowerType==TYPE_SJB)
	{
		if(pDesItem->uiCategoryId==1)
			imagefile.Printf("%sImage_SJB.bmp",(char*)sys_path);
		else if(pDesItem->uiCategoryId==2)
			imagefile.Printf("%sT_TX4.bmp",(char*)sys_path);
		else if(pDesItem->uiCategoryId==3)
			imagefile.Printf("%sT_TX3.bmp",(char*)sys_path);
		else if(pDesItem->uiCategoryId==4||pDesItem->uiCategoryId==6)
			imagefile.Printf("%sT_HX14.bmp",(char*)sys_path);
		else if(pDesItem->uiCategoryId==5||pDesItem->uiCategoryId==7)
			imagefile.Printf("%sT_HX13.bmp",(char*)sys_path);
		if(imagefile.GetLength()>0)
			xPromptLife.SetPromptPicture(imagefile);
	}
	else if(pModel->m_xDesPara.m_ciTowerType==TYPE_SJC)
	{
		if(pDesItem->uiCategoryId==1)
			imagefile.Printf("%sImage_SJC.bmp",(char*)sys_path);
		else if(pDesItem->uiCategoryId==2)
			imagefile.Printf("%sT_TX6.bmp",(char*)sys_path);
		else if(pDesItem->uiCategoryId==3)
			imagefile.Printf("%sT_TX5.bmp",(char*)sys_path);
		else if(pDesItem->uiCategoryId==4||pDesItem->uiCategoryId==6)
			imagefile.Printf("%sT_HX14.bmp",(char*)sys_path);
		else if(pDesItem->uiCategoryId==5||pDesItem->uiCategoryId==7)
			imagefile.Printf("%sT_HX13.bmp",(char*)sys_path);
		if(imagefile.GetLength()>0)
			xPromptLife.SetPromptPicture(imagefile);
	}
	else if(pModel->m_xDesPara.m_ciTowerType==TYPE_ZC)
	{
		if(pDesItem->uiCategoryId==1)
			imagefile.Printf("%sImage_ZC.bmp",(char*)sys_path);
		else if(pDesItem->uiCategoryId==2)
			imagefile.Printf("%sT_TX12.bmp",(char*)sys_path);
		if(imagefile.GetLength()>0)
			xPromptLife.SetPromptPicture(imagefile);
	}
	else if(pModel->m_xDesPara.m_ciTowerType==TYPE_ZBC)
	{
		if(pDesItem->uiCategoryId==1)
			imagefile.Printf("%sImage_ZBC.bmp",(char*)sys_path);
		else if(pDesItem->uiCategoryId==2)
			imagefile.Printf("%sT_ZX3.bmp",(char*)sys_path);
		if(imagefile.GetLength()>0)
			xPromptLife.SetPromptPicture(imagefile);
	}
	else if(pModel->m_xDesPara.m_ciTowerType==TYPE_ZM)
	{
		if(pDesItem->uiCategoryId==1)
			imagefile.Printf("%sImage_ZM.bmp",(char*)sys_path);
		else if(pDesItem->uiCategoryId==2)
			imagefile.Printf("%sT_ZM1.bmp",(char*)sys_path);
		if(imagefile.GetLength()>0)
			xPromptLife.SetPromptPicture(imagefile);
	}
	else if(pModel->m_xDesPara.m_ciTowerType==TYPE_SZ)
	{
		if(pDesItem->uiCategoryId==1)
			imagefile.Printf("%sImage_SZ.bmp",(char*)sys_path);
		else if(pDesItem->uiCategoryId==2)
			imagefile.Printf("%sT_TX13.bmp",(char*)sys_path);
		else if(pDesItem->uiCategoryId==3||pDesItem->uiCategoryId==4)
			imagefile.Printf("%sT_HX2.bmp",(char*)sys_path);
		if(imagefile.GetLength()>0)
			xPromptLife.SetPromptPicture(imagefile);
	}
	else if(pModel->m_xDesPara.m_ciTowerType==TYPE_JC)
	{
		if(pDesItem->uiCategoryId==1)
			imagefile.Printf("%sImage_JC.bmp",(char*)sys_path);
		else if(pDesItem->uiCategoryId==2||pDesItem->uiCategoryId==3)
			imagefile.Printf("%sT_HX1.bmp",(char*)sys_path);
		else if(pDesItem->uiCategoryId==4||pDesItem->uiCategoryId==5)
			imagefile.Printf("%sT_HX15.bmp",(char*)sys_path);
		if(imagefile.GetLength()>0)
			xPromptLife.SetPromptPicture(imagefile);
	}
	else if (pModel->m_xDesPara.m_ciTowerType == TYPE_JBB)
	{
		if (pDesItem->uiCategoryId == 1)
			imagefile.Printf("%sImage_JBB.bmp", (char*)sys_path);
		else if (pDesItem->uiCategoryId == 2)
			imagefile.Printf("%sT_ZX4.bmp", (char*)sys_path);
		if (imagefile.GetLength() > 0)
			xPromptLife.SetPromptPicture(imagefile);
	}
	else
		return FALSE;
	return TRUE;
}
//////////////////////////////////////////////////////////////////////////
//
CString MakeModuleStr(CTower* pTower)
{
	CString sModuleArr;
	if(pTower==NULL)
		return sModuleArr;
	for(CLDSModule *pModule=pTower->Module.GetFirst();pModule;pModule=pTower->Module.GetNext())
	{
		sModuleArr.Append(pModule->description);
		sModuleArr.Append("|");
	}
	return sModuleArr;
}
void UpdateInstanceProperty(CPropertyList* pPropList,CPropTreeItem* pParentItem)
{
	CTPSMModel* pModel=(Manager.m_pActivePrj)?pModel=Manager.m_pActivePrj->m_pActiveModel:NULL;
	if(pModel==NULL)
		return;
	CLDSModule *pModule=pModel->m_xTower.Module.FromHandle(pModel->m_xTower.m_hActiveModule);
	if(pModule==NULL)
		return;
	CPropertyListOper<CTPSMModel> oper(pPropList,pModel);
	CString sLegArr=pModel->MakeModuleLegStr(pModule);
	oper.InsertCmbListPropItem(pParentItem,"ActiveQuadLegNo1",sLegArr,"","",-1,TRUE);
	oper.InsertCmbListPropItem(pParentItem,"ActiveQuadLegNo2",sLegArr,"","",-1,TRUE);
	oper.InsertCmbListPropItem(pParentItem,"ActiveQuadLegNo3",sLegArr,"","",-1,TRUE);
	oper.InsertCmbListPropItem(pParentItem,"ActiveQuadLegNo4",sLegArr,"","",-1,TRUE);
}
void CMainFrame::DisplayTowerParaProperty()
{
	CPropertyDlg *pPropDlg=GetPropertyPage();
	if(pPropDlg==NULL)
		return;
	CTPSMModel* pModel=(Manager.m_pActivePrj)?pModel=Manager.m_pActivePrj->m_pActiveModel:NULL;
	if(pModel==NULL)
		return;
	CPropertyList *pPropList=pPropDlg->GetPropertyList();
	pPropList->CleanCallBackFunc();
	pPropList->CleanTree();
	pPropList->m_nObjClassTypeId = 0;
	pPropList->SetModifyValueFunc(ModifyProperty);
	pPropList->SetButtonClickFunc(ButtonClick);
	pPropList->SetPropHelpPromptFunc(FireHelpStr);
	pPropList->SetPopMenuClickFunc(PopMenuChick);
	CPropertyListOper<CTPSMModel> oper(pPropList,pModel);
	CPropTreeItem* pRootItem=pPropList->GetRootItem();
	CPropTreeItem *pGroupItem=NULL,*pPropItem=NULL;
	//
	pGroupItem=oper.InsertPropItem(pRootItem,"HeightGroup");
	pPropItem=oper.InsertCmbListPropItem(pGroupItem,"ActiveModule",MakeModuleStr(&pModel->m_xTower));
	UpdateInstanceProperty(pPropList,pPropItem);
	//模型尺寸参数
	if(pModel->m_xDesPara.m_ciTowerType==TYPE_SZC)
	{	//控制参数
		pGroupItem=oper.InsertPropItem(pRootItem,"CONTRl_PARA");
		oper.InsertEditPropItem(pGroupItem,"Z","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"Z1","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"Z2","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"Z3","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"Z4","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"Z5","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"K1","","",-1,TRUE);
		pPropItem=oper.InsertEditPropItem(pGroupItem,"K2","","",-1,TRUE);
		pPropItem->SetReadOnly(TRUE);
		pPropItem=oper.InsertEditPropItem(pGroupItem,"K3","","",-1,TRUE);
		pPropItem->SetReadOnly(TRUE);
		oper.InsertEditPropItem(pGroupItem,"P1","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"P2","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"L","","",-1,TRUE);
		pPropItem=oper.InsertEditPropItem(pGroupItem,"A","","",-1,TRUE);
		pPropItem->SetReadOnly(TRUE);
		//上侧横担
		pGroupItem=oper.InsertPropItem(pRootItem,"ARM1_PARA","导地线复合横担");
		oper.InsertEditPropItem(pGroupItem,"XA1","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"XA2","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"XA3","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"YA1","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"YA2","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"ZA1","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"ZA2","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"ZA3","","",-1,TRUE);
		//中间横担
		pGroupItem=oper.InsertPropItem(pRootItem,"ARM2_PARA","中导线横担");
		oper.InsertEditPropItem(pGroupItem,"XB","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"YB","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"ZB","","",-1,TRUE);
		//下侧横担
		pGroupItem=oper.InsertPropItem(pRootItem,"ARM3_PARA","下导线横担");
		oper.InsertEditPropItem(pGroupItem,"XC","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"YC","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"ZC","","",-1,TRUE);
	}
	else if(pModel->m_xDesPara.m_ciTowerType==TYPE_JG)
	{
		//控制参数
		pGroupItem=oper.InsertPropItem(pRootItem,"CONTRl_PARA");
		oper.InsertEditPropItem(pGroupItem,"Z1","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"Z2","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"Z3","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"Z4","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"K1","","",-1,TRUE);
		pPropItem=oper.InsertEditPropItem(pGroupItem,"K2","","",-1,TRUE);
		pPropItem->SetReadOnly(TRUE);
		pPropItem=oper.InsertEditPropItem(pGroupItem,"K3","","",-1,TRUE);
		pPropItem->SetReadOnly(TRUE);
		oper.InsertEditPropItem(pGroupItem,"P1","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"P2","","",-1,TRUE);
		//第一层横担
		pGroupItem=oper.InsertPropItem(pRootItem,"ARM1_PARA","第一层横担");
		oper.InsertEditPropItem(pGroupItem,"ZAB","","",-1,TRUE);
		//左上横担
		pPropItem=oper.InsertPropItem(pGroupItem,"ARM1_L_PARA","左侧横担");
		pPropItem->m_bHideChildren=TRUE;
		oper.InsertEditPropItem(pPropItem,"XA","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"YA","","",-1,TRUE);
		//右上横担
		pPropItem=oper.InsertPropItem(pGroupItem,"ARM1_R_PARA","右侧横担");
		pPropItem->m_bHideChildren=TRUE;
		oper.InsertEditPropItem(pPropItem,"XB1","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XB2","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XB3","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"YB","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"YB1","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"YB2","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"ZB","","",-1,TRUE);
		//第二层横担
		pGroupItem=oper.InsertPropItem(pRootItem,"ARM2_PARA","第二层横担");
		oper.InsertEditPropItem(pGroupItem,"ZCD","","",-1,TRUE);
		//左下横担
		pPropItem=oper.InsertPropItem(pGroupItem,"ARM2_L_PARA","左侧横担");
		pPropItem->m_bHideChildren=TRUE;
		oper.InsertEditPropItem(pPropItem,"XC1","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XC2","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XC3","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"YC","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"ZC","","",-1,TRUE);
		//右下横担
		pPropItem=oper.InsertPropItem(pGroupItem,"ARM2_R_PARA","右侧横担");
		pPropItem->m_bHideChildren=TRUE;
		oper.InsertEditPropItem(pPropItem,"XD1","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XD2","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"YD","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"ZD","","",-1,TRUE);
	}
	else if(pModel->m_xDesPara.m_ciTowerType==TYPE_JG2)
	{
		//控制参数
		pGroupItem=oper.InsertPropItem(pRootItem,"CONTRl_PARA");
		oper.InsertEditPropItem(pGroupItem,"Z1","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"Z2","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"Z3","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"Z4","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"K1","","",-1,TRUE);
		pPropItem=oper.InsertEditPropItem(pGroupItem,"K2","","",-1,TRUE);
		pPropItem->SetReadOnly(TRUE);
		pPropItem=oper.InsertEditPropItem(pGroupItem,"K3","","",-1,TRUE);
		pPropItem->SetReadOnly(TRUE);
		oper.InsertEditPropItem(pGroupItem,"P1","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"P2","","",-1,TRUE);
		//第一层横担
		pGroupItem=oper.InsertPropItem(pRootItem,"ARM1_PARA","上层横担");
		oper.InsertEditPropItem(pGroupItem,"ZAB","","",-1,TRUE);
		//左上横担
		pPropItem=oper.InsertPropItem(pGroupItem,"ARM1_L_PARA","左侧横担");
		pPropItem->m_bHideChildren=TRUE;
		oper.InsertEditPropItem(pPropItem,"XA","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"YA","","",-1,TRUE);
		//右上横担
		pPropItem=oper.InsertPropItem(pGroupItem,"ARM1_R_PARA","右侧横担");
		pPropItem->m_bHideChildren=TRUE;
		oper.InsertEditPropItem(pPropItem,"XB1","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XB2","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XB3","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"YB","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"YB1","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"YB2","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"ZB","","",-1,TRUE);
		//中层横担
		pGroupItem=oper.InsertPropItem(pRootItem,"ARM2_PARA","中层横担");
		oper.InsertEditPropItem(pGroupItem,"XC","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"YC","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"ZC","","",-1,TRUE);
		//下层横担
		pGroupItem=oper.InsertPropItem(pRootItem,"ARM3_PARA","下层横担");
		oper.InsertEditPropItem(pGroupItem,"ZDE","","",-1,TRUE);
		//左下横担
		pPropItem=oper.InsertPropItem(pGroupItem,"ARM3_L_PARA","左侧横担");
		pPropItem->m_bHideChildren=TRUE;
		oper.InsertEditPropItem(pPropItem,"XD1","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XD2","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XD3","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"YD","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"ZD","","",-1,TRUE);
		//右下横担
		pPropItem=oper.InsertPropItem(pGroupItem,"ARM3_R_PARA","右侧横担");
		pPropItem->m_bHideChildren=TRUE;
		oper.InsertEditPropItem(pPropItem,"XE1","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XE2","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"YE","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"ZE","","",-1,TRUE);
	}
	else if(pModel->m_xDesPara.m_ciTowerType==TYPE_SJ)
	{
		pGroupItem=oper.InsertPropItem(pRootItem,"CONTRl_PARA");
		oper.InsertEditPropItem(pGroupItem,"Z1","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"Z2","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"Z3","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"Z4","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"Z5","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"K1","","",-1,TRUE);
		pPropItem=oper.InsertEditPropItem(pGroupItem,"K2","","",-1,TRUE);
		pPropItem->SetReadOnly(TRUE);
		pPropItem=oper.InsertEditPropItem(pGroupItem,"K3","","",-1,TRUE);
		pPropItem->SetReadOnly(TRUE);
		oper.InsertEditPropItem(pGroupItem,"P1","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"P2","","",-1,TRUE);
		//第一层横担
		pGroupItem=oper.InsertPropItem(pRootItem,"ARM1_PARA","第一层横担");
		pGroupItem->m_bHideChildren=TRUE;
		oper.InsertEditPropItem(pGroupItem,"ZAB","","",-1,TRUE);
		//左侧
		pPropItem=oper.InsertPropItem(pGroupItem,"ARM1_L_PARA","左侧横担");
		oper.InsertEditPropItem(pPropItem,"XA","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"YA","","",-1,TRUE);
		//右侧
		pPropItem=oper.InsertPropItem(pGroupItem,"ARM1_R_PARA","右侧横担");
		oper.InsertEditPropItem(pPropItem,"XB","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"YB","","",-1,TRUE);
		//第二层横担
		pGroupItem=oper.InsertPropItem(pRootItem,"ARM2_PARA","第二层横担");
		pGroupItem->m_bHideChildren=TRUE;
		oper.InsertEditPropItem(pGroupItem,"ZCD","","",-1,TRUE);
		//左侧
		pPropItem=oper.InsertPropItem(pGroupItem,"ARM2_L_PARA","左侧横担");
		oper.InsertEditPropItem(pPropItem,"XC1","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XC2","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XC3","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XC4","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"YC","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"ZC2","","",-1,TRUE);
		//右侧
		pPropItem=oper.InsertPropItem(pGroupItem,"ARM2_R_PARA","右侧横担");
		oper.InsertEditPropItem(pPropItem,"XD1","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XD2","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XD3","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XD4","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"YD","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"ZD2","","",-1,TRUE);
		//第三层横担
		pGroupItem=oper.InsertPropItem(pRootItem,"ARM3_PARA","第三层横担");
		pGroupItem->m_bHideChildren=TRUE;
		oper.InsertEditPropItem(pGroupItem,"ZEF","","",-1,TRUE);
		//左侧
		pPropItem=oper.InsertPropItem(pGroupItem,"ARM3_L_PARA","左侧横担");
		oper.InsertEditPropItem(pPropItem,"XE1","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XE2","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XE3","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XE4","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"YE","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"ZE2","","",-1,TRUE);
		//右侧
		pPropItem=oper.InsertPropItem(pGroupItem,"ARM3_R_PARA","右侧横担");
		oper.InsertEditPropItem(pPropItem,"XF1","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XF2","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XF3","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XF4","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"YF","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"ZF2","","",-1,TRUE);
		//第四层横担
		pGroupItem=oper.InsertPropItem(pRootItem,"ARM4_PARA","第四层横担");
		pGroupItem->m_bHideChildren=TRUE;
		oper.InsertEditPropItem(pGroupItem,"ZGH","","",-1,TRUE);
		//左侧
		pPropItem=oper.InsertPropItem(pGroupItem,"ARM4_L_PARA","左侧横担");
		oper.InsertEditPropItem(pPropItem,"XG1","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XG2","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XG3","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XG4","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"YG","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"ZG2","","",-1,TRUE);
		//右侧
		pPropItem=oper.InsertPropItem(pGroupItem,"ARM4_R_PARA","右侧横担");
		oper.InsertEditPropItem(pPropItem,"XH1","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XH2","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XH3","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XH4","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"YH","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"ZH2","","",-1,TRUE);
	}
	else if(pModel->m_xDesPara.m_ciTowerType==TYPE_SJB)
	{	//控制参数
		pGroupItem=oper.InsertPropItem(pRootItem,"CONTRl_PARA");
		oper.InsertEditPropItem(pGroupItem,"Z1","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"Z2","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"Z3","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"Z4","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"Z5","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"K1","","",-1,TRUE);
		pPropItem=oper.InsertEditPropItem(pGroupItem,"K2","","",-1,TRUE);
		pPropItem->SetReadOnly(TRUE);
		pPropItem=oper.InsertEditPropItem(pGroupItem,"K3","","",-1,TRUE);
		pPropItem->SetReadOnly(TRUE);
		oper.InsertEditPropItem(pGroupItem,"P1","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"P2","","",-1,TRUE);
		//第一层横担
		pGroupItem=oper.InsertPropItem(pRootItem,"ARM1_PARA","第一层横担");
		pGroupItem->m_bHideChildren=TRUE;
		//左侧
		pPropItem=oper.InsertPropItem(pGroupItem,"ARM1_L_PARA","左侧横担");
		oper.InsertEditPropItem(pPropItem,"XA1","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XA2","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XA3","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XA4","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XA5","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XA6","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"YA1","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"YA2","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"ZA1","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"ZA2","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"ZA3","","",-1,TRUE);
		//右侧
		pPropItem=oper.InsertPropItem(pGroupItem,"ARM1_R_PARA","右侧横担");
		oper.InsertEditPropItem(pPropItem,"XB1","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XB2","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XB3","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XB4","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XB5","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"YB1","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"YB2","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"ZB1","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"ZB2","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"ZB3","","",-1,TRUE);
		//第二层横担
		pGroupItem=oper.InsertPropItem(pRootItem,"ARM2_PARA","第二层横担");
		pGroupItem->m_bHideChildren=TRUE;
		oper.InsertEditPropItem(pGroupItem,"ZCD","","",-1,TRUE);
		//左侧
		pPropItem=oper.InsertPropItem(pGroupItem,"ARM2_L_PARA","左侧横担");
		oper.InsertEditPropItem(pPropItem,"XC1","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XC2","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XC3","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XC4","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"YC","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"ZC2","","",-1,TRUE);
		//右侧
		pPropItem=oper.InsertPropItem(pGroupItem,"ARM2_R_PARA","右侧横担");
		oper.InsertEditPropItem(pPropItem,"XD1","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XD2","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XD3","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"YD","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"ZD2","","",-1,TRUE);
		//第三层横担
		pGroupItem=oper.InsertPropItem(pRootItem,"ARM3_PARA","第三层横担");
		pGroupItem->m_bHideChildren=TRUE;
		oper.InsertEditPropItem(pGroupItem,"ZEF","","",-1,TRUE);
		//左侧
		pPropItem=oper.InsertPropItem(pGroupItem,"ARM3_L_PARA","左侧横担");
		oper.InsertEditPropItem(pPropItem,"XE1","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XE2","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XE3","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XE4","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"YE","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"ZE2","","",-1,TRUE);
		//右侧
		pPropItem=oper.InsertPropItem(pGroupItem,"ARM3_R_PARA","右侧横担");
		oper.InsertEditPropItem(pPropItem,"XF1","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XF2","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XF3","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"YF","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"ZF2","","",-1,TRUE);
	}
	else if(pModel->m_xDesPara.m_ciTowerType==TYPE_SJC)
	{	//控制参数
		pGroupItem=oper.InsertPropItem(pRootItem,"CONTRl_PARA");
		oper.InsertEditPropItem(pGroupItem,"Z1","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"Z2","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"Z3","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"Z4","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"Z5","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"K1","","",-1,TRUE);
		pPropItem=oper.InsertEditPropItem(pGroupItem,"K2","","",-1,TRUE);
		pPropItem->SetReadOnly(TRUE);
		pPropItem=oper.InsertEditPropItem(pGroupItem,"K3","","",-1,TRUE);
		pPropItem->SetReadOnly(TRUE);
		oper.InsertEditPropItem(pGroupItem,"P1","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"P2","","",-1,TRUE);
		//第一层横担
		pGroupItem=oper.InsertPropItem(pRootItem,"ARM1_PARA","第一层横担");
		pGroupItem->m_bHideChildren=TRUE;
		//左侧
		pPropItem=oper.InsertPropItem(pGroupItem,"ARM1_L_PARA","左侧横担");
		oper.InsertEditPropItem(pPropItem,"XA1","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XA2","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XA3","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XA4","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XA5","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XA6","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"YA1","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"YA2","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"ZA1","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"ZA2","","",-1,TRUE);
		//右侧
		pPropItem=oper.InsertPropItem(pGroupItem,"ARM1_R_PARA","右侧横担");
		oper.InsertEditPropItem(pPropItem,"XB1","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XB2","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XB3","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XB4","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XB5","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"YB1","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"YB2","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"ZB1","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"ZB2","","",-1,TRUE);
		//第二层横担
		pGroupItem=oper.InsertPropItem(pRootItem,"ARM2_PARA","第二层横担");
		pGroupItem->m_bHideChildren=TRUE;
		oper.InsertEditPropItem(pGroupItem,"ZCD","","",-1,TRUE);
		//左侧
		pPropItem=oper.InsertPropItem(pGroupItem,"ARM2_L_PARA","左侧横担");
		oper.InsertEditPropItem(pPropItem,"XC1","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XC2","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XC3","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XC4","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"YC","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"ZC2","","",-1,TRUE);
		//右侧
		pPropItem=oper.InsertPropItem(pGroupItem,"ARM2_R_PARA","右侧横担");
		oper.InsertEditPropItem(pPropItem,"XD1","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XD2","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XD3","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"YD","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"ZD2","","",-1,TRUE);
		//第三层横担
		pGroupItem=oper.InsertPropItem(pRootItem,"ARM3_PARA","第三层横担");
		pGroupItem->m_bHideChildren=TRUE;
		oper.InsertEditPropItem(pGroupItem,"ZEF","","",-1,TRUE);
		//左侧
		pPropItem=oper.InsertPropItem(pGroupItem,"ARM3_L_PARA","左侧横担");
		oper.InsertEditPropItem(pPropItem,"XE1","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XE2","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XE3","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XE4","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"YE","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"ZE2","","",-1,TRUE);
		//右侧
		pPropItem=oper.InsertPropItem(pGroupItem,"ARM3_R_PARA","右侧横担");
		oper.InsertEditPropItem(pPropItem,"XF1","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XF2","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XF3","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"YF","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"ZF2","","",-1,TRUE);
	}
	else if(pModel->m_xDesPara.m_ciTowerType==TYPE_ZC)
	{	//控制参数
		pGroupItem=oper.InsertPropItem(pRootItem,"CONTRl_PARA");
		oper.InsertEditPropItem(pGroupItem,"Z1","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"Z2","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"Z3","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"Z4","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"K1","","",-1,TRUE);
		pPropItem=oper.InsertEditPropItem(pGroupItem,"K2","","",-1,TRUE);
		pPropItem->SetReadOnly(TRUE);
		pPropItem=oper.InsertEditPropItem(pGroupItem,"K3","","",-1,TRUE);
		pPropItem->SetReadOnly(TRUE);
		oper.InsertEditPropItem(pGroupItem,"P1","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"P2","","",-1,TRUE);
		//第一层横担
		pGroupItem=oper.InsertPropItem(pRootItem,"ARM1_PARA","第一层横担");
		oper.InsertEditPropItem(pGroupItem,"XA1","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"XA2","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"XA3","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"XA4","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"XA5","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"XA6","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"YA1","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"YA2","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"ZA1","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"ZA2","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"ZA3","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"ZA4","","",-1,TRUE);
	}
	else if(pModel->m_xDesPara.m_ciTowerType==TYPE_ZBC)
	{
		pGroupItem=oper.InsertPropItem(pRootItem,"CONTRl_PARA");
		oper.InsertEditPropItem(pGroupItem,"Z","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"K1","","",-1,TRUE);
		pPropItem=oper.InsertEditPropItem(pGroupItem,"K2","","",-1,TRUE);
		pPropItem->SetReadOnly(TRUE);
		oper.InsertEditPropItem(pGroupItem,"P","","",-1,TRUE);
		//酒杯塔头
		pGroupItem=oper.InsertPropItem(pRootItem,"ARM1_PARA","酒杯塔头");
		oper.InsertEditPropItem(pGroupItem,"XA1","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"XA2","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"XA3","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"XA4","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"XA5","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"XA6","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"YA1","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"YA2","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"YA3","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"ZA1","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"ZA2","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"ZA3","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"ZA4","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"ZA5","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"ZA6","","",-1,TRUE);
	}
	else if(pModel->m_xDesPara.m_ciTowerType==TYPE_ZM)
	{
		pGroupItem=oper.InsertPropItem(pRootItem,"CONTRl_PARA");
		oper.InsertEditPropItem(pGroupItem,"Z","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"K1","","",-1,TRUE);
		pPropItem=oper.InsertEditPropItem(pGroupItem,"K2","","",-1,TRUE);
		pPropItem->SetReadOnly(TRUE);
		oper.InsertEditPropItem(pGroupItem,"P","","",-1,TRUE);
		//猫头塔头
		pGroupItem=oper.InsertPropItem(pRootItem,"ARM1_PARA","猫头塔头");
		oper.InsertEditPropItem(pGroupItem,"X1","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"X2","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"X3","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"X4","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"X5","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"X6","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"X7","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"X8","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"Y1","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"Y2","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"Y3","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"Z1","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"Z2","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"Z3","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"Z4","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"Z5","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"Z6","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"Z7","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"Z8","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"Z9","","",-1,TRUE);
	}
	else if(pModel->m_xDesPara.m_ciTowerType==TYPE_SZ)
	{
		pGroupItem=oper.InsertPropItem(pRootItem,"CONTRl_PARA");
		oper.InsertEditPropItem(pGroupItem,"Z","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"Z1","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"Z2","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"Z3","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"Z4","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"Z5","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"K1","","",-1,TRUE);
		pPropItem=oper.InsertEditPropItem(pGroupItem,"K2","","",-1,TRUE);
		pPropItem->SetReadOnly(TRUE);
		pPropItem=oper.InsertEditPropItem(pGroupItem,"K3","","",-1,TRUE);
		pPropItem->SetReadOnly(TRUE);
		oper.InsertEditPropItem(pGroupItem,"P1","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"P2","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"L","","",-1,TRUE);
		pPropItem=oper.InsertEditPropItem(pGroupItem,"A","","",-1,TRUE);
		pPropItem->SetReadOnly(TRUE);
		//上侧横担
		pGroupItem=oper.InsertPropItem(pRootItem,"ARM1_PARA","导地线复合横担");
		oper.InsertEditPropItem(pGroupItem,"XA1","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"XA2","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"XA3","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"XA4","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"XA5","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"YA1","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"YA2","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"ZA1","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"ZA2","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"ZA3","","",-1,TRUE);
		//中间横担
		pGroupItem=oper.InsertPropItem(pRootItem,"ARM2_PARA","中导线横担");
		oper.InsertEditPropItem(pGroupItem,"XB","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"YB","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"ZB","","",-1,TRUE);
		//下侧横担
		pGroupItem=oper.InsertPropItem(pRootItem,"ARM3_PARA","下导线横担");
		oper.InsertEditPropItem(pGroupItem,"XC","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"YC","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"ZC","","",-1,TRUE);
	}
	else if(pModel->m_xDesPara.m_ciTowerType==TYPE_JC)
	{
		pGroupItem=oper.InsertPropItem(pRootItem,"CONTRl_PARA");
		oper.InsertEditPropItem(pGroupItem,"Z1","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"Z2","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"Z3","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"K1","","",-1,TRUE);
		pPropItem=oper.InsertEditPropItem(pGroupItem,"K2","","",-1,TRUE);
		pPropItem->SetReadOnly(TRUE);
		pPropItem=oper.InsertEditPropItem(pGroupItem,"K3","","",-1,TRUE);
		pPropItem->SetReadOnly(TRUE);
		oper.InsertEditPropItem(pGroupItem,"P1","","",-1,TRUE);
		oper.InsertEditPropItem(pGroupItem,"P2","","",-1,TRUE);
		//地线支架
		pGroupItem=oper.InsertPropItem(pRootItem,"ARM1_PARA","地线支架");
		oper.InsertEditPropItem(pGroupItem,"ZAB","","",-1,TRUE);
		pPropItem=oper.InsertPropItem(pGroupItem,"ARM1_L_PARA","左侧地线支架");
		oper.InsertEditPropItem(pPropItem,"XA","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"YA","","",-1,TRUE);
		pPropItem=oper.InsertPropItem(pGroupItem,"ARM1_R_PARA","右侧地线支架");
		oper.InsertEditPropItem(pPropItem,"XB","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"YB","","",-1,TRUE);
		//横担
		pGroupItem=oper.InsertPropItem(pRootItem,"ARM2_PARA","导线横担");
		oper.InsertEditPropItem(pGroupItem,"ZCD","","",-1,TRUE);
		//左侧
		pPropItem=oper.InsertPropItem(pGroupItem,"ARM2_L_PARA","左侧横担");
		oper.InsertEditPropItem(pPropItem,"XC1","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XC2","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XC3","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XC4","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"YC","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"ZC","","",-1,TRUE);
		//右侧
		pPropItem=oper.InsertPropItem(pGroupItem,"ARM2_R_PARA","右侧横担");
		oper.InsertEditPropItem(pPropItem,"XD1","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XD2","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XD3","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"XD4","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"YD","","",-1,TRUE);
		oper.InsertEditPropItem(pPropItem,"ZD","","",-1,TRUE);
	}
	else if (pModel->m_xDesPara.m_ciTowerType == TYPE_JBB)
	{
		pGroupItem = oper.InsertPropItem(pRootItem, "CONTRl_PARA");
		oper.InsertEditPropItem(pGroupItem, "Z", "", "", -1, TRUE);
		oper.InsertEditPropItem(pGroupItem, "K1", "", "", -1, TRUE);
		pPropItem = oper.InsertEditPropItem(pGroupItem, "K2", "", "", -1, TRUE);
		pPropItem->SetReadOnly(TRUE);
		oper.InsertEditPropItem(pGroupItem, "P", "", "", -1, TRUE);
		//酒杯塔头
		pGroupItem = oper.InsertPropItem(pRootItem, "ARM1_PARA", "酒杯塔头");
		oper.InsertEditPropItem(pGroupItem, "X1", "", "", -1, TRUE);
		oper.InsertEditPropItem(pGroupItem, "X2", "", "", -1, TRUE);
		oper.InsertEditPropItem(pGroupItem, "X3", "", "", -1, TRUE);
		oper.InsertEditPropItem(pGroupItem, "X4", "", "", -1, TRUE);
		oper.InsertEditPropItem(pGroupItem, "X5", "", "", -1, TRUE);
		oper.InsertEditPropItem(pGroupItem, "X6", "", "", -1, TRUE);
		oper.InsertEditPropItem(pGroupItem, "X7", "", "", -1, TRUE);
		oper.InsertEditPropItem(pGroupItem, "X8", "", "", -1, TRUE);
		oper.InsertEditPropItem(pGroupItem, "X9", "", "", -1, TRUE);
		oper.InsertEditPropItem(pGroupItem, "W1", "", "", -1, TRUE);
		oper.InsertEditPropItem(pGroupItem, "W2", "", "", -1, TRUE);
		oper.InsertEditPropItem(pGroupItem, "W3", "", "", -1, TRUE);
		oper.InsertEditPropItem(pGroupItem, "W4", "", "", -1, TRUE);
		oper.InsertEditPropItem(pGroupItem, "W5", "", "", -1, TRUE);
		oper.InsertEditPropItem(pGroupItem, "Y1", "", "", -1, TRUE);
		oper.InsertEditPropItem(pGroupItem, "Y2", "", "", -1, TRUE);
		oper.InsertEditPropItem(pGroupItem, "Y3", "", "", -1, TRUE);
		oper.InsertEditPropItem(pGroupItem, "Z1", "", "", -1, TRUE);
		oper.InsertEditPropItem(pGroupItem, "Z2", "", "", -1, TRUE);
		oper.InsertEditPropItem(pGroupItem, "Z3", "", "", -1, TRUE);
		oper.InsertEditPropItem(pGroupItem, "Z4", "", "", -1, TRUE);
		oper.InsertEditPropItem(pGroupItem, "Z5", "", "", -1, TRUE);
	}
	pPropList->Redraw();
}
