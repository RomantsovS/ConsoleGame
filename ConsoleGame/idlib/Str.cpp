#include <cstdarg>
#include <cstdio>

#include "Str.h"

template<>
std::string idStr::GetFromValue<int>(const int i)
{
	char text[64];

	sprintf_s(text, "%d", i);

	return std::string(text);
}

template<>
std::string idStr::GetFromValue<float>(const float i)
{
	char text[64];

	sprintf_s(text, "%f", i);

	return std::string(text);
}

int idStr::vsnPrintf(char* dest, int size, const char* fmt, va_list argptr)
{
	int ret = vsnprintf_s(dest, size, size, fmt, argptr);

	dest[size - 1] = '\0';

	if (ret < 0 || ret >= size) {
		return -1;
	}
	return ret;
}

/*
============
sprintf

Sets the value of the string using a printf interface.
============
*/
int sprintf(std::string& string, const char* fmt, ...) {
	int l;
	va_list argptr;
	char buffer[32000];

	va_start(argptr, fmt);
	l = idStr::vsnPrintf(buffer, sizeof(buffer) - 1, fmt, argptr);
	va_end(argptr);
	buffer[sizeof(buffer) - 1] = '\0';

	string = buffer;
	return l;
}

void idStr::StripTrailing(std::string& str, const char c)
{
	int i;

	for (i = str.size(); i > 0 && str[i - 1] == c; i--) {
		str[i - 1] = '\0';
	}

	str.shrink_to_fit();
}

/*
============
idStr::SlashesToBackSlashes
============
*/
void idStr::SlashesToBackSlashes(std::string& str) {
	auto pos = str.find('/', 0);

	while (pos != std::string::npos) {
		str.replace(pos, 1, 1, '\\');
	}
}

/*
============
idStr::IsNumeric

Checks a string to see if it contains only numerical values.
============
*/
bool idStr::IsNumeric(const std::string& s) {
	if (s.empty())
		return true;

	auto iter = s.begin();

	if (*iter == '-') {
		iter++;
	}

	bool dot = false;
	for (; iter != s.end(); ++iter) {
		if (!isdigit((const unsigned char)*iter)) {
			if ((*iter == '.') && !dot) {
				dot = true;
				continue;
			}
			return false;
		}
	}

	return true;
}