// TowerDataDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "TPSM.h"
#include "WireDataDlg.h"
#include "PropertyListOper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////////
//回调函数处理
static void FocusToHangNode(CXhWirePoint* pHangNode)
{
	SCOPE_STRU scope;
	scope.VerifyVertex(pHangNode->xPosition);
	scope.fMinX-=600;
	scope.fMaxX+=600;
	scope.fMinY-=600;
	scope.fMaxY+=600;
	scope.fMinZ-=600;
	scope.fMaxZ+=600;
	CTPSMView* pView=theApp.GetActiveView();
	if(pView)
	{
		double fFocusZoomCoef=1.0/pView->SolidOper()->GetScaleUserToScreen();
		double fZoomCoef=max(fFocusZoomCoef,0.05);
		pView->SolidOper()->FocusTo(scope,fZoomCoef);
		pView->SolidDraw()->Draw();
	}
}
static BOOL FireValueModify(CSuperGridCtrl* pListCtrl,CSuperGridCtrl::CTreeItem* pSelItem,
								int iSubItem,CString& sTextValue)
{
	CXhWirePoint* pHangNode=pSelItem?(CXhWirePoint*)pSelItem->m_idProp:NULL;
	if(pHangNode==NULL)
		return FALSE;
	GEPOINT pos = pHangNode->xPosition;
	if (iSubItem == 1)
		pHangNode->m_sGimName.Copy(sTextValue);
	else if(iSubItem==2)
	{
		double fValue=atof(sTextValue);
		pos.x=fValue*1000;
	}
	else if(iSubItem==3)
	{
		double fValue=atof(sTextValue);
		pos.y=fValue*1000;
	}
	else if(iSubItem==4)
	{
		double fValue=atof(sTextValue);
		pos.z=fValue*1000;
	}
	pHangNode->set_xPosition(pos);
	CTPSMView *pView=theApp.GetActiveView();
	if(pView)
	{
		pView->Refresh();
		FocusToHangNode(pHangNode);
	}
	return TRUE;
}
static BOOL FireItemChanged(CSuperGridCtrl* pListCtrl,CSuperGridCtrl::CTreeItem* pSelItem,
								NM_LISTVIEW* pNMListView)
{
	CXhWirePoint* pHangNode=pSelItem?(CXhWirePoint*)pSelItem->m_idProp:NULL;
	if(pHangNode==NULL)
		return FALSE;
	FocusToHangNode(pHangNode);
	return TRUE;
}
//////////////////////////////////////////////////////////////////////////
// CTowerDataDlg 对话框
IMPLEMENT_DYNCREATE(CWireDataDlg, CDialogEx)
CWireDataDlg::CWireDataDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CWireDataDlg::IDD, pParent)
{

}

CWireDataDlg::~CWireDataDlg()
{
}

void CWireDataDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_DATA_OPENING, m_xDataListCtrl);
}


BEGIN_MESSAGE_MAP(CWireDataDlg, CDialogEx)
	ON_WM_SIZE()
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
// CTowerDataDlg 消息处理程序
BOOL CWireDataDlg::OnInitDialog() 
{
	CDialogEx::OnInitDialog();
	//
	long col_wide_arr[5]={45,80,60,60,60};
	m_xDataListCtrl.AddColumnHeader("..");
	m_xDataListCtrl.AddColumnHeader("挂点名称");
	m_xDataListCtrl.AddColumnHeader("X坐标");
	m_xDataListCtrl.AddColumnHeader("Y坐标");
	m_xDataListCtrl.AddColumnHeader("Z坐标");
	m_xDataListCtrl.InitListCtrl(col_wide_arr);
	m_xDataListCtrl.SetModifyValueFunc(FireValueModify);
	m_xDataListCtrl.SetItemChangedFunc(FireItemChanged);
	return TRUE; 
}
CSuperGridCtrl::CTreeItem* CWireDataDlg::AddItem(CXhWirePoint* pWirePt,CSuperGridCtrl::CTreeItem* pGroupItem)
{
	if (pWirePt == NULL || pGroupItem == NULL)
		return NULL;
	CXhChar16 sValue;
	CListCtrlItemInfo *lpInfo=new CListCtrlItemInfo();
	//挂点名称
	lpInfo->SetSubItemText(1, pWirePt->m_sGimName,FALSE);
	//X
	sValue.Printf("%g",pWirePt->xPosition.x*0.001);
	SimplifiedNumString(sValue);
	lpInfo->SetSubItemText(2,(char*)sValue,FALSE);
	//Y
	sValue.Printf("%g", pWirePt->xPosition.y*0.001);
	SimplifiedNumString(sValue);
	lpInfo->SetSubItemText(3,(char*)sValue,FALSE);
	//Z
	sValue.Printf("%f", pWirePt->xPosition.z*0.001);
	SimplifiedNumString(sValue);
	lpInfo->SetSubItemText(4,(char*)sValue,FALSE);
	//
	CSuperGridCtrl::CTreeItem *pItem=m_xDataListCtrl.InsertItem(pGroupItem,lpInfo);
	pItem->m_idProp = (long)pWirePt;
	return pItem;
}
void CWireDataDlg::UpdateWireNodeList(CTPSMModel* pActiveModel /*= NULL*/)
{
	CTPSMModel* pModel = pActiveModel;
	if(pModel==NULL)
		pModel=(Manager.m_pActivePrj) ? Manager.m_pActivePrj->m_pActiveModel : NULL;
	if(pModel==NULL)
		return;
	m_xDataListCtrl.DeleteAllItems();
	int nPhaseNum = 0;
	//添加地线挂点分组
	CSuperGridCtrl::CTreeItem *pGroupItem = NULL;
	CListCtrlItemInfo *lpInfo = new CListCtrlItemInfo();
	lpInfo->SetSubItemText(0, "地线", TRUE);
	pGroupItem = m_xDataListCtrl.InsertRootItem(lpInfo);
	CXhWirePoint* pWirePoint = NULL;
	for (pWirePoint = pModel->m_xWirePointModel.m_hashWirePoints.GetFirst(); pWirePoint; pWirePoint = pModel->m_xWirePointModel.m_hashWirePoints.GetNext())
	{
		if (pWirePoint->m_xWirePlace.ciWireType != 'E')
		{
			nPhaseNum = max(nPhaseNum, pWirePoint->m_xWirePlace.iSerial);
			continue;
		}
		AddItem(pWirePoint, pGroupItem);
	}
	//添加导线挂点分组
	for (int iSerial = 1; iSerial <= nPhaseNum; iSerial++)
	{
		lpInfo = new CListCtrlItemInfo();
		lpInfo->SetSubItemText(0, CXhChar16("相%d",iSerial), TRUE);
		pGroupItem = m_xDataListCtrl.InsertRootItem(lpInfo);
		//
		for (pWirePoint = pModel->m_xWirePointModel.m_hashWirePoints.GetFirst(); pWirePoint; pWirePoint = pModel->m_xWirePointModel.m_hashWirePoints.GetNext())
		{
			if(pWirePoint->m_xWirePlace.ciWireType=='E')
				continue;
			if(pWirePoint->m_xWirePlace.iSerial!=iSerial)
				continue;
			AddItem(pWirePoint, pGroupItem);
		}
	}
	m_xDataListCtrl.Redraw();
	UpdateData(FALSE);
}

void CWireDataDlg::OnOK() 
{
}

void CWireDataDlg::OnCancel() 
{
}

void CWireDataDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialogEx::OnSize(nType, cx, cy);
	//
	if(m_xDataListCtrl.GetSafeHwnd())
		m_xDataListCtrl.MoveWindow(0,0,cx,cy);
}
