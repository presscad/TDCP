
// GIMDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "GIM.h"
#include "GIMDlg.h"
#include "afxdialogex.h"
#include "PropertyListOper.h"
#include "folder_dialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//ԭʼGIMͷ����+Ŀ��GIM��7Z����=�µ�GIM
void Test1()
{
	//1.��ȡԭʼ��GIM�ļ�
	CXhChar200 file_path("E:\\GIM\\2E16-SZCK.gim");
	FILE* fp=fopen(file_path,"rb");
	if(fp==NULL)
	{
		logerr.Log(CXhChar100("%s��������ʧ��(��)!",file_path));
		return;
	}
	fseek(fp,0,SEEK_END);
	long buf_size=ftell(fp);
	fseek(fp,0,SEEK_SET);
	CBuffer srcGimBuf;
	srcGimBuf.Write(NULL,buf_size);
	fread(srcGimBuf.GetBufferPtr(),srcGimBuf.GetLength(),1,fp);
	fclose(fp);
	//��ȡͷ��Ϣ
	CBuffer head_buf;
	head_buf.Write(NULL,768);
	srcGimBuf.SeekToBegin();
	srcGimBuf.Read(head_buf.GetBufferPtr(),768);
	//2.��ȡĿ���GIM�ļ�
	CXhChar100 gim_path("E:\\GIM\\2E16-SZCK\\2E16-SZCK.gim");
	fp=fopen(gim_path,"rb");
	if(fp==NULL)
	{
		logerr.Log(CXhChar100("%s��������ʧ��(��)!",gim_path));
		return;
	}
	fseek(fp,0,SEEK_END);
	buf_size=ftell(fp);
	fseek(fp,0,SEEK_SET);
	CBuffer destGimBuf;
	destGimBuf.Write(NULL,buf_size);
	fread(destGimBuf.GetBufferPtr(),destGimBuf.GetLength(),1,fp);
	fclose(fp);
	//��ȡ7Z����
	CBuffer zip_buf;
	char szBuf[8]={0};
	destGimBuf.SeekToBegin();
	destGimBuf.SeekOffset(768);
	destGimBuf.Read(szBuf,8);
	zip_buf.Write(NULL,atoi(szBuf));
	destGimBuf.Read(zip_buf.GetBufferPtr(),atoi(szBuf));		//�洢�����С
	//3.����д��GIM�ļ�(����ͷ+�ź�7z)
	CBuffer buffer(10000000);
	buffer.Write(head_buf.GetBufferPtr(),head_buf.GetLength());
	buffer.Write(szBuf,8);
	buffer.Write(zip_buf.GetBufferPtr(),zip_buf.GetLength());
	buffer.SeekToBegin();
	gim_path.Copy("E:\\GIM\\2E16-SZCK-D-X.gim");
	fp=fopen(gim_path,"wb");
	if(fp==NULL)
	{
		logerr.Log(CXhChar100("%s��������ʧ��(д)!",gim_path));
		return;
	}
	fwrite(buffer.GetBufferPtr(),buffer.GetLength(),1,fp);
	fclose(fp);
}
//��ȡ7Z�洢����
void Test2()
{
	CFileDialog dlg(TRUE,"gim","�������.gim",
		OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT|OFN_ALLOWMULTISELECT,
		"����ļ�(*.gim)|*.gim|�����ļ�(*.*)|*.*||");
	if(dlg.DoModal()!=IDOK)
		return;
	CXhChar200 file_path(dlg.GetPathName());
	FILE* fp=fopen(file_path,"rb");
	if(fp==NULL)
	{
		logerr.Log(CXhChar100("%s��������ʧ��(��)!",file_path));
		return;
	}
	CBuffer srcGimBuf,destGimBuf;
	fseek(fp,0,SEEK_END);
	long buf_size=ftell(fp);
	fseek(fp,0,SEEK_SET);
	srcGimBuf.Write(NULL,buf_size);
	fread(srcGimBuf.GetBufferPtr(),srcGimBuf.GetLength(),1,fp);
	fclose(fp);
	//ȥ��ͷ��Ϣ
	srcGimBuf.SeekToBegin();
	srcGimBuf.SeekOffset(776);
	destGimBuf.Write(NULL,buf_size-776);
	srcGimBuf.Read(destGimBuf.GetBufferPtr(),buf_size-776);
	//����д��
	fp=fopen(file_path,"wb");
	if(fp==NULL)
	{
		logerr.Log(CXhChar100("%s��������ʧ��(��)!",file_path));
		return;
	}
	destGimBuf.SeekToBegin();
	fwrite(destGimBuf.GetBufferPtr(),destGimBuf.GetLength(),1,fp);
	fclose(fp);
}
//��ȡͷ�洢����
void Test3()
{
	CFileDialog dlg(TRUE,"gim","�������.gim",
		OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT|OFN_ALLOWMULTISELECT,
		"����ļ�(*.gim)|*.gim|�����ļ�(*.*)|*.*||");
	if(dlg.DoModal()!=IDOK)
		return;
	CXhChar200 file_path(dlg.GetPathName());
	FILE* fp=fopen(file_path,"rb");
	if(fp==NULL)
	{
		logerr.Log(CXhChar100("%s��������ʧ��(��)!",file_path));
		return;
	}
	CBuffer srcGimBuf,destGimBuf;
	fseek(fp,0,SEEK_END);
	long buf_size=ftell(fp);
	fseek(fp,0,SEEK_SET);
	srcGimBuf.Write(NULL,buf_size);
	fread(srcGimBuf.GetBufferPtr(),srcGimBuf.GetLength(),1,fp);
	fclose(fp);
	//��ȡͷ��Ϣ
	destGimBuf.Write(NULL,768);
	srcGimBuf.SeekToBegin();
	srcGimBuf.Read(destGimBuf.GetBufferPtr(),768);
	//����д��
	CXhChar100 path("E:\\GIM\\Head.gim");
	fp=fopen(path,"wb");
	if(fp==NULL)
	{
		logerr.Log(CXhChar100("%s��������ʧ��(��)!",file_path));
		return;
	}
	destGimBuf.SeekToBegin();
	fwrite(destGimBuf.GetBufferPtr(),destGimBuf.GetLength(),1,fp);
	fclose(fp);
}
//�滻�汾��
void Test4()
{
	//1.��ȡ��ȷ��Head����
	CXhChar200 file_path("E:\\GIM\\Head.gim");
	FILE* fp=fopen(file_path,"rb");
	if(fp==NULL)
	{
		logerr.Log(CXhChar100("%s��������ʧ��(��)!",file_path));
		return;
	}
	fseek(fp,0,SEEK_END);
	long buf_size=ftell(fp);
	fseek(fp,0,SEEK_SET);
	CBuffer head_buf;
	head_buf.Write(NULL,buf_size);
	fread(head_buf.GetBufferPtr(),head_buf.GetLength(),1,fp);
	fclose(fp);
	//��ȡ������Ϣ
	CBuffer buf1;
	buf1.Write(NULL,32);
	head_buf.SeekToBegin();
	head_buf.SeekOffset(736);
	head_buf.Read(buf1.GetBufferPtr(),32);
	//2.��ȡĿ���GIM�ļ�
	CXhChar100 gim_path("E:\\GIM\\2E16-SZCK\\2E16-SZCK.gim");
	fp=fopen(gim_path,"rb");
	if(fp==NULL)
	{
		logerr.Log(CXhChar100("%s��������ʧ��(��)!",gim_path));
		return;
	}
	fseek(fp,0,SEEK_END);
	buf_size=ftell(fp);
	fseek(fp,0,SEEK_SET);
	CBuffer destGimBuf;
	destGimBuf.Write(NULL,buf_size);
	fread(destGimBuf.GetBufferPtr(),destGimBuf.GetLength(),1,fp);
	fclose(fp);
	//��ȡ7Z����
	CBuffer buf2,zip_buf;
	char szBuf[8]={0};
	buf2.Write(NULL,736);
	destGimBuf.SeekToBegin();
	destGimBuf.Read(buf2.GetBufferPtr(),736);
	destGimBuf.SeekToBegin();
	destGimBuf.SeekOffset(768);
	destGimBuf.Read(szBuf,8);
	zip_buf.Write(NULL,atoi(szBuf));
	destGimBuf.Read(zip_buf.GetBufferPtr(),atoi(szBuf));		//�洢�����С
	//3.����д��GIM�ļ�(����ͷ+�ź�7z)
	CBuffer buffer(10000000);
	buffer.Write(buf2.GetBufferPtr(),buf2.GetLength());	
	buffer.Write(buf1.GetBufferPtr(),buf1.GetLength());
	buffer.Write(szBuf,8);
	buffer.Write(zip_buf.GetBufferPtr(),zip_buf.GetLength());
	buffer.SeekToBegin();
	gim_path.Copy("E:\\GIM\\2E16-SZCK-V.gim");
	fp=fopen(gim_path,"wb");
	if(fp==NULL)
	{
		logerr.Log(CXhChar100("%s��������ʧ��(д)!",gim_path));
		return;
	}
	fwrite(buffer.GetBufferPtr(),buffer.GetLength(),1,fp);
	fclose(fp);
}
//�滻�������
void Test5()
{
	//1.��ȡ��ȷ��Head����
	CXhChar200 file_path("E:\\GIM\\Head.gim");
	FILE* fp=fopen(file_path,"rb");
	if(fp==NULL)
	{
		logerr.Log(CXhChar100("%s��������ʧ��(��)!",file_path));
		return;
	}
	fseek(fp,0,SEEK_END);
	long buf_size=ftell(fp);
	fseek(fp,0,SEEK_SET);
	CBuffer head_buf;
	head_buf.Write(NULL,buf_size);
	fread(head_buf.GetBufferPtr(),head_buf.GetLength(),1,fp);
	fclose(fp);
	//��ȡ������Ϣ
	CBuffer buf1;
	buf1.Write(NULL,128);
	head_buf.SeekToBegin();
	head_buf.SeekOffset(592);
	head_buf.Read(buf1.GetBufferPtr(),128);
	//2.��ȡĿ���GIM�ļ�
	CXhChar100 gim_path("E:\\GIM\\2E16-SZCK\\2E16-SZCK.gim");
	fp=fopen(gim_path,"rb");
	if(fp==NULL)
	{
		logerr.Log(CXhChar100("%s��������ʧ��(��)!",gim_path));
		return;
	}
	fseek(fp,0,SEEK_END);
	buf_size=ftell(fp);
	fseek(fp,0,SEEK_SET);
	CBuffer destGimBuf;
	destGimBuf.Write(NULL,buf_size);
	fread(destGimBuf.GetBufferPtr(),destGimBuf.GetLength(),1,fp);
	fclose(fp);
	//
	CBuffer buf2,buf3,zip_buf;
	char szBuf[8]={0};
	buf2.Write(NULL,592);
	destGimBuf.SeekToBegin();
	destGimBuf.Read(buf2.GetBufferPtr(),592);
	destGimBuf.SeekToBegin();
	destGimBuf.SeekOffset(720);
	buf3.Write(NULL,48);
	destGimBuf.Read(buf3.GetBufferPtr(),48);
	destGimBuf.SeekToBegin();
	destGimBuf.SeekOffset(768);
	destGimBuf.Read(szBuf,8);
	zip_buf.Write(NULL,atoi(szBuf));
	destGimBuf.Read(zip_buf.GetBufferPtr(),atoi(szBuf));		//�洢�����С
	//3.����д��GIM�ļ�(����ͷ+�ź�7z)
	CBuffer buffer(10000000);
	buffer.Write(buf2.GetBufferPtr(),buf2.GetLength());	
	buffer.Write(buf1.GetBufferPtr(),buf1.GetLength());
	buffer.Write(buf3.GetBufferPtr(),buf3.GetLength());
	buffer.Write(szBuf,8);
	buffer.Write(zip_buf.GetBufferPtr(),zip_buf.GetLength());
	buffer.SeekToBegin();
	gim_path.Copy("E:\\GIM\\2E16-SZCK-S.gim");
	fp=fopen(gim_path,"wb");
	if(fp==NULL)
	{
		logerr.Log(CXhChar100("%s��������ʧ��(д)!",gim_path));
		return;
	}
	fwrite(buffer.GetBufferPtr(),buffer.GetLength(),1,fp);
	fclose(fp);
}
//�滻��֯��λ
void Test6()
{
	//1.��ȡ��ȷ��Head����
	CXhChar200 file_path("E:\\GIM\\Head.gim");
	FILE* fp=fopen(file_path,"rb");
	if(fp==NULL)
	{
		logerr.Log(CXhChar100("%s��������ʧ��(��)!",file_path));
		return;
	}
	fseek(fp,0,SEEK_END);
	long buf_size=ftell(fp);
	fseek(fp,0,SEEK_SET);
	CBuffer head_buf;
	head_buf.Write(NULL,buf_size);
	fread(head_buf.GetBufferPtr(),head_buf.GetLength(),1,fp);
	fclose(fp);
	//��ȡ������Ϣ
	CBuffer buf1;
	buf1.Write(NULL,256);
	head_buf.SeekToBegin();
	head_buf.SeekOffset(336);
	head_buf.Read(buf1.GetBufferPtr(),256);
	//2.��ȡĿ���GIM�ļ�
	CXhChar100 gim_path("E:\\GIM\\2E16-SZCK\\2E16-SZCK.gim");
	fp=fopen(gim_path,"rb");
	if(fp==NULL)
	{
		logerr.Log(CXhChar100("%s��������ʧ��(��)!",gim_path));
		return;
	}
	fseek(fp,0,SEEK_END);
	buf_size=ftell(fp);
	fseek(fp,0,SEEK_SET);
	CBuffer destGimBuf;
	destGimBuf.Write(NULL,buf_size);
	fread(destGimBuf.GetBufferPtr(),destGimBuf.GetLength(),1,fp);
	fclose(fp);
	//
	CBuffer buf2,buf3,zip_buf;
	char szBuf[8]={0};
	buf2.Write(NULL,336);
	destGimBuf.SeekToBegin();
	destGimBuf.Read(buf2.GetBufferPtr(),336);
	destGimBuf.SeekToBegin();
	destGimBuf.SeekOffset(592);
	buf3.Write(NULL,176);
	destGimBuf.Read(buf3.GetBufferPtr(),176);
	destGimBuf.SeekToBegin();
	destGimBuf.SeekOffset(768);
	destGimBuf.Read(szBuf,8);
	zip_buf.Write(NULL,atoi(szBuf));
	destGimBuf.Read(zip_buf.GetBufferPtr(),atoi(szBuf));		//�洢�����С
	//3.����д��GIM�ļ�(����ͷ+�ź�7z)
	CBuffer buffer(10000000);
	buffer.Write(buf2.GetBufferPtr(),buf2.GetLength());	
	buffer.Write(buf1.GetBufferPtr(),buf1.GetLength());
	buffer.Write(buf3.GetBufferPtr(),buf3.GetLength());
	buffer.Write(szBuf,8);
	buffer.Write(zip_buf.GetBufferPtr(),zip_buf.GetLength());
	buffer.SeekToBegin();
	gim_path.Copy("E:\\GIM\\2E16-SZCK-U.gim");
	fp=fopen(gim_path,"wb");
	if(fp==NULL)
	{
		logerr.Log(CXhChar100("%s��������ʧ��(д)!",gim_path));
		return;
	}
	fwrite(buffer.GetBufferPtr(),buffer.GetLength(),1,fp);
	fclose(fp);
}
extern void SimplifiedNumString(char *sNumStr);
//////////////////////////////////////////////////////////////////////////
// CGIMDlg �Ի���
CGIMDlg::CGIMDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CGIMDlg::IDD, pParent)
	, m_bPack(TRUE)
	, m_sBatchModFile(_T(""))
	, m_sTowerModPath(_T(""))
	, m_sOutPutPath(_T(""))
{
	m_pCurGim=NULL;

}

void CGIMDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHE_PACK, m_bPack);
	DDX_Text(pDX, IDC_E_BATCH_FILE, m_sBatchModFile);
	DDX_Text(pDX, IDC_E_MOD_FILE, m_sTowerModPath);
	DDX_Text(pDX, IDC_E_OUTPUT_PATH, m_sOutPutPath);
}

BEGIN_MESSAGE_MAP(CGIMDlg, CDialogEx)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BTN_BATCH_FILE, OnBtnClickBatchFile)
	ON_BN_CLICKED(IDC_BTN_MOD_FILE, OnBtnClickModFile)
	ON_BN_CLICKED(IDC_BTN_OUTPUT_PATH, OnBtnClickOutPutPath)
	ON_BN_CLICKED(IDC_BTN_TRANSFORM, OnBtnClickTransform)
	ON_BN_CLICKED(IDC_BTN_CLOSE, OnBtnClickClose)
	ON_BN_CLICKED(IDC_CHE_PACK, &CGIMDlg::OnBnClickedChePack)
END_MESSAGE_MAP()


// CGIMDlg ��Ϣ�������

BOOL CGIMDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	//
	//GetDlgItem(IDC_BTN_MOD_FILE)->ShowWindow(SW_HIDE);
	//GetDlgItem(IDC_BTN_OUTPUT_PATH)->ShowWindow(SW_HIDE);
	((CEdit*)GetDlgItem(IDC_E_MOD_FILE))->SetReadOnly(TRUE);
	((CEdit*)GetDlgItem(IDC_E_OUTPUT_PATH))->SetReadOnly(TRUE);
	UpdateData(FALSE);
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CGIMDlg::OnOK()
{

}
void CGIMDlg::OnPaint()
{
	CDialogEx::OnPaint();
}
void CGIMDlg::OnBtnClickBatchFile()
{
	CFileDialog dlg(TRUE,"xls","�������.xls",
		OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT|OFN_ALLOWMULTISELECT,
		"����ļ�(*.xls)|*.xls|����ļ�(*.xlsx)|*.xlsx|�����ļ�(*.*)|*.*||");
	if(dlg.DoModal()!=IDOK)
		return;
	//��ȡ��Ϣ�ļ��������д������
	gim_modle.ReadTaModBatchGzipFile(dlg.GetPathName());
	if(gim_modle.GetCount()>0)
		m_pCurGim=gim_modle.EnumFirTowerGim();
	//
	m_sBatchModFile=dlg.GetPathName();
	if(m_pCurGim)
	{
		m_sTowerModPath=m_pCurGim->m_sModPath;
		m_sOutPutPath=m_pCurGim->m_sOutputPath;
	}
	UpdateData(FALSE);
}
void CGIMDlg::OnBtnClickModFile()
{
	if(InvokeFolderPickerDlg(m_sTowerModPath))
	{
		for(CTowerGim* pGimItem=gim_modle.EnumFirTowerGim();pGimItem;pGimItem=gim_modle.EnumNextTowerGim())
			pGimItem->m_sModPath.Copy(m_sTowerModPath);
	}
	UpdateData(FALSE);
}
void CGIMDlg::OnBtnClickOutPutPath()
{
	if(InvokeFolderPickerDlg(m_sOutPutPath))
	{
		for(CTowerGim* pGimItem=gim_modle.EnumFirTowerGim();pGimItem;pGimItem=gim_modle.EnumNextTowerGim())
			pGimItem->m_sOutputPath.Copy(m_sOutPutPath);
	}
	UpdateData(FALSE);
}
void CGIMDlg::OnBtnClickTransform()
{
	UpdateData();
	for(CTowerGim* pGimItem=gim_modle.EnumFirTowerGim();pGimItem;pGimItem=gim_modle.EnumNextTowerGim())
		pGimItem->CreateGuidFile();
	if(m_bPack)
	{	//����ѹ�����
		for(CTowerGim* pGimItem=gim_modle.EnumFirTowerGim();pGimItem;pGimItem=gim_modle.EnumNextTowerGim())
			pGimItem->PackTowerGimFile();
		for(CTowerGim* pGimItem=gim_modle.EnumFirTowerGim();pGimItem;pGimItem=gim_modle.EnumNextTowerGim())
			pGimItem->UpdateGimFile();
	}
	if(AfxMessageBox("�ļ�ת����ɣ��Ƿ�����·����",MB_YESNO)==IDYES)
	{
		CTowerGim* pGimItem=gim_modle.EnumFirTowerGim();
		if(pGimItem)
			WinExec(CXhChar500("explorer.exe %s",(char*)pGimItem->m_sOutputPath),SW_SHOW);
	}
	if(logerr.IsHasContents())
		logerr.ShowToScreen();
	return CDialogEx::OnOK();
}
void CGIMDlg::OnBtnClickClose()
{
	//Test1();
	return CDialogEx::OnCancel();
}
void CGIMDlg::OnBnClickedChePack()
{
	m_bPack=!m_bPack;
}
