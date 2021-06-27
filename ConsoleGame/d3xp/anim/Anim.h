#ifndef __ANIM_H__
#define __ANIM_H__

class idDeclModelDef : public idDecl {
public:
	idDeclModelDef();
	~idDeclModelDef();

	std::string DefaultDefinition() const override;
	bool Parse(const char* text, const int textLength, bool allowBinaryVersion) override;
	void FreeData() override;

	std::weak_ptr<idRenderModel> ModelHandle() const;
private:
	std::weak_ptr<idRenderModel> modelHandle;
};

#endif