#pragma once

#include "supergridctrl.h"
#include "TPSMModel.h"
//////////////////////////////////////////////////////////////////////////
// CTowerDataDlg 对话框
class CWireDataDlg : public CDialogEx
{
	DECLARE_DYNCREATE(CWireDataDlg)
private:
	CSuperGridCtrl::CTreeItem* AddItem(CXhWirePoint* pWirePt,CSuperGridCtrl::CTreeItem* pGroupItem);
public:
	CWireDataDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CWireDataDlg();
	void UpdateWireNodeList(CTPSMModel* pActiveModel = NULL);
	//
// 对话框数据
	enum { IDD = IDD_WIRE_DATA_DLG };
	CSuperGridCtrl	m_xDataListCtrl;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize(UINT nType, int cx, int cy);
};
