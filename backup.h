/*
 * Created by Ethelnol on 16/05/2025
 */
#ifndef FLEXIBLEBACKUP_BACKUP_H
#define FLEXIBLEBACKUP_BACKUP_H

#include <filesystem>

/**
 * @brief Backs up <entry> to <bac_dir>/<entry>.<arc_ext>
 *
 * @pre <entry> exists and can be read from
 * @pre bac_dir exists and can be written to
 * @post any file that exists at <bac_dir>/<entry>.<arc_ext> will be removed and replaced with the new backup
 *
 * @param entry file or directory to be backed up
 */
bool backup(const std::filesystem::path& entry);

#endif //FLEXIBLEBACKUP_BACKUP_H