#include <fstream>

#include "FileSystem.h"
#include "File.h"
#include "Common_local.h"

class idFile;

class idFileSystemLocal : public idFileSystem {
public:
	idFileSystemLocal() : is_initialized(false) {}

	virtual void Init() override;
	virtual void Shutdown(bool reloading) override;
	virtual bool IsInitialized() const override;

	virtual int ReadFile(const std::string& file_name) override;
	virtual std::shared_ptr<idFile> OpenFileWrite(const std::string& file_name) override;

	virtual void CloseFile(std::shared_ptr<idFile> f) override;
private:
	void Startup();

	bool is_initialized;
};

idFileSystemLocal fileSystemLocal;
idFileSystem* fileSystem = &fileSystemLocal;

void idFileSystemLocal::Init()
{
	// try to start up normally
	Startup();

	// if we can't find default.cfg, assume that the paths are
	// busted and error out now, rather than getting an unreadable
	// graphics screen when the font fails to load
	// Dedicated servers can run with no outside files at all
	if (ReadFile("default.cfg") <= 0) {
		common->FatalError("Couldn't load default.cfg");
	}

	is_initialized = true;
}

void idFileSystemLocal::Shutdown(bool reloading)
{
}

bool idFileSystemLocal::IsInitialized() const
{
	return is_initialized;
}

int idFileSystemLocal::ReadFile(const std::string& file_name)
{
	std::ifstream input_file_stream(file_name);

	if (input_file_stream)
	{
		input_file_stream.seekg(0, input_file_stream.end);
		int length = static_cast<int>(input_file_stream.tellg());
		input_file_stream.seekg(0, input_file_stream.beg);

		return length;
	}

	return -1;
}

std::shared_ptr<idFile> idFileSystemLocal::OpenFileWrite(const std::string& file_name)
{
	if (!IsInitialized()) {
		common->FatalError("Filesystem call made without initialization\n");
	}

	auto file = std::make_shared<idFile>(file_name, std::ios::out);

	if (!file->fstream_)
		return nullptr;

	return file;
}

void idFileSystemLocal::CloseFile(std::shared_ptr<idFile> f)
{
	f->fstream_.close();
}

void idFileSystemLocal::Startup()
{
	common->Printf("------ Initializing File System ------\n");

	common->Printf("file system initialized.\n");
	common->Printf("--------------------------------------\n");
}
