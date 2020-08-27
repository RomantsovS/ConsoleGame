#include <map>
#include <memory>
#include "CVarSystem.h"
#include "../idlib/CmdArgs.h"
#include "CmdSystem.h"
#include "../idlib/Str.h"
#include "Common_local.h"

idCVar * idCVar::staticVars = NULL;

/*
===============================================================================

	idInternalCVar

===============================================================================
*/

class idInternalCVar : public idCVar {
	friend class idCVarSystemLocal;
public:
	idInternalCVar();
	idInternalCVar(const std::string& newName, const std::string& newValue, int newFlags);
	idInternalCVar(const idCVar* cvar);
	virtual ~idInternalCVar();

	//const char** CopyValueStrings(const char** strings);
	void Update(const idCVar* cvar);
	void UpdateValue();
	//void					UpdateCheat();
	void Set(std::string newValue, bool force, bool fromServer);
	//void					Reset();

private:
	std::string nameString;				// name
	std::string resetString;			// resetting will change to this value
	std::string valueString;			// value
	std::string descriptionString;		// description

	//virtual const std::string& InternalGetResetString() const;

	virtual void InternalSetString(const std::string& newValue) override;
	/*virtual void			InternalServerSetString(const std::string& newValue) override;
	virtual void			InternalSetBool(const bool newValue) override;
	virtual void			InternalSetInteger(const int newValue) override;
	virtual void			InternalSetFloat(const float newValue) override;*/
};

/*
============
idInternalCVar::idInternalCVar
============
*/
idInternalCVar::idInternalCVar() {
}

/*
============
idInternalCVar::idInternalCVar
============
*/
idInternalCVar::idInternalCVar(const std::string& newName, const std::string& newValue, int newFlags) {
	nameString = newName;
	name = nameString;
	valueString = newValue;
	value = valueString;
	resetString = newValue;
	descriptionString = "";
	description = descriptionString;
	flags = (newFlags & ~CVAR_STATIC) | CVAR_MODIFIED;
	valueMin = 1;
	valueMax = -1;
	//valueStrings = NULL;
	//valueCompletion = 0;
	UpdateValue();
	//UpdateCheat();
	internalVar = this;
}

/*
============
idInternalCVar::idInternalCVar
============
*/
idInternalCVar::idInternalCVar(const idCVar* cvar) {
	nameString = cvar->GetName();
	name = nameString;
	valueString = cvar->GetString();
	value = valueString;
	resetString = cvar->GetString();
	descriptionString = cvar->GetDescription();
	description = descriptionString;
	flags = cvar->GetFlags() | CVAR_MODIFIED;
	valueMin = cvar->GetMinValue();
	valueMax = cvar->GetMaxValue();
	//valueStrings = CopyValueStrings(cvar->GetValueStrings());
	//valueCompletion = cvar->GetValueCompletion();
	UpdateValue();
	//UpdateCheat();
	internalVar = this;
}

/*
============
idInternalCVar::~idInternalCVar
============
*/
idInternalCVar::~idInternalCVar() {
	//Mem_Free(valueStrings);
	//valueStrings = NULL;
}

/*
============
idInternalCVar::Update
============
*/
void idInternalCVar::Update(const idCVar* cvar) {

	// if this is a statically declared variable
	if (cvar->GetFlags() & CVAR_STATIC) {

		if (flags & CVAR_STATIC) {

			// the code has more than one static declaration of the same variable, make sure they have the same properties
			if (resetString.compare(cvar->GetString()) != 0) {
				common->Warning("CVar '%s' declared multiple times with different initial value", nameString.c_str());
			}
			if ((flags & (CVAR_BOOL | CVAR_INTEGER | CVAR_FLOAT)) != (cvar->GetFlags() & (CVAR_BOOL | CVAR_INTEGER | CVAR_FLOAT))) {
				common->Warning("CVar '%s' declared multiple times with different type", nameString.c_str());
			}
			if (valueMin != cvar->GetMinValue() || valueMax != cvar->GetMaxValue()) {
				common->Warning("CVar '%s' declared multiple times with different minimum/maximum", nameString.c_str());
			}

		}

		// the code is now specifying a variable that the user already set a value for, take the new value as the reset value
		resetString = cvar->GetString();
		descriptionString = cvar->GetDescription();
		description = descriptionString.c_str();
		valueMin = cvar->GetMinValue();
		valueMax = cvar->GetMaxValue();
		//Mem_Free(valueStrings);
		//valueStrings = CopyValueStrings(cvar->GetValueStrings());
		//valueCompletion = cvar->GetValueCompletion();
		UpdateValue();
		cvarSystem->SetModifiedFlags(cvar->GetFlags());
	}

	flags |= cvar->GetFlags();

	//UpdateCheat();

	// only allow one non-empty reset string without a warning
	if (resetString.empty()) {
		resetString = cvar->GetString();
	}
	else if (cvar->GetString()[0] && resetString.compare(cvar->GetString()) != 0) {
		common->Warning("cvar \"%s\" given initial values: \"%s\" and \"%s\"\n", nameString.c_str(), resetString.c_str(), cvar->GetString());
	}
}

/*
============
idInternalCVar::UpdateValue
============
*/
void idInternalCVar::UpdateValue() {
	bool clamped = false;

	if (flags & CVAR_BOOL) {
		integerValue = (atoi(value.c_str()) != 0);
		floatValue = static_cast<float>(integerValue);
		if (value.compare("0") != 0 && value.compare("1") != 0) {
			if (integerValue != 0) {
				valueString = '1';
			}
			else
				valueString = '0';
			value = valueString;
		}
	}
	else if (flags & CVAR_INTEGER) {
		integerValue = (int)atoi(value.c_str());
		if (valueMin < valueMax) {
			if (integerValue < valueMin) {
				integerValue = (int)valueMin;
				clamped = true;
			}
			else if (integerValue > valueMax) {
				integerValue = (int)valueMax;
				clamped = true;
			}
		}
		if (clamped || !idStr::IsNumeric(value) || value.find('.') != std::string::npos) {
			valueString = idStr::GetFromValue(integerValue);
			value = valueString;
		}
		floatValue = (float)integerValue;
	}
	else if (flags & CVAR_FLOAT) {
		floatValue = (float)atof(value.c_str());
		if (valueMin < valueMax) {
			if (floatValue < valueMin) {
				floatValue = valueMin;
				clamped = true;
			}
			else if (floatValue > valueMax) {
				floatValue = valueMax;
				clamped = true;
			}
		}
		if (clamped || !idStr::IsNumeric(value)) {
			valueString = idStr::GetFromValue(floatValue);
			value = valueString.c_str();
		}
		integerValue = (int)floatValue;
	}
	else {
		/*if (valueStrings && valueStrings[0]) {
			integerValue = 0;
			for (int i = 0; valueStrings[i]; i++) {
				if (valueString.Icmp(valueStrings[i]) == 0) {
					integerValue = i;
					break;
				}
			}
			valueString = valueStrings[integerValue];
			value = valueString.c_str();
			floatValue = (float)integerValue;
		}
		else*/ if (valueString.size() < 32) {
			floatValue = (float)atof(value.c_str());
			integerValue = (int)floatValue;
		}
		else {
			floatValue = 0.0f;
			integerValue = 0;
		}
	}
}

/*
============
idInternalCVar::Set
============
*/
void idInternalCVar::Set(std::string newValue, bool force, bool fromServer) {
	if (/*common->IsMultiplayer() &&*/ !fromServer) {
#ifndef ID_TYPEINFO
		if ((flags & CVAR_NETWORKSYNC) /*&& common->IsClient()*/) {
			common->Printf("%s is a synced over the network and cannot be changed on a multiplayer client.\n", nameString.c_str());
			return;
		}
#endif
		if ((flags & CVAR_CHEAT) /*&& !net_allowCheats.GetBool()*/) {
			common->Printf("%s cannot be changed in multiplayer.\n", nameString.c_str());
			return;
		}
	}

	if (newValue.empty()) {
		newValue = resetString;
	}

	if (!force) {
		if (flags & CVAR_ROM) {
			common->Printf("%s is read only.\n", nameString.c_str());
			return;
		}

		if (flags & CVAR_INIT) {
			common->Printf("%s is write protected.\n", nameString.c_str());
			return;
		}
	}

	if (valueString.compare(newValue) == 0) {
		return;
	}

	valueString = newValue;
	value = valueString;
	UpdateValue();

	SetModified();
	cvarSystem->SetModifiedFlags(flags);
}

/*
============
idInternalCVar::InternalSetString
============
*/
void idInternalCVar::InternalSetString(const std::string& newValue) {
	Set(newValue, true, false);
}

/*
===============================================================================

	idCVarSystemLocal

===============================================================================
*/

class idCVarSystemLocal : public idCVarSystem {
public:
	idCVarSystemLocal();

	virtual ~idCVarSystemLocal() {}

	virtual void Init() override;
	virtual void Shutdown() override;
	virtual bool IsInitialized() const override;

	virtual void Register(idCVar* cvar) override;

	virtual void SetCVarString(const std::string& name, const std::string& value, int flags = 0) override;

	virtual bool Command(const idCmdArgs& args) override;

	virtual void SetModifiedFlags(int flags) override;

	idInternalCVar* FindInternal(const std::string& name) const;
	void SetInternal(const std::string& name, const std::string& value, int flags);
private:
	bool initialized;
	using cvars_map_type = std::map<std::string, idInternalCVar*>;
	cvars_map_type cvars;
	int modifiedFlags;
private:
	static void Set_f(const idCmdArgs& args);
};

idCVarSystemLocal			localCVarSystem;
idCVarSystem* cvarSystem = &localCVarSystem;

/*
============
idCVarSystemLocal::FindInternal
============
*/
idInternalCVar* idCVarSystemLocal::FindInternal(const std::string& name) const {
	auto iter = cvars.find(name);

	if (iter == cvars.end())
		return nullptr;
	
	return iter->second;
}

/*
============
idCVarSystemLocal::SetInternal
============
*/
void idCVarSystemLocal::SetInternal(const std::string& name, const std::string& value, int flags) {
	auto internal = FindInternal(name);

	if (internal) {
		internal->InternalSetString(value);
		internal->flags |= flags & ~CVAR_STATIC;
		//internal->UpdateCheat();
	}
	else {
		internal = new idInternalCVar(name, value, flags);
		cvars.insert(cvars_map_type::value_type(internal->GetName(), internal));
	}
}

/*
============
idCVarSystemLocal::idCVarSystemLocal
============
*/
idCVarSystemLocal::idCVarSystemLocal() : initialized(false), modifiedFlags(0) {
}

/*
============
idCVarSystemLocal::Init
============
*/
void idCVarSystemLocal::Init() {

	modifiedFlags = 0;

	cmdSystem->AddCommand("set", Set_f, CMD_FL_SYSTEM, "sets a cvar");
	/*cmdSystem->AddCommand("seta", Set_f, CMD_FL_SYSTEM, "sets a cvar");
	cmdSystem->AddCommand("sets", Set_f, CMD_FL_SYSTEM, "sets a cvar");
	cmdSystem->AddCommand("sett", Set_f, CMD_FL_SYSTEM, "sets a cvar");
	cmdSystem->AddCommand("setu", Set_f, CMD_FL_SYSTEM, "sets a cvar");*/

	initialized = true;
}

/*
============
idCVarSystemLocal::Shutdown
============
*/
void idCVarSystemLocal::Shutdown() {
	
	for (auto iter = cvars.begin(); iter != cvars.end(); ++iter) {
		delete iter->second;
	}

	initialized = false;
}

/*
============
idCVarSystemLocal::IsInitialized
============
*/
bool idCVarSystemLocal::IsInitialized() const {
	return initialized;
}

/*
============
idCVarSystemLocal::Register
============
*/
void idCVarSystemLocal::Register(idCVar* cvar) {
	cvar->SetInternalVar(cvar);

	auto internal = FindInternal(cvar->GetName());

	if (internal) {
		internal->Update(cvar);
	}
	else {
		internal = new idInternalCVar(cvar);
		cvars.insert({ internal->nameString, internal });
	}

	cvar->SetInternalVar(internal);
}

/*
============
idCVarSystemLocal::SetCVarString
============
*/
void idCVarSystemLocal::SetCVarString(const std::string& name, const std::string& value, int flags) {
	SetInternal(name, value, flags);
}

/*
============
idCVarSystemLocal::Command
============
*/
bool idCVarSystemLocal::Command(const idCmdArgs& args) {
	idInternalCVar* internal;

	internal = FindInternal(args.Argv(0));

	if (internal == NULL) {
		return false;
	}

	if (args.Argc() == 1) {
		// print the variable
		common->Printf("\"%s\" is:\"%s\" default:\"%s\"\n",
			internal->nameString.c_str(), internal->valueString.c_str(), internal->resetString.c_str());
		if (!internal->GetDescription().empty()) {
			common->Printf("%s\n", internal->GetDescription().c_str());
		}
	}
	else {
		// set the value
		internal->Set(args.Args(), false, false);
	}
	return true;
}

/*
============
idCVarSystemLocal::SetModifiedFlags
============
*/
void idCVarSystemLocal::SetModifiedFlags(int flags) {
	modifiedFlags |= flags;
}

/*
============
idCVarSystemLocal::Set_f
============
*/
void idCVarSystemLocal::Set_f(const idCmdArgs& args) {
	std::string str;

	str = args.Args(2, args.Argc() - 1);
	localCVarSystem.SetCVarString(args.Argv(1), str);
}