TP2: Procesos de usuario
========================

env_alloc
---------

1 ¿Qué identificadores se asignan a los primeros 5 procesos creados? (Usar base hexadecimal.)
generation = (0x0 + 0x1000) & ~(0x0400 - 0x1)
	     (0x1000 & ~(0x03FF))
	     (0x1000 & 0xFC00)
	     0x1000

(e - envs) = 0,1,2,3,4 respectivamente.

Entonces:
    e->env_id = 0x1000, 0x1001, 0x1002, 0x1003, 0x1004 respectivamente.

Supongamos que al arrancar el kernel se lanzan NENV proceso a ejecución. A continuación se destruye el proceso a envs[630] y se lanza un proceso que cada segundo muere y se vuelve a lanzar. ¿Qué identificadores este proceso en sus sus primeras cinco ejecuciones?
id_old = 0x1276 --> id de envs[630] que fue destruido
generation = (0x1276 + 0x1000) & ~(0x0400 - 0x1)
	     (0x1276 & ~(0x03FF))
	     (0x1276 & 0xFC00)
	     0x2000
Entonces id_nueva = 0x2276

id_old = 0x2276 --> id de envs[630] que fue destruido recien
generation = (0x2276 + 0x1000) & ~(0x0400 - 0x1)
	     (0x2276 & ~(0x03FF))
	     (0x2276 & 0xFC00)
	     0x3000
Entonces id_nueva = 0x3276

Se nota que se cumple un patron, por lo que podemos decir que las siguientes id's seran:
0x4276, 0x5276, 0x6276 para los siguientes procesos creados.


env_init_percpu
---------------
¿Cuantos bytes escribe la función lgdt, y dónde?
La función lgdt escribe 6 bytes en la dirección &gdt_pd .

¿Qué representan esos bytes?
Los 4 bytes menos significativos representan la dirección base donde empieza el global descriptor table y los 2 bytes más significativos representan el límite del gdt.

Dos hilos distintos ejecutandose en paralelo ¿podrían usar distintas GDT?
Si. La GDT define características de varias áreas de memoria compartidas globalmente por los procesos (a diferencia de la LDT que define áreas privadas de un proceso particular). Entonces si se tiene que los hilos pertenecen a procesos distintos se esperaría
.


env_pop_tf
----------
¿Qué hay en (%esp) tras el primer movl de la función?
En el stack pointer se encuentra la dirección del struct trap frame pasado por parámetro

¿Qué hay en (%esp) justo antes de la instrucción iret? ¿Y en 8(%esp)?
En el stack pointer se encuentra el instruction pointer correspondiente al código del proceso a ejecutarse.
En 8(esp) se encuentra los flags de ejecución del programa a ejecutarse

En la documentación de iret en [IA32-2A] se dice:
¿Cómo puede determinar la CPU si hay un cambio de ring (nivel de privilegio)?
La cpu mantiene un campo de dos bit en el segment registre que indica la prioridad con la que se está ejecutando.
El current privilege level sólo puede ser modificado mediante instrucciones del procesador.

gdb_hello
---------
1. Poner un breakpoint en env_pop_tf() y continuar la ejecución hasta allí.
(gdb) b env_pop_tf
Punto de interrupción 1 at 0xf0103370: file kern/env.c, line 482.
(gdb) c
Continuando.
Se asume que la arquitectura objetivo es i386
=> 0xf0103370 <env_pop_tf>:    push   %ebp

Breakpoint 1, env_pop_tf (tf=0xf01c0000) at kern/env.c:482
482    {


2. En QEMU, entrar en modo monitor (Ctrl-a c), y mostrar las cinco primeras líneas del comando info registers.
(qemu) info registers
EAX=f01c0000 EBX=00010094 ECX=f03bc000 EDX=00000213
ESI=00010094 EDI=00000000 EBP=f0118fd8 ESP=f0118fbc
EIP=f0103370 EFL=00000012 [----A--] CPL=0 II=0 A20=1 SMM=0 HLT=0
ES =0010 00000000 ffffffff 00cf9300 DPL=0 DS   [-WA]
CS =0008 00000000 ffffffff 00cf9a00 DPL=0 CS32 [-R-]


3. De vuelta a GDB, imprimir el valor del argumento tf:
(gdb) p tf
$1 = (struct Trapframe *) 0xf01c0000


4. Imprimir, con x/Nx tf tantos enteros como haya en el struct Trapframe donde N = sizeof(Trapframe) / sizeof(int).
(gdb) print sizeof(struct Trapframe) / sizeof(int)
$2 = 17
(gdb) x/17x tf
0xf01c0000:    0x00000000    0x00000000    0x00000000    0x00000000
0xf01c0010:    0x00000000    0x00000000    0x00000000    0x00000000
0xf01c0020:    0x00000023    0x00000023    0x00000000    0x00000000
0xf01c0030:    0x00800020    0x0000001b    0x00000000    0xeebfe000
0xf01c0040:    0x00000023


5. Avanzar hasta justo después del movl ...,%esp, usando si M para ejecutar tantas instrucciones como sea necesario en un solo paso:
(gdb) disas
Dump of assembler code for function env_pop_tf:
=> 0xf0103370 <+0>:    push   %ebp
   0xf0103371 <+1>:    mov    %esp,%ebp
   0xf0103373 <+3>:    sub    $0x18,%esp
   0xf0103376 <+6>:    mov    0x8(%ebp),%esp
   0xf0103379 <+9>:    popa   
   0xf010337a <+10>:    pop    %es
   0xf010337b <+11>:    pop    %ds
   0xf010337c <+12>:    add    $0x8,%esp
   0xf010337f <+15>:    iret   
   0xf0103380 <+16>:    movl   $0xf0105953,0x8(%esp)
   0xf0103388 <+24>:    movl   $0x1ee,0x4(%esp)
   0xf0103390 <+32>:    movl   $0xf01058ea,(%esp)
   0xf0103397 <+39>:    call   0xf01000c6 <_panic>
End of assembler dump.

(gdb) si 4
=> 0xf0103379 <env_pop_tf+9>:    popa   
0xf0103379    485        asm volatile("\tmovl %0,%%esp\n"


6. Comprobar, con x/Nx $sp que los contenidos son los mismos que tf (donde N es el tamaño de tf).
(gdb) x/17x $sp
0xf01c0000:    0x00000000    0x00000000    0x00000000    0x00000000
0xf01c0010:    0x00000000    0x00000000    0x00000000    0x00000000
0xf01c0020:    0x00000023    0x00000023    0x00000000    0x00000000
0xf01c0030:    0x00800020    0x0000001b    0x00000000    0xeebfe000
0xf01c0040:    0x00000023


7. Explicar con el mayor detalle posible cada uno de los valores. Para los valores no nulos, se debe indicar dónde se configuró inicialmente el valor, y qué representa.
Los primeros ocho valores, que tienen como valor 0x00000000, corresponden a los registros de propósito general que se encuentran en el struct PushRegs tf_regs. El orden de estos es:  %edi, %esi, %ebp, %oesp, %ebx, %edx, %ecx y %eax.

0x00000023 corresponde al selector %es: en binario, 0..00100011, se puede ver que los últimos dos bits (en decimal: 3) corresponden al RPL (Requested Privilege Level), el bit siguiente (0) indica la tabla GDT,los siguientes bits (en decimal: 4) indican el índice. 
Inicialmente se configuró en env_alloc: e->env_tf.tf_es = GD_UD | 3.

0x00000023 corresponde al selector %ds: Ídem %es.

0x00000000 corresponde a tf_trapno.

0x00000000 corresponde a tf_err.

0x00800020 corresponde a %eip: representa la siguiente instrucción a ejecutarse del proceso. Su valor se asignó en load_icode: e->env_tf.tf_eip = bin->e_entry.

0x0000001b corresponde al selector %cs: en binario, 0011011, se puede ver que los últimos dos bits (en decimal: 3) corresponden al RPL, el bit siguiente (0) indica la tabla GDT,los siguientes bits indican el índice (en decimal: 3).
Inicialmente se configuró en env_alloc: e->env_tf.tf_cs = GD_UT | 3. 

0x00000000 corresponde a %eflags: Es el registro en donde se almacenan en distintos bits los resultados de las operaciones y el estado del procesador.
Inicialmente se configuró en env_alloc: memset(&e->env_tf, 0, sizeof(e->env_tf)).

0xeebfe000 corresponde a %esp: Apunta al inicio del user stack.
Inicialmente se configuró en env_alloc: e->env_tf.tf_esp = USTACKTOP.

0x0000000023 corresponde al selector %ss: Ídem %es.


8. Continuar hasta la instrucción iret, sin llegar a ejecutarla. Mostrar en este punto, de nuevo, las cinco primeras líneas de info registers en el monitor de QEMU. Explicar los cambios producidos.
(gdb) si 4
=> 0xf010337f <env_pop_tf+15>:    iret   
0xf010337f    485        asm volatile("\tmovl %0,%%esp\n"

(qemu) info registers
EAX=00000000 EBX=00000000 ECX=00000000 EDX=00000000
ESI=00000000 EDI=00000000 EBP=00000000 ESP=f01c0030
EIP=f010337f EFL=00000096 [--S-AP-] CPL=0 II=0 A20=1 SMM=0 HLT=0
ES =0023 00000000 ffffffff 00cff300 DPL=3 DS   [-WA]
CS =0008 00000000 ffffffff 00cf9a00 DPL=0 CS32 [-R-]


Los registros de propósito general (desde EAX hasta EBP) se recuperaron, por ello valen  0x00000000.

ESP=f01c0030 : dentro de esta dirección se guarda el %eip a recuperar. En el punto anterior pudimos ver que a lo que se está apuntando es a 0x00800020 (0xf01e6030:    0x00800020 ).

ES=... DPL=3 : se pasa de tener DPL=0 a DPL=3 porque se recuperó %es.
ES=0023 00000000 ffffffff 00cff300 : ES comenzaba con 0010 porque en la función env_init_percpu se cargó el valor GD_KD (kernel data). El valor actual de ES (0023...) se da por GD_UD|3 (3: valor del %es recuperado); donde GD_UD es el global descriptor number para user data y 3 es el user mode.


9. Ejecutar la instrucción iret. En ese momento se ha realizado el cambio de contexto y los símbolos del kernel ya no son válidos.
imprimir el valor del contador de programa con p $pc o p $eip
cargar los símbolos de hello con symbol-file obj/user/hello
volver a imprimir el valor del contador de programa
Mostrar una última vez la salida de info registers en QEMU, y explicar los cambios producidos.

(gdb) si
=> 0x800020:    cmp    $0xeebfe000,%esp
0x00800020 in ?? ()
(gdb) p $pc
$3 = (void (*)()) 0x800020
(gdb) symbol-file obj/user/hello
¿Cargar una tabla de símbolos nueva desde «obj/user/hello»? (y o n) y
Leyendo símbolos desde obj/user/hello...hecho.
(gdb) p $pc
$4 = (void (*)()) 0x800020 <_start>

(qemu) info registers
EAX=00000000 EBX=00000000 ECX=00000000 EDX=00000000
ESI=00000000 EDI=00000000 EBP=00000000 ESP=eebfe000
EIP=00800020 EFL=00000002 [-------] CPL=3 II=0 A20=1 SMM=0 HLT=0
ES =0023 00000000 ffffffff 00cff300 DPL=3 DS   [-WA]
CS =001b 00000000 ffffffff 00cffa00 DPL=3 CS32 [-R-]


EIP=00800020 : Como ya se ejecutó iret se recuperó el instruction pointer, ahora vale lo que era el entry point asignado en load_icode. 
CPL=3 : porque se realizó un context switch.
CS=... DPL=3 :Se recuperó %cs.
ESP=eebfe000 :Se pasó de un nivel de mayor privilegio (0) a uno de menor privilegio (3), iret también hizo pop de %esp y %ss.
CS =001b 00000000 ffffffff 00cffa00 : CS comenzaba con 0008 porque en la función env_init_percpu se cargó el valor GD_KT (kernel text). El valor actual de CS (001b…) es debido al valor GD_UT | 3 (valor del %cs recuperado); donde GD_UT corresponde a user text.


10. Poner un breakpoint temporal (tbreak, se aplica una sola vez) en la función syscall() y explicar qué ocurre justo tras ejecutar la instrucción int $0x30. Usar, de ser necesario, el monitor de QEMU.
(gdb) tbreak syscall
Punto de interrupción temporal 2 at 0x800ab8: file lib/syscall.c, line 23.
(gdb) c
Continuando.
=> 0x800ab8 <syscall+17>:    mov    0x8(%ebp),%ecx

Temporary breakpoint 2, syscall (num=0, check=-289415544, a1=4005551752,
    a2=13, a3=0, a4=0, a5=0) at lib/syscall.c:23
23        asm volatile("int %1\n"
(gdb) si
=> 0x800abb <syscall+20>:    mov    0xc(%ebp),%ebx
0x00800abb    23        asm volatile("int %1\n"
(gdb) si
=> 0x800abe <syscall+23>:    mov    0x10(%ebp),%edi
0x00800abe    23        asm volatile("int %1\n"
(gdb) si 2
=> 0x800ac4 <syscall+29>:    int    $0x30
0x00800ac4    23        asm volatile("int %1\n"
(gdb) si
aviso: A handler for the OS ABI "GNU/Linux" is not built into this configuration
of GDB.  Attempting to continue with the default i8086 settings.

Se asume que la arquitectura objetivo es i8086
[f000:e05b]    0xfe05b:    cmpl   $0x0,%cs:0x6574
0x0000e05b in ?? ()


//FALTA EXPLICACIÓN

kern_idt
---------
1) ¿Cómo decidir si usar TRAPHANDLER o TRAPHANDLER_NOEC? ¿Qué pasaría si se usara solamente la primera?
TRAPHANDLER se usa para las excepciones/interrupciones para las cuales el CPU automáticamente pushea un código de error, de lo contrario, se debe utilizar TRAPHANDLER_NOEC. Esta última opción pushea 0 en lugar del código de error.
Si se usara sólamente la primera, los ‘trap frame’ tendrían distintos formatos. Los que pushean el código de error, tendría una línea más que los que no.

2) ¿Qué cambia, en la invocación de handlers, el segundo parámetro (istrap) de la macro SETGATE? ¿Por qué se elegiría un comportamiento u otro durante un syscall?
El parámetro ‘istrap’ indica si es una excepción (1) o una interrupción (0).
Si se elige que es una interrupción, el IF (interrupt-enable flag) se resetea...
//HAY QUE PONER ESTO:
//"The difference between
    //   an interrupt gate and a trap gate is in the effect on IF (the
    //   interrupt-enable flag). An interrupt that vectors through an
    //   interrupt gate resets IF, thereby preventing other interrupts from
    //   interfering with the current interrupt handler. A subsequent IRET
    //   instruction restores IF to the value in the EFLAGS image on the
    //   stack. An interrupt through a trap gate does not change IF."

3) Leer user/softint.c y ejecutarlo con make run-softint-nox. ¿Qué excepción se genera? Si hay diferencias con la que invoca el programa… ¿por qué mecanismo ocurre eso, y por qué razones?

$ make run-softint-nox
make[1]: se ingresa al directorio «/home/flor/Escritorio/SisOp/TP1/jos»
+ cc kern/init.c
+ ld obj/kern/kernel
+ mk obj/kern/kernel.img
make[1]: se sale del directorio «/home/flor/Escritorio/SisOp/TP1/jos»
qemu-system-i386 -nographic -drive file=obj/kern/kernel.img,index=0,media=disk,format=raw -serial mon:stdio -gdb tcp:127.0.0.1:26000 -D qemu.log  -d guest_errors
6828 decimal is 15254 octal!
Physical memory: 131072K available, base = 640K, extended = 130432K
test
check_page_alloc() succeeded!
check_page() succeeded!
check_kern_pgdir() succeeded!
check_page_installed_pgdir() succeeded!
[00000000] new env 00001000
Incoming TRAP frame at 0xefffffbc
TRAP frame at 0xf01c0000
  edi  0x00000000
  esi  0x00000000
  ebp  0xeebfdfd0
  oesp 0xefffffdc
  ebx  0x00000000
  edx  0x00000000
  ecx  0x00000000
  eax  0x00000000
  es   0x----0023
  ds   0x----0023
  trap 0x0000000d General Protection
  err  0x00000072
  eip  0x00800036
  cs   0x----001b
  flag 0x00000046
  esp  0xeebfdfd0
  ss   0x----0023
[00001000] free env 00001000
Destroyed the only environment - nothing more to do!
Welcome to the JOS kernel monitor!
Type 'help' for a list of commands.
K>

//VER INTERRUPCION 14 QUÉ ES (es interrupción por page fault) Y QUÉ HACE

