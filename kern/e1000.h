#ifndef JOS_KERN_E1000_H
#define JOS_KERN_E1000_H
#include<kern/pci.h>
#endif  // JOS_KERN_E1000_H
#define E1000_TDL_SIZE 64       /* Transmit descriptor list length in descriptors */
#define E1000_TDBAL    0x03800  /* TX Descriptor Base Address Low - RW */
#define E1000_TDBAH    0x03804  /* TX Descriptor Base Address High - RW */
#define E1000_TDLEN    0x03808  /* TX Descriptor Length - RW */
#define E1000_TDH      0x03810  /* TX Descriptor Head - RW */
#define E1000_TDT      0x03818  /* TX Descripotr Tail - RW */
#define E1000_TCTL     0x00400  /* TX Control - RW */
/* Transmit Control */
#define E1000_TCTL_RST    0x00000001    /* software reset */
#define E1000_TCTL_EN     0x00000002    /* enable tx */
#define E1000_TCTL_BCE    0x00000004    /* busy check enable */
#define E1000_TCTL_PSP    0x00000008    /* pad short packets */
#define E1000_TCTL_CT     0x00000ff0    /* collision threshold */
#define E1000_TCTL_COLD   0x003ff000    /* collision distance */
#define E1000_TCTL_SWXOFF 0x00400000    /* SW Xoff transmission */
#define E1000_TCTL_PBE    0x00800000    /* Packet Burst Enable */
#define E1000_TCTL_RTLC   0x01000000    /* Re-transmit on late collision */
#define E1000_TCTL_NRTU   0x02000000    /* No Re-transmit on underrun */
#define E1000_TCTL_MULR   0x10000000    /* Multiple request support */
#define E1000_TCTL_COLD_FULL_DUPLEX 0x00040000
#define E1000_TIPG     0x00410  /* TX Inter-packet gap -RW */
#define E1000_TIPG_IEEE 0x0060100a
/*Transmit descriptor bite definition*/
#define E1000_CMD_RS 0x8
#define E1000_STATUS_DD 0x1
#define MAX_ETH_PKT_SZ 0x5EE


struct tx_desc
{
	uint64_t addr;
	uint16_t length;
	uint8_t cso;
	uint8_t cmd;
	uint8_t status;
	uint8_t css;
	uint16_t special;
} __attribute__((packed,aligned(16)));
	
/* Receive Descriptor */
struct e1000_rx_desc {
    uint64_t buffer_addr; /* Address of the descriptor's data buffer */
    uint16_t length;     /* Length of data DMAed into data buffer */
    uint16_t csum;       /* Packet checksum */
    uint8_t status;      /* Descriptor status */
    uint8_t errors;      /* Descriptor Errors */
    uint16_t special;
} __attribute__((packed,aligned(16)));

int attach_e1000(struct pci_func *pcif);

int e1000_packet_try_send(uint8_t *data, uint32_t size,uint32_t envid);
