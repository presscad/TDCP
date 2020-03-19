#include "stdafx.h"
#include "Global.h"
#include "Tower.h"
#include "SortFunc.h"

CTower Ta;
Global globalVars;
Global::Global()
{
	m_siVarCount=0;
}


Global::~Global()
{
}
Global::VAR* Global::RegisterGlobalVar(Global::VAR var)
{
	m_siVarCount++;
	xarrGlobalVars[m_siVarCount-1]=var;
	return &xarrGlobalVars[m_siVarCount-1];
}
void Global::InitialzeGlobalVars()
{
	CHeapSort<Global::VAR>::HeapSortClassic(xarrGlobalVars,m_siVarCount);
	for (short i=0;i<m_siVarCount;i++)
		xarrGlobalVars[i].pVar->GlobalInitialze();
}
