#ifndef IMAGE_H_
#define IMAGE_H_

class ModelPixel;

class idImage {
public:
	idImage(const std::string& name);

	const std::string& GetName() const { return imgName; }

	void MakeDefault();	// fill with a grid pattern

	void ActuallyLoadImage(bool fromBackEnd);

	bool IsLoaded() const { return texnum != -1; }

	std::vector<ModelPixel>& GetPixels() { return pixels; }
private:
	friend class idImageManager;

	// parameters that define this image
	std::string imgName; // game path, including extension (except for cube maps), may be an image program
	void (*generatorFunction)(gsl::not_null<idImage*> image) = nullptr;	// NULL for files

	std::vector<ModelPixel> pixels;
	int texnum = -1;
};

inline idImage::idImage(const std::string& name) : imgName(name) {
}

class idImageManager {
public:

	idImageManager() {
	}

	void Init();
	void Shutdown();

	// If the exact combination of parameters has been asked for already, an existing
	// image will be returned, otherwise a new image will be created.
	// Be careful not to use the same image file with different filter / repeat / etc parameters
	// if possible, because it will cause a second copy to be loaded.
	// If the load fails for any reason, the image will be filled in with the default
	// grid pattern.
	// Will automatically execute image programs if needed.
	std::shared_ptr<idImage> ImageFromFile(const std::string& name);

	// The callback will be issued immediately, and later if images are reloaded or vid_restart
	// The callback function should call one of the idImage::Generate* functions to fill in the data
	std::shared_ptr<idImage> ImageFromFunction(const std::string& name, void (*generatorFunction)(gsl::not_null<idImage*> image));

	// built-in images
	void CreateIntrinsicImages();
	std::shared_ptr<idImage> defaultImage;

	std::shared_ptr<idImage> AllocImage(const std::string& name);

	std::vector<std::shared_ptr<idImage>> images;

};

extern idImageManager* globalImages;		// pointer to global list for the rest of the system

#endif