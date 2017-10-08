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
#include <sys/types.h>
#include <pwd.h>

#include "src/common/libsbig/sbig.h"
#include "src/common/libutil/log.h"
#include "src/common/libutil/xzmalloc.h"
#include "src/common/libini/ini.h"

struct options {
    char *device;
    char *sbigudrv;
    char *xpa_nsinet;
};

static char *prog;
char *dir_self (void);

void exec_subcommand (char *argv[]);

int config_cb (void *user, const char *section, const char *name,
               const char *value);

#define OPTIONS "+hx:S:c:d:X:"
static const struct option longopts[] = {
    {"exec-dir",         required_argument,  0, 'x'},
    {"sbig-udrv",        required_argument,  0, 'S'},
    {"config",           required_argument,  0, 'c'},
    {"device",           required_argument,  0, 'd'},
    {"xpa-nsinet",       required_argument,  0, 'X'},
    {"help",             no_argument,        0, 'h'},
    {0, 0, 0, 0},
};

static void usage (void)
{
    fprintf (stderr, 
"Usage: sbig [OPTIONS] COMMAND ARGS\n"
"    -x,--exec-dir DIR     set directory to search for commands\n"
"    -S,--sbig-udrv FILE   set path to SBIG universal driver .so file\n"
"    -c,--config FILE      set path to config file\n"
"    -d,--device DEV       set device type (USB1, LPT1, 192.168.1.99, ...)\n"
"    -X,--xpa-nsinet IP:PORT   set for remote ds9 preview\n"
);
}

static void help (void)
{
    usage ();
    fprintf (stderr,
"The most commonly used sbig commands are:\n"
"   info       Display info about sbig devices\n"
"   cooler     Configure the TE cooler setpoint\n"
"   cfw        Select a filter on CFW device\n"
"   snap       Take a picture\n"
"   focus      Preview images quickly in a loop\n"
);
}

int main (int argc, char *argv[])
{
    int ch;
    bool hopt = false;
    char *config_filename = NULL;
    struct options *opt;

    opt = xzmalloc (sizeof (*opt));
    opt->device = xstrdup ("USB1");

    prog = basename (argv[0]);

    while ((ch = getopt_long (argc, argv, OPTIONS, longopts, NULL)) != -1) {
        switch (ch) {
            case 'c': /* --config FILE */
                config_filename = xstrdup (optarg);
                break;
            default:
                break;
        }
    }
    if (!config_filename) {
        struct passwd *pw = getpwuid (getuid ());
        if (!pw)
            msg_exit ("Who are you?");
        if (asprintf (&config_filename, "%s/.sbig/config.ini", pw->pw_dir) < 0)
            oom ();
    }
    if (setenv ("SBIG_CONFIG_FILE", config_filename, 1) < 0)
        err_exit ("setenv");
    (void)ini_parse (config_filename, config_cb, opt);

    optind = 0;
    while ((ch = getopt_long (argc, argv, OPTIONS, longopts, NULL)) != -1) {
        switch (ch) {
            case 'c': /* --config FILE */
                break;
            case 'X': /* --xpa-nsinet */
                if (opt->xpa_nsinet)
                    free (opt->xpa_nsinet);
                opt->xpa_nsinet = xstrdup (optarg);
                break;
            case 'x': /* --exec-dir */
                if (setenv ("SBIG_EXEC_DIR", optarg, 1) < 0)
                    err_exit ("setenv");
                break;
            case 'S': /* --sbig-udrv FILE */
                if (opt->sbigudrv)
                    free (opt->sbigudrv);
                opt->sbigudrv = xstrdup (optarg);                
                break;
            case 'd': /* --device DEV */
                if (opt->device)
                    free (opt->device);
                opt->device = xstrdup (optarg);
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

    if (opt->sbigudrv) {
        if (setenv ("SBIG_UDRV", opt->sbigudrv, 1) < 0)
            err_exit ("setenv");
    }
    if (opt->xpa_nsinet) {
        if (setenv ("XPA_NSINET", opt->xpa_nsinet, 1) < 0)
            err_exit ("setenv");
    }
    if (setenv ("SBIG_DEVICE", opt->device, 1) < 0)
        err_exit ("setenv");

    if (!strcmp (dir_self (), X_BINDIR)) {
        if (setenv ("SBIG_EXEC_DIR", EXEC_DIR, 0) < 0)
            err_exit ("setenv");
    } else {
        if (setenv ("SBIG_EXEC_DIR", ".", 0) < 0)
            err_exit ("setenv");
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

    free (opt);
    return 0;
}

int config_cb (void *user, const char *section, const char *name,
               const char *value)
{
    struct options *opt = user;

    if (!strcmp (section, "system")) {
        if (!strcmp (name, "sbigudrv")) {
            if (opt->sbigudrv)
                free (opt->sbigudrv);
            opt->sbigudrv = xstrdup (value);
        } else if (!strcmp (name, "device")) {
            if (opt->device)
                free (opt->device);
            opt->device = xstrdup (value);
        }
    } else if (!strcmp (section, "ds9")) {
        if (!strcmp (name, "xpa_nsinet")) {
            if (opt->xpa_nsinet)
                free (opt->xpa_nsinet);
            opt->xpa_nsinet = xstrdup (value);
        }
    }
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
    return dirname (path);
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
