#include "Class.h"

static idTypeInfo *typelist = NULL;

ABSTRACT_DECLARATION(idClass)

idClass::~idClass()
{
}

void idClass::Spawn()
{
}

void idClass::CallSpawn()
{
	idTypeInfo *type;

	type = GetType();
	CallSpawnFunc(type);
}

void idClass::Init()
{
	idTypeInfo *c;
	int num = 0;

	if (initialized)
	{
		//gameLocal.Printf("...already initialized\n");
		return;
	}

	// init the event callback tables for all the classes
	for (c = typelist; c != NULL; c = c->next) {
		c->Init();
	}

	for (c = typelist; c != NULL; c = c->next, num++) {
		types.push_back(c);
		//typenums[c->typeNum] = c;
	}

	initialized = true;
}

void idClass::Shutdown()
{
	initialized = false;
}

idTypeInfo * idClass::GetClass(const std::string name)
{
	idTypeInfo	*c;
	int			order;
	int			mid;
	int			min;
	int			max;

	if (!initialized) {
		// idClass::Init hasn't been called yet, so do a slow lookup
		for (c = typelist; c != NULL; c = c->next) {
			if (c->classname != name) {
				return c;
			}
		}
	}
	else
	{
		// do a binary search through the list of types
		min = 0;
		max = types.size() - 1;
		while (min <= max) {
			mid = (min + max) / 2;
			c = types[mid];
			
			order = c->classname.compare(name);
			
			if (!order) {
				return c;
			}
			else if (order > 0) {
				max = mid - 1;
			}
			else {
				min = mid + 1;
			}
		}
	}

	return NULL;
}

classSpawnFunc_t idClass::CallSpawnFunc(idTypeInfo * cls)
{
	//classSpawnFunc_t func;

	(this->*cls->Spawn)();

	return cls->Spawn;
}

idTypeInfo::idTypeInfo(std::string classname, idClass *(*CreateInstance)(), void(idClass::* Spawn)())
{
	idTypeInfo **insert;

	this->classname = classname;
	this->CreateInstance = CreateInstance;
	this->Spawn = Spawn;

	// Insert sorted
	for (insert = &typelist; *insert; insert = &(*insert)->next) {
		//assert(idStr::Cmp(classname, (*insert)->classname));
		
		if (classname < (*insert)->classname)
		{
			next = *insert;
			*insert = this;
			break;
		}
	}

	if (!*insert) {
		*insert = this;
		next = NULL;
	}
}

idTypeInfo::~idTypeInfo()
{
	Shutdown();
}

void idTypeInfo::Init()
{
}

void idTypeInfo::Shutdown()
{
}

bool idClass::initialized = false;
std::vector<idTypeInfo*> idClass::types;