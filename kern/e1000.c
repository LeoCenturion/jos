#include <kern/e1000.h>
#include <inc/stdio.h>
#include <kern/pmap.h>
// LAB 6: Your driver code here
volatile uint32_t *bar0_addr;
struct pci_func *e100_pci_func;
volatile struct tx_desc trans_descr_list[E1000_TDL_SIZE] __attribute__ ((aligned (16)));
int attach_e1000(struct pci_func *pcif){
	e100_pci_func = pcif;
	pci_func_enable(pcif);
	bar0_addr = mmio_map_region(pcif->reg_base[0],pcif->reg_size[0]);
	 cprintf("reg_base = %x \n reg_size = %d \n map = %x \n", 
	 	pcif->reg_base[0], pcif->reg_size[0], bar0_addr); 
	 uint32_t *status_reg = (void *)((uint32_t )bar0_addr + (uint32_t)0x8 ); 
	 cprintf("status_reg = %x \n", *status_reg); 
 	*(uint32_t*)(bar0_addr + E1000_TDBAL) = (uint32_t)&trans_descr_list[0]; 
	cprintf("tdbal = %p \n", *(uint32_t*)(bar0_addr + E1000_TDBAL)); 

 	*(uint32_t*)(bar0_addr + E1000_TDLEN) = (uint32_t)E1000_TDL_SIZE*sizeof(struct tx_desc); 
 	if( *(uint32_t*)(bar0_addr + E1000_TDLEN) & 0xFF)  
 		panic("TDLEN unaligned\n"); 

 	if( *(uint32_t*)(bar0_addr + E1000_TDBAL) & 0xF)  
 		panic("TDBAL unaligned\n"); 

	 *(bar0_addr + E1000_TDH) = 0; 
	 *(bar0_addr + E1000_TDT) = 0;
	 *(bar0_addr + E1000_TCTL) = E1000_TCTL_EN | E1000_TCTL_PSP | E1000_TCTL_COLD_FULL_DUPLEX;
	 *(bar0_addr + E1000_TIPG) = E1000_TIPG_IEEE;
	return 0;
}
