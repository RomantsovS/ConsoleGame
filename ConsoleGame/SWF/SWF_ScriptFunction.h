#ifndef GUI_SCRIPTSUNCTION_H_
#define GUI_SCRIPTSUNCTION_H_

/*
========================
Interface for calling functions from script
========================
*/
class idSWFScriptFunction {
public:
	idSWFScriptFunction() {
#ifdef DEBUG_PRINT_Ctor_Dtor
		common->DPrintf("%s ctor\n", "idSWFScriptFunction");
#endif // DEBUG_PRINT_Ctor_Dtor;
	}
	virtual ~idSWFScriptFunction() {
#ifdef DEBUG_PRINT_Ctor_Dtor
		common->DPrintf("%s dtor\n", "idSWFScriptFunction");
#endif // DEBUG_PRINT_Ctor_Dtor;
	};
	idSWFScriptFunction(const idSWFScriptFunction&) = default;
	idSWFScriptFunction& operator=(const idSWFScriptFunction&) = default;
	idSWFScriptFunction(idSWFScriptFunction&&) = default;
	idSWFScriptFunction& operator=(idSWFScriptFunction&&) = default;

	virtual idSWFScriptVar Call(idSWFScriptObject* thisObject, const idSWFParmList& parm) { return idSWFScriptVar(); }; // this should never be hit
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
