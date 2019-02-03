#ifndef Game_H
#define Game_H

#include <list>
#include <ctime>
#include <random>

#include "Entity.h"
#include "tr_local.h"
#include "Class.h"
#include "Dict.h"

extern RenderWorld *gameRenderWorld;

class Game
{
public:
	Game();

	~Game();

	void Init();
	void Destroy();

	void AddObject(Entity *object);

	bool IsGameRunning() const { return gameRunning; }

	void Frame();

	/*void onKeyPressed(char c);

	bool checkCollideObjects(SimpleObject *object);*/

	template <typename T>
	T GetRandomValue(T min, T max);

	Screen::ConsoleColor GetRandomColor();

	bool SpawnEntityDef(const Dict &args);

	void RegisterEntity(Entity *ent, int forceSpawnId, const Dict & spawnArgsToCopy);
	//void UnregisterEntity(Entity *ent);

	const Dict &GetSpawnArgs() const { return spawnArgs; }
private:
	void BreakTime();

	void AddRandomPoint();

	void RemoveInactiveObjects();

	/*void onMoveKeyPressed(SimpleObject::directions dir);

	bool checkCollidePosToAllObjects(pos_type pos);*/

	//RenderSystem *renderSystem;
	RenderWorld *renderWorld;

	//std::shared_ptr<Snake> snake;
	std::list<Entity*> entities;
	//std::vector<std::shared_ptr<EntityBase>> collideObjects;

	Dict spawnArgs;

	size_t height, width;

	clock_t lastClock;
	int delayMilliseconds;

	std::vector<Screen::ConsoleColor> colors;

	bool gameRunning;

	std::default_random_engine rand_eng;
};

template<typename T>
inline T Game::GetRandomValue(T min, T max)
{
	std::uniform_int_distribution<Screen::pos> u(min, max);

	return u(rand_eng);
}

extern Game gameLocal;

#endif
