* static_assert

*env_return
    * Al terminar un proceso su función umain() ¿dónde retoma la ejecución el kernel?
 Describir la secuencia de llamadas desde que termina umain() hasta que el kernel
 dispone del proceso.
    La ejecución del kernel retorna cuando haya una interrupción, por lo tanto retornará al trapframe ...
    * ¿en qué cambia la función env_destroy() en este TP, respecto al TP anterior?
    
*sys_yield

qemu-system-i386 -nographic -drive file=obj/kern/kernel.img,index=0,media=disk,format=raw -serial mon:stdio -gdb tcp:127.0.0.1:26000 -D qemu.log -smp 1  -d guest_errors
6828 decimal is 15254 octal!
Physical memory: 131072K available, base = 640K, extended = 130432K
check_page_free_list() succeeded!
test
check_page_alloc() succeeded!
check_page() succeeded!
check_kern_pgdir() succeeded!
check_page_free_list() succeeded!
check_page_installed_pgdir() succeeded!
SMP: CPU 0 found 1 CPU(s)
enabled interrupts: 1 2
[00000000] new env 00001000
[00000000] new env 00001001
[00000000] new env 00001002
Hello, I am environment 00001000.
curenv f02b8000 
Hello, I am environment 00001001.
curenv f02b807c 
Hello, I am environment 00001002.
curenv f02b80f8 
Back in environment 00001000, iteration 0.
curenv f02b8000 
Back in environment 00001001, iteration 0.
curenv f02b807c 
Back in environment 00001002, iteration 0.
curenv f02b80f8 
Back in environment 00001000, iteration 1.
curenv f02b8000 
Back in environment 00001001, iteration 1.
curenv f02b807c 
Back in environment 00001002, iteration 1.
curenv f02b80f8 
Back in environment 00001000, iteration 2.
curenv f02b8000 
Back in environment 00001001, iteration 2.
curenv f02b807c 
Back in environment 00001002, iteration 2.
curenv f02b80f8 
Back in environment 00001000, iteration 3.
curenv f02b8000 
Back in environment 00001001, iteration 3.
curenv f02b807c 
Back in environment 00001002, iteration 3.
curenv f02b80f8 
Back in environment 00001000, iteration 4.
All done in environment 00001000.
[00001000] exiting gracefully
[00001000] free env 00001000
MBack in environment 00001001, iteration 4.
All done in environment 00001001.
[00001001] exiting gracefully
[00001001] free env 00001001
Back in environment 00001002, iteration 4.
All done in environment 00001002.
[00001002] exiting gracefully
[00001002] free env 00001002
No runnable environments in the system!

En la salida se ve que, cómo es de esperarse, después de cada iteración una instancia del  programa 'yield.c' cede el uso del procesador el cual es retomado por otra instancia del mísmo programa entonces se puede ver que primero se realizan todas las primeras iteraciones de las instancias, luego todas las segundas y así hasta terminar.

* multicore init
Preguntas:

1 ¿Qué código copia, y a dónde, la siguiente línea de la función boot_aps()?
    La función nombrada copia el código de mpentry.S a una posición de memoria tal que pueda ser referenciada en modo real que es el modo en el cual se inician los núcleos. Ésta posición de memoria se establece en MPENTRY_PADDR
    
memmove(code, mpentry_start, mpentry_end - mpentry_start);
2 ¿Para qué se usa la variable global mpentry_kstack? ¿Qué ocurriría si el espacio para este stack se reservara en el archivo
 kern/mpentry.S, de manera similar a bootstack en el archivo kern/entry.S?
    La variable mpentry_kstack indica el inicio del stack del núcleo en el que se está.

3 Cuando QEMU corre con múltiples CPUs, éstas se muestran en GDB como hilos de ejecución separados. Mostrar una sesión de GDB en la
 que se muestre cómo va cambiando el valor de la variable global mpentry_kstack:
 (gdb) watch mpentry_kstack
Hardware watchpoint 1: mpentry_kstack
(gdb) c
Continuing.
The target architecture is assumed to be i386
=> 0xf010018b <boot_aps+127>:	mov    %esi,%ecx

Thread 1 hit Hardware watchpoint 1: mpentry_kstack

Old value = (void *) 0x0
New value = (void *) 0xf0246000 <percpu_kstacks+65536>
boot_aps () at kern/init.c:116
116			lapic_startap(c->cpu_id, PADDR(code));
(gdb) bt
#0  boot_aps () at kern/init.c:116
#1  0xf0100214 in i386_init () at kern/init.c:55
#2  0xf010004c in relocated () at kern/entry.S:88
(gdb) info threads
  Id   Target Id         Frame 
* 1    Thread 1 (CPU#0 [running]) boot_aps () at kern/init.c:116
  2    Thread 2 (CPU#1 [halted ]) 0x000fd412 in ?? ()
  3    Thread 3 (CPU#2 [halted ]) 0x000fd412 in ?? ()
  4    Thread 4 (CPU#3 [halted ]) 0x000fd412 in ?? ()
(gdb) c
Continuing.
=> 0xf010018b <boot_aps+127>:	mov    %esi,%ecx

Thread 1 hit Hardware watchpoint 1: mpentry_kstack

Old value = (void *) 0xf0246000 <percpu_kstacks+65536>
New value = (void *) 0xf024e000 <percpu_kstacks+98304>
boot_aps () at kern/init.c:116
116			lapic_startap(c->cpu_id, PADDR(code));
(gdb) info threads
  Id   Target Id         Frame 
* 1    Thread 1 (CPU#0 [running]) boot_aps () at kern/init.c:116
  2    Thread 2 (CPU#1 [running]) 0xf01002a2 in mp_main () at kern/init.c:134
  3    Thread 3 (CPU#2 [halted ]) 0x000fd412 in ?? ()
  4    Thread 4 (CPU#3 [halted ]) 0x000fd412 in ?? ()
(gdb) thread 2
[Switching to thread 2 (Thread 2)]
#0  0xf01002a2 in mp_main () at kern/init.c:134
134		xchg(&thiscpu->cpu_status, CPU_STARTED); // tell boot_aps() we're up
(gdb) bt
#0  0xf01002a2 in mp_main () at kern/init.c:134
#1  0x00007065 in ?? ()
(gdb) bt 
#0  0xf01002a2 in mp_main () at kern/init.c:134
#1  0x00007065 in ?? ()
(gdb) p cpunum()
$1 = 1
(gdb) thread 1
[Switching to thread 1 (Thread 1)]
#0  boot_aps () at kern/init.c:118
118			while(c->cpu_status != CPU_STARTED)
(gdb) p cpunum()
$2 = 0
(gdb) c
Continuing.
=> 0xf010018b <boot_aps+127>:	mov    %esi,%ecx

Thread 1 hit Hardware watchpoint 1: mpentry_kstack

Old value = (void *) 0xf024e000 <percpu_kstacks+98304>
New value = (void *) 0xf0256000 <percpu_kstacks+131072>
boot_aps () at kern/init.c:116
116			lapic_startap(c->cpu_id, PADDR(code));

*4
    * ¿Qué valor tiene el registro %eip cuando se ejecuta esa línea?
    Responder con redondeo a 12 bits, justificando desde qué región de memoria se está ejecutando este código.
	    Con un redondeo a 12 bits el registro %eip tiene el valor 0x7000 ya que está ejecutando instrucciones en la región de memoria indicada por  MPENTRY_PADDR

   * ¿Se detiene en algún momento la ejecución si se pone un breakpoint en mpentry_start? ¿Por qué? 
    No se detiene núnca la ejecución 
