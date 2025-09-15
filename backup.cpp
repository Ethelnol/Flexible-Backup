/** 
  * Created by Caitlyn Briggs on 16/05/2025
  **/

#include <unordered_map>

#include "backup.h"
#include "shared.h"

//hash table for paths already checked for removal by removeArchive()
std::unordered_map<size_t, path> remove_table;

bool backup(const path& entry){
	path archive = bacDir;
	archive += entry;
	archive += conExt;

	if (!isRealPath(archive.parent_path()))
		create_directories(archive.parent_path());

	if (isRealPath(archive) && last_write_time(entry) <= last_write_time(archive)){return false;}

	removeArchive(archive);

	std::string cmd = "sudo tar --absolute-names --directory=\"" +
	                  entry.parent_path().string() + "\" --create --file - \"" +
	                  entry.filename().string() +
	                  "\"";
	if (!comArgs.empty()){
		cmd += " | " + comArgs;
	}

	cmd += " > \"" + archive.string() + '\"';

	uint8_t ret = system(cmd.c_str());
	if (ret) sig_handler(ret, &archive);

	return true;
}

/**
  * Returns p without extensions after stopper or first stem of p if stopper isn't in p
  * @param ext populated with first extension after stem, stopper if stopper is present
  * @example removeArchive("/path/to/file.txt.tar.xz", ".tar") returns "file.txt" and ".tar"
  * @example removeArchive("/file.txt.tar.xz", ".mp4") returns "file" and ".txt"
  **/
path getStem(path p, const path& stopper, path& ext){
	while (p.stem().has_extension() && p.extension() != stopper){p = p.stem();}
	ext = p.extension();
	return p.stem();
}

bool removeArchive(path archive){
	bool ret = false;

	while (archive != bacDir){
		path ext;
		const path tar = ".tar";
		const path a_stem = getStem(archive, tar, ext);

		for (const path& i : directory_iterator(archive.parent_path())){
			if (!i.has_stem() || !i.has_extension()){continue;}

			const path i_stem = getStem(i, tar, ext);

			if (ext != tar || a_stem != i_stem){continue;}

			size_t i_hash = hash_value(i);
			if (remove_table.find(i_hash) != remove_table.end()){continue;}

			remove_table.insert({i_hash, i});

			if (remove_all(i) == UINTMAX_MAX){
				error("could not remove outdated archive, ", i.c_str());
			}

			ret = true;
		}

		archive = archive.parent_path();
	}

	return ret;
}