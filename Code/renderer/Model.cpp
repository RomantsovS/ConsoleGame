#include "idlib/precompiled.h"


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

	color = colorNone;
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
bool idRenderModelStatic::IsDefaultModel() const noexcept {
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
bool idRenderModelStatic::LoadBinaryModel(idFile* file) noexcept {
	if (file == nullptr) {
		return false;
	}

	return false;
}

void idRenderModelStatic::PurgeModel() noexcept {
	surfaces.clear();

	purged = true;
}

void idRenderModelStatic::LoadModel() {
	PurgeModel();
	InitFromFile(name);
}

bool idRenderModelStatic::IsLoaded() noexcept {
	return !purged;
}

/*
================
idRenderModelStatic::SetLevelLoadReferenced
================
*/
void idRenderModelStatic::SetLevelLoadReferenced(bool referenced) noexcept {
	levelLoadReferenced = referenced;
}

/*
================
idRenderModelStatic::IsLevelLoadReferenced
================
*/
bool idRenderModelStatic::IsLevelLoadReferenced() noexcept {
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
const std::string& idRenderModelStatic::Name() const {
	return name;
}

int idRenderModelStatic::NumSurfaces() const noexcept {
	return surfaces.size();
}

const ModelPixel & idRenderModelStatic::Surface(int surfaceNum) const noexcept {
	return surfaces[surfaceNum];
}

dynamicModel_t idRenderModelStatic::IsDynamicModel() const noexcept {
	// dynamic subclasses will override this
	return DM_STATIC;
}

/*
================
idRenderModelStatic::IsReloadable
================
*/
bool idRenderModelStatic::IsReloadable() const noexcept {
	return reloadable;
}

void idRenderModelStatic::MakeDefaultModel() {
	defaulted = true;

	// throw out any surfaces we already have
	PurgeModel();

	for (int i = -8 / window_font_height.GetInteger(); i < 8 / window_font_height.GetInteger(); ++i) {
		for (int j = -8 / window_font_width.GetInteger(); j < 8 / window_font_width.GetInteger(); ++j) {
			surfaces.emplace_back(Vector2(j, i), Screen::Pixel('?', colorWhite));
		}
	}
}

/*
=================
idRenderModelStatic::LoadASE
=================
*/
bool idRenderModelStatic::LoadTextModel(const std::string& fileName) {
	ID_TIME_T timeStamp;
	int len;

	auto buf = fileSystem->ReadFile(fileName, len, &timeStamp, true);
	if (!buf) {
		return false;
	}

	idLexer src;
	idToken token;
	idToken token2;

	src.LoadMemory(buf.get(), len, fileName, 0);
	//src.SetFlags(DECL_LEXER_FLAGS);
	src.SkipUntilString("{");

	float x{}, y{};
	char symbol;
	int col{};

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

				col = std::stoi(token2);

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
	
	ConvertBMPToModelSurfaces(bmp, surfaces);
	
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

bool ConvertBMPToModelSurfaces(const BMP& bmp, std::vector<ModelPixel>& surfaces) {
	const char symbol{'\xDB'};
	int col{};

	for (int j = bmp.bmp_info_header.height - 1; j >= 0; --j) {
		for (int i = 0; i < bmp.bmp_info_header.width; ++i) {
			int cur_pixel = (j * bmp.bmp_info_header.width + i) * 3;

			if (bmp.data.at(cur_pixel + 0) > 200 && bmp.data.at(cur_pixel + 1) > 200 &&
				bmp.data.at(cur_pixel + 2) > 200) {
				col = colorWhite;
			}
			else if (bmp.data.at(cur_pixel + 1) > 100 && bmp.data.at(cur_pixel + 2) > 200) {
				col = colorYellow;
			}
			else if (bmp.data.at(cur_pixel + 0) > 200 && bmp.data.at(cur_pixel + 1) > 200 &&
				bmp.data.at(cur_pixel + 3) > 100) {
				col = colorLightCyan;
			}
			else if (bmp.data.at(cur_pixel + 0) > 200) {
				col = colorBlue;
			}
			else if (bmp.data.at(cur_pixel + 1) > 200) {
				col = colorGreen;
			}
			else if (bmp.data.at(cur_pixel + 2) > 200) {
				col = colorLightRed;
			}
			else if (bmp.data.at(cur_pixel + 0) < 50 && bmp.data.at(cur_pixel + 1) < 50 &&
				bmp.data.at(cur_pixel + 2) < 50) {
				col = colorBlack;
			}
			else if (bmp.data.at(cur_pixel + 0) > 100 && bmp.data.at(cur_pixel + 1) > 100 &&
				bmp.data.at(cur_pixel + 2) < 50) {
				col = colorLightGray;
			}
			else if (bmp.data.at(cur_pixel + 0) < 100 && bmp.data.at(cur_pixel + 1) < 100 &&
				bmp.data.at(cur_pixel + 2) < 100) {
				col = colorCyan;
			}
			else if (bmp.data.at(cur_pixel + 1) > 100) {
				col = colorLightGreen;
			}
			else if (bmp.data.at(cur_pixel + 2) > 50) {
				col = colorRed;
			}
			else
				col = colorWhite;

			surfaces.emplace_back(Vector2(i, bmp.bmp_info_header.height - j - 1), Screen::Pixel(symbol, col));
		}
	}

	return true;
}

Screen::color_type idRenderModelStatic::GetColor() const noexcept {
	return color;
}

void idRenderModelStatic::SetColor(Screen::color_type col) noexcept {
	/*for (auto iter = surfaces.begin(); iter != surfaces.end(); ++iter)
	{
		iter->screenPixel.color = col;
	}*/
	color = col;
}

ModelPixel::ModelPixel(Vector2 origin, Screen::Pixel pixel) noexcept {
	this->origin = origin;
	this->screenPixel = pixel;
}
