//<LOCALE_TRANSLATE BY wbt />
#include "stdafx.h"
#include "NativePropertyList.h"
#include "LogFile.h"

static BOOL NativeButtonClick(CPropertyList* pPropList,CPropTreeItem* pItem)
{
	BOOL rslt;
	((CNativePropertyList*)pPropList)->ButtonClick(pItem,&rslt);
	return rslt;
}
static BOOL NativeModifyValueFunc(CPropertyList* pPropList,CPropTreeItem* pItem,CString& valueStr)
{
	BOOL rslt;
	((CNativePropertyList*)pPropList)->ModifyValueFunc(pItem,valueStr,&rslt);
	return rslt;
}
static BOOL NativeModifyStatusFunc(CPropertyList* pPropList,CPropTreeItem* pItem)
{
	BOOL rslt;
	((CNativePropertyList*)pPropList)->ModifyStatusFunc(pItem,&rslt);
	//CLDSLineTube::SetPropStatus(pItem->m_idProp,pItem->m_bHideChildren);
	return rslt;
}
static BOOL NativeHelpPromptProperty(CPropertyList *pPropList,CPropTreeItem* pItem)
{
	BOOL rslt=((CNativePropertyList*)pPropList)->HelpPromptProperty(pItem);
	return rslt;
}

BOOL NativeCallBackCommand(CWnd* pCallWnd,UINT message,WPARAM wParam)
{
	BOOL rslt;
	((CNativePropertyList*)pCallWnd)->CallBackCommand(message,wParam,&rslt);
	return rslt;
}
CNativePropertyList::CNativePropertyList(void)
{
	SetModifyValueFunc(NativeModifyValueFunc);
	SetButtonClickFunc(NativeButtonClick);
	SetModifyStatusFunc(NativeModifyStatusFunc);
	SetPropHelpPromptFunc(NativeHelpPromptProperty);
	ReflectModifyValueFunc=NULL;
	ReflectButtonClickFunc=NULL;
}

bool CNativePropertyList::ModifyStatusFunc(CPropTreeItem* pItem,BOOL *lpRslt)
{
	DWORD dwStatus;
	if(pItem->m_bHideChildren)
		dwStatus = STATUS_COLLAPSE;
	else
		dwStatus = STATUS_EXPAND;
	if(lpRslt)
		*lpRslt=TRUE;
	return propStatusHashtable.SetValueAt(pItem->m_idProp,dwStatus);
}
bool CNativePropertyList::SetPropStatus(long id,BOOL bHide)
{
	DWORD dwStatus = bHide?STATUS_COLLAPSE:STATUS_EXPAND;
	return propStatusHashtable.SetValueAt(id,dwStatus);
}
int CNativePropertyList::GetPropStatus(long id)
{	
	DWORD dwStatus;
	if(propStatusHashtable.GetValueAt(id,dwStatus))
	{
		if(dwStatus==STATUS_EXPAND)
			return 0;	//不隐藏
		else if(dwStatus==STATUS_COLLAPSE)
			return 1;	//隐藏
	}
	return -1;	//未找到保存的状态
}
BOOL CNativePropertyList::HelpPromptProperty(CPropTreeItem* pItem)
{
	return TRUE;
}

long CNativePropertyList::InitPropHashtable(long initId/*=1*/,long initHashSize/*=50*/,long initStatusHashSize/*=0*/)	//返回下一空闲Id
{
	if(initHashSize<=0)
		initHashSize=50;
	propHashtable.SetHashTableGrowSize(initHashSize);
	if(initStatusHashSize<=0)
		initStatusHashSize=initHashSize;
	propStatusHashtable.CreateHashTable(initHashSize);
	return 1;
}
bool CNativePropertyList::SetPropKey(char* keyStr,long keyVal)
{
	PROPLIST_ITEM* pItem=propHashtable.GetValue(keyStr);
	if(pItem!=NULL)
	{
#ifdef AFX_TARG_ENU_ENGLISH
		logerr.Log("Attribute primary key string %s has repeated",keyStr);
#else 
		logerr.Log("属性主键字符串%s出现重复",keyStr);
#endif
		return false;
	}
	else
		AddPropItem(keyStr,PROPLIST_ITEM(keyVal));
	return true;
}
PROPLIST_ITEM* CNativePropertyList::AddPropItem(char* keyStr,PROPLIST_ITEM item)
{
	PROPLIST_ITEM* pItem=propHashtable.GetValue(keyStr);
	if(pItem!=NULL)
	{
#ifdef AFX_TARG_ENU_ENGLISH
		logerr.Log("Attribute primary key string %s has repeated",keyStr);
#else 
		logerr.Log("属性主键字符串%s出现重复",keyStr);
#endif
		return pItem;
	}
	else
		pItem=propHashtable.SetValue(keyStr,item);
	pItem->propKeyStr.Copy(keyStr);
	hashPropItemById.SetValue(item.id,pItem);
	return pItem;
}
long CNativePropertyList::GetPropID(const char* propStr)			//得到属性ID
{
	PROPLIST_ITEM* pItem=propHashtable.GetValue(propStr);
	if(pItem)
		return pItem->id;
	else
		return 0;
}
long CNativePropertyList::GetPropValueStrByKeyStr(char* key,char* valueStr)
{
	long id=GetPropID(key);
	return GetPropValueStr(id,valueStr);
}
long CNativePropertyList:: GetPropValueStr(long id,char* valueStr,UINT nMaxStrBufLen/*=100*/)
{
	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
//CNativePropListManager 定义
