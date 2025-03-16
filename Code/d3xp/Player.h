#ifndef PLAYER_ENTITY_H
#define PLAYER_ENTITY_H

class idPlayer : public idActor {
 public:
  usercmd_t usercmd;

  idPlayerView playerView;  // handles damage kicks and effects

  int oldButtons{};

  // timers
  int minRespawnTime =
      0;  // can respawn when time > this, force after g_forcerespawn
 public:
  CLASS_PROTOTYPE(idPlayer);

  idPlayer() = default;
  virtual ~idPlayer() = default;
  idPlayer(const idPlayer&) = default;
  idPlayer& operator=(const idPlayer&) = default;
  idPlayer(idPlayer&&) = default;
  idPlayer& operator=(idPlayer&&) = default;

  void Spawn();
  void Think() override;

  void Init();
  void SetupWeaponEntity();
  void SelectInitialSpawnPoint(Vector2& origin, Vector2& angles);
  void SpawnFromSpawnSpot();
  void SpawnToPoint(const Vector2& spawn_origin, const Vector2& spawn_angles);
  void SetClipModel();

  void UpdateConditions();

  void RestorePersistantInfo();

  bool Collide(const trace_t& collision,
               const Vector2& velocity) noexcept override;

  void CalcDamagePoints(idEntity* inflictor, idEntity* attacker,
                        const idDict* damageDef, int* health);
  void Damage(idEntity* inflictor, idEntity* attacker, const Vector2& dir,
              const std::string& damageDefName) override;

  // New damage path for instant client feedback.
  void ServerDealDamage(
      int damage, idEntity& inflictor, idEntity& attacker, const Vector2& dir,
      const std::string& damageDefName);  // Actually updates the player's
                                          // health independent of feedback.

  void Killed(idEntity* inflictor, idEntity* attacker, int damage,
              const Vector2& dir) noexcept override;

  std::shared_ptr<renderView_t> GetRenderView();
  void CalculateRenderView();  // called every tic by player code

  void ClientThink(
      /*const int curTime, const float fraction, const bool predict*/) override;
  void WriteToSnapshot(idBitMsg& msg) const override;
  void ReadFromSnapshot(const idBitMsg& msg) override;

  void HandleUserCmds(const usercmd_t& newcmd) noexcept;

  bool IsLocallyControlled() const noexcept;

 private:
  std::shared_ptr<idWeapon> weapon;

  std::shared_ptr<Physics_PlayerMy> physicsObj;  // player physics

  int currentWeapon{-1};
  int idealWeapon{-1};

  void EvaluateControls() noexcept;
  void AdjustSpeed() noexcept;
  void Move();

  void FireWeapon() noexcept;
  void Weapon_Combat();
  void UpdateWeapon();
};

#endif