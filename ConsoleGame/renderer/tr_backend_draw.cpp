#pragma hdrstop
#include <precompiled.h>

#include "tr_local.h"

/*
==================
RB_DrawViewInternal
==================
*/
void RB_DrawViewInternal(/*const viewDef_t * viewDef, const int stereoEye*/) {
	//-------------------------------------------------
	// render debug tools
	//-------------------------------------------------
	RB_RenderDebugTools();

	if (tr.update_frame) {
		tr.Display();

		tr.update_frame = false;
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
