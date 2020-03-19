
// BETD.cpp : ����Ӧ�ó��������Ϊ��
//

#include "stdafx.h"
#include "BETD.h"
#include "ModelDataDlg.h"
#include "direct.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Ψһ��һ�� CBETDApp ����
CBETDApp theApp;
char APP_PATH[MAX_PATH];

//////////////////////////////////////////////////////////////////////////
//��̬����
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

// CBETDApp ����
CBETDApp::CBETDApp()
{
	// ֧����������������
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}
// CBETDApp ��ʼ��
BOOL CBETDApp::InitInstance()
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

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// ���� shell ���������Է��Ի������
	// �κ� shell ����ͼ�ؼ��� shell �б���ͼ�ؼ���
	CShellManager *pShellManager = new CShellManager;

	// ��׼��ʼ��
	// ���δʹ����Щ���ܲ�ϣ����С
	// ���տ�ִ���ļ��Ĵ�С����Ӧ�Ƴ�����
	// ����Ҫ���ض���ʼ������
	// �������ڴ洢���õ�ע�����
	// TODO: Ӧ�ʵ��޸ĸ��ַ�����
	// �����޸�Ϊ��˾����֯��
	SetRegistryKey(_T("Xerofox"));
#ifndef _HANDOVER_TO_CLIENT_
	//��Ȩ����
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
#endif
	//
	CModelDataDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: �ڴ˷��ô����ʱ��
		//  ��ȷ�������رնԻ���Ĵ���
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: �ڴ˷��ô����ʱ��
		//  ��ȡ�������رնԻ���Ĵ���
	}

	// ɾ�����洴���� shell ��������
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// ���ڶԻ����ѹرգ����Խ����� FALSE �Ա��˳�Ӧ�ó���
	//  ����������Ӧ�ó������Ϣ�á�
	return FALSE;
}

