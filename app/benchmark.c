#include "../user/user.h"
#include "../common/user_syscalls.h"

// Declare the function signature to make it known to main()
void print_int(unsigned long num);
void main() {
    unsigned long start_time = call_syscall_get_time();
    
    // We use "int" (32-bit) instead of "long" (64-bit) for the calculation.
    // 32-bit mathematical operations take up very little space in the binary file!
    int calcolo = 0;
    for(int i = 0; i < 50000000; i++) {
        calcolo += i;
        calcolo ^= (i % 3); 
    }
    
    unsigned long end_time = call_syscall_get_time();
    unsigned long delta_microsecondi = end_time - start_time;
    
    call_syscall_write("Benchmark completed! Time elapsed (microseconds): ");
    
    print_int(delta_microsecondi);
    
    call_syscall_write("\n");
    
    call_syscall_exit();
}

// ==========================================
// HELPER FUNCTION: Lightweight Converter
// ==========================================
void print_int(unsigned long num) {
    if (num == 0) {
        call_syscall_write("0");
        return;
    }

    char buffer[32];
    int i = 0;
    
    // Manual initialization: prevents bugs with the Kernel's .rodata section
    unsigned long divisors[10];
    divisors[0] = 1000000000;
    divisors[1] = 100000000;
    divisors[2] = 10000000;
    divisors[3] = 1000000;
    divisors[4] = 100000;
    divisors[5] = 10000;
    divisors[6] = 1000;
    divisors[7] = 100;
    divisors[8] = 10;
    divisors[9] = 1;

    int started = 0;

    for (int d = 0; d < 10; d++) {
        int count = 0;
        while (num >= divisors[d]) {
            num -= divisors[d];
            count++;
        }
        // Avoid printing leading zeros (e.g., we print "123" and not "000123")
        if (count > 0 || started) {
            buffer[i] = count + '0';
            i++;
            started = 1;
        }
    }
    buffer[i] = '\0';
    call_syscall_write(buffer);
}
