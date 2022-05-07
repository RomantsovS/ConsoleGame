
#include "idlib/precompiled.h"

#include "tr_local.h"

/*
===============
ActuallyLoadImage

Absolutely every image goes through this path
On exit, the idImage will have a valid OpenGL texture number that can be bound
===============
*/
void idImage::ActuallyLoadImage(bool fromBackEnd) {

	// if we don't have a rendering context yet, just return
	/*if (!R_IsInitialized()) {
		return;
	}*/

	// this is the ONLY place generatorFunction will ever be called
	if (generatorFunction) {
		generatorFunction(this);
		return;
	}

	BMP bmp(imgName);

	if (bmp.data.empty())
		return;

	ConvertBMPToModelSurfaces(bmp, pixels);
}