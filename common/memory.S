.globl memzero
memzero:
    str xzr, [x0], #8
    subs x1, x1, #8
    b.gt memzero
    ret

.globl memcpy
memcpy:
    ldrb w3, [x1], #1
    strb w3, [x0], #1
    subs w2, w2, #1
    b.gt memcpy
    ret

.globl memcmp
memcmp:
    // If the number of bytes to check is zero, I return true
    cbz x2, .lEqual

.lLoop:
    // I store the given addresses in two registers
    ldrb w3, [x0], #1
    ldrb w4, [x1], #1
    // I perform the subtraction of the two memory addresses
    subs w5, w3, w4

    // If the subtraction is not zero, I exit because the two addresses
    // don't have the same content
    cbnz w5, .lExit

    // I decrease the counter of the bytes to compare
    subs x2, x2, #1

    // Otherwhise I continue checking the next bytes
    cbnz x2, .lLoop

.lEqual:
    mov w5, #0


.globl memset
memset:
    // If the counter is 0 I have to return
    cbz x2, .lExit
    strb w1, [x0]
    add x0, x0, #1
    sub x2, x2, #1

    b memset

.lExit:
    mov w0, w5
    ret
