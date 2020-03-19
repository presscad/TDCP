#pragma once
#include "TidCplus.h"
#include "SolidBody.h"
#include "ArrayList.h"
struct SBoolKeyCloner { void Clone(Lib3dsBoolKey*& keyDst, const Lib3dsBoolKey* keySrc); };
struct SLin1KeyCloner { void Clone(Lib3dsLin1Key*& keyDst, const Lib3dsLin1Key* keySrc); };
struct SLin3KeyCloner { void Clone(Lib3dsLin3Key*& keyDst, const Lib3dsLin3Key* keySrc); };
struct SQuatKeyCloner { void Clone(Lib3dsQuatKey*& keyDst, const Lib3dsQuatKey* keySrc); };
struct SMorphKeyCloner { void Clone(Lib3dsMorphKey*& keyDst, const Lib3dsMorphKey* keySrc); };

class C3DSData
{
private:
	string m_strOutPath;
	vector<Lib3dsFile*> m_vSource;
	vector<Lib3dsMesh*> m_vMesh;
	vector<Lib3dsNode*> m_vNode;
	vector<Lib3dsMaterial*> m_vMaterial;

	Lib3dsFile* m_3ds;
	int m_nName;
public:
	C3DSData(void);
	~C3DSData(void);
	C3DSData(const char* sOutPath);
	void Creat3DSFile(ITidTowerInstance* pInstance,char* sOutPath);
	
private:
	void CreatNewMaterial(float fRed,float fGreen,float fBlue,char* sName);
	void InitDataFromInstance(ITidTowerInstance* pInstance);
	void GetFaceFromSolidFace(Lib3dsFace* pFaceData,int nSize,IRawSolidFace* pRawFace);
	void MakeDirectory(char* path);
	bool BuildNode();
	void BuildMesh();
	void BuildMaterial();
	void FillName(char* strDst, const char* strSrc, size_t nMaxLen = 0);
	bool RectifyName(string& strName, set<string>& setName, size_t nMaxLen);
	void RectifyMaterialReference(const string& strNewName, const char* strOldName);
	void FillMaterial(Lib3dsMaterial* pMat, const Lib3dsMaterial* pSrc);
	void FillTexturemap(Lib3dsTextureMap& texMap, const Lib3dsTextureMap& src);
	void RectifyMeshReference(const string& strNewName, const char* strOldName);
	bool RectifyNodeId();
	Lib3dsWord RectifyNodeId(Lib3dsNode* pNode, Lib3dsWord nStep);
	void CloneNodeDataAmbient(Lib3dsAmbientData& dstAmbient, const Lib3dsAmbientData& srcAmbient);
	void CloneNodeDataObject(Lib3dsObjectData& dstObject, const Lib3dsObjectData& srcObject);
	void CloneNodeDataCamera(Lib3dsCameraData& dstCamera, const Lib3dsCameraData& srcCamera);
	void CloneNodeDataTarget(Lib3dsTargetData& dstTarget, const Lib3dsTargetData& srcTarget);
	void CloneNodeDataLight(Lib3dsLightData& dstLight, const Lib3dsLightData& srcLight);
	void CloneNodeDataSpot(Lib3dsSpotData& dstSpot, const Lib3dsSpotData& srcSpot);
	void CloneNodeData(Lib3dsNodeData& data, const Lib3dsNodeData& srcData);
	void CloneNode(const Lib3dsNode* pSrcNode);
	template <typename TTrack, typename TKey, typename TCloner>
	void CloneTrack(TTrack& dstTrack, const TTrack& srcTrack)
	{
		dstTrack.flags = srcTrack.flags;
		dstTrack.keyL = NULL;

		const TKey *keySrc = srcTrack.keyL;
		TKey *keyDst = NULL, *keyPrev = NULL;

		TCloner cloner;
		while (keySrc)
		{
			cloner.Clone(keyDst, keySrc);

			if (!dstTrack.keyL)
				dstTrack.keyL = keyDst;

			if (keyPrev)
				keyPrev->next = keyDst;

			keyPrev = keyDst;
			keySrc = keySrc->next;
		}
	}
	bool BuildSolidBodyFromTidSolidBody(ITidSolidBody* pTidSolid,int nId,char* sSolidName,int nParentId=-1);
	int GetIndexFromArr(Lib3dsPoint* pvecteData,int nSize,f3dPoint point);
};

