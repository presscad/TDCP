#ifndef __STD_FACE_PANEL_DEF_H_
#define __STD_FACE_PANEL_DEF_H_

#include "f_ent.h"
#include "f_alg_fun.h"
#include "f_ent_list.h"
#include "Expression.h"
#include "AngleLayout.h"

class CHeadParameter{
public:
	CHeadParameter();
	void ToStream(IStream *pStream);
	void FromStream(IStream *pStream);
	EXPRESSION_VAR var;
	char description[50];
};
class CFacePanelNode{
public:
	CFacePanelNode();
	void ToStream(IStream *pStream);
	void FromStream(IStream *pStream);
	char sTitle[16];
	f3dPoint pos;		//ģ����ԭ����ֵ,���ܸ���,������ܵ��¶Գ�ʧ��
	f3dPoint actual_pos;//ģ����뵽������ʱ�γɵ���ʵ����λ��
	char posx_expression[20];
	char posy_expression[20];
	char posz_expression[20];
	char scale_expression[20];
	bool m_bInternodeRepeat;	//�ڼ��е��ظ��ڵ㣬�ظ�����Ϊ"i"
	/*
		0-�������κ������Ľڵ�;
		1-�˼���X����ֵ�����;
		2-�˼���Y����ֵ�����;
		3-�˼���Z����ֵ�����;
		4-���˼����潻��;
		5-�˼��ϵı����ȷֵ�;
	*/
	short attach_type;	
	short attach_node_index[4];
	double attach_scale;	//��ֵ��ʾС����ʽ�ı���λ��ֵ,��ֵ��ʾ����.��ĸ
};

class CFacePanelPole{
public:
	CFacePanelPole(){memset(this,0,sizeof(CFacePanelPole));}
	void ToStream(IStream *pStream);
	void FromStream(IStream *pStream);
	char sTitle[16],sRefPole[16],sRefPole2[16],sLamdaRefPole[16];
	char sLayer[4];
	char cMaterial;	//'S':Q235;'H':Q345;'G':Q390;'T':Q420;'U':Q460
	bool m_bInternodeRepeat;	//�ڼ��е��ظ��˼����ظ�����Ϊ"i"
	short start_i,end_i;
	short start_force_type,end_force_type;
	short callen_type;
	short start_offset,end_offset;	//Ĭ��ƫ��80
	char sStartOffsetRod[16],sEndOffsetRod[16];
	f3dPoint eyeSideNorm;	//���۹ۿ��෨�߷���
	ANGLE_LAYOUT wxLayout;
};
class CFacePanel  
{
public:
	long m_iFaceType;			//1:�����׼��2:����(���)��׼��3:��ڼ��׼��4.K�ͱ�׼��5.V�ͱ�׼��6.����������
	bool m_bInternodeRepeat;	//�����ڼ��ظ��˼�
	bool m_bWing2Upward;		//��m_iFaceType��ʾ�����ʱ����ʾ����Ƿ�֫�ڳ���
	long m_nKeyNode;
	long m_nInternodeNum;		//Ĭ�Ͻڼ���
	long m_nRepeatLessNum;		//�ظ����Ӹ˼�������Ƚڼ����Ĳ��
	void UpdateNodePos();
	char sLabel[24];
	CFacePanel();
	virtual ~CFacePanel();
	bool IsRepeateType();
	void ToStream(IStream *pStream);
	void FromStream(IStream *pStream);
	ATOM_LIST<CFacePanelNode>node_list;
	ATOM_LIST<CFacePanelPole>pole_list;
	bool ReadFromLibFile(char* lib_file,char *label);
};
class CFaceWindSegment{
public:
	CFaceWindSegment();
	int GetMaxNodeCount(){return sizeof(nodeIndexArr)/4;}
	void ToStream(IStream *pStream);
	void FromStream(IStream *pStream);
public:
	BYTE cType;					//��׼�κ�
	BYTE cMirType;				//�Գ���Ϣ0:�޶Գ�;1:Y��Գ�;2:X��Գ�
	BYTE cCalStyle;				//���㷽ʽ0:������ʽ�����ѹ;1:��X����ˮƽ�ᵣ��ʽ�����ѹ;2:��Y����ˮƽ�ᵣ��ʽ�����ѹ
	double CqiFront,CqiSide;	//��/��������ϵ��
	double BetaZ;				//����ϵ����Ʒ���ص���ϵ������z��
	long nodeIndexArr[8];		//�˸������ڵ������
};
class CHeadTemplate
{
public:
	long	m_nKeyNode;
	void	UpdateNodePos();
	void ToStream(IStream *pStream);
	void FromStream(IStream *pStream);
	char	sLabel[24];
	BITMAP image;	//�洢ʾ��ͼ
	CHeadTemplate();
	virtual ~CHeadTemplate();
	UCS_STRU rot_ucs;
	ATOM_LIST<CHeadParameter>para_list;
	ATOM_LIST<CFacePanelNode>node_list;
	ATOM_LIST<CFacePanelPole>pole_list;
	ATOM_LIST<CFaceWindSegment>windseg_list;
};
//////////////////////////////////////////////////////////////////////////
#include "LifeObj.h"
#include "ComplexId.h"
#include "DesignParamItem.h"
#include "HashTable.h"
#include "ArrayList.h"
class CParaNode;
class CParaRod;
class CParaAngle;
class CLDSNode;
class CLDSLinePart;

struct TAG_PARANODE_INFO : public TAG_INFO<CParaNode>
{
	CLDSNode* pModelNode;
	TAG_PARANODE_INFO(){pModelNode=NULL;}
};
struct TAG_PARAROD_INFO : public TAG_INFO<CParaRod>
{
	CLDSLinePart* pModelRod;
	TAG_PARAROD_INFO(){pModelRod=NULL;}
};
class CParaNode{
protected:
	TAG_PARANODE_INFO* pTagInfo;
public:	//��CFacePanelNode��������Ҫ������
	GEPOINT pos;		//ģ����ԭ����ֵ,���ܸ���,������ܵ��¶Գ�ʧ��
	GEPOINT actualpos;	//ģ����뵽������ʱ�γɵ���ʵ����λ��
public:	//ģ�����
	CXhChar16 label;
	KEY4C keyLabel;				//�ڵ��ģ���ʶ
	KEY4C keyLabelFatherRod;	//�������˼���ģ���ʶ
	bool m_bSetupJoint;			//װ���ϵ�
	char posx_expression[20];
	char posy_expression[20];
	char posz_expression[20];
	char scale_expression[20];
	char exist_logicexpr[20];	//�ýڵ�����Ƿ���ڵ��߼����ʽ��Ŀǰ�ñ��ʽ��ֻ�ܳ���һ��'<'��'>'��'='��'<='��'>='��'!='�߼�������
	//bool m_bInternodeRepeat;	//�ڼ��е��ظ��ڵ㣬�ظ�����Ϊ"i"
	KEY4C keyRepeatParam;		//>0ʱ��ʾ����ýڵ��ظ�������CBlockParaModel::listParams������ֵ���ظ�����Ϊ"i"����ֵΪ�ظ��ڵ��������Ľڵ��ظ����ߵı�ʶ
public:	//����������ͣ�attach_type
	static const BYTE INDEPENDENT		 = 0;
	static const BYTE COORD_X_SPECIFIED  = 1;
	static const BYTE COORD_Y_SPECIFIED  = 2;
	static const BYTE COORD_Z_SPECIFIED  = 3;
	static const BYTE INTERSECTION		 = 4;
	static const BYTE SCALE				 = 5;
	static const BYTE PLANE_XY_SPECIFIED = 6;
	static const BYTE PLANE_YZ_SPECIFIED = 7;
	static const BYTE PLANE_XZ_SPECIFIED = 8;
	/*
		0-�������κ������Ľڵ�;
		1-�˼���X����ֵ�����;
		2-�˼���Y����ֵ�����;
		3-�˼���Z����ֵ�����;
		4-���˼����潻��;
		5-�˼��ϵı����ȷֵ�;
	*/
	short attach_type;	
public:
	BYTE m_cPosCalViceType;	//��ʾͶӰ������, 0:��ʾ������ΪͶӰ��,1:X-Zƽ��;2:Y-Zƽ��;3:X-Yƽ��
	double attach_scale;	//��ֵ��ʾС����ʽ�ı���λ��ֵ,��ֵ��ʾ����.��ĸ
	KEY4C attachNodeArr[4];
	KEY4C keyLabelOtherRod;		//�������Ǹ��󽻲��������˼�
public:
	CParaNode();
	virtual ~CParaNode();
	void SetTagInfo(void* pTag){pTagInfo=(TAG_PARANODE_INFO*)pTag;}
	void SetKey(DWORD key){keyLabel=key;}
	TAG_PARANODE_INFO* TagInfo(){return pTagInfo;}
	virtual void ToStream(IStream *pStream);
	virtual void FromStream(IStream *pStream);
};
typedef CParaNode* CParaNodePtr;
class CParaPart{
public:
	CXhChar16 label;
	KEY4C keyLabel;	//��ǩ���ַ���ʽ�ı�ʶ�����ڲ�������ģʱ����Id�������б��ڹ�����ţ�
	char sLayer[4];
	char cMaterial;	//'S':Q235;'H':Q345;'G':Q390;'T':Q420;'U':Q460
	char exist_logicexpr[20];	//�ýڵ�����Ƿ���ڵ��߼����ʽ��Ŀǰ�ñ��ʽ��ֻ�ܳ���һ��'<'��'>'��'='��'<='��'>='��'!='�߼�������
public:
	//static const int CLS_LINEANGLE = 13;
	//static const int CLS_LINETUBE  = 17;
	int m_idPartClsType;	//�ø˼��Ĺ������ͣ���Ǹ֡��ֹܡ��ְ��, CLS_LINEANGLE,CLS_LINETUBE
public:
	CParaPart();
	virtual void SetKey(DWORD key){keyLabel=key;}
	virtual void ToStream(IStream *pStream){};
	virtual void FromStream(IStream *pStream){};
};
class CParaRod : public CParaPart{
protected:
	TAG_PARAROD_INFO* pTagInfo;
public:
	KEY4C keyRefPole,keyRefPole2,keyLamdaRefPole;
	KEY4C keyStartNode,keyEndNode;
	BYTE  nIterStep;	//�ظ��˼�ʱ�ڵ���������������, Ĭ��Ϊ1
	CXhChar16 indexParaStart,indexParaEnd;	//���ڿ��ظ��ڵ��������������"i"��"i+1"��"i+N"��
	BYTE start_joint_type,end_joint_type;
	BYTE start_force_type,end_force_type;
	short callen_type;
	short layout_style;		//�ǸֵĲ��÷�ʽ�����������ڳ��Ͽڳ��£���ȡֵ0,1,2,3
	short m_iElemType;		//0��ʾ�Զ��ж�;1��ʾ���������;2��ʾ��������Ԫ;3��ʾ������Ԫ
	f3dPoint eyeSideNorm;	//���۹ۿ��෨�߷���
public:
	//JOINT_TYPE ���͵�ö��ֵ
	static const BYTE JOINT_HINGE	= 0;	//�½�
	static const BYTE JOINT_RIGID	= 1;	//�ս�
	static const BYTE JOINT_AUTOJUSTIFY = 2;//�Զ��ж�
	//�˼����˵��������� FORCE_TYPE
	static const BYTE CENTRIC_FORCE		= 0;//��������
	static const BYTE ECCENTRIC_FORCE	= 1;//ƫ������
	//�˼������޵�Ԫ�������� ELEM_TYPE
	static const BYTE FEMELEM_TRUSS	= 1;//��������ܵ�Ԫ
	static const BYTE FEMELEM_BEAM	= 2;//��������Ԫ
	static const BYTE FEMELEM_GUYS	= 3;//������Ԫ
public:
	CParaRod();
	virtual ~CParaRod();
	virtual bool IsAngleObject(){return false;}
	virtual void CopyParaRod(CParaRod* pRod);
	void SetTagInfo(void* pTag){pTagInfo=(TAG_PARAROD_INFO*)pTag;}
	TAG_PARAROD_INFO* TagInfo(){return pTagInfo;}
	virtual void ToStream(IStream *pStream,double version=0);
	virtual void FromStream(IStream *pStream,double version=0);
};
class CParaAngle : public CParaRod{
public:
	struct POS{
		BYTE cTouchLine;			//0.X֫����;1.Y֫����;2.����;3.�Զ��ж����ӹ���֫
		BYTE cTouchLineType;		//0.����;1.������;2.ƫ��׼��;3.���Ƶ�;4.ʵ�干���
		GEPOINT face_offset_norm;	//ƽ���淨��
		KEY4C labelHuoquRod;
		BYTE cPlanePtType;	//0.ָ��XY�����;1.ָ��YZ�����;2.ָ��XZ�����
		KEY4C keyLabelPlane;	//��׼���ǩ
		BYTE cSymmetryType;	//���յ�ԳƷ�ʽ 0.�޶ԳƲ��� 1.X��Գ� 2.Y��Գ� 3.Z��Գ�
		BYTE coordCtrlType;	//0:X-Z����;1:Y-Z����;2.X-Y����(����spatialOperationStyle=15ʱ��Ч)
		BYTE cMainCoordCtrlWing;//�Ǹ�����֫��'X'֫�����Ϲؼ���;'Y'֫�����Ϲؼ���;���������Ϲؼ���
		struct WING_OFFSET_STYLE{
			BYTE gStyle;		//�Ǹ�֫ƫ����:0.��׼׼��;1:g1;2:g2;3:g3;4:�Զ���
			BYTE operStyle;		//��������0.����ͶӰ;1.X-Yƽ��ƽ��;2.Y-Zƽ��ƽ��;3.X-Zƽ��ƽ��;4.�Զ���ƽ����
			WING_OFFSET_STYLE(){gStyle=operStyle=0;}
		}XOrCtrlWing,YOrOtherWing;//�Ǹ���Ҫ����֫(�����������������֫),�Ǹָ�������֫(����һ�����������֫)
	}desStart,desEnd;
	//��װ��λ��Ϣ
	BYTE cPosCtrlDatumLineType;	//��Ӷ�λ��׼ 'X':X֫��׼���߿���;'Y':Y֫��׼���߿���;��
	BYTE cPosCtrlType;			//��ӵ㶨λ��ʽ 0.����ͶӰ;1.X-Yƽ��ƽ��;2.Y-Zƽ��ƽ��;3.X-Zƽ��ƽ��;4.�Զ���ƽ����
public:
	CParaAngle();
	virtual ~CParaAngle();
	virtual bool IsAngleObject(){return true;}
	virtual void CopyParaRod(CParaRod* pRod);
	virtual void ToStream(IStream *pStream,double version=0);
	virtual void FromStream(IStream *pStream,double version=0);
};
//	CparaDatumPlane ʵ���׼��
class CParaDatumPlane
{
public:
	struct DATUM_POS{
		KEY4C keyRefRod;
		BOOL bStart;
	}XFirPos,XSecPos,XThirPos;
	KEY4C keyLabel;
	CXhChar16 label;
	CXhChar100 description;
public:
	CParaDatumPlane();
	virtual ~CParaDatumPlane();
	void SetKey(DWORD key){keyLabel=key;}
	void ToStream(IStream *pStream,double version=0);
	void FromStream(IStream *pStream,double version=0);
};
class CParaTemplate
{
	ATOM_LIST<TAG_PARANODE_INFO>listTagNodeInfo;		//��ʱ�ҽӵĸ��ӵ�������{PART_ITEM}�ϵ����ݶ�������
	ATOM_LIST<TAG_PARAROD_INFO>listTagRodInfo;		//��ʱ�ҽӵĸ��ӵ�������{PART_ITEM}�ϵ����ݶ�������
private:
	bool IsLegacyHeadTemplate(IStream* pStream);
public:
	bool MigrateFromHeadTemplate(CHeadTemplate* pHeadTempl);
	static const int SUBSTATION_TEMPLATE	= 0;		//��繹��ģ��
	static const int CROSSARM_TEMPLATE		= 1;		//���ͺᵣģ��
public:
	char sLabel[24];
	BITMAP image;	//�洢ʾ��ͼ
	UCS_STRU rot_ucs;
	CHashListEx<DESIGN_PARAM_ITEM>listParams;
	CHashListEx<CParaNode>listParaNodes;
	CSuperHashList<CParaRod>listParaRods;
	CHashListEx<CParaDatumPlane> listParaPlanes;
	ATOM_LIST<CFaceWindSegment>windseg_list;
public:
	CParaTemplate();
	virtual ~CParaTemplate();
	long KeyNodeNum();
	void UpdateNodePos();
	void ToStream(IStream *pStream);
	void FromStream(IStream *pStream);
	bool AssertParamsValid();//���ø�ģ����Ӧ֧�ֵĲ��������Ϸ���У��
	//void RebuildModel();		//��������ģ��, ��ʵ��
	//void ResetPosition();
	//bool MechanicsDesign(CParaModel* pBlockModel);
	//bool StructuralDesign();
};
EXPRESSION_VAR ConvertParamItemToExprVar(DESIGN_PARAM_ITEM *pItem);
#endif