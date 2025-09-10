/** 
  * Created by Caitlyn Briggs on 18/07/2025
  **/
#ifndef FLEXIBLEBACKUP_CONFIG_H
#define FLEXIBLEBACKUP_CONFIG_H

/**
  * Open config and save details to shared.cpp variables
  * @return true if config was found
  **/
bool OpenConfig();

/**
  * Reads config file and populates arr1 and arr2 with results
  **/
void ReadConfig();

/**
  * Writes example config to loc
  */
void WriteConfig();

/**
  * Get and process arguments passed by command line
  * @param argc number of arguments
  * @param argv arguments
  **/
void GetArgs(int argc, char* argv[]);

#endif //FLEXIBLEBACKUP_CONFIG_H
