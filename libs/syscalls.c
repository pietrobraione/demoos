#include "syscalls.h"
#include "../drivers/uart/uart.h"
#include "./fat32/fat.h"
#include "allocator.h"
#include "fork.h"
#include "scheduler.h"
#include "utils.h"
#include "../common/string.h"

#define MAX_PATH 128

void syscall_write(char *buffer) {
  unsigned long kernel_buffer = user_to_kernel_address((unsigned long)buffer);
  uart_puts((char*)kernel_buffer);
}

int syscall_clone() { return copy_process(0, 0, 0); }

unsigned long syscall_malloc() {
  unsigned long address = allocate_kernel_page();
  if (!address) {
    return -1;
  }
  return address;
}

void syscall_exit() { exit_process(); }

int syscall_create_dir(char *dir_relative_path) {
  char complete_path[MAX_PATH];
  strcpy(complete_path, "/mnt/");
  if (strlen(complete_path) + strlen(dir_relative_path) >= MAX_PATH) {
    return -1;
  }
  strcat(complete_path, dir_relative_path);

  int file_descriptor = -1;
  for (int i = 0; i < MAX_FILES_PER_PROCESS; i++) {
    if (current_process->files[i] == NULL) {
      file_descriptor = i;
      break;
    }
  }

  Dir *dir = (Dir *)allocate_kernel_page();
  int error = fat_dir_create(dir, complete_path);
  if (error) {
    return -1;
  }

  current_process->files[file_descriptor]->resource_type = RESOURCE_TYPE_FOLDER;
  current_process->files[file_descriptor]->d = dir;

  return file_descriptor;
}

int syscall_open_dir(const char *dir_relative_path) {
  char complete_path[MAX_PATH];
  strcpy(complete_path, "/mnt/");
  if (strlen(complete_path) + strlen(dir_relative_path) >= MAX_PATH) {
    return -1;
  }

  strcat(complete_path, dir_relative_path);
  
  int file_descriptor = -1;
  for (int i = 0; i < MAX_FILES_PER_PROCESS; i++) {
    if (current_process->files[i] == NULL) {
      file_descriptor = i;
      break;
    }
  }
  
  Dir *dir = (Dir *)allocate_kernel_page();
  int error = fat_dir_open(dir, complete_path);
  
  if (error) {
    return -1;
  }

  current_process->files[file_descriptor] = (FatResource*)allocate_kernel_page();
  current_process->files[file_descriptor]->resource_type = RESOURCE_TYPE_FOLDER;
  current_process->files[file_descriptor]->d = dir;

  return file_descriptor;
}

int syscall_open_file(char *file_relative_path, uint8_t flags) {
  char complete_path[MAX_PATH];
  strcpy(complete_path, "/mnt/");

  if (strlen(complete_path) + strlen(file_relative_path) >= MAX_PATH) {
    return -1;
  }
  strcat(complete_path, file_relative_path);

  int file_descriptor = -1;
  for (int i = 0; i < MAX_FILES_PER_PROCESS; i++) {
    if (current_process->files[i] == NULL) {
      file_descriptor = i;
      break;
    }
  }

  File *file = (File *)allocate_kernel_page();
  int error = fat_file_open(file, complete_path, flags);

  if (error) {
    return -1;
  }

  current_process->files[file_descriptor]->resource_type = RESOURCE_TYPE_FILE;
  current_process->files[file_descriptor]->f = file;
  return file_descriptor;
}

int syscall_close_file(int file_descriptor) {
  FatResource *fat_resource = current_process->files[file_descriptor];
  File *file = fat_resource->f;
  int error = fat_file_close(file);
  return error;
}

int syscall_write_file(int file_descriptor, char *buffer, int len, int *bytes) {
  FatResource *fat_resource = current_process->files[file_descriptor];
  File *file = fat_resource->f;
  int error = fat_file_write(file, buffer, len, bytes);
  return error;
}

int syscall_read_file(int file_descriptor, char *buffer, int len, int *bytes) {
  FatResource *fat_resource = current_process->files[file_descriptor];
  File *file = fat_resource->f;
  fat_file_seek(file, 0, FAT_SEEK_START);
  int error = fat_file_read(file, buffer, len, bytes);
  return error;
}

void syscall_yield() { schedule(); }

struct PCB *uart_owner = NULL;

int syscall_input(char *buffer, int len) {
  int current_len = 0;

  while (uart_owner != NULL && uart_owner != current_process) {
    schedule();
  }

  uart_owner = current_process;

  int max_len = (len > 0) ? len - 1 : 0;

  while (1) {
    if (uart_head == uart_tail) {
      current_process->state = PROCESS_WAITING_UART_INPUT;
      schedule();
      continue;
    }

    while (uart_tail != uart_head) {
      char c = uart_buffer[uart_tail];
      uart_tail = (uart_tail + 1) % UART_BUFFER_SIZE;

      uart_putc(c);

      if (c == '\r' || c == '\n') {
        // Se il carattere è invio, termino di occupare il buffer
        uart_owner = NULL;
        buffer[current_len] = '\0';  // termina la stringa
        return current_len;
      } else if (c == 0x7F) {
        // Se il carattere è backspace, cancello l'ultimo carattere scritto
        if (current_len > 0) {
          uart_puts("\b \b");
          current_len--;
          buffer[current_len] = '\0';
        }
      } else {
        if (current_len < max_len) {
            buffer[current_len] = c;
            current_len++;
            buffer[current_len] = '\0';
        }
      }
    }
  }
}

int syscall_get_next_entry(int file_descriptor, FatEntryInfo *entry_info) {
  FatResource *fat_resource = current_process->files[file_descriptor];
  Dir *dir = fat_resource->d;
  
  DirInfo dir_info;
  int error = fat_dir_read(dir, &dir_info);

  if (error) {
    return -1;
  }

  int size = dir_info.name_len < 255 ? dir_info.name_len : 255;
  for (int i = 0; i < size; i++) {
    entry_info->name[i] = dir_info.name[i];
  }
  entry_info->is_dir = (dir_info.attr & FAT_ATTR_DIR) ? 1 : 0;

  error = fat_dir_next(dir);
  if (error) {
    return -1;
  }

  return 1;
}

int syscall_fork() {
  int pid = copy_process(0, 0, 0);
  return pid;
}

int syscall_send_message(int destination_pid, MessageType message_type, char* body) {
  if (message_type == MESSAGE_TYPE_RAW) {
    return send_message(current_process, destination_pid, message_type, body);
  } else {
    uart_puts("[KERNEL] Message received from the kernel\n");
  }

  return 0;
}

void syscall_receive_message(MessageType message_type, char* body) {
  receive_message(current_process, message_type, body);
}

void *const sys_call_table[] = {
    syscall_write,          syscall_malloc,     syscall_clone,
    syscall_exit,           syscall_create_dir, syscall_open_dir,
    syscall_open_file,      syscall_close_file, syscall_write_file,
    syscall_read_file,      syscall_yield,      syscall_input,
    syscall_get_next_entry, syscall_fork,
    syscall_send_message,   syscall_receive_message
};
