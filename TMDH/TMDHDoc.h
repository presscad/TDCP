#pragma once

#include "TMDHModel.h"

class CTMDHDoc : public CDocument
{
public:
	static const int LDS_DATA	=0;
	static const int GIM_DATA	=1;
	static const int TMD_DATA	=2;
	//
	BYTE m_ciDataFlag;	//0.LDS|1.GIM|2.TMD
	void* m_pData;
protected: // 仅从序列化创建
	CTMDHDoc();
	DECLARE_DYNCREATE(CTMDHDoc)

// 特性
public:
	CView* GetView(const CRuntimeClass* pClass);
	void InitTowerModel();
// 操作
public:

// 重写
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// 实现
public:
	virtual ~CTMDHDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成的消息映射函数
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// 用于为搜索处理程序设置搜索内容的 Helper 函数
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
};
