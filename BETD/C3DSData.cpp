#include "StdAfx.h"
#include "C3DSData.h"
#include "CreateFace.h"
#include <direct.h>
#include "TIDModel.h"
int C3DSData::GetIndexFromArr(Lib3dsPoint* pvecteData,int nSize,f3dPoint point)
{
	Lib3dsPoint* pPoint;
	ARRAY_LIST<Lib3dsPoint> vecteArr;
	vecteArr.SetSize(nSize);
	memcpy(vecteArr.m_pData,pvecteData,sizeof(Lib3dsPoint)*nSize);
	for (int nIndex=0;nIndex<vecteArr.GetSize();nIndex++)
	{
		pPoint=vecteArr.GetByIndex(nIndex);
		if(vecteArr[nIndex].pos[0]==(float)point.x &&
			vecteArr[nIndex].pos[1]==(float)point.y &&
			vecteArr[nIndex].pos[2]==(float)point.z)
			return nIndex;
	}
	return -1;
}
bool C3DSData::BuildSolidBodyFromTidSolidBody(ITidSolidBody* pTidSolid,int nId,char* sSolidName,int nParentId)
{
	//解析实体buffer,重新生成面
	
	Lib3dsMesh* pMesh=lib3ds_mesh_new(sSolidName);
	Lib3dsNode* pNode=lib3ds_node_new_object();
	float fMatrix[4][4]= {{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}};
	float red,green,blue;
	ARRAY_LIST<Lib3dsPoint> vecteArr; //点
	ARRAY_LIST<Lib3dsTexel> texelArr;//纹理
	ARRAY_LIST<Lib3dsFace>  faceArr; //面
	BOOL isFirstFace=TRUE;
	TID_COORD3D vertice;
	pTidSolid->SplitToBasicFacets();
	int i,j,vertex_n=pTidSolid->KeyPointNum();
	vecteArr.SetSize(vertex_n);
	for(i=0;i<vertex_n;i++)
	{
		vertice=pTidSolid->GetKeyPointAt(i);
		vecteArr[i].pos[0]=vertice.x;
		vecteArr[i].pos[1]=vertice.y;
		vecteArr[i].pos[2]=vertice.z;
	}
	int face_n=pTidSolid->BasicFaceNum();
	for(i=0;i<face_n;i++)
	{
		ITidBasicFace* pBasicFace=pTidSolid->GetBasicFaceAt(i);
		int cluster_n=pBasicFace->FacetClusterNumber();

		for(j=0;j<cluster_n;j++)
		{
			IFacetCluster* pFacetCluster=pBasicFace->FacetClusterAt(j);
			int k,facet_n=pFacetCluster->FacetNumber();
			DYN_ARRAY<f3dPoint> vptrs(pFacetCluster->VertexNumber());
			for(int jj=0;jj<pFacetCluster->VertexNumber();jj++)
			{
				TID_COORD3D v=pFacetCluster->VertexAt(jj);
				vptrs[jj]=f3dPoint(v.x,v.y,v.z);
				if(GetIndexFromArr(vecteArr.m_pData,vecteArr.GetSize(),vptrs[jj])==-1)
				{
					Lib3dsPoint point;
					point.pos[0]=v.x;
					point.pos[1]=v.y;
					point.pos[2]=v.z;
					vecteArr.append(point);
				}
			}
			for(k=0;k<facet_n;k++)
			{
				Lib3dsFace* pFace=faceArr.append();
				TID_COORD3D vv[3];
				if(pFacetCluster->Mode()==IFacetCluster::TRIANGLES)
				{
					for(int ii=0;ii<3;ii++)
						pFace->points[ii]=GetIndexFromArr(vecteArr.m_pData,vecteArr.GetSize(),vptrs[k*3+ii]);
				}
				else if(pFacetCluster->Mode()==IFacetCluster::TRIANGLE_STRIP)
				{
					if(k%2==0)
					{
						for(int ii=0;ii<3;ii++)
							pFace->points[ii]=GetIndexFromArr(vecteArr.m_pData,vecteArr.GetSize(),vptrs[k+ii]);
					}
					else
					{
						pFace->points[0]=GetIndexFromArr(vecteArr.m_pData,vecteArr.GetSize(),vptrs[k]);
						pFace->points[1]=GetIndexFromArr(vecteArr.m_pData,vecteArr.GetSize(),vptrs[k+2]);
						pFace->points[2]=GetIndexFromArr(vecteArr.m_pData,vecteArr.GetSize(),vptrs[k+1]);
					}
				}
				else if(pFacetCluster->Mode()==IFacetCluster::TRIANGLE_FAN)
				{
					pFace->points[0]=GetIndexFromArr(vecteArr.m_pData,vecteArr.GetSize(),vptrs[0]);
					pFace->points[1]=GetIndexFromArr(vecteArr.m_pData,vecteArr.GetSize(),vptrs[k+1]);
					pFace->points[2]=GetIndexFromArr(vecteArr.m_pData,vecteArr.GetSize(),vptrs[k+2]);
				}
				else
					continue;
				if(isFirstFace)
				{
					red   = GetRValue(pBasicFace->Color())/255.0f;
					green = GetGValue(pBasicFace->Color())/255.0f;
					blue  = GetBValue(pBasicFace->Color())/255.0f;
					CreatNewMaterial(red,green,blue,sSolidName);
					isFirstFace=FALSE;
				}
				memcpy(pMesh->matrix,fMatrix,sizeof(Lib3dsMatrix));
				TID_COORD3D pNormal=pFacetCluster->Normal();
				pFace->normal[0]=(float)pNormal.x;
				pFace->normal[1]=(float)pNormal.y;
				pFace->normal[2]=(float)pNormal.z;
				strcpy(pFace->material,"Part0");
				//strcpy(pFace->material,sSolidName);
			}
			
		}
	}
	if(vecteArr.GetSize()>0)
	{
		lib3ds_mesh_new_point_list(pMesh, vecteArr.GetSize());
		memcpy(pMesh->pointL,vecteArr.m_pData,vecteArr.Size() * sizeof(Lib3dsPoint));
	}
	if(faceArr.GetSize()>0)
	{
		lib3ds_mesh_new_face_list(pMesh, faceArr.GetSize());
		memcpy(pMesh->faceL,faceArr.m_pData,faceArr.Size() * sizeof(Lib3dsFace));
	}
	if(texelArr.GetSize()>0)
	{
		lib3ds_mesh_new_texel_list(pMesh, texelArr.GetSize());
		memcpy(pMesh->texelL,texelArr.m_pData,vecteArr.Size() * sizeof(Lib3dsPoint));
	}
	pNode->node_id=nId;
	pNode->type=LIB3DS_OBJECT_NODE;
	FillName(pNode->name,sSolidName,64);
	if(nParentId!=-1)
		pNode->parent_id=nParentId;
	else
		pNode->parent_id=LIB3DS_NO_PARENT;
	memcpy(pNode->matrix,fMatrix,sizeof(Lib3dsMatrix));
	m_vNode.push_back(pNode);
	m_vMesh.push_back(pMesh);
	pNode=pNode->next;
	pMesh=pMesh->next;
	return true;
}
C3DSData::C3DSData(void)
{
}
void C3DSData::CreatNewMaterial(float fRed,float fGreen,float fBlue,char* sName)
{
	double alpha = 0.6;	//考虑到显示效果的经验系数
	Lib3dsMaterial* pMaterial=lib3ds_material_new();
	strcpy(pMaterial->name,sName);
	pMaterial->ambient[0]=fRed;
	pMaterial->ambient[1]=fGreen;
	pMaterial->ambient[2]=fBlue;
	pMaterial->ambient[3]=alpha;
	float fDiffuse[4]={0.4,0.4,0.4,1};
	float fSpecular[4]={0.898,0.898,0.898,1};
	memcpy(pMaterial->diffuse, fDiffuse, 4 * sizeof(float));
	memcpy(pMaterial->specular, fSpecular, 4 * sizeof(float));
	pMaterial->shininess=0.67;
	Lib3dsTextureMap textule;
	strcpy(textule.name,"BUTTFACE.JPG");
	textule.offset[0]=0;
	textule.offset[1]=0;
	textule.scale[0]=1;
	textule.scale[1]=1;
	textule.percent=1;
	FillTexturemap(pMaterial->texture1_map,textule);
	m_vMaterial.push_back(pMaterial);
	pMaterial=pMaterial->next;
}
void C3DSData::GetFaceFromSolidFace(Lib3dsFace* pFaceData,int nSize,IRawSolidFace* pRawFace)
{
	
}

void C3DSData::Creat3DSFile(ITidTowerInstance* pInstance,char* sOutPath)
{
	InitDataFromInstance(pInstance);
	const static char* SCENE_NAME = "custom";
	m_3ds = lib3ds_file_new();
	CString sPath;
	sPath.Format("%s",sOutPath);
	FillName(m_3ds->name, SCENE_NAME, 12);
	if (!BuildNode())
		return ;
	for (size_t i = 0; i < m_vMaterial.size(); ++i)
		lib3ds_file_insert_material(m_3ds, m_vMaterial[i]);
	for (size_t i = 0; i < m_vMesh.size(); ++i)
		lib3ds_file_insert_mesh(m_3ds, m_vMesh[i]);
	sPath=sPath+"\\3ds";
	MakeDirectory(sPath.GetBuffer());
	sPath=sPath+"\\yyy.3DS";
	lib3ds_file_save(m_3ds, sPath.GetBuffer());
	lib3ds_file_free(m_3ds);

	for (size_t i = 0; i < m_vSource.size(); ++i)
		lib3ds_file_free(m_vSource[i]);

	m_vSource.clear();
	m_vNode.clear();
	m_vMesh.clear();
	m_vMaterial.clear();
}

C3DSData::~C3DSData(void)
{
}
void C3DSData::MakeDirectory(char* path)
{
	char bak_path[MAX_PATH],drive[MAX_PATH];
	strcpy(bak_path,path);
	char *dir = strtok(bak_path,"/\\");
	if(strlen(dir)==2&&dir[1]==':')
	{
		strcpy(drive,dir);
		strcat(drive,"\\");
		_chdir(drive);
		dir = strtok(NULL,"/\\");
	}
	while(dir)
	{
		_mkdir(dir);
		_chdir(dir);
		dir = strtok(NULL,"/\\");
	}
}
void C3DSData::InitDataFromInstance(ITidTowerInstance* pInstance)
{
	int nSolidId=0;
	int nSolidNameId=0;
	int nPortParentId,nBoltParentId,nAnchorBoltParentId;
	for(ITidAssemblePart* pAssmPart=pInstance->EnumAssemblePartFirst();pAssmPart;pAssmPart=pInstance->EnumAssemblePartNext(),nSolidId++,nSolidNameId++)
	{
		CString sSolidName;
		sSolidName.Format("Part%d",nSolidNameId);
		ITidSolidBody* pSolidBody=pAssmPart->GetSolidPart();
		if(nSolidNameId==0)
		{
			BuildSolidBodyFromTidSolidBody(pSolidBody,nSolidId,sSolidName.GetBuffer());
			nPortParentId=nSolidId;
		}
		else
			BuildSolidBodyFromTidSolidBody(pSolidBody,nSolidId,sSolidName.GetBuffer(),nPortParentId);
	}
	nSolidNameId=0;
	for(ITidAssembleBolt* pAssmBolt=pInstance->EnumAssembleBoltFirst();pAssmBolt;pAssmBolt=pInstance->EnumAssembleBoltNext(),nSolidId++,nSolidNameId++)
	{
		CString sSolidName;
		sSolidName.Format("Bolt%d",nSolidNameId);
		ITidSolidBody* pSolidBody=pAssmBolt->GetBoltSolid();
		ITidSolidBody* pNutSolid=pAssmBolt->GetNutSolid();
		if(pSolidBody==NULL||pNutSolid==NULL)
			continue;
		if(nSolidNameId==0)
		{
			BuildSolidBodyFromTidSolidBody(pSolidBody,nSolidId,sSolidName.GetBuffer());
			nBoltParentId=nSolidId;
			nSolidId+=1;
			nSolidNameId+=1;
			sSolidName.Format("Bolt%d",nSolidNameId);
			BuildSolidBodyFromTidSolidBody(pNutSolid,nSolidId,sSolidName.GetBuffer());
		}
		else
		{
			BuildSolidBodyFromTidSolidBody(pSolidBody,nSolidId,sSolidName.GetBuffer(),nBoltParentId);
			nSolidId+=1;
			nSolidNameId+=1;
			sSolidName.Format("Bolt%d",nSolidNameId);
			BuildSolidBodyFromTidSolidBody(pNutSolid,nSolidId,sSolidName.GetBuffer(),nBoltParentId);
		}
	}
	nSolidNameId=0;
	for(ITidAssembleAnchorBolt* pAssmAnchorBolt=pInstance->EnumFirstAnchorBolt();pAssmAnchorBolt;pAssmAnchorBolt=pInstance->EnumNextAnchorBolt(),nSolidId++,nSolidNameId++)
	{
		CString sSolidName;
		sSolidName.Format("AnchorBolt%d",nSolidNameId);
		ITidSolidBody* pSolidBody=pAssmAnchorBolt->GetBoltSolid();
		ITidSolidBody* pNutSolid=pAssmAnchorBolt->GetNutSolid();
		if(pSolidBody==NULL||pNutSolid==NULL)
			continue;
		if(nSolidNameId==0)
		{
			BuildSolidBodyFromTidSolidBody(pSolidBody,nSolidId,sSolidName.GetBuffer());
			nAnchorBoltParentId=nSolidId;
			nSolidId+=1;
			nSolidNameId+=1;
			sSolidName.Format("AnchorBolt%d",nSolidNameId);
			BuildSolidBodyFromTidSolidBody(pNutSolid,nSolidId,sSolidName.GetBuffer());
		}
		else
		{
			BuildSolidBodyFromTidSolidBody(pSolidBody,nSolidId,sSolidName.GetBuffer(),nAnchorBoltParentId);
			nSolidId+=1;
			nSolidNameId+=1;
			sSolidName.Format("AnchorBolt%d",nSolidNameId);
			BuildSolidBodyFromTidSolidBody(pNutSolid,nSolidId,sSolidName.GetBuffer(),nAnchorBoltParentId);
		}
	}
}
void C3DSData::BuildMaterial()
{
	set<string> setNames;
	for (size_t i = 0; i < m_vSource.size(); ++i)
	{
		Lib3dsMaterial* listMat = m_vSource[i]->materials;
		while (listMat)
		{
			string strName = listMat->name;
			if (RectifyName(strName, setNames, 63))
				RectifyMaterialReference(strName, listMat->name);

			Lib3dsMaterial* pMat = lib3ds_material_new();
			FillName(pMat->name, strName.c_str());
			pMat->next = NULL;

			FillMaterial(pMat, listMat);

			m_vMaterial.push_back(pMat);
			listMat = listMat->next;
		}
	}
	for (size_t i = 0; i < m_vMaterial.size(); ++i)
		lib3ds_file_insert_material(m_3ds, m_vMaterial[i]);
}
void C3DSData::BuildMesh()
{
	set<string> setNames;
	for (size_t i = 0; i < m_vSource.size(); ++i)
	{
		Lib3dsMesh* listMesh = m_vSource[i]->meshes;
		while (listMesh)
		{
			string strName = listMesh->name;
			if (RectifyName(strName, setNames, 63))
				RectifyMeshReference(strName, listMesh->name);

			Lib3dsMesh* pMesh = lib3ds_mesh_new(strName.c_str());
			pMesh->user = listMesh->user;
			pMesh->next = NULL;

			pMesh->object_flags = listMesh->object_flags;
			pMesh->color = listMesh->color;
			memcpy(pMesh->matrix, listMesh->matrix, sizeof(Lib3dsMatrix));

			if (listMesh->points)
			{
				lib3ds_mesh_new_point_list(pMesh, listMesh->points);
				memcpy(pMesh->pointL, listMesh->pointL, listMesh->points * sizeof(Lib3dsPoint));
			}

			if (listMesh->flags)
			{
				lib3ds_mesh_new_flag_list(pMesh, listMesh->flags);
				memcpy(pMesh->flagL, listMesh->flagL, listMesh->flags * sizeof(Lib3dsWord));
			}

			if (listMesh->texels)
			{
				lib3ds_mesh_new_texel_list(pMesh, listMesh->texels);
				memcpy(pMesh->texelL, listMesh->texelL, listMesh->texels * sizeof(Lib3dsTexel));
			}

			if (listMesh->faces)
			{
				lib3ds_mesh_new_face_list(pMesh, listMesh->faces);
				memcpy(pMesh->faceL, listMesh->faceL, listMesh->faces * sizeof(Lib3dsFace));
			}

			pMesh->box_map = listMesh->box_map;
			pMesh->map_data = listMesh->map_data;

			m_vMesh.push_back(pMesh);
			listMesh = listMesh->next;
		}
	}

	for (size_t i = 0; i < m_vMesh.size(); ++i)
		lib3ds_file_insert_mesh(m_3ds, m_vMesh[i]);
}
bool C3DSData::BuildNode()
{
	if (!RectifyNodeId())
		return false;

	/*for (size_t i = 0; i < m_vSource.size(); ++i)
	{
	Lib3dsNode* pNode = m_vSource[i]->nodes;
	while (pNode)
	{
	if (pNode->type == LIB3DS_OBJECT_NODE)
	CloneNode(pNode);

	pNode = pNode->next;
	}
	}*/

	for (size_t i = 0; i < m_vNode.size(); ++i)
		lib3ds_file_insert_node(m_3ds, m_vNode[i]);

	return true;
}
void C3DSData::FillName(char* strDst, const char* strSrc, size_t nMaxLen)
{
	size_t nNameLen = strlen(strSrc);
	if (nMaxLen && nNameLen > nMaxLen)
		nNameLen = nMaxLen;

	memcpy(strDst, strSrc, nNameLen);
	strDst[nNameLen] = 0;
}
bool C3DSData::RectifyName(string& strName, set<string>& setName, size_t nMaxLen)
{
	if (setName.find(strName) == setName.end())
		return false;

	char digit[32];
	strName = strName + _itoa(m_nName++, digit, 10);
	while (strName.size() > nMaxLen || setName.find(strName) != setName.end())
		strName = _itoa(m_nName++, digit, 10);

	setName.insert(strName);
	return true;
}
void C3DSData::FillMaterial(Lib3dsMaterial* pMat, const Lib3dsMaterial* pSrc)
{
	pMat->ambient[0] = pSrc->ambient[0]; // r
	pMat->ambient[1] = pSrc->ambient[1]; // g;
	pMat->ambient[2] = pSrc->ambient[2]; // b;
	pMat->ambient[3] = pSrc->ambient[3]; // a

	memcpy(pMat->diffuse, pSrc->diffuse, 4 * sizeof(float));
	memcpy(pMat->specular, pSrc->specular, 4 * sizeof(float));

	pMat->shininess = pSrc->shininess;
	pMat->shin_strength = pSrc->shin_strength;
	pMat->use_blur = pSrc->use_blur;
	pMat->blur = pSrc->blur;
	pMat->transparency = pSrc->transparency;
	pMat->falloff = pSrc->falloff;
	pMat->additive = pSrc->additive;
	pMat->self_ilpct = pSrc->self_ilpct;
	pMat->use_falloff = pSrc->use_falloff;
	pMat->self_illum = pSrc->self_illum;
	pMat->shading = pSrc->shading;
	pMat->soften = pSrc->soften;
	pMat->face_map = pSrc->face_map;
	pMat->two_sided = pSrc->two_sided;
	pMat->map_decal = pSrc->map_decal;
	pMat->use_wire = pSrc->use_wire;
	pMat->use_wire_abs = pSrc->use_wire_abs;
	pMat->wire_size = pSrc->wire_size;

	FillTexturemap(pMat->texture1_map, pSrc->texture1_map);
	FillTexturemap(pMat->texture1_mask, pSrc->texture1_mask);
	FillTexturemap(pMat->texture2_map, pSrc->texture2_map);
	FillTexturemap(pMat->texture2_mask, pSrc->texture2_mask);
	FillTexturemap(pMat->opacity_map, pSrc->opacity_map);
	FillTexturemap(pMat->opacity_mask, pSrc->opacity_mask);
	FillTexturemap(pMat->bump_map, pSrc->bump_map);
	FillTexturemap(pMat->bump_mask, pSrc->bump_mask);
	FillTexturemap(pMat->specular_map, pSrc->specular_map);
	FillTexturemap(pMat->specular_mask, pSrc->specular_mask);
	FillTexturemap(pMat->shininess_map, pSrc->shininess_map);
	FillTexturemap(pMat->shininess_mask, pSrc->shininess_mask);
	FillTexturemap(pMat->self_illum_map, pSrc->self_illum_map);
	FillTexturemap(pMat->self_illum_mask, pSrc->self_illum_mask);
	FillTexturemap(pMat->reflection_map, pSrc->reflection_map);
	FillTexturemap(pMat->reflection_mask, pSrc->reflection_mask);

	pMat->autorefl_map = pSrc->autorefl_map;
}
void C3DSData::FillTexturemap(Lib3dsTextureMap& texMap, const Lib3dsTextureMap& src)
{
	texMap.user = src.user;
	FillName(texMap.name, src.name);
	texMap.flags = src.flags;
	texMap.percent = src.percent;
	texMap.blur = src.blur;
	memcpy(texMap.scale, src.scale, 2*sizeof(Lib3dsFloat));
	memcpy(texMap.offset, src.offset, 2*sizeof(Lib3dsFloat));
	texMap.rotation = src.rotation;
	memcpy(texMap.tint_1, src.tint_1, 3*sizeof(Lib3dsFloat));
	memcpy(texMap.tint_2, src.tint_2, 3*sizeof(Lib3dsFloat));
	memcpy(texMap.tint_r, src.tint_r, 3*sizeof(Lib3dsFloat));
	memcpy(texMap.tint_g, src.tint_g, 3*sizeof(Lib3dsFloat));
	memcpy(texMap.tint_b, src.tint_b, 3*sizeof(Lib3dsFloat));
}
void C3DSData::RectifyMaterialReference(const string& strNewName, const char* strOldName)
{
	for (size_t i = 0; i < m_vSource.size(); ++i)
	{
		Lib3dsMesh* pMesh = m_vSource[i]->meshes;
		while (pMesh)
		{
			for (Lib3dsWord i = 0; i < pMesh->faces; ++i)
			{
				Lib3dsFace& face = pMesh->faceL[i];
				if (strcmp(strOldName, face.material) == 0)
					FillName(face.material, strNewName.c_str());
			}
		}
	}
}
void C3DSData::RectifyMeshReference(const string& strNewName, const char* strOldName)
{
	for (size_t i = 0; i < m_vSource.size(); ++i)
	{
		Lib3dsNode* pNode = m_vSource[i]->nodes;
		while (pNode)
		{
			if (strcmp(pNode->name, strOldName) == 0)
				FillName(pNode->name, strNewName.c_str());
		}
	}
}
bool C3DSData::RectifyNodeId()
{
	Lib3dsWord nIdStep = 2, nMaxId = 0;
	for (size_t i = 0; i < m_vSource.size(); ++i)
	{
		Lib3dsNode* listNode = m_vSource[i]->nodes;
		while (listNode)
		{
			nMaxId = RectifyNodeId(listNode, nIdStep);

			nIdStep = nMaxId + 1;
			listNode = listNode->next;
		}
	}

	return nMaxId != 0xFFFF;
}
Lib3dsWord C3DSData::RectifyNodeId(Lib3dsNode* pNode, Lib3dsWord nStep)
{
	unsigned int nTestNodeId = pNode->node_id + (unsigned int)nStep;
	if (nTestNodeId > 0xFFFF)
		return 0xFFFF;

	pNode->node_id += nStep;
	if (pNode->parent_id != LIB3DS_NO_PARENT)
		pNode->parent_id += nStep;

	Lib3dsWord nMaxId = pNode->node_id, nNodeId = 0;
	Lib3dsNode* pChild = pNode->childs;
	while (pChild)
	{
		nNodeId = RectifyNodeId(pChild, nStep);
		if (nNodeId > nMaxId)
			nMaxId = nNodeId;

		pChild = pChild->next;
	}

	return nMaxId;
}
void C3DSData::CloneNodeDataAmbient(Lib3dsAmbientData& dstAmbient, const Lib3dsAmbientData& srcAmbient)
{
	memcpy(dstAmbient.col, srcAmbient.col, sizeof(Lib3dsRgb));
	CloneTrack<Lib3dsLin3Track, Lib3dsLin3Key, SLin3KeyCloner>(dstAmbient.col_track, srcAmbient.col_track);
}
void C3DSData::CloneNodeDataObject(Lib3dsObjectData& dstObject, const Lib3dsObjectData& srcObject)
{
	memcpy(dstObject.pivot, srcObject.pivot, sizeof(Lib3dsVector));
	memcpy(dstObject.instance, srcObject.instance, 64 * sizeof(char));
	memcpy(dstObject.bbox_min, srcObject.bbox_min, sizeof(Lib3dsVector));
	memcpy(dstObject.bbox_max, srcObject.bbox_max, sizeof(Lib3dsVector));
	memcpy(dstObject.pos, srcObject.pos, sizeof(Lib3dsVector));
	CloneTrack<Lib3dsLin3Track, Lib3dsLin3Key, SLin3KeyCloner>(dstObject.pos_track, srcObject.pos_track);
	memcpy(dstObject.rot, srcObject.rot, sizeof(Lib3dsVector));
	CloneTrack<Lib3dsQuatTrack, Lib3dsQuatKey, SQuatKeyCloner>(dstObject.rot_track, srcObject.rot_track);
	memcpy(dstObject.scl, srcObject.scl, sizeof(Lib3dsVector));
	CloneTrack<Lib3dsLin3Track, Lib3dsLin3Key, SLin3KeyCloner>(dstObject.scl_track, dstObject.scl_track);
	dstObject.morph_smooth = srcObject.morph_smooth;
	memcpy(dstObject.morph, srcObject.morph, 64 * sizeof(char));
	CloneTrack<Lib3dsMorphTrack, Lib3dsMorphKey, SMorphKeyCloner>(dstObject.morph_track, srcObject.morph_track);
	dstObject.hide = srcObject.hide;
	CloneTrack<Lib3dsBoolTrack, Lib3dsBoolKey, SBoolKeyCloner>(dstObject.hide_track, dstObject.hide_track);
}
void C3DSData::CloneNodeDataCamera(Lib3dsCameraData& dstCamera, const Lib3dsCameraData& srcCamera)
{
	memcpy(dstCamera.pos, srcCamera.pos, sizeof(Lib3dsVector));
	CloneTrack<Lib3dsLin3Track, Lib3dsLin3Key, SLin3KeyCloner>(dstCamera.pos_track, srcCamera.pos_track);
	dstCamera.fov = srcCamera.fov;
	CloneTrack<Lib3dsLin1Track, Lib3dsLin1Key, SLin1KeyCloner>(dstCamera.fov_track, srcCamera.fov_track);
	dstCamera.roll = srcCamera.roll;
	CloneTrack<Lib3dsLin1Track, Lib3dsLin1Key, SLin1KeyCloner>(dstCamera.roll_track, srcCamera.roll_track);
}
void C3DSData::CloneNodeDataTarget(Lib3dsTargetData& dstTarget, const Lib3dsTargetData& srcTarget)
{
	memcpy(dstTarget.pos, srcTarget.pos, sizeof(Lib3dsVector));
	CloneTrack<Lib3dsLin3Track, Lib3dsLin3Key, SLin3KeyCloner>(dstTarget.pos_track, srcTarget.pos_track);
}
void C3DSData::CloneNodeDataLight(Lib3dsLightData& dstLight, const Lib3dsLightData& srcLight)
{
	memcpy(dstLight.pos, srcLight.pos, sizeof(Lib3dsVector));
	CloneTrack<Lib3dsLin3Track, Lib3dsLin3Key, SLin3KeyCloner>(dstLight.pos_track, srcLight.pos_track);
	memcpy(dstLight.col, srcLight.col, sizeof(Lib3dsRgb));
	CloneTrack<Lib3dsLin3Track, Lib3dsLin3Key, SLin3KeyCloner>(dstLight.col_track, srcLight.col_track);
	dstLight.hotspot = srcLight.hotspot;
	CloneTrack<Lib3dsLin1Track, Lib3dsLin1Key, SLin1KeyCloner>(dstLight.hotspot_track, srcLight.hotspot_track);
	dstLight.falloff = srcLight.falloff;
	CloneTrack<Lib3dsLin1Track, Lib3dsLin1Key, SLin1KeyCloner>(dstLight.falloff_track, srcLight.falloff_track);
	dstLight.roll = srcLight.roll;
	CloneTrack<Lib3dsLin1Track, Lib3dsLin1Key, SLin1KeyCloner>(dstLight.roll_track, srcLight.roll_track);
}
void C3DSData::CloneNodeDataSpot(Lib3dsSpotData& dstSpot, const Lib3dsSpotData& srcSpot)
{
	memcpy(dstSpot.pos, srcSpot.pos, sizeof(Lib3dsVector));
	CloneTrack<Lib3dsLin3Track, Lib3dsLin3Key, SLin3KeyCloner>(dstSpot.pos_track, srcSpot.pos_track);
}
void C3DSData::CloneNodeData(Lib3dsNodeData& data, const Lib3dsNodeData& srcData)
{
	CloneNodeDataAmbient(data.ambient, srcData.ambient);
	CloneNodeDataObject(data.object, srcData.object);
	CloneNodeDataCamera(data.camera, srcData.camera);
	CloneNodeDataTarget(data.target, srcData.target);
	CloneNodeDataLight(data.light, srcData.light);
	CloneNodeDataSpot(data.spot, srcData.spot);
}
void C3DSData::CloneNode(const Lib3dsNode* pSrcNode)
{
	Lib3dsNode* pNode = lib3ds_node_new_object();
	pNode->user = pSrcNode->user;
	pNode->node_id = pSrcNode->node_id;
	memcpy(pNode->matrix, pSrcNode->matrix, sizeof(Lib3dsMatrix));
	pNode->type = pSrcNode->type;
	FillName(pNode->name, pSrcNode->name);
	pNode->flags1 = pSrcNode->flags1;
	pNode->flags2 = pSrcNode->flags2;
	pNode->parent_id = pSrcNode->parent_id;
	CloneNodeData(pNode->data, pSrcNode->data);
	m_vNode.push_back(pNode);

	Lib3dsNode* pChild = pSrcNode->childs;
	while (pChild)
	{
		CloneNode(pChild);
		pChild = pChild->next;
	}
}
void SBoolKeyCloner::Clone(Lib3dsBoolKey*& keyDst, const Lib3dsBoolKey* keySrc)
{
	keyDst = lib3ds_bool_key_new();
	keyDst->tcb = keySrc->tcb;
}
void SLin1KeyCloner::Clone(Lib3dsLin1Key*& keyDst, const Lib3dsLin1Key* keySrc)
{
	keyDst = lib3ds_lin1_key_new();
	keyDst->tcb = keySrc->tcb;
	keyDst->value = keySrc->value;
	keyDst->dd = keySrc->dd;
	keyDst->ds = keyDst->ds;
}
void SLin3KeyCloner::Clone(Lib3dsLin3Key*& keyDst, const Lib3dsLin3Key* keySrc)
{
	keyDst = lib3ds_lin3_key_new();
	keyDst->tcb = keySrc->tcb;
	keyDst->next = NULL;
	memcpy(keyDst->value, keySrc->value, sizeof(Lib3dsVector));
	memcpy(keyDst->dd, keySrc->dd, sizeof(Lib3dsVector));
	memcpy(keyDst->ds, keySrc->ds, sizeof(Lib3dsVector));
}
void SQuatKeyCloner::Clone(Lib3dsQuatKey*& keyDst, const Lib3dsQuatKey* keySrc)
{
	keyDst = lib3ds_quat_key_new();
	keyDst->tcb = keySrc->tcb;
	memcpy(keyDst->axis, keySrc->axis, sizeof(Lib3dsVector));
	keyDst->angle = keySrc->angle;
	memcpy(keyDst->q, keySrc->q, sizeof(Lib3dsQuat));
	memcpy(keyDst->dd, keySrc->dd, sizeof(Lib3dsQuat));
	memcpy(keyDst->ds, keySrc->ds, sizeof(Lib3dsQuat));
}
void SMorphKeyCloner::Clone(Lib3dsMorphKey*& keyDst, const Lib3dsMorphKey* keySrc)
{
	keyDst = lib3ds_morph_key_new();
	keyDst->tcb = keySrc->tcb;
	memcpy(keyDst->name, keySrc->name, 64 * sizeof(char));
}