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

	parser.ExpectTokenString("textcoord");
	text_coords.x = parser.ParseInt();
	text_coords.y = parser.ParseInt();

	parser.ExpectTokenString("}");
}

void Mesh::UpdateSurface(const struct renderEntity_t* ent, std::vector<ModelPixel>& surfaces, const idImage& image) const {
	const char symbol{ '\xDB' };

	if (!image.IsLoaded()) {
		common->DWarning("image %s wasn't loaded", image.GetName());
	}

	const auto& imagePixels = image.GetPixels();

	for (int j = 0; j < size.y; ++j) {
		for (int i = 0; i < size.x; ++i) {
			int pixelIndex = (text_coords.y + j) * image.GetWidth() + text_coords.x + i;
			surfaces.emplace_back(Vector2(i, j), Screen::Pixel(symbol, imagePixels[pixelIndex].screenPixel.color));
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

idRenderModel* RenderModelMesh::InstantiateDynamicModel(const renderEntity_t* ent, const viewDef_t* view,
	idRenderModel* cachedModel) {
	if (purged) {
		common->DWarning("model %s instantiated while purged", Name());
		LoadModel();
	}

	idRenderModelStatic* staticModel;
	if (cachedModel) {
		assert(dynamic_cast<idRenderModelStatic*>(cachedModel));
		staticModel = static_cast<idRenderModelStatic*>(cachedModel);
	}
	else {
		staticModel = new idRenderModelStatic;
		staticModel->InitEmpty("");
	}

	staticModel->surfaces.clear();

	for (int i = 0; i < meshes.size(); i++) {
		auto& mesh = meshes[i];
		const auto& shader = mesh.shader;

		if (!shader) {
			staticModel->surfaces.clear();
			continue;
		}

		mesh.UpdateSurface(ent, staticModel->surfaces, *shader->GetStage()->image.get());
	}

	staticModel->ShiftSurfaces();

	return staticModel;
}

void RenderModelMesh::PurgeModel() noexcept {
	purged = true;
	meshes.clear();
}

dynamicModel_t RenderModelMesh::IsDynamicModel() const noexcept {
	return DM_CACHED;
}
