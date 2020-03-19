#pragma once
//
#include "resource.h"
#include "PropertyList.h"
/////////////////////////////////////////////////////////////////////////////
// CPartPropertyDlg dialog

class CPropertyDlg : public CDialogEx
{
	DECLARE_DYNCREATE(CPropertyDlg)
public:
	RECT m_rcClient;
	int m_nOldHorzY;
	HICON m_hCursorArrow;
	HICON m_hCursorSize;
	BOOL m_bTracking;
	int m_nSplitterWidth;
	CStringArray m_sTabLabelArr;
public:
	CPropertyDlg(CWnd* pParent = NULL);   // standard constructor
	//
	void InvertLine(CDC* pDC,CPoint ptFrom,CPoint ptTo);
	void RefreshTabCtrl(int iCurSel);
	void RelayoutDlg();
	CPropertyList *GetPropertyList(){return &m_xPropList;}
// Dialog Data
	//{{AFX_DATA(CPropertyDlg)
	enum { IDD = IDD_PROPERTY_DLG };
	CTabCtrl		m_xTabCtrl;
	CPropertyList	m_xPropList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPropertyDlg)
protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_VIRTUAL
protected:
	// Generated message map functions
	//{{AFX_MSG(CPropertyDlg)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnSelchangeTabGroup(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
