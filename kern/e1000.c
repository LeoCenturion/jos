#include <kern/e1000.h>
#include <inc/stdio.h>
#include <kern/pmap.h>
// LAB 6: Your driver code here
static volatile uint32_t *bar0_addr;
struct pci_func *e100_pci_func;
volatile struct tx_desc trans_descr_list[E1000_TDL_SIZE] __attribute__ ((aligned (16)));

void transmit_initialization( volatile uint32_t *addr){
	uint32_t *tdbal = (uint32_t*)(bar0_addr + E1000_TDBAL);
	*tdbal = (uint32_t)&trans_descr_list[0];
	
	uint32_t *tdbah = (uint32_t*)(bar0_addr + E1000_TDBAH);
	*tdbah = 0;
	
	uint32_t *tdlen = (uint32_t*)(bar0_addr + E1000_TDLEN);
	*tdlen= (uint32_t)E1000_TDL_SIZE*sizeof(struct tx_desc);

 	if( *tdlen & 0xFF)  
 		panic("TDLEN unaligned\n"); 

 	if( *tdbal & 0xF)  
 		panic("TDBAL unaligned\n"); 

	volatile uint32_t *tdh = (uint32_t*)(bar0_addr + E1000_TDH);
	*tdh = 0; 
	volatile uint32_t *tdt = (uint32_t*)(bar0_addr + E1000_TDT);
	*tdt = 0;
	
	volatile uint32_t *tctl = (uint32_t*)(bar0_addr + E1000_TCTL);
	*tctl = E1000_TCTL_EN | E1000_TCTL_PSP | E1000_TCTL_COLD_FULL_DUPLEX;
	volatile uint32_t *tipg = (uint32_t*)(bar0_addr + E1000_TIPG);
	*tipg= 50; //E1000_TIPG_IEEE;
	cprintf("TDBAL = %x \nTDBAH = %x  \nTDLEN = %d \nTDT = %x \nTDH = %x \nTCTL = %x \nTIPG = %x\n",*tdbal,*tdbah,*tdlen,*tdt,*tdh,*tctl,*tipg);
}


int attach_e1000(struct pci_func *pcif){
	e100_pci_func = pcif;
	pci_func_enable(pcif);
	bar0_addr = mmio_map_region(pcif->reg_base[0],pcif->reg_size[0]);
	cprintf("reg_base = %x \n reg_size = %d \n map = %x \n", 
	 	pcif->reg_base[0], pcif->reg_size[0], bar0_addr); 
	uint32_t *status_reg = (void *)((uint32_t )bar0_addr + (uint32_t)0x8 );
	cprintf("status_reg = %x \n", *status_reg); 

	transmit_initialization(bar0_addr);
	return 0;
}

