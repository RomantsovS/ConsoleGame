#ifndef FILE_H
#define FILE_H

class idFileSystemLocal;

class idFile {
	friend class idFileSystemLocal;
public:
	idFile();
	virtual ~idFile();
	// Get the full file path.
	virtual const std::string GetFullPath() const;
	// Read data from the file to the buffer.
	virtual int Read(void* buffer, int len);
	// Write data from the buffer to the file.
	virtual int Write(const void* buffer, int len);
	// Returns the length of the file.
	virtual int Length() const;
	// Return a time value for reload operations.
	virtual ID_TIME_T Timestamp() const;
	// Forces flush on files being writting to.
	virtual void ForceFlush();
	// Causes any buffered data to be written to the file.
	virtual void Flush();

	virtual int WriteString(const std::string& str);

	template<class type> inline size_t ReadBig(type& c) {
		size_t r = Read(&c, sizeof(c));
		return r;
	}
};

class idFile_Permanent : public idFile {
	friend class			idFileSystemLocal;

public:
	idFile_Permanent(const std::string &file_name, std::ios_base::openmode ios_mode);
	virtual ~idFile_Permanent();

	virtual int Read(void* buffer, int len) override;
	virtual int Write(const void* buffer, int len) override;
	virtual int Length() const override;
	virtual void ForceFlush() override;
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

/*
================================================
idFileLocal is a FileStream wrapper that automatically closes a file when the
class variable goes out of scope. Note that the pointer passed in to the constructor can be for
any type of File Stream that ultimately inherits from idFile, and that this is not actually a
SmartPointer, as it does not keep a reference count.
================================================
*/
class idFileLocal {
public:
	// Constructor that accepts and stores the file pointer.
	idFileLocal(std::shared_ptr<idFile> _file) : file(_file) {
	}

	// Destructor that will destroy (close) the file when this wrapper class goes out of scope.
	~idFileLocal();

	// Cast to a file pointer.
	operator idFile* () const {
		return file.get();
	}

	// Member access operator for treating the wrapper as if it were the file, itself.
	idFile* operator -> () const {
		return file.get();
	}

protected:
	std::shared_ptr<idFile> file;	// The managed file pointer.
};

#endif // !FILE_H

