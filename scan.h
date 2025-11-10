/** 
  * Created by Caitlyn Briggs on 16/05/2025
  **/
#ifndef FLEXIBLEBACKUP_SCAN_H
#define FLEXIBLEBACKUP_SCAN_H

#include <filesystem>

using namespace std::filesystem;

/**
  * Gets list of paths under dir and passes path to scan
  * @param depth number of directories recursed from conf[bacRoot]
  * @return true if any path under dir was backed up
  **/
bool plunge(size_t depth, const path& dir);

/**
  * Backups, calls plunge with, or skips entry from backup
  * @param depth number of directories recursed from bacRoot
  * @return true if entry or any path under entry was backed up
  **/
bool scan(size_t depth, const path& entry);

/**
  * Checks if entry is a subpath of root
  * @return false if root and entry are the same
  **/
bool isSubPath(const path& root, const path& entry);

/**
  * Returns size in bytes of root iteratively
  * @pre root is a real path
  * @return 0 if root is not readable
  **/
uint64_t getSize(const path& root);

#endif //FLEXIBLEBACKUP_SCAN_H