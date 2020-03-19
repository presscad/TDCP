// CmdLineDlg.cpp : implementation file
//<LOCALE_TRANSLATE BY wbt />

#include "stdafx.h"
#include "CmdLineDlg.h"
#include "UserDefMsg.h"
#include "XhCharString.h"
#include "MainFrm.h"
#include "TPSM.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern ::CEvent LButtonDown,RButtonDown,HaltProc;
/////////////////////////////////////////////////////////////////////////////
// CCmdLineDlg dialog
IMPLEMENT_DYNCREATE(CCmdLineDlg, CDialog)

CCmdLineDlg::CCmdLineDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCmdLineDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCmdLineDlg)
#ifdef AFX_TARG_ENU_ENGLISH
	m_sCmdPrompt = _T("Command:");
#else 
	m_sCmdPrompt = _T("命令:");
#endif
	m_sCmdHistory = _T("");
	m_sUserInputCmd = _T("");
	//}}AFX_DATA_INIT
	m_sPreCmd = _T("");
	m_nCmdLineHeight = 18;
	m_nCmdPromptWndWidth = 20;
	m_hMsgProcessWnd = NULL;
	m_bGettingStrFromCmdLine = m_bForceExitCommand = FALSE;
	m_ctrlCmdLabel.SetTextColor(RGB(0,0,0));
	m_ctrlCmdLabel.SetBackgroundColour(RGB(255,255,255));
	m_pMiniPanel=NULL;
}
void CCmdLineDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCmdLineDlg)
	DDX_Control(pDX, IDC_RCE_CMD_HISTORY, m_richCmdHistory);
	DDX_Text(pDX, IDC_RCE_CMD_PROMPT, m_sCmdPrompt);
	DDX_Text(pDX, IDC_RCE_CMD_HISTORY, m_sCmdHistory);
	DDX_Text(pDX, IDC_E_USER_INPUT_CMD, m_sUserInputCmd);
	DDX_Control(pDX, IDC_RCE_CMD_PROMPT, m_ctrlCmdLabel);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CCmdLineDlg, CDialog)
	//{{AFX_MSG_MAP(CCmdLineDlg)
	ON_WM_SIZE()
	ON_NOTIFY(NM_RETURN, IDC_E_USER_INPUT_CMD, OnReturnEUserInputParam)
	ON_WM_PAINT()
	ON_NOTIFY(EN_SELCHANGE, IDC_RCE_CMD_HISTORY, OnSelchangeCmdHistory)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCmdLineDlg message handlers

BOOL CCmdLineDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	RelayoutWnd();
	//
	m_richCmdHistory.SetEventMask(m_richCmdHistory.GetEventMask()|ENM_KEYEVENTS|ENM_MOUSEEVENTS);//|ENM_SELCHANGE);
	return TRUE;
}

void CCmdLineDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	RelayoutWnd();
}

void CCmdLineDlg::OnPaint() 
{
	RECT rc;
	GetClientRect(&rc);
	CPaintDC dc(this);
	CPen grayPen(PS_SOLID, 1, RGB(120,120,120));
	CPen whitePen(PS_SOLID, 1, RGB(255,255,255));
	CPen *pOldPen = dc.SelectObject(&whitePen);
	dc.MoveTo(0,rc.bottom-m_nCmdLineHeight);
	dc.LineTo(rc.right,rc.bottom-m_nCmdLineHeight);
	dc.SelectObject(&grayPen);
	dc.MoveTo(0,rc.bottom-m_nCmdLineHeight-1);
	dc.LineTo(rc.right,rc.bottom-m_nCmdLineHeight-1);
	// TODO: Add your message handler code here
	dc.SelectObject(pOldPen);
	grayPen.DeleteObject();
	whitePen.DeleteObject();
}

void CCmdLineDlg::OnOK() 
{
	UpdateData();
	::PostMessage(m_hMsgProcessWnd,WM_CMDLINE_AFFIRMED,(WPARAM)&m_sUserInputCmd,0);
}

void CCmdLineDlg::OnCancel() 
{
	UpdateData();
	::PostMessage(m_hMsgProcessWnd,WM_CMDLINE_CANCELED,(WPARAM)&m_sUserInputCmd,0);
}

void CCmdLineDlg::OnReturnEUserInputParam(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	if(pNMHDR->code==VK_RETURN)
		*pResult = 0;
	else if(pNMHDR->code==VK_ESCAPE)
		*pResult = 0;
}

BOOL CCmdLineDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if(pMsg->message == WM_KEYDOWN)   
	{   
		if(!(GetKeyState(VK_CONTROL)&0x8000))
		{
			CWnd *pWnd = GetDlgItem(IDC_E_USER_INPUT_CMD);
			if(pWnd)
			{
				pWnd->SetFocus();
				pMsg->hwnd = pWnd->GetSafeHwnd();
			}
		}
		if((pMsg->wParam==VK_UP) || (pMsg->wParam==VK_RIGHT))
		{
			UpdateData();
			m_sUserInputCmd = m_sPreCmd;
			UpdateData(FALSE);
		}
		if(pMsg->wParam==VK_SPACE)	//使空格等同回车 wht 10-03-25
			pMsg->wParam=VK_RETURN;
		return CDialog::PreTranslateMessage(pMsg);
	}   
	return CDialog::PreTranslateMessage(pMsg);
}

void CCmdLineDlg::FinishCmdLine(const char* inputCmdStr/*=NULL*/)
{
	//UpdateData();
	this->DetachMiniCmdOptionPanel();
	if(inputCmdStr==NULL)
		GetDlgItem(IDC_E_USER_INPUT_CMD)->GetWindowText(m_sUserInputCmd);
	else
		m_sUserInputCmd=inputCmdStr;
	if(m_sCmdHistory.GetLength()>0)
		m_sCmdHistory+="\n";
	m_sCmdHistory+=m_sCmdPrompt+m_sUserInputCmd;
#ifdef AFX_TARG_ENU_ENGLISH
	m_sCmdPrompt="Command:";
#else 
	m_sCmdPrompt="命令:";
#endif
	m_sUserInputCmd = "";
	RelayoutWnd();	//重设光标位置
	GetDlgItem(IDC_RCE_CMD_PROMPT)->SetWindowText(m_sCmdPrompt);
	GetDlgItem(IDC_E_USER_INPUT_CMD)->SetWindowText(m_sUserInputCmd);
	GetDlgItem(IDC_RCE_CMD_HISTORY)->SetWindowText(m_sCmdHistory);
	//UpdateData(FALSE);
	CRichEditCtrl *pRichEdit = (CRichEditCtrl*)GetDlgItem(IDC_RCE_CMD_HISTORY);
	if(pRichEdit)
	{
		CClientDC dc(this);
		dc.SelectObject(&afxGlobalData.fontRegular);
#ifdef AFX_TARG_ENU_ENGLISH
		CSize size = dc.GetTextExtent("Command:",5);
#else 
		CSize size = dc.GetTextExtent("命令:",5);
#endif
		size.cy=12+4;	//根据实测行间隙为4,总半行距16象素
		RECT rc;
		pRichEdit->GetClientRect(&rc);
		int nLineCount = pRichEdit->GetLineCount();
		int nCurLine = pRichEdit->GetFirstVisibleLine();
		int nLine = (rc.bottom-rc.top)/size.cy-1;	//减1防止最下边一行仅显示上半部分 
		pRichEdit->LineScroll(nLineCount-nCurLine-nLine);
	}
}

void CCmdLineDlg::CancelCmdLine()
{
	UpdateData();
	this->DetachMiniCmdOptionPanel();
	m_bGettingStrFromCmdLine=FALSE;
	if(m_sCmdHistory.GetLength()>0)
		m_sCmdHistory+="\n";
#ifdef AFX_TARG_ENU_ENGLISH
	m_sCmdHistory+=m_sCmdPrompt+m_sUserInputCmd+"*Cancel*";
	FillCmdLine("Command:","");
#else 
	m_sCmdHistory+=m_sCmdPrompt+m_sUserInputCmd+"*取消*";
	FillCmdLine("命令:","");
#endif
	UpdateData(FALSE);
	CRichEditCtrl *pRichEdit = (CRichEditCtrl*)GetDlgItem(IDC_RCE_CMD_HISTORY);
	if(pRichEdit)
	{
		CClientDC dc(this);
		dc.SelectObject(&afxGlobalData.fontRegular);
#ifdef AFX_TARG_ENU_ENGLISH
		CSize size = dc.GetTextExtent("Command:",5);
#else 
		CSize size = dc.GetTextExtent("命令:",5);
#endif
		size.cy=12+4;	//根据实测行间隙为4,总半行距16象素
		RECT rc;
		pRichEdit->GetClientRect(&rc);
		int nLineCount = pRichEdit->GetLineCount();
		int nCurLine = pRichEdit->GetFirstVisibleLine();
		int nLine = (rc.bottom-rc.top)/size.cy-1;	//减1防止最下边一行仅显示上半部分
		pRichEdit->LineScroll(nLineCount-nCurLine-nLine);
	}
}

void CCmdLineDlg::FillCmdLine(CString cmdPromptStr,CString cmdStr)
{
	m_sCmdPrompt = cmdPromptStr;
	m_sUserInputCmd = cmdStr;
	RelayoutWnd();
	GetDlgItem(IDC_RCE_CMD_PROMPT)->SetWindowText(m_sCmdPrompt);
	GetDlgItem(IDC_E_USER_INPUT_CMD)->SetWindowText(m_sUserInputCmd);
	//UpdateData(FALSE);
	/*CRichEditCtrl *pRichEdit = (CRichEditCtrl*)GetDlgItem(IDC_RCE_CMD_HISTORY);
	if(pRichEdit)
	{
		CClientDC dc(this);
		CSize size = dc.GetTextExtent("命令:",5);
		RECT rc;
		pRichEdit->GetClientRect(&rc);
		int nLineCount = pRichEdit->GetLineCount();
		int nCurLine = pRichEdit->GetFirstVisibleLine();
		int nLine = (rc.bottom-rc.top)/size.cy-1;	//减1防止最下边一行仅显示上半部分
		pRichEdit->LineScroll(nLineCount-nCurLine-nLine);
	}*/
}
void CCmdLineDlg::PromptStrInCmdLine(const char* cmdPromptStr)
{
	m_sCmdPrompt = cmdPromptStr;
	m_sUserInputCmd = "";
	RelayoutWnd();
	GetDlgItem(IDC_RCE_CMD_PROMPT)->SetWindowText(m_sCmdPrompt);
	GetDlgItem(IDC_E_USER_INPUT_CMD)->SetWindowText(m_sUserInputCmd);
	//UpdateData(FALSE);
	FinishCmdLine();
}
BOOL CCmdLineDlg::AttachMiniCmdOptionPanel(CCmdLineMiniPanel* pMiniPanel,int cx)
{
	if(m_pMiniPanel!=pMiniPanel&&pMiniPanel&&pMiniPanel->GetSafeHwnd()!=NULL)
		pMiniPanel->DestroyWindow();
	m_pMiniPanel=pMiniPanel;
	pMiniPanel->SetParentCmdLineDlg(this);

	RECT rcCmdLine,rcMiniPanel;
	CWnd *pCmdLineWnd = GetDlgItem(IDC_E_USER_INPUT_CMD);
	pCmdLineWnd->GetWindowRect(&rcCmdLine);
	if(pMiniPanel->GetSafeHwnd()==NULL)
		pMiniPanel->Create(pMiniPanel->DlgResourceID(),this);
	pMiniPanel->GetWindowRect(&rcMiniPanel);

	int height=rcMiniPanel.bottom-rcMiniPanel.top+1;
	int yOffsetPanel=(rcCmdLine.bottom-rcCmdLine.top+1)/2-height/2;

	ScreenToClient(&rcCmdLine);
	rcMiniPanel=rcCmdLine;
	rcCmdLine.right=rcCmdLine.left+cx-1;
	rcMiniPanel.top=rcCmdLine.top+yOffsetPanel;
	rcMiniPanel.left=rcCmdLine.right+1;
	pCmdLineWnd->MoveWindow(&rcCmdLine);
	pMiniPanel->MoveWindow(&rcMiniPanel);
	pMiniPanel->ShowWindow(SW_SHOW);
	pCmdLineWnd->GetWindowRect(&rcCmdLine);
	pCmdLineWnd->SetFocus();
	return pMiniPanel->GetSafeHwnd()!=NULL;
}
void CCmdLineDlg::DetachMiniCmdOptionPanel()
{
	if(m_pMiniPanel&&m_pMiniPanel->GetSafeHwnd())
	{
		m_pMiniPanel->SetParentCmdLineDlg(NULL);
		m_pMiniPanel->ShowWindow(SW_HIDE);
		m_pMiniPanel->DestroyWindow();
		m_pMiniPanel=NULL;
		RelayoutWnd();
	}
}
HWND CCmdLineDlg::SetCmdLineFocus()
{
	CWnd *pWnd = GetDlgItem(IDC_E_USER_INPUT_CMD);
	if(pWnd)
	{
		pWnd->SetFocus();
		return pWnd->GetSafeHwnd();
	}
	return 0;
}

void CCmdLineDlg::RelayoutWnd()
{
	CWnd *pCmdPromptWnd = GetDlgItem(IDC_RCE_CMD_PROMPT);
	CWnd *pCmdLineWnd = GetDlgItem(IDC_E_USER_INPUT_CMD);
	CWnd *pCmdHistoryWnd = GetDlgItem(IDC_RCE_CMD_HISTORY);
	CClientDC dc(this); // device context for painting
	CFont* pOldFont=GetFont();
	if(pOldFont)
		dc.SelectObject(pOldFont);//dc.SelectObject(&afxGlobalData.fontRegular);
	m_nCmdPromptWndWidth=dc.GetTextExtent(m_sCmdPrompt).cx+6;//margin=6;
	if(pCmdPromptWnd&&pCmdLineWnd&&pCmdHistoryWnd)
	{
		RECT rcFull,rcCmdPrompt,rcCmdLine,rcCmdHistory;
		GetClientRect(&rcFull);
		rcCmdHistory = rcFull;
		rcCmdHistory.bottom -= m_nCmdLineHeight+2;
		rcCmdPrompt.left = 0;
		rcCmdPrompt.bottom = rcCmdLine.bottom = rcFull.bottom;
		rcCmdPrompt.top = rcCmdLine.top = rcCmdLine.bottom-m_nCmdLineHeight+1;
		rcCmdPrompt.right = m_nCmdPromptWndWidth;
		rcCmdLine.left = rcCmdPrompt.right;
		rcCmdLine.right = rcFull.right;
		pCmdHistoryWnd->MoveWindow(&rcCmdHistory);
		pCmdPromptWnd->MoveWindow(&rcCmdPrompt);
		if(m_pMiniPanel!=NULL&&m_pMiniPanel->GetSafeHwnd()!=NULL)
		{
			RECT rcMiniPanel;
			m_pMiniPanel->GetWindowRect(&rcMiniPanel);
			this->ScreenToClient(&rcMiniPanel);
			rcCmdLine.right=rcMiniPanel.left-1;
		}
		pCmdLineWnd->MoveWindow(&rcCmdLine);
	}
}
static bool IsKeyWord(const char* cmdStr,CXhChar16* keyword_arr,int keyword_count=1)
{
	for(int i=0;i<keyword_count;i++)
	{
		if(stricmp(cmdStr,keyword_arr[i])==0)
			return true;
	}
	return false;
}
BOOL CCmdLineDlg::GetStrFromCmdLine(CString &cmdStr,DWORD dwFlag/*=0*/,const char* keywords/*=NULL*/)//默认不响应鼠标左键抬起的消息
{
	BOOL bRet=TRUE;
	CPoint point;
	CRect clientrc;
	CTPSMView *pView=theApp.GetActiveView();
	if(pView)
		pView->GetWindowRect(&clientrc);
	ARRAY_LIST<CXhChar16> arrCmdKeywords(0,2);
	if((dwFlag&KEYWORD_AS_RETURN)&&keywords!=NULL)
	{
		CXhChar200 keyliststr=keywords;
		char* key=strtok(keyliststr,"|,");
		while(key!=NULL)
		{
			arrCmdKeywords.append(key);
			key=strtok(NULL,"|,");
		}
	}
	bool charChanged=false;
	for(;;)
	{
		m_bGettingStrFromCmdLine = TRUE;
		if(m_bForceExitCommand)
		{
			m_bGettingStrFromCmdLine = m_bForceExitCommand = FALSE;
			return FALSE;
		}
		MSG message;
		if(PeekMessage(&message,NULL,0,0,PM_REMOVE))
		{
			GetCursorPos(&point);
			if((((dwFlag&LBUTTONDOWN_AS_RETURN)&&message.message==WM_LBUTTONDOWN)||
				((dwFlag&LBUTTONUP_AS_RETURN)&&message.message==WM_LBUTTONUP))
				&&clientrc.PtInRect(point))
			{	//点击鼠标左键和按回车或按空格效果相同
				UpdateData();
				cmdStr = m_sUserInputCmd;
				//FinishCmdLine();
				m_bGettingStrFromCmdLine = m_bForceExitCommand = FALSE;
				return TRUE;
			}
			else if(charChanged&&(dwFlag&KEYWORD_AS_RETURN)&&IsKeyWord(m_sUserInputCmd,arrCmdKeywords.m_pData,arrCmdKeywords.GetSize()))
			{
				cmdStr = m_sUserInputCmd;
				FinishCmdLine();
				m_bGettingStrFromCmdLine = m_bForceExitCommand = FALSE;
				return TRUE;
			}
			else if(::WaitForSingleObject(RButtonDown,0) == WAIT_OBJECT_0)
			{
				CancelCmdLine();
				m_bGettingStrFromCmdLine = m_bForceExitCommand = FALSE;
				return FALSE;
			}
			else if(message.message==WM_KEYDOWN)
			{
				if(message.wParam==VK_RETURN||message.wParam==VK_SPACE)
				{	//按回车或按空格效果相同
					UpdateData();
					cmdStr = m_sUserInputCmd;
					FinishCmdLine();
					m_bGettingStrFromCmdLine = m_bForceExitCommand = FALSE;
					return TRUE;
				}
				else if(message.wParam==VK_ESCAPE)
				{
					CancelCmdLine();
					m_bGettingStrFromCmdLine = m_bForceExitCommand = FALSE;
					return FALSE;
				}
				CWnd *pWnd = CWnd::FromHandle(message.hwnd);
				CWnd *pParentWnd = NULL;
				HWND hParent = NULL;
				if(pWnd)
				{
					pParentWnd = pWnd->GetParent();
					if(pParentWnd)
						hParent = pParentWnd->GetSafeHwnd();
				}
				if((message.hwnd==GetParent()->GetSafeHwnd()||message.hwnd==GetSafeHwnd()||hParent==GetSafeHwnd())&&
					message.wParam!=VK_SHIFT&&message.wParam!=VK_CONTROL&&message.wParam!=VK_CAPITAL)
				{
					CWnd *pWnd = GetDlgItem(IDC_E_USER_INPUT_CMD);
					if(pWnd)
					{
						pWnd->SetFocus();
						message.hwnd = pWnd->GetSafeHwnd();
					}
				}
			}
			TranslateMessage(&message);
			DispatchMessage(&message);
			charChanged=false;
			if(message.message==WM_CHAR&&(dwFlag&KEYWORD_AS_RETURN)>0)
			{
				CString old=m_sUserInputCmd;
				GetDlgItem(IDC_E_USER_INPUT_CMD)->GetWindowText(m_sUserInputCmd);
				if(old.CompareNoCase(m_sUserInputCmd)!=0)
					charChanged=true;
			}
		}
	}
	return FALSE;
}
BOOL CCmdLineDlg::PeekStrFromCmdLine(CString& cmdStr)
{
	UpdateData();
	cmdStr = m_sUserInputCmd;
	return m_sUserInputCmd.GetLength()>0;
}

BOOL CCmdLineDlg::IsHasFocus()
{
	CWnd *pWnd = GetFocus();
	CWnd *pParentWnd = NULL;
	HWND hParent = NULL;
	if(pWnd)
	{
		pParentWnd = pWnd->GetParent();
		if(pParentWnd)
			hParent = pParentWnd->GetSafeHwnd();
	}
	if(pWnd->GetSafeHwnd()==GetParent()->GetSafeHwnd()||pWnd->GetSafeHwnd()==GetSafeHwnd()||hParent==GetSafeHwnd())
		return TRUE;
	else
		return FALSE;
}

static void FindPartHandle(char *sLineText,long &hFirstPart,long &hSecPart)
{
	CXhChar50 sHandle;
	hFirstPart=hSecPart=0;
	int nStrLen=strlen(sLineText);
	for(int i=0;i<nStrLen;i++)
	{
		char cCurChar=sLineText[i];
		char cPrevChar=' ';
		if(i>1)
			cPrevChar=sLineText[i-1];
		//
		if(cPrevChar=='0'&&(cCurChar=='X'||cCurChar=='x'))
			sHandle.Copy("0X");
		else if(sHandle.GetLength()>1)
		{
			cCurChar=(char)toupper(cCurChar);
			if(isdigit(cCurChar)||cCurChar=='A'||cCurChar=='B'
				||cCurChar=='C'||cCurChar=='D'||cCurChar=='E'||cCurChar=='F')
				sHandle.Append(CXhChar16("%c",cCurChar));
			else
			{
				if(hFirstPart==0)
					sscanf(sHandle,"%X",&hFirstPart);
				else if(hSecPart==0)
					sscanf(sHandle,"%X",&hSecPart);
				if(hFirstPart&&hSecPart)
					break;
				sHandle.Empty();
			}
		}
	}
}

void CCmdLineDlg::OnSelchangeCmdHistory(NMHDR* pNMHDR, LRESULT* pResult) 
{
	SELCHANGE *pSelChange = reinterpret_cast<SELCHANGE *>(pNMHDR);
	CTPSMView* pView=theApp.GetActiveView();
	if(pView==NULL)
		return;
	//得到当前选中行的索引号
	int iCurLine=-1,iBeginChar=-1,iEndChar=-1,nLineCount = m_richCmdHistory.GetLineCount();
	iBeginChar=m_richCmdHistory.LineIndex();
	for(int i=0;i<nLineCount;i++)
	{
		int iCurBegin=m_richCmdHistory.LineIndex(i);
		if(iCurBegin==iBeginChar)
		{
			iCurLine=i;
			iEndChar=m_richCmdHistory.LineIndex(i+1);
			break;
		}
	}
	if(iCurLine==-1)
		return;
	//选中当前行，设置当前行背景颜色为蓝色
	m_richCmdHistory.SetSel(iBeginChar,iEndChar);
	CHARFORMAT2 cf;
	m_richCmdHistory.GetSelectionCharFormat(cf);
	cf.crBackColor=RGB(0,0,255);
	m_richCmdHistory.SetSelectionCharFormat(cf);
	//得到选中行内容
	char sCurText[501]="";
	m_richCmdHistory.GetLine(iCurLine,sCurText,500);
	//得到构件句柄
	long hFirstPart=0,hSecPart=0;
	FindPartHandle(sCurText,hFirstPart,hSecPart);
	pView->SolidDraw()->ReleaseSnapStatus();
	pView->SolidDraw()->SetEntSnapStatus(hFirstPart);
	pView->SolidDraw()->SetEntSnapStatus(hSecPart);
	*pResult = 0;
}
//仅下面的函数还不够，很多时候系统还会后续执行CancelCmdLine()，依然会在Destroyed后调用命令行 wjh-2011.8.29
void CCmdLineDlg::OnDestroy() 
{
	m_bForceExitCommand=TRUE;
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	
}
/////////////////////////////////////////////////////////////////////////////
// PEEKCMDLINE class
PEEKCMDLINE::PEEKCMDLINE(const char* keywords,CCmdLineDlg* pCmdLine/*=NULL*/)
{
	m_pCmdLine=pCmdLine!=NULL?pCmdLine:((CMainFrame*)AfxGetMainWnd())->GetCmdLinePage();
	InitKeywords(keywords);
}
bool PEEKCMDLINE::InitKeywords(const char* keywords)
{
	if(keywords!=NULL)
	{
		m_arrKeywords.SetSize(0,2);
		CXhChar200 keyliststr=keywords;
		char* key=strtok(keyliststr,"|,");
		while(key!=NULL)
		{
			m_arrKeywords.append(key);
			key=strtok(NULL,"|,");
		}
	}
	return m_arrKeywords.GetSize()>0;
}
bool PEEKCMDLINE::MatchKeyword()
{
	CString cmdStr;
	if(!m_pCmdLine->PeekStrFromCmdLine(cmdStr))
		return false;
	return IsKeyWord(cmdStr,m_arrKeywords.m_pData,m_arrKeywords.GetSize());
}
int PEEKCMDLINE::MatchKeyword(ISolidSnap::SNAPMSGLOOP_CONTEXT* pContext)
{
	CCmdLineDlg *pCmdLine=((PEEKCMDLINE*)pContext->pExterContext)->GetCmdLineWnd();
	if(pContext==NULL)
		return -1;
	else if(pContext->message==WM_KEYDOWN)
	{
		if(pContext->wParam==VK_RETURN)
			return -1;
		else if(pContext->wParam==VK_ESCAPE)
			return -1;
		CWnd *pWnd = CWnd::FromHandle(pContext->hMsgSrcWnd);
		CWnd *pParentWnd = NULL;
		HWND hParent = NULL;
		if(pWnd)
		{
			pParentWnd = pWnd->GetParent();
			if(pParentWnd)
				hParent = pParentWnd->GetSafeHwnd();
		}
		if((pContext->hMsgSrcWnd==pCmdLine->GetParent()->GetSafeHwnd()||pContext->hMsgSrcWnd==pCmdLine->GetSafeHwnd()||hParent==pCmdLine->GetSafeHwnd())&&
			pContext->wParam!=VK_SHIFT&&pContext->wParam!=VK_CONTROL&&pContext->wParam!=VK_CAPITAL)
		{
			pContext->hMsgSrcWnd=pCmdLine->SetCmdLineFocus();
			return 0;
		}
		return -1;
	}
	else if(pContext->message==WM_CHAR)
	{
		return ((PEEKCMDLINE*)pContext->pExterContext)->MatchKeyword()?1:0;
	}
	else
		return -1;
}
