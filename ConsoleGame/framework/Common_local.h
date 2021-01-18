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

	virtual void Init(int argc, const char * const * argv, const char *cmdline) override;
	virtual void Shutdown() override;
	virtual	void CreateMainMenu() override;
	virtual void Quit() override;

	virtual void Frame() override;

	virtual void Printf(const char* fmt, ...) override;
	virtual void VPrintf(const char* fmt, va_list arg) override;
	virtual void DPrintf(const char* fmt, ...) override;
	virtual void Warning(const char* fmt, ...) override;
	virtual void DWarning(const char* fmt, ...) override;
	virtual void Error(const char* fmt, ...) override;
	virtual void FatalError(const char* fmt, ...) override;

	virtual bool ProcessEvent(const sysEvent_t* event) override;
public:
	void RunGameAndDraw();
	void Draw();			// called by gameThread
private:
	errorParm_t com_errorEntered;
	bool com_shuttingDown;

	std::shared_ptr<idFile> logFile;
	bool log_file_closed;

	char errorMessage[MAX_PRINT_MSG_SIZE];

	// The main render world and sound world
	std::shared_ptr<idRenderWorld> renderWorld;
	
	std::string currentMapName;			// for checking reload on same level
	bool mapSpawned; // cleared on Stop()

	int gameFrame;			// Frame number of the local game
	double gameTimeResidual;	// left over msec from the last game frame

	size_t FPSupdateMilliseconds;
	int delayMilliseconds;

private:
	void CloseLogFile();
	void CleanupShell();
	
	void ExitMenu();
	bool MenuEvent(const sysEvent_t* event);

	void StartMenu(bool playIntro = false);
	void GuiFrameEvents();

	void ExecuteMapChange();
	void UnloadMap();

	void Stop(bool resetSession = true);
};

extern idCommonLocal commonLocal;

#endif