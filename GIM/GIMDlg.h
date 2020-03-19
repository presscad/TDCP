
// GIMDlg.h : ͷ�ļ�
//

#pragma once
#include "GIMModle.h"

// CGIMDlg �Ի���
class CGIMDlg : public CDialogEx
{
// ����
public:
	CTowerGim* m_pCurGim;
public:
	CGIMDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_GIM_DIALOG };
	BOOL m_bPack;	// ѹ�����
	CString m_sBatchModFile;
	CString m_sTowerModPath;
	CString m_sOutPutPath;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��
	virtual BOOL OnInitDialog();	
// ʵ��
protected:
	// ���ɵ���Ϣӳ�亯��
	afx_msg void OnPaint();
	afx_msg void OnOK();
	afx_msg void OnBtnClickBatchFile();
	afx_msg void OnBtnClickModFile();
	afx_msg void OnBtnClickOutPutPath();
	afx_msg void OnBtnClickTransform();
	afx_msg void OnBnClickedChePack();
	afx_msg void OnBtnClickClose();
	DECLARE_MESSAGE_MAP()
};
