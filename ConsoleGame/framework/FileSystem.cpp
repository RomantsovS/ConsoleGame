#include "../idlib/precompiled.h"
#pragma hdrstop

// search flags when opening a file
const int FSFLAG_SEARCH_DIRS = (1 << 0);
const int FSFLAG_RETURN_FILE_MEM = (1 << 1);

const std::string BASE_GAMEDIR = "base";

struct searchpath_t {
	searchpath_t(const std::string& path, const std::string& gamedir)
	{
		this->path = path;
		this->gamedir = gamedir;
	}

	std::string	path;		// c:\doom
	std::string gamedir;	// base
};

class idFileSystemLocal : public idFileSystem {
public:
	idFileSystemLocal() {}

	virtual void Init() override;
	virtual void Shutdown(bool reloading) override;
	virtual bool IsInitialized() const override;
	virtual std::string BuildOSPath(const std::string &base, const std::string &game, const std::string &relativePath);
	virtual void CreateOSPath(const std::string &OSPath);
	virtual int ReadFile(const std::string& relativePath, void** buffer) override;
	virtual void FreeFile(void* buffer);
	virtual std::shared_ptr<idFile> OpenFileReadFlags(const std::string &relativePath, int searchFlags, bool allowCopyFiles = true, const std::string &gamedir = nullptr);
	virtual std::shared_ptr<idFile> OpenFileRead(const std::string &relativePath, bool allowCopyFiles = true, const std::string &gamedir = "") override;
	virtual std::shared_ptr<idFile> OpenFileWrite(const std::string &relativePath, const std::string &basePath) override;

	virtual void CloseFile(std::shared_ptr<idFile> f) override;
private:
	std::vector<searchpath_t> searchPaths;
	std::string gameFolder;

	static idCVar fs_game_base;
private:
	void AddGameDirectory(const std::string &path, const std::string &dir);

	void SetupGameDirectories(const std::string &gameName);
	void Startup();
private:
	void ReplaceSeparators(std::string& path, char sep = PATHSEPARATOR_CHAR);
	std::shared_ptr<idFile> OpenOSFile(const std::string &name, fsMode_t mode);
	int DirectFileLength(idFileHandle &o);
};

idCVar idFileSystemLocal::fs_game_base("fs_game_base", "", CVAR_SYSTEM | CVAR_INIT | CVAR_SERVERINFO, "alternate mod path, searched after the main fs_game path, before the basedir");
idCVar fs_basepath("fs_basepath", "", CVAR_SYSTEM | CVAR_INIT, "");

idFileSystemLocal fileSystemLocal;
idFileSystem* fileSystem = &fileSystemLocal;

/*
================
idFileSystemLocal::OpenOSFile
================
*/
std::shared_ptr<idFile> idFileSystemLocal::OpenOSFile(const std::string &fileName, fsMode_t mode) {
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
void idFileSystemLocal::CreateOSPath(const std::string &OSPath) {
	char* ofs;

	// make absolutely sure that it can't back up the path
	// FIXME: what about c: ?
	if (OSPath.find("..") != std::string::npos || OSPath.find("::") != std::string::npos) {
#ifdef _DEBUG		
		common->DPrintf("refusing to create relative path \"%s\"\n", OSPath);
#endif
		return;
	}

	auto path(OSPath);
	idStr::SlashesToBackSlashes(path);
	for (ofs = &path[1]; *ofs; ofs++) {
		if (*ofs == PATHSEPARATOR_CHAR) {
			// create the directory
			*ofs = 0;
			Sys_Mkdir(path);
			*ofs = PATHSEPARATOR_CHAR;
		}
	}
}

void idFileSystemLocal::Init()
{
	if (fs_basepath.GetString().empty()) {
		fs_basepath.SetString(Sys_DefaultBasePath());
	}

	// try to start up normally
	Startup();

	// if we can't find default.cfg, assume that the paths are
	// busted and error out now, rather than getting an unreadable
	// graphics screen when the font fails to load
	// Dedicated servers can run with no outside files at all
	if (ReadFile("default.cfg", nullptr) <= 0) {
		common->FatalError("Couldn't load default.cfg");
	}
}

void idFileSystemLocal::Shutdown(bool reloading)
{
}

bool idFileSystemLocal::IsInitialized() const
{
	return !searchPaths.empty();
}

/*
====================
idFileSystemLocal::ReplaceSeparators

Fix things up differently for win/unix/mac
====================
*/
void idFileSystemLocal::ReplaceSeparators(std::string& path, char sep) {
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
static bool IsOSPath(const std::string path) {
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
std::string idFileSystemLocal::BuildOSPath(const std::string &base, const std::string &game, const std::string &relativePath) {
	static std::string OSPath;
	OSPath.resize(max_string_chars);

	std::string newPath;

	// handle case of this already being an OS path
	if (IsOSPath(relativePath)) {
		return relativePath;
	}

	std::string strBase = base;
	idStr::StripTrailing(strBase, '/');
	idStr::StripTrailing(strBase, '\\');
	sprintf(newPath, "%s/%s/%s", strBase.c_str(), game.c_str(), relativePath.c_str());
	ReplaceSeparators(newPath);
	OSPath = newPath;
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
std::shared_ptr<idFile> idFileSystemLocal::OpenFileReadFlags(const std::string &relativePath, int searchFlags, bool allowCopyFiles, const std::string &gamedir) {

	if (!IsInitialized()) {
		common->FatalError("Filesystem call made without initialization\n");
		return nullptr;
	}

	if (relativePath.empty()) {
		common->FatalError("idFileSystemLocal::OpenFileRead: empty 'relativePath' parameter passed\n");
		return nullptr;
	}

	// qpaths are not supposed to have a leading slash
	/*if (relativePath[0] == '/' || relativePath[0] == '\\') {
		relativePath++;
	}*/

	// make absolutely sure that it can't back up the path.
	// The searchpaths do guarantee that something will always
	// be prepended, so we don't need to worry about "c:" or "//limbo" 
	if (relativePath.find("..") != std::string::npos|| relativePath.find("::") != std::string::npos) {
		return nullptr;
	}

	// edge case
	if (relativePath[0] == '\0') {
		return nullptr;
	}

	if (true) {
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
			std::shared_ptr<idFile_Permanent> file = std::reinterpret_pointer_cast<idFile_Permanent>(OpenOSFile(netpath, FS_READ));
			if (!file->o) {
				continue;
			}

			file->name = relativePath;
			file->mode = (1 << FS_READ);
			file->fileSize = DirectFileLength(file->o);
			if (true) {
				common->Printf("idFileSystem::OpenFileRead: %s (found in '%s/%s')\n", relativePath.c_str(), searchPaths[sp].path.c_str(), searchPaths[sp].gamedir.c_str());
			}

			return file;
		}
	}

	if (true) {
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
============
idFileSystemLocal::ReadFile

Filename are relative to the search path
a null buffer will just return the file length and time without loading
timestamp can be NULL if not required
============
*/
int idFileSystemLocal::ReadFile(const std::string& relativePath, void** buffer)
{
	if (!IsInitialized()) {
		common->FatalError("Filesystem call made without initialization\n");
		return 0;
	}

	if (relativePath.empty() || !relativePath[0]) {
		common->FatalError("idFileSystemLocal::ReadFile with empty name\n");
		return 0;
	}

	if (buffer) {
		*buffer = nullptr;
	}

	// look for it in the filesystem or pack files
	auto f = OpenFileRead(relativePath, (buffer != nullptr));
	if (!f) {
		if (buffer) {
			*buffer = nullptr;
		}
		return -1;
	}
	auto len = f->Length();

	if (!buffer) {
		CloseFile(f);
		return len;
	}

	unsigned char* buf = new unsigned char[len + 1];
	*buffer = buf;

	f->Read(buf, len);

	// guarantee that it will have a trailing 0 for string operations
	buf[len] = 0;
	CloseFile(f);

	return len;
}

/*
=============
idFileSystemLocal::FreeFile
=============
*/
void idFileSystemLocal::FreeFile(void* buffer) {
	if (!IsInitialized()) {
		common->FatalError("Filesystem call made without initialization\n");
	}
	if (!buffer) {
		common->FatalError("idFileSystemLocal::FreeFile( NULL )");
	}

	delete[] buffer;
}

std::shared_ptr<idFile> idFileSystemLocal::OpenFileWrite(const std::string& relativePath, const std::string& basePath)
{
	if (!IsInitialized()) {
		common->FatalError("Filesystem call made without initialization\n");
	}

	auto path = basePath;
	auto OSpath = BuildOSPath(path, gameFolder, relativePath);

	if (true) {
		common->Printf("idFileSystem::OpenFileWrite: %s\n", OSpath.c_str());
	}

	common->DPrintf("writing to: %s\n", OSpath.c_str());
	CreateOSPath(OSpath);

	auto f = std::reinterpret_pointer_cast<idFile_Permanent>(OpenOSFile(OSpath, FS_WRITE));
	if (!f->o) {
		f = nullptr;
		return nullptr;
	}
	f->name = relativePath;
	f->mode = (1 << FS_WRITE);
	f->fileSize = 0;

	return f;
}

void idFileSystemLocal::CloseFile(std::shared_ptr<idFile> f)
{
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

void idFileSystemLocal::Startup()
{
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
