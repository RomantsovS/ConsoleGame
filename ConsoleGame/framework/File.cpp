#include "File.h"
#include "Common_local.h"

idFile::idFile(const std::string file_name, std::ios::openmode mode) : file_name_(file_name), fstream_(file_name, mode)
{
#ifdef DEBUG_PRINT_Ctor_Dtor
	common->DPrintf("%s ctor\n", "idFile");
#endif // DEBUG_PRINT_Ctor_Dtor
}

idFile::~idFile()
{
	common->DPrintf("%s dtor\n", "idFile");

	if (fstream_ && fstream_.is_open())
		fstream_.close();

}

int idFile::Write(const std::string& buffer)
{
	fstream_ << buffer;

	return 1;
}

void idFile::Flush()
{
	fstream_.flush();
}

void idFile::WriteStr(const std::string& str)
{
	fstream_ << str;
}
