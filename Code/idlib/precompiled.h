#ifndef IDLIB_PRECOMPILED_H_
#define IDLIB_PRECOMPILED_H_

#ifdef DEBUG
#ifdef _DEBUG
#define DBG_NEW new (_NORMAL_BLOCK, __FILE__, __LINE__)
// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
// allocations to be of _CLIENT_BLOCK type
#else
#define DBG_NEW new
#endif
#endif

#ifdef _WIN32
#include <SDKDDKVer.h>
#endif

#include <array>
#include <cstdarg>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <queue>
#include <random>
#include <sstream>
#include <string>
#include <vector>
#ifdef _WIN32
#include <direct.h>
#endif
#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <filesystem>
#include <gsl/gsl>
#include <mutex>
#include <optional>
#include <thread>

#include <boost/asio.hpp>

#include "sys/sys_assert.h"
#include "sys/sys_defines.h"
#include "sys/sys_includes.h"
#include "sys/sys_types.h"

#define ID_TIME_T \
  int64  // Signed because -1 means "File not found" and we don't want that to
         // compare > than any other time

// non-portable system services
#include "../sys/sys_public.h"

// id lib
#include "../idlib/Lib.h"

#include "sys/proflie.h"
#include "sys/sys_filesystem.h"

// framework
#include "../framework/CVarSystem.h"
#include "../framework/CmdSystem.h"
#include "../framework/Common.h"
#include "../framework/File.h"
#include "../framework/FileSystem.h"
#include "../framework/UsercmdGen.h"

// decls
#include "../framework/DeclManager.h"
#include "../framework/DeclEntityDef.h"
#include "../framework/DeclParticle.h"

// renderer
#include "../renderer/Image.h"
#include "../renderer/Material.h"
#include "../renderer/Model.h"
#include "../renderer/ModelManager.h"
#include "../renderer/RenderSystem.h"
#include "../renderer/RenderWorld.h"
#include "../renderer/Screen.h"

#include "../SWF/SWF.h"

// collision detection system
#include "../cm/CollisionModel.h"

// game
#include "../d3xp/Game.h"

// Session / Network
#include "../sys/PacketProcessor.h"
#include "../sys/Snapshot.h"
#include "../sys/SnapshotProcessor.h"

#include "../sys/sys_session.h"

// framework
#include "../framework/Console.h"
#include "../framework/EditField.h"
#include "../framework/EventLoop.h"
#include "../framework/KeyInput.h"

#undef max
#undef min

#ifdef _MSC_VER
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

#endif
