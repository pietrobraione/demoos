#ifndef __FAT_TYPES_H
#define __FAT_TYPES_H

/**
 * fat_types.h contains the definition of the constants and structures which will be used to handle
 * the filesystem functions
 */

#include <stdint.h>
#include <stdbool.h>

#define FAT_MAX_NAME_SIZE 64
#define FAT_MAX_PATH_SIZE 128

// Enums moved from fat.h
enum
{
  FAT_ERR_NONE     =  0,
  FAT_ERR_NOFAT    = -1,
  FAT_ERR_BROKEN   = -2,
  FAT_ERR_IO     = -3,
  FAT_ERR_PARAM    = -4,
  FAT_ERR_PATH     = -5,
  FAT_ERR_EOF      = -6,
  FAT_ERR_DENIED   = -7,
  FAT_ERR_FULL     = -8,
};

enum
{
  FAT_ATTR_NONE     = 0x00,
  FAT_ATTR_RO       = 0x01,
  FAT_ATTR_HIDDEN   = 0x02,
  FAT_ATTR_SYS      = 0x04,
  FAT_ATTR_LABEL    = 0x08,
  FAT_ATTR_DIR      = 0x10,
  FAT_ATTR_ARCHIVE  = 0x20,
  FAT_ATTR_LFN      = 0x0f,
};

enum
{
  FAT_WRITE      = 0x01, // Open file for writing
  FAT_READ       = 0x02, // Open file for reading
  FAT_APPEND     = 0x04, // Set file offset to the end of the file
  FAT_TRUNC      = 0x08, // Truncate the file after opening
  FAT_CREATE     = 0x10, // Create the file if it do not exist
};

enum
{
  FAT_SEEK_START,
  FAT_SEEK_CURR,
  FAT_SEEK_END,
};

// Struct which contains the data about a resource in the FAT filesystem. This struct is returned 
// from the syscalls which gets the FAT resources data (like syscall_get_next_entry)
typedef struct {
    uint32_t size;
    char name[FAT_MAX_NAME_SIZE];
    bool is_dir;
} FatEntryInfo;

#endif
