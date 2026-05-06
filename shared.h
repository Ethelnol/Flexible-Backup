/** 
  * Created by Ethelnol on 24/07/2025
  **/
#ifndef FLEXIBLEBACKUP_SHARED_H
#define FLEXIBLEBACKUP_SHARED_H

#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

using std::filesystem::path;
using std::string;
using std::unordered_map;
using std::vector;

enum listType{
	blacklist           = 0b000,
	split               = 0b010,
	collective          = 0b100,
};

extern uid_t u_uid;
extern uint64_t maxSize;
extern string comArgs;
extern path config, bacRoot, bacDir, home, conExt;
extern vector<gid_t> groups;
extern vector<path> whitelist;
extern unordered_map<uint32_t, path> list[6];

/**
  * Check if p is a real path that is a directory or file
  **/
bool isRealPath(const path& p, bool allow_symlink = false);

/**
  * Check if list[<t>] contains <p>
  **/
bool pathInList(const path& p, uint8_t t);

/**
  * Check if a path p has permission to read, write, or execute
  * @return true if p has the requested permission
  **/
bool checkPerm(const path& p, char rwx);

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
void sig_handler(int signal, const path* i_arch);

/**
  * Output "Error: (error)(detail)" and exits with code
  * @param code defaults to 1
  **/
void error(const string& error, const string& detail = "", uint8_t code = 1);

/**
  * Output "Error: (error)(detail)" and exits with code
  * @param code defaults to 1
  **/
void error(const char* error, const string& detail = "", uint8_t code = 1);

/**
  * Output "Error: (error)(detail)" and exits with code
  * @param code defaults to 1
  **/
void error(const string& error, const char* detail, uint8_t code = 1);

/**
  * Output "Error: (error)(detail)" and exits with code
  * @param code defaults to 1
  **/
void error(const char* error, const char* detail, uint8_t code = 1);

#endif //FLEXIBLEBACKUP_SHARED_H
