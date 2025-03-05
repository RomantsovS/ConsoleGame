#ifndef __MATERIAL_H__
#define __MATERIAL_H__

class idImage;

struct textureStage_t {
  std::shared_ptr<idImage> image;
};

enum class contentsFlags_t {
  CONTENTS_SOLID = BIT(0),         // an eye is never valid in a solid
  CONTENTS_OPAQUE = BIT(1),        // blocks visibility (for ai)
  CONTENTS_WATER = BIT(2),         // used for water
  CONTENTS_PLAYERCLIP = BIT(3),    // solid to players
  CONTENTS_MONSTERCLIP = BIT(4),   // solid to monsters
  CONTENTS_MOVEABLECLIP = BIT(5),  // solid to moveable entities
  CONTENTS_IKCLIP = BIT(6),        // solid to IK
  CONTENTS_BLOOD = BIT(7),         // used to detect blood decals
  CONTENTS_BODY = BIT(8),          // used for actors
  CONTENTS_PROJECTILE = BIT(9),    // used for projectiles
  CONTENTS_CORPSE = BIT(10),       // used for dead bodies
  CONTENTS_RENDERMODEL =
      BIT(11),                 // used for render models for collision detection
  CONTENTS_TRIGGER = BIT(12),  // used for triggers
  CONTENTS_AAS_SOLID = BIT(13),  // solid for AAS
  CONTENTS_AAS_OBSTACLE = BIT(
      14),  // used to compile an obstacle into AAS that can be enabled/disabled
  CONTENTS_FLASHLIGHT_TRIGGER =
      BIT(15),  // used for triggers that are activated by the flashlight

  // contents used by utils
  CONTENTS_AREAPORTAL = BIT(20),  // portal separating renderer areas
  CONTENTS_NOCSG =
      BIT(21),  // don't cut this brush with CSG operations in the editor

  CONTENTS_REMOVE_UTIL = ~(CONTENTS_AREAPORTAL | CONTENTS_NOCSG)
};

class idMaterial : public idDecl {
 public:
  idMaterial();
  virtual ~idMaterial();
  idMaterial(const idMaterial&) = default;
  idMaterial& operator=(const idMaterial&) = default;
  idMaterial(idMaterial&&) = default;
  idMaterial& operator=(idMaterial&&) = default;

  bool SetDefaultText() noexcept override;
  std::string DefaultDefinition() const override;
  bool Parse(const char* text, const int textLength,
             bool allowBinaryVersion) override;

  // get a specific stage
  const textureStage_t* GetStage() const noexcept { return stage.get(); }

 private:
  // parse the entire material
  void CommonInit() noexcept;
  void ParseMaterial(idLexer& src);
  void ParseStage(idLexer& src);

 private:
  std::unique_ptr<textureStage_t> stage;
};

#endif