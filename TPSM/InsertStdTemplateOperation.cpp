#include "stdafx.h"
#include "InsertStdTemplateOperation.h"
#include "StdFacePanel.h"
#include "SortFunc.h"
//#include "MirTaAtom.h"
#include "TPSM.h"
#include "TPSMView.h"
#include "TPSMModel.h"
#include "Hash64Table.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////////
//�ֲ�����
void NewNode(CLDSNode *pNode)
{
	if(pNode==NULL)
		return;
	f3dPoint node_pos = pNode->ActivePosition();
	node_pos.ID = pNode->handle;
	//node_pos.pen.crColor = g_sysPara.crPartMode.crNode;
	//g_pSolidDraw->NewPoint(node_pos);
}

void NewLinePart(CLDSLinePart *pPart,bool updatesolid=true)
{
	if(pPart==NULL)
		return;
	PEN_STRU linepen;
	linepen.style=PS_SOLID;
	linepen.width=0;
	//linepen.crColor = g_sysPara.crTsaMode.crPrimaryLine;
	//g_pSolidSet->SetPen(linepen);
	pPart->Create3dSolidModel();
	//g_pSolidDraw->NewSolidPart(pPart->GetSolidPartObject());	
}

void NewPlate(CLDSGeneralPlate* pPlate)
{
	if(pPlate==NULL)
		return;
	pPlate->Create3dSolidModel();
	//g_pSolidDraw->NewSolidPart(pPlate->GetSolidPartObject());
}
//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
void CInsertStdTemplateOperation::SolveNodePosExpression(CFacePanelNode *pFaceNode,CExpression &expression,BOOL bIgnorePosExpression/*=FALSE*/)
{
	EXPRESSION_VAR *pVar;
	if(!bIgnorePosExpression&&strlen(pFaceNode->posx_expression)>0)
		pFaceNode->actual_pos.x=expression.SolveExpression(pFaceNode->posx_expression);
	pVar=expression.varList.Append();
	pVar->fValue=pFaceNode->actual_pos.x;
	_snprintf(pVar->variableStr,8,"%s_X",pFaceNode->sTitle);
	if(!bIgnorePosExpression&&strlen(pFaceNode->posy_expression)>0)
		pFaceNode->actual_pos.y=expression.SolveExpression(pFaceNode->posy_expression);
	pVar=expression.varList.Append();
	pVar->fValue=pFaceNode->actual_pos.y;
	_snprintf(pVar->variableStr,8,"%s_Y",pFaceNode->sTitle);
	if(!bIgnorePosExpression&&strlen(pFaceNode->posz_expression)>0)
		pFaceNode->actual_pos.z=expression.SolveExpression(pFaceNode->posz_expression);
	pVar=expression.varList.Append();
	pVar->fValue=pFaceNode->actual_pos.z;
	_snprintf(pVar->variableStr,8,"%s_Z",pFaceNode->sTitle);
}
void CInsertStdTemplateOperation::SolveNodePosExpression(CParaNode *pParaNode,CExpression &expression,BOOL bIgnorePosExpression/*=FALSE*/)
{	//CParaNode::actualpos��Ϊģ��ֲ�����ϵ������ֵ wjh-2015.11.07
	EXPRESSION_VAR *pVar;
	if(!bIgnorePosExpression&&strlen(pParaNode->posx_expression)>0)
		pParaNode->actualpos.x=expression.SolveExpression(pParaNode->posx_expression);
	pVar=expression.varList.Append();
	pVar->fValue=pParaNode->actualpos.x;
	_snprintf(pVar->variableStr,8,"%s_X",pParaNode->keyLabel.Label());
	if(!bIgnorePosExpression&&strlen(pParaNode->posy_expression)>0)
		pParaNode->actualpos.y=expression.SolveExpression(pParaNode->posy_expression);
	pVar=expression.varList.Append();
	pVar->fValue=pParaNode->actualpos.y;
	_snprintf(pVar->variableStr,8,"%s_Y",pParaNode->keyLabel.Label());
	if(!bIgnorePosExpression&&strlen(pParaNode->posz_expression)>0)
		pParaNode->actualpos.z=expression.SolveExpression(pParaNode->posz_expression);
	pVar=expression.varList.Append();
	pVar->fValue=pParaNode->actualpos.z;
	_snprintf(pVar->variableStr,8,"%s_Z",pParaNode->keyLabel.Label());
}
struct DATUMPLANE_STRU{	DWORD index[3];long hPlane;};
long IsSamePlane(DATUMPLANE_STRU *pPlane,short nodeIArr[3])
{
	for(int i=0;i<3;i++)
	{
		for(int j=0;j<3;j++)
		{
			if(pPlane->index[j]==nodeIArr[i])
				break;
		}
		if(j==3)	//ֻҪ��һ������ֵ�Ҳ�������Ϊ����ͬһ����׼��
			return false;
	}
	return true;
}
long IsSamePlane(DATUMPLANE_STRU *pPlane,KEY4C nodeLabelArr[3])
{
	for(int i=0;i<3;i++)
	{
		for(int j=0;j<3;j++)
		{
			if(pPlane->index[j]==nodeLabelArr[i].dwItem)
				break;
		}
		if(j==3)	//ֻҪ��һ������ֵ�Ҳ�������Ϊ����ͬһ����׼��
			return false;
	}
	return true;
}
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CInsertStdTemplateOperation::CInsertStdTemplateOperation(CTower *pTower)
{
	m_pFacePanel=NULL;
	m_pHeadTemplate=NULL;
	m_iTemplateGroup=1;	//0:��׼��ͷģ���1:�����׼��2:�����׼��3:��ڼ������׼��
	m_iKeyNodeMir=2;		//�ؼ���ĶԳ����뷽ʽ
	m_iTemplateMir=0;		//��׼�λ��׼��ĶԳ����ɷ�ʽ
	memset(m_arrShankKeyNode,0,sizeof(CLDSNode*)*4);	//ԭʼ¼��������׼����ĸ�����ڵ�
	m_pActiveModule=NULL;	//������ʼֵΪ�գ��ᵼ�º�����������ؼ���ʱָ��Ƿ��ڴ�������wjh-2014.1.21
	if(pTower==NULL)
		m_pTower=Manager.GetActiveTa();
	else
		m_pTower=pTower;
	acs.InitStdCS();
	m_cTransCS=0;
}
CLDSNode *CInsertStdTemplateOperation::FindLDSNodeByPos(const f3dPoint& pos,CFGWORD* pBodyWord/*=NULL*/,CFGWORD* pLegWord/*=NULL*/)
{
	CLDSNode *pLdsNode=m_pTower->CheckOverlappedNode(pos,m_pActiveModule,pBodyWord,pLegWord);
	if(pLdsNode)
		pLdsNode->SetModified();	//���ں���������޸Ľڵ�����,�����趨�޸ı��,����Undo��ʧ�� wjh 2011.8.12
	return pLdsNode;
}
CLDSNode *CInsertStdTemplateOperation::InsertNewLdsNode(const f3dPoint& pos,char firstLayerChar/*=0*/,CParaNode* pParaNode/*=NULL*/)
{
	CLDSNode *pNode=FindLDSNodeByPos(pos);
	if(pNode)
		return pNode;
	else //�Ҳ����˽ڵ㣬������Ҫ�����½ڵ�
	{
		if(pParaNode==NULL)
			pNode=m_pTower->Node.append();
		else
			pNode=ValidateLiveParaModelNode(pParaNode);
		pNode->SetPosition(pos);
		pNode->layer(1)='P';
		if(firstLayerChar!=0)
		{
			pNode->layer(0)=firstLayerChar;
			if(pos.x>=0&&pos.y>=0)
				pNode->layer(2)='1';
			else if(pos.x<=0&&pos.y>=0)
				pNode->layer(2)='2';
			else if(pos.x>=0&&pos.y<=0)
				pNode->layer(2)='3';
			else //if(pos.x<=0&&pos.y<=0)
				pNode->layer(2)='4';
		}
		NewNode(pNode);
		return pNode;
	}
}

CInsertStdTemplateOperation::~CInsertStdTemplateOperation()
{
	m_pActiveModule=NULL;

}
void LoadParaModelNodeInfo(CParaNode *pItem,DWORD key);	//defined in ParaTempl.cpp
void LoadParaModelRodInfo(CParaRod *pItem,DWORD key);	//defined in ParaTempl.cpp
CParaRod* CreateNewParaRod(int idClsType,DWORD key,void* pContext);	//����ص�����
BOOL DeleteParaRod(CParaRod *pRod);				//����ص�����
//��CParaNode�Ѿ��йҽӶ���ʱִ��TagInfo(...), ���򴴽��¹ҽ����ݶ���󷵻�
TAG_PARANODE_INFO* CInsertStdTemplateOperation::LoadTagInfo(CParaNode* pItem)
{
	if(pItem->TagInfo()!=NULL)
		return pItem->TagInfo();
	TAG_PARANODE_INFO* pTagInfo=listTagNodeInfo.append();
	pItem->SetTagInfo(pTagInfo);
	pTagInfo->AttachObject(pItem);
	return pTagInfo;
}
//��CParaNode�Ѿ��йҽӶ���ʱִ��TagInfo(...), ���򴴽��¹ҽ����ݶ���󷵻�
TAG_PARAROD_INFO* CInsertStdTemplateOperation::LoadTagInfo(CParaRod* pItem)
{
	if(pItem->TagInfo()!=NULL)
		return pItem->TagInfo();
	TAG_PARAROD_INFO* pTagInfo=listTagRodInfo.append();
	pItem->SetTagInfo(pTagInfo);
	pTagInfo->AttachObject(pItem);
	return pTagInfo;
}

CLDSLinePart* CInsertStdTemplateOperation::InsertNewPole(CParaRod *pParaRod)
{
	CParaNode* pParaStart=m_pParaTemplate->listParaNodes.GetValue(pParaRod->keyStartNode);
	CParaNode* pParaEnd=m_pParaTemplate->listParaNodes.GetValue(pParaRod->keyEndNode);
	CLDSNode *pStart=pParaStart->TagInfo()->pModelNode;
	CLDSNode *pEnd  =pParaEnd->TagInfo()->pModelNode;
	CLDSLinePart* pPole;
	for(pPole=(CLDSLinePart*)m_pTower->Parts.GetFirst(CLS_LINEPART);pPole;pPole=(CLDSLinePart*)m_pTower->Parts.GetNext(CLS_LINEPART))
	{
		if(pPole->pStart==NULL||pPole->pEnd==NULL)
			continue;
		else if(!(pPole->cfgword.And(m_pTower->GetDefaultCfgPartNo())))
			continue;
		else if(pPole->pStart==pStart&&pPole->pEnd==pEnd)
			break;
		else if(pPole->pStart==pEnd&&pPole->pEnd==pStart)
			break;
		else
		{
			f3dLine line;
			line.startPt=pPole->pStart->Position(false);
			line.endPt=pPole->pEnd->Position(false);
			pPole->SetModified();	//���ں���������޸ĸ˼�����,�����趨�޸ı��,����Undo��ʧ�� wjh 2011.8.12
			if(line.PtInLine(pStart->Position(false))>0&&line.PtInLine(pEnd->Position(false))>0)
				break;
		}
	}
	if(pPole)
		m_arrTemplateLdsLinePart.append(pPole);
	else
	{
#ifdef __ANGLE_PART_INC_//!defined(__TSA_)&&!defined(__TSA_FILE_)
		pPole=(CLDSLineAngle*)m_pTower->Parts.append(pParaRod->m_idPartClsType);
#else
		pPole=(CLDSLinePart*)m_pTower->Parts.append(CLS_LINEPART);
#endif
		pPole->size_idClassType=pParaRod->m_idPartClsType;
		//pPole->iSeg=iSeg;	//�κ�
		pPole->pStart=pStart;
		pPole->pEnd=pEnd;
		pPole->layer(0)='T';
		pPole->layer(1)=pParaRod->sLayer[1];
		if(pPole->layer(1)=='B')
			pPole->layer(1)='F';
		if(pPole->IsAuxPole())
			pPole->connectStart.N=pPole->connectEnd.N=1;	//����Ĭ��һ����˨���Ա����Զ���ֽڵ����������˨��� wjh-2017.12.17
		//�˼�ͼ�����е�λ���ַ�ͨ��CalPoleQuad�жϲ�׼��������ֱ�Ӽ̳�ԭ��ģ��ָ��ֵ wjh-2015.11.09
		//pPole->layer(2)=CalPoleQuad(pPole->pStart->Position(false),pPole->pEnd->Position(false));
		pPole->layer(2)=pParaRod->sLayer[2];
		pPole->des_wing_norm.wLayoutStyle=pParaRod->layout_style;
		if((pParaRod->layout_style&0x0f00)==0)	//�����˼������ܱ���ԭ��������ӦΪX/Y�Գ�����ͬ�� wjh-2016.5.31
			pPole->des_wing_norm.wLayoutStyle|=0x0100;
		bool bProcessedAngleWingNorm=false;
#ifdef __ANGLE_PART_INC_//!defined(__TSA_)&&!defined(__TSA_FILE_)
		if(pPole->GetClassTypeId()==CLS_LINEANGLE)
		{
			bProcessedAngleWingNorm=true;
			CLDSLineAngle* pAngle=(CLDSLineAngle*)pPole;
			if((pParaRod->layout_style&0x00ff)==0)
			{
				if(pParaRod->eyeSideNorm.IsZero())
					pAngle->des_norm_x.near_norm.Set(0,-1,0);
				else
					pAngle->des_norm_x.near_norm=-pParaRod->eyeSideNorm;
				pAngle->des_norm_y.bByOtherWing=TRUE;
				pAngle->desStartPos.wing_x_offset.gStyle=pAngle->desEndPos.wing_x_offset.gStyle=4;
				pAngle->desStartPos.wing_x_offset.offsetDist=pAngle->desEndPos.wing_x_offset.offsetDist=0;
				pAngle->desStartPos.wing_y_offset.gStyle=pAngle->desEndPos.wing_y_offset.gStyle=0;
			}
			else if((pParaRod->layout_style&0x00ff)==1)
			{
				if(pParaRod->eyeSideNorm.IsZero())
					pAngle->des_norm_y.near_norm.Set(0,1,0);
				else
					pAngle->des_norm_y.near_norm=pParaRod->eyeSideNorm;
				pAngle->des_norm_x.bByOtherWing=TRUE;
				pAngle->desStartPos.wing_y_offset.gStyle=pAngle->desEndPos.wing_y_offset.gStyle=4;
				pAngle->desStartPos.wing_y_offset.offsetDist=pAngle->desEndPos.wing_y_offset.offsetDist=0;
				pAngle->desStartPos.wing_x_offset.gStyle=pAngle->desEndPos.wing_x_offset.gStyle=0;
			}
			else if((pParaRod->layout_style&0x00ff)==2)
			{
				if(pParaRod->eyeSideNorm.IsZero())
					pAngle->des_norm_x.near_norm.Set(0,1,0);
				else
					pAngle->des_norm_x.near_norm=pParaRod->eyeSideNorm;
				pAngle->des_norm_y.bByOtherWing=TRUE;
				pAngle->desStartPos.wing_x_offset.gStyle=pAngle->desEndPos.wing_x_offset.gStyle=4;
				pAngle->desStartPos.wing_x_offset.offsetDist=pAngle->desEndPos.wing_x_offset.offsetDist=0;
				pAngle->desStartPos.wing_y_offset.gStyle=pAngle->desEndPos.wing_y_offset.gStyle=0;
			}
			else
			{
				if(pParaRod->eyeSideNorm.IsZero())
					pAngle->des_norm_y.near_norm.Set(0,-1,0);
				else
					pAngle->des_norm_y.near_norm=-pParaRod->eyeSideNorm;
				pAngle->des_norm_x.bByOtherWing=TRUE;
				pAngle->desStartPos.wing_y_offset.gStyle=pAngle->desEndPos.wing_y_offset.gStyle=4;
				pAngle->desStartPos.wing_y_offset.offsetDist=pAngle->desEndPos.wing_y_offset.offsetDist=0;
				pAngle->desStartPos.wing_x_offset.gStyle=pAngle->desEndPos.wing_x_offset.gStyle=0;
			}
		}
#endif
		if(bProcessedAngleWingNorm)
		{
			pPole->des_wing_norm.wLayoutStyle=(WORD)pParaRod->layout_style;
			if((pParaRod->layout_style&0x00ff)==0)
			{
				if(pParaRod->eyeSideNorm.IsZero())
					pPole->des_wing_norm.viewNorm.Set(0,-1,0);
				else
					pPole->des_wing_norm.viewNorm=-pParaRod->eyeSideNorm;
			}
			else if((pParaRod->layout_style&0x00ff)==1)
			{
				if(pParaRod->eyeSideNorm.IsZero())
					pPole->des_wing_norm.viewNorm.Set(0,1,0);
				else
					pPole->des_wing_norm.viewNorm=pParaRod->eyeSideNorm;
			}
			else if((pParaRod->layout_style&0x00ff)==2)
			{
				if(pParaRod->eyeSideNorm.IsZero())
					pPole->des_wing_norm.viewNorm.Set(0,1,0);
				else
					pPole->des_wing_norm.viewNorm=pParaRod->eyeSideNorm;
			}
			else
			{
				if(pParaRod->eyeSideNorm.IsZero())
					pPole->des_wing_norm.viewNorm.Set(0,-1,0);
				else
					pPole->des_wing_norm.viewNorm=-pParaRod->eyeSideNorm;
			}
		}
#ifdef __PART_DESIGN_INC_
		if(pParaRod->start_force_type==0)
			pPole->start_force_type=CENTRIC_FORCE;
		else
			pPole->start_force_type=ECCENTRIC_FORCE;
		if(pParaRod->end_force_type==0)
			pPole->end_force_type=CENTRIC_FORCE;
		else
			pPole->end_force_type=ECCENTRIC_FORCE;;
		if(pParaRod->start_joint_type==0)
			pPole->start_joint_type=JOINT_HINGE;
		else if(pParaRod->start_joint_type==1)
			pPole->start_joint_type=JOINT_RIGID;
		else
			pPole->start_joint_type=JOINT_AUTOJUSTIFY;
		if(pParaRod->end_joint_type==0)
			pPole->end_joint_type=JOINT_HINGE;
		else if(pParaRod->end_joint_type==1)
			pPole->end_joint_type=JOINT_RIGID;
		else
			pPole->end_joint_type=JOINT_AUTOJUSTIFY;;
		pPole->m_iElemType=pParaRod->m_iElemType;
#endif
		pPole->SetStart(pStart->Position(false));	//����ʼֵ
		pPole->SetEnd(pEnd->Position(false));		//����ʼֵ
		m_arrTemplateLdsLinePart.append(pPole);
	}

	pPole->SetStart(pParaStart->TagInfo()->pModelNode->Position(false));	
	pPole->SetEnd(pParaEnd->TagInfo()->pModelNode->Position(false));
	pPole->pStart=pParaStart->TagInfo()->pModelNode;
	pPole->pEnd=pParaEnd->TagInfo()->pModelNode;
	LoadTagInfo(pParaRod)->pModelRod=pPole;
	return pPole;
}

static void CopyWingOffsetStyle(CParaAngle::POS::WING_OFFSET_STYLE from, WING_OFFSET_STYLE* to)
{
	to->gStyle=from.gStyle;
	to->operStyle=from.operStyle;
}
bool CInsertStdTemplateOperation::AssignAnglePos(CMultiOffsetPos& desBerPos,CParaAngle::POS desTemplPos)
{
	if(desTemplPos.cTouchLineType==0||desTemplPos.cTouchLineType==2)
	{	//���ڹ���֫׼�ߣ�ƫ��׼���ݰ����ߴ���
		if(desTemplPos.cTouchLine==0)
		{	//������X֫����
			if(desTemplPos.face_offset_norm.IsZero())	//ͶӰ
				desBerPos.spatialOperationStyle=2;	//��X֫����ͶӰ
			else
			{
				desBerPos.spatialOperationStyle=5;	//��X֫����ƽ��
				desBerPos.face_offset_norm=desTemplPos.face_offset_norm;
				CopyWingOffsetStyle(desTemplPos.XOrCtrlWing ,&desBerPos.wing_x_offset);
				CopyWingOffsetStyle(desTemplPos.YOrOtherWing,&desBerPos.wing_y_offset);
			}
		}
		else if(desTemplPos.cTouchLine==1)
		{	//������Y֫����
			if(desTemplPos.face_offset_norm.IsZero())	//ͶӰ
				desBerPos.spatialOperationStyle=3;	//��Y֫����ͶӰ
			else
			{
				desBerPos.spatialOperationStyle=6;	//��Y֫����ƽ��
				desBerPos.face_offset_norm=desTemplPos.face_offset_norm;
				CopyWingOffsetStyle(desTemplPos.XOrCtrlWing ,&desBerPos.wing_x_offset);
				CopyWingOffsetStyle(desTemplPos.YOrOtherWing,&desBerPos.wing_y_offset);
			}
		}
		else if(desTemplPos.cTouchLine==2)
		{	//��������
			if(desTemplPos.face_offset_norm.IsZero())	//ͶӰ
				desBerPos.spatialOperationStyle=1;	//������ͶӰ
			else
			{
				desBerPos.spatialOperationStyle=4;	//������ƽ��
				desBerPos.face_offset_norm=desTemplPos.face_offset_norm;
				CopyWingOffsetStyle(desTemplPos.XOrCtrlWing ,&desBerPos.wing_x_offset);
				CopyWingOffsetStyle(desTemplPos.YOrOtherWing,&desBerPos.wing_y_offset);
			}
		}
		else //if(desTemplPos.cTouchLine==3)
		{	//����֫�Զ��ж�
			desBerPos.spatialOperationStyle=2;	//��ʱ��X֫����ͶӰ
			desBerPos.face_offset_norm=desTemplPos.face_offset_norm;
			CopyWingOffsetStyle(desTemplPos.XOrCtrlWing ,&desBerPos.wing_x_offset);
			CopyWingOffsetStyle(desTemplPos.YOrOtherWing,&desBerPos.wing_y_offset);
		}
		return true;
	}
	else if(desTemplPos.cTouchLineType==1)
	{	//���ڹ���֫������
		//KEY4C labelHuoquRod;
		if(desTemplPos.cTouchLine==0)
		{
			if(desTemplPos.face_offset_norm.IsZero())	//ͶӰ
				desBerPos.spatialOperationStyle=8;	//��X֫����ͶӰ
			else
			{
				desBerPos.spatialOperationStyle=11;	//��X֫���߻���ƽ��
				desBerPos.face_offset_norm=desTemplPos.face_offset_norm;
				CopyWingOffsetStyle(desTemplPos.XOrCtrlWing ,&desBerPos.wing_x_offset);
				CopyWingOffsetStyle(desTemplPos.YOrOtherWing,&desBerPos.wing_y_offset);
			}
		}
		else if(desTemplPos.cTouchLine==1)
		{
			if(desTemplPos.face_offset_norm.IsZero())	//ͶӰ
				desBerPos.spatialOperationStyle=9;	//��X֫����ͶӰ
			else
			{
				desBerPos.spatialOperationStyle=12;	//��Y֫���߻�����ƽ��
				desBerPos.face_offset_norm=desTemplPos.face_offset_norm;
				CopyWingOffsetStyle(desTemplPos.XOrCtrlWing ,&desBerPos.wing_x_offset);
				CopyWingOffsetStyle(desTemplPos.YOrOtherWing,&desBerPos.wing_y_offset);
			}
		}
		else if(desTemplPos.cTouchLine==2)
		{
			if(desTemplPos.face_offset_norm.IsZero())	//ͶӰ
				desBerPos.spatialOperationStyle=7;	//��X֫����ͶӰ
			else
			{
				desBerPos.spatialOperationStyle=10;	//��X֫���߻�����ƽ��
				desBerPos.face_offset_norm=desTemplPos.face_offset_norm;
				CopyWingOffsetStyle(desTemplPos.XOrCtrlWing ,&desBerPos.wing_x_offset);
				CopyWingOffsetStyle(desTemplPos.YOrOtherWing,&desBerPos.wing_y_offset);
			}
		}
		else //if(desTemplPos.cTouchLine==3)
		{	//����֫�Զ��ж�
			desBerPos.spatialOperationStyle=8;	//��ʱ��X֫���߻�����ͶӰ
			desBerPos.face_offset_norm=desTemplPos.face_offset_norm;
			CopyWingOffsetStyle(desTemplPos.XOrCtrlWing ,&desBerPos.wing_x_offset);
			CopyWingOffsetStyle(desTemplPos.YOrOtherWing,&desBerPos.wing_y_offset);
		}
	}
	else if(desTemplPos.cTouchLineType==3)
	{	//���Ƶ�
		if(desTemplPos.cMainCoordCtrlWing=='X'||desTemplPos.cMainCoordCtrlWing=='Y')	//ͶӰ
		{
			desBerPos.spatialOperationStyle=15;	//��֫�������
			desBerPos.coordCtrlType=desTemplPos.coordCtrlType;	//0:X-Z����;1:Y-Z����;2.X-Y����(����spatialOperationStyle=15ʱ��Ч)
			desBerPos.cMainCoordCtrlWing=desTemplPos.cMainCoordCtrlWing;
			CopyWingOffsetStyle(desTemplPos.XOrCtrlWing ,&desBerPos.ctrlWing );
			CopyWingOffsetStyle(desTemplPos.YOrOtherWing,&desBerPos.otherWing);
		}
		else
		{
			if(desTemplPos.face_offset_norm.IsZero())
				desBerPos.spatialOperationStyle=18;
			else
			{
				desBerPos.spatialOperationStyle=19;
				desBerPos.face_offset_norm=desTemplPos.face_offset_norm;
			}
			CopyWingOffsetStyle(desTemplPos.XOrCtrlWing ,&desBerPos.wing_x_offset);
			CopyWingOffsetStyle(desTemplPos.YOrOtherWing,&desBerPos.wing_y_offset);
		}
	}
	else if(desTemplPos.cTouchLineType==4)
	{	//�����
		desBerPos.spatialOperationStyle=0;	//��׼��
		desBerPos.datumPoint.datum_pos_style=6;	//��׼�����������겻���
		if(desTemplPos.cSymmetryType==1)
			desBerPos.datumPoint.des_para.DATUMPLANE.cMirTransType='X';
		else if(desTemplPos.cSymmetryType==2)
			desBerPos.datumPoint.des_para.DATUMPLANE.cMirTransType='Y';
		else if(desTemplPos.cSymmetryType==3)
			desBerPos.datumPoint.des_para.DATUMPLANE.cMirTransType='Z';
		else if(desTemplPos.cSymmetryType==0)
			desBerPos.datumPoint.des_para.DATUMPLANE.cMirTransType=0;
		if(desTemplPos.cPlanePtType==0)		//ָ��XY�������
			desBerPos.datumPoint.des_para.DATUMPLANE.fix_axis_style_x0_y1_z2=2;
		else if(desTemplPos.cPlanePtType==1)//ָ��YZ�������
			desBerPos.datumPoint.des_para.DATUMPLANE.fix_axis_style_x0_y1_z2=0;
		else if(desTemplPos.cPlanePtType==2)//ָ��XZ�������
			desBerPos.datumPoint.des_para.DATUMPLANE.fix_axis_style_x0_y1_z2=1;
		if(fabs(acs.axis_x.y)>EPS_COS)
		{	//����X\Y�᷽��
			if(desTemplPos.cSymmetryType=1||desTemplPos.cSymmetryType==2)
				desBerPos.datumPoint.des_para.DATUMPLANE.cMirTransType='X'+'Y'-desBerPos.datumPoint.des_para.DATUMPLANE.cMirTransType;
			if(desBerPos.datumPoint.des_para.DATUMPLANE.fix_axis_style_x0_y1_z2<2)
				desBerPos.datumPoint.des_para.DATUMPLANE.fix_axis_style_x0_y1_z2=1-desBerPos.datumPoint.des_para.DATUMPLANE.fix_axis_style_x0_y1_z2;
		}
		//desBerPos.datumPoint.des_para.DATUMPLANE.hDatumPlane=0;
		CopyWingOffsetStyle(desTemplPos.XOrCtrlWing ,&desBerPos.wing_x_offset);
		CopyWingOffsetStyle(desTemplPos.YOrOtherWing,&desBerPos.wing_y_offset);
		if(desTemplPos.cTouchLine==0)
			desBerPos.cWorkWingStyle=1;	//X֫����
		else if(desTemplPos.cTouchLine==1)
			desBerPos.cWorkWingStyle=2;	//Y֫����
		else if(desTemplPos.cTouchLine==2)
			desBerPos.cWorkWingStyle=3;	//����->˫֫
		else
			desBerPos.cWorkWingStyle=4;
	}
	else
		return false;
	if(m_cTransCS=='L')
	{
		if(desBerPos.cMainCoordCtrlWing=='X'||desBerPos.cMainCoordCtrlWing=='Y')
			desBerPos.cMainCoordCtrlWing='X'+'Y'-desBerPos.cMainCoordCtrlWing;
		if(desBerPos.cWorkWingStyle==1||desBerPos.cWorkWingStyle==2)
			desBerPos.cWorkWingStyle=3-desBerPos.cWorkWingStyle;
		if(desBerPos.coordCtrlType==1||desBerPos.coordCtrlType==2)
			desBerPos.coordCtrlType=3-desBerPos.coordCtrlType;
		WING_OFFSET_STYLE offset=desBerPos.wing_x_offset;
		desBerPos.wing_x_offset=desBerPos.wing_y_offset;
		desBerPos.wing_y_offset=offset;
	}
	return true;
}
EXPRESSION_VAR ConvertParamItemToExprVar(DESIGN_PARAM_ITEM *pItem);//defined in ParaTempl.cpp
//bool CorrectSingleAngleLayout(CLDSLineAngle* pJg,bool updatePartSolid=true);	//defined in LDSView.cpp
CLDSNode* CInsertStdTemplateOperation::ValidateLiveParaModelNode(CParaNode* pParaNode)
{
	if(LoadTagInfo(pParaNode)->pModelNode!=NULL)
		return LoadTagInfo(pParaNode)->pModelNode;
	pParaNode->TagInfo()->pModelNode=m_pTower->Node.append();
	pParaNode->actualpos=pParaNode->pos+m_vDefaultAssembleOffset;
	GEPOINT modelpos=acs.TransPFromCS(pParaNode->actualpos);
	pParaNode->TagInfo()->pModelNode->SetPosition(modelpos);
	return pParaNode->TagInfo()->pModelNode;
}
bool CInsertStdTemplateOperation::CreateStdTemplate(
	CParaTemplate* pParaTemplate,CLDSNode* keyNodeArr[4],int keyNodeMir,int templateMir,const double* axis_x_coords/*=NULL*/)
{
	m_cTransCS=0;
	if(axis_x_coords||fabs(axis_x_coords[0])+fabs(axis_x_coords[1])<EPS)
	{
		m_cTransCS='R';
		acs.axis_x.Set(axis_x_coords[0],axis_x_coords[1]);
		normalize(acs.axis_x);
		if( acs.axis_x.x-acs.axis_x.y>0 || 
			acs.axis_x.x==acs.axis_x.y&&acs.axis_x.x<0)	//4���޶Խ��߷���
			acs.axis_y.Set(-acs.axis_x.y,acs.axis_x.x);
		else	//��������
		{
			m_cTransCS='L';
			acs.axis_y.Set(acs.axis_x.y,-acs.axis_x.x);
		}
	}
	//TODO:��ȷ���Ƿ���Ҫ�������ᵣ�����wjh-2016.5.31
	/*
	int cMirTransX1Y2=0;
	if(keyNodeArr[0]&&keyNodeArr[1])
	{
		CParaNode* pParaNode0=pParaTemplate->listParaNodes.GetFirst();
		CParaNode* pParaNode1=pParaTemplate->listParaNodes.GetNext();
		GEPOINT templ_datum_vec=pParaNode1->pos-pParaNode0->pos;
		GEPOINT pos0=keyNodeArr[0]->Position();
		GEPOINT pos1=keyNodeArr[1]->Position();
		if(fabs(templ_datum_vec.x)>EPS)
		{
			GEPOINT hvec=pos1-pos0;
			if(hvec.x*templ_datum_vec.x<0)
				cMirTransX1Y2=2;
		}
		else if(pos0.x<0)
			cMirTransX1Y2=2;
	}*/
	//���ԭ������
	listTagNodeInfo.Empty();
	listTagRodInfo.Empty();
	m_arrTemplateLdsNode.Empty();	//ԭʼ���ɵı�׼�λ��׼���ڵĽڵ㼯��
	m_arrTemplateLdsLinePart.Empty();//ԭʼ���ɵı�׼�λ��׼���ڵĸ˼�����
	m_arrMirSumLdsNode.Empty();	//��ԭʼ���ɼ��Գ���������(������������)�ı�׼�λ��׼���ڵĽڵ㼯��
	m_arrMirSumLdsLinePart.Empty();	//��ԭʼ���ɼ��Գ���������(������������)�ı�׼�λ��׼���ڵĸ˼�����
	CHashSet<CParaNode*>hashKeyParaNodes;
	CHashList<CParaNode>listRepeatedParaNodes;	//��ʱ�洢�ظ����ɵ����нڵ�
	CSuperHashList<CParaRod>listRepeatedParaRods;	//��ʱ�洢�ظ����ɵ����и˼�
	listRepeatedParaNodes.LoadDefaultObjectInfo=LoadParaModelNodeInfo;
	listRepeatedParaRods.DeleteAtom = DeleteParaRod;
	listRepeatedParaRods.CreateNewAtom=CreateNewParaRod;
	listRepeatedParaRods.LoadDefaultObjectInfo=LoadParaModelRodInfo;

	m_pParaTemplate=pParaTemplate;
	m_iKeyNodeMir=keyNodeMir;
	m_iTemplateMir=templateMir;
	if((keyNodeMir==1||keyNodeMir==2) && fabs(acs.axis_x.y)>=EPS_COS)	//ģ������ϵX����ģ������ϵY�ᱣ��һ��
		keyNodeMir=3-keyNodeMir;	//�����ģ��ֲ�����ϵ�Ĺؼ���ԳƷ�ʽ

	SEGI iSeg=keyNodeArr[0]->iSeg;//ȡ��һ���ؼ���Ķκ�Ϊ��ǰ������Ķκ� wht 10-11-20

	DESIGN_PARAM_ITEM* pPara,*pItem;
	CParaNode *pParaNode,*pRepeatNode;
	CParaRod *pParaRod,*pRepeatRod;
	CLDSNode *pNode;
	CLDSLinePart *pPole=NULL;
	GEPOINT modelpos;	//��CParaNode::actualpos��Ӧ��ģ������ϵ�µ�����ֵ wjh-2015.11.07
	EXPRESSION_VAR* pIndexVar=NULL;
	ATOM_LIST<DATUMPLANE_STRU> planeList;
	//��׼��ͷģ�����Ʋ��������ʽ����
	CExpression expression;
	expression.varList.Empty();
	//�ѳ�ʼ�����ʽ�еĲ�������
	for(pPara=pParaTemplate->listParams.GetFirst();pPara;pPara=pParaTemplate->listParams.GetNext())
		expression.varList.Append(ConvertParamItemToExprVar(pPara));
	//ԭʼ��׼��������
	//1.1ԭʼ��׼��ڵ�������
	int i=0;
	bool initoffset=false;
	for(pParaNode=pParaTemplate->listParaNodes.GetFirst();pParaNode&&i<4;pParaNode=pParaTemplate->listParaNodes.GetNext())
	{
		if(pParaNode->m_bSetupJoint&&keyNodeArr[i]!=NULL)
		{
			hashKeyParaNodes.SetValue(keyNodeArr[i]->handle,pParaNode);
			LoadTagInfo(pParaNode)->pModelNode=keyNodeArr[i];
			modelpos.Set(keyNodeArr[i]->Position('x',false),keyNodeArr[i]->Position('y',false),keyNodeArr[i]->Position('z',false));
			pParaNode->actualpos=acs.TransPToCS(modelpos);
			if(!initoffset)
			{
				initoffset=true;
				m_vDefaultAssembleOffset=pParaNode->actualpos-pParaNode->pos;
			}
			SolveNodePosExpression(pParaNode,expression,TRUE);
			i++;
		}
		if(keyNodeArr[i]==NULL)
			break;
	}
	int nKeyNodeCount=i;
	for(pParaNode=pParaTemplate->listParaNodes.GetFirst();pParaNode;pParaNode=pParaTemplate->listParaNodes.GetNext())
	{
		if(pParaNode->m_bSetupJoint&&LoadTagInfo(pParaNode)->pModelNode!=NULL)
			continue;//����װ���Ͻڵ�
		if(pParaNode->attach_type==CParaNode::INDEPENDENT)
		{	//����ֵ���κ������ڵ㣨���û�ָ������Ŀ��Ƶ㣩
			for(int j=0;j<nKeyNodeCount;j++)
			{	//����ѡ�ؼ������ҵ���Ӧ�ԳƵĹؼ���
				if(keyNodeArr[j]==NULL)
					continue;
				CParaNode* pKeyParaNode=hashKeyParaNodes.GetValue(keyNodeArr[j]->handle);

				if((keyNodeMir==1||keyNodeMir==3)&&
					fabs(pParaNode->pos.x-pKeyParaNode->pos.x)<EPS&&
					fabs(pParaNode->pos.y+pKeyParaNode->pos.y)<EPS&&
					fabs(pParaNode->pos.z-pKeyParaNode->pos.z)<EPS)
				{	//�ؼ���X��Գ�,���ڵ�X��Գ�
					CLDSNode *pKeyNode=keyNodeArr[j];
					if(m_iKeyNodeMir==2)
						pNode=pKeyNode->GetMirNode(MIRMSG(2));
					else
						pNode=pKeyNode->GetMirNode(MIRMSG(1));
					if(pNode==NULL)
					{	//ֻ������ָ���ؼ����޺Ϸ��ԳƵĽڵ�
						f3dPoint mirpos=GetMirPos(pKeyNode->Position(false),MIRMSG(1));
						pNode=InsertNewLdsNode(mirpos,'T',pParaNode);
						pNode->iSeg=iSeg;	//�κ�
						pNode->AppendRelativeObj(RELATIVE_OBJECT(pKeyNode->handle,MIRMSG(1)));
						pKeyNode->AppendRelativeObj(RELATIVE_OBJECT(pNode->handle,MIRMSG(1)));
					}
					m_arrTemplateLdsNode.append(pNode);
					LoadTagInfo(pParaNode)->pModelNode=pNode;//�����̣ģ�ģ�ͽڵ���ģ��ڵ�
					modelpos.Set(pNode->Position('x',false),pNode->Position('y',false),pNode->Position('z',false));
					pParaNode->actualpos=acs.TransPToCS(modelpos);
					SolveNodePosExpression(pParaNode,expression,TRUE);
					break;
				}
				else if((keyNodeMir==2||keyNodeMir==3)&&
					fabs(pParaNode->pos.z-pKeyParaNode->pos.z)<EPS&&
					fabs(pParaNode->pos.y-pKeyParaNode->pos.y)<EPS&&
					fabs(pParaNode->pos.x+pKeyParaNode->pos.x)<EPS)
				{	//�ؼ���Y��Գ�,���ڵ�Y��Գ�
					CLDSNode *pKeyNode=keyNodeArr[j];
					if(m_iKeyNodeMir==1)
						pNode=pKeyNode->GetMirNode(MIRMSG(1));
					else
						pNode=pKeyNode->GetMirNode(MIRMSG(2));
					if(pNode==NULL)
					{
						f3dPoint mirpos=GetMirPos(pKeyNode->Position(false),MIRMSG(2));
						pNode=InsertNewLdsNode(mirpos,'T',pParaNode);
						pNode->iSeg=iSeg;	//�κ�
						pNode->AppendRelativeObj(RELATIVE_OBJECT(pKeyNode->handle,MIRMSG(2)));
						pKeyNode->AppendRelativeObj(RELATIVE_OBJECT(pNode->handle,MIRMSG(2)));
					}
					m_arrTemplateLdsNode.append(pNode);
					LoadTagInfo(pParaNode)->pModelNode=pNode;//�����̣ģ�ģ�ͽڵ���ģ��ڵ�
					modelpos.Set(pNode->Position('x',false),pNode->Position('y',false),pNode->Position('z',false));
					pParaNode->actualpos=acs.TransPToCS(modelpos);
					SolveNodePosExpression(pParaNode,expression,TRUE);
					break;
				}
				else if(m_iKeyNodeMir==3&&
					fabs(pParaNode->pos.x+pKeyParaNode->pos.x)<EPS&&
					fabs(pParaNode->pos.y+pKeyParaNode->pos.y)<EPS&&
					fabs(pParaNode->pos.z-pKeyParaNode->pos.z)<EPS)//Z��Գ�
				{	//�ؼ���XYZ��Գ�,���ڵ�XYZ��Գ�
					CLDSNode *pKeyNode=keyNodeArr[j];
					pNode=pKeyNode->GetMirNode(MIRMSG(3));
					if(pNode==NULL)
					{
						f3dPoint mirpos=GetMirPos(pKeyNode->Position(false),MIRMSG(3));
						pNode=InsertNewLdsNode(mirpos,'T',pParaNode);
						pNode->iSeg=iSeg;	//�κ�
						pNode->AppendRelativeObj(RELATIVE_OBJECT(pKeyNode->handle,MIRMSG(3)));
						pKeyNode->AppendRelativeObj(RELATIVE_OBJECT(pNode->handle,MIRMSG(3)));
					}
					m_arrTemplateLdsNode.append(pNode);
					LoadTagInfo(pParaNode)->pModelNode=pNode;//�����̣ģ�ģ�ͽڵ���ģ��ڵ�
					modelpos.Set(pNode->Position('x',false),pNode->Position('y',false),pNode->Position('z',false));
					pParaNode->actualpos=acs.TransPToCS(modelpos);
					SolveNodePosExpression(pParaNode,expression,TRUE);
					break;
				}
			}
			if(j==nKeyNodeCount)
			{
				SolveNodePosExpression(pParaNode,expression,FALSE);
				modelpos=acs.TransPFromCS(pParaNode->actualpos);
				LoadTagInfo(pParaNode)->pModelNode=pNode=InsertNewLdsNode(modelpos,'T',pParaNode);
				m_arrTemplateLdsNode.append(pNode);
				pNode->iSeg=iSeg;	//�κ�
			}
		}
		else if(pParaNode->attach_type>=CParaNode::COORD_X_SPECIFIED&&pParaNode->attach_type<=CParaNode::COORD_Z_SPECIFIED)
		{	//�˼���X(Y��Z)����ֵ�����
			f3dPoint start,end;
			CParaNode* pAttachStart=NULL,*pAttachEnd=NULL;
			if(pParaNode->attachNodeArr[0].dwItem>0)
				pAttachStart=pParaTemplate->listParaNodes.GetValue(pParaNode->attachNodeArr[0].dwItem);
			if(pAttachStart&&ValidateLiveParaModelNode(pAttachStart))	//&&LoadTagInfo(pAttachStart)->pModelNode!=NULL)	//��Ч�����ڵ�
				start=pAttachStart->TagInfo()->pModelNode->Position(false);
			else
#ifdef AFX_TARG_ENU_ENGLISH
				throw "Standard plane Lacks of reasonable attached node ,standard plane inserts failure��";
#else 
				throw "��׼���еĽڵ�ȱ�ٺ��������ڵ㣬�����׼��ʧ�ܣ�";
#endif
			if(pParaNode->attachNodeArr[1].dwItem>0)
				pAttachEnd=pParaTemplate->listParaNodes.GetValue(pParaNode->attachNodeArr[1].dwItem);
			if(pAttachEnd&&ValidateLiveParaModelNode(pAttachEnd))//&&LoadTagInfo(pAttachEnd)->pModelNode!=NULL)	//��Ч�����ڵ�
				end=pAttachEnd->TagInfo()->pModelNode->Position(false);	//��Ч�����ڵ�
			else
#ifdef AFX_TARG_ENU_ENGLISH
				throw "Standard plane Lacks of reasonable attached node ,standard plane inserts failure��";
#else 
				throw "��׼���еĽڵ�ȱ�ٺ��������ڵ㣬�����׼��ʧ�ܣ�";
#endif
			start=acs.TransPToCS(start);
			end=acs.TransPToCS(end);
			if(pParaNode->attach_type==CParaNode::COORD_X_SPECIFIED)
			{
				pParaNode->actualpos=pParaNode->pos;
				if(strlen(pParaNode->posx_expression)>0)
					pParaNode->actualpos.x=expression.SolveExpression(pParaNode->posx_expression);
				pParaNode->actualpos.y=start.y+(pParaNode->actualpos.x-start.x)*(end.y-start.y)/(end.x-start.x);
				pParaNode->actualpos.z=start.z+(pParaNode->actualpos.x-start.x)*(end.z-start.z)/(end.x-start.x);
			}
			else if(pParaNode->attach_type==CParaNode::COORD_Y_SPECIFIED)
			{
				pParaNode->actualpos=pParaNode->pos;
				if(strlen(pParaNode->posy_expression)>0)
					pParaNode->actualpos.y=expression.SolveExpression(pParaNode->posy_expression);
				pParaNode->actualpos.x=start.z+(pParaNode->actualpos.y-start.y)*(end.x-start.x)/(end.y-start.y);
				pParaNode->actualpos.z=start.z+(pParaNode->actualpos.y-start.y)*(end.z-start.z)/(end.y-start.y);
			}
			else if(pParaNode->attach_type==CParaNode::COORD_Z_SPECIFIED)
			{
				pParaNode->actualpos=pParaNode->pos;
				if(strlen(pParaNode->posz_expression)>0)
					pParaNode->actualpos.z=expression.SolveExpression(pParaNode->posz_expression);
				pParaNode->actualpos.x=start.x+(pParaNode->actualpos.z-start.z)*(end.x-start.x)/(end.z-start.z);
				pParaNode->actualpos.y=start.y+(pParaNode->actualpos.z-start.z)*(end.y-start.y)/(end.z-start.z);
			}
			SolveNodePosExpression(pParaNode,expression,TRUE);
			modelpos=acs.TransPFromCS(pParaNode->actualpos);
			pNode=FindLDSNodeByPos(modelpos);
			if(pNode!=NULL)
				m_arrTemplateLdsNode.append(pNode);
			else //if(pNode==NULL)
			{	//�Ҳ����˽ڵ㣬������Ҫ�����½ڵ�
				pNode=InsertNewLdsNode(modelpos,'T',pParaNode);
				if(pParaNode->attach_type==CParaNode::COORD_Z_SPECIFIED)
					pNode->m_cPosCalType=CLDSNode::COORD_Z_SPECIFIED;
				else if(fabs(acs.axis_x.x)>=EPS_COS)	//ģ������ϵX����ģ������ϵX�ᱣ��һ��
					pNode->m_cPosCalType=(BYTE)pParaNode->attach_type;	//X(Y��Z)���겻���
				else if(fabs(acs.axis_x.y)>=EPS_COS)	//ģ������ϵX����ģ������ϵY�ᱣ��һ��
					pNode->m_cPosCalType=(BYTE)(3-pParaNode->attach_type);	//����X\Y����
				else	//б��װ��ģ��ʱ��ֻ�ܽ�ָ��X\Y����������Ϊ��������������
					pNode->m_cPosCalType=CLDSNode::COORD_INDEPENDENT;
				pNode->arrRelationNode[0]=pAttachStart->TagInfo()->pModelNode->handle;
				pNode->arrRelationNode[1]=pAttachEnd->TagInfo()->pModelNode->handle;
				m_arrTemplateLdsNode.append(pNode);
				pNode->iSeg=iSeg;	//�κ�
			}
			LoadTagInfo(pParaNode)->pModelNode=pNode;
		}
		else if(pParaNode->attach_type==CParaNode::INTERSECTION)
		{	//���˼����潻��
			CParaNode* pAttachParaStart1=NULL,*pAttachParaStart2=NULL,*pAttachParaEnd1=NULL,*pAttachParaEnd2=NULL;
			CLDSNode *pStartNode1=NULL,*pStartNode2=NULL,*pEndNode1=NULL,*pEndNode2=NULL;
			if(pParaNode->attachNodeArr[0].dwItem>0)
				pAttachParaStart1=pParaTemplate->listParaNodes.GetValue(pParaNode->attachNodeArr[0]);
			if(pAttachParaStart1!=NULL&&ValidateLiveParaModelNode(pAttachParaStart1))//&&pAttachParaStart1->TagInfo())
				pStartNode1=pAttachParaStart1->TagInfo()->pModelNode;
			else
#ifdef AFX_TARG_ENU_ENGLISH
				throw "Standard plane Lacks of reasonable attached node ,standard plane inserts failure��";
#else 
				throw "��׼���еĽڵ�ȱ�ٺ��������ڵ㣬�����׼��ʧ�ܣ�";
#endif
			if(pParaNode->attachNodeArr[1].dwItem>0)
				pAttachParaEnd1=pParaTemplate->listParaNodes.GetValue(pParaNode->attachNodeArr[1]);
			if(pAttachParaEnd1!=NULL&&ValidateLiveParaModelNode(pAttachParaEnd1))//&&pAttachParaEnd1->TagInfo())
				pEndNode1=pAttachParaEnd1->TagInfo()->pModelNode;
			else
#ifdef AFX_TARG_ENU_ENGLISH
				throw "Standard plane Lacks of reasonable attached node ,standard plane inserts failure��";
#else 
				throw "��׼���еĽڵ�ȱ�ٺ��������ڵ㣬�����׼��ʧ�ܣ�";
#endif
			if(pParaNode->attachNodeArr[2].dwItem>0)
				pAttachParaStart2=pParaTemplate->listParaNodes.GetValue(pParaNode->attachNodeArr[2]);
			if(pAttachParaStart2!=NULL&&ValidateLiveParaModelNode(pAttachParaStart2))//&&pAttachParaStart2->TagInfo())
				pStartNode2=pAttachParaStart2->TagInfo()->pModelNode;
			else
#ifdef AFX_TARG_ENU_ENGLISH
				throw "Standard plane Lacks of reasonable attached node ,standard plane inserts failure��";
#else 
				throw "��׼���еĽڵ�ȱ�ٺ��������ڵ㣬�����׼��ʧ�ܣ�";
#endif
			if(pParaNode->attachNodeArr[3].dwItem>0)
				pAttachParaEnd2=pParaTemplate->listParaNodes.GetValue(pParaNode->attachNodeArr[3]);
			if(pAttachParaEnd2!=NULL&&ValidateLiveParaModelNode(pAttachParaEnd2))//&&pAttachParaEnd2->TagInfo())
				pEndNode2=pAttachParaEnd2->TagInfo()->pModelNode;
			else
#ifdef AFX_TARG_ENU_ENGLISH
				throw "Standard plane Lacks of reasonable attached node ,standard plane inserts failure��";
#else 
				throw "��׼���еĽڵ�ȱ�ٺ��������ڵ㣬�����׼��ʧ�ܣ�";
#endif
			f3dPoint pos;
			char cPrjAxis=0;//0:��ʾ������ΪͶӰ��
			if(pParaNode->m_cPosCalViceType==1)
				cPrjAxis='Y';	//1:X-Zƽ��;
			else if(pParaNode->m_cPosCalViceType==2)
				cPrjAxis='X';	//2:Y-Zƽ��;
			else if(pParaNode->m_cPosCalViceType==3)
				cPrjAxis='Z';	//3:X-Yƽ��
			int iRet=Int3dDifferPlaneLL(pStartNode1->Position(false),pEndNode1->Position(false),pStartNode2->Position(false),pEndNode2->Position(false),pos,cPrjAxis);
			pNode=FindLDSNodeByPos(pos);
			if(pNode==NULL)
			{	//�Ҳ����˽ڵ㣬������Ҫ�����½ڵ�
				pNode=InsertNewLdsNode(pos,'T',pParaNode);
				pNode->m_cPosCalType=4;	//���潻��
				pNode->m_cPosCalViceType=pParaNode->m_cPosCalViceType;
				pNode->arrRelationNode[0]=pStartNode1->handle;
				pNode->arrRelationNode[1]=pEndNode1->handle;
				pNode->arrRelationNode[2]=pStartNode2->handle;
				pNode->arrRelationNode[3]=pEndNode2->handle;
				pNode->m_cPosCalViceType=pParaNode->m_cPosCalViceType;
				pNode->iSeg=iSeg;	//�κ�
			}
			m_arrTemplateLdsNode.append(pNode);
			LoadTagInfo(pParaNode)->pModelNode=pNode;
		}
		else if(pParaNode->attach_type==CParaNode::SCALE)
		{	//�˼��ϱ����ȷֵ�
			DESIGN_PARAM_ITEM* pItem=pParaTemplate->listParams.GetValue(pParaNode->keyRepeatParam);
			int repeat_num=0;	//Ĭ�ϲ��ظ�
			if(pItem)	//�ظ�����ѭ���ڵ�
				repeat_num=pItem->value.iType;
			pIndexVar=NULL;
			i=1;
			do
			{
				if(repeat_num>0)
				{
					if(pIndexVar==NULL)
						pIndexVar=expression.varList.Append();
					pIndexVar->fValue=i;
					strcpy(pIndexVar->variableStr,"i");
					CParaNode* pRepeatNode=listRepeatedParaNodes.Add(KEY4C(pParaNode->keyLabel.pair.wItem,i));
					*pRepeatNode=*pParaNode;
					pRepeatNode->keyLabel=KEY4C(pParaNode->keyLabel.pair.wItem,i);
					pRepeatNode->SetTagInfo(NULL);
					sprintf(pRepeatNode->scale_expression,"%d/%d",i,repeat_num);
					pParaNode=pRepeatNode;
					i++;
				}
				f3dPoint start,end;
				CParaNode* pAttachStart=NULL,*pAttachEnd=NULL;
				if(pParaNode->attachNodeArr[0].dwItem>0)
					pAttachStart=pParaTemplate->listParaNodes.GetValue(pParaNode->attachNodeArr[0].dwItem);
				if(pAttachStart&&ValidateLiveParaModelNode(pAttachStart))//��ȷ����Ч�����ڵ�
					start=pAttachStart->TagInfo()->pModelNode->Position(false);
				else
#ifdef AFX_TARG_ENU_ENGLISH
					throw "Standard plane Lacks of reasonable attached node ,standard plane inserts failure��";
#else 
					throw "��׼���еĽڵ�ȱ�ٺ��������ڵ㣬�����׼��ʧ�ܣ�";
#endif
				if(pParaNode->attachNodeArr[1].dwItem>0)
					pAttachEnd=pParaTemplate->listParaNodes.GetValue(pParaNode->attachNodeArr[1].dwItem);
				if(pAttachEnd&&ValidateLiveParaModelNode(pAttachEnd))//��ȷ����Ч�����ڵ�
					end=pAttachEnd->TagInfo()->pModelNode->Position(false);	//��Ч�����ڵ�
				else
#ifdef AFX_TARG_ENU_ENGLISH
					throw "Standard plane Lacks of reasonable attached node ,standard plane inserts failure��";
#else 
					throw "��׼���еĽڵ�ȱ�ٺ��������ڵ㣬�����׼��ʧ�ܣ�";
#endif
				f3dPoint pos;
				double real_scale=pParaNode->attach_scale;
				if(pParaNode->attach_scale<0)
				{
					real_scale=-pParaNode->attach_scale;
					real_scale=0.1*((int)real_scale)/(real_scale-(int)real_scale);
				}
				pos.x=start.x+real_scale*(end.x-start.x);
				pos.y=start.y+real_scale*(end.y-start.y);
				pos.z=start.z+real_scale*(end.z-start.z);
				pNode=FindLDSNodeByPos(pos);
				if(pNode==NULL)
				{	//�Ҳ����˽ڵ㣬������Ҫ�����½ڵ�
					pNode=InsertNewLdsNode(pos,'T',pParaNode);
					pNode->m_cPosCalType=5;	//�����ȷֵ�
					pNode->attach_scale=real_scale;
					pNode->arrRelationNode[0]=pAttachStart->TagInfo()->pModelNode->handle;
					pNode->arrRelationNode[1]=pAttachEnd->TagInfo()->pModelNode->handle;
					pNode->iSeg=iSeg;	//�κ�
				}
				m_arrTemplateLdsNode.append(pNode);
				LoadTagInfo(pParaNode)->pModelNode=pNode;
			}while(i<repeat_num);
			
			if(repeat_num>1)
			{//�趨�ظ��ڵ����ĩ�ڵ�Ӱ��,������ܲ����ظ��˼�ȱ����ĩ��Ӱ�䣨��P��P0Ӧָͬ���׽ڵ㣬����P0-B�˼����޷�����Ӱ��)
				CParaNode* pRepeatNode=NULL;
				BOOL pushed=pParaTemplate->listParaNodes.push_stack();
				CParaNode* pDependencyStart=pParaTemplate->listParaNodes.GetValue(pParaNode->attachNodeArr[0]);
				CParaNode* pDependencyEnd=pParaTemplate->listParaNodes.GetValue(pParaNode->attachNodeArr[1]);
				if(pDependencyStart!=NULL)
				{
					pRepeatNode=listRepeatedParaNodes.Add(pParaNode->keyLabel.ClonePrior(0));
					LoadTagInfo(pRepeatNode)->pModelNode=pDependencyStart->TagInfo()->pModelNode;
				}
				if(pDependencyEnd!=NULL)
				{
					pRepeatNode=listRepeatedParaNodes.Add(pParaNode->keyLabel.ClonePrior(repeat_num));
					LoadTagInfo(pRepeatNode)->pModelNode=pDependencyEnd->TagInfo()->pModelNode;
				}
				pParaTemplate->listParaNodes.pop_stack(pushed);
			}
		}
		else if(pParaNode->attach_type>=CParaNode::PLANE_XY_SPECIFIED&&pParaNode->attach_type<=CParaNode::PLANE_XZ_SPECIFIED)
		{	//��׼����ָ��XY(YZ��XZ)�����
			f3dPoint start,end,face_pick,actualpos;
			CParaNodePtr attachParaNodeArr[3]={NULL};
			CLDSNodePtr  attachLdsNodeArr[3]={NULL};
			for(short i=0;i<3;i++)
			{
				if(pParaNode->attachNodeArr[i].dwItem>0)
					attachParaNodeArr[i]=pParaTemplate->listParaNodes.GetValue(pParaNode->attachNodeArr[i]);
				if(attachParaNodeArr[i]!=NULL&&ValidateLiveParaModelNode(attachParaNodeArr[i]))
					attachLdsNodeArr[i]=LoadTagInfo(attachParaNodeArr[i])->pModelNode;
			}
			if( attachLdsNodeArr[0]!=NULL&&attachLdsNodeArr[1]!=NULL&&attachLdsNodeArr[2]!=NULL)
			{	//��Ч�����ڵ�
				start=attachLdsNodeArr[0]->Position(false);
				end=attachLdsNodeArr[1]->Position(false);
				face_pick=attachLdsNodeArr[2]->Position(false);
				f3dPoint line_vec1=end-start;
				f3dPoint line_vec2=end-face_pick;
				f3dPoint face_norm=line_vec1^line_vec2;
				face_pick=acs.TransPToCS(face_pick);
				face_norm=acs.TransVToCS(face_norm);
				if(pParaNode->attach_type==CParaNode::PLANE_XY_SPECIFIED)
				{
					if(strlen(pParaNode->posx_expression)>0)
						actualpos.x=expression.SolveExpression(pParaNode->posx_expression);
					if(strlen(pParaNode->posy_expression)>0)
						actualpos.y=expression.SolveExpression(pParaNode->posy_expression);
					Int3dlf(actualpos,f3dPoint(actualpos.x,actualpos.y,0),f3dPoint(0,0,1),face_pick,face_norm);
				}
				else if(pParaNode->attach_type==CParaNode::PLANE_YZ_SPECIFIED)
				{
					if(strlen(pParaNode->posy_expression)>0)
						actualpos.y=expression.SolveExpression(pParaNode->posy_expression);
					if(strlen(pParaNode->posz_expression)>0)
						actualpos.z=expression.SolveExpression(pParaNode->posz_expression);
					Int3dlf(actualpos,f3dPoint(0,actualpos.y,actualpos.z),f3dPoint(1,0,0),face_pick,face_norm);
				}
				else if(pParaNode->attach_type==CParaNode::PLANE_XZ_SPECIFIED)
				{
					if(strlen(pParaNode->posx_expression)>0)
						actualpos.x=expression.SolveExpression(pParaNode->posx_expression);
					if(strlen(pParaNode->posz_expression)>0)
						actualpos.z=expression.SolveExpression(pParaNode->posz_expression);
					Int3dlf(actualpos,f3dPoint(actualpos.x,0,actualpos.z),f3dPoint(0,1,0),face_pick,face_norm);
				}
				modelpos=acs.TransPFromCS(actualpos);
				pNode=FindLDSNodeByPos(modelpos);
				if(pNode==NULL)
				{	//�Ҳ����˽ڵ㣬������Ҫ�����½ڵ�
					pNode=InsertNewLdsNode(modelpos,'T',pParaNode);
					//�趨��׼����ָ��XY(YZ��XZ)�����
					if(pParaNode->attach_type==CParaNode::PLANE_XY_SPECIFIED)
						pNode->m_cPosCalType=CLDSNode::PLANE_XY_SPECIFIED;
					else if(fabs(acs.axis_x.x)>=EPS_COS)	//ģ������ϵX����ģ������ϵX�ᱣ��һ��
						pNode->m_cPosCalType=8+pParaNode->attach_type-CParaNode::PLANE_XY_SPECIFIED;
					else if(fabs(acs.axis_x.y)>=EPS_COS)	//ģ������ϵX����ģ������ϵY�ᱣ��һ��
						pNode->m_cPosCalType=8+(15-pParaNode->attach_type)-CParaNode::PLANE_XY_SPECIFIED;
					else	//б��װ��ģ��ʱ��ֻ�ܽ�ָ��X\Y����������Ϊ��������������
						pNode->m_cPosCalType=CLDSNode::COORD_INDEPENDENT;
					DATUMPLANE_STRU *pPlane=NULL;
					for(pPlane=planeList.GetFirst();pPlane;pPlane=planeList.GetNext())
					{
						if(IsSamePlane(pPlane,pParaNode->attachNodeArr))
							break;
					}
					if(pPlane==NULL)
					{
						pPlane=planeList.append();
						CLDSPlane *pDatumPlane=m_pTower->Plane.append();
						CObjNoGroup *pPointNoGroup=m_pTower->NoManager.FromGroupID(MODEL_DATUM_POINT_GROUP);
						CObjNoGroup *pPlaneNoGroup=m_pTower->NoManager.FromGroupID(MODEL_DATUM_PLANE_GROUP);
						pDatumPlane->iNo=pPlaneNoGroup->EnumIdleNo();
						pPlaneNoGroup->SetNoState(pDatumPlane->iNo);
						sprintf(pDatumPlane->description,"%d",pDatumPlane->iNo);
						pPlane->hPlane=pDatumPlane->handle;
						memcpy(pPlane->index,pParaNode->attachNodeArr,sizeof(DWORD)*3);
						for(int kk=0;kk<3;kk++)
						{
							CLDSPoint *pDatumPoint=m_pTower->Point.append();
							pDatumPoint->iNo=pPointNoGroup->EnumIdleNo();
							pPointNoGroup->SetNoState(pDatumPoint->iNo);
							pDatumPoint->datum_pos_style=8;
							pDatumPoint->des_para.hNode=attachLdsNodeArr[kk]->handle;
							pDatumPlane->hPointArr[kk]=pDatumPoint->handle;
						}
					}
					pNode->hRelationPlane=pPlane->hPlane;
					pNode->iSeg=iSeg;	//�κ�
				}
				m_arrTemplateLdsNode.append(pNode);
				LoadTagInfo(pParaNode)->pModelNode=pNode;
				pParaNode->actualpos=actualpos;
				SolveNodePosExpression(pParaNode,expression,TRUE);
			}
			else
#ifdef AFX_TARG_ENU_ENGLISH
				throw "Standard plane Lacks of reasonable attached node ,standard plane inserts failure��";
#else 
				throw "��׼���еĽڵ�ȱ�ٺ��������ڵ㣬�����׼��ʧ�ܣ�";
#endif
		}
		else
#ifdef AFX_TARG_ENU_ENGLISH
			throw "Does not support the type of node's coordinate calculation";
#else 
			throw "��֧�ֵĽڵ������������";
#endif
	}
	for(pRepeatNode=listRepeatedParaNodes.GetFirst();pRepeatNode;pRepeatNode=listRepeatedParaNodes.GetNext())
		pParaTemplate->listParaNodes.SetValue(pRepeatNode->keyLabel,*pRepeatNode);
	//2.2ԭʼ��׼��˼��������
	//���������ɸ˼�
	for(pParaRod=pParaTemplate->listParaRods.GetFirst();pParaRod;pParaRod=pParaTemplate->listParaRods.GetNext())
	{
		BOOL bRepeatPole=FALSE;
		CParaNode* pParaStartNode=pParaTemplate->listParaNodes.GetValue(pParaRod->keyStartNode);
		CParaNode* pParaEndNode=pParaTemplate->listParaNodes.GetValue(pParaRod->keyEndNode);
		int repeat_num1=0,repeat_num2=0;
		if(pParaStartNode->keyRepeatParam.dwItem>0||pParaEndNode->keyRepeatParam.dwItem>0)
		{
			pItem=pParaTemplate->listParams.GetValue(pParaStartNode->keyRepeatParam);
			repeat_num1=pItem->value.iType;
			pItem=pParaTemplate->listParams.GetValue(pParaEndNode->keyRepeatParam);
			repeat_num2=pItem->value.iType;
			bRepeatPole=TRUE;
		}
		if(bRepeatPole)		//�ظ��˼�
		{
			int index=1;
			int repeat_num=(repeat_num1<repeat_num2)?repeat_num1:repeat_num2;
			CParaNode* pStartNode=NULL,*pEndNode=NULL;
			for(int i=0;i<=repeat_num;i++)
			{
				if(pIndexVar==NULL)
					break;	//ȱ��������������
				pIndexVar->fValue=i;
				if(expression.SolveLogicalExpression(pParaRod->exist_logicexpr))
				{
					int indexStart=ftoi(expression.SolveExpression(pParaRod->indexParaStart));
					int indexEnd=ftoi(expression.SolveExpression(pParaRod->indexParaEnd));
					KEY4C keyStart=pParaRod->keyStartNode.ClonePrior(indexStart);
					KEY4C keyEnd  =pParaRod->keyEndNode.ClonePrior(indexEnd);
					pStartNode=pParaTemplate->listParaNodes.GetValue(keyStart);
					pEndNode=pParaTemplate->listParaNodes.GetValue(keyEnd);
					if(pStartNode==NULL || pEndNode==NULL)
						continue;
					pRepeatRod=listRepeatedParaRods.Add(pParaRod->keyLabel.ClonePrior(i));
					*pRepeatRod=*pParaRod;
					pRepeatRod->keyStartNode=pStartNode->keyLabel;
					pRepeatRod->keyEndNode=pEndNode->keyLabel;
					InsertNewPole(pRepeatRod);
				}
			}
		}
		else	//���ظ��˼�
			InsertNewPole(pParaRod);
	}
	for(pRepeatRod=listRepeatedParaRods.GetFirst();pRepeatRod;pRepeatRod=listRepeatedParaRods.GetNext())
	{
		pParaRod=pParaTemplate->listParaRods.Add(pRepeatRod->keyLabel,pRepeatRod->m_idPartClsType,FALSE);
		pParaRod->CopyParaRod(pRepeatRod);
	}
	//2.3�²���ԭʼ��׼���ڵĽڵ�˼���ϵ��Ϣ��д
	//�ڵ㸸�˼���Ϣ�������˼���Ϣ
	for(pParaNode=pParaTemplate->listParaNodes.GetFirst();pParaNode;pParaNode=pParaTemplate->listParaNodes.GetNext())
	{
		pNode=LoadTagInfo(pParaNode)->pModelNode;
		if(pNode==NULL)
		{
#ifdef AFX_TARG_ENU_ENGLISH
			logerr.Log("Parameterized node %s doesn't generate correspond model node of LDS��",pParaNode->keyLabel.Label());
#else 
			logerr.Log("�������ڵ�%sδ���ɶ�Ӧ�ģ̣ģ�ģ�ͽڵ㣡",pParaNode->keyLabel.Label());
#endif
			continue;
		}
		if(pNode->m_cPosCalType==4)	//�����
		{
			pNode->arrRelationPole[0]=pNode->arrRelationPole[1]=0;
			for(int j=0;j<m_arrTemplateLdsLinePart.GetSize();j++)
			{
#ifdef __ANGLE_PART_INC_//__LDS_
				pPole=(CLDSLineAngle*)m_arrTemplateLdsLinePart[j];
#else
				pPole=m_arrTemplateLdsLinePart[j];
#endif
				if((pPole->pStart->handle==pNode->arrRelationNode[0]&&pPole->pEnd->handle==pNode->arrRelationNode[1])||
					(pPole->pStart->handle==pNode->arrRelationNode[1]&&pPole->pEnd->handle==pNode->arrRelationNode[0]))
					pNode->arrRelationPole[0]=pPole->handle;
				else if((pPole->pStart->handle==pNode->arrRelationNode[2]&&pPole->pEnd->handle==pNode->arrRelationNode[3])||
					(pPole->pStart->handle==pNode->arrRelationNode[3]&&pPole->pEnd->handle==pNode->arrRelationNode[2]))
					pNode->arrRelationPole[1]=pPole->handle;
			}
		}
		else if(pNode->m_cPosCalType!=0)	//�����κ������ڵ�
		{
			for(int j=0;j<m_arrTemplateLdsLinePart.GetSize();j++)
			{
				pPole=m_arrTemplateLdsLinePart[j];
				if((pPole->pStart->handle==pNode->arrRelationNode[0]&&pPole->pEnd->handle==pNode->arrRelationNode[1])||
					(pPole->pStart->handle==pNode->arrRelationNode[1]&&pPole->pEnd->handle==pNode->arrRelationNode[0]))
				{
					pNode->arrRelationPole[0]=pPole->handle;
					break;
				}
			}
		}
		if(pParaNode->keyLabelFatherRod.dwItem>0)
		{
			CParaRod* pFatherParaRod=pParaTemplate->listParaRods.GetValue(pParaNode->keyLabelFatherRod);
			if(pFatherParaRod&&LoadTagInfo(pFatherParaRod)->pModelRod)
				pNode->hFatherPart=LoadTagInfo(pFatherParaRod)->pModelRod->handle;
		}
		if(pNode->hFatherPart<0x20)
		{	//�޸��˼���Ϣ
			CLDSLinePart *pViceFatherPole=NULL;
			for(int j=0;j<m_arrTemplateLdsLinePart.GetSize();j++)
			{
				pPole=m_arrTemplateLdsLinePart[j];
				f3dLine line(pPole->pStart->Position(false),pPole->pEnd->Position(false));
				int ret=line.PtInLine(pNode->Position(false));
				if(ret==2)		//���ڸ˼�ֱ����
				{
					pNode->hFatherPart=pPole->handle;
					pNode->layer(2)=pPole->layer(2);
					break;
				}
				else if(ret==1&&pViceFatherPole==NULL)	//׼���˼�(�ڵ�Ϊ�˼��˽ڵ�)
					pViceFatherPole=pPole;
			}
			if(pNode->hFatherPart<0x20&&pViceFatherPole!=NULL)	//δ�ҵ����˼������Ե�һ��׼���˼�Ϊ���˼�
			{
				pNode->hFatherPart=pViceFatherPole->handle;
				pNode->layer(2)=pViceFatherPole->layer(2);
			}
		}
	}
	//2.4����˼�֫���߷���λ��
	//���ʵ���׼ƽ��
	for(CParaDatumPlane* pParaPlane=pParaTemplate->listParaPlanes.GetFirst();pParaPlane;pParaPlane=pParaTemplate->listParaPlanes.GetNext())
	{
		PLANE_MAPPING* pMapPlane=hashPlanes.Add(pParaPlane->keyLabel);
		pMapPlane->pMappingPlane=m_pTower->Plane.append();
		StrCpy(pMapPlane->pMappingPlane->description,pParaPlane->description);
		CLDSPoint* pPoint1=m_pTower->Point.append();
		CLDSPoint* pPoint2=m_pTower->Point.append();
		CLDSPoint* pPoint3=m_pTower->Point.append();
		pMapPlane->pMappingPlane->hPointArr[0]=pPoint1->handle;
		pMapPlane->pMappingPlane->hPointArr[1]=pPoint2->handle;
		pMapPlane->pMappingPlane->hPointArr[2]=pPoint3->handle;
		pPoint1->datum_pos_style=pPoint2->datum_pos_style=pPoint3->datum_pos_style=1;
		pPoint1->des_para.RODEND.wing_offset_style=pPoint2->des_para.RODEND.wing_offset_style=pPoint3->des_para.RODEND.wing_offset_style=4;
		pPoint1->des_para.RODEND.wing_offset_dist=pPoint2->des_para.RODEND.wing_offset_dist=pPoint3->des_para.RODEND.wing_offset_dist=0;
		pParaRod=pParaTemplate->listParaRods.GetValue(pParaPlane->XFirPos.keyRefRod);
		pPoint1->des_para.RODEND.hRod=LoadTagInfo(pParaRod)->pModelRod->handle;
		if(pParaPlane->XFirPos.bStart)
			pPoint1->des_para.RODEND.direction=0;
		else
			pPoint1->des_para.RODEND.direction=1;
		pParaRod=pParaTemplate->listParaRods.GetValue(pParaPlane->XSecPos.keyRefRod);
		pPoint2->des_para.RODEND.hRod=LoadTagInfo(pParaRod)->pModelRod->handle;
		if(pParaPlane->XSecPos.bStart)
			pPoint2->des_para.RODEND.direction=0;
		else
			pPoint2->des_para.RODEND.direction=1;
		pParaRod=pParaTemplate->listParaRods.GetValue(pParaPlane->XThirPos.keyRefRod);
		pPoint3->des_para.RODEND.hRod=LoadTagInfo(pParaRod)->pModelRod->handle;
		if(pParaPlane->XThirPos.bStart)
			pPoint3->des_para.RODEND.direction=0;
		else
			pPoint3->des_para.RODEND.direction=1;
	}
	for(pParaNode=pParaTemplate->listParaNodes.GetFirst();pParaNode;pParaNode=pParaTemplate->listParaNodes.GetNext())
	{
		if(pParaNode->attach_type==CParaNode::INTERSECTION&&pParaNode->m_cPosCalViceType>0)
		{
			pParaRod=pParaTemplate->listParaRods.GetValue(pParaNode->keyLabelFatherRod);
			if(pParaRod&&pParaRod->TagInfo()->pModelRod)
				pParaNode->TagInfo()->pModelNode->arrRelationPole[0]=pParaRod->TagInfo()->pModelRod->handle;
			pParaRod=pParaTemplate->listParaRods.GetValue(pParaNode->keyLabelOtherRod);
			if(pParaRod&&pParaRod->TagInfo()->pModelRod)
				pParaNode->TagInfo()->pModelNode->arrRelationPole[1]=pParaRod->TagInfo()->pModelRod->handle;
		}
	}
	for(pParaRod=pParaTemplate->listParaRods.GetFirst();pParaRod;pParaRod=pParaTemplate->listParaRods.GetNext())
	{
		if(LoadTagInfo(pParaRod)->pModelRod==NULL)
			continue;
		CLDSLinePart* pRod=LoadTagInfo(pParaRod)->pModelRod;
		if(pParaRod->IsAngleObject()&&pRod!=NULL)
		{
			CParaAngle* pParaAngle=(CParaAngle*)pParaRod;
			AssignAnglePos(((CLDSLineAngle*)pRod)->desStartPos,pParaAngle->desStart);
			if(((CLDSLineAngle*)pRod)->desStartPos.spatialOperationStyle==1)
			{
				((CLDSLineAngle*)pRod)->desStartPos.wing_x_offset.gStyle=((CLDSLineAngle*)pRod)->desStartPos.wing_y_offset.gStyle=4;
				((CLDSLineAngle*)pRod)->desStartPos.wing_x_offset.offsetDist=((CLDSLineAngle*)pRod)->desStartPos.wing_y_offset.offsetDist=0;
				((CLDSLineAngle*)pRod)->LockStartPos();	//��ֹ��������CorrectSingleAngleLayoutʱ����������ƫ������wjh-2016.6.4
			}
			if(pParaAngle->desStart.cTouchLineType==4)
			{
				PLANE_MAPPING* pMapPlane=hashPlanes.GetValue(pParaAngle->desStart.keyLabelPlane);
				if(pMapPlane&&pMapPlane->pMappingPlane)
					((CLDSLineAngle*)pRod)->desStartPos.datumPoint.des_para.DATUMPLANE.hDatumPlane=pMapPlane->pMappingPlane->handle;
				if(pRod->pStart)
					((CLDSLineAngle*)pRod)->desStartPos.datumPoint.datum_pos=pRod->pStart->Position(false);
			}
			AssignAnglePos(((CLDSLineAngle*)pRod)->desEndPos,pParaAngle->desEnd);
			if(((CLDSLineAngle*)pRod)->desEndPos.spatialOperationStyle==1)
			{
				((CLDSLineAngle*)pRod)->desEndPos.wing_x_offset.gStyle=((CLDSLineAngle*)pRod)->desEndPos.wing_y_offset.gStyle=4;
				((CLDSLineAngle*)pRod)->desEndPos.wing_x_offset.offsetDist=((CLDSLineAngle*)pRod)->desEndPos.wing_y_offset.offsetDist=0;
				((CLDSLineAngle*)pRod)->LockEndPos();	//��ֹ��������CorrectSingleAngleLayoutʱ����������ƫ������wjh-2016.6.4
			}
			if(m_cTransCS=='L'&&(pParaAngle->cPosCtrlDatumLineType=='X'||pParaAngle->cPosCtrlDatumLineType=='Y'))
				((CLDSLineAngle*)pRod)->m_cPosCtrlDatumLineType='X'+'Y'-pParaAngle->cPosCtrlDatumLineType;
			else
				((CLDSLineAngle*)pRod)->m_cPosCtrlDatumLineType=pParaAngle->cPosCtrlDatumLineType;
			if(fabs(acs.axis_x.y)>EPS_COS&&(pParaAngle->cPosCtrlType==2||pParaAngle->cPosCtrlType==3))
				((CLDSLineAngle*)pRod)->m_cPosCtrlType=5-pParaAngle->cPosCtrlType;
			else
				((CLDSLineAngle*)pRod)->m_cPosCtrlType=pParaAngle->cPosCtrlType;
			if(pParaAngle->desEnd.cTouchLineType==4)
			{
				PLANE_MAPPING* pMapPlane=hashPlanes.GetValue(pParaAngle->desEnd.keyLabelPlane);
				if(pMapPlane&&pMapPlane->pMappingPlane)
					((CLDSLineAngle*)pRod)->desEndPos.datumPoint.des_para.DATUMPLANE.hDatumPlane=pMapPlane->pMappingPlane->handle;
				if(pRod->pEnd)
					((CLDSLineAngle*)pRod)->desEndPos.datumPoint.datum_pos=pRod->pEnd->Position(false);
			}
		}
	//2.5������׼���ڸ˼���Ϣ�����㳤�����ͼ��������͵ȣ���д
#ifdef __PART_DESIGN_INC_
		CLDSLinePart *pTmPole=NULL;
		if(pParaRod->keyLamdaRefPole.dwItem>0)
		{
			CParaRod* pRefParaRod=pParaTemplate->listParaRods.GetValue(pParaRod->keyLamdaRefPole);
			if(pRefParaRod&&pRefParaRod->TagInfo())
				pRod->CalLenCoef.hHorizPole=pRefParaRod->TagInfo()->pModelRod->handle;
		}
#endif
	}
	for(pParaRod=pParaTemplate->listParaRods.GetFirst();pParaRod;pParaRod=pParaTemplate->listParaRods.GetNext())
	{
		TAG_PARAROD_INFO* pTagInfo=pParaRod->TagInfo();
		if(pTagInfo==NULL || pTagInfo->pModelRod==NULL)
			continue;
		CLDSLineAngle* pCurJg=NULL;
		CLDSLinePart* pRefRod=NULL;
		CParaRod* pRefParaRod=NULL;
		if(pParaRod->keyRefPole.dwItem>0)
			pRefParaRod=pParaTemplate->listParaRods.GetValue(pParaRod->keyRefPole);
		if(pRefParaRod)
			pRefRod=pRefParaRod->TagInfo()->pModelRod;
		GEPOINT eyeSideNorm=acs.TransVFromCS(pParaRod->eyeSideNorm);
		if(pTagInfo->pModelRod->IsAngle(FALSE))
		{	//����Ǹֵ�֫����
			pCurJg=(CLDSLineAngle*)pTagInfo->pModelRod;
			switch(pParaRod->layout_style)
			{
			case 0:
			case 2:
				if(pParaRod->layout_style==0)	//������ˮ
					pCurJg->des_norm_x.near_norm=-eyeSideNorm;
				else							//������ˮ
					pCurJg->des_norm_x.near_norm=eyeSideNorm;
				if(pRefRod)
					pCurJg->des_norm_x.hViceJg=pRefRod->handle;
				else if(pCurJg->pStart->hFatherPart!=pCurJg->handle)
					pCurJg->des_norm_x.hViceJg = pCurJg->pStart->hFatherPart;
				else if(pCurJg->pEnd->hFatherPart!=pCurJg->handle)
					pCurJg->des_norm_x.hViceJg = pCurJg->pEnd->hFatherPart;
				else
				{
					pCurJg->des_norm_x.bSpecific=TRUE;
					pCurJg->des_norm_x.spec_norm.vector=pCurJg->des_norm_x.near_norm;
				}
				pCurJg->des_norm_x.bByOtherWing = FALSE;
				pCurJg->des_norm_y.bByOtherWing = TRUE;		//Y֫������X֫����Ϊ��׼
				break;
			case 1:
			case 3:
			default:
				if(pParaRod->layout_style==1)	//����
					pCurJg->des_norm_y.near_norm = eyeSideNorm;
				else					//����
					pCurJg->des_norm_y.near_norm =-eyeSideNorm;
				if(pRefRod)
					pCurJg->des_norm_y.hViceJg=pRefRod->handle;
				else if(pCurJg->pStart->hFatherPart!=pCurJg->handle)
					pCurJg->des_norm_y.hViceJg = pCurJg->pStart->hFatherPart;
				else if(pCurJg->pEnd->hFatherPart!=pCurJg->handle)
					pCurJg->des_norm_y.hViceJg = pCurJg->pEnd->hFatherPart;
				else
				{
					pCurJg->des_norm_y.bSpecific=TRUE;
					pCurJg->des_norm_y.spec_norm.vector=pCurJg->des_norm_y.near_norm;
				}
				pCurJg->des_norm_y.bByOtherWing = FALSE;
				pCurJg->des_norm_x.bByOtherWing = TRUE;		//X֫������Y֫����Ϊ��׼
				break;
			}
			if(m_cTransCS=='L')
			{	//��������ϵװ��ʱ,X\Y֫����
				DESIGN_JGWING_NORM des_wing_norm=pCurJg->des_norm_x;
				pCurJg->des_norm_x=pCurJg->des_norm_y;
				pCurJg->des_norm_y=des_wing_norm;
			}
			pCurJg->cal_x_wing_norm();
			pCurJg->cal_y_wing_norm();
		}
		else
		{
			pTagInfo->pModelRod->des_wing_norm.wLayoutStyle=(WORD)pParaRod->layout_style;
			pTagInfo->pModelRod->des_wing_norm.viewNorm=eyeSideNorm;
			if(pTagInfo->pModelRod->pStart->hFatherPart!=pTagInfo->pModelRod->handle)
				pTagInfo->pModelRod->des_wing_norm.hRefPoleArr[0]=pTagInfo->pModelRod->pStart->hFatherPart;
			else if(pTagInfo->pModelRod->pEnd->hFatherPart!=pTagInfo->pModelRod->handle)
				pTagInfo->pModelRod->des_wing_norm.hRefPoleArr[0]=pTagInfo->pModelRod->pEnd->hFatherPart;
		}
		pTagInfo->pModelRod->CalPosition();
		NewLinePart(pTagInfo->pModelRod);
	}
	for(pParaRod=pParaTemplate->listParaRods.GetFirst();pParaRod;pParaRod=pParaTemplate->listParaRods.GetNext())
	{
		TAG_PARAROD_INFO* pTagInfo=pParaRod->TagInfo();
		if(pTagInfo==NULL || pTagInfo->pModelRod==NULL || !pParaRod->IsAngleObject() || !pTagInfo->pModelRod->IsAngle(FALSE))
			continue;
		CLDSLineAngle* pCurJg=(CLDSLineAngle*)pTagInfo->pModelRod;
		if(((CParaAngle*)pParaRod)->desStart.cTouchLine==3||((CParaAngle*)pParaRod)->desEnd.cTouchLine==3)
		{
			//CorrectSingleAngleLayout(pCurJg);
			pCurJg->CalPosition();
		}
		pCurJg->UnlockStartPos();
		pCurJg->UnlockEndPos();
	}
	for(i=0;i<m_arrTemplateLdsNode.GetSize();i++)
		m_arrMirSumLdsNode.append(m_arrTemplateLdsNode[i]);
	for(i=0;i<m_arrTemplateLdsLinePart.GetSize();i++)
		m_arrMirSumLdsLinePart.append(m_arrTemplateLdsLinePart[i]);
	FillMirInfo();
	//��ֵ�ڵ�ĸ��˼����
	for(pParaNode=pParaTemplate->listParaNodes.GetFirst();pParaNode;pParaNode=pParaTemplate->listParaNodes.GetNext())
	{
		pParaRod=pParaTemplate->listParaRods.GetValue(pParaNode->keyLabelFatherRod);
		if(pParaRod&&pParaNode->TagInfo()&&pParaRod->TagInfo())
			pParaNode->TagInfo()->pModelNode->hFatherPart=pParaRod->TagInfo()->pModelRod->handle;
	}
	//������׼��ԳƲ���
	MirTemplate();	 		//��׼��Գ�
	return true;
}
bool CInsertStdTemplateOperation::CreateStdTemplate(CHeadTemplate* pHeadTemplate,CLDSNode* m_arrTemplateLdsNodeArr[4],
													int m_arrTemplateLdsNodeMir,int templateMir)
{
#ifdef __ANGLE_PART_INC_//!defined(__TSA_)&&!defined(__TSA_FILE_)
	CLDSLineAngle *pPole=NULL;
#else
	CLDSLinePart *pPole=NULL;
#endif
	ARRAY_LIST<CFacePanelNode>arrFaceNodeTemp;
	ARRAY_LIST<CFacePanelPole>arrFacePoleTemp;
	//���ԭ������
	m_arrTemplateLdsNode.Empty();	//ԭʼ���ɵı�׼�λ��׼���ڵĽڵ㼯��
	m_arrMirSumLdsNode.Empty();	//��ԭʼ���ɼ��Գ���������(������������)�ı�׼�λ��׼���ڵĽڵ㼯��
	m_arrTemplateLdsLinePart.Empty();//ԭʼ���ɵı�׼�λ��׼���ڵĸ˼�����
	m_arrMirSumLdsLinePart.Empty();	//��ԭʼ���ɼ��Գ���������(������������)�ı�׼�λ��׼���ڵĸ˼�����

	m_pHeadTemplate=pHeadTemplate;
	m_iKeyNodeMir=m_arrTemplateLdsNodeMir;
	m_iTemplateMir=templateMir;

	int i;
	SEGI iSeg;//��ǰ��׼��Ķκ�
	for(i=0;i<4;i++)
	{
		if(m_arrTemplateLdsNodeArr[i]!=NULL)
		{
			m_arrTemplateLdsNode.append(m_arrTemplateLdsNodeArr[i]);
			if(i==0)	//ȡ��һ���ؼ���Ķκ�Ϊ��ǰ������Ķκ� wht 10-11-20
				iSeg=m_arrTemplateLdsNodeArr[i]->iSeg;
		}
	}
	CHeadParameter* pPara;
	CFacePanelNode *pFaceNode;
	CFacePanelPole *pFacePole;
	CLDSNode *pNode;
	ATOM_LIST<DATUMPLANE_STRU> planeList;
	//��׼��ͷģ�����Ʋ��������ʽ����
	CExpression expression;
	expression.varList.Empty();
	//�ѳ�ʼ�����ʽ�еĲ�������
	for(pPara=pHeadTemplate->para_list.GetFirst();pPara;pPara=pHeadTemplate->para_list.GetNext())
		expression.varList.Append(pPara->var);
	//ԭʼ��׼��������
	//1.1ԭʼ��׼��ڵ�������
	for(pFaceNode=pHeadTemplate->node_list.GetFirst();pFaceNode;pFaceNode=pHeadTemplate->node_list.GetNext())
		arrFaceNodeTemp.append(*pFaceNode);
	for(pFacePole=pHeadTemplate->pole_list.GetFirst();pFacePole;pFacePole=pHeadTemplate->pole_list.GetNext())
		arrFacePoleTemp.append(*pFacePole);
	for(i=0;i<m_arrTemplateLdsNode.GetSize();i++)
	{
		pNode=m_arrTemplateLdsNode[i];
		arrFaceNodeTemp[i].actual_pos.Set(pNode->Position(false).x,pNode->Position(false).y,pNode->Position(false).z);
		SolveNodePosExpression(&arrFaceNodeTemp[i],expression,TRUE);
	}
	for(i=m_arrTemplateLdsNode.GetSize();i<pHeadTemplate->node_list.GetNodeNum();i++)
	{
		CFacePanelNode *pMirFaceNode=&arrFaceNodeTemp[i];
		if(pMirFaceNode->attach_type==0)	//����ֵ�����
		{
			for(int j=0;j<m_arrTemplateLdsNode.GetSize();j++)
			{	//����ѡ�ؼ������ҵ���Ӧ�ԳƵĹؼ���
				pFaceNode=&arrFaceNodeTemp[j];
				if((m_iKeyNodeMir==1||m_iKeyNodeMir==3)&&
					fabs(pFaceNode->pos.x-pMirFaceNode->pos.x)<EPS&&
					fabs(pFaceNode->pos.y+pMirFaceNode->pos.y)<EPS&&
					fabs(pFaceNode->pos.z-pMirFaceNode->pos.z)<EPS)
				{	//�ؼ���X��Գ�,���ڵ�X��Գ�
					CLDSNode *pKeyNode=m_arrTemplateLdsNode[j];
					pNode=pKeyNode->GetMirNode(MIRMSG(1));
					if(pNode)
					{
						m_arrTemplateLdsNode.append(pNode);
						arrFaceNodeTemp[i].actual_pos.Set(pNode->Position(false).x,pNode->Position(false).y,pNode->Position(false).z);
						SolveNodePosExpression(&arrFaceNodeTemp[i],expression,TRUE);
					}
					else
					{
						f3dPoint mirpos=GetMirPos(pKeyNode->Position(false),MIRMSG(1));
						pNode=InsertNewLdsNode(mirpos,'T');
						m_arrTemplateLdsNode.append(pNode);
						pNode->iSeg=iSeg;	//�κ�
						arrFaceNodeTemp[i].actual_pos.Set(pNode->Position(false).x,pNode->Position(false).y,pNode->Position(false).z);
						SolveNodePosExpression(&arrFaceNodeTemp[i],expression,TRUE);
						pNode->AppendRelativeObj(RELATIVE_OBJECT(pKeyNode->handle,MIRMSG(1)));
						pKeyNode->AppendRelativeObj(RELATIVE_OBJECT(pNode->handle,MIRMSG(1)));
					}
					break;
				}
				else if((m_iKeyNodeMir==2||m_iKeyNodeMir==3)&&
					fabs(pFaceNode->pos.z-pMirFaceNode->pos.z)<EPS&&
					fabs(pFaceNode->pos.y-pMirFaceNode->pos.y)<EPS&&
					fabs(pFaceNode->pos.x+pMirFaceNode->pos.x)<EPS)
				{	//�ؼ���Y��Գ�,���ڵ�Y��Գ�
					CLDSNode *pKeyNode=m_arrTemplateLdsNode[j];
					pNode=pKeyNode->GetMirNode(MIRMSG(2));
					if(pNode)
					{
						m_arrTemplateLdsNode.append(pNode);
						arrFaceNodeTemp[i].actual_pos.Set(pNode->Position(false).x,pNode->Position(false).y,pNode->Position(false).z);
						SolveNodePosExpression(&arrFaceNodeTemp[i],expression,TRUE);
					}
					else
					{
						f3dPoint mirpos=GetMirPos(pKeyNode->Position(false),MIRMSG(2));
						pNode=InsertNewLdsNode(mirpos,'T');
						m_arrTemplateLdsNode.append(pNode);
						pNode->iSeg=iSeg;	//�κ�
						arrFaceNodeTemp[i].actual_pos.Set(pNode->Position(false).x,pNode->Position(false).y,pNode->Position(false).z);
						SolveNodePosExpression(&arrFaceNodeTemp[i],expression,TRUE);
						pNode->AppendRelativeObj(RELATIVE_OBJECT(pKeyNode->handle,MIRMSG(2)));
						pKeyNode->AppendRelativeObj(RELATIVE_OBJECT(pNode->handle,MIRMSG(2)));
					}
					break;
				}
				else if(m_iKeyNodeMir==3&&
					fabs(pFaceNode->pos.x+pMirFaceNode->pos.x)<EPS&&
					fabs(pFaceNode->pos.y+pMirFaceNode->pos.y)<EPS&&
					fabs(pFaceNode->pos.z-pMirFaceNode->pos.z)<EPS)//Z��Գ�
				{	//�ؼ���XYZ��Գ�,���ڵ�XYZ��Գ�
					CLDSNode *pKeyNode=m_arrTemplateLdsNode[j];
					pNode=pKeyNode->GetMirNode(MIRMSG(3));
					if(pNode)
					{
						m_arrTemplateLdsNode.append(pNode);
						arrFaceNodeTemp[i].actual_pos.Set(pNode->Position(false).x,pNode->Position(false).y,pNode->Position(false).z);
						SolveNodePosExpression(&arrFaceNodeTemp[i],expression,TRUE);
					}
					else
					{
						f3dPoint mirpos=GetMirPos(pKeyNode->Position(false),MIRMSG(3));
						pNode=InsertNewLdsNode(mirpos,'T');
						m_arrTemplateLdsNode.append(pNode);
						pNode->iSeg=iSeg;	//�κ�
						arrFaceNodeTemp[i].actual_pos.Set(pNode->Position(false).x,pNode->Position(false).y,pNode->Position(false).z);
						SolveNodePosExpression(&arrFaceNodeTemp[i],expression,TRUE);
						pNode->AppendRelativeObj(RELATIVE_OBJECT(pKeyNode->handle,MIRMSG(3)));
						pKeyNode->AppendRelativeObj(RELATIVE_OBJECT(pNode->handle,MIRMSG(3)));
					}
					break;
				}
			}
			if(j==m_arrTemplateLdsNode.GetSize())
			{
				SolveNodePosExpression(pMirFaceNode,expression);
				pNode=InsertNewLdsNode(pMirFaceNode->actual_pos,'T');
				m_arrTemplateLdsNode.append(pNode);
				pNode->iSeg=iSeg;	//�κ�
			}
			/*{
				AfxMessageBox("��׼���в��ֹؼ��㣨�������κ������㣩δѡ���������׼��ʧ�ܣ�");
				throw -1;
			}*/
		}
		else if(pMirFaceNode->attach_type==1)
		{	//�˼���X����ֵ�����
			f3dPoint start,end;
			if(pMirFaceNode->attach_node_index[0]>=0&&pMirFaceNode->attach_node_index[0]<m_arrTemplateLdsNode.GetSize())
			{	//��Ч�����ڵ�
				start=m_arrTemplateLdsNode[pMirFaceNode->attach_node_index[0]]->Position(false);
			}
			else
#ifdef AFX_TARG_ENU_ENGLISH
				throw "Standard plane Lacks of reasonable attached node ,standard plane inserts failure��";
#else 
				throw "��׼���еĽڵ�ȱ�ٺ��������ڵ㣬�����׼��ʧ�ܣ�";
#endif
			if(pMirFaceNode->attach_node_index[1]>=0&&pMirFaceNode->attach_node_index[1]<m_arrTemplateLdsNode.GetSize())
			{	//��Ч�����ڵ�
				end=m_arrTemplateLdsNode[pMirFaceNode->attach_node_index[1]]->Position(false);
			}
			else
#ifdef AFX_TARG_ENU_ENGLISH
				throw "Standard plane Lacks of reasonable attached node ,standard plane inserts failure��";
#else 
				throw "��׼���еĽڵ�ȱ�ٺ��������ڵ㣬�����׼��ʧ�ܣ�";
#endif
			pMirFaceNode->actual_pos.x=expression.SolveExpression(pMirFaceNode->posx_expression);
			/*else	//��X���������ڵ㣬���˽ڵ��ԭʼ����λ��X�����趨X����Ϊ��׼
			{
				f3dPoint vec_sum,vec;
				vec_sum.x=arrFaceNodeTemp[pMirFaceNode->attach_node_index[1]]->pos.x-
					arrFaceNodeTemp[pMirFaceNode->attach_node_index[0]]->pos.x;
				vec_sum.y=arrFaceNodeTemp[pMirFaceNode->attach_node_index[1]]->pos.y-
					arrFaceNodeTemp[pMirFaceNode->attach_node_index[0]]->pos.y;
				vec.x=pMirFaceNode->pos.x-arrFaceNodeTemp[pMirFaceNode->attach_node_index[0]]->pos.x;
				vec.y=pMirFaceNode->pos.y-arrFaceNodeTemp[pMirFaceNode->attach_node_index[0]]->pos.y;
				double scale=vec.mod()/vec_sum.mod();
				pos=start+(end-start)*scale;
			}*/
			pMirFaceNode->actual_pos.y=start.y+(pMirFaceNode->actual_pos.x-start.x)*(end.y-start.y)/(end.x-start.x);
			pMirFaceNode->actual_pos.z=start.z+(pMirFaceNode->actual_pos.x-start.x)*(end.z-start.z)/(end.x-start.x);
			SolveNodePosExpression(&arrFaceNodeTemp[i],expression,TRUE);
			pNode=FindLDSNodeByPos(pMirFaceNode->actual_pos);
			if(pNode!=NULL)
				m_arrTemplateLdsNode.append(pNode);
			else //if(pNode==NULL)
			{	//�Ҳ����˽ڵ㣬������Ҫ�����½ڵ�
				pNode=InsertNewLdsNode(pMirFaceNode->actual_pos,'T');
				CLDSNode *pStartNode=m_arrTemplateLdsNode[pMirFaceNode->attach_node_index[0]];
				CLDSNode *pEndNode=m_arrTemplateLdsNode[pMirFaceNode->attach_node_index[1]];
				pNode->m_cPosCalType=1;	//X���겻���
				pNode->arrRelationNode[0]=pStartNode->handle;
				pNode->arrRelationNode[1]=pEndNode->handle;
				m_arrTemplateLdsNode.append(pNode);
				pNode->iSeg=iSeg;	//�κ�
			}
		}
		else if(pMirFaceNode->attach_type==2)
		{	//�˼���Y����ֵ�����
			f3dPoint start,end;
			if(pMirFaceNode->attach_node_index[0]>=0&&pMirFaceNode->attach_node_index[0]<m_arrTemplateLdsNode.GetSize())
			{	//��Ч�����ڵ�
				start=m_arrTemplateLdsNode[pMirFaceNode->attach_node_index[0]]->Position(false);
			}
			else
#ifdef AFX_TARG_ENU_ENGLISH
				throw "Standard plane Lacks of reasonable attached node ,standard plane inserts failure��";
#else 
				throw "��׼���еĽڵ�ȱ�ٺ��������ڵ㣬�����׼��ʧ�ܣ�";
#endif
			if(pMirFaceNode->attach_node_index[1]>=0&&pMirFaceNode->attach_node_index[1]<m_arrTemplateLdsNode.GetSize())
			{	//��Ч�����ڵ�
				end=m_arrTemplateLdsNode[pMirFaceNode->attach_node_index[1]]->Position(false);
			}
			else
#ifdef AFX_TARG_ENU_ENGLISH
				throw "Standard plane Lacks of reasonable attached node ,standard plane inserts failure��";
#else 
				throw "��׼���еĽڵ�ȱ�ٺ��������ڵ㣬�����׼��ʧ�ܣ�";
#endif
			pMirFaceNode->actual_pos.y=expression.SolveExpression(pMirFaceNode->posy_expression);
			/*else	//��Y���������ڵ㣬���˽ڵ��ԭʼ����λ��Y�����趨X����Ϊ��׼
			{
				f3dPoint vec_sum,vec;
				vec_sum.x=arrFaceNodeTemp[pMirFaceNode->attach_node_index[1]]->pos.x-
					arrFaceNodeTemp[pMirFaceNode->attach_node_index[0]]->pos.x;
				vec_sum.y=arrFaceNodeTemp[pMirFaceNode->attach_node_index[1]]->pos.y-
					arrFaceNodeTemp[pMirFaceNode->attach_node_index[0]]->pos.y;
				vec.x=pMirFaceNode->pos.x-arrFaceNodeTemp[pMirFaceNode->attach_node_index[0]]->pos.x;
				vec.y=pMirFaceNode->pos.y-arrFaceNodeTemp[pMirFaceNode->attach_node_index[0]]->pos.y;
				double scale=vec.mod()/vec_sum.mod();
				pos=start+(end-start)*scale;
			}*/
			pMirFaceNode->actual_pos.x=start.z+(pMirFaceNode->actual_pos.y-start.y)*(end.x-start.x)/(end.y-start.y);
			pMirFaceNode->actual_pos.z=start.z+(pMirFaceNode->actual_pos.y-start.y)*(end.z-start.z)/(end.y-start.y);
			pNode=FindLDSNodeByPos(pMirFaceNode->actual_pos);
			if(pNode!=NULL)
				m_arrTemplateLdsNode.append(pNode);
			else //if(pNode==NULL)
			{	//�Ҳ����˽ڵ㣬������Ҫ�����½ڵ�
				pNode=InsertNewLdsNode(pMirFaceNode->actual_pos,'T');
				CLDSNode *pStartNode=m_arrTemplateLdsNode[pMirFaceNode->attach_node_index[0]];
				CLDSNode *pEndNode=m_arrTemplateLdsNode[pMirFaceNode->attach_node_index[1]];
				pNode->m_cPosCalType=2;	//X���겻���
				pNode->arrRelationNode[0]=pStartNode->handle;
				pNode->arrRelationNode[1]=pEndNode->handle;
				m_arrTemplateLdsNode.append(pNode);
				pNode->iSeg=iSeg;	//�κ�
			}
		}
		else if(pMirFaceNode->attach_type==3)
		{	//�˼���Z����ֵ�����
			f3dPoint start,end;
			if(pMirFaceNode->attach_node_index[0]>=0&&pMirFaceNode->attach_node_index[0]<m_arrTemplateLdsNode.GetSize())
			{	//��Ч�����ڵ�
				start=m_arrTemplateLdsNode[pMirFaceNode->attach_node_index[0]]->Position(false);
			}
			else
#ifdef AFX_TARG_ENU_ENGLISH
				throw "Standard plane Lacks of reasonable attached node ,standard plane inserts failure��";
#else 
				throw "��׼���еĽڵ�ȱ�ٺ��������ڵ㣬�����׼��ʧ�ܣ�";
#endif
			if(pMirFaceNode->attach_node_index[1]>=0&&pMirFaceNode->attach_node_index[1]<m_arrTemplateLdsNode.GetSize())
			{	//��Ч�����ڵ�
				end=m_arrTemplateLdsNode[pMirFaceNode->attach_node_index[1]]->Position(false);
			}
			else
#ifdef AFX_TARG_ENU_ENGLISH
				throw "Standard plane Lacks of reasonable attached node ,standard plane inserts failure��";
#else 
				throw "��׼���еĽڵ�ȱ�ٺ��������ڵ㣬�����׼��ʧ�ܣ�";
#endif
			pMirFaceNode->actual_pos.z=expression.SolveExpression(pMirFaceNode->posz_expression);
			/*else	//��Z���������ڵ㣬���˽ڵ��ԭʼ����λ��Z�����趨X����Ϊ��׼
			{
				f3dPoint vec_sum,vec;
				vec_sum.x=arrFaceNodeTemp[pMirFaceNode->attach_node_index[1]]->pos.x-
					arrFaceNodeTemp[pMirFaceNode->attach_node_index[0]]->pos.x;
				vec_sum.y=arrFaceNodeTemp[pMirFaceNode->attach_node_index[1]]->pos.y-
					arrFaceNodeTemp[pMirFaceNode->attach_node_index[0]]->pos.y;
				vec.x=pMirFaceNode->pos.x-arrFaceNodeTemp[pMirFaceNode->attach_node_index[0]]->pos.x;
				vec.y=pMirFaceNode->pos.y-arrFaceNodeTemp[pMirFaceNode->attach_node_index[0]]->pos.y;
				double scale=vec.mod()/vec_sum.mod();
				pos=start+(end-start)*scale;
			}*/
			pMirFaceNode->actual_pos.x=start.x+(pMirFaceNode->actual_pos.z-start.z)*(end.x-start.x)/(end.z-start.z);
			pMirFaceNode->actual_pos.y=start.y+(pMirFaceNode->actual_pos.z-start.z)*(end.y-start.y)/(end.z-start.z);
			pNode=FindLDSNodeByPos(pMirFaceNode->actual_pos);
			if(pNode!=NULL)
				m_arrTemplateLdsNode.append(pNode);
			else //if(pNode==NULL)
			{	//�Ҳ����˽ڵ㣬������Ҫ�����½ڵ�
				pNode=InsertNewLdsNode(pMirFaceNode->actual_pos,'T');
				CLDSNode *pStartNode=m_arrTemplateLdsNode[pMirFaceNode->attach_node_index[0]];
				CLDSNode *pEndNode=m_arrTemplateLdsNode[pMirFaceNode->attach_node_index[1]];
				pNode->m_cPosCalType=3;	//Z���겻���
				pNode->arrRelationNode[0]=pStartNode->handle;
				pNode->arrRelationNode[1]=pEndNode->handle;
				m_arrTemplateLdsNode.append(pNode);
				pNode->iSeg=iSeg;	//�κ�
			}
		}
		else if(pMirFaceNode->attach_type==4)
		{	//���˼����潻��
			CLDSNode *pStartNode1,*pStartNode2,*pEndNode1,*pEndNode2;
			if(pMirFaceNode->attach_node_index[0]>=0&&pMirFaceNode->attach_node_index[0]<m_arrTemplateLdsNode.GetSize())
			{	//��Ч�����ڵ�
				pStartNode1=m_arrTemplateLdsNode[pMirFaceNode->attach_node_index[0]];
			}
			else
#ifdef AFX_TARG_ENU_ENGLISH
				throw "Standard plane Lacks of reasonable attached node ,standard plane inserts failure��";
#else 
				throw "��׼���еĽڵ�ȱ�ٺ��������ڵ㣬�����׼��ʧ�ܣ�";
#endif
			if(pMirFaceNode->attach_node_index[1]>=0&&pMirFaceNode->attach_node_index[1]<m_arrTemplateLdsNode.GetSize())
				pEndNode1=m_arrTemplateLdsNode[pMirFaceNode->attach_node_index[1]];	//��Ч�����ڵ�
			else
#ifdef AFX_TARG_ENU_ENGLISH
				throw "Standard plane Lacks of reasonable attached node ,standard plane inserts failure��";
#else 
				throw "��׼���еĽڵ�ȱ�ٺ��������ڵ㣬�����׼��ʧ�ܣ�";
#endif
			if(pMirFaceNode->attach_node_index[2]>=0&&pMirFaceNode->attach_node_index[2]<m_arrTemplateLdsNode.GetSize())
				pStartNode2=m_arrTemplateLdsNode[pMirFaceNode->attach_node_index[2]];	//��Ч�����ڵ�
			else
#ifdef AFX_TARG_ENU_ENGLISH
				throw "Standard plane Lacks of reasonable attached node ,standard plane inserts failure��";
#else 
				throw "��׼���еĽڵ�ȱ�ٺ��������ڵ㣬�����׼��ʧ�ܣ�";
#endif
			if(pMirFaceNode->attach_node_index[3]>=0&&pMirFaceNode->attach_node_index[3]<m_arrTemplateLdsNode.GetSize())
				pEndNode2=m_arrTemplateLdsNode[pMirFaceNode->attach_node_index[3]];	//��Ч�����ڵ�
			else
#ifdef AFX_TARG_ENU_ENGLISH
				throw "Standard plane Lacks of reasonable attached node ,standard plane inserts failure��";
#else 
				throw "��׼���еĽڵ�ȱ�ٺ��������ڵ㣬�����׼��ʧ�ܣ�";
#endif
			f3dPoint pos;
			int iRet=Int3dll(pStartNode1->Position(false),pEndNode1->Position(false),pStartNode2->Position(false),pEndNode2->Position(false),pos);
			pNode=FindLDSNodeByPos(pos);
			if(pNode!=NULL)
				m_arrTemplateLdsNode.append(pNode);
			else //if(pNode==NULL)
			{	//�Ҳ����˽ڵ㣬������Ҫ�����½ڵ�
				pNode=InsertNewLdsNode(pos,'T');
				CLDSNode *pStartNode1=m_arrTemplateLdsNode[pMirFaceNode->attach_node_index[0]];
				CLDSNode *pEndNode1=m_arrTemplateLdsNode[pMirFaceNode->attach_node_index[1]];
				CLDSNode *pStartNode2=m_arrTemplateLdsNode[pMirFaceNode->attach_node_index[2]];
				CLDSNode *pEndNode2=m_arrTemplateLdsNode[pMirFaceNode->attach_node_index[3]];
				pNode->m_cPosCalType=4;	//���潻��
				pNode->arrRelationNode[0]=pStartNode1->handle;
				pNode->arrRelationNode[1]=pEndNode1->handle;
				pNode->arrRelationNode[2]=pStartNode2->handle;
				pNode->arrRelationNode[3]=pEndNode2->handle;
				m_arrTemplateLdsNode.append(pNode);
				pNode->iSeg=iSeg;	//�κ�
			}
		}
		else if(pMirFaceNode->attach_type==5)
		{	//�˼��ϱ����ȷֵ�
			f3dPoint start,end;
			if(pMirFaceNode->attach_node_index[0]>=0&&pMirFaceNode->attach_node_index[0]<m_arrTemplateLdsNode.GetSize())
				start=m_arrTemplateLdsNode[pMirFaceNode->attach_node_index[0]]->Position(false);	//��Ч�����ڵ�
			else
#ifdef AFX_TARG_ENU_ENGLISH
				throw "Standard plane Lacks of reasonable attached node ,standard plane inserts failure��";
#else 
				throw "��׼���еĽڵ�ȱ�ٺ��������ڵ㣬�����׼��ʧ�ܣ�";
#endif
			if(pMirFaceNode->attach_node_index[1]>=0&&pMirFaceNode->attach_node_index[1]<m_arrTemplateLdsNode.GetSize())
				end=m_arrTemplateLdsNode[pMirFaceNode->attach_node_index[1]]->Position(false);	//��Ч�����ڵ�
			else
#ifdef AFX_TARG_ENU_ENGLISH
				throw "Standard plane Lacks of reasonable attached node ,standard plane inserts failure��";
#else 
				throw "��׼���еĽڵ�ȱ�ٺ��������ڵ㣬�����׼��ʧ�ܣ�";
#endif
			f3dPoint pos;
			double real_scale=pMirFaceNode->attach_scale;
			if(pMirFaceNode->attach_scale<0)
			{
				real_scale=-pMirFaceNode->attach_scale;
				real_scale=0.1*((int)real_scale)/(real_scale-(int)real_scale);
			}
			pos.x=start.x+real_scale*(end.x-start.x);
			pos.y=start.y+real_scale*(end.y-start.y);
			pos.z=start.z+real_scale*(end.z-start.z);
			pNode=FindLDSNodeByPos(pos);
			if(pNode!=NULL)
				m_arrTemplateLdsNode.append(pNode);
			else //if(pNode==NULL)
			{	//�Ҳ����˽ڵ㣬������Ҫ�����½ڵ�
				pNode=InsertNewLdsNode(pos,'T');
				CLDSNode *pStartNode=m_arrTemplateLdsNode[pMirFaceNode->attach_node_index[0]];
				CLDSNode *pEndNode=m_arrTemplateLdsNode[pMirFaceNode->attach_node_index[1]];
				pNode->m_cPosCalType=5;	//�����ȷֵ�
				pNode->attach_scale=real_scale;
				pNode->arrRelationNode[0]=pStartNode->handle;
				pNode->arrRelationNode[1]=pEndNode->handle;
				m_arrTemplateLdsNode.append(pNode);
				pNode->iSeg=iSeg;	//�κ�
			}
		}
		else if(pMirFaceNode->attach_type==6)
		{	//��׼����ָ��XY�����
			f3dPoint start,end,face_pick,pos;
			if( pMirFaceNode->attach_node_index[0]>=0&&pMirFaceNode->attach_node_index[0]<m_arrTemplateLdsNode.GetSize() &&
				pMirFaceNode->attach_node_index[1]>=0&&pMirFaceNode->attach_node_index[1]<m_arrTemplateLdsNode.GetSize() &&
				pMirFaceNode->attach_node_index[2]>=0&&pMirFaceNode->attach_node_index[2]<m_arrTemplateLdsNode.GetSize())
			{	//��Ч�����ڵ�
				start=m_arrTemplateLdsNode[pMirFaceNode->attach_node_index[0]]->Position(false);
				end=m_arrTemplateLdsNode[pMirFaceNode->attach_node_index[1]]->Position(false);
				face_pick=m_arrTemplateLdsNode[pMirFaceNode->attach_node_index[2]]->Position(false);
				f3dPoint line_vec1=end-start;
				f3dPoint line_vec2=end-face_pick;
				f3dPoint face_norm=line_vec1^line_vec2;
				if(strlen(pMirFaceNode->posx_expression)>0)
					pos.x=expression.SolveExpression(pMirFaceNode->posx_expression);
				if(strlen(pMirFaceNode->posy_expression)>0)
					pos.y=expression.SolveExpression(pMirFaceNode->posy_expression);
				Int3dlf(pos,f3dPoint(pos.x,pos.y,0),f3dPoint(0,0,1),face_pick,face_norm);
				pNode=FindLDSNodeByPos(pos);
				if(pNode!=NULL)
					m_arrTemplateLdsNode.append(pNode);
				else //if(pNode==NULL)
				{	//�Ҳ����˽ڵ㣬������Ҫ�����½ڵ�
					pNode=InsertNewLdsNode(pos,'T');
					pNode->m_cPosCalType=8;	//��׼����ָ��XY�����
					DATUMPLANE_STRU *pPlane=NULL;
					for(pPlane=planeList.GetFirst();pPlane;pPlane=planeList.GetNext())
					{
						if(IsSamePlane(pPlane,pMirFaceNode->attach_node_index))
							break;
					}
					if(pPlane==NULL)
					{
						pPlane=planeList.append();
						CLDSPlane *pDatumPlane=m_pTower->Plane.append();
						CObjNoGroup *pPointNoGroup=m_pTower->NoManager.FromGroupID(MODEL_DATUM_POINT_GROUP);
						CObjNoGroup *pPlaneNoGroup=m_pTower->NoManager.FromGroupID(MODEL_DATUM_PLANE_GROUP);
						pDatumPlane->iNo=pPlaneNoGroup->EnumIdleNo();
						pPlaneNoGroup->SetNoState(pDatumPlane->iNo);
						sprintf(pDatumPlane->description,"%d",pDatumPlane->iNo);
						pPlane->hPlane=pDatumPlane->handle;
						for(short jp=0;jp<3;jp++)	//���ڽ����޸ĺ����Ͳ�ͬ������ֱ����memcpy���� wjh-2014.11.27
							pPlane->index[jp]=pMirFaceNode->attach_node_index[jp];
						for(int kk=0;kk<3;kk++)
						{
							CLDSPoint *pDatumPoint=m_pTower->Point.append();
							pDatumPoint->iNo=pPointNoGroup->EnumIdleNo();
							pPointNoGroup->SetNoState(pDatumPoint->iNo);
							pDatumPoint->datum_pos_style=8;
							pDatumPoint->des_para.hNode=m_arrTemplateLdsNode[pMirFaceNode->attach_node_index[kk]]->handle;
							pDatumPlane->hPointArr[kk]=pDatumPoint->handle;
						}
					}
					pNode->hRelationPlane=pPlane->hPlane;
					m_arrTemplateLdsNode.append(pNode);
					pNode->iSeg=iSeg;	//�κ�
				}
				pMirFaceNode->actual_pos=pos;
				SolveNodePosExpression(pMirFaceNode,expression,TRUE);
			}
			else
#ifdef AFX_TARG_ENU_ENGLISH
				throw "Standard plane Lacks of reasonable attached node ,standard plane inserts failure��";
#else 
				throw "��׼���еĽڵ�ȱ�ٺ��������ڵ㣬�����׼��ʧ�ܣ�";
#endif
		}
		else if(pMirFaceNode->attach_type==7)
		{	//��׼����ָ��YZ�����
			f3dPoint start,end,face_pick,pos;
			if( pMirFaceNode->attach_node_index[0]>=0&&pMirFaceNode->attach_node_index[0]<m_arrTemplateLdsNode.GetSize() &&
				pMirFaceNode->attach_node_index[1]>=0&&pMirFaceNode->attach_node_index[1]<m_arrTemplateLdsNode.GetSize() &&
				pMirFaceNode->attach_node_index[2]>=0&&pMirFaceNode->attach_node_index[2]<m_arrTemplateLdsNode.GetSize())
			{	//��Ч�����ڵ�
				start=m_arrTemplateLdsNode[pMirFaceNode->attach_node_index[0]]->Position(false);
				end=m_arrTemplateLdsNode[pMirFaceNode->attach_node_index[1]]->Position(false);
				face_pick=m_arrTemplateLdsNode[pMirFaceNode->attach_node_index[2]]->Position(false);
				f3dPoint line_vec1=end-start;
				f3dPoint line_vec2=end-face_pick;
				f3dPoint face_norm=line_vec1^line_vec2;
				if(strlen(pMirFaceNode->posy_expression)>0)
					pos.y=expression.SolveExpression(pMirFaceNode->posy_expression);
				if(strlen(pMirFaceNode->posz_expression)>0)
					pos.z=expression.SolveExpression(pMirFaceNode->posz_expression);
				Int3dlf(pos,f3dPoint(0,pos.y,pos.z),f3dPoint(1,0,0),face_pick,face_norm);
				pNode=FindLDSNodeByPos(pos);
				if(pNode!=NULL)
					m_arrTemplateLdsNode.append(pNode);
				else //if(pNode==NULL)
				{	//�Ҳ����˽ڵ㣬������Ҫ�����½ڵ�
					pNode=InsertNewLdsNode(pos,'T');
					pNode->m_cPosCalType=9;	//��׼����ָ��XY�����
					DATUMPLANE_STRU *pPlane=NULL;
					for(pPlane=planeList.GetFirst();pPlane;pPlane=planeList.GetNext())
					{
						if(IsSamePlane(pPlane,pMirFaceNode->attach_node_index))
							break;
					}
					if(pPlane==NULL)
					{
						pPlane=planeList.append();
						CLDSPlane *pDatumPlane=m_pTower->Plane.append();
						CObjNoGroup *pPointNoGroup=m_pTower->NoManager.FromGroupID(MODEL_DATUM_POINT_GROUP);
						CObjNoGroup *pPlaneNoGroup=m_pTower->NoManager.FromGroupID(MODEL_DATUM_PLANE_GROUP);
						pDatumPlane->iNo=pPlaneNoGroup->EnumIdleNo();
						pPlaneNoGroup->SetNoState(pDatumPlane->iNo);
						sprintf(pDatumPlane->description,"%d",pDatumPlane->iNo);
						pPlane->hPlane=pDatumPlane->handle;
						for(short jp=0;jp<3;jp++)	//���ڽ����޸ĺ����Ͳ�ͬ������ֱ����memcpy���� wjh-2014.11.27
							pPlane->index[jp]=pMirFaceNode->attach_node_index[jp];
						for(int kk=0;kk<3;kk++)
						{
							CLDSPoint *pDatumPoint=m_pTower->Point.append();
							pDatumPoint->iNo=pPointNoGroup->EnumIdleNo();
							pPointNoGroup->SetNoState(pDatumPoint->iNo);
							pDatumPoint->datum_pos_style=8;
							pDatumPoint->des_para.hNode=m_arrTemplateLdsNode[pMirFaceNode->attach_node_index[kk]]->handle;
							pDatumPlane->hPointArr[kk]=pDatumPoint->handle;
						}
					}
					pNode->hRelationPlane=pPlane->hPlane;
					m_arrTemplateLdsNode.append(pNode);
					pNode->iSeg=iSeg;	//�κ�
				}
				pMirFaceNode->actual_pos=pos;
				SolveNodePosExpression(pMirFaceNode,expression,TRUE);
			}
			else
#ifdef AFX_TARG_ENU_ENGLISH
				throw "Standard plane Lacks of reasonable attached node ,standard plane inserts failure��";
#else 
				throw "��׼���еĽڵ�ȱ�ٺ��������ڵ㣬�����׼��ʧ�ܣ�";
#endif
		}
		else if(pMirFaceNode->attach_type==8)
		{	//��׼����ָ��XZ�����
			f3dPoint start,end,face_pick,pos;
			if( pMirFaceNode->attach_node_index[0]>=0&&pMirFaceNode->attach_node_index[0]<m_arrTemplateLdsNode.GetSize() &&
				pMirFaceNode->attach_node_index[1]>=0&&pMirFaceNode->attach_node_index[1]<m_arrTemplateLdsNode.GetSize() &&
				pMirFaceNode->attach_node_index[2]>=0&&pMirFaceNode->attach_node_index[2]<m_arrTemplateLdsNode.GetSize())
			{	//��Ч�����ڵ�
				start=m_arrTemplateLdsNode[pMirFaceNode->attach_node_index[0]]->Position(false);
				end=m_arrTemplateLdsNode[pMirFaceNode->attach_node_index[1]]->Position(false);
				face_pick=m_arrTemplateLdsNode[pMirFaceNode->attach_node_index[2]]->Position(false);
				f3dPoint line_vec1=end-start;
				f3dPoint line_vec2=end-face_pick;
				f3dPoint face_norm=line_vec1^line_vec2;
				if(strlen(pMirFaceNode->posx_expression)>0)
					pos.x=expression.SolveExpression(pMirFaceNode->posx_expression);
				if(strlen(pMirFaceNode->posz_expression)>0)
					pos.z=expression.SolveExpression(pMirFaceNode->posz_expression);
				Int3dlf(pos,f3dPoint(pos.x,0,pos.z),f3dPoint(0,1,0),face_pick,face_norm);
				pNode=FindLDSNodeByPos(pos);
				if(pNode!=NULL)
					m_arrTemplateLdsNode.append(pNode);
				else //if(pNode==NULL)
				{	//�Ҳ����˽ڵ㣬������Ҫ�����½ڵ�
					pNode=InsertNewLdsNode(pos,'T');
					pNode->m_cPosCalType=10;	//��׼����ָ��XY�����
					DATUMPLANE_STRU *pPlane=NULL;
					for(pPlane=planeList.GetFirst();pPlane;pPlane=planeList.GetNext())
					{
						if(IsSamePlane(pPlane,pMirFaceNode->attach_node_index))
							break;
					}
					if(pPlane==NULL)
					{
						pPlane=planeList.append();
						CLDSPlane *pDatumPlane=m_pTower->Plane.append();
						CObjNoGroup *pPointNoGroup=m_pTower->NoManager.FromGroupID(MODEL_DATUM_POINT_GROUP);
						CObjNoGroup *pPlaneNoGroup=m_pTower->NoManager.FromGroupID(MODEL_DATUM_PLANE_GROUP);
						pDatumPlane->iNo=pPlaneNoGroup->EnumIdleNo();
						pPlaneNoGroup->SetNoState(pDatumPlane->iNo);
						sprintf(pDatumPlane->description,"%d",pDatumPlane->iNo);
						pPlane->hPlane=pDatumPlane->handle;
						for(short jp=0;jp<3;jp++)	//���ڽ����޸ĺ����Ͳ�ͬ������ֱ����memcpy���� wjh-2014.11.27
							pPlane->index[jp]=pMirFaceNode->attach_node_index[jp];
						for(int kk=0;kk<3;kk++)
						{
							CLDSPoint *pDatumPoint=m_pTower->Point.append();
							pDatumPoint->iNo=pPointNoGroup->EnumIdleNo();
							pPointNoGroup->SetNoState(pDatumPoint->iNo);
							pDatumPoint->datum_pos_style=8;
							pDatumPoint->des_para.hNode=m_arrTemplateLdsNode[pMirFaceNode->attach_node_index[kk]]->handle;
							pDatumPlane->hPointArr[kk]=pDatumPoint->handle;
						}
					}
					pNode->hRelationPlane=pPlane->hPlane;
					m_arrTemplateLdsNode.append(pNode);
					pNode->iSeg=iSeg;	//�κ�
				}
				pMirFaceNode->actual_pos=pos;
				SolveNodePosExpression(pMirFaceNode,expression,TRUE);
			}
			else
#ifdef AFX_TARG_ENU_ENGLISH
				throw "Standard plane Lacks of reasonable attached node ,standard plane inserts failure��";
#else 
				throw "��׼���еĽڵ�ȱ�ٺ��������ڵ㣬�����׼��ʧ�ܣ�";
#endif
		}
	}
	//2.2ԭʼ��׼��˼��������
#ifdef __ANGLE_PART_INC_//!defined(__TSA_)&&!defined(__TSA_FILE_)
	CStringKeyHashTable<CLDSLineAngle*>poleSetTbl;
#else
	CStringKeyHashTable<CLDSLinePart*>poleSetTbl;
#endif
	poleSetTbl.CreateHashTable(pHeadTemplate->pole_list.GetNodeNum());
	for(pFacePole=pHeadTemplate->pole_list.GetFirst();pFacePole;pFacePole=pHeadTemplate->pole_list.GetNext())
	{
		CLDSNode *pStart=m_arrTemplateLdsNode[pFacePole->start_i];
		CLDSNode *pEnd  =m_arrTemplateLdsNode[pFacePole->end_i];
#ifdef __ANGLE_PART_INC_//!defined(__TSA_)&&!defined(__TSA_FILE_)
		for(pPole=(CLDSLineAngle*)m_pTower->Parts.GetFirst(CLS_LINEANGLE);pPole;pPole=(CLDSLineAngle*)m_pTower->Parts.GetNext(CLS_LINEANGLE))
#else
		for(pPole=(CLDSLinePart*)m_pTower->Parts.GetFirst(CLS_LINEPART);pPole;pPole=(CLDSLinePart*)m_pTower->Parts.GetNext(CLS_LINEPART))
#endif
		{
			if(pPole->pStart==NULL||pPole->pEnd==NULL)
				continue;
			else if(!(pPole->cfgword.And(m_pTower->GetDefaultCfgPartNo())))
				continue;
			else if(pPole->pStart==pStart&&pPole->pEnd==pEnd)
				break;
			else if(pPole->pStart==pEnd&&pPole->pEnd==pStart)
				break;
			else
			{
				f3dLine line;
				line.startPt=pPole->pStart->Position(false);
				line.endPt=pPole->pEnd->Position(false);
				pPole->SetModified();	//���ں���������޸ĸ˼�����,�����趨�޸ı��,����Undo��ʧ�� wjh 2011.8.12
				if(line.PtInLine(pStart->Position(false))>0&&line.PtInLine(pEnd->Position(false))>0)
					break;
			}
		}
		if(pPole)
			m_arrTemplateLdsLinePart.append(pPole);
		else
		{
#ifdef __ANGLE_PART_INC_//!defined(__TSA_)&&!defined(__TSA_FILE_)
			pPole=(CLDSLineAngle*)m_pTower->Parts.append(CLS_LINEANGLE);
#else
			pPole=(CLDSLinePart*)m_pTower->Parts.append(CLS_LINEPART);
#endif
			pPole->iSeg=iSeg;	//�κ�
			pPole->pStart=pStart;
			pPole->pEnd=pEnd;
			pPole->layer(0)='T';
			pPole->layer(1)=pFacePole->sLayer[1];
			if(pPole->layer(1)=='B')
				pPole->layer(1)='F';
			if(pPole->IsAuxPole())
				pPole->connectStart.N=pPole->connectEnd.N=1;	//����Ĭ��һ����˨���Ա����Զ���ֽڵ����������˨��� wjh-2017.12.17
			pPole->layer(2)=CalPoleQuad(pPole->pStart->Position(false),pPole->pEnd->Position(false));
#ifdef __ANGLE_PART_INC_//!defined(__TSA_)&&!defined(__TSA_FILE_)
			if(pFacePole->wxLayout.ciLayoutStyle==1)
			{
				if(pFacePole->eyeSideNorm.IsZero())
					pPole->des_norm_x.near_norm.Set(0,-1,0);
				else
					pPole->des_norm_x.near_norm=-pFacePole->eyeSideNorm;
				pPole->des_norm_y.bByOtherWing=TRUE;
				pPole->desStartPos.wing_x_offset.gStyle=pPole->desEndPos.wing_x_offset.gStyle=4;
				pPole->desStartPos.wing_x_offset.offsetDist=pPole->desEndPos.wing_x_offset.offsetDist=0;
				pPole->desStartPos.wing_y_offset.gStyle=pPole->desEndPos.wing_y_offset.gStyle=0;
			}
			else if(pFacePole->wxLayout.ciLayoutStyle==2)
			{
				if(pFacePole->eyeSideNorm.IsZero())
					pPole->des_norm_y.near_norm.Set(0,1,0);
				else
					pPole->des_norm_y.near_norm=pFacePole->eyeSideNorm;
				pPole->des_norm_x.bByOtherWing=TRUE;
				pPole->desStartPos.wing_y_offset.gStyle=pPole->desEndPos.wing_y_offset.gStyle=4;
				pPole->desStartPos.wing_y_offset.offsetDist=pPole->desEndPos.wing_y_offset.offsetDist=0;
				pPole->desStartPos.wing_x_offset.gStyle=pPole->desEndPos.wing_x_offset.gStyle=0;
			}
			else if(pFacePole->wxLayout.ciLayoutStyle==3)
			{
				if(pFacePole->eyeSideNorm.IsZero())
					pPole->des_norm_x.near_norm.Set(0,1,0);
				else
					pPole->des_norm_x.near_norm=pFacePole->eyeSideNorm;
				pPole->des_norm_y.bByOtherWing=TRUE;
				pPole->desStartPos.wing_x_offset.gStyle=pPole->desEndPos.wing_x_offset.gStyle=4;
				pPole->desStartPos.wing_x_offset.offsetDist=pPole->desEndPos.wing_x_offset.offsetDist=0;
				pPole->desStartPos.wing_y_offset.gStyle=pPole->desEndPos.wing_y_offset.gStyle=0;
			}
			else
			{
				if(pFacePole->eyeSideNorm.IsZero())
					pPole->des_norm_y.near_norm.Set(0,-1,0);
				else
					pPole->des_norm_y.near_norm=-pFacePole->eyeSideNorm;
				pPole->des_norm_x.bByOtherWing=TRUE;
				pPole->desStartPos.wing_y_offset.gStyle=pPole->desEndPos.wing_y_offset.gStyle=4;
				pPole->desStartPos.wing_y_offset.offsetDist=pPole->desEndPos.wing_y_offset.offsetDist=0;
				pPole->desStartPos.wing_x_offset.gStyle=pPole->desEndPos.wing_x_offset.gStyle=0;
			}
#else
			pPole->des_wing_norm.wLayoutStyle=(WORD)pFacePole->layout_style;
			if(pFacePole->wxLayout.ciLayoutStyle==1)
			{
				if(pFacePole->eyeSideNorm.IsZero())
					pPole->des_wing_norm.viewNorm.Set(0,-1,0);
				else
					pPole->des_wing_norm.viewNorm=-pFacePole->eyeSideNorm;
			}
			else if(pFacePole->wxLayout.ciLayoutStyle==2)
			{
				if(pFacePole->eyeSideNorm.IsZero())
					pPole->des_wing_norm.viewNorm.Set(0,1,0);
				else
					pPole->des_wing_norm.viewNorm=pFacePole->eyeSideNorm;
			}
			else if(pFacePole->wxLayout.ciLayoutStyle==3)
			{
				if(pFacePole->eyeSideNorm.IsZero())
					pPole->des_wing_norm.viewNorm.Set(0,1,0);
				else
					pPole->des_wing_norm.viewNorm=pFacePole->eyeSideNorm;
			}
			else
			{
				if(pFacePole->eyeSideNorm.IsZero())
					pPole->des_wing_norm.viewNorm.Set(0,-1,0);
				else
					pPole->des_wing_norm.viewNorm=-pFacePole->eyeSideNorm;
			}
#endif
#ifdef __PART_DESIGN_INC_
			if(pFacePole->start_force_type==0)
				pPole->start_force_type=CENTRIC_FORCE;
			else
				pPole->start_force_type=ECCENTRIC_FORCE;
			if(pFacePole->end_force_type==0)
				pPole->end_force_type=CENTRIC_FORCE;
			else
				pPole->end_force_type=ECCENTRIC_FORCE;;
#endif
			pPole->SetStart(pStart->Position(false));	//����ʼֵ
			pPole->SetEnd(pEnd->Position(false));		//����ʼֵ
			m_arrTemplateLdsLinePart.append(pPole);
		}
		poleSetTbl.SetValueAt(pFacePole->sTitle,pPole);
	}
	//2.3�²���ԭʼ��׼���ڵĽڵ�˼���ϵ��Ϣ��д
	//�ڵ㸸�˼���Ϣ�������˼���Ϣ
	for(i=0;i<m_arrTemplateLdsNode.GetSize();i++)
	{
		pNode=m_arrTemplateLdsNode[i];
		if(pNode->m_cPosCalType==4)	//�����
		{
			pNode->arrRelationPole[0]=pNode->arrRelationPole[1]=0;
			for(int j=0;j<m_arrTemplateLdsLinePart.GetSize();j++)
			{
#ifdef __ANGLE_PART_INC_//__LDS_
				pPole=(CLDSLineAngle*)m_arrTemplateLdsLinePart[j];
#else
				pPole=m_arrTemplateLdsLinePart[j];
#endif
				if((pPole->pStart->handle==pNode->arrRelationNode[0]&&pPole->pEnd->handle==pNode->arrRelationNode[1])||
					(pPole->pStart->handle==pNode->arrRelationNode[1]&&pPole->pEnd->handle==pNode->arrRelationNode[0]))
					pNode->arrRelationPole[0]=pPole->handle;
				else if((pPole->pStart->handle==pNode->arrRelationNode[2]&&pPole->pEnd->handle==pNode->arrRelationNode[3])||
					(pPole->pStart->handle==pNode->arrRelationNode[3]&&pPole->pEnd->handle==pNode->arrRelationNode[2]))
					pNode->arrRelationPole[1]=pPole->handle;
			}
		}
		else if(pNode->m_cPosCalType!=0)	//�����κ������ڵ�
		{
			for(int j=0;j<m_arrTemplateLdsLinePart.GetSize();j++)
			{
#ifdef __ANGLE_PART_INC_//__LDS_
				pPole=(CLDSLineAngle*)m_arrTemplateLdsLinePart[j];
#else
				pPole=m_arrTemplateLdsLinePart[j];
#endif
				if((pPole->pStart->handle==pNode->arrRelationNode[0]&&pPole->pEnd->handle==pNode->arrRelationNode[1])||
					(pPole->pStart->handle==pNode->arrRelationNode[1]&&pPole->pEnd->handle==pNode->arrRelationNode[0]))
				{
					pNode->arrRelationPole[0]=pPole->handle;
					break;
				}
			}
		}
		if(pNode->hFatherPart<0x20)
		{	//�޸��˼���Ϣ
			CLDSLinePart *pViceFatherPole=NULL;
			for(int j=0;j<m_arrTemplateLdsLinePart.GetSize();j++)
			{
#ifdef __ANGLE_PART_INC_//__LDS_
				pPole=(CLDSLineAngle*)m_arrTemplateLdsLinePart[j];
#else
				pPole=m_arrTemplateLdsLinePart[j];
#endif
				f3dLine line(pPole->pStart->Position(false),pPole->pEnd->Position(false));
				int ret=line.PtInLine(pNode->Position(false));
				if(ret==2)		//���ڸ˼�ֱ����
				{
					pNode->hFatherPart=pPole->handle;
					pNode->layer(2)=pPole->layer(2);
					break;
				}
				else if(ret==1&&pViceFatherPole==NULL)	//׼���˼�(�ڵ�Ϊ�˼��˽ڵ�)
					pViceFatherPole=pPole;
			}
			if(pNode->hFatherPart<0x20)	//δ�ҵ����˼������Ե�һ��׼���˼�Ϊ���˼�
			{
				pNode->hFatherPart=pViceFatherPole->handle;
				pNode->layer(2)=pViceFatherPole->layer(2);
			}
		}
	}
	//2.4����˼�֫���߷���λ��
	for(pFacePole=pHeadTemplate->pole_list.GetFirst();pFacePole;pFacePole=pHeadTemplate->pole_list.GetNext())
	{
		if(poleSetTbl.GetValueAt(pFacePole->sTitle,pPole))
		{
#ifdef __ANGLE_PART_INC_//!defined(__TSA_)&&!defined(__TSA_FILE_)
			CLDSLineAngle *pWingRefPole=NULL;
			if(pFacePole->wxLayout.ciLayoutStyle==1)
			{
				if(strlen(pFacePole->sRefPole)>0&&poleSetTbl.GetValueAt(pFacePole->sRefPole,pWingRefPole))
					pPole->des_norm_x.hViceJg=pWingRefPole->handle;
				else if(pPole->pStart->hFatherPart!=pPole->handle)
					pPole->des_norm_x.hViceJg=pPole->pStart->hFatherPart;
				else if(pPole->pEnd->hFatherPart!=pPole->handle)
					pPole->des_norm_x.hViceJg=pPole->pEnd->hFatherPart;
				pPole->cal_x_wing_norm();
				pPole->cal_y_wing_norm();
				if(strlen(pFacePole->sRefPole2)>0&&poleSetTbl.GetValueAt(pFacePole->sRefPole2,pWingRefPole))
				{
					pPole->des_norm_y.hViceJg=pWingRefPole->handle;
					pPole->des_norm_y.bSpecific=false;
					pPole->des_norm_y.bByOtherWing=false;
					pPole->des_norm_y.near_norm=pPole->get_norm_y_wing();
					pPole->cal_y_wing_norm();
				}
			}
			else if(pFacePole->wxLayout.ciLayoutStyle==2)
			{
				if(strlen(pFacePole->sRefPole)>0&&poleSetTbl.GetValueAt(pFacePole->sRefPole,pWingRefPole))
					pPole->des_norm_y.hViceJg=pWingRefPole->handle;
				else if(pPole->pStart->hFatherPart!=pPole->handle)
					pPole->des_norm_y.hViceJg=pPole->pStart->hFatherPart;
				else if(pPole->pEnd->hFatherPart!=pPole->handle)
					pPole->des_norm_y.hViceJg=pPole->pEnd->hFatherPart;
				pPole->cal_y_wing_norm();
				pPole->cal_x_wing_norm();
				if(strlen(pFacePole->sRefPole2)>0&&poleSetTbl.GetValueAt(pFacePole->sRefPole2,pWingRefPole))
				{
					pPole->des_norm_x.hViceJg=pWingRefPole->handle;
					pPole->des_norm_x.bSpecific=false;
					pPole->des_norm_x.bByOtherWing=false;
					pPole->des_norm_x.near_norm=pPole->get_norm_x_wing();
					pPole->cal_x_wing_norm();
				}
			}
			else if(pFacePole->wxLayout.ciLayoutStyle==3)
			{
				if(strlen(pFacePole->sRefPole)>0&&poleSetTbl.GetValueAt(pFacePole->sRefPole,pWingRefPole))
					pPole->des_norm_x.hViceJg=pWingRefPole->handle;
				else if(pPole->pStart->hFatherPart!=pPole->handle)
					pPole->des_norm_x.hViceJg=pPole->pStart->hFatherPart;
				else if(pPole->pEnd->hFatherPart!=pPole->handle)
					pPole->des_norm_x.hViceJg=pPole->pEnd->hFatherPart;
				pPole->cal_x_wing_norm();
				pPole->cal_y_wing_norm();
				if(strlen(pFacePole->sRefPole2)>0&&poleSetTbl.GetValueAt(pFacePole->sRefPole2,pWingRefPole))
				{
					pPole->des_norm_y.hViceJg=pWingRefPole->handle;
					pPole->des_norm_y.bSpecific=false;
					pPole->des_norm_y.bByOtherWing=false;
					pPole->des_norm_y.near_norm=pPole->get_norm_y_wing();
					pPole->cal_y_wing_norm();
				}
			}
			else if(pFacePole->wxLayout.ciLayoutStyle==4)
			{
				if(strlen(pFacePole->sRefPole)>0&&poleSetTbl.GetValueAt(pFacePole->sRefPole,pWingRefPole))
					pPole->des_norm_y.hViceJg=pWingRefPole->handle;
				else if(pPole->pStart->hFatherPart!=pPole->handle)
					pPole->des_norm_y.hViceJg=pPole->pStart->hFatherPart;
				else if(pPole->pEnd->hFatherPart!=pPole->handle)
					pPole->des_norm_y.hViceJg=pPole->pEnd->hFatherPart;
				pPole->cal_y_wing_norm();
				pPole->cal_x_wing_norm();
				if(strlen(pFacePole->sRefPole2)>0&&poleSetTbl.GetValueAt(pFacePole->sRefPole2,pWingRefPole))
				{
					pPole->des_norm_x.hViceJg=pWingRefPole->handle;
					pPole->des_norm_x.bSpecific=false;
					pPole->des_norm_x.bByOtherWing=false;
					pPole->des_norm_x.near_norm=pPole->get_norm_x_wing();
					pPole->cal_x_wing_norm();
				}
			}
			if(!pPole->des_norm_x.bByOtherWing&&pPole->des_norm_x.hViceJg==0)
			{
				pPole->des_norm_x.bSpecific=TRUE;
				pPole->des_norm_x.spec_norm.vector=pPole->des_norm_x.near_norm;
			}
			if(!pPole->des_norm_y.bByOtherWing&&pPole->des_norm_y.hViceJg==0)
			{
				pPole->des_norm_y.bSpecific=TRUE;
				pPole->des_norm_y.spec_norm.vector=pPole->des_norm_y.near_norm;
			}
#else
			CLDSLinePart *pWingRefPole=NULL;
			pPole->des_wing_norm.wLayoutStyle=(WORD)pFacePole->layout_style;
			pPole->des_wing_norm.viewNorm=pFacePole->eyeSideNorm;
			if(strlen(pFacePole->sRefPole)>0&&poleSetTbl.GetValueAt(pFacePole->sRefPole,pWingRefPole))
				pPole->des_wing_norm.hRefPoleArr[0]=pWingRefPole->handle;
			else if(pPole->pStart->hFatherPart!=pPole->handle)
				pPole->des_wing_norm.hRefPoleArr[0]=pPole->pStart->hFatherPart;
			else if(pPole->pEnd->hFatherPart!=pPole->handle)
				pPole->des_wing_norm.hRefPoleArr[0]=pPole->pEnd->hFatherPart;
			if(strlen(pFacePole->sRefPole2)>0&&poleSetTbl.GetValueAt(pFacePole->sRefPole2,pWingRefPole))
				pPole->des_wing_norm.hRefPoleArr[1]=pWingRefPole->handle;
#endif
			pPole->CalPosition();
			NewLinePart(pPole);
			//g_pSolidDraw->SetEntSnapStatus(pPole->handle);
		}
	}
	//2.5������׼���ڸ˼���Ϣ�����㳤�����ͼ��������͵ȣ���д
#ifdef __PART_DESIGN_INC_
	for(pFacePole=pHeadTemplate->pole_list.GetFirst();pFacePole;pFacePole=pHeadTemplate->pole_list.GetNext())
	{
		if(poleSetTbl.GetValueAt(pFacePole->sTitle,pPole))
		{
			pPole->CalLenCoef.iTypeNo=pFacePole->callen_type;
			if(pFacePole->start_force_type==0)
				pPole->start_force_type=CENTRIC_FORCE;
			else
				pPole->start_force_type=ECCENTRIC_FORCE;
			if(pFacePole->end_force_type==0)
				pPole->end_force_type=CENTRIC_FORCE;
			else
				pPole->end_force_type=ECCENTRIC_FORCE;
#ifdef __ANGLE_PART_INC_//!defined(__TSA_)&&!defined(__TSA_FILE_)
			CLDSLineAngle *pTmPole=NULL;
			if(strlen(pFacePole->sLamdaRefPole)>0)
			{
				if(poleSetTbl.GetValueAt(pFacePole->sLamdaRefPole,pTmPole))
					pPole->CalLenCoef.hHorizPole=pTmPole->handle;
			}
			if(strlen(pFacePole->sLamdaRefPole)>0)
			{
				if(poleSetTbl.GetValueAt(pFacePole->sLamdaRefPole,pTmPole))
					pPole->CalLenCoef.hRefPole=pTmPole->handle;
			}
#else
			CLDSLinePart *pTmPole=NULL;
			if(strlen(pFacePole->sLamdaRefPole)>0)
			{
				if(poleSetTbl.GetValueAt(pFacePole->sLamdaRefPole,pTmPole))
					pPole->CalLenCoef.hHorizPole=pTmPole->handle;
			}
			if(strlen(pFacePole->sLamdaRefPole)>0)
			{
				if(poleSetTbl.GetValueAt(pFacePole->sLamdaRefPole,pTmPole))
					pPole->CalLenCoef.hRefPole=pTmPole->handle;
			}
#endif
		}
	}
#endif
	for(i=0;i<m_arrTemplateLdsNode.GetSize();i++)
		m_arrMirSumLdsNode.append(m_arrTemplateLdsNode[i]);
	for(i=0;i<m_arrTemplateLdsLinePart.GetSize();i++)
		m_arrMirSumLdsLinePart.append(m_arrTemplateLdsLinePart[i]);
	FillMirInfo();
	//������׼��ԳƲ���
	MirTemplate();	 		//��׼��Գ�
	return true;
#ifdef alsdf	//��ʱ��֧����ͷģ��ĶԳ�
	MIRMSG zmirmsg,sidemirmsg;
	zmirmsg.axis_flag=4;
	sidemirmsg.axis_flag=8;
	sidemirmsg.rotate_angle=-90;
	NODESET sumnodeset;
#ifdef __ANGLE_PART_INC_//!defined(__TSA_)&&!defined(__TSA_FILE_)
	JGSET sumpoleset;
#else
	LINEPARTSET sumpoleset;
#endif
	for(pNode=m_arrTemplateLdsNode.GetFirst();pNode;pNode=m_arrTemplateLdsNode.GetNext())
		sumnodeset.append(pNode);
	for(pPole=facepole.GetFirst();pPole;pPole=facepole.GetNext())
		sumpoleset.append(pPole);
	if(m_iTemplateMir==3||m_iTemplateMir==5)	//Z��Գ�
	{
		//Z��Գƽڵ�
		for(pNode=m_arrTemplateLdsNode.GetFirst();pNode;pNode=m_arrTemplateLdsNode.GetNext())
		{
			CLDSNode *pMirNode=pNode->GetMirNode(zmirmsg);
			if(pMirNode==NULL)	//δ�ҵ���ӦZ��ԳƵĽڵ�
			{
				pMirNode=m_pTower->Node.append();
				pMirNode->CopyProperty(pNode);
				pMirNode->pos=GetMirPos(pNode->pos,zmirmsg);
				CalMirAtomLayer(pNode->layer(),pMirNode->layer(),zmirmsg);
				//��ӶԳ����ɵĶ���Դ����Ĺ��������б�
				AddMirObjToSrcObjRelaObjList(pNode,pMirNode,zmirmsg);
				pMirNode->feature=1;	//�����ɽڵ�
			}
			else
				pMirNode->feature=0;	//ԭ�нڵ�
			sumnodeset.append(pMirNode);
		}
		//Z��ԳƸ˼�
		for(pPole=facepole.GetFirst();pPole;pPole=facepole.GetNext())
		{
			CLDSLinePart *pMirPole=(CLDSLinePart*)pPole->GetMirPart(zmirmsg);
			if(pMirPole==NULL)
			{
				MirTaAtom(pPole,zmirmsg);
				pMirPole=(CLDSLinePart*)pPole->GetMirPart(zmirmsg);
			}
			if(pMirPole)
#ifdef __ANGLE_PART_INC_//__LDS_
				sumpoleset.append((CLDSLineAngle*)pMirPole);
#else
				sumpoleset.append(pMirPole);
#endif
		}
		for(pNode=m_arrTemplateLdsNode.GetFirst();pNode;pNode=m_arrTemplateLdsNode.GetNext())
		{
			CLDSNode *pMirNode=pNode->GetMirNode(zmirmsg);
			if(pMirNode&&pMirNode->feature==1)
				ConvertAttachHandle(pNode,pMirNode,zmirmsg);
		}
	}
	//������׼�澵�����
	if(m_iTemplateMir==4||m_iTemplateMir==5)//����
	{	
		//����ڵ�
		int nNode=sumnodeset.GetNodeNum();
		int nPole=sumpoleset.GetNodeNum();
		for(pNode=sumnodeset.GetFirst();pNode;pNode=sumnodeset.GetNext())
		{
			CLDSNode *pSideMirNode=pNode->GetMirNode(sidemirmsg);
			if(pSideMirNode==NULL)
			{	//δ�ҵ���Ӧ����ԳƵĽڵ�
				pSideMirNode=m_pTower->Node.append();
				pSideMirNode->CopyProperty(pNode);
				pSideMirNode->pos=GetMirPos(pNode->pos,sidemirmsg);
				CalMirAtomLayer(pNode->layer(),pSideMirNode->layer(),sidemirmsg);
				//��ӶԳ����ɵĶ���Դ����Ĺ��������б�
				AddMirObjToSrcObjRelaObjList(pNode,pSideMirNode,sidemirmsg);
				pSideMirNode->feature=1;	//�����ɽڵ�
			}
			else
				pSideMirNode->feature=0;	//ԭ�нڵ�
		}
		//����˼�
		for(pPole=sumpoleset.GetFirst();pPole;pPole=sumpoleset.GetNext())
		{
			CLDSLinePart *pSideMirPole=(CLDSLinePart*)pPole->GetMirPart(sidemirmsg);
			if(pSideMirPole==NULL)
				MirTaAtom(pPole,sidemirmsg);
		}
		for(pNode=sumnodeset.GetFirst();pNode;pNode=sumnodeset.GetNext())
		{
			CLDSNode *pSideMirNode=pNode->GetMirNode(sidemirmsg);
			if(pSideMirNode&&pSideMirNode->feature==1)
				ConvertAttachHandle(pNode,pSideMirNode,sidemirmsg);
		}
	}
	CObjNoGroup *pWindSegNoGroup=m_pTower->NoManager.FromGroupID(WINDSEG_GROUP);
	for(CFaceWindSegment* pFaceWindSeg=pHeadTemplate->windseg_list.GetFirst();pFaceWindSeg;
	pFaceWindSeg=pHeadTemplate->windseg_list.GetNext())
	{
		CWindSegment *pSeg=m_pTower->WindLoadSeg.append();
		pSeg->iNo=pWindSegNoGroup->EnumIdleNo();
		pWindSegNoGroup->SetNoState(pSeg->iNo);
		pSeg->cType=pFaceWindSeg->cType;					//��׼�κ�
		pSeg->cMirType=pFaceWindSeg->cMirType;				//�Գ���Ϣ0:�޶Գ�;1:Y��Գ�;2:X��Գ�
		pSeg->cCalStyle=pFaceWindSeg->cCalStyle;				//���㷽ʽ0:������ʽ�����ѹ;1:��X����ˮƽ�ᵣ��ʽ�����ѹ;2:��Y����ˮƽ�ᵣ��ʽ�����ѹ
		pSeg->CqiFront=pFaceWindSeg->CqiFront;
		pSeg->CqiSide=pFaceWindSeg->CqiSide;	//��/��������ϵ��
		pSeg->BetaZ=pFaceWindSeg->BetaZ;				//����ϵ����Ʒ���ص���ϵ������z��
		for(i=0;i<pFaceWindSeg->GetMaxNodeCount();i++)
		{
			if(pFaceWindSeg->nodeIndexArr[i]<0)
				break;
			pSeg->nodeArr[i]=m_arrTemplateLdsNode[pFaceWindSeg->nodeIndexArr[i]]->handle;		//�˸������ڵ������
		}
	}
#endif
}
bool CInsertStdTemplateOperation::CreateStdTemplate(CFacePanel* pFacePanel,CLDSNode* m_arrTemplateLdsNodeArr[4],
													int m_arrTemplateLdsNodeMir,int templateMir,STDPANEL_INFO* pPanelInfo/*=NULL*/,
													CFGWORD* pBodyWord/*=NULL*/,CFGWORD* pLegWord/*=NULL*/)
{
	int i,j,keyNodeNum,repeatNum=1;
	CLDSNode *pLdsNode;
	long hInitHandle=m_pTower->handle;
#ifdef __ANGLE_PART_INC_//!defined(__TSA_)&&!defined(__TSA_FILE_)
	CLDSLineAngle *pAngle;
#endif
	CLDSLinePart *pLinePart;
	ARRAY_LIST<CFacePanelNode>arrFaceNodeTemp;
	ARRAY_LIST<CFacePanelPole>arrFacePoleTemp;
	char cFirstLayer='?',cThirdLayer='?';
	if(pPanelInfo!=NULL&&pPanelInfo->szLayer[0]!=0)
		cFirstLayer=pPanelInfo->szLayer[0];
	if(pPanelInfo!=NULL&&pPanelInfo->szLayer[0]!=NULL&&pPanelInfo->szLayer[1]!=NULL&&pPanelInfo->szLayer[2]!=NULL)
		cThirdLayer=pPanelInfo->szLayer[2];
	//���ԭ������
	memset(m_arrShankKeyNode,0,sizeof(CLDSNode*)*4);	//ԭʼ¼��������׼����ĸ�����ڵ�
	m_arrTemplateLdsNode.Empty();	//ԭʼ���ɵı�׼�λ��׼���ڵĽڵ㼯��
	m_arrMirSumLdsNode.Empty();	//��ԭʼ���ɼ��Գ���������(������������)�ı�׼�λ��׼���ڵĽڵ㼯��
	m_arrTemplateLdsLinePart.Empty();//ԭʼ���ɵı�׼�λ��׼���ڵĸ˼�����
	m_arrMirSumLdsLinePart.Empty();	//��ԭʼ���ɼ��Գ���������(������������)�ı�׼�λ��׼���ڵĸ˼�����

	m_pFacePanel=pFacePanel;
	m_iKeyNodeMir=m_arrTemplateLdsNodeMir;
	m_iTemplateMir=templateMir;
	for(CFacePanelNode *pFaceNode=m_pFacePanel->node_list.GetFirst();pFaceNode;pFaceNode=m_pFacePanel->node_list.GetNext())
		arrFaceNodeTemp.append(*pFaceNode);
	for(CFacePanelPole *pFacePole=m_pFacePanel->pole_list.GetFirst();pFacePole;pFacePole=m_pFacePanel->pole_list.GetNext())
		arrFacePoleTemp.append(*pFacePole);
	int cMirTransX1Y2=0;
	if(pFacePanel->m_iFaceType==3&&m_arrTemplateLdsNodeArr[0]&&m_arrTemplateLdsNodeArr[1])
	{
		GEPOINT templ_datum_vec=arrFaceNodeTemp[1].pos-arrFaceNodeTemp[0].pos;
		GEPOINT pos0=m_arrTemplateLdsNodeArr[0]->Position();
		GEPOINT pos1=m_arrTemplateLdsNodeArr[1]->Position();
		if(fabs(templ_datum_vec.x)>EPS)
		{
			GEPOINT hvec=pos1-pos0;
			if(hvec.x*templ_datum_vec.x<0)
				cMirTransX1Y2=2;
		}
		else if(pos0.x<0)
			cMirTransX1Y2=2;
	}
	else if(pFacePanel->m_iFaceType==6&&m_arrTemplateLdsNodeArr[0]&&m_arrTemplateLdsNodeArr[1]&&m_arrTemplateLdsNodeArr[2])
	{
		GEPOINT pos0=m_arrTemplateLdsNodeArr[0]->Position();
		GEPOINT pos1=m_arrTemplateLdsNodeArr[1]->Position();
		GEPOINT pos2=m_arrTemplateLdsNodeArr[2]->Position();
		GEPOINT hvec=pos0-pos2;
		if(fabs(pos0.z-pos2.z)<EPS)
		{
			if( (fabs(hvec.y)<EPS&&pos0.y>0&&hvec.x<0)||//ǰ����������
				(fabs(hvec.y)<EPS&&pos0.y<0&&hvec.x>0))	//�����Ҳ������
				cMirTransX1Y2=2;
			else if((fabs(hvec.x)<EPS&&pos0.x>0&&hvec.y>0)||//����ǰ�������
					(fabs(hvec.x)<EPS&&pos0.x<0&&hvec.y<0))	//�����������
				cMirTransX1Y2=1;
		}
	}
	SEGI iSeg;//��ǰ��׼��Ķκ�
	for(i=0;i<4;i++)
	{
		if(m_arrTemplateLdsNodeArr[i]!=NULL)
		{
			if(i==0)	//ȡ��һ���ؼ���Ķκ�Ϊ��ǰ������Ķκ� wht 10-11-20
				iSeg=m_arrTemplateLdsNodeArr[i]->iSeg;
			if(pFacePanel->m_iFaceType==1)	//����ģʽ
				m_arrShankKeyNode[i]=m_arrTemplateLdsNodeArr[i];
			else
				m_arrTemplateLdsNode.append(m_arrTemplateLdsNodeArr[i]);
		}
	}
	if(m_pFacePanel->m_iFaceType==1)
	{	//�����׼������K�ͱ�׼��
		repeatNum=m_pFacePanel->m_nInternodeNum;
		if(m_arrShankKeyNode[0]==NULL||m_arrShankKeyNode[1]==NULL)
			return false;	//ȱ�������׼��ؼ���
		for(i=0;i<4;i++)
		{
			if(m_arrShankKeyNode[i]!=NULL)
				continue;	//���ֹ�����˹ؼ��ڵ�
			if(arrFaceNodeTemp[i].attach_type!=0)
#ifdef AFX_TARG_ENU_ENGLISH
				throw "The coordinates type of standard plane's key point isn't reasonable��standard plane inserts failure��";
#else 
				throw "�����׼��ؼ����������Ͳ����������׼��ʧ�ܣ�";
#endif
			for(j=0;j<i;j++)
			{
				if((m_iKeyNodeMir==1||m_iKeyNodeMir==2) &&
					fabs(arrFaceNodeTemp[j].pos.x+arrFaceNodeTemp[i].pos.x)<EPS &&
					fabs(arrFaceNodeTemp[j].pos.y-arrFaceNodeTemp[i].pos.y)<EPS)
				{
					m_arrShankKeyNode[i]=m_arrShankKeyNode[j]->GetMirNode(MIRMSG(m_iKeyNodeMir));
					if(m_arrShankKeyNode[i])
						break;
					else
#ifdef AFX_TARG_ENU_ENGLISH
						throw "Can't find the corresponding symmetric key nodes of standard plane��standard plane inserts failure��";
#else 
						throw "�Ҳ������׼�����Ӧ�ĶԳƹؼ��ڵ㣬�����׼��ʧ�ܣ�";
#endif
				}
			}
		}
		if(m_arrShankKeyNode[0]==NULL||m_arrShankKeyNode[1]==NULL||m_arrShankKeyNode[2]==NULL||m_arrShankKeyNode[3]==NULL)
#ifdef AFX_TARG_ENU_ENGLISH
			throw "Can't find the corresponding symmetric key nodes of standard plane��standard plane inserts failure��";
#else 
			throw "�Ҳ������׼�����Ӧ�ĶԳƹؼ��ڵ㣬�����׼��ʧ�ܣ�";
#endif
		keyNodeNum=4;
	}
	else if(m_pFacePanel->m_iFaceType==2||m_pFacePanel->m_iFaceType==4||
		m_pFacePanel->m_iFaceType==5||m_pFacePanel->m_iFaceType==6)
	{	//�����׼�桢K�ͱ�׼���V�ͱ�׼��
		for(i=0;i<arrFaceNodeTemp.GetSize();i++)
		{
			if(m_arrTemplateLdsNode.GetSize()>i)
				continue;	//���ֹ�����˹ؼ��ڵ�
			if(arrFaceNodeTemp[i].attach_type!=0)
				break;
			for(j=0;j<i;j++)
			{
				if((m_iKeyNodeMir==1||m_iKeyNodeMir==3) &&	//X��Գ�
					fabs(arrFaceNodeTemp[j].pos.x-arrFaceNodeTemp[i].pos.x)<EPS &&
					fabs(arrFaceNodeTemp[j].pos.y+arrFaceNodeTemp[i].pos.y)<EPS)
				{
					pLdsNode=m_arrTemplateLdsNode[j]->GetMirNode(MIRMSG(1));
					if(pLdsNode)
					{
						m_arrTemplateLdsNode.append(pLdsNode);
						break;
					}
					else
#ifdef AFX_TARG_ENU_ENGLISH
						throw "Can't find the corresponding symmetric key nodes of standard plane��standard plane inserts failure��";
#else 
						throw "�Ҳ������׼�����Ӧ�ĶԳƹؼ��ڵ㣬�����׼��ʧ�ܣ�";
#endif
				}
				if((m_iKeyNodeMir==2||m_iKeyNodeMir==3) &&	//Y��Գ�
					fabs(arrFaceNodeTemp[j].pos.x+arrFaceNodeTemp[i].pos.x)<EPS &&
					fabs(arrFaceNodeTemp[j].pos.y-arrFaceNodeTemp[i].pos.y)<EPS)
				{
					pLdsNode=m_arrTemplateLdsNode[j]->GetMirNode(MIRMSG(2));
					if(pLdsNode)
					{
						m_arrTemplateLdsNode.append(pLdsNode);
						break;
					}
					else
#ifdef AFX_TARG_ENU_ENGLISH
						throw "Can't find the corresponding symmetric key nodes of standard plane��standard plane inserts failure��";
#else 
						throw "�Ҳ������׼�����Ӧ�ĶԳƹؼ��ڵ㣬�����׼��ʧ�ܣ�";
#endif
				}
				if((m_iKeyNodeMir==3) &&	//Z��Գ�
					fabs(arrFaceNodeTemp[j].pos.x+arrFaceNodeTemp[i].pos.x)<EPS &&
					fabs(arrFaceNodeTemp[j].pos.y+arrFaceNodeTemp[i].pos.y)<EPS)
				{
					pLdsNode=m_arrTemplateLdsNode[j]->GetMirNode(MIRMSG(3));
					if(pLdsNode)
					{
						m_arrTemplateLdsNode.append(pLdsNode);
						break;
					}
					else
#ifdef AFX_TARG_ENU_ENGLISH
						throw "Can't find the corresponding symmetric key nodes of standard plane��standard plane inserts failure��";
#else 
						throw "�Ҳ������׼�����Ӧ�ĶԳƹؼ��ڵ㣬�����׼��ʧ�ܣ�";
#endif
				}
			}
		}
		if(m_pFacePanel->IsRepeateType())
			repeatNum=m_pFacePanel->m_nInternodeNum-m_pFacePanel->m_nRepeatLessNum;
		keyNodeNum=m_arrTemplateLdsNode.GetSize();
		if(keyNodeNum<3)
#ifdef AFX_TARG_ENU_ENGLISH
			throw "Can't find enough corresponding symmetric key nodes of standard plane��standard plane inserts failure��";
#else 
			throw "�Ҳ����㹻�ı�׼���Ӧ�ؼ��ڵ㣬�����׼��ʧ�ܣ�";
#endif
	}
	else if(m_pFacePanel->m_iFaceType==3)
	{	//�����ڼ��׼��
		repeatNum=m_pFacePanel->m_nInternodeNum-m_pFacePanel->m_nRepeatLessNum;
		keyNodeNum=m_pFacePanel->m_nKeyNode;
		if(m_arrTemplateLdsNode.GetSize()<m_pFacePanel->m_nKeyNode)
#ifdef AFX_TARG_ENU_ENGLISH
			throw "Can't find enough corresponding symmetric key nodes of standard plane��standard plane inserts failure��";
#else 
			throw "�Ҳ����㹻�ı�׼���Ӧ�ؼ��ڵ㣬�����׼��ʧ�ܣ�";
#endif
	}
	CExpression expression;
	EXPRESSION_VAR *pVar=expression.varList.Append();
	strcpy(pVar->variableStr,"N");
	if(m_pFacePanel->m_iFaceType!=2)
		pVar->fValue=m_pFacePanel->m_nInternodeNum;
	else
		pVar->fValue=1;
	pVar=expression.varList.Append();
	strcpy(pVar->variableStr,"i");
	//CDisplayView* pActiveView=console.GetActiveView();
	CLDSModule* pActiveModule=m_pTower->GetActiveModule();

	//ԭʼ��׼��������
	int iRepeatIndex=0;	//����iRepeatIndex����������Ϊ�����Ӵ�ѭ���ж�ѭ�������Ŀɶ��� wjh-2018.8.28
	for(i=0;i<repeatNum;i++)
	{
		iRepeatIndex=0;
		CStringKeyHashTable<CLDSNode*>hashNodeTbl;
		hashNodeTbl.CreateHashTable(arrFaceNodeTemp.GetSize());
		//1.0���ݵ�ǰ�ڼ��������õ�ǰ�ڼ������׼����ĸ��ؼ��ڵ�
		if(pFacePanel->m_iFaceType==1)	//����ģʽ
		{
			f3dPoint key_point_arr[4];
			f3dPoint line_vec1=m_arrShankKeyNode[1]->Position(false)-m_arrShankKeyNode[0]->Position(false);
			f3dPoint line_vec2=m_arrShankKeyNode[3]->Position(false)-m_arrShankKeyNode[2]->Position(false);
			key_point_arr[0]=m_arrShankKeyNode[0]->Position(false)+line_vec1*i/(double)pFacePanel->m_nInternodeNum;
			key_point_arr[1]=m_arrShankKeyNode[0]->Position(false)+line_vec1*(1.0+i)/pFacePanel->m_nInternodeNum;
			key_point_arr[2]=m_arrShankKeyNode[2]->Position(false)+line_vec2*i/(double)pFacePanel->m_nInternodeNum;
			key_point_arr[3]=m_arrShankKeyNode[2]->Position(false)+line_vec2*(1.0+i)/pFacePanel->m_nInternodeNum;
			m_arrTemplateLdsNode.Empty();
			for(j=0;j<4;j++)
			{
				pLdsNode=FindLDSNodeByPos(key_point_arr[j],pBodyWord,pLegWord);
				if(pLdsNode!=NULL)
					m_arrTemplateLdsNode.append(pLdsNode);
				else
				{
					pLdsNode=m_pTower->Node.append();
					pLdsNode->iSeg=iSeg;	//�κ� wht 10-11-20
					pLdsNode->SetPosition(key_point_arr[j]);
					NewNode(pLdsNode);
					pLdsNode->m_cPosCalType=5;	//�����ȷֵ�
					pLdsNode->attach_scale=(j%2+(double)i)/pFacePanel->m_nInternodeNum;
					if(j<2)
					{
						pLdsNode->arrRelationNode[0]=m_arrShankKeyNode[0]->handle;
						pLdsNode->arrRelationNode[1]=m_arrShankKeyNode[1]->handle;
						pLdsNode->SetLayer(m_arrShankKeyNode[0]->layer());
					}
					else
					{
						pLdsNode->arrRelationNode[0]=m_arrShankKeyNode[2]->handle;
						pLdsNode->arrRelationNode[1]=m_arrShankKeyNode[3]->handle;
						pLdsNode->SetLayer(m_arrShankKeyNode[2]->layer());
					}
					m_arrTemplateLdsNode.append(pLdsNode);
				}
				hashNodeTbl.SetValueAt(arrFaceNodeTemp[j].sTitle,pLdsNode);
			}
		}
		else
		{	//��շǹؼ���
			for(j=0;j<arrFaceNodeTemp.GetSize();j++)
			{
				if(j<keyNodeNum)
					hashNodeTbl.SetValueAt(arrFaceNodeTemp[j].sTitle,m_arrTemplateLdsNode[j]);
				else
				{
					if(!m_arrTemplateLdsNode.RemoveAt(j))
						break;
					else
						j--;
				}
			}
		}
		m_pActiveModule=m_pTower->GetActiveModule();
		//1.1ԭʼ��׼��ڵ�������
		for(j=keyNodeNum;j<arrFaceNodeTemp.GetSize();j++)
		{
			if(arrFaceNodeTemp[j].m_bInternodeRepeat)
			{
				expression.VarAt(1)->fValue=i;//expression.varList[1].fValue=i;
				double scale=expression.SolveExpression(arrFaceNodeTemp[j].scale_expression);
				CLDSNode *pScaleStart=NULL,*pScaleEnd=NULL;
				hashNodeTbl.GetValueAt(arrFaceNodeTemp[arrFaceNodeTemp[j].attach_node_index[0]].sTitle,pScaleStart);
				hashNodeTbl.GetValueAt(arrFaceNodeTemp[arrFaceNodeTemp[j].attach_node_index[1]].sTitle,pScaleEnd);
				f3dPoint pos=pScaleStart->Position(false)+scale*(pScaleEnd->Position(false)-pScaleStart->Position(false));
				pLdsNode=FindLDSNodeByPos(pos,pBodyWord,pLegWord);
				if(pLdsNode==NULL)
				{
					pLdsNode=m_pTower->Node.append();
					pLdsNode->iSeg=iSeg;	//�κ� wht 10-11-20
					pLdsNode->SetPosition(pos);
					NewNode(pLdsNode);
					pLdsNode->attach_scale=scale;
					pLdsNode->m_cPosCalType=5;
					pLdsNode->arrRelationNode[0]=pScaleStart->handle;
					pLdsNode->arrRelationNode[1]=pScaleEnd->handle;
					pLdsNode->cfgword=pScaleStart->UnifiedCfgword()&pScaleEnd->UnifiedCfgword();
					if(pScaleEnd->Layer(0)=='L')
						pLdsNode->SetLayer(pScaleEnd->layer());
					else
						pLdsNode->SetLayer(pScaleStart->layer());
					if(toupper(pScaleStart->layer(0))!=toupper(pScaleEnd->layer(0))&&cFirstLayer!='?')
						pLdsNode->layer(0)=cFirstLayer;
					if(toupper(pScaleStart->layer(2))!=toupper(pScaleEnd->layer(2))&&cThirdLayer!='?')
						pLdsNode->layer(2)=cThirdLayer;
				}
				m_arrTemplateLdsNode.append(pLdsNode);
			}
			else if(arrFaceNodeTemp[j].attach_type==1)
			{	//�˼���X����ֵ�����
				f3dPoint start,end;
				if(arrFaceNodeTemp[j].attach_node_index[0]>=0&&arrFaceNodeTemp[j].attach_node_index[0]<m_arrTemplateLdsNode.GetSize())
					start=m_arrTemplateLdsNode[arrFaceNodeTemp[j].attach_node_index[0]]->Position(false);	//��Ч�����ڵ�
				else
#ifdef AFX_TARG_ENU_ENGLISH
					throw "Standard plane Lacks of reasonable attached node ,standard plane inserts failure��";
#else 
					throw "��׼���еĽڵ�ȱ�ٺ��������ڵ㣬�����׼��ʧ�ܣ�";
#endif
				if(arrFaceNodeTemp[j].attach_node_index[1]>=0&&arrFaceNodeTemp[j].attach_node_index[1]<m_arrTemplateLdsNode.GetSize())
					end=m_arrTemplateLdsNode[arrFaceNodeTemp[j].attach_node_index[1]]->Position(false);	//��Ч�����ڵ�
				else
#ifdef AFX_TARG_ENU_ENGLISH
					throw "Standard plane Lacks of reasonable attached node ,standard plane inserts failure��";
#else 
					throw "��׼���еĽڵ�ȱ�ٺ��������ڵ㣬�����׼��ʧ�ܣ�";
#endif
				f3dPoint pos;
				if(arrFaceNodeTemp[j].attach_node_index[2]>=0&&arrFaceNodeTemp[j].attach_node_index[2]<m_arrTemplateLdsNode.GetSize())
					pos=m_arrTemplateLdsNode[arrFaceNodeTemp[j].attach_node_index[2]]->Position(false);//��X���������ڵ㰴�˽ڵ��ʵ��X����Ϊ��׼
				else	//��X���������ڵ㣬���˽ڵ��ԭʼ����λ��X�����趨X����Ϊ��׼
				{
					f3dPoint vec_sum,vec;
					vec_sum.x=arrFaceNodeTemp[arrFaceNodeTemp[j].attach_node_index[1]].pos.x-
						arrFaceNodeTemp[arrFaceNodeTemp[j].attach_node_index[0]].pos.x;
					vec_sum.y=arrFaceNodeTemp[arrFaceNodeTemp[j].attach_node_index[1]].pos.y-
						arrFaceNodeTemp[arrFaceNodeTemp[j].attach_node_index[0]].pos.y;
					vec.x=arrFaceNodeTemp[j].pos.x-arrFaceNodeTemp[arrFaceNodeTemp[j].attach_node_index[0]].pos.x;
					vec.y=arrFaceNodeTemp[j].pos.y-arrFaceNodeTemp[arrFaceNodeTemp[j].attach_node_index[0]].pos.y;
					double scale=vec.mod()/vec_sum.mod();
					pos=start+(end-start)*scale;
				}
				pos.y=start.y+(pos.x-start.x)*(end.y-start.y)/(end.x-start.x);
				pos.z=start.z+(pos.x-start.x)*(end.z-start.z)/(end.x-start.x);
				pLdsNode=FindLDSNodeByPos(pos,pBodyWord,pLegWord);
				if(pLdsNode!=NULL)
					m_arrTemplateLdsNode.append(pLdsNode);
				else //if(pLdsNode==NULL)
				{	//�Ҳ����˽ڵ㣬������Ҫ�����½ڵ�
					pLdsNode=m_pTower->Node.append();
					pLdsNode->iSeg=iSeg;	//�κ� wht 10-11-20
					CLDSNode *pStartNode=m_arrTemplateLdsNode[arrFaceNodeTemp[j].attach_node_index[0]];
					CLDSNode *pEndNode=m_arrTemplateLdsNode[arrFaceNodeTemp[j].attach_node_index[1]];
					if(pEndNode->Layer(0)=='L')
						pLdsNode->SetLayer(pEndNode->layer());
					else
						pLdsNode->SetLayer(pStartNode->layer());
					if(toupper(pStartNode->layer(0))!=toupper(pEndNode->layer(0))&&cFirstLayer!='?')
						pLdsNode->layer(0)=cFirstLayer;
					if(toupper(pStartNode->layer(2))!=toupper(pEndNode->layer(2))&&cThirdLayer!='?')
						pLdsNode->layer(2)=cThirdLayer;
					pLdsNode->m_cPosCalType=1;	//X���겻���
					pLdsNode->SetPosition(pos);
					pLdsNode->cfgword=pStartNode->UnifiedCfgword()&pEndNode->UnifiedCfgword();
					NewNode(pLdsNode);
					pLdsNode->arrRelationNode[0]=pStartNode->handle;
					pLdsNode->arrRelationNode[1]=pEndNode->handle;
					pLdsNode->SetLayer(pStartNode->layer());
					m_arrTemplateLdsNode.append(pLdsNode);
				}
			}
			else if(arrFaceNodeTemp[j].attach_type==2)
			{	//�˼���Y����ֵ�����
				f3dPoint start,end;
				if(arrFaceNodeTemp[j].attach_node_index[0]>=0&&arrFaceNodeTemp[j].attach_node_index[0]<m_arrTemplateLdsNode.GetSize())
				{	//��Ч�����ڵ�
					start=m_arrTemplateLdsNode[arrFaceNodeTemp[j].attach_node_index[0]]->Position(false);
				}
				else
#ifdef AFX_TARG_ENU_ENGLISH
					throw "Standard plane Lacks of reasonable attached node ,standard plane inserts failure��";
#else 
					throw "��׼���еĽڵ�ȱ�ٺ��������ڵ㣬�����׼��ʧ�ܣ�";
#endif
				if(arrFaceNodeTemp[j].attach_node_index[1]>=0&&arrFaceNodeTemp[j].attach_node_index[1]<m_arrTemplateLdsNode.GetSize())
				{	//��Ч�����ڵ�
					end=m_arrTemplateLdsNode[arrFaceNodeTemp[j].attach_node_index[1]]->Position(false);
				}
				else
#ifdef AFX_TARG_ENU_ENGLISH
					throw "Standard plane Lacks of reasonable attached node ,standard plane inserts failure��";
#else 
					throw "��׼���еĽڵ�ȱ�ٺ��������ڵ㣬�����׼��ʧ�ܣ�";
#endif
				f3dPoint pos;
				if(arrFaceNodeTemp[j].attach_node_index[2]>=0&&arrFaceNodeTemp[j].attach_node_index[2]<m_arrTemplateLdsNode.GetSize())
				{		//��Y���������ڵ㰴�˽ڵ��ʵ��X����Ϊ��׼
					pos=m_arrTemplateLdsNode[arrFaceNodeTemp[j].attach_node_index[2]]->Position(false);
				}
				else	//��Y���������ڵ㣬���˽ڵ��ԭʼ����λ��Y�����趨X����Ϊ��׼
				{
					f3dPoint vec_sum,vec;
					vec_sum.x=arrFaceNodeTemp[arrFaceNodeTemp[j].attach_node_index[1]].pos.x-
						arrFaceNodeTemp[arrFaceNodeTemp[j].attach_node_index[0]].pos.x;
					vec_sum.y=arrFaceNodeTemp[arrFaceNodeTemp[j].attach_node_index[1]].pos.y-
						arrFaceNodeTemp[arrFaceNodeTemp[j].attach_node_index[0]].pos.y;
					vec.x=arrFaceNodeTemp[j].pos.x-arrFaceNodeTemp[arrFaceNodeTemp[j].attach_node_index[0]].pos.x;
					vec.y=arrFaceNodeTemp[j].pos.y-arrFaceNodeTemp[arrFaceNodeTemp[j].attach_node_index[0]].pos.y;
					double scale=vec.mod()/vec_sum.mod();
					pos=start+(end-start)*scale;
				}
				pos.x=start.z+(pos.y-start.y)*(end.x-start.x)/(end.y-start.y);
				pos.z=start.z+(pos.y-start.y)*(end.z-start.z)/(end.y-start.y);
				pLdsNode=FindLDSNodeByPos(pos,pBodyWord,pLegWord);
				if(pLdsNode!=NULL)
					m_arrTemplateLdsNode.append(pLdsNode);
				else //if(pLdsNode==NULL)
				{	//�Ҳ����˽ڵ㣬������Ҫ�����½ڵ�
					pLdsNode=m_pTower->Node.append();
					pLdsNode->iSeg=iSeg;	//�κ� wht 10-11-20
					CLDSNode *pStartNode=m_arrTemplateLdsNode[arrFaceNodeTemp[j].attach_node_index[0]];
					CLDSNode *pEndNode=m_arrTemplateLdsNode[arrFaceNodeTemp[j].attach_node_index[1]];
					if(pEndNode->Layer(0)=='L')
						pLdsNode->SetLayer(pEndNode->layer());
					else
						pLdsNode->SetLayer(pStartNode->layer());
					if(toupper(pStartNode->layer(0))!=toupper(pEndNode->layer(0))&&cFirstLayer!='?')
						pLdsNode->layer(0)=cFirstLayer;
					if(toupper(pStartNode->layer(2))!=toupper(pEndNode->layer(2))&&cThirdLayer!='?')
						pLdsNode->layer(2)=cThirdLayer;
					pLdsNode->m_cPosCalType=2;	//X���겻���
					pLdsNode->SetPosition(pos);
					pLdsNode->cfgword=pStartNode->UnifiedCfgword()&pEndNode->UnifiedCfgword();
					NewNode(pLdsNode);
					pLdsNode->arrRelationNode[0]=pStartNode->handle;
					pLdsNode->arrRelationNode[1]=pEndNode->handle;
					pLdsNode->SetLayer(pStartNode->layer());
					m_arrTemplateLdsNode.append(pLdsNode);
				}
			}
			else if(arrFaceNodeTemp[j].attach_type==3)
			{	//�˼���Z����ֵ�����
				f3dPoint start,end;
				if(arrFaceNodeTemp[j].attach_node_index[0]>=0&&arrFaceNodeTemp[j].attach_node_index[0]<m_arrTemplateLdsNode.GetSize())
				{	//��Ч�����ڵ�
					start=m_arrTemplateLdsNode[arrFaceNodeTemp[j].attach_node_index[0]]->Position(false);
				}
				else
#ifdef AFX_TARG_ENU_ENGLISH
					throw "Standard plane Lacks of reasonable attached node ,standard plane inserts failure��";
#else 
					throw "��׼���еĽڵ�ȱ�ٺ��������ڵ㣬�����׼��ʧ�ܣ�";
#endif
				if(arrFaceNodeTemp[j].attach_node_index[1]>=0&&arrFaceNodeTemp[j].attach_node_index[1]<m_arrTemplateLdsNode.GetSize())
				{	//��Ч�����ڵ�
					end=m_arrTemplateLdsNode[arrFaceNodeTemp[j].attach_node_index[1]]->Position(false);
				}
				else
#ifdef AFX_TARG_ENU_ENGLISH
					throw "Standard plane Lacks of reasonable attached node ,standard plane inserts failure��";
#else 
					throw "��׼���еĽڵ�ȱ�ٺ��������ڵ㣬�����׼��ʧ�ܣ�";
#endif
				f3dPoint pos;
				if(arrFaceNodeTemp[j].attach_node_index[2]>=0&&arrFaceNodeTemp[j].attach_node_index[2]<m_arrTemplateLdsNode.GetSize())
				{		//��X���������ڵ㰴�˽ڵ��ʵ��X����Ϊ��׼
					pos=m_arrTemplateLdsNode[arrFaceNodeTemp[j].attach_node_index[2]]->Position(false);
				}
				else	//��Z���������ڵ㣬���˽ڵ��ԭʼ����λ��Z�����趨X����Ϊ��׼
				{
					f3dPoint vec_sum,vec;
					vec_sum.x=arrFaceNodeTemp[arrFaceNodeTemp[j].attach_node_index[1]].pos.x-
						arrFaceNodeTemp[arrFaceNodeTemp[j].attach_node_index[0]].pos.x;
					vec_sum.y=arrFaceNodeTemp[arrFaceNodeTemp[j].attach_node_index[1]].pos.y-
						arrFaceNodeTemp[arrFaceNodeTemp[j].attach_node_index[0]].pos.y;
					vec.x=arrFaceNodeTemp[j].pos.x-arrFaceNodeTemp[arrFaceNodeTemp[j].attach_node_index[0]].pos.x;
					vec.y=arrFaceNodeTemp[j].pos.y-arrFaceNodeTemp[arrFaceNodeTemp[j].attach_node_index[0]].pos.y;
					double scale=vec.mod()/vec_sum.mod();
					pos=start+(end-start)*scale;
				}
				pos.x=start.x+(pos.z-start.z)*(end.x-start.x)/(end.z-start.z);
				pos.y=start.y+(pos.z-start.z)*(end.y-start.y)/(end.z-start.z);
				pLdsNode=FindLDSNodeByPos(pos,pBodyWord,pLegWord);
				if(pLdsNode!=NULL)
					m_arrTemplateLdsNode.append(pLdsNode);
				else //if(pLdsNode==NULL)
				{	//�Ҳ����˽ڵ㣬������Ҫ�����½ڵ�
					pLdsNode=m_pTower->Node.append();
					pLdsNode->iSeg=iSeg;	//�κ� wht 10-11-20
					CLDSNode *pStartNode=m_arrTemplateLdsNode[arrFaceNodeTemp[j].attach_node_index[0]];
					CLDSNode *pEndNode=m_arrTemplateLdsNode[arrFaceNodeTemp[j].attach_node_index[1]];
					if(pEndNode->Layer(0)=='L')
						pLdsNode->SetLayer(pEndNode->layer());
					else
						pLdsNode->SetLayer(pStartNode->layer());
					if(toupper(pStartNode->layer(0))!=toupper(pEndNode->layer(0))&&cFirstLayer!='?')
						pLdsNode->layer(0)=cFirstLayer;
					if(toupper(pStartNode->layer(2))!=toupper(pEndNode->layer(2))&&cThirdLayer!='?')
						pLdsNode->layer(2)=cThirdLayer;
					pLdsNode->m_cPosCalType=3;	//Z���겻���
					pLdsNode->SetPosition(pos);
					pLdsNode->cfgword=pStartNode->UnifiedCfgword()&pEndNode->UnifiedCfgword();
					NewNode(pLdsNode);
					pLdsNode->arrRelationNode[0]=pStartNode->handle;
					pLdsNode->arrRelationNode[1]=pEndNode->handle;
					pLdsNode->SetLayer(pStartNode->layer());
					m_arrTemplateLdsNode.append(pLdsNode);
				}
			}
			else if(arrFaceNodeTemp[j].attach_type==4)
			{	//���˼����潻��
				CLDSNode *pStartNode1,*pStartNode2,*pEndNode1,*pEndNode2;
				if(arrFaceNodeTemp[j].attach_node_index[0]>=0&&arrFaceNodeTemp[j].attach_node_index[0]<m_arrTemplateLdsNode.GetSize())
				{	//��Ч�����ڵ�
					pStartNode1=m_arrTemplateLdsNode[arrFaceNodeTemp[j].attach_node_index[0]];
				}
				else
#ifdef AFX_TARG_ENU_ENGLISH
					throw "Standard plane Lacks of reasonable attached node ,standard plane inserts failure��";
#else 
					throw "��׼���еĽڵ�ȱ�ٺ��������ڵ㣬�����׼��ʧ�ܣ�";
#endif
				if(arrFaceNodeTemp[j].attach_node_index[1]>=0&&arrFaceNodeTemp[j].attach_node_index[1]<m_arrTemplateLdsNode.GetSize())
				{	//��Ч�����ڵ�
					pEndNode1=m_arrTemplateLdsNode[arrFaceNodeTemp[j].attach_node_index[1]];
				}
				else
#ifdef AFX_TARG_ENU_ENGLISH
					throw "Standard plane Lacks of reasonable attached node ,standard plane inserts failure��";
#else 
					throw "��׼���еĽڵ�ȱ�ٺ��������ڵ㣬�����׼��ʧ�ܣ�";
#endif
				if(arrFaceNodeTemp[j].attach_node_index[2]>=0&&arrFaceNodeTemp[j].attach_node_index[2]<m_arrTemplateLdsNode.GetSize())
				{	//��Ч�����ڵ�
					pStartNode2=m_arrTemplateLdsNode[arrFaceNodeTemp[j].attach_node_index[2]];
				}
				else
#ifdef AFX_TARG_ENU_ENGLISH
					throw "Standard plane Lacks of reasonable attached node ,standard plane inserts failure��";
#else 
					throw "��׼���еĽڵ�ȱ�ٺ��������ڵ㣬�����׼��ʧ�ܣ�";
#endif
				if(arrFaceNodeTemp[j].attach_node_index[3]>=0&&arrFaceNodeTemp[j].attach_node_index[3]<m_arrTemplateLdsNode.GetSize())
				{	//��Ч�����ڵ�
					pEndNode2=m_arrTemplateLdsNode[arrFaceNodeTemp[j].attach_node_index[3]];
				}
				else
#ifdef AFX_TARG_ENU_ENGLISH
					throw "Standard plane Lacks of reasonable attached node ,standard plane inserts failure��";
#else 
					throw "��׼���еĽڵ�ȱ�ٺ��������ڵ㣬�����׼��ʧ�ܣ�";
#endif
				f3dPoint pos;
				int iRet=Int3dll(pStartNode1->Position(false),pEndNode1->Position(false),pStartNode2->Position(false),pEndNode2->Position(false),pos);
				pLdsNode=FindLDSNodeByPos(pos,pBodyWord,pLegWord);
				if(pLdsNode!=NULL)
					m_arrTemplateLdsNode.append(pLdsNode);
				else //if(pLdsNode==NULL)
				{	//�Ҳ����˽ڵ㣬������Ҫ�����½ڵ�
					pLdsNode=m_pTower->Node.append();
					pLdsNode->iSeg=iSeg;	//�κ� wht 10-11-20
					CLDSNode *pStartNode1=m_arrTemplateLdsNode[arrFaceNodeTemp[j].attach_node_index[0]];
					CLDSNode *pEndNode1=m_arrTemplateLdsNode[arrFaceNodeTemp[j].attach_node_index[1]];
					CLDSNode *pStartNode2=m_arrTemplateLdsNode[arrFaceNodeTemp[j].attach_node_index[2]];
					CLDSNode *pEndNode2=m_arrTemplateLdsNode[arrFaceNodeTemp[j].attach_node_index[3]];
					pLdsNode->SetLayer(pStartNode1->layer());
					if(cFirstLayer!='?')
						pLdsNode->layer(0)=cFirstLayer;
					if(cThirdLayer!='?')
						pLdsNode->layer(2)=cThirdLayer;
					pLdsNode->m_cPosCalType=4;	//���潻��
					pLdsNode->SetPosition(pos);
					pLdsNode->cfgword=pStartNode1->UnifiedCfgword();
					pLdsNode->cfgword&=pEndNode1->UnifiedCfgword();
					pLdsNode->cfgword&=pStartNode2->UnifiedCfgword();
					pLdsNode->cfgword&=pEndNode2->UnifiedCfgword();
					pLdsNode->arrRelationNode[0]=pStartNode1->handle;
					pLdsNode->arrRelationNode[1]=pEndNode1->handle;
					pLdsNode->arrRelationNode[2]=pStartNode2->handle;
					pLdsNode->arrRelationNode[3]=pEndNode2->handle;
					NewNode(pLdsNode);
					m_arrTemplateLdsNode.append(pLdsNode);
				}
			}
			else if(arrFaceNodeTemp[j].attach_type==5)
			{	//�˼��ϱ����ȷֵ�
				f3dPoint start,end;
				if(arrFaceNodeTemp[j].attach_node_index[0]>=0&&arrFaceNodeTemp[j].attach_node_index[0]<m_arrTemplateLdsNode.GetSize())
				{	//��Ч�����ڵ�
					start=m_arrTemplateLdsNode[arrFaceNodeTemp[j].attach_node_index[0]]->Position(false);
				}
				else
#ifdef AFX_TARG_ENU_ENGLISH
					throw "Standard plane Lacks of reasonable attached node ,standard plane inserts failure��";
#else 
					throw "��׼���еĽڵ�ȱ�ٺ��������ڵ㣬�����׼��ʧ�ܣ�";
#endif
				if(arrFaceNodeTemp[j].attach_node_index[1]>=0&&arrFaceNodeTemp[j].attach_node_index[1]<m_arrTemplateLdsNode.GetSize())
				{	//��Ч�����ڵ�
					end=m_arrTemplateLdsNode[arrFaceNodeTemp[j].attach_node_index[1]]->Position(false);
				}
				else
#ifdef AFX_TARG_ENU_ENGLISH
					throw "Standard plane Lacks of reasonable attached node ,standard plane inserts failure��";
#else 
					throw "��׼���еĽڵ�ȱ�ٺ��������ڵ㣬�����׼��ʧ�ܣ�";
#endif
				f3dPoint pos;
				expression.VarAt(1)->fValue=0;//expression.varList[1].fValue=0;
				if(strlen(arrFaceNodeTemp[j].scale_expression)>0)
					arrFaceNodeTemp[j].attach_scale=expression.SolveExpression(arrFaceNodeTemp[j].scale_expression);

				double real_scale=arrFaceNodeTemp[j].attach_scale;
				if(arrFaceNodeTemp[j].attach_scale<0)
				{
					real_scale=-arrFaceNodeTemp[j].attach_scale;
					real_scale=0.1*((int)real_scale)/(real_scale-(int)real_scale);
				}
				pos.x=start.x+real_scale*(end.x-start.x);
				pos.y=start.y+real_scale*(end.y-start.y);
				pos.z=start.z+real_scale*(end.z-start.z);
				pLdsNode=FindLDSNodeByPos(pos,pBodyWord,pLegWord);
				if(pLdsNode!=NULL)
					m_arrTemplateLdsNode.append(pLdsNode);
				else //if(pLdsNode==NULL)
				{	//�Ҳ����˽ڵ㣬������Ҫ�����½ڵ�
					pLdsNode=m_pTower->Node.append();
					pLdsNode->iSeg=iSeg;	//�κ� wht 10-11-20
					CLDSNode *pStartNode=m_arrTemplateLdsNode[arrFaceNodeTemp[j].attach_node_index[0]];
					CLDSNode *pEndNode=m_arrTemplateLdsNode[arrFaceNodeTemp[j].attach_node_index[1]];
					if(pEndNode->Layer(0)=='L')
						pLdsNode->SetLayer(pEndNode->layer());
					else
						pLdsNode->SetLayer(pStartNode->layer());
					if(toupper(pStartNode->layer(0))!=toupper(pEndNode->layer(0))&&cFirstLayer!='?')
						pLdsNode->layer(0)=cFirstLayer;
					if(toupper(pStartNode->layer(2))!=toupper(pEndNode->layer(2))&&cThirdLayer!='?')
						pLdsNode->layer(2)=cThirdLayer;
					pLdsNode->m_cPosCalType=5;	//�����ȷֵ�
					pLdsNode->attach_scale=real_scale;
					pLdsNode->SetPosition(pos);
					pLdsNode->cfgword=pStartNode->UnifiedCfgword()&pEndNode->UnifiedCfgword();
					NewNode(pLdsNode);
					pLdsNode->arrRelationNode[0]=pStartNode->handle;
					pLdsNode->arrRelationNode[1]=pEndNode->handle;
					m_arrTemplateLdsNode.append(pLdsNode);
				}
			}
			if(pLdsNode)
				hashNodeTbl.SetValueAt(arrFaceNodeTemp[j].sTitle,pLdsNode);
		}
		//2.2ԭʼ��׼��˼��������
		f3dPoint line_vec1=m_arrTemplateLdsNode[1]->Position(false)-m_arrTemplateLdsNode[0]->Position(false);
		f3dPoint line_vec2=m_arrTemplateLdsNode[2]->Position(false)-m_arrTemplateLdsNode[1]->Position(false);
		f3dPoint work_norm=line_vec1^line_vec2;
		if(work_norm.IsZero())
			work_norm.Set(0,1,0);
		else
			normalize(work_norm);
		//������������
		f3dPoint near_norm;
		if(pPanelInfo)
		{	
			if(pPanelInfo->ciPanelNormalStyle==2)
				near_norm=pPanelInfo->vPanelNormal;
			else if(pPanelInfo->ciPanelNormalStyle==1)
			{
				if(toupper(pPanelInfo->szLayer[2])=='Q')
					near_norm.Set(0,1,0);
				else if(toupper(pPanelInfo->szLayer[2])=='Y')
					near_norm.Set(1,0,0);
				else if(toupper(pPanelInfo->szLayer[2])=='H')
					near_norm.Set(0,-1,0);
				else if(toupper(pPanelInfo->szLayer[2])=='Z')
					near_norm.Set(-1,0,0);
			}
		}
		if(!near_norm.IsZero()&&(near_norm*work_norm)<0)
			work_norm*=-1;
	
		bool bFirstLineAngleIsNew=false;
		m_arrTemplateLdsLinePart.Empty();
		CLDSModule *pModule=m_pTower->GetActiveModule();
		for(j=0;j<arrFacePoleTemp.GetSize();j++)
		{
			if(!arrFacePoleTemp[j].m_bInternodeRepeat&&iRepeatIndex>0)
				continue;	//���ظ��˼����ڵ�һ�����
			CLDSNode *pStart=m_arrTemplateLdsNode[arrFacePoleTemp[j].start_i];
			CLDSNode *pEnd  =m_arrTemplateLdsNode[arrFacePoleTemp[j].end_i];
			pLinePart=m_pTower->CheckOverLappedLinePart(pStart->Position(false),pEnd->Position(false),pModule,pBodyWord,pLegWord);
			if(pLinePart)
			{
				m_arrTemplateLdsLinePart.append(pLinePart);
#ifdef __ANGLE_PART_INC_//!defined(__TSA_)&&!defined(__TSA_FILE_)
				if(pLinePart->GetClassTypeId()==CLS_LINEANGLE)
					pAngle=(CLDSLineAngle*)pLinePart;
#endif
			}
			else
			{
#if !defined(__TSA_)&&!defined(__TSA_FILE_)
				pLinePart=pAngle=(CLDSLineAngle*)m_pTower->Parts.append(CLS_LINEANGLE);
#else
				pLinePart=(CLDSLinePart*)m_pTower->Parts.append(CLS_LINEPART);
#endif
				pLinePart->iSeg=iSeg;	//�κ� wht 10-11-20
				pLinePart->pStart=pStart;
				pLinePart->pEnd=pEnd;
				if(pAngle)
					pAngle->MirLayStyle=arrFacePoleTemp[j].wxLayout.ciMirLayStyle;
				if(cFirstLayer!='?')
					pLinePart->layer(0)=cFirstLayer;
				else
					pLinePart->layer(0)=pStart->layer(0);
				if(cFirstLayer=='?'&&pEnd->IsLegObj())
					pLinePart->layer(0)='L';
				pLinePart->cfgword=pStart->UnifiedCfgword()&pEnd->UnifiedCfgword();
				if(arrFacePoleTemp[j].sLayer[1]!=0)
					pLinePart->layer(1)=arrFacePoleTemp[j].sLayer[1];
				if(pLinePart->layer(1)=='B')
					pLinePart->layer(1)='F';
				if(pLinePart->IsAuxPole())
					pLinePart->connectStart.N=pLinePart->connectEnd.N=1;	//����Ĭ��һ����˨���Ա����Զ���ֽڵ����������˨��� wjh-2017.12.17
				if(m_pFacePanel->m_iFaceType==2&&j==0)	//��������ǰ���
					pLinePart->layer(2)='Q';
				else if(m_pFacePanel->m_iFaceType==2&&j==1)	//�������������
					pLinePart->layer(2)='Y';
				else if(m_pFacePanel->m_iFaceType==2&&j==2)	//�������������
					pLinePart->layer(2)='H';
				else if(m_pFacePanel->m_iFaceType==2&&j==3)	//�������������
					pLinePart->layer(2)='Z';
				else
					pLinePart->layer(2)=cThirdLayer;
				ANGLE_LAYOUT wxAngleLayStyle=pLinePart->des_wing_norm.wLayoutStyle=arrFacePoleTemp[j].wxLayout.ciLayoutStyle;
				if(cMirTransX1Y2==1||cMirTransX1Y2==2)
				{
					if((pLinePart->des_wing_norm.wLayoutStyle&0x0f00)==0x0200)
					{	//X/Y���������Գ�
						if(wxAngleLayStyle.ciLayoutStyle==1)
							wxAngleLayStyle.ciLayoutStyle=2;
						else if(wxAngleLayStyle.ciLayoutStyle==2)
							wxAngleLayStyle.ciLayoutStyle=1;
						else if(wxAngleLayStyle.ciLayoutStyle==3)
							wxAngleLayStyle.ciLayoutStyle=4;
						else if(wxAngleLayStyle.ciLayoutStyle==4)
							wxAngleLayStyle.ciLayoutStyle=3;
						pLinePart->des_wing_norm.wLayoutStyle=wxAngleLayStyle;
					}
					else if((pLinePart->des_wing_norm.wLayoutStyle&0x0f00)==0x0100)
					{	//X/Y������ͬ��Գ�
						wxAngleLayStyle.ciLayoutStyle=4-wxAngleLayStyle.ciLayoutStyle;//(2+arrFacePoleTemp[j].GetAngleLayStyle())%4;
						pLinePart->des_wing_norm.wLayoutStyle=wxAngleLayStyle;
					}
				}
				if((pLinePart->des_wing_norm.wLayoutStyle&&0x0f00)==0)
					pLinePart->des_wing_norm.wLayoutStyle|=0x0100;//CLDSLinePart::MIRXorY_SAMEDIRECT;
#ifdef __ANGLE_PART_INC_//!defined(__TSA_)&&!defined(__TSA_FILE_)
				if(pAngle)
				{
					if(wxAngleLayStyle.ciLayoutStyle==1)
					{
						pAngle->des_norm_x.near_norm=-work_norm;
						if(j>0)
							pAngle->des_norm_x.hViceJg=m_arrTemplateLdsLinePart[0]->handle;
						pAngle->des_norm_y.bByOtherWing=TRUE;
						pAngle->desStartPos.wing_x_offset.gStyle=pAngle->desEndPos.wing_x_offset.gStyle=4;
						pAngle->desStartPos.wing_x_offset.offsetDist=pAngle->desEndPos.wing_x_offset.offsetDist=0;
						pAngle->desStartPos.wing_y_offset.gStyle=pAngle->desEndPos.wing_y_offset.gStyle=0;
					}
					else if(wxAngleLayStyle.ciLayoutStyle==2)
					{
						pAngle->des_norm_y.near_norm=work_norm;
						if(j>0)
							pAngle->des_norm_y.hViceJg=m_arrTemplateLdsLinePart[0]->handle;
						pAngle->des_norm_x.bByOtherWing=TRUE;
						pAngle->desStartPos.wing_y_offset.gStyle=pAngle->desEndPos.wing_y_offset.gStyle=4;
						pAngle->desStartPos.wing_y_offset.offsetDist=pAngle->desEndPos.wing_y_offset.offsetDist=0;
						pAngle->desStartPos.wing_x_offset.gStyle=pAngle->desEndPos.wing_x_offset.gStyle=0;
					}
					else if(wxAngleLayStyle.ciLayoutStyle==3)
					{
						pAngle->des_norm_x.near_norm=work_norm;
						if(j>0)
							pAngle->des_norm_x.hViceJg=m_arrTemplateLdsLinePart[0]->handle;
						pAngle->des_norm_y.bByOtherWing=TRUE;
						pAngle->desStartPos.wing_x_offset.gStyle=pAngle->desEndPos.wing_x_offset.gStyle=4;
						pAngle->desStartPos.wing_x_offset.offsetDist=pAngle->desEndPos.wing_x_offset.offsetDist=0;
						pAngle->desStartPos.wing_y_offset.gStyle=pAngle->desEndPos.wing_y_offset.gStyle=0;
					}
					else //if(wxAngleLayStyle.ciLayoutStyle==4)
					{
						pAngle->des_norm_y.near_norm=-work_norm;
						if(j>0)
							pAngle->des_norm_y.hViceJg=m_arrTemplateLdsLinePart[0]->handle;
						pAngle->des_norm_x.bByOtherWing=TRUE;
						pAngle->desStartPos.wing_y_offset.gStyle=pAngle->desEndPos.wing_y_offset.gStyle=4;
						pAngle->desStartPos.wing_y_offset.offsetDist=pAngle->desEndPos.wing_y_offset.offsetDist=0;
						pAngle->desStartPos.wing_x_offset.gStyle=pAngle->desEndPos.wing_x_offset.gStyle=0;
					}
				}
#else
				pLinePart->des_wing_norm.wLayoutStyle=(WORD)arrFacePoleTemp[j].layout_style;
				if(j>0)
					pLinePart->des_wing_norm.hRefPoleArr[0]=m_arrTemplateLdsLinePart[0]->handle;
				if(arrFacePoleTemp[j].wxLayout.ciLayoutStyle==1)
					pLinePart->des_wing_norm.viewNorm=-work_norm;
				else if(arrFacePoleTemp[j].wxLayout.ciLayoutStyle==2)
					pLinePart->des_wing_norm.viewNorm=work_norm;
				else if(arrFacePoleTemp[j].wxLayout.ciLayoutStyle==3)
					pLinePart->des_wing_norm.viewNorm=work_norm;
				else
					pLinePart->des_wing_norm.viewNorm=-work_norm;
#endif
#ifdef __PART_DESIGN_INC_
				if(arrFacePoleTemp[j].start_force_type==0)
					pLinePart->start_force_type=CENTRIC_FORCE;
				else
					pLinePart->start_force_type=ECCENTRIC_FORCE;
				if(arrFacePoleTemp[j].end_force_type==0)
					pLinePart->end_force_type=CENTRIC_FORCE;
				else
					pLinePart->end_force_type=ECCENTRIC_FORCE;;
#endif
				pLinePart->SetStart(pStart->Position(false));
				pLinePart->SetEnd(pEnd->Position(false));
				if(j>0)
				{
#ifdef __ANGLE_PART_INC_//!defined(__TSA_)&&!defined(__TSA_FILE_)
					if(pAngle)
					{
						pAngle->cal_x_wing_norm();
						pAngle->cal_y_wing_norm();
						pAngle->CalPosition();
					}
#endif
					NewLinePart(pLinePart);
					//g_pSolidDraw->SetEntSnapStatus(pLinePart->handle);
				}
				else
					bFirstLineAngleIsNew=true;
				m_arrTemplateLdsLinePart.append(pLinePart);
			}
		}
		if(bFirstLineAngleIsNew)
		{
#ifdef __ANGLE_PART_INC_//!defined(__TSA_)&&!defined(__TSA_FILE_)
			if(m_arrTemplateLdsLinePart[0]->GetClassTypeId()==CLS_LINEANGLE)
			{
				pAngle=(CLDSLineAngle*)m_arrTemplateLdsLinePart[0];
				if(!pAngle->des_norm_x.bByOtherWing)
					pAngle->des_norm_x.hViceJg=m_arrTemplateLdsLinePart[1]->handle;
				else
					pAngle->des_norm_y.hViceJg=m_arrTemplateLdsLinePart[1]->handle;
				pAngle->cal_x_wing_norm();
				pAngle->cal_y_wing_norm();
			}
#else
			m_arrTemplateLdsLinePart[0]->des_wing_norm.hRefPoleArr[0]=m_arrTemplateLdsLinePart[1]->handle;
#endif
			NewLinePart(m_arrTemplateLdsLinePart[0]);
			//g_pSolidDraw->SetEntSnapStatus(m_arrTemplateLdsLinePart[0]->handle);
		}
		//2.3�²���ԭʼ��׼���ڵĽڵ�˼���ϵ��Ϣ��д
		//�ڵ㸸�˼���Ϣ�������˼���Ϣ
		for(j=0;j<m_arrTemplateLdsNode.GetSize();j++)
		{
			if(m_arrTemplateLdsNode[j]->m_cPosCalType==4)	//�����
			{
				m_arrTemplateLdsNode[j]->arrRelationPole[0]=pLdsNode->arrRelationPole[1]=0;
				for(int k=0;k<m_arrTemplateLdsLinePart.GetSize();k++)
				{
					f3dLine line(m_arrTemplateLdsLinePart[k]->pStart->Position(false),m_arrTemplateLdsLinePart[k]->pEnd->Position(false));
					if(line.PtInLine(m_arrTemplateLdsNode[j]->Position(false))<=0)
						continue;
					else if(line.PtInLine(m_arrTemplateLdsNode[arrFaceNodeTemp[j].attach_node_index[0]]->Position(false))>0
						&& line.PtInLine(m_arrTemplateLdsNode[arrFaceNodeTemp[j].attach_node_index[1]]->Position(false))>0)
						m_arrTemplateLdsNode[j]->arrRelationPole[0]=m_arrTemplateLdsLinePart[k]->handle;
					else if(line.PtInLine(m_arrTemplateLdsNode[arrFaceNodeTemp[j].attach_node_index[2]]->Position(false))>0
						&& line.PtInLine(m_arrTemplateLdsNode[arrFaceNodeTemp[j].attach_node_index[3]]->Position(false))>0)
						m_arrTemplateLdsNode[j]->arrRelationPole[1]=m_arrTemplateLdsLinePart[k]->handle;
				}
			}
			else if(m_arrTemplateLdsNode[j]->m_cPosCalType!=0)	//�����κ������ڵ�
			{
				for(int k=0;k<m_arrTemplateLdsLinePart.GetSize();k++)
				{
					if((m_arrTemplateLdsLinePart[k]->pStart->handle==m_arrTemplateLdsNode[j]->arrRelationNode[0]&&m_arrTemplateLdsLinePart[k]->pEnd->handle==m_arrTemplateLdsNode[j]->arrRelationNode[1])||
						(m_arrTemplateLdsLinePart[k]->pStart->handle==m_arrTemplateLdsNode[j]->arrRelationNode[1]&&m_arrTemplateLdsLinePart[k]->pEnd->handle==m_arrTemplateLdsNode[j]->arrRelationNode[0]))
					{
						m_arrTemplateLdsNode[j]->arrRelationPole[0]=m_arrTemplateLdsLinePart[k]->handle;
						break;
					}
				}
			}
			if(m_arrTemplateLdsNode[j]->hFatherPart<0x20)
			{	//�޸��˼���Ϣ
				CLDSLinePart *pViceFatherPole=NULL;
				for(int k=0;k<m_arrTemplateLdsLinePart.GetSize();k++)
				{
					f3dLine line(m_arrTemplateLdsLinePart[k]->pStart->Position(false),m_arrTemplateLdsLinePart[k]->pEnd->Position(false));
					int ret=line.PtInLine(m_arrTemplateLdsNode[j]->Position(false));
					if(ret==2)		//���ڸ˼�ֱ����
					{
						m_arrTemplateLdsNode[j]->hFatherPart=m_arrTemplateLdsLinePart[k]->handle;
						m_arrTemplateLdsNode[j]->layer(0)=m_arrTemplateLdsLinePart[k]->layer(0);
						m_arrTemplateLdsNode[j]->layer(2)=m_arrTemplateLdsLinePart[k]->layer(2);
						break;
					}
					else if(ret==1&&pViceFatherPole==NULL)	//׼���˼�(�ڵ�Ϊ�˼��˽ڵ�)
						pViceFatherPole=m_arrTemplateLdsLinePart[k];
				}
				if(m_arrTemplateLdsNode[j]->hFatherPart<0x20)	//δ�ҵ����˼������Ե�һ��׼���˼�Ϊ���˼�
				{
					m_arrTemplateLdsNode[j]->hFatherPart=pViceFatherPole->handle;
					m_arrTemplateLdsNode[j]->layer(2)=pViceFatherPole->layer(2);
				}
			}
		}
		//2.4������׼���ڸ˼���Ϣ�����㳤�����ͼ��������͵ȣ���д
#ifdef __PART_DESIGN_INC_
		for(j=0;j<arrFacePoleTemp.GetSize();j++)
		{
			m_arrTemplateLdsLinePart[j]->CalLenCoef.iTypeNo=arrFacePoleTemp[j].callen_type;
			if(arrFacePoleTemp[j].start_force_type==0)
				m_arrTemplateLdsLinePart[j]->start_force_type=CENTRIC_FORCE;
			else
				m_arrTemplateLdsLinePart[j]->start_force_type=ECCENTRIC_FORCE;
			if(arrFacePoleTemp[j].end_force_type==0)
				m_arrTemplateLdsLinePart[j]->end_force_type=CENTRIC_FORCE;
			else
				m_arrTemplateLdsLinePart[j]->end_force_type=ECCENTRIC_FORCE;
			
			if(strlen(arrFacePoleTemp[j].sLamdaRefPole)>0)
			{
				for(int index=0;index<arrFacePoleTemp.GetSize();index++)
				{
					if(stricmp(arrFacePoleTemp[index].sTitle,arrFacePoleTemp[j].sLamdaRefPole)==0)
					{
						if(m_arrTemplateLdsLinePart[j]->CalLenCoef.iTypeNo==0)
							m_arrTemplateLdsLinePart[j]->CalLenCoef.hRefPole=m_arrTemplateLdsLinePart[index]->handle;
						else if(m_arrTemplateLdsLinePart[j]->CalLenCoef.iTypeNo==3||m_arrTemplateLdsLinePart[j]->CalLenCoef.iTypeNo==4)
							m_arrTemplateLdsLinePart[j]->CalLenCoef.hHorizPole=m_arrTemplateLdsLinePart[index]->handle;
					}
				}
			}
		}
#endif
		for(j=0;j<m_arrTemplateLdsNode.GetSize();j++)
		{
			if(m_arrTemplateLdsNode[j]->handle>=hInitHandle&&m_arrTemplateLdsNode[j]->IsLegObj()&&pActiveModule)
			{
				int iDefaultLegQuad=1;
				if(m_arrTemplateLdsNode[j]->Layer(2)>'1'&&m_arrTemplateLdsNode[j]->Layer(2)<='4')
				{	
					iDefaultLegQuad=m_arrTemplateLdsNode[j]->Layer(2)-'0';
					m_arrTemplateLdsNode[j]->cfgword=CFGWORD(pActiveModule->m_arrActiveQuadLegNo[iDefaultLegQuad-1]);
				}
			}
			for(int k=0;k<m_arrMirSumLdsNode.GetSize();k++)
			{
				if(m_arrMirSumLdsNode[k]==m_arrTemplateLdsNode[j])
					break;
			}
			if(k==m_arrMirSumLdsNode.GetSize())	//�½ڵ�
				m_arrMirSumLdsNode.append(m_arrTemplateLdsNode[j]);
		}
		for(j=0;j<m_arrTemplateLdsLinePart.GetSize();j++)
		{
			if(m_arrTemplateLdsLinePart[j]->handle>=hInitHandle)
			{
				int iDefaultLegQuad=1;
				char cStartLayer0=m_arrTemplateLdsLinePart[j]->pStart->Layer(0);
				char cEndLayer0=m_arrTemplateLdsLinePart[j]->pEnd->Layer(0);
				if(cStartLayer0!='S')
					m_arrTemplateLdsLinePart[j]->layer(0)=cStartLayer0;
				else if(cEndLayer0!='S')
					m_arrTemplateLdsLinePart[j]->layer(0)=cEndLayer0;
				if(m_arrTemplateLdsLinePart[j]->IsLegObj()&&pActiveModule)
				{
					if(m_arrTemplateLdsLinePart[j]->Layer(2)>'1'&&m_arrTemplateLdsLinePart[j]->Layer(2)<='4')
					{
						iDefaultLegQuad=m_arrTemplateLdsLinePart[j]->Layer(2)-'0';
						m_arrTemplateLdsLinePart[j]->cfgword=CFGWORD(pActiveModule->m_arrActiveQuadLegNo[iDefaultLegQuad-1]);
					}
				}
			}
#ifdef __ANGLE_PART_INC_//!defined(__TSA_)&&!defined(__TSA_FILE_)
			if(m_pFacePanel->m_iFaceType==1&&m_arrTemplateLdsLinePart[j]->layer(1)!='Z'&&
				m_arrTemplateLdsLinePart[j]->GetClassTypeId()==CLS_LINEANGLE)
			{	//�����׼���ڵ��Ǹ� wjh-2011.6.26
				pAngle=(CLDSLineAngle*)m_arrTemplateLdsLinePart[j];
				CLDSLineAngle *pStartFatherAngle=(CLDSLineAngle*)m_pTower->Parts.FromHandle(pAngle->pStart->hFatherPart,CLS_LINEANGLE);
				CLDSLineAngle *pEndFatherAngle=(CLDSLineAngle*)m_pTower->Parts.FromHandle(pAngle->pEnd->hFatherPart,CLS_LINEANGLE);
				/**Ĭ��ƽ�ƻᵼ��С��б���Ĳ�ֽڵ�ʱ��ƫ�Ƶ�ƽ�ƣ��������¹���λ�ô��� wjh-2018.1.12
				if(pStartFatherAngle&&pStartFatherAngle->layer(0)=='S'&&pStartFatherAngle->layer(1)=='Z'&&
					pStartFatherAngle->desStartPos.IsFaceOffset())
					pAngle->desStartPos.spatialOperationStyle=4;	//Ĭ��Ϊƽ��
				if(pEndFatherAngle&&pEndFatherAngle->layer(0)=='S'&&pEndFatherAngle->layer(1)=='Z'&&
					pEndFatherAngle->desStartPos.IsFaceOffset())
					pAngle->desEndPos.spatialOperationStyle=4;	//Ĭ��Ϊƽ��
				*/
			}
#endif
			for(int k=0;k<m_arrMirSumLdsLinePart.GetSize();k++)
			{
				if(m_arrMirSumLdsLinePart[k]==m_arrTemplateLdsLinePart[j])
					break;
			}
			if(k==m_arrMirSumLdsLinePart.GetSize())	//�¸˼�
				m_arrMirSumLdsLinePart.append(m_arrTemplateLdsLinePart[j]);
		}
		FillMirInfo();
	}
	MirTemplate();	 		//��׼��Գ�
	return true;
}
//����ƥ��һ����׼���ڵĸ˼����ڵ�Գ���Ϣ
void CInsertStdTemplateOperation::FillMirInfo()
{
	int i,j;
	RELATIVE_OBJECT relaObj;
	//ƥ��ڵ�Գ���Ϣ
	for(i=0;i<m_arrTemplateLdsNode.GetSize();i++)
	{
		CLDSNode *pNode=m_arrTemplateLdsNode[i];
		for(j=i+1;j<m_arrTemplateLdsNode.GetSize();j++)
		{
			CLDSNode *pCheckNode=m_arrTemplateLdsNode[j];
			if(fabs(pNode->Position('Z')-pCheckNode->Position('Z'))>EPS)
				continue;	//����ͬһˮƽ����
			if(fabs(pNode->Position(false).x-pCheckNode->Position(false).x)<EPS&&fabs(pNode->Position(false).y+pCheckNode->Position(false).y)<EPS)
				relaObj.mirInfo.axis_flag=1;	//Y���귴���ǣ���Գ�
			else if(fabs(pNode->Position(false).x+pCheckNode->Position(false).x)<EPS&&fabs(pNode->Position(false).y-pCheckNode->Position(false).y)<EPS)
				relaObj.mirInfo.axis_flag=2;	//X���귴���ǣ���Գ�
			else if(fabs(pNode->Position(false).x+pCheckNode->Position(false).x)<EPS&&fabs(pNode->Position(false).y+pCheckNode->Position(false).y)<EPS)
				relaObj.mirInfo.axis_flag=4;	//X&Y���귴���ǣ���Գ�
			else
				continue;	//�޶Գ�
			relaObj.hObj=pCheckNode->handle;
			pNode->AppendRelativeObj(relaObj);
			relaObj.hObj=pNode->handle;
			pCheckNode->AppendRelativeObj(relaObj);
		}
	}
	//ƥ��˼��Գ���Ϣ
	for(i=0;i<m_arrTemplateLdsLinePart.GetSize();i++)
	{
		CLDSLinePart *pLinePart=m_arrTemplateLdsLinePart[i];
		for(j=i+1;j<m_arrTemplateLdsLinePart.GetSize();j++)
		{
			CLDSLinePart *pCheckLinePart=m_arrTemplateLdsLinePart[j];
			double highZ=min(pLinePart->pStart->Position('Z'),pLinePart->pEnd->Position('Z'));
			double lowZ =max(pLinePart->pStart->Position('Z'),pLinePart->pEnd->Position('Z'));
			double mirHighZ=min(pCheckLinePart->pStart->Position('Z'),pCheckLinePart->pEnd->Position('Z'));
			double mirLowZ =max(pCheckLinePart->pStart->Position('Z'),pCheckLinePart->pEnd->Position('Z'));
			if(fabs(highZ-mirHighZ)+fabs(lowZ-mirLowZ)>EPS)
				continue;	//����ͬһˮƽ����
			if( fabs(pLinePart->pStart->Position(false).x-pCheckLinePart->pStart->Position(false).x)<EPS&&
				fabs(pLinePart->pStart->Position(false).y+pCheckLinePart->pStart->Position(false).y)<EPS&&
				fabs(pLinePart->pEnd->Position(false).x-pCheckLinePart->pEnd->Position(false).x)<EPS&&
				fabs(pLinePart->pEnd->Position(false).y+pCheckLinePart->pEnd->Position(false).y)<EPS)
				relaObj.mirInfo.axis_flag=1;	//Y���귴���ǣ���Գ�
			else if( fabs(pLinePart->pStart->Position(false).x+pCheckLinePart->pStart->Position(false).x)<EPS&&
				fabs(pLinePart->pStart->Position(false).y-pCheckLinePart->pStart->Position(false).y)<EPS&&
				fabs(pLinePart->pEnd->Position(false).x+pCheckLinePart->pEnd->Position(false).x)<EPS&&
				fabs(pLinePart->pEnd->Position(false).y-pCheckLinePart->pEnd->Position(false).y)<EPS)
				relaObj.mirInfo.axis_flag=2;	//X���귴���ǣ���Գ�
			else if( fabs(pLinePart->pStart->Position(false).x+pCheckLinePart->pStart->Position(false).x)<EPS&&
				fabs(pLinePart->pStart->Position(false).y+pCheckLinePart->pStart->Position(false).y)<EPS&&
				fabs(pLinePart->pEnd->Position(false).x+pCheckLinePart->pEnd->Position(false).x)<EPS&&
				fabs(pLinePart->pEnd->Position(false).y+pCheckLinePart->pEnd->Position(false).y)<EPS)
				relaObj.mirInfo.axis_flag=4;	//X&Y���귴���ǣ���Գ�
			else
				continue;	//�޶Գ�
			relaObj.hObj=pCheckLinePart->handle;
			pLinePart->AppendRelativeObj(relaObj);
			relaObj.hObj=pLinePart->handle;
			pCheckLinePart->AppendRelativeObj(relaObj);
		}
	}
}
//��׼��ԳƲ���
typedef CLDSDbObject* DBOBJPTR;
static BOOL IsIncludeHandle(CLDSPoint *pDatumPoint,long hMainObj)
{
	if(pDatumPoint&&hMainObj>0x20)
	{
		int datum_pos_style=pDatumPoint->datum_pos_style;
		CLDSPoint::DES_PARA des_para=pDatumPoint->des_para;
		if(des_para.hNode==hMainObj
			||(datum_pos_style==1&&des_para.RODEND.hRod==hMainObj)
			||(datum_pos_style==2&&(des_para.RODNODE.hRod ==hMainObj||des_para.RODNODE.hNode==hMainObj))
			||(datum_pos_style==3&&(des_para.AXIS_INTERS.hDatum1==hMainObj||des_para.AXIS_INTERS.hDatum2==hMainObj))	
			||(datum_pos_style==4&&(des_para.AXIS_INTERS.hDatum1==hMainObj||des_para.AXIS_INTERS.hDatum2==hMainObj))
			||(datum_pos_style==5&&des_para.DATUMLINE.hDatumLine==hMainObj)
			||(datum_pos_style==6&&des_para.DATUMPLANE.hDatumPlane==hMainObj)
			||(datum_pos_style==7&&des_para.RODEND.hRod==hMainObj)
			||(datum_pos_style==8&&des_para.hNode==hMainObj)
			||(datum_pos_style==9&&des_para.hPart==hMainObj))
			return TRUE;
		else
			return FALSE;
	}
	return FALSE;
}
static BOOL IsIncludeHandle(CLDSVector *pDatumVector,long hMainObj)
{
	if(pDatumVector&&hMainObj>0x20)
	{
		if(pDatumVector->hCrossPart==hMainObj
			||pDatumVector->hVicePart==hMainObj)
			return TRUE;
		else
			return FALSE;
	}
	return FALSE;
}
//1:pObj1������pObj2 pObj1��pObj2֮������
//0:pObj1��pObj2��������ϵ
//-1:pObj2������pObj1 pObj2��pObj1֮������
int CompareObjAttachRelation(const DBOBJPTR &pObj1,const DBOBJPTR &pObj2)
{
	if(pObj1==NULL||pObj2==NULL)
		return 0;
	//����ѥ�����������Ҫ�ȶԳ���˨�ף���Ӧ����˨������������Գ�֮ǰ wjh-2015.6.30
	if(pObj1->GetClassTypeId()==CLS_BOLT && pObj2->GetClassTypeId()!=CLS_BOLT)
		return -1;
	else if(pObj1->GetClassTypeId()!=CLS_BOLT && pObj2->GetClassTypeId()==CLS_BOLT)
		return  1;
	else if(pObj1->GetClassTypeId()==CLS_POLYJOINT&&pObj2->GetClassTypeId()!=CLS_POLYJOINT)
		return  1;	//�����ͷ�������Գ� wjh-2016.11.8
	for(int k=0;k<2;k++)
	{
		int nRetCode=1;
		long hMainObj=0;
		CLDSDbObject *pCurObj=NULL,*pMainObj=NULL;
		if(k==0)
		{	//�ж�pObj1�Ƿ�������pObj2
			hMainObj=pObj2->handle;
			pCurObj=pObj1;
			pMainObj=pObj2;
			nRetCode=1;	//pObj1������pObj2 pObj1��pObj2֮������
		}
		else 
		{	//�ж�pObj2�Ƿ�������pObj1
			hMainObj=pObj1->handle;
			pMainObj=pObj1;
			pCurObj=pObj2;
			nRetCode=-1;//pObj2������pObj1 pObj2��pObj1֮������
		}
		for(RELATIVE_OBJECT *pRelaObj=pCurObj->relativeObjs.GetFirst();pRelaObj;pRelaObj=pCurObj->relativeObjs.GetNext())
		{
			if(pRelaObj->hObj==hMainObj)
				return nRetCode; 
		}
		if(pCurObj->GetClassTypeId()==CLS_NODE)
		{
			CLDSNode *pNode=(CLDSNode*)pCurObj;
			if(pNode->hFatherPart==hMainObj)
				return nRetCode;
			for(int i=0;i<4;i++)
			{
				if(pNode->arrRelationNode[0]==hMainObj)
					return nRetCode;
			}
			if(pMainObj&&pMainObj->GetClassTypeId()==CLS_LINEANGLE)
			{	//�ڽ���ڵ㴦��Ϻ�������ϵ���ܳ���"����"���� ��ʱ���ж�������˼�Ϊ�Ǹֵ���� wht 10-03-02 
				if(pNode->arrRelationPole[0]==hMainObj||pNode->arrRelationPole[1]==hMainObj)
					return nRetCode;
			}
		}
		else if(pCurObj->IsPart())
		{	
			CLDSPart *pPart=(CLDSPart*)pCurObj;
			if(pPart->m_hPartWeldParent==hMainObj)
				return nRetCode;
			if(pPart->IsLinePart())
			{
				CLDSLinePart *pLinePart=(CLDSLinePart*)pPart;
				//���ն˽ڵ�
				if((pLinePart->pStart&&pLinePart->pStart->handle==hMainObj)
					||(pLinePart->pEnd&&pLinePart->pEnd->handle==hMainObj))
					return nRetCode;
				//����ͷ
				if(pLinePart->desStartOdd.m_hRefPart1==hMainObj||pLinePart->desStartOdd.m_hRefPart2==hMainObj
					||pLinePart->desEndOdd.m_hRefPart1==hMainObj||pLinePart->desEndOdd.m_hRefPart2==hMainObj)
					return nRetCode;
#ifdef __ANGLE_PART_INC_
				if(pPart->GetClassTypeId()==CLS_LINEANGLE
					||pPart->GetClassTypeId()==CLS_GROUPLINEANGLE)
				{
					CLDSLineAngle *pLineAngle=(CLDSLineAngle*)pPart;
					//ʼ��λ��
					if(pLineAngle->des_norm_x.hViceJg==hMainObj||pLineAngle->des_norm_y.hViceJg==hMainObj
						||pLineAngle->desStartPos.datum_jg_h==hMainObj||pLineAngle->desStartPos.huoqu_jg_h==hMainObj
						||pLineAngle->desStartPos.datumLineStart.des_para.RODEND.hRod==hMainObj
						||pLineAngle->desStartPos.datumLineEnd.des_para.RODEND.hRod==hMainObj)
						return nRetCode;
					//��׼��
					if(IsIncludeHandle(&pLineAngle->desStartPos.datumPoint,hMainObj))
						return nRetCode;
					//�ն�λ��
					if(pLineAngle->desStartPos.datum_jg_h==hMainObj||pLineAngle->desStartPos.huoqu_jg_h==hMainObj
						||pLineAngle->desStartPos.datumLineStart.des_para.RODEND.hRod==hMainObj
						||pLineAngle->desStartPos.datumLineEnd.des_para.RODEND.hRod==hMainObj)
						return nRetCode;
					//��׼��
					if(IsIncludeHandle(&pLineAngle->desEndPos.datumPoint,hMainObj))
						return nRetCode;
					if(pLineAngle->group_father_jg_h==hMainObj)
						return nRetCode;
					if(pPart->GetClassTypeId()==CLS_GROUPLINEANGLE)
					{
						for(int i=0;i<4;i++)
						{
							if(pLineAngle->son_jg_h[i]==hMainObj)
								return nRetCode;
						}
					}
				}
				else if(pCurObj->GetClassTypeId()==CLS_LINETUBE
					||pCurObj->GetClassTypeId()==CLS_LINESLOT
					||pCurObj->GetClassTypeId()==CLS_LINEFLAT)
				{
					CTubeEndPosPara *pCurStartPosPara=NULL,*pCurEndPosPara=NULL;
					if(pCurObj->GetClassTypeId()==CLS_LINETUBE)
					{
						CLDSLineTube *pLineTube=(CLDSLineTube*)pCurObj;
						if(pLineTube->hWorkNormRefPart==hMainObj
							||pLineTube->hTransTube==hMainObj
							||pLineTube->m_tJointStart.hLinkObj==hMainObj
							||pLineTube->m_tJointStart.hViceLinkObj==hMainObj
							||pLineTube->m_tJointEnd.hLinkObj==hMainObj
							||pLineTube->m_tJointEnd.hViceLinkObj==hMainObj)
							return nRetCode;
						pCurStartPosPara=&pLineTube->desStartPos;
						pCurEndPosPara=&pLineTube->desEndPos;
					}
					else if(pCurObj->GetClassTypeId()==CLS_LINESLOT)
					{
						pCurStartPosPara=&((CLDSLineSlot*)pCurObj)->desStartPos;
						pCurEndPosPara=&((CLDSLineSlot*)pCurObj)->desEndPos;
					}
					else if(pCurObj->GetClassTypeId()==CLS_LINEFLAT)
					{
						pCurStartPosPara=&((CLDSLineFlat*)pCurObj)->desStartPos;
						pCurEndPosPara=&((CLDSLineFlat*)pCurObj)->desEndPos;
					}
					if((pCurStartPosPara&&(pCurStartPosPara->hDatumPart==hMainObj||pCurStartPosPara->hDatumStartPart==hMainObj||pCurStartPosPara->hDatumEndPart==hMainObj))
						||(pCurEndPosPara&&(pCurEndPosPara->hDatumPart==hMainObj||pCurEndPosPara->hDatumStartPart==hMainObj||pCurEndPosPara->hDatumEndPart==hMainObj)))
						return nRetCode;
				}
#endif
			}
			/*else if(pPart->IsArcPart())
			{
				
			}*/
#ifdef __ANGLE_PART_INC_
			else if(pPart->GetClassTypeId()==CLS_PARAMPLATE)
			{
				DESIGN_PLATE_ITEM *pItem=NULL;
				CLDSParamPlate *pParamPlate=(CLDSParamPlate*)pPart;
				for(pItem=pParamPlate->params.GetFirst();pItem;pItem=pParamPlate->params.GetNext())
				{
					if(pItem->dataType==1&&pItem->value.hPart==hMainObj)
						return nRetCode;
				}		
			}
			else if(pPart->GetClassTypeId()==CLS_PLATE)
			{
				CLDSPlate *pPlate=(CLDSPlate*)pPart;
				if(pPlate->designInfo.m_hBaseNode==hMainObj
					||pPlate->designInfo.m_hBasePart==hMainObj)
					return nRetCode;
				//��׼��
				if(IsIncludeHandle(&pPlate->designInfo.origin,hMainObj))
					return nRetCode;
				int i=0;
				for(i=0;i<2;i++)
				{	
					if(IsIncludeHandle(&pPlate->designInfo.huoquline_start[i],hMainObj))
						return nRetCode;
					if(IsIncludeHandle(&pPlate->designInfo.huoquline_end[i],hMainObj))
						return nRetCode;
					if(IsIncludeHandle(&pPlate->designInfo.huoqufacenorm[i],hMainObj))
						return nRetCode;
				}
				if(IsIncludeHandle(&pPlate->designInfo.norm,hMainObj))
					return nRetCode;
				CDesignLjPartPara *pLjPara=NULL;
				for(pLjPara=pPlate->designInfo.partList.GetFirst();pLjPara;pLjPara=pPlate->designInfo.partList.GetNext())
				{
					if(pLjPara->hPart==hMainObj)
						return nRetCode;
				}
			}
			else if(pPart->GetClassTypeId()==CLS_BOLT)
			{
				CLDSBolt *pBolt=(CLDSBolt*)pPart;
				if(pBolt->des_base_pos.hPart==hMainObj)
					return nRetCode;
				if(IsIncludeHandle(&pBolt->des_base_pos.datumPoint,hMainObj))
					return nRetCode;
				if(IsIncludeHandle(&pBolt->des_work_norm,hMainObj))
					return nRetCode;
				//ƫ�����ַ���
				THICK_ITEM item;
				for(item=pBolt->des_base_pos.norm_offset.GetFirstItem();item.val!=0;
				item=pBolt->des_base_pos.norm_offset.GetNextItem())
				{
					if(item.IsHandle()&&abs(item.val)==hMainObj)
						return nRetCode;
				}
				for(item=pBolt->GetL0ThickPara()->GetFirstItem();item.val!=0;
				item=pBolt->GetL0ThickPara()->GetNextItem())
				{
					if(item.IsHandle()&&abs(item.val)==hMainObj)
						return nRetCode;
				}
			}
			else if(pPart->GetClassTypeId()==CLS_SPHERE)
			{
				CLDSSphere *pSphere=(CLDSSphere*)pPart;
				if(pSphere->hDatumNode==hMainObj)
					return nRetCode;
			}
#endif
		}
	}
#ifdef __COMMON_PART_INC_ //ifndef __TSA_
	//���������ڹ�����˨
	if(pObj1->IsPart()&&pObj2->IsPart())
	{
		CLsRefList *pLsRefList1=((CLDSPart*)pObj1)->GetLsRefList();
		CLsRefList *pLsRefList2=((CLDSPart*)pObj2)->GetLsRefList();
		if(pLsRefList1&&pLsRefList2)
		{	
			CLsRef *pLsRef=NULL;
			for(pLsRef=pLsRefList1->GetFirst();pLsRef;pLsRef=pLsRefList1->GetNext())
			{
				if(pLsRefList2->FromHandle(pLsRef->GetLsPtr()->handle))
					break;
			}
			if(pLsRef&&pLsRef->GetLsPtr())
			{	//���ڹ�����˨��Ӧ����������˨��λ����(��Ϊ�Գ����ɹ���ʱ��ͬʱ�Գ����ɹ����ϵ���˨) wht 12-03-19
				CLDSBolt *pBolt=pLsRef->GetLsPtr();
				if(pBolt->des_base_pos.hPart==pObj1->handle
					||IsIncludeHandle(&pBolt->des_base_pos.datumPoint,pObj1->handle))
					return -1;
				else if(pBolt->des_base_pos.hPart==pObj2->handle
					||IsIncludeHandle(&pBolt->des_base_pos.datumPoint,pObj2->handle))
					return 1;
			}
		}
	}
#endif
	return 0;
}
bool CInsertStdTemplateOperation::MirTemplate()
{
	if(m_iTemplateMir==0)
		return false;	//����Գ�
	//���ݹ��������ͽ�������
	ARRAY_LIST<DBOBJPTR> dbObjPtrArr;
	int i,n=m_arrMirSumLdsNode.GetSize()+m_arrMirSumLdsLinePart.GetSize();
	dbObjPtrArr.SetSize(n);
	for(i=0;i<n;i++)
	{
		if(i<m_arrMirSumLdsNode.GetSize())
			dbObjPtrArr[i] = m_arrMirSumLdsNode[i];
		else
			dbObjPtrArr[i] = m_arrMirSumLdsLinePart[i-m_arrMirSumLdsNode.GetSize()];
	}
	CBubbleSort<DBOBJPTR>::BubSort(dbObjPtrArr.m_pData,n,CompareObjAttachRelation);
	MIRMSG mirmsg;
	if(m_iTemplateMir==1)	//X��Գ�
		mirmsg.axis_flag=1;
	else if(m_iTemplateMir==2)	//Y��Գ�
		mirmsg.axis_flag=2;
	else if(m_iTemplateMir==3)	//Z��Գ�
		mirmsg.axis_flag=4;
	else if(m_iTemplateMir==4)	//����
	{
		mirmsg.axis_flag=8;
		mirmsg.array_num=1;
		mirmsg.rotate_angle=90;
	}
	else if(m_iTemplateMir==5)	//Z�Գ�&����
	{
		mirmsg.axis_flag=12;
		mirmsg.array_num=1;
		mirmsg.rotate_angle=90;
	}
	m_pTower->Parts.GetTail();
	BOOL pushed=m_pTower->Parts.push_stack();
	CHash64List<bool>hashExistParts;
	for(i=0;i<n;i++)
	{
		if(dbObjPtrArr[i]->IsPart()&&((CLDSPart*)dbObjPtrArr[i])->GetMirPart(mirmsg)!=NULL)
			hashExistParts.Add(DUALKEY(dbObjPtrArr[i]->handle,mirmsg.axis_flag));
		//MirTaAtom(dbObjPtrArr[i],mirmsg,FALSE);
	}
/*
	�ڶ๹��һ��Գ�ʱ��ǰһ������������Ϣ������õ���һ�������Գ�ǰһ����ʱ�����ں�һ�����ĶԳƹ���
	��δ���ɣ��ᵼ��ĳЩ������Ϣ�еľ�����ԶԳƶ�ʧ����˶������˺�����Ҫ������ȫ���Գƹ��������ɺ�
	����һЩ���ֵ���ԵĶԳƹ�����
*/
	for(i=0;i<n;i++)
	{
		//if(hashExistParts.GetValue(DUALKEY(dbObjPtrArr[i]->handle,mirmsg.axis_flag))==NULL)
			//MirTaAtom(dbObjPtrArr[i],mirmsg,TRUE);
	}
	m_pTower->Parts.pop_stack(pushed);
	//for(CLDSPart *pPart=m_pTower->Parts.GetNext();pPart;pPart=m_pTower->Parts.GetNext())
		//g_pSolidDraw->SetEntSnapStatus(pPart->handle);
	return true;
}
