#pragma hdrstop
#include "../idlib/precompiled.h"

#include "tr_local.h"
#include "Model_local.h"

idRenderModelStatic::idRenderModelStatic() {
#ifdef DEBUG_PRINT_Ctor_Dtor
	common->DPrintf("%s ctor\n", "idRenderModelStatic");
#endif // DEBUG_PRINT_Ctor_Dtor

	name = "<undefined>";
	purged = false;
	reloadable = true;
	levelLoadReferenced = false;

	color = Screen::ConsoleColor::None;
}

idRenderModelStatic::~idRenderModelStatic() {
#ifdef DEBUG_PRINT_Ctor_Dtor
	common->DPrintf("%s dtor\n", "idRenderModelStatic");
#endif // DEBUG_PRINT_Ctor_Dtor

	PurgeModel();
}

/*
================
idRenderModelStatic::IsDefaultModel
================
*/
bool idRenderModelStatic::IsDefaultModel() const {
	return defaulted;
}

void idRenderModelStatic::InitFromFile(std::string fileName) {
	bool loaded{};
	std::string extension;

	InitEmpty(fileName);

	idStr::ExtractFileExtension(name, extension);

	if (extension == "textmodel") {
		loaded = LoadTextModel(name);
		reloadable = true;
	}
	else if (extension == "bmp") {
		loaded = LoadBMPModel(name);
		reloadable = true;
	}

	if (!loaded) {
		common->Warning("Couldn't load model: '%s'", name.c_str());
		MakeDefaultModel();
		return;
	}

	// it is now available for use
	purged = false;
}

/*
========================
idRenderModelStatic::LoadBinaryModel
========================
*/
bool idRenderModelStatic::LoadBinaryModel(idFile* file) {
	if (file == nullptr) {
		return false;
	}

	return false;
}

void idRenderModelStatic::PurgeModel() {
	surfaces.clear();

	purged = true;
}

void idRenderModelStatic::LoadModel() {
	PurgeModel();
	InitFromFile(name);
}

bool idRenderModelStatic::IsLoaded() {
	return !purged;
}

/*
================
idRenderModelStatic::SetLevelLoadReferenced
================
*/
void idRenderModelStatic::SetLevelLoadReferenced(bool referenced) {
	levelLoadReferenced = referenced;
}

/*
================
idRenderModelStatic::IsLevelLoadReferenced
================
*/
bool idRenderModelStatic::IsLevelLoadReferenced() {
	return levelLoadReferenced;
}

void idRenderModelStatic::InitEmpty(const std::string fileName) {
	name = fileName;
	reloadable = false;	// if it didn't come from a file, we can't reload it
	PurgeModel();
	purged = false;
}

/*
================
idRenderModelStatic::Name
================
*/
const std::string idRenderModelStatic::Name() const {
	return name;
}

int idRenderModelStatic::NumSurfaces() const {
	return surfaces.size();
}

const ModelPixel & idRenderModelStatic::Surface(int surfaceNum) const {
	return surfaces[surfaceNum];
}

dynamicModel_t idRenderModelStatic::IsDynamicModel() const {
	// dynamic subclasses will override this
	return DM_STATIC;
}

/*
================
idRenderModelStatic::IsReloadable
================
*/
bool idRenderModelStatic::IsReloadable() const {
	return reloadable;
}

void idRenderModelStatic::MakeDefaultModel() {
	defaulted = true;

	// throw out any surfaces we already have
	PurgeModel();

	for (int i = -8 / window_font_height.GetInteger(); i < 8 / window_font_height.GetInteger(); ++i) {
		for (int j = -8 / window_font_width.GetInteger(); j < 8 / window_font_width.GetInteger(); ++j) {
			surfaces.emplace_back(Vector2(j, i), Screen::Pixel('?', Screen::ConsoleColor::White));
		}
	}
}

/*
=================
idRenderModelStatic::LoadASE
=================
*/
bool idRenderModelStatic::LoadTextModel(const std::string& fileName) {
	char* buf;
	ID_TIME_T timeStamp;

	size_t len = fileSystem->ReadFile(fileName, (void**)&buf, &timeStamp);
	if (!buf) {
		return false;
	}

	idLexer src;
	idToken token;
	idToken token2;

	src.LoadMemory(buf, len, fileName, 0);
	//src.SetFlags(DECL_LEXER_FLAGS);
	src.SkipUntilString("{");

	float x{}, y{};
	char symbol;
	Screen::ConsoleColor col{};

	while (1) {
		if (!src.ReadToken(&token)) {
			break;
		}

		if (token == "}") {
			break;
		}

		if (token == "{") {
			while (1) {
				if (!src.ReadToken(&token2)) {
					src.Warning("Unexpected end of file");
					return false;
				}

				if (token2 == "}") {
					++y;
					x = 0;
					break;
				}

				if (!src.ReadToken(&token2)) {
					src.Warning("Unexpected end of file");
					return false;
				}

				symbol = token2.at(0);

				if (!src.ReadToken(&token2)) {
					src.Warning("Unexpected end of file");
					return false;
				}

				col = static_cast<Screen::ConsoleColor>(std::stoi(token2));

				surfaces.emplace_back(Vector2(x, y), Screen::Pixel(symbol, col));

				++x;

				if (!src.ExpectTokenString("}")) {
					src.Warning("Unexpected token");
					return false;
				}
			}
		}
		else {
			src.Warning("unknown token '%s'", token.c_str());
			return false;
		}
	}

	fileSystem->FreeFile(buf);

	auto max_x = surfaces.back().origin.x + 1;
	auto max_y = surfaces.back().origin.y + 1;

	for (float i = 0; i < max_y; ++i) {
		for (float j = 0; j < max_x; ++j) {
			surfaces.at(static_cast<size_t>(i * max_x + j)).origin.x -= (max_x - 1) / 2;
			surfaces.at(static_cast<size_t>(i * max_x + j)).origin.y -= (max_y - 1) / 2;
		}
	}

	return true;
}

bool idRenderModelStatic::LoadBMPModel(const std::string& fileName) {
	BMP bmp(fileName);

	if (bmp.data.empty())
		return false;
	
	ConvertBMPToModelSurfaces(bmp);
	
	return true;
}

bool idRenderModelStatic::ConvertBMPToModelSurfaces(const BMP& bmp) {
	const char symbol{'\xDB'};
	Screen::ConsoleColor col{};

	for (int j = bmp.bmp_info_header.height - 1; j >= 0; --j) {
		for (int i = 0; i < bmp.bmp_info_header.width; ++i) {
			int cur_pixel = (j * bmp.bmp_info_header.width + i) * 3;

			if (bmp.data.at(cur_pixel + 0) > 200 &&
				bmp.data.at(cur_pixel + 1) > 200 &&
				bmp.data.at(cur_pixel + 2) > 200) {
				col = Screen::ConsoleColor::White;
			}
			else if (bmp.data.at(cur_pixel + 1) > 100 &&
				bmp.data.at(cur_pixel + 2) > 200) {
				col = Screen::ConsoleColor::Yellow;
			}
			else if (bmp.data.at(cur_pixel + 0) > 200) {
				col = Screen::ConsoleColor::Blue;
			}
			else if (bmp.data.at(cur_pixel + 1) > 200) {
				col = Screen::ConsoleColor::Green;
			}
			else if (bmp.data.at(cur_pixel + 2) > 200) {
				col = Screen::ConsoleColor::Red;
			}
			else if (bmp.data.at(cur_pixel + 0) < 50 &&
				bmp.data.at(cur_pixel + 1) < 50 &&
				bmp.data.at(cur_pixel + 2) < 50) {
				col = Screen::ConsoleColor::Black;
			}
			else if (bmp.data.at(cur_pixel + 1) > 100) {
				col = Screen::ConsoleColor::LightGreen;
			}
			else
				col = Screen::ConsoleColor::None;

			surfaces.emplace_back(Vector2(i, bmp.bmp_info_header.height - j - 1), Screen::Pixel(symbol, col));
		}
	}

	auto max_x = surfaces.back().origin.x + 1;
	auto max_y = surfaces.back().origin.y + 1;

	for (float i = 0; i < max_y; ++i) {
		for (float j = 0; j < max_x; ++j) {
			surfaces.at(static_cast<size_t>(i * max_x + j)).origin.x -= (max_x - 1) / 2;
			surfaces.at(static_cast<size_t>(i * max_x + j)).origin.y -= (max_y - 1) / 2;
		}
	}

	return true;
}

Screen::ConsoleColor idRenderModelStatic::GetColor() const {
	return color;
}

void idRenderModelStatic::SetColor(Screen::ConsoleColor col) {
	/*for (auto iter = surfaces.begin(); iter != surfaces.end(); ++iter)
	{
		iter->screenPixel.color = col;
	}*/
	color = col;
}

ModelPixel::ModelPixel(Vector2 origin, Screen::Pixel pixel) {
	this->origin = origin;
	this->screenPixel = pixel;
}
