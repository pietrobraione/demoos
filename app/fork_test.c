#include "../common/user_syscalls.h"
#include "../common/memory.h"

void main() {
  int pid = call_syscall_fork();
  if (pid == 0) {
    // This yield forces the father process to start sending messages, so the son buffer will be
    // filled and the user process will need to be blocked and resumed
    call_syscall_yield();
    char buffer[64];
    for (int i = 0; i < 5; i++) {
      memzero((unsigned long)buffer, 64);
      call_syscall_receive_message(buffer);
      call_syscall_write("[FATHER] Message received from FATHER: '");
      call_syscall_write(buffer);
      call_syscall_write("'.\n");
    }
    call_syscall_exit();
  } else {
    // The father will send 5 messages to the son
    char* messages[5];
    memzero((unsigned long)messages, 5);
    messages[0] = "hello";
    messages[1] = "world";
    messages[2] = "demoos";
    messages[3] = "is";
    messages[4] = "great!";

    call_syscall_write("[SON] Sending message to SON\n");
    for (int i = 0; i < 5; i++) {
      int ok = call_syscall_send_message(pid, messages[i]);
      if (ok == -1) {
        call_syscall_write("[SON] Error sending message to SON.\n");
        break;
      }
      call_syscall_write("[SON] Message '");
      call_syscall_write(messages[i]);
      call_syscall_write("' sent to SON.\n");
    }
    call_syscall_write("[SON] I sent all the messages\n");
  }

  call_syscall_exit();
}
