#ifndef FILE_H
#define FILE_H

#include <fstream>
#include <string>
#include "../sys/sys_public.h"

class idFileSystemLocal;

class idFile {
	friend class idFileSystemLocal;
public:
	idFile();
	virtual ~idFile();

	// Read data from the file to the buffer.
	virtual int Read(void* buffer, int len);
	// Write data from the buffer to the file.
	virtual int Write(const void* buffer, int len);
	// Returns the length of the file.
	virtual int Length() const;
	// Causes any buffered data to be written to the file.
	virtual void Flush();

	virtual int WriteString(const std::string& str);
};

class idFile_Permanent : public idFile {
	friend class			idFileSystemLocal;

public:
	idFile_Permanent(const std::string &file_name, std::ios_base::openmode ios_mode);
	virtual ~idFile_Permanent();

	virtual int Read(void* buffer, int len) override;
	virtual int Write(const void* buffer, int len) override;
	virtual int Length() const override;
	virtual void Flush() override;

	// returns file pointer
	idFileHandle &GetFilePtr() { return o; }

private:
	std::string name;		// relative path of the file - relative path
	std::string fullPath;	// full file path - OS path
	int mode;				// open mode
	int fileSize;			// size of the file
	idFileHandle o;			// file handle
};

#endif // !FILE_H

