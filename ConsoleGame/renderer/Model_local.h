#ifndef MODEL_LOCAL_H
#define MODEL_LOCAL_H

class idRenderModelStatic : public idRenderModel
{
public:
	idRenderModelStatic();
	virtual ~idRenderModelStatic();
	idRenderModelStatic(const idRenderModelStatic&) = default;
	idRenderModelStatic& operator=(const idRenderModelStatic&) = default;
	idRenderModelStatic(idRenderModelStatic&&) = default;
	idRenderModelStatic& operator=(idRenderModelStatic&&) = default;

	void InitFromFile(std::string fileName) override;
	bool LoadBinaryModel(idFile* file) override;
	bool SupportsBinaryModel() override { return false; }

	void PurgeModel() override;
	void LoadModel() override;
	bool IsLoaded() override;
	void SetLevelLoadReferenced(bool referenced) override;
	bool IsLevelLoadReferenced() override;

	void InitEmpty(const std::string fileName) override;
	const std::string Name() const override;
	int NumSurfaces() const override;
	const ModelPixel &Surface(int surfaceNum) const override;

	dynamicModel_t IsDynamicModel() const override;
	bool IsDefaultModel() const override;
	bool IsReloadable() const override;

	void MakeDefaultModel();

	bool LoadTextModel(const std::string& fileName);
	bool LoadBMPModel(const std::string& fileName);

	int GetColor() const override;
	void SetColor(int col) override;
public:
	std::vector<ModelPixel> surfaces;

protected:
	std::string name;
	bool defaulted = false;
	bool purged; // eventually we will have dynamic reloading
	bool reloadable; // if not, reloadModels won't check timestamp
	bool levelLoadReferenced; // for determining if it needs to be freed
private:
	int color;
};

#endif