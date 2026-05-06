/**
  * Created by Ethelnol on 17/09/2025
  **/
#ifndef FLEXIBLE_BACKUP_OUT_H
#define FLEXIBLE_BACKUP_OUT_H

#include <cstdint>
#include <filesystem>

using std::filesystem::path;

enum STATUS{
	SCANNING = 0, //beginning processing
	BACKING,      //beginning backup process
	UNNEEDED,     //backup wasn't needed
	BACKED_UP,    //backed up successfully
	SKIPPING,     //not backing up
	DEEPER,       //recursing further into directory
	NUM_STATES    //enum for number of possible states
};
  
void log_init();

/**
  * Outputs (depth * TABS) spaces, msg, and p depending on step
  * @param step specifies which message to output
  **/
void out(uint32_t depth, const path& p, STATUS step);

#endif //FLEXIBLE_BACKUP_OUT_H
