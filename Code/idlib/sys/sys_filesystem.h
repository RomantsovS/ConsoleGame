#ifndef IDLIB_SYS_SYS_FILESYSTEM_H
#define IDLIB_SYS_SYS_FILESYSTEM_H

void Sys_Mkdir(const std::filesystem::path& path) noexcept;

// use fs_debug to verbose Sys_ListFiles
// returns -1 if directory was not found (the list is cleared)
int Sys_ListFiles(const std::filesystem::path& directory,
                  const std::filesystem::path& extension,
                  std::vector<std::string>& list);

#endif
