#include "idlib/precompiled.h"
#pragma hdrstop

#include "../Game_local.h"

constexpr auto MAX_EVENTSPERFRAME = 4096;
constexpr auto MAX_STRING_LEN = 128;

/***********************************************************************

  idEventDef

***********************************************************************/

idEventDef* idEventDef::eventDefList[MAX_EVENTS];
int idEventDef::numEventDefs = 0;

static bool eventError = false;
static char eventErrorMsg[128];

idEventDef::idEventDef(const std::string& command, const std::string& formatspec, char returnType){
	//assert(command);
	//assert(!idEvent::initialized);

	this->name = command;
	this->formatspec = formatspec;

	numargs = formatspec.size();
	if (numargs > D_EVENT_MAXARGS) {
		eventError = true;
		sprintf_s(eventErrorMsg, "idEventDef::idEventDef : Too many args for '%s' event.", name.c_str());
		return;
	}

	// make sure the format for the args is valid, calculate the formatspecindex, and the offsets for each arg
	size_t bits = 0;
	argsize = 0;
	//memset(argOffset, 0, sizeof(argOffset));
	for (auto i = 0; i < numargs; i++) {
		//argOffset[i] = argsize;
		switch (formatspec[i]) {
		case D_EVENT_FLOAT:
			bits |= 1 << i;
			argsize += sizeof(float);
			break;

		case D_EVENT_INTEGER:
			argsize += sizeof(int);
			break;

		case D_EVENT_VECTOR:
			argsize += sizeof(Vector2);
			break;

		/*case D_EVENT_STRING:
			argsize += MAX_STRING_LEN;
			break;*/

		case D_EVENT_ENTITY:
			argsize += sizeof(std::shared_ptr<idEntity>);
			break;

		case D_EVENT_ENTITY_NULL:
			argsize += sizeof(std::shared_ptr<idEntity>);
			break;

		/*case D_EVENT_TRACE:
			argsize += sizeof(trace_t) + MAX_STRING_LEN + sizeof(bool);
			break;*/

		default:
			eventError = true;
			sprintf_s(eventErrorMsg, "idEventDef::idEventDef : Invalid arg format '%s' string for '%s' event.", formatspec.c_str(), name.c_str());
			return;
			break;
		}
	}

	// calculate the formatspecindex
	formatspecIndex = (1 << (numargs + D_EVENT_MAXARGS)) | bits;

	// go through the list of defined events and check for duplicates
	// and mismatched format strings
	eventnum = numEventDefs;
	for (int i = 0; i < eventnum; i++) {
		auto ev = eventDefList[i];
		if (command == ev->name) {
			if (formatspec != ev->formatspec) {
				eventError = true;
				sprintf_s(eventErrorMsg, "idEvent '%s' defined twice with same name but differing format strings ('%s'!='%s').",
					command.c_str(), formatspec.c_str(), ev->formatspec.c_str());
				return;
			}

			/*if (ev->returnType != returnType) {
				eventError = true;
				sprintf(eventErrorMsg, "idEvent '%s' defined twice with same name but differing return types ('%c'!='%c').",
					command, returnType, ev->returnType);
				return;
			}*/
			// Don't bother putting the duplicate event in list.
			eventnum = ev->eventnum;
			return;
		}
	}

	auto ev = this;

	if (numEventDefs >= MAX_EVENTS) {
		eventError = true;
		sprintf_s(eventErrorMsg, "numEventDefs >= MAX_EVENTS");
		return;
	}
	eventDefList[numEventDefs] = ev;
	numEventDefs++;
}

/*
================
idEventDef::NumEventCommands
================
*/
int	idEventDef::NumEventCommands() noexcept {
	return numEventDefs;
}

/***********************************************************************

  idEvent

***********************************************************************/

static idLinkList<idEvent> FreeEvents;
static idLinkList<idEvent> EventQueue;
static std::vector<std::shared_ptr<idEvent>> EventPool;

bool idEvent::initialized = false;

std::allocator<byte> idEvent::eventDataAllocator;

/*
================
idEvent::idEvent()
================
*/
idEvent::idEvent() {
#ifdef DEBUG_PRINT_Ctor_Dtor
	common->DPrintf("%s ctor\n", "idEvent");
#endif // DEBUG_PRINT_Ctor_Dtor
}

/*
================
idEvent::~idEvent()
================
*/
idEvent::~idEvent() {
#ifdef DEBUG_PRINT_Ctor_Dtor
	if(isCommonExists)
		common->DPrintf("%s dtor\n", "idEvent");
#endif // DEBUG_PRINT_Ctor_Dtor
	Free(false);
}

/*
================
idEvent::Alloc
================
*/
std::shared_ptr<idEvent> idEvent::Alloc(gsl::not_null<const idEventDef*> evdef, int numargs, va_list args) {
	idEventArg* arg;

	if (FreeEvents.IsListEmpty()) {
		gameLocal.Error("idEvent::Alloc : No more free events");
	}

	auto ev = FreeEvents.Next();
	ev->eventNode.Remove();

	ev->eventdef = evdef;

	if (numargs != evdef->GetNumArgs()) {
		gameLocal.Error("idEvent::Alloc : Wrong number of args for '%s' event.", evdef->GetName());
	}

	auto size = evdef->GetArgSize();
	if (size) {
		ev->data = eventDataAllocator.allocate(size);
		memset(ev->data, 0, size);
	}
	else {
		ev->data = NULL;
		return ev;
	}

	auto format = evdef->GetArgFormat();
	for (auto i = 0; i < numargs; i++) {
		arg = va_arg(args, idEventArg*);
		if (format[i] != arg->type) {
			// when NULL is passed in for an entity, it gets cast as an integer 0, so don't give an error when it happens
			if (!(((format[i] == D_EVENT_TRACE) || (format[i] == D_EVENT_ENTITY)) && (arg->type == 'd') && (arg->value == 0))) {
				gameLocal.Error("idEvent::Alloc : Wrong type passed in for arg # %d on '%s' event.", i, evdef->GetName());
			}
		}

		auto dataPtr = &ev->data[/*evdef->GetArgOffset(i)*/0];

		switch (format[i]) {
		case D_EVENT_FLOAT:
		case D_EVENT_INTEGER:
			*reinterpret_cast<int*>(dataPtr) = arg->value;
			break;

		case D_EVENT_VECTOR:
			if (arg->value) {
				*reinterpret_cast<Vector2*>(dataPtr) = *reinterpret_cast<const Vector2*>(arg->value);
			}
			break;

		case D_EVENT_STRING:
			if (arg->value) {
				idStr::Copynz(reinterpret_cast<char*>(dataPtr), reinterpret_cast<const char*>(arg->value), MAX_STRING_LEN);
			}
			break;

		/*case D_EVENT_ENTITY:
		case D_EVENT_ENTITY_NULL:
			*reinterpret_cast<idEntity>(dataPtr) = reinterpret_cast<idEntity*>(arg->value);
			break;
			*/
		/*case D_EVENT_TRACE:
			if (arg->value) {
				*reinterpret_cast<bool*>(dataPtr) = true;
				*reinterpret_cast<trace_t*>(dataPtr + sizeof(bool)) = *reinterpret_cast<const trace_t*>(arg->value);

				// save off the material as a string since the pointer won't be valid in save games.
				// since we save off the entire trace_t structure, if the material is NULL here,
				// it will be NULL when we process it, so we don't need to save off anything in that case.
				if (reinterpret_cast<const trace_t*>(arg->value)->c.material) {
					materialName = reinterpret_cast<const trace_t*>(arg->value)->c.material->GetName();
					idStr::Copynz(reinterpret_cast<char*>(dataPtr + sizeof(bool) + sizeof(trace_t)), materialName, MAX_STRING_LEN);
				}
			}
			else {
				*reinterpret_cast<bool*>(dataPtr) = false;
			}
			break;
			*/
		default:
			gameLocal.Error("idEvent::Alloc : Invalid arg format '%s' string for '%s' event.", format, evdef->GetName());
			break;
		}
	}

	return ev;
}

/*
================
idEvent::Free
================
*/
void idEvent::Free(bool setOwner) {
	if (data) {
		eventDataAllocator.deallocate(data, sizeof(data));
		data = nullptr;
	}

	eventdef = nullptr;
	time = 0;
	object = nullptr;
	typeinfo = nullptr;

	if(setOwner)
		eventNode.SetOwner(shared_from_this());
	eventNode.AddToEnd(FreeEvents);
}

/*
================
idEvent::Schedule
================
*/
void idEvent::Schedule(idClass* obj, const idTypeInfo* type, int time) noexcept {
	//assert(initialized);
	if (!initialized) {
		return;
	}

	object = obj;
	typeinfo = type;

	// wraps after 24 days...like I care. ;)
	this->time = gameLocal.time + time;

	eventNode.Remove();

	//this->time = gameLocal.slow.time + time;

	auto event = EventQueue.Next();
	while ((event != NULL) && (this->time >= event->time)) {
		event = event->eventNode.Next();
	}

	if (event) {
		eventNode.InsertBefore(event->eventNode);
	}
	else {
		eventNode.AddToEnd(EventQueue);
	}
}

/*
================
idEvent::CancelEvents
================
*/
void idEvent::CancelEvents(const idClass* obj, const idEventDef* evdef) {
	if (!initialized) {
		return;
	}

	std::shared_ptr<idEvent> next;

	for (auto event = EventQueue.Next(); event != NULL; event = next) {
		next = event->eventNode.Next();
		if (event->object == obj) {
			if (!evdef || (evdef == event->eventdef)) {
				event->Free();
			}
		}
	}
}

/*
================
idEvent::ClearEventList
================
*/
void idEvent::ClearEventList() {
	//
	// initialize lists
	//
	FreeEvents.Clear();
	EventQueue.Clear();

	// 
	// add the events to the free list
	//
	for (size_t i = 0; i < EventPool.size(); i++) {
		if (EventPool[i]) {
			EventPool[i]->Free();
		}
	}
}

/*
================
idEvent::ServiceEvents
================
*/
void idEvent::ServiceEvents() {
	int args[D_EVENT_MAXARGS];

	int num = 0;
	while (!EventQueue.IsListEmpty()) {
		auto event = EventQueue.Next();
		//assert(event);

		if (event->time > gameLocal.time) {
			break;
		}

		// copy the data into the local args array and set up pointers
		gsl::not_null<const idEventDef*> ev = event->eventdef;
		std::string formatspec = ev->GetArgFormat();
		int numargs = ev->GetNumArgs();
		for (auto i = 0; i < numargs; i++) {
			auto offset = 0;
			auto data = event->data;
			switch (formatspec[i]) {
			case D_EVENT_FLOAT:
			case D_EVENT_INTEGER:
				args[i] = *reinterpret_cast<int*>(&data[offset]);
				break;

			case D_EVENT_VECTOR:
				*reinterpret_cast<Vector2**>(&args[i]) = reinterpret_cast<Vector2*>(&data[offset]);
				break;

			case D_EVENT_STRING:
				*reinterpret_cast<const char**>(&args[i]) = reinterpret_cast<const char*>(&data[offset]);
				break;

			case D_EVENT_ENTITY:
			case D_EVENT_ENTITY_NULL:
				//*reinterpret_cast<idEntity**>(&args[i]) = reinterpret_cast<idEntityPtr<idEntity>*>(&data[offset])->GetEntity();
				break;

			/*case D_EVENT_TRACE:
				tracePtr = reinterpret_cast<trace_t**>(&args[i]);
				if (*reinterpret_cast<bool*>(&data[offset])) {
					*tracePtr = reinterpret_cast<trace_t*>(&data[offset + sizeof(bool)]);

					if ((*tracePtr)->c.material != NULL) {
						// look up the material name to get the material pointer
						materialName = reinterpret_cast<const char*>(&data[offset + sizeof(bool) + sizeof(trace_t)]);
						(*tracePtr)->c.material = declManager->FindMaterial(materialName, true);
					}
				}
				else {
					*tracePtr = NULL;
				}
				break;
				*/
			default:
				gameLocal.Error("idEvent::ServiceEvents : Invalid arg format '%s' string for '%s' event.", formatspec, ev->GetName());
			}
		}

		// the event is removed from its list so that if then object
		// is deleted, the event won't be freed twice
		event->eventNode.Remove();
		//assert(event->object);
		event->object->ProcessEventArgPtr(ev, args);

		// return the event to the free list
		event->Free();

		// Don't allow ourselves to stay in here too long.  An abnormally high number
		// of events being processed is evidence of an infinite loop of events.
		num++;
		if (num > MAX_EVENTSPERFRAME) {
			gameLocal.Error("Event overflow.  Possible infinite loop in script.");
		}
	}
}

/*
================
idEvent::Init
================
*/
void idEvent::Init() {
	gameLocal.Printf("Initializing event system\n");

	if (eventError) {
		gameLocal.Error("%s", eventErrorMsg);
	}

	if (initialized) {
		gameLocal.Printf("...already initialized\n");
		ClearEventList();
		return;
	}

	EventPool.resize(MAX_EVENTS);
	for (size_t i = 0; i < MAX_EVENTS; ++i) {
		EventPool[i] = std::make_shared<idEvent>();
	}

	ClearEventList();

	//eventDataAllocator.Init();

	//gameLocal.Printf("...%i event definitions\n", idEventDef::NumEventCommands());

	// the event system has started
	initialized = true;
}

/*
================
idEvent::Shutdown
================
*/
void idEvent::Shutdown() {
	gameLocal.Printf("Shutdown event system\n");

	if (!initialized) {
		gameLocal.Printf("...not started\n");
		return;
	}

	ClearEventList();
	EventPool.clear();


	//eventDataAllocator.Shutdown();

	// say it is now shutdown
	initialized = false;
}