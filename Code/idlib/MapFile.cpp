#include "precompiled.h"

/*
=================
idMapBrush::Parse
=================
*/
std::shared_ptr<idMapBrush> idMapBrush::Parse(idLexer& src, const Vector2& origin, bool newFormat, float version) {
	//idVec3 planepts[3];
	idToken token;
	std::vector<std::shared_ptr<idMapBrushSide>> sides;
	idDict epairs;

	if (!src.ExpectTokenString("{")) {
		return NULL;
	}

	do {
		if (!src.ReadToken(&token)) {
			src.Error("idMapBrush::Parse: unexpected EOF");
			sides.clear();
			return nullptr;
		}
		if (token == "}") {
			break;
		}

		// here we may have to jump over brush epairs ( only used in editor )
		do {
			// if token is a brace
			if (token == "(") {
				break;
			}
			// the token should be a key string for a key/value pair
			if (token.type != TT_STRING) {
				src.Error("idMapBrush::Parse: unexpected %s, expected ( or epair key string", token.c_str());
				sides.clear();
				return NULL;
			}

			idToken key = token;

			if (!src.ReadTokenOnLine(&token) || token.type != TT_STRING) {
				src.Error("idMapBrush::Parse: expected epair value string not found");
				sides.clear();
				return NULL;
			}

			epairs.Set(key, token);

			// try to read the next key
			if (!src.ReadToken(&token)) {
				src.Error("idMapBrush::Parse: unexpected EOF");
				sides.clear();
				return NULL;
			}
		} while (1);

		src.UnreadToken(&token);

		std::shared_ptr<idMapBrushSide> side = std::make_shared<idMapBrushSide>();
		sides.push_back(side);

		if (newFormat) {
			if (!src.Parse1DMatrix(4, gsl::span<float>{ side->points.at(0).ToFloatPtr(), 4 })) {
				src.Error("idMapBrush::Parse: unable to read brush side definition");
				sides.clear();
				return nullptr;
			}
		}

		side->origin = origin;
	} while (1);

	if (!src.ExpectTokenString("}")) {
		sides.clear();
		return NULL;
	}

	std::shared_ptr<idMapBrush> brush = std::make_shared<idMapBrush>();
	for (size_t i = 0; i < sides.size(); i++) {
		brush->AddSide(sides.at(i));
	}

	brush->epairs = epairs;

	return brush;
}

/*
================
idMapEntity::Parse
================
*/
std::shared_ptr<idMapEntity> idMapEntity::Parse(idLexer& src, bool worldSpawn, float version) {
	idToken	token;
	bool worldent;
	Vector2 origin;
	double v1, v2;

	if (!src.ReadToken(&token)) {
		return nullptr;
	}

	if (token != "{") {
		src.Error("idMapEntity::Parse: { not found, found %s", token.c_str());
		return nullptr;
	}

	std::shared_ptr<idMapEntity> mapEnt = std::make_shared<idMapEntity>();

	if (worldSpawn) {
		mapEnt->primitives.reserve(1024);
	}

	origin.Zero();
	worldent = false;
	do {
		if (!src.ReadToken(&token)) {
			src.Error("idMapEntity::Parse: EOF without closing brace");
			return nullptr;
		}
		if (token == "}") {
			break;
		}

		if (token == "{") {
			// parse a brush or patch
			if (!src.ReadToken(&token)) {
				src.Error("idMapEntity::Parse: unexpected EOF");
				return nullptr;
			}

			if (worldent) {
				origin.Zero();
			}

			// if is it a brush: brush, brushDef, brushDef2, brushDef3
			if (token.compare(0, 5, "brush") == 0) {
				std::shared_ptr<idMapBrush> mapBrush = idMapBrush::Parse(src, origin, true, version);
				if (!mapBrush) {
					return nullptr;
				}
				mapEnt->AddPrimitive(mapBrush);
			}
			// if is it a patch: patchDef2, patchDef3
			/*else if (token.Icmpn("patch", 5) == 0) {
				mapPatch = idMapPatch::Parse(src, origin, !token.Icmp("patchDef3"), version);
				if (!mapPatch) {
					return NULL;
				}
				mapEnt->AddPrimitive(mapPatch);
			}
			// assume it's a brush in Q3 or older style
			else {
				src.UnreadToken(&token);
				mapBrush = idMapBrush::ParseQ3(src, origin);
				if (!mapBrush) {
					return NULL;
				}
				mapEnt->AddPrimitive(mapBrush);
			}*/
		}
		else {
			std::string key, value;

			// parse a key / value pair
			key = token;
			src.ReadTokenOnLine(&token);
			value = token;

			// strip trailing spaces that sometimes get accidentally
			// added in the editor
			idStr::StripTrailingWhitespace(value);
			idStr::StripTrailingWhitespace(key);

			mapEnt->epairs.Set(key, value);

			if (key ==  "origin") {
				// scanf into doubles, then assign, so it is idVec size independent
				v1 = v2 = 0;
				sscanf_s(value.c_str(), "%lf %lf", &v1, &v2);
				origin.x = static_cast<float>(v1);
				origin.y = static_cast<float>(v2);
			}
			else if (key == "classname" && value == "worldspawn") {
				worldent = true;
			}
		}
	} while (1);

	return mapEnt;
}

/*
===============
idMapEntity::RemovePrimitiveData
===============
*/
void idMapEntity::RemovePrimitiveData() noexcept {
	primitives.clear();
}

/*
===============
idMapFile::Parse
===============
*/
bool idMapFile::Parse(const std::string& filename, bool ignoreRegion, bool osPath) {
	// no string concatenation for epairs and allow path names for materials
	idLexer src/*(LEXFL_NOSTRINGCONCAT | LEXFL_NOSTRINGESCAPECHARS | LEXFL_ALLOWPATHNAMES)*/;
	idToken token;
	std::string fullName;
	//int i, j, k;

	name = filename;
	idStr::StripFileExtension(name);
	fullName = name;
	//hasPrimitiveData = false;

	if (!src.IsLoaded()) {
		// now try a .map file
		idStr::SetFileExtension(fullName, "map");
		src.LoadFile(fullName, osPath);
		if (!src.IsLoaded()) {
			// didn't get anything at all
			return false;
		}
	}

	version = OLD_MAP_VERSION;
	fileTime = src.GetFileTime();
	entities.clear();

	if (src.CheckTokenString("Version")) {
		src.ReadTokenOnLine(&token);
		version = token.GetFloatValue();
	}

	while (1) {
		std::shared_ptr<idMapEntity> mapEnt = idMapEntity::Parse(src, (entities.size() == 0), version);
		if (!mapEnt) {
			break;
		}
		entities.push_back(mapEnt);
	}

	//hasPrimitiveData = true;
	return true;
}

/*
===============
idMapFile::RemovePrimitiveData
===============
*/
void idMapFile::RemovePrimitiveData() noexcept {
	for (size_t i = 0; i < entities.size(); i++) {
		std::shared_ptr<idMapEntity> ent = entities[i];
		ent->RemovePrimitiveData();
	}
	//hasPrimitiveData = false;
}

/*
===============
idMapFile::NeedsReload
===============
*/
bool idMapFile::NeedsReload() {
	if (!name.empty()) {
		ID_TIME_T time = FILE_NOT_FOUND_TIMESTAMP;
		int len;
		if (fileSystem->ReadFile(name, len, &time) > 0) {
			return (time > fileTime);
		}
	}
	return true;
}
