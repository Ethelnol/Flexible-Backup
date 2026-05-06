/*
 * Created by Ethelnol on 18/07/2025
 */
#ifndef FLEXIBLEBACKUP_CONFIG_H
#define FLEXIBLEBACKUP_CONFIG_H

/**
 * @brief Open config and save details to global "shared.cpp" variables
 *
 * @return true if config was found
 */
bool OpenConfig();

/**
 * @brief Reads data from config
 *
 * @details
    Reads config and assigns data to global variables in "shared.cpp".  Fixes instances of unassigned variables such as
    <max_size>, adds <bac_dir> to <blacklist> if missing, sets the command arguments for <com_args>, and reduces size of
    <whitelist>, <blacklist>, <split>, and <collective> lists
 */
void ReadConfig();

/**
 * @brief Writes default conf to config
 */
void WriteConfig();

/**
 * @brief Get and process arguments passed by command line
 *
 * @param argc number of arguments
 * @param argv arguments
 */
void GetArgs(int argc, const char* argv[]);

#endif //FLEXIBLEBACKUP_CONFIG_H
