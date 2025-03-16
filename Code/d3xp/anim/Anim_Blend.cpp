#include "idlib/precompiled.h"

#include "../Game_local.h"

/*
=====================
idDeclModelDef::idDeclModelDef
=====================
*/
idDeclModelDef::idDeclModelDef() {}

/*
=====================
idDeclModelDef::~idDeclModelDef
=====================
*/
idDeclModelDef::~idDeclModelDef() { FreeData(); }

/*
=====================
idDeclModelDef::FreeData
=====================
*/
void idDeclModelDef::FreeData() noexcept {}

/*
================
idDeclModelDef::DefaultDefinition
================
*/
std::string idDeclModelDef::DefaultDefinition() const { return "{ }"; }

/*
=====================
idDeclModelDef::ModelHandle
=====================
*/
std::weak_ptr<idRenderModel> idDeclModelDef::ModelHandle() const noexcept {
  return modelHandle;
}

bool idDeclModelDef::ParseAnim(idLexer& src) {
  int i;
  idToken realname;
  idToken token;

  if (!src.ReadToken(&realname)) {
    src.Warning("Unexpected end of file");
    MakeDefault();
    return false;
  }

  auto alias = realname;

  for (i = 0; i < anims.size(); i++) {
    if (anims[i]->FullName() == realname) {
      break;
    }
  }

  if (i < anims.size()) {
    src.Warning("Duplicate anim '%s'", realname.c_str());
    MakeDefault();
    return false;
  }

  // create the alias associated with this animation
  auto anim = std::make_shared<idAnim>();
  anims.push_back(anim);

  // random anims end with a number.  find the numeric suffix of the animation.
  for (i = alias.size() - 1; i > 0; i--) {
    if (!std::isdigit((unsigned char)alias[i])) {
      break;
    }
  }

  // check for zero length name, or a purely numeric name
  if (i <= 0) {
    src.Warning("Invalid animation name '%s'", alias.c_str());
    MakeDefault();
    return false;
  }

  // remove the numeric suffix
  alias.resize(i + 1);

  // parse the anims from the string
  if (!src.ReadToken(&token)) {
    src.Warning("Unexpected end of file");
    MakeDefault();
    return false;
  }

  // lookup the animation
  auto meshanim = std::make_shared<MeshAnim>();
  if (!meshanim->LoadAnim(token)) {
    src.Warning("Couldn't load anim '%s'", token.c_str());
    MakeDefault();
    return false;
  }

  anim->SetAnim(this, realname, alias, meshanim);

  return true;
}

bool idDeclModelDef::Parse(const char* text, const int textLength,
                           bool allowBinaryVersion) {
  std::string filename, extension;
  idLexer src;
  idToken token;
  idToken token2;

  src.LoadMemory(text, textLength, GetFileName(), GetLineNum());
  src.SkipUntilString("{");

  while (1) {
    if (!src.ReadToken(&token)) {
      break;
    }

    if (token == "}") {
      break;
    }

    if (token == "inherit") {
      if (!src.ReadToken(&token2)) {
        src.Warning("Unexpected end of file");
        MakeDefault();
        return false;
      }

      /*const std::shared_ptr<idDeclModelDef> copy =
      std::dynamic_pointer_cast<idDeclModelDef>(declManager->FindType(declType_t::DECL_MODELDEF,
      token2, false)); if (!copy) { common->Warning("Unknown model definition
      '%s'", token2.c_str());
      }
      else if (copy->GetState() == declState_t::DS_DEFAULTED) {
              common->Warning("inherited model definition '%s' defaulted",
      token2.c_str()); MakeDefault(); return false;
      }
      else {
              CopyDecl(copy);
              numDefaultAnims = anims.Num();
      }*/
    } else if (token == "mesh") {
      if (!src.ReadToken(&token2)) {
        src.Warning("Unexpected end of file");
        MakeDefault();
        return false;
      }
      filename = token2;
      idStr::ExtractFileExtension(filename, extension);
      if (extension != "mesh") {
        src.Warning("Invalid model for MD5 mesh");
        MakeDefault();
        return false;
      }
      modelHandle = renderModelManager->FindModel(filename);
      if (modelHandle.expired()) {
        src.Warning("Model '%s' not found", filename.c_str());
        MakeDefault();
        return false;
      }

      if (auto spModelHandle = modelHandle.lock()) {
        if (spModelHandle->IsDefaultModel()) {
          src.Warning("Model '%s' defaulted", filename.c_str());
          MakeDefault();
          return false;
        }
      }
    } else if (token == "anim") {
      if (!modelHandle.lock()) {
        src.Warning("Must specify mesh before defining anims");
        MakeDefault();
        return false;
      }
      if (!ParseAnim(src)) {
        MakeDefault();
        return false;
      }
    } else {
      src.Warning("unknown token '%s'", token.c_str());
      MakeDefault();
      return false;
    }
  }

  return true;
}

int idDeclModelDef::GetSpecificAnim(const std::string& name) const {
  int i;

  // find a specific animation
  for (i = 0; i < anims.size(); i++) {
    if (anims[i]->FullName() == name) {
      return i + 1;
    }
  }

  // didn't find it
  return 0;
}

const idAnim* idDeclModelDef::GetAnim(int index) const {
  if ((index < 1) || (index > anims.size())) {
    return nullptr;
  }

  return anims[index - 1].get();
}

int idDeclModelDef::GetAnim(const std::string& name) const {
  const int MAX_ANIMS = 64;
  std::array<int, MAX_ANIMS> animList;

  if (!name.empty() && std::isdigit(name.back())) {
    // find a specific animation
    return GetSpecificAnim(name);
  }

  // find all animations with same name
  int numAnims = 0;
  for (int i = 0; i < anims.size(); i++) {
    if (anims[i]->Name() == name) {
      animList[numAnims++] = i;
      if (numAnims >= MAX_ANIMS) {
        break;
      }
    }
  }

  if (!numAnims) {
    return 0;
  }

  // get a random anim
  // FIXME: don't access gameLocal here?
  return animList[gameLocal.GetRandomValue(0, numAnims - 1)] + 1;
}

idAnim::idAnim() {
  modelDef = nullptr;
  anim = nullptr;
}

idAnim::idAnim(const idDeclModelDef* modelDef,
               std::shared_ptr<idAnim> other_anim) {
  this->modelDef = modelDef;
  name = other_anim->name;
  realname = other_anim->realname;

  anim = other_anim->anim;
}

idAnim::~idAnim() {}

void idAnim::SetAnim(const idDeclModelDef* modelDef,
                     const std::string& sourcename, const std::string& animname,
                     const std::shared_ptr<MeshAnim> meshanim) {
  this->modelDef = modelDef;

  anim = nullptr;

  realname = sourcename;
  name = animname;

  anim = meshanim;
}

std::string idAnim::Name() const { return name; }

std::string idAnim::FullName() const { return realname; }

const MeshAnim* idAnim::GetMeshAnim(int num) const { return anim.get(); }

int idAnim::NumFrames() const {
  if (!anim) {
    return 0;
  }

  return anim->NumFrames();
}

idAnimBlend::idAnimBlend() { Reset(nullptr); }

void idAnimBlend::Reset(const idDeclModelDef* _modelDef) {
  modelDef = _modelDef;
  cycle = 1;
  starttime = 0;
  endtime = 0;
  frame = 0;
  animNum = 0;
}

int idAnimBlend::NumFrames() const {
  const idAnim* anim = Anim();
  if (!anim) {
    return 0;
  }

  return anim->NumFrames();
}

void idAnimBlend::SetFrame(const idDeclModelDef* modelDef, int _animNum,
                           int _frame, int currentTime, int blendTime) {
  Reset(modelDef);
  if (!modelDef) {
    return;
  }

  const idAnim* _anim = modelDef->GetAnim(_animNum);
  if (!_anim) {
    return;
  }

  animNum = _animNum;
  starttime = currentTime;
  endtime = -1;
  cycle = -1;
  frame = _frame;

  // a frame of 0 means it's not a single frame blend, so we set it to frame + 1
  if (frame <= 0) {
    frame = 1;
  } else if (frame > _anim->NumFrames()) {
    frame = _anim->NumFrames();
  }
}

void idAnimBlend::Clear(int currentTime, int clearTime) {
  if (!clearTime) {
    Reset(modelDef);
  }
}

bool idAnimBlend::IsDone(int currentTime) const {
  if (!frame && (endtime > 0) && (currentTime >= endtime)) {
    return true;
  }

  return false;
}

bool idAnimBlend::FrameHasChanged(int currentTime) const {
  // if we don't have an anim, no change
  if (!animNum) {
    return false;
  }

  // if anim is done playing, no change
  if ((endtime > 0) && (currentTime > endtime)) {
    return false;
  }

  // if we're a single frame anim and this isn't the frame we started on, we
  // don't need to update
  if ((frame || (NumFrames() == 1)) && (currentTime != starttime)) {
    return false;
  }

  return true;
}

const idAnim* idAnimBlend::Anim() const {
  if (!modelDef) {
    return nullptr;
  }

  const idAnim* anim = modelDef->GetAnim(animNum);
  return anim;
}

int idAnimBlend::AnimTime(int currenttime) const {
  int time = 0;
  const idAnim* anim = Anim();

  if (anim) {
    if (frame) {
      return FRAME2MS(frame - 1);
    }

    // most of the time we're running at the original frame rate, so avoid the
    // int-to-float-to-int conversion
    /* if (rate == 1.0f) {
      time = currentTime - starttime + timeOffset;
    } else {
      time = static_cast<int>((currentTime - starttime) * rate) + timeOffset;
    }

    // given enough time, we can easily wrap time around in our frame
    // calculations, so keep cycling animations' time within the length of the
    // anim.
    length = anim->Length();
    if ((cycle < 0) && (length > 0)) {
      time %= length;

      // time will wrap after 24 days (oh no!), resulting in negative results
      // for the %. adding the length gives us the proper result.
      if (time < 0) {
        time += length;
      }
    }*/
    return time;
  } else {
    return 0;
  }
}

bool idAnimBlend::BlendAnim(int currentTime, Vector2& text_coords) const {
  const idAnim* anim = Anim();
  if (!anim) {
    return false;
  }

  auto time = AnimTime(currentTime);

  auto* meshanim = anim->GetMeshAnim(0);
  if (frame) {
    meshanim->GetSingleFrame(frame - 1, text_coords);
  } else {
    // meshanim->ConvertTimeToFrame(time, cycle, frametime);
    // md5anim->GetInterpolatedFrame(frametime, text_coords);
  }

  return true;
}

idAnimator::idAnimator() {
  modelDef = nullptr;
  entity = nullptr;
  text_coords = nullptr;

  channel.Reset(nullptr);
}

idAnimator::~idAnimator() { FreeData(); }

void idAnimator::FreeData() {
  if (entity) {
    entity->BecomeInactive(TH_ANIMATE);
  }

  channel.Reset(nullptr);

  text_coords = nullptr;

  modelDef = nullptr;

  // ForceUpdate();
}

void idAnimator::PushAnims(int currentTime, int blendTime) {
  if (channel.starttime == currentTime) {
    return;
  }

  channel.Reset(modelDef);
  // ForceUpdate();
}

idRenderModel* idAnimator::SetModel(const std::string& modelname) {
  FreeData();

  // check if we're just clearing the model
  if (modelname.empty()) {
    return nullptr;
  }

  modelDef =
      std::static_pointer_cast<idDeclModelDef>(
          declManager->FindType(declType_t::DECL_MODELDEF, modelname, false))
          .get();
  if (!modelDef) {
    return nullptr;
  }

  auto renderModel = modelDef->ModelHandle();
  if (renderModel.expired()) {
    modelDef = nullptr;
    return nullptr;
  }

  text_coords = std::make_unique<Vector2>();

  channel.Reset(nullptr);

  return modelDef->ModelHandle().lock().get();
}

void idAnimator::SetEntity(idEntity* ent) { entity = ent; }

idEntity* idAnimator::GetEntity() const { return entity; }

int idAnimator::GetAnim(const std::string& name) const {
  if (!modelDef) {
    return 0;
  }

  return modelDef->GetAnim(name);
}

idRenderModel* idAnimator::ModelHandle() const {
  if (!modelDef) {
    return nullptr;
  }

  return modelDef->ModelHandle().lock().get();
}

const idDeclModelDef* idAnimator::ModelDef() const { return modelDef; }

void idAnimator::Clear(int channelNum, int currentTime, int cleartime) {
  channel.Clear(currentTime, cleartime);
  // ForceUpdate();
}

void idAnimator::SetFrame(int animNum, int frame, int currentTime,
                          int blendTime) {
  if (!modelDef || !modelDef->GetAnim(animNum)) {
    return;
  }

  PushAnims(currentTime, blendTime);
  channel.SetFrame(modelDef, animNum, frame, currentTime, blendTime);
  if (entity) {
    entity->BecomeActive(TH_ANIMATE);
  }
}

void idAnimator::ClearAllAnims(int currentTime, int cleartime) {
  Clear(0, currentTime, cleartime);

  // ForceUpdate();
}

void idAnimator::ServiceAnims(int fromtime, int totime) {
  if (!modelDef) {
    return;
  }

  if (!IsAnimating(totime)) {
    // stoppedAnimatingUpdate = true;
    if (entity) {
      entity->BecomeInactive(TH_ANIMATE);

      // present one more time with stopped animations so the renderer can
      // properly recreate interactions
      entity->BecomeActive(TH_UPDATEVISUALS);
    }
  }
}

bool idAnimator::IsAnimating(int currentTime) const {
  if (!modelDef || modelDef->ModelHandle().expired()) {
    return false;
  }

  if (!channel.IsDone(currentTime)) {
    return true;
  }

  return false;
}

bool idAnimator::FrameHasChanged(int currentTime) const {
  if (!modelDef || modelDef->ModelHandle().expired()) {
    return false;
  }

  if (channel.FrameHasChanged(currentTime)) {
    return true;
  }

  /*if (forceUpdate && IsAnimating(currentTime)) {
    return true;
  }*/

  return false;
}

bool idAnimator::CreateFrame(int currentTime, bool force) {
  return channel.BlendAnim(currentTime, *text_coords.get());
}

void idAnimator::GetTextCoords(Vector2** out_text_coords) {
  *out_text_coords = text_coords.get();
}
