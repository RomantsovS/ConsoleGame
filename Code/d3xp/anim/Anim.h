#ifndef __ANIM_H__
#define __ANIM_H__

class idDeclModelDef : public idDecl {
 public:
  idDeclModelDef();
  ~idDeclModelDef();
  idDeclModelDef(const idDeclModelDef&) = default;
  idDeclModelDef& operator=(const idDeclModelDef&) = default;
  idDeclModelDef(idDeclModelDef&&) = default;
  idDeclModelDef& operator=(idDeclModelDef&&) = default;

  std::string DefaultDefinition() const override;
  bool Parse(const char* text, const int textLength,
             bool allowBinaryVersion) override;
  void FreeData() noexcept override;

  std::weak_ptr<idRenderModel> ModelHandle() const noexcept;

 private:
  std::weak_ptr<idRenderModel> modelHandle;
};

#endif