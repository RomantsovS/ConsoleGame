#ifndef DICT_H
#define DICT_H

#include <map>
#include <string>

#include "Vector2.h"

class Dict
{
public:
	Dict();
	Dict(const Dict &other);
	~Dict();



	void Set(std::string key, std::string value);

	bool GetString(const std::string key, const std::string defaultString, std::string *out) const;

	bool GetVector(const std::string key, std::string defaultString, Vector2 &out) const;
private:
	std::map<std::string, std::string> args;
};

#endif