#ifndef SYS_SYS_PROFILE_H_
#define SYS_SYS_PROFILE_H_

class idLocalUser;
class idPlayerProfile;

/*
================================================
idProfileMgr
================================================
*/
class idProfileMgr {
public:
	idProfileMgr();
	~idProfileMgr();

	// Called the first time it's asked to load
	void				Init(idLocalUser* user);

	void 				Pump();
	idPlayerProfile* GetProfile();

private:
	void				LoadSettingsAsync();
	//void				SaveSettingsAsync();

	void				OnLoadSettingsCompleted(/*idSaveLoadParms* parms*/);
	//void				OnSaveSettingsCompleted(idSaveLoadParms* parms);

private:
	idLocalUser* user;					// reference passed in
	idPlayerProfile* profile;
};

#endif
