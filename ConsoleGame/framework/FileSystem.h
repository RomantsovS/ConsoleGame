#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include <string>

class idFile;

class idFileSystem {
public:
	virtual					~idFileSystem() {}
	// Initializes the file system.
	virtual void			Init() = 0;
	// Shutdown the file system.
	virtual void			Shutdown(bool reloading) = 0;
	// Returns true if the file system is initialized.
	virtual bool			IsInitialized() const = 0;

	virtual int				ReadFile(const std::string& file_name) = 0;

	// Opens a file for writing, will create any needed subdirectories.
	virtual std::shared_ptr<idFile> OpenFileWrite(const std::string& file_name) = 0;

	// Closes a file.
	virtual void			CloseFile(std::shared_ptr<idFile> f) = 0;
};

extern idFileSystem* fileSystem;

#endif // !FILE_SYSTEM_H