
// TMDH.h : TMDH 应用程序的主头文件
//
#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#include "resource.h"       // 主符号
#include "TMDHDoc.h"
#include "TMDHView.h"
#include "MainFrm.h"
#include "TMDHModel.h"
// CTMDHApp:
// 有关此类的实现，请参阅 TMDH.cpp
//

class CTMDHApp : public CWinAppEx
{
public:
	CMultiDocTemplate* m_pDocTemplate;
	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;
	char execute_path[MAX_PATH];//获取执行文件的启动目录
public:
	CTMDHApp();
//
	CTMDHDoc* GetActiveDoc();
	CTMDHView* GetActiveView();
	BOOL SetActiveDoc(void* pModel,BYTE ciModelFlag=0,BOOL bDraw=TRUE);
	void DeleteDoc(void* pModel);
	void GetAppVersion(char *file_version,char *product_version);
// 重写
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// 实现
	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();
	virtual BOOL SaveState(LPCTSTR lpszSectionName = NULL, CFrameImpl* pFrameImpl = NULL);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnAppAbout();
	afx_msg void OnFileOpen();
};

extern CTMDHApp theApp;
