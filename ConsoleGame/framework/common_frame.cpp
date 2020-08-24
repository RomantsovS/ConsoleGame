#include "Common_local.h"
#include "..//d3xp/Game.h"
#include "../renderer/tr_local.h"
#include "UsercmdGen.h"
#include "EventLoop.h"

constexpr size_t update_frame_time = 50;
constexpr size_t update_info_time = 200;

idCVar com_deltaTimeClamp("com_deltaTimeClamp", "50", CVAR_INTEGER, "don't process more than this time in a single frame");
idCVar com_fixedTic("com_fixedTic", "0", CVAR_BOOL, "run a single game frame per render frame");
idCVar com_noSleep("com_noSleep", "0", CVAR_BOOL, "don't sleep if the game is running too fast");
idCVar timescale("timescale", "1", CVAR_SYSTEM | CVAR_FLOAT, "Number of game frames to run per render frame", 0.001f, 100.0f);

void idCommonLocal::Frame()
{
	/*unsigned key = 0;

	if (tr.screen.readInput(key))
	{
		std::string text_input;

		switch (key)
		{
		case 27:
			tr.screen.setStdOutputBuffer();

			tr.screen.writeConsoleOutput("enter Q to quit or any key to continue: ");

			text_input = tr.screen.waitConsoleInput();

			if (text_input == "Q" || text_input == "q")
			{
				Quit();
			}
			else if (text_input == "listEntities")
			{
				Cmd_EntityList_f();
			}
		default:
			;
		}
		tr.screen.clearConsoleOutut();
		tr.screen.setDrawOutputBuffer();
	}*/

	try
	{
		const bool pauseGame = !mapSpawned;

		eventLoop->RunEventLoop();

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

		if (!mapSpawned)
			ExecuteMapChange();

		usercmdGen->BuildCurrentUsercmd(0);

		if (pauseGame) {
			usercmdGen->Clear();
		}

		usercmd_t newCmd = usercmdGen->GetCurrentUsercmd();

		Draw();
	}
	catch (std::exception& err)
	{
		common->Error(err.what());
	}
}

void idCommonLocal::Draw()
{
	if (game)
	{		
		static auto prev_frame_update_time = Sys_Milliseconds();
		static auto prev_info_update_time = prev_frame_update_time;

		auto t = Sys_Milliseconds();

		if (t - prev_frame_update_time > update_frame_time) {
			tr.update_frame = true;
			prev_frame_update_time = t;
		}

		if (t - prev_info_update_time > update_info_time) {
			tr.update_info = true;
			prev_info_update_time = t;
		}

		game->RunFrame();

		game->Draw(0);

		RB_DrawView();
	}
}