// ShankDefCls.cpp: implementation of the CShankSlope class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TPSM.h"
#include "ShankDefCls.h"
#include "InsertStdTemplateOperation.h"
#include "LogFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#if defined(__LDS_FILE_)
#include "DrawEngine.h"
//////////////////////////////////////////////////////////////////////
// 局部函数
//////////////////////////////////////////////////////////////////////
SKETCH_SIZEDIM::tagSKETCH_SIZEDIM()
{
	memset(this,0,sizeof(tagSKETCH_SIZEDIM));
	cSizeType='L';
	fTextHeight=300;
	wAlignStyle=CDrawEngine::BottomCenter;
}
double SKETCH_SIZEDIM::Size()
{
	if(cSizeType=='X')
		return fabs(dimEnd.x-dimStart.x);
	else if(cSizeType=='Y')
		return fabs(dimEnd.y-dimStart.y);
	else if(cSizeType=='Z')
		return fabs(dimEnd.z-dimStart.z);
	else
		return DISTANCE(dimEnd,dimStart);
}
//////////////////////////////////////////////////////////////////////
// 局部函数
//////////////////////////////////////////////////////////////////////
int CShankDefineToTowerModel::m_iViewType;
CShankDefine *CShankDefineToTowerModel::m_pDefine;
CTower* CShankDefineToTowerModel::m_pTower=NULL;
ARRAY_LIST<SKETCH_SIZEDIM> CShankDefineToTowerModel::dims;
static void FillLinePartRelativeInfo(CSuperSmartPtr<CLDSLinePart> linePartArr[],int nPoly)
{
	if(nPoly!=3&&nPoly!=4&&nPoly!=5&&nPoly!=6&&nPoly!=8)
		return;
	int j=0;
	if(3==nPoly)
	{
#ifndef __TSA_
		if(linePartArr[0]->GetClassTypeId()==CLS_LINEANGLE)
		{	//角钢
			linePartArr[0].LineAnglePointer()->des_norm_x.hViceJg=linePartArr[2]->handle;
			linePartArr[0].LineAnglePointer()->des_norm_x.near_norm.Set(-1,0,0);
			linePartArr[0].LineAnglePointer()->des_norm_y.hViceJg=linePartArr[1]->handle;
			linePartArr[0].LineAnglePointer()->des_norm_y.near_norm.Set(0,1,0);
			linePartArr[1].LineAnglePointer()->des_norm_x.hViceJg=linePartArr[0]->handle;
			linePartArr[1].LineAnglePointer()->des_norm_x.near_norm.Set(0,1,0);
			linePartArr[1].LineAnglePointer()->des_norm_y.hViceJg=linePartArr[2]->handle;
			linePartArr[1].LineAnglePointer()->des_norm_y.near_norm.Set(1,0,0);
			linePartArr[2].LineAnglePointer()->des_norm_x.hViceJg=linePartArr[1]->handle;
			linePartArr[2].LineAnglePointer()->des_norm_x.near_norm.Set(1,0,0);
			linePartArr[2].LineAnglePointer()->des_norm_y.hViceJg=linePartArr[0]->handle;
			linePartArr[2].LineAnglePointer()->des_norm_y.near_norm.Set(-1,0,0);
		}
#endif
#ifdef __COMPLEX_PART_INC_//defined(__LMA_)||defined(__LDS_)||defined(__TDA_)
		if(linePartArr[0]->GetClassTypeId()==CLS_LINETUBE)
		{	//钢管 三管塔主材基准面计算方式默认为共面参照点 wht 15-08-12
			linePartArr[0].LineTubePointer()->m_iNormCalStyle=2;
			linePartArr[1].LineTubePointer()->m_iNormCalStyle=2;
			linePartArr[2].LineTubePointer()->m_iNormCalStyle=2;
			linePartArr[0].LineTubePointer()->nearWorkPlaneNorm.Set(0,0,0);	//此处使用nearWorkPlaneNorm给pickWorkPlane赋值仅仅是为减少代码行数
			linePartArr[1].LineTubePointer()->nearWorkPlaneNorm.Set(0,0,0);
			linePartArr[2].LineTubePointer()->nearWorkPlaneNorm.Set(0,0,0);
		}
#endif
		if(linePartArr[0]->GetClassTypeId()==CLS_LINEPART)
		{	//杆件
			linePartArr[0]->des_wing_norm.hRefPoleArr[0]=linePartArr[2]->handle;
			linePartArr[0]->des_wing_norm.hRefPoleArr[1]=linePartArr[1]->handle;
			linePartArr[1]->des_wing_norm.hRefPoleArr[0]=linePartArr[0]->handle;
			linePartArr[1]->des_wing_norm.hRefPoleArr[1]=linePartArr[2]->handle;
			linePartArr[2]->des_wing_norm.hRefPoleArr[0]=linePartArr[1]->handle;
			linePartArr[2]->des_wing_norm.hRefPoleArr[1]=linePartArr[0]->handle;
			linePartArr[0]->des_wing_norm.viewNorm.Set(0,1,0);
			linePartArr[1]->des_wing_norm.viewNorm.Set(0,1,0);
			linePartArr[2]->des_wing_norm.viewNorm.Set(0,-1,0);
		}
	}
	else if(4==nPoly)
	{
#ifndef __TSA_
		if(linePartArr[0]->GetClassTypeId()==CLS_LINEANGLE)
		{	//角钢
			linePartArr[0].LineAnglePointer()->des_norm_x.hViceJg=linePartArr[3]->handle;
			linePartArr[0].LineAnglePointer()->des_norm_x.near_norm.Set(-1,0,0);
			linePartArr[0].LineAnglePointer()->des_norm_y.hViceJg=linePartArr[1]->handle;
			linePartArr[0].LineAnglePointer()->des_norm_y.near_norm.Set(0,1,0);
			linePartArr[1].LineAnglePointer()->des_norm_x.hViceJg=linePartArr[0]->handle;
			linePartArr[1].LineAnglePointer()->des_norm_x.near_norm.Set(0,1,0);
			linePartArr[1].LineAnglePointer()->des_norm_y.hViceJg=linePartArr[2]->handle;
			linePartArr[1].LineAnglePointer()->des_norm_y.near_norm.Set(1,0,0);
			linePartArr[2].LineAnglePointer()->des_norm_x.hViceJg=linePartArr[1]->handle;
			linePartArr[2].LineAnglePointer()->des_norm_x.near_norm.Set(1,0,0);
			linePartArr[2].LineAnglePointer()->des_norm_y.hViceJg=linePartArr[3]->handle;
			linePartArr[2].LineAnglePointer()->des_norm_y.near_norm.Set(0,-1,0);
			linePartArr[3].LineAnglePointer()->des_norm_x.hViceJg=linePartArr[2]->handle;
			linePartArr[3].LineAnglePointer()->des_norm_x.near_norm.Set(0,-1,0);
			linePartArr[3].LineAnglePointer()->des_norm_y.hViceJg=linePartArr[0]->handle;
			linePartArr[3].LineAnglePointer()->des_norm_y.near_norm.Set(-1,0,0);
		}
#endif
#ifdef __COMPLEX_PART_INC_//defined(__LMA_)||defined(__LDS_)||defined(__TDA_)
		if(linePartArr[0]->GetClassTypeId()==CLS_LINETUBE)
		{	//钢管
			linePartArr[0].LineTubePointer()->hWorkNormRefPart=linePartArr[1]->handle;
			linePartArr[1].LineTubePointer()->hWorkNormRefPart=linePartArr[0]->handle;
			linePartArr[2].LineTubePointer()->hWorkNormRefPart=linePartArr[3]->handle;
			linePartArr[3].LineTubePointer()->hWorkNormRefPart=linePartArr[2]->handle;
			linePartArr[0].LineTubePointer()->nearWorkPlaneNorm.Set(0,1,0);
			linePartArr[1].LineTubePointer()->nearWorkPlaneNorm.Set(0,1,0);
			linePartArr[2].LineTubePointer()->nearWorkPlaneNorm.Set(0,-1,0);
			linePartArr[3].LineTubePointer()->nearWorkPlaneNorm.Set(0,-1,0);
		}
#endif
		if(linePartArr[0]->GetClassTypeId()==CLS_LINEPART)
		{	//杆件
			linePartArr[0]->des_wing_norm.hRefPoleArr[0]=linePartArr[1]->handle;
			linePartArr[0]->des_wing_norm.hRefPoleArr[1]=linePartArr[3]->handle;
			linePartArr[1]->des_wing_norm.hRefPoleArr[0]=linePartArr[0]->handle;
			linePartArr[1]->des_wing_norm.hRefPoleArr[1]=linePartArr[2]->handle;
			linePartArr[2]->des_wing_norm.hRefPoleArr[0]=linePartArr[3]->handle;
			linePartArr[2]->des_wing_norm.hRefPoleArr[1]=linePartArr[1]->handle;
			linePartArr[3]->des_wing_norm.hRefPoleArr[0]=linePartArr[2]->handle;
			linePartArr[3]->des_wing_norm.hRefPoleArr[1]=linePartArr[0]->handle;
			linePartArr[0]->des_wing_norm.viewNorm.Set(0,1,0);
			linePartArr[1]->des_wing_norm.viewNorm.Set(0,1,0);
			linePartArr[2]->des_wing_norm.viewNorm.Set(0,-1,0);
			linePartArr[3]->des_wing_norm.viewNorm.Set(0,-1,0);
		}
	}
	else if(5==nPoly)
	{
		double piece_angle=Pi*0.4;
		for(j=0;j<nPoly;j++)
		{
#ifndef __TSA_
			if(linePartArr[0]->GetClassTypeId()==CLS_LINEANGLE)
			{	//角钢
				linePartArr[j].LineAnglePointer()->des_norm_x.hViceJg=linePartArr[(j+4)%5]->handle;
				linePartArr[j].LineAnglePointer()->des_norm_x.near_norm.Set(sin(piece_angle*(j-1)),cos(piece_angle*(j-1)),0);
				linePartArr[j].LineAnglePointer()->des_norm_y.hViceJg=linePartArr[(j+1)%5]->handle;
				linePartArr[j].LineAnglePointer()->des_norm_y.near_norm.Set(sin(piece_angle*j),cos(piece_angle*j),0);
			}
#endif
#ifdef __COMPLEX_PART_INC_//defined(__LMA_)||defined(__LDS_)
			if(linePartArr[0]->GetClassTypeId()==CLS_LINETUBE)
			{	//钢管
				//linePartArr[j].LineTubePointer()->hWorkNormRefPart=linePartArr[(j+4)%5]->handle;
				//linePartArr[j].LineTubePointer()->nearWorkPlaneNorm.Set(sin(piece_angle*j),cos(piece_angle*j),0);
				//五边管塔主材基准面计算方式默认为共面参照点 wht 15-08-12
				linePartArr[j].LineTubePointer()->m_iNormCalStyle=2;
				linePartArr[j].LineTubePointer()->pickWorkPlane[0]=0;
				linePartArr[j].LineTubePointer()->pickWorkPlane[1]=0;
				linePartArr[j].LineTubePointer()->pickWorkPlane[2]=0;
			}
#endif
			if(linePartArr[0]->GetClassTypeId()==CLS_LINEPART)
			{	//杆件
				linePartArr[j]->des_wing_norm.hRefPoleArr[0]=linePartArr[(j+4)%5]->handle;
				linePartArr[j]->des_wing_norm.hRefPoleArr[1]=linePartArr[(j+1)%5]->handle;
				if(j==0||j==1)
					linePartArr[j]->des_wing_norm.viewNorm.Set(0,1,0);
				else if(j==3)
					linePartArr[j]->des_wing_norm.viewNorm.Set(1,0,0);
				else if(j==4)
					linePartArr[j]->des_wing_norm.viewNorm.Set(0,-1,0);
				else if(j==5)
					linePartArr[j]->des_wing_norm.viewNorm.Set(-1,0,0);
			}
		}
	}
	else if(6==nPoly)
	{
		double piece_angle=Pi/3;
		for(j=0;j<nPoly;j++)
		{
#ifndef __TSA_
			if(linePartArr[0]->GetClassTypeId()==CLS_LINEANGLE)
			{	//角钢
				linePartArr[j].LineAnglePointer()->des_norm_x.hViceJg=linePartArr[(j+5)%6]->handle;
				linePartArr[j].LineAnglePointer()->des_norm_x.near_norm.Set(-cos(piece_angle*(j-0.5)),sin(piece_angle*(j-0.5)),0);
				linePartArr[j].LineAnglePointer()->des_norm_y.hViceJg=linePartArr[(j+1)%6]->handle;
				linePartArr[j].LineAnglePointer()->des_norm_y.near_norm.Set(-cos(piece_angle*(j+0.5)),sin(piece_angle*(j+0.5)),0);
			}
#endif
#ifdef __COMPLEX_PART_INC_//defined(__LMA_)||defined(__LDS_)
			if(linePartArr[0]->GetClassTypeId()==CLS_LINETUBE)
			{	//钢管
				//linePartArr[j].LineTubePointer()->hWorkNormRefPart=linePartArr[(j+5)%6]->handle;
				//linePartArr[j].LineTubePointer()->nearWorkPlaneNorm.Set(-cos(piece_angle*(j-0.5)),sin(piece_angle*(j-0.5)),0);
				//六边管塔主材基准面计算方式默认为共面参照点 wht 15-08-12
				linePartArr[j].LineTubePointer()->m_iNormCalStyle=2;
				linePartArr[j].LineTubePointer()->pickWorkPlane[0]=0;
				linePartArr[j].LineTubePointer()->pickWorkPlane[1]=0;
				linePartArr[j].LineTubePointer()->pickWorkPlane[2]=0;
			}
#endif
			if(linePartArr[0]->GetClassTypeId()==CLS_LINEPART)
			{	//杆件
				linePartArr[j]->des_wing_norm.hRefPoleArr[0]=linePartArr[(j+5)%6]->handle;
				linePartArr[j]->des_wing_norm.hRefPoleArr[1]=linePartArr[(j+1)%6]->handle;
				if(j==0)
					linePartArr[j]->des_wing_norm.viewNorm.Set(0,-1,0);
				else if(j==2||j==3)
					linePartArr[j]->des_wing_norm.viewNorm.Set(0,1,0);
				else if(j==4)
					linePartArr[j]->des_wing_norm.viewNorm.Set(0,1,0);
				else if(j==5||j==6)
					linePartArr[j]->des_wing_norm.viewNorm.Set(0,-1,0);
			}
		}
	}
	else if(8==nPoly)
	{
		double piece_angle=Pi/4;
		for(j=0;j<8;j++)
		{
#ifndef __TSA_
			if(linePartArr[0]->GetClassTypeId()==CLS_LINEANGLE)
			{	//角钢
				linePartArr[j].LineAnglePointer()->des_norm_x.hViceJg=linePartArr[(j+7)%8]->handle;
				linePartArr[j].LineAnglePointer()->des_norm_x.near_norm.Set(-cos(piece_angle*j),sin(piece_angle*j),0);
				linePartArr[j].LineAnglePointer()->des_norm_y.hViceJg=linePartArr[(j+1)%8]->handle;
				linePartArr[j].LineAnglePointer()->des_norm_y.near_norm.Set(-cos(piece_angle*(j+1)),sin(piece_angle*(j+1)),0);
			}
#endif
#ifdef __COMPLEX_PART_INC_//defined(__LMA_)||defined(__LDS_)
			if(linePartArr[0]->GetClassTypeId()==CLS_LINETUBE)
			{	//钢管
				//linePartArr[j].LineTubePointer()->hWorkNormRefPart=linePartArr[(j+7)%8]->handle;
				//linePartArr[j].LineTubePointer()->nearWorkPlaneNorm.Set(-cos(piece_angle*j),sin(piece_angle*j),0);
				//八边管塔主材基准面计算方式默认为共面参照点 wht 15-08-12
				linePartArr[j].LineTubePointer()->m_iNormCalStyle=2;
				linePartArr[j].LineTubePointer()->pickWorkPlane[0]=0;
				linePartArr[j].LineTubePointer()->pickWorkPlane[1]=0;
				linePartArr[j].LineTubePointer()->pickWorkPlane[2]=0;
			}
#endif
			if(linePartArr[0]->GetClassTypeId()==CLS_LINEPART)
			{	//杆件
				linePartArr[j]->des_wing_norm.hRefPoleArr[0]=linePartArr[(j+7)%8]->handle;
				linePartArr[j]->des_wing_norm.hRefPoleArr[1]=linePartArr[(j+1)%8]->handle;
				if(j<4)
					linePartArr[j]->des_wing_norm.viewNorm.Set(0,1,0);
				else 
					linePartArr[j]->des_wing_norm.viewNorm.Set(0,-1,0);
			}
		}
	}
	for(j=0;j<nPoly;j++)
	{	//填充对称关联信息
		for(int k=0; k<nPoly; k++)
		{
			if(j==k)	//跳过本身
				continue;
			RELATIVE_OBJECT *pMirObj=linePartArr[j]->relativeObjs.Add(linePartArr[k]->handle);
			if(nPoly==4)
			{	//此处根据杆件端节点的对称关系就可以直接确定杆件的对称关系
				if(linePartArr[j]->pStart->Position(false).x==linePartArr[k]->pStart->Position(false).x&&
					linePartArr[j]->pStart->Position(false).y==-linePartArr[k]->pStart->Position(false).y)
					pMirObj->mirInfo=MIRMSG(1);	//X轴对称
				else if(linePartArr[j]->pStart->Position(false).x==-linePartArr[k]->pStart->Position(false).x&&
					linePartArr[j]->pStart->Position(false).y==linePartArr[k]->pStart->Position(false).y)
					pMirObj->mirInfo=MIRMSG(2);	//Y轴对称
				else if(linePartArr[j]->pStart->Position(false).x==-linePartArr[k]->pStart->Position(false).x&&
					linePartArr[j]->pStart->Position(false).y==-linePartArr[k]->pStart->Position(false).y)
					pMirObj->mirInfo=MIRMSG(3);	//Z轴对称
			}
			else
			{
				pMirObj->mirInfo.axis_flag=8;
				pMirObj->mirInfo.rotate_angle=-(k-j)*360/nPoly;
			}
		}
	}
}
static void FillNodeRelativeInfo(CLDSNodePtr nodeArr[],int nPoly)
{
	if(nPoly!=3&&nPoly!=4&&nPoly!=5&&nPoly!=6&&nPoly!=8)
		return;
	for(int j=0;j<nPoly;j++)
	{	//填充对称关联信息
		for(int k=0; k<nPoly; k++)
		{
			if(j==k)	//跳过本身
				continue;
			RELATIVE_OBJECT mirObj;
			mirObj.hObj=nodeArr[k]->handle;
			if(4==nPoly)
			{	//此处根据杆件端节点的对称关系就可以直接确定杆件的对称关系
				if(nodeArr[j]->Position(false).x==nodeArr[k]->Position(false).x&&
					nodeArr[j]->Position(false).y==-nodeArr[k]->Position(false).y)
					mirObj.mirInfo=MIRMSG(1);	//X轴对称
				else if(nodeArr[j]->Position(false).x==-nodeArr[k]->Position(false).x&&
					nodeArr[j]->Position(false).y==nodeArr[k]->Position(false).y)
					mirObj.mirInfo=MIRMSG(2);	//Y轴对称
				else if(nodeArr[j]->Position(false).x==-nodeArr[k]->Position(false).x&&
					nodeArr[j]->Position(false).y==-nodeArr[k]->Position(false).y)
					mirObj.mirInfo=MIRMSG(3);	//Z轴对称
			}
			else
			{
				mirObj.mirInfo.axis_flag=8;
				mirObj.mirInfo.rotate_angle=-(k-j)*360/nPoly;
			}
			nodeArr[j]->AppendRelativeObj(mirObj);
		}
	}
}
static void AdjustLinePartPosPara(LINEPARTSET &linePartList,const SLOPE_POSINFO& slopePosInfo)
{
	CSuperSmartPtr<CLDSLinePart> pStartLinePart=linePartList.GetFirst(),pEndLinePart=linePartList.GetTail();
	if(pStartLinePart.IsNULL()||pEndLinePart.IsNULL())
		return;
	int i=0;
	f3dPoint pos;
	f3dLine ber_line(pStartLinePart->Start(),pEndLinePart->End());
	if(pStartLinePart->GetClassTypeId()==CLS_LINEANGLE)
	{
		CSuperSmartPtr<CLDSLinePart> pLinePart;
		for(pLinePart=linePartList.GetFirst();pLinePart.IsHasPtr();pLinePart=linePartList.GetNext())
		{	
			if(pLinePart.IsNULL())
				continue;
#ifdef __SMART_DRAW_INC_//__LDS_
			//添加"!slopePosInfo.bBottomSlope"主要是用于中间坡段末端应设为向下一坡段起始角钢楞线投影
			if(pLinePart!=pEndLinePart||!slopePosInfo.bBottomSlope||pLinePart->pEnd->Position(false).z<slopePosInfo.coordBottomZ)
			{	//不是终端分段杆件或当前坡度下面还接有其它坡度
				pLinePart.LineAnglePointer()->desEndPos.jgber_cal_style=0;	//点向楞线投影
				pLinePart.LineAnglePointer()->desEndPos.spatialOperationStyle=1;
				pLinePart.LineAnglePointer()->desEndPos.wing_x_offset.offsetDist=0;
				pLinePart.LineAnglePointer()->desEndPos.wing_y_offset.offsetDist=0;
				pLinePart.LineAnglePointer()->desEndPos.wing_x_offset.gStyle=4;
				pLinePart.LineAnglePointer()->desEndPos.wing_y_offset.gStyle=4;
				SnapPerp(&pos,ber_line,pLinePart->pEnd->Position(false));
				pLinePart->SetEnd(pos);
			}
			else //终端分段杆件
			{
				//pLinePart.LineAnglePointer()->desEndPos.datum_to_ber_style=1;	//平推
				pLinePart.LineAnglePointer()->desEndPos.spatialOperationStyle=19;
				pLinePart.LineAnglePointer()->desEndPos.cFaceOffsetDatumLine='X';
				if((pLinePart->pEnd->Position(false).x<0&&pLinePart->pEnd->Position(false).y>0)||
				   (pLinePart->pEnd->Position(false).x>0&&pLinePart->pEnd->Position(false).y<0))
					pLinePart.LineAnglePointer()->desEndPos.cFaceOffsetDatumLine='Y';
			}

			if(pLinePart!=pStartLinePart)
			{	//不是始分段杆件
				//pLinePart.LineAnglePointer()->des_start_pos.jgber_cal_style=1;	//点线投影
				pLinePart.LineAnglePointer()->desStartPos.spatialOperationStyle=16;
				pLinePart.LineAnglePointer()->desStartPos.datumLineStart.datum_pos_style=
					pLinePart.LineAnglePointer()->desStartPos.datumLineEnd.datum_pos_style=1;
				pLinePart.LineAnglePointer()->desStartPos.datumLineStart.des_para.RODEND.hRod=pStartLinePart->handle;
				pLinePart.LineAnglePointer()->desStartPos.datumLineEnd.des_para.RODEND.hRod=pEndLinePart->handle;
				pLinePart.LineAnglePointer()->desStartPos.datumLineStart.des_para.RODEND.direction=0;
				pLinePart.LineAnglePointer()->desStartPos.datumLineEnd.des_para.RODEND.direction=1;
				pLinePart.LineAnglePointer()->desStartPos.datumLineStart.des_para.RODEND.wing_offset_style=
					pLinePart.LineAnglePointer()->desStartPos.datumLineEnd.des_para.RODEND.wing_offset_style=4;
				pLinePart.LineAnglePointer()->desStartPos.datumLineStart.des_para.RODEND.wing_offset_dist=
					pLinePart.LineAnglePointer()->desStartPos.datumLineEnd.des_para.RODEND.wing_offset_dist=0;
				/*pLinePart.LineAnglePointer()->desStartPos.start_jg_h=pStartLinePart->handle;
				pLinePart.LineAnglePointer()->desStartPos.end_jg_h=pEndLinePart->handle;
				pLinePart.LineAnglePointer()->desStartPos.iStartJgBerStyle=0;
				pLinePart.LineAnglePointer()->desStartPos.iEndJgBerStyle=1;
				pLinePart.LineAnglePointer()->desStartPos.fStartJgLengOffsetX=
					pLinePart.LineAnglePointer()->desStartPos.fStartJgLengOffsetY=
					pLinePart.LineAnglePointer()->desStartPos.fEndJgLengOffsetX=
					pLinePart.LineAnglePointer()->desStartPos.fEndJgLengOffsetY=0;*/
				if((pStartLinePart.LineAnglePointer()->desStartPos.spatialOperationStyle==19||
					pStartLinePart.LineAnglePointer()->desStartPos.spatialOperationStyle==17)&&
					pStartLinePart.LineAnglePointer()->desStartPos.jgber_cal_style!=1) //平推
				{
					pLinePart.LineAnglePointer()->desStartPos.spatialOperationStyle=17;	//点线平推
					pLinePart.LineAnglePointer()->desStartPos.face_offset_norm=
						pStartLinePart.LineAnglePointer()->desStartPos.face_offset_norm;
					//pLinePart.LineAnglePointer()->desStartPos.bByOtherEndOffset=
					//	pStartLinePart.LineAnglePointer()->desStartPos.bByOtherEndOffset;
					pLinePart.LineAnglePointer()->desStartPos.cFaceOffsetDatumLine=
						pStartLinePart.LineAnglePointer()->desStartPos.cFaceOffsetDatumLine;
					pLinePart.LineAnglePointer()->desStartPos.cFaceOffsetDatumLine='X';
					if((pLinePart->pEnd->Position(false).x<0&&pLinePart->pEnd->Position(false).y>0)||
					   (pLinePart->pEnd->Position(false).x>0&&pLinePart->pEnd->Position(false).y<0))
						pLinePart.LineAnglePointer()->desStartPos.cFaceOffsetDatumLine='Y';
				}
				SnapPerp(&pos,ber_line,pLinePart->pStart->Position(false));
				pLinePart->SetStart(pos);
			}
			else
			{
				//pLinePart.LineAnglePointer()->des_start_pos.datum_to_ber_style=1;	//平推
				pLinePart.LineAnglePointer()->desStartPos.spatialOperationStyle=19;
				if(!slopePosInfo.bTopSlope)	//当前最高分段上面还接有其它坡度段，此时为保证变坡面必须以楞线为平推基准
					pLinePart.LineAnglePointer()->desStartPos.cFaceOffsetDatumLine=0;
				else
				{
					pLinePart.LineAnglePointer()->desStartPos.cFaceOffsetDatumLine='X';
					if((pLinePart->pEnd->Position(false).x<0&&pLinePart->pEnd->Position(false).y>0)||
						(pLinePart->pEnd->Position(false).x>0&&pLinePart->pEnd->Position(false).y<0))
						pLinePart.LineAnglePointer()->desStartPos.cFaceOffsetDatumLine='Y';
				}
			}
			//else //始端分段杆件
			if(pLinePart!=pStartLinePart)
			{
				pLinePart.LineAnglePointer()->des_norm_x=pStartLinePart.LineAnglePointer()->des_norm_x;
				pLinePart.LineAnglePointer()->des_norm_y=pStartLinePart.LineAnglePointer()->des_norm_y;
				pLinePart.LineAnglePointer()->set_norm_x_wing(pStartLinePart.LineAnglePointer()->get_norm_x_wing());
				pLinePart.LineAnglePointer()->set_norm_y_wing(pStartLinePart.LineAnglePointer()->get_norm_y_wing());
			}
			pLinePart.LineAnglePointer()->des_norm_x.bSpecific=FALSE;
			pLinePart.LineAnglePointer()->des_norm_y.bSpecific=FALSE;
			pLinePart.LineAnglePointer()->des_norm_x.bByOtherWing=FALSE;
			pLinePart.LineAnglePointer()->des_norm_y.bByOtherWing=FALSE;
			//对于非末了坡段的终端节点父杆件有时是在下一坡段设定的，这样由于父杆件不对可能会导致位置计算错误异常
			//故在此处不计算位置而是在整塔建完后统一计算
			//pLinePart.LinePartPointer()->CalPosition();
			pLinePart.LineAnglePointer()->cal_x_wing_norm();
			pLinePart.LineAnglePointer()->cal_y_wing_norm();
#else
			//添加"!slopePosInfo.bBottomSlope"主要是用于中间坡段末端应设为向下一坡段起始角钢楞线投影
			if(pLinePart!=pEndLinePart||!slopePosInfo.bBottomSlope)
			{	//不是终端分段杆件或当前坡度下面还接有其它坡度
				SnapPerp(&pos,ber_line,pLinePart->pEnd->Position(false));
				pLinePart->SetEnd(pos);
			}

			if(pLinePart!=pStartLinePart)
			{	//不是始分段杆件
				SnapPerp(&pos,ber_line,pLinePart->pStart->Position(false));
				pLinePart->SetStart(pos);
			}
#endif
		}
	}
	else if(pStartLinePart->GetClassTypeId()==CLS_LINETUBE)
	{
		CSuperSmartPtr<CLDSLinePart> pLinePart;
		for(pLinePart=linePartList.GetFirst();pLinePart.IsHasPtr();pLinePart=linePartList.GetNext())
		{	
			if(pLinePart.IsNULL())
				continue;
#ifdef __SMART_DRAW_INC_//__LDS_
			//被打断的杆件起始/终止点定位方式都不是点线投影
			if(pLinePart!=pEndLinePart)
			{	//不是终分段杆件
				pLinePart.LineTubePointer()->desEndPos.endPosType=2;	//点线投影
				pLinePart.LineTubePointer()->desEndPos.bDatumStartStartEnd=TRUE;
				pLinePart.LineTubePointer()->desEndPos.bDatumEndStartEnd=FALSE;
				pLinePart.LineTubePointer()->desEndPos.hDatumStartPart=pStartLinePart->handle;
				pLinePart.LineTubePointer()->desEndPos.hDatumEndPart=pEndLinePart->handle;
				SnapPerp(&pos,ber_line,pLinePart->pEnd->Position(false));
				pLinePart->SetEnd(pos);
			}
			//else //终端分段杆件
			if(pLinePart!=pStartLinePart)
			{	//不是始分段杆件
				pLinePart.LineTubePointer()->desStartPos.endPosType=2;	//点线投影
				pLinePart.LineTubePointer()->desStartPos.bDatumStartStartEnd=TRUE;
				pLinePart.LineTubePointer()->desStartPos.bDatumEndStartEnd=FALSE;
				pLinePart.LineTubePointer()->desStartPos.hDatumStartPart=pStartLinePart->handle;
				pLinePart.LineTubePointer()->desStartPos.hDatumEndPart=pEndLinePart->handle;
				SnapPerp(&pos,ber_line,pLinePart->pStart->Position(false));
				pLinePart->SetStart(pos);
			}
			//else //始端分段杆件
			if(pLinePart!=pStartLinePart)
			{
				pLinePart.LineTubePointer()->nearWorkPlaneNorm=pStartLinePart.LineTubePointer()->nearWorkPlaneNorm;
				pLinePart.LineTubePointer()->hWorkNormRefPart=pStartLinePart.LineTubePointer()->hWorkNormRefPart;
			}
			pLinePart.LineTubePointer()->CalPosition();
			pLinePart.LineTubePointer()->CalWorkPlaneNorm();
#else
			//被打断的杆件起始/终止点定位方式都不是点线投影
			if(pLinePart!=pEndLinePart)
			{	//不是终分段杆件
				SnapPerp(&pos,ber_line,pLinePart->pEnd->Position(false));
				pLinePart->SetEnd(pos);
			}
			//else //终端分段杆件
			if(pLinePart!=pStartLinePart)
			{	//不是始分段杆件
				SnapPerp(&pos,ber_line,pLinePart->pStart->Position(false));
				pLinePart->SetStart(pos);
			}
#endif
		}
	}
}
void CShankDefineToTowerModel::AddShankModule(CShankModule *pModuleDef,SECTION_NODEARR startNodeArr,SLOPE_POSINFO& slopePosInfo,
					 SECTION_NODEARR& slopeStartNodeArr,SECTION_NODEARR slopeEndNodeArr,int nPoly,int idLinePartClsType)
{
	CLDSModule *pModule;
	if(pModuleDef->IsFirstModule())	//系统默认存在的呼高组模型,通过GetActiveModule()获取初始呼高，不稳定且限制了m_hActiveModule账值　wjh-2014.4.19
		pModule=m_pTower->Module.GetFirst();//GetActiveModule();
	else
	{
		pModule=m_pTower->Module.append();
		CObjNoGroup *pModuleNoGroup=m_pTower->NoManager.FromGroupID(MODEL_GROUP);
		pModule->iNo=pModuleNoGroup->EnumIdleNo();
		pModuleNoGroup->SetNoState(pModule->iNo);
		//此处不能更改激活呼高句柄，之后可能需要根据激活呼高句柄找到InitModule()时添加的默认呼高
		//修改后将找不到默认呼高，导致呼高配置错误 wht 13-03-30
		//m_pTower->m_hActiveModule=pModule->handle;	//激活当前呼高
	}
	strcpy(pModule->description,pModuleDef->name);
	pModule->SetBodyNo(pModuleDef->m_iBodyNo);
	pModule->m_dwLegCfgWord=pModuleDef->legword.ToCfgWord();
	memcpy(pModule->m_arrActiveQuadLegNo,pModuleDef->m_arrActiveQuadLegNo,sizeof(WORD)*4);
	int i;
	LINEPARTSET linePartList[8];
	SECTION_NODEARR startTempNodeArr=startNodeArr;
	for(int k=0;k<nPoly;k++)
		linePartList[k].append(slopeStartNodeArr.linePartPtrArr[k]);//startTempNodeArr.linePartPtrArr[k]);
	m_pTower->m_hActiveModule=pModule->handle;
	for(i=0;i<pModuleDef->SegmentNum();i++)
	{
		AddShankSegment(pModuleDef->SegmentAtIndex(i),startTempNodeArr,slopePosInfo,slopeStartNodeArr,slopeEndNodeArr,nPoly,idLinePartClsType);
		for(int k=0;k<nPoly;k++)
			linePartList[k].append(startTempNodeArr.linePartPtrArr[k]);
	}
	for(i=0;i<pModuleDef->LegNum();i++)
	{
		SECTION_NODEARR legStartTempNodeArr=startTempNodeArr;
		//必须传入startTempNodeArr的拷贝，否则会导到呼高组下多接腿计算高度累加错误 wjh 2011-1-24
		CShankLeg *pLeg=pModuleDef->LegAtIndex(i);
		//TODO: 目前用的是最笨的方法获取腿整体配材序号，将来随着拓展每呼高超过8个腿时，这里再进行完善 wjh-2013.11.11
		CFGWORD legcfgword=pLeg->legword.ToCfgWord();
		for(int serial=1;serial<=192;serial++)
		{
			if(legcfgword.IsHasNo(serial))
				break;
		}
		if(serial>192)
			serial=1;
		pModule->m_arrActiveQuadLegNo[0]=pModule->m_arrActiveQuadLegNo[1]=	//设定当前接腿用于添加腿部标准面时生成独立节点和杆件
			pModule->m_arrActiveQuadLegNo[2]=pModule->m_arrActiveQuadLegNo[3]=serial;
		AddShankLeg(pLeg,legStartTempNodeArr,slopeStartNodeArr,slopeEndNodeArr,nPoly,idLinePartClsType);
		for(int k=0;k<nPoly;k++)
		{
			linePartList[k].append(legStartTempNodeArr.linePartPtrArr[k]);
			if(pLeg==slopePosInfo.pDatumLeg)	//将基准接腿各主材杆件记入坡段信息中，以便设定全部接腿主材的基础端的角钢位置
				slopePosInfo.legRodPtrArr[k]=legStartTempNodeArr.linePartPtrArr[k];
		}
	}
	//调整杆件始端节点段号 wht 10-11-16
	for(k=0;k<nPoly;k++)
	{
		for(CLDSLinePart *pLinePart=linePartList[k].GetFirst();pLinePart;pLinePart=linePartList[k].GetNext())
		{
			if(pLinePart->pStart)
				pLinePart->pStart->iSeg=pLinePart->iSeg;
		}
	}
#ifdef __SMART_DRAW_INC_//!defined(__TSA_)&&!defined(__TSA_FILE_)
	//最后一个坡段的杆件始终端位置在添加不同模型时进行调整	wht 10-06-06
	for(k=0;k<nPoly;k++)
		AdjustLinePartPosPara(linePartList[k],slopePosInfo);	//调整杆件始终端定位方式
#endif
}
void CShankDefineToTowerModel::AddShankSegment(CShankSegment *pSegment,SECTION_NODEARR& startNodeArr,SLOPE_POSINFO& slopePosInfo,
					 SECTION_NODEARR& slopeStartNodeArr,SECTION_NODEARR slopeEndNodeArr,int nPoly,int idLinePartClsType)
{
	int i;
	CLDSNodePtr nodeArr[8]={NULL},keyNodeArr[8]={NULL},keyNodeTopSectArr[8]={NULL},keyNodeBtmSectArr[8]={NULL};
	CSuperSmartPtr<CLDSLinePart> linePartArr[8];
	for(i=0;i<nPoly;i++)
	{	//生成分段节点以及杆件
		//TODO:未考虑当前段节点位置与前一呼高组中专有节点位置相重带来的少点问题
		nodeArr[i]=AddFixCoordZNode(slopeStartNodeArr.nodePtrArr[i],slopeEndNodeArr.nodePtrArr[i],startNodeArr.nodePtrArr[i]->Position(false).z+pSegment->m_fHeight,true,'S');
		linePartArr[i]=AddLinePart(startNodeArr.nodePtrArr[i],nodeArr[i],idLinePartClsType);
		memcpy(&linePartArr[i]->feature,&pSegment,4);
		nodeArr[i]->cfgword=linePartArr[i]->cfgword=pSegment->bodyword.ToCfgWord();
		linePartArr[i]->iSeg=pSegment->m_iSeg;
		nodeArr[i]->iSeg=linePartArr[i]->iSeg;
		//调整开始节点即上一杆件的终端节点段号与当前段段号一致 wht 10-11-16
		if(startNodeArr.nodePtrArr[i])
			startNodeArr.nodePtrArr[i]->iSeg=pSegment->m_iSeg;
		startNodeArr.nodePtrArr[i]=nodeArr[i];		//将新生成的节点添加到坡段开始节点下的节点数据
		startNodeArr.linePartPtrArr[i]=linePartArr[i];//将新生成的杆件添加到坡段开始节点下的杆件数据	
		if(slopeStartNodeArr.linePartPtrArr[i]==NULL)	//坡度段起始段杆件
			slopeStartNodeArr.linePartPtrArr[i]=linePartArr[i];
	}
	FillLinePartRelativeInfo(linePartArr,nPoly);//填充杆件关联信息
	FillNodeRelativeInfo(nodeArr,nPoly);		//填充节点关联信息
	for(i=0;i<pSegment->JunctionSegmentNum();i++)		//添加呼高分组
	{
		SECTION_NODEARR endNodeArr=startNodeArr;
		AddShankSegment(pSegment->JunctionSegmentAtIndex(i),endNodeArr,slopePosInfo,slopeStartNodeArr,slopeEndNodeArr,nPoly,idLinePartClsType);
#ifdef __SMART_DRAW_INC_//!defined(__TSA_)&&!defined(__TSA_FILE_)
		for(int k=0;k<nPoly;k++)
		{
			LINEPARTSET rodset;
			rodset.append(slopeStartNodeArr.linePartPtrArr[k]);
			rodset.append(endNodeArr.linePartPtrArr[k]);
			//TODO: 杆件始端点线投影时投影线终点基准杆件还有问题，暂无更好方法判断 wjh-2013.5.17
			AdjustLinePartPosPara(rodset,slopePosInfo);	//调整杆件始终端定位方式
		}
#endif
	}
	//调整杆件始端节点段号 wht 10-11-16
	for(i=0;i<nPoly;i++)
	{
		if(linePartArr[i]->pStart)
			linePartArr[i]->pStart->iSeg=linePartArr[i]->iSeg;
	}
	for(i=0;i<pSegment->ModuleNum();i++)		//添加呼高分组
		AddShankModule(pSegment->ModuleAtIndex(i),startNodeArr,slopePosInfo,slopeStartNodeArr,slopeEndNodeArr,nPoly,idLinePartClsType);

	//添加分段的垂高标注
	SKETCH_SIZEDIM *pVertDim=dims.append();
	pVertDim->cascade=1;	//第一层
	pVertDim->cfgword=linePartArr[0]->cfgword;
	pVertDim->dimStart=linePartArr[0]->pStart->Position(false);
	pVertDim->dimEnd=linePartArr[0]->pEnd->Position(false);
	pVertDim->cSizeType='Z';
	pVertDim->align_vec.Set(0,0,-1);
	pVertDim->pos.Set(0,0,(pVertDim->dimStart.z+pVertDim->dimEnd.z)/2);
	if(m_iViewType==1)	//右侧面视图
	{
		pVertDim->dimStart.x=pVertDim->dimEnd.x=0;
		pVertDim->norm.Set(1,0,0);
	}
	else
	{
		pVertDim->dimStart.y=pVertDim->dimEnd.y=0;
		pVertDim->norm.Set(0,1,0);
	}
	//插入标准面内的杆件节点
	CFacePanel facepanel;
	CXhChar100 lib_file;
	lib_file.Copy(theApp.execute_path);
	lib_file.Append("facepanel.pns");
	CInsertStdTemplateOperation operation(m_pTower);
	for(i=0;i<nPoly;i++)
		keyNodeTopSectArr[i]=linePartArr[i]->pStart;	//顶部横隔面关键点
	CExpression expression;
	double btm_z=linePartArr[0]->pStart->Position(false).z;
	//将节点和构件压栈以便获取新生成的节点和构件集合
	m_pTower->Node.GetTail();
	m_pTower->Parts.GetTail();
	m_pTower->Node.push_stack();
	m_pTower->Parts.push_stack();
	int nSection=pSegment->SectionNum();
	if(nSection==0&&(pSegment->divide_n>1||strlen(pSegment->front_layout)>0||strlen(pSegment->side_layout)>0||
		strlen(pSegment->hsect_layout)>0||strlen(pSegment->v_layout)>0))
		nSection=1;	//无节间时，如分段有相应属性，则视为有一节间
	for(i=0;i<nSection;i++)
	{
		double height=0;
		int divide_n=1;
		CXhChar50 front_layout,side_layout,hsect_layout,v_layout;
		if(pSegment->SectionNum()>0)
		{
			CShankSection* pSection=pSegment->SectionAtIndex(i);
			height=expression.SolveExpression(pSection->btm_zstr);
			if(height<=1)	//表示节间底部在当前分段的比例位置值
				btm_z =linePartArr[0]->pStart->Position(false).z+pSegment->m_fHeight*height;
			else
				btm_z+=height;
			divide_n=pSection->divide_n;
			front_layout.Copy(pSection->front_layout);
			side_layout.Copy(pSection->side_layout);
			hsect_layout.Copy(pSection->hsect_layout);
			v_layout.Copy(pSection->v_layout);
		}
		else
		{
			btm_z+=pSegment->m_fHeight;
			divide_n=pSegment->divide_n;
			front_layout.Copy(pSegment->front_layout);
			side_layout.Copy(pSegment->side_layout);
			hsect_layout.Copy(pSegment->hsect_layout);
			v_layout.Copy(pSegment->v_layout);
		}
		CFGWORD bodyword=pSegment->bodyword.ToCfgWord();
		if(nPoly==4)	//四边塔支持横隔面输入
		{
			facepanel.m_iFaceType=2;
			STDPANEL_INFO panelinfo("??G",GEPOINT(0,0,-1));
			if(hsect_layout.GetLength()>0&&facepanel.ReadFromLibFile(lib_file,hsect_layout))
				operation.CreateStdTemplate(&facepanel,keyNodeTopSectArr,0,0,&panelinfo,&bodyword);
			else if(hsect_layout.GetLength()>0)
#ifdef AFX_TARG_ENU_ENGLISH
				logerr.Log("Fail to read horizon isolation plane{%s}.",(char*)hsect_layout);
#else
				logerr.Log("读取横隔面{%s} 失败",(char*)hsect_layout);
#endif
		}
		//所有类型塔都支持塔身正侧面标准面输入，正侧面交替布置
		facepanel.m_iFaceType=1;	//默认塔身主体标准面类型
		for(int j=0;j<nPoly;j++)
		{
			int jn=(j+1)%nPoly;
			if(pSegment->SectionNum()>0)
			{	//存在节间时，需生成节间节点
				if(j==0)
					keyNodeBtmSectArr[0]=AddFixCoordZNode(slopeStartNodeArr.nodePtrArr[j],slopeEndNodeArr.nodePtrArr[j],btm_z);
				if(jn>0)
					keyNodeBtmSectArr[jn]=AddFixCoordZNode(slopeStartNodeArr.nodePtrArr[jn],slopeEndNodeArr.nodePtrArr[jn],btm_z);
			}
			else 
			{	//不存在节间时，直接使用该分段的底部节点
				if(j==0)
					keyNodeBtmSectArr[0]=startNodeArr.nodePtrArr[0];
				if(jn>0)
					keyNodeBtmSectArr[jn]=startNodeArr.nodePtrArr[jn];
			}
			keyNodeArr[0]=keyNodeTopSectArr[j];
			keyNodeArr[1]=keyNodeBtmSectArr[j];
			keyNodeArr[2]=keyNodeTopSectArr[jn];
			keyNodeArr[3]=keyNodeBtmSectArr[jn];
			CXhChar50 layout;
			char specLayer[4]="???",quad_layer[5]="QYHZ";
			if(j%2==0)
				layout.Copy(front_layout);
			else
				layout.Copy(side_layout);
			if(nPoly==4)
				specLayer[2]=quad_layer[j];
			else
				specLayer[2]='1'+j;
			if(facepanel.ReadFromLibFile(lib_file,layout))
			{
				STDPANEL_INFO panelinfo(specLayer,NULL,1);
				facepanel.m_nInternodeNum=divide_n;
				operation.CreateStdTemplate(&facepanel,keyNodeArr,0,0,&panelinfo,&bodyword);
			}
			//当只有节间而没有给塔身正侧面布材时，需给定节间节点的父杆件句柄
			else if(keyNodeBtmSectArr[jn]->hFatherPart<=0x20) 
				keyNodeBtmSectArr[jn]->hFatherPart = startNodeArr.linePartPtrArr[jn]->handle;
		}
		memcpy(keyNodeTopSectArr,keyNodeBtmSectArr,sizeof(CLDSNodePtr)*4);
	}
	m_pTower->Node.pop_stack();
	m_pTower->Parts.pop_stack();
	for(CLDSNode *pNode=m_pTower->Node.GetNext();pNode;pNode=m_pTower->Node.GetNext())
	{
		pNode->cfgword=pSegment->bodyword.ToCfgWord();
		pNode->iSeg=pSegment->m_iSeg;
	}
	for(CLDSPart *pPart=m_pTower->Parts.GetNext();pPart;pPart=m_pTower->Parts.GetNext())
	{
		pPart->cfgword=pSegment->bodyword.ToCfgWord();
		pPart->iSeg=pSegment->m_iSeg;
		memcpy(&pPart->feature,&pSegment,4);
	}
}

void CShankDefineToTowerModel::AddShankLeg(CShankLeg *pLeg,SECTION_NODEARR &startNodeArr,
				SECTION_NODEARR& slopeStartNodeArr,SECTION_NODEARR slopeEndNodeArr,int nPoly,int idLinePartClsType)
{
	int i;
	CLDSNodePtr nodeArr[8],keyNodeArr[8]={NULL},keyNodeTopSectArr[8]={NULL},keyNodeBtmSectArr[8]={NULL};
	CSuperSmartPtr<CLDSLinePart> linePartArr[8];
	for(i=0;i<nPoly;i++)
	{
		nodeArr[i]=AddFixCoordZNode(slopeStartNodeArr.nodePtrArr[i],slopeEndNodeArr.nodePtrArr[i],
			startNodeArr.nodePtrArr[i]->Position(false).z+pLeg->m_fHeight,pLeg->m_bDatumLeg);
		linePartArr[i]=AddLinePart(startNodeArr.nodePtrArr[i],nodeArr[i],idLinePartClsType);
		memcpy(&linePartArr[i]->feature,&pLeg,4);
		linePartArr[i]->layer(0)='L';
		nodeArr[i]->layer(0)=linePartArr[i]->layer(0)='L';
		nodeArr[i]->cfgword=linePartArr[i]->cfgword=pLeg->legword.ToCfgWord();
		linePartArr[i]->iSeg=pLeg->m_iSeg;
		nodeArr[i]->iSeg=linePartArr[i]->iSeg;
		startNodeArr.nodePtrArr[i]=nodeArr[i];	//将新生成的节点添加到坡段开始节点下的节点数据
		startNodeArr.linePartPtrArr[i]=linePartArr[i];//将新生成的杆件添加到坡段开始节点下的杆件数据
		if(slopeStartNodeArr.linePartPtrArr[i]==NULL)	//坡度段起始段杆件
			slopeStartNodeArr.linePartPtrArr[i]=linePartArr[i];
	}
	FillLinePartRelativeInfo(linePartArr,nPoly);
	FillNodeRelativeInfo(nodeArr,nPoly);
	//添加接腿根开标注
	SKETCH_SIZEDIM *pFrontDim=dims.append();
	pFrontDim->cfgword=linePartArr[0]->cfgword;
	pFrontDim->bLegDim=TRUE;
	pFrontDim->dimStart=linePartArr[0]->pEnd->Position(false);
	pFrontDim->dimEnd=linePartArr[1]->pEnd->Position(false);
	pFrontDim->cSizeType='X';
	pFrontDim->norm.Set(0,1,0);
	pFrontDim->align_vec.Set(1,0,0);
	pFrontDim->pos.Set(0,linePartArr[1]->pEnd->Position(false).y,linePartArr[1]->pEnd->Position(false).z+500);
	SKETCH_SIZEDIM *pSideDim=dims.append();
	pSideDim->cfgword=linePartArr[1]->cfgword;
	pSideDim->bLegDim=TRUE;
	pSideDim->dimStart=linePartArr[1]->pEnd->Position(false);
	pSideDim->dimEnd=linePartArr[2]->pEnd->Position(false);
	pSideDim->cSizeType='Y';
	pSideDim->norm.Set(1,0,0);
	pSideDim->align_vec.Set(0,-1,0);
	pSideDim->pos.Set(linePartArr[1]->pEnd->Position(false).x,0,linePartArr[1]->pEnd->Position(false).z+500);
	//添加接腿的垂高标注
	SKETCH_SIZEDIM *pVertDim=dims.append();
	pVertDim->cascade=1;	//第一层
	pVertDim->bLegDim=TRUE;	//接腿
	pVertDim->cfgword=linePartArr[0]->cfgword;
	pVertDim->dimStart=linePartArr[0]->pStart->Position(false);
	pVertDim->dimEnd=linePartArr[0]->pEnd->Position(false);
	pVertDim->cSizeType='Z';
	pVertDim->align_vec.Set(0,0,-1);
	pVertDim->pos.Set(0,0,(pVertDim->dimStart.z+pVertDim->dimEnd.z)/2);
	if(m_iViewType==1)	//右侧面视图
	{
		pVertDim->dimStart.x=pVertDim->dimEnd.x=0;
		pVertDim->norm.Set(1,0,0);
	}
	else
	{
		pVertDim->dimStart.y=pVertDim->dimEnd.y=0;
		pVertDim->norm.Set(0,1,0);
	}
	//塔腿所在坡段垂高标注
	SKETCH_SIZEDIM *pSlopeDim=dims.append();
	pSlopeDim->cascade=2;	//第二层
	pSlopeDim->bLegDim=TRUE;	//接腿
	pSlopeDim->cfgword=linePartArr[0]->cfgword;
	pSlopeDim->dimStart=slopeStartNodeArr.nodePtrArr[0]->Position(false);
	pSlopeDim->dimEnd=linePartArr[0]->pEnd->Position(false);
	pSlopeDim->cSizeType='Z';
	pSlopeDim->align_vec.Set(0,0,-1);
	pSlopeDim->pos.Set(0,0,(pSlopeDim->dimStart.z+pSlopeDim->dimEnd.z)/2);
	if(m_iViewType==1)	//右侧面视图
	{
		pSlopeDim->dimStart.x=pSlopeDim->dimEnd.x=0;
		pSlopeDim->norm.Set(1,0,0);
	}
	else
	{
		pSlopeDim->dimStart.y=pSlopeDim->dimEnd.y=0;
		pSlopeDim->norm.Set(0,1,0);
	}
	//插入标准面内的杆件节点
	CFacePanel facepanel;
	CXhChar100 lib_file;
	lib_file.Copy(theApp.execute_path);
	lib_file.Append("facepanel.pns");
	CInsertStdTemplateOperation operation(m_pTower);
	for(i=0;i<nPoly;i++)
	{
		keyNodeTopSectArr[i]=linePartArr[i]->pStart;//顶部横隔面关键点
		keyNodeBtmSectArr[i]=linePartArr[i]->pEnd;	//底部横隔面关键点
	}
	//将节点和构件压栈以便获取新生成的节点和构件集合
	m_pTower->Node.GetTail();
	m_pTower->Parts.GetTail();
	m_pTower->Node.push_stack();
	m_pTower->Parts.push_stack();

	CXhChar50 front_layout,side_layout,hsect_layout,v_layout;
	front_layout.Copy(pLeg->front_layout);
	side_layout.Copy(pLeg->side_layout);
	hsect_layout.Copy(pLeg->hsect_layout);
	v_layout.Copy(pLeg->v_layout);
	CFGWORD legword=pLeg->legword.ToCfgWord();
	CFGWORD bodyword=legword.ToBodyWord();
	if(nPoly==4)	//四边塔支持横隔面输入(属于塔身)
	{
		facepanel.m_iFaceType=2;
		STDPANEL_INFO panelinfo("S?G",GEPOINT(0,0,-1));
		if(facepanel.ReadFromLibFile(lib_file,hsect_layout))
			operation.CreateStdTemplate(&facepanel,keyNodeTopSectArr,0,0,&panelinfo,&bodyword,&legword);
		m_pTower->Node.pop_stack();
		m_pTower->Parts.pop_stack();
		for(CLDSNode *pNode=m_pTower->Node.GetNext();pNode;pNode=m_pTower->Node.GetNext())
		{
			pNode->cfgword=pLeg->legword.ToCfgWord();//linePartArr[0]->pStart->cfgword;
			pNode->iSeg=linePartArr[0]->pStart->iSeg;
			pNode->layer(0)='S';
		}
		for(CLDSPart *pPart=m_pTower->Parts.GetNext();pPart;pPart=m_pTower->Parts.GetNext())
		{
			pPart->cfgword=pLeg->legword.ToCfgWord();//linePartArr[0]->pStart->cfgword;
			pPart->iSeg=linePartArr[0]->pStart->iSeg;
			pPart->layer(0)='S';
			memcpy(&pPart->feature,&pLeg,4);
		}
		m_pTower->Node.GetTail();
		m_pTower->Parts.GetTail();
		m_pTower->Node.push_stack();
		m_pTower->Parts.push_stack();
	}
	int j;
	//所有类型塔都支持塔身正侧面标准面输入，正侧面交替布置
	facepanel.m_iFaceType=6;	//倒L型接腿正侧面
	for(j=0;j<nPoly;j++)
	{
		int jn=(j+1)%nPoly;
		int jnn=(j+2)%nPoly;
		keyNodeArr[0]=keyNodeTopSectArr[j];
		keyNodeArr[1]=keyNodeBtmSectArr[j];
		keyNodeArr[2]=keyNodeTopSectArr[jn];
		keyNodeArr[3]=keyNodeBtmSectArr[jn];

		CXhChar50 layout;
		char specLayer[4]="???",quad_layer[5]="QYHZ";
		if(j%2==0)
			layout.Copy(front_layout);
		else
			layout.Copy(side_layout);
		if(nPoly==4)
			specLayer[2]=quad_layer[j];
		else
			specLayer[2]='1'+j;
		if(facepanel.ReadFromLibFile(lib_file,layout))
		{
			STDPANEL_INFO panelinfo(specLayer,NULL,1);
			facepanel.m_nInternodeNum=pLeg->divide_n;
			keyNodeArr[3]=NULL;
			CLDSNode* pTopHoriMidNode=AddDivScaleNode(keyNodeArr[0],keyNodeArr[2],0.5);
			keyNodeArr[2]=pTopHoriMidNode;
			operation.CreateStdTemplate(&facepanel,keyNodeArr,0,0,&panelinfo,&bodyword,&legword);
			keyNodeArr[0]=keyNodeTopSectArr[jn];
			keyNodeArr[1]=keyNodeBtmSectArr[jn];
			operation.CreateStdTemplate(&facepanel,keyNodeArr,0,0,&panelinfo,&bodyword,&legword);
		}
	}
	//所有类型塔都支持V型标准面输入
	facepanel.m_iFaceType=5;	//V型标准面
	for(j=0;j<nPoly;j++)
	{
		int jn=(j+1)%nPoly;
		int jnn=(j+2)%nPoly;
		keyNodeArr[0]=keyNodeTopSectArr[j];
		keyNodeArr[1]=keyNodeBtmSectArr[j];
		keyNodeArr[2]=keyNodeTopSectArr[jn];
		keyNodeArr[3]=keyNodeBtmSectArr[jn];
		if(facepanel.ReadFromLibFile(lib_file,v_layout))
		{	//设定有V型标准面
			keyNodeArr[0]=AddDivScaleNode(keyNodeTopSectArr[jn],keyNodeTopSectArr[jnn],0.5);
			keyNodeArr[1]=AddDivScaleNode(keyNodeTopSectArr[j],keyNodeTopSectArr[jn],0.5);
			keyNodeArr[2]=keyNodeBtmSectArr[jn];
			keyNodeArr[3]=NULL;
			facepanel.m_nInternodeNum=1;
			STDPANEL_INFO panelinfo("L?G",GEPOINT(0,0,1));
			facepanel.m_nInternodeNum=pLeg->divide_n;	//增加对接腿模板节间数支持
			operation.CreateStdTemplate(&facepanel,keyNodeArr,0,0,&panelinfo,NULL,&legword);
		}
	}

	m_pTower->Node.pop_stack();
	m_pTower->Parts.pop_stack();
	for(CLDSNode *pNode=m_pTower->Node.GetNext();pNode;pNode=m_pTower->Node.GetNext())
	{
		pNode->cfgword=pLeg->legword.ToCfgWord();
		pNode->iSeg=pLeg->m_iSeg;
		//pNode->layer(0)='L';
	}
	for(CLDSPart *pPart=m_pTower->Parts.GetNext();pPart;pPart=m_pTower->Parts.GetNext())
	{
		pPart->cfgword=pLeg->legword.ToCfgWord();
		pPart->iSeg=pLeg->m_iSeg;
		//pPart->layer(0)='L';
		memcpy(&pPart->feature,&pLeg,4);
	}
}
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//layer_first_char!=0时需要判断原坡度端节点的图层名是否与指定字符一致，以防基准腿基础节点与塔身节点共用，导致配材号混乱 wjh-2013.6.25
CLDSNode* CShankDefineToTowerModel::AddFixCoordZNode(CLDSNode *pAttachStartNode,CLDSNode *pAttachEndNode,
													 double coordZ,BOOL bPriorFromSlopeNode/*=true*/,char layer_first_char/*=0*/)
{
	if(bPriorFromSlopeNode&&coordZ==pAttachStartNode->Position(false).z&&(layer_first_char==0||layer_first_char==pAttachStartNode->layer(0)))
		return pAttachStartNode;
	else if(bPriorFromSlopeNode&&coordZ==pAttachEndNode->Position(false).z&&(layer_first_char==0||layer_first_char==pAttachEndNode->layer(0)))
		return pAttachEndNode;
	CLDSNode *pNode=NULL;
	/*CLDSNode tempnode;
	tempnode.SetBelongModel(m_pTower);
	tempnode.SetPositionZ(coordZ);
	tempnode.m_cPosCalType=3;
	tempnode.arrRelationNode[0]=pAttachStartNode->handle;
	tempnode.arrRelationNode[1]=pAttachEndNode->handle;
	tempnode.Set();
	f3dPoint pos=tempnode.Position(false);
	if(m_pTower->m_hActiveModule>0)
		pNode=m_pTower->CheckOverlappedNode(pos,m_pTower->GetActiveModule());*/
	if(pNode==NULL)
	{
		pNode=m_pTower->Node.append();
		pNode->SetPositionZ(coordZ);
		pNode->m_cPosCalType=3;	//Z坐标不变点
		pNode->arrRelationNode[0]=pAttachStartNode->handle;
		pNode->arrRelationNode[1]=pAttachEndNode->handle;
		pNode->Set();
		pNode->SetLayer(pAttachStartNode->layer());
	}
	return pNode;
}
CLDSNode* CShankDefineToTowerModel::AddDivScaleNode(CLDSNode *pAttachStartNode,CLDSNode *pAttachEndNode,double scale)
{
	if(scale==0)
		return pAttachStartNode;
	else if(scale==1)
		return pAttachEndNode;
	f3dPoint vec=pAttachEndNode->Position(false)-pAttachStartNode->Position(false);
	f3dPoint pos=pAttachStartNode->Position(false)+vec*scale;
	CLDSNode* pNode=m_pTower->CheckOverlappedNode(pos);
	if(pNode==NULL)
	{
		pNode=m_pTower->Node.append();
		pNode->SetPosition(pos);
		pNode->m_cPosCalType=5;	//比例等分点
		pNode->attach_scale=scale;
		pNode->arrRelationNode[0]=pAttachStartNode->handle;
		pNode->arrRelationNode[1]=pAttachEndNode->handle;
		pNode->Set();
		pNode->SetLayer(pAttachStartNode->layer());
	}
	return pNode;
}
CLDSLinePart* CShankDefineToTowerModel::AddLinePart(CLDSNode *pStart,CLDSNode *pEnd,int idLinePartClsType)
{
	CLDSLinePart *pLinePart;
#ifdef __TSA_
	if(theApp.m_bAnalysisWorkEnv)
	{
		pLinePart = (CLDSLinePart*)m_pTower->Parts.append(CLS_LINEPART);
		pLinePart->size_idClassType=idLinePartClsType;
	}
	else
#endif
		pLinePart = (CLDSLinePart*)m_pTower->Parts.append(idLinePartClsType);
#ifdef __SMART_DRAW_INC_//defined(__LMA_)||defined(__LDS_)
	if(pLinePart->GetClassTypeId()==CLS_LINETUBE)	//钢管
	{	//设置钢管规格为140X4
		pLinePart->size_wide=140;
		pLinePart->size_thick=4;
		((CLDSLineTube*)pLinePart)->m_bHasWeldRoad=FALSE;
	}
#endif
	pLinePart->pStart=pStart;
	pLinePart->pEnd=pEnd;
	pLinePart->pStart->hFatherPart=pLinePart->handle;
	pLinePart->pEnd->hFatherPart=pLinePart->handle;
	pLinePart->SetStart(pStart->Position(false));
	pLinePart->SetEnd(pEnd->Position(false));
	pLinePart->SetLayer("SZ1");
	pLinePart->layer(2)=pStart->layer(2);
	return pLinePart;
}

bool CShankDefineToTowerModel::UpdateTowerModel(CTower* pTower,CShankDefine *pDefine,int nPoly,long idClassType)
{
	m_pDefine=pDefine;
	m_pTower=pTower;
	m_pTower->Empty();
	m_pTower->InitTower();
	m_pTower->InitModule();
	if(pDefine->SlopeNum()<1)
		return false;
	pDefine->AllocateCfgWord();
	dims.Empty();
	SECTION_NODEARR slopeStartNodeArr;
	memset(&slopeStartNodeArr,0,sizeof(SECTION_NODEARR));
	for(int i=0; i<=pDefine->SlopeNum();i++)
	{
		if(!AddSlopeToTowerModel(slopeStartNodeArr,nPoly,i,pDefine->SlopeNum(),idClassType))
			return false;
	}
	//临时合并重复节点代码，以后应从根本源头避免出现重复节点
	for(CLDSNode* pNode=pTower->Node.GetFirst();pNode;pNode=pTower->Node.GetNext())
	{
		BOOL pushed=pTower->Node.push_stack();
		//if(theApp.m_bCooperativeWork&&!theApp.IsHasModifyPerm(pNode->dwPermission))
		//	continue;	//网络协同时，对此节点没有拥有修改权限
		for(CLDSNode *pOthNode=pTower->Node.GetNext();pOthNode;pOthNode=pTower->Node.GetNext())
		{
			if(!pNode->cfgword.And(pOthNode->cfgword)||pNode->Position(false)!=pOthNode->Position(false))	//至少会在一个模型中同时出现
				continue;
			if(pOthNode->hFatherPart!=0&&pNode->hFatherPart==0)
			{	//保留节点如缺失父杆件时，应借用待删除节点的父杆件 wjh-2013.6.6
				pNode->hFatherPart=pOthNode->hFatherPart;
				pNode->SetModified();
			}
			BOOL pushed1=pTower->Node.push_stack();
			for(CLDSNode *pTmNode=pTower->Node.GetFirst();pTmNode;pTmNode=pTower->Node.GetNext())
			{
				BOOL pushed2=pTower->Node.push_stack();
				if(pTmNode->handle!=pOthNode->handle)
					pTmNode->ReplaceReferenceHandle(CHandleReplacer(pOthNode->handle,pNode->handle),CLS_NODE);
				pTower->Node.pop_stack(pushed2);
			}
			pTower->Node.pop_stack(pushed1);
			for(CLDSPart* pPart=pTower->Parts.GetFirst();pPart;pPart=pTower->Parts.GetNext())
			{
				BOOL pushed =pTower->Parts.push_stack();
				pPart->ReplaceReferenceHandle(CHandleReplacer(pOthNode->handle,pNode->handle),CLS_NODE);
				pTower->Parts.pop_stack(pushed);
			}
			pTower->DeleteNode(pOthNode->handle);
		}
		pTower->Node.pop_stack(pushed);
	}

	/*m_pTower->InitEvolveCfgwordMap(1,4);	//肯定需要演进配材号，所以版本号暂赋为最小的1
	for(CLDSNode* pNode=m_pTower->Node.GetFirst();pNode;pNode=m_pTower->Node.GetNext())
		m_pTower->EvolveCfgword(pNode->cfgword,1,4);
	for(CLDSPart* pPart=m_pTower->Parts.GetFirst();pPart;pPart=m_pTower->Parts.GetNext())
		m_pTower->EvolveCfgword(pPart->cfgword,1,4);*/
	return true;
}

bool CShankDefineToTowerModel::AddSlopeToTowerModel(SECTION_NODEARR &slopeStartNodeArr,int nPoly,int iSlope,int nSlopeNum,int idClassType)
{
	double fX=0,fY=0,fZ=0;
	CLDSNode *pVertexNodeArr[8]={NULL};
	fX = m_pDefine->SlopeTopFrontWidth(iSlope)*0.5;
	fZ = m_pDefine->SlopeTopCoordZ(iSlope);
	if(3==nPoly)
	{
		double fSideWidth=m_pDefine->SlopeTopSideWidth(iSlope);
		if(iSlope!=0)
			fSideWidth=fSideWidth*m_pDefine->SlopeTopSideWidth(0)/m_pDefine->SlopeTopFrontWidth(0);
		fY=sqrt(fSideWidth*fSideWidth-fX*fX)/3;
		//添加结点并设置结点坐标以及图层名
		pVertexNodeArr[0] = m_pTower->Node.append();	//添加结点
		pVertexNodeArr[0]->SetLayer("SP1");		//设置图层名
		pVertexNodeArr[0]->SetPosition(-fX, fY, fZ);
		pVertexNodeArr[1] = m_pTower->Node.append();
		pVertexNodeArr[1]->SetLayer("SP2");
		pVertexNodeArr[1] ->SetPosition(fX, fY, fZ);
		pVertexNodeArr[2] = m_pTower->Node.append();
		pVertexNodeArr[2]->SetLayer("SP3");
		pVertexNodeArr[2] ->SetPosition(0, -2*fY, fZ);
	}
	else if(4==nPoly) 
	{
		fY = m_pDefine->SlopeTopSideWidth(iSlope)*0.5;
		//添加结点并设置结点坐标以及图层名
		pVertexNodeArr[0] = m_pTower->Node.append();
		pVertexNodeArr[0]->SetLayer("SP2");
		pVertexNodeArr[0]->SetPosition(-fX,fY,fZ);//dlg.OpenList[i][0]);
		pVertexNodeArr[1] = m_pTower->Node.append();
		pVertexNodeArr[1]->SetLayer("SP1");
		pVertexNodeArr[1]->SetPosition(fX,fY,fZ);
		pVertexNodeArr[2] = m_pTower->Node.append();
		pVertexNodeArr[2]->SetLayer("SP3");
		pVertexNodeArr[2]->SetPosition(fX,-fY,fZ);
		pVertexNodeArr[3] = m_pTower->Node.append();
		pVertexNodeArr[3]->SetLayer("SP4");
		pVertexNodeArr[3]->SetPosition(-fX,-fY,fZ);
		
		if(iSlope<m_pDefine->SlopeNum())
		{//添加开口标注
			SKETCH_SIZEDIM *pFrontDim=dims.append();
			pFrontDim->cfgword=m_pDefine->SlopeTopCfgword(iSlope);
			pFrontDim->dimStart=pVertexNodeArr[0]->Position(false);
			pFrontDim->dimEnd=pVertexNodeArr[1]->Position(false);
			pFrontDim->cSizeType='X';
			pFrontDim->norm.Set(0,1,0);
			pFrontDim->align_vec.Set(1,0,0);
			pFrontDim->pos.Set(0,fY,fZ-500);
			pFrontDim->wAlignStyle=CDrawEngine::BottomCenter;
			SKETCH_SIZEDIM *pSideDim=dims.append();
			pSideDim->cfgword=m_pDefine->SlopeTopCfgword(iSlope);
			pSideDim->dimStart=pVertexNodeArr[1]->Position(false);
			pSideDim->dimEnd=pVertexNodeArr[2]->Position(false);
			pSideDim->cSizeType='Y';
			pSideDim->norm.Set(1,0,0);
			pSideDim->align_vec.Set(0,-1,0);
			pSideDim->pos.Set(fX,0,fZ-500);
			pSideDim->wAlignStyle=CDrawEngine::BottomCenter;
		}
	}
	else if(5==nPoly)
	{
		double cos54=0.58778525229247312916870595463907;
		double tan54=1.3763819204711735382072095819109;
		double sin72=0.95105651629515357211643933337938;
		double cos72=0.30901699437494742410229341718282;
		double fR=0,fR1=0,fY1=0;
		fR = (fX)/cos54;
		fR1 = (fX)*tan54;
		fY = 2*fX*sin72;
		fY1 = 2*fY*cos72;
		//添加结点并设置结点坐标以及图层名
		pVertexNodeArr[0] = m_pTower->Node.append();
		pVertexNodeArr[0]->SetLayer("SP1");
		pVertexNodeArr[0]->SetPosition(-fX,fR1,fZ);
		pVertexNodeArr[1] = m_pTower->Node.append();
		pVertexNodeArr[1]->SetLayer("SP2");
		pVertexNodeArr[1]->SetPosition(fX,fR1,fZ);
		pVertexNodeArr[2] = m_pTower->Node.append();
		pVertexNodeArr[2]->SetLayer("SP3");
		pVertexNodeArr[2]->SetPosition(fX+fY1,fR1-fY,fZ);
		pVertexNodeArr[3] = m_pTower->Node.append();
		pVertexNodeArr[3]->SetLayer("SP4");
		pVertexNodeArr[3]->SetPosition(0,-fR,fZ);
		pVertexNodeArr[4] = m_pTower->Node.append();
		pVertexNodeArr[4]->SetLayer("SP5");
		pVertexNodeArr[4]->SetPosition(-fX-fY1,fR1-fY,fZ);
	}
	else if(6==nPoly)
	{
		fY = sqrt(3.0)*fX;
		//添加结点并设置结点坐标以及图层名
		pVertexNodeArr[0] = m_pTower->Node.append();
		pVertexNodeArr[0]->SetLayer("SP1");
		pVertexNodeArr[0]->SetPosition(-2*fX,0,fZ);
		pVertexNodeArr[1] = m_pTower->Node.append();
		pVertexNodeArr[1]->SetLayer("SP2");
		pVertexNodeArr[1]->SetPosition(-fX,fY,fZ);
		pVertexNodeArr[2] = m_pTower->Node.append();
		pVertexNodeArr[2]->SetLayer("SP3");
		pVertexNodeArr[2]->SetPosition(fX,fY,fZ);
		pVertexNodeArr[3] = m_pTower->Node.append();
		pVertexNodeArr[3]->SetLayer("SP4");
		pVertexNodeArr[3]->SetPosition(2*fX,0,fZ);
		pVertexNodeArr[4] = m_pTower->Node.append();
		pVertexNodeArr[4]->SetLayer("SP5");
		pVertexNodeArr[4]->SetPosition(fX,-fY,fZ);
		pVertexNodeArr[5] = m_pTower->Node.append();
		pVertexNodeArr[5]->SetLayer("SP6");
		pVertexNodeArr[5]->SetPosition(-fX,-fY,fZ);
	}
	else if(8==nPoly)
	{
		fY = (1.0/sqrt(2.0))*fX*2;
		//添加结点并设置结点坐标以及图层名
		pVertexNodeArr[0] = m_pTower->Node.append();
		pVertexNodeArr[0]->SetLayer("SP1");
		pVertexNodeArr[0]->SetPosition(-fX-fY,fX,fZ);
		pVertexNodeArr[1] = m_pTower->Node.append();
		pVertexNodeArr[1]->SetLayer("SP2");
		pVertexNodeArr[1]->SetPosition(-fX,fX+fY,fZ);
		pVertexNodeArr[2] = m_pTower->Node.append();
		pVertexNodeArr[2]->SetLayer("SP3");
		pVertexNodeArr[2]->SetPosition(fX,fX+fY,fZ);
		pVertexNodeArr[3] = m_pTower->Node.append();
		pVertexNodeArr[3]->SetLayer("SP4");
		pVertexNodeArr[3]->SetPosition(fX+fY,fX,fZ);
		pVertexNodeArr[4] = m_pTower->Node.append();
		pVertexNodeArr[4]->SetLayer("SP5");
		pVertexNodeArr[4]->SetPosition(fX+fY,-fX,fZ);
		pVertexNodeArr[5] = m_pTower->Node.append();
		pVertexNodeArr[5]->SetLayer("SP6");
		pVertexNodeArr[5]->SetPosition(fX,-fX-fY,fZ);
		pVertexNodeArr[6] = m_pTower->Node.append();
		pVertexNodeArr[6]->SetLayer("SP7");
		pVertexNodeArr[6]->SetPosition(-fX,-fX-fY,fZ);
		pVertexNodeArr[7] = m_pTower->Node.append();
		pVertexNodeArr[7]->SetLayer("SP8");
		pVertexNodeArr[7]->SetPosition(-fX-fY,-fX,fZ);
	}
	SECTION_NODEARR slopeEndNodeArr;
	for(int j=0; j<nPoly; j++)
	{	//显示结点
		pVertexNodeArr[j]->cfgword=m_pDefine->SlopeTopCfgword(iSlope);
		pVertexNodeArr[j]->iSeg=m_pDefine->SlopeTopSegI(iSlope); //坡段顶部节点编号 wht 10-11-16
		//NewNode(pVertexNodeArr[j]);
		//填充对称关联信息
		for(int k=0; k<nPoly; k++)
		{
			if(j==k)	//跳过节点本身
				continue;
			if(nPoly==3||nPoly==5||nPoly==6||nPoly==8)
			{	//对称关系
				RELATIVE_OBJECT *pMirObj=pVertexNodeArr[j]->relativeObjs.Add(pVertexNodeArr[k]->handle);
				pMirObj->mirInfo.axis_flag=8;
				pMirObj->mirInfo.rotate_angle=-(k-j)*360/nPoly;
			}
			else if(nPoly==4)
			{
				RELATIVE_OBJECT *pMirObj=pVertexNodeArr[j]->relativeObjs.Add(pVertexNodeArr[k]->handle);
				if(pVertexNodeArr[j]->Position(false).x==pVertexNodeArr[k]->Position(false).x&&
					pVertexNodeArr[j]->Position(false).y==-pVertexNodeArr[k]->Position(false).y)
					pMirObj->mirInfo=MIRMSG(1);	//X轴对称
				else if(pVertexNodeArr[j]->Position(false).x==-pVertexNodeArr[k]->Position(false).x&&
					pVertexNodeArr[j]->Position(false).y==pVertexNodeArr[k]->Position(false).y)
					pMirObj->mirInfo=MIRMSG(2);	//Y轴对称
				else if(pVertexNodeArr[j]->Position(false).x==-pVertexNodeArr[k]->Position(false).x&&
					pVertexNodeArr[j]->Position(false).y==-pVertexNodeArr[k]->Position(false).y)
				pMirObj->mirInfo=MIRMSG(3);	//Z轴对称
			}
		}
		if(slopeStartNodeArr.nodePtrArr[j]==NULL)
		{
			slopeStartNodeArr.nodePtrArr[j]=pVertexNodeArr[j];
			continue;
		}
		else
			slopeEndNodeArr.nodePtrArr[j]=pVertexNodeArr[j];
	}
	if(iSlope>0)
	{
		SLOPE_POSINFO slopePosInfo;
		slopePosInfo.pDatumLeg=NULL;
		memset(slopePosInfo.legRodPtrArr,0,sizeof(slopePosInfo.legRodPtrArr));
		slopePosInfo.bTopSlope=(iSlope==1);
		slopePosInfo.bBottomSlope=(iSlope==nSlopeNum);
		slopePosInfo.coordTopZ   = m_pDefine->SlopeTopCoordZ(iSlope);
		slopePosInfo.coordBottomZ= m_pDefine->SlopeBtmCoordZ(iSlope);
		SECTION_NODEARR startNodeArr=slopeStartNodeArr;
		LINEPARTSET linePartList[8];
		memset(slopeStartNodeArr.linePartPtrArr,0,sizeof(slopeStartNodeArr.linePartPtrArr));
		CShankSlope *pShankSlope=m_pDefine->SlopeAtIndex(iSlope-1);
		CShankLeg* pDatumLeg=pShankSlope->GetDatumLeg();
		if(pDatumLeg)	//不在此赋值将导致基准腿的基础节点与坡段底部节点重复 wjh-2013.6.5
			pDatumLeg->m_bDatumLeg=TRUE;
		if(slopePosInfo.bBottomSlope)
		{
			slopePosInfo.pDatumLeg=pShankSlope->GetDatumLeg();
			if(slopePosInfo.pDatumLeg==NULL)
				return false;
		}
		if(pShankSlope->m_cMainRodType=='T')
			idClassType=CLS_LINETUBE;
		else if(pShankSlope->m_cMainRodType=='L')
			idClassType=CLS_LINEANGLE;
		for(int k=0;k<pShankSlope->ModuleNum();k++)		//添加直属于坡段数据的呼高分组数据 wht 10-07-24
			AddShankModule(pShankSlope->ModuleAtIndex(k),startNodeArr,slopePosInfo,slopeStartNodeArr,slopeEndNodeArr,nPoly,idClassType);
		for(j=0;j<pShankSlope->SegmentNum();j++)
		{
			CShankSegment *pSegment=pShankSlope->SegmentAtIndex(j);
			AddShankSegment(pSegment,startNodeArr,slopePosInfo,slopeStartNodeArr,slopeEndNodeArr,nPoly,idClassType);
			for(int k=0;k<nPoly;k++)
				linePartList[k].append(startNodeArr.linePartPtrArr[k]);
		}
#ifdef __SMART_DRAW_INC_//!defined(__TSA_)&&!defined(__TSA_FILE_)
		for(k=0;k<nPoly;k++)
			AdjustLinePartPosPara(linePartList[k],slopePosInfo);	//调整杆件始终端定位方式
		if(slopePosInfo.pDatumLeg!=NULL)
		{	//调整多呼高多接腿时各接腿主材基础端的位置设计参数
			CLDSLineAngle *pLegAngle;
			for(pLegAngle=(CLDSLineAngle*)m_pTower->Parts.GetFirst(CLS_LINEANGLE);pLegAngle;pLegAngle=(CLDSLineAngle*)m_pTower->Parts.GetNext(CLS_LINEANGLE))
			{
				if(pLegAngle->layer(0)=='L'&&pLegAngle->layer(1)=='Z'&&isdigit(pLegAngle->layer(2)))
				{
					char cQuad=pLegAngle->layer(2);
					if(nPoly==4&&cQuad=='1')
						cQuad='2';
					else if(nPoly==4&&cQuad=='2')
						cQuad='1';
					if(cQuad=='0'||cQuad=='9')	//不支持也不可能出现
						continue;
					if(pLegAngle==slopePosInfo.legRodPtrArr[cQuad-'1'])
					{
						pLegAngle->pEnd->hFatherPart=pLegAngle->handle;
						//pLegAngle->desEndPos.datum_pos_style=0;
						//pLegAngle->desEndPos.datum_to_ber_style=1;
						pLegAngle->desEndPos.spatialOperationStyle=19;
						pLegAngle->desEndPos.wing_x_offset.gStyle=0;
						pLegAngle->desEndPos.wing_y_offset.gStyle=0;
						if(nPoly==4&&(pLegAngle->layer(2)=='2'||pLegAngle->layer(2)=='3'))
							pLegAngle->desEndPos.cFaceOffsetDatumLine='Y';
						else
							pLegAngle->desEndPos.cFaceOffsetDatumLine='X';
						continue;	//基准接腿主材不需要调整
					}
					else
					{
						pLegAngle->desEndPos.spatialOperationStyle=1;
						pLegAngle->desEndPos.datum_jg_h=slopePosInfo.legRodPtrArr[cQuad-'1']->handle;
						pLegAngle->desEndPos.wing_x_offset.gStyle=4;
						pLegAngle->desEndPos.wing_y_offset.gStyle=4;
						pLegAngle->desEndPos.wing_x_offset.offsetDist=0;
						pLegAngle->desEndPos.wing_y_offset.offsetDist=0;
						if(nPoly==4&&(pLegAngle->layer(2)=='2'||pLegAngle->layer(2)=='3'))
							pLegAngle->desEndPos.cFaceOffsetDatumLine='Y';
						else
							pLegAngle->desEndPos.cFaceOffsetDatumLine='X';
					}
				}
			}
		}
#endif
		if(iSlope<nSlopeNum)
		{	//当前坡段垂高标注
			SKETCH_SIZEDIM *pSlopeDim=dims.append();
			pSlopeDim->cascade=2;	//第二层
			pSlopeDim->bLegDim=FALSE;	//接腿
			pSlopeDim->cfgword=slopeStartNodeArr.nodePtrArr[0]->cfgword;
			pSlopeDim->dimStart=slopeStartNodeArr.nodePtrArr[0]->Position(false);
			pSlopeDim->dimEnd=slopeEndNodeArr.nodePtrArr[0]->Position(false);
			pSlopeDim->cSizeType='Z';
			pSlopeDim->align_vec.Set(0,0,-1);
			pSlopeDim->pos.Set(0,0,(pSlopeDim->dimStart.z+pSlopeDim->dimEnd.z)/2);
			if(m_iViewType==1)	//右侧面视图
			{
				pSlopeDim->dimStart.x=pSlopeDim->dimEnd.x=0;
				pSlopeDim->norm.Set(1,0,0);
			}
			else
			{
				pSlopeDim->dimStart.y=pSlopeDim->dimEnd.y=0;
				pSlopeDim->norm.Set(0,1,0);
			}
		}
		slopeStartNodeArr=slopeEndNodeArr;
	}
	return true;
}
#endif