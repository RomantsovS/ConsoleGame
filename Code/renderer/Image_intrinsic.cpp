
#include "idlib/precompiled.h"

#include "tr_local.h"

const size_t DEFAULT_SIZE = 16;

/*
==================
idImage::MakeDefault

the default image will be grey with a white box outline
to allow you to see the mapping coordinates on a surface
==================
*/
void idImage::MakeDefault() noexcept {
	height = 4;
	width = 4;
	pixels.resize(height * width);
	for (int j = 0; j < height; ++j) {
		for (int i = 0; i < height; ++i) {
			pixels[j * width + i] = ModelPixel(Vector2(i, j), { '?', colorYellow });
		}
	}
}

static void R_DefaultImage(gsl::not_null<idImage*> image) {
	image->MakeDefault();
}

/*
================
idImageManager::CreateIntrinsicImages
================
*/
void idImageManager::CreateIntrinsicImages() {
	// create built in images
	defaultImage = ImageFromFunction("_default", R_DefaultImage);
}