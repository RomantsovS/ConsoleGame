#ifndef FRAMEWORK_EDIT_FIELD_H_
#define FRAMEWORK_EDIT_FIELD_H_

#include <string>

class idEditField {
public:
	idEditField();
	~idEditField();

	void Clear();
	void CharEvent(int c);
	void KeyDownEvent(int key);
	const char* GetBuffer();
	const std::string& GetBufferString() { return buffer; }

private:
	std::string buffer;
};

#endif