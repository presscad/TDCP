
// TMDH.cpp : ����Ӧ�ó��������Ϊ��
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

// Ψһ��һ�� CTMDHApp ����
CTMDHApp theApp;
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
		line_txt.Replace("��","=");
		char* final=SearchChar(line_txt,';');
		if(final!=NULL)
			*final=0;
		char *skey=strtok(line_txt," =,");
		//��������
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
	// �����ļ��ı�׼�ĵ�����
	//ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
END_MESSAGE_MAP()


// CTMDHApp ����

CTMDHApp::CTMDHApp()
{
	m_bHiColorIcons = TRUE;
	m_pDocTemplate=NULL;
	// ֧����������������
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_ALL_ASPECTS;
#ifdef _MANAGED
	// ���Ӧ�ó��������ù�����������ʱ֧��(/clr)�����ģ���:
	//     1) �����д˸������ã�������������������֧�ֲ�������������
	//     2) ��������Ŀ�У������밴������˳���� System.Windows.Forms ������á�
	System::Windows::Forms::Application::SetUnhandledExceptionMode(System::Windows::Forms::UnhandledExceptionMode::ThrowException);
#endif

	// TODO: ������Ӧ�ó��� ID �ַ����滻ΪΨһ�� ID �ַ�����������ַ�����ʽ
	//Ϊ CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("TMDH.AppID.NoVersion"));

	// TODO: �ڴ˴���ӹ�����룬
	strcpy(execute_path,"");
}

// CTMDHApp ��ʼ��
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
	{	//ͨ���ĵ�ģ�崴�����ĵ���ͬʱ������ܺ���ͼ
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
	//ˢ������ģ��
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
	// AfxInitRichEdit2();
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
	//ע�������ӹؼ�·��
	char APP_PATH[MAX_PATH],lic_file[MAX_PATH],sys_path[MAX_PATH],lds_path[MAX_PATH];
	GetAppPath(APP_PATH);
	strcpy(execute_path,APP_PATH);
	sprintf(lic_file,"%sGIM.lic",APP_PATH);
	WriteProfileString("Settings","lic_file",lic_file);
	sprintf(sys_path,"%sSys\\",APP_PATH);
	WriteProfileString("Settings","SYS_PATH",sys_path);
	sprintf(lds_path,"%slds\\",APP_PATH);	
	WriteProfileString("Settings","LDS_PATH",lds_path);
	//��Ȩ����
	DWORD version[2]={0,20190430};	//�汾��������
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
	CXhChar50 errormsgstr("δָ֤���ȡ����");
	for(int i=0;true;i++)
	{
		DWORD retCode=ImportLicFile(lic_file,cProductType,version);
		if(retCode==0)
		{
			passed_liccheck=false;
			if(GetLicVersion()<1000005)
				errormsgstr.Copy("��֤���ļ��ѹ�ʱ����ǰ����汾����ʹ����֤�飡");
			else if(!VerifyValidFunction(LICFUNC::FUNC_IDENTITY_BASIC))
				errormsgstr.Copy("���ȱ�ٺϷ�ʹ����Ȩ!");
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
				errormsgstr.Copy("0#���ܹ���ʼ��ʧ��!");
			else if(retCode==1)
				errormsgstr.Copy("1#�޷���֤���ļ�");
			else if(retCode==2)
				errormsgstr.Copy("2#֤���ļ��⵽�ƻ�");
			else if(retCode==3||retCode==4)
				errormsgstr.Copy("3#ִ��Ŀ¼�²�����֤���ļ���֤���ļ�����ܹ���ƥ��");
			else if(retCode==5)
				errormsgstr.Copy("5#֤������ܹ���Ʒ��Ȩ�汾��ƥ��");
			else if(retCode==6)
				errormsgstr.Copy("6#�����汾ʹ����Ȩ��Χ");
			else if(retCode==7)
				errormsgstr.Copy("7#������Ѱ汾������Ȩ��Χ");
			else if(retCode==8)
				errormsgstr.Copy("8#֤������뵱ǰ���ܹ���һ��");
			else if(retCode==9)
				errormsgstr.Copy("9#��Ȩ���ڣ���������Ȩ");
			else if(retCode==10)
				errormsgstr.Copy("10#����ȱ����Ӧִ��Ȩ�ޣ����Թ���ԱȨ�����г���");
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
				break;	//�ڲ��ѳɹ�����֤���ļ�
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
	//��ʼ������ģ�Ͳ���
	CTMDHGim::InitPropHashtable();
	Manager.DisplayProcess=DisplayProcess;
	//
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	// ע��Ӧ�ó�����ĵ�ģ�塣�ĵ�ģ��
	// �������ĵ�����ܴ��ں���ͼ֮�������
	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(IDR_MAINFRAME,
		RUNTIME_CLASS(CTMDHDoc),
		RUNTIME_CLASS(CChildFrame), // �Զ��� MDI �ӿ��
		RUNTIME_CLASS(CTMDHView));
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

int CTMDHApp::ExitInstance()
{
	//TODO: �����������ӵĸ�����Դ
	AfxOleTerm(FALSE);

	return CWinAppEx::ExitInstance();
}

// CTMDHApp ��Ϣ�������


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
void CTMDHApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CTMDHApp �Զ������/���淽��

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
	{	//ɾ��ע����в˵�������� wht 14-11-26
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

// CTMDHApp ��Ϣ�������
void CTMDHApp::OnFileOpen()
{
	CMainFrame* pMainFrm = (CMainFrame*)GetMainWnd();
	if (pMainFrm == NULL)
		return;
	CXhChar500 filter;
	filter.Append("������ƻ�ͼһ�廯���(*.lds)|*.lds|");
	filter.Append("����ģ����Ϣչʾ�ļ�(*.tid)|*.tid|");
	filter.Append("�����ƽ�����ģ���ļ�(*.mod)|*.mod|");
	filter.Append("������ά���ģ���ļ�(*.gim)|*.gim|");
	filter.Append("�����ļ�(*.*)|*.*||");
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


