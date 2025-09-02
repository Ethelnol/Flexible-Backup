/** 
  * Created by Caitlyn Briggs on 18/07/2025
  **/
#ifndef FLEXIBLEBACKUP_CONFIG_H
#define FLEXIBLEBACKUP_CONFIG_H

#include <filesystem>
#include <string>

/**
  * Open config and save details to shared.cpp variables
  * @param loc location of the config file, defaults to $HOME/.config/flexible_backup.conf
  * @return true if config was found
  **/
bool OpenConfig(const std::string& loc = "");

/**
  * Reads config file and populates arr1 and arr2 with results
  * @param arr1 backup root, backup dir, compression information, and max file size
  * @param arr2 whitelist, blacklist, split scan, collective
  **/
void ReadConfig(const std::filesystem::path& config);

/**
  * Writes example config to loc
  * @param loc location of the config file, defaults to $HOME/.config/flexible_backup.conf
  */
void WriteConfig(const std::filesystem::path& conf);

#endif //FLEXIBLEBACKUP_CONFIG_H
