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
	idEventArg(int data) noexcept { type = D_EVENT_INTEGER; value = data; };
	idEventArg(float data) noexcept { type = D_EVENT_FLOAT; value = *reinterpret_cast<int*>(&data); };
	// idEventArg(Vector2& data) noexcept { type = D_EVENT_VECTOR; value = reinterpret_cast<int>(&data); };
	// idEventArg(const std::string& data) noexcept { type = D_EVENT_STRING; value = reinterpret_cast<int>(data.c_str()); };
	// idEventArg(const char* data) noexcept { type = D_EVENT_STRING; value = reinterpret_cast<int>(data); };
	// idEventArg(const class idEntity* data) noexcept { type = D_EVENT_ENTITY; value = reinterpret_cast<int>(data); };
	// idEventArg(const struct trace_s* data) noexcept { type = D_EVENT_TRACE; value = reinterpret_cast<int>(data); };
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
	idTypeInfo *GetType() const noexcept override;		\
	static	idEventFunc<nameofclass> eventCallbacks[]

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
	idTypeInfo *nameofclass::GetType() const noexcept {														\
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
	virtual idTypeInfo *GetType() const noexcept;		\
	static idEventFunc<nameofclass> eventCallbacks[]

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
	idTypeInfo *nameofclass::GetType() const noexcept {												\
		return &( nameofclass::Type );																\
	}																								\
	idEventFunc<nameofclass> nameofclass::eventCallbacks[] = {

typedef void (idClass::*classSpawnFunc_t)();

class idClass
{
public:
	ABSTRACT_PROTOTYPE(idClass);
	
	idClass() = default;
	virtual ~idClass();
	idClass(const idClass&) = default;
	idClass& operator=(const idClass&) = default;
	idClass(idClass&&) = default;
	idClass& operator=(idClass&&) = default;

	void Spawn() noexcept;
	void CallSpawn();
	bool IsType(const idTypeInfo &c) const;
	const std::string GetClassname() const;

	bool PostEventMS(const idEventDef* ev, int time);

	bool PostEventSec(const idEventDef* ev, float time);

	bool ProcessEventArgPtr(const idEventDef* ev, int* data);
	void CancelEvents(const idEventDef* ev);

	void Event_Remove() noexcept;
	virtual void Remove() noexcept;

	// Static functions
	static void Init();
	static void	Shutdown() noexcept;
	static idTypeInfo * GetClass(const std::string& name) noexcept;
	static idTypeInfo* GetType(int num);
private:
	classSpawnFunc_t CallSpawnFunc(gsl::not_null<idTypeInfo*> cls);

	bool PostEventArgs(const idEventDef* ev, int time, int numargs, ...);

	static bool initialized;
	static std::vector<idTypeInfo*> types;
	static std::vector<idTypeInfo*> typenums;
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
	idTypeInfo(const idTypeInfo&) = default;
	idTypeInfo& operator=(const idTypeInfo&) = default;
	idTypeInfo(idTypeInfo&&) = default;
	idTypeInfo& operator=(idTypeInfo&&) = default;


	void Init();
	void Shutdown() noexcept;

	bool IsType(const idTypeInfo &superclass) const noexcept;
};

/*
================
idTypeInfo::IsType

Checks if the object's class is a subclass of the class defined by the
passed in idTypeInfo.
================
*/
inline bool idTypeInfo::IsType(const idTypeInfo& type) const noexcept {
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