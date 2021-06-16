#ifndef FRAMEWORK_DECLMANAGER_H_
#define FRAMEWORK_DECLMANAGER_H_

enum class declType_t {
	DECL_TABLE = 0,
	DECL_MATERIAL,
	DECL_SKIN,
	DECL_SOUND,
	DECL_ENTITYDEF,
	DECL_MODELDEF,
	DECL_FX,
	DECL_PARTICLE,
	DECL_AF,
	DECL_PDA,
	DECL_VIDEO,
	DECL_AUDIO,
	DECL_EMAIL,
	DECL_MODELEXPORT,
	DECL_MAPDEF,

	// new decl types can be added here

	DECL_MAX_TYPES = 32
};

enum class declState_t {
	DS_UNPARSED,
	DS_DEFAULTED,			// set if a parse failed due to an error, or the lack of any source
	DS_PARSED
};

class idDeclBase {
public:
	virtual ~idDeclBase() {};
	virtual const std::string& GetName() const = 0;
	virtual int GetLineNum() const = 0;
	virtual std::string GetFileName() const = 0;
	virtual void GetText(char* text) const = 0;
	virtual int GetTextLength() const = 0;
	virtual void MakeDefault() = 0;
	virtual bool SetDefaultText() = 0;
	virtual std::string DefaultDefinition() const = 0;
	virtual bool Parse(const char* text, const int textLength, bool allowBinaryVersion) = 0;
	virtual void FreeData() = 0;
};

class idDecl {
public:
	// The constructor should initialize variables such that
	// an immediate call to FreeData() does no harm.
	idDecl() { base.reset(); }
	virtual ~idDecl() {};

	// Returns the name of the decl.
	const std::string& GetName() const { return base.lock()->GetName(); }

	// Returns the line number the decl starts.
	int GetLineNum() const { return base.lock()->GetLineNum(); }

	// Returns the name of the file in which the decl is defined.
	std::string GetFileName() const { return base.lock()->GetFileName(); }

	// Frees data and makes the decl a default.
	void MakeDefault() { base.lock()->MakeDefault(); }
public:
	// Sets textSource to a default text if necessary.
	// This may be overridden to provide a default definition based on the
	// decl name. For instance materials may default to an implicit definition
	// using a texture with the same name as the decl.
	virtual bool SetDefaultText() { return base.lock()->SetDefaultText(); }

	// Each declaration type must have a default string that it is guaranteed
	// to parse acceptably. When a decl is not explicitly found, is purged, or
	// has an error while parsing, MakeDefault() will do a FreeData(), then a
	// Parse() with DefaultDefinition(). The defaultDefintion should start with
	// an open brace and end with a close brace.
	virtual std::string DefaultDefinition() const { return base.lock()->DefaultDefinition(); }

	// The manager will have already parsed past the type, name and opening brace.
	// All necessary media will be touched before return.
	// The manager will have called FreeData() before issuing a Parse().
	// The subclass can call MakeDefault() internally at any point if
	// there are parse errors.
	virtual bool Parse(const char* text, const int textLength, bool allowBinaryVersion = false) {
		return base.lock()->Parse(text, textLength, allowBinaryVersion); }

	// Frees any pointers held by the subclass. This may be called before
	// any Parse(), so the constructor must have set sane values. The decl will be
	// invalid after issuing this call, but it will always be immediately followed
	// by a Parse()
	virtual void FreeData() { base.lock()->FreeData(); }
public:
	std::weak_ptr<idDeclBase> base;
};

template< class type >
inline std::shared_ptr<idDecl> idDeclAllocator() {
	return std::make_shared<type>();
}

class idDeclManager {
public:
	virtual ~idDeclManager() {}

	virtual void Init() = 0;
	virtual void Shutdown() = 0;

	// Registers a new decl type.
	virtual void RegisterDeclType(const std::string& typeName, declType_t type, std::shared_ptr<idDecl> (*allocator)()) = 0;

	// Registers a new folder with decl files.
	virtual void RegisterDeclFolder(const std::string& folder, const std::string& extension, declType_t defaultType) = 0;

	// Returns the number of decl types.
	virtual int GetNumDeclTypes() const = 0;

	// Returns the type name for a decl type.
	virtual const std::string& GetDeclNameFromType(declType_t type) const = 0;

	// If makeDefault is true, a default decl of appropriate type will be created
	// if an explicit one isn't found. If makeDefault is false, NULL will be returned
	// if the decl wasn't explcitly defined.
	virtual const std::shared_ptr<idDecl> FindType(declType_t type, std::string name, bool makeDefault = true) = 0;

	// List and print decls.
	virtual void ListType(const idCmdArgs& args, declType_t type) = 0;
	virtual void PrintType(const idCmdArgs& args, declType_t type) = 0;
};

extern idDeclManager* declManager;

template< declType_t type >
inline void idListDecls_f(const idCmdArgs& args) {
	declManager->ListType(args, type);
}

template< declType_t type >
inline void idPrintDecls_f(const idCmdArgs& args) {
	declManager->PrintType(args, type);
}

#endif /* !FRAMEWORK_DECLMANAGER_H_*/
