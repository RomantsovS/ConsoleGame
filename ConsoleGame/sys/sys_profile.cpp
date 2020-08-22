#include "sys_profile.h"
#include "../framework/PlayerProfile.h"
#include "sys_localuser.h"
#include "../framework/Common_local.h"

/*
========================
idProfileMgr
========================
*/
idProfileMgr::idProfileMgr() : profile(nullptr), user(nullptr) {
}


/*
================================================
~idProfileMgr
================================================
*/
idProfileMgr::~idProfileMgr() {
}

/*
========================
idProfileMgr::Init
========================
*/
void idProfileMgr::Init(idLocalUser* user_) {
	user = user_;
}

/*
========================
idProfileMgr::Pump
========================
*/
void idProfileMgr::Pump() {
	// profile can be NULL if we forced the user to register as in the case of map-ing into a level from the press start screen
	if (!profile) {
		return;
	}

	// See if we are done with saving/loading the profile
	bool saving = profile->GetState() == idPlayerProfile::SAVING;
	bool loading = profile->GetState() == idPlayerProfile::LOADING;
	/*if ((saving || loading) && session->IsSaveGameCompletedFromHandle(handle)) {
		profile->SetState(idPlayerProfile::IDLE);

		if (saving) {
			// Done saving
		}
		else if (loading) {
			// Done loading
			const idSaveLoadParms& parms = profileLoadProcessor->GetParms();
			if (parms.GetError() == SAVEGAME_E_FOLDER_NOT_FOUND || parms.GetError() == SAVEGAME_E_FILE_NOT_FOUND) {
				profile->SaveSettings(true);
			}
			else if (parms.GetError() == SAVEGAME_E_CORRUPTED) {
				idLib::Warning("Profile corrupt, creating a new one...");
				common->Dialog().AddDialog(GDM_CORRUPT_PROFILE, DIALOG_CONTINUE, NULL, NULL, false);
				profile->SetDefaults();
				profile->SaveSettings(true);
			}
			else if (parms.GetError() != SAVEGAME_E_NONE) {
				profile->SetState(idPlayerProfile::ERR);
			}

			session->OnLocalUserProfileLoaded(user);
		}
	}
	else if (saving || loading) {
		return;
	}*/

	// See if we need to save/load the profile
	if (profile->GetRequestedState() == idPlayerProfile::SAVE_REQUESTED /*&& profile->IsDirty()*/) {
		/*profile->MarkDirty(false);
		SaveSettingsAsync();
		// Syncs the steam data
		//session->StoreStats();
		profile->SetRequestedState(idPlayerProfile::IDLE);*/
	}
	else if (profile->GetRequestedState() == idPlayerProfile::LOAD_REQUESTED) {
		LoadSettingsAsync();
		profile->SetRequestedState(idPlayerProfile::IDLE);
	}
}

/*
========================
idProfileMgr::GetProfile
========================
*/
idPlayerProfile* idProfileMgr::GetProfile() {
	//assert(user != NULL);
	if (!profile) {
		// Lazy instantiation
		// Create a new profile
		profile = idPlayerProfile::CreatePlayerProfile(user->GetInputDevice());
		if (!profile) {
			return nullptr;
		}
	}

	bool loading = (profile->GetState() == idPlayerProfile::LOADING) || (profile->GetRequestedState() == idPlayerProfile::LOAD_REQUESTED);
	if (loading) {
		return nullptr;
	}

	return profile;
}

/*
========================
idProfileMgr::LoadSettingsAsync
========================
*/
void idProfileMgr::LoadSettingsAsync() {
	if (profile && saveGame_enable.GetBool()) {
		if (profileLoadProcessor->InitLoadProfile(profile, "")) {
			// Skip the not found error because this might be the first time to play the game!
			profileLoadProcessor->SetSkipSystemErrorDialogMask(SAVEGAME_E_FOLDER_NOT_FOUND | SAVEGAME_E_FILE_NOT_FOUND);

			profileLoadProcessor->AddCompletedCallback(MakeCallback(this, &idProfileMgr::OnLoadSettingsCompleted, &profileLoadProcessor->GetParmsNonConst()));
			handle = session->GetSaveGameManager().ExecuteProcessor(profileLoadProcessor.get());
			profile->SetState(idPlayerProfile::LOADING);


		}
	}
	else {
		// If not able to save the profile, just change the state and leave
		if (!profile) {
			common->Warning("Not loading profile, profile is NULL.");
		}
		if (!saveGame_enable.GetBool()) {
			common->Warning("Skipping profile load because saveGame_enable = 0");
		}
	}
}

/*
========================
idProfileMgr::OnLoadSettingsCompleted
========================
*/
void idProfileMgr::OnLoadSettingsCompleted(idSaveLoadParms* parms) {





	// Don't process if error already detected
	if (parms->errorCode != SAVEGAME_E_NONE) {
		return;
	}

	// Serialize the loaded profile
	idFile_SaveGame** profileFileContainer = FindFromGenericPtr(parms->files, SAVEGAME_PROFILE_FILENAME);
	idFile_SaveGame* profileFile = profileFileContainer == NULL ? NULL : *profileFileContainer;

	bool foundProfile = profileFile != NULL && profileFile->Length() > 0;

	if (foundProfile) {
		idTempArray< byte > buffer(MAX_PROFILE_SIZE);

		// Serialize settings from this buffer
		profileFile->MakeReadOnly();
		unsigned int originalChecksum;
		profileFile->ReadBig(originalChecksum);

		int dataLength = profileFile->Length() - (int)sizeof(originalChecksum);
		profileFile->ReadBigArray(buffer.Ptr(), dataLength);

		// Validate the checksum before we let the game serialize the settings
		unsigned int checksum = MD5_BlockChecksum(buffer.Ptr(), dataLength);
		if (originalChecksum != checksum) {
			idLib::Warning("Checksum: 0x%08x, originalChecksum: 0x%08x, size = %d", checksum, originalChecksum, dataLength);
			parms->errorCode = SAVEGAME_E_CORRUPTED;
		}
		else {
			idBitMsg msg;
			msg.InitRead(buffer.Ptr(), (int)buffer.Size());
			idSerializer ser(msg, false);
			if (!profile->Serialize(ser)) {
				parms->errorCode = SAVEGAME_E_CORRUPTED;
			}
		}

	}
	else {
		parms->errorCode = SAVEGAME_E_FILE_NOT_FOUND;
	}
}