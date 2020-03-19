#pragma once
#include "list.h"
#include "ArrayList.h"
#include "XhLdsLm.h"

class CTower;
class CLDSModule;
class CLDSNode;
//塔身分段中的上或下侧的水平截面Z坐标及及开口宽(mm)
struct TRUNK_PLANE{
	long ziPlane;
	double dfWidth;
	TRUNK_PLANE(){dfWidth=ziPlane=0;}
};
struct PLAN_SECTION{
protected:
	UINT id;
protected:	//原始联动设计参数
	BYTE _ciLegSerial;		//0.表示塔身;>-1表示归属呼高的接腿号
	WORD _wiSegHeight;		//节间垂高
	double _dfSlopeGradient;//坡度δx(δy)/δz
	double _dfZp2Zh;		//Zp表示坡面高，Zh表示三维空间高
protected:	//被动计算的结果值
	TRUNK_PLANE xBtmPlane;	//下开口截面
public:
	CFGWORD cfgword;
	static const UINT SECT_WHOLE	= 1;
	static const UINT SECT_HALF_UP	= 2;
	static const UINT SECT_HALF_DOWN= 3;
	static const UINT SECT_SUBLEG	= 4;
	UINT idPanelType;		//节间布材型式
	TRUNK_PLANE xTopPlane;	//上开口截面
public:
	PLAN_SECTION(){idPanelType=1;}
	void SetKey(UINT key){id=key;}
	UINT get_Id(){return id;}
	__declspec(property(get=get_Id)) UINT Id;
	//坡度δx(δy)/δz
	double get_dfSlopeGradient(){return _dfSlopeGradient;}
	double set_dfSlopeGradient(double dfGradient);
	__declspec(property(put=set_dfSlopeGradient,get=get_dfSlopeGradient)) double dfSlopeGradient;
	long set_ziBtmPlane(long ziPlane);		//铁塔基础平面Z坐标
	long get_ziBtmPlane(){return this->xBtmPlane.ziPlane;}		//铁塔基础平面Z坐标
	__declspec(property(put=set_ziBtmPlane,get=get_ziBtmPlane)) long ziBtmPlane;	//接腿基础平面Z坐标
	double set_dfBtmWidth(double dfWidth);	//铁塔设计根开
	double get_dfBtmWidth(){return this->xBtmPlane.dfWidth;}	//铁塔设计根开
	__declspec(property(put=set_dfBtmWidth,get=get_dfBtmWidth)) double dfBtmWidth;	//接腿基础节点设计根开
	virtual bool IsLegSegment(){return _ciLegSerial>0;}
};
struct TRUNK_SEGMENT : public PLAN_SECTION
{
/*protected:	//原始联动设计参数
	WORD _wiSegHeight;		//节间垂高
	double _dfTopWidth;		//上开口
	double _dfSlopeGradient;//坡度δx(δy)/δz
protected:	//被动计算的结果值
	double _dfZp2Zh;	//Zp表示坡面高，Zh表示三维空间高
	double _dfBtmWidth;	//下开口
public:
	long ziTopPlane;
	UINT idPanelType;*/
	TRUNK_SEGMENT(double dfTopTrunkWidth=4500,double dfGradientCoef=0.11);
public:	//属性
	//塔身节间垂高
	WORD get_wiSegHeight(){return _wiSegHeight;}
	WORD set_wiSegHeight(WORD wiHeight);
	__declspec(property(put=set_wiSegHeight,get=get_wiSegHeight)) WORD wiSegHeight;
	//上开口宽度
	double get_dfTopWidth(){return xTopPlane.dfWidth;}
	double set_dfTopWidth(double dfWidth);
	__declspec(property(put=set_dfTopWidth,get=get_dfTopWidth)) double dfTopWidth;
	//下开口宽度
	double get_dfBtmWidth(){return xBtmPlane.dfWidth;}
	__declspec(property(get=get_dfBtmWidth)) double dfBtmWidth;
	//腹杆水平夹角
	double get_dfBevelGradDegAng();
	__declspec(property(get=get_dfBevelGradDegAng)) double dfBevelGradDegAng;//腹杆水平夹角
	//塔身节间规划设计函数
	double CalBevelGradDegAngle();		//左右贯通腹杆水平夹角
	double CalUpHalfBevelGradDegAngle();	//上侧半贯通腹杆水平夹角
	double CalDownHalfBevelGradDegAngle();	//下侧半贯通腹杆水平夹角
	bool CalWholeSectHeight(double* pfMinHeight,double* pfMaxHeight,double dfMinBevelDegAngle=25,double dfMaxBevelDegAngle=40);
	bool CalUpHalfSectHeight(double* pfMinHeight,double* pfMaxHeight,double dfMinBevelDegAngle=25,double dfMaxBevelDegAngle=40);
	bool CalDownHalfSectHeight(double* pfMinHeight,double* pfMaxHeight,double dfMinBevelDegAngle=25,double dfMaxBevelDegAngle=40);
};
struct TRUNK_SUBLEG : public PLAN_SECTION{
	TRUNK_SUBLEG();
public:
	BYTE set_ciLegSerial(BYTE ciSerial){return _ciLegSerial=ciSerial;}
	BYTE get_ciLegSerial(){return _ciLegSerial;}
	__declspec(property(put=set_ciLegSerial,get=get_ciLegSerial)) BYTE ciLegSerial;	//接腿号
	WORD set_wiLegHeight(WORD _wiLegHeight){return this->_wiSegHeight=_wiLegHeight;}			//接腿垂高
	WORD get_wiLegHeight(){return this->_wiSegHeight;}			//接腿垂高
	__declspec(property(put=set_wiLegHeight,get=get_wiLegHeight)) WORD wiLegHeight;	//接腿高
};
struct HEIGHTGROUP{
protected:	//原始联动设计参数
	BYTE _ciBodySerial;	//1 based serial
public:		//被动计算的结果值
	long hRelaHeightModule;
	UINT uiBodyHeight;	//接身长，mm
	UINT uiNamedHeight;	//名义呼高
	double dfSlopeGradient;		//坡度δx(δy)/δz
	TRUNK_PLANE xLegTopPlane,xBtmPlane;	//呼高分组腿身换接隔面Z坐标及最长腿基础平面Z坐标
	static const WORD MAX_SUBLEG_COUNT = 16;
	PRESET_ARRAY16<TRUNK_SUBLEG> xarrSubLegs;
	struct LEGINFO{
		WORD wiLevelDiff;	//mm
		WORD wnLegCount;	//最多接腿数
		WORD wiMaxLegHeight;
		long ziMinTopPlane,ziMaxTopPlane;
		union MIDNODE{
			CLDSNode* pNode;
			struct{CLDSNode* pLeftNode,*pRightNode;};
		}xFront,xRight,xBack,xLeft;
	}xSublegInfo;
	long ziCurrPlane;	//规划过程中的临时参数，用于表示当前已规划塔身节间的最低平面Z坐标
	HEIGHTGROUP();
	BYTE set_ciBodySerial(BYTE ciBodyNo);
	BYTE get_ciBodySerial(){return _ciBodySerial;}
	__declspec(property(put=set_ciBodySerial,get=get_ciBodySerial)) BYTE ciBodySerial;
	void SetKey(DWORD key){_ciBodySerial=(BYTE)key;}
public:
	WORD CalTopPlane(WORD wnLegCount=3,WORD wiLevelDiff=1000,HEIGHTGROUP::LEGINFO* pxLegInfo=NULL);
	double CalPlaneWidth(double zfPlane);
};
class CLegSchemaLife{
	BYTE ciOldLegSchema;
public:
	CLegSchemaLife(BYTE schema=0){
		if(schema>0)
			ciOldLegSchema=0;
		else
		{
			ciOldLegSchema=CFGLEG::Schema();
			CFGLEG::SetSchema(schema);
		}
	}
	~CLegSchemaLife(){
		if(ciOldLegSchema>0)
			CFGLEG::SetSchema(ciOldLegSchema);
	}
};
struct TRUNK_NODE{
	long ziPlane;
	CFGWORD cfgword;
	TRUNK_NODE* linknext;
	long harrQuadNodes[4];
	void SetKey(UINT key){ziPlane=(long)key;}
	TRUNK_NODE* Append();
	TRUNK_NODE();
	~TRUNK_NODE();
	bool DeleteChilds();
};
class CTrunkBodyPlanner{
public:
	BYTE ciLegSchema;		//接腿模式
	WORD wnHeightGroup;		//规划的呼高分组数
	WORD wnMaxLegCount;
	WORD wiLegDiffLevel;	//规划的呼高内减腿级差
	UINT uiMinHeight,uiMaxHeight;	//指定呼高最小和最大范围(mm)
	UINT uiTopWidth;		//塔身顶部口宽(mm)
	long ziTopPlane;		//塔身顶面Z值
	long ziHeight0Plane;	//呼高计算起始基准平面的Z坐标
	double dfSlopeGradient;	//坡度δx(δy)/δz
	CHashListEx<TRUNK_SEGMENT> hashBodySegments;
public:	//在已有塔身模型基础上进行规划时，存储已有模型的塔身信息
	struct EXIST_SECTION{
		CFGWORD cfgword;
		UINT idPanelType;
		union{
			long hQuadRod;
			long harrRod[4];
		};
		long hTopNode,hBtmNode;
		long ziTopPlane,ziBtmPlane;
		double dfTopWidth,dfBtmWidth;
		EXIST_SECTION();
	};
	struct EXIST_HEIGHT{
	protected:
		UINT _uiNamedHeight;
	public:
		BYTE ciBodySerial;	//1 based serial
		long hRelaHeightModule;
		long hTransitNode,hPostLegRod,hLowestBaseNode;
		struct INERNODES{
			long hCornerNode,hFrontInnerNode,hSideInnerNode;
		}pants;	//大裤衩结构
		EXIST_HEIGHT(){
			_uiNamedHeight=30000;
			hRelaHeightModule=hTransitNode=hPostLegRod=hLowestBaseNode=0;
			pants.hCornerNode=pants.hFrontInnerNode=pants.hSideInnerNode=0;
		}
		void SetKey(DWORD key){_uiNamedHeight=key;}
	public:	//属性
		UINT get_uiNamedHeight(){return _uiNamedHeight;}
		__declspec(property(get=get_uiNamedHeight)) UINT uiNamedHeight;
	};
	struct TRUNKSLOPE{
		union{
			long hTopNode;
			long harrTopNodes[4];
		};
		union{
			long hBtmNode;
			long harrBtmNodes[4];
		};
	}xSlope;
	CHashListEx<TRUNK_NODE>		 hashTrunkNodes;
	CHashListEx<EXIST_HEIGHT>	 hashExistHeights;
	CXhSimpleList<EXIST_SECTION> listExistSections;
protected:
	void AppendTransitHoriSectionToModel(HEIGHTGROUP* pHeight,CTower* pTower);
	void AppendSectionToModel(PLAN_SECTION* pSection,HEIGHTGROUP* pBelongHeight,CTower* pTower);
	bool Planning(IXhList<HEIGHTGROUP>* pListPlanHeights,bool bTopHalfSection=false);
	bool AutoRetrieveTrunkInfo(CTower* pTower);
public:
	CTrunkBodyPlanner();
	double CalPlaneWidth(double zfPlane);
	bool InitTrunkInfo(UINT idTowerModelTmpl=0);
	bool UpdateTowerTrunk(CTower* pTower,CHashListEx<HEIGHTGROUP>* pHashPlanHeights=NULL);
};
