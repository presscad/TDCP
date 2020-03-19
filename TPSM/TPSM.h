
// TPSM.h : TPSM 应用程序的主头文件
//
#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#include "resource.h"       // 主符号
#include "TPSMDoc.h"
#include "TPSMView.h"
#include "TPSMModel.h"
#include "MainFrm.h"

// CTPSMApp:
// 有关此类的实现，请参阅 TPSM.cpp
//

class CTPSMApp : public CWinAppEx
{
public:
	CMultiDocTemplate* m_pDocTemplate;
	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;
	char execute_path[MAX_PATH];//获取执行文件的启动目录
public:
	CTPSMApp();

// 操作
	CTPSMDoc* GetActiveDoc();
	CTPSMView* GetActiveView();
	BOOL SetActiveDoc(CTPSMModel* pModel,BOOL bDraw=TRUE);
	void DeleteDoc(CTPSMModel* pModel);
	void GetAppVersion(char *file_version,char *product_version);
	//
	void ExportTidData();
	void ExportModData();
	void ExportDxfData();
	void Export3dsData();
	void ExportLdsData();
// 重写
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnAppAbout();
	afx_msg void OnFileNew();
	afx_msg void OnFileOpen();
	afx_msg void OnExportTidData();
	afx_msg void OnExportModData();
};
extern CTPSMApp theApp;
