
#include "idlib/precompiled.h"

#include "Model_local.h"
#include "tr_local.h"	// just for R_FreeWorldInteractions and R_CreateWorldInteractions

class idRenderModelManagerLocal : public idRenderModelManager {
public:
	idRenderModelManagerLocal();
	~idRenderModelManagerLocal() = default;
	idRenderModelManagerLocal(const idRenderModelManagerLocal&) = default;
	idRenderModelManagerLocal& operator=(const idRenderModelManagerLocal&) = default;
	idRenderModelManagerLocal(idRenderModelManagerLocal&&) = default;
	idRenderModelManagerLocal& operator=(idRenderModelManagerLocal&&) = default;

	// registers console commands and clears the list
	void Init() override;
	void Shutdown() noexcept override;
	std::shared_ptr<idRenderModel> FindModel(const std::string &modelName) override;
	std::shared_ptr<idRenderModel> DefaultModel() noexcept override;
	void AddModel(std::shared_ptr<idRenderModel> model) override;
	void BeginLevelLoad() override;
	void EndLevelLoad() override;
private:
	std::vector<std::shared_ptr<idRenderModel>> models;
	std::map<std::string, size_t> hash;

	std::shared_ptr<idRenderModel> defaultModel;
	bool insideLevelLoad; // don't actually load now

	std::shared_ptr<idRenderModel> GetModel(const std::string &modelName, bool createIfNotFound);
};

idRenderModelManagerLocal localModelManager;
idRenderModelManager * renderModelManager = &localModelManager;

std::shared_ptr<idRenderModel> idRenderModelManagerLocal::FindModel(const std::string &modelName) {
	return GetModel(modelName, true);
}

std::shared_ptr<idRenderModel> idRenderModelManagerLocal::DefaultModel() noexcept {
	return defaultModel;
}

idRenderModelManagerLocal::idRenderModelManagerLocal() {
	defaultModel = nullptr;
	insideLevelLoad = false;
}

void idRenderModelManagerLocal::Init() {
	insideLevelLoad = false;

	// create a default model
	auto model = std::make_shared<idRenderModelStatic>();
	model->InitEmpty("_DEFAULT");
	model->MakeDefaultModel();
	model->SetLevelLoadReferenced(true);
	defaultModel = model;
	AddModel(model);
}

/*
=================
idRenderModelManagerLocal::Shutdown
=================
*/
void idRenderModelManagerLocal::Shutdown() noexcept {
	defaultModel = nullptr;
	models.clear();
	hash.clear();
}

std::shared_ptr<idRenderModel> idRenderModelManagerLocal::GetModel(const std::string & modelName, bool createIfNotFound) {
	if (modelName.empty())
		return nullptr;

	std::string canonical = modelName;

	std::string extension;
	idStr::ExtractFileExtension(canonical, extension);

	// see if it is already present
	auto key = modelName;
	auto iter = hash.find(key);

	if (iter != hash.end()) {
		auto model = models[iter->second];

		if (!model->IsLoaded()) {
			// reload it if it was purged
			if (model->SupportsBinaryModel()) {
				idFileLocal file(fileSystem->OpenFileReadMemory(canonical));
				model->PurgeModel();
				if (!model->LoadBinaryModel(file)) {
					model->LoadModel();
				}
			}
			else {
				model->LoadModel();
			}
		}
		else if (insideLevelLoad && !model->IsLevelLoadReferenced()) {
			// we are reusing a model already in memory, but
			// touch all the materials to make sure they stay
			// in memory as well
			//model->TouchData();
		}
		model->SetLevelLoadReferenced(true);
		return model;
	}

	std::shared_ptr<idRenderModel> model;

	if (extension == "textmodel" || extension == "bmp") {
		model = std::make_shared<idRenderModelStatic>();
	}

	if (model) {
		idFileLocal file(fileSystem->OpenFileReadMemory(canonical));

		if (!model->SupportsBinaryModel()) {
			model->InitFromFile(canonical);
		}
		else {
			if (!model->LoadBinaryModel(file)) {
				model->InitFromFile(canonical);
			}
		}
	}

	if (!model) {
		if (!extension.empty()) {
			common->Warning("unknown model type '%s'", canonical.c_str());
		}

		if (!createIfNotFound) {
			return nullptr;
		}

		auto smodel = std::make_shared<idRenderModelStatic>();
		smodel->InitEmpty(modelName);
		smodel->MakeDefaultModel();

		model = smodel;
	}

	model->SetLevelLoadReferenced(true);

	if (!createIfNotFound && model->IsDefaultModel()) {
		model = nullptr;

		return nullptr;
	}

	AddModel(model);

	return model;
}

void idRenderModelManagerLocal::AddModel(std::shared_ptr<idRenderModel> model) {
	models.push_back(model);

	hash[model->Name()] = models.size() - 1;
}

/*
=================
idRenderModelManagerLocal::BeginLevelLoad
=================
*/
void idRenderModelManagerLocal::BeginLevelLoad() {
	insideLevelLoad = true;

	for (auto &model : models)
	{
		// always reload all models 
		if (model->IsReloadable()) {
			R_CheckForEntityDefsUsingModel(model);
			model->PurgeModel();
		}
	}
}

/*
=================
idRenderModelManagerLocal::EndLevelLoad
=================
*/
void idRenderModelManagerLocal::EndLevelLoad() {
	insideLevelLoad = false;

	int	purgeCount = 0;
	int	keepCount = 0;
	int	loadCount = 0;

	// purge any models not touched
	for (auto &model : models) {
		if (!model->IsLevelLoadReferenced() && model->IsLoaded() && model->IsReloadable()) {

			common->Printf( "purging %s\n", model->Name().c_str() );

			purgeCount++;

			R_CheckForEntityDefsUsingModel(model);

			model->PurgeModel();

		}
		else {
			common->Printf( "keeping %s\n", model->Name().c_str() );

			keepCount++;
		}
	}

	// load any new ones
	for (auto &model : models) {
		if (model->IsLevelLoadReferenced() && !model->IsLoaded() && model->IsReloadable()) {
			loadCount++;
			model->LoadModel();
		}
	}
}
