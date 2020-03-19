#pragma once
#include "IModel.h"
#include "Tower.h"
#include "ldsptr_list.h"
#include "lds_part.h"
#include "Block.h"
//�û����������еĺ�̨��������̨
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
	BLK_RELATIVEOBJS relaObjs;	//��ʱ�洢�������ڵĹ�����������
public:
	CConsole(void);
	virtual ~CConsole(void);
	CTmaPtrList<CLDSNode,CLDSNode*> DispNodeSet;//NODESET DispNodeSet;
	CTmaPtrList<CLDSPart,CLDSPart*> DispPartSet;//PARTSET DispPartSet;
	CTmaPtrList<CBlockReference,CBlockReference*> DispBlockRefSet;
	CTmaPtrList<CLDSFoundation,CLDSFoundation*> DispFoundationSet;
	CHash64ListEx<HOLE_WALL> hashHoles;	//��Ҫ���ױڵ���������˨��ʶ���Ե�ʵ�����
	CLDSPart *m_pWorkPart;	//��ǰҪ�༭�Ĺ������
	CBuffer clipper_board;		//������
	UCS_STRU m_curACS;			//��ǰװ������ϵ(Assemble Coordinate System)
	UCS_STRU m_curWCS;			//��ǰ��������ϵ(Work Coordinate System)
	CParaCS des_wcs;			//��������ϵ����Ʋ���
public:
	virtual IModel* GetActiveModel(){return m_pModel;}
#ifndef __DRAWING_CONTEXT_
	void Display(CDisplayView* pView);
#endif
	virtual bool ActivateModel(IModel *pModel,long hView=0);
	//��ʾ��ͼ��������
	virtual CDisplayView *SetActiveView(long hView);
	virtual CDisplayView *GetActiveView();
	virtual BOOL DeleteDisplayView(long hView);
	virtual long ActiveModuleHandle();
	virtual CLDSModule* GetActiveModule();
#ifdef __PART_DESIGN_INC_
	virtual CWorkStatus* GetActiveWorkStatus();
#endif
	virtual UCS_STRU WCS(){return m_curWCS;}	//��ǰ��������ϵ
	virtual GEPOINT TransPFromScr(const double* point);
	virtual GEPOINT TransVFromScr(const double* vector); 
	virtual bool AddToObjSetBuf(CLDSObject* pObj);	//��Ӷ��󵽿���̨�Ĳ���������
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
	virtual bool EmptyAllDispObjList();	//���������ʾ�����б�
	//ɾ���¼�
	virtual void FireDeleteObject(CLDSObject *pObj,BOOL bDelFromSolidDraw=FALSE);
public:
	CLDSModule* InitTowerModel();
	//CLdsList<CDisplayView> View;	//��ʾ��ͼ
	//CLdsList<CLDSPoint> Point;			//��׼��
	//CLdsList<CLDSLine>  Line;			//��׼��
	//CLdsList<CLDSPlane> Plane;			//��׼ƽ��
	virtual CParaCS *AppendAcs();	//����װ������ϵ
	virtual CLDSPoint* AppendPoint(BOOL bGetNewHandle=TRUE);
	virtual CLDSLine* AppendLine(BOOL bGetNewHandle=TRUE);
	virtual CLDSPlane* AppendPlane(BOOL bGetNewHandle=TRUE);
	virtual CDisplayView* AppendView(BOOL bGetNewHandle=TRUE);
	virtual CLDSNode* AppendNode(BOOL bGetNewHandle=TRUE);
	virtual CLDSPart* AppendPart(int CLASS_TYPEID,BOOL bGetNewHandle=TRUE);
	virtual BOOL DeleteAcs(long hAcs);//ɾ��װ������ϵ
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
	//��ȡ����ģ�Ϳռ���ֱ�ӹ���ʵ�ʴ��ڵĹ������飬filterShadowPart=true��ʾֻ����ĸ��������Ӱ�乹�������S��
	virtual bool GetRealityPartArr(DYN_ARRAY<CLDSBolt*>& arrBoltPtr,DYN_ARRAY<CLDSPart*>& arrPartPtr,
								   void *pPolyPartList,bool filterShadowPart=false);
public:	//���ݾ�����Ҷ���
	virtual CLDSObject *FromHandle(long h,BOOL bDbObjOnly=FALSE);
	virtual CDisplayView *FromViewHandle(long h);
	virtual CParaCS *FromAcsHandle(long hAcs);
	virtual CLDSPoint *FromPointHandle(long h);
	virtual CLDSLine *FromLineHandle(long h);
	virtual CLDSPlane *FromPlaneHandle(long h);
	virtual CLDSNode *FromNodeHandle(long h);
	virtual CLDSPart *FromPartHandle(long h,int CLASS_TYPEID=0);
	virtual CLDSPart* FromPartHandle(long h,int CLASS_TYPEID1,int CLASS_TYPEID2);	//���ݾ����ýڵ�
	virtual CLDSPart* FromPartHandle(long h,int CLASS_TYPEID1,int CLASS_TYPEID2,int CLASS_TYPEID3);	//���ݾ����ýڵ�
	virtual CLDSLinePart *FromRodHandle(long hRod);
	virtual CLDSArcLift *FromArcLiftHandle(long h);
public:	//Լ����صĺ���
	virtual CONSTRAINT* AppendConstraint(CONSTRAINT* pCopyFrom=NULL);
	virtual CONSTRAINT* GetConstraint(long constraint_id);
	virtual bool RemoveConstraint(long constraint_id,long hFromMasterObj=0);
	virtual CONSTRAINT* EnumConstraintFirst(BOOL bIterDelete=FALSE);
	virtual CONSTRAINT* EnumConstraintNext(BOOL bIterDelete=FALSE);
	virtual void ClearConstraintSyncStates();
public:	//��������
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
