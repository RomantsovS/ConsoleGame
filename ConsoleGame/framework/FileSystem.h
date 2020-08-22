#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include <string>

static const int MAX_OSPATH = 256;

// modes for OpenFileByMode
enum fsMode_t {
	FS_READ = 0,
	FS_WRITE = 1,
	FS_APPEND = 2
};

class idFile;

class idFileSystem {
public:
	virtual ~idFileSystem() {}
	// Initializes the file system.
	virtual void Init() = 0;
	// Shutdown the file system.
	virtual void Shutdown(bool reloading) = 0;
	// Returns true if the file system is initialized.
	virtual bool IsInitialized() const = 0;
	// Reads a complete file.
	// Returns the length of the file, or -1 on failure.
	// A null buffer will just return the file length without loading.
	// A null timestamp will be ignored.
	// As a quick check for existance. -1 length == not present.
	// A 0 byte will always be appended at the end, so string ops are safe.
	// The buffer should be considered read-only, because it may be cached for other uses.
	virtual int ReadFile(const std::string& file_name, void** buffer) = 0;
	// Frees the memory allocated by ReadFile.
	virtual void FreeFile(void* buffer) = 0;
	// Opens a file for reading.
	virtual std::shared_ptr<idFile> OpenFileRead(const std::string &relativePath, bool allowCopyFiles = true, const std::string &gamedir = nullptr) = 0;
	// Opens a file for writing, will create any needed subdirectories.
	virtual std::shared_ptr<idFile> OpenFileWrite(const std::string& relativePath, const std::string& basePath) = 0;

	// Closes a file.
	virtual void			CloseFile(std::shared_ptr<idFile> f) = 0;
};

extern idFileSystem* fileSystem;

#endif // !FILE_SYSTEM_H