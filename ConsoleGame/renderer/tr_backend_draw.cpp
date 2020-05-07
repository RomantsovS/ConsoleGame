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
	if (!tr.updateFrame)
	{
		tr.console.clear();
		return;
	}

	// render the scene
	RB_DrawViewInternal(/*cmd->viewDef, stereoEye*/);

	tr.updateFrame = false;
}