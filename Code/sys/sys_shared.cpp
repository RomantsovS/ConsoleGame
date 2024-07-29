#include "idlib/precompiled.h"

int64_t Sys_Time() noexcept {
	return std::chrono::duration_cast<std::chrono::seconds>(
		std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}