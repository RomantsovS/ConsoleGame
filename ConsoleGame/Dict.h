#ifndef DICT_H
#define DICT_H

#include <map>
#include <string>

#include "Vector2.h"

class idDict
{
public:
	idDict();
	idDict(const idDict &other);
	~idDict();

	// clear dict freeing up memory
	void Clear();

	void Set(std::string key, std::string value);

	// these return default values of 0.0, 0 and false
	std::string GetString(const char *key, std::string defaultString = "") const;
	int	GetInt(const std::string key, const int defaultInt = 0) const;

	bool GetString(const std::string key, const std::string defaultString, std::string *out) const;
	bool GetInt(const std::string key, const std::string defaultString, int &out) const;
	bool GetVector(const std::string key, std::string defaultString, Vector2 &out) const;
private:
	std::map<std::string, std::string> args;
};

#endif