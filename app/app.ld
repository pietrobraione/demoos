SECTIONS
{
    . = 0x0000000000000000;      /* Indirizzo di partenza User Space */
    .text : { *(.text) }         /* Codice eseguibile */
    . = ALIGN(8);
    .rodata : { *(.rodata) }     /* Dati in sola lettura */
    . = ALIGN(8);
    .data : { *(.data) }         /* Dati inizializzati */
    . = ALIGN(8);
    .bss : { *(.bss) }           /* Dati non inizializzati */
}