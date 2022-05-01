#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

static const long long	FILE_NOT_FOUND_TIMESTAMP = (long long)-1;
static const int MAX_OSPATH = 256;

// modes for OpenFileByMode
enum fsMode_t {
	FS_READ = 0,
	FS_WRITE = 1,
	FS_APPEND = 2
};

// file list for directory listings
class idFileList {
	friend class idFileSystemLocal;
public:
	const std::string& GetBasePath() const noexcept { return basePath; }
	size_t GetNumFiles() const noexcept { return list.size(); }
	const std::string& GetFile(int index) const noexcept { return list[index]; }
	const std::vector<std::string>& GetList() const noexcept { return list; }

private:
	std::string basePath;
	std::vector<std::string> list;
};

class idFile;

class idFileSystem {
public:
	idFileSystem() = default;
	virtual ~idFileSystem() = default;
	idFileSystem(const idFileSystem&) = default;
	idFileSystem& operator=(const idFileSystem&) = default;
	idFileSystem(idFileSystem&&) = default;
	idFileSystem& operator=(idFileSystem&&) = default;

	// Initializes the file system.
	virtual void Init() = 0;
	// Shutdown the file system.
	virtual void Shutdown(bool reloading) = 0;
	// Returns true if the file system is initialized.
	virtual bool IsInitialized() const = 0;
	// Lists files with the given extension in the given directory.
	// Directory should not have either a leading or trailing '/'
	// The returned files will not include any directories or '/' unless fullRelativePath is set.
	// The extension must include a leading dot and may not contain wildcards.
	// If extension is "/", only subdirectories will be returned.
	virtual std::shared_ptr<idFileList> ListFiles(const std::string& relativePath, const std::string& extension, bool sort = false, bool fullRelativePath = false, const std::string& gamedir = "") = 0;
	// Frees the given file list.
	virtual void FreeFileList(std::shared_ptr<idFileList> fileList) = 0;
	// Reads a complete file.
	// Returns the length of the file, or -1 on failure.
	// A null buffer will just return the file length without loading.
	// A null timestamp will be ignored.
	// As a quick check for existance. -1 length == not present.
	// A 0 byte will always be appended at the end, so string ops are safe.
	// The buffer should be considered read-only, because it may be cached for other uses.
	virtual std::unique_ptr<char[]> ReadFile(const std::string& relativePath, int& len, ID_TIME_T* timestamp, bool returnBuffer = false) = 0;

	// Opens a file for reading.
	virtual std::shared_ptr<idFile> OpenFileRead(const std::string &relativePath, bool allowCopyFiles = true, const std::string &gamedir = "") = 0;
	// Opens a file for reading, reads the file completely in memory and returns an idFile_Memory obj.
	virtual std::shared_ptr<idFile> OpenFileReadMemory(const std::string& relativePath, bool allowCopyFiles = true, const std::string& gamedir = "") = 0;
	// Opens a file for writing, will create any needed subdirectories.
	virtual std::shared_ptr<idFile> OpenFileWrite(const std::string& relativePath, const std::string& basePath = "fs_basepath") = 0;

	// Closes a file.
	virtual void CloseFile(std::shared_ptr<idFile> f) = 0;
};

extern idFileSystem* fileSystem;
extern bool isFileSystemExists;

#endif // !FILE_SYSTEM_H