#ifndef GAME_LOCAL_H
#define GAME_LOCAL_H

#include <random>

#include "Game.h"
#include "RenderWorld.h"

extern RenderWorld *gameRenderWorld;

const int GENTITYNUM_BITS = 12;
const int MAX_GENTITIES = 1 << GENTITYNUM_BITS;
const int ENTITYNUM_NONE = MAX_GENTITIES - 1;

class GameLocal : public Game
{
public:
	int framenum;
	int time;					// in msec
	int	previousTime;			// time in msec of last frame

	~GameLocal();

	virtual void Init();

	virtual bool IsGameRunning() const { return gameRunning; }

	virtual void Frame();

	/*void onKeyPressed(char c);

	bool checkCollideObjects(SimpleObject *object);*/

	template <typename T>
	T GetRandomValue(T min, T max);

	Screen::ConsoleColor GetRandomColor();

	bool SpawnEntityDef(const Dict &args);

	void RegisterEntity(Entity *ent, int forceSpawnId, const Dict & spawnArgsToCopy);
	void UnregisterEntity(Entity *ent);

	const Dict &GetSpawnArgs() const { return spawnArgs; }

	size_t GetHeight() { return height; }
	size_t GetWidth() { return width; }
private:
	void BreakTime();

	void AddRandomPoint();

	void RemoveInactiveObjects();

	/*void onMoveKeyPressed(SimpleObject::directions dir);

	bool checkCollidePosToAllObjects(pos_type pos);*/

	//RenderSystem *renderSystem;
	RenderWorld *renderWorld;

	//std::shared_ptr<Snake> snake;
	std::vector<Entity*> entities;
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