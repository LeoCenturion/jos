#include <kern/e1000.h>

// LAB 6: Your driver code here
int attach_e1000(struct pci_func *pcif){
	pci_func_enable(pcif);
	return 0;
}
