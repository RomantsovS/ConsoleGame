#ifndef MODEL_LOCAL_H
#define MODEL_LOCAL_H

class idRenderModelStatic : public idRenderModel
{
public:
	idRenderModelStatic();
	virtual ~idRenderModelStatic();

	virtual void InitFromFile(std::string fileName) override;

	virtual void PurgeModel() override;
	virtual void LoadModel() override;
	virtual bool IsLoaded() override;
	virtual void SetLevelLoadReferenced(bool referenced) override;
	virtual bool IsLevelLoadReferenced() override;

	virtual void InitEmpty(const std::string fileName) override;
	virtual const std::string Name() const override;
	virtual int NumSurfaces() const override;
	virtual const ModelPixel &Surface(int surfaceNum) const override;

	virtual dynamicModel_t IsDynamicModel() const override;
	virtual bool IsReloadable() const;

	void MakeDefaultModel();

	virtual Screen::ConsoleColor GetColor() const override;
	virtual void SetColor(Screen::ConsoleColor col) override;
public:
	std::vector<ModelPixel> surfaces;

protected:
	std::string name;
	bool purged; // eventually we will have dynamic reloading
	bool reloadable; // if not, reloadModels won't check timestamp
	bool levelLoadReferenced; // for determining if it needs to be freed
private:
	Screen::ConsoleColor color;
};

#endif