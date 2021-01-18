#ifndef RENDER_MODEL_H
#define RENDER_MODEL_H

class ModelPixel
{
public:
	ModelPixel(Vector2 pos, Screen::Pixel pixel);

	Vector2 origin;

	Screen::Pixel screenPixel;
};

enum dynamicModel_t {
	DM_STATIC,		// never creates a dynamic model
	DM_CACHED,		// once created, stays constant until the entity is updated (animating characters)
	DM_CONTINUOUS	// must be recreated for every single view (time dependent things like particles)
};

class idRenderModel
{
public:	
	virtual ~idRenderModel() {}

	// Loads static models only, dynamic models must be loaded by the modelManager
	virtual void				InitFromFile(std::string fileName) = 0;

	// this is used for dynamically created surfaces, which are assumed to not be reloadable.
	// It can be called again to clear out the surfaces of a dynamic model for regeneration.
	virtual void				InitEmpty(const std::string fileName) = 0;

	// frees all the data, but leaves the class around for dangling references,
	// which can regenerate the data with LoadModel()
	virtual void				PurgeModel() = 0;

	// used for initial loads, reloadModel, and reloading the data of purged models
	// Upon exit, the model will absolutely be valid, but possibly as a default model
	virtual void				LoadModel() = 0;

	// internal use
	virtual bool				IsLoaded() = 0;
	virtual void				SetLevelLoadReferenced(bool referenced) = 0;
	virtual bool				IsLevelLoadReferenced() = 0;

	// returns the name of the model
	virtual const std::string Name() const = 0;

	// returns the number of surfaces
	virtual int					NumSurfaces() const = 0;

	// get a pointer to a surface
	virtual const ModelPixel &Surface(int surfaceNum) const = 0;

	// models parsed from inside map files or dynamically created cannot be reloaded by
	// reloadmodels
	virtual bool				IsReloadable() const = 0;

	// md3, md5, particles, etc
	virtual dynamicModel_t		IsDynamicModel() const = 0;

	virtual Screen::ConsoleColor GetColor() const = 0;
	virtual void SetColor(Screen::ConsoleColor col) = 0;
};

#endif