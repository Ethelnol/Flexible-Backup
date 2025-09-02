/** 
  * Created by Caitlyn Briggs on 24/07/2025
  **/
#ifndef FLEXIBLEBACKUP_SHARED_H
#define FLEXIBLEBACKUP_SHARED_H

#include <array>
#include <csignal>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

using namespace std::filesystem;
using std::string;
using std::vector;

extern uid_t u_uid; //user id
extern uint64_t maxSize;
extern string comArgs;
extern path bacRoot, bacDir, home, conExt;
extern vector<gid_t> groups;
extern vector<path> whitelist, blacklist, split, collective;

/**
  * Check if entry is a real path that is a directory or file
  * @param symlink allow for entry to be a symlink
  * @return
  **/
bool isRealPath(const path& entry, bool symlink = false);

bool checkPerm(const path& entry, char rwx);

/**
  * Output error message and exit
  * @param signal signal to exit with
  **/
void sig_handler(int signal);

/**
  * Output error message, cleanup incomplete archive if one exists, and exit
  * @param signal signal to exit with
  * @param i_arch incomplete archive path
  **/
void sig_handler(int signal, path* i_arch);

/**
  * Output "Error: <error><detail>" and exits with code
  * @param code defaults to 1
  **/
void error(const string& error, const string& detail = "", uint8_t code = 1);

/**
  * Output "Error: <error><detail>" and exits with code
  * @param code defaults to 1
  **/
void error(const char* error, const char* detail, uint8_t code = 1);

#endif //FLEXIBLEBACKUP_SHARED_H
