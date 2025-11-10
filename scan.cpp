/** 
  * Created by Caitlyn Briggs on 16/05/2025
  **/

#include <csignal>
#include "grp.h"
#include <pwd.h>
#include <queue>

#include "backup.h"
#include "config.h"
#include "out.h"
#include "scan.h"
#include "shared.h"

int main(int argc, char* argv[]){
	signal(SIGINT, sig_handler);

	//set uid and gid information
	{
		char* user = getenv("USER");
		passwd* login = getpwnam(getlogin());
		int ngroups = 0;
		uint32_t group = login->pw_gid;
		u_uid = login->pw_uid;
		home = login->pw_dir;

		config = (home / ".config/flexible-backup.conf");

		getgrouplist(user, group, nullptr, &ngroups);
		groups.resize(ngroups);
		getgrouplist(user, group, groups.data(), &ngroups);
	}

	GetArgs(argc, argv);

	if (!OpenConfig()){return 1;}

	if (!exists(bacDir)){create_directories(bacDir);}
	if (!checkPerm(bacDir, 'w')){
		error("backup directory is not writable", bacDir);
	}

	log_init();

	//predefined whitelisted directories
	if (!whitelist.empty()){
		bool ret;
		for (const path& entry : (whitelist)){
			if (scan(0, entry)){ret = true;}
		}
		return ret;
	}
	//no predefined whitelisted directories
	return plunge(0, bacRoot);
}

bool plunge(const size_t depth, const path& dir){
	bool ret = false;

	for (const path& entry : directory_iterator(dir)){
		if (scan(depth, entry)){ret = true;}
	}

	std::cout << std::endl;

	return ret;
}

/**
  * scan helper function, handles backup text output and returns backup value
  **/
bool startBackup(const size_t depth, const path& entry){
	Backing(depth, entry);

	bool ret = backup(entry);

	if (ret){Backed(depth, entry);}
	else{A_Backed(depth, entry);}

	return ret;
}

/**
  * scan helper function, checks if vec contains p or p is a subpath of vec
  * @param invert true if checking that vec contains entries are subpath of p or are p
  **/
bool vecSearch(const path& p, const vector<path>& vec, const bool invert){
	if (!invert){
		for (const path& root : vec){
			if (isSubPath(root, p) || root == p){return true;}
		}
	}
	else{
		for (const path& entry : vec){
			if (isSubPath(p, entry) || p == entry){return true;}
		}
	}

	return false;
}

bool scan(const size_t depth, const path& entry){
	Scanning(depth, entry);

	if (!isRealPath(entry) || !checkPerm(entry, 'r') ||
	    vecSearch(entry, blacklist, false)){
		Skipping(depth, entry);
		return false;
	}

	if (!is_directory(entry) || vecSearch(entry, collective, false)){
		return startBackup(depth, entry);
	}

	//has blacklisted or split subentry
	if (vecSearch(entry, blacklist, true) ||
	    vecSearch(entry, split, true) ||
	    getSize(entry) > maxSize){
		Deeper(depth, entry);
		return plunge(depth + 1, entry);
	}

	return startBackup(depth, entry);
}

bool isSubPath(const path& root, const path& entry){
	//iterators
	path::iterator i_root = root.begin(), i_entry = entry.begin();
	//stop values
	const path::iterator s_root = root.end(), s_entry = entry.end();

	while (i_root != s_root && i_entry != s_entry){
		if (*i_root++ != *i_entry++){return false;}
	}

	return (i_entry != s_entry);
}

uint64_t getSize(const path& root){
	if (!isRealPath(root) || !checkPerm(root, 'r')){return 0;}
	if (!is_directory(root)){return file_size(root);}

	std::queue<path> q;
	q.push(root);

	uint64_t ret = 0;
	while (!q.empty()){
		for (const path& i : directory_iterator(q.front())){
			if (!isRealPath(i) || !checkPerm(i, 'r')){continue;}
			if (is_directory(i)){q.push(i);}
			else{ret = ret + file_size(i);}
		}
		q.pop();
	}

	return ret;
}