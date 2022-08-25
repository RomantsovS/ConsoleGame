#include "precompiled.h"

/*
========================
idSysThread::idSysThread
========================
*/
idSysThread::idSysThread() :
	isWorker(false),
	isRunning(false),
	isTerminating(false),
	moreWorkToDo(false),
	signalWorkerDone(true) {
}

/*
========================
idSysThread::~idSysThread
========================
*/
idSysThread::~idSysThread() {
	StopThread(true);
	if (threadHandle.joinable())
		threadHandle.join();
}

/*
========================
idSysThread::StartThread
========================
*/
bool idSysThread::StartThread(const std::string& name_) {
	if (isRunning) {
		return false;
	}

	name = name_;

	isTerminating = false;

	if (threadHandle.joinable()) {
		threadHandle.detach();
	}

	threadHandle = std::thread(ThreadProc, this);

	isRunning = true;
	return true;
}

/*
========================
idSysThread::StartWorkerThread
========================
*/
bool idSysThread::StartWorkerThread(const std::string& name_) {
	if (isRunning) {
		return false;
	}

	isWorker = true;

	bool result = StartThread(name_);

	std::unique_lock lock(workerDoneMutex);
	signalWorkerDone_cv.wait(lock, [this] { return signalWorkerDone; });

	return result;
}

/*
========================
idSysThread::StopThread
========================
*/
void idSysThread::StopThread(bool wait) {
	if (!isRunning) {
		return;
	}
	if (isWorker) {
		//std::lock_guard lg(signalMutex);
		moreWorkToDo = true;
		signalWorkerDone = false;
		isTerminating = true;
	}
	else {
		isTerminating = true;
	}
	if (wait) {
		WaitForThread();
	}
}

/*
========================
idSysThread::WaitForThread
========================
*/
void idSysThread::WaitForThread() {
	if (isWorker) {
		//std::ostringstream oss;
		//oss << std::this_thread::get_id() << " idSysThread::WaitForThread()\n";
		//common->DPrintf(oss.str().c_str());
		std::unique_lock lock(workerDoneMutex);
		signalWorkerDone_cv.wait(lock, [this] { return signalWorkerDone; });
	}
	else if (isRunning) {
		threadHandle.detach();
	}
}

/*
========================
idSysThread::SignalWork
========================
*/
void idSysThread::SignalWork() {
	if (isWorker) {
		//std::ostringstream oss;
		//oss << std::this_thread::get_id() << " idSysThread::SignalWork()\n";
		//common->DPrintf(oss.str().c_str());
		std::lock_guard lg(moreWorkToDoMutex);
		moreWorkToDo = true;
		signalWorkerDone = false;
		signalMoreWorkToDo_cv.notify_one();
	}
}

/*
========================
idSysThread::IsWorkDone
========================
*/
bool idSysThread::IsWorkDone() {
	if (isWorker) {
		// a timeout of 0 will return immediately with true if signaled
		//if (signalWorkerDone.Wait(0)) {
			return true;
		//}
	}
	return false;
}

/*
========================
idSysThread::ThreadProc
========================
*/
int idSysThread::ThreadProc(idSysThread* thread) {
	int retVal = 0;

	try {
		if (thread->isWorker) {
			for (; ; ) {
				//std::ostringstream oss;
				//oss << std::this_thread::get_id() << " idSysThread::ThreadProc()\n";
				//common->DPrintf(oss.str().c_str());
				//std::lock_guard lg(thread->signalMutex);
				if (thread->moreWorkToDo) {
					thread->moreWorkToDo = false;
					//thread->signalMoreWorkToDo.Clear();
				}
				else {
					{
						std::unique_lock lock(thread->workerDoneMutex);
						thread->signalWorkerDone = true;
					}
					thread->signalWorkerDone_cv.notify_one();

					//std::ostringstream oss;
					//oss << std::this_thread::get_id() << " idSysThread::ThreadProc() after signalWorkerDone_cv.notify_one()\n";
					//common->DPrintf(oss.str().c_str());

					std::unique_lock lock(thread->moreWorkToDoMutex);
					thread->signalMoreWorkToDo_cv.wait(lock, [thread] { return thread->moreWorkToDo; });
					continue;
				}

				if (thread->isTerminating) {
					break;
				}

				retVal = thread->Run();
			}
			thread->signalWorkerDone = true;
			thread->signalWorkerDone_cv.notify_one();
		}
		else {
			retVal = thread->Run();
		}
	}
	catch (std::exception& ex) {
		idLib::Warning("Fatal error in thread %s: %s", thread->GetName(), ex.what());
		// We don't handle threads terminating unexpectedly very well, so just terminate the whole process
		_exit(0);
	}

	thread->isRunning = false;

	return retVal;
}

/*
========================
idSysThread::Run
========================
*/
int idSysThread::Run() {
	// The Run() is not pure virtual because on destruction of a derived class
	// the virtual function pointer will be set to NULL before the idSysThread
	// destructor actually stops the thread.
	return 0;
}