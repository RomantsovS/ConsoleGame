#ifndef GUI_SCRIPTSUNCTION_H_
#define GUI_SCRIPTSUNCTION_H_

/*
========================
Interface for calling functions from script
========================
*/
class idSWFScriptFunction {
public:
	virtual ~idSWFScriptFunction() {};

	virtual idSWFScriptVar Call(std::shared_ptr<idSWFScriptObject> thisObject, const idSWFParmList& parm) { return idSWFScriptVar(); }; // this should never be hit
};

/*
========================
Interface for calling functions from script, implemented natively on a nested class object
========================
*/
template<typename T>
class idSWFScriptFunction_Nested : public idSWFScriptFunction {
protected:
	T* pThis;
public:
	idSWFScriptFunction_Nested() : pThis(nullptr) { }

	idSWFScriptFunction* Bind(T* _pThis) { pThis = _pThis; return this; }
};

#endif
