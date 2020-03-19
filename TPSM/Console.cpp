//<LOCALE_TRANSLATE Confirm by hxr/>
#include "stdafx.h"
#include "TPSMModel.h"
#include "Block.h"
#include "console.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//全局操控类对象
CConsole console;
void BLK_RELATIVEOBJS::Empty()
{
	hashNodePtr.Empty();
	hashPartPtr.Empty();
	hashPointPtr.Empty();
	hashLinePtr.Empty();
	hashPlanePtr.Empty();
}
CConsole::CConsole(void)
{
	m_pModel=NULL;
	m_pWorkPart = NULL;
	m_hActiveDisplayView=0;
	defaultView.m_iViewType=0;
	defaultView.ucs.axis_x.Set(1,0,0);
	defaultView.ucs.axis_y.Set(0,0,-1);
	defaultView.ucs.axis_z.Set(0,1,0);
	DispNodeSet.Empty();
	DispPartSet.Empty();
	DispBlockRefSet.Empty();
	DispFoundationSet.Empty();
}

CConsole::~CConsole(void)
{
}

bool CConsole::ActivateModel(IModel *pModel,long hView/*=0*/)
{
	m_pModel=pModel;
	defaultView.SetBelongModel(pModel);
#ifndef __DRAWING_CONTEXT_
	if(hView==0)
	{	//当前模型为导入式部件时，默认视图坐标系为部件定位坐标系
		//当前模型为非导入时部件时，默认视图坐标系为铁塔透视图默认坐标系	wht 13-04-16
		if(pModel->IsTowerModel())
		{
			console.GetActiveView()->ucs.origin.Set();
			console.GetActiveView()->ucs.axis_x.Set(1,0,0);
			console.GetActiveView()->ucs.axis_y.Set(0,0,-1);
			console.GetActiveView()->ucs.axis_z.Set(0,1,0);
		}
		else 
			console.GetActiveView()->ucs=((CBlockModel*)pModel)->GetLCS();
	}
	/*if(g_pSolidSet!=NULL)
	{
		if(pModel->IsTowerModel())
			g_pSolidSet->SetActiveDisplayBuffer(0);
		else
			g_pSolidSet->SetActiveDisplayBuffer(((CBlockModel*)m_pModel)->handle);
	}*/
#endif
	SetActiveView(hView);
	return true;
}
long CConsole::ActiveModuleHandle()
{
	if(m_pModel&&m_pModel->IsTowerModel())
		return ((CTower*)m_pModel)->m_hActiveModule;
	else
		return 0;
}

CParaCS *CConsole::AppendAcs()
{
	if(m_pModel)
		return m_pModel->AppendAcs();
	else
		return NULL;
}
BOOL CConsole::DeleteAcs(long hAcs)
{
	if(m_pModel)
		return m_pModel->DeleteAcs(hAcs);
	else
		return FALSE;
}
CParaCS *CConsole::FromAcsHandle(long hAcs)
{
	if(m_pModel)
		return m_pModel->FromAcsHandle(hAcs);
	else
		return NULL;
}

CLDSModule* CConsole::GetActiveModule()
{
	if(m_pModel&&m_pModel->IsTowerModel())
		return ((CTower*)m_pModel)->GetActiveModule();
	else
		return NULL;
}
#ifdef __PART_DESIGN_INC_
CWorkStatus* CConsole::GetActiveWorkStatus()
{
	if(m_pModel&&m_pModel->IsTowerModel())
		return ((CTower*)m_pModel)->GetActiveWorkStatus();
	else
		return NULL;
}
#endif
GEPOINT CConsole::TransPFromScr(const double* point)
{
	GEPOINT dynScrPoint(point);
#if defined(__LDS_CONTEXT_)||defined(__LDS_GROUP_CONTEXT_)
	if(g_pSolidOper)
		dynScrPoint=g_pSolidOper->TransPFromScr(point);
#endif
	return dynScrPoint;
}
GEPOINT CConsole::TransVFromScr(const double* vector)
{
	GEPOINT dynScrVect(vector);
#if defined(__LDS_CONTEXT_)||defined(__LDS_GROUP_CONTEXT_)
	if(g_pSolidOper)
		dynScrVect=g_pSolidOper->TransVFromScr(vector);
#endif
	return dynScrVect;
}
CDisplayView *CConsole::SetActiveView(long hView)
{
	if(m_pModel==NULL)
		return NULL;
	CDisplayView* pActiveDisplayView=m_pModel->FromViewHandle(hView);
	if(pActiveDisplayView==NULL)
	{
		//不能以第一个视图为默认视图，否则会导致展开某一平面时，错把展开面绘图坐标系赋值给透视图 wjh-2012.12.21
		//if(m_pModel->GetObjNum(CLS_DISPLAYVIEW)>0)
		//	pActiveDisplayView=m_pModel->EnumViewFirst();
		//else
		pActiveDisplayView=&defaultView;
	}
	m_hActiveDisplayView=pActiveDisplayView->handle;
	return pActiveDisplayView;
}
CDisplayView *CConsole::GetActiveView()
{
	return SetActiveView(m_hActiveDisplayView);
}
#ifndef __DRAWING_CONTEXT_
void CConsole::Display(CDisplayView* pView)
{
	if(pView==NULL)
		pView=GetActiveView();
	pView->SelectDispSet(DispNodeSet,DispPartSet,DispBlockRefSet,DispFoundationSet);
	//if(g_pSolidDraw==NULL||g_pSolidOper==NULL)
	//	return;
	//g_pSolidDraw->BuildDisplayList();
	//g_pSolidOper->ZoomAll(0.95);
	//g_pSolidOper->ReSetView();
}
#endif
BOOL CConsole::DeleteDisplayView(long hView)
{
	if(m_hActiveDisplayView==hView)
		return FALSE;
	else
		return m_pModel->DeleteView(hView);
}

CLDSPoint* CConsole::AppendPoint(BOOL bGetNewHandle/*=TRUE*/)
{
	if(m_pModel==NULL)
		return NULL;
	return m_pModel->AppendPoint(bGetNewHandle);
}

CLDSLine* CConsole::AppendLine(BOOL bGetNewHandle/*=TRUE*/)
{
	if(m_pModel==NULL)
		return NULL;
	return m_pModel->AppendLine(bGetNewHandle);
}

CLDSPlane* CConsole::AppendPlane(BOOL bGetNewHandle/*=TRUE*/)
{
	if(m_pModel==NULL)
		return NULL;
	return m_pModel->AppendPlane(bGetNewHandle);
}

CDisplayView* CConsole::AppendView(BOOL bGetNewHandle/*=TRUE*/)
{
	if(m_pModel==NULL)
		return NULL;
	return m_pModel->AppendView(bGetNewHandle);
}

CLDSNode* CConsole::AppendNode(BOOL bGetNewHandle/*=TRUE*/)
{
	if(m_pModel==NULL)
		return NULL;
	return m_pModel->AppendNode(bGetNewHandle);
}

CLDSPart* CConsole::AppendPart(int CLASS_TYPEID,BOOL bGetNewHandle/*=TRUE*/)
{
	if(m_pModel==NULL)
		return NULL;
	return m_pModel->AppendPart(CLASS_TYPEID,bGetNewHandle);
}
BOOL CConsole::DeleteNode(long h,BOOL bIncPerm/*=TRUE*/)
{
	DispNodeSet.DeleteNode(h);
	//g_pSolidDraw->DelEnt(h);
	if(m_pModel!=NULL)
		return m_pModel->DeleteNode(h,bIncPerm);
	return FALSE;
}
BOOL CConsole::DeletePart(long h,BOOL bIncPerm/*=TRUE*/)
{
	DispPartSet.DeleteNode(h);
	//g_pSolidDraw->DelEnt(h);
	if(m_pModel==NULL)
		return FALSE;
#ifndef __DRAWING_CONTEXT_
	//IDrawing* pBuddyDrawing=g_pSolidSet->GetSolidBuddyDrawing();
	//if(pBuddyDrawing)
	//{
	//	if((pAngle=(CLDSLineAngle*)m_pModel->FromPartHandle(h,CLS_LINEANGLE,CLS_GROUPLINEANGLE))!=NULL)
	//		pBuddyDrawing->DeleteDbEntityByHiberMasterId(pAngle->handle);
	//	if((pPlate=(CLDSGeneralPlate*)m_pModel->FromPartHandle(h,CLS_PLATE,CLS_PARAMPLATE))!=NULL)
	//		pBuddyDrawing->DeleteDbEntityByHiberMasterId(pPlate->handle);
	//}
#endif
	hashHoles.DeleteNode(h);
	return m_pModel->DeletePart(h,bIncPerm);
}
BOOL CConsole::DeleteView(long h,BOOL bIncPerm/*=TRUE*/)
{
	if(m_pModel==NULL)
		return FALSE;
	return m_pModel->DeleteView(h,bIncPerm);
}
CLDSModule* CConsole::InitTowerModel()
{
	if(m_pModel==NULL||!m_pModel->IsTowerModel())
		return NULL;
	CTower *pTower=(CTower*)m_pModel;
	CLDSModule* pModule=pTower->InitModule();
	/*if(pTower->View.GetNodeNum()>0)
		m_pActiveDisplayView=pTower->View.GetFirst();
	else 
		m_pActiveDisplayView=&defaultView;*/
	return pModule;
}
bool CConsole::EmptyAllDispObjList()	//清空所有显示对象列表
{
	DispNodeSet.Empty();//NODESET DispNodeSet;
	DispPartSet.Empty();//PARTSET DispPartSet;
	DispBlockRefSet.Empty();
	DispFoundationSet.Empty();
	return true;
}
HOLE_WALL* CConsole::MakeHoleWall(CLDSBolt* pBolt, CLDSPart* pResidePart)
{
	if (pResidePart == NULL || (!pResidePart->IsAngle()&&!pResidePart->IsPlate()))
		return NULL;
	HOLE_WALL* pHoleWall=hashHoles.Add(DUALKEY(pBolt->handle, pResidePart->handle));
	pHoleWall->pBolt = pBolt;

	GEPOINT inters;
	int liResidePlane=0;
	pResidePart->GetBoltIntersPos(pBolt, inters, &liResidePlane);
	SmartPartPtr pSmartPart = pResidePart;
	GEPOINT vWorkNormal;
	if (pSmartPart->IsAngle())
	{
		int wingX0Y1 = liResidePlane;
		vWorkNormal = wingX0Y1 == 0 ?-pSmartPart.pAngle->vxWingNorm :-pSmartPart.pAngle->vyWingNorm;
	}
	else if (pSmartPart->IsPlate())
	{
		if (liResidePlane == 1||liResidePlane==2)
			vWorkNormal = pSmartPart.pBasicPlate->GetHuoquFaceNorm(liResidePlane-1);
		else
			vWorkNormal = pSmartPart.pBasicPlate->ucs.axis_z;
	}
	if (pBolt->ucs.axis_z*vWorkNormal > 0)
	{
		pHoleWall->vStart = inters;
		pHoleWall->vEnd = inters+vWorkNormal * pSmartPart->Thick;
	}
	else
	{
		pHoleWall->vEnd = inters;
		pHoleWall->vStart = inters+vWorkNormal * pSmartPart->Thick;
	}
	pHoleWall->items[0].ptS = pHoleWall->vStart;
	pHoleWall->items[0].ptE = pHoleWall->vEnd;
	for (int j = 0; j < 4; j++)
	{
		if (pHoleWall->items[j].hRelaPart == pSmartPart->handle)
			break;
		else if (pHoleWall->items[j].hRelaPart == 0)
		{
			pHoleWall->items[j].hRelaPart = pSmartPart->handle;
			break;
		}
	}
	pHoleWall->is_visible = TRUE;
	pHoleWall->Create3dSolidModel();
	return pHoleWall;
}
//删除事件
void CConsole::FireDeleteObject(CLDSObject *pObj,BOOL bDelFromSolidDraw/*=FALSE*/)
{
	if(pObj==NULL)
		return;
	if(!pObj->IsDbObject())
		return;
	if(pObj==m_pWorkPart)
		m_pWorkPart=NULL;
	CLDSDbObject *pDbObj=(CLDSDbObject*)pObj;
#ifdef __LDS_CONTEXT_
	if(bDelFromSolidDraw && g_pSolidDraw)
		g_pSolidDraw->DelEnt(pDbObj->handle);
#endif
	if(pDbObj->GetClassTypeId()==CLS_NODE)
	{
		for(CLDSNode* pDispNode=DispNodeSet.GetFirst();pDispNode;pDispNode=DispNodeSet.GetNext())
		{
			if(pDispNode==pObj)
				DispNodeSet.DeleteCursor();
		}
		DispNodeSet.Clean();
	}
	else if(pDbObj->IsPart())
	{
		for(CLDSPart* pDispPart=DispPartSet.GetFirst();pDispPart;pDispPart=DispPartSet.GetNext())
		{
			if(pDispPart==pDbObj)
				DispPartSet.DeleteCursor();
		}
		DispPartSet.Clean();
		for (HOLE_WALL* pHoleWall=hashHoles.GetFirst();pHoleWall;pHoleWall=hashHoles.GetNext())
		{
			if (pHoleWall->hBolt==pObj->handle||pHoleWall->items[0].hRelaPart==pObj->handle)
			{
				hashHoles.DeleteCursor();
#ifdef __LDS_CONTEXT_
				if(g_pSolidDraw)
				{
					OBJECT_HANDLER objhandler;
					HIBERID hiberid(pHoleWall->hBolt);
					hiberid.SetHiberarchy(1,pHoleWall->items[0].hRelaPart);	//设定关联构件，即孔壁宿主构件
					UINT idSolidObj=g_pSolidDraw->QueryBuffSolidObject(hiberid);
					g_pSolidDraw->DelEnt(idSolidObj);
				}
#endif
			}
		}
	}
	return;
}
bool CConsole::AddToObjSetBuf(CLDSObject* pObj)
{
	if(pObj->GetClassTypeId()==CLS_GEPOINT)
	{
		if(relaObjs.hashPointPtr.GetValue(pObj->handle)!=NULL)
			return false;
		else
			relaObjs.hashPointPtr.SetValue(pObj->handle,(CLDSPoint*)pObj);
	}
	else if(pObj->GetClassTypeId()==CLS_GELINE)
	{
		if(relaObjs.hashLinePtr.GetValue(pObj->handle)!=NULL)
			return false;
		else
			relaObjs.hashLinePtr.SetValue(pObj->handle,(CLDSLine*)pObj);
	}
	else if(pObj->GetClassTypeId()==CLS_GEPLANE)
	{
		if(relaObjs.hashPlanePtr.GetValue(pObj->handle)!=NULL)
			return false;
		else
			relaObjs.hashPlanePtr.SetValue(pObj->handle,(CLDSPlane*)pObj);
	}
	else if(pObj->GetClassTypeId()==CLS_NODE)
	{
		if(relaObjs.hashNodePtr.GetValue(pObj->handle)!=NULL)
			return false;
		else
			relaObjs.hashNodePtr.SetValue(pObj->handle,(CLDSNode*)pObj);
	}
	else if(pObj->IsDbObject()&&((CLDSDbObject*)pObj)->IsPart())
	{
		if(relaObjs.hashPartPtr.GetValue(pObj->handle)!=NULL)
			return false;
		else
			relaObjs.hashPartPtr.SetValue(pObj->handle,(CLDSPart*)pObj);
	}
	return true;
}
struct CROSSNODEEX{
	long hNode,hRod,hCrossRod;
	//void SetKey(DWORD key){hNode=key;}
	CROSSNODEEX(){hNode=hRod=hCrossRod=0;}
};
bool CConsole::FillBlock(CBlockModel* pBlock)
{
#ifndef __DRAWING_CONTEXT_
	CLDSPoint* pPoint,**ppPoint;
	CLDSLine*  pLine,**ppLine;
	CLDSPlane* pPlane,**ppPlane;
	CLDSNode *pNode;
	CLDSPart *pPart;
	CROSSNODEEX* pCrossNode;
	CHashList<CLDSNodePtr>hashNodePtr;
	CHashList<CLDSPart*>hashPartPtr;
	CHashList<CLDSPoint*>hashPointPtr;
	CHashList<CLDSLine*>hashLinePtr;
	CHashList<CLDSPlane*>hashPlanePtr;
	CXhSimpleList<CROSSNODEEX> listCrossNodes;
	for(pPoint=m_pModel->EnumPointFirst();pPoint;pPoint=m_pModel->EnumPointNext())
		pPoint->Unlock();
	for(pLine=m_pModel->EnumLineFirst();pLine;pLine=m_pModel->EnumLineNext())
		pLine->Unlock();
	for(pPlane=m_pModel->EnumPlaneFirst();pPlane;pPlane=m_pModel->EnumPlaneNext())
		pPlane->Unlock();
	for(pNode=m_pModel->EnumNodeFirst();pNode;pNode=m_pModel->EnumNodeNext())
	{
		pNode->Unlock();
		if(pNode->m_cPosCalType==4&&pNode->m_cPosCalViceType==0&&(pNode->arrRelationPole[0]>0x20&&pNode->arrRelationPole[1]>0x20))
		{
			pCrossNode=listCrossNodes.AttachObject();
			pCrossNode->hNode=pNode->handle;
			pCrossNode->hRod=pNode->arrRelationPole[0];
			pCrossNode->hCrossRod=pNode->arrRelationPole[1];
		}
	}
	for(pPart=m_pModel->EnumPartFirst();pPart;pPart=m_pModel->EnumPartNext())
		pPart->Unlock();
	relaObjs.Empty();
	long* id_arr=NULL;
	long nSel = 0;// g_pSolidSnap->GetLastSelectEnts(id_arr);
	for(long i=0;i<nSel;i++)
	{
		pPart=m_pModel->FromPartHandle(id_arr[i]);
		if(pPart!=NULL)
			pPart->AddToConsoleObjBuf(true,false);
		else
		{
			pNode=m_pModel->FromNodeHandle(id_arr[i]);
			if(pNode)
				pNode->AddToConsoleObjBuf(true,false);
		}
	}
	for(ppPoint=relaObjs.hashPointPtr.GetFirst();ppPoint;ppPoint=relaObjs.hashPointPtr.GetNext())
		(*ppPoint)->m_hBlock=pBlock->handle;
	for(ppLine=relaObjs.hashLinePtr.GetFirst();ppLine;ppLine=relaObjs.hashLinePtr.GetNext())
		(*ppLine)->m_hBlock=pBlock->handle;
	for(ppPlane=relaObjs.hashPlanePtr.GetFirst();ppPlane;ppPlane=relaObjs.hashPlanePtr.GetNext())
		(*ppPlane)->m_hBlock=pBlock->handle;
	for(pNode=relaObjs.hashNodePtr.GetFirst();pNode;pNode=relaObjs.hashNodePtr.GetNext())
		pNode->m_hBlock=pBlock->handle;
	for(pPart=relaObjs.hashPartPtr.GetFirst();pPart;pPart=relaObjs.hashPartPtr.GetNext())
		pPart->m_hBlock=pBlock->handle;
	for(pCrossNode=listCrossNodes.EnumObjectFirst();pCrossNode;pCrossNode=listCrossNodes.EnumObjectNext())
	{
		if(relaObjs.hashPartPtr.GetValue(pCrossNode->hRod)!=NULL&&relaObjs.hashPartPtr.GetValue(pCrossNode->hCrossRod)!=NULL)
		{
			if((pNode=m_pModel->FromNodeHandle(pCrossNode->hNode))!=NULL)
				pNode->m_hBlock=pBlock->handle;
		}
	}
	CDisplayView* pPerspectView=pBlock->AppendView();
	pPerspectView->m_iViewType=0;
#ifdef AFX_TARG_ENU_ENGLISH
	strcpy(pPerspectView->name,"perspect view");
#else
	strcpy(pPerspectView->name,"透视图");
#endif
	strcpy(pPerspectView->defaultLayer,"SPQ");
	strcpy(pPerspectView->filter.FirstCharSet,"?");
	strcpy(pPerspectView->filter.SecondCharSet,"?");
	strcpy(pPerspectView->filter.ThirdCharSet,"?");
	pPerspectView->ucs.axis_x.Set(1,0,0);
	pPerspectView->ucs.axis_y.Set(0,0,-1);
	pPerspectView->ucs.axis_z.Set(0,1,0);

#endif
	return true;
}
CLDSNode* CConsole::EnumBlkBufNodeFirst()
{
	return relaObjs.hashNodePtr.GetFirst();
}
CLDSNode* CConsole::EnumBlkBufNodeNext()
{
	return relaObjs.hashNodePtr.GetNext();
}
CLDSPart* CConsole::EnumBlkBufPartFirst(bool bOnlyRod/*=false*/)
{
	for(SmartPartPtr pSmartPart=relaObjs.hashPartPtr.GetFirst();pSmartPart.IsHasPtr();pSmartPart=relaObjs.hashPartPtr.GetNext())
	{
		if(bOnlyRod&&!pSmartPart->IsLinePart())
			continue;
		return pSmartPart;
	}
	return NULL;
}
CLDSPart* CConsole::EnumBlkBufPartNext(bool bOnlyRod/*=false*/)
{
	for(SmartPartPtr pSmartPart=relaObjs.hashPartPtr.GetNext();pSmartPart.IsHasPtr();pSmartPart=relaObjs.hashPartPtr.GetNext())
	{
		if(bOnlyRod&&!pSmartPart->IsLinePart())
			continue;
		return pSmartPart;
	}
	return NULL;
}

//获取杆塔模型空间中直接管理并实际存在的构件数组，filterShadowPart=true表示只计入母本构件，影射构件将被怱略
bool CConsole::GetRealityPartArr(DYN_ARRAY<CLDSBolt*>& arrBoltPtr,DYN_ARRAY<CLDSPartPtr>& arrPartPtr,void *pPolyPartList,bool filterShadowPart)
{
	if(m_pModel==NULL)
		return NULL;
	return m_pModel->GetRealityPartArr(arrBoltPtr,arrPartPtr,pPolyPartList,filterShadowPart);
}
CLDSObject* CConsole::FromHandle(long h,BOOL bDbObjOnly/*=FALSE*/)
{
	if(m_pModel==NULL)
		return NULL;
	return m_pModel->FromHandle(h,bDbObjOnly);
}
CDisplayView* CConsole::FromViewHandle(long h)
{
	if(m_pModel==NULL)
		return NULL;
	return m_pModel->FromViewHandle(h);
}
CLDSPoint* CConsole::FromPointHandle(long h)
{
	if(m_pModel==NULL)
		return NULL;
	return m_pModel->FromPointHandle(h);
}
CLDSLine* CConsole::FromLineHandle(long h)
{
	if(m_pModel==NULL)
		return NULL;
	return m_pModel->FromLineHandle(h);
}
CLDSPlane* CConsole::FromPlaneHandle(long h)
{
	if(m_pModel==NULL)
		return NULL;
	return m_pModel->FromPlaneHandle(h);
}
CLDSNode* CConsole::FromNodeHandle(long h)
{
	if(m_pModel==NULL)
		return NULL;
	return m_pModel->FromNodeHandle(h);
}
CLDSPart* CConsole::FromPartHandle(long h,int CLASS_TYPEID/*=0*/)
{
	if(m_pModel==NULL)
		return NULL;
	return m_pModel->FromPartHandle(h,CLASS_TYPEID);
}
CLDSLinePart* CConsole::FromRodHandle(long hRod)
{
	if(m_pModel==NULL)
		return NULL;
	return m_pModel->FromRodHandle(hRod);
}
CLDSPart* CConsole::FromPartHandle(long h,int CLASS_TYPEID1,int CLASS_TYPEID2)	//根据句柄获得节点
{
	if(m_pModel==NULL)
		return NULL;
	return m_pModel->FromPartHandle(h,CLASS_TYPEID1,CLASS_TYPEID2);
}
CLDSPart* CConsole::FromPartHandle(long h,int CLASS_TYPEID1,int CLASS_TYPEID2,int CLASS_TYPEID3)	//根据句柄获得节点
{
	if(m_pModel==NULL)
		return NULL;
	return m_pModel->FromPartHandle(h,CLASS_TYPEID1,CLASS_TYPEID2,CLASS_TYPEID3);
}
CLDSArcLift* CConsole::FromArcLiftHandle(long h)
{
	if(m_pModel==NULL)
		return NULL;
	return m_pModel->FromArcLiftHandle(h);
}
//约束相关的函数
CONSTRAINT* CConsole::AppendConstraint(CONSTRAINT* pCopyFrom/*=NULL*/)
{
	if(m_pModel==NULL)
		return NULL;
	return m_pModel->AppendConstraint(pCopyFrom);
}
CONSTRAINT* CConsole::GetConstraint(long constraint_id)
{
	if(m_pModel==NULL)
		return NULL;
	return m_pModel->GetConstraint(constraint_id);
}
bool CConsole::RemoveConstraint(long constraint_id,long hFromMasterObj/*=0*/)
{
	if(m_pModel==NULL)
		return false;
	return m_pModel->RemoveConstraint(constraint_id,hFromMasterObj);
}
CONSTRAINT* CConsole::EnumConstraintFirst(BOOL bIterDelete/*=FALSE*/)
{
	if(m_pModel==NULL)
		return NULL;
	return m_pModel->EnumConstraintFirst(bIterDelete);
}
CONSTRAINT* CConsole::EnumConstraintNext(BOOL bIterDelete/*=FALSE*/)
{
	if(m_pModel==NULL)
		return NULL;
	return m_pModel->EnumConstraintNext(bIterDelete);
}
void CConsole::ClearConstraintSyncStates()
{
	if(m_pModel!=NULL)
		m_pModel->ClearConstraintSyncStates();
}
long CConsole::GetNewHandle()
{
	if(m_pModel==NULL)
		return 0;
	return m_pModel->GetNewHandle();
}
bool CConsole::IsTowerModel()
{
	if(m_pModel==NULL)
		return false;
	return m_pModel->IsTowerModel();
}
long CConsole::GetObjNum(int CLASS_TYPEID)
{
	if(m_pModel==NULL)
		return 0;
	return m_pModel->GetObjNum(CLASS_TYPEID);
}
CDisplayView* CConsole::EnumViewFirst(BOOL bIterDelete/*=FALSE*/)
{
	if(m_pModel==NULL)
		return NULL;
	return m_pModel->EnumViewFirst(bIterDelete);
}
CDisplayView* CConsole::EnumViewNext(BOOL bIterDelete/*=FALSE*/)
{
	if(m_pModel==NULL)
		return NULL;
	return m_pModel->EnumViewNext(bIterDelete);
}
CLDSNode* CConsole::EnumNodeFirst(BOOL bIterDelete/*=FALSE*/)
{
	if(m_pModel==NULL)
		return NULL;
	if(m_pModel->IsTowerModel())
		return ((CTower*)m_pModel)->Node.GetFirst(bIterDelete);
	else 
		return ((CBlockModel*)m_pModel)->Node.GetFirst(bIterDelete);
}
CLDSNode* CConsole::EnumNodeNext(BOOL bIterDelete/*=FALSE*/)
{
	if(m_pModel==NULL)
		return NULL;
	if(m_pModel->IsTowerModel())
		return ((CTower*)m_pModel)->Node.GetNext(bIterDelete);
	else 
		return ((CBlockModel*)m_pModel)->Node.GetNext(bIterDelete);
}
CLDSPart* CConsole::EnumPartFirst(int CLASS_TYPEID/*=0*/,BOOL bIterDelete/*=FALSE*/)
{
	if(m_pModel==NULL)
		return NULL;
	if(m_pModel->IsTowerModel())
		return ((CTower*)m_pModel)->Parts.GetFirst(CLASS_TYPEID,bIterDelete);
	else 
		return ((CBlockModel*)m_pModel)->Parts.GetFirst(CLASS_TYPEID,bIterDelete);

}
CLDSPart* CConsole::EnumPartNext(int CLASS_TYPEID/*=0*/,BOOL bIterDelete/*=FALSE*/)
{
	if(m_pModel==NULL)
		return NULL;
	if(m_pModel->IsTowerModel())
		return ((CTower*)m_pModel)->Parts.GetNext(CLASS_TYPEID,bIterDelete);
	else 
		return ((CBlockModel*)m_pModel)->Parts.GetNext(CLASS_TYPEID,bIterDelete);
}
CLDSNode* CConsole::MoveToTailNode(BOOL bIterDelete/* = FALSE*/) 
{
	return m_pModel->MoveToTailNode(bIterDelete); 
}
CLDSPart* CConsole::MoveToTailPart(int CLASS_TYPEID/*=0*/,BOOL bIterDelete/*=FALSE*/)
{
	return m_pModel->MoveToTailPart(CLASS_TYPEID,bIterDelete);
}
CLDSLinePart* CConsole::EnumRodFirst()
{
	if(m_pModel==NULL)
		return NULL;
	if(m_pModel->IsTowerModel())
		return ((CTower*)m_pModel)->Parts.GetFirstLinePart();
	else 
		return ((CBlockModel*)m_pModel)->Parts.GetFirstLinePart();
}
CLDSLinePart* CConsole::EnumRodNext()
{
	if(m_pModel==NULL)
		return NULL;
	if(m_pModel->IsTowerModel())
		return ((CTower*)m_pModel)->Parts.GetNextLinePart();
	else 
		return ((CBlockModel*)m_pModel)->Parts.GetNextLinePart();
}
CLDSPoint* CConsole::EnumPointFirst(BOOL bIterDelete/*=FALSE*/)
{
	if(m_pModel==NULL)
		return NULL;
	if(m_pModel->IsTowerModel())
		return ((CTower*)m_pModel)->Point.GetFirst(bIterDelete);
	else 
		return ((CBlockModel*)m_pModel)->Point.GetNext(bIterDelete);
}
CLDSPoint* CConsole::EnumPointNext(BOOL bIterDelete/*=FALSE*/)
{
	if(m_pModel==NULL)
		return NULL;
	if(m_pModel->IsTowerModel())
		return ((CTower*)m_pModel)->Point.GetNext(bIterDelete);
	else 
		return ((CBlockModel*)m_pModel)->Point.GetNext(bIterDelete);
}
CLDSLine* CConsole::EnumLineFirst(BOOL bIterDelete/*=FALSE*/)
{
	if(m_pModel==NULL)
		return NULL;
	if(m_pModel->IsTowerModel())
		return ((CTower*)m_pModel)->Line.GetFirst(bIterDelete);
	else 
		return ((CBlockModel*)m_pModel)->Line.GetNext(bIterDelete);
}
CLDSLine* CConsole::EnumLineNext(BOOL bIterDelete/*=FALSE*/)
{
	if(m_pModel==NULL)
		return NULL;
	if(m_pModel->IsTowerModel())
		return ((CTower*)m_pModel)->Line.GetNext(bIterDelete);
	else 
		return ((CBlockModel*)m_pModel)->Line.GetNext(bIterDelete);
}
CLDSPlane* CConsole::EnumPlaneFirst(BOOL bIterDelete/*=FALSE*/)
{
	if(m_pModel==NULL)
		return NULL;
	if(m_pModel->IsTowerModel())
		return ((CTower*)m_pModel)->Plane.GetFirst(bIterDelete);
	else 
		return ((CBlockModel*)m_pModel)->Plane.GetNext(bIterDelete);
}
CLDSPlane* CConsole::EnumPlaneNext(BOOL bIterDelete/*=FALSE*/)
{
	if(m_pModel==NULL)
		return NULL;
	if(m_pModel->IsTowerModel())
		return ((CTower*)m_pModel)->Plane.GetNext(bIterDelete);
	else 
		return ((CBlockModel*)m_pModel)->Plane.GetNext(bIterDelete);
}
int CConsole::PushNodeStack()
{
	if(m_pModel==NULL)
		return 0;
	return m_pModel->PushNodeStack();
}
bool CConsole::PopNodeStack(int pos/*=-1*/)
{
	if(m_pModel==NULL)
		return false;
	return m_pModel->PopNodeStack(pos);
}
int CConsole::PushPartStack()
{
	if(m_pModel==NULL)
		return 0;
	return m_pModel->PushPartStack();
}
bool CConsole::PopPartStack(int pos/*=-1*/)
{
	if(m_pModel==NULL)
		return false;
	return m_pModel->PopPartStack(pos);
}
#ifdef __COMMON_PART_INC_
CLDSPartGroup* CConsole::EnumPartGroupFirst()
{
	if(m_pModel==NULL)
		return NULL;
	return m_pModel->EnumPartGroupFirst();
}
CLDSPartGroup* CConsole::EnumPartGroupNext()
{
	if(m_pModel==NULL)
		return NULL;
	return m_pModel->EnumPartGroupNext();
}
int CConsole::GetPartGroupPartSet(long hPartGroup,CTmaPtrList<CLDSPart,CLDSPart*> &partset,int CLASS_TYPEID/*=0*/,bool bClearLock/*=true*/,
								  long* phExcludePartBranchArr/*=NULL*/,int count/*=1*/)
{
	if(m_pModel==NULL)
		return NULL;
	return m_pModel->GetPartGroupPartSet(hPartGroup,partset,CLASS_TYPEID,bClearLock,phExcludePartBranchArr,count);
}
long CConsole::GetPartGroupNum()
{
	if(m_pModel==NULL)
		return 0;
	return m_pModel->GetPartGroupNum();
}
#endif
