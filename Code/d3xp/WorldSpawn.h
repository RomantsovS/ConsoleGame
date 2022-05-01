#ifndef __GAME_WORLDSPAWN_H__
#define __GAME_WORLDSPAWN_H__

/*
===============================================================================

  World entity.

===============================================================================
*/

class idWorldspawn : public idEntity {
public:
	CLASS_PROTOTYPE(idWorldspawn);

	idWorldspawn() = default;
	~idWorldspawn();
	idWorldspawn(const idWorldspawn&) = default;
	idWorldspawn& operator=(const idWorldspawn&) = default;
	idWorldspawn(idWorldspawn&&) = default;
	idWorldspawn& operator=(idWorldspawn&&) = default;

	void Spawn();

private:
	void Event_Remove();
};

#endif
