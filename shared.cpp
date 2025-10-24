/** 
  * Created by Caitlyn Briggs on 18/07/2025
  **/

#include <sys/stat.h>

#include "shared.h"

uid_t u_uid = 0; //user id
uint64_t maxSize = 0; //max directory size
string comArgs; //arguments for compression call
path config; //path to config
path conExt; //extension to affix to archive
path home; //user home directory
path bacRoot; //directory to begin backup in
path bacDir; //directory of archives
vector<gid_t> groups; //user groups
vector<path> whitelist, blacklist, split, collective;

bool isRealPath(const path& p, bool symlink){
	if (p.empty()) return false;
	if (!exists(p)) return false;
	if (is_other(p)) return false;
	if (!symlink && is_symlink(p)) return false;

	return true;
}

bool checkPerm(const path& p, char rwx){
	using std::filesystem::file_status;
	using std::filesystem::perms;

	const file_status s = status(p);

	uid_t e_uid; //p uid
	gid_t e_gid; //p gid
	{
		struct stat fileStat{};
		stat(p.c_str(), &fileStat);
		e_uid = fileStat.st_uid;
		e_gid = fileStat.st_gid;
	}

	//user, group, other
	perms perm[3] = {perms::none, perms::none, perms::none};
	
	switch (rwx){
		case 'r':{
			perm[0] = perms::owner_read;
			perm[1] = perms::group_read;
			perm[2] = perms::others_read;
			break;
		}
		case 'w':{
			perm[0] = perms::owner_write;
			perm[1] = perms::group_write;
			perm[2] = perms::others_write;
			break;
		}
		case 'x':{
			perm[0] = perms::owner_exec;
			perm[1] = perms::group_exec;
			perm[2] = perms::others_exec;
			break;
		}
		default:{
			std::cerr << "\n\ncheckPermissions() incorrect parameter: \""
			          << rwx << '"' << std::endl;
			exit(rwx);
		}
	}

	//check for owner permissions
	if (e_uid == u_uid){
		return (s.permissions() & perm[0]) != perms::none;
	}

	//check for group permissions for all user groups
	for (gid_t u_gid : groups){
		if (e_gid == u_gid){
			return (s.permissions() & perm[1]) != perms::none;
		}
	}

	//check for others' permissions
	return (s.permissions() & perm[2]) != perms::none;
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

void error(const string& error, const string& detail, uint8_t code){
	::error(error.c_str(), detail.c_str(), code);
}

void error(const char* error, const string& detail, uint8_t code){
	::error(error, detail.c_str(), code);
}

void error(const string& error, const char* detail, uint8_t code){
	::error(error.c_str(), detail, code);
}

void error(const char* error, const char* detail, uint8_t code){
	std::cerr << "Error: " << error << detail << std::endl;
	exit(code);
}