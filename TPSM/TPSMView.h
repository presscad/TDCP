
// TPSMView.h : CTPSMView 类的接口
//

#pragma once
#include "TPSMDoc.h"
#include "I_DrawSolid.h"

class CTPSMView : public CView
{
	PEN_STRU m_pen;
	IDrawSolid *m_pDrawSolid;
	ISolidDraw *m_pSolidDraw;
	ISolidSet  *m_pSolidSet;
	ISolidSnap *m_pSolidSnap;
	ISolidOper *m_pSolidOper;
protected: // 仅从序列化创建
	CTPSMView();
	virtual ~CTPSMView();
	DECLARE_DYNCREATE(CTPSMView)
// 特性
public:
	CTPSMDoc* GetDocument() const;
// 操作
public:
	ISolidDraw* SolidDraw(){return m_pSolidDraw;}
	ISolidOper* SolidOper(){return m_pSolidOper;}
	ISolidSnap* SolidSnap(){return m_pSolidSnap;}
	ISolidSet*  SolidSet(){return m_pSolidSet;}
	void Refresh(BOOL bZoomAll=TRUE);
// 重写
public:
	virtual void OnDraw(CDC* pDC);  // 重写以绘制该视图
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
// 生成的消息映射函数
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

#ifndef _DEBUG  // TPSMView.cpp 中的调试版本
inline CTPSMDoc* CTPSMView::GetDocument() const
   { return reinterpret_cast<CTPSMDoc*>(m_pDocument); }
#endif

