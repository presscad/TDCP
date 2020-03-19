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
//�ڼ�'J'
class CShankSection : public CShankDefineObject
{
public:
	CShankDefineObject* m_pParent;	//���ֶΣ�����������ֶ�Ҳ�����ǽ��ȣ�
	int divide_n;		//������ڼ��ڲ��ȷ��ӽڼ���
	CXhChar16 btm_zstr;	//�ײ�Z����λ���ַ���,��ֵ>1��ʾ����,С��1��ʾ�ײ���ռ��ǰ�ֶεĴ��߱���
	char front_layout[8],side_layout[8],hsect_layout[8],v_layout[8];//���沼�ġ����沼�ġ���������沼�ļ�V�沼��
public:
	CShankSection();
	virtual ~CShankSection(){;}
};
//��'L'
class CShankLeg : public CShankDefineObject 
{
	friend class CShankModule;
	ARRAYPTR_LIST<CShankSection> sectionList;
	CShankModule *m_pParentModule;
public:
	CFGLEG legword;			//���Ⱥ�(��ĺ�)
	SEGI m_iSeg;			//�ֶκ�
	BOOL m_bDatumLeg;		//�¶ȶ����׼��
	double m_fHeight;		//���ȴ���(mm)
	double m_fFrontWidth;	//�������(mm)
	double m_fSideWidth;	//�������(mm)
	int divide_n;			//�����漰V��ĵȷ��ӽڼ���
	char front_layout[8],side_layout[8],hsect_layout[8],v_layout[8];//���沼�ġ����沼�ġ���������沼�ļ�V�沼��
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
//�ֶ�'D'
class CShankSegment : public CShankDefineObject 
{
	friend class CShankModule;
	friend class CShankSlope;
	ARRAY_LIST<CShankModule*> modulePtrList;
	ARRAYPTR_LIST<CShankSection> sectionList;
	ARRAYPTR_LIST<CShankSegment> segmentList;	//�����ӶΣ��Ӷ����Ӷμ��ǲ��л�����ϵ��˳���νӹ�ϵ��
	CShankDefine *m_pShankDef;
	double _fHeight;		//�ֶδ���(mm)
public:
	DECLARE_PROPERTY(double,m_fHeight);		//�ֶδ���(mm)
	CShankDefineObject* m_pParent;	//���ֶΣ�����������ֶ�Ҳ�����ǽ��ȣ�
	CFGLEG bodyword;	//�����
	SEGI m_iSeg;		//�ֶκ�
	int divide_n;		//������ڼ��ڲ��ȷ��ӽڼ���
	char front_layout[8],side_layout[8],hsect_layout[8],v_layout[8];//���沼�ġ����沼�ġ���������沼�ļ�V�沼��
public:
	CShankSegment();
	virtual ~CShankSegment();
	//��������
	int ModuleNum(){return modulePtrList.GetSize();}
	bool DeleteModuleAtIndex(int index){return modulePtrList.RemoveAt(index);}
	CShankModule* AppendModule();
	CShankModule* ModuleAtIndex(int index);
	//���
	int SectionNum(){return sectionList.GetSize();}
	int GetSectionIndex(CShankSection* pSection){return sectionList.GetIndex(pSection);}
	bool DeleteSectionAtIndex(int index){return sectionList.RemoveAt(index);}
	CShankSection* AppendSection();
	CShankSection* InsertSection(int index=-1);
	CShankSection* SectionAtIndex(int index);
	//�����Ӷ�
	int JunctionSegmentNum(){return segmentList.GetSize();}
	bool DeleteSegmentAtIndex(int index){return segmentList.RemoveAt(index);}
	CShankSegment* AppendJunctionSegment();
	CShankSegment* InsertJunctionSegment(int index=-1);
	CShankSegment* JunctionSegmentAtIndex(int index);
	//
	CShankLeg* GetDatumLeg();
	double GetDatumLegHeight();
	CShankModule *GetSpecModule(char *sModuleName);		//�õ�ָ�����߷���
	double GetSpecModuleLegHeight(char *sModuleName);	//�õ�ָ�����߷���߶�
	double GetSpecLegHeight(CShankLeg *pLeg);	//����ָ�����ȸ߶ȣ�Ҫ��pLeg����ָ��ǰ�¶��е�ĳһ���ȶ���
	int AllocateBodyNo(int iInitLegNo);	//������Ϳɷ�����Ⱥ�,���ط������Ϳ�����߽��Ⱥ�
	CFGLEG AllocateBodyWord();
	bool IsSegParentsHasModule();		//�жϷֶ������ĸ��ڵ����Ƿ���ں���
};
//����'B'
class CShankModule : public CShankDefineObject 
{
	friend class CShankSlope;
	friend class CShankSegment;
	CShankDefine *m_pShankDef;
	ARRAYPTR_LIST<CShankSegment> segList;
	ARRAYPTR_LIST<CShankLeg> legList;
public:
	CShankDefineObject* m_pParent;	//���ֶ�(�������¶�Ҳ�����Ƿֶ�)
	CXhChar50 name;
	WORD m_iBodyNo;					//���岿�ֶ�Ӧ�ı����1~24
	CFGLEG legword;					//������ĺ�
	WORD m_arrActiveQuadLegNo[4];	//��ģ�͵�ǰ��һ�������޵ĵ�ǰ���Ⱥ�
public:
	CShankModule();
	virtual ~CShankModule();
	//��������
	int LegNum(){return legList.GetSize();}
	bool DeleteLegAtIndex(int index){return legList.RemoveAt(index);}
	CShankLeg* LegAtIndex(int index){return &legList[index];}
	CShankLeg* AppendLeg();
	//�����Ӷ�
	int SegmentNum(){return segList.GetSize();}
	bool DeleteSegmentAtIndex(int index){return segList.RemoveAt(index);}
	int GetSegmentIndex(CShankSegment* pSeg){return segList.GetIndex(pSeg);}
	CShankSegment* AppendSegment();
	CShankSegment* SegmentAtIndex(int index);
	CShankSegment* InsertSegment(int index=-1);
	//
	bool IsFirstModule();
	int AllocateBodyNo(int iInitLegNo);	//������Ϳɷ�����Ⱥ�,���ط������Ϳ�����߽��Ⱥ�
	bool AllocateLegWord();
};
//�¶�'K'
class CShankSlope : public CShankDefineObject 
{
	friend class CShankDefine;
	CShankDefine *m_pShankDef;
	ARRAYPTR_LIST<CShankSegment> segList;
	ARRAYPTR_LIST<CShankModule*> modulePtrList;
public:
	char m_cMainRodType;		//'L'�Ǹ�;'T'�ֹ�;
	double m_fTopCoordZ;		//��λ��mm
	double m_fTopFrontWidth;	//���濪�ڿ��(mm)
	double m_fTopSideWidth;		//���濪�ڿ��(mm)
public:
	CShankSlope();
	virtual ~CShankSlope();
	//�����Ӷ�
	CShankSegment *AppendSegment();
	CShankSegment* InsertSegment(int index=-1);
	CShankSegment *SegmentAtIndex(int index){return &segList[index];}
	int GetSegmentIndex(CShankSegment* pSeg){return segList.GetIndex(pSeg);}
	bool DeleteSegmentAtIndex(int index){return segList.RemoveAt(index);}
	int SegmentNum(){return segList.GetNodeNum();}
	double SlopeBodyHeight();		//���ش��¶α��岿���ܸ߶�(mm)
	//
	int ModuleNum(){return modulePtrList.GetSize();}
	bool DeleteModuleAtIndex(int index){return modulePtrList.RemoveAt(index);}
	CShankModule* AppendModule();
	CShankModule* ModuleAtIndex(int index);
	//
	CShankLeg* GetDatumLeg();
	double GetDatumLegHeight();
	double GetSpecLegHeight(CShankLeg *pLeg);	//����ָ�����ȸ߶ȣ�Ҫ��pLeg����ָ��ǰ�¶��е�ĳһ���ȶ���
	CShankModule *GetSpecModule(char *sModuleName);		//�õ�ָ�����߷���
	double GetSpecModuleLegHeight(char *sModuleName);	//�õ�ָ�����߷���߶�
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
	//�¶�
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
	double SlopeTopCoordZ(int iSlope);		//iSlopeΪ0Ϊ�����ı��¶�������
	double SlopeBtmFrontWidth(UINT iSlope,CShankLeg *pLeg=NULL);
	double SlopeBtmSideWidth(UINT iSlope,CShankLeg *pLeg=NULL);
	double SlopeBtmCoordZ(int iSlope,CShankLeg *pLeg=NULL);
	CFGWORD SlopeTopCfgword(int iSlope);	//iSlopeΪ0Ϊ�����ı��¶�������
	SEGI SlopeTopSegI(int iSlope);	//�¶ζ����ڵ�Ķκ� wht 10-11-16
	bool AllocateCfgWord();
};

typedef struct tagSECTION_NODEARR
{
	CLDSNodePtr nodePtrArr[8];			//�¶���ʼ�ڵ��µĽڵ�����
	CLDSLinePartPtr linePartPtrArr[8];	//�¶���ʼ�ڵ��µĸ˼�����
	tagSECTION_NODEARR(){memset(this,0,sizeof(tagSECTION_NODEARR));}
}SECTION_NODEARR;
struct SLOPE_POSINFO{
	double coordTopZ,coordBottomZ;
	bool bTopSlope,bBottomSlope;
	CShankLeg *pDatumLeg;				//���һ���¶��еĻ�׼����
	CLDSLinePartPtr legRodPtrArr[8];	//���һ���¶��еĻ�׼���ȵ����ĸ˼�
};
//��XML��ʽ������ģ��ת��Ϊʵ�ʵ�����ģ��
typedef struct tagSKETCH_SIZEDIM
{
	f3dPoint dimStart,dimEnd;
	BYTE cSizeType;	//'X','Y',Z',"L'
	WORD wAlignStyle;
	double fTextHeight;
	int cascade;	//�ֲ��עʱ��ʾ�ֲ��
	f3dPoint align_vec,norm,pos;
	BOOL bLegDim;	//�Ƿ�Ϊ�����ϵı�ע
	CFGWORD cfgword;//ͬLDS��������ĺţ����ڿ��ƶ����ʱ��ע����ʾ
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
	//Ĭ�ϸ˼�����
	static bool UpdateTowerModel(CTower* pTower,CShankDefine *pDefine,int nPoly=4,long idClassType=CLS_LINEANGLE);
};