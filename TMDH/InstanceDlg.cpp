// InstanceDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "TMDH.h"
#include "InstanceDlg.h"
#include "afxdialogex.h"


// CInstanceDlg 对话框

IMPLEMENT_DYNAMIC(CInstanceDlg, CDialogEx)

CInstanceDlg::CInstanceDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CInstanceDlg::IDD, pParent)
	, m_sHeightName(_T(""))
	, m_sLegASerial(_T(""))
	, m_sLegBSerial(_T(""))
	, m_sLegCSerial(_T(""))
	, m_sLegDSerial(_T(""))
{
	m_ciModelFlag=0;
	m_pModel=NULL;
}

CInstanceDlg::~CInstanceDlg()
{
}

void CInstanceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_CBString(pDX, IDC_CMB_HUGAO, m_sHeightName);
	DDX_CBString(pDX, IDC_CMB_LEG1, m_sLegASerial);
	DDX_CBString(pDX, IDC_CMB_LEG2, m_sLegBSerial);
	DDX_CBString(pDX, IDC_CMB_LEG3, m_sLegCSerial);
	DDX_CBString(pDX, IDC_CMB_LEG4, m_sLegDSerial);
}


BEGIN_MESSAGE_MAP(CInstanceDlg, CDialogEx)
	ON_CBN_SELCHANGE(IDC_CMB_HUGAO, OnCbnSelchangeCmbHugao)
	ON_CBN_SELCHANGE(IDC_CMB_LEG1, OnCbnSelchangeCmbLegA)
	ON_CBN_SELCHANGE(IDC_CMB_LEG2, OnCbnSelchangeCmbLegB)
	ON_CBN_SELCHANGE(IDC_CMB_LEG3, OnCbnSelchangeCmbLegC)
	ON_CBN_SELCHANGE(IDC_CMB_LEG4, OnCbnSelchangeCmbLegD)
END_MESSAGE_MAP()


// CInstanceDlg 消息处理程序
BOOL CInstanceDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	//
	CComboBox* pCmbHugao=(CComboBox*)GetDlgItem(IDC_CMB_HUGAO);
	pCmbHugao->ResetContent();
	int iSelItem=0,i=0;
	if(m_ciModelFlag==0)
	{	//LDS
		CTMDHTower* pTower=(CTMDHTower*)m_pModel;
		for(CLDSModule *pModule=pTower->m_xTower.Module.GetFirst();pModule;pModule=pTower->m_xTower.Module.GetNext(),i++)
		{
			pCmbHugao->AddString(pModule->description);
			if(pModule==pTower->m_xTower.GetActiveModule())
				iSelItem=i;
		}
	}
	else
	{
		CTMDHModel* pModel=(CTMDHModel*)m_pModel;
		CXhChar50 sName;
		if(pModel->m_ciModelFlag==CTMDHModel::TYPE_TID)
		{	//TID
			ITidModel* pModel=Manager.m_pTidModel;
			for(int i=0;i<pModel->HeightGroupCount();i++)
			{
				ITidHeightGroup* pHuGao=pModel->GetHeightGroupAt(i);
				pHuGao->GetName(sName,50);
				pCmbHugao->AddString(sName);
			}
		}
		else if(pModel->m_ciModelFlag==CTMDHModel::TYPE_MOD)
		{	//MOD
			IModModel* pModel=Manager.m_pModModel;
			for(IModHeightGroup* pHuGao=pModel->EnumFirstHGroup();pHuGao;pHuGao=pModel->EnumNextHGroup())
			{
				pHuGao->GetName(sName);
				pCmbHugao->AddString(sName);
			}
		}
	}
	pCmbHugao->SetCurSel(iSelItem);
	OnCbnSelchangeCmbHugao();
	//
	UpdateData(FALSE);
	return TRUE;
}
void CInstanceDlg::OnCbnSelchangeCmbHugao()
{
	UpdateData(TRUE);
	m_hashSubLeg.Empty();
	if(m_ciModelFlag==0)
	{	//LDS
		CTMDHTower* pTower=(CTMDHTower*)m_pModel;
		CLDSModule *pModule=NULL;
		for(pModule=pTower->m_xTower.Module.GetFirst();pModule;pModule=pTower->m_xTower.Module.GetNext())
		{
			if(stricmp(pModule->description,m_sHeightName)==0)
				break;
		}
		if(pModule)
		{
			pTower->m_xTower.m_hActiveModule=pModule->handle;
			//接腿信息
			for(int i=1;i<=192;i++)
			{
				if(!pModule->m_dwLegCfgWord.IsHasNo(i))
					continue;
				m_hashSubLeg.SetValue(CXhChar16("%C",(i-1)%pTower->m_nMaxLegs+'A'),i);
			}
		}
	}
	else
	{
		CTMDHModel* pModel=(CTMDHModel*)m_pModel;
		if(pModel->m_ciModelFlag==CTMDHModel::TYPE_TID)
		{	//TID
			ITidHeightGroup* pHuGao=NULL;
			for(int i=0;i<Manager.m_pTidModel->HeightGroupCount();i++)
			{
				CXhChar50 sName;
				ITidHeightGroup* pTemHuGao=Manager.m_pTidModel->GetHeightGroupAt(i);
				if(pTemHuGao)
					pTemHuGao->GetName(sName,50);
				if(sName.EqualNoCase(m_sHeightName))
				{
					pHuGao=pTemHuGao;
					break;
				}
			}
			if(pHuGao)
			{
				pModel->m_uiHeightSerial=pHuGao->GetSerialId();
				//接腿信息
				ARRAY_LIST<int> legSerialArr;
				int nLeg=pHuGao->GetLegSerialArr(NULL);
				legSerialArr.SetSize(nLeg);
				pHuGao->GetLegSerialArr(legSerialArr.m_pData);
				for(int i=0;i<nLeg;i++)
				{
					double fValue=pHuGao->GetLegHeightDifference(legSerialArr[i]);
					m_hashSubLeg.SetValue(CXhChar16(fValue),legSerialArr[i]);
				}
			}
		}
		else if(pModel->m_ciModelFlag==CTMDHModel::TYPE_MOD)
		{	//MOD
			IModHeightGroup* pHuGao=Manager.m_pModModel->GetHeightGroup(m_sHeightName);
			if(pHuGao)
			{
				pModel->m_uiHeightSerial=pHuGao->GetNo();
				//接腿信息
				for(int *pLegI=pHuGao->EnumFirstLegSerial();pLegI;pLegI=pHuGao->EnumNextLegSerial())
				{
					double fValue=pHuGao->GetLegDiffDist(*pLegI);
					m_hashSubLeg.SetValue(CXhChar16(fValue),*pLegI);
				}
			}
		}
	}
	//更新配腿信息
	int nLegSize=m_hashSubLeg.GetNodeNum();
	if(nLegSize>0)
	{	
		CString sText;
		//A腿
		CComboBox* pCmbLegA=(CComboBox*)GetDlgItem(IDC_CMB_LEG1);
		pCmbLegA->ResetContent();
		for(int *pLegI=m_hashSubLeg.GetFirst();pLegI;pLegI=m_hashSubLeg.GetNext())
		{
			sText.Format("%s",m_hashSubLeg.GetCursorKey());
			pCmbLegA->AddString(sText);
		}
		pCmbLegA->GetLBText(0,m_sLegASerial);
		pCmbLegA->SetCurSel(0);
		//B腿
		CComboBox* pCmbLegB=(CComboBox*)GetDlgItem(IDC_CMB_LEG2);
		pCmbLegB->ResetContent();
		for(int *pLegI=m_hashSubLeg.GetFirst();pLegI;pLegI=m_hashSubLeg.GetNext())
		{
			sText.Format("%s",m_hashSubLeg.GetCursorKey());
			pCmbLegB->AddString(sText);
		}
		pCmbLegB->GetLBText(0,m_sLegBSerial);
		pCmbLegB->SetCurSel(0);
		//C腿
		CComboBox* pCmbLegC=(CComboBox*)GetDlgItem(IDC_CMB_LEG3);
		pCmbLegC->ResetContent();
		for(int *pLegI=m_hashSubLeg.GetFirst();pLegI;pLegI=m_hashSubLeg.GetNext())
		{
			sText.Format("%s",m_hashSubLeg.GetCursorKey());
			pCmbLegC->AddString(sText);
		}
		pCmbLegC->GetLBText(0,m_sLegCSerial);
		pCmbLegC->SetCurSel(0);
		//D腿
		CComboBox* pCmbLegD=(CComboBox*)GetDlgItem(IDC_CMB_LEG4);
		pCmbLegD->ResetContent();
		for(int *pLegI=m_hashSubLeg.GetFirst();pLegI;pLegI=m_hashSubLeg.GetNext())
		{
			sText.Format("%s",m_hashSubLeg.GetCursorKey());
			pCmbLegD->AddString(sText);
		}
		pCmbLegD->GetLBText(0,m_sLegDSerial);
		pCmbLegD->SetCurSel(0);
	}
	UpdateData(FALSE);
}
void CInstanceDlg::OnCbnSelchangeCmbLegA()
{
	UpdateData(TRUE);
	if(m_ciModelFlag==0)
	{
		CTMDHTower* pTower=(CTMDHTower*)m_pModel;
		CLDSModule* pActiveModule=pTower->m_xTower.GetActiveModule();
		if(pActiveModule)
			pActiveModule->m_arrActiveQuadLegNo[1]=*m_hashSubLeg.GetValue(m_sLegASerial);
	}
	else
	{
		CTMDHModel* pModel=(CTMDHModel*)m_pModel;
		pModel->m_uiLegSerialArr[1]=*m_hashSubLeg.GetValue(m_sLegASerial);
	}
	UpdateData(FALSE);
}
void CInstanceDlg::OnCbnSelchangeCmbLegB()
{
	UpdateData(TRUE);
	if(m_ciModelFlag==0)
	{
		CTMDHTower* pTower=(CTMDHTower*)m_pModel;
		CLDSModule* pActiveModule=pTower->m_xTower.GetActiveModule();
		if(pActiveModule)
			pActiveModule->m_arrActiveQuadLegNo[3]=*m_hashSubLeg.GetValue(m_sLegBSerial);
	}
	else
	{
		CTMDHModel* pModel=(CTMDHModel*)m_pModel;
		pModel->m_uiLegSerialArr[3]=*m_hashSubLeg.GetValue(m_sLegBSerial);
	}
	UpdateData(FALSE);
}
void CInstanceDlg::OnCbnSelchangeCmbLegC()
{
	UpdateData(TRUE);
	if(m_ciModelFlag==0)
	{
		CTMDHTower* pTower=(CTMDHTower*)m_pModel;
		CLDSModule* pActiveModule=pTower->m_xTower.GetActiveModule();
		if(pActiveModule)
			pActiveModule->m_arrActiveQuadLegNo[2]=*m_hashSubLeg.GetValue(m_sLegCSerial);
	}
	else
	{
		CTMDHModel* pModel=(CTMDHModel*)m_pModel;
		pModel->m_uiLegSerialArr[2]=*m_hashSubLeg.GetValue(m_sLegCSerial);
	}
	UpdateData(FALSE);
}
void CInstanceDlg::OnCbnSelchangeCmbLegD()
{
	UpdateData(TRUE);
	if(m_ciModelFlag==0)
	{
		CTMDHTower* pTower=(CTMDHTower*)m_pModel;
		CLDSModule* pActiveModule=pTower->m_xTower.GetActiveModule();
		if(pActiveModule)
			pActiveModule->m_arrActiveQuadLegNo[0]=*m_hashSubLeg.GetValue(m_sLegDSerial);
	}
	else
	{
		CTMDHModel* pModel=(CTMDHModel*)m_pModel;
		pModel->m_uiLegSerialArr[0]=*m_hashSubLeg.GetValue(m_sLegDSerial);
	}
	UpdateData(FALSE);
}