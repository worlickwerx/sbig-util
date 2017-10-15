/*****************************************************************************
 *  Copyright (c) 2017 Jim Garlick All rights reserved.
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
#include <stdio.h>
#include <libgen.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <getopt.h>
#include <ctype.h>

#include "src/common/libsbig/sbig.h"
#include "src/common/libutil/xzmalloc.h"
#include "src/common/libutil/log.h"
#include "src/common/libutil/bcd.h"
#include "src/common/libutil/hash.h"
#include "src/common/libutil/list.h"

#define MAX_KEYLEN  128

struct xhash {
    hash_t hash;
    List keys;
};

void get_driver_info  (sbig_t *sb, const char *key_prefix, struct xhash *h);
void get_cooling_info  (sbig_t *sb, CAMERA_TYPE camera_type,
                        const char *key_prefix, struct xhash *h);
void get_cfw_info  (sbig_t *sb, const char *key_prefix, struct xhash *h);
void get_ccd_info  (sbig_t *sb, const char *key_prefix, struct xhash *h);

void xhash_insert (struct xhash *h, const char *prefix, const char *name,
                   const char *fmt, ...);
struct xhash *xhash_create (void);
void xhash_destroy (struct xhash *h);
void xhash_sort (struct xhash *h);

void ctl_list_value (struct xhash *h, const char *key_match);
void ctl_set_value (struct xhash *h, const char *key_value);


#define OPTIONS "ah"
static const struct option longopts[] = {
    {"all",           no_argument,           0, 'a'},
    {"help",          no_argument,           0, 'h'},
    {0, 0, 0, 0},
};

void usage (void)
{
    fprintf (stderr,
"Usage: sbig ctl key[=val]\n"
"       sbig ctl -a\n"
);
    exit (1);
}

int main (int argc, char *argv[])
{
    const char *sbig_udrv = getenv ("SBIG_UDRV");
    const char *sbig_device = getenv ("SBIG_DEVICE");
    int ch;
    bool list_all = false;
    int e;
    sbig_t *sb;
    CAMERA_TYPE camera_type;
    struct xhash *h;

    log_init ("sbig-ctl");

    while ((ch = getopt_long (argc, argv, OPTIONS, longopts, NULL)) != -1) {
        switch (ch) {
            case 'a': /* --all */
                list_all = true;
                break;
            case 'h': /* --help */
            default:
                usage ();
        }
    }
    if (list_all && optind < argc)
        usage ();
    if (!list_all && optind == argc)
        usage ();

    if (!sbig_device)
        msg_exit ("SBIG_DEVICE is not set");

    /* Initialize driver
     */
    if (!(sb = sbig_new ()))
        err_exit ("sbig_new");
    if (sbig_dlopen (sb, sbig_udrv) != 0)
        msg_exit ("%s", dlerror ());
    if ((e = sbig_open_driver (sb)) != 0)
        msg_exit ("sbig_open_driver: %s", sbig_get_error_string (sb, e));

    /* Link to camera
     */
    if ((e = sbig_open_device (sb, sbig_device) != 0))
        msg_exit ("%s: %s", sbig_device, sbig_get_error_string (sb, e));
    if ((e = sbig_establish_link (sb, &camera_type)) != 0)
        msg_exit ("sbig_establish_link: %s", sbig_get_error_string (sb, e));

    /* Populate hash
     */
    h = xhash_create ();
    xhash_insert (h, NULL, "model", "%s", sbig_strcam (camera_type));

    get_driver_info (sb, "driver", h);
    get_cooling_info (sb, camera_type, "cooling", h);
    get_cfw_info (sb, "cfw", h);
    get_ccd_info (sb, "ccd", h);

    xhash_sort (h);
    if (list_all) {
        ctl_list_value (h, "");
    }
    else {
        char kv[2*MAX_KEYLEN];
        int i, n, used;
        kv[0] = '\0';
        for (i = optind; i < argc; i++) {
            used = strlen (kv);
            n = snprintf (kv + used, sizeof (kv) - used, "%s", argv[i]);
            if (n >= sizeof (kv) - used)
                errn_exit (EINVAL, "buffer exceeded");
        }
       if (strchr (kv, '='))
            ctl_set_value (h, kv);
       else
            ctl_list_value (h, kv);
    }

    xhash_destroy (h);

    /* Disconnect from camera
     */
    if ((e = sbig_close_device (sb)) != 0)
        msg_exit ("sbig_close_device: %s", sbig_get_error_string (sb, e));

    /* Finalize driver
     */
    sbig_destroy (sb);

    log_fini ();
    return 0;
}

/* List all keys that match "^key_match"
 */
void ctl_list_value (struct xhash *h, const char *key_match)
{
    ListIterator itr = list_iterator_create (h->keys);
    const char *key, *val;
    while ((key = list_next (itr))) {
        val = hash_find (h->hash, key);
        if (!strncmp (key, key_match, strlen (key_match)))
            printf ("%s = %s\n", key, val);
    }
    list_iterator_destroy (itr);
}

void ctl_set_value (struct xhash *h, const char *key_value)
{
    char key[MAX_KEYLEN*2];
    char *val;

    /* Parse out key=value
     */
    while (isspace (*key_value))
        key_value++;
    if (snprintf (key, sizeof (key), "%s", key_value) >= sizeof (key))
        errn_exit (EINVAL, "input buffer exceeded");
    val = strchr (key, '=');
    if (!val)
        errn_exit (EINVAL, "missing =value");
    do {
        *val++ = '\0';
    } while (isspace (*val));
    if (strlen (val) == 0)
        errn_exit (EINVAL, "missing =value");
    while (isspace (key[strlen (key) - 1]))
        key[strlen (key) - 1] = '\0';
    if (strlen (key) == 0)
        errn_exit (EINVAL, "missing key=");
    /* Try to set it
     */
    msg ("set '%s' to '%s'", key, val);
}

void xhash_sort (struct xhash *h)
{
    list_sort (h->keys, (ListCmpF)strcmp);
}

void xhash_destroy (struct xhash *h)
{
    if (h) {
        list_destroy (h->keys);
        hash_destroy (h->hash);
        free (h);
    }
}


struct xhash *xhash_create (void)
{
    struct xhash *h = xzmalloc (sizeof (*h));
    h->hash = hash_create (0,  (hash_key_f)hash_key_string,
                               (hash_cmp_f)strcmp,
                               (hash_del_f)free);
    h->keys = list_create ((ListDelF)free);
    return h;
}

void xhash_insert (struct xhash *h, const char *prefix, const char *name,
                   const char *fmt, ...)
{
    char *key;
    va_list ap;
    char *val;

    if (prefix)
        key = xasprintf ("%s.%s", prefix, name);
    else
        key = xstrdup (name);
    va_start (ap, fmt);
    val = xvasprintf (fmt, ap);
    va_end (ap);
    hash_insert (h->hash, key, val);
    list_append (h->keys, key);
}

bool has_int_tracking (sbig_t *sb)
{
    sbig_ccd_t *ccd;

    if (sbig_ccd_create (sb, CCD_TRACKING, &ccd) != CE_NO_ERROR)
        return false;
    sbig_ccd_destroy (ccd);
    return true;
}

bool has_ext_tracking_capability (sbig_t *sb)
{
    sbig_ccd_t *ccd;
    GetCCDInfoResults4 info4;
    bool result = false;

    if (sbig_ccd_create (sb, CCD_TRACKING, &ccd) != CE_NO_ERROR)
        goto done;
    if (sbig_ccd_get_info4 (ccd, &info4) != CE_NO_ERROR)
        goto done_destroy;
    if ((info4.capabilitiesBits & CB_CCD_EXT_TRACKER_MASK)
                                                == CB_CCD_EXT_TRACKER_YES)
        result = true;
done_destroy:
    sbig_ccd_destroy (ccd);
done:
    return result;
}

bool has_fan (sbig_t *sb)
{
    sbig_ccd_t *ccd;
    GetCCDInfoResults0 info0;
    bool result = true;

    if (sbig_ccd_create (sb, CCD_IMAGING, &ccd) != CE_NO_ERROR)
        goto done;
    if (sbig_ccd_get_info0 (ccd, &info0) != CE_NO_ERROR)
        goto done_destroy;
    if (info0.cameraType == ST5C_CAMERA)
        result = false;
done_destroy:
    sbig_ccd_destroy (ccd);
done:
    return result;
}

void get_cooling_info (sbig_t *sb, CAMERA_TYPE camera_type,
                       const char *key_prefix, struct xhash *h)
{
    QueryTemperatureStatusResults2 info;
    int e;
    bool have_int_tracking = has_int_tracking (sb);
    bool have_ext_tracking = has_ext_tracking_capability (sb);
    bool have_fan = has_fan (sb);

    if ((e = sbig_temp_get_info (sb, &info)) != CE_NO_ERROR)
        msg_exit ("sbig_temp_get_info: %s", sbig_get_error_string (sb, e));

    /* Main cooling status
     */
    xhash_insert (h, key_prefix, "enabled", "%d", info.coolingEnabled);
    xhash_insert (h, key_prefix, "ambient", "%.2f", info.ambientTemperature);
    xhash_insert (h, key_prefix, "heatsink", "%.2f", info.heatsinkTemperature);

    xhash_insert (h, key_prefix, "imaging.setpoint", "%.2f", info.ccdSetpoint);
    xhash_insert (h, key_prefix, "imaging.temperature", "%.2f",
                              info.imagingCCDTemperature);
    xhash_insert (h, key_prefix, "imaging.power", "%.0f",
                              info.imagingCCDPower);

    /* Internal guide chip cooling status
     */
    if (have_int_tracking) {
        xhash_insert (h, key_prefix, "tracking.setpoint", "%.2f",
                                  info.trackingCCDSetpoint);
        xhash_insert (h, key_prefix, "tracking.temperature", "%.2f",
                                  info.trackingCCDTemperature);
        xhash_insert (h, key_prefix, "tracking.power", "%.0f",
                                  info.trackingCCDPower);
    }

    /* Remote guide head cooling status
     * N.B. where is the 'setpoint'?
     */
    if (have_ext_tracking) {
        xhash_insert (h, key_prefix, "tracking-ext.temperature", "%.2f",
                                  info.externalTrackingCCDTemperature);
        xhash_insert (h, key_prefix, "tracking-ext.power", "%.0f",
                                  info.externalTrackingCCDPower);
    }

    /* Fan status
     */
    if (have_fan) {
        xhash_insert (h, key_prefix, "fan.enabled", "%s",
                                  info.fanEnabled == FS_OFF ? "off" :
                                  info.fanEnabled == FS_ON ? "manual" : "auto");
        xhash_insert (h, key_prefix, "fan.power", "%.0f", info.fanPower);
        xhash_insert (h, key_prefix, "fan.rpm", "%.0f", info.fanSpeed);
    }
}

void get_driver_info (sbig_t *sb, const char *key_prefix, struct xhash *h)
{
    GetDriverInfoResults0 info;
    int e;
    char version[16];

    if ((e = sbig_get_driver_info (sb, DRIVER_STD, &info)) != 0)
        msg_exit ("sbig_get_driver_info: %s", sbig_get_error_string (sb, e));
    bcd4str (info.version, version, sizeof (version));

    xhash_insert (h, key_prefix, "version", "%s", version);
    xhash_insert (h, key_prefix, "name", "%s", info.name);
    xhash_insert (h, key_prefix, "maxreq", "%d", info.maxRequest);
}

void get_cfw_info (sbig_t *sb, const char *key_prefix, struct xhash *h)
{
    int e;
    ulong fwrev, numpos;
    CFW_MODEL_SELECT model;
    char version[16];
    CFW_STATUS status;
    CFW_POSITION position;

    if ((e = sbig_cfw_get_info (sb, &model, &fwrev, &numpos)) != 0)
        msg_exit ("sbig_cfw_get_info: %s", sbig_get_error_string (sb, e));
    bcd4str (fwrev, version, sizeof (version));

    xhash_insert (h, key_prefix, "model", "%s", sbig_strcfw (model));
    xhash_insert (h, key_prefix, "firmware-revision", "%s", version);
    xhash_insert (h, key_prefix, "slots", "%d", numpos);

    if ((e = sbig_cfw_query (sb, &status, &position)) != CE_NO_ERROR)
        msg_exit ("sbig_cfw_query: %s", sbig_get_error_string (sb, e));

    xhash_insert (h, key_prefix, "status", "%s",
                                status == CFWS_UNKNOWN ? "unknown" :
                                status == CFWS_IDLE ? "idle" : "busy");
    if (position == CFWP_UNKNOWN)
        xhash_insert (h, key_prefix, "position", "%s", "unknown");
    else
        xhash_insert (h, key_prefix, "position", "%d", position);
}

/* Query 0 obtains Results0 for imaging ccd
 * Query 1 obtains Results0 for tracking ccd
 * 'ccd' parameter will already be tied to imaging or tracking.
 */
static void get_ccd_info0 (sbig_t *sb, sbig_ccd_t *ccd,
                           const char *key_prefix, struct xhash *h)
{
    GetCCDInfoResults0 info0;
    int i, e;
    char version[16];

    e = sbig_ccd_get_info0 (ccd, &info0);
    if (e != CE_NO_ERROR)
        msg_exit ("sbig_ccd_get_info: %s", sbig_get_error_string (sb, e));

    bcd4str (info0.firmwareVersion, version, sizeof (version));
    xhash_insert (h, key_prefix, "firmware-revision", "%s", version);

    for (i = 0; i < info0.readoutModes; i++) {
        char mode[128];
        snprintf (mode, sizeof (mode), "%s.mode.%d",
                  key_prefix, info0.readoutInfo[i].mode);
        xhash_insert (h, mode, "width", "%d", info0.readoutInfo[i].width);
        xhash_insert (h, mode, "height", "%d", info0.readoutInfo[i].height);
        xhash_insert (h, mode, "gain", "%2.2f",
                      bcd2_2 (info0.readoutInfo[i].gain));
        xhash_insert (h, mode, "pixel-width", "%3.2f",
                      bcd6_2 (info0.readoutInfo[i].pixelWidth));
        xhash_insert (h, mode, "pixel-height", "%3.2f",
                      bcd6_2 (info0.readoutInfo[i].pixelHeight));
    }
}

/* Query 2 obtains Results2 for imaging ccd on all but ST-5C and ST-237
 */
static void get_ccd_info2 (sbig_t *sb, sbig_ccd_t *ccd,
                           const char *key_prefix, struct xhash *h)
{
    GetCCDInfoResults2 info2;
    int e;

    e = sbig_ccd_get_info2 (ccd, &info2);
    if (e != CE_NO_ERROR) {
        if (e == CE_BAD_PARAMETER)
            return;
        msg_exit ("sbig_ccd_get_info2:  %s", sbig_get_error_string (sb, e));
    }
    xhash_insert (h, key_prefix, "badcolumns", "%d", info2.badColumns);
    xhash_insert (h, key_prefix, "abg", "%d",
                  info2.imagingABG == ABG_PRESENT ? 1 : 0);
    xhash_insert (h, key_prefix, "serial-number", "%s", info2.serialNumber);
}

/* Query 3 obtains Results3 for imaging chip on ST-5C and ST-237
 */
static void get_ccd_info3 (sbig_t *sb, sbig_ccd_t *ccd,
                           const char *key_prefix, struct xhash *h)
{
    GetCCDInfoResults3 info3;
    int e;

    e = sbig_ccd_get_info3 (ccd, &info3);
    if (e != CE_NO_ERROR) {
        if (e == CE_BAD_PARAMETER)
            return;
        msg_exit ("sbig_ccd_get_info3: %s", sbig_get_error_string (sb, e));
    }
    xhash_insert (h, key_prefix, "adsize", "%d",
                  info3.adSize == AD_12_BITS ? 12 :
                  info3.adSize == AD_16_BITS ? 16 :
                  info3.adSize == AD_UNKNOWN ? 0 : 0);

    // redundant with cfw
    //xhash_insert (h, key_prefix, "filtertype", "%s",
    //            info3.filterType == FW_EXTERNAL ? "external" :
    //            info3.filterType == FW_VANE ? "2 position" :
    //            info3.filterType == FW_FILTER_WHEEL ? "5 position" :
    //            info3.filterType == FW_UNKNOWN ? "unknown" : "unknown");
}

/* Query 4 obtains Results4 for imaging ccd
 * Query 5 obtains Results4 for tracking ccd
 * 'ccd' paramter will already be tied to imaging or tracking.
 */
static void get_ccd_info4 (sbig_t *sb, sbig_ccd_t *ccd,
                           const char *key_prefix, struct xhash *h)
{
    GetCCDInfoResults4 info4;
    int e;

    e = sbig_ccd_get_info4 (ccd, &info4);
    if (e != CE_NO_ERROR)
        msg_exit ("sbig_get_ccd_xinfo4: %s", sbig_get_error_string (sb, e));

    int ccd_type = info4.capabilitiesBits & CB_CCD_TYPE_MASK;
    int eshutter = info4.capabilitiesBits & CB_CCD_ESHUTTER_MASK;
    int ext_tracker = info4.capabilitiesBits & CB_CCD_EXT_TRACKER_MASK;
    int btdi = info4.capabilitiesBits & CB_CCD_BTDI_MASK;
    int ao8 = info4.capabilitiesBits & CB_AO8_MASK;
    int framebuf = info4.capabilitiesBits & CB_FRAME_BUFFER_MASK;
    //int sex2 = info4.capabilitiesBits & CB_REQUIRES_STARTEXP2_MASK;

    xhash_insert (h, key_prefix, "type", "%s",
                  ccd_type == CB_CCD_TYPE_FULL_FRAME ? "full-frame" :
                  "frame-transfer");
    xhash_insert (h, key_prefix, "eshutter", "%d",
                  eshutter == CB_CCD_ESHUTTER_YES ? 1 : 0);
    xhash_insert (h, key_prefix, "ext-tracker", "%d",
                  ext_tracker == CB_CCD_EXT_TRACKER_YES ? 1 : 0);
    xhash_insert (h, key_prefix, "bdti", "%d",
                  btdi == CB_CCD_BTDI_YES ? 1 : 0);
    xhash_insert (h, key_prefix, "ao8", "%d",
                  ao8 == CB_AO8_YES ? 1 : 0);
    xhash_insert (h, key_prefix, "framebuffer", "%d",
                  framebuf == CB_FRAME_BUFFER_YES ? 1 : 0);
    // not useful to expose externally
    //xhash_insert (h, key_prefix, "use-startexp2", "%d",
    //            sex2 == CB_REQUIRES_STARTEXP2_YES ? 1 : 0);
}

/* Query 6 obtains Results6 for imaging chip
 */
static void get_ccd_info6 (sbig_t *sb, sbig_ccd_t *ccd,
                           const char *key_prefix, struct xhash *h)
{
    GetCCDInfoResults6 info6;
    int e;
    int color;

    e = sbig_ccd_get_info6 (ccd, &info6);
    if (e != CE_NO_ERROR) {
        if (e == CE_BAD_PARAMETER)
            return;
        msg_exit ("sbig_ccd_get_info6: %s", sbig_get_error_string (sb, e));
    }

    // XXX cameraBits

    xhash_insert (h, key_prefix, "mech-shutter", "%d",
                  !(info6.cameraBits & 2) ? 1 : 0);

    color = (info6.ccdBits & 1) ? 1: 0;
    xhash_insert (h, key_prefix, "color", "%d", color);
    if (color) {
        xhash_insert (h, key_prefix, "color-mask", "%s",
                      !(info6.ccdBits & 2) ? "bayer" : "truesense");
    }
}

void get_ccd_info (sbig_t *sb, const char *key_prefix, struct xhash *h)
{
    int e;
    sbig_ccd_t *ccd;
    char key[MAX_KEYLEN];

    /* Get imaging chip info
     */
    e = sbig_ccd_create (sb, CCD_IMAGING, &ccd);
    if (e != CE_NO_ERROR)
        msg_exit ("sbig_ccd_create: %s", sbig_get_error_string (sb, e));
    snprintf (key, sizeof (key), "%s.%s", key_prefix, "imaging");

    get_ccd_info0 (sb, ccd, key, h);
    get_ccd_info2 (sb, ccd, key, h);
    get_ccd_info3 (sb, ccd, key, h);
    get_ccd_info4 (sb, ccd, key, h);
    get_ccd_info6 (sb, ccd, key, h);

    sbig_ccd_destroy (ccd);

    /* Get tracking chip info
     * Query will fail if there is none.
     */
    e = sbig_ccd_create (sb, CCD_TRACKING, &ccd);
    if (e == CE_NO_ERROR) {
        snprintf (key, sizeof (key), "%s.%s", key_prefix, "tracking");

        get_ccd_info0 (sb, ccd, key, h);
        get_ccd_info3 (sb, ccd, key, h);
        get_ccd_info4 (sb, ccd, key, h);

        sbig_ccd_destroy (ccd);
    }
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
