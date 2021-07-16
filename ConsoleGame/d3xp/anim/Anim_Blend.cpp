#include <precompiled.h>
#pragma hdrstop

#include "../Game_local.h"

/*
=====================
idDeclModelDef::idDeclModelDef
=====================
*/
idDeclModelDef::idDeclModelDef() {
}

/*
=====================
idDeclModelDef::~idDeclModelDef
=====================
*/
idDeclModelDef::~idDeclModelDef() {
	FreeData();
}

/*
=====================
idDeclModelDef::FreeData
=====================
*/
void idDeclModelDef::FreeData() {
}

/*
================
idDeclModelDef::DefaultDefinition
================
*/
std::string idDeclModelDef::DefaultDefinition() const {
	return "{ }";
}

/*
=====================
idDeclModelDef::ModelHandle
=====================
*/
std::weak_ptr<idRenderModel> idDeclModelDef::ModelHandle() const {
	return modelHandle;
}

/*
================
idDeclModelDef::Parse
================
*/
bool idDeclModelDef::Parse(const char* text, const int textLength, bool allowBinaryVersion) {
	std::string filename, extension;
	idLexer src;
	idToken token;
	idToken token2;

	src.LoadMemory(text, textLength, GetFileName(), GetLineNum());
	//src.SetFlags(DECL_LEXER_FLAGS);
	src.SkipUntilString("{");

	while (1) {
		if (!src.ReadToken(&token)) {
			break;
		}

		if (token == "}") {
			break;
		}

		if (token == "inherit") {
			if (!src.ReadToken(&token2)) {
				src.Warning("Unexpected end of file");
				MakeDefault();
				return false;
			}

			/*const std::shared_ptr<idDeclModelDef> copy = std::dynamic_pointer_cast<idDeclModelDef>(declManager->FindType(declType_t::DECL_MODELDEF, token2, false));
			if (!copy) {
				common->Warning("Unknown model definition '%s'", token2.c_str());
			}
			else if (copy->GetState() == declState_t::DS_DEFAULTED) {
				common->Warning("inherited model definition '%s' defaulted", token2.c_str());
				MakeDefault();
				return false;
			}
			else {
				CopyDecl(copy);
				numDefaultAnims = anims.Num();
			}*/
		}
		else if (token == "mesh") {
			if (!src.ReadToken(&token2)) {
				src.Warning("Unexpected end of file");
				MakeDefault();
				return false;
			}
			filename = token2;
			idStr::ExtractFileExtension(filename, extension);
			if (extension != MD5_MESH_EXT) {
				src.Warning("Invalid model for MD5 mesh");
				MakeDefault();
				return false;
			}
			modelHandle = renderModelManager->FindModel(filename);
			if (modelHandle.expired()) {
				src.Warning("Model '%s' not found", filename.c_str());
				MakeDefault();
				return false;
			}

			if (auto spModelHandle = modelHandle.lock()) {
				if (spModelHandle->IsDefaultModel()) {
					src.Warning("Model '%s' defaulted", filename.c_str());
					MakeDefault();
					return false;
				}
			}
		}
		else {
			src.Warning("unknown token '%s'", token.c_str());
			MakeDefault();
			return false;
		}
	}

	return true;
}