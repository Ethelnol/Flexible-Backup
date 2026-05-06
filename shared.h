/** 
  * Created by Ethelnol on 24/07/2025
  **/
#ifndef FLEXIBLEBACKUP_SHARED_H
#define FLEXIBLEBACKUP_SHARED_H

#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

using std::filesystem::path;
using std::string;
using std::vector;

extern uid_t u_uid;             //user id
extern uint64_t max_size;       //max directory size
extern string com_args;         //arguments for compression call
extern path config;             //path to config
extern path arc_ext;            //extension to affix to archive
extern path home;               //user home directory
extern path bac_root;           //directory to begin backup in
extern path bac_dir;            //directory of archives
extern vector<gid_t> groups;    //user groups
extern vector<path> whitelist;  //list of paths to backup
extern vector<path> blacklist;  //list of paths to exclude
extern vector<path> split;      //list of directories to forcefully backup subpaths instead of all together
extern vector<path> collective; //list of directories to forcefully instead of backing up subpaths

/**
  * @brief Check if <p> is a real path that is a directory or file
  **/
bool isRealPath(const path& p, bool allow_symlink = false);

/**
  * @brief Check if <p> has permission to read, write, or execute
  *
  * @return true if <p> has the requested permission
  **/
bool checkPerm(const path& p, char rwx);

/**
  * @brief Output error message and exit
  * @param signal signal to exit with
  **/
void sig_handler(int signal);

/**
  * @brief Output error message, remove <i_arch>, and exit
  * 
  * @param signal signal to exit with
  * @param i_arch incomplete archive path
  **/
void sig_handler(int signal, const path* i_arch);

/**
  * @brief Output "Error: <error><detail>" and exits with <code>
  *
  * @param code defaults to 1
  **/
void error(const string& error, const string& detail = "", uint8_t code = 1);

/**
  * @brief Output "Error: <error><detail>" and exits with <code>
  *
  * @param code defaults to 1
  **/
void error(const char* error, const string& detail = "", uint8_t code = 1);

/**
  * @brief Output "Error: <error><detail>" and exits with <code>
  *
  * @param code defaults to 1
  **/
void error(const string& error, const char* detail, uint8_t code = 1);

/**
  * @brief Output "Error: <error><detail>" and exits with <code>
  *
  * @param code defaults to 1
  **/
void error(const char* error, const char* detail, uint8_t code = 1);

#endif //FLEXIBLEBACKUP_SHARED_H
