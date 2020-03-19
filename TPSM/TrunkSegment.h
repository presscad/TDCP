#pragma once
#include "list.h"
#include "ArrayList.h"
#include "XhLdsLm.h"

class CTower;
class CLDSModule;
class CLDSNode;
//����ֶ��е��ϻ��²��ˮƽ����Z���꼰�����ڿ�(mm)
struct TRUNK_PLANE{
	long ziPlane;
	double dfWidth;
	TRUNK_PLANE(){dfWidth=ziPlane=0;}
};
struct PLAN_SECTION{
protected:
	UINT id;
protected:	//ԭʼ������Ʋ���
	BYTE _ciLegSerial;		//0.��ʾ����;>-1��ʾ�������ߵĽ��Ⱥ�
	WORD _wiSegHeight;		//�ڼ䴹��
	double _dfSlopeGradient;//�¶Ȧ�x(��y)/��z
	double _dfZp2Zh;		//Zp��ʾ����ߣ�Zh��ʾ��ά�ռ��
protected:	//��������Ľ��ֵ
	TRUNK_PLANE xBtmPlane;	//�¿��ڽ���
public:
	CFGWORD cfgword;
	static const UINT SECT_WHOLE	= 1;
	static const UINT SECT_HALF_UP	= 2;
	static const UINT SECT_HALF_DOWN= 3;
	static const UINT SECT_SUBLEG	= 4;
	UINT idPanelType;		//�ڼ䲼����ʽ
	TRUNK_PLANE xTopPlane;	//�Ͽ��ڽ���
public:
	PLAN_SECTION(){idPanelType=1;}
	void SetKey(UINT key){id=key;}
	UINT get_Id(){return id;}
	__declspec(property(get=get_Id)) UINT Id;
	//�¶Ȧ�x(��y)/��z
	double get_dfSlopeGradient(){return _dfSlopeGradient;}
	double set_dfSlopeGradient(double dfGradient);
	__declspec(property(put=set_dfSlopeGradient,get=get_dfSlopeGradient)) double dfSlopeGradient;
	long set_ziBtmPlane(long ziPlane);		//��������ƽ��Z����
	long get_ziBtmPlane(){return this->xBtmPlane.ziPlane;}		//��������ƽ��Z����
	__declspec(property(put=set_ziBtmPlane,get=get_ziBtmPlane)) long ziBtmPlane;	//���Ȼ���ƽ��Z����
	double set_dfBtmWidth(double dfWidth);	//������Ƹ���
	double get_dfBtmWidth(){return this->xBtmPlane.dfWidth;}	//������Ƹ���
	__declspec(property(put=set_dfBtmWidth,get=get_dfBtmWidth)) double dfBtmWidth;	//���Ȼ����ڵ���Ƹ���
	virtual bool IsLegSegment(){return _ciLegSerial>0;}
};
struct TRUNK_SEGMENT : public PLAN_SECTION
{
/*protected:	//ԭʼ������Ʋ���
	WORD _wiSegHeight;		//�ڼ䴹��
	double _dfTopWidth;		//�Ͽ���
	double _dfSlopeGradient;//�¶Ȧ�x(��y)/��z
protected:	//��������Ľ��ֵ
	double _dfZp2Zh;	//Zp��ʾ����ߣ�Zh��ʾ��ά�ռ��
	double _dfBtmWidth;	//�¿���
public:
	long ziTopPlane;
	UINT idPanelType;*/
	TRUNK_SEGMENT(double dfTopTrunkWidth=4500,double dfGradientCoef=0.11);
public:	//����
	//����ڼ䴹��
	WORD get_wiSegHeight(){return _wiSegHeight;}
	WORD set_wiSegHeight(WORD wiHeight);
	__declspec(property(put=set_wiSegHeight,get=get_wiSegHeight)) WORD wiSegHeight;
	//�Ͽ��ڿ��
	double get_dfTopWidth(){return xTopPlane.dfWidth;}
	double set_dfTopWidth(double dfWidth);
	__declspec(property(put=set_dfTopWidth,get=get_dfTopWidth)) double dfTopWidth;
	//�¿��ڿ��
	double get_dfBtmWidth(){return xBtmPlane.dfWidth;}
	__declspec(property(get=get_dfBtmWidth)) double dfBtmWidth;
	//����ˮƽ�н�
	double get_dfBevelGradDegAng();
	__declspec(property(get=get_dfBevelGradDegAng)) double dfBevelGradDegAng;//����ˮƽ�н�
	//����ڼ�滮��ƺ���
	double CalBevelGradDegAngle();		//���ҹ�ͨ����ˮƽ�н�
	double CalUpHalfBevelGradDegAngle();	//�ϲ���ͨ����ˮƽ�н�
	double CalDownHalfBevelGradDegAngle();	//�²���ͨ����ˮƽ�н�
	bool CalWholeSectHeight(double* pfMinHeight,double* pfMaxHeight,double dfMinBevelDegAngle=25,double dfMaxBevelDegAngle=40);
	bool CalUpHalfSectHeight(double* pfMinHeight,double* pfMaxHeight,double dfMinBevelDegAngle=25,double dfMaxBevelDegAngle=40);
	bool CalDownHalfSectHeight(double* pfMinHeight,double* pfMaxHeight,double dfMinBevelDegAngle=25,double dfMaxBevelDegAngle=40);
};
struct TRUNK_SUBLEG : public PLAN_SECTION{
	TRUNK_SUBLEG();
public:
	BYTE set_ciLegSerial(BYTE ciSerial){return _ciLegSerial=ciSerial;}
	BYTE get_ciLegSerial(){return _ciLegSerial;}
	__declspec(property(put=set_ciLegSerial,get=get_ciLegSerial)) BYTE ciLegSerial;	//���Ⱥ�
	WORD set_wiLegHeight(WORD _wiLegHeight){return this->_wiSegHeight=_wiLegHeight;}			//���ȴ���
	WORD get_wiLegHeight(){return this->_wiSegHeight;}			//���ȴ���
	__declspec(property(put=set_wiLegHeight,get=get_wiLegHeight)) WORD wiLegHeight;	//���ȸ�
};
struct HEIGHTGROUP{
protected:	//ԭʼ������Ʋ���
	BYTE _ciBodySerial;	//1 based serial
public:		//��������Ľ��ֵ
	long hRelaHeightModule;
	UINT uiBodyHeight;	//������mm
	UINT uiNamedHeight;	//�������
	double dfSlopeGradient;		//�¶Ȧ�x(��y)/��z
	TRUNK_PLANE xLegTopPlane,xBtmPlane;	//���߷��������Ӹ���Z���꼰��Ȼ���ƽ��Z����
	static const WORD MAX_SUBLEG_COUNT = 16;
	PRESET_ARRAY16<TRUNK_SUBLEG> xarrSubLegs;
	struct LEGINFO{
		WORD wiLevelDiff;	//mm
		WORD wnLegCount;	//��������
		WORD wiMaxLegHeight;
		long ziMinTopPlane,ziMaxTopPlane;
		union MIDNODE{
			CLDSNode* pNode;
			struct{CLDSNode* pLeftNode,*pRightNode;};
		}xFront,xRight,xBack,xLeft;
	}xSublegInfo;
	long ziCurrPlane;	//�滮�����е���ʱ���������ڱ�ʾ��ǰ�ѹ滮����ڼ�����ƽ��Z����
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
	BYTE ciLegSchema;		//����ģʽ
	WORD wnHeightGroup;		//�滮�ĺ��߷�����
	WORD wnMaxLegCount;
	WORD wiLegDiffLevel;	//�滮�ĺ����ڼ��ȼ���
	UINT uiMinHeight,uiMaxHeight;	//ָ��������С�����Χ(mm)
	UINT uiTopWidth;		//�������ڿ�(mm)
	long ziTopPlane;		//������Zֵ
	long ziHeight0Plane;	//���߼�����ʼ��׼ƽ���Z����
	double dfSlopeGradient;	//�¶Ȧ�x(��y)/��z
	CHashListEx<TRUNK_SEGMENT> hashBodySegments;
public:	//����������ģ�ͻ����Ͻ��й滮ʱ���洢����ģ�͵�������Ϣ
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
		}pants;	//����ýṹ
		EXIST_HEIGHT(){
			_uiNamedHeight=30000;
			hRelaHeightModule=hTransitNode=hPostLegRod=hLowestBaseNode=0;
			pants.hCornerNode=pants.hFrontInnerNode=pants.hSideInnerNode=0;
		}
		void SetKey(DWORD key){_uiNamedHeight=key;}
	public:	//����
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
