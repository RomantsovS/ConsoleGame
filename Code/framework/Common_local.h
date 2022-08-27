#ifndef COMMON_LOCAL_H
#define COMMON_LOCAL_H

class idGameThread : public idSysThread {
public:
	idGameThread() :
		ret(),
		numGameFrames(),
		isClient()
	{}

	// the gameReturn_t is from the previous frame, the
	// new frame will be running in parallel on exit
	gameReturn_t RunGameAndDraw(int numGameFrames, bool isClient_, int startGameFrame);

private:
	int Run() override;

	gameReturn_t	ret;
	int				numGameFrames;
	bool			isClient;
};

enum errorParm_t {
	ERP_NONE,
	ERP_FATAL,						// exit the entire game with a popup window
	ERP_DROP,						// print to console and disconnect from game
	ERP_DISCONNECT					// don't kill server
};

#define	MAX_PRINT_MSG_SIZE	4096

class idCommonLocal : public idCommon {
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

	bool IsMultiplayer() override;
	bool IsServer() override;
	bool IsClient() override;

	bool ProcessEvent(const sysEvent_t* event) override;

	void OnStartHosting(idMatchParameters& parms) override;

	void InitializeMPMapsModes() override;
	const std::vector<mpMap_t>& GetMapList() const override { return mpGameMaps; }
public:
	void Draw(); // called by gameThread

	// loads a map and starts a new game on it
	void StartNewGame(const std::string& mapName, bool devmap, int gameMode);
	//void LeaveGame();

	void QuitRequest() noexcept override { quit_requested = true; }
public:
	std::string GetCurrentMapName() { return currentMapName; }
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

	std::vector<mpMap_t> mpGameMaps;

	idGameThread gameThread; // the game and draw code can be run in parallel

	size_t FPSupdateMilliseconds;
	int delayMilliseconds;

	gameReturn_t ret;

private:
	void InitCommands();
	void CloseLogFile();
	void CleanupShell();
	
	void ExitMenu();
	bool MenuEvent(const sysEvent_t* event);

	void StartMenu(bool playIntro = false);
	void GuiFrameEvents();

	void ProcessGameReturn(const gameReturn_t& ret);

	// Meant to be used like:
	// while ( waiting ) { BusyWait(); }
	void BusyWait();
	bool WaitForSessionState(idSession::sessionState_t desiredState);

	void ExecuteMapChange();
	void UnloadMap();

	void Stop(bool resetSession = true);

	void MoveToNewMap(const std::string& mapName, bool devmap);
};

extern idCommonLocal commonLocal;

#endif