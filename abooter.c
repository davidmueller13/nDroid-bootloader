/*
    TI-NSPIRE Linux In-Place Bootloader boot.img Module
    Copyright (C) 2015  Josh Max

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
    printl("Loading boot image kernel... ");

    if (fread(settings.mem_block.start, 1, n, f) != n) {
        printl("Couldn't load kernel from boot.img" NEWLINE);
        return;
    }

    printl("Done!" NEWLINE);

    settings.kernel.addr = settings.mem_block.start;
    settings.kernel.size = n;
    settings.kernel_loaded = 1;

    n = ROUND_TO_PAGE(hdr->ramdisk_size);

    printl("Loading boot image ramdisk...");

    /* Set initrd load address location */
    void *initrd_laddr = ((char*) settings.mem_block.start + settings.mem_block.size - n);
    initrd_laddr = ROUND_PAGE_BOUND(initrd_laddr);
    size_t needed_size = ((char*) settings.mem_block.start + settings.mem_block.size)
                        - (char*) initrd_laddr;

    if (fread(initrd_laddr, 1, n, f) != n) {
        printl("Couldn't load ramdisk from boot.img" NEWLINE);
        return;
    }

    printl("Done!" NEWLINE);

    settings.initrd.addr = initrd_laddr;
    settings.initrd.size = needed_size;
    settings.initrd_loaded = 1;

    /* Only copy the first 127 characters of cmdline */
    if (hdr->cmdline[0]) {
        for (n = 0; n < 127; n++)
            settings.kernel_cmdline[n] = hdr->cmdline[n];
        hdr->cmdline[127] = 0;
        printl("Kernel cmdline: %s" NEWLINE, hdr->cmdline);
    }

    fclose(f);
    printl("Boot image successfully loaded" NEWLINE);
    return;
}
