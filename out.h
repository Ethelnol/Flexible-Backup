/** 
  * Created by Caitlyn Briggs on 17/09/2025
  **/
#ifndef FLEXIBLE_BACKUP_OUT_H
#define FLEXIBLE_BACKUP_OUT_H

#include <cstdint>
#include <filesystem>

using std::filesystem::path;
  
void log_init();

void Scanning(uint32_t depth, const path& p);

void Skipping(const uint32_t depth, const path& p);

void Deeper(const uint32_t depth, const path& p);

void Backing(const uint32_t depth, const path& p);

void Backed(const uint32_t depth, const path& p);

void A_Backed(const uint32_t depth, const path& p);

#endif //FLEXIBLE_BACKUP_OUT_H
