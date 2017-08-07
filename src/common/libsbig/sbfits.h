typedef struct sbfits_struct sbfits_t;

typedef enum {
    SBFITS_TYPE_LF,
    SBFITS_TYPE_DF,
    SBFITS_TYPE_BF,
    SBFITS_TYPE_FF,
} sbfits_type_t;

sbfits_t *sbfits_create (void);
void sbfits_destroy (sbfits_t *sbf);

int sbfits_create_file (sbfits_t *sbf, const char *imagedir, const char *prefix);
int sbfits_write_file (sbfits_t *sbf);
int sbfits_close_file (sbfits_t *sbf);

const char *sbfits_get_errstr (sbfits_t *sbf);
const char *sbfits_get_filename (sbfits_t *sbf);

void sbfits_set_ccdinfo (sbfits_t *sbf, sbig_ccd_t *ccd);
void sbfits_set_num_exposures (sbfits_t *sbf, ushort num_exposures);
void sbfits_set_observer (sbfits_t *sbf, const char *observer);
void sbfits_set_telescope (sbfits_t *sbf, const char *telescope);
void sbfits_set_filter (sbfits_t *sbf, const char *filter);
void sbfits_set_object (sbfits_t *sbf, const char *object);
void sbfits_set_temperature (sbfits_t *sbf, double setpoint, double temperature);
void sbfits_set_annotation (sbfits_t *sbf, const char *str);
void sbfits_set_focal_length (sbfits_t *sbf, double n);
void sbfits_set_aperture_diameter (sbfits_t *sbf, double n);
void sbfits_set_aperture_area (sbfits_t *sbf, double n);
void sbfits_set_site (sbfits_t *sbf, const char *name,
                      const char *lat, const char *lng, double elevation);
void sbfits_set_swcreate (sbfits_t *sbf, const char *swcreate);
void sbfits_set_imagetype (sbfits_t *sbf, sbfits_type_t type);
void sbfits_set_history (sbfits_t *sbf, const char *swmodify, const char *str);
void sbfits_set_contrast (sbfits_t *sbf, ulong cblack, ulong cwhite);
void sbfits_set_pedestal (sbfits_t *sbf, ulong pedestal);

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
