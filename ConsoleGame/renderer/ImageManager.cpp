#pragma hdrstop
#include <precompiled.h>

#include "tr_local.h"

// do this with a pointer, in case we want to make the actual manager
// a private virtual subclass
idImageManager	imageManager;
idImageManager* globalImages = &imageManager;

const size_t MAX_IMAGE_NAME = 256;

/*
==============
AllocImage

Allocates an idImage, adds it to the list,
copies the name, and adds it to the hash chain.
==============
*/
std::shared_ptr<idImage> idImageManager::AllocImage(const std::string& name) {
	if (name.size() >= MAX_IMAGE_NAME) {
		common->Error("idImageManager::AllocImage: \"%s\" is too long\n", name.c_str());
	}

	std::shared_ptr<idImage> image = std::make_shared<idImage>(name);

	images.push_back(image);

	return image;
}

/*
==================
ImageFromFunction

Images that are procedurally generated are allways specified
with a callback which must work at any time, allowing the OpenGL
system to be completely regenerated if needed.
==================
*/
std::shared_ptr<idImage> idImageManager::ImageFromFunction(const std::string& _name, void (*generatorFunction)(idImage* image)) {
	std::string name = _name;

	// see if the image already exists
	for (auto& image : images) {
		if (name == image->GetName()) {
			if (image->generatorFunction != generatorFunction) {
				common->DPrintf("WARNING: reused image %s with mixed generators\n", name.c_str());
			}
			return image;
		}
	}

	// create the image and issue the callback
	std::shared_ptr<idImage> image = AllocImage(name);

	image->generatorFunction = generatorFunction;

	// check for precompressed, load is from the front end
	image->ActuallyLoadImage(false);

	return image;
}

/*
===============
ImageFromFile

Finds or loads the given image, always returning a valid image pointer.
Loading of the image may be deferred for dynamic loading.
==============
*/
std::shared_ptr<idImage> idImageManager::ImageFromFile(const std::string& _name) {

	if (_name.empty() || _name == "_default") {
		return globalImages->defaultImage;
	}

	std::string name = _name;
	//
	// see if the image is already loaded, unless we
	// are in a reloadImages call
	//
	for(auto &image : images) {
		if (name == image->GetName()) {
			// the built in's, like _white and _flat always match the other options
			if (name[0] == '_') {
				return image;
			}

			if (!image->IsLoaded()) {
				image->ActuallyLoadImage(false);	// load is from front end
			}
			return image;
		}
	}

	//
	// create a new image
	//
	std::shared_ptr<idImage> image = AllocImage(name);

	image->ActuallyLoadImage(false);	// load is from front end

	return image;
}

/*
===============
Init
===============
*/
void idImageManager::Init() {

	images.reserve(10);

	CreateIntrinsicImages();
}

/*
===============
Shutdown
===============
*/
void idImageManager::Shutdown() {
	images.clear();

}