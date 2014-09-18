/*****************************************************************************\
 *  Copyright (c) 2014 Jim Garlick All rights reserved.
 *
 *  This file is part of the sbig-util.
 *  For details, see https://github.com/garlick/sbig-util.
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 3 of the license, or (at your option)
 *  any later version.
 *
 *  sbig-util is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the IMPLIED WARRANTY OF MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the terms and conditions of the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 *  See also:  http://www.gnu.org/licenses/
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif
#include <sys/param.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <libgen.h>
#include <stdbool.h>

#include "src/common/libsbig/sbig.h"

static char *prog;
char *dir_self (void);

void exec_subcommand (char *argv[]);

#define OPTIONS "+hx:S:"
static const struct option longopts[] = {
    {"exec-dir",         required_argument,  0, 'x'},
    {"sbig-udrv",        required_argument,  0, 'S'},
    {"help",             no_argument,        0, 'h'},
    {0, 0, 0, 0},
};

static void usage (void)
{
    fprintf (stderr, 
"Usage: sbig [OPTIONS] COMMAND ARGS\n"
"    -x,--exec-dir DIR     set directory to search for commands\n"
"    -S,--sbig-udrv FILE   set path to SBIG universal driver .so file\n"
);
}

static void help (void)
{
    usage ();
    fprintf (stderr,
"The most commonly used sbig commands are:\n"
"   info       Display info about sbig devices\n"
);
}

int main (int argc, char *argv[])
{
    int ch;
    bool hopt = false;

    prog = basename (argv[0]);

    while ((ch = getopt_long (argc, argv, OPTIONS, longopts, NULL)) != -1) {
        switch (ch) {
            case 'x': /* --exec-dir */
                if (setenv ("SBIG_EXEC_DIR", optarg, 1) < 0) {
                    fprintf (stderr, "%s: setenv", prog);
                    exit (1);
                }
                break;
            case 'S': /* --sbig-udrv FILE */
                if (setenv ("SBIG_UDRV", optarg, 1) < 0) {
                    fprintf (stderr, "%s: setenv", prog);
                    exit (1);
                }
                break;
            case 'h': /* --help  */
                hopt = true;
                break;
            default:
                usage ();
                exit (1);
        }
    }
    argc -= optind;
    argv += optind;

    if (!strcmp (dir_self (), X_BINDIR)) {
        if (setenv ("SBIG_EXEC_DIR", EXEC_DIR, 0) < 0) {
            fprintf (stderr, "%s: setenv", prog);
            exit (1);
        }
        if (setenv ("SBIG_UDRV", PATH_SBIGUDRV, 0) < 0) {
            fprintf (stderr, "%s: setenv", prog);
            exit (1);
        }
    } else {
        if (setenv ("SBIG_EXEC_DIR", ".", 0) < 0) {
            fprintf (stderr, "%s: setenv", prog);
            exit (1);
        }
        if (setenv ("SBIG_UDRV", PATH_SBIGUDRV_BUILD, 0) < 0) {
            fprintf (stderr, "%s: setenv", prog);
            exit (1);
        }
    }

    if (hopt) {
        if (argc > 0) {
            char *av[] = { argv[0], "--help", NULL };
            exec_subcommand (av);
        } else
            help ();
        exit (0);
    }
    if (argc == 0) {
        usage ();
        exit (1);
    }

    exec_subcommand (argv);
    fprintf (stderr, "`%s' is not an sbig command.  See 'sbig --help\n'",
             argv[0]);

    return 0;
}

char *dir_self (void)
{
    static char path[MAXPATHLEN];
    memset (path, 0, sizeof (path));
    if (readlink ("/proc/self/exe", path, sizeof (path) - 1) < 0) {
        fprintf (stderr, "/proc/self/exe");
        exit (1);
    }
    return path; 
}

void exec_subcommand (char *argv[])
{
    char *dir = getenv ("SBIG_EXEC_DIR");
    char path[MAXPATHLEN];
    int n;
    n = snprintf (path, sizeof (path), "%s/sbig-%s", dir, argv[0]);
    if (n < 0 || n >= sizeof (path)) {
        fprintf (stderr, "%s: subcommand path is too long\n", prog);
        exit (1);
    }
    execvp (path, argv); /* no return if successful */
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
