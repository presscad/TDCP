
// TMDHDoc.cpp : CTMDHDoc 类的实现
//

#include "stdafx.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "TMDH.h"
#endif

#include "TMDHDoc.h"
#include "LibraryQuery.h"
#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CTMDHDoc

IMPLEMENT_DYNCREATE(CTMDHDoc, CDocument)

BEGIN_MESSAGE_MAP(CTMDHDoc, CDocument)
END_MESSAGE_MAP()


// CTMDHDoc 构造/析构

CTMDHDoc::CTMDHDoc()
{
	m_pData=NULL;
	InitTowerModel();
}

CTMDHDoc::~CTMDHDoc()
{
}

BOOL CTMDHDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: 在此添加重新初始化代码
	// (SDI 文档将重用该文档)

	return TRUE;
}

CView* CTMDHDoc::GetView(const CRuntimeClass* pClass)
{
	CView *pView=NULL;
	POSITION position = GetFirstViewPosition();
	for(;;)
	{
		if(position==NULL)
		{
			pView = NULL;
			break;
		}
		pView = GetNextView(position);
		if(pView->IsKindOf(pClass))
			break;
	}
	return pView;
}

void CTMDHDoc::InitTowerModel()
{
	CLDSPart::library=&globalLibrary;	//构件库
	CLDSObject::_console=NULL;			//操作控制台
	CWorkStatus::sysPara=NULL;			//环境配置
	CLDSLinePart::sysPara=NULL;			//环境配置
	CLDSObject::log2file=&logerr;
	if(m_ciDataFlag==0&&m_pData)
	{
		CTMDHTower* pLdsModel=(CTMDHTower*)m_pData;
		CDesignThickPara::m_pTower=&(pLdsModel->m_xTower);		
		CLDSPoint::m_pTower=&(pLdsModel->m_xTower);
		CLDSVector::m_pTower=&(pLdsModel->m_xTower);
	}
}

// CTMDHDoc 序列化

void CTMDHDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: 在此添加存储代码
	}
	else
	{
		// TODO: 在此添加加载代码
	}
}

#ifdef SHARED_HANDLERS

// 缩略图的支持
void CTMDHDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// 修改此代码以绘制文档数据
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// 搜索处理程序的支持
void CTMDHDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// 从文档数据设置搜索内容。
	// 内容部分应由“;”分隔

	// 例如:  strSearchContent = _T("point;rectangle;circle;ole object;")；
	SetSearchContent(strSearchContent);
}

void CTMDHDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CTMDHDoc 诊断

#ifdef _DEBUG
void CTMDHDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CTMDHDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CTMDHDoc 命令
