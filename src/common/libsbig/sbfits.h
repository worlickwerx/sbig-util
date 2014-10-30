typedef struct sbfits_struct *sbfits_t;

sbfits_t sbfits_create (const char *imagedir, const char *prefix);
int sbfits_write (sbfits_t sbf);
int sbfits_close (sbfits_t sbf);

void sbfits_set_ccdinfo (sbfits_t sbf, sbig_ccd_t ccd);
void sbfits_set_num_exposures (sbfits_t sbf, ushort num_exposures);
void sbfits_set_observer (sbfits_t sbf, const char *observer);
void sbfits_set_telescope (sbfits_t sbf, const char *telescope);
void sbfits_set_filter (sbfits_t sbf, const char *filter);
void sbfits_set_object (sbfits_t sbf, const char *object);
void sbfits_set_temperature (sbfits_t sbf, double temperature);
void sbfits_set_annotation (sbfits_t sbf, const char *str);

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
