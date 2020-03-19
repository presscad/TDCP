// CallBackDialog.cpp : implementation file
//<LOCALE_TRANSLATE BY wbt />

#include "stdafx.h"
#include "TPSM.h"
#include "CallBackDialog.h"
#include "MainFrm.h"
#include "CmdLineDlg.h"
#include "PropertyList.h"
#include "SnapTypeVerify.h"
#include "Console.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCallBackDialog dialog


CCallBackDialog::CCallBackDialog(UINT nIDTemplate,CWnd* pParent /*=NULL*/)
	: CDialog(nIDTemplate, pParent)
{
	
	//{{AFX_DATA_INIT(CCallBackDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_iPickObjType=CLS_PART;
	m_hPickObj=0;
	m_bInernalStart=FALSE;
	m_dwSelectPartsType=0;
	m_bPauseBreakExit=FALSE;
	m_bOpenWndSel=FALSE;
	m_bFireListItemChange=TRUE;
	m_bModelessDlg=FALSE;
	m_idEventProp=0;
	m_iBreakType=0;
	FireCallBackFunc=NULL;
	m_pBlock=NULL;
}


void CCallBackDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCallBackDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCallBackDialog, CDialog)
	//{{AFX_MSG_MAP(CCallBackDialog)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCallBackDialog message handlers
void CCallBackDialog::ExecuteCommand(CWnd* pCallWnd,UINT message/*=0*/,WPARAM wParam/*=0*/)
{
	m_bPauseBreakExit=TRUE;
	m_bInernalStart=TRUE;
	m_iBreakType=1;
	m_pCallWnd=pCallWnd;
	m_message=message;
	m_wParam=wParam;
	if(m_bModelessDlg)
	{
		ShowWindow(SW_HIDE);	//隐藏非模态对话框
		if(m_bPauseBreakExit)
			PickObject();		//拾取构件
		ShowModelessDlg();		//显示非模态对话框
	}
	else	
		CDialog::OnOK();		//关闭模态对话框
}
void CCallBackDialog::SelectObject(int cls_id/*=0*/,int idEventProp/*=0*/) 
{
	m_bPauseBreakExit=TRUE;
	m_bInernalStart=TRUE;
	m_iPickObjType=cls_id;
	m_idEventProp=idEventProp;
	m_iBreakType=0;
	if(cls_id==CLS_NODE)
	{
		m_dwSelectPartsType=GetSingleWord(SELECTINDEX_NODE);
#ifdef AFX_TARG_ENU_ENGLISH
		m_sCmdPickPrompt="Please select datum node<Press enter key to ensure>:";
#else 
		m_sCmdPickPrompt="请选择基准节点<回车确认>:";
#endif
	}
	else if(cls_id==CLS_PART)
	{
		m_dwSelectPartsType|=GetSingleWord(SELECTINDEX_PLATE);
		m_dwSelectPartsType|=GetSingleWord(SELECTINDEX_PARAMPLATE);
		m_dwSelectPartsType|=GetSingleWord(SELECTINDEX_LINEANGLE);
		m_dwSelectPartsType|=GetSingleWord(SELECTINDEX_LINETUBE);
		m_dwSelectPartsType|=GetSingleWord(SELECTINDEX_LINESLOT);
		m_dwSelectPartsType|=GetSingleWord(SELECTINDEX_LINEFLAT);
		m_dwSelectPartsType|=GetSingleWord(SELECTINDEX_STDPART);
		m_dwSelectPartsType|=GetSingleWord(SELECTINDEX_PARAMPLATE);
		m_dwSelectPartsType|=GetSingleWord(SELECTINDEX_PARAMPLATE);
#ifdef AFX_TARG_ENU_ENGLISH
		m_sCmdPickPrompt="Please select datum part<Press enter key to ensure>:";
#else 
		m_sCmdPickPrompt="请选择基准构件<回车确认>:";
#endif
	}
	else if(cls_id==CLS_BOLT)
	{
		m_dwSelectPartsType=GetSingleWord(SELECTINDEX_BOLT);
#ifdef AFX_TARG_ENU_ENGLISH
		if(m_bOpenWndSel)
			m_sCmdPickPrompt="Please select bolt<Press enter key to ensure>:";
		else 
			m_sCmdPickPrompt="Please select one bolt<Press enter key to ensure>:";
#else 
		if(m_bOpenWndSel)
			m_sCmdPickPrompt="请选择螺栓<回车确认>:";
		else 
			m_sCmdPickPrompt="请选择一颗螺栓<回车确认>:";
#endif
	}
	else if(cls_id==CLS_PLATE||cls_id==CLS_PARAMPLATE)
	{
		m_dwSelectPartsType=GetSingleWord(SELECTINDEX_PARAMPLATE);
#ifdef AFX_TARG_ENU_ENGLISH
		if(m_bOpenWndSel)
			m_sCmdPickPrompt="Please select plate<Press enter key to ensure>:";
		else 
			m_sCmdPickPrompt="Please select one plate<Press enter key to ensure>:";
#else 
		if(m_bOpenWndSel)
			m_sCmdPickPrompt="请选择钢板<回车确认>:";
		else 
			m_sCmdPickPrompt="请选择一块钢板<回车确认>:";
#endif
	}
	else if(cls_id==CLS_LINEANGLE)
	{
		m_dwSelectPartsType=GetSingleWord(SELECTINDEX_LINEANGLE);
#ifdef AFX_TARG_ENU_ENGLISH
		if(m_bOpenWndSel)
			m_sCmdPickPrompt="Please select angle<Press enter key to ensure>:";
		else 
			m_sCmdPickPrompt="Please select one angle<Press enter key to ensure>:";
#else 
		if(m_bOpenWndSel)
			m_sCmdPickPrompt="请选择角钢<回车确认>:";
		else 
			m_sCmdPickPrompt="请选择一根角钢<回车确认>:";
#endif
	}
	else if(cls_id==CLS_LINETUBE)
	{
		m_dwSelectPartsType=GetSingleWord(SELECTINDEX_LINETUBE);
#ifdef AFX_TARG_ENU_ENGLISH
		if(m_bOpenWndSel)
			m_sCmdPickPrompt="Please select tube<Press enter key to ensure>:";
		else
			m_sCmdPickPrompt="Please select one tube<Press enter key to ensure>:";
#else 
		if(m_bOpenWndSel)
			m_sCmdPickPrompt="请选择钢管<回车确认>:";
		else
			m_sCmdPickPrompt="请选择一根钢管<回车确认>:";
#endif
	}
	else if(cls_id==CLS_LINESLOT)
	{
		m_dwSelectPartsType=GetSingleWord(SELECTINDEX_LINESLOT);
#ifdef AFX_TARG_ENU_ENGLISH
		if(m_bOpenWndSel)
			m_sCmdPickPrompt="Please select U-steel<Press enter key to ensure>:";
		else 
			m_sCmdPickPrompt="Please select one U-steel<Press enter key to ensure>:";
#else 
		if(m_bOpenWndSel)
			m_sCmdPickPrompt="请选择槽钢<回车确认>:";
		else 
			m_sCmdPickPrompt="请选择一根槽钢<回车确认>:";
#endif
	}
	else if(cls_id==CLS_LINEFLAT)
	{
		m_dwSelectPartsType=GetSingleWord(SELECTINDEX_LINEFLAT);
#ifdef AFX_TARG_ENU_ENGLISH
		if(m_bOpenWndSel)
			m_sCmdPickPrompt="Please select flat steel<Press enter key to ensure>:";
		else
			m_sCmdPickPrompt="Please select one flat steel<Press enter key to ensure>:";
#else 
		if(m_bOpenWndSel)
			m_sCmdPickPrompt="请选择扁钢<回车确认>:";
		else
			m_sCmdPickPrompt="请选择一根扁钢<回车确认>:";
#endif
	}
	else if(cls_id==CLS_LINEPART)
	{
		m_dwSelectPartsType|=GetSingleWord(SELECTINDEX_LINEANGLE);
		m_dwSelectPartsType|=GetSingleWord(SELECTINDEX_LINETUBE);
		m_dwSelectPartsType|=GetSingleWord(SELECTINDEX_LINEFLAT);
		m_dwSelectPartsType|=GetSingleWord(SELECTINDEX_LINESLOT);
#ifdef AFX_TARG_ENU_ENGLISH
		if(m_bOpenWndSel)
			m_sCmdPickPrompt="Please select rod<Press enter key to ensure>:";
		else 
			m_sCmdPickPrompt="Please select one rod<Press enter key to ensure>:";
#else 
		if(m_bOpenWndSel)
			m_sCmdPickPrompt="请选择杆件<回车确认>:";
		else 
			m_sCmdPickPrompt="请选择一根杆件<回车确认>:";
#endif
	}
	else if(cls_id==CLS_ARCFLAT||cls_id==CLS_ARCSLOT||cls_id==CLS_ARCTUBE
		||cls_id==CLS_ARCANGLE||cls_id==CLS_ARCPART)
	{
		m_dwSelectPartsType=GetSingleWord(SELECTINDEX_ARCANGLE|SELECTINDEX_ARCFLAT|SELECTINDEX_ARCSLOT);
#ifdef AFX_TARG_ENU_ENGLISH
		if(m_bOpenWndSel)
			m_sCmdPickPrompt="Please select circular part<Press enter key to ensure>:";
		else 
			m_sCmdPickPrompt="Please select one circular part<Press enter key to ensure>:";
#else 
		if(m_bOpenWndSel)
			m_sCmdPickPrompt="请选择环形杆件<回车确认>:";
		else 
			m_sCmdPickPrompt="请选择一根环形杆件<回车确认>:";
#endif
	}
	else if(cls_id==CLS_SPHERE)
	{
		m_dwSelectPartsType=GetSingleWord(SELECTINDEX_SPHERE);
#ifdef AFX_TARG_ENU_ENGLISH
		if(m_bOpenWndSel)
			m_sCmdPickPrompt="Please select sphere<Press enter key to ensure>:";
		else 
			m_sCmdPickPrompt="Please select one sphere<Press enter key to ensure>:";
#else 
		if(m_bOpenWndSel)
			m_sCmdPickPrompt="请选择球体<回车确认>:";
		else 
			m_sCmdPickPrompt="请选择一个球体<回车确认>:";
#endif
	}
	else 
	{
		m_dwSelectPartsType=GetSingleWord(SELECT_ALL);
#ifdef AFX_TARG_ENU_ENGLISH
		m_sCmdPickPrompt="Please select datum part<Press enter key to ensure>:";
#else 
		m_sCmdPickPrompt="请选择基准构件<回车确认>:";
#endif
	}
	if(m_bModelessDlg)
	{
		ShowWindow(SW_HIDE);	//隐藏非模态对话框
		if(m_bPauseBreakExit)
			PickObject();		//拾取构件
		ShowModelessDlg();		//显示非模态对话框
	}
	else	
		CDialog::OnOK();		//关闭模态对话框
}

void CCallBackDialog::PickObject()
{
	CTPSMView* pView=theApp.GetActiveView();
	if(pView==NULL)
		return;
	CString cmdStr;
	m_hPickObj=0;
	CCmdLineDlg *pCmdLine = ((CMainFrame*)AfxGetMainWnd())->GetCmdLinePage();
	pCmdLine->FillCmdLine(m_sCmdPickPrompt,"");
	while(1)
	{
		DWORD dwOldSelectTypeFlag=pView->SolidSnap()->SetSelectPartsType(m_dwSelectPartsType);
		if(!(m_bOpenWndSel&&m_dwSelectPartsType==GetSingleWord(SELECTINDEX_BOLT)))
			pView->SolidDraw()->ReleaseSnapStatus();	//框选螺栓时不清空当前选中的构件 wht 11-05-19
		if(m_bOpenWndSel)
		{	//开窗选择构件
			pView->SolidSet()->SetCreateEnt(FALSE);
			pView->SolidSet()->SetDrawType(DRAW_RECTANGLE);
			if(pCmdLine->GetStrFromCmdLine(cmdStr))
			{	//等待框选构件,选择完成后当前选中构件即为所需构件,故未另行设置变量存储 wht 11-05-16
				;
			}
			if(pView->SolidSnap())
				pView->SolidSnap()->SetSnapType(SNAP_ALL);
			pView->SolidSet()->SetOperType(OPER_OTHER);
			m_bOpenWndSel=FALSE;
		}
		else 
		{	//点选构件
			if(m_iPickObjType==CLS_NODE)
			{	//切换到单线显示状态便于显示节点
				DWORD dwPickObj=0,dwExportFlag=0;
				CSnapTypeVerify verifier(OBJPROVIDER::SOLIDSPACE,GetSingleWord(SELECTINDEX_NODE));
				verifier.AddVerifyType(OBJPROVIDER::LINESPACE,AtomType::prPoint);
				//CDisplayNodeAtFrontLife displayNode;
				//displayNode.DisplayNodeAtFront();
				while(true)
				{
					if(pView->SolidSnap()->SnapObject(&dwPickObj,&dwExportFlag,&verifier)<0)
					{
						pCmdLine->CancelCmdLine();
						break;;
					}
					SELOBJ obj(dwPickObj,dwExportFlag);
					CLDSNode *pNode= console.FromNodeHandle(obj.hRelaObj);
					if(pNode)
					{
						m_hPickObj=pNode->handle;
						break;
					}
				}
				pView->SolidDraw()->SetEntSnapStatus(m_hPickObj,TRUE);
			}
			else
			{	
				//根据不同的选取类型，设置相应的CSnapTypeVerify.m_dwSolidSpaceFlagv wh-2016.10.9
				CSnapTypeVerify verifier(OBJPROVIDER::SOLIDSPACE,SELECT_ALL);
				verifier.AddVerifyType(OBJPROVIDER::LINESPACE,AtomType::prPoint);
				verifier.AddVerifyType(OBJPROVIDER::LINESPACE,AtomType::prLine);
				CString cmdValue;
				long* id_arr=NULL,retcode=0;
				DWORD hPickObj=0,dwExportFlag=0;
				CLDSPart *pPart=NULL;
				while(true)
				{
					if((retcode=pView->SolidSnap()->SnapObject(&hPickObj,&dwExportFlag,&verifier))<0)
					{
						pCmdLine->CancelCmdLine();
						break;
					}
					SELOBJ obj(hPickObj,dwExportFlag);
					m_hPickObj=obj.hRelaObj;
					if(obj.provider==OBJPROVIDER::SOLIDSPACE)
						m_hPickObj=obj.hObj;
					if(m_hPickObj==0&&pView->SolidSnap()->GetLastSelectEnts(id_arr)>0)
						m_hPickObj=id_arr[0];
					else if(m_hPickObj==0&&cmdValue.GetLength()>0)
						m_hPickObj=HexStrToLong(cmdValue);
					pPart=console.FromPartHandle(m_hPickObj);
					if(pPart!=NULL || obj.ciTriggerType==SELOBJ::TRIGGER_KEYRETURN)
					{
						if(pPart!=NULL)
							pView->SolidDraw()->SetEntSnapStatus(pPart->handle);
						break;
					}
				}
				pView->SolidDraw()->SetEntSnapStatus(m_hPickObj,TRUE);
			}
		}
		if(pView->SolidSnap())
			pView->SolidSnap()->SetSelectPartsType(dwOldSelectTypeFlag);

		if(m_pBlock&&!m_pBlock->IsEmbeded()&&m_hPickObj>0x20&&console.GetActiveModel()!=m_pBlock)
		{	//导入式部件，不允许使用部件空间以外的构件 wht 12-10-18
#ifdef AFX_TARG_ENU_ENGLISH
			pCmdLine->FillCmdLine("Command:","The selected parts isn't the part of block's inner parts.Please select again!");
#else 
			pCmdLine->FillCmdLine("命令:","选中的构件不是部件内构件，请重新选择!");
#endif
			pCmdLine->FinishCmdLine();
			m_hPickObj=0;
			continue;
		}
		else 
		{
			m_bInernalStart=TRUE;
			m_bPauseBreakExit=FALSE;
#ifdef AFX_TARG_ENU_ENGLISH
			pCmdLine->FillCmdLine("Command:","");
#else 
			pCmdLine->FillCmdLine("命令:","");
#endif
			pCmdLine->FinishCmdLine();
			break;
		}
	}
}

BOOL CCallBackDialog::ShowModelessDlg()
{
	m_bModelessDlg=TRUE;	//非模态对话框
	BOOL bRetCode=ShowWindow(SW_SHOW);
	FinishSelectObjOper();
	m_bInernalStart=FALSE;
	return bRetCode;
}

int CCallBackDialog::DoModal() 
{
	m_bModelessDlg=FALSE;	//模态对话框
	CPropertyList *pPropList=((CMainFrame*)AfxGetMainWnd())->GetPropertyPage()->GetPropertyList();
	m_bInernalStart=FALSE;
	int ret=-1;
	while(1)
	{
		{	//放到同一个生命周期，避免对话框出现异常时导致属性栏锁定 wxc-2016.12.27
			CLockPropertyList lockProperty(pPropList);
			ret=CDialog::DoModal();
		}
		if(m_bPauseBreakExit)
		{
			if(m_iBreakType==1)
			{
				if(FireCallBackFunc)
					FireCallBackFunc(m_pCallWnd,m_message,m_wParam);
				m_bInernalStart=TRUE;
				m_bPauseBreakExit=FALSE;
			}
			else 
				PickObject();	//拾取构件
		}
		else
			break;
	}
	return ret;
}
