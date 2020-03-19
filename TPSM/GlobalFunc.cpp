#include "stdafx.h"
#include "GlobalFunc.h"
#include <direct.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

char* SearchChar(char* srcStr,char ch,bool reverseOrder/*=false*/)
{
	if(!reverseOrder)
		return strchr(srcStr,ch);
	else
	{
		int len=strlen(srcStr);
		for(int i=len-1;i>=0;i--)
		{
			if(srcStr[i]==ch)
				return &srcStr[i];
		}
	}
	return NULL;
}
char* SearchChar(char* srcStr,const char* delimiter,bool reverseOrder/*=false*/)
{
	char* iter=NULL;
	const char* iter2=NULL;
	if(!reverseOrder)
	{
		for(iter=srcStr;*iter!=0;iter++)
		{
			for(iter2=delimiter;*iter2!=0;iter2++)
			{
				if(*iter==*iter2)
					return iter;
			}
		}
	}
	else
	{
		int len=strlen(srcStr);
		for(int i=len-1;i>=0;i--)
		{
			for(iter2=delimiter;*iter2!=0;iter2++)
			{
				if(srcStr[i]==*iter2)
					return &srcStr[i];
			}
		}
	}
	return NULL;
}
void SimplifiedNumString(CString &num_str)
{
	int str_len = num_str.GetLength();
	int pointPos = -1;
	for(int i=str_len-1;i>=0;i--)
	{
		if(num_str[i]=='.')
		{
			pointPos=i;
			break;
		}
	}
	if(pointPos<0)
		return;
	if(pointPos>0&&!isdigit((BYTE)num_str[pointPos-1]))
		return; //小数点前一位为数字时才进行下一步的简化 wht 11-04-01
	int iPos=str_len-1;
	while(iPos>=pointPos)
	{
		if(num_str[iPos]=='0'||num_str[iPos]=='.')
			iPos--;
		else
			break;
	}
	num_str=num_str.Left(iPos+1);
	if(num_str.Compare("-0")==0)
		num_str="0";
}
void MakeDirectory(const char *path)
{
	char bak_path[MAX_PATH],drive[MAX_PATH];
	strcpy(bak_path,path);
	char *dir = strtok(bak_path,"/\\");
	if(strlen(dir)==2&&dir[1]==':')
	{
		strcpy(drive,dir);
		strcat(drive,"\\");
		_chdir(drive);
		dir = strtok(NULL,"/\\");
	}
	while(dir)
	{
		_mkdir(dir);
		_chdir(dir);
		dir = strtok(NULL,"/\\");
	}
}
//判断dest_val是否为四个参数中的绝对值最大的一个(包括与其它值相等)
BOOL IsMaxInAll(double dest_val,double other1,double other2,double other3)
{
	if(fabs(dest_val)-fabs(other1)<-EPS)
		return FALSE;
	else if(fabs(dest_val)-fabs(other2)<-EPS)
		return FALSE;
	else if(fabs(dest_val)-fabs(other3)<-EPS)
		return FALSE;
	else
		return TRUE;
}
//计算角钢上指定节点处开合角起始与终止位置(角钢相对坐标值)
static BOOL CalKaiHeStartEndPos(CLDSLineAngle *pRayJg,const f3dLine &lineX,const f3dLine &lineY,const f3dLine &lineBer,f3dLine &rayLineWing,
								BOOL bInXWing,BOOL bStartJoint,const UCS_STRU &ucs,f3dPoint &startPos,f3dPoint &endPos)
{
	BOOL bNeedKaiHe=FALSE;
	f3dLine rayLineBer,line_wing;
	f3dPoint vec,inters;
	Sub_Pnt(vec,pRayJg->End(),pRayJg->Start());
	normalize(vec);
	if(bStartJoint)
	{
		Sub_Pnt(rayLineWing.startPt,rayLineWing.startPt,vec*pRayJg->startOdd());
		Sub_Pnt(rayLineBer.startPt,pRayJg->Start(),vec*pRayJg->startOdd());
		rayLineBer.endPt = pRayJg->End();
	}
	else
	{
		rayLineBer.startPt = pRayJg->Start();
		Add_Pnt(rayLineWing.endPt,rayLineWing.endPt,vec*pRayJg->endOdd());
		Add_Pnt(rayLineBer.endPt,pRayJg->End(),vec*pRayJg->endOdd());
	}
	//转换为基准角钢相对坐标系下的直线
	coord_trans(rayLineWing.startPt,ucs,FALSE);
	coord_trans(rayLineWing.endPt,ucs,FALSE);
	coord_trans(rayLineBer.startPt,ucs,FALSE);
	coord_trans(rayLineBer.endPt,ucs,FALSE);
	f3dPoint current_end=startPos;
	if(bStartJoint)
		current_end=rayLineWing.startPt;
	else
		current_end=rayLineWing.endPt;
	if(bInXWing)
	{
		rayLineWing.startPt.y = rayLineWing.endPt.y = 0;
		rayLineBer.startPt.y = rayLineBer.endPt.y = 0;
		line_wing = lineX;
		if(current_end.x>0&&current_end.x<lineX.startPt.x)
		{
			if(startPos.z>current_end.z)
				startPos.z=current_end.z;
			if(endPos.z<current_end.z)
				endPos.z=current_end.z;
		}
	}
	else
	{
		rayLineWing.startPt.x = rayLineWing.endPt.x = 0;
		rayLineBer.startPt.x = rayLineBer.endPt.x = 0;
		line_wing = lineY;
		if(current_end.y>0&&current_end.y<lineY.startPt.y)
		{
			if(startPos.z>current_end.z)
				startPos.z=current_end.z;
			if(endPos.z<current_end.z)
				endPos.z=current_end.z;
		}
	}

	if(Int3dll(rayLineWing,line_wing,inters)>0)
	{
		if(startPos.z>inters.z)
			startPos = inters;
		if(endPos.z<inters.z)
			endPos = inters;
		bNeedKaiHe=TRUE;
	}
	if(Int3dll(rayLineWing,lineBer,inters)>0)
	{
		if(startPos.z>inters.z)
			startPos = inters;
		if(endPos.z<inters.z)
			endPos = inters;
		bNeedKaiHe=TRUE;
	}
	if(Int3dll(rayLineBer,line_wing,inters)>0)
	{
		if(startPos.z>inters.z)
			startPos = inters;
		if(endPos.z<inters.z)
			endPos = inters;
		bNeedKaiHe=TRUE;
	}
	if(Int3dll(rayLineBer,lineBer,inters)>0)
	{
		if(startPos.z>inters.z)
			startPos = inters;
		if(endPos.z<inters.z)
			endPos = inters;
		bNeedKaiHe=TRUE;
	}
	return bNeedKaiHe;
}
//计算角钢上指定节点处开合角起始与终止位置(角钢相对坐标值)Z坐标分量
static BOOL CalKaiHeStartEndPosByRayJg(CLDSLineAngle *pJg,CLDSNode *pNode,const f3dLine &lineX,const f3dLine &lineY,
								const f3dLine &lineBer,f3dPoint &startPos,f3dPoint &endPos,UCS_STRU &angleUcs)
{
	CTower* pTower=Manager.GetActiveTa();
	if(pTower==NULL)
		return FALSE;
	CLDSLineAngle *pRayJg;
	LINEPARTSET jgset;
	f3dPoint start_pos,end_pos;
	f3dLine rayLineWing;
	double ddmx_rx,ddmx_ry,ddmy_rx,ddmy_ry;
	start_pos = startPos;
	end_pos = endPos;
	BOOL bNeedKaiHe=FALSE;
	BOOL bKaiHeWingX=TRUE;
	int kaihe_base_wing=pJg->GetKaiHeWingX0Y1();
	if(kaihe_base_wing==-1)	//人工指定不需要开合角
		return FALSE;
	if(kaihe_base_wing==1)
		bKaiHeWingX=FALSE;
	double length=pJg->GetLength();
	for(CLDSPart *pPart=pTower->EnumPartFirst(CLS_LINEANGLE);pPart;pPart=pTower->EnumPartNext(CLS_LINEANGLE))
	{
		CLDSLinePart *pLinePart=(CLDSLinePart*)pPart;
		if((pLinePart->pStart==pNode&&!pLinePart->bHuoQuStart)||(pLinePart->pEnd==pNode&&!pLinePart->bHuoQuEnd))
			jgset.append(pLinePart);
	}
	for(pRayJg=(CLDSLineAngle*)jgset.GetFirst();pRayJg!=NULL;pRayJg=(CLDSLineAngle*)jgset.GetNext())
	{
		double wide,thick;
		f3dPoint vec;
		if(pRayJg==pJg)
			continue;
		Sub_Pnt(vec,pRayJg->End(),pRayJg->Start());
		normalize(vec);
		if(fabs(vec*angleUcs.axis_z)>EPS_COS2)
			continue;	//共线
		wide = pRayJg->GetWidth();
		thick = pRayJg->GetThick();
		ddmx_rx = dot_prod(pJg->get_norm_x_wing(),pRayJg->get_norm_x_wing());
		ddmx_ry = dot_prod(pJg->get_norm_x_wing(),pRayJg->get_norm_y_wing());
		ddmy_rx = dot_prod(pJg->get_norm_y_wing(),pRayJg->get_norm_x_wing());
		ddmy_ry = dot_prod(pJg->get_norm_y_wing(),pRayJg->get_norm_y_wing());
		if(IsMaxInAll(ddmx_rx,ddmx_ry,ddmy_rx,ddmy_ry)&&fabs(ddmx_rx)>EPS_COS2&&bKaiHeWingX)
		{	//X肢开合角
			Sub_Pnt(rayLineWing.startPt,pRayJg->Start(),pRayJg->get_norm_y_wing()*wide);
			Sub_Pnt(rayLineWing.endPt,pRayJg->End(),pRayJg->get_norm_y_wing()*wide);
			if(pRayJg->pStart==pNode)
				bNeedKaiHe=CalKaiHeStartEndPos(pRayJg,lineX,lineY,lineBer,rayLineWing,TRUE,TRUE,angleUcs,start_pos,end_pos);
			else
				bNeedKaiHe=CalKaiHeStartEndPos(pRayJg,lineX,lineY,lineBer,rayLineWing,TRUE,FALSE,angleUcs,start_pos,end_pos);
		}
		else if(IsMaxInAll(ddmx_ry,ddmx_rx,ddmy_rx,ddmy_ry)&&fabs(ddmx_ry)>EPS_COS2&&bKaiHeWingX)
		{	//X肢开合角
			Sub_Pnt(rayLineWing.startPt,pRayJg->Start(),pRayJg->get_norm_x_wing()*wide);
			Sub_Pnt(rayLineWing.endPt,pRayJg->End(),pRayJg->get_norm_x_wing()*wide);
			if(pRayJg->pStart==pNode)
				bNeedKaiHe=CalKaiHeStartEndPos(pRayJg,lineX,lineY,lineBer,rayLineWing,TRUE,TRUE,angleUcs,start_pos,end_pos);
			else
				bNeedKaiHe=CalKaiHeStartEndPos(pRayJg,lineX,lineY,lineBer,rayLineWing,TRUE,FALSE,angleUcs,start_pos,end_pos);
		}
		else if(IsMaxInAll(ddmy_rx,ddmx_rx,ddmx_ry,ddmy_ry)&&fabs(ddmy_rx)>EPS_COS2&&!bKaiHeWingX)
		{	//Y肢开合角
			Sub_Pnt(rayLineWing.startPt,pRayJg->Start(),pRayJg->get_norm_y_wing()*wide);
			Sub_Pnt(rayLineWing.endPt,pRayJg->End(),pRayJg->get_norm_y_wing()*wide);
			if(pRayJg->pStart==pNode)
				bNeedKaiHe=CalKaiHeStartEndPos(pRayJg,lineX,lineY,lineBer,rayLineWing,FALSE,TRUE,angleUcs,start_pos,end_pos);
			else
				bNeedKaiHe=CalKaiHeStartEndPos(pRayJg,lineX,lineY,lineBer,rayLineWing,FALSE,FALSE,angleUcs,start_pos,end_pos);
		}
		else if(IsMaxInAll(ddmy_ry,ddmy_rx,ddmx_rx,ddmx_ry)&&fabs(ddmy_ry)>EPS_COS2&&!bKaiHeWingX)
		{	//Y肢开合角
			Sub_Pnt(rayLineWing.startPt,pRayJg->Start(),pRayJg->get_norm_x_wing()*wide);
			Sub_Pnt(rayLineWing.endPt,pRayJg->End(),pRayJg->get_norm_x_wing()*wide);
			if(pRayJg->pStart==pNode)
				bNeedKaiHe=CalKaiHeStartEndPos(pRayJg,lineX,lineY,lineBer,rayLineWing,FALSE,TRUE,angleUcs,start_pos,end_pos);
			else
				bNeedKaiHe=CalKaiHeStartEndPos(pRayJg,lineX,lineY,lineBer,rayLineWing,FALSE,FALSE,angleUcs,start_pos,end_pos);
		}
		else
			continue;
		//因为角钢是斜的所以应起止各应再扩大肢宽的0.8(经验常数)
		startPos.z = __min(startPos.z,start_pos.z);
		endPos.z = __max(endPos.z,end_pos.z);
	}
	if(bNeedKaiHe)
	{	//10mm的开合角角钢间隙
		startPos.z-=10;
		endPos.z+=10;
	}
	return bNeedKaiHe;
}
BOOL CalKaiHeStartEndPos(CLDSLineAngle *pJg,CLDSNode *pNode,f3dPoint &startPos,f3dPoint &endPos,
				UCS_STRU &angleUcs,BOOL bIntelligent/*=FALSE*/,BOOL bUcsIncStartOdd/*=FALSE*/)
{
	CTower* pTower=Manager.GetActiveTa();
	if(pTower==NULL)
		return FALSE;
	f3dPoint inters;
	CTmaPtrList<CLDSLineAngle,CLDSLineAngle*> jgset,jgsetX,jgsetY;
	f3dLine lineX,lineY,lineBer,rayLineWing,rayLineBer,edgeline;
	double len,wing_wide,wing_thick;
	BOOL bKaiHe=FALSE;
	BOOL bKaiHeWingX=TRUE;
	int kaihe_base_wing=pJg->GetKaiHeWingX0Y1();
	if(kaihe_base_wing==-1)	//人工指定不需要开合角
		return FALSE;
	if(kaihe_base_wing==1)
		bKaiHeWingX=FALSE;
	len = pJg->GetLength();
	wing_wide = pJg->GetWidth();
	wing_thick = pJg->GetThick();
	//基准角钢上的根棱线(角钢相对坐标系下)
	double TOLERANCE=10;
	if(bUcsIncStartOdd)
	{	//lineX/Y/Ber用于与钢板轮廓边求交时,钢板轮廓边的坐标是在angleUcs下的,必须统一,否则对于存在正负头或制弯角钢时会漏判 wjh-2017.8.27
		f3dPoint xAcualStart=pJg->Start()-angleUcs.axis_z*pJg->startOdd();
		f3dPoint xAcualEnd=pJg->End()+angleUcs.axis_z*pJg->endOdd();
		double xAcualStartZ=(xAcualStart-angleUcs.origin)*angleUcs.axis_z-TOLERANCE;
		double xAcualEndZ=(xAcualEnd-angleUcs.origin)*angleUcs.axis_z+TOLERANCE;
		lineBer.startPt.Set(0,0,xAcualStartZ);
		lineBer.endPt.Set(0,0,xAcualEndZ);
		lineX.startPt.Set(wing_wide,0,xAcualStartZ);
		lineX.endPt.Set(wing_wide,0,xAcualEndZ);
		lineY.startPt.Set(0,wing_wide,xAcualStartZ);
		lineY.endPt.Set(0,wing_wide,xAcualEndZ);
	}
	else
	{
		lineBer.startPt.Set(0,0, -pJg->startOdd()-TOLERANCE);
		lineBer.endPt.Set(0,0,len-pJg->startOdd()+TOLERANCE);
		lineX.startPt.Set(wing_wide,0, -pJg->startOdd()-TOLERANCE);
		lineX.endPt.Set(wing_wide,0,len-pJg->startOdd()+TOLERANCE);
		lineY.startPt.Set(0,wing_wide, -pJg->startOdd()-TOLERANCE);
		lineY.endPt.Set(0,wing_wide,len-pJg->startOdd()+TOLERANCE);
	}

	int nPush=pTower->PushPartStack();
	PARTSET plateSet;
	BYTE wing_x0_y1_all2=2;
	if(!bIntelligent&&bKaiHeWingX)
		wing_x0_y1_all2=0;
	else if(!bIntelligent&&!bKaiHeWingX)
		wing_x0_y1_all2=1;
	GetPlateSetConnectedInAngle(pJg,plateSet,wing_x0_y1_all2,pNode);
	for(CLDSPlate *pPlate=(CLDSPlate*)plateSet.GetFirst();pPlate;pPlate=(CLDSPlate*)plateSet.GetNext())
	{
		int n=pPlate->vertex_list.GetNodeNum();
		if(n>2)	//有板
		{
			for(int j=0;j<n;j++)
			{
				edgeline.startPt=pPlate->GetRealVertex(pPlate->vertex_list[j].vertex);
				edgeline.endPt=pPlate->GetRealVertex(pPlate->vertex_list[(j+1)%n].vertex);
				coord_trans(edgeline.startPt,pPlate->ucs,TRUE);
				coord_trans(edgeline.endPt,pPlate->ucs,TRUE);
				coord_trans(edgeline.startPt,angleUcs,FALSE);
				coord_trans(edgeline.endPt,angleUcs,FALSE);
				if(bKaiHeWingX)
				{
					edgeline.startPt.y=edgeline.endPt.y=0;
					if(startPos.z>edgeline.startPt.z)
						startPos.z=edgeline.startPt.z;
					if(endPos.z<edgeline.startPt.z)
						endPos.z=edgeline.startPt.z;
					if(Int3dll(edgeline,lineX,inters)>0)
					{
						if(startPos.z>inters.z)
							startPos = inters;
						if(endPos.z<inters.z)
							endPos = inters;
						bKaiHe = TRUE;
					}
					if(Int3dll(edgeline,lineBer,inters)>0)
					{
						if(startPos.z>inters.z)
							startPos = inters;
						if(endPos.z<inters.z)
							endPos = inters;
						bKaiHe = TRUE;
					}
				}
				else
				{
					edgeline.startPt.x=edgeline.endPt.x=0;
					if(startPos.z>edgeline.startPt.z)
						startPos.z=edgeline.startPt.z;
					if(endPos.z<edgeline.startPt.z)
						endPos.z=edgeline.startPt.z;
					if(Int3dll(edgeline,lineY,inters)>0)
					{
						if(startPos.z>inters.z)
							startPos = inters;
						if(endPos.z<inters.z)
							endPos = inters;
						bKaiHe = TRUE;
					}
					if(Int3dll(edgeline,lineBer,inters)>0)
					{
						if(startPos.z>inters.z)
							startPos = inters;
						if(endPos.z<inters.z)
							endPos = inters;
						bKaiHe = TRUE;
					}
				}
			}
		}
	}
	pTower->PopPartStack(nPush);
	//退化为角钢直接连接的板连接设计
	if(bKaiHe)
	{	//10mm的开合角角钢间隙
		startPos.z-=10;
		endPos.z+=10;
	}
	else
		bKaiHe=CalKaiHeStartEndPosByRayJg(pJg,pNode,lineX,lineY,lineBer,startPos,endPos,angleUcs);
	return bKaiHe;
}
//转换超出顶点显示边界很多的二维线为有效线(否则WINDOWS显示不正常)
static BOOL Valid2dLine(GEPOINT2D start,GEPOINT2D end,GEPOINT2D topleft,GEPOINT2D btmright)
{
	double deitax=end.x-start.x;
	double deitay=end.y-start.y;
	if(fabs(deitax)<=EPS)
	{	//竖线
		if(start.x<topleft.x||start.x>btmright.x)
			return FALSE;
		if(start.y<topleft.y&&end.y<topleft.y)
			return FALSE;
		if(start.y>btmright.y&&end.y>btmright.y)
			return FALSE;
		return TRUE;
	}
	else if(fabs(deitay)<=EPS)
	{	//水平横线
		if(start.y<topleft.y||start.y>btmright.y)
			return FALSE;
		if(start.x<topleft.x&&end.x<topleft.x)
			return FALSE;
		if(start.x>btmright.x&&end.x>btmright.x)
			return FALSE;
		return TRUE;
	}
	double tan_val =(end.y-start.y)/deitax;
	double ctan_val=(end.x-start.x)/deitay;
	if(start.x<topleft.x)
	{
		start.y -= (start.x-topleft.x)*tan_val;
		start.x  = topleft.x;
	}
	else if(start.x>btmright.x)
	{
		start.y -= (start.x-btmright.x)*tan_val;
		start.x  = btmright.x;
	}
	if(start.y<topleft.y)
	{
		start.x-= (start.y-topleft.y)*ctan_val;
		start.y = topleft.y;
	}
	else if(start.y>btmright.y)
	{
		start.x -= (start.y-btmright.y)*ctan_val;
		start.y  = btmright.y;
	}
	if(end.x<topleft.x)
	{
		end.y -= (end.x-topleft.x)*tan_val;
		end.x  = topleft.x;
	}
	else if(end.x>btmright.x)
	{
		end.y -= (end.x-btmright.x)*tan_val;
		end.x  = btmright.x;
	}
	if(end.y<topleft.y)
	{
		end.x-= (end.y-topleft.y)*ctan_val;
		end.y = topleft.y;
	}
	else if(end.y>btmright.y)
	{
		end.x -= (end.y-btmright.y)*ctan_val;
		end.y = btmright.y;
	}
	else
		return FALSE;
	return TRUE;
}
static BOOL IsOusideLine(GEPOINT2D start,GEPOINT2D end, GEPOINT2D topleft,GEPOINT2D btmright)
{
	if(topleft.y>btmright.y)	//规范为左上角Y值小
	{
		double ff=topleft.y;
		topleft.y=btmright.y;
		btmright.y=ff;
	}
	if(start.x<start.x&&end.x<topleft.x)
		return TRUE;
	else if(start.y<topleft.y&&end.y<topleft.y)
		return TRUE;
	else if(start.x>btmright.x&&end.x>btmright.x)
		return TRUE;
	else if(start.y>btmright.y&&end.y>btmright.y)
		return TRUE;
	return FALSE;
}
static BOOL IsLineInRect(GEPOINT2D start,GEPOINT2D end, GEPOINT2D topleft,GEPOINT2D btmright)
{
	if(IsOusideLine(start,end,topleft,btmright))//区域外直线
		return FALSE;
	if( start.x<topleft.x||start.y<topleft.y||start.x>btmright.x||start.y>btmright.y||
		end.x  <topleft.x||end.y  <topleft.y||end.x  >btmright.x||end.y  >btmright.y)
		return Valid2dLine(start,end,topleft,btmright);
	else
		return TRUE;
}
//
void GetPlateSetConnectedInAngle(CLDSLineAngle *pAngle,PARTSET &plateSet,BYTE wing_x0_y1_all2,CLDSNode* pNode)
{
	CTower* pTower=Manager.GetActiveTa();
	if(pTower==NULL)
		return;
	plateSet.Empty();
	POLYGON rgn;
	int i=0;
	f3dPoint pos;
	if(pNode)
	{
		pos=pNode->Position();
		if(pNode==pAngle->pStart)
			pos+=pAngle->ucs.axis_z*(-pAngle->startOdd()+60);
		else if(pNode==pAngle->pEnd)
			pos-=pAngle->ucs.axis_z*(pAngle->endOdd()+60);
		SnapPerp(&pos,pAngle->Start(),pAngle->End(),pos);
	}
	const double MARGIN_EPS = pAngle->GetWidth();
	GECS wingx_cs,wingy_cs;
	//X肢坐标系
	wingx_cs.axis_x=pAngle->End()-pAngle->Start();
	normalize(wingx_cs.axis_x);
	wingx_cs.origin=GEPOINT(pAngle->Start())-wingx_cs.axis_x*pAngle->startOdd();
	wingx_cs.axis_y=pAngle->GetWingVecX();
	wingx_cs.axis_z=pAngle->vxWingNorm;
	//Y肢坐标系
	wingy_cs=wingx_cs;
	wingy_cs.axis_y=pAngle->GetWingVecY();
	wingy_cs.axis_z=pAngle->vyWingNorm;
	GEPOINT2D topleft(0,0),btmright(pAngle->GetLength(),pAngle->GetWidth());
	for(CLDSPart* pPart=pTower->EnumPartFirst(CLS_PLATE);pPart;pPart=pTower->EnumPartNext(CLS_PLATE))
	{	
		CLDSPlate* pPlate=(CLDSPlate*)pPart;
		int nFace=pPlate->GetFaceN();
		//1.过滤板的法线和角钢的法线不平行的
		f3dPoint vCurNorm;
		f3dPoint datum_pos=pPlate->ucs.origin;
		char cLappedWing='X';
		for(i=0;i<nFace;i++)
		{
			f3dPoint vPlateNorm=(i==0)?pPlate->ucs.axis_z:pPlate->GetHuoquFaceNorm(i-1);
			double ddx=vPlateNorm*pAngle->vxWingNorm;
			double ddy=vPlateNorm*pAngle->vyWingNorm;
			double justify=0;
			if((wing_x0_y1_all2==0||wing_x0_y1_all2==2)&&
				fabs(ddx)>EPS_COS2&&fabs(ddx)>fabs(justify))
			{
				vCurNorm=pAngle->vxWingNorm;
				cLappedWing='X';
				break;
			}
			if((wing_x0_y1_all2==1||wing_x0_y1_all2==2)&&
				fabs(ddy)>EPS_COS2&&fabs(ddy)>fabs(justify))
			{
				vCurNorm=pAngle->vyWingNorm;
				cLappedWing='Y';
				break;
			}
		}
		if(i>=nFace)
			continue;
		else if(i>0)
		{
			datum_pos=pPlate->GetHuoquLine(i-1).endPt;
			coord_trans(datum_pos,pPlate->ucs,TRUE);
		}
		//2.指定节点时首先根据钢板基准节点进行判断
		if(pNode&&pPlate->designInfo.m_hBaseNode==pNode->handle)
		{
			BOOL bIncAngle=FALSE;
			if(pPlate->designInfo.m_hBasePart==pAngle->handle)
				bIncAngle=TRUE;
			else
			{
				for(CDesignLjPartPara* pLjPart=pPlate->designInfo.partList.GetFirst();pLjPart;
					pLjPart=pPlate->designInfo.partList.GetNext())
				{
					if(pLjPart->hPart==pAngle->handle)
						bIncAngle=TRUE;
				}
			}
			if(bIncAngle)	//此连接板中没有连接当前角钢
			{
				plateSet.append(pPlate);
				continue;
			}
		}
		//3.未指定节点或根据指定节点判断无效时判断钢板是否与角钢相连
		if(fabs((datum_pos-pAngle->Start())*vCurNorm)>MARGIN_EPS)
			continue;
		//4.判断钢板与角钢是否有螺栓连接
		CLsRef *pLsRef=NULL;
		for(pLsRef=pPlate->GetFirstLsRef();pLsRef;pLsRef=pPlate->GetNextLsRef())
		{
			if(pAngle->FindLsByHandle((*pLsRef)->handle)!=NULL)
				break;
		}
		if(pLsRef==NULL)
			continue;
		//5.判断角钢是否与钢板贴合
		GECS cs=(cLappedWing=='X')?wingx_cs:wingy_cs;
		GEPOINT prevVertex;
		bool findPrevVertex=false,bValidPlate=false;
		CMaxDouble max_x;
		CMinDouble min_x;
		for (PROFILE_VERTEX* pVertex=pPlate->vertex_list.GetFirst();pVertex;pVertex=pPlate->vertex_list.GetNext())
		{
			//此处应该是获取钢板上轮廓点的真实坐标，不应调用SpreadToACS()函数  wxc-2017.1.20
			GEPOINT vertice=pPlate->GetRealVertex(pVertex->vertex);	
			vertice.z+=pPlate->GetNormOffset();
			coord_trans(vertice,pPlate->ucs,TRUE,TRUE);
			vertice=cs.TransPToCS(vertice);
			min_x.Update(vertice.x);
			max_x.Update(vertice.x);
			if(fabs(vertice.z)>pAngle->GetThick()+pPlate->GetThick()*2)
				continue;
			if(!findPrevVertex)
			{
				prevVertex=vertice;
				findPrevVertex=true;
				continue;
			}
			else if(!bValidPlate&&IsLineInRect(GEPOINT2D(prevVertex.x,prevVertex.y),GEPOINT2D(vertice.x,vertice.y),topleft,btmright))
				bValidPlate=true;
			prevVertex=vertice;
		}
		if(!bValidPlate)
			continue;
		if(pNode)
		{
			f3dPoint node_pos=pos;
			node_pos=cs.TransPToCS(node_pos);
			if(node_pos.x<min_x.number||node_pos.x>max_x.number)
				continue;
		}
		plateSet.append(pPlate);
	}
}

// return string length or -1 if UNICODE string is found in the archive
static UINT ReadStringLengthFromStream(IStream *pStream)
{
	DWORD nNewLen;

	// attempt BYTE length first
	BYTE bLen;
	pStream->Read(&bLen,1,NULL);

	if (bLen < 0xff)
		return bLen;

	// attempt WORD length
	WORD wLen;
	pStream->Read(&wLen,sizeof(WORD),NULL);
	if (wLen == 0xfffe)
	{
		// UNICODE string prefix (length will follow)
		return (UINT)-1;
	}
	else if (wLen == 0xffff)
	{
		// read DWORD of length
		pStream->Read(&nNewLen,sizeof(DWORD),NULL);
		return (UINT)nNewLen;
	}
	else
		return wLen;
}
//必须保证字符串空间足够大
UINT ReadStringFromStream(IStream *pStream,char *sReadString)
{
	UINT len=ReadStringLengthFromStream(pStream);
	if(sReadString==NULL)
		return len;
	else
		pStream->Read(sReadString,len,NULL);
	sReadString[len]='\0';
	return len;
}
// String format:
//      UNICODE strings are always prefixed by 0xff, 0xfffe
//      if < 0xff chars: len:BYTE, TCHAR chars
//      if >= 0xff characters: 0xff, len:WORD, TCHAR chars
//      if >= 0xfffe characters: 0xff, 0xffff, len:DWORD, TCHARs
void WriteStringToStream(IStream *pStream,char *ss)
{
	BYTE b;
	WORD w;
	DWORD dw,str_len;
	// special signature to recognize unicode strings
#ifdef _UNICODE
	b=0xff;
	w=0xfffe;
	pStream->Write(&b,1,NULL);
	pStream->Write(&w,sizeof(WORD),NULL);
#endif
	if(ss)
		str_len=strlen(ss);
	else
		str_len=0;

	if (strlen(ss)< 255)
	{
		b=(BYTE)strlen(ss);
		pStream->Write(&b,1,NULL);
	}
	else if (strlen(ss) < 0xfffe)
	{
		b=0xff;
		w=(WORD)strlen(ss);
		pStream->Write(&b,1,NULL);
		pStream->Write(&w,sizeof(WORD),NULL);
	}
	else
	{
		b=0xff;
		w=0xffff;
		dw=strlen(ss);
		pStream->Write(&b,1,NULL);
		pStream->Write(&w,sizeof(WORD),NULL);
		pStream->Write(&dw,sizeof(DWORD),NULL);
	}
	if(str_len>0)
		pStream->Write(ss,str_len,NULL);
}
//
BOOL GetSysPath(char* sys_path)
{
	char sPath[MAX_PATH]="",sSubKey[MAX_PATH]="";
	strcpy(sSubKey,"Software\\Xerofox\\TPSM\\Settings");
	//
	HKEY hKey;
	RegOpenKeyEx(HKEY_CURRENT_USER,sSubKey,0,KEY_READ,&hKey);
	DWORD dwDataType,dwLength=MAX_PATH;
	if(RegQueryValueEx(hKey,_T("SYS_PATH"),NULL,&dwDataType,(BYTE*)&sPath[0],&dwLength)== ERROR_SUCCESS)
		strcpy(sys_path,sPath);
	RegCloseKey(hKey);
	if(strlen(sys_path)>1)
		return TRUE;
	return FALSE;
}
BOOL GetLicFile(char* lic_path)
{
	char sPath[MAX_PATH]="",sSubKey[MAX_PATH]="";
	strcpy(sSubKey,"Software\\Xerofox\\TPSM\\Settings");
	//
	HKEY hKey;
	RegOpenKeyEx(HKEY_CURRENT_USER,sSubKey,0,KEY_READ,&hKey);
	DWORD dwDataType,dwLength=MAX_PATH;
	if(RegQueryValueEx(hKey,_T("lic_file"),NULL,&dwDataType,(BYTE*)&sPath[0],&dwLength)== ERROR_SUCCESS)
		strcpy(lic_path,sPath);
	RegCloseKey(hKey);
	if(strlen(lic_path)>1)
		return TRUE;
	return FALSE;
}
BOOL GetLdsPath(char* lds_path)
{
	char sPath[MAX_PATH]="",sSubKey[MAX_PATH]="";
	strcpy(sSubKey,"Software\\Xerofox\\TPSM\\Settings");
	//
	HKEY hKey;
	RegOpenKeyEx(HKEY_CURRENT_USER,sSubKey,0,KEY_READ,&hKey);
	DWORD dwDataType,dwLength=MAX_PATH;
	if(RegQueryValueEx(hKey,_T("LDS_PATH"),NULL,&dwDataType,(BYTE*)&sPath[0],&dwLength)== ERROR_SUCCESS)
		strcpy(lds_path,sPath);
	RegCloseKey(hKey);
	if(strlen(lds_path)>1)
		return TRUE;
	return FALSE;
}

char restore_JG_guige(const char* guige, double &wing_wide, double &wing_thick)
{
	char mark,material;
	sscanf(guige,"%lf%c%lf%c",&wing_wide,&mark,&wing_thick,&material);
	material  = (char)toupper(material);//角钢材质A3(S)或Q345(H)
	return material;
}