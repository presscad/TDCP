
// TMDHDoc.cpp : CTMDHDoc ���ʵ��
//

#include "stdafx.h"
// SHARED_HANDLERS ������ʵ��Ԥ��������ͼ������ɸѡ�������
// ATL ��Ŀ�н��ж��壬�����������Ŀ�����ĵ����롣
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


// CTMDHDoc ����/����

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

	// TODO: �ڴ�������³�ʼ������
	// (SDI �ĵ������ø��ĵ�)

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
	CLDSPart::library=&globalLibrary;	//������
	CLDSObject::_console=NULL;			//��������̨
	CWorkStatus::sysPara=NULL;			//��������
	CLDSLinePart::sysPara=NULL;			//��������
	CLDSObject::log2file=&logerr;
	if(m_ciDataFlag==0&&m_pData)
	{
		CTMDHTower* pLdsModel=(CTMDHTower*)m_pData;
		CDesignThickPara::m_pTower=&(pLdsModel->m_xTower);		
		CLDSPoint::m_pTower=&(pLdsModel->m_xTower);
		CLDSVector::m_pTower=&(pLdsModel->m_xTower);
	}
}

// CTMDHDoc ���л�

void CTMDHDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: �ڴ���Ӵ洢����
	}
	else
	{
		// TODO: �ڴ���Ӽ��ش���
	}
}

#ifdef SHARED_HANDLERS

// ����ͼ��֧��
void CTMDHDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// �޸Ĵ˴����Ի����ĵ�����
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

// ������������֧��
void CTMDHDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// ���ĵ����������������ݡ�
	// ���ݲ���Ӧ�ɡ�;���ָ�

	// ����:  strSearchContent = _T("point;rectangle;circle;ole object;")��
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

// CTMDHDoc ���

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


// CTMDHDoc ����
