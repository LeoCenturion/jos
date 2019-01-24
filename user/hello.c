// hello, world
#include <inc/lib.h>


void
umain(int argc, char **argv)
{
	cprintf("hello, world\n");
	cprintf("i am environment %08x\n", sys_getenvid());
	uint32_t data[5] = {1,2,3,4,5};
	uint32_t size = 20;
	cprintf("test hello\n");
	sys_packet_try_send((uint8_t *)data,size);
}

