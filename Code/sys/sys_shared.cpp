#include "idlib/precompiled.h"

int64_t Sys_Time() noexcept {
	return std::chrono::duration_cast<std::chrono::seconds>(
		std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

int64_t Sys_Milliseconds() noexcept {
	static auto sys_timeBase = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::high_resolution_clock::now() - sys_timeBase;
	return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

long long Sys_Microseconds() {
	static auto sys_timeBase = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::high_resolution_clock::now() - sys_timeBase;
	return std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
}

void Sys_Quit() noexcept {
	exit(EXIT_SUCCESS);
}

void Sys_Sleep(int msec) noexcept {
	std::this_thread::sleep_for(std::chrono::milliseconds(msec));
}

void Sys_Mkdir(const std::filesystem::path& path) noexcept {
	std::filesystem::create_directory(path);
}

std::filesystem::path Sys_DefaultBasePath() noexcept {
	return std::filesystem::current_path();
}

int Sys_ListFiles(const std::filesystem::path& directory, const std::filesystem::path& extension, std::vector<std::string>& list) {
	if (!std::filesystem::exists(directory))
		return 0;

	for (auto const& dir_entry : std::filesystem::directory_iterator{ directory }) {
		if (dir_entry.path().extension().string() == extension) {
			list.push_back(dir_entry.path().filename().string());
		}
	}

	return list.size();
}
