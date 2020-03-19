#pragma once
#include "IModel.h"
#include "Tower.h"
#include "ldsptr_list.h"
#include "lds_part.h"
#include "Block.h"
//用户交互过程中的后台操作控制台
class CLDSNode;
class CLDSPart;
class BLK_RELATIVEOBJS
{
public:
	CHashSet<CLDSNodePtr>hashNodePtr;
	CHashSet<CLDSPart*>hashPartPtr;
	CHashList<CLDSPoint*>hashPointPtr;
	CHashList<CLDSLine*>hashLinePtr;
	CHashList<CLDSPlane*>hashPlanePtr;
	void Empty();
};
class CConsole : public IConsole
{
	IModel *m_pModel;
	long m_hActiveDisplayView;
	CDisplayView defaultView;
	BLK_RELATIVEOBJS relaObjs;	//临时存储部件块内的关联构件集合
public:
	CConsole(void);
	virtual ~CConsole(void);
	CTmaPtrList<CLDSNode,CLDSNode*> DispNodeSet;//NODESET DispNodeSet;
	CTmaPtrList<CLDSPart,CLDSPart*> DispPartSet;//PARTSET DispPartSet;
	CTmaPtrList<CBlockReference,CBlockReference*> DispBlockRefSet;
	CTmaPtrList<CLDSFoundation,CLDSFoundation*> DispFoundationSet;
	CHash64ListEx<HOLE_WALL> hashHoles;	//需要将孔壁单独赋予螺栓标识属性的实体对象
	CLDSPart *m_pWorkPart;	//当前要编辑的工作零件
	CBuffer clipper_board;		//剪贴板
	UCS_STRU m_curACS;			//当前装配坐标系(Assemble Coordinate System)
	UCS_STRU m_curWCS;			//当前工作坐标系(Work Coordinate System)
	CParaCS des_wcs;			//工作坐标系的设计参数
public:
	virtual IModel* GetActiveModel(){return m_pModel;}
#ifndef __DRAWING_CONTEXT_
	void Display(CDisplayView* pView);
#endif
	virtual bool ActivateModel(IModel *pModel,long hView=0);
	//显示视图操作函数
	virtual CDisplayView *SetActiveView(long hView);
	virtual CDisplayView *GetActiveView();
	virtual BOOL DeleteDisplayView(long hView);
	virtual long ActiveModuleHandle();
	virtual CLDSModule* GetActiveModule();
#ifdef __PART_DESIGN_INC_
	virtual CWorkStatus* GetActiveWorkStatus();
#endif
	virtual UCS_STRU WCS(){return m_curWCS;}	//当前工作坐标系
	virtual GEPOINT TransPFromScr(const double* point);
	virtual GEPOINT TransVFromScr(const double* vector); 
	virtual bool AddToObjSetBuf(CLDSObject* pObj);	//添加对象到控制台的部件缓存中
	virtual HOLE_WALL* GetHoleWall(CLDSBolt* pBolt,long hResidePart=0)
	{
		if (pBolt == NULL)
			return NULL;
		if (hResidePart == 0)
		{
			CLDSPart* ResidePart =FromPartHandle(pBolt->des_base_pos.hPart);
			hResidePart = ResidePart !=NULL ? ResidePart->handle : 0;
		}
		return hashHoles.GetValue(DUALKEY(pBolt->handle, hResidePart));
	}
	HOLE_WALL* MakeHoleWall(CLDSBolt* pBolt, CLDSPart* pResidePart);
	virtual CLDSNode* AddToDispSet(CLDSNode* pNode){
		CLDSNode* pDispNode=DispNodeSet.FromHandle(pNode->handle);
		if(pDispNode==NULL)
			pDispNode=DispNodeSet.append(pNode);
		return pDispNode;
	}
	virtual CLDSPart* AddToDispSet(CLDSPart* pPart){
		CLDSPart* pDispPart=DispPartSet.FromHandle(pPart->handle);
		if(pDispPart==NULL)
			pDispPart=DispPartSet.append(pPart);
		return pDispPart;
	}
	virtual NODESET& GetDispNodeSet(){return DispNodeSet;}
	virtual PARTSET& GetDispPartSet(){return DispPartSet;}
	virtual CLDSNode* EnumFirstDispNode(){return DispNodeSet.GetFirst();}
	virtual CLDSNode* EnumNextDispNode(){return DispNodeSet.GetNext();}
	virtual CLDSPart* EnumFirstDispPart(){return DispPartSet.GetFirst();}
	virtual CLDSPart* EnumNextDispPart(){return DispPartSet.GetNext();}
	virtual void EmptyDispNode(){DispNodeSet.Empty();}
	virtual void EmptyDispPart(){DispPartSet.Empty();}
	virtual bool EmptyAllDispObjList();	//清空所有显示对象列表
	//删除事件
	virtual void FireDeleteObject(CLDSObject *pObj,BOOL bDelFromSolidDraw=FALSE);
public:
	CLDSModule* InitTowerModel();
	//CLdsList<CDisplayView> View;	//显示视图
	//CLdsList<CLDSPoint> Point;			//基准点
	//CLdsList<CLDSLine>  Line;			//基准线
	//CLdsList<CLDSPlane> Plane;			//基准平面
	virtual CParaCS *AppendAcs();	//增加装配坐标系
	virtual CLDSPoint* AppendPoint(BOOL bGetNewHandle=TRUE);
	virtual CLDSLine* AppendLine(BOOL bGetNewHandle=TRUE);
	virtual CLDSPlane* AppendPlane(BOOL bGetNewHandle=TRUE);
	virtual CDisplayView* AppendView(BOOL bGetNewHandle=TRUE);
	virtual CLDSNode* AppendNode(BOOL bGetNewHandle=TRUE);
	virtual CLDSPart* AppendPart(int CLASS_TYPEID,BOOL bGetNewHandle=TRUE);
	virtual BOOL DeleteAcs(long hAcs);//删除装配坐标系
	virtual BOOL DeleteNode(long h,BOOL bIncPerm=TRUE);
	virtual BOOL DeletePart(long h,BOOL bIncPerm=TRUE);
	virtual BOOL DeleteView(long h,BOOL bIncPerm=TRUE);
	bool FillBlock(CBlockModel* pBlock);
	virtual void EmptyBlkObjSetBuf(){relaObjs.Empty();}
	virtual CLDSNode* EnumBlkBufNodeFirst();
	virtual CLDSNode* EnumBlkBufNodeNext();
	virtual CLDSPart* EnumBlkBufPartFirst(bool bOnlyRod=false);
	virtual CLDSPart* EnumBlkBufPartNext(bool bOnlyRod=false);
public:
	virtual long GetNewHandle();
	virtual bool IsTowerModel();
	//获取杆塔模型空间中直接管理并实际存在的构件数组，filterShadowPart=true表示只计入母本构件，影射构件将被怱略
	virtual bool GetRealityPartArr(DYN_ARRAY<CLDSBolt*>& arrBoltPtr,DYN_ARRAY<CLDSPart*>& arrPartPtr,
								   void *pPolyPartList,bool filterShadowPart=false);
public:	//根据句柄查找对象
	virtual CLDSObject *FromHandle(long h,BOOL bDbObjOnly=FALSE);
	virtual CDisplayView *FromViewHandle(long h);
	virtual CParaCS *FromAcsHandle(long hAcs);
	virtual CLDSPoint *FromPointHandle(long h);
	virtual CLDSLine *FromLineHandle(long h);
	virtual CLDSPlane *FromPlaneHandle(long h);
	virtual CLDSNode *FromNodeHandle(long h);
	virtual CLDSPart *FromPartHandle(long h,int CLASS_TYPEID=0);
	virtual CLDSPart* FromPartHandle(long h,int CLASS_TYPEID1,int CLASS_TYPEID2);	//根据句柄获得节点
	virtual CLDSPart* FromPartHandle(long h,int CLASS_TYPEID1,int CLASS_TYPEID2,int CLASS_TYPEID3);	//根据句柄获得节点
	virtual CLDSLinePart *FromRodHandle(long hRod);
	virtual CLDSArcLift *FromArcLiftHandle(long h);
public:	//约束相关的函数
	virtual CONSTRAINT* AppendConstraint(CONSTRAINT* pCopyFrom=NULL);
	virtual CONSTRAINT* GetConstraint(long constraint_id);
	virtual bool RemoveConstraint(long constraint_id,long hFromMasterObj=0);
	virtual CONSTRAINT* EnumConstraintFirst(BOOL bIterDelete=FALSE);
	virtual CONSTRAINT* EnumConstraintNext(BOOL bIterDelete=FALSE);
	virtual void ClearConstraintSyncStates();
public:	//遍历函数
	virtual long GetObjNum(int CLASS_TYPEID);
	virtual CDisplayView* EnumViewFirst(BOOL bIterDelete=FALSE);
	virtual CDisplayView* EnumViewNext(BOOL bIterDelete=FALSE);
	virtual CLDSNode* EnumNodeFirst(BOOL bIterDelete=FALSE);
	virtual CLDSNode* EnumNodeNext(BOOL bIterDelete=FALSE);
	virtual CLDSPart* EnumPartFirst(int CLASS_TYPEID=0,BOOL bIterDelete=FALSE);
	virtual CLDSPart* EnumPartNext(int CLASS_TYPEID=0,BOOL bIterDelete=FALSE);
	virtual CLDSNode* MoveToTailNode(BOOL bIterDelete = FALSE);
	virtual CLDSPart* MoveToTailPart(int CLASS_TYPEID=0,BOOL bIterDelete=FALSE);
	virtual CLDSLinePart* EnumRodFirst();
	virtual CLDSLinePart* EnumRodNext();
	virtual CLDSPoint* EnumPointFirst(BOOL bIterDelete=FALSE);
	virtual CLDSPoint* EnumPointNext(BOOL bIterDelete=FALSE);
	virtual CLDSLine* EnumLineFirst(BOOL bIterDelete=FALSE);
	virtual CLDSLine* EnumLineNext(BOOL bIterDelete=FALSE);
	virtual CLDSPlane* EnumPlaneFirst(BOOL bIterDelete=FALSE);
	virtual CLDSPlane* EnumPlaneNext(BOOL bIterDelete=FALSE);
	virtual int PushNodeStack();
	virtual bool PopNodeStack(int pos=-1);
	virtual int PushPartStack();
	virtual bool PopPartStack(int pos=-1);
#ifdef __COMMON_PART_INC_
	virtual CLDSPartGroup* EnumPartGroupFirst();
	virtual CLDSPartGroup* EnumPartGroupNext();
	virtual int GetPartGroupPartSet(long hPartGroup,CTmaPtrList<CLDSPart,CLDSPart*> &partset,int CLASS_TYPEID=0,bool bClearLock=true,long* phExcludePartBranchArr=0,int count=1);
	virtual long GetPartGroupNum();
#endif
};
extern CConsole console;
