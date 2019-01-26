#include "ns.h"
#include <inc/malloc.h>
#include <inc/lib.h>
extern union Nsipc nsipcbuf;

void
output(envid_t ns_envid)
{
	binaryname = "ns_output";
	
	// LAB 6: Your code here:
	// 	- read a packet from the network server
	//	- send the packet to the device driver
	void *rcv_pg = malloc(PGSIZE);
	int32_t envid = sys_getenvid();

	int32_t value = 0;
	int perm_store =  PTE_U | PTE_P | PTE_W;
	while(1){
		value = ipc_recv(&ns_envid, rcv_pg, &perm_store);
		if (value == NSREQ_OUTPUT){
			struct jif_pkt *jif = (struct jif_pkt *)rcv_pg;
			cprintf("len = %d\n",jif->jp_len);
			sys_packet_try_send((uint8_t *)jif->jp_data,jif->jp_len);
		}
	}
}
