SECTIONS
{
    . = 0x0000000000000000;      /* Indirizzo di partenza User Space */
    .text : { *(.text) }         /* Codice eseguibile */
    .rodata : { *(.rodata) }     /* Dati in sola lettura */
    .data : { *(.data) }         /* Dati inizializzati */
    .bss : { *(.bss) }           /* Dati non inizializzati */
}