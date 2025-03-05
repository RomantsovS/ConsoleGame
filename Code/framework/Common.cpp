#include "idlib/precompiled.h"

#include "Common_local.h"

idCVar com_allowConsole("com_allowConsole", "1",
                        CVAR_BOOL | CVAR_SYSTEM | CVAR_INIT,
                        "allow toggling console with the tilde key");
idCVar com_speeds("com_speeds", "0", CVAR_BOOL | CVAR_SYSTEM | CVAR_NOCHEAT,
                  "show engine timings");
idCVar com_showFPS("com_showFPS", "1",
                   CVAR_BOOL | CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_NOCHEAT,
                   "show frames rendered per second");

idCVar com_engineHz("com_engineHz", "60", CVAR_FLOAT | CVAR_ARCHIVE,
                    "Frames per second the engine runs at", 10.0f, 1024.0f);
long long com_engineHz_numerator = 100LL * 1000LL;
long long com_engineHz_denominator = 100LL * 60LL;

idCommonLocal commonLocal;
idCommon* common = &commonLocal;

bool isCommonExists = false;

idCommonLocal::idCommonLocal() : readSnapshotIndex(0), writeSnapshotIndex(0) {
  snapCurrent.localTime = -1;
  snapPrevious.localTime = -1;
  snapCurrent.serverTime = -1;
  snapPrevious.serverTime = -1;
  snapTimeBuffered = 0.0f;
  effectiveSnapRate = 0.0f;
  totalBufferedTime = 0;
  totalRecvTime = 0;

  com_errorEntered = ERP_NONE;
  com_shuttingDown = false;

  logFile = nullptr;

  renderWorld = nullptr;

  gameFrame = 0;
  gameTimeResidual = 0;

  nextSnapshotSendTime = 0;
  // nextUsercmdSendTime = 0;

  isCommonExists = true;
}

idCommonLocal::~idCommonLocal() { isCommonExists = false; }

/*
==================
idCommonLocal::Quit
==================
*/
void idCommonLocal::Quit() {
  // don't try to shutdown if we are in a recursive error
  if (!com_errorEntered) {
    Shutdown();
  }

  isCommonExists = false;

  Sys_Quit();
}

/*
=================
idCommonLocal::UnloadGameDLL
=================
*/
void idCommonLocal::CleanupShell() {
  if (game != nullptr) {
    game->Shell_Cleanup();
  }
}

void idCommonLocal::Init(int argc, const char* const* argv,
                         const char* cmdline) {
  try {
    // set interface pointers used by idLib
    idLib::common = common;

    Printf(va("Command line: %s\n", cmdline).c_str());

    // init console command system
    cmdSystem->Init();

    // init CVar system
    cvarSystem->Init();

    // register all static CVars
    idCVar::RegisterStaticVars();

    // initialize key input/binding, done early so bind command exists
    idKeyInput::Init();

    // init the console so we can take prints
    console->Init();

    // initialize the file system
    fileSystem->Init();

    // initialize the declaration manager
    declManager->Init();

    // init journaling, etc
    eventLoop->Init();

    InitCommands();

    // exec the startup scripts
    cmdSystem->BufferCommandText(CMD_EXEC_APPEND, "exec default.cfg\n");

    // run cfg execution
    cmdSystem->ExecuteCommandBuffer();

    com_engineHz_denominator =
        100LL * static_cast<long long>(com_engineHz.GetFloat());

    // initialize the renderSystem data structures
    renderSystem->Init();

    // spawn the game thread, even if we are going to run without SMP
    // one meg stack, because it can parse decls from gui surfaces
    // (unfortunately) use a lower priority so job threads can run on the same
    // core
    gameThread.StartWorkerThread("Game/Draw");
    // boost this thread's priority, so it will prevent job threads from running
    // while the render back end still has work to do

    // init the user command input code
    usercmdGen->Init();

    game->Init();

    // the same idRenderWorld will be used for all games
    // and demos, insuring that level specific models
    // will be freed
    renderWorld = renderSystem->AllocRenderWorld();

    // init the session
    session->Initialize();

    InitializeMPMapsModes();

    CreateMainMenu();

    StartMenu(true);

    delayMilliseconds = 100;
    FPSupdateMilliseconds = 1000;

    Printf("--- Common Initialization Complete ---\n");

    cmdSystem->BufferCommandText(CMD_EXEC_APPEND, "exec autoexec.cfg\n");

    // run cfg execution
    cmdSystem->ExecuteCommandBuffer();
  } catch (const std::exception& msg) {
    Sys_Error("Error during initialization %s", msg.what());
  }
}

/*
=================
idCommonLocal::Shutdown
=================
*/
void idCommonLocal::Shutdown() {
  if (com_shuttingDown) {
    return;
  }
  com_shuttingDown = true;

  Printf("Stop();\n");
  Stop();

  Printf("CleanupShell();\n");
  CleanupShell();

  Printf("delete renderWorld;\n");
  renderWorld = nullptr;

  // shut down the session
  Printf("session->Shutdown();\n");
  session->Shutdown();

  // shut down the user command input code
  Printf("usercmdGen->Shutdown();\n");
  usercmdGen->Shutdown();

  // shut down the event loop
  Printf("eventLoop->Shutdown();\n");
  eventLoop->Shutdown();

  // shutdown the decl manager
  Printf("declManager->Shutdown();\n");
  declManager->Shutdown();

  // shut down the renderSystem
  Printf("renderSystem->Shutdown();\n");
  renderSystem->Shutdown();

  // unload the game dll
  Printf("UnloadGameDLL();\n");
  // shut down the game object
  if (game) {
    game->Shutdown();
  }

  // only shut down the log file after all output is done
  Printf("CloseLogFile();\n");
  CloseLogFile();

  // shut down the file system
  printf("fileSystem->Shutdown( false );\n");
  fileSystem->Shutdown(false);

  // shut down the console
  printf("console->Shutdown();\n");
  console->Shutdown();

  // shut down the key system
  printf("idKeyInput::Shutdown();\n");
  idKeyInput::Shutdown();

  // shut down the cvar system
  printf("cvarSystem->Shutdown();\n");
  cvarSystem->Shutdown();

  // shut down the console command system
  printf("cmdSystem->Shutdown();\n");
  cmdSystem->Shutdown();
}

/*
========================
idCommonLocal::CreateMainMenu
========================
*/
void idCommonLocal::CreateMainMenu() {
  if (game != nullptr) {
    // note which media we are going to need to load
    renderSystem->BeginLevelLoad();

    // create main inside an "empty" game level load - so assets get
    // purged automatically when we transition to a "real" map
    game->Shell_CreateMenu(false);
    game->Shell_Show(true);
    game->Shell_SyncWithSession();

    // load
    renderSystem->EndLevelLoad();
  }
}

void idCommonLocal::Stop(bool resetSession) {
  // clear mapSpawned and demo playing flags
  UnloadMap();

  // drop all guis
  ExitMenu();

  if (resetSession) {
    session->QuitMatchToTitle();
  }
}

/*
===============
idCommonLocal::BusyWait
===============
*/
void idCommonLocal::BusyWait() {
  const bool captureToImage = false;
  UpdateScreen(captureToImage);

  session->UpdateSignInManager();
  session->Pump();
}

/*
===============
idCommonLocal::WaitForSessionState
===============
*/
bool idCommonLocal::WaitForSessionState(
    idSession::sessionState_t desiredState) {
  if (session->GetState() == desiredState) {
    return true;
  }

  while (true) {
    BusyWait();

    idSession::sessionState_t sessionState = session->GetState();
    if (sessionState == desiredState) {
      return true;
    }
    if (sessionState != idSession::sessionState_t::LOADING &&
        sessionState != idSession::sessionState_t::SEARCHING &&
        sessionState != idSession::sessionState_t::CONNECTING &&
        sessionState != idSession::sessionState_t::BUSY &&
        sessionState != desiredState) {
      return false;
    }

    Sys_Sleep(10);
  }
}

/*
===============
idCommonLocal::ProcessEvent
===============
*/
bool idCommonLocal::ProcessEvent(const sysEvent_t* event) {
  // hitting escape anywhere brings up the menu
  if (game && game->IsInGame()) {
    if (event->evType == SE_KEY && event->evValue2 == 1 &&
        (event->evValue == static_cast<int>(keyNum_t::K_ESCAPE))) {
      if (!game->Shell_IsActive()) {
        // menus / etc
        if (MenuEvent(event)) {
          return true;
        }

        console->Close();

        StartMenu();
        return true;
      } else {
        console->Close();

        // menus / etc
        if (MenuEvent(event)) {
          return true;
        }
      }
    }
  }

  // let the pull-down console take it if desired
  if (console->ProcessEvent(event, false)) {
    return true;
  }

  // menus / etc
  if (MenuEvent(event)) {
    return true;
  }

  // if we aren't in a game, force the console to take it
  if (!mapSpawned) {
    console->ProcessEvent(event, true);
    return true;
  }

  // in game, exec bindings for all key downs
  if (event->evType == SE_KEY && event->evValue2 == 1) {
    idKeyInput::ExecKeyBinding(event->evValue);
    return true;
  }

  return false;
}
