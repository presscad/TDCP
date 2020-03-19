#pragma once

#include "supergridctrl.h"
#include "TPSMModel.h"
//////////////////////////////////////////////////////////////////////////
// CTowerDataDlg �Ի���
class CWireDataDlg : public CDialogEx
{
	DECLARE_DYNCREATE(CWireDataDlg)
private:
	CSuperGridCtrl::CTreeItem* AddItem(CXhWirePoint* pWirePt,CSuperGridCtrl::CTreeItem* pGroupItem);
public:
	CWireDataDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CWireDataDlg();
	void UpdateWireNodeList(CTPSMModel* pActiveModel = NULL);
	//
// �Ի�������
	enum { IDD = IDD_WIRE_DATA_DLG };
	CSuperGridCtrl	m_xDataListCtrl;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize(UINT nType, int cx, int cy);
};
