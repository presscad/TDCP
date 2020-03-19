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
		{	//1字节(0xxxxxxx): 值小于0x80的为ASCII字符  	
			start++; 
		}
		else if(*start<(0xC0)) 
		{	//值介于0x80与0xC0之间的为无效UTF-8字符 
			bIsUTF8=FALSE;  
			break;
		}
		else if(*start<(0xE0)) 
		{	//2字节(110xxxxx 10xxxxxx)
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
		{	//3字节(1110xxxx 10xxxxxx 10xxxxx)
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
		{	//4字节(11110xxx 10xxxxxx 10xxxxxx 10xxxxxx)
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
		{	//5字节(111110xx	10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx)
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
		{	//6字节(1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx)
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
	{	//根据字符匹配比例再次进行判断
		int goodbytes = 0, asciibytes = 0;
		int rawtextlen = lSize;
		char* rawtext=buffer.GetBufferPtr();
		for(int i = 0; i < rawtextlen; i++) 
		{
			if ((rawtext[i] &  0x7F) == rawtext[i]) 
			{	//最高位是0的ASCII字符
				//一位编码的情况
				asciibytes++;
			} 
			else if (-64 <= rawtext[i] && rawtext[i] <= -33
				//两位编码的情况,第一位11000000--11011111
				//后一位跟10000000--10111111
				&&i + 1 < rawtextlen 
				&& -128 <= rawtext[i + 1] && rawtext[i + 1] <= -65) 
			{
				goodbytes += 2;
				i++;
			} 
			else if (-32 <= rawtext[i]&& rawtext[i] <= -17
				//三位编码的情况,第一位11100000--11101111
				//后两位跟10000000--10111111
				&&i + 2 < rawtextlen 
				&& -128 <= rawtext[i + 1] && rawtext[i + 1] <= -65 
				&& -128 <= rawtext[i + 2] && rawtext[i + 2] <= -65) 
			{
				goodbytes += 3;
				i += 2;
			}
			else if(-16 <= rawtext[i]&& rawtext[i] <= -9
				//四位编码的情况,第一位11110000--11110111
				//后三位跟10000000--10111111
				&&i + 3 < rawtextlen 
				&&  -128 <= rawtext[i + 1] && rawtext[i + 1] <= -65 
				&& -128 <= rawtext[i + 2] && rawtext[i + 2] <= -65
				&& -128 <= rawtext[i + 3] && rawtext[i + 3] <= -65)

			{
				goodbytes += 4;
				i += 3;
			}
			else if(-8 <= rawtext[i]&& rawtext[i] <= -5
				//五位编码的情况,第一位11111000--11111011
				//后四位跟10000000--10111111
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
				//六位编码的情况,第一位11111100--11111101
				//后五位跟10000000--10111111
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
		if (score>98) //如果匹配率达到98%以上,则成功
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
		return;	//不存在此目录
	char  path[MAX_PATH];
	strcpy(path,dirPath);
	strcat (path,"\\*");
	_finddata_t fb;   //查找相同属性文件的存储结构体
	long handle =_findfirst(path,&fb);
	if(handle != 0)
	{
		while(0 == _findnext(handle,&fb))
		{
			if(0 != strcmp(fb.name,".."))
			{	//windows下，常有个系统文件，名为“..”,对它不做处理
				memset(path,0,sizeof(path));
				strcpy(path,dirPath);
				strcat(path,"\\");
				strcat (path,fb.name);
				if(fb.attrib==16)
					DeleteDirectory(path);	//迭代删除子文件夹
				else
					remove(path);	//删除文件
			}	
		}
		_findclose(handle);
	}
	//移除文件夹
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
		return; //小数点前一位为数字时才进行下一步的简化 wht 11-04-01
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
	//智能判断文件的编码格式
	CXhChar200 mod_file("%s\\%s.mod",(char*)m_sModPath,(char*)m_sModName);
	if(IsUTF8File(mod_file))
		m_ciCodingType=UTF8_CODING;
	else
		m_ciCodingType=ANSI_CODING;
	//在输出目录下生成该塔文件夹
	CXhChar100 sOutPath=m_sOutputPath;
	sOutPath.Append("\\");
	sOutPath.Append(m_sModName);
	_mkdir(sOutPath);
	//在输出目录下生成MOD文件
	CXhChar100 sFilePath=sOutPath;
	sFilePath.Append("\\MOD");
	_mkdir(sFilePath);
	CXhChar200 sModFilePath("%s\\%s.mod",(char*)sFilePath,(char*)m_sModName);
	WriteTowerModFile(sModFilePath);
	//生成引用MOD文件的PHM文件
	sFilePath=sOutPath;
	sFilePath.Append("\\PHM");
	_mkdir(sFilePath);
	m_sPhmGuid=CreateGuidStr();
	CXhChar200 sPhmFilePath("%s\\%s.phm",(char*)sFilePath,(char*)m_sPhmGuid);
	WriteTowerPhmFile(sPhmFilePath,m_sModName);
	//生成引用PHM文件的DEV文件及相应属性文件
	sFilePath=sOutPath;
	sFilePath.Append("\\DEV");
	_mkdir(sFilePath);
	m_sFamGuid=m_sDevGuid=CreateGuidStr();
	CXhChar200 sFamFilePath("%s\\%s.fam",(char*)sFilePath,(char*)m_sFamGuid);
	WriteTowerFamFile(sFamFilePath);
	CXhChar200 sDevFilePath("%s\\%s.dev",(char*)sFilePath,(char*)m_sDevGuid);
	WriteTowerDevFile(sDevFilePath);
	//生成引用DEV文件的设备工程文件CBM
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
		logerr.Log(CXhChar100("%s读取失败!",(char*)mod_file));
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
		//电压等级
		fprintf(fp,"VOLTAGE = 电压等级 = %s\n",(char*)m_xTowerProp.m_sVoltGrade);
		//塔型
		fprintf(fp,"TYPE = 塔型 = %s\n",(char*)m_xTowerProp.m_sType);
		//杆塔材质
		fprintf(fp,"TEXTURE = 杆塔材质 = %s\n",(char*)m_xTowerProp.m_sTexture);
		//固定方式
		fprintf(fp,"FIXEDTYPE = 固定方式 = %s\n",(char*)m_xTowerProp.m_sFixedType);
		//杆塔类型
		fprintf(fp,"TOWERTYPE = 杆塔类型 = %s\n",(char*)m_xTowerProp.m_sTaType);
		//回路数
		fprintf(fp,"CIRCUIT = 回路数 = %d\n",m_xTowerProp.m_nCircuit);
		//导线型号
		fprintf(fp,"CONDUCTOR = 导线型号 = %s\n",(char*)m_xTowerProp.m_sCWireSpec);
		//地线型号
		fprintf(fp,"GROUNDWIRE = 地线型号 = %s\n",(char*)m_xTowerProp.m_sEWireSpec);
		//基本风速
		ss.Printf("%f",m_xTowerProp.m_fWindSpeed);
		SimplifiedNumString(ss);
		fprintf(fp,"REFERENCEWINDSPEED = 设计基本风速 = %s\n",(char*)ss);
		//覆冰厚度
		ss.Printf("%f",m_xTowerProp.m_fNiceThick);
		SimplifiedNumString(ss);
		fprintf(fp,"DESIGNICETHICKNESS = 设计覆冰厚度 = %s\n",(char*)ss);
		//水平档距
		fprintf(fp,"DESIGNWINDSPAN = 设计水平档距 = %s\n",(char*)m_xTowerProp.m_sWindSpan);
		//垂直档距
		fprintf(fp,"DESIGNWEIGHTSPAN = 设计垂直档距 = %s\n",(char*)m_xTowerProp.m_sWeightSpan);
		//前侧代表档距
		ss.Printf("%f",m_xTowerProp.m_fFrontRulingSpan);
		SimplifiedNumString(ss);
		fprintf(fp,"DESIGNFRONTRULINGSPAN = 设计前侧代表档距 = %s\n",(char*)ss);
		//后侧代表档距
		ss.Printf("%f",m_xTowerProp.m_fBackRulingSpan);
		SimplifiedNumString(ss);
		fprintf(fp,"DESIGNBACKRULINGSPAN=设计后侧代表档距=%s\n",(char*)ss);
		//最大档距
		ss.Printf("%f",m_xTowerProp.m_fMaxSpan);
		SimplifiedNumString(ss);
		fprintf(fp,"DESIGNMAXSPAN = 设计最大档距 =%s\n",(char*)ss);
		//转角范围
		fprintf(fp,"ANGLERANGE = 设计转角范围 =%s\n",(char*)m_xTowerProp.m_sAngleRange);
		//Kv
		ss.Printf("%f",m_xTowerProp.m_fDesignKV);
		SimplifiedNumString(ss);
		fprintf(fp,"DESIGNKV = 设计Kv值 = %s\n",(char*)ss);
		//计算呼高
		fprintf(fp,"RATEDNOMINALHEIGHT = 计算呼高 = %s\n",(char*)m_xTowerProp.m_sRatedHeight);
		//呼高范围
		fprintf(fp,"NOMINALHEIGHTRANGE = 呼高范围 = %s\n",(char*)m_xTowerProp.m_sHeightRange);
		//塔重
		fprintf(fp,"TOWERWEIGHT = 塔重 = %s\n",(char*)m_xTowerProp.m_sTowerWeight);
		//设计工频摇摆角度
		ss.Printf("%f",m_xTowerProp.m_fFrequencyRockAngle);
		SimplifiedNumString(ss);
		fprintf(fp,"ROCKANGLEOFDESIGNPOWERFREQUENCY = 设计工频摇摆角度 = %s\n",(char*)ss);
		//设计雷电摇摆角度
		ss.Printf("%f",m_xTowerProp.m_fLightningRockAngle);
		SimplifiedNumString(ss);
		fprintf(fp,"ROCKANGLEOFDESIGNLIGHTNING = 设计雷电摇摆角度 = %s\n",(char*)ss);
		//设计操作摇摆角度
		ss.Printf("%f",m_xTowerProp.m_fSwitchingRockAngle);
		SimplifiedNumString(ss);
		fprintf(fp,"ROCKANGLEOFDESIGNSWITCHING = 设计操作摇摆角度 = %s\n",(char*)ss);
		//设计带电作业摇摆角度
		ss.Printf("%f",m_xTowerProp.m_fWorkingRockAngle);
		SimplifiedNumString(ss);
		fprintf(fp,"ROCKANGLEOFDESIGNLIVEWORKING = 设计带电作业摇摆角度 = %s\n",(char*)ss);
		//生产厂家
		fprintf(fp,"MANUFACTURER = 生产厂家 = %s\n",(char*)m_xTowerProp.m_sManuFacturer);
		//物资编码
		fprintf(fp,"MATERIALCODE = 物资编码 = %s\n",(char*)m_xTowerProp.m_sMaterialCode);
		//装配模型编号
		fprintf(fp,"PROCESSMODELCODE = 装配模型编号 = %s\n",(char*)m_xTowerProp.m_sProModelCode);
	}
	else
	{
		CXhChar500 sValue;
		wchar_t sWValue[MAX_PATH];
		//电压等级
		sValue.Printf("VOLTAGE = 电压等级 = %s",(char*)m_xTowerProp.m_sVoltGrade);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//塔型
		sValue.Printf("TYPE = 塔型 = %s",(char*)m_xTowerProp.m_sType);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//杆塔材质
		sValue.Printf("TEXTURE = 杆塔材质 = %s",(char*)m_xTowerProp.m_sTexture);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//固定方式
		sValue.Printf("FIXEDTYPE = 固定方式 = %s",(char*)m_xTowerProp.m_sFixedType);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//杆塔类型
		sValue.Printf("TOWERTYPE = 杆塔类型 = %s",(char*)m_xTowerProp.m_sTaType);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//回路数
		sValue.Printf("CIRCUIT = 回路数 = %d",m_xTowerProp.m_nCircuit);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//导线型号
		sValue.Printf("CONDUCTOR = 导线型号 = %s",(char*)m_xTowerProp.m_sCWireSpec);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//地线型号
		sValue.Printf("GROUNDWIRE = 地线型号 = %s",(char*)m_xTowerProp.m_sEWireSpec);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//设计基本风速
		ss.Printf("%f",m_xTowerProp.m_fWindSpeed);
		SimplifiedNumString(ss);
		sValue.Printf("REFERENCEWINDSPEED = 设计基本风速 = %s",(char*)ss);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//设计覆冰厚度
		ss.Printf("%f",m_xTowerProp.m_fNiceThick);
		SimplifiedNumString(ss);
		sValue.Printf("DESIGNICETHICKNESS = 设计覆冰厚度 = %s",(char*)ss);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//设计水平档距
		sValue.Printf("DESIGNWINDSPAN = 设计水平档距 = %s",(char*)m_xTowerProp.m_sWindSpan);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//设计垂直档距
		sValue.Printf("DESIGNWEIGHTSPAN = 设计垂直档距 = %s",(char*)m_xTowerProp.m_sWeightSpan);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//设计前侧代表档距
		ss.Printf("%f",m_xTowerProp.m_fFrontRulingSpan);
		SimplifiedNumString(ss);
		sValue.Printf("DESIGNFRONTRULINGSPAN = 设计前侧代表档距 = %s",(char*)ss);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//设计后侧代表档距
		ss.Printf("%f",m_xTowerProp.m_fBackRulingSpan);
		SimplifiedNumString(ss);
		sValue.Printf("DESIGNBACKRULINGSPAN = 设计后侧代表档距 = %s",(char*)ss);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//设计最大档距
		ss.Printf("%f",m_xTowerProp.m_fMaxSpan);
		SimplifiedNumString(ss);
		sValue.Printf("DESIGNMAXSPAN = 设计最大档距 = %s",(char*)ss);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//设计转角范围
		sValue.Printf("ANGLERANGE = 设计转角范围 = %s",(char*)m_xTowerProp.m_sAngleRange);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//Kv
		ss.Printf("%f",m_xTowerProp.m_fDesignKV);
		SimplifiedNumString(ss);
		sValue.Printf("DESIGNKV = 设计Kv值 = %s",(char*)ss);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//计算呼高
		sValue.Printf("RATEDNOMINALHEIGHT = 计算呼高 = %s",(char*)m_xTowerProp.m_sRatedHeight);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//呼高范围
		sValue.Printf("NOMINALHEIGHTRANGE = 呼高范围 = %s",(char*)m_xTowerProp.m_sHeightRange);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//塔重
		sValue.Printf("TOWERWEIGHT = 塔重 = %s",(char*)m_xTowerProp.m_sTowerWeight);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//设计工频摇摆角度
		ss.Printf("%f",m_xTowerProp.m_fFrequencyRockAngle);
		SimplifiedNumString(ss);
		sValue.Printf("ROCKANGLEOFDESIGNPOWERFREQUENCY = 设计工频摇摆角度 = %s",(char*)ss);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//设计雷电摇摆角度
		ss.Printf("%f",m_xTowerProp.m_fLightningRockAngle);
		SimplifiedNumString(ss);
		sValue.Printf("ROCKANGLEOFDESIGNLIGHTNING = 设计雷电摇摆角度 = %s",(char*)ss);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//设计操作摇摆角度
		ss.Printf("%f",m_xTowerProp.m_fSwitchingRockAngle);
		SimplifiedNumString(ss);
		sValue.Printf("ROCKANGLEOFDESIGNSWITCHING = 设计操作摇摆角度 = %s",(char*)ss);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//设计带电作业摇摆角度
		ss.Printf("%f",m_xTowerProp.m_fWorkingRockAngle);
		SimplifiedNumString(ss);
		sValue.Printf("ROCKANGLEOFDESIGNLIVEWORKING = 设计带电作业摇摆角度 = %s",(char*)ss);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//生产厂家
		sValue.Printf("MANUFACTURER = 生产厂家 = %s",(char*)m_xTowerProp.m_sManuFacturer);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//物资编码
		sValue.Printf("MATERIALCODE = 物资编码 = %s",(char*)m_xTowerProp.m_sMaterialCode);
		ANSIToUnicode(sValue,sWValue);
		fwprintf(fp,L"%s\n",sWValue);
		//装配模型编号
		sValue.Printf("PROCESSMODELCODE = 装配模型编号 = %s",(char*)m_xTowerProp.m_sProModelCode);
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
	//获取当前时间，并转换为数字型
	if(strlen(m_xGimHeadProp.m_sTime)<=0)
	{
		time_t tt = time(0);
		char sTime[32]={0};
		strftime(sTime, sizeof(sTime),"%Y-%m-%d %H:%M", localtime(&tt));
		memcpy(m_xGimHeadProp.m_sTime,sTime,16);
	}
	CXhString szBuffSize(m_xGimHeadProp.m_sBufSize,8);
	szBuffSize.Printf("%d",zipBuffer.GetLength());
	//写入指定头部分
	buffer.Write(m_xGimHeadProp.m_sFileTag,16);			//文件标识
	buffer.Write(m_xGimHeadProp.m_sFileName,256);		//文件名称
	buffer.Write(m_xGimHeadProp.m_sDesigner,64);		//设计者
	buffer.Write(m_xGimHeadProp.m_sUnit,256);			//组织单位
	buffer.Write(m_xGimHeadProp.m_sSoftName,128);		//软件名称
	buffer.Write(m_xGimHeadProp.m_sTime,16);			//创建时间		
	buffer.Write(m_xGimHeadProp.m_sSoftMajorVer,8);		//软件主版本号
	buffer.Write(m_xGimHeadProp.m_sSoftMinorVer,8);		//软件次版本号
	buffer.Write(m_xGimHeadProp.m_sMajorVersion,8);		//标准主版本号
	buffer.Write(m_xGimHeadProp.m_sMinorVersion,8);		//标准次版本号
	buffer.Write(m_xGimHeadProp.m_sBufSize,8);			//存储区域大小
	//写入7Z部分
	buffer.Write(zipBuffer.GetBufferPtr(),zipBuffer.GetLength());
}
void CTowerGim::FromBuffer(CBuffer &buffer,CBuffer &zipBuffer)
{
	buffer.SeekToBegin();
	//读取头部分
	buffer.Read(m_xGimHeadProp.m_sFileTag,16);		//文件标识
	buffer.Read(m_xGimHeadProp.m_sFileName,256);	//文件名称
	buffer.Read(m_xGimHeadProp.m_sDesigner,64);		//设计者
	buffer.Read(m_xGimHeadProp.m_sUnit,256);		//组织单位
	buffer.Read(m_xGimHeadProp.m_sSoftName,128);	//软件名称
	buffer.Read(m_xGimHeadProp.m_sTime,16);			//创建时间		
	buffer.Read(m_xGimHeadProp.m_sSoftMajorVer,8);	//软件主版本号
	buffer.Read(m_xGimHeadProp.m_sSoftMinorVer,8);	//软件次版本号
	buffer.Read(m_xGimHeadProp.m_sMajorVersion,8);	//标准主版本号
	buffer.Read(m_xGimHeadProp.m_sMinorVersion,8);	//标准次版本号
	buffer.Read(m_xGimHeadProp.m_sBufSize,8);		//存储区域大小
	//读取7Z部分存储部分
	int buf_size=atoi(m_xGimHeadProp.m_sBufSize);
	zipBuffer.Write(NULL,buf_size);
	buffer.Read(zipBuffer.GetBufferPtr(),buf_size);		//存储区域大小
}
//进行压缩打包处理
BOOL CTowerGim::PackTowerGimFile()
{
	//默认压缩为7Z格式包
	CXhChar100 sOutPath=m_sOutputPath;
	sOutPath.Append("\\");
	sOutPath.Append(m_sModName);
	CXhChar100 gim_path("%s\\%s.gim",(char*)m_sOutputPath,(char*)m_sModName);
	//7z.exe V18.5支持压缩后自动删除文件夹
	//CXhChar200 cmd_str("7z.exe a %s %s\\* -sdel",(char*)gim_path,(char*)sOutPath);
	//7z.exe V4.65不支持自动删除文件夹
	CXhChar200 cmd_str("7z.exe a %s %s\\*",(char*)gim_path,(char*)sOutPath);
	if(!CGimModle::Run7zCmd(cmd_str))
		return FALSE;
	//删除临时生成的文件夹
	DeleteGuidFile();
	return TRUE;
}
BOOL CTowerGim::UpdateGimFile()
{
	//读取7z压缩包默认的字节
	//CXhChar100 sOutPath=m_sOutputPath;
	//sOutPath.Append("\\");
	//sOutPath.Append(m_sModName);
	CXhChar100 gim_path("%s\\%s.gim",(char*)m_sOutputPath,(char*)m_sModName);
	FILE* fp=fopen(gim_path,"rb");
	if(fp==NULL)
	{
		logerr.Log(CXhChar100("%s更新内容失败(读)!",(char*)gim_path));
		return FALSE;
	}
	fseek(fp,0,SEEK_END);
	long buf_size=ftell(fp);
	fseek(fp,0,SEEK_SET);
	CBuffer zipBuf;
	zipBuf.Write(NULL,buf_size);
	fread(zipBuf.GetBufferPtr(),zipBuf.GetLength(),1,fp);
	fclose(fp);
	//在GIM文件头部添加特定内容
	CBuffer gimBuf(10000000);
	ToBuffer(gimBuf,zipBuf);
	fp=fopen(gim_path,"wb");
	if(fp==NULL)
	{
		logerr.Log(CXhChar100("%s更新内容失败(写)!",(char*)gim_path));
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
		logerr.Log("进程启动失败");
		return FALSE;
	}
	//等待进程结束
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
	//第一、二行记录基本信息
	GIM_HEAD_PROP_ITEM head_info;
	char sModPath[MAX_PATH]={0},sOutPath[MAX_PATH]={0};
	VARIANT value;
	sheetContentMap.GetValueAt(1,0,value);
	strcpy(sModPath,VariantToString(value));	//MOD路径
	sheetContentMap.GetValueAt(1,1,value);
	strcpy(sOutPath,VariantToString(value));	//输出路径
	sheetContentMap.GetValueAt(1,2,value);
	strcpy(head_info.m_sDesigner,VariantToString(value));//设计者
	sheetContentMap.GetValueAt(1,3,value);
	strcpy(head_info.m_sUnit,VariantToString(value));	//组织单位
	sheetContentMap.GetValueAt(1,4,value);
	strcpy(head_info.m_sSoftMajorVer,VariantToString(value));	//软件主版本
	sheetContentMap.GetValueAt(1,5,value);
	strcpy(head_info.m_sSoftMinorVer,VariantToString(value));	//软件次版本
	sheetContentMap.GetValueAt(1,6,value);
	strcpy(head_info.m_sMajorVersion,VariantToString(value));	//标准主版本
	sheetContentMap.GetValueAt(1,7,value);
	strcpy(head_info.m_sMinorVersion,VariantToString(value));	//标准次版本
	if(strlen(sModPath)<=0 || strlen(sOutPath)<=0)
		return FALSE;
	//第三行记录属性列，根据列名记录列序号
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
		//MOD文件名称
		pColIndex=hashColIndexByColTitle.GetValue(T_MOD_NAME1);
		if(pColIndex==NULL)
			pColIndex=hashColIndexByColTitle.GetValue(T_MOD_NAME2);
		if(pColIndex==NULL)
		{
			logerr.Log("读取的Excle文件与模板文件不一致!");
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
		//电压等级
		pColIndex=hashColIndexByColTitle.GetValue(T_VOLTGRADE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_sVoltGrade=VariantToString(value);
		//型号
		pColIndex=hashColIndexByColTitle.GetValue(T_TYPE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_sType=VariantToString(value);
		//杆塔材质
		pColIndex=hashColIndexByColTitle.GetValue(T_TEXTURE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_sTexture=VariantToString(value);
		//固定方式
		pColIndex=hashColIndexByColTitle.GetValue(T_FIXEDTYPE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_sFixedType=VariantToString(value);
		//杆塔类型
		pColIndex=hashColIndexByColTitle.GetValue(T_TATYPE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_sTaType=VariantToString(value);
		//回路数
		pColIndex=hashColIndexByColTitle.GetValue(T_CIRCUIT);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_nCircuit=atoi(VariantToString(value));
		//导线型号
		pColIndex=hashColIndexByColTitle.GetValue(T_CWIRESPEC);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_sCWireSpec=VariantToString(value);
		//地线型号
		pColIndex=hashColIndexByColTitle.GetValue(T_EWIRESPEC);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_sEWireSpec=VariantToString(value);
		//设计基本风速
		pColIndex=hashColIndexByColTitle.GetValue(T_WINDSPEED);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_fWindSpeed=atof(VariantToString(value));
		//设计覆冰厚度
		pColIndex=hashColIndexByColTitle.GetValue(T_NICETHICK);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_fNiceThick=atof(VariantToString(value));
		//设计水平档距
		pColIndex=hashColIndexByColTitle.GetValue(T_WINDSPAN);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_sWindSpan=VariantToString(value);
		//设计垂直档距
		pColIndex=hashColIndexByColTitle.GetValue(T_WEIGHTSPAN);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_sWeightSpan=VariantToString(value);
		//设计前侧代表档距
		pColIndex=hashColIndexByColTitle.GetValue(T_FRONTRULINGSPAN);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_fFrontRulingSpan=atof(VariantToString(value));
		//设计后侧代表档距
		pColIndex=hashColIndexByColTitle.GetValue(T_BACKRULINGSPAN);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_fBackRulingSpan=atof(VariantToString(value));
		//设计最大档距
		pColIndex=hashColIndexByColTitle.GetValue(T_MAXSPAN);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_fMaxSpan=atof(VariantToString(value));
		//设计转角范围
		pColIndex=hashColIndexByColTitle.GetValue(T_ANGLERANGE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_sAngleRange=VariantToString(value);
		//设计Kv值
		pColIndex=hashColIndexByColTitle.GetValue(T_DESIGNKV);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_fDesignKV=atof(VariantToString(value));
		//计算呼高
		pColIndex=hashColIndexByColTitle.GetValue(T_RATEDHEIGHT);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_sRatedHeight=VariantToString(value);
		//呼高范围
		pColIndex=hashColIndexByColTitle.GetValue(T_HEIGHTRANGE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_sHeightRange=VariantToString(value);
		//塔重
		pColIndex=hashColIndexByColTitle.GetValue(T_TOWERWEIGHT);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_sTowerWeight=VariantToString(value);
		//设计工频摇摆角度
		pColIndex=hashColIndexByColTitle.GetValue(T_FREQUENCYROCKANGLE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_fFrequencyRockAngle=atof(VariantToString(value));
		//设计雷电摇摆角度
		pColIndex=hashColIndexByColTitle.GetValue(T_LIGHTNINGROCKANGLE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_fLightningRockAngle=atof(VariantToString(value));
		//设计操作摇摆角度
		pColIndex=hashColIndexByColTitle.GetValue(T_SWITCHINGROCKANGLE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_fSwitchingRockAngle=atof(VariantToString(value));
		//设计带电作业摇摆角度
		pColIndex=hashColIndexByColTitle.GetValue(T_WORKINGROCKANGLE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_fWorkingRockAngle=atof(VariantToString(value));
		//生产厂家
		pColIndex=hashColIndexByColTitle.GetValue(T_MANUFACTURER);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_sManuFacturer=VariantToString(value);
		//物资编码
		pColIndex=hashColIndexByColTitle.GetValue(T_MATERIALCODE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pTowerGimInfo->m_xTowerProp.m_sMaterialCode=VariantToString(value);
		//装配模型编号
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
	Sheets       excel_sheets;	//工作表集合
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
	//1.获取Excel指定Sheet内容存储至sheetContentMap
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
		//2、解析数据
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