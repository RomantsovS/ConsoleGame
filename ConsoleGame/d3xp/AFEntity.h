#ifndef D3XP_AFENTITY_H_
#define D3XP_AFENTITY_H_

#include <vector>

#include "Entity.h"

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

	void Spawn();
	~idMultiModelAF();

	virtual void Think() override;
	virtual void Present() override;

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
	void BuildChain(const std::string& name, const Vector2& origin, float linkLength, int numLinks);
};

#endif