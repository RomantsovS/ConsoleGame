#pragma hdrstop
#include "../idlib/precompiled.h"

#include "tr_local.h"

/*
=============
idMaterial::CommonInit
=============
*/
void idMaterial::CommonInit() {
}

/*
=============
idMaterial::idMaterial
=============
*/
idMaterial::idMaterial() {
	CommonInit();
}

/*
=============
idMaterial::~idMaterial
=============
*/
idMaterial::~idMaterial() {
}

/*
=================
idMaterial::ParseStage

An open brace has been parsed


{
	if <expression>
	map <imageprogram>
	"nearest" "linear" "clamp" "zeroclamp" "uncompressed" "highquality" "nopicmip"
	scroll, scale, rotate
}

=================
*/
void idMaterial::ParseStage(idLexer& src) {
	idToken token, token2;
	std::string imageName;
	stage = std::make_unique<textureStage_t>();

	while (1) {
		if (!src.ReadToken(&token)) {
			break;
		}

		// the close brace for the entire material ends the draw block
		if (token == "}") {
			break;
		}

		if (token == "map") {
			if (!src.ReadToken(&token2)) {
				src.Warning("Unexpected end of file");
				MakeDefault();
				return;
			}
			imageName = token2;
			continue;
		}

		common->Warning("unknown token '%s' in material '%s'", token.c_str(), GetName());
		//SetMaterialFlag(MF_DEFAULTED);
		return;
	}

	// now load the image with all the parms we parsed
	if (!imageName.empty()) {
		stage->image = globalImages->ImageFromFile(imageName);
		if (!stage->image) {
			stage->image = globalImages->defaultImage;
		}
	}
	else {
		common->Warning("material '%s' had stage with no image", GetName());
		stage->image = globalImages->defaultImage;
	}
}

/*
=================
idMaterial::ParseMaterial

The current text pointer is at the explicit text definition of the
Parse it into the global material variable. Later functions will optimize it.

If there is any error during parsing, defaultShader will be set.
=================
*/
void idMaterial::ParseMaterial(idLexer& src) {
	idToken token;

	while (1) {
		if (!src.ReadToken(&token)) {
			break;
		}

		// end of material definition
		if (token == "}") {
			break;
		}
		else if (token == "{") {
			// create the new stage
			ParseStage(src);
			continue;
		}
		else {
			common->Warning("unknown general material parameter '%s' in '%s'", token.c_str(), GetName());
			//SetMaterialFlag(MF_DEFAULTED);
			return;
		}
	}
}

/*
=========================
idMaterial::Parse

Parses the current material definition and finds all necessary images.
=========================
*/
bool idMaterial::Parse(const char* text, const int textLength, bool allowBinaryVersion) {
	idLexer	src;
	idToken	token;

	src.LoadMemory(text, textLength, GetFileName(), GetLineNum());
	//src.SetFlags(DECL_LEXER_FLAGS);
	src.SkipUntilString("{");

	// reset to the unparsed state
	CommonInit();

	// parse it
	ParseMaterial(src);

	return true;
}

/*
===================
idMaterial::SetDefaultText
===================
*/
bool idMaterial::SetDefaultText() {
	// if there exists an image with the same name
	if (1) {
		return true;
	}
	else {
		return false;
	}
}

/*
===================
idMaterial::DefaultDefinition
===================
*/
std::string idMaterial::DefaultDefinition() const {
	return "{ }";
}