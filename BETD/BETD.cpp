
// BETD.cpp : 定义应用程序的类行为。
//

#include "stdafx.h"
#include "BETD.h"
#include "ModelDataDlg.h"
#include "direct.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 唯一的一个 CBETDApp 对象
CBETDApp theApp;
char APP_PATH[MAX_PATH];

//////////////////////////////////////////////////////////////////////////
//静态函数
void GetSysPath(char* startPath)
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
//////////////////////////////////////////////////////////////////////////
// CBETDApp
BEGIN_MESSAGE_MAP(CBETDApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

// CBETDApp 构造
CBETDApp::CBETDApp()
{
	// 支持重新启动管理器
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}
// CBETDApp 初始化
BOOL CBETDApp::InitInstance()
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

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// 创建 shell 管理器，以防对话框包含
	// 任何 shell 树视图控件或 shell 列表视图控件。
	CShellManager *pShellManager = new CShellManager;

	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO: 应适当修改该字符串，
	// 例如修改为公司或组织名
	SetRegistryKey(_T("Xerofox"));
#ifndef _HANDOVER_TO_CLIENT_
	//授权管理
	DWORD version[2]={0,20170615};
	BYTE* pByteVer=(BYTE*)version;
	pByteVer[0]=1;
	pByteVer[1]=2;
	pByteVer[2]=0;
	pByteVer[3]=0;
	char lic_file[MAX_PATH];
	GetSysPath(APP_PATH);
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
#endif
	//
	CModelDataDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: 在此放置处理何时用
		//  “确定”来关闭对话框的代码
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: 在此放置处理何时用
		//  “取消”来关闭对话框的代码
	}

	// 删除上面创建的 shell 管理器。
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// 由于对话框已关闭，所以将返回 FALSE 以便退出应用程序，
	//  而不是启动应用程序的消息泵。
	return FALSE;
}

