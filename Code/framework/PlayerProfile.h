#ifndef FRAMEWORK_PLAYER_PROFILE_H_
#define FRAMEWORK_PLAYER_PROFILE_H_

/*
================================================
idPlayerProfile

The general rule for using cvars for settings is that if you want the player's
profile settings to affect the startup of the game before there is a player
associated with the game, use cvars.  Example: video & volume settings.
================================================
*/
class idPlayerProfile {
  friend class idLocalUser;
  friend class idProfileMgr;

 public:
  enum state_t {
    IDLE = 0,
    SAVING,
    LOADING,
    SAVE_REQUESTED,
    LOAD_REQUESTED,
    ERR
  };

 protected:
  idPlayerProfile();  // don't instantiate. we static_cast the child all over
                      // the place
 public:
  virtual ~idPlayerProfile();

  static idPlayerProfile* CreatePlayerProfile(int deviceIndex);

  void SetDefaults();
  state_t GetState() const { return state; }
  state_t GetRequestedState() const { return requestedState; }
  //------------------------
  // Config
  //------------------------
  /*int				GetConfig() const { return configSet; }
  void			SetConfig(int config, bool save);
  void			RestoreDefault();*/

 private:
  void SetState(state_t value) { state = value; }
  void SetRequestedState(state_t value) { requestedState = value; }

  void ExecConfig(bool save = false, bool forceDefault = false);

 protected:
  // Do not save:
  state_t state;
  state_t requestedState;
  int deviceNum;

  // Save:
  /*uint64			achievementBits;
  uint64			achievementBits2;
  int				dlcReleaseVersion;
  int				configSet;*/
  bool customConfig;
  // bool			leftyFlip;

  // bool			dirty;		// dirty bit to indicate whether
  // or not we need to save
};

#endif