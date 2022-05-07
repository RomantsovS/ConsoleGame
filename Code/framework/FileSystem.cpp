#include "idlib/precompiled.h"


// search flags when opening a file
const int FSFLAG_SEARCH_DIRS = (1 << 0);
const int FSFLAG_RETURN_FILE_MEM = (1 << 1);

const std::string BASE_GAMEDIR = "base";

struct searchpath_t {
	searchpath_t(const std::filesystem::path& _path, const std::filesystem::path& _gamedir) : path(_path), gamedir(_gamedir) {
#ifdef DEBUG_PRINT_Ctor_Dtor
		common->DPrintf("%s ctor\n", "searchpath_t");
#endif // DEBUG_PRINT_Ctor_Dtor
	}

	~searchpath_t() {
#ifdef DEBUG_PRINT_Ctor_Dtor
		idLib::Printf("%s dtor\n", "searchpath_t");
#endif // DEBUG_PRINT_Ctor_Dtor
	}
	searchpath_t(const searchpath_t&) = default;
	searchpath_t& operator=(const searchpath_t&) = default;
	searchpath_t(searchpath_t&&) = default;
	searchpath_t& operator=(searchpath_t&&) = default;

	std::filesystem::path path; // c:\doom
	std::filesystem::path gamedir; // base
};

class idFileSystemLocal : public idFileSystem {
public:
	idFileSystemLocal() {}
	~idFileSystemLocal() {
		isFileSystemExists = false;
	}
	idFileSystemLocal(const idFileSystemLocal&) = default;
	idFileSystemLocal& operator=(const idFileSystemLocal&) = default;
	idFileSystemLocal(idFileSystemLocal&&) = default;
	idFileSystemLocal& operator=(idFileSystemLocal&&) = default;

	void Init() override;
	void Shutdown(bool reloading) noexcept override;
	bool IsInitialized() const noexcept override;
	std::shared_ptr<idFileList> ListFiles(const std::string& relativePath, const std::string& extension, bool sort = false, bool fullRelativePath = false, const std::string& gamedir = "") override;
	void FreeFileList(std::shared_ptr<idFileList> fileList) noexcept override;
	virtual std::filesystem::path BuildOSPath(const std::filesystem::path& base, const std::filesystem::path& game, const std::filesystem::path& relativePath);
	virtual void CreateOSPath(const std::filesystem::path& OSPath);
	std::unique_ptr<char[]> ReadFile(const std::string& relativePath, int& len, ID_TIME_T* timestamp, bool returnBuffer = false) override;
	virtual std::shared_ptr<idFile> OpenFileReadFlags(const std::string &relativePath, int searchFlags, bool allowCopyFiles = true, const std::string &gamedir = nullptr);
	std::shared_ptr<idFile> OpenFileRead(const std::string &relativePath, bool allowCopyFiles = true, const std::string &gamedir = "") override;
	std::shared_ptr<idFile> OpenFileReadMemory(const std::string& relativePath, bool allowCopyFiles = true, const std::string& gamedir = "") override;
	std::shared_ptr<idFile> OpenFileWrite(const std::string &relativePath, const std::string &basePath = "fs_basepath") override;

	void CloseFile(std::shared_ptr<idFile> f) override;
private:
	std::vector<searchpath_t> searchPaths;
	std::string gameFolder;

	static idCVar fs_debug;
	static idCVar fs_game_base;
private:
	void ReplaceSeparators(std::string& path, char sep = PATHSEPARATOR_CHAR) noexcept;
	int ListOSFiles(const std::filesystem::path& directory, const std::string& extension, std::vector<std::string>& list);
	std::shared_ptr<idFile> OpenOSFile(const std::filesystem::path& fileName, fsMode_t mode);

	void GetExtensionList(const std::string& extension, std::vector<std::string>& extensionList) const;
	int GetFileList(const std::filesystem::path& relativePath, const std::vector<std::string>& extensions, std::vector<std::string>& list, bool fullRelativePath, const std::string& gamedir = "");

	void AddGameDirectory(const std::string &path, const std::string &dir);

	void SetupGameDirectories(const std::string &gameName);
	void Startup();

	int DirectFileLength(idFileHandle &o);
};

idCVar idFileSystemLocal::fs_debug("fs_debug", "1", CVAR_SYSTEM | CVAR_INTEGER, "", 0, 2);

idCVar idFileSystemLocal::fs_game_base("fs_game_base", "", CVAR_SYSTEM | CVAR_INIT | CVAR_SERVERINFO, "alternate mod path, searched after the main fs_game path, before the basedir");
idCVar fs_basepath("fs_basepath", "", CVAR_SYSTEM | CVAR_INIT, "");

idFileSystemLocal fileSystemLocal;
idFileSystem* fileSystem = &fileSystemLocal;

bool isFileSystemExists = true;

/*
================
idFileSystemLocal::OpenOSFile
================
*/
std::shared_ptr<idFile> idFileSystemLocal::OpenOSFile(const std::filesystem::path& fileName, fsMode_t mode) {
	std::ios_base::openmode ios_mode;

	if (mode == FS_WRITE) {
		ios_mode = std::ios_base::out;
	}
	else if (mode == FS_READ) {
		ios_mode = std::ios_base::in;
	}
	else if (mode == FS_APPEND) {
		ios_mode = std::ios_base::out | std::ios_base::app;
	}

	ios_mode |= std::ifstream::binary;

	auto file = std::make_shared<idFile_Permanent>(fileName, ios_mode);

	return file;
}

/*
================
idFileSystemLocal::DirectFileLength
================
*/
int idFileSystemLocal::DirectFileLength(idFileHandle &o) {
	if (o) {
		o.seekg(0, o.end);
		int length = static_cast<int>(o.tellg());
		o.seekg(0, o.beg);

		return length;
	}

	return 0;
}

/*
============
idFileSystemLocal::CreateOSPath

Creates any directories needed to store the given filename
============
*/
void idFileSystemLocal::CreateOSPath(const std::filesystem::path& OSPath) {
	Sys_Mkdir(OSPath.parent_path());
}

void idFileSystemLocal::Init() {
	if (fs_basepath.GetString().empty()) {
		size_t i = 0;
		std::vector<std::string> list;
		std::filesystem::path path = Sys_DefaultBasePath();
		while (i < 5 && ListOSFiles(path / "base", "cfg", list) < 1) {
			path = path.parent_path();
			++i;
		}
		fs_basepath.SetString(path.string());
	}

	// try to start up normally
	Startup();

	// if we can't find default.cfg, assume that the paths are
	// busted and error out now, rather than getting an unreadable
	// graphics screen when the font fails to load
	// Dedicated servers can run with no outside files at all
	int len;
	ReadFile("default.cfg", len, nullptr);
	if (len <= 0) {
		common->FatalError("Couldn't load default.cfg");
	}
}

void idFileSystemLocal::Shutdown(bool reloading) noexcept {
}

bool idFileSystemLocal::IsInitialized() const noexcept {
	return !searchPaths.empty();
}

/*
====================
idFileSystemLocal::ReplaceSeparators

Fix things up differently for win/unix/mac
====================
*/
void idFileSystemLocal::ReplaceSeparators(std::string& path, char sep) noexcept {
	for (auto iter = path.begin(); iter != path.end(); ++iter) {
		if (*iter == '/' || *iter == '\\') {
			*iter = sep;
		}
	}
}

/*
========================
IsOSPath
========================
*/
static bool IsOSPath(const std::string path) noexcept {
	//assert(path);

	if (path.size() >= 2) {
		if (path[1] == ':') {
			if ((path[0] > 64 && path[0] < 91) || (path[0] > 96 && path[0] < 123)) {
				// already an OS path starting with a drive.
				return true;
			}
		}
		if (path[0] == '\\' || path[0] == '/') {
			// a root path
			return true;
		}
	}
	return false;
}

/*
===================
idFileSystemLocal::BuildOSPath
===================
*/
std::filesystem::path idFileSystemLocal::BuildOSPath(const std::filesystem::path& base, const std::filesystem::path& game, const std::filesystem::path& relativePath) {
	static std::filesystem::path OSPath;

	OSPath = base / game / relativePath;
	return OSPath;
}

/*
===========
idFileSystemLocal::OpenFileReadFlags

Finds the file in the search path, following search flag recommendations
Returns filesize and an open FILE pointer.
Used for streaming data out of either a
separate file or a ZIP file.
===========
*/
std::shared_ptr<idFile> idFileSystemLocal::OpenFileReadFlags(const std::string& relativePath, int searchFlags, bool allowCopyFiles, const std::string& gamedir) {

	if (!IsInitialized()) {
		common->FatalError("Filesystem call made without initialization\n");
		return nullptr;
	}

	if (relativePath.empty()) {
		common->FatalError("idFileSystemLocal::OpenFileRead: empty 'relativePath' parameter passed\n");
		return nullptr;
	}

	// make absolutely sure that it can't back up the path.
	// The searchpaths do guarantee that something will always
	// be prepended, so we don't need to worry about "c:" or "//limbo" 
	if (relativePath.find("..") != std::string::npos || relativePath.find("::") != std::string::npos) {
		return nullptr;
	}

	// edge case
	if (relativePath[0] == '\0') {
		return nullptr;
	}

	if (fs_debug.GetBool()) {
		common->Printf("FILE DEBUG: opening %s\n", relativePath.c_str());
	}

	//
	// search through the path, one element at a time
	//
	if (searchFlags & FSFLAG_SEARCH_DIRS) {
		for (int sp = searchPaths.size() - 1; sp >= 0; sp--) {
			if (!gamedir.empty() && gamedir[0] != 0) {
				if (searchPaths[sp].gamedir != gamedir) {
					continue;
				}
			}

			auto netpath = BuildOSPath(searchPaths[sp].path, searchPaths[sp].gamedir, relativePath);
			std::shared_ptr<idFile_Permanent> file = std::dynamic_pointer_cast<idFile_Permanent>(OpenOSFile(netpath, FS_READ));
			if (!file->o) {
				continue;
			}

			file->name = relativePath;
			file->mode = (1 << FS_READ);
			file->fileSize = DirectFileLength(file->o);
			if (fs_debug.GetInteger()) {
				common->Printf("idFileSystem::OpenFileRead: %s (found in '%s')\n", relativePath.c_str(),
					(searchPaths[sp].path / searchPaths[sp].gamedir).string().c_str());
			}

			return file;
		}
	}

	if (fs_debug.GetInteger()) {
		common->Printf("Can't find %s\n", relativePath.c_str());
	}

	return nullptr;
}

/*
===========
idFileSystemLocal::OpenFileRead
===========
*/
std::shared_ptr<idFile> idFileSystemLocal::OpenFileRead(const std::string &relativePath, bool allowCopyFiles, const std::string &gamedir) {
	return OpenFileReadFlags(relativePath, FSFLAG_SEARCH_DIRS, allowCopyFiles, gamedir);
}

/*
===========
idFileSystemLocal::OpenFileReadMemory
===========
*/
std::shared_ptr<idFile> idFileSystemLocal::OpenFileReadMemory(const std::string& relativePath, bool allowCopyFiles, const std::string& gamedir) {
	return OpenFileReadFlags(relativePath, FSFLAG_SEARCH_DIRS | FSFLAG_RETURN_FILE_MEM, allowCopyFiles, gamedir);
}

/*
============
idFileSystemLocal::ReadFile

Filename are relative to the search path
a null buffer will just return the file length and time without loading
timestamp can be NULL if not required
============
*/
std::unique_ptr<char[]> idFileSystemLocal::ReadFile(const std::string& relativePath, int& len, ID_TIME_T* timestamp, bool returnBuffer) {
	if (!IsInitialized()) {
		common->FatalError("Filesystem call made without initialization\n");
		return 0;
	}

	if (relativePath.empty() || !relativePath[0]) {
		common->FatalError("idFileSystemLocal::ReadFile with empty name\n");
		return 0;
	}

	if (timestamp) {
		*timestamp = FILE_NOT_FOUND_TIMESTAMP;
	}

	// look for it in the filesystem or pack files
	auto f = OpenFileRead(relativePath, returnBuffer);
	if (!f) {
		len = -1;
		return nullptr;
	}
	len = f->Length();

	if (timestamp) {
		*timestamp = f->Timestamp();
	}

	if (!returnBuffer) {
		CloseFile(f);
		return nullptr;
	}

	auto buf = std::make_unique<char[]>(len + 1);

	f->Read(buf.get(), len);

	// guarantee that it will have a trailing 0 for string operations
	buf.operator[](len) = 0;
	CloseFile(f);

	return buf;
}

/*
===============
idFileSystemLocal::GetExtensionList
===============
*/
void idFileSystemLocal::GetExtensionList(const std::string& extension, std::vector<std::string>& extensionList) const {
	int s, e, l;

	l = extension.size();
	s = 0;
	while (1) {
		e = extension.find('|');
		if (e != std::string::npos) {
			extensionList.push_back(extension.substr(s, e));
			s = e + 1;
		}
		else {
			extensionList.push_back(extension.substr(s, l));
			break;
		}
	}
}

/*
===============
idFileSystemLocal::GetFileList

Does not clear the list first so this can be used to progressively build a file list.
When 'sort' is true only the new files added to the list are sorted.
===============
*/
int idFileSystemLocal::GetFileList(const std::filesystem::path& relativePath, const std::vector<std::string>& extensions, std::vector<std::string>& list, bool fullRelativePath, const std::string& gamedir) {
	if (!IsInitialized()) {
		common->FatalError("Filesystem call made without initialization\n");
	}

	if (extensions.empty()) {
		return 0;
	}

	if (relativePath.empty()) {
		return 0;
	}

	// search through the path, one element at a time, adding to list
	for (int sp = searchPaths.size() - 1; sp >= 0; sp--) {
		if (!gamedir.empty() && gamedir[0] != 0) {
			if (searchPaths[sp].gamedir != gamedir) {
				continue;
			}
		}

		std::filesystem::path netpath = BuildOSPath(searchPaths[sp].path, searchPaths[sp].gamedir, relativePath);

		for (size_t i = 0; i < extensions.size(); i++) {

			// scan for files in the filesystem
			std::vector<std::string> sysFiles;
			ListOSFiles(netpath, extensions[i], sysFiles);

			// if we are searching for directories, remove . and ..
			if (extensions[i][0] == '/' && extensions[i][1] == 0) {
				sysFiles.erase(std::remove(sysFiles.begin(), sysFiles.end(), "."), sysFiles.end());
				sysFiles.erase(std::remove(sysFiles.begin(), sysFiles.end(), ".."), sysFiles.end());
			}

			for (size_t j = 0; j < sysFiles.size(); j++) {
				// unique the match
				if (fullRelativePath) {
					std::filesystem::path work = relativePath / sysFiles[j];
					if(std::find(list.begin(), list.end(), work) == list.end())
						list.push_back(work.string());
				}
				else {
					if (std::find(list.begin(), list.end(), sysFiles[j]) == list.end())
						list.push_back(sysFiles[j]);
				}
			}
		}
	}

	return list.size();
}

/*
===============
idFileSystemLocal::ListFiles
===============
*/
std::shared_ptr<idFileList> idFileSystemLocal::ListFiles(const std::string& relativePath, const std::string& extension, bool sort, bool fullRelativePath, const std::string& gamedir) {
	std::vector<std::string> extensionList;

	std::shared_ptr<idFileList> fileList = std::make_shared<idFileList>();
	fileList->basePath = relativePath;

	GetExtensionList(extension, extensionList);

	GetFileList(relativePath, extensionList, fileList->list, fullRelativePath, gamedir);

	if (sort) {
		std::sort(fileList->list.begin(), fileList->list.end());
	}

	return fileList;
}

/*
===============
idFileSystemLocal::FreeFileList
===============
*/
void idFileSystemLocal::FreeFileList(std::shared_ptr<idFileList> fileList) noexcept {
	fileList = nullptr;
}

/*
===============
idFileSystemLocal::ListOSFiles

 call to the OS for a listing of files in an OS directory
===============
*/
int	idFileSystemLocal::ListOSFiles(const std::filesystem::path& directory, const std::string& extension, std::vector<std::string>& list) {
	return Sys_ListFiles(directory, extension, list);
}

std::shared_ptr<idFile> idFileSystemLocal::OpenFileWrite(const std::string& relativePath, const std::string& basePath) {
	if (!IsInitialized()) {
		common->FatalError("Filesystem call made without initialization\n");
	}

	std::filesystem::path path = cvarSystem->GetCVarString(basePath);
	if (path.empty()) {
		path = Sys_DefaultBasePath();
	}
	auto OSpath = BuildOSPath(path, gameFolder, relativePath);

	if (fs_debug.GetInteger()) {
		common->Printf("idFileSystem::OpenFileWrite: %s\n", OSpath.string().c_str());
	}

	CreateOSPath(OSpath);

	auto f = std::dynamic_pointer_cast<idFile_Permanent>(OpenOSFile(OSpath, FS_WRITE));
	if (!f->o) {
		f = nullptr;
		return nullptr;
	}
	f->name = relativePath;
	f->mode = (1 << FS_WRITE);
	f->fileSize = 0;

	return f;
}

void idFileSystemLocal::CloseFile(std::shared_ptr<idFile> f) {
	if (!IsInitialized()) {
		common->FatalError("Filesystem call made without initialization\n");
	}
	
	f = nullptr;
}

/*
================
idFileSystemLocal::AddGameDirectory

Sets gameFolder, adds the directory to the head of the search paths
================
*/
void idFileSystemLocal::AddGameDirectory(const std::string &path, const std::string &dir) {
	// check if the search path already exists
	for (size_t i = 0; i < searchPaths.size(); i++) {
		if (searchPaths[i].path.compare(path) == 0 && searchPaths[i].gamedir.compare(dir) == 0) {
			return;
		}
	}

	gameFolder = dir;

	//
	// add the directory to the search path
	//
	searchPaths.emplace_back(path, dir);
}

/*
================
idFileSystemLocal::SetupGameDirectories

  Takes care of the correct search order.
================
*/
void idFileSystemLocal::SetupGameDirectories(const std::string &gameName) {
	// setup basepath
	AddGameDirectory(fs_basepath.GetString(), gameName);
}

void idFileSystemLocal::Startup() {
	common->Printf("------ Initializing File System ------\n");

	SetupGameDirectories(BASE_GAMEDIR);

	// fs_game_base override
	if (!fs_game_base.GetString().empty() &&
		(fs_game_base.GetString().compare(BASE_GAMEDIR))) {
		SetupGameDirectories(fs_game_base.GetString());
	}

	common->Printf("file system initialized.\n");
	common->Printf("--------------------------------------\n");
}
