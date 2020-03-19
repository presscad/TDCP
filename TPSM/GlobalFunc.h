#pragma once

#include "TPSMModel.h"

char* SearchChar(char* srcStr,char ch,bool reverseOrder=false);
char* SearchChar(char* srcStr,const char* delimiter,bool reverseOrder=false);
void SimplifiedNumString(CString &num_str);
void MakeDirectory(const char *path);
//�ж�dest_val�Ƿ�Ϊ�ĸ������еľ���ֵ����һ��(����������ֵ���)
BOOL IsMaxInAll(double dest_val,double other1,double other2,double other3);
//����Ǹ���ָ���ڵ㴦���Ͻ���ʼ����ֹλ��(�Ǹ��������ֵ)Z�������
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
