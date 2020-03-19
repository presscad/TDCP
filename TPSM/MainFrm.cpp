
// MainFrm.cpp : CMainFrame 类的实现
//

#include "stdafx.h"
#include "TPSM.h"
#include "MainFrm.h"
#include "TPSMModel.h"
#include "PropertyListOper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////////
//静态和回调函数
static BOOL CreateDockingWindow(CWnd *pParentWnd,UINT nDlgID,UINT nViewNameID,CDialogPanel &dlgPanel,
									DWORD dwPosStyle,int nWidth=200,int nHeight=200)
{
	CString sViewName="";
	BOOL bNameValid = sViewName.LoadString(nViewNameID);
	ASSERT(bNameValid);
	if (!dlgPanel.Create(sViewName, pParentWnd, CRect(0, 0, nWidth, nHeight), TRUE, nDlgID,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | dwPosStyle | CBRS_FLOAT_MULTI))
	{
		TRACE0("未能创建“"+sViewName+"”窗口\n");
		return FALSE;
	}
	return TRUE;
}
static BOOL CreateToolBar(CWnd *pParentWnd,CMFCToolBar &toolBar,UINT uToolBarID,UINT uToolBarNameID,BOOL bSupportCustomize)
{
	if (!toolBar.CreateEx(pParentWnd, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC,CRect(1,1,1,1),uToolBarID) ||
		!toolBar.LoadToolBar(uToolBarID))
	{
		TRACE0("未能创建工具栏\n");
		return FALSE;   // fail to create
	}
	CString strToolBarName;
	BOOL bNameValid = strToolBarName.LoadString(uToolBarNameID);
	ASSERT(bNameValid);
	toolBar.SetWindowText(strToolBarName);
	if(bSupportCustomize)
	{
		CString strCustomize;
		bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
		ASSERT(bNameValid);
		toolBar.EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
	}
	return TRUE;
}
//////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWndEx)

const int  iMaxUserToolbars = 10;
const UINT uiFirstUserToolBarId = AFX_IDW_CONTROLBAR_FIRST + 40;
const UINT uiLastUserToolBarId = uiFirstUserToolBarId + iMaxUserToolbars - 1;

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWndEx)
	ON_WM_CREATE()
	ON_COMMAND(ID_WINDOW_MANAGER, &CMainFrame::OnWindowManager)
	ON_COMMAND(ID_VIEW_CUSTOMIZE, &CMainFrame::OnViewCustomize)
	ON_REGISTERED_MESSAGE(AFX_WM_CREATETOOLBAR, &CMainFrame::OnToolbarCreateNew)
	ON_COMMAND_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7, &CMainFrame::OnApplicationLook)
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7, &CMainFrame::OnUpdateApplicationLook)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // 状态行指示器
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

// CMainFrame 构造/析构

CMainFrame::CMainFrame()
{
	// TODO: 在此添加成员初始化代码
	theApp.m_nAppLook = theApp.GetInt(_T("ApplicationLook"), ID_VIEW_APPLOOK_VS_2008);
	m_PanelProperty.Init(RUNTIME_CLASS(CPropertyDlg), IDD_PROPERTY_DLG);
	m_PanelWireData.Init(RUNTIME_CLASS(CWireDataDlg),IDD_WIRE_DATA_DLG);
	m_PanelProjectTree.Init(RUNTIME_CLASS(CProjectTreeDlg),IDD_PROJECT_TREE_DLG);
	m_PanelCmdLine.Init(RUNTIME_CLASS(CCmdLineDlg), IDD_CMD_LINE_DLG);
}

CMainFrame::~CMainFrame()
{
}

CSysPromptObject CMainFrame::prompt;
CMainFrame::PROMPT_LIFE::PROMPT_LIFE(bool disableAutoCloseOnUse/*=false*/)
{
	m_bDisableAutoCloseOnUse=disableAutoCloseOnUse;
	m_bAutoClose=true;
}
CMainFrame::PROMPT_LIFE::~PROMPT_LIFE()
{
	if(m_bAutoClose)
		CMainFrame::prompt.Close();
}
void CMainFrame::PROMPT_LIFE::Prompt(const char* promptstr){
	if(m_bDisableAutoCloseOnUse)
		m_bAutoClose=false;
	CMainFrame::prompt.Prompt(promptstr);
}
void CMainFrame::PROMPT_LIFE::SetPromptPicture(long bitmapId){
	if(m_bDisableAutoCloseOnUse)
		m_bAutoClose=false;
	CMainFrame::prompt.SetPromptPicture(bitmapId);
}
void CMainFrame::PROMPT_LIFE::SetPromptPicture(const char* bmpfile){
	if(m_bDisableAutoCloseOnUse)
		m_bAutoClose=false;
	CMainFrame::prompt.SetPromptPicture(bmpfile);
}

void CMainFrame::OnUpdateFrameTitle(BOOL bAddToTitle)
{
	CString app_name="铁塔规划三维建模-TPSM";
	SetWindowText(app_name);
}
BOOL CMainFrame::CreateDockingWindows()
{
	CreateDockingWindow(this,IDD_PROPERTY_DLG,IDS_PROP_VIEW,m_PanelProperty,CBRS_RIGHT);
	CreateDockingWindow(this,IDD_PROJECT_TREE_DLG,IDS_PROJECT_VIEW,m_PanelProjectTree,CBRS_LEFT);
	//CreateDockingWindow(this,IDD_CMD_LINE_DLG,IDS_CMD_LINE_VIEW,m_PanelCmdLine,CBRS_BOTTOM);
	CreateDockingWindow(this,IDD_WIRE_DATA_DLG,IDS_MODEL_VIEW,m_PanelWireData, CBRS_BOTTOM);
	return TRUE;
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;
	// 基于持久值设置视觉管理器和样式
	OnApplicationLook(theApp.m_nAppLook);

	CMDITabInfo mdiTabParams;
	mdiTabParams.m_style = CMFCTabCtrl::STYLE_3D_ONENOTE; // 其他可用样式...
	mdiTabParams.m_bActiveTabCloseButton = TRUE;      // 设置为 FALSE 会将关闭按钮放置在选项卡区域的右侧
	mdiTabParams.m_bTabIcons = FALSE;    // 设置为 TRUE 将在 MDI 选项卡上启用文档图标
	mdiTabParams.m_bAutoColor = TRUE;    // 设置为 FALSE 将禁用 MDI 选项卡的自动着色
	mdiTabParams.m_bDocumentMenu = TRUE; // 在选项卡区域的右边缘启用文档菜单
	EnableMDITabbedGroups(TRUE, mdiTabParams);

	if (!m_wndMenuBar.Create(this))
	{
		TRACE0("未能创建菜单栏\n");
		return -1;      // 未能创建
	}

	m_wndMenuBar.SetPaneStyle(m_wndMenuBar.GetPaneStyle() | CBRS_SIZE_DYNAMIC | CBRS_TOOLTIPS | CBRS_FLYBY);

	// 防止菜单栏在激活时获得焦点
	CMFCPopupMenu::SetForceMenuFocus(FALSE);
	//标准工具栏
	UINT uToolBarID=theApp.m_bHiColorIcons ? IDR_MAINFRAME_256 : IDR_MAINFRAME;
	if(!CreateToolBar(this,m_wndToolBar,uToolBarID,IDS_TOOLBAR_GL,TRUE))
		return -1;
	//自定义工具栏
	if(!CreateToolBar(this,m_wndGLBar,IDR_BAR_OPENGL,IDS_TOOLBAR_GL,TRUE))
		return -1;

	// 允许用户定义的工具栏操作:
	InitUserToolbars(NULL, uiFirstUserToolBarId, uiLastUserToolBarId);

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("未能创建状态栏\n");
		return -1;      // 未能创建
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));

	// TODO: 如果您不希望工具栏和菜单栏可停靠，请删除这五行
	m_wndMenuBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndGLBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndMenuBar);
	DockPane(&m_wndToolBar);
	DockPane(&m_wndGLBar);
	DockPaneLeftOf(&m_wndToolBar,&m_wndGLBar);

	// 启用 Visual Studio 2005 样式停靠窗口行为
	CDockingManager::SetDockingMode(DT_SMART);
	//
	EnableDocking(CBRS_ALIGN_ANY);
	// 启用 Visual Studio 2005 样式停靠窗口自动隐藏行为
	EnableAutoHidePanes(CBRS_ALIGN_ANY);
	// 启用增强的窗口管理对话框
	EnableWindowsDialog(ID_WINDOW_MANAGER, ID_WINDOW_MANAGER, TRUE);
	// 启用工具栏和停靠窗口菜单替换
	CString strCustomize;
	strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	EnablePaneMenu(TRUE, ID_VIEW_CUSTOMIZE, strCustomize, ID_VIEW_TOOLBAR);

	// 创建停靠窗口
	if(!CreateDockingWindows())
	{
		TRACE0("未能创建停靠窗口\n");
		return -1;
	}
	m_PanelProjectTree.EnableDocking(CBRS_ALIGN_LEFT);
	DockPane(&m_PanelProjectTree);
	//m_PanelCmdLine.EnableDocking(CBRS_ALIGN_ANY);
	//DockPane(&m_PanelCmdLine);
	m_PanelProperty.EnableDocking(CBRS_ALIGN_RIGHT);
	DockPane(&m_PanelProperty);
	m_PanelWireData.EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_PanelWireData);
	//m_PanelWireData.AttachToTabWnd(&m_PanelProperty,DM_SHOW,FALSE);	
	//设置正常字体
	LOGFONT lf;
	afxGlobalData.fontRegular.GetLogFont(&lf);   
	lf.lfHeight =-12;   
	lstrcpy(lf.lfFaceName, _T("宋体"));     // using without style office 2007   
	afxGlobalData.fontRegular.DeleteObject();   
	afxGlobalData.fontRegular.CreateFontIndirect(&lf);
	// 启用快速(按住 Alt 拖动)工具栏自定义
	CMFCToolBar::EnableQuickCustomization();

	if (CMFCToolBar::GetUserImages() == NULL)
	{
		// 加载用户定义的工具栏图像
		if (m_UserImages.Load(_T(".\\UserImages.bmp")))
		{
			CMFCToolBar::SetUserImages(&m_UserImages);
		}
	}

	// 启用菜单个性化(最近使用的命令)
	// TODO: 定义您自己的基本命令，确保每个下拉菜单至少有一个基本命令。
	CList<UINT, UINT> lstBasicCommands;
	lstBasicCommands.AddTail(ID_FILE_NEW);
	lstBasicCommands.AddTail(ID_FILE_OPEN);
	lstBasicCommands.AddTail(ID_FILE_SAVE);
	lstBasicCommands.AddTail(ID_FILE_PRINT);
	lstBasicCommands.AddTail(ID_APP_EXIT);
	lstBasicCommands.AddTail(ID_EDIT_CUT);
	lstBasicCommands.AddTail(ID_EDIT_PASTE);
	lstBasicCommands.AddTail(ID_EDIT_UNDO);
	lstBasicCommands.AddTail(ID_APP_ABOUT);
	lstBasicCommands.AddTail(ID_VIEW_STATUS_BAR);
	lstBasicCommands.AddTail(ID_VIEW_TOOLBAR);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2003);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_VS_2005);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2007_BLUE);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2007_SILVER);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2007_BLACK);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2007_AQUA);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_WINDOWS_7);
	CMFCToolBar::SetBasicCommands(lstBasicCommands);

	// 将文档名和应用程序名称在窗口标题栏上的顺序进行交换。这
	// 将改进任务栏的可用性，因为显示的文档名带有缩略图。
	ModifyStyle(0, FWS_PREFIXTITLE);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWndEx::PreCreateWindow(cs) )
		return FALSE;
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return TRUE;
}

// CMainFrame 诊断

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWndEx::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWndEx::Dump(dc);
}
#endif //_DEBUG


// CMainFrame 消息处理程序

void CMainFrame::OnWindowManager()
{
	ShowWindowsDialog();
}

void CMainFrame::OnViewCustomize()
{
	CMFCToolBarsCustomizeDialog* pDlgCust = new CMFCToolBarsCustomizeDialog(this, TRUE /* 扫描菜单*/);
	pDlgCust->EnableUserDefinedToolbars();
	pDlgCust->Create();
}

LRESULT CMainFrame::OnToolbarCreateNew(WPARAM wp,LPARAM lp)
{
	LRESULT lres = CMDIFrameWndEx::OnToolbarCreateNew(wp,lp);
	if (lres == 0)
	{
		return 0;
	}

	CMFCToolBar* pUserToolbar = (CMFCToolBar*)lres;
	ASSERT_VALID(pUserToolbar);

	BOOL bNameValid;
	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);

	pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
	return lres;
}

void CMainFrame::OnApplicationLook(UINT id)
{
	CWaitCursor wait;

	theApp.m_nAppLook = id;

	switch (theApp.m_nAppLook)
	{
	case ID_VIEW_APPLOOK_WIN_2000:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManager));
		break;

	case ID_VIEW_APPLOOK_OFF_XP:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOfficeXP));
		break;

	case ID_VIEW_APPLOOK_WIN_XP:
		CMFCVisualManagerWindows::m_b3DTabsXPTheme = TRUE;
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
		break;

	case ID_VIEW_APPLOOK_OFF_2003:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2003));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	case ID_VIEW_APPLOOK_VS_2005:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2005));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	case ID_VIEW_APPLOOK_VS_2008:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2008));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	case ID_VIEW_APPLOOK_WINDOWS_7:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows7));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	default:
		switch (theApp.m_nAppLook)
		{
		case ID_VIEW_APPLOOK_OFF_2007_BLUE:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_LunaBlue);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_BLACK:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_SILVER:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Silver);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_AQUA:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Aqua);
			break;
		}

		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
		CDockingManager::SetDockingMode(DT_SMART);
	}

	RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ERASE);

	theApp.WriteInt(_T("ApplicationLook"), theApp.m_nAppLook);
}

void CMainFrame::OnUpdateApplicationLook(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(theApp.m_nAppLook == pCmdUI->m_nID);
}

BOOL CMainFrame::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext) 
{
	// 基类将执行真正的工作

	if (!CMDIFrameWndEx::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))
	{
		return FALSE;
	}


	// 为所有用户工具栏启用自定义按钮
	BOOL bNameValid;
	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);

	for (int i = 0; i < iMaxUserToolbars; i ++)
	{
		CMFCToolBar* pUserToolbar = GetUserToolBarByIndex(i);
		if (pUserToolbar != NULL)
		{
			pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
		}
	}

	return TRUE;
}

void CMainFrame::UpdatePropertyPage(CTPSMModel* pActiveModel/*=NULL*/)
{
	CLogErrorLife logLife;
	CTPSMModel* pModel = pActiveModel;
	if(pModel==NULL)
		pModel=(Manager.m_pActivePrj) ? pModel = Manager.m_pActivePrj->m_pActiveModel : NULL;
	if(pModel==NULL)
		DisplaySysProperty();
	else
	{
		CTPSMModel::InitPropHashtable(pModel);
		DisplayTowerParaProperty();
	}
}
