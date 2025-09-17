/** 
  * Created by Caitlyn Briggs on 16/05/2025
  **/

#include <csignal>
#include "grp.h"
#include <pwd.h>
#include <unistd.h>

#include "backup.h"
#include "config.h"
#include "scan.h"
#include "shared.h"

#define tab(d) (4 * d) //outputs tab for cout
#define out(m) (std::cout << string(tab(depth), ' ') \
                          << std::setw(17) << std::right \
						  << m << " : " << entry.filename() \
						  << std::flush)
#define full(d, e) (tab(d) + 22 + e.string().length()) //setw(17) + " : " + '\"' + e.length() + '\"'

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

	if (!isRealPath(bacDir)){create_directories(bacDir);}
	if (!checkPerm(bacDir, 'w')){
		error("backup directory is not writable", bacDir);
	}

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
	using std::cout;

	const size_t backNum = full(depth, entry);

	cout << string(backNum, '\b');
	out("Backing up");
	cout << string(backNum, '\b');

	bool ret = backup(entry);
	out(((ret) ? "Backed up" : "Already backed up"));
	cout << std::endl;
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
	using std::cout;

	out("Scanning");

	if (!isRealPath(entry) || !checkPerm(entry, 'r') ||
	    vecSearch(entry, blacklist, false)){
		cout << string(((full(depth, entry))), '\b');
		out("Skipping");
		cout << std::endl;
		return false;
	}

	if (!is_directory(entry) || vecSearch(entry, collective, false))
		return startBackup(depth, entry);

	//has blacklisted or split subentry
	if (vecSearch(entry, blacklist, true) ||
	    vecSearch(entry, split, true) ||
	    getSize(entry) > maxSize){
		cout << string((full(depth, entry)), '\b');
		out("Scanning deeper");
		cout << std::endl << std::endl;
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

	uint64_t ret = 0;
	for (const path& entry : directory_iterator(root)){
		if (ret > maxSize){return ret;}
		ret += getSize(entry);
	}
	return ret;
}