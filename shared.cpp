/** 
  * Created by Ethelnol on 18/07/2025
  **/

#include <sys/stat.h>

#include "shared.h"

uid_t u_uid = 0;
uint64_t max_size = 0;
string com_args;
path config, arc_ext, home, bac_root, bac_dir;
vector<gid_t> groups;
vector<path> whitelist, blacklist, split, collective;

bool isRealPath(const path& p, const bool allow_symlink){
	if (!allow_symlink && is_symlink(p)){
		return false;
	}

	return !is_other(p);
}

bool checkPerm(const path& p, const char rwx){
	using std::filesystem::perms;

	const perms p_perms = status(p).permissions();
	if (p_perms == perms::unknown){
		return false;
	}

	perms check_perms;
	switch (rwx){
		case 'r':{
			//0x124
			//444
			//100 100 100
			check_perms = perms::owner_read | perms::group_read | perms::others_read;
			break;
		}
		case 'w':{
			//0x812
			//222
			//010 010 010
			check_perms = perms::owner_write | perms::group_write | perms::others_write;
			break;
		}
		case 'x':{
			//0x49
			//111
			//001 001 001
			check_perms = perms::owner_exec | perms::group_exec | perms::others_exec;
			break;
		}
		default:{
			std::cerr << "\n\ncheckPermissions() incorrect parameter: \""
					  << rwx << '"' << std::endl;
			exit(rwx);
		}
	}
	check_perms &= p_perms;

	struct stat p_stats{};
	stat(p.c_str(), &p_stats);

	//check for owner permissions
	if (p_stats.st_uid == u_uid){
		return (perms::owner_all & check_perms) != perms::none;
	}

	//check for group permissions for all user groups
	for (gid_t u_gid : groups){
		if (p_stats.st_gid == u_gid){
			return (perms::group_all & check_perms) != perms::none;
		}
	}

	//check for others' permissions
	return (perms::others_all & check_perms) != perms::none;
}

void sig_handler(const int signal){
	std::cerr << "\n\nStopping backup.\n";
	exit(signal);
}

void sig_handler(const int signal, path* i_arch){
	using std::cerr, std::endl;
	cerr << "\n\nStopping backup.\n";

	if (i_arch && exists(*i_arch)){
		cerr << "Archive incomplete, removing: " << i_arch->filename() << ".\n";
		remove(*i_arch);
	}

	exit(signal);
}

void error(const string& error, const string& detail, const uint8_t code){
	::error(error.c_str(), detail.c_str(), code);
}

void error(const char* error, const string& detail, const uint8_t code){
	::error(error, detail.c_str(), code);
}

void error(const string& error, const char* detail, const uint8_t code){
	::error(error.c_str(), detail, code);
}

void error(const char* error, const char* detail, const uint8_t code){
	std::cerr << "Error: " << error << detail << '\n';
	exit(code);
}