#include "idlib/precompiled.h"

#include "tr_local.h"
#include "Model_local.h"

void Mesh::ParseMesh(idLexer& parser) {
  idToken token;

  parser.ExpectTokenString("{");

  parser.ExpectTokenString("shader");

  parser.ReadToken(&token);
  std::string shaderName = token;

  shader = declManager->FindMaterial(shaderName);

  parser.ExpectTokenString("size");
  size.x = parser.ParseInt();
  size.y = parser.ParseInt();

  parser.ExpectTokenString("anim_stages");
  anim_stages.resize(parser.ParseInt());
  
  parser.ExpectTokenString("{");
  
  for (auto& stage : anim_stages) {
    stage.x = parser.ParseInt();
    stage.y = parser.ParseInt();
  }

  parser.ExpectTokenString("}");

  parser.ExpectTokenString("}");
}

void Mesh::UpdateSurface(const struct renderEntity_t* ent,
                         std::vector<ModelPixel>& surfaces,
                         const idImage& image, Vector2 st) const {
  if (!image.IsLoaded()) {
    common->Warning("image %s wasn't loaded", image.GetName().c_str());
    return;
  }

  const auto& imagePixels = image.GetPixels();

  surfaces.resize(size.x * size.y);

  for (int j = 0; j < size.y; ++j) {
    for (int i = 0; i < size.x; ++i) {
      int pixelIndex = (st.y + j) * image.GetWidth() + i + st.x;
      if (pixelIndex >= imagePixels.size()) return;
      surfaces[j * size.x + i] =
          ModelPixel(Vector2(i, j), imagePixels[pixelIndex].screenPixel);
    }
  }
}

void RenderModelMesh::InitFromFile(const std::string& fileName) {
  name = fileName;
  LoadModel();
}

void RenderModelMesh::LoadModel() {
  idToken token;
  idLexer parser;

  if (!purged) {
    PurgeModel();
  }
  purged = false;

  if (!parser.LoadFile(name)) {
    MakeDefaultModel();
    return;
  }

  parser.ExpectTokenString("numMeshes");
  meshes.resize(parser.ParseInt());

  for (auto& mesh : meshes) {
    mesh.ParseMesh(parser);
  }
}

void RenderModelMesh::InstantiateDynamicModel(
    const renderEntity_t* ent, const viewDef_t* view,
    std::unique_ptr<idRenderModel>& cachedModel) {
  if (purged) {
    common->DWarning("model %s instantiated while purged", Name().c_str());
    LoadModel();
  }

  idRenderModelStatic* staticModel;
  if (cachedModel) {
    assert(dynamic_cast<idRenderModelStatic*>(cachedModel.get()));
    staticModel = static_cast<idRenderModelStatic*>(cachedModel.get());
  } else {
    cachedModel = std::make_unique<idRenderModelStatic>();
    staticModel = static_cast<idRenderModelStatic*>(cachedModel.get());
    staticModel->InitEmpty("");
  }

  for (int i = 0; i < meshes.size(); i++) {
    auto& mesh = meshes[i];
    const auto& shader = mesh.shader;

    if (!shader) {
      staticModel->surfaces.clear();
      continue;
    }

    if (!shader->GetStage()) {
      common->Warning("Material %s has empty stage", shader->GetName());
      continue;
    }

    auto stage = mesh.anim_stages[rand()% mesh.anim_stages.size()];

    mesh.UpdateSurface(renderEntity, staticModel->surfaces,
                       *shader->GetStage()->image.get(), stage);
  }

  staticModel->ShiftSurfaces();
}

void RenderModelMesh::PurgeModel() noexcept {
  purged = true;
  meshes.clear();
}

dynamicModel_t RenderModelMesh::IsDynamicModel() const noexcept {
  return DM_CACHED;
}
