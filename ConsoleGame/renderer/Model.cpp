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

void idRenderModelStatic::InitFromFile(std::string fileName) {
	bool loaded;

	InitEmpty(fileName);

	auto ch = '*';
	if (fileName == "asterisk") {

	}

	surfaces.emplace_back(Vector2(), Screen::Pixel(ch, Screen::ConsoleColor::White));

	loaded = true;
	reloadable = true;

	if (!loaded) {
		MakeDefaultModel();
		return;
	}

	// it is now available for use
	purged = false;
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
	// throw out any surfaces we already have
	PurgeModel();

	surfaces.emplace_back(Vector2(), Screen::Pixel('?', Screen::ConsoleColor::White));
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
