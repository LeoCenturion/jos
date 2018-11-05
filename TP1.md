TP1: Memoria virtual en JOS
===========================

page2pa
-------

Función que dada un puntero a una página (a un struct PageInfo), devuelve la dirección física en la que se encuentra la misma. Para realizar esto, la función calcula la diferencia entre la dirección "page" (que es donde se comienzan a guardar las estructuras PageInfo) y la dirección de la PageInfo pasada por parámetro. Esta diferencia nos dará el índice de la página, entonces queda multiplicarlo por el tamaño de una página para obtener la dirección de memoria física donde se encuentra la misma.



boot_alloc_pos 

1 Un cálculo manual de la primera dirección de memoria que devolverá boot_alloc() tras el arranque. Se puede
 calcular a partir del binario compilado (obj/kern/kernel), usando los comandos readelf y/o nm y operaciones
 matemáticas.
Cálculo:

.bss:		0xf0118300
	       +
size(bss):	0x00000650
		----------
		0xf0118950
		+
PGSIZE		0x00001000
		----------
		0xf0119950 --> ROUNDUP: 0xf011a000

La primera dirección de memoria que devolverá es 0xf011a000


2 Una sesión de GDB en la que, poniendo un breakpoint en la función boot_alloc(), se muestre el valor de end y
 nextfree al comienzo y fin de esa primera llamada a boot_alloc().

(gdb) p/x (char *)end
$1 = 0xf0102f15
(gdb) p/x nextfree
$2 = 0x0
(gdb) n
=> 0xf0100a25 <boot_alloc+11>:	test   %edx,%edx
104		if(n==0){
(gdb) 
=> 0xf0100a29 <boot_alloc+15>:	mov    0xf0118538,%eax
112		if( nextfree + n > (char *)(npages*pgsize + KERNBASE) ){
(gdb) 
=> 0xf0100a43 <boot_alloc+41>:	add    $0xfff,%edx
118		nextfree = ROUNDUP(nextfree, PGSIZE);
(gdb) 
=> 0xf0100a55 <boot_alloc+59>:	ret    
123	}
(gdb) p/x (char *)end
$3 = 0xf0102f15
(gdb) p/x nextfree
$4 = 0xf011a000
(gdb) 
--------------

...


page_alloc
----------
page2pa devuelve la dirección física donde se almacena una página, en cambio, page2kva devuelve la dirección virtual del kernel que corresponde a esta dirección física.
...


