#pragma once
#include "resource.h"
#ifndef _HANDOVER_TO_CLIENT_
#include "I_DrawSolid.h"
#endif
#include "XhListCtrl.h"

// CMainDlg 对话框

class CModelDataDlg : public CDialog
{
	BOOL m_bBatchMode;
protected:
	CString m_sFileName;
	CHashStrList<int> m_hashSubLeg;
#ifndef _HANDOVER_TO_CLIENT_
	IDrawSolid* m_pDrawSolid;
	ISolidDraw *m_pSolidDraw;
	ISolidSet *m_pSolidSet;
	ISolidSnap *m_pSolidSnap;
	ISolidOper *m_pSolidOper;
#endif
	//
	int m_nDistR,m_nDistB,m_nBtnW;
	DECLARE_DYNAMIC(CModelDataDlg)
public:
	CModelDataDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CModelDataDlg();
// 属性
public:
#ifndef _HANDOVER_TO_CLIENT_
	UCS_STRU rot_ucs,rot_ucs_static;
	ISolidDraw* SolidDraw(){return m_pSolidDraw;}
	ISolidSet * SolidSet(){return m_pSolidSet;}
	ISolidSnap* SolidSnap(){return m_pSolidSnap;}
	ISolidOper* SolidOper(){return m_pSolidOper;}
#else
	void RedrawTowerSketchImage();
#endif
	void RedrawTowerSketch();
	void InitDrawSolidInfo();
	void RereshContrl();
	void RefeshListCtrl();
// 对话框数据
	enum { IDD = IDD_MODEL_DATA_DLG };
	CString m_sHeightName;
	CString m_sLegASerial;
	CString m_sLegBSerial;
	CString m_sLegCSerial;
	CString m_sLegDSerial;
	int m_iModelLevel;
	int m_iLengthUnit;
	BOOL m_bExport3ds;
	BOOL m_bExportXml;
	BOOL m_bExportMod;
	CXhListCtrl m_xListCtrl;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedSelectFile();
	afx_msg void OnBnClickedPreview();
	afx_msg void OnBnClickedExportFile();
	afx_msg void OnCbnSelchangeCmbModelLevel();
	afx_msg void OnCbnSelchangeCmbLengthUnit();
	afx_msg void OnCbnSelchangeCmbFileType();
	afx_msg void OnCbnSelchangeCmbHugao();
	afx_msg void OnCbnSelchangeCmbLegA();
	afx_msg void OnCbnSelchangeCmbLegB();
	afx_msg void OnCbnSelchangeCmbLegC();
	afx_msg void OnCbnSelchangeCmbLegD();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
};
