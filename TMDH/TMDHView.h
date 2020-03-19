
// TMDHView.h : CTMDHView ��Ľӿ�
//

#pragma once
#include "I_DrawSolid.h"

class CTMDHView : public CView
{
	PEN_STRU m_pen;
	IDrawSolid *m_pDrawSolid;
	ISolidDraw *m_pSolidDraw;
	ISolidSet  *m_pSolidSet;
	ISolidSnap *m_pSolidSnap;
	ISolidOper *m_pSolidOper;
protected: // �������л�����
	CTMDHView();
	DECLARE_DYNCREATE(CTMDHView)

// ����
public:
	CTMDHDoc* GetDocument() const;
// ����
public:
	ISolidDraw* SolidDraw(){return m_pSolidDraw;}
	ISolidOper* SolidOper(){return m_pSolidOper;}
	ISolidSnap* SolidSnap(){return m_pSolidSnap;}
	ISolidSet*  SolidSet(){return m_pSolidSet;}
	void Refresh(BOOL bZoomAll=TRUE);
// ��д
public:
	virtual void OnDraw(CDC* pDC);  // ��д�Ի��Ƹ���ͼ
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
protected:

// ʵ��
public:
	virtual ~CTMDHView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// ���ɵ���Ϣӳ�亯��
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // TMDHView.cpp �еĵ��԰汾
inline CTMDHDoc* CTMDHView::GetDocument() const
   { return reinterpret_cast<CTMDHDoc*>(m_pDocument); }
#endif

