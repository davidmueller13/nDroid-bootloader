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

#include "kernel.h"
#include "common.h"
#include "load.h"
#include "bootimg.h"
#include "mach.h"
#include "memory.h"
#include "cmd.h"

int load_script(const char *filename) {
    FILE *script = fopen(filename, "r");
    if (!script) {
        printl("Warning: Cannot open script file %s" NEWLINE, filename);
    }else{
        while (!feof(script)) {
            char cmd[128];
            if (!fgets(cmd, sizeof(cmd), script)) break;
            if (cmd[strlen(cmd)-1] == '\n') cmd[strlen(cmd)-1] = '\0';
            printl("%s" NEWLINE, cmd);
            process_cmd(cmd);
        }
        fclose(script);
    }

    return 0;
}

/*
    Returns 1 if program should gracefully exit
    Returns 0 if further commands can be executed
*/
#define DEFINE_COMMAND(s, f)   else if (!strncmp(#s, cmd, sizeof(#s)-1)) (cmd[sizeof(#s)-1] ? f(cmd+sizeof(#s)) : f(""))
int process_cmd(char *cmd) {
    if (*cmd == '#') return 0;
    if (*cmd == '\0') return 0;
    else if (!strcmp("exit", cmd)) return 1;
    else if (!strcmp("q", cmd)) return 1;
    /*
        Define custom commands below
        DEFINE_COMMAND( function_name, function_to_call );
        function_to_call should accept one parameter which is
        pointer to arguments
    */
    DEFINE_COMMAND(kernel, load_kernel);
    DEFINE_COMMAND(initrd, load_initrd);
    DEFINE_COMMAND(dtb, load_dtb);
    DEFINE_COMMAND(script, load_script);
    DEFINE_COMMAND(dump, dump_settings);
    DEFINE_COMMAND(free, show_mem);
    DEFINE_COMMAND(mach, setget_mach);
    DEFINE_COMMAND(phys, setget_phys);
    DEFINE_COMMAND(rdsize, setget_rdisksize);
    DEFINE_COMMAND(cmdline, kernel_cmdline);
    DEFINE_COMMAND(boot, kernel_boot);
    DEFINE_COMMAND(probemem, force_guess_memory);
    DEFINE_COMMAND(poke, poke);
    DEFINE_COMMAND(peek, peek);
    DEFINE_COMMAND(enterbkpt, break_on_entry);
    DEFINE_COMMAND(aboot, load_bootimg);
    /*
        End command list. Do not add any more after here
    */
    else { printl("Unknown command" NEWLINE); }
    return 0;
}
#undef DEFINE_COMMAND
