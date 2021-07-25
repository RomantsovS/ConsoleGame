#ifndef D3XP_AFENTITY_H_
#define D3XP_AFENTITY_H_

/*
===============================================================================

idMultiModelAF

Entity using multiple separate visual models animated with a single
articulated figure. Only used for debugging!

===============================================================================
*/

#include "physics/Physics_AF.h"

class idMultiModelAF : public idEntity {
public:
	CLASS_PROTOTYPE(idMultiModelAF);

	idMultiModelAF() = default;
	~idMultiModelAF();
	idMultiModelAF(const idMultiModelAF&) = default;
	idMultiModelAF& operator=(const idMultiModelAF&) = default;
	idMultiModelAF(idMultiModelAF&&) = default;
	idMultiModelAF& operator=(idMultiModelAF&&) = default;

	void Spawn();

	virtual void Think() override;
	virtual void Present() override;

	virtual void Remove() override;

	virtual bool Collide(const trace_t& collision, const Vector2& velocity) override;
protected:
	std::shared_ptr<idPhysics_AF> physicsObj;

	void SetModelForId(int id, const std::string& modelName);

private:
	std::vector<std::shared_ptr<idRenderModel>> modelHandles;
	std::vector<int> modelDefHandles;
};

/*
===============================================================================

idChain

Chain hanging down from the ceiling. Only used for debugging!

===============================================================================
*/

class idChain : public idMultiModelAF {
public:
	CLASS_PROTOTYPE(idChain);

	void Spawn();

protected:
	void BuildChain(const std::string& name, const Vector2& origin, float linkLength, int numLinks, const Vector2& dir);
};

#endif