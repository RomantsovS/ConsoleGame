#ifndef SWF_SWF_SPRITEINSTANCE_H_
#define SWF_SWF_SPRITEINSTANCE_H_

class idSWF;

class idSWFSpriteInstance
    : public std::enable_shared_from_this<idSWFSpriteInstance> {
 public:
  idSWFSpriteInstance(std::shared_ptr<idSWF> _swf) noexcept;
  ~idSWFSpriteInstance();
  idSWFSpriteInstance(const idSWFSpriteInstance&) = default;
  idSWFSpriteInstance& operator=(const idSWFSpriteInstance&) = default;
  idSWFSpriteInstance(idSWFSpriteInstance&&) = default;
  idSWFSpriteInstance& operator=(idSWFSpriteInstance&&) = default;

  void Init();

  std::shared_ptr<idSWF> GetSWF() noexcept {
    if (auto res = swf.lock()) {
      return res;
    } else {
      return nullptr;
    }
  }

  std::shared_ptr<idSWFScriptObject> GetScriptObject() noexcept {
    return scriptObject;
  }

  void SetVisible(bool visible) noexcept;
  bool IsVisible() noexcept { return isVisible; }
  void SetColor(const Screen::color_type color) noexcept;

  void SetXPos(float xPos = -1.0f) noexcept;

  // sprite instances can be nested
  // std::weak_ptr<idSWFSpriteInstance> parent;

  // depth of this sprite instance in the parent's display list
  // int depth;

  std::shared_ptr<idSWFScriptObject> scriptObject;

  // children display entries
  std::vector<swfDisplayEntry_t> displayList;
  swfDisplayEntry_t* FindDisplayEntry(int depth) noexcept;

  // name of this sprite instance
  std::string name;

  //----------------------------------
  // SWF_PlaceObject.cpp
  //----------------------------------
  swfDisplayEntry_t* PlaceObject(size_t depth, bool isSprite,
                                 const std::string& name,
                                 const Vector2& pos = vec2_origin);
  void RemoveObject() noexcept;

  //----------------------------------
  // SWF_SpriteInstance.cpp
  //----------------------------------
  void RunTo(int frameNum);

  void FreeDisplayList();
  void Clear();
  swfDisplayEntry_t* AddDisplayEntry(int depth, bool isSprite);

 private:
  std::weak_ptr<idSWF> swf;
  bool isVisible;
};

#endif
