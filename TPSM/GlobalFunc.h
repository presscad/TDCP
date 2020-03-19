#pragma once

#include "TPSMModel.h"

char* SearchChar(char* srcStr,char ch,bool reverseOrder=false);
char* SearchChar(char* srcStr,const char* delimiter,bool reverseOrder=false);
void SimplifiedNumString(CString &num_str);
void MakeDirectory(const char *path);
//判断dest_val是否为四个参数中的绝对值最大的一个(包括与其它值相等)
BOOL IsMaxInAll(double dest_val,double other1,double other2,double other3);
//计算角钢上指定节点处开合角起始与终止位置(角钢相对坐标值)Z坐标分量
BOOL CalKaiHeStartEndPos(CLDSLineAngle *pJg,CLDSNode *pNode,f3dPoint &startPos,f3dPoint &endPos,
					UCS_STRU &angleUcs,BOOL bIntelligent=FALSE,BOOL bUcsIncStartOdd=FALSE);

void GetPlateSetConnectedInAngle(CLDSLineAngle *pAngle,PARTSET &plateSet,BYTE wing_x0_y1_all2,CLDSNode* pNode);

UINT ReadStringFromStream(IStream *pStream,char *sReadString);
void WriteStringToStream(IStream *pStream,char *sReadString);
//
BOOL GetSysPath(char* sys_path);
BOOL GetLicFile(char* lic_path);
BOOL GetLdsPath(char* app_path);
//
char restore_JG_guige(const char* guige, double &wing_wide, double &wing_thick);
