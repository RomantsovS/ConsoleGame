#ifndef MODEL_LOCAL_H
#define MODEL_LOCAL_H

class idRenderModelStatic : public idRenderModel {
 public:
  idRenderModelStatic();
  virtual ~idRenderModelStatic();
  idRenderModelStatic(const idRenderModelStatic&) = default;
  idRenderModelStatic& operator=(const idRenderModelStatic&) = default;
  idRenderModelStatic(idRenderModelStatic&&) = default;
  idRenderModelStatic& operator=(idRenderModelStatic&&) = default;

  void InitFromFile(const std::string& fileName) override;

  void PurgeModel() noexcept override;
  void LoadModel() override;
  bool IsLoaded() noexcept override;
  void SetLevelLoadReferenced(bool referenced) noexcept override;
  bool IsLevelLoadReferenced() noexcept override;

  void InitEmpty(const std::string& fileName) override;
  const std::string& Name() const override;
  int NumSurfaces() const noexcept override;
  const ModelPixel& Surface(int surfaceNum) const noexcept override;

  dynamicModel_t IsDynamicModel() const noexcept override;
  bool IsDefaultModel() const noexcept override;
  bool IsReloadable() const noexcept override;
  void InstantiateDynamicModel(
      const renderEntity_t* ent, const viewDef_t* view,
      std::unique_ptr<idRenderModel>& cachedModel) override;

  void MakeDefaultModel();

  bool LoadTextModel(const std::string& fileName);
  bool LoadMeshStatic(const std::string& fileName);

  void ShiftSurfaces();

  Screen::color_type GetColor() const noexcept override;
  void SetColor(Screen::color_type col) noexcept override;

 public:
  std::vector<ModelPixel> surfaces;

 protected:
  std::string name;
  bool defaulted = false;
  bool purged;               // eventually we will have dynamic reloading
  bool reloadable;           // if not, reloadModels won't check timestamp
  bool levelLoadReferenced;  // for determining if it needs to be freed
 private:
  Screen::color_type color;
};

class Mesh {
 public:
  void ParseMesh(idLexer& parser);

  void UpdateSurface(const renderEntity_t* ent, const Vector2& text_coords,
                     std::vector<ModelPixel>& surfaces,
                     const idImage& image) const;


  std::shared_ptr<idMaterial> GetShader();
 private:
  std::shared_ptr<idMaterial> shader;  // material applied to mesh
  Vector2 size;
};

class RenderModelMesh : public idRenderModelStatic {
 public:
  void InitFromFile(const std::string& fileName) override;
  void LoadModel() override;
  void InstantiateDynamicModel(
      const renderEntity_t* ent, const viewDef_t* view,
      std::unique_ptr<idRenderModel>& cachedModel) override;
  void PurgeModel() noexcept override;

  dynamicModel_t IsDynamicModel() const noexcept override;

 private:
  std::vector<Mesh> meshes;
};

#endif
