#ifndef __ANIM_H__
#define __ANIM_H__

// for converting from 24 frames per second to milliseconds
inline int FRAME2MS(int framenum) { return (framenum * 1000) / 24; }

class MeshAnim {
 private:
  int numFrames;
  int frameRate;
  int animLength;
  Vector2 baseFrame;
  std::vector<Vector2> componentFrames;
  std::string name;

 public:
  MeshAnim();
  ~MeshAnim();

  void Free();
  bool Reload();
  bool LoadAnim(const std::string& filename);

  void GetSingleFrame(int framenum, Vector2& text_coords) const;

  int Length() const;
  int NumFrames() const;
  std::string Name() const;

  // void ConvertTimeToFrame(int time, int cyclecount, frameBlend_t& frame)
  // const;
};

class idDeclModelDef;

class idAnim {
 private:
  const idDeclModelDef* modelDef;
  std::shared_ptr<MeshAnim> anim;
  std::string name;
  std::string realname;

 public:
  idAnim();
  idAnim(const idDeclModelDef* modelDef, std::shared_ptr<idAnim> other_anim);
  ~idAnim();

  void SetAnim(const idDeclModelDef* modelDef, const std::string& sourcename,
               const std::string& animname, std::shared_ptr<MeshAnim> meshanim);
  std::string Name() const;
  std::string FullName() const;
  const MeshAnim* GetMeshAnim(int num) const;
  int NumFrames() const;
};

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

  const idAnim* GetAnim(int index) const;
  int GetSpecificAnim(const std::string& name) const;
  int GetAnim(const std::string& name) const;

 private:
  bool ParseAnim(idLexer& src);

 private:
  std::weak_ptr<idRenderModel> modelHandle;
  std::vector<std::shared_ptr<idAnim>> anims;
};

class idAnimBlend {
 private:
  const class idDeclModelDef* modelDef;
  const MeshAnim* anim;
  int starttime;
  int endtime;

  short cycle;
  short frame;
  short animNum;

  friend class idAnimator;

  void Reset(const idDeclModelDef* _modelDef);
  void CallFrameCommands(idEntity* ent, int fromtime, int totime) const;
  void SetFrame(const idDeclModelDef* modelDef, int animnum, int frame,
                int currenttime, int blendtime);
  void CycleAnim(const idDeclModelDef* modelDef, int animnum, int currenttime,
                 int blendtime);
  void PlayAnim(const idDeclModelDef* modelDef, int animnum, int currenttime,
                int blendtime);
  bool BlendAnim(int currentTime, Vector2& text_coords) const;

 public:
  idAnimBlend();
  std::string AnimName() const;
  std::string AnimFullName() const;
  int NumSyncedAnims() const;
  void Clear(int currentTime, int clearTime);
  bool IsDone(int currentTime) const;
  bool FrameHasChanged(int currentTime) const;
  int GetCycleCount() const;
  void SetCycleCount(int count);
  void SetPlaybackRate(int currentTime, float newRate);
  float GetPlaybackRate() const;
  void SetStartTime(int startTime);
  int GetStartTime() const;
  int GetEndTime() const;
  int GetFrameNumber(int currenttime) const;
  int AnimTime(int currenttime) const;
  int NumFrames() const;
  int Length() const;
  int PlayLength() const;
  const idAnim* Anim() const;
  int AnimNum() const;
};

class idAnimator {
 public:
  idAnimator();
  ~idAnimator();

  void SetEntity(idEntity* ent);
  idEntity* GetEntity() const;

  int GetAnim(const std::string& name) const;

  void ServiceAnims(int fromtime, int totime);
  bool IsAnimating(int currentTime) const;

  void GetTextCoords(Vector2** out_text_coords);

  idRenderModel* SetModel(const std::string& modelname);
  idRenderModel* ModelHandle() const;
  const idDeclModelDef* ModelDef() const;

  bool CreateFrame(int animtime, bool force);
  bool FrameHasChanged(int animtime) const;

  void Clear(int channelNum, int currentTime, int cleartime);
  void SetFrame(int animnum, int frame, int currenttime, int blendtime);
  void CycleAnim(int channelNum, int animnum, int currenttime, int blendtime);
  void PlayAnim(int channelNum, int animnum, int currenttime, int blendTime);

  void ClearAllAnims(int currentTime, int cleartime);

 private:
  void FreeData();
  void PushAnims(int currentTime, int blendTime);

 private:
  const idDeclModelDef* modelDef;
  idEntity* entity;

  idAnimBlend channel;
  std::unique_ptr<Vector2> text_coords;
};

#endif
