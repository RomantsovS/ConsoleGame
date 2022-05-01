#ifndef SWF_PARAMLIST_H_
#define SWF_PARAMLIST_H_

// static list for script parameters
static const int SWF_MAX_PARMS = 16;

/*
================================================
idSWFParmList

A static list for script parameters that reduces the number of SWF allocations dramatically.
================================================
*/
class idSWFParmList : public std::vector<std::shared_ptr<idSWFScriptVar>> {
public:
	idSWFParmList() {
	}
	explicit idSWFParmList(const int num_) {
		resize(num_);
	}

	std::shared_ptr<idSWFScriptVar> Alloc();

	void	Append(const idSWFScriptVar& other);
	void	Append(std::shared_ptr<idSWFScriptObject> o);
	void	Append(std::shared_ptr<idSWFScriptFunction> f);
	void	Append(const char* s);
	void	Append(const std::string& s);
	//void	Append(std::shared_ptr<idSWFScriptString> s);
	void	Append(const float f);
	void	Append(const int i);
	void	Append(const size_t i);
	void	Append(const bool b);
};

#endif // !SWF_PARAMLIST_H_
