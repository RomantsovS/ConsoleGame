#ifndef DICT_H
#define DICT_H

class idDict {
public:
	using map_type = std::map<std::string, std::string>;
	using args_pair = map_type::value_type;

	idDict();
	idDict(const idDict &other);
	~idDict();
	idDict& operator=(const idDict&) = default;
	idDict(idDict&&) = default;
	idDict& operator=(idDict&&) = default;

	// copy key/value pairs from other dict not present in this dict
	void SetDefaults(const idDict* dict);
	// clear dict freeing up memory
	void Clear() noexcept;

	void Set(const std::string& key, const std::string& value);
	void SetInt(const std::string& key, int val);

	// these return default values of 0.0, 0 and false
	const std::string& GetString(const std::string& key, const std::string& defaultString = "") const;
	int GetInt(const std::string& key, const std::string& defaultString) const;
	bool GetBool(const std::string key, const char* defaultString) const;
	float GetFloat(const std::string& key, const float defaultFloat = 0.0f) const;
	int	GetInt(const std::string& key, const int defaultInt = 0) const;
	bool GetBool(const std::string& key, const bool defaultBool = false) const;
	Vector2 GetVector(const std::string key, const std::string defaultString = "") const;

	bool GetString(const std::string& key, const std::string& defaultString, gsl::not_null<std::string*> out) const;
	bool GetInt(const std::string key, const std::string defaultString, int &out) const;
	bool GetVector(const std::string key, std::string defaultString, Vector2 &out) const;

	// returns the key/value pair with the given key
	// returns NULL if the key/value pair does not exist
	std::string FindKey(const std::string& key) const;
	// finds the next key/value pair with the given key prefix.
	// lastMatch can be used to do additional searches past the first match.
	const idDict::args_pair* MatchPrefix(const std::string &prefix, const std::string lastMatch = "") const;
private:
	map_type args;
};

inline void idDict::SetInt(const std::string& key, int val) {
	Set(key, va("%i", val));
}

inline bool idDict::GetBool(const std::string key, const char* defaultString) const {
	return (atoi(GetString(key, defaultString).c_str()) != 0);
}

inline float idDict::GetFloat(const std::string& key, const float defaultFloat) const {
	auto iter = args.find(key);

	if (iter != args.end()) {
		return static_cast<float>(std::atof(iter->second.c_str()));
	}

	return defaultFloat;
}

#endif