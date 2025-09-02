/** 
  * Created by Caitlyn Briggs on 18/07/2025
  **/

#include <sys/stat.h>

#include "shared.h"

uid_t u_uid; //user id
uint64_t maxSize; //max directory size
string comArgs; //arguments for compression call
path conExt; //extension to affix to archive
path home; //user home directory
path bacRoot; //directory to begin backup in
path bacDir; //directory of archives
vector<gid_t> groups; //user groups
vector<path> whitelist, blacklist, split, collective;

bool isRealPath(const path& entry, bool symlink){
	if (entry.empty()) return false;
	if (!exists(entry)) return false;
	if (is_other(entry)) return false;
	if (!symlink && is_symlink(entry)) return false;

	return true;
}

bool checkPerm(const path& entry, const char rwx){
	const file_status s = status(entry);

	uid_t e_uid; //entry uid
	gid_t e_gid; //entry gid
	{
		struct stat fileStat{};
		stat(entry.c_str(), &fileStat);
		e_uid = fileStat.st_uid;
		e_gid = fileStat.st_gid;
	}

	//user, group, other
	std::array<perms, 3> p{perms::none, perms::none, perms::none};
	
	switch (rwx){
		case 'r':{
			p = {perms::owner_read,  perms::group_read,  perms::others_read};
			break;
		}
		case 'w':{
			p = {perms::owner_write, perms::group_write, perms::others_write};
			break;
		}
		case 'x':{
			p = {perms::owner_exec, perms::group_exec, perms::others_exec,};
			break;
		}
		default:{
			std::cerr << "\n\ncheckPermissions() incorrect parameter: \""
			          << rwx << '"' << std::endl;
			exit(rwx);
		}
	}

	//check for owner permissions
	if (e_uid == u_uid)
		return (s.permissions() & p[0]) != perms::none;

	//check for group permissions for all user groups
	for (gid_t u_gid : groups)
		if (e_gid == u_gid)
			return (s.permissions() & p[1]) != perms::none;

	//check for others' permissions
	if ((s.permissions() & p[2]) != perms::none)
		return true;

	return false;
}

void sig_handler(int signal){
	sig_handler(signal, nullptr);
}

void sig_handler(int signal, path* i_arch){
	using std::cerr, std::endl;
	cerr << endl << endl << "Stopping backup." << endl;

	if (i_arch && isRealPath(*i_arch)){
		cerr << "Archive incomplete, removing: " << i_arch->filename() << '.' << endl;
		remove(*i_arch);
	}

	exit(signal);
}

void error(const string& error, const string& detail, const uint8_t code){
	::error(error.c_str(), detail.c_str(), code);
}

void error(const char* error, const char* detail, const uint8_t code){
	std::cerr << "Error: " << error;

	if (detail) std::cerr << detail;

	std::cerr << std::endl;

	exit(code);
}