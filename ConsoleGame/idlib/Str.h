#ifndef IDLIB_TEXT_STR_H_
#define IDLIB_TEXT_STR_H_

class idStr {
public:
	template<typename T>
	static std::string GetFromValue(const T i);

	static void StripTrailingWhitespace(std::string& str);				// strip trailing white space characters

	static int vsnPrintf(gsl::not_null<gsl::zstring<>> dest, int size, gsl::czstring<> fmt, va_list argptr);
	static void StripTrailing(std::string& str, const char c);
	static void SlashesToBackSlashes(std::string &str); // convert slashes
	static bool IsNumeric(const std::string& s) noexcept;

	static void Copynz(gsl::not_null<gsl::zstring<>> dest, gsl::czstring<> src, int destsize);

	static bool caseInSensStringCompareCpp11(const std::string& str1, const std::string& str2);

	static void SetFileExtension(std::string& str, const std::string extension);		// set the given file extension
	static void StripFileExtension(std::string& str); // remove any file extension
	static void ExtractFileExtension(const std::string& str, std::string& dest);		// copy the file extension to another string
};

std::string va(const char* fmt, ...);

int sprintf(std::string& string, const char* fmt, ...);

#endif // !IDLIB_TEXT_STR_H_
