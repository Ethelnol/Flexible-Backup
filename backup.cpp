/** 
  * Created by Caitlyn Briggs on 16/05/2025
  **/

#include <unordered_map>

#include "backup.h"
#include "shared.h"

//hash table for paths already checked for removal by removeArchive()
std::unordered_map<size_t, path> remove_table;

/**
  * Check for and remove other archives that would contain archive's files
  * @return true if any archive was removed or previously remove
  **/
bool removeArchive(path archive){
	const path stopDir = bacDir.string() + conExt.string();

	while (archive != stopDir){
		const size_t a_hash = hash_value(archive);
		const auto rmv_path = remove_table.find(a_hash);

		//archive has already been removed
		if (rmv_path != remove_table.end()){return true;}

		std::error_code err;
		if (!remove(archive, err)){
			archive = archive.parent_path();
			archive += conExt;
			continue;
		}
		if (err){
			error("could not remove archive \"", archive.string() + '\"');
		}

		remove_table.emplace(a_hash, archive);
		return true;
	}

	return false;
}

bool backup(const path& entry){
	path archive = bacDir;
	archive += entry;
	archive += conExt;

	if (!exists(archive.parent_path())){
		create_directories(archive.parent_path());
	}

	if (exists(archive) && last_write_time(entry) <= last_write_time(archive)){
		return false;
	}

	removeArchive(archive);

	//set directory to parent path so archived paths are relative to entry
	std::string cmd = "sudo tar --absolute-names --directory=\'" +
					  entry.parent_path().string() + "\' --create --file - \'" +
					  entry.filename().string() + "\'";
	if (!comArgs.empty()){cmd += " | " + comArgs;}

	cmd += " > \'" + archive.string() + '\'';

	const auto ret = system(cmd.c_str());
	if (ret){sig_handler(ret, &archive);}

	return true;
}