
// ChildFrm.h : CChildFrame ��Ľӿ�
//

#pragma once

class CChildFrame : public CMDIChildWndEx
{
	DECLARE_DYNCREATE(CChildFrame)
public:
	CChildFrame();

// ����
public:

// ����
public:

// ��д
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// ʵ��
public:
	virtual ~CChildFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// ���ɵ���Ϣӳ�亯��
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnGlAllZoom();
	afx_msg void OnGlZoom();
	afx_msg void OnUpdateGlZoom(CCmdUI* pCmdUI);
	afx_msg void OnGlRotated();
	afx_msg void OnUpdateGlRotated(CCmdUI* pCmdUI);
	afx_msg void OnResetView();
	afx_msg void OnOperOther();
	afx_msg void OnGlPan();
	afx_msg void OnUpdateGlPan(CCmdUI* pCmdUI);
	afx_msg void OnSolidModeDisplay();
	afx_msg void OnUpdateSolidModeDisplay(CCmdUI* pCmdUI);
	afx_msg void OnClickLevelCmbBox();
	afx_msg void OnSelChangeClickCmbBox();
};
