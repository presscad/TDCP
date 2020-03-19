#include "stdafx.h"
#include "GIMModle.h"
#include "ExcelOper.h"
#include "HashTable.h"
#include "ExcelColumn.h"
#include "direct.h"
#include "Buffer.h"
#include "io.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BOOL IsUTF8File(const char* mod_file)
{
	FILE *fp = fopen(mod_file,"rb");
	if(fp==NULL)
		return FALSE;
	fseek(fp, 0, SEEK_END);
	long lSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	CBuffer buffer;
	buffer.Write(NULL,lSize);
	fread(buffer.GetBufferPtr(),buffer.GetLength(),1,fp);
	fclose(fp);
	//
	unsigned char* start = (unsigned char*)buffer.GetBufferPtr();
	unsigned char* end = (unsigned char*)start + lSize;
	BOOL bIsUTF8 = TRUE;
	while(start<end)
	{
		if(*start<0x80)
		{	//1�ֽ�(0xxxxxxx): ֵС��0x80��ΪASCII�ַ�  	
			start++; 
		}
		else if(*start<(0xC0)) 
		{	//ֵ����0x80��0xC0֮���Ϊ��ЧUTF-8�ַ� 
			bIsUTF8=FALSE;  
			break;
		}
		else if(*start<(0xE0)) 
		{	//2�ֽ�(110xxxxx 10xxxxxx)
			if(start>=end-1)
				break;
			if((start[1]&(0xC0))!=0x80)
			{
				bIsUTF8=FALSE;
				break;
			}
			start+=2;
		}
		else if(*start<(0xF0)) 
		{	//3�ֽ�(1110xxxx 10xxxxxx 10xxxxx)
			if(start>=end-2)
				break;
			if((start[1]&(0xC0))!=0x80||(start[2]&(0xC0))!=0x80)
			{
				bIsUTF8=FALSE;
				break;
			}
			start+=3;
		}
		else if(*start<(0xF8))
		{	//4�ֽ�(11110xxx 10xxxxxx 10xxxxxx 10xxxxxx)
			if(start>=end-3)
				break;
			if((start[1]&(0xC0))!=0x80||(start[2]&(0xC0))!=0x80||(start[3]&(0xC0))!=0x80)
			{
				bIsUTF8=FALSE;
				break;
			}
			start+=4;
		}
		else if(*start<(0xFC))
		{	//5�ֽ�(111110xx	10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx)
			if(start>=end-4)
				break;
			if((start[1]&(0xC0))!=0x80||(start[2]&(0xC0))!=0x80||(start[3]&(0xC0))!=0x80||
				(start[4]&(0xC0))!=0x80)
			{
				bIsUTF8=FALSE;
				break;
			}
			start+=5;
		}
		else if(*start<(0xFE))
		{	//6�ֽ�(1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx)
			if(start>=end-5)
				break;
			if((start[1]&(0xC0))!=0x80||(start[2]&(0xC0))!=0x80||(start[3]&(0xC0))!=0x80||
				(start[4]&(0xC0))!=0x80||(start[5]&(0xC0))!=0x80)
			{
				bIsUTF8=FALSE;
				break;
			}
			start+=6;
		}
		else
		{
			bIsUTF8 = FALSE;
			break;
		}
	}
	if(!bIsUTF8)
	{	//�����ַ�ƥ������ٴν����ж�
		int goodbytes = 0, asciibytes = 0;
		int rawtextlen = lSize;
		char* rawtext=buffer.GetBufferPtr();
		for(int i = 0; i < rawtextlen; i++) 
		{
			if ((rawtext[i] &  0x7F) == rawtext[i]) 
			{	//���λ��0��ASCII�ַ�
				//һλ��������
				asciibytes++;
			} 
			else if (-64 <= rawtext[i] && rawtext[i] <= -33
				//��λ��������,��һλ11000000--11011111
				//��һλ��10000000--10111111
				&&i + 1 < rawtextlen 
				&& -128 <= rawtext[i + 1] && rawtext[i + 1] <= -65) 
			{
				goodbytes += 2;
				i++;
			} 
			else if (-32 <= rawtext[i]&& rawtext[i] <= -17
				//��λ��������,��һλ11100000--11101111
				//����λ��10000000--10111111
				&&i + 2 < rawtextlen 
				&& -128 <= rawtext[i + 1] && rawtext[i + 1] <= -65 
				&& -128 <= rawtext[i + 2] && rawtext[i + 2] <= -65) 
			{
				goodbytes += 3;
				i += 2;
			}
			else if(-16 <= rawtext[i]&& rawtext[i] <= -9
				//��λ��������,��һλ11110000--11110111
				//����λ��10000000--10111111
				&&i + 3 < rawtextlen 
				&&  -128 <= rawtext[i + 1] && rawtext[i + 1] <= -65 
				&& -128 <= rawtext[i + 2] && rawtext[i + 2] <= -65
				&& -128 <= rawtext[i + 3] && rawtext[i + 3] <= -65)

			{
				goodbytes += 4;
				i += 3;
			}
			else if(-8 <= rawtext[i]&& rawtext[i] <= -5
				//��λ��������,��һλ11111000--11111011
				//����λ��10000000--10111111
				&&i + 4 < rawtextlen 
				&&  -128 <= rawtext[i + 1] && rawtext[i + 1] <= -65 
				&& -128 <= rawtext[i + 2] && rawtext[i + 2] <= -65
				&& -128 <= rawtext[i + 3] && rawtext[i + 3] <= -65
				&& -128 <= rawtext[i + 4] && rawtext[i + 4] <= -65)
			{
				goodbytes += 5;
				i += 4;
			}
			else if(-4 <= rawtext[i]&& rawtext[i] <= -3
				//��λ��������,��һλ11111100--11111101
				//����λ��10000000--10111111
				&&i + 5 < rawtextlen 
				&&  -128 <= rawtext[i + 1] && rawtext[i + 1] <= -65 
				&& -128 <= rawtext[i + 2] && rawtext[i + 2] <= -65
				&& -128 <= rawtext[i + 3] && rawtext[i + 3] <= -65
				&& -128 <= rawtext[i + 4] && rawtext[i + 4] <= -65
				&& -128 <= rawtext[i + 5] && rawtext[i + 5] <= -65)
			{
				goodbytes += 6;
				i += 5;
			}
		}
		if (asciibytes == rawtextlen) 
			return true;
		int score=100*goodbytes/(rawtextlen - asciibytes);
		if (score>98) //���ƥ���ʴﵽ98%����,��ɹ�
			return true;
		else if(score>95&&goodbytes>30) 
			return true;
		else 
			return false;
	}
	return true;
}
void ANSIToUnicode(const char* src_str,wchar_t* des_str)
{
	int nLen=MultiByteToWideChar(CP_ACP,0,src_str,-1,NULL,0 );
	wchar_t* sResult=(wchar_t*)malloc((nLen+1)*sizeof(wchar_t));
	memset(sResult,0,sizeof(wchar_t)*(nLen+1));
	MultiByteToWideChar(CP_ACP,0,src_str,-1,(LPWSTR)sResult,nLen);
	//
	memset(des_str,0,MAX_PATH);
	wcscpy(des_str,sResult);
	free(sResult);
}
void DeleteDirectory(const char*  dirPath)
{
	if(access(dirPath, 0)==-1)
		return;	//�����ڴ�Ŀ¼
	char  path[MAX_PATH];
	strcpy(path,dirPath);
	strcat (path,"\\*");
	_finddata_t fb;   //������ͬ�����ļ��Ĵ洢�ṹ��
	long handle =_findfirst(path,&fb);
	if(handle != 0)
	{
		while(0 == _findnext(handle,&fb))
		{
			if(0 != strcmp(fb.name,".."))
			{	//windows�£����и�ϵͳ�ļ�����Ϊ��..��,������������
				memset(path,0,sizeof(path));
				strcpy(path,dirPath);
				strcat(path,"\\");
				strcat (path,fb.name);
				if(fb.attrib==16)
					DeleteDirectory(path);	//����ɾ�����ļ���
				else
					remove(path);	//ɾ���ļ�
			}	
		}
		_findclose(handle);
	}
	//�Ƴ��ļ���
	int iRet=rmdir(dirPath);
}
void SimplifiedNumString(char *sNumStr)
{
	CString num_str=sNumStr;
	int str_len = num_str.GetLength();
	int pointPos = -1;
	for(int i=str_len-1;i>=0;i--)
	{
		if(num_str[i]=='.')
		{
			pointPos=i;
			break;
		}
	}
	if(pointPos<0)
		return;
	if(pointPos>0&&!isdigit((BYTE)num_str[pointPos-1]))
		return; //С����ǰһλΪ����ʱ�Ž�����һ���ļ� wht 11-04-01
	int iPos=str_len-1;
	while(iPos>=pointPos)
	{
		if(num_str[iPos]=='0'||num_str[iPos]=='.')
			iPos--;
		else
			break;
	}
	num_str=num_str.Left(iPos+1);
	if(num_str.Compare("-0")==0)
		num_str="0";
	strcpy(sNumStr,num_str);
}
static CXhChar100 CreateGuidStr()
{
	CXhChar100 szGUID; 
	GUID guid;
	if(S_OK==::CoCreateGuid(&guid)) 
	{
		szGUID.Printf("%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x" 
			,guid.Data1,guid.Data2,guid.Data3,guid.Data4[0],guid.Data4[1] ,guid.Data4[2],
			guid.Data4[3],guid.Data4[4],guid.Data4[5],guid.Data4[6],guid.Data4[7]); 
	}
	return szGUID;
}
static CXhChar100 VariantToString(VARIANT value)
{
	CXhChar100 sValue;
	if(value.vt==VT_BSTR)
		return sValue.Copy(CString(value.bstrVal));
	else if(value.vt==VT_R8)
		return sValue.Copy(CXhChar100(value.dblVal));
	else if(value.vt==VT_R4)
		return sValue.Copy(CXhChar100(value.fltVal));
	else if(value.vt==VT_INT)
		return sValue.Copy(CXhChar100(value.intVal));
	else 
		return sValue;
}
//////////////////////////////////////////////////////////////////////////
//CTowerGimInfo
CGimModle gim_modle;
CTowerGim::CTowerGim()
{
	m_ciCodingType=ANSI_CODING;
}
CTowerGim::~CTowerGim()
{

}
void CTowerGim::CreateGuidFile()
{
	//�����ж��ļ��ı����ʽ
	CXhChar200 mod_file("%s\\%s.mod",(char*)m_sModPath,(char*)m_sModName);
	if(IsUTF8File(mod_file))
		m_ciCodingType=UTF8_CODING;
	else
		m_ciCodingType=ANSI_CODING;
	//�����Ŀ¼�����ɸ����ļ���
	CXhChar100 sOutPath=m_sOutputPath;
	sOutPath.Append("\\");
	sOutPath.Append(m_sModName);
	_mkdir(sOutPath);
	//�����Ŀ¼������MOD�ļ�
	CXhChar100 sFilePath=sOutPath;
	sFilePath.Append("\\MOD");
	_mkdir(sFilePath);
	CXhChar200 sModFilePath("%s\\%s.mod",(char*)sFilePath,(char*)m_sModName);
	WriteTowerModFile(sModFilePath);
	//��������MOD�ļ���PHM�ļ�
	sFilePath=sOutPath;
	sFilePath.Append("\\PHM");
	_mkdir(sFilePath);
	m_sPhmGuid=CreateGuidStr();
	CXhChar200 sPhmFilePath("%s\\%s.phm",(char*)sFilePath,(char*)m_sPhmGuid);
	WriteTowerPhmFile(sPhmFilePath,m_sModName);
	//��������PHM�ļ���DEV�ļ�����Ӧ�����ļ�
	sFilePath=sOutPath;
	sFilePath.Append("\\DEV");
	_mkdir(sFilePath);
	m_sFamGuid=m_sDevGuid=CreateGuidStr();
	CXhChar200 sFamFilePath("%s\\%s.fam",(char*)sFilePath,(char*)m_sFamGuid);
	WriteTowerFamFile(sFamFilePath);
	CXhChar200 sDevFilePath("%s\\%s.dev",(char*)sFilePath,(char*)m_sDevGuid);
	WriteTowerDevFile(sDevFilePath);
	//��������DEV�ļ����豸�����ļ�CBM
	sFilePath=sOutPath;
	sFilePath.Append("\\CBM");
	_mkdir(sFilePath);
	m_sCbmGuid=CreateGuidStr();
	CXhChar200 sCbmFilePath("%s\\%s.cbm",(char*)sFilePath,(char*)m_sCbmGuid);
	WriteTowerCbmFile(sCbmFilePath);
}
void CTowerGim::DeleteGuidFile()
{
	CXhChar100 sOutPath=m_sOutputPath;
	sOutPath.Append("\\");
	sOutPath.Append(m_sModName);
	CXhChar100 sFilePath=sOutPath;
	sFilePath.Append("\\MOD");
	DeleteDirectory(sFilePath);
	sFilePath=sOutPath;
	sFilePath.Append("\\PHM");
	DeleteDirectory(sFilePath);
	sFilePath=sOutPath;
	sFilePath.Append("\\DEV");
	DeleteDirectory(sFilePath);
	sFilePath=sOutPath;
	sFilePath.Append("\\CBM");
	DeleteDirectory(sFilePath);
	DeleteDirectory(sOutPath);
}
void CTowerGim::WriteTowerModFile(const char* sModFilePath)
{
	CXhChar200 mod_file("%s\\%s.mod",(char*)m_sModPath,(char*)m_sModName);
	FILE *fp = fopen(mod_file,"rb");
	if(fp==NULL)
	{
		logerr.Log(CXhChar100("%s��ȡʧ��!",(char*)mod_file));
		return;
	}
	fseek(fp,0,SEEK_END);
	long buf_size=ftell(fp);
	fseek(fp,0,SEEK_SET);
	CBuffer buffer;
	buffer.Write(NULL,buf_size);
	fread(buffer.GetBufferPtr(),buffer.GetLength(),1,fp);
	fclose(fp);
	//
	fp = fopen(sModFilePath,"wb");
	if(fp==NULL)
		return;
	fwrite(buffer.GetBufferPtr(),buffer.GetLength(),1,fp);
	fclose(fp);
}
void CTowerGim::WriteTowerPhmFile(const char* sPhmFile,char* sModName)
{
	FILE *fp=NULL;
	if(m_ciCodingType==ANSI_CODING)
		fp=fopen(sPhmFile,"wt");
	else
		fp=fopen(sPhmFile,"wt,ccs=UTF-8");
	if(fp==NULL)
		return;
	if(m_ciCodingType==ANSI_CODING)
	{	//ANSI
		fprintf(fp,"SOLIDMODELS.NUM=1\n");
		fprintf(fp,"SOLIDMODEL0=%s.mod\n",sModName);
		fprintf(fp,"TRANSFORMMATRIX0=1.000000,0.000000,0.000000,0.000000,0.000000,1.000000,0.000000,0.000000,0.000000,0.000000,1.000000,0.000000,0.000000,0.000000,0.000000,1.000000\n");
		fprintf(fp,"COLOR0=255,255,255\n");
	}
	else
	{	//UTF-8
		wchar_t sWValue[MAX_PATH];
		ANSIToUnicode(sModName,sWValue);
		fwprintf(fp,L"SOLIDMODELS.NUM=1\n");
		fwprintf(fp,L"SOLIDMODEL0=%s.mod\n",sWValue);
		fwprintf(fp,L"TRANSFORMMATRIX0=1.000000,0.000000,0.000000,0.000000,0.000000,1.000000,0.000000,0.000000,0.000000,0.000000,1.000000,0.000000,0.000000,0.000000,0.000000,1.000000\n");
		fwprintf(fp,L"COLOR0=255,255,255\n");
	}
	fclose(fp);
}
void CTowerGim::WriteTowerDevFile(const char* sDevFile)
{
	FILE *fp=NULL;
	if(m_ciCodingType==ANSI_CODING)
		fp=fopen(sDevFile,"wt");
	else
		fp=fopen(sDevFile,"wt,ccs=UTF-8");
	if(fp==NULL)
		return;
	if(m_ciCodingType==ANSI_CODING)
	{	//ANSI
		fprintf(fp,"DEVICETYPE = TOWER\n");
		fprintf(fp,"SYMBOLNAME = TOWER\n");
		fprintf(fp,"BASEFAMILYPOINTER=%s.fam\n",(char*)m_sFamGuid);
		fprintf(fp,"SOLIDMODELS.NUM = 1\n");
		fprintf(fp,"SOLIDMODEL0 = %s.phm\n",(char*)m_sPhmGuid);
		fprintf(fp,"TRANSFORMMATRIX0=1.000000,0.000000,0.000000,0.000000,0.000000,1.000000,0.000000,0.000000,0.000000,0.000000,1.000000,0.000000,0.000000,0.000000,0.000000,1.000000\n");
	}
	else
	{	//UTF-8
		wchar_t sWValue[MAX_PATH];
		fwprintf(fp,L"DEVICETYPE = TOWER\n");
		fwprintf(fp,L"SYMBOLNAME = TOWER\n");
		ANSIToUnicode(m_sFamGuid,sWValue);
		fwprintf(fp,L"BASEFAMILYPOINTER=%s.fam\n",sWValue);
		fwprintf(fp,L"SOLIDMODELS.NUM = 1\n");
		ANSIToUnicode(m_sPhmGuid,sWValue);
		fwprintf(fp,L"SOLIDMODEL0 = %s.phm\n",sWValue);
		fwprintf(fp,L"TRANSFORMMATRIX0=1.000000,0.000000,0.000000,0.000000,0.000000,1.000000,0.000000,0.000000,0.000000,0.000000,1.000000,0.000000,0.000000,0.000000,0.000000,1.000000\n");
	}
	fclose(fp);
}
void CTowerGim::WriteTowerFamFile(const char* sFamFile)
{
	FILE *fp=NULL;
	if(m_ciCodingType==ANSI_CODING)
		fp=fopen(sFamFile,"wt");
	else
		fp=fopen(sFamFile,"wt,ccs=UTF-8");
	if(fp==NULL)
		return;
	CXhChar50 ss;
	if(m_ciCodingType==ANSI_CODING)
	{
		//��ѹ�ȼ�
		fprintf(fp,"VOLTAGE = ��ѹ�ȼ� = %s\n",(char*)m_xTowerProp.m_sVoltGrade);
		//����
		fprintf(fp,"TYPE = ���� = %s\n",(char*)m_xTowerProp.m_sType);
		//��������
		fprintf(fp,"TEXTURE = �������� = %s\n",(char*)m_xTowerProp.m_sTexture);
		//�̶���ʽ
		fprintf(fp,"FIXEDTYPE = �̶���ʽ = %s\n",(char*)m_xTowerProp.m_sFixedType);
		//��������
		fprintf(fp,"TOWERTYPE = �������� = %s\n",(char*)m_xTowerProp.m_sTaType);
		//��·��
		fprintf(fp,"CIRCUIT = ��·�� = %d\n",m_xTowerProp.m_nCircuit);
		//�����ͺ�
		fprintf(fp,"CONDUCTOR = �����ͺ� = %s\n",(char*)m_xTowerProp.m_sCWireSpec);
		//�����ͺ�
		fprintf(fp,"GROUNDWIRE = �����ͺ� = %s\n",(char*)m_xTowerProp.m_sEWireSpec);
		//��������
		ss.Printf("%f",m_xTowerProp.m_fWindSpeed);
		SimplifiedNumString(ss);
		fprintf(fp,"REFERENCEWINDSPEED = ��ƻ������� = %s\n",(char*)ss);
		//�������
		ss.Printf("%f",m_xTowerProp.m_fNiceThick);
		SimplifiedNumString(ss);
		fprintf(fp,"DESIGNICETHICKNESS = ��Ƹ������ = %s\n",(char*)ss);
		//ˮƽ����
		fprintf(fp,"DESIGNWINDSPAN = ���ˮƽ���� = %s\n",(char*)m_xTowerProp.m_sWindSpan);
		//��ֱ����
		fprintf(fp,"DESIGNWEIGHTSPAN = ��ƴ�ֱ���� = %s\n",(char*)m_xTowerProp.m_sWeightSpan);
		//ǰ�������
		ss.Printf("%f",m_xTowerProp.m_fFrontRulingSpan);
		SimplifiedNumString(ss);
		fprintf(fp,"DESIGNFRONTRULINGSPAN = ���ǰ������� = %s\n",(char*)ss);
		//��������
		ss.Printf("%f",m_xTowerProp.m_fBackRulingSpan);
		SimplifiedNumString(ss);
		fprintf(fp,"DESIGNBACKRULINGSPAN=��ƺ�������=%s\n",(char*)ss);
		//��󵵾�
		ss.Printf("%f",m_xTowerProp.m_fMaxSpan);
		SimplifiedNumString(ss);
		fprintf(fp,"DESIGNMAXSPAN = �����󵵾� =%s\n",(char*)ss);
		//ת�Ƿ�Χ
		fprintf(fp,"ANGLERANGE = ���ת�Ƿ�Χ =%s\n",(char*)m_xTowerProp.m_sAngleRange);
		//Kv
		ss.Printf("%f",m_xTowerProp.m_fDesignKV);
		SimplifiedNumString(ss);
		fprintf(fp,"DESIGNKV = ���Kvֵ = %s\n",(char*)ss);
		//�������
		fprintf(fp,"RATEDNOMINALHEIGHT = ������� = %s\n",(char*)m_xTowerProp.m_sRatedHeight);
		//���߷�Χ
		fprintf(fp,"NOMINALHEIGHTRANGE = ���߷�Χ = %s\n",(char*)m_xTowerProp.m_sHeightRange);
		//����
		fprintf(fp,"TOWERWEIGHT = ���� = %s\n",(char*)m_xTowerProp.m_sTowerWeight);
		//��ƹ�Ƶҡ�ڽǶ�
		ss.Printf("%f",m_xTowerProp.m_fFrequencyRockAngle);
		SimplifiedNumString(ss);
		fprintf(fp,"ROCKANGLEOFDESIGNPOWERFREQUENCY = ��ƹ�Ƶҡ�ڽǶ� = %s\n",(char*)ss);
		//����׵�ҡ�ڽǶ�
		ss.Printf("%f",m_xTowerProp.m_fLightningRockAngle);
		SimplifiedNumString(ss);
		fprintf(fp,"ROCKANGLEOFDESIGNLIGHTNING = ����׵�ҡ�ڽǶ� = %s\n",(char*)ss);
		//��Ʋ���ҡ�ڽǶ�
		ss.Printf("%f",m_xTowerProp.m_fSwitchingRockAngle);
		SimplifiedNumString(ss);
		fprintf(fp,"ROCKANGLEOFDESIGNSWITCHING = ��Ʋ���ҡ�ڽǶ� = %s\n",(char*)ss);
		//��ƴ�����ҵҡ�ڽǶ�
		ss.Printf("%f",m_xTowerProp.m_fWorkingRockAngle);
		SimplifiedNumString(ss);
		fprintf(fp,"ROCKANGLEOFDESIGNLIVEWORKING = ��ƴ�����ҵҡ�ڽǶ� = %s\n",(char*)ss);
		//��������
		fprintf(fp,"MANUFACTURER = �������� = %s\n",(char*)m_xTowerProp.m_sManuFacturer);
		//���ʱ���
		fprintf(fp,"MATERIALCODE = ���ʱ��� = %s\n",(char*)m_xTowerProp.m_sMaterialCode);
		//װ��ģ�ͱ��
		fprintf(fp,"PROCESSMODELCODE = װ��ģ�ͱ�� = %s\n",(char*)m_xTowerProp.m_sProModelCode);
	}
	else
	{
		CXhChar500 sValue;
		wchar_t sWValue[MAX_PATH];
		//��ѹ�ȼ�
		sValue.Printf("VOLTAGE = ��ѹ�ȼ� = %s",(char*)m_xTowerProp.m_sVoltGrade);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//����
		sValue.Printf("TYPE = ���� = %s",(char*)m_xTowerProp.m_sType);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//��������
		sValue.Printf("TEXTURE = �������� = %s",(char*)m_xTowerProp.m_sTexture);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//�̶���ʽ
		sValue.Printf("FIXEDTYPE = �̶���ʽ = %s",(char*)m_xTowerProp.m_sFixedType);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//��������
		sValue.Printf("TOWERTYPE = �������� = %s",(char*)m_xTowerProp.m_sTaType);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//��·��
		sValue.Printf("CIRCUIT = ��·�� = %d",m_xTowerProp.m_nCircuit);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//�����ͺ�
		sValue.Printf("CONDUCTOR = �����ͺ� = %s",(char*)m_xTowerProp.m_sCWireSpec);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//�����ͺ�
		sValue.Printf("GROUNDWIRE = �����ͺ� = %s",(char*)m_xTowerProp.m_sEWireSpec);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//��ƻ�������
		ss.Printf("%f",m_xTowerProp.m_fWindSpeed);
		SimplifiedNumString(ss);
		sValue.Printf("REFERENCEWINDSPEED = ��ƻ������� = %s",(char*)ss);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//��Ƹ������
		ss.Printf("%f",m_xTowerProp.m_fNiceThick);
		SimplifiedNumString(ss);
		sValue.Printf("DESIGNICETHICKNESS = ��Ƹ������ = %s",(char*)ss);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//���ˮƽ����
		sValue.Printf("DESIGNWINDSPAN = ���ˮƽ���� = %s",(char*)m_xTowerProp.m_sWindSpan);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//��ƴ�ֱ����
		sValue.Printf("DESIGNWEIGHTSPAN = ��ƴ�ֱ���� = %s",(char*)m_xTowerProp.m_sWeightSpan);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//���ǰ�������
		ss.Printf("%f",m_xTowerProp.m_fFrontRulingSpan);
		SimplifiedNumString(ss);
		sValue.Printf("DESIGNFRONTRULINGSPAN = ���ǰ������� = %s",(char*)ss);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//��ƺ�������
		ss.Printf("%f",m_xTowerProp.m_fBackRulingSpan);
		SimplifiedNumString(ss);
		sValue.Printf("DESIGNBACKRULINGSPAN = ��ƺ������� = %s",(char*)ss);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//�����󵵾�
		ss.Printf("%f",m_xTowerProp.m_fMaxSpan);
		SimplifiedNumString(ss);
		sValue.Printf("DESIGNMAXSPAN = �����󵵾� = %s",(char*)ss);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//���ת�Ƿ�Χ
		sValue.Printf("ANGLERANGE = ���ת�Ƿ�Χ = %s",(char*)m_xTowerProp.m_sAngleRange);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//Kv
		ss.Printf("%f",m_xTowerProp.m_fDesignKV);
		SimplifiedNumString(ss);
		sValue.Printf("DESIGNKV = ���Kvֵ = %s",(char*)ss);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//�������
		sValue.Printf("RATEDNOMINALHEIGHT = ������� = %s",(char*)m_xTowerProp.m_sRatedHeight);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//���߷�Χ
		sValue.Printf("NOMINALHEIGHTRANGE = ���߷�Χ = %s",(char*)m_xTowerProp.m_sHeightRange);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//����
		sValue.Printf("TOWERWEIGHT = ���� = %s",(char*)m_xTowerProp.m_sTowerWeight);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//��ƹ�Ƶҡ�ڽǶ�
		ss.Printf("%f",m_xTowerProp.m_fFrequencyRockAngle);
		SimplifiedNumString(ss);
		sValue.Printf("ROCKANGLEOFDESIGNPOWERFREQUENCY = ��ƹ�Ƶҡ�ڽǶ� = %s",(char*)ss);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//����׵�ҡ�ڽǶ�
		ss.Printf("%f",m_xTowerProp.m_fLightningRockAngle);
		SimplifiedNumString(ss);
		sValue.Printf("ROCKANGLEOFDESIGNLIGHTNING = ����׵�ҡ�ڽǶ� = %s",(char*)ss);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//��Ʋ���ҡ�ڽǶ�
		ss.Printf("%f",m_xTowerProp.m_fSwitchingRockAngle);
		SimplifiedNumString(ss);
		sValue.Printf("ROCKANGLEOFDESIGNSWITCHING = ��Ʋ���ҡ�ڽǶ� = %s",(char*)ss);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//��ƴ�����ҵҡ�ڽǶ�
		ss.Printf("%f",m_xTowerProp.m_fWorkingRockAngle);
		SimplifiedNumString(ss);
		sValue.Printf("ROCKANGLEOFDESIGNLIVEWORKING = ��ƴ�����ҵҡ�ڽǶ� = %s",(char*)ss);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//��������
		sValue.Printf("MANUFACTURER = �������� = %s",(char*)m_xTowerProp.m_sManuFacturer);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//���ʱ���
		sValue.Printf("MATERIALCODE = ���ʱ��� = %s",(char*)m_xTowerProp.m_sMaterialCode);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//װ��ģ�ͱ��
		sValue.Printf("PROCESSMODELCODE = װ��ģ�ͱ�� = %s",(char*)m_xTowerProp.m_sProModelCode);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
	}
	fclose(fp);
}
void CTowerGim::WriteTowerCbmFile(const char* sCbmFile)
{
	FILE *fp=NULL;
	if(m_ciCodingType==ANSI_CODING)
		fp=fopen(sCbmFile,"wt");
	else
		fp=fopen(sCbmFile,"wt,ccs=UTF-8");
	if(fp==NULL)
		return;
	if(m_ciCodingType==ANSI_CODING)
	{
		fprintf(fp,"ENTITYNAME = Device\n");
		fprintf(fp,"OBJECTMODELPOINTER = %s.dev\n",(char*)m_sDevGuid);
		fprintf(fp,"BASEFAMILY = \n");
		fprintf(fp,"TRANSFORMMATRIX = 0.000000,0.000000,-0.000000,0.000000,-0.000000,0.000000,-0.000000,0.000000,-0.000000,0.000000,1.000000,0.000000,-0.000000,0.000000,-0.000000,1.000000\n");
	}
	else
	{
		wchar_t sWValue[MAX_PATH];
		ANSIToUnicode(m_sDevGuid,sWValue);
		fwprintf(fp,L"ENTITYNAME = Device\n");
		fwprintf(fp,L"OBJECTMODELPOINTER = %s.dev\n",sWValue);
		fwprintf(fp,L"BASEFAMILY = \n");
		fwprintf(fp,L"TRANSFORMMATRIX = 0.000000,0.000000,-0.000000,0.000000,-0.000000,0.000000,-0.000000,0.000000,-0.000000,0.000000,1.000000,0.000000,-0.000000,0.000000,-0.000000,1.000000\n");
	}
	fclose(fp);
}
void CTowerGim::ToBuffer(CBuffer &buffer,CBuffer &zipBuffer)
{
	//��ȡ��ǰʱ�䣬��ת��Ϊ������
	if(strlen(m_xGimHeadProp.m_sTime)<=0)
	{
		time_t tt = time(0);
		char sTime[32]={0};
		strftime(sTime, sizeof(sTime),"%Y-%m-%d %H:%M", localtime(&tt));
		memcpy(m_xGimHeadProp.m_sTime,sTime,16);
	}
	CXhString szBuffSize(m_xGimHeadProp.m_sBufSize,8);
	szBuffSize.Printf("%d",zipBuffer.GetLength());
	//д��ָ��ͷ����
	buffer.Write(m_xGimHeadProp.m_sFileTag,16);			//�ļ���ʶ
	buffer.Write(m_xGimHeadProp.m_sFileName,256);		//�ļ�����
	buffer.Write(m_xGimHeadProp.m_sDesigner,64);		//�����
	buffer.Write(m_xGimHeadProp.m_sUnit,256);			//��֯��λ
	buffer.Write(m_xGimHeadProp.m_sSoftName,128);		//�������
	buffer.Write(m_xGimHeadProp.m_sTime,16);			//����ʱ��		
	buffer.Write(m_xGimHeadProp.m_sSoftMajorVer,8);		//������汾��
	buffer.Write(m_xGimHeadProp.m_sSoftMinorVer,8);		//����ΰ汾��
	buffer.Write(m_xGimHeadProp.m_sMajorVersion,8);		//��׼���汾��
	buffer.Write(m_xGimHeadProp.m_sMinorVersion,8);		//��׼�ΰ汾��
	buffer.Write(m_xGimHeadProp.m_sBufSize,8);			//�洢�����С
	//д��7Z����
	buffer.Write(zipBuffer.GetBufferPtr(),zipBuffer.GetLength());
}
void CTowerGim::FromBuffer(CBuffer &buffer,CBuffer &zipBuffer)
{
	buffer.SeekToBegin();
	//��ȡͷ����
	buffer.Read(m_xGimHeadProp.m_sFileTag,16);		//�ļ���ʶ
	buffer.Read(m_xGimHeadProp.m_sFileName,256);	//�ļ�����
	buffer.Read(m_xGimHeadProp.m_sDesigner,64);		//�����
	buffer.Read(m_xGimHeadProp.m_sUnit,256);		//��֯��λ
	buffer.Read(m_xGimHeadProp.m_sSoftName,128);	//�������
	buffer.Read(m_xGimHeadProp.m_sTime,16);			//����ʱ��		
	buffer.Read(m_xGimHeadProp.m_sSoftMajorVer,8);	//������汾��
	buffer.Read(m_xGimHeadProp.m_sSoftMinorVer,8);	//����ΰ汾��
	buffer.Read(m_xGimHeadProp.m_sMajorVersion,8);	//��׼���汾��
	buffer.Read(m_xGimHeadProp.m_sMinorVersion,8);	//��׼�ΰ汾��
	buffer.Read(m_xGimHeadProp.m_sBufSize,8);		//�洢�����С
	//��ȡ7Z���ִ洢����
	int buf_size=atoi(m_xGimHeadProp.m_sBufSize);
	zipBuffer.Write(NULL,buf_size);
	buffer.Read(zipBuffer.GetBufferPtr(),buf_size);		//�洢�����С
}
//����ѹ���������
BOOL CTowerGim::PackTowerGimFile()
{
	//Ĭ��ѹ��Ϊ7Z��ʽ��
	CXhChar100 sOutPath=m_sOutputPath;
	sOutPath.Append("\\");
	sOutPath.Append(m_sModName);
	CXhChar100 gim_path("%s\\%s.gim",(char*)m_sOutputPath,(char*)m_sModName);
	//7z.exe V18.5֧��ѹ�����Զ�ɾ���ļ���
	//CXhChar200 cmd_str("7z.exe a %s %s\\* -sdel",(char*)gim_path,(char*)sOutPath);
	//7z.exe V4.65��֧���Զ�ɾ���ļ���
	CXhChar200 cmd_str("7z.exe a %s %s\\*",(char*)gim_path,(char*)sOutPath);
	if(!CGimModle::Run7zCmd(cmd_str))
		return FALSE;
	//ɾ����ʱ���ɵ��ļ���
	DeleteGuidFile();
	return TRUE;
}
BOOL CTowerGim::UpdateGimFile()
{
	//��ȡ7zѹ����Ĭ�ϵ��ֽ�
	//CXhChar100 sOutPath=m_sOutputPath;
	//sOutPath.Append("\\");
	//sOutPath.Append(m_sModName);
	CXhChar100 gim_path("%s\\%s.gim",(char*)m_sOutputPath,(char*)m_sModName);
	FILE* fp=fopen(gim_path,"rb");
	if(fp==NULL)
	{
		logerr.Log(CXhChar100("%s��������ʧ��(��)!",(char*)gim_path));
		return FALSE;
	}
	fseek(fp,0,SEEK_END);
	long buf_size=ftell(fp);
	fseek(fp,0,SEEK_SET);
	CBuffer zipBuf;
	zipBuf.Write(NULL,buf_size);
	fread(zipBuf.GetBufferPtr(),zipBuf.GetLength(),1,fp);
	fclose(fp);
	//��GIM�ļ�ͷ������ض�����
	CBuffer gimBuf(10000000);
	ToBuffer(gimBuf,zipBuf);
	fp=fopen(gim_path,"wb");
	if(fp==NULL)
	{
		logerr.Log(CXhChar100("%s��������ʧ��(д)!",(char*)gim_path));
		return FALSE;
	}
	fwrite(gimBuf.GetBufferPtr(),gimBuf.GetLength(),1,fp);
	fclose(fp);
	return TRUE;
}
//////////////////////////////////////////////////////////////////////////
//CGimModle
CGimModle::CGimModle()
{
}
CGimModle::~CGimModle()
{

}
BOOL CGimModle::Run7zCmd(char* sCmd)
{
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(pi));
	if (!CreateProcess(NULL,sCmd,NULL,NULL,FALSE,CREATE_NO_WINDOW,NULL, NULL, &si,&pi))
	{
		logerr.Log("��������ʧ��");
		return FALSE;
	}
	//�ȴ����̽���
	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return TRUE;
}
BOOL CGimModle::ParseTaModSheetContent(CVariant2dArray &sheetContentMap)
{
	if(sheetContentMap.RowsCount()<1)
		return FALSE;
	m_listTowerGim.Empty();
	//��һ�����м�¼������Ϣ
	GIM_HEAD_PROP_ITEM head_info;
	char sModPath[MAX_PATH]={0},sOutPath[MAX_PATH]={0};
	VARIANT value;
	sheetContentMap.GetValueAt(1,0,value);
	strcpy(sModPath,VariantToString(value));	//MOD·��
	sheetContentMap.GetValueAt(1,1,value);
	strcpy(sOutPath,VariantToString(value));	//���·��
	sheetContentMap.GetValueAt(1,2,value);
	strcpy(head_info.m_sDesigner,VariantToString(value));//�����
	sheetContentMap.GetValueAt(1,3,value);
	strcpy(head_info.m_sUnit,VariantToString(value));	//��֯��λ
	sheetContentMap.GetValueAt(1,4,value);
	strcpy(head_info.m_sSoftMajorVer,VariantToString(value));	//������汾
	sheetContentMap.GetValueAt(1,5,value);
	strcpy(head_info.m_sSoftMinorVer,VariantToString(value));	//����ΰ汾
	sheetContentMap.GetValueAt(1,6,value);
	strcpy(head_info.m_sMajorVersion,VariantToString(value));	//��׼���汾
	sheetContentMap.GetValueAt(1,7,value);
	strcpy(head_info.m_sMinorVersion,VariantToString(value));	//��׼�ΰ汾
	if(strlen(sModPath)<=0 || strlen(sOutPath)<=0)
		return FALSE;
	//�����м�¼�����У�����������¼�����
	CHashStrList<DWORD> hashColIndexByColTitle;
	for(int i=0;i<TA_MOD_EXCEL_COL_COUNT;i++)
	{
		VARIANT value;
		sheetContentMap.GetValueAt(2,i,value);
		if(CString(value.bstrVal).CompareNoCase(T_MOD_NAME1)==0) 
			hashColIndexByColTitle.SetValue(T_MOD_NAME1,i);
		else if(CString(value.bstrVal).CompareNoCase(T_MOD_NAME2)==0) 
			hashColIndexByColTitle.SetValue(T_MOD_NAME2,i);
		else if(CString(value.bstrVal).CompareNoCase(T_OUT_PATH)==0)
			hashColIndexByColTitle.SetValue(T_OUT_PATH,i);
		else if(CString(value.bstrVal).CompareNoCase(T_DESIGNER)==0)
			hashColIndexByColTitle.SetValue(T_DESIGNER,i);
		else if(CString(value.bstrVal).CompareNoCase(T_UNIT)==0)
			hashColIndexByColTitle.SetValue(T_UNIT,i);
		else if(CString(value.bstrVal).CompareNoCase(T_SOFTMAJORVER)==0)
			hashColIndexByColTitle.SetValue(T_SOFTMAJORVER,i);
		else if(CString(value.bstrVal).CompareNoCase(T_SOFTMINORVER)==0)
			hashColIndexByColTitle.SetValue(T_SOFTMINORVER,i);
		else if(CString(value.bstrVal).CompareNoCase(T_MAJORVERSION)==0)
			hashColIndexByColTitle.SetValue(T_MAJORVERSION,i);
		else if(CString(value.bstrVal).CompareNoCase(T_MINORVERSION)==0)
			hashColIndexByColTitle.SetValue(T_MINORVERSION,i);
		else if(CString(value.bstrVal).CompareNoCase(T_VOLTGRADE)==0)
			hashColIndexByColTitle.SetValue(T_VOLTGRADE,i);
		else if(CString(value.bstrVal).CompareNoCase(T_TYPE)==0)
			hashColIndexByColTitle.SetValue(T_TYPE,i);
		else if(CString(value.bstrVal).CompareNoCase(T_TEXTURE)==0)
			hashColIndexByColTitle.SetValue(T_TEXTURE,i);
		else if(CString(value.bstrVal).CompareNoCase(T_FIXEDTYPE)==0)
			hashColIndexByColTitle.SetValue(T_FIXEDTYPE,i);
		else if(CString(value.bstrVal).CompareNoCase(T_TATYPE)==0)
			hashColIndexByColTitle.SetValue(T_TATYPE,i);
		else if(CString(value.bstrVal).CompareNoCase(T_CIRCUIT)==0)
			hashColIndexByColTitle.SetValue(T_CIRCUIT,i);
		else if(CString(value.bstrVal).CompareNoCase(T_CWIRESPEC)==0)
			hashColIndexByColTitle.SetValue(T_CWIRESPEC,i);
		else if(CString(value.bstrVal).CompareNoCase(T_EWIRESPEC)==0)
			hashColIndexByColTitle.SetValue(T_EWIRESPEC,i);
		else if(CString(value.bstrVal).CompareNoCase(T_WINDSPEED)==0)
			hashColIndexByColTitle.SetValue(T_WINDSPEED,i);
		else if(CString(value.bstrVal).CompareNoCase(T_NICETHICK)==0)
			hashColIndexByColTitle.SetValue(T_NICETHICK,i);
		else if(CString(value.bstrVal).CompareNoCase(T_WINDSPAN)==0)
			hashColIndexByColTitle.SetValue(T_WINDSPAN,i);
		else if(CString(value.bstrVal).CompareNoCase(T_WEIGHTSPAN)==0)
			hashColIndexByColTitle.SetValue(T_WEIGHTSPAN,i);
		else if(CString(value.bstrVal).CompareNoCase(T_FRONTRULINGSPAN)==0)
			hashColIndexByColTitle.SetValue(T_FRONTRULINGSPAN,i);
		else if(CString(value.bstrVal).CompareNoCase(T_BACKRULINGSPAN)==0)
			hashColIndexByColTitle.SetValue(T_BACKRULINGSPAN,i);
		else if(CString(value.bstrVal).CompareNoCase(T_MAXSPAN)==0)
			hashColIndexByColTitle.SetValue(T_MAXSPAN,i);
		else if(CString(value.bstrVal).CompareNoCase(T_ANGLERANGE)==0)
			hashColIndexByColTitle.SetValue(T_ANGLERANGE,i);
		else if(CString(value.bstrVal).CompareNoCase(T_DESIGNKV)==0)
			hashColIndexByColTitle.SetValue(T_DESIGNKV,i);
		else if(CString(value.bstrVal).CompareNoCase(T_RATEDHEIGHT)==0)
			hashColIndexByColTitle.SetValue(T_RATEDHEIGHT,i);
		else if(CString(value.bstrVal).CompareNoCase(T_HEIGHTRANGE)==0)
			hashColIndexByColTitle.SetValue(T_HEIGHTRANGE,i);
		else if(CString(value.bstrVal).CompareNoCase(T_TOWERWEIGHT)==0)
			hashColIndexByColTitle.SetValue(T_TOWERWEIGHT,i);
		else if(CString(value.bstrVal).CompareNoCase(T_FREQUENCYROCKANGLE)==0)
			hashColIndexByColTitle.SetValue(T_FREQUENCYROCKANGLE,i);
		else if(CString(value.bstrVal).CompareNoCase(T_LIGHTNINGROCKANGLE)==0)
			hashColIndexByColTitle.SetValue(T_LIGHTNINGROCKANGLE,i);
		else if(CString(value.bstrVal).CompareNoCase(T_SWITCHINGROCKANGLE)==0)
			hashColIndexByColTitle.SetValue(T_SWITCHINGROCKANGLE,i);
		else if(CString(value.bstrVal).CompareNoCase(T_WORKINGROCKANGLE)==0)
			hashColIndexByColTitle.SetValue(T_WORKINGROCKANGLE,i);
		else if(CString(value.bstrVal).CompareNoCase(T_MANUFACTURER)==0)
			hashColIndexByColTitle.SetValue(T_MANUFACTURER,i);
		else if(CString(value.bstrVal).CompareNoCase(T_MATERIALCODE)==0)
			hashColIndexByColTitle.SetValue(T_MATERIALCODE,i);
		else if(CString(value.bstrVal).CompareNoCase(T_PROMODELCODE)==0)
			hashColIndexByColTitle.SetValue(T_PROMODELCODE,i);
	}
	//
	DWORD *pColIndex=NULL;
	CTowerGim* pTowerGimInfo=NULL;
	for(int i=3;i<=sheetContentMap.RowsCount();i++)
	{
		VARIANT value;
		//MOD�ļ�����
		pColIndex=hashColIndexByColTitle.GetValue(T_MOD_NAME1);
		if(pColIndex==NULL)
			pColIndex=hashColIndexByColTitle.GetValue(T_MOD_NAME2);
		if(pColIndex==NULL)
		{
			logerr.Log("��ȡ��Excle�ļ���ģ���ļ���һ��!");
			return FALSE;
		}
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		if(value.vt==VT_EMPTY)
			continue;
		CXhChar50 sModeName;
		_splitpath(VariantToString(value),NULL,NULL,sModeName,NULL);
		//
		pTowerGimInfo=m_listTowerGim.append();
		pTowerGimInfo->m_sModName=sModeName;
		pTowerGimInfo->m_sModPath.Copy(sModPath);
		pTowerGimInfo->m_sOutputPath.Copy(sOutPath);
		strcpy(pTowerGimInfo->m_xGimHeadProp.m_sFileName,pTowerGimInfo->m_sModName);
		strcpy(pTowerGimInfo->m_xGimHeadProp.m_sDesigner,head_info.m_sDesigner);
		strcpy(pTowerGimInfo->m_xGimHeadProp.m_sUnit,head_info.m_sUnit);
		strcpy(pTowerGimInfo->m_xGimHeadProp.m_sSoftMajorVer,head_info.m_sSoftMajorVer);
		strcpy(pTowerGimInfo->m_xGimHeadProp.m_sSoftMinorVer,head_info.m_sSoftMinorVer);
		strcpy(pTowerGimInfo->m_xGimHeadProp.m_sMajorVersion,head_info.m_sMajorVersion);
		strcpy(pTowerGimInfo->m_xGimHeadProp.m_sMinorVersion,head_info.m_sMinorVersion);
		//��ѹ�ȼ�
		pColIndex=hashColIndexByColTitle.GetValue(T_VOLTGRADE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_sVoltGrade=VariantToString(value);
		//�ͺ�
		pColIndex=hashColIndexByColTitle.GetValue(T_TYPE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_sType=VariantToString(value);
		//��������
		pColIndex=hashColIndexByColTitle.GetValue(T_TEXTURE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_sTexture=VariantToString(value);
		//�̶���ʽ
		pColIndex=hashColIndexByColTitle.GetValue(T_FIXEDTYPE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_sFixedType=VariantToString(value);
		//��������
		pColIndex=hashColIndexByColTitle.GetValue(T_TATYPE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_sTaType=VariantToString(value);
		//��·��
		pColIndex=hashColIndexByColTitle.GetValue(T_CIRCUIT);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_nCircuit=atoi(VariantToString(value));
		//�����ͺ�
		pColIndex=hashColIndexByColTitle.GetValue(T_CWIRESPEC);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_sCWireSpec=VariantToString(value);
		//�����ͺ�
		pColIndex=hashColIndexByColTitle.GetValue(T_EWIRESPEC);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_sEWireSpec=VariantToString(value);
		//��ƻ�������
		pColIndex=hashColIndexByColTitle.GetValue(T_WINDSPEED);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_fWindSpeed=atof(VariantToString(value));
		//��Ƹ������
		pColIndex=hashColIndexByColTitle.GetValue(T_NICETHICK);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_fNiceThick=atof(VariantToString(value));
		//���ˮƽ����
		pColIndex=hashColIndexByColTitle.GetValue(T_WINDSPAN);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_sWindSpan=VariantToString(value);
		//��ƴ�ֱ����
		pColIndex=hashColIndexByColTitle.GetValue(T_WEIGHTSPAN);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_sWeightSpan=VariantToString(value);
		//���ǰ�������
		pColIndex=hashColIndexByColTitle.GetValue(T_FRONTRULINGSPAN);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_fFrontRulingSpan=atof(VariantToString(value));
		//��ƺ�������
		pColIndex=hashColIndexByColTitle.GetValue(T_BACKRULINGSPAN);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_fBackRulingSpan=atof(VariantToString(value));
		//�����󵵾�
		pColIndex=hashColIndexByColTitle.GetValue(T_MAXSPAN);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_fMaxSpan=atof(VariantToString(value));
		//���ת�Ƿ�Χ
		pColIndex=hashColIndexByColTitle.GetValue(T_ANGLERANGE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_sAngleRange=VariantToString(value);
		//���Kvֵ
		pColIndex=hashColIndexByColTitle.GetValue(T_DESIGNKV);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_fDesignKV=atof(VariantToString(value));
		//�������
		pColIndex=hashColIndexByColTitle.GetValue(T_RATEDHEIGHT);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_sRatedHeight=VariantToString(value);
		//���߷�Χ
		pColIndex=hashColIndexByColTitle.GetValue(T_HEIGHTRANGE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_sHeightRange=VariantToString(value);
		//����
		pColIndex=hashColIndexByColTitle.GetValue(T_TOWERWEIGHT);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_sTowerWeight=VariantToString(value);
		//��ƹ�Ƶҡ�ڽǶ�
		pColIndex=hashColIndexByColTitle.GetValue(T_FREQUENCYROCKANGLE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_fFrequencyRockAngle=atof(VariantToString(value));
		//����׵�ҡ�ڽǶ�
		pColIndex=hashColIndexByColTitle.GetValue(T_LIGHTNINGROCKANGLE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_fLightningRockAngle=atof(VariantToString(value));
		//��Ʋ���ҡ�ڽǶ�
		pColIndex=hashColIndexByColTitle.GetValue(T_SWITCHINGROCKANGLE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_fSwitchingRockAngle=atof(VariantToString(value));
		//��ƴ�����ҵҡ�ڽǶ�
		pColIndex=hashColIndexByColTitle.GetValue(T_WORKINGROCKANGLE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_fWorkingRockAngle=atof(VariantToString(value));
		//��������
		pColIndex=hashColIndexByColTitle.GetValue(T_MANUFACTURER);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_sManuFacturer=VariantToString(value);
		//���ʱ���
		pColIndex=hashColIndexByColTitle.GetValue(T_MATERIALCODE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_sMaterialCode=VariantToString(value);
		//װ��ģ�ͱ��
		pColIndex=hashColIndexByColTitle.GetValue(T_PROMODELCODE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_sProModelCode=VariantToString(value);
	}
	return TRUE;
}
BOOL CGimModle::ReadTaModBatchGzipFile(const char* sFilePath)
{
	CExcelOperObject xExcelObj;
	if(!xExcelObj.OpenExcelFile(sFilePath))
		return FALSE;
	LPDISPATCH pWorksheets=xExcelObj.GetWorksheets();
	ASSERT(pWorksheets != NULL);
	Sheets       excel_sheets;	//��������
	excel_sheets.AttachDispatch(pWorksheets);
	int nSheetNum=excel_sheets.GetCount();
	if(nSheetNum<1)
	{
		excel_sheets.ReleaseDispatch();
		return FALSE;
	}
	LPDISPATCH pWorksheet=excel_sheets.GetItem(COleVariant((short) 1));
	_Worksheet  excel_sheet;
	excel_sheet.AttachDispatch(pWorksheet);
	excel_sheet.Select();
	//1.��ȡExcelָ��Sheet���ݴ洢��sheetContentMap
	Range excel_usedRange,excel_range;
	excel_usedRange.AttachDispatch(excel_sheet.GetUsedRange());
	excel_range.AttachDispatch(excel_usedRange.GetRows());
	long nRowNum = excel_range.GetCount();
	excel_range.AttachDispatch(excel_usedRange.GetColumns());
	long nColNum = excel_range.GetCount();
	if(nColNum>=TA_MOD_EXCEL_COL_COUNT)
	{
		CXhChar50 sCellS("A1"),sCellE=CExcelOper::GetCellPos(nColNum,nRowNum);
		LPDISPATCH pRange = excel_sheet.GetRange(COleVariant(sCellS),COleVariant(sCellE));
		excel_range.AttachDispatch(pRange,FALSE);
		CVariant2dArray sheetContentMap(1,1);
		sheetContentMap.var=excel_range.GetValue();
		excel_range.ReleaseDispatch();
		//2����������
		if(!ParseTaModSheetContent(sheetContentMap))
		{
			excel_sheet.ReleaseDispatch();
			return FALSE;
		}
		excel_sheet.ReleaseDispatch();
	}
	excel_sheets.ReleaseDispatch();
	return TRUE;
}