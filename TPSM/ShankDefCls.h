#pragma once
#include "Tower.h"
#include "ArrayList.h"
#include "ArrayPtrList.h"
#include "XhCharString.h"
#include "HashTable.h"
//
class CShankDefine;
class CShankSlope;
class CShankSegment;
class CShankModule;
class CShankLeg;
class CShankSection;
class CShankDefineObject
{
public:
	char m_cObjType;	//K:CShankSlope;D:CShankSegment;B:CShankModule;L:CShankLeg;J:CShankSection
public:
	CShankDefineObject(){m_cObjType=0;}
	virtual ~CShankDefineObject(){;}
	//
	CShankSlope* Slope(){return (CShankSlope*)this;}
	CShankSegment* Segment(){return (CShankSegment*)this;}
	CShankModule* Module(){return (CShankModule*)this;}
	CShankLeg* Leg(){return (CShankLeg*)this;}
	CShankSection* Section(){return (CShankSection*)this;}
};
//节间'J'
class CShankSection : public CShankDefineObject
{
public:
	CShankDefineObject* m_pParent;	//父分段（可能是塔身分段也可能是接腿）
	int divide_n;		//正侧面节间内部等分子节间数
	CXhChar16 btm_zstr;	//底部Z坐标位置字符串,其值>1表示垂高,小于1表示底部所占当前分段的垂高比例
	char front_layout[8],side_layout[8],hsect_layout[8],v_layout[8];//正面布材、侧面布材、顶部横隔面布材及V面布材
public:
	CShankSection();
	virtual ~CShankSection(){;}
};
//腿'L'
class CShankLeg : public CShankDefineObject 
{
	friend class CShankModule;
	ARRAYPTR_LIST<CShankSection> sectionList;
	CShankModule *m_pParentModule;
public:
	CFGLEG legword;			//接腿号(配材号)
	SEGI m_iSeg;			//分段号
	BOOL m_bDatumLeg;		//坡度定义基准腿
	double m_fHeight;		//接腿垂高(mm)
	double m_fFrontWidth;	//正面根开(mm)
	double m_fSideWidth;	//侧面根开(mm)
	int divide_n;			//正侧面及V面的等分子节间数
	char front_layout[8],side_layout[8],hsect_layout[8],v_layout[8];//正面布材、侧面布材、顶部横隔面布材及V面布材
public:
	CShankLeg();
	virtual ~CShankLeg();
	//
	CShankModule* ParentModule(){return m_pParentModule;}
	//
	int SectionNum(){return sectionList.GetSize();}
	int GetSectionIndex(CShankSection* pSection){return sectionList.GetIndex(pSection);}
	bool DeleteSectionAtIndex(int index){return sectionList.RemoveAt(index);}
	CShankSection* AppendSection();
	CShankSection* InsertSection(int index=-1);
	CShankSection* SectionAtIndex(int index);
};
//分段'D'
class CShankSegment : public CShankDefineObject 
{
	friend class CShankModule;
	friend class CShankSlope;
	ARRAY_LIST<CShankModule*> modulePtrList;
	ARRAYPTR_LIST<CShankSection> sectionList;
	ARRAYPTR_LIST<CShankSegment> segmentList;	//连接子段（子段与子段间是并列互换关系非顺序衔接关系）
	CShankDefine *m_pShankDef;
	double _fHeight;		//分段垂高(mm)
public:
	DECLARE_PROPERTY(double,m_fHeight);		//分段垂高(mm)
	CShankDefineObject* m_pParent;	//父分段（可能是塔身分段也可能是接腿）
	CFGLEG bodyword;	//本体号
	SEGI m_iSeg;		//分段号
	int divide_n;		//正侧面节间内部等分子节间数
	char front_layout[8],side_layout[8],hsect_layout[8],v_layout[8];//正面布材、侧面布材、顶部横隔面布材及V面布材
public:
	CShankSegment();
	virtual ~CShankSegment();
	//归属呼高
	int ModuleNum(){return modulePtrList.GetSize();}
	bool DeleteModuleAtIndex(int index){return modulePtrList.RemoveAt(index);}
	CShankModule* AppendModule();
	CShankModule* ModuleAtIndex(int index);
	//间接
	int SectionNum(){return sectionList.GetSize();}
	int GetSectionIndex(CShankSection* pSection){return sectionList.GetIndex(pSection);}
	bool DeleteSectionAtIndex(int index){return sectionList.RemoveAt(index);}
	CShankSection* AppendSection();
	CShankSection* InsertSection(int index=-1);
	CShankSection* SectionAtIndex(int index);
	//连接子段
	int JunctionSegmentNum(){return segmentList.GetSize();}
	bool DeleteSegmentAtIndex(int index){return segmentList.RemoveAt(index);}
	CShankSegment* AppendJunctionSegment();
	CShankSegment* InsertJunctionSegment(int index=-1);
	CShankSegment* JunctionSegmentAtIndex(int index);
	//
	CShankLeg* GetDatumLeg();
	double GetDatumLegHeight();
	CShankModule *GetSpecModule(char *sModuleName);		//得到指定呼高分组
	double GetSpecModuleLegHeight(char *sModuleName);	//得到指定呼高分组高度
	double GetSpecLegHeight(CShankLeg *pLeg);	//返回指定接腿高度，要求pLeg必须指向当前坡段中的某一接腿对象
	int AllocateBodyNo(int iInitLegNo);	//输入最低可分配接腿号,返回分配后最低可用最高接腿号
	CFGLEG AllocateBodyWord();
	bool IsSegParentsHasModule();		//判断分段所属的父节点中是否存在呼高
};
//呼高'B'
class CShankModule : public CShankDefineObject 
{
	friend class CShankSlope;
	friend class CShankSegment;
	CShankDefine *m_pShankDef;
	ARRAYPTR_LIST<CShankSegment> segList;
	ARRAYPTR_LIST<CShankLeg> legList;
public:
	CShankDefineObject* m_pParent;	//父分段(可能是坡段也可能是分段)
	CXhChar50 name;
	WORD m_iBodyNo;					//本体部分对应的本体号1~24
	CFGLEG legword;					//接腿配材号
	WORD m_arrActiveQuadLegNo[4];	//本模型当前第一～四象限的当前接腿号
public:
	CShankModule();
	virtual ~CShankModule();
	//连接子腿
	int LegNum(){return legList.GetSize();}
	bool DeleteLegAtIndex(int index){return legList.RemoveAt(index);}
	CShankLeg* LegAtIndex(int index){return &legList[index];}
	CShankLeg* AppendLeg();
	//连接子段
	int SegmentNum(){return segList.GetSize();}
	bool DeleteSegmentAtIndex(int index){return segList.RemoveAt(index);}
	int GetSegmentIndex(CShankSegment* pSeg){return segList.GetIndex(pSeg);}
	CShankSegment* AppendSegment();
	CShankSegment* SegmentAtIndex(int index);
	CShankSegment* InsertSegment(int index=-1);
	//
	bool IsFirstModule();
	int AllocateBodyNo(int iInitLegNo);	//输入最低可分配接腿号,返回分配后最低可用最高接腿号
	bool AllocateLegWord();
};
//坡段'K'
class CShankSlope : public CShankDefineObject 
{
	friend class CShankDefine;
	CShankDefine *m_pShankDef;
	ARRAYPTR_LIST<CShankSegment> segList;
	ARRAYPTR_LIST<CShankModule*> modulePtrList;
public:
	char m_cMainRodType;		//'L'角钢;'T'钢管;
	double m_fTopCoordZ;		//单位：mm
	double m_fTopFrontWidth;	//正面开口宽度(mm)
	double m_fTopSideWidth;		//侧面开口宽度(mm)
public:
	CShankSlope();
	virtual ~CShankSlope();
	//连接子段
	CShankSegment *AppendSegment();
	CShankSegment* InsertSegment(int index=-1);
	CShankSegment *SegmentAtIndex(int index){return &segList[index];}
	int GetSegmentIndex(CShankSegment* pSeg){return segList.GetIndex(pSeg);}
	bool DeleteSegmentAtIndex(int index){return segList.RemoveAt(index);}
	int SegmentNum(){return segList.GetNodeNum();}
	double SlopeBodyHeight();		//返回此坡段本体部分总高度(mm)
	//
	int ModuleNum(){return modulePtrList.GetSize();}
	bool DeleteModuleAtIndex(int index){return modulePtrList.RemoveAt(index);}
	CShankModule* AppendModule();
	CShankModule* ModuleAtIndex(int index);
	//
	CShankLeg* GetDatumLeg();
	double GetDatumLegHeight();
	double GetSpecLegHeight(CShankLeg *pLeg);	//返回指定接腿高度，要求pLeg必须指向当前坡段中的某一接腿对象
	CShankModule *GetSpecModule(char *sModuleName);		//得到指定呼高分组
	double GetSpecModuleLegHeight(char *sModuleName);	//得到指定呼高分组高度
};

class CShankDefine 
{
	friend class CShankSlope;
	friend class CShankSegment;
	friend class CShankModule;
	ARRAYPTR_LIST<CShankModule> Modules;
	ARRAYPTR_LIST<CShankSlope> Slopes;
public:
	CShankDefine(){;}
	~CShankDefine(){;}
	void Empty(){Slopes.Empty();Modules.Empty();}
	bool FromString(char* strDef){return false;}
	//坡段
	CShankSlope *AppendSlope();
	CShankSlope *SlopeAtIndex(int index);
	int SlopeNum(){return Slopes.GetSize();}
	bool DeleteSlopeAtIndex(int index){return Slopes.RemoveAt(index);}
	//
	CShankModule *ModuleAtIndex(int index);
	int ModuleNum(){return Modules.GetSize();}
	bool DeleteModuleAtIndex(int index){return Modules.RemoveAt(index);}
	//
	double SlopeHeight(int iSlope,CShankLeg *pLeg=NULL);
	double SlopeTopFrontWidth(int iSlope,CShankLeg *pLeg=NULL);
	double SlopeTopSideWidth(int iSlope,CShankLeg *pLeg=NULL);
	double SlopeTopCoordZ(int iSlope);		//iSlope为0为基数的变坡段索引号
	double SlopeBtmFrontWidth(UINT iSlope,CShankLeg *pLeg=NULL);
	double SlopeBtmSideWidth(UINT iSlope,CShankLeg *pLeg=NULL);
	double SlopeBtmCoordZ(int iSlope,CShankLeg *pLeg=NULL);
	CFGWORD SlopeTopCfgword(int iSlope);	//iSlope为0为基数的变坡段索引号
	SEGI SlopeTopSegI(int iSlope);	//坡段顶部节点的段号 wht 10-11-16
	bool AllocateCfgWord();
};

typedef struct tagSECTION_NODEARR
{
	CLDSNodePtr nodePtrArr[8];			//坡段起始节点下的节点数据
	CLDSLinePartPtr linePartPtrArr[8];	//坡段起始节点下的杆件数据
	tagSECTION_NODEARR(){memset(this,0,sizeof(tagSECTION_NODEARR));}
}SECTION_NODEARR;
struct SLOPE_POSINFO{
	double coordTopZ,coordBottomZ;
	bool bTopSlope,bBottomSlope;
	CShankLeg *pDatumLeg;				//最后一个坡段中的基准接腿
	CLDSLinePartPtr legRodPtrArr[8];	//最后一个坡段中的基准接腿的主材杆件
};
//由XML型式的数据模型转化为实际的铁塔模型
typedef struct tagSKETCH_SIZEDIM
{
	f3dPoint dimStart,dimEnd;
	BYTE cSizeType;	//'X','Y',Z',"L'
	WORD wAlignStyle;
	double fTextHeight;
	int cascade;	//分层标注时表示分层号
	f3dPoint align_vec,norm,pos;
	BOOL bLegDim;	//是否为接腿上的标注
	CFGWORD cfgword;//同LDS构件的配材号，用于控制多接腿时标注的显示
	tagSKETCH_SIZEDIM();
	double Size();
}SKETCH_SIZEDIM;
class CShankDefineToTowerModel
{
	static CTower* m_pTower;
	static CShankDefine *m_pDefine;
	static bool AddSlopeToTowerModel(SECTION_NODEARR &slopeStartNodeArr,int nPoly,int iSlope,int nSlopeNum,int idClassType);
	static void AddShankModule(CShankModule *pModuleDef,SECTION_NODEARR startNodeArr,SLOPE_POSINFO& slopePosInfo,
		SECTION_NODEARR& slopeStartNodeArr,SECTION_NODEARR slopeEndNodeArr,int nPoly,int idLinePartClsType);
	static void AddShankSegment(CShankSegment *pSegment,SECTION_NODEARR& startNodeArr,SLOPE_POSINFO& slopePosInfo,
		SECTION_NODEARR& slopeStartNodeArr,SECTION_NODEARR slopeEndNodeArr,int nPoly,int idLinePartClsType);
	static void AddShankLeg(CShankLeg *pLeg,SECTION_NODEARR &startNodeArr,
		SECTION_NODEARR& slopeStartNodeArr,SECTION_NODEARR slopeEndNodeArr,int nPoly,int idLinePartClsType);
	static CLDSNode *AddFixCoordZNode(CLDSNode *pAttachStartNode,CLDSNode *pAttachEndNode,double coordZ,BOOL bPriorFromSlopeNode=true,char layer_first_char=0);
	static CLDSNode* AddDivScaleNode(CLDSNode *pAttachStartNode,CLDSNode *pAttachEndNode,double scale);
	static CLDSLinePart* AddLinePart(CLDSNode *pStart,CLDSNode *pEnd,int idLinePartClsType);
public:
	static int m_iViewType;
	static ARRAY_LIST<SKETCH_SIZEDIM>dims;
	//默认杆件类型
	static bool UpdateTowerModel(CTower* pTower,CShankDefine *pDefine,int nPoly=4,long idClassType=CLS_LINEANGLE);
};