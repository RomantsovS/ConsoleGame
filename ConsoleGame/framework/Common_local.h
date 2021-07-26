#ifndef COMMON_LOCAL_H
#define COMMON_LOCAL_H

enum errorParm_t {
	ERP_NONE,
	ERP_FATAL,						// exit the entire game with a popup window
	ERP_DROP,						// print to console and disconnect from game
	ERP_DISCONNECT					// don't kill server
};

#define	MAX_PRINT_MSG_SIZE	4096

class idRenderWorld;

class idCommonLocal : public idCommon
{
public:
	idCommonLocal();
	~idCommonLocal();
	idCommonLocal(const idCommonLocal&) = default;
	idCommonLocal& operator=(const idCommonLocal&) = default;
	idCommonLocal(idCommonLocal&&) = default;
	idCommonLocal& operator=(idCommonLocal&&) = default;

	void Init(int argc, const char * const * argv, const char *cmdline) override;
	void Shutdown() override;
	void CreateMainMenu() override;
	void Quit() override;

	void Frame() override;
	void UpdateScreen(bool captureToImage) override;
	void Printf(const char* fmt, ...) override;
	void VPrintf(const char* fmt, va_list arg) override;
	void DPrintf(const char* fmt, ...) override;
	void Warning(const char* fmt, ...) override;
	void DWarning(const char* fmt, ...) override;
	void Error(const char* fmt, ...) override;
	void FatalError(const char* fmt, ...) override;

	bool ProcessEvent(const sysEvent_t* event) override;
public:
	void RunGameAndDraw(size_t numGameFrames_);
	void Draw(); // called by gameThread

	// loads a map and starts a new game on it
	void StartNewGame(const std::string& mapName, bool devmap, int gameMode);
	//void LeaveGame();

	void QuitRequest() override { quit_requested = true; }
private:
	errorParm_t com_errorEntered;
	bool com_shuttingDown;
	bool quit_requested = false;

	std::shared_ptr<idFile> logFile;

	char errorMessage[MAX_PRINT_MSG_SIZE];

	// The main render world and sound world
	std::shared_ptr<idRenderWorld> renderWorld;
	
	std::string currentMapName;			// for checking reload on same level
	bool mapSpawned; // cleared on Stop()

	bool insideUpdateScreen;		// true while inside ::UpdateScreen()

	int gameFrame;			// Frame number of the local game
	double gameTimeResidual;	// left over msec from the last game frame

	size_t FPSupdateMilliseconds;
	int delayMilliseconds;

private:
	void InitCommands();
	void CloseLogFile();
	void CleanupShell();
	
	void ExitMenu();
	bool MenuEvent(const sysEvent_t* event);

	void StartMenu(bool playIntro = false);
	void GuiFrameEvents();

	// Meant to be used like:
	// while ( waiting ) { BusyWait(); }
	void BusyWait();
	bool WaitForSessionState(idSession::sessionState_t desiredState);

	void ExecuteMapChange();
	void UnloadMap();

	void Stop(bool resetSession = true);
};

extern idCommonLocal commonLocal;

#endif