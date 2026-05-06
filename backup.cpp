/*
 * Created by Ethelnol on 16/05/2025
 */

#include <unordered_map>

#include "backup.h"
#include "shared.h"

//hash table for paths already checked for removal by removeArchive()
std::unordered_map<uint32_t, path> remove_table;

/**
 * @brief Check for and remove archives that would contain <archive>'s files
 *
 * @param archive path to archive to be removed
 *
 * @return true if any archive was removed or previously remove
 *
 * @details
    Iteratively checks <archive> or any of it's parent directories have a backup removes the first it finds. If archive
    was /path/to/file, the existence of <bac_dir>/path/to/file.<arc_ext> is evaluated followed by
    <bac_dir>/path/to.<arc_ext> and so on. If an archive of <archive> or it's parent directories is found, it is
    removed and the function returns true.
 */
bool removeArchive(path archive){
	const path stop_dir = bac_dir.string() + arc_ext.string();

	while (archive != stop_dir){
		const uint32_t a_hash   = hash_value(archive);
		const auto     rmv_path = remove_table.find(a_hash);

		//archive has already been removed
		if (rmv_path != remove_table.end()){
			return true;
		}

		std::error_code err;
		if (!remove(archive, err)){
			archive = archive.parent_path();
			archive += arc_ext;
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
	path archive = bac_dir;
	archive      += entry;
	archive      += arc_ext;

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

	if (!com_args.empty()){
		cmd += " | " + com_args;
	}

	cmd += " > \'" + archive.string() + '\'';

	if (const auto ret = system(cmd.c_str())){
		sig_handler(ret);
	}

	return true;
}
