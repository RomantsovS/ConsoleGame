#include "precompiled.h"
#pragma hdrstop

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

/*
============
idStr::StripTrailingWhitespace
============
*/
void idStr::StripTrailingWhitespace(std::string& str) {
	int i;

	// cast to unsigned char to prevent stripping off high-ASCII characters
	for (i = str.size(); i > 0 && (unsigned char)(str[i - 1]) <= ' '; i--) {
		str.resize(str.size() - 1);
	}
}

/*
=============
idStr::Copynz

Safe strncpy that ensures a trailing zero
=============
*/
void idStr::Copynz(char* dest, const char* src, int destsize) {
	if (!src) {
		//common->Warning("idStr::Copynz: NULL src");
		return;
	}
	if (destsize < 1) {
		//common->Warning("idStr::Copynz: destsize < 1");
		return;
	}

	strncpy_s(dest, destsize - 1, src, 256);
	dest[destsize - 1] = 0;
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

std::string va(const char* fmt, ...) {
	std::string str;
	int l;
	va_list argptr;
	char buffer[32000];

	va_start(argptr, fmt);
	l = idStr::vsnPrintf(buffer, sizeof(buffer) - 1, fmt, argptr);
	va_end(argptr);
	buffer[sizeof(buffer) - 1] = '\0';

	return buffer;
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

bool idStr::caseInSensStringCompareCpp11(const std::string& str1, const std::string& str2)
{
	return ((str1.size() == str2.size()) && std::equal(str1.begin(), str1.end(), str2.begin(),
		[](auto& c1, auto& c2) { return (c1 == c2 || std::toupper(c1) == std::toupper(c2)); }
	));
}

/*
============
idStr::SetFileExtension
============
*/
void idStr::SetFileExtension(std::string& str, const std::string extension) {
	StripFileExtension(str);
	if (!extension.empty() && extension[0] != '.') {
		str.append(".");
	}
	str.append(extension);
}

/*
============
idStr::StripFileExtension
============
*/
void idStr::StripFileExtension(std::string& str) {
	int i;

	for (i = str.size() - 1; i >= 0; i--) {
		if (str[i] == '.') {
			str.resize(i);
			break;
		}
	}
}
