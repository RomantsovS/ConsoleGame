#include <sstream>

#include "Dict.h"

Dict::Dict()
{
}

Dict::Dict(const Dict & other)
{
	args.clear();

	args = other.args;
}

Dict::~Dict()
{
}

void Dict::Set(std::string key, std::string value)
{
	args.insert(std::make_pair(key, value));
}

bool Dict::GetString(const std::string key, std::string defaultString, std::string *out) const
{
	auto iter = args.find(key);
	
	if(iter != args.end())
	{
		*out = iter->second;
		return true;
	}

	*out = defaultString;

	return false;
}

int Dict::GetInt(const std::string key, const int defaultInt) const
{
	auto iter = args.find(key);

	if (iter != args.end())
	{
		return std::atoi(iter->second.c_str());
	}

	return defaultInt;
}

bool Dict::GetVector(const std::string key, std::string defaultString, Vector2 & out) const
{
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
