#include <kern/e1000.h>
#include <inc/stdio.h>
#include <kern/pmap.h>
#include <inc/string.h>

// LAB 6: Your driver code here
static volatile uint8_t *bar0_addr;

volatile struct tx_desc  *trans_descr_list  ;
static uint32_t tdl_index = 0;

void *packet_buffer_list[E1000_TDL_SIZE];

 uint32_t get_low(uint64_t x){
	return	((uint32_t*)(&x))[0];
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
	setreg(E1000_TDBAL, (uint32_t)trans_descr_list);
	uint32_t tdbal = getreg(E1000_TDBAL);

	uint32_t *tdbah = (uint32_t*)(bar0_addr + E1000_TDBAH);
	setreg(E1000_TDBAH, 0);

	uint32_t *tdlen = (uint32_t*)(bar0_addr + E1000_TDLEN);
	setreg(E1000_TDLEN, (uint32_t)E1000_TDL_SIZE*sizeof(struct tx_desc)) ;


	volatile uint32_t *tdh = (uint32_t*)(bar0_addr + E1000_TDH);
	setreg(E1000_TDH, 0) ;

	volatile uint32_t *tdt = (uint32_t*)(bar0_addr + E1000_TDT);
	setreg(E1000_TDT, 0) ;
	
	volatile uint32_t *tctl = (uint32_t*)(bar0_addr + E1000_TCTL);
	setreg(E1000_TCTL, E1000_TCTL_EN | E1000_TCTL_PSP | E1000_TCTL_COLD_FULL_DUPLEX);

	volatile uint32_t *tipg = (uint32_t*)(bar0_addr + E1000_TIPG);
	setreg(E1000_TIPG,E1000_TIPG_IEEE);

	cprintf("TDBAL = %x \nTDBAH = %x  \nTDLEN = %d \nTDT = %x \nTDH = %x \nTCTL = %x \nTIPG = %x\n",tdbal,*tdbah,*tdlen,*tdt,*tdh,*tctl,*tipg);
	if( *tdlen & 0xFF)  
 		panic("TDLEN unaligned\n"); 

 	if( tdbal & 0xF)  
 		panic("TDBAL unaligned\n"); 

}


int attach_e1000(struct pci_func *pcif){

	pci_func_enable(pcif);

	bar0_addr = mmio_map_region(pcif->reg_base[0],pcif->reg_size[0]);
	trans_descr_list =(void *) ROUNDUP(page_alloc(0),sizeof(struct tx_desc));
	cprintf("reg_base = %x \nreg_size = %d \nmap = %x \ntdl = %x \n", pcif->reg_base[0], pcif->reg_size[0], bar0_addr, trans_descr_list); 
	uint32_t *status_reg = (void *)((uint32_t )bar0_addr + (uint32_t)0x8 );
	cprintf("status_reg = %x \n", *status_reg); 

	transmit_initialization(bar0_addr);
	return 0;
}
struct tx_desc
make_packet(uint8_t *data, uint32_t size){
	struct tx_desc desc;
	desc.addr = (uint64_t)((uint32_t)data);
	desc.length = (uint16_t)size;
	desc.cso = 0;
	desc.cmd = E1000_CMD_RS;
	desc.status = 0;
	desc.css = 0;
	desc.special = 0;
	return desc;
}


int e1000_packet_try_send(uint8_t *data, uint32_t size){
	struct tx_desc desc = make_packet(data,size);
	volatile uint32_t tdt = getreg(E1000_TDT);
	trans_descr_list[tdt].cmd = desc.cmd | 0x8;

	if( !(trans_descr_list[tdt].status & E1000_STATUS_DD) ){
		return 1;
	}
	desc.cmd = desc.cmd | 0x8;
	
	packet_buffer_list[tdt] =  page_alloc(0) ; //allocate buffer
	void* addr = (void *)get_low(desc.addr);
	memcpy(packet_buffer_list[tdt],addr,desc.length);

	desc.addr =  (uint64_t)(uint32_t)packet_buffer_list[tdt];
	trans_descr_list[tdt] = desc;
	
	if(tdt == E1000_TDL_SIZE - 1){//update tdt
		setreg(E1000_TDT, 0);
	}
	else{
		setreg(E1000_TDT, tdt + 1);
	}
	return 0;
}

