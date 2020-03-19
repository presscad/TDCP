#include "stdafx.h"
#include "PromptDlg.h"
#include "XhLicAgent.h"
#include "ArrayList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static BOOL GetBitMap(const char* bmp_file,CBitmap& xBitMap)
{
	xBitMap.DeleteObject();
	FILE *fp = fopen(bmp_file,"rt");
	if(fp==NULL)
		return FALSE;
	BITMAPFILEHEADER fileHeader;
	BITMAPINFOHEADER infoHeader;
	fread(&fileHeader,1,sizeof(BITMAPFILEHEADER),fp);
	fread(&infoHeader,1,sizeof(BITMAPINFOHEADER),fp);
	int i,nPalette = infoHeader.biClrUsed;
	if(nPalette==0&&infoHeader.biBitCount<=8)
		nPalette=GetSingleWord(infoHeader.biBitCount+1);
	DYN_ARRAY<RGBQUAD> palette(nPalette);
	for(i=0;i<nPalette;i++)
	{
		fread(&palette[i].rgbBlue,1,1,fp);
		fread(&palette[i].rgbGreen,1,1,fp);
		fread(&palette[i].rgbRed,1,1,fp);
		fread(&palette[i].rgbReserved,1,1,fp);
	}
	BITMAP image;
	image.bmType=0;
	image.bmPlanes=1;
	image.bmWidth=infoHeader.biWidth;
	image.bmBitsPixel=infoHeader.biBitCount;
	image.bmHeight=infoHeader.biHeight;
	int widthBits = infoHeader.biWidth*infoHeader.biBitCount;
	image.bmWidthBytes=((widthBits+31)/32)*4;
	BYTE_ARRAY fileBytes(infoHeader.biHeight*image.bmWidthBytes);
	image.bmBits = fileBytes;
	fread(image.bmBits,1,infoHeader.biHeight*image.bmWidthBytes,fp);
	fclose(fp);
	//
	BYTE byteConstArr[8]={0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};
	int bm32WidthBytes=infoHeader.biWidth*4;
	BYTE_ARRAY bit32bytes(infoHeader.biHeight*bm32WidthBytes);
	for(int rowi=0;rowi<infoHeader.biHeight;rowi++)
	{
		int row=infoHeader.biHeight-rowi-1;
		for(int i=0;i<infoHeader.biWidth;i++)
		{
			int colorindex=0;
			RGBQUAD crPixelColor;
			crPixelColor.rgbReserved=0;
			if(infoHeader.biBitCount==1)
			{	//单色图
				int ibyte=row*image.bmWidthBytes+i/8;
				if(fileBytes[ibyte]&byteConstArr[i%8])
					crPixelColor.rgbBlue=crPixelColor.rgbGreen=crPixelColor.rgbRed=255;
				else
					crPixelColor=palette[0];
			}
			else if(infoHeader.biBitCount==4)
			{	//16色图
				int ibyte=row*image.bmWidthBytes+i/2;
				if(i%2==0)
					colorindex=(fileBytes[ibyte]&0xf0)>>4;
				else
					colorindex=fileBytes[ibyte]&0x0f;
				crPixelColor=palette[colorindex];
			}
			else if(infoHeader.biBitCount==8)
			{	//256色图
				int ibyte=row*image.bmWidthBytes+i;
				colorindex=fileBytes[ibyte];
			}
			else if(infoHeader.biBitCount==24||infoHeader.biBitCount==32)
			{	//真彩24位或32位
				BYTE ciPixelBytes=infoHeader.biBitCount/8;
				int ibyte=row*image.bmWidthBytes+i*ciPixelBytes;
				memcpy(&crPixelColor,&fileBytes[ibyte],ciPixelBytes);
			}
			memcpy(&bit32bytes[rowi*bm32WidthBytes+i*4],&crPixelColor,4);
		}
	}

	image.bmBitsPixel=32;
	image.bmBits=bit32bytes;
	image.bmWidthBytes=bm32WidthBytes;
	if(!xBitMap.CreateBitmapIndirect(&image))
		return FALSE;
	return TRUE;
}
//////////////////////////////////////////////////////////////////////////
//
CImageRecord::CImageRecord()
{

}
CImageRecord::CImageRecord(char* sketch_file,char* image_file,char* sLabel)
{
	m_sSketchFile.Copy(sketch_file);
	m_sImageFile.Copy(image_file);
	m_sLabel.Copy(sLabel);
}
BOOL CImageRecord::GetSketchBitMap(CBitmap& xBitMap)
{
	return GetBitMap(m_sSketchFile,xBitMap);
}
BOOL CImageRecord::GetImageBitMap(CBitmap& xBitMap)
{
	return GetBitMap(m_sImageFile,xBitMap);
}
CXhChar50 CImageRecord::GetLabelStr()
{
	return m_sLabel;
}
/////////////////////////////////////////////////////////////////////////////
// CPromptDlg dialog

CPromptDlg *g_pPromptMsg;
int CPromptDlg::nScrLocationX=GetSystemMetrics(SM_CXSCREEN);
int CPromptDlg::nScrLocationY=0;
CPromptDlg::CPromptDlg(CWnd* pWnd)
{
	m_sPromptMsg = _T("");
	m_pWnd = pWnd;
	m_iType=0;
	m_idBmpResource=0;
	m_dwIconWidth=100;
	m_dwIconHeight=100;
	drawpane_func=NULL;
	m_bSelBitMap=FALSE;
	m_bFirstPopup=true;
	m_bEnableWindowMoveListen=false;
	m_ciWndPosType=0;
}

void CPromptDlg::SetUdfDrawFunc(void (*drawfunc)(void* pContext))// 设置预置重显函数
{
	drawpane_func=drawfunc;
}
void CPromptDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_MSG, m_sPromptMsg);
}


BEGIN_MESSAGE_MAP(CPromptDlg, CDialog)
	ON_WM_SIZE()
	ON_WM_MOVE()
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_ITEMS, OnItemchangedListCtrl)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPromptDlg message handlers
BOOL CPromptDlg::Create()
{
	return CDialog::Create(CPromptDlg::IDD);
}
BOOL CPromptDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	//初始化ListCtrol
	CListCtrl* pListCtrl=(CListCtrl*)GetDlgItem(IDC_LIST_ITEMS);
	if(pListCtrl)
	{
		pListCtrl->SetExtendedStyle(LVS_ICON|LVS_AUTOARRANGE);
		pListCtrl->SetBkColor(RGB(255,255,255));
		if(!m_bFirstPopup)
			imageList.DeleteImageList();
		imageList.Create(m_dwIconWidth,m_dwIconHeight,ILC_COLOR32,2,4);
		m_bFirstPopup=false;
		pListCtrl->SetImageList(&imageList,LVSIL_NORMAL);
		pListCtrl->SetIconSpacing(CSize(m_dwIconWidth+30,m_dwIconHeight+30));	//设置图片间隔
	}
	m_bBrush.CreateSolidBrush(RGB(253,247,198));
	//布局窗口显示
	RelayoutWnd();
	m_bEnableWindowMoveListen=true;
	m_bRelayout=true;
	return TRUE;  
}

void CPromptDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
}
void CPromptDlg::OnMove(int x, int y)
{
	CDialog::OnMove(x, y);

	if(m_bEnableWindowMoveListen&&!m_bRelayout)
	{	//传入的x,y参数是指客户区的左上角,而不是窗口的左上角 wjh-2016.10.22
		CRect rect;
		GetWindowRect(rect);
		nScrLocationX=rect.right;
		nScrLocationY=rect.top;
	}
}
void CPromptDlg::InitStartPos(int nClientW,int nClientH,BYTE ciWndPosType)
{
	int cx=GetSystemMetrics(SM_CXSCREEN);
	int cy=GetSystemMetrics(SM_CYSCREEN);
	if(ciWndPosType==POS_CENTER)
	{
		CPromptDlg::nScrLocationX=(int)(0.5*(cx+nClientW));
		CPromptDlg::nScrLocationY=(int)(0.5*(cy-nClientH));
	}
	else if(ciWndPosType==POS_LEFT_TOP)
	{
		CPromptDlg::nScrLocationX=nClientW;
		CPromptDlg::nScrLocationY=0;
	}
	else if(ciWndPosType==POS_LEFT_BOTTOM)
	{
		CPromptDlg::nScrLocationX=nClientW;
		CPromptDlg::nScrLocationY=cy-nClientH;
	}
	else if(ciWndPosType==POS_RIGHT_BOTTOM)
	{
		CPromptDlg::nScrLocationX=cx;
		CPromptDlg::nScrLocationY=cy-nClientH;
	}
	else //if(ciWndPosType==POS_RIGHT_TOP)
	{
		CPromptDlg::nScrLocationX=cx;
		CPromptDlg::nScrLocationY=0;
	}
}
void CPromptDlg::RelayoutWnd(bool bInitStartPos/*=false*/)
{
	CRect rect,client_rect;
	GetWindowRect(rect);
	GetClientRect(client_rect);
	long fWidth=rect.Width()-client_rect.Width();
	long fHight=rect.Height()-client_rect.Height();
	CEdit*     pEditCtrl=(CEdit*)GetDlgItem(IDC_MSG);
	CListCtrl* pListCtrl=(CListCtrl*)GetDlgItem(IDC_LIST_ITEMS);
	int nClientW=0,nClientH=0;
	BITMAP bmInfo;
	if(m_iType==0)
	{
		pListCtrl->ShowWindow(FALSE);
		pEditCtrl->ShowWindow(TRUE);
		pEditCtrl->MoveWindow(client_rect);
		nClientW=client_rect.Width();
		nClientH=client_rect.Height();
	}
	else if(m_iType==1)
	{
		pEditCtrl->ShowWindow(FALSE);
		pListCtrl->ShowWindow(FALSE);
		m_bBitMap.GetBitmap(&bmInfo);
		nClientW=bmInfo.bmWidth;
		nClientH=bmInfo.bmHeight;
	}
	else //if(m_iType==2)
	{
		pListCtrl->ShowWindow(TRUE);
		pEditCtrl->ShowWindow(FALSE);
		int num=pListCtrl->GetItemCount();
		int nColumns=num/2;
		if(num%2!=0)
			nColumns+=1;
		nClientW=nColumns*(m_dwIconWidth+30)+5;
		nClientH=2*(m_dwIconHeight+30)+5;
		pListCtrl->MoveWindow(CRect(0,0,nClientW,nClientH));
		if(m_bSelBitMap)
		{
			m_bBitMap.GetBitmap(&bmInfo);
			nClientH+=bmInfo.bmHeight;
		}
	}
	if(bInitStartPos)
		InitStartPos(nClientW,nClientH,m_ciWndPosType);
	rect.right=nScrLocationX;
	rect.left =rect.right-nClientW-fWidth;
	rect.top=nScrLocationY;
	rect.bottom =rect.top+nClientH+fHight;
	MoveWindow(rect,TRUE);
	m_bRelayout=false;
}
void CPromptDlg::OnItemchangedListCtrl(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	m_bSelBitMap=FALSE;
	m_bBitMap.DeleteObject();
	CListCtrl* pListCtrl=(CListCtrl*)GetDlgItem(IDC_LIST_ITEMS);
	POSITION pos = pListCtrl->GetFirstSelectedItemPosition();
	int iCurSel=pListCtrl->GetNextSelectedItem(pos);
	if(iCurSel>=0)
	{
		CImageRecord* pImageRec=(CImageRecord*)pListCtrl->GetItemData(iCurSel);
		if(pImageRec&&pImageRec->GetImageBitMap(m_bBitMap))
			m_bSelBitMap=TRUE;
		RelayoutWnd();
		Invalidate();
	}
	*pResult = 0;
}
HBRUSH CPromptDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	if(nCtlColor==CTLCOLOR_STATIC)
	{
		pDC->SetBkColor(RGB(253,247,199));
		return m_bBrush;
	}
	else
		return hbr;
}

void CPromptDlg::SetMsg(CString msg)
{
	m_idBmpResource=0;
	if(GetSafeHwnd()==0)
		Create();
	m_sPromptMsg = msg;
	m_iType=0;
	UpdateData(FALSE);
}
void CPromptDlg::SetPromptPicture(long bitmapId)
{
	bool needreload=!(m_bBitMap.GetSafeHandle()&&m_idBmpResource==bitmapId)||m_iType!=1||!m_bFromInternalResource;
	if(m_bBitMap.GetSafeHandle()&&needreload)
		m_bBitMap.DeleteObject();
	m_idBmpResource=bitmapId;
	if(needreload && !m_bBitMap.LoadBitmap(bitmapId))
	{
#ifdef AFX_TARG_ENU_ENGLISH
		AfxMessageBox("Please make sure the bitmap resources exist!");
#else 
		AfxMessageBox("请确定位图资源存在!");
#endif
		return;
	}
	if(GetSafeHwnd()==0)
		Create();
	m_iType=1;
	m_bFromInternalResource=true;
	RelayoutWnd();
	Invalidate();
}
void CPromptDlg::SetPromptPicture(const char* bmpfile)
{
	bool needreload=true;
	if(m_bBitMap.GetSafeHandle()&&needreload)
		m_bBitMap.DeleteObject();
	if(!GetBitMap(bmpfile,m_bBitMap))
		return;
	bitmapfile=bmpfile;
	if(GetSafeHwnd()==0)
		Create();
	m_iType=1;
	m_bFromInternalResource=false;
	RelayoutWnd();
	Invalidate();
}
void CPromptDlg::SetPromptPictures()
{
	while(imageList.GetImageCount()>0)
		imageList.Remove(0);		//清空图标
	CListCtrl* pListCtrl=(CListCtrl*)GetDlgItem(IDC_LIST_ITEMS);
	pListCtrl->DeleteAllItems();
	CBitmap bitmap;
	for(CImageRecord* pImageRec=m_xImageRecList.GetFirst();pImageRec;pImageRec=m_xImageRecList.GetNext())
	{
		if(pImageRec->GetSketchBitMap(bitmap)==FALSE)
			continue;
		int index=imageList.Add(&bitmap,RGB(0,0,0));
		int iItem=pListCtrl->InsertItem(pListCtrl->GetItemCount(),pImageRec->GetLabelStr(),index);
		pListCtrl->SetItemData(iItem,(DWORD)pImageRec);
	}
	//
	m_iType=2;
	RelayoutWnd();
	Invalidate();
}
BOOL CPromptDlg::Destroy()
{
	m_bBrush.DeleteObject();
	return DestroyWindow();	
}

BOOL CPromptDlg::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}
void CPromptDlg::OnPaint()
{
	CPaintDC dc(this);
	CDC MemDc;
	BITMAP bmInfo;
	if(m_iType==1)
	{
		m_bBitMap.GetBitmap(&bmInfo);	
		MemDc.CreateCompatibleDC(&dc);
		CBitmap* pOldMap=MemDc.SelectObject(&m_bBitMap);
		dc.BitBlt(0,0,bmInfo.bmWidth,bmInfo.bmHeight,&MemDc,0,0,SRCCOPY);
		MemDc.SelectObject(pOldMap);
	}
	else if(m_iType==2&&m_bSelBitMap)
	{
		CRect client_rect;
		GetClientRect(client_rect);
		m_bBitMap.GetBitmap(&bmInfo);
		int y=client_rect.Height()-bmInfo.bmHeight-5;
		int x=client_rect.Width()-bmInfo.bmWidth;
		dc.FillSolidRect(client_rect,RGB(255,255,255));
		//绘制示意图
		MemDc.CreateCompatibleDC(&dc);
		CBitmap* pOldMap=MemDc.SelectObject(&m_bBitMap);
		dc.BitBlt(int(x*0.5),y,bmInfo.bmWidth,bmInfo.bmHeight,&MemDc,0,0,SRCCOPY);
		MemDc.SelectObject(pOldMap);
	}
}

void CPromptDlg::OnCancel()
{
	CDialog::OnCancel();
}
/////////////////////////////////////////////////
CSysPromptObject::CSysPromptObject()
{
	m_pPromptWnd=NULL;
}
void CSysPromptObject::AssureValid(CWnd *pParentWnd/*=NULL*/)
{
	if(m_pPromptWnd==NULL)
	{
		if(pParentWnd==NULL)
			pParentWnd=AfxGetMainWnd();
		m_pPromptWnd=new CPromptDlg(pParentWnd);
	}
	if(m_pPromptWnd->GetSafeHwnd()==0)
		m_pPromptWnd->Create();
}
void CSysPromptObject::Close()
{
	if(m_pPromptWnd!=NULL&&m_pPromptWnd->GetSafeHwnd()>0)
		m_pPromptWnd->Destroy();
}
void CSysPromptObject::Prompt(const char* promptstr)
{
	//if(!g_sysPara.m_bEnablePrompt)//判断是否启动提示框
	//	return;
	HWND hFocusWnd=GetFocus();
	AssureValid();
	m_pPromptWnd->SetMsg(promptstr);
	SetFocus(hFocusWnd);	//恢复原有窗口的输入焦点
}

void CSysPromptObject::SetPromptPicture(long bitmapId)
{
	//if(!g_sysPara.m_bEnablePrompt)//判断是否启动提示框
	//	return;
	HWND hFocusWnd=GetFocus();
	AssureValid();
	m_pPromptWnd->SetPromptPicture(bitmapId);
	SetFocus(hFocusWnd);	//恢复原有窗口的输入焦点
}
void CSysPromptObject::SetPromptPicture(const char* bmpfile)
{
	//if(!g_sysPara.m_bEnablePrompt)//判断是否启动提示框
	//	return;
	HWND hFocusWnd=GetFocus();
	AssureValid();
	m_pPromptWnd->SetPromptPicture(bmpfile);
	SetFocus(hFocusWnd);	//恢复原有窗口的输入焦点
}
void CSysPromptObject::AddImageRecord(CImageRecord& imageRec)
{
	//if(!g_sysPara.m_bEnablePrompt)//判断是否启动提示框
	//	return;
	HWND hFocusWnd=GetFocus();
	if(m_pPromptWnd==NULL)
		AssureValid();
	m_pPromptWnd->m_xImageRecList.append(imageRec);
	SetFocus(hFocusWnd);	//恢复原有窗口的输入焦点
}
void CSysPromptObject::SetPromptPictures()
{
	//if(!g_sysPara.m_bEnablePrompt)//判断是否启动提示框
	//	return;
	HWND hFocusWnd=GetFocus();
	AssureValid();
	m_pPromptWnd->SetPromptPictures();
	SetFocus(hFocusWnd);	//恢复原有窗口的输入焦点
}
CSysPromptObject::~CSysPromptObject()
{
	if(m_pPromptWnd&&m_pPromptWnd->GetSafeHwnd()>0)
		m_pPromptWnd->Destroy();
	if(m_pPromptWnd)
		delete m_pPromptWnd;
	m_pPromptWnd=NULL;
}

