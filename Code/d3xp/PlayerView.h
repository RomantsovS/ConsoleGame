#ifndef __GAME_PLAYERVIEW_H__
#define __GAME_PLAYERVIEW_H__

class idPlayerView {
 public:
  idPlayerView();

  void SetPlayerEntity(class idPlayer* playerEnt);

  void RenderPlayerView();

 private:
  void SingleView(const renderView_t* view);

  idPlayer* player;
  renderView_t view;
};

#endif