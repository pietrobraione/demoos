#include "../drivers/irq/controller.h"
#include "../drivers/sd/sd.h"
#include "../drivers/sd/sd_filesystem.h"
#include "../drivers/timer/timer.h"
#include "../drivers/uart/uart.h"
#include "../libs/fork.h"
#include "../libs/scheduler.h"
#include "../libs/syscalls.h"
#include "../libs/utils.h"
#include "../user/user.h"
#include <stddef.h>
#include <stdint.h>

void useless_process();
void kernel_process();

void kernel_main() {
  uart_init();

  uart_puts("demoOS v.0.0.1\n");

  irq_vector_init();
  uart_puts("[DONE] irq vector init\n");
  enable_interrupt_controller();
  uart_puts("[DONE] enable interrupt controller\n");
  enable_irq();
  uart_puts("[DONE] enable irq\n");

  int fs_ok = sd_filesystem_init();
  if (fs_ok == SD_FILESYSTEM_INIT_OK) {
    uart_puts("[DONE] SD filesystem init success\n");
  } else {
    uart_puts("[ERROR] SD filesystem init error\n");
  }

  int res = copy_process(PF_KTHREAD, (unsigned long)&kernel_process, 0);
  if (res < 0) {
      uart_puts("[ERROR] Cannot create kernel process.\n");
  }

  timer_init();
  uart_puts("[DONE] timer init\n");

  while (1) {}
}

void useless_process() {
  while (1) {
    uart_puts("[KERNEL] I am a useless process.\n");
  }
}

void kernel_process() {
    uart_puts("[DEBUG] Kernel process started.\n");

    unsigned long process = (unsigned long)&shell;
    unsigned long size = ((unsigned long)&user_end - (unsigned long)&user_begin);
    unsigned long pc = (process - (unsigned long)&user_begin);

    int error = move_to_user_mode((unsigned long)&user_begin, size, pc);
    if (error < 0) {
        uart_puts("[ERROR] Cannot move process from kernel mode to user mode\n");
    }
}
