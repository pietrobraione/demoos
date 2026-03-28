#include "syscalls.h"
#include "../drivers/uart/uart.h"
#include "./fat32/fat.h"
#include "mm.h"
#include "fork.h"
#include "scheduler.h"
#include "utils.h"
#include "../common/string.h"
#include "../common/syscalls_types.h"
#include "../common/memory.h"
#include "filesystem.h"

#define MAX_PATH 128

struct PCB* search_process(int pid);

void syscall_write(char *buffer) {
  unsigned long kernel_buffer = user_to_kernel_address((unsigned long)buffer);
  uart_puts((char*)kernel_buffer);
}

void syscall_write_hex(int number) {
  uart_hex(number);
}

int syscall_clone() {
  return copy_process(0, 0, 0);
}

unsigned long syscall_malloc() {
  unsigned long address = allocate_kernel_page();
  if (!address) {
    return -1;
  }
  return address;
}

void syscall_exit() {
  exit_process();
}

int get_first_free_file_descriptor(struct PCB* process) {
  for (int i = 0; i < MAX_FILES_PER_PROCESS; i++) {
    if (process->files[i] == NULL) {
      return i;
    }
  }
  return -1;
}

int syscall_create_dir(char *dir_path) {
  int file_descriptor = get_first_free_file_descriptor(current_process);
  if (file_descriptor == -1) {
    return -1;
  }

  Dir* created_dir = (Dir*)allocate_kernel_page();
  int error = create_dir(dir_path, created_dir);
  if (error) {
    free_page((unsigned long)created_dir);
    return error;
  }

  current_process->files[file_descriptor] = (FatResource*)allocate_kernel_page();
  current_process->files[file_descriptor]->resource_type = RESOURCE_TYPE_FOLDER;
  current_process->files[file_descriptor]->d = created_dir;

  return file_descriptor;
}

int syscall_open_dir(const char *dir_path) {
  int file_descriptor = get_first_free_file_descriptor(current_process);
  if (file_descriptor == -1) {
    return -1;
  }

  Dir* dir = (Dir*)allocate_kernel_page();
  int error = open_dir((char*)dir_path, dir);
  if (error) {
    free_page((unsigned long)dir);
    return -1;
  }

  current_process->files[file_descriptor] = (FatResource*)allocate_kernel_page();
  current_process->files[file_descriptor]->resource_type = RESOURCE_TYPE_FOLDER;
  current_process->files[file_descriptor]->d = dir;

  return file_descriptor;
}

int syscall_open_file(char* file_path, uint8_t flags) {
  int file_descriptor = get_first_free_file_descriptor(current_process);
  if (file_descriptor == -1) {
    return -1;
  }
  
  File* file = (File*)allocate_kernel_page();
  int error = open_file(file_path, flags, file);
  if (error) {
    free_page((unsigned long)file);
    return -1;
  }

  current_process->files[file_descriptor] = (FatResource*)allocate_kernel_page();
  current_process->files[file_descriptor]->resource_type = RESOURCE_TYPE_FILE;
  current_process->files[file_descriptor]->f = file;

  return file_descriptor;
}

int syscall_close_file(int file_descriptor) {
  if (file_descriptor < 0 || file_descriptor >= MAX_FILES_PER_PROCESS) {
    return -1;
  }

  File* file = current_process->files[file_descriptor]->f;
  int error = fat_file_close(file);
  if (error) {
    return error;
  }

  free_page((unsigned long)file);

  current_process->files[file_descriptor] = (FatResource*)allocate_kernel_page();
  current_process->files[file_descriptor]->f = NULL;
  current_process->files[file_descriptor]->resource_type = 0;

  return 0;
}

int syscall_write_file(int file_descriptor, char *buffer, int len, int *bytes) {
  FatResource* resource = current_process->files[file_descriptor];
  if (resource == NULL) {
    return -1;
  }
  File* file = resource->f;
  int error = fat_file_write(file, buffer, len, bytes);
  return error;
}

int syscall_read_file(int file_descriptor, char *buffer, int len, int *bytes) {
  File *file = current_process->files[file_descriptor]->f;
  fat_file_seek(file, 0, FAT_SEEK_START);
  int error = fat_file_read(file, buffer, len, bytes);
  return error;
}

int syscall_get_next_entry(int file_descriptor, FatEntryInfo *entry_info) {
  FatResource *fat_resource = current_process->files[file_descriptor];
  Dir *dir = fat_resource->d;
  
  if (fat_resource->d == NULL) {
    return -1;
  }

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

  return 0;
}

void syscall_yield() {
  schedule();
}

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

int syscall_fork() {
  int pid = copy_process(0, 0, 0);
  return pid;
}

int syscall_send_message(int destination_pid, char* body) {
  struct PCB* destination_process = search_process(destination_pid);

  if (destination_process == NULL) {
      return -1;
  }

  return send_message(current_process, destination_process, body);
}

void syscall_receive_message(char* body) {
  receive_message(current_process, body);
}

int syscall_send_signal(int destination_pid, int signal_flag) {
  struct PCB* destination_process = search_process(destination_pid);

  if (destination_process == NULL) {
      return -1;
  }

  return send_signal(destination_process, signal_flag);
}

int syscall_exec(char* path, unsigned long* trap_frame, int n_arguments, char arguments[][SYSCALL_EXEC_ARGUMENT_DIMENSION]) {  
  int fd = syscall_open_file(path, FAT_READ);
  if (fd == -1) {
    return -1;
  }

  char* buffer = (char*)allocate_kernel_page();
  int read_bytes;
  syscall_read_file(fd, buffer, PAGE_SIZE * 16, &read_bytes);
  syscall_close_file(fd);

  // Code can't be more than 15 pages, because at page 16 there is the stack
  if (read_bytes > PAGE_SIZE * 15) {
    return -1;
  }

  int n_user_pages = current_process->mm.n_user_pages;
  for (int i = 0; i < n_user_pages; i++) {
    unsigned long user_page = current_process->mm.user_pages[i].physical_address + VA_START;
    free_page(user_page);

    current_process->mm.n_user_pages--;
    current_process->mm.user_pages[i].physical_address = 0;
    current_process->mm.user_pages[i].virtual_address = 0;
  }
  copy_code(current_process, buffer, read_bytes);

  // Now I have to save the arguments from the shell to the new user memory
  trap_frame[0] = n_arguments;
  unsigned long stack_pointer = 16 * PAGE_SIZE;
  for (int i = 0; i < n_arguments; i++) {
    stack_pointer -= SYSCALL_EXEC_ARGUMENT_DIMENSION;
    unsigned long* stack_pointer_kernel_address = (unsigned long*)user_to_kernel_address(stack_pointer);
    strcpy((char*)stack_pointer_kernel_address, arguments[i]);
    trap_frame[i + 1] = stack_pointer;
  }
  
  current_process->cpu_context.pc = 0;
  current_process->cpu_context.sp = stack_pointer;

  trap_frame[31] = stack_pointer;  // I reset the stack pointer
  trap_frame[32] = 0;              // I reset the program counter
  
  return 0;
}

int syscall_wait(int pid) {
  struct PCB* destination_process = search_process(pid);
  if (destination_process == NULL) {
    return -1;
  }

  current_process->state = PROCESS_WAITING_ANOTHER_PROCESS;
  current_process->pid_to_wait = pid;
  schedule();

  return 0;
}

void syscall_dispatcher(unsigned long* registers) {
  unsigned long syscall_number = registers[8];

  if (syscall_number > SYSCALLS_NUMBER) {
    return;
  }

  switch (syscall_number) {
    case SYSCALL_WRITE_NUMBER:
      syscall_write((char*)registers[0]);
      break;
    case SYSCALL_WRITE_HEX_NUMBER:
      syscall_write_hex((int)registers[0]);
      break;
    case SYSCALL_MALLOC_NUMBER:
      registers[0] = syscall_malloc();
      break;
    case SYSCALL_EXIT_NUMBER:
      syscall_exit();
      break;
    case SYSCALL_CREATE_DIR_NUMBER:
      registers[0] = syscall_create_dir((char*)registers[0]);
      break;
    case SYSCALL_OPEN_DIR_NUMBER:
      registers[0] = syscall_open_dir((char*)registers[0]);
      break;
    case SYSCALL_OPEN_FILE_NUMBER:
      registers[0] = syscall_open_file((char*)registers[0], (uint8_t)registers[1]);
      break;
    case SYSCALL_CLOSE_FILE_NUMBER:
      registers[0] = syscall_close_file((int)registers[0]);
      break;
    case SYSCALL_WRITE_FILE_NUMBER:
      registers[0] = syscall_write_file((int)registers[0], (char*)registers[1], (int)registers[2], (int*)registers[3]);
      break;
    case SYSCALL_READ_FILE_NUMBER:
      registers[0] = syscall_read_file((int)registers[0], (char*)registers[1], (int)registers[2], (int*)registers[3]);
      break;
    case SYSCALL_YIELD_NUMBER:
      syscall_yield();
      break;
    case SYSCALL_INPUT_NUMBER:
      registers[0] = syscall_input((char*)registers[0], (int)registers[1]);
      break;
    case SYSCALL_GET_NEXT_ENTRY_NUMBER:
      registers[0] = syscall_get_next_entry((int)registers[0], (FatEntryInfo*)registers[1]);
      break;
    case SYSCALL_FORK_NUMBER:
      registers[0] = syscall_fork();
      break;
    case SYSCALL_SEND_MESSAGE_NUMBER:
      registers[0] = syscall_send_message((int)registers[0], (char*)registers[1]);
      break;
    case SYSCALL_RECEIVE_MESSAGE_NUMBER:
      syscall_receive_message((char*)registers[0]);
      break;
    case SYSCALL_SEND_SIGNAL_NUMBER:
      registers[0] = syscall_send_signal((int)registers[0], (int)registers[1]);
      break;
    case SYSCALL_EXEC_NUMBER:
      char (*kernel_arguments)[SYSCALL_EXEC_ARGUMENT_DIMENSION] = (char(*)[SYSCALL_EXEC_ARGUMENT_DIMENSION])user_to_kernel_address(registers[2]);
      syscall_exec((char*)registers[0], registers, (int)registers[1], kernel_arguments);
      break;
    case SYSCALL_WAIT_NUMBER:
      registers[0] = syscall_wait((int)registers[0]);  
      break;
  }
}

struct PCB* search_process(int pid) {
  for (int i = 0; i < N_PROCESSES; i++) {
      if (processes[i]->pid == pid) {
          return processes[i];
      }
  }
  return NULL;
}
