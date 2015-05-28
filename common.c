/*
    TI-NSPIRE Linux In-Place Bootloader
    Copyright (C) 2012  Daniel Tang

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

#include <os.h>
#include "common.h"

#define DUMP(n, s, x) printl( n #x " = 0x%x" NEWLINE, (unsigned)(s.x) );
#define DUMP_LEVEL0(s, x) DUMP("", s, x)
#define DUMP_LEVEL1(s, x) DUMP("    ", s, x)
#define DUMP_LEVEL2(s, x) DUMP("        ", s, x)

#define HEADER(n, s) printl(n #s " = {" NEWLINE);
#define HEADER_LEVEL0(s) HEADER("", s)
#define HEADER_LEVEL1(s) HEADER("    ", s)
#define HEADER_LEVEL2(s) HEADER("        ", s)

#define FOOTER(n) printl(n "}" NEWLINE);
#define FOOTER_LEVEL0() FOOTER("")
#define FOOTER_LEVEL1() FOOTER("    ")
#define FOOTER_LEVEL2() FOOTER("    ")

void setget_mach(char *arg) {
    int num;
    if ( (num = strtol(arg, NULL, 10)) ) {
        settings.machine_id = num;
    }
    printl("Machine ID is set to %d" NEWLINE, settings.machine_id);
}

void setget_phys(char *arg) {
    unsigned start, size;
    if ( (start = strtoul(arg, &arg, 16)) && (size = strtoul(arg, NULL, 16)) ) {
        settings.phys.start = (void*)start;
        settings.phys.size  = size;
    }
    printl("Physical memory range is 0x%p-0x%p" NEWLINE, settings.phys.start, (void*)((char*)settings.phys.start + settings.phys.size));
}

void setget_rdisksize(char *arg) {
    unsigned num;
    if ( (num = strtoul(arg, NULL, 16)) ) {
        settings.ramdisk_size = num;
    }
    printl("Kernel RAMDISK size set to %uK" NEWLINE, settings.ramdisk_size);
}

void peek(char *arg) {
    char *endptr;
    unsigned addr;

    addr = strtoul(arg, &endptr, 16);

    if ( endptr != arg ) {
        if (addr & 0x3) {
            printl("Warning: Address 0x%x is not word-aligned" NEWLINE, addr);
        } else {
            printl("*0x%x = 0x%x" NEWLINE, addr, *(io32_t)addr);
        }
    } else {
        printl("Invalid address `%s'" NEWLINE, arg);
    }
}

void poke(char *arg) {
    unsigned addr, value;
    char *nextarg, *endptr;

    addr = strtoul(arg, &nextarg, 16);
    if (nextarg == arg) {
        goto end;
    }
    value = strtoul(nextarg, &endptr, 16);

    if ( endptr != nextarg ) {
        if (!(addr & 0x3)) {
            *(io32_t)addr = value;
        }
    } else {
        if (*nextarg == ' ') nextarg++;
        printl("Invalid value `%s'" NEWLINE, nextarg);
    }
end:
    peek(arg);
}

void break_on_entry(char *arg UNUSED) {
    settings.break_on_entry = 1;
}

void dump_settings(char *ignored UNUSED) {
    HEADER_LEVEL0(kernel);
        DUMP_LEVEL1(settings.kernel, addr);
        DUMP_LEVEL1(settings.kernel, size);
    FOOTER_LEVEL0();

    HEADER_LEVEL0(initrd);
        DUMP_LEVEL1(settings.initrd, addr);
        DUMP_LEVEL1(settings.initrd, size);
    FOOTER_LEVEL0();

    HEADER_LEVEL0(atag);
        DUMP_LEVEL1(settings.boot_param, start);
        DUMP_LEVEL1(settings.boot_param, size);
    FOOTER_LEVEL0();

    HEADER_LEVEL0(mem_block);
        DUMP_LEVEL1(settings.mem_block, start);
        DUMP_LEVEL1(settings.mem_block, size);
    FOOTER_LEVEL0();

    HEADER_LEVEL0(phys);
        DUMP_LEVEL1(settings.phys, start);
        DUMP_LEVEL1(settings.phys, size);
    FOOTER_LEVEL0();

    DUMP_LEVEL0(settings, machine_id);
    DUMP_LEVEL0(settings, rev);
    DUMP_LEVEL0(settings, ramdisk_size);
    DUMP_LEVEL0(settings, initrd_loaded);
    DUMP_LEVEL0(settings, kernel_loaded);
    DUMP_LEVEL0(settings, dtb_loaded);
    DUMP_LEVEL0(settings, break_on_entry);

    printl("kernel_cmdline = \"%s\"" NEWLINE, settings.kernel_cmdline);
    printl("serialnr = 0x%x%x" NEWLINE, settings.serialnr[1], settings.serialnr[0]);

}

/* Standalone memcpy for when we can't have external dependencies */
void *builtin_memcpy(void *_dst, const void *_src, size_t size) {
    const char *src = _src;
    char *dst = _dst;

    while (size--)
        *dst++ = *src++;

    return _dst;
}
