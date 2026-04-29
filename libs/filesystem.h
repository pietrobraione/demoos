#ifndef __SD_FILESYSTEM_H
#define __SD_FILESYSTEM_H

#define SD_FILESYSTEM_INIT_OK      0
#define SD_FILESYSTEM_INIT_ERROR   1

#include "./fat32/fat.h"

// Variable that represents the mounted file system and which is shared between all the filesystem operations
extern Fat g_fat;

int sd_filesystem_init();
int create_dir(char* dir_path, Dir* created_dir);
int open_dir(char* dir_path, Dir* dir);
int open_file(char* file_path, uint8_t flags, File* file);

#endif
