
#pragma once
#include "resource.h"       // main symbols
#include "XhCharString.h"
#include "f_ent_list.h"

/////////////////////////////////////////////////////////////////////////////
// CPromptDlg dialog
class CImageRecord
{
private:
	CXhChar200 m_sSketchFile;	//简易图文件
	CXhChar200 m_sImageFile;	//详解图文件
	CXhChar50 m_sLabel;
public:
	CImageRecord();
	CImageRecord(char* sketch_file,char* image_file,char* sLabel);
	BOOL GetSketchBitMap(CBitmap& xBitMap);
	BOOL GetImageBitMap(CBitmap& pBitMap);
	CXhChar50 GetLabelStr();
};
class CPromptDlg : public CDialog
{
	bool m_bEnableWindowMoveListen;
	bool m_bFirstPopup;
	bool m_bRelayout;
	static int nScrLocationX,nScrLocationY;
	void (*drawpane_func)(void* pContext);			//渲染回调函数
	void InitStartPos(int nClientW,int nClientH,BYTE ciWndPosType);
protected:
	CBrush m_bBrush;
	CWnd* m_pWnd;
	bool m_bFromInternalResource;	//true:从EXE文件内部资源加载图片；false:从外部文件加载图片
	int  m_iType;			//0.显示文字 1.显示图片 2.显示图片列表
	long m_idBmpResource;
	CImageList imageList;
	CBitmap m_bBitMap;
	BOOL m_bSelBitMap;
	CXhChar200 bitmapfile;
	int m_dwIconWidth,m_dwIconHeight;
	static const BYTE POS_RIGHT_TOP		= 0;
	static const BYTE POS_LEFT_TOP		= 1;
	static const BYTE POS_CENTER		= 2;
	static const BYTE POS_RIGHT_BOTTOM	= 3;
	static const BYTE POS_LEFT_BOTTOM	= 4;
	BYTE m_ciWndPosType;
public:
	ATOM_LIST<CImageRecord> m_xImageRecList;
	CPromptDlg(CWnd* pWnd);  
	BOOL Create();
	BOOL Destroy();
	void SetMsg(CString msg);
	void SetPromptPicture(long bitmapId);
	void SetPromptPicture(const char* bmpfile);
	void SetPromptPictures();
	void SetUdfDrawFunc(void (*drawfunc)(void* pContext));		// 设置预置重显函数
	void RelayoutWnd(bool bInitStartPos=false);
	enum { IDD = IDD_PROMPT_DLG };
	CString	m_sPromptMsg;
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

protected:
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMove(int x, int y);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnItemchangedListCtrl(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()
public:
	virtual void OnCancel();
};
extern CPromptDlg *g_pPromptMsg;
class CSysPromptObject{
	CPromptDlg *m_pPromptWnd;
	void AssureValid(CWnd *pParentWnd=NULL);
public:
	CSysPromptObject();
	~CSysPromptObject();
	void Close();
	void Prompt(const char* promptstr);
	void SetPromptPicture(long bitmapId);
	void SetPromptPicture(const char* bmpfile);
	void AddImageRecord(CImageRecord& imageRec);
	void SetPromptPictures();
};
