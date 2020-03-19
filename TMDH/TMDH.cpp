
// TMDH.cpp : 定义应用程序的类行为。
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "TMDH.h"
#include "MainFrm.h"
#include "ChildFrm.h"
#include "TMDHDoc.h"
#include "TMDHView.h"
#include "direct.h"
#include "ProcBarDlg.h"
#include "GlobalFunc.h"
#include "LicFuncDef.h"
#include "XhLmd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 唯一的一个 CTMDHApp 对象
CTMDHApp theApp;
//////////////////////////////////////////////////////////////////////////
//静态函数
void GetAppPath(char* startPath)
{
	char drive[4];
	char dir[MAX_PATH];
	char fname[MAX_PATH];
	char ext[MAX_PATH];

	_splitpath(__argv[0],drive,dir,fname,ext);
	strcpy(startPath,drive);
	strcat(startPath,dir);
	_chdir(startPath);
}
bool DetectSpecifiedHaspKeyFile(const char* default_file)
{
	FILE* fp=fopen(default_file,"rt");
	if(fp==NULL)
		return false;
	bool detected=false;
	CXhChar200 line_txt;//[MAX_PATH];
	CXhChar200 scope_xmlstr;
	scope_xmlstr.Append(
		"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"
		"<haspscope>");
	while(!feof(fp))
	{
		if(fgets(line_txt,line_txt.GetLengthMax(),fp)==NULL)
			break;
		line_txt.Replace("＝","=");
		char* final=SearchChar(line_txt,';');
		if(final!=NULL)
			*final=0;
		char *skey=strtok(line_txt," =,");
		//常规设置
		if(_stricmp(skey,"Key")==0)
		{
			if(skey=strtok(NULL,"=,"))
			{
				scope_xmlstr.Append("<hasp id=\"");
				scope_xmlstr.Append(skey);
				scope_xmlstr.Append("\" />");
				detected=true;
			}
		}
	}
	fclose(fp);
	scope_xmlstr.Append("</haspscope>");
	if(detected)
		SetHaspLoginScope(scope_xmlstr);
	return detected;
}
//////////////////////////////////////////////////////////////////////////
// CTMDHApp

BEGIN_MESSAGE_MAP(CTMDHApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CTMDHApp::OnAppAbout)
	// 基于文件的标准文档命令
	//ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
END_MESSAGE_MAP()


// CTMDHApp 构造

CTMDHApp::CTMDHApp()
{
	m_bHiColorIcons = TRUE;
	m_pDocTemplate=NULL;
	// 支持重新启动管理器
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_ALL_ASPECTS;
#ifdef _MANAGED
	// 如果应用程序是利用公共语言运行时支持(/clr)构建的，则:
	//     1) 必须有此附加设置，“重新启动管理器”支持才能正常工作。
	//     2) 在您的项目中，您必须按照生成顺序向 System.Windows.Forms 添加引用。
	System::Windows::Forms::Application::SetUnhandledExceptionMode(System::Windows::Forms::UnhandledExceptionMode::ThrowException);
#endif

	// TODO: 将以下应用程序 ID 字符串替换为唯一的 ID 字符串；建议的字符串格式
	//为 CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("TMDH.AppID.NoVersion"));

	// TODO: 在此处添加构造代码，
	strcpy(execute_path,"");
}

// CTMDHApp 初始化
CTMDHDoc* CTMDHApp::GetActiveDoc()
{
	CMDIChildWnd* pChildWnd=((CMainFrame*)m_pMainWnd)->MDIGetActive();
	if(pChildWnd)
	{
		CDocument *pDoc=pChildWnd->GetActiveDocument();
		if(pDoc->IsKindOf(RUNTIME_CLASS(CTMDHDoc)))
			return (CTMDHDoc*)pDoc;
	}
	return NULL;
}
CTMDHView* CTMDHApp::GetActiveView()
{
	CTMDHDoc* pDoc=GetActiveDoc();
	if(pDoc)
		return (CTMDHView*)pDoc->GetView(RUNTIME_CLASS(CTMDHView));
	else
		return NULL;
}
BOOL CTMDHApp::SetActiveDoc(void* pModel,BYTE ciModelFlag/*=0*/,BOOL bDraw/*=TRUE*/)
{
	POSITION doc_pos=m_pDocTemplate->GetFirstDocPosition();
	CTMDHDoc* pActiveDoc=NULL;
	while(doc_pos!=NULL)
	{
		CDocument *pDoc=m_pDocTemplate->GetNextDoc(doc_pos);
		if(pDoc!=NULL && pDoc->IsKindOf(RUNTIME_CLASS(CTMDHDoc)))
		{
			CTMDHDoc* pTmdhDoc=(CTMDHDoc*)pDoc;
			CTMDHView* pTmdhView=(CTMDHView*)pTmdhDoc->GetView(RUNTIME_CLASS(CTMDHView));
			if(pTmdhDoc->m_pData==pModel && pTmdhView)
			{
				((CMDIFrameWnd *)m_pMainWnd)->MDIActivate(pTmdhView->GetParentFrame());
				pActiveDoc=pTmdhDoc;
				break;
			}
		}
	}
	if(pActiveDoc==NULL)
	{	//通过文档模板创建新文档，同时创建框架和视图
		pActiveDoc=(CTMDHDoc*)m_pDocTemplate->OpenDocumentFile(NULL);
		pActiveDoc->m_pData=pModel;
		pActiveDoc->m_ciDataFlag=ciModelFlag;
		if(ciModelFlag==0)
		{
			CTMDHTower* pLdsData=(CTMDHTower*)pActiveDoc->m_pData;
			pActiveDoc->SetTitle(pLdsData->m_sTower);
			pActiveDoc->SetPathName(pLdsData->m_sPath,FALSE);
			pActiveDoc->InitTowerModel();
		}
		else if(ciModelFlag==1)
		{
			CTMDHGim* pGimData=(CTMDHGim*)pActiveDoc->m_pData;
			pActiveDoc->SetTitle(pGimData->m_sModName);
		}
		else if(ciModelFlag==2)
		{
			CTMDHModel* pModel=(CTMDHModel*)pActiveDoc->m_pData;
			pActiveDoc->SetTitle(pModel->m_sTowerType);
		}
	}
	//刷新塔型模型
	CTMDHView* pActiveView=(CTMDHView*)pActiveDoc->GetView(RUNTIME_CLASS(CTMDHView));
	if(bDraw && pActiveView)
		pActiveView->Refresh();
	return TRUE;
}
void CTMDHApp::DeleteDoc(void* pModel)
{
	POSITION doc_pos=m_pDocTemplate->GetFirstDocPosition();
	while(doc_pos!=NULL)
	{
		CDocument *pDoc=m_pDocTemplate->GetNextDoc(doc_pos);
		if(pDoc!=NULL && pDoc->IsKindOf(RUNTIME_CLASS(CTMDHDoc)))
		{
			CTMDHDoc* pTmdhDoc=(CTMDHDoc*)pDoc;
			if(pTmdhDoc->m_pData!=pModel)
				continue;
			CTMDHView* pTmdhView=(CTMDHView*)pTmdhDoc->GetView(RUNTIME_CLASS(CTMDHView));
			if(pTmdhView==NULL)
				continue;
			CMDIChildWnd* pChildWnd=(CMDIChildWnd*)pTmdhView->GetParentFrame();
			pChildWnd->MDIDestroy();
			break;
		}
	}
}
void CTMDHApp::GetAppVersion(char *file_version,char *product_version)
{
	DWORD dwLen=GetFileVersionInfoSize(__argv[0],0);
	BYTE *data=new BYTE[dwLen];
	WORD file_ver[4],product_ver[4];
	if(GetFileVersionInfo(__argv[0],0,dwLen,data))
	{
		VS_FIXEDFILEINFO *info;
		UINT uLen;
		VerQueryValue(data,"\\",(void**)&info,&uLen);
		memcpy(file_ver,&info->dwFileVersionMS,4);
		memcpy(&file_ver[2],&info->dwFileVersionLS,4);
		memcpy(product_ver,&info->dwProductVersionMS,4);
		memcpy(&product_ver[2],&info->dwProductVersionLS,4);
		if(file_version)
			sprintf(file_version,"%d.%d.%d.%d",file_ver[1],file_ver[0],file_ver[3],file_ver[2]);
		if(product_version)
			sprintf(product_version,"%d.%d.%d.%d",product_ver[1],product_ver[0],product_ver[3],product_ver[2]);
	}
	delete data;
}
BOOL CTMDHApp::InitInstance()
{
	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControlsEx()。否则，将无法创建窗口。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 将它设置为包括所有要在应用程序中使用的
	// 公共控件类。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);
	CWinAppEx::InitInstance();
	// 初始化 OLE 库
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();
	EnableTaskbarInteraction();
	// 使用 RichEdit 控件需要  AfxInitRichEdit2()	
	// AfxInitRichEdit2();
	if(!AfxInitRichEdit())
		return FALSE;
	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO: 应适当修改该字符串，
	// 例如修改为公司或组织名
	SetRegistryKey(_T("Xerofox"));
	LoadStdProfileSettings(4);  // 加载标准 INI 文件选项(包括 MRU)
	InitContextMenuManager();
	InitKeyboardManager();
	InitTooltipManager();
	//注册表中添加关键路径
	char APP_PATH[MAX_PATH],lic_file[MAX_PATH],sys_path[MAX_PATH],lds_path[MAX_PATH];
	GetAppPath(APP_PATH);
	strcpy(execute_path,APP_PATH);
	sprintf(lic_file,"%sGIM.lic",APP_PATH);
	WriteProfileString("Settings","lic_file",lic_file);
	sprintf(sys_path,"%sSys\\",APP_PATH);
	WriteProfileString("Settings","SYS_PATH",sys_path);
	sprintf(lds_path,"%slds\\",APP_PATH);	
	WriteProfileString("Settings","LDS_PATH",lds_path);
	//授权管理
	DWORD version[2]={0,20190430};	//版本发布日期
	BYTE* byteVer=(BYTE*)version;
	byteVer[0]=1;
	byteVer[1]=2;
	byteVer[2]=0;
	byteVer[3]=0;	
	char default_file[MAX_PATH];
	strcpy(default_file,lic_file);
	char* separator=SearchChar(default_file,'.',true);
	strcpy(separator,".key");
	DetectSpecifiedHaspKeyFile(default_file);
	BYTE cProductType=14;
	bool passed_liccheck=true;
	CXhChar50 errormsgstr("未指证书读取错误");
	for(int i=0;true;i++)
	{
		DWORD retCode=ImportLicFile(lic_file,cProductType,version);
		if(retCode==0)
		{
			passed_liccheck=false;
			if(GetLicVersion()<1000005)
				errormsgstr.Copy("该证书文件已过时，当前软件版本必须使用新证书！");
			else if(!VerifyValidFunction(LICFUNC::FUNC_IDENTITY_BASIC))
				errormsgstr.Copy("软件缺少合法使用授权!");
			else
			{
				passed_liccheck=true;
				WriteProfileString("Settings","lic_file",lic_file);
			}
			if(!passed_liccheck)
			{
#ifndef _LEGACY_LICENSE
				ExitCurrentDogSession();
#elif defined(_NET_DOG)
				ExitNetLicEnv(m_wDogModule);
#endif
			}
			else
				break;
		}
		else
		{
			if(retCode==-1)
				errormsgstr.Copy("0#加密狗初始化失败!");
			else if(retCode==1)
				errormsgstr.Copy("1#无法打开证书文件");
			else if(retCode==2)
				errormsgstr.Copy("2#证书文件遭到破坏");
			else if(retCode==3||retCode==4)
				errormsgstr.Copy("3#执行目录下不存在证书文件或证书文件与加密狗不匹配");
			else if(retCode==5)
				errormsgstr.Copy("5#证书与加密狗产品授权版本不匹配");
			else if(retCode==6)
				errormsgstr.Copy("6#超出版本使用授权范围");
			else if(retCode==7)
				errormsgstr.Copy("7#超出免费版本升级授权范围");
			else if(retCode==8)
				errormsgstr.Copy("8#证书序号与当前加密狗不一致");
			else if(retCode==9)
				errormsgstr.Copy("9#授权过期，请续借授权");
			else if(retCode==10)
				errormsgstr.Copy("10#程序缺少相应执行权限，请以管理员权限运行程序");
#ifndef _LEGACY_LICENSE
			ExitCurrentDogSession();
#elif defined(_NET_DOG)
			ExitNetLicEnv(m_wDogModule);
#endif
			passed_liccheck=false;
		}
		if(!passed_liccheck)
		{
			DWORD dwRet=ProcessLicenseAuthorize(cProductType,version,execute_path,false,errormsgstr);
			if(dwRet==0)
				return FALSE;
			if(passed_liccheck=(dwRet==1))
				break;	//内部已成功导入证书文件
		}
	}
	if(!passed_liccheck)
	{
#ifndef _LEGACY_LICENSE
		ExitCurrentDogSession();
#elif defined(_NET_DOG)
		ExitNetLicEnv(m_wDogModule);
#endif
		return FALSE;
	}
	//初始化数据模型参数
	CTMDHGim::InitPropHashtable();
	Manager.DisplayProcess=DisplayProcess;
	//
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	// 注册应用程序的文档模板。文档模板
	// 将用作文档、框架窗口和视图之间的连接
	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(IDR_MAINFRAME,
		RUNTIME_CLASS(CTMDHDoc),
		RUNTIME_CLASS(CChildFrame), // 自定义 MDI 子框架
		RUNTIME_CLASS(CTMDHView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);
	m_pDocTemplate=pDocTemplate;
	// 创建主 MDI 框架窗口
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))
	{
		delete pMainFrame;
		return FALSE;
	}
	m_pMainWnd = pMainFrame;
	// 仅当具有后缀时才调用 DragAcceptFiles
	//  在 MDI 应用程序中，这应在设置 m_pMainWnd 之后立即发生

	// 分析标准 shell 命令、DDE、打开文件操作的命令行
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	//启动软件时，不创建新的文档
	cmdInfo.m_nShellCommand=CCommandLineInfo::FileNothing;
	// 调度在命令行中指定的命令。如果
	// 用 /RegServer、/Register、/Unregserver 或 /Unregister 启动应用程序，则返回 FALSE。
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
	// 主窗口已初始化，因此显示它并对其进行更新
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	return TRUE;
}

int CTMDHApp::ExitInstance()
{
	//TODO: 处理可能已添加的附加资源
	AfxOleTerm(FALSE);

	return CWinAppEx::ExitInstance();
}

// CTMDHApp 消息处理程序


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// 用于运行对话框的应用程序命令
void CTMDHApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CTMDHApp 自定义加载/保存方法

void CTMDHApp::PreLoadState()
{
	/*BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_EDIT_MENU);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);*/
}

void CTMDHApp::LoadCustomState()
{
}

void CTMDHApp::SaveCustomState()
{
}
BOOL CTMDHApp::SaveState(LPCTSTR lpszSectionName/*=NULL*/, CFrameImpl* pFrameImpl/*=NULL*/)
{
	/*BOOL bRetCode = CWinAppEx::SaveState(lpszSectionName, pFrameImpl);
	if (bRetCode)
	{	//删除注册表中菜单相关内容 wht 14-11-26
		HKEY hParentKey = GetSectionKey(m_strRegSection);
		if (hParentKey)
		{
			CString strMenuBarSection;
			strMenuBarSection.Format("BasePane-%d0", m_nMainMenuID);
			DelRegTree(hParentKey, strMenuBarSection);
			strMenuBarSection.Format("Pane-%d0", m_nMainMenuID);
			DelRegTree(hParentKey, strMenuBarSection);
			RegCloseKey(hParentKey);
		}
	}*/
	return FALSE;
}

// CTMDHApp 消息处理程序
void CTMDHApp::OnFileOpen()
{
	CMainFrame* pMainFrm = (CMainFrame*)GetMainWnd();
	if (pMainFrm == NULL)
		return;
	CXhChar500 filter;
	filter.Append("铁塔设计绘图一体化软件(*.lds)|*.lds|");
	filter.Append("铁塔模型信息展示文件(*.tid)|*.tid|");
	filter.Append("国网移交几何模型文件(*.mod)|*.mod|");
	filter.Append("国网三维设计模型文件(*.gim)|*.gim|");
	filter.Append("所有文件(*.*)|*.*||");
	CFileDialog dlg(TRUE, "lds", "LDS", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, filter);
	if (dlg.DoModal() != IDOK)
		return;
	CTMDHPageDlg* pPage = pMainFrm->GetTMDHPage();
	CString sExtName = dlg.GetFileExt();
	if (sExtName.CompareNoCase("lds") == 0 || sExtName.CompareNoCase("LDS") == 0)
		pPage->AppendLdsFile(dlg.GetPathName());
	else if (sExtName.CompareNoCase("tid") == 0 || sExtName.CompareNoCase("TID") == 0)
		pPage->AppendTidFile(dlg.GetPathName());
	else if (sExtName.CompareNoCase("mod") == 0 || sExtName.CompareNoCase("MOD") == 0)
		pPage->AppendModFile(dlg.GetPathName());
	else if (sExtName.CompareNoCase("gim") == 0 || sExtName.CompareNoCase("GIM") == 0)
		pPage->AppendGimFile(dlg.GetPathName());
}


