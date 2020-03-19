
// TPSM.cpp : 定义应用程序的类行为。
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "TPSM.h"
#include "ChildFrm.h"
#include "direct.h"
#include "ProcBarDlg.h"
#include "folder_dialog.h"
#include "GlobalFunc.h"
#include "TowerTemplateDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 唯一的一个 CTPSMApp 对象
CTPSMApp theApp;
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

// CTPSMApp 初始化
BOOL GetLicFilePathFromReg(char* licfile_path)
{
	const char* szRegItemKey="lic_file2";
	char sPath[MAX_PATH]="",sSubKey[MAX_PATH]="";
	strcpy(sSubKey,"Software\\Xerofox\\LDS\\Settings");
	//
	HKEY hKey;
	RegOpenKeyEx(HKEY_CURRENT_USER,sSubKey,0,KEY_READ,&hKey);
	DWORD dwDataType,dwLength=MAX_PATH;
	if(RegQueryValueEx(hKey,_T(szRegItemKey),NULL,&dwDataType,(BYTE*)&sPath[0],&dwLength)== ERROR_SUCCESS)
		strcpy(licfile_path,sPath);
	RegCloseKey(hKey);
	if(strlen(licfile_path)>1)
		return TRUE;
	return FALSE;
}
//////////////////////////////////////////////////////////////////////////
// CTPSMApp
BEGIN_MESSAGE_MAP(CTPSMApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CTPSMApp::OnAppAbout)
	// 基于文件的标准文档命令
	//ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	//ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
	//
	ON_COMMAND(ID_FILE_NEW, OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_EXPORT_TID, OnExportTidData)
	ON_COMMAND(ID_EXPORT_MOD, OnExportModData)
END_MESSAGE_MAP()

// CTPSMApp 构造
CTPSMApp::CTPSMApp()
{
	m_pDocTemplate=NULL;
	m_bHiColorIcons = TRUE;

	// 支持重新启动管理器
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_ALL_ASPECTS;
	// TODO: 将以下应用程序 ID 字符串替换为唯一的 ID 字符串；建议的字符串格式
	//为 CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("TPSM.AppID.NoVersion"));

	// TODO: 在此处添加构造代码，
	strcpy(execute_path,"");
}
CTPSMDoc* CTPSMApp::GetActiveDoc()
{
	CMDIChildWnd* pChildWnd=((CMainFrame*)m_pMainWnd)->MDIGetActive();
	if(pChildWnd)
	{
		CDocument *pDoc=pChildWnd->GetActiveDocument();
		if(pDoc->IsKindOf(RUNTIME_CLASS(CTPSMDoc)))
			return (CTPSMDoc*)pDoc;
	}
	return NULL;
}
CTPSMView* CTPSMApp::GetActiveView()
{
	CTPSMDoc* pTpsmDoc=GetActiveDoc();
	if(pTpsmDoc)
		return (CTPSMView*)pTpsmDoc->GetView(RUNTIME_CLASS(CTPSMView));
	else
		return NULL;
}
BOOL CTPSMApp::SetActiveDoc(CTPSMModel* pModel,BOOL bDraw/*=TRUE*/)
{
	POSITION doc_pos=m_pDocTemplate->GetFirstDocPosition();
	CTPSMDoc* pActiveDoc=NULL;
	while(doc_pos!=NULL)
	{
		CDocument *pDoc=m_pDocTemplate->GetNextDoc(doc_pos);
		if(pDoc!=NULL && pDoc->IsKindOf(RUNTIME_CLASS(CTPSMDoc)))
		{
			CTPSMDoc* pTpsmDoc=(CTPSMDoc*)pDoc;
			CTPSMView* pTpsmView=(CTPSMView*)pTpsmDoc->GetView(RUNTIME_CLASS(CTPSMView));
			if(pTpsmDoc->m_pModel==pModel && pTpsmView)
			{
				((CMDIFrameWnd *)m_pMainWnd)->MDIActivate(pTpsmView->GetParentFrame());
				pActiveDoc=pTpsmDoc;
				break;
			}
		}
	}
	if(pActiveDoc==NULL)
	{	//通过文档模板创建新文档，同时创建框架和视图
		pActiveDoc=(CTPSMDoc*)m_pDocTemplate->OpenDocumentFile(NULL);
		pActiveDoc->m_pModel=pModel;
		pActiveDoc->SetTitle(pModel->m_sTower);
		pActiveDoc->SetPathName(pModel->m_sFullPath,FALSE);
	}
	//刷新塔型模型
	CTPSMView* pActiveView=(CTPSMView*)pActiveDoc->GetView(RUNTIME_CLASS(CTPSMView));
	if(bDraw && pActiveView)
		pActiveView->Refresh();
	return TRUE;
}
void CTPSMApp::DeleteDoc(CTPSMModel* pModel)
{
	POSITION doc_pos=m_pDocTemplate->GetFirstDocPosition();
	while(doc_pos!=NULL)
	{
		CDocument *pDoc=m_pDocTemplate->GetNextDoc(doc_pos);
		if(pDoc!=NULL && pDoc->IsKindOf(RUNTIME_CLASS(CTPSMDoc)))
		{
			CTPSMDoc* pTpsmDoc=(CTPSMDoc*)pDoc;
			if(pTpsmDoc->m_pModel!=pModel)
				continue;
			CTPSMView* pTpsmView=(CTPSMView*)pTpsmDoc->GetView(RUNTIME_CLASS(CTPSMView));
			if(pTpsmView==NULL)
				continue;
			CMDIChildWnd* pChildWnd=(CMDIChildWnd*)pTpsmView->GetParentFrame();
			pChildWnd->MDIDestroy();
			break;
		}
	}
}
void CTPSMApp::GetAppVersion(char *file_version,char *product_version)
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
// CTPSMApp 初始化
BOOL CTPSMApp::InitInstance()
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
	//AfxInitRichEdit2();
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
	//授权管理
	DWORD version[2]={0,20170615};
	BYTE* pByteVer=(BYTE*)version;
	pByteVer[0]=1;
	pByteVer[1]=2;
	pByteVer[2]=0;
	pByteVer[3]=0;
	char APP_PATH[MAX_PATH],lic_file[MAX_PATH];
	GetAppPath(APP_PATH);
	sprintf(lic_file,"%sLDS.lic",APP_PATH);
	ULONG retCode=ImportLicFile(lic_file,PRODUCT_LDS,version);
	if(retCode!=0)
	{
		if(retCode==-2)
			AfxMessageBox("首次使用，还未指定过证书文件！");
		else if(retCode==-1)
			AfxMessageBox("加密锁初始化失败");
		else if(retCode==1)
			AfxMessageBox("1#无法打开证书文件");
		else if(retCode==2)
			AfxMessageBox("2#证书文件遭到破坏");
		else if(retCode==3)
			AfxMessageBox("3#证书与加密狗不匹配");
		else if(retCode==4)
			AfxMessageBox("4#授权证书的加密版本不对");
		else if(retCode==5)
			AfxMessageBox("5#证书与加密狗产品授权版本不匹配");
		else if(retCode==6)
			AfxMessageBox("6#超出版本使用授权范围");
		else if(retCode==7)
			AfxMessageBox("7#超出免费版本升级授权范围");
		else if(retCode==8)
			AfxMessageBox("8#证书序号与当前加密狗不一致");
		else if(retCode==9)
			AfxMessageBox("9#授权过期，请续借授权");
		else if(retCode==10)
			AfxMessageBox("10#程序缺少相应执行权限，请以管理员权限运行程序");
		return FALSE;
	}
	CLDSNode::InitNodePropHashtable();
	CString sys_path,lds_path;
	sys_path=APP_PATH;
	sys_path+="Sys\\";
	lds_path=APP_PATH;
	lds_path+="lds\\";
	strcpy(execute_path,APP_PATH);
	WriteProfileString("Settings","lic_file",lic_file);
	WriteProfileString("Settings","SYS_PATH",sys_path);
	WriteProfileString("Settings","LDS_PATH",lds_path);
	//
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	// 注册应用程序的文档模板。文档模板
	// 将用作文档、框架窗口和视图之间的连接
	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(IDR_TPSMTYPE,
		RUNTIME_CLASS(CTPSMDoc),
		RUNTIME_CLASS(CChildFrame), // 自定义 MDI 子框架
		RUNTIME_CLASS(CTPSMView));
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

int CTPSMApp::ExitInstance()
{
	//TODO: 处理可能已添加的附加资源
	AfxOleTerm(FALSE);

	return CWinAppEx::ExitInstance();
}

// CTPSMApp 消息处理程序


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
void CTPSMApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CTPSMApp 自定义加载/保存方法

void CTPSMApp::PreLoadState()
{
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_EDIT_MENU);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
}

void CTPSMApp::LoadCustomState()
{
}

void CTPSMApp::SaveCustomState()
{
}

// CTPSMApp 消息处理程序
void CTPSMApp::OnFileOpen()
{
	CXhChar500 filter("塔型规划三维模型文件(*.tpi)|*.tpi");
	filter.Append("|铁塔设计绘图一体化软件(*.lds)|*.lds");
	filter.Append("|铁塔三维数据模型文件(*.tid)|*.tid");
	filter.Append("|国网移交几何模型文件(*.mod)|*.mod");
	filter.Append("|所有文件(*.*)|*.*||");
	CFileDialog dlg(TRUE,"lds","LDS",OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,filter);
	if(dlg.DoModal()!=IDOK)
		return;
	CWinAppEx::OpenDocumentFile(dlg.GetPathName());
}
void CTPSMApp::OnFileNew()
{
	CProjectTreeDlg* pDlg=((CMainFrame*)m_pMainWnd)->GetTowerTreePage();
	if(pDlg)
		pDlg->NewTowerModel(pDlg->m_hPrjTowerModel);
}
void CTPSMApp::OnExportTidData()
{
	ExportTidData();
}
void CTPSMApp::OnExportModData()
{
	ExportModData();
}
void CTPSMApp::ExportTidData()
{
	if(Manager.TowerNum()<=0)
	{
		AfxMessageBox("塔型模型数据为空!");
		return;
	}
	CString sFolder;
	if(!InvokeFolderPickerDlg(sFolder))
		return;
	sFolder.Append("\\TID");
	MakeDirectory(sFolder);
	CLogErrorLife logErrLife;
	for(CTPSMProject* pPrj=Manager.EnumFirstPrj();pPrj;pPrj=Manager.EnumNextPrj())
	{
		for(CTPSMModel* pModel=pPrj->EnumFirstModel();pModel;pModel=pPrj->EnumNextModel())
		{
			if(pModel->m_xTower.Parts.GetNodeNum()<=0)
			{
				logerr.Log("%s规划塔型模型数据为空!",(char*)pModel->m_sTower);
				continue;
			}
			if(pModel->m_xTower.DisplayProcess==NULL)
				pModel->m_xTower.DisplayProcess=DisplayProcess;
			CXhChar200 sFilename("%s\\%s.tid",sFolder.GetBuffer(),(char*)pModel->m_sTower);
			pModel->m_xTower.ExportTowerSolidDataExchangeFile(sFilename);
		}
	}
}
void CTPSMApp::ExportModData()
{
	int nNum=Manager.TowerNum(),index=0;
	if(nNum<=0)
	{
		AfxMessageBox("塔型模型数据为空!");
		return;
	}
	CString sFolder;
	if(!InvokeFolderPickerDlg(sFolder))
		return;
	sFolder.Append("\\MOD");
	MakeDirectory(sFolder);
	CLogErrorLife logErrLife;
	DisplayProcess(0,"导出国网移交几何模型文件.....");
	for(CTPSMProject* pPrj=Manager.EnumFirstPrj();pPrj;pPrj=Manager.EnumNextPrj())
	{
		for(CTPSMModel* pModel=pPrj->EnumFirstModel();pModel;pModel=pPrj->EnumNextModel())
		{
			index++;
			DisplayProcess(ftoi(100*index/nNum),"导出国网移交几何模型文件.....");
			if(pModel->m_xTower.Parts.GetNodeNum()<=0)
			{
				logerr.Log("%s规划塔型模型数据为空!",(char*)pModel->m_sTower);
				continue;
			}
			CXhChar200 sFilename("%s\\%s.mod",sFolder.GetBuffer(),(char*)pModel->m_sTower);
			pModel->CreateModFile(sFilename);
		}
	}
	DisplayProcess(100,"导出国网移交几何模型文件.....");
}
void CTPSMApp::ExportDxfData()
{
	int nNum=Manager.TowerNum(),index=0;
	if(nNum<=0)
	{
		AfxMessageBox("塔型模型数据为空!");
		return;
	}
	CString sFolder;
	if(!InvokeFolderPickerDlg(sFolder))
		return;
	sFolder.Append("\\DXF");
	MakeDirectory(sFolder);
	CLogErrorLife logErrLife;
	DisplayProcess(0,"导出Dxf文件.....");
	for(CTPSMProject* pPrj=Manager.EnumFirstPrj();pPrj;pPrj=Manager.EnumNextPrj())
	{
		for(CTPSMModel* pModel=pPrj->EnumFirstModel();pModel;pModel=pPrj->EnumNextModel())
		{
			index++;
			DisplayProcess(ftoi(100*index/nNum),"导出Dxf文件.....");
			if(pModel->m_xTower.Parts.GetNodeNum()<=0)
			{
				logerr.Log("%s规划塔型模型数据为空!",(char*)pModel->m_sTower);
				continue;
			}
			CXhChar200 sFilename("%s\\%s.dxf",sFolder.GetBuffer(),(char*)pModel->m_sTower);
			pModel->CreateDxfFile(sFilename);
		}
	}
	DisplayProcess(100,"导出Dxf文件.....");
}
void CTPSMApp::Export3dsData()
{
	int nNum=Manager.TowerNum(),index=0;
	if(nNum<=0)
	{
		AfxMessageBox("塔型模型数据为空!");
		return;
	}
	CString sFolder;
	if(!InvokeFolderPickerDlg(sFolder))
		return;
	sFolder.Append("\\3DS");
	MakeDirectory(sFolder);
	CLogErrorLife logErrLife;
	DisplayProcess(0,"导出3ds文件.....");
	for(CTPSMProject* pPrj=Manager.EnumFirstPrj();pPrj;pPrj=Manager.EnumNextPrj())
	{
		for(CTPSMModel* pModel=pPrj->EnumFirstModel();pModel;pModel=pPrj->EnumNextModel())
		{
			index++;
			DisplayProcess(ftoi(100*index/nNum),"导出3ds文件.....");
			if(pModel->m_xTower.Parts.GetNodeNum()<=0)
			{
				logerr.Log("%s规划塔型模型数据为空!",(char*)pModel->m_sTower);
				continue;
			}
			CXhChar200 sFilename("%s\\%s.3ds",sFolder.GetBuffer(),(char*)pModel->m_sTower);
			pModel->Create3dsFile(sFilename);
		}
	}
	DisplayProcess(100,"导出3ds文件.....");
}

void CTPSMApp::ExportLdsData()
{
	int nNum=Manager.TowerNum(),index=0;
	if(nNum<=0)
	{
		AfxMessageBox("塔型模型数据为空!");
		return;
	}
	CString sFolder;
	if(!InvokeFolderPickerDlg(sFolder))
		return;
	sFolder.Append("\\LDS");
	MakeDirectory(sFolder);
	CLogErrorLife logErrLife;
	DisplayProcess(0,"导出LDS文件.....");
	for(CTPSMProject* pPrj=Manager.EnumFirstPrj();pPrj;pPrj=Manager.EnumNextPrj())
	{
		for(CTPSMModel* pModel=pPrj->EnumFirstModel();pModel;pModel=pPrj->EnumNextModel())
		{
			index++;
			DisplayProcess(ftoi(100*index/nNum),"导出LDS文件.....");
			if(pModel->m_xTower.Parts.GetNodeNum()<=0)
			{
				logerr.Log("%s规划塔型模型数据为空!",(char*)pModel->m_sTower);
				continue;
			}
			CXhChar200 sFilename("%s\\%s.LDS",sFolder.GetBuffer(),(char*)pModel->m_sTower);
			pModel->CreateLDSFile(sFilename);
		}
	}
	DisplayProcess(100,"导出LDS文件.....");
}
