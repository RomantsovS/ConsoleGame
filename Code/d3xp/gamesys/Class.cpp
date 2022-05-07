#include "idlib/precompiled.h"

#include "../Game_local.h"

static idTypeInfo *typelist = NULL;
static idHierarchy<idTypeInfo> classHierarchy;

idTypeInfo::idTypeInfo(const std::string& classname, const std::string& superclass,
	idEventFunc<idClass>* eventCallbacks, std::shared_ptr<idClass> (*CreateInstance)(), void(idClass::* Spawn)()) {
	idTypeInfo *type;
	idTypeInfo** insert;

	this->classname = classname;
	this->superclass = superclass;
	this->eventCallbacks = eventCallbacks;
	this->eventMap = nullptr;
	this->Spawn = Spawn;
	this->CreateInstance = CreateInstance;
	this->super = idClass::GetClass(superclass);
	typeNum = 0;
	lastChild = 0;

	// Check if any subclasses were initialized before their superclass
	for (type = typelist; type != NULL; type = type->next) {
		if ((type->super == NULL) && type->superclass == this->classname && type->classname != "idClass") {
			type->super = this;
		}
	}

	// Insert sorted
	for (insert = &typelist; insert && *insert; insert = &(*insert)->next) {
		assert(classname != (*insert)->classname);

		if (classname < (*insert)->classname) {
			next = *insert;
			*insert = this;
			break;
		}
	}

	if (insert && !*insert) {
		*insert = this;
		next = NULL;
	}
}

idTypeInfo::~idTypeInfo() {
	Shutdown();
}

void idTypeInfo::Init() {
	if (eventMap) {
		// we've already been initialized by a subclass
		return;
	}

	// make sure our superclass is initialized first
	if (super && !super->eventMap) {
		super->Init();
	}

	// add to our node hierarchy
	if (super) {
		node.ParentTo(super->node);
	}
	else {
		node.ParentTo(classHierarchy);
	}
	node.SetOwner(this);

	// keep track of the number of children below each class
	for (auto c = super; c != NULL; c = c->super) {
		c->lastChild++;
	}

	// if we're not adding any new event callbacks, we can just use our superclass's table
	if ((!eventCallbacks || !eventCallbacks->event) && super) {
		eventMap = super->eventMap;
		return;
	}

	// set a flag so we know to delete the eventMap table
	freeEventMap = true;

	// Allocate our new table.  It has to have as many entries as there
	// are events.  NOTE: could save some space by keeping track of the maximum
	// event that the class responds to and doing range checking.
	auto num = idEventDef::NumEventCommands();
	eventMap = std::make_shared<std::vector<eventCallback_t>>(num);
	//eventCallbackMemory += sizeof(eventCallback_t) * num;

	// allocate temporary memory for flags so that the subclass's event callbacks
	// override the superclass's event callback
	std::vector<bool> set(num, 0);

	// go through the inheritence order and copies the event callback function into
	// a list indexed by the event number.  This allows fast lookups of
	// event functions.
	for (auto c = this; c != nullptr; c = c->super) {
		auto def = c->eventCallbacks;
		if (!def) {
			continue;
		}

		// go through each entry until we hit the NULL terminator
		for (size_t i = 0; def[i].event != nullptr; i++) {
			auto ev = def[i].event->GetEventNum();

			if (set[ev]) {
				continue;
			}
			set[ev] = true;
			eventMap->operator[](ev) = def[i].function;
		}
	}
}

void idTypeInfo::Shutdown() noexcept {
	// free up the memory used for event lookups
	if (eventMap) {
		if (freeEventMap) {
		}
		eventMap = nullptr;
	}
	typeNum = 0;
	lastChild = 0;
}

/***********************************************************************

  idClass

***********************************************************************/

const idEventDef EV_Remove("<immediateremove>");

ABSTRACT_DECLARATION(NULL, idClass)
EVENT(EV_Remove, idClass::Event_Remove)
END_CLASS

void idClass::CallSpawn() {
	idTypeInfo* type;

	type = GetType();
	CallSpawnFunc(type);
}

void idClass::Spawn() noexcept {
}

/*
================
idClass::~idClass

Destructor for object.  Cancels any events that depend on this object.
================
*/
idClass::~idClass() {
	idEvent::CancelEvents(this);
}

void idClass::Init() {
	idTypeInfo* c;
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
	num = 0;
	for (c = classHierarchy.GetNext(); c != nullptr; c = c->node.GetNext(), num++) {
		c->typeNum = num;
		c->lastChild += num;
	}

	for (c = typelist; c != NULL; c = c->next, num++) {
		types.push_back(c);
		//typenums[c->typeNum] = c;
	}

	initialized = true;

	gameLocal.Printf("...%i classes\n", types.size());
}

void idClass::Shutdown() noexcept {
	idTypeInfo* c;

	for (c = typelist; c != NULL; c = c->next) {
		c->Shutdown();
	}
	types.clear();

	initialized = false;
}

idTypeInfo* idClass::GetClass(const std::string& name) noexcept {
	idTypeInfo* c;
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
const std::string idClass::GetClassname() const {
	idTypeInfo* type;

	type = GetType();
	return type->classname;
}

/*
================
idClass::CancelEvents
================
*/
void idClass::CancelEvents(const idEventDef* ev) {
	idEvent::CancelEvents(this, ev);
}


/*
================
idClass::PostEventArgs
================
*/
bool idClass::PostEventArgs(const idEventDef* ev, int time, int numargs, ...) {
	va_list		args;

	assert(ev);

	if (!idEvent::initialized) {
		return false;
	}

	auto c = GetType();
	if (!c->eventMap->operator[](ev->GetEventNum())) {
		// we don't respond to this event, so ignore it
		return false;
	}

	// we service events on the client to avoid any bad code filling up the event pool
	// we don't want them processed usually, unless when the map is (re)loading.
	// we allow threads to run fine, though.
	/*if (gameLocal.GameState() != GAMESTATE_STARTUP) {
		return true;
	}*/

	va_start(args, numargs);
	auto event = idEvent::Alloc(ev, numargs, args);
	va_end(args);

	event->Schedule(this, c, time);

	return true;
}

/*
================
idClass::PostEventMS
================
*/
bool idClass::PostEventMS(const idEventDef* ev, int time) {
	return PostEventArgs(ev, time, 0);
}

/*
================
idClass::PostEventSec
================
*/
bool idClass::PostEventSec(const idEventDef* ev, float time) {
	return PostEventArgs(ev, static_cast<int>(SEC2MS(time)), 0);
}


classSpawnFunc_t idClass::CallSpawnFunc(gsl::not_null<idTypeInfo*> cls) {
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

/*
================
idClass::ProcessEventArgPtr
================
*/
bool idClass::ProcessEventArgPtr(const idEventDef* ev, int* data) {
	assert(ev);
	assert(idEvent::initialized);

	/*SetTimeState ts;

	if (IsType(idEntity::Type)) {
		idEntity* ent = (idEntity*)this;
		ts.PushState(ent->timeGroup);
	}

	if (g_debugTriggers.GetBool() && (ev == &EV_Activate) && IsType(idEntity::Type)) {
		const idEntity* ent = *reinterpret_cast<idEntity**>(data);
		gameLocal.Printf("%d: '%s' activated by '%s'\n", gameLocal.framenum, static_cast<idEntity*>(this)->GetName(), ent ? ent->GetName() : "NULL");
	}*/

	auto c = GetType();
	auto num = ev->GetEventNum();
	if (!c->eventMap->operator[](num)) {
		// we don't respond to this event, so ignore it
		return false;
	}

	gsl::not_null<eventCallback_t> callback = c->eventMap->operator[](num);

	/*
	on ppc architecture, floats are passed in a seperate set of registers
	the function prototypes must have matching float declaration

	http://developer.apple.com/documentation/DeveloperTools/Conceptual/MachORuntime/2rt_powerpc_abi/chapter_9_section_5.html
	*/

	switch (ev->GetFormatspecIndex()) {
	case 1 << D_EVENT_MAXARGS:
		(this->*callback)();
		break;

	default:
		gameLocal.Warning("Invalid formatspec on event '%s'", ev->GetName().c_str());
		break;
	}

	return true;
}

/*
================
idClass::Event_Remove
================
*/
void idClass::Event_Remove() noexcept {
	Remove();

	//delete this;
}

void idClass::Remove() noexcept {
}

bool idClass::initialized = false;
std::vector<idTypeInfo*> idClass::types;