#include "ns.h"
#include <inc/lib.h>
extern union Nsipc nsipcbuf;

void
input(envid_t ns_envid)
{
	binaryname = "ns_input";

	// LAB 6: Your code here:
	// 	- read a packet from the device driver
	//	- send it to the network server
	// Hint: When you IPC a page to the network server, it will be
	// reading from it for a while, so don't immediately receive
	// another packet in to the same physical page.
	static uint8_t buff[256][4096] __attribute__((aligned(4096)));
	uint8_t idx = 0;
	for(int i = 0; i< 256; i++){
		memset(buff[i], 1,4096);
	}
	while( 1 ){
		struct jif_pkt *pkt = (struct jif_pkt *)buff[idx];
		int size = sys_packet_try_recv(
			(uint8_t*)pkt->jp_data
			);

		if(size < 0){
			sys_yield();
		}
		else{
			pkt->jp_len = size;
			ipc_send(ns_envid, NSREQ_INPUT, pkt, PTE_P|PTE_W|PTE_U);
			idx++;
		}
	}
	
}
