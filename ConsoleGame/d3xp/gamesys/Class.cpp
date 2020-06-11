#include "Class.h"
#include "../Game_local.h"

static idTypeInfo *typelist = NULL;

ABSTRACT_DECLARATION(NULL, idClass)

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

	gameLocal.Printf("Initializing class hierarchy\n");

	if (initialized)
	{
		gameLocal.Printf("...already initialized\n");
		return;
	}

	// init the event callback tables for all the classes
	for (c = typelist; c != NULL; c = c->next) {
		c->Init();
	}

	// number the types according to the class hierarchy so we can quickly determine if a class
	// is a subclass of another
	/*num = 0;
	for (c = classHierarchy.GetNext(); c != NULL; c = c->node.GetNext(), num++) {
		c->typeNum = num;
		c->lastChild += num;
	}*/

	for (c = typelist; c != NULL; c = c->next, num++) {
		types.push_back(c);
		//typenums[c->typeNum] = c;
	}

	initialized = true;

	gameLocal.Printf("...%i classes\n", types.size());
}

void idClass::Shutdown()
{
	idTypeInfo	*c;

	for (c = typelist; c != NULL; c = c->next) {
		c->Shutdown();
	}
	types.clear();

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
			if (c->classname == name) {
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

/*
================
idClass::GetClassname

Returns the text classname of the object.
================
*/
const std::string idClass::GetClassname() const
{
	idTypeInfo *type;

	type = GetType();
	return type->classname;
}

classSpawnFunc_t idClass::CallSpawnFunc(idTypeInfo * cls)
{
	classSpawnFunc_t func;

	if (cls->super) {
		func = CallSpawnFunc(cls->super);
		if (func == cls->Spawn) {
			// don't call the same function twice in a row.
			// this can happen when subclasses don't have their own spawn function.
			return func;
		}
	}

	(this->*cls->Spawn)();

	return cls->Spawn;
}

idTypeInfo::idTypeInfo(std::string classname, std::string superclass, std::shared_ptr<idClass> (*CreateInstance)(), void(idClass::* Spawn)())
{
	idTypeInfo *type;
	idTypeInfo **insert;

	this->classname = classname;
	this->superclass = superclass;
	this->Spawn = Spawn;
	this->CreateInstance = CreateInstance;
	this->super = idClass::GetClass(superclass);
	typeNum = 0;
	lastChild = 0;

	// Check if any subclasses were initialized before their superclass
	for (type = typelist; type != NULL; type = type->next) {
		if ((type->super == NULL) && type->superclass == this->classname &&
			type->classname != "idClass")
		{
			type->super = this;
		}
	}

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
	idTypeInfo* c;

	if (super) {
		super->Init();
	}

	// add to our node hierarchy
	/*if (super) {
		node.ParentTo(super->node);
	}
	else {
		node.ParentTo(classHierarchy);
	}
	node.SetOwner(this);*/

	// keep track of the number of children below each class
	for (c = super; c != NULL; c = c->super) {
		c->lastChild++;
	}
}

void idTypeInfo::Shutdown()
{
	typeNum = 0;
	lastChild = 0;
}

bool idClass::initialized = false;
std::vector<idTypeInfo*> idClass::types;