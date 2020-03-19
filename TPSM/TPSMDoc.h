#pragma once
#include "TPSMModel.h"
#include "ModCore.h"
#include "TidCplus.h"

class CTPSMDoc : public CDocument
{
public:
	CTower Ta;
	CTPSMModel Tpi;
	CTPSMModel* m_pModel;
	ITidModel* m_pTidModel;
	IModModel* m_pModModel;
	BYTE m_ciDocType;		//0.TPI|1.LDS|2.TID|3.MOD
	const static BYTE DOC_TPI	=0;
	const static BYTE DOC_LDS	=1;
	const static BYTE DOC_TID	=2;
	const static BYTE DOC_MOD	=3;
protected: // 仅从序列化创建
	CTPSMDoc();
	DECLARE_DYNCREATE(CTPSMDoc)
	
// 特性
public:
	CView* GetView(const CRuntimeClass* pClass);
	void InitTowerModel();
// 重写
public:
	virtual ~CTPSMDoc();
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual void DeleteContents();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	DECLARE_MESSAGE_MAP()

};
