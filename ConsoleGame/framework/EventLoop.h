#ifndef FRAMEWORK_EVENTLOOP_H__
#define FRAMEWORK_EVENTLOOP_H__

class idEventLoop {
public:
	idEventLoop();
	~idEventLoop();
	idEventLoop(const idEventLoop&) = default;
	idEventLoop& operator=(const idEventLoop&) = default;
	idEventLoop(idEventLoop&&) = default;
	idEventLoop& operator=(idEventLoop&&) = default;

	void			Init();

	// Closes the journal file if needed.
	void			Shutdown();

	// It is possible to get an event at the beginning of a frame that
	// has a time stamp lower than the last event from the previous frame.
	sysEvent_t		GetEvent();

	// Dispatches all pending events and returns the current time.
	int				RunEventLoop(bool commandExecution = true);

private:
	sysEvent_t		GetRealEvent();
	void			ProcessEvent(sysEvent_t ev);
};

extern	idEventLoop* eventLoop;

#endif // !FRAMEWORK_EVENTLOOP_H__
