/* Source: RealTek8139 specifications; PCI Configuration Space Table */
#define REALTEK_VENDOR_ID	0x10EC
#define REALTEK_DEVICE_ID	0x8139

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/stddef.h>
#include <linux/pci.h>		/* Peripheral Component Interconnect Bus */


static struct pci_dev *probe_device(void)
{
	struct pci_dev *pdev;
	if (!pci_present()) {
		printk(KERN_ERR "PCI BIOS not supported in the system\n");
		return NULL;
	}

	if (pdev = pci_find_device(REALTEK_VENDOR_ID, REALTEK_DEVICE_ID, NULL)) {
		if (pci_enable_device(pdev)) {
			printk(KERN_ERR "Could not enable the device\n");
			return NULL;
		}
		else {
			printk(KERN_NOTICE "Device enabled\n");
		}
	}
	else {
		printk(KERN_ERR "Realtek8139 chip not found\n");
		return NULL;
	}

	return pdev;
}

static __init int init_module(void)
{
	struct pci_dev *pdev;
	if ((pdev = probe_device()) != NULL)
		printk(KERN_INFO "Device detected\n");
	return 0;
}