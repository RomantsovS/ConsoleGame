#include <time.h>
#include <cstdarg>

#include "Common_local.h"
#include "FileSystem.h"
#include "../sys/sys_public.h"
#include "../idlib/Str.h"

void idCommonLocal::Printf(const char* fmt, ...)
{
	va_list argptr;
	va_start(argptr, fmt);
	VPrintf(fmt, argptr);
	va_end(argptr);
}

void idCommonLocal::VPrintf(const char* fmt, va_list args)
{
	static bool	logFileFailed = false;

	// optionally put a timestamp at the beginning of each print,
	// so we can see how long different init sections are taking
	int timeLength = 0;
	char msg[MAX_PRINT_MSG_SIZE];
	msg[0] = '\0';
	if (true) {
		int	t = Sys_Milliseconds();
		if (true) {
			sprintf_s(msg, MAX_PRINT_MSG_SIZE, "[%5.2f]", t * 0.001f);
		}
		else {
			sprintf_s(msg, MAX_PRINT_MSG_SIZE, "[%i]", t);
		}
	}
	timeLength = strlen(msg);

	if (idStr::vsnPrintf(msg + timeLength, MAX_PRINT_MSG_SIZE - timeLength - 1, fmt, args) < 0)
	{
		msg[sizeof(msg) - 2] = '\n'; msg[sizeof(msg) - 1] = '\0'; // avoid output garbling
		//Sys_Printf("idCommon::VPrintf: truncated to %d characters\n", strlen(msg) - 1);
	}

	// logFile
	if (fileSystem->IsInitialized()) {
		static bool recursing;

		if (!logFile && !recursing) {
			// fileSystem->OpenFileWrite can cause recursive prints into here
			recursing = true;

			time_t aclock;
			tm newtime;
			
#ifdef LOG_FILE_NAME_TIME
			time(&aclock);
			localtime_s(&newtime, &aclock);

			char cur_local_time[50];
			cur_local_time[0] = '\0';

			sprintf_s(cur_local_time, 50, "qconsole_%2d-%2d-%2d", newtime.tm_hour, newtime.tm_min, newtime.tm_sec);

			std::string fileName = cur_local_time;
#else
			std::string fileName = "qconsole";
#endif // LOG_FILE_NAME_TIME

			if (log_file_closed)
				fileName += "_additional";

			fileName += ".log";

			logFile = fileSystem->OpenFileWrite(fileName);
			if (!logFile) {
				logFileFailed = true;
				FatalError("failed to open log file '%s'\n", fileName);
			}

			recursing = false;

			time(&aclock);
			localtime_s(&newtime, &aclock);
			char buf[256];

			asctime_s(buf, &newtime);

			Printf("log file '%s' opened on %s\n", fileName.c_str(), buf);
		}
		if (logFile) {
			logFile->Write(msg);
			logFile->Flush();	// ForceFlush doesn't help a whole lot
		}
	}
}

void idCommonLocal::DPrintf(const char* fmt, ...)
{
	va_list		argptr;
	char		msg[MAX_PRINT_MSG_SIZE];

	va_start(argptr, fmt);
	idStr::vsnPrintf(msg, sizeof(msg), fmt, argptr);
	va_end(argptr);
	msg[sizeof(msg) - 1] = '\0';

	Printf("%s", msg);
}

/*
==================
idCommonLocal::DWarning

prints warning message in yellow that only shows up if the "developer" cvar is set
==================
*/
void idCommonLocal::DWarning(const char* fmt, ...) {
	va_list		argptr;
	char		msg[MAX_PRINT_MSG_SIZE];

	va_start(argptr, fmt);
	idStr::vsnPrintf(msg, sizeof(msg), fmt, argptr);
	va_end(argptr);
	msg[sizeof(msg) - 1] = '\0';

	Printf("WARNING: %s\n", msg);
}

void idCommonLocal::CloseLogFile()
{
	if (logFile) {
		Printf("log file closed\n");

		fileSystem->CloseFile(logFile);

		log_file_closed = true;

		logFile = nullptr;

	}
}

void idCommonLocal::Warning(const char* fmt, ...)
{
	va_list		argptr;
	char		msg[MAX_PRINT_MSG_SIZE];

	va_start(argptr, fmt);
	idStr::vsnPrintf(msg, sizeof(msg), fmt, argptr);
	va_end(argptr);
	msg[sizeof(msg) - 1] = 0;

	Printf("WARNING: " "%s\n", msg);
}

void idCommonLocal::Error(const char* fmt, ...)
{
	va_list		argptr;
	static int	lastErrorTime;
	static int	errorCount;
	int			currentTime;

	errorParm_t code = ERP_DROP;

	// if we got a recursive error, make it fatal
	if (com_errorEntered) {
		// if we are recursively erroring while exiting
		// from a fatal error, just kill the entire
		// process immediately, which will prevent a
		// full screen rendering window covering the
		// error dialog
		if (com_errorEntered == ERP_FATAL) {
			Sys_Quit();
		}
		code = ERP_FATAL;
	}

	// if we are getting a solid stream of ERP_DROP, do an ERP_FATAL
	currentTime = Sys_Milliseconds();
	if (currentTime - lastErrorTime < 100) {
		if (++errorCount > 3) {
			code = ERP_FATAL;
		}
	}
	else {
		errorCount = 0;
	}
	lastErrorTime = currentTime;

	com_errorEntered = code;

	va_start(argptr, fmt);
	idStr::vsnPrintf(errorMessage, sizeof(errorMessage), fmt, argptr);
	va_end(argptr);
	errorMessage[sizeof(errorMessage) - 1] = '\0';

	Stop();

	Printf("********************\nERROR: %s\n********************\n", errorMessage);

	Sys_Error("%s", errorMessage);
}

void idCommonLocal::FatalError(const char* fmt, ...)
{
	va_list		argptr;

	if (com_errorEntered) {
		// if we are recursively erroring while exiting
		// from a fatal error, just kill the entire
		// process immediately, which will prevent a
		// full screen rendering window covering the
		// error dialog

		va_start(argptr, fmt);
		idStr::vsnPrintf(errorMessage, sizeof(errorMessage), fmt, argptr);
		va_end(argptr);
		errorMessage[sizeof(errorMessage) - 1] = '\0';

		// write the console to a log file?
		Sys_Quit();
	}
	com_errorEntered = ERP_FATAL;

	va_start(argptr, fmt);
	idStr::vsnPrintf(errorMessage, sizeof(errorMessage), fmt, argptr);
	va_end(argptr);
	errorMessage[sizeof(errorMessage) - 1] = '\0';

	Sys_Error("%s", errorMessage);
}