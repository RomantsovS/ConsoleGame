#ifndef COMMON_LOCAL_H
#define COMMON_LOCAL_H

#include <memory>

#include "Common.h"
#include "RenderWorld.h"

enum errorParm_t {
	ERP_NONE,
	ERP_FATAL,						// exit the entire game with a popup window
	ERP_DROP,						// print to console and disconnect from game
	ERP_DISCONNECT					// don't kill server
};

class idCommonLocal : public idCommon
{
public:
	idCommonLocal();

	virtual void Init(int argc, const char * const * argv, const char *cmdline) override;
	virtual void Shutdown() override;
	virtual void Quit() override;

	virtual void Frame() override;

	bool IsGameRunning() const { return gameRunning; }
public:
	void Draw();			// called by gameThread
private:
	errorParm_t com_errorEntered;
	bool com_shuttingDown;

	// The main render world and sound world
	std::shared_ptr<idRenderWorld> renderWorld;

	bool mapSpawned; // cleared on Stop()

	size_t FPSupdateMilliseconds;
	int delayMilliseconds;

	bool gameRunning;

	void ExecuteMapChange();
};

extern idCommonLocal commonLocal;

#endif