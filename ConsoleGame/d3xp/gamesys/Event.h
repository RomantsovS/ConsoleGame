#ifndef D3XP_GAMESYS_EVENT_H_
#define D3XP_GAMESYS_EVENT_H_

constexpr auto D_EVENT_MAXARGS = 8;

constexpr auto D_EVENT_VOID = ((char)0);
constexpr auto D_EVENT_INTEGER = 'd';
constexpr auto D_EVENT_FLOAT = 'f';
constexpr auto D_EVENT_VECTOR = 'v';
constexpr auto D_EVENT_STRING = 's';
constexpr auto D_EVENT_ENTITY = 'e';
constexpr auto D_EVENT_ENTITY_NULL = 'E'; // event can handle NULL entity pointers;
constexpr auto D_EVENT_TRACE = 't';

constexpr auto MAX_EVENTS = 4096;

class idClass;
class idTypeInfo;

class idEventDef {
private:
	std::string name;
	std::string formatspec;
	unsigned int formatspecIndex;
	int numargs;
	size_t argsize;
	int eventnum;

	static idEventDef* eventDefList[MAX_EVENTS];
	static int numEventDefs;
public:
	idEventDef(const std::string& command, const std::string& formatspec = "", char returnType = 0);

	const std::string GetName() const;
	const std::string GetArgFormat() const;
	unsigned int GetFormatspecIndex() const;
	int GetEventNum() const;
	int GetNumArgs() const;
	size_t GetArgSize() const;

	static int NumEventCommands();
};

class idEvent : public std::enable_shared_from_this<idEvent>  {
private:
	const idEventDef* eventdef = nullptr;
	unsigned char* data = nullptr;
	int time;
	idClass* object;
	const idTypeInfo* typeinfo;

	idLinkList<idEvent> eventNode;

	static std::allocator<unsigned char> eventDataAllocator;
public:
	static bool initialized;
	
	idEvent();
	~idEvent();
	idEvent(const idEvent&) = default;
	idEvent& operator=(const idEvent&) = default;
	idEvent(idEvent&&) = default;
	idEvent& operator=(idEvent&&) = default;

	static std::shared_ptr<idEvent> Alloc(gsl::not_null<const idEventDef*> evdef, int numargs, va_list args);
	//static void CopyArgs(const idEventDef* evdef, int numargs, va_list args, int data[D_EVENT_MAXARGS]);

	void Free(bool setOwner = true);
	void Schedule(idClass* object, const idTypeInfo* cls, int time);
	unsigned char* GetData();

	static void CancelEvents(const idClass* obj, const idEventDef* evdef = NULL);
	static void ClearEventList();
	static void ServiceEvents();
	static void Init();
	static void Shutdown();

};

/*
================
idEvent::GetData
================
*/
inline unsigned char* idEvent::GetData() {
	return data;
}

/*
================
idEventDef::GetName
================
*/
inline const std::string idEventDef::GetName() const {
	return name;
}

/*
================
idEventDef::GetArgFormat
================
*/
inline const std::string idEventDef::GetArgFormat() const {
	return formatspec;
}

/*
================
idEventDef::GetFormatspecIndex
================
*/
inline unsigned int idEventDef::GetFormatspecIndex() const {
	return formatspecIndex;
}

/*
================
idEventDef::GetNumArgs
================
*/
inline int idEventDef::GetNumArgs() const {
	return numargs;
}

/*
================
idEventDef::GetArgSize
================
*/
inline size_t idEventDef::GetArgSize() const {
	return argsize;
}

/*
================
idEventDef::GetEventNum
================
*/
inline int idEventDef::GetEventNum() const {
	return eventnum;
}

#endif