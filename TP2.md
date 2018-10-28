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

...
