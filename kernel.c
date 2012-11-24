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
#include "macros.h"
#include "atag.h"

typedef void kentry(int, int, void*);

void kernel_cmdline(char * cmdline) {
    if (strlen(cmdline)) strncpy(settings.kernel_cmdline, cmdline, sizeof(settings.kernel_cmdline)-1);
    printl("Kernel command line: \"%s\"" NEWLINE, settings.kernel_cmdline);
}

void kernel_boot(char * ignored __attribute__((unused))) {
    kentry* entry = (kentry*)settings.kernel.addr;

    if (!settings.kernel_loaded) {
        printl("Kernel not loaded." NEWLINE);
        return;
    }

    /* Kernels and ramdisks should already be loaded to their correct places */
    /* Build atag next */
    if (atag_build()) return;

    clear_cache();
    /* Disable D-Cache and MMU */
    asm volatile("mrc p15, 0, r0, c1, c0, 0 " NEWLINE
                 "bic r0, r0, #0x5 " NEWLINE
                 "mcr p15, 0, r0, c1, c0,0 " NEWLINE
                 : : : "r0" );
    /* Bye bye */
    entry(0, settings.machine_id, settings.atag.start);
    __builtin_unreachable();
}
