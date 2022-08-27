#ifndef COMMON_H
#define COMMON_H

extern idCVar com_engineHz;
extern long long com_engineHz_numerator;
extern long long com_engineHz_denominator;

// Returns the msec the frame starts on
inline int FRAME_TO_MSEC(long long frame) noexcept {
	return (int)((frame * com_engineHz_numerator) / com_engineHz_denominator);
}

class idMatchParameters;

extern idCVar com_allowConsole;
extern idCVar com_showFPS;

struct mpMap_t {

	void operator=(const mpMap_t& src) {
		mapFile = src.mapFile;
		mapName = src.mapName;
	}

	std::string mapFile;
	std::string mapName;
};

class idCommon {
public:
	idCommon() = default;
	virtual ~idCommon() = default;
	idCommon(const idCommon&) = default;
	idCommon& operator=(const idCommon&) = default;
	idCommon(idCommon&&) = default;
	idCommon& operator=(idCommon&&) = default;

	// Initialize everything.
	// if the OS allows, pass argc/argv directly (without executable name)
	// otherwise pass the command line in a single string (without executable name)
	virtual void				Init(int argc, const char * const * argv, const char *cmdline) = 0;

	// Shuts down everything.
	virtual void				Shutdown() = 0;

	virtual	void				CreateMainMenu() = 0;

	// Shuts down everything.
	virtual void				Quit() = 0;

	// Called repeatedly as the foreground thread for rendering and game logic.
	virtual void				Frame() = 0;

	// Redraws the screen, handling games, guis, console, etc
	// in a modal manner outside the normal frame loop
	virtual void				UpdateScreen(bool captureToImage) = 0;

	// Prints message to the console, which may cause a screen update if com_refreshOnPrint is set.
	virtual void				Printf(const char* fmt, ...) = 0;
	// Same as Printf, with a more usable API - Printf pipes to this.
	virtual void				VPrintf(const char* fmt, va_list args) = 0;

	// Prints message that only shows up if the "developer" cvar is set,
	// and NEVER forces a screen update, which could cause reentrancy problems.
	virtual void				DPrintf(const char* fmt, ...) = 0;

	// Prints WARNING %s message and adds the warning message to a queue for printing later on.
	virtual void				Warning(const char* fmt, ...) = 0;

	// Prints WARNING %s message in yellow that only shows up if the "developer" cvar is set.
	virtual void				DWarning(const char* fmt, ...) = 0;

	// Issues a C++ throw. Normal errors just abort to the game loop,
	// which is appropriate for media or dynamic logic errors.
	virtual void				Error(const char* fmt, ...) = 0;

	// Fatal errors quit all the way to a system dialog box, which is appropriate for
	// static internal errors or cases where the system may be corrupted.
	virtual void				FatalError(const char* fmt, ...) = 0;

	// Returns true if a multiplayer game is running.
	// CVars and commands are checked differently in multiplayer mode.
	virtual bool IsMultiplayer() = 0;
	virtual bool IsServer() = 0;
	virtual bool IsClient() = 0;

	// Processes the given event.
	virtual	bool				ProcessEvent(const sysEvent_t* event) = 0;

	virtual void OnStartHosting(idMatchParameters& parms) = 0;

	virtual void InitializeMPMapsModes() = 0;
	virtual const std::vector<mpMap_t>& GetMapList() const = 0;

	virtual void QuitRequest() = 0;
};

extern idCommon * common;
extern bool isCommonExists;

#endif