/* Source: RealTek8139 specifications; PCI Configuration Space Table */
#define REALTEK_VENDOR_ID	0x10EC
#define REALTEK_DEVICE_ID	0x8139

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/stddef.h>
#include <linux/pci.h>		/* Peripheral Component Interconnect Bus */


static __init int init_module(void)
{
	struct pci_dev *pdev;
	if ((pdev = pci_find_device(REALTEK_VENDOR_ID, REALTEK_DEVICE_ID, NULL)) > 0)
		printk(KERN_INFO "Device detected\n");
	return 0;
}