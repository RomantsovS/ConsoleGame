#include <windows.h>
#include "File.h"
#include "Common_local.h"
#include "FileSystem.h"

idFile::idFile()
{
#ifdef DEBUG_PRINT_Ctor_Dtor
	common->DPrintf("%s ctor\n", "idFile");
#endif // DEBUG_PRINT_Ctor_Dtor
}

idFile::~idFile()
{
#ifdef DEBUG_PRINT_Ctor_Dtor
	common->DPrintf("%s dtor\n", "idFile");
#endif
}

/*
=================
idFile::Read
=================
*/
int idFile::Read(void* buffer, int len) {
	common->FatalError("idFile::Read: cannot read from idFile");
	return 0;
}

int idFile::Write(const void* buffer, int len)
{
	common->FatalError("idFile::Write: cannot write to idFile");
	return 0;
}

/*
=================
idFile::Length
=================
*/
int idFile::Length() const {
	return 0;
}

void idFile::Flush()
{
}

int idFile::WriteString(const std::string& str)
{
	return Write(str.c_str(), str.size());
}

/*
=================================================================================

idFile_Permanent

=================================================================================
*/

/*
=================
idFile_Permanent::idFile_Permanent
=================
*/
idFile_Permanent::idFile_Permanent(const std::string& file_name, std::ios_base::openmode ios_mode) : fullPath(file_name), o(file_name, ios_mode) {
}

/*
=================
idFile_Permanent::~idFile_Permanent
=================
*/
idFile_Permanent::~idFile_Permanent() {
	if (o) {
		o.close();
	}
}

/*
=================
idFile_Permanent::Read

Properly handles partial reads
=================
*/
int idFile_Permanent::Read(void* buffer, int len) {
	int remaining;
	int read;
	char *buf;
	int tries;

	if (!(mode & (1 << FS_READ))) {
		common->FatalError("idFile_Permanent::Read: %s not opened in read mode", name.c_str());
		return 0;
	}

	if (!o) {
		return 0;
	}

	buf = (char*)buffer;

	remaining = len;
	tries = 0;
	while (remaining) {
		o.read(buf, fileSize);
		auto state = o.rdstate();
		if (o.eof()) {
			common->Warning("idFile_Permanent::Read failed with %d from %s", "eofbit", name.c_str());
		}
		else if (o.fail()) {
			common->Warning("idFile_Permanent::Read failed with %d from %s", "failbit", name.c_str());
		}
		else if (o.bad()) {
			common->Warning("idFile_Permanent::Read failed with %d from %s", "badbit", name.c_str());
		}

		if (o) {
			read = fileSize;
		}
		else
		{
			read = static_cast<int>(o.gcount());

			if (read == 0) {
				// we might have been trying to read from a CD, which
				// sometimes returns a 0 read on windows
				if (!tries) {
					tries = 1;
				}
				else {
					return len - remaining;
				}
			}
		}

		if (read == -1) {
			common->FatalError("idFile_Permanent::Read: -1 bytes read from %s", name.c_str());
		}

		remaining -= read;
		buf += read;
	}
	return len;
}

/*
=================
idFile_Permanent::Write

Properly handles partial writes
=================
*/
int idFile_Permanent::Write(const void* buffer, int len) {
	int		block, remaining;
	int		written = 0;
	char* buf;
	int		tries;

	if (!(mode & (1 << FS_WRITE))) {
		common->FatalError("idFile_Permanent::Write: %s not opened in write mode", name.c_str());
		return 0;
	}

	if (!o) {
		return 0;
	}

	buf = (char*)buffer;

	remaining = len;
	tries = 0;
	while (remaining) {
		block = remaining;
		o.write(buf, len);

		if (o) {
			written = len;
		}
		else {
			if (!tries) {
				tries = 1;
			}
			else {
				common->Printf("idFile_Permanent::Write: 0 bytes written to %s\n", name.c_str());
				return 0;
			}
		}

		remaining -= written;
		buf += written;
		fileSize += written;
	}

	return len;
}

/*
=================
idFile_Permanent::Flush
=================
*/
void idFile_Permanent::Flush() {
	o.flush();
}

/*
================
idFile_Permanent::Length
================
*/
int idFile_Permanent::Length() const {
	return fileSize;
}