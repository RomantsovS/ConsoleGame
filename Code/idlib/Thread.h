#ifndef IDLIB_THREAD_H
#define IDLIB_THREAD_H

class idSysThread {
public:
	idSysThread();
	virtual ~idSysThread();

	const std::string GetName() const { return name; }
	bool IsRunning() const { return isRunning; }
	bool IsTerminating() const { return isTerminating; }

	//------------------------
	// Thread Start/Stop/Wait
	//------------------------

	bool StartThread(const std::string& name_);

	bool StartWorkerThread(const std::string& name_);

	void StopThread(bool wait = true);

	// This can be called from multiple other threads. However, in the case
	// of a worker thread, the work being "done" has little meaning if other
	// threads are continuously signalling more work.
	void WaitForThread();

	//------------------------
	// Worker Thread
	//------------------------

	// Signals the thread to notify work is available.
	// This can be called from multiple other threads.
	void SignalWork();

	// Returns true if the work is done without waiting.
	// This can be called from multiple other threads. However, the work
	// being "done" has little meaning if other threads are continuously
	// signalling more work.
	bool IsWorkDone();

protected:
	// The routine that performs the work.
	virtual int Run();

private:
	std::string name;
	std::thread threadHandle;
	bool isWorker;
	bool isRunning;
	volatile bool isTerminating;
	volatile bool moreWorkToDo;
	volatile bool signalWorkerDone;

	//std::mutex signalMutex;
	std::mutex workerDoneMutex;
	std::condition_variable signalWorkerDone_cv;
	std::mutex moreWorkToDoMutex;
	std::condition_variable signalMoreWorkToDo_cv;

	static int ThreadProc(idSysThread* thread);
};

#endif