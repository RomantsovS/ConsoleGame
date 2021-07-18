#ifndef CLASS_H
#define CLASS_H

class idClass;
class idTypeInfo;

extern const idEventDef EV_Remove;

typedef void (idClass::* eventCallback_t)();

template< class Type >
struct idEventFunc {
	const idEventDef* event;
	eventCallback_t function;
};

// added & so gcc could compile this
#define EVENT( event, function )	{ &( event ), ( void ( idClass::* )() )( &function ) },
#define END_CLASS					{ NULL, NULL } };

class idEventArg {
public:
	int			type;
	int			value;

	idEventArg() { type = D_EVENT_INTEGER; value = 0; };
	idEventArg(int data) { type = D_EVENT_INTEGER; value = data; };
	idEventArg(float data) { type = D_EVENT_FLOAT; value = *reinterpret_cast<int*>(&data); };
	idEventArg(Vector2& data) { type = D_EVENT_VECTOR; value = reinterpret_cast<int>(&data); };
	idEventArg(const std::string& data) { type = D_EVENT_STRING; value = reinterpret_cast<int>(data.c_str()); };
	idEventArg(const char* data) { type = D_EVENT_STRING; value = reinterpret_cast<int>(data); };
	idEventArg(const class idEntity* data) { type = D_EVENT_ENTITY; value = reinterpret_cast<int>(data); };
	idEventArg(const struct trace_s* data) { type = D_EVENT_TRACE; value = reinterpret_cast<int>(data); };
};

/***********************************************************************

  idClass

***********************************************************************/

/*
================
CLASS_PROTOTYPE

This macro must be included in the definition of any subclass of idClass.
It prototypes variables used in class instanciation and type checking.
Use this on single inheritance concrete classes only.
================
*/
#define CLASS_PROTOTYPE( nameofclass )									\
public:																	\
	static idTypeInfo Type;						\
	static std::shared_ptr<idClass> CreateInstance();	\
	virtual idTypeInfo *GetType() const;		\
	static	idEventFunc<nameofclass>		eventCallbacks[]

/*
================
CLASS_DECLARATION

This macro must be included in the code to properly initialize variables
used in type checking and run-time instanciation.  It also defines the list
of events that the class responds to.  Take special care to ensure that the
proper superclass is indicated or the run-time type information will be
incorrect.  Use this on concrete classes only.
================
*/
#define CLASS_DECLARATION( nameofsuperclass, nameofclass )											\
	idTypeInfo nameofclass::Type( #nameofclass, #nameofsuperclass,									\
		( idEventFunc<idClass> * )nameofclass::eventCallbacks, nameofclass::CreateInstance, ( void ( idClass::* )() )&nameofclass::Spawn); \
	std::shared_ptr<idClass> nameofclass::CreateInstance() {										\
	std::shared_ptr<nameofclass> ptr = std::make_shared<nameofclass>();						\
			return ptr;																				\
	}																								\
	idTypeInfo *nameofclass::GetType() const {														\
		return &( nameofclass::Type );																\
	}																								\
idEventFunc<nameofclass> nameofclass::eventCallbacks[] = {

/*
================
ABSTRACT_PROTOTYPE

This macro must be included in the definition of any abstract subclass of idClass.
It prototypes variables used in class instanciation and type checking.
Use this on single inheritance abstract classes only.
================
*/
#define ABSTRACT_PROTOTYPE( nameofclass )								\
public:																	\
	static idTypeInfo Type;						\
	static std::shared_ptr<idClass> CreateInstance();	\
	virtual idTypeInfo *GetType() const;		\
	static	idEventFunc<nameofclass>		eventCallbacks[]

/*
================
ABSTRACT_DECLARATION

This macro must be included in the code to properly initialize variables
used in type checking.  It also defines the list of events that the class
responds to.  Take special care to ensure that the proper superclass is
indicated or the run-time tyep information will be incorrect.  Use this
on abstract classes only.
================
*/
#define ABSTRACT_DECLARATION( nameofsuperclass, nameofclass )										\
	idTypeInfo nameofclass::Type( #nameofclass, #nameofsuperclass,									\
		( idEventFunc<idClass> * )nameofclass::eventCallbacks, nameofclass::CreateInstance, ( void ( idClass::* )() )&nameofclass::Spawn);	\
	std::shared_ptr<idClass> nameofclass::CreateInstance() {													\
		gameLocal.Error( "Cannot instanciate abstract class %s.", #nameofclass );					\
		return nullptr;																				\
	}																								\
	idTypeInfo *nameofclass::GetType() const {												\
		return &( nameofclass::Type );																\
	}																								\
	idEventFunc<nameofclass> nameofclass::eventCallbacks[] = {

typedef void (idClass::*classSpawnFunc_t)();

class idClass
{
public:
	ABSTRACT_PROTOTYPE(idClass);
	
	virtual ~idClass();

	void Spawn();
	void CallSpawn();
	bool IsType(const idTypeInfo &c) const;
	const std::string GetClassname() const;

	bool PostEventMS(const idEventDef* ev, int time);

	bool PostEventSec(const idEventDef* ev, float time);

	bool ProcessEventArgPtr(const idEventDef* ev, int* data);
	void CancelEvents(const idEventDef* ev);

	void Event_Remove();
	virtual void Remove();

	// Static functions
	static void Init();
	static void	Shutdown();
	static idTypeInfo * GetClass(const std::string& name);
private:
	classSpawnFunc_t CallSpawnFunc(gsl::not_null<idTypeInfo*> cls);

	bool PostEventArgs(const idEventDef* ev, int time, int numargs, ...);

	static bool initialized;
	static std::vector<idTypeInfo*> types;
};

class idTypeInfo {
public:
	std::string classname;
	std::string superclass;
	std::shared_ptr<idClass> (*CreateInstance)();
	void (idClass::*Spawn)();

	idEventFunc<idClass>* eventCallbacks;
	std::shared_ptr<std::vector<eventCallback_t>> eventMap;
	idTypeInfo *super;
	idTypeInfo *next;
	bool freeEventMap;
	int typeNum;
	int lastChild;

	idHierarchy<idTypeInfo> node;

	idTypeInfo(const std::string& classname, const std::string& superclass,
		idEventFunc<idClass>* eventCallbacks,
		std::shared_ptr<idClass> (*CreateInstance)(), void (idClass::*Spawn)());
	
	~idTypeInfo();

	void Init();
	void Shutdown();

	bool IsType(const idTypeInfo &superclass) const;
};

/*
================
idTypeInfo::IsType

Checks if the object's class is a subclass of the class defined by the
passed in idTypeInfo.
================
*/
inline bool idTypeInfo::IsType(const idTypeInfo& type) const {
	return ((typeNum >= type.typeNum) && (typeNum <= type.lastChild));
}

/*
================
idClass::IsType

Checks if the object's class is a subclass of the class defined by the
passed in idTypeInfo.
================
*/
inline bool idClass::IsType(const idTypeInfo& superclass) const {
	gsl::not_null<idTypeInfo*> subclass = GetType();
	return subclass->IsType(superclass);
}

#endif