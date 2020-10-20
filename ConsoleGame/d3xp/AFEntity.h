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
const int GIB_DELAY = 200;  // only gib this often to keep performace hits when blowing up several mobs

class idMultiModelAF : public idEntity {
public:
	CLASS_PROTOTYPE(idMultiModelAF);

	void Spawn();
	~idMultiModelAF();

	virtual void Think() override;
	virtual void Present() override;

protected:
	idPhysics_AF physicsObj;

	void SetModelForId(int id, const idStr& modelName);

private:
	std::vector<std::shared_ptr<idRenderModel>> modelHandles;
	std::vector<int> modelDefHandles;
};

#endif