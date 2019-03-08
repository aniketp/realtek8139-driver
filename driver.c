/* Source: RealTek8139 specifications; PCI Configuration Space Table */
#define REALTEK_VENDOR_ID	0x10EC
#define REALTEK_DEVICE_ID	0x8139

#define DRIVER_NAME		"rltk8139"

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/stddef.h>
#include <linux/pci.h>		/* Peripheral Component Interconnect Bus */


static struct  net_device *realtk_dev;

struct realtk_private {
	struct pci_dev	*rtldev;
	void 		*mmio_addr;
	unsigned long	regs_len;
}

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
			printk(KERN_INFO "Device enabled\n");
		}
	}
	else {
		printk(KERN_ERR "Realtek8139 chip not found\n");
		return NULL;
	}

	return pdev;
}

static int realtk_init(struct pci_dev *pdev, struct net_device **dev_out)
{
	struct net_device *dev;
	struct realtk_private *tp;

	// Allocate memory for the global net_device
	if (!(dev = allocate_etherdev(sizeof(struct realtk_private)))) {
		printk(KERN_INFO "Could not allocate etherdev\n");
		return -1;
	}

	// Update pointers
	tp = dev->priv;
	tp->pci_dev = pdev;
	*dev_out = dev;

	return 0;
}

static __init int init_module(void)
{
	struct pci_dev *pdev;
	struct realtk_private *tp;
	void *ioaddr;
	u64 mmio_start, mmio_end, mmio_len, mmio_flags;

	if ((pdev = probe_device()) == NULL) {
		printk(KERN_WARNING "Device not detected\n");
		return 1;
	}

	if (realtk_init(pdev, &realtk_dev)) {
		printk(KERN_WARNING "Could not initialize device\n");
		return 1;
	}

	tp = realtk_dev->priv;

	// Get mapped I/O space from Base Address Registers (BAR1)
	mmio_start = pci_resource_start(pdev, 1);
	mmio_end = pci_resource_end(pdev, 1);
	mmio_len = pci_resource_len(pdev, 1);
	mmio_flags = pci_resource_flags(pdev, 1);
	
	// Make sure the above region is for Memory-mapped I/O
	if (!(mmio_flags & IORESOURCE_MEM)) {
		printk(KERN_INFO "Region not for Memory-mapped I/O\n");
		goto cleanup1;
	}

	if (pci_request_regions(pdev, DRIVER)) {
		printk(KERN_INFO "Could not obtain the PCI region\n");
		goto cleanup1;
	}
	pci_set_master(pdev);

	/* I/O-Remap MMI/O region */
	if (!(ioaddr = ioremap(mmio_start, mmio_len))) {
		printk(KERN_INFO "Could not I/O-Remap the address region\n");
		goto cleanup2;
	}

	// Set rest of the net_device fields
	realtk_dev->base_addr = (long) ioaddr;
	tp->mmio_addr = ioaddr;
	tp->regs_len = mmio_len;

	/* Fill up net_device structure */

	// Set 6-octet (byte) Hardware address (MAC address)
	// Broadcast address: ff:ff:ff:ff:ff:ff 
	for (int i = 0; i < 6; i++) {
		realtk_dev->dev_addr[i] = readb(realtk_dev->base_addr + i);
		realtk_dev->broadcast[i] = 0xff;
	}
	realtk_dev->hard_header_len = 14;

	memcpy(realtk_dev->name, DRIVER, sizeof(DRIVER));
	realtk_dev->irq = pdev->irq;		// Interrupt (Handler) Number
	// Handler functions
	realtk_dev->open = realtk_open;
	realtk_dev->stop = realtk_stop;
	realtk_dev->hard_start_xmit = realtk_start_xmit;
	realtk_dev->get_stats = realtk_get_stats;

	if (register_netdev(realtk_dev)) {
		printk(KERN_WARNING "Could not register net_device\n");
		goto cleanup();
	}
	
	return 0;
}