#ifndef PLAYER_BOMBER_H
#define PLAYER_BASE_H

class PlayerBomber : public idPlayer {
 public:
  CLASS_PROTOTYPE(PlayerBomber);

  PlayerBomber() = default;
  virtual ~PlayerBomber() = default;
  PlayerBomber(const PlayerBomber&) = default;
  PlayerBomber& operator=(const PlayerBomber&) = default;
  PlayerBomber(PlayerBomber&&) = default;
  PlayerBomber& operator=(PlayerBomber&&) = default;

  void Spawn();

  void Killed(idEntity* inflictor, idEntity* attacker, int damage,
              const Vector2& dir) noexcept override;

 private:

  std::shared_ptr<Physics_PlayerMy> physicsObj;  // player physics
};

#endif
