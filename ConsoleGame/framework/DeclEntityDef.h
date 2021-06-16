#ifndef FRAMEWORK_DECLENTITYDEF_H_
#define FRAMEWORK_DECLENTITYDEF_H_

/*
===============================================================================

	idDeclEntityDef

===============================================================================
*/

class idDeclEntityDef : public idDecl {
public:
	idDict dict;

	//virtual size_t Size() const;
	virtual std::string DefaultDefinition() const override;
	virtual bool Parse(const char* text, const int textLength, bool allowBinaryVersion) override;
	virtual void FreeData() override;
	//virtual void Print();
};

#endif /* !FRAMEWORK_DECLENTITYDEF_H_ */
