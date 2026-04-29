#include "filesystem.h"
#include "../drivers/sd/sd.h"
#include "../common/fat_types.h"
#include "../common/string.h"

Fat g_fat;

// Function which will be called from the FAT32 library that reads from the SD
bool sd_read_adapter(uint8_t *buffer, uint32_t sector) {
  int res = sd_readblock(sector, buffer, 1);
  return res != 0;
}

// Function which will be called from the FAT32 library that writes into the SD
bool sd_write_adapter(const uint8_t *buffer, uint32_t sector) {
  int res = sd_writeblock((unsigned char*)buffer, sector, 1);
  return res != 0;
}

// Probes the filesystem from the SD
int sd_filesystem_init() {
  int sd_result = sd_init();
  if (sd_result != SD_OK) {
    return SD_FILESYSTEM_INIT_ERROR;
  }

  DiskOps disk_operations = {sd_read_adapter, sd_write_adapter};

  int probe_error = fat_probe(&disk_operations, 0);
  if (probe_error) {
    return SD_FILESYSTEM_INIT_ERROR;
  }

  int mount_error = fat_mount(&disk_operations, 0, &g_fat, "mnt\0");
  if (mount_error) {
    return SD_FILESYSTEM_INIT_ERROR;
  }

  return SD_FILESYSTEM_INIT_OK;
}

// Creates a dir in the filesystem
int create_dir(char* dir_path, Dir* created_dir) {
  char complete_path[FAT_MAX_PATH_SIZE];
  strcpy(complete_path, "/mnt/");
  if (strlen(complete_path) + strlen(dir_path) >= FAT_MAX_PATH_SIZE) {
    return -1;
  }
  strcat(complete_path, dir_path);

  int error = fat_dir_create(created_dir, complete_path);
  return error;
}

// Opens an existing dir from the filesystem
int open_dir(char* dir_path, Dir* dir) {
  char complete_path[FAT_MAX_PATH_SIZE];
  strcpy(complete_path, "/mnt/");
  if (strlen(complete_path) + strlen(dir_path) >= FAT_MAX_PATH_SIZE) {
    return -1;
  }

  strcat(complete_path, dir_path);

  int error = fat_dir_open(dir, complete_path);
  return error;
}

// Opens a file from the filesystem
int open_file(char* file_path, uint8_t flags, File* file) {
  char complete_path[FAT_MAX_PATH_SIZE];
  strcpy(complete_path, "/mnt/");

  if (strlen(complete_path) + strlen(file_path) >= FAT_MAX_PATH_SIZE) {
    return -1;
  }
  strcat(complete_path, file_path);

  int error = fat_file_open(file, complete_path, flags);
  return error;
}
