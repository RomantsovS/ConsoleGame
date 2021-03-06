#ifndef __GAME_WEAPON_H__
#define __GAME_WEAPON_H__

enum class weaponStatus_t {
	WP_READY,
	WP_OUTOFAMMO,
	WP_RELOAD,
	WP_HOLSTERED,
	WP_RISING,
	WP_LOWERING,
	WP_FIRING,
	WP_WAITING
};

class idWeapon : public idAnimatedEntity {
public:
	CLASS_PROTOTYPE(idWeapon);

	idWeapon();
	virtual ~idWeapon();

	// Init
	void Spawn();
	void SetOwner(std::shared_ptr<idPlayer> owner);

	static void CacheWeapon(const std::string& weaponName);

	// Weapon definition management
	void Clear();
	void GetWeaponDef(const std::string& objectname, int ammoinclip);

	// State control/player interface
	void Think();
	void BeginAttack();
	void EndAttack();
	bool IsReady() const;
	bool IsHolstered() const;

	weaponStatus_t GetStatus() { return status; };
	void SetStatus(weaponStatus_t _status) { status = _status; }

	// Script state management
	void UpdateScript();

	// Visual presentation
	void PresentWeapon();

	friend class idPlayer;
private:
	// script control
	bool WEAPON_ATTACK{};
	weaponStatus_t status{};

	std::weak_ptr<idPlayer> owner;

	// weapon definition
	// we maintain local copies of the projectile and brass dictionaries so they
	// do not have to be copied across the DLL boundary when entities are spawned
	std::weak_ptr<idDeclEntityDef> weaponDef;
	idDict projectileDict{};

	int lastAttack{};			// last time an attack occured

	// script events
	void Event_WeaponReady();
};

#endif