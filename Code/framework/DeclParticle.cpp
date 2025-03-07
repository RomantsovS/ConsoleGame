#include "idlib/precompiled.h"
#pragma hdrstop

std::shared_ptr<idParticleStage> idDeclParticle::ParseParticleStage(idLexer& src) {
  idToken token;

  auto stage = std::make_shared<idParticleStage>();
  stage->Default();

  while (1) {
    if (!src.ReadToken(&token)) {
      break;
    }
    if (token == "}") {
      break;
    }
    if (token == "material") {
      src.ReadToken(&token);
      stage->material = declManager->FindMaterial(token.c_str());
      continue;
    }
    if (token == "time") {
      stage->particleLife = src.ParseFloat();
      continue;
    }
    if (token == "cycles") {
      stage->cycles = src.ParseFloat();
      continue;
    }
    if (token == "timeOffset") {
      stage->timeOffset = src.ParseFloat();
      continue;
    }
    if (token == "animationFrames") {
      stage->animationFrames = src.ParseInt();
      continue;
    }
    if (token == "animationRate") {
      stage->animationRate = src.ParseFloat();
      continue;
    }
    src.Error("unknown token %s\n", token.c_str());
  }

  // derive values
  stage->cycleMsec = (stage->particleLife) * 1000;

  return stage;
}

bool idDeclParticle::Parse(const char* text, const int textLength,
                           bool allowBinaryVersion) {
  idLexer src;
  idToken token;

  src.LoadMemory(text, textLength, GetFileName(), GetLineNum());
  src.SkipUntilString("{");

  while (1) {
    if (!src.ReadToken(&token)) {
      break;
    }

    if (token == "}") {
      break;
    }

    if (token == "{") {
      if (stages.size() >= MAX_PARTICLE_STAGES) {
        src.Error("Too many particle stages");
        MakeDefault();
        return false;
      }
      auto stage = ParseParticleStage(src);
      if (!stage) {
        src.Warning("Particle stage parse failed");
        MakeDefault();
        return false;
      }
      stages.push_back(std::move(stage));
      continue;
    }

    src.Warning("bad token %s", token.c_str());
    MakeDefault();
    return false;
  }

  return true;
}

void idDeclParticle::FreeData() { stages.clear(); }

std::string idDeclParticle::DefaultDefinition() const {
  return "{\n"
         "\t"
         "{\n"
         "\t\t"
         "material\t_default\n"
         "\t\t"
         "count\t20\n"
         "\t\t"
         "time\t\t1.0\n"
         "\t"
         "}\n"
         "}";
}

idParticleStage::idParticleStage() {
  material = nullptr;
  cycles = 0.0f;
  cycleMsec = 0;
  spawnBunching = 0.0f;
  particleLife = 0.0f;
  timeOffset = 0.0f;
  animationFrames = 0;
  animationRate = 0.0f;
}

void idParticleStage::Default() {
  material = declManager->FindMaterial("_default");
  spawnBunching = 1.0f;
  particleLife = 1.5f;
  timeOffset = 0.0f;
  animationFrames = 0;
  animationRate = 0.0f;
  cycleMsec = (particleLife) * 1000;
}

std::pair<int, int> idParticleStage::ParticleTexCoords(particleGen_t* g) const {
  float s, width;
  float t, height;

  if (animationFrames > 1) {
    width = 1.0f / animationFrames;
    float floatFrame;
    if (animationRate) {
      // explicit, cycling animation
      floatFrame = g->age * animationRate;
    } else {
      // single animation cycle over the life of the particle
      floatFrame = g->frac * animationFrames;
    }
    int intFrame = (int)floatFrame;
    g->animationFrameFrac = floatFrame - intFrame;
    s = width * intFrame;
  } else {
    s = 0.0f;
    width = 1.0f;
  }

  t = 0.0f;
  height = 1.0f;

  return {s, width};
}

int idParticleStage::CreateParticle(particleGen_t* g) const {
  auto st = ParticleTexCoords(g);

  if (animationFrames <= 1) {
    return 0;
  }
  return 0;
}