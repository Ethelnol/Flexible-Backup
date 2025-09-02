/** 
  * Created by Caitlyn Briggs on 16/05/2025
  **/
#ifndef FLEXIBLEBACKUP_BACKUP_H
#define FLEXIBLEBACKUP_BACKUP_H

#include <filesystem>
#include <string>

/**
  * Backs up entry to conf[bacDir]
  * @param conf array containing config information
  * @pre entry exists can be read from
  * @pre conf.at(bacDir) exists can be written to
  * @return true if backup was attempted
  * @return false if newer archive exists (already backed up)
  **/
bool backup(const std::filesystem::path& entry);

/**
  * Check for and remove other archives that would contain archive's files
  * @return true if any archive was removed
  **/
bool removeArchive(std::filesystem::path archive);

#endif //FLEXIBLEBACKUP_BACKUP_H