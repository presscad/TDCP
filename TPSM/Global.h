#pragma once
struct IGlobalInitializer {
	virtual bool GlobalInitialze()=0;
};
class Global
{
	short m_siVarCount;
public:
	struct VAR {
		short siInitSerial;
		IGlobalInitializer* pVar;
		VAR (short _siInitSerial=0,IGlobalInitializer* pGlobalVar=NULL)
		{
			siInitSerial=_siInitSerial;
			pVar=pGlobalVar;
		}
		int Compare(const VAR* pItem) const{
			return siInitSerial-pItem->siInitSerial;
		}
	};
public:
	Global();
	~Global();
	VAR* RegisterGlobalVar(VAR var);
	void InitialzeGlobalVars();
private:
	VAR xarrGlobalVars[16];
};
extern Global globalVars;
