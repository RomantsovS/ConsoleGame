#ifndef D3XP_FRAMEWORK_CMDSYSTEM_H_
#define D3XP_FRAMEWORK_CMDSYSTEM_H_

#include "../idlib/sys/sys_types.h"

// command flags
enum cmdFlags_t  {
	CMD_FL_ALL = -1,
	CMD_FL_CHEAT = BIT(0),	// command is considered a cheat
	CMD_FL_SYSTEM = BIT(1),	// system command
	CMD_FL_RENDERER = BIT(2),	// renderer command
	CMD_FL_SOUND = BIT(3),	// sound command
	CMD_FL_GAME = BIT(4),	// game command
	CMD_FL_TOOL = BIT(5)	// tool command
};

void Cmd_EntityList_f();

#endif