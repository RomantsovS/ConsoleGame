#include "../idlib/precompiled.h"
#pragma hdrstop

#include "Common_local.h"

idCVar com_deltaTimeClamp("com_deltaTimeClamp", "50", CVAR_INTEGER, "don't process more than this time in a single frame");
idCVar com_fixedTic("com_fixedTic", "0", CVAR_BOOL, "run a single game frame per render frame");
idCVar com_noSleep("com_noSleep", "0", CVAR_BOOL, "don't sleep if the game is running too fast");
idCVar timescale("timescale", "1", CVAR_SYSTEM | CVAR_FLOAT, "Number of game frames to run per render frame", 0.001f, 100.0f);

void idCommonLocal::Frame() {
	try
	{
		if (quit_requested)
			Quit();

		eventLoop->RunEventLoop();

		const bool pauseGame = !mapSpawned || (game && game->Shell_IsActive());

		// How many game frames to run
		int numGameFrames = 0;

		for (;;) {
			const int thisFrameTime = Sys_Milliseconds();
			static int lastFrameTime = thisFrameTime;	// initialized only the first time
			const int deltaMilliseconds = thisFrameTime - lastFrameTime;
			lastFrameTime = thisFrameTime;

			// if there was a large gap in time since the last frame, or the frame
			// rate is very very low, limit the number of frames we will run
			const int clampedDeltaMilliseconds = min(deltaMilliseconds, com_deltaTimeClamp.GetInteger());

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

			for (;; ) {
				// How much time to wait before running the next frame,
				// based on com_engineHz
				const int frameDelay = FRAME_TO_MSEC(gameFrame + 1) - FRAME_TO_MSEC(gameFrame);
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
		}

		// Update session and syncronize to the new session state after sleeping
		session->UpdateSignInManager();

		if (session->GetState() == idSession::sessionState_t::LOADING) {
			// If the session reports we should be loading a map, load it!
			ExecuteMapChange();
			return;
		}
		else if (session->GetState() != idSession::sessionState_t::INGAME && mapSpawned) {
			// If the game is running, but the session reports we are not in a game, disconnect
			// This happens when a server disconnects us or we sign out
			//LeaveGame();
			return;
		}
		if (session->GetState() == idSession::sessionState_t::INGAME) {
			if (gameFrame == 50)
				StartMenu();
			if (gameFrame == 51) {
				sysEvent_t ev;
				ev.evType = SE_KEY;
				ev.evValue = 27;
				ev.evValue2 = 1;
				ProcessEvent(&ev);
			}
		}

		// send frame and mouse events to active guis
		GuiFrameEvents();

		usercmdGen->BuildCurrentUsercmd(0);

		if (pauseGame) {
			usercmdGen->Clear();
		}

		//usercmd_t newCmd = usercmdGen->GetCurrentUsercmd();

		RunGameAndDraw(numGameFrames);

		renderSystem->RenderCommandBuffers();
	}
	catch (const std::exception& err)
	{
		common->Error(err.what());
	}
}

void idCommonLocal::RunGameAndDraw(size_t numGameFrames_) {
	for(size_t i = 0; i < numGameFrames_; ++i)
		game->RunFrame();

	Draw();
}

void idCommonLocal::Draw() {
	if (game && game->Shell_IsActive()) {
		const bool gameDraw = game->Draw(game->GetLocalClientNum());
		/*if (!gameDraw) {
			renderSystem->SetColor(colorBlack);
			renderSystem->DrawStretchPic(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 1, 1, whiteMaterial);
		}*/
		game->Shell_Render();
	}
	else if (mapSpawned) {
		if (game) {
			game->Draw(0);
		}
		else {
			//renderSystem->SetColor4(0, 0, 0, 1);
			//renderSystem->DrawStretchPic(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 1, 1, whiteMaterial);
		}
	}
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
