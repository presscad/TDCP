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
static const char* T_INDEX			= "������";
static const char* T_VOLTGRADE		= "��ѹ�ȼ�";
static const char* T_TOWERNAME		= "����";
static const char* T_SHAPETYPE		= "��������";
static const char* T_MATERIAL		= "�Ǹ���";
static const char* T_CIRCUIT		= "��·��";
static const char* T_ALTITUDE		= "���θ߶�";
static const char* T_WINDSPEED		= "��Ʒ���";
static const char* T_NICETHICK		= "��Ʊ���";
static const char* T_CHECKNT		= "�������";
static const char* T_WINDSPAN		= "ˮƽ����";
static const char* T_WEIGHTSPAN		= "��ֱ����";
static const char* T_RULINGSPAN		= "������";
static const char* T_ANGLE			= "ת�Ƕ���";
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
		logerr.Log("%sģ���ļ���ʧ�ܣ�",(char*)sLdsFile);
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
	//���ӻ���
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
	//�ҵ�ģ�ͻ���
	CBuffer* pUniWireModel = m_xTower.AttachBuffer.GetValue(CTower::UNI_WIREPOINT_MODEL);
	if (pUniWireModel&&pUniWireModel->Length > 0)
		m_xWirePointModel.FromBuffer(*pUniWireModel);
	else
		m_xWirePointModel.m_bGimWireModelInherited = false;
	if (!m_xWirePointModel.m_bGimWireModelInherited)
		m_xTower.RetrieveWireModelFromNodes(&m_xWirePointModel);
	//��ʼ�����Ʋ���
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
	//��ȡMOD������Ϣ������MOD����ϵ
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
	//��ȡ�ҵ���Ϣ
	f3dPoint maxWireNodePos;
	for(CLDSNode* pNode=m_xTower.EnumNodeFirst();pNode;pNode=m_xTower.EnumNodeNext())
	{
		if(pNode->m_cHangWireType!='C'&&pNode->m_cHangWireType!='E'&&pNode->m_cHangWireType!='J')
			continue;
		MOD_HANG_NODE* pGuaInfo=pModModel->AppendHangNode();
		pGuaInfo->m_xHangPos=ucs.TransPFromCS(pNode->xOriginalPos);
		if(pNode->m_cHangWireType=='C')			//����
			pGuaInfo->m_ciWireType='C';
		else if(pNode->m_cHangWireType=='E')	//����
			pGuaInfo->m_ciWireType='G';
		else //if(pNode->m_cHangWireType=='J')	//����
			pGuaInfo->m_ciWireType='T';
		if(strlen(pNode->m_sHangWireDesc)>0)
			strcpy(pGuaInfo->m_sHangName,pNode->m_sHangWireDesc);
	}
	//��ȡ��Ч�ڵ㣬���б�Ŵ���
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
	//��ʼ��MODģ�͵Ľڵ���˼�
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
			pModNode->SetLayer('L');	//�Ȳ�Leg
		else
			pModNode->SetLayer('B');	//����Body
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
			pModRod->SetLayer('L');	//�Ȳ�Leg
		else
			pModRod->SetLayer('B');	//����Body
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
	//��ʼ����������͵�MOD�ṹ
	pModModel->InitMultiModData();
	//����Mod�ļ�
	FILE *fp=fopen(sFileName,"wt,ccs=UTF-8");
	if(fp==NULL)
	{
		logerr.Log("%s�ļ���ʧ��!",sFileName);
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
	//��ʼ��GIM����ϵ
	UCS_STRU draw_ucs;
	draw_ucs.origin.Set(0,0,lowest_module_z);
	draw_ucs.axis_x.Set(1,0,0);
	draw_ucs.axis_y.Set(0,-1,0);
	draw_ucs.axis_z.Set(0,0,-1);
	//��ȡֱ�߼��ϣ�����ʼ������
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
	//����DXF�ļ�
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
	//��ʼ��GIM����ϵ
	UCS_STRU draw_ucs;
	draw_ucs.origin.Set(0,0,lowest_module_z);
	draw_ucs.axis_x.Set(1,0,0);
	draw_ucs.axis_y.Set(0,-1,0);
	draw_ucs.axis_z.Set(0,0,-1);
	//������άʵ��Ļ�����������Ϣ
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
	buffer.WriteString("������������ͼϵͳ");
	buffer.WriteString(lds_file_v);
	buffer.WriteDword(m_xTower.user_pipeline_no);
	buffer.WriteDword(cursor_pipeline_no);
	CBuffer ta_buf(10000000);
	m_xTower.ToBuffer(ta_buf,cursor_pipeline_no,lds_file_v,PRODUCT_LDS);
	EncryptBuffer(ta_buf,2);
	DWORD dwFileLen=ta_buf.GetLength();
	buffer.WriteDword(dwFileLen);
	buffer.Write(ta_buf.GetBufferPtr(),dwFileLen);
	//�ҵ���Ϣ
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
		logerr.Log(CXhChar100("%s�ļ���ʧ��!",sFileName));
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
		logerr.Log(CXhChar100("%s�ļ���ʧ��!",m_sFullPath));
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
		logerr.Log(CXhChar100("%s�ļ���ʧ��!",(char*)m_sFullPath));
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
	//������Ʋ���
	buffer.WriteString(m_sTower);
	buffer.WriteByte(m_xDesPara.m_ciTowerType);		//����	
	buffer.WriteByte(m_xDesPara.m_ciShapeType);		//��������
	buffer.WriteByte(m_xDesPara.m_ciMainRodType);	//
	buffer.WriteInteger(m_xDesPara.m_nVoltGrade);	//��ѹ�ȼ�
	buffer.WriteInteger(m_xDesPara.m_nCircuit);		//��·��
	buffer.WriteDouble(m_xDesPara.m_fAaltitude);	//�����߶�
	buffer.WriteDouble(m_xDesPara.m_fWindSpeed);	//����
	buffer.WriteDouble(m_xDesPara.m_fNiceThick);	//����
	buffer.WriteDouble(m_xDesPara.m_fCheckNiceThick);
	buffer.WriteDouble(m_xDesPara.m_fWindSpan);		//ˮƽ����
	buffer.WriteDouble(m_xDesPara.m_fWeightSpan);	//��ֱ����
	buffer.WriteDouble(m_xDesPara.m_fRulingSpan);	//������
	buffer.WriteString(m_xDesPara.m_sAngleRange,100);	//
	//����ģ�Ͳ���
	BYTE ciFlag=(m_xTower.Parts.GetNodeNum()>0)?1:0;
	buffer.WriteByte(ciFlag);
	if(ciFlag==1)
	{
		buffer.WriteString("������������ͼϵͳ");
		buffer.WriteString(lds_file_v);
		buffer.WriteDword(m_xTower.user_pipeline_no);
		buffer.WriteDword(cursor_pipeline_no);
		CBuffer ta_buf(10000000);
		m_xTower.ToBuffer(ta_buf,cursor_pipeline_no,lds_file_v,PRODUCT_LDS);
		EncryptBuffer(ta_buf,2);
		DWORD dwFileLen=ta_buf.GetLength();
		buffer.WriteDword(dwFileLen);
		buffer.Write(ta_buf.GetBufferPtr(),dwFileLen);
		//д��ҵ���Ϣģ��
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
	//������Ʋ���
	buffer.SeekToBegin();
	buffer.ReadString(m_sTower);
	buffer.ReadByte(&m_xDesPara.m_ciTowerType);		//����	
	buffer.ReadByte(&m_xDesPara.m_ciShapeType);		//��������
	buffer.ReadByte(&m_xDesPara.m_ciMainRodType);	//
	buffer.ReadInteger(&m_xDesPara.m_nVoltGrade);	//��ѹ�ȼ�
	buffer.ReadInteger(&m_xDesPara.m_nCircuit);		//��·��
	buffer.ReadDouble(&m_xDesPara.m_fAaltitude);	//�����߶�
	buffer.ReadDouble(&m_xDesPara.m_fWindSpeed);	//����
	buffer.ReadDouble(&m_xDesPara.m_fNiceThick);	//����
	buffer.ReadDouble(&m_xDesPara.m_fCheckNiceThick);
	buffer.ReadDouble(&m_xDesPara.m_fWindSpan);		//ˮƽ����
	buffer.ReadDouble(&m_xDesPara.m_fWeightSpan);	//��ֱ����
	buffer.ReadDouble(&m_xDesPara.m_fRulingSpan);	//������
	buffer.ReadString(m_xDesPara.m_sAngleRange,100);	//
	//����ģ�Ͳ���
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
		//��ȡ�ҵ���Ϣ
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

//������
const DWORD HASHTABLESIZE = 500;
const DWORD STATUSHASHTABLESIZE = 50;
IMPLEMENT_PROP_FUNC(CTPSMModel);
void CTPSMModel::InitPropHashtable(void* pContext)
{
	int id=1;
	CTPSMModel::propHashtable.SetHashTableGrowSize(HASHTABLESIZE);
	CTPSMModel::propStatusHashtable.CreateHashTable(STATUSHASHTABLESIZE);
	//��Ʋ���
	CTPSMModel::AddPropItem("BasicInfo",PROPLIST_ITEM(id++,"�����Ϣ"));
	CTPSMModel::AddPropItem("DesPara.VoltGrade",PROPLIST_ITEM(id++,"��ѹ�ȼ�","��ѹ�ȼ�(KV)"));
	CTPSMModel::AddPropItem("DesPara.ShapeType",PROPLIST_ITEM(id++,"������ʽ","","0.����|1.èͷ��"));
	CTPSMModel::AddPropItem("DesPara.Circuit",PROPLIST_ITEM(id++,"��·��"));
	CTPSMModel::AddPropItem("DesPara.Aaltitude",PROPLIST_ITEM(id++,"���θ߶�","���θ߶�(m)"));
	CTPSMModel::AddPropItem("DesPara.WindSpeed",PROPLIST_ITEM(id++,"��Ʒ���","10����Ʒ���(m/s)"));
	CTPSMModel::AddPropItem("DesPara.NiceThick",PROPLIST_ITEM(id++,"��Ʊ���","��Ʊ���(mm)"));
	CTPSMModel::AddPropItem("DesPara.CheckNiceThick",PROPLIST_ITEM(id++,"�������","�������(mm)"));
	CTPSMModel::AddPropItem("DesPara.WindSpan",PROPLIST_ITEM(id++,"ˮƽ����","ˮƽ����(m)"));
	CTPSMModel::AddPropItem("DesPara.WeightSpan",PROPLIST_ITEM(id++,"��ֱ����","��ֱ����(m)"));
	CTPSMModel::AddPropItem("DesPara.RulingSpan",PROPLIST_ITEM(id++,"������","������(m)"));
	CTPSMModel::AddPropItem("DesPara.m_sAngleRange",PROPLIST_ITEM(id++,"ת�Ƕ���","ת�Ƕ�����Χ"));
	//�ߴ����
	CTPSMModel::AddPropItem("HeightGroup",PROPLIST_ITEM(id++,"������Ϣ"));
	CTPSMModel::AddPropItem("ActiveModule",PROPLIST_ITEM(id++,"�������",""));
	CTPSMModel::AddPropItem("ActiveQuadLegNo1",PROPLIST_ITEM(id++,"1������",""));
	CTPSMModel::AddPropItem("ActiveQuadLegNo2",PROPLIST_ITEM(id++,"2������",""));
	CTPSMModel::AddPropItem("ActiveQuadLegNo3",PROPLIST_ITEM(id++,"3������",""));
	CTPSMModel::AddPropItem("ActiveQuadLegNo4",PROPLIST_ITEM(id++,"4������",""));
	//ģ�Ͳ���
	CTPSMModel::AddPropItem("CONTRl_PARA",PROPLIST_ITEM(id++,"���γߴ�"));
	CTPSMModel::AddPropItem("ARM1_PARA",PROPLIST_ITEM(id++,"�ᵣ���Ʋ���"));
	CTPSMModel::AddPropItem("ARM1_L_PARA",PROPLIST_ITEM(id++,"�ᵣ���Ʋ���"));
	CTPSMModel::AddPropItem("ARM1_R_PARA",PROPLIST_ITEM(id++,"�ᵣ���Ʋ���"));
	CTPSMModel::AddPropItem("ARM2_PARA",PROPLIST_ITEM(id++,"�ᵣ���Ʋ���"));
	CTPSMModel::AddPropItem("ARM2_L_PARA",PROPLIST_ITEM(id++,"�ᵣ���Ʋ���"));
	CTPSMModel::AddPropItem("ARM2_R_PARA",PROPLIST_ITEM(id++,"�ᵣ���Ʋ���"));
	CTPSMModel::AddPropItem("ARM3_PARA",PROPLIST_ITEM(id++,"�ᵣ���Ʋ���"));
	CTPSMModel::AddPropItem("ARM3_L_PARA",PROPLIST_ITEM(id++,"�ᵣ���Ʋ���"));
	CTPSMModel::AddPropItem("ARM3_R_PARA",PROPLIST_ITEM(id++,"�ᵣ���Ʋ���"));
	CTPSMModel::AddPropItem("ARM4_PARA",PROPLIST_ITEM(id++,"�ᵣ���Ʋ���"));
	CTPSMModel::AddPropItem("ARM4_L_PARA",PROPLIST_ITEM(id++,"�ᵣ���Ʋ���"));
	CTPSMModel::AddPropItem("ARM4_R_PARA",PROPLIST_ITEM(id++,"�ᵣ���Ʋ���"));
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
			strcpy(sText,"0.����");
		else if(m_xDesPara.m_ciShapeType==1)
			strcpy(sText,"1.èͷ��");
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
		logerr.Log("�ļ���ʽ����");
		return FALSE;
	}
	//��ȡ���ݣ���ȡ���������Ϣ
	m_xTpsmModelList.Empty();
	int nRows=sheetContentMap.RowsCount();
	for(int i=1;i<=nRows;i++)
	{
		//���
		DWORD *pColIndex=hashColIndexByColTitle.GetValue(T_INDEX);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		if(value.vt==VT_EMPTY)
			continue;
		CTPSMModel* pModel=AppendModel();
		pModel->SetBelongPrj(this);
		//����
		pColIndex=hashColIndexByColTitle.GetValue(T_TOWERNAME);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pModel->m_sTower=VariantToString(value);
		//��ѹ�ȼ�
		pColIndex=hashColIndexByColTitle.GetValue(T_VOLTGRADE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pModel->m_xDesPara.m_nVoltGrade=atoi(VariantToString(value));
		//��������
		pColIndex=hashColIndexByColTitle.GetValue(T_SHAPETYPE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		CXhChar50 ss1=VariantToString(value);
		if(ss1.EqualNoCase("����"))
			pModel->m_xDesPara.m_ciShapeType=0;
		else if(ss1.EqualNoCase("èͷ��"))
			pModel->m_xDesPara.m_ciShapeType=1;
		//��������
		pColIndex=hashColIndexByColTitle.GetValue(T_MATERIAL);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		CXhChar50 ss2=VariantToString(value);
		if(ss2.EqualNoCase("�Ǹ���"))
			pModel->m_xDesPara.m_ciMainRodType=0;
		else if(ss2.EqualNoCase("�ֹ���"))
			pModel->m_xDesPara.m_ciMainRodType=1;
		//��·��
		pColIndex=hashColIndexByColTitle.GetValue(T_CIRCUIT);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pModel->m_xDesPara.m_nCircuit=atoi(VariantToString(value));
		//���θ߶�
		pColIndex=hashColIndexByColTitle.GetValue(T_ALTITUDE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pModel->m_xDesPara.m_fAaltitude=atof(VariantToString(value));
		//��Ʒ���
		pColIndex=hashColIndexByColTitle.GetValue(T_WINDSPEED);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pModel->m_xDesPara.m_fWindSpeed=atof(VariantToString(value));
		//��Ʊ���
		pColIndex=hashColIndexByColTitle.GetValue(T_NICETHICK);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pModel->m_xDesPara.m_fNiceThick=atof(VariantToString(value));
		//�������
		pColIndex=hashColIndexByColTitle.GetValue(T_CHECKNT);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pModel->m_xDesPara.m_fCheckNiceThick=atof(VariantToString(value));
		//ˮƽ����
		pColIndex=hashColIndexByColTitle.GetValue(T_WINDSPAN);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pModel->m_xDesPara.m_fWindSpan=atof(VariantToString(value));
		//��ֱ����
		pColIndex=hashColIndexByColTitle.GetValue(T_WEIGHTSPAN);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pModel->m_xDesPara.m_fWeightSpan=atof(VariantToString(value));
		//������
		pColIndex=hashColIndexByColTitle.GetValue(T_RULINGSPAN);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		pModel->m_xDesPara.m_fRulingSpan=atof(VariantToString(value));
		//ת�Ƕ���
		pColIndex=hashColIndexByColTitle.GetValue(T_ANGLE);
		sheetContentMap.GetValueAt(i,*pColIndex,value);
		strcpy(pModel->m_xDesPara.m_sAngleRange,VariantToString(value));
	}
	return TRUE;
}
BOOL CTPSMProject::SaveProjectFile()
{
	//���ɹ�����Ϣ
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
	//����������Ϣ
	for(CTPSMModel* pModel=EnumFirstModel();pModel;pModel=EnumNextModel())
		pModel->SaveTpiFile();
	return TRUE;
}
BOOL CTPSMProject::OpenProjectFile()
{
	CMarkup xml;
	if(!xml.Load((char*)m_sPrjFullPath))
	{
		logerr.Log(CXhChar100("%s���̷����ļ���ʧ��!",(char*)m_sPrjFullPath));
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
	//��ȡ�����ļ�����ʼ��׮λ����
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
	//���ɽ��������Ϣ
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
	//���ɹ�����Ϣ
	for(CTPSMProject* pProject=EnumFirstPrj();pProject;pProject=EnumNextPrj())
		pProject->SaveProjectFile();
}
void CProjectManager::OpenProjSln()
{
	CMarkup xml;
	if(!xml.Load((char*)m_sPrjSlnFullPath))
	{
		logerr.Log(CXhChar100("%s���̷����ļ���ʧ��!",(char*)m_sPrjSlnFullPath));
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
	//��ȡ�����ļ�
	for(CTPSMProject* pPrj=EnumFirstPrj();pPrj;pPrj=EnumNextPrj())
		pPrj->OpenProjectFile();
}
