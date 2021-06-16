#ifndef IDLIB_PRECOMPILED_H_
#define IDLIB_PRECOMPILED_H_

#include <string>
#include <memory>
#include <vector>
#include <array>
#include <list>
#include <random>
#include <iostream>
#include <fstream>
#include <sstream>
#include <direct.h>

#include "sys/sys_defines.h"
#include "sys/sys_includes.h"
#include "sys/sys_assert.h"
#include "sys/sys_types.h"

#define ID_TIME_T int64 // Signed because -1 means "File not found" and we don't want that to compare > than any other time

// non-portable system services
#include "../sys/sys_public.h"

// id lib
#include "../idlib/Lib.h"

#include "sys/sys_filesystem.h"

// framework
#include "../framework/CmdSystem.h"
#include "../framework/CVarSystem.h"
#include "../framework/Common.h"
#include "../framework/File.h"
#include "../framework/FileSystem.h"
#include "../framework/UsercmdGen.h"

// decls
#include "../framework/DeclManager.h"
#include "../framework/DeclEntityDef.h"

// renderer
#include "../renderer/Screen.h"
#include "../renderer/Model.h"
#include "../renderer/ModelManager.h"
#include "../renderer/RenderSystem.h"
#include "../renderer/RenderWorld.h"

#include "../SWF/SWF.h"

// collision detection system
#include "../cm/CollisionModel.h"

// game
#include "../d3xp/Game.h"

#include "../sys/sys_profile.h"
#include "../sys/sys_localuser.h"
#include "../sys/sys_session.h"

// framework
#include "../framework/EventLoop.h"
#include "../framework/KeyInput.h"
#include "../framework/EditField.h"
#include "../framework/Console.h"

#endif
