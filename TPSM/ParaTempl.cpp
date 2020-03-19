#include "stdafx.h"
#include "TPSM.h"
#include "GlobalFunc.h"
#include "StdFacePanel.h"
#include "ximapng.h"
#include "ImageFile.h"
#include "zlib.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// CParaNode Construction/Destruction
//////////////////////////////////////////////////////////////////////
CParaNode::CParaNode()
{
	pTagInfo=NULL;
	m_bSetupJoint=false;
	strcpy(posx_expression,"0");
	strcpy(posy_expression,"0");
	strcpy(posz_expression,"0");
	strcpy(scale_expression,"");
	strcpy(exist_logicexpr,"");
	keyRepeatParam.dwItem=0;	//>0ʱ��ʾ�ýڵ�Ϊ�ڼ��е��ظ��ڵ㣬�ظ�����Ϊ"i"����ֵΪ�ظ��ڵ��������Ľڵ��ظ����ߵı�ʶ
	/*
		0-�������κ������Ľڵ�;
		1-�˼���X����ֵ�����;
		2-�˼���Y����ֵ�����;
		3-�˼���Z����ֵ�����;
		4-���˼����潻��;
		5-�˼��ϵı����ȷֵ�;
	*/
	attach_type=0;	
	memset(attachNodeArr,0,sizeof(short)*4);
	attach_scale=0;	//��ֵ��ʾС����ʽ�ı���λ��ֵ,��ֵ��ʾ����.��ĸ
	m_cPosCalViceType=0;
}
CParaNode::~CParaNode()
{
	if(pTagInfo)
		pTagInfo->DetachObject();
}
void CParaNode::FromStream(IStream *pStream)
{
	double version=0;
	pStream->Read(&version,sizeof(double),NULL);
	ReadStringFromStream(pStream,label);
	pStream->Read(&keyLabelFatherRod,4,NULL);
	pStream->Read(&keyRepeatParam,4,NULL);
	pStream->Read(&pos.x,sizeof(double),NULL);
	pStream->Read(&pos.y,sizeof(double),NULL);
	pStream->Read(&pos.z,sizeof(double),NULL);
	pStream->Read(&m_bSetupJoint,sizeof(bool),NULL);
	//��ȡ���ʽ
	ReadStringFromStream(pStream,posx_expression);
	ReadStringFromStream(pStream,posy_expression);
	ReadStringFromStream(pStream,posz_expression);
	ReadStringFromStream(pStream,scale_expression);
	ReadStringFromStream(pStream,exist_logicexpr);

	pStream->Read(&attach_type,sizeof(short),NULL);
	pStream->Read(&m_cPosCalViceType,1,NULL);
	pStream->Read(&attach_scale,sizeof(double),NULL);
	pStream->Read(&attachNodeArr[0],4,NULL);
	pStream->Read(&attachNodeArr[1],4,NULL);
	pStream->Read(&attachNodeArr[2],4,NULL);
	pStream->Read(&attachNodeArr[3],4,NULL);
	if(version>=2.3)
		pStream->Read(&keyLabelOtherRod,4,NULL);
}
void CParaNode::ToStream(IStream *pStream)
{
	double version=0;
	pStream->Write(&version,sizeof(double),NULL);
	WriteStringToStream(pStream,label);
	pStream->Write(&keyLabelFatherRod,4,NULL);
	pStream->Write(&keyRepeatParam,4,NULL);
	pStream->Write(&pos.x,sizeof(double),NULL);
	pStream->Write(&pos.y,sizeof(double),NULL);
	pStream->Write(&pos.z,sizeof(double),NULL);
	pStream->Write(&m_bSetupJoint,sizeof(bool),NULL);
	//��ȡ���ʽ
	WriteStringToStream(pStream,posx_expression);
	WriteStringToStream(pStream,posy_expression);
	WriteStringToStream(pStream,posz_expression);
	WriteStringToStream(pStream,scale_expression);
	WriteStringToStream(pStream,exist_logicexpr);

	pStream->Write(&attach_type,sizeof(short),NULL);
	pStream->Write(&m_cPosCalViceType,1,NULL);
	pStream->Write(&attach_scale,sizeof(double),NULL);
	pStream->Write(&attachNodeArr[0],4,NULL);
	pStream->Write(&attachNodeArr[1],4,NULL);
	pStream->Write(&attachNodeArr[2],4,NULL);
	pStream->Write(&attachNodeArr[3],4,NULL);
	pStream->Write(&keyLabelOtherRod,4,NULL);
}
//////////////////////////////////////////////////////////////////////
// CParaPart Construction/Destruction
//////////////////////////////////////////////////////////////////////
CParaPart::CParaPart()
{
	memset(this,0,sizeof(CParaPart));
	m_idPartClsType=CLS_LINEANGLE;
	strcpy(exist_logicexpr,"1=0");
}
CParaRod::CParaRod()
{
	pTagInfo=NULL;
	start_joint_type=end_joint_type=JOINT_AUTOJUSTIFY;
	start_force_type=end_force_type=0;
	nIterStep = 1;
	layout_style=0;
	callen_type=0;
	m_iElemType=0;	//��ʾ�Զ��ж����޵�Ԫ�ĳ�������
}
CParaRod::~CParaRod()
{
	if(pTagInfo)
		pTagInfo->DetachObject();
}
void CParaRod::CopyParaRod(CParaRod* pRod)
{
	keyLabel=pRod->keyLabel;	//��ǩ���ַ���ʽ�ı�ʶ�����ڲ�������ģʱ����Id�������б��ڹ�����ţ�
	strcpy(sLayer,pRod->sLayer);
	m_idPartClsType=pRod->m_idPartClsType;;	//�ø˼��Ĺ������ͣ���Ǹ֡��ֹܡ��ְ��, CLS_LINEANGLE,CLS_LINETUBE
	cMaterial=pRod->cMaterial;	//'S':Q235;'H':Q345;'G':Q390;'T':Q420;'U':Q460
	strncpy(exist_logicexpr,pRod->exist_logicexpr,19);	//�ýڵ�����Ƿ���ڵ��߼����ʽ��Ŀǰ�ñ��ʽ��ֻ�ܳ���һ��'<'��'>'��'='��'<='��'>='��'!='�߼�������
	keyRefPole=pRod->keyRefPole;
	keyRefPole2=pRod->keyRefPole2;
	keyLamdaRefPole=pRod->keyLamdaRefPole;
	keyStartNode=pRod->keyStartNode;
	keyEndNode=pRod->keyEndNode;
	nIterStep=pRod->nIterStep;	//�ظ��˼�ʱ�ڵ���������������, Ĭ��Ϊ1
	indexParaStart=pRod->indexParaStart;
	indexParaEnd=pRod->indexParaEnd;	//���ڿ��ظ��ڵ��������������"i"��"i+1"��"i+N"��
	start_joint_type=pRod->start_joint_type;
	end_joint_type=pRod->end_joint_type;
	start_force_type=pRod->start_force_type;
	end_force_type=pRod->end_force_type;
	callen_type=pRod->callen_type;
	layout_style=pRod->layout_style;		//�ǸֵĲ��÷�ʽ�����������ڳ��Ͽڳ��£���ȡֵ0,1,2,3
	m_iElemType=pRod->m_iElemType;		//0��ʾ�Զ��ж�;1��ʾ���������;2��ʾ��������Ԫ;3��ʾ������Ԫ
	eyeSideNorm=pRod->eyeSideNorm;	//���۹ۿ��෨�߷���
}
void CParaRod::ToStream(IStream *pStream,double version)
{
	WriteStringToStream(pStream,label);
	pStream->Write(sLayer,4,NULL);
	pStream->Write(&cMaterial,1,NULL);	//'S':Q235;'H':Q345;'G':Q390;'T':Q420;'U':Q460
	WriteStringToStream(pStream,exist_logicexpr);	//�ýڵ�����Ƿ���ڵ��߼����ʽ��Ŀǰ�ñ��ʽ��ֻ�ܳ���һ��'<'��'>'��'='��'<='��'>='��'!='�߼�������
	pStream->Write(&keyRefPole,4,NULL);
	pStream->Write(&keyRefPole2,4,NULL);
	pStream->Write(&keyLamdaRefPole,4,NULL);
	pStream->Write(&keyStartNode,4,NULL);
	pStream->Write(&keyEndNode,4,NULL);
	pStream->Write(&nIterStep,1,NULL);	//�ظ��˼�ʱ�ڵ���������������, Ĭ��Ϊ1
	pStream->Write(&indexParaStart,4,NULL);
	pStream->Write(&indexParaEnd,4,NULL);	//���ڿ��ظ��ڵ��������������"i"��"i+1"��"i+N"��
	pStream->Write(&start_joint_type,1,NULL);
	pStream->Write(&end_joint_type,1,NULL);
	pStream->Write(&start_force_type,1,NULL);
	pStream->Write(&end_force_type,1,NULL);
	pStream->Write(&callen_type,2,NULL);
	pStream->Write(&layout_style,2,NULL);		//�ǸֵĲ��÷�ʽ�����������ڳ��Ͽڳ��£���ȡֵ0,1,2,3
	pStream->Write(&m_iElemType,2,NULL);		//0��ʾ�Զ��ж�;1��ʾ���������;2��ʾ��������Ԫ;3��ʾ������Ԫ
	pStream->Write((double*)eyeSideNorm,24,NULL);	//���۹ۿ��෨�߷���
}
void CParaRod::FromStream(IStream *pStream,double version)
{
	ReadStringFromStream(pStream,label);
	pStream->Read(sLayer,4,NULL);
	pStream->Read(&cMaterial,1,NULL);	//'S':Q235;'H':Q345;'G':Q390;'T':Q420;'U':Q460
	ReadStringFromStream(pStream,exist_logicexpr);	//�ýڵ�����Ƿ���ڵ��߼����ʽ��Ŀǰ�ñ��ʽ��ֻ�ܳ���һ��'<'��'>'��'='��'<='��'>='��'!='�߼�������
	pStream->Read(&keyRefPole,4,NULL);
	pStream->Read(&keyRefPole2,4,NULL);
	pStream->Read(&keyLamdaRefPole,4,NULL);
	pStream->Read(&keyStartNode,4,NULL);
	pStream->Read(&keyEndNode,4,NULL);
	pStream->Read(&nIterStep,1,NULL);	//�ظ��˼�ʱ�ڵ���������������, Ĭ��Ϊ1
	pStream->Read(&indexParaStart,4,NULL);
	pStream->Read(&indexParaEnd,4,NULL);	//���ڿ��ظ��ڵ��������������"i"��"i+1"��"i+N"��
	pStream->Read(&start_joint_type,1,NULL);
	pStream->Read(&end_joint_type,1,NULL);
	pStream->Read(&start_force_type,1,NULL);
	pStream->Read(&end_force_type,1,NULL);
	pStream->Read(&callen_type,2,NULL);
	pStream->Read(&layout_style,2,NULL);		//�ǸֵĲ��÷�ʽ�����������ڳ��Ͽڳ��£���ȡֵ0,1,2,3
	pStream->Read(&m_iElemType,2,NULL);		//0��ʾ�Զ��ж�;1��ʾ���������;2��ʾ��������Ԫ;3��ʾ������Ԫ
	pStream->Read((double*)eyeSideNorm,24,NULL);	//���۹ۿ��෨�߷���
}
//////////////////////////////////////////////////////////////////////
// CParaAngle Construction/Destruction
//////////////////////////////////////////////////////////////////////
CParaAngle::CParaAngle()
{
	desStart.cTouchLine=0;			//0.X֫����;1.Y֫����;2.����;3.���Ƶ�
	desStart.cTouchLineType=0;		//0.����;1.������;2.ƫ��׼��
	desStart.face_offset_norm.Set(0,0,1);	//ƽ���淨��
	desStart.coordCtrlType=0;	//0:X-Z����;1:Y-Z����;2.X-Y����(����spatialOperationStyle=15ʱ��Ч)
	desStart.cMainCoordCtrlWing='X';//�Ǹ�����֫��'X'֫�����Ϲؼ���;'Y'֫�����Ϲؼ���;���������Ϲؼ���
	desStart.cPlanePtType=0;
	desStart.cSymmetryType=0;
	desEnd=desStart;
	cPosCtrlDatumLineType=0;
	cPosCtrlType=0;	
}
CParaAngle::~CParaAngle()
{
}
void CParaAngle::CopyParaRod(CParaRod* pRod)
{
	CParaRod::CopyParaRod(pRod);
	if(pRod->IsAngleObject())
	{
		desStart=((CParaAngle*)pRod)->desStart;
		desEnd  =((CParaAngle*)pRod)->desEnd;
	}
}
void CParaAngle::ToStream(IStream *pStream,double version)
{
	CParaRod::ToStream(pStream,version);
	pStream->Write(&desStart.cTouchLine,1,NULL);			//0.X֫����;1.Y֫����;2.����;3.���Ƶ�
	pStream->Write(&desStart.cTouchLineType,1,NULL);		//0.����;1.������;2.ƫ��׼��
	pStream->Write(&desStart.labelHuoquRod,4,NULL);	//0:X-Z����;1:Y-Z����;2.X-Y����(����spatialOperationStyle=15ʱ��Ч)
	pStream->Write(&desStart.face_offset_norm.x,8,NULL);	//ƽ���淨��
	pStream->Write(&desStart.face_offset_norm.y,8,NULL);	//ƽ���淨��
	pStream->Write(&desStart.face_offset_norm.z,8,NULL);	//ƽ���淨��
	pStream->Write(&desStart.coordCtrlType,1,NULL);	//0:X-Z����;1:Y-Z����;2.X-Y����(����spatialOperationStyle=15ʱ��Ч)
	pStream->Write(&desStart.cMainCoordCtrlWing,1,NULL);//�Ǹ�����֫��'X'֫�����Ϲؼ���;'Y'֫�����Ϲؼ���;���������Ϲؼ���
	pStream->Write(&desStart.XOrCtrlWing.gStyle,1,NULL);
	pStream->Write(&desStart.XOrCtrlWing.operStyle,1,NULL);
	pStream->Write(&desStart.YOrOtherWing.gStyle,1,NULL);
	pStream->Write(&desStart.YOrOtherWing.operStyle,1,NULL);
	if(desStart.cTouchLineType==4)
	{
		pStream->Write(&desStart.cPlanePtType,1,NULL);
		pStream->Write(&desStart.cSymmetryType,1,NULL);
		pStream->Write(&desStart.keyLabelPlane,4,NULL);
	}

	pStream->Write(&desEnd.cTouchLine,1,NULL);			//0.X֫����;1.Y֫����;2.����;3.���Ƶ�
	pStream->Write(&desEnd.cTouchLineType,1,NULL);		//0.����;1.������;2.ƫ��׼��
	pStream->Write(&desEnd.labelHuoquRod,4,NULL);	//0:X-Z����;1:Y-Z����;2.X-Y����(����spatialOperationStyle=15ʱ��Ч)
	pStream->Write(&desEnd.face_offset_norm.x,8,NULL);	//ƽ���淨��
	pStream->Write(&desEnd.face_offset_norm.y,8,NULL);	//ƽ���淨��
	pStream->Write(&desEnd.face_offset_norm.z,8,NULL);	//ƽ���淨��
	pStream->Write(&desEnd.coordCtrlType,1,NULL);	//0:X-Z����;1:Y-Z����;2.X-Y����(����spatialOperationStyle=15ʱ��Ч)
	pStream->Write(&desEnd.cMainCoordCtrlWing,1,NULL);//�Ǹ�����֫��'X'֫�����Ϲؼ���;'Y'֫�����Ϲؼ���;���������Ϲؼ���
	pStream->Write(&desEnd.XOrCtrlWing.gStyle,1,NULL);
	pStream->Write(&desEnd.XOrCtrlWing.operStyle,1,NULL);
	pStream->Write(&desEnd.YOrOtherWing.gStyle,1,NULL);
	pStream->Write(&desEnd.YOrOtherWing.operStyle,1,NULL);
	if(desEnd.cTouchLineType==4)
	{
		pStream->Write(&desEnd.cPlanePtType,1,NULL);
		pStream->Write(&desEnd.cSymmetryType,1,NULL);
		pStream->Write(&desEnd.keyLabelPlane,4,NULL);
	}
}
void CParaAngle::FromStream(IStream *pStream,double version)
{
	CParaRod::FromStream(pStream,version);
	if(version>=2.2)
	{
		pStream->Read(&cPosCtrlDatumLineType,1,NULL);
		pStream->Read(&cPosCtrlType,1,NULL);
	}
	pStream->Read(&desStart.cTouchLine,1,NULL);			//0.X֫����;1.Y֫����;2.����;3.���Ƶ�
	pStream->Read(&desStart.cTouchLineType,1,NULL);		//0.����;1.������;2.ƫ��׼��
	pStream->Read(&desStart.labelHuoquRod,4,NULL);	//0:X-Z����;1:Y-Z����;2.X-Y����(����spatialOperationStyle=15ʱ��Ч)
	pStream->Read(&desStart.face_offset_norm.x,8,NULL);	//ƽ���淨��
	pStream->Read(&desStart.face_offset_norm.y,8,NULL);	//ƽ���淨��
	pStream->Read(&desStart.face_offset_norm.z,8,NULL);	//ƽ���淨��
	pStream->Read(&desStart.coordCtrlType,1,NULL);	//0:X-Z����;1:Y-Z����;2.X-Y����(����spatialOperationStyle=15ʱ��Ч)
	pStream->Read(&desStart.cMainCoordCtrlWing,1,NULL);//�Ǹ�����֫��'X'֫�����Ϲؼ���;'Y'֫�����Ϲؼ���;���������Ϲؼ���
	pStream->Read(&desStart.XOrCtrlWing.gStyle,1,NULL);
	pStream->Read(&desStart.XOrCtrlWing.operStyle,1,NULL);
	pStream->Read(&desStart.YOrOtherWing.gStyle,1,NULL);
	pStream->Read(&desStart.YOrOtherWing.operStyle,1,NULL);
	if(version>=2.1&&desStart.cTouchLineType==4)
	{
		pStream->Read(&desStart.cPlanePtType,1,NULL);
		pStream->Read(&desStart.cSymmetryType,1,NULL);
		pStream->Read(&desStart.keyLabelPlane,4,NULL);
	}

	pStream->Read(&desEnd.cTouchLine,1,NULL);			//0.X֫����;1.Y֫����;2.����;3.���Ƶ�
	pStream->Read(&desEnd.cTouchLineType,1,NULL);		//0.����;1.������;2.ƫ��׼��
	pStream->Read(&desEnd.labelHuoquRod,4,NULL);	//0:X-Z����;1:Y-Z����;2.X-Y����(����spatialOperationStyle=15ʱ��Ч)
	pStream->Read(&desEnd.face_offset_norm.x,8,NULL);	//ƽ���淨��
	pStream->Read(&desEnd.face_offset_norm.y,8,NULL);	//ƽ���淨��
	pStream->Read(&desEnd.face_offset_norm.z,8,NULL);	//ƽ���淨��
	pStream->Read(&desEnd.coordCtrlType,1,NULL);	//0:X-Z����;1:Y-Z����;2.X-Y����(����spatialOperationStyle=15ʱ��Ч)
	pStream->Read(&desEnd.cMainCoordCtrlWing,1,NULL);//�Ǹ�����֫��'X'֫�����Ϲؼ���;'Y'֫�����Ϲؼ���;���������Ϲؼ���
	pStream->Read(&desEnd.XOrCtrlWing.gStyle,1,NULL);
	pStream->Read(&desEnd.XOrCtrlWing.operStyle,1,NULL);
	pStream->Read(&desEnd.YOrOtherWing.gStyle,1,NULL);
	pStream->Read(&desEnd.YOrOtherWing.operStyle,1,NULL);
	if(version>=2.1&&desEnd.cTouchLineType==4)
	{
		pStream->Read(&desEnd.cPlanePtType,1,NULL);
		pStream->Read(&desEnd.cSymmetryType,1,NULL);
		pStream->Read(&desEnd.keyLabelPlane,4,NULL);
	}
}
//////////////////////////////////////////////////////////////////////
// CParaDatumPlane Construction/Destruction
//////////////////////////////////////////////////////////////////////
CParaDatumPlane::CParaDatumPlane()
{
	XFirPos.bStart=FALSE;
	XSecPos.bStart=FALSE;
	XThirPos.bStart=FALSE;
}
CParaDatumPlane::~CParaDatumPlane()
{

}
void CParaDatumPlane::ToStream(IStream *pStream,double version)
{
	BYTE bValue;
	pStream->Write(&version,sizeof(double),NULL);
	WriteStringToStream(pStream,label);
	WriteStringToStream(pStream,description);
	pStream->Write(&XFirPos.keyRefRod,4,NULL);
	bValue=(BYTE)XFirPos.bStart;
	pStream->Write(&bValue,1,NULL);
	pStream->Write(&XSecPos.keyRefRod,4,NULL);
	bValue=(BYTE)XSecPos.bStart;
	pStream->Write(&bValue,1,NULL);
	pStream->Write(&XThirPos.keyRefRod,4,NULL);
	bValue=(BYTE)XThirPos.bStart;
	pStream->Write(&bValue,1,NULL);
}
void CParaDatumPlane::FromStream(IStream *pStream,double version)
{
	BYTE bValue;
	pStream->Read(&version,sizeof(double),NULL);
	ReadStringFromStream(pStream,label);
	ReadStringFromStream(pStream,description);
	pStream->Read(&XFirPos.keyRefRod,4,NULL);
	pStream->Read(&bValue,1,NULL);
	XFirPos.bStart=bValue;
	pStream->Read(&XSecPos.keyRefRod,4,NULL);
	pStream->Read(&bValue,1,NULL);
	XSecPos.bStart=bValue;
	pStream->Read(&XThirPos.keyRefRod,4,NULL);
	pStream->Read(&bValue,1,NULL);
	XThirPos.bStart=bValue;
}
//////////////////////////////////////////////////////////////////////
// DESIGN_PARAM_ITEM Construction/Destruction
//////////////////////////////////////////////////////////////////////
EXPRESSION_VAR ConvertParamItemToExprVar(DESIGN_PARAM_ITEM *pItem)
{
	EXPRESSION_VAR var;
	if(pItem->dataType==DESIGN_PARAM_ITEM::TYPE_DOUBLE)
		var.fValue=pItem->value.fValue;
	else if(pItem->dataType==DESIGN_PARAM_ITEM::TYPE_INTEGER)
		var.fValue=pItem->value.iType;
	else if(pItem->dataType==DESIGN_PARAM_ITEM::TYPE_DOUBLE_ARR)
	{
		var.uiUpperBound=pItem->value.uiUpperBound;
		var.arrValues=pItem->value.arrValues;
	}
	strncpy(var.variableStr,pItem->key.Label(),8);
	return var;
}
//////////////////////////////////////////////////////////////////////
// CParaTemplate Construction/Destruction
//////////////////////////////////////////////////////////////////////
void LoadParaModelNodeInfo(CParaNode *pItem,DWORD key){pItem->keyLabel=key;}
void LoadParaModelRodInfo(CParaRod *pItem,DWORD key){pItem->keyLabel=key;}
CParaRod* CreateNewParaRod(int idClsType,DWORD key,void* pContext)	//����ص�����
{
	CParaRod* pRod;
	switch(idClsType)
	{
	case CLS_LINEANGLE:
		pRod=new CParaAngle();
		break;
	case CLS_LINETUBE:
	default:
		pRod=new CParaRod();
		break;
	}
	pRod->keyLabel=key;
	return pRod;
}
BOOL DeleteParaRod(CParaRod *pRod)				//����ص�����
{
	if(pRod->IsAngleObject())
		delete (CParaAngle*)pRod;
	else
		delete pRod;
	return TRUE;
}
//long (*AllocateUniqueIdFunc)();						//��ѡ���趨�ص�����
CParaTemplate::CParaTemplate()
{
	strcpy(sLabel,"");
	memset(&image,0,sizeof(image));
	LoadDefaultUCS(&rot_ucs);
	listParaNodes.LoadDefaultObjectInfo=LoadParaModelNodeInfo;
	listParaRods.LoadDefaultObjectInfo=LoadParaModelRodInfo;
	listParaRods.CreateNewAtom=CreateNewParaRod;
	listParaRods.DeleteAtom=DeleteParaRod;
}
CParaTemplate::~CParaTemplate()
{
	if(image.bmBits)
		delete []image.bmBits;
}
long CParaTemplate::KeyNodeNum()
{
	int hits=0;
	for(CParaNode* pParaNode=listParaNodes.GetFirst();pParaNode;pParaNode=listParaNodes.GetNext())
	{
		if(pParaNode->m_bSetupJoint)
			hits++;
	}
	return hits;
}
//�ж����ļ��Ƿ�Ϊ��ʷ��ͷ��ģ��
bool CParaTemplate::IsLegacyHeadTemplate(IStream* pStream)
{
	LARGE_INTEGER move;move.QuadPart=0;
	ULARGE_INTEGER current;
	if(pStream->Seek(move,STREAM_SEEK_CUR,&current)!=S_OK)	//��ȡ��ǰ��ָ��λ��
		return false;
	pStream->Seek(move,STREAM_SEEK_SET,NULL);
	double version=1.0;
	pStream->Read(&version,sizeof(double),NULL);
	move.QuadPart=current.QuadPart;
	pStream->Seek(move,STREAM_SEEK_SET,NULL);	//��ԭ��ǰ��ԭָ��λ��
	if(version<2.0)
		return true;
	else
		return false;
}
//��HeadTemplateģ����ϢǨ��ΪParaTemplate��Ϣ
bool CParaTemplate::MigrateFromHeadTemplate(CHeadTemplate* pHeadTempl)
{
	DWORD i;
	strncpy(sLabel,pHeadTempl->sLabel,23);
	sLabel[23]=0;
	rot_ucs=pHeadTempl->rot_ucs;
	image=pHeadTempl->image;
	ULONG image_datalen = image.bmHeight*image.bmWidthBytes;
	if(image_datalen>0)
	{
		image.bmBits = new BYTE[image_datalen];
		memcpy(image.bmBits,pHeadTempl->image.bmBits,image_datalen);
	}
	else
		image.bmBits = NULL;
	CFacePanelNode* pFaceNode=NULL;
	CFacePanelPole* pFacePole=NULL;
	CHeadParameter* pParam   =NULL;
	//Ǩ��ģ�����
	listParams.Empty();
	for(pParam=pHeadTempl->para_list.GetFirst();pParam;pParam=pHeadTempl->para_list.GetNext())
	{
		DESIGN_PARAM_ITEM* pItem;
		if(strlen(pParam->var.variableStr)<=4)
			pItem=listParams.Add(KEY4C(pParam->var.variableStr));
		else
		{
			pItem=listParams.Add(0);
#ifdef AFX_TARG_ENU_ENGLISH
			logerr.Log("parameter %s convert error!",pParam->var.variableStr);
#else
			logerr.Log("����%sת������!",pParam->var.variableStr);
#endif
		}
		pItem->sNotes.Copy(pParam->description);
		pItem->sLabel.Copy(pParam->var.variableStr);
		pItem->dataType=DESIGN_PARAM_ITEM::TYPE_DOUBLE;
		pItem->value.fValue=pParam->var.fValue;
	}
	//Ǩ��ģ��ڵ�
	listParaNodes.Empty();
	DYN_ARRAY<CFacePanelNode*> arrFaceNodes(pHeadTempl->node_list.GetNodeNum());
	DYN_ARRAY<CParaNodePtr> mapParaNodes(pHeadTempl->node_list.GetNodeNum());
	for(pFaceNode=pHeadTempl->node_list.GetFirst(),i=0;pFaceNode;pFaceNode=pHeadTempl->node_list.GetNext(),i++)
	{
		CParaNode* pParaNode;
		if(strlen(pFaceNode->sTitle)<=4)
			pParaNode=listParaNodes.Add(KEY4C(pFaceNode->sTitle));
		else
		{
			pParaNode=listParaNodes.Add(0);
#ifdef AFX_TARG_ENU_ENGLISH
			logerr.Log("node %s convert error!",pFaceNode->sTitle);
#else
			logerr.Log("�ڵ�%sת������!",pFaceNode->sTitle);
#endif
		}
		if(i<(DWORD)pHeadTempl->m_nKeyNode)
			pParaNode->m_bSetupJoint=true;
		else
			pParaNode->m_bSetupJoint=false;
		pParaNode->pos=pFaceNode->pos;
		pParaNode->keyLabel=listParaNodes.GetCursorKey();
		pParaNode->label.Copy(pFaceNode->sTitle);
		//keyLabelFatherRod;
		//pFaceNode->m_bInternodeRepeat�����ڽڼ��棬��CHeadTemplate����ʵ�����ã��ʲ�Ǩ��
		strncpy(pParaNode->posx_expression,pFaceNode->posx_expression,19);
		strncpy(pParaNode->posy_expression,pFaceNode->posy_expression,19);
		strncpy(pParaNode->posz_expression,pFaceNode->posz_expression,19);
		strncpy(pParaNode->scale_expression,pFaceNode->scale_expression,19);
		pParaNode->attach_scale=pFaceNode->attach_scale;
		pParaNode->attach_type=pFaceNode->attach_type;
		arrFaceNodes[i]=pFaceNode;
		mapParaNodes[i]=pParaNode;
	}
	for(i=0;i<arrFaceNodes.Size();i++)
	{
		for(int j=0;j<4;j++)
		{
			if(arrFaceNodes[i]->attach_node_index[j]>=0)
				mapParaNodes[i]->attachNodeArr[j]=mapParaNodes[arrFaceNodes[i]->attach_node_index[j]]->keyLabel;
		}
	}
	//Ǩ��ģ��˼�
	listParaRods.Empty();
	DYN_ARRAY<CFacePanelPole*> arrFaceRods(pHeadTempl->pole_list.GetNodeNum());
	DYN_ARRAY<CParaRod*> mapParaRods(pHeadTempl->pole_list.GetNodeNum());
	for(pFacePole=pHeadTempl->pole_list.GetFirst(),i=0;pFacePole;pFacePole=pHeadTempl->pole_list.GetNext(),i++)
	{
		CParaRod* pParaRod;
		if(strlen(pFacePole->sTitle)<=4)
			pParaRod=listParaRods.Add(KEY4C(pFacePole->sTitle));
		else
			pParaRod=listParaRods.Add(0);
		pParaRod->label.Copy(pFacePole->sTitle);
		pParaRod->cMaterial=pFacePole->cMaterial;
		pParaRod->keyStartNode=mapParaNodes[pFacePole->start_i]->keyLabel;
		pParaRod->keyEndNode=mapParaNodes[pFacePole->end_i]->keyLabel;
		pParaRod->callen_type=pFacePole->callen_type;
		memcpy(pParaRod->sLayer,pFacePole->sLayer,4);
		pParaRod->layout_style=pFacePole->wxLayout;
		//CFacePanelPole::layout_style��ΪwxLayout����ʼ������0��Ϊ1 wjh-2018.8.24
		pParaRod->layout_style-=1;
		pParaRod->start_force_type=(BYTE)pFacePole->start_force_type;
		pParaRod->end_force_type=(BYTE)pFacePole->end_force_type;
		//pParaRod->start_joint_type
		//pParaRod->end_joint_type
		pParaRod->eyeSideNorm=pFacePole->eyeSideNorm;
		arrFaceRods[i]=pFacePole;
		mapParaRods[i]=pParaRod;
	}
	for(i=0;i<arrFaceRods.Size();i++)
	{
		if(strlen(arrFaceRods[i]->sRefPole)>0)
		{
			for(UINT j=0;j<arrFaceRods.Size();j++)
			{
				if(stricmp(arrFaceRods[j]->sTitle,arrFaceRods[i]->sRefPole)==0)
				{
					mapParaRods[i]->keyRefPole=mapParaRods[j]->keyLabel;
					break;
				}
			}
		}
		if(strlen(arrFaceRods[i]->sRefPole2)>0)
		{
			for(UINT j=0;j<arrFaceRods.Size();j++)
			{
				if(stricmp(arrFaceRods[j]->sTitle,arrFaceRods[i]->sRefPole2)==0)
				{
					mapParaRods[i]->keyRefPole2=mapParaRods[j]->keyLabel;
					break;
				}
			}
		}
		if(strlen(arrFaceRods[i]->sLamdaRefPole)>0)
		{
			for(UINT j=0;j<arrFaceRods.Size();j++)
			{
				if(stricmp(arrFaceRods[j]->sTitle,arrFaceRods[i]->sLamdaRefPole)==0)
				{
					mapParaRods[i]->keyLamdaRefPole=mapParaRods[j]->keyLabel;
					break;
				}
			}
		}
	}
	//pHeadTempl->windseg_list;
	return true;
}
static LARGE_INTEGER MKLARGE_INTEGER(long number)
{
	LARGE_INTEGER large;
	large.QuadPart=number;
	return large;
}
static ULARGE_INTEGER MKULARGE_INTEGER(DWORD number)
{
	ULARGE_INTEGER large;
	large.QuadPart=number;
	return large;
}
void CParaTemplate::FromStream(IStream *pStream)
{
	ULARGE_INTEGER oldpos;
	pStream->Seek(MKLARGE_INTEGER(0),STREAM_SEEK_SET,&oldpos);
	if(IsLegacyHeadTemplate(pStream))
	{
		CHeadTemplate headTempl;
		headTempl.FromStream(pStream);
		MigrateFromHeadTemplate(&headTempl);
		return;
	}
	else
	{
		double version=1.0;
		pStream->Read(&version,sizeof(double),NULL);
		//pStream->Read(&m_nKeyNode,sizeof(long),NULL);
		pStream->Read(&rot_ucs.origin.x,sizeof(double),NULL);
		pStream->Read(&rot_ucs.origin.y,sizeof(double),NULL);
		pStream->Read(&rot_ucs.origin.z,sizeof(double),NULL);
		pStream->Read(&rot_ucs.axis_x.x,sizeof(double),NULL);
		pStream->Read(&rot_ucs.axis_x.y,sizeof(double),NULL);
		pStream->Read(&rot_ucs.axis_x.z,sizeof(double),NULL);
		pStream->Read(&rot_ucs.axis_y.x,sizeof(double),NULL);
		pStream->Read(&rot_ucs.axis_y.y,sizeof(double),NULL);
		pStream->Read(&rot_ucs.axis_y.z,sizeof(double),NULL);
		pStream->Read(&rot_ucs.axis_z.x,sizeof(double),NULL);
		pStream->Read(&rot_ucs.axis_z.y,sizeof(double),NULL);
		pStream->Read(&rot_ucs.axis_z.z,sizeof(double),NULL);
		//ʾ��ͼ����
		if(image.bmBits!=NULL)
		{
			delete []image.bmBits;
			image.bmBits=NULL;
		}
		image.bmWidth=image.bmHeight=image.bmWidthBytes=0;
		BYTE bHasImage=0;
		pStream->Read(&bHasImage,1,NULL);
		if(bHasImage==1)
		{
			pStream->Read(&image.bmType,sizeof(LONG),NULL);
			pStream->Read(&image.bmWidth,sizeof(LONG),NULL);
			pStream->Read(&image.bmHeight,sizeof(LONG),NULL);
			pStream->Read(&image.bmWidthBytes,sizeof(LONG),NULL);
			pStream->Read(&image.bmPlanes,sizeof(WORD),NULL);
			pStream->Read(&image.bmBitsPixel,sizeof(WORD),NULL);
			//��ԭλͼ�ļ�ѹ����PNG��ʽ�洢
			ULONG comp_len;
			ULONG pLen = image.bmHeight*image.bmWidthBytes;
			pStream->Read(&comp_len,sizeof(ULONG),NULL);
			BYTE *buffer = new BYTE[comp_len];
			pStream->Read(buffer,comp_len,NULL);
			image.bmBits = new BYTE[pLen];
			if(Z_OK != uncompress((Byte*)image.bmBits,&pLen,buffer,comp_len))
			{
				delete []buffer;
				return;
			}
			delete []buffer;
		}
		//����
		DWORD i,n;
		pStream->Read(&n,sizeof(DWORD),NULL);
		listParams.Empty();
		KEY4C keyLabel;
		for(i=0;i<n;i++)
		{
			pStream->Read(&keyLabel,4,NULL);
			DESIGN_PARAM_ITEM* pPara=listParams.Add(keyLabel);
			ReadStringFromStream(pStream,pPara->sLabel);
			ReadStringFromStream(pStream,pPara->sNotes);
			pStream->Read(&pPara->dataType,sizeof(BYTE),NULL);
			if(pPara->dataType==DESIGN_PARAM_ITEM::TYPE_DOUBLE)
				pStream->Read(&pPara->value.fValue,sizeof(double),NULL);
			else //if(pPara->dataType==DESIGN_PARAM_ITEM::TYPE_INTEGER)
				pStream->Read(&pPara->value.iType,sizeof(long),NULL);
		}
		//�ڵ�
		pStream->Read(&n,sizeof(DWORD),NULL);
		listParaNodes.Empty();
		for(i=0;i<n;i++)
		{
			pStream->Read(&keyLabel,4,NULL);
			CParaNode* pNode=listParaNodes.Add(keyLabel);
			pNode->FromStream(pStream);
		}
		//�˼�
		pStream->Read(&n,sizeof(DWORD),NULL);
		listParaRods.Empty();
		for(i=0;i<n;i++)
		{
			pStream->Read(&keyLabel,4,NULL);
			int idPartClsType=CLS_LINEANGLE;
			pStream->Read(&idPartClsType,sizeof(int),NULL);	//�ø˼��Ĺ������ͣ���Ǹ֡��ֹܡ��ְ��, CLS_LINEANGLE,CLS_LINETUBE
			CParaRod* pPole=listParaRods.Add(keyLabel,idPartClsType);
			pPole->FromStream(pStream,version);
		}
		//��׼��
		pStream->Read(&n,sizeof(DWORD),NULL);
		listParaPlanes.Empty();
		for(i=0;i<n;i++)
		{
			pStream->Read(&keyLabel,4,NULL);
			CParaDatumPlane* pPlane=listParaPlanes.Add(keyLabel);
			pPlane->FromStream(pStream,version);
		}
		//��ѹ��
		pStream->Read(&n,sizeof(DWORD),NULL);
		windseg_list.Empty();
		for(i=0;i<n;i++)
		{
			CFaceWindSegment *pWindSeg=windseg_list.append();
			pWindSeg->FromStream(pStream);
		}
	}
}
void CParaTemplate::ToStream(IStream *pStream)
{
	double version=2.0;
	ULARGE_INTEGER newSize;
	newSize.QuadPart=10;
	pStream->SetSize(newSize);
	pStream->Write(&version,sizeof(double),NULL);
	//pStream->Write(&m_nKeyNode,sizeof(long),NULL);
	pStream->Write(&rot_ucs.origin.x,sizeof(double),NULL);
	pStream->Write(&rot_ucs.origin.y,sizeof(double),NULL);
	pStream->Write(&rot_ucs.origin.z,sizeof(double),NULL);
	pStream->Write(&rot_ucs.axis_x.x,sizeof(double),NULL);
	pStream->Write(&rot_ucs.axis_x.y,sizeof(double),NULL);
	pStream->Write(&rot_ucs.axis_x.z,sizeof(double),NULL);
	pStream->Write(&rot_ucs.axis_y.x,sizeof(double),NULL);
	pStream->Write(&rot_ucs.axis_y.y,sizeof(double),NULL);
	pStream->Write(&rot_ucs.axis_y.z,sizeof(double),NULL);
	pStream->Write(&rot_ucs.axis_z.x,sizeof(double),NULL);
	pStream->Write(&rot_ucs.axis_z.y,sizeof(double),NULL);
	pStream->Write(&rot_ucs.axis_z.z,sizeof(double),NULL);
	//ʾ��ͼ����
	BYTE bHasImage=0;
	if(image.bmBits==NULL)
		pStream->Write(&bHasImage,1,NULL);
	else
	{
		bHasImage=1;
		pStream->Write(&bHasImage,1,NULL);
		pStream->Write(&image.bmType,sizeof(LONG),NULL);
		pStream->Write(&image.bmWidth,sizeof(LONG),NULL);
		pStream->Write(&image.bmHeight,sizeof(LONG),NULL);
		pStream->Write(&image.bmWidthBytes,sizeof(LONG),NULL);
		pStream->Write(&image.bmPlanes,sizeof(WORD),NULL);
		pStream->Write(&image.bmBitsPixel,sizeof(WORD),NULL);

		//��ԭλͼ�ļ�ѹ����PNG��ʽ�洢
		ULONG comp_len=0;
		BYTE *buffer=NULL;
		comp_len = image.bmWidthBytes*image.bmHeight;
		uLong pLen = compressBound(comp_len);
		buffer = new BYTE[pLen];
		if(Z_OK !=compress(buffer,&pLen,(Byte*)image.bmBits,comp_len))
			return;
		pStream->Write(&pLen,sizeof(ULONG),NULL);
		pStream->Write(buffer,pLen,NULL);
		delete [] buffer;
	}
	//����
	DWORD n=listParams.GetNodeNum();
	pStream->Write(&n,sizeof(DWORD),NULL);
	for(DESIGN_PARAM_ITEM* pPara=listParams.GetFirst();pPara;pPara=listParams.GetNext())
	{
		pStream->Write(&pPara->key,4,NULL);
		WriteStringToStream(pStream,pPara->sLabel);
		WriteStringToStream(pStream,pPara->sNotes);
		pStream->Write(&pPara->dataType,sizeof(BYTE),NULL);
		if(pPara->dataType==DESIGN_PARAM_ITEM::TYPE_DOUBLE)
			pStream->Write(&pPara->value.fValue,sizeof(double),NULL);
		else //if(pPara->dataType==DESIGN_PARAM_ITEM::TYPE_INTEGER)
			pStream->Write(&pPara->value.iType,sizeof(long),NULL);
	}
	//�ڵ�
	n=listParaNodes.GetNodeNum();
	pStream->Write(&n,sizeof(DWORD),NULL);
	for(CParaNode* pNode=listParaNodes.GetFirst();pNode;pNode=listParaNodes.GetNext())
	{
		pStream->Write(&pNode->keyLabel,4,NULL);
		pNode->ToStream(pStream);
	}
	//�˼�
	n=listParaRods.GetNodeNum();
	pStream->Write(&n,sizeof(DWORD),NULL);
	for(CParaRod* pPole=listParaRods.GetFirst();pPole;pPole=listParaRods.GetNext())
	{	
		pStream->Write(&pPole->keyLabel,4,NULL);
		pStream->Write(&pPole->m_idPartClsType,sizeof(int),NULL);
		pPole->ToStream(pStream);
	}
	//��ѹ��
	n=this->windseg_list.GetNodeNum();
	pStream->Write(&n,sizeof(DWORD),NULL);
	for(CFaceWindSegment* pWindSeg=windseg_list.GetFirst();pWindSeg;pWindSeg=windseg_list.GetNext())
		pWindSeg->ToStream(pStream);
}
void CParaTemplate::UpdateNodePos()
{
	CExpression expression;
	for(DESIGN_PARAM_ITEM* pPara=listParams.GetFirst();pPara;pPara=listParams.GetNext())
		expression.varList.Append(ConvertParamItemToExprVar(pPara));
	for(CParaNode* pNode=listParaNodes.GetFirst();pNode;pNode=listParaNodes.GetNext())
	{
		int nPush=listParaNodes.push_stack();
		try{
			if(pNode->attach_type==CParaNode::INDEPENDENT)	//���κ������ڵ�
			{
				if(strlen(pNode->posx_expression)>0)
					pNode->pos.x=expression.SolveExpression(pNode->posx_expression);
				if(strlen(pNode->posy_expression)>0)
					pNode->pos.y=expression.SolveExpression(pNode->posy_expression);
				if(strlen(pNode->posz_expression)>0)
					pNode->pos.z=expression.SolveExpression(pNode->posz_expression);
			}
			else if(pNode->attach_type==CParaNode::COORD_X_SPECIFIED)	//�˼���X����ֵ�����
			{	
				CParaNode* pStartNode=listParaNodes.GetValue(pNode->attachNodeArr[0]);
				CParaNode* pEndNode=listParaNodes.GetValue(pNode->attachNodeArr[1]);
				f3dPoint startPt=pStartNode->pos;
				f3dPoint endPt=pEndNode->pos;
				f3dPoint pos;
				pos.x=expression.SolveExpression(pNode->posx_expression);
				pos.y=startPt.y+(pos.x-startPt.x)*(endPt.y-startPt.y)/(endPt.x-startPt.x);
				pos.z=startPt.z+(pos.x-startPt.x)*(endPt.z-startPt.z)/(endPt.x-startPt.x);
				pNode->pos=pos;
			}
			else if(pNode->attach_type==CParaNode::COORD_Y_SPECIFIED)	//�˼���Y����ֵ�����
			{
				CParaNode* pStartNode=listParaNodes.GetValue(pNode->attachNodeArr[0]);
				CParaNode* pEndNode=listParaNodes.GetValue(pNode->attachNodeArr[1]);
				f3dPoint startPt=pStartNode->pos;
				f3dPoint endPt=pEndNode->pos;
				f3dPoint pos;
				pos.y=expression.SolveExpression(pNode->posy_expression);
				pos.x=startPt.x+(pos.y-startPt.y)*(endPt.x-startPt.x)/(endPt.y-startPt.y);
				pos.z=startPt.z+(pos.y-startPt.y)*(endPt.z-startPt.z)/(endPt.y-startPt.y);
				pNode->pos=pos;
			}
			else if(pNode->attach_type==CParaNode::COORD_Z_SPECIFIED)	//�˼���Z����ֵ�����
			{
				CParaNode* pStartNode=listParaNodes.GetValue(pNode->attachNodeArr[0]);
				CParaNode* pEndNode=listParaNodes.GetValue(pNode->attachNodeArr[1]);
				f3dPoint startPt=pStartNode->pos;
				f3dPoint endPt=pEndNode->pos;
				f3dPoint pos;
				pos.z=expression.SolveExpression(pNode->posz_expression);
				pos.x=startPt.x+(pos.z-startPt.z)*(endPt.x-startPt.x)/(endPt.z-startPt.z);
				pos.y=startPt.y+(pos.z-startPt.z)*(endPt.y-startPt.y)/(endPt.z-startPt.z);
				pNode->pos=pos;
			}
			else if(pNode->attach_type==CParaNode::INTERSECTION)		//���˼����潻��
			{	
				CParaNode* pStartNode1=listParaNodes.GetValue(pNode->attachNodeArr[0]);
				CParaNode* pEndNode1=listParaNodes.GetValue(pNode->attachNodeArr[1]);
				CParaNode* pStartNode2=listParaNodes.GetValue(pNode->attachNodeArr[2]);
				CParaNode* pEndNode2=listParaNodes.GetValue(pNode->attachNodeArr[3]);
				f3dPoint start1=pStartNode1->pos;
				f3dPoint end1=pEndNode1->pos;
				f3dPoint start2=pStartNode2->pos;
				f3dPoint end2=pEndNode2->pos;
				f3dPoint pos;
				char cPrjAxis=0;
				if(pNode->m_cPosCalViceType==1)			//������X-Zƽ��
					cPrjAxis='Y';
				else if(pNode->m_cPosCalViceType==2)	//������Y-Zƽ��
					cPrjAxis='X';
				else if(pNode->m_cPosCalViceType==3)	//������X-Yƽ��
					cPrjAxis='Z';
				Int3dDifferPlaneLL(start1,end1,start2,end2,pos,cPrjAxis);
				pNode->pos=pos;
			}
			else if(pNode->attach_type==CParaNode::SCALE)				////�˼��ϵı����ȷֵ�
			{	
				CParaNode* pStartNode=listParaNodes.GetValue(pNode->attachNodeArr[0]);
				CParaNode* pEndNode=listParaNodes.GetValue(pNode->attachNodeArr[1]);
				f3dPoint startPt=pStartNode->pos;
				f3dPoint endPt  =pEndNode->pos;
				double scale=expression.SolveExpression(pNode->scale_expression);
				if(scale<0)
				{
					scale*=-1;
					scale=0.1*((int)scale)/(scale-(int)scale);
				}
				pNode->pos=startPt+(endPt-startPt)*scale;
			}
			else if(pNode->attach_type==CParaNode::PLANE_XY_SPECIFIED)
			{	//��׼����ָ��XY������
				f3dPoint start=listParaNodes.GetValue(pNode->attachNodeArr[0])->pos;
				f3dPoint end  =listParaNodes.GetValue(pNode->attachNodeArr[1])->pos;
				f3dPoint face_pick =listParaNodes.GetValue(pNode->attachNodeArr[2])->pos;
				f3dPoint line_vec1=end-start;
				f3dPoint line_vec2=end-face_pick;
				f3dPoint face_norm=line_vec1^line_vec2;
				if(strlen(pNode->posx_expression)>0)
					pNode->pos.x=expression.SolveExpression(pNode->posx_expression);
				if(strlen(pNode->posy_expression)>0)
					pNode->pos.y=expression.SolveExpression(pNode->posy_expression);
				Int3dlf(pNode->pos,f3dPoint(pNode->pos.x,pNode->pos.y,0),f3dPoint(0,0,1),face_pick,face_norm);
			}
			else if(pNode->attach_type==CParaNode::PLANE_YZ_SPECIFIED)
			{	//��׼����ָ��YZ������
				f3dPoint start=listParaNodes.GetValue(pNode->attachNodeArr[0])->pos;
				f3dPoint end  =listParaNodes.GetValue(pNode->attachNodeArr[1])->pos;
				f3dPoint face_pick =listParaNodes.GetValue(pNode->attachNodeArr[2])->pos;
				f3dPoint line_vec1=end-start;
				f3dPoint line_vec2=end-face_pick;
				f3dPoint face_norm=line_vec1^line_vec2;
				if(strlen(pNode->posy_expression)>0)
					pNode->pos.y=expression.SolveExpression(pNode->posy_expression);
				if(strlen(pNode->posz_expression)>0)
					pNode->pos.z=expression.SolveExpression(pNode->posz_expression);
				Int3dlf(pNode->pos,f3dPoint(0,pNode->pos.y,pNode->pos.z),f3dPoint(1,0,0),face_pick,face_norm);
			}
			else if(pNode->attach_type==CParaNode::PLANE_XZ_SPECIFIED)
			{	//��׼����ָ��XZ������
				f3dPoint start=listParaNodes.GetValue(pNode->attachNodeArr[0])->pos;
				f3dPoint end  =listParaNodes.GetValue(pNode->attachNodeArr[1])->pos;
				f3dPoint face_pick =listParaNodes.GetValue(pNode->attachNodeArr[2])->pos;
				f3dPoint line_vec1=end-start;
				f3dPoint line_vec2=end-face_pick;
				f3dPoint face_norm=line_vec1^line_vec2;
				if(strlen(pNode->posx_expression)>0)
					pNode->pos.x=expression.SolveExpression(pNode->posx_expression);
				if(strlen(pNode->posz_expression)>0)
					pNode->pos.z=expression.SolveExpression(pNode->posz_expression);
				Int3dlf(pNode->pos,f3dPoint(pNode->pos.x,0,pNode->pos.z),f3dPoint(0,1,0),face_pick,face_norm);
			}
			EXPRESSION_VAR *pVar;
			pVar=expression.varList.Append();
			sprintf(pVar->variableStr,"%s_X",pNode->keyLabel.Label());
			pVar->fValue=pNode->pos.x;
			pVar=expression.varList.Append();
			sprintf(pVar->variableStr,"%s_Y",pNode->keyLabel.Label());
			pVar->fValue=pNode->pos.y;
			pVar=expression.varList.Append();
			sprintf(pVar->variableStr,"%s_Z",pNode->keyLabel.Label());
			pVar->fValue=pNode->pos.z;
		}
		catch(char* sError)
		{
			AfxMessageBox(sError);
		}
		listParaNodes.pop_stack(nPush);
	}
}

bool CParaTemplate::AssertParamsValid()
{
	//���ø�ģ����Ӧ֧�ֵĲ��������Ϸ���У��
	return true;//NativeAssertParamsValid(m_iTempType);
}
