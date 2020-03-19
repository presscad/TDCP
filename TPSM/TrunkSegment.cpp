#include "stdafx.h"
#include <math.h>
#include "f_ent.h"
#include "f_alg_fun.h"
#include "HashTable.h"
#include "TrunkSegment.h"
#include "LogFile.h"
#include "Tower.h"
#include "ParseAdaptNo.h"
#include "DxfFile.h"
#include "TowerTypeTemp.h"

//坡度δx(δy)/δz
double PLAN_SECTION::set_dfSlopeGradient(double dfGradient)
{
	if(_dfSlopeGradient>=0)
	{
		_dfSlopeGradient=dfGradient;
		_dfZp2Zh=sqrt(1+dfSlopeGradient*dfSlopeGradient);
		if(_wiSegHeight>0&&!IsLegSegment())
		{
			double dfHalfDetaX=dfSlopeGradient*_wiSegHeight;
			double _dfBtmWidth=this->xTopPlane.dfWidth+dfHalfDetaX*2;
		}
	}
	return _dfSlopeGradient;
}
long PLAN_SECTION::set_ziBtmPlane(long ziPlane)
{	//铁塔基础平面Z坐标
	return this->xBtmPlane.ziPlane=ziPlane;
}
double PLAN_SECTION::set_dfBtmWidth(double dfWidth)
{	//铁塔设计根开
	return this->xBtmPlane.dfWidth=dfWidth;
}
TRUNK_SEGMENT::TRUNK_SEGMENT(double dfTopTrunkWidth/*=4500*/,double dfGradientCoef/*=0.11*/)
{
	_ciLegSerial=0;	//塔身
	idPanelType=0;
	xTopPlane.ziPlane=0;
	//原始联动设计参数
	_wiSegHeight=4000;
	_dfSlopeGradient=dfGradientCoef;
	xTopPlane.dfWidth=dfTopTrunkWidth;
	//Zp表示坡面高，Zh表示三维空间高
	_dfZp2Zh=1.00603180864;//sqrt(1+dfSlopeGradient*dfSlopeGradient);
	xBtmPlane.dfWidth=5380;//_dfTopWidth+2*dfSlopeGradient*_wiSegHeight;
	cfgword.Clear();
}
//塔身节间垂高
WORD TRUNK_SEGMENT::set_wiSegHeight(WORD wiHeight)
{
	if(wiHeight==_wiSegHeight||wiHeight==0)
		return wiHeight;
	_wiSegHeight=wiHeight;
	if(_dfSlopeGradient>=0&&_wiSegHeight>0)
	{
		double dfHalfDetaX=dfSlopeGradient*_wiSegHeight;
		this->xBtmPlane.dfWidth=this->xTopPlane.dfWidth+dfHalfDetaX*2;
		this->xBtmPlane.ziPlane=this->xTopPlane.ziPlane+_wiSegHeight;
	}
	return _wiSegHeight;
}
double TRUNK_SEGMENT::set_dfTopWidth(double dfWidth)
{
	if(dfWidth==xTopPlane.dfWidth||dfWidth==0)
		return xTopPlane.dfWidth;
	xTopPlane.dfWidth=dfWidth;
	if(_dfSlopeGradient>=0&&_wiSegHeight>0)
	{
		double dfHalfDetaX=dfSlopeGradient*_wiSegHeight;
		double _dfBtmWidth=this->xTopPlane.dfWidth+dfHalfDetaX*2;
	}
	return xTopPlane.dfWidth;
}
//腹杆水平夹角
double TRUNK_SEGMENT::get_dfBevelGradDegAng()
{
	return 0;
}

double TRUNK_SEGMENT::CalBevelGradDegAngle()		//左右贯通腹杆水平夹角
{
	//Zp表示坡面高，Zh表示三维空间高
	double dfZp2Zh=_dfZp2Zh;//sqrt(1+dfSlopeGradient*dfSlopeGradient);
	double dfSlopeHeight=wiSegHeight*dfZp2Zh;
	double dfHalfDetaX=(xBtmPlane.dfWidth-xTopPlane.dfWidth)*0.5;
	double dfDegAngle=atan(dfSlopeHeight/(xTopPlane.dfWidth+dfHalfDetaX))*DEGTORAD_COEF;
	return dfDegAngle;
}
double TRUNK_SEGMENT::CalUpHalfBevelGradDegAngle()
{	//上侧半贯通（正K字布材）腹杆水平夹角
	//Zp表示坡面高，Zh表示三维空间高
	double dfZp2Zh=_dfZp2Zh;//sqrt(1+dfSlopeGradient*dfSlopeGradient);
	double dfSlopeHeight=wiSegHeight*dfZp2Zh;
	double dfDegAngle=atan((dfSlopeHeight+dfSlopeHeight)/this->dfBtmWidth)*DEGTORAD_COEF;
	return dfDegAngle;
}
double TRUNK_SEGMENT::CalDownHalfBevelGradDegAngle()
{	//下侧半贯通（倒K字布材）腹杆水平夹角
	//Zp表示坡面高，Zh表示三维空间高
	double dfZp2Zh=_dfZp2Zh;//sqrt(1+dfSlopeGradient*dfSlopeGradient);
	double dfSlopeHeight=wiSegHeight*dfZp2Zh;
	double dfDegAngle=atan((dfSlopeHeight+dfSlopeHeight)/dfTopWidth)*DEGTORAD_COEF;
	return dfDegAngle;
}
bool TRUNK_SEGMENT::CalWholeSectHeight(double* pfMinHeight,double* pfMaxHeight,double dfMinBevelDegAngle/*=25*/,double dfMaxBevelDegAngle/*=40*/)
{
	//左右贯通腹杆水平夹角
	//Zp表示坡面高，Zh表示三维空间高
	double dfZh2Zp=1.0/_dfZp2Zh;//sqrt(1+dfSlopeGradient*dfSlopeGradient);
	double dfMinTanBevelAngle=tan(dfMinBevelDegAngle*RADTODEG_COEF);
	double dfMaxTanBevelAngle=tan(dfMaxBevelDegAngle*RADTODEG_COEF);
	//  (dfTopWidth+dfHalfDetaX)/dfSlopeHeight<=1/dfMinTanBevelAngle;
	//->dfTopWidth/dfSlopeHeight+dfSlopeGradient/dfZp2Zh<=1/dfMinTanBevelAngle;
	//->dfSlopeHeight>=dfTopWidth/(1/dfMinTanBevelAngle-dfSlopeGradient/dfZp2Zh)
	double dfMinHeight=dfZh2Zp*xTopPlane.dfWidth/(1/dfMinTanBevelAngle-dfSlopeGradient*dfZh2Zp);
	double dfMaxHeight=dfZh2Zp*xTopPlane.dfWidth/(1/dfMaxTanBevelAngle-dfSlopeGradient*dfZh2Zp);
	if(pfMinHeight)
		*pfMinHeight=dfMinHeight;
	if(pfMaxHeight)
		*pfMaxHeight=dfMaxHeight;
	return true;
}
bool TRUNK_SEGMENT::CalUpHalfSectHeight(double* pfMinHeight,double* pfMaxHeight,double dfMinBevelDegAngle/*=25*/,double dfMaxBevelDegAngle/*=40*/)
{
	//Zp表示坡面高，Zh表示三维空间高
	double dfZh2Zp=1.0/_dfZp2Zh;//sqrt(1+dfSlopeGradient*dfSlopeGradient);
	double dfMinTanBevelAngle=tan(dfMinBevelDegAngle*RADTODEG_COEF);
	double dfMaxTanBevelAngle=tan(dfMaxBevelDegAngle*RADTODEG_COEF);
	//  (0.5*dfTopWidth+dfHalfDetaX)/dfSlopeHeight<=1/dfMinTanBevelAngle;
	//-> 0.5*dfTopWidth/dfSlopeHeight+dfSlopeGradient/dfZp2Zh<=1/dfMinTanBevelAngle;
	//->dfSlopeHeight>=0.5*dfTopWidth/(1/dfMinTanBevelAngle-dfSlopeGradient/dfZp2Zh)
	double dfMinHeight=dfZh2Zp*0.5*xTopPlane.dfWidth/(1/dfMinTanBevelAngle-dfSlopeGradient*dfZh2Zp);
	double dfMaxHeight=dfZh2Zp*0.5*xTopPlane.dfWidth/(1/dfMaxTanBevelAngle-dfSlopeGradient*dfZh2Zp);
	if(pfMinHeight)
		*pfMinHeight=dfMinHeight;
	if(pfMaxHeight)
		*pfMaxHeight=dfMaxHeight;
	return true;
}
bool TRUNK_SEGMENT::CalDownHalfSectHeight(double* pfMinHeight,double* pfMaxHeight,double dfMinBevelDegAngle/*=25*/,double dfMaxBevelDegAngle/*=40*/)
{
	//Zp表示坡面高，Zh表示三维空间高
	double dfZh2Zp=1.0/_dfZp2Zh;//sqrt(1+dfSlopeGradient*dfSlopeGradient);
	double dfMinTanBevelAngle=tan(dfMinBevelDegAngle*RADTODEG_COEF);
	double dfMaxTanBevelAngle=tan(dfMaxBevelDegAngle*RADTODEG_COEF);
	//  dfSlopeHeight/(0.5*dfTopWidth)>=dfMinTanBevelAngle;
	//->dfSlopeHeight>=0.5*dfTopWidth*dfMinTanBevelAngle
	double dfMinHeight=dfZh2Zp*0.5*xTopPlane.dfWidth*dfMinTanBevelAngle;
	double dfMaxHeight=dfZh2Zp*0.5*xTopPlane.dfWidth*dfMaxTanBevelAngle;
	if(pfMinHeight)
		*pfMinHeight=dfMinHeight;
	if(pfMaxHeight)
		*pfMaxHeight=dfMaxHeight;
	return true;
}
TRUNK_SUBLEG::TRUNK_SUBLEG()
{
	this->idPanelType=SECT_SUBLEG;
	this->_ciLegSerial=1;	//>=1表示接腿分段节间
}
HEIGHTGROUP::HEIGHTGROUP()
{
	hRelaHeightModule=0;
	_ciBodySerial=1;
	dfSlopeGradient=0.11;
	ziCurrPlane=0;
	xSublegInfo.xFront.pLeftNode=xSublegInfo.xFront.pRightNode=NULL;
	xSublegInfo.xRight=xSublegInfo.xBack=xSublegInfo.xLeft=xSublegInfo.xFront;
}
BYTE HEIGHTGROUP::set_ciBodySerial(BYTE ciBodyNo)
{
	if(ciBodyNo>0&&ciBodyNo<=24)
		_ciBodySerial=ciBodyNo;	//1 based serial
	return _ciBodySerial;
}
WORD HEIGHTGROUP::CalTopPlane(WORD wnLegCount,WORD wiLevelDiff,HEIGHTGROUP::LEGINFO* pxLegInfo)
{
	//Zp表示坡面高，Zh表示三维空间高
	double dfZp2Zh=sqrt(1+dfSlopeGradient*dfSlopeGradient);
	double dfZh2Zp=1.0/dfZp2Zh;
	double dfMaxZp2WxIsDegA20=tan(70*RADTODEG_COEF);	//实际LV斜材与主材夹角大于20°，即，ctan(α)<=ctan(20°)=tan_deg70
	double dfMinZp2WxIsDegB30=tan(30*RADTODEG_COEF);	//实际LV斜材水平夹角大于30°，即tan(β)>=tan(30°)
	double halfwidth=xBtmPlane.dfWidth*0.5;
	xSublegInfo.wiMaxLegHeight=(WORD)(halfwidth*dfMaxZp2WxIsDegA20*dfZh2Zp);
	xSublegInfo.wiMaxLegHeight-=xSublegInfo.wiMaxLegHeight%500;	//最长腿高一般为0.5m整数倍
	xSublegInfo.ziMinTopPlane=xBtmPlane.ziPlane-xSublegInfo.wiMaxLegHeight;
	double dfShortestLegCoordZ=xBtmPlane.ziPlane-wiLevelDiff*(wnLegCount-1);
	double dfShortestLegBtmWidth=xBtmPlane.dfWidth-2*dfSlopeGradient*(wiLevelDiff*(wnLegCount-1));
	xSublegInfo.ziMaxTopPlane=(int)(dfShortestLegCoordZ-(dfShortestLegBtmWidth*0.5)*dfMinZp2WxIsDegB30);
	xSublegInfo.ziMaxTopPlane-=xSublegInfo.ziMaxTopPlane%500;
	if(xSublegInfo.ziMaxTopPlane<xSublegInfo.ziMinTopPlane)
	{
		wnLegCount-=(WORD)(xSublegInfo.ziMinTopPlane-xSublegInfo.ziMaxTopPlane)/wiLevelDiff;
		xSublegInfo.ziMaxTopPlane=xSublegInfo.ziMinTopPlane;
	}
	wnLegCount=min(MAX_SUBLEG_COUNT,wnLegCount);
	xSublegInfo.wiLevelDiff=wiLevelDiff;
	xSublegInfo.wnLegCount=wnLegCount;
	if(pxLegInfo)
		*pxLegInfo=xSublegInfo;
	return wnLegCount;
}
double HEIGHTGROUP::CalPlaneWidth(double zfPlane)
{
	return xBtmPlane.dfWidth+(zfPlane-xBtmPlane.ziPlane)*dfSlopeGradient*2;
}
CTrunkBodyPlanner::EXIST_SECTION::EXIST_SECTION()
{
	idPanelType=TRUNK_SEGMENT::SECT_WHOLE;
	memset(harrRod,0,16);//[4];
	hTopNode=hBtmNode=0;
	ziTopPlane=ziBtmPlane=0;
	dfTopWidth=dfBtmWidth=0;
}
TRUNK_NODE* TRUNK_NODE::Append()
{
	if(this->linknext==NULL)
	{
		linknext=new TRUNK_NODE();
		return linknext;
	}
	else
		return linknext->Append();
}
TRUNK_NODE::TRUNK_NODE()
{
	ziPlane=0;
	memset(harrQuadNodes,0,16);
	linknext=NULL;
}
TRUNK_NODE::~TRUNK_NODE()
{
	DeleteChilds();
}
bool TRUNK_NODE::DeleteChilds()
{
	if(linknext==NULL)
		return false;
	linknext->DeleteChilds();
	delete linknext;	//最末尾级链接节点
	linknext=NULL;
	return true;
}
CTrunkBodyPlanner::CTrunkBodyPlanner()
{
	ciLegSchema=CFGLEG::MULTILEG_MAX08;
	wnHeightGroup=3;		//规划的呼高分组数
	wnMaxLegCount=1;
	wiLegDiffLevel=1000;	//规划的呼高内减腿级差
	ziTopPlane=25850;		//塔身顶面Z值
	ziHeight0Plane=8000;	//呼高计算起始基准平面的Z坐标
	dfSlopeGradient=0.11;	//坡度δx(δy)/δz
	uiTopWidth=4700;
	uiMinHeight=30000;
	uiMaxHeight=48000;
	memset(xSlope.harrTopNodes,0,16);
	memset(xSlope.harrBtmNodes,0,16);
}
double CTrunkBodyPlanner::CalPlaneWidth(double zfPlane)
{
	return uiTopWidth+(zfPlane-ziTopPlane)*dfSlopeGradient*2;
}
bool CTrunkBodyPlanner::Planning(IXhList<HEIGHTGROUP>* pListPlanHeights,bool bTopHalfSection/*=false*/)
{
	UINT uiVarHeight=uiMaxHeight-uiMinHeight;	//指
	UINT i,j,uiHeightLevel=0;
	if(wnHeightGroup>1)
	{
		if(uiVarHeight%(wnHeightGroup-1)>0)
			logerr.Log("规划呼高分组数与呼高规划范围不一致");
		uiHeightLevel=uiVarHeight/(wnHeightGroup-1);
	}
	CHashListEx<HEIGHTGROUP> hashPlanHeights;
	if(pListPlanHeights==NULL)
		pListPlanHeights=&hashPlanHeights;
	//1.计算最高呼高最长腿的铁塔设计根开
	double dfBtmWidth=this->uiTopWidth+(this->ziHeight0Plane+this->uiMaxHeight-this->ziTopPlane)*this->dfSlopeGradient*2;
	//2.生成呼高分组及基本信息
	HEIGHTGROUP* pHeight;
	PRESET_ARRAY32<HEIGHTGROUP*> heightset;	//从低到高排序
	for(i=this->wnHeightGroup;i>0;i--)
	{
		pHeight=pListPlanHeights->Append(i);
		pHeight->ciBodySerial=i;
		heightset.Set(i-1,pHeight);
		int index=this->wnHeightGroup-i;
		pHeight->dfSlopeGradient=this->dfSlopeGradient;
		pHeight->uiNamedHeight=this->uiMaxHeight-uiHeightLevel*index;//呼高
		pHeight->ziCurrPlane=this->ziTopPlane;
		pHeight->xBtmPlane.ziPlane=this->ziHeight0Plane+pHeight->uiNamedHeight;
		pHeight->xBtmPlane.dfWidth=dfBtmWidth-uiHeightLevel*index*this->dfSlopeGradient*2;
	}
	//3.生成各呼高的减腿基本信息
	wnMaxLegCount=uiHeightLevel*2/this->wiLegDiffLevel;
	for(pHeight=pListPlanHeights->MoveFirst();pHeight!=NULL;pHeight=pListPlanHeights->MoveNext())
	{
		WORD wnLegCount=pHeight->CalTopPlane(wnMaxLegCount,this->wiLegDiffLevel);
		if(wnLegCount>wnMaxLegCount)
			wnMaxLegCount=wnLegCount;
		//pHeight->xarrSubLegs.ZeroPresetMemory();
		pHeight->xarrSubLegs.ReSize(wnLegCount);
		for(j=0;j<wnLegCount;j++)
		{
			pHeight->xarrSubLegs[j].ciLegSerial=j+1;
			pHeight->xarrSubLegs[j].dfSlopeGradient=this->dfSlopeGradient;
			pHeight->xarrSubLegs[j].wiLegHeight=pHeight->xSublegInfo.wiMaxLegHeight-j*this->wiLegDiffLevel;
			pHeight->xarrSubLegs[j].ziBtmPlane=pHeight->xBtmPlane.ziPlane-j*this->wiLegDiffLevel;
			pHeight->xarrSubLegs[j].dfBtmWidth=pHeight->CalPlaneWidth(pHeight->xarrSubLegs[j].ziBtmPlane);
		}
	}
	ciLegSchema=wnMaxLegCount<=8?CFGLEG::MULTILEG_MAX08:CFGLEG::MULTILEG_MAX16;
	CLegSchemaLife schemalife(ciLegSchema);
	//4.计算全部呼高的共用区间Body
	TRUNK_PLANE xCurrPlane;
	TRUNK_SEGMENT* pSegment;
	xCurrPlane.ziPlane=this->ziTopPlane;
	xCurrPlane.dfWidth=this->uiTopWidth;
	long ziLowestPlane=this->ziHeight0Plane+this->uiMaxHeight;
	bool bFirstSection=true;
	while(xCurrPlane.ziPlane<ziLowestPlane)
	{
		TRUNK_SEGMENT body(xCurrPlane.dfWidth,this->dfSlopeGradient);
		double dfMinHeight=4500,dfMaxHeight=4500;
		bool successed=true;
		if(bFirstSection&&bTopHalfSection)
		{
			successed=body.CalUpHalfSectHeight(&dfMinHeight,&dfMaxHeight);
			body.idPanelType=TRUNK_SEGMENT::SECT_HALF_UP;
		}
		else
			successed=body.CalWholeSectHeight(&dfMinHeight,&dfMaxHeight);
		bFirstSection=false;
		long uiPreferHeight=ftoi((dfMinHeight+dfMaxHeight)*0.5);
		long uiCurrHeight=ftoi(dfMaxHeight);
		uiPreferHeight-=uiPreferHeight%100;	//规划阶段节间高度到100mm精度就够了。
		uiCurrHeight-=uiCurrHeight%100;
		for(j=0;j<heightset.Count;j++)
		{
			pHeight=heightset[j];
			if(xCurrPlane.ziPlane+uiCurrHeight<pHeight->xSublegInfo.ziMaxTopPlane)
			{
				body.cfgword.AddBodyLegs(pHeight->ciBodySerial);
				continue;
			}
			else if(xCurrPlane.ziPlane+dfMinHeight<pHeight->xSublegInfo.ziMaxTopPlane)
			{
				uiCurrHeight=pHeight->xSublegInfo.ziMaxTopPlane-xCurrPlane.ziPlane;
				body.cfgword.AddBodyLegs(pHeight->ciBodySerial);
				continue;
			}
		}
		if(body.cfgword.IsNull())
			break;
		pSegment=this->hashBodySegments.Append(body);
		pSegment->xTopPlane.ziPlane=xCurrPlane.ziPlane;
		pSegment->wiSegHeight=(WORD)min(uiPreferHeight,uiCurrHeight);
		xCurrPlane.dfWidth=pSegment->dfBtmWidth;
		xCurrPlane.ziPlane=xCurrPlane.ziPlane+pSegment->wiSegHeight;
	}
	//5.补充呼高分组的专有塔身节间, 确定腿身隔面位置
	for(j=0;j<heightset.Count;j++)
	{	//计算各呼高分组当前已规划到水平面位置
		for(pSegment=this->hashBodySegments.GetFirst();pSegment;pSegment=this->hashBodySegments.GetNext())
		{
			if(pSegment->cfgword.IsHasBodyNo(heightset[j]->ciBodySerial))
				heightset[j]->ziCurrPlane=max(pSegment->ziBtmPlane,heightset[j]->ziCurrPlane);
		}
	}
	for(j=0;j<heightset.Count;j++)
	{
		pHeight=heightset[j];
		if(pHeight->xSublegInfo.ziMinTopPlane-pHeight->ziCurrPlane<=0)
		{	//已不需要直接布塔身节间，直接接各减腿
			pHeight->xLegTopPlane.ziPlane=pHeight->ziCurrPlane;
			pHeight->xLegTopPlane.dfWidth=pHeight->CalPlaneWidth(pHeight->ziCurrPlane);
			for(UINT k=0;k<pHeight->xarrSubLegs.Count;k++)
				pHeight->xarrSubLegs[k].xTopPlane=pHeight->xLegTopPlane;
		}
		else
		{
			double dfCurrWidth=pHeight->CalPlaneWidth(pHeight->ziCurrPlane);
			TRUNK_SEGMENT halfseg(dfCurrWidth,this->dfSlopeGradient);
			double dfMinHeight=4500,dfMaxHeight=4500;
			bool successed=halfseg.CalDownHalfSectHeight(&dfMinHeight,&dfMaxHeight);
			long uiPreferHeight=ftoi((dfMinHeight+dfMaxHeight)*0.5);
			uiPreferHeight-=uiPreferHeight%100;
			if(pHeight->ziCurrPlane+uiPreferHeight<=pHeight->xSublegInfo.ziMaxTopPlane)
				uiPreferHeight=max(uiPreferHeight,pHeight->xSublegInfo.ziMinTopPlane-pHeight->ziCurrPlane);
			else
				uiPreferHeight=pHeight->xSublegInfo.ziMaxTopPlane-pHeight->ziCurrPlane;
			halfseg.cfgword.SetBodyLegs(pHeight->ciBodySerial);
			pSegment=this->hashBodySegments.Append(halfseg,0);
			pSegment->xTopPlane.ziPlane=pHeight->ziCurrPlane;
			pSegment->wiSegHeight=(WORD)uiPreferHeight;
			pSegment->idPanelType=TRUNK_SEGMENT::SECT_HALF_DOWN;
			pHeight->xLegTopPlane.ziPlane=pSegment->ziBtmPlane;
			pHeight->xLegTopPlane.dfWidth=pSegment->dfBtmWidth;//CalPlaneWidth(pHeight->xLegTopPlane.ziPlane);
			for(UINT k=0;k<pHeight->xarrSubLegs.Count;k++)
				pHeight->xarrSubLegs[k].xTopPlane=pHeight->xLegTopPlane;
		}
	}
	return true;
}

static CFGWORD StrToCfgWord(const char* bodystr)
{
	CFGWORD cfgword;
	long serial=FindAdaptNo(bodystr,",","-");
	cfgword.AddBodyLegs(serial);
	while((serial=FindAdaptNo(NULL))>0)
		cfgword.AddBodyLegs(serial);
	return cfgword;
}
bool CTrunkBodyPlanner::InitTrunkInfo(UINT idTowerModelTmpl/*=0*/)
{
	EXIST_HEIGHT* pHeight=NULL;
	EXIST_SECTION* pSection=NULL;
	if(idTowerModelTmpl==TYPE_SZC)
	{
		dfSlopeGradient=0.1;
		uiTopWidth=4000;
		ziTopPlane=29550;	//初始化最下面坡度塔身的顶部位置
		ziHeight0Plane=26550;
		//初始化已有各呼高的腿身过渡平面信息
		hashExistHeights.Empty();
		pHeight=hashExistHeights.Add(27000);	//27m
		pHeight->ciBodySerial=1;
		pHeight->hTransitNode=0x92;
		pHeight->hPostLegRod =0x40C;
		pHeight->hLowestBaseNode=0x9A;
		pHeight=hashExistHeights.Add(33000);	//33m
		pHeight->ciBodySerial=2;
		pHeight->hTransitNode	=0xA2;
		pHeight->hPostLegRod	=0x468;
		pHeight->hLowestBaseNode=0xA6;
		pHeight=hashExistHeights.Add(42000);	//42m
		pHeight->ciBodySerial=3;
		pHeight->hTransitNode	=0xB6;
		pHeight->hPostLegRod	=0x4F4;
		pHeight->hLowestBaseNode=0xC2;
		//塔身节间
		pSection=listExistSections.Append();//1-3
		pSection->hTopNode=0x3A;
		pSection->hBtmNode=0x82;
		pSection->hQuadRod=0x376;
		pSection=listExistSections.Append();//1-3
		pSection->hTopNode=0x82;
		pSection->hBtmNode=0x86;
		pSection->hQuadRod=0x37a;
		pSection=listExistSections.Append();//1-3
		pSection->hTopNode=0x86;
		pSection->hBtmNode=0x8A;
		pSection->hQuadRod=0x3A4;
		pSection=listExistSections.Append();//1-3
		pSection->hTopNode=0x8A;
		pSection->hBtmNode=0x8E;
		pSection->hQuadRod=0x3A8;
		pSection=listExistSections.Append();//3
		pSection->hTopNode=0x8E;
		pSection->hBtmNode=0xAE;
		pSection->hQuadRod=0x480;
		pSection=listExistSections.Append();//3
		pSection->hTopNode=0xAE;
		pSection->hBtmNode=0xB2;
		pSection->hQuadRod=0x48C;
		pSection=listExistSections.Append();//3
		pSection->hTopNode=0xB2;
		pSection->hBtmNode=0xB6;
		pSection->hQuadRod=0x490;
		//初始化塔身主材投影基准线
		xSlope.hTopNode=0x3A;
		xSlope.hBtmNode=0xC2;
	}
	else if(idTowerModelTmpl==TYPE_JG||idTowerModelTmpl==TYPE_JG2)
	{
		dfSlopeGradient=0.17;
		uiTopWidth=3860;
		ziTopPlane=18000;
		ziHeight0Plane=14000;
		//初始化已有各呼高的腿身过渡平面信息
		hashExistHeights.Empty();
		pHeight=hashExistHeights.Add(30000);	//30m
		pHeight->ciBodySerial=1;
		pHeight->hTransitNode=0x8D;
		pHeight->hPostLegRod =0x3BA;
		pHeight->hLowestBaseNode=0xB5;
		pHeight=hashExistHeights.Add(36000);	//36m
		pHeight->ciBodySerial=2;
		pHeight->hTransitNode	=0xC9;
		pHeight->hPostLegRod	=0x446;
		pHeight->hLowestBaseNode=0x7D;
		//塔身节间
		pSection=listExistSections.Append();//2
		pSection->hTopNode=0x33;
		pSection->hBtmNode=0xB9;
		pSection->hQuadRod=0x3C6;
		pSection=listExistSections.Append();//2
		pSection->hTopNode=0xB9;
		pSection->hBtmNode=0xBD;
		pSection->hQuadRod=0x3CA;
		pSection=listExistSections.Append();//2
		pSection->hTopNode=0xBD;
		pSection->hBtmNode=0xC1;
		pSection->hQuadRod=0x3CE;
		pSection=listExistSections.Append();//2
		pSection->hTopNode=0xC1;
		pSection->hBtmNode=0xC5;
		pSection->hQuadRod=0x3EA;
		pSection=listExistSections.Append();//2
		pSection->hTopNode=0xC5;
		pSection->hBtmNode=0xC9;
		pSection->hQuadRod=0x3EE;
		//初始化塔身主材投影基准线
		xSlope.hTopNode=0x33;
		xSlope.hBtmNode=0x7D;
	}
	else if(idTowerModelTmpl==TYPE_SJ)
	{
		dfSlopeGradient=0.32;
		uiTopWidth=7000;
		ziTopPlane=41200;	//初始化最下面坡度塔身的顶部位置
		ziHeight0Plane=38000;
		//初始化已有各呼高的腿身过渡平面信息
		hashExistHeights.Empty();
		pHeight=hashExistHeights.Add(30000);	//30M
		pHeight->ciBodySerial=1;
		pHeight->hTransitNode=0xB6;
		pHeight->hPostLegRod =0x5BC;
		pHeight->hLowestBaseNode=0xBE;
		pHeight=hashExistHeights.Add(34500);	//34.5m
		pHeight->ciBodySerial=2;
		pHeight->hTransitNode	=0xC2;
		pHeight->hPostLegRod	=0x618;
		pHeight->hLowestBaseNode=0xCA;
		pHeight=hashExistHeights.Add(39000);	//42m
		pHeight->ciBodySerial=3;
		pHeight->hTransitNode	=0xD2;
		pHeight->hPostLegRod	=0x680;
		pHeight->hLowestBaseNode=0xDA;
		//塔身节间
		pSection=listExistSections.Append();//1-3
		pSection->hTopNode=0x32;
		pSection->hBtmNode=0xAE;
		pSection->hQuadRod=0x82B;
		pSection=listExistSections.Append();//1-3
		pSection->hTopNode=0xAE;
		pSection->hBtmNode=0xB2;
		pSection->hQuadRod=0x82C;
		pSection=listExistSections.Append();//3
		pSection->hTopNode=0xB2;
		pSection->hBtmNode=0xCE;
		pSection->hQuadRod=0x83B;
		pSection=listExistSections.Append();//3
		pSection->hTopNode=0xCE;
		pSection->hBtmNode=0xD2;
		pSection->hQuadRod=0x83C;
		//初始化塔身主材投影基准线
		xSlope.hTopNode=0x32;
		xSlope.hBtmNode=0xDA;
	}
	else if(idTowerModelTmpl==TYPE_SJB)
	{
		dfSlopeGradient=0.16;
		uiTopWidth=7200;
		ziTopPlane=43900;
		ziHeight0Plane=40700;
		//初始化已有各呼高的腿身过渡平面信息
		hashExistHeights.Empty();
		pHeight=hashExistHeights.Add(30000);	//30m
		pHeight->ciBodySerial=1;
		pHeight->hTransitNode=0x72;
		pHeight->hPostLegRod =0x5B7;
		pHeight->hLowestBaseNode=0x76;
		pHeight=hashExistHeights.Add(36000);	//36m
		pHeight->ciBodySerial=2;
		pHeight->hTransitNode	=0x86;
		pHeight->hPostLegRod	=0x62B;
		pHeight->hLowestBaseNode=0x8A;
		pHeight=hashExistHeights.Add(42000);	//36m
		pHeight->ciBodySerial=3;
		pHeight->hTransitNode	=0xF2;
		pHeight->hPostLegRod	=0x66B;
		pHeight->hLowestBaseNode=0xFA;
		//塔身节间
		pSection=listExistSections.Append();//1-3
		pSection->hTopNode=0x2E;
		pSection->hBtmNode=0x6A;
		pSection->hQuadRod=0x55B;
		pSection=listExistSections.Append();//3
		pSection->hTopNode=0x6A;
		pSection->hBtmNode=0xF2;
		pSection->hQuadRod=0x643;
		//初始化塔身主材投影基准线
		xSlope.hTopNode=0x2E;
		xSlope.hBtmNode=0xFA;
	}
	else if(idTowerModelTmpl==TYPE_SJC)
	{
		dfSlopeGradient=0.13;
		uiTopWidth=6200;
		ziTopPlane=34000;
		ziHeight0Plane=31000;
		//初始化已有各呼高的腿身过渡平面信息
		hashExistHeights.Empty();
		pHeight=hashExistHeights.Add(30000);	//30m
		pHeight->ciBodySerial=1;
		pHeight->hTransitNode=0x5D;
		pHeight->hPostLegRod =0x1F0D;
		pHeight->hLowestBaseNode=0x85;
		pHeight=hashExistHeights.Add(36000);	//36m
		pHeight->ciBodySerial=2;
		pHeight->hTransitNode	=0x95;
		pHeight->hPostLegRod	=0x1F55;
		pHeight->hLowestBaseNode=0x31;
		//塔身节间
		pSection=listExistSections.Append();//2
		pSection->hTopNode=0x2D;
		pSection->hBtmNode=0x8D;
		pSection->hQuadRod=0x1F49;
		pSection=listExistSections.Append();//2
		pSection->hTopNode=0x8D;
		pSection->hBtmNode=0x95;
		pSection->hQuadRod=0x1F4F;
		//初始化塔身主材投影基准线
		xSlope.hTopNode=0x2D;
		xSlope.hBtmNode=0x31;
	}
	else if(idTowerModelTmpl==TYPE_ZBC)
	{
		dfSlopeGradient=0.11;
		uiTopWidth=3400;
		ziTopPlane=20400;
		ziHeight0Plane=5600;
		//初始化已有各呼高的腿身过渡平面信息
		hashExistHeights.Empty();
		pHeight=hashExistHeights.Add(27000);	//27m
		pHeight->ciBodySerial=1;
		pHeight->hTransitNode=0x72;
		pHeight->hPostLegRod =0x48E;
		pHeight->hLowestBaseNode=0x76;
		pHeight=hashExistHeights.Add(33000);	//33m
		pHeight->ciBodySerial=2;
		pHeight->hTransitNode	=0x86;
		pHeight->hPostLegRod	=0x4EA;
		pHeight->hLowestBaseNode=0x8A;
		pHeight=hashExistHeights.Add(36000);	//36m
		pHeight->ciBodySerial=3;
		pHeight->hTransitNode=0xAA;
		pHeight->hPostLegRod =0x56A;
		pHeight->hLowestBaseNode=0xF6;
		pHeight=hashExistHeights.Add(39000);	//39m
		pHeight->ciBodySerial=4;
		pHeight->hTransitNode	=0xB6;
		pHeight->hPostLegRod	=0x632;
		pHeight->hLowestBaseNode=0x112;
		pHeight=hashExistHeights.Add(42000);	//42m
		pHeight->ciBodySerial=5;
		pHeight->hTransitNode	=0xC2;
		pHeight->hPostLegRod	=0x706;
		pHeight->hLowestBaseNode=0x66;
		//塔身节间
		pSection=listExistSections.Append();//1-5
		pSection->hTopNode=0x60;
		pSection->hBtmNode=0x72;
		pSection->hQuadRod=0x432;
		pSection=listExistSections.Append();//5
		pSection->hTopNode=0x72;
		pSection->hBtmNode=0xC2;
		pSection->hQuadRod=0x686;
		//初始化塔身主材投影基准线
		xSlope.hTopNode=0x60;
		xSlope.hBtmNode=0x66;
	}
	else if(idTowerModelTmpl==TYPE_ZC)
	{
		dfSlopeGradient=0.1;
		uiTopWidth=5500;
		ziTopPlane=30800;
		ziHeight0Plane=6800;
		//初始化已有各呼高的腿身过渡平面信息
		hashExistHeights.Empty();
		pHeight=hashExistHeights.Add(39000);	//39
		pHeight->ciBodySerial=1;
		pHeight->hTransitNode=0xAA;
		pHeight->hPostLegRod =0x4F8;
		pHeight->hLowestBaseNode=0xAE;
		pHeight=hashExistHeights.Add(42000);	//42
		pHeight->ciBodySerial=2;
		pHeight->hTransitNode	=0xBA;
		pHeight->hPostLegRod	=0x510;
		pHeight->hLowestBaseNode=0xBE;
		pHeight=hashExistHeights.Add(45000);	//45
		pHeight->ciBodySerial=3;
		pHeight->hTransitNode=0xC6;
		pHeight->hPostLegRod =0x528;
		pHeight->hLowestBaseNode=0xCA;
		pHeight=hashExistHeights.Add(48000);	//48
		pHeight->ciBodySerial=4;
		pHeight->hTransitNode	=0xD2;
		pHeight->hPostLegRod	=0x540;
		pHeight->hLowestBaseNode=0xD6;
		pHeight=hashExistHeights.Add(54000);	//54
		pHeight->ciBodySerial=5;
		pHeight->hTransitNode	=0xE2;
		pHeight->hPostLegRod	=0x558;
		pHeight->hLowestBaseNode=0xE6;
		pHeight=hashExistHeights.Add(60000);	//60
		pHeight->ciBodySerial=6;
		pHeight->hTransitNode	=0xF2;
		pHeight->hPostLegRod	=0x570;
		pHeight->hLowestBaseNode=0xF6;
		pHeight=hashExistHeights.Add(66000);	//66m
		pHeight->ciBodySerial=7;
		pHeight->hTransitNode	=0xFE;
		pHeight->hPostLegRod	=0x594;
		pHeight->hLowestBaseNode=0x3A;
		//塔身节间
		pSection=listExistSections.Append();//3-7
		pSection->hTopNode=0x36;
		pSection->hBtmNode=0x96;
		pSection->hQuadRod=0x42C;
		pSection=listExistSections.Append();//3-7
		pSection->hTopNode=0x96;
		pSection->hBtmNode=0x9A;
		pSection->hQuadRod=0x430;
		pSection=listExistSections.Append();//6-7
		pSection->hTopNode=0x9A;
		pSection->hBtmNode=0x9E;
		pSection->hQuadRod=0x45C;
		pSection=listExistSections.Append();//7
		pSection->hTopNode=0x9E;
		pSection->hBtmNode=0xA2;
		pSection->hQuadRod=0x468;
		pSection=listExistSections.Append();//7
		pSection->hTopNode=0xA2;
		pSection->hBtmNode=0xFE;
		pSection->hQuadRod=0x4EC;
		//初始化塔身主材投影基准线
		xSlope.hTopNode=0x36;
		xSlope.hBtmNode=0x3A;
	}
	else if(idTowerModelTmpl==TYPE_ZM)
	{
		dfSlopeGradient=0.1;
		uiTopWidth=3200;
		ziTopPlane=12600;
		ziHeight0Plane=9500;
		//初始化已有各呼高的腿身过渡平面信息
		hashExistHeights.Empty();
		pHeight=hashExistHeights.Add(27000);	//27m
		pHeight->ciBodySerial=1;
		pHeight->hTransitNode=0x85;
		pHeight->hPostLegRod =0x3f4;
		pHeight->hLowestBaseNode=0x89;
		pHeight=hashExistHeights.Add(33000);	//33m
		pHeight->ciBodySerial=2;
		pHeight->hTransitNode	=0x8d;
		pHeight->hPostLegRod	=0x450;
		pHeight->hLowestBaseNode=0x95;
		pHeight=hashExistHeights.Add(39000);	//39m
		pHeight->ciBodySerial=3;
		pHeight->hTransitNode=0x9d;
		pHeight->hPostLegRod =0x4b8;
		pHeight->hLowestBaseNode=0xa5;
		pHeight=hashExistHeights.Add(45000);	//45m
		pHeight->ciBodySerial=4;
		pHeight->hTransitNode	=0xB5;
		pHeight->hPostLegRod	=0x570;
		pHeight->hLowestBaseNode=0xbd;
		//塔身节间
		pSection=listExistSections.Append();//1-4
		pSection->hTopNode=0x69;
		pSection->hBtmNode=0x75;
		pSection->hQuadRod=0x368;
		pSection=listExistSections.Append();//2-4
		pSection->hTopNode=0x75;
		pSection->hBtmNode=0x7d;
		pSection->hQuadRod=0x38c;
		pSection=listExistSections.Append();//4
		pSection->hTopNode=0x7d;
		pSection->hBtmNode=0xb5;
		pSection->hQuadRod=0x4c4;
		//初始化塔身主材投影基准线
		xSlope.hTopNode=0x69;
		xSlope.hBtmNode=0xBD;
	}
	else if(idTowerModelTmpl==TYPE_SZ)
	{
		dfSlopeGradient=0.1;
		uiTopWidth=4400;
		ziTopPlane=29600;
		ziHeight0Plane=26600;
		//初始化已有各呼高的腿身过渡平面信息
		pHeight=hashExistHeights.Add(36000);	//36m
		pHeight->ciBodySerial=2;
		pHeight->hTransitNode=0xE1;
		pHeight->hLowestBaseNode=0xF1;
		pHeight->hPostLegRod=0x62F;
		pHeight=hashExistHeights.Add(39000);	//39m
		pHeight->ciBodySerial=3;
		pHeight->hTransitNode=0x101;
		pHeight->hLowestBaseNode=0x109;
		pHeight->hPostLegRod=0x6B7;
		pHeight=hashExistHeights.Add(42000);	//42m
		pHeight->ciBodySerial=4;
		pHeight->hTransitNode=0x115;
		pHeight->hLowestBaseNode=0x125;
		pHeight->hPostLegRod=0x743;
		pHeight=hashExistHeights.Add(45000);	//45m
		pHeight->ciBodySerial=5;
		pHeight->hTransitNode=0x12D;
		pHeight->hLowestBaseNode=0x13D;
		pHeight->hPostLegRod=0x7CF;
		pHeight=hashExistHeights.Add(48000);	//48m
		pHeight->ciBodySerial=6;
		pHeight->hTransitNode=0x145;
		pHeight->hLowestBaseNode=0x155;
		pHeight->hPostLegRod=0x85B;
		//塔身节间
		pSection=listExistSections.Append();	//1-6
		pSection->hTopNode=0x5D;
		pSection->hBtmNode=0xA9;
		pSection->hQuadRod=0x4E5;
		pSection=listExistSections.Append();	//4-6
		pSection->hTopNode=0xA9;
		pSection->hBtmNode=0xB1;
		pSection->hQuadRod=0x9A4;
		pSection=listExistSections.Append();	//6
		pSection->hTopNode=0xB1;
		pSection->hBtmNode=0xBD;
		pSection->hQuadRod=0x9A5;
		pSection=listExistSections.Append();	//6
		pSection->hTopNode=0xBD;
		pSection->hBtmNode=0x141;
		pSection->hQuadRod=0x7DF;
		pSection=listExistSections.Append();	//6
		pSection->hTopNode=0x141;
		pSection->hBtmNode=0x145;
		pSection->hQuadRod=0x7E3;
		pSection->idPanelType=4;
		//初始化塔身主材投影基准线
		xSlope.hTopNode=0x5D;
		xSlope.hBtmNode=0x155;
	}
	else if(idTowerModelTmpl==TYPE_JC)
	{
		dfSlopeGradient=0.12;
		uiTopWidth=4800;
		ziTopPlane=20500;
		ziHeight0Plane=16000;
		//初始化已有各呼高的腿身过渡平面信息
		hashExistHeights.Empty();
		pHeight=hashExistHeights.Add(39000);	//39m
		pHeight->ciBodySerial=1;
		pHeight->hTransitNode=0x7C;
		pHeight->hLowestBaseNode=0x84;
		pHeight->hPostLegRod =0x43A;
		pHeight=hashExistHeights.Add(42000);	//42m
		pHeight->ciBodySerial=2;
		pHeight->hTransitNode=0x90;
		pHeight->hLowestBaseNode=0x98;
		pHeight->hPostLegRod=0x4AE;
		pHeight=hashExistHeights.Add(45000);	//45m
		pHeight->ciBodySerial=3;
		pHeight->hTransitNode=0xA4;
		pHeight->hLowestBaseNode=0xAC;
		pHeight->hPostLegRod=0x536;
		pHeight=hashExistHeights.Add(48000);	//48m
		pHeight->ciBodySerial=4;
		pHeight->hTransitNode=0xC0;
		pHeight->hLowestBaseNode=0xC8;
		pHeight->hPostLegRod=0x5AE;
		pHeight=hashExistHeights.Add(51000);	//51m
		pHeight->ciBodySerial=5;
		pHeight->hTransitNode=0xD8;
		pHeight->hLowestBaseNode=0xE0;
		pHeight->hPostLegRod=0x66E;
		//塔身节间
		pSection=listExistSections.Append();	//1-5
		pSection->hTopNode=0x30;
		pSection->hBtmNode=0x70;
		pSection->hQuadRod=0x3C2;
		pSection=listExistSections.Append();	//5
		pSection->hTopNode=0x70;
		pSection->hBtmNode=0xD0;
		pSection->hQuadRod=0x4C2;
		pSection=listExistSections.Append();	//5
		pSection->hTopNode=0xD0;
		pSection->hBtmNode=0xD4;
		pSection->hQuadRod=0x5CA;
		pSection=listExistSections.Append();	//5
		pSection->hTopNode=0xD4;
		pSection->hBtmNode=0xD8;
		pSection->hQuadRod=0x5E6;
		//初始化塔身主材投影基准线
		xSlope.hTopNode=0x30;
		xSlope.hBtmNode=0xE0;
	}
	return true;
}
bool CTrunkBodyPlanner::AutoRetrieveTrunkInfo(CTower* pTower)
{
	CLDSNode* pTopNode,*pBtmNode,*pMirNode;
	pTopNode=pTower->FromNodeHandle(xSlope.harrTopNodes[0]);
	if(xSlope.harrTopNodes[1]==0&&(pMirNode=pTopNode->GetMirNode(MIRMSG(2)))!=NULL)
		xSlope.harrTopNodes[1]=pMirNode->handle;
	if(xSlope.harrTopNodes[2]==0&&(pMirNode=pTopNode->GetMirNode(MIRMSG(1)))!=NULL)
		xSlope.harrTopNodes[2]=pMirNode->handle;
	if(xSlope.harrTopNodes[3]==0&&(pMirNode=pTopNode->GetMirNode(MIRMSG(3)))!=NULL)
		xSlope.harrTopNodes[3]=pMirNode->handle;
	pBtmNode=pTower->FromNodeHandle(xSlope.harrBtmNodes[0]);
	if(xSlope.harrBtmNodes[1]==0&&(pMirNode=pBtmNode->GetMirNode(MIRMSG(2)))!=NULL)
		xSlope.harrBtmNodes[1]=pMirNode->handle;
	if(xSlope.harrBtmNodes[2]==0&&(pMirNode=pBtmNode->GetMirNode(MIRMSG(1)))!=NULL)
		xSlope.harrBtmNodes[2]=pMirNode->handle;
	if(xSlope.harrBtmNodes[3]==0&&(pMirNode=pBtmNode->GetMirNode(MIRMSG(3)))!=NULL)
		xSlope.harrBtmNodes[3]=pMirNode->handle;

	for(EXIST_SECTION* pSection=listExistSections.EnumObjectFirst();pSection;pSection=listExistSections.EnumObjectNext())
	{
		CLDSLinePart* pRod=pTower->FromRodHandle(pSection->harrRod[0]);
		CLDSLinePart* pMirRod=NULL;
		pTopNode=pTower->FromNodeHandle(pSection->hTopNode);
		pBtmNode=pTower->FromNodeHandle(pSection->hBtmNode);
		pSection->ziTopPlane=ftoi(pTopNode->xOriginalPos.z);// 27950;
		pSection->dfTopWidth=pTopNode->xOriginalPos.x*2;
		pSection->ziBtmPlane=ftoi(pBtmNode->xOriginalPos.z);// 32200;
		pSection->dfBtmWidth=pBtmNode->xOriginalPos.x*2;
		pSection->cfgword=pRod->cfgword;
		if(pSection->harrRod[1]==0&&(pMirRod=pRod->GetMirRod(MIRMSG(2)))!=NULL)
			pSection->harrRod[1]=pMirRod->handle;
		if(pSection->harrRod[2]==0&&(pMirRod=pRod->GetMirRod(MIRMSG(1)))!=NULL)
			pSection->harrRod[2]=pMirRod->handle;
		if(pSection->harrRod[3]==0&&(pMirRod=pRod->GetMirRod(MIRMSG(3)))!=NULL)
			pSection->harrRod[3]=pMirRod->handle;
	}
	for(EXIST_HEIGHT* pExistHeight=hashExistHeights.GetFirst();pExistHeight;pExistHeight=hashExistHeights.GetNext())
	{
		for(CLDSModule* pModule=pTower->Module.GetFirst();pModule;pModule=pTower->Module.GetNext())
		{
			if(pModule->idBodyNo==pExistHeight->ciBodySerial)
				pExistHeight->hRelaHeightModule=pModule->handle;
		}
	}
	return true;
}
void CTrunkBodyPlanner::AppendTransitHoriSectionToModel(HEIGHTGROUP* pHeight,CTower* pTower)
{
	CLDSNodePtr pxarrCornerNodes[4]={NULL};
	typedef CLDSLineAngle* CLDSLineAnglePtr;
	CLDSLineAnglePtr pxarrQuadAngles[4]={NULL};
	int j,signx[4]={1,-1,1,-1},signy[4]={1,1,-1,-1};
	//添加主材节点及角钢
	CXhChar16 szLayer("LP1");
	GEPOINT xCorner(pHeight->xLegTopPlane.dfWidth*0.5,pHeight->xLegTopPlane.dfWidth*0.5,pHeight->xLegTopPlane.ziPlane);
	TRUNK_NODE* pHeadTrunkNode=this->hashTrunkNodes.GetValue(pHeight->xLegTopPlane.ziPlane);
	TRUNK_NODE* pTrunkNode=pHeadTrunkNode;
	while(pTrunkNode!=NULL){
		if(!pTrunkNode->cfgword.IsHasBodyNo(pHeight->ciBodySerial))
			pTrunkNode=pTrunkNode->linknext;
		else
			break;
	};
	if(pTrunkNode==NULL)
	{
		for(j=0;j<4;j++)
		{
			pxarrCornerNodes[j]=pTower->Node.append();
			szLayer.Printf("SP%d",j+1);
			pxarrCornerNodes[j]->SetLayer(szLayer);
			pxarrCornerNodes[j]->cfgword.Clear();
			pxarrCornerNodes[j]->cfgword.SetBodyLegs(pHeight->ciBodySerial);
			pxarrCornerNodes[j]->m_cPosCalType=CLDSNode::COORD_Z_SPECIFIED;
			pxarrCornerNodes[j]->SetPosition(xCorner.x*signx[j],xCorner.y*signy[j],xCorner.z);
			pxarrCornerNodes[j]->arrRelationNode[0]=this->xSlope.harrTopNodes[j];
			pxarrCornerNodes[j]->arrRelationNode[1]=this->xSlope.harrBtmNodes[j];
		}
	}
	else
	{
		for(j=0;j<4;j++)
			pxarrCornerNodes[j]=pTower->FromNodeHandle(pTrunkNode->harrQuadNodes[j]);
	}
	//添加腿身过渡横隔面
	CLDSLineAngle* pHoriFrontAngle=(CLDSLineAngle*)pTower->AppendPart(CLS_LINEANGLE);
	pHoriFrontAngle->pStart=pxarrCornerNodes[1];
	pHoriFrontAngle->pEnd  =pxarrCornerNodes[0];
	pHoriFrontAngle->SetLayer("SHQ");
	pHoriFrontAngle->cfgword.Clear();
	pHoriFrontAngle->cfgword.SetBodyLegs(pHeight->ciBodySerial);
	pHoriFrontAngle->SetStart(pHoriFrontAngle->pStart->xOriginalPos);
	pHoriFrontAngle->SetEnd  (pHoriFrontAngle->pEnd  ->xOriginalPos);
	
	CLDSLineAngle* pHoriBackAngle=(CLDSLineAngle*)pTower->AppendPart(CLS_LINEANGLE);
	pHoriBackAngle->pStart=pxarrCornerNodes[3];
	pHoriBackAngle->pEnd  =pxarrCornerNodes[2];
	pHoriBackAngle->SetLayer("SHH");
	pHoriBackAngle->cfgword.Clear();
	pHoriBackAngle->cfgword.SetBodyLegs(pHeight->ciBodySerial);
	pHoriBackAngle->SetStart(pHoriBackAngle->pStart->xOriginalPos);
	pHoriBackAngle->SetEnd  (pHoriBackAngle->pEnd  ->xOriginalPos);
	
	CLDSLineAngle* pHoriRightAngle=(CLDSLineAngle*)pTower->AppendPart(CLS_LINEANGLE);
	pHoriRightAngle->pStart=pxarrCornerNodes[0];
	pHoriRightAngle->pEnd  =pxarrCornerNodes[2];
	pHoriRightAngle->SetLayer("SHY");
	pHoriRightAngle->cfgword.Clear();
	pHoriRightAngle->cfgword.SetBodyLegs(pHeight->ciBodySerial);
	pHoriRightAngle->SetStart(pHoriRightAngle->pStart->xOriginalPos);
	pHoriRightAngle->SetEnd  (pHoriRightAngle->pEnd  ->xOriginalPos);

	CLDSLineAngle* pHoriLeftAngle=(CLDSLineAngle*)pTower->AppendPart(CLS_LINEANGLE);
	pHoriLeftAngle->pStart=pxarrCornerNodes[1];
	pHoriLeftAngle->pEnd  =pxarrCornerNodes[3];
	pHoriLeftAngle->SetLayer("SHZ");
	pHoriLeftAngle->cfgword.Clear();
	pHoriLeftAngle->cfgword.SetBodyLegs(pHeight->ciBodySerial);
	pHoriLeftAngle->SetStart(pHoriLeftAngle->pStart->xOriginalPos);
	pHoriLeftAngle->SetEnd  (pHoriLeftAngle->pEnd  ->xOriginalPos);
	//添加横隔面中间点
	CLDSNode* pFrontMidNode=pTower->AppendNode();
	pFrontMidNode->m_cPosCalType=CLDSNode::COORD_SCALE;
	pFrontMidNode->attach_scale=0.5;
	pFrontMidNode->hFatherPart=pHoriFrontAngle->handle;
	pFrontMidNode->cfgword.Clear();
	pFrontMidNode->cfgword.SetBodyLegs(pHeight->ciBodySerial);
	pFrontMidNode->arrRelationNode[0]=pxarrCornerNodes[0]->handle;
	pFrontMidNode->arrRelationNode[1]=pxarrCornerNodes[1]->handle;
	pFrontMidNode->SetPosition(0,pxarrCornerNodes[0]->xOriginalPos.y,pxarrCornerNodes[0]->xOriginalPos.z);
	CLDSNode* pRightMidNode=pTower->AppendNode();
	pRightMidNode->m_cPosCalType=CLDSNode::COORD_SCALE;
	pRightMidNode->attach_scale=0.5;
	pRightMidNode->hFatherPart=pHoriRightAngle->handle;
	pRightMidNode->cfgword.Clear();
	pRightMidNode->cfgword.SetBodyLegs(pHeight->ciBodySerial);
	pRightMidNode->arrRelationNode[0]=pxarrCornerNodes[0]->handle;
	pRightMidNode->arrRelationNode[1]=pxarrCornerNodes[2]->handle;
	pRightMidNode->SetPosition(pxarrCornerNodes[0]->xOriginalPos.x,0,pxarrCornerNodes[0]->xOriginalPos.z);
	CLDSNode* pBackMidNode=pTower->AppendNode();
	pBackMidNode->m_cPosCalType=CLDSNode::COORD_SCALE;
	pBackMidNode->attach_scale=0.5;
	pBackMidNode->hFatherPart=pHoriBackAngle->handle;
	pBackMidNode->cfgword.Clear();
	pBackMidNode->cfgword.SetBodyLegs(pHeight->ciBodySerial);
	pBackMidNode->arrRelationNode[0]=pxarrCornerNodes[3]->handle;
	pBackMidNode->arrRelationNode[1]=pxarrCornerNodes[2]->handle;
	pBackMidNode->SetPosition(0,-pxarrCornerNodes[0]->xOriginalPos.y,pxarrCornerNodes[0]->xOriginalPos.z);
	CLDSNode* pLeftMidNode=pTower->AppendNode();
	pLeftMidNode->m_cPosCalType=CLDSNode::COORD_SCALE;
	pLeftMidNode->attach_scale=0.5;
	pLeftMidNode->hFatherPart=pHoriRightAngle->handle;
	pLeftMidNode->cfgword.Clear();
	pLeftMidNode->cfgword.SetBodyLegs(pHeight->ciBodySerial);
	pLeftMidNode->arrRelationNode[0]=pxarrCornerNodes[1]->handle;
	pLeftMidNode->arrRelationNode[1]=pxarrCornerNodes[3]->handle;
	pLeftMidNode->SetPosition(-pxarrCornerNodes[0]->xOriginalPos.x,0,pxarrCornerNodes[0]->xOriginalPos.z);
	//添加腿身过渡隔面横材中间节点
	pHeight->xSublegInfo.xFront.pNode=pFrontMidNode;
	pHeight->xSublegInfo.xRight.pNode=pRightMidNode;
	pHeight->xSublegInfo.xBack.pNode =pBackMidNode;
	pHeight->xSublegInfo.xLeft.pNode =pLeftMidNode;
}
void CTrunkBodyPlanner::AppendSectionToModel(PLAN_SECTION* pSection,HEIGHTGROUP* pBelongHeight,CTower* pTower)
{
	CLDSNodePtr pxarrTopNodes[4]={NULL},pxarrBtmNodes[4]={NULL};
	typedef CLDSLineAngle* CLDSLineAnglePtr;
	CLDSLineAnglePtr pxarrQuadAngles[4]={NULL};
	int j,signx[4]={1,-1,1,-1},signy[4]={1,1,-1,-1};
	//添加主材节点及角钢
	CXhChar16 szLayer("LP1");
	GEPOINT xCorner(pSection->dfBtmWidth*0.5,pSection->dfBtmWidth*0.5,pSection->ziBtmPlane);
	for(j=0;j<4;j++)
	{
		pxarrBtmNodes[j]=pTower->Node.append();
		if(pSection->idPanelType==TRUNK_SEGMENT::SECT_SUBLEG)
			szLayer.Printf("LP%d",j+1);
		else
			szLayer.Printf("SP%d",j+1);
		pxarrBtmNodes[j]->SetLayer(szLayer);
		pxarrBtmNodes[j]->cfgword=pSection->cfgword;
		pxarrBtmNodes[j]->m_cPosCalType=CLDSNode::COORD_Z_SPECIFIED;
		pxarrBtmNodes[j]->SetPosition(xCorner.x*signx[j],xCorner.y*signy[j],xCorner.z);
		pxarrBtmNodes[j]->arrRelationNode[0]=this->xSlope.harrTopNodes[j];
		pxarrBtmNodes[j]->arrRelationNode[1]=this->xSlope.harrBtmNodes[j];
	}
	TRUNK_NODE* pHeadTrunkNode=this->hashTrunkNodes.GetValue(pSection->xTopPlane.ziPlane);
	TRUNK_NODE* pTrunkNode=pHeadTrunkNode;
	while(pTrunkNode!=NULL){
		if(!pTrunkNode->cfgword.And(pSection->cfgword))
			pTrunkNode=pTrunkNode->linknext;
		else
			break;
	};
	if(pTrunkNode==NULL)
	{
		GEPOINT xCorner(pSection->xTopPlane.dfWidth*0.5,pSection->xTopPlane.dfWidth*0.5,pSection->xTopPlane.ziPlane);
		for(j=0;j<4;j++)
		{
			pxarrTopNodes[j]=pTower->Node.append();
			szLayer.Printf("SP%d",j+1);
			pxarrTopNodes[j]->SetLayer(szLayer);
			pxarrTopNodes[j]->cfgword=pSection->cfgword;
			pxarrTopNodes[j]->UnifiedCfgword();
			pxarrTopNodes[j]->m_cPosCalType=CLDSNode::COORD_Z_SPECIFIED;
			pxarrTopNodes[j]->SetPosition(xCorner.x*signx[j],xCorner.y*signy[j],xCorner.z);
			pxarrTopNodes[j]->arrRelationNode[0]=this->xSlope.harrTopNodes[j];
			pxarrTopNodes[j]->arrRelationNode[1]=this->xSlope.harrBtmNodes[j];
		}
	}
	else
	{
		for(j=0;j<4;j++)
			pxarrTopNodes[j]=pTower->FromNodeHandle(pTrunkNode->harrQuadNodes[j]);
	}
	for(j=0;j<4;j++)
	{
		pxarrQuadAngles[j]=(CLDSLineAngle*)pTower->AppendPart(CLS_LINEANGLE);
		pxarrQuadAngles[j]->pStart=pxarrTopNodes[j];
		pxarrQuadAngles[j]->pEnd  =pxarrBtmNodes[j];
		if(pSection->idPanelType==TRUNK_SEGMENT::SECT_SUBLEG)
			szLayer.Printf("LZ%d",j+1);	//接腿
		else
			szLayer.Printf("SZ%d",j+1);	//接腿
		pxarrQuadAngles[j]->SetLayer(szLayer);
		pxarrQuadAngles[j]->cfgword=pSection->cfgword;
		if(pSection->idPanelType!=TRUNK_SEGMENT::SECT_SUBLEG)
			pxarrQuadAngles[j]->UnifiedCfgword();
		pxarrQuadAngles[j]->SetStart(pxarrTopNodes[j]->xOriginalPos);
		pxarrQuadAngles[j]->SetEnd  (pxarrBtmNodes[j]->xOriginalPos);
		//重置角钢肢法线pxarrQuadAngles[j]->des_norm_x.hViceJg=
		//重置角钢端点位置
	}
	if(pSection->idPanelType==TRUNK_SEGMENT::SECT_SUBLEG)
	{
		//添加接腿八字铁斜材
		CLDSNodePtr parrTopNodes[4]={
			pBelongHeight->xSublegInfo.xFront.pNode,pBelongHeight->xSublegInfo.xRight.pNode,
			pBelongHeight->xSublegInfo.xBack.pNode ,pBelongHeight->xSublegInfo.xLeft.pNode};
		BYTE iarrBtmNodeIndex[8]={1,0,0,2,2,3,3,1};
		for(j=0;j<4;j++)
		{
			CLDSLineAngle* pPieLegAngle=(CLDSLineAngle*)pTower->AppendPart(CLS_LINEANGLE);
			pPieLegAngle->pStart=parrTopNodes[j];
			pPieLegAngle->pEnd=pxarrBtmNodes[iarrBtmNodeIndex[j*2]];
			pPieLegAngle->cfgword=pPieLegAngle->pEnd->cfgword;
			pPieLegAngle->SetStart(pPieLegAngle->pStart->xOriginalPos);
			pPieLegAngle->SetEnd(pPieLegAngle->pEnd->xOriginalPos);
			pPieLegAngle->ciLegQuad=iarrBtmNodeIndex[j*2]+1;
			pPieLegAngle->SetLayer("LX1");
			pPieLegAngle->layer(2)='0'+pPieLegAngle->ciLegQuad;
			CLDSLineAngle* pNaiLegAngle=(CLDSLineAngle*)pTower->AppendPart(CLS_LINEANGLE);
			pNaiLegAngle->pStart=parrTopNodes[j];
			pNaiLegAngle->pEnd=pxarrBtmNodes[iarrBtmNodeIndex[j*2+1]];
			pNaiLegAngle->cfgword=pNaiLegAngle->pEnd->cfgword;
			pNaiLegAngle->SetStart(pNaiLegAngle->pStart->xOriginalPos);
			pNaiLegAngle->SetEnd(pNaiLegAngle->pEnd->xOriginalPos);
			pNaiLegAngle->ciLegQuad=iarrBtmNodeIndex[j*2+1]+1;
			pNaiLegAngle->SetLayer("LX1");
			pNaiLegAngle->layer(2)='0'+pNaiLegAngle->ciLegQuad;
		}
	}
	//else if(pSection->idPanelType==TRUNK_SEGMENT::SECT_HALF_UP)
	//{
	//}
	else if(pSection->idPanelType==TRUNK_SEGMENT::SECT_HALF_DOWN)
	{
	}
	else //if(pSection->idPanelType==TRUNK_SEGMENT::SECT_WHOLE)
	{
	}
}
bool CTrunkBodyPlanner::UpdateTowerTrunk(CTower* pTower,CHashListEx<HEIGHTGROUP>* pHashPlanHeights)
{
	AutoRetrieveTrunkInfo(pTower);
	//1.按要求规划塔身
	bool bTopHalfSection=false;
	CHashListEx<HEIGHTGROUP> hashPlanHeights;
	if(pHashPlanHeights==NULL)
		pHashPlanHeights=&hashPlanHeights;
	CLogErrorLife life(&logerr);
	if(!Planning(pHashPlanHeights,bTopHalfSection))
		return false;
	CLDSNode* pNode;
	CLDSLinePart* pRod;
	EXIST_HEIGHT*  pExistHeight=hashExistHeights.GetFirst();
	EXIST_SECTION* pExistSect  =this->listExistSections.EnumObjectFirst();
	//匹配呼高后，重置配材号
	HEIGHTGROUP* pHeight;
	EXIST_SECTION* pSection=NULL;
	BYTE ciMinIdleBodyNo=1;
	long hExcludeTransitNode=0;	//当前最高呼高中腿身过渡横隔面应排除，只保留四个主材上的角节点
	for(pExistHeight=hashExistHeights.GetFirst();pExistHeight;pExistHeight=hashExistHeights.GetNext())
	{
		if(pExistHeight->ciBodySerial+1>ciMinIdleBodyNo)
		{
			hExcludeTransitNode=pExistHeight->hTransitNode;
			ciMinIdleBodyNo=pExistHeight->ciBodySerial+1;
		}
	}
	BYTE idHightestBody=ciMinIdleBodyNo-1;
	CXhSimpleList<TRUNK_SEGMENT> listInstSegments;
	for(pHeight=pHashPlanHeights->GetTail();pHeight;pHeight=pHashPlanHeights->GetPrev())
	{
		if((pExistHeight=hashExistHeights.GetValue(pHeight->uiNamedHeight))!=NULL)
		{
			pHeight->ciBodySerial=pExistHeight->ciBodySerial;
			pHeight->hRelaHeightModule=pExistHeight->hRelaHeightModule;
			//设计补充旧呼高接腿
			continue;	//已存在该呼高
		}
		//筛选过往类似呼高的塔身节间中可用部分
		for(pSection=this->listExistSections.EnumObjectFirst();pSection;pSection=this->listExistSections.EnumObjectNext())
		{
			if(!pSection->cfgword.IsHasBodyNo(idHightestBody))
				continue;
			if(pSection->ziBtmPlane<pHeight->xSublegInfo.ziMinTopPlane)
			{
				pHeight->ziCurrPlane=pSection->ziBtmPlane;
				continue;
			}
			if(pSection->ziBtmPlane>=pHeight->xSublegInfo.ziMinTopPlane&&pSection->ziBtmPlane<=pHeight->xSublegInfo.ziMaxTopPlane)
			{
				pHeight->ziCurrPlane=pSection->ziBtmPlane;
				break;
			}
			else //if(pSection->ziBtmPlane>pHeight->xSublegInfo.ziMaxTopPlane)
			{	//再加当前节间就一步跨过了过渡区，此时只能依赖减少接腿数或调整上一节间位置来处理
				//优先调整接腿数
				if(pSection->ziBtmPlane-pHeight->xSublegInfo.ziMaxTopPlane<(int)(pHeight->xSublegInfo.wiLevelDiff*(pHeight->xarrSubLegs.Count-1)))
				{
					int wiReduceLegCount=(pSection->ziBtmPlane-pHeight->xSublegInfo.ziMaxTopPlane)/pHeight->xSublegInfo.wiLevelDiff;
					if((pSection->ziBtmPlane-pHeight->xSublegInfo.ziMaxTopPlane)%pHeight->xSublegInfo.wiLevelDiff>0)
						wiReduceLegCount+=1;
					pHeight->xSublegInfo.ziMaxTopPlane+=wiReduceLegCount*pHeight->xSublegInfo.wiLevelDiff;
					pHeight->xarrSubLegs.ReSize(pHeight->xarrSubLegs.Count-wiReduceLegCount);
					pHeight->xLegTopPlane.ziPlane=pHeight->ziCurrPlane=pSection->ziBtmPlane;
					pHeight->xLegTopPlane.dfWidth=pHeight->CalPlaneWidth(pSection->ziBtmPlane);
				}
				else	//调整接腿数失败时，则直接调整原有节间
					break;
			}
		}
		//新增加呼高
		CLDSModule* pHeightModule=pTower->Module.append();
		pHeight->ciBodySerial=ciMinIdleBodyNo;
		CXhString(pHeightModule->description,51).Printf("%dm",pHeight->uiNamedHeight/1000);
		//TODO:检验ciMinIdleBodyNo是否重号
		pHeightModule->iNo=ciMinIdleBodyNo;
		pHeightModule->SetBodyNo(ciMinIdleBodyNo);
		pHeight->hRelaHeightModule=pHeightModule->handle;
		//
		CFGWORD legs;
		for(WORD iLeg=1;iLeg<=pHeight->xarrSubLegs.Count;iLeg++)
			legs.AddSpecNoWord(iLeg);
		pHeightModule->m_dwLegCfgWord.Clear();
		pHeightModule->m_dwLegCfgWord.SetBodyLegs(pHeight->ciBodySerial,legs.flag.word[0]);
		ciMinIdleBodyNo++;
	}
	//补充中间空缺节间
	//4.计算全部呼高的共用区间Body
	//筛选过往类似呼高的塔身节间中可用部分
	TRUNK_PLANE xCurrPlane;
	TRUNK_SEGMENT* pSegment;
	CXhPtrSet<EXIST_SECTION> listModifiedExistSects;
	for(pSection=this->listExistSections.EnumObjectFirst();pSection;pSection=this->listExistSections.EnumObjectNext())
	{
		if(!pSection->cfgword.IsHasBodyNo(idHightestBody)||
			pSection->idPanelType!=TRUNK_SEGMENT::SECT_WHOLE)
			continue;
		if(xCurrPlane.dfWidth<EPS)
		{
			xCurrPlane.ziPlane=pSection->ziBtmPlane;
			xCurrPlane.dfWidth=pSection->dfBtmWidth;//CalPlaneWidth(xCurrPlane.ziPlane);
		}
		else if(xCurrPlane.ziPlane<pSection->ziBtmPlane)
		{
			xCurrPlane.ziPlane=pSection->ziBtmPlane;
			xCurrPlane.dfWidth=pSection->dfBtmWidth;
		}
		bool modified=false;
		for(pHeight=pHashPlanHeights->GetFirst();pHeight;pHeight=pHashPlanHeights->GetNext())
		{
			if(pHeight->ciBodySerial>idHightestBody&&pSection->ziBtmPlane<pHeight->xSublegInfo.ziMaxTopPlane)
			{
				pSection->cfgword.AddBodyLegs(pHeight->ciBodySerial);
				pHeight->ziCurrPlane=max(pHeight->ziCurrPlane,pSection->ziBtmPlane);
				modified=true;
				continue;
			}
		}
		if(modified)
			listModifiedExistSects.append(pSection);
	}
	while(true)
	{
		TRUNK_SEGMENT body(xCurrPlane.dfWidth,this->dfSlopeGradient);
		double dfMinHeight=4500,dfMaxHeight=4500;
		bool successed=body.CalWholeSectHeight(&dfMinHeight,&dfMaxHeight);
		long uiPreferHeight=ftoi((dfMinHeight+dfMaxHeight)*0.5);
		long uiCurrHeight=ftoi(dfMaxHeight);
		uiPreferHeight-=uiPreferHeight%100;	//规划阶段节间高度到100mm精度就够了。
		uiCurrHeight-=uiCurrHeight%100;
		for(pHeight=pHashPlanHeights->GetFirst();pHeight;pHeight=pHashPlanHeights->GetNext())
		{
			if(pHeight->ciBodySerial<=idHightestBody)
				continue;	//此时新增呼高hRelaHeightModule属性也已赋值
			if(xCurrPlane.ziPlane+uiCurrHeight<pHeight->xSublegInfo.ziMaxTopPlane)
			{	//当前规划的完整塔身节间可以完全的放到当前呼高中
				body.cfgword.AddBodyLegs(pHeight->ciBodySerial);
				continue;
			}
			else if(xCurrPlane.ziPlane+dfMinHeight<pHeight->xSublegInfo.ziMaxTopPlane)
			{
				uiCurrHeight=pHeight->xSublegInfo.ziMaxTopPlane-xCurrPlane.ziPlane;
				body.cfgword.AddBodyLegs(pHeight->ciBodySerial);
				continue;
			}
		}
		if(body.cfgword.IsNull())
			break;
		pSegment=listInstSegments.Append(body);
		pSegment->xTopPlane.ziPlane=xCurrPlane.ziPlane;
		pSegment->wiSegHeight=(WORD)min(uiPreferHeight,uiCurrHeight);
		xCurrPlane.dfWidth=pSegment->dfBtmWidth;
		xCurrPlane.ziPlane=xCurrPlane.ziPlane+pSegment->wiSegHeight;
		for(pHeight=pHashPlanHeights->GetFirst();pHeight;pHeight=pHashPlanHeights->GetNext())
		{
			if(pHeight->ciBodySerial<=idHightestBody||!pSegment->cfgword.IsHasBodyNo(pHeight->ciBodySerial))
				continue;	//此时新增呼高hRelaHeightModule属性也已赋值
			pHeight->ziCurrPlane=pSegment->ziBtmPlane;
		}
	}
	//添加塔腿
	for(pHeight=pHashPlanHeights->GetFirst();pHeight;pHeight=pHashPlanHeights->GetNext())
	{
		if(pHeight->ciBodySerial<=idHightestBody)
			continue;
		if(pHeight->ziCurrPlane>=pHeight->xSublegInfo.ziMinTopPlane)
		{	//已不需要直接布塔身节间，直接接各减腿
			pHeight->xLegTopPlane.ziPlane=pHeight->ziCurrPlane;
			pHeight->xLegTopPlane.dfWidth=pHeight->CalPlaneWidth(pHeight->ziCurrPlane);
			for(UINT k=0;k<pHeight->xarrSubLegs.Count;k++)
				pHeight->xarrSubLegs[k].xTopPlane=pHeight->xLegTopPlane;
		}
		else //if(pHeight->ziCurrPlane<pHeight->xSublegInfo.ziMinTopPlane)
		{	//需要补接半贯通节间
			double dfCurrWidth=pHeight->CalPlaneWidth(pHeight->ziCurrPlane);
			TRUNK_SEGMENT halfseg(dfCurrWidth,this->dfSlopeGradient);
			double dfMinHeight=4500,dfMaxHeight=4500;
			bool successed=halfseg.CalDownHalfSectHeight(&dfMinHeight,&dfMaxHeight);
			long uiPreferHeight=ftoi((dfMinHeight+dfMaxHeight)*0.5);
			uiPreferHeight-=uiPreferHeight%100;
			if(pHeight->ziCurrPlane+uiPreferHeight<=pHeight->xSublegInfo.ziMaxTopPlane)
				uiPreferHeight=max(uiPreferHeight,pHeight->xSublegInfo.ziMinTopPlane-pHeight->ziCurrPlane);
			else
				uiPreferHeight=pHeight->xSublegInfo.ziMaxTopPlane-pHeight->ziCurrPlane;
			halfseg.cfgword.SetBodyLegs(pHeight->ciBodySerial);
			pSegment=listInstSegments.Append(halfseg,0);
			pSegment->xTopPlane.ziPlane=pHeight->ziCurrPlane;
			pSegment->wiSegHeight=(WORD)uiPreferHeight;
			pSegment->idPanelType=TRUNK_SEGMENT::SECT_HALF_DOWN;
			pHeight->xLegTopPlane.ziPlane=pSegment->ziBtmPlane;
			pHeight->xLegTopPlane.dfWidth=pSegment->dfBtmWidth;//CalPlaneWidth(pHeight->xLegTopPlane.ziPlane);
			for(UINT k=0;k<pHeight->xarrSubLegs.Count;k++)
				pHeight->xarrSubLegs[k].xTopPlane=pHeight->xLegTopPlane;
		}
	}
	//更新现存节间的新配材号
	CLDSNode* pTransitCornerNode=pTower->FromNodeHandle(hExcludeTransitNode);
	for(pSection=listModifiedExistSects.GetFirst();pSection;pSection=listModifiedExistSects.GetNext())
	{
		TAPER_SCOPE taper;
		f3dPoint xTopPoint(pSection->dfTopWidth*0.5,pSection->dfTopWidth*0.5,pSection->ziTopPlane);
		f3dPoint xBtmPoint(pSection->dfBtmWidth*0.5,pSection->dfBtmWidth*0.5,pSection->ziBtmPlane);
		taper.VerifyVertex(xTopPoint);
		taper.VerifyVertex(xBtmPoint);
		taper.VerifyVertex(-xTopPoint.x,-xTopPoint.y,xTopPoint.z);
		taper.VerifyVertex(-xBtmPoint.x,-xBtmPoint.y,xBtmPoint.z);
		bool bRemovedHoriPanel=false;
		if(pTransitCornerNode&&fabs(pSection->ziBtmPlane-pTransitCornerNode->xOriginalPos.z)<EPS)
			bRemovedHoriPanel=true;

		for(pNode=pTower->EnumNodeFirst();pNode;pNode=pTower->EnumNodeNext())
		{
			if(!pSection->cfgword.And(pNode->cfgword))
				continue;	//不归属当前节间
			if(bRemovedHoriPanel&&fabs(pNode->xOriginalPos.z-pSection->ziBtmPlane)<EPS)
			{	//只保留横隔面角节点，其余跳过
				GEPOINT xUnifyPoint(fabs(pNode->xOriginalPos.x),fabs(pNode->xOriginalPos.y),pNode->xOriginalPos.z);
				if(!xUnifyPoint.IsEqual(xBtmPoint))
					continue;
			}
			if(taper.IsIncludePoint(pNode->xOriginalPos))
				pNode->cfgword.AddSpecWord(pSection->cfgword);
		}
		for(pRod=pTower->EnumRodFirst();pRod;pRod=pTower->EnumRodNext())
		{
			if(!pSection->cfgword.And(pRod->cfgword))
				continue;	//不归属当前节间
			if(pRod->pStart==NULL||pRod->pEnd==NULL)
				continue;
			if( bRemovedHoriPanel&&
				fabs(pRod->pStart->xOriginalPos.z-pSection->ziBtmPlane)<EPS&&
				fabs(pRod->pEnd->xOriginalPos.z-pSection->ziBtmPlane)<EPS)
				continue;	//之前呼高腿身过渡隔面内的横材应跳过
			if( !taper.IsIncludePoint(pRod->pStart->xOriginalPos,0.5)||
				!taper.IsIncludePoint(pRod->pEnd->xOriginalPos,0.5))
				continue;
			pRod->cfgword.AddSpecWord(pSection->cfgword);
		}
	}
	//提取汇总整体规划数据，重新合理化调整
	hashTrunkNodes.Empty();
	for(pSegment=listInstSegments.EnumObjectFirst();pSegment;pSegment=listInstSegments.EnumObjectNext())
		AppendSectionToModel(pSegment,NULL,pTower);
	for(pHeight=pHashPlanHeights->GetFirst();pHeight;pHeight=pHashPlanHeights->GetNext())
	{
		if(pHeight->ciBodySerial<=idHightestBody)
			continue;
		this->AppendTransitHoriSectionToModel(pHeight,pTower);
		for(UINT i=0;i<pHeight->xarrSubLegs.Count;i++)
		{
			CFGLEG cfgleg(pHeight->ciBodySerial,i+1);
			pHeight->xarrSubLegs[i].cfgword=cfgleg.ToCfgWord();
			AppendSectionToModel(&pHeight->xarrSubLegs[i],pHeight,pTower);
		}
	}
	//扩充塔头等全局共用部分的配材号
	for(pNode=pTower->EnumNodeFirst();pNode;pNode=pTower->EnumNodeNext())
	{
		if(pNode->IsLegObj())
			continue;
		bool bGlobalShareObject=true;
		for(pExistHeight=hashExistHeights.GetFirst();bGlobalShareObject&&pExistHeight;pExistHeight=hashExistHeights.GetNext())
		{
			if(!pNode->cfgword.IsHasBodyNo(pExistHeight->ciBodySerial))
				bGlobalShareObject=false;
		}
		if(bGlobalShareObject)
		{
			for(pHeight=pHashPlanHeights->GetFirst();pHeight;pHeight=pHashPlanHeights->GetNext())
				pNode->cfgword.AddBodyLegs(pHeight->ciBodySerial);
		}
	}
	for(CLDSPart* pPart=pTower->EnumPartFirst();pPart;pPart=pTower->EnumPartNext())
	{
		if(pPart->IsLegObj())
			continue;
		bool bGlobalShareObject=true;
		for(pExistHeight=hashExistHeights.GetFirst();bGlobalShareObject&&pExistHeight;pExistHeight=hashExistHeights.GetNext())
		{
			if(!pPart->cfgword.IsHasBodyNo(pExistHeight->ciBodySerial))
				bGlobalShareObject=false;
		}
		if(bGlobalShareObject)
		{
			for(pHeight=pHashPlanHeights->GetFirst();pHeight;pHeight=pHashPlanHeights->GetNext())
				pPart->cfgword.AddBodyLegs(pHeight->ciBodySerial);
		}
	}
	//重新计算节点杆件坐标
	for(CLDSNode* pNode=pTower->EnumNodeFirst();pNode;pNode=pTower->EnumNodeNext())
		pNode->Unlock();
	for(CLDSLinePart* pRod=pTower->EnumRodFirst();pRod;pRod=pTower->EnumRodNext())
		pRod->ClearFlag();
	for(CLDSNode* pNode=pTower->EnumNodeFirst();pNode;pNode=pTower->EnumNodeNext())
		pNode->CalPosition();
	for(CLDSLinePart* pRod=pTower->EnumRodFirst();pRod;pRod=pTower->EnumRodNext())
		pRod->CalPosition();
	return true;
}
static void DrawSegment(GEPOINT origin,PLAN_SECTION* pSect,CDxfFile* pDxfFile)
{
	GEPOINT xTopPoint(origin.x+pSect->xTopPlane.dfWidth*0.5,-(origin.z+pSect->xTopPlane.ziPlane));
	GEPOINT xBtmPoint(origin.x+pSect->dfBtmWidth*0.5,-(origin.z+pSect->ziBtmPlane));
	if( pSect->idPanelType==PLAN_SECTION::SECT_HALF_UP||
		pSect->idPanelType==PLAN_SECTION::SECT_SUBLEG)
	{
		pDxfFile->NewLine(xTopPoint,xBtmPoint);
		pDxfFile->NewLine(f3dPoint(xTopPoint.x-pSect->xTopPlane.dfWidth,xTopPoint.y),f3dPoint(xBtmPoint.x-pSect->dfBtmWidth,xBtmPoint.y));
		//横隔材
		pDxfFile->NewLine(f3dPoint(xTopPoint.x-pSect->xTopPlane.dfWidth,xTopPoint.y),f3dPoint(xTopPoint.x,xTopPoint.y));
		//八字撇材
		pDxfFile->NewLine(f3dPoint(xTopPoint.x-pSect->xTopPlane.dfWidth/2,xTopPoint.y),f3dPoint(xBtmPoint.x-pSect->dfBtmWidth,xBtmPoint.y));
		//八字捺材
		pDxfFile->NewLine(f3dPoint(xTopPoint.x-pSect->xTopPlane.dfWidth/2,xTopPoint.y),f3dPoint(xBtmPoint.x,xBtmPoint.y));
	}
	else if(pSect->idPanelType==PLAN_SECTION::SECT_HALF_DOWN)
	{
		pDxfFile->NewLine(xTopPoint,xBtmPoint);
		pDxfFile->NewLine(f3dPoint(xTopPoint.x-pSect->xTopPlane.dfWidth,xTopPoint.y),f3dPoint(xBtmPoint.x-pSect->dfBtmWidth,xBtmPoint.y));
		//横隔材
		pDxfFile->NewLine(f3dPoint(xBtmPoint.x-pSect->dfBtmWidth,xBtmPoint.y),f3dPoint(xBtmPoint.x,xBtmPoint.y));
		//八字撇材
		pDxfFile->NewLine(f3dPoint(xBtmPoint.x-pSect->dfBtmWidth/2,xBtmPoint.y),f3dPoint(xTopPoint.x-pSect->xTopPlane.dfWidth,xTopPoint.y));
		//八字捺材
		pDxfFile->NewLine(f3dPoint(xBtmPoint.x-pSect->dfBtmWidth/2,xBtmPoint.y),f3dPoint(xTopPoint.x,xTopPoint.y));
	}
	else //if(pSect->idPanelType==PLAN_SECTION::SECT_WHOLE)
	{
		pDxfFile->NewLine(xTopPoint,xBtmPoint);
		pDxfFile->NewLine(f3dPoint(xTopPoint.x-pSect->xTopPlane.dfWidth,xTopPoint.y),f3dPoint(xBtmPoint.x-pSect->dfBtmWidth,xBtmPoint.y));
		//X字撇材
		pDxfFile->NewLine(f3dPoint(xTopPoint.x,xTopPoint.y),f3dPoint(xBtmPoint.x-pSect->dfBtmWidth,xBtmPoint.y));
		//X字捺材
		pDxfFile->NewLine(f3dPoint(xTopPoint.x-pSect->xTopPlane.dfWidth,xTopPoint.y),xBtmPoint);
	}
}
#ifdef __ALFA_TEST_VERSION_
#include "LDS.h"
#include "MainFrm.h"
bool Test_TrunkPlanning(CTower* pTower=NULL)
{
	if(pTower==NULL)
		pTower=&Ta;
	CUndoOperObject undo(pTower,true);
	CTrunkBodyPlanner trunk;
	//填充当前呼高规划信息
	trunk.uiMinHeight=30000;	//***
	trunk.uiMaxHeight=54000;	//***
	trunk.wnHeightGroup=9;		//***
	trunk.wiLegDiffLevel=1000;	//***
	trunk.wnMaxLegCount=9;
	trunk.InitTrunkInfo();
	CHashListEx<HEIGHTGROUP> hashPlanHeights;
	CLogErrorLife life(&logerr);
	if(!trunk.UpdateTowerTrunk(pTower,&hashPlanHeights))
		return false;
	CMainFrame* pMainFrm=(CMainFrame*)theApp.GetMainWnd();
	pMainFrm->GetTowerTreePage()->RefreshTreeCtrl();
	HEIGHTGROUP* pHeight=hashPlanHeights.GetFirst();
	long liMaxWidth=ftoi(pHeight->xBtmPlane.dfWidth);
	liMaxWidth+=1000-liMaxWidth%500;

	CDxfFile dxffile;
	dxffile.extmin.Set(-liMaxWidth);
	dxffile.extmax.x=hashPlanHeights.Count*liMaxWidth;
	dxffile.extmax.y=trunk.ziHeight0Plane+pHeight->uiNamedHeight;
	dxffile.OpenFile("d:/trunk.dxf");
	PRESET_ARRAY64<bool> xarrSegStates;
	xarrSegStates.ZeroPresetMemory();
	GEPOINT origin;
	int iOffsetIndex=0;
	for(pHeight=hashPlanHeights.GetFirst();pHeight;pHeight=hashPlanHeights.GetNext(),iOffsetIndex++)
	{
		origin.x+=liMaxWidth;
		for(TRUNK_SEGMENT* pSegment=trunk.hashBodySegments.GetFirst();pSegment;pSegment=trunk.hashBodySegments.GetNext())
		{
			bool* pbState=xarrSegStates.GetAt(pSegment->Id);
			if(pbState!=NULL&&*pbState)
				continue;
			if(pSegment->cfgword.IsHasBodyNo(pHeight->ciBodySerial))
			{
				DrawSegment(origin,pSegment,&dxffile);
				xarrSegStates.Set(pSegment->Id,true);
			}
		}
		//绘制最高塔呼高减腿
		for(UINT j=0;j<pHeight->xarrSubLegs.Count;j++)
			DrawSegment(origin,pHeight->xarrSubLegs.GetAt(j),&dxffile);
	}
	dxffile.CloseFile();
	return true;

	/*HEIGHTGROUP height;
	TRUNK_SEGMENT body;
	double dfMinHeight,dfMaxHeight;
	body.dfTopWidth=4700;
	body.dfSlopeGradient=height.dfSlopeGradient=0.11;
	body.CalUpHalfSectHeight(&dfMinHeight,&dfMaxHeight,34.8,40);
	body.dfTopWidth=5085;
	body.CalWholeSectHeight(&dfMinHeight,&dfMaxHeight,25,40);
	//
	height.xBtmPlane.dfWidth=11333;
	height.xBtmPlane.ziPlane=56000;
	HEIGHTGROUP::LEGINFO xLegInfo48m,xLegInfo45m,xLegInfo42m;
	WORD wnLegCount=height.CalTopPlane(9,1000,&xLegInfo48m);
	height.xBtmPlane.dfWidth=10673;
	height.xBtmPlane.ziPlane=53000;
	wnLegCount=height.CalTopPlane(9,1000,&xLegInfo45m);
	height.xBtmPlane.dfWidth=10013;
	height.xBtmPlane.ziPlane=50000;
	wnLegCount=height.CalTopPlane(9,1000,&xLegInfo42m);
	return wnLegCount>0;*/
}
#endif
