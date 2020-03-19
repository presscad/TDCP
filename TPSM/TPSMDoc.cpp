
// TPSMDoc.cpp : CTPSMDoc ���ʵ��
//

#include "stdafx.h"
#include "TPSM.h"
#include "TPSMDoc.h"
#include "LibraryQuery.h"
#include "CryptBuffer.h"
#include "ProcBarDlg.h"
#include <propkey.h>
#include "GlobalFunc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CTPSMDoc

IMPLEMENT_DYNCREATE(CTPSMDoc, CDocument)

BEGIN_MESSAGE_MAP(CTPSMDoc, CDocument)
	
END_MESSAGE_MAP()


// CTPSMDoc ����/����

CTPSMDoc::CTPSMDoc()
{
	// TODO: �ڴ����һ���Թ������
	m_ciDocType=0;
	m_pModel=NULL;
	m_pModModel=NULL;
	m_pTidModel=NULL;
	//
	InitTowerModel();
}

CTPSMDoc::~CTPSMDoc()
{
	if(m_pModModel!=NULL)
		CModModelFactory::Destroy(m_pModModel->GetSerialId());
	m_pModModel=NULL;
	//
	if(m_pTidModel!=NULL)
		CTidModelFactory::Destroy(m_pTidModel->GetSerialId());
	m_pTidModel=NULL;
}

CView* CTPSMDoc::GetView(const CRuntimeClass* pClass)
{
	CView *pView=NULL;
	POSITION position = GetFirstViewPosition();
	for(;;)
	{
		if(position==NULL)
		{
			pView = NULL;
			break;
		}
		pView = GetNextView(position);
		if(pView->IsKindOf(pClass))
			break;
	}
	return pView;
}

void CTPSMDoc::InitTowerModel()
{
	CLDSObject::_console=NULL;//&console;		//��������̨
	CLDSPart::library=&globalLibrary;	//������
	CWorkStatus::sysPara=NULL;//&g_sysPara;	//��������
	CLDSLinePart::sysPara=NULL;//&g_sysPara;	//��������
	CDesignThickPara::m_pTower=&Ta;		
	CLDSPoint::m_pTower=&Ta;
	CLDSVector::m_pTower=&Ta;
	CLDSObject::log2file=&logerr;
	//
	Ta.user_pipeline_no=DogSerialNo();
	Ta.InitTower();
	//��ȡ�ļ��������ص������趨
	Ta.DisplayProcess=DisplayProcess;
}

BOOL CTPSMDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: �ڴ�������³�ʼ������
	// (SDI �ĵ������ø��ĵ�)

	return TRUE;
}
// CTPSMDoc ���л�

void CTPSMDoc::Serialize(CArchive& ar)
{
	BeginWaitCursor();
	DWORD cursor_pipeline_no=DogSerialNo();
	if (ar.IsStoring())
	{
		if(m_pModel)
		{
			CBuffer buffer(10000000);
			m_pModel->ToBuffer(buffer);
			ar.Write(buffer.GetBufferPtr(),buffer.GetLength());
		}
		else if(m_ciDocType==CTPSMDoc::DOC_LDS)
		{	
			/*ar <<CString("������������ͼϵͳ");
			ar <<CString(CTPSMModel::lds_file_v);
			ar <<Ta.user_pipeline_no;
			ar <<cursor_pipeline_no;
			CBuffer buffer(10000000);
			Ta.ToBuffer(buffer,DogSerialNo(),CTPSMModel::lds_file_v,4);
			EncryptBuffer(buffer,true);
			DWORD dwFileLen = buffer.GetLength();
			ar.Write(&dwFileLen,sizeof(DWORD));
			ar.Write(buffer.GetBufferPtr(),dwFileLen);*/
		}
		else if(m_ciDocType==CTPSMDoc::DOC_TPI)
		{
			CBuffer buffer(10000000);
			Tpi.ToBuffer(buffer);
			ar.Write(buffer.GetBufferPtr(),buffer.GetLength());
		}
	}
	else
	{
		if(m_ciDocType==DOC_LDS)
		{
			CString sDocTypeName,sFileVersion;
			ar >> sDocTypeName;
			int doc_type=0;
			if(sDocTypeName.Compare("������������") ==0)
				doc_type=1;
			else if(sDocTypeName.Compare("����������ϵͳ") ==0||sDocTypeName.Compare("��������������") ==0)
				doc_type=2;
			else if(sDocTypeName.Compare("����Ӧ�����") ==0||sDocTypeName.Compare("���������������") ==0)
				doc_type=3;
			else if(sDocTypeName.Compare("������ͼ����") ==0)
				doc_type=5;
			else //defined(__LDS_FILE_)||defined(__LDS_)
				doc_type=4;
			ar >> sFileVersion;
			_snprintf(Ta.version,19,"%s",sFileVersion);
			DWORD buffer_len;
			ar>>Ta.user_pipeline_no;
			ar>>cursor_pipeline_no;
			ar>>buffer_len;
			BYTE *pBuf = new BYTE[buffer_len];
			ar.Read(pBuf,buffer_len);
			CBuffer buffer(buffer_len);
			buffer.Write(pBuf,buffer_len);
			delete []pBuf;
			if((Ta.user_pipeline_no=DogSerialNo())==0)
#ifdef AFX_TARG_ENU_ENGLISH
				throw "Can't find the encryption dog,so unable to open the data file!";
#else 
				throw "�Ҳ������ܹ�,�޷��������ļ�!";
#endif
			char bEncryptByAES=false;
			if(doc_type==1)
			{
				if(compareVersion(Ta.version,"5.1.7.0")>=0)
					bEncryptByAES=2;	//��ָ�Ƶļ���ģʽ
				else if(compareVersion(Ta.version,"4.1.3.0")>=0)
					bEncryptByAES=true;
			}
			else if(doc_type==2)
			{
				if(compareVersion(Ta.version,"2.0.8.0")>=0)
					bEncryptByAES=2;	//LMA
				else if(compareVersion(Ta.version,"1.1.3.0")>=0)
					bEncryptByAES=true;	//LMA
			}
			else if(doc_type==3&&compareVersion(Ta.version,"2.1.0.0")>=0)
				bEncryptByAES=true;	//TSA
			else if(doc_type==4||doc_type==5)
			{
				if(compareVersion(Ta.version,"1.3.8.0")>=0)
					bEncryptByAES=2;	//LMA
				else if(compareVersion(Ta.version,"1.3.0.0")>=0)
					bEncryptByAES=true;	//LDS or TDA
			}
			DecryptBuffer(buffer,bEncryptByAES,cursor_pipeline_no);
			if(!Ta.FromBuffer(buffer,doc_type))
				DisplayProcess(100,NULL);
			//��ȡ���ӻ���
			DWORD buf_len = 0;
			DWORD dwRead = ar.Read(&buf_len, 4);
			if (dwRead == 4 && buf_len > 0)
			{
				CHAR_ARRAY buf(buf_len);
				CBuffer attachBuffer(buf, buf_len);
				ar.Read(buf, buf_len);
				Ta.FromAttachBuffer(attachBuffer);
			}
			//�ҵ�ģ�ͻ���
			CBuffer* pUniWireModel = Ta.AttachBuffer.GetValue(CTower::UNI_WIREPOINT_MODEL);
			if (pUniWireModel&&pUniWireModel->Length > 0)
				gxWirePointModel.FromBuffer(*pUniWireModel);
			else
				gxWirePointModel.m_bGimWireModelInherited = false;
			if (doc_type == 4 && !gxWirePointModel.m_bGimWireModelInherited)
				Ta.RetrieveWireModelFromNodes(&gxWirePointModel);
			//
			Ta.SetDocModifiedFlag(FALSE);
			SetModifiedFlag(FALSE);
		}
		else if(m_ciDocType==DOC_TPI)
		{
			CFile* pFile=ar.GetFile();
			DWORD dwFileLen=(DWORD)pFile->GetLength();
			CBuffer buffer;
			buffer.Write(NULL,dwFileLen);
			ar.Read(buffer.GetBufferPtr(),dwFileLen);
			Tpi.FromBuffer(buffer);
		}
	}
	EndWaitCursor();
}

// CTPSMDoc ���
#ifdef _DEBUG
void CTPSMDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CTPSMDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CTPSMDoc ����
BOOL CTPSMDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	DeleteContents();
	SetPathName(lpszPathName);
	SetModifiedFlag();  // dirty during de-serialize
	CXhChar16 extension;
	_splitpath(lpszPathName,NULL,NULL,NULL,extension);
	if(extension.EqualNoCase(".tpi"))
		m_ciDocType=DOC_TPI;
	else if(extension.EqualNoCase(".lds"))
		m_ciDocType=DOC_LDS;
	else if(extension.EqualNoCase(".tid"))
	{	
		m_ciDocType=DOC_TID;
		FILE *fp = fopen(lpszPathName,"rt");
		if(fp==NULL)
			return FALSE;
		fseek(fp,0,SEEK_END);
		long buf_size=ftell(fp);
		fseek(fp,0,SEEK_SET);
		CBuffer buffer;
		buffer.Write(NULL,buf_size);
		fread(buffer.GetBufferPtr(),buffer.GetLength(),1,fp);
		fclose(fp);
		if(m_pTidModel==NULL)
			m_pTidModel=CTidModelFactory::CreateTidModel();
		m_pTidModel->InitTidBuffer(buffer.GetBufferPtr(),buf_size);
		return TRUE;
	}
	else if(extension.EqualNoCase(".mod"))
	{
		m_ciDocType=DOC_MOD;
		if(m_pModModel==NULL)
			m_pModModel=CModModelFactory::CreateModModel();
		m_pModModel->ImportModFile(lpszPathName);
		return TRUE;
	}
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;
	return TRUE;
}
void CTPSMDoc::DeleteContents()
{
	if(m_pModModel!=NULL)
		CModModelFactory::Destroy(m_pModModel->GetSerialId());
	m_pModModel=NULL;
	//
	if(m_pTidModel!=NULL)
		CTidModelFactory::Destroy(m_pTidModel->GetSerialId());
	m_pTidModel=NULL;
	//
	CTPSMView *pView=theApp.GetActiveView();
	if(pView)
	{
		ISolidDraw* pSolidDraw=pView->SolidDraw();
		if(pSolidDraw)
			pSolidDraw->EmptyDisplayBuffer();
	}
	CDocument::DeleteContents();
}
