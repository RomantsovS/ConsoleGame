#ifndef FILE_H
#define FILE_H

#include <fstream>
#include <string>

class idFileSystemLocal;

class idFile {
	friend class idFileSystemLocal;
public:
	idFile(const std::string file_name, std::ios::openmode mode);
	~idFile();

	// Write data from the buffer to the file.
	virtual int Write(const std::string& buffer);
	// Causes any buffered data to be written to the file.
	virtual void Flush();

	void WriteStr(const std::string& str);
private:
	std::fstream fstream_;
	std::string file_name_;
};

#endif // !FILE_H

