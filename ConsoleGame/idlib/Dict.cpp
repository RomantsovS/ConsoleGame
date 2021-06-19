#include "precompiled.h"
#pragma hdrstop

idDict::idDict() {
}

idDict::idDict(const idDict & other) {
	args.clear();

	args = other.args;
}

idDict::~idDict() {
}

/*
================
idDict::SetDefaults
================
*/
void idDict::SetDefaults(const idDict* dict) {
	for (const auto& def : dict->args) {
		auto iter = args.find(def.first);

		if (iter == args.end()) {
			args.insert_or_assign(def.first, def.second);
		}
	}
}

void idDict::Clear() {
	args.clear();
}

void idDict::Set(std::string key, std::string value) {
	args.insert_or_assign(key, value);
}

std::string idDict::GetString(const char * key, std::string defaultString) const {
	auto iter = args.find(key);

	if (iter != args.end()) {
		return iter->second;
	}

	return defaultString;
}

bool idDict::GetString(const std::string key, std::string defaultString, std::string *out) const {
	auto iter = args.find(key);
	
	if(iter != args.end()) {
		*out = iter->second;
		return true;
	}

	*out = defaultString;

	return false;
}

bool idDict::GetInt(const std::string key, const std::string defaultString, int & out) const {
	std::string s;
	bool found;

	found = GetString(key, defaultString, &s);
	out = atoi(s.c_str());
	return found;
}

int idDict::GetInt(const std::string key, const int defaultInt) const {
	auto iter = args.find(key);

	if (iter != args.end())
	{
		return std::atoi(iter->second.c_str());
	}

	return defaultInt;
}

Vector2 idDict::GetVector(const std::string key, const std::string defaultString) const {
	Vector2 out;
	GetVector(key, defaultString, out);
	return out;
}

bool idDict::GetVector(const std::string key, std::string defaultString, Vector2 & out) const {
	bool found;
	std::string s;

	if (defaultString.empty())
	{
		defaultString = "0 0";
	}

	found = GetString(key, defaultString, &s);
	
	out.Zero();
	
	std::istringstream is;

	is.str(s);

	is >> out.x >> out.y;

	return found;
}

/*
================
idDict::FindKey
================
*/
std::string idDict::FindKey(const std::string& key) const {
	if (key.empty()) {
		common->DWarning("idDict::FindKey: empty key");
		return "";
	}

	auto iter = args.find(key);
	if (iter != args.end())
		return iter->first;

	return "";
}

const idDict::args_pair * idDict::MatchPrefix(const std::string & prefix, const std::string lastMatch) const {
	map_type::const_iterator iter;

	if (!lastMatch.empty())
	{
		iter = args.lower_bound(lastMatch);
		++iter;
	}
	else
		iter = args.lower_bound(prefix);

	if (iter != args.end())
		if(!iter->first.compare(0, prefix.size(), prefix))
			return &*iter;

	return nullptr;
}
