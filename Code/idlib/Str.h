#ifndef IDLIB_TEXT_STR_H_
#define IDLIB_TEXT_STR_H_

#define idassert_ENUM_STRING( string, index )		( 1 / (int)!( static_cast<int>(string) - index ) ) ? #string : ""

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

template<typename ... Args>
std::string string_format(const std::string& format, Args ... args) {
	int size_s = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
	if (size_s <= 0) { throw std::runtime_error("Error during formatting."); }
	auto size = static_cast<size_t>(size_s);
	std::unique_ptr<char[]> buf(new char[size]);
	std::snprintf(buf.get(), size, format.c_str(), args ...);
	return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}

#endif // !IDLIB_TEXT_STR_H_
