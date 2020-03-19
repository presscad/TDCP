#pragma once

enum PROJECT_IMAGE{
	IMG_TOWER_ROOT,
	IMG_TOWER_ITEM,
	IMG_MODEL_ITEM,
	IMG_LEG_ITEM,
	IMG_DATA_GROUP,
	IMG_DATA_ITEM,
	IMG_GIM_ITEM,
	IMG_MOD_ITEM,
};
struct TREEITEM_INFO{
	DWORD dwRefData;
	int itemType;
	//
	static const int INFO_TA_SET	= 1;
	static const int INFO_TOWER		= 2;
	static const int INFO_HUGAO		= 3;
	static const int INFO_LEG1		= 4;
	static const int INFO_LEG2		= 5;
	static const int INFO_LEG3		= 6;
	static const int INFO_LEG4		= 7;
	static const int INFO_MOD_SET	= 8;
	static const int INFO_MOD		= 9;
	static const int INFO_MODEL_SET	= 10;
	static const int INFO_MODEL		= 11;
	static const int INFO_GIM_SET	= 12;
	static const int INFO_PRJ_GIM	= 13;
	static const int INFO_DEV_GIM	= 14;
	static const int INFO_ROOT		= 100;
public:
	TREEITEM_INFO(){;}
	TREEITEM_INFO(int type,DWORD dw){itemType=type;dwRefData=dw;}
};