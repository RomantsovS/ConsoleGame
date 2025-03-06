#ifndef __DECLPARTICLE_H__
#define __DECLPARTICLE_H__

static const int MAX_PARTICLE_STAGES = 32;

struct renderEntity_t;
struct renderView_t;

struct particleGen_t {
  const renderEntity_t* renderEnt;  // for shaderParms, etc
  const renderView_t* renderView;
  int index;   // particle number in the system
  float frac;  // 0.0 to 1.0

  float age;  // in seconds, calculated as fraction * stage->particleLife
  float animationFrameFrac;  // set by ParticleTexCoords, used to make the cross
                             // faded version
};

class idParticleStage {
 public:
  idParticleStage();
  ~idParticleStage() {}

  void Default();

  // returns the number of verts created, which will range from 0 to
  // 4*NumQuadsPerParticle()
  int CreateParticle(particleGen_t* g /*, idDrawVert* verts*/) const;

  void ParticleTexCoords(particleGen_t* g /*, idDrawVert* verts*/) const;

  std::shared_ptr<idMaterial> material;
  // invisible at a given time
  float cycles;  // allows things to oneShot ( 1 cycle ) or run for a set number
                 // of cycles on a per stage basis

  int cycleMsec;  // ( particleLife + deadTime ) in msec

  float spawnBunching;  // 0.0 = all come out at first instant, 1.0 = evenly
                        // spaced over cycle time
  float particleLife;   // total seconds of life for each particle
  float timeOffset;  // time offset from system start for the first particle to
                     // spawn

  int animationFrames;  // if > 1, subdivide the texture S axis into frames and
                        // crossfade
  float animationRate;  // frames per second
};

class idDeclParticle : public idDecl {
 public:
  std::string DefaultDefinition() const override;
  bool Parse(const char* text, const int textLength,
             bool allowBinaryVersion) override;
  void FreeData() override;

  std::vector<std::shared_ptr<idParticleStage>> stages;

 private:
  std::shared_ptr<idParticleStage> ParseParticleStage(idLexer& src);
};

#endif
