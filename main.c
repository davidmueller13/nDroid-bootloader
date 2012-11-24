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
#include <nspireio2.h>
#include "common.h"
#include "macros.h"
#include "memory.h"
#include "cmd.h"
#include "mach.h"

struct params settings;
nio_console csl;

int main(int argc, char *argv[]) {
    nio_InitConsole(&csl,SCREEN_WIDTH,SCREEN_HEIGHT,0,0,WHITE,BLACK);

    printl("Linux in-place bootloader v2" NEWLINE);
    alloc_memory();

    if (detect_machine())
        printl("Warning: Could not detect machine number!" NEWLINE);
    else
        printl("Machine number: %d" NEWLINE, settings.machine_id);

    if (detect_memory())
        printl("Warning: Could not detect amount of memory!" NEWLINE);
    else
        printl("Physical memory at: 0x%p-0x%p" NEWLINE,
            settings.phys.start, (void*)((char*)settings.phys.start + settings.phys.size));

    if (detect_serialnr())
        printl("Warning: Could not get serial number!" NEWLINE);
    else
        printl("Serial number: %x%x rev%d (%s)" NEWLINE,
                settings.serialnr[1], settings.serialnr[0], settings.rev,
                settings.rev?"CAS":"Non-CAS");

    if (argc > 1) {
        FILE *script = fopen(argv[1], "r");
        if (!script) {
            printl("Warning: Cannot open script file %s" NEWLINE, argv[1]);
        }else{
            while (!feof(script)) {
                char cmd[128];
                if (!fgets(cmd, 128, script)) break;
                if (cmd[strlen(cmd)-1] == '\n') cmd[strlen(cmd)-1] = '\0';
                printl("%s" NEWLINE, cmd);
                process_cmd(cmd);

            }
            fclose(script);
        }
    }

    while (1) {
        char cmd[128];
        nio_printf(&csl, "> ");
        if (nio_GetStr(&csl, cmd)) {
            if (process_cmd(cmd)) break;
        }
    }

    free_memory();
    nio_CleanUp(&csl);
    return 0;
}
