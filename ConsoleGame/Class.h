#ifndef CLASS_H
#define CLASS_H

#include <string>
#include <vector>

class idClass;
class idTypeInfo;

#define CLASS_PROTOTYPE( nameofclass )									\
public:																	\
	static idTypeInfo Type;						\
	static std::shared_ptr<idClass> CreateInstance();	\
	virtual idTypeInfo *GetType() const;		\

#define CLASS_DECLARATION( nameofsuperclass, nameofclass )											\
	idTypeInfo nameofclass::Type( #nameofclass, #nameofsuperclass,									\
		nameofclass::CreateInstance, ( void ( idClass::* )() )&nameofclass::Spawn);	                \
	std::shared_ptr<idClass> nameofclass::CreateInstance() {										\
        std::shared_ptr<nameofclass> ptr = std::make_shared<nameofclass>();						\
			return ptr;																				\
	}																								\
	idTypeInfo *nameofclass::GetType() const {														\
		return &( nameofclass::Type );																\
	}																								\

/*
try {																						\
			std::shared_ptr<nameofclass> ptr = std::make_shared<nameofclass>();						\
			return ptr;																				\
		}																							\
		catch( std::exception & ) {																    \
			return nullptr;																			\
		}																							\
*/

#define ABSTRACT_PROTOTYPE( nameofclass )								\
public:																	\
	static idTypeInfo Type;						\
	static std::shared_ptr<idClass> CreateInstance();	\
	virtual idTypeInfo *GetType() const;		\

#define ABSTRACT_DECLARATION( nameofsuperclass, nameofclass )										\
	idTypeInfo nameofclass::Type( #nameofclass, #nameofsuperclass,									\
		nameofclass::CreateInstance, ( void ( idClass::* )() )&nameofclass::Spawn);	\
	std::shared_ptr<idClass> nameofclass::CreateInstance() {													\
		return nullptr;																				\
	}																								\
	idTypeInfo *nameofclass::GetType() const {												\
		return &( nameofclass::Type );																\
	}																								\

typedef void (idClass::*classSpawnFunc_t)();

class idClass
{
public:
	ABSTRACT_PROTOTYPE(idClass);
	
	virtual ~idClass();

	void Spawn();
	void CallSpawn();
	//bool IsType(const TypeInfo &c) const;
	const std::string GetClassname() const;

	// Static functions
	static void Init();
	static void	Shutdown();
	static idTypeInfo * GetClass(const std::string name);
private:
	classSpawnFunc_t CallSpawnFunc(idTypeInfo *cls);

	static bool initialized;
	static std::vector<idTypeInfo*> types;
};

class idTypeInfo {
public:
	std::string classname;
	std::string superclass;
	std::shared_ptr<idClass> (*CreateInstance)();
	void (idClass::*Spawn)();

	idTypeInfo *super;
	idTypeInfo *next;

	idTypeInfo(std::string classname, std::string superclass, std::shared_ptr<idClass> (*CreateInstance)(), void (idClass::*Spawn)());
	
	~idTypeInfo();

	void Init();
	void Shutdown();

	//bool IsType(const TypeInfo &superclass) const;
};

#endif