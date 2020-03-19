
// TPSM.h : TPSM Ӧ�ó������ͷ�ļ�
//
#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"       // ������
#include "TPSMDoc.h"
#include "TPSMView.h"
#include "TPSMModel.h"
#include "MainFrm.h"

// CTPSMApp:
// �йش����ʵ�֣������ TPSM.cpp
//

class CTPSMApp : public CWinAppEx
{
public:
	CMultiDocTemplate* m_pDocTemplate;
	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;
	char execute_path[MAX_PATH];//��ȡִ���ļ�������Ŀ¼
public:
	CTPSMApp();

// ����
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
// ��д
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
