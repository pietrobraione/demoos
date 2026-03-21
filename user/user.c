#include "user.h"
#include "../common/user_syscalls.h"
#include "../common/string.h"
#include "../common/memory.h"
#include "../common/ipc_types.h"
#include "../common/syscalls_types.h"

#define UART_NORMAL_COLOR "\x1B[0m\0"
#define UART_RED_COLOR "\x1B[31m\0"
#define UART_GREEN_COLOR "\x1B[32m\0"
#define UART_YELLOW_COLOR "\x1B[33m\0"
#define UART_BLUE_COLOR "\x1B[34m\0"
#define UART_WHITE_COLOR "\x1B[37m\0"
#define UART_CLEAR_SCREEN "\e[1;1H\e[2J\0"

// Max dimension of a path handled by the shell
#define MAX_PATH_DIMENSION 64
// Max dimension of a command handled by the shell
#define MAX_COMMAND_DIMENSION 64
// Max dimension of the buffer which handles files operations
#define MAX_FILE_DIMENSION 256
// Max number of arguments that can be passed to exec
#define MAX_EXEC_ARGUMENTS 4

void handle_help();
void handle_ls(char* buffer, char* working_directory);
void handle_pwd(char* working_directory);
void handle_mkdir(char* buffer, char* working_directory);
void handle_cd(char* buffer, char* working_directory);
void handle_write(char* buffer, char* working_directory);
void handle_show(char* buffer, char* working_directory);
void handle_tree(char *buffer, char *working_directory);
void print_tree(const char *path, int depth);
void normalize_path(char* path);
void handle_fork_and_messages();
void handle_signals();
void handle_exec(char* buffer, char* working_directory);
void handle_exec_from_bin(char* buffer);

void shell() {
  char working_directory[MAX_PATH_DIMENSION] = "/";

  char* ascii_art[6] = {
    "▄▄▄▄▄▄                          ▄▄▄▄▄    ▄▄▄▄▄▄▄ ",
    "███▀▀██▄                      ▄███████▄ █████▀▀▀ ",
    "███  ███ ▄█▀█▄ ███▄███▄ ▄███▄ ███   ███  ▀████▄  ",
    "███  ███ ██▄█▀ ██ ██ ██ ██ ██ ███▄▄▄███    ▀████ ",
    "██████▀  ▀█▄▄▄ ██ ██ ██ ▀███▀  ▀█████▀  ███████▀ ",
  };

  call_syscall_write(UART_GREEN_COLOR);
  for (int i = 0; i < 5; i++) {
    call_syscall_write(ascii_art[i]);
    call_syscall_write("\n");
  }
  call_syscall_write(UART_WHITE_COLOR);
  call_syscall_write("\n");

  while (1) {
    call_syscall_write(UART_GREEN_COLOR);
    call_syscall_write("demoos:\0");
    call_syscall_write(UART_BLUE_COLOR);
    call_syscall_write(working_directory);
    call_syscall_write(UART_WHITE_COLOR);
    call_syscall_write("$ \0");

    char buffer[MAX_COMMAND_DIMENSION];
    memset(buffer, 0, MAX_COMMAND_DIMENSION);
    
    call_syscall_input(buffer, MAX_COMMAND_DIMENSION);
    call_syscall_write("\n\0");

    if (strlen(buffer) == 0) {
      continue;
    }

    if (memcmp(buffer, "help", 4) == 0) {
      handle_help();
    } else if (memcmp(buffer, "ls", 2) == 0) {
      handle_ls(buffer, working_directory);
    } else if (memcmp(buffer, "pwd", 3) == 0) {
      handle_pwd(working_directory);
    } else if (memcmp(buffer, "mkdir", 5) == 0) {
      handle_mkdir(buffer, working_directory);
    } else if (memcmp(buffer, "cd", 2) == 0) {
      handle_cd(buffer, working_directory);
    } else if (memcmp(buffer, "tree", 4) == 0) {
      handle_tree(buffer, working_directory);
    } else if (memcmp(buffer, "clear", 5) == 0) {
      call_syscall_write(UART_CLEAR_SCREEN);
    } else if (memcmp(buffer, "write", 5) == 0) {
      handle_write(buffer, working_directory);
    } else if (memcmp(buffer, "show", 4) == 0) {
      handle_show(buffer, working_directory);
    } else if (memcmp(buffer, "fork", 4) == 0) {
      handle_fork_and_messages();
    } else if (memcmp(buffer, "signals", 7) == 0) {
      handle_signals();
    } else if (memcmp(buffer, "exec", 4) == 0) {
      handle_exec(buffer, working_directory);
    } else {
      handle_exec_from_bin(buffer);
    }
  }

  call_syscall_exit();
}

void handle_help() {
    call_syscall_write("[demoos shell - 0.0.1]\n\0");
    call_syscall_write("Available commands:\n\0");
    call_syscall_write("  help       - Show this help message\n\0");
    call_syscall_write("  pwd        - Show the current working directory\n\0");
    call_syscall_write("  ls         - Show content of the current folder\n\0");
    call_syscall_write("  tree       - Show directory tree\n\0");
    call_syscall_write("  mkdir      - Create a directory in the working directory\n\0");
    call_syscall_write("  write      - Creates a file and writes the given content in it\n\0");
    call_syscall_write("  show       - Shows the content of the given file\n\0");
    call_syscall_write("  clear      - Clears the screen\n\0");
    call_syscall_write("  fork       - Forks the current process, and the new one will send a message to the father\n\0");
    call_syscall_write("  signals    - Tests the signals by creating and killing a process\n\0");
    call_syscall_write("  exec       - Forks the current process and launches a new one from the file system\n\0");
}

void handle_ls(char *buffer, char *working_directory) {
  int fd = call_syscall_open_dir(working_directory);
  if (fd == -1) {
    call_syscall_write("[SHELL] Error opening folder '\0");
    call_syscall_write(buffer);
    call_syscall_write("'.\n\0");
  }

  FatEntryInfo info;
  while (1) {
      memset(info.name, 0, FAT_MAX_NAME_SIZE);
      int result = call_syscall_get_next_entry(fd, &info);
      if (result) {
          break;
      }

      if (info.is_dir) {
          call_syscall_write("\x1b[34m\0");
          call_syscall_write(info.name);
          call_syscall_write("\x1b[0m\0");
      } else {
          call_syscall_write(info.name);
      }

      call_syscall_write("\n");
  }

  call_syscall_write("\n");
}

void handle_pwd(char *working_directory) {
  call_syscall_write(working_directory);
  call_syscall_write("\n");
}

void handle_mkdir(char *buffer, char *working_directory) {
  char command[MAX_COMMAND_DIMENSION] = {0};
  char dir_name[MAX_COMMAND_DIMENSION] = {0};
  strsplit(buffer, ' ', command, dir_name);

  if (*dir_name == 0) {
    call_syscall_write("[SHELL] Error: please specify the directory name.\n");
    return;
  }

  char temp[MAX_PATH_DIMENSION];
  memset(temp, 0, MAX_PATH_DIMENSION);

  if (strlen(working_directory) < MAX_PATH_DIMENSION) {
    strcat(temp, working_directory);
  } else {
    call_syscall_write("[SHELL] Error: working directory string is too big.\n");
    return;
  }

  if (dir_name[0] != '/') {
    strcat(temp, "/");
  }
  strcat(temp, dir_name);

  int fd = call_syscall_create_dir(temp);
  if (fd == -1) {
    call_syscall_write("[SHELL] Cannot create '");
    call_syscall_write(temp);
    call_syscall_write("'.\n");
  }
}

void handle_cd(char *buffer, char *working_directory) {
  char command[MAX_COMMAND_DIMENSION] = {0};
  char destination[MAX_COMMAND_DIMENSION] = {0};
  char temp[MAX_COMMAND_DIMENSION] = {0};

  strsplit(buffer, ' ', command, destination);

  int destination_len = strlen(destination);
  if (destination[destination_len - 1] != '/') {
    strcat(destination, "/\0");
  }

  if (destination[0] == '/') {
    // If the path is absolute I don't need to use working directory
    memcpy(temp, destination, MAX_PATH_DIMENSION);
  } else {
    // If the path is relative, I append the working directory
    memcpy(temp, working_directory, MAX_PATH_DIMENSION);
    int len = strlen(temp);

    if (len > 2 && temp[len - 1] != '/') {
      temp[len] = '/';
      temp[len + 1] = '\0';
    }
    strcat(temp, destination);
  }

  normalize_path(temp);

  if (call_syscall_open_dir(temp) == -1) {
    call_syscall_write("[SHELL] Error: cannot change directory to '");
    call_syscall_write(temp);
    call_syscall_write("'.\n");
    return;
  }

  memcpy(working_directory, temp, MAX_PATH_DIMENSION);
}

void handle_show(char* buffer, char* working_directory) {
  char command[MAX_COMMAND_DIMENSION] = {0};
  char destination[MAX_COMMAND_DIMENSION] = {0};
  char temp[MAX_PATH_DIMENSION] = {0};

  strsplit(buffer, ' ', command, destination);

  if (destination[0] == '/') {
    // If the path is absolute I don't need to use working directory
    memcpy(temp, destination, MAX_PATH_DIMENSION);
  } else {
    // If the path is relative, I append the working directory
    memcpy(temp, working_directory, MAX_PATH_DIMENSION);
    int len = strlen(temp);

    if (len > 2 && temp[len - 1] != '/') {
      temp[len] = '/';
      temp[len + 1] = '\0';
    }
    strcat(temp, destination);
  }

  normalize_path(temp);

  int fd = call_syscall_open_file(temp, FAT_READ);
  if (fd == -1) {
    call_syscall_write("[SHELL] Cannot open '");
    call_syscall_write(temp);
    call_syscall_write("'.\n");
  }

  if (fd > -1) {
    char file_content[MAX_FILE_DIMENSION] = {0};
    int read_bytes;
    int error = call_syscall_read_file(fd, file_content, MAX_FILE_DIMENSION, &read_bytes);
    if (error) {
      call_syscall_write("[SHELL] Cannot read '");
      call_syscall_write(temp);
      call_syscall_write("'.\n");
    } else {
      call_syscall_write(file_content);
      call_syscall_write("\n");
    }

    call_syscall_close_file(fd);
  }
}

void normalize_path(char* path) {
  int read = 0, write = 0;

  while (path[read] != '\0') {
    if (path[read] == '.' &&
        (path[read + 1] == '/' || path[read + 1] == '\0')) {
      // I skip the "./" because it's useless to build the path
      read += (path[read + 1] == '/') ? 2 : 1;
      continue;
    } else if (path[read] == '.' && path[read + 1] == '.' &&
               (path[read + 2] == '/' || path[read + 2] == '\0')) {
      read += (path[read + 2] == '/') ? 3 : 2;

      write--;
      while (write > 0 && path[write - 1] != '/') {
        write--;
      }

      continue;
    }

    path[write] = path[read];
    write++;
    read++;
  }

  path[write] = '\0';
  if (write == 0) {
    path[write] = '/';
    path[write + 1] = '\0';
  }
}

void print_tree(const char *path, int depth) {
  int fd = call_syscall_open_dir((char*)path);
  if (fd == -1) {
    return;
  }
  FatEntryInfo info;
  while (1) {
    memset(info.name, 0, FAT_MAX_NAME_SIZE);
    int result = call_syscall_get_next_entry(fd, &info);
    if (result != 1) {
      break;
    }
    if ((memcmp(info.name, ".", 2) == 0) || (memcmp(info.name, "..", 3) == 0)) {
      continue;
    }
    for (int i = 0; i < depth; i++) {
      call_syscall_write("   ");
    }
    if (info.is_dir) {
      call_syscall_write("\x1b[34m");
      call_syscall_write(info.name);
      call_syscall_write("/\x1b[0m\n");
    } else {
      call_syscall_write(info.name);
      call_syscall_write("\n");
    }
    if (info.is_dir) {
      char child_path[MAX_PATH_DIMENSION] = {0};
      strcpy(child_path, path);
      int len = strlen(child_path);
      if (child_path[len - 1] != '/') {
        strcat(child_path, "/");
      }
      strcat(child_path, info.name);
      normalize_path(child_path);
      print_tree(child_path, depth + 1);
    }
  }
}

void handle_tree(char *buffer, char *working_directory) {
  char command[MAX_COMMAND_DIMENSION] = {0};
  char target[MAX_COMMAND_DIMENSION] = {0};
  char fullpath[MAX_PATH_DIMENSION] = {0};
  strsplit(buffer, ' ', command, target);
  if (target[0] == 0) {
    memcpy(fullpath, working_directory, MAX_PATH_DIMENSION);
  } else if (target[0] == '/') {
    memcpy(fullpath, target, MAX_PATH_DIMENSION);
  } else {
    memcpy(fullpath, working_directory, MAX_PATH_DIMENSION);
    int len = strlen(fullpath);
    if (len > 1 && fullpath[len - 1] != '/') {
      strcat(fullpath, "/");
    }
    strcat(fullpath, target);
  }
  normalize_path(fullpath);
  int fd = call_syscall_open_dir(fullpath);
  if (fd == -1) {
    call_syscall_write("[SHELL] Error: cannot open directory '");
    call_syscall_write(fullpath);
    call_syscall_write("'.\n");
    return;
  }
  call_syscall_write(fullpath);
  call_syscall_write("\n");
  print_tree(fullpath, 0);
}

void handle_write(char* buffer, char* working_directory) {
  char file_name[MAX_COMMAND_DIMENSION] = {0};
  char file_content[MAX_COMMAND_DIMENSION] = {0};

  char* p = buffer;
  while (*p != '\0' && *p != ' ') {
    p++;
  }

  p++;

  int i = 0;
  while (*p != '\0' && *p != ' ') {
    file_name[i] = *p;
    p++;
    i++;
  }

  file_name[i] = '\0';

  p++;

  i = 0;
  while (*p != '\0') {
    file_content[i] = *p;
    p++;
    i++;
  }

  file_content[i] = '\0';

  char file_path[MAX_PATH_DIMENSION] = {0};
  if (strlen(working_directory) + strlen(file_name) < MAX_PATH_DIMENSION) {
    strcat(file_path, working_directory);
    strcat(file_path, file_name);
  } else {
    call_syscall_write("[SHELL] Error: file path is too big\n");
  }

  int fd = call_syscall_open_file(file_path, FAT_CREATE | FAT_WRITE);
  if (fd == -1) {
    call_syscall_write("[SHELL] Error: cannot open file '");
    call_syscall_write(file_path);
    call_syscall_write("'.\n");
  }

  if (fd != -1 && i > 0) {
    int written_bytes;
    int error = call_syscall_write_file(fd, file_content, MAX_FILE_DIMENSION, &written_bytes);
    if (error) {
      call_syscall_write("[SHELL] Error: cannot write on file '");
      call_syscall_write(file_path);
      call_syscall_write("'.\n");
    }
  }

  call_syscall_close_file(fd);
}

void handle_fork_and_messages() {
  int pid = call_syscall_fork();  
  if (pid == 0) {
    // The son will send 5 messages to the father
    char* messages[] = {
      "hello", "world", "demoos", "is", "great!"
    };
    call_syscall_write("[SON] Sending message to father\n");
    for (int i = 0; i < 5; i++) {
      int ok = call_syscall_send_message(0, messages[i]);
      if (ok == -1) {
        call_syscall_write("[SON] Error sending message to father.\n");
        break;
      }
      call_syscall_write("[SON] Message sent to father.\n");
    }
    call_syscall_exit();
  } else {
    // This yield forces the child process to start sending messages, so the father buffer will be
    // filled and the user process will need to be blocked and resumed
    call_syscall_yield();

    char buffer[MAX_FILE_DIMENSION];
    for (int i = 0; i < 5; i++) {
      memset(buffer, 0, MAX_FILE_DIMENSION);
      call_syscall_receive_message(buffer);
      call_syscall_write("[FATHER] Message received from SON: '");
      call_syscall_write(buffer);
      call_syscall_write("'.\n");
    }
  }
}

void handle_signals() {
  int pid = call_syscall_fork();
  if (pid == 0) {
    call_syscall_write("[SON] Started...\n");
    while (1) {
      call_syscall_write("[SON] I am the son and I'm using the CPU just for fun\n");
      call_syscall_yield();
    }
    call_syscall_write("[SON] This line should never be printed\n");
  } else {
    call_syscall_yield();
    call_syscall_send_signal(pid, SIGNAL_KILL);
    call_syscall_write("[FATHER] I terminate my son.\n");
  }
}

void handle_exec(char* buffer, char* working_directory) {
  char command[MAX_COMMAND_DIMENSION] = {0};
  char path[MAX_COMMAND_DIMENSION] = {0};

  strsplit(buffer, ' ', command, path);
  
  if (strlen(path) == 0) {
    call_syscall_write("[SHELL] Please specify the path of the new program to execute.\n");
    return;
  }

  char complete_path[MAX_PATH_DIMENSION] = {0};
  strcat(complete_path, working_directory);
  strcat(complete_path, path);
  normalize_path(complete_path);
  int pid = call_syscall_fork();
  if (pid == 0) {
    call_syscall_write("[SON] I am the son.\n");
    int error = call_syscall_exec(complete_path, 0, NULL);
    if (error) {
      call_syscall_write("[SON] Error running new process.\n");
      call_syscall_exit();
    }
    
    while (1) {
      call_syscall_write("[SON] This line should never be printed.\n");
    }
  } else {
    int ok = call_syscall_wait(pid);
    if (ok) {
      call_syscall_write("[SHELL] Cannot find process '");
      call_syscall_write_hex(pid);
      call_syscall_write("'\n");
    }
  }
}

void handle_exec_from_bin(char* buffer) {
  char file_name[MAX_PATH_DIMENSION] = {0};
  memzero(file_name, MAX_PATH_DIMENSION);
  char arguments_raw[MAX_PATH_DIMENSION] = {0};
  memzero(arguments_raw, MAX_PATH_DIMENSION);

  strsplit(buffer, ' ', file_name, arguments_raw);

  char complete_path[MAX_PATH_DIMENSION] = {0};
  strcat(complete_path, "/bin/");
  strcat(complete_path, file_name);
  strcat(complete_path, ".bin");
  normalize_path(complete_path);

  int n_arguments = 0;
  char arguments[MAX_EXEC_ARGUMENTS][MAX_PATH_DIMENSION];
  memzero(arguments, MAX_EXEC_ARGUMENTS * MAX_PATH_DIMENSION);

  while (strlen(arguments_raw) > 0 && n_arguments < MAX_EXEC_ARGUMENTS) {
    char temp[] = {0};
    memzero(temp, SYSCALL_EXEC_ARGUMENT_DIMENSION);

    char argument[SYSCALL_EXEC_ARGUMENT_DIMENSION] = {0};
    memzero(temp, SYSCALL_EXEC_ARGUMENT_DIMENSION);

    strsplit(arguments_raw, ' ', arguments[n_arguments], temp);

    n_arguments++;
    
    strcpy(arguments_raw, temp);
  }

  int pid = call_syscall_fork();
  if (pid == 0) {
    int error = call_syscall_exec(complete_path, n_arguments, arguments);
    if (error) {
      call_syscall_write("[SHELL] Cannot find '");
      call_syscall_write(file_name);
      call_syscall_write("' binary file.\n");
      call_syscall_exit();
    }
  } else {
    int ok = call_syscall_wait(pid);
    if (ok) {
      call_syscall_write("[SHELL] Cannot find process '");
      call_syscall_write_hex(pid);
      call_syscall_write("'\n");
    }
  }
}
