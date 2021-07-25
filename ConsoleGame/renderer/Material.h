#ifndef __MATERIAL_H__
#define __MATERIAL_H__

class idImage;

struct textureStage_t {
	std::shared_ptr<idImage> image;
};

class idMaterial : public idDecl {
public:
	idMaterial();
	virtual ~idMaterial();
	idMaterial(const idMaterial&) = default;
	idMaterial& operator=(const idMaterial&) = default;
	idMaterial(idMaterial&&) = default;
	idMaterial& operator=(idMaterial&&) = default;

	bool SetDefaultText() override;
	std::string DefaultDefinition() const override;
	bool Parse(const char* text, const int textLength, bool allowBinaryVersion) override;

	// get a specific stage
	const textureStage_t* GetStage() const { return stage.get(); }

private:
	// parse the entire material
	void CommonInit();
	void ParseMaterial(idLexer& src);
	void ParseStage(idLexer& src);
private:
	std::unique_ptr<textureStage_t> stage;
};

#endif