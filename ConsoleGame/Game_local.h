#ifndef GAME_LOCAL_H
#define GAME_LOCAL_H

#include <random>

#include "Game.h"
#include "RenderWorld.h"

extern RenderWorld *gameRenderWorld;

class GameLocal : public Game
{
public:
	~GameLocal();

	virtual void Init();

	void AddObject(Entity *object);

	virtual bool IsGameRunning() const { return gameRunning; }

	virtual void Frame();

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
inline T GameLocal::GetRandomValue(T min, T max)
{
	std::uniform_int_distribution<Screen::pos> u(min, max);

	return u(rand_eng);
}

extern GameLocal gameLocal;

#endif