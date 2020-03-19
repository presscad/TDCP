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
	f3dPoint pos;		//模板中原参数值,不能更改,否则可能导致对称失败
	f3dPoint actual_pos;//模板插入到铁塔中时形成的真实坐标位置
	char posx_expression[20];
	char posy_expression[20];
	char posz_expression[20];
	char scale_expression[20];
	bool m_bInternodeRepeat;	//节间中的重复节点，重复参数为"i"
	/*
		0-坐标无任何依赖的节点;
		1-杆件上X坐标值不变点;
		2-杆件上Y坐标值不变点;
		3-杆件上Z坐标值不变点;
		4-两杆件交叉交点;
		5-杆件上的比例等分点;
	*/
	short attach_type;	
	short attach_node_index[4];
	double attach_scale;	//正值表示小数形式的比例位置值,负值表示分子.分母
};

class CFacePanelPole{
public:
	CFacePanelPole(){memset(this,0,sizeof(CFacePanelPole));}
	void ToStream(IStream *pStream);
	void FromStream(IStream *pStream);
	char sTitle[16],sRefPole[16],sRefPole2[16],sLamdaRefPole[16];
	char sLayer[4];
	char cMaterial;	//'S':Q235;'H':Q345;'G':Q390;'T':Q420;'U':Q460
	bool m_bInternodeRepeat;	//节间中的重复杆件，重复参数为"i"
	short start_i,end_i;
	short start_force_type,end_force_type;
	short callen_type;
	short start_offset,end_offset;	//默认偏移80
	char sStartOffsetRod[16],sEndOffsetRod[16];
	f3dPoint eyeSideNorm;	//人眼观看侧法线方向
	ANGLE_LAYOUT wxLayout;
};
class CFacePanel  
{
public:
	long m_iFaceType;			//1:塔身标准面2:孤立(横隔)标准面3:多节间标准面4.K型标准面5.V型标准面6.接腿正侧面
	bool m_bInternodeRepeat;	//允许多节间重复杆件
	bool m_bWing2Upward;		//当m_iFaceType表示横隔面时，表示横材是否肢口朝上
	long m_nKeyNode;
	long m_nInternodeNum;		//默认节间数
	long m_nRepeatLessNum;		//重复连接杆件次数相比节间数的差额
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
	BYTE cType;					//标准段号
	BYTE cMirType;				//对称信息0:无对称;1:Y轴对称;2:X轴对称
	BYTE cCalStyle;				//计算方式0:按塔身方式计算风压;1:按X方向水平横担方式计算风压;2:按Y方向水平横担方式计算风压
	double CqiFront,CqiSide;	//正/侧面增大系数
	double BetaZ;				//风振系数或称风荷载调整系数（βz）
	long nodeIndexArr[8];		//八个轮廓节点的索引
};
class CHeadTemplate
{
public:
	long	m_nKeyNode;
	void	UpdateNodePos();
	void ToStream(IStream *pStream);
	void FromStream(IStream *pStream);
	char	sLabel[24];
	BITMAP image;	//存储示意图
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
public:	//与CFacePanelNode兼容所需要的属性
	GEPOINT pos;		//模板中原参数值,不能更改,否则可能导致对称失败
	GEPOINT actualpos;	//模板插入到铁塔中时形成的真实坐标位置
public:	//模板参数
	CXhChar16 label;
	KEY4C keyLabel;				//节点的模板标识
	KEY4C keyLabelFatherRod;	//归属父杆件的模板标识
	bool m_bSetupJoint;			//装配结合点
	char posx_expression[20];
	char posy_expression[20];
	char posz_expression[20];
	char scale_expression[20];
	char exist_logicexpr[20];	//该节点对象是否存在的逻辑表达式，目前该表达式中只能出现一次'<'、'>'、'='、'<='、'>='、'!='逻辑操作符
	//bool m_bInternodeRepeat;	//节间中的重复节点，重复参数为"i"
	KEY4C keyRepeatParam;		//>0时表示代表该节点重复次数的CBlockParaModel::listParams参数键值，重复参数为"i"；其值为重复节点所归属的节点重复轴线的标识
public:	//坐标计算类型，attach_type
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
		0-坐标无任何依赖的节点;
		1-杆件上X坐标值不变点;
		2-杆件上Y坐标值不变点;
		3-杆件上Z坐标值不变点;
		4-两杆件交叉交点;
		5-杆件上的比例等分点;
	*/
	short attach_type;	
public:
	BYTE m_cPosCalViceType;	//表示投影面类型, 0:表示交叉面为投影面,1:X-Z平面;2:Y-Z平面;3:X-Y平面
	double attach_scale;	//正值表示小数形式的比例位置值,负值表示分子.分母
	KEY4C attachNodeArr[4];
	KEY4C keyLabelOtherRod;		//根据两角钢求交叉点的依附杆件
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
	KEY4C keyLabel;	//标签（字符形式的标识可用于参数化建模时代替Id索引，有别于构件编号）
	char sLayer[4];
	char cMaterial;	//'S':Q235;'H':Q345;'G':Q390;'T':Q420;'U':Q460
	char exist_logicexpr[20];	//该节点对象是否存在的逻辑表达式，目前该表达式中只能出现一次'<'、'>'、'='、'<='、'>='、'!='逻辑操作符
public:
	//static const int CLS_LINEANGLE = 13;
	//static const int CLS_LINETUBE  = 17;
	int m_idPartClsType;	//该杆件的构件类型，如角钢、钢管、钢板等, CLS_LINEANGLE,CLS_LINETUBE
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
	BYTE  nIterStep;	//重复杆件时节点索引的增长步长, 默认为1
	CXhChar16 indexParaStart,indexParaEnd;	//对于可重复节点的索引参数，如"i"、"i+1"、"i+N"等
	BYTE start_joint_type,end_joint_type;
	BYTE start_force_type,end_force_type;
	short callen_type;
	short layout_style;		//角钢的布置方式（里外铁，口朝上口朝下），取值0,1,2,3
	short m_iElemType;		//0表示自动判断;1表示二力杆桁架;2表示经典梁单元;3表示柔索单元
	f3dPoint eyeSideNorm;	//人眼观看侧法线方向
public:
	//JOINT_TYPE 类型的枚举值
	static const BYTE JOINT_HINGE	= 0;	//铰接
	static const BYTE JOINT_RIGID	= 1;	//刚接
	static const BYTE JOINT_AUTOJUSTIFY = 2;//自动判断
	//杆件两端的受力类型 FORCE_TYPE
	static const BYTE CENTRIC_FORCE		= 0;//中心受力
	static const BYTE ECCENTRIC_FORCE	= 1;//偏心受力
	//杆件的有限单元承力类型 ELEM_TYPE
	static const BYTE FEMELEM_TRUSS	= 1;//二力杆桁架单元
	static const BYTE FEMELEM_BEAM	= 2;//经典梁单元
	static const BYTE FEMELEM_GUYS	= 3;//柔索单元
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
		BYTE cTouchLine;			//0.X肢心线;1.Y肢心线;2.楞线;3.自动判断连接工作肢
		BYTE cTouchLineType;		//0.心线;1.火曲线;2.偏移准线;3.控制点;4.实体共面点
		GEPOINT face_offset_norm;	//平推面法线
		KEY4C labelHuoquRod;
		BYTE cPlanePtType;	//0.指定XY坐标点;1.指定YZ坐标点;2.指定XZ坐标点
		KEY4C keyLabelPlane;	//基准面标签
		BYTE cSymmetryType;	//参照点对称方式 0.无对称参照 1.X轴对称 2.Y轴对称 3.Z轴对称
		BYTE coordCtrlType;	//0:X-Z主控;1:Y-Z主控;2.X-Y主控(仅在spatialOperationStyle=15时有效)
		BYTE cMainCoordCtrlWing;//角钢主控肢：'X'肢心线上关键点;'Y'肢心线上关键点;其余楞线上关键点
		struct WING_OFFSET_STYLE{
			BYTE gStyle;		//角钢肢偏移量:0.标准准距;1:g1;2:g2;3:g3;4:自定义
			BYTE operStyle;		//操作类型0.垂向投影;1.X-Y平面平推;2.Y-Z平面平推;3.X-Z平面平推;4.自定义平推面
			WING_OFFSET_STYLE(){gStyle=operStyle=0;}
		}XOrCtrlWing,YOrOtherWing;//角钢主要控制肢(控制两个坐标分量的肢),角钢辅助控制肢(控制一个坐标分量的肢)
	}desStart,desEnd;
	//安装定位信息
	BYTE cPosCtrlDatumLineType;	//搭接定位基准 'X':X肢标准心线控制;'Y':Y肢标准心线控制;无
	BYTE cPosCtrlType;			//搭接点定位方式 0.垂向投影;1.X-Y平面平推;2.Y-Z平面平推;3.X-Z平面平推;4.自定义平推面
public:
	CParaAngle();
	virtual ~CParaAngle();
	virtual bool IsAngleObject(){return true;}
	virtual void CopyParaRod(CParaRod* pRod);
	virtual void ToStream(IStream *pStream,double version=0);
	virtual void FromStream(IStream *pStream,double version=0);
};
//	CparaDatumPlane 实体基准面
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
	ATOM_LIST<TAG_PARANODE_INFO>listTagNodeInfo;		//临时挂接的附加到构件项{PART_ITEM}上的数据对象链表
	ATOM_LIST<TAG_PARAROD_INFO>listTagRodInfo;		//临时挂接的附加到构件项{PART_ITEM}上的数据对象链表
private:
	bool IsLegacyHeadTemplate(IStream* pStream);
public:
	bool MigrateFromHeadTemplate(CHeadTemplate* pHeadTempl);
	static const int SUBSTATION_TEMPLATE	= 0;		//变电构架模版
	static const int CROSSARM_TEMPLATE		= 1;		//典型横担模版
public:
	char sLabel[24];
	BITMAP image;	//存储示意图
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
	bool AssertParamsValid();//调用各模板中应支持的部件参数合法性校验
	//void RebuildModel();		//创建几何模型, 已实现
	//void ResetPosition();
	//bool MechanicsDesign(CParaModel* pBlockModel);
	//bool StructuralDesign();
};
EXPRESSION_VAR ConvertParamItemToExprVar(DESIGN_PARAM_ITEM *pItem);
#endif