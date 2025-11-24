# demoos
**demoOS** è un sistema operativo **bare‑metal** progettato per il Raspberry Pi 3B, scritto in C e Assembly. Non dipende da alcun sistema operativo o libreria standard: viene eseguito direttamente sull’hardware, gestendo periferiche e risorse in modo autonomo.

## Installazione
Prima di eseguire il sistema operativo, assicurati di avere installato i seguenti pacchetti:
``` bash
sudo apt-get install build-essential

sudo apt-get install gcc-aarch64-linux-gnu
sudo ln -s $(which aarch64-linux-gnu-gcc) /usr/local/bin/aarch64-elf-gcc

sudo apt-get install binutils-aarch64-linux-gnu
sudo ln -s $(which aarch64-linux-gnu-ld) /usr/local/bin/aarch64-elf-ld

sudo apt-get install binutils-aarch64-linux-gnu
sudo ln -s $(which aarch64-linux-gnu-objcopy) /usr/local/bin/aarch64-elf-objcopy

sudo apt-get install binutils-aarch64-linux-gnu
sudo ln -s $(which aarch64-linux-gnu-ld) /usr/local/bin/aarch64-elf-ld

sudo apt-get install qemu-system-arm
```

## Compilazione ed Esecuzione

### Comandi principali
``` bash
make          # compila il tutto e genera il file kernel8.img
make run      # esegue il kernel in QEMU (emulazione Raspberry Pi 3B)
make clean    # rimuove file oggetto e binari generati
```

### Primo avvio
Se è la prima volta che compili ed esegui DemoOS in QEMU:
``` bash
make
make run
```

### Avvio successivo
Se il file `kernel8.img` è già presente e non hai modificato il codice:
``` bash
make run
```

## Struttura

- `boot`: contiene il codice di bootstrap in assembly
- `arch`: contiene le costanti riguardo il sistema
- `script`: script che dice al linker come posizionare le sezioni del programma in memoria, e che definisce l'entry point
- `kernel`: contiene il codice del kernel
- `drivers`: contiene i driver per le periferiche
  - `uart`: driver periferiche UART
  - `timer`: driver per timer
  - `mbox` driver per comunicare con framebuffer
  - `irq`: vettori di eccezioni e interrupt, con un controller scritto in c
- `libs`: contiene le librerie di utilità

## Flusso di avvio

### Boot
`start.s` è il codice assembly di bootstrap
  - `_start` è il punto di ingresso; verifica il livello di eccezione, e se questo non è EL1 lo porta a EL1
  - Inizializza il core primario: stack pointer, prepara e azzera la bss
  - Mette in standby i core secondari, in attesa che vengano risvegliati dal core primario
  - Lancia il kernel (prima lanciando il file kernel.c, e poi rimanendo in attesa in eventi andando in loop)
    N.B. Nel nostro caso il kernel va esso stesso in loop; è possibile rimuovere il loop del kernel in quanto il bootstrap va già in loop

### Kernel
Il kernel svolge i seguenti passi:
- `uart_init`: inizializza i registri memory mapped della UART
- `uart_putc`: aspetta che la UART sia pronta, e poi scrive il carattere nel registro memory mapped
- `irq_vector_init`: inizializza il vettore delle eccezioni
  - Per farlo, chiama una procedura assembly che carica in un registro la posizione in memoria del vettore
- `timer_init`: scrive nel registri memory mapped del timer il valore del clock
- `enable_interrupt_controller`: abilita i registri memory mapped per attivare la ricezione degli interrupt di timer e UART
- `enable_irq`: procedura assembly che abilita la ricezione degli interrupt

Dopo questo, il kernel va in loop; la CPU è stata istruita dal kernel su come gestire ogni interrupt.

### Interrupt

Cosa succede quando viene generato un interrupt? Supponiamo di aver inviato un carattere sulla UART
- La UART genera un interrupt
- La CPU controlla l'indirizzo in cui si trova la tabella degli interrupt (è stato scritto durante l'inizializzazione)
- La CPU, in base al livello di eccezione, salta a una procedura assembly; nel nostro caso salta a `irq_el1`
- Questa procedura passa in modalità kernel e invoca `handle_irq`
- La funzione `handle_irq` è scritta in c e controlla quale dispositivo ha generato l'interrupt
- Chiama una funzione diversa per gestire ogni dispositivo
- Nel nostro esempio, chiama la funzione `handle_uart_irq` che stampa a schermo il carattere scritto
