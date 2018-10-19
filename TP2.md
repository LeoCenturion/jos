TP2: Procesos de usuario
========================

env_alloc
---------

1 ¿Qué identificadores se asignan a los primeros 5 procesos creados? (Usar base hexadecimal.)
generation = (0x0 + 0x1000 & ~(0x0400 - 0x1)
	     (0x1000 & ~(0x03FF))
	     (0x1000 & 0xFC00)
	     0x1000
	     
(e - envs) = 0,1,2,3,4 respectivamente.

Entonces:
    e->env_id = 0x1000, 0x1001, 0x1002, 0x1003, 0x1004 respectivamente.
    
Supongamos que al arrancar el kernel se lanzan NENV proceso a ejecución. A continuación se destruye el proceso a envs[630] y se lanza un proceso que cada segundo muere y se vuelve a lanzar. ¿Qué identificadores este proceso en sus sus primeras cinco ejecuciones?

generation = (0x0 + 0x1000 & ~(0x0400 - 0x1)
	     (0x1000 & ~(0x03FF))
	     (0x1000 & 0xFC00)
	     0x1000
(e - envs) = 630 siempre.
...


env_init_percpu
---------------

...


env_pop_tf
----------

...


gdb_hello
---------

...
