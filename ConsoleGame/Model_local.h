#ifndef MODEL_LOCAL_H
#define MODEL_LOCAL_H

#include "Model.h"

class idRenderModelStatic : public idRenderModel
{
public:
	idRenderModelStatic();
	virtual ~idRenderModelStatic();

	virtual void InitFromFile(std::string fileName);

	virtual void PurgeModel();
	virtual void LoadModel();
	virtual bool IsLoaded();
	virtual void SetLevelLoadReferenced(bool referenced);
	virtual bool IsLevelLoadReferenced();

	virtual void InitEmpty(const std::string fileName);
	virtual const std::string Name() const;
	virtual int NumSurfaces() const;
	virtual const ModelPixel &Surface(int surfaceNum) const;

	virtual dynamicModel_t IsDynamicModel() const;
	virtual bool IsReloadable() const;

	void MakeDefaultModel();

	virtual void SetColor(Screen::ConsoleColor col) override;
public:
	std::vector<ModelPixel> surfaces;

protected:
	std::string name;
	bool purged; // eventually we will have dynamic reloading
	bool reloadable; // if not, reloadModels won't check timestamp
	bool levelLoadReferenced; // for determining if it needs to be freed
};

#endif