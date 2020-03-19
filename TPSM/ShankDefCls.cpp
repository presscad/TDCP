#include "stdafx.h"
#include "ShankDefCls.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// CShankSection
//////////////////////////////////////////////////////////////////////
CShankSection::CShankSection()
{
	m_cObjType='J';
	m_pParent=NULL;
	btm_zstr.Copy("1/2");	//底部Z坐标位置字符串
	strcpy(front_layout,"");
	strcpy(side_layout,"");	//空表示不进行自动布材
	strcpy(hsect_layout,"");
	strcpy(v_layout,"");	//正面布材、侧面布材
	divide_n=1;				//节间内部等分子节间数
}
//////////////////////////////////////////////////////////////////////
// CShankLeg Class
//////////////////////////////////////////////////////////////////////
CShankLeg::CShankLeg()
{
	m_cObjType='L';
	m_pParentModule=NULL;
	m_bDatumLeg=FALSE;
	m_fFrontWidth=m_fSideWidth=m_fHeight=0;
	strcpy(front_layout,"");
	strcpy(side_layout,"");	//空表示不进行自动布材
	strcpy(hsect_layout,"");
	strcpy(v_layout,"");	//正面布材、侧面布材
}
CShankLeg::~CShankLeg()
{

}
CShankSection* CShankLeg::AppendSection()
{
	CShankSection section;
	section.m_pParent=this;
	CShankSection *pSection=sectionList.append(section);
	return pSection;
}
CShankSection* CShankLeg::InsertSection(int index/*=-1*/)
{
	CShankSection section;
	CShankSection *pSection=sectionList.insert(section,index);
	pSection->m_pParent=this;
	return pSection;
}
CShankSection* CShankLeg::SectionAtIndex(int index)
{
	if(index<0||index>=sectionList.GetSize())
		return NULL;
	else
		return &sectionList[index];
}
//////////////////////////////////////////////////////////////////////////
//CShankSegment
//////////////////////////////////////////////////////////////////////////
CShankSegment::CShankSegment()
{
	m_cObjType='D';
	m_fHeight=0;
	m_pShankDef=NULL;
	m_pParent=NULL;
	strcpy(front_layout,"");
	strcpy(side_layout,"");	//空表示不进行自动布材
	strcpy(hsect_layout,"");
	strcpy(v_layout,"");	//正面布材、侧面布材
	divide_n=1;				//节间内部等分子节间数
}
CShankSegment::~CShankSegment()
{

}
IMPLEMENT_GET(CShankSegment,m_fHeight)
{
	double height=0;
	for(int i=0;i<sectionList.GetSize();i++)
	{
		double h=atof(sectionList[i].btm_zstr);
		if(h>0)
			height+=h;
		else
			height+=h*_fHeight;
	}
	if(height>0&&_fHeight<=EPS)
		return height;
	return _fHeight;
}
IMPLEMENT_SET(CShankSegment,m_fHeight){_fHeight=value;}
CShankModule* CShankSegment::AppendModule()
{
	CShankModule* pModule=m_pShankDef->Modules.append();
	pModule->m_pShankDef=m_pShankDef;
	pModule->m_pParent=this;
	modulePtrList.append(pModule);
	return pModule;
}
CShankModule* CShankSegment::ModuleAtIndex(int index)
{
	if(index<0||index>=modulePtrList.GetSize())
		return NULL;
	else
		return modulePtrList[index];
}
CShankSection* CShankSegment::AppendSection()
{
	CShankSection section;
	section.m_pParent=this;
	CShankSection *pSection=sectionList.append(section);
	return pSection;
}
CShankSection* CShankSegment::InsertSection(int index/*=-1*/)
{
	CShankSection section;
	CShankSection *pSection=sectionList.insert(section,index);
	pSection->m_pParent=this;
	return pSection;
}

CShankSection* CShankSegment::SectionAtIndex(int index)
{
	if(index<0||index>=sectionList.GetSize())
		return NULL;
	else
		return sectionList.GetByIndex(index);
}
CShankSegment* CShankSegment::AppendJunctionSegment()
{
	CShankSegment *pSegment=segmentList.append();
	pSegment->m_pShankDef=m_pShankDef;
	pSegment->m_pParent=this;
	return pSegment;
}
CShankSegment* CShankSegment::InsertJunctionSegment(int index/*=-1*/)
{
	CShankSegment segment;
	CShankSegment *pSegment=segmentList.insert(segment,index);
	pSegment->m_pShankDef=m_pShankDef;
	pSegment->m_pParent=this;
	return pSegment;
}
CShankSegment* CShankSegment::JunctionSegmentAtIndex(int index)
{
	if(index<0||index>=segmentList.GetSize())
		return NULL;
	else
		return &segmentList[index];
}
CShankLeg* CShankSegment::GetDatumLeg()
{
	CShankLeg *pLeg=NULL;
	for(int i=0;i<modulePtrList.GetSize();i++)
	{
		CShankModule *pShankModule=modulePtrList[i];
		for(int j=0;j<pShankModule->SegmentNum();j++)
		{
			pLeg =pShankModule->SegmentAtIndex(j)->GetDatumLeg();
			if(pLeg!=NULL)
				return pLeg;
		}
		for(int j=0;j<pShankModule->LegNum();j++)
		{
			if(pShankModule->LegAtIndex(j)->m_fFrontWidth>0&&pShankModule->LegAtIndex(j)->m_fSideWidth>0)
				return pShankModule->LegAtIndex(j);
		}
	}
	return NULL;
}
double CShankSegment::GetDatumLegHeight()
{
	for(int i=0;i<modulePtrList.GetSize();i++)
	{
		CShankModule *pShankModule=modulePtrList[i];
		double height=m_fHeight;
		for(int j=0;j<pShankModule->SegmentNum();j++)
		{
			double h=pShankModule->SegmentAtIndex(j)->GetDatumLegHeight();
			if(h>0)
				return height+h;
		}
		for(int j=0;j<pShankModule->LegNum();j++)
		{
			if(pShankModule->LegAtIndex(j)->m_fFrontWidth>0&&pShankModule->LegAtIndex(j)->m_fSideWidth>0)
			{
				for(int k=0;k<pShankModule->SegmentNum();k++)	//呼高分组下的分段数据
					height+=pShankModule->SegmentAtIndex(k)->m_fHeight;	//可能会在删除接腿后输入新段时再加腿索引溢出错误 wjh-2011-11-25
				return height+pShankModule->LegAtIndex(j)->m_fHeight;
			}
		}
	}
	return 0;
}
//得到指定呼高分组
CShankModule *CShankSegment::GetSpecModule(char *sModuleName)
{
	for(int i=0;i<modulePtrList.GetSize();i++)
	{
		CShankModule *pShankModule=modulePtrList[i];
		if(pShankModule==NULL)
			continue;
		for(int j=0;j<pShankModule->SegmentNum();j++)
		{
			CShankModule *pModule=pShankModule->SegmentAtIndex(j)->GetSpecModule(sModuleName);
			if(pModule)
				return pModule;
		}
		if(sModuleName==NULL)
		{
			for(int j=0;j<pShankModule->LegNum();j++)
			{
				if(pShankModule->LegAtIndex(j)->m_fFrontWidth>0&&pShankModule->LegAtIndex(j)->m_fSideWidth>0)
					return pShankModule;	//获得基准呼高分组
			}
		}
		else if(stricmp(pShankModule->name,sModuleName)==0)
			return pShankModule;	//获得指定呼高分组
	}
	return NULL;
}
//获得指定呼高分组塔腿高度
double CShankSegment::GetSpecModuleLegHeight(char *sModuleName)
{
	if(sModuleName==NULL)
		return 0;
	int i=0,j=0,k=0;
	for(i=0;i<modulePtrList.GetSize();i++)
	{
		CShankModule *pShankModule=modulePtrList[i];
		double height=m_fHeight;
		for(j=0;j<pShankModule->SegmentNum();j++)
		{
			double h=pShankModule->SegmentAtIndex(j)->GetSpecModuleLegHeight(sModuleName);
			if(h>0)
				return height+h;
		}
		if(stricmp(pShankModule->name,sModuleName)==0)
		{
			for(j=0;j<pShankModule->LegNum();j++)
				height+=pShankModule->LegAtIndex(j)->m_fHeight;
			for(k=0;k<pShankModule->SegmentNum();k++)	//呼高分组下的分段数据
				height+=pShankModule->SegmentAtIndex(k)->m_fHeight;
			return height;
		}
	}
	return 0;
}
//返回指定接腿高度，要求pLeg必须指向当前坡段中的某一接腿对象
double CShankSegment::GetSpecLegHeight(CShankLeg *pLeg)
{
	for(int i=0;i<modulePtrList.GetSize();i++)
	{
		CShankModule *pShankModule=modulePtrList[i];
		double height=m_fHeight;
		for(int j=0;j<pShankModule->SegmentNum();j++)
		{
			double h=pShankModule->SegmentAtIndex(j)->GetSpecLegHeight(pLeg);
			if(h>0)
				return height+h;
			else
				height+=pShankModule->SegmentAtIndex(j)->m_fHeight;
		}
		for(int j=0;j<pShankModule->LegNum();j++)
		{
			if(pLeg==pShankModule->LegAtIndex(j))
			{
				height+=pLeg->m_fHeight;
				return height;
			}
		}
	}
	return 0;
}
int CShankSegment::AllocateBodyNo(int iInitLegNo)	//输入最低可分配接腿号,返回已占用最高接腿号
{
	for(int j=0;j<segmentList.GetSize();j++)
		iInitLegNo=segmentList[j].AllocateBodyNo(iInitLegNo);
	for(int i=0;i<modulePtrList.GetSize();i++)
		iInitLegNo=modulePtrList[i]->AllocateBodyNo(iInitLegNo);
	return iInitLegNo;
}
CFGLEG CShankSegment::AllocateBodyWord()
{
	bodyword=0;
	for(int i=0;i<modulePtrList.GetSize();i++)
		bodyword.AddBodyNo(modulePtrList[i]->m_iBodyNo);
	for(int j=0;j<segmentList.GetSize();j++)
		bodyword.AddSpecWord(segmentList[j].AllocateBodyWord());
	return bodyword;
}
bool CShankSegment::IsSegParentsHasModule()
{
	if(m_pParent->m_cObjType=='K')
		return false;
	else if(m_pParent->m_cObjType=='B')
		return true;
	else if(m_pParent->m_cObjType=='D')
		return m_pParent->Segment()->IsSegParentsHasModule();
	return false;
}
//////////////////////////////////////////////////////////////////////
// CShankModule
//////////////////////////////////////////////////////////////////////
CShankModule::CShankModule()
{
	m_cObjType='B';
	m_pShankDef=NULL;
	m_pParent=NULL;
	m_iBodyNo=1;						//本体部分对应接腿号
	legword.SetLegFlag(1);				//接腿配材号
	for(int i=0;i<4;i++)
		m_arrActiveQuadLegNo[i]=1;				//本模型当前第一～四象限的当前接腿号
}
CShankModule::~CShankModule()
{

}
bool CShankModule::IsFirstModule()
{
	if(m_pShankDef&&m_pShankDef->Modules.GetSize()>0&&m_pShankDef->Modules.GetFirst()==this)
		return true;
	else
		return false;
}
CShankSegment* CShankModule::AppendSegment()
{
	CShankSegment *pSegment=segList.append();
	pSegment->m_pShankDef=m_pShankDef;
	pSegment->m_pParent=this;
	return pSegment;
}
CShankSegment* CShankModule::InsertSegment(int index/*=-1*/)
{
	CShankSegment segment;
	CShankSegment *pSegment=segList.insert(segment,index);
	pSegment->m_pShankDef=m_pShankDef;
	pSegment->m_pParent=this;
	return pSegment;
}
CShankSegment* CShankModule::SegmentAtIndex(int index)
{
	if(index<0||index>=segList.GetSize())
		return NULL;
	else
		return &segList[index];
}
CShankLeg* CShankModule::AppendLeg()
{
	CShankLeg* pLeg=legList.append();
	pLeg->m_pParentModule=this;
	return pLeg;
}
int CShankModule::AllocateBodyNo(int iInitLegNo)	//输入最低可分配接腿号,返回已占用最高接腿号
{
	m_iBodyNo=iInitLegNo;
	m_arrActiveQuadLegNo[0]=m_arrActiveQuadLegNo[1]=m_arrActiveQuadLegNo[2]=m_arrActiveQuadLegNo[3]=(iInitLegNo-1)*8+1;
	for(int i=SegmentNum()-1;i>=0;i--)
		SegmentAtIndex(i)->bodyword.SetBodyFlag(GetSingleWord(m_iBodyNo));
	iInitLegNo++;
	return iInitLegNo;
}
bool CShankModule::AllocateLegWord()		//输入最低可分配接腿号,返回已占用最高接腿号
{
	legword=CFGLEG((BYTE)m_iBodyNo,1);
	int nMaxLegs=CFGLEG::MaxLegs();
	if(legList.GetNodeNum()>CFGLEG::MaxLegs())
	{
#ifdef AFX_TARG_ENU_ENGLISH
		AfxMessageBox(CXhChar100("Joint leg total number(%d) of current module %s is more than the joint leg distribution mode limited number %d of current module.",legList.GetNodeNum(),(char*)name,nMaxLegs));
#else
		AfxMessageBox(CXhChar100("当前呼高%s接腿总数(%d个)超过当前呼高接腿分配模式限定%d个",(char*)name,legList.GetNodeNum(),nMaxLegs));
#endif
		return false;
	}
	for(char i=0;i<legList.GetNodeNum()&&i<nMaxLegs;i++)
	{
		LegAtIndex(i)->legword=CFGLEG((BYTE)m_iBodyNo,i+1);
		legword.AddLegNo(i+1);
	}
	return true;
}
//////////////////////////////////////////////////////////////////////
//CShankSlope
//////////////////////////////////////////////////////////////////////
CShankSlope::CShankSlope()
{
	m_cObjType='K';
	m_cMainRodType='L';
	m_pShankDef=NULL;
	m_fTopFrontWidth=m_fTopSideWidth=m_fTopCoordZ=0;
	segList.Empty();
	modulePtrList.Empty();
}
CShankSlope::~CShankSlope()
{
	
}
CShankSegment* CShankSlope::AppendSegment()
{
	CShankSegment *pSegment=segList.append();
	pSegment->m_pShankDef=m_pShankDef;
	pSegment->m_pParent=this;
	return pSegment;
}
CShankSegment* CShankSlope::InsertSegment(int index/*=-1*/)
{
	CShankSegment segment;
	segment.m_pShankDef=m_pShankDef;
	segment.m_pParent=this;
	CShankSegment *pSegment=segList.insert(segment,index);
	return pSegment;
}
double CShankSlope::SlopeBodyHeight()
{
	double height=0;
	for(int i=0;i<segList.GetNodeNum();i++)
		height+=SegmentAtIndex(i)->m_fHeight;
	return height;
}
CShankModule* CShankSlope::AppendModule()
{
	CShankModule* pModule=m_pShankDef->Modules.append();
	pModule->m_pShankDef=m_pShankDef;
	pModule->m_pParent=this;
	modulePtrList.append(pModule);
	return pModule;
}
CShankModule* CShankSlope::ModuleAtIndex(int index)
{
	if(index<0||index>=modulePtrList.GetSize())
		return NULL;
	else
		return modulePtrList[index];
}
CShankLeg* CShankSlope::GetDatumLeg()
{
	//首先在坡段下的呼高分组中查找基准塔腿
	CShankLeg *pLeg=NULL;
	for(int i=0;i<modulePtrList.GetSize();i++)
	{
		CShankModule *pShankModule=modulePtrList[i];
		for(int j=0;j<pShankModule->SegmentNum();j++)
		{
			pLeg =pShankModule->SegmentAtIndex(j)->GetDatumLeg();
			if(pLeg!=NULL)
				return pLeg;
		}
		for(int j=0;j<pShankModule->LegNum();j++)
		{
			if(pShankModule->LegAtIndex(j)->m_fFrontWidth>0&&pShankModule->LegAtIndex(j)->m_fSideWidth>0)
				return pShankModule->LegAtIndex(j);
		}
	}
	//如果坡段数据下没有呼高分组则在分段数据下继续查找基准塔腿
	for(int i=0;i<SegmentNum();i++)
	{
		CShankLeg *pDatumLeg=SegmentAtIndex(i)->GetDatumLeg();
		if(pDatumLeg)
			return pDatumLeg;
	}
	return NULL;
}
double CShankSlope::GetDatumLegHeight()
{
	//首先在坡段下的呼高分组中查找基准塔腿
	for(int i=0;i<modulePtrList.GetSize();i++)
	{
		CShankModule *pShankModule=modulePtrList[i];
		double height=0;
		for(int j=0;j<pShankModule->SegmentNum();j++)
		{
			double h=pShankModule->SegmentAtIndex(j)->GetDatumLegHeight();
			if(h>0)
				return height+h;
		}
		for(int j=0;j<pShankModule->LegNum();j++)
		{
			if(pShankModule->LegAtIndex(j)->m_fFrontWidth>0&&pShankModule->LegAtIndex(j)->m_fSideWidth>0)
			{
				for(int k=0;k<pShankModule->SegmentNum();k++)	//呼高分组下的分段数据
					height+=pShankModule->SegmentAtIndex(i)->m_fHeight;
				return height+pShankModule->LegAtIndex(j)->m_fHeight;
			}
		}
	}
	//如果坡段数据下没有呼高分组则在分段数据下继续查找基准塔腿
	double height=0;
	for(int i=0;i<SegmentNum();i++)
	{
		CShankSegment *pSegment=SegmentAtIndex(i);
		double h=pSegment->GetDatumLegHeight();
		if(h>0)	//找到基准腿
			return height+h;
		else
			height+=pSegment->m_fHeight;
	}
	return 0;
}
//返回指定接腿高度，要求pLeg必须指向当前坡段中的某一接腿对象
double CShankSlope::GetSpecLegHeight(CShankLeg *pLeg)
{
	//首先在坡段下的呼高分组中查找基准塔腿
	for(int i=0;i<modulePtrList.GetSize();i++)
	{
		CShankModule *pShankModule=modulePtrList[i];
		double height=0;
		for(int j=0;j<pShankModule->SegmentNum();j++)
		{
			double h=pShankModule->SegmentAtIndex(j)->GetDatumLegHeight();
			if(h>0)
				return height+h;
		}
		for(int j=0;j<pShankModule->LegNum();j++)
		{
			if(pShankModule->LegAtIndex(j)==pLeg)
			{
				for(int k=0;k<pShankModule->SegmentNum();k++)	//呼高分组下的分段数据
					height+=pShankModule->SegmentAtIndex(i)->m_fHeight;
				return height+pLeg->m_fHeight;
			}
		}
	}
	//如果坡段数据下没有呼高分组则在分段数据下继续查找基准塔腿
	double height=0;
	for(int i=0;i<SegmentNum();i++)
	{
		CShankSegment *pSegment=SegmentAtIndex(i);
		double h=pSegment->GetDatumLegHeight();
		if(h>0)	//找到基准腿
			return height+h;
		else
			height+=pSegment->m_fHeight;
	}
	return 0;
}
//得到指定呼高分组
CShankModule *CShankSlope::GetSpecModule(char *sModuleName)
{
	//首先在坡段下的呼高分组中查找基准塔腿
	for(int i=0;i<modulePtrList.GetSize();i++)
	{
		CShankModule *pShankModule=modulePtrList[i];
		if(pShankModule==NULL)
			continue;
		for(int j=0;j<pShankModule->SegmentNum();j++)
		{
			CShankModule *pModule=pShankModule->SegmentAtIndex(j)->GetSpecModule(sModuleName);
			if(pModule)
				return pModule;
		}
		if(sModuleName==NULL)
		{
			for(int j=0;j<pShankModule->LegNum();j++)
			{
				if(pShankModule->LegAtIndex(j)->m_fFrontWidth>0&&pShankModule->LegAtIndex(j)->m_fSideWidth>0)
					return pShankModule;	//获得基准呼高分组
			}
		}
		else if(stricmp(pShankModule->name,sModuleName)==0)
			return pShankModule;	//获得指定呼高分组
	}
	//如果坡段数据下没有呼高分组则在分段数据下继续查找基准塔腿
	for(int i=0;i<SegmentNum();i++)
	{
		CShankModule *pShankModule=SegmentAtIndex(i)->GetSpecModule(sModuleName);
		if(pShankModule)
			return pShankModule;
	}
	return NULL;
}
//获得指定呼高分组塔腿高度
double CShankSlope::GetSpecModuleLegHeight(char *sModuleName)
{
	if(sModuleName==NULL)
		return 0;
	//首先在坡段下的呼高分组中查找基准塔腿
	int i=0,j=0,k=0;
	for(i=0;i<modulePtrList.GetSize();i++)
	{
		CShankModule *pShankModule=modulePtrList[i];
		double height=0;
		for(j=0;j<pShankModule->SegmentNum();j++)
		{
			double h=pShankModule->SegmentAtIndex(j)->GetSpecModuleLegHeight(sModuleName);
			if(h>0)
				return height+h;
		}
		if(stricmp(pShankModule->name,sModuleName)==0)
		{
			for(j=0;j<pShankModule->LegNum();j++)
				height+=pShankModule->LegAtIndex(j)->m_fHeight;
			for(k=0;k<pShankModule->SegmentNum();k++)	//呼高分组下的分段数据
				height+=pShankModule->SegmentAtIndex(k)->m_fHeight;
			return height;
		}
	}
	//如果坡段数据下没有呼高分组则在分段数据下继续查找基准塔腿
	double height=0;
	for(i=0;i<SegmentNum();i++)
	{
		CShankSegment *pSegment=SegmentAtIndex(i);
		double h=pSegment->GetSpecModuleLegHeight(sModuleName);
		if(h>0)	//找到基准腿
			return height+h;
		else
			height+=pSegment->m_fHeight;
	}
	return 0;
}
//////////////////////////////////////////////////////////////////////
//CShankDefine
//////////////////////////////////////////////////////////////////////
CShankSlope *CShankDefine::AppendSlope()
{
	CShankSlope *pSlope=Slopes.append();
	pSlope->m_pShankDef=this;
	return pSlope;
}
CShankSlope *CShankDefine::SlopeAtIndex(int index)
{
	if(index<0||index>=Slopes.GetSize())
		return NULL;
	return &Slopes[index];
}
CShankModule *CShankDefine::ModuleAtIndex(int index)
{
	if(index<0||index>=Modules.GetSize())
		return NULL;
	else
		return &Modules[index];
}
double CShankDefine::SlopeHeight(int iSlope,CShankLeg *pLeg/*=NULL*/)
{
	if(iSlope>=0&&iSlope<Slopes.GetSize()-1)
		return SlopeAtIndex(iSlope)->SlopeBodyHeight();
	else if(iSlope<0||iSlope>=Slopes.GetSize())
		return 0;
	else
	{
		double height=0;
		if(pLeg)
			height=SlopeAtIndex(iSlope)->GetSpecLegHeight(pLeg);
		else
			height=SlopeAtIndex(iSlope)->GetDatumLegHeight();
		return height;
	}
}
double CShankDefine::SlopeTopFrontWidth(int iSlope,CShankLeg *pLeg/*=NULL*/)
{
	if(iSlope>=0&&iSlope<Slopes.GetSize())
		return SlopeAtIndex(iSlope)->m_fTopFrontWidth;
	else if(iSlope==Slopes.GetSize())	//最末尾坡度段底部宽度,
	{
		CShankSlope *pSlope=SlopeAtIndex(iSlope-1);
		if(pSlope==NULL)
			return 0;
		if(pLeg==NULL)
		{	//基准接腿正面宽度
			CShankLeg *pDatumLeg=pSlope->GetDatumLeg();
			if(pDatumLeg)
				return pDatumLeg->m_fFrontWidth;
		}
		else
		{	//指定接腿正面宽度
			double fSlopeTopWidth=SlopeTopFrontWidth(iSlope-1);
			double fSlopeBottomWidth=SlopeBtmFrontWidth(iSlope-1);
			double fSlopeHeight=SlopeHeight(iSlope-1);
			double fSpecSlopeHeight=SlopeHeight(iSlope-1,pLeg);
			if(fSlopeHeight>0)
			{
				double fScale=0.5*(fSlopeBottomWidth-fSlopeTopWidth)/fSlopeHeight;
				return fSlopeTopWidth+fScale*fSpecSlopeHeight*2;
			}
		}
	}
	return 0;
}
double CShankDefine::SlopeBtmFrontWidth(UINT iSlope,CShankLeg *pLeg/*=NULL*/)
{
	return SlopeTopFrontWidth(iSlope+1,pLeg);
}
double CShankDefine::SlopeTopSideWidth(int iSlope,CShankLeg *pLeg/*=NULL*/)
{
	if(iSlope>=0&&iSlope<Slopes.GetSize())
		return SlopeAtIndex(iSlope)->m_fTopSideWidth;
	else if(iSlope==Slopes.GetSize())	//最末尾坡度段底部宽度,
	{
		CShankSlope *pSlope=SlopeAtIndex(iSlope-1);
		if(pSlope==NULL)
			return 0;
		if(pLeg==NULL)
		{
			CShankLeg *pDatumLeg=pSlope->GetDatumLeg();
			if(pDatumLeg)
				return pDatumLeg->m_fSideWidth;
			/*
			for(int i=0;i<pSlope->SegmentNum();i++)
			{
				CShankLeg *pDatumLeg=pSlope->SegmentAtIndex(i)->GetDatumLeg();
				if(pDatumLeg)
					return pDatumLeg->m_fSideWidth;
			}*/
		}
		else
		{	//指定接腿侧面宽度
			double fSlopeTopWidth=SlopeTopSideWidth(iSlope-1);
			double fSlopeBottomWidth=SlopeBtmSideWidth(iSlope-1);
			double fSlopeHeight=SlopeHeight(iSlope-1);
			double fSpecSlopeHeight=SlopeHeight(iSlope-1,pLeg);
			if(fSlopeHeight>0)
			{
				double fScale=0.5*(fSlopeBottomWidth-fSlopeTopWidth)/fSlopeHeight;
				return fSlopeTopWidth+fScale*fSpecSlopeHeight*2;
			}
		}

	}
	return 0;
}
double CShankDefine::SlopeBtmSideWidth(UINT iSlope,CShankLeg *pLeg/*=NULL*/)
{
	return SlopeTopSideWidth(iSlope+1,pLeg);
}
double CShankDefine::SlopeTopCoordZ(int iSlope)
{
	if(iSlope>0&&iSlope<Slopes.GetSize())
	{
		double coordZ=SlopeAtIndex(0)->m_fTopCoordZ;
		for(int i=0;i<iSlope;i++)
			coordZ+=SlopeAtIndex(i)->SlopeBodyHeight();
		return coordZ;
	}
	else if(iSlope==0)
		return SlopeAtIndex(0)->m_fTopCoordZ;
	else if(iSlope==Slopes.GetSize())
		return SlopeBtmCoordZ(iSlope-1);
	else
		return 0;
}
double CShankDefine::SlopeBtmCoordZ(int iSlope,CShankLeg *pLeg/*=NULL*/)
{
	return SlopeTopCoordZ(iSlope)+SlopeHeight(iSlope,pLeg);
}
CFGWORD CShankDefine::SlopeTopCfgword(int iSlope)
{
	if(iSlope<Slopes.GetSize())
	{
		CFGLEG bodyword;
		for(int i=0;i<SlopeAtIndex(iSlope)->ModuleNum();i++)
			bodyword.AddBodyNo(SlopeAtIndex(iSlope)->ModuleAtIndex(i)->m_iBodyNo);
		if(SlopeAtIndex(iSlope)->SegmentNum()>0)
			bodyword.AddSpecWord(SlopeAtIndex(iSlope)->SegmentAtIndex(0)->bodyword);
		return bodyword.ToCfgWord();
	}
	else if(iSlope==Slopes.GetSize())
	{
		for(int i=0;i<SlopeAtIndex(iSlope-1)->SegmentNum();i++)
		{
			CShankLeg *pLeg=SlopeAtIndex(iSlope-1)->SegmentAtIndex(i)->GetDatumLeg();
			if(pLeg)
				return pLeg->legword.ToCfgWord();
		}
	}
	return CFGWORD(1);
}
//获得坡段顶部节点段号
SEGI CShankDefine::SlopeTopSegI(int iSlope)
{
	if(iSlope<Slopes.GetSize())
	{
		if(SlopeAtIndex(iSlope)->SegmentNum()>0)
			return SlopeAtIndex(iSlope)->SegmentAtIndex(0)->m_iSeg;
	}
	else if(iSlope==Slopes.GetSize())
	{	
		for(int i=0;i<SlopeAtIndex(iSlope-1)->SegmentNum();i++)
		{
			CShankLeg *pLeg=SlopeAtIndex(iSlope-1)->SegmentAtIndex(i)->GetDatumLeg();
			if(pLeg)
				return pLeg->m_iSeg;
		}
	}
	return SEGI();
}
bool CShankDefine::AllocateCfgWord()
{
	int i,j,k;
	int iInitLegNo=1;
	CShankSlope *pBottomSlope=Slopes.GetTail();
	if(pBottomSlope==NULL)
		return false;
	//1.首先为直属于底部坡段的呼高分组分配配材号	wht 10-07-24
	if(pBottomSlope->ModuleNum()>CFGLEG::MaxBodys())
	{
#ifdef AFX_TARG_ENU_ENGLISH
		AfxMessageBox(CXhChar50("Module total number (%d) is more than the joint leg distribution mode limited number %d of current module.",pBottomSlope->ModuleNum(),CFGLEG::MaxBodys()));
#else
		AfxMessageBox(CXhChar50("呼高总数(%d个)超过当前呼高接腿分配模式限定%d个",pBottomSlope->ModuleNum(),CFGLEG::MaxBodys()));
#endif
		return false;
	}
	for(i=0;i<pBottomSlope->ModuleNum();i++)
		iInitLegNo=pBottomSlope->ModuleAtIndex(i)->AllocateBodyNo(iInitLegNo);
	//2.然后为间接属于底部坡段的呼高分组分配配材号
	for(i=0;i<pBottomSlope->SegmentNum();i++)
		iInitLegNo=pBottomSlope->SegmentAtIndex(i)->AllocateBodyNo(iInitLegNo);
	CFGLEG bodyword(0);
	for(CShankSlope *pSlope=Slopes.GetTail();pSlope;pSlope=Slopes.GetPrev())
	{
		for(j=pSlope->SegmentNum()-1;j>=0;j--)
		{
			//将当前段之下的分段的塔身本体号包括进来
			bodyword.AddSpecWord(pSlope->SegmentAtIndex(j)->AllocateBodyWord());
			pSlope->SegmentAtIndex(j)->bodyword=bodyword;
			//需要从当前坡段下塔身分段的配材号中将坡段直属呼高分组本体号剔除（其实感觉不需要以下代码，但需要测试后再删除） wjh-2013.5.17
			//for(k=0;k<pSlope->ModuleNum();k++)
			//	pSlope->SegmentAtIndex(j)->bodyword.RemoveSpecNoWord(pSlope->ModuleAtIndex(k)->m_iBodyNo,false);
		}
		//针对通信塔中最下坡段仅一个呼高接腿段情况，添加相应配材号 wjh-2016.3.2
		for(k=0;k<pSlope->ModuleNum();k++)
			bodyword.AddBodyNo(pSlope->ModuleAtIndex(k)->m_iBodyNo);
	}
	//自动分配腿部接腿号
	for(j=0;j<ModuleNum();j++)
	{
		if(!ModuleAtIndex(j)->AllocateLegWord())
			return false;
	}
	return true;
}