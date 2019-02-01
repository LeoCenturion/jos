#include <kern/e1000.h>
#include <kern/syscall.h>
#include <inc/string.h>
#include <kern/pmap.h>
#include <kern/env.h>
#include <inc/error.h>
// LAB 6: Your driver code here
static volatile uint8_t *bar0_addr;

volatile struct tx_desc trans_descr_list[E1000_TDL_SIZE]  ;
void *packet_buffer_list[E1000_TDL_SIZE];

volatile struct rx_desc recv_descr_list[E1000_RDL_LEN];
static uint8_t recv_buffer_list[E1000_RDL_LEN][RECV_BUFF_SIZE];

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
void receive_initialization(volatile uint8_t *addr){
//	setreg(E1000_RA,QEMU_MAC_ADDR_LOW);
//	setreg(E1000_RA + 0x4, QEMU_MAC_ADDR_HIGH);
	setreg(E1000_RA + 0x0 , 0x12005452);
	setreg(E1000_RA + 0x4 , 0x5634 | E1000_RAH_VALID);
	for(int i = 0; i < E1000_RDL_LEN; i++){
		recv_descr_list[i].buffer_addr = (uint32_t)PADDR(recv_buffer_list[i]);
	}
	for(int i = E1000_MTA; i < E1000_MTA + 0x200; i += 0x4){
		setreg(i,0);
	}

	setreg(E1000_RDBAL,(uint32_t)PADDR((void*)recv_descr_list));
	setreg(E1000_RDBAH, 0);

	setreg(E1000_RDLEN, E1000_RDL_LEN*sizeof(struct rx_desc));
	
	for(int i = 0; i < E1000_RDL_LEN; i++){
		memset(recv_buffer_list[i],0,RECV_BUFF_SIZE);
	}

	setreg(E1000_RDT,E1000_RDL_LEN - 1 );
	setreg(E1000_RDH,0);

	setreg(E1000_RCTL,E1000_RCTL_EN | E1000_RCTL_SECRC);

	uint32_t ral = getreg(E1000_RA);
	uint32_t rah = getreg(E1000_RA + 0x4);
	uint32_t rdbal = getreg(E1000_RDBAL);
	uint32_t rdlen = getreg(E1000_RDLEN);
	uint32_t rdt = getreg(E1000_RDT);
	uint32_t rdh = getreg(E1000_RDH);
	uint32_t rctl = getreg(E1000_RCTL);
	
//	cprintf("RAL = %lx RAH = %lx \nRDBAL = %x  \nRDLEN = %x \nRDT = %x \nRDH = %x \nRCTL = %x ",ral,rah,rdbal,rdlen,rdt,rdh,rctl);
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

int e1000_packet_try_recv(uint8_t *buff){

	uint32_t rdt = getreg(E1000_RDT);
	uint32_t next_rdt = (rdt == E1000_RDL_LEN - 1) ? 0 : rdt + 1;
	struct rx_desc desc = recv_descr_list[next_rdt];
	uint8_t *desc_buffer = recv_buffer_list[next_rdt];
	buff[0] = 1;
	if(desc.status & E1000_RXD_STAT_DD){
		uint8_t *x = (uint8_t *)((uint32_t)desc.buffer_addr);
		setreg(E1000_RDT, next_rdt);
		memcpy(buff, desc_buffer, (uint16_t)desc.length);
		return desc.length;
	}

	return -E_E1000_RECV_EMPTY;
}

int e1000_packet_try_send(uint8_t *data, uint32_t size, uint32_t envid){
	volatile uint32_t tdt = getreg(E1000_TDT);
	if(size > PKT_MAX_SIZE)
		return -E_E1000_PKT_TOO_BIG;
	memcpy(packet_buffer_list[tdt],data,size);
	struct tx_desc desc = make_packet((uint8_t *)packet_buffer_list[tdt],size);

	if( !(trans_descr_list[tdt].status & E1000_STATUS_DD) ){
		return -E_E1000_TRANS_FULL;
	}
	
	trans_descr_list[tdt] = desc;

	if(tdt == E1000_TDL_SIZE - 1){
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

//	cprintf("reg_base = %x \nreg_size = %d \nmap = %x \ntdl = %x \n",pcif->reg_base[0], pcif->reg_size[0], bar0_addr, trans_descr_list); 
//	cprintf("status_reg = %x \n", *status_reg); 

	transmit_initialization(bar0_addr);

	for(int i=0; i<E1000_TDL_SIZE; i++){
		struct PageInfo *page = page_alloc(PTE_U | PTE_P | PTE_W);
		char *page_va = page2kva(page);
		packet_buffer_list[i] = page_va;
		trans_descr_list[i].cmd = E1000_CMD_RS;
		trans_descr_list[i].status = E1000_STATUS_DD;

	}


	receive_initialization(bar0_addr);	
	return 0;
}
