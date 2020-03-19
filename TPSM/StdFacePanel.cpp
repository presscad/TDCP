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

/////////////////////////////////////////////////////////////////////////////
// CFaceWindSegment
CFaceWindSegment::CFaceWindSegment()
{
	cType=0;					//标准段号
	cMirType=0;				//对称信息0:无对称;1:Y轴对称;2:X轴对称
	cCalStyle=0;				//计算方式0:按塔身方式计算风压;1:按X方向水平横担方式计算风压;2:按Y方向水平横担方式计算风压
	CqiFront=CqiSide=1.2;	//正/侧面增大系数
	BetaZ=1.0;				//风振系数或称风荷载调整系数（βz）
	memset(nodeIndexArr,-1,sizeof(nodeIndexArr));		//八个轮廓节点的索引
}
void CFaceWindSegment::ToStream(IStream *pStream)
{
	double version = 1.0;
	pStream->Write(&version,sizeof(double),NULL);
	pStream->Write(&cType,1,NULL);
	pStream->Write(&cMirType,1,NULL);
	pStream->Write(&cCalStyle,1,NULL);
	pStream->Write(&CqiFront,sizeof(double),NULL);
	pStream->Write(&CqiSide,sizeof(double),NULL);
	pStream->Write(&BetaZ,sizeof(double),NULL);
	long n=GetMaxNodeCount();
	pStream->Write(&n,sizeof(long),NULL);
	pStream->Write(nodeIndexArr,sizeof(long)*n,NULL);
}
void CFaceWindSegment::FromStream(IStream *pStream)
{
	double version = 1.0;
	pStream->Read(&version,sizeof(double),NULL);
	pStream->Read(&cType,1,NULL);
	pStream->Read(&cMirType,1,NULL);
	pStream->Read(&cCalStyle,1,NULL);
	pStream->Read(&CqiFront,sizeof(double),NULL);
	pStream->Read(&CqiSide,sizeof(double),NULL);
	pStream->Read(&BetaZ,sizeof(double),NULL);
	long n=0;
	pStream->Read(&n,sizeof(long),NULL);
	pStream->Read(nodeIndexArr,sizeof(long)*n,NULL);
}
/////////////////////////////////////////////////////////////////////////////
// CHeadParameter
CHeadParameter::CHeadParameter()
{
	memset(this,0,sizeof(CHeadParameter));
}

void CHeadParameter::FromStream(IStream *pStream)
{
	double version=0;
	pStream->Read(&version,sizeof(double),NULL);
	ReadStringFromStream(pStream,var.variableStr);
	pStream->Read(&var.fValue,sizeof(double),NULL);
	ReadStringFromStream(pStream,description);
	if(version<1.1)
	{
		bool m_bStartDimPoint,m_bEndDimPoint;
		f3dPoint m_dimStart,m_dimEnd;
		char startposx_expression[20];
		char startposy_expression[20];
		char startposz_expression[20];
		char endposx_expression[20];
		char endposy_expression[20];
		char endposz_expression[20];
		pStream->Read(&m_dimStart.x,sizeof(double),NULL);
		pStream->Read(&m_dimStart.y,sizeof(double),NULL);
		pStream->Read(&m_dimStart.z,sizeof(double),NULL);
		pStream->Read(&m_dimEnd.x,sizeof(double),NULL);
		pStream->Read(&m_dimEnd.y,sizeof(double),NULL);
		pStream->Read(&m_dimEnd.z,sizeof(double),NULL);
		pStream->Read(&m_bStartDimPoint,sizeof(bool),NULL);
		pStream->Read(&m_bEndDimPoint,sizeof(bool),NULL);
		ReadStringFromStream(pStream,startposx_expression);
		ReadStringFromStream(pStream,startposy_expression);
		ReadStringFromStream(pStream,startposz_expression);
		ReadStringFromStream(pStream,endposx_expression);
		ReadStringFromStream(pStream,endposy_expression);
		ReadStringFromStream(pStream,endposz_expression);
	}
}

void CHeadParameter::ToStream(IStream *pStream)
{
	double version=1.1;
	pStream->Write(&version,sizeof(double),NULL);
	WriteStringToStream(pStream,var.variableStr);
	pStream->Write(&var.fValue,sizeof(double),NULL);
	WriteStringToStream(pStream,description);
	/*pStream->Write(&m_dimStart.x,sizeof(double),NULL);
	pStream->Write(&m_dimStart.y,sizeof(double),NULL);
	pStream->Write(&m_dimStart.z,sizeof(double),NULL);
	pStream->Write(&m_dimEnd.x,sizeof(double),NULL);
	pStream->Write(&m_dimEnd.y,sizeof(double),NULL);
	pStream->Write(&m_dimEnd.z,sizeof(double),NULL);
	pStream->Write(&m_bStartDimPoint,sizeof(bool),NULL);
	pStream->Write(&m_bEndDimPoint,sizeof(bool),NULL);
	WriteStringToStream(pStream,startposx_expression);
	WriteStringToStream(pStream,startposy_expression);
	WriteStringToStream(pStream,startposz_expression);
	WriteStringToStream(pStream,endposx_expression);
	WriteStringToStream(pStream,endposy_expression);
	WriteStringToStream(pStream,endposz_expression);*/
}
/////////////////////////////////////////////////////////////////////////////
// CFacePanelNode
CFacePanelNode::CFacePanelNode()
{
	memset(this,0,sizeof(CFacePanelNode));
	attach_node_index[0]=-1;
	attach_node_index[1]=-1;
	attach_node_index[2]=-1;
	attach_node_index[3]=-1;
	strcpy(posx_expression,"");
	strcpy(posy_expression,"");
	strcpy(posz_expression,"");
	strcpy(scale_expression,"");
}

CFacePanel::CFacePanel()
{
	m_iFaceType=0;
	m_nKeyNode=2;
	strcpy(sLabel,"");
	m_bInternodeRepeat=false;
	m_bWing2Upward=false;
	m_nInternodeNum=3;
	m_nRepeatLessNum=1;
}

CFacePanel::~CFacePanel()
{

}
bool CFacePanel::IsRepeateType()
{
	if( m_iFaceType==3||	//多节间横担面
		m_iFaceType==5||	//V型标准面
		m_iFaceType==6)	//接腿正侧面
		return true;
	else
		return false;
}
void CFacePanel::ToStream(IStream *pStream)
{
	double version=1.2;
	pStream->Write(&version,sizeof(double),NULL);
	pStream->Write(&m_nKeyNode,sizeof(long),NULL);
	pStream->Write(&m_bInternodeRepeat,sizeof(bool),NULL);
	pStream->Write(&m_bWing2Upward,sizeof(bool),NULL);
	pStream->Write(&m_nInternodeNum,sizeof(long),NULL);
	pStream->Write(&m_nRepeatLessNum,sizeof(long),NULL);
	long n=node_list.GetNodeNum();
	pStream->Write(&n,sizeof(long),NULL);
	for(CFacePanelNode *pNode=node_list.GetFirst();pNode;pNode=node_list.GetNext())
		pNode->ToStream(pStream);
	n=pole_list.GetNodeNum();
	pStream->Write(&n,sizeof(long),NULL);
	for(CFacePanelPole *pPole=pole_list.GetFirst();pPole;pPole=pole_list.GetNext())
		pPole->ToStream(pStream);
}
void CFacePanel::FromStream(IStream *pStream)
{
	long i,n;
	double version=0;
	pStream->Read(&version,sizeof(double),NULL);
	pStream->Read(&m_nKeyNode,sizeof(long),NULL);
	if(version>=1.1)
	{
		pStream->Read(&m_bInternodeRepeat,sizeof(bool),NULL);
		if(version>=1.2)
			pStream->Read(&m_bWing2Upward,sizeof(bool),NULL);
		pStream->Read(&m_nInternodeNum,sizeof(long),NULL);
		pStream->Read(&m_nRepeatLessNum,sizeof(long),NULL);
	}
	pStream->Read(&n,sizeof(long),NULL);
	node_list.Empty();
	for(i=0;i<n;i++)
	{
		CFacePanelNode *pNode=node_list.append();
		pNode->FromStream(pStream);
	}
	pStream->Read(&n,sizeof(long),NULL);
	pole_list.Empty();
	for(i=0;i<n;i++)
	{
		CFacePanelPole *pPole=pole_list.append();
		pPole->FromStream(pStream);
	}
}

void CFacePanel::UpdateNodePos()
{
	for(CFacePanelNode *pNode=node_list.GetFirst();pNode;pNode=node_list.GetNext())
	{
		node_list.push_stack();
		if(pNode->attach_type==0)	//无任何依赖节点
			continue;
		else if(pNode->attach_type==1)
		{	//杆件上X坐标值不变点
			CFacePanelNode *pStartNode=&node_list[pNode->attach_node_index[0]];
			CFacePanelNode *pEndNode  =&node_list[pNode->attach_node_index[1]];
			if(pNode->attach_node_index[2]>=0&&pNode->attach_node_index[2]<node_list.GetNodeNum())
				pNode->pos.x=node_list[pNode->attach_node_index[2]].pos.x;
			pNode->pos.y=pStartNode->pos.y+
				(pNode->pos.x-pStartNode->pos.x)*(pEndNode->pos.y-pStartNode->pos.y)/(pEndNode->pos.x-pStartNode->pos.x);
		}
		else if(pNode->attach_type==2)
		{	//杆件上Y坐标值不变点
			CFacePanelNode *pStartNode=&node_list[pNode->attach_node_index[0]];
			CFacePanelNode *pEndNode  =&node_list[pNode->attach_node_index[1]];
			if(pNode->attach_node_index[2]>=0&&pNode->attach_node_index[2]<node_list.GetNodeNum())
				pNode->pos.y=node_list[pNode->attach_node_index[2]].pos.y;
			pNode->pos.x=pStartNode->pos.x+
				(pNode->pos.y-pStartNode->pos.y)*(pEndNode->pos.x-pStartNode->pos.x)/(pEndNode->pos.y-pStartNode->pos.y);
		}
		else if(pNode->attach_type==3)
		{	//杆件上Z坐标值不变点(暂按Y值不变计算)
			CFacePanelNode *pStartNode=&node_list[pNode->attach_node_index[0]];
			CFacePanelNode *pEndNode  =&node_list[pNode->attach_node_index[1]];
			if(pNode->attach_node_index[2]>=0&&pNode->attach_node_index[2]<node_list.GetNodeNum())
				pNode->pos.y=node_list[pNode->attach_node_index[2]].pos.y;
			pNode->pos.x=pStartNode->pos.x+
				(pNode->pos.y-pStartNode->pos.y)*(pEndNode->pos.x-pStartNode->pos.x)/(pEndNode->pos.y-pStartNode->pos.y);
		}
		else if(pNode->attach_type==4)
		{	//两杆件交叉交点
			f2dPoint start1=node_list[pNode->attach_node_index[0]].pos;
			f2dPoint end1=node_list[pNode->attach_node_index[1]].pos;
			f2dPoint start2  =node_list[pNode->attach_node_index[2]].pos;
			f2dPoint end2  =node_list[pNode->attach_node_index[3]].pos;
			Int2dll(f2dLine(start1,end1),f2dLine(start2,end2),pNode->pos.x,pNode->pos.y);
		}
		else if(pNode->attach_type==5)
		{	//杆件上的比例等分点
			f2dPoint start=node_list[pNode->attach_node_index[0]].pos;
			f2dPoint end  =node_list[pNode->attach_node_index[1]].pos;
			double real_scale=pNode->attach_scale;
			if(pNode->attach_scale<0)
			{
				real_scale=-pNode->attach_scale;
				real_scale=0.1*((int)real_scale)/(real_scale-(int)real_scale);
			}
			pNode->pos.x=start.x+real_scale*(end.x-start.x);
			pNode->pos.y=start.y+real_scale*(end.y-start.y);
		}
		node_list.pop_stack();
	}
}
bool CFacePanel::ReadFromLibFile(char* lib_file,char *label)
{
	if(strlen(label)==0)
		return false;
	IStorage *pRootStg;
	DWORD grfMode = STGM_READ | STGM_SHARE_EXCLUSIVE;
	HRESULT hr = ::StgOpenStorage(_bstr_t(lib_file), NULL,grfMode,NULL, 0, &pRootStg);
	if(pRootStg==NULL)
		return false;
	strncpy(sLabel,label,23);
	IStream *pStream=NULL;
	IStorage *pStg=NULL;
	CString Part;
	if(m_iFaceType==1)		//1表示塔身
	{
#ifdef AFX_TARG_ENU_ENGLISH
		Part="tower body standard plane";
#else
		Part="塔身标准面";
#endif
		pRootStg->OpenStorage(_bstr_t(Part),NULL,grfMode,NULL,0,&pStg);
		if(pStg!=NULL)
			pStg->OpenStream(_bstr_t(sLabel),NULL,grfMode,0,&pStream);
	}
	if((m_iFaceType==1&&pStream==NULL)||m_iFaceType==4)	//4:K型标准面
	{
#ifdef AFX_TARG_ENU_ENGLISH
		Part="k-shape standard plane";
#else
		Part="K型标准面";
#endif
		if(pStg)
		{
			pStg->Release();
			pStg=NULL;
		}
		pRootStg->OpenStorage(_bstr_t(Part),NULL,grfMode,NULL,0,&pStg);
		if(pStg!=NULL)
		{
			m_iFaceType=4;
			pStg->OpenStream(_bstr_t(sLabel),NULL,grfMode,0,&pStream);
		}
	}
	else if(m_iFaceType==2)	//2:表示横隔面;
	{
#ifdef AFX_TARG_ENU_ENGLISH
		Part="cross separate standard plane";
#else
		Part="横隔标准面";
#endif
		pRootStg->OpenStorage(_bstr_t(Part),NULL,grfMode,NULL,0,&pStg);
		if(pStg!=NULL)
			pStg->OpenStream(_bstr_t(sLabel),NULL,grfMode,0,&pStream);
	}
	else if(m_iFaceType==3)	//3:表示多节间面;
	{
#ifdef AFX_TARG_ENU_ENGLISH
		Part="many internode plane";
#else
		Part="多节间面";
#endif
		pRootStg->OpenStorage(_bstr_t(Part),NULL,grfMode,NULL,0,&pStg);
		if(pStg!=NULL)
			pStg->OpenStream(_bstr_t(sLabel),NULL,grfMode,0,&pStream);
	}
	else if(m_iFaceType==5)	//5:V型标准面
	{
#ifdef AFX_TARG_ENU_ENGLISH
		Part="V-shape standard plane";
#else
		Part="V型标准面";
#endif
		pRootStg->OpenStorage(_bstr_t(Part),NULL,grfMode,NULL,0,&pStg);
		if(pStg!=NULL)
			pStg->OpenStream(_bstr_t(sLabel),NULL,grfMode,0,&pStream);
	}
	else if(m_iFaceType==6)	//6:接腿正侧面
	{
#ifdef AFX_TARG_ENU_ENGLISH
		Part="Leg Front Plane";
#else
		Part="接腿正侧面";
#endif
		pRootStg->OpenStorage(_bstr_t(Part),NULL,grfMode,NULL,0,&pStg);
		if(pStg!=NULL)
			pStg->OpenStream(_bstr_t(sLabel),NULL,grfMode,0,&pStream);
	}
	if(pStream)
	{
		FromStream(pStream);
		pStream->Release();
	}
	if(pStg)
		pStg->Release();
	pRootStg->Release();
	if(pStream!=NULL)
		return true;
	else
		return false;
}

void CFacePanelNode::FromStream(IStream *pStream)
{
	double version=0;
	pStream->Read(&version,sizeof(double),NULL);
	pStream->Read(sTitle,16,NULL);
	pStream->Read(&pos.x,sizeof(double),NULL);
	pStream->Read(&pos.y,sizeof(double),NULL);
	if(version>=1.1)
		pStream->Read(&pos.z,sizeof(double),NULL);
	if(version>=1.3)
		pStream->Read(&m_bInternodeRepeat,sizeof(bool),NULL);
	//读取表达式
	if(version>=1.2)
	{
		ReadStringFromStream(pStream,posx_expression);
		ReadStringFromStream(pStream,posy_expression);
		ReadStringFromStream(pStream,posz_expression);
		ReadStringFromStream(pStream,scale_expression);
	}
	pStream->Read(&attach_type,sizeof(short),NULL);
	pStream->Read(&attach_scale,sizeof(double),NULL);
	pStream->Read(&attach_node_index[0],sizeof(short),NULL);
	pStream->Read(&attach_node_index[1],sizeof(short),NULL);
	pStream->Read(&attach_node_index[2],sizeof(short),NULL);
	pStream->Read(&attach_node_index[3],sizeof(short),NULL);
}

void CFacePanelNode::ToStream(IStream *pStream)
{
	//版本号加了0.1
	double version=1.3;
	pStream->Write(&version,sizeof(double),NULL);
	pStream->Write(sTitle,16,NULL);
	pStream->Write(&pos.x,sizeof(double),NULL);
	pStream->Write(&pos.y,sizeof(double),NULL);
	pStream->Write(&pos.z,sizeof(double),NULL);
	pStream->Write(&m_bInternodeRepeat,sizeof(bool),NULL);
	//写表达式
	WriteStringToStream(pStream,posx_expression);
	WriteStringToStream(pStream,posy_expression);
	WriteStringToStream(pStream,posz_expression);
	WriteStringToStream(pStream,scale_expression);
	pStream->Write(&attach_type,sizeof(short),NULL);
	pStream->Write(&attach_scale,sizeof(double),NULL);
	pStream->Write(&attach_node_index[0],sizeof(short),NULL);
	pStream->Write(&attach_node_index[1],sizeof(short),NULL);
	pStream->Write(&attach_node_index[2],sizeof(short),NULL);
	pStream->Write(&attach_node_index[3],sizeof(short),NULL);
}

void CFacePanelPole::FromStream(IStream *pStream)
{
	double version=0;
	pStream->Read(&version,sizeof(double),NULL);
	pStream->Read(sTitle,16,NULL);
	pStream->Read(sLayer,4,NULL);
	pStream->Read(&cMaterial,1,NULL);
	pStream->Read(&start_i,sizeof(short),NULL);
	pStream->Read(&end_i,sizeof(short),NULL);
	if(version>=1.4)
	{
		pStream->Read(&start_offset,sizeof(short),NULL);
		if(start_offset!=0)
			pStream->Read(sStartOffsetRod,16,NULL);
		pStream->Read(&end_offset,sizeof(short),NULL);
		if(end_offset!=0)
			pStream->Read(sEndOffsetRod,16,NULL);
	}
	else
		start_offset=end_offset=0;
	if(version>=1.2)
		pStream->Read(&m_bInternodeRepeat,sizeof(bool),NULL);
	pStream->Read(&start_force_type,sizeof(short),NULL);
	pStream->Read(&end_force_type,sizeof(short),NULL);
	pStream->Read(&callen_type,sizeof(short),NULL);
	if(version<=1.2)	//从V1.3开始长细比计算类型进行了简化
	{
		if(callen_type>0&&callen_type<4)
			callen_type=0;
		else if(callen_type>=4)
			callen_type-=3;
	}
	pStream->Read(sRefPole,16,NULL);
	if(version>=1.1)
		pStream->Read(sRefPole2,16,NULL);
	pStream->Read(sLamdaRefPole,16,NULL);
	WORD siLayoutStyleIndex;//角钢的布置方式(以0为起始索引值)，取值0,1,2,3
	pStream->Read(&siLayoutStyleIndex,sizeof(short),NULL);
	siLayoutStyleIndex+=1;	//实际模型中以1为起始值，故需统一加1 wjh-2018.8.24
	wxLayout=ANGLE_LAYOUT(siLayoutStyleIndex);
	if(version>=1.1)
	{
		pStream->Read(&eyeSideNorm.x,sizeof(double),NULL);
		pStream->Read(&eyeSideNorm.y,sizeof(double),NULL);
		pStream->Read(&eyeSideNorm.z,sizeof(double),NULL);
	}
}

void CFacePanelPole::ToStream(IStream *pStream)
{
	double version=1.4;
	pStream->Write(&version,sizeof(double),NULL);
	pStream->Write(sTitle,16,NULL);
	pStream->Write(sLayer,4,NULL);
	pStream->Write(&cMaterial,1,NULL);
	pStream->Write(&start_i,sizeof(short),NULL);
	pStream->Write(&end_i,sizeof(short),NULL);
	pStream->Write(&start_offset,sizeof(short),NULL);
	if(start_offset!=0)
		pStream->Write(sStartOffsetRod,16,NULL);
	pStream->Write(&end_offset,sizeof(short),NULL);
	if(end_offset!=0)
		pStream->Write(sEndOffsetRod,16,NULL);
	pStream->Write(&m_bInternodeRepeat,sizeof(bool),NULL);
	pStream->Write(&start_force_type,sizeof(short),NULL);
	pStream->Write(&end_force_type,sizeof(short),NULL);
	pStream->Write(&callen_type,sizeof(short),NULL);
	pStream->Write(sRefPole,16,NULL);
	pStream->Write(sRefPole2,16,NULL);
	pStream->Write(sLamdaRefPole,16,NULL);
	short siLayoutStyleIndex=wxLayout%2==0?wxLayout:(WORD)wxLayout-1;
	pStream->Write(&siLayoutStyleIndex,sizeof(short),NULL);
	pStream->Write(&eyeSideNorm.x,sizeof(double),NULL);
	pStream->Write(&eyeSideNorm.y,sizeof(double),NULL);
	pStream->Write(&eyeSideNorm.z,sizeof(double),NULL);
}
/////////////////////////////////////////////////////////////////////////////
// CHeadTemplate
CHeadTemplate::CHeadTemplate()
{
	m_nKeyNode=2;
	strcpy(sLabel,"");
	memset(&image,0,sizeof(image));
	LoadDefaultUCS(&rot_ucs);
}

CHeadTemplate::~CHeadTemplate()
{
	if(image.bmBits!=NULL)
		delete[] image.bmBits;
}

void CHeadTemplate::UpdateNodePos()
{
	for(CFacePanelNode *pNode=node_list.GetFirst();pNode;pNode=node_list.GetNext())
	{
		node_list.push_stack();
		if(pNode->attach_type==0)	//无任何依赖节点
			continue;
		else if(pNode->attach_type==1)
		{	//杆件上X坐标值不变点
			CFacePanelNode *pStartNode=&node_list[pNode->attach_node_index[0]];
			CFacePanelNode *pEndNode  =&node_list[pNode->attach_node_index[1]];
			if(pNode->attach_node_index[2]>=0&&pNode->attach_node_index[2]<node_list.GetNodeNum())
				pNode->pos.x=node_list[pNode->attach_node_index[2]].pos.x;
			pNode->pos.y=pStartNode->pos.y+
				(pNode->pos.x-pStartNode->pos.x)*(pEndNode->pos.y-pStartNode->pos.y)/(pEndNode->pos.x-pStartNode->pos.x);
		}
		else if(pNode->attach_type==2)
		{	//杆件上Y坐标值不变点
			CFacePanelNode *pStartNode=&node_list[pNode->attach_node_index[0]];
			CFacePanelNode *pEndNode  =&node_list[pNode->attach_node_index[1]];
			if(pNode->attach_node_index[2]>=0&&pNode->attach_node_index[2]<node_list.GetNodeNum())
				pNode->pos.y=node_list[pNode->attach_node_index[2]].pos.y;
			pNode->pos.x=pStartNode->pos.x+
				(pNode->pos.y-pStartNode->pos.y)*(pEndNode->pos.x-pStartNode->pos.x)/(pEndNode->pos.y-pStartNode->pos.y);
		}
		else if(pNode->attach_type==3)
		{	//杆件上Z坐标值不变点(暂按Y值不变计算)
			CFacePanelNode *pStartNode=&node_list[pNode->attach_node_index[0]];
			CFacePanelNode *pEndNode  =&node_list[pNode->attach_node_index[1]];
			if(pNode->attach_node_index[2]>=0&&pNode->attach_node_index[2]<node_list.GetNodeNum())
				pNode->pos.y=node_list[pNode->attach_node_index[2]].pos.y;
			pNode->pos.x=pStartNode->pos.x+
				(pNode->pos.y-pStartNode->pos.y)*(pEndNode->pos.x-pStartNode->pos.x)/(pEndNode->pos.y-pStartNode->pos.y);
		}
		else if(pNode->attach_type==4)
		{	//两杆件交叉交点
			f2dPoint start1=node_list[pNode->attach_node_index[0]].pos;
			f2dPoint end1=node_list[pNode->attach_node_index[1]].pos;
			f2dPoint start2  =node_list[pNode->attach_node_index[2]].pos;
			f2dPoint end2  =node_list[pNode->attach_node_index[3]].pos;
			Int2dll(f2dLine(start1,end1),f2dLine(start2,end2),pNode->pos.x,pNode->pos.y);
		}
		else if(pNode->attach_type==5)
		{	//杆件上的比例等分点
			f2dPoint start=node_list[pNode->attach_node_index[0]].pos;
			f2dPoint end  =node_list[pNode->attach_node_index[1]].pos;
			double real_scale=pNode->attach_scale;
			if(pNode->attach_scale<0)
			{
				real_scale=-pNode->attach_scale;
				real_scale=0.1*((int)real_scale)/(real_scale-(int)real_scale);
			}
			pNode->pos.x=start.x+real_scale*(end.x-start.x);
			pNode->pos.y=start.y+real_scale*(end.y-start.y);
		}
		node_list.pop_stack();
	}
}
void CHeadTemplate::ToStream(IStream *pStream)
{
	double version=1.2;
	ULARGE_INTEGER newSize;
	newSize.QuadPart=10;
	pStream->SetSize(newSize);
	pStream->Write(&version,sizeof(double),NULL);
	pStream->Write(&m_nKeyNode,sizeof(long),NULL);
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
	//示意图数据
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

		//将原位图文件压缩成PNG格式存储
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

	long n=para_list.GetNodeNum();
	pStream->Write(&n,sizeof(long),NULL);
	for(CHeadParameter *pPara=para_list.GetFirst();pPara;pPara=para_list.GetNext())
		pPara->ToStream(pStream);
	n=node_list.GetNodeNum();
	pStream->Write(&n,sizeof(long),NULL);
	for(CFacePanelNode *pNode=node_list.GetFirst();pNode;pNode=node_list.GetNext())
		pNode->ToStream(pStream);
	n=pole_list.GetNodeNum();
	pStream->Write(&n,sizeof(long),NULL);
	for(CFacePanelPole *pPole=pole_list.GetFirst();pPole;pPole=pole_list.GetNext())
		pPole->ToStream(pStream);
	n=windseg_list.GetNodeNum();
	pStream->Write(&n,sizeof(long),NULL);
	for(CFaceWindSegment *pWindSeg=windseg_list.GetFirst();pWindSeg;pWindSeg=windseg_list.GetNext())
		pWindSeg->ToStream(pStream);
}
void CHeadTemplate::FromStream(IStream *pStream)
{
	long n;
	double version=1.0;
	pStream->Read(&version,sizeof(double),NULL);
	pStream->Read(&m_nKeyNode,sizeof(long),NULL);
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
	//示意图数据
	if(image.bmBits!=NULL)
	{
		delete []image.bmBits;
		image.bmBits=NULL;
	}
	image.bmWidth=image.bmHeight=image.bmWidthBytes=0;
	if(version>=1.1)
	{
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

			//将原位图文件压缩成PNG格式存储
			ULONG comp_len;
			ULONG pLen = image.bmHeight*image.bmWidthBytes;
			pStream->Read(&comp_len,sizeof(ULONG),NULL);
			BYTE *buffer = new BYTE[comp_len];
			pStream->Read(buffer,comp_len,NULL);
			if(version>=1.2)
			{
				//
				if(image.bmBits!=NULL)
					delete []image.bmBits;
				image.bmBits = new BYTE[pLen];
				if(Z_OK != uncompress((Byte*)image.bmBits,&pLen,buffer,comp_len))
				{
					delete []buffer;
					return;
				}
				delete []buffer;
			}
			else
			{
				FILE* fp = fopen("png.tmp","w+b");
				fwrite(buffer,1,comp_len,fp);
				delete []buffer;
				fseek(fp,0,SEEK_SET);
				CImageFilePng pngfile;
				pngfile.ReadImageFile(fp);
				fclose(fp);
				if(image.bmBits!=NULL)
					delete []image.bmBits;
				image.bmBits = new BYTE[image.bmHeight*image.bmWidthBytes];
				pngfile.GetBitmapBits(image.bmHeight*image.bmWidthBytes,(BYTE*)image.bmBits);
			}
		}
	}
	pStream->Read(&n,sizeof(long),NULL);
	para_list.Empty();
	for(int i=0;i<n;i++)
	{
		CHeadParameter *pPara=para_list.append();
		pPara->FromStream(pStream);
	}
	pStream->Read(&n,sizeof(long),NULL);
	node_list.Empty();
	for(i=0;i<n;i++)
	{
		CFacePanelNode *pNode=node_list.append();
		pNode->FromStream(pStream);
	}
	pStream->Read(&n,sizeof(long),NULL);
	pole_list.Empty();
	for(i=0;i<n;i++)
	{
		CFacePanelPole *pPole=pole_list.append();
		pPole->FromStream(pStream);
	}
	if(version>=1.1)
	{
		pStream->Read(&n,sizeof(long),NULL);
		windseg_list.Empty();
		for(i=0;i<n;i++)
		{
			CFaceWindSegment *pWindSeg=windseg_list.append();
			pWindSeg->FromStream(pStream);
		}
	}
}
