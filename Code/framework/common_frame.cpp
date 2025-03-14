#include "idlib/precompiled.h"

#include "Common_local.h"

idCVar com_deltaTimeClamp(
    "com_deltaTimeClamp", "50", CVAR_INTEGER,
    "don't process more than this time in a single frame");
idCVar com_fixedTic("com_fixedTic", "0", CVAR_BOOL,
                    "run a single game frame per render frame");
idCVar com_noSleep("com_noSleep", "0", CVAR_BOOL,
                   "don't sleep if the game is running too fast");
idCVar com_smp("com_smp", "1", CVAR_BOOL | CVAR_SYSTEM | CVAR_NOCHEAT,
               "run the game and draw code in a separate thread");
idCVar timescale("timescale", "1", CVAR_SYSTEM | CVAR_FLOAT,
                 "Number of game frames to run per render frame", 0.001f,
                 100.0f);

/*
===============
idGameThread::Run

Run in a background thread for performance, but can also
be called directly in the foreground thread for comparison.
===============
*/
int idGameThread::Run() {
  if (numGameFrames == 0) {
    // Ensure there's no stale gameReturn data from a paused game
    ret = gameReturn_t();
  }

  if (isClient) {
    // run the game logic
    for (int i = 0; i < numGameFrames; i++) {
      if (userCmdMgr) {
        game->ClientRunFrame(*userCmdMgr, (i == numGameFrames - 1), ret);
      }
      /*if (ret.syncNextGameFrame || ret.sessionCommand[0] != 0) {
              break;
      }*/
    }
  } else {
    // run the game logic
    for (int i = 0; i < numGameFrames; i++) {
      if (userCmdMgr) {
        game->RunFrame(*userCmdMgr, ret);
      }
    }
  }

  // std::ostringstream oss;
  // oss << std::this_thread::get_id() << " idGameThread::Run()\n";
  // common->DPrintf(oss.str().c_str());

  // we should have consumed all of our usercmds
  if (userCmdMgr) {
    if (userCmdMgr->HasUserCmdForPlayer(game->GetLocalClientNum())) {
      // idLib::Printf("idGameThread::Run: didn't consume all usercmds\n");
    }
  }

  // commonLocal.frameTiming.finishGameTime = Sys_Microseconds();

  // SetThreadGameTime((commonLocal.frameTiming.finishGameTime -
  // commonLocal.frameTiming.startGameTime) / 1000);*/

  // build render commands and geometry
  commonLocal.Draw();

  /*commonLocal.frameTiming.finishDrawTime = Sys_Microseconds();

  SetThreadRenderTime((commonLocal.frameTiming.finishDrawTime -
  commonLocal.frameTiming.finishGameTime) / 1000);

  SetThreadTotalTime((commonLocal.frameTiming.finishDrawTime -
  commonLocal.frameTiming.startGameTime) / 1000);*/

  return 0;
}

/*
===============
idGameThread::RunGameAndDraw

===============
*/
gameReturn_t idGameThread::RunGameAndDraw(int numGameFrames_,
                                          idUserCmdMgr& userCmdMgr_,
                                          bool isClient_, int startGameFrame) {
  // this should always immediately return
  this->WaitForThread();

  // save the usercmds for the background thread to pick up
  userCmdMgr = &userCmdMgr_;

  isClient = isClient_;

  // grab the return value created by the last thread execution
  gameReturn_t latchedRet = ret;

  numGameFrames = numGameFrames_;

  // start the thread going
  if (com_smp.GetBool() == false) {
    // run it in the main thread so PIX profiling catches everything
    Run();
  } else {
    this->SignalWork();
  }

  // return the latched result while the thread runs in the background
  return latchedRet;
}

void idCommonLocal::Draw() {
  if (game && game->Shell_IsActive()) {
    const bool gameDraw = game->Draw(game->GetLocalClientNum());
    /*if (!gameDraw) {
            renderSystem->SetColor(colorBlack);
            renderSystem->DrawStretchPic(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0,
    0, 1, 1, whiteMaterial);
    }*/
    game->Shell_Render();
  } else if (mapSpawned) {
    bool gameDraw = false;
    // normal drawing for both single and multi player
    if (/*!com_skipGameDraw.GetBool() &&*/ Game()->GetLocalClientNum() >= 0) {
      // draw the game view
      int start = Sys_Milliseconds();
      if (game) {
        gameDraw = game->Draw(Game()->GetLocalClientNum());
      }
      int end = Sys_Milliseconds();
      time_gameDraw += (end - start);  // note time used for com_speeds
    }
    /*if (!gameDraw) {
            renderSystem->SetColor(colorBlack);
            renderSystem->DrawStretchPic(0, 0, 640, 480, 0, 0, 1, 1,
    whiteMaterial);
    }*/
  } else {
    // renderSystem->SetColor4(0, 0, 0, 1);
    // renderSystem->DrawStretchPic(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 1,
    // 1, whiteMaterial);
  }

  // draw the half console / notify console on top of everything
  console->Draw(false);
}

/*
===============
idCommonLocal::UpdateScreen

This is an out-of-sequence screen update, not the normal game rendering
===============
*/
void idCommonLocal::UpdateScreen(bool captureToImage) {
  if (insideUpdateScreen) {
    return;
  }
  insideUpdateScreen = true;

  // build all the draw commands without running a new game tic
  Draw();

  // get the GPU busy with new commands
  renderSystem->RenderCommandBuffers();

  insideUpdateScreen = false;
}

/*
================
idCommonLocal::ProcessGameReturn
================
*/
void idCommonLocal::ProcessGameReturn(const gameReturn_t& ret) {
  if (ret.sessionCommand[0]) {
    idCmdArgs args;

    args.TokenizeString(ret.sessionCommand, false);

    if (args.Argv(0) == "map") {
      MoveToNewMap(args.Argv(1), false);
    } else if (args.Argv(0) == "devmap") {
      MoveToNewMap(args.Argv(1), true);
    } else if (args.Argv(0) == "died") {
      if (!IsMultiplayer()) {
        game->Shell_Show(true);
      }
    }
  }
}

void idCommonLocal::Frame() {
  try {
    if (quit_requested) Quit();

    // This is the only place this is incremented
    idLib::frameNumber++;

    eventLoop->RunEventLoop();

    if (console->Active() || (game && game->InhibitControls())) {
      usercmdGen->InhibitUsercmd(inhibit_t::INHIBIT_SESSION, true);
    } else {
      usercmdGen->InhibitUsercmd(inhibit_t::INHIBIT_SESSION, false);
    }

    const bool pauseGame = !mapSpawned || (game && game->Shell_IsActive());

    // How many game frames to run
    int numGameFrames = 0;

    for (;;) {
      const int thisFrameTime = Sys_Milliseconds();
      static int lastFrameTime =
          thisFrameTime;  // initialized only the first time
      const int deltaMilliseconds = thisFrameTime - lastFrameTime;
      lastFrameTime = thisFrameTime;

      // if there was a large gap in time since the last frame, or the frame
      // rate is very very low, limit the number of frames we will run
      const int clampedDeltaMilliseconds =
          std::min(deltaMilliseconds, com_deltaTimeClamp.GetInteger());

      gameTimeResidual += clampedDeltaMilliseconds * timescale.GetFloat();

      // don't run any frames when paused
      if (pauseGame) {
        gameFrame++;
        gameTimeResidual = 0;
        break;
      }

      // debug cvar to force multiple game tics
      if (com_fixedTic.GetInteger() > 0) {
        numGameFrames = com_fixedTic.GetInteger();
        gameFrame += numGameFrames;
        gameTimeResidual = 0;
        break;
      }

      for (;;) {
        // How much time to wait before running the next frame,
        // based on com_engineHz
        const int frameDelay =
            FRAME_TO_MSEC(gameFrame + 1) - FRAME_TO_MSEC(gameFrame);
        if (gameTimeResidual < frameDelay) {
          break;
        }
        gameTimeResidual -= frameDelay;
        gameFrame++;
        numGameFrames++;
        // if there is enough residual left, we may run additional frames
      }

      if (numGameFrames > 0) {
        // ready to actually run them
        break;
      }

      // if we are vsyncing, we always want to run at least one game
      // frame and never sleep, which might happen due to scheduling issues
      // if we were just looking at real time.
      if (com_noSleep.GetBool()) {
        numGameFrames = 1;
        gameFrame += numGameFrames;
        gameTimeResidual = 0;
        break;
      }

      // not enough time has passed to run a frame, as might happen if
      // we don't have vsync on, or the monitor is running at 120hz while
      // com_engineHz is 60, so sleep a bit and check again
      Sys_Sleep(0);
      // std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // Update session and syncronize to the new session state after sleeping
    session->UpdateSignInManager();
    session->Pump();

    if (session->GetState() == idSession::sessionState_t::LOADING) {
      // If the session reports we should be loading a map, load it!
      ExecuteMapChange();
      return;
    } else if (session->GetState() != idSession::sessionState_t::INGAME &&
               mapSpawned) {
      // If the game is running, but the session reports we are not in a game,
      // disconnect This happens when a server disconnects us or we sign out
      // LeaveGame();
      return;
    }

    if (mapSpawned && !pauseGame) {
      if (IsClient()) {
        RunNetworkSnapshotFrame();
      }
    }

    // send frame and mouse events to active guis
    GuiFrameEvents();

    //--------------------------------------------
    // Prepare usercmds and kick off the game processing
    // in a background thread
    //--------------------------------------------

    // get the previous usercmd for bypassed head tracking transform
    const usercmd_t previousCmd = usercmdGen->GetCurrentUsercmd();

    // build a new usercmd
    usercmdGen->BuildCurrentUsercmd(0);
    if (pauseGame) {
      usercmdGen->Clear();
    }

    usercmd_t newCmd = usercmdGen->GetCurrentUsercmd();

    // Store server game time - don't let time go past last SS time in case we
    // are extrapolating
    /*if (IsClient()) {
            newCmd.serverGameMilliseconds =
    std::min(Game()->GetServerGameTimeMs(), Game()->GetSSEndTime());
    }
    else {
            newCmd.serverGameMilliseconds = Game()->GetServerGameTimeMs();
    }*/

    userCmdMgr.MakeReadPtrCurrentForPlayer(Game()->GetLocalClientNum());

    // Stuff a copy of this userCmd for each game frame we are going to run.
    // Ideally, the usercmds would be built in another thread so you could
    // still get 60hz control accuracy when the game is running slower.
    for (int i = 0; i < numGameFrames; i++) {
      // newCmd.clientGameMilliseconds = FRAME_TO_MSEC(gameFrame - numGameFrames
      // + i + 1);
      userCmdMgr.PutUserCmdForPlayer(game->GetLocalClientNum(), newCmd);
    }

    renderSystem->UpdateTimers();

    // start the game / draw command generation thread going in the background
    gameReturn_t ret = gameThread.RunGameAndDraw(
        numGameFrames, userCmdMgr, IsClient(), gameFrame - numGameFrames);

    // make sure the game / draw thread has completed
    // This may block if the game is taking longer than the render back end
    gameThread.WaitForThread();

    // std::ostringstream oss;
    // oss << std::this_thread::get_id() << " idCommonLocal::Frame()\n";
    // common->DPrintf(oss.str().c_str());

    renderSystem->RenderCommandBuffers();

    // Send local usermds to the server.
    // This happens after the game frame has run so that prediction data is up
    // to date.
    // SendUsercmds(Game()->GetLocalClientNum());

    // Now that we have an updated game frame, we can send out new snapshots to
    // our clients
    session->Pump();  // Pump to get updated usercmds to relay
    SendSnapshots();

    // process the game return for map changes, etc
    ProcessGameReturn(ret);

    // report timing information
    if (com_speeds.GetBool()) {
      static int lastTime = Sys_Milliseconds();
      int nowTime = Sys_Milliseconds();
      int com_frameMsec = nowTime - lastTime;
      lastTime = nowTime;
      Printf("frame:%d all:%3d gfr:%3d\n", idLib::frameNumber, com_frameMsec,
             time_gameFrame);
      time_gameFrame = 0;
      time_gameDraw = 0;
    }
  } catch (const std::exception& err) {
    common->Error(err.what());
  }
}
