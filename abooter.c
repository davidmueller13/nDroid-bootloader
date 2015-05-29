#include "common.h"
#include "memory.h"
#include "bootimg.h"

void load_bootimg(const char *filename) {
    size_t bootimg_size = file_size(filename);
    size_t size_free_memory = mem_block_size_free() -
        (settings.kernel.size + settings.initrd.size +
        sizeof(settings.kernel_cmdline) + 2048);
    static unsigned char buf[ABOOT_PAGE_SIZE];
    struct boot_img_hdr *hdr = (void*) buf;
    unsigned int n;
    FILE *f;

    if (!bootimg_size) {
        printl("boot.img doesn't exist or empty" NEWLINE);
        return;
    }

    if (bootimg_size > size_free_memory) {
        printl( "boot.img is too large!" NEWLINE
                "Tried to load %u bytes into %u bytes of free space" NEWLINE,
                bootimg_size, size_free_memory);
        return;
    }

    f = fopen(filename, "rb");
    if (!f) {
        printl("Failed to open boot image %s" NEWLINE, filename);
        return;
    }

    if (fread(buf, 1, ABOOT_PAGE_SIZE, f) != ABOOT_PAGE_SIZE) {
        printl("Failed to read boot image metadata" NEWLINE);
        return;
    }

    if (memcmp(hdr->magic, BOOT_MAGIC, BOOT_MAGIC_SIZE) != 0) {
        printl("Invalid boot image magic" NEWLINE);
        return;
    }

    n = ROUND_TO_PAGE(hdr->kernel_size);

    /* Load the kernel and hope we didn't hit anything important */
    if (fread((void*) hdr->kernel_addr, 1, n, f) != n) {
        printl("Couldn't load kernel from boot.img" NEWLINE);
        return;
    }

    /* Update mem_block.start to point to loading address */
    settings.mem_block.start = (void*) hdr->kernel_addr;
    settings.kernel.addr = settings.mem_block.start;
    settings.kernel.size = n;
    settings.kernel_loaded = 1;

    n = ROUND_TO_PAGE(hdr->ramdisk_size);

    if (fread((void*) hdr->ramdisk_addr, 1, n, f) != n) {
        printl("Couldn't load ramdisk from boot.img" NEWLINE);
        return;
    }

    settings.initrd.addr = (void*) hdr->ramdisk_addr;
    settings.initrd.size = n;
    settings.initrd_loaded = 1;

    /* Only copy the first 127 characters of cmdline */
    if (hdr->cmdline[0]) {
        for (n = 0; n < 127; n++)
            settings.kernel_cmdline[n] = hdr->cmdline[n];
        hdr->cmdline[127] = 0;
    }

    fclose(f);
    printl("Boot image successfully loaded" NEWLINE);
    return;
}
