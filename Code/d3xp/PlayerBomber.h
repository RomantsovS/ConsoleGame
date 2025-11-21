#ifndef PLAYER_BOMBER_H
#define PLAYER_BASE_H

class PlayerBomber: public idPlayer {
 public:

 public:
  CLASS_PROTOTYPE(PlayerBomber);

  PlayerBomber() = default;
  virtual ~PlayerBomber() = default;
  PlayerBomber(const PlayerBomber&) = default;
  PlayerBomber& operator=(const PlayerBomber&) = default;
  PlayerBomber(PlayerBomber&&) = default;
  PlayerBomber& operator=(PlayerBomber&&) = default;

 private:

};

#endif
