
// TPSM.cpp : ����Ӧ�ó��������Ϊ��
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

// Ψһ��һ�� CTPSMApp ����
CTPSMApp theApp;
//////////////////////////////////////////////////////////////////////////
//��̬����
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

// CTPSMApp ��ʼ��
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
	// �����ļ��ı�׼�ĵ�����
	//ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	//ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
	//
	ON_COMMAND(ID_FILE_NEW, OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_EXPORT_TID, OnExportTidData)
	ON_COMMAND(ID_EXPORT_MOD, OnExportModData)
END_MESSAGE_MAP()

// CTPSMApp ����
CTPSMApp::CTPSMApp()
{
	m_pDocTemplate=NULL;
	m_bHiColorIcons = TRUE;

	// ֧����������������
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_ALL_ASPECTS;
	// TODO: ������Ӧ�ó��� ID �ַ����滻ΪΨһ�� ID �ַ�����������ַ�����ʽ
	//Ϊ CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("TPSM.AppID.NoVersion"));

	// TODO: �ڴ˴���ӹ�����룬
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
	{	//ͨ���ĵ�ģ�崴�����ĵ���ͬʱ������ܺ���ͼ
		pActiveDoc=(CTPSMDoc*)m_pDocTemplate->OpenDocumentFile(NULL);
		pActiveDoc->m_pModel=pModel;
		pActiveDoc->SetTitle(pModel->m_sTower);
		pActiveDoc->SetPathName(pModel->m_sFullPath,FALSE);
	}
	//ˢ������ģ��
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
// CTPSMApp ��ʼ��
BOOL CTPSMApp::InitInstance()
{
	// ���һ�������� Windows XP �ϵ�Ӧ�ó����嵥ָ��Ҫ
	// ʹ�� ComCtl32.dll �汾 6 ����߰汾�����ÿ��ӻ���ʽ��
	//����Ҫ InitCommonControlsEx()�����򣬽��޷��������ڡ�
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// ��������Ϊ��������Ҫ��Ӧ�ó�����ʹ�õ�
	// �����ؼ��ࡣ
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();
	// ��ʼ�� OLE ��
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();
	EnableTaskbarInteraction();
	// ʹ�� RichEdit �ؼ���Ҫ  AfxInitRichEdit2()	
	//AfxInitRichEdit2();
	if(!AfxInitRichEdit())
		return FALSE;
	// ��׼��ʼ��
	// ���δʹ����Щ���ܲ�ϣ����С
	// ���տ�ִ���ļ��Ĵ�С����Ӧ�Ƴ�����
	// ����Ҫ���ض���ʼ������
	// �������ڴ洢���õ�ע�����
	// TODO: Ӧ�ʵ��޸ĸ��ַ�����
	// �����޸�Ϊ��˾����֯��
	SetRegistryKey(_T("Xerofox"));
	LoadStdProfileSettings(4);  // ���ر�׼ INI �ļ�ѡ��(���� MRU)
	InitContextMenuManager();
	InitKeyboardManager();
	InitTooltipManager();
	//��Ȩ����
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
			AfxMessageBox("�״�ʹ�ã���δָ����֤���ļ���");
		else if(retCode==-1)
			AfxMessageBox("��������ʼ��ʧ��");
		else if(retCode==1)
			AfxMessageBox("1#�޷���֤���ļ�");
		else if(retCode==2)
			AfxMessageBox("2#֤���ļ��⵽�ƻ�");
		else if(retCode==3)
			AfxMessageBox("3#֤������ܹ���ƥ��");
		else if(retCode==4)
			AfxMessageBox("4#��Ȩ֤��ļ��ܰ汾����");
		else if(retCode==5)
			AfxMessageBox("5#֤������ܹ���Ʒ��Ȩ�汾��ƥ��");
		else if(retCode==6)
			AfxMessageBox("6#�����汾ʹ����Ȩ��Χ");
		else if(retCode==7)
			AfxMessageBox("7#������Ѱ汾������Ȩ��Χ");
		else if(retCode==8)
			AfxMessageBox("8#֤������뵱ǰ���ܹ���һ��");
		else if(retCode==9)
			AfxMessageBox("9#��Ȩ���ڣ���������Ȩ");
		else if(retCode==10)
			AfxMessageBox("10#����ȱ����Ӧִ��Ȩ�ޣ����Թ���ԱȨ�����г���");
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

	// ע��Ӧ�ó�����ĵ�ģ�塣�ĵ�ģ��
	// �������ĵ�����ܴ��ں���ͼ֮�������
	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(IDR_TPSMTYPE,
		RUNTIME_CLASS(CTPSMDoc),
		RUNTIME_CLASS(CChildFrame), // �Զ��� MDI �ӿ��
		RUNTIME_CLASS(CTPSMView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);
	m_pDocTemplate=pDocTemplate;
	// ������ MDI ��ܴ���
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))
	{
		delete pMainFrame;
		return FALSE;
	}
	m_pMainWnd = pMainFrame;
	// �������к�׺ʱ�ŵ��� DragAcceptFiles
	//  �� MDI Ӧ�ó����У���Ӧ������ m_pMainWnd ֮����������

	// ������׼ shell ���DDE�����ļ�������������
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	//�������ʱ���������µ��ĵ�
	cmdInfo.m_nShellCommand=CCommandLineInfo::FileNothing;
	// ��������������ָ����������
	// �� /RegServer��/Register��/Unregserver �� /Unregister ����Ӧ�ó����򷵻� FALSE��
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
	// �������ѳ�ʼ���������ʾ����������и���
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	return TRUE;
}

int CTPSMApp::ExitInstance()
{
	//TODO: �����������ӵĸ�����Դ
	AfxOleTerm(FALSE);

	return CWinAppEx::ExitInstance();
}

// CTPSMApp ��Ϣ�������


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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

// �������жԻ����Ӧ�ó�������
void CTPSMApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CTPSMApp �Զ������/���淽��

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

// CTPSMApp ��Ϣ�������
void CTPSMApp::OnFileOpen()
{
	CXhChar500 filter("���͹滮��άģ���ļ�(*.tpi)|*.tpi");
	filter.Append("|������ƻ�ͼһ�廯���(*.lds)|*.lds");
	filter.Append("|������ά����ģ���ļ�(*.tid)|*.tid");
	filter.Append("|�����ƽ�����ģ���ļ�(*.mod)|*.mod");
	filter.Append("|�����ļ�(*.*)|*.*||");
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
		AfxMessageBox("����ģ������Ϊ��!");
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
				logerr.Log("%s�滮����ģ������Ϊ��!",(char*)pModel->m_sTower);
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
		AfxMessageBox("����ģ������Ϊ��!");
		return;
	}
	CString sFolder;
	if(!InvokeFolderPickerDlg(sFolder))
		return;
	sFolder.Append("\\MOD");
	MakeDirectory(sFolder);
	CLogErrorLife logErrLife;
	DisplayProcess(0,"���������ƽ�����ģ���ļ�.....");
	for(CTPSMProject* pPrj=Manager.EnumFirstPrj();pPrj;pPrj=Manager.EnumNextPrj())
	{
		for(CTPSMModel* pModel=pPrj->EnumFirstModel();pModel;pModel=pPrj->EnumNextModel())
		{
			index++;
			DisplayProcess(ftoi(100*index/nNum),"���������ƽ�����ģ���ļ�.....");
			if(pModel->m_xTower.Parts.GetNodeNum()<=0)
			{
				logerr.Log("%s�滮����ģ������Ϊ��!",(char*)pModel->m_sTower);
				continue;
			}
			CXhChar200 sFilename("%s\\%s.mod",sFolder.GetBuffer(),(char*)pModel->m_sTower);
			pModel->CreateModFile(sFilename);
		}
	}
	DisplayProcess(100,"���������ƽ�����ģ���ļ�.....");
}
void CTPSMApp::ExportDxfData()
{
	int nNum=Manager.TowerNum(),index=0;
	if(nNum<=0)
	{
		AfxMessageBox("����ģ������Ϊ��!");
		return;
	}
	CString sFolder;
	if(!InvokeFolderPickerDlg(sFolder))
		return;
	sFolder.Append("\\DXF");
	MakeDirectory(sFolder);
	CLogErrorLife logErrLife;
	DisplayProcess(0,"����Dxf�ļ�.....");
	for(CTPSMProject* pPrj=Manager.EnumFirstPrj();pPrj;pPrj=Manager.EnumNextPrj())
	{
		for(CTPSMModel* pModel=pPrj->EnumFirstModel();pModel;pModel=pPrj->EnumNextModel())
		{
			index++;
			DisplayProcess(ftoi(100*index/nNum),"����Dxf�ļ�.....");
			if(pModel->m_xTower.Parts.GetNodeNum()<=0)
			{
				logerr.Log("%s�滮����ģ������Ϊ��!",(char*)pModel->m_sTower);
				continue;
			}
			CXhChar200 sFilename("%s\\%s.dxf",sFolder.GetBuffer(),(char*)pModel->m_sTower);
			pModel->CreateDxfFile(sFilename);
		}
	}
	DisplayProcess(100,"����Dxf�ļ�.....");
}
void CTPSMApp::Export3dsData()
{
	int nNum=Manager.TowerNum(),index=0;
	if(nNum<=0)
	{
		AfxMessageBox("����ģ������Ϊ��!");
		return;
	}
	CString sFolder;
	if(!InvokeFolderPickerDlg(sFolder))
		return;
	sFolder.Append("\\3DS");
	MakeDirectory(sFolder);
	CLogErrorLife logErrLife;
	DisplayProcess(0,"����3ds�ļ�.....");
	for(CTPSMProject* pPrj=Manager.EnumFirstPrj();pPrj;pPrj=Manager.EnumNextPrj())
	{
		for(CTPSMModel* pModel=pPrj->EnumFirstModel();pModel;pModel=pPrj->EnumNextModel())
		{
			index++;
			DisplayProcess(ftoi(100*index/nNum),"����3ds�ļ�.....");
			if(pModel->m_xTower.Parts.GetNodeNum()<=0)
			{
				logerr.Log("%s�滮����ģ������Ϊ��!",(char*)pModel->m_sTower);
				continue;
			}
			CXhChar200 sFilename("%s\\%s.3ds",sFolder.GetBuffer(),(char*)pModel->m_sTower);
			pModel->Create3dsFile(sFilename);
		}
	}
	DisplayProcess(100,"����3ds�ļ�.....");
}

void CTPSMApp::ExportLdsData()
{
	int nNum=Manager.TowerNum(),index=0;
	if(nNum<=0)
	{
		AfxMessageBox("����ģ������Ϊ��!");
		return;
	}
	CString sFolder;
	if(!InvokeFolderPickerDlg(sFolder))
		return;
	sFolder.Append("\\LDS");
	MakeDirectory(sFolder);
	CLogErrorLife logErrLife;
	DisplayProcess(0,"����LDS�ļ�.....");
	for(CTPSMProject* pPrj=Manager.EnumFirstPrj();pPrj;pPrj=Manager.EnumNextPrj())
	{
		for(CTPSMModel* pModel=pPrj->EnumFirstModel();pModel;pModel=pPrj->EnumNextModel())
		{
			index++;
			DisplayProcess(ftoi(100*index/nNum),"����LDS�ļ�.....");
			if(pModel->m_xTower.Parts.GetNodeNum()<=0)
			{
				logerr.Log("%s�滮����ģ������Ϊ��!",(char*)pModel->m_sTower);
				continue;
			}
			CXhChar200 sFilename("%s\\%s.LDS",sFolder.GetBuffer(),(char*)pModel->m_sTower);
			pModel->CreateLDSFile(sFilename);
		}
	}
	DisplayProcess(100,"����LDS�ļ�.....");
}
