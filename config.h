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
  * Reads data from config
  **/
void ReadConfig();

/**
  * Writes default conf to config
  **/
void WriteConfig();

/**
  * Get and process arguments passed by command line
  * @param argc number of arguments
  * @param argv arguments
  **/
void GetArgs(int argc, char* argv[]);

#endif //FLEXIBLEBACKUP_CONFIG_H
