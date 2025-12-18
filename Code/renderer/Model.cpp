#include "idlib/precompiled.h"

#include "tr_local.h"
#include "Model_local.h"

idRenderModelStatic::idRenderModelStatic() {
#ifdef DEBUG_PRINT_Ctor_Dtor
  common->DPrintf("%s ctor\n", "idRenderModelStatic");
#endif  // DEBUG_PRINT_Ctor_Dtor

  name = "<undefined>";
  purged = false;
  reloadable = true;
  levelLoadReferenced = false;

  color = colorNone;
}

idRenderModelStatic::~idRenderModelStatic() {
#ifdef DEBUG_PRINT_Ctor_Dtor
  common->DPrintf("%s dtor\n", "idRenderModelStatic");
#endif  // DEBUG_PRINT_Ctor_Dtor

  PurgeModel();
}

/*
================
idRenderModelStatic::IsDefaultModel
================
*/
bool idRenderModelStatic::IsDefaultModel() const noexcept { return defaulted; }

void idRenderModelStatic::InitFromFile(const std::string& fileName) {
  bool loaded{};
  std::string extension;

  InitEmpty(fileName);

  idStr::ExtractFileExtension(name, extension);

  if (extension == "textmodel") {
    loaded = LoadTextModel(name);
    reloadable = true;
  } else if (extension == "meshstatic") {
    loaded = LoadMeshStatic(name);
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

void idRenderModelStatic::PurgeModel() noexcept {
  surfaces.clear();

  purged = true;
}

void idRenderModelStatic::LoadModel() {
  PurgeModel();
  InitFromFile(name);
}

bool idRenderModelStatic::IsLoaded() noexcept { return !purged; }

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

void idRenderModelStatic::InitEmpty(const std::string& fileName) {
  name = fileName;
  reloadable = false;  // if it didn't come from a file, we can't reload it
  PurgeModel();
  purged = false;
}

/*
================
idRenderModelStatic::Name
================
*/
const std::string& idRenderModelStatic::Name() const { return name; }

int idRenderModelStatic::NumSurfaces() const noexcept {
  return surfaces.size();
}

const ModelPixel& idRenderModelStatic::Surface(int surfaceNum) const noexcept {
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
bool idRenderModelStatic::IsReloadable() const noexcept { return reloadable; }

void idRenderModelStatic::InstantiateDynamicModel(
    const renderEntity_t* ent, const viewDef_t* view,
    std::unique_ptr<idRenderModel>& cachedModel) {
  common->Error("InstantiateDynamicModel called on static model '%s'",
                name.c_str());
  return;
}

void idRenderModelStatic::MakeDefaultModel() {
  defaulted = true;

  // throw out any surfaces we already have
  PurgeModel();

  for (int i = -8 / window_font_height.GetInteger();
       i < 8 / window_font_height.GetInteger(); ++i) {
    for (int j = -8 / window_font_width.GetInteger();
         j < 8 / window_font_width.GetInteger(); ++j) {
      surfaces.emplace_back(Vector2(j, i), Screen::Pixel('?', colorWhite));
    }
  }
  common->Warning("Model '%s' defaulted", name.c_str());
}

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
    } else {
      src.Warning("unknown token '%s'", token.c_str());
      return false;
    }
  }

  ShiftSurfaces();

  return true;
}

bool idRenderModelStatic::LoadMeshStatic(const std::string& fileName) {
  idToken token;
  idLexer parser;

  if (!parser.LoadFile(fileName)) {
    MakeDefaultModel();
    return false;
  }

  std::vector<Mesh> meshes;
  parser.ExpectTokenString("numMeshes");
  meshes.resize(parser.ParseInt());

  for (auto& mesh : meshes) {
    mesh.ParseMesh(parser);

    if (!parser.ReadToken(&token)) continue;

    if (token == "text_coords") {
      Vector2 text_coords;
      text_coords.x = parser.ParseInt();
      text_coords.y = parser.ParseInt();

      auto shader = mesh.GetShader();
      if (!shader) continue;

      mesh.UpdateSurface(nullptr, text_coords, surfaces,
                         *shader->GetStage()->image.get());

      ShiftSurfaces();
    }
  }
}

void idRenderModelStatic::ShiftSurfaces() {
  if (surfaces.empty()) return;

  int max_x = surfaces.back().origin.x + 1;
  int max_y = surfaces.back().origin.y + 1;

  for (int i = 0; i < max_y; ++i) {
    for (int j = 0; j < max_x; ++j) {
      surfaces.at(static_cast<size_t>(i * max_x + j)).origin.x -= max_x / 2;
      surfaces.at(static_cast<size_t>(i * max_x + j)).origin.y -= max_y / 2;
    }
  }
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
