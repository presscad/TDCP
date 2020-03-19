#pragma once
#include "propertylist.h"
#include "PropListItem.h"
#include "Hashtable.h"
class CNativePropertyList : public CPropertyList
{
protected:
	CHashTable<DWORD>propStatusHashtable;	//属性状态哈希表
	CHashStrList<PROPLIST_ITEM>propHashtable;	//属性字符串哈希表
	CHashSet<PROPLIST_ITEM*> hashPropItemById;
	bool (*ReflectModifyValueFunc)(CNativePropertyList* pPropList,CPropTreeItem* pItem,CString& valueStr);
	bool(*ReflectButtonClickFunc)(CNativePropertyList* pPropList,CPropTreeItem* pItem);
public:
	CNativePropertyList(void);
	virtual ~CNativePropertyList(void){;}
	void SetReflectModifyValueFunc(bool (*func)(CNativePropertyList* pPropList,CPropTreeItem* pItem,CString &valueStr)){ReflectModifyValueFunc = func;}
	void SetReflectButtonClickFunc(bool (*func)(CNativePropertyList* pPropList,CPropTreeItem* pItem)){ReflectButtonClickFunc = func;}
public:	//属性栏操作
	bool SetPropKey(char* keyStr,long keyVal);
	PROPLIST_ITEM* AddPropItem(char* keyStr,PROPLIST_ITEM item);
	PROPLIST_ITEM* GetPropListItemByKeyStr(char* keyStr){return propHashtable.GetValue(keyStr);}
	PROPLIST_ITEM* GetPropListItemByPropId(long idProp){return hashPropItemById.GetValue(idProp);}
	virtual long InitPropHashtable(long idInit=1,long initHashSize=50,long initStatusHashSize=0);	//返回下一空闲Id
	virtual PROPLIST_ITEM* GetPropItem(long id){return hashPropItemById.GetValue(id);}
	virtual long GetPropID(const char* propStr);			//得到属性ID
	virtual long GetPropValueStrByKeyStr(char* key,char* valueStr);
	virtual long GetPropValueStr(long id,char* valueStr,UINT nMaxStrBufLen=100);
	virtual bool SetPropStatus(long id,BOOL bHide);
	virtual int  GetPropStatus(long id);
	virtual bool ButtonClick(CPropTreeItem* pItem,BOOL *lpRslt){return false;}
	virtual bool ModifyValueFunc(CPropTreeItem* pItem,CString& valueStr,BOOL *lpRslt){return false;}
	virtual bool ModifyStatusFunc(CPropTreeItem* pItem,BOOL *lpRslt);
	virtual BOOL HelpPromptProperty(CPropTreeItem* pItem);
	virtual bool CallBackCommand(UINT message,WPARAM wParam,BOOL *lpRslt){return false;}
	virtual BOOL FinishSelectObjOper(long hPickObj,long idEventProp){return FALSE;}
	virtual void DisplayPropertyList(int overwrite0_append1=0,DWORD dwPropGroup=1)=0;
	virtual bool DisplaySlavePropertyList(CPropTreeItem* pItem,DWORD dwRelaInfo=0){return false;}
};
BOOL NativeCallBackCommand(CWnd* pCallWnd,UINT message,WPARAM wParam);
///////////////////////////////////////////////////////////////////////////////////////////////////
class CNativePropListManager
{
	CNativePropertyList *m_pPropList;
	DWORD m_dwPreferPropGroup;
public:
	CPropertyList *GetPropList(){return m_pPropList;}
	CNativePropListManager(CNativePropertyList* pNativePropListObj=NULL)
	{
		m_pPropList=pNativePropListObj;
		m_dwPreferPropGroup=0;
	}
	~CNativePropListManager(void){;}
	void SetPreferPropGroup(DWORD dwPreferBelongGroup=0){m_dwPreferPropGroup=dwPreferBelongGroup;}
	CPropTreeItem *InsertPropItem(CPropTreeItem *pParentItem,char* propKey,
		CString sPropName="",CString sPropHelp="",int ctrlType=PIT_STATIC,
		int subType=0,CString sItemAttachValue="",int iInsertIndex=-1,BOOL bUpdate=FALSE)
	{
		DWORD dwCurrPropGroup=m_dwPreferPropGroup!=0?m_dwPreferPropGroup:pParentItem->m_dwPropGroup;
		long id=m_pPropList->GetPropID(propKey);
		if((dwCurrPropGroup&GetBinaryWord(m_pPropList->m_iPropGroup))==0)
			bUpdate=FALSE;
		PROPLIST_ITEM *pItem=m_pPropList->GetPropItem(id);
		if(pItem==NULL)
			return NULL;
		if(sPropName.GetLength()<=0)
			sPropName=pItem->name;
		if(sPropHelp.GetLength()<=0)
			sPropHelp=pItem->propHelpStr;
		if(sPropHelp.GetLength()<=0)
			sPropHelp=pItem->name;
		if((ctrlType==PIT_COMBO||ctrlType==PIT_BUTTON||ctrlType==PIT_GRAYBUTTON)&&sItemAttachValue.GetLength()<=0)
			sItemAttachValue=pItem->propCmbItemsStr;
		//
		CXhChar500 valueText;
		CPropTreeItem* pPropItem=m_pPropList->FindItemByPropId(pItem->id,NULL);
		if(pPropItem==NULL)
		{	//新插入项
			CItemInfo* lpInfo = new CItemInfo();
			lpInfo->m_controlType=ctrlType;
			lpInfo->m_strPropName=sPropName;
			lpInfo->m_strPropHelp=sPropHelp;
			if(lpInfo->m_controlType==PIT_BUTTON||lpInfo->m_controlType==PIT_GRAYBUTTON)
			{
				lpInfo->m_buttonType=subType;
				if(subType==BDT_POPMENU||subType==BDT_POPMENUEDIT)
					lpInfo->m_cmbItems=sItemAttachValue;
				else if(sItemAttachValue.GetLength()>0)
					lpInfo->m_sButtonName=sItemAttachValue;
			}
			else if(lpInfo->m_controlType==PIT_COMBO)
			{
				lpInfo->m_cmbType=subType;
				lpInfo->m_cmbItems = sItemAttachValue;
			}
			if(m_pPropList->GetPropValueStr(id,valueText,valueText.GetLengthMax())>0)
				lpInfo->m_strPropValue = valueText;
			//插入属性项
			pPropItem = m_pPropList->InsertItem(pParentItem,lpInfo,iInsertIndex,bUpdate);
			pPropItem->m_idProp = id;
			pPropItem->m_dwPropGroup=dwCurrPropGroup;
		}
		else
		{	//更新已有项
			pPropItem->m_lpNodeInfo->m_strPropName=sPropName;
			pPropItem->m_lpNodeInfo->m_strPropHelp=sPropHelp;
			if(m_pPropList->GetPropValueStr(id,valueText,valueText.GetLengthMax())>0)
				pPropItem->m_lpNodeInfo->m_strPropValue=valueText;
			if(bUpdate&&!pPropItem->IsHideSelf())
			{
				RECT rc;
				m_pPropList->GetItemRect(pPropItem->m_iIndex,&rc);
				m_pPropList->InvalidateRect(&rc);
			}
		}
		return pPropItem;
	}
	//下拉框
	CPropTreeItem *InsertCmbListPropItem(CPropTreeItem *pParentItem,char* propKey,
		CString sItemAttachValue="",CString sPropName="",CString sPropHelp="",int iInsertIndex=-1,BOOL bUpdate=FALSE)
	{
		return InsertPropItem(pParentItem,propKey,sPropName,sPropHelp,PIT_COMBO,CDT_LIST,sItemAttachValue,iInsertIndex,bUpdate);
	}
	CPropTreeItem *InsertCmbEditPropItem(CPropTreeItem *pParentItem,char* propKey,
		CString sItemAttachValue="",CString sPropName="",CString sPropHelp="",int iInsertIndex=-1,BOOL bUpdate=FALSE)
	{
		return InsertPropItem(pParentItem,propKey,sPropName,sPropHelp,PIT_COMBO,CDT_EDIT,sItemAttachValue,iInsertIndex,bUpdate);
	}
	CPropTreeItem* InsertCmbColorPropItem(CPropTreeItem *pParentItem,char* propKey,
		CString sItemAttachValue="",CString sPropName="",CString sPropHelp="",int iInsertIndex=-1,BOOL bUpdate=FALSE)
	{
		return InsertPropItem(pParentItem,propKey,sPropName,sPropHelp,PIT_COMBO,CDT_COLOR,sItemAttachValue,iInsertIndex,bUpdate);
	}
	//编辑框
	CPropTreeItem *InsertEditPropItem(CPropTreeItem *pParentItem,char* propKey,CString sPropName="",
										CString sPropHelp="",int iInsertIndex=-1,BOOL bUpdate=FALSE)
	{
		return InsertPropItem(pParentItem,propKey,sPropName,sPropHelp,PIT_EDIT,0,"",iInsertIndex,bUpdate);
	}
	//按钮
	CPropTreeItem *InsertFilePathPropItem(CPropTreeItem *pParentItem,char* propKey,CString sPropName="",
										CString sPropHelp="",int iInsertIndex=-1,BOOL bUpdate=FALSE)
	{
		return InsertPropItem(pParentItem,propKey,sPropName,sPropHelp,PIT_BUTTON,BDT_FILEPATH,"",iInsertIndex,bUpdate);
	}
	CPropTreeItem *InsertButtonPropItem(CPropTreeItem *pParentItem,char* propKey,CString sPropName="",
										CString sPropHelp="",int iInsertIndex=-1,BOOL bUpdate=FALSE)
	{
		return InsertPropItem(pParentItem,propKey,sPropName,sPropHelp,PIT_BUTTON,BDT_COMMON,"",iInsertIndex,bUpdate);
	}
	CPropTreeItem *InsertBtnEditPropItem(CPropTreeItem *pParentItem,char* propKey,CString sPropName="",
										CString sPropHelp="",int iInsertIndex=-1,BOOL bUpdate=FALSE)
	{
		return InsertPropItem(pParentItem,propKey,sPropName,sPropHelp,PIT_BUTTON,BDT_COMMONEDIT,"",iInsertIndex,bUpdate);
	}
	CPropTreeItem *InsertBtnPopMenuEditPropItem(CPropTreeItem *pParentItem,char* propKey,CString sPropName="",
										CString sPropHelp="",CString sItemAttachValue="",int iInsertIndex=-1,BOOL bUpdate=FALSE)
	{
		return InsertPropItem(pParentItem,propKey,sPropName,sPropHelp,PIT_BUTTON,BDT_POPMENUEDIT,sItemAttachValue,iInsertIndex,bUpdate);
	}
	CPropTreeItem *InsertPopMenuItem(CPropTreeItem *pParentItem,char* propKey,CString sPropName="",
									 CString sPropHelp="",CString sItemAttachValue="",int iInsertIndex=-1,BOOL bUpdate=FALSE)
	{
		return InsertPropItem(pParentItem,propKey,sPropName,sPropHelp,PIT_BUTTON,BDT_POPMENU,sItemAttachValue,iInsertIndex,bUpdate);
	}
	CPropTreeItem *InsertGrayButtonPropItem(CPropTreeItem *pParentItem,char* propKey,CString sPropName="",
		CString sPropHelp="",CString sItemAttachValue="",int iInsertIndex=-1,BOOL bUpdate=FALSE)
	{
		return InsertPropItem(pParentItem,propKey,sPropName,sPropHelp,PIT_GRAYBUTTON,BDT_COMMON,sItemAttachValue,iInsertIndex,bUpdate);
	}
	CPropTreeItem *InsertGrayPopMenuItem(CPropTreeItem *pParentItem,char* propKey,CString sPropName="",
		CString sPropHelp="",CString sItemAttachValue="",int iInsertIndex=-1,BOOL bUpdate=FALSE)
	{
		return InsertPropItem(pParentItem,propKey,sPropName,sPropHelp,PIT_GRAYBUTTON,BDT_POPMENU,sItemAttachValue,iInsertIndex,bUpdate);
	}
	CPropTreeItem *InsertGrayPopMenuEditItem(CPropTreeItem *pParentItem,char* propKey,CString sPropName="",
		CString sPropHelp="",CString sItemAttachValue="",int iInsertIndex=-1,BOOL bUpdate=FALSE)
	{
		return InsertPropItem(pParentItem,propKey,sPropName,sPropHelp,PIT_GRAYBUTTON,BDT_POPMENUEDIT,sItemAttachValue,iInsertIndex,bUpdate);
	}
	//更新指定项属性值
	BOOL SetItemPropValue(char* propKey)
	{
		long item_id=m_pPropList->GetPropID(propKey);
		return SetItemPropValue(item_id);
	}
	BOOL SetItemPropValue(long prop_id)
	{
		CXhChar100 sText;
		if(m_pPropList->GetPropValueStr(prop_id,sText,sText.GetLengthMax())>0)
		{
			m_pPropList->SetItemPropValue(prop_id,CString(sText));
			return TRUE;
		}
		else 
			return FALSE;
	}
	BOOL UpdatePropItemValue(const char* propKey)
	{
		long id=m_pPropList->GetPropID(propKey);
		if(m_pPropList->FindItemByPropId(id,NULL)==NULL)
			return FALSE;
		return SetItemPropValue(id);
	}
};
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "Tower.h"
class CParaCSPropertyList : public CNativePropertyList
{
	CParaCS paracs;
	IModel* m_pBelongModel;
public:
	CParaCSPropertyList(void){m_pBelongModel=NULL;}
	~CParaCSPropertyList(void){;}
	IModel* SetBelongModel(IModel* pModel){m_pBelongModel=pModel;}
	void CopyFrom(CParaCS* pParaCS);
	void CopyTo(CParaCS* pParaCS);
	virtual long InitPropHashtable(long initId=1,long initHashSize=50,long initStatusHashSize=0);	//返回下一空闲Id
	virtual long GetPropValueStr(long id,char* valueStr,UINT nMaxStrBufLen=100);
	virtual bool ButtonClick(CPropTreeItem* pItem,BOOL *lpRslt);
	virtual bool ModifyValueFunc(CPropTreeItem* pItem,CString& valueStr,BOOL *lpRslt);
	virtual bool CallBackCommand(UINT message,WPARAM wParam,BOOL *lpRslt);
	virtual void DisplayPropertyList(int overwrite0_append1=0,DWORD dwPropGroup=1);
};

class CDefinedViewPropertyList : public CParaCSPropertyList
{
	CDisplayView m_udfView;
public:
	CDefinedViewPropertyList(void){;}
	~CDefinedViewPropertyList(void){;}
	void CopyFrom(CDisplayView* pUdfView);
	void CopyTo(CDisplayView* pUdfView);
	virtual long InitPropHashtable(long initId=1,long initHashSize=50,long initStatusHashSize=0);	//返回下一空闲Id
	virtual long GetPropValueStr(long id,char* valueStr,UINT nMaxStrBufLen=100);
	virtual bool ModifyValueFunc(CPropTreeItem* pItem,CString& valueStr,BOOL *lpRslt);
	virtual void DisplayPropertyList(int overwrite0_append1=0,DWORD dwPropGroup=1);
};

class CSectionViewPropertyList : public CParaCSPropertyList
{
	bool m_bSlaveView;
	CSlaveSectionView m_xSonView;
	void UpdateDatumPointPropItem(CPropTreeItem *pParentItem,CLDSPoint *pPoint,BOOL bUpdate);
	void UpdateDatumVectorPropItem(CPropTreeItem *pParentItem,BOOL bUpdate);
	bool ModifyDatumPointPropValue(CPropTreeItem *pItem, CString &valueStr);
	bool ModifyDatumVectorPropValue(CPropTreeItem *pItem, CString &valueStr);
	void InsertPointPropItem(CPropTreeItem *pParentItem,char *sPropID,CString sPropName,CString sPropHelp);
	bool GetAssistLineStartPropValueStr(long id,char* valueStr);
	bool GetAssistFaceNormPropValueStr(long id,char* valueStr);
public:
	CSectionViewPropertyList(void){m_bSlaveView=false;}
	~CSectionViewPropertyList(void){;}
	void CopyFrom(CSectionView* pSectView);
	void CopyTo(CSectionView* pSectView);
	virtual long InitPropHashtable(long initId=1,long initHashSize=50,long initStatusHashSize=0);	//返回下一空闲Id
	virtual long GetPropValueStr(long id,char* valueStr,UINT nMaxStrBufLen=100);
	virtual bool ButtonClick(CPropTreeItem* pItem,BOOL *lpRslt);
	virtual bool ModifyValueFunc(CPropTreeItem* pItem,CString& valueStr,BOOL *lpRslt);
	virtual void DisplayPropertyList(int overwrite0_append1=0,DWORD dwPropGroup=1);
	virtual BOOL FinishSelectObjOper(long hPickObj,long idEventProp);
};

//TODO: 晓川结合CDisplayPointProperty、CLDSPoint及ParaCSPropertyList.cpp实现
//针对CLDSPoint的属性栏
class CParaPointPropertyList : public CNativePropertyList
{
	CLDSPoint m_xPoint;
	IModel* m_pBelongModel;
public:
	BOOL m_bDisBaseInfo;//局部使用基准点，不需要显示基本信息（句柄、描述）
	BOOL m_bDisPosStyle;//节点定位方式是否在当前属性栏中显示（有时可能已在外围显示如CDatumPointPropertyDlg) -wjh 2015.1.23
	CParaPointPropertyList(void){m_pBelongModel=NULL;m_bDisPosStyle=FALSE;m_bDisBaseInfo=TRUE;}
	~CParaPointPropertyList(void){;}
	IModel* SetBelongModel(IModel* pModel){m_pBelongModel=pModel;}
	void CopyFrom(CLDSPoint* pParaPoint);
	void CopyTo(CLDSPoint* pParaPoint);
	void UpdateDatumPointParaProperty();
	bool UpdateColumnIntersSectCenterParamProperty(CPropTreeItem* pParentItem);
	virtual BOOL FinishSelectObjOper(long hPickObj,long idEventProp);
	//
	virtual long InitPropHashtable(long initId=1,long initHashSize=50,long initStatusHashSize=0);	//返回下一空闲Id
	virtual long GetPropValueStr(long id,char* valueStr,UINT nMaxStrBufLen=100);
	virtual bool ButtonClick(CPropTreeItem* pItem,BOOL *lpRslt);
	virtual bool ModifyValueFunc(CPropTreeItem* pItem,CString& valueStr,BOOL *lpRslt);
	virtual bool CallBackCommand(UINT message,WPARAM wParam,BOOL *lpRslt);
	virtual void DisplayPropertyList(int overwrite0_append1=0,DWORD dwPropGroup=1);
	
};
//针对CLDSVector的属性栏
class CParaVectorPropertyList : public CNativePropertyList
{
	CLDSVector m_xVector;
	IModel* m_pBelongModel;
public:
	CParaVectorPropertyList(void){m_pBelongModel=NULL;}
	~CParaVectorPropertyList(void){;}
	IModel* SetBelongModel(IModel* pModel){m_pBelongModel=pModel;}
	void CopyFrom(CLDSVector* pParaVector);
	void CopyTo(CLDSVector* pParaVector);
	void UpdateDatumVectorPropItem();
	BOOL FinishSelectObjOper(long hPickObj,long idEventProp);
	//
	virtual long InitPropHashtable(long initId=1,long initHashSize=50,long initStatusHashSize=0);	//返回下一空闲Id
	virtual long GetPropValueStr(long id,char* valueStr,UINT nMaxStrBufLen=100);
	virtual bool ButtonClick(CPropTreeItem* pItem,BOOL *lpRslt);
	virtual bool ModifyValueFunc(CPropTreeItem* pItem,CString& valueStr,BOOL *lpRslt);
	virtual bool CallBackCommand(UINT message,WPARAM wParam,BOOL *lpRslt);
	virtual void DisplayPropertyList(int overwrite0_append1=0,DWORD dwPropGroup=1);
};