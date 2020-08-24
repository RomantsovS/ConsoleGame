#include "tr_local.h"
#include "../framework/Console.h"

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