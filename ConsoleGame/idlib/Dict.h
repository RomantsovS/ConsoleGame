#ifndef DICT_H
#define DICT_H

#include <map>
#include <string>

#include "Math/Vector2.h"
#include "Str.h"

class idDict
{
public:
	using map_type = std::map<std::string, std::string>;
	using args_pair = map_type::value_type;

	idDict();
	idDict(const idDict &other);
	~idDict();

	// copy key/value pairs from other dict not present in this dict
	void SetDefaults(const idDict* dict);
	// clear dict freeing up memory
	void Clear();

	void Set(std::string key, std::string value);
	void SetInt(const std::string& key, int val);

	// these return default values of 0.0, 0 and false
	std::string GetString(const char *key, std::string defaultString = "") const;
	int	GetInt(const std::string key, const int defaultInt = 0) const;
	Vector2 GetVector(const std::string key, const std::string defaultString = "") const;

	bool GetString(const std::string key, const std::string defaultString, std::string *out) const;
	bool GetInt(const std::string key, const std::string defaultString, int &out) const;
	bool GetVector(const std::string key, std::string defaultString, Vector2 &out) const;

	// returns the key/value pair with the given key
	// returns NULL if the key/value pair does not exist
	const std::string& FindKey(const std::string& key) const;
	// finds the next key/value pair with the given key prefix.
	// lastMatch can be used to do additional searches past the first match.
	const idDict::args_pair* MatchPrefix(const std::string &prefix, const std::string lastMatch = "") const;
private:
	map_type args;
};

inline void idDict::SetInt(const std::string& key, int val) {
	Set(key, va("%i", val));
}

#endif