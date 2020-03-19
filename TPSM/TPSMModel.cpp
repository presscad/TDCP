#include "stdafx.h"
#include "TpsmModel.h"
#include "ExcelOper.h"
#include "CryptBuffer.h"
#include "Markup.h"
#include "ModCore.h"
#include "SortFunc.h"
#include "TowerTypeTemp.h"
#include "DxfFile.h"
#include "I3DS.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CProjectManager Manager;
//////////////////////////////////////////////////////////////////////////
//
static const int EXCEL_COL_COUNT	= 14;
static const char* T_INDEX			= "检索号";
static const char* T_VOLTGRADE		= "电压等级";
static const char* T_TOWERNAME		= "塔名";
static const char* T_SHAPETYPE		= "铁塔类型";
static const char* T_MATERIAL		= "角钢塔";
static const char* T_CIRCUIT		= "回路数";
static const char* T_ALTITUDE		= "海拔高度";
static const char* T_WINDSPEED		= "设计风速";
static const char* T_NICETHICK		= "设计冰厚";
static const char* T_CHECKNT		= "验算冰厚";
static const char* T_WINDSPAN		= "水平档距";
static const char* T_WEIGHTSPAN		= "垂直档距";
static const char* T_RULINGSPAN		= "代表档距";
static const char* T_ANGLE			= "转角度数";
//
CXhChar100 VariantToString(VARIANT value)
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
static int comparefun(const NODE_PTR& item1,const NODE_PTR& item2)
{
	return compare_long(item1->pointI,item2->pointI);
}
static BOOL IsHasValidPointI(CHashSet<NODE_PTR>& hashLinkNode)
{
	CHashList<BOOL> hashByPointI;
	for(CLDSNode* pNode=hashLinkNode.GetFirst();pNode;pNode=hashLinkNode.GetNext())
	{
		if(pNode->point_i<=0)
			return FALSE;
		if(hashByPointI.GetValue(pNode->point_i))
			return FALSE;
		hashByPointI.SetValue(pNode->point_i,TRUE);
	}
	return TRUE;
}
GECS TransToUcs(MOD_CS modCs)
{
	GECS cs;
	cs.origin.Set(modCs.origin.x,modCs.origin.y,modCs.origin.z);
	cs.axis_x.Set(modCs.axisX.x,modCs.axisX.y,modCs.axisX.z);
	cs.axis_y.Set(modCs.axisY.x,modCs.axisY.y,modCs.axisY.z);
	cs.axis_z.Set(modCs.axisZ.x,modCs.axisZ.y,modCs.axisZ.z);
	return cs;
}
//////////////////////////////////////////////////////////////////////////
//CTPSMModel
char CTPSMModel::lds_file_v[20]="1,3,10,0";
CTPSMModel::CTPSMModel()
{
	m_pBelongPrj=NULL;
	m_xTower.InitTower();
	m_nMaxLegs=8;
}
CTPSMModel::~CTPSMModel()
{

}
void CTPSMModel::ReadTowerTempFile(const char* sLdsFile,BOOL bInitDesPara/*=FALSE*/)
{
	FILE *fp=fopen(sLdsFile,"rt");
	if(fp==NULL)
	{
		logerr.Log("%s模板文件打开失败！",(char*)sLdsFile);
		return;
	}
	fclose(fp);
	//
	m_xTower.Empty();
	m_xTower.InitTower();
	DWORD cursor_pipeline_no,buffer_len;
	CString sDocTypeName,sFileVersion;
	CFile *pFile=new CFile(sLdsFile,CFile::modeRead);
	CArchive *pAr=new CArchive(pFile,CArchive::load);
	*pAr>>sDocTypeName;
	*pAr>>sFileVersion;
	*pAr>>m_xTower.user_pipeline_no;
	*pAr>>cursor_pipeline_no;
	*pAr>>buffer_len;
	BYTE_ARRAY buffpool(buffer_len);
	pAr->Read(buffpool, buffer_len);
	CBuffer buffer(buffpool, buffer_len);
	_snprintf(m_xTower.version, 19, "%s", sFileVersion);
	char bEncryptByAES = false;
	if (compareVersion(m_xTower.version, "1.3.8.0") >= 0)
		bEncryptByAES = 2;
	else if (compareVersion(m_xTower.version, "1.3.0.0") >= 0)
		bEncryptByAES = true;
	DecryptBuffer(buffer, bEncryptByAES, cursor_pipeline_no);
	m_xTower.FromBuffer(buffer, 4);
	//附加缓冲
	DWORD buf_len = 0;
	DWORD dwRead = pAr->Read(&buf_len, 4);
	if (dwRead == 4 && buf_len > 0)
	{
		CHAR_ARRAY buf(buf_len);
		CBuffer attachBuffer(buf, buf_len);
		pAr->Read(buf, buf_len);
		m_xTower.FromAttachBuffer(attachBuffer);
	}
	if(pAr)
		pAr->Close();
	if(pFile)
		pFile->Close();
	delete pAr;
	delete pFile;
	//挂点模型缓存
	CBuffer* pUniWireModel = m_xTower.AttachBuffer.GetValue(CTower::UNI_WIREPOINT_MODEL);
	if (pUniWireModel&&pUniWireModel->Length > 0)
		m_xWirePointModel.FromBuffer(*pUniWireModel);
	else
		m_xWirePointModel.m_bGimWireModelInherited = false;
	if (!m_xWirePointModel.m_bGimWireModelInherited)
		m_xTower.RetrieveWireModelFromNodes(&m_xWirePointModel);
	//初始化控制参数
	if(bInitDesPara)
		InitTempDesParaInfo();
}
void CTPSMModel::InitTempDesParaInfo()
{
	m_xTower.hashParams.Empty();
	m_xTower.hashSlaveNodes.Empty();
	CTowerTemplateToModel::InitTowerTempParas(m_xDesPara.m_ciTowerType, &m_xTower);
}
void CTPSMModel::InitParaSketchItems()
{
	CTowerTemplateToModel::InitTempParaSketchs(m_xDesPara.m_ciTowerType,m_hashParaSketch);
}
void CTPSMModel::ReBuildTower()
{
	if(m_xTower.hashParams.GetNodeNum()<=0)
		return;
	CExpression expr;
	for(DESIGN_PARAM_ITEM* pItem=m_xTower.hashParams.GetFirst();pItem;pItem=m_xTower.hashParams.GetNext())
	{
		EXPRESSION_VAR* pVar=expr.varList.Append(0);
		StrCopy(pVar->variableStr,pItem->key.Label(),9);
		pVar->fValue=pItem->value.fValue;
	}
	for(SLAVE_NODE* pSlaveNode=m_xTower.hashSlaveNodes.GetFirst();pSlaveNode;pSlaveNode=m_xTower.hashSlaveNodes.GetNext())
	{
		CLDSNode* pNode=m_xTower.Node.FromHandle(pSlaveNode->hRelaNode);
		if(pNode==NULL)
			continue;
		GEPOINT pos;
		if(pSlaveNode->IsControlCoordX())
			pNode->SetPositionX(pos.x=expr.SolveExpression(pSlaveNode->posx_expr));
		if(pSlaveNode->IsControlCoordY())
			pNode->SetPositionY(pos.y=expr.SolveExpression(pSlaveNode->posy_expr));
		if(pSlaveNode->IsControlCoordZ())
			pNode->SetPositionZ(pos.z=expr.SolveExpression(pSlaveNode->posz_expr));
		pNode->CalPosition(true);
		if(pSlaveNode->cbMirFlag&0x01)
			pNode->SyncMirProp("pos",1);
		if(pSlaveNode->cbMirFlag&0x02)
			pNode->SyncMirProp("pos",2);
		if(pSlaveNode->cbMirFlag&0x04)
			pNode->SyncMirProp("pos",3);
	}
	for(CLDSNode* pNode=m_xTower.Node.GetFirst();pNode;pNode=m_xTower.Node.GetNext())
		pNode->Unlock();
	for(CLDSLinePart* pRod=m_xTower.EnumRodFirst();pRod;pRod=m_xTower.EnumRodNext())
		pRod->ClearFlag();
	int index=0;
	for(CLDSNode* pNode=m_xTower.Node.GetFirst();pNode;pNode=m_xTower.Node.GetNext())
		pNode->CalPosition();
	for(CLDSLinePart* pRod=m_xTower.EnumRodFirst();pRod;pRod=m_xTower.EnumRodNext())
		pRod->CalPosition();
}
bool CTPSMModel::UpdateSlopeDesPara()
{
	return CTowerTemplateToModel::UpdateTowerSlopeDesPara(m_xDesPara.m_ciTowerType,&m_xTower);
}
void CTPSMModel::CreateModFile(const char* sFileName)
{
	IModModel* pModModel=CModModelFactory::CreateModModel();
	if(m_xTower.Parts.GetNodeNum()<=0 || pModModel==NULL )
		return;
	//提取MOD呼高信息，建立MOD坐标系
	double fTowerHeight=0;
	for(CLDSModule *pModule=m_xTower.Module.GetFirst();pModule;pModule=m_xTower.Module.GetNext())
	{
		double lowest_module_z=0;
		pModule->GetModuleScopeZ(NULL,&lowest_module_z);
		if(lowest_module_z>fTowerHeight)
			fTowerHeight=lowest_module_z;
		//
		IModHeightGroup* pHeightGroup=pModModel->AppendHeightGroup(pModule->GetBodyNo());
		pHeightGroup->SetBelongModel(pModModel);
		pHeightGroup->SetLowestZ(pModule->LowestZ());
		pHeightGroup->SetLegCfg(pModule->m_dwLegCfgWord.flag.bytes);
		pHeightGroup->SetNameHeight(pModule->NamedHeight);
	}
	pModModel->SetTowerHeight(fTowerHeight);
	GECS ucs=TransToUcs(pModModel->BuildUcsByModCS());
	//提取挂点信息
	f3dPoint maxWireNodePos;
	for(CLDSNode* pNode=m_xTower.EnumNodeFirst();pNode;pNode=m_xTower.EnumNodeNext())
	{
		if(pNode->m_cHangWireType!='C'&&pNode->m_cHangWireType!='E'&&pNode->m_cHangWireType!='J')
			continue;
		MOD_HANG_NODE* pGuaInfo=pModModel->AppendHangNode();
		pGuaInfo->m_xHangPos=ucs.TransPFromCS(pNode->xOriginalPos);
		if(pNode->m_cHangWireType=='C')			//导线
			pGuaInfo->m_ciWireType='C';
		else if(pNode->m_cHangWireType=='E')	//地线
			pGuaInfo->m_ciWireType='G';
		else //if(pNode->m_cHangWireType=='J')	//跳线
			pGuaInfo->m_ciWireType='T';
		if(strlen(pNode->m_sHangWireDesc)>0)
			strcpy(pGuaInfo->m_sHangName,pNode->m_sHangWireDesc);
	}
	//提取有效节点，进行编号处理
	CHashSet<NODE_PTR> hashLinkNode;
	ARRAY_LIST<NODE_PTR> nodeArr;
	for(CLDSLinePart* pRod=m_xTower.EnumRodFirst();pRod;pRod=m_xTower.EnumRodNext())
	{
		CLDSNode* pNodeS=pRod->pStart;
		if(pNodeS && hashLinkNode.GetValue(pNodeS->handle)==NULL)
		{
			hashLinkNode.SetValue(pNodeS->handle,pNodeS);
			nodeArr.append(pNodeS);
		}
		//
		CLDSNode* pNodeE=pRod->pEnd;
		if(pNodeE && hashLinkNode.GetValue(pNodeE->handle)==NULL)
		{
			hashLinkNode.SetValue(pNodeE->handle,pNodeE);
			nodeArr.append(pNodeE);
		}
	}
	BOOL bHasPtI=IsHasValidPointI(hashLinkNode);
	if(bHasPtI)
	{
		CQuickSort<NODE_PTR>::QuickSort(nodeArr.m_pData,nodeArr.GetSize(),comparefun);
		hashLinkNode.Empty();
		for(int i=0;i<nodeArr.GetSize();i++)
			hashLinkNode.SetValue(nodeArr[i]->handle,nodeArr[i]);
	}
	int index=1;
	CHashList<int> hashNodePointI;
	for(CLDSNode* pNode=hashLinkNode.GetFirst();pNode;pNode=hashLinkNode.GetNext())
	{
		int iNode=bHasPtI?pNode->point_i:index++;
		hashNodePointI.SetValue(pNode->handle,iNode);
	}
	//初始化MOD模型的节点与杆件
	index=0;
	for(CLDSNode* pNode=hashLinkNode.GetFirst();pNode;pNode=hashLinkNode.GetNext())
	{
		int *pIndex=hashNodePointI.GetValue(pNode->handle);
		if(pIndex==NULL)
			continue;
		GEPOINT org_pt=ucs.TransPFromCS(pNode->xOriginalPos);
		IModNode* pModNode=pModModel->AppendNode(*pIndex);
		pModNode->SetBelongModel(pModModel);
		pModNode->SetCfgword(pNode->cfgword.flag.bytes);
		pModNode->SetLdsOrg(MOD_POINT(pNode->xOriginalPos));
		pModNode->SetOrg(MOD_POINT(org_pt));
		if(pNode->IsLegObj())
			pModNode->SetLayer('L');	//腿部Leg
		else
			pModNode->SetLayer('B');	//塔身Body
	}
	index=0;
	for(CLDSLinePart* pRod=m_xTower.EnumRodFirst();pRod;pRod=m_xTower.EnumRodNext())
	{
		if(pRod==NULL||pRod->pStart==NULL||pRod->pEnd==NULL)
			continue;
		if(pRod->GetClassTypeId()==CLS_GROUPLINEANGLE)
			continue;
		int *iNodeIdS=hashNodePointI.GetValue(pRod->pStart->handle);
		int *iNodeIdE=hashNodePointI.GetValue(pRod->pEnd->handle);
		IModNode* pModNodeS=pModModel->FindNode(*iNodeIdS);
		IModNode* pModNodeE=pModModel->FindNode(*iNodeIdE);
		if(pModNodeS==NULL || pModNodeE==NULL)
			continue;
		IModRod* pModRod=pModModel->AppendRod(index++);
		pModRod->SetBelongModel(pModModel);
		pModRod->SetCfgword(pRod->cfgword.flag.bytes);
		pModRod->SetNodeS(pModNodeS);
		pModRod->SetNodeE(pModNodeE);
		pModRod->SetMaterial(pRod->cMaterial);
		pModRod->SetWidth(pRod->GetWidth());
		pModRod->SetThick(pRod->GetThick());
		if(pRod->IsLegObj())
			pModRod->SetLayer('L');	//腿部Leg
		else
			pModRod->SetLayer('B');	//塔身Body
		if(pRod->GetClassTypeId()==CLS_LINEANGLE)
		{
			CLDSLineAngle* pJg=(CLDSLineAngle*)pRod;
			GEPOINT wing_vec_x=ucs.TransVToCS(pJg->GetWingVecX());
			GEPOINT wing_vec_y=ucs.TransVToCS(pJg->GetWingVecY());
			pModRod->SetWingXVec(MOD_POINT(wing_vec_x));
			pModRod->SetWingYVec(MOD_POINT(wing_vec_y));
			pModRod->SetRodType(1);
		}
		else
			pModRod->SetRodType(2);
	}
	//初始化多胡高塔型的MOD结构
	pModModel->InitMultiModData();
	//生成Mod文件
	FILE *fp=fopen(sFileName,"wt,ccs=UTF-8");
	if(fp==NULL)
	{
		logerr.Log("%s文件打开失败!",sFileName);
		return;
	}
	pModModel->WriteModFileByUtf8(fp);
}
void CTPSMModel::CreateDxfFile(const char* sFileName)
{
	CLDSModule *pModule=m_xTower.GetActiveModule();
	if(pModule==NULL)
		return;
	double lowest_module_z=0;
	pModule->GetModuleScopeZ(NULL,&lowest_module_z);
	//初始化GIM坐标系
	UCS_STRU draw_ucs;
	draw_ucs.origin.Set(0,0,lowest_module_z);
	draw_ucs.axis_x.Set(1,0,0);
	draw_ucs.axis_y.Set(0,-1,0);
	draw_ucs.axis_z.Set(0,0,-1);
	//提取直线集合，并初始化区域
	SCOPE_STRU scope;
	ARRAY_LIST<f3dLine> line_arr;
	for(CLDSLinePart* pRod=m_xTower.EnumRodFirst();pRod;pRod=m_xTower.EnumRodNext())
	{
		if(!pModule->IsSonPart(pRod))
			continue;
		f3dPoint ptS=pRod->Start();
		if(pRod->pStart)
			ptS=pRod->pStart->xOriginalPos;
		coord_trans(ptS,draw_ucs,FALSE);
		scope.VerifyVertex(ptS);
		f3dPoint ptE=pRod->End();
		if(pRod->pEnd)
			ptE=pRod->pEnd->xOriginalPos;
		coord_trans(ptE,draw_ucs,FALSE);
		scope.VerifyVertex(ptE);
		//
		line_arr.append(f3dLine(ptS,ptE));
	}
	//生成DXF文件
	CDxfFile dxf;
	dxf.extmin.Set(scope.fMinX,scope.fMinY,scope.fMinZ);
	dxf.extmax.Set(scope.fMaxX,scope.fMaxY,scope.fMaxZ);
	dxf.OpenFile(sFileName);
	for(int i=0;i<line_arr.GetSize();i++)
		dxf.New3dLine(line_arr[i].startPt,line_arr[i].endPt);
	dxf.CloseFile();
}
void CTPSMModel::Create3dsFile(const char* sFileName)
{
	CLDSModule *pModule=m_xTower.GetActiveModule();
	if(pModule==NULL)
		return;
	double lowest_module_z=0;
	pModule->GetModuleScopeZ(NULL,&lowest_module_z);
	//初始化GIM坐标系
	UCS_STRU draw_ucs;
	draw_ucs.origin.Set(0,0,lowest_module_z);
	draw_ucs.axis_x.Set(1,0,0);
	draw_ucs.axis_y.Set(0,-1,0);
	draw_ucs.axis_z.Set(0,0,-1);
	//解析三维实体的基本三角面信息
	int serial=1;
	I3DSData* p3dsFile=C3DSFactory::Create3DSInstance();
	for(CLDSPart* pPart=m_xTower.EnumPartFirst();pPart;pPart=m_xTower.EnumPartNext())
	{
		if(!pModule->IsSonPart(pPart))
			continue;
		pPart->Create3dSolidModel();
		CSolidPart solidPart=pPart->GetSolidPartObject();
		CSolidBody solidBody;
		solidBody.CopyBuffer(solidPart.pBody->BufferPtr(),solidPart.pBody->BufferLength());
		solidBody.TransToACS(draw_ucs);
		p3dsFile->AddSolidPart(&solidBody,serial,CXhChar16("Part%d",serial));
	}
	p3dsFile->Creat3DSFile(sFileName);
	//
	C3DSFactory::Destroy(p3dsFile->GetSerial());
	p3dsFile=NULL;
}
void CTPSMModel::CreateLDSFile(const char* sFileName)
{
	if(m_xTower.Node.GetNodeNum()<=0)
		return;
	DWORD cursor_pipeline_no=DogSerialNo();
	CBuffer buffer(10000000);
	buffer.WriteString("格构铁塔设计与绘图系统");
	buffer.WriteString(lds_file_v);
	buffer.WriteDword(m_xTower.user_pipeline_no);
	buffer.WriteDword(cursor_pipeline_no);
	CBuffer ta_buf(10000000);
	m_xTower.ToBuffer(ta_buf,cursor_pipeline_no,lds_file_v,PRODUCT_LDS);
	EncryptBuffer(ta_buf,2);
	DWORD dwFileLen=ta_buf.GetLength();
	buffer.WriteDword(dwFileLen);
	buffer.Write(ta_buf.GetBufferPtr(),dwFileLen);
	//挂点信息
	if (m_xWirePointModel.m_hashWirePoints.Count > 0)
	{
		CBuffer* pUniWireModel = m_xTower.AttachBuffer.GetValue(CTower::UNI_WIREPOINT_MODEL);
		if (pUniWireModel)
		{
			pUniWireModel->ClearContents();
			m_xWirePointModel.ToBuffer(*pUniWireModel);
		}
	}
	CBuffer attachBuffer(1000000);
	m_xTower.ToAttachBuffer(attachBuffer);
	buffer.WriteDword(attachBuffer.GetLength());
	buffer.Write(attachBuffer.GetBufferPtr(), attachBuffer.GetLength());
	//
	FILE* fp=fopen(sFileName,"wb");
	if(fp==NULL)
	{
		logerr.Log(CXhChar100("%s文件打开失败!",sFileName));
		return;
	}
	fwrite(buffer.GetBufferPtr(),buffer.GetLength(),1,fp);
	fclose(fp);
}
BOOL CTPSMModel::SaveTpiFile()
{
	CBuffer tip_buf(10000000);
	ToBuffer(tip_buf);
	//
	FILE* fp=fopen(m_sFullPath,"wb");
	if(fp==NULL)
	{
		logerr.Log(CXhChar100("%s文件打开失败!",m_sFullPath));
		return FALSE;
	}
	fwrite(tip_buf.GetBufferPtr(),tip_buf.GetLength(),1,fp);
	fclose(fp);
	return TRUE;
}
BOOL CTPSMModel::OpenTpiFile()
{
	FILE* fp=fopen((char*)m_sFullPath,"rb");
	if(fp==NULL)
	{
		logerr.Log(CXhChar100("%s文件打开失败!",(char*)m_sFullPath));
		return FALSE;
	}
	fseek(fp,0,SEEK_END);
	long buf_size=ftell(fp);
	fseek(fp,0,SEEK_SET);
	CBuffer tip_buf;
	tip_buf.Write(NULL,buf_size);
	fread(tip_buf.GetBufferPtr(),tip_buf.GetLength(),1,fp);
	fclose(fp);
	//
	FromBuffer(tip_buf);
	return TRUE;
}
void CTPSMModel::ToBuffer(CBuffer& buffer)
{
	DWORD cursor_pipeline_no=DogSerialNo();
	//塔型设计参数
	buffer.WriteString(m_sTower);
	buffer.WriteByte(m_xDesPara.m_ciTowerType);		//类型	
	buffer.WriteByte(m_xDesPara.m_ciShapeType);		//外形类型
	buffer.WriteByte(m_xDesPara.m_ciMainRodType);	//
	buffer.WriteInteger(m_xDesPara.m_nVoltGrade);	//电压等级
	buffer.WriteInteger(m_xDesPara.m_nCircuit);		//回路数
	buffer.WriteDouble(m_xDesPara.m_fAaltitude);	//海波高度
	buffer.WriteDouble(m_xDesPara.m_fWindSpeed);	//风速
	buffer.WriteDouble(m_xDesPara.m_fNiceThick);	//冰厚
	buffer.WriteDouble(m_xDesPara.m_fCheckNiceThick);
	buffer.WriteDouble(m_xDesPara.m_fWindSpan);		//水平档距
	buffer.WriteDouble(m_xDesPara.m_fWeightSpan);	//垂直档距
	buffer.WriteDouble(m_xDesPara.m_fRulingSpan);	//代表档距
	buffer.WriteString(m_xDesPara.m_sAngleRange,100);	//
	//塔型模型参数
	BYTE ciFlag=(m_xTower.Parts.GetNodeNum()>0)?1:0;
	buffer.WriteByte(ciFlag);
	if(ciFlag==1)
	{
		buffer.WriteString("格构铁塔设计与绘图系统");
		buffer.WriteString(lds_file_v);
		buffer.WriteDword(m_xTower.user_pipeline_no);
		buffer.WriteDword(cursor_pipeline_no);
		CBuffer ta_buf(10000000);
		m_xTower.ToBuffer(ta_buf,cursor_pipeline_no,lds_file_v,PRODUCT_LDS);
		EncryptBuffer(ta_buf,2);
		DWORD dwFileLen=ta_buf.GetLength();
		buffer.WriteDword(dwFileLen);
		buffer.Write(ta_buf.GetBufferPtr(),dwFileLen);
		//写入挂点信息模型
		if (m_xWirePointModel.m_hashWirePoints.Count > 0)
		{
			CBuffer attachBuf(10000000);
			m_xWirePointModel.ToBuffer(attachBuf);
			DWORD dwBufLen = attachBuf.GetLength();
			buffer.WriteDword(dwBufLen);
			buffer.Write(attachBuf.GetBufferPtr(), dwBufLen);
		}
	}
}
void CTPSMModel::FromBuffer(CBuffer& buffer)
{
	//塔型设计参数
	buffer.SeekToBegin();
	buffer.ReadString(m_sTower);
	buffer.ReadByte(&m_xDesPara.m_ciTowerType);		//类型	
	buffer.ReadByte(&m_xDesPara.m_ciShapeType);		//外形类型
	buffer.ReadByte(&m_xDesPara.m_ciMainRodType);	//
	buffer.ReadInteger(&m_xDesPara.m_nVoltGrade);	//电压等级
	buffer.ReadInteger(&m_xDesPara.m_nCircuit);		//回路数
	buffer.ReadDouble(&m_xDesPara.m_fAaltitude);	//海波高度
	buffer.ReadDouble(&m_xDesPara.m_fWindSpeed);	//风速
	buffer.ReadDouble(&m_xDesPara.m_fNiceThick);	//冰厚
	buffer.ReadDouble(&m_xDesPara.m_fCheckNiceThick);
	buffer.ReadDouble(&m_xDesPara.m_fWindSpan);		//水平档距
	buffer.ReadDouble(&m_xDesPara.m_fWeightSpan);	//垂直档距
	buffer.ReadDouble(&m_xDesPara.m_fRulingSpan);	//代表档距
	buffer.ReadString(m_xDesPara.m_sAngleRange,100);	//
	//塔型模型参数
	BYTE ciFlag=0;
	buffer.ReadByte(&ciFlag);
	if(ciFlag==1)
	{
		DWORD buffer_len=0,cursor_pipeline_no=0;
		CXhChar100 sDocTypeName,sFileVersion;
		buffer.ReadString(sDocTypeName);
		buffer.ReadString(sFileVersion);
		buffer.ReadDword(&m_xTower.user_pipeline_no);
		buffer.ReadDword(&cursor_pipeline_no);
		buffer.ReadDword(&buffer_len);
		CBuffer ta_buf;
		ta_buf.Write(NULL,buffer_len);
		buffer.Read(ta_buf.GetBufferPtr(),buffer_len);
		DecryptBuffer(ta_buf,2,cursor_pipeline_no);
		m_xTower.FromBuffer(ta_buf,PRODUCT_LDS);
		//提取挂点信息
		buffer.ReadDword(&buffer_len);
		if (buffer_len > 0)
		{
			CBuffer attach_buf;
			attach_buf.Write(NULL, buffer_len);
			buffer.Read(attach_buf.GetBufferPtr(), buffer_len);
			m_xWirePointModel.FromBuffer(attach_buf);
		}
		else
			m_xWirePointModel.m_bGimWireModelInherited = false;
		if (!m_xWirePointModel.m_bGimWireModelInherited)
			m_xTower.RetrieveWireModelFromNodes(&m_xWirePointModel);
	}
}

//属性栏
const DWORD HASHTABLESIZE = 500;
const DWORD STATUSHASHTABLESIZE = 50;
IMPLEMENT_PROP_FUNC(CTPSMModel);
void CTPSMModel::InitPropHashtable(void* pContext)
{
	int id=1;
	CTPSMModel::propHashtable.SetHashTableGrowSize(HASHTABLESIZE);
	CTPSMModel::propStatusHashtable.CreateHashTable(STATUSHASHTABLESIZE);
	//设计参数
	CTPSMModel::AddPropItem("BasicInfo",PROPLIST_ITEM(id++,"设计信息"));
	CTPSMModel::AddPropItem("DesPara.VoltGrade",PROPLIST_ITEM(id++,"电压等级","电压等级(KV)"));
	CTPSMModel::AddPropItem("DesPara.ShapeType",PROPLIST_ITEM(id++,"塔形样式","","0.鼓型|1.猫头型"));
	CTPSMModel::AddPropItem("DesPara.Circuit",PROPLIST_ITEM(id++,"回路数"));
	CTPSMModel::AddPropItem("DesPara.Aaltitude",PROPLIST_ITEM(id++,"海拔高度","海拔高度(m)"));
	CTPSMModel::AddPropItem("DesPara.WindSpeed",PROPLIST_ITEM(id++,"设计风速","10米设计风速(m/s)"));
	CTPSMModel::AddPropItem("DesPara.NiceThick",PROPLIST_ITEM(id++,"设计冰厚","设计冰厚(mm)"));
	CTPSMModel::AddPropItem("DesPara.CheckNiceThick",PROPLIST_ITEM(id++,"验算冰厚","验算冰厚(mm)"));
	CTPSMModel::AddPropItem("DesPara.WindSpan",PROPLIST_ITEM(id++,"水平档距","水平档距(m)"));
	CTPSMModel::AddPropItem("DesPara.WeightSpan",PROPLIST_ITEM(id++,"垂直档距","垂直档距(m)"));
	CTPSMModel::AddPropItem("DesPara.RulingSpan",PROPLIST_ITEM(id++,"代表档距","代表档距(m)"));
	CTPSMModel::AddPropItem("DesPara.m_sAngleRange",PROPLIST_ITEM(id++,"转角度数","转角度数范围"));
	//尺寸参数
	CTPSMModel::AddPropItem("HeightGroup",PROPLIST_ITEM(id++,"呼高信息"));
	CTPSMModel::AddPropItem("ActiveModule",PROPLIST_ITEM(id++,"激活呼高",""));
	CTPSMModel::AddPropItem("ActiveQuadLegNo1",PROPLIST_ITEM(id++,"1号配腿",""));
	CTPSMModel::AddPropItem("ActiveQuadLegNo2",PROPLIST_ITEM(id++,"2号配腿",""));
	CTPSMModel::AddPropItem("ActiveQuadLegNo3",PROPLIST_ITEM(id++,"3号配腿",""));
	CTPSMModel::AddPropItem("ActiveQuadLegNo4",PROPLIST_ITEM(id++,"4号配腿",""));
	//模型参数
	CTPSMModel::AddPropItem("CONTRl_PARA",PROPLIST_ITEM(id++,"外形尺寸"));
	CTPSMModel::AddPropItem("ARM1_PARA",PROPLIST_ITEM(id++,"横担控制参数"));
	CTPSMModel::AddPropItem("ARM1_L_PARA",PROPLIST_ITEM(id++,"横担控制参数"));
	CTPSMModel::AddPropItem("ARM1_R_PARA",PROPLIST_ITEM(id++,"横担控制参数"));
	CTPSMModel::AddPropItem("ARM2_PARA",PROPLIST_ITEM(id++,"横担控制参数"));
	CTPSMModel::AddPropItem("ARM2_L_PARA",PROPLIST_ITEM(id++,"横担控制参数"));
	CTPSMModel::AddPropItem("ARM2_R_PARA",PROPLIST_ITEM(id++,"横担控制参数"));
	CTPSMModel::AddPropItem("ARM3_PARA",PROPLIST_ITEM(id++,"横担控制参数"));
	CTPSMModel::AddPropItem("ARM3_L_PARA",PROPLIST_ITEM(id++,"横担控制参数"));
	CTPSMModel::AddPropItem("ARM3_R_PARA",PROPLIST_ITEM(id++,"横担控制参数"));
	CTPSMModel::AddPropItem("ARM4_PARA",PROPLIST_ITEM(id++,"横担控制参数"));
	CTPSMModel::AddPropItem("ARM4_L_PARA",PROPLIST_ITEM(id++,"横担控制参数"));
	CTPSMModel::AddPropItem("ARM4_R_PARA",PROPLIST_ITEM(id++,"横担控制参数"));
	if(pContext)
	{
		CTPSMModel* pModel=(CTPSMModel*)pContext;
		DESIGN_PARAM_ITEM* pItem=NULL;
		for(pItem=pModel->m_xTower.hashParams.GetFirst();pItem;pItem=pModel->m_xTower.hashParams.GetNext())
			CTPSMModel::AddPropItem(pItem->key.Label(),PROPLIST_ITEM(id++,pItem->sLabel,pItem->sNotes));
	}
}

KEY4C CTPSMModel::GetPropKey(long id)
{
	PROPLIST_ITEM *pPropItem=CTPSMModel::GetPropItem(id);
	if(pPropItem)
		return KEY4C(pPropItem->propKeyStr);
	return KEY4C();
}
int CTPSMModel::GetPropValueStr(long id,char* valueStr,UINT nMaxStrBufLen/*=100*/)
{
	char sText[100]="";
	if(GetPropID("DesPara.VoltGrade")==id)
		sprintf(sText,"%d",m_xDesPara.m_nVoltGrade);
	else if(GetPropID("DesPara.ShapeType")==id)
	{
		if(m_xDesPara.m_ciShapeType==0)
			strcpy(sText,"0.鼓型");
		else if(m_xDesPara.m_ciShapeType==1)
			strcpy(sText,"1.猫头型");
	}
	else if(GetPropID("DesPara.Circuit")==id)
		sprintf(sText,"%d",m_xDesPara.m_nCircuit);
	else if(GetPropID("DesPara.Aaltitude")==id)
	{
		sprintf(sText,"%f",m_xDesPara.m_fAaltitude);
		SimplifiedNumString(sText);
	}
	else if(GetPropID("DesPara.WindSpeed")==id)
	{
		sprintf(sText,"%f",m_xDesPara.m_fWindSpeed);
		SimplifiedNumString(sText);
	}
	else if(GetPropID("DesPara.NiceThick")==id)
	{
		sprintf(sText,"%f",m_xDesPara.m_fNiceThick);
		SimplifiedNumString(sText);
	}
	else if(GetPropID("DesPara.CheckNiceThick")==id)
	{
		sprintf(sText,"%f",m_xDesPara.m_fCheckNiceThick);
		SimplifiedNumString(sText);
	}
	else if(GetPropID("DesPara.WindSpan")==id)
	{
		sprintf(sText,"%f",m_xDesPara.m_fWindSpan);
		SimplifiedNumString(sText);
	}
	else if(GetPropID("DesPara.WeightSpan")==id)
	{
		sprintf(sText,"%f",m_xDesPara.m_fWeightSpan);
		SimplifiedNumString(sText);
	}
	else if(GetPropID("DesPara.RulingSpan")==id)
	{
		sprintf(sText,"%f",m_xDesPara.m_fRulingSpan);
		SimplifiedNumString(sText);
	}
	else if(GetPropID("DesPara.m_sAngleRange")==id)
		strcpy(sText,m_xDesPara.m_sAngleRange);
	else if(GetPropID("DesPara.TowerType")==id)
	{
		if(m_xDesPara.m_ciTowerType==TYPE_SZC)
			strcpy(sText,"1.SZ");
		else if(m_xDesPara.m_ciTowerType==TYPE_JG)
			strcpy(sText,"2.JG");
		else if(m_xDesPara.m_ciTowerType==TYPE_SJ)
			strcpy(sText,"3.SJ");
		else if(m_xDesPara.m_ciTowerType==TYPE_SJB)
			strcpy(sText,"4.SJB");
		else if(m_xDesPara.m_ciTowerType==TYPE_SJC)
			strcpy(sText,"5.SJC");
		else if(m_xDesPara.m_ciTowerType==TYPE_ZBC)
			strcpy(sText,"6.ZBC");
		else if(m_xDesPara.m_ciTowerType==TYPE_ZC)
			strcpy(sText,"7.ZC");
		else if(m_xDesPara.m_ciTowerType==TYPE_ZM)
			strcpy(sText,"8.ZM");
		else if(m_xDesPara.m_ciTowerType==TYPE_SZ)
			strcpy(sText,"9.SA");
	}
	else if(GetPropID("ActiveModule")==id)
	{
		CLDSModule *pModule=m_xTower.Module.FromHandle(m_xTower.m_hActiveModule);
		if(pModule)
			strcpy(sText,pModule->description);
	}
	else if(GetPropID("ActiveQuadLegNo1")==id)
	{
		CLDSModule *pModule=m_xTower.Module.FromHandle(m_xTower.m_hActiveModule);
		if(pModule)
			sprintf(sText,"%C",(pModule->m_arrActiveQuadLegNo[0]-1)%m_nMaxLegs+'A');
	}
	else if(GetPropID("ActiveQuadLegNo2")==id)
	{
		CLDSModule *pModule=m_xTower.Module.FromHandle(m_xTower.m_hActiveModule);
		if(pModule)
			sprintf(sText,"%C",(pModule->m_arrActiveQuadLegNo[1]-1)%m_nMaxLegs+'A');
	}
	else if(GetPropID("ActiveQuadLegNo3")==id)
	{
		CLDSModule *pModule=m_xTower.Module.FromHandle(m_xTower.m_hActiveModule);
		if(pModule)
			sprintf(sText,"%C",(pModule->m_arrActiveQuadLegNo[2]-1)%m_nMaxLegs+'A');
	}
	else if(GetPropID("ActiveQuadLegNo4")==id)
	{
		CLDSModule *pModule=m_xTower.Module.FromHandle(m_xTower.m_hActiveModule);
		if(pModule)
			sprintf(sText,"%C",(pModule->m_arrActiveQuadLegNo[3]-1)%m_nMaxLegs+'A');
	}
	else
	{
		KEY4C paramKey=CTPSMModel::GetPropKey(id);
		DESIGN_PARAM_ITEM* pItem=m_xTower.hashParams.GetValue(paramKey);
		if(pItem)
		{
			if(pItem->dataType==0)
			{
				double fValue=pItem->value.fValue;
				if(paramKey!=KEY4C("P1")&&paramKey!=KEY4C("P2")&&paramKey!=KEY4C("P")&&paramKey!=KEY4C("A"))
					fValue*=0.001;
				sprintf(sText,"%f",fValue);
				SimplifiedNumString(sText);
			}
			else if(pItem->dataType==1)
				sprintf(sText,"0x%X",pItem->value.hPart);
			else if(pItem->dataType==2)
				sprintf(sText,"%d",pItem->value.iType);
		}
	}
	//
	if(valueStr)
		StrCopy(valueStr,sText,nMaxStrBufLen);
	return strlen(sText);
}
CString CTPSMModel::MakeModuleLegStr(CLDSModule *pModule)
{
	CString sLegArr;
	if(pModule==NULL)
		return sLegArr;
	for(int i=1;i<=192;i++)
	{
		if(pModule->m_dwLegCfgWord.IsHasNo(i))
		{
			if(sLegArr.GetLength()<=0)
				sLegArr.Append(CXhChar16("%C",(i-1)%m_nMaxLegs+'A'));
			else
				sLegArr.Append(CXhChar16("|%C",(i-1)%m_nMaxLegs+'A'));
		}
	}
	return sLegArr;
}
//////////////////////////////////////////////////////////////////////////
//CTPSMProject
CTPSMProject::CTPSMProject()
{
	m_pActiveModel=NULL;
	Empty();
}
CTPSMProject::~CTPSMProject()
{

}
void CTPSMProject::DeleteModel(CTPSMModel* pTpsmModel)
{
	if(pTpsmModel==NULL)
		return;
	for(CTPSMModel* pModel=m_xTpsmModelList.GetFirst();pModel;pModel=m_xTpsmModelList.GetNext())
	{
		if(pModel==pTpsmModel)
		{
			m_xTpsmModelList.DeleteCursor();
			break;
		}	
	}
	m_xTpsmModelList.Clean();
}
CTower* CTPSMProject::GetActiveTa()
{
	if(m_pActiveModel)
		return m_pActiveModel->GetTa();
	return NULL;
}
BOOL CTPSMProject::ReadDesParaExcelFile(const char* sFile)
{
	CVariant2dArray sheetContentMap(1,1);
	if(!CExcelOper::GetExcelContentOfSpecifySheet(sFile,sheetContentMap,1))
		return FALSE;
	VARIANT value;
	CHashStrList<DWORD> hashColIndexByColTitle;
	for(int i=0;i<EXCEL_COL_COUNT;i++)
	{
		sheetContentMap.GetValueAt(0,i,value);
		CString itemstr(value.bstrVal);
		if(strstr(itemstr,T_INDEX))
			hashColIndexByColTitle.SetValue(T_INDEX,i);
		else if(strstr(itemstr,T_VOLTGRADE))
			hashColIndexByColTitle.SetValue(T_VOLTGRADE,i);
		else if(itemstr.CompareNoCase(T_TOWERNAME)==0)
			hashColIndexByColTitle.SetValue(T_TOWERNAME,i);
		else if(itemstr.CompareNoCase(T_SHAPETYPE)==0)
			hashColIndexByColTitle.SetValue(T_SHAPETYPE,i);
		else if(strstr(itemstr,T_MATERIAL))
			hashColIndexByColTitle.SetValue(T_MATERIAL,i);
		else if(strstr(itemstr,T_CIRCUIT))
			hashColIndexByColTitle.SetValue(T_CIRCUIT,i);
		else if(strstr(itemstr,T_ALTITUDE))
			hashColIndexByColTitle.SetValue(T_ALTITUDE,i);
		else if(strstr(itemstr,T_WINDSPEED))
			hashColIndexByColTitle.SetValue(T_WINDSPEED,i);
		else if(strstr(itemstr,T_NICETHICK))
			hashColIndexByColTitle.SetValue(T_NICETHICK,i);
		else if(strstr(itemstr,T_CHECKNT))
			hashColIndexByColTitle.SetValue(T_CHECKNT,i);
		else if(strstr(itemstr,T_WINDSPAN))
			hashColIndexByColTitle.SetValue(T_WINDSPAN,i);
		else if(strstr(itemstr,T_WEIGHTSPAN))
			hashColIndexByColTitle.SetValue(T_WEIGHTSPAN,i);
		else if(strstr(itemstr,T_RULINGSPAN))
			hashColIndexByColTitle.SetValue(T_RULINGSPAN,i);
		else if(strstr(itemstr,T_ANGLE))
			hashColIndexByColTitle.SetValue(T_ANGLE,i);
	}
	if (hashColIndexByColTitle.GetValue(T_INDEX)==NULL||
		hashColIndexByColTitle.GetValue(T_VOLTGRADE)==NULL||
		hashColIndexByColTitle.GetValue(T_TOWERNAME)==NULL||
		hashColIndexByColTitle.GetValue(T_SHAPETYPE)==NULL||
		hashColIndexByColTitle.GetValue(T_MATERIAL)==NULL||
		hashColIndexByColTitle.GetValue(T_CIRCUIT)==NULL||
		hashColIndexByColTitle.GetValue(T_ALTITUDE)==NULL||
		hashColIndexByColTitle.GetValue(T_WINDSPEED)==NULL)
	{
		logerr.Log("文件格式不对");
		return FALSE;
	}
	//读取数据，获取塔例设计信息
	m_xTpsmModelList.Empty();
	int nRows=sheetContentMap.RowsCount();
	for(int i=1;i<=nRows;i++)
	{
		//序号
		DWORD *pColIndex=hashColIndexByColTitle.GetValue(T_INDEX);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		if(value.vt==VT_EMPTY)
			continue;
		CTPSMModel* pModel=AppendModel();
		pModel->SetBelongPrj(this);
		//塔名
		pColIndex=hashColIndexByColTitle.GetValue(T_TOWERNAME);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pModel->m_sTower=VariantToString(value);
		//电压等级
		pColIndex=hashColIndexByColTitle.GetValue(T_VOLTGRADE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pModel->m_xDesPara.m_nVoltGrade=atoi(VariantToString(value));
		//铁塔外形
		pColIndex=hashColIndexByColTitle.GetValue(T_SHAPETYPE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		CXhChar50 ss1=VariantToString(value);
		if(ss1.EqualNoCase("鼓型"))
			pModel->m_xDesPara.m_ciShapeType=0;
		else if(ss1.EqualNoCase("猫头型"))
			pModel->m_xDesPara.m_ciShapeType=1;
		//材料类型
		pColIndex=hashColIndexByColTitle.GetValue(T_MATERIAL);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		CXhChar50 ss2=VariantToString(value);
		if(ss2.EqualNoCase("角钢塔"))
			pModel->m_xDesPara.m_ciMainRodType=0;
		else if(ss2.EqualNoCase("钢管塔"))
			pModel->m_xDesPara.m_ciMainRodType=1;
		//回路数
		pColIndex=hashColIndexByColTitle.GetValue(T_CIRCUIT);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pModel->m_xDesPara.m_nCircuit=atoi(VariantToString(value));
		//海拔高度
		pColIndex=hashColIndexByColTitle.GetValue(T_ALTITUDE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pModel->m_xDesPara.m_fAaltitude=atof(VariantToString(value));
		//设计风速
		pColIndex=hashColIndexByColTitle.GetValue(T_WINDSPEED);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pModel->m_xDesPara.m_fWindSpeed=atof(VariantToString(value));
		//设计冰厚
		pColIndex=hashColIndexByColTitle.GetValue(T_NICETHICK);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pModel->m_xDesPara.m_fNiceThick=atof(VariantToString(value));
		//验算冰厚
		pColIndex=hashColIndexByColTitle.GetValue(T_CHECKNT);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pModel->m_xDesPara.m_fCheckNiceThick=atof(VariantToString(value));
		//水平档距
		pColIndex=hashColIndexByColTitle.GetValue(T_WINDSPAN);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pModel->m_xDesPara.m_fWindSpan=atof(VariantToString(value));
		//垂直档距
		pColIndex=hashColIndexByColTitle.GetValue(T_WEIGHTSPAN);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pModel->m_xDesPara.m_fWeightSpan=atof(VariantToString(value));
		//代表档距
		pColIndex=hashColIndexByColTitle.GetValue(T_RULINGSPAN);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pModel->m_xDesPara.m_fRulingSpan=atof(VariantToString(value));
		//转角度数
		pColIndex=hashColIndexByColTitle.GetValue(T_ANGLE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		strcpy(pModel->m_xDesPara.m_sAngleRange,VariantToString(value));
	}
	return TRUE;
}
BOOL CTPSMProject::SaveProjectFile()
{
	//生成工程信息
	CMarkup xml;
	xml.AddElem("Project");
	xml.AddAttrib("Name",(char*)m_sPrjName);
	for(CTPSMModel* pModel=EnumFirstModel();pModel;pModel=EnumNextModel())
	{
		xml.IntoElem();
		xml.AddElem("TpiFile");
		xml.AddAttrib("FilePath",(char*)pModel->m_sFullPath);
		xml.OutOfElem();
	}
	xml.Save((char*)m_sPrjFullPath);
	//生成塔型信息
	for(CTPSMModel* pModel=EnumFirstModel();pModel;pModel=EnumNextModel())
		pModel->SaveTpiFile();
	return TRUE;
}
BOOL CTPSMProject::OpenProjectFile()
{
	CMarkup xml;
	if(!xml.Load((char*)m_sPrjFullPath))
	{
		logerr.Log(CXhChar100("%s工程方案文件打开失败!",(char*)m_sPrjFullPath));
		return FALSE;
	}
	xml.ResetMainPos();
	CString sValue=_T("");
	if(xml.FindElem("Project"))
	{
		sValue=xml.GetAttrib("Name");
		m_sPrjName.Copy(sValue.GetString());
	}
	while(xml.FindChildElem("TpiFile"))
	{
		xml.IntoElem();
		sValue=xml.GetAttrib("FilePath");
		CTPSMModel* pModel=m_xTpsmModelList.append();
		pModel->SetBelongPrj(this);
		pModel->m_sFullPath.Copy(sValue.GetString());
		xml.OutOfElem();
	}
	//读取塔型文件，初始化桩位塔例
	for(CTPSMModel* pTpi=EnumFirstModel();pTpi;pTpi=EnumNextModel())
		pTpi->OpenTpiFile();
	return TRUE;
}
//////////////////////////////////////////////////////////////////////////
//CProjectManager
CProjectManager::CProjectManager()
{
	m_pActivePrj=NULL;
	Empty();
}

CProjectManager::~CProjectManager()
{

}

void CProjectManager::DeletePrj(CTPSMProject* pProject)
{
	if(pProject==NULL)
		return;
	for(CTPSMProject* pPrj=m_xProjectList.GetFirst();pPrj;pPrj=m_xProjectList.GetNext())
	{
		if(pPrj!=pProject)
			continue;
		m_xProjectList.DeleteCursor();
		break;
	}
	m_xProjectList.Clean();
}

CTower* CProjectManager::GetActiveTa()
{
	if(m_pActivePrj)
		return m_pActivePrj->GetActiveTa();
	return NULL;
}
int CProjectManager::TowerNum()
{
	int nNum=0;
	for(CTPSMProject* pProject=EnumFirstPrj();pProject;pProject=EnumNextPrj())
		nNum+=pProject->TowerNum();
	return nNum;
}
void CProjectManager::SaveProjSln()
{
	//生成解决方案信息
	CMarkup xml;
	xml.AddElem("PrjSln");
	xml.AddAttrib("Name",(char*)m_sPrjSlnName);
	for(CTPSMProject* pProject=EnumFirstPrj();pProject;pProject=EnumNextPrj())
	{
		xml.IntoElem();
		xml.AddElem("ProjectFile");
		xml.AddAttrib("PrjFileName",(char*)pProject->m_sPrjFullPath);
		xml.OutOfElem();
	}
	xml.Save((char*)m_sPrjSlnFullPath);
	//生成工程信息
	for(CTPSMProject* pProject=EnumFirstPrj();pProject;pProject=EnumNextPrj())
		pProject->SaveProjectFile();
}
void CProjectManager::OpenProjSln()
{
	CMarkup xml;
	if(!xml.Load((char*)m_sPrjSlnFullPath))
	{
		logerr.Log(CXhChar100("%s工程方案文件打开失败!",(char*)m_sPrjSlnFullPath));
		return;
	}
	m_xProjectList.Empty();
	xml.ResetMainPos();
	CString sValue=_T("");
	if(xml.FindElem("PrjSln"))
	{
		sValue=xml.GetAttrib("Name");
		m_sPrjSlnName.Copy(sValue.GetString());
	}
	while(xml.FindChildElem("ProjectFile"))
	{
		xml.IntoElem();
		sValue=xml.GetAttrib("PrjFileName");
		CTPSMProject* pPrj=m_xProjectList.append();
		pPrj->m_sPrjFullPath.Copy(sValue.GetString());
		xml.OutOfElem();
	}
	//读取工程文件
	for(CTPSMProject* pPrj=EnumFirstPrj();pPrj;pPrj=EnumNextPrj())
		pPrj->OpenProjectFile();
}
