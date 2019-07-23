#ifndef COMMON_H
#define COMMON_H

class idCommon
{
public:
	virtual ~idCommon() {}

	// Initialize everything.
	// if the OS allows, pass argc/argv directly (without executable name)
	// otherwise pass the command line in a single string (without executable name)
	virtual void				Init(int argc, const char * const * argv, const char *cmdline) = 0;

	// Shuts down everything.
	virtual void				Shutdown() = 0;

	// Shuts down everything.
	virtual void				Quit() = 0;

	// Called repeatedly as the foreground thread for rendering and game logic.
	virtual void				Frame() = 0;
};

extern idCommon * common;

#endif