TP1: Memoria virtual en JOS
===========================

page2pa
-------

...



boot_alloc_pos 

1 Un cálculo manual de la primera dirección de memoria que devolverá boot_alloc() tras el arranque. Se puede
 calcular a partir del binario compilado (obj/kern/kernel), usando los comandos readelf y/o nm y operaciones
 matemáticas.

2 Una sesión de GDB en la que, poniendo un breakpoint en la función boot_alloc(), se muestre el valor de end y
 nextfree al comienzo y fin de esa primera llamada a boot_alloc().


Breakpoint 1, boot_alloc (n=4096) at kern/pmap.c:89
89	{
(gdb) display nextfree
1: nextfree = 0x0
(gdb) s
=> 0xf0100a3e <boot_alloc+7>:	cmpl   $0x0,0xf0113538
98		if (!nextfree) {
1: nextfree = 0x0
(gdb) 
=> 0xf0100a7c <boot_alloc+69>:	mov    $0xf011494f,%edx
100			nextfree = ROUNDUP((char *) end, PGSIZE);
1: nextfree = 0x0
(gdb) 
=> 0xf0100a47 <boot_alloc+16>:	mov    %eax,%edx
108	       int num_of_pages =n/PGSIZE + 1;// ROUNDUP(n,PGSIZE);
1: nextfree = 0xf0114000 ""
(gdb) 
=> 0xf0100a4f <boot_alloc+24>:	mov    0xf0113538,%ebx
110		if(nextfree+n>(char *)KERNBASE+npages*PGSIZE){
1: nextfree = 0xf0114000 ""
(gdb) 
=> 0xf0100a6a <boot_alloc+51>:	shl    $0xc,%edx
115			nextfree += num_of_pages*PGSIZE;
1: nextfree = 0xf0114000 ""

--------------

...


page_alloc
----------

...


