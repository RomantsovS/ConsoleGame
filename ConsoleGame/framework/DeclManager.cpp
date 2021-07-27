#include <precompiled.h>
#pragma hdrstop

class idDeclType {
public:
	std::string typeName;
	declType_t type;
	std::shared_ptr<idDecl>(*allocator)();
};

class idDeclFolder {
public:
	std::string folder;
	std::string extension;
	declType_t defaultType;
};

class idDeclFile;

class idDeclLocal : public idDeclBase, public std::enable_shared_from_this<idDeclLocal> {
	friend class idDeclFile;
	friend class idDeclManagerLocal;

public:
	idDeclLocal();
	virtual ~idDeclLocal() = default;
	idDeclLocal(const idDeclLocal&) = default;
	idDeclLocal& operator=(const idDeclLocal&) = default;
	idDeclLocal(idDeclLocal&&) = default;
	idDeclLocal& operator=(idDeclLocal&&) = default;

	const std::string& GetName() const noexcept override;
	int GetLineNum() const noexcept override;
	std::string GetFileName() const override;
	void GetText(char* text) const override;
	int GetTextLength() const noexcept override;
	void MakeDefault() override;
protected:
	bool SetDefaultText() noexcept override;
	std::string DefaultDefinition() const override;
	bool Parse(const char* text, const int textLength, bool allowBinaryVersion) override;
	void FreeData() noexcept override;
protected:
	void AllocateSelf();

	// Parses the decl definition.
	// After calling parse, a decl will be guaranteed usable.
	void ParseLocal();

	// Set textSource possible with compression.
	void SetTextLocal(const char* text, const int length);
private:
	std::shared_ptr<idDecl> self;

	std::string name; // name of the decl
	std::vector<char> textSource; // decl text definition
	int textLength; // length of textSource
	std::weak_ptr<idDeclFile> sourceFile; // source file in which the decl was defined
	int sourceTextOffset;		// offset in source file to decl text
	int sourceTextLength;		// length of decl text in source file
	int sourceLine;				// this is where the actual declaration token starts
	declType_t type; // decl type
	declState_t declState; // decl state

	bool everReferenced; // set to true if the decl was ever used
	bool referencedThisLevel; // set to true when the decl is used for the current level

};

class idDeclFile : public std::enable_shared_from_this<idDeclFile> {
public:
	idDeclFile();
	idDeclFile(const std::string& fileName, declType_t defaultType);

	//void						Reload(bool force);
	int LoadAndParse();

public:
	std::string fileName;
	declType_t defaultType;

	ID_TIME_T						timestamp;
	int							checksum;
	int							fileSize;
	int							numLines;

	std::shared_ptr<idDeclLocal> decls;
};

class idDeclManagerLocal : public idDeclManager {
	friend class idDeclLocal;
public:
	idDeclManagerLocal() { linearLists.resize(static_cast<int>(declType_t::DECL_MAX_TYPES)); }

	void Init() override;
	void Shutdown() override;
	void RegisterDeclType(const std::string& typeName, declType_t type, std::shared_ptr<idDecl>(*allocator)()) override;
	void RegisterDeclFolder(const std::string& folder, const std::string& extension, declType_t defaultType) override;
	int GetNumDeclTypes() const noexcept override;
	const std::string& GetDeclNameFromType(declType_t type) const override;
	const std::shared_ptr<idDecl> FindType(declType_t type, std::string name, bool makeDefault = true) override;

	void ListType(const idCmdArgs& args, declType_t type) override;
	void PrintType(const idCmdArgs& args, declType_t type) override;

	const std::shared_ptr<idMaterial> FindMaterial(const std::string& name, bool makeDefault = true) override;
public:
	static void MakeNameCanonical(const std::string& name, std::string& result, int maxLength) noexcept;
	std::shared_ptr<idDeclLocal> FindTypeWithoutParsing(declType_t type, const std::string& name, bool makeDefault = true);

	std::shared_ptr<idDeclType> GetDeclType(int type) const noexcept { return declTypes[type]; }
private:
	std::vector<std::shared_ptr<idDeclType>> declTypes;
	std::vector<std::shared_ptr<idDeclFolder>> declFolders;

	std::vector<std::shared_ptr<idDeclFile>> loadedFiles;
	std::vector<std::vector<std::shared_ptr<idDeclLocal>>> linearLists;
private:
	static void ListDecls_f(const idCmdArgs& args);
};

idDeclManagerLocal	declManagerLocal;
idDeclManager* declManager = &declManagerLocal;

/*
====================================================================================

 idDeclFile

====================================================================================
*/

/*
================
idDeclFile::idDeclFile
================
*/
idDeclFile::idDeclFile(const std::string& fileName, declType_t defaultType) {
	this->fileName = fileName;
	this->defaultType = defaultType;
	this->timestamp = 0;
	this->checksum = 0;
	this->fileSize = 0;
	this->numLines = 0;
	this->decls = NULL;
}

/*
================
idDeclFile::idDeclFile
================
*/
idDeclFile::idDeclFile() {
	this->fileName = "<implicit file>";
	this->defaultType = declType_t::DECL_MAX_TYPES;
	this->timestamp = 0;
	this->checksum = 0;
	this->fileSize = 0;
	this->numLines = 0;
	this->decls = NULL;
}

/*
================
idDeclFile::LoadAndParse

This is used during both the initial load, and any reloads
================
*/
int c_savedMemory = 0;

int idDeclFile::LoadAndParse() {
	int			i, numTypes;
	idLexer		src;
	idToken		token;
	int			startMarker;
	std::unique_ptr<char[]> buffer;
	int			length, size;
	int			sourceLine;
	std::string		name;
	bool		reparse;

	// load the text
	common->DPrintf("...loading '%s'\n", fileName.c_str());
	buffer = fileSystem->ReadFile(fileName, length, &timestamp, true);
	if (length == -1) {
		common->FatalError("couldn't load %s", fileName.c_str());
		return 0;
	}

	if (!src.LoadMemory(buffer.get(), length, fileName)) {
		common->Error("Couldn't parse %s", fileName.c_str());
		return 0;
	}

	// mark all the defs that were from the last reload of this file
	/*for (idDeclLocal* decl = decls; decl; decl = decl->nextInFile) {
		decl->redefinedInReload = false;
	}*/

	//src.SetFlags(DECL_LEXER_FLAGS);

	//checksum = MD5_BlockChecksum(buffer, length);

	fileSize = length;

	// scan through, identifying each individual declaration
	while (1) {

		startMarker = src.GetFileOffset();
		sourceLine = src.GetLineNum();

		// parse the decl type name
		if (!src.ReadToken(&token)) {
			break;
		}

		declType_t identifiedType = declType_t::DECL_MAX_TYPES;

		// get the decl type from the type name
		numTypes = declManagerLocal.GetNumDeclTypes();
		for (i = 0; i < numTypes; i++) {
			std::shared_ptr<idDeclType> typeInfo = declManagerLocal.GetDeclType(i);
			if (typeInfo && typeInfo->typeName == token) {
				identifiedType = static_cast<declType_t>(typeInfo->type);
				break;
			}
		}

		if (i >= numTypes) {

			if (token == "{") {

				// if we ever see an open brace, we somehow missed the [type] <name> prefix
				src.Warning("Missing decl name");
				src.SkipBracedSection(false);
				continue;

			}
			else {

				if (defaultType == declType_t::DECL_MAX_TYPES) {
					src.Warning("No type");
					continue;
				}
				src.UnreadToken(&token);
				// use the default type
				identifiedType = defaultType;
			}
		}

		// now parse the name
		if (!src.ReadToken(&token)) {
			src.Warning("Type without definition at end of file");
			break;
		}

		if (token == "{") {
			// if we ever see an open brace, we somehow missed the [type] <name> prefix
			src.Warning("Missing decl name");
			src.SkipBracedSection(false);
			continue;
		}

		// FIXME: export decls are only used by the model exporter, they are skipped here for now
		if (identifiedType == declType_t::DECL_MODELEXPORT) {
			src.SkipBracedSection();
			continue;
		}

		name = token;

		// make sure there's a '{'
		if (!src.ReadToken(&token)) {
			src.Warning("Type without definition at end of file");
			break;
		}
		if (token != "{") {
			src.Warning("Expecting '{' but found '%s'", token.c_str());
			continue;
		}
		src.UnreadToken(&token);

		// now take everything until a matched closing brace
		src.SkipBracedSection();
		size = src.GetFileOffset() - startMarker;

		// look it up, possibly getting a newly created default decl
		reparse = false;
		std::shared_ptr<idDeclLocal> newDecl = declManagerLocal.FindTypeWithoutParsing(identifiedType, name, false);
		if (newDecl) {
			// update the existing copy
			if (newDecl->sourceFile.lock().get() != this /*|| newDecl->redefinedInReload*/) {
				src.Warning("%s '%s' previously defined at %s:%i", declManagerLocal.GetDeclNameFromType(identifiedType),
					name.c_str(), newDecl->sourceFile.lock()->fileName.c_str(), newDecl->sourceLine);
				continue;
			}
			if (newDecl->declState != declState_t::DS_UNPARSED) {
				reparse = true;
			}
		}
		else {
			// allow it to be created as a default, then add it to the per-file list
			newDecl = declManagerLocal.FindTypeWithoutParsing(identifiedType, name, true);
			//newDecl->nextInFile = this->decls;
			this->decls = newDecl;
		}

		//newDecl->redefinedInReload = true;

		if (!newDecl->textSource.empty()) {
			newDecl->textSource.clear();
		}

		newDecl->SetTextLocal(buffer.get() + startMarker, size);
		newDecl->sourceFile = shared_from_this();
		newDecl->sourceTextOffset = startMarker;
		newDecl->sourceTextLength = size;
		newDecl->sourceLine = sourceLine;
		newDecl->declState = declState_t::DS_UNPARSED;

		// if it is currently in use, reparse it immedaitely
		if (reparse) {
			newDecl->ParseLocal();
		}
	}

	numLines = src.GetLineNum();

	// any defs that weren't redefinedInReload should now be defaulted
	/*for (idDeclLocal* decl = decls; decl; decl = decl->nextInFile) {
		if (decl->redefinedInReload == false) {
			decl->MakeDefault();
			decl->sourceTextOffset = decl->sourceFile->fileSize;
			decl->sourceTextLength = 0;
			decl->sourceLine = decl->sourceFile->numLines;
		}
	}*/

	return checksum;
}

/*
===================
idDeclManagerLocal::Init
===================
*/
void idDeclManagerLocal::Init() {

	common->Printf("----- Initializing Decls -----\n");

	//checksum = 0;

#ifdef USE_COMPRESSED_DECLS
	SetupHuffman();
#endif

#ifdef GET_HUFFMAN_FREQUENCIES
	ClearHuffmanFrequencies();
#endif

	// decls used throughout the engine
	RegisterDeclType("material", declType_t::DECL_MATERIAL, idDeclAllocator<idMaterial>);
	RegisterDeclType("entityDef", declType_t::DECL_ENTITYDEF, idDeclAllocator<idDeclEntityDef>);

	RegisterDeclFolder("materials", ".mtr", declType_t::DECL_MATERIAL);

	// add console commands
	cmdSystem->AddCommand("listDecls", ListDecls_f, CMD_FL_SYSTEM, "lists all decls");

	cmdSystem->AddCommand("listEntityDefs", idListDecls_f<declType_t::DECL_ENTITYDEF>, CMD_FL_SYSTEM, "lists entity defs");

	cmdSystem->AddCommand("printEntityDef", idPrintDecls_f<declType_t::DECL_ENTITYDEF>, CMD_FL_SYSTEM, "prints an entity def");

	common->Printf("------------------------------\n");
}

/*
===================
idDeclManagerLocal::Shutdown
===================
*/
void idDeclManagerLocal::Shutdown() {
	// free decls
	for (size_t i = 0; i < static_cast<size_t>(declType_t::DECL_MAX_TYPES); i++) {
		for (size_t j = 0; j < linearLists[i].size(); j++) {
			std::shared_ptr<idDeclLocal> decl = linearLists[i][j];
			if (decl->self) {
				decl->self->FreeData();
				decl->self = nullptr;;
			}
			if (!decl->textSource.empty()) {
				decl->textSource.clear();
			}
			decl = nullptr;
		}
		linearLists[i].clear();
	}

	// free decl files
	loadedFiles.clear();

	// free the decl types and folders
	declTypes.clear();
	declFolders.clear();

#ifdef USE_COMPRESSED_DECLS
	ShutdownHuffman();
#endif
}

/*
===================
idDeclManagerLocal::RegisterDeclType
===================
*/
void idDeclManagerLocal::RegisterDeclType(const std::string& typeName, declType_t type, std::shared_ptr<idDecl>(*allocator)()) {
	if (static_cast<size_t>(type) < declTypes.size() && declTypes.at(static_cast<int>(type))) {
		common->Warning("idDeclManager::RegisterDeclType: type '%s' already exists", typeName);
		return;
	}

	std::shared_ptr<idDeclType> declType = std::make_shared<idDeclType>();
	declType->typeName = typeName;
	declType->type = type;
	declType->allocator = allocator;

	if (static_cast<size_t>(type) + 1 > declTypes.size()) {
		declTypes.resize(static_cast<int>(type) + 1);
	}
	declTypes.at(static_cast<size_t>(type)) = declType;
}

/*
===================
idDeclManagerLocal::RegisterDeclFolder
===================
*/
void idDeclManagerLocal::RegisterDeclFolder(const std::string& folder, const std::string& extension, declType_t defaultType) {
	size_t i, j;
	std::string fileName;
	std::shared_ptr<idDeclFolder> declFolder;
	std::shared_ptr<idDeclFile> df;

	// check whether this folder / extension combination already exists
	for (i = 0; i < declFolders.size(); i++) {
		if (declFolders[i]->folder == folder && declFolders[i]->extension == extension) {
			break;
		}
	}
	if (i < declFolders.size()) {
		declFolder = declFolders[i];
	}
	else {
		declFolder = std::make_shared<idDeclFolder>();
		declFolder->folder = folder;
		declFolder->extension = extension;
		declFolder->defaultType = defaultType;
		declFolders.push_back(declFolder);
	}

	// scan for decl files
	std::shared_ptr<idFileList> fileList = fileSystem->ListFiles(declFolder->folder, declFolder->extension, true);

	// load and parse decl files
	for (i = 0; i < fileList->GetNumFiles(); i++) {
		fileName = declFolder->folder + "/" + fileList->GetFile(i);

		// check whether this file has already been loaded
		for (j = 0; j < loadedFiles.size(); j++) {
			if (loadedFiles[j]->fileName == fileName) {
				break;
			}
		}
		if (j < loadedFiles.size()) {
			df = loadedFiles[j];
		}
		else {
			df = std::make_shared<idDeclFile>(fileName, defaultType);
			loadedFiles.push_back(df);
		}
		df->LoadAndParse();
	}

	fileSystem->FreeFileList(fileList);
}

/*
===================
idDeclManagerLocal::GetNumDeclTypes
===================
*/
int idDeclManagerLocal::GetNumDeclTypes() const noexcept {
	return declTypes.size();
}

/*
===================
idDeclManagerLocal::GetDeclNameFromType
===================
*/
const std::string& idDeclManagerLocal::GetDeclNameFromType(declType_t type) const {
	size_t typeIndex = static_cast<size_t>(type);

	if (typeIndex < 0 || typeIndex >= declTypes.size() || !declTypes[typeIndex]) {
		common->FatalError("idDeclManager::GetDeclNameFromType: bad type: %i", typeIndex);
	}
	return declTypes[typeIndex]->typeName;
}

/*
=================
idDeclManagerLocal::FindType

External users will always cause the decl to be parsed before returning
=================
*/
const std::shared_ptr<idDecl> idDeclManagerLocal::FindType(declType_t type, std::string name, bool makeDefault) {
	//idScopedCriticalSection cs(mutex);

	if (name.empty()) {
		name = "_emptyName";
		common->Warning( "idDeclManager::FindType: empty %s name", GetDeclType( static_cast<int>(type) )->typeName.c_str() );
	}

	std::shared_ptr<idDeclLocal> decl = FindTypeWithoutParsing(type, name, makeDefault);
	if (!decl) {
		return nullptr;
	}

	decl->AllocateSelf();

	// if it hasn't been parsed yet, parse it now
	if (decl->declState == declState_t::DS_UNPARSED) {
		decl->ParseLocal();
	}

	// mark it as referenced
	decl->referencedThisLevel = true;
	decl->everReferenced = true;
	/*if (insideLevelLoad) {
		decl->parsedOutsideLevelLoad = false;
	}*/

	return decl->self;
}

/*
===================
idDeclManagerLocal::ListType

list*
Lists decls currently referenced

list* ever
Lists decls that have been referenced at least once since app launched

list* all
Lists every decl declared, even if it hasn't been referenced or parsed

FIXME: alphabetized, wildcards?
===================
*/
void idDeclManagerLocal::ListType(const idCmdArgs& args, declType_t type) {
	bool all, ever;

	if (args.Argv(1) == "all") {
		all = true;
	}
	else {
		all = false;
	}
	if (args.Argv(1) == "ever") {
		ever = true;
	}
	else {
		ever = false;
	}

	common->Printf("--------------------\n");
	int printed = 0;
	int count = 0;/* linearLists[(int)type].Num();
	for (int i = 0; i < count; i++) {
		idDeclLocal* decl = linearLists[(int)type][i];

		if (!all && decl->declState == declState_t::DS_UNPARSED) {
			continue;
		}

		if (!all && !ever && !decl->referencedThisLevel) {
			continue;
		}

		if (decl->referencedThisLevel) {
			common->Printf("*");
		}
		else if (decl->everReferenced) {
			common->Printf(".");
		}
		else {
			common->Printf(" ");
		}
		if (decl->declState == declState_t::DS_DEFAULTED) {
			common->Printf("D");
		}
		else {
			common->Printf(" ");
		}
		common->Printf("%4i: ", decl->index);
		printed++;
		if (decl->declState == declState_t::DS_UNPARSED) {
			// doesn't have any type specific data yet
			common->Printf("%s\n", decl->GetName());
		}
		else {
			decl->self->List();
		}
	}*/

	common->Printf("--------------------\n");
	common->Printf("%i of %i %s\n", printed, count, declTypes[static_cast<int>(type)]->typeName.c_str());
}

/*
===================
idDeclManagerLocal::PrintType
===================
*/
void idDeclManagerLocal::PrintType(const idCmdArgs& args, declType_t type) {
	// individual decl types may use additional command parameters
	if (args.Argc() < 2) {
		common->Printf("USAGE: Print<decl type> <decl name> [type specific parms]\n");
		return;
	}

	// look it up, skipping the public path so it won't parse or reference
	/*idDeclLocal* decl = FindTypeWithoutParsing(type, args.Argv(1), false);
	if (!decl) {
		common->Printf("%s '%s' not found.\n", declTypes[type]->typeName.c_str(), args.Argv(1));
		return;
	}

	// print information common to all decls
	common->Printf("%s %s:\n", declTypes[type]->typeName.c_str(), decl->name.c_str());
	common->Printf("source: %s:%i\n", decl->sourceFile->fileName.c_str(), decl->sourceLine);
	common->Printf("----------\n");
	if (decl->textSource != NULL) {
		char* declText = (char*)_alloca(decl->textLength + 1);
		decl->GetText(declText);
		common->Printf("%s\n", declText);
	}
	else {
		common->Printf("NO SOURCE\n");
	}
	common->Printf("----------\n");
	switch (decl->declState) {
	case DS_UNPARSED:
		common->Printf("Unparsed.\n");
		break;
	case DS_DEFAULTED:
		common->Printf("<DEFAULTED>\n");
		break;
	case DS_PARSED:
		common->Printf("Parsed.\n");
		break;
	}

	if (decl->referencedThisLevel) {
		common->Printf("Currently referenced this level.\n");
	}
	else if (decl->everReferenced) {
		common->Printf("Referenced in a previous level.\n");
	}
	else {
		common->Printf("Never referenced.\n");
	}

	// allow type-specific data to be printed
	if (decl->self != NULL) {
		decl->self->Print();
	}*/
}

const std::shared_ptr<idMaterial> idDeclManagerLocal::FindMaterial(const std::string& name, bool makeDefault) {
	return std::static_pointer_cast<idMaterial>(FindType(declType_t::DECL_MATERIAL, name, makeDefault));
}

/*
===================
idDeclManagerLocal::MakeNameCanonical
===================
*/
void idDeclManagerLocal::MakeNameCanonical(const std::string& name, std::string& result, int maxLength) noexcept {
	int i, lastDot;

	lastDot = -1;
	for (i = 0; i < maxLength && name[i] != '\0'; i++) {
		int c = name[i];
		if (c == '\\') {
			result[i] = '/';
		}
		else if (c == '.') {
			lastDot = i;
			result[i] = c;
		}
		else {
			result[i] = std::tolower(c);
		}
	}
	if (lastDot != -1) {
		result[lastDot] = '\0';
	}
	else {
		result[i] = '\0';
	}
}

/*
================
idDeclManagerLocal::ListDecls_f
================
*/
void idDeclManagerLocal::ListDecls_f(const idCmdArgs& args) {
	int		totalDecls = 0;
	int		totalText = 0;
	int		totalStructs = 0;

	for (size_t i = 0; i < declManagerLocal.declTypes.size(); i++) {
		//int size, num;

		if (declManagerLocal.declTypes[i] == NULL) {
			continue;
		}

		/*num = declManagerLocal.linearLists[i].Num();
		totalDecls += num;

		size = 0;
		for (j = 0; j < num; j++) {
			size += declManagerLocal.linearLists[i][j]->Size();
			if (declManagerLocal.linearLists[i][j]->self != NULL) {
				size += declManagerLocal.linearLists[i][j]->self->Size();
			}
		}
		totalStructs += size;*/

		//common->Printf("%4ik %4i %s\n", size >> 10, num, declManagerLocal.declTypes[i]->typeName.c_str());
	}

	/*for (i = 0; i < declManagerLocal.loadedFiles.Num(); i++) {
		idDeclFile* df = declManagerLocal.loadedFiles[i];
		totalText += df->fileSize;
	}*/

	//common->Printf("%i total decls is %i decl files\n", totalDecls, declManagerLocal.loadedFiles.Num());
	common->Printf("%iKB in text, %iKB in structures\n", totalText >> 10, totalStructs >> 10);
}

/*
===================
idDeclManagerLocal::FindTypeWithoutParsing

This finds or creats the decl, but does not cause a parse.  This is only used internally.
===================
*/
std::shared_ptr<idDeclLocal> idDeclManagerLocal::FindTypeWithoutParsing(declType_t type, const std::string& name, bool makeDefault) {
	int typeIndex = static_cast<int>(type);

	if (typeIndex < 0 || static_cast<size_t>(typeIndex) >= declTypes.size() || !declTypes[typeIndex] ||
		typeIndex >= static_cast<int>(declType_t::DECL_MAX_TYPES)) {
		common->FatalError("idDeclManager::FindTypeWithoutParsing: bad type: %i", typeIndex);
		return NULL;
	}

	std::string canonicalName;
	canonicalName.resize(MAX_STRING_CHARS);

	MakeNameCanonical(name, canonicalName, canonicalName.size());

	// see if it already exists
	std::vector<std::shared_ptr<idDeclLocal>> list = linearLists.at(typeIndex);
	auto iter = std::find_if(list.begin(), list.end(),
		[&canonicalName](std::shared_ptr<idDeclLocal> decl) noexcept { return decl->name == canonicalName; });
	if (iter != list.end()) {
		// only print these when decl_show is set to 2, because it can be a lot of clutter
		/*if (decl_show.GetInteger() > 1) {
			MediaPrint("referencing %s %s\n", declTypes[type]->typeName.c_str(), name);
		}*/
		return *iter;
	}

	if (!makeDefault) {
		return nullptr;
	}

	std::shared_ptr<idDeclLocal> decl = std::make_shared<idDeclLocal>();

	decl->self = NULL;
	decl->name = canonicalName;
	decl->type = type;
	decl->declState = declState_t::DS_UNPARSED;
	decl->textLength = 0;
	//decl->sourceFile = &implicitDecls;
	decl->referencedThisLevel = false;
	decl->everReferenced = false;
	//decl->parsedOutsideLevelLoad = !insideLevelLoad;

	// add it to the linear list and hash table
	//decl->index = linearLists[typeIndex].size();
	linearLists.at(typeIndex).push_back(decl);

	return decl;
}

/*
====================================================================================

	idDeclLocal

====================================================================================
*/

/*
=================
idDeclLocal::idDeclLocal
=================
*/
idDeclLocal::idDeclLocal() {
	name = "unnamed";
	textLength = 0;
	//compressedLength = 0;
	sourceFile.reset();
	sourceTextOffset = 0;
	sourceTextLength = 0;
	sourceLine = 0;
	//checksum = 0;
	type = declType_t::DECL_ENTITYDEF;
	//index = 0;
	declState = declState_t::DS_UNPARSED;
	//parsedOutsideLevelLoad = false;
	referencedThisLevel = false;
	everReferenced = false;
	//redefinedInReload = false;
	//nextInFile = NULL;
}

/*
=================
idDeclLocal::GetName
=================
*/
const std::string& idDeclLocal::GetName() const noexcept {
	return name;
}

/*
=================
idDeclLocal::GetLineNum
=================
*/
int idDeclLocal::GetLineNum() const noexcept {
	return sourceLine;
}

/*
=================
idDeclLocal::GetFileName
=================
*/
std::string idDeclLocal::GetFileName() const {
	return (sourceFile.lock()) ? sourceFile.lock()->fileName : "*invalid*";
}

/*
=================
idDeclLocal::GetText
=================
*/
void idDeclLocal::GetText(char* text) const {
#ifdef USE_COMPRESSED_DECLS
	HuffmanDecompressText(text, textLength, (byte*)textSource, compressedLength);
#else
	//memcpy(text, textSource, textLength + 1);
	std::copy(textSource.begin(), textSource.end(), text);
#endif
}

/*
=================
idDeclLocal::GetTextLength
=================
*/
int idDeclLocal::GetTextLength() const noexcept {
	return textLength;
}

/*
=================
idDeclLocal::SetTextLocal
=================
*/
void idDeclLocal::SetTextLocal(const char* text, const int length) {

	textSource.clear();

	//checksum = MD5_BlockChecksum(text, length);

#ifdef GET_HUFFMAN_FREQUENCIES
	for (int i = 0; i < length; i++) {
		huffmanFrequencies[((const unsigned char*)text)[i]]++;
	}
#endif

#ifdef USE_COMPRESSED_DECLS
	int maxBytesPerCode = (maxHuffmanBits + 7) >> 3;
	byte* compressed = (byte*)_alloca(length * maxBytesPerCode);
	compressedLength = HuffmanCompressText(text, length, compressed, length * maxBytesPerCode);
	textSource = (char*)Mem_Alloc(compressedLength, TAG_DECLTEXT);
	memcpy(textSource, compressed, compressedLength);
#else
	//compressedLength = length;
	textSource.resize(length + 1);
	std::copy(text, text + length, textSource.begin());
#endif
	textLength = length;
}

/*
=================
idDeclLocal::MakeDefault
=================
*/
void idDeclLocal::MakeDefault() {
	static int recursionLevel;

	//declManagerLocal.MediaPrint("DEFAULTED\n");
	declState = declState_t::DS_DEFAULTED;

	AllocateSelf();

	std::string defaultText = self->DefaultDefinition();

	// a parse error inside a DefaultDefinition() string could
	// cause an infinite loop, but normal default definitions could
	// still reference other default definitions, so we can't
	// just dump out on the first recursion
	if (++recursionLevel > 100) {
		common->FatalError("idDecl::MakeDefault: bad DefaultDefinition(): %s", defaultText.c_str());
	}

	// always free data before parsing
	self->FreeData();

	// parse
	self->Parse(defaultText.c_str(), defaultText.size(), false);

	// we could still eventually hit the recursion if we have enough Error() calls inside Parse...
	--recursionLevel;
}

/*
=================
idDeclLocal::SetDefaultText
=================
*/
bool idDeclLocal::SetDefaultText() noexcept {
	return false;
}

/*
=================
idDeclLocal::DefaultDefinition
=================
*/
std::string idDeclLocal::DefaultDefinition() const {
	return "{ }";
}

/*
=================
idDeclLocal::Parse
=================
*/
bool idDeclLocal::Parse(const char* text, const int textLength, bool allowBinaryVersion) {
	idLexer src;

	src.LoadMemory(text, textLength, GetFileName(), GetLineNum());
	//src.SetFlags(DECL_LEXER_FLAGS);
	src.SkipUntilString("{");
	src.SkipBracedSection(false);
	return true;
}

/*
=================
idDeclLocal::FreeData
=================
*/
void idDeclLocal::FreeData() noexcept {
}

/*
=================
idDeclLocal::AllocateSelf
=================
*/
void idDeclLocal::AllocateSelf() {
	if (!self) {
		self = declManagerLocal.GetDeclType(static_cast<int>(type))->allocator();
		self->base = shared_from_this();
	}
}

/*
=================
idDeclLocal::ParseLocal
=================
*/
void idDeclLocal::ParseLocal() {
	bool generatedDefaultText = false;

	AllocateSelf();

	// always free data before parsing
	self->FreeData();

	//declManagerLocal.MediaPrint("parsing %s %s\n", declManagerLocal.declTypes[type]->typeName.c_str(), name.c_str());

	// if no text source try to generate default text
	if (textSource.empty()) {
		generatedDefaultText = self->SetDefaultText();
	}

	// indent for DEFAULTED or media file references
	//declManagerLocal.indent++;

	// no text immediately causes a MakeDefault()
	if (textSource.empty()) {
		MakeDefault();
		//declManagerLocal.indent--;
		return;
	}

	declState = declState_t::DS_PARSED;

	// parse
	std::vector<char> declText(GetTextLength() + 1);

	GetText(declText.data());
	self->Parse(declText.data(), GetTextLength(), true);

	// free generated text
	if (generatedDefaultText) {
		textSource.clear();
		textLength = 0;
	}

	//declManagerLocal.indent--;
}
