# PARTE 1
## static_assert
    ¿cómo y por qué funciona la macro static_assert que define JOS?
    La macro definida funciona porque todos los casos a evaluarse dentro del switch deben ser distintos. Como el primer caso es 0 resulta que si ( MPENTRY_PADDR % PGSIZE == 0) es falso se levanta un error en tiempo de compilación

## env_return
    * Al terminar un proceso su función umain() ¿dónde retoma la ejecución el kernel?
 Describir la secuencia de llamadas desde que termina umain() hasta que el kernel
 dispone del proceso.
    La ejecución del kernel retorna cuando haya una interrupción, por lo tanto retornará al trapframe ...
    * ¿en qué cambia la función env_destroy() en este TP, respecto al TP anterior?
      A diferencia del tp anterior ahora hay más de un entorno, entonces primero se cambia el estado del entorno a destruir a ENV_DYING y, si se trata del entorno actual, se libera el procesador
      
## Sys_yield

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




# PARTE 2
## Tarea: envid2env
1) Responder qué ocurre:
en JOS, si un proceso llama a sys_env_destroy(0)
en Linux, si un proceso llama a kill(0, 9)
E ídem para:
JOS: sys_env_destroy(-1)
Linux: kill(-1, 9)

JOS:
sys_env_destroy(0) → mata al enviroment actual
sys_env_destroy(-1) → mata a todos los enviroments

Linux:
La declaración de la instrucción kill es la siguiente: int     kill(pid_t pid, int sig)

“sig” es la señal que se envía, dentro de las señales se encuentra: SIGKILL 9 /* hard kill */.
“pid” 
Si “pid” es cero: 
La señal se envía a todos los procesos cuyo grupo ID es igual al del proceso que envía la señal, y para el cual el proceso tiene permiso.

Si “pid” es -1: 
Si el usuario tiene privilegios de superusuario, la señal se envía a todos los procesos, excluyendo los procesos del sistema, el proceso con ID 1 (generalmente init (8)) y el proceso que envía la señal. 
Si el usuario no es el superusuario, la señal se envía a todos los procesos con el mismo UID que el usuario, excluyendo el proceso que envía la señal. No se devuelve ningún error si se puede señalar algún proceso.

## Tarea: dumbfork
1) Si, antes de llamar a dumbfork(), el proceso se reserva a sí mismo una página con sys_page_alloc() ¿se propagará una copia al proceso hijo? ¿Por qué?
dumbfork copia los register state y el stack del proceso padre, entonces si se reserva una página, tendrá la referencia a la misma por el stack pero no podrá acceder a ella porque no tiene permiso. Es decir, el hijo no tendrá esa página porque dumbfork no copia el espacio de memoria.

2) ¿Se preserva el estado de solo-lectura en las páginas copiadas? Mostrar, con código en espacio de usuario, cómo saber si una dirección de memoria es modificable por el proceso, o no. (Ayuda: usar las variables globales uvpd y/o uvpt.)
No, no se preserva el estado de solo-lectura en las páginas copiadas, también se les pone permiso de escritura.
Cómo saber si una dirección de memoria es modificable por el proceso, o no:
if((uvpd[PDX(addr)]&PTE_P ) && (uvpt[PGNUM(addr)])&PTE_P)
    la dirección addr es modificable por el proceso
    
3) Describir el funcionamiento de la función duppage().
duppage() aloca una página en el espacio de direcciones del proceso hijo con dirección virtual “addr”. Luego hace que la dirección addr del hijo apunte a UTEMP del proceso actual (proceso padre), donde se copiará la página que se quiere compartir. Finalmente hace que la dirección virtual del proceso padre que apuntaba a UTEMP, ya no apunte allí.


4) Supongamos que se añade a duppage() un argumento booleano que indica si la página debe quedar como solo-lectura en el proceso hijo:
indicar qué llamada adicional se debería hacer si el booleano es true
describir un algoritmo alternativo que no aumente el número de llamadas al sistema, que debe quedar en 3 (1 × alloc, 1 × map, 1 × unmap).

si fuese solo de lectura, sólo se debería llamar a la función sys_page_map(). haciendo que en enviroment del nuevo proceso tenga la dirección de la página de lectura.


5) ¿Por qué se usa ROUNDDOWN(&addr) para copiar el stack? ¿Qué es addr y por qué, si el stack crece hacia abajo, se usa ROUNDDOWN y no ROUNDUP?
addr es una variable local que está en el stack. Se usa ROUNDDOWN porque se quiere el principio de la página.

# Parte 3
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

4 ¿Qué valor tiene el registro %eip cuando se ejecuta esa línea? Responder con redondeo a 12 bits, justificando desde qué región de memoria se está ejecutando este código.
    Con un redondeo a 12 bits el registro %eip tiene el valor 0x7000 ya que está ejecutando instrucciones en la región de memoria indicada por  MPENTRY_PADDR
    ¿Se detiene en algún momento la ejecución si se pone un breakpoint en mpentry_start? ¿Por qué? 
	    No se detiene núnca la ejecución 


# PARTE 4
## ipc_recv
    Un proceso podría intentar enviar el valor númerico -E_INVAL vía ipc_send(). ¿Cómo es posible distinguir si es un error, o no? En estos casos:
    * Caso A: En este caso habría que hacer el chequeo (src != -1). Si  la expresión anterior es verdadera significa que, en efecto el valor retornado por ipc_send() es un mensaje enviado por el proceso src, de lo contrario es un error.
    * Caso B: En este caso no hay forma de saber si se trata de un error o un mensaje.

## sys_ipc_try_send     
    * ¿Cómo se podría hacer bloqueante esta llamada? Esto es: qué estrategia de implementación se podría usar para que, si un proceso A intenta a enviar a B, pero B no está esperando un mensaje, el proceso A sea puesto en estado ENV_NOT_RUNNABLE, y sea despertado una vez B llame a ipc_recv().
      Se pordría implementar una estragetia similar a la implementada en ipc_recv donde el proceso se pone en estado ENV_NOT_RUNNABLE y se hace yield del procesador si el otro proceso no esperando mensaje. En éste caso la llamada a sistema ipc_recv despierta al proceso remitente y, lógicamente, no pone a dormir al proceso destinatario.
    
    * Con esta nueva estrategia de implementación mejorada ¿podría ocurrir un deadlock? Poner un ejemplo de código de usuario que entre en deadlock.
      Además de la situación trivial donde hay un send/receive en un proceso pero no hay un receive/send en otro, no hay una situación de deadlock causada por esta nueva
      implementación
    * ¿Podría el kernel detectar el deadlock, e impedirlo devolviendo un nuevo error, E_DEADLOCK? ¿Qué función o funciones tendrían que modificarse para ello?
      
# PARTE 6
## fork 
¿Puede hacerse con la función set_pgfault_handler()? De no poderse, ¿cómo llega al hijo el valor correcto de la variable global _pgfault_handler?
Si puede hacerse con la función set_pgfault_handler(). Si no se pudiese 
