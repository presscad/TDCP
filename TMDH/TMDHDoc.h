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
protected: // �������л�����
	CTMDHDoc();
	DECLARE_DYNCREATE(CTMDHDoc)

// ����
public:
	CView* GetView(const CRuntimeClass* pClass);
	void InitTowerModel();
// ����
public:

// ��д
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// ʵ��
public:
	virtual ~CTMDHDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// ���ɵ���Ϣӳ�亯��
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// ����Ϊ����������������������ݵ� Helper ����
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
};
