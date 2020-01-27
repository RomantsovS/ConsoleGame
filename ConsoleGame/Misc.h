#ifndef MISC_H
#define MISC_H

#include "Entity.h"

class idStaticEntity : public idEntity
{
public:
	CLASS_PROTOTYPE(idStaticEntity);

	idStaticEntity();

	virtual ~idStaticEntity() override;

	void Spawn();
	virtual void Think() override;
private:
	int spawnTime;
};

#endif