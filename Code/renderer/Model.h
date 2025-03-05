#ifndef RENDER_MODEL_H
#define RENDER_MODEL_H

#include "Screen.h"

struct renderEntity_t;
struct viewDef_t;

class ModelPixel {
 public:
  ModelPixel() = default;
  ModelPixel(Vector2 pos, Screen::Pixel pixel) noexcept;

  Vector2 origin;
  Screen::Pixel screenPixel;
};

enum dynamicModel_t {
  DM_STATIC,     // never creates a dynamic model
  DM_CACHED,     // once created, stays constant until the entity is updated
                 // (animating characters)
  DM_CONTINUOUS  // must be recreated for every single view (time dependent
                 // things like particles)
};

class idRenderModel {
 public:
  idRenderModel() = default;
  virtual ~idRenderModel() = default;
  idRenderModel(const idRenderModel&) = default;
  idRenderModel& operator=(const idRenderModel&) = default;
  idRenderModel(idRenderModel&&) = default;
  idRenderModel& operator=(idRenderModel&&) = default;

  // Loads static models only, dynamic models must be loaded by the modelManager
  virtual void InitFromFile(const std::string& fileName) = 0;

  // Supports reading/writing binary file formats
  virtual bool LoadBinaryModel(idFile* file) = 0;
  virtual bool SupportsBinaryModel() = 0;

  // this is used for dynamically created surfaces, which are assumed to not be
  // reloadable. It can be called again to clear out the surfaces of a dynamic
  // model for regeneration.
  virtual void InitEmpty(const std::string& fileName) = 0;

  // frees all the data, but leaves the class around for dangling references,
  // which can regenerate the data with LoadModel()
  virtual void PurgeModel() = 0;

  // used for initial loads, reloadModel, and reloading the data of purged
  // models Upon exit, the model will absolutely be valid, but possibly as a
  // default model
  virtual void LoadModel() = 0;

  // internal use
  virtual bool IsLoaded() = 0;
  virtual void SetLevelLoadReferenced(bool referenced) = 0;
  virtual bool IsLevelLoadReferenced() = 0;

  // returns the name of the model
  virtual const std::string& Name() const = 0;

  // returns the number of surfaces
  virtual int NumSurfaces() const = 0;

  // get a pointer to a surface
  virtual const ModelPixel& Surface(int surfaceNum) const = 0;

  // models parsed from inside map files or dynamically created cannot be
  // reloaded by reloadmodels
  virtual bool IsReloadable() const = 0;

  // md3, md5, particles, etc
  virtual dynamicModel_t IsDynamicModel() const = 0;

  // if the load failed for any reason, this will return true
  virtual bool IsDefaultModel() const = 0;

  // returns a static model based on the definition and view
  // currently, this will be regenerated for every view, even though
  // some models, like character meshes, could be used for multiple (mirror)
  // views in a frame, or may stay static for multiple frames (corpses)
  // The renderer will delete the returned dynamic model the next view
  // This isn't const, because it may need to reload a purged model if it
  // wasn't precached correctly.
  virtual idRenderModel* InstantiateDynamicModel(
      const renderEntity_t* ent, const viewDef_t* view,
      idRenderModel* cachedModel) = 0;

  virtual Screen::color_type GetColor() const = 0;
  virtual void SetColor(Screen::color_type col) = 0;
};

#endif