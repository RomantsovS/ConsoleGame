
#include "idlib/precompiled.h"

#include "tr_local.h"

void R_RenderView(std::shared_ptr<viewDef_t> parms) {
	tr.viewDef = parms;

	// identify all the visible portal areas, and create view lights and view entities
	// for all the the entityDefs and lightDefs that are in the visible portal areas
	parms->renderWorld->FindViewLightsAndEntities();

	// adds ambient surfaces and create any necessary interaction surfaces to add to the light lists
	R_AddModels();
}