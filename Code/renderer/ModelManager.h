#ifndef MODEL_MANAGER_H
#define MODEL_MANAGER_H

class idRenderModelManager {
 public:
  idRenderModelManager() = default;
  virtual ~idRenderModelManager() = default;
  idRenderModelManager(const idRenderModelManager&) = default;
  idRenderModelManager& operator=(const idRenderModelManager&) = default;
  idRenderModelManager(idRenderModelManager&&) = default;
  idRenderModelManager& operator=(idRenderModelManager&&) = default;

  // registers console commands and clears the list
  virtual void Init() = 0;

  // frees all the models
  virtual void Shutdown() = 0;

  // called only by renderer::BeginLevelLoad
  virtual void BeginLevelLoad() = 0;

  // called only by renderer::EndLevelLoad
  virtual void EndLevelLoad() = 0;

  // returns NULL if modelName is NULL or an empty string, otherwise
  // it will create a default model if not loadable
  virtual std::shared_ptr<idRenderModel> FindModel(
      const std::string& modelName) = 0;

  // returns the default cube model
  virtual std::shared_ptr<idRenderModel> DefaultModel() = 0;

  // world map parsing will add all the inline models with this call
  virtual void AddModel(std::shared_ptr<idRenderModel> model) = 0;
};

// this will be statically pointed at a private implementation
extern idRenderModelManager* renderModelManager;

#endif