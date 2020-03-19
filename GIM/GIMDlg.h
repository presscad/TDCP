
// GIMDlg.h : 头文件
//

#pragma once
#include "GIMModle.h"

// CGIMDlg 对话框
class CGIMDlg : public CDialogEx
{
// 构造
public:
	CTowerGim* m_pCurGim;
public:
	CGIMDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_GIM_DIALOG };
	BOOL m_bPack;	// 压缩打包
	CString m_sBatchModFile;
	CString m_sTowerModPath;
	CString m_sOutPutPath;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	virtual BOOL OnInitDialog();	
// 实现
protected:
	// 生成的消息映射函数
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
