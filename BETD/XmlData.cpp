#include "stdafx.h"
#include "XmlData.h"
#include "Markup.h"
#include "XhCharString.h"
#include "SortFunc.h"

static int comparefun(const HANG_POINT& item1, const HANG_POINT& item2)
{
	return compare_double(item1.m_xHangPos.y, item2.m_xHangPos.y);
}
//////////////////////////////////////////////////////////////////////////
//CHangPhase
CHangPhase::CHangPhase()
{
	m_dwHangPhase=1;			//相序号
	m_xBackHangPtArr.Empty();	//后侧挂点
	m_xFrontHangPtArr.Empty();	//前侧挂点
	m_xWireJHangPtArr.Empty();	//跳线挂点
}
BOOL CHangPhase::GetBKVirtualHangPt(HANG_POINT& hang_pt_v)
{
	int nSize=m_xBackHangPtArr.GetSize();
	if(nSize<=1)
		return FALSE;
	f3dPoint pos;
	for(int i=0;i<nSize;i++)
		pos+=m_xBackHangPtArr[i].m_xHangPos;
	pos/=nSize;
	hang_pt_v.m_xHangPos=pos;
	strcpy(hang_pt_v.m_sHangName,"虚拟挂点");
	strcpy(hang_pt_v.m_sHangOrder,"1");
	hang_pt_v.m_ciWireType=m_xBackHangPtArr[0].m_ciWireType;
	hang_pt_v.m_ciTensionType=m_xBackHangPtArr[0].m_ciTensionType;
	hang_pt_v.m_ciHangingStyle=m_xBackHangPtArr[0].m_ciHangingStyle;
	hang_pt_v.m_ciHangDirect=m_xBackHangPtArr[0].m_ciHangDirect;
	return TRUE;
}
BOOL CHangPhase::GetFTVirtualHangPt(HANG_POINT& hang_pt_v)
{
	int nSize=m_xFrontHangPtArr.GetSize();
	if(nSize<=1)
		return FALSE;
	f3dPoint pos;
	for(int i=0;i<nSize;i++)
		pos+=m_xFrontHangPtArr[i].m_xHangPos;
	pos/=nSize;
	hang_pt_v.m_xHangPos=pos;
	strcpy(hang_pt_v.m_sHangName,"虚拟挂点");
	if(m_xWireJHangPtArr.GetSize()<=0)
		strcpy(hang_pt_v.m_sHangOrder,"2");
	else
		strcpy(hang_pt_v.m_sHangOrder,"3");
	hang_pt_v.m_ciWireType=m_xFrontHangPtArr[0].m_ciWireType;
	hang_pt_v.m_ciTensionType=m_xFrontHangPtArr[0].m_ciTensionType;
	hang_pt_v.m_ciHangingStyle=m_xFrontHangPtArr[0].m_ciHangingStyle;
	hang_pt_v.m_ciHangDirect=m_xFrontHangPtArr[0].m_ciHangDirect;
	return TRUE;
}
BOOL CHangPhase::GetWJVirtualHangPt(HANG_POINT& hang_pt_v)
{
	int nSize=m_xWireJHangPtArr.GetSize();
	if(nSize<=1)
		return FALSE;
	f3dPoint pos;
	for(int i=0;i<nSize;i++)
		pos+=m_xWireJHangPtArr[i].m_xHangPos;	
	pos/=nSize;
	hang_pt_v.m_xHangPos=pos;
	strcpy(hang_pt_v.m_sHangName,"虚拟挂点");
	strcpy(hang_pt_v.m_sHangOrder,"2");
	hang_pt_v.m_ciWireType=m_xWireJHangPtArr[0].m_ciWireType;
	hang_pt_v.m_ciTensionType=m_xWireJHangPtArr[0].m_ciTensionType;
	hang_pt_v.m_ciHangingStyle=m_xWireJHangPtArr[0].m_ciHangingStyle;
	hang_pt_v.m_ciHangDirect=m_xWireJHangPtArr[0].m_ciHangingStyle;
	return TRUE;
}
//////////////////////////////////////////////////////////////////////////
//CEarthLoop
BOOL CEarthLoop::GetBKVirtualHangPt(HANG_POINT& hang_pt_v)
{
	int nSize = m_xBackHangPtList.GetSize();
	if (nSize <= 1)
		return FALSE;
	f3dPoint pos;
	for (int i = 0; i < nSize; i++)
		pos += m_xBackHangPtList[i].m_xHangPos;
	pos /= nSize;
	hang_pt_v.m_xHangPos = pos;
	strcpy(hang_pt_v.m_sHangName, "虚拟挂点");
	strcpy(hang_pt_v.m_sHangOrder, "1");
	hang_pt_v.m_ciWireType = m_xBackHangPtList[0].m_ciWireType;
	hang_pt_v.m_ciTensionType = m_xBackHangPtList[0].m_ciTensionType;
	hang_pt_v.m_ciHangingStyle = m_xBackHangPtList[0].m_ciHangingStyle;
	hang_pt_v.m_ciHangDirect = m_xBackHangPtList[0].m_ciHangingStyle;
	return TRUE;
}
BOOL CEarthLoop::GetFTVirtualHangPt(HANG_POINT& hang_pt_v)
{
	int nSize = m_xFrontHangPtList.GetSize();
	if (nSize <= 1)
		return FALSE;
	f3dPoint pos;
	for (int i = 0; i < nSize; i++)
		pos += m_xFrontHangPtList[i].m_xHangPos;
	pos /= nSize;
	hang_pt_v.m_xHangPos = pos;
	strcpy(hang_pt_v.m_sHangName, "虚拟挂点");
	strcpy(hang_pt_v.m_sHangOrder, "1");
	hang_pt_v.m_ciWireType = m_xFrontHangPtList[0].m_ciWireType;
	hang_pt_v.m_ciTensionType = m_xFrontHangPtList[0].m_ciTensionType;
	hang_pt_v.m_ciHangingStyle = m_xFrontHangPtList[0].m_ciHangingStyle;
	hang_pt_v.m_ciHangDirect = m_xFrontHangPtList[0].m_ciHangingStyle;
	return TRUE;
}
//////////////////////////////////////////////////////////////////////////
//CXmlModel
CXmlModel::CXmlModel(BOOL bUseUnitM/*=FALSE*/)
{
	m_bTransPtMMtoM=bUseUnitM;
	m_ciModel=0;
	strcpy(m_sCategory,"铁塔");
	strcpy(m_sTowerFile,"");
	strcpy(m_sTowerName,"");
	strcpy(m_xFundation[0].m_sFeetName,"A");
	strcpy(m_xFundation[1].m_sFeetName,"B");
	strcpy(m_xFundation[2].m_sFeetName,"C");
	strcpy(m_xFundation[3].m_sFeetName,"D");
	m_hashTowerLoop.Empty();
}
CXmlModel::~CXmlModel()
{

}
void CXmlModel::AmendHangInfo(BOOL bTurnLeft /*= FALSE*/)
{
	CHashListEx<CTowerLoop> hashLoop;
	if (bTurnLeft && m_hashTowerLoop.GetNodeNum() == 1)
	{
		CTowerLoop* pLoop = m_hashTowerLoop.GetFirst();
		CTowerLoop* pNewLoop = hashLoop.Add(pLoop->m_dwLoop);
		DWORD dwPhaseId = 1;
		for (CHangPhase* pPhase = pLoop->m_hashHangPhase.GetTail(); pPhase; pPhase = pLoop->m_hashHangPhase.GetPrev())
		{
			CHangPhase* pNewPhase = pNewLoop->m_hashHangPhase.Add(dwPhaseId);
			if (pPhase->m_xFrontHangPtArr.GetSize() > 0)
			{	//前后对调
				for (HANG_POINT* pHangPt = pPhase->m_xBackHangPtArr.GetFirst(); pHangPt; pHangPt = pPhase->m_xBackHangPtArr.GetNext())
				{
					HANG_POINT* pNewHangPt = pNewPhase->m_xFrontHangPtArr.append();
					pNewHangPt->CopyMember(pHangPt,TRUE);
				}
				for (HANG_POINT* pHangPt = pPhase->m_xFrontHangPtArr.GetFirst(); pHangPt; pHangPt = pPhase->m_xFrontHangPtArr.GetNext())
				{
					HANG_POINT* pNewHangPt = pNewPhase->m_xBackHangPtArr.append();
					pNewHangPt->CopyMember(pHangPt,TRUE);
				}
			}
			else
			{
				for (HANG_POINT* pHangPt = pPhase->m_xBackHangPtArr.GetFirst(); pHangPt; pHangPt = pPhase->m_xBackHangPtArr.GetNext())
				{
					HANG_POINT* pNewHangPt = pNewPhase->m_xBackHangPtArr.append();
					pNewHangPt->CopyMember(pHangPt);
				}
			}
			for (HANG_POINT* pHangPt = pPhase->m_xWireJHangPtArr.GetFirst(); pHangPt; pHangPt = pPhase->m_xWireJHangPtArr.GetNext())
			{
				HANG_POINT* pNewHangPt = pNewPhase->m_xWireJHangPtArr.append();
				pNewHangPt->CopyMember(pHangPt);
			}
			//
			dwPhaseId++;
		}
	}
	else if (bTurnLeft && m_hashTowerLoop.GetNodeNum() > 1)
	{	//左转杆塔，需要调整回路
		DWORD dwPhaseId = 1;
		for (CTowerLoop* pLoop = m_hashTowerLoop.GetTail(); pLoop; pLoop = m_hashTowerLoop.GetPrev())
		{
			CTowerLoop* pNewLoop = hashLoop.Add(0);
			for (CHangPhase* pPhase = pLoop->m_hashHangPhase.GetFirst(); pPhase; pPhase = pLoop->m_hashHangPhase.GetNext())
			{
				CHangPhase* pNewPhase = pNewLoop->m_hashHangPhase.Add(dwPhaseId);
				if (pPhase->m_xFrontHangPtArr.GetSize() > 0)
				{	//前后对调
					for (HANG_POINT* pHangPt = pPhase->m_xBackHangPtArr.GetFirst(); pHangPt; pHangPt = pPhase->m_xBackHangPtArr.GetNext())
					{
						HANG_POINT* pNewHangPt = pNewPhase->m_xFrontHangPtArr.append();
						pNewHangPt->CopyMember(pHangPt,TRUE);
					}
					for (HANG_POINT* pHangPt = pPhase->m_xFrontHangPtArr.GetFirst(); pHangPt; pHangPt = pPhase->m_xFrontHangPtArr.GetNext())
					{
						HANG_POINT* pNewHangPt = pNewPhase->m_xBackHangPtArr.append();
						pNewHangPt->CopyMember(pHangPt,TRUE);
					}
				}
				else
				{
					for (HANG_POINT* pHangPt = pPhase->m_xBackHangPtArr.GetFirst(); pHangPt; pHangPt = pPhase->m_xBackHangPtArr.GetNext())
					{
						HANG_POINT* pNewHangPt = pNewPhase->m_xBackHangPtArr.append();
						pNewHangPt->CopyMember(pHangPt);
					}
				}
				for (HANG_POINT* pHangPt = pPhase->m_xWireJHangPtArr.GetFirst(); pHangPt; pHangPt = pPhase->m_xWireJHangPtArr.GetNext())
				{
					HANG_POINT* pNewHangPt = pNewPhase->m_xWireJHangPtArr.append();
					pNewHangPt->CopyMember(pHangPt);
				}
				//
				dwPhaseId++;
			}
		}
	}
	if(hashLoop.GetNodeNum()>0)
	{
		m_hashTowerLoop.Empty();
		for (CTowerLoop* pLoop = hashLoop.GetTail(); pLoop; pLoop = hashLoop.GetPrev())
		{
			CTowerLoop* pNewLoop = m_hashTowerLoop.Add(pLoop->m_dwLoop);
			for (CHangPhase* pPhase = pLoop->m_hashHangPhase.GetFirst(); pPhase; pPhase = pLoop->m_hashHangPhase.GetNext())
			{
				CHangPhase* pNewPhase = pNewLoop->m_hashHangPhase.Add(pPhase->m_dwHangPhase);
				for (HANG_POINT* pHangPt = pPhase->m_xBackHangPtArr.GetFirst(); pHangPt; pHangPt = pPhase->m_xBackHangPtArr.GetNext())
				{
					HANG_POINT* pNewHangPt = pNewPhase->m_xBackHangPtArr.append();
					pNewHangPt->CopyMember(pHangPt);
				}
				for (HANG_POINT* pHangPt = pPhase->m_xFrontHangPtArr.GetFirst(); pHangPt; pHangPt = pPhase->m_xFrontHangPtArr.GetNext())
				{
					HANG_POINT* pNewHangPt = pNewPhase->m_xFrontHangPtArr.append();
					pNewHangPt->CopyMember(pHangPt);
				}
				for (HANG_POINT* pHangPt = pPhase->m_xWireJHangPtArr.GetFirst(); pHangPt; pHangPt = pPhase->m_xWireJHangPtArr.GetNext())
				{
					HANG_POINT* pNewHangPt = pNewPhase->m_xWireJHangPtArr.append();
					pNewHangPt->CopyMember(pHangPt);
				}
			}
		}
	}
	//对导线挂点进行排序(线路走向是从后向前，Y值是从负到正)
	for (CTowerLoop* pLoop = m_hashTowerLoop.GetFirst(); pLoop; pLoop = m_hashTowerLoop.GetNext())
	{
		for (CHangPhase* pPhase = pLoop->m_hashHangPhase.GetFirst(); pPhase; pPhase = pLoop->m_hashHangPhase.GetNext())
		{
			CQuickSort<HANG_POINT>::QuickSort(pPhase->m_xBackHangPtArr.m_pData, pPhase->m_xBackHangPtArr.GetSize(), comparefun);
			if(pPhase->m_xFrontHangPtArr.GetSize()>0)
				CQuickSort<HANG_POINT>::QuickSort(pPhase->m_xFrontHangPtArr.m_pData, pPhase->m_xFrontHangPtArr.GetSize(), comparefun);
			if(pPhase->m_xWireJHangPtArr.GetSize()>0)
				CQuickSort<HANG_POINT>::QuickSort(pPhase->m_xWireJHangPtArr.m_pData, pPhase->m_xWireJHangPtArr.GetSize(), comparefun);
		}
	}
	//导线编号
	for(CTowerLoop* pLoop=m_hashTowerLoop.GetFirst();pLoop;pLoop=m_hashTowerLoop.GetNext())
	{
		for(CHangPhase* pPhase=pLoop->m_hashHangPhase.GetFirst();pPhase;pPhase=pLoop->m_hashHangPhase.GetNext())
		{
			int iWireCode=1,iJumpCode=1,iOrder=1,index=1;
			for(HANG_POINT* pHangPt=pPhase->m_xBackHangPtArr.GetFirst();pHangPt;pHangPt=pPhase->m_xBackHangPtArr.GetNext(),iOrder++)
			{
				if(pHangPt->m_ciTensionType==0)
					sprintf(pHangPt->m_sHangName,"P%dX%d",pPhase->m_dwHangPhase,iWireCode);
				else
					sprintf(pHangPt->m_sHangName,"P%dN%d",pPhase->m_dwHangPhase,iWireCode);
				sprintf(pHangPt->m_sHangOrder,"1-%d",iOrder);
				iWireCode++;
			}
			for(HANG_POINT* pHangPt=pPhase->m_xWireJHangPtArr.GetFirst();pHangPt;pHangPt=pPhase->m_xWireJHangPtArr.GetNext(),iOrder++)
			{
				sprintf(pHangPt->m_sHangName,"P%dT%d",pPhase->m_dwHangPhase,iJumpCode);
				sprintf(pHangPt->m_sHangOrder,"2-%d",iOrder);
				iJumpCode++;
			}
			int ii=(pPhase->m_xWireJHangPtArr.GetSize()>0)?3:2;
			for(HANG_POINT* pHangPt=pPhase->m_xFrontHangPtArr.GetFirst();pHangPt;pHangPt=pPhase->m_xFrontHangPtArr.GetNext(),iOrder++)
			{
				if(pHangPt->m_ciTensionType==0)
					sprintf(pHangPt->m_sHangName,"P%dX%d",pPhase->m_dwHangPhase,iWireCode);
				else
					sprintf(pHangPt->m_sHangName,"P%dN%d",pPhase->m_dwHangPhase,iWireCode);
				sprintf(pHangPt->m_sHangOrder,"%d-%d",ii,iOrder);
				iWireCode++;
			}
		}
		if(pLoop->m_hashHangPhase.GetNodeNum()==3)
			pLoop->m_sCurrent.Copy("交流");
		else //if(pLoop->m_hashHangPhase.GetNodeNum()==2)
			pLoop->m_sCurrent.Copy("直流");
	}
	//地线编号
	if(bTurnLeft && m_hashEarthLoop.GetNodeNum()>1)
	{
		CHashListEx<CEarthLoop> hashEarth;
		for(CEarthLoop* pLoop = m_hashEarthLoop.GetTail(); pLoop; pLoop = m_hashEarthLoop.GetPrev())
		{
			CEarthLoop* pNewLoop = hashEarth.Add(0);
			if (pLoop->m_xFrontHangPtList.GetSize() > 0)
			{	//前后对调
				for (HANG_POINT* pHangPt = pLoop->m_xBackHangPtList.GetFirst(); pHangPt; pHangPt = pLoop->m_xBackHangPtList.GetNext())
				{
					HANG_POINT* pNewHangPt = pNewLoop->m_xFrontHangPtList.append();
					pNewHangPt->CopyMember(pHangPt,TRUE);
				}
				for (HANG_POINT* pHangPt = pLoop->m_xFrontHangPtList.GetFirst(); pHangPt; pHangPt = pLoop->m_xFrontHangPtList.GetNext())
				{
					HANG_POINT* pNewHangPt = pNewLoop->m_xBackHangPtList.append();
					pNewHangPt->CopyMember(pHangPt,TRUE);
				}
			}
			else
			{
				for (HANG_POINT* pHangPt = pLoop->m_xBackHangPtList.GetFirst(); pHangPt; pHangPt = pLoop->m_xBackHangPtList.GetNext())
				{
					HANG_POINT* pNewHangPt = pNewLoop->m_xBackHangPtList.append();
					pNewHangPt->CopyMember(pHangPt);
				}
			}
		}
		//
		m_hashEarthLoop.Empty();
		for(CEarthLoop* pLoop = hashEarth.GetFirst(); pLoop; pLoop = hashEarth.GetNext())
		{
			CEarthLoop* pNewLoop = m_hashEarthLoop.Add(pLoop->m_dwLoop);
			for (HANG_POINT* pHangPt = pLoop->m_xBackHangPtList.GetFirst(); pHangPt; pHangPt = pLoop->m_xBackHangPtList.GetNext())
			{
				HANG_POINT* pNewHangPt = pNewLoop->m_xBackHangPtList.append();
				pNewHangPt->CopyMember(pHangPt);
			}
			for (HANG_POINT* pHangPt = pLoop->m_xFrontHangPtList.GetFirst(); pHangPt; pHangPt = pLoop->m_xFrontHangPtList.GetNext())
			{
				HANG_POINT* pNewHangPt = pNewLoop->m_xFrontHangPtList.append();
				pNewHangPt->CopyMember(pHangPt);
			}
		}
	}
	//对地线挂点进行排序(线路走向是从后向前，Y值是从负到正)
	for (CEarthLoop* pLoop = m_hashEarthLoop.GetFirst(); pLoop; pLoop = m_hashEarthLoop.GetNext())
	{
		CQuickSort<HANG_POINT>::QuickSort(pLoop->m_xBackHangPtList.m_pData, pLoop->m_xBackHangPtList.GetSize(), comparefun);
		if (pLoop->m_xFrontHangPtList.GetSize() > 0)
			CQuickSort<HANG_POINT>::QuickSort(pLoop->m_xFrontHangPtList.m_pData, pLoop->m_xFrontHangPtList.GetSize(), comparefun);
	}
	for(CEarthLoop* pLoop=m_hashEarthLoop.GetFirst();pLoop;pLoop=m_hashEarthLoop.GetNext())
	{
		int iCode=1;
		for(HANG_POINT* pHangPt=pLoop->m_xBackHangPtList.GetFirst();pHangPt;pHangPt=pLoop->m_xBackHangPtList.GetNext())
		{
			if(pHangPt->m_ciTensionType==0)
				sprintf(pHangPt->m_sHangName,"G%dX%d",pLoop->m_dwLoop,iCode);
			else
				sprintf(pHangPt->m_sHangName,"G%dN%d",pLoop->m_dwLoop,iCode);
			sprintf(pHangPt->m_sHangOrder,"%d",iCode);
			iCode++;
		}
		for (HANG_POINT* pHangPt = pLoop->m_xFrontHangPtList.GetFirst(); pHangPt; pHangPt = pLoop->m_xFrontHangPtList.GetNext())
		{
			if (pHangPt->m_ciTensionType == 0)
				sprintf(pHangPt->m_sHangName, "G%dX%d", pLoop->m_dwLoop, iCode);
			else
				sprintf(pHangPt->m_sHangName, "G%dN%d", pLoop->m_dwLoop, iCode);
			sprintf(pHangPt->m_sHangOrder, "%d", iCode);
			iCode++;
		}
	}
}
//杆塔的整体挂线属性，0.悬垂|1.耐张
BYTE CXmlModel::get_ciHangingType()
{
	CTowerLoop* pLoop=m_hashTowerLoop.GetFirst();
	if(pLoop==NULL)
		return 0xff;
	CHangPhase* pPhase=pLoop->m_hashHangPhase.GetFirst();
	if(pPhase==NULL)
		return 0xff;
	//目前逻辑如果是悬垂串，挂点放在m_xBackHangPtArr中，而m_xFrontHangPtArr为空，故只需根据后侧挂点即可统计出悬垂还是耐张 wjh-2019.4.25
	HANG_POINT* pWirePoint;
	UINT uiXuanChuiCount=0,uiNaiZhangCount=0;
	for(pWirePoint=pPhase->m_xBackHangPtArr.GetFirst();pWirePoint;pWirePoint=pPhase->m_xBackHangPtArr.GetNext())
	{
		if(pWirePoint->m_ciTensionType==0)
			uiXuanChuiCount++;
		else if(pWirePoint->m_ciTensionType==1)
			uiNaiZhangCount++;
	}
	if(uiXuanChuiCount>uiNaiZhangCount)
		return 0;	//整塔判定为悬垂塔
	else if(uiXuanChuiCount<uiNaiZhangCount)
		return 1;	//整塔判定为耐张塔
	else
		return 0xff;//数据出错
}
void CXmlModel::CreateOldXmlFile(const char* sFilePath)
{
	CMarkup xml;
	//xml.SetDoc("<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n");
	xml.AddElem("Xml");
	xml.AddAttrib("Category",(char*)m_sCategory);
	xml.AddAttrib("Model",m_ciModel);
	xml.AddAttrib("File",(char*)m_sTowerFile);
	if(this->ciHangingType==0)
		xml.AddAttrib("Type","悬垂常规");
	else
		xml.AddAttrib("Type","耐张常规");
	xml.AddAttrib("Name",(char*)m_sTowerName);
	xml.AddAttrib("Loop",m_hashTowerLoop.GetNodeNum());
	//添加LoopRoot信息节间
	xml.IntoElem();
	xml.AddElem("TowerLoopRoot");
	for(CTowerLoop* pLoop=m_hashTowerLoop.GetFirst();pLoop;pLoop=m_hashTowerLoop.GetNext())
		AddWireLoop(xml,pLoop);
	for(CEarthLoop* pLoop=m_hashEarthLoop.GetFirst();pLoop;pLoop=m_hashEarthLoop.GetNext())
		AddEarthLoop(xml,pLoop);
	xml.OutOfElem();
	//添加PropertyRoot信息节间
	xml.IntoElem();
	xml.AddElem("PropertyRoot");
		xml.IntoElem();
		xml.AddElem("Property");
		xml.AddAttrib("PropertyName",(char*)m_sPropName);
		xml.AddAttrib("PropertyValue",(char*)m_sPropValue);
		xml.OutOfElem();
	xml.OutOfElem();
	//添加FundationRoot信息节间
	xml.IntoElem();
	xml.AddElem("FundationRoot");
	for(int i=0;i<4;i++)
	{
		GEPOINT pos=m_xFundation[i].m_xFeetPos;
		if(m_bTransPtMMtoM)
			pos*=0.001;
		xml.IntoElem();
		xml.AddElem("FeetPoint");
		xml.AddAttrib("FeetName",m_xFundation[i].m_sFeetName);
		xml.AddAttrib("Position",(char*)CXhChar100("(%.6f,%.6f,%.6f)",pos.x,pos.y,pos.z));
		xml.OutOfElem();
	}
	xml.OutOfElem();
	//将编码格式ANSI转化为UNICODE
	CString strvalue=xml.GetDoc();
	int nLen=MultiByteToWideChar(CP_ACP,0,strvalue.GetBuffer(),-1,NULL,0 );
	wchar_t* sResult=(wchar_t*)malloc((nLen+1)*sizeof(wchar_t));
	memset(sResult,0,sizeof(wchar_t)*(nLen+1));
	MultiByteToWideChar(CP_ACP,0,strvalue.GetBuffer(),-1,(LPWSTR)sResult,nLen);
	//
	WORD wSignature = 0xFEFF;
	FILE* fp=fopen(sFilePath,"wb");
	if(fp)
	{
		fwrite(&wSignature,2,1,fp);	//UNICODE编码的头
		fwrite(sResult,(nLen+1)*sizeof(wchar_t),1,fp);
		fclose(fp);
	}
	free(sResult);
}
void CXmlModel::AddEarthLoop(CMarkup& xml,CEarthLoop* pEarthLoop)
{
	xml.IntoElem();
	xml.AddElem("TowerLoop");
	xml.AddAttrib("LoopTag",(char*)CXhChar50("地线%d",pEarthLoop->m_dwLoop));
	HANG_POINT xHangNode;
	if(pEarthLoop->GetBKVirtualHangPt(xHangNode))
		AddHangPoint(xml, &xHangNode,&(pEarthLoop->m_xBackHangPtList));
	else if(pEarthLoop->m_xBackHangPtList.GetSize()==1)
		AddHangPoint(xml,pEarthLoop->m_xBackHangPtList.GetByIndex(0));
	//
	if(pEarthLoop->GetFTVirtualHangPt(xHangNode))
		AddHangPoint(xml, &xHangNode, &(pEarthLoop->m_xFrontHangPtList));
	else if(pEarthLoop->m_xFrontHangPtList.GetSize()==1)
		AddHangPoint(xml, pEarthLoop->m_xFrontHangPtList.GetByIndex(0));
	xml.OutOfElem();
}
void CXmlModel::AddWireLoop(CMarkup& xml,CTowerLoop* pWireLoop)
{
	xml.IntoElem();
	xml.AddElem("TowerLoop");
	xml.AddAttrib("Current",(char*)pWireLoop->m_sCurrent);
	xml.AddAttrib("LoopTag",(char*)CXhChar50("回路%d",pWireLoop->m_dwLoop));
	xml.AddAttrib("LoopCode","00");
	for(CHangPhase* pHangPhase=pWireLoop->m_hashHangPhase.GetFirst();pHangPhase;pHangPhase=pWireLoop->m_hashHangPhase.GetNext())
	{
		xml.IntoElem();
		xml.AddElem("Phase");
		xml.AddAttrib("HangPhase",(char*)CXhChar50("相%d",pHangPhase->m_dwHangPhase));
		HANG_POINT xHangNode;
		if(pHangPhase->GetBKVirtualHangPt(xHangNode))
			AddHangPoint(xml,&xHangNode,&(pHangPhase->m_xBackHangPtArr));
		else if(pHangPhase->m_xBackHangPtArr.GetSize()==1)
			AddHangPoint(xml,pHangPhase->m_xBackHangPtArr.GetByIndex(0));
		// 
		if(pHangPhase->GetWJVirtualHangPt(xHangNode))
			AddHangPoint(xml,&xHangNode,&(pHangPhase->m_xWireJHangPtArr));
		else if(pHangPhase->m_xWireJHangPtArr.GetSize()==1)
			AddHangPoint(xml,pHangPhase->m_xWireJHangPtArr.GetByIndex(0));
		//
		if(pHangPhase->GetFTVirtualHangPt(xHangNode))
			AddHangPoint(xml,&xHangNode,&(pHangPhase->m_xFrontHangPtArr));
		else if(pHangPhase->m_xFrontHangPtArr.GetSize()==1)
			AddHangPoint(xml,pHangPhase->m_xFrontHangPtArr.GetByIndex(0));
		xml.OutOfElem();
	}
	xml.OutOfElem();
}
void CXmlModel::AddHangPoint(CMarkup& xml,HANG_POINT* pHangPt,ARRAY_LIST<HANG_POINT>* pHangPtArr/*=NULL*/)
{
	GEPOINT pos=pHangPt->m_xHangPos;
	if(m_bTransPtMMtoM)
		pos*=0.001;
	xml.IntoElem();
	xml.AddElem("TowerHangPoint");
	xml.AddAttrib("HangName",(char*)pHangPt->m_sHangName);
	if(pHangPt->m_ciWireType=='J')
		xml.AddAttrib("HangType","跳线");
	else if(pHangPt->m_ciTensionType==0)
		xml.AddAttrib("HangType","悬垂");
	else
		xml.AddAttrib("HangType","耐张");
	if(pHangPt->m_ciHangingStyle=='S'||pHangPt->m_ciHangingStyle=='V')
		xml.AddAttrib("HangPresen","多挂点");
	else
		xml.AddAttrib("HangPresen","单挂点");
	if(pHangPt->m_ciHangDirect=='Q')
		xml.AddAttrib("HangDirect","前外");
	else if(pHangPt->m_ciHangDirect=='H')
		xml.AddAttrib("HangDirect","后外");
	else
		xml.AddAttrib("HangDirect","");
	xml.AddAttrib("HangOrder",pHangPt->m_sHangOrder);
	xml.AddAttrib("Position",(char*)CXhChar100("(%.6f,%.6f,%.6f)",pos.x,pos.y,pos.z));
	if(pHangPtArr)
	{
		for(int i=0;i<pHangPtArr->GetSize();i++)
			AddHangPoint(xml,pHangPtArr->GetByIndex(i));
	}
	xml.OutOfElem();
}
