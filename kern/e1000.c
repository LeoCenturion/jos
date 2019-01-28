#include <kern/e1000.h>
#include <kern/syscall.h>
#include <inc/string.h>
#include <kern/pmap.h>
#include <kern/env.h>
#include <inc/error.h>
// LAB 6: Your driver code here
static volatile uint8_t *bar0_addr;

volatile struct tx_desc trans_descr_list[E1000_TDL_SIZE]  ;
static uint32_t tdl_index = 0;

void *packet_buffer_list[E1000_TDL_SIZE];

 uint32_t get_low(uint64_t x){
	 return	(uint32_t)(x);
}

static uint32_t
getreg(uint32_t offset)
{
  return *(volatile uint32_t *) (bar0_addr + offset);
}

static void
setreg(uint32_t offset, uint32_t value)
{
  *(volatile uint32_t *) (bar0_addr + offset) = value;
}
void transmit_initialization( volatile uint8_t *addr){
	setreg(E1000_TDBAL, PADDR((void *)trans_descr_list));
	uint32_t tdbal = getreg(E1000_TDBAL);

	uint32_t *tdbah = (uint32_t*)(bar0_addr + E1000_TDBAH);
	setreg(E1000_TDBAH, 0);

	uint32_t *tdlen = (uint32_t*)(bar0_addr + E1000_TDLEN);
	setreg(E1000_TDLEN, (uint32_t)E1000_TDL_SIZE*sizeof(struct tx_desc)) ;

	volatile uint32_t *tdh = (uint32_t*)(bar0_addr + E1000_TDH);
	setreg(E1000_TDH, 0) ;

	volatile uint32_t *tdt = (uint32_t*)(bar0_addr + E1000_TDT);
	setreg(E1000_TDT, 0);
	
	volatile uint32_t *tctl = (uint32_t*)(bar0_addr + E1000_TCTL);
	setreg(E1000_TCTL, E1000_TCTL_EN | E1000_TCTL_PSP | E1000_TCTL_COLD_FULL_DUPLEX);

	volatile uint32_t *tipg = (uint32_t*)(bar0_addr + E1000_TIPG);
	setreg(E1000_TIPG,E1000_TIPG_IEEE);

	//cprintf("TDBAL = %x \nTDBAH = %x  \nTDLEN = %d \nTDT = %x \nTDH = %x \nTCTL = %x \nTIPG = %x\n",tdbal,*tdbah,*tdlen,*tdt,*tdh,*tctl,*tipg);
	if( *tdlen & 0xFF)  
 		panic("TDLEN unaligned\n"); 

 	if( tdbal & 0xF)  
 		panic("TDBAL unaligned\n"); 

}



struct tx_desc
make_packet(uint8_t *data, uint32_t size){
	struct tx_desc desc;
	desc.addr = (uint64_t)((uint32_t)PADDR(data));
	desc.length = (uint16_t)size;
	desc.cso = 0;
	desc.cmd = E1000_CMD_RS | E1000_CMD_EOP;
	desc.status = 0;
	desc.css = 0;
	desc.special = 0;
	return desc;
}

int copy_from_user_space(envid_t srcenvid, void *srcva, envid_t dstenvid, void *dstva, int perm){
	struct Env * srcEnv;
	struct Env * dstEnv;
	if((envid2env(srcenvid, &srcEnv, 1)<0) || (envid2env(dstenvid, &dstEnv, 1)<0) )
		return -E_BAD_ENV;

	if((uint32_t)srcva >= UTOP )
		return -E_INVAL;

	if(!(perm&(PTE_U | PTE_P)) || (perm&(~PTE_SYSCALL)) )
		return -E_INVAL;


	pte_t * pte_store;
	struct PageInfo *pageI = page_lookup(srcEnv->env_pgdir, srcva, &pte_store);
	if(!pageI)
		return -E_INVAL;

	if((perm & PTE_W) && !(*pte_store&PTE_W))
		return -E_INVAL;


	if(page_insert(dstEnv->env_pgdir, pageI, dstva, perm)<0){
		page_free(pageI);
		return -E_NO_MEM;
	}

	return 0;

}

int e1000_packet_try_send(uint8_t *data, uint32_t size, uint32_t envid){
	cprintf("printing data \n");
	for(uint32_t i = 0; i < size; i++){
		cprintf("%d ",data[i]);
	}
	cprintf("\n");
	volatile uint32_t tdt = getreg(E1000_TDT);
	
	int e = copy_from_user_space(envid, (void *)data, 0, (void *)packet_buffer_list[tdt], PTE_U | PTE_P | PTE_W);
	if(e) return e;

	uint32_t pg_offset = (uint32_t)data & 0xfff; 
	packet_buffer_list[tdt] = (void *)((uint32_t )packet_buffer_list[tdt]  + pg_offset);
	
	struct tx_desc desc = make_packet((uint8_t *)packet_buffer_list[tdt],size);

	uint8_t *data_again = (uint8_t *)(packet_buffer_list[tdt]);
	cprintf("printing data again \n");
	
	for(uint32_t i = 0; i < size; i++){
		cprintf("%d ",data_again[i]);
	}
	cprintf("\n");

	if( !(trans_descr_list[tdt].status & E1000_STATUS_DD) ){
		return -E_E1000_TRANS_FULL;
	}
	
	trans_descr_list[tdt] = desc;

	if(tdt == E1000_TDL_SIZE - 1){//update tdt
		setreg(E1000_TDT, 0);
	}
	else{
		setreg(E1000_TDT, tdt + 1);
	}
	return 0;
}

int attach_e1000(struct pci_func *pcif){

	pci_func_enable(pcif);

	bar0_addr = mmio_map_region(pcif->reg_base[0],pcif->reg_size[0]);

	uint32_t *status_reg = (void *)((uint32_t )bar0_addr + (uint32_t)0x8 );

	cprintf("reg_base = %x \nreg_size = %d \nmap = %x \ntdl = %x \n",pcif->reg_base[0], pcif->reg_size[0], bar0_addr, trans_descr_list); 
	cprintf("status_reg = %x \n", *status_reg); 

	transmit_initialization(bar0_addr);
	for(int i=0; i<E1000_TDL_SIZE; i++){
		struct PageInfo *page = page_alloc(PTE_U | PTE_P | PTE_W);
		char *page_va = page2kva(page);
		packet_buffer_list[i] = page_va;
		trans_descr_list[i].cmd = E1000_CMD_RS;
		trans_descr_list[i].status = E1000_STATUS_DD;

	}


	
	return 0;
}
