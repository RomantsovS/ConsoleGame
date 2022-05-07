
#include "idlib/precompiled.h"

std::shared_ptr<idSWFScriptVar> idSWFParmList::Alloc() {
	emplace_back(std::make_shared<idSWFScriptVar>());
	return back();
}

void idSWFParmList::Append(const idSWFScriptVar& other) {
	std::shared_ptr<idSWFScriptVar> var = Alloc();
	if (var) {
		*var = other;
	}
}

void idSWFParmList::Append(std::shared_ptr<idSWFScriptObject> o) {
	std::shared_ptr<idSWFScriptVar> var = Alloc();
	if (var) {
		var->SetObject(o);
	}
}

void idSWFParmList::Append(std::shared_ptr<idSWFScriptFunction> f) {
	std::shared_ptr<idSWFScriptVar> var = Alloc();
	if (var) {
		var->SetFunction(f);
	}
}

void idSWFParmList::Append(const char* s) {
	std::shared_ptr<idSWFScriptVar> var = Alloc();
	if (var) {
		var->SetString(s);
	}
}

void idSWFParmList::Append(const std::string& s) {
	std::shared_ptr<idSWFScriptVar> var = Alloc();
	if (var) {
		var->SetString(s);
	}
}
/*
void idSWFParmList::Append(idSWFScriptString* s) {
	std::shared_ptr<idSWFScriptVar> var = Alloc();
	if (var != NULL) {
		var->SetString(s);
	}
}*/

void idSWFParmList::Append(const float f) {
	std::shared_ptr<idSWFScriptVar> var = Alloc();
	if (var) {
		var->SetFloat(f);
	}
}

void idSWFParmList::Append(const int i) {
	std::shared_ptr<idSWFScriptVar> var = Alloc();
	if (var) {
		var->SetInteger(i);
	}
}

void idSWFParmList::Append(const size_t i) {
	std::shared_ptr<idSWFScriptVar> var = Alloc();
	if (var) {
		var->SetInteger(i);
	}
}

void idSWFParmList::Append(const bool b) {
	std::shared_ptr<idSWFScriptVar> var = Alloc();
	if (var) {
		var->SetBool(b);
	}
}
