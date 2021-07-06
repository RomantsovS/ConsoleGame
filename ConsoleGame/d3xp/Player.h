#ifndef PLAYER_ENTITY_H
#define PLAYER_ENTITY_H

class idPlayer : public idActor
{
public:
	usercmd_t usercmd;
	int oldButtons{};
public:
	CLASS_PROTOTYPE(idPlayer);

	idPlayer();
	virtual ~idPlayer();

	void Spawn();
	void Think() override;

	void Init();
	void SetupWeaponEntity();
	void SelectInitialSpawnPoint(Vector2& origin, Vector2& angles);
	void SpawnFromSpawnSpot();
	void SpawnToPoint(const Vector2& spawn_origin, const Vector2& spawn_angles);
	void SetClipModel();

	void RestorePersistantInfo();

	bool Collide(const trace_t& collision, const Vector2& velocity) override;

	void HandleUserCmds(const usercmd_t& newcmd);

	bool IsLocallyControlled() const;
private:
	std::shared_ptr<idWeapon> weapon;

	std::shared_ptr<Physics_PlayerMy> physicsObj; // player physics

	int currentWeapon{ -1 };
	int idealWeapon{ -1 };

	void EvaluateControls();
	void AdjustSpeed();
	void Move();

	void FireWeapon();
	void Weapon_Combat();
	void UpdateWeapon();
};

#endif