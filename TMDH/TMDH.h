
// TMDH.h : TMDH Ӧ�ó������ͷ�ļ�
//
#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"       // ������
#include "TMDHDoc.h"
#include "TMDHView.h"
#include "MainFrm.h"
#include "TMDHModel.h"
// CTMDHApp:
// �йش����ʵ�֣������ TMDH.cpp
//

class CTMDHApp : public CWinAppEx
{
public:
	CMultiDocTemplate* m_pDocTemplate;
	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;
	char execute_path[MAX_PATH];//��ȡִ���ļ�������Ŀ¼
public:
	CTMDHApp();
//
	CTMDHDoc* GetActiveDoc();
	CTMDHView* GetActiveView();
	BOOL SetActiveDoc(void* pModel,BYTE ciModelFlag=0,BOOL bDraw=TRUE);
	void DeleteDoc(void* pModel);
	void GetAppVersion(char *file_version,char *product_version);
// ��д
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// ʵ��
	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();
	virtual BOOL SaveState(LPCTSTR lpszSectionName = NULL, CFrameImpl* pFrameImpl = NULL);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnAppAbout();
	afx_msg void OnFileOpen();
};

extern CTMDHApp theApp;
