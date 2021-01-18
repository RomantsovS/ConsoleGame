#pragma hdrstop
#include "../idlib/precompiled.h"

#include "tr_local.h"

const size_t update_frame_time = 50;
const size_t update_info_time = 200;

/*
==================
RB_DrawViewInternal
==================
*/
void RB_DrawViewInternal(/*const viewDef_t * viewDef, const int stereoEye*/) {
	if (tr.update_frame)
	{
		tr.Display();

		tr.update_frame = false;
	}

	if (com_showFPS.GetBool()) {
		DrawFPS();
	}

	if (tr.update_info)
	{
		tr.screen.clearTextInfo();

		// draw the half console / notify console on top of everything
		console->Draw(false);

	//-------------------------------------------------
	// render debug tools
	//-------------------------------------------------
		RB_RenderDebugTools();
		tr.update_info = false;
	}

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
}

/*
==================
RB_DrawView

StereoEye will always be 0 in mono modes, or -1 / 1 in stereo modes.
If the view is a GUI view that is repeated for both eyes, the viewDef.stereoEye value
is 0, so the stereoEye parameter is not always the same as that.
==================
*/
void RB_DrawView(/*const void *data, const int stereoEye*/) {
	// render the scene
	RB_DrawViewInternal(/*cmd->viewDef, stereoEye*/);
}

/*
====================
RB_ExecuteBackEndCommands

This function will be called syncronously if running without
smp extensions, or asyncronously by another thread.
====================
*/
void RB_ExecuteBackEndCommands() {
	RB_DrawView();
}
